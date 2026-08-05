#include "g_main_mp.h"
#include <universal/dvar.h>
#include <gfx_d3d/r_reflection_probe.h>
#include <universal/assertive.h>
#include <game/teams.h>
#include <demo/demo_common.h>
#include <game/pathnode.h>
#include <universal/com_memory.h>
#include <clientscript/cscr_stringlist.h>
#include <bgame/bg_misc.h>
#include <bgame/bg_weapons.h>
#include <clientscript/scr_const.h>
#include <qcommon/common.h>
#include <qcommon/threads.h>
#include <qcommon/cm_load.h>
#include <game/enthandle.h>
#include <game/g_helicopter1.h>
#include <game/turret.h>
#include <game/g_missile.h>
#include <turret/turret_placement.h>
#include <DynEntity/DynEntity_server.h>
#include <qcommon/dobj_management.h>
#include "g_utils_mp.h"
#include <game/g_targets.h>
#include "actor_mp.h"
#include <gfx_d3d/r_dvars.h>
#include <bgame/bg_dog_animations_mp.h>
#include "g_active_mp.h"
#include <glass/glass_server.h>
#include <universal/com_files.h>
#include <game/g_hudelem.h>
#include <clientscript/cscr_vm.h>
#include <client_mp/sv_client_mp.h>
#include <server/sv_game.h>
#include <universal/com_shared.h>
#include <server_mp/sv_main_mp.h>
#include <server_mp/sv_init_mp.h>
#include <game/actor_script_cmd.h>
#include <client_mp/cl_cgame_mp.h>
#include "g_misc_mp.h"
#include "g_combat_mp.h"
#include <bgame/bg_fire.h>
#include <game/g_debug.h>
#include <ik/ik.h>
#include "g_spawn_mp.h"
#include "g_team_mp.h"
#include "g_spawnsystem_mp.h"
#include <game/actor_corpse.h>
#include <live/live_steam_server.h>
#include <bgame/bg_pmove.h>
#include <server/sv_world.h>
#include <flame/flame_damage.h>
#include <game/g_mover.h>
#include <game/g_player_corpse.h>
#include <game/g_weapon.h>
#include <client/cl_debugdata.h>
#include <game/g_svcmds.h>
#include <ui_mp/ui_gametype_custom_mp.h>
#include <qcommon/cm_world.h>
#include <bgame/bg_vehicle_anim.h>
#include <sound/snd_bank.h>
#include "g_cmds_mp.h"
#include <game/actor_spawner.h>
#include "pregame.h"

const char *g_entcountNames[8] =
{
  "off",
  "basic entity statistics",
  "entities by type",
  "entities in server snapshot (NO_CLIENT unset)",
  "entities considered for delta (NO_CLIENT and CLIENT_ONCE unset)",
  "delta compared script_model breakdown",
  "entities in server snapshot by eType",
  NULL
};

static const char *g_entinfoTypeNames[4] =
{ "all", "AI only", "vehicle only", NULL };

const char *g_entinfoAITextNames[6] =
{ "all", "brief", "combat", "movement", "state", NULL };

const char *g_entinfoNames[9] =
{
  "off",
  "all ents / draw lines / draw info",
  "selected ent / draw lines / draw info",
  "selected ent / draw info",
  "all ents / draw goal lines and radii",
  "selected ent / draw goal lines and radii",
  "script models considered for network",
  "selected ent / animation tree",
  NULL
};

const char *moveOrientModeStrings[8] =
{
  "invalid",
  "dont_change",
  "motion",
  "enemy",
  "enemy_or_motion",
  "enemy_or_motion_sidestep",
  "goal",
  NULL
};

const dvar_t *g_connectpaths;
const dvar_t *g_loadScripts;
const dvar_t *g_cheats;
const dvar_t *g_erroronpathsnotconnected;
const dvar_t *sv_mapname;
const dvar_t *g_gametype;
const dvar_t *g_synchronousClients;
const dvar_t *g_log;
const dvar_t *g_logTimeStampInSeconds;
const dvar_t *g_logSync;
const dvar_t *g_password;
const dvar_t *g_banIPs;
const dvar_t *g_speed;
const dvar_t *g_knockback;
const dvar_t *g_maxDroppedWeapons;
const dvar_t *g_inactivity;
const dvar_t *g_debugDamage;
const dvar_t *g_debugBullets;
const dvar_t *g_vehicleDrawPath;
const dvar_t *ai_enableBadPlaces;
const dvar_t *g_ai;
const dvar_t *g_spawnai;
const dvar_t *g_dumpAIEvents;
const dvar_t *ai_turnRate;
const dvar_t *ai_useFacingTranslation;
const dvar_t *ai_useLeanRunAnimations;
const dvar_t *ai_useBetterLookahead;
const dvar_t *ai_slowdownMinYawDiff;
const dvar_t *ai_slowdownMaxYawDiff;
const dvar_t *ai_slowdownMinRate;
const dvar_t *ai_slowdownRateBlendFactor;
const dvar_t *ai_angularYawEnabled;
const dvar_t *ai_angularYawAccelRate;
const dvar_t *ai_angularYawDecelFactor;
const dvar_t *ai_corpseCount;
const dvar_t *ai_showNodes;
const dvar_t *ai_showNodesDist;
const dvar_t *ai_showNearestNode;
const dvar_t *ai_showVisData;
const dvar_t *ai_showVisDataDist;
const dvar_t *ai_showPaths;
const dvar_t *ai_debugFindPath;
const dvar_t *ai_debugFindPathDirect;
const dvar_t *ai_debugFindPathWidth;
const dvar_t *ai_debugFindPathLock;
const dvar_t *ai_debugClaimedNodes;
const dvar_t *ai_disableSpawn;
const dvar_t *ai_moveOrientMode;
const dvar_t *ai_pathNegotiationOverlapCost;
const dvar_t *ai_showPotentialThreatDir;
const dvar_t *ai_debugCoverEntityNum;
const dvar_t *ai_showBadPlaces;
const dvar_t *ai_showDodge;
const dvar_t *ai_noDodge;
const dvar_t *ai_pathMomentum;
const dvar_t *ai_debugMayMove;
const dvar_t *ai_showVolume;
const dvar_t *ai_debugAnimDeltas;
const dvar_t *ai_debugThreatSelection;
const dvar_t *ai_debugMeleeAttackSpots;
const dvar_t *ai_debugEntIndex;
const dvar_t *ai_eventDistFootstep;
const dvar_t *ai_eventDistFootstepLite;
const dvar_t *ai_eventDistNewEnemy;
const dvar_t *ai_eventDistReact;
const dvar_t *ai_eventDistPain;
const dvar_t *ai_eventDistDeath;
const dvar_t *ai_eventDistExplosion;
const dvar_t *ai_eventDistGrenadePing;
const dvar_t *ai_eventDistProjPing;
const dvar_t *ai_eventDistGunShot;
const dvar_t *ai_eventDistSilencedShot;
const dvar_t *ai_eventDistBullet;
const dvar_t *ai_eventDistBulletRunning;
const dvar_t *ai_eventDistProjImpact;
const dvar_t *ai_eventDistBadPlace;
const dvar_t *ai_playerNearAccuracy;
const dvar_t *ai_playerNearRange;
const dvar_t *ai_playerFarAccuracy;
const dvar_t *ai_playerFarRange;
const dvar_t *ai_threatUpdateInterval;
const dvar_t *ai_foliageIngoreDist;
const dvar_t *ai_friendlySuppression;
const dvar_t *ai_friendlySuppressionDist;
const dvar_t *ai_meleeRange;
const dvar_t *ai_meleeWidth;
const dvar_t *ai_meleeHeight;
const dvar_t *ai_meleeDamage;
const dvar_t *ai_maxAttackerCount;
const dvar_t *ai_noPathToEnemyGiveupTime;
const dvar_t *bullet_penetrationEnabled;
const dvar_t *g_entinfo;
const dvar_t *g_entinfo_type;
const dvar_t *g_entinfo_AItext;
const dvar_t *g_entinfo_maxdist;
const dvar_t *g_entinfo_scale;
const dvar_t *g_debugPlayerAnimScript;
const dvar_t *g_motd;
const dvar_t *g_playerCollisionEjectSpeed;
const dvar_t *g_dropForwardSpeed;
const dvar_t *g_dropUpSpeedBase;
const dvar_t *g_dropUpSpeedRand;
const dvar_t *g_dropHorzSpeedRand;
const dvar_t *g_clonePlayerMaxVelocity;
const dvar_t *voice_global;
const dvar_t *voice_localEcho;
const dvar_t *voice_deadChat;
const dvar_t *g_allowVote;
const dvar_t *g_allow_teamchange;
const dvar_t *g_listEntity;
const dvar_t *g_listEntityCounts;
const dvar_t *g_entsInSnapshot;
const dvar_t *g_maxEntsInSnapshot;
const dvar_t *g_deadChat;
const dvar_t *g_voiceChatTalkingDuration;
const dvar_t *g_TeamIcon_Axis;
const dvar_t *g_TeamIcon_Allies;
const dvar_t *g_TeamIcon_Free;
const dvar_t *g_TeamIcon_Spectator;
const dvar_t *g_ScoresColor_MyTeam;
const dvar_t *g_ScoresColor_EnemyTeam;
const dvar_t *g_ScoresColor_Spectator;
const dvar_t *g_ScoresColor_Free;
const dvar_t *g_ScoresColor_Allies;
const dvar_t *g_ScoresColor_Axis;
const dvar_t *g_ScoresPing_Interval;
const dvar_t *g_TeamName_Allies;
const dvar_t *g_TeamName_Axis;
const dvar_t *g_TeamColor_Allies;
const dvar_t *g_TeamColor_Axis;
const dvar_t *g_TeamColor_MyTeam;
const dvar_t *g_TeamColor_EnemyTeam;
const dvar_t *g_TeamColor_MyTeamAlt;
const dvar_t *g_TeamColor_EnemyTeamAlt;
const dvar_t *g_TeamColor_Squad;
const dvar_t *g_TeamColor_Spectator;
const dvar_t *g_TeamColor_Free;
const dvar_t *g_debugLocDamage;
const dvar_t *g_debugLocHit;
const dvar_t *g_debugLocHitTime;
const dvar_t *g_smoothClients;
const dvar_t *g_antilag;
const dvar_t *g_oldVoting;
const dvar_t *g_voteAbstainWeight;
const dvar_t *g_NoScriptSpam;
const dvar_t *g_friendlyfireDist;
const dvar_t *g_friendlyNameDist;
const dvar_t *melee_debug;
const dvar_t *radius_damage_debug;
const dvar_t *player_throwbackInnerRadius;
const dvar_t *player_throwbackOuterRadius;
const dvar_t *player_useRadius;
const dvar_t *player_MGUseRadius;
const dvar_t *vehicle_useRadius;
const dvar_t *g_minGrenadeDamageSpeed;
//const dvar_t *g_compassShowEnemies;
const dvar_t *pickupPrints;
const dvar_t *g_revive;
const dvar_t *g_dumpAnims;
const dvar_t *g_useholdtime;
const dvar_t *g_useholdspawndelay;
const dvar_t *g_redCrosshairs;
const dvar_t *g_mantleBlockTimeBuffer;
const dvar_t *g_vehicleDebug;
const dvar_t *vehGunnerSplashDamage;
const dvar_t *turretPlayerAvoidScale;
const dvar_t *g_enableAttachWeaponFix;
const dvar_t *anim_deltas_debug;
const dvar_t *g_destructibleDraw;
const dvar_t *g_debugServerAiming;
const dvar_t *g_fogColorReadOnly;
const dvar_t *g_fogStartDistReadOnly;
const dvar_t *g_fogHalfDistReadOnly;
const dvar_t *vehPlaneRollDeadZone;
const dvar_t *vehPlaneRollAccel;
const dvar_t *vehPlanePitchAccel;
const dvar_t *vehPlaneYawSpeed;
const dvar_t *vehPlaneYawFromRollScale;
const dvar_t *vehPlaneLiftForce;
const dvar_t *vehPlaneFakeLiftForce;
const dvar_t *vehPlaneLowSpeed;
const dvar_t *vehPlaneGravityForce;
const dvar_t *vehicle_switch_seat_delay;
const dvar_t *vehicle_damage_max_shielding;
const dvar_t *vehicle_damage_zone_front;
const dvar_t *vehicle_damage_zone_side;
const dvar_t *vehicle_damage_zone_rear;
const dvar_t *vehicle_damage_zone_under;
const dvar_t *vehicle_damage_bullet;
const dvar_t *vehicle_damage_grenade;
const dvar_t *vehicle_damage_projectile;
const dvar_t *vehicle_damage_bouncing_betty;
const dvar_t *vehicle_damage_satchel_charge;
const dvar_t *vehicle_damage_sticky_grenade;
const dvar_t *vehicle_piece_damagesfx_threshold;
const dvar_t *vehicle_destructible_damage_grenade;
const dvar_t *vehicle_destructible_damage_bouncing_betty;
const dvar_t *vehicle_destructible_damage_satchel_charge;
const dvar_t *vehicle_destructible_damage_sticky_grenade;
const dvar_t *vehicle_destructible_damage_grenade_radius;
const dvar_t *vehicle_destructible_damage_bouncing_betty_radius;
const dvar_t *vehicle_destructible_damage_satchel_charge_radius;
const dvar_t *vehicle_destructible_damage_sticky_grenade_radius;
const dvar_t *vehicle_destructible_damage_projectile_radius;
const dvar_t *vehicle_perk_leadfoot_speed_increase;
const dvar_t *g_turretServerPitchMin;
const dvar_t *g_turretServerPitchMax;
const dvar_t *g_turretBipodOffset;

bgsAnim_s level_bgsAnim;
level_locals_t level;

int g_timed_radius_damage_count;
TIMED_RADIUS_DAMAGE g_timed_radius_damage[512];

gentity_s g_entities[MAX_GENTITIES];
sentient_s g_sentients[48];
actor_s g_actors[16];
gclient_s g_clients[32];

playerState_s g_defaultPlayerState;

int __cdecl G_GetTime()
{
    return level.time;
}

void __cdecl G_RegisterConnectPaths()
{
    g_connectpaths = _Dvar_RegisterInt("g_connectpaths", 0, 0, 3, 0, "Connect paths");
}

bool __cdecl G_OnlyConnectingPaths()
{
    return g_connectpaths->current.integer >= 2;
}

void __cdecl G_RegisterRegisterToolDvars()
{
    bool v0; // al

    G_RegisterConnectPaths();
    R_ReflectionProbeRegisterDvars();
    v0 = G_ExitAfterToolComplete();
    g_loadScripts = _Dvar_RegisterBool("g_loadScripts", !v0, 0, "Disable scripts from loading");
}

int __cdecl G_GetSavePersist()
{
    return level.savepersist;
}

void __cdecl G_SetSavePersist(int savepersist)
{
    level.savepersist = savepersist;
}

double __cdecl G_GetFogOpaqueDistSqrd()
{
    return level.fFogOpaqueDistSqrd;
}

int __cdecl G_GetClientScore(unsigned int clientNum)
{
    if ( clientNum >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    1376,
                    0,
                    "clientNum doesn't index level.maxclients\n\t%i not in [0, %i)",
                    clientNum,
                    level.maxclients) )
    {
        __debugbreak();
    }
    return level.clients[clientNum].sess.cs.score.score;
}

int __cdecl G_GetClientKills(unsigned int clientNum)
{
    if ( clientNum >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    1388,
                    0,
                    "clientNum doesn't index level.maxclients\n\t%i not in [0, %i)",
                    clientNum,
                    level.maxclients) )
    {
        __debugbreak();
    }
    return level.clients[clientNum].sess.cs.score.kills;
}

int __cdecl G_GetClientAssists(unsigned int clientNum)
{
    if ( clientNum >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    1400,
                    0,
                    "clientNum doesn't index level.maxclients\n\t%i not in [0, %i)",
                    clientNum,
                    level.maxclients) )
    {
        __debugbreak();
    }
    return level.clients[clientNum].sess.cs.score.assists;
}

int __cdecl G_GetClientRank(unsigned int clientNum)
{
    if ( clientNum >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    1412,
                    0,
                    "clientNum doesn't index level.maxclients\n\t%i not in [0, %i)",
                    clientNum,
                    level.maxclients) )
    {
        __debugbreak();
    }
    return level.clients[clientNum].sess.cs.rank;
}

int __cdecl G_GetClientPrestige(unsigned int clientNum)
{
    if ( clientNum >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    1424,
                    0,
                    "clientNum doesn't index level.maxclients\n\t%i not in [0, %i)",
                    clientNum,
                    level.maxclients) )
    {
        __debugbreak();
    }
    return level.clients[clientNum].sess.cs.prestige;
}

team_t __cdecl G_GetClientTeam(unsigned int clientNum)
{
    if ( clientNum >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    1436,
                    0,
                    "clientNum doesn't index level.maxclients\n\t%i not in [0, %i)",
                    clientNum,
                    level.maxclients) )
    {
        __debugbreak();
    }
    return level.clients[clientNum].sess.cs.team;
}

int __cdecl G_GetClientDeaths(unsigned int clientNum)
{
    if ( clientNum >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    1449,
                    0,
                    "clientNum doesn't index level.maxclients\n\t%i not in [0, %i)",
                    clientNum,
                    level.maxclients) )
    {
        __debugbreak();
    }
    return level.clients[clientNum].sess.cs.score.deaths;
}

int __cdecl G_GetClientArchiveTime(unsigned int clientNum)
{
    if ( clientNum >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    1462,
                    0,
                    "clientNum doesn't index level.maxclients\n\t%i not in [0, %i)",
                    clientNum,
                    level.maxclients) )
    {
        __debugbreak();
    }
    return level.clients[clientNum].sess.archiveTime;
}

void __cdecl G_SetClientArchiveTime(unsigned int clientNum, int time)
{
    if ( clientNum >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    1475,
                    0,
                    "clientNum doesn't index level.maxclients\n\t%i not in [0, %i)",
                    clientNum,
                    level.maxclients) )
    {
        __debugbreak();
    }
    level.clients[clientNum].sess.archiveTime = time;
}

MatchState *__cdecl G_GetMatchState()
{
    return &level.matchState;
}

clientState_s *__cdecl G_GetClientState(unsigned int clientNum)
{
    if ( !Demo_IsPlaying()
        && clientNum >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    1501,
                    0,
                    "clientNum doesn't index level.maxclients\n\t%i not in [0, %i)",
                    clientNum,
                    level.maxclients) )
    {
        __debugbreak();
    }
    return &level.clients[clientNum].sess.cs;
}

gclient_s *__cdecl G_GetPlayerState(unsigned int clientNum)
{
    if ( !Demo_IsPlaying()
        && clientNum >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    1518,
                    0,
                    "clientNum doesn't index level.maxclients\n\t%i not in [0, %i)",
                    clientNum,
                    level.maxclients) )
    {
        __debugbreak();
    }
    return &level.clients[clientNum];
}

int __cdecl G_GetClientSize()
{
    return 10720;
}

void __cdecl G_FreeEntities(bool clearTargets)
{
    gentity_s *e; // [esp+0h] [ebp-Ch]
    int i; // [esp+8h] [ebp-4h]
    int ia; // [esp+8h] [ebp-4h]
    int ib; // [esp+8h] [ebp-4h]

    e = g_entities;
    for ( i = 0; i < level.num_entities; ++i )
    {
        if ( e->r.inuse )
            G_FreeEntity(e);
        ++e;
    }
    if ( g_entities[ENTITYNUM_WORLD].r.inuse )
        G_FreeEntity(&g_entities[ENTITYNUM_WORLD]);
    if ( clearTargets )
        Targ_RemoveAll();
    if ( level.actors )
    {
        for ( ia = 0; ia < 16; ++ia )
        {
            if ( level.actors[ia].inuse )
                Actor_ClearPileUp(&level.actors[ia]);
        }
    }

    for ( ib = 0; ib < 32; ++ib )
        level.droppedWeaponCue[ib].setEnt(0);

    if ( g_entities[1023].r.inuse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    1582,
                    0,
                    "%s",
                    "!g_entities[ENTITYNUM_NONE].r.inuse") )
    {
        __debugbreak();
    }
    level.num_entities = 0;
    level.firstFreeEnt = 0;
    level.lastFreeEnt = 0;
    Path_CheckLinkLeaks();
    Path_CheckUserCountLeaks();
}

void *__cdecl Hunk_AllocXAnimServer(unsigned int size)
{
    return Hunk_AllocLow(size, "Hunk_AllocXAnimServer", 13);
}

bool __cdecl G_ExitAfterConnectPaths()
{
    return g_connectpaths && g_connectpaths->current.integer >= 2;
}

bool __cdecl G_ExitAfterToolComplete()
{
    return G_ExitAfterConnectPaths() || R_ReflectionProbeGenerateExitWhenDone();
}

bool __cdecl G_ExitOnComError(int code)
{
    if ( g_connectpaths && g_connectpaths->current.integer == 3 && !code )
        return 1;
    if ( g_connectpaths->current.integer >= 2 && code == 1 )
        return 1;
    return r_reflectionProbeGenerate && r_reflectionProbeGenerate->current.enabled;
}

int __cdecl G_IsServerGameSystem(int clientNum)
{
    if ( !g_debugPlayerAnimScript )
        return 0;
    if ( clientNum != g_debugPlayerAnimScript->current.integer )
        return 0;
    if (bgs != &level_bgs)
        return 0;
    Com_Printf(19, "(%i) ", level.time);
    return 1;
}

unsigned __int16 __cdecl G_GetWeaponAttachBone(clientInfo_t *ci, weapType_t weapType, weapInventoryType_t invType)
{
    if ( weapType == WEAPTYPE_GRENADE || weapType == WEAPTYPE_MINE )
    {
        if ( invType != WEAPINVENTORY_ITEM )
            return scr_const.tag_inhand;
    }
    else if ( ci->leftHandGun )
    {
        return SL_FindString(bg_weaponleftbone->current.string, SCRIPTINSTANCE_SERVER);
    }
    return SL_FindString(bg_weaponrightbone->current.string, SCRIPTINSTANCE_SERVER);
}

void G_FreeAnimTreeInstances()
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < com_maxclients->current.integer; ++i )
    {
        if ( level_bgs.clientinfo[i].pXAnimTree )
        {
            XAnimFreeTree(level_bgs.clientinfo[i].pXAnimTree, 0, SCRIPTINSTANCE_SERVER);
            level_bgs.clientinfo[i].pXAnimTree = 0;
        }
    }
    for ( i = 0; i < 4; ++i )
    {
        if (g_scr_data.playerCorpseInfo[i].tree)
        {
            XAnimFreeTree(g_scr_data.playerCorpseInfo[i].tree, 0, SCRIPTINSTANCE_SERVER);
            g_scr_data.playerCorpseInfo[i].tree = NULL;
        }
    }
    for ( i = 0; i < 8; ++i )
    {
        if (g_scr_data.actorCorpseInfo[i].tree)
        {
            XAnimFreeTree(g_scr_data.actorCorpseInfo[i].tree, 0, SCRIPTINSTANCE_SERVER);
            g_scr_data.actorCorpseInfo[i].tree = NULL;
        }
    }
}

void __cdecl    G_InitGame(int levelTime, int randomSeed, int restart, int registerDvars)
{
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    2026,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    
    PROF_SCOPED("G_InitGame");

    Com_Printf(15, "------- Game Initialization -------\n");
    Com_Printf(15, "gamename: %s\n", "Call of Duty®");
    Com_Printf(15, "gamedate: %s\n", "Nov    5 2010");
    Rope_InitRopes();
    Swap_Init();
    EntHandle::Init();
    SentientHandle::Init();
    memset(&level, 0, sizeof(level));
    level.initializing = 1;
    level.currentEntityThink = -1;
    level.scriptPrintChannel = 25;
    srand(randomSeed);
    Rand_Init(randomSeed);
    if ( registerDvars )
        G_RegisterDvars();
    if ( !Dvar_GetBool("sv_cheats") && !restart )
        Dvar_SetCheatState();
    GScr_LoadConsts();

    // === IDA OMITTED BELOW THIS === 
    // (to get around it, I undefined the code and just created a new function where it was stopping)
    G_SetupWeaponDef();
    G_ProcessIPBans();
    UI_Gametype_CustomGameModeDataToDvars();

    level_bgs.animData = &level_bgsAnim;
    level_bgs.GetXModel = SV_XModelGet;
    level_bgs.CreateDObj = G_CreateDObj;
    level_bgs.AttachWeapon = G_AttachWeapon;
    level_bgs.GetDObj = G_GetDObj;
    level_bgs.SafeDObjFree = G_SafeDObjFree;
    level_bgs.AllocXAnim = Hunk_AllocXAnimServer;
    level_bgs.anim_user = 1;
    level_bgs.Rand = G_rand;
    level_bgs.Random = G_random;
    level_bgsAnim.done_notify = scr_const.done;

    G_InitDestructibles();
    CM_LinkWorld();

    level.time = levelTime;
    level.startTime = levelTime;

    G_srand(randomSeed);

    if (*(_BYTE *)g_log->current.integer)
    {
        if (g_logSync->current.enabled)
            FS_FOpenFileByMode((char *)g_log->current.integer, &level.logFile, FS_APPEND_SYNC);
        else
            FS_FOpenFileByMode((char *)g_log->current.integer, &level.logFile, FS_APPEND);
        if (level.logFile)
        {
            char serverinfo[1024];
            SV_GetServerinfo(serverinfo, sizeof(serverinfo));
            G_LogPrintf("------------------------------------------------------------\n");
            G_LogPrintf("InitGame: %s\n", serverinfo);
            G_LogPrintf(
                "InitGame: %s\\g_logTimeStampInSeconds\\%d\n",
                serverinfo,
                g_logTimeStampInSeconds->current.color[0] != 0);
        }
        else
        {
            Com_PrintWarning(15, "WARNING: Couldn't open logfile: %s\n", g_log->current.string);
        }
    }
    else
    {
        Com_Printf(15, "Not logging to disk.\n");
    }

    for (int i = 0; i < 1; i++)
    {
        level.openScriptIOFileHandles[i] = 0;
        level.openScriptIOFileBuffers[i] = 0;
        memset(&level.currentScriptIOLineMark[i], 0, sizeof(level.currentScriptIOLineMark[i]));
    }

    Mantle_CreateAnims(Hunk_AllocXAnimServer);
    level.actorCorpseCount = ai_corpseCount->current.integer;
    if ((level.actorCorpseCount < 1 || level.actorCorpseCount > 8)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
            2159,
            1,
            "%s",
            "level.actorCorpseCount >= 1 && level.actorCorpseCount <= MAX_ACTOR_CORPSES"))
    {
        __debugbreak();
    }

    Dog_CreateAnims(Hunk_AllocXAnimServer);
    SpawnSystem_Init();

    VehAnim_Init();

    if (restart)
        ; //BG_EvalVehicleName();
    else
        G_ParseScrVehicleInfo();

    iassert(bgs == 0);
    bgs = &level_bgs;

    if (!restart)
    {
        memset(&bgs->animData->animScriptData, 0, sizeof(animScriptData_t)/*0x8D388u*/);
        static_assert(sizeof(animScriptData_t) == 0x8D388);

        bgs->animData->animScriptData.soundAlias = SND_FindAlias;
        bgs->animData->animScriptData.playSoundAlias = G_AnimScriptSound;
        //*(_DWORD *)(a1 - 2076) = Dvar_GetString("mapname");

        {
            PROF_SCOPED("GScr_LoadScripts");
            if (!G_ExitAfterToolComplete())
                DB_SyncXAssets();

            GScr_LoadScripts(SCRIPTINSTANCE_SERVER);
        }
        
        // IDA being annoying here too
        const char *mapname = Dvar_GetString("mapname");
        BG_LoadAnim(mapname);
        BG_PostLoadAnim(mapname);
        G_LoadAnimTreeInstances();
    }

    char buffer[1024];
    SV_GetConfigstring(0x15u, buffer, 1024);
    Info_SetValueForKey(buffer, "winner", "0");
    SV_SetConfigstring(21, buffer);

    memset(g_entities, 0, sizeof(g_entities));
    level.gentities = g_entities;
    g_entities[1023].flags |= 0x4000000u;
    level.maxclients = com_maxclients->current.integer;
    memset(g_clients, 0, sizeof(g_clients));
    level.clients = g_clients;
    memset(g_timed_radius_damage, 0, sizeof(g_timed_radius_damage));

    Path_Init(restart);

    level.sentients = g_sentients;
    level.actors = g_actors;
    G_InitSentients();
    G_InitActors();

    for (int i = 0; i < level.maxclients; ++i)
        g_entities[i].client = &level.clients[i];

    level.num_entities = 44;
    level.firstFreeEnt = 0;
    level.lastFreeEnt = 0;
    SV_LocateGameData(level.gentities, level.num_entities, 760, &level.clients->ps, 10720);

    G_ParseHitLocDmgTable();
    BG_LoadPenetrationDepthTable();
    G_InitVehiclePaths();
    G_InitScrVehicles();
    G_InitTurrets();
    GlassSv_Init();
    DynEntSv_InitEntities();
    Path_PreSpawnInitPaths();

    if (!restart)
        G_DropPathnodesToFloor();

    G_SpawnEntitiesFromString();
    G_setfog((char*)"0");

    if (!restart && !IsFastFileLoad())
    {
        G_DropPathnodesToFloor();
    }

    G_SetupVehiclePaths();
    G_SetupScrVehicles();
    G_InitObjectives();
    Missile_InitAttractors();
    Path_PreSpawnInitPaths();

    if (!restart)
        G_DropPathnodesToFloor();

    G_UpdateTrackExtraNodes();

    if (!restart && !IsFastFileLoad())
    {
        G_DropPathnodesToFloor();
    }

    G_DropActorSpawnersToFloor();
    Scr_FreeEntityList(SCRIPTINSTANCE_SERVER);
    Com_Printf(15, "-----------------------------------\n");
    G_InitTargets();
    Scr_InitSystem(SCRIPTINSTANCE_SERVER, 1);
    Scr_SetLoading(1, SCRIPTINSTANCE_SERVER);
    Scr_AllocGameVariable(SCRIPTINSTANCE_SERVER);

    {
        PROF_SCOPED("Load Scripts");

        if (g_loadScripts && g_loadScripts->current.enabled)
        {
            {
                PROF_SCOPED("Load Structs");
                G_LoadStructs();
            }

            if (!r_reflectionProbeGenerate->current.enabled)
            {
                Path_InitPaths();
                Actor_FinishSpawningAll();
                Path_AutoDisconnectPaths();
            }

            {
                PROF_SCOPED("Load Game");
                if (Pregame_ShouldLoadPregame())
                {
                    Pregame_StartPregame();
                    Scr_LoadPreGame();
                }
                else
                {
                    Scr_LoadGameType();
                }
            }

            {
                PROF_SCOPED("Load Level");
                Scr_LoadLevel();
            }

            {
                PROF_SCOPED("Startup Gametype");
                Scr_StartupGameType();
            }
        }
    }

    
    for (int i = 0; i < 4; i++)
    {
        g_scr_data.playerCorpseInfo[i].entnum = -1;
    }

    if (IsFastFileLoad())
    {
        G_PrintAllFastFileErrors();
    }

    iassert(bgs == &level_bgs);
    bgs = NULL;

    g_timed_radius_damage_count = 0;
    level.initializing = 0;
    SaveRegisteredWeapons();
    SaveRegisteredItems();

    if (!restart)
        RadiantRemoteInit();

    RunSavedRadiantCmds();

    //result = GetCurrentThreadId();
    //*(_DWORD *)(a1 - 2140) = result;
    //*(_DWORD *)(a1 - 2136) = 0;
    //if (*(_DWORD *)(a1 - 2140) == (_DWORD)g_DXDeviceThread)
    //{
    //    result = *(_DWORD *)(a1 - 2136);
    //    if (result == HIDWORD(g_DXDeviceThread))
    //        return D3DPERF_EndEvent();
    //}
    //return result;
}

void G_RegisterDvars()
{
    const dvar_s *result; // eax

    g_cheats = _Dvar_RegisterBool("sv_cheats", 1, 0, "Enable cheats");
    g_erroronpathsnotconnected = _Dvar_RegisterBool(
                                                                 "g_erroronpathsnotconnected",
                                                                 1,
                                                                 0x80u,
                                                                 "Errors out during load if paths are not connected.");
    //sv_mapname = _Dvar_RegisterString("sv_mapname", (char *)"", 0x44u, "The current map name");
    _Dvar_RegisterString("sv_mapname", (char *)"", 0x44u, "The current map name");
    g_gametype = _Dvar_RegisterString("g_gametype", "tdm", 0x24u, "The current campaign");
    g_synchronousClients = _Dvar_RegisterBool(
                                                     "g_synchronousClients",
                                                     0,
                                                     0x100u,
                                                     "Call 'client think' exactly once for each server frame to make smooth demos");
    g_log = _Dvar_RegisterString("g_log", "games_mp.log", 1u, "Log file name");
    g_logTimeStampInSeconds = _Dvar_RegisterBool(
                                                            "g_logTimeStampInSeconds",
                                                            0,
                                                            1u,
                                                            "Enable logging with time stamps in seconds since UTC 1/1/1970");
    g_logSync = _Dvar_RegisterBool("g_logSync", 0, 1u, "Enable synchronous logging");
    g_password = _Dvar_RegisterString("g_password", (char *)"", 0, "Password");
    g_banIPs = _Dvar_RegisterString("g_banIPs", (char *)"", 1u, "IP addresses to ban from playing");
    g_speed = _Dvar_RegisterInt("g_speed", 190, 0x80000000, 0x7FFFFFFF, 0, "Player speed");
    g_knockback = _Dvar_RegisterFloat("g_knockback", 1000.0, -3.4028235e38, 3.4028235e38, 0, "Maximum knockback");
    g_maxDroppedWeapons = _Dvar_RegisterInt("g_maxDroppedWeapons", 16, 2, 32, 0, "Maximum number of dropped weapons");
    g_inactivity = _Dvar_RegisterInt(
                                     "g_inactivity",
                                     0,
                                     0,
                                     0x7FFFFFFF,
                                     0,
                                     "Time delay before player is kicked for inactivity");
    g_debugDamage = _Dvar_RegisterBool("g_debugDamage", 0, 0x80u, "Show debug information for damage");
    g_debugBullets = _Dvar_RegisterInt("g_debugBullets", 0, -3, 6, 0x80u, "Show debug information for bullets");
    g_vehicleDrawPath = _Dvar_RegisterString(
                                                "g_vehicleDrawPath",
                                                (char *)"",
                                                0x80u,
                                                "Turn on debug information for vehicle paths");
    ai_enableBadPlaces = _Dvar_RegisterBool("ai_enableBadPlaces", 1, 0, "toggle badplaces system on/off");
    g_ai = _Dvar_RegisterBool("g_ai", 1, 0x2080u, "Enable AI");
    g_spawnai = _Dvar_RegisterBool("g_spawnai", 1, 0x20A0u, "Enable AI spawning");
    g_dumpAIEvents = _Dvar_RegisterInt("g_aiEventDump", -1, -1, 1023, 0x80u, "Print AI events happening for this entity");
    ai_turnRate = _Dvar_RegisterFloat("ai_turnRate", 0.30000001, 0.0099999998, 0.5, 0x4080u, "turn rate for AI");
    ai_useFacingTranslation = _Dvar_RegisterBool(
                                                            "ai_useFacingTranslation",
                                                            0,
                                                            0x4080u,
                                                            "whether to use facing to determine direction of translation");
    ai_useLeanRunAnimations = _Dvar_RegisterBool(
                                                            "ai_useLeanRunAnimations",
                                                            0,
                                                            0x4080u,
                                                            "whether to use lean run animations instead of strafes");
    ai_useBetterLookahead = _Dvar_RegisterBool("ai_useBetterLookahead", 1, 0x4080u, "t5 lookahead improvements");
    ai_slowdownMinYawDiff = _Dvar_RegisterFloat(
                                                        "ai_slowdownMinYawDiff",
                                                        0.0,
                                                        0.0,
                                                        180.0,
                                                        0x4080u,
                                                        "min yaw diff before slowdown kicks in");
    ai_slowdownMaxYawDiff = _Dvar_RegisterFloat(
                                                        "ai_slowdownMaxYawDiff",
                                                        90.0,
                                                        0.0,
                                                        180.0,
                                                        0x4080u,
                                                        "max yaw diff used for slowdown cal");
    ai_slowdownMinRate = _Dvar_RegisterFloat(
                                                 "ai_slowdownMinRate",
                                                 1.0,
                                                 0.0,
                                                 1.0,
                                                 0x4080u,
                                                 "anim rate at ai_slowdownMaxYawDiff");
    ai_slowdownRateBlendFactor = _Dvar_RegisterFloat(
                                                                 "ai_slowdownRateBlendFactor",
                                                                 1.0,
                                                                 0.0,
                                                                 1.0,
                                                                 0x4080u,
                                                                 "percent of desired rate that goes into final rate");
    ai_angularYawEnabled = _Dvar_RegisterBool("ai_angularYawEnabled", 0, 0x4080u, "turn on velocity based body rotation");
    ai_angularYawAccelRate = _Dvar_RegisterFloat(
                                                         "ai_angularYawAccelRate",
                                                         1200.0,
                                                         0.0,
                                                         5000.0,
                                                         0x4080u,
                                                         "yaw acceleration rate");
    ai_angularYawDecelFactor = _Dvar_RegisterFloat(
                                                             "ai_angularYawDecelFactor",
                                                             1.0,
                                                             0.0,
                                                             5.0,
                                                             0x4080u,
                                                             "yaw deceleration factor (decel rate = factor * accel rate)");
    ai_corpseCount = _Dvar_RegisterInt("ai_corpseCount", 5, 0, 8, 0x2001u, "Maximum number of AI corpses");
    ai_showNodes = _Dvar_RegisterInt("ai_showNodes", 0, 0, 4, 0x80u, "Show AI navigation node debug information");
    ai_showNodesDist = _Dvar_RegisterFloat(
                                             "ai_showNodesDist",
                                             384.0,
                                             0.0,
                                             3.4028235e38,
                                             0x80u,
                                             "Maximum distance from the camera at which AI nodes are shown");
    ai_showNearestNode = _Dvar_RegisterInt("ai_showNearestNode", 0, 0, 64, 0x80u, "Show nodes closest to AI");
    ai_showVisData = _Dvar_RegisterInt("ai_showVisData", 0, 0, 2, 0x80u, "Display debug information for visibility data");
    ai_showVisDataDist = _Dvar_RegisterFloat(
                                                 "ai_showVisDataDist",
                                                 1000.0,
                                                 0.0,
                                                 3.4028235e38,
                                                 0x80u,
                                                 "Maximum distance for visibility data debugging information to be shown");
    ai_showPaths = _Dvar_RegisterInt("ai_showPaths", 0, 0, 2, 0x80u, "Show AI navigation paths");
    ai_debugFindPath = _Dvar_RegisterInt(
                                             "ai_debugFindPath",
                                             0,
                                             0,
                                             5,
                                             0x80u,
                                             "Display AI 'find path' debugging information");
    ai_debugFindPathDirect = _Dvar_RegisterBool(
                                                         "ai_debugFindPathDirect",
                                                         0,
                                                         0x80u,
                                                         "Display AI 'find direct path' debugging information");
    ai_debugFindPathWidth = _Dvar_RegisterFloat(
                                                        "ai_debugFindPathWidth",
                                                        0.0,
                                                        -3.4028235e38,
                                                        3.4028235e38,
                                                        0x80u,
                                                        "Display paths with the given width");
    ai_debugFindPathLock = _Dvar_RegisterBool("ai_debugFindPathLock", 0, 0x80u, "Find path lock");
    ai_debugClaimedNodes = _Dvar_RegisterInt(
                                                     "ai_debugClaimedNodes",
                                                     0,
                                                     0,
                                                     64,
                                                     0x80u,
                                                     "Enable debugging information claimed status of nodes");
    ai_disableSpawn = _Dvar_RegisterBool("ai_disableSpawn", 0, 0x2080u, "Do not spawn AI");
    ai_moveOrientMode = _Dvar_RegisterEnum("ai_moveOrientMode", moveOrientModeStrings, 0, 0x80u, "Debug AI Orient Mode");
    ai_pathNegotiationOverlapCost = _Dvar_RegisterFloat(
                                                                        "ai_pathNegotiationOverlapCost",
                                                                        300.0,
                                                                        0.0,
                                                                        1000.0,
                                                                        0x2080u,
                                                                        "The distance AI would travel around to avoid going to a negotiation being used. Mult"
                                                                        "iplied by number of users of the negotiation");
    ai_showPotentialThreatDir = _Dvar_RegisterBool(
                                                                "ai_showPotentialThreatDir",
                                                                0,
                                                                0x80u,
                                                                "Display AI potential threat direction");
    ai_debugCoverEntityNum = _Dvar_RegisterInt(
                                                         "ai_debugCoverEntityNum",
                                                         -1,
                                                         -1,
                                                         1023,
                                                         0x80u,
                                                         "Display debug info for cover");
    ai_showBadPlaces = _Dvar_RegisterBool("ai_showBadPlaces", 0, 0x80u, "Display debug information for 'bad places'");
    ai_showDodge = _Dvar_RegisterBool("ai_showDodge", 0, 0x80u, "Display debug information for AI dodging");
    ai_noDodge = _Dvar_RegisterBool("ai_noDodge", 0, 0x2080u, "AI won't dodge to the side");
    ai_pathMomentum = _Dvar_RegisterFloat(
                                            "ai_pathMomentum",
                                            0.77999997,
                                            0.0,
                                            0.89999998,
                                            0x2080u,
                                            "Momentum factor for continuing motion in previous direction. 0 for no momentum carry over");
    ai_debugMayMove = _Dvar_RegisterBool(
                                            "ai_debugMayMove",
                                            0,
                                            0x80u,
                                            "Display debug information for AI 'may move' calculations");
    ai_showVolume = _Dvar_RegisterInt(
                                        "ai_showVolume",
                                        -1,
                                        -1,
                                        1023,
                                        0x80u,
                                        "Draw the goal volume and fixed node safe volume for an AI");
    ai_debugAnimDeltas = _Dvar_RegisterInt(
                                                 "ai_debugAnimDeltas",
                                                 0,
                                                 0,
                                                 1023,
                                                 0x80u,
                                                 "Display animation delta debug information");
    ai_debugThreatSelection = _Dvar_RegisterBool(
                                                            "ai_debugThreatSelection",
                                                            0,
                                                            0x81u,
                                                            "Enable debugging information for threat selection");
    ai_debugMeleeAttackSpots = _Dvar_RegisterBool(
                                                             "ai_debugMeleeAttackSpots",
                                                             0,
                                                             0x81u,
                                                             "Enable debugging information for melee attack spots");
    ai_debugEntIndex = _Dvar_RegisterInt("ai_debugEntIndex", -1, -1, 1023, 0, "Entity index of an entity to debug");
    ai_eventDistFootstep = _Dvar_RegisterFloat(
                                                     "ai_eventDistFootstep",
                                                     512.0,
                                                     0.0,
                                                     30000.0,
                                                     0x3080u,
                                                     "Distance used for AI event");
    ai_eventDistFootstepLite = _Dvar_RegisterFloat(
                                                             "ai_eventDistFootstepLite",
                                                             256.0,
                                                             0.0,
                                                             30000.0,
                                                             0x3080u,
                                                             "Distance used for AI event");
    ai_eventDistNewEnemy = _Dvar_RegisterFloat(
                                                     "ai_eventDistNewEnemy",
                                                     1024.0,
                                                     0.0,
                                                     30000.0,
                                                     0x3080u,
                                                     "Distance used for AI event");
    ai_eventDistReact = _Dvar_RegisterFloat(
                                                "ai_eventDistReact",
                                                512.0,
                                                0.0,
                                                3.4028235e38,
                                                0x3080u,
                                                "Distance used for AI event");
    ai_eventDistPain = _Dvar_RegisterFloat("ai_eventDistPain", 512.0, 0.0, 30000.0, 0x3080u, "Distance used for AI event");
    ai_eventDistDeath = _Dvar_RegisterFloat(
                                                "ai_eventDistDeath",
                                                1024.0,
                                                0.0,
                                                30000.0,
                                                0x3080u,
                                                "Distance used for AI event");
    ai_eventDistExplosion = _Dvar_RegisterFloat(
                                                        "ai_eventDistExplosion",
                                                        1024.0,
                                                        0.0,
                                                        30000.0,
                                                        0x3080u,
                                                        "Distance used for AI event");
    ai_eventDistGrenadePing = _Dvar_RegisterFloat(
                                                            "ai_eventDistGrenadePing",
                                                            512.0,
                                                            0.0,
                                                            30000.0,
                                                            0x3080u,
                                                            "Distance used for AI event");
    ai_eventDistProjPing = _Dvar_RegisterFloat(
                                                     "ai_eventDistProjPing",
                                                     128.0,
                                                     0.0,
                                                     30000.0,
                                                     0x3080u,
                                                     "Distance used for AI event");
    ai_eventDistGunShot = _Dvar_RegisterFloat(
                                                    "ai_eventDistGunShot",
                                                    2048.0,
                                                    0.0,
                                                    30000.0,
                                                    0x3080u,
                                                    "Distance used for AI event");
    ai_eventDistSilencedShot = _Dvar_RegisterFloat(
                                                             "ai_eventDistSilencedShot",
                                                             128.0,
                                                             0.0,
                                                             30000.0,
                                                             0x3080u,
                                                             "Distance used for AI event");
    ai_eventDistBullet = _Dvar_RegisterFloat(
                                                 "ai_eventDistBullet",
                                                 96.0,
                                                 0.0,
                                                 30000.0,
                                                 0x3080u,
                                                 "Distance used for AI event");
    ai_eventDistBulletRunning = _Dvar_RegisterFloat(
                                                                "ai_eventDistBulletRunning",
                                                                200.0,
                                                                0.0,
                                                                3.4028235e38,
                                                                0x3080u,
                                                                "Distance used for AI event");
    ai_eventDistProjImpact = _Dvar_RegisterFloat(
                                                         "ai_eventDistProjImpact",
                                                         512.0,
                                                         0.0,
                                                         30000.0,
                                                         0x3080u,
                                                         "Distance used for AI event");
    ai_eventDistBadPlace = _Dvar_RegisterFloat(
                                                     "ai_eventDistBadPlace",
                                                     256.0,
                                                     0.0,
                                                     30000.0,
                                                     0x3080u,
                                                     "Distance used for AI event");
    ai_playerNearAccuracy = _Dvar_RegisterFloat(
                                                        "ai_playerNearAccuracy",
                                                        0.5,
                                                        0.0,
                                                        1.0,
                                                        0x2000u,
                                                        "Accuracy for an AI near to a player");
    ai_playerNearRange = _Dvar_RegisterFloat(
                                                 "ai_playerNearRange",
                                                 800.0,
                                                 0.0,
                                                 3.4028235e38,
                                                 0x2000u,
                                                 "Maximum range for AI to use 'near' accuracy");
    ai_playerFarAccuracy = _Dvar_RegisterFloat(
                                                     "ai_playerFarAccuracy",
                                                     0.1,
                                                     0.0,
                                                     1.0,
                                                     0x2000u,
                                                     "Accuracy for AI far away from the player");
    ai_playerFarRange = _Dvar_RegisterFloat(
                                                "ai_playerFarRange",
                                                2000.0,
                                                0.0,
                                                3.4028235e38,
                                                0x2000u,
                                                "Minimum range for AI to use 'far' accuracy");
    ai_threatUpdateInterval = _Dvar_RegisterInt(
                                                            "ai_threatUpdateInterval",
                                                            500,
                                                            0,
                                                            0x7FFFFFFF,
                                                            0x2080u,
                                                            "AI target threat update interval in milliseconds");
    ai_foliageIngoreDist = _Dvar_RegisterFloat(
                                                     "ai_foliageSeeThroughDist",
                                                     128.0,
                                                     0.0,
                                                     3.4028235e38,
                                                     0x3080u,
                                                     "Maximum distance AI ignore foliage for sight trace to targets");
    ai_friendlySuppression = _Dvar_RegisterBool(
                                                         "ai_friendlySuppression",
                                                         1,
                                                         0x3080u,
                                                         "Whether AI fire will suppression teammates or not.");
    ai_friendlySuppressionDist = _Dvar_RegisterFloat(
                                                                 "ai_friendlySuppressionDist",
                                                                 128.0,
                                                                 0.0,
                                                                 2048.0,
                                                                 0x3080u,
                                                                 "Max distance at which AI suppress teammates");
    ai_meleeRange = _Dvar_RegisterFloat(
                                        "ai_meleeRange",
                                        64.0,
                                        0.0,
                                        1000.0,
                                        0x80u,
                                        "The maximum range of the AI's melee attack");
    ai_meleeWidth = _Dvar_RegisterFloat("ai_meleeWidth", 20.0, 0.0, 1000.0, 0x80u, "The width of the AI's melee attack");
    ai_meleeHeight = _Dvar_RegisterFloat(
                                         "ai_meleeHeight",
                                         10.0,
                                         0.0,
                                         1000.0,
                                         0x80u,
                                         "The height of the AI's melee attack");
    ai_meleeDamage = _Dvar_RegisterInt(
                                         "ai_meleeDamage",
                                         100,
                                         0,
                                         1000,
                                         0x80u,
                                         "The amount of damage dealt by AI's melee attack");
    ai_maxAttackerCount = _Dvar_RegisterInt(
                                                    "ai_maxAttackerCount",
                                                    2,
                                                    0,
                                                    1000,
                                                    0x80u,
                                                    "Max number of AI's that will attack one player");
    ai_noPathToEnemyGiveupTime = _Dvar_RegisterInt(
                                                                 "ai_noPathToEnemyGiveupTime",
                                                                 6000,
                                                                 0,
                                                                 60000,
                                                                 0x80u,
                                                                 "Time the AI will continue to attack if the player goes off the path grid.");
    bullet_penetrationEnabled = _Dvar_RegisterBool(
                                                                "bullet_penetrationEnabled",
                                                                1,
                                                                0x80u,
                                                                "Enable/Disable bullet penetration.");
    g_entinfo = _Dvar_RegisterEnum("g_entinfo", g_entinfoNames, 0, 0x80u, "Display entity information");
    g_entinfo_type = _Dvar_RegisterEnum(
                                         "g_entinfo_type",
                                         g_entinfoTypeNames,
                                         1,
                                         0x80u,
                                         "Type of entities to display information");
    g_entinfo_AItext = _Dvar_RegisterEnum(
                                             "g_entinfo_AItext",
                                             g_entinfoAITextNames,
                                             1,
                                             0x80u,
                                             "Type of text information for AI entinfo");
    g_entinfo_maxdist = _Dvar_RegisterFloat(
                                                "g_entinfo_maxdist",
                                                2048.0,
                                                0.0,
                                                3.4028235e38,
                                                0x80u,
                                                "Maximum distance of an entity from the camera at which to show entity information");
    g_entinfo_scale = _Dvar_RegisterFloat(
                                            "g_entinfo_scale",
                                            1.0,
                                            0.0,
                                            10.0,
                                            0x80u,
                                            "Scale of the entity information text");
    g_debugPlayerAnimScript = _Dvar_RegisterInt(
                                                            "g_debugPlayerAnimScript",
                                                            -1,
                                                            -1,
                                                            32,
                                                            0,
                                                            "Show debug information for playeranim.script");
    g_motd = _Dvar_RegisterString("g_motd", (char *)"", 0, "The message of the day");
    g_playerCollisionEjectSpeed = _Dvar_RegisterInt(
                                                                    "g_playerCollisionEjectSpeed",
                                                                    25,
                                                                    0,
                                                                    32000,
                                                                    1u,
                                                                    "Speed at which to push intersecting players away from each other");
    g_dropForwardSpeed = _Dvar_RegisterFloat(
                                                 "g_dropForwardSpeed",
                                                 10.0,
                                                 0.0,
                                                 1000.0,
                                                 1u,
                                                 "Forward speed of a dropped item");
    g_dropUpSpeedBase = _Dvar_RegisterFloat(
                                                "g_dropUpSpeedBase",
                                                10.0,
                                                0.0,
                                                1000.0,
                                                1u,
                                                "Base component of the initial vertical speed of a dropped item");
    g_dropUpSpeedRand = _Dvar_RegisterFloat(
                                                "g_dropUpSpeedRand",
                                                5.0,
                                                0.0,
                                                1000.0,
                                                1u,
                                                "Random component of the initial vertical speed of a dropped item");
    g_dropHorzSpeedRand = _Dvar_RegisterFloat(
                                                    "g_dropHorzSpeedRand",
                                                    100.0,
                                                    0.0,
                                                    1000.0,
                                                    1u,
                                                    "Random component of the initial horizontal speed of a dropped item");
    g_clonePlayerMaxVelocity = _Dvar_RegisterFloat(
                                                             "g_clonePlayerMaxVelocity",
                                                             80.0,
                                                             0.0,
                                                             3.4028235e38,
                                                             1u,
                                                             "Maximum velocity in each axis of a cloned player\n(for death animations)");
    voice_global = _Dvar_RegisterBool("voice_global", 0, 1u, "Send voice messages to everybody");
    voice_localEcho = _Dvar_RegisterBool("voice_localEcho", 0, 1u, "Echo voice chat back to the player");
    voice_deadChat = _Dvar_RegisterBool("voice_deadChat", 0, 1u, "Allow dead players to talk to living players");
    g_allowVote = _Dvar_RegisterBool("g_allowVote", 1, 0, "Enable voting on this server");
    g_allow_teamchange = _Dvar_RegisterBool("g_allow_teamchange", 1, 0, "Enable changing teams on this server");
    g_listEntity = _Dvar_RegisterBool("g_listEntity", 0, 0, "List the entities");
    g_listEntityCounts = _Dvar_RegisterEnum(
                                                 "g_listEntityCounts",
                                                 g_entcountNames,
                                                 0,
                                                 0x80u,
                                                 "list all of the current entities counts");
    g_entsInSnapshot = _Dvar_RegisterInt(
                                             "g_entsInSnapshot",
                                             0,
                                             0,
                                             0x7FFFFFFF,
                                             0x80u,
                                             "the number of ents in the snapshot (read only)");
    g_maxEntsInSnapshot = _Dvar_RegisterInt(
                                                    "g_maxEntsInSnapshot",
                                                    0,
                                                    0,
                                                    0x7FFFFFFF,
                                                    0x80u,
                                                    "the high water mark number of ents in all snapshots (read only)");
    g_deadChat = _Dvar_RegisterBool("g_deadChat", 0, 1u, "Allow dead players to chat with living players");
    g_voiceChatTalkingDuration = _Dvar_RegisterInt(
                                                                 "g_voiceChatTalkingDuration",
                                                                 500,
                                                                 0,
                                                                 10000,
                                                                 1u,
                                                                 "Time after the last talk packet was received that the player is considered by the\n"
                                                                 "server to still be talking in milliseconds");
    g_TeamIcon_Allies = _Dvar_RegisterString(
                                                "g_TeamIcon_Allies",
                                                "faction_128_usmc",
                                                0x100u,
                                                "Shader name for the allied scores banner");
    g_TeamIcon_Axis = _Dvar_RegisterString(
                                            "g_TeamIcon_Axis",
                                            "faction_128_arab",
                                            0x100u,
                                            "Shader name for the axis scores banner");
    g_TeamIcon_Free = _Dvar_RegisterString(
                                            "g_TeamIcon_Free",
                                            (char *)"",
                                            0x100u,
                                            "Shader name for the scores of players with no team");
    g_TeamIcon_Spectator = _Dvar_RegisterString(
                                                     "g_TeamIcon_Spectator",
                                                     (char *)"",
                                                     0x100u,
                                                     "Shader name for the scores of players who are spectators");
    g_ScoresColor_MyTeam = _Dvar_RegisterColor(
                                                     "g_ScoresColor_MyTeam",
                                                     0.25,
                                                     0.72000003,
                                                     0.25,
                                                     1.0,
                                                     0x100u,
                                                     "Player team color on scoreboard");
    g_ScoresColor_EnemyTeam = _Dvar_RegisterColor(
                                                            "g_ScoresColor_EnemyTeam",
                                                            0.69,
                                                            0.07,
                                                            0.050000001,
                                                            1.0,
                                                            0x100u,
                                                            "Enemy team color on scoreboard");
    g_ScoresColor_Spectator = _Dvar_RegisterColor(
                                                            "g_ScoresColor_Spectator",
                                                            0.25,
                                                            0.25,
                                                            0.25,
                                                            1.0,
                                                            0x100u,
                                                            "Spectator team color on scoreboard");
    g_ScoresColor_Free = _Dvar_RegisterColor(
                                                 "g_ScoresColor_Free",
                                                 0.75999999,
                                                 0.77999997,
                                                 0.1,
                                                 1.0,
                                                 0x100u,
                                                 "Free Team color on scoreboard");
    g_ScoresColor_Allies = _Dvar_RegisterColor(
                                                     "g_ScoresColor_Allies",
                                                     0.090000004,
                                                     0.46000001,
                                                     0.07,
                                                     1.0,
                                                     0x100u,
                                                     "Allies team color on scoreboard");
    g_ScoresColor_Axis = _Dvar_RegisterColor(
                                                 "g_ScoresColor_Axis",
                                                 0.69,
                                                 0.07,
                                                 0.050000001,
                                                 1.0,
                                                 0x100u,
                                                 "Axis team color on scoreboard");
    g_ScoresPing_Interval = _Dvar_RegisterInt(
                                                        "cg_ScoresPing_Interval",
                                                        1,
                                                        1,
                                                        1,
                                                        0x40u,
                                                        "Number of milliseconds each bar represents");
    g_TeamName_Allies = _Dvar_RegisterString("g_TeamName_Allies", "GAME_ALLIES", 0x100u, "Allied team name");
    g_TeamName_Axis = _Dvar_RegisterString("g_TeamName_Axis", "GAME_AXIS", 0x100u, "Axis team name");
    g_TeamColor_Allies = _Dvar_RegisterColor(
                                                 "g_TeamColor_Allies",
                                                 0.60000002,
                                                 0.63999999,
                                                 0.69,
                                                 1.0,
                                                 0x100u,
                                                 "Allies team color");
    g_TeamColor_Axis = _Dvar_RegisterColor(
                                             "g_TeamColor_Axis",
                                             0.64999998,
                                             0.56999999,
                                             0.41,
                                             1.0,
                                             0x100u,
                                             "Axis team color");
    g_TeamColor_MyTeam = _Dvar_RegisterColor(
                                                 "g_TeamColor_MyTeam",
                                                 0.40000001,
                                                 0.69999999,
                                                 0.40000001,
                                                 1.0,
                                                 0x100u,
                                                 "Player team color");
    g_TeamColor_EnemyTeam = _Dvar_RegisterColor(
                                                        "g_TeamColor_EnemyTeam",
                                                        1.0,
                                                        0.315,
                                                        0.34999999,
                                                        1.0,
                                                        0x100u,
                                                        "Enemy team color");
    g_TeamColor_MyTeamAlt = _Dvar_RegisterColor(
                                                        "g_TeamColor_MyTeamAlt",
                                                        0.34999999,
                                                        1.0,
                                                        1.0,
                                                        1.0,
                                                        0x100u,
                                                        "Player team color");
    g_TeamColor_EnemyTeamAlt = _Dvar_RegisterColor(
                                                             "g_TeamColor_EnemyTeamAlt",
                                                             1.0,
                                                             0.5,
                                                             0.0,
                                                             1.0,
                                                             0x100u,
                                                             "Enemy team color");
    g_TeamColor_Squad = _Dvar_RegisterColor("g_TeamColor_Squad", 0.25, 0.25, 0.75, 1.0, 0x100u, "Squad color");
    g_TeamColor_Spectator = _Dvar_RegisterColor(
                                                        "g_TeamColor_Spectator",
                                                        0.25,
                                                        0.25,
                                                        0.25,
                                                        1.0,
                                                        0x100u,
                                                        "Spectator team color");
    g_TeamColor_Free = _Dvar_RegisterColor("g_TeamColor_Free", 0.75, 0.25, 0.25, 1.0, 0x100u, "Free Team color");
    g_debugLocDamage = _Dvar_RegisterInt(
                                             "g_debugLocDamage",
                                             0,
                                             0,
                                             2,
                                             0x80u,
                                             "Turn on debugging information for locational damage (2 = show results of bullet trace pose only)");
    g_debugLocHit = _Dvar_RegisterInt(
                                        "g_debugLocHit",
                                        0,
                                        0,
                                        2,
                                        0x80u,
                                        "Display locational damage info for an entity when the entity is hit");
    g_debugLocHitTime = _Dvar_RegisterInt(
                                                "g_debugLocHitTime",
                                                500,
                                                0,
                                                0x7FFFFFFF,
                                                0x80u,
                                                "Time duration of g_debugLocHit lines");
    g_smoothClients = _Dvar_RegisterBool("g_smoothClients", 1, 0, "Enable extrapolation between client states");
    g_antilag = _Dvar_RegisterBool("g_antilag", 1, 0x40u, "Turn on antilag checks for weapon hits");
    g_oldVoting = _Dvar_RegisterBool("g_oldVoting", 1, 1u, "Use old voting method");
    g_voteAbstainWeight = _Dvar_RegisterFloat(
                                                    "g_voteAbstainWeight",
                                                    0.5,
                                                    0.0,
                                                    1.0,
                                                    1u,
                                                    "How much an abstained vote counts as a 'no' vote");
    g_NoScriptSpam = _Dvar_RegisterBool("g_no_script_spam", 0, 0, "Turn off script debugging info");
    g_friendlyfireDist = _Dvar_RegisterFloat(
                                                 "g_friendlyfireDist",
                                                 256.0,
                                                 0.0,
                                                 15000.0,
                                                 0x80u,
                                                 "Maximum range for disabling fire at a friendly");
    g_friendlyNameDist = _Dvar_RegisterFloat(
                                                 "g_friendlyNameDist",
                                                 15000.0,
                                                 0.0,
                                                 15000.0,
                                                 0x80u,
                                                 "Maximum range for seeing a friendly's name");
    melee_debug = _Dvar_RegisterBool("melee_debug", 0, 0x80u, "Turn on debug lines for melee traces");
    radius_damage_debug = _Dvar_RegisterBool(
                                                    "radius_damage_debug",
                                                    0,
                                                    0x80u,
                                                    "Turn on debug lines for radius damage traces");
    player_throwbackInnerRadius = _Dvar_RegisterFloat(
                                                                    "player_throwbackInnerRadius",
                                                                    90.0,
                                                                    0.0,
                                                                    3.4028235e38,
                                                                    0x80u,
                                                                    "The radius to a live grenade player must be within initially to do a throwback");
    player_throwbackOuterRadius = _Dvar_RegisterFloat(
                                                                    "player_throwbackOuterRadius",
                                                                    160.0,
                                                                    0.0,
                                                                    3.4028235e38,
                                                                    0x80u,
                                                                    "The radius player is allow to throwback a grenade once the player has been in the inner radius");
    player_useRadius = _Dvar_RegisterFloat(
                                             "player_useRadius",
                                             128.0,
                                             0.0,
                                             3.4028235e38,
                                             0x3080u,
                                             "The radius within which a player can use things");
    player_MGUseRadius = _Dvar_RegisterFloat(
                                                 "player_MGUseRadius",
                                                 128.0,
                                                 0.0,
                                                 3.4028235e38,
                                                 0x80u,
                                                 "The radius within which a player can mount a machine gun");
    vehicle_useRadius = _Dvar_RegisterFloat(
                                                "vehicle_useRadius",
                                                256.0,
                                                0.0,
                                                3.4028235e38,
                                                0x80u,
                                                "The radius within which a player can enter a vehicle");
    g_minGrenadeDamageSpeed = _Dvar_RegisterFloat(
                                                            "g_minGrenadeDamageSpeed",
                                                            400.0,
                                                            0.0,
                                                            3.4028235e38,
                                                            0x80u,
                                                            "Minimum speed at which getting hit be a grenade will do damage (not the grenade explosion damage)");
    g_compassShowEnemies = _Dvar_RegisterBool(
                                                     "g_compassShowEnemies",
                                                     0,
                                                     0x80u,
                                                     "Whether enemies are visible on the compass at all times");
    pickupPrints = _Dvar_RegisterBool(
                                     "pickupPrints",
                                     0,
                                     0x80u,
                                     "Print a message to the game window when picking up ammo, etc.");
    g_revive = _Dvar_RegisterBool("g_revive", 0, 0, "Enable revive");
    g_dumpAnims = _Dvar_RegisterInt(
                                    "g_dumpAnims",
                                    -1,
                                    -1,
                                    1023,
                                    0x80u,
                                    "Animation debugging info for the given character number");
    g_useholdtime = _Dvar_RegisterInt(
                                        "g_useholdtime",
                                        250,
                                        0,
                                        0x7FFFFFFF,
                                        0,
                                        "Time to hold the 'use' button to activate use");
    g_useholdspawndelay = _Dvar_RegisterInt(
                                                    "g_useholdspawndelay",
                                                    500,
                                                    0,
                                                    1000,
                                                    0x81u,
                                                    "Time in milliseconds that the player is unable to 'use' after spawning");
    g_redCrosshairs = _Dvar_RegisterBool("g_redCrosshairs", 1, 0x21u, "Whether red crosshairs are enabled");
    g_mantleBlockTimeBuffer = _Dvar_RegisterInt(
                                                            "g_mantleBlockTimeBuffer",
                                                            500,
                                                            0,
                                                            60000,
                                                            0x80u,
                                                            "Time that the client think is delayed after mantling");
    g_vehicleDebug = _Dvar_RegisterBool("g_vehicleDebug", 0, 0x80u, "Turn on debug information for vehicles");
    vehGunnerSplashDamage = _Dvar_RegisterFloat(
                                                        "vehGunnerSplashDamage",
                                                        0.1,
                                                        0.0,
                                                        3.4028235e38,
                                                        0x80u,
                                                        "Percentage of projectile and grenade splash damage that vehicle gunners take.");
    turretPlayerAvoidScale = _Dvar_RegisterFloat(
                                                         "turretPlayerAvoidScale",
                                                         1.7,
                                                         0.0,
                                                         3.4028235e38,
                                                         0x80u,
                                                         "Auto turrets will try to avoid the player.    They will not choose a target that is within a "
                                                         "cone around the player.    The diameter of the cone is the player's height, so the cone is sm"
                                                         "aller, the farther the player is from the turret.    Use this dvar to scale the cone size.");
    g_enableAttachWeaponFix = _Dvar_RegisterBool(
                                                            "g_enableAttachWeaponFix",
                                                            1,
                                                            0,
                                                            "Enable fix for attach weapons being out of sync on server/client");
    anim_deltas_debug = _Dvar_RegisterBool("anim_deltas_debug", 0, 0, "Enable animation debug data");
    g_destructibleDraw = _Dvar_RegisterBool("g_destructibleDraw", 0, 0x80u, "Render destructible debug info");
    g_debugServerAiming = _Dvar_RegisterBool(
                                                    "g_debugServerAiming",
                                                    0,
                                                    0x80u,
                                                    "Render where the server thinks the client is aiming");
    Helicopter_RegisterDvars();
    Turret_RegisterDvars();
    G_RegisterMissileDvars();
    G_RegisterMissileDebugDvars();
    Turret_PlaceTurret_RegisterDvars();
    DynEntSv_RegisterDvars();
    G_RegisterConnectPaths();
    BG_RegisterDvars();
    g_fogColorReadOnly = _Dvar_RegisterColor(
                                                 "g_fogColorReadOnly",
                                                 1.0,
                                                 0.0,
                                                 0.0,
                                                 1.0,
                                                 0x10C0u,
                                                 "Fog color that was set in the most recent call to \"setexpfog\"");
    g_fogStartDistReadOnly = _Dvar_RegisterFloat(
                                                         "g_fogStartDistReadOnly",
                                                         0.0,
                                                         0.0,
                                                         3.4028235e38,
                                                         0x10C0u,
                                                         "Fog start distance that was set in the most recent call to \"setexpfog\"");
    g_fogHalfDistReadOnly = _Dvar_RegisterFloat(
                                                        "g_fogHalfDistReadOnly",
                                                        0.1,
                                                        0.0,
                                                        3.4028235e38,
                                                        0x10C0u,
                                                        "Fog start distance that was set in the most recent call to \"setexpfog\"");
    vehPlaneRollDeadZone = _Dvar_RegisterFloat(
                                                     "vehPlaneRollDeadZone",
                                                     0.15000001,
                                                     0.0,
                                                     1.0,
                                                     0x80u,
                                                     "Roll input dead zone. Percentage of stick movement to ignore for roll.");
    vehPlaneRollAccel = _Dvar_RegisterFloat(
                                                "vehPlaneRollAccel",
                                                16.0,
                                                0.0,
                                                3.4028235e38,
                                                0x80u,
                                                "Adjustable rotation scaler.");
    vehPlanePitchAccel = _Dvar_RegisterFloat(
                                                 "vehPlanePitchAccel",
                                                 16.0,
                                                 0.0,
                                                 3.4028235e38,
                                                 0x80u,
                                                 "Adjustable rotation scaler.");
    vehPlaneYawSpeed = _Dvar_RegisterFloat(
                                             "vehPlaneYawSpeed",
                                             50.0,
                                             0.0,
                                             3.4028235e38,
                                             0x80u,
                                             "Adjustable rotation scaler.");
    vehPlaneYawFromRollScale = _Dvar_RegisterFloat(
                                                             "vehPlaneYawFromRollScale",
                                                             0.1,
                                                             0.0,
                                                             3.4028235e38,
                                                             0x80u,
                                                             "Adjustable rotation scaler.");
    vehPlaneLiftForce = _Dvar_RegisterFloat("vehPlaneLiftForce", 0.0, 0.0, 3.4028235e38, 0x80u, "Lift force on aircraft.");
    vehPlaneFakeLiftForce = _Dvar_RegisterFloat(
                                                        "vehPlaneFakeLiftForce",
                                                        0.0,
                                                        0.0,
                                                        3.4028235e38,
                                                        0x80u,
                                                        "Fake lift force on aircraft.    Adds extra pitch when non-horizontal.");
    vehPlaneLowSpeed = _Dvar_RegisterFloat("vehPlaneLowSpeed", 1500.0, 0.0, 3.4028235e38, 0x80u, "Min speed.");
    vehPlaneGravityForce = _Dvar_RegisterFloat(
                                                     "vehPlaneGravityForce",
                                                     0.0,
                                                     0.0,
                                                     3.4028235e38,
                                                     0x80u,
                                                     "Gravity force on aircraft.");
    vehicle_switch_seat_delay = _Dvar_RegisterFloat(
                                                                "vehicle_switch_seat_delay",
                                                                0.5,
                                                                0.0,
                                                                3.4028235e38,
                                                                0x80u,
                                                                "Delay before player can switch seats again.");
    vehicle_damage_max_shielding = _Dvar_RegisterFloat(
                                                                     "vehicle_damage_max_shielding",
                                                                     0.25,
                                                                     0.0,
                                                                     1.0,
                                                                     0x80u,
                                                                     "Percent of core damage that armor can shield.");
    vehicle_damage_zone_front = _Dvar_RegisterFloat(
                                                                "vehicle_damage_zone_front",
                                                                1.0,
                                                                0.0,
                                                                3.4028235e38,
                                                                0x80u,
                                                                "Front zone damage for vehicles.");
    vehicle_damage_zone_side = _Dvar_RegisterFloat(
                                                             "vehicle_damage_zone_side",
                                                             1.2,
                                                             0.0,
                                                             3.4028235e38,
                                                             0x80u,
                                                             "Side zone damage for vehicles.");
    vehicle_damage_zone_rear = _Dvar_RegisterFloat(
                                                             "vehicle_damage_zone_rear",
                                                             2.0,
                                                             0.0,
                                                             3.4028235e38,
                                                             0x80u,
                                                             "Rear zone damage for vehicles.");
    vehicle_damage_zone_under = _Dvar_RegisterFloat(
                                                                "vehicle_damage_zone_under",
                                                                2.25,
                                                                0.0,
                                                                3.4028235e38,
                                                                0x80u,
                                                                "Bottom zone damage for vehicles.");
    vehicle_damage_bullet = _Dvar_RegisterFloat(
                                                        "vehicle_damage_bullet",
                                                        0.0,
                                                        0.0,
                                                        3.4028235e38,
                                                        0x80u,
                                                        "Bullet damage for vehicles.");
    vehicle_damage_grenade = _Dvar_RegisterFloat(
                                                         "vehicle_damage_grenade",
                                                         0.30000001,
                                                         0.0,
                                                         3.4028235e38,
                                                         0x80u,
                                                         "Grenade damage for vehicles.");
    vehicle_damage_projectile = _Dvar_RegisterFloat(
                                                                "vehicle_damage_projectile",
                                                                1.2,
                                                                0.0,
                                                                3.4028235e38,
                                                                0x80u,
                                                                "Projectile damage for vehicles.");
    vehicle_damage_bouncing_betty = _Dvar_RegisterFloat(
                                                                        "vehicle_damage_bouncing_betty",
                                                                        1.25,
                                                                        0.0,
                                                                        3.4028235e38,
                                                                        0x80u,
                                                                        "Bouncing betty damage for vehicles.");
    vehicle_damage_satchel_charge = _Dvar_RegisterFloat(
                                                                        "vehicle_damage_satchel_charge",
                                                                        1.5,
                                                                        0.0,
                                                                        3.4028235e38,
                                                                        0x80u,
                                                                        "Satchel charge damage for vehicles.");
    vehicle_damage_sticky_grenade = _Dvar_RegisterFloat(
                                                                        "vehicle_damage_sticky_grenade",
                                                                        1.35,
                                                                        0.0,
                                                                        3.4028235e38,
                                                                        0x80u,
                                                                        "Sticky grenade damage for vehicles.");
    vehicle_piece_damagesfx_threshold = _Dvar_RegisterFloat(
                                                                                "vehicle_piece_damagesfx_threshold",
                                                                                10.0,
                                                                                0.0,
                                                                                3.4028235e38,
                                                                                0x80u,
                                                                                "Minimum amount of damage for which a destructible piece damageSound SFX will be played.");
    vehicle_destructible_damage_grenade = _Dvar_RegisterFloat(
                                                                                    "vehicle_destructible_damage_grenade",
                                                                                    0.2,
                                                                                    0.0,
                                                                                    3.4028235e38,
                                                                                    0x80u,
                                                                                    "Grenade damage for destructible armor on vehicles.");
    vehicle_destructible_damage_bouncing_betty = _Dvar_RegisterFloat(
                                                                                                 "vehicle_destructible_damage_bouncing_betty",
                                                                                                 1.0,
                                                                                                 0.0,
                                                                                                 3.4028235e38,
                                                                                                 0x80u,
                                                                                                 "Bouncing betty damage for destructible armor on vehicles.");
    vehicle_destructible_damage_satchel_charge = _Dvar_RegisterFloat(
                                                                                                 "vehicle_destructible_damage_satchel_charge",
                                                                                                 1.5,
                                                                                                 0.0,
                                                                                                 3.4028235e38,
                                                                                                 0x80u,
                                                                                                 "Satchel charge damage for destructible armor on vehicles.");
    vehicle_destructible_damage_sticky_grenade = _Dvar_RegisterFloat(
                                                                                                 "vehicle_destructible_damage_sticky_grenade",
                                                                                                 1.5,
                                                                                                 0.0,
                                                                                                 3.4028235e38,
                                                                                                 0x80u,
                                                                                                 "Sticky grenade damage for destructible armor on vehicles.");
    vehicle_destructible_damage_grenade_radius = _Dvar_RegisterFloat(
                                                                                                 "vehicle_destructible_damage_grenade_radius",
                                                                                                 100.0,
                                                                                                 0.0,
                                                                                                 3.4028235e38,
                                                                                                 0x80u,
                                                                                                 "Radius for grenade damage for destructible armor on vehicles.");
    vehicle_destructible_damage_bouncing_betty_radius = _Dvar_RegisterFloat(
                                                                                                                "vehicle_destructible_damage_bouncing_betty_radius",
                                                                                                                13.1,
                                                                                                                0.0,
                                                                                                                3.4028235e38,
                                                                                                                0x80u,
                                                                                                                "Radius for bouncing betty damage for destructible armor on vehicles.");
    vehicle_destructible_damage_satchel_charge_radius = _Dvar_RegisterFloat(
                                                                                                                "vehicle_destructible_damage_satchel_charge_radius",
                                                                                                                21.1,
                                                                                                                0.0,
                                                                                                                3.4028235e38,
                                                                                                                0x80u,
                                                                                                                "Radius for satchel charge damage for destructible armor on vehicles.");
    vehicle_destructible_damage_sticky_grenade_radius = _Dvar_RegisterFloat(
                                                                                                                "vehicle_destructible_damage_sticky_grenade_radius",
                                                                                                                13.1,
                                                                                                                0.0,
                                                                                                                3.4028235e38,
                                                                                                                0x80u,
                                                                                                                "Radius for sticky grenade damage for destructible armor on vehicles.");
    vehicle_destructible_damage_projectile_radius = _Dvar_RegisterFloat(
                                                                                                        "vehicle_destructible_damage_projectile_radius",
                                                                                                        4.0,
                                                                                                        0.0,
                                                                                                        3.4028235e38,
                                                                                                        0x80u,
                                                                                                        "Radius for projectile damage for destructible armor on vehicles.");
    vehicle_perk_leadfoot_speed_increase = _Dvar_RegisterFloat(
                                                                                     "vehicle_perk_leadfoot_speed_increase",
                                                                                     1.35,
                                                                                     0.0,
                                                                                     3.4028235e38,
                                                                                     0,
                                                                                     "Vehicle perk leadfoot speed increase percentage.");
    g_turretServerPitchMin = _Dvar_RegisterFloat(
                                                         "g_turretServerPitchMin",
                                                         15.0,
                                                         -50.0,
                                                         50.0,
                                                         0x80u,
                                                         "Limit turret pitch range on server (visual only)");
    g_turretServerPitchMax = _Dvar_RegisterFloat(
                                                         "g_turretServerPitchMax",
                                                         10.0,
                                                         -50.0,
                                                         50.0,
                                                         0x80u,
                                                         "Limit turret pitch range on server (visual only)");
    g_turretBipodOffset = _Dvar_RegisterFloat(
        "g_turretBipodOffset",
        17.0,
        -50.0,
        50.0,
        0x80u,
        "Offset bipod mount position on gun by this distance");
}

//void(__cdecl *CreateDObj)(struct DObjModel_s *, unsigned __int16, XAnimTree_s *, int, int, clientInfo_t *);
void __cdecl G_CreateDObj(
                DObjModel_s *dobjModels,
                unsigned __int16 numModels,
                XAnimTree_s *tree,
                int handle,
                int unusedLocalClientNum,
                clientInfo_t *__formal)
{
    if ( unusedLocalClientNum != -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    1600,
                    0,
                    "unusedLocalClientNum == INVALID_CLIENT_NUMBER\n\t%i, %i",
                    unusedLocalClientNum,
                    -1) )
    {
        __debugbreak();
    }
    Com_ServerDObjCreate(dobjModels, numModels, tree, handle);
}

DObj *__cdecl G_GetDObj(unsigned int handle, int unusedLocalClientNum)
{
    if ( unusedLocalClientNum != -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    1607,
                    0,
                    "unusedLocalClientNum == INVALID_CLIENT_NUMBER\n\t%i, %i",
                    unusedLocalClientNum,
                    -1) )
    {
        __debugbreak();
    }
    return Com_GetServerDObj(handle);
}

void __cdecl G_SafeDObjFree(unsigned int handle, int unusedLocalClientNum)
{
    if ( unusedLocalClientNum != -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    1614,
                    0,
                    "unusedLocalClientNum == INVALID_CLIENT_NUMBER\n\t%i, %i",
                    unusedLocalClientNum,
                    -1) )
    {
        __debugbreak();
    }
    Com_SafeServerDObjFree(handle);
}

void G_LoadAnimTreeInstances()
{
    XAnim_s *anims; // [esp+8h] [ebp-4h]

    for ( int i = 0; i < com_maxclients->current.integer; ++i )
        level_bgs.clientinfo[i].pXAnimTree = XAnimCreateTree(level_bgs.animData->generic_human.tree.anims, Hunk_AllocXAnimServer);
    for ( int i = 0; i < 4; ++i )
        g_scr_data.playerCorpseInfo[i].tree = XAnimCreateTree(level_bgs.animData->generic_human.tree.anims, Hunk_AllocXAnimServer);

    anims = Dog_GetAnims();
    iassert(anims);

    for ( int i = 0; i < 8; ++i )
    {
        g_scr_data.actorCorpseInfo[i].tree = XAnimCreateTree(anims, (void* (*)(unsigned int))Hunk_AllocActorXAnimServer);
        g_scr_data.actorCorpseInfo[i].entnum = -1;
    }
}

void *__cdecl Hunk_AllocActorXAnimServer(int size)
{
    return Hunk_AllocLow(size, "Hunk_AllocActorXAnimServer", 5);
}

void G_PrintAllFastFileErrors()
{
    if ( !sv_mapname
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp", 1852, 0, "%s", "sv_mapname") )
    {
        __debugbreak();
    }
    G_PrintFastFileErrors((char*)"code_post_gfx_mp");
    G_PrintFastFileErrors((char*)"common_mp");
    G_PrintFastFileErrors((char *)sv_mapname->current.integer);
}

void __cdecl G_PrintFastFileErrors(char *fastfile)
{
    RawFile *rawfile; // [esp+4h] [ebp-4h]

    if ( !fastfile
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp", 1836, 0, "%s", "fastfile") )
    {
        __debugbreak();
    }
    rawfile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, fastfile, 1, -1).rawfile;
    if ( !rawfile
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp", 1839, 1, "%s", "rawfile") )
    {
        __debugbreak();
    }
    if ( rawfile->len )
    {
        Com_PrintError(1, "There were errors when building fast file '%s'\n", fastfile);
        Com_PrintError(1, (char *)rawfile->buffer);
    }
}

unsigned __int16 __cdecl G_AttachWeapon(DObjModel_s *dobjModels, unsigned __int16 numModels, clientInfo_t *ci)
{
    int oldLeftHand; // [esp+4h] [ebp-Ch]
    unsigned __int8 weaponModel; // [esp+Bh] [ebp-5h]
    const WeaponDef *weapDef; // [esp+Ch] [ebp-4h]
    const WeaponDef *weapDefa; // [esp+Ch] [ebp-4h]

    if ( ci->iDObjWeapon )
    {
        weapDef = BG_GetWeaponDef(ci->iDObjWeapon);
        weaponModel = ci->weaponModel;
        if ( g_enableAttachWeaponFix->current.enabled )
        {
            if ( weapDef->worldModel[weaponModel] && !weapDef->bHideThirdPerson )
            {
                if ( numModels >= 0x20u
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                                1898,
                                0,
                                "%s",
                                "numModels < DOBJ_MAX_SUBMODELS") )
                {
                    __debugbreak();
                }
                dobjModels[numModels].model = weapDef->worldModel[weaponModel];
                dobjModels[numModels].boneName = G_GetWeaponAttachBone(ci, weapDef->weapType, weapDef->inventoryType);
                dobjModels[numModels++].ignoreCollision = 1;
            }
        }
        else if ( weapDef->worldModel[weaponModel] )
        {
            if ( numModels >= 0x20u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                            1911,
                            0,
                            "%s",
                            "numModels < DOBJ_MAX_SUBMODELS") )
            {
                __debugbreak();
            }
            dobjModels[numModels].model = weapDef->worldModel[weaponModel];
            dobjModels[numModels].boneName = G_GetWeaponAttachBone(ci, weapDef->weapType, weapDef->inventoryType);
            dobjModels[numModels++].ignoreCollision = 1;
        }
        if ( weapDef->bDualWield && !ci->usingKnife )
        {
            weapDef = BG_GetWeaponDef(weapDef->dualWieldWeaponIndex);
            if ( weapDef->worldModel[weaponModel] )
            {
                if ( numModels >= 0x20u
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                                1929,
                                0,
                                "%s",
                                "numModels < DOBJ_MAX_SUBMODELS") )
                {
                    __debugbreak();
                }
                dobjModels[numModels].model = weapDef->worldModel[weaponModel];
                oldLeftHand = ci->leftHandGun;
                ci->leftHandGun = 1;
                ci->leftHandGun = oldLeftHand;
                dobjModels[numModels].boneName = G_GetWeaponAttachBone(ci, weapDef->weapType, weapDef->inventoryType);
                dobjModels[numModels++].ignoreCollision = 1;
            }
        }
        if ( weapDef->additionalMeleeModel )
        {
            if ( numModels >= 0x20u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                            1944,
                            0,
                            "%s",
                            "numModels < DOBJ_MAX_SUBMODELS") )
            {
                __debugbreak();
            }
            dobjModels[numModels].model = weapDef->additionalMeleeModel;
            dobjModels[numModels].boneName = scr_const.tag_weapon_left;
            dobjModels[numModels++].ignoreCollision = 0;
        }
        else if ( ci->usingKnife )
        {
            if ( ci->iDObjMeleeWeapon )
            {
                weapDefa = BG_GetWeaponDef(ci->iDObjMeleeWeapon);
                if ( weapDefa->worldModel )
                {
                    if ( numModels >= 0x20u
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                                    1957,
                                    0,
                                    "%s",
                                    "numModels < DOBJ_MAX_SUBMODELS") )
                    {
                        __debugbreak();
                    }
                    dobjModels[numModels].model = weapDefa->worldModel[ci->meleeWeaponModel];
                    dobjModels[numModels].boneName = scr_const.tag_weapon_left;
                    dobjModels[numModels++].ignoreCollision = 0;
                }
            }
        }
    }
    return numModels;
}

void __cdecl G_ShutdownGame(int freeScripts)
{
    colgeom_visitor_inlined_t<200> *p_proximity_data; // [esp+0h] [ebp-14h]
    int i; // [esp+Ch] [ebp-8h]
    int file; // [esp+10h] [ebp-4h]

    for ( i = 0; i < 32; ++i )
    {
        p_proximity_data = &g_pmove[i].proximity_data;
        p_proximity_data->reset();
        //colgeom_visitor_inlined_t<500>::reset(p_proximity_data);
    }
    ShutdownRopes();
    GlassSv_Shutdown();
    Com_Printf(15, "==== ShutdownGame (%d) ====\n", freeScripts);
    if ( level.logFile )
    {
        G_LogPrintf("ShutdownGame:\n");
        G_LogPrintf("------------------------------------------------------------\n");
        FS_FCloseFile(level.logFile);
    }
    //*(unsigned int *)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 8) = 0;
    bgs = 0;
    G_FreeEntities(1);
    HudElem_DestroyAll();
    Path_Shutdown();
    G_FreeScrVehicles();
    G_FreeVehiclePaths();
    if ( Scr_IsSystemActive(1u, SCRIPTINSTANCE_SERVER) && !level.savepersist )
        SV_FreeClientScriptPers();
    Scr_ShutdownSystem(SCRIPTINSTANCE_SERVER, 1u, level.savepersist == 0);
    if ( freeScripts )
    {
        Mantle_ShutdownAnims();
        Dog_ShutdownAnims();
        GScr_FreeScripts(SCRIPTINSTANCE_SERVER);
        Scr_FreeScripts(SCRIPTINSTANCE_SERVER, 1u);
        G_FreeAnimTreeInstances();
        Scr_ShutdownAnimTrees(1);
        Com_FreeWeaponInfoMemory(1);
        Hunk_ClearToMarkLow(0);
    }
    for ( file = 0; file < 1; ++file )
    {
        if ( level.openScriptIOFileBuffers[file] )
            Z_VirtualFree(level.openScriptIOFileBuffers[file], 11);
        level.openScriptIOFileBuffers[file] = 0;
        if ( level.openScriptIOFileHandles[file] )
            FS_FCloseFile(level.openScriptIOFileHandles[file]);
        level.openScriptIOFileHandles[file] = 0;
    }
    EntHandle::Shutdown();
    SV_track_shutdown();
}

void __cdecl CalculateRanks()
{
    unsigned int clientNum; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]
    int ia; // [esp+4h] [ebp-4h]

    level.numConnectedClients = 0;
    level.numVotingClients = 0;
    for ( i = 0; i < level.maxclients; ++i )
    {
        if ( level.clients[i].sess.connected )
        {
            level.sortedClients[level.numConnectedClients++] = i;
            if ( level.clients[i].sess.cs.team != TEAM_SPECTATOR && level.clients[i].sess.connected == CON_CONNECTED )
                ++level.numVotingClients;
        }
    }
    qsort(level.sortedClients, level.numConnectedClients, 4u, (int (__cdecl *)(const void *, const void *))SortRanks);
    for ( ia = 0; ia < level.numConnectedClients; ++ia )
    {
        clientNum = level.sortedClients[ia];
        if ( clientNum >= level.maxclients
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                        2613,
                        0,
                        "clientNum doesn't index level.maxclients\n\t%i not in [0, %i)",
                        clientNum,
                        level.maxclients) )
        {
            __debugbreak();
        }
        level.clients[clientNum].sess.cs.score.place = ia + 1;
    }
    level.bUpdateScoresForIntermission = 1;
}

int __cdecl SortRanks(int *a, int *b)
{
    gclient_s *cb; // [esp+0h] [ebp-8h]
    gclient_s *ca; // [esp+4h] [ebp-4h]

    if ( (unsigned int)*a >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    2534,
                    0,
                    "*(int *)a doesn't index level.maxclients\n\t%i not in [0, %i)",
                    *a,
                    level.maxclients) )
    {
        __debugbreak();
    }
    if ( (unsigned int)*b >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    2535,
                    0,
                    "*(int *)b doesn't index level.maxclients\n\t%i not in [0, %i)",
                    *b,
                    level.maxclients) )
    {
        __debugbreak();
    }
    ca = &level.clients[*a];
    cb = &level.clients[*b];
    if ( ca->sess.connected == CON_CONNECTING )
        return 1;
    if ( cb->sess.connected == CON_CONNECTING )
        return -1;
    if ( ca->sess.cs.team == TEAM_SPECTATOR && cb->sess.cs.team == TEAM_SPECTATOR )
    {
        if ( ca >= cb )
            return ca > cb;
        else
            return -1;
    }
    else if ( ca->sess.cs.team == TEAM_SPECTATOR )
    {
        return 1;
    }
    else if ( cb->sess.cs.team == TEAM_SPECTATOR )
    {
        return -1;
    }
    else if ( ca->sess.cs.score.score <= cb->sess.cs.score.score )
    {
        if ( ca->sess.cs.score.score >= cb->sess.cs.score.score )
        {
            if ( ca->sess.cs.score.deaths >= cb->sess.cs.score.deaths )
                return ca->sess.cs.score.deaths > cb->sess.cs.score.deaths;
            else
                return -1;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return -1;
    }
}

void __cdecl ExitLevel()
{
    int i; // [esp+0h] [ebp-4h]

    Cbuf_AddText(0, "map_rotate\n");
    for ( i = 0; i < level.maxclients; ++i )
    {
        if ( level.clients[i].sess.connected == CON_CONNECTED )
            level.clients[i].sess.connected = CON_CONNECTING;
    }
    G_LogPrintf("ExitLevel: executed\n");
}

void G_LogPrintf(const char *fmt, ...)
{
    char string[1024]; // [esp+10h] [ebp-818h] BYREF
    char *argptr; // [esp+410h] [ebp-418h]
    int tens; // [esp+414h] [ebp-414h]
    char string2[1024]; // [esp+418h] [ebp-410h] BYREF
    int min; // [esp+81Ch] [ebp-Ch]
    int sec; // [esp+820h] [ebp-8h]
    int time1970; // [esp+824h] [ebp-4h]
    va_list va; // [esp+834h] [ebp+Ch] BYREF

    va_start(va, fmt);
    if ( level.logFile )
    {
        _vsnprintf(string2, 0x400u, fmt, va);
        argptr = 0;
        if ( g_logTimeStampInSeconds && g_logTimeStampInSeconds->current.enabled )
        {
            time1970 = Com_RealTime(0, 1);
            Com_sprintf(string, 0x400u, "%d %s", time1970, string2);
        }
        else
        {
            min = level.time / 1000 / 60;
            tens = level.time / 1000 % 60 / 10;
            sec = level.time / 1000 % 60 % 10;
            Com_sprintf(string, 0x400u, "%3i:%i%i %s", min, tens, sec, string2);
        }
        FS_Write(string, &string[strlen(string) + 1] - &string[1], level.logFile);
    }
}

void __cdecl CheckVote()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    int passCount; // [esp+20h] [ebp-4h]

    if ( level.voteExecuteTime )
    {
        if ( level.voteExecuteTime < level.time )
        {
            level.voteExecuteTime = 0;
            v0 = va("%s\n", level.voteString);
            Cbuf_AddText(0, v0);
        }
    }
    if ( level.voteTime )
    {
        if ( level.time - level.voteTime >= 0 )
        {
            if ( level.voteYes <= (int)((float)((float)(level.numVotingClients - (level.voteNo + level.voteYes))
                                                                                * g_voteAbstainWeight->current.value)
                                                                + 0.4999999990686774)
                                                    + level.voteNo )
                goto LABEL_11;
LABEL_9:
            v1 = va("%c \"GAME_VOTEPASSED\"", 101);
            SV_GameSendServerCommand(-1, SV_CMD_CAN_IGNORE, v1);
            level.voteExecuteTime = level.time + 3000;
LABEL_13:
            level.voteTime = 0;
            SV_SetConfigstring(15, (char *)"");
            return;
        }
        passCount = level.numVotingClients / 2 + 1;
        if ( level.voteYes >= passCount )
            goto LABEL_9;
        if ( level.voteNo > level.numVotingClients - passCount )
        {
LABEL_11:
            v2 = va("%c \"GAME_VOTEFAILED\"", 101);
            SV_GameSendServerCommand(-1, SV_CMD_CAN_IGNORE, v2);
            goto LABEL_13;
        }
    }
}

void __cdecl G_UpdateObjectiveToClients()
{
    objective_t *obj; // [esp+8h] [ebp-18h]
    int team; // [esp+Ch] [ebp-14h]
    gentity_s *ent; // [esp+10h] [ebp-10h]
    int clientNum; // [esp+14h] [ebp-Ch]
    int objNum; // [esp+18h] [ebp-8h]
    playerState_s *ps; // [esp+1Ch] [ebp-4h]

    for ( clientNum = 0; clientNum < level.maxclients; ++clientNum )
    {
        ent = &level.gentities[clientNum];
        if ( ent->r.inuse )
        {
            if ( !ent->client
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp", 2810, 0, "%s", "ent->client") )
            {
                __debugbreak();
            }
            ps = &ent->client->ps;
            team = ps[1].corpseIndex;
            for ( objNum = 0; objNum < 32; ++objNum )
            {
                obj = &level.objectives[objNum];
                if ( obj->state
                    && (!obj->teamNum || obj->teamNum == team)
                    && (level.objectivesClientMask[objNum][clientNum >> 5] & (1 << (clientNum & 0x1F))) == 0 )
                {
                    memcpy(&ps->objective[objNum], obj, sizeof(ps->objective[objNum]));
                }
                else
                {
                    ps->objective[objNum].state = OBJST_EMPTY;
                }
            }
        }
    }
}

void __cdecl G_UpdateHudElemsToClients()
{
    gentity_s *ent; // [esp+0h] [ebp-8h]
    int clientNum; // [esp+4h] [ebp-4h]

    for ( clientNum = 0; clientNum < level.maxclients; ++clientNum )
    {
        ent = &level.gentities[clientNum];
        if ( ent->r.inuse )
        {
            if ( !ent->client
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp", 2841, 0, "%s", "ent->client") )
            {
                __debugbreak();
            }
            HudElem_UpdateClient(ent->client, ent->s.number, HUDELEM_UPDATE_ARCHIVAL_AND_CURRENT);
        }
    }
}

void __cdecl G_RunThink(gentity_s *ent)
{
    void (__cdecl *think)(gentity_s *); // [esp+0h] [ebp-8h]
    int thinktime; // [esp+4h] [ebp-4h]

    thinktime = ent->nextthink;
    if ( thinktime > 0 && thinktime <= level.time )
    {
        ent->nextthink = 0;
        think = entityHandlers[ent->handler].think;
        if ( !think )
            Com_Error(ERR_DROP, "NULL ent->think");
        think(ent);
    }
}

void __cdecl DebugDumpAnims()
{
    if ( g_dumpAnims->current.integer >= 0 )
    {
        if ( g_dumpAnims->current.integer >= 1024
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                        2885,
                        1,
                        "%s",
                        "g_dumpAnims->current.integer < MAX_GENTITIES") )
        {
            __debugbreak();
        }
        SV_DObjDisplayAnim(&level.gentities[g_dumpAnims->current.integer], "server:\n");
    }
}

void __cdecl G_XAnimUpdateEnt(gentity_s *ent)
{
    if ( ent->r.inuse )
    {
        if ( (ent->flags & 0x2000) == 0 )
            G_DObjUpdateServerTime(ent, 1, ScriptPump);
    }
}

void __cdecl ScriptPump()
{
    Scr_RunCurrentThreads(SCRIPTINSTANCE_SERVER);
}

int lastEntTime;
void __cdecl ShowEntityInfo()
{
    int v0; // [esp+0h] [ebp-A4h]
    int integer; // [esp+4h] [ebp-A0h]
    float vEnd[3]; // [esp+Ch] [ebp-98h] BYREF
    float vForward[3]; // [esp+54h] [ebp-50h] BYREF
    unsigned __int16 hitEntId; // [esp+60h] [ebp-44h]
    void (__cdecl *entinfo)(gentity_s *, float *); // [esp+64h] [ebp-40h]
    col_context_t context; // [esp+68h] [ebp-3Ch] BYREF
    gentity_s *ent; // [esp+90h] [ebp-14h]
    int i; // [esp+94h] [ebp-10h]
    float vStart[3]; // [esp+98h] [ebp-Ch] BYREF

    //col_context_t::col_context_t(&context);
    CL_GetDebugViewPos(vStart);
    if ( g_radiant_selected_ent )
        misc_EntInfo(g_radiant_selected_ent, vStart);
    if ( g_entinfo->current.integer )
    {
        if ( g_entinfo->current.integer == 2
            || g_entinfo->current.integer == 3
            || g_entinfo->current.integer == 5
            || g_entinfo->current.integer == 7 )
        {
            trace_t trace; // [esp+18h] [ebp-8Ch] BYREF
            CL_GetDebugViewForward(vForward);
            vEnd[0] = (float)(16000.0 * vForward[0]) + vStart[0];
            vEnd[1] = (float)(16000.0 * vForward[1]) + vStart[1];
            vEnd[2] = (float)(16000.0 * vForward[2]) + vStart[2];
            integer = g_entinfo_type->current.integer;
            if ( integer == 1 )
            {
                G_TraceCapsule(&trace, vStart, vec3_origin, vec3_origin, vEnd, 0, 0x8000, &context);
            }
            else if ( integer == 2 )
            {
                G_TraceCapsule(&trace, vStart, vec3_origin, vec3_origin, vEnd, 0, 0x800000, &context);
            }
            else
            {
                G_TraceCapsule(&trace, vStart, vec3_origin, vec3_origin, vEnd, 0, 0x80A080, &context);
            }
            hitEntId = Trace_GetEntityHitId(&trace);
            entinfo = entityHandlers[g_entities[hitEntId].handler].entinfo;
            if ( entinfo )
            {
                Dvar_SetIntByName("ai_debugEntIndex", hitEntId);
                lastEntTime = level.time;
            }
            if ( ai_debugEntIndex->current.integer != -1 && lastEntTime + 30000 > level.time )
            {
                ent = &g_entities[ai_debugEntIndex->current.integer];
                if ( ent->actor && ai_debugCoverEntityNum->current.integer > 0 )
                    Dvar_SetInt((dvar_s *)ai_debugCoverEntityNum, ent->s.number);
                entinfo = entityHandlers[ent->handler].entinfo;
                if ( entinfo )
                    entinfo(ent, vStart);
            }
            return;
        }
        ent = g_entities;
        i = 0;
        while ( 1 )
        {
            if ( i >= level.num_entities )
                return;
            if ( ent->r.inuse && ent->r.linked )
            {
                if ( g_entinfo->current.integer == 6 )
                {
                    if ( (ent->r.svFlags & 1) != 0 || ent->classname != scr_const.script_model && !ent->client )
                        goto LABEL_24;
                }
                else
                {
                    v0 = g_entinfo_type->current.integer;
                    if ( v0 == 1 )
                    {
                        if ( !ent->actor )
                            goto LABEL_24;
                    }
                    else if ( v0 == 2 && !ent->scr_vehicle )
                    {
                        goto LABEL_24;
                    }
                }
                entinfo = entityHandlers[ent->handler].entinfo;
                if ( entinfo )
                    entinfo(ent, vStart);
            }
LABEL_24:
            ++i;
            ++ent;
        }
    }
}

void __cdecl G_UpdateIKPlayerClipTerrainTimeout(gentity_s *ent)
{
    if ( ent->client )
    {
        if ( (ent->client->ps.eFlags2 & 0x10000) != 0 && ent->ikPlayerclipTerrainTime < level.time - 1000 )
            ent->client->ps.eFlags2 &= ~0x10000u;
    }
    else if ( (ent->s.lerp.eFlags2 & 0x10000) != 0 && ent->ikPlayerclipTerrainTime < level.time - 1000 )
    {
        ent->s.lerp.eFlags2 &= ~0x10000u;
    }
}

void __cdecl G_UpdateIKDisableTerrainMappingTimeout(gentity_s *ent)
{
    if ( ent->client )
    {
        if ( (ent->client->ps.eFlags2 & 0x100000) != 0 && ent->ikDisableTerrainMappingTime < level.time - 1000 )
            ent->client->ps.eFlags2 &= ~0x100000u;
    }
    else if ( (ent->s.lerp.eFlags2 & 0x100000) != 0 && ent->ikDisableTerrainMappingTime < level.time - 1000 )
    {
        ent->s.lerp.eFlags2 &= ~0x100000u;
    }
}

static float dir[3];
static float hitPos[3];
void __cdecl G_UpdateTimedDamage(gentity_s *ent)
{
    int max; // [esp+Ch] [ebp-10h]
    float dist; // [esp+10h] [ebp-Ch]
    int i; // [esp+14h] [ebp-8h]
    gentity_s *fireStarterEnt; // [esp+18h] [ebp-4h]

    if ( level.time >= ent->last_timed_radius_damage )
    {
        max = 0;
        for ( i = 0; i < g_timed_radius_damage_count; ++i )
        {
            if ( level.time < g_timed_radius_damage[i].life )
            {
                max = i + 1;
                dist = Vec3DistanceSq(g_timed_radius_damage[i].pos, ent->r.currentOrigin);
                if ( g_timed_radius_damage[i].radiusSqr >= dist )
                {
                    ent->last_timed_radius_damage = g_timed_radius_damage[i].rate + level.time;
                    if ( g_timed_radius_damage[i].fireStarterClientNum < 0x20u )
                    {
                        fireStarterEnt = &g_entities[g_timed_radius_damage[i].fireStarterClientNum];
                        G_Damage(
                            ent,
                            fireStarterEnt,
                            fireStarterEnt,
                            dir,
                            hitPos,
                            g_timed_radius_damage[i].damage,
                            1,
                            g_timed_radius_damage[i].mod,
                            g_timed_radius_damage[i].weapon,
                            HITLOC_NONE,
                            0,
                            0,
                            0);
                    }
                }
            }
        }
        if ( max < g_timed_radius_damage_count )
            g_timed_radius_damage_count = max;
    }
}

void __cdecl G_DebugTimedDamage()
{
    int i; // [esp+14h] [ebp-4h]

    if ( fire_debug->current.enabled )
    {
        for ( i = 0; i < g_timed_radius_damage_count; ++i )
        {
            if ( level.time < g_timed_radius_damage[i].life )
                G_DebugCircle(g_timed_radius_damage[i].pos, g_timed_radius_damage[i].radius, colorRed, 1, 0, 0);
        }
    }
}

void __cdecl G_UpdateClientLinkInfo(gentity_s *ent)
{
    char *v1; // eax
    float *v2; // [esp+4h] [ebp-14h]
    int tagIndex; // [esp+10h] [ebp-8h]
    gentity_s *entParent; // [esp+14h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp", 3486, 0, "%s", "ent") )
        __debugbreak();
    if ( ent->tagChildren )
        ent->s.clientLinkInfo.flags |= 1u;
    else
        ent->s.clientLinkInfo.flags &= ~1u;
    if ( ent )
    {
        if ( ent->tagInfo )
        {
            if ( !ent->tagInfo->parent
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                            3503,
                            0,
                            "%s",
                            "ent->tagInfo->parent") )
            {
                __debugbreak();
            }
            entParent = ent->tagInfo->parent;
            if ( (entParent->r.svFlags & 1) != 0 )
            {
                if ( (float)((float)((float)(entParent->s.lerp.pos.trDelta[0] * entParent->s.lerp.pos.trDelta[0])
                                                     + (float)(entParent->s.lerp.pos.trDelta[1] * entParent->s.lerp.pos.trDelta[1]))
                                     + (float)(entParent->s.lerp.pos.trDelta[2] * entParent->s.lerp.pos.trDelta[2])) != 0.0
                    || (float)((float)((float)(entParent->s.lerp.apos.trDelta[0] * entParent->s.lerp.apos.trDelta[0])
                                                     + (float)(entParent->s.lerp.apos.trDelta[1] * entParent->s.lerp.apos.trDelta[1]))
                                     + (float)(entParent->s.lerp.apos.trDelta[2] * entParent->s.lerp.apos.trDelta[2])) != 0.0 )
                {
                    Com_PrintWarning(
                        15,
                        "Ent #%i of type %i is SVF_NOCLIENT, but has children and seems to be moving.    There is a potential optimization here.\n",
                        ent->s.number,
                        ent->s.eType);
                }
                ent->s.clientLinkInfo.parentEnt = 0;
            }
            else if ( entParent->s.number != 1022 )
            {
                if ( ent->s.clientLinkInfo.parentEnt != ent->tagInfo->parent->s.number + 1 )
                {
                    AssignToSmallerType<short>((__int16 *)&ent->s.clientLinkInfo, ent->tagInfo->parent->s.number + 1);
                    if ( ent->tagInfo->name )
                    {
                        v1 = SL_ConvertToString(ent->tagInfo->name, SCRIPTINSTANCE_SERVER);
                        tagIndex = G_TagIndex(v1);
                        AssignToSmallerType<unsigned char>(&ent->s.clientLinkInfo.tagIndex, tagIndex);
                        ent->s.clientLinkInfo.flags &= ~2u;
                    }
                    else
                    {
                        ent->s.clientLinkInfo.tagIndex = 0;
                        ent->s.clientLinkInfo.flags |= 2u;
                    }
                }
                v2 = ent->tagInfo->axis[3];
                ent->s.lerp.pos.trDelta[0] = *v2;
                ent->s.lerp.pos.trDelta[1] = v2[1];
                ent->s.lerp.pos.trDelta[2] = v2[2];
                AxisToAngles(ent->tagInfo->axis, ent->s.lerp.apos.trDelta);
                ResolveParentClientMask(ent, entParent);
            }
        }
        else
        {
            ent->s.clientLinkInfo.parentEnt = 0;
            ent->s.clientLinkInfo.tagIndex = 0;
        }
    }
}

bool __cdecl ResolveParentClientMask(const gentity_s *entChild, gentity_s *entParent)
{
    int checkBits; // [esp+0h] [ebp-Ch]
    int seg; // [esp+4h] [ebp-8h]
    bool changed; // [esp+Bh] [ebp-1h]

    if ( !entChild
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp", 3455, 0, "%s", "entChild") )
    {
        __debugbreak();
    }
    if ( !entParent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp", 3456, 0, "%s", "entParent") )
    {
        __debugbreak();
    }
    changed = 0;
    for ( seg = 0; seg < 1; ++seg )
    {
        checkBits = entParent->r.clientMask[seg] & ~entChild->r.clientMask[seg];
        if ( checkBits )
        {
            changed = 1;
            entParent->r.clientMask[seg] &= ~checkBits;
        }
    }
    return changed;
}

void    G_RunFrame(int levelTime)
{
    int Time; // eax
    int v3; // eax
    trigger_info_t *v4; // eax
    char *v5; // eax
    gentity_s *other; // [esp+54h] [ebp-424h]
    trigger_info_t *trigger_info; // [esp+58h] [ebp-420h]
    unsigned __int8 index; // [esp+5Fh] [ebp-419h]
    unsigned __int8 entIndex[1028]; // [esp+60h] [ebp-418h] BYREF
    gentity_s *ent; // [esp+468h] [ebp-10h]
    int bMoreTriggered; // [esp+46Ch] [ebp-Ch]
    int i; // [esp+470h] [ebp-8h]
    int entnum; // [esp+474h] [ebp-4h]

    gScrExecuteTime[0] = 0;
    SV_CheckThread();
    ++level.framenum;
    level.previousTime = level.time;
    level.time = levelTime;
    level.frametime = levelTime - level.previousTime;
    level_bgs.time = levelTime;
    level_bgs.latestSnapshotTime = levelTime;
    level_bgs.frametime = levelTime - level.previousTime;
    iassert(bgs == 0);
    bgs = &level_bgs;
    if ( level.frametime < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    3584,
                    0,
                    "%s",
                    "level.frametime >= 0") )
    {
        __debugbreak();
    }

    {
        PROF_SCOPED("Update clients");
        ent = g_entities;
        i = 0;
        while (i < level.maxclients)
        {
            if (ent->r.inuse)
            {
                if (!ent->client
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp", 3600, 0, "%s", "ent->client"))
                {
                    __debugbreak();
                }
                if ((ent->client->flags & 3) == 0)
                    G_DoTouchTriggers(ent);
                G_UpdateWeapons(ent);
                G_UpdateTimedDamage(ent);
                G_UpdateIKPlayerClipTerrainTimeout(ent);
                G_UpdateIKDisableTerrainMappingTimeout(ent);
                G_UpdateIKCulling(ent);
            }
            ++i;
            ++ent;
        }
    }

#ifdef KISAK_LIVE
    if ( onlinegame->current.enabled && com_sv_running->current.enabled )
        MatchRecordMovement();
#endif
    G_DebugTimedDamage();
    Time = G_GetTime();
    SV_Flame_Age_All_Objects(Time);
    v3 = G_GetTime();
    IK_UpdateTimeAll(v3, -1, 0);

    {
        PROF_SCOPED("G_XAnimUpdate");
        ent = g_entities;
        i = 0;
        while (i < level.num_entities)
        {
            if (ent->r.inuse)
                SV_DObjInitServerTime(ent, (float)level.frametime * 0.001);
            ++i;
            ++ent;
        }
    }

    memset(entIndex, 0, 0x400u);
    index = 0;

    {
        PROF_SCOPED("G_TriggerChecks");

        if (level.currentTriggerListSize
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                3654,
                0,
                "%s",
                "level.currentTriggerListSize == 0"))
        {
            __debugbreak();
        }
        Com_Memcpy(level.currentTriggerList, level.pendingTriggerList, 12 * level.pendingTriggerListSize);
        level.currentTriggerListSize = level.pendingTriggerListSize;
        level.pendingTriggerListSize = 0;
        do
        {
            bMoreTriggered = 0;
            ++index;
            for (i = 0; i < level.currentTriggerListSize; ++i)
            {
                trigger_info = &level.currentTriggerList[i];
                entnum = trigger_info->entnum;
                ent = &g_entities[entnum];
                if (ent->useCount == trigger_info->useCount)
                {
                    iassert(ent->r.inuse);
                    other = &g_entities[trigger_info->otherEntnum];
                    if (other->useCount == trigger_info->otherUseCount)
                    {
                        iassert(other->r.inuse);
                        
                        if (entIndex[entnum] == index)
                        {
                            bMoreTriggered = 1;
                            continue;
                        }
                        entIndex[entnum] = index;
                        Scr_AddEntity(other, SCRIPTINSTANCE_SERVER);
                        Scr_Notify(ent, scr_const.trigger, 1u);
                    }
                }
                --level.currentTriggerListSize;
                --i;
                v4 = &level.currentTriggerList[level.currentTriggerListSize];
                *(unsigned int *)&trigger_info->entnum = *(unsigned int *)&v4->entnum;
                trigger_info->useCount = v4->useCount;
                trigger_info->otherUseCount = v4->otherUseCount;
            }

            {
                PROF_SCOPED("Scr_RunCurrentThreads");
                Scr_RunCurrentThreads(SCRIPTINSTANCE_SERVER);
            }
        } while (bMoreTriggered);
        iassert(level.currentTriggerListSize == 0);
    }
    
    ent = g_entities;
    i = 0;
    while ( i < level.maxclients )
    {
        if ( ent->r.inuse )
            G_ClientDoPerFrameNotifies(ent);
        ++i;
        ++ent;
    }

    {
        PROF_SCOPED("G_XAnimUpdate");
        ent = g_entities;
        i = 0;
        while (i < level.num_entities)
        {
            G_XAnimUpdateEnt(ent);
            ++i;
            ++ent;
        }
    }

    Scr_IncTime(SCRIPTINSTANCE_SERVER);
    SV_ResetSkeletonCache();

    {
        PROF_SCOPED("G_RunFrameForEntity");
        if (level.currentEntityThink != -1
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                3730,
                0,
                "%s",
                "level.currentEntityThink == -1"))
        {
            __debugbreak();
        }
        ent = g_entities;
        level.currentEntityThink = 0;
        while (level.currentEntityThink < level.num_entities)
        {
            if (ent->r.inuse)
            {
                G_RunFrameForEntity(ent);
                G_UpdateClientLinkInfo(ent);
            }
            ++level.currentEntityThink;
            ++ent;
        }
        level.currentEntityThink = -1;
    }
    
    {
        PROF_SCOPED("G_UpdateObjectiveToClients");
        G_UpdateObjectiveToClients();
    }
    {
        PROF_SCOPED("G_UpdateHudElemsToClients");
        G_UpdateHudElemsToClients();
    }
    
    {
        PROF_SCOPED("ClientEndFrame");

        ent = g_entities;
        i = 0;
        while (i < level.maxclients)
        {
            if (ent->r.inuse)
            {
                if ((unsigned int)i >= 0x20
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                        3765,
                        0,
                        "i doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        i,
                        32))
                {
                    __debugbreak();
                }
                if (!level_bgs.clientinfo[i].infoValid
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                        3766,
                        0,
                        "%s",
                        "level_bgs.clientinfo[i].infoValid"))
                {
                    __debugbreak();
                }
                if (ent->client - level.clients != i
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                        3767,
                        0,
                        "ent->client - level.clients == i\n\t%i, %i",
                        ent->client - level.clients,
                        i))
                {
                    __debugbreak();
                }
                ClientEndFrame(ent);
            }
            ++i;
            ++ent;
        }
    }
    
    {
        PROF_SCOPED("CheckTeamStatus");
        CheckTeamStatus();
    }
    
    if ( g_oldVoting->current.enabled )
        CheckVote();
    if ( g_listEntity->current.enabled )
    {
        for ( i = 0; i < 1024; ++i )
        {
            v5 = SL_ConvertToString(g_entities[i].classname, SCRIPTINSTANCE_SERVER);
            Com_Printf(15, "%4i: %s\n", i, v5);
        }
        Dvar_SetBool((dvar_s *)g_listEntity, 0);
    }
    if ( level.registerWeapons )
        SaveRegisteredWeapons();
    if ( level.bRegisterItems )
        SaveRegisteredItems();
    SpawnSystem_Update();
    G_PopulateMatchState();
    G_ProcessRadiantCmds();
    G_UpdateActorCorpses();
    Path_Update();
    DebugDumpAnims();

    iassert(bgs == &level_bgs);
    bgs = 0;
    ShowEntityInfo();
    //BLOPS_NULLSUB();
    GlassSv_Update();
    LiveSteam_Server_RunCallbacks();
}

void __cdecl G_ClientDoPerFrameNotifies(gentity_s *ent)
{
    char *v1; // eax
    bool v2; // al
    bool IsSprinting; // al
    bool previouslySprinting; // [esp-Ch] [ebp-14h]
    unsigned __int16 sprint_begin; // [esp-8h] [ebp-10h]
    unsigned __int16 sprint_end; // [esp-4h] [ebp-Ch]
    gclient_s *client; // [esp+4h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp", 2923, 0, "%s", "ent") )
        __debugbreak();
    client = ent->client;
    if ( !client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp", 2927, 0, "%s", "client") )
    {
        __debugbreak();
    }
    if ( client->sess.connected == CON_DISCONNECTED
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    2928,
                    0,
                    "%s",
                    "client->sess.connected != CON_DISCONNECTED") )
    {
        __debugbreak();
    }
    if ( client->ps.weapon != LOWORD(client->lastWeapon) )
    {
        v1 = (char *)BG_WeaponName(client->ps.weapon);
        Scr_AddString(v1, SCRIPTINSTANCE_SERVER);
        Scr_Notify(ent, scr_const.weapon_change, 1u);
        client->lastWeapon = client->ps.weapon;
    }
    if ( client->ps.lastDtpEnd && client->ps.lastDtpEnd == client->ps.commandTime )
        Scr_Notify(ent, scr_const.dtp_end, 0);
    if ( (client->ps.weaponstate == 6 || client->ps.weaponstate == 31) && client->ps.pm_type < 9 )
        v2 = DoPerFrameNotify(ent, 1, client->previouslyFiring, scr_const.begin_firing, scr_const.end_firing);
    else
        v2 = DoPerFrameNotify(ent, 0, client->previouslyFiring, scr_const.begin_firing, scr_const.end_firing);
    client->previouslyFiring = v2;
    client->previouslyUsingNightVision = DoPerFrameNotify(
                                                                                 ent,
                                                                                 (client->ps.weapFlags & 0x40) != 0,
                                                                                 client->previouslyUsingNightVision,
                                                                                 scr_const.night_vision_on,
                                                                                 scr_const.night_vision_off);
    sprint_end = scr_const.sprint_end;
    sprint_begin = scr_const.sprint_begin;
    previouslySprinting = client->previouslySprinting;
    IsSprinting = PM_IsSprinting(&client->ps);
    client->previouslySprinting = DoPerFrameNotify(ent, IsSprinting, previouslySprinting, sprint_begin, sprint_end);
}

bool __cdecl DoPerFrameNotify(
                gentity_s *ent,
                bool isCurrently,
                bool wasPreviously,
                unsigned __int16 begin,
                unsigned __int16 end)
{
    if ( isCurrently == wasPreviously )
        return wasPreviously;
    if ( isCurrently )
        Scr_Notify(ent, begin, 0);
    else
        Scr_Notify(ent, end, 0);
    return isCurrently;
}

void __cdecl G_UpdateIKCulling(gentity_s *ent)
{
    if ( ent->r.inuse )
    {
        if ( ent->client )
        {
            ent->client->ps.eFlags |= 0x20000u;
            if ( (0x90000C & ent->client->ps.pm_flags) == 0
                && ((ent->client->ps.pm_flags & 0x8000) == 0
                 || g_pmove[ent->client->ps.clientNum].xyspeed <= player_sprintThreshhold->current.value) )
            {
                ent->client->ps.eFlags &= ~0x20000u;
            }
        }
    }
}

void __cdecl G_RunFrameForEntity(gentity_s *ent)
{
    char *v1; // eax
    const char *v2; // eax
    char *v3; // eax
    const char *v4; // eax
    char *v5; // eax
    const char *v6; // eax

    iassert(ent->r.inuse);

    if ( ent->processedFrame != level.framenum )
    {
        ent->processedFrame = level.framenum;
        if ( ent->tagInfo )
        {
            iassert(ent->tagInfo->parent);
            G_RunFrameForEntity(ent->tagInfo->parent);
        }

        iassert(((ent->r.svFlags & ((1 << 1) | (1 << 2))) != ((1 << 1) | (1 << 2))));
        iassert(ent->r.maxs[0] >= ent->r.mins[0]);
        iassert(ent->r.maxs[1] >= ent->r.mins[1]);
        iassert(ent->r.maxs[2] >= ent->r.mins[2]);

        if ( ent->s.loopSoundId )
        {
            if ( ent->s.loopSoundFade < 0 )
            {
                ent->s.loopSoundFade += LOWORD(level.time) - LOWORD(level.previousTime);
                if ( ent->s.loopSoundFade >= 0 )
                {
                    ent->s.loopSoundId = 0;
                    ent->s.loopSoundFade = 0;
                }
            }
        }
        if ( ent->scr_vehicle )
        {
            iassert((unsigned)(ent->scr_vehicle - level.vehicles) < MAX_VEHICLES);
            iassert(ent->scr_vehicle->entNum != ENTITYNUM_NONE);
        }
        else
        {
            iassert(ent->s.eType != ET_VEHICLE);
        }

        if ( !ent->client
            && ent->s.eType != ET_VEHICLE
            && ent->s.eType != ET_HELICOPTER
            && ent->s.eType != ET_PLAYER_CORPSE
            && (ent->s.lerp.eFlags & 0x4000) != 0
            && level.time > ent->s.time2 )
        {
            Scr_Notify(ent, scr_const.death, 0);
            G_FreeEntity(ent);
            return;
        }
        if ( level.time - ent->eventTime > 300 )
        {
            if ( ent->freeAfterEvent )
            {
                G_FreeEntity(ent);
                return;
            }
            if ( ent->unlinkAfterEvent )
            {
                ent->unlinkAfterEvent = 0;
                SV_UnlinkEntity(ent);
            }
        }
        if ( !ent->freeAfterEvent )
        {
            switch ( ent->s.eType )
            {
                case ET_MISSILE:
                    G_RunMissile(ent);
                    if ( ent->tagInfo )
                        G_GeneralLink(ent);
                    return;
                case ET_ITEM:
                    if ( ent->tagInfo )
                    {
                        G_GeneralLink(ent);
                        G_RunThink(ent);
                        return;
                    }
LABEL_63:
                    G_RunItem(ent);
                    return;
                case ET_PLAYER_CORPSE:
                    G_RunCorpse(ent);
                    return;
                case ET_ACTOR_CORPSE:
                    G_RunThink(ent);
                    return;
            }
            if ( ent->physicsObject )
                goto LABEL_63;
            if ( ent->s.eType == ET_SCRIPTMOVER || ent->s.eType == ET_PLANE || ent->s.eType == ET_PRIMARY_LIGHT)
            {
                SV_Flame_Apply_Damage(ent);
                G_RunMover(ent);
            }
            else if ( ent->client )
            {
                G_RunClient(ent);
            }
            else
            {
                if ( !ent->s.eType || ent->s.eType == 20 )
                {
                    if ( ent->tagInfo )
                        G_GeneralLink(ent);
                }
                SV_Flame_Apply_Damage(ent);
                G_RunThink(ent);
            }
        }
    }
}

void    G_UpdateWeapons(gentity_s *ent)
{
    weaponParms wp; // [esp+4h] [ebp-48h] BYREF

    if ( ent->client )
    {
        if ( ent->client->sess.sessionState == SESS_STATE_SPECTATOR )
            return;
        Weapon_SetWeaponParamsWeapon(&wp, ent->client->ps.weapon);
    }
    else
    {
        if ( !ent->scr_vehicle )
            return;
        Weapon_SetWeaponParamsWeapon(&wp, ent->s.weapon);
    }
    if ( wp.weapDef->weapClass == WEAPCLASS_GAS || wp.weapDef->weapType == WEAPTYPE_GAS )
    {
        G_CalcMuzzlePoints(ent, &wp, 0);
        Weapon_Flamethrower_Update(ent, &wp);
    }
    if ( ent->client )
        Weapon_Overheat_Update(ent);
}

int G_PopulateMatchState()
{
    const char *v0; // eax
    int result; // eax

    level.matchState.index = 0;
    level.matchState.unarchivedState.alliesScore = level.teamScores[2];
    level.matchState.unarchivedState.axisScore = level.teamScores[1];
    v0 = va("scr_%s_scorelimit", g_gametype->current.string);
    result = Dvar_GetInt(v0);
    level.matchState.unarchivedState.scoreLimit = result;
    level.matchState.unarchivedState.mapCenter[0] = svs.mapCenter[0];
    level.matchState.unarchivedState.mapCenter[1] = svs.mapCenter[1];
    level.matchState.unarchivedState.mapCenter[2] = svs.mapCenter[2];
    return result;
}

bool __cdecl G_IsEntWalkable(int localClientNum, int entityNum)
{
    return g_entities[entityNum].client == 0;
}

bool __cdecl G_GetEntityOriginAngles(int localClientNum, int entityNum, float *origin, float *angles)
{
    gentity_s *ent; // [esp+8h] [ebp-4h]

    ent = &g_entities[entityNum];
    if ( !ent->r.inuse )
        return 0;
    *origin = ent->r.currentOrigin[0];
    origin[1] = ent->r.currentOrigin[1];
    origin[2] = ent->r.currentOrigin[2];
    *angles = ent->r.currentAngles[0];
    angles[1] = ent->r.currentAngles[1];
    angles[2] = ent->r.currentAngles[2];
    return 1;
}

void __cdecl G_EntityLinkFromPMove(unsigned int entityNum, int parentEntityNum, int tagName)
{
    gentity_s *parent; // [esp+0h] [ebp-8h]

    if ( entityNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    3852,
                    0,
                    "%s",
                    "entityNum >= 0 && entityNum < MAX_CLIENTS") )
    {
        __debugbreak();
    }
    if ( (parentEntityNum < 32 || parentEntityNum >= 1024)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp",
                    3853,
                    0,
                    "%s",
                    "parentEntityNum >= MAX_CLIENTS && parentEntityNum < MAX_GENTITIES") )
    {
        __debugbreak();
    }
    parent = &g_entities[parentEntityNum];
    if ( !g_entities[entityNum].r.inuse
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp", 3858, 0, "%s", "ent->r.inuse") )
    {
        __debugbreak();
    }
    if ( !parent->r.inuse
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_main_mp.cpp", 3859, 0, "%s", "parent->r.inuse") )
    {
        __debugbreak();
    }
    G_EntLinkToWithOffset(&g_entities[entityNum], parent, tagName, vec3_origin, vec3_origin);
}

void __cdecl G_AddDebugString(const float *xyz, const float *color, float scale, char *pszText, int duration)
{
    CL_AddDebugString(xyz, color, scale, pszText, duration);
}

gclient_s::gclient_s()
{
    int m; // [esp+8h] [ebp-34h]
    int k; // [esp+10h] [ebp-2Ch]
    int j; // [esp+18h] [ebp-24h]
    int i; // [esp+20h] [ebp-1Ch]

    //clientSession_t::clientSession_t(&this->sess);
    for ( i = 0; i < 2; ++i )
        this->button_bits.array[i] = 0;
    for ( j = 0; j < 2; ++j )
        this->oldbutton_bits.array[j] = 0;
    for ( k = 0; k < 2; ++k )
        this->latched_button_bits.array[k] = 0;
    for ( m = 0; m < 2; ++m )
        this->button_bitsSinceLastFrame.array[m] = 0;
    //return this;
}