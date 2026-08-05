#include "sv_archive_mp.h"
#include <qcommon/msg_mp.h>
#include <demo/demo_common.h>
#include "sv_main_mp.h"
#include <qcommon/sv_msg_write_mp.h>
#include <qcommon/threads.h>
#include <qcommon/sv_msg_write.h>

void __cdecl SV_ArchiveSnapshot(msg_t *msg)
{
    MatchState *MatchStateLocal; // eax
    clientState_s *ClientStateLocal; // eax
    clientState_s *v3; // eax
    const char *v4; // eax
    float v5; // edx
    const char *v6; // eax
    const clientState_s *v7; // eax
    int FollowPlayerStateLocal; // eax
    const char *v9; // eax
    float v10; // ecx
    float *absmax; // [esp+28h] [ebp-2868h]
    float *v12; // [esp+2Ch] [ebp-2864h]
    float *absmin; // [esp+30h] [ebp-2860h]
    float *v14; // [esp+34h] [ebp-285Ch]
    cachedClient_s *v15; // [esp+50h] [ebp-2840h]
    int num; // [esp+54h] [ebp-283Ch]
    int numa; // [esp+54h] [ebp-283Ch]
    client_t *clients; // [esp+5Ch] [ebp-2834h]
    int clientNum; // [esp+68h] [ebp-2828h]
    SnapshotInfo_s snapInfo; // [esp+7Ch] [ebp-2814h] BYREF
    archivedEntity_s *v21; // [esp+94h] [ebp-27FCh]
    int v22; // [esp+98h] [ebp-27F8h]
    int clientIndex; // [esp+9Ch] [ebp-27F4h]
    cachedSnapshot_t *v24; // [esp+A0h] [ebp-27F0h]
    cachedSnapshot_t *v25; // [esp+A4h] [ebp-27ECh]
    int maxclients; // [esp+A8h] [ebp-27E8h]
    int num_clients; // [esp+ACh] [ebp-27E4h]
    gentity_s *v28; // [esp+B0h] [ebp-27E0h]
    int v29; // [esp+B4h] [ebp-27DCh]
    float v30; // [esp+B8h] [ebp-27D8h]
    float *p_number; // [esp+BCh] [ebp-27D4h]
    archivedEntity_s to; // [esp+C0h] [ebp-27D0h] BYREF
    int i; // [esp+1C8h] [ebp-26C8h]
    MatchState *v34; // [esp+1CCh] [ebp-26C4h]
    playerState_s ps; // [esp+1D0h] [ebp-26C0h] BYREF
    MatchState *from; // [esp+287Ch] [ebp-14h]
    int v37; // [esp+2880h] [ebp-10h]
    archivedEntity_s *v38; // [esp+2884h] [ebp-Ch]
    cachedSnapshot_t *v39; // [esp+2888h] [ebp-8h]
    cachedClient_s *v40; // [esp+288Ch] [ebp-4h]
    int savedregs; // [esp+2890h] [ebp+0h] BYREF

    v37 = 0;
    *(unsigned int *)&snapInfo.fromBaseline = 256;
    snapInfo.packetEntityType = ANALYZE_DATATYPE_ENTITYTYPE_GENERALENTITY;
    *(unsigned int *)&snapInfo.packetEntityTypeArchived = 0;
    memset(&snapInfo, 0, 12);
    if ( !svsHeaderValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_archive_mp.cpp",
                    145,
                    0,
                    "%s",
                    "svsHeaderValid") )
    {
        __debugbreak();
    }
    if ( !svsHeader.cachedSnapshotEntities
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_archive_mp.cpp",
                    146,
                    0,
                    "%s",
                    "svsHeader.cachedSnapshotEntities") )
    {
        __debugbreak();
    }
    if ( !svsHeader.cachedSnapshotClients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_archive_mp.cpp",
                    147,
                    0,
                    "%s",
                    "svsHeader.cachedSnapshotClients") )
    {
        __debugbreak();
    }
    if ( !svsHeader.archivedSnapshotBuffer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_archive_mp.cpp",
                    148,
                    0,
                    "%s",
                    "svsHeader.archivedSnapshotBuffer") )
    {
        __debugbreak();
    }
    if ( !svsHeader.cachedSnapshotFrames
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_archive_mp.cpp",
                    149,
                    0,
                    "%s",
                    "svsHeader.cachedSnapshotFrames") )
    {
        __debugbreak();
    }
    if ( !Sys_IsServerThread()
        && sv.smp
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_archive_mp.cpp",
                    155,
                    0,
                    "%s",
                    "Sys_IsServerThread() || !sv.smp") )
    {
        __debugbreak();
    }
    v29 = svsHeader.nextCachedSnapshotFrames - 512;
    if ( svsHeader.nextCachedSnapshotFrames - 512 < 0 )
        v29 = 0;
    svsHeader.archivedEntityCount = 0;
    for ( i = svsHeader.nextCachedSnapshotFrames - 1; ; --i )
    {
        if ( i < v29 )
            goto LABEL_84;
        v39 = &svsHeader.cachedSnapshotFrames[i % 512];
        if ( v39->archivedFrame >= svsHeader.nextArchivedSnapshotFrames - svsHeader.fps && !v39->usesDelta )
            break;
    }
    if ( v39->first_entity >= svsHeader.nextCachedSnapshotEntities - svsHeader.numCachedSnapshotEntities
        && v39->first_client >= svsHeader.nextCachedSnapshotClients - svsHeader.numCachedSnapshotClients )
    {
        MSG_WriteBit0(msg);
        MSG_WriteLong(msg, v39->archivedFrame);
        MSG_WriteLong(msg, svsHeader.time);
        MSG_WriteLong(msg, svsHeader.physicsTime);
        MSG_ClearLastReferencedEntity(msg);
        from = &svsHeader.cachedSnapshotMatchStates[v39->matchState % svsHeader.numCachedSnapshotMatchStates];
        MatchStateLocal = (MatchState *)G_GetMatchStateLocal();
        MSG_WriteDeltaMatchState(&snapInfo, msg, svsHeader.time, from, MatchStateLocal);
        MSG_ClearLastReferencedEntity(msg);
        maxclients = svsHeader.maxclients;
        num_clients = v39->num_clients;
        v40 = 0;
        clientNum = 0;
        v22 = 0;

        while ( clientNum < maxclients || v22 < num_clients )
        {
            PROF_SCOPED("clients");

            if ( clientNum >= maxclients || svsHeader.clients[clientNum].header.state >= CS_CONNECTED )
            {
                if ( v22 < num_clients )
                {
                    v15 = &svsHeader.cachedSnapshotClients[(v22 + v39->first_client) % svsHeader.numCachedSnapshotClients];
                    clientIndex = v15->cs.clientIndex;
                }
                else
                {
                    v15 = 0;
                    clientIndex = 9999;
                }
                snapInfo.clientNum = clientNum;
                if ( clientNum == clientIndex )
                {
                    if ( !v15
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_archive_mp.cpp",
                                    239,
                                    0,
                                    "%s",
                                    "cachedClient2") )
                    {
                        __debugbreak();
                    }
                    ClientStateLocal = (clientState_s *)G_GetClientStateLocal(clientNum);
                    MSG_WriteDeltaClient(&snapInfo, msg, svsHeader.time, &v15->cs, ClientStateLocal, 1);
                    if ( GetFollowPlayerStateLocal(clientNum, &ps) )
                    {
                        MSG_WriteBit1(msg);
                        if ( v15->playerStateExists )
                            MSG_WriteDeltaPlayerstate(&snapInfo, msg, svsHeader.time, &v15->ps, &ps);
                        else
                            MSG_WriteDeltaPlayerstate(&snapInfo, msg, svsHeader.time, 0, &ps);
                    }
                    else
                    {
                        MSG_WriteBit0(msg);
                    }
                    ++v22;
                    ++clientNum;
                }
                else if ( clientNum >= clientIndex )
                {
                    if ( clientNum > clientIndex )
                        ++v22;
                }
                else
                {
                    v3 = (clientState_s *)G_GetClientStateLocal(clientNum);
                    MSG_WriteDeltaClient(&snapInfo, msg, svsHeader.time, 0, v3, 1);
                    if ( GetFollowPlayerStateLocal(clientNum, &ps) )
                    {
                        MSG_WriteBit1(msg);
                        MSG_WriteDeltaPlayerstate(&snapInfo, msg, svsHeader.time, 0, &ps);
                    }
                    else
                    {
                        MSG_WriteBit0(msg);
                    }
                    ++clientNum;
                }
            }
            else
            {
                ++clientNum;
            }
        }
        MSG_WriteBit0(msg);
        MSG_ClearLastReferencedEntity(msg);
        v30 = NAN;

        for ( num = 0; num < svsHeader.num_entities; ++num )
        {
            PROF_SCOPED("entities");
            v28 = SV_GentityNumLocal(num);
            if ( v28->r.linked )
            {
                p_number = (float *)&v28->s.number;
                if ( v28->s.number != num )
                {
                    v4 = va(
                                 "entnum: %d vs %d, eType: %d, origin: %f %f %f",
                                 *(unsigned int *)p_number,
                                 num,
                                 *((__int16 *)p_number + 95),
                                 p_number[73],
                                 p_number[74],
                                 p_number[75]);
                    if ( !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_archive_mp.cpp",
                                    301,
                                    0,
                                    "%s\n\t%s",
                                    "ent->s.number == e",
                                    v4) )
                        __debugbreak();
                }
                if ( *((unsigned int *)p_number + 59)
                    || (*((_BYTE *)p_number + 221) & 1) != 0
                    || (*((_BYTE *)p_number + 226) & 1) == 0
                    && ((*((_BYTE *)p_number + 226) & 0x18) != 0 || svsHeader.svEntities[*(unsigned int *)p_number].numClusters)
                    && (*((_BYTE *)p_number + 226) & 0x80) == 0 )
                {
                    LODWORD(v5) = 360 * *(unsigned int *)p_number;
                    v38 = (archivedEntity_s *)((char *)&svsHeader.svEntities->baseline + LODWORD(v5));
                    if ( (svEntity_s *)((char *)svsHeader.svEntities + LODWORD(v5)) == (svEntity_s *)-4
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_archive_mp.cpp",
                                    319,
                                    0,
                                    "%s",
                                    "baseline") )
                    {
                        __debugbreak();
                    }
                    memcpy(&to, p_number, 0xE0u);
                    to.r.svFlags = *((unsigned __int8 *)p_number + 226);
                    if ( *((unsigned int *)p_number + 59) )
                        to.r.svFlags |= 8u;
                    to.r.clientMask[0] = (int)p_number[57];
                    to.r.absmin[0] = p_number[67];
                    to.r.absmin[1] = p_number[68];
                    to.r.absmin[2] = p_number[69];
                    to.r.absmax[0] = p_number[70];
                    to.r.absmax[1] = p_number[71];
                    to.r.absmax[2] = p_number[72];
                    if ( LODWORD(v30) == *(unsigned int *)p_number )
                    {
                        v6 = va("lastEntityNum is %i, cur entnum is %i", v30, *(unsigned int *)p_number);
                        if ( !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_archive_mp.cpp",
                                        332,
                                        0,
                                        "%s\n\t%s",
                                        "lastEntityNum != ent->s.number",
                                        v6) )
                            __debugbreak();
                    }
                    snapInfo.fromBaseline = 1;
                    if ( MSG_WriteDeltaArchivedEntity(&snapInfo, msg, svsHeader.time, v38, &to, DELTA_FLAGS_FORCE) )
                        ++svsHeader.archivedEntityCount;
                    snapInfo.fromBaseline = 0;
                    v30 = *p_number;
                }
            }
        }
        goto skipDelta;
    }
LABEL_84:
    //PROF_SCOPED("write delta");
    MSG_WriteBit1(msg);
    MSG_WriteLong(msg, svsHeader.time);
    MSG_WriteLong(msg, svsHeader.physicsTime);
    MSG_ClearLastReferencedEntity(msg);
    v25 = &svsHeader.cachedSnapshotFrames[svsHeader.nextCachedSnapshotFrames % 512];
    v24 = v25;
    v25->archivedFrame = svsHeader.nextArchivedSnapshotFrames;
    v24->matchState = svsHeader.nextCachedSnapshotMatchStates;
    v24->num_entities = 0;
    v24->first_entity = svsHeader.nextCachedSnapshotEntities;
    v24->num_clients = 0;
    v24->first_client = svsHeader.nextCachedSnapshotClients;
    v24->usesDelta = 0;
    v24->time = svsHeader.time;
    v24->physicsTime = svsHeader.physicsTime;
    MSG_ClearLastReferencedEntity(msg);
    v34 = &svsHeader.cachedSnapshotMatchStates[svsHeader.nextCachedSnapshotMatchStates
                                                                                     % svsHeader.numCachedSnapshotMatchStates];
    memcpy(v34, G_GetMatchStateLocal(), sizeof(MatchState));
    MSG_WriteDeltaMatchState(&snapInfo, msg, svsHeader.time, 0, v34);
    if ( ++svsHeader.nextCachedSnapshotMatchStates >= 2147483646 )
        Com_Error(ERR_FATAL, "svsHeader.nextCachedSnapshotMatchStates wrapped");
    MSG_ClearLastReferencedEntity(msg);
    i = 0;
    clients = svsHeader.clients;
    while ( i < svsHeader.maxclients )
    {
        if ( clients->header.state >= CS_CONNECTED )
        {
            v40 = &svsHeader.cachedSnapshotClients[svsHeader.nextCachedSnapshotClients % svsHeader.numCachedSnapshotClients];
            v7 = G_GetClientStateLocal(i);
            memcpy(&v40->cs, v7, sizeof(v40->cs));
            MSG_WriteDeltaClient(&snapInfo, msg, svsHeader.time, 0, &v40->cs, 1);
            FollowPlayerStateLocal = GetFollowPlayerStateLocal(i, &v40->ps);
            v40->playerStateExists = FollowPlayerStateLocal;
            if ( v40->playerStateExists )
            {
                MSG_WriteBit1(msg);
                MSG_WriteDeltaPlayerstate(&snapInfo, msg, svsHeader.time, 0, &v40->ps);
            }
            else
            {
                MSG_WriteBit0(msg);
            }
            if ( ++svsHeader.nextCachedSnapshotClients >= 2147483646 )
                Com_Error(ERR_FATAL, "svsHeader.nextCachedSnapshotClients wrapped");
            ++v24->num_clients;
        }
        ++i;
        ++clients;
    }
    MSG_WriteBit0(msg);
    MSG_ClearLastReferencedEntity(msg);
    for ( numa = 0; numa < svsHeader.num_entities; ++numa )
    {
        v28 = SV_GentityNumLocal(numa);
        if ( v28->r.linked )
        {
            p_number = (float *)&v28->s.number;
            if ( v28->s.number != numa )
            {
                v9 = va(
                             "entnum: %d vs %d, eType: %d, origin: %f %f %f",
                             *(unsigned int *)p_number,
                             numa,
                             *((__int16 *)p_number + 95),
                             p_number[73],
                             p_number[74],
                             p_number[75]);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_archive_mp.cpp",
                                440,
                                0,
                                "%s\n\t%s",
                                "ent->s.number == e",
                                v9) )
                    __debugbreak();
            }
            if ( *((unsigned int *)p_number + 59)
                || (*((_BYTE *)p_number + 221) & 1) != 0
                || (*((_BYTE *)p_number + 226) & 1) == 0
                && ((*((_BYTE *)p_number + 226) & 0x18) != 0 || svsHeader.svEntities[*(unsigned int *)p_number].numClusters)
                && (*((_BYTE *)p_number + 226) & 0x80) == 0 )
            {
                LODWORD(v10) = 360 * *(unsigned int *)p_number;
                v38 = (archivedEntity_s *)((char *)&svsHeader.svEntities->baseline + LODWORD(v10));
                if ( (svEntity_s *)((char *)svsHeader.svEntities + LODWORD(v10)) == (svEntity_s *)-4
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_archive_mp.cpp",
                                458,
                                0,
                                "%s",
                                "baseline") )
                {
                    __debugbreak();
                }
                v21 = &svsHeader.cachedSnapshotEntities[svsHeader.nextCachedSnapshotEntities
                                                                                            % svsHeader.numCachedSnapshotEntities];
                memcpy(v21, p_number, 0xE0u);
                v21->r.svFlags = *((unsigned __int8 *)p_number + 226);
                if ( *((unsigned int *)p_number + 59) )
                    v21->r.svFlags |= 8u;
                v21->r.clientMask[0] = (int)p_number[57];
                absmin = v21->r.absmin;
                v14 = p_number + 67;
                v21->r.absmin[0] = p_number[67];
                absmin[1] = v14[1];
                absmin[2] = v14[2];
                absmax = v21->r.absmax;
                v12 = p_number + 70;
                v21->r.absmax[0] = p_number[70];
                absmax[1] = v12[1];
                absmax[2] = v12[2];
                snapInfo.fromBaseline = 1;
                MSG_WriteDeltaArchivedEntity(&snapInfo, msg, svsHeader.time, v38, v21, DELTA_FLAGS_FORCE);
                ++svsHeader.archivedEntityCount;
                snapInfo.fromBaseline = 0;
                if ( ++svsHeader.nextCachedSnapshotEntities >= 2147483646 )
                    Com_Error(ERR_FATAL, "svsHeader.nextCachedSnapshotEntities wrapped");
                ++v24->num_entities;
            }
        }
    }
    if ( ++svsHeader.nextCachedSnapshotFrames >= 2147483646 )
        Com_Error(ERR_FATAL, "svsHeader.nextCachedSnapshotFrames wrapped");
skipDelta:
    MSG_WriteEntityIndex(&snapInfo, msg, 1023, 10);
}

gentity_s *__cdecl SV_GentityNumLocal(int num)
{
    if ( !svsHeaderValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_archive_mp.cpp",
                    37,
                    0,
                    "%s",
                    "svsHeaderValid") )
    {
        __debugbreak();
    }
    return (gentity_s *)((char *)svsHeader.gentities + num * svsHeader.gentitySize);
}

const clientState_s *__cdecl G_GetClientStateLocal(int clientNum)
{
    if ( !svsHeaderValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_archive_mp.cpp",
                    44,
                    0,
                    "%s",
                    "svsHeaderValid") )
    {
        __debugbreak();
    }
    return (clientState_s *)((char *)svsHeader.firstClientState + clientNum * svsHeader.clientSize);
}

const MatchState *__cdecl G_GetMatchStateLocal()
{
    if ( !svsHeaderValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_archive_mp.cpp",
                    50,
                    0,
                    "%s",
                    "svsHeaderValid") )
    {
        __debugbreak();
    }
    return svsHeader.matchState;
}

hudelem_s g_dummyHudCurrent_1;
int __cdecl GetFollowPlayerStateLocal(int clientNum, playerState_s *ps)
{
    unsigned int index; // [esp+8h] [ebp-8h]

    if ( !svsHeaderValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_archive_mp.cpp",
                    66,
                    0,
                    "%s",
                    "svsHeaderValid") )
    {
        __debugbreak();
    }
    if ( (*(int *)((_BYTE *)&svsHeader.firstPlayerState->otherFlags + clientNum * svsHeader.clientSize) & 4) != 0 )
    {
        memcpy(
            (unsigned __int8 *)ps,
            (unsigned __int8 *)svsHeader.firstPlayerState + clientNum * svsHeader.clientSize,
            sizeof(playerState_s));
        for ( index = 0; index < 0x1F && ps->hud.current[index].type; ++index )
        {
            memset(&ps->hud.current[index], 0, sizeof(ps->hud.current[index]));
            if ( ps->hud.current[index].type )
            {
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_archive_mp.cpp",
                                82,
                                0,
                                "%s",
                                "ps->hud.current[index].type == HE_TYPE_FREE") )
                    __debugbreak();
            }
        }
        while ( index < 0x1F )
        {
            if ( memcmp(&ps->hud.current[index], &g_dummyHudCurrent_1, 0x70u)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_archive_mp.cpp",
                            89,
                            0,
                            "%s",
                            "!memcmp( &ps->hud.current[index], &g_dummyHudCurrent, sizeof( g_dummyHudCurrent ) )") )
            {
                __debugbreak();
            }
            ++index;
        }
        return 1;
    }
    else
    {
        memset(ps, 0, sizeof(playerState_s));
        return 0;
    }
}

