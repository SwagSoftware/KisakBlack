#include "cl_parse_mp.h"
#include "cl_main_mp.h"
#include <qcommon/common.h>
#include <qcommon/msg.h>
#include "cl_cgame_mp.h"
#include <universal/mem_largelocal.h>
#include <cgame_mp/cg_main_mp.h>
#include <client/client.h>
#include <ui_mp/ui_gametype_custom_mp.h>
#include <qcommon/files.h>
#include <client/cl_console.h>
#include <live/live_storage_win.h>
#include <universal/com_constantconfigstrings.h>
#include <universal/com_files.h>
#include <win32/win_net.h>
#include <physics/rope_gamestate.h>
#include <glass/glass_client.h>
#include <win32/win_main.h>
#include <qcommon/dl_main.h>
#include <qcommon/legacyhacks.h>
#include <client/cl_parse.h>
#include <bgame/bg_misc.h>
#include "cl_input_mp.h"
#include <database/db_registry.h>
#include "sv_client_mp.h"

// this seems slightly wrong in the real binary, guessing it wasn't kept up to date
const char *svc_strings[] =
{
    "svc_nop", // 0
    "svc_gamestate",
    "svc_configstring",
    "svc_gameinformation",
    "svc_baseline",
    "svc_entitycache",
    "svc_dynentstate",
    "svc_destructiblestate",
    "svc_ropestate",
    "svc_glassstate",
    "svc_serverCommand",
    "svc_download",
    "svc_snapshot",
    "svc_servercommands",
    "svc_EOF", // 14
};

char parseDownloadData[2048];

int cl_connectedToPureServer;

void __cdecl SHOWNET(msg_t *msg, char *s)
{
    if ( cl_shownet->current.integer >= 2 )
        Com_Printf(14, "%3i:%s\n", msg->readcount - 1, s);
}

void __cdecl CL_SavePredictedOriginForServerTime(
                clientActive_t *cl,
                int serverTime,
                float *predictedOrigin,
                float *predictedVelocity,
                float *viewangles,
                int bobCycle,
                int movementDir)
{
    unsigned int v7; // ecx
    float *v8; // [esp+0h] [ebp-10h]
    float *velocity; // [esp+4h] [ebp-Ch]
    float *origin; // [esp+8h] [ebp-8h]
    int lastIndex; // [esp+Ch] [ebp-4h]

    v7 = (cl->clientArchiveIndex + 255) % 256;
    lastIndex = v7;
    if ( v7 >= 0x100
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_parse_mp.cpp",
                    106,
                    0,
                    "lastIndex doesn't index CLIENT_ARCHIVE_SIZE\n\t%i not in [0, %i)",
                    v7,
                    256) )
    {
        __debugbreak();
    }
    if ( cl->clientArchive[lastIndex].serverTime != serverTime )
    {
        cl->clientArchive[cl->clientArchiveIndex].serverTime = serverTime;
        origin = cl->clientArchive[cl->clientArchiveIndex].origin;
        *origin = *predictedOrigin;
        origin[1] = predictedOrigin[1];
        origin[2] = predictedOrigin[2];
        velocity = cl->clientArchive[cl->clientArchiveIndex].velocity;
        *velocity = *predictedVelocity;
        velocity[1] = predictedVelocity[1];
        velocity[2] = predictedVelocity[2];
        cl->clientArchive[cl->clientArchiveIndex].bobCycle = bobCycle;
        cl->clientArchive[cl->clientArchiveIndex].movementDir = movementDir;
        v8 = cl->clientArchive[cl->clientArchiveIndex].viewangles;
        *v8 = *viewangles;
        v8[1] = viewangles[1];
        v8[2] = viewangles[2];
        cl->clientArchiveIndex = (cl->clientArchiveIndex + 1) % 256;
        if ( cl->clientArchiveIndex >= 0x100u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_parse_mp.cpp",
                        118,
                        0,
                        "cl->clientArchiveIndex doesn't index CLIENT_ARCHIVE_SIZE\n\t%i not in [0, %i)",
                        cl->clientArchiveIndex,
                        256) )
        {
            __debugbreak();
        }
    }
}

char __cdecl CL_GetPredictedOriginForServerTime(
                clientActive_t *cl,
                int serverTime,
                float *predictedOrigin,
                float *predictedVelocity,
                float *viewangles,
                int *bobCycle,
                int *movementDir)
{
    unsigned int v7; // edx
    unsigned int v9; // ecx
    int index; // [esp+24h] [ebp-8h]
    int indexa; // [esp+24h] [ebp-8h]
    int cmd; // [esp+28h] [ebp-4h]
    int cmda; // [esp+28h] [ebp-4h]

    for ( cmd = 0; cmd < 256; ++cmd )
    {
        v7 = (cl->clientArchiveIndex + 256 - cmd - 1) % 256;
        index = v7;
        if ( v7 >= 0x100
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_parse_mp.cpp",
                        134,
                        0,
                        "index doesn't index CLIENT_ARCHIVE_SIZE\n\t%i not in [0, %i)",
                        v7,
                        256) )
        {
            __debugbreak();
        }
        if ( cl->clientArchive[index].serverTime <= serverTime )
        {
            if ( cl->clientArchive[index].serverTime != serverTime )
                Com_Printf(
                    14,
                    "Couldn't find exact match for servertime %i, using servertime %i\n",
                    serverTime,
                    cl->clientArchive[index].serverTime);
            *predictedOrigin = cl->clientArchive[index].origin[0];
            predictedOrigin[1] = cl->clientArchive[index].origin[1];
            predictedOrigin[2] = cl->clientArchive[index].origin[2];
            *predictedVelocity = cl->clientArchive[index].velocity[0];
            predictedVelocity[1] = cl->clientArchive[index].velocity[1];
            predictedVelocity[2] = cl->clientArchive[index].velocity[2];
            *viewangles = cl->clientArchive[index].viewangles[0];
            viewangles[1] = cl->clientArchive[index].viewangles[1];
            viewangles[2] = cl->clientArchive[index].viewangles[2];
            *bobCycle = cl->clientArchive[index].bobCycle;
            *movementDir = cl->clientArchive[index].movementDir;

            nanassertvec3(predictedOrigin);
            nanassertvec3(predictedVelocity);

            return 1;
        }
    }
    Com_PrintError(14, "Unable to find predicted origin for server time %i.    Here's what we have:\n", serverTime);
    for ( cmda = 0; cmda < 256; ++cmda )
    {
        v9 = (cl->clientArchiveIndex + 256 - cmda - 1) % 256;
        indexa = v9;
        if ( v9 >= 0x100
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_parse_mp.cpp",
                        156,
                        0,
                        "index doesn't index CLIENT_ARCHIVE_SIZE\n\t%i not in [0, %i)",
                        v9,
                        256) )
        {
            __debugbreak();
        }
        Com_PrintError(14, "%i: %i\n", indexa, cl->clientArchive[indexa].serverTime);
    }
    return 0;
}

void __cdecl CL_DeltaClient(
                clientActive_t *cl,
                msg_t *msg,
                int time,
                clSnapshot_t *frame,
                unsigned int newnum,
                clientState_s *old,
                int unchanged)
{
    clientState_s *state; // [esp+8h] [ebp-4h]

    state = &cl->parseClients[cl->parseClientsNum & 0x7FF];
    if ( unchanged )
    {
        memcpy(state, old, sizeof(clientState_s));
    }
    else if ( MSG_ReadDeltaClient(msg, time, old, state, newnum) )
    {
        return;
    }
    ++cl->parseClientsNum;
    ++frame->numClients;
}

void __cdecl CL_ProcessMapCenterFromMatchState(int localClientNum, clSnapshot_t *clSnap)
{
    float *mapCenter; // [esp+0h] [ebp-8h]

    mapCenter = CL_GetLocalClientGlobals(localClientNum)->parseMatchStates[clSnap->parseMatchStateNum & 0x1F].unarchivedState.mapCenter;
    cls.mapCenter[0] = *mapCenter;
    cls.mapCenter[1] = mapCenter[1];
    cls.mapCenter[2] = mapCenter[2];
}

void __cdecl CL_UpdateMapCenterFromMatchState(float *newcenter)
{
    cls.mapCenter[0] = *newcenter;
    cls.mapCenter[1] = newcenter[1];
    cls.mapCenter[2] = newcenter[2];
}

void __cdecl CL_ServerIdChanged(int localClientNum)
{
    char *ConfigString; // eax
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    ConfigString = CL_GetConfigString(3u);
    cls.serverId = atoi(ConfigString);
    LocalClientGlobals->serverId = cls.serverId;
}

void __cdecl CL_SystemInfoChanged(int localClientNum)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *String; // eax
    char *v4; // eax
    char *t; // [esp+4h] [ebp-24h]
    char *ta; // [esp+4h] [ebp-24h]
    LargeLocal value_large_local(0x2000); // [esp+8h] [ebp-20h] BYREF
    const char *systemInfo; // [esp+10h] [ebp-18h]
    char (*key)[8192]; // [esp+14h] [ebp-14h]
    LargeLocal key_large_local(0x2000); // [esp+18h] [ebp-10h] BYREF
    const char *s; // [esp+20h] [ebp-8h] BYREF
    char (*value)[8192]; // [esp+24h] [ebp-4h]

    //LargeLocal::LargeLocal(&key_large_local, 0x2000);
    key = (char (*)[8192])key_large_local.GetBuf(); // LargeLocal::GetBuf(&key_large_local);
    //LargeLocal::LargeLocal(&value_large_local, 0x2000);
    value = (char (*)[8192])value_large_local.GetBuf(); // LargeLocal::GetBuf(&value_large_local);
    systemInfo = CL_GetConfigString(1u);
    if ( CL_GetLocalClientConnection(localClientNum)->demoplaying )
    {
        s = Info_ValueForKey((char *)systemInfo, (char*)"fs_game");
        if ( *s && (v1 = Dvar_GetString("fs_game"), I_strcmp(s, v1)) )
        {
            v2 = va("PATCH_LOADMOD_DEMO", s);
            Com_Error(ERR_DROP, v2);
        }
        else
        {
            String = Dvar_GetString("fs_game");
            if ( I_strcmp(s, String) )
                Dvar_SetStringByName("fs_game", (char *)s);
        }
        goto LABEL_9;
    }
    if ( Demo_IsPlaying() )
    {
LABEL_9:
        //LargeLocal::~LargeLocal(&value_large_local);
        //LargeLocal::~LargeLocal(&key_large_local);
        return;
    }
    if ( !com_sv_running->current.enabled && CL_GetLocalClientConnectionState(localClientNum) < 10 )
    {
        s = Info_ValueForKey((char *)systemInfo, "sv_cheats");
        if ( !atoi(s) )
        {
            Dvar_SetCheatState();
            UI_Gametype_CustomGameModeDataToDvars();
        }
    }
    s = Info_ValueForKey((char *)systemInfo, "sv_iwds");
    t = Info_ValueForKey((char *)systemInfo, "sv_iwdNames");
    if ( FS_PureServerSetLoadedIwds((char *)s, t) )
        cls.doVidRestart = 1;
    s = Info_ValueForKey((char *)systemInfo, "sv_referencedIwds");
    v4 = Info_ValueForKey((char *)systemInfo, "sv_referencedIwdNames");
    FS_ServerSetReferencedIwds(s, v4);
    s = Info_ValueForKey((char *)systemInfo, "sv_referencedFFCheckSums");
    ta = Info_ValueForKey((char *)systemInfo, "sv_referencedFFNames");
    FS_ServerSetReferencedFFs(s, ta);
    if ( !com_sv_running->current.enabled )
    {
        s = systemInfo;
        while ( s )
        {
            Info_NextPair(&s, (char *)key, (char *)value);
            if ( !*(_BYTE *)key )
                break;
            Dvar_SetFromStringByName((const char *)key, (char *)value);
        }
    }
    cl_connectedToPureServer = Dvar_GetBool("sv_pure");
    //LargeLocal::~LargeLocal(&value_large_local);
    //LargeLocal::~LargeLocal(&key_large_local);
}

void __cdecl CL_ParseMapCenter()
{
    char *mapCenterString; // [esp+0h] [ebp-4h]

    mapCenterString = CL_GetConfigString(0xDu);
    sscanf(mapCenterString, "%f %f %f", cls.mapCenter, &cls.mapCenter[1], &cls.mapCenter[2]);
}

void __cdecl CL_ParseGamestate(int localClientNum, msg_t *msg)
{
    unsigned int v4; // [esp+0h] [ebp-1A0h]
    unsigned int v5; // [esp+10h] [ebp-190h]
    unsigned int v6; // [esp+20h] [ebp-180h]
    int nextConstConfigStringIndex; // [esp+34h] [ebp-16Ch]
    int lastStringIndex; // [esp+3Ch] [ebp-164h]
    int numConfigStrings; // [esp+40h] [ebp-160h]
    int nextConstConfigStringNumber; // [esp+44h] [ebp-15Ch]
    int weaponOffset; // [esp+48h] [ebp-158h]
    clientActive_t *LocalClientGlobals; // [esp+4Ch] [ebp-154h]
    unsigned int configStringIndex; // [esp+50h] [ebp-150h]
    unsigned int newnum; // [esp+54h] [ebp-14Ch]
    entityState_s nullstate; // [esp+58h] [ebp-148h] BYREF
    int clientConfigStringChecksum; // [esp+13Ch] [ebp-64h]
    int party; // [esp+140h] [ebp-60h]
    char gametype[32]; // [esp+144h] [ebp-5Ch] BYREF
    clientConnection_t *clc; // [esp+164h] [ebp-3Ch]
    entityState_s *to; // [esp+168h] [ebp-38h]
    int file; // [esp+16Ch] [ebp-34h]
    const char *s; // [esp+170h] [ebp-30h]
    char mapname[32]; // [esp+174h] [ebp-2Ch] BYREF
    int cmd; // [esp+198h] [ebp-8h]
    int serverConfigStringChecksum; // [esp+19Ch] [ebp-4h]

    Con_Close(localClientNum);
    CL_AllocatePerLocalClientMemory();
    clc = CL_GetLocalClientConnection(localClientNum);
    clc->connectPacketCount = 0;
    if ( !clc->demoplaying )
    {
        weaponOffset = 0;
        if ( collectors->current.enabled && presell->current.enabled )
        {
            Dvar_SetIntByName("primaryWeaponOffset", 7);
        }
        else if ( !collectors->current.enabled || presell->current.enabled )
        {
            if ( !collectors->current.enabled && presell->current.enabled )
                weaponOffset = 5;
            Dvar_SetIntByName("primaryWeaponOffset", weaponOffset);
        }
        else
        {
            Dvar_SetIntByName("primaryWeaponOffset", 6);
        }
    }
    CL_ClearState(localClientNum);
    MSG_ClearLastReferencedEntity(msg);
    cls.mapCenter[0] = 0.0;
    cls.mapCenter[1] = 0.0;
    cls.mapCenter[2] = 0.0;
    clc->serverCommandSequence = MSG_ReadLong(msg);
    MSG_ReadString(msg, mapname, 0x20u);
    MSG_ReadString(msg, gametype, 0x20u);
    serverConfigStringChecksum = MSG_ReadLong(msg);
    party = 1;
    if ( CCS_ShouldLoadConstConfigStrings(1) )
        CCS_LoadConstantConfigStrings(mapname, gametype);
    else
        CCS_ClearConstantConfigStrings();
    clientConfigStringChecksum = CCS_GetChecksum();
    CCS_ValidateChecksums(serverConfigStringChecksum, clientConfigStringChecksum);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    memset(cls.gameState.stringOffsets, 0, sizeof(cls.gameState.stringOffsets));
    cls.gameState.dataCount = 1;
    while ( 1 )
    {
        cmd = MSG_ReadByte(msg);
        if ( cmd == svc_EOF)
            break;
        switch ( cmd )
        {
            case svc_configstring:
                lastStringIndex = -1;
                numConfigStrings = MSG_ReadShort(msg);
                nextConstConfigStringIndex = 0;
                for ( nextConstConfigStringNumber = CCS_GetConfigStringNum(0);
                            !nextConstConfigStringNumber && nextConstConfigStringIndex < MAX_CONFIGSTRINGS;
                            nextConstConfigStringNumber = CCS_GetConfigStringNum(nextConstConfigStringIndex) )
                {
                    ++nextConstConfigStringIndex;
                }
                while ( numConfigStrings )
                {
                    if ( MSG_ReadBit(msg) )
                        configStringIndex = lastStringIndex + 1;
                    else
                        configStringIndex = MSG_ReadBits(msg, 0xCu);
                    if ( configStringIndex >= MAX_CONFIGSTRINGS )
                        Com_Error(ERR_DROP, "configstring > MAX_CONFIGSTRINGS");
                    while ( nextConstConfigStringNumber && nextConstConfigStringNumber < (int)configStringIndex )
                    {
                        s = CCS_GetConfigStringValue(nextConstConfigStringIndex);
                        v6 = strlen(s);
                        cls.gameState.stringOffsets[nextConstConfigStringNumber] = cls.gameState.dataCount;
                        memcpy((unsigned __int8 *)&cls.gameState.stringData[cls.gameState.dataCount], (unsigned __int8 *)s, v6 + 1);
                        cls.gameState.dataCount += v6 + 1;
                        nextConstConfigStringNumber = CCS_GetConfigStringNum(++nextConstConfigStringIndex);
                    }
                    if ( nextConstConfigStringNumber && nextConstConfigStringNumber == configStringIndex )
                        nextConstConfigStringNumber = CCS_GetConfigStringNum(++nextConstConfigStringIndex);
                    s = MSG_ReadBigString(msg);
                    v5 = strlen(s);
                    if ( (int)(v5 + cls.gameState.dataCount + 1) > 0x10000 )
                        Com_Error(ERR_DROP, "MAX_GAMESTATE_CHARS exceeded");
                    cls.gameState.stringOffsets[configStringIndex] = cls.gameState.dataCount;
                    memcpy((unsigned __int8 *)&cls.gameState.stringData[cls.gameState.dataCount], (unsigned __int8 *)s, v5 + 1);
                    cls.gameState.dataCount += v5 + 1;
                    lastStringIndex = configStringIndex;
                    Com_Printf(0, "cs[%u]=%u offset=%u (remaining=%i)\n",
                        configStringIndex, lastStringIndex,
                        cls.gameState.stringOffsets[configStringIndex],
                        numConfigStrings);
                    --numConfigStrings;
                }
                while ( nextConstConfigStringNumber )
                {
                    s = CCS_GetConfigStringValue(nextConstConfigStringIndex);
                    v4 = strlen(s);
                    cls.gameState.stringOffsets[nextConstConfigStringNumber] = cls.gameState.dataCount;
                    memcpy((unsigned __int8 *)&cls.gameState.stringData[cls.gameState.dataCount], (unsigned __int8 *)s, v4 + 1);
                    cls.gameState.dataCount += v4 + 1;
                    nextConstConfigStringNumber = CCS_GetConfigStringNum(++nextConstConfigStringIndex);
                }
                CL_ParseMapCenter();
                break;
            case 4:
                newnum = MSG_ReadEntityIndex(msg, 0xAu);
                if ( newnum >= 0x400 )
                    Com_Error(ERR_DROP, "Baseline number out of range: %i", newnum);
                memset(&nullstate, 0, sizeof(nullstate));
                to = &LocalClientGlobals->entityBaselines[newnum];
                MSG_ReadDeltaEntity(msg, 0, &nullstate, to, newnum);
                break;
            case 3:
                cls.gameState.matchUIVisibilityFlags = MSG_ReadLong(msg);
                break;
            default:
                file = FS_FOpenFileWrite((char*)"badpacket.dat");
                if ( file )
                {
                    FS_Write((char *)msg->data, msg->cursize, file);
                    FS_FCloseFile(file);
                }
                Com_PrintError(1, "CL_ParseGamestate: bad command byte %d\n", cmd);
                MSG_Discard(msg);
                return;
        }
    }

    clc->clientNum = MSG_ReadLong(msg);
    if (clc->clientNum >= 32) // KISAKTODO: should probably be com_maxclients instead?
    {
        Com_PrintError(1, "CL_ParseGamestate: bad clientNum %i\n", clc->clientNum);
        clc->clientNum = 0;
        MSG_Discard(msg);
    }

    clc->checksumFeed = MSG_ReadLong(msg);

    if ( useFastFile->current.enabled )
        DB_SyncXAssets();

    CL_ServerIdChanged(localClientNum);
    CL_SystemInfoChanged(localClientNum);

    cls.doVidRestart |= fs_gameDirVar->modified;
    if ( FS_NeedRestart(clc->checksumFeed) )
    {
        UI_CloseAll(0);
        FS_Restart(localClientNum, clc->checksumFeed);
    }

#if 0
    if (net_lanauthorize->current.enabled || !Sys_IsLANAddress(clc->serverAddress))
    {
        //BLOPS_NULLSUB(); CL_RequestAuthorization
    }
#endif

    CL_InitDownloads(localClientNum);

    // New stuff below here
    cmd = MSG_ReadByte(msg);
    iassert(cmd == svc_dynentstate);
    DynEnt_ReadGameState(msg, localClientNum);

    cmd = MSG_ReadByte(msg);
    iassert(cmd == svc_destructiblestate);
    Destructible_ReadGameState(msg, localClientNum);

    cmd = MSG_ReadByte(msg);
    iassert(cmd == svc_ropestate);
    Rope_ReadGameState(msg, localClientNum);

    cmd = MSG_ReadByte(msg);
    iassert(cmd == svc_glassstate);
    GlassCl_ReadGameState(localClientNum, msg);

    if ( UI_Gametype_Custom_ReadClientData(msg) && !UI_Gametype_CanUseCustom() )
        Com_Error(ERR_DROP, "EXE_TRANSMITERROR");

    Dvar_SetInt((dvar_s *)cl_paused, 0);
}

void __cdecl CL_ParseWWWDownload(int localClientNum, msg_t *msg)
{
    const char *String; // eax
    char *downloadPath; // [esp+0h] [ebp-50Ch]
    char strBuf[1024]; // [esp+4h] [ebp-508h] BYREF
    char toOSPath[260]; // [esp+404h] [ebp-108h] BYREF

    downloadPath = (char *)fs_homepath->current.integer;
    I_strncpyz(cls.originalDownloadName, cls.downloadName, 64);
    String = MSG_ReadString(msg, strBuf, 0x400u);
    I_strncpyz(cls.downloadName, String, 256);
    cls.downloadSize = MSG_ReadLong(msg);
    cls.downloadFlags = MSG_ReadLong(msg);
    if ( (cls.downloadFlags & 2) != 0 )
    {
        Sys_OpenURL(cls.downloadName, 1);
        Cbuf_AddText(localClientNum, "quit\n");
        CL_AddReliableCommand(localClientNum, "wwwdl bbl8r");
        DL_CancelDownload();
        cls.wwwDlInProgress = 0;
    }
    else
    {
        legacyHacks.cl_downloadSize = cls.downloadSize;
        Com_DPrintf(14, "Server redirected download: %s\n", cls.downloadName);
        cls.wwwDlInProgress = 1;
        CL_AddReliableCommand(localClientNum, "wwwdl ack");
        FS_BuildOSPath(downloadPath, 0, cls.downloadTempName, toOSPath);
        I_strncpyz(cls.downloadTempName, toOSPath, 256);
        if ( !DL_BeginDownload(cls.downloadTempName, cls.downloadName) )
        {
            CL_AddReliableCommand(localClientNum, "wwwdl fail");
            DL_CancelDownload();
            cls.wwwDlInProgress = 0;
            Com_Printf(14, "Failed to initialize download for '%s'\n", cls.downloadName);
        }
        if ( (cls.downloadFlags & 1) != 0 )
        {
            CL_AddReliableCommand(localClientNum, "wwwdl bbl8r");
            cls.wwwDlDisconnected = 1;
        }
    }
}

static unsigned __int8 msgCompressed_buf[0x10000];
void __cdecl CL_ParseServerMessage(int localClientNum, msg_t *msg)
{
    msg_t msgCompressed; // [esp+4h] [ebp-38h] BYREF
    int file; // [esp+34h] [ebp-8h]
    int cmd; // [esp+38h] [ebp-4h]

    if ( cl_shownet->current.integer == 1 )
    {
        Com_Printf(14, "%i ", msg->cursize);
    }
    else if ( cl_shownet->current.integer >= 2 )
    {
        Com_Printf(14, "------------------\n");
    }

    MSG_Init(&msgCompressed, msgCompressed_buf, sizeof(msgCompressed_buf));

    if ( (unsigned int)(msg->cursize - msg->readcount) > sizeof(msgCompressed_buf))
        Com_Error(ERR_DROP, "Compressed msg overflow in CL_ParseServerMessage");

    msgCompressed.cursize = MSG_ReadBitsCompress(
        &msg->data[msg->readcount], 
        msg->cursize - msg->readcount, 
        msgCompressed_buf, 
        sizeof(msgCompressed_buf)
    );

    while ( 2 )
    {
        if (msgCompressed.overflowed)
        {
            MSG_Discard(msg);
            return;
        }

        cmd = MSG_ReadByte(&msgCompressed);

        if ( cmd == svc_EOF || msgCompressed.overflowed )
        {
            SHOWNET(&msgCompressed, (char*)"END OF MESSAGE");
            if ( msgCompressed.overflowed )
                MSG_Discard(msg);
            return;
        }

        if ( cl_shownet->current.integer >= 2 )
        {
            if ( svc_strings[cmd] )
                SHOWNET(&msgCompressed, (char*)svc_strings[cmd]);
            else
                Com_Printf(14, "%3i:BAD CMD %i\n", msgCompressed.readcount - 1, cmd);
        }

        switch ( cmd )
        {
            case svc_nop:
                continue;
            case svc_gamestate:
                CL_ParseGamestate(localClientNum, &msgCompressed); // contains newer ops like ropestate, glassstate, etc
                continue;
            case svc_serverCommand:
                CL_ParseCommandString(localClientNum, &msgCompressed);
                continue;
            case svc_download:
                CL_ParseDownload(localClientNum, &msgCompressed);
                continue;
            case svc_snapshot:
                CL_ParseSnapshot(localClientNum, &msgCompressed);
                continue;

            default:
                file = FS_FOpenFileWrite((char *)"badpacket.dat");
                if ( file )
                {
                    FS_Write((char *)msg->data, msg->cursize, file);
                    FS_FCloseFile(file);
                }
                Com_PrintError(1, "CL_ParseServerMessage: Illegible server message %d\n", cmd);
                MSG_Discard(msg);
                break;
        }
        break;
    }
}

static clSnapshot_t newSnap;
void __cdecl CL_ParseSnapshot(int localClientNum, msg_t *msg)
{
    const char *v2; // eax
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-24h]
    int serverTimeBackup; // [esp+4h] [ebp-20h]
    clientConnection_t *clc; // [esp+8h] [ebp-1Ch]
    int deltaNum; // [esp+Ch] [ebp-18h]
    int oldMessageNum; // [esp+10h] [ebp-14h]
    int i; // [esp+14h] [ebp-10h]
    clSnapshot_t *old; // [esp+1Ch] [ebp-8h]
    int packetNum; // [esp+20h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    clc = CL_GetLocalClientConnection(localClientNum);
    memset(&newSnap, 0, sizeof(newSnap));
    newSnap.serverCommandNum = clc->serverCommandSequence;
    newSnap.serverTime = MSG_ReadLong(msg);
    newSnap.physicsTime = MSG_ReadLong(msg);
    newSnap.messageNum = clc->serverMessageSequence;
    deltaNum = MSG_ReadByte(msg);
    if (deltaNum)
        newSnap.deltaNum = newSnap.messageNum - deltaNum;
    else
        newSnap.deltaNum = -1;
    newSnap.snapFlags = MSG_ReadByte(msg);
    if (newSnap.deltaNum > 0)
    {
        old = &LocalClientGlobals->snapshots[newSnap.deltaNum & 0x1F];
        if (!old->valid)
        {
            Com_PrintError(14, "Delta from invalid frame (not supposed to happen!).\n");
            MSG_Discard(msg);
            return;
        }
        if (LocalClientGlobals->snapshots[newSnap.deltaNum & 0x1F].messageNum != newSnap.deltaNum)
        {
            Com_DPrintf(14, "Delta frame too old.\n");
            MSG_Discard(msg);
            return;
        }
        if (LocalClientGlobals->parseEntitiesNum - LocalClientGlobals->snapshots[newSnap.deltaNum & 0x1F].parseEntitiesNum > 1920)
        {
            Com_DPrintf(14, "Delta parseEntitiesNum too old.\n");
            MSG_Discard(msg);
            return;
        }
        if (LocalClientGlobals->parseClientsNum - LocalClientGlobals->snapshots[newSnap.deltaNum & 0x1F].parseClientsNum > 1920)
        {
            Com_DPrintf(14, "Delta parseClientsNum too old.\n");
            MSG_Discard(msg);
            return;
        }
        newSnap.valid = 1;
    }
    else
    {
        newSnap.valid = 1;
        old = 0;
        clc->demowaiting = 0;
        clc->demoRequestUncompressedPacket = 0;
        clc->demoReceivedUncompressedPacket = 1;
    }
    serverTimeBackup = LocalClientGlobals->serverTime;
    MSG_ClearLastReferencedEntity(msg);
    CL_ParsePacketMatchState(LocalClientGlobals, msg, newSnap.serverTime, old, &newSnap);
    CL_ProcessMapCenterFromMatchState(localClientNum, &newSnap);
    MSG_ClearLastReferencedEntity(msg);
    SHOWNET(msg, (char *)"playerstate");
    if (old)
        MSG_ReadDeltaPlayerstate(localClientNum, msg, newSnap.serverTime, &old->ps, &newSnap.ps, 1);
    else
        MSG_ReadDeltaPlayerstate(localClientNum, msg, newSnap.serverTime, 0, &newSnap.ps, 1);
    if (serverTimeBackup != LocalClientGlobals->serverTime)
    {
        v2 = va(
            "cl->serverTime changed from %i to %i in MSG_ReadDeltaPlayerstate()\n",
            serverTimeBackup,
            LocalClientGlobals->serverTime);
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_parse_mp.cpp",
            801,
            0,
            "%s\n\t%s",
            "serverTimeBackup == cl->serverTime",
            v2))
            __debugbreak();
    }
    MSG_ClearLastReferencedEntity(msg);
    SHOWNET(msg, (char *)"packet entities");
    CL_ParsePacketEntities(LocalClientGlobals, msg, newSnap.serverTime, old, &newSnap);
    MSG_ClearLastReferencedEntity(msg);
    SHOWNET(msg, (char*)"packet clients");
    CL_ParsePacketClients(LocalClientGlobals, msg, newSnap.serverTime, old, &newSnap);
    if (msg->overflowed)
    {
        newSnap.valid = 0;
    }
    else
    {
        GlassCl_ParseSnapshot(localClientNum, msg);
        if (msg->overflowed)
        {
            newSnap.valid = 0;
        }
        else if (newSnap.valid)
        {
            oldMessageNum = LocalClientGlobals->snap.messageNum + 1;
            if (newSnap.messageNum - oldMessageNum >= 32)
                oldMessageNum = newSnap.messageNum - 31;
            while (oldMessageNum < newSnap.messageNum)
                LocalClientGlobals->snapshots[oldMessageNum++ & 0x1F].valid = 0;
            LocalClientGlobals->oldSnapServerTime = LocalClientGlobals->snap.serverTime;
            memcpy(
                (unsigned __int8 *)&LocalClientGlobals->snap,
                (unsigned __int8 *)&newSnap,
                sizeof(LocalClientGlobals->snap));
            LocalClientGlobals->snap.ping = 999;
            for (i = 0; i < 32; ++i)
            {
                packetNum = ((unsigned __int8)clc->netchan.outgoingSequence - 1 - (_BYTE)i) & 0x1F;
                if (LocalClientGlobals->snap.ps.commandTime >= LocalClientGlobals->outPackets[packetNum].p_serverTime)
                {
                    LocalClientGlobals->snap.ping = cls.realtime - LocalClientGlobals->outPackets[packetNum].p_realtime;
                    break;
                }
            }
            memcpy(
                (unsigned __int8 *)&LocalClientGlobals->snapshots[LocalClientGlobals->snap.messageNum & 0x1F],
                (unsigned __int8 *)&LocalClientGlobals->snap,
                sizeof(LocalClientGlobals->snapshots[LocalClientGlobals->snap.messageNum & 0x1F]));
            if (cl_shownet->current.integer == 3)
                Com_Printf(
                    14,
                    "   snapshot:%i  delta:%i  ping:%i\n",
                    LocalClientGlobals->snap.messageNum,
                    LocalClientGlobals->snap.deltaNum,
                    LocalClientGlobals->snap.ping);
            LocalClientGlobals->newSnapshots = 1;
        }
    }
}

int __cdecl CL_ParsePacketEntities(
                clientActive_t *cl,
                msg_t *msg,
                int time,
                clSnapshot_t *oldframe,
                clSnapshot_t *newframe)
{
    char *EntityTypeName; // eax
    char *v6; // eax
    double v8; // [esp+0h] [ebp-2Ch]
    double v9; // [esp+8h] [ebp-24h]
    double v10; // [esp+10h] [ebp-1Ch]
    entityState_s *oldstate; // [esp+18h] [ebp-14h]
    signed int newnum; // [esp+1Ch] [ebp-10h]
    int oldindex; // [esp+20h] [ebp-Ch]
    int oldnum; // [esp+24h] [ebp-8h]
    int numChanged; // [esp+28h] [ebp-4h]

    newframe->parseEntitiesNum = cl->parseEntitiesNum;
    newframe->numEntities = 0;
    numChanged = 0;
    oldindex = 0;
    oldstate = 0;
    if ( oldframe )
    {
        if ( oldframe->numEntities > 0 )
        {
            oldstate = &cl->parseEntities[oldframe->parseEntitiesNum & 0x7FF];
            oldnum = oldstate->number;
        }
        else
        {
            oldnum = 99999;
        }
    }
    else
    {
        oldnum = 99999;
    }
    while ( !msg->overflowed )
    {
        newnum = MSG_ReadEntityIndex(msg, 0xAu);
        if ( (unsigned int)newnum >= 0x400
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_parse_mp.cpp",
                        354,
                        0,
                        "%s\n\t(newnum) = %i",
                        "(newnum >= 0 && newnum < (1<<10))",
                        newnum) )
        {
            __debugbreak();
        }
        if ( newnum == 1023 )
            break;
        if ( msg->readcount > msg->cursize )
            Com_Error(ERR_DROP, "CL_ParsePacketEntities: end of message");
        while ( oldnum < newnum && !msg->overflowed )
        {
            if ( cl_shownet->current.integer == 3 )
                Com_Printf(14, "%3i:    unchanged: %i\n", msg->readcount, oldnum);
            CL_CopyOldEntity(cl, newframe, oldstate);
            if ( ++oldindex < oldframe->numEntities )
            {
                oldstate = &cl->parseEntities[((_WORD)oldindex + (unsigned __int16)oldframe->parseEntitiesNum) & 0x7FF];
                oldnum = oldstate->number;
            }
            else
            {
                oldnum = 99999;
            }
            if ( msg_dumpEnts->current.enabled )
            {
                EntityTypeName = BG_GetEntityTypeName(oldstate->eType);
                Com_Printf(14, "%3i: unchanged ent, eType %s\n", oldnum, EntityTypeName);
            }
        }
        if ( oldnum == newnum )
        {
            ++numChanged;
            if ( cl_shownet->current.integer == 3 )
                Com_Printf(14, "%3i:    delta: %i\n", msg->readcount, newnum);
            CL_DeltaEntity(cl, msg, time, newframe, newnum, oldstate);
            if ( ++oldindex < oldframe->numEntities )
            {
                oldstate = &cl->parseEntities[((_WORD)oldindex + (unsigned __int16)oldframe->parseEntitiesNum) & 0x7FF];
                oldnum = oldstate->number;
            }
            else
            {
                oldnum = 99999;
            }
        }
        else
        {
            if ( oldnum <= newnum
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_parse_mp.cpp",
                            416,
                            0,
                            "%s",
                            "oldnum > newnum") )
            {
                __debugbreak();
            }
            ++numChanged;
            if ( cl_shownet->current.integer == 3 )
                Com_Printf(14, "%3i:    baseline: %i\n", msg->readcount, newnum);
            CL_DeltaEntity(cl, msg, time, newframe, newnum, &cl->entityBaselines[newnum]);
        }
    }
    while ( oldnum != 99999 && !msg->overflowed )
    {
        if ( cl_shownet->current.integer == 3 )
            Com_Printf(14, "%3i:    unchanged: %i\n", msg->readcount, oldnum);
        CL_CopyOldEntity(cl, newframe, oldstate);
        if ( ++oldindex < oldframe->numEntities )
        {
            oldstate = &cl->parseEntities[((_WORD)oldindex + (unsigned __int16)oldframe->parseEntitiesNum) & 0x7FF];
            oldnum = oldstate->number;
        }
        else
        {
            oldnum = 99999;
        }
        if ( msg_dumpEnts->current.enabled )
        {
            v10 = oldstate->lerp.pos.trBase[2];
            v9 = oldstate->lerp.pos.trBase[1];
            v8 = oldstate->lerp.pos.trBase[0];
            v6 = BG_GetEntityTypeName(oldstate->eType);
            Com_Printf(14, "%3i: unchanged ent, eType %s at %f, %f, %f\n", oldnum, v6, v8, v9, v10);
        }
    }
    if ( cl_shownuments->current.enabled || msg_dumpEnts->current.enabled )
        Com_Printf(14, "Entities in packet: %i\n", newframe->numEntities);
    return numChanged;
}

void __cdecl CL_DeltaEntity(
                clientActive_t *cl,
                msg_t *msg,
                int time,
                clSnapshot_t *frame,
                unsigned int newnum,
                entityState_s *old)
{
    if ( !MSG_ReadDeltaEntity(msg, time, old, &cl->parseEntities[cl->parseEntitiesNum & 0x7FF], newnum) )
    {
        ++cl->parseEntitiesNum;
        ++frame->numEntities;
    }
}

void __cdecl CL_CopyOldEntity(clientActive_t *cl, clSnapshot_t *frame, entityState_s *old)
{
    memcpy(
        &cl->parseEntities[cl->parseEntitiesNum++ & 0x7FF],
        old,
        sizeof(cl->parseEntities[cl->parseEntitiesNum++ & 0x7FF]));
    ++frame->numEntities;
}

void __cdecl CL_ParsePacketClients(
                clientActive_t *cl,
                msg_t *msg,
                int time,
                clSnapshot_t *oldframe,
                clSnapshot_t *newframe)
{
    clientState_s *oldstate; // [esp+0h] [ebp-E8h]
    signed int newnum; // [esp+4h] [ebp-E4h]
    clientState_s dummy; // [esp+8h] [ebp-E0h] BYREF
    int oldindex; // [esp+E0h] [ebp-8h]
    int oldnum; // [esp+E4h] [ebp-4h]

    newframe->parseClientsNum = cl->parseClientsNum;
    newframe->numClients = 0;
    oldindex = 0;
    oldstate = 0;
    if ( oldframe )
    {
        if ( oldindex < oldframe->numClients )
        {
            oldstate = &cl->parseClients[((_WORD)oldindex + (unsigned __int16)oldframe->parseClientsNum) & 0x7FF];
            oldnum = oldstate->clientIndex;
        }
        else
        {
            oldnum = 99999;
        }
    }
    else
    {
        oldnum = 99999;
    }
    while ( !msg->overflowed && MSG_ReadBit(msg) )
    {
        newnum = MSG_ReadEntityIndex(msg, 5u);
        if ( msg->readcount > msg->cursize )
            Com_Error(ERR_DROP, "CL_ParsePacketClients: end of message");
        while ( oldnum < newnum )
        {
            if ( cl_shownet->current.integer == 3 )
                Com_Printf(14, "%3i:    unchanged: %i\n", msg->readcount, oldnum);
            CL_DeltaClient(cl, msg, time, newframe, oldnum, oldstate, 1);
            if ( ++oldindex < oldframe->numClients )
            {
                oldstate = &cl->parseClients[((_WORD)oldindex + (unsigned __int16)oldframe->parseClientsNum) & 0x7FF];
                oldnum = oldstate->clientIndex;
            }
            else
            {
                oldnum = 99999;
            }
        }
        if ( oldnum == newnum )
        {
            if ( cl_shownet->current.integer == 3 )
                Com_Printf(14, "%3i:    delta: %i\n", msg->readcount, newnum);
            CL_DeltaClient(cl, msg, time, newframe, newnum, oldstate, 0);
            if ( ++oldindex < oldframe->numClients )
            {
                oldstate = &cl->parseClients[((_WORD)oldindex + (unsigned __int16)oldframe->parseClientsNum) & 0x7FF];
                oldnum = oldstate->clientIndex;
            }
            else
            {
                oldnum = 99999;
            }
        }
        else
        {
            if ( oldnum <= newnum
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_parse_mp.cpp",
                            565,
                            0,
                            "%s",
                            "oldnum > newnum") )
            {
                __debugbreak();
            }
            if ( cl_shownet->current.integer == 3 )
                Com_Printf(14, "%3i:    baseline: %i\n", msg->readcount, newnum);
            memset(&dummy, 0, sizeof(dummy));
            CL_DeltaClient(cl, msg, time, newframe, newnum, &dummy, 0);
        }
    }
    while ( oldnum != 99999 && !msg->overflowed )
    {
        if ( cl_shownet->current.integer == 3 )
            Com_Printf(14, "%3i:    unchanged: %i\n", msg->readcount, oldnum);
        CL_DeltaClient(cl, msg, time, newframe, oldnum, oldstate, 1);
        if ( ++oldindex < oldframe->numClients )
        {
            oldstate = &cl->parseClients[((_WORD)oldindex + (unsigned __int16)oldframe->parseClientsNum) & 0x7FF];
            oldnum = oldstate->clientIndex;
        }
        else
        {
            oldnum = 99999;
        }
    }
    if ( cl_shownuments->current.enabled )
        Com_Printf(14, "Clients in packet: %i\n", newframe->numClients);
}

void __cdecl CL_ParseDownload(int localClientNum, msg_t *msg)
{
    const char *String; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    int block; // [esp+0h] [ebp-410h]
    int size; // [esp+4h] [ebp-40Ch]
    char strBuf[1024]; // [esp+8h] [ebp-408h] BYREF
    clientConnection_t *clc; // [esp+40Ch] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    block = MSG_ReadLong(msg);
    if ( block == -1 )
    {
        if ( cls.wwwDlInProgress )
        {
            MSG_ReadString(msg, strBuf, 0x400u);
            MSG_ReadLong(msg);
            MSG_ReadLong(msg);
        }
        else
        {
            CL_ParseWWWDownload(localClientNum, msg);
        }
    }
    else
    {
        if ( !block )
        {
            cls.downloadSize = MSG_ReadLong(msg);
            legacyHacks.cl_downloadSize = cls.downloadSize;
            if ( cls.downloadSize < 0 )
            {
                String = MSG_ReadString(msg, strBuf, 0x400u);
                v3 = va("%s", String);
                Com_Error(ERR_DROP, v3);
                return;
            }
        }
        size = MSG_ReadShort(msg);
        if ( size > 0 )
            MSG_ReadData(msg, (unsigned __int8 *)parseDownloadData, size);
        if ( cls.downloadBlock == block )
        {
            if ( cls.download )
                goto LABEL_19;
            if ( !cls.downloadTempName[0] )
            {
                Com_Printf(14, "Server sending download, but no download was requested\n");
                CL_AddReliableCommand(localClientNum, "stopdl");
                return;
            }
            cls.download = FS_SV_FOpenFileWrite(cls.downloadTempName, 0);
            if ( cls.download )
            {
LABEL_19:
                if ( size )
                    FS_Write(parseDownloadData, size, cls.download);
                v5 = va("nextdl %d", cls.downloadBlock);
                CL_AddReliableCommand(localClientNum, v5);
                ++cls.downloadBlock;
                cls.downloadCount += size;
                legacyHacks.cl_downloadCount = cls.downloadCount;
                if ( !size )
                {
                    if ( cls.download )
                    {
                        FS_FCloseFile(cls.download);
                        cls.download = 0;
                        FS_SV_Rename(cls.downloadTempName, cls.downloadName, (char *)"");
                    }
                    cls.downloadName[0] = 0;
                    cls.downloadTempName[0] = 0;
                    legacyHacks.cl_downloadName[0] = 0;
                    CL_WritePacket(localClientNum);
                    CL_WritePacket(localClientNum);
                    CL_NextDownload(localClientNum);
                }
            }
            else
            {
                Com_Printf(14, "Could not create %s\n", cls.downloadTempName);
                CL_AddReliableCommand(localClientNum, "stopdl");
                CL_NextDownload(localClientNum);
            }
        }
        else
        {
            Com_DPrintf(14, "CL_ParseDownload: Expected block %d, got %d\n", cls.downloadBlock, block);
            if ( block > cls.downloadBlock )
            {
                Com_DPrintf(14, "CL_ParseDownload: Sending retransmit request to get the missed block\n");
                v4 = va("retransdl %d", cls.downloadBlock);
                CL_AddReliableCommand(localClientNum, v4);
            }
        }
    }
}

void __cdecl CL_ParseCommandString(int localClientNum, msg_t *msg)
{
    int seq; // [esp+0h] [ebp-414h]
    char strBuf[1024]; // [esp+4h] [ebp-410h] BYREF
    clientConnection_t *clc; // [esp+408h] [ebp-Ch]
    int index; // [esp+40Ch] [ebp-8h]
    const char *s; // [esp+410h] [ebp-4h]

    seq = MSG_ReadLong(msg);
    s = MSG_ReadString(msg, strBuf, 0x400u);
    clc = CL_GetLocalClientConnection(localClientNum);
    if ( clc->serverCommandSequence < seq )
    {
        clc->serverCommandSequence = seq;
        index = seq & 0x7F;
        I_strncpyz(clc->serverCommands[index], s, 1024);
    }
}

