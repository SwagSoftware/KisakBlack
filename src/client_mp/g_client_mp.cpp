#include "g_client_mp.h"
#include <bgame/bg_misc.h>
#include <clientscript/scr_const.h>
#include <game_mp/g_utils_mp.h>
#include <bgame/bg_weapons_def.h>
#include <universal/com_math_anglevectors.h>
#include <server_mp/sv_init_mp.h>
#include <server/sv_game.h>
#include <game_mp/g_spawn_mp.h>
#include <game/turret.h>
#include <server/sv_world.h>
#include <clientscript/cscr_stringlist.h>
#include <game_mp/g_active_mp.h>
#include <clientscript/cscr_vm.h>
#include <game_mp/g_cmds_mp.h>
#include <game_mp/g_team_mp.h>

void __cdecl SetClientViewAngle(gentity_s *ent, const float *angle)
{
    double v2; // st7
    float *viewangles; // [esp+30h] [ebp-34h]
    float fDeltad; // [esp+50h] [ebp-14h]
    float fDelta; // [esp+50h] [ebp-14h]
    float fDeltaa; // [esp+50h] [ebp-14h]
    float fDeltae; // [esp+50h] [ebp-14h]
    float fDeltab; // [esp+50h] [ebp-14h]
    float fDeltac; // [esp+50h] [ebp-14h]
    float newAngle[3]; // [esp+54h] [ebp-10h]
    int i; // [esp+60h] [ebp-4h]

    newAngle[0] = *angle;
    newAngle[1] = angle[1];
    newAngle[2] = angle[2];
    if ( (ent->client->ps.pm_flags & 1) != 0 && (ent->client->ps.eFlags & 0x300) == 0 )
    {
        fDeltad = AngleDelta(ent->client->ps.proneDirection, newAngle[1]);
        fDelta = AngleNormalize180(fDeltad);
        if ( fDelta > bg_prone_yawcap->current.value || (-bg_prone_yawcap->current.value) > fDelta )
        {
            if ( fDelta <= bg_prone_yawcap->current.value )
                fDeltaa = fDelta + bg_prone_yawcap->current.value;
            else
                fDeltaa = fDelta - bg_prone_yawcap->current.value;
            ent->client->ps.delta_angles[1] = ent->client->ps.delta_angles[1] + fDeltaa;
            if ( fDeltaa <= 0.0 )
                newAngle[1] = AngleNormalize360(ent->client->ps.proneDirection + bg_prone_yawcap->current.value);
            else
                newAngle[1] = AngleNormalize360(ent->client->ps.proneDirection - bg_prone_yawcap->current.value);
        }
        fDeltae = AngleDelta(ent->client->ps.proneTorsoPitch, newAngle[0]);
        fDeltab = AngleNormalize180(fDeltae);
        if ( fDeltab > 45.0 || fDeltab < -45.0 )
        {
            if ( fDeltab <= 45.0 )
                fDeltac = fDeltab + 45.0;
            else
                fDeltac = fDeltab - 45.0;
            ent->client->ps.delta_angles[0] = ent->client->ps.delta_angles[0] + fDeltac;
            if ( fDeltac <= 0.0 )
                newAngle[0] = AngleNormalize180(ent->client->ps.proneTorsoPitch + 45.0);
            else
                newAngle[0] = AngleNormalize180(ent->client->ps.proneTorsoPitch - 45.0);
        }
    }
    for ( i = 0; i < 3; ++i )
    {
        v2 = AngleNormalize180(newAngle[i] - (float)((float)ent->client->sess.cmd.angles[i] * 0.0054931641));
        ent->client->ps.delta_angles[i] = v2;
    }
    ent->r.currentAngles[0] = newAngle[0];
    ent->r.currentAngles[1] = newAngle[1];
    ent->r.currentAngles[2] = newAngle[2];
    viewangles = ent->client->ps.viewangles;
    *viewangles = ent->r.currentAngles[0];
    viewangles[1] = ent->r.currentAngles[1];
    viewangles[2] = ent->r.currentAngles[2];
    Com_Printf(
        0,
        "SetClientViewAngle() - (%3.0f, %3.0f, %3.0f), delta (%3.0f, %3.0f, %3.0f)\n",
        newAngle[0],
        newAngle[1],
        newAngle[2],
        ent->client->ps.delta_angles[0],
        ent->client->ps.delta_angles[1],
        ent->client->ps.delta_angles[2]);
}

void __cdecl G_GetPlayerViewOrigin_Internal(const playerState_s *ps, float *origin, bool useBodyPosition)
{
    float v3; // [esp+10h] [ebp-A4h]
    float v4; // [esp+18h] [ebp-9Ch]
    float v5; // [esp+1Ch] [ebp-98h]
    float v6; // [esp+20h] [ebp-94h]
    float viewHeightCurrent; // [esp+2Ch] [ebp-88h]
    unsigned int weapon; // [esp+40h] [ebp-74h]
    gentity_s *vehicle; // [esp+44h] [ebp-70h]
    float viewAxis[3][3]; // [esp+48h] [ebp-6Ch] BYREF
    float offsetVec[3]; // [esp+6Ch] [ebp-48h] BYREF
    unsigned __int16 vehicleTag; // [esp+78h] [ebp-3Ch]
    float playerMtx[4][3]; // [esp+7Ch] [ebp-38h] BYREF
    const WeaponDef *weapDef; // [esp+ACh] [ebp-8h]
    gentity_s *turretEnt; // [esp+B0h] [ebp-4h]

    if ((ps->eFlags & 0x300) != 0)
    {
        if (ps->viewlocked == PLAYERVIEWLOCK_NONE
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp",
                109,
                0,
                "%s",
                "ps->viewlocked"))
        {
            __debugbreak();
        }
        if (ps->viewlocked_entNum == 1023
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp",
                110,
                0,
                "%s",
                "ps->viewlocked_entNum != ENTITYNUM_NONE"))
        {
            __debugbreak();
        }
        turretEnt = &g_entities[ps->viewlocked_entNum];
        if (!G_DObjGetWorldTagPos(turretEnt, scr_const.tag_player, origin))
        {
            Com_Error(ERR_DROP, "G_GetPlayerViewOrigin: Couldn't find [tag_player] on turret");
            *origin = ps->origin[0];
            origin[1] = ps->origin[1];
            origin[2] = ps->origin[2];
            origin[2] = origin[2] + 8.0;
        }
    }
    else if ((ps->eFlags & 0x4000) == 0 || useBodyPosition && (ps->eFlags2 & 0x10000000) != 0)
    {
        BG_GetPlayerViewOrigin(ps, origin, level.time);
    }
    else
    {
        if (ps->viewlocked_entNum == 1023
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp",
                128,
                0,
                "%s",
                "ps->viewlocked_entNum != ENTITYNUM_NONE"))
        {
            __debugbreak();
        }
        vehicle = &g_entities[ps->viewlocked_entNum];
        vehicleTag = scr_const.tag_barrel;
        if (ps->vehiclePos < 1 || ps->vehiclePos > 4)
        {
            if (!ps->vehiclePos)
                vehicleTag = scr_const.tag_player;
        }
        else
        {
            vehicleTag = scr_const.tag_gunner_barrel1;
        }
        if (G_DObjGetWorldTagMatrix(vehicle, vehicleTag, playerMtx)
            || (vehicleTag = scr_const.tag_player, G_DObjGetWorldTagMatrix(vehicle, scr_const.tag_player, playerMtx)))
        {
            weapon = G_GetPlayerWeapon(ps, 0);
            weapDef = BG_GetWeaponDef(weapon);
            *origin = playerMtx[3][0];
            origin[1] = playerMtx[3][1];
            origin[2] = playerMtx[3][2];
            if (!ps->vehiclePos)
            {
                if ((ps->eFlags2 & 0x10000000) != 0)
                {
                    *origin = (float)(60.0 * playerMtx[2][0]) + *origin;
                    origin[1] = (float)(60.0 * playerMtx[2][1]) + origin[1];
                    origin[2] = (float)(60.0 * playerMtx[2][2]) + origin[2];
                }
                else
                {
                    viewHeightCurrent = ps->viewHeightCurrent;
                    *origin = (float)(viewHeightCurrent * playerMtx[2][0]) + *origin;
                    origin[1] = (float)(viewHeightCurrent * playerMtx[2][1]) + origin[1];
                    origin[2] = (float)(viewHeightCurrent * playerMtx[2][2]) + origin[2];
                }
            }
            memset(offsetVec, 0, sizeof(offsetVec));
            if (weapDef && weapon)
                BG_CalcVehicleTurretWeaponPosOffset(ps->fWeaponPosFrac, weapDef, offsetVec);
            if ((-(offsetVec[0])) <= 200.0)
            {
                v5 = offsetVec[0];
                *origin = (float)(offsetVec[0] * playerMtx[0][0]) + *origin;
                origin[1] = (float)(v5 * playerMtx[0][1]) + origin[1];
                origin[2] = (float)(v5 * playerMtx[0][2]) + origin[2];
            }
            else
            {
                AnglesToAxis(ps->viewangles, viewAxis);
                v6 = offsetVec[0];
                *origin = (float)(offsetVec[0] * viewAxis[0][0]) + *origin;
                origin[1] = (float)(v6 * viewAxis[0][1]) + origin[1];
                origin[2] = (float)(v6 * viewAxis[0][2]) + origin[2];
            }
            v4 = offsetVec[1];
            *origin = (float)(offsetVec[1] * playerMtx[1][0]) + *origin;
            origin[1] = (float)(v4 * playerMtx[1][1]) + origin[1];
            origin[2] = (float)(v4 * playerMtx[1][2]) + origin[2];
            v3 = offsetVec[2];
            *origin = (float)(offsetVec[2] * playerMtx[2][0]) + *origin;
            origin[1] = (float)(v3 * playerMtx[2][1]) + origin[1];
            origin[2] = (float)(v3 * playerMtx[2][2]) + origin[2];
        }
        else
        {
            Com_Error(ERR_DROP, "G_GetPlayerViewOrigin: Couldn',27h,'t find [tag_player] on vehicle");
            *origin = ps->origin[0];
            origin[1] = ps->origin[1];
            origin[2] = ps->origin[2];
            origin[2] = origin[2] + 8.0;
        }
    }
}

void __cdecl G_GetPlayerViewOrigin(const playerState_s *ps, float *origin)
{
    G_GetPlayerViewOrigin_Internal(ps, origin, false);
}

void __cdecl G_GetPlayerBodyViewOrigin(const playerState_s *ps, float *origin)
{
    G_GetPlayerViewOrigin_Internal(ps, origin, true);
}

void __cdecl G_GetPlayerViewDirection(const gentity_s *ent, float *forward, float *right, float *up)
{
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp", 215, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp", 216, 0, "%s", "ent->client") )
    {
        __debugbreak();
    }
    BG_GetPlayerViewDirection(&ent->client->ps, forward, right, up);
}

void __cdecl ClientUserinfoChanged(unsigned int clientNum)
{
    char *v1; // eax
    unsigned __int64 xuid; // [esp+8h] [ebp-820h] BYREF
    gclient_s *client; // [esp+14h] [ebp-814h]
    char oldname[1024]; // [esp+18h] [ebp-810h] BYREF
    char userinfo[1024]; // [esp+418h] [ebp-410h] BYREF
    clientInfo_t *ci; // [esp+81Ch] [ebp-Ch]
    gentity_s *ent; // [esp+820h] [ebp-8h]
    const char *s; // [esp+824h] [ebp-4h]

    ent = &g_entities[clientNum];
    client = ent->client;
    if ( clientNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp",
                    317,
                    0,
                    "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    clientNum,
                    32) )
    {
        __debugbreak();
    }
    SV_GetUserinfo(clientNum, userinfo, 1024);
    if ( !Info_Validate(userinfo) )
        strcpy(userinfo, "\\name\\badinfo");
    client->sess.localClient = SV_IsLocalClient(clientNum);
    s = Info_ValueForKey(userinfo, "cg_predictItems");
    if ( atoi(s) )
        client->sess.predictItemPickup = 1;
    else
        client->sess.predictItemPickup = 0;
    if ( client->sess.connected == CON_CONNECTED && level.manualNameChange )
    {
        s = Info_ValueForKey(userinfo, "name");
        ClientCleanName(s, client->sess.newnetname, 32);
    }
    else
    {
        I_strncpyz(oldname, client->sess.cs.name, 1024);
        s = Info_ValueForKey(userinfo, "name");
        ClientCleanName(s, client->sess.cs.name, 32);
        I_strncpyz(client->sess.newnetname, client->sess.cs.name, 32);
        s = Info_ValueForKey(userinfo, "clanAbbrev");
        if ( *s )
            I_strncpyz(client->sess.cs.clanAbbrev, s, 8);
        else
            client->sess.cs.clanAbbrev[0] = 0;
        v1 = Info_ValueForKey(userinfo, "bdOnlineUserID");
        StringToXUID(v1, &xuid);
        if ( (xuid & 0xFFFFFF00) == 0xFFFFFF00 && HIDWORD(xuid) == -1 )
        {
            LODWORD(xuid) = clientNum | 0xFFFFFF00;
            HIDWORD(xuid) = -1;
        }
        client->sess.cs.xuid = xuid;
    }
    if ( clientNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp",
                    374,
                    0,
                    "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    clientNum,
                    32) )
    {
        __debugbreak();
    }
    ci = &level_bgs.clientinfo[clientNum];
    if ( !ci->infoValid
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp", 376, 0, "%s", "ci->infoValid") )
    {
        __debugbreak();
    }
    ci->clientNum = clientNum;
    I_strncpyz(ci->name, client->sess.cs.name, 32);
    ci->team = client->sess.cs.team;
    if ( !ent->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp", 385, 0, "%s", "ent->sentient") )
    {
        __debugbreak();
    }
    ent->sentient->eTeam = client->sess.cs.team;
}

void __cdecl ClientCleanName(const char *in, char *out, int outSize)
{
    char v3; // [esp+3h] [ebp-11h]
    int len; // [esp+4h] [ebp-10h]
    int colorlessLen; // [esp+8h] [ebp-Ch]
    char *p; // [esp+Ch] [ebp-8h]
    int spaces; // [esp+10h] [ebp-4h]
    int outSizea; // [esp+24h] [ebp+10h]

    outSizea = outSize - 1;
    len = 0;
    colorlessLen = 0;
    p = out;
    *out = 0;
    spaces = 0;
    while ( 1 )
    {
        v3 = *in++;
        if ( !v3 )
            break;
        if ( *p || v3 != 32 )
        {
            if ( v3 == 94 )
            {
                ++in;
            }
            else
            {
                if ( v3 != 32 )
                {
                    spaces = 0;
                    goto LABEL_11;
                }
                if ( ++spaces <= 3 )
                {
LABEL_11:
                    if ( len > outSizea - 1 )
                        break;
                    *out++ = v3;
                    ++colorlessLen;
                    ++len;
                }
            }
        }
    }
    *out = 0;
    if ( !*p || !colorlessLen )
        I_strncpyz(p, "UnnamedPlayer", outSizea);
}

const char *__cdecl ClientConnect(unsigned int clientNum, unsigned int scriptPersId)
{
    gclient_s *client; // [esp+0h] [ebp-420h]
    XAnimTree_s *pXAnimTree; // [esp+4h] [ebp-41Ch]
    sentient_s *sentient; // [esp+8h] [ebp-418h]
    char userinfo[1028]; // [esp+10h] [ebp-410h] BYREF
    clientInfo_t *ci; // [esp+418h] [ebp-8h]
    gentity_s *ent; // [esp+41Ch] [ebp-4h]

    if ( !scriptPersId
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp", 429, 0, "%s", "scriptPersId") )
    {
        __debugbreak();
    }
    if ( clientNum >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp",
                    431,
                    0,
                    "clientNum doesn't index level.maxclients\n\t%i not in [0, %i)",
                    clientNum,
                    level.maxclients) )
    {
        __debugbreak();
    }
    if ( G_OnlyConnectingPaths() && !clientNum )
        Path_InitPaths();
    ent = &g_entities[clientNum];
    client = &level.clients[clientNum];
    ClientClearFields(client);
    memset(client, 0, sizeof(gclient_s));
    ci = &level_bgs.clientinfo[clientNum];
    pXAnimTree = ci->pXAnimTree;
    memset(ci, 0, sizeof(clientInfo_t));
    ci->pXAnimTree = pXAnimTree;
    ci->infoValid = 1;
    ci->nextValid = 1;
    client->sess.connected = CON_CONNECTING;
    client->sess.scriptPersId = scriptPersId;
    client->sess.cs.team = TEAM_FREE;//  RETURN_ZERO32();
    client->sess.sessionState = SESS_STATE_SPECTATOR;
    client->spectatorClient = -1;
    client->sess.forceSpectatorClient = -1;
    client->sess.killCamEntity = -1;
    client->sess.killCamTargetEntity = -1;
    G_InitGentity(ent);
    ent->handler = 0;
    ent->client = client;
    AssignToSmallerType<unsigned char>(&ent->s.clientNum, clientNum);
    client->sess.cs.clientIndex = clientNum;
    AssignToSmallerType<unsigned char>(&client->ps.clientNum, clientNum);
    client->ps.corpseIndex = -1;
    client->sess.moveSpeedScaleMultiplier = 1.0f;
    client->ps.moveSpeedScaleMultiplier = client->sess.moveSpeedScaleMultiplier;
    sentient = Sentient_Alloc();
    if ( !sentient )
        Com_Error(ERR_DROP, "No sentient for player.");
    ent->sentient = sentient;
    sentient->ent = ent;
    sentient->eTeam = client->sess.cs.team;
    sentient->maxVisibleDist = 8192.0f;
    sentient->attackerAccuracy = 1.0f;
    ClientUserinfoChanged(clientNum);
    SV_GetUserinfo(clientNum, userinfo, 1024);
    if ( !G_ExitAfterToolComplete() )
        Scr_PlayerConnect(ent);
    CalculateRanks();
    if ( ent->s.clientNum != ent->s.number
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp",
                    515,
                    0,
                    "%s",
                    "es->clientNum == es->number") )
    {
        __debugbreak();
    }
    return 0;
}

void __cdecl ClientClearFields(gclient_s *client)
{
    client->useHoldEntity.setEnt(0);
}

void __cdecl ClientBegin(unsigned int clientNum)
{
    gclient_s *client; // [esp+0h] [ebp-4h]

    if ( clientNum >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp",
                    534,
                    0,
                    "clientNum doesn't index level.maxclients\n\t%i not in [0, %i)",
                    clientNum,
                    level.maxclients) )
    {
        __debugbreak();
    }
    client = &level.clients[clientNum];
    client->sess.connected = CON_CONNECTED;
    client->ps.pm_type = 4;
    CalculateRanks();
    Scr_Notify(&g_entities[clientNum], scr_const.begin, 0);
}

void __cdecl ClientSpawn(gentity_s *ent, const float *spawn_origin, const float *spawn_angles)
{
    int m; // [esp+8h] [ebp-26Ch]
    char v4; // [esp+Fh] [ebp-265h]
    int k; // [esp+10h] [ebp-264h]
    char v6; // [esp+17h] [ebp-25Dh]
    int j; // [esp+18h] [ebp-25Ch]
    float v8; // [esp+20h] [ebp-254h]
    int i; // [esp+58h] [ebp-21Ch]
    gclient_s *client; // [esp+5Ch] [ebp-218h]
    float clean_spawn_angles[3]; // [esp+60h] [ebp-214h] BYREF
    int index; // [esp+6Ch] [ebp-208h]
    int iFlags; // [esp+70h] [ebp-204h]
    clientSession_t savedSess; // [esp+74h] [ebp-200h] BYREF
    int savedSpawnCount; // [esp+26Ch] [ebp-8h]
    int savedServerTime; // [esp+270h] [ebp-4h]

    //clientSession_t::clientSession_t(&savedSess);
    index = ent - g_entities;
    client = ent->client;
    if ( (unsigned int)index >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp",
                    589,
                    0,
                    "index doesn't index level.maxclients\n\t%i not in [0, %i)",
                    index,
                    level.maxclients) )
    {
        __debugbreak();
    }
    if ( ent->client != &level.clients[index]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp",
                    591,
                    0,
                    "%s",
                    "ent->client == &level.clients[index]") )
    {
        __debugbreak();
    }
    if ( !ent->r.inuse
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp", 592, 0, "%s", "ent->r.inuse") )
    {
        __debugbreak();
    }
    if ( (client->ps.otherFlags & 4) != 0 && (client->ps.eFlags & 0x300) != 0 )
    {
        if ( client->ps.clientNum != ent->s.number
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp",
                        597,
                        0,
                        "%s",
                        "client->ps.clientNum == ent->s.number") )
        {
            __debugbreak();
        }
        if ( client->ps.viewlocked_entNum == 1023
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp",
                        598,
                        0,
                        "%s",
                        "client->ps.viewlocked_entNum != ENTITYNUM_NONE") )
        {
            __debugbreak();
        }
        iassert(level.gentities[client->ps.viewlocked_entNum].r.ownerNum.isDefined());
        iassert(level.gentities[client->ps.viewlocked_entNum].r.ownerNum.ent() == ent);

        G_ClientStopUsingTurret(&level.gentities[client->ps.viewlocked_entNum]);
    }
    G_EntUnlink(ent);
    if ( ent->r.linked )
        SV_UnlinkEntity(ent);
    AssignToSmallerType<int>(&ent->s.lerp.useCount, ent->useCount + 1);
    ent->s.groundEntityNum = 1023;
    Scr_SetString(&ent->classname, scr_const.player, SCRIPTINSTANCE_SERVER);
    ent->clipmask = 0x2818011;
    ent->r.svFlags |= 1u;
    ent->takedamage = 0;
    G_SetClientContents(ent);
    ent->handler = 14;
    ent->flags = 4096;
    ent->r.mins[0] = playerMins[0];
    ent->r.mins[1] = -15.0;
    ent->r.mins[2] = 0.0;
    ent->r.maxs[0] = playerMaxs[0];
    ent->r.maxs[1] = 15.0;
    ent->r.maxs[2] = 70.0;
    iFlags = client->ps.eFlags & 0x100002;
    memcpy(&savedSess, &client->sess, sizeof(savedSess));
    savedSpawnCount = client->ps.stats[4];
    savedServerTime = client->lastServerTime;
    ClientClearFields(client);
    memset(client, 0, sizeof(gclient_s));
    memcpy(&client->sess, &savedSess, sizeof(client->sess));
    client->lastServerTime = savedServerTime;
    client->spectatorClient = -1;
    client->ps.stats[4] = savedSpawnCount + 1;
    client->ps.stats[2] = client->sess.maxHealth;
    client->ps.eFlags = iFlags;
    client->sess.cs.clientIndex = index;
    client->sess.cs.attachedVehEntNum = 1023;
    AssignToSmallerType<unsigned char>(&client->ps.clientNum, index);
    client->ps.viewlocked_entNum = 1023;
    if ( (unsigned int)(client - level.clients) >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp",
                    658,
                    0,
                    "client - level.clients doesn't index level.maxclients\n\t%i not in [0, %i)",
                    client - level.clients,
                    level.maxclients) )
    {
        __debugbreak();
    }
    SV_GetUsercmd(client - level.clients, &client->sess.cmd);
    client->ps.eFlags ^= 2u;
    client->ps.viewHeightTarget = 60;
    client->ps.viewHeightCurrent = 60.0f;
    client->ps.viewHeightLerpTime = 0;
    client->ps.dofNearBlur = 6.0f;
    client->ps.dofFarBlur = 1.8f;
    client->ps.spreadOverride = 0;
    client->ps.spreadOverrideState = 0;
    client->ps.throwBackGrenadeTimeLeft = 0;
    client->ps.throwBackGrenadeOwner = 1023;
    for ( i = 0; i < 15; ++i )
        BG_PlayerSetWeaponQuickReload(&client->ps, client->ps.heldWeapons[i].weapon, 1);
    if ( BG_GetWeaponDef(client->ps.weapon)->bDualWield )
        PM_StartWeaponAnim(&client->ps, 1, 1);
    PM_StartWeaponAnim(&client->ps, 1, 0);
    G_SetOrigin(ent, spawn_origin);
    client->ps.origin[0] = *spawn_origin;
    client->ps.origin[1] = spawn_origin[1];
    client->ps.origin[2] = spawn_origin[2];
    client->ps.pm_flags |= 0x400u;
    v8 = spawn_angles[1];
    clean_spawn_angles[0] = *spawn_angles;
    clean_spawn_angles[1] = v8;
    clean_spawn_angles[2] = 0.0f;
    SetClientViewAngle(ent, clean_spawn_angles);
    client->inactivityTime = level.time + 1000 * g_inactivity->current.integer;
    for ( j = 0; j < 2; ++j )
    {
        if ( client->latched_button_bits.array[j] )
        {
            v6 = 1;
            goto LABEL_41;
        }
    }
    v6 = 0;
LABEL_41:
    if ( v6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp",
                    708,
                    0,
                    "%s",
                    "!client->latched_button_bits.anyBitsOn()") )
    {
        __debugbreak();
    }
    for ( k = 0; k < 2; ++k )
    {
        if ( client->button_bitsSinceLastFrame.array[k] )
        {
            v4 = 1;
            goto LABEL_50;
        }
    }
    v4 = 0;
LABEL_50:
    if ( v4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp",
                    709,
                    0,
                    "%s",
                    "!client->button_bitsSinceLastFrame.anyBitsOn()") )
    {
        __debugbreak();
    }
    //bitarray<51>::resetBit(&client->sess.cmd.button_bits, 8u);
    client->sess.cmd.button_bits.resetBit(8);
    //bitarray<51>::resetBit(&client->sess.cmd.button_bits, 9u);
    client->sess.cmd.button_bits.resetBit(9);
    for ( m = 0; m < 2; ++m )
        client->button_bits.array[m] = client->sess.cmd.button_bits.array[m];
    level.clientIsSpawning = 1;
    client->lastSpawnTime = level.time;
    client->sess.cmd.serverTime = level.time;
    client->ps.commandTime = level.time - 100;
    ClientEndFrame(ent);
    ClientThink_real(ent, &client->sess.cmd);
    level.clientIsSpawning = 0;
    BG_PlayerStateToEntityState(&client->ps, &ent->s, 1, 1u);
}

//clientSession_t *__thiscall clientSession_t::clientSession_t(clientSession_t *this)
//{
//    int j; // [esp+Ch] [ebp-10h]
//    int i; // [esp+18h] [ebp-4h]
//
//    for ( i = 0; i < 2; ++i )
//        this->cmd.button_bits.array[i] = 0;
//    for ( j = 0; j < 2; ++j )
//        this->oldcmd.button_bits.array[j] = 0;
//    return this;
//}

void __cdecl ClientDisconnect(unsigned int clientNum)
{
    int j; // [esp+4h] [ebp-10h]
    gclient_s *client; // [esp+8h] [ebp-Ch]
    gentity_s *ent; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    if ( clientNum >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp",
                    754,
                    0,
                    "clientNum doesn't index level.maxclients\n\t%i not in [0, %i)",
                    clientNum,
                    level.maxclients) )
    {
        __debugbreak();
    }
    client = &level.clients[clientNum];
    ent = &g_entities[clientNum];
    if ( !ent->r.inuse
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp", 759, 0, "%s", "ent->r.inuse") )
    {
        __debugbreak();
    }
    if ( Scr_IsSystemActive(1u, SCRIPTINSTANCE_SERVER) )
    {
        Scr_AddString("disconnect", SCRIPTINSTANCE_SERVER);
        Scr_AddString("-1", SCRIPTINSTANCE_SERVER);
        Scr_Notify(ent, scr_const.menuresponse, 2u);
    }
    for ( i = 0; i < level.maxclients; ++i )
    {
        if ( level.clients[i].sess.connected
            && level.clients[i].sess.sessionState == SESS_STATE_SPECTATOR
            && level.clients[i].spectatorClient == clientNum
            && level.clients[i].sess.archiveTime <= 0 )
        {
            StopFollowing(&g_entities[i]);
            level.clients[i].sess.archiveTime = 0;
        }
    }
    if ( client->sess.sessionState == SESS_STATE_SPECTATOR )
        StopFollowing(ent);
    if ( (client->ps.eFlags & 0x4000) != 0 )
        VEH_UnlinkPlayer(ent, 0, (char*)"ClientDisconnect");
    if ( ent->client != client
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp",
                    791,
                    0,
                    "%s",
                    "ent->client == client") )
    {
        __debugbreak();
    }
    HudElem_ClientDisconnect(ent);
    if ( Scr_IsSystemActive(1u, SCRIPTINSTANCE_SERVER) )
        Scr_PlayerDisconnect(ent);
    for ( j = 0; j < 1024; ++j )
    {
        if ( ((g_entities[j].s.renderOptions.i >> 21) & 0x1F) - 1 == ent->s.number )
            g_entities[j].s.renderOptions.i = g_entities[j].s.renderOptions.i & 0xFC1FFFFF | 0x200000;
    }
    G_FreeEntity(ent);
    ent->client = client;
    ClientClearFields(client);
    client->sess.connected = CON_DISCONNECTED;
    memset(&client->sess.cs, 0, sizeof(client->sess.cs));
    CalculateRanks();
    if ( ent->client != client
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_client_mp.cpp",
                    818,
                    0,
                    "%s",
                    "ent->client == client") )
    {
        __debugbreak();
    }
}

unsigned int __cdecl G_GetNonPVSPlayerInfo(gentity_s *pSelf, float *vPosition, int iLastUpdateEnt)
{
    bool v4; // [esp+0h] [ebp-3Ch]
    team_t team; // [esp+10h] [ebp-2Ch]
    float fScale; // [esp+18h] [ebp-24h]
    float fScale_4; // [esp+1Ch] [ebp-20h]
    int iPos; // [esp+20h] [ebp-1Ch]
    int iPos_4; // [esp+24h] [ebp-18h]
    int iEntCount; // [esp+28h] [ebp-14h]
    gentity_s *pEnt; // [esp+2Ch] [ebp-10h]
    int iBaseEnt; // [esp+38h] [ebp-4h]

    team = pSelf->client->sess.cs.team;
    if ( team == TEAM_SPECTATOR )
        return 0;
    v4 = g_compassShowEnemies->current.enabled
        || level.teamHasSatellite[team]
        || pSelf->client->hasSatellite
        || level.teamHasSpyplane[team]
        || pSelf->client->hasSpyplane;
    if ( team == TEAM_FREE && !v4 )
        return 0;
    if ( iLastUpdateEnt == 1023 )
        iBaseEnt = 0;
    else
        iBaseEnt = iLastUpdateEnt + 1;
    for ( iEntCount = 0; ; ++iEntCount )
    {
        if ( iEntCount >= com_maxclients->current.integer )
            return 0;
        pEnt = &g_entities[(iEntCount + iBaseEnt) % 32];
        if ( pEnt->r.inuse
            && pEnt->client
            && pEnt->client->sess.sessionState == SESS_STATE_PLAYING
            && (pEnt->client->sess.cs.team == team || v4)
            && pSelf != pEnt
            && !SV_inSnapshot(vPosition, pEnt->s.number) )
        {
            break;
        }
    }
    iPos = (int)(float)((float)(pEnt->r.currentOrigin[0] - *vPosition) + 0.5);
    iPos_4 = (int)(float)((float)(pEnt->r.currentOrigin[1] - vPosition[1]) + 0.5);
    fScale = 1.0f;
    fScale_4 = 1.0f;
    if ( iPos <= 1024 )
    {
        if ( iPos < -1022 )
            fScale = -1022.0 / (float)iPos;
    }
    else
    {
        fScale = 1024.0 / (float)iPos;
    }
    if ( iPos_4 <= 1024 )
    {
        if ( iPos_4 < -1022 )
            fScale_4 = -1022.0 / (float)iPos_4;
    }
    else
    {
        fScale_4 = 1024.0 / (float)iPos_4;
    }
    if ( fScale < 1.0 || fScale_4 < 1.0 )
    {
        if ( fScale_4 <= fScale )
        {
            if ( fScale > fScale_4 )
                iPos = (int)(float)((float)iPos * fScale_4);
        }
        else
        {
            iPos_4 = (int)(float)((float)iPos_4 * fScale);
        }
    }
    if ( iPos <= 1024 )
    {
        if ( iPos < -1022 )
            iPos = -1022;
    }
    else
    {
        iPos = 1024;
    }
    if ( iPos_4 <= 1024 )
    {
        if ( iPos_4 < -1022 )
            iPos_4 = -1022;
    }
    else
    {
        iPos_4 = 1024;
    }
    return ((unsigned __int8)(int)(float)(pEnt->r.currentAngles[1] * 0.71111113) << 24)
        | ((((unsigned __int16)((iPos_4 + 2) / 4) + 255) & 0x1FF) << 15) & 0xFFFFFF
        | ((((unsigned __int16)((iPos + 2) / 4) + 255) & 0x1FF) << 6) & 0x7FFF
        | pEnt->s.number & 0x3F;
}

void __cdecl G_BroadcastVoice(gentity_s *talker, VoicePacket_t *voicePacket)
{
    gclient_s *client; // [esp+0h] [ebp-Ch]
    signed int otherPlayer; // [esp+4h] [ebp-8h]
    gentity_s *ent; // [esp+8h] [ebp-4h]

    talker->client->lastVoiceTime = level.time;
    for ( otherPlayer = 0; otherPlayer < com_maxclients->current.integer; ++otherPlayer )
    {
        ent = &g_entities[otherPlayer];
        client = ent->client;
        if ( ent->r.inuse
            && client
            && (client->sess.sessionState == SESS_STATE_INTERMISSION
             || voice_global->current.enabled
             || OnSameTeam(talker, ent)
             || talker->client->sess.cs.team == TEAM_FREE)
            && (ent->client->sess.sessionState == talker->client->sess.sessionState
             || (ent->client->sess.sessionState == SESS_STATE_DEAD || talker->client->sess.sessionState == SESS_STATE_DEAD)
             && voice_deadChat->current.enabled)
            && (talker != ent || voice_localEcho->current.enabled)
            && !SV_ClientHasClientMuted(otherPlayer, talker->s.number)
            && SV_ClientWantsVoiceData(otherPlayer) )
        {
            SV_QueueVoicePacket(talker->s.number, otherPlayer, voicePacket);
        }
    }
}

