#include "actor_mp.h"
#include "g_main_mp.h"
#include "g_utils_mp.h"
#include <game/actor_spawner.h>
#include <game/actor_orientation.h>
#include <game/g_actor_prone.h>
#include <game/actor_lookat.h>
#include <bgame/bg_dog_animations_mp.h>
#include <server/sv_world.h>
#include <clientscript/cscr_stringlist.h>
#include <qcommon/dobj_management.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/scr_const.h>
#include <client_mp/g_client_mp.h>
#include "g_spawn_mp.h"
#include <game/actor_state.h>
#include <universal/com_memory.h>
#include <game/actor_senses.h>
#include <game/actor_events.h>
#include <bgame/bg_dog.h>
#include <game/actor_navigation.h>
#include <game/actor_exposed.h>
#include <cgame/cg_event.h>
#include "g_combat_mp.h"
#include "g_active_mp.h"
#include <server/sv_game.h>
#include <game/actor_corpse.h>
#include <bgame/bg_slidemove.h>
#include <game/bullet.h>
#include <client_mp/cl_cgame_mp.h>
#include "g_misc_mp.h"
#include <game/g_debug.h>
#include <cgame/cg_drawtools.h>
#include <universal/com_math_anglevectors.h>
#include <xanim/dobj_utils.h>
#include <game/actor_team_move.h>
#include <glass/glass_server.h>

unsigned __int16 *modNames[21] =
{
    &scr_const.mod_unknown,
    &scr_const.mod_pistol_bullet,
    &scr_const.mod_rifle_bullet,
    &scr_const.mod_grenade,
    &scr_const.mod_grenade_splash,
    &scr_const.mod_projectile,
    &scr_const.mod_projectile_splash,
    &scr_const.mod_melee,
    &scr_const.mod_bayonet,
    &scr_const.mod_head_shot,
    &scr_const.mod_crush,
    &scr_const.mod_telefrag,
    &scr_const.mod_falling,
    &scr_const.mod_suicide,
    &scr_const.mod_trigger_hurt,
    &scr_const.mod_explosive,
    &scr_const.mod_impact,
    &scr_const.mod_burned,
    &scr_const.mod_hit_by_object,
    &scr_const.mod_drown,
    &scr_const.mod_gas,
};

float colorTeam[5][4] =
{
  { 1.0, 1.0, 1.0, 1.0 },
  { 1.0, 0.40000001, 0.40000001, 1.0 },
  { 0.5, 0.5, 1.0, 1.0 },
  { 0.0, 1.0, 0.0, 1.0 },
  { 0.5, 0.5, 0.5, 1.0 }
};

const char *ai_orient_mode_text[7] =
{
  "invalid",
  "dont_change",
  "motion",
  "enemy",
  "enemy_or_motion",
  "enemy_or_motion_sidestep",
  "goal"
};

const char *animModeNames[10] =
{
  "none",
  "(code)",
  "(pos deltas)",
  "angle_deltas",
  "gravity",
  "(noclip)",
  "nogravity",
  "zonly_physics",
  "(nophysics)",
  "point_relative"
};


static const char *g_entinfoAITextNames[6] = { "all", "brief", "combat", "movement", "state", NULL };




void __fastcall VisCache_Update(vis_cache_t *pCache, bool bVisible)
{
    if ( !pCache && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 140, 0, "%s", "pCache") )
        __debugbreak();
    pCache->bVisible = bVisible;
    pCache->iLastUpdateTime = level.time;
    if ( bVisible )
        pCache->iLastVisTime = level.time;
}

void __fastcall SentientInfo_Clear(sentient_info_t *pInfo)
{
    if ( !pInfo && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 152, 0, "%s", "pInfo") )
        __debugbreak();
    *(unsigned int *)&pInfo->VisCache.bVisible = 0;
    pInfo->VisCache.iLastUpdateTime = 0;
    pInfo->VisCache.iLastVisTime = 0;
    pInfo->iLastAttackMeTime = 0;
    pInfo->lastKnownPosTime = 0;
    pInfo->attackTime = 0;
    pInfo->vLastKnownPos[0] = 0.0;
    pInfo->vLastKnownPos[1] = 0.0;
    pInfo->vLastKnownPos[2] = 0.0;
    pInfo->pLastKnownNode = 0;
}

void __fastcall SentientInfo_Copy(actor_s *pTo, const actor_s *pFrom, int index)
{
    if ( !pTo && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 159, 0, "%s", "pTo") )
        __debugbreak();
    if ( !pFrom && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 160, 0, "%s", "pFrom") )
        __debugbreak();
    if ( (pFrom->talkToSpecies & (1 << pTo->species)) != 0
        && pTo->sentientInfo[index].lastKnownPosTime < pFrom->sentientInfo[index].lastKnownPosTime )
    {
        SentientInfo_ForceCopy(&pTo->sentientInfo[index], &pFrom->sentientInfo[index]);
    }
}

void __fastcall SentientInfo_ForceCopy(sentient_info_t *pTo, const sentient_info_t *pFrom)
{
    pTo->iLastAttackMeTime = 0;
    pTo->attackTime = 0;
    pTo->lastKnownPosTime = pFrom->lastKnownPosTime;
    pTo->vLastKnownPos[0] = pFrom->vLastKnownPos[0];
    pTo->vLastKnownPos[1] = pFrom->vLastKnownPos[1];
    pTo->vLastKnownPos[2] = pFrom->vLastKnownPos[2];
    pTo->pLastKnownNode = pFrom->pLastKnownNode;
}

int __cdecl Actor_droptofloor(gentity_s *ent)
{
    col_context_t context; // [esp+Ch] [ebp-88h] BYREF
    float dropMins[3]; // [esp+34h] [ebp-60h] BYREF
    float vEnd[3]; // [esp+40h] [ebp-54h] BYREF
    trace_t trace; // [esp+4Ch] [ebp-48h] BYREF
    float dropMaxs[3]; // [esp+88h] [ebp-Ch] BYREF

    //col_context_t::col_context_t(&context);
    vEnd[0] = ent->r.currentOrigin[0];
    vEnd[1] = ent->r.currentOrigin[1];
    vEnd[2] = ent->r.currentOrigin[2];
    ent->r.currentOrigin[2] = ent->r.currentOrigin[2] + 1.0;
    vEnd[2] = vEnd[2] - 128.0;
    dropMins[0] = actorMins[0];
    dropMins[1] = -15.0;
    dropMins[2] = 0.0;
    dropMaxs[0] = actorMaxs[0];
    dropMaxs[1] = 15.0;
    dropMaxs[2] = (float)(15.0 - -15.0) + 0.0;
    G_TraceCapsule(
        &trace,
        ent->r.currentOrigin,
        dropMins,
        dropMaxs,
        vEnd,
        1023,
        0x2820011,
        &context);
    if ( trace.startsolid )
        return 1;
    Vec3Lerp(ent->r.currentOrigin, vEnd, trace.fraction, ent->r.currentOrigin);
    return 0;
}

int __cdecl SP_actor(gentity_s *ent, SpawnVar *spawnVar)
{
    sentient_s *sentient; // [esp+38h] [ebp-Ch]
    actor_s *actor; // [esp+3Ch] [ebp-8h]
    XAnim_s *anims; // [esp+40h] [ebp-4h]
    int savedregs; // [esp+44h] [ebp+0h] BYREF

    ent->model = 0;
    if ( !g_spawnai->current.enabled )
    {
LABEL_2:
        G_FreeEntity(ent);
        return 0;
    }
    if ( (ent->spawnflags & 1) != 0 && spawnVar )
        return SP_actor_spawner(ent, spawnVar);
    Actor_droptofloor(ent);
    actor = Actor_Alloc();
    if ( !actor && (ent->spawnflags & 2) != 0 )
    {
        Actor_FreeExpendable();
        actor = Actor_Alloc();
        if ( !actor && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 265, 0, "%s", "actor") )
            __debugbreak();
    }
    if ( !actor )
        goto LABEL_2;
    sentient = Sentient_Alloc();
    if ( sentient )
    {
        ent->actor = actor;
        ent->sentient = sentient;
        ent->nextthink = level.time + 50;
        ent->handler = 2;
        ent->r.svFlags |= 2u;
        ent->takedamage = 1;
        ent->maxHealth = 100;
        ent->health = 100;
        ent->flags |= 0x1000u;
        ent->r.mins[0] = actorMins[0];
        ent->r.mins[1] = -15.0;
        ent->r.mins[2] = 0.0;
        ent->r.maxs[0] = actorMaxs[0];
        ent->r.maxs[1] = 15.0;
        ent->r.maxs[2] = 48.0;
        ent->clipmask = 0x2820011;
        ent->r.contents = 0x8000;
        ent->s.eType = ET_ACTOR;
        AssignToSmallerType<int>(&ent->s.lerp.useCount, ent->useCount + 1);
        ent->s.lerp.u.turret.ownerNum = 1023;
        G_SetOrigin(ent, ent->r.currentOrigin);
        actor->ent = ent;
        actor->sentient = sentient;
        Actor_InitMove(actor);
        Actor_InitAnim(actor);
        Actor_InitPath(actor);
        actor->accuracy = 0.2f;
        actor->playerSightAccuracy = 1.0f;
        actor->debugLastAccuracy = -6969.0f;
        actor->sideMove = 0.0f;
        actor->missCount = (__int64)(1.0 / actor->accuracy);
        actor->safeToChangeScript = 1;
        actor->ignoreTriggers = 0;
        actor->pushable = 1;
        actor->bDropWeapon = 1;
        actor->bDrawOnCompass = 1;
        actor->bActivateCrosshair = 1;
        actor->allowPain = 1;
        actor->allowDeath = 0;
        Actor_SetDesiredAngles(&actor->CodeOrient, ent->r.currentAngles[0], ent->r.currentAngles[1]);
        G_SetAngle(ent, ent->r.currentAngles);
        Actor_SetLookAngles(actor, ent->r.currentAngles[0], ent->r.currentAngles[1]);
        Actor_ClearMoveHistory(actor);
        actor->scriptGoal.pos[0] = ent->r.currentOrigin[0];
        actor->scriptGoal.pos[1] = ent->r.currentOrigin[1];
        actor->scriptGoal.pos[2] = ent->r.currentOrigin[2];
        if ( actor->scriptGoal.node
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                        342,
                        0,
                        "%s",
                        "!actor->scriptGoal.node") )
        {
            __debugbreak();
        }
        if ( actor->scriptGoal.volume
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                        343,
                        0,
                        "%s",
                        "!actor->scriptGoal.volume") )
        {
            __debugbreak();
        }
        Actor_SetGoalRadius(&actor->scriptGoal, 0.0);
        Actor_SetGoalHeight(&actor->scriptGoal, 0.0);
        memcpy(&actor->codeGoal, &actor->scriptGoal, sizeof(actor->codeGoal));
        actor->keepClaimedNode = 0;
        actor->keepClaimedNodeInGoal = 0;
        actor->noDodgeMove = 0;
        actor->goalPosChanged = 0;
        actor->fixedNode = 0;
        actor->fixedNodeSafeRadius = 64.0f;
        actor->isFacingMotion = 0;
        actor->arrivalInfo.animscriptOverrideRunTo = 0;
        actor->arrivalInfo.arrivalNotifyRequested = 0;
        actor->exposedStartTime = 0x80000000;
        actor->exposedDuration = 5000;
        Actor_InitThreatUpdateInterval(actor);
        sentient->ent = ent;
        sentient->eTeam = TEAM_SPECTATOR;
        sentient->iThreatBias = 0;
        sentient->iThreatBiasGroupIndex = 0;
        sentient->pPrevClaimedNode = 0;
        sentient->maxVisibleDist = 8192.0f;
        sentient->attackerAccuracy = 1.0f;
        sentient->oldOrigin[0] = ent->r.currentOrigin[0];
        sentient->oldOrigin[1] = ent->r.currentOrigin[1];
        sentient->oldOrigin[2] = ent->r.currentOrigin[2];
        G_InitActorProneInfo(actor);
        actor->fInvProneAnimLowPitch = 0.0f;
        actor->fInvProneAnimHighPitch = 0.0f;
        actor->fProneLastDiff = 0.0f;
        Actor_InitLookAt(actor);
        Actor_InitActorState(ent);
        anims = Dog_GetAnims();
        if ( !anims && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 403, 0, "%s", "anims") )
            __debugbreak();
        ent->pAnimTree = Com_XAnimCreateSmallTree(anims);
        if ( !ent->pAnimTree
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 405, 0, "%s", "ent->pAnimTree") )
        {
            __debugbreak();
        }
        G_DObjUpdate(ent);
        SV_LinkEntity(ent);
        Sentient_NearestNode(sentient);
        return 1;
    }
    else
    {
        actor->inuse = 0;
        G_FreeEntity(ent);
        return 0;
    }
}

void __cdecl Actor_InitActorState(gentity_s *ent)
{
    actor_s *actor; // [esp+0h] [ebp-8h]
    unsigned int actorNum; // [esp+4h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 214, 0, "%s", "ent") )
        __debugbreak();
    actor = ent->actor;
    if ( !actor && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 217, 0, "%s", "actor") )
        __debugbreak();
    if ( !actor->inuse
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 218, 0, "%s", "actor->inuse") )
    {
        __debugbreak();
    }
    actorNum = G_GetActorIndex(actor);
    if ( actorNum >= 0x10
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    221,
                    0,
                    "actorNum doesn't index MAX_ACTORS\n\t%i not in [0, %i)",
                    actorNum,
                    16) )
    {
        __debugbreak();
    }
    ent->s.lerp.u.actor.actorNum = actorNum;
}

actor_s *__cdecl Actor_Alloc()
{
    actor_s *actor; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if ( !level.actors
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    422,
                    0,
                    "%s",
                    "level.actors != NULL") )
    {
        __debugbreak();
    }
    for ( i = 0; i < 16; ++i )
    {
        actor = &level.actors[i];
        if ( !actor->inuse )
        {
            memset(actor, 0, sizeof(actor_s));
            actor->inuse = 1;
            Actor_SetDefaults(actor);
            return actor;
        }
    }
    Com_DPrintf(18, "Actor allocation failed\n");
    return 0;
}

void __cdecl Scr_FreeFields(const actor_field_t *fields, unsigned __int8 *base)
{
    while ( fields->type )
    {
        if ( fields->type == AF_STRING )
            Scr_SetString((unsigned __int16 *)&base[fields->ofs], 0, SCRIPTINSTANCE_SERVER);
        ++fields;
    }
}

const actor_field_t actorFields_0[9] =
{
  { 540, { 2 }, AF_STRING },
  { 516, { 2 }, AF_STRING },
  { 518, { 2 }, AF_STRING },
  { 520, { 2 }, AF_STRING },
  { 236, { 2 }, AF_STRING },
  { 5896, { 2 }, AF_STRING },
  { 5992, { 2 }, AF_STRING },
  { 5994, { 2 }, AF_STRING },
  { 5996, { 2 }, AF_STRING }
};

void __cdecl Scr_FreeActorFields(actor_s *pActor)
{
    Scr_FreeFields(actorFields_0, (unsigned __int8 *)pActor);
}

void __cdecl Actor_Free(actor_s *actor)
{
    DObj *dobj; // [esp+0h] [ebp-14h]
    actor_s *other; // [esp+4h] [ebp-10h]
    gentity_s *ent; // [esp+8h] [ebp-Ch]
    int i; // [esp+Ch] [ebp-8h]
    int entnum; // [esp+10h] [ebp-4h]

    if ( !actor && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 519, 0, "%s", "actor") )
        __debugbreak();
    if ( !level.actors
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 520, 0, "%s", "level.actors") )
    {
        __debugbreak();
    }
    if ( (actor < level.actors || actor >= &level.actors[16])
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    521,
                    0,
                    "%s",
                    "actor >= level.actors && actor < level.actors + MAX_ACTORS") )
    {
        __debugbreak();
    }
    if ( &level.actors[actor - level.actors] != actor
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    522,
                    0,
                    "%s",
                    "&level.actors[actor - level.actors] == actor") )
    {
        __debugbreak();
    }
    if ( !actor->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 523, 0, "%s", "actor->sentient") )
    {
        __debugbreak();
    }
    if ( !actor->inuse
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 524, 0, "%s", "actor->inuse") )
    {
        __debugbreak();
    }
    ent = actor->ent;
    if ( actor->ent->pAnimTree )
    {
        XAnimClearTree(ent->pAnimTree);
        Com_XAnimFreeSmallTree(ent->pAnimTree);
        ent->pAnimTree = 0;
        dobj = Com_GetServerDObj(ent->s.number);
        if ( dobj )
            DObjSetTree(dobj, 0);
        G_DObjUpdate(ent);
    }
    Actor_ClearPath(actor);
    if ( Scr_IsSystemActive(1u, SCRIPTINSTANCE_SERVER) )
        Actor_KillAnimScript(actor);
    Sentient_Dissociate(actor->sentient);
    entnum = ent->s.number;
    for ( i = 0; i < 16; ++i )
    {
        other = &level.actors[i];
        if ( other->inuse )
        {
            if ( other->Path.wDodgeEntity == entnum )
                other->Path.wDodgeEntity = 1023;
            if ( other->Physics.iHitEntnum == entnum )
                other->Physics.iHitEntnum = 1023;
            if ( other->pPileUpActor == actor )
                Actor_ClearPileUp(other);
        }
    }
    ent->actor = 0;
    Sentient_Free(actor->sentient);
    actor->sentient = 0;
    Scr_FreeActorFields(actor);
    ent->s.lerp.u.actor.actorNum = 16;
    memset(actor, 0xF0u, sizeof(actor_s));
    actor->inuse = 0;
}

void __cdecl Actor_FreeExpendable()
{
    float *v0; // [esp+0h] [ebp-4Ch]
    float *v1; // [esp+4h] [ebp-48h]
    float *v2; // [esp+8h] [ebp-44h]
    float *currentOrigin; // [esp+Ch] [ebp-40h]
    float fDot; // [esp+10h] [ebp-3Ch]
    float fMaxDistSqrd; // [esp+14h] [ebp-38h]
    float vRefDir[3]; // [esp+18h] [ebp-34h] BYREF
    float vRefPos[3]; // [esp+24h] [ebp-28h] BYREF
    float fDistSqrd; // [esp+30h] [ebp-1Ch]
    actor_s *pExpendable; // [esp+34h] [ebp-18h]
    actor_s *actor; // [esp+38h] [ebp-14h]
    gentity_s *player; // [esp+3Ch] [ebp-10h]
    float vDelta[3]; // [esp+40h] [ebp-Ch]

    Com_Printf(18, "^3trying to delete somebody to make room for spawned AI (time %d)\n", level.time);
    player = G_Find(0, 356, scr_const.player);
    if ( !player && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 634, 0, "%s", "player") )
        __debugbreak();
    if ( !player->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 635, 0, "%s", "player->client") )
    {
        __debugbreak();
    }
    if ( !player->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 636, 0, "%s", "player->sentient") )
    {
        __debugbreak();
    }
    Sentient_GetEyePosition(player->sentient, vRefPos);
    fMaxDistSqrd = 0.0f;
    pExpendable = 0;
    for ( actor = Actor_FirstActor(-1); actor; actor = Actor_NextActor(actor, -1) )
    {
        if ( Actor_IsDeletable(actor) )
        {
            currentOrigin = actor->ent->r.currentOrigin;
            vDelta[0] = *currentOrigin - vRefPos[0];
            vDelta[1] = currentOrigin[1] - vRefPos[1];
            vDelta[2] = currentOrigin[2] - vRefPos[2];
            fDistSqrd = (float)((float)(vDelta[0] * vDelta[0]) + (float)(vDelta[1] * vDelta[1]))
                                + (float)(vDelta[2] * vDelta[2]);
            if ( fDistSqrd > fMaxDistSqrd )
            {
                fMaxDistSqrd = fDistSqrd;
                pExpendable = actor;
            }
        }
    }
    if ( !pExpendable )
    {
        if ( fMaxDistSqrd != 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                        664,
                        0,
                        "%s",
                        "fMaxDistSqrd == 0") )
        {
            __debugbreak();
        }
        G_GetPlayerViewDirection(player, vRefDir, 0, 0);
        for ( actor = Actor_FirstActor(-1); actor; actor = Actor_NextActor(actor, -1) )
        {
            if ( Actor_IsDeletable(actor) )
            {
                v2 = actor->ent->r.currentOrigin;
                vDelta[0] = *v2 - vRefPos[0];
                vDelta[1] = v2[1] - vRefPos[1];
                vDelta[2] = v2[2] - vRefPos[2];
                if ( (float)((float)((float)(vDelta[0] * vRefDir[0]) + (float)(vDelta[1] * vRefDir[1]))
                                     + (float)(vDelta[2] * vRefDir[2])) < 0.0 )
                {
                    fDistSqrd = (float)((float)(vDelta[0] * vDelta[0]) + (float)(vDelta[1] * vDelta[1]))
                                        + (float)(vDelta[2] * vDelta[2]);
                    if ( fDistSqrd > fMaxDistSqrd )
                    {
                        fMaxDistSqrd = fDistSqrd;
                        pExpendable = actor;
                    }
                }
            }
        }
    }
    if ( !pExpendable )
    {
        if ( fMaxDistSqrd != 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                        690,
                        0,
                        "%s",
                        "fMaxDistSqrd == 0") )
        {
            __debugbreak();
        }
        for ( actor = Actor_FirstActor(-1); actor; actor = Actor_NextActor(actor, -1) )
        {
            if ( Actor_IsDeletable(actor) )
            {
                v1 = actor->ent->r.currentOrigin;
                vDelta[0] = *v1 - vRefPos[0];
                vDelta[1] = v1[1] - vRefPos[1];
                vDelta[2] = v1[2] - vRefPos[2];
                fDistSqrd = (float)((float)(vDelta[0] * vDelta[0]) + (float)(vDelta[1] * vDelta[1]))
                                    + (float)(vDelta[2] * vDelta[2]);
                if ( fDistSqrd > fMaxDistSqrd )
                {
                    fDot = (float)((float)(vDelta[0] * vRefDir[0]) + (float)(vDelta[1] * vRefDir[1]))
                             + (float)(vDelta[2] * vRefDir[2]);
                    if ( (float)(fDot * fDot) < (float)(0.5 * fDistSqrd) )
                    {
                        fMaxDistSqrd = fDistSqrd;
                        pExpendable = actor;
                    }
                }
            }
        }
    }
    if ( !pExpendable )
    {
        if ( fMaxDistSqrd != 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                        717,
                        0,
                        "%s",
                        "fMaxDistSqrd == 0") )
        {
            __debugbreak();
        }
        G_GetPlayerViewDirection(player, vRefDir, 0, 0);
        for ( actor = Actor_FirstActor(-1); actor; actor = Actor_NextActor(actor, -1) )
        {
            if ( Actor_IsDeletable(actor) )
            {
                v0 = actor->ent->r.currentOrigin;
                vDelta[0] = *v0 - vRefPos[0];
                vDelta[1] = v0[1] - vRefPos[1];
                vDelta[2] = v0[2] - vRefPos[2];
                fDistSqrd = (float)((float)(vDelta[0] * vDelta[0]) + (float)(vDelta[1] * vDelta[1]))
                                    + (float)(vDelta[2] * vDelta[2]);
                if ( fDistSqrd >= fMaxDistSqrd )
                {
                    fMaxDistSqrd = fDistSqrd;
                    pExpendable = actor;
                }
            }
        }
    }
    if ( !pExpendable )
        Com_Error(ERR_DROP, "Tried to force spawning of an AI when all AI slots are used by undeletable AI");
    Com_Printf(18, "^3deleting entity %i\n", pExpendable->ent->s.number);
    Actor_ClearPath(pExpendable);
    Scr_Notify(pExpendable->ent, scr_const.death, 0);
    G_FreeEntity(pExpendable->ent);
    if ( pExpendable->ent->actor
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    753,
                    0,
                    "%s",
                    "pExpendable->ent->actor == NULL") )
    {
        __debugbreak();
    }
    if ( pExpendable->ent->sentient
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    754,
                    0,
                    "%s",
                    "pExpendable->ent->sentient == NULL") )
    {
        __debugbreak();
    }
    if ( pExpendable->inuse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    756,
                    0,
                    "%s",
                    "!pExpendable->inuse") )
    {
        __debugbreak();
    }
    if ( pExpendable->sentient->inuse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    757,
                    0,
                    "%s",
                    "!pExpendable->sentient->inuse") )
    {
        __debugbreak();
    }
}

bool __cdecl Actor_IsDeletable(actor_s *actor)
{
    return (actor->ent->spawnflags & 4) == 0 && actor->ent->s.number != level.currentEntityThink;
}

void __cdecl G_InitActors()
{
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 16; ++i )
        level.actors[i].inuse = 0;
}

int __cdecl G_GetActorIndex(actor_s *actor)
{
    if ( !actor && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 780, 0, "%s", "actor") )
        __debugbreak();
    if ( actor - level.actors < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    781,
                    0,
                    "%s",
                    "actor - level.actors >= 0") )
    {
        __debugbreak();
    }
    if ( actor - level.actors >= 16
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    782,
                    0,
                    "%s",
                    "actor - level.actors < MAX_ACTORS") )
    {
        __debugbreak();
    }
    return actor - level.actors;
}

XAnimTree_s *__cdecl G_GetActorAnimTree(actor_s *actor)
{
    return *(XAnimTree_s **)&g_scr_data.actorCorpseInfo[4 * G_GetActorIndex(actor) - 32];
}

void __cdecl Actor_SetDefaults(actor_s *actor)
{
    Actor_SetDefaultState(actor);
    actor->species = AI_SPECIES_DOG;
    actor->talkToSpecies = -1;
    actor->deathContents = (int)&objBuf[1824][4];
    actor->fovDot = ACTOR_DEFAULT_FOV_COS;
    actor->fMaxSightDistSqrd = 6.7108864e7f;
    actor->eTraverseMode = AI_TRAVERSE_NOGRAVITY;
    actor->CodeOrient.eMode = AI_ORIENT_DONT_CHANGE;
    actor->ScriptOrient.eMode = AI_ORIENT_INVALID;
    actor->iPacifistWait = 20000;
    actor->fWalkDist = ACTOR_DEFAULT_WALK_DIST;
    actor->fInterval = ACTOR_DEFAULT_INTERVAL;
    actor->grenadeAwareness = ACTOR_DEFAULT_GRENADE_AWARENESS;
    actor->badPlaceAwareness = ACTOR_DEFAULT_BADPLACE_AWARENESS;
    actor->bThrowbackGrenades = 1;
    actor->goodShootPosValid = 0;
}

void __cdecl Actor_FinishSpawning(actor_s *self)
{
    unsigned __int16 hThread; // [esp+0h] [ebp-10h]
    char *classname; // [esp+4h] [ebp-Ch]
    AITypeScript *typeScript; // [esp+8h] [ebp-8h]
    gentity_s *ent; // [esp+Ch] [ebp-4h]

    if ( !self->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 839, 0, "%s", "self->ent") )
    {
        __debugbreak();
    }
    if ( self->ent->actor != self
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    840,
                    0,
                    "%s",
                    "self->ent->actor == self") )
    {
        __debugbreak();
    }
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 841, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( self->ent->sentient != self->sentient
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    842,
                    0,
                    "%s",
                    "self->ent->sentient == self->sentient") )
    {
        __debugbreak();
    }
    if ( self->sentient->ent != self->ent
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    843,
                    0,
                    "%s",
                    "self->sentient->ent == self->ent") )
    {
        __debugbreak();
    }
    ent = self->ent;
    classname = SL_ConvertToString(self->ent->classname, SCRIPTINSTANCE_SERVER);
    if ( strncmp(classname, "actor_", 6u)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    850,
                    0,
                    "%s",
                    "!strncmp( classname, ACTOR_CLASSNAME_PREFIX, ACTOR_CLASSNAME_PREFIX_LEN )") )
    {
        __debugbreak();
    }
    typeScript = (AITypeScript *)Hunk_FindDataForFile(0, classname + 6);
    if ( !typeScript
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 853, 0, "%s", "typeScript") )
    {
        __debugbreak();
    }
    if ( !typeScript->main
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 854, 0, "%s", "typeScript->main") )
    {
        __debugbreak();
    }
    hThread = Scr_ExecEntThread(ent, typeScript->main, 0);
    Scr_FreeThread(hThread, SCRIPTINSTANCE_SERVER);
}

void __cdecl Actor_InitAnimScript(actor_s *self)
{
    unsigned __int16 hThread; // [esp+4h] [ebp-8h]
    gentity_s *ent; // [esp+8h] [ebp-4h]

    if ( !self->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 875, 0, "%s", "self->ent") )
    {
        __debugbreak();
    }
    if ( self->ent->actor != self
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    876,
                    0,
                    "%s",
                    "self->ent->actor == self") )
    {
        __debugbreak();
    }
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 877, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( self->ent->sentient != self->sentient
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    878,
                    0,
                    "%s",
                    "self->ent->sentient == self->sentient") )
    {
        __debugbreak();
    }
    if ( self->sentient->ent != self->ent
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    879,
                    0,
                    "%s",
                    "self->sentient->ent == self->ent") )
    {
        __debugbreak();
    }
    ent = self->ent;
    hThread = Scr_ExecEntThread(self->ent, g_animScriptTable[self->species]->init.func, 0);
    Scr_FreeThread(hThread, SCRIPTINSTANCE_SERVER);
    if ( self->stateLevel
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    888,
                    0,
                    "%s\n\t(self->stateLevel) = %i",
                    "(self->stateLevel == 0)",
                    self->stateLevel) )
    {
        __debugbreak();
    }
    if ( (self->eState[0] <= AIS_INVALID || self->eState[0] >= AIS_COUNT)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    889,
                    0,
                    "%s\n\t(self->eState[0]) = %i",
                    "(self->eState[0] > AIS_INVALID && self->eState[0] < AIS_COUNT)",
                    self->eState[0]) )
    {
        __debugbreak();
    }
    if ( !AIFuncTable[self->species][self->eState[0]].pfnStart
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    890,
                    0,
                    "%s\n\t(self->eState[0]) = %i",
                    "(AIFuncTable[self->species][self->eState[0]].pfnStart)",
                    self->eState[0]) )
    {
        __debugbreak();
    }
    AIFuncTable[self->species][self->eState[0]].pfnStart(self, AIS_INVALID);
    ent->nextthink = level.time;
    if ( !level.time )
        ent->nextthink = 50;
    ent->handler = 2;
}

void __cdecl Actor_FinishSpawningAll()
{
    actor_s *actor; // [esp+0h] [ebp-4h]

    for ( actor = Actor_FirstActor(-1); actor; actor = Actor_NextActor(actor, -1) )
        Actor_InitAnimScript(actor);
}

void __cdecl Actor_DissociateSentient(actor_s *self, sentient_s *other)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 966, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 967, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( self->sentient == other
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    968,
                    0,
                    "%s",
                    "self->sentient != other") )
    {
        __debugbreak();
    }
    SentientInfo_Clear(&self->sentientInfo[other - level.sentients]);
    if ( other == Actor_GetTargetSentient(self) )
        Sentient_SetEnemy(self->sentient, 0, 1);
}

actor_s *__fastcall Actor_FirstActor(int iTeamFlags)
{
    int i; // [esp+4h] [ebp-4h]

    if ( iTeamFlags > 15
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    989,
                    0,
                    "%s",
                    "iTeamFlags <= (1 << TEAM_NUM_TEAMS) - 1") )
    {
        __debugbreak();
    }
    for ( i = 0; i < 16; ++i )
    {
        if ( level.actors[i].inuse )
        {
            if ( !level.actors[i].sentient
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                            998,
                            0,
                            "%s",
                            "level.actors[i].sentient") )
            {
                __debugbreak();
            }
            if ( (iTeamFlags & (1 << level.actors[i].sentient->eTeam)) != 0 )
                return &level.actors[i];
        }
    }
    return 0;
}

actor_s *__fastcall Actor_NextActor(actor_s *pPrevActor, int iTeamFlags)
{
    int i; // [esp+8h] [ebp-4h]

    if ( iTeamFlags > 15
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    1014,
                    0,
                    "%s",
                    "iTeamFlags <= (1 << TEAM_NUM_TEAMS) - 1") )
    {
        __debugbreak();
    }
    if ( !pPrevActor
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1016, 0, "%s", "pPrevActor") )
    {
        __debugbreak();
    }
    if ( (pPrevActor < level.actors || pPrevActor >= &level.actors[16])
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    1017,
                    0,
                    "%s",
                    "pPrevActor >= level.actors && pPrevActor < level.actors + MAX_ACTORS") )
    {
        __debugbreak();
    }
    if ( pPrevActor != &level.actors[pPrevActor - level.actors]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    1018,
                    0,
                    "%s",
                    "pPrevActor == level.actors + (pPrevActor - level.actors)") )
    {
        __debugbreak();
    }
    for ( i = pPrevActor - level.actors + 1; i < 16; ++i )
    {
        if ( level.actors[i].inuse )
        {
            if ( !level.actors[i].sentient
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                            1027,
                            0,
                            "%s",
                            "level.actors[i].sentient") )
            {
                __debugbreak();
            }
            if ( (iTeamFlags & (1 << level.actors[i].sentient->eTeam)) != 0 )
                return &level.actors[i];
        }
    }
    return 0;
}

void __fastcall Actor_NodeClaimRevoked(actor_s *self, int invalidTime)
{
    pathnode_t *node; // [esp+8h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1045, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1046, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    node = self->sentient->pClaimedNode;
    if ( node )
    {
        if ( Actor_HasPath(self) && Actor_PointAt(self->Path.vFinalGoal, node->constant.vOrigin) )
            Actor_ClearPath(self);
        Path_MarkNodeInvalid(node, self->sentient->eTeam);
    }
}

char __fastcall Actor_KeepClaimedNode(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1082, 0, "%s", "self") )
        __debugbreak();
    if ( self->keepClaimedNode )
        return 1;
    if ( !self->keepClaimedNodeInGoal )
        return 0;
    if ( self->sentient->pClaimedNode
        && Actor_PointAtGoal(self->sentient->pClaimedNode->constant.vOrigin, &self->codeGoal) )
    {
        return 1;
    }
    self->keepClaimedNodeInGoal = 0;
    return 0;
}

void __fastcall Actor_ClearKeepClaimedNode(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1112, 0, "%s", "self") )
        __debugbreak();
    self->keepClaimedNode = 0;
    self->keepClaimedNodeInGoal = 0;
    Actor_ClearArrivalPos(self);
}

void __fastcall Actor_ClearArrivalPos(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1122, 0, "%s", "self") )
        __debugbreak();
    self->arrivalInfo.animscriptOverrideRunTo = 0;
    self->arrivalInfo.arrivalNotifyRequested = 0;
}

void __fastcall Actor_PreThink(actor_s *self)
{
    sentient_s *v1; // eax

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1179, 0, "%s", "self") )
        __debugbreak();
    if ( self->preThinkTime != level.time )
    {
        self->preThinkTime = level.time;
        //if ( SentientHandle::isDefined(&self->pFavoriteEnemy) )
        if ( self->pFavoriteEnemy.isDefined() )
        {
            //v1 = SentientHandle::sentient(&self->pFavoriteEnemy);
            v1 = self->pFavoriteEnemy.sentient();
            Actor_GetPerfectInfo(self, v1);
        }
        Actor_UpdateSight(self);
        Actor_UpdateThreat(self);
        Actor_UpdateLastEnemySightPos(self);
    }
}

void __fastcall Actor_PostThink(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1203, 0, "%s", "self") )
        __debugbreak();
    if ( self->eAnimMode == AI_ANIM_UNKNOWN
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    1204,
                    0,
                    "%s",
                    "self->eAnimMode != AI_ANIM_UNKNOWN") )
    {
        __debugbreak();
    }
    Actor_UpdateOriginAndAngles(self);
    BG_Dog_UpdateAnimationState(-1, &self->ent->s, &level_bgs.actorinfo[self - level.actors]);
    if ( self->eAnimMode != AI_ANIM_MOVE_CODE )
    {
        if ( ai_showPaths->current.integer )
            Path_DebugDraw(&self->Path, self->ent->r.currentOrigin, 1, -1);
        if ( Actor_HasPath(self) )
            Path_UpdateLookahead_NonCodeMove(&self->Path, self->sentient->oldOrigin, self->ent->r.currentOrigin);
    }
    Actor_CheckNodeClaim(self);
    Actor_CheckClearNodeClaimCloseEnt(self);
}

void __fastcall Actor_CheckClearNodeClaimCloseEnt(actor_s *self)
{
    gentity_s *v1; // eax
    const pathnode_t *pClaimedNode; // [esp+4h] [ebp-4h]

    pClaimedNode = self->sentient->pClaimedNode;
    //if ( pClaimedNode && EntHandle::isDefined(&self->pCloseEnt) )
    if ( pClaimedNode && self->pCloseEnt.isDefined() )
    {
        //v1 = EntHandle::ent(&self->pCloseEnt);
        v1 = self->pCloseEnt.ent();
        if ( Actor_PointNearNode(v1->r.currentOrigin, pClaimedNode) )
            Actor_NodeClaimRevoked(self, 1000);
    }
}

void __fastcall Actor_CheckNodeClaim(actor_s *self)
{
    float *vFinalGoal; // [esp+0h] [ebp-14h]
    pathnode_t *node; // [esp+8h] [ebp-Ch]
    const pathnode_t *pClaimedNode; // [esp+10h] [ebp-4h]

    if ( self->Physics.bIsAlive
        && !self->fixedNode
        && !self->arrivalInfo.animscriptOverrideRunTo
        && !Actor_KeepClaimedNode(self) )
    {
        if ( Actor_HasPath(self) )
            vFinalGoal = self->Path.vFinalGoal;
        else
            vFinalGoal = self->ent->r.currentOrigin;
        pClaimedNode = self->sentient->pClaimedNode;
        if ( pClaimedNode )
        {
            if ( Actor_PointNearNode(vFinalGoal, pClaimedNode) || level.time < self->iTeamMoveWaitTime )
                return;
            Sentient_ClaimNode(self->sentient, 0);
        }
        node = Sentient_NearestNode(self->sentient);
        if ( node && Path_IsCoverNode(node) && Path_IsValidClaimNode(node) && Actor_PointNearNode(vFinalGoal, node) )
        {
            if ( Path_CanClaimNode(node, self->sentient) )
                Sentient_ClaimNode(self->sentient, node);
            else
                self->sentient->bNearestNodeValid = 0;
        }
    }
}

bool __fastcall Actor_IsDying(const actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1441, 0, "%s", "self") )
        __debugbreak();
    return self->eState[self->stateLevel] == AIS_DEATH;
}

gentity_s *__fastcall Actor_GetTargetEntity(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1448, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1449, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    //if ( EntHandle::isDefined(&self->sentient->targetEnt) )
    if (self->sentient->targetEnt.isDefined())
    {
        //return EntHandle::ent(&self->sentient->targetEnt);
        return self->sentient->targetEnt.ent();
    }
    else
        return 0;
}

sentient_s *__fastcall Actor_GetTargetSentient(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1457, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1458, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }

    //if ( EntHandle::isDefined(&self->sentient->targetEnt) )
    if (self->sentient->targetEnt.isDefined())
    {
        //return EntHandle::ent(&self->sentient->targetEnt)->sentient;
        return self->sentient->targetEnt.ent()->sentient;
    }
    else
        return 0;
}

void __fastcall Actor_GetTargetPosition(actor_s *self, float *position)
{
    float *v2; // eax

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1469, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1470, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    iassert(self->sentient->targetEnt.isDefined());

    //v2 = (float *)EntHandle::ent(&self->sentient->targetEnt);
    v2 = (float *)self->sentient->targetEnt.ent();
    *position = v2[73];
    v2 += 73;
    position[1] = v2[1];
    position[2] = v2[2];
}

void __fastcall Actor_GetTargetLookPosition(actor_s *self, float *position)
{
    gentity_s *targetEnt; // [esp+8h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1486, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1487, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }

    iassert(self->sentient->targetEnt.isDefined());
    
    targetEnt = Actor_GetTargetEntity(self);
    if ( targetEnt->sentient )
        Sentient_GetEyePosition(targetEnt->sentient, position);
    else
        G_EntityCentroid(targetEnt, position);
}

gentity_s *__fastcall Actor_GetScriptTargetEntity(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1504, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1505, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    //if ( EntHandle::isDefined(&self->sentient->scriptTargetEnt) )
    if ( self->sentient->scriptTargetEnt.isDefined() )
        return self->sentient->scriptTargetEnt.ent();
    else
        return 0;
}
#define ACTOR_CALL_THINK_REPEAT_MAX 10

void __cdecl Actor_Think(gentity_s *self)
{
    const char *v1; // eax
    bool isRagdoll; // [esp+4Bh] [ebp-15h]
    actor_think_result_t result; // [esp+4Ch] [ebp-14h]
    bool originChanged; // [esp+52h] [ebp-Eh]
    bool updateProne; // [esp+53h] [ebp-Dh]
    int callThinkCounter; // [esp+54h] [ebp-Ch]
    actor_s *actor; // [esp+58h] [ebp-8h]
    bool anglesChanged; // [esp+5Fh] [ebp-1h]
    int savedregs; // [esp+60h] [ebp+0h] BYREF

    callThinkCounter = 0;
    if ( g_ai->current.enabled )
    {
        iassert(self);
        actor = self->actor;

        iassert(actor);
        iassert(actor->inuse);

        if ( Com_GetServerDObj(self->s.number) )
        {
            iassert(actor->ent);
            iassert(actor->ent == self);
            iassert(actor->sentient);
            iassert(actor->sentient->ent == self);
            iassert(self->sentient == actor->sentient);
            iassert(self->client == NULL);
            bcassert(actor->stateLevel, ARRAY_COUNT(actor->eState));
            iassert(actor->eState[actor->stateLevel] > AIS_INVALID && actor->eState[actor->stateLevel] < AIS_COUNT);
            iassert(actor->Path.iPathEndTime >= 0);
            iassert(actor->Path.iPathEndTime <= level.time + ACTOR_STOP_TIME);

            if ( actor->Physics.bIsAlive )
                Actor_UpdatePileUp(actor);

            result = ACTOR_THINK_REPEAT;
            while ( result == ACTOR_THINK_REPEAT )
            {
                result = Actor_CallThink(actor);
                callThinkCounter++;
                iassert(callThinkCounter < ACTOR_CALL_THINK_REPEAT_MAX);
            }

            originChanged = !(self->r.currentOrigin[0] == actor->sentient->oldOrigin[0]
                           && self->r.currentOrigin[1] == actor->sentient->oldOrigin[1]
                           && self->r.currentOrigin[2] == actor->sentient->oldOrigin[2]);

            Vec3Copy(self->r.currentOrigin, actor->sentient->oldOrigin);

            // set lerp.pos
            isRagdoll = Com_IsRagdollTrajectory(&self->s.lerp.pos);
            AssignToSmallerType<unsigned char>(&self->s.lerp.pos.trType, isRagdoll ? 14 : 1);
            Vec3Copy(self->r.currentOrigin, self->s.lerp.pos.trBase);

            anglesChanged = !(self->r.currentAngles[0] == self->s.lerp.apos.trBase[0]
                           && self->r.currentAngles[1] == self->s.lerp.apos.trBase[1]
                           && self->r.currentAngles[2] == self->s.lerp.apos.trBase[2]);

            updateProne = originChanged || anglesChanged;

            // set lerp.apos
            isRagdoll = Com_IsRagdollTrajectory(&self->s.lerp.apos);
            AssignToSmallerType<unsigned char>(&self->s.lerp.apos.trType, isRagdoll ? 14 : 1);
            Vec3Copy(self->r.currentAngles, self->s.lerp.apos.trBase);

            if ( originChanged )
            {
                SV_LinkEntity(self);
                Actor_PostPhysics(&actor->Physics);
            }

            if ( Sentient_NearestNodeDirty(actor->sentient, originChanged) )
                Sentient_InvalidateNearestNode(actor->sentient);

            Sentient_BanNearNodes(actor->sentient);

            if ( !BG_ActorIsProne(&actor->ProneInfo, level.time) )
                actor->Physics.prone = 0;

            Actor_UpdateLookAt(actor);

            if ( actor->delayedDeath && !Actor_InScriptedState(actor) )
                G_Damage(self, 0, 0, 0, self->r.currentOrigin, self->health + 1, 0, 0, 0xFFFFFFFF, HITLOC_HEAD, 0, 0, 0);

            if ( self->actor->Physics.bIsAlive && !self->actor->ignoreTriggers )
                G_DoTouchTriggers(self);

            if ( result != ACTOR_THINK_MOVE_TO_BODY_QUEUE )
                Actor_CheckNotify(actor);

            Actor_UpdateActorInfo(self);
            if ( g_debugLocDamage->current.integer == 1 && SV_DObjExists(self) )
            {
                int partBits[5]; // [esp+34h] [ebp-2Ch] BYREF
                memset(partBits, 255, sizeof(partBits));
                G_DObjCalcPose(self, partBits);
                SV_XModelDebugBoxes(self, colorWhite, 0, 0);
            }
            if ( result != ACTOR_THINK_MOVE_TO_BODY_QUEUE || Actor_BecomeCorpse(self) )
                self->nextthink = level.time + 50;
            else
                G_FreeEntity(self);
        }
        else
        {
            Com_Printf(18, "^3Deleting AI without a model.\n");
            Scr_Notify(self, scr_const.death, 0);
            G_FreeEntity(self);
        }
    }
    else //     if ( g_ai->current.enabled )
    {
        self->nextthink = level.time + 50;
    }
}

actor_think_result_t __fastcall Actor_CallThink(actor_s *self)
{
    const char *v1; // eax
    bool v3; // [esp+10h] [ebp-40h]
    float *currentOrigin; // [esp+28h] [ebp-28h]
    actor_think_result_t eThinkResult; // [esp+2Ch] [ebp-24h]
    gjkcc_input_t gjkcc_in; // [esp+30h] [ebp-20h] BYREF
    int savedregs; // [esp+50h] [ebp+0h] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1349, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1350, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( self->stateLevel >= 6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    1351,
                    0,
                    "self->stateLevel doesn't index ARRAY_COUNT( self->eState )\n\t%i not in [0, %i)",
                    self->stateLevel,
                    6) )
    {
        __debugbreak();
    }
    if ( !AIFuncTable[self->species][self->eState[self->stateLevel]].pfnThink
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    1352,
                    0,
                    "%s\n\t(self->eState[self->stateLevel]) = %i",
                    "(AIFuncTable[self->species][self->eState[self->stateLevel]].pfnThink)",
                    self->eState[self->stateLevel]) )
    {
        __debugbreak();
    }
    //BLOPS_NULLSUB();
    if ( self->transitionCount )
    {
        Actor_ThinkStateTransitions(self);
        if ( self->transitionCount )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                            1362,
                            0,
                            "%s",
                            "self->transitionCount == 0") )
                __debugbreak();
        }
    }
    Actor_UpdateCloseEnt(self);
    CM_CheckForTraps(self->ent);
    currentOrigin = self->ent->r.currentOrigin;
    self->Physics.vOrigin[0] = *currentOrigin;
    self->Physics.vOrigin[1] = currentOrigin[1];
    self->Physics.vOrigin[2] = currentOrigin[2];
    setup_gjkcc_input(&self->Physics, &gjkcc_in);
    self->Physics.m_gjkcc_input = &gjkcc_in;
    gjkcc_prolog(&gjkcc_in, self->Physics.vOrigin);
    //eThinkResult = ((int (__thiscall *)(actor_s *))AIFuncTable[self->species][self->eState[self->stateLevel]].pfnThink)(self);
    eThinkResult = (actor_think_result_t)((int (__thiscall *)(actor_s *))AIFuncTable[self->species][self->eState[self->stateLevel]].pfnThink)(self);
    gjkcc_epilog(&gjkcc_in, self->Physics.vOrigin);
    self->Physics.m_gjkcc_input = 0;
    if ( (unsigned int)eThinkResult > ACTOR_THINK_MOVE_TO_BODY_QUEUE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    1377,
                    0,
                    "%s",
                    "eThinkResult == ACTOR_THINK || eThinkResult == ACTOR_THINK_REPEAT || eThinkResult == ACTOR_THINK_MOVE_TO_BODY_QUEUE") )
    {
        __debugbreak();
    }
    if ( self->Path.wPathLen > 1
        && *((float *)&self->Physics.proximity_data.prims[195].tree + 7 * self->Path.wPathLen) < self->Path.fCurrLength )
    {
        v1 = va(
                     "self->Path.fCurrLength: %g, self->Path.pts[self->Path.wPathLen - 2].fOrigLength: %g",
                     self->Path.fCurrLength,
                     *((float *)&self->Physics.proximity_data.prims[195].tree + 7 * self->Path.wPathLen));
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                        1379,
                        0,
                        "%s\n\t%s",
                        "self->Path.wPathLen <= 1 || self->Path.fCurrLength <= self->Path.pts[self->Path.wPathLen - 2].fOrigLength",
                        v1) )
            __debugbreak();
    }
    if ( Path_HasNegotiationNode(&self->Path)
        && (self->Path.pts[self->Path.wNegotiationStartNode].iNodeNum < 0
         || *((int *)&self->Physics.proximity_data.prims[199].tree + 7 * self->Path.wNegotiationStartNode) < 0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    1380,
                    0,
                    "%s",
                    "!Path_HasNegotiationNode( &self->Path ) || (self->Path.pts[self->Path.wNegotiationStartNode].iNodeNum >= 0 && "
                    "self->Path.pts[self->Path.wNegotiationStartNode - 1].iNodeNum >= 0)") )
    {
        __debugbreak();
    }
    if ( self->Path.wPathLen && self->Path.wNegotiationStartNode == self->Path.wPathLen - 1 )
    {
        v3 = self->Path.pts[self->Path.wNegotiationStartNode].vOrigPoint[0] == self->Path.vCurrPoint[0]
            && self->Path.pts[self->Path.wNegotiationStartNode].vOrigPoint[1] == self->Path.vCurrPoint[1]
            && self->Path.pts[self->Path.wNegotiationStartNode].vOrigPoint[2] == self->Path.vCurrPoint[2];
        if ( !v3
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                        1381,
                        0,
                        "%s",
                        "!self->Path.wPathLen || !Path_AtEndOrNegotiation( &self->Path ) || Vec3Compare( self->Path.pts[self->Path.wN"
                        "egotiationStartNode].vOrigPoint, self->Path.vCurrPoint )") )
        {
            __debugbreak();
        }
    }
    if ( self->Path.wPathLen
        && self->Path.lookaheadNextNode >= self->Path.wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    1382,
                    0,
                    "%s",
                    "!self->Path.wPathLen || (self->Path.lookaheadNextNode < self->Path.wPathLen)") )
    {
        __debugbreak();
    }
    if ( self->Path.wPathLen
        && self->Path.fLookaheadDistToNextNode != 0.0
        && self->Path.lookaheadNextNode >= self->Path.wPathLen - 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    1383,
                    0,
                    "%s",
                    "!self->Path.wPathLen || !self->Path.fLookaheadDistToNextNode || (self->Path.lookaheadNextNode < self->Path.wPathLen - 1)") )
    {
        __debugbreak();
    }
    //BLOPS_NULLSUB();
    return eThinkResult;
}

void __fastcall Actor_UpdateCloseEnt(actor_s *self)
{
    gentity_s *v1; // eax
    gentity_s *v2; // eax
    float *currentOrigin; // [esp+8h] [ebp-28h]
    float v5; // [esp+10h] [ebp-20h]
    float v6; // [esp+14h] [ebp-1Ch]
    float distSqr; // [esp+18h] [ebp-18h]
    actor_s *closeActor; // [esp+1Ch] [ebp-14h]
    float delta[2]; // [esp+20h] [ebp-10h] BYREF
    int clientNum; // [esp+28h] [ebp-8h]
    gentity_s *player; // [esp+2Ch] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1277, 0, "%s", "self") )
        __debugbreak();
    //if ( EntHandle::isDefined(&self->pCloseEnt) )
    if ( self->pCloseEnt.isDefined() )
    {
        closeActor =    self->pCloseEnt.ent()->actor;
        currentOrigin = self->pCloseEnt.ent()->r.currentOrigin;
        v5 = *currentOrigin - self->ent->r.currentOrigin[0];
        v6 = currentOrigin[1] - self->ent->r.currentOrigin[1];
        if ( (float)((float)(v5 * v5) + (float)(v6 * v6)) < 1406.25 )
        {
            if ( !closeActor || closeActor->pPileUpEnt != self->ent )
                goto LABEL_14;
        }
        else if ( closeActor
                     && closeActor->pCloseEnt.isDefined()
                     && closeActor->pCloseEnt.ent() == self->ent)
        {
            closeActor->pCloseEnt.setEnt(0);
        }
        //EntHandle::setEnt(&self->pCloseEnt, 0);
        self->pCloseEnt.setEnt(0);
    }
LABEL_14:
    for ( clientNum = 0; clientNum < com_maxclients->current.integer; ++clientNum )
    {
        player = G_GetPlayer(clientNum);

        //if ( player->sentient && EntHandle::isDefined(&self->pCloseEnt) && EntHandle::ent(&self->pCloseEnt) != player )
        if ( player->sentient && self->pCloseEnt.isDefined() && self->pCloseEnt.ent() != player)
            Actor_UpdatePlayerPush(self, player);
    }
    //if ( EntHandle::isDefined(&self->pCloseEnt) )
    if ( self->pCloseEnt.isDefined() )
    {
        iassert(self->pCloseEnt.isDefined());
        
        v1 = self->pCloseEnt.ent();
        distSqr = Vec2DistanceSq(self->ent->r.currentOrigin, v1->r.currentOrigin);
        if ( distSqr >= 900.0 && !self->pPileUpActor && self->eAnimMode == AI_ANIM_MOVE_CODE && Actor_HasPath(self) )
        {
            v2 = self->pCloseEnt.ent();
            Vec2Sub(self->ent->r.currentOrigin, v2->r.currentOrigin, delta);
            if ( (float)((float)(self->Path.lookaheadDir[0] * delta[0]) + (float)(self->Path.lookaheadDir[1] * delta[1])) > 0.0 )
                self->pCloseEnt.setEnt(0);
        }
    }
}

void Vec2Sub(const vec2r a, const vec2r b, vec2r diff)
{
    diff[0] = a[0] - b[0];
    diff[1] = a[1] - b[1];
}

double __cdecl Vec2DistanceSq(const float *v0, const float *v1)
{
    float dir_4; // [esp+4h] [ebp-4h]

    dir_4 = v1[1] - v0[1];
    return (float)(*v1 - *v0) * (float)(*v1 - *v0) + dir_4 * dir_4;
}

void __fastcall Actor_UpdatePlayerPush(actor_s *self, gentity_s *player)
{
    float *velocity; // [esp+8h] [ebp-44h]
    float mins[3]; // [esp+24h] [ebp-28h] BYREF
    int playerTeamFlags; // [esp+30h] [ebp-1Ch]
    float offset[3]; // [esp+34h] [ebp-18h]
    float maxs[3]; // [esp+40h] [ebp-Ch] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1231, 0, "%s", "self") )
        __debugbreak();
    if ( !self->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1232, 0, "%s", "self->ent") )
    {
        __debugbreak();
    }
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1233, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( !player && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1235, 0, "%s", "player") )
        __debugbreak();
    if ( !player->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1236, 0, "%s", "player->client") )
    {
        __debugbreak();
    }
    if ( !player->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1237, 0, "%s", "player->sentient") )
    {
        __debugbreak();
    }
    if (!self->bDontAvoidPlayer
        && (self->Physics.iTraceMask & 0x2000000) != 0
        && self->eState[self->stateLevel] != AIS_TURRET)
    {
        playerTeamFlags = ~(1 << Sentient_EnemyTeam(player->sentient->eTeam));
        if ( (playerTeamFlags & (1 << self->sentient->eTeam)) != 0 )
        {
            velocity = player->client->ps.velocity;
            offset[0] = (float)(0.1 * *velocity) + player->r.currentOrigin[0];
            offset[1] = (float)(0.1 * velocity[1]) + player->r.currentOrigin[1];
            offset[2] = (float)(0.1 * velocity[2]) + player->r.currentOrigin[2];
            mins[0] = offset[0] + -15.0;
            mins[1] = offset[1] + -15.0;
            mins[2] = offset[2] + 0.0;
            maxs[0] = offset[0] + 15.0;
            maxs[1] = offset[1] + 15.0;
            maxs[2] = offset[2] + 70.0;
            if ( SV_EntityContact(mins, maxs, self->ent) )
            {
                if ( Actor_AtClaimNode(self) )
                    Sentient_StealClaimNode(player->sentient, self->sentient);
                self->pCloseEnt.setEnt(player);
            }
        }
    }
}

void __fastcall Actor_CheckNotify(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1419, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1420, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( !self->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1421, 0, "%s", "self->ent") )
    {
        __debugbreak();
    }
    if ( Actor_CheckGoalNotify(self) )
        Scr_Notify(self->ent, scr_const.goal, 0);
    if ( self->arrivalInfo.animscriptOverrideRunTo )
    {
        if ( Actor_PointAt(self->ent->r.currentOrigin, self->arrivalInfo.animscriptOverrideRunToPos) )
        {
            Scr_Notify(self->ent, scr_const.runto_arrived, 0);
            self->arrivalInfo.animscriptOverrideRunTo = 0;
        }
    }
}

bool __fastcall Actor_CheckGoalNotify(actor_s *self)
{
    pathnode_t *node; // [esp+18h] [ebp-8h]
    float fAngleDelta; // [esp+1Ch] [ebp-4h]

    if ( !Actor_IsAtScriptGoal(self) )
        return 0;
    if ( self->ScriptOrient.eMode )
        return 1;
    if ( self->CodeOrient.eMode != AI_ORIENT_TO_GOAL )
        return 1;
    node = self->sentient->pClaimedNode;
    if ( !node || Path_IsCoverNode(node) || !Path_IsValidClaimNode(node) )
        return 1;
    fAngleDelta = AngleNormalize180(node->constant.fAngle - self->ent->r.currentAngles[1]);
    return fAngleDelta >= (-(ACTOR_GOAL_ANGLE_TOLERANCE)) && fAngleDelta <= 20.0;
}

int __cdecl Actor_UpdateActorInfo(gentity_s *ent)
{
    actor_s *actor; // [esp+4h] [ebp-8h]
    unsigned int actorNum; // [esp+8h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1516, 0, "%s", "ent") )
        __debugbreak();
    actor = ent->actor;
    if ( !actor && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1519, 0, "%s", "actor") )
        __debugbreak();
    if ( !actor->inuse
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1520, 0, "%s", "actor->inuse") )
    {
        __debugbreak();
    }
    actorNum = G_GetActorIndex(actor);
    if ( actorNum >= 0x10
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    1523,
                    0,
                    "actorNum doesn't index MAX_ACTORS\n\t%i not in [0, %i)",
                    actorNum,
                    16) )
    {
        __debugbreak();
    }
    ent->s.lerp.u.actor.actorNum = actorNum;
    if ( ent->sentient->eTeam != TEAM_SPECTATOR )
        ent->s.lerp.u.actor.team = ent->sentient->eTeam;
    return 0;
}

bool __cdecl Actor_InScriptedState(const actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1729, 0, "%s", "self") )
        __debugbreak();
    return Actor_IsStateOnStack(self, AIS_SCRIPTEDANIM) || Actor_IsStateOnStack(self, AIS_NEGOTIATION) != 0;
}

void __cdecl Actor_Touch(gentity_s *self, gentity_s *other, int __formal)
{
    actor_s *actor; // [esp+4h] [ebp-4h]

    actor = self->actor;
    if ( !actor && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1746, 0, "%s", "actor") )
        __debugbreak();
    if ( !actor->inuse
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1747, 0, "%s", "actor->inuse") )
    {
        __debugbreak();
    }
    if ( actor->stateLevel >= 6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    1756,
                    0,
                    "actor->stateLevel doesn't index ARRAY_COUNT( actor->eState )\n\t%i not in [0, %i)",
                    actor->stateLevel,
                    6) )
    {
        __debugbreak();
    }
    if ( (actor->eState[actor->stateLevel] <= AIS_INVALID || actor->eState[actor->stateLevel] >= AIS_COUNT)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    1757,
                    0,
                    "%s\n\t(actor->eState[actor->stateLevel]) = %i",
                    "(actor->eState[actor->stateLevel] > AIS_INVALID && actor->eState[actor->stateLevel] < AIS_COUNT)",
                    actor->eState[actor->stateLevel]) )
    {
        __debugbreak();
    }
    if ( !AIFuncTable[actor->species][actor->eState[actor->stateLevel]].pfnTouch
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    1758,
                    0,
                    "%s\n\t(actor->eState[actor->stateLevel]) = %i",
                    "(AIFuncTable[actor->species][actor->eState[actor->stateLevel]].pfnTouch)",
                    actor->eState[actor->stateLevel]) )
    {
        __debugbreak();
    }
    AIFuncTable[actor->species][actor->eState[actor->stateLevel]].pfnTouch(actor, other);
}

void __cdecl Actor_Pain(
                gentity_s *self,
                gentity_s *pAttacker,
                int iDamage,
                const float *vPoint,
                int iMod,
                const float *vDir,
                hitLocation_t hitLoc,
                int weaponIdx)
{
    unsigned __int16 HitLocationString; // ax
    char *v9; // eax
    float angle; // [esp+0h] [ebp-20h]
    actor_s *actor; // [esp+1Ch] [ebp-4h]

    actor = self->actor;
    if ( !actor && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1775, 0, "%s", "actor") )
        __debugbreak();
    if ( !actor->inuse
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1776, 0, "%s", "actor->inuse") )
    {
        __debugbreak();
    }
    if ( !vDir && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1777, 0, "%s", "vDir") )
        __debugbreak();

    nanassertvec3(vDir);

    if ( actor->stateLevel >= 6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    1797,
                    0,
                    "actor->stateLevel doesn't index ARRAY_COUNT( actor->eState )\n\t%i not in [0, %i)",
                    actor->stateLevel,
                    6) )
    {
        __debugbreak();
    }
    if ( (actor->eState[actor->stateLevel] <= AIS_INVALID || actor->eState[actor->stateLevel] >= AIS_COUNT)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    1798,
                    0,
                    "%s\n\t(actor->eState[actor->stateLevel]) = %i",
                    "(actor->eState[actor->stateLevel] > AIS_INVALID && actor->eState[actor->stateLevel] < AIS_COUNT)",
                    actor->eState[actor->stateLevel]) )
    {
        __debugbreak();
    }
    actor->iDamageTaken = iDamage;
    angle = vectoyaw(vDir) - self->r.currentAngles[1];
    actor->iDamageYaw = (int)AngleNormalize180(angle);
    actor->damageDir[0] = *vDir;
    actor->damageDir[1] = vDir[1];
    actor->damageDir[2] = vDir[2];
    HitLocationString = G_GetHitLocationString(hitLoc);
    Scr_SetString(&actor->damageHitLoc, HitLocationString, SCRIPTINSTANCE_SERVER);
    Scr_SetString(&actor->damageMod, *modNames[iMod], SCRIPTINSTANCE_SERVER);
    if ( pAttacker )
    {
        if ( !BG_WeaponName(weaponIdx)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                        1810,
                        0,
                        "%s",
                        "BG_WeaponName( weaponIdx )") )
        {
            __debugbreak();
        }
        v9 = (char *)BG_WeaponName(weaponIdx);
        Scr_SetStringFromCharString(&actor->damageWeapon, v9, SCRIPTINSTANCE_SERVER);
    }
    if ( !AIFuncTable[actor->species][actor->eState[actor->stateLevel]].pfnPain
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    1815,
                    0,
                    "%s",
                    "AIFuncTable[actor->species][actor->eState[actor->stateLevel]].pfnPain") )
    {
        __debugbreak();
    }
    AIFuncTable[actor->species][actor->eState[actor->stateLevel]].pfnPain(
        actor,
        pAttacker,
        iDamage,
        vPoint,
        iMod,
        vDir,
        hitLoc);
    self->sentient->lastAttacker.setEnt(pAttacker);
    if ( pAttacker->sentient )
        Actor_WasAttackedBy(actor, pAttacker->sentient);
    if ( !actor->sentient->syncedMeleeEnt.isDefined() && actor->allowPain)
    {
        if ( Actor_PushState(actor, AIS_PAIN) )
            Actor_KillAnimScript(actor);
    }
}

void __cdecl Actor_Die(
                gentity_s *self,
                gentity_s *pInflictor,
                gentity_s *pAttacker,
                int iDamage,
                int iMod,
                const int iWeapon,
                const float *vDir,
                const hitLocation_t hitLoc,
                int timeOffset)
{
    unsigned __int16 HitLocationString; // ax
    char *v10; // eax
    float angle; // [esp+0h] [ebp-1Ch]
    actor_s *actor; // [esp+18h] [ebp-4h]

    actor = self->actor;
    if ( !actor && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1848, 0, "%s", "actor") )
        __debugbreak();
    if ( !actor->inuse
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1849, 0, "%s", "actor->inuse") )
    {
        __debugbreak();
    }
    if ( !vDir && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1850, 0, "%s", "vDir") )
        __debugbreak();

    nanassertvec3(vDir);

    if ( !actor->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1864, 0, "%s", "actor->ent") )
    {
        __debugbreak();
    }
    if ( actor->ent->health > 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    1865,
                    0,
                    "%s\n\t(actor->ent->health) = %i",
                    "(actor->ent->health <= 0)",
                    actor->ent->health) )
    {
        __debugbreak();
    }
    if ( actor->stateLevel >= 6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    1866,
                    0,
                    "actor->stateLevel doesn't index ARRAY_COUNT( actor->eState )\n\t%i not in [0, %i)",
                    actor->stateLevel,
                    6) )
    {
        __debugbreak();
    }
    if ( (actor->eState[actor->stateLevel] <= AIS_INVALID || actor->eState[actor->stateLevel] >= AIS_COUNT)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    1867,
                    0,
                    "%s\n\t(actor->eState[actor->stateLevel]) = %i",
                    "(actor->eState[actor->stateLevel] > AIS_INVALID && actor->eState[actor->stateLevel] < AIS_COUNT)",
                    actor->eState[actor->stateLevel]) )
    {
        __debugbreak();
    }
    if ( actor->eState[actor->stateLevel] != AIS_DEATH )
    {
        Scr_ActorKilled(self, pInflictor, pAttacker, iDamage, iMod, iWeapon, (float*)vDir, hitLoc, timeOffset);
        actor->iDamageTaken = iDamage;
        actor->damageDir[0] = *vDir;
        actor->damageDir[1] = vDir[1];
        actor->damageDir[2] = vDir[2];
        angle = vectoyaw(vDir) - self->r.currentAngles[1];
        actor->iDamageYaw = (int)AngleNormalize180(angle);
        HitLocationString = G_GetHitLocationString(hitLoc);
        Scr_SetString(&actor->damageHitLoc, HitLocationString, SCRIPTINSTANCE_SERVER);
        Scr_SetString(&actor->damageMod, *modNames[iMod], SCRIPTINSTANCE_SERVER);
        if ( pInflictor )
        {
            if ( !BG_WeaponName(pInflictor->s.weapon)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                            1887,
                            0,
                            "%s",
                            "BG_WeaponName( pInflictor->s.weapon )") )
            {
                __debugbreak();
            }
            v10 = (char *)BG_WeaponName(pInflictor->s.weapon);
            Scr_SetStringFromCharString(&actor->damageWeapon, v10, SCRIPTINSTANCE_SERVER);
        }
        Actor_ForceState(actor, AIS_DEATH);
        actor->Physics.bIsAlive = 0;
        self->sentient->lastAttacker.setEnt(pAttacker);
        Scr_SetString(&self->targetname, 0, SCRIPTINSTANCE_SERVER);
        Actor_KillAnimScript(actor);
    }
}

bool __cdecl usingCodeGoal(actor_s *actor)
{
    int v2; // [esp+0h] [ebp-10h]
    bool v3; // [esp+4h] [ebp-Ch]

    v2 = 0;
    if ( actor->codeGoalSrc )
    {
        if ( actor->scriptGoalEnt.isDefined() )
            return 1;
        v3 = actor->scriptGoal.pos[0] == actor->codeGoal.pos[0]
            && actor->scriptGoal.pos[1] == actor->codeGoal.pos[1]
            && actor->scriptGoal.pos[2] == actor->codeGoal.pos[2];
        if ( !v3 || actor->scriptGoal.radius != actor->codeGoal.radius )
            return 1;
    }
    return v2;
}

bool __cdecl isNodeInRegion(pathnode_t *node, gentity_s *volume)
{
    if ( !node && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1971, 0, "%s", "node") )
        __debugbreak();
    if ( !volume && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1972, 0, "%s", "volume") )
        __debugbreak();
    return SV_EntityContact(node->constant.vOrigin, node->constant.vOrigin, volume);
}

void __fastcall Actor_DebugDrawNodesInVolume(actor_s *self)
{
    pathnode_t *node; // [esp+18h] [ebp-181Ch]
    gentity_s *volume; // [esp+1Ch] [ebp-1818h]
    int i; // [esp+20h] [ebp-1814h]
    pathsort_t nodes[512]; // [esp+24h] [ebp-1810h] BYREF
    float pos[3]; // [esp+1824h] [ebp-10h] BYREF
    int v7; // [esp+1830h] [ebp-4h]

    CL_GetDebugViewPos(pos);
    volume = self->codeGoal.volume;
    if ( !volume && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 1992, 0, "%s", "volume") )
        __debugbreak();
    v7 = Path_NodesInCylinder(self->codeGoal.pos, self->codeGoal.radius, self->codeGoal.height, nodes, 512, 540668);
    for ( i = 0; i < v7; ++i )
    {
        node = nodes[i].node;
        if ( isNodeInRegion(node, volume) )
            Path_DrawDebugNode(pos, node);
    }
}

void __cdecl Actor_EntInfo(gentity_s *self, float *source)
{
    gentity_s *v2; // eax
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
    gentity_s *v17; // eax
    char *v18; // eax
    char *v19; // eax
    char *v20; // eax
    char *v21; // [esp+14h] [ebp-998h]
    const char *v22; // [esp+1Ch] [ebp-990h]
    const char *v23; // [esp+20h] [ebp-98Ch]
    unsigned int missCount; // [esp+24h] [ebp-988h]
    int number; // [esp+28h] [ebp-984h]
    const char *v26; // [esp+2Ch] [ebp-980h]
    const char *v27; // [esp+30h] [ebp-97Ch]
    const float *v28; // [esp+34h] [ebp-978h]
    const char *v29; // [esp+38h] [ebp-974h]
    const char *v30; // [esp+3Ch] [ebp-970h]
    float v31; // [esp+70h] [ebp-93Ch]
    float v[3]; // [esp+90h] [ebp-91Ch] BYREF
    float myPos[3]; // [esp+9Ch] [ebp-910h] BYREF
    float enemyPos[3]; // [esp+A8h] [ebp-904h]
    float range; // [esp+B4h] [ebp-8F8h]
    float pos[3]; // [esp+B8h] [ebp-8F4h] BYREF
    float fact; // [esp+C4h] [ebp-8E8h]
    int cycleTime; // [esp+C8h] [ebp-8E4h]
    float timingColor[3]; // [esp+CCh] [ebp-8E0h] BYREF
    float v40; // [esp+D8h] [ebp-8D4h]
    float vAngles[3]; // [esp+DCh] [ebp-8D0h] BYREF
    float fBaseYaw; // [esp+E8h] [ebp-8C4h]
    float vColor[4]; // [esp+ECh] [ebp-8C0h] BYREF
    float vDelta[3]; // [esp+FCh] [ebp-8B0h] BYREF
    float vDebugTargetPosition[3]; // [esp+108h] [ebp-8A4h] BYREF
    float goalYaw[3]; // [esp+114h] [ebp-898h] BYREF
    float rawYaw; // [esp+120h] [ebp-88Ch]
    vis_cache_t *pCached; // [esp+124h] [ebp-888h]
    unsigned int i; // [esp+128h] [ebp-884h]
    char header[32]; // [esp+12Ch] [ebp-880h] BYREF
    char buffer[2048]; // [esp+14Ch] [ebp-860h] BYREF
    char *pszText; // [esp+950h] [ebp-5Ch]
    DObj *obj; // [esp+954h] [ebp-58h]
    bool drawLines; // [esp+95Bh] [ebp-51h]
    float delta[3]; // [esp+95Ch] [ebp-50h] BYREF
    float xyz[3]; // [esp+968h] [ebp-44h] BYREF
    float dist; // [esp+974h] [ebp-38h]
    gentity_s *target; // [esp+978h] [ebp-34h]
    sentient_s *enemy; // [esp+97Ch] [ebp-30h]
    float vEnemyEye[3]; // [esp+980h] [ebp-2Ch] BYREF
    bool drawGoalLineRadius; // [esp+98Fh] [ebp-1Dh]
    float forward[3]; // [esp+990h] [ebp-1Ch] BYREF
    int textMode; // [esp+99Ch] [ebp-10h]
    actor_s *actor; // [esp+9A0h] [ebp-Ch]
    const float *color; // [esp+9A4h] [ebp-8h]
    float infoScale; // [esp+9A8h] [ebp-4h]

    drawLines = 1;
    drawGoalLineRadius = 0;
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 2052, 0, "%s", "self") )
        __debugbreak();
    if ( SV_DObjExists(self) )
    {
        actor = self->actor;
        if ( !actor && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 2057, 0, "%s", "actor") )
            __debugbreak();
        delta[0] = *source - self->r.currentOrigin[0];
        delta[1] = source[1] - self->r.currentOrigin[1];
        delta[2] = source[2] - self->r.currentOrigin[2];
        dist = Vec3Length(delta);
        if ( g_entinfo_maxdist->current.value <= 0.0 || dist <= g_entinfo_maxdist->current.value )
        {
            infoScale = G_GetEntInfoScale();
            infoScale = (float)(dist * 0.0026041667) * infoScale;
            Sentient_GetDebugEyePosition(actor->ent->sentient, xyz);
            if ( g_entinfo->current.integer == 7 )
            {
                if ( ai_debugEntIndex->current.integer == self->s.number )
                {
                    obj = Com_GetServerDObj(self->s.number);
                    if ( obj )
                    {
                        sprintf(header, "Entity %d\n\n", self->s.number);
                        DObjDisplayAnimToBuffer(obj, header, buffer, 2048);
                        for ( pszText = strtok(buffer, "\n"); pszText; pszText = strtok(0, "\n") )
                        {
                            for ( i = 0; i < strlen(pszText); ++i )
                            {
                                if ( pszText[i] == 94 )
                                {
                                    pszText[i] = 32;
                                    pszText[i + 1] = 32;
                                    i += 2;
                                }
                            }
                            xyz[2] = xyz[2] - (float)(infoScale * 7.0);
                            G_AddDebugString(xyz, colorWhite, infoScale * 0.60000002, pszText, 0);
                        }
                    }
                }
            }
            else
            {
                if ( g_entinfo->current.integer == 3 )
                    drawLines = 0;
                if ( g_entinfo->current.integer == 4 || g_entinfo->current.integer == 5 )
                {
                    drawLines = 0;
                    drawGoalLineRadius = 1;
                }
                actor = self->actor;
                if ( !actor
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 2116, 0, "%s", "actor") )
                {
                    __debugbreak();
                }
                target = Actor_GetTargetEntity(actor);
                enemy = Actor_GetTargetSentient(actor);
                Sentient_GetDebugEyePosition(actor->ent->sentient, xyz);
                if ( drawLines )
                {
                    if ( enemy )
                    {
                        pCached = &actor->sentientInfo[enemy - level.sentients].VisCache;
                        if ( pCached->bVisible )
                        {
                            if ( level.time - pCached->iLastUpdateTime > 250 )
                                color = colorYellow;
                            else
                                color = colorGreen;
                        }
                        else
                        {
                            color = colorRed;
                        }
                        Sentient_GetDebugEyePosition(enemy, vEnemyEye);
                        G_DebugLine(xyz, vEnemyEye, color, 1);
                    }
                    else if ( target )
                    {
                        G_DebugLine(xyz, target->r.currentOrigin, colorBlue, 1);
                    }
                }
                if ( g_entinfo->current.integer != 3 )
                    CG_DebugBox(self->r.currentOrigin, self->r.mins, self->r.maxs, self->r.currentAngles[1], colorMagenta, 1, 0);
                forward[0] = actor->eyeInfo.dir[0];
                forward[1] = actor->eyeInfo.dir[1];
                forward[2] = actor->eyeInfo.dir[2];
                forward[0] = (float)(48.0 * forward[0]) + xyz[0];
                forward[1] = (float)(48.0 * forward[1]) + xyz[1];
                forward[2] = (float)(48.0 * forward[2]) + xyz[2];
                if ( drawLines )
                    G_DebugLine(xyz, forward, colorBlue, 1);
                if ( actor->ScriptOrient.eMode )
                    rawYaw = actor->ScriptOrient.fDesiredBodyYaw;
                else
                    rawYaw = actor->CodeOrient.fDesiredBodyYaw;
                goalYaw[0] = (float)(48.0 * actor->Path.lookaheadDir[0]) + xyz[0];
                goalYaw[1] = (float)(48.0 * actor->Path.lookaheadDir[1]) + xyz[1];
                goalYaw[2] = xyz[2];
                G_DebugLine(xyz, goalYaw, colorMagenta, 1);
                goalYaw[0] = (float)(48.0 * actor->prevMoveDir[0]) + xyz[0];
                goalYaw[1] = (float)(48.0 * actor->prevMoveDir[1]) + xyz[1];
                goalYaw[2] = xyz[2];
                G_DebugLine(xyz, goalYaw, colorOrange, 1);
                if ( drawLines && (actor->lookAtInfo.bDoLookAt || actor->lookAtInfo.fLookAtTurnAngle != 0.0) )
                {
                    fBaseYaw = self->r.currentAngles[1];
                    vDelta[0] = actor->lookAtInfo.vLookAtPos[0] - xyz[0];
                    vDelta[1] = actor->lookAtInfo.vLookAtPos[1] - xyz[1];
                    vDelta[2] = actor->lookAtInfo.vLookAtPos[2] - xyz[2];
                    vectoangles(vDelta, vAngles);
                    vColor[0] = 1.0f;
                    vColor[1] = 1.0f;
                    vColor[2] = 0.25f;
                    vColor[3] = 0.5f;
                    vDebugTargetPosition[0] = actor->lookAtInfo.vLookAtPos[0];
                    vDebugTargetPosition[1] = actor->lookAtInfo.vLookAtPos[1];
                    vDebugTargetPosition[2] = actor->lookAtInfo.vLookAtPos[2];
                    vDebugTargetPosition[2] = vDebugTargetPosition[2] - 2.0;
                    G_DebugLine(xyz, vDebugTargetPosition, vColor, 1);
                    vColor[3] = 0.75f;
                    v31 = AngleNormalize360(fBaseYaw - actor->lookAtInfo.fLookAtTurnAngle);
                    vDebugTargetPosition[0] = 0.0f;
                    vDebugTargetPosition[1] = v31;
                    vDebugTargetPosition[2] = 0.0f;
                    AngleVectors(vDebugTargetPosition, forward, 0, 0);
                    vDebugTargetPosition[0] = (float)(20.0 * forward[0]) + xyz[0];
                    vDebugTargetPosition[1] = (float)(20.0 * forward[1]) + xyz[1];
                    vDebugTargetPosition[2] = (float)(20.0 * forward[2]) + xyz[2];
                    G_DebugLine(xyz, vDebugTargetPosition, vColor, 1);
                    vColor[0] = 0.5f;
                    vColor[1] = 0.5f;
                    vColor[2] = 0.0f;
                    vColor[3] = 0.75f;
                    vDebugTargetPosition[0] = 0.0f;
                    vDebugTargetPosition[1] = vAngles[1];
                    vDebugTargetPosition[2] = 0.0f;
                    AngleVectors(vDebugTargetPosition, forward, 0, 0);
                    vDebugTargetPosition[0] = (float)(20.0 * forward[0]) + xyz[0];
                    vDebugTargetPosition[1] = (float)(20.0 * forward[1]) + xyz[1];
                    vDebugTargetPosition[2] = (float)(20.0 * forward[2]) + xyz[2];
                    G_DebugLine(xyz, vDebugTargetPosition, vColor, 1);
                    vDebugTargetPosition[0] = 0.0f;
                    vDebugTargetPosition[1] = fBaseYaw;
                    vDebugTargetPosition[2] = 0.0f;
                    AngleVectors(vDebugTargetPosition, forward, 0, 0);
                    vDebugTargetPosition[0] = (float)(20.0 * forward[0]) + xyz[0];
                    vDebugTargetPosition[1] = (float)(20.0 * forward[1]) + xyz[1];
                    vDebugTargetPosition[2] = (float)(20.0 * forward[2]) + xyz[2];
                    G_DebugLine(xyz, vDebugTargetPosition, vColor, 1);
                    if ( AngleNormalize180(fBaseYaw - vAngles[1]) <= 0.0 )
                        G_DebugArc(xyz, 16.0, fBaseYaw, vAngles[1], vColor, 1, 0);
                    else
                        G_DebugArc(xyz, 16.0, vAngles[1], fBaseYaw, vColor, 1, 0);
                }
                if ( drawLines || drawGoalLineRadius )
                {
                    cycleTime = 1000;
                    fact = 0.0f;
                    v40 = 1.0f;

                    static int endTime;
                    static int direction = 1;

                    if ( level.time <= endTime )
                    {
                        if ( endTime - level.time > 1000 )
                            endTime = 0;
                    }
                    else
                    {
                        endTime = level.time + 1000;
                        direction = direction == 0;
                    }

                    fact = (float)(endTime - level.time) / 1000.0;
                    if ( direction )
                    {
                        timingColor[2] = fact;
                        timingColor[1] = fact;
                        timingColor[0] = fact;
                    }
                    else
                    {
                        timingColor[2] = 1.0 - fact;
                        timingColor[1] = 1.0 - fact;
                        timingColor[0] = 1.0 - fact;
                    }
                    if ( !actor->scriptGoalEnt.isDefined() )
                    {
                        pos[0] = actor->scriptGoal.pos[0];
                        pos[1] = actor->scriptGoal.pos[1];
                        pos[2] = actor->scriptGoal.pos[2];
                        pos[2] = pos[2] + 16.0;
                        G_DebugCircle(pos, actor->scriptGoal.radius, timingColor, 0, 1, 0);
                    }
                    if ( actor->fixedNode )
                    {
                        pos[0] = actor->scriptGoal.pos[0];
                        pos[1] = actor->scriptGoal.pos[1];
                        pos[2] = actor->scriptGoal.pos[2];
                        pos[2] = pos[2] + 16.0;
                        timingColor[2] = 0.0f;
                        timingColor[0] = 0.0f;
                        G_DebugLine(xyz, pos, timingColor, 0);
                        G_DebugCircle(pos, actor->fixedNodeSafeRadius, timingColor, 0, 1, 0);
                        if ( actor->fixedNodeSafeVolume.isDefined() )
                        {
                            if ( ai_showVolume->current.integer == self->s.number
                                || ai_showVolume->current.integer > 0 && ai_debugEntIndex->current.integer == self->s.number )
                            {
                                v2 = actor->fixedNodeSafeVolume.ent();
                                G_DebugDrawBrushModel(v2, colorGreenFaded, 0, 0);
                            }
                        }
                    }
                    if ( usingCodeGoal(actor) )
                    {
                        pos[0] = actor->codeGoal.pos[0];
                        pos[1] = actor->codeGoal.pos[1];
                        pos[2] = actor->codeGoal.pos[2];
                        pos[2] = pos[2] + 16.0;
                        G_DebugLine(xyz, pos, colorMagenta, 0);
                        if ( actor->codeGoalSrc == AI_GOAL_SRC_SCRIPT_ENTITY_GOAL )
                        {
                            timingColor[0] = colorBlue[0];
                            timingColor[1] = 0.0;
                            timingColor[2] = 1.0;
                            v40 = 1.0;
                        }
                        else if ( actor->codeGoalSrc == AI_GOAL_SRC_FRIENDLY_CHAIN )
                        {
                            timingColor[0] = colorGreen[0];
                            timingColor[1] = 1.0;
                            timingColor[2] = 0.0;
                            v40 = 1.0;
                        }
                        else
                        {
                            if ( actor->codeGoalSrc == AI_GOAL_SRC_ENEMY )
                            {
                                timingColor[0] = colorMagenta[0];
                                timingColor[1] = 0.0;
                                timingColor[2] = 1.0;
                            }
                            else
                            {
                                memset(timingColor, 0, sizeof(timingColor));
                            }
                            v40 = 1.0;
                        }
                        G_DebugCircle(pos, actor->codeGoal.radius, timingColor, 0, 1, 0);
                    }
                    if ( actor->codeGoal.volume )
                    {
                        if ( ai_showVolume->current.integer == self->s.number
                            || ai_showVolume->current.integer > 0 && ai_debugEntIndex->current.integer == self->s.number )
                        {
                            G_DebugDrawBrushModel(actor->codeGoal.volume, colorWhiteFaded, 1, 0);
                        }
                        // LWSS: wtf, I dont see this dvar anywhere
                        //if ( ai_showRegion->current.enabled )
                        //    Actor_DebugDrawNodesInVolume(actor);
                    }
                }
                if ( actor->sentient->eTeam > (unsigned int)TEAM_NUM_TEAMS )
                    color = colorYellow;
                else
                    color = colorTeam[actor->sentient->eTeam];
                if ( drawGoalLineRadius )
                {
                    xyz[2] = (float)((float)(infoScale * 7.0) * 0.5) + xyz[2];
                    v3 = va("%i", self->s.number);
                    G_AddDebugString(xyz, color, infoScale * 0.60000002, v3, 0);
                }
                else
                {
                    xyz[2] = (float)((float)(infoScale * 7.0) * 10.0) + xyz[2];
                    if ( g_entinfo->current.integer != 2 && g_entinfo_AItext->current.integer )
                    {
                        if ( self->targetname )
                            v29 = SL_ConvertToString(self->targetname, SCRIPTINSTANCE_SERVER);
                        else
                            v29 = "<noname>";
                        v5 = va("%i : %s (%s)", self->s.number, v29, g_entinfoAITextNames[g_entinfo_AItext->current.integer]);
                        G_AddDebugString(xyz, color, infoScale * 0.60000002, v5, 0);
                        textMode = 1 << g_entinfo_AItext->current.integer;
                    }
                    else
                    {
                        if ( self->targetname )
                            v30 = SL_ConvertToString(self->targetname, SCRIPTINSTANCE_SERVER);
                        else
                            v30 = "<noname>";
                        v4 = va("%i : %s", self->s.number, v30);
                        G_AddDebugString(xyz, color, infoScale * 0.60000002, v4, 0);
                        textMode = 30;
                    }
                    if ( (textMode & 6) != 0 )
                    {
                        range = 0.0f;
                        if ( (textMode & 4) != 0 )
                        {
                            xyz[2] = xyz[2] - (float)(infoScale * 7.0);
                            v6 = va("health: %i", self->health);
                            G_AddDebugString(xyz, color, infoScale * 0.60000002, v6, 0);
                        }
                        if ( target )
                        {
                            Sentient_GetOrigin(actor->sentient, myPos);
                            enemyPos[0] = target->r.currentOrigin[0];
                            enemyPos[1] = target->r.currentOrigin[1];
                            enemyPos[2] = target->r.currentOrigin[2];
                            v[0] = myPos[0] - enemyPos[0];
                            v[1] = myPos[1] - enemyPos[1];
                            v[2] = myPos[2] - enemyPos[2];
                            range = Vec3Length(v);
                        }
                        if ( target )
                            v28 = colorRed;
                        else
                            v28 = colorYellow;
                        color = v28;
                        xyz[2] = xyz[2] - (float)(infoScale * 7.0);
                        if ( target )
                        {
                            if ( target->targetname )
                                v26 = SL_ConvertToString(target->targetname, SCRIPTINSTANCE_SERVER);
                            else
                                v26 = "<noname target>";
                            v27 = v26;
                        }
                        else
                        {
                            v27 = "no target";
                        }
                        if ( target )
                            number = target->s.number;
                        else
                            number = 0;
                        v7 = va("%i : %s", number, v27);
                        G_AddDebugString(xyz, color, infoScale * 0.60000002, v7, 0);
                        if ( (textMode & 4) != 0 )
                        {
                            xyz[2] = xyz[2] - (float)(infoScale * 7.0);
                            if ( actor->missCount )
                                missCount = actor->missCount;
                            else
                                missCount = actor->hitCount;
                            if ( actor->missCount )
                                v23 = "MISS";
                            else
                                v23 = "HIT";
                            v8 = va("range: %.2f ac: %.2f %s %u", range, actor->debugLastAccuracy, v23, missCount);
                            G_AddDebugString(xyz, colorWhite, infoScale * 0.60000002, v8, 0);
                            xyz[2] = xyz[2] - (float)(infoScale * 7.0);
                            v9 = va("talkto: %d", actor->talkToSpecies);
                            G_AddDebugString(xyz, colorWhite, infoScale * 0.60000002, v9, 0);
                        }
                    }
                    if ( (textMode & 0x10) != 0 )
                    {
                        xyz[2] = xyz[2] - (float)(infoScale * 7.0);
                        v10 = va(
                                        "(%i)%i:%i = %s",
                                        actor->stateLevel,
                                        actor->eState[actor->stateLevel],
                                        actor->eSubState[actor->stateLevel],
                                        actor->pszDebugInfo);
                        G_AddDebugString(xyz, colorWhite, infoScale * 0.60000002, v10, 0);
                    }
                    if ( (textMode & 0x12) != 0 )
                    {
                        xyz[2] = xyz[2] - (float)(infoScale * 7.0);
                        if ( actor->pAnimScriptFunc )
                            v22 = SL_ConvertToString(actor->pAnimScriptFunc->name, SCRIPTINSTANCE_SERVER);
                        else
                            v22 = "<none>";
                        v11 = SL_ConvertToString(actor->scriptState, SCRIPTINSTANCE_SERVER);
                        v12 = va("%s [%s]", v22, v11);
                        G_AddDebugString(xyz, colorWhite, infoScale * 0.60000002, v12, 0);
                    }
                    if ( (textMode & 0x10) != 0 )
                    {
                        xyz[2] = xyz[2] - (float)(infoScale * 7.0);
                        v21 = SL_ConvertToString(actor->stateChangeReason, SCRIPTINSTANCE_SERVER);
                        v13 = SL_ConvertToString(actor->lastScriptState, SCRIPTINSTANCE_SERVER);
                        v14 = va("<-    %s [%s]", v13, v21);
                        G_AddDebugString(xyz, colorWhite, infoScale * 0.60000002, v14, 0);
                    }
                    if ( (textMode & 8) != 0 )
                    {
                        xyz[2] = xyz[2] - (float)(infoScale * 7.0);
                        if ( actor->ScriptOrient.eMode == AI_ORIENT_INVALID || actor->ScriptOrient.eMode == actor->CodeOrient.eMode )
                        {
                            if ( actor->ScriptOrient.eMode )
                                color = colorYellow;
                            else
                                color = colorGreen;
                        }
                        else
                        {
                            color = colorRed;
                        }
                        if ( actor->ScriptOrient.eMode )
                            v15 = va(
                                            "orient: %s (%s <- script)",
                                            ai_orient_mode_text[actor->CodeOrient.eMode],
                                            ai_orient_mode_text[actor->ScriptOrient.eMode]);
                        else
                            v15 = va("orient: %s", ai_orient_mode_text[actor->CodeOrient.eMode]);
                        G_AddDebugString(xyz, color, infoScale * 0.60000002, v15, 0);
                        if ( Actor_HasPath(actor) )
                        {
                            xyz[2] = xyz[2] - (float)(infoScale * 7.0);
                            G_AddDebugString(xyz, colorWhite, infoScale * 0.60000002, (char*)"has path", 0);
                        }
                        if ( actor->Path.wPathLen > 0 && !Path_DistanceGreaterThan(&actor->Path, 128.0) )
                        {
                            xyz[2] = xyz[2] - (float)(infoScale * 7.0);
                            G_AddDebugString(xyz, colorWhite, infoScale * 0.60000002, (char *)"minDistForceFaceEnemy", 0);
                        }
                        if ( actor->pPileUpActor )
                        {
                            xyz[2] = xyz[2] - (float)(infoScale * 7.0);
                            v16 = va("blockee: %d, blocker: %d", actor->pPileUpActor->ent->s.number, actor->pPileUpEnt->s.number);
                            G_AddDebugString(xyz, colorWhite, infoScale * 0.60000002, v16, 0);
                        }
                        if ( actor->pCloseEnt.isDefined() )
                        {
                            xyz[2] = xyz[2] - (float)(infoScale * 7.0);
                            v17 = actor->pCloseEnt.ent();
                            v18 = va("closeEnt: %d", v17->s.number);
                            G_AddDebugString(xyz, colorWhite, infoScale * 0.60000002, v18, 0);
                        }
                        if ( actor->bDontAvoidPlayer )
                        {
                            xyz[2] = xyz[2] - (float)(infoScale * 7.0);
                            G_AddDebugString(xyz, colorWhite, infoScale * 0.60000002, (char *)"dontavoidplayer", 0);
                        }
                        if ((actor->Physics.iTraceMask & 0x2000000) == 0)
                        {
                            xyz[2] = xyz[2] - (float)(infoScale * 7.0);
                            G_AddDebugString(xyz, colorWhite, infoScale * 0.60000002, (char *)"pushPlayer", 0);
                        }
                        xyz[2] = xyz[2] - (float)(infoScale * 7.0);
                        v19 = va("physics %d", actor->Physics.ePhysicsType);
                        G_AddDebugString(xyz, colorWhite, infoScale * 0.60000002, v19, 0);
                    }
                    if ( (textMode & 0xA) != 0 )
                    {
                        if ( actor->eAnimMode >= (unsigned int)(AI_ANIM_NOPHYSICS|AI_ANIM_USE_POS_DELTAS)
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                                        2522,
                                        0,
                                        "actor->eAnimMode doesn't index ARRAY_COUNT( animModeNames )\n\t%i not in [0, %i)",
                                        actor->eAnimMode,
                                        10) )
                        {
                            __debugbreak();
                        }
                        xyz[2] = xyz[2] - (float)(infoScale * 7.0);
                        v20 = va(
                                        "animmode %s script: %s",
                                        animModeNames[actor->eAnimMode],
                                        animModeNames[actor->eScriptSetAnimMode]);
                        G_AddDebugString(xyz, colorWhite, infoScale * 0.60000002, v20, 0);
                    }
                    if ( (textMode & 4) != 0 )
                    {
                        if ( !actor->ent->takedamage )
                        {
                            xyz[2] = xyz[2] - (float)(infoScale * 7.0);
                            G_AddDebugString(xyz, colorRed, infoScale * 0.60000002, (char*)"Invulnerable", 0);
                        }
                        if ( actor->provideCoveringFire )
                        {
                            xyz[2] = xyz[2] - (float)(infoScale * 7.0);
                            G_AddDebugString(xyz, colorWhite, infoScale * 0.60000002, (char *)"Covering fire", 0);
                        }
                    }
                }
            }
        }
    }
}

void __fastcall Actor_InitMove(actor_s *self)
{
    float *maxs; // [esp+8h] [ebp-34h]
    float *mins; // [esp+10h] [ebp-2Ch]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 2568, 0, "%s", "self") )
        __debugbreak();
    if ( !self->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 2569, 0, "%s", "self->ent") )
    {
        __debugbreak();
    }

    memset(&self->Physics, 0, sizeof(self->Physics));

    static colgeom_visitor_inlined_t<200> dummy_3;
    //if ( (_S2_8 & 1) == 0 )
    //{
    //    _S2_8 |= 1u;
    //    colgeom_visitor_t::colgeom_visitor_t(&dummy_3);
    //    dummy_3.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&colgeom_visitor_inlined_t<200>::`vftable';
    //    colgeom_visitor_inlined_t<500>::reset(&dummy_3);
    //    atexit(Actor_InitMove_::_2_::_dynamic_atexit_destructor_for__dummy__);
    //}

    //self->Physics.proximity_data.__vftable = dummy_3.__vftable;
    self->Physics.proximity_data = dummy_3;
    //colgeom_visitor_inlined_t<500>::reset(&self->Physics.proximity_data);
    self->Physics.proximity_data.reset();
    self->Physics.vOrigin[0] = 0.0f;
    self->Physics.vOrigin[1] = 0.0f;
    self->Physics.vOrigin[2] = 0.0f;
    self->Physics.iEntNum = self->ent->s.number;
    self->Physics.ePhysicsType = AIPHYS_BAD;
    self->Physics.iMsec = 50;
    self->Physics.bIsAlive = 1;
    if ( self->ent->health <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    2582,
                    0,
                    "%s",
                    "self->ent->health > 0") )
    {
        __debugbreak();
    }
    self->Physics.iTraceMask = 0x2820011;
    self->Physics.iHitEntnum = 1023;
    self->Physics.groundEntNum = 1022;
    mins = self->ent->r.mins;
    self->Physics.vMins[0] = *mins;
    self->Physics.vMins[1] = mins[1];
    self->Physics.vMins[2] = mins[2];
    maxs = self->ent->r.maxs;
    self->Physics.vMaxs[0] = *maxs;
    self->Physics.vMaxs[1] = maxs[1];
    self->Physics.vMaxs[2] = maxs[2];
}

bool __cdecl Actor_IsDodgeEntity(actor_s *self, int entnum)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    if ( ((unsigned int)&objBuf[1758][2] & level.gentities[1023].flags) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    2598,
                    0,
                    "%s",
                    "level.gentities[ENTITYNUM_NONE].flags & FL_OBSTACLE") )
    {
        __debugbreak();
    }
    ent = &level.gentities[entnum];
    if ( ent->sentient )
        return level.time < self->iTeamMoveDodgeTime;
    if ( ((unsigned int)&objBuf[1758][2] & ent->flags) != 0 )
        return 0;
    return self->Path.iPathTime > ent->iDisconnectTime;
}

void __fastcall Actor_DoMove(actor_s *self)
{
    bool v1; // [esp+18h] [ebp-3C0h]
    bool v2; // [esp+1Ch] [ebp-3BCh]
    float *currentOrigin; // [esp+54h] [ebp-384h]
    float right[3]; // [esp+8Ch] [ebp-34Ch] BYREF
    float forward[3]; // [esp+98h] [ebp-340h] BYREF
    pathnode_t *node; // [esp+A4h] [ebp-334h]
    int iNodeCount; // [esp+A8h] [ebp-330h]
    float bestDist; // [esp+ACh] [ebp-32Ch]
    float dist; // [esp+B0h] [ebp-328h]
    unsigned __int16 oldGroundEntNum; // [esp+B4h] [ebp-324h]
    float deltaHeight; // [esp+B8h] [ebp-320h]
    pathnode_t *pTestNode; // [esp+BCh] [ebp-31Ch]
    pathsort_t nodes[64]; // [esp+C0h] [ebp-318h] BYREF
    int bSuccess; // [esp+3D0h] [ebp-8h]
    int i; // [esp+3D4h] [ebp-4h]

    iassert(self);
    iassert(self->ent);
    iassert(self->ent->s.number == self->Physics.iEntNum);
    iassert(Vec3Compare(self->ent->r.mins, self->Physics.vMins));
    iassert(Vec3Compare(self->ent->r.maxs, self->Physics.vMaxs));
    iassert(self->Physics.ePhysicsType != AIPHYS_BAD);

    oldGroundEntNum = self->Physics.groundEntNum;
    if ( self->Physics.ePhysicsType != AIPHYS_NORMAL_ABSOLUTE )
    {
        YawVectors(self->fDesiredBodyYaw, forward, right);
        
        float wishdelta[3];
        Vec3Copy(self->Physics.vWishDelta, wishdelta);

        Vec3Scale(forward, wishdelta[0], self->Physics.vWishDelta);

        self->Physics.vWishDelta[0] += ((-(wishdelta[1])) * right[0]);
        self->Physics.vWishDelta[1] += ((-(wishdelta[1])) * right[1]);
        self->Physics.vWishDelta[2] += ((-(wishdelta[1])) * right[2]);

        self->Physics.vWishDelta[2] += wishdelta[2];
    }

    self->Physics.fGravity = bg_gravity->current.value;

    if ( self->eAnimMode != AI_ANIM_MOVE_CODE
        || !self->moveMode
        || !Actor_HasPath(self)
        || self->pCloseEnt.isDefined() )
    {
        if ( !Actor_ShouldMoveAwayFromCloseEnt(self) )
        {
            self->ent->flags &= 0xE7FFFFFF;
            currentOrigin = self->ent->r.currentOrigin;
            self->Physics.vOrigin[0] = *currentOrigin;
            self->Physics.vOrigin[1] = currentOrigin[1];
            self->Physics.vOrigin[2] = currentOrigin[2];
            bSuccess = Actor_Physics(&self->Physics);
LABEL_41:
            if ( !bSuccess )
            {
                float vOrigin[3]; // [esp+3C4h] [ebp-14h] BYREF
                Vec3Copy(self->ent->r.currentOrigin, vOrigin);

                iNodeCount = Path_NodesInCylinder(self->ent->r.currentOrigin, 384.0, 128.0, nodes, 64, -1);
                node = 0;
                bestDist = FLT_MAX;
                for ( i = 0; i < iNodeCount; ++i )
                {
                    pTestNode = nodes[i].node;
                    dist = Vec3DistanceSq(vOrigin, pTestNode->constant.vOrigin);
                    if ( dist <= bestDist )
                    {
                        bestDist = dist;
                        node = pTestNode;
                    }
                }
                if ( node )
                {
                    deltaHeight = node->constant.vOrigin[2] - self->ent->r.currentOrigin[2];
                    if ( deltaHeight <= 8.0 )
                    {
                        if ( deltaHeight < 0.0 )
                        {
                            if ( deltaHeight >= -18.0 )
                                deltaHeight = 0.0f;
                            else
                                deltaHeight = -8.0f;
                        }
                    }
                    else
                    {
                        deltaHeight = 8.0f;
                    }
                    Vec3Copy(self->ent->r.currentOrigin, self->Physics.vOrigin);
                    self->Physics.vOrigin[2] += deltaHeight;
                    self->Physics.vVelocity[2] = 0.0f;
                }
            }
            goto LABEL_67;
        }
        bSuccess = Actor_PhysicsMoveAway(self);
        if ( bSuccess || self->eAnimMode != AI_ANIM_MOVE_CODE || !Actor_HasPath(self) )
            goto LABEL_41;
    }
    bSuccess = Actor_PhysicsAndDodge(self);
    if ( bSuccess )
    {
        if ( self->Path.lookaheadDir[2] <= 4.0
            || self->Physics.vWishDelta[2] <= 0.0
            || self->ent->r.currentOrigin[2] < self->Physics.vOrigin[2] )
        {
            if ( self->sentient->bNearestNodeBad )
            {
                Sentient_InvalidateNearestNode(self->sentient);
                Sentient_NearestNode(self->sentient);
                if ( self->sentient->bNearestNodeBad )
                    bSuccess = 0;
            }
        }
        else
        {
            bSuccess = 0;
        }
    }
    if ( !bSuccess )
    {
        Vec3Add(self->ent->r.currentOrigin, self->Physics.vWishDelta, self->Physics.vOrigin);
        self->Physics.vVelocity[2] = 0.0f;
        self->Path.wDodgeEntity = ENTITYNUM_NONE;

        if ( self->Path.fLookaheadAmount < 64.0 )
            self->Path.fLookaheadAmount = 64.0f;
    }

LABEL_67:
    Vec3Copy(self->Physics.vOrigin, self->ent->r.currentOrigin);
    self->Physics.ePhysicsType = AIPHYS_BAD;
    self->ent->s.groundEntityNum = self->Physics.groundEntNum;
    if ( oldGroundEntNum != self->Physics.groundEntNum )
        Scr_Notify(self->ent, scr_const.groundEntChanged, 0);
}

int __fastcall Actor_PhysicsAndDodge(actor_s *self)
{
    int v2; // [esp+4h] [ebp-C4h]
    bool v3; // [esp+Ch] [ebp-BCh]
    float *v5; // [esp+30h] [ebp-98h]
    float *currentOrigin; // [esp+50h] [ebp-78h]
    int newFlags; // [esp+68h] [ebp-60h]
    float wishLen; // [esp+6Ch] [ebp-5Ch]
    float vNextNodeDir; // [esp+70h] [ebp-58h]
    float vNextNodeDir_4; // [esp+74h] [ebp-54h]
    int bDoFailsafe; // [esp+78h] [ebp-50h]
    int attempts; // [esp+80h] [ebp-48h]
    int oldPathLen; // [esp+84h] [ebp-44h]
    int iHitEntnum; // [esp+88h] [ebp-40h]
    float *pathDir; // [esp+8Ch] [ebp-3Ch]
    int oldFlags; // [esp+90h] [ebp-38h]
    float length; // [esp+98h] [ebp-30h]
    float wish[3]; // [esp+9Ch] [ebp-2Ch] BYREF
    PhysicsInputs physicsInputs; // [esp+A8h] [ebp-20h] BYREF
    float dot; // [esp+C4h] [ebp-4h]

    if ( (self->eAnimMode != AI_ANIM_MOVE_CODE || !Actor_HasPath(self))
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    2710,
                    0,
                    "%s",
                    "self->eAnimMode == AI_ANIM_MOVE_CODE && Actor_HasPath( self )") )
    {
        __debugbreak();
    }
    Actor_PhysicsBackupInputs(self, &physicsInputs);
    currentOrigin = self->ent->r.currentOrigin;
    self->Physics.vOrigin[0] = *currentOrigin;
    self->Physics.vOrigin[1] = currentOrigin[1];
    self->Physics.vOrigin[2] = currentOrigin[2];
    if ( !Actor_Physics(&self->Physics) )
    {
        self->ent->flags &= 0xE7FFFFFF;
        return 0;
    }
    if ( self->Physics.iHitEntnum == 1023 )
    {
        self->ent->flags &= 0xE7FFFFFF;
        return 1;
    }
    iHitEntnum = self->Physics.iHitEntnum;
    newFlags = 0;
    if ( !self->Physics.bStuck && Actor_IsDodgeEntity(self, self->Physics.iHitEntnum) && Path_IsTrimmed(&self->Path) )
    {
        if ( self->Path.wNegotiationStartNode < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                        2734,
                        0,
                        "%s",
                        "self->Path.wNegotiationStartNode >= 0") )
        {
            __debugbreak();
        }
        if ( self->Path.wPathLen - 2 < self->Path.wNegotiationStartNode
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                        2735,
                        0,
                        "%s",
                        "self->Path.wPathLen - 2 >= self->Path.wNegotiationStartNode") )
        {
            __debugbreak();
        }
        //pathDir = (float *)(&self->Physics.proximity_data.prims[194].4 + 7 * self->Path.wPathLen);
        pathDir = (float *)(&self->Physics.proximity_data.prims[194].tree + 7 * self->Path.wPathLen);
        length = Vec2Length(self->Physics.vWishDelta);
        for ( attempts = 0; attempts < 2; ++attempts )
        {
            vNextNodeDir = self->Path.vCurrPoint[0] - self->Physics.vHitOrigin[0];
            vNextNodeDir_4 = self->Path.vCurrPoint[1] - self->Physics.vHitOrigin[1];
            Vec2Normalize(self->Physics.vHitNormal);
            length = length * 0.75;
            newFlags = (int)Actor_Physics_GetLeftOrRightDodge(
                                                self,
                                                (float)(vNextNodeDir * pathDir[1]) >= (float)(vNextNodeDir_4 * *pathDir),
                                                length);
            Actor_PhysicsRestoreInputs(self, &physicsInputs);
            v5 = self->ent->r.currentOrigin;
            self->Physics.vOrigin[0] = *v5;
            self->Physics.vOrigin[1] = v5[1];
            self->Physics.vOrigin[2] = v5[2];
            if ( !Actor_Physics(&self->Physics) )
            {
                self->ent->flags &= 0xE7FFFFFF;
                return 0;
            }
            if ( self->Physics.bStuck || !Actor_IsDodgeEntity(self, self->Physics.iHitEntnum) )
                break;
        }
    }
    oldFlags = self->ent->flags & 0x18000000;
    if ( oldFlags )
    {
        v3 = newFlags && newFlags != oldFlags;
        bDoFailsafe = v3;
        self->ent->flags &= 0xE7FFFFFF;
    }
    else
    {
        bDoFailsafe = 0;
    }
    self->ent->flags |= newFlags;
    if ( self->Physics.iHitEntnum == 1023 )
    {
LABEL_56:
        self->Physics.iHitEntnum = iHitEntnum;
        return 1;
    }
    if ( !bDoFailsafe )
    {
        bDoFailsafe = Path_FailedLookahead(&self->Path);
        if ( !bDoFailsafe )
        {
            wishLen = Vec2NormalizeTo(self->Physics.vWishDelta, wish);
            dot = (float)((float)(self->Physics.vOrigin[0] - self->ent->r.currentOrigin[0]) * wish[0])
                    + (float)((float)(self->Physics.vOrigin[1] - self->ent->r.currentOrigin[1]) * wish[1]);
            bDoFailsafe = (float)(0.25 * wishLen) > dot;
        }
    }
    if ( !level.gentities[iHitEntnum].sentient && !level.gentities[self->Physics.iHitEntnum].sentient )
    {
        if ( bDoFailsafe )
        {
            oldPathLen = self->Path.wPathLen;
            Actor_RecalcPath(self);
            if ( oldPathLen == self->Path.wPathLen )
                return 0;
        }
        goto LABEL_56;
    }
    if ( level.gentities[iHitEntnum].sentient )
        LOWORD(v2) = iHitEntnum;
    else
        v2 = self->Physics.iHitEntnum;
    self->Path.wDodgeEntity = v2;
    if ( !self->Path.wDodgeCount )
        self->Path.wDodgeCount = -1;
    if ( !bDoFailsafe )
    {
        self->Physics.iHitEntnum = 1023;
        return 1;
    }
    if ( level.gentities[iHitEntnum].sentient )
        goto LABEL_56;
    if ( !level.gentities[self->Physics.iHitEntnum].sentient
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    2823,
                    0,
                    "%s",
                    "level.gentities[self->Physics.iHitEntnum].sentient") )
    {
        __debugbreak();
    }
    return 1;
}

int __fastcall Actor_Physics_GetLeftOrRightDodge(actor_s *self, bool dodgeRight, float length)
{
    float v3; // xmm0_4
    float v5; // xmm0_4

    iassert(self);

    if (dodgeRight)
    {
        if ((self->ent->flags & 0x8000000) != 0)
        {
            self->Physics.vWishDelta[0] = (float)((float)((-(self->Physics.vHitNormal[1])) * length) * 0.33000001) + self->Physics.vWishDelta[0];
            v3 = (float)((float)(self->Physics.vHitNormal[0] * length) * 0.33000001) + self->Physics.vWishDelta[1];
        }
        else
        {
            self->Physics.vWishDelta[0] = (-(self->Physics.vHitNormal[1])) * length;
            v3 = self->Physics.vHitNormal[0] * length;
        }
        self->Physics.vWishDelta[1] = v3;
        return 0x10000000;
    }
    else
    {
        if ((self->ent->flags & 0x10000000) != 0)
        {
            self->Physics.vWishDelta[0] = (float)((float)(self->Physics.vHitNormal[1] * length) * 0.33000001) + self->Physics.vWishDelta[0];
            v5 = (float)((float)((-(self->Physics.vHitNormal[0])) * length) * 0.33000001) + self->Physics.vWishDelta[1];
        }
        else
        {
            self->Physics.vWishDelta[0] = self->Physics.vHitNormal[1] * length;
            v5 = (-(self->Physics.vHitNormal[0])) * length;
        }
        self->Physics.vWishDelta[1] = v5;
        return 0x8000000;
    }
}

void __cdecl Actor_PhysicsBackupInputs(actor_s *self, PhysicsInputs *inputs)
{
    inputs->vVelocity[0] = self->Physics.vVelocity[0];
    inputs->vVelocity[1] = self->Physics.vVelocity[1];
    inputs->vVelocity[2] = self->Physics.vVelocity[2];
    inputs->groundEntNum = self->Physics.groundEntNum;
    inputs->bHasGroundPlane = self->Physics.bHasGroundPlane;
    inputs->groundplaneSlope = self->Physics.groundplaneSlope;
    inputs->iFootstepTimer = self->Physics.iFootstepTimer;
}

void __cdecl Actor_PhysicsRestoreInputs(actor_s *self, PhysicsInputs *inputs)
{
    self->Physics.vVelocity[0] = inputs->vVelocity[0];
    self->Physics.vVelocity[1] = inputs->vVelocity[1];
    self->Physics.vVelocity[2] = inputs->vVelocity[2];
    self->Physics.groundEntNum = inputs->groundEntNum;
    self->Physics.bHasGroundPlane = inputs->bHasGroundPlane;
    self->Physics.groundplaneSlope = inputs->groundplaneSlope;
    self->Physics.iFootstepTimer = inputs->iFootstepTimer;
}

bool __fastcall Actor_ShouldMoveAwayFromCloseEnt(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 2923, 0, "%s", "self") )
        __debugbreak();
    //if ( !EntHandle::isDefined(&self->pCloseEnt) )
    if ( !self->pCloseEnt.isDefined() )
        return 0;
    if ( self->Physics.ePhysicsType == AIPHYS_NOCLIP )
        return 0;
    return self->pushable;
}

int __fastcall Actor_PhysicsMoveAway(actor_s *self)
{
    float *v3; // [esp+14h] [ebp-BCh]
    float *v4; // [esp+20h] [ebp-B0h]
    float v5; // [esp+40h] [ebp-90h]
    float v6; // [esp+44h] [ebp-8Ch]
    float *currentOrigin; // [esp+6Ch] [ebp-64h]
    int newFlags; // [esp+74h] [ebp-5Ch]
    float translation[3]; // [esp+7Ch] [ebp-54h] BYREF
    float distance; // [esp+88h] [ebp-48h]
    float rotation[2]; // [esp+8Ch] [ebp-44h] BYREF
    gentity_s *other; // [esp+94h] [ebp-3Ch]
    int NUM_ATTEMPTS; // [esp+98h] [ebp-38h]
    int i; // [esp+9Ch] [ebp-34h]
    float length; // [esp+A0h] [ebp-30h]
    PhysicsInputs physicsInputs; // [esp+A4h] [ebp-2Ch] BYREF
    float distanceSqrd; // [esp+C0h] [ebp-10h]
    float vDelta[2]; // [esp+C4h] [ebp-Ch] BYREF
    float lengthSqrd; // [esp+CCh] [ebp-4h]

    if ( !Actor_ShouldMoveAwayFromCloseEnt(self)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    2979,
                    0,
                    "%s",
                    "Actor_ShouldMoveAwayFromCloseEnt( self )") )
    {
        __debugbreak();
    }
    iassert(self->pCloseEnt.isDefined());
    other = self->pCloseEnt.ent();
    Actor_PhysicsBackupInputs(self, &physicsInputs);
    currentOrigin = self->ent->r.currentOrigin;
    self->Physics.vOrigin[0] = *currentOrigin;
    self->Physics.vOrigin[1] = currentOrigin[1];
    self->Physics.vOrigin[2] = currentOrigin[2];
    if ( other->client )
    {
        v5 = other->r.currentOrigin[0] - self->Physics.vOrigin[0];
        v6 = other->r.currentOrigin[1] - self->Physics.vOrigin[1];
        distanceSqrd = (float)(v5 * v5) + (float)(v6 * v6);
        length = Actor_CalcultatePlayerPushDelta(self, other, vDelta);
        lengthSqrd = length * length;
    }
    else
    {
        vDelta[0] = self->Physics.vOrigin[0] - other->r.currentOrigin[0];
        vDelta[1] = self->Physics.vOrigin[1] - other->r.currentOrigin[1];
        distance = Vec2Normalize(vDelta);
        if ( distance == 0.0 )
        {
            vDelta[0] = G_crandom();
            vDelta[1] = G_crandom();
            Vec2Normalize(vDelta);
        }
        distanceSqrd = distance * distance;
        Actor_GetAnimDeltas(self, rotation, translation);
        lengthSqrd = (float)(translation[0] * translation[0]) + (float)(translation[1] * translation[1]);
        if ( lengthSqrd < 2.2500002 )
            lengthSqrd = 2.2500002f;
        length = sqrtf(lengthSqrd);
    }
    lengthSqrd = lengthSqrd * 0.25;
    self->Physics.vWishDelta[0] = length * vDelta[0];
    self->Physics.vWishDelta[1] = length * vDelta[1];
    if ( Actor_Physics(&self->Physics) )
    {
        if ( Actor_PhysicsCheckMoveAwayNoWorse(self, other, 0, distanceSqrd, lengthSqrd) )
        {
            return 1;
        }
        else if ( self->Physics.groundEntNum == 1023 )
        {
            return 1;
        }
        else
        {
            NUM_ATTEMPTS = 2;
            for ( i = 0; i < 2 && self->Physics.iHitEntnum != 1023; ++i )
            {
                Vec2Normalize(self->Physics.vHitNormal);
                length = length * 0.75;
                lengthSqrd = lengthSqrd * 0.5625;
                newFlags = Actor_Physics_GetLeftOrRightDodge(
                                         self,
                                         (float)(self->Physics.vHitNormal[0] * vDelta[1]) >= (float)(self->Physics.vHitNormal[1] * vDelta[0]),
                                         length);
                Actor_PhysicsRestoreInputs(self, &physicsInputs);
                v4 = self->ent->r.currentOrigin;
                self->Physics.vOrigin[0] = *v4;
                self->Physics.vOrigin[1] = v4[1];
                self->Physics.vOrigin[2] = v4[2];
                if ( !Actor_Physics(&self->Physics) )
                {
                    self->ent->flags &= 0xE7FFFFFF;
                    return 0;
                }
                if ( Actor_PhysicsCheckMoveAwayNoWorse(self, other, (int)newFlags, distanceSqrd, lengthSqrd) )
                    return 1;
            }
            self->Physics.iHitEntnum = other->s.number;
            if ( other->actor )
            {
                if ( !other->actor->pCloseEnt.isDefined() )
                    other->actor->pCloseEnt.setEnt(self->ent);
            }
            self->ent->flags &= 0xE7FFFFFF;
            Actor_PhysicsRestoreInputs(self, &physicsInputs);
            v3 = self->ent->r.currentOrigin;
            self->Physics.vOrigin[0] = *v3;
            self->Physics.vOrigin[1] = v3[1];
            self->Physics.vOrigin[2] = v3[2];
            return 1;
        }
    }
    else
    {
        self->ent->flags &= 0xE7FFFFFF;
        return 0;
    }
}

double __cdecl Actor_CalcultatePlayerPushDelta(const actor_s *self, const gentity_s *pusher, float *pushDir)
{
    float delta; // [esp+20h] [ebp-24h]
    float delta_4; // [esp+24h] [ebp-20h]
    float speed; // [esp+2Ch] [ebp-18h]
    float viewDir[3]; // [esp+30h] [ebp-14h] BYREF
    float moveDir[2]; // [esp+3Ch] [ebp-8h] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 2888, 0, "%s", "self") )
        __debugbreak();
    if ( !self->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 2889, 0, "%s", "self->ent") )
    {
        __debugbreak();
    }
    if ( !pusher && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 2890, 0, "%s", "pusher") )
        __debugbreak();
    if ( !pusher->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 2891, 0, "%s", "pusher->client") )
    {
        __debugbreak();
    }
    delta = self->ent->r.currentOrigin[0] - pusher->r.currentOrigin[0];
    delta_4 = self->ent->r.currentOrigin[1] - pusher->r.currentOrigin[1];
    speed = Vec2NormalizeTo(pusher->client->ps.velocity, moveDir);
    if ( speed == 0.0 )
    {
        G_GetPlayerViewDirection(pusher, viewDir, 0, 0);
        Vec2NormalizeTo(viewDir, moveDir);
    }
    *pushDir = moveDir[1];
    pushDir[1] = -moveDir[0];
    //*((unsigned int *)pushDir + 1) = LODWORD(moveDir[0]) ^ _mask__NegFloat_;

    if ( (float)((float)(*pushDir * delta) + (float)(pushDir[1] * delta_4)) < 0.0 )
    {
        //*(unsigned int *)pushDir = LODWORD(moveDir[1]) ^ _mask__NegFloat_;
        pushDir[0] = -moveDir[1];
        pushDir[1] = moveDir[0];
    }
    if ( speed < 60.0 )
        speed = ACTOR_PLAYER_PUSH_MIN_SPEED;
    return (float)(speed * 0.050000001);
}

int __fastcall Actor_PhysicsCheckMoveAwayNoWorse(
                actor_s *self,
                gentity_s *other,
                int flags,
                float distanceSqrd,
                float lengthSqrd)
{
    float v7; // [esp+10h] [ebp-18h]
    float v8; // [esp+14h] [ebp-14h]
    float v9; // [esp+20h] [ebp-8h]
    float v10; // [esp+24h] [ebp-4h]

    v9 = other->r.currentOrigin[0] - self->Physics.vOrigin[0];
    v10 = other->r.currentOrigin[1] - self->Physics.vOrigin[1];
    if ( (float)((float)(v9 * v9) + (float)(v10 * v10)) <= distanceSqrd )
        return 0;
    v7 = self->ent->r.currentOrigin[0] - self->Physics.vOrigin[0];
    v8 = self->ent->r.currentOrigin[1] - self->Physics.vOrigin[1];
    if ( (float)((float)(v7 * v7) + (float)(v8 * v8)) <= lengthSqrd || !Actor_MoveAwayNoWorse(self) )
        return 0;
    self->ent->flags &= 0xE7FFFFFF;
    self->ent->flags |= flags;
    return 1;
}

int __cdecl Actor_MoveAwayNoWorse(actor_s *self)
{
    float v2; // [esp+8h] [ebp-24h]
    float v3; // [esp+Ch] [ebp-20h]
    float v4; // [esp+18h] [ebp-14h]
    float v5; // [esp+1Ch] [ebp-10h]
    int bResult; // [esp+20h] [ebp-Ch]
    actor_s *other; // [esp+24h] [ebp-8h]
    int i; // [esp+28h] [ebp-4h]

    bResult = 1;
    for ( i = 0; i < 16; ++i )
    {
        other = &level.actors[i];
        if ( other->inuse
            && other != self
            && (!self->pCloseEnt.isDefined() || other->ent != self->pCloseEnt.ent()) )
        {
            v4 = other->ent->r.currentOrigin[0] - self->Physics.vOrigin[0];
            v5 = other->ent->r.currentOrigin[1] - self->Physics.vOrigin[1];
            if ( (float)((float)(v4 * v4) + (float)(v5 * v5)) < 900.0 )
            {
                v2 = other->ent->r.currentOrigin[0] - self->ent->r.currentOrigin[0];
                v3 = other->ent->r.currentOrigin[1] - self->ent->r.currentOrigin[1];
                if ( (float)((float)(v2 * v2) + (float)(v3 * v3)) >= 900.0
                    && !Actor_AtDifferentElevation(self->ent->r.currentOrigin, other->ent->r.currentOrigin) )
                {
                    if ( !other->pCloseEnt.isDefined() && !Actor_AtClaimNode(other))
                        other->pCloseEnt.setEnt(self->ent);
                    bResult = 0;
                }
            }
        }
    }
    return bResult;
}

bool __cdecl Actor_AtDifferentElevation(float *vOrgSelf, float *vOrgOther)
{
    return fabs(vOrgSelf[2] - vOrgOther[2]) >= 80.0;
}

void __cdecl actor_controller(const gentity_s *self, int *partBits)
{
    float v2; // [esp+0h] [ebp-18h]
    float fFrac; // [esp+Ch] [ebp-Ch]
    DObjAnimMat *rotTrans; // [esp+10h] [ebp-8h]
    actor_prone_info_s *pProneInfo; // [esp+14h] [ebp-4h]

    if ( !self->actor
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 3233, 0, "%s", "self->actor") )
    {
        __debugbreak();
    }
    pProneInfo = &self->actor->ProneInfo;
    if ( BG_ActorIsProne(pProneInfo, level.time) && SV_DObjSetRotTransIndex(self, partBits, 0) )
    {
        fFrac = BG_GetActorProneFraction(pProneInfo, level.time);
        rotTrans = SV_DObjGetRotTransArray(self);
        if ( rotTrans )
        {
            v2 = (float)(pProneInfo->fTorsoPitch * fFrac) * 0.0087266462;
            rotTrans->quat[0] = 0.0f;
            rotTrans->quat[2] = 0.0f;
            rotTrans->quat[3] = cos(v2);
            rotTrans->quat[1] = sin(v2);
            DObjSetTrans(rotTrans, vec3_origin);
        }
    }
}

bool __fastcall Actor_PointNear(const float *vPoint, const float *vGoalPos)
{
    float v3; // [esp+10h] [ebp-8h]
    float deltaHeight; // [esp+14h] [ebp-4h]

    deltaHeight = vPoint[2] - vGoalPos[2];
    if ( (float)(deltaHeight * deltaHeight) > 6400.0 )
        return 0;
    v3 = vGoalPos[1] - vPoint[1];
    return (float)((float)((float)(*vGoalPos - *vPoint) * (float)(*vGoalPos - *vPoint)) + (float)(v3 * v3)) <= 900.0;
}

bool __fastcall Actor_PointNearNode(const float *vPoint, const pathnode_t *node)
{
    float v3; // [esp+14h] [ebp-8h]
    float deltaHeight; // [esp+18h] [ebp-4h]

    deltaHeight = vPoint[2] - node->constant.vOrigin[2];
    if ( (float)(deltaHeight * deltaHeight) > 6400.0 )
        return 0;
    v3 = node->constant.vOrigin[1] - vPoint[1];
    return (float)(15.0 * 15.0) >= (float)((float)((float)(node->constant.vOrigin[0] - *vPoint)
                                                                                             * (float)(node->constant.vOrigin[0] - *vPoint))
                                                                             + (float)(v3 * v3));
}

bool __fastcall Actor_PointAtGoal(const float *vPoint, const actor_goal_s *goal)
{
    float v3; // [esp+Ch] [ebp-Ch]
    const gentity_s *volume; // [esp+10h] [ebp-8h]
    float deltaHeight; // [esp+14h] [ebp-4h]

    deltaHeight = vPoint[2] - goal->pos[2];
    if ( (float)(deltaHeight * deltaHeight) > (float)(goal->height * goal->height) )
        return 0;
    v3 = goal->pos[1] - vPoint[1];
    if ( (float)((float)((float)(goal->pos[0] - *vPoint) * (float)(goal->pos[0] - *vPoint)) + (float)(v3 * v3)) > (float)(goal->radius * goal->radius) )
        return 0;
    volume = goal->volume;
    return !volume || SV_EntityContact(vPoint, vPoint, volume);
}

bool __fastcall Actor_PointNearPoint(const float *vPoint, const float *vGoalPos, float buffer)
{
    float v4; // [esp+10h] [ebp-8h]
    float deltaHeight; // [esp+14h] [ebp-4h]

    deltaHeight = vPoint[2] - vGoalPos[2];
    if ( (float)(deltaHeight * deltaHeight) > 6400.0 )
        return 0;
    v4 = vGoalPos[1] - vPoint[1];
    return (float)(buffer * buffer) >= (float)((float)((float)(*vGoalPos - *vPoint) * (float)(*vGoalPos - *vPoint))
                                                                                     + (float)(v4 * v4));
}

bool __fastcall Actor_PointAt(const float *vPoint, const float *vGoalPos)
{
    float v3; // [esp+10h] [ebp-8h]
    float deltaHeight; // [esp+14h] [ebp-4h]

    deltaHeight = vPoint[2] - vGoalPos[2];
    if ( (float)(deltaHeight * deltaHeight) > 6400.0 )
        return 0;
    v3 = vGoalPos[1] - vPoint[1];
    return (float)((float)((float)(*vGoalPos - *vPoint) * (float)(*vGoalPos - *vPoint)) + (float)(v3 * v3)) <= 4.0;
}

bool __fastcall Actor_IsAtGoal(actor_s *self)
{
    gentity_s *ent; // [esp+4h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 3366, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 3367, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    ent = self->ent;
    if ( !self->ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 3370, 0, "%s", "ent") )
        __debugbreak();
    if ( !Actor_PointAtGoal(ent->r.currentOrigin, &self->codeGoal) )
        return 0;
    if ( Actor_HasPath(self) )
        return Actor_PointAtGoal(self->Path.vFinalGoal, &self->codeGoal)
                && (!self->sentient->pClaimedNode
                 || Actor_KeepClaimedNode(self)
                 || Actor_PointAt(ent->r.currentOrigin, self->sentient->pClaimedNode->constant.vOrigin));
    else
        return !self->sentient->pClaimedNode || Actor_IsNearClaimedNode(self);
}

bool __fastcall Actor_IsAtScriptGoal(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 3398, 0, "%s", "self") )
        __debugbreak();
    if ( !self->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 3399, 0, "%s", "self->ent") )
    {
        __debugbreak();
    }
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 3400, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( !Actor_PointAtGoal(self->ent->r.currentOrigin, &self->scriptGoal) )
        return 0;
    if ( Actor_HasPath(self) )
        return Actor_PointAtGoal(self->Path.vFinalGoal, &self->scriptGoal);
    return 1;
}

bool __fastcall Actor_IsNearClaimedNode(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 3418, 0, "%s", "self") )
        __debugbreak();
    if ( !self->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 3419, 0, "%s", "self->ent") )
    {
        __debugbreak();
    }
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 3420, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( !self->sentient->pClaimedNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    3421,
                    0,
                    "%s",
                    "self->sentient->pClaimedNode") )
    {
        __debugbreak();
    }
    return Actor_KeepClaimedNode(self) || Actor_PointNearNode(self->ent->r.currentOrigin, self->sentient->pClaimedNode);
}

gentity_s *__fastcall Actor_IsKnownEnemyInRegion(
                const actor_s *self,
                const gentity_s *volume,
                const float *position,
                float radius)
{
    float v7; // [esp+14h] [ebp-18h]
    float v8; // [esp+18h] [ebp-14h]
    const sentient_s *sentient; // [esp+20h] [ebp-Ch]
    const actor_s *actor; // [esp+24h] [ebp-8h]
    unsigned int i; // [esp+28h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 3436, 0, "%s", "self") )
        __debugbreak();
    if ( !position
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 3437, 0, "%s", "position") )
    {
        __debugbreak();
    }
    if ( radius == 0.0 && !volume )
        return 0;
    for ( i = 0; i < 0x30; ++i )
    {
        if ( level.sentients[i].inuse )
        {
            if ( self->sentientInfo[i].lastKnownPosTime )
            {
                if ( level.time - self->sentientInfo[i].lastKnownPosTime <= 10000 )
                {
                    sentient = &level.sentients[i];
                    if ( sentient->eTeam != self->sentient->eTeam && (float)sentient->ent->health != 0.0 )
                    {
                        actor = sentient->ent->actor;
                        if ( !actor || !actor->ignoreForFixedNodeSafeCheck && !Actor_IsDying(actor) )
                        {
                            if ( volume )
                            {
                                v7 = volume->r.currentOrigin[0] - self->sentientInfo[i].vLastKnownPos[0];
                                v8 = volume->r.currentOrigin[1] - self->sentientInfo[i].vLastKnownPos[1];
                                if ( (float)((float)(v7 * v7) + (float)(v8 * v8)) <= self->fixedNodeSafeVolumeRadiusSq
                                    && SV_EntityContact(self->sentientInfo[i].vLastKnownPos, self->sentientInfo[i].vLastKnownPos, volume) )
                                {
                                    return level.sentients[i].ent;
                                }
                            }
                            else if ( Actor_PointNearPoint(position, self->sentientInfo[i].vLastKnownPos, radius) )
                            {
                                return level.sentients[i].ent;
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

void __fastcall Actor_FindPathToGoalDirect(actor_s *self)
{
    bool validPath; // [esp+7h] [ebp-1h]

    if ( Actor_HasPath(self) || level.time >= self->pathWaitTime )
    {
        validPath = Actor_FindPathToGoalDirectInternal(self);
        if ( !Actor_HasPath(self) )
        {
            Actor_ClearPileUp(self);
            if ( !validPath )
                Actor_HandleInvalidPath(self);
        }
    }
    else
    {
        Actor_ClearPileUp(self);
    }
}

bool __fastcall Actor_FindPathToGoalDirectInternal(actor_s *self)
{
    float vGoalPos[3]; // [esp+38h] [ebp-374h] BYREF
    float perp[2]; // [esp+44h] [ebp-368h]
    float dist[4]; // [esp+4Ch] [ebp-360h] BYREF
    pathsort_t nodes[64]; // [esp+5Ch] [ebp-350h] BYREF
    float stepheight; // [esp+360h] [ebp-4Ch]
    pathnode_t *pNearestNode; // [esp+364h] [ebp-48h]
    float vNewGoalPos[3]; // [esp+368h] [ebp-44h] BYREF
    int iPlaneCount; // [esp+374h] [ebp-38h]
    pathnode_t *pNodeTo; // [esp+378h] [ebp-34h]
    int nodeCount; // [esp+37Ch] [ebp-30h] BYREF
    float sideMove; // [esp+380h] [ebp-2Ch]
    team_t eTeam; // [esp+384h] [ebp-28h]
    pathnode_t *pNodeTo2; // [esp+388h] [ebp-24h]
    float vNormal[4][2]; // [esp+38Ch] [ebp-20h] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 3490, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 3491, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( Actor_HasPath(self) )
    {
        if ( !Path_NeedsReevaluation(&self->Path) && Actor_PointAtGoal(self->Path.vFinalGoal, &self->codeGoal) )
            return 1;
        Actor_ClearPath(self);
    }
    else if ( self->meleeAttackDist == 0.0 && Actor_PointAtGoal(self->ent->r.currentOrigin, &self->codeGoal) )
    {
        return 1;
    }
    sideMove = fabs(self->sideMove);
    if ( sideMove > (float)(self->codeGoal.radius - 15.0) )
        sideMove = self->codeGoal.radius - 15.0;
    if ( sideMove <= 0.0 )
        return Actor_FindPath(self, self->codeGoal.pos, 1, 0);
    iPlaneCount = 0;
    Sentient_InvalidateNearestNode(self->sentient);
    pNearestNode = Sentient_NearestNode(self->sentient);
    if ( !pNearestNode )
        return 0;
    pNodeTo = Path_NearestNode(self->codeGoal.pos, nodes, -2, 192.0, &nodeCount, 64, NEAREST_NODE_DO_HEIGHT_CHECK);
    if ( !pNodeTo )
        return 0;
    perp[0] = self->Path.lookaheadDir[1];
    perp[1] = -self->Path.lookaheadDir[0];
    if (self->sideMove < 0.0)
        sideMove = -sideMove;
    vNewGoalPos[0] = (float)(sideMove * perp[0]) + self->codeGoal.pos[0];
    vNewGoalPos[1] = (float)(sideMove * perp[1]) + self->codeGoal.pos[1];
    vNewGoalPos[2] = self->codeGoal.pos[2];
    eTeam = self->sentient->eTeam;
    if ( self->Physics.prone )
        stepheight = 10.0f;
    else
        stepheight = 18.0f;
    pNodeTo2 = Path_FindCloseNode(eTeam, pNodeTo, vNewGoalPos, 1);
    Path_PredictionTrace(pNodeTo2->constant.vOrigin, vNewGoalPos, 1023, 0x820011, vGoalPos, stepheight, 1);
    if ( Actor_PointAtGoal(vGoalPos, &self->codeGoal) )
    {
        pNodeTo = pNodeTo2;
    }
    else if ( Actor_PointAtGoal(pNodeTo2->constant.vOrigin, &self->codeGoal) )
    {
        pNodeTo = pNodeTo2;
        vGoalPos[0] = pNodeTo2->constant.vOrigin[0];
        vGoalPos[1] = pNodeTo2->constant.vOrigin[1];
        vGoalPos[2] = pNodeTo2->constant.vOrigin[2];
    }
    else
    {
        vGoalPos[0] = self->codeGoal.pos[0];
        vGoalPos[1] = self->codeGoal.pos[1];
        vGoalPos[2] = self->codeGoal.pos[2];
    }
    if ( iPlaneCount )
    {
        if ( self->badPlaceAwareness == 0.0 )
            Path_FindPathFromToNotCrossPlanes(
                &self->Path,
                eTeam,
                pNearestNode,
                self->ent->r.currentOrigin,
                pNodeTo,
                vGoalPos,
                vNormal,
                dist,
                iPlaneCount,
                1,
                1);
        else
            Path_FindPathFromToNotCrossPlanes(
                &self->Path,
                eTeam,
                pNearestNode,
                self->ent->r.currentOrigin,
                pNodeTo,
                vGoalPos,
                vNormal,
                dist,
                iPlaneCount,
                1,
                0);
    }
    else if ( self->badPlaceAwareness == 0.0 )
    {
        Path_FindPathFromTo(&self->Path, eTeam, pNearestNode, self->ent->r.currentOrigin, pNodeTo, vGoalPos, 1, 1);
    }
    else
    {
        Path_FindPathFromTo(&self->Path, eTeam, pNearestNode, self->ent->r.currentOrigin, pNodeTo, vGoalPos, 1, 0);
    }
    return Actor_HasPath(self);
}

void __fastcall Actor_HandleInvalidPath(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 3607, 0, "%s", "self") )
        __debugbreak();
    self->pathWaitTime = level.time + 500;
    if ( !self->useMeleeAttackSpot )
    {
        Actor_TeamMoveBlocked(self);
        Com_Printf(
            18,
            "%sAI (entity %d, origin %.1f %.1f %.1f) couldn't find path to goal.\n",
            "^1",
            self->ent->s.number,
            self->ent->r.currentOrigin[0],
            self->ent->r.currentOrigin[1],
            self->ent->r.currentOrigin[2]);
        if ( !self->ent
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 3630, 0, "%s", "self->ent") )
        {
            __debugbreak();
        }
        Scr_AddVector(g_pathAttemptGoalPos, SCRIPTINSTANCE_SERVER);
        Scr_Notify(self->ent, scr_const.bad_path, 1u);
    }
}

bool __fastcall Actor_FindPath(
                actor_s *self,
                const float *vGoalPos,
                int bAllowNegotiationLinks,
                bool ignoreSuppression)
{
    pathnode_t *pNearestNode; // [esp+20h] [ebp-28h]
    int iPlaneCount; // [esp+24h] [ebp-24h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 4344, 0, "%s", "self") )
        __debugbreak();
    if ( !vGoalPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 4345, 0, "%s", "vGoalPos") )
    {
        __debugbreak();
    }
    if ( self->ent->tagInfo )
    {
        Actor_ClearPath(self);
        return 1;
    }
    if ( Actor_HasPath(self) )
    {
        if ( !Path_NeedsReevaluation(&self->Path) && Actor_PointAt(self->Path.vFinalGoal, vGoalPos) )
            return 1;
        Actor_ClearPath(self);
    }
    else if ( Actor_PointAt(self->ent->r.currentOrigin, vGoalPos) )
    {
        return 1;
    }
    iPlaneCount = 0;
    Sentient_InvalidateNearestNode(self->sentient);
    pNearestNode = Sentient_NearestNode(self->sentient);
    if ( !pNearestNode )
        return 0;
    if ( self->badPlaceAwareness == 0.0 )
        Path_FindPathFrom(
            &self->Path,
            self->sentient->eTeam,
            pNearestNode,
            self->ent->r.currentOrigin,
            vGoalPos,
            bAllowNegotiationLinks,
            1);
    else
        Path_FindPathFrom(
            &self->Path,
            self->sentient->eTeam,
            pNearestNode,
            self->ent->r.currentOrigin,
            vGoalPos,
            bAllowNegotiationLinks,
            0);
    return Actor_HasPath(self);
}

void __fastcall Actor_RecalcPath(actor_s *self)
{
    bool bAllowNegotiationLinks; // [esp+8h] [ebp-8h]
    pathnode_t *pNearestNode; // [esp+Ch] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 4413, 0, "%s", "self") )
        __debugbreak();
    if ( Actor_HasPath(self) )
    {
        bAllowNegotiationLinks = Path_AllowsObstacleNegotiation(&self->Path);
        Actor_ClearPath(self);
        self->Path.flags |= 0x80u;
        Sentient_InvalidateNearestNode(self->sentient);
        pNearestNode = Sentient_NearestNode(self->sentient);
        if ( pNearestNode )
        {
            if ( self->badPlaceAwareness == 0.0 )
                Path_FindPathFrom(
                    &self->Path,
                    self->sentient->eTeam,
                    pNearestNode,
                    self->ent->r.currentOrigin,
                    self->Path.vFinalGoal,
                    bAllowNegotiationLinks,
                    1);
            else
                Path_FindPathFrom(
                    &self->Path,
                    self->sentient->eTeam,
                    pNearestNode,
                    self->ent->r.currentOrigin,
                    self->Path.vFinalGoal,
                    bAllowNegotiationLinks,
                    0);
            self->Path.flags |= 0x80u;
        }
    }
}

bool __fastcall Actor_FindPathToNode(actor_s *self, pathnode_t *pGoalNode, int bSuppressable)
{
    return Actor_FindPathToNode(self, pGoalNode, bSuppressable, 0);
}

bool __fastcall Actor_FindPathToNode(actor_s *self, pathnode_t *pGoalNode, int bSuppressable, int bIgnoreBadplaces)
{
    float dist[4]; // [esp+20h] [ebp-340h] BYREF
    pathsort_t nodes[64]; // [esp+30h] [ebp-330h] BYREF
    pathnode_t *pNearestNode; // [esp+334h] [ebp-2Ch]
    int iPlaneCount; // [esp+338h] [ebp-28h]
    int nodeCount; // [esp+33Ch] [ebp-24h] BYREF
    float vNormal[4][2]; // [esp+340h] [ebp-20h] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 4450, 0, "%s", "self") )
        __debugbreak();
    if ( !pGoalNode
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 4451, 0, "%s", "pGoalNode") )
    {
        __debugbreak();
    }
    if ( Actor_HasPath(self) )
    {
        if ( !Path_NeedsReevaluation(&self->Path) && Actor_PointAt(self->Path.vFinalGoal, pGoalNode->constant.vOrigin) )
            return 1;
        Actor_ClearPath(self);
    }
    else if ( Actor_PointAt(self->ent->r.currentOrigin, pGoalNode->constant.vOrigin) )
    {
        return 1;
    }
    Sentient_InvalidateNearestNode(self->sentient);
    iPlaneCount = 0;
    pNearestNode = Sentient_NearestNode(self->sentient);
    if ( !pNearestNode )
        return 0;
    if ( (pGoalNode->constant.spawnflags & 1) != 0 )
    {
        pGoalNode = Path_NearestNode(
                                    pGoalNode->constant.vOrigin,
                                    nodes,
                                    -2,
                                    192.0,
                                    &nodeCount,
                                    64,
                                    NEAREST_NODE_DO_HEIGHT_CHECK);
        if ( !pGoalNode )
            return 0;
    }
    if ( iPlaneCount )
    {
        if ( self->badPlaceAwareness == 0.0 )
            Path_FindPathFromToNotCrossPlanes(
                &self->Path,
                self->sentient->eTeam,
                pNearestNode,
                self->ent->r.currentOrigin,
                pGoalNode,
                pGoalNode->constant.vOrigin,
                vNormal,
                dist,
                iPlaneCount,
                1,
                1);
        else
            Path_FindPathFromToNotCrossPlanes(
                &self->Path,
                self->sentient->eTeam,
                pNearestNode,
                self->ent->r.currentOrigin,
                pGoalNode,
                pGoalNode->constant.vOrigin,
                vNormal,
                dist,
                iPlaneCount,
                1,
                0);
    }
    else if ( self->badPlaceAwareness == 0.0 )
    {
        Path_FindPathFromTo(
            &self->Path,
            self->sentient->eTeam,
            pNearestNode,
            self->ent->r.currentOrigin,
            pGoalNode,
            pGoalNode->constant.vOrigin,
            1,
            1);
    }
    else
    {
        Path_FindPathFromTo(
            &self->Path,
            self->sentient->eTeam,
            pNearestNode,
            self->ent->r.currentOrigin,
            pGoalNode,
            pGoalNode->constant.vOrigin,
            1,
            0);
    }
    return Actor_HasPath(self);
}

bool __fastcall Actor_FindPathToSentient(actor_s *self, sentient_s *pGoalEnt, int bSuppressable)
{
    float vGoalPos[3]; // [esp+Ch] [ebp-14h] BYREF
    pathnode_t *pNearestNode; // [esp+18h] [ebp-8h]
    pathnode_t *pGoalNode; // [esp+1Ch] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 4516, 0, "%s", "self") )
        __debugbreak();
    if ( !pGoalEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 4517, 0, "%s", "pGoalEnt") )
    {
        __debugbreak();
    }
    Sentient_GetOrigin(pGoalEnt, vGoalPos);
    if ( Actor_HasPath(self) )
    {
        if ( !Path_NeedsReevaluation(&self->Path) && Actor_PointAt(self->Path.vFinalGoal, vGoalPos) )
            return 1;
        Actor_ClearPath(self);
    }
    Sentient_InvalidateNearestNode(self->sentient);
    pNearestNode = Sentient_NearestNode(self->sentient);
    if ( !pNearestNode )
        return 0;
    pGoalNode = Sentient_NearestNode(pGoalEnt);
    if ( !pGoalNode )
        return 0;
    if ( self->badPlaceAwareness == 0.0 )
        Path_FindPathFromTo(
            &self->Path,
            self->sentient->eTeam,
            pNearestNode,
            self->ent->r.currentOrigin,
            pGoalNode,
            vGoalPos,
            1,
            1);
    else
        Path_FindPathFromTo(
            &self->Path,
            self->sentient->eTeam,
            pNearestNode,
            self->ent->r.currentOrigin,
            pGoalNode,
            vGoalPos,
            1,
            0);
    return Actor_HasPath(self);
}

void __fastcall Actor_FindPathInGoalWithLOS(
                actor_s *self,
                const float *vGoalPos,
                float fWithinDistSqrd,
                bool ignoreSuppression)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 4583, 0, "%s", "self") )
        __debugbreak();
    if ( !vGoalPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 4584, 0, "%s", "vGoalPos") )
    {
        __debugbreak();
    }
    Actor_ClearPath(self);
    Path_FindPathInCylinderWithLOS(
        &self->Path,
        self->sentient->eTeam,
        self->ent->r.currentOrigin,
        vGoalPos,
        &self->codeGoal,
        fWithinDistSqrd,
        1);
}

void __cdecl Actor_BadPlacesChanged()
{
    actor_s *actor; // [esp+0h] [ebp-4h]

    for ( actor = Actor_FirstActor(-1); actor; actor = Actor_NextActor(actor, -1) )
    {
        if ( Actor_HasPath(actor) )
            Path_TrimToBadPlaceLink(&actor->Path, actor->sentient->eTeam);
    }
}

bool __fastcall Actor_HasPath(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 4683, 0, "%s", "self") )
        __debugbreak();
    return Path_Exists(&self->Path);
}

void __fastcall Actor_InitPath(actor_s *self)
{
    Path_Begin(&self->Path);
    self->iTeamMoveDodgeTime = 0;
}

void __fastcall Actor_ClearPath(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 4709, 0, "%s", "self") )
        __debugbreak();
    if ( self->Path.wPathLen )
    {
        Path_AddTrimmedAmount(&self->Path, self->ent->r.currentOrigin);
        Path_Clear(&self->Path);
        self->iTeamMoveDodgeTime = 0;
    }
}

void __fastcall Actor_GetAnimDeltas(actor_s *self, float *rotation, float *translation)
{
    DObj *obj; // [esp+20h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 4729, 0, "%s", "self") )
        __debugbreak();
    if ( !rotation
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 4730, 0, "%s", "rotation") )
    {
        __debugbreak();
    }
    if ( !translation
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 4731, 0, "%s", "translation") )
    {
        __debugbreak();
    }
    obj = Com_GetServerDObj(self->ent->s.number);
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 4734, 0, "%s", "obj") )
        __debugbreak();
    if ( DObjGetTree(obj) )
    {
        XAnimCalcDelta(obj, 0, rotation, translation, self->bUseGoalWeight);
        if ( ai_debugAnimDeltas->current.integer == self->ent->s.number )
            Com_Printf(18, "%i deltas = %g %g %g\n", level.time, *translation, translation[1], translation[2]);
    }
    else
    {
        *translation = 0.0f;
        translation[1] = 0.0f;
        translation[2] = 0.0f;
    }
}

void __fastcall Actor_UpdateAnglesAndDelta(actor_s *self)
{
    bool IsDodgeEntity; // eax
    const char *v2; // eax
    float translation[3]; // [esp+30h] [ebp-20h] BYREF
    float yawChange; // [esp+3Ch] [ebp-14h]
    float dist; // [esp+40h] [ebp-10h]
    float rotation[2]; // [esp+44h] [ebp-Ch] BYREF
    gentity_s *ent; // [esp+4Ch] [ebp-4h]

    iassert(self);
    ent = self->ent;
    iassert(ent);

    if ( self->eAnimMode != AI_ANIM_MOVE_CODE && self->Path.iPathEndTime && self->Path.iPathEndTime - level.time <= 0 )
    {
        Actor_ClearPath(self);
        self->Path.iPathEndTime = 0;
    }
    switch ( self->eAnimMode )
    {
        case AI_ANIM_MOVE_CODE:
            if ( self->moveMode && Actor_HasPath(self) && !self->pCloseEnt.isDefined() )
            {
                Actor_GetAnimDeltas(self, rotation, translation);
                iassert(Actor_HasPath(self));
                self->Physics.ePhysicsType = AIPHYS_NORMAL_ABSOLUTE;
                dist = Vec2Length(translation);
                Path_UpdateMovementDelta(self, dist);
                Actor_PathEndActions(self);
                yawChange = 0.0f;
                if ( ai_showPaths->current.integer )
                    Path_DebugDraw(&self->Path, ent->r.currentOrigin, 1, -1);
            }
            else
            {
                if ( Actor_HasPath(self) )
                {
                    IsDodgeEntity = Actor_IsDodgeEntity(self, self->Physics.iHitEntnum);
                    Path_UpdateLookahead(&self->Path, ent->r.currentOrigin, IsDodgeEntity, 0, 1, 1);
                    Actor_AddStationaryMoveHistory(self);
                }
                self->Physics.ePhysicsType = AIPHYS_NORMAL_ABSOLUTE;
                self->Physics.vWishDelta[0] = 0.0f;
                self->Physics.vWishDelta[1] = 0.0f;
                self->Physics.vWishDelta[2] = 0.0f;
                yawChange = 0.0f;
                if ( ai_showPaths->current.integer == 2 )
                    Path_DebugDraw(&self->Path, ent->r.currentOrigin, 1, -1);
            }
            goto LABEL_36;
        case AI_ANIM_USE_POS_DELTAS:
            Actor_GetAnimDeltas(self, rotation, translation);
            self->Physics.ePhysicsType = AIPHYS_NORMAL_RELATIVE;
            self->Physics.vWishDelta[0] = translation[0];
            self->Physics.vWishDelta[1] = translation[1];
            self->Physics.vWishDelta[2] = 0.0f;
            yawChange = 0.0f;
            goto LABEL_36;
        case AI_ANIM_USE_ANGLE_DELTAS:
            Actor_GetAnimDeltas(self, rotation, translation);
            self->Physics.ePhysicsType = AIPHYS_NORMAL_ABSOLUTE;
            self->Physics.vWishDelta[0] = 0.0f;
            self->Physics.vWishDelta[1] = 0.0f;
            self->Physics.vWishDelta[2] = 0.0f;
            yawChange = RotationToYaw(rotation);
            goto LABEL_36;
        case AI_ANIM_USE_BOTH_DELTAS:
            Actor_GetAnimDeltas(self, rotation, translation);
            self->Physics.ePhysicsType = AIPHYS_NORMAL_RELATIVE;
            self->Physics.vWishDelta[0] = translation[0];
            self->Physics.vWishDelta[1] = translation[1];
            self->Physics.vWishDelta[2] = 0.0f;
            yawChange = RotationToYaw(rotation);
            goto LABEL_36;
        case AI_ANIM_USE_BOTH_DELTAS_NOCLIP:
            Actor_GetAnimDeltas(self, rotation, self->Physics.vWishDelta);
            self->Physics.ePhysicsType = AIPHYS_NOCLIP;
            yawChange = RotationToYaw(rotation);
            goto LABEL_36;
        case AI_ANIM_USE_BOTH_DELTAS_NOGRAVITY:
            Actor_GetAnimDeltas(self, rotation, self->Physics.vWishDelta);
            self->Physics.ePhysicsType = AIPHYS_NOGRAVITY;
            yawChange = RotationToYaw(rotation);
            goto LABEL_36;
        case AI_ANIM_USE_BOTH_DELTAS_ZONLY_PHYSICS:
            Actor_GetAnimDeltas(self, rotation, translation);
            self->Physics.ePhysicsType = AIPHYS_ZONLY_PHYSICS_RELATIVE;
            self->Physics.vWishDelta[0] = translation[0];
            self->Physics.vWishDelta[1] = translation[1];
            self->Physics.vWishDelta[2] = 0.0f;
            yawChange = RotationToYaw(rotation);
            goto LABEL_36;
        case AI_ANIM_POINT_RELATIVE:
            self->Physics.ePhysicsType = AIPHYS_ZONLY_PHYSICS_RELATIVE;
            Actor_SetDesiredAngles(&self->CodeOrient, self->ent->r.currentAngles[0], self->ent->r.currentAngles[1]);
            self->Physics.vVelocity[0] = 0.0f;
            self->Physics.vVelocity[1] = 0.0f;
            self->Physics.vVelocity[2] = 0.0f;
            self->Physics.vWishDelta[0] = 0.0f;
            self->Physics.vWishDelta[1] = 0.0f;
            self->Physics.vWishDelta[2] = 0.0f;
            return;
        default:
            v2 = va("bad eAnimMode value: %d", self->eAnimMode);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5041, 0, v2) )
                __debugbreak();
            yawChange = 0.0f;
LABEL_36:
            if ( ai_debugAnimDeltas->current.integer == ent->s.number )
                Com_Printf(18, "%i yawChange = %g\n", level.time, yawChange);
            if ( yawChange != 0.0 )
                Actor_ChangeAngles(self, 0.0, yawChange);
            Actor_DecideOrientation(self);
            Actor_UpdateBodyAngle(self);
            Actor_UpdateLookAngles(self);
            return;
    }
}

void __fastcall Actor_PathEndActions(actor_s *self)
{
    float v1; // [esp+8h] [ebp-50h]
    float v3; // [esp+30h] [ebp-28h]
    float fInvTimeSteps; // [esp+34h] [ebp-24h]
    float fDeltaScale; // [esp+3Ch] [ebp-1Ch]
    float vGoalDelta; // [esp+48h] [ebp-10h]
    float vGoalDelta_4; // [esp+4Ch] [ebp-Ch]
    float vGoalDelta_8; // [esp+50h] [ebp-8h]
    float distSqrd; // [esp+54h] [ebp-4h]
    float distSqrda; // [esp+54h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 4822, 0, "%s", "self") )
        __debugbreak();
    if ( !Actor_HasPath(self)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    4823,
                    0,
                    "%s",
                    "Actor_HasPath( self )") )
    {
        __debugbreak();
    }
    if ( self->Path.pathEndAnimDistSq > 0.0 )
    {
        v3 = self->ent->r.currentOrigin[1] - self->Path.vFinalGoal[1];
        distSqrd = (float)((float)(self->ent->r.currentOrigin[0] - self->Path.vFinalGoal[0])
                                         * (float)(self->ent->r.currentOrigin[0] - self->Path.vFinalGoal[0]))
                         + (float)(v3 * v3);
        if ( self->Path.pathEndAnimNotified && distSqrd > self->Path.pathEndAnimDistSq )
        {
            Scr_Notify(self->ent, scr_const.run, 0);
            self->Path.pathEndAnimNotified = 0;
        }
        else if ( !self->Path.pathEndAnimNotified
                     && self->Path.pathEndAnimDistSq > distSqrd
                     && Path_CompleteLookahead(&self->Path)
                     && !Actor_IsMovingToMeleeAttack(self) )
        {
            Scr_AddFloat(sqrtf(distSqrd), SCRIPTINSTANCE_SERVER);
            Scr_Notify(self->ent, scr_const.stop_soon, 1u);
            self->Path.pathEndAnimNotified = 1;
        }
    }
    if ( !Actor_SkipPathEndActions(self) && self->Path.pathEndAnimDistSq <= 0.0 )
    {
        vGoalDelta = self->Path.pts[self->Path.wNegotiationStartNode].vOrigPoint[0] - self->ent->r.currentOrigin[0];
        vGoalDelta_4 = self->Path.pts[self->Path.wNegotiationStartNode].vOrigPoint[1] - self->ent->r.currentOrigin[1];
        vGoalDelta_8 = self->Path.pts[self->Path.wNegotiationStartNode].vOrigPoint[2] - self->ent->r.currentOrigin[2];
        if ( self->Path.iPathEndTime )
            goto LABEL_23;
        distSqrda = (float)(vGoalDelta * vGoalDelta) + (float)(vGoalDelta_4 * vGoalDelta_4);
        if ( (distSqrda == 0.0 || Path_CompleteLookahead(&self->Path))
            && distSqrda <= (float)((float)((float)(self->Physics.vVelocity[0] * self->Physics.vVelocity[0])
                                                                        + (float)(self->Physics.vVelocity[1] * self->Physics.vVelocity[1]))
                                                        * 0.0625) )
        {
            self->Path.iPathEndTime = level.time + 500;
LABEL_23:
            if ( self->Path.iPathEndTime - level.time > 0 )
            {
                if ( self->Physics.bHasGroundPlane )
                    v1 = 1.0 / self->Physics.groundplaneSlope;
                else
                    v1 = 1.0f;
                fInvTimeSteps = 50.0 / (float)(self->Path.iPathEndTime - level.time);
                fDeltaScale = (float)((float)(2.0 - fInvTimeSteps) * fInvTimeSteps) * v1;
                self->Physics.vWishDelta[0] = fDeltaScale * vGoalDelta;
                self->Physics.vWishDelta[1] = fDeltaScale * vGoalDelta_4;
                self->Physics.vWishDelta[2] = fDeltaScale * vGoalDelta_8;
            }
            else
            {
                Actor_ClearPath(self);
                self->Path.iPathEndTime = 0;
                self->Physics.vWishDelta[0] = vGoalDelta;
                self->Physics.vWishDelta[1] = vGoalDelta_4;
                self->Physics.vWishDelta[2] = vGoalDelta_8;
            }
        }
    }
}

bool __fastcall Actor_IsMovingToMeleeAttack(actor_s *self)
{
    return self->meleeAttackDist != 0.0 && self->useEnemyGoal && self->useMeleeAttackSpot;
}

bool __fastcall Actor_SkipPathEndActions(actor_s *self)
{
    float vGoalDelta; // [esp+18h] [ebp-Ch]
    float vGoalDelta_4; // [esp+1Ch] [ebp-8h]
    float vGoalDelta_8; // [esp+20h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 4768, 0, "%s", "self") )
        __debugbreak();
    if ( self->Path.iPathEndTime )
        return 0;
    if ( !self->Physics.bHasGroundPlane && self->Physics.groundEntNum == 1023 )
        return 1;
    if ( !Path_AttemptedCompleteLookahead(&self->Path) )
        return 1;
    if ( Path_UsesObstacleNegotiation(&self->Path) )
    {
        vGoalDelta = self->Path.pts[self->Path.wNegotiationStartNode].vOrigPoint[0] - self->ent->r.currentOrigin[0];
        vGoalDelta_4 = self->Path.pts[self->Path.wNegotiationStartNode].vOrigPoint[1] - self->ent->r.currentOrigin[1];
        vGoalDelta_8 = self->Path.pts[self->Path.wNegotiationStartNode].vOrigPoint[2] - self->ent->r.currentOrigin[2];
        if ( (float)((float)((float)((float)((float)(self->Physics.vVelocity[0] * self->Physics.vVelocity[0])
                                                                             + (float)(self->Physics.vVelocity[1] * self->Physics.vVelocity[1]))
                                                             + (float)(self->Physics.vVelocity[2] * self->Physics.vVelocity[2]))
                                             * 0.0049999999)
                             + 0.000001) >= (float)((float)(vGoalDelta * vGoalDelta) + (float)(vGoalDelta_4 * vGoalDelta_4)) )
        {
            if ( Actor_PushState(self, AIS_NEGOTIATION) )
            {
                Path_GetObstacleNegotiationScript(&self->Path, &self->AnimScriptSpecific);
                self->Physics.vWishDelta[0] = vGoalDelta;
                self->Physics.vWishDelta[1] = vGoalDelta_4;
                self->Physics.vWishDelta[2] = vGoalDelta_8;
            }
        }
        return 1;
    }
    else
    {
        return self->Path.pathEndAnimNotified
                || self->arrivalInfo.animscriptOverrideRunTo
                || Actor_IsMovingToMeleeAttack(self);
    }
}

void __fastcall Actor_UpdateOriginAndAngles(actor_s *self)
{
    gentity_s *ent; // [esp+Ch] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5087, 0, "%s", "self") )
        __debugbreak();
    ent = self->ent;
    if ( !self->ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5089, 0, "%s", "ent") )
        __debugbreak();
    if ( self->eAnimMode != AI_ANIM_NOPHYSICS )
    {
        if ( ent->tagInfo )
        {
            G_SetFixedLink(ent, 1);
            Actor_ClearPath(self);
            Actor_UpdateAnglesAndDelta(self);
            self->Physics.vVelocity[0] = 0.0f;
            self->Physics.vVelocity[1] = 0.0f;
            self->Physics.vVelocity[2] = 0.0f;
            self->Physics.vWishDelta[0] = 0.0f;
            self->Physics.vWishDelta[1] = 0.0f;
            self->Physics.vWishDelta[2] = 0.0f;
            G_CalcTagAxis(ent, 1);
        }
        else
        {
            Actor_UpdateAnglesAndDelta(self);
            Actor_DoMove(self);
            GlassSv_PredictTouch(self->ent);
            if ( level.gentities[self->Physics.iHitEntnum].sentient )
            {
                if ( !self->noDodgeMove )
                    Actor_TeamMoveBlocked(self);
            }
        }
    }
}

void __fastcall Actor_PredictOriginAndAngles(actor_s *self)
{
    gentity_s *ent; // [esp+8h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5129, 0, "%s", "self") )
        __debugbreak();
    ent = self->ent;
    if ( !self->ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5131, 0, "%s", "ent") )
        __debugbreak();
    if ( ent->tagInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5133, 0, "%s", "!ent->tagInfo") )
    {
        __debugbreak();
    }
    if ( self->eAnimMode == AI_ANIM_MOVE_CODE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                    5134,
                    0,
                    "%s",
                    "self->eAnimMode != AI_ANIM_MOVE_CODE") )
    {
        __debugbreak();
    }
    Actor_UpdateAnglesAndDelta(self);
    Actor_DoMove(self);
    SV_DObjInitServerTime(ent, 0.050000001);
}

void __fastcall Actor_PredictAnim(actor_s *self)
{
    VariableUnion v1; // eax
    gentity_s *ent; // [esp+4h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5150, 0, "%s", "self") )
        __debugbreak();
    ent = self->ent;
    if ( !self->ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5152, 0, "%s", "ent") )
        __debugbreak();
    v1.intValue = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    G_DObjUpdateServerTime(ent, v1.intValue, BLOPS_NULLSUB);
    Scr_AddInt(1, SCRIPTINSTANCE_SERVER);
}

bool __fastcall Actor_AtClaimNode(actor_s *self)
{
    const pathnode_t *node; // [esp+4h] [ebp-4h]

    node = self->sentient->pClaimedNode;
    return node && Actor_PointNearNode(self->ent->r.currentOrigin, node);
}

bool __fastcall Actor_NearClaimNode(actor_s *self, float dist)
{
    pathnode_t *node; // [esp+8h] [ebp-4h]

    node = self->sentient->pClaimedNode;
    return node && Actor_PointNearPoint(self->ent->r.currentOrigin, node->constant.vOrigin, dist);
}

void __fastcall Actor_CheckCollisions(actor_s *self)
{
    bool v1; // [esp+0h] [ebp-24h]
    float *currentOrigin; // [esp+8h] [ebp-1Ch]
    float vOrgSelf[3]; // [esp+Ch] [ebp-18h] BYREF
    actor_s *other; // [esp+18h] [ebp-Ch]
    int i; // [esp+1Ch] [ebp-8h]
    int bDontDisturb; // [esp+20h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5193, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5194, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( !self->pCloseEnt.isDefined() )
    {
        currentOrigin = self->ent->r.currentOrigin;
        vOrgSelf[0] = *currentOrigin;
        vOrgSelf[1] = currentOrigin[1];
        vOrgSelf[2] = currentOrigin[2];
        v1 = self->pAnimScriptFunc == &g_animScriptTable[self->species]->grenade_cower || Actor_AtClaimNode(self);
        bDontDisturb = v1;
        for ( i = 0; i < 16; ++i )
        {
            other = &level.actors[i];
            if ( other->inuse
                && other != self
                && (!bDontDisturb || level.gentities[other->Physics.iHitEntnum].sentient)
                && Actor_PointNear(other->sentient->ent->r.currentOrigin, vOrgSelf) )
            {
                self->pCloseEnt.setEnt(other->ent);
                return;
            }
        }
    }
}

void __fastcall Actor_UpdatePileUp(actor_s *self)
{
    actor_s *other; // [esp+4h] [ebp-8h]
    int iTeamMoveWaitTime; // [esp+8h] [ebp-4h]

    if ( self->eAnimMode == AI_ANIM_MOVE_CODE )
    {
        if ( self->pPileUpActor == self )
            return;
        other = self;
        while ( 1 )
        {
            other = other->pPileUpActor;
            if ( !other )
                break;
            if ( other->pPileUpActor == other )
            {
                if ( !other->pPileUpEnt
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                                5247,
                                0,
                                "%s",
                                "other->pPileUpEnt") )
                {
                    __debugbreak();
                }
                if ( other == self
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                                5248,
                                0,
                                "%s",
                                "other != self") )
                {
                    __debugbreak();
                }
                iTeamMoveWaitTime = other->iTeamMoveWaitTime;
                if ( level.time >= other->ent->nextthink )
                    iTeamMoveWaitTime += 50;
                if ( level.time < iTeamMoveWaitTime && self->ent != other->pPileUpEnt )
                {
                    self->pPileUpActor = other;
                    self->pPileUpEnt = other->pPileUpEnt;
                    return;
                }
                break;
            }
        }
    }
    Actor_ClearPileUp(self);
}

void __fastcall Actor_ClearPileUp(actor_s *self)
{
    self->pPileUpActor = 0;
    self->pPileUpEnt = 0;
}

void __fastcall Actor_ClipPathToGoal(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5278, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5279, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( !Path_ClipToGoal(&self->Path, &self->codeGoal) )
        Actor_ClearPath(self);
}

void __fastcall Actor_BeginTrimPath(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5291, 0, "%s", "self") )
        __debugbreak();
    Path_BeginTrim(&self->Path, &self->TrimInfo);
}

int __fastcall Actor_TrimPathToAttack(actor_s *self)
{
    gentity_s *targetEnt; // [esp+10h] [ebp-10h]
    float vEnemyPos[3]; // [esp+14h] [ebp-Ch] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5305, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5306, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    targetEnt = Actor_GetTargetEntity(self);
    if ( !targetEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5312, 0, "%s", "targetEnt") )
    {
        __debugbreak();
    }
    Actor_GetTargetLookPosition(self, vEnemyPos);
    return Path_TrimToSeePoint(
                     &self->Path,
                     &self->TrimInfo,
                     self,
                     self->fMaxSightDistSqrd,
                     targetEnt->s.number,
                     vEnemyPos);
}

void __fastcall Actor_ClearMoveHistory(actor_s *self)
{
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 10; ++i )
    {
        self->moveHistory[i][0] = 0.0f;
        self->moveHistory[i][1] = 0.0f;
    }
    self->moveHistoryIndex = 0;
}

bool __fastcall Actor_MayReacquireMove(actor_s *self)
{
    return self->Path.wPathLen <= 0 || self->TrimInfo.iDelta;
}

void __fastcall Actor_GetMoveHistoryAverage(actor_s *self, float *vDir)
{
    int i; // [esp+8h] [ebp-4h]

    *vDir = 0.0f;
    vDir[1] = 0.0f;
    vDir[2] = 0.0f;
    for ( i = 0; i < 10; ++i )
    {
        *vDir = *vDir + self->moveHistory[i][0];
        vDir[1] = vDir[1] + self->moveHistory[i][1];
    }
}

void __fastcall Actor_UpdateMoveHistory(actor_s *self)
{
    int v1; // [esp+0h] [ebp-54h]
    int moveHistoryIndex; // [esp+4h] [ebp-50h]
    float perp_4; // [esp+34h] [ebp-20h]
    float nextWeight; // [esp+38h] [ebp-1Ch]
    int index2; // [esp+3Ch] [ebp-18h]
    int index; // [esp+40h] [ebp-14h]
    float weight; // [esp+44h] [ebp-10h]
    float weighta; // [esp+44h] [ebp-10h]
    int index3; // [esp+48h] [ebp-Ch]
    int i; // [esp+4Ch] [ebp-8h]
    int ia; // [esp+4Ch] [ebp-8h]

    index = self->moveHistoryIndex;
    if ( index )
        moveHistoryIndex = self->moveHistoryIndex;
    else
        moveHistoryIndex = 10;
    index2 = moveHistoryIndex - 1;
    if ( moveHistoryIndex == 1 )
        v1 = 10;
    else
        v1 = moveHistoryIndex - 1;
    index3 = v1 - 1;
    perp_4 = -self->moveHistory[index2][0];
    if ( (float)((float)((float)(self->moveHistory[index2][1] * *((float *)&self->pCloseEnt.infoIndex + 2 * v1))
                                         + (float)(perp_4 * self->moveHistory[v1 - 1][1]))
                         * (float)((float)(self->moveHistory[index2][1] * self->moveHistory[index][0])
                                         + (float)(perp_4 * self->moveHistory[index][1]))) <= 0.0
        && (float)((float)(self->moveHistory[index2][0] * self->moveHistory[index][0])
                         + (float)(self->moveHistory[index2][1] * self->moveHistory[index][1])) >= 0.0
        && (float)((float)(self->moveHistory[index2][0] * self->moveHistory[index3][0])
                         + (float)(self->moveHistory[index2][1] * self->moveHistory[index3][1])) >= 0.0 )
    {
        self->moveHistory[index3][0] = self->moveHistory[index][0];
        self->moveHistory[index3][1] = self->moveHistory[index][1];
    }
    nextWeight = 1.0f;
    for ( i = index + 1; i < 10; ++i )
    {
        weight = nextWeight;
        nextWeight = nextWeight + 1.0;
        self->moveHistory[i][0] = self->moveHistory[i][0] * (float)(weight / nextWeight);
        self->moveHistory[i][1] = self->moveHistory[i][1] * (float)(weight / nextWeight);
    }
    for ( ia = 0; ia < index; ++ia )
    {
        weighta = nextWeight;
        nextWeight = nextWeight + 1.0;
        self->moveHistory[ia][0] = self->moveHistory[ia][0] * (float)(weighta / nextWeight);
        self->moveHistory[ia][1] = self->moveHistory[ia][1] * (float)(weighta / nextWeight);
    }
    self->moveHistoryIndex = (index + 1) % 10;
}

void __fastcall Actor_UpdateNetworkLeanAmount(actor_s *self)
{
    self->ent->s.animState.fLeanAmount = self->leanAmount;
}

void __fastcall Path_UpdateLeanAmount(actor_s *self, float *vWishDir)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5425, 0, "%s", "self") )
        __debugbreak();
    if ( !vWishDir
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5426, 0, "%s", "vWishDir") )
    {
        __debugbreak();
    }
    if ( self->Path.wNegotiationStartNode == self->Path.wPathLen - 2 )
    {
        if ( Actor_PointNearPoint(
                     self->ent->r.currentOrigin,
                     self->Path.pts[self->Path.wNegotiationStartNode].vOrigPoint,
                     100.0) )
        {
            self->leanAmount = (float)((float)(1.0 - 0.5) * self->leanAmount) + (float)(1.0 * 0.5);
            self->prevMoveDir[0] = 0.0f;
            self->prevMoveDir[1] = 0.0f;
            self->leanAmount = 0.0f;
        }
        else
        {
            self->leanAmount = (float)(*vWishDir * self->prevMoveDir[0]) + (float)(vWishDir[1] * self->prevMoveDir[1]);
        }
    }
    else
    {
        self->leanAmount = (float)(*vWishDir * self->prevMoveDir[0]) + (float)(vWishDir[1] * self->prevMoveDir[1]);
    }
    Actor_UpdateNetworkLeanAmount(self);
}

double __fastcall Path_UpdateMomentum(actor_s *self, float *vWishDir, float fMoveDist)
{
    float value; // [esp+18h] [ebp-Ch]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5466, 0, "%s", "self") )
        __debugbreak();
    if ( !vWishDir
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5467, 0, "%s", "vWishDir") )
    {
        __debugbreak();
    }
    if ( fMoveDist > 15.0 )
    {
        Path_UpdateLeanAmount(self, vWishDir);
        fMoveDist = (float)((float)((float)((float)(self->leanAmount + 1.0) * 0.5) * 0.5) + 0.5) * fMoveDist;
        if ( self->leanAmount > 0.0
            && (float)((float)(*vWishDir * self->prevMoveDir[1]) - (float)(vWishDir[1] * self->prevMoveDir[0])) < 0.0 )
        {
            //LODWORD(self->leanAmount) ^= _mask__NegFloat_;
            self->leanAmount = -self->leanAmount;
            Actor_UpdateNetworkLeanAmount(self);
        }
        value = ai_pathMomentum->current.value;
        *vWishDir = (float)((float)(self->prevMoveDir[0] - *vWishDir) * value) + *vWishDir;
        vWishDir[1] = (float)((float)(self->prevMoveDir[1] - vWishDir[1]) * value) + vWishDir[1];
        Vec2Normalize(vWishDir);
    }
    self->prevMoveDir[0] = *vWishDir;
    self->prevMoveDir[1] = vWishDir[1];
    return fMoveDist;
}

void Path_UpdateMovementDelta(actor_s *self, float fMoveDist)
{
    // kcod4
    path_t *pPath; // r30
    int iHitEntnum; // r4
    long double lookAheadLen; // fp2
    double calculatedLen; // fp12
    int moveHistoryIndex; // r29
    float vWishDir[3];
    float vNewDir[3];
    float maxSideMove[2];
    float perp[3];
    float vEndPos[3];
    float dot[3];

    pPath = &self->Path;
    iassert(pPath->wPathLen > 0);

    vWishDir[0] = self->ent->r.currentOrigin[0];
    vWishDir[1] = self->ent->r.currentOrigin[1];
    vWishDir[2] = self->ent->r.currentOrigin[2];

    Path_UpdateLookahead(pPath, vWishDir, Actor_IsDodgeEntity(self, self->Physics.iHitEntnum), 0, 1, 1);

    perp[0] = pPath->lookaheadDir[0];
    perp[1] = pPath->lookaheadDir[1];
    perp[2] = pPath->lookaheadDir[2];
    moveHistoryIndex = self->moveHistoryIndex;
    float speed = g_actorAssumedSpeed[self->species];
    float t = ((fMoveDist * 20.0f) - (speed * 0.5f)) / (speed * 0.5f);

    float vLookDir[2];

    if (t > 0.0f)
    {
        if (t < 1.0f)
        {
            //forwardLookaheadDir2D = self_->Path.forwardLookaheadDir2D;
            vLookDir[0] = t * self->Path.forwardLookaheadDir2D[0];
            vLookDir[1] = t * self->Path.forwardLookaheadDir2D[1];
            t = 1.0 - t;
            vLookDir[0] = (float)(t * perp[0]) + vLookDir[0];
            vLookDir[1] = (float)(t * perp[1]) + vLookDir[1];
            Vec2Normalize(vLookDir);
        }
        else
        {
            //LODWORD(v16[0]) = self_->Path.forwardLookaheadDir2D;
            //*(_QWORD *)vLookDir = *(_QWORD *)self_->Path.forwardLookaheadDir2D;
            vLookDir[0] = self->Path.forwardLookaheadDir2D[0];
            vLookDir[1] = self->Path.forwardLookaheadDir2D[1];
        }
    }
    else
    {
        vLookDir[0] = perp[0];
        vLookDir[1] = perp[1];
    }
    if (self->sideMove != 0.0 && !Path_CompleteLookahead(pPath))
    {
        vNewDir[1] = perp[1];
        vNewDir[2] = (-perp[0]);
        vNewDir[0] = pPath->fLookaheadDist;
        maxSideMove[0] = vNewDir[0] * perp[0];
        maxSideMove[1] = vNewDir[0] * perp[1];
        vEndPos[2] = pPath->fLookaheadDist * 0.5f;
        vEndPos[1] = self->sideMove;
        vEndPos[0] = (-vEndPos[1]);

        if (vEndPos[2] > -vEndPos[1])
            vEndPos[2] = vEndPos[0];

        if (self->sideMove < 0.0)
            vEndPos[2] = -vEndPos[2];

        vNewDir[1] = vEndPos[2] * vNewDir[1];
        vNewDir[2] = vEndPos[2] * vNewDir[2];
        maxSideMove[0] = maxSideMove[0] + vNewDir[1];
        maxSideMove[1] = maxSideMove[1] + vNewDir[2];
        Vec2Normalize(maxSideMove);
        dot[0] = (60.0f * maxSideMove[0]) + vWishDir[0];
        dot[1] = (60.0f * maxSideMove[1]) + vWishDir[1];
        dot[2] = (60.0f * perp[2]) + vWishDir[2];
        if (Path_LookaheadPredictionTrace(pPath, vWishDir, dot))
        {
            perp[0] = maxSideMove[0];
            perp[1] = maxSideMove[1];
            vLookDir[0] = maxSideMove[0];
            vLookDir[1] = maxSideMove[1];
        }
    }

    lookAheadLen = (pPath->lookaheadDir[0] * pPath->forwardLookaheadDir2D[0]) + (pPath->lookaheadDir[1] * pPath->forwardLookaheadDir2D[1]);

    if (lookAheadLen < 0.000001f)
        lookAheadLen = 0.000001f;

    //__libm_sse2_log(v4);
    //__libm_sse2_exp(v5);

    lookAheadLen = log(lookAheadLen);
    lookAheadLen = exp(lookAheadLen);

    calculatedLen = 0.333 * lookAheadLen;

    if (calculatedLen < 0.6f)
        calculatedLen = 0.6f;

    fMoveDist = fMoveDist * calculatedLen;

    if (fMoveDist > pPath->fLookaheadDist)
        fMoveDist = pPath->fLookaheadDist;

    if (self->species == AI_SPECIES_DOG && (self->Path.flags & 2) != 0)
        fMoveDist = Path_UpdateMomentum(self, perp, fMoveDist);

    self->Physics.vWishDelta[0] = fMoveDist * perp[0];
    self->Physics.vWishDelta[1] = fMoveDist * perp[1];
    self->Physics.vWishDelta[2] = fMoveDist * perp[2];

    self->moveHistory[moveHistoryIndex][0] = vLookDir[0];
    self->moveHistory[moveHistoryIndex][0] = vLookDir[1];

    Actor_UpdateMoveHistory(self);
}

void __cdecl Actor_SetFlashed(actor_s *self, int flashed, float strength)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5608, 0, "%s", "self") )
        __debugbreak();
    if ( !flashed || self->flashBangImmunity )
    {
        self->flashBanged = 0;
        self->flashBangedStrength = 0.0f;
    }
    else
    {
        if ( strength > self->flashBangedStrength )
            self->flashBangedStrength = strength;
        self->flashBanged = 1;
        self->aiBadPlace = AI_BADPLACE_NONE;
    }
}

void __fastcall Actor_AddStationaryMoveHistory(actor_s *self)
{
    float *v1; // edx
    float *v3; // [esp+Ch] [ebp-Ch]

    if ( level.time / 50 % 10 )
    {
        v3 = self->moveHistory[self->moveHistoryIndex];
        *v3 = (float)(0.1 * self->Path.lookaheadDir[0]) + *v3;
        v3[1] = (float)(0.1 * self->Path.lookaheadDir[1]) + v3[1];
    }
    else
    {
        Actor_UpdateMoveHistory(self);
        v1 = self->moveHistory[self->moveHistoryIndex];
        *v1 = 0.1 * self->Path.lookaheadDir[0];
        v1[1] = 0.1 * self->Path.lookaheadDir[1];
    }
}

bool __fastcall Actor_IsMoving(actor_s *self)
{
    return self->eAnimMode == AI_ANIM_MOVE_CODE && self->moveMode;
}

void __fastcall Actor_UpdateGoalPos(actor_s *self)
{
    gentity_s *v1; // eax
    float *vLastKnownPos; // [esp+34h] [ebp-24h]
    sentient_s *enemy; // [esp+3Ch] [ebp-1Ch]
    float prevGoalPos[3]; // [esp+4Ch] [ebp-Ch] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5677, 0, "%s", "self") )
        __debugbreak();
    prevGoalPos[0] = self->codeGoal.pos[0];
    prevGoalPos[1] = self->codeGoal.pos[1];
    prevGoalPos[2] = self->codeGoal.pos[2];
    if ( self->useEnemyGoal )
    {
        enemy = Actor_GetTargetSentient(self);
        if ( !enemy && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5684, 0, "%s", "enemy") )
            __debugbreak();
        vLastKnownPos = self->sentientInfo[enemy - level.sentients].vLastKnownPos;
        self->codeGoal.pos[0] = *vLastKnownPos;
        self->codeGoal.pos[1] = vLastKnownPos[1];
        self->codeGoal.pos[2] = vLastKnownPos[2];
        self->codeGoalSrc = AI_GOAL_SRC_ENEMY;
        self->codeGoal.node = 0;
        self->codeGoal.volume = 0;
        Actor_SetGoalRadius(&self->codeGoal, self->pathEnemyFightDist);
        Actor_CheckOverridePos(self, prevGoalPos);
    }
    else
    {
        if ( self->scriptGoalEnt.isDefined() )
        {
            iassert(self->scriptGoalEnt.ent()->r.inuse);
            
            if ( self->scriptGoal.node
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                            5702,
                            0,
                            "%s",
                            "!self->scriptGoal.node") )
            {
                __debugbreak();
            }
            if ( self->scriptGoal.volume
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp",
                            5703,
                            0,
                            "%s",
                            "!self->scriptGoal.volume") )
            {
                __debugbreak();
            }
            self->codeGoal.node = 0;
            self->codeGoal.volume = 0;
            if ( self->scriptGoalEnt.ent()->sentient)
                Sentient_EnemyTeam(self->sentient->eTeam);
            v1 = self->scriptGoalEnt.ent();
            self->codeGoal.pos[0] = v1->r.currentOrigin[0];
            self->codeGoal.pos[1] = v1->r.currentOrigin[1];
            self->codeGoal.pos[2] = v1->r.currentOrigin[2];
            self->codeGoalSrc = AI_GOAL_SRC_SCRIPT_ENTITY_GOAL;
        }
        else
        {
            self->codeGoal.pos[0] = self->scriptGoal.pos[0];
            self->codeGoal.pos[1] = self->scriptGoal.pos[1];
            self->codeGoal.pos[2] = self->scriptGoal.pos[2];
            self->codeGoalSrc = AI_GOAL_SRC_SCRIPT_GOAL;
            self->codeGoal.node = self->scriptGoal.node;
            self->codeGoal.volume = self->scriptGoal.volume;
        }
        Actor_SetGoalRadius(&self->codeGoal, self->scriptGoal.radius);
        Actor_SetGoalHeight(&self->codeGoal, self->scriptGoal.height);
        Actor_CheckOverridePos(self, prevGoalPos);
    }
}

void __cdecl Actor_CheckOverridePos(actor_s *self, const float *prevGoalPos)
{
    if ( self->arrivalInfo.animscriptOverrideRunTo )
    {
        if ( self->codeGoal.pos[0] != *prevGoalPos
            || self->codeGoal.pos[1] != prevGoalPos[1]
            || self->codeGoal.pos[2] != prevGoalPos[2] )
        {
            self->arrivalInfo.animscriptOverrideRunTo = 0;
        }
    }
}

void __fastcall Actor_SetGoalRadius(actor_goal_s *goal, float radius)
{
    iassert(goal);
    iassert(radius >= 0);

    if (radius < 4.0f)
        radius = 4.0f;

    goal->radius = radius;
}

void __fastcall Actor_SetGoalHeight(actor_goal_s *goal, float height)
{
    iassert(goal);
    iassert(height >= 0);

    if (height < 80.0)
        height = 80.0f;

    goal->height = height;
}

bool __fastcall Actor_IsInsideArc(
                actor_s *self,
                const float *origin,
                float radius,
                float angle0,
                float angle1,
                float halfHeight)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\actor_mp.cpp", 5795, 0, "%s", "self") )
        __debugbreak();
    return IsPosInsideArc(self->ent->r.currentOrigin, 15.0, origin, radius, angle0, angle1, halfHeight) != 0;
}

