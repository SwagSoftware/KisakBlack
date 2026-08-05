#include "sv_snapshot_mp.h"
#include <demo/demo_common.h>
#include <qcommon/sv_msg_write_mp.h>
#include "sv_main_mp.h"

#include <cstring>
#include <qcommon/common.h>
#include <qcommon/sv_msg_write.h>
#include <glass/glass_server.h>
#include "sv_init_mp.h"
#include <stdarg.h>
#include "sv_snapshot_profile_mp.h"
#include <cgame/cg_draw_debug.h>
#include <client_mp/sv_client_mp.h>
#include <qcommon/msg.h>
#include <bgame/bg_misc.h>
#include <game_mp/g_active_mp.h>
#include <qcommon/com_gamemodes.h>
#include <live/live_stats.h>
#include <server/sv_game.h>
#include <qcommon/cm_test.h>
#include <qcommon/cm_load.h>
#include <win32/win_shared.h>
#include "sv_net_chan_mp.h"
#include <win32/win_net.h>
#include <client_mp/console_mp.h>
#include "sv_bot_mp.h"
#include <demo/demo_recording.h>
#include "sv_archive_mp.h"

msg_t g_archiveMsg;

void __cdecl SV_WriteSnapshotToClient(client_t *client, msg_t *msg)
{
    int v2; // edx
    clientSnapshot_t *frame; // [esp+8h] [ebp-60h]
    int lastframe; // [esp+10h] [ebp-58h]
    int from_num_entities; // [esp+18h] [ebp-50h]
    int from_first_entity; // [esp+20h] [ebp-48h]
    SnapshotInfo_s snapInfo; // [esp+24h] [ebp-44h] BYREF
    int sendAsActive; // [esp+3Ch] [ebp-2Ch]
    int from_num_clients; // [esp+40h] [ebp-28h]
    const MatchState *toEA; // [esp+44h] [ebp-24h]
    clientSnapshot_t *remoteFrame; // [esp+48h] [ebp-20h]
    int lastServerTime; // [esp+4Ch] [ebp-1Ch]
    int i; // [esp+50h] [ebp-18h]
    const MatchState *to; // [esp+54h] [ebp-14h]
    const clientSnapshot_t *oldframe; // [esp+58h] [ebp-10h]
    int snapFlags; // [esp+5Ch] [ebp-Ch]
    int clientNum; // [esp+60h] [ebp-8h]
    int from_first_client; // [esp+64h] [ebp-4h]
    int savedregs; // [esp+68h] [ebp+0h] BYREF

    if ( !svsHeaderValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    849,
                    0,
                    "%s",
                    "svsHeaderValid") )
    {
        __debugbreak();
    }
    if ( !client
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    856,
                    0,
                    "%s",
                    "localClient") )
    {
        __debugbreak();
    }
    clientNum = client - svsHeader.clients;
    if ( (unsigned int)clientNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    860,
                    0,
                    "%s\n\t(clientNum) = %i",
                    "(clientNum >= 0 && clientNum < 32)",
                    clientNum) )
    {
        __debugbreak();
    }
    memset(&snapInfo.snapshotDeltaTime, 0, 16);
    snapInfo.clientNum = clientNum;
    snapInfo.client = &client->header;
    remoteFrame = &client->frames[client->header.netchan.outgoingSequence & 0x1F];
    remoteFrame->serverTime = svsHeader.time;
    remoteFrame->physicsTime = svsHeader.physicsTime;
    frame = remoteFrame;
    if ( !remoteFrame
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp", 874, 0, "%s", "frame") )
    {
        __debugbreak();
    }
    if ( client->header.deltaMessage > 0 && client->header.state == CS_ACTIVE )
    {
        if ( client->header.netchan.outgoingSequence - client->header.deltaMessage < 29 )
        {
            v2 = client->header.deltaMessage & 0x1F;
            oldframe = &client->frames[v2];
            //if ( (client_t *)((char *)client + v2 * 9932) == (client_t *)-22388
            //    && !Assert_MyHandler(
            //                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
            //                911,
            //                0,
            //                "%s",
            //                "oldframe") )
            //{
            //    __debugbreak();
            //}
            lastframe = client->header.netchan.outgoingSequence - client->header.deltaMessage;
            lastServerTime = oldframe->serverTime;
            if ( oldframe->first_entity < svsHeader.nextSnapshotEntities - svsHeader.numSnapshotEntities )
            {
                Com_PrintWarning(
                    15,
                    "%s: Delta request from out of date entities - delta against entity %i, oldest is %i, current is %i.    Their old"
                    " snapshot had %i entities in it\n",
                    client->name,
                    oldframe->first_entity,
                    svs.nextSnapshotEntities - svs.numSnapshotEntities,
                    svs.nextSnapshotEntities,
                    oldframe->num_entities);
                oldframe = 0;
                (lastframe) = 0;
                lastServerTime = 0;
            }
        }
        else
        {
            Com_DPrintf(15, "%s: Delta request from out of date packet.\n", client->name);
            oldframe = 0;
            (lastframe) = 0;
            lastServerTime = 0;
        }
    }
    else
    {
        oldframe = 0;
        (lastframe) = 0;
        lastServerTime = 0;
    }
    MSG_WriteByte(msg, 0xCu);
    MSG_WriteLong(msg, svsHeader.time);
    MSG_WriteLong(msg, svsHeader.physicsTime);
    MSG_WriteByte(msg, lastframe);
    snapInfo.snapshotDeltaTime = lastServerTime;
    snapFlags = svsHeader.snapFlagServerBit;
    if ( client->header.rateDelayed )
        snapFlags |= 1u;
    sendAsActive = client->header.sendAsActive;
    if ( client->header.state == CS_ACTIVE )
    {
        sendAsActive = 1;
    }
    else if ( client->header.state != CS_ZOMBIE )
    {
        sendAsActive = 0;
    }
    if ( client->header.sendAsActive != sendAsActive )
        client->header.sendAsActive = sendAsActive;
    if ( !sendAsActive )
        snapFlags |= 2u;
    MSG_WriteByte(msg, snapFlags);
    MSG_GetUsedBitCount(msg);
    MSG_ClearLastReferencedEntity(msg);
    toEA = &svsHeader.snapshotMatchStates[frame->matchState % svsHeader.numSnapshotMatchStates];
    to = toEA;
    if ( oldframe )
        MSG_WriteDeltaMatchState(
            &snapInfo,
            msg,
            svsHeader.time,
            &svsHeader.snapshotMatchStates[oldframe->matchState % svsHeader.numSnapshotMatchStates],
            (MatchState *)to);
    else
        MSG_WriteDeltaMatchState(&snapInfo, msg, svsHeader.time, 0, (MatchState *)to);
    MSG_ClearLastReferencedEntity(msg);
    if ( oldframe )
    {
        MSG_WriteDeltaPlayerstate(&snapInfo, msg, svsHeader.time, &oldframe->ps, &frame->ps);
        from_num_entities = oldframe->num_entities;
        from_first_entity = oldframe->first_entity;
        from_num_clients = oldframe->num_clients;
        from_first_client = oldframe->first_client;
    }
    else
    {
        MSG_WriteDeltaPlayerstate(&snapInfo, msg, svsHeader.time, 0, &frame->ps);
        from_num_entities = 0;
        from_first_entity = 0;
        from_num_clients = 0;
        from_first_client = 0;
    }
    MSG_ClearLastReferencedEntity(msg);
    SV_EmitPacketEntities(&snapInfo, from_num_entities, from_first_entity, frame->num_entities, frame->first_entity, msg);
    SV_EmitPacketClients(&snapInfo, from_num_clients, from_first_client, frame->num_clients, frame->first_client, msg);
    if ( oldframe )
        GlassSv_WriteSnapshotToClient(msg, oldframe->serverTime);
    else
        GlassSv_WriteSnapshotToClient(msg, 0);
    for ( i = 0; i < sv_padPackets->current.integer; ++i )
        MSG_WriteByte(msg, 0);
}

void __cdecl SV_EmitPacketEntities(
                SnapshotInfo_s *snapInfo,
                int from_num_entities,
                int from_first_entity,
                int to_num_entities,
                int to_first_entity,
                msg_t *msg)
{
    const char *v6; // eax
    const char *v7; // [esp+4h] [ebp-444h]
    const char *v8; // [esp+Ch] [ebp-43Ch]
    int number; // [esp+10h] [ebp-438h]
    int newnum; // [esp+1Ch] [ebp-42Ch] BYREF
    const entityState_s *oldent; // [esp+20h] [ebp-428h] BYREF
    int oldindex; // [esp+24h] [ebp-424h]
    int oldnum; // [esp+28h] [ebp-420h] BYREF
    int newindex; // [esp+2Ch] [ebp-41Ch]
    bool entityFound[1024]; // [esp+30h] [ebp-418h] BYREF
    const entityState_s *baseline; // [esp+434h] [ebp-14h]
    const entityState_s *newent; // [esp+438h] [ebp-10h] BYREF
    int bitsStart; // [esp+43Ch] [ebp-Ch]
    int quickBits; // [esp+440h] [ebp-8h] BYREF
    int total_bits; // [esp+444h] [ebp-4h]

    bitsStart = MSG_GetUsedBitCount(msg);
    quickBits = 0;
    total_bits = MSG_GetUsedBitCount(msg);
    MSG_ClearLastReferencedEntity(msg);
    memset(entityFound, 0, sizeof(entityFound));
    g_entsTransmitted = 0;
    g_entsDeltaCompared = 0;
    g_entsClientOnce = 0;
    newent = 0;
    oldent = 0;
    newindex = 0;
    oldindex = 0;
    while ( newindex < to_num_entities || oldindex < from_num_entities )
    {
        SV_EmitPacketEntities_GetOldEnt(newindex, to_num_entities, to_first_entity, &newent, &newnum);
        SV_EmitPacketEntities_GetOldEnt(oldindex, from_num_entities, from_first_entity, &oldent, &oldnum);
        //BLOPS_NULLSUB();
        if ( newnum <= oldnum )
        {
            number = newent->number;
            if ( (newent->number < 0 || number >= 1024)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                            409,
                            0,
                            "%s\n\t(number) = %i",
                            "(( number >= 0 && number < (1<<10) ))",
                            number) )
            {
                __debugbreak();
            }
            if ( entityFound[number] )
            {
                v6 = va(
                             "number is %i, oldnum is %i, newnum is %i, to_first is %i, from_first is %i, newindex is %i, oldindex is %i",
                             number,
                             oldnum,
                             newnum,
                             to_first_entity,
                             from_first_entity,
                             newindex,
                             oldindex);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                410,
                                0,
                                "%s\n\t%s",
                                "!entityFound[ number ]",
                                v6) )
                    __debugbreak();
            }
            entityFound[number] = 1;
        }
        SV_EmitPacketEntities_DebugStart(&quickBits, msg);
        if ( newnum == oldnum )
        {
            MSG_WriteEntity(snapInfo, msg, svsHeader.time, (entityState_s *)oldent, newent, DELTA_FLAGS_NONE);
            if ( newent )
                v8 = "";
            else
                v8 = " (remove)";
            if ( newent )
                SV_EmitPacketEntities_Debug(
                    snapInfo,
                    &quickBits,
                    msg,
                    "Entity %d delta %d bits%s\n",
                    newent->number,
                    quickBits,
                    v8);
            else
                SV_EmitPacketEntities_Debug(
                    snapInfo,
                    &quickBits,
                    msg,
                    "Entity %d delta %d bits%s\n",
                    oldent->number,
                    quickBits,
                    v8);
            ++oldindex;
            ++newindex;
        }
        else if ( newnum >= oldnum )
        {
            if ( newnum > oldnum )
            {
                MSG_WriteEntity(snapInfo, msg, svsHeader.time, (entityState_s *)oldent, 0, DELTA_FLAGS_FORCE);
                SV_EmitPacketEntities_Debug(snapInfo, &quickBits, msg, 0);
                ++oldindex;
            }
        }
        else
        {
            baseline = &svsHeader.svEntities[newnum].baseline.s;
            if ( &svsHeader.svEntities[newnum] == (svEntity_s *)-4
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                            462,
                            0,
                            "%s",
                            "baseline") )
            {
                __debugbreak();
            }
            snapInfo->fromBaseline = 1;
            MSG_WriteEntity(snapInfo, msg, svsHeader.time, (entityState_s *)baseline, newent, DELTA_FLAGS_FORCE);
            snapInfo->fromBaseline = 0;
            if ( newent )
                v7 = "";
            else
                v7 = " (remove)";
            if ( newent )
                SV_EmitPacketEntities_Debug(
                    snapInfo,
                    &quickBits,
                    msg,
                    "Entity %d delta %d bits%s\n",
                    newent->number,
                    quickBits,
                    v7);
            else
                SV_EmitPacketEntities_Debug(
                    snapInfo,
                    &quickBits,
                    msg,
                    "Entity %d delta %d bits%s\n",
                    oldent->number,
                    quickBits,
                    v7);
            ++newindex;
        }
    }
    MSG_WriteEntityIndex(snapInfo, msg, 1023, 10);
    MSG_GetUsedBitCount(msg);
}

void __cdecl SV_EmitPacketEntities_GetOldEnt(
                int oldindex,
                int from_num_entities,
                int from_first_entity,
                const entityState_s **oldent,
                int *oldnum)
{
    if ( oldindex < from_num_entities )
    {
        *oldent = &svsHeader.snapshotEntities[(oldindex + from_first_entity) % svsHeader.numSnapshotEntities];
        *oldnum = (*oldent)->number;
    }
    else
    {
        *oldent = 0;
        *oldnum = 9999;
    }
}

void __cdecl SV_EmitPacketEntities_DebugStart(int *quickBits, msg_t *msg)
{
    *quickBits = MSG_GetUsedBitCount(msg);
}

void SV_EmitPacketEntities_Debug(SnapshotInfo_s *snapInfo, int *quickBits, msg_t *msg, const char *str, ...)
{
    char text[1028]; // [esp+4h] [ebp-408h] BYREF
    va_list va; // [esp+424h] [ebp+18h] BYREF

    va_start(va, str);
    *quickBits = MSG_GetUsedBitCount(msg) - *quickBits;
    if ( quickBits )
    {
        if ( sv_debugPacketContentsQuick->current.integer > 0 && !snapInfo->archived )
        {
            sv_quickBitsTotal += *quickBits;
            if ( msg )
            {
                _vsnprintf(text, 0x400u, str, va);
                CG_QuickPrint(text);
            }
        }
    }
    if ( *quickBits )
        ++g_entsTransmitted;
}

void __cdecl SV_EmitPacketClients(
                SnapshotInfo_s *snapInfo,
                int from_num_clients,
                int from_first_client,
                int to_num_clients,
                int to_first_client,
                msg_t *msg)
{
    int newnum; // [esp+4h] [ebp-1Ch] BYREF
    const clientState_s *oldclient; // [esp+8h] [ebp-18h] BYREF
    int oldindex; // [esp+Ch] [ebp-14h]
    int oldnum; // [esp+10h] [ebp-10h] BYREF
    int newindex; // [esp+14h] [ebp-Ch]
    const clientState_s *newclient; // [esp+18h] [ebp-8h] BYREF
    int bitsStart; // [esp+1Ch] [ebp-4h]

    bitsStart = MSG_GetUsedBitCount(msg);
    if ( !svsHeaderValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    646,
                    0,
                    "%s",
                    "svsHeaderValid") )
    {
        __debugbreak();
    }
    MSG_ClearLastReferencedEntity(msg);
    newclient = 0;
    oldclient = 0;
    newindex = 0;
    for ( oldindex = 0; ; ++oldindex )
    {
        do
        {
            while ( 1 )
            {
                while ( 1 )
                {
                    if ( newindex >= to_num_clients && oldindex >= from_num_clients )
                    {
                        MSG_WriteBit0(msg);
                        return;
                    }
                    SV_EmitPacketClients_GetOldClient(newindex, to_num_clients, to_first_client, &newclient, &newnum);
                    SV_EmitPacketClients_GetOldClient(oldindex, from_num_clients, from_first_client, &oldclient, &oldnum);
                    //BLOPS_NULLSUB();
                    if ( newnum != oldnum )
                        break;
                    MSG_WriteDeltaClient(snapInfo, msg, svsHeader.time, (clientState_s *)oldclient, (clientState_s *)newclient, 0);
                    ++oldindex;
                    ++newindex;
                }
                if ( newnum >= oldnum )
                    break;
                MSG_WriteDeltaClient(snapInfo, msg, svsHeader.time, 0, (clientState_s *)newclient, 1);
                ++newindex;
            }
        }
        while ( newnum <= oldnum );
        if ( !oldclient
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                        708,
                        0,
                        "%s",
                        "oldclient") )
        {
            __debugbreak();
        }
        if ( oldclient->clientIndex <= msg->lastEntityRef )
            break;
        MSG_WriteDeltaClient(snapInfo, msg, svsHeader.time, (clientState_s *)oldclient, 0, 1);
    }
    Com_PrintError(
        15,
        "** Client index LE msg->lastEntityRef:\n"
        "**     lastEntityRef is %d    clientIndex is %d\n"
        "**     newnum %d    oldnum %d    from_num_clients %d    to_num_clients %d\n"
        "**     from_first_client %d    to_first_client %d    numSnapshotClients %d\n",
        msg->lastEntityRef,
        oldclient->clientIndex,
        newnum,
        oldnum,
        from_num_clients,
        to_num_clients,
        from_first_client,
        to_first_client,
        svsHeader.numSnapshotClients);
    if ( !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    712,
                    0,
                    "oldclient->clientIndex <= msg->lastEntityRef (please attach console log to bug)") )
        __debugbreak();
    MSG_WriteBit0(msg);
    SV_DropClient(&svs.clients[snapInfo->clientNum], "EXE_TRANSMITERROR", 1, 1);
}

void __cdecl SV_EmitPacketClients_GetOldClient(
                int oldindex,
                int from_num_clients,
                int from_first_client,
                const clientState_s **oldclient,
                int *oldnum)
{
    if ( oldindex < from_num_clients )
    {
        *oldclient = &svsHeader.snapshotClients[(oldindex + from_first_client) % svsHeader.numSnapshotClients];
        *oldnum = (*oldclient)->clientIndex;
    }
    else
    {
        *oldclient = 0;
        *oldnum = 9999;
    }
}

void __cdecl SV_UpdateServerCommandsToClient(client_t *client, msg_t *msg)
{
    int bitsUsed; // [esp+0h] [ebp-8h]
    int bitsUseda; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if ( (unsigned int)(client - svs.clients) >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    1368,
                    0,
                    "client - svs.clients doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    client - svs.clients,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    if ( client->reliableAcknowledge + 1 < client->reliableSequence && sv_debugReliableCmds->current.enabled )
        Com_Printf(15, "Client %s has the following un-ack'd reliable commands:\n", client->name);
    bitsUsed = MSG_GetUsedBitCount(msg);
    for ( i = client->reliableAcknowledge + 1; i <= client->reliableSequence; ++i )
    {
        MSG_WriteByte(msg, 0xAu);
        MSG_WriteLong(msg, i);
        MSG_WriteString(msg, client->reliableCommandInfo[i & 0x7F].cmd);
        if ( sv_debugReliableCmds->current.enabled )
            Com_Printf(15, "%i: %s\n", i - (client->reliableAcknowledge + 1), client->reliableCommandInfo[i & 0x7F].cmd);
        if ( sv_debugPacketContentsQuick->current.integer > 1 )
            CG_QuickPrint(
                "ServerCmd: %i: %s\n",
                i - (client->reliableAcknowledge + 1),
                client->reliableCommandInfo[i & 0x7F].cmd);
    }
    bitsUseda = MSG_GetUsedBitCount(msg) - bitsUsed;
    if ( bitsUseda )
    {
        sv_quickBitsTotal += bitsUseda;
        if ( sv_debugPacketContentsQuick->current.integer > 0 )
            CG_QuickPrint("ServerCmd bits %d\n", bitsUseda);
    }
    client->reliableSent = client->reliableSequence;
}

void __cdecl SV_UpdateServerCommandsToClient_PreventOverflow(client_t *client, msg_t *msg, int iMsgSize)
{
    int i; // [esp+14h] [ebp-4h]
    int ia; // [esp+14h] [ebp-4h]

    for ( i = client->reliableAcknowledge + 1;
                i <= client->reliableSequence
         && (int)(strlen(client->reliableCommandInfo[i & 0x7F].cmd) + 6 + msg->cursize) < iMsgSize;
                ++i )
    {
        MSG_WriteByte(msg, 0xAu);
        MSG_WriteLong(msg, i);
        MSG_WriteString(msg, client->reliableCommandInfo[i & 0x7F].cmd);
    }
    ia = i - 1;
    if ( ia > client->reliableSent )
        client->reliableSent = ia;
}

char __cdecl SV_GetClientPositionsAtTime(int gametime, float (*pos)[3], float (*angles)[3], bool *success)
{
    const char *v4; // eax
    const char *v5; // eax
    const char *v7; // eax
    const char *v8; // eax
    float *v9; // [esp+20h] [ebp-6C0h]
    float *v10; // [esp+24h] [ebp-6BCh]
    float *v11; // [esp+28h] [ebp-6B8h]
    float *v12; // [esp+2Ch] [ebp-6B4h]
    float *v13; // [esp+48h] [ebp-698h]
    float *v14; // [esp+4Ch] [ebp-694h]
    float *v15; // [esp+50h] [ebp-690h]
    float *v16; // [esp+54h] [ebp-68Ch]
    float endPos[32][3]; // [esp+70h] [ebp-670h] BYREF
    int startOffset; // [esp+1F0h] [ebp-4F0h] BYREF
    int rewindDeltaTime; // [esp+1F4h] [ebp-4ECh]
    float startAngles[32][3]; // [esp+1F8h] [ebp-4E8h] BYREF
    bool foundStart; // [esp+37Fh] [ebp-361h]
    int endOffset; // [esp+380h] [ebp-360h] BYREF
    float progress; // [esp+384h] [ebp-35Ch]
    float startPos[32][3]; // [esp+388h] [ebp-358h] BYREF
    bool startSuccess[32]; // [esp+508h] [ebp-1D8h] BYREF
    bool endSuccess[32]; // [esp+528h] [ebp-1B8h] BYREF
    cachedSnapshot_t *frameStart; // [esp+54Ch] [ebp-194h]
    float endAngles[32][3]; // [esp+550h] [ebp-190h] BYREF
    bool foundEnd; // [esp+6D3h] [ebp-Dh]
    int clientNum; // [esp+6D4h] [ebp-Ch]
    cachedSnapshot_t *frameEnd; // [esp+6D8h] [ebp-8h]
    int snapOffset; // [esp+6DCh] [ebp-4h]

    rewindDeltaTime = svs.time - gametime;
    startOffset = 50 * ((svs.time - gametime) / 50 + 2);
    endOffset = 50 * ((svs.time - gametime) / 50 + 1);
    frameStart = SV_GetCachedSnapshot(&startOffset);
    frameEnd = SV_GetCachedSnapshot(&endOffset);
    if ( frameStart || frameEnd )
    {
        snapOffset = 1;
        while ( frameEnd && frameEnd->time < gametime )
        {
            frameStart = frameEnd;
            endOffset = 50 * (--snapOffset + rewindDeltaTime / 50);
            frameEnd = SV_GetCachedSnapshot(&endOffset);
        }
        if ( frameEnd && snapOffset != 1 )
            Com_Printf(15, "Adjusted frameEnd by %i snaps\n", 1 - snapOffset);
        snapOffset = 2;
        while ( frameStart && frameStart->time > gametime )
        {
            frameStart = frameEnd;
            startOffset = 50 * (++snapOffset + rewindDeltaTime / 50);
            frameStart = SV_GetCachedSnapshot(&startOffset);
        }
        if ( frameStart && snapOffset != 2 )
            Com_Printf(15, "Adjusted frameStart by %i snaps\n", snapOffset - 2);
        foundStart = frameStart != 0;
        foundEnd = frameEnd != 0;
        memset(startSuccess, 0, sizeof(startSuccess));
        memset(endSuccess, 0, sizeof(endSuccess));
        if ( frameStart )
        {
            if ( frameStart->time > gametime )
            {
                v4 = va("frameStart->time is %i for gametime %i", frameStart->time, gametime);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                2491,
                                0,
                                "%s\n\t%s",
                                "frameStart->time <= gametime",
                                v4) )
                    __debugbreak();
            }
            SV_GetClientPositionsFromCachedSnap(frameStart, startPos, startAngles, startSuccess);
        }
        if ( foundEnd )
        {
            if ( frameEnd->time < gametime )
            {
                v5 = va("frameEnd->time is %i for gametime %i", frameEnd->time, gametime);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                2496,
                                0,
                                "%s\n\t%s",
                                "frameEnd->time >= gametime",
                                v5) )
                    __debugbreak();
            }
            SV_GetClientPositionsFromCachedSnap(frameEnd, endPos, endAngles, endSuccess);
        }
        if ( foundStart && foundEnd )
        {
            if ( startOffset == endOffset )
                progress = 0.0f;
            else
                progress = (float)(gametime - frameStart->time) / (float)(frameEnd->time - frameStart->time);
            if ( progress < 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                            2513,
                            0,
                            "%s\n\t(progress) = %g",
                            "(progress >= 0)",
                            progress) )
            {
                __debugbreak();
            }
            if ( progress > 1.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                            2514,
                            0,
                            "%s\n\t(progress) = %g",
                            "(progress <= 1)",
                            progress) )
            {
                __debugbreak();
            }
        }
        else
        {
            progress = 0.0f;
        }
        for ( clientNum = 0; clientNum < 32; ++clientNum )
        {
            if ( startSuccess[clientNum] && endSuccess[clientNum] )
            {
                if ( frameStart->time > gametime )
                {
                    v7 = va("%i > %i", frameStart->time, gametime);
                    if ( !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                    2527,
                                    0,
                                    "%s\n\t%s",
                                    "frameStart->time <= gametime",
                                    v7) )
                        __debugbreak();
                }
                if ( frameEnd->time < gametime )
                {
                    v8 = va("%i < %i", frameEnd->time, gametime);
                    if ( !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                    2528,
                                    0,
                                    "%s\n\t%s",
                                    "frameEnd->time >= gametime",
                                    v8) )
                        __debugbreak();
                }
                success[clientNum] = 1;
                Vec3Lerp(startPos[clientNum], endPos[clientNum], progress, &(*pos)[3 * clientNum]);
                Vec3Lerp(startAngles[clientNum], endAngles[clientNum], progress, &(*angles)[3 * clientNum]);
                if ( (LODWORD((*pos)[3 * clientNum]) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                2532,
                                0,
                                "%s",
                                "!IS_NAN( pos[clientNum][0] )") )
                {
                    __debugbreak();
                }
                if ( (LODWORD((*pos)[3 * clientNum + 1]) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                2533,
                                0,
                                "%s",
                                "!IS_NAN( pos[clientNum][1] )") )
                {
                    __debugbreak();
                }
                if ( (LODWORD((*pos)[3 * clientNum + 2]) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                2534,
                                0,
                                "%s",
                                "!IS_NAN( pos[clientNum][2] )") )
                {
                    __debugbreak();
                }
                if ( (LODWORD((*angles)[3 * clientNum]) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                2535,
                                0,
                                "%s",
                                "!IS_NAN( angles[clientNum][0] )") )
                {
                    __debugbreak();
                }
                if ( (LODWORD((*angles)[3 * clientNum + 1]) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                2536,
                                0,
                                "%s",
                                "!IS_NAN( angles[clientNum][1] )") )
                {
                    __debugbreak();
                }
                if ( (LODWORD((*angles)[3 * clientNum + 2]) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                2537,
                                0,
                                "%s",
                                "!IS_NAN( angles[clientNum][2] )") )
                {
                    __debugbreak();
                }
            }
            else if ( startSuccess[clientNum] )
            {
                success[clientNum] = 1;
                v15 = &(*pos)[3 * clientNum];
                v16 = startPos[clientNum];
                *v15 = *v16;
                v15[1] = v16[1];
                v15[2] = v16[2];
                v13 = &(*angles)[3 * clientNum];
                v14 = startAngles[clientNum];
                *v13 = *v14;
                v13[1] = v14[1];
                v13[2] = v14[2];
                if ( (LODWORD((*pos)[3 * clientNum]) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                2544,
                                0,
                                "%s",
                                "!IS_NAN( pos[clientNum][0] )") )
                {
                    __debugbreak();
                }
                if ( (LODWORD((*pos)[3 * clientNum + 1]) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                2545,
                                0,
                                "%s",
                                "!IS_NAN( pos[clientNum][1] )") )
                {
                    __debugbreak();
                }
                if ( (LODWORD((*pos)[3 * clientNum + 2]) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                2546,
                                0,
                                "%s",
                                "!IS_NAN( pos[clientNum][2] )") )
                {
                    __debugbreak();
                }
                if ( (LODWORD((*angles)[3 * clientNum]) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                2547,
                                0,
                                "%s",
                                "!IS_NAN( angles[clientNum][0] )") )
                {
                    __debugbreak();
                }
                if ( (LODWORD((*angles)[3 * clientNum + 1]) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                2548,
                                0,
                                "%s",
                                "!IS_NAN( angles[clientNum][1] )") )
                {
                    __debugbreak();
                }
                if ( (LODWORD((*angles)[3 * clientNum + 2]) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                2549,
                                0,
                                "%s",
                                "!IS_NAN( angles[clientNum][2] )") )
                {
                    __debugbreak();
                }
            }
            else if ( endSuccess[clientNum] )
            {
                success[clientNum] = 1;
                v11 = &(*pos)[3 * clientNum];
                v12 = endPos[clientNum];
                *v11 = *v12;
                v11[1] = v12[1];
                v11[2] = v12[2];
                v9 = &(*angles)[3 * clientNum];
                v10 = endAngles[clientNum];
                *v9 = *v10;
                v9[1] = v10[1];
                v9[2] = v10[2];
                if ( (LODWORD((*pos)[3 * clientNum]) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                2556,
                                0,
                                "%s",
                                "!IS_NAN( pos[clientNum][0] )") )
                {
                    __debugbreak();
                }
                if ( (LODWORD((*pos)[3 * clientNum + 1]) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                2557,
                                0,
                                "%s",
                                "!IS_NAN( pos[clientNum][1] )") )
                {
                    __debugbreak();
                }
                if ( (LODWORD((*pos)[3 * clientNum + 2]) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                2558,
                                0,
                                "%s",
                                "!IS_NAN( pos[clientNum][2] )") )
                {
                    __debugbreak();
                }
                if ( (LODWORD((*angles)[3 * clientNum]) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                2559,
                                0,
                                "%s",
                                "!IS_NAN( angles[clientNum][0] )") )
                {
                    __debugbreak();
                }
                if ( (LODWORD((*angles)[3 * clientNum + 1]) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                2560,
                                0,
                                "%s",
                                "!IS_NAN( angles[clientNum][1] )") )
                {
                    __debugbreak();
                }
                if ( (LODWORD((*angles)[3 * clientNum + 2]) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                2561,
                                0,
                                "%s",
                                "!IS_NAN( angles[clientNum][2] )") )
                {
                    __debugbreak();
                }
            }
        }
        return 1;
    }
    else
    {
        Com_Printf(15, "Failed to get a cached snapshot for antilag - offset is %i\n", rewindDeltaTime);
        return 0;
    }
}

cachedSnapshot_t *__cdecl SV_GetCachedSnapshot(int *pArchiveTime)
{
    int archivedFrame; // [esp+4h] [ebp-8h]
    cachedSnapshot_t *cachedFrame; // [esp+8h] [ebp-4h]

    if ( !SV_Loaded()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    2346,
                    0,
                    "%s",
                    "SV_Loaded()") )
    {
        __debugbreak();
    }
    if ( !sv_fps->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    2347,
                    0,
                    "%s",
                    "sv_fps->current.integer") )
    {
        __debugbreak();
    }
    if ( *pArchiveTime <= 0 )
        return 0;
    archivedFrame = svs.nextArchivedSnapshotFrames - sv_fps->current.integer * *pArchiveTime / 1000;
    if ( archivedFrame < svs.nextArchivedSnapshotFrames - 1200 )
    {
        archivedFrame = svs.nextArchivedSnapshotFrames - 1200;
        *pArchiveTime = 1000
                                    * (svs.nextArchivedSnapshotFrames - (svs.nextArchivedSnapshotFrames - 1200))
                                    / sv_fps->current.integer;
    }
    if ( archivedFrame < 0 )
    {
        archivedFrame = 0;
        *pArchiveTime = 1000 * svs.nextArchivedSnapshotFrames / sv_fps->current.integer;
    }
    while ( archivedFrame < svs.nextArchivedSnapshotFrames )
    {
        cachedFrame = SV_GetCachedSnapshotInternal(archivedFrame, 0, 1);
        if ( cachedFrame )
            return cachedFrame;
        ++archivedFrame;
        *pArchiveTime = 50 * (svs.nextArchivedSnapshotFrames - archivedFrame);
    }
    *pArchiveTime = 0;
    return 0;
}

cachedSnapshot_t *__cdecl SV_GetCachedSnapshotInternal(int archivedFrame, int depth, bool expectedToSucceed)
{
    int v4; // edx
    int v7; // eax
    int v8; // eax
    int v9; // eax
    int v10; // eax
    int v11; // eax
    archivedSnapshot_s *frame; // [esp+0h] [ebp-68h]
    archivedSnapshot_s *framea; // [esp+0h] [ebp-68h]
    int oldArchivedFrame; // [esp+4h] [ebp-64h]
    int newnum; // [esp+8h] [ebp-60h]
    int newnuma; // [esp+8h] [ebp-60h]
    int newnumb; // [esp+8h] [ebp-60h]
    int newnumc; // [esp+8h] [ebp-60h]
    int oldindex; // [esp+14h] [ebp-54h]
    int oldnum; // [esp+18h] [ebp-50h]
    msg_t msg; // [esp+1Ch] [ebp-4Ch] BYREF
    cachedSnapshot_t *cachedFrame; // [esp+4Ch] [ebp-1Ch]
    cachedClient_s *oldCachedClient; // [esp+50h] [ebp-18h]
    int partSize; // [esp+54h] [ebp-14h]
    int firstCachedSnapshotFrame; // [esp+58h] [ebp-10h]
    int i; // [esp+5Ch] [ebp-Ch]
    cachedSnapshot_t *oldCachedFrame; // [esp+60h] [ebp-8h]
    cachedClient_s *cachedClient; // [esp+64h] [ebp-4h]

    frame = &svs.archivedSnapshotFrames[archivedFrame % 1200];
    if (!frame->size
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
            1970,
            0,
            "%s\n\t(archivedFrame) = %i",
            "(frame->size)",
            archivedFrame))
    {
        __debugbreak();
    }
    if (!SV_FrameIsStillInArchivedSnapshotBuffer(frame->start))
    {
        if (expectedToSucceed)
            Com_Printf(
                15,
                "Failed to get archived snapshot for archived frame %i - frame->start is too old - %i < %i - %i\n",
                archivedFrame,
                frame->start,
                svs.nextArchivedSnapshotBuffer,
                0x1000000);
        return 0;
    }
    firstCachedSnapshotFrame = svs.nextCachedSnapshotFrames - 512;
    if (svs.nextCachedSnapshotFrames - 512 < 0)
        firstCachedSnapshotFrame = 0;
    for (i = svs.nextCachedSnapshotFrames - 1; i >= firstCachedSnapshotFrame; --i)
    {
        cachedFrame = &svs.cachedSnapshotFrames[i % 512];
        if (cachedFrame->archivedFrame == archivedFrame)
        {
            if (cachedFrame->first_entity < 0
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    1988,
                    0,
                    "%s\n\t(cachedFrame->first_entity) = %i",
                    "(cachedFrame->first_entity >= 0)",
                    cachedFrame->first_entity))
            {
                __debugbreak();
            }
            if (cachedFrame->matchState > depth + svs.nextCachedSnapshotMatchStates - svs.numCachedSnapshotMatchStates
                && cachedFrame->first_entity > 160 * depth + svs.nextCachedSnapshotEntities - svs.numCachedSnapshotEntities
                && cachedFrame->first_client > 32 * depth + svs.nextCachedSnapshotClients - svs.numCachedSnapshotClients)
            {
                return cachedFrame;
            }
            break;
        }
    }
    v4 = frame->start % 0x1000000;
    partSize = 0x1000000 - v4;
    if (frame->size > 0x1000000 - v4)
        MSG_InitReadOnlySplit(
            &msg,
            &svs.archivedSnapshotBuffer[v4],
            partSize,
            svs.archivedSnapshotBuffer,
            frame->size - partSize);
    else
        MSG_InitReadOnly(&msg, &svs.archivedSnapshotBuffer[v4], frame->size);
    MSG_BeginReading(&msg);
    if (MSG_ReadBit(&msg))
    {
        if (msg.overflowed
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                2226,
                0,
                "%s",
                "!msg.overflowed"))
        {
            __debugbreak();
        }
        cachedFrame = &svs.cachedSnapshotFrames[svs.nextCachedSnapshotFrames % 512];
        cachedFrame->archivedFrame = archivedFrame;
        cachedFrame->matchState = svs.nextCachedSnapshotMatchStates;
        cachedFrame->num_entities = 0;
        cachedFrame->first_entity = svs.nextCachedSnapshotEntities;
        cachedFrame->num_clients = 0;
        cachedFrame->first_client = svs.nextCachedSnapshotClients;
        cachedFrame->usesDelta = 0;
        v9 = MSG_ReadLong(&msg);
        cachedFrame->time = v9;
        v10 = MSG_ReadLong(&msg);
        cachedFrame->physicsTime = v10;
        MSG_ClearLastReferencedEntity(&msg);
        MSG_ReadDeltaMatchState(
            &msg,
            cachedFrame->time,
            0,
            &svs.cachedSnapshotMatchStates[svs.nextCachedSnapshotMatchStates % svs.numCachedSnapshotMatchStates]);
        ++svs.nextCachedSnapshotMatchStates;
        MSG_ClearLastReferencedEntity(&msg);
        while (MSG_ReadBit(&msg))
        {
            newnumb = MSG_ReadEntityIndex(&msg, 5);
            if (msg.overflowed)
                Com_Error(ERR_DROP, "SV_GetCachedSnapshot: end of message");
            cachedClient = &svs.cachedSnapshotClients[svs.nextCachedSnapshotClients % svs.numCachedSnapshotClients];
            MSG_ReadDeltaClient(&msg, cachedFrame->time, 0, &cachedClient->cs, newnumb);
            v11 = MSG_ReadBit(&msg);
            cachedClient->playerStateExists = v11;
            if (cachedClient->playerStateExists)
                MSG_ReadDeltaPlayerstate(0, &msg, cachedFrame->time, 0, &cachedClient->ps, 0);
            if (svsHeaderValid
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    2273,
                    0,
                    "%s",
                    "!svsHeaderValid"))
            {
                __debugbreak();
            }
            if (++svs.nextCachedSnapshotClients >= 2147483646)
                Com_Error(ERR_FATAL, "svs.nextCachedSnapshotClients wrapped");
            ++cachedFrame->num_clients;
        }
        MSG_ClearLastReferencedEntity(&msg);
        while (1)
        {
            newnumc = MSG_ReadEntityIndex(&msg, 10);
            if (newnumc == 1023)
                break;
            if (msg.overflowed)
                Com_Error(ERR_DROP, "SV_GetCachedSnapshot: end of message");
            MSG_ReadDeltaArchivedEntity(
                &msg,
                cachedFrame->time,
                &sv.svEntities[newnumc].baseline,
                &svs.cachedSnapshotEntities[svs.nextCachedSnapshotEntities % svs.numCachedSnapshotEntities],
                newnumc);
            if (++svs.nextCachedSnapshotEntities >= 2147483646)
                Com_Error(ERR_FATAL, "svs.nextCachedSnapshotEntities wrapped");
            ++cachedFrame->num_entities;
        }
        if (++svs.nextCachedSnapshotFrames >= 2147483646)
            Com_Error(ERR_FATAL, "svs.nextCachedSnapshotFrames wrapped");
    }
    else
    {
        if (msg.overflowed
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                2035,
                0,
                "%s",
                "!msg.overflowed"))
        {
            __debugbreak();
        }
        oldArchivedFrame = MSG_ReadLong(&msg);
        if (oldArchivedFrame < svs.nextArchivedSnapshotFrames - 1200)
        {
            if (expectedToSucceed)
                Com_Printf(
                    15,
                    "getting archive snapshot failed for time %i - oldArchiveFrame(%i) < svs.nextArchivedSnapshotFrames(%i) - NUM_A"
                    "RCHIVED_FRAMES(%i)\n",
                    archivedFrame,
                    oldArchivedFrame,
                    svs.nextArchivedSnapshotFrames,
                    1200);
            return 0;
        }
        framea = &svs.archivedSnapshotFrames[oldArchivedFrame % 1200];
        if (!SV_FrameIsStillInArchivedSnapshotBuffer(framea->start))
        {
            if (expectedToSucceed)
                Com_Printf(
                    15,
                    "getting archive snapshot failed for time %i - frame->start(%i) < svs.nextArchivedSnapshotBuffer(%i) - ARCHIVED"
                    "_SNAPSHOT_BUFFER_SIZE(%i)\n",
                    archivedFrame,
                    framea->start,
                    svs.nextArchivedSnapshotBuffer,
                    0x1000000);
            return 0;
        }
        oldCachedFrame = SV_GetCachedSnapshotInternal(oldArchivedFrame, depth + 1, expectedToSucceed);
        if (!oldCachedFrame)
        {
            if (expectedToSucceed)
                Com_Printf(
                    15,
                    "failed to get snapshot for time %i - it was delta'd off time %i, and we couldn't get that snapshot\n",
                    archivedFrame,
                    oldArchivedFrame);
            return 0;
        }
        if (oldCachedFrame->usesDelta
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                2061,
                0,
                "%s",
                "!oldCachedFrame->usesDelta"))
        {
            __debugbreak();
        }
        cachedFrame = &svs.cachedSnapshotFrames[svs.nextCachedSnapshotFrames % 512];
        cachedFrame->archivedFrame = archivedFrame;
        cachedFrame->matchState = svs.nextCachedSnapshotMatchStates;
        cachedFrame->num_entities = 0;
        cachedFrame->first_entity = svs.nextCachedSnapshotEntities;
        cachedFrame->num_clients = 0;
        cachedFrame->first_client = svs.nextCachedSnapshotClients;
        cachedFrame->usesDelta = 1;
        cachedFrame->time = MSG_ReadLong(&msg);
        cachedFrame->physicsTime = MSG_ReadLong(&msg);
        MSG_ClearLastReferencedEntity(&msg);
        MSG_ReadDeltaMatchState(
            &msg,
            cachedFrame->time,
            &svs.cachedSnapshotMatchStates[oldCachedFrame->matchState % svs.numCachedSnapshotMatchStates],
            &svs.cachedSnapshotMatchStates[svs.nextCachedSnapshotMatchStates % svs.numCachedSnapshotMatchStates]);
        ++svs.nextCachedSnapshotMatchStates;
        MSG_ClearLastReferencedEntity(&msg);
        oldindex = 0;
        oldCachedClient = 0;
        if (oldCachedFrame->num_clients > 0)
        {
            oldCachedClient = &svs.cachedSnapshotClients[oldCachedFrame->first_client % svs.numCachedSnapshotClients];
            oldnum = oldCachedClient->cs.clientIndex;
        }
        else
        {
            oldnum = 99999;
        }
        while (MSG_ReadBit(&msg))
        {
            newnum = MSG_ReadEntityIndex(&msg, 5);
            if (newnum < 0
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    2113,
                    0,
                    "%s\n\t(newnum) = %i",
                    "(newnum >= 0)",
                    newnum))
            {
                __debugbreak();
            }
            if (msg.overflowed)
                Com_Error(ERR_DROP, "SV_GetCachedSnapshot: end of message");
            while (oldnum < newnum)
            {
                if (++oldindex < oldCachedFrame->num_clients)
                {
                    oldCachedClient = &svs.cachedSnapshotClients[(oldindex + oldCachedFrame->first_client)
                        % svs.numCachedSnapshotClients];
                    oldnum = oldCachedClient->cs.clientIndex;
                }
                else
                {
                    oldnum = 99999;
                }
            }
            if (oldnum == newnum)
            {
                cachedClient = &svs.cachedSnapshotClients[svs.nextCachedSnapshotClients % svs.numCachedSnapshotClients];
                if (cachedClient == oldCachedClient
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                        2137,
                        0,
                        "%s",
                        "cachedClient != oldCachedClient"))
                {
                    __debugbreak();
                }
                MSG_ReadDeltaClient(&msg, cachedFrame->time, &oldCachedClient->cs, &cachedClient->cs, newnum);
                v7 = MSG_ReadBit(&msg);
                cachedClient->playerStateExists = v7;
                if (cachedClient->playerStateExists)
                {
                    if (oldCachedClient->playerStateExists)
                        MSG_ReadDeltaPlayerstate(0, &msg, cachedFrame->time, &oldCachedClient->ps, &cachedClient->ps, 0);
                    else
                        MSG_ReadDeltaPlayerstate(0, &msg, cachedFrame->time, 0, &cachedClient->ps, 0);
                }
                if (svsHeaderValid
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                        2148,
                        0,
                        "%s",
                        "!svsHeaderValid"))
                {
                    __debugbreak();
                }
                if (++svs.nextCachedSnapshotClients >= 2147483646)
                    Com_Error(ERR_FATAL, "svs.nextCachedSnapshotClients wrapped");
                ++cachedFrame->num_clients;
                if (++oldindex < oldCachedFrame->num_clients)
                {
                    oldCachedClient = &svs.cachedSnapshotClients[(oldindex + oldCachedFrame->first_client)
                        % svs.numCachedSnapshotClients];
                    oldnum = oldCachedClient->cs.clientIndex;
                }
                else
                {
                    oldnum = 99999;
                }
            }
            else
            {
                if (oldnum <= newnum
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                        2169,
                        0,
                        "%s",
                        "oldnum > newnum"))
                {
                    __debugbreak();
                }
                cachedClient = &svs.cachedSnapshotClients[svs.nextCachedSnapshotClients % svs.numCachedSnapshotClients];
                MSG_ReadDeltaClient(&msg, cachedFrame->time, 0, &cachedClient->cs, newnum);
                v8 = MSG_ReadBit(&msg);
                cachedClient->playerStateExists = v8;
                if (cachedClient->playerStateExists)
                    MSG_ReadDeltaPlayerstate(0, &msg, cachedFrame->time, 0, &cachedClient->ps, 0);
                if (svsHeaderValid
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                        2178,
                        0,
                        "%s",
                        "!svsHeaderValid"))
                {
                    __debugbreak();
                }
                if (++svs.nextCachedSnapshotClients >= 2147483646)
                    Com_Error(ERR_FATAL, "svs.nextCachedSnapshotClients wrapped");
                ++cachedFrame->num_clients;
            }
        }
        MSG_ClearLastReferencedEntity(&msg);
        while (1)
        {
            newnuma = MSG_ReadEntityIndex(&msg, 10);
            if (newnuma == 1023)
                break;
            if (msg.overflowed)
                Com_Error(ERR_DROP, "SV_GetCachedSnapshot: end of message");
            MSG_ReadDeltaArchivedEntity(
                &msg,
                cachedFrame->time,
                &sv.svEntities[newnuma].baseline,
                &svs.cachedSnapshotEntities[svs.nextCachedSnapshotEntities % svs.numCachedSnapshotEntities],
                newnuma);
            if (++svs.nextCachedSnapshotEntities >= 2147483646)
                Com_Error(ERR_FATAL, "svs.nextCachedSnapshotEntities wrapped");
            ++cachedFrame->num_entities;
        }
        if (++svs.nextCachedSnapshotFrames >= 2147483646)
            Com_Error(ERR_FATAL, "svs.nextCachedSnapshotFrames wrapped");
    }
    if (cachedFrame->num_entities >= svs.numCachedSnapshotEntities
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
            2324,
            1,
            "%s\n\t(cachedFrame->num_entities) = %i",
            "(cachedFrame->num_entities < svs.numCachedSnapshotEntities)",
            cachedFrame->num_entities))
    {
        __debugbreak();
    }
    if (cachedFrame->num_clients >= svs.numCachedSnapshotClients
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
            2325,
            1,
            "%s\n\t(cachedFrame->num_clients) = %i",
            "(cachedFrame->num_clients < svs.numCachedSnapshotClients)",
            cachedFrame->num_clients))
    {
        __debugbreak();
    }
    if (cachedFrame->matchState < svs.nextCachedSnapshotMatchStates - svs.numCachedSnapshotMatchStates
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
            2326,
            1,
            "%s",
            "cachedFrame->matchState >= svs.nextCachedSnapshotMatchStates - svs.numCachedSnapshotMatchStates"))
    {
        __debugbreak();
    }
    if (cachedFrame->first_entity < svs.nextCachedSnapshotEntities - svs.numCachedSnapshotEntities
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
            2327,
            1,
            "%s",
            "cachedFrame->first_entity >= svs.nextCachedSnapshotEntities - svs.numCachedSnapshotEntities"))
    {
        __debugbreak();
    }
    if (cachedFrame->first_client < svs.nextCachedSnapshotClients - svs.numCachedSnapshotClients
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
            2328,
            1,
            "%s",
            "cachedFrame->first_client >= svs.nextCachedSnapshotClients - svs.numCachedSnapshotClients"))
    {
        __debugbreak();
    }
    return cachedFrame;
}

bool __cdecl SV_FrameIsStillInArchivedSnapshotBuffer(int frameStart)
{
    int bufferStart; // [esp+4h] [ebp-4h]

    if ( frameStart < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    1760,
                    0,
                    "%s",
                    "frameStart >= 0") )
    {
        __debugbreak();
    }
    bufferStart = svs.nextArchivedSnapshotBuffer - 0x1000000;
    if ( svs.nextArchivedSnapshotBuffer - 0x1000000 < 0 )
        return frameStart < svs.nextArchivedSnapshotBuffer || frameStart >= bufferStart + 0x28000000;
    return frameStart >= bufferStart && frameStart < svs.nextArchivedSnapshotBuffer;
}

void __cdecl SV_GetClientPositionsFromCachedSnap(
                const cachedSnapshot_t *cachedFrame,
                float (*pos)[3],
                float (*angles)[3],
                bool *success)
{
    const archivedEntity_s *cachedEnt; // [esp+0h] [ebp-Ch]
    unsigned int i; // [esp+4h] [ebp-8h]
    int clientNum; // [esp+8h] [ebp-4h]

    for ( i = 0; i < cachedFrame->num_entities; ++i )
    {
        if ( cachedFrame->first_entity < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                        2408,
                        0,
                        "%s\n\t(cachedFrame->first_entity) = %i",
                        "(cachedFrame->first_entity >= 0)",
                        cachedFrame->first_entity) )
        {
            __debugbreak();
        }
        cachedEnt = &svs.cachedSnapshotEntities[(i + cachedFrame->first_entity) % svs.numCachedSnapshotEntities];
        clientNum = cachedEnt->s.number;
        if ( cachedEnt->s.number >= 32 )
            break;
        success[clientNum] = 1;
        BG_EvaluateTrajectory(&cachedEnt->s.lerp.pos, cachedFrame->time, &(*pos)[3 * clientNum]);
        (*angles)[3 * clientNum] = 0.0f;
        (*angles)[3 * clientNum + 1] = cachedEnt->s.lerp.apos.trBase[1];
        (*angles)[3 * clientNum + 2] = 0.0f;
    }
}

int __cdecl SV_GetEarliestArchivedClientInfoTime()
{
    bool HasCachedSnapshotInternal; // esi
    int archivedFrame; // [esp+4h] [ebp-4h]

    if ( !SV_Loaded()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    2578,
                    0,
                    "%s",
                    "SV_Loaded()") )
    {
        __debugbreak();
    }
    archivedFrame = svs.nextArchivedSnapshotFrames - 1200;
    if ( svs.nextArchivedSnapshotFrames - 1200 < 0 )
        archivedFrame = 0;
    while ( archivedFrame < svs.nextArchivedSnapshotFrames )
    {
        HasCachedSnapshotInternal = SV_HasCachedSnapshotInternal(archivedFrame, 0);
        if ( HasCachedSnapshotInternal != (SV_GetCachedSnapshotInternal(archivedFrame, 0, 0) != 0)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                        2585,
                        0,
                        "%s",
                        "SV_HasCachedSnapshotInternal( archivedFrame, 0 ) == (SV_GetCachedSnapshotInternal( archivedFrame, 0, false ) != 0)") )
        {
            __debugbreak();
        }
        if ( SV_HasCachedSnapshotInternal(archivedFrame, 0) )
            break;
        ++archivedFrame;
    }
    svs.oldestArchiveTime = 50 * (svs.nextArchivedSnapshotFrames - archivedFrame);
    return 50 * (svs.nextArchivedSnapshotFrames - archivedFrame);
}

bool __cdecl SV_HasCachedSnapshotInternal(int archivedFrame, int callDepth)
{
    int v3; // edx
    bool HasCachedSnapshotInternal; // esi
    archivedSnapshot_s *frame; // [esp+4h] [ebp-4Ch]
    int oldArchivedFrame; // [esp+8h] [ebp-48h]
    msg_t msg; // [esp+10h] [ebp-40h] BYREF
    cachedSnapshot_t *cachedFrame; // [esp+40h] [ebp-10h]
    int partSize; // [esp+44h] [ebp-Ch]
    int firstCachedSnapshotFrame; // [esp+48h] [ebp-8h]
    int i; // [esp+4Ch] [ebp-4h]

    frame = &svs.archivedSnapshotFrames[archivedFrame % 1200];
    if ( !SV_FrameIsStillInArchivedSnapshotBuffer(frame->start) )
        return 0;
    firstCachedSnapshotFrame = svs.nextCachedSnapshotFrames - 512;
    if ( svs.nextCachedSnapshotFrames - 512 < 0 )
        firstCachedSnapshotFrame = 0;
    for ( i = svs.nextCachedSnapshotFrames - 1; i >= firstCachedSnapshotFrame; --i )
    {
        cachedFrame = &svs.cachedSnapshotFrames[i % 512];
        if ( cachedFrame->archivedFrame == archivedFrame )
        {
            if ( cachedFrame->first_entity < 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                            1882,
                            0,
                            "%s\n\t(cachedFrame->first_entity) = %i",
                            "(cachedFrame->first_entity >= 0)",
                            cachedFrame->first_entity) )
            {
                __debugbreak();
            }
            if ( cachedFrame->matchState > callDepth + svs.nextCachedSnapshotMatchStates - svs.numCachedSnapshotMatchStates
                && cachedFrame->first_entity >= 160 * callDepth + svs.nextCachedSnapshotEntities - svs.numCachedSnapshotEntities
                && cachedFrame->first_client >= 32 * callDepth + svs.nextCachedSnapshotClients - svs.numCachedSnapshotClients )
            {
                return 1;
            }
            break;
        }
    }
    v3 = frame->start % 0x1000000;
    partSize = 0x1000000 - v3;
    if (frame->size > 0x1000000 - v3)
        MSG_InitReadOnlySplit(
            &msg,
            &svs.archivedSnapshotBuffer[v3],
            partSize,
            svs.archivedSnapshotBuffer,
            frame->size - partSize);
    else
        MSG_InitReadOnly(&msg, &svs.archivedSnapshotBuffer[v3], frame->size);
    MSG_BeginReading(&msg);
    if ( MSG_ReadBit(&msg) )
        return 1;
    oldArchivedFrame = MSG_ReadLong(&msg);
    if ( oldArchivedFrame < svs.nextArchivedSnapshotFrames - 1200 )
        return 0;
    if ( !SV_FrameIsStillInArchivedSnapshotBuffer(svs.archivedSnapshotFrames[oldArchivedFrame % 1200].start) )
        return 0;
    HasCachedSnapshotInternal = SV_HasCachedSnapshotInternal(oldArchivedFrame, callDepth + 1);
    if ( HasCachedSnapshotInternal != (SV_GetCachedSnapshotInternal(oldArchivedFrame, callDepth + 1, 0) != 0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    1937,
                    0,
                    "%s",
                    "SV_HasCachedSnapshotInternal( oldArchivedFrame, callDepth + 1 ) == (SV_GetCachedSnapshotInternal( oldArchivedF"
                    "rame, callDepth + 1, false ) != 0)") )
    {
        __debugbreak();
    }
    return SV_HasCachedSnapshotInternal(oldArchivedFrame, callDepth + 1);
}

int __cdecl SV_GetArchivedClientInfo(
                unsigned int clientNum,
                int *pArchiveTime,
                playerState_s *ps,
                clientState_s *cs,
                float *origin,
                int *health,
                int *otherFlags)
{
    float *v9; // [esp+0h] [ebp-274Ch]
    playerState_s v10; // [esp+Ch] [ebp-2740h] BYREF
    cachedSnapshot_t *CachedSnapshot; // [esp+273Ch] [ebp-10h]
    unsigned int i; // [esp+2740h] [ebp-Ch]
    int v13; // [esp+2744h] [ebp-8h]
    cachedClient_s *v14; // [esp+2748h] [ebp-4h]
    int savedregs; // [esp+274Ch] [ebp+0h] BYREF

    if (!health
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp", 2610, 0, "%s", "health"))
    {
        __debugbreak();
    }
    if (!otherFlags
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
            2611,
            0,
            "%s",
            "otherFlags"))
    {
        __debugbreak();
    }
    CachedSnapshot = SV_GetCachedSnapshot(pArchiveTime);
    if (CachedSnapshot)
    {
        if (*pArchiveTime <= 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                2639,
                0,
                "%s",
                "*pArchiveTime > 0"))
        {
            __debugbreak();
        }
        v13 = svs.time - CachedSnapshot->time;
        v14 = 0;
        for (i = 0; ; ++i)
        {
            if (i >= CachedSnapshot->num_clients)
                return 0;
            v14 = &svs.cachedSnapshotClients[(i + CachedSnapshot->first_client) % svs.numCachedSnapshotClients];
            if (v14->cs.clientIndex == clientNum)
                break;
        }
        if (!v14->playerStateExists)
            return 0;
        if (!v14
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                2660,
                0,
                "%s",
                "cachedClient"))
        {
            __debugbreak();
        }
        if ((v14->ps.otherFlags & 4) == 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                2661,
                0,
                "%s",
                "cachedClient->ps.otherFlags & POF_PLAYER"))
        {
            __debugbreak();
        }
        if (origin)
        {
            v9 = v14->ps.origin;
            *origin = v14->ps.origin[0];
            origin[1] = v9[1];
            origin[2] = v9[2];
        }
        if (cs)
            Com_Memcpy(cs, &v14->cs, 208);
        *health = v14->ps.stats[0];
        *otherFlags = v14->ps.otherFlags;
        if (ps)
        {
            Com_Memcpy(ps, &v14->ps, 9892);
            if (ps->commandTime)
                ps->commandTime += v13;
            if (ps->pm_time)
                ps->pm_time += v13;
            if (ps->foliageSoundTime)
                ps->foliageSoundTime += v13;
            if (ps->jumpTime)
                ps->jumpTime += v13;
            if (ps->viewHeightLerpTime)
                ps->viewHeightLerpTime += v13;
            if (ps->shellshockTime)
                ps->shellshockTime += v13;
            for (i = 0; i < 0x1F && ps->hud.archival[i].type; ++i)
            {
                if (ps->hud.archival[i].time)
                    ps->hud.archival[i].time += v13;
                if (ps->hud.archival[i].fadeStartTime)
                {
                    ps->hud.archival[i].fadeStartTime += v13;
                    if (ps->hud.archival[i].fadeStartTime > svs.time
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                            2703,
                            0,
                            "ps->hud.archival[i].fadeStartTime <= svs.time\n\t%i, %i",
                            ps->hud.archival[i].fadeStartTime,
                            svs.time))
                    {
                        __debugbreak();
                    }
                }
                if (ps->hud.archival[i].scaleStartTime)
                    ps->hud.archival[i].scaleStartTime += v13;
                if (ps->hud.archival[i].moveStartTime)
                    ps->hud.archival[i].moveStartTime += v13;
            }
            ps->deltaTime += v13;
        }
        return 1;
    }
    else
    {
        // LWSS: this is likely some 128bit aligned alloca
        //if ((((unsigned __int8)&v10 + ((0x80 - (((unsigned __int8)&savedregs - 64) & 0x7F)) & 0x7F)) & 0x7F) != 0
        //    && !Assert_MyHandler(
        //        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
        //        2622,
        //        0,
        //        "%s",
        //        "( (unsigned int)backupPs & (128-1) ) == 0"))
        //{
        //    __debugbreak();
        //}
        if (!ps)
        {
            //ps = (playerState_s *)((char *)&v10 + ((0x80 - (((unsigned __int8)&savedregs - 64) & 0x7F)) & 0x7F));
            ps = &v10;
        }
        if (!ps
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp", 2627, 0, "%s", "ps"))
        {
            __debugbreak();
        }
        if (*pArchiveTime <= 0 && SV_GetCurrentClientInfo(clientNum, ps, cs))
        {
            *health = ps->stats[0];
            *otherFlags = ps->otherFlags;
            if (origin)
            {
                *origin = ps->origin[0];
                origin[1] = ps->origin[1];
                origin[2] = ps->origin[2];
            }
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

int __cdecl SV_GetCurrentClientInfo(unsigned int clientNum, playerState_s *ps, clientState_s *cs)
{
    if ( clientNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    2389,
                    0,
                    "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    clientNum,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    if ( svs.clients[clientNum].header.state != CS_ACTIVE )
        return 0;
    if ( !GetFollowPlayerState(clientNum, ps) )
        return 0;
    memcpy(cs, G_GetClientState(clientNum), sizeof(clientState_s));
    return 1;
}

void __cdecl SV_DecToHex(int decValue, char *hexValue, int hexLength)
{
    int i; // [esp+0h] [ebp-4h]

    if ( decValue >= 1 << (4 * hexLength)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    2722,
                    0,
                    "%s",
                    "decValue < ( 1 << ( hexLength * 4 ) )") )
    {
        __debugbreak();
    }
    if ( decValue < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    2723,
                    0,
                    "%s",
                    "decValue >= 0") )
    {
        __debugbreak();
    }
    for ( i = hexLength - 1; i >= 0; --i )
    {
        if ( decValue )
        {
            hexValue[i] = (decValue & 0xF) + 48;
            if ( hexValue[i] > 57 )
                hexValue[i] += 7;
            decValue >>= 4;
        }
        else
        {
            hexValue[i] = 48;
        }
    }
}

int __cdecl SV_AppendToBuffer(char *msgBuffer, int *msgBufferIndex, char *data, int dataSize)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < dataSize; ++i )
        msgBuffer[(*msgBufferIndex)++] = data[i];
    return 1;
}

int __cdecl SV_AddModifiedStatsWithinOffset(
                unsigned int clientNum,
                int startOffset,
                int *endOffset,
                char *msgBuff,
                int *msgBuffIndex)
{
    int i; // [esp+4h] [ebp-20h]
    int byteSize; // [esp+8h] [ebp-1Ch]
    char *buffer; // [esp+Ch] [ebp-18h]
    char buffHexByte[4]; // [esp+10h] [ebp-14h] BYREF
    char buffHexShort[7]; // [esp+14h] [ebp-10h] BYREF
    char buffHexCharacter; // [esp+1Bh] [ebp-9h] BYREF
    int isMsgWithinMaxSize; // [esp+1Ch] [ebp-8h]
    unsigned int value; // [esp+20h] [ebp-4h] BYREF

    if ( startOffset > *endOffset
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    2764,
                    0,
                    "%s",
                    "startOffset <= endOffset") )
    {
        __debugbreak();
    }
    if ( clientNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    2765,
                    0,
                    "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    clientNum,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    byteSize = *endOffset - startOffset + 1;
    isMsgWithinMaxSize = 1;
    if ( byteSize > 16
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    2770,
                    0,
                    "%s",
                    "byteSize <= SV_STAT_MAX_RUNNING_BLOCK_SIZE") )
    {
        __debugbreak();
    }
    if ( *msgBuffIndex + 2 * byteSize + 5 >= 64 )
    {
        if ( *msgBuffIndex + 5 >= 64 )
            return 0;
        byteSize = (64 - *msgBuffIndex - 5) / 2;
        if ( byteSize <= 0 )
            return 0;
        *endOffset = startOffset + byteSize - 1;
        isMsgWithinMaxSize = 0;
    }
    if ( startOffset > *endOffset
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    2794,
                    0,
                    "%s",
                    "startOffset <= endOffset") )
    {
        __debugbreak();
    }
    if ( *msgBuffIndex + 2 * byteSize + 5 > 64
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    2795,
                    0,
                    "%s",
                    "( msgBuffIndex + SV_STAT_MSG_HEADER_SIZE + ( byteSize * 2 ) ) <= SV_STAT_MSG_SIZE") )
    {
        __debugbreak();
    }
    if ( xblive_basictraining->current.enabled )
        buffer = (char *)svs.clients[clientNum].stats;
    else
        buffer = (char *)svs.clients[clientNum].globalStats;
    SV_DecToHex(startOffset, buffHexShort, 4);
    SV_AppendToBuffer(msgBuff, msgBuffIndex, buffHexShort, 4);
    SV_DecToHex(byteSize - 1, &buffHexCharacter, 1);
    SV_AppendToBuffer(msgBuff, msgBuffIndex, &buffHexCharacter, 1);
    for ( i = startOffset; i <= *endOffset; ++i )
    {
        DDL_GetBits(buffer, 8 * i, 8u, &value);
        SV_DecToHex(value, buffHexByte, 2);
        SV_AppendToBuffer(msgBuff, msgBuffIndex, buffHexByte, 2);
        svs.clients[clientNum].modifiedStatBytes[i >> 3] &= ~(1 << (i & 7));
    }
    return isMsgWithinMaxSize;
}

void __cdecl SV_AddModifiedStats(unsigned int clientNum)
{
    int j; // [esp+0h] [ebp-6Ch]
    unsigned __int8 *dirtyByte; // [esp+4h] [ebp-68h]
    int startOffset; // [esp+8h] [ebp-64h]
    char msgBuff[72]; // [esp+Ch] [ebp-60h] BYREF
    int isMessageFull; // [esp+58h] [ebp-14h]
    int endOffset; // [esp+5Ch] [ebp-10h] BYREF
    int i; // [esp+60h] [ebp-Ch]
    int msgBuffIndex; // [esp+64h] [ebp-8h] BYREF
    int isRunningStatBlock; // [esp+68h] [ebp-4h]

    if ( clientNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    2835,
                    0,
                    "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    clientNum,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    msgBuffIndex = 0;
    isMessageFull = 0;
    if ( svs.clients[clientNum].statsModified )
    {
        isRunningStatBlock = 0;
        startOffset = 0;
        endOffset = 0;
        memset(msgBuff, 0, 0x41u);
        if ( svs.clients[clientNum].statsSentIndex >= 5021
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                        2850,
                        0,
                        "%s",
                        "svs.clients[ clientNum ].statsSentIndex < MODIFIED_STATS_BYTE_SIZE") )
        {
            __debugbreak();
        }
        if ( svs.clients[clientNum].statsSentIndex >= 5021 )
            svs.clients[clientNum].statsSentIndex = 0;
        for ( i = svs.clients[clientNum].statsSentIndex; i < 5021; ++i )
        {
            dirtyByte = &svs.clients[clientNum].modifiedStatBytes[i];
            if ( *dirtyByte )
            {
                for ( j = 0; j < 8; ++j )
                {
                    if ( ((1 << j) & *dirtyByte) != 0 )
                    {
                        if ( isRunningStatBlock )
                        {
                            endOffset = j & 7 | (8 * i);
                            if ( endOffset - startOffset == 15 )
                            {
                                if ( !SV_AddModifiedStatsWithinOffset(clientNum, startOffset, &endOffset, msgBuff, &msgBuffIndex) )
                                {
                                    isMessageFull = 1;
                                    break;
                                }
                                isRunningStatBlock = 0;
                            }
                        }
                        else
                        {
                            startOffset = j & 7 | (8 * i);
                            endOffset = startOffset;
                            isRunningStatBlock = 1;
                        }
                    }
                    else if ( isRunningStatBlock )
                    {
                        if ( !SV_AddModifiedStatsWithinOffset(clientNum, startOffset, &endOffset, msgBuff, &msgBuffIndex) )
                        {
                            isMessageFull = 1;
                            break;
                        }
                        isRunningStatBlock = 0;
                    }
                }
                if ( isMessageFull )
                    break;
            }
            else if ( isRunningStatBlock )
            {
                if ( !SV_AddModifiedStatsWithinOffset(clientNum, startOffset, &endOffset, msgBuff, &msgBuffIndex) )
                {
                    isMessageFull = 1;
                    break;
                }
                isRunningStatBlock = 0;
            }
        }
        msgBuff[msgBuffIndex] = 0;
        SV_SendServerCommand(&svs.clients[clientNum], SV_CMD_RELIABLE, "%c %s", 78, msgBuff);
        if ( isMessageFull )
        {
            svs.clients[clientNum].statsSentIndex = i;
        }
        else if ( svs.clients[clientNum].statsSentIndex )
        {
            svs.clients[clientNum].statsSentIndex = 0;
        }
        else
        {
            svs.clients[clientNum].statsModified = 0;
            if ( !svs.clients[clientNum].bIsDemoClient && !svs.clients[clientNum].bIsTestClient )
                SV_UpdatePersonalBestsForClient(clientNum);
        }
    }
}

void __cdecl SV_BuildClientSnapshot(client_t *client)
{
    unsigned __int8 *v1; // eax
    clientState_s *ClientState; // eax
    ArchivedMatchState *p_archivedState; // ecx
    ArchivedMatchState *v4; // edx
    int v5; // [esp+18h] [ebp-10E0h]
    entityState_s *v6; // [esp+24h] [ebp-10D4h]
    clientSnapshot_t *v7; // [esp+28h] [ebp-10D0h]
    int pArchiveTime; // [esp+2Ch] [ebp-10CCh] BYREF
    snapshotEntityNumbers_t eNums; // [esp+30h] [ebp-10C8h] BYREF
    MatchState *v10; // [esp+103Ch] [ebp-BCh]
    archivedEntity_s *v11; // [esp+1040h] [ebp-B8h]
    clientState_s *v12; // [esp+1044h] [ebp-B4h]
    cachedSnapshot_t *CachedSnapshot; // [esp+104Ch] [ebp-ACh]
    unsigned int v15[32]; // [esp+1050h] [ebp-A8h] BYREF
    float position[3]; // [esp+10D0h] [ebp-28h] BYREF
    //float v17; // [esp+10D8h] [ebp-20h]
    const void *v18; // [esp+10DCh] [ebp-1Ch]
    int i; // [esp+10E0h] [ebp-18h]
    MatchState *v20; // [esp+10E4h] [ebp-14h]
    int clientNum; // [esp+10E8h] [ebp-10h]
    unsigned __int8 *dst; // [esp+10ECh] [ebp-Ch]
    int v23; // [esp+10F0h] [ebp-8h]
    cachedClient_s *v24; // [esp+10F4h] [ebp-4h]
    client_t *clients; // [esp+1100h] [ebp+8h]

    v7 = &client->frames[client->header.netchan.outgoingSequence & 0x1F];
    v7->physicsTime = sv.physicsTime;
    v7->num_entities = 0;
    v7->num_clients = 0;
    if ( client->gentity )
    {
        if ( client->header.state != CS_ZOMBIE )
        {
            v7->matchState = svs.nextSnapshotMatchStates;
            v7->first_entity = svs.nextSnapshotEntities;
            v7->first_client = svs.nextSnapshotClients;
            if ( sv.state == SS_GAME )
            {
                eNums.numSnapshotEntities = 0;
                clientNum = client - svs.clients;
                if ( (unsigned int)clientNum >= com_maxclients->current.integer
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                3008,
                                0,
                                "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                                clientNum,
                                com_maxclients->current.integer) )
                {
                    __debugbreak();
                }
                pArchiveTime = G_GetClientArchiveTime(clientNum);
                CachedSnapshot = SV_GetCachedSnapshot(&pArchiveTime);
                G_SetClientArchiveTime(clientNum, pArchiveTime);
                if ( CachedSnapshot )
                    v5 = svs.time - CachedSnapshot->time;
                else
                    v5 = 0;
                v23 = v5;
                dst = (unsigned __int8 *)v7;
                v1 = (unsigned __int8 *)SV_GameClientNum(clientNum);
                memcpy(dst, v1, 0x26A4u);
                clientNum = dst[304];
                position[0] = *((float *)dst + 9);
                position[1] = *((float *)dst + 10);
                position[2] = *((float *)dst + 11);
                position[2] = position[2] + *((float *)dst + 100);
                AddLeanToPosition(position, *((float *)dst + 97), *((float *)dst + 31), 16.0, 20.0);
                memset(v15, 0, sizeof(v15));
                if ( CachedSnapshot )
                {
                    SV_AddCachedEntitiesVisibleFromPoint(
                        CachedSnapshot->num_entities,
                        CachedSnapshot->first_entity,
                        position,
                        clientNum,
                        &eNums,
                        (playerState_s *)dst);
                    v20 = &svs.cachedSnapshotMatchStates[CachedSnapshot->matchState % svs.numCachedSnapshotMatchStates];
                    v10 = &svs.snapshotMatchStates[svs.nextSnapshotMatchStates % svs.numSnapshotMatchStates];
                    p_archivedState = &v20->archivedState;
                    v4 = &v10->archivedState;
                    v10->archivedState.matchUIVisibilityFlags = v20->archivedState.matchUIVisibilityFlags;
                    v4->bombTimer[0] = p_archivedState->bombTimer[0];
                    v4->bombTimer[1] = p_archivedState->bombTimer[1];
                    memcpy(&v10->unarchivedState, &G_GetMatchState()->unarchivedState, sizeof(v10->unarchivedState));
                    if ( ++svs.nextSnapshotMatchStates >= 2147483646 )
                        Com_Error(ERR_FATAL, "svs.nextSnapshotMatchStates wrapped");
                    for ( i = 0; i < eNums.numSnapshotEntities; ++i )
                    {
                        v11 = &svs.cachedSnapshotEntities[(eNums.snapshotEntities[i] + CachedSnapshot->first_entity)
                                                                                        % svs.numCachedSnapshotEntities];
                        v6 = &svs.snapshotEntities[svs.nextSnapshotEntities % svs.numSnapshotEntities];
                        memcpy(v6, v11, sizeof(entityState_s));
                        if ( v6->lerp.pos.trTime )
                            v6->lerp.pos.trTime += v23;
                        if ( v6->lerp.apos.trTime )
                            v6->lerp.apos.trTime += v23;
                        if ( v6->time2 )
                            v6->time2 += v23;
                        if ( v6->eType == 4 )
                        {
                            v6->lerp.u.actor.actorNum += v23;
                        }
                        else if ( !v6->eType || v6->eType == 82 )
                        {
                            v6->lerp.u.actor.actorNum += v23;
                        }
                        if ( ++svs.nextSnapshotEntities >= 2147483646 )
                            Com_Error(ERR_FATAL, "svs.nextSnapshotEntities wrapped");
                        ++v7->num_entities;
                    }
                    for ( i = 0; i < CachedSnapshot->num_clients; ++i )
                    {
                        v24 = &svs.cachedSnapshotClients[(i + CachedSnapshot->first_client) % svs.numCachedSnapshotClients];
                        v12 = &svs.snapshotClients[svs.nextSnapshotClients % svs.numSnapshotClients];
                        memcpy(v12, &v24->cs, sizeof(clientState_s));
                        if ( v15[v12->clientIndex]
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                        3143,
                                        0,
                                        "%s",
                                        "!clientIndex[clientState->clientIndex]") )
                        {
                            __debugbreak();
                        }
                        v15[v12->clientIndex] = 1;
                        if ( ++svs.nextSnapshotClients >= 2147483646 )
                            Com_Error(ERR_FATAL, "svs.nextSnapshotClients wrapped");
                        ++v7->num_clients;
                    }
                }
                else
                {
                    SV_AddEntitiesVisibleFromPoint(position, clientNum, &eNums);
                    v10 = &svs.snapshotMatchStates[svs.nextSnapshotMatchStates % svs.numSnapshotMatchStates];
                    memcpy(v10, G_GetMatchState(), sizeof(MatchState));
                    if ( ++svs.nextSnapshotMatchStates >= 2147483646 )
                        Com_Error(ERR_FATAL, "svs.nextSnapshotMatchStates wrapped");
                    for ( i = 0; i < eNums.numSnapshotEntities; ++i )
                    {
                        v18 = (char *)sv.gentities + eNums.snapshotEntities[i] * sv.gentitySize;
                        memcpy(
                            &svs.snapshotEntities[svs.nextSnapshotEntities++ % svs.numSnapshotEntities],
                            v18,
                            sizeof(svs.snapshotEntities[svs.nextSnapshotEntities++ % svs.numSnapshotEntities]));
                        if ( svs.nextSnapshotEntities >= 2147483646 )
                            Com_Error(ERR_FATAL, "svs.nextSnapshotEntities wrapped");
                        ++v7->num_entities;
                    }
                    i = 0;
                    clients = svs.clients;
                    while ( i < com_maxclients->current.integer )
                    {
                        if ( clients->header.state >= CS_CONNECTED )
                        {
                            v12 = &svs.snapshotClients[svs.nextSnapshotClients % svs.numSnapshotClients];
                            ClientState = G_GetClientState(i);
                            memcpy(v12, ClientState, sizeof(clientState_s));
                            if ( v12->clientIndex != i
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                            3076,
                                            0,
                                            "%s",
                                            "clientState->clientIndex == i") )
                            {
                                __debugbreak();
                            }
                            if ( v15[v12->clientIndex]
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                            3079,
                                            0,
                                            "%s",
                                            "!clientIndex[clientState->clientIndex]") )
                            {
                                __debugbreak();
                            }
                            v15[v12->clientIndex] = 1;
                            if ( ++svs.nextSnapshotClients >= 2147483646 )
                                Com_Error(ERR_FATAL, "svs.nextSnapshotClients wrapped");
                            ++v7->num_clients;
                        }
                        ++i;
                        ++clients;
                    }
                }
                svs.activeEntities = eNums.numSnapshotEntities;
            }
        }
    }
}

void __cdecl SV_AddEntitiesVisibleFromPoint(float *org, int clientNum, snapshotEntityNumbers_t *eNums)
{
    char *v4; // eax
    int e; // [esp+1Ch] [ebp-28h]
    int clientcluster; // [esp+20h] [ebp-24h]
    float fogOpaqueDistSqrd; // [esp+24h] [ebp-20h]
    svEntity_s *svEnt; // [esp+28h] [ebp-1Ch]
    int l; // [esp+2Ch] [ebp-18h]
    unsigned int leafnum; // [esp+30h] [ebp-14h]
    gentity_s *ent; // [esp+34h] [ebp-10h]
    int i; // [esp+38h] [ebp-Ch]
    unsigned __int8 *clientpvs; // [esp+40h] [ebp-4h]

    if (!SV_Loaded()
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
            1556,
            0,
            "%s",
            "SV_Loaded()"))
    {
        __debugbreak();
    }
    leafnum = CM_PointLeafnum(org);
    clientcluster = CM_LeafCluster(leafnum);
    if (clientcluster >= 0)
        clientpvs = CM_ClusterPVS(clientcluster);
    else
        clientpvs = 0;
    fogOpaqueDistSqrd = G_GetFogOpaqueDistSqrd();
    if (fogOpaqueDistSqrd == 3.4028235e38)
        fogOpaqueDistSqrd = 0.0f;
    for (e = 0; e < sv.num_entities; ++e)
    {
        ent = (gentity_s *)((char *)sv.gentities + e * sv.gentitySize);
        if (ent->r.linked)
        {
            if (ent->s.number != e)
            {
                v4 = va(
                    "entnum: %d vs %d, eType: %d, origin: %f %f %f",
                    ent->s.number,
                    e,
                    ent->s.eType,
                    ent->r.currentOrigin[0],
                    ent->r.currentOrigin[1],
                    ent->r.currentOrigin[2]);
                if (!Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    1578,
                    0,
                    "%s\n\t%s",
                    "ent->s.number == e",
                    v4))
                    __debugbreak();
            }
            if (e != clientNum)
            {
                if (ent->r.broadcastTime)
                {
                    if (ent->r.broadcastTime < 0 || ent->r.broadcastTime - svs.time >= 0)
                        goto LABEL_29;
                    ent->r.broadcastTime = 0;
                }
                else if ((ent->r.svFlags & 1) != 0 || (ent->r.clientMask[clientNum >> 5] & (1 << (clientNum & 0x1F))) != 0)
                {
                    continue;
                }
                if ((ent->r.svFlags & 0x18) != 0)
                    goto LABEL_29;
                svEnt = SV_SvEntityForGentity(ent);
                if (!svEnt->numClusters || !clientpvs)
                    goto LABEL_29;
                l = 0;
                for (i = 0; i < svEnt->numClusters; ++i)
                {
                    l = svEnt->clusternums[i];
                    if (((1 << (l & 7)) & clientpvs[l >> 3]) != 0)
                        break;
                }
                if (i != svEnt->numClusters)
                    goto LABEL_45;
                if (svEnt->lastCluster)
                {
                    while (l <= svEnt->lastCluster && ((1 << (l & 7)) & clientpvs[l >> 3]) == 0)
                        ++l;
                    if (l != svEnt->lastCluster)
                    {
                    LABEL_45:
                        if (fogOpaqueDistSqrd == 0.0 || !BoxDistSqrdExceeds(ent->r.absmin, ent->r.absmax, org, fogOpaqueDistSqrd))
                        {
                        LABEL_29:
                            SV_AddArchivedEntToSnapshot(e, eNums);
                            continue;
                        }
                    }
                }
            }
        }
    }
}

void __cdecl SV_AddArchivedEntToSnapshot(int e, snapshotEntityNumbers_t *eNums)
{
    if ( eNums->numSnapshotEntities != 1024 )
        eNums->snapshotEntities[eNums->numSnapshotEntities++] = e;
}

void __cdecl SV_AddCachedEntitiesVisibleFromPoint(
                int from_num_entities,
                int from_first_entity,
                float *org,
                int clientNum,
                snapshotEntityNumbers_t *eNums,
                playerState_s *ps)
{
    int e; // [esp+4h] [ebp-1130h]
    int cluster; // [esp+8h] [ebp-112Ch]
    int lastLeaf; // [esp+Ch] [ebp-1128h] BYREF
    float fogOpaqueDistSqrd; // [esp+10h] [ebp-1124h]
    unsigned int dst[1025]; // [esp+14h] [ebp-1120h] BYREF
    int v11; // [esp+1018h] [ebp-11Ch]
    unsigned __int16 list[128]; // [esp+101Ch] [ebp-118h] BYREF
    int leafnum; // [esp+1120h] [ebp-14h]
    archivedEntity_s *v14; // [esp+1124h] [ebp-10h]
    int v15; // [esp+1128h] [ebp-Ch]
    int i; // [esp+112Ch] [ebp-8h]
    unsigned __int8 *v17; // [esp+1130h] [ebp-4h]

    if ( !SV_Loaded()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    1686,
                    0,
                    "%s",
                    "SV_Loaded()") )
    {
        __debugbreak();
    }
    leafnum = CM_PointLeafnum(org);
    cluster = CM_LeafCluster(leafnum);
    if ( cluster >= 0 )
    {
        v17 = CM_ClusterPVS(cluster);
        fogOpaqueDistSqrd = G_GetFogOpaqueDistSqrd();
        if ( fogOpaqueDistSqrd == 3.4028235e38 )
            fogOpaqueDistSqrd = 0.0f;
        memset(dst, 0, 0x1000u);
        for ( e = 0; ; ++e )
        {
            if ( e >= from_num_entities )
                return;
            v14 = &svs.cachedSnapshotEntities[(e + from_first_entity) % svs.numCachedSnapshotEntities];
            if ( dst[v14->s.number]
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                            1710,
                            0,
                            "%s",
                            "!entityIndex[ent->s.number]") )
            {
                __debugbreak();
            }
            dst[v14->s.number] = 1;
            if ( (v14->r.clientMask[clientNum >> 5] & (1 << (clientNum & 0x1F))) == 0 )
            {
                if ( v14->s.number == clientNum )
                {
                    if ( (ps->otherFlags & 2) != 0 )
                        continue;
                    if ( (ps->otherFlags & 4) != 0
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                                    1722,
                                    0,
                                    "%s",
                                    "!( ps->otherFlags & POF_PLAYER )") )
                    {
                        __debugbreak();
                    }
                }
                if ( (v14->r.svFlags & 0x18) != 0 )
                    goto LABEL_20;
                v15 = CM_BoxLeafnums(v14->r.absmin, v14->r.absmax, list, 128, &lastLeaf);
                if ( v15 )
                {
                    for ( i = 0; i < v15; ++i )
                    {
                        v11 = CM_LeafCluster(list[i]);
                        if ( v11 != -1 && ((1 << (v11 & 7)) & v17[v11 >> 3]) != 0 )
                            break;
                    }
                    if ( (i != v15 || (v14->s.clientLinkInfo.flags & 1) != 0)
                        && (fogOpaqueDistSqrd == 0.0 || !BoxDistSqrdExceeds(v14->r.absmin, v14->r.absmax, org, fogOpaqueDistSqrd)) )
                    {
LABEL_20:
                        SV_AddArchivedEntToSnapshot(e, eNums);
                    }
                }
            }
        }
    }
}

unsigned __int8 svCompressedBuf[131072];
//unsigned __int8 to[65536];
void __cdecl SV_SendMessageToClient(msg_t *msg, client_t *client, bool reliable)
{
    int lowest_send_count; // [esp+10h] [ebp-Ch]
    int compressedSize; // [esp+14h] [ebp-8h]
    int rateMsec; // [esp+18h] [ebp-4h]

    if ( (unsigned int)(client - svs.clients) >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3248,
                    0,
                    "client - svs.clients doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    client - svs.clients,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    if ( msg->cursize < 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3250,
                    0,
                    "%s",
                    "msg->cursize >= SV_ENCODE_START") )
    {
        __debugbreak();
    }
    *(unsigned int *)svCompressedBuf = *(unsigned int *)msg->data;
    // 'to' is not used anywhere, leading svCompressedBuf to be used (with only the 1st 4 bytes set, rest zero). Great!
    //compressedSize = MSG_WriteBitsCompress(client->header.state == CS_ACTIVE, msg->data + 4, msg->cursize - 4, to, 65532) + 4;
    compressedSize = MSG_WriteBitsCompress(client->header.state == CS_ACTIVE, msg->data + 4, msg->cursize - 4, svCompressedBuf + 4, 65532) + 4;
    if ( sv_showHuffmanData->current.enabled )
    {
        showHuffmanData();
        Dvar_SetBool((dvar_s *)sv_showHuffmanData, 0);
    }
    if ( sv_debugPacketContentsQuick->current.integer > 0 || sv_printMessageSize->current.enabled )
        CG_QuickPrint(
            ">>> MESSAGE: %d %s %d/%d %2.2f%% %d/%d <<<\n",
            svs.time,
            client->name,
            compressedSize,
            msg->cursize,
            (float)(msg->cursize - compressedSize) * 100.0 / (double)msg->cursize,
            sv_quickBitsTotal,
            8 * msg->cursize);
    CG_QuickPrintFlush();
    if ( client->header.netchan.remoteAddress.type != NA_LOOPBACK )
        SV_PacketAnalyze_TrackPacketCompression(client - svs.clients, msg->cursize, compressedSize);
    if ( client->dropReason )
    {
        SV_DropClient(client, client->dropReason, 1, 1);
        
        iassert(!client->dropReason);
        iassert(client->header.state == CS_ZOMBIE);
    }
    client->frames[client->header.netchan.outgoingSequence & 0x1F].messageSize = compressedSize;
    client->frames[client->header.netchan.outgoingSequence & 0x1F].messageSent = Sys_Milliseconds();
    client->frames[client->header.netchan.outgoingSequence & 0x1F].messageAcked = -1;
    SV_Netchan_Transmit(client, svCompressedBuf, compressedSize, reliable);
    if ( msg->flush && client->header.netchan.unsentFragments )
    {
        if ( reliable )
        {
            lowest_send_count = client->header.netchan.lowest_send_count;
            while ( client->header.state != CS_FREE && lowest_send_count == client->header.netchan.lowest_send_count )
                SV_Netchan_TransmitNextFragment(client, &client->header.netchan);
        }
        else
        {
            while ( client->header.state != CS_FREE && client->header.netchan.unsentFragments )
                SV_Netchan_TransmitNextFragment(client, &client->header.netchan);
        }
        if ( client->header.netchan.remoteAddress.type == NA_LOOPBACK
            || Sys_IsLANAddress(client->header.netchan.remoteAddress) )
        {
            client->nextSnapshotTime = svs.time - 1;
        }
        else
        {
            client->nextSnapshotTime = SV_WindowedRateMsec(client) + svs.time;
            client->lastSnapshotTime = svs.time;
        }
    }
    else if ( client->header.netchan.remoteAddress.type == NA_LOOPBACK
                 || Sys_IsLANAddress(client->header.netchan.remoteAddress) )
    {
        client->nextSnapshotTime = svs.time - 1;
        client->lastSnapshotTime = svs.time;
    }
    else
    {
        rateMsec = SV_WindowedRateMsec(client);
        if ( rateMsec >= client->snapshotMsec )
        {
            client->header.rateDelayed = 1;
        }
        else
        {
            rateMsec = client->snapshotMsec;
            client->header.rateDelayed = 0;
        }
        client->nextSnapshotTime = rateMsec + svs.time;
        client->lastSnapshotTime = svs.time;
        if ( client->header.state != CS_ACTIVE && !client->downloadName[0] && client->nextSnapshotTime < svs.time + 1000 )
            client->nextSnapshotTime = svs.time + 1000;
        sv.bpsTotalBytes += compressedSize;
    }
}

int __cdecl SV_WindowedRateMsec(client_t *client)
{
    int rate; // [esp+0h] [ebp-1Ch]
    int msec; // [esp+Ch] [ebp-10h]
    int history; // [esp+10h] [ebp-Ch]
    unsigned int rateTime; // [esp+14h] [ebp-8h]
    int accumSize; // [esp+18h] [ebp-4h]

    rateTime = Sys_Milliseconds();
    accumSize = 0;
    rate = client->rate;
    if ( sv_maxRate->current.integer )
    {
        if ( sv_maxRate->current.integer < 1000 )
            Dvar_SetInt((dvar_s *)sv_maxRate, 1000);
        if ( sv_maxRate->current.integer < rate )
            rate = sv_maxRate->current.integer;
    }
    for ( history = 0;
                history < 32
         && history < client->header.netchan.outgoingSequence
         && (client->frames[((unsigned __int8)client->header.netchan.outgoingSequence - (_BYTE)history) & 0x1F].messageSize <= 0
            || client->frames[((unsigned __int8)client->header.netchan.outgoingSequence - (_BYTE)history) & 0x1F].messageSent > (signed int)(rateTime - 1000));
                ++history )
    {
        accumSize += client->frames[((unsigned __int8)client->header.netchan.outgoingSequence - (_BYTE)history) & 0x1F].messageSize
                             + 48;
    }
    if ( sv_debugRate->current.enabled )
        printf("Accumulated send window to client %s: %dbytes\n", client->name, accumSize);
    msec = 1000 * (accumSize - rate) / rate;
    if ( msec < 0 )
        return -1;
    if ( sv_debugRate->current.enabled )
        printf("Send to client %s throttled for %dmsec. (rate %i)\n", client->name, msec, rate);
    return 1000 * (accumSize - rate) / rate;
}

unsigned __int8 tempSnapshotMsgBuf[65536];
void __cdecl SV_BeginClientSnapshot(client_t *client, msg_t *msg)
{
    unsigned int clientNum; // [esp+0h] [ebp-4h]

    client->tempPacketDebugging = 0;
    if ( sv_debugPacketContentsForClientThisFrame->current.enabled )
    {
        client->tempPacketDebugging = 1;
        Dvar_SetBool((dvar_s *)sv_debugPacketContentsForClientThisFrame, 0);
        Dvar_SetBool((dvar_s *)sv_debugPacketContents, 1);
    }
    if ( sv_debugPacketContents->current.enabled )
        Com_Printf(15, "Starting snapshot for %s\n", client->name);
    sv_quickBitsTotal = 0;
    if ( sv_debugPacketContentsQuick->current.integer > 0 )
        CG_QuickPrint("-- Quick Packet Inspect Start Snap --\n");
    clientNum = client - svs.clients;
    if ( clientNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3413,
                    0,
                    "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    clientNum,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    SV_AddModifiedStats(clientNum);
    MSG_Init(msg, tempSnapshotMsgBuf, 0x10000);
    MSG_ClearLastReferencedEntity(msg);
    MSG_WriteLong(msg, client->lastClientCommand);
    if ( client->header.state == CS_ACTIVE || client->header.state == CS_ZOMBIE )
        SV_UpdateServerCommandsToClient(client, msg);
}

void __cdecl SV_EndClientSnapshot(client_t *client, msg_t *msg)
{
    if ( (unsigned int)(client - svs.clients) >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3451,
                    0,
                    "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    client - svs.clients,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    if ( client->header.state != CS_ZOMBIE )
        SV_WriteDownloadToClient(client, msg);
    MSG_WriteByte(msg, svc_EOF);
    if ( msg->overflowed )
    {
        Com_PrintWarning(15, "WARNING: msg overflowed for %s, trying to recover\n", client->name);
        if ( client->header.state == CS_ACTIVE || client->header.state == CS_ZOMBIE )
        {
            SV_PrintServerCommandsForClient(client);
            MSG_Init(msg, tempSnapshotMsgBuf, 0x10000);
            MSG_WriteLong(msg, client->lastClientCommand);
            SV_UpdateServerCommandsToClient_PreventOverflow(client, msg, 0x10000);
            MSG_WriteByte(msg, 0xEu);
        }
        if ( msg->overflowed )
        {
            Com_PrintWarning(15, "WARNING: client disconnected for msg overflow: %s\n", client->name);
            NET_OutOfBandPrint(NS_SERVER, client->header.netchan.remoteAddress, "disconnect");
            SV_DropClient(client, "EXE_SERVERMESSAGEOVERFLOW", 1, 1);
        }
    }
    if ( sv_debugPacketContents->current.enabled )
        Com_Printf(15, "Snapshot finished for %s\n", client->name);
    if ( client->tempPacketDebugging )
        Dvar_SetBool((dvar_s *)sv_debugPacketContents, 0);
    if ( client->header.netchan.remoteAddress.type != NA_LOOPBACK )
        SV_PacketAnalyze_TrackSnapshotSize(msg->cursize);
    if ( MSG_GetUsedBitCount(msg) > 9312 )
        msg->flush = 1;
    SV_SendMessageToClient(msg, client, 0);
}

void __cdecl SV_PrintServerCommandsForClient(client_t *client)
{
    int i; // [esp+0h] [ebp-4h]

    Com_Printf(15, "-- Unacknowledged Server Commands for client %i:%s --\n", client - svs.clients, client->name);
    for ( i = client->reliableAcknowledge + 1; i <= client->reliableSequence; ++i )
        Com_Printf(
            15,
            "cmd %5d: %8d: %s\n",
            i,
            client->reliableCommandInfo[i & 0x7F].time,
            client->reliableCommandInfo[i & 0x7F].cmd);
    Com_Printf(15, "----------");
}

void __cdecl SV_SetServerStaticHeader()
{
    svsHeader.clients = svs.clients;
    svsHeader.time = svs.time;
    svs.physicsTime = sv.physicsTime;
    svsHeader.physicsTime = sv.physicsTime;
    svsHeader.snapFlagServerBit = svs.snapFlagServerBit;
    svsHeader.numCachedSnapshotMatchStates = svs.numCachedSnapshotMatchStates;
    svsHeader.numCachedSnapshotEntities = svs.numCachedSnapshotEntities;
    svsHeader.numCachedSnapshotClients = svs.numCachedSnapshotClients;
    svsHeader.numSnapshotMatchStates = svs.numSnapshotMatchStates;
    svsHeader.numSnapshotEntities = svs.numSnapshotEntities;
    svsHeader.numSnapshotClients = svs.numSnapshotClients;
    svsHeader.nextSnapshotMatchStates = svs.nextSnapshotMatchStates;
    svsHeader.nextSnapshotEntities = svs.nextSnapshotEntities;
    svsHeader.snapshotMatchStates = svs.snapshotMatchStates;
    svsHeader.snapshotEntities = svs.snapshotEntities;
    svsHeader.snapshotClients = svs.snapshotClients;
    svsHeader.svEntities = sv.svEntities;
    svsHeader.mapCenter[0] = svs.mapCenter[0];
    svsHeader.mapCenter[1] = svs.mapCenter[1];
    svsHeader.mapCenter[2] = svs.mapCenter[2];
    svsHeader.cachedSnapshotMatchStates = svs.cachedSnapshotMatchStates;
    svsHeader.cachedSnapshotEntities = svs.cachedSnapshotEntities;
    svsHeader.cachedSnapshotClients = svs.cachedSnapshotClients;
    svsHeader.archivedSnapshotBuffer = svs.archivedSnapshotBuffer;
    svsHeader.cachedSnapshotFrames = svs.cachedSnapshotFrames;
    svsHeader.nextCachedSnapshotFrames = svs.nextCachedSnapshotFrames;
    svsHeader.nextArchivedSnapshotFrames = svs.nextArchivedSnapshotFrames;
    svsHeader.nextCachedSnapshotMatchStates = svs.nextCachedSnapshotMatchStates;
    svsHeader.nextCachedSnapshotEntities = svs.nextCachedSnapshotEntities;
    svsHeader.nextCachedSnapshotClients = svs.nextCachedSnapshotClients;
    svsHeader.archivedEntityCount = svs.archivedEntityCount;
    svsHeader.num_entities = sv.num_entities;
    svsHeader.maxclients = com_maxclients->current.integer;
    svsHeader.fps = sv_fps->current.integer;
    svsHeader.clientArchive = sv_clientArchive->current.color[0];
    svsHeader.gentities = sv.gentities;
    svsHeader.gentitySize = sv.gentitySize;
    svsHeader.firstClientState = G_GetClientState(0);
    svsHeader.firstPlayerState = (playerState_s *)G_GetPlayerState(0);
    svsHeader.clientSize = G_GetClientSize();
    svsHeader.matchState = G_GetMatchState();
    svsHeaderValid = 1;
}

void __cdecl SV_GetServerStaticHeader()
{
    bool v0; // [esp+0h] [ebp-4h]

    if ( svsHeader.clients != svs.clients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3576,
                    0,
                    "%s",
                    "svsHeader.clients == svs.clients") )
    {
        __debugbreak();
    }
    if ( svsHeader.physicsTime != svs.physicsTime
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3579,
                    0,
                    "%s",
                    "svsHeader.physicsTime == svs.physicsTime") )
    {
        __debugbreak();
    }
    if ( svsHeader.snapFlagServerBit != svs.snapFlagServerBit
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3580,
                    0,
                    "%s",
                    "svsHeader.snapFlagServerBit == svs.snapFlagServerBit") )
    {
        __debugbreak();
    }
    if ( svsHeader.numCachedSnapshotMatchStates != svs.numCachedSnapshotMatchStates
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3581,
                    0,
                    "%s",
                    "svsHeader.numCachedSnapshotMatchStates == svs.numCachedSnapshotMatchStates") )
    {
        __debugbreak();
    }
    if ( svsHeader.numCachedSnapshotEntities != svs.numCachedSnapshotEntities
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3582,
                    0,
                    "%s",
                    "svsHeader.numCachedSnapshotEntities == svs.numCachedSnapshotEntities") )
    {
        __debugbreak();
    }
    if ( svsHeader.numCachedSnapshotClients != svs.numCachedSnapshotClients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3583,
                    0,
                    "%s",
                    "svsHeader.numCachedSnapshotClients == svs.numCachedSnapshotClients") )
    {
        __debugbreak();
    }
    if ( svsHeader.numSnapshotMatchStates != svs.numSnapshotMatchStates
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3584,
                    0,
                    "%s",
                    "svsHeader.numSnapshotMatchStates == svs.numSnapshotMatchStates") )
    {
        __debugbreak();
    }
    if ( svsHeader.numSnapshotEntities != svs.numSnapshotEntities
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3585,
                    0,
                    "%s",
                    "svsHeader.numSnapshotEntities == svs.numSnapshotEntities") )
    {
        __debugbreak();
    }
    if ( svsHeader.numSnapshotClients != svs.numSnapshotClients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3586,
                    0,
                    "%s",
                    "svsHeader.numSnapshotClients == svs.numSnapshotClients") )
    {
        __debugbreak();
    }
    if ( svsHeader.nextSnapshotMatchStates != svs.nextSnapshotMatchStates
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3587,
                    0,
                    "%s",
                    "svsHeader.nextSnapshotMatchStates == svs.nextSnapshotMatchStates") )
    {
        __debugbreak();
    }
    if ( svsHeader.nextSnapshotEntities != svs.nextSnapshotEntities
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3588,
                    0,
                    "%s",
                    "svsHeader.nextSnapshotEntities == svs.nextSnapshotEntities") )
    {
        __debugbreak();
    }
    if ( svsHeader.snapshotMatchStates != svs.snapshotMatchStates
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3589,
                    0,
                    "%s",
                    "svsHeader.snapshotMatchStates == svs.snapshotMatchStates") )
    {
        __debugbreak();
    }
    if ( svsHeader.snapshotEntities != svs.snapshotEntities
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3590,
                    0,
                    "%s",
                    "svsHeader.snapshotEntities == svs.snapshotEntities") )
    {
        __debugbreak();
    }
    if ( svsHeader.snapshotClients != svs.snapshotClients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3591,
                    0,
                    "%s",
                    "svsHeader.snapshotClients == svs.snapshotClients") )
    {
        __debugbreak();
    }
    iassert(svsHeader.svEntities == sv.svEntities);
    v0 = svs.mapCenter[0] == svsHeader.mapCenter[0]
        && svs.mapCenter[1] == svsHeader.mapCenter[1]
        && svs.mapCenter[2] == svsHeader.mapCenter[2];
    if ( !v0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3593,
                    0,
                    "%s",
                    "Vec3Compare( svs.mapCenter, svsHeader.mapCenter )") )
    {
        __debugbreak();
    }
    if ( svsHeader.cachedSnapshotMatchStates != svs.cachedSnapshotMatchStates
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3594,
                    0,
                    "%s",
                    "svsHeader.cachedSnapshotMatchStates == svs.cachedSnapshotMatchStates") )
    {
        __debugbreak();
    }
    if ( svsHeader.cachedSnapshotEntities != svs.cachedSnapshotEntities
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3595,
                    0,
                    "%s",
                    "svsHeader.cachedSnapshotEntities == svs.cachedSnapshotEntities") )
    {
        __debugbreak();
    }
    if ( svsHeader.cachedSnapshotClients != svs.cachedSnapshotClients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3596,
                    0,
                    "%s",
                    "svsHeader.cachedSnapshotClients == svs.cachedSnapshotClients") )
    {
        __debugbreak();
    }
    if ( svsHeader.archivedSnapshotBuffer != svs.archivedSnapshotBuffer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3597,
                    0,
                    "%s",
                    "svsHeader.archivedSnapshotBuffer == svs.archivedSnapshotBuffer") )
    {
        __debugbreak();
    }
    if ( svsHeader.cachedSnapshotFrames != svs.cachedSnapshotFrames
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3598,
                    0,
                    "%s",
                    "svsHeader.cachedSnapshotFrames == svs.cachedSnapshotFrames") )
    {
        __debugbreak();
    }
    svs.nextCachedSnapshotFrames = svsHeader.nextCachedSnapshotFrames;
    if ( svsHeader.nextArchivedSnapshotFrames != svs.nextArchivedSnapshotFrames
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3600,
                    0,
                    "%s",
                    "svsHeader.nextArchivedSnapshotFrames == svs.nextArchivedSnapshotFrames") )
    {
        __debugbreak();
    }
    svs.nextCachedSnapshotMatchStates = svsHeader.nextCachedSnapshotMatchStates;
    svs.nextCachedSnapshotEntities = svsHeader.nextCachedSnapshotEntities;
    svs.nextCachedSnapshotClients = svsHeader.nextCachedSnapshotClients;
    svs.archivedEntityCount = svsHeader.archivedEntityCount;
    iassert(svsHeader.num_entities == sv.num_entities);
    if ( svsHeader.maxclients != com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3606,
                    0,
                    "%s",
                    "svsHeader.maxclients == com_maxclients->current.integer") )
    {
        __debugbreak();
    }
    if ( svsHeader.fps != sv_fps->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3607,
                    0,
                    "%s",
                    "svsHeader.fps == sv_fps->current.integer") )
    {
        __debugbreak();
    }
    if ( svsHeader.clientArchive != sv_clientArchive->current.color[0]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3608,
                    0,
                    "%s",
                    "svsHeader.clientArchive == (qboolean)sv_clientArchive->current.enabled") )
    {
        __debugbreak();
    }
    iassert(svsHeader.gentities == sv.gentities);
    iassert(svsHeader.gentitySize == sv.gentitySize);
    if ( svsHeader.firstClientState != G_GetClientState(0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3611,
                    0,
                    "%s",
                    "svsHeader.firstClientState == G_GetClientState( 0 )") )
    {
        __debugbreak();
    }
    if ( (gclient_s *)svsHeader.firstPlayerState != G_GetPlayerState(0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3612,
                    0,
                    "%s",
                    "svsHeader.firstPlayerState == G_GetPlayerState( 0 )") )
    {
        __debugbreak();
    }
    if ( svsHeader.clientSize != G_GetClientSize()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3613,
                    0,
                    "%s",
                    "svsHeader.clientSize == G_GetClientSize()") )
    {
        __debugbreak();
    }
    if ( svsHeader.matchState != G_GetMatchState()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3614,
                    0,
                    "%s",
                    "svsHeader.matchState == G_GetMatchState()") )
    {
        __debugbreak();
    }
    svsHeaderValid = 0;
}

int lastClientSent = -1;
void __cdecl SV_SendClientMessages()
{
    char *v1; // eax
    int v2; // eax
    float comp_ratio; // [esp+74h] [ebp-98h]
    float ave; // [esp+78h] [ebp-94h]
    float uave; // [esp+7Ch] [ebp-90h]
    float uavea; // [esp+7Ch] [ebp-90h]
    float uaveb; // [esp+7Ch] [ebp-90h]
    int total; // [esp+80h] [ebp-8Ch]
    archivedSnapshot_s *frame; // [esp+88h] [ebp-84h]
    client_t *c; // [esp+8Ch] [ebp-80h]
    client_t *ca; // [esp+8Ch] [ebp-80h]
    client_t *cb; // [esp+8Ch] [ebp-80h]
    int maxclients; // [esp+94h] [ebp-78h]
    int startClient; // [esp+98h] [ebp-74h]
    int numclients; // [esp+9Ch] [ebp-70h]
    bool valid[32]; // [esp+A0h] [ebp-6Ch] BYREF
    int startIndex; // [esp+C4h] [ebp-48h]
    msg_t msg; // [esp+C8h] [ebp-44h] BYREF
    int partSize; // [esp+F8h] [ebp-14h]
    int clientCounter; // [esp+FCh] [ebp-10h]
    int i; // [esp+100h] [ebp-Ch]
    int clientNum; // [esp+104h] [ebp-8h]
    int maxBytesPerFrame; // [esp+108h] [ebp-4h]

    numclients = 0;

    PROF_SCOPED("SV_SendClientMessages");

    sv.bpsTotalBytes = 0;
    sv.ubpsTotalBytes = 0;
    memset(valid, 0, sizeof(valid));
    maxclients = com_maxclients->current.integer;
    maxBytesPerFrame = Live_GetNecessaryBandwidth() / 160;
    startClient = (lastClientSent + 1) % maxclients;

    {
        PROF_SCOPED("extra messages");

        for (clientCounter = 0; clientCounter < maxclients; ++clientCounter)
        {
            clientNum = (clientCounter + startClient) % maxclients;
            c = &svs.clients[clientNum];
            if (c->header.state
                && svs.time >= c->nextSnapshotTime
                && (!c->bIsTestClient || c->header.netchan.remoteAddress.type || sv_botSnapshotDebug->current.enabled))
            {
                if (sv.bpsTotalBytes > maxBytesPerFrame)
                    break;
                ++numclients;
                if (c->header.netchan.unsentFragments)
                {
                    if (c->header.netchan.remoteAddress.type == NA_LOOPBACK)
                        c->nextSnapshotTime = svs.time - 1;
                    else
                        c->nextSnapshotTime = 50 * c->header.netchan.lowest_send_count + svs.time + SV_WindowedRateMsec(c);
                    c->lastSnapshotTime = svs.time;
                    SV_Netchan_TransmitNextFragment(c, &c->header.netchan);
                    lastClientSent = clientNum;
                }
                else
                {
                    valid[clientNum] = 1;
                    if (sv.bpsTotalBytes < maxBytesPerFrame && (c->header.state == CS_ACTIVE || c->header.state == CS_ZOMBIE))
                    {
                        PROF_SCOPED("SV_BuildClientSnapshot");
                        SV_BuildClientSnapshot(c);
                    }
                }
            }
        }
    }

    SV_SetServerStaticHeader();

    {
        PROF_SCOPED("SV_SendClientSnapshot");
        for (clientCounter = 0; clientCounter < maxclients; ++clientCounter)
        {
            clientNum = (clientCounter + startClient) % maxclients;
            ca = &svs.clients[clientNum];
            if (valid[clientNum])
            {
                if (sv.bpsTotalBytes >= maxBytesPerFrame)
                    break;
                lastClientSent = clientNum;
                
                {
                    v1 = va("SV_SendClientSnapshot %d", clientNum);
                    PROF_SCOPED_RUNTIME_NAME(v1);

                    SV_BeginClientSnapshot(ca, &msg);
                    if (ca->header.state == CS_ACTIVE || ca->header.state == CS_ZOMBIE)
                    {
                        PROF_SCOPED("SV_WriteSnapshotToClient");
                        SV_WriteSnapshotToClient(ca, &msg);
                    }
                    SV_EndClientSnapshot(ca, &msg);
                }
                
                SV_SendClientVoiceData(ca);
            }
        }
    }

    if (sv.bpsTotalBytes >= maxBytesPerFrame)
        StatMon_Warning(13, 3000, (char*)"code_warning_bandwidthlimited");
    else
        lastClientSent = -1;
    if (Demo_ShouldBuildDemoSnapshot() && Demo_IsRecording())
    {
        PROF_SCOPED("Demo_BuildDemoSnapshot");

        cb = &svs.clients[Demo_GetDemoClientIndex()];
        if (!cb->bIsDemoClient
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                3839,
                0,
                "%s",
                "c->bIsDemoClient"))
        {
            __debugbreak();
        }
        if (cb->header.state == CS_ACTIVE || cb->header.state == CS_ZOMBIE)
            Demo_BuildDemoSnapshot();
    }
    if (numclients > 0)
    {
        uave = 0.0f;
        total = 0;
        for (i = 0; i < 19; ++i)
        {
            sv.bpsWindow[i] = sv.bpsWindow[i + 1];
            total += sv.bpsTotalBytes;
            sv.ubpsWindow[i] = sv.ubpsWindow[i + 1];
            uave = (float)sv.ubpsWindow[i] + uave;
        }
        sv.bpsWindow[19] = sv.bpsTotalBytes;
        sv.ubpsWindow[19] = sv.ubpsTotalBytes;
        uavea = (float)sv.ubpsTotalBytes + uave;
        if (sv.bpsTotalBytes >= sv.bpsMaxBytes)
            sv.bpsMaxBytes = sv.bpsTotalBytes;
        if (sv.ubpsTotalBytes >= sv.ubpsMaxBytes)
            sv.ubpsMaxBytes = sv.ubpsTotalBytes;
        if (++sv.bpsWindowSteps >= 20 && sv_showAverageBPS->current.enabled)
        {
            sv.bpsWindowSteps = 0;
            ave = (float)(sv.bpsTotalBytes + total) / 20.0;
            uaveb = uavea / 20.0;
            comp_ratio = (float)(1.0 - (float)(ave / uaveb)) * 100.0;
            sv.ucompAve = sv.ucompAve + comp_ratio;
            Com_DPrintf(
                15,
                "bpspc(%2.0f) bps(%2.0f) pk(%i) ubps(%2.0f) upk(%i) cr(%2.2f) acr(%2.2f)\n",
                (float)(ave / (float)numclients),
                ave,
                sv.bpsMaxBytes,
                uaveb,
                sv.ubpsMaxBytes,
                comp_ratio,
                (float)(sv.ucompAve / (float)++sv.ucompNum));
        }
    }
    g_archivingSnapshot = 1;
    if (sv.state == SS_GAME)
    {
        PROF_SCOPED("SV_ArchiveSnapshot");
        MSG_Init(&g_archiveMsg, tempServerMsgBuf, 0x10000);
        SV_ArchiveSnapshot(&g_archiveMsg);
    }
    SV_GetServerStaticHeader();
    if (sv.state == SS_GAME)
    {
        if (g_archiveMsg.overflowed)
        {
            Com_DPrintf(15, "SV_ArchiveSnapshot: ignoring snapshot because it overflowed.\n");
        }
        else
        {
            if (!svs.archivedSnapshotFrames
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_mp.cpp",
                    3963,
                    0,
                    "%s",
                    "svs.archivedSnapshotFrames"))
            {
                __debugbreak();
            }
            frame = &svs.archivedSnapshotFrames[svs.nextArchivedSnapshotFrames % 1200];
            frame->start = svs.nextArchivedSnapshotBuffer;
            frame->size = g_archiveMsg.cursize;
            v2 = svs.nextArchivedSnapshotBuffer % 0x1000000;
            startIndex = svs.nextArchivedSnapshotBuffer % 0x1000000;
            svs.nextArchivedSnapshotBuffer = (g_archiveMsg.cursize + svs.nextArchivedSnapshotBuffer) % 671088640;
            partSize = 0x1000000 - v2;
            if (g_archiveMsg.cursize > 0x1000000 - v2)
            {
                memcpy(&svs.archivedSnapshotBuffer[startIndex], g_archiveMsg.data, partSize);
                memcpy(svs.archivedSnapshotBuffer, &g_archiveMsg.data[partSize], g_archiveMsg.cursize - partSize);
            }
            else
            {
                memcpy(&svs.archivedSnapshotBuffer[startIndex], g_archiveMsg.data, g_archiveMsg.cursize);
            }
            if (++svs.nextArchivedSnapshotFrames >= 2147483646)
                Com_Error(ERR_FATAL, "svs.nextArchivedSnapshotFrames wrapped");
        }
    }
    g_archivingSnapshot = 0;
    //if (GetCurrentThreadId() == g_DXDeviceThread)
    //    D3DPERF_EndEvent();
}

