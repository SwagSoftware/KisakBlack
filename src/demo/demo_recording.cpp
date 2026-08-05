#include "demo_recording.h"
#include <universal/com_workercmds.h>

#include <cstring>
#include "demo_files.h"
#include "demo_playback.h"
#include <qcommon/threads.h>
#include <live/live_storage.h>
#include <live/live_win.h>
#include <qcommon/common.h>
#include <qcommon/cmd.h>
#include <win32/win_shared.h>
#include <win32/win_net.h>
#include <server_mp/sv_main_mp.h>
#include <qcommon/sv_msg_write_mp.h>
#include "demo_profile.h"
#include <glass/glass_server.h>
#include <ui_mp/ui_main_mp.h>
#include <ui/ui_main.h>
#include <universal/q_parse.h>
#include <stdlib.h>
#include <qcommon/sv_msg_write.h>
#include <game_mp/g_main_mp.h>
#include <universal/com_shared.h>
#include <time.h>

uploadStreamData_t s_uploadStreamData;
uploadStreamHistory_t s_uploadStreamUsage;

volatile unsigned int Demo_SaveLimit;
jqModule Demo_SaveModule =
{
    .Name = "Demo_Save",
    .Type = JQ_WORKER_GENERIC,
    .Code = Demo_SaveCallback,
    //.Group = 0
};
jqWorkerCmd Demo_SaveWorkerCmd = { &Demo_SaveModule, 12u, 0, 0, &Demo_SaveLimit, NULL, 0u };

int g_democlientindex;

SnapshotInfo_s g_snapInfo;

void __cdecl Demo_StartSaveProcess()
{
    unsigned __int8 *ActiveMemoryBuffer; // eax
    const char *v1; // eax
    int cursize; // [esp-8h] [ebp-Ch]
    unsigned __int8 *mem; // [esp+0h] [ebp-4h]

    Sys_AssistAndWaitWorkerCmdInternal(&Demo_SaveWorkerCmd);
    MSG_WriteBits(&demo.msg, 0, 2u);
    cursize = demo.msg.cursize;
    ActiveMemoryBuffer = Demo_GetActiveMemoryBuffer();
    Demo_Save(ActiveMemoryBuffer, cursize, 0);
    mem = Demo_SwapActiveMemoryBuffer();
    MSG_Init(&demo.msg, mem, 49152);
    v1 = va("DEMO: Switching to buffer %d\n", demo.msgBufFlag);
    Demo_Printf(3, v1);
}

void __cdecl Demo_Save(unsigned __int8 *data, int size, bool writeInfoFile)
{
    demoSaveCmd cmd; // [esp+0h] [ebp-Ch] BYREF

    if ( demo_save_smp->current.enabled )
    {
        cmd.data = data;
        cmd.size = size;
        cmd.writeInfoFile = writeInfoFile;
        Sys_AddWorkerCmdInternal(&Demo_SaveWorkerCmd, (unsigned __int8 *)&cmd, 0);
    }
    else
    {
        Demo_SaveInternal(data, size, writeInfoFile);
    }
}

void __cdecl Demo_SaveInternal(unsigned __int8 *data, int size, bool writeInfoFile)
{
    const char *v3; // eax
    unsigned int compressedSize; // [esp+8h] [ebp-38h]
    unsigned int bytesWritten; // [esp+Ch] [ebp-34h]
    msg_t compressedMsg; // [esp+10h] [ebp-30h] BYREF

    PROF_SCOPED("Demo Recording - Demo_SaveInternal()");

    memset(demo.memBlock.compressedMsgBuf, 0, sizeof(demo.memBlock.compressedMsgBuf));
    MSG_Init(&compressedMsg, demo.memBlock.compressedMsgBuf, 49152);
    compressedSize = MSG_CompressWithZLib(data, size, demo.memBlock.compressedMsgBuf, 0xC000u);
    demo.memBlock.compressedSize = compressedSize;
    demo.memBlock.uncompressedSize = size;
    bytesWritten = Demo_Write((char *)&demo.memBlock, compressedSize + 8, demo.demoFileHandle);
    demo.totalSize += bytesWritten;
    v3 = va(
                 "DEMO: CurSize: %d CompressedSize: %d. Written %d bytes to demo file %s.demo\n",
                 size,
                 compressedSize,
                 bytesWritten,
                 demo.demoName);
    Demo_Printf(2, v3);
    Demo_PopulateHeliPatches(&demo.info);
    if ( writeInfoFile )
        Demo_WriteInfoData(demo.demoFileHandle, &demo.info);
}

void __cdecl Demo_StartStreaming(int controllerIndex)
{
    s_uploadStreamData.active = 1;
    s_uploadStreamData.writer = 0;
    s_uploadStreamData.reader = 0;
    s_uploadStreamData.controllerIndex = controllerIndex;
    s_uploadStreamData.overflow = 0;
    s_uploadStreamUsage.changed = 0;
    s_uploadStreamUsage.highwater = 0;
    memset(s_uploadStreamData.sendHistory, 0, sizeof(s_uploadStreamData.sendHistory));
    s_uploadStreamData.sendHistoryIndex = 0;
}

void __cdecl Demo_StopStreaming()
{
    s_uploadStreamData.active = 0;
    Sys_SetDemoStreamingEvent();
}

bool __cdecl Demo_AreDifferencesValid(int diffBefore, int diffAfter)
{
    if ( diffBefore >= 0 && diffAfter > 0 )
        return 1;
    return diffBefore < 0 && diffAfter < 0;
}

void __cdecl Demo_AppendToStreamingHistory(int val, bool isAdded)
{
    int i; // [esp+0h] [ebp-4h]

    // KISAKTODO: this is s_uploadStreamUsage struct -2 offset or something
    //if ( fsShowStreamingGraph->current.enabled )
    //{
    //    for ( i = 1; i < 60; ++i )
    //    {
    //        dword_A7DDB08[2 * i] = *(unsigned int *)&s_uploadStreamUsage.history[i].isDataAdded;
    //        dword_A7DDB0C[2 * i] = s_uploadStreamUsage.history[i].value;
    //    }
    //    s_uploadStreamUsage.history[59].isDataAdded = isAdded;
    //    s_uploadStreamUsage.history[59].value = val;
    //    s_uploadStreamUsage.changed = 1;
    //}

    if ( isAdded && s_uploadStreamUsage.highwater < val )
        s_uploadStreamUsage.highwater = val;
}

int __cdecl Demo_GetCurrUsedBuffer()
{
    if ( s_uploadStreamData.reader == s_uploadStreamData.writer )
        return 0;
    if ( s_uploadStreamData.writer <= s_uploadStreamData.reader )
        return s_uploadStreamData.writer + 393216 - s_uploadStreamData.reader;
    return s_uploadStreamData.writer - s_uploadStreamData.reader;
}

int __cdecl Demo_CurrAvailableBuffer()
{
    return 393216 - Demo_GetCurrUsedBuffer();
}

void __cdecl Demo_SaveToStreamBuffer(unsigned __int8 *data, unsigned int dataSize)
{
    int CurrUsedBuffer; // eax
    volatile int overflowAmount; // [esp+10h] [ebp-14h]
    int fillAmount; // [esp+14h] [ebp-10h]
    int diffBefore; // [esp+18h] [ebp-Ch]

    PROF_SCOPED("Demo_SaveToStreamBuffer");

    if ( !s_uploadStreamData.active )
    {
        return;
    }
    if ( (int)(dataSize + s_uploadStreamData.writer) > 393216 )
    {
        fillAmount = 393216 - s_uploadStreamData.writer;
        overflowAmount = dataSize - (393216 - s_uploadStreamData.writer);
        memcpy(&s_uploadStreamData.buffer[s_uploadStreamData.writer], data, 393216 - s_uploadStreamData.writer);
        memcpy(s_uploadStreamData.buffer, &data[fillAmount], overflowAmount);
        if ( s_uploadStreamData.writer < s_uploadStreamData.reader )
        {
            LiveStorage_FileShare_AbortOperation(s_uploadStreamData.controllerIndex, FILESHARE_LOCATION_POOLEDSTORAGE);
            Live_DelayedComError("Demo streaming buffer overflow (Err 2).");
        }
        s_uploadStreamData.writer = overflowAmount;
        if ( overflowAmount > s_uploadStreamData.reader )
        {
            LiveStorage_FileShare_AbortOperation(s_uploadStreamData.controllerIndex, FILESHARE_LOCATION_POOLEDSTORAGE);
            Live_DelayedComError("Demo streaming buffer overflow (Err 3).");
        }
    }
    else
    {
        memcpy(&s_uploadStreamData.buffer[s_uploadStreamData.writer], data, dataSize);
        diffBefore = s_uploadStreamData.writer - s_uploadStreamData.reader;
        s_uploadStreamData.writer += dataSize;
        if ( !Demo_AreDifferencesValid(diffBefore, s_uploadStreamData.writer - s_uploadStreamData.reader) )
        {
            LiveStorage_FileShare_AbortOperation(s_uploadStreamData.controllerIndex, FILESHARE_LOCATION_POOLEDSTORAGE);
            Live_DelayedComError("Demo streaming buffer overflow.");
        }
        if ( s_uploadStreamData.writer == 393216 )
            s_uploadStreamData.writer = 0;
    }
    s_uploadStreamData.lastWrite = Sys_Milliseconds();
    CurrUsedBuffer = Demo_GetCurrUsedBuffer();
    Demo_AppendToStreamingHistory(CurrUsedBuffer, 1);
    Sys_SetDemoStreamingEvent();
    if ( Demo_CurrAvailableBuffer() < 49152 && !s_uploadStreamData.overflow )
    {
        Com_PrintError(16, "Stopping demo recording due to dangerously low buffer space");
        s_uploadStreamData.overflow = 1;
        Cbuf_InsertText(0, "demo_stoprecord;");
    }
}

void __cdecl Demo_RecordSentPacket(unsigned int size)
{
    int index; // [esp+0h] [ebp-4h]

    if ( s_uploadStreamData.sendHistoryIndex >= 0x64u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_recording.cpp",
                    411,
                    0,
                    "s_uploadStreamData.sendHistoryIndex doesn't index DEMOSTREAMING_SEND_HISTORY\n\t%i not in [0, %i)",
                    s_uploadStreamData.sendHistoryIndex,
                    100) )
    {
        __debugbreak();
    }
    index = s_uploadStreamData.sendHistoryIndex;
    s_uploadStreamData.sendHistory[index].ms = Sys_Milliseconds();
    s_uploadStreamData.sendHistory[index].size = size;
    s_uploadStreamData.sendHistoryIndex = (index + 1) % 100;
}

bool __cdecl Demo_ShouldThrottle()
{
    return s_uploadStreamData.active || s_uploadStreamData.overflow;
}

unsigned int __cdecl Demo_ThrottleStream(unsigned int dataSize)
{
    int v1; // eax
    float v3; // [esp+0h] [ebp-30h]
    float v4; // [esp+4h] [ebp-2Ch]
    int timeUntilThrottle; // [esp+8h] [ebp-28h]
    int bytesPerSecond; // [esp+Ch] [ebp-24h]
    int minInterval; // [esp+10h] [ebp-20h]
    float packetsPerSecond; // [esp+14h] [ebp-1Ch]
    int throttlePointTime; // [esp+18h] [ebp-18h]
    unsigned int currentTime; // [esp+1Ch] [ebp-14h]
    unsigned int index; // [esp+20h] [ebp-10h]
    int historicalBytes; // [esp+24h] [ebp-Ch]
    float howScrewedAreWe; // [esp+28h] [ebp-8h]
    float howScrewedAreWea; // [esp+28h] [ebp-8h]
    int timeSinceLast; // [esp+2Ch] [ebp-4h]

    if (Demo_ShouldThrottle())
    {
        if (s_uploadStreamData.overflow)
        {
            howScrewedAreWe = 0.75f;
        }
        else
        {
            howScrewedAreWea = (float)((float)((float)Demo_GetCurrUsedBuffer() / 393216.0) * 2.0) - 0.2;
            if ((float)(howScrewedAreWea - 1.0) < 0.0)
                v4 = howScrewedAreWea;
            else
                v4 = 1.0f;
            if ((float)(0.0 - howScrewedAreWea) < 0.0)
                v3 = v4;
            else
                v3 = 0.0f;
            howScrewedAreWe = v3;
        }
        packetsPerSecond = (float)((float)(demo_packetsPerSecondMax->current.integer
            - demo_packetsPerSecondMin->current.integer)
            * howScrewedAreWe)
            + (float)demo_packetsPerSecondMin->current.integer;
        bytesPerSecond = demo_bytesPerSecondMin->current.integer
            + (int)(float)((float)(demo_bytesPerSecondMax->current.integer
                - demo_bytesPerSecondMin->current.integer)
                * howScrewedAreWe);
    }
    else
    {
        packetsPerSecond = 99.0f;
        bytesPerSecond = Live_GetUploadSpeed() / 16;
    }
    minInterval = (int)(float)(1000.0 / packetsPerSecond);
    historicalBytes = bytesPerSecond - dataSize;
    index = s_uploadStreamData.sendHistoryIndex - 1;
    if (!s_uploadStreamData.sendHistoryIndex)
        index = 99;
    timeSinceLast = Sys_Milliseconds() - s_uploadStreamData.sendHistory[index].ms;
    if (timeSinceLast < minInterval)
    {
        Com_Printf(16, "DEMO THROTTLE CLOSED FOR INTERVAL: %dms\n", minInterval - timeSinceLast);
        NET_Sleep(minInterval - timeSinceLast);
    }
    currentTime = Sys_Milliseconds();
    throttlePointTime = 0;
    while (index != s_uploadStreamData.sendHistoryIndex)
    {
        if (index >= 0x64
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_recording.cpp",
                502,
                0,
                "index doesn't index DEMOSTREAMING_SEND_HISTORY\n\t%i not in [0, %i)",
                index,
                100))
        {
            __debugbreak();
        }
        if (!s_uploadStreamData.sendHistory[index].size)
            break;
        historicalBytes -= s_uploadStreamData.sendHistory[index].size;
        if (historicalBytes <= 0)
        {
            throttlePointTime = s_uploadStreamData.sendHistory[index].ms;
            break;
        }
        if (--index == -1)
            index = 99;
    }
    timeUntilThrottle = currentTime - throttlePointTime;
    v1 = Demo_CurrAvailableBuffer();
    if ((int)(currentTime - throttlePointTime) >= 1000)
    {
        Com_Printf(16, "DEMO THROTTLE OPEN: %dms, %dbytes. Buffer Available: %d\n", 1000 - timeUntilThrottle, dataSize, v1);
    }
    else
    {
        Com_Printf(
            16,
            "DEMO THROTTLE CLOSED: %dms, %dbytes. Buffer Available: %d\n",
            1000 - timeUntilThrottle,
            dataSize,
            v1);
        NET_Sleep(1000 - timeUntilThrottle);
    }
    return dataSize;
}

int __cdecl Demo_WriteToStream(unsigned __int8 *data, unsigned int dataSize)
{
    int CurrUsedBuffer; // eax
    int bytesToUpload; // [esp+0h] [ebp-4h]

    while ( s_uploadStreamData.reader == s_uploadStreamData.writer )
    {
        if ( s_uploadStreamData.active )
            Sys_WaitForDemoStreamingEvent();
        if ( !s_uploadStreamData.active && s_uploadStreamData.reader == s_uploadStreamData.writer )
            return 0;
    }
    if ( s_uploadStreamData.writer <= s_uploadStreamData.reader )
    {
        bytesToUpload = 393216 - s_uploadStreamData.reader;
        if ( 393216 - s_uploadStreamData.reader <= dataSize )
        {
            memcpy(data, &s_uploadStreamData.buffer[s_uploadStreamData.reader], bytesToUpload);
            s_uploadStreamData.reader = 0;
        }
        else
        {
            bytesToUpload = dataSize;
            memcpy(data, &s_uploadStreamData.buffer[s_uploadStreamData.reader], dataSize);
            s_uploadStreamData.reader += dataSize;
        }
    }
    else
    {
        bytesToUpload = s_uploadStreamData.writer - s_uploadStreamData.reader;
        if ( s_uploadStreamData.writer - s_uploadStreamData.reader > dataSize )
            bytesToUpload = dataSize;
        memcpy(data, &s_uploadStreamData.buffer[s_uploadStreamData.reader], bytesToUpload);
        s_uploadStreamData.reader += bytesToUpload;
    }
    if ( !bytesToUpload
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_recording.cpp",
                    604,
                    0,
                    "%s",
                    "bytesToUpload > 0") )
    {
        __debugbreak();
    }
    if ( !bytesToUpload )
        Com_Error(ERR_DROP, "Demo streaming buffer error.\n");
    CurrUsedBuffer = Demo_GetCurrUsedBuffer();
    Demo_AppendToStreamingHistory(CurrUsedBuffer, 0);
    Demo_RecordSentPacket(bytesToUpload);
    return bytesToUpload;
}

int __cdecl Demo_SaveCallback(jqBatch *batch)
{
    demoSaveCmd *cmd; // [esp+14h] [ebp-4h]

    PROF_SCOPED("Demo_SaveCmd");
    cmd = (demoSaveCmd *)jqLockData(batch);
    Demo_SaveInternal(cmd->data, cmd->size, cmd->writeInfoFile);
    jqUnlockData(batch);
    return 0;
}

unsigned __int8 *__cdecl Demo_GetActiveMemoryBuffer()
{
    if ( demo.msgBufFlag )
        return demo.msgBuf1;
    else
        return demo.msgBuf0;
}

unsigned __int8 *__cdecl Demo_SwapActiveMemoryBuffer()
{
    unsigned __int8 *mem; // [esp+0h] [ebp-4h]

    if ( demo.msgBufFlag )
        mem = demo.msgBuf0;
    else
        mem = demo.msgBuf1;
    demo.msgBufFlag = !demo.msgBufFlag;
    return mem;
}

void __cdecl Demo_SetIntermissionPoint(float *origin, float *angles)
{
    demo.intermissionPointOrigin[0] = *origin;
    demo.intermissionPointOrigin[1] = origin[1];
    demo.intermissionPointOrigin[2] = origin[2];
    demo.intermissionPointAngles[0] = *angles;
    demo.intermissionPointAngles[1] = angles[1];
    demo.intermissionPointAngles[2] = angles[2];
}

void __cdecl Demo_GetIntermissionPointOrigin(float (*origin)[3])
{
    (*origin)[0] = demo.intermissionPointOrigin[0];
    (*origin)[1] = demo.intermissionPointOrigin[1];
    (*origin)[2] = demo.intermissionPointOrigin[2];
}

void __cdecl Demo_GetIntermissionPointAngles(float (*angles)[3])
{
    (*angles)[0] = demo.intermissionPointAngles[0];
    (*angles)[1] = demo.intermissionPointAngles[1];
    (*angles)[2] = demo.intermissionPointAngles[2];
}

void __cdecl Demo_RecordPlayerSessionState(int clientNum, int state)
{
    demo.playerSessionState[clientNum] = state;
}

bool __cdecl Demo_IsPlayerSpectating(int clientNum)
{
    return demo.playerSessionState[clientNum] == 2 || demo.playerSessionState[clientNum] == 3;
}

bool __cdecl Demo_IsInFinalKillcam()
{
    int i; // [esp+0h] [ebp-Ch]
    bool result; // [esp+Bh] [ebp-1h]

    result = 1;
    for (i = 0; i < demo.header.maxClients; ++i)
    {
        if (svs.clients[i].header.state >= CS_ACTIVE && svs.clients[i].gentity->client->sess.forceSpectatorClient < 0)
            return 0;
    }
    return result;
}

bool __cdecl Demo_IsPlayerDead(int clientNum)
{
    return demo.ps[clientNum].pm_type >= 9;
}

void __cdecl Demo_ClientConnected(int clientNum)
{
    demo.lastReliableCommandRecorded[clientNum] = 0;
}

int __cdecl Demo_GetDemoClientIndex()
{
    return g_democlientindex;
}

void __cdecl Demo_SetDemoClientIndex(int index)
{
    if ( index >= 0 && index < demo.header.maxClients )
    {
        g_democlientindex = index;
    }
    else
    {
        Com_PrintError(0, "ERROR: can't set demo client. range is 0 to %d.\n", demo.header.maxClients);
        Demo_End(0);
    }
}

bool __cdecl Demo_ShouldWriteToDemoFile()
{
    return Demo_IsEnabled() && demo.msg.cursize >= 5120;
}

bool __cdecl Demo_ShouldBuildDemoSnapshot()
{
    if ( !Demo_IsEnabled() )
        return 0;
    if ( Demo_GetDemoClientState() != 1 )
        return 0;
    if ( demo.lastProcessedTime == -1 )
        return 1;
    return svsHeader.time - demo.lastProcessedTime >= demo_recordingrate->current.integer
            && svs.clients[g_democlientindex].header.netchan.outgoingSequence != demo.lastProcessedMsgNum;
}

void __cdecl Demo_WriteHeader(msg_t *msg, bool isDemoClip)
{
    unsigned __int8 DemoClientIndex; // al
    const char *v3; // eax

    MSG_WriteByte(msg, demo.header.version);
    MSG_WriteByte(msg, demo.header.maxClients);
    DemoClientIndex = Demo_GetDemoClientIndex();
    MSG_WriteByte(msg, DemoClientIndex);
    if ( isDemoClip )
        MSG_WriteBit1(msg);
    else
        MSG_WriteBit0(msg);
    if ( demo.header.settings.onlinegame )
        MSG_WriteBit1(msg);
    else
        MSG_WriteBit0(msg);
    if ( demo.header.settings.xblive_privatematch )
        MSG_WriteBit1(msg);
    else
        MSG_WriteBit0(msg);
    if ( demo.header.settings.xblive_wagermatch )
        MSG_WriteBit1(msg);
    else
        MSG_WriteBit0(msg);
    if ( demo.header.settings.systemlink )
        MSG_WriteBit1(msg);
    else
        MSG_WriteBit0(msg);
    if ( demo.header.settings.customGameMode )
        MSG_WriteBit1(msg);
    else
        MSG_WriteBit0(msg);
    v3 = va("DEMO Header: Version: %d\n", demo.header.version);
    Demo_Printf(3, v3);
}

void __cdecl Demo_BuildDemoSnapshot()
{
    const char *v0; // eax
    int UsedBitCount; // eax
    client_t *client; // [esp+0h] [ebp-18h]
    int lastframe; // [esp+4h] [ebp-14h]
    int initBitsUsed; // [esp+8h] [ebp-10h]
    unsigned __int8 snapFlags; // [esp+10h] [ebp-8h]

    if ( Demo_ShouldWriteToDemoFile() )
        Demo_StartSaveProcess();
    if ( (float)demo.totalSize < (float)((float)(demo_filesizeLimit->current.value * 1024.0) * 1024.0) )
    {
        if ( demo.lastProcessedTime == -1 )
            demo.info.startTime = svs.time;
        client = &svs.clients[g_democlientindex];
        initBitsUsed = MSG_GetUsedBitCount(&demo.msg);
        lastframe = client->header.netchan.outgoingSequence - demo.lastProcessedMsgNum;
        g_snapInfo.clientNum = client - svs.clients;
        g_snapInfo.client = &client->header;
        g_snapInfo.archived = 0;
        v0 = va(
                     "Demo_BuildDemoSnapshot() - msgType: %d msgSeq: %d clientCmd: %d\n",
                     3,
                     client->header.netchan.outgoingSequence,
                     client->lastClientCommand);
        Demo_Printf(1, v0);
        g_snapInfo.snapshotDeltaTime = svsHeader.time;
        snapFlags = svsHeader.snapFlagServerBit;
        if ( client->header.rateDelayed )
            snapFlags = LOBYTE(svsHeader.snapFlagServerBit) | 1;
        if ( !client->header.sendAsActive )
            snapFlags |= 2u;
        Demo_WriteSnapshotStartData(
            0,
            &demo.msg,
            client->header.netchan.outgoingSequence,
            client->lastClientCommand,
            svsHeader.time,
            svsHeader.physicsTime,
            lastframe,
            snapFlags);
        UsedBitCount = MSG_GetUsedBitCount(&demo.msg);
        Demo_RecordProfileData(1, (UsedBitCount - initBitsUsed) / 8);
        Demo_WriteMatchState(&demo.msg);
        Demo_WritePlayerStates(&demo.msg);
        Demo_WritePacketEntities(&demo.msg);
        Demo_WritePacketClients(&demo.msg);
        GlassSv_WriteSnapshotToClient(&demo.msg, demo.lastProcessedTime);
        MSG_WriteBit0(&demo.msg);
        MSG_WriteByte(&demo.msg, 0xEu);
        demo.lastProcessedTime = svsHeader.time;
        demo.lastProcessedMsgNum = client->header.netchan.outgoingSequence;
    }
    else
    {
        Demo_End(0);
    }
}

void __cdecl Demo_WriteSnapshotStartData(
                int localClientNum,
                msg_t *msg,
                int msgSequence,
                int cmdSequence,
                int serverTime,
                int physicsTime,
                unsigned __int8 lastFrame,
                unsigned __int8 snapFlags)
{
    MSG_WriteBits(msg, 3, 2u);
    MSG_ClearLastReferencedEntity(msg);
    MSG_WriteLong(msg, msgSequence);
    MSG_WriteLong(msg, cmdSequence);
    Demo_WriteServerCommands(msg);
    MSG_WriteByte(msg, 0xCu);
    MSG_WriteLong(msg, serverTime);
    MSG_WriteLong(msg, physicsTime);
    MSG_WriteByte(msg, lastFrame);
    MSG_WriteByte(msg, snapFlags);
}

void __cdecl Demo_WriteGamestateToBuffer(int clientNum, int demoMsgType, msg_t *msg, int msgSequence)
{
    char *MapName; // eax
    const char *v5; // eax
    char *v6; // eax
    const char *v7; // eax
    char *s; // [esp+0h] [ebp-4h]

    if ( Demo_IsEnabled() )
    {
        MSG_WriteBits(&demo.msg, demoMsgType, 2u);
        MSG_WriteLong(&demo.msg, msgSequence);
        MSG_Embed(&demo.msg, msg);
        Demo_RecordProfileData(0, msg->cursize);
        Demo_StartSaveProcess();
        if ( clientNum != g_democlientindex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_recording.cpp",
                        1093,
                        0,
                        "%s",
                        "clientNum == g_democlientindex") )
        {
            __debugbreak();
        }
        MapName = UI_GetMapName(demo.info.mapName, 1);
        v5 = va("%s_CAPS", MapName);
        v6 = UI_SafeTranslateString(v5);
        sprintf(demo.info.name, "%s", v6);
        s = UI_GetGameTypeName(demo.info.gameType, 0);
        sprintf(demo.info.description, "%s", s);
        demo.info.type = 0;
        Demo_SetDemoClientState(1);
        v7 = va("DEMO: w Type: %d Client: %d Size: %d msgSeq: %d\n", demoMsgType, clientNum, msg->cursize, msgSequence);
        Demo_Printf(1, v7);
    }
}

bool __cdecl Demo_ShouldFilterServerCommand(int clientNum, const char *cmd)
{
    const char *v2; // eax
    bool result; // al
    const char *v4; // eax
    parseInfo_t *cmdToken; // [esp+8h] [ebp-8h]
    parseInfo_t *cmdTokena; // [esp+8h] [ebp-8h]
    int configStringType; // [esp+Ch] [ebp-4h]

    cmdToken = Com_Parse(&cmd);
    switch ( cmdToken->token[0] )
    {
        case '8':
        case 'L':
        case 'N':
        case 't':
        case 'u':
            v2 = va("DEMO: w ServerCommands Filtering %s since it has commandType %s\n", cmd, cmdToken->token);
            Demo_Printf(4, v2);
            result = 1;
            break;
        case 'R':
            result = 0;
            break;
        case 'd':
            cmdTokena = Com_Parse(&cmd);
            configStringType = atoi(cmdTokena->token);
            if ( Demo_IsPlayerSpectating(clientNum) || Demo_IsPlayerDead(clientNum) )
            {
                result = (configStringType < 2276 || configStringType > 2531)
                            && (configStringType < 3115 || configStringType > 3146)
                            && (configStringType < 1568 || configStringType > 2079)
                            && configStringType != 12;
            }
            else
            {
                if ( configStringType != 14 && (configStringType <= 1549 || configStringType > 1551) )
                    goto LABEL_23;
                v4 = va("DEMO: w ServerCommands Filtering %s since its a configstring of key %s\n", cmd, cmdTokena->token);
                Demo_Printf(4, v4);
                result = 1;
            }
            break;
        default:
            if ( Demo_IsPlayerSpectating(clientNum) || Demo_IsPlayerDead(clientNum) )
                result = 1;
            else
LABEL_23:
                result = 0;
            break;
    }
    return result;
}

void __cdecl Demo_WriteServerCommands(msg_t *msg)
{
    char string[4096]; // [esp+Ch] [ebp-11028h] BYREF
    client_t *v2; // [esp+1010h] [ebp-10024h]
    int clientNum; // [esp+1014h] [ebp-10020h]
    int j; // [esp+1018h] [ebp-1001Ch]
    unsigned int dst[8192]; // [esp+101Ch] [ebp-10018h] BYREF
    char v6; // [esp+901Eh] [ebp-8016h]
    char v7; // [esp+901Fh] [ebp-8015h]
    int v8; // [esp+9020h] [ebp-8014h]
    int v9; // [esp+9024h] [ebp-8010h]
    int v10; // [esp+9028h] [ebp-800Ch]
    unsigned int v11[4097]; // [esp+902Ch] [ebp-8008h]
    int i; // [esp+D030h] [ebp-4004h]
    unsigned int v13[4096]; // [esp+D034h] [ebp-4000h]

    v9 = 0;
    v6 = 0;

    PROF_SCOPED("Demo Recording - Writing Server Commands");

    memset(dst, 0, sizeof(dst));
    for ( clientNum = 0; clientNum < demo.header.maxClients; ++clientNum )
    {
        v2 = &svs.clients[clientNum];
        if ( v2->header.state == CS_ACTIVE )
        {
            for ( i = demo.lastReliableCommandRecorded[clientNum] + 1; i <= v2->reliableSequence; ++i )
            {
                if ( !Demo_ShouldFilterServerCommand(clientNum, v2->reliableCommandInfo[i & 0x7F].cmd) )
                {
                    if ( v6 )
                    {
                        v7 = 0;
                        for ( j = 0; j < v9; ++j )
                        {
                            v10 = v13[j];
                            v8 = v11[j];
                            if ( !I_stricmp(
                                            v2->reliableCommandInfo[i & 0x7F].cmd,
                                            svs.clients[v10].reliableCommandInfo[v8 & 0x7F].cmd) )
                            {
                                v7 = 1;
                                break;
                            }
                        }
                        if ( v7 )
                        {
                            dst[2 * j + (clientNum >> 5)] |= 1 << (clientNum & 0x1F);
                        }
                        else
                        {
                            v13[v9] = clientNum;
                            v11[v9] = i;
                            dst[2 * v9++ + (clientNum >> 5)] |= 1 << (clientNum & 0x1F);
                        }
                    }
                    else
                    {
                        v13[v9] = clientNum;
                        v11[v9] = i;
                        dst[2 * v9++ + (clientNum >> 5)] |= 1 << (clientNum & 0x1F);
                    }
                }
            }
            v6 = 1;
            demo.lastReliableCommandRecorded[clientNum] = v2->reliableSequence;
        }
    }
    if ( v9 > 4096
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_recording.cpp",
                    1257,
                    0,
                    "%s",
                    "numCmds <= MAX_CLIENTS * MAX_RELIABLE_COMMANDS") )
    {
        __debugbreak();
    }
    for ( j = 0; j < v9; ++j )
    {
        v10 = v13[j];
        v8 = v11[j];
        v2 = &svs.clients[v10];
        MSG_WriteByte(msg, 0xAu);
        MSG_WriteLong(msg, dst[2 * j]);
        MSG_WriteString(msg, v2->reliableCommandInfo[v8 & 0x7F].cmd);
        _snprintf(
            string,
            0x1000u,
            "DEMO: w ServerCommands MaskCmdNum: %i %i CmdString: %s\n",
            dst[2 * j],
            dst[2 * j + 1],
            v2->reliableCommandInfo[v8 & 0x7F].cmd);
        Demo_Printf(4, string);
    }
}

void __cdecl Demo_WriteMatchState(msg_t *msg)
{
    const char *v1; // eax
    int bitsUsed; // [esp+10h] [ebp-8h]
    int bitsStart; // [esp+14h] [ebp-4h]
    int savedregs; // [esp+18h] [ebp+0h] BYREF

    PROF_SCOPED("Demo Recording - Writing MatchState");

    g_snapInfo.clientNum = g_democlientindex;
    g_snapInfo.client = &svs.clients[g_democlientindex].header;
    MSG_ClearLastReferencedEntity(msg);
    bitsStart = MSG_GetUsedBitCount(msg);
    MSG_WriteDeltaMatchState(&g_snapInfo, msg, svsHeader.time, &demo.matchState, svsHeader.matchState);
    memcpy(&demo.matchState, svsHeader.matchState, sizeof(demo.matchState));
    bitsUsed = MSG_GetUsedBitCount(msg) - bitsStart;
    v1 = va("DEMO: w Type: MatchState Size: %d bytes\n", bitsUsed / 8);
    Demo_Printf(9, v1);
    Demo_RecordProfileData(2, bitsUsed / 8);
}

void __cdecl Demo_WritePlayerStates(msg_t *msg)
{
    unsigned __int8 *PlayerState; // eax
    int v2; // eax
    const char *v3; // eax
    unsigned __int8 *v4; // eax
    int clientNum; // [esp+8h] [ebp-26C0h]
    int v6; // [esp+10h] [ebp-26B8h]
    bool v7; // [esp+17h] [ebp-26B1h]
    playerState_s dst; // [esp+18h] [ebp-26B0h] BYREF
    int UsedBitCount; // [esp+26C4h] [ebp-4h]

    PROF_SCOPED("Demo Recording - Writing PlayerStates");

    v7 = Demo_IsInFinalKillcam();
    for ( clientNum = 0; clientNum < demo.header.maxClients; ++clientNum )
    {
        g_snapInfo.clientNum = clientNum;
        g_snapInfo.client = &svs.clients[clientNum].header;
        if ( svs.clients[clientNum].header.state == CS_ACTIVE && G_GetPlayerState(clientNum) )
        {
            UsedBitCount = MSG_GetUsedBitCount(msg);
            MSG_WriteBit1(msg);
            if ( Demo_IsPlayerSpectating(clientNum) && Demo_IsPlayerDead(clientNum) || v7 )
            {
                memcpy((unsigned __int8 *)&dst, (unsigned __int8 *)&demo.ps[clientNum], sizeof(dst));
                dst.eFlags |= 2u;
                dst.stats[4] = 12;
                MSG_WriteDeltaPlayerstate(&g_snapInfo, msg, svsHeader.time, &demo.ps[clientNum], &dst);
            }
            else
            {
                PlayerState = (unsigned __int8 *)G_GetPlayerState(clientNum);
                memcpy((unsigned __int8 *)&dst, PlayerState, sizeof(dst));
                if ( Demo_IsPlayerSpectating(clientNum) )
                {
                    memcpy((unsigned __int8 *)&dst, (unsigned __int8 *)&g_defaultPlayerState, sizeof(dst));
                    dst.clientNum = clientNum;
                    dst.pm_type = 4;
                    dst.eFlags |= 2u;
                    dst.stats[4] = 13;
                    Demo_GetIntermissionPointOrigin((float (*)[3])dst.origin);
                    Demo_GetIntermissionPointAngles((float (*)[3])dst.viewangles);
                }
                else if ( Demo_IsPlayerDead(clientNum) )
                {
                    dst.stats[4] = 12;
                }
                else
                {
                    dst.stats[4] = 14;
                }
                MSG_WriteDeltaPlayerstate(&g_snapInfo, msg, svsHeader.time, &demo.ps[clientNum], &dst);
            }
            v2 = MSG_GetUsedBitCount(msg);
            v6 = v2 - UsedBitCount;
            v3 = va("DEMO: w Type: PlayerState Client: %d Size: %d bytes\n", clientNum, (v2 - UsedBitCount) / 8);
            Demo_Printf(17, v3);
            Demo_RecordProfileData(3, v6 / 8);
            if ( Demo_IsPlayerSpectating(clientNum) || Demo_IsPlayerDead(clientNum) )
            {
                memcpy((unsigned __int8 *)&demo.ps[clientNum], (unsigned __int8 *)&dst, sizeof(demo.ps[clientNum]));
            }
            else
            {
                v4 = (unsigned __int8 *)G_GetPlayerState(clientNum);
                memcpy((unsigned __int8 *)&demo.ps[clientNum], v4, sizeof(demo.ps[clientNum]));
            }
        }
        else
        {
            MSG_WriteBit0(msg);
        }
    }
    g_snapInfo.clientNum = g_democlientindex;
    g_snapInfo.client = &svs.clients[g_democlientindex].header;
}

void __cdecl Demo_WritePacketEntities(msg_t *msg)
{
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    int v7; // eax
    char *v8; // eax
    int v9; // eax
    int j; // [esp+1Ch] [ebp-1044h]
    int i; // [esp+20h] [ebp-1040h]
    DeltaFlags flags; // [esp+24h] [ebp-103Ch]
    int v13; // [esp+28h] [ebp-1038h]
    int v14; // [esp+2Ch] [ebp-1034h]
    int v15; // [esp+30h] [ebp-1030h]
    entityState_s *to; // [esp+34h] [ebp-102Ch]
    int v17; // [esp+38h] [ebp-1028h]
    int number; // [esp+40h] [ebp-1020h]
    archivedEntity_s *from; // [esp+44h] [ebp-101Ch]
    int eFlags; // [esp+48h] [ebp-1018h]
    int v21[1024]; // [esp+50h] [ebp-1010h]
    int value; // [esp+1050h] [ebp-10h]
    int UsedBitCount; // [esp+1054h] [ebp-Ch]
    int v24; // [esp+1058h] [ebp-8h]
    entityState_s *v25; // [esp+105Ch] [ebp-4h]

    value = 0;
    v24 = 0;
    v15 = 0;

    PROF_SCOPED("Demo Recording - Writing EntityStates");

    for (i = 0; i < sv.num_entities; ++i)
    {
        v25 = (entityState_s *)((char *)&sv.gentities->s + i * sv.gentitySize);
        if (LOBYTE(v25[1].number)
            && (*(_DWORD *)&v25[1].lerp.pos.trType || (v25[1].number & 0x10000) == 0)
            && (v25->number >= 32 || !Demo_IsPlayerSpectating(v25->number) && !Demo_IsPlayerDead(v25->number)))
        {
            v21[value++] = i;
        }
    }
    UsedBitCount = MSG_GetUsedBitCount(msg);
    MSG_ClearLastReferencedEntity(msg);
    MSG_WriteBits(msg, value, 0xBu);
    while (v24 < demo.prevNumEntities || v15 < value)
    {
        v25 = (entityState_s *)((char *)&sv.gentities->s + v21[v15] * sv.gentitySize);
        if (v15 < value)
            number = v25->number;
        else
            number = 9999;
        if (v24 < demo.prevNumEntities)
            v14 = demo.ent[v24].number;
        else
            v14 = 9999;
        if (v14 == number)
        {
            from = (archivedEntity_s *)&demo.ent[v24];
            to = v25;
            flags = DELTA_FLAGS_NONE;
            v17 = demo.entClientMask[v24][0];
            eFlags = v25[1].lerp.eFlags;
            v2 = va("EntityState: OldNum = NewNum = %d\n", v14);
            Demo_Printf(32, v2);
        }
        else if (number >= v14)
        {
            from = (archivedEntity_s *)&demo.ent[v24];
            to = 0;
            flags = DELTA_FLAGS_FORCE;
            v17 = demo.entClientMask[v24][0];
            eFlags = 0;
            v4 = va("EntityState: NewNum (%d) > OldNum (%d). Entity is removed\n", number, v14);
            Demo_Printf(32, v4);
        }
        else
        {
            from = &svsHeader.svEntities[number].baseline;
            to = v25;
            if (number != v25->number
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_recording.cpp",
                    1503,
                    0,
                    "%s",
                    "newEntNum == newEnt->number"))
            {
                __debugbreak();
            }
            g_snapInfo.fromBaseline = 1;
            flags = DELTA_FLAGS_FORCE;
            v17 = 0;
            eFlags = v25[1].lerp.eFlags;
            v3 = va("EntityState: NewNum (%d) < OldNum (%d). Writing with baselines\n", number, v14);
            Demo_Printf(32, v3);
        }
        if (to && v17 != eFlags)
            flags = DELTA_FLAGS_FORCE;
        v13 = MSG_GetUsedBitCount(msg);
        MSG_WriteEntity(&g_snapInfo, msg, svsHeader.time, &from->s, to, flags);
        g_snapInfo.fromBaseline = 0;
        if (MSG_GetUsedBitCount(msg) != v13)
        {
            if (to)
            {
                if (v17 == eFlags)
                {
                    MSG_WriteBit0(msg);
                    v6 = va("OldClientMask: (%d) == NewClientMask: (%d). So, writing clientMask bit 0.\n", v17, eFlags);
                }
                else
                {
                    MSG_WriteBit1(msg);
                    MSG_WriteBits(msg, eFlags, 0x20u);
                    v6 = va("Writing ClientMask (%d) for entity %d\n", eFlags, number);
                }
                Demo_Printf(32, v6);
            }
            else
            {
                MSG_WriteBit0(msg);
                demo.entClientMask[v24][0] = 0;
                demo.entClientMask[v24][1] = 0;
                v5 = va("Entity %d is removed. So, writing clientMask bit 0.\n", v14);
                Demo_Printf(32, v5);
            }
        }
        if (v14 == number)
        {
            ++v24;
            ++v15;
        }
        else if (number >= v14)
        {
            ++v24;
        }
        else
        {
            ++v15;
        }
    }
    MSG_WriteEntityIndex(&g_snapInfo, msg, 1023, 10);
    for (j = 0; j < value; ++j)
    {
        v25 = (entityState_s *)((char *)&sv.gentities->s + v21[j] * sv.gentitySize);
        memcpy(&demo.ent[j], v25, sizeof(demo.ent[j]));
        demo.entClientMask[j][0] = v25[1].lerp.eFlags;
    }
    v7 = MSG_GetUsedBitCount(msg);
    v8 = va("DEMO: w Type: PacketEntities NumEntities: %d FirstEntity: 0 Size: %d bits\n", value, v7 - UsedBitCount);
    Demo_Printf(33, v8);
    v9 = MSG_GetUsedBitCount(msg);
    Demo_RecordProfileData(4, (v9 - UsedBitCount) / 8);
    demo.prevNumEntities = value;
}

void __cdecl Demo_WritePacketClients(msg_t *msg)
{
    const char *v1; // eax
    const char *v2; // eax
    int UsedBitCount; // eax
    const char *v4; // eax
    int v5; // eax
    int j; // [esp+10h] [ebp-B0h]
    int i; // [esp+18h] [ebp-A8h]
    int forceWrite; // [esp+1Ch] [ebp-A4h]
    int newClientNum; // [esp+20h] [ebp-A0h]
    int newIndex; // [esp+24h] [ebp-9Ch]
    int clientNums[32]; // [esp+28h] [ebp-98h]
    clientState_s *oldClient; // [esp+A8h] [ebp-18h]
    int oldClientNum; // [esp+ACh] [ebp-14h]
    int initBitsUsed; // [esp+B0h] [ebp-10h]
    int oldIndex; // [esp+B4h] [ebp-Ch]
    clientState_s *newClient; // [esp+B8h] [ebp-8h]
    int numClients; // [esp+BCh] [ebp-4h]

    numClients = 0;
    oldIndex = 0;
    newIndex = 0;
    
    PROF_SCOPED("Demo Recording - Writing ClientStates");

    for ( i = 0; i < demo.header.maxClients; ++i )
    {
        if ( svs.clients[i].header.state >= CS_ACTIVE )
            clientNums[numClients++] = i;
    }
    initBitsUsed = MSG_GetUsedBitCount(msg);
    MSG_ClearLastReferencedEntity(msg);
    MSG_WriteBits(msg, numClients, 5u);
    while ( oldIndex < demo.prevNumClients || newIndex < numClients )
    {
        if ( newIndex < numClients )
            newClientNum = G_GetClientState(clientNums[newIndex])->clientIndex;
        else
            newClientNum = 9999;
        if ( oldIndex < demo.prevNumClients )
            oldClientNum = demo.client[oldIndex].clientIndex;
        else
            oldClientNum = 9999;
        if ( oldClientNum == newClientNum )
        {
            oldClient = &demo.client[oldIndex];
            newClient = G_GetClientState(clientNums[newIndex]);
            forceWrite = 0;
            v1 = va("ClientState: OldNum = NewNum = %d\n", oldClientNum);
            Demo_Printf(64, v1);
        }
        else
        {
            if ( newClientNum >= oldClientNum )
            {
                oldClient = &demo.client[oldIndex];
                newClient = 0;
                forceWrite = 1;
                Demo_InfoPlayerDisconnected(oldClientNum);
                v2 = va("ClientState: NewNum (%d) > OldNum (%d). Client is removed\n", newClientNum, oldClientNum);
            }
            else
            {
                oldClient = 0;
                newClient = G_GetClientState(clientNums[newIndex]);
                forceWrite = 1;
                Demo_InfoPlayerConnected(newClientNum);
                v2 = va("ClientState: NewNum (%d) < OldNum (%d). Writing with baselines\n", newClientNum, oldClientNum);
            }
            Demo_Printf(64, v2);
        }
        MSG_WriteDeltaClient(&g_snapInfo, msg, svsHeader.time, oldClient, newClient, forceWrite);
        if ( oldClientNum == newClientNum )
        {
            ++oldIndex;
            ++newIndex;
        }
        else if ( newClientNum >= oldClientNum )
        {
            ++oldIndex;
        }
        else
        {
            ++newIndex;
        }
    }
    MSG_WriteBit0(msg);
    for ( j = 0; j < numClients; ++j )
        memcpy(&demo.client[j], G_GetClientState(clientNums[j]), sizeof(demo.client[j]));
    UsedBitCount = MSG_GetUsedBitCount(msg);
    v4 = va(
                 "DEMO: w Type: PacketClients NumClients: %d FirstClient: 0 Size: %d bits\n",
                 numClients,
                 UsedBitCount - initBitsUsed);
    Demo_Printf(65, v4);
    v5 = MSG_GetUsedBitCount(msg);
    Demo_RecordProfileData(5, (v5 - initBitsUsed) / 8);
    demo.prevNumClients = numClients;
}

void __cdecl Demo_PopulateStaticInfoData(demoMetaInfo *info)
{
    const char *String; // eax
    const char *v2; // eax
    qtime_s systemTime; // [esp+0h] [ebp-24h] BYREF

    Com_RealTime(&systemTime, 1);
    String = Dvar_GetString("g_gametype");
    I_strncpyz(info->gameType, String, 256);
    v2 = Dvar_GetString("mapname");
    I_strncpyz(info->mapName, v2, 256);
    info->week = systemTime.tm_yday / 7;
    info->month = systemTime.tm_mon + 1;
    info->year = systemTime.tm_year + 1900;
    info->day = systemTime.tm_mday;
    info->hour = systemTime.tm_hour;
    info->min = systemTime.tm_min;
    info->createTime = _time64(0);
}

unsigned int __cdecl Demo_WriteToBuffer(
                unsigned __int8 *to,
                unsigned __int8 *from,
                unsigned int size,
                int bytesWritten,
                int maxBufferSize)
{
    if ( (int)(size + bytesWritten) >= maxBufferSize )
        return 0;
    memcpy(to, from, size);
    return size;
}

unsigned int __cdecl Demo_WriteInfoDataToBuffer(demoMetaInfo *info, unsigned __int8 *buffer, int maxBufferSize)
{
    unsigned int v3; // eax
    int bytesWritten; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittena; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenb; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenc; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittend; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittene; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenf; // [esp+0h] [ebp-10h]
    unsigned int bytesWritteng; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenh; // [esp+0h] [ebp-10h]
    unsigned int bytesWritteni; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenj; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenk; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenl; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenm; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenn; // [esp+0h] [ebp-10h]
    unsigned int bytesWritteno; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenp; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenq; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenr; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittens; // [esp+0h] [ebp-10h]
    int version; // [esp+4h] [ebp-Ch] BYREF
    int totalBytesWritten; // [esp+8h] [ebp-8h] BYREF
    int i; // [esp+Ch] [ebp-4h]

    totalBytesWritten = 0;
    version = demo.header.version;
    bytesWritten = Demo_WriteToBuffer(buffer, (unsigned __int8 *)&version, 4u, 0, maxBufferSize);
    bytesWrittena = bytesWritten
                                + Demo_WriteToBuffer(
                                        &buffer[bytesWritten],
                                        (unsigned __int8 *)&info->startTime,
                                        4u,
                                        bytesWritten,
                                        maxBufferSize);
    bytesWrittenb = bytesWrittena
                                + Demo_WriteToBuffer(
                                        &buffer[bytesWrittena],
                                        (unsigned __int8 *)&info->endTime,
                                        4u,
                                        bytesWrittena,
                                        maxBufferSize);
    bytesWrittenc = bytesWrittenb
                                + Demo_WriteToBuffer(
                                        &buffer[bytesWrittenb],
                                        (unsigned __int8 *)&info->numConnectedPlayersInfoCount,
                                        4u,
                                        bytesWrittenb,
                                        maxBufferSize);
    bytesWrittend = bytesWrittenc
                                + Demo_WriteToBuffer(
                                        &buffer[bytesWrittenc],
                                        (unsigned __int8 *)&info->numDefaultBookmarksCount,
                                        4u,
                                        bytesWrittenc,
                                        maxBufferSize);
    bytesWrittene = bytesWrittend
                                + Demo_WriteToBuffer(
                                        &buffer[bytesWrittend],
                                        (unsigned __int8 *)&info->numHeliPatchesCount,
                                        4u,
                                        bytesWrittend,
                                        maxBufferSize);
    bytesWrittenf = bytesWrittene
                                + Demo_WriteToBuffer(
                                        &buffer[bytesWrittene],
                                        (unsigned __int8 *)info->author,
                                        0x20u,
                                        bytesWrittene,
                                        maxBufferSize);
    bytesWritteng = bytesWrittenf
                                + Demo_WriteToBuffer(
                                        &buffer[bytesWrittenf],
                                        (unsigned __int8 *)info->gameType,
                                        0x100u,
                                        bytesWrittenf,
                                        maxBufferSize);
    bytesWrittenh = bytesWritteng
                                + Demo_WriteToBuffer(
                                        &buffer[bytesWritteng],
                                        (unsigned __int8 *)info->mapName,
                                        0x100u,
                                        bytesWritteng,
                                        maxBufferSize);
    bytesWritteni = bytesWrittenh
                                + Demo_WriteToBuffer(
                                        &buffer[bytesWrittenh],
                                        (unsigned __int8 *)info->name,
                                        0x16u,
                                        bytesWrittenh,
                                        maxBufferSize);
    bytesWrittenj = bytesWritteni
                                + Demo_WriteToBuffer(
                                        &buffer[bytesWritteni],
                                        (unsigned __int8 *)info->description,
                                        0x16u,
                                        bytesWritteni,
                                        maxBufferSize);
    bytesWrittenk = bytesWrittenj
                                + Demo_WriteToBuffer(
                                        &buffer[bytesWrittenj],
                                        (unsigned __int8 *)&info->week,
                                        4u,
                                        bytesWrittenj,
                                        maxBufferSize);
    bytesWrittenl = bytesWrittenk
                                + Demo_WriteToBuffer(
                                        &buffer[bytesWrittenk],
                                        (unsigned __int8 *)&info->month,
                                        4u,
                                        bytesWrittenk,
                                        maxBufferSize);
    bytesWrittenm = bytesWrittenl
                                + Demo_WriteToBuffer(
                                        &buffer[bytesWrittenl],
                                        (unsigned __int8 *)&info->year,
                                        4u,
                                        bytesWrittenl,
                                        maxBufferSize);
    bytesWrittenn = bytesWrittenm
                                + Demo_WriteToBuffer(
                                        &buffer[bytesWrittenm],
                                        (unsigned __int8 *)&info->day,
                                        4u,
                                        bytesWrittenm,
                                        maxBufferSize);
    bytesWritteno = bytesWrittenn
                                + Demo_WriteToBuffer(
                                        &buffer[bytesWrittenn],
                                        (unsigned __int8 *)&info->hour,
                                        4u,
                                        bytesWrittenn,
                                        maxBufferSize);
    totalBytesWritten = bytesWritteno
                                        + Demo_WriteToBuffer(
                                                &buffer[bytesWritteno],
                                                (unsigned __int8 *)&info->min,
                                                4u,
                                                bytesWritteno,
                                                maxBufferSize);
    for ( i = 0; i < info->numConnectedPlayersInfoCount; ++i )
    {
        if ( !info->connectedPlayers[i].disconnectTime )
            info->connectedPlayers[i].disconnectTime = info->endTime;
        bytesWrittenp = Demo_WriteToBuffer(
                                            &buffer[totalBytesWritten],
                                            &info->connectedPlayers[i].playerIndex,
                                            0x18u,
                                            totalBytesWritten,
                                            maxBufferSize);
        totalBytesWritten += bytesWrittenp;
    }
    for ( i = 0; i < info->numDefaultBookmarksCount; ++i )
    {
        bytesWrittenq = Demo_WriteToBuffer(
                                            &buffer[totalBytesWritten],
                                            &info->defaultBookmarks[i].type,
                                            0xCu,
                                            totalBytesWritten,
                                            maxBufferSize);
        totalBytesWritten += bytesWrittenq;
    }
    for ( i = 0; i < info->numHeliPatchesCount; ++i )
    {
        bytesWrittenr = Demo_WriteToBuffer(
                                            &buffer[totalBytesWritten],
                                            (unsigned __int8 *)&info->heliPatches[i],
                                            0x18u,
                                            totalBytesWritten,
                                            maxBufferSize);
        totalBytesWritten += bytesWrittenr;
    }
    v3 = Demo_WriteToBuffer(
                 &buffer[totalBytesWritten],
                 (unsigned __int8 *)&totalBytesWritten,
                 4u,
                 totalBytesWritten,
                 maxBufferSize);
    totalBytesWritten += v3;
    bytesWrittens = Demo_WriteToBuffer(
                                        &buffer[totalBytesWritten],
                                        (unsigned __int8 *)&version,
                                        4u,
                                        totalBytesWritten,
                                        maxBufferSize);
    return bytesWrittens + totalBytesWritten;
}

void __cdecl Demo_WriteInfoData(int handle, demoMetaInfo *info)
{
    const char *v2; // eax
    unsigned int bytesWritten; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittena; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenb; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenc; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittend; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittene; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenf; // [esp+0h] [ebp-10h]
    unsigned int bytesWritteng; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenh; // [esp+0h] [ebp-10h]
    unsigned int bytesWritteni; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenj; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenk; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenl; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenm; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenn; // [esp+0h] [ebp-10h]
    unsigned int bytesWritteno; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenp; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenq; // [esp+0h] [ebp-10h]
    unsigned int bytesWrittenr; // [esp+0h] [ebp-10h]
    int version; // [esp+4h] [ebp-Ch] BYREF
    int totalBytesWritten; // [esp+8h] [ebp-8h] BYREF
    int i; // [esp+Ch] [ebp-4h]

    totalBytesWritten = 0;
    version = demo.header.version;
    bytesWritten = Demo_Write((char *)&version, 4u, handle);
    bytesWrittena = bytesWritten + Demo_Write((char *)&info->startTime, 4u, handle);
    bytesWrittenb = bytesWrittena + Demo_Write((char *)&info->endTime, 4u, handle);
    bytesWrittenc = bytesWrittenb + Demo_Write((char *)&info->numConnectedPlayersInfoCount, 4u, handle);
    bytesWrittend = bytesWrittenc + Demo_Write((char *)&info->numDefaultBookmarksCount, 4u, handle);
    bytesWrittene = bytesWrittend + Demo_Write((char *)&info->numHeliPatchesCount, 4u, handle);
    bytesWrittenf = bytesWrittene + Demo_Write(info->author, 0x20u, handle);
    bytesWritteng = bytesWrittenf + Demo_Write(info->gameType, 0x100u, handle);
    bytesWrittenh = bytesWritteng + Demo_Write(info->mapName, 0x100u, handle);
    bytesWritteni = bytesWrittenh + Demo_Write(info->name, 0x16u, handle);
    bytesWrittenj = bytesWritteni + Demo_Write(info->description, 0x16u, handle);
    bytesWrittenk = bytesWrittenj + Demo_Write((char *)&info->week, 4u, handle);
    bytesWrittenl = bytesWrittenk + Demo_Write((char *)&info->month, 4u, handle);
    bytesWrittenm = bytesWrittenl + Demo_Write((char *)&info->year, 4u, handle);
    bytesWrittenn = bytesWrittenm + Demo_Write((char *)&info->day, 4u, handle);
    bytesWritteno = bytesWrittenn + Demo_Write((char *)&info->hour, 4u, handle);
    totalBytesWritten = bytesWritteno + Demo_Write((char *)&info->min, 4u, handle);
    for ( i = 0; i < info->numConnectedPlayersInfoCount; ++i )
    {
        if ( !info->connectedPlayers[i].disconnectTime )
            info->connectedPlayers[i].disconnectTime = info->endTime;
        bytesWrittenp = Demo_Write((char *)&info->connectedPlayers[i], 0x18u, handle);
        totalBytesWritten += bytesWrittenp;
    }
    for ( i = 0; i < info->numDefaultBookmarksCount; ++i )
    {
        bytesWrittenq = Demo_Write((char *)&info->defaultBookmarks[i], 0xCu, handle);
        totalBytesWritten += bytesWrittenq;
    }
    for ( i = 0; i < info->numHeliPatchesCount; ++i )
    {
        bytesWrittenr = Demo_Write((char *)&info->heliPatches[i], 0x18u, handle);
        totalBytesWritten += bytesWrittenr;
    }
    Demo_Write((char *)&totalBytesWritten, 4u, handle);
    Demo_Write((char *)&version, 4u, handle);
    v2 = va("DEMO Info: Written %d bytes to demo file.\n", totalBytesWritten + 8);
    Demo_Printf(258, v2);
}

void __cdecl Demo_InfoPlayerConnected(int clientNum)
{
    int v1; // edx
    client_t *clients; // eax
    const char *v3; // eax
    int index; // [esp+4h] [ebp-4h]

    index = demo.info.numConnectedPlayersInfoCount;
    if ( demo.info.numConnectedPlayersInfoCount < 50 )
    {
        demo.info.connectedPlayers[demo.info.numConnectedPlayersInfoCount].playerIndex = demo.info.numConnectedPlayersInfoCount;
        demo.info.connectedPlayers[index].clientNum = clientNum;
        demo.info.connectedPlayers[index].connectTime = svs.time;
        v1 = index;
        clients = svs.clients;
        LODWORD(demo.info.connectedPlayers[v1].xuid) = svs.clients[clientNum].dw_userID;
        HIDWORD(demo.info.connectedPlayers[v1].xuid) = HIDWORD(clients[clientNum].dw_userID);
        if ( demo.info.connectedPlayers[index].clientNum != clientNum
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_recording.cpp",
                        2166,
                        0,
                        "%s",
                        "demo.info.connectedPlayers[index].clientNum == clientNum") )
        {
            __debugbreak();
        }
        v3 = va("DEMO Info: Player Index: %d ClientNum: %d Connect Time: %d\n", index, clientNum, svs.time);
        Demo_Printf(256, v3);
        ++demo.info.numConnectedPlayersInfoCount;
    }
}

void __cdecl Demo_InfoPlayerDisconnected(int clientNum)
{
    const char *v1; // eax
    int i; // [esp+0h] [ebp-8h]
    int playerIndex; // [esp+4h] [ebp-4h]

    playerIndex = -1;
    for ( i = 0; i < demo.info.numConnectedPlayersInfoCount; ++i )
    {
        if ( demo.info.connectedPlayers[i].clientNum == clientNum )
            playerIndex = demo.info.connectedPlayers[i].playerIndex;
    }
    if ( playerIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_recording.cpp",
                    2192,
                    0,
                    "%s",
                    "playerIndex != -1") )
    {
        __debugbreak();
    }
    v1 = va("DEMO Info: Player Index: %d ClientNum: %d Disconnected Time: %d\n", playerIndex, clientNum, svs.time);
    Demo_Printf(256, v1);
    demo.info.connectedPlayers[playerIndex].disconnectTime = svs.time;
}

void __cdecl Demo_AddBookmark(int type, int time, int clientNum1, int clientNum2)
{
    const char *v4; // eax
    int index; // [esp+0h] [ebp-4h]

    index = demo.info.numDefaultBookmarksCount;
    if ( demo.info.numDefaultBookmarksCount < 1000 )
    {
        demo.info.defaultBookmarks[demo.info.numDefaultBookmarksCount].type = type;
        demo.info.defaultBookmarks[index].time = time;
        demo.info.defaultBookmarks[index].clientNum1 = clientNum1;
        demo.info.defaultBookmarks[index].clientNum2 = clientNum2;
        if ( (demo.info.defaultBookmarks[index].type != type
             || demo.info.defaultBookmarks[index].clientNum1 != clientNum1
             || demo.info.defaultBookmarks[index].clientNum2 != clientNum2)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_recording.cpp",
                        2220,
                        0,
                        "%s",
                        "demo.info.defaultBookmarks[index].type == type && demo.info.defaultBookmarks[index].clientNum1 == clientNum1"
                        " && demo.info.defaultBookmarks[index].clientNum2 == clientNum2") )
        {
            __debugbreak();
        }
        v4 = va(
                     "DEMO Info: Bookmark - Type: %d Time: %d ClientNum1: %d ClientNum2: %d\n",
                     type,
                     time,
                     clientNum1,
                     clientNum2);
        Demo_Printf(256, v4);
        ++demo.info.numDefaultBookmarksCount;
    }
}

