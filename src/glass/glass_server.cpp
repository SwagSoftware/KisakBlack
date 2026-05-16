#include "glass_server.h"
#include <game_mp/g_main_mp.h>
#include <game_mp/g_utils_mp.h>
#include "glass_load_obj.h"
#include <server_mp/sv_main_mp.h>
#include <universal/com_math_anglevectors.h>
#include <qcommon/cm_tracebox.h>
#include <game_mp/g_spawn_mp.h>
#include <clientscript/scr_const.h>
#include <server/sv_game.h>
#include <physics/phys_main.h>
#include <flame/flame_system.h>

GlassesServer svGlasses;

void __thiscall GlassServer::SetState(GlassState::State st, float *pos, float *dir)
{
    if ( st != (this->state.val.i & 0xF) )
    {
        this->stateChangeTime = level.time;
        if ( st == GlassState::State::CRACKED )
        {
            this->crackedDamageRate = G_flrand(
                                                                    svGlasses.crackedDamageRateRange->current.value,
                                                                    svGlasses.crackedDamageRateRange->current.vector[1]);
        }
        else if ( st == GlassState::State::SHATTERED )
        {
            Scr_GlassSmash(pos, dir);
            this->hitPos[0] = *pos;
            this->hitPos[1] = pos[1];
            this->hitPos[2] = pos[2];
            this->hitDir[0] = *dir;
            this->hitDir[1] = dir[1];
            this->hitDir[2] = dir[2];
        }
        this->state.val.i = st & 0xF | this->state.val.i & 0xFFFFFFF0;
    }
}

cmd_function_s ShatterAllCmd_VAR;
cmd_function_s ResetAllCmd_VAR;

void __thiscall GlassesServer::Init()
{
    unsigned int i; // [esp+20h] [ebp-Ch]
    Glasses *glss; // [esp+24h] [ebp-8h]

    if ( !this->inited )
    {
        Cmd_AddCommandInternal("shatterAllGlass", GlassesServer::ShatterAllCmd, &ShatterAllCmd_VAR);
        Cmd_AddCommandInternal("resetGlass", GlassesServer::ResetAllCmd, &ResetAllCmd_VAR);
        this->inited = 1;
        this->damageMultiplier = _Dvar_RegisterFloat(
                                                             "glassDamageMultiplier",
                                                             1.0,
                                                             0.1,
                                                             10.0,
                                                             0,
                                                             "Multiplier the damage applied to the glass (server).");
        this->crackedDamageRateRange = _Dvar_RegisterVec2(
                                                                         "glassCrackedDamageRateRange",
                                                                         (1.0),
                                                                         (10.0),
                                                                         0.0,
                                                                         10000.0,
                                                                         0,
                                                                         "Range of the rate a cracked glass looses health points");
    }
    if ( !this->numGlasses )
    {
        glss = GetGlasses();
        if ( glss )
        {
            if ( glss->numGlasses >= 0x3E8
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_server.cpp",
                            136,
                            0,
                            "%s",
                            "glss->numGlasses < MAX_GLASSES") )
            {
                __debugbreak();
            }
            this->numGlasses = glss->numGlasses;
            for ( i = 0; i < this->numGlasses; ++i )
            {
                this->glasses[i].glass = &glss->glasses[i];
                this->glasses[i].state.val.i = (16 * (i & 0xFFF)) | this->glasses[i].state.val.i & 0xFFFF000F;
                //GlassServer::SetState(&this->glasses[i], PRISTINE, 0, 0);
                this->glasses[i].SetState(GlassState::State::PRISTINE, 0, 0);
                this->glasses[i].health = (float)this->glasses[i].glass->glassDef->maxHealth;
                this->glasses[i].stateChangeTime = 0;
            }
        }
        else
        {
            this->numGlasses = 0;
        }
    }
    this->nextShatterTime = 0;
}

void __thiscall GlassesServer::Shutdown()
{
    this->numGlasses = 0;
}

void __thiscall GlassesServer::Update()
{
    float v3; // [esp+1Ch] [ebp-54h]
    float v4; // [esp+20h] [ebp-50h]
    float angles[3]; // [esp+28h] [ebp-48h] BYREF
    float axis[3][3]; // [esp+34h] [ebp-3Ch] BYREF
    unsigned int glassId; // [esp+58h] [ebp-18h]
    int toShatter; // [esp+5Ch] [ebp-14h]
    unsigned int j; // [esp+60h] [ebp-10h]
    int nPristine; // [esp+64h] [ebp-Ch]
    unsigned int i; // [esp+68h] [ebp-8h]

    if ( this->numGlasses )
    {
        PROF_SCOPED("GlassesServer.Update");

        for ( i = 0; i < this->numGlasses; ++i )
        {
            if ( (this->glasses[i].state.val.i & 0xF) == 1 )
            {
                this->glasses[i].health = this->glasses[i].health
                                                                - (float)((float)((float)(this->glasses[i].crackedDamageRate
                                                                                                                * this->damageMultiplier->current.value)
                                                                                                * (float)level.frametime)
                                                                                * 0.001);
                if (this->glasses[i].health < 0.0)
                {
                    //GlassServer::SetState(&this->glasses[i], GlassState::State::SHATTERED, (float *)vec3_origin, (float *)vec3_origin);
                    this->glasses[i].SetState(GlassState::State::SHATTERED, (float *)vec3_origin, (float *)vec3_origin);
                }
            }
        }
        while ( this->nextShatterTime > 0 && svs.time > this->nextShatterTime )
        {
            nPristine = 0;
            for ( j = 0; j < this->numGlasses; ++j )
            {
                if ( (this->glasses[j].state.val.i & 0xF) != 2 )
                    ++nPristine;
            }
            if ( nPristine <= 0 )
            {
                this->nextShatterTime = 0;
            }
            else
            {
                toShatter = G_irand(0, nPristine);
                for ( glassId = 0; glassId < this->numGlasses; ++glassId )
                {
                    if ( (this->glasses[glassId].state.val.i & 0xF) != 2 && !toShatter-- )
                    {
                        v3 = G_flrand(0.0, 360.0);
                        v4 = G_flrand(-90.0, 90.0);
                        angles[0] = v3;
                        angles[1] = v4;
                        angles[2] = 0.0f;
                        AnglesToAxis(angles, axis);
                        if ( G_flrand(0.0, 1.0) <= 0.5 )
                            GlassSv_Damage(glassId, 1, 2, this->glasses[glassId].glass->origin, axis[0]);
                        else
                            GlassSv_Damage(glassId, 1, 3, this->glasses[glassId].glass->origin, axis[0]);
                        break;
                    }
                }
                this->nextShatterTime += this->shatterIntervalsMS;
            }
        }
    }
}

void __cdecl GlassesServer::ShatterAllCmd()
{
    //GlassesServer::ShatterAll(&svGlasses);
    svGlasses.ShatterAll();
}

void __thiscall GlassesServer::ShatterAll()
{
    const char *v1; // eax
    int v2; // [esp+8h] [ebp-60h]
    float v4; // [esp+10h] [ebp-58h]
    float v5; // [esp+14h] [ebp-54h]
    int v6; // [esp+18h] [ebp-50h]
    float angles[3]; // [esp+2Ch] [ebp-3Ch] BYREF
    float axis[3][3]; // [esp+38h] [ebp-30h] BYREF
    unsigned int i; // [esp+5Ch] [ebp-Ch]
    int nShatters; // [esp+60h] [ebp-8h]
    int MAX_SHATTERS; // [esp+64h] [ebp-4h]

    if ( Cmd_Argc() <= 1 )
    {
        MAX_SHATTERS = 100;
        nShatters = 0;
        for ( i = 0; i < this->numGlasses; ++i )
        {
            if ( (this->glasses[i].state.val.i & 0xF) != 2 )
            {
                v4 = G_flrand(0.0, 360.0);
                v5 = G_flrand(-90.0, 90.0);
                angles[0] = v4;
                angles[1] = v5;
                angles[2] = 0.0f;
                AnglesToAxis(angles, axis);
                GlassSv_Damage(i, 1, 3, g_entities[i].s.lerp.pos.trBase, axis[0]);
                if ( ++nShatters == 100 )
                    break;
            }
        }
    }
    else
    {
        v1 = Cmd_Argv(1);
        v6 = atoi(v1);
        if ( v6 < 10 )
            v2 = 10;
        else
            v2 = v6;
        this->shatterIntervalsMS = v2;
        this->nextShatterTime = svs.time;
    }
}

void __cdecl GlassesServer::ResetAllCmd()
{
    //GlassesServer::ResetAll(&svGlasses);
    svGlasses.ResetAll();
}

void __thiscall GlassesServer::ResetAll()
{
    unsigned int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < this->numGlasses; ++i )
    {
        if ( (this->glasses[i].state.val.i & 0xF) != 0 )
        {
            //GlassServer::SetState(&this->glasses[i], PRISTINE, 0, 0);
            this->glasses[i].SetState(GlassState::State::PRISTINE, 0, 0);
            this->glasses[i].health = (float)this->glasses[i].glass->glassDef->maxHealth;
        }
    }
}

void __cdecl GlassSv_Init()
{
    svGlasses.Init();
}

void __cdecl GlassSv_Shutdown()
{
    svGlasses.Shutdown();
}

void __cdecl GlassSv_Update()
{
    svGlasses.Update();
}

unsigned int __cdecl GlassSv_AreaGlasses(
                const float *mins,
                const float *maxs,
                const Glass **glasses,
                unsigned int maxGlasses)
{
    const GlassServer *glass; // [esp+14h] [ebp-Ch]
    unsigned int i; // [esp+18h] [ebp-8h]
    unsigned int num; // [esp+1Ch] [ebp-4h]

    num = 0;
    if ( svGlasses.numGlasses )
    {
        for ( i = 0; i < svGlasses.numGlasses && num < maxGlasses; ++i )
        {
            glass = &svGlasses.glasses[i];
            if ( (glass->state.val.i & 0xF) != 2
                && *maxs >= glass->glass->absmin[0]
                && maxs[1] >= glass->glass->absmin[1]
                && maxs[2] >= glass->glass->absmin[2]
                && glass->glass->absmax[0] >= *mins
                && glass->glass->absmax[1] >= mins[1]
                && glass->glass->absmax[2] >= mins[2] )
            {
                glasses[num++] = glass->glass;
            }
        }
    }
    return num;
}

void __cdecl GlassSv_ClipMoveTrace(const moveclip_t *clip, trace_t *results)
{
    float v2; // [esp+0h] [ebp-29Ch]
    float v3; // [esp+4h] [ebp-298h]
    unsigned int v4; // [esp+8h] [ebp-294h]
    float v5; // [esp+Ch] [ebp-290h]
    float v6; // [esp+10h] [ebp-28Ch]
    unsigned int v7; // [esp+14h] [ebp-288h]
    unsigned __int16 index; // [esp+22h] [ebp-27Ah]
    const Glass *glass; // [esp+68h] [ebp-234h]
    float oldFraction; // [esp+6Ch] [ebp-230h]
    unsigned int j; // [esp+70h] [ebp-22Ch]
    int i; // [esp+74h] [ebp-228h]
    float mins[3]; // [esp+78h] [ebp-224h] BYREF
    const Glass *glss[128]; // [esp+84h] [ebp-218h] BYREF
    float maxs[3]; // [esp+288h] [ebp-14h] BYREF
    unsigned int num; // [esp+298h] [ebp-4h]

    if ( (clip->contentmask & 0x10) != 0 && svGlasses.numGlasses )
    {
        PROF_SCOPED("GlassSv_ClipMoveTrace");

        for ( i = 0; i < 3; ++i )
        {
            if ( (float)(clip->extents.end.vec.v[i] - clip->extents.start.vec.v[i]) < 0.0 )
                v7 = clip->extents.end.vec.u[i];
            else
                v7 = clip->extents.start.vec.u[i];
            LODWORD(mins[i]) = v7;
            if ( (float)((float)(clip->extents.start.vec.v[i] + clip->mins[i]) - mins[i]) < 0.0 )
                v6 = clip->extents.start.vec.v[i] + clip->mins[i];
            else
                v6 = mins[i];
            mins[i] = v6;
            if ( (float)((float)(clip->extents.end.vec.v[i] + clip->mins[i]) - mins[i]) < 0.0 )
                v5 = clip->extents.end.vec.v[i] + clip->mins[i];
            else
                v5 = mins[i];
            mins[i] = v5;
            if ( (float)(clip->extents.start.vec.v[i] - clip->extents.end.vec.v[i]) < 0.0 )
                v4 = clip->extents.end.vec.u[i];
            else
                v4 = clip->extents.start.vec.u[i];
            LODWORD(maxs[i]) = v4;
            if ( (float)(maxs[i] - (float)(clip->extents.start.vec.v[i] + clip->maxs[i])) < 0.0 )
                v3 = clip->extents.start.vec.v[i] + clip->maxs[i];
            else
                v3 = maxs[i];
            maxs[i] = v3;
            if ( (float)(maxs[i] - (float)(clip->extents.end.vec.v[i] + clip->maxs[i])) < 0.0 )
                v2 = clip->extents.end.vec.v[i] + clip->maxs[i];
            else
                v2 = maxs[i];
            maxs[i] = v2;
        }
        num = GlassSv_AreaGlasses(mins, maxs, glss, 0x80u);
        for ( j = 0; j < num; ++j )
        {
            glass = glss[j];
            oldFraction = results->fraction;
            CM_TransformedBoxTrace(
                results,
                clip->extents.start.vec.v,
                clip->extents.end.vec.v,
                clip->mins,
                clip->maxs,
                glass->brushModel,
                clip->contentmask,
                glass->origin,
                glass->angles);
            if ( results->fraction < oldFraction )
            {
                index = glass->index;
                if ( !results
                    && !Assert_MyHandler(
                                "c:\\projects_pc\\cod\\codsrc\\src\\physics\\../qcommon/cm_public.h",
                                175,
                                0,
                                "%s",
                                "trace") )
                {
                    __debugbreak();
                }
                results->hitType = TRACE_HITTYPE_GLASS;
                results->hitId = index;
            }
        }
    }
}

void __cdecl GlassSv_PointTrace(const pointtrace_t *clip, trace_t *results)
{
    float oldFraction; // [esp+14h] [ebp-10h]
    const GlassServer *glass; // [esp+18h] [ebp-Ch]
    unsigned int i; // [esp+1Ch] [ebp-8h]

    if ( (clip->contentmask & 0x10) != 0 && svGlasses.numGlasses )
    {
        PROF_SCOPED("GlassSv_PointTrace");

        for ( i = 0; i < svGlasses.numGlasses; ++i )
        {
            glass = &svGlasses.glasses[i];
            if ( (glass->state.val.i & 0xF) != 2 )
            {
                if ( intersect_extents_aabb(&clip->extents, glass->glass->absmin, glass->glass->absmax, results->fraction) )
                {
                    oldFraction = results->fraction;
                    CM_TransformedBoxTrace(
                        results,
                        clip->extents.start.vec.v,
                        clip->extents.end.vec.v,
                        vec3_origin,
                        vec3_origin,
                        glass->glass->brushModel,
                        clip->contentmask,
                        glass->glass->origin,
                        glass->glass->angles);
                    if ( results->fraction < oldFraction )
                    {
                        results->modelIndex = 0;
                        results->partName = 0;
                        results->boneIndex = 0;
                        results->partGroup = 0;
                        if ( !results
                            && !Assert_MyHandler(
                                        "c:\\projects_pc\\cod\\codsrc\\src\\physics\\../qcommon/cm_public.h",
                                        175,
                                        0,
                                        "%s",
                                        "trace") )
                        {
                            __debugbreak();
                        }
                        results->hitType = TRACE_HITTYPE_GLASS;
                        results->hitId = i;
                        results->cflags = 16;
                    }
                }
            }
        }
    }
}

void __cdecl GlassSv_Damage(unsigned int glassId, int damage, int mod, float *pos, float *dir)
{
    char v6; // [esp+4h] [ebp-20h]
    GlassServer *glass; // [esp+20h] [ebp-4h]

    PROF_SCOPED("GlassSv_Damage");

    if ( glassId >= svGlasses.numGlasses
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_server.cpp",
                    646,
                    0,
                    "%s",
                    "glassId < svGlasses.NumGlasses()") )
    {
        __debugbreak();
    }
    glass = &svGlasses.glasses[glassId];
    if ( glass->health <= 0.0
        || (float)(5.0 * svGlasses.damageMultiplier->current.value) > (float)damage
        && (mod == 3 || mod == 5 || mod == 7 || mod == 8 || mod == 17 || mod == 16 ? (v6 = 1) : (v6 = 0), !v6) )
    {
        return;
    }
    if ( mod == 3 || mod == 5 || mod == 7 || mod == 8 || mod == 17 || mod == 16 )
        glass->health = 0.0f;
    else
        glass->health = glass->health - (float)(int)(float)((float)damage * svGlasses.damageMultiplier->current.value);
    if (glass->health <= 0.0)
    {
        //GlassServer::SetState(glass, SHATTERED, pos, dir);
        glass->SetState(GlassState::State::SHATTERED, pos, dir);
    }
    else
    {
        //GlassServer::SetState(glass, CRACKED, 0, 0);
        glass->SetState(GlassState::State::CRACKED, 0, 0);
    }
}

void __cdecl GlassSv_RadiusDamage(
                float *origin,
                float radius,
                float coneAngleCos,
                float *coneDirection,
                float innerDamage,
                float outerDamage,
                int mod)
{
    float v7; // [esp+8h] [ebp-1ECh]
    const float *v8; // [esp+10h] [ebp-1E4h]
    float v9; // [esp+18h] [ebp-1DCh]
    float distSqr; // [esp+20h] [ebp-1D4h]
    float damage; // [esp+24h] [ebp-1D0h]
    float dir[3]; // [esp+28h] [ebp-1CCh] BYREF
    float dist; // [esp+34h] [ebp-1C0h]
    unsigned int i; // [esp+38h] [ebp-1BCh]
    float radiusMaxs[3]; // [esp+3Ch] [ebp-1B8h] BYREF
    unsigned int MAX_CLOSESTS; // [esp+48h] [ebp-1ACh]
    float radiusSqr; // [esp+4Ch] [ebp-1A8h]
    float radiusMins[3]; // [esp+50h] [ebp-1A4h] BYREF
    unsigned int num; // [esp+60h] [ebp-194h]
    const Glass *closest[100]; // [esp+64h] [ebp-190h] BYREF

    if ( svGlasses.numGlasses )
    {
        PROF_SCOPED("GlassSv_RadiusDamage");

        MAX_CLOSESTS = 100;
        radiusSqr = radius * radius;
        //LODWORD(v9) = (1.4142135 * radius) ^ _mask__NegFloat_;
        (v9) = -(1.4142135 * radius);
        radiusMins[0] = *origin + v9;
        radiusMins[1] = origin[1] + v9;
        radiusMins[2] = origin[2] + v9;
        radiusMaxs[0] = *origin + (float)(1.4142135 * radius);
        radiusMaxs[1] = origin[1] + (float)(1.4142135 * radius);
        radiusMaxs[2] = origin[2] + (float)(1.4142135 * radius);
        num = GlassSv_AreaGlasses(radiusMins, radiusMaxs, closest, 0x64u);
        //PIXSetMarker(0, "num=%d", num);
        for ( i = 0; i < num; ++i )
        {
            v8 = closest[i]->origin;
            dir[0] = *v8 - *origin;
            dir[1] = v8[1] - origin[1];
            dir[2] = v8[2] - origin[2];
            distSqr = (float)((float)(dir[0] * dir[0]) + (float)(dir[1] * dir[1])) + (float)(dir[2] * dir[2]);
            if ( distSqr < radiusSqr )
            {
                dist = sqrtf(distSqr);
                if ( dist >= 0.0099999998 )
                {
                    dir[0] = (float)(1.0 / dist) * dir[0];
                    dir[1] = (float)(1.0 / dist) * dir[1];
                    dir[2] = (float)(1.0 / dist) * dir[2];
                }
                else
                {
                    dir[0] = 0.0f;
                    dir[1] = 0.0f;
                    dir[2] = 1.0f;
                }
                if ( !coneDirection
                    || dist <= 0.0099999998
                    || coneAngleCos <= (float)((float)((float)(dir[0] * *coneDirection) + (float)(dir[1] * coneDirection[1]))
                                                                     + (float)(dir[2] * coneDirection[2])) )
                {
                    damage = (float)((float)(innerDamage - outerDamage) * (float)(1.0 - (float)(dist / radius))) + outerDamage;
                    v7 = (float)(damage / (float)closest[i]->glassDef->maxHealth) + 1.0;
                    dir[0] = v7 * dir[0];
                    dir[1] = v7 * dir[1];
                    dir[2] = v7 * dir[2];
                    GlassSv_Damage(closest[i]->index, (int)damage, mod, origin, dir);
                }
            }
        }
    }
}

void __cdecl GlassSv_Touch(unsigned int glassId, gentity_s *other)
{
    float v2[3]; // [esp+30h] [ebp-50h] BYREF
    float aVel[3]; // [esp+3Ch] [ebp-44h] BYREF
    float tVel[3]; // [esp+48h] [ebp-38h] BYREF
    float v5[3]; // [esp+54h] [ebp-2Ch] BYREF
    float out[3]; // [esp+60h] [ebp-20h] BYREF
    const vehicle_physic_t *phys; // [esp+6Ch] [ebp-14h]
    float dir[3]; // [esp+70h] [ebp-10h] BYREF
    const scr_vehicle_s *veh; // [esp+7Ch] [ebp-4h]

    if ( other->s.eType == 14 )
    {
        veh = other->scr_vehicle;
        phys = &veh->phys;
        Vec3NormalizeTo(veh->phys.vel, dir);
        GlassSv_Damage(glassId, 5000, 16, (float*)phys->origin, dir);
    }
    else if ( other->s.eType == 1 && (other->client->ps.pm_flags & 0x400000) != 0 )
    {
        Vec3NormalizeTo(other->client->ps.velocity, out);
        GlassSv_Damage(glassId, 5000, 16, other->client->ps.origin, out);
        Scr_Notify(other, scr_const.dtp_through_glass, 0);
    }
    else if ( other->actor && Flame_GetLocalClientSourceRange() )
    {
        Vec3NormalizeTo(other->actor->Physics.vVelocity, v5);
        GlassSv_Damage(glassId, 5000, 16, other->actor->Physics.vOrigin, v5);
    }
    else if ( other->s.eType == 6 )
    {
        if ( other->physObjId )
        {
            Phys_ObjGetVelocities(other->physObjId, tVel, aVel);
            Vec3NormalizeTo(tVel, v2);
            GlassSv_Damage(glassId, 5000, 16, other->r.currentOrigin, v2);
        }
    }
}

float PREDICT_TIME = 0.15f;
void __cdecl GlassSv_PredictTouch(gentity_s *other)
{
    scr_vehicle_s *scr_vehicle; // ecx
    actor_s *actor; // edx
    float v4; // [esp+6Ch] [ebp-10B0h] BYREF
    float v5; // [esp+70h] [ebp-10ACh]
    float v6; // [esp+74h] [ebp-10A8h]
    float out[3][4]; // [esp+78h] [ebp-10A4h] BYREF
    float v8; // [esp+A8h] [ebp-1074h] BYREF
    float v9; // [esp+ACh] [ebp-1070h]
    float v10; // [esp+B0h] [ebp-106Ch]
    const Glass *v11; // [esp+B4h] [ebp-1068h]
    float v12; // [esp+B8h] [ebp-1064h]
    float v13; // [esp+BCh] [ebp-1060h]
    float v14; // [esp+C0h] [ebp-105Ch]
    float v15; // [esp+C4h] [ebp-1058h]
    float v16; // [esp+C8h] [ebp-1054h]
    float v17; // [esp+CCh] [ebp-1050h]
    unsigned int i; // [esp+D0h] [ebp-104Ch]
    float avel[3]; // [esp+D4h] [ebp-1048h] BYREF
    float mins; // [esp+E0h] [ebp-103Ch] BYREF
    float v21; // [esp+E4h] [ebp-1038h]
    float v22; // [esp+E8h] [ebp-1034h]
    float maxs; // [esp+ECh] [ebp-1030h] BYREF
    float v24; // [esp+F0h] [ebp-102Ch]
    float v25; // [esp+F4h] [ebp-1028h]
    float tvel[3] = { 0.0f, 0.0f, 0.0f }; // [esp+F8h] [ebp-1024h] BYREF
    float v29; // [esp+108h] [ebp-1014h]
    float v30; // [esp+10Ch] [ebp-1010h]
    float v31; // [esp+110h] [ebp-100Ch]
    const Glass *glasses[1025]; // [esp+114h] [ebp-1008h] BYREF
    unsigned int v33; // [esp+1118h] [ebp-4h]

    iassert(other);
    if (svGlasses.numGlasses)
    {
        if (other->physObjId)
        {
            Phys_ObjGetVelocities(other->physObjId, tvel, avel);
        }
        else if (other->scr_vehicle)
        {
            scr_vehicle = other->scr_vehicle;
            tvel[0] = scr_vehicle->phys.vel[0];
            tvel[1] = scr_vehicle->phys.vel[1];
            tvel[2] = scr_vehicle->phys.vel[2];
        }
        else
        {
            if (!other->actor || !Flame_GetLocalClientSourceRange())
                return;
            actor = other->actor;
            tvel[0] = actor->Physics.vVelocity[0];
            tvel[1] = actor->Physics.vVelocity[1];
            tvel[2] = actor->Physics.vVelocity[2];
        }
        PROF_SCOPED("GlassSv_PredictTouch");

        maxs = other->r.absmax[0];
        v24 = other->r.absmax[1];
        v25 = other->r.absmax[2];
        mins = other->r.absmin[0];
        v21 = other->r.absmin[1];
        v22 = other->r.absmin[2];
        v29 = PREDICT_TIME * tvel[0];
        v30 = PREDICT_TIME * tvel[1];
        v31 = PREDICT_TIME * tvel[2];
        mins = mins + (float)(PREDICT_TIME * tvel[0]);
        v21 = v21 + (float)(PREDICT_TIME * tvel[1]);
        v22 = v22 + (float)(PREDICT_TIME * tvel[2]);
        maxs = maxs + (float)(PREDICT_TIME * tvel[0]);
        v24 = v24 + (float)(PREDICT_TIME * tvel[1]);
        v25 = v25 + (float)(PREDICT_TIME * tvel[2]);
        v33 = GlassSv_AreaGlasses(&mins, &maxs, glasses, 0x400u);
        for (i = 0; i < v33; ++i)
        {
            v11 = glasses[i];
            v8 = v11->absmin[0];
            v9 = v11->absmin[1];
            v10 = v11->absmin[2];
            v4 = v11->absmax[0];
            v5 = v11->absmax[1];
            v6 = v11->absmax[2];
            ExpandBoundsToWidth(&v8, &v4);
            v8 = v8 - v29;
            v9 = v9 - v30;
            v10 = v10 - v31;
            v4 = v4 - v29;
            v5 = v5 - v30;
            v6 = v6 - v31;
            if (SV_EntityContact(&v8, &v4, other))
            {
                MatrixInverseOrthogonal43(v11->outlineAxis, (float (*)[3])out);
                v12 = other->r.currentOrigin[0];
                v13 = other->r.currentOrigin[1];
                v14 = other->r.currentOrigin[2];
                v15 = (float)(tvel[0] * out[0][0]) + 0.0;
                v16 = (float)(tvel[0] * out[0][1]) + 0.0;
                v17 = (float)(tvel[0] * out[0][2]) + 0.0;
                v15 = (float)(tvel[1] * out[0][3]) + v15;
                v16 = (float)(tvel[1] * out[1][0]) + v16;
                v17 = (float)(tvel[1] * out[1][1]) + v17;
                v15 = (float)(tvel[2] * out[1][2]) + v15;
                v16 = (float)(tvel[2] * out[1][3]) + v16;
                v17 = (float)(tvel[2] * out[2][0]) + v17;
                if ((float)((float)((float)(v14 * out[2][0])
                    + (float)((float)(v13 * out[1][1]) + (float)((float)(v12 * out[0][2]) + out[2][3])))
                    * v17) < 0.0
                    && (fabs(v17)) > 100.0)
                {
                    GlassSv_Touch(v11->index, other);
                }
            }
        }
    }
}

void __thiscall GlassesServer::WriteSnapshotToClient(msg_t *msg, int sinceTime)
{
    GlassServer *v4; // [esp+18h] [ebp-34h]
    unsigned int value; // [esp+1Ch] [ebp-30h]
    unsigned int j; // [esp+24h] [ebp-28h]
    unsigned int i; // [esp+2Ch] [ebp-20h]
    unsigned int numPositions; // [esp+38h] [ebp-14h]
    unsigned int numShattered; // [esp+3Ch] [ebp-10h]
    bool sendFull; // [esp+42h] [ebp-Ah]
    unsigned int numChanges; // [esp+44h] [ebp-8h]
    unsigned int _numGlasses; // [esp+48h] [ebp-4h]

    // LWSS HACK AAHH
    //MSG_WriteShort(msg, 13337);

    _numGlasses = this->numGlasses;
    if ( _numGlasses )
    {
        PROF_SCOPED("GlassesServer::WriteSnapshotToClient");

        sendFull = 1;
        numChanges = 0;
        if ( sinceTime > 0 )
        {
            numPositions = 0;
            numShattered = 0;
            for ( i = 0; i < _numGlasses; ++i )
            {
                if ( (this->glasses[i].state.val.i & 0xF) == 2 )
                    ++numShattered;
                if ( this->glasses[i].stateChangeTime >= sinceTime )
                {
                    ++numChanges;
                    if ( (this->glasses[i].state.val.i & 0xF) == 2 )
                        ++numPositions;
                }
            }
            if ( !numChanges )
            {
                MSG_WriteBit0(msg);
                MSG_WriteShort(msg, 30154);
                return;
            }
            if ( 192 * numShattered + 2 * _numGlasses > 192 * numPositions + 12 * numChanges )
                sendFull = 0;
        }
        MSG_WriteBit1(msg);
        if ( sendFull )
        {
            MSG_WriteBit1(msg);
            MSG_WriteShort(msg, _numGlasses);
            for ( j = 0; j < _numGlasses; ++j )
            {
                MSG_WriteBits(msg, this->glasses[j].state.val.i & 0xF, 2u);
                if ( (this->glasses[j].state.val.i & 0xF) == 2 )
                {
                    MSG_WriteFloat(msg, (this->glasses[j].hitPos[0]));
                    MSG_WriteFloat(msg, (this->glasses[j].hitPos[1]));
                    MSG_WriteFloat(msg, (this->glasses[j].hitPos[2]));
                    MSG_WriteFloat(msg, (this->glasses[j].hitDir[0]));
                    MSG_WriteFloat(msg, (this->glasses[j].hitDir[1]));
                    MSG_WriteFloat(msg, (this->glasses[j].hitDir[2]));
                }
            }
        }
        else
        {
            MSG_WriteBit0(msg);
            MSG_WriteShort(msg, numChanges);
            for ( value = 0; value < _numGlasses; ++value )
            {
                v4 = &this->glasses[value];
                if ( this->glasses[value].stateChangeTime >= sinceTime )
                {
                    MSG_WriteBits(msg, value, 0xAu);
                    MSG_WriteBits(msg, v4->state.val.i & 0xF, 2u);
                    if ( (v4->state.val.i & 0xF) == 2 )
                    {
                        MSG_WriteFloat(msg, (this->glasses[value].hitPos[0]));
                        MSG_WriteFloat(msg, (this->glasses[value].hitPos[1]));
                        MSG_WriteFloat(msg, (this->glasses[value].hitPos[2]));
                        MSG_WriteFloat(msg, (this->glasses[value].hitDir[0]));
                        MSG_WriteFloat(msg, (this->glasses[value].hitDir[1]));
                        MSG_WriteFloat(msg, (this->glasses[value].hitDir[2]));
                    }
                }
            }
        }
        MSG_WriteShort(msg, 30154);
    }
}

void __cdecl GlassSv_WriteSnapshotToClient(msg_t *msg, int sinceTime)
{
    //GlassesServer::WriteSnapshotToClient(&svGlasses, msg, sinceTime);
    svGlasses.WriteSnapshotToClient(msg, sinceTime);
}

void __cdecl GlassSv_WriteGameState(msg_t *msg)
{
    GlassSv_WriteSnapshotToClient(msg, 0);
}

