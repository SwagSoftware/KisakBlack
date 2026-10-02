#include "g_misc_mp.h"
#include <clientscript/cscr_stringlist.h>
#include "g_main_mp.h"
#include <cgame/cg_drawtools.h>
#include <bgame/bg_weapons.h>
#include "g_utils_mp.h"
#include <cgame_mp/cg_view_mp.h>
#include <gfx_d3d/r_primarylights.h>
#include "g_spawn_mp.h"
#include <game/g_load_utils.h>
#include <cgame_mp/cg_ents_mp.h>
#include <cgame/cg_scr_main.h>
#include <server/sv_world.h>
#include <client_mp/g_client_mp.h>
#include <bgame/bg_misc.h>
#include <clientscript/cscr_vm.h>

static const float color[4] = { 0.5, 0.5, 0.5, 1.0 };
void __cdecl misc_EntInfo(gentity_s *self, float *source)
{
    char *v2; // eax
    char *v3; // eax
    const char *v4; // [esp+10h] [ebp-3Ch]
    float xyz[3]; // [esp+1Ch] [ebp-30h] BYREF
    float mins[3]; // [esp+28h] [ebp-24h] BYREF
    float dist; // [esp+34h] [ebp-18h] BYREF
    float maxs[3]; // [esp+38h] [ebp-14h] BYREF
    const char *classname; // [esp+44h] [ebp-8h]
    float infoScale; // [esp+48h] [ebp-4h] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_misc_mp.cpp", 61, 0, "%s", "self") )
        __debugbreak();
    EntinfoPosAndScale(self, source, xyz, &infoScale, &dist);
    if ( !self->classname
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_misc_mp.cpp", 65, 0, "%s", "self->classname") )
    {
        __debugbreak();
    }
    classname = SL_ConvertToString(self->classname, SCRIPTINSTANCE_SERVER);
    if ( g_entinfo->current.integer == 4 || g_entinfo->current.integer == 5 )
    {
        v2 = va("%i", self->s.number);
        G_AddDebugString(xyz, color, infoScale * 0.75, v2, 0);
    }
    else
    {
        if ( self->targetname )
            v4 = SL_ConvertToString(self->targetname, SCRIPTINSTANCE_SERVER);
        else
            v4 = "<noname>";
        v3 = va("%i : %s : %i : %s", self->s.number, v4, self->health, classname);
        G_AddDebugString(xyz, color, infoScale * 0.75, v3, 0);
    }
    if ( I_stristr(classname, "_spawn") || I_stristr(classname, "_uspawn") )
    {
        mins[0] = -16.0f;
        mins[1] = -16.0f;
        mins[2] = 0.0f;
        maxs[0] = 16.0f;
        maxs[1] = 16.0f;
        maxs[2] = 72.0f;
    }
    else
    {
        mins[0] = self->r.mins[0];
        mins[1] = self->r.mins[1];
        mins[2] = self->r.mins[2];
        maxs[0] = self->r.maxs[0];
        maxs[1] = self->r.maxs[1];
        maxs[2] = self->r.maxs[2];
    }
    CG_DebugBox(self->r.currentOrigin, mins, maxs, self->r.currentAngles[1], colorMagenta, 1, 0);
}

void __cdecl EntinfoPosAndScale(gentity_s *self, float *source, float *pos, float *textScale, float *dist)
{
    float delta[3]; // [esp+14h] [ebp-Ch] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_misc_mp.cpp", 26, 0, "%s", "self") )
        __debugbreak();
    if ( self->r.absmin[0] == 0.0 || self->r.absmin[1] == 0.0 )
    {
        *pos = self->r.currentOrigin[0];
        pos[1] = self->r.currentOrigin[1];
        pos[2] = self->r.currentOrigin[2];
    }
    else
    {
        *pos = self->r.absmin[0] + self->r.absmax[0];
        pos[1] = self->r.absmin[1] + self->r.absmax[1];
        pos[2] = self->r.absmin[2] + self->r.absmax[2];
        *pos = 0.5 * *pos;
        pos[1] = 0.5 * pos[1];
        pos[2] = 0.5 * pos[2];
    }
    *textScale = G_GetEntInfoScale();
    delta[0] = *source - self->r.currentOrigin[0];
    delta[1] = source[1] - self->r.currentOrigin[1];
    delta[2] = source[2] - self->r.currentOrigin[2];
    *dist = Vec3Length(delta);
    if ( g_entinfo->current.integer >= 2
        || g_entinfo_maxdist->current.value <= 0.0
        || *dist <= g_entinfo_maxdist->current.value )
    {
        *textScale = (float)(*dist * 0.0026041667) * *textScale;
    }
}

void __cdecl EntInfo_Vehicle(gentity_s *self, float *source)
{
    misc_EntInfo(self, source);
    CG_DebugBox(self->r.currentOrigin, self->r.mins, self->r.maxs, 0.0, colorCyan, 1, 0);
}

unsigned int _S1_8;
float MY_MAX_DIST_HALF;
static const float MY_MAX_DIST = 500.0f;
static const float MY_NEXTLINE = -3.0f;
static const float MY_RGB[3] = { 0.6f, 0.5f, 0.5f };
void __cdecl EntInfo_Item(gentity_s *self, float *source)
{
    const char *v2; // eax
    char *v3; // eax
    int scale; // [esp+4h] [ebp-34h]
    int ammoCount; // [esp+8h] [ebp-30h]
    float pos[3]; // [esp+10h] [ebp-28h] BYREF
    float textScale; // [esp+1Ch] [ebp-1Ch] BYREF
    float dist; // [esp+20h] [ebp-18h] BYREF
    int idx; // [esp+24h] [ebp-14h]
    float color[4]; // [esp+28h] [ebp-10h] BYREF

    if ((_S1_8 & 1) == 0)
    {
        _S1_8 |= 1u;
        MY_MAX_DIST_HALF = MY_MAX_DIST * 0.5;
    }
    if (!self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_misc_mp.cpp", 108, 0, "%s", "self"))
        __debugbreak();
    EntinfoPosAndScale(self, source, pos, &textScale, &dist);
    if (dist <= MY_MAX_DIST)
    {
        color[0] = MY_RGB[0];
        color[1] = MY_RGB[1];
        color[2] = MY_RGB[2];
        if (MY_MAX_DIST_HALF <= dist)
            color[3] = 1.0 - (float)((float)(dist - MY_MAX_DIST_HALF) / MY_MAX_DIST_HALF);
        else
            color[3] = 1.0f;
        pos[2] = pos[2] - (float)(MY_NEXTLINE * 0.5);
        for (idx = 0; idx < 2; ++idx)
        {
            if (self->item[idx].index)
            {
                ammoCount = self->item[idx].ammoCount;
                scale = self->item[idx].clipAmmoCount;
                v2 = BG_WeaponName(self->item[idx].index);
                v3 = va("%i: %s (%i + %i)", self->s.number, v2, scale, ammoCount);
                G_AddDebugString(pos, color, textScale * 0.80000001, v3, 0);
                CG_DebugBox(self->r.currentOrigin, self->r.mins, self->r.maxs, self->r.currentAngles[1], colorRedFaded, 1, 0);
                pos[2] = pos[2] + MY_NEXTLINE;
            }
        }
    }
}

float __cdecl G_GetEntInfoScale()
{
    return (float)((float)((float)tan((float)DEG2RAD(80.0f) * 0.5f) * 0.75f) * 1.5890048f) * g_entinfo_scale->current.value;
}

void __cdecl SP_info_notnull(gentity_s *self, SpawnVar *v)
{
    G_SetOrigin(self, self->r.currentOrigin);
}

void __cdecl SP_light(gentity_s *self, SpawnVar *spawnVar)
{
    float radius; // [esp+8h] [ebp-CCh]
    float v3; // [esp+18h] [ebp-BCh]
    int primaryLightIndex; // [esp+A0h] [ebp-34h] BYREF
    const ComPrimaryLight *light; // [esp+A4h] [ebp-30h]
    float facingDir[3]; // [esp+A8h] [ebp-2Ch] BYREF
    float facingAngles[3]; // [esp+B4h] [ebp-20h] BYREF
    float normalizedColor[3]; // [esp+C0h] [ebp-14h] BYREF
    unsigned int flags; // [esp+CCh] [ebp-8h]
    const char *spawnFlags; // [esp+D0h] [ebp-4h] BYREF
    int savedregs; // [esp+D4h] [ebp+0h] BYREF

    if ( !spawnVar->spawnVarsValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_misc_mp.cpp",
                    169,
                    0,
                    "%s",
                    "spawnVar->spawnVarsValid") )
    {
        __debugbreak();
    }
    if ( G_SpawnInt(spawnVar, "pl#", "0", &primaryLightIndex)
        && (G_SpawnString(spawnVar, "spawnflags", 0, &spawnFlags), (flags = atoi(spawnFlags)) != 0)
        && (flags & 0x10) != 0 )
    {
        light = Com_GetPrimaryLight(primaryLightIndex);
        self->s.index.brushmodel = primaryLightIndex;
        if ( self->s.index.brushmodel != primaryLightIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_misc_mp.cpp",
                        192,
                        1,
                        "self->s.index.primaryLight == primaryLightIndex\n\t%i, %i",
                        self->s.index.brushmodel,
                        primaryLightIndex) )
        {
            __debugbreak();
        }
        self->s.lerp.u.turret.gunAngles[1] = ColorNormalize(light->color, normalizedColor);
        Byte4PackRgba(normalizedColor, (unsigned __int8 *)&self->s.lerp.u);
        self->s.lerp.u.primaryLight.colorAndExp[3] = light->exponent;
        self->s.lerp.u.turret.gunAngles[2] = light->radius;
        self->s.lerp.u.primaryLight.cosHalfFovOuter = light->cosHalfFovOuter;
        self->s.lerp.u.turret.heatVal = light->cosHalfFovInner;
        facingDir[0] = -light->dir[0];
        facingDir[1] = -light->dir[1];
        facingDir[2] = -light->dir[2];
        vectoangles(facingDir, facingAngles);
        G_SetAngle(self, facingAngles);
        G_SetOrigin(self, light->origin);
        v3 = -light->radius;
        self->r.mins[0] = v3;
        self->r.mins[1] = v3;
        self->r.mins[2] = v3;
        radius = light->radius;
        self->r.maxs[0] = radius;
        self->r.maxs[1] = radius;
        self->r.maxs[2] = radius;
        self->s.eType = ET_PRIMARY_LIGHT;
        if ( self->r.contents )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_misc_mp.cpp",
                            211,
                            1,
                            "%s",
                            "self->r.contents == 0") )
                __debugbreak();
        }
        self->handler = 23;
        self->flags |= 0x1000u;
        SV_LinkEntity(self);
    }
    else
    {
        G_FreeEntity(self);
    }
}

void __cdecl TeleportPlayer(gentity_s *player, float *origin, float *angles)
{
    float *v3; // [esp+4h] [ebp-Ch]
    float *v4; // [esp+8h] [ebp-8h]
    int linked; // [esp+Ch] [ebp-4h]
    int savedregs; // [esp+10h] [ebp+0h] BYREF

    if ( !player->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_misc_mp.cpp", 229, 0, "%s", "player->client") )
    {
        __debugbreak();
    }
    if ( player->client->sess.connected == CON_DISCONNECTED
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_misc_mp.cpp",
                    230,
                    0,
                    "%s",
                    "player->client->sess.connected != CON_DISCONNECTED") )
    {
        __debugbreak();
    }
    linked = player->r.linked;
    SV_UnlinkEntity(player);
    v4 = player->client->ps.origin;
    *v4 = *origin;
    v4[1] = origin[1];
    v4[2] = origin[2];
    player->client->ps.origin[2] = player->client->ps.origin[2] + 1.0;
    player->client->ps.eFlags ^= 2u;
    SetClientViewAngle(player, angles);
    if ( !player->tagInfo )
        player->r.currentAngles[0] = 0.0f;
    BG_PlayerStateToEntityState(&player->client->ps, &player->s, 1, 1u);
    v3 = player->client->ps.origin;
    player->r.currentOrigin[0] = *v3;
    player->r.currentOrigin[1] = v3[1];
    player->r.currentOrigin[2] = v3[2];
    if ( linked )
        SV_LinkEntity(player);
}

void __cdecl ScrNotify_FaceEvent(gentity_s *self, unsigned __int16 face_event)
{
    unsigned __int16 t; // [esp+0h] [ebp-4h]

    Scr_AddEntity(self, SCRIPTINSTANCE_SERVER);
    Scr_AddConstString(face_event, SCRIPTINSTANCE_SERVER);
    t = Scr_ExecThread(SCRIPTINSTANCE_SERVER, g_scr_data.faceeventnotify, 2u);
    Scr_FreeThread(t, SCRIPTINSTANCE_SERVER);
}

