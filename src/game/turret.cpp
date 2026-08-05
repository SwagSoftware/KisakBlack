#include "turret.h"
#include <game_mp/g_main_mp.h>
#include <game_mp/g_utils_mp.h>
#include <game_mp/g_misc_mp.h>
#include <game_mp/g_spawn_mp.h>
#include <clientscript/scr_const.h>
#include <qcommon/dobj_management.h>
#include <cgame/cg_drawtools.h>
#include <universal/com_math_anglevectors.h>
#include <bgame/bg_misc.h>
#include <server/sv_world.h>
#include <bgame/bg_weapons_ammo.h>
#include "g_targets.h"
#include "g_weapon.h"
#include "bullet.h"
#include <clientscript/cscr_stringlist.h>
#include <client_mp/g_client_mp.h>
#include <xanim/dobj_utils.h>
#include <clientscript/cscr_vm.h>
#include "g_load_utils.h"
#include <bgame/bg_weapons_def.h>
#include <game_mp/g_combat_mp.h>
#include <xanim/xanim.h>
#include <sound/snd_bank.h>

const dvar_t *turret_KillstreakTargetTime;
const dvar_t *turret_SentryTargetTime;
const dvar_t *turret_TargetLeadBias;

TurretInfo turretInfoStore[32];

void __cdecl Turret_RegisterDvars()
{
    turret_KillstreakTargetTime = _Dvar_RegisterInt(
                                                                    "turret_KillstreakTargetTime",
                                                                    4000,
                                                                    0,
                                                                    0x7FFFFFFF,
                                                                    0x80u,
                                                                    "Time in milliseconds that a TOW turret will stay locked on a target before verifying line of sight");
    turret_SentryTargetTime = _Dvar_RegisterInt(
                                                            "turret_SentryTargetTime",
                                                            1000,
                                                            0,
                                                            0x7FFFFFFF,
                                                            0x80u,
                                                            "Time in milliseconds that a sentry turret will stay locked on a target before verifying line of sight");
    turret_TargetLeadBias = _Dvar_RegisterInt(
                                                        "turret_TargetLeadBias",
                                                        1,
                                                        0,
                                                        25,
                                                        0x80u,
                                                        "The number of server frames used to predict an entity's position. Used for turret aiming when firing.");
}

void __cdecl G_InitTurrets()
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 32; ++i )
        turretInfoStore[i].inuse = 0;
    level.turrets = turretInfoStore;
}

void __cdecl G_ClientStopUsingTurret(gentity_s *self)
{
    gentity_s *owner; // [esp+0h] [ebp-8h]
    TurretInfo *pTurretInfo; // [esp+4h] [ebp-4h]

    pTurretInfo = self->pTurretInfo;
    if ( !pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 727, 0, "%s", "pTurretInfo") )
    {
        __debugbreak();
    }
    iassert(self->r.ownerNum.isDefined());
    
    owner = self->r.ownerNum.ent();
    if ( !owner->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 732, 0, "%s", "owner->client") )
    {
        __debugbreak();
    }
    pTurretInfo->fireSndDelay = 0;
    self->s.loopSoundId = 0;
    self->s.loopSoundFade = 0;
    if ( pTurretInfo->prevStance != -1 )
    {
        if ( pTurretInfo->prevStance == 2 )
        {
            G_AddEvent(owner, 0xAu, 0);
        }
        else if ( pTurretInfo->prevStance == 1 )
        {
            G_AddEvent(owner, 9u, 0);
        }
        else
        {
            G_AddEvent(owner, 8u, 0);
        }
        pTurretInfo->prevStance = -1;
    }
    if ( self->tagInfo && self->tagInfo->parent && G_EntIsLinkedTo(owner, self->tagInfo->parent) )
        G_EntUnlink(owner);
    TeleportPlayer(owner, pTurretInfo->userOrigin, owner->r.currentAngles);
    owner->client->ps.eFlags &= 0xFFFFFCFF;
    owner->client->ps.viewlocked = PLAYERVIEWLOCK_NONE;
    owner->client->ps.viewlocked_entNum = 1023;
    owner->active = 0;
    G_DeactivateTurret(self);
    //EntHandle::setEnt(&self->r.ownerNum, 0);
    self->r.ownerNum.setEnt(0);
    pTurretInfo->flags &= ~0x800u;
    Scr_Notify(self, scr_const.turretownerchange, 0);
    if ( self->classname == scr_const.deployed_turret )
        G_FreeEntity(self);
}

void __cdecl turret_think_client(gentity_s *self)
{
    gentity_s *owner; // [esp+0h] [ebp-4h]

    iassert(self->r.ownerNum.isDefined());

    owner = self->r.ownerNum.ent();
    if ( !owner->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 828, 0, "%s", "owner->client") )
    {
        __debugbreak();
    }
    if ( owner->active == 1 && owner->health > 0 )
    {
        if ( !self->active
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 832, 0, "%s", "self->active") )
        {
            __debugbreak();
        }
        turret_track(self, owner);
        turret_UpdateSound(self);
    }
    else if ( self->classname == scr_const.deployed_turret )
    {
        owner->client->ps.weaponstate = WEAPON_DEPLOYED;
        owner->client->ps.eFlags &= 0xFFFFFCFF;
    }
    else
    {
        G_ClientStopUsingTurret(self);
    }
}

void __cdecl turret_track(gentity_s *self, gentity_s *other)
{
    TurretInfo *turretInfo; // [esp+0h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    turretInfo = self->pTurretInfo;
    if ( !turretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 637, 0, "%s", "turretInfo") )
    {
        __debugbreak();
    }
    if ( !self->active
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 639, 0, "%s", "self->active") )
    {
        __debugbreak();
    }
    iassert(self->r.ownerNum.isDefined() && (self->r.ownerNum.ent() == other));

    if ( !other->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 641, 0, "%s", "other->client") )
    {
        __debugbreak();
    }
    turret_clientaim(self, other);
    G_PlayerTurretPositionAndBlend(other, self);
    weapDef = BG_GetWeaponDef(self->s.weapon);
    turretInfo->fireTime -= 50;
    if ( turretInfo->fireTime <= 0 )
    {
        turretInfo->fireTime = 0;
        if ( (other->client->ps.pm_flags & 0x800) != 0
            || other->client->ps.weaponstate == 35
            //|| !bitarray<51>::testBit(&other->client->button_bits, 0)
            || !other->client->button_bits.testBit(0)
            || turret_is_overheating(self) )
        {
            turretInfo->triggerDown = 0;
            self->s.lerp.eFlags &= ~0x40u;
        }
        else if ( weapDef->fireType != WEAPON_FIRETYPE_SINGLESHOT || !turretInfo->triggerDown )
        {
            turretInfo->triggerDown = 1;
            turretInfo->fireTime = weapDef->iFireTime;
            turret_shoot_internal(self, other);
            self->s.lerp.eFlags |= 0x40u;
        }
    }
}

void __cdecl G_PlayerTurretPositionAndBlend(gentity_s *ent, gentity_s *pTurretEnt)
{
    const char *AnimDebugName; // eax
    const char *v3; // eax
    double v4; // st7
    gclient_s *client; // edx
    float *v6; // [esp+28h] [ebp-244h]
    float *v7; // [esp+30h] [ebp-23Ch]
    float *origin; // [esp+34h] [ebp-238h]
    float fHeightRatio; // [esp+A0h] [ebp-1CCh]
    unsigned int iPrevBlend; // [esp+A4h] [ebp-1C8h]
    col_context_t context; // [esp+A8h] [ebp-1C4h] BYREF
    float fPrevTransZ; // [esp+D0h] [ebp-19Ch]
    DObj *obj; // [esp+D4h] [ebp-198h]
    int numVertChildren; // [esp+D8h] [ebp-194h]
    float trans2[3]; // [esp+DCh] [ebp-190h] BYREF
    float yaw; // [esp+E8h] [ebp-184h]
    float trans[3]; // [esp+ECh] [ebp-180h] BYREF
    float start[3]; // [esp+F8h] [ebp-174h] BYREF
    float end[3]; // [esp+104h] [ebp-168h] BYREF
    int iBlend; // [esp+110h] [ebp-15Ch]
    DObjAnimMat *tagMat; // [esp+114h] [ebp-158h]
    unsigned int heightAnim; // [esp+118h] [ebp-154h]
    float fDelta; // [esp+11Ch] [ebp-150h]
    float fPrevBlend; // [esp+120h] [ebp-14Ch]
    float rot[2]; // [esp+124h] [ebp-148h] BYREF
    float tagAxis[3][3]; // [esp+12Ch] [ebp-140h] BYREF
    float localAxis[4][3]; // [esp+150h] [ebp-11Ch] BYREF
    unsigned int leafAnim1; // [esp+180h] [ebp-ECh]
    trace_t trace; // [esp+184h] [ebp-E8h] BYREF
    float endpos[3]; // [esp+1C0h] [ebp-ACh] BYREF
    int numHorChildren; // [esp+1CCh] [ebp-A0h]
    clientInfo_t *ci; // [esp+1D0h] [ebp-9Ch]
    float tagHeight; // [esp+1D4h] [ebp-98h]
    int i; // [esp+1D8h] [ebp-94h]
    unsigned int baseAnim; // [esp+1DCh] [ebp-90h]
    int clientNum; // [esp+1E0h] [ebp-8Ch]
    lerpFrame_t *pLerpAnim; // [esp+1E4h] [ebp-88h]
    const WeaponDef *weapDef; // [esp+1E8h] [ebp-84h]
    float fBlend; // [esp+1ECh] [ebp-80h]
    float axis[4][3]; // [esp+1F0h] [ebp-7Ch] BYREF
    XAnimTree_s *pAnimTree; // [esp+220h] [ebp-4Ch]
    XAnim_s *pXAnims; // [esp+224h] [ebp-48h]
    unsigned int leafAnim2; // [esp+228h] [ebp-44h]
    float localYaw; // [esp+22Ch] [ebp-40h]
    float turretAxis[4][3]; // [esp+230h] [ebp-3Ch] BYREF
    float vDelta[3]; // [esp+260h] [ebp-Ch]

    //col_context_t::col_context_t(&context);
    clientNum = ent->s.clientNum;
    if ( (unsigned int)clientNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp",
                    362,
                    0,
                    "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    clientNum,
                    32) )
    {
        __debugbreak();
    }
    ci = &level_bgs.clientinfo[clientNum];
    if ( !ci->infoValid
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 364, 0, "%s", "ci->infoValid") )
    {
        __debugbreak();
    }
    pLerpAnim = &ci->legs;
    if ( ci->legs.animationNumber && pLerpAnim->animation && (pLerpAnim->animation->flags & 4) != 0 )
    {
        tagMat = G_DObjGetLocalTagMatrix(pTurretEnt, scr_const.tag_weapon);
        if ( tagMat )
        {
            obj = Com_GetServerDObj(ent->s.number);
            if ( obj )
            {
                if ( !pTurretEnt->s.weapon
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp",
                                382,
                                0,
                                "%s",
                                "pTurretEnt->s.weapon") )
                {
                    __debugbreak();
                }
                weapDef = BG_GetWeaponDef(pTurretEnt->s.weapon);
                if ( weapDef->weapClass != WEAPCLASS_TURRET
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp",
                                384,
                                0,
                                "%s",
                                "weapDef->weapClass == WEAPCLASS_TURRET") )
                {
                    __debugbreak();
                }
                if ( weapDef->fAnimHorRotateInc == 0.0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp",
                                385,
                                0,
                                "%s",
                                "weapDef->fAnimHorRotateInc") )
                {
                    __debugbreak();
                }
                pAnimTree = ci->pXAnimTree;
                pXAnims = level_bgs.animData->animScriptData.animTree.anims;
                baseAnim = pLerpAnim->animationNumber & 0xFFFFFBFF;
                ConvertQuatToMat(tagMat, tagAxis);
                localYaw = vectosignedyaw(tagAxis[0]);
                AnglesToAxis(pTurretEnt->r.currentAngles, turretAxis);
                turretAxis[3][0] = pTurretEnt->r.currentOrigin[0];
                turretAxis[3][1] = pTurretEnt->r.currentOrigin[1];
                turretAxis[3][2] = pTurretEnt->r.currentOrigin[2];
                vDelta[0] = ent->r.currentOrigin[0] - turretAxis[3][0];
                vDelta[1] = ent->r.currentOrigin[1] - turretAxis[3][1];
                vDelta[2] = ent->r.currentOrigin[2] - turretAxis[3][2];
                tagHeight = (float)((float)(vDelta[0] * turretAxis[2][0]) + (float)(vDelta[1] * turretAxis[2][1]))
                                    + (float)(vDelta[2] * turretAxis[2][2]);
                fDelta = tagHeight - tagMat->trans[2];
                numVertChildren = XAnimGetNumChildren(pXAnims, baseAnim);
                fPrevTransZ = 0.0f;
                fPrevBlend = 0.0f;
                iPrevBlend = 0;
                leafAnim2 = 0;
                if ( !numVertChildren )
                {
                    AnimDebugName = XAnimGetAnimDebugName(pXAnims, baseAnim);
                    Com_Error(ERR_DROP, "player anim %s has no children", AnimDebugName);
                }
                i = 0;
                do
                {
                    heightAnim = XAnimGetChildAt(pXAnims, baseAnim, numVertChildren - 1 - i);
                    numHorChildren = XAnimGetNumChildren(pXAnims, heightAnim);
                    if ( !numHorChildren )
                    {
                        v3 = XAnimGetAnimDebugName(pXAnims, heightAnim);
                        Com_Error(ERR_DROP, "player anim %s has no children", v3);
                    }
                    fBlend = (float)((float)numHorChildren * 0.5) + (float)(localYaw / weapDef->fAnimHorRotateInc);
                    if ( fBlend >= 0.0 )
                    {
                        if ( fBlend >= (float)(numHorChildren - 1) )
                            fBlend = (float)(numHorChildren - 1);
                    }
                    else
                    {
                        fBlend = 0.0f;
                    }
                    iBlend = (int)fBlend;
                    fBlend = fBlend - (float)(int)fBlend;
                    leafAnim1 = XAnimGetChildAt(pXAnims, heightAnim, iBlend);
                    XAnimGetAbsDelta(pXAnims, leafAnim1, rot, trans, 0.0);
                    if ( fBlend != 0.0 )
                    {
                        leafAnim2 = XAnimGetChildAt(pXAnims, heightAnim, iBlend + 1);
                        XAnimGetAbsDelta(pXAnims, leafAnim2, rot, trans2, 0.0);
                        Vec3Lerp(trans, trans2, fBlend, trans);
                    }
                    if ( trans[2] >= fDelta )
                        break;
                    fPrevTransZ = trans[2];
                    iPrevBlend = iBlend;
                    fPrevBlend = fBlend;
                    ++i;
                }
                while ( i < numVertChildren );
                XAnimClearTreeGoalWeightsStrict(pAnimTree, baseAnim, 0.2, -1);
                XAnimSetGoalWeight(obj, leafAnim1, 1.0 - fBlend, 0.0, 1.0, 0, 0, 0, -1);
                if ( fBlend != 0.0 )
                    XAnimSetGoalWeight(obj, leafAnim2, fBlend, 0.0, 1.0, 0, 0, 0, -1);
                if ( i && i != numVertChildren )
                {
                    if ( (float)(trans[2] - fPrevTransZ) == 0.0
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp",
                                    461,
                                    0,
                                    "%s",
                                    "trans[2] - fPrevTransZ") )
                    {
                        __debugbreak();
                    }
                    fHeightRatio = (float)(fDelta - fPrevTransZ) / (float)(trans[2] - fPrevTransZ);
                    XAnimSetGoalWeight(obj, heightAnim, fHeightRatio, 0.0, 1.0, 0, 0, 0, -1);
                    heightAnim = XAnimGetChildAt(pXAnims, baseAnim, numVertChildren - i);
                    XAnimSetGoalWeight(obj, heightAnim, 1.0 - fHeightRatio, 0.0, 1.0, 0, 0, 0, -1);
                    leafAnim1 = XAnimGetChildAt(pXAnims, heightAnim, iPrevBlend);
                    XAnimSetGoalWeight(obj, leafAnim1, 1.0 - fPrevBlend, 0.0, 1.0, 0, 0, 0, -1);
                    if ( fPrevBlend != 0.0 )
                    {
                        leafAnim2 = XAnimGetChildAt(pXAnims, heightAnim, iPrevBlend + 1);
                        XAnimSetGoalWeight(obj, leafAnim2, fPrevBlend, 0.0, 1.0, 0, 0, 0, -1);
                    }
                }
                else
                {
                    XAnimSetGoalWeight(obj, heightAnim, 1.0, 0.0, 1.0, 0, 0, 0, -1);
                }
                XAnimCalcAbsDelta(obj, baseAnim, rot, trans);
                VectorAngleMultiply(trans, localYaw);
                localAxis[3][0] = trans[0] + tagMat->trans[0];
                localAxis[3][1] = trans[1] + tagMat->trans[1];
                localAxis[3][2] = tagHeight;
                v4 = RotationToYaw(rot);
                yaw = v4 + localYaw;
                YawToAxis(yaw, localAxis);
                MatrixMultiply43(localAxis, turretAxis, axis);
                origin = ent->client->ps.origin;
                *origin = axis[3][0];
                origin[1] = axis[3][1];
                origin[2] = axis[3][2];
                v7 = ent->client->ps.origin;
                start[0] = *v7;
                start[1] = v7[1];
                start[2] = v7[2];
                client = ent->client;
                end[0] = client->ps.origin[0];
                end[1] = client->ps.origin[1];
                end[2] = client->ps.origin[2];
                start[2] = start[2] + ent->client->ps.viewHeightCurrent;
                end[2] = end[2] - 100.0;
                G_TraceCapsule(&trace, start, vec3_origin, vec3_origin, end, ent->s.number, 0x2818011, &context);
                if ( trace.fraction < 1.0 && !trace.allsolid && !trace.startsolid )
                {
                    Vec3Lerp(start, end, trace.fraction, endpos);
                    ent->r.currentOrigin[2] = endpos[2];
                    ent->client->ps.origin[2] = endpos[2];
                }
                BG_PlayerStateToEntityState(&ent->client->ps, &ent->s, 1, 1u);
                v6 = ent->client->ps.origin;
                ent->r.currentOrigin[0] = *v6;
                ent->r.currentOrigin[1] = v6[1];
                ent->r.currentOrigin[2] = v6[2];
                AxisToAngles(axis, ent->r.currentAngles);
                ent->s.lerp.apos.trBase[0] = ent->r.currentAngles[0];
                ent->s.lerp.apos.trBase[1] = ent->r.currentAngles[1];
                ent->s.lerp.apos.trBase[2] = ent->r.currentAngles[2];
                SV_LinkEntity(ent);
            }
        }
        else
        {
            Com_PrintWarning(17, "WARNING: aborting player positioning on turret since 'tag_weapon' does not exist\n");
        }
    }
}

void __cdecl turret_clientaim(gentity_s *self, gentity_s *other)
{
    int v2; // [esp+4h] [ebp-4Ch]
    int v3; // [esp+8h] [ebp-48h]
    float v4; // [esp+Ch] [ebp-44h]
    int period; // [esp+18h] [ebp-38h]
    float v6; // [esp+28h] [ebp-28h]
    int actorNum; // [esp+34h] [ebp-1Ch]
    const WeaponDef *weapDef; // [esp+44h] [ebp-Ch]
    TurretInfo *pTurretInfo; // [esp+48h] [ebp-8h]
    gclient_s *ps; // [esp+4Ch] [ebp-4h]

    if (!self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 528, 0, "%s", "self"))
        __debugbreak();
    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 531, 0, "%s", "pTurretInfo"))
    {
        __debugbreak();
    }
    if (!other && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 533, 0, "%s", "other"))
        __debugbreak();
    if (!other->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 534, 0, "%s", "other->client"))
    {
        __debugbreak();
    }
    ps = other->client;
    if (!self->active
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 538, 0, "%s", "self->active"))
    {
        __debugbreak();
    }
    iassert(self->r.ownerNum.isDefined());
    iassert(self->r.ownerNum.ent() == other);

    if (ps->ps.viewlocked == PLAYERVIEWLOCK_WEAPONJITTER)
    {
        weapDef = BG_GetWeaponDef(self->s.weapon);
        if (pTurretInfo->fireTime <= 0 || pTurretInfo->fireTime < weapDef->iFireTime - 200)
            ps->ps.viewlocked = PLAYERVIEWLOCK_FULL;
    }
    else
    {
        ps->ps.viewlocked = PLAYERVIEWLOCK_FULL;
    }
    AssignToSmallerType<short>(&ps->ps.viewlocked_entNum, self->s.number);
    self->s.lerp.u.turret.gunAngles[0] = AngleNormalize180(ps->ps.viewangles[0] - self->r.currentAngles[0]);
    v6 = self->s.lerp.u.turret.gunAngles[0];
    if ((float)(v6 - pTurretInfo->arcmax[0]) < 0.0)
        actorNum = self->s.lerp.u.actor.actorNum;
    else
        actorNum = LODWORD(pTurretInfo->arcmax[0]);
    if ((float)(pTurretInfo->arcmin[0] - v6) < 0.0)
        v3 = actorNum;
    else
        v3 = LODWORD(pTurretInfo->arcmin[0]);
    self->s.lerp.u.actor.actorNum = v3;
    self->s.lerp.u.turret.gunAngles[1] = AngleNormalize180(ps->ps.viewangles[1] - self->r.currentAngles[1]);
    v4 = self->s.lerp.u.turret.gunAngles[1];
    if ((float)(v4 - pTurretInfo->arcmax[1]) < 0.0)
        period = self->s.lerp.u.loopFx.period;
    else
        period = LODWORD(pTurretInfo->arcmax[1]);
    if ((float)(pTurretInfo->arcmin[1] - v4) < 0.0)
        v2 = period;
    else
        v2 = LODWORD(pTurretInfo->arcmin[1]);
    self->s.lerp.u.loopFx.period = v2;
    self->s.lerp.u.actor.team = 0;// *(_DWORD *)&FLOAT_0_0;
    self->s.lerp.u.turret.heatVal = g_entities[ps->ps.viewlocked_entNum].pTurretInfo->heatVal / 100.0;
    self->s.lerp.u.turret.overheating = g_entities[ps->ps.viewlocked_entNum].pTurretInfo->overheating;
    if ((pTurretInfo->flags & 0x800) != 0)
    {
        pTurretInfo->flags &= ~0x800u;
        self->s.lerp.eFlags ^= 2u;
    }
}

void __cdecl turret_shoot_internal(gentity_s *self, gentity_s *other)
{
    const WeaponVariantDef *WeaponVariantDef; // eax
    const WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    if ( !self->pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 580, 0, "%s", "self->pTurretInfo") )
    {
        __debugbreak();
    }
    if ( !other && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 581, 0, "%s", "other") )
        __debugbreak();
    if ( (self->pTurretInfo->flags & 0x4000) == 0 )
    {
        self->pTurretInfo->fireSndDelay = 3 * BG_GetWeaponDef(self->s.weapon)->iFireTime;
        if ( other->client )
        {
            Fire_Lead(self, other);
            if ( other->client->ps.weaponstate == 36 )
            {
                other->client->ps.viewlocked = PLAYERVIEWLOCK_WEAPONJITTER;
                WeaponVariantDef = BG_GetWeaponVariantDef(other->client->ps.weapon);
                BG_AddAmmoToClip(&other->client->ps, WeaponVariantDef->iClipIndex, -1);
            }
            else
            {
                if ( (other->client->ps.eFlags & 0x300) != 0 )
                    other->client->ps.viewlocked = PLAYERVIEWLOCK_WEAPONJITTER;
                weapDef = BG_GetWeaponDef(self->s.weapon);
                if ( weapDef->overheatWeapon )
                    self->pTurretInfo->heatVal = (float)((float)((float)(level.time - level.previousTime) * weapDef->overheatRate)
                                                                                         / 1000.0)
                                                                         + self->pTurretInfo->heatVal;
            }
        }
        else
        {
            self->s.lerp.u.turret.flags &= ~0x20u;
            Fire_Lead(self, other);
        }
    }
}

void __cdecl Fire_Lead(gentity_s *ent, gentity_s *activator)
{
    gentity_s *v2; // [esp+18h] [ebp-78h]
    int i; // [esp+20h] [ebp-70h]
    target_t *targets; // [esp+24h] [ebp-6Ch]
    const target_t *targetEnt; // [esp+28h] [ebp-68h]
    float spread; // [esp+30h] [ebp-60h]
    TurretInfo *turretInfo; // [esp+34h] [ebp-5Ch]
    weaponParms wp; // [esp+38h] [ebp-58h] BYREF
    float targetPos[3]; // [esp+84h] [ebp-Ch]
    int savedregs; // [esp+90h] [ebp+0h] BYREF

    if ( !activator && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 193, 0, "%s", "activator") )
        __debugbreak();
    if ( activator == &g_entities[1023] )
        v2 = ent;
    else
        v2 = activator;
    Turret_FillWeaponParms(ent, v2, ent->s.weapon, &wp);
    turretInfo = ent->pTurretInfo;
    if ( !turretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 200, 0, "%s", "turretInfo") )
    {
        __debugbreak();
    }
    targetPos[0] = turretInfo->targetPos[0];
    targetPos[1] = turretInfo->targetPos[1];
    targetPos[2] = turretInfo->targetPos[2];
    if ( wp.weapDef->weapType )
    {
        if ( wp.weapDef->weapType == WEAPTYPE_GAS )
        {
            Weapon_Flamethrower_Fire(ent, &wp);
        }
        //else if ( (turretInfo->flags & 0x20000) != 0 && EntHandle::isDefined(&turretInfo->target) )
        else if ( (turretInfo->flags & 0x20000) != 0 && turretInfo->target.isDefined() )
        {
            targets = Target_GetTargetArray();
            targetEnt = 0;
            for ( i = 0; i < 32; ++i )
            {
                //if ( targets->targets[i].ent == EntHandle::ent(&turretInfo->target) )
                if ( targets[i].ent == turretInfo->target.ent() )
                {
                    targetEnt = &targets[i];
                    break;
                }
            }
            if ( targetEnt )
                Weapon_RocketLauncher_Fire(
                    ent,
                    ent->s.weapon,
                    0.0,
                    &wp,
                    vec3_origin,
                    targetEnt->ent,
                    targetEnt->offset);
            else
                Weapon_RocketLauncher_Fire(ent, ent->s.weapon, 0.0, &wp, vec3_origin, 0, 0);
        }
        else
        {
            Weapon_RocketLauncher_Fire(v2, ent->s.weapon, 0.0, &wp, vec3_origin, 0, 0);
        }
    }
    else
    {
        if ( activator->client )
            spread = turretInfo->playerSpread;
        else
            spread = turretInfo->aiSpread;
        Bullet_Fire(v2, spread, &wp, ent, level.time);
    }
    if ( turretInfo->fireBarrel )
        G_AddEvent(ent, 0x30u, v2->s.number);
    else
        G_AddEvent(ent, 0x2Fu, v2->s.number);
    if ( (turretInfo->flags & 0x8000) != 0 )
    {
        ++turretInfo->fireBarrel;
        turretInfo->fireBarrel %= 2;
    }
}

void __cdecl Turret_FillWeaponParms(
                const gentity_s *ent,
                const gentity_s *activator,
                unsigned int weapon,
                weaponParms *wp)
{
    char *v4; // eax
    float diff[3]; // [esp+24h] [ebp-58h] BYREF
    float playerPos[3]; // [esp+30h] [ebp-4Ch] BYREF
    float len; // [esp+3Ch] [ebp-40h]
    float angles[3]; // [esp+40h] [ebp-3Ch] BYREF
    float flashTag[4][3]; // [esp+4Ch] [ebp-30h] BYREF

    if ( !G_DObjGetWorldTagMatrix(ent, scr_const.tag_flash, flashTag) )
    {
        v4 = SL_ConvertToString(ent->classname, SCRIPTINSTANCE_SERVER);
        Com_Error(ERR_DROP, "Couldn't find %s on turret (entity %d, classname %s)", "tag_flash", ent->s.number, v4);
    }
    Weapon_SetWeaponParamsWeapon(wp, weapon);
    if ( !activator->client || zombietron->current.enabled )
    {
        angles[1] = ent->r.currentAngles[1] + ent->s.lerp.u.turret.gunAngles[1];
        angles[2] = ent->r.currentAngles[2];
        angles[0] = ent->r.currentAngles[0] + ent->s.lerp.u.turret.gunAngles[0];
        AngleVectors(angles, wp->forward, wp->right, wp->up);
        *(_QWORD *)wp->gunForward = *(_QWORD *)&flashTag[0][0];
        wp->gunForward[2] = flashTag[0][2];
        wp->muzzleTrace[0] = flashTag[3][0];
        wp->muzzleTrace[1] = flashTag[3][1];
        wp->muzzleTrace[2] = flashTag[3][2];
    }
    else
    {
        G_GetPlayerViewOrigin(&activator->client->ps, playerPos);
        G_GetPlayerViewDirection(activator, wp->forward, wp->right, wp->up);
        wp->gunForward[0] = wp->forward[0];
        wp->gunForward[1] = wp->forward[1];
        wp->gunForward[2] = wp->forward[2];
        diff[0] = flashTag[3][0] - playerPos[0];
        diff[1] = flashTag[3][1] - playerPos[1];
        diff[2] = flashTag[3][2] - playerPos[2];
        len = Vec3Normalize(diff);
        wp->muzzleTrace[0] = (float)(len * wp->forward[0]) + playerPos[0];
        wp->muzzleTrace[1] = (float)(len * wp->forward[1]) + playerPos[1];
        wp->muzzleTrace[2] = (float)(len * wp->forward[2]) + playerPos[2];
    }
}

bool __cdecl turret_is_overheating(gentity_s *self)
{
    return self->pTurretInfo->heatVal >= 100.0 || self->pTurretInfo->overheating;
}

void __cdecl turret_UpdateSound(gentity_s *self)
{
    bool v1; // [esp+0h] [ebp-Ch]
    TurretInfo *pTurretInfo; // [esp+8h] [ebp-4h]

    pTurretInfo = self->pTurretInfo;
    if ( !pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 688, 0, "%s", "pTurretInfo") )
    {
        __debugbreak();
    }
    //v1 = EntHandle::isDefined(&self->r.ownerNum) && EntHandle::ent(&self->r.ownerNum)->client;
    v1 = self->r.ownerNum.isDefined() && self->r.ownerNum.ent()->client;
    self->s.loopSoundId = 0;
    self->s.loopSoundFade = 0;
    if ( pTurretInfo->fireSndDelay > 0 )
    {
        if ( pTurretInfo->fireSndPlayer && v1 )
            self->s.loopSoundId = pTurretInfo->fireSndPlayer;
        else
            self->s.loopSoundId = pTurretInfo->fireSnd;
        pTurretInfo->fireSndDelay -= 50;
        if ( pTurretInfo->fireSndDelay <= 0 && pTurretInfo->stopSnd )
        {
            self->s.loopSoundId = 0;
            if ( pTurretInfo->stopSndPlayer && v1 )
                G_PlaySoundAlias(self, pTurretInfo->stopSndPlayer, 0, 0);
            else
                G_PlaySoundAlias(self, pTurretInfo->stopSnd, 0, 0);
        }
    }
}

bool __cdecl turret_IsFiringInternal(int state)
{
    return state != 0;
}

bool __cdecl turret_IsFiring(gentity_s *self)
{
    TurretInfo *pTurretInfo; // [esp+0h] [ebp-4h]

    pTurretInfo = self->pTurretInfo;
    if ( !pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 1080, 0, "%s", "pTurretInfo") )
    {
        __debugbreak();
    }
    return turret_IsFiringInternal(pTurretInfo->state);
}

void __cdecl turret_SetState(gentity_s *self, int state)
{
    bool IsFiring; // esi
    int v3; // eax
    TurretInfo *pTurretInfo; // [esp+4h] [ebp-4h]

    pTurretInfo = self->pTurretInfo;
    if ( !pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 1091, 0, "%s", "pTurretInfo") )
    {
        __debugbreak();
    }
    IsFiring = turret_IsFiring(self);
    if ( IsFiring != turret_IsFiringInternal(state) )
        Scr_Notify(self, scr_const.turretstatechange, 0);
    pTurretInfo->state = state;
    if ( turret_IsFiring(self) )
        v3 = self->s.lerp.u.turret.flags | 4;
    else
        v3 = self->s.lerp.u.turret.flags & 0xFFFFFFFB;
    self->s.lerp.u.turret.flags = v3;
}

void __cdecl turret_ClearTargetEnt(gentity_s *self)
{
    TurretInfo *pTurretInfo; // [esp+0h] [ebp-4h]

    pTurretInfo = self->pTurretInfo;
    if ( !pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 1110, 0, "%s", "pTurretInfo") )
    {
        __debugbreak();
    }
    pTurretInfo->target.setEnt(0);
    pTurretInfo->flags &= 0xFFFFFFB7;
    turret_SetState(self, 0);
}

void __cdecl turret_SetDefaultDropPitch(gentity_s *self, float pitch)
{
    TurretInfo *pTurretInfo; // [esp+0h] [ebp-4h]

    pTurretInfo = self->pTurretInfo;
    if ( !pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 1147, 0, "%s", "pTurretInfo") )
    {
        __debugbreak();
    }
    pTurretInfo->dropPitch = pitch;
}

void __cdecl turret_SetScanningPitch(gentity_s *self, float pitch)
{
    TurretInfo *pTurretInfo; // [esp+0h] [ebp-4h]

    pTurretInfo = self->pTurretInfo;
    if ( !pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 1157, 0, "%s", "pTurretInfo") )
    {
        __debugbreak();
    }
    pTurretInfo->scanningPitch = pitch;
}

void __cdecl turret_shoot(gentity_s *self)
{
    gentity_s *other; // [esp+0h] [ebp-4h]

    if ( self->r.ownerNum.isDefined() )
    {
        other = self->r.ownerNum.ent();
        turret_shoot_internal(self, other);
    }
    else
    {
        turret_shoot_internal(self, &g_entities[1023]);
    }
}

int __cdecl turret_CanTargetPoint(const gentity_s *self, const float *vPoint, float *vSource, float *localAngles)
{
    double v5; // st7
    float angles[3]; // [esp+14h] [ebp-2Ch] BYREF
    TurretInfo *pTurretInfo; // [esp+20h] [ebp-20h]
    float tagFlashPos[3]; // [esp+24h] [ebp-1Ch] BYREF
    int i; // [esp+30h] [ebp-10h]
    float vDelta[3]; // [esp+34h] [ebp-Ch] BYREF

    if ( !G_DObjGetWorldTagPos(self, scr_const.tag_flash, tagFlashPos) )
        return 0;
    pTurretInfo = self->pTurretInfo;
    if ( !pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 1184, 0, "%s", "pTurretInfo") )
    {
        __debugbreak();
    }
    vDelta[0] = *vPoint - tagFlashPos[0];
    vDelta[1] = vPoint[1] - tagFlashPos[1];
    vDelta[2] = vPoint[2] - tagFlashPos[2];
    vectosignedangles(vDelta, angles);
    for ( i = 0; i < 2; ++i )
    {
        v5 = AngleNormalize180(angles[i] - self->r.currentAngles[i]);
        localAngles[i] = v5;
        if ( localAngles[i] > pTurretInfo->arcmax[i] || pTurretInfo->arcmin[i] > localAngles[i] )
            return 0;
    }
    *vSource = tagFlashPos[0];
    vSource[1] = tagFlashPos[1];
    vSource[2] = tagFlashPos[2];
    return 1;
}

int __cdecl turret_CanTargetSentient(
                const gentity_s *self,
                const sentient_s *sentient,
                float *targetPosition,
                float *muzzlePosition,
                float *localAngles)
{
    float *v6; // [esp+4h] [ebp-78h]
    float v7; // [esp+8h] [ebp-74h]
    float *currentOrigin; // [esp+14h] [ebp-68h]
    float *v9; // [esp+30h] [ebp-4Ch]
    float v10; // [esp+34h] [ebp-48h]
    float *v11; // [esp+40h] [ebp-3Ch]
    float angles[3]; // [esp+50h] [ebp-2Ch] BYREF
    TurretInfo *pTurretInfo; // [esp+5Ch] [ebp-20h]
    int bTooLow; // [esp+60h] [ebp-1Ch]
    float tagFlashPos[3]; // [esp+64h] [ebp-18h] BYREF
    float vDelta[3]; // [esp+70h] [ebp-Ch] BYREF

    if ( sentient->ent->client )
        Turret_PredictPosition(sentient->ent, &sentient->ent->client->ps, targetPosition);
    else
        Sentient_GetEyePosition(sentient, targetPosition);
    if ( !G_DObjGetWorldTagPos(self, scr_const.tag_flash, tagFlashPos) )
        return 0;
    pTurretInfo = self->pTurretInfo;
    if ( !pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 1244, 0, "%s", "pTurretInfo") )
    {
        __debugbreak();
    }
    vDelta[0] = *targetPosition - tagFlashPos[0];
    vDelta[1] = targetPosition[1] - tagFlashPos[1];
    vDelta[2] = targetPosition[2] - tagFlashPos[2];
    vectosignedangles(vDelta, angles);
    localAngles[1] = AngleNormalize180(angles[1] - self->r.currentAngles[1]);
    if ( localAngles[1] > pTurretInfo->arcmax[1] || pTurretInfo->arcmin[1] > localAngles[1] )
        return 0;
    *muzzlePosition = tagFlashPos[0];
    muzzlePosition[1] = tagFlashPos[1];
    muzzlePosition[2] = tagFlashPos[2];
    if ( pTurretInfo->state == 1 )
    {
        *localAngles = AngleNormalize180(angles[0] - self->r.currentAngles[0]);
        if ( *localAngles <= pTurretInfo->arcmax[0] )
        {
            if ( pTurretInfo->arcmin[0] <= *localAngles )
                return 1;
            bTooLow = 0;
        }
        else
        {
            bTooLow = 1;
        }
        currentOrigin = sentient->ent->r.currentOrigin;
        vDelta[0] = *currentOrigin - tagFlashPos[0];
        vDelta[1] = currentOrigin[1] - tagFlashPos[1];
        vDelta[2] = currentOrigin[2] - tagFlashPos[2];
        vDelta[2] = vDelta[2] + 2.0;
        v7 = vectosignedpitch(vDelta);
        *localAngles = AngleNormalize180(v7 - self->r.currentAngles[0]);
        if ( *localAngles <= pTurretInfo->arcmax[0] )
        {
            if ( pTurretInfo->arcmin[0] <= *localAngles )
            {
                v6 = sentient->ent->r.currentOrigin;
                *targetPosition = *v6;
                targetPosition[1] = v6[1];
                targetPosition[2] = v6[2];
                targetPosition[2] = targetPosition[2] + 2.0;
                return 1;
            }
            else if ( bTooLow )
            {
                *localAngles = 0.0f;
                targetPosition[2] = tagFlashPos[2];
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if ( bTooLow )
        {
            return 0;
        }
        else
        {
            *localAngles = 0.0f;
            targetPosition[2] = tagFlashPos[2];
            return 1;
        }
    }
    else
    {
        v11 = sentient->ent->r.currentOrigin;
        vDelta[0] = *v11 - tagFlashPos[0];
        vDelta[1] = v11[1] - tagFlashPos[1];
        vDelta[2] = v11[2] - tagFlashPos[2];
        vDelta[2] = vDelta[2] + 2.0;
        v10 = vectosignedpitch(vDelta);
        *localAngles = AngleNormalize180(v10 - self->r.currentAngles[0]);
        if ( *localAngles <= pTurretInfo->arcmax[0] )
        {
            if ( pTurretInfo->arcmin[0] <= *localAngles )
            {
                v9 = sentient->ent->r.currentOrigin;
                *targetPosition = *v9;
                targetPosition[1] = v9[1];
                targetPosition[2] = v9[2];
                targetPosition[2] = targetPosition[2] + 2.0;
                return 1;
            }
            bTooLow = 0;
        }
        else
        {
            bTooLow = 1;
        }
        *localAngles = AngleNormalize180(angles[0] - self->r.currentAngles[0]);
        if ( *localAngles <= pTurretInfo->arcmax[0] )
        {
            if ( pTurretInfo->arcmin[0] <= *localAngles )
            {
                return 1;
            }
            else if ( bTooLow )
            {
                *localAngles = 0.0f;
                targetPosition[2] = tagFlashPos[2];
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if ( bTooLow )
        {
            return 0;
        }
        else
        {
            *localAngles = 0.0f;
            targetPosition[2] = tagFlashPos[2];
            return 1;
        }
    }
}

void __cdecl Turret_PredictPosition(const gentity_s *ent, const playerState_s *ps, float *vPos)
{
    trajectory_t tr; // [esp+1Ch] [ebp-24h] BYREF

    G_GetPlayerViewOrigin(ps, tr.trBase);
    if ( ((LODWORD(tr.trBase[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(tr.trBase[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(tr.trBase[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp",
                    1209,
                    0,
                    "%s",
                    "!IS_NAN((tr.trBase)[0]) && !IS_NAN((tr.trBase)[1]) && !IS_NAN((tr.trBase)[2])") )
    {
        __debugbreak();
    }
    if ( BG_GetWeaponDef(ps->weapon)->offhandSlot == OFFHAND_SLOT_EQUIPMENT )
        tr.trBase[2] = tr.trBase[2] - 8.0;
    tr.trDelta[0] = ps->velocity[0];
    tr.trDelta[1] = ps->velocity[1];
    tr.trDelta[2] = ps->velocity[2];
    if ( ((LODWORD(tr.trDelta[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(tr.trDelta[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(tr.trDelta[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp",
                    1215,
                    0,
                    "%s",
                    "!IS_NAN((tr.trDelta)[0]) && !IS_NAN((tr.trDelta)[1]) && !IS_NAN((tr.trDelta)[2])") )
    {
        __debugbreak();
    }
    tr.trTime = level.time;
    tr.trType = 3;
    BG_EvaluateTrajectory(&tr, level.time + 50 * turret_TargetLeadBias->current.integer, vPos);
}

int __cdecl turret_SightTrace(const float *start, const float *end, int passEnt1, int passEnt2, gentity_s *turret)
{
    int hitNum; // [esp+A8h] [ebp-4h] BYREF

    //col_context_t::col_context_t(&context);
    col_context_t context; // [esp+80h] [ebp-2Ch] BYREF
    hitNum = 0;
    if (turret)
    {
        if ((turret->pTurretInfo->flags & 0x20000) != 0)
        {
            col_context_t v9(0x280E893); // [esp+58h] [ebp-54h] BYREF
            //col_context_t::col_context_t(&v9, 0x280E893);
            memcpy(&context, &v9, sizeof(context));
        }
        else
        {
            col_context_t v8(0x810011); // [esp+30h] [ebp-7Ch] BYREF
            //col_context_t::col_context_t(&v8, 0x810011);
            memcpy(&context, &v8, sizeof(context));
        }
    }
    else
    {
        col_context_t v7(0x801803); // [esp+8h] [ebp-A4h] BYREF
        //col_context_t::col_context_t(&v7, 0x801803);
        memcpy(&context, &v7, sizeof(context));
    }
    context.mask &= ~0x10u;
    context.passEntityNum0 = passEnt1;
    context.passEntityNum1 = passEnt2;
    SV_SightTracePoint(&hitNum, start, end, &context);
    return hitNum;
}

void __cdecl turret_think_init(gentity_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 2349, 0, "%s", "self") )
        __debugbreak();
    if ( self->handler != 17
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp",
                    2350,
                    0,
                    "%s",
                    "self->handler == ENT_HANDLER_TURRET_INIT") )
    {
        __debugbreak();
    }
    self->handler = 18;
    self->nextthink = level.time + 50;
    self->s.lerp.u.actor.team = 0;
    if ( self->tagInfo )
        G_GeneralLink(self);
    if ( !self->pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 2359, 0, "%s", "self->pTurretInfo") )
    {
        __debugbreak();
    }
    if ( self->pTurretInfo->dropPitch == -90.0 )
        turret_RestoreDefaultDropPitch(self);
}

void __cdecl turret_RestoreDefaultDropPitch(gentity_s *self)
{
    unsigned __int8 boneIndex; // [esp+17h] [ebp-E5h] BYREF
    DObjAnimMat *weaponMtx; // [esp+18h] [ebp-E4h]
    float mtx[3][3]; // [esp+1Ch] [ebp-E0h] BYREF
    float dir[3]; // [esp+40h] [ebp-BCh] BYREF
    DObj *obj; // [esp+4Ch] [ebp-B0h]
    float transDir[3]; // [esp+50h] [ebp-ACh] BYREF
    float start[3]; // [esp+5Ch] [ebp-A0h] BYREF
    float end[3]; // [esp+68h] [ebp-94h] BYREF
    TurretInfo *pTurretInfo; // [esp+74h] [ebp-88h]
    DObjAnimMat *aimMtx; // [esp+78h] [ebp-84h]
    float angles[3]; // [esp+7Ch] [ebp-80h] BYREF
    float baseMtx[4][3]; // [esp+88h] [ebp-74h] BYREF
    trace_t trace; // [esp+B8h] [ebp-44h] BYREF
    int i; // [esp+F4h] [ebp-8h]
    int numSteps; // [esp+F8h] [ebp-4h]

    numSteps = 30;
    pTurretInfo = self->pTurretInfo;
    if ( !pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 2375, 0, "%s", "pTurretInfo") )
    {
        __debugbreak();
    }
    obj = Com_GetServerDObj(self->s.number);
    boneIndex = -2;
    if ( DObjGetBoneIndex(obj, scr_const.tag_aim_pitch, &boneIndex, -1) )
        aimMtx = G_DObjGetLocalTagMatrix(self, scr_const.tag_aim_pitch);
    else
        aimMtx = G_DObjGetLocalTagMatrix(self, scr_const.tag_aim);
    if ( aimMtx )
    {
        weaponMtx = G_DObjGetLocalTagMatrix(self, scr_const.tag_butt);
        if ( weaponMtx )
        {
            AnglesToAxis(self->r.currentAngles, baseMtx);
            baseMtx[3][0] = self->r.currentOrigin[0];
            baseMtx[3][1] = self->r.currentOrigin[1];
            baseMtx[3][2] = self->r.currentOrigin[2];
            dir[0] = weaponMtx->trans[0] - aimMtx->trans[0];
            dir[1] = weaponMtx->trans[1] - aimMtx->trans[1];
            dir[2] = weaponMtx->trans[2] - aimMtx->trans[2];
            MatrixTransformVector43(aimMtx->trans, baseMtx, start);
            for ( i = 0; i <= 30; ++i )
            {
                angles[0] = (float)(-90.0 / 30.0) * (float)i;
                angles[1] = 0.0f;
                angles[2] = 0.0f;
                AnglesToAxis(angles, mtx);
                MatrixTransformVector(dir, mtx, transDir);
                transDir[0] = aimMtx->trans[0] + transDir[0];
                transDir[1] = aimMtx->trans[1] + transDir[1];
                transDir[2] = aimMtx->trans[2] + transDir[2];
                MatrixTransformVector43(transDir, baseMtx, end);
                G_LocationalTrace(&trace, start, end, self->s.number, (int)0x800811, bulletPriorityMap, 0);
                if ( trace.fraction < 1.0 )
                {
                    pTurretInfo->dropPitch = angles[0] + self->s.lerp.u.turret.gunAngles[0];
                    return;
                }
            }
        }
    }
}

void __cdecl turret_think(gentity_s *self)
{
    gentity_s *v1; // [esp+0h] [ebp-30h]
    TurretInfo *pTurretInfo; // [esp+24h] [ebp-Ch]
    bool overheatFiring; // [esp+2Bh] [ebp-5h]
    const WeaponDef *weapDef; // [esp+2Ch] [ebp-4h]

    overheatFiring = 0;

    PROF_SCOPED("Turret_think");

    pTurretInfo = self->pTurretInfo;
    if ( !pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 2442, 0, "%s", "pTurretInfo") )
    {
        __debugbreak();
    }
    self->nextthink = level.time + 50;
    self->s.lerp.u.actor.team = 0;
    if ( self->tagInfo )
        G_GeneralLink(self);
    if ( self->r.ownerNum.isDefined() )
        v1 = self->r.ownerNum.ent();
    else
        v1 = &g_entities[1023];
    if ( v1 && v1->client && (self->s.lerp.eFlags & 0x40) != 0 )
        overheatFiring = 1;
    weapDef = BG_GetWeaponDef(self->s.weapon);
    if ( weapDef->overheatWeapon && (!overheatFiring || weapDef->coolWhileFiring) )
    {
        self->pTurretInfo->heatVal = self->pTurretInfo->heatVal
                                                             - (float)((float)((float)(level.time - level.previousTime) * weapDef->cooldownRate)
                                                                             / 1000.0);
        if ( self->pTurretInfo->heatVal < 0.0 )
            self->pTurretInfo->heatVal = 0.0f;
        if ( weapDef->overheatEndVal >= self->pTurretInfo->heatVal )
            self->pTurretInfo->overheating = 0;
    }
    turret_UpdateSound(self);
    if ( (pTurretInfo->flags & 2) == 0 || (pTurretInfo->flags & 1) != 0 )
    {
        turret_ReturnToDefaultPos(self, 0);
    }
    else
    {
        turret_think_auto_nonai(self);
    }
}

int __cdecl turret_ReturnToDefaultPos(gentity_s *self, int bManned)
{
    float dropPitch; // [esp+0h] [ebp-10h]
    float desiredAngles[2]; // [esp+4h] [ebp-Ch] BYREF
    TurretInfo *pTurretInfo; // [esp+Ch] [ebp-4h]

    pTurretInfo = self->pTurretInfo;
    if ( !pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 1132, 0, "%s", "pTurretInfo") )
    {
        __debugbreak();
    }
    if ( bManned )
        dropPitch = 0.0f;
    else
        dropPitch = pTurretInfo->dropPitch;
    desiredAngles[0] = dropPitch;
    desiredAngles[1] = 0.0f;
    turret_SetState(self, 0);
    return turret_UpdateTargetAngles(self, desiredAngles, bManned);
}

int __cdecl turret_UpdateTargetAngles(gentity_s *self, const float *desiredAngles, int bManned)
{
    float v4; // [esp+4h] [ebp-4Ch]
    float v5; // [esp+8h] [ebp-48h]
    float v6; // [esp+Ch] [ebp-44h]
    float v7; // [esp+10h] [ebp-40h]
    const WeaponDef *turretDef; // [esp+30h] [ebp-20h]
    float fDelta; // [esp+34h] [ebp-1Ch]
    TurretInfo *pTurretInfo; // [esp+38h] [ebp-18h]
    int bComplete; // [esp+3Ch] [ebp-14h]
    float fSpeed[2]; // [esp+44h] [ebp-Ch] BYREF
    int i; // [esp+4Ch] [ebp-4h]

    pTurretInfo = self->pTurretInfo;
    if ( !pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 995, 0, "%s", "pTurretInfo") )
    {
        __debugbreak();
    }
    Turret_ScanStop(self);
    bComplete = 1;
    turret_CalculateConvergenceAngularVelocity(self, desiredAngles, fSpeed);
    if ( bManned )
    {
        turretDef = BG_GetWeaponDef(self->s.weapon);
        if ( (float)(turretDef->maxTurnSpeed[0] - fSpeed[0]) < 0.0 )
            v7 = turretDef->maxTurnSpeed[0];
        else
            v7 = fSpeed[0];
        fSpeed[0] = v7;
        if ( (float)(v7 - turretDef->minTurnSpeed[0]) < 0.0 )
            v6 = turretDef->minTurnSpeed[0];
        else
            v6 = fSpeed[0];
        fSpeed[0] = v6;
        if ( (float)(turretDef->maxTurnSpeed[1] - fSpeed[1]) < 0.0 )
            v5 = turretDef->maxTurnSpeed[1];
        else
            v5 = fSpeed[1];
        fSpeed[1] = v5;
        if ( (float)(v5 - turretDef->minTurnSpeed[1]) < 0.0 )
            v4 = turretDef->minTurnSpeed[1];
        else
            v4 = fSpeed[1];
        fSpeed[1] = v4;
    }
    else
    {
        fSpeed[0] = 200.0f;
        fSpeed[1] = 200.0f;
    }

    static const float INITIAL_PITCH_SPEED = 360.0;

    if ( (pTurretInfo->flags & 0x200) != 0 && (pTurretInfo->flags & 0x100) != 0 )
        fSpeed[0] = INITIAL_PITCH_SPEED;

    for ( i = 0; i < 2; ++i )
    {
        fSpeed[i] = fSpeed[i] * 0.050000001;
        if ( fSpeed[i] < 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 1032, 0, "%s", "fSpeed[i] >= 0") )
        {
            __debugbreak();
        }
        fDelta = AngleNormalize180(desiredAngles[i] - self->s.lerp.u.turret.gunAngles[i]);
        if ( fDelta <= fSpeed[i] )
        {
            if ( (-(fSpeed[i])) > fDelta )
            {
                bComplete = 0;
                fDelta = -fSpeed[i];
            }
        }
        else
        {
            bComplete = 0;
            fDelta = fSpeed[i];
        }
        self->s.lerp.u.turret.gunAngles[i] = self->s.lerp.u.turret.gunAngles[i] + fDelta;
    }
    if ( (pTurretInfo->flags & 0x200) != 0 )
    {
        if ( (pTurretInfo->flags & 0x400) != 0 )
        {
            if ( pTurretInfo->pitchCap <= self->s.lerp.u.turret.gunAngles[0] )
                pTurretInfo->flags &= ~0x100u;
            else
                self->s.lerp.u.turret.gunAngles[2] = self->s.lerp.u.turret.gunAngles[0] - pTurretInfo->pitchCap;
        }
        else if ( self->s.lerp.u.turret.gunAngles[0] <= pTurretInfo->pitchCap )
        {
            pTurretInfo->flags &= ~0x100u;
        }
        else
        {
            self->s.lerp.u.turret.gunAngles[2] = self->s.lerp.u.turret.gunAngles[0] - pTurretInfo->pitchCap;
        }
    }
    return bComplete;
}

void __cdecl turret_CalculateConvergenceAngularVelocity(
                const gentity_s *self,
                const float *desiredAngles,
                float *angularVelocity)
{
    float v3; // [esp+4h] [ebp-38h]
    float v4; // [esp+14h] [ebp-28h]
    const TurretInfo *turretInfo; // [esp+24h] [ebp-18h]
    int remainingYawConvergenceTime; // [esp+30h] [ebp-Ch]
    int nextFrameTime; // [esp+34h] [ebp-8h]
    int remainingPitchConvergenceTime; // [esp+38h] [ebp-4h]

    turretInfo = self->pTurretInfo;
    if ( !turretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 794, 0, "%s", "turretInfo") )
    {
        __debugbreak();
    }
    if ( !angularVelocity
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 795, 0, "%s", "angularVelocity") )
    {
        __debugbreak();
    }
    remainingYawConvergenceTime = Turret_GetRemainingConvergenceTime(turretInfo, 1u);
    remainingPitchConvergenceTime = Turret_GetRemainingConvergenceTime(turretInfo, 0);
    nextFrameTime = self->nextthink - level.time;
    v4 = AngleNormalize180(self->s.lerp.u.turret.gunAngles[0] - *desiredAngles);
    v3 = AngleNormalize180(self->s.lerp.u.turret.gunAngles[1] - desiredAngles[1]);
    *angularVelocity = (float)(fabs(v4) * 1000.0) / (float)nextFrameTime;
    angularVelocity[1] = (float)(fabs(v3) * 1000.0) / (float)nextFrameTime;
    if ( remainingYawConvergenceTime > 0 && nextFrameTime < remainingYawConvergenceTime )
        angularVelocity[1] = (float)((float)nextFrameTime / (float)remainingYawConvergenceTime) * angularVelocity[1];
    if ( remainingPitchConvergenceTime > 0 && nextFrameTime < remainingPitchConvergenceTime )
        *angularVelocity = (float)((float)nextFrameTime / (float)remainingPitchConvergenceTime) * *angularVelocity;
}

int __cdecl Turret_GetRemainingConvergenceTime(const TurretInfo *turretInfo, unsigned int type)
{
    if ( type >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp",
                    100,
                    0,
                    "%s",
                    "type == PITCH || type == YAW") )
    {
        __debugbreak();
    }
    return turretInfo->convergenceTime[type] - (level.time - turretInfo->targetTime);
}

void __cdecl Turret_ScanStop(gentity_s *self)
{
    TurretInfo *turret; // [esp+0h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 890, 0, "%s", "self") )
        __debugbreak();
    if ( !self->pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 891, 0, "%s", "self->pTurretInfo") )
    {
        __debugbreak();
    }
    turret = self->pTurretInfo;
    if ( (turret->flags & 0x40000) != 0 )
        Scr_Notify(self, scr_const.turret_scan_stop, 0);
    turret->flags &= ~0x40000u;
    turret->flags ^= 0x80000u;
    turret->scanSpeed = 0.0f;
    turret->scanDecelYaw = 0.0f;
    turret->scanPauseTime = 0;
}

void __cdecl turret_think_auto_nonai(gentity_s *self)
{
    TurretInfo *turretInfo; // [esp+0h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 1997, 0, "%s", "self") )
        __debugbreak();
    turretInfo = self->pTurretInfo;
    if ( !turretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 2000, 0, "%s", "turretInfo") )
    {
        __debugbreak();
    }
    if ( (turretInfo->flags & 0x10000) != 0 )
        turret_target_sentient(self, turretInfo);
    if ( (turretInfo->flags & 0x20000) != 0 )
        turret_target_killstreak(self, turretInfo);
}

void __cdecl turret_target_sentient(gentity_s *self, TurretInfo *turretInfo)
{
    gentity_s *v2; // [esp+0h] [ebp-Ch]
    sentient_s *target_sent; // [esp+4h] [ebp-8h]
    gentity_s *target_ent; // [esp+8h] [ebp-4h]

    target_sent = turret_findBestTarget(self);
    if ( turretInfo->manualTarget.isDefined() )
        v2 = turretInfo->manualTarget.ent();
    else
        v2 = 0;
    target_ent = v2;
    if ( target_sent )
    {
        if ( turret_aimat_Sentient(self, target_sent, 1, turretInfo->convergenceTime[1]) )
            return;
        if ( v2 == target_sent->ent )
            target_ent = 0;
    }
    if ( target_ent )
    {
        if ( target_ent->sentient )
        {
            if ( turret_isTargetVisible(self, target_ent, 0)
                && turret_aimat_Sentient(self, target_ent->sentient, 1, turretInfo->convergenceTime[1]) )
            {
                return;
            }
        }
        else if ( turret_aimat_Ent(self, target_ent, 1) )
        {
            return;
        }
    }
    turret_ClearTargetEnt(self);
    if ( ((turretInfo->flags & 0x10000) != 0 || (turretInfo->flags & 0x20000) != 0) && (turretInfo->flags & 0x100000) == 0 )
        Turret_ScanForEnemies(self);
    else
        turret_ReturnToDefaultPos(self, 1);
}

void __cdecl Turret_ScanForEnemies(gentity_s *self)
{
    float v1; // [esp+10h] [ebp-30h]
    float v2; // [esp+14h] [ebp-2Ch]
    TurretInfo *turret; // [esp+20h] [ebp-20h]
    float sign; // [esp+28h] [ebp-18h]
    const WeaponDef *weapDef; // [esp+30h] [ebp-10h]
    float deltaYaw; // [esp+3Ch] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 922, 0, "%s", "self") )
        __debugbreak();
    if ( !self->pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 923, 0, "%s", "self->pTurretInfo") )
    {
        __debugbreak();
    }
    turret = self->pTurretInfo;
    weapDef = BG_GetWeaponDef(self->s.weapon);
    if ( weapDef->scanSpeed <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp",
                    928,
                    0,
                    "%s",
                    "weapDef->scanSpeed > 0.0f") )
    {
        __debugbreak();
    }
    if ( weapDef->scanAccel <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp",
                    929,
                    0,
                    "%s",
                    "weapDef->scanAccel > 0.0f") )
    {
        __debugbreak();
    }
    self->s.lerp.u.turret.gunAngles[0] = LinearTrackAngle(
                                                                                 turret->scanningPitch,
                                                                                 self->s.lerp.u.turret.gunAngles[0],
                                                                                 weapDef->scanSpeed,
                                                                                 0.050000001);
    if ( turret->scanningPitch == self->s.lerp.u.turret.gunAngles[0] )
    {
        self->s.lerp.u.turret.flags &= ~0x20u;
        if ( (float)((float)(turret->arcmin[1] + 4.0) - -90.0) < 0.0 )
            v2 = -90.0f;
        else
            v2 = turret->arcmin[1] + 4.0;
        if ( (float)(90.0 - (float)(turret->arcmax[1] - 4.0)) < 0.0 )
            v1 = 90.0f;
        else
            v1 = turret->arcmax[1] - 4.0;
        if ( (turret->flags & 0x80000) != 0 )
        {
            deltaYaw = self->s.lerp.u.turret.gunAngles[1] - v2;
            sign = -1.0f;
        }
        else
        {
            deltaYaw = v1 - self->s.lerp.u.turret.gunAngles[1];
            sign = 1.0f;
        }
        if ( (turret->flags & 0x40000) == 0 )
            Turret_ScanStart(self, weapDef, deltaYaw);
        if ( level.time >= turret->scanPauseTime )
        {
            if ( turret->scanDecelYaw <= deltaYaw )
            {
                turret->scanSpeed = (float)(weapDef->scanAccel * 0.050000001) + turret->scanSpeed;
                if ( turret->scanSpeed >= weapDef->scanSpeed )
                    turret->scanSpeed = weapDef->scanSpeed;
                self->s.lerp.u.turret.gunAngles[1] = (float)((float)(turret->scanSpeed * 0.050000001) * sign)
                                                                                     + self->s.lerp.u.turret.gunAngles[1];
            }
            else
            {
                turret->scanSpeed = turret->scanSpeed - (float)(weapDef->scanAccel * 0.050000001);
                if ( turret->scanSpeed > 0.0 )
                    self->s.lerp.u.turret.gunAngles[1] = (float)((float)(turret->scanSpeed * 0.050000001) * sign)
                                                                                         + self->s.lerp.u.turret.gunAngles[1];
                else
                    Turret_ScanStop(self);
            }
        }
    }
}

void __cdecl Turret_ScanStart(gentity_s *self, const WeaponDef *weapDef, float deltaYaw)
{
    TurretInfo *turret; // [esp+0h] [ebp-8h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 856, 0, "%s", "self") )
        __debugbreak();
    if ( !self->pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 857, 0, "%s", "self->pTurretInfo") )
    {
        __debugbreak();
    }
    turret = self->pTurretInfo;
    if ( (turret->flags & 0x40000) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp",
                    861,
                    0,
                    "%s",
                    "!(turret->flags & TURRET_SCANNING)") )
    {
        __debugbreak();
    }
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 863, 0, "%s", "weapDef") )
        __debugbreak();
    if ( weapDef->scanSpeed <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp",
                    864,
                    0,
                    "%s",
                    "weapDef->scanSpeed > 0.0f") )
    {
        __debugbreak();
    }
    if ( weapDef->scanAccel <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp",
                    865,
                    0,
                    "%s",
                    "weapDef->scanAccel > 0.0f") )
    {
        __debugbreak();
    }
    turret->flags |= 0x40000u;
    turret->scanSpeed = 0.0f;
    turret->scanPauseTime = weapDef->scanPauseTime + level.time;
    turret->scanDecelYaw = (float)(weapDef->scanSpeed * weapDef->scanSpeed) / (float)(2.0 * weapDef->scanAccel);
    if ( turret->scanDecelYaw > (float)(deltaYaw * 0.5) )
        turret->scanDecelYaw = deltaYaw * 0.5;
    Scr_Notify(self, scr_const.turret_scan_start, 0);
}

int __cdecl turret_aimat_Sentient(gentity_s *self, sentient_s *enemy, int bShoot, int missTime)
{
    float desiredAngles[2]; // [esp+0h] [ebp-8h] BYREF

    if ( !turret_aimat_Sentient_Internal(self, enemy, bShoot, missTime, desiredAngles) )
        return 0;
    turret_UpdateTargetAngles(self, desiredAngles, 1);
    return 1;
}

int __cdecl turret_aimat_Sentient_Internal(
                gentity_s *self,
                sentient_s *enemy,
                int bShoot,
                int missTime,
                float *desiredAngles)
{
    float *v6; // [esp+Ch] [ebp-C4h]
    float *v7; // [esp+1Ch] [ebp-B4h]
    float *missOffsetNormalized; // [esp+2Ch] [ebp-A4h]
    float divergence; // [esp+48h] [ebp-88h]
    float origin[3]; // [esp+4Ch] [ebp-84h] BYREF
    float muzzlePosition[3]; // [esp+58h] [ebp-78h] BYREF
    float right[2]; // [esp+64h] [ebp-6Ch] BYREF
    float targetPosition[3]; // [esp+6Ch] [ebp-64h] BYREF
    float missOffsetInner[3]; // [esp+78h] [ebp-58h]
    float tagMat[4][3]; // [esp+84h] [ebp-4Ch] BYREF
    float missOffsetOuter[3]; // [esp+B4h] [ebp-1Ch]
    TurretInfo *pTurretInfo; // [esp+C0h] [ebp-10h]
    float missTarget[3]; // [esp+C4h] [ebp-Ch] BYREF

    if ( missTime < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 1466, 0, "%s", "missTime >= 0") )
    {
        __debugbreak();
    }
    turret_SetTargetEnt(self, enemy->ent);
    if ( !turret_CanTargetSentient(self, enemy, targetPosition, muzzlePosition, desiredAngles) )
        return 0;
    pTurretInfo = self->pTurretInfo;
    if ( !pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 1474, 0, "%s", "pTurretInfo") )
    {
        __debugbreak();
    }
    if ( Turret_GetRemainingConvergenceTime(pTurretInfo, 1u) > 0 )
    {
        if ( (pTurretInfo->flags & 8) != 0 )
        {
            if ( missTime <= 0 )
                divergence = 0.0f;
            else
                divergence = (float)(missTime + pTurretInfo->targetTime - level.time) / (float)missTime;
            missOffsetOuter[0] = (float)(divergence * 30.0) * pTurretInfo->missOffsetNormalized[0];
            missOffsetOuter[1] = (float)(divergence * 30.0) * pTurretInfo->missOffsetNormalized[1];
            missOffsetOuter[2] = (float)(divergence * 30.0) * pTurretInfo->missOffsetNormalized[2];
            missOffsetInner[0] = 33.0 * pTurretInfo->missOffsetNormalized[0];
            missOffsetInner[1] = 33.0 * pTurretInfo->missOffsetNormalized[1];
            missOffsetInner[2] = 33.0 * pTurretInfo->missOffsetNormalized[2];
            missTarget[0] = targetPosition[0] + (float)(missOffsetInner[0] + missOffsetOuter[0]);
            missTarget[1] = targetPosition[1] + (float)(missOffsetInner[1] + missOffsetOuter[1]);
            missTarget[2] = targetPosition[2] + (float)(missOffsetInner[2] + missOffsetOuter[2]);
            if ( turret_aimat_vector(self, missTarget, bShoot, desiredAngles) )
                return 1;
        }
        else
        {
            pTurretInfo->flags |= 8u;
            if ( !G_DObjGetWorldTagMatrix(self, scr_const.tag_aim, tagMat) )
                return 0;
            right[0] = targetPosition[1] - tagMat[3][1];
            right[1] = tagMat[3][0] - targetPosition[0];
            Vec2Normalize(right);
            right[0] = 63.0 * right[0];
            right[1] = 63.0 * right[1];
            if ( (float)((float)(tagMat[0][0] * right[0]) + (float)(tagMat[0][1] * right[1])) < 0.0 )
            {
                origin[0] = targetPosition[0] - right[0];
                origin[1] = targetPosition[1] - right[1];
            }
            else
            {
                origin[0] = targetPosition[0] + right[0];
                origin[1] = targetPosition[1] + right[1];
            }
            origin[2] = enemy->ent->r.currentOrigin[2];
            if ( turret_aimat_vector(self, origin, bShoot, desiredAngles) )
            {
                missOffsetNormalized = pTurretInfo->missOffsetNormalized;
                pTurretInfo->missOffsetNormalized[0] = origin[0] - targetPosition[0];
                missOffsetNormalized[1] = origin[1] - targetPosition[1];
                missOffsetNormalized[2] = origin[2] - targetPosition[2];
                Vec3Normalize(pTurretInfo->missOffsetNormalized);
                return 1;
            }
            origin[2] = targetPosition[2];
            if ( turret_aimat_vector(self, origin, bShoot, desiredAngles) )
            {
                v7 = pTurretInfo->missOffsetNormalized;
                pTurretInfo->missOffsetNormalized[0] = origin[0] - targetPosition[0];
                v7[1] = origin[1] - targetPosition[1];
                v7[2] = origin[2] - targetPosition[2];
                Vec3Normalize(pTurretInfo->missOffsetNormalized);
                return 1;
            }
            v6 = pTurretInfo->missOffsetNormalized;
            pTurretInfo->missOffsetNormalized[0] = 0.0f;
            v6[1] = 0.0f;
            v6[2] = 0.0f;
        }
    }
    turret_aimat_vector_internal(self, targetPosition, bShoot, desiredAngles);
    return 1;
}

void __cdecl turret_aimat_vector_internal(gentity_s *self, const float *origin, int bShoot, const float *desiredAngles)
{
    int state; // [esp+4h] [ebp-38h]
    float v5; // [esp+Ch] [ebp-30h]
    float v6; // [esp+1Ch] [ebp-20h]
    TurretInfo *turretInfo; // [esp+2Ch] [ebp-10h]
    bool alwaysCheckAngles; // [esp+33h] [ebp-9h]
    int remainingConvergenceTime; // [esp+34h] [ebp-8h]

    turretInfo = self->pTurretInfo;
    if ( !turretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 1344, 0, "%s", "turretInfo") )
    {
        __debugbreak();
    }
    alwaysCheckAngles = (turretInfo->flags & 0x20000) != 0;
    if ( !bShoot )
    {
        turret_SetState(self, 0);
        goto LABEL_17;
    }
    remainingConvergenceTime = Turret_GetRemainingConvergenceTime(turretInfo, 1u);
    state = turretInfo->state;
    if ( state )
    {
        if ( state != 2 )
            goto LABEL_17;
        goto LABEL_13;
    }
    if ( level.time >= turretInfo->targetTime + 250 && !alwaysCheckAngles
        || (v6 = AngleNormalize180(self->s.lerp.u.turret.gunAngles[0] - *desiredAngles),
                fabs(v6) < 5.0)
        && (v5 = AngleNormalize180(self->s.lerp.u.turret.gunAngles[1] - desiredAngles[1]),
                fabs(v5) < 5.0) )
    {
        turret_SetState(self, 2);
LABEL_13:
        BG_GetWeaponDef(self->s.weapon);
        if ( remainingConvergenceTime < 0 )
            turret_SetState(self, 1);
    }
LABEL_17:
    turretInfo->flags |= 0x40u;
    turretInfo->targetPos[0] = *origin;
    turretInfo->targetPos[1] = origin[1];
    turretInfo->targetPos[2] = origin[2];
}

int __cdecl turret_aimat_vector(gentity_s *self, float *origin, int bShoot, float *desiredAngles)
{
    float vSource[3]; // [esp+0h] [ebp-Ch] BYREF

    if ( turret_CanTargetPoint(self, origin, vSource, desiredAngles) )
    {
        turret_aimat_vector_internal(self, origin, bShoot, desiredAngles);
        return 1;
    }
    else
    {
        if ( !self->pTurretInfo
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 1404, 0, "%s", "self->pTurretInfo") )
        {
            __debugbreak();
        }
        self->pTurretInfo->flags &= ~0x40u;
        return 0;
    }
}

void __cdecl turret_SetTargetEnt(gentity_s *self, gentity_s *ent)
{
    gentity_s *v2; // [esp+0h] [ebp-Ch]
    TurretInfo *pTurretInfo; // [esp+8h] [ebp-4h]

    pTurretInfo = self->pTurretInfo;
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 1427, 0, "%s", "ent") )
        __debugbreak();
    if ( !pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 1428, 0, "%s", "pTurretInfo") )
    {
        __debugbreak();
    }
    if ( pTurretInfo->target.isDefined() )
        v2 = pTurretInfo->target.ent();
    else
        v2 = 0;
    if ( v2 == ent )
    {
        if ( level.time > turret_KillstreakTargetTime->current.integer + pTurretInfo->targetTime )
            pTurretInfo->targetTime = level.time;
    }
    else
    {
        pTurretInfo->target.setEnt(ent);
        pTurretInfo->targetTime = level.time;
        pTurretInfo->flags &= ~8u;
    }
}

int __cdecl turret_aimat_Ent(gentity_s *self, gentity_s *ent, int bShoot)
{
    float desiredAngles[2]; // [esp+0h] [ebp-8h] BYREF

    turret_SetTargetEnt(self, ent);
    if ( !turret_aimat_vector(self, ent->r.currentOrigin, bShoot, desiredAngles) )
        return 0;
    turret_UpdateTargetAngles(self, desiredAngles, 1);
    return 1;
}

int __cdecl turret_isTargetVisible(gentity_s *self, const gentity_s *target, float *distSqr)
{
    TurretInfo *turretInfo; // [esp+10h] [ebp-30h]
    float distSquared; // [esp+14h] [ebp-2Ch]
    float flashOrigin[3]; // [esp+18h] [ebp-28h] BYREF
    float angles[3]; // [esp+24h] [ebp-1Ch] BYREF
    float targetOrigin[3]; // [esp+34h] [ebp-Ch] BYREF

    if ( !target && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 1705, 0, "%s", "target") )
        __debugbreak();
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 1706, 0, "%s", "self") )
        __debugbreak();
    turretInfo = self->pTurretInfo;
    if ( !turretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 1709, 0, "%s", "turretInfo") )
    {
        __debugbreak();
    }
    if ( target->client && G_IsPlayerDrivingVehicle(target) )
        return 0;
    distSquared = Vec3DistanceSq(self->r.currentOrigin, target->r.currentOrigin);
    if ( (turretInfo->flags & 0x20000) == 0 && distSquared >= turretInfo->maxRangeSquared )
        return 0;
    if ( target->sentient )
    {
        if ( !turret_CanTargetSentient(self, target->sentient, targetOrigin, flashOrigin, angles) )
            return 0;
        Sentient_GetEyePosition(target->sentient, targetOrigin);
    }
    else
    {
        if ( !turret_CanTargetPoint(self, target->r.currentOrigin, flashOrigin, angles) )
            return 0;
        targetOrigin[0] = target->r.currentOrigin[0];
        targetOrigin[1] = target->r.currentOrigin[1];
        targetOrigin[2] = target->r.currentOrigin[2];
    }
    if ( turret_SightTrace(flashOrigin, targetOrigin, self->s.number, target->s.number, self) )
        return 0;
    if ( distSqr )
        *distSqr = distSquared;
    return 1;
}

sentient_s *__cdecl turret_findBestTarget(gentity_s *self)
{
    float distSqr; // [esp+30h] [ebp-18h] BYREF
    float curDistSqr; // [esp+34h] [ebp-14h]
    TurretInfo *pTurretInfo; // [esp+38h] [ebp-10h]
    sentient_s *sent; // [esp+3Ch] [ebp-Ch]
    sentient_s *cur; // [esp+40h] [ebp-8h]
    int teamFlags; // [esp+44h] [ebp-4h]

    pTurretInfo = self->pTurretInfo;
    if ( !pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 1839, 0, "%s", "pTurretInfo") )
    {
        __debugbreak();
    }
    cur = 0;
    if ( (pTurretInfo->flags & 0x40) != 0
        && pTurretInfo->target.isDefined()
        && pTurretInfo->target.ent()->sentient)
    {
        if ( pTurretInfo->target.ent()->client && pTurretInfo->target.ent()->client->ps.pm_type >= 9)
        {
            cur = 0;
        }
        else
        {
            if ( level.time <= turret_SentryTargetTime->current.integer + pTurretInfo->targetTime )
                return pTurretInfo->target.ent()->sentient;
            cur = pTurretInfo->target.ent()->sentient;
        }
    }
    if ( cur )
    {
        if ( cur->bIgnoreMe )
        {
            cur = 0;
        }
        else if ( !turret_isTargetVisible(self, cur->ent, &distSqr) )
        {
            cur = 0;
        }
    }
    teamFlags = 1 << Sentient_EnemyTeam(pTurretInfo->eTeam);
    if ( pTurretInfo->prevSentTarget <= -1 )
        sent = Sentient_FirstSentient(teamFlags);
    else
        sent = Sentient_NextSentient(&level.sentients[pTurretInfo->prevSentTarget], teamFlags);
    if ( sent )
    {
        pTurretInfo->prevSentTarget = sent - level.sentients;
        if ( sent->bIgnoreMe )
        {
            return cur;
        }
        else if ( sent->ent->s.number == self->s.otherEntityNum )
        {
            return cur;
        }
        else if ( sent->ent->client
                     && (sent->ent->client->ps.pm_type == 4 || sent->ent->client->sess.sessionState == SESS_STATE_SPECTATOR) )
        {
            return cur;
        }
        else if ( sent->ent->client && sent->ent->client->ps.pm_type >= 9 )
        {
            return cur;
        }
        else if ( sent->ent->sentient->scriptOwner.isDefined() 
            && sent->ent->sentient->scriptOwner.entnum() == self->s.otherEntityNum)
        {
            return cur;
        }
        else if ( turret_isTargetVisible(self, sent->ent, &distSqr) )
        {
            if ( sent->ent->client && (sent->ent->client->ps.perks[1] & 8) != 0 )
            {
                return cur;
            }
            else if ( cur )
            {
                curDistSqr = Vec3DistanceSq(self->r.currentOrigin, cur->ent->r.currentOrigin);
                if ( curDistSqr <= distSqr )
                    return cur;
                else
                    return sent;
            }
            else
            {
                return sent;
            }
        }
        else
        {
            return cur;
        }
    }
    else
    {
        pTurretInfo->prevSentTarget = -1;
        return cur;
    }
}

void __cdecl turret_target_killstreak(gentity_s *self, TurretInfo *turretInfo)
{
    gentity_s *v2; // [esp+0h] [ebp-8h]
    gentity_s *target_ent; // [esp+4h] [ebp-4h]

    target_ent = turret_findBestKillstreakTarget(self);
    //if ( EntHandle::isDefined(&turretInfo->manualTarget) )
    if (turretInfo->manualTarget.isDefined())
    {
        //v2 = EntHandle::ent(&turretInfo->manualTarget);
        v2 = turretInfo->manualTarget.ent();
    }
    else
        v2 = target_ent;
    if ( !v2 || !turret_aimat_Ent(self, v2, 1) )
    {
        turret_ClearTargetEnt(self);
        if ( ((turretInfo->flags & 0x10000) != 0 || (turretInfo->flags & 0x20000) != 0)
            && (turretInfo->flags & 0x100000) == 0 )
        {
            Turret_ScanForEnemies(self);
        }
        else
        {
            turret_ReturnToDefaultPos(self, 1);
        }
        Scr_Notify(self, scr_const.turretstatechange, 0);
    }
}

gentity_s *__cdecl turret_findBestKillstreakTarget(gentity_s *self)
{
    gentity_s *v1; // eax
    int i; // [esp+0h] [ebp-1Ch]
    float distSqr; // [esp+4h] [ebp-18h] BYREF
    const target_t *targets; // [esp+8h] [ebp-14h]
    TurretInfo *pTurretInfo; // [esp+Ch] [ebp-10h]
    gentity_s *ent; // [esp+10h] [ebp-Ch]
    gentity_s *cur; // [esp+14h] [ebp-8h]
    int teamFlags; // [esp+18h] [ebp-4h]

    pTurretInfo = self->pTurretInfo;
    if ( !pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 1778, 0, "%s", "pTurretInfo") )
    {
        __debugbreak();
    }
    cur = 0;
    if ( (pTurretInfo->flags & 0x40) != 0
        //&& EntHandle::isDefined(&pTurretInfo->target)
        && pTurretInfo->target.isDefined()
        //&& EntHandle::ent(&pTurretInfo->target) )
        && pTurretInfo->target.ent() )
    {
        //v1 = EntHandle::ent(&pTurretInfo->target);
        v1 = pTurretInfo->target.ent();
        if ( Target_GetTargetIndex(v1) == 32 )
        {
            cur = 0;
        }
        else
        {
            if (level.time <= turret_KillstreakTargetTime->current.integer + pTurretInfo->targetTime)
            {
                //return EntHandle::ent(&pTurretInfo->target);
                return pTurretInfo->target.ent();
            }
            //cur = EntHandle::ent(&pTurretInfo->target);
            cur = pTurretInfo->target.ent();
        }
    }
    targets = (const target_t *)Target_GetTargetArray();
    for ( i = 0; i < 32; ++i )
    {
        if ( targets[i].ent )
        {
            if ( cur && !turret_isTargetVisible(self, cur, &distSqr) )
                cur = 0;
            ent = targets[i].ent;
            teamFlags = ent->s.faction.iHeadIconTeam & 3;
            if ( (pTurretInfo->eTeam != teamFlags || pTurretInfo->eTeam == TEAM_FREE)
                && self->s.otherEntityNum != (int)ent->s.faction.iHeadIconTeam >> 2
                && (targets[i].flags & 4) == 0
                && turret_isTargetVisible(self, ent, &distSqr) )
            {
                turret_SetTargetEnt(self, ent);
                return ent;
            }
        }
    }
    return cur;
}

void __cdecl turret_controller(const gentity_s *self, int *partBits)
{
    unsigned __int8 boneIndex; // [esp+3h] [ebp-11h] BYREF
    DObj *obj; // [esp+4h] [ebp-10h]
    float angles[3]; // [esp+8h] [ebp-Ch] BYREF

    obj = Com_GetServerDObj(self->s.number);
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 2677, 0, "%s", "obj") )
        __debugbreak();
    memset(angles, 0, sizeof(angles));
    angles[0] = self->s.lerp.u.turret.gunAngles[0] - self->s.lerp.u.turret.gunAngles[2];
    boneIndex = -2;
    DObjGetBoneIndex(obj, scr_const.tag_aim_pitch, &boneIndex, -1);
    if ( boneIndex != 255 )
    {
        if ( boneIndex == 254
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp",
                        2685,
                        0,
                        "%s",
                        "boneIndex != UNDEFINED_BONEINDEX") )
        {
            __debugbreak();
        }
        DObjSetLocalTag(obj, partBits, boneIndex, vec3_origin, angles);
        angles[0] = 0.0f;
    }
    angles[1] = self->s.lerp.u.turret.gunAngles[1];
    angles[2] = 0.0f;
    boneIndex = -2;
    DObjGetBoneIndex(obj, scr_const.tag_aim, &boneIndex, -1);
    DObjSetControlTagAngles(obj, partBits, boneIndex, angles);
    boneIndex = -2;
    DObjGetBoneIndex(obj, scr_const.tag_aim_animated, &boneIndex, -1);
    DObjSetControlTagAngles(obj, partBits, boneIndex, angles);
    angles[0] = self->s.lerp.u.turret.gunAngles[2];
    angles[1] = 0.0f;
    boneIndex = -2;
    DObjGetBoneIndex(obj, scr_const.tag_flash, &boneIndex, -1);
    DObjSetControlTagAngles(obj, partBits, boneIndex, angles);
}

void __cdecl G_FreeTurret(gentity_s *self)
{
    gentity_s *v1; // [esp+0h] [ebp-Ch]
    TurretInfo *pTurretInfo; // [esp+8h] [ebp-4h]

    //if ( EntHandle::isDefined(&self->r.ownerNum) )
    if ( self->r.ownerNum.isDefined() )
        v1 = self->r.ownerNum.ent();
    else
        v1 = &g_entities[1023];
    pTurretInfo = self->pTurretInfo;
    if ( !pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 2777, 0, "%s", "pTurretInfo") )
    {
        __debugbreak();
    }
    if ( v1->client )
        G_ClientStopUsingTurret(self);
    G_DeactivateTurret(self);
    pTurretInfo->manualTarget.setEnt(0);
    pTurretInfo->target.setEnt(0);
    pTurretInfo->detachSentient.setSentient(0);
    pTurretInfo->inuse = 0;
    self->pTurretInfo = 0;
}

bool __cdecl G_IsTurretUsable(gentity_s *self, gentity_s *owner)
{
    TurretInfo *pTurretInfo; // [esp+0h] [ebp-4h]

    pTurretInfo = self->pTurretInfo;
    if ( !pTurretInfo )
        return 0;
    if ( (pTurretInfo->flags & 0x1000) == 0 )
        return 0;
    if ( self->active || !self->pTurretInfo )
        return 0;
    if ( !turret_behind(self, owner) )
        return 0;
    if ( owner->client->ps.grenadeTimeLeft )
        return 0;
    return owner->client->ps.groundEntityNum != 1023;
}

bool __cdecl turret_behind(gentity_s *self, gentity_s *other)
{
    float dir[3]; // [esp+3Ch] [ebp-30h] BYREF
    float centerYaw; // [esp+48h] [ebp-24h]
    float minYaw; // [esp+4Ch] [ebp-20h]
    float angle; // [esp+50h] [ebp-1Ch]
    float forward[3]; // [esp+54h] [ebp-18h] BYREF
    TurretInfo *pTurretInfo; // [esp+60h] [ebp-Ch]
    float yawSpan; // [esp+64h] [ebp-8h]
    float dot; // [esp+68h] [ebp-4h]

    pTurretInfo = self->pTurretInfo;
    if ( !other->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 2748, 0, "%s", "other->client") )
    {
        __debugbreak();
    }
    minYaw = self->r.currentAngles[1] + pTurretInfo->arcmin[1];
    yawSpan = (float)(fabs(pTurretInfo->arcmax[1])
                                    + fabs(pTurretInfo->arcmin[1]))
                    * 0.5;
    centerYaw = AngleNormalize180(minYaw + yawSpan);
    YawVectors(centerYaw, forward, 0);
    Vec3Normalize(forward);
    dir[0] = self->r.currentOrigin[0] - other->r.currentOrigin[0];
    dir[1] = self->r.currentOrigin[1] - other->r.currentOrigin[1];
    dir[2] = 0.0f;
    Vec3Normalize(dir);
    dot = (float)((float)(forward[0] * dir[0]) + (float)(forward[1] * dir[1])) + (float)(forward[2] * dir[2]);
    angle = Q_acos(dot) * 57.295776;
    return angle <= yawSpan;
}

void __cdecl G_DeactivateTurret(gentity_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 2825, 0, "%s", "self") )
        __debugbreak();
    if ( self->active == 1 && Scr_IsSystemActive(1u, SCRIPTINSTANCE_SERVER) )
        Scr_Notify(self, scr_const.turret_deactivate, 0);
    self->active = 0;
}

void __cdecl turret_find_max_angles(gentity_s *pOwner, gentity_s *pTurret)
{
    double v2; // st7
    col_context_t context; // [esp+50h] [ebp-120h] BYREF
    float maxTurretArc; // [esp+78h] [ebp-F8h]
    bool drawLines; // [esp+7Fh] [ebp-F1h]
    float rightPoint[3]; // [esp+80h] [ebp-F0h] BYREF
    float minTurretArc; // [esp+8Ch] [ebp-E4h]
    float dropMins[3]; // [esp+90h] [ebp-E0h] BYREF
    float mins[3]; // [esp+9Ch] [ebp-D4h] BYREF
    float start[3]; // [esp+A8h] [ebp-C8h] BYREF
    float end[3]; // [esp+B4h] [ebp-BCh] BYREF
    float dropTraceWidth; // [esp+C0h] [ebp-B0h]
    TurretInfo *pTurretInfo; // [esp+C4h] [ebp-ACh]
    float leftPoint[3]; // [esp+C8h] [ebp-A8h] BYREF
    float maxs[3]; // [esp+D4h] [ebp-9Ch] BYREF
    trace_t tr; // [esp+E0h] [ebp-90h] BYREF
    float stepIncrement; // [esp+11Ch] [ebp-54h]
    float up[3]; // [esp+120h] [ebp-50h]
    float dropMaxs[3]; // [esp+12Ch] [ebp-44h] BYREF
    float heightTestDist; // [esp+138h] [ebp-38h]
    playerState_s *ps; // [esp+13Ch] [ebp-34h]
    float dropPoint[3]; // [esp+140h] [ebp-30h] BYREF
    float distToTurret; // [esp+14Ch] [ebp-24h]
    float stepCount; // [esp+150h] [ebp-20h]
    float color[4]; // [esp+154h] [ebp-1Ch] BYREF
    float toPlayer[3]; // [esp+164h] [ebp-Ch] BYREF

    stepCount = 3.0f;
    dropTraceWidth = 0.25f;
    heightTestDist = 4.0f;
    //col_context_t::col_context_t(&context);
    drawLines = cg_debugMounting->current.integer > 1;
    up[0] = 0.0f;
    up[1] = 0.0f;
    up[2] = 1.0f;
    pTurretInfo = pTurret->pTurretInfo;
    ps = &pOwner->client->ps;
    minTurretArc = pTurretInfo->arcmin[1];
    maxTurretArc = pTurretInfo->arcmax[1];
    dropMins[0] = -dropTraceWidth;
    dropMins[1] = -dropTraceWidth;
    dropMins[2] = 0.0f;
    dropMaxs[0] = dropTraceWidth;
    dropMaxs[1] = dropTraceWidth;
    dropMaxs[2] = heightTestDist * 2.0;
    mins[0] = pOwner->r.mins[0];
    mins[1] = pOwner->r.mins[1];
    mins[2] = pOwner->r.mins[2];
    mins[0] = mins[0] - 5.0;
    mins[1] = mins[1] - 5.0;
    maxs[0] = pOwner->r.maxs[0];
    maxs[1] = pOwner->r.maxs[1];
    maxs[2] = pOwner->r.maxs[2];
    maxs[0] = maxs[0] + 5.0;
    maxs[1] = maxs[1] + 5.0;
    toPlayer[0] = pOwner->s.lerp.pos.trBase[0] - pTurret->s.lerp.pos.trBase[0];
    toPlayer[1] = pOwner->s.lerp.pos.trBase[1] - pTurret->s.lerp.pos.trBase[1];
    toPlayer[2] = 0.0f;
    distToTurret = Vec3Normalize(toPlayer) + 24.0;
    toPlayer[0] = (float)(distToTurret * toPlayer[0]) + pTurret->s.lerp.pos.trBase[0];
    toPlayer[1] = (float)(distToTurret * toPlayer[1]) + pTurret->s.lerp.pos.trBase[1];
    toPlayer[2] = (float)(distToTurret * toPlayer[2]) + pTurret->s.lerp.pos.trBase[2];
    toPlayer[2] = pOwner->s.lerp.pos.trBase[2] + heightTestDist;
    leftPoint[0] = 0.0f;
    leftPoint[1] = 1.0f;
    leftPoint[2] = 0.0f;
    YawVectors((float)(pTurret->r.currentAngles[1] + 180.0) + minTurretArc, leftPoint, 0);
    leftPoint[0] = (float)(distToTurret * leftPoint[0]) + pTurret->s.lerp.pos.trBase[0];
    leftPoint[1] = (float)(distToTurret * leftPoint[1]) + pTurret->s.lerp.pos.trBase[1];
    leftPoint[2] = toPlayer[2];
    rightPoint[0] = 0.0f;
    rightPoint[1] = 1.0f;
    rightPoint[2] = 0.0f;
    YawVectors((float)(pTurret->r.currentAngles[1] + 180.0) + maxTurretArc, rightPoint, 0);
    rightPoint[0] = (float)(distToTurret * rightPoint[0]) + pTurret->s.lerp.pos.trBase[0];
    rightPoint[1] = (float)(distToTurret * rightPoint[1]) + pTurret->s.lerp.pos.trBase[1];
    rightPoint[2] = toPlayer[2];
    if ( drawLines )
    {
        start[0] = leftPoint[0];
        start[1] = leftPoint[1];
        start[2] = leftPoint[2];
        end[0] = (float)(20.0 * up[0]) + leftPoint[0];
        end[1] = (float)(20.0 * up[1]) + leftPoint[1];
        end[2] = (float)(20.0 * up[2]) + leftPoint[2];
        color[0] = 0.0f;
        color[1] = 0.0f;
        color[2] = 1.0f;
        color[3] = 1.0f;
        CG_DebugLine(start, end, color, 0, 1000);
    }
    G_TraceCapsule(&tr, toPlayer, mins, maxs, leftPoint, pOwner->s.number, pOwner->clipmask & 0xFDFF7FFF, &context);
    if ( tr.fraction < 1.0 )
        minTurretArc = minTurretArc * tr.fraction;
    stepIncrement = (float)(minTurretArc / stepCount) * -1.0;
    while ( minTurretArc != 0.0 )
    {
        leftPoint[0] = 0.0f;
        leftPoint[1] = 1.0f;
        leftPoint[2] = 0.0f;
        YawVectors((float)(pTurret->r.currentAngles[1] + 180.0) + minTurretArc, leftPoint, 0);
        leftPoint[0] = (float)(distToTurret * leftPoint[0]) + pTurret->s.lerp.pos.trBase[0];
        leftPoint[1] = (float)(distToTurret * leftPoint[1]) + pTurret->s.lerp.pos.trBase[1];
        leftPoint[2] = toPlayer[2];
        dropPoint[0] = leftPoint[0];
        dropPoint[1] = leftPoint[1];
        dropPoint[2] = toPlayer[2] - (float)(2.0 * heightTestDist);
        G_TraceCapsule(
            &tr,
            leftPoint,
            dropMins,
            dropMaxs,
            dropPoint,
            pOwner->s.number,
            pOwner->clipmask & 0xFDFF7FFF,
            &context);
        if ( tr.fraction < 1.0 )
        {
            if ( drawLines )
            {
                start[0] = dropPoint[0];
                start[1] = dropPoint[1];
                start[2] = dropPoint[2];
                end[0] = (float)(20.0 * up[0]) + dropPoint[0];
                end[1] = (float)(20.0 * up[1]) + dropPoint[1];
                end[2] = (float)(20.0 * up[2]) + dropPoint[2];
                color[0] = 0.0f;
                color[1] = 1.0f;
                color[2] = 0.0f;
                color[3] = 1.0f;
                CG_DebugBox(start, dropMins, dropMaxs, 0.0, color, 0, 1000);
            }
            break;
        }
        if ( drawLines )
        {
            start[0] = dropPoint[0];
            start[1] = dropPoint[1];
            start[2] = dropPoint[2];
            end[0] = (float)(20.0 * up[0]) + dropPoint[0];
            end[1] = (float)(20.0 * up[1]) + dropPoint[1];
            end[2] = (float)(20.0 * up[2]) + dropPoint[2];
            color[0] = 1.0f;
            color[1] = 0.0f;
            color[2] = 0.0f;
            color[3] = 1.0f;
            CG_DebugBox(start, dropMins, dropMaxs, 0.0, color, 0, 1000);
        }
        minTurretArc = minTurretArc + stepIncrement;
        if ( stepIncrement < 0.0 && minTurretArc < 0.0 || stepIncrement > 0.0 && minTurretArc > 0.0 )
        {
            minTurretArc = 0.0f;
            break;
        }
    }
    if ( drawLines )
    {
        start[0] = rightPoint[0];
        start[1] = rightPoint[1];
        start[2] = rightPoint[2];
        end[0] = (float)(20.0 * up[0]) + rightPoint[0];
        end[1] = (float)(20.0 * up[1]) + rightPoint[1];
        end[2] = (float)(20.0 * up[2]) + rightPoint[2];
        color[0] = 0.0f;
        color[1] = 0.0f;
        color[2] = 1.0f;
        color[3] = 1.0f;
        CG_DebugLine(start, end, color, 0, 1000);
    }
    G_TraceCapsule(&tr, toPlayer, mins, maxs, rightPoint, pOwner->s.number, pOwner->clipmask & 0xFDFF7FFF, &context);
    if ( tr.fraction < 1.0 )
        maxTurretArc = maxTurretArc * tr.fraction;
    stepIncrement = (float)(maxTurretArc / stepCount) * -1.0;
    while ( maxTurretArc != 0.0 )
    {
        rightPoint[0] = 0.0f;
        rightPoint[1] = 1.0f;
        rightPoint[2] = 0.0f;
        YawVectors((float)(pTurret->r.currentAngles[1] + 180.0) + maxTurretArc, rightPoint, 0);
        rightPoint[0] = (float)(distToTurret * rightPoint[0]) + pTurret->s.lerp.pos.trBase[0];
        rightPoint[1] = (float)(distToTurret * rightPoint[1]) + pTurret->s.lerp.pos.trBase[1];
        rightPoint[2] = toPlayer[2];
        dropPoint[0] = rightPoint[0];
        dropPoint[1] = rightPoint[1];
        dropPoint[2] = toPlayer[2] - (float)(2.0 * heightTestDist);
        G_TraceCapsule(
            &tr,
            rightPoint,
            dropMins,
            dropMaxs,
            dropPoint,
            pOwner->s.number,
            pOwner->clipmask & 0xFDFF7FFF,
            &context);
        if ( tr.fraction < 1.0 )
        {
            if ( drawLines )
            {
                start[0] = dropPoint[0];
                start[1] = dropPoint[1];
                start[2] = dropPoint[2];
                end[0] = (float)(20.0 * up[0]) + dropPoint[0];
                end[1] = (float)(20.0 * up[1]) + dropPoint[1];
                end[2] = (float)(20.0 * up[2]) + dropPoint[2];
                color[0] = 0.0f;
                color[1] = 1.0f;
                color[2] = 0.0f;
                color[3] = 1.0f;
                CG_DebugBox(start, dropMins, dropMaxs, 0.0, color, 0, 1000);
            }
            break;
        }
        if ( drawLines )
        {
            start[0] = dropPoint[0];
            start[1] = dropPoint[1];
            start[2] = dropPoint[2];
            end[0] = (float)(20.0 * up[0]) + dropPoint[0];
            end[1] = (float)(20.0 * up[1]) + dropPoint[1];
            end[2] = (float)(20.0 * up[2]) + dropPoint[2];
            color[0] = 1.0f;
            color[1] = 0.0f;
            color[2] = 0.0f;
            color[3] = 1.0f;
            CG_DebugBox(start, dropMins, dropMaxs, 0.0, color, 0, 1000);
        }
        maxTurretArc = maxTurretArc + stepIncrement;
        if ( stepIncrement < 0.0 && maxTurretArc < 0.0 || stepIncrement > 0.0 && maxTurretArc > 0.0 )
        {
            maxTurretArc = 0.0f;
            break;
        }
    }
    if ( minTurretArc != pTurretInfo->arcmin[1] || maxTurretArc != pTurretInfo->arcmax[1] )
    {
        v2 = AngleNormalize180(maxTurretArc - minTurretArc);
        ps->viewAngleClampRange[1] = v2 * 0.5;
        ps->viewAngleClampBase[1] = (float)((float)(maxTurretArc + minTurretArc) * 0.5) + ps->viewAngleClampBase[1];
    }
    if ( drawLines )
    {
        if ( minTurretArc != pTurretInfo->arcmin[1] )
        {
            leftPoint[0] = 0.0f;
            leftPoint[1] = 1.0f;
            leftPoint[2] = 0.0f;
            YawVectors((float)(pTurret->r.currentAngles[1] + 180.0) + minTurretArc, leftPoint, 0);
            leftPoint[0] = (float)(distToTurret * leftPoint[0]) + pTurret->s.lerp.pos.trBase[0];
            leftPoint[1] = (float)(distToTurret * leftPoint[1]) + pTurret->s.lerp.pos.trBase[1];
            leftPoint[2] = toPlayer[2];
            start[0] = leftPoint[0];
            start[1] = leftPoint[1];
            start[2] = toPlayer[2];
            end[0] = (float)(20.0 * up[0]) + leftPoint[0];
            end[1] = (float)(20.0 * up[1]) + leftPoint[1];
            end[2] = (float)(20.0 * up[2]) + toPlayer[2];
            color[0] = 1.0f;
            color[1] = 1.0f;
            color[2] = 0.0f;
            color[3] = 1.0f;
            CG_DebugLine(start, end, color, 0, 1000);
        }
        if ( maxTurretArc != pTurretInfo->arcmax[1] )
        {
            rightPoint[0] = 0.0f;
            rightPoint[1] = 1.0f;
            rightPoint[2] = 0.0f;
            YawVectors((float)(pTurret->r.currentAngles[1] + 180.0) + maxTurretArc, rightPoint, 0);
            rightPoint[0] = (float)(distToTurret * rightPoint[0]) + pTurret->s.lerp.pos.trBase[0];
            rightPoint[1] = (float)(distToTurret * rightPoint[1]) + pTurret->s.lerp.pos.trBase[1];
            rightPoint[2] = toPlayer[2];
            start[0] = rightPoint[0];
            start[1] = rightPoint[1];
            start[2] = toPlayer[2];
            end[0] = (float)(20.0 * up[0]) + rightPoint[0];
            end[1] = (float)(20.0 * up[1]) + rightPoint[1];
            end[2] = (float)(20.0 * up[2]) + toPlayer[2];
            color[0] = 1.0f;
            color[1] = 1.0f;
            color[2] = 0.0f;
            color[3] = 1.0f;
            CG_DebugLine(start, end, color, 0, 1000);
        }
    }
}

void __cdecl turret_use(gentity_s *self, gentity_s *owner, gentity_s *__formal)
{
    int v2; // edx
    double v3; // st7
    double v4; // st7
    int v5; // [esp+4h] [ebp-68h]
    int v6; // [esp+8h] [ebp-64h]
    float v7; // [esp+Ch] [ebp-60h]
    int period; // [esp+18h] [ebp-54h]
    float v9; // [esp+28h] [ebp-44h]
    int actorNum; // [esp+34h] [ebp-38h]
    float *userOrigin; // [esp+44h] [ebp-28h]
    float adjPos_4; // [esp+50h] [ebp-1Ch]
    float adjPos_8; // [esp+54h] [ebp-18h]
    float normal[3]; // [esp+58h] [ebp-14h] BYREF
    TurretInfo *pTurretInfo; // [esp+64h] [ebp-8h]
    playerState_s *ps; // [esp+68h] [ebp-4h]

    if (!self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 3129, 0, "%s", "self"))
        __debugbreak();
    pTurretInfo = self->pTurretInfo;
    if (!pTurretInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 3132, 0, "%s", "pTurretInfo"))
    {
        __debugbreak();
    }
    if (!owner && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 3134, 0, "%s", "owner"))
        __debugbreak();
    if (!owner->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 3135, 0, "%s", "owner->client"))
    {
        __debugbreak();
    }
    if (owner->actor
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 3136, 0, "%s", "!owner->actor"))
    {
        __debugbreak();
    }
    if (owner->s.number >= level.maxclients
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp",
            3137,
            0,
            "%s",
            "owner->s.number < level.maxclients"))
    {
        __debugbreak();
    }
    if (!self->active)
    {
        ps = &owner->client->ps;
        owner->active = 1;
        self->active = 1;
        //EntHandle::setEnt(&self->r.ownerNum, owner);
        self->r.ownerNum.setEnt(owner);
        ps->viewlocked = PLAYERVIEWLOCK_FULL;
        AssignToSmallerType<short>(&ps->viewlocked_entNum, self->s.number);
        pTurretInfo->flags |= 0x800u;
        pTurretInfo->arcmin[1] = pTurretInfo->initialYawmin;
        pTurretInfo->arcmax[1] = pTurretInfo->initialYawmax;
        AngleVectors(owner->r.currentAngles, normal, 0, 0);
        adjPos_4 = owner->r.currentOrigin[1] - normal[1];
        adjPos_8 = owner->r.currentOrigin[2] - normal[2];
        userOrigin = pTurretInfo->userOrigin;
        pTurretInfo->userOrigin[0] = owner->r.currentOrigin[0] - normal[0];
        userOrigin[1] = adjPos_4;
        userOrigin[2] = adjPos_8;
        if ((ps->pm_flags & 1) != 0)
            pTurretInfo->prevStance = 2;
        else
            pTurretInfo->prevStance = (ps->pm_flags & 2) != 0;
        if (pTurretInfo->stance == 2)
        {
            ps->eFlags |= 0x100u;
            ps->eFlags &= ~0x200u;
        }
        else
        {
            if (pTurretInfo->stance == 1)
            {
                ps->eFlags |= 0x200u;
                v2 = ps->eFlags & 0xFFFFFEFF;
            }
            else
            {
                v2 = ps->eFlags | 0x300;
            }
            ps->eFlags = v2;
        }
        self->s.lerp.u.turret.gunAngles[0] = AngleNormalize180(ps->viewangles[0] - self->r.currentAngles[0]);
        v9 = self->s.lerp.u.turret.gunAngles[0];
        if ((float)(v9 - pTurretInfo->arcmax[0]) < 0.0)
            actorNum = self->s.lerp.u.actor.actorNum;
        else
            actorNum = LODWORD(pTurretInfo->arcmax[0]);
        if ((float)(pTurretInfo->arcmin[0] - v9) < 0.0)
            v6 = actorNum;
        else
            v6 = LODWORD(pTurretInfo->arcmin[0]);
        self->s.lerp.u.actor.actorNum = v6;
        self->s.lerp.u.turret.gunAngles[1] = AngleNormalize180(ps->viewangles[1] - self->r.currentAngles[1]);
        v7 = self->s.lerp.u.turret.gunAngles[1];
        if ((float)(v7 - pTurretInfo->arcmax[1]) < 0.0)
            period = self->s.lerp.u.loopFx.period;
        else
            period = LODWORD(pTurretInfo->arcmax[1]);
        if ((float)(pTurretInfo->arcmin[1] - v7) < 0.0)
            v5 = period;
        else
            v5 = LODWORD(pTurretInfo->arcmin[1]);
        self->s.lerp.u.loopFx.period = v5;
        self->s.lerp.u.actor.team = 0;// *(_DWORD *)&FLOAT_0_0;
        ps->viewAngleClampRange[0] = (float)(pTurretInfo->arcmax[0] - pTurretInfo->arcmin[0]) * 0.5;
        ps->viewAngleClampBase[0] = self->r.currentAngles[0] + pTurretInfo->arcmax[0];
        v3 = AngleNormalize360(ps->viewAngleClampBase[0] - ps->viewAngleClampRange[0]);
        ps->viewAngleClampBase[0] = v3;
        ps->viewAngleClampRange[1] = (float)(pTurretInfo->arcmax[1] - pTurretInfo->arcmin[1]) * 0.5;
        ps->viewAngleClampBase[1] = self->r.currentAngles[1] + pTurretInfo->arcmax[1];
        v4 = AngleNormalize360(ps->viewAngleClampBase[1] - ps->viewAngleClampRange[1]);
        ps->viewAngleClampBase[1] = v4;
        if (self->classname == scr_const.deployed_turret)
            turret_find_max_angles(owner, self);
        Scr_Notify(self, scr_const.turretownerchange, 0);
    }
}

void __cdecl G_SpawnTurret(gentity_s *self, const char *weaponinfoname, SpawnVar *spawnVar)
{
    int WeaponIndexForName; // eax
    int v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    double v7; // xmm0_8
    long double v8; // [esp+0h] [ebp-40h]
    float v9; // [esp+0h] [ebp-40h]
    float forwardAngleDot; // [esp+18h] [ebp-28h]
    float v11; // [esp+1Ch] [ebp-24h]
    unsigned __int8 boneIndex; // [esp+23h] [ebp-1Dh] BYREF
    DObj *obj; // [esp+24h] [ebp-1Ch]
    float suppressTime; // [esp+28h] [ebp-18h] BYREF
    float maxRange; // [esp+2Ch] [ebp-14h] BYREF
    float convergenceTime; // [esp+30h] [ebp-10h] BYREF
    TurretInfo *turretInfo; // [esp+34h] [ebp-Ch]
    int i; // [esp+38h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+3Ch] [ebp-4h]

    turretInfo = 0;
    for ( i = 0; i < 32; ++i )
    {
        turretInfo = &turretInfoStore[i];
        if ( !turretInfo->inuse )
            break;
    }
    if ( i == 32 )
        Com_Error(ERR_DROP, "G_SpawnTurret: max number of turrets (%d) exceeded", 32);
    memset(turretInfo, 0, sizeof(TurretInfo));
    self->pTurretInfo = turretInfo;
    turretInfo->inuse = 1;
    turretInfo->scanningPitch = 0.0f;
    WeaponIndexForName = G_GetWeaponIndexForName((char*)weaponinfoname);
    AssignToSmallerType<unsigned short>(&self->s.weapon, WeaponIndexForName);
    self->s.weaponModel = 0;
    if ( !self->s.weapon )
    {
        v4 = BG_GetWeaponIndexForName(weaponinfoname, G_RegisterWeapon);
        AssignToSmallerType<unsigned short>(&self->s.weapon, v4);
        if ( !self->s.weapon )
            Com_Error(ERR_DROP, "bad weaponinfo %s specified for turret", weaponinfoname);
    }
    weapDef = BG_GetWeaponDef(self->s.weapon);
    if ( weapDef->weapClass != WEAPCLASS_TURRET )
    {
        v5 = va("Could not load turret '%s'", weaponinfoname);
        Scr_Error(v5, 0);
    }
    if ( !level.initializing && !IsItemRegistered(self->s.weapon) )
    {
        v6 = va("turret '%s' not precached", weaponinfoname);
        Scr_Error(v6, 0);
    }
    turretInfo->fireTime = 0;
    turretInfo->stance = weapDef->stance;
    turretInfo->prevStance = -1;
    turretInfo->fireSndDelay = 0;
    turretInfo->heatVal = 0.0f;
    turretInfo->overheating = 0;

    if ( weapDef->fireLoopSound )
        turretInfo->fireSnd = SND_FindAliasId(weapDef->fireLoopSound);
    else
        turretInfo->fireSnd = 0;

    if ( weapDef->fireLoopSoundPlayer )
        turretInfo->fireSndPlayer = SND_FindAliasId(weapDef->fireLoopSoundPlayer);
    else
        turretInfo->fireSndPlayer = 0;

    if ( weapDef->fireStopSound )
        turretInfo->stopSnd = SND_FindAliasId(weapDef->fireStopSound);
    else
        turretInfo->stopSnd = 0;

    if ( weapDef->fireStopSoundPlayer )
        turretInfo->stopSndPlayer = SND_FindAliasId(weapDef->fireStopSoundPlayer);
    else
        turretInfo->stopSndPlayer = 0;

    if ( !spawnVar || !G_SpawnFloat(spawnVar, "rightarc", "", &turretInfo->arcmin[1]) )
        turretInfo->arcmin[1] = weapDef->rightArc;
    turretInfo->arcmin[1] = turretInfo->arcmin[1] * -1.0;
    if ( turretInfo->arcmin[1] > 0.0 )
        turretInfo->arcmin[1] = 0.0f;
    turretInfo->initialYawmin = turretInfo->arcmin[1];
    if ( !spawnVar || !G_SpawnFloat(spawnVar, "leftarc", "", &turretInfo->arcmax[1]) )
        turretInfo->arcmax[1] = weapDef->leftArc;
    if ( turretInfo->arcmax[1] < 0.0 )
        turretInfo->arcmax[1] = 0.0f;
    turretInfo->initialYawmax = turretInfo->arcmax[1];
    turretInfo->forwardAngleDot = turretInfo->initialYawmax - turretInfo->initialYawmin;
    forwardAngleDot = turretInfo->forwardAngleDot;
    if ( (float)(forwardAngleDot - 180.0) < 0.0 )
        v11 = turretInfo->forwardAngleDot;
    else
        v11 = 180.0f;
    if ( (float)(90.0 - forwardAngleDot) < 0.0 )
        *((float *)&v8 + 1) = v11;
    else
        *((float *)&v8 + 1) = 90.0f;
    turretInfo->forwardAngleDot = *((float *)&v8 + 1);
    v7 = (float)(turretInfo->forwardAngleDot * 0.017453292);
    //__libm_sse2_cos(v8);
    //*(float *)&v7 = v7;
    v7 = cos(v7);
    turretInfo->forwardAngleDot = *(float *)&v7;
    if ( !spawnVar || !G_SpawnFloat(spawnVar, "toparc", "", turretInfo->arcmin) )
        turretInfo->arcmin[0] = weapDef->topArc;
    turretInfo->arcmin[0] = turretInfo->arcmin[0] * -1.0;
    if ( turretInfo->arcmin[0] > 0.0 )
        turretInfo->arcmin[0] = 0.0f;
    if ( !spawnVar || !G_SpawnFloat(spawnVar, "bottomarc", "", turretInfo->arcmax) )
        turretInfo->arcmax[0] = weapDef->bottomArc;
    if ( turretInfo->arcmax[0] < 0.0 )
        turretInfo->arcmax[0] = 0.0f;
    if ( (!spawnVar || !G_SpawnFloat(spawnVar, "yawconvergencetime", "", &convergenceTime))
        && (!spawnVar || !G_SpawnFloat(spawnVar, "convergencetime", "", &convergenceTime)) )
    {
        convergenceTime = weapDef->yawConvergenceTime;
    }
    if ( convergenceTime < 0.0 )
        convergenceTime = 0.0f;
    turretInfo->convergenceTime[1] = (int)(float)((float)(convergenceTime * 1000.0) + 0.5);
    if ( !spawnVar || !G_SpawnFloat(spawnVar, "pitchconvergencetime", "", &convergenceTime) )
        convergenceTime = weapDef->pitchConvergenceTime;
    if ( convergenceTime < 0.0 )
        convergenceTime = 0.0f;
    turretInfo->convergenceTime[0] = (int)(float)((float)(convergenceTime * 1000.0) + 0.5);
    if ( !spawnVar || !G_SpawnFloat(spawnVar, "suppressionTime", "", &suppressTime) )
        suppressTime = weapDef->suppressTime;
    if ( suppressTime < 0.0 )
        suppressTime = 0.0f;
    turretInfo->suppressTime = (int)(float)((float)(suppressTime * 1000.0) + 0.5);
    if ( !spawnVar || !G_SpawnFloat(spawnVar, "maxrange", "", &maxRange) )
        maxRange = weapDef->maxRange;
    if ( maxRange <= 0.0 )
        v9 = FLT_MAX;
    else
        v9 = maxRange * maxRange;
    turretInfo->maxRangeSquared = v9;
    turretInfo->dropPitch = -90.0f;
    if ( !self->health )
        self->health = 100;
    if ( !spawnVar || !G_SpawnFloat(spawnVar, "accuracy", "1", &turretInfo->accuracy) )
        turretInfo->accuracy = weapDef->accuracy;
    if ( turretInfo->accuracy >= 0.0 )
    {
        if ( turretInfo->accuracy > 1.0 )
            turretInfo->accuracy = 1.0f;
    }
    else
    {
        turretInfo->accuracy = 0.0f;
    }
    if ( !spawnVar || !G_SpawnFloat(spawnVar, "aiSpread", "1", &turretInfo->aiSpread) )
        turretInfo->aiSpread = weapDef->aiSpread;
    if ( turretInfo->aiSpread < 0.0 )
        turretInfo->aiSpread = 0.0f;
    if ( !spawnVar || !G_SpawnFloat(spawnVar, "playerSpread", "1", &turretInfo->playerSpread) )
        turretInfo->playerSpread = weapDef->playerSpread;
    if ( turretInfo->playerSpread < 0.0 )
        turretInfo->playerSpread = 0.0f;
    turretInfo->flags = 4099;
    turretInfo->state = 0;
    turretInfo->prevSentTarget = -1;
    self->clipmask = 1;
    self->r.contents = 2097156;
    self->r.svFlags = 0;
    self->s.eType = ET_MG42;
    self->flags |= 0x1000u;
    G_DObjUpdate(self);
    obj = Com_GetServerDObj(self->s.number);
    if ( obj )
    {
        boneIndex = -2;
        if ( DObjGetBoneIndex(obj, scr_const.tag_flash_2, &boneIndex, -1) )
            turretInfo->flags |= 0x8000u;
    }
    self->r.mins[0] = -32.0f;
    self->r.mins[1] = -32.0f;
    self->r.mins[2] = 0.0f;
    self->r.maxs[0] = 32.0f;
    self->r.maxs[1] = 32.0f;
    self->r.maxs[2] = 56.0f;
    G_SetOrigin(self, self->r.currentOrigin);
    G_SetAngle(self, self->r.currentAngles);
    self->s.lerp.u.actor.actorNum = 0;
    self->s.lerp.u.loopFx.period = 0;
    self->s.lerp.u.actor.team = 0;
    self->handler = 17;
    self->nextthink = level.time + 50;
    self->s.lerp.apos.trType = 4;
    self->takedamage = 0;
    iassert(!self->r.ownerNum.isDefined());
    SV_LinkEntity(self);
}

void __cdecl SP_turret(gentity_s *self, SpawnVar *spawnVar)
{
    const char *weaponinfoname; // [esp+0h] [ebp-4h] BYREF

    if ( !G_SpawnString(spawnVar, "weaponinfo", "", &weaponinfoname) )
        Com_Error(ERR_DROP, "no weaponinfo specified for turret");
    G_SpawnTurret(self, weaponinfoname, spawnVar);
}

void __cdecl Turret_SetTurretCarried(gentity_s *self, int carried)
{
    int v2; // ecx

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 3447, 0, "%s", "self") )
        __debugbreak();
    if ( self->s.otherEntityNum == 1023
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp",
                    3448,
                    0,
                    "%s",
                    "self->s.otherEntityNum != ENTITYNUM_NONE") )
    {
        __debugbreak();
    }
    if ( carried )
    {
        v2 = self->s.lerp.u.turret.flags | 8;
    }
    else
    {
        self->s.lerp.u.turret.flags &= ~8u;
        v2 = self->s.lerp.u.turret.flags | 0x20;
    }
    self->s.lerp.u.turret.flags = v2;
}

void __cdecl Turret_SetTurretOwner(gentity_s *self, gentity_s *owner)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp", 3463, 0, "%s", "self") )
        __debugbreak();
    if ( !self->pTurretInfo
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\turret.cpp",
                    3464,
                    0,
                    "%s",
                    "self->pTurretInfo != NULL") )
    {
        __debugbreak();
    }
    if ( owner && self->s.otherEntityNum != 1023 && owner->s.number != self->s.otherEntityNum )
        Com_PrintWarning(
            17,
            "WARNING: turret owner ent has been set while there was already another owner ( %i => %i )\n",
            self->s.otherEntityNum,
            owner->s.number);
    if ( owner )
        self->s.otherEntityNum = owner->s.number;
    else
        self->s.otherEntityNum = 1023;
}

