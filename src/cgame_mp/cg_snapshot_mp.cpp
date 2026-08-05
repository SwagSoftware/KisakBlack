#include "cg_snapshot_mp.h"
#include "cg_main_mp.h"
#include <cgame/cg_scr_main.h>
#include <clientscript/scr_const.h>
#include <client/splitscreen.h>
#include "cg_ents_mp.h"
#include <clientscript/cscr_vm.h>
#include <cgame/cg_event.h>
#include <ragdoll/ragdoll.h>
#include <cgame/cg_colltree.h>
#include <cgame/cg_world.h>
#include "cg_animtree_mp.h"
#include <clientscript/cscr_memorytree.h>
#include <physics/phys_auto_rigid_body.h>
#include <gfx_d3d/r_shader_constant_set.h>
#include <gfx_d3d/r_dpvs.h>
#include <universal/com_math_anglevectors.h>
#include <sound/snd_public_async.h>
#include <qcommon/dobj_management.h>
#include <cgame/cg_spawn.h>
#include <demo/demo_playback.h>
#include "cg_servercmds_mp.h"
#include <live/live_pcache.h>
#include <client_mp/cl_cgame_mp.h>
#include <EffectsCore/fx_marks.h>
#include <EffectsCore/fx_unique_handle.h>
#include <EffectsCore/fx_system.h>
#include "cg_players_mp.h"
#include "cg_scr_main_mp.h"
#include "cg_predict_mp.h"
#include "cg_scoreboard_mp.h"
#include "cg_animscripted_mp.h"
#include "cg_actors_mp.h"
#include <aim_assist/aim_assist.h>
#include <cgame/cg_vehicle.h>
#include <cgame/offhandweapons.h>
#include <bgame/bg_dog_animations_mp.h>
#include "cg_vehicles_mp.h"
#include <client/cl_rank.h>
#include "cg_draw_net_mp.h"
#include <gfx_d3d/r_scene.h>

void __cdecl CG_ShutdownEntity(int localClientNum, centity_s *cent, bool shutdown_script_for_local_client)
{
    unsigned int v3; // [esp+0h] [ebp-20h]
    unsigned int number; // [esp+Ch] [ebp-14h]
    bool v5; // [esp+1Bh] [ebp-5h]
    unsigned __int16 t; // [esp+1Ch] [ebp-4h]

    if ( shutdown_script_for_local_client || CG_GetClientNumForLocalClient(localClientNum) != cent->nextState.number )
    {
        CScr_Notify(localClientNum, cent, cscr_const.entityshutdown, 0);
        if ( CL_LocalClient_IsFirstActive(localClientNum) )
        {
            if ( cent )
                v5 = ((*((unsigned int *)cent + 201) >> 8) & 1) != 0;
            else
                v5 = 0;
        }
        else
        {
            v5 = 0;
        }
        if ( v5 )
        {
            *((unsigned int *)cent + 201) &= ~0x100u;
            CScr_AddEntity(cent, (unsigned __int16)localClientNum);
            Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
            t = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_data.entityshutdownCB, 2u);
            Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
        }
        CScr_FreeEntity(cent, (unsigned __int16)localClientNum);
    }
    if ( cent->pose.isRagdoll || Com_IsRagdollTrajectory(&cent->currentState.pos) )
    {
        cent->pose.isRagdoll = 0;
        if ( cent->pose.ragdollHandle > 0 )
        {
            Ragdoll_Remove(cent->pose.ragdollHandle);
            cent->pose.ragdollHandle = 0;
        }
        if ( cent->pose.killcamRagdollHandle > 0 )
        {
            Ragdoll_Remove(cent->pose.killcamRagdollHandle);
            cent->pose.killcamRagdollHandle = 0;
        }
        cent->currentState.pos.trType = 0;
        cent->currentState.apos.trType = 0;
    }
    if ( cent->nextState.number >= 1024 && !cent->pose.eType && ((*((unsigned int *)cent + 201) >> 15) & 1) != 0 )
    {
        number = cent->nextState.number;
        if ( localClientNum
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_world.h",
                        17,
                        0,
                        "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        localClientNum,
                        1) )
        {
            __debugbreak();
        }
        if ( number >= 0x600
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_world.h",
                        18,
                        0,
                        "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                        number,
                        1536) )
        {
            __debugbreak();
        }
        if ( localClientNum
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_colltree.h",
                        108,
                        0,
                        "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        localClientNum,
                        1) )
        {
            __debugbreak();
        }
        if ( number >= 0x600
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_colltree.h",
                        109,
                        0,
                        "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                        number,
                        1536) )
        {
            __debugbreak();
        }
        if ( cgEntCollNodes[localClientNum][number].sector )
            CG_UnlinkEntity(localClientNum, cent->nextState.number);
    }
    if ( cent->pose.physObjId && cent->pose.physObjId != -1 )
    {
        v3 = cent->nextState.number;
        if ( localClientNum
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_world.h",
                        17,
                        0,
                        "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        localClientNum,
                        1) )
        {
            __debugbreak();
        }
        if ( v3 >= 0x600
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_world.h",
                        18,
                        0,
                        "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                        v3,
                        1536) )
        {
            __debugbreak();
        }
        if ( localClientNum
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_colltree.h",
                        108,
                        0,
                        "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        localClientNum,
                        1) )
        {
            __debugbreak();
        }
        if ( v3 >= 0x600
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_colltree.h",
                        109,
                        0,
                        "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                        v3,
                        1536) )
        {
            __debugbreak();
        }
        if ( cgEntCollNodes[localClientNum][v3].sector )
            CG_UnlinkEntity(localClientNum, cent->nextState.number);
        Phys_ObjDestroy(0, cent->pose.physObjId);
        cent->currentState.pos.trType = 0;
        cent->currentState.apos.trType = 0;
    }
    if ( cent->pose.physUserBody )
        Phys_ObjDestroy(0, cent->pose.physUserBody);
    if ( cent->pose.physUserBodyProneFeet )
        Phys_ObjDestroy(0, cent->pose.physUserBodyProneFeet);
    cent->pose.physObjId = 0;
    cent->pose.physUserBody = 0;
    cent->pose.physUserBodyProneFeet = 0;
    cent->pose.startBurnTime = 0;
    *((unsigned int *)cent + 201) &= ~0x20u;
    if ( cent->destructible )
    {
        CG_FreeDestructible(localClientNum, cent->destructible);
        cent->destructible = 0;
        cent->pose.destructiblePose = 0;
        CG_SafeDObjFree(localClientNum, cent->nextState.number);
    }
    if ( cent->nitrousVeh )
    {
        NitrousVehicle::remove_vehicle(cent->nitrousVeh);
        cent->nitrousVeh = 0;
    }
    if ( cent->cScriptMover )
    {
        MT_Free((unsigned char *)cent->cScriptMover, 96, SCRIPTINSTANCE_SERVER);
        cent->cScriptMover = 0;
    }
    if ( cent->vehicle )
    {
        if ( cent->vehicle->vehicle_cache )
        {
            MT_Free((unsigned char *)cent->vehicle->vehicle_cache, 1928, SCRIPTINSTANCE_SERVER);
            cent->vehicle->vehicle_cache = 0;
        }
        MT_Free((unsigned char *)cent->vehicle, 84, SCRIPTINSTANCE_SERVER);
        cent->vehicle = 0;
    }
    auto_rigid_body::remove_ent(cent);
    R_InitShaderConstantSet(&cent->pose.constantSet);
}

void __cdecl CG_ShutdownEntities(int localClientNum)
{
    centity_s *cent; // [esp+8h] [ebp-8h]
    int j; // [esp+Ch] [ebp-4h]

    for ( j = 0; j < 1024; ++j )
    {
        cent = CG_GetEntity(localClientNum, j);
        CG_ShutdownEntity(localClientNum, cent, 1);
    }
}

void __cdecl CG_SetInitialSnapshot(int localClientNum, snapshot_s *snap)
{
    cg_s *cgameGlob; // [esp+Ch] [ebp-38h]
    float clientViewAxis[3][3]; // [esp+10h] [ebp-34h] BYREF
    float clientViewOrigin[3]; // [esp+34h] [ebp-10h] BYREF
    int i; // [esp+40h] [ebp-4h]

    R_InitSceneData(localClientNum);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    memset(cgameGlob->centInNextSnapshot, 0, sizeof(cgameGlob->centInNextSnapshot));
    CG_SetNextSnap(localClientNum, 0);
    if ( cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                    471,
                    0,
                    "%s",
                    "!cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    cgameGlob->snap = snap;
    cgameGlob->nextSnap = snap;
    cgameGlob->time = snap->serverTime;
    cgameGlob->bgs.time = cgameGlob->time;
    cgameGlob->oldTime = cgameGlob->time;
    clientViewOrigin[0] = snap->ps.origin[0];
    clientViewOrigin[1] = snap->ps.origin[1];
    clientViewOrigin[2] = snap->ps.origin[2];
    clientViewOrigin[2] = clientViewOrigin[2] + snap->ps.viewHeightCurrent;
    AnglesToAxis(snap->ps.viewangles, clientViewAxis);
    SND_SetListener(
        localClientNum,
        snap->ps.clientNum,
        cgameGlob->bgs.clientinfo[snap->ps.clientNum].team,
        clientViewOrigin,
        clientViewAxis);
    for ( i = 0; i < 1024; ++i )
    {
        if ( ((*((unsigned int *)CG_GetEntity(localClientNum, i) + 201) >> 1) & 1) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                        493,
                        0,
                        "%s",
                        "!CG_GetEntity( localClientNum, i )->nextValid") )
        {
            __debugbreak();
        }
    }
    SND_FadeIn();
    CG_Respawn(localClientNum, 0);
    CG_RestartSmokeGrenades(localClientNum);
    CG_RestartDestructibles(localClientNum);
    CG_ClearEntityCollWorld(localClientNum);
    CG_InitView(localClientNum);
    cgameGlob->nextSnap = 0;
}

void __cdecl CG_TransitionSnapshot(int localClientNum)
{
    fake_centity_s *fcent; // [esp+1Ch] [ebp-20h]
    cg_s *cgameGlob; // [esp+20h] [ebp-1Ch]
    centity_s *cent; // [esp+24h] [ebp-18h]
    centity_s *centa; // [esp+24h] [ebp-18h]
    XAnimTree_s *pXAnimTree; // [esp+28h] [ebp-14h]
    clientState_s *clientState; // [esp+2Ch] [ebp-10h]
    clientInfo_t *ci; // [esp+30h] [ebp-Ch]
    int i; // [esp+34h] [ebp-8h]
    signed int ia; // [esp+34h] [ebp-8h]
    int ib; // [esp+34h] [ebp-8h]
    int ic; // [esp+34h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->snap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                    799,
                    0,
                    "%s",
                    "cgameGlob->snap") )
    {
        __debugbreak();
    }
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                    800,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    for ( i = 0; i < cgameGlob->snap->numClients; ++i )
    {
        clientState = &cgameGlob->snap->clients[i];
        if ( clientState->clientIndex >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                        805,
                        0,
                        "clientState->clientIndex doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        clientState->clientIndex,
                        32) )
        {
            __debugbreak();
        }
        ci = &cgameGlob->bgs.clientinfo[clientState->clientIndex];
        if ( cgameGlob->bgs.clientinfo[clientState->clientIndex].nextValid )
        {
            cgameGlob->bgs.clientinfo[clientState->clientIndex].nextValid = 0;
        }
        else
        {
            pXAnimTree = cgameGlob->bgs.clientinfo[clientState->clientIndex].pXAnimTree;
            memset(ci, 0, sizeof(clientInfo_t));
            ci->pXAnimTree = pXAnimTree;
            XAnimClearTree(ci->pXAnimTree);
            CG_SafeDObjFree(localClientNum, clientState->clientIndex);
        }
    }
    for ( ia = 0; ia < com_maxclients->current.integer; ++ia )
    {
        if ( !cgameGlob->bgs.clientinfo[ia].infoValid
            && Com_GetClientDObj(ia, localClientNum)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                        825,
                        0,
                        "%s",
                        "!Com_GetClientDObj( i, localClientNum )") )
        {
            __debugbreak();
        }
    }
    for ( ib = 0; ib < 512; ++ib )
    {
        fcent = &cg_fakeEntitiesArray[512 * localClientNum + ib];
        if ( cg_fakeEntitiesInuseArray[512 * localClientNum + fcent - &cg_fakeEntitiesArray[512 * localClientNum]]
            && (fcent->flags & 4) != 0 )
        {
            CG_Free(localClientNum, fcent->cent.nextState.number);
        }
    }
    cgameGlob->snap = cgameGlob->nextSnap;
    if ( (cgameGlob->nextSnap->ps.otherFlags & 6) != 0 )
    {
        cent = CG_GetEntity(localClientNum, cgameGlob->nextSnap->ps.clientNum);
        memcpy(&cent->currentState, &cent->nextState.lerp, sizeof(cent->currentState));
    }
    for ( ic = 0; ic < cgameGlob->nextSnap->numEntities; ++ic )
    {
        centa = CG_GetEntity(localClientNum, cgameGlob->nextSnap->entities[ic].number);
        memcpy(&centa->currentState, &centa->nextState.lerp, sizeof(centa->currentState));
    }
}

void __cdecl CG_SetNextSnap(int localClientNum, snapshot_s *snap)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    char *tweenTime; // [esp+0h] [ebp-234h]
    team_t team; // [esp+68h] [ebp-1CCh]
    float *trBase; // [esp+6Ch] [ebp-1C8h]
    float v8; // [esp+74h] [ebp-1C0h]
    float v9; // [esp+78h] [ebp-1BCh]
    float *predictedError; // [esp+BCh] [ebp-178h]
    float *v11; // [esp+C0h] [ebp-174h]
    unsigned __int16 t; // [esp+F4h] [ebp-140h]
    int oldEFlags2; // [esp+F8h] [ebp-13Ch]
    int oldEntNum; // [esp+FCh] [ebp-138h]
    const char *tagName; // [esp+100h] [ebp-134h]
    unsigned int centInPrevSnapshot[32]; // [esp+104h] [ebp-130h] BYREF
    bool bWasDemoJump; // [esp+187h] [ebp-ADh]
    cg_s *cgameGlob; // [esp+188h] [ebp-ACh]
    centity_s *cent; // [esp+18Ch] [ebp-A8h]
    int oldRank; // [esp+190h] [ebp-A4h]
    const char *modelName; // [esp+194h] [ebp-A0h]
    entityState_s *v22; // [esp+198h] [ebp-9Ch]
    clientState_s *clientState; // [esp+19Ch] [ebp-98h]
    clientInfo_t *ci; // [esp+1A0h] [ebp-94h]
    int clientIndex[32]; // [esp+1A4h] [ebp-90h] BYREF
    int num; // [esp+224h] [ebp-10h]
    int entnum; // [esp+228h] [ebp-Ch]
    int i; // [esp+22Ch] [ebp-8h]
    bool bSwitchedDemoPlayers; // [esp+233h] [ebp-1h]

    bWasDemoJump = 0;
    bSwitchedDemoPlayers = 0;
    memset(centInPrevSnapshot, 0, sizeof(centInPrevSnapshot));
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->nextSnap )
    {
        if ( Demo_GetJumpTimeFlag() )
        {
            bWasDemoJump = 1;
            Demo_SetJumpTimeFlag(0);
        }
        if ( Demo_GetSwitchedPlayersFlag() )
        {
            bSwitchedDemoPlayers = 1;
            Demo_SetSwitchedPlayersFlag(0);
        }
        for ( num = 0; num < cgameGlob->nextSnap->numEntities; ++num )
        {
            v22 = &cgameGlob->nextSnap->entities[num];
            entnum = v22->number;
            cent = CG_GetEntity(localClientNum, entnum);
            if ( ((*((unsigned int *)cent + 201) >> 1) & 1) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                            1117,
                            0,
                            "%s",
                            "cent->nextValid") )
            {
                __debugbreak();
            }
            *((unsigned int *)cent + 201) &= ~2u;
            centInPrevSnapshot[entnum >> 5] |= 0x80000000 >> (entnum & 0x1F);
        }
        entnum = cgameGlob->nextSnap->ps.clientNum;
        cent = CG_GetEntity(localClientNum, entnum);
        if ( ((*((unsigned int *)cent + 201) >> 1) & 1) != 0 )
        {
            *((unsigned int *)cent + 201) &= ~2u;
            centInPrevSnapshot[entnum >> 5] |= 0x80000000 >> (entnum & 0x1F);
        }
    }
    cgameGlob->nextSnap = snap;
    if ( snap )
    {
        if ( !cgameGlob->snap
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                        1151,
                        0,
                        "%s",
                        "cgameGlob->snap") )
        {
            __debugbreak();
        }
        CG_UpdateViewOffset(localClientNum);
        CG_SetFrameInterpolation(localClientNum);
        CG_UnpackMatchState(localClientNum, &snap->matchState);
        CG_ExecuteNewServerCommands(localClientNum, snap->serverCommandSequence);
        CG_CheckOpenWaitingScriptMenu(localClientNum);
        memset(clientIndex, 0, sizeof(clientIndex));
        for ( num = 0; num < 32; ++num )
            cgameGlob->scoreOrder[num] = 32;
        for ( num = 0; num < snap->numClients; ++num )
        {
            clientState = &snap->clients[num];
            if ( clientIndex[clientState->clientIndex]
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                            1176,
                            0,
                            "%s",
                            "!clientIndex[clientState->clientIndex]") )
            {
                __debugbreak();
            }
            clientIndex[clientState->clientIndex] = 1;
            if ( clientState->clientIndex >= 0x20u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                            1179,
                            0,
                            "clientState->clientIndex doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                            clientState->clientIndex,
                            32) )
            {
                __debugbreak();
            }
            ci = &cgameGlob->bgs.clientinfo[clientState->clientIndex];
            oldRank = ci->rank;
            if ( ci->infoValid )
                team = ci->team;
            else
                team = clientState->team;
            ci->oldteam = team;
            ci->infoValid = 1;
            ci->nextValid = 1;
            ci->clientNum = clientState->clientIndex;
            ci->team = clientState->team;
            ci->ffaTeam = clientState->ffaTeam;
            ci->lastStandStartTime = clientState->lastStandStartTime;
            ci->needsRevive = clientState->needsRevive;
            ci->rank = clientState->rank;
            ci->prestige = clientState->prestige;
            *(_QWORD *)ci->perks = *(_QWORD *)clientState->perks;
            ci->xuid = clientState->xuid;
            if ( oldRank != ci->rank )
                PCache_InvalidatePlayerEmblem(-1, ci->xuid);
            PCache_GetPlayerEmblem(-1, ci->xuid);
            I_strncpyz(ci->clanAbbrev, clientState->clanAbbrev, 8);
            ci->attachedVehEntNum = clientState->attachedVehEntNum;
            ci->attachedVehSeat = clientState->attachedVehSeat;
            if ( !ci->attachedVehEntNum
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                            1210,
                            0,
                            "%s",
                            "ci->attachedVehEntNum != 0") )
            {
                __debugbreak();
            }
            if ( strcmp(ci->name, clientState->name) )
            {
                if ( ci->name[0] )
                {
                    tweenTime = clientState->name;
                    v2 = UI_SafeTranslateString("CGAME_PLAYERRENAMES");
                    v3 = va("%s^7 %s %s", ci->name, v2, tweenTime);
                    CG_GameMessage(localClientNum, v3);
                }
                I_strncpyz(ci->name, clientState->name, 32);
            }
            modelName = CL_GetConfigString(clientState->modelindex + 1568);
            if ( strcmp(ci->model, modelName) )
            {
                I_strncpyz(ci->model, modelName, 64);
                ci->dobjDirty = 1;
            }
            for ( i = 0; i < 6; ++i )
            {
                modelName = CL_GetConfigString(clientState->attachModelIndex[i] + 1568);
                if ( strcmp(ci->attachModelNames[i], modelName) )
                {
                    I_strncpyz(ci->attachModelNames[i], modelName, 64);
                    ci->dobjDirty = 1;
                }
                tagName = CL_GetConfigString(clientState->attachTagIndex[i] + 3115);
                if ( strcmp(ci->attachTagNames[i], tagName) )
                {
                    I_strncpyz(ci->attachTagNames[i], tagName, 64);
                    ci->dobjDirty = 1;
                }
            }
        }
        cgameGlob->identifyClientNum = snap->ps.stats[3];
        entnum = snap->ps.clientNum;
        cgameGlob->playerTeleported = 0;
        if ( (snap->ps.otherFlags & 6) != 0 )
        {
            cent = CG_GetEntity(localClientNum, entnum);
            cent->nextState.number = (unsigned __int16)entnum;
            BG_PlayerStateToEntityState(&snap->ps, &cent->nextState, 0, 0);
            *((unsigned int *)cent + 201) |= 2u;
            if ( (centInPrevSnapshot[entnum >> 5] & (0x80000000 >> (entnum & 0x1F))) != 0
                && cent->nextState.lerp.useCount == cent->currentState.useCount )
            {
                CG_UpdateClientFlags(localClientNum, cent, cent->currentState.eFlags2, 0);
            }
            else
            {
                CG_UpdateClientFlags(localClientNum, cent, cent->currentState.eFlags2, 1);
            }
            if ( Com_IsRagdollTrajectory(&cent->nextState.lerp.pos) )
            {
                if ( bWasDemoJump )
                {
                    if ( (centInPrevSnapshot[entnum >> 5] & (0x80000000 >> (entnum & 0x1F))) != 0 )
                    {
                        if ( Com_IsRagdollTrajectory(&cent->currentState.pos) )
                        {
                            if ( Vec3Distance(cent->currentState.pos.trBase, cent->nextState.lerp.pos.trBase) > 128.0 )
                                *((unsigned int *)cent + 201) |= 0x200000u;
                        }
                        else
                        {
                            *((unsigned int *)cent + 201) |= 0x200000u;
                        }
                    }
                    else
                    {
                        *((unsigned int *)cent + 201) |= 0x200000u;
                    }
                }
            }
            else if ( ((*((unsigned int *)cent + 201) >> 21) & 1) != 0 )
            {
                *((unsigned int *)cent + 201) &= ~0x200000u;
            }
            if ( (snap->ps.otherFlags & 2) != 0 )
            {
                if ( ((cgameGlob->snap->ps.eFlags ^ snap->ps.eFlags) & 0x4000) != 0 )
                {
                    CL_SetViewAngles(localClientNum, vec3_origin);
                    Com_Printf(0, "CL_SetViewAngles() - SPARTAN\n");
                }
                else if ( snap->ps.vehiclePos != cgameGlob->snap->ps.vehiclePos && (snap->ps.eFlags & 0x4000) != 0 )
                {
                    CL_SetViewAngles(localClientNum, vec3_origin);
                    CG_StartCameraTween(localClientNum, 0.60000002);
                    Com_Printf(0, "CL_SetViewAngles() - CASSIO\n");
                }
            }
            if ( ((cgameGlob->snap->ps.eFlags ^ snap->ps.eFlags) & 0x4000) != 0 && (snap->ps.eFlags & 0x4000) != 0 )
                CG_MenuShowNotify(localClientNum, 8);
            if ( cgameGlob->mapRestart
                || CG_LocalPlayerRespawned(localClientNum, snap)
                || entnum != cgameGlob->snap->ps.clientNum
                || bWasDemoJump )
            {
                cgameGlob->playerTeleported = 1;
                cgameGlob->flareRatioLastFrame = 0.0f;
                centInPrevSnapshot[(int)cgameGlob->snap->ps.clientNum >> 5] &= ~(0x80000000 >> (cgameGlob->snap->ps.clientNum
                                                                                                                                                                            & 0x1F));
                oldEntNum = cgameGlob->snap->ps.clientNum;
                memcpy((unsigned __int8 *)&cgameGlob->snap->ps, (unsigned __int8 *)&snap->ps, sizeof(cgameGlob->snap->ps));
                CG_ResetEntity(localClientNum, cent, 1);
                if ( !cgameGlob->mapRestart && (snap->ps.otherFlags & 2) != 0 || bWasDemoJump )
                    CG_Respawn(localClientNum, 1);
                else
                    CG_Respawn(localClientNum, 0);
                if ( entnum != oldEntNum )
                    CG_ExtractTransPlayerState(&snap->ps, &cgameGlob->oldTransPlayerState);
            }
            else if ( (centInPrevSnapshot[entnum >> 5] & (0x80000000 >> (entnum & 0x1F))) != 0 && !cgameGlob->playerTeleported )
            {
                if ( ((cent->nextState.lerp.eFlags ^ cent->currentState.eFlags) & 2) != 0 )
                {
                    memcpy((unsigned __int8 *)&cgameGlob->snap->ps, (unsigned __int8 *)&snap->ps, sizeof(cgameGlob->snap->ps));
                    CG_ResetEntity(localClientNum, cent, 0);
                    predictedError = cgameGlob->predictedError;
                    cgameGlob->predictedError[0] = 0.0f;
                    predictedError[1] = 0.0f;
                    predictedError[2] = 0.0f;
                }
            }
            else
            {
                memcpy((unsigned __int8 *)&cgameGlob->snap->ps, (unsigned __int8 *)&snap->ps, sizeof(cgameGlob->snap->ps));
                CG_ResetEntity(localClientNum, cent, 1);
                v11 = cgameGlob->predictedError;
                cgameGlob->predictedError[0] = 0.0f;
                v11[1] = 0.0f;
                v11[2] = 0.0f;
            }
        }
        else if ( snap->ps.pm_type == 4 )
        {
            cent = CG_GetEntity(localClientNum, entnum);
            centInPrevSnapshot[(int)cgameGlob->snap->ps.clientNum >> 5] &= ~(0x80000000 >> (cgameGlob->snap->ps.clientNum
                                                                                                                                                                        & 0x1F));
            CG_ResetEntity(localClientNum, cent, 1);
            *((unsigned int *)cent + 201) &= ~0x200000u;
        }
        else if ( cgameGlob->mapRestart
                     || (!Demo_IsPlaying() || snap->ps.stats[4] == 14) && snap->ps.stats[4] != cgameGlob->snap->ps.stats[4]
                     || entnum != cgameGlob->snap->ps.clientNum
                     || bWasDemoJump )
        {
            memcpy((unsigned __int8 *)&cgameGlob->snap->ps, (unsigned __int8 *)&snap->ps, sizeof(cgameGlob->snap->ps));
            CG_Respawn(localClientNum, cgameGlob->mapRestart == 0);
        }
        for ( num = 0; num < snap->numEntities; ++num )
        {
            v22 = &snap->entities[num];
            entnum = v22->number;
            cent = CG_GetEntity(localClientNum, entnum);
            memcpy(&cent->nextState, v22, sizeof(cent->nextState));
            if ( !Demo_IsPlaying() && ((*((unsigned int *)cent + 201) >> 1) & 1) != 0 )
            {
                v4 = va(
                             "entnum %d num %d numEntities %d clientNum %d flags 0x%x",
                             entnum,
                             num,
                             snap->numEntities,
                             snap->ps.clientNum,
                             snap->ps.pm_flags);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                                1366,
                                0,
                                "%s\n\t%s",
                                "!cent->nextValid",
                                v4) )
                    __debugbreak();
            }
            *((unsigned int *)cent + 201) |= 2u;
            if ( Com_IsRagdollTrajectory(&cent->nextState.lerp.pos) )
            {
                if ( bWasDemoJump )
                {
                    if ( (centInPrevSnapshot[entnum >> 5] & (0x80000000 >> (entnum & 0x1F))) != 0 )
                    {
                        if ( Com_IsRagdollTrajectory(&cent->currentState.pos) )
                        {
                            if ( Vec3Distance(cent->currentState.pos.trBase, cent->nextState.lerp.pos.trBase) > 128.0 )
                                *((unsigned int *)cent + 201) |= 0x200000u;
                        }
                        else
                        {
                            *((unsigned int *)cent + 201) |= 0x200000u;
                        }
                    }
                    else
                    {
                        *((unsigned int *)cent + 201) |= 0x200000u;
                    }
                }
            }
            else if ( ((*((unsigned int *)cent + 201) >> 21) & 1) != 0 )
            {
                *((unsigned int *)cent + 201) &= ~0x200000u;
            }
            oldEFlags2 = cent->currentState.eFlags2;
            if ( (centInPrevSnapshot[entnum >> 5] & (0x80000000 >> (entnum & 0x1F))) != 0 )
            {
                if ( bWasDemoJump
                    && (cent->currentState.eFlags & 0x40000) != 0
                    && !cent->pose.isRagdoll
                    && !Com_IsRagdollTrajectory(&cent->currentState.pos)
                    || ((v22->lerp.eFlags ^ cent->currentState.eFlags) & 2) != 0 )
                {
                    CG_ResetEntity(localClientNum, cent, 0);
                }
            }
            else
            {
                CG_ResetEntity(localClientNum, cent, 1);
            }
            if ( (centInPrevSnapshot[entnum >> 5] & (0x80000000 >> (entnum & 0x1F))) != 0
                && cent->nextState.lerp.useCount == cent->currentState.useCount )
            {
                CG_UpdateClientFlags(localClientNum, cent, oldEFlags2, 0);
            }
            else
            {
                CG_UpdateClientFlags(localClientNum, cent, oldEFlags2, 1);
            }
            centInPrevSnapshot[entnum >> 5] &= ~(0x80000000 >> (entnum & 0x1F));
            cgameGlob->centInNextSnapshot[entnum >> 5] |= 0x80000000 >> (entnum & 0x1F);
        }
        for ( num = 0; num < cgameGlob->snap->numEntities; ++num )
        {
            v22 = &cgameGlob->snap->entities[num];
            entnum = v22->number;
            if ( (centInPrevSnapshot[entnum >> 5] & (0x80000000 >> (entnum & 0x1F))) != 0 )
            {
                cgameGlob->centInNextSnapshot[entnum >> 5] &= ~(0x80000000 >> (entnum & 0x1F));
                R_UnlinkEntity(localClientNum, entnum);
                CG_UnlinkEntity(localClientNum, entnum);
                FX_MarkEntDetachAll(localClientNum, entnum);
                cent = CG_GetEntity(localClientNum, entnum);
                CG_ShutdownEntity(localClientNum, cent, 1);
                if ( cent->nextState.eType == 8 || cent->nextState.eType == 9 )
                {
                    if ( UniqueHandleToEffect(localClientNum, cent->pose.fx.effect) )
                    {
                        if ( cgameGlob->mapRestart
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                                        1436,
                                        0,
                                        "%s",
                                        "!cgameGlob->mapRestart") )
                        {
                            __debugbreak();
                        }
                        FX_ThroughWithEffect(localClientNum, cent->pose.fx.effect, 1);
                        cent->pose.fx.effect = 0;
                        cent->pose.fx.triggerTime = 0;
                    }
                    if ( cent->pose.fx.triggerTime
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                                    1441,
                                    0,
                                    "%s\n\t(cent->pose.fx.triggerTime) = %i",
                                    "(cent->pose.fx.triggerTime == 0)",
                                    cent->pose.fx.triggerTime) )
                    {
                        __debugbreak();
                    }
                }
            }
        }
        for ( num = 0; num < snap->numClients; ++num )
        {
            cent = CG_GetEntity(localClientNum, snap->clients[num].clientIndex);
            if ( bWasDemoJump )
                CG_UpdateWeaponVisibilityImmediate(localClientNum, cent);
            CG_UpdatePlayerDObj(localClientNum, cent);
        }
        if ( bSwitchedDemoPlayers && cg_scr_mp_data.demo_playerSwitch )
        {
            Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
            t = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_mp_data.demo_playerSwitch, 1u);
            Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
        }
        CG_UpdateWeaponViewmodels(localClientNum);
        CG_UpdateHandViewmodels(localClientNum);
        CG_BuildItemList(localClientNum, cgameGlob->nextSnap);
        CG_TransitionKillcam(localClientNum);
        for ( num = 0; num < snap->numClients; ++num )
        {
            clientState = &snap->clients[num];
            ci = &cgameGlob->bgs.clientinfo[clientState->clientIndex];
            CG_CopyScoreFromClientState(localClientNum, ci, clientState);
            cgameGlob->teamPlayers[ci->score.place + 3] = ci->clientNum;
            CG_CopyUIVisibilityBitsFomClientState(localClientNum, ci, clientState);
        }
        CG_UpdateScoreData(localClientNum);
        CG_UpdateMatchScoreboard(localClientNum);
        for ( num = 0; num < snap->numEntities; ++num )
        {
            cent = CG_GetEntity(localClientNum, snap->entities[num].number);
            CG_UpdateEntityLink(localClientNum, cent);
            if ( cent->nextState.eType == 17 )
                CG_ActorProcessSnapshot(localClientNum, cent);
        }
        if ( (snap->ps.otherFlags & 6) != 0 && (snap->ps.eFlags & 0x4000) != 0 )
        {
            cent = CG_GetEntity(localClientNum, snap->ps.clientNum);
            if ( (snap->ps.eFlags2 & 0x10000000) != 0 )
            {
                trBase = cent->nextState.lerp.apos.trBase;
                v8 = snap->ps.linkAngles[1];
                v9 = snap->ps.linkAngles[2];
                cent->nextState.lerp.apos.trBase[0] = snap->ps.linkAngles[0];
                trBase[1] = v8;
                trBase[2] = v9;
            }
        }
        if ( cgameGlob->snap->ps.fWeaponPosFrac == 0.0 && cgameGlob->nextSnap->ps.fWeaponPosFrac != 0.0 )
            cgameGlob->weapIdleTime = cgameGlob->nextSnap->ps.weaponIdleTime;
        cgameGlob->mapRestart = 0;
        for ( i = 0; i < com_maxclients->current.integer; ++i )
        {
            ci = &cgameGlob->bgs.clientinfo[i];
            if ( !ci->infoValid
                && Com_GetClientDObj(i, localClientNum)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                            1539,
                            0,
                            "%s",
                            "!Com_GetClientDObj( i, localClientNum )") )
            {
                __debugbreak();
            }
        }
    }
    else
    {
        if ( cgameGlob->snap )
        {
            for ( num = 0; num < cgameGlob->snap->numEntities; ++num )
            {
                v22 = &cgameGlob->snap->entities[num];
                entnum = v22->number;
                cent = CG_GetEntity(localClientNum, entnum);
                CG_ShutdownEntity(localClientNum, cent, 1);
                CG_ClearUnion(localClientNum, cent);
            }
            cgameGlob->snap = 0;
        }
        CG_ClearItemList();
    }
}

void CG_ResetEntity(int localClientNum, centity_s *cent, int newEntity)
{
    centity_s *Entity; // eax
    float *v5; // [esp+Ch] [ebp-40h]
    actorInfo_t *ai; // [esp+20h] [ebp-2Ch]
    actorInfo_t *aiCorpseInfo; // [esp+24h] [ebp-28h]
    unsigned __int16 t; // [esp+28h] [ebp-24h]
    DObj *obj; // [esp+2Ch] [ebp-20h]
    DObj *obja; // [esp+2Ch] [ebp-20h]
    cg_s *cgameGlob; // [esp+30h] [ebp-1Ch]
    XAnimTree_s *pXAnimTree; // [esp+34h] [ebp-18h]
    XAnimTree_s *pXAnimTreea; // [esp+34h] [ebp-18h]
    cgs_t *cgs; // [esp+38h] [ebp-14h]
    cgs_t *cgsa; // [esp+38h] [ebp-14h]
    unsigned int corpseIndex; // [esp+3Ch] [ebp-10h]
    unsigned int corpseIndexa; // [esp+3Ch] [ebp-10h]
    clientInfo_t *cia; // [esp+40h] [ebp-Ch]
    clientInfo_t *ci; // [esp+40h] [ebp-Ch]
    int i; // [esp+44h] [ebp-8h]
    int ia; // [esp+44h] [ebp-8h]
    clientInfo_t *corpseInfo; // [esp+48h] [ebp-4h]

    if ((cent->currentState.useCount != cent->nextState.lerp.useCount
        || cent->nextState.eType != 14
        && cent->nextState.eType != 1
        && cent->nextState.eType != 2
        && cent->nextState.eType != 17
        && cent->nextState.eType != 19)
        && (!cent->destructible || cent->nitrousVeh))
    {
        CG_ShutdownEntity(localClientNum, cent, 0);
        CG_ClearUnion(localClientNum, cent);
    }
    if (newEntity && cent->nextState.number >= 44)
        CG_SafeDObjFree(localClientNum, cent->nextState.number);
    AimAssist_ClearEntityReference(localClientNum, cent->nextState.number);
    memcpy(&cent->currentState, &cent->nextState.lerp, sizeof(cent->currentState));
    *((_DWORD *)cent + 201) &= ~0x10u;
    *((_DWORD *)cent + 201) &= ~0x800000u;
    cent->pose.cullIn = 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    BG_EvaluateTrajectory(&cent->nextState.lerp.pos, cgameGlob->time, cent->pose.origin);
    BG_EvaluateTrajectory(&cent->nextState.lerp.apos, cgameGlob->time, cent->pose.angles);
    if (cent->pose.localClientNum != localClientNum
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
            250,
            0,
            "%s",
            "cent->pose.localClientNum == localClientNum"))
    {
        __debugbreak();
    }
    cent->pose.eType = cent->nextState.eType;
    if (cent->pose.eType != cent->nextState.eType
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
            253,
            0,
            "%s",
            "cent->pose.eType == cent->nextState.eType"))
    {
        __debugbreak();
    }
    CG_UnlinkEntity(localClientNum, cent->nextState.number);
    R_InitShaderConstantSet(&cent->pose.constantSet);
    switch (cent->nextState.eType)
    {
    case 0:
    case 4:
        if ((cent->nextState.lerp.eFlags & 0x4000) != 0
            && cgameGlob->time - cent->nextState.lerp.u.actor.actorNum > 200)
        {
            cent->previousEventSequence = cent->nextState.eventSequence;
        }
        else
        {
            cent->previousEventSequence = 0;
        }
        goto LABEL_74;
    case 1:
        for (ia = 0; ia < 6; ++ia)
            cent->pose.player.tag[ia] = -2;
        cent->previousEventSequence = cent->nextState.eventSequence;
        if (cent->nextState.clientNum >= 0x20u
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                295,
                0,
                "cent->nextState.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                cent->nextState.clientNum,
                32))
        {
            __debugbreak();
        }
        cia = &cgameGlob->bgs.clientinfo[cent->nextState.clientNum];
        cia->lerpMoveDir = (float)cent->nextState.lerp.u.loopFx.period;
        cia->lerpLean = cent->nextState.lerp.u.turret.gunAngles[0];
        cia->playerAngles[0] = cent->pose.angles[0];
        cia->playerAngles[1] = cent->pose.angles[1];
        cia->playerAngles[2] = cent->pose.angles[2];
        cent->pose.angles[0] = 0.0f;
        cent->pose.angles[2] = 0.0f;
        CG_ResetPlayerEntity(localClientNum, cgameGlob, cent, newEntity);
        obja = Com_GetClientDObj(cent->nextState.number, localClientNum);
        if (obja)
            CG_Player_PreControllers(obja, cent);
        goto LABEL_74;
    case 2:
        corpseIndex = cent->nextState.number - 32;
        if (cent->nextState.clientNum >= 0x20u
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                311,
                0,
                "cent->nextState.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                cent->nextState.clientNum,
                32))
        {
            __debugbreak();
        }
        ci = &cgameGlob->bgs.clientinfo[cent->nextState.clientNum];
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        if (corpseIndex >= 4
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                314,
                0,
                "corpseIndex doesn't index MAX_CLIENT_CORPSES\n\t%i not in [0, %i)",
                corpseIndex,
                4))
        {
            __debugbreak();
        }
        corpseInfo = &cgs->corpseinfo[corpseIndex];
        pXAnimTree = cgs->corpseinfo[corpseIndex].pXAnimTree;
        cent->miscTime = cgameGlob->time;
        if ((cent->nextState.lerp.eFlags & 0x10) != 0)
        {
            *((_DWORD *)cent + 201) |= 0x100u;
            Entity = CG_GetEntity(localClientNum, cent->nextState.clientNum);
            CScr_AddEntity(Entity, localClientNum);
            Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
            t = CScr_ExecEntThread(cent, cg_scr_data.corpse_callback, 2u);
            Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
            CG_CopyCorpseInfo(corpseInfo, ci);
            cgs->corpseinfo[corpseIndex].pXAnimTree = pXAnimTree;
            XAnimCloneAnimTree(ci->pXAnimTree, pXAnimTree);
            cent->previousEventSequence = 0;
        }
        else
        {
            if (!cgs->corpseinfo[corpseIndex].model[0] || cgs->corpseinfo[corpseIndex].clientNum != ci->clientNum)
            {
                CG_CopyCorpseInfo(corpseInfo, ci);
                cgs->corpseinfo[corpseIndex].pXAnimTree = pXAnimTree;
            }
            cent->previousEventSequence = cent->nextState.eventSequence;
        }
        cgs->corpseinfo[corpseIndex].dobjDirty = 1;
        goto LABEL_74;
    case 6:
    case 0xD:
        cent->previousEventSequence = cent->nextState.eventSequence;
        if (cent->nextState.solid != 0xFFFFFF)
            goto LABEL_74;
        CG_UpdateBModelWorldBounds(localClientNum, cent, 1);
        break;
    case 8:
    case 9:
        if (UniqueHandleToEffect(localClientNum, cent->pose.fx.effect)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                364,
                0,
                "%s",
                "!UniqueHandleToEffect(localClientNum,cent->pose.fx.effect)"))
        {
            __debugbreak();
        }
        goto LABEL_74;
    case 0xB:
        cent->previousEventSequence = cent->nextState.eventSequence;
        cent->pose.turret.tag_aim = -2;
        cent->pose.turret.tag_aim_pivot = -2;
        cent->pose.turret.tag_aim_animated = -2;
        cent->pose.turret.tag_flash = -2;
        goto LABEL_74;
    case 0xE:
    case 0x10:
        cent->previousEventSequence = cent->nextState.eventSequence;
        cent->pose.vehicle.tag_body = -2;
        cent->pose.vehicle.tag_turret = -2;
        cent->pose.vehicle.tag_barrel = -2;
        cent->pose.vehicle.tag_barrel_recoil = -2;
        for (i = 0; i < 6; ++i)
            cent->pose.vehicle.wheelBoneIndex[i] = -2;
        obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
        if (obj)
            CG_Vehicle_PreControllers(localClientNum, obj, cent);
        goto LABEL_74;
    case 0x11:
        cent->previousEventSequence = cent->nextState.eventSequence;
        CG_ResetActorEntity(localClientNum, cgameGlob, cent);
        goto LABEL_74;
    case 0x13:
        cent->previousEventSequence = cent->nextState.eventSequence;
        if ((unsigned int)(cent->nextState.number - 36) >= 8
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                387,
                0,
                "cent->nextState.number - ACTOR_CORPSES doesn't index MAX_ACTOR_CORPSES\n\t%i not in [0, %i)",
                cent->nextState.number - 36,
                8))
        {
            __debugbreak();
        }
        corpseIndexa = cent->nextState.number - 36;
        ai = &cgameGlob->bgs.actorinfo[cent->nextState.lerp.u.actor.actorNum];
        cgsa = CG_GetLocalClientStaticGlobals(localClientNum);
        if (corpseIndexa >= 8
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                391,
                0,
                "corpseIndex doesn't index MAX_ACTOR_CORPSES\n\t%i not in [0, %i)",
                corpseIndexa,
                8))
        {
            __debugbreak();
        }
        aiCorpseInfo = &cgsa->actorCorpseInfo[corpseIndexa];
        pXAnimTreea = cgsa->actorCorpseInfo[corpseIndexa].pXAnimTree;
        if ((cent->nextState.lerp.eFlags & 0x10) != 0)
        {
            CG_CopyActorCorpseInfo(aiCorpseInfo, ai);
            cgsa->actorCorpseInfo[corpseIndexa].pXAnimTree = pXAnimTreea;
            XAnimCloneAnimTree(ai->pXAnimTree, pXAnimTreea);
            cent->previousEventSequence = 0;
        }
        else
        {
            if (cgsa->actorCorpseInfo[corpseIndexa].actorNum != ai->actorNum)
            {
                CG_CopyActorCorpseInfo(aiCorpseInfo, ai);
                cgsa->actorCorpseInfo[corpseIndexa].pXAnimTree = pXAnimTreea;
            }
            cent->previousEventSequence = cent->nextState.eventSequence;
        }
        cgsa->actorCorpseInfo[corpseIndexa].dobjDirty = 1;
        goto LABEL_74;
    default:
        cent->previousEventSequence = cent->nextState.eventSequence;
    LABEL_74:
        v5 = cg_entityOriginArray[localClientNum][cent->nextState.number];
        *v5 = 131072.0f;
        v5[1] = 131072.0f;
        v5[2] = 131072.0f;
        break;
    }
}

void __cdecl CG_CopyCorpseInfo(clientInfo_t *corpseInfo, const clientInfo_t *ci)
{
    memcpy(corpseInfo, ci, sizeof(clientInfo_t));
}

void __cdecl CG_CopyActorCorpseInfo(actorInfo_t *corpseInfo, const actorInfo_t *ai)
{
    memcpy(corpseInfo, ai, sizeof(actorInfo_t));
}

void __cdecl CG_TransitionKillcam(int localClientNum)
{
    const vehicle_info_t *VehicleInfo; // eax
    float killcamDist; // [esp+14h] [ebp-78h]
    float killcamZTargetOffset; // [esp+18h] [ebp-74h]
    centity_s *parentCent; // [esp+40h] [ebp-4Ch]
    const WeaponDef *weapDef; // [esp+44h] [ebp-48h]
    const WeaponDef *weapDefa; // [esp+44h] [ebp-48h]
    centity_s *Entity; // [esp+4Ch] [ebp-40h]
    int newKillCamEntity; // [esp+50h] [ebp-3Ch]
    unsigned __int16 v10; // [esp+58h] [ebp-34h]
    unsigned __int16 t; // [esp+5Ch] [ebp-30h]
    bool killCamJustStarted; // [esp+63h] [ebp-29h]
    cg_s *cgameGlob; // [esp+64h] [ebp-28h]
    XAnimTree_s *pXAnimTree; // [esp+68h] [ebp-24h]
    centity_s *cent; // [esp+6Ch] [ebp-20h]
    cgs_t *cgs; // [esp+70h] [ebp-1Ch]
    unsigned int corpseIndex; // [esp+74h] [ebp-18h]
    int anim; // [esp+7Ch] [ebp-10h]
    int i; // [esp+80h] [ebp-Ch]
    XAnim_s *anims; // [esp+84h] [ebp-8h]

    killCamJustStarted = 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgameGlob->wasInKillCam = cgameGlob->inKillCam;
    if ( !cgameGlob->inKillCam )
    {
        if ( cgameGlob->nextSnap->ps.deltaTime )
        {
            cgameGlob->inKillCam = 1;
            killCamJustStarted = 1;
            cgameGlob->nextSnap->ps.weapAnimLeft = 66;
            cgameGlob->killCamFirstFrameRan = 0;
            cgameGlob->killCamLerpEndTime = 0;
            cgameGlob->killCamEntityType = KC_NO_ENTITY;
            cgameGlob->invalidKillCamEntity = 1023;
            cgameGlob->killCamStoppedTime = 0;
            cgameGlob->killCamLookAt[0] = 0.0f;
            cgameGlob->killCamLookAt[1] = 0.0f;
            cgameGlob->killCamLookAt[2] = 0.0f;
            cgameGlob->killCamLastEntityNum = 1023;
            cgameGlob->killCamLastEntityOrg[0] = 0.0f;
            cgameGlob->killCamLastEntityOrg[1] = 0.0f;
            cgameGlob->killCamLastEntityOrg[2] = 0.0f;
            cgameGlob->killCamLastEntityAngles[0] = 0.0f;
            cgameGlob->killCamLastEntityAngles[1] = 0.0f;
            cgameGlob->killCamLastEntityAngles[2] = 0.0f;
            cgameGlob->clearMarks = 1;
            cgameGlob->weapIdleTime = cgameGlob->nextSnap->ps.weaponIdleTime;
            CG_SetEquippedOffHand(localClientNum, 0);
            CG_RestartSmokeGrenades(localClientNum);
            CG_RestartDestructibles(localClientNum);
            if ( cg_scr_mp_data.demo_jump )
            {
                Scr_AddInt(cgameGlob->time, SCRIPTINSTANCE_CLIENT);
                Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
                t = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_mp_data.demo_jump, 2u);
                Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
            }
        }
    }
    if ( cgameGlob->inKillCam && !cgameGlob->nextSnap->ps.deltaTime )
    {
        cgameGlob->inKillCam = 0;
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        CG_RestartSmokeGrenades(localClientNum);
        CG_RestartDestructibles(localClientNum);
        for ( i = 0; i < cgameGlob->nextSnap->numEntities; ++i )
        {
            cent = CG_GetEntity(localClientNum, cgameGlob->nextSnap->entities[i].number);
            if ( cent->nextState.eType == 2 )
            {
                corpseIndex = cent->nextState.number - 32;
                if ( corpseIndex >= 4
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                                603,
                                0,
                                "corpseIndex doesn't index MAX_CLIENT_CORPSES\n\t%i not in [0, %i)",
                                corpseIndex,
                                4) )
                {
                    __debugbreak();
                }
                pXAnimTree = cgs->corpseinfo[corpseIndex].pXAnimTree;
                anim = cgs->corpseinfo[corpseIndex].legs.animationNumber & 0xFFFFFBFF;
                anims = XAnimGetAnims(pXAnimTree);
                if ( anim && !XAnimIsLooped(anims, anim) && !XAnimGetNumChildren(anims, anim) )
                    XAnimSetTime(pXAnimTree, anim, 1.0, 0xFFFFu);
            }
            if ( cent->nextState.eType == ET_ACTOR_CORPSE )
            {
                bcassert(cent->nextState.number - ACTOR_CORPSES, MAX_ACTOR_CORPSES);
                BG_Actor_FastForwardAnimState(
                    localClientNum,
                    &cent->nextState,
                    &cgs->actorCorpseInfo[cent->nextState.number - ACTOR_CORPSES]);
            }
        }
        if ( cg_scr_mp_data.demo_jump )
        {
            Scr_AddInt(cgameGlob->time, SCRIPTINSTANCE_CLIENT);
            Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
            v10 = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_mp_data.demo_jump, 2u);
            Scr_FreeThread(v10, SCRIPTINSTANCE_CLIENT);
        }
    }
    if ( cgameGlob->inKillCam )
    {
        newKillCamEntity = cgameGlob->nextSnap->ps.killCamEntity;
        cgameGlob->killCamLastEntityNum = 1023;
        if ( newKillCamEntity == 1023 )
        {
            cgameGlob->killCamEntityType = KC_NO_ENTITY;
            cgameGlob->invalidKillCamEntity = 1023;
        }
        else if ( newKillCamEntity == cgameGlob->invalidKillCamEntity )
        {
            newKillCamEntity = 1023;
        }
        else
        {
            Entity = CG_GetEntity(localClientNum, newKillCamEntity);
            if ( ((*((unsigned int *)Entity + 201) >> 1) & 1) != 0 )
            {
                cgameGlob->killCamLastEntityNum = newKillCamEntity;
                cgameGlob->invalidKillCamEntity = 1023;
                switch ( Entity->pose.eType )
                {
                    case 0xCu:
                        VehicleInfo = CG_GetVehicleInfo(Entity->nextState.vehicleState.vehicleInfoIndex);
                        cgameGlob->killCamEntityType = KC_HELICOPTER;
                        killcamDist = VehicleInfo->killcamDist;
                        killcamZTargetOffset = VehicleInfo->killcamZTargetOffset;
                        cgameGlob->killCamHelicopterOffset[0] = VehicleInfo->killcamZDist;
                        cgameGlob->killCamHelicopterOffset[1] = killcamDist;
                        cgameGlob->killCamHelicopterOffset[2] = killcamZTargetOffset;
                        break;
                    case 0x11u:
                        cgameGlob->killCamEntityType = KC_DOG;
                        break;
                    case 4u:
                        weapDef = BG_GetWeaponDef(Entity->nextState.weapon);
                        if ( !weapDef
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                                        685,
                                        0,
                                        "%s",
                                        "weapDef") )
                        {
                            __debugbreak();
                        }
                        if ( weapDef->weapClass == WEAPCLASS_ROCKETLAUNCHER )
                        {
                            cgameGlob->killCamEntityType = KC_ROCKET;
                        }
                        else if ( weapDef->weapType == WEAPTYPE_PROJECTILE || weapDef->impactType == IMPACT_TYPE_BLADE )
                        {
                            cgameGlob->killCamEntityType = KC_FAST_EXPLOSIVE;
                        }
                        else
                        {
                            cgameGlob->killCamEntityType = KC_EXPLOSIVE;
                        }
                        weapDefa = BG_GetWeaponDef(Entity->nextState.weapon);
                        if ( !weapDefa
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                                        695,
                                        0,
                                        "%s",
                                        "weapDef") )
                        {
                            __debugbreak();
                        }
                        cgameGlob->killCamEntityRestState = KC_ENT_MOVING;
                        if ( weapDefa->guidedMissileType == MISSILE_GUIDANCE_BALLISTIC && weapDefa->inventoryType )
                        {
                            cgameGlob->killCamEntityType = KC_ARTILLERY;
                        }
                        else if ( Entity->nextState.groundEntityNum != 1023 )
                        {
                            cgameGlob->killCamEntityRestState = KC_ENT_AT_REST;
                            if ( weapDefa->stickiness == WEAPSTICKINESS_ALL || weapDefa->stickiness == WEAPSTICKINESS_ALL_NO_SENTIENTS )
                            {
                                cgameGlob->killCamEntityRestState = KC_ENT_STUCK_WALL;
                            }
                            else if ( weapDefa->stickiness == WEAPSTICKINESS_GROUND
                                         || weapDefa->stickiness == WEAPSTICKINESS_GROUND_WITH_YAW )
                            {
                                cgameGlob->killCamEntityRestState = KC_ENT_STUCK_GROUND;
                            }
                        }
                        parentCent = CG_EntGetLinkToParent(localClientNum, Entity);
                        if ( parentCent && (parentCent->nextState.eType == 1 || parentCent->nextState.eType == 2) )
                        {
                            cgameGlob->invalidKillCamEntity = newKillCamEntity;
                            newKillCamEntity = 1023;
                            if ( cgameGlob->killCamLastEntityNum == 1023
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                                            721,
                                            0,
                                            "%s",
                                            "cgameGlob->killCamLastEntityNum != ENTITYNUM_NONE") )
                            {
                                __debugbreak();
                            }
                        }
                        break;
                    case 0xEu:
                        cgameGlob->killCamEntityType = KC_VEHICLE;
                        break;
                    case 0xBu:
                        if ( (Entity->nextState.lerp.u.turret.flags & 8) != 0 || (Entity->nextState.lerp.u.turret.flags & 0x20) != 0 )
                        {
                            newKillCamEntity = 1023;
                            cgameGlob->killCamEntityType = KC_NO_ENTITY;
                            cgameGlob->killCamLastEntityNum = 1023;
                        }
                        else
                        {
                            cgameGlob->killCamEntityType = KC_TURRET;
                        }
                        break;
                    default:
                        if ( Entity->destructible )
                            cgameGlob->killCamEntityType = KC_DESTRUCTIBLE;
                        else
                            cgameGlob->killCamEntityType = KC_SCRIPTED;
                        break;
                }
                cgameGlob->invalidKillCamEntity = 1023;
            }
            else
            {
                if ( cgameGlob->killCamEntity == newKillCamEntity )
                    cgameGlob->invalidKillCamEntity = cgameGlob->killCamEntity;
                newKillCamEntity = 1023;
            }
        }
        if ( newKillCamEntity != cgameGlob->killCamEntity
            && newKillCamEntity == cgameGlob->nextSnap->ps.killCamEntity
            && !killCamJustStarted )
        {
            cgameGlob->killCamLerpEndTime = cgameGlob->time + 300;
        }
        cgameGlob->killCamEntity = newKillCamEntity;
        cgameGlob->killCamLookAtEntity = cgameGlob->nextSnap->ps.killCamTargetEntity;
    }
}

void __cdecl CG_UnpackMatchState(int localClientNum, MatchState *matchState)
{
    int column; // [esp+4h] [ebp-8h]
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp", 859, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    cgameGlob->teamScores[2] = matchState->unarchivedState.alliesScore;
    cgameGlob->teamScores[1] = matchState->unarchivedState.axisScore;
    cgameGlob->scoreLimit = matchState->unarchivedState.scoreLimit;
    cgameGlob->talkFlags = matchState->unarchivedState.talkFlags;
    for ( column = 0; column < 4; ++column )
        cgameGlob->scoreboardColumnTypes[column] = matchState->unarchivedState.scoreboardColumnTypes[column];
    *(_QWORD *)cgameGlob->bombTimer = *(_QWORD *)matchState->archivedState.bombTimer;
    cgameGlob->matchUIVisibilityFlags = matchState->unarchivedState.matchUIVisibilityFlags
                                                                        | matchState->archivedState.matchUIVisibilityFlags;
}

void __cdecl CG_CopyScoreFromClientState(int localClientNum, clientInfo_t *ci, clientState_s *cs)
{
    char *pszIcon; // [esp+4h] [ebp-Ch]
    int column; // [esp+8h] [ebp-8h]
    const cg_s *cgameGlob; // [esp+Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob && !cgameGlob->inKillCam )
    {
        ci->score.kills = cs->score.kills;
        ci->score.ping = cg_ScoresPing_Interval->current.integer * cs->score.ping;
        ci->score.place = cs->score.place;
        ci->score.score = cs->score.score;
        ci->score.deaths = cs->score.deaths;
        for ( column = 0; column < 4; ++column )
            ci->score.scoreboardColumns[column] = cs->score.scoreboardColumns[column];
        if ( ci->score.status_icon != cs->score.status_icon )
        {
            ci->score.status_icon = cs->score.status_icon;
            if ( ci->score.status_icon <= 0 || ci->score.status_icon > 8 )
            {
                if ( !ci->score.status_icon )
                    ci->hStatusIcon = 0;
            }
            else
            {
                pszIcon = CL_GetConfigString(ci->score.status_icon + 3091);
                ci->hStatusIcon = Material_RegisterHandle(pszIcon, 7);
            }
        }
        CL_GetRankIcon(ci->rank, ci->prestige, &ci->hRankIcon);
    }
}

void __cdecl CG_CopyUIVisibilityBitsFomClientState(int localClientNum, clientInfo_t *ci, clientState_s *cs)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob )
    {
        if ( !cgameGlob->inKillCam )
            ci->clientUIVisibilityFlags = cs->clientUIVisibilityFlags;
    }
}

void __cdecl CG_UpdateScoreData(int localClientNum)
{
    team_t team; // [esp+0h] [ebp-14h]
    unsigned int clientNum; // [esp+4h] [ebp-10h]
    int i; // [esp+Ch] [ebp-8h]
    cg_s *cgameGlob; // [esp+10h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->numScores <= 0 )
        cgameGlob->scoresTop = -1;
    cgameGlob->numScores = 0;
    cgameGlob->teamPings[0] = 0;
    cgameGlob->teamPings[1] = 0;
    cgameGlob->teamPings[2] = 0;
    cgameGlob->teamPings[3] = 0;
    cgameGlob->teamPlayers[0] = 0;
    cgameGlob->teamPlayers[1] = 0;
    cgameGlob->teamPlayers[2] = 0;
    cgameGlob->teamPlayers[3] = 0;
    for ( i = 0; i < 32; ++i )
    {
        clientNum = cgameGlob->scoreOrder[i];
        if ( clientNum >= 0x20 )
            break;
        ++cgameGlob->numScores;
        if ( cgameGlob->bgs.clientinfo[clientNum].infoValid )
        {
            team = cgameGlob->bgs.clientinfo[clientNum].team;
            ++cgameGlob->teamPlayers[team];
            cgameGlob->teamPings[team] += cgameGlob->bgs.clientinfo[clientNum].score.ping;
        }
    }
}

void __cdecl CG_UpdateClientFlags(int localClientNum, centity_s *cent, int oldEFlags2, bool newEnt)
{
    signed int flagNum; // [esp+0h] [ebp-8h]
    signed int i; // [esp+4h] [ebp-4h]

    if ( !cent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp", 962, 0, "%s", "cent") )
    {
        __debugbreak();
    }
    if ( ((*((unsigned int *)cent + 201) >> 22) & 1) == 0 )
        CG_ClientFlagResetAll(cent);
    if ( newEnt )
    {
        for ( i = 0; i < 16; ++i )
        {
            if ( (cent->nextState.lerp.eFlags2 & (1 << i)) != 0 )
                CG_ClientFlagSet(cent, i);
        }
    }
    else if ( (unsigned __int16)cent->nextState.lerp.eFlags2 != (unsigned __int16)oldEFlags2 )
    {
        for ( flagNum = 0; flagNum < 16; ++flagNum )
        {
            if ( (cent->nextState.lerp.eFlags2 & (1 << flagNum)) == 0 || (oldEFlags2 & (1 << flagNum)) != 0 )
            {
                if ( (cent->nextState.lerp.eFlags2 & (1 << flagNum)) == 0 && (oldEFlags2 & (1 << flagNum)) != 0 )
                    CG_ClientFlagClear(cent, flagNum);
            }
            else
            {
                CG_ClientFlagSet(cent, flagNum);
            }
        }
    }
}

char __cdecl CG_LocalPlayerRespawned(int localClientNum, snapshot_s *snap)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->snap || !snap )
        return 0;
    if ( snap->ps.stats[4] != cgameGlob->snap->ps.stats[4] )
    {
        if ( !Demo_IsPlaying() )
            return 1;
        if ( snap->ps.stats[4] == 14 )
            return 1;
    }
    return 0;
}

void __cdecl CG_ProcessSnapshots(int localClientNum)
{
    cg_s *cgameGlob; // [esp+8h] [ebp-Ch]
    snapshot_s *snap; // [esp+Ch] [ebp-8h]
    snapshot_s *snapa; // [esp+Ch] [ebp-8h]
    int n; // [esp+10h] [ebp-4h] BYREF

    PROF_SCOPED("process snapshots");

    g_processEvents = 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    CL_GetCurrentSnapshotNumber(localClientNum, &n, &cgameGlob->latestSnapshotTime);
    if ( n != cgameGlob->latestSnapshotNum )
    {
        if ( !Demo_IsPlaying() && n < cgameGlob->latestSnapshotNum )
            Com_Error(ERR_DROP, "CG_ProcessSnapshots: n < cgameGlob->latestSnapshotNum");
        cgameGlob->latestSnapshotNum = n;
    }
    while ( !cgameGlob->snap )
    {
        snap = CG_ReadNextSnapshot(localClientNum);
        if ( !snap )
            return;
        if ( (snap->snapFlags & 2) == 0 )
        {
            CG_SetInitialSnapshot(localClientNum, snap);
            CG_SetNextSnap(localClientNum, snap);
            CG_TransitionSnapshot(localClientNum);
            AimAssist_Setup(localClientNum, &cgameGlob->predictedPlayerState);
        }
    }
    CG_SetFrameInterpolation(localClientNum);
    while ( 1 )
    {
        if ( cgameGlob->nextSnap != cgameGlob->snap && cgameGlob->cubemapShot == CUBEMAPSHOT_NONE )
            goto LABEL_30;
        snapa = CG_ReadNextSnapshot(localClientNum);
        if ( !snapa )
            break;
        if ( !cgameGlob->nextSnap
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                        1704,
                        0,
                        "%s",
                        "cgameGlob->nextSnap") )
        {
            __debugbreak();
        }
        if ( Demo_IsPlaying() || ((cgameGlob->nextSnap->snapFlags ^ snapa->snapFlags) & 4) == 0 )
        {
            if ( !Demo_IsPlaying() && snapa->serverTime - cgameGlob->nextSnap->serverTime < 0 )
                Com_Error(ERR_DROP, "CG_ProcessSnapshots: Server time went backwards");
            CG_SetNextSnap(localClientNum, snapa);
            if ( cgameGlob->newPlayerViewmodel )
            {
                cgameGlob->newPlayerViewmodel = 0;
                CG_UpdateViewWeaponAnim(localClientNum, 1);
            }
            g_processEvents = 1;
            cgameGlob->bgs.latestSnapshotTime = snapa->serverTime;
LABEL_30:
            if ( cgameGlob->time - cgameGlob->snap->serverTime >= 0 && cgameGlob->time - cgameGlob->nextSnap->serverTime < 0 )
                goto LABEL_33;
            CG_TransitionSnapshot(localClientNum);
        }
        else
        {
            CG_SetInitialSnapshot(localClientNum, snapa);
            CG_SetNextSnap(localClientNum, snapa);
            CG_TransitionSnapshot(localClientNum);
        }
    }
    if ( cgameGlob->newPlayerViewmodel )
    {
        cgameGlob->newPlayerViewmodel = 0;
        CG_UpdateViewWeaponAnim(localClientNum, 1);
    }
LABEL_33:
    if ( CG_ItemListLocalClientNum() != localClientNum )
        CG_BuildItemList(localClientNum, cgameGlob->nextSnap);
    if ( !cgameGlob->snap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                    1739,
                    0,
                    "%s",
                    "cgameGlob->snap") )
    {
        __debugbreak();
    }
    if ( cgameGlob->time - cgameGlob->snap->serverTime < 0 )
    {
        cgameGlob->time = cgameGlob->snap->serverTime;
        cgameGlob->physicsTime = cgameGlob->snap->physicsTime;
        cgameGlob->bgs.time = cgameGlob->time;
    }
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                    1748,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    if ( cgameGlob->nextSnap != cgameGlob->snap
        && cgameGlob->nextSnap->serverTime - cgameGlob->time <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_snapshot_mp.cpp",
                    1749,
                    0,
                    "%s",
                    "cgameGlob->nextSnap == cgameGlob->snap || cgameGlob->nextSnap->serverTime - cgameGlob->time > 0") )
    {
        __debugbreak();
    }
}

snapshot_s *__cdecl CG_ReadNextSnapshot(int localClientNum)
{
    cg_s *cgameGlob; // [esp+4h] [ebp-Ch]
    cgs_t *cgs; // [esp+8h] [ebp-8h]
    snapshot_s *dest; // [esp+Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    if ( cgameGlob->latestSnapshotNum > cgs->processedSnapshotNum + 1000 )
        Com_PrintWarning(
            14,
            "WARNING: CG_ReadNextSnapshot: way out of range, %i > %i\n",
            cgameGlob->latestSnapshotNum,
            cgs->processedSnapshotNum);
    while ( cgs->processedSnapshotNum < cgameGlob->latestSnapshotNum )
    {
        dest = &cgameGlob->activeSnapshots[cgameGlob->snap == cgameGlob->activeSnapshots];
        if ( CL_GetSnapshot(localClientNum, ++cgs->processedSnapshotNum, dest) )
        {
            CG_AddLagometerSnapshotInfo(dest);
            return dest;
        }
        CG_AddLagometerSnapshotInfo(0);
    }
    return 0;
}

