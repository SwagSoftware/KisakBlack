#include "cl_cgame_mp.h"
#include <universal/assertive.h>
#include "cl_main_mp.h"
#include <qcommon/common.h>
#include <cstring>
#include <cgame_mp/cg_local_mp.h>
#include <qcommon/statmonitor.h>
#include <demo/demo_playback.h>
#include <cgame_mp/cg_main_mp.h>
#include <stringed/stringed_hooks.h>
#include <client/cl_console.h>
#include <universal/mem_largelocal.h>
#include "cl_parse_mp.h"
#include <universal/com_files.h>
#include <universal/com_memory.h>
#include <universal/q_parse.h>
#include <qcommon/com_bsp_load_obj.h>
#include <qcommon/cm_load.h>
#include <qcommon/cm_world.h>
#include <cgame_mp/cg_consolecmds_mp.h>
#include <qcommon/mem_track.h>
#include <client/cl_main.h>
#include "cl_pose_mp.h"
#include <xanim/dobj_utils.h>
#include <gfx_d3d/r_model.h>
#include <gfx_d3d/r_material.h>
#include <client/client.h>
#include "cl_scrn_mp.h"
#include <win32/win_shared.h>
#include <live/live_steam.h>
#include <qcommon/dobj_management.h>
#include <client/con_channels.h>
#include <qcommon/com_clients.h>
#include <cgame/cg_main.h>
#include <cgame_mp/cg_snapshot_mp.h>
#include <cgame_mp/cg_servercmds_mp.h>
#include <EffectsCore/fx_system.h>
#include <cgame/cg_drawtools.h>

const float g_color_table[17][4] =
{
  { 0.0, 0.0, 0.0, 1.0 },
  { 1.0, 0.2, 0.2, 1.0 },
  { 0.0, 1.0, 0.0, 1.0 },
  { 1.0, 1.0, 0.5, 1.0 },
  { 0.0, 0.0, 1.0, 1.0 },
  { 0.0, 1.0, 1.0, 1.0 },
  { 1.0, 0.36000001, 1.0, 1.0 },
  { 1.0, 1.0, 1.0, 1.0 },
  { 1.0, 1.0, 1.0, 1.0 },
  { 1.0, 1.0, 1.0, 1.0 },
  { 0.85000002, 0.0, 0.0, 1.0 },
  { 0.2, 0.72000003, 0.47, 1.0 },
  { 0.97000003, 0.57999998, 0.11, 1.0 },
  { 0.0, 0.47, 0.76999998, 1.0 },
  { 0.68000001, 0.75, 0.77999997, 1.0 },
  { 0.5, 0.28, 0.57999998, 1.0 },
  { 0.64999998, 0.34999999, 0.0, 1.0 }
};

char bigConfigString[16384];

void __cdecl CL_GetScreenDimensions(int *width, int *height, float *aspect)
{
    if ( !width
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_cgame_mp.cpp", 121, 0, "%s", "width") )
    {
        __debugbreak();
    }
    if ( !height
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_cgame_mp.cpp", 122, 0, "%s", "height") )
    {
        __debugbreak();
    }
    if ( !aspect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_cgame_mp.cpp", 123, 0, "%s", "aspect") )
    {
        __debugbreak();
    }
    *width = cls.vidConfig.displayWidth;
    *height = cls.vidConfig.displayHeight;
    *aspect = cls.vidConfig.aspectRatioWindow;
}

double __cdecl CL_GetScreenAspectRatioDisplayPixel()
{
    return cls.vidConfig.aspectRatioDisplayPixel;
}

int __cdecl CL_GetUserCmd(int localClientNum, int cmdNumber, usercmd_s *ucmd)
{
    clientActive_t *LocalClientGlobals; // [esp+8h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if ( cmdNumber > LocalClientGlobals->cmdNumber )
        Com_Error(ERR_DROP, "CL_GetUserCmd: %i >= %i", cmdNumber, LocalClientGlobals->cmdNumber);
    if ( cmdNumber <= LocalClientGlobals->cmdNumber - 128 )
        return 0;
    memcpy(ucmd, &LocalClientGlobals->cmds[cmdNumber & 0x7F], sizeof(usercmd_s));
    return 1;
}

void __cdecl CL_AdjustUserCmdAngles(int localClientNum, int cmdNumber, float *adjust)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-18h]
    float angles[3]; // [esp+4h] [ebp-14h]
    usercmd_s *ucmd; // [esp+10h] [ebp-8h]
    int i; // [esp+14h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if ( cmdNumber > LocalClientGlobals->cmdNumber )
        Com_Error(ERR_DROP, "CL_AdjustUserCmdAngles: %i >= %i", cmdNumber, LocalClientGlobals->cmdNumber);
    if ( cmdNumber > LocalClientGlobals->cmdNumber - 128 )
    {
        ucmd = &LocalClientGlobals->cmds[cmdNumber & 0x7F];
        for ( i = 0; i < 3; ++i )
        {
            angles[i] = (float)ucmd->angles[i] * 0.0054931641;
            ucmd->angles[i] = (unsigned __int16)(int)(float)((float)(angles[i] + adjust[i]) * 182.04445);
        }
    }
}

int __cdecl CL_GetCurrentCmdNumber(int localClientNum)
{
    return CL_GetLocalClientGlobals(localClientNum)->cmdNumber;
}

void __cdecl CL_GetCurrentSnapshotNumber(int localClientNum, int *snapshotNumber, int *serverTime)
{
    clientActive_t *LocalClientGlobals; // eax

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    *snapshotNumber = LocalClientGlobals->snap.messageNum;
    *serverTime = LocalClientGlobals->snap.serverTime;
}

int __cdecl CL_GetSnapshot(int localClientNum, int snapshotNumber, snapshot_s *snapshot)
{
    const char *v4; // eax
    unsigned int number; // [esp+8h] [ebp-41Ch]
    clientActive_t *LocalClientGlobals; // [esp+Ch] [ebp-418h]
    int snapshotEntityIndex; // [esp+10h] [ebp-414h]
    bool entityFound[1024]; // [esp+14h] [ebp-410h] BYREF
    const clSnapshot_t *clSnap; // [esp+418h] [ebp-Ch]
    int i; // [esp+41Ch] [ebp-8h]
    int count; // [esp+420h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if ( snapshotNumber > LocalClientGlobals->snap.messageNum )
        Com_Error(ERR_DROP, "CL_GetSnapshot: snapshotNumber > cl->snapshot.messageNum");
    if ( LocalClientGlobals->snap.messageNum - snapshotNumber >= 32 )
        return 0;
    clSnap = &LocalClientGlobals->snapshots[snapshotNumber & 0x1F];
    if ( !clSnap->valid )
        return 0;
    if ( LocalClientGlobals->parseEntitiesNum - clSnap->parseEntitiesNum >= 2048 )
        return 0;
    if ( LocalClientGlobals->parseClientsNum - clSnap->parseClientsNum >= 2048 )
        return 0;
    snapshot->snapFlags = clSnap->snapFlags;
    snapshot->serverCommandSequence = clSnap->serverCommandNum;
    snapshot->ping = clSnap->ping;
    snapshot->serverTime = clSnap->serverTime;
    snapshot->physicsTime = clSnap->physicsTime;
    memcpy((unsigned __int8 *)&snapshot->ps, (unsigned __int8 *)&clSnap->ps, sizeof(snapshot->ps));
    memcpy(
        &snapshot->matchState,
        &LocalClientGlobals->parseMatchStates[clSnap->parseMatchStateNum & 0x1F],
        sizeof(snapshot->matchState));
    count = clSnap->numEntities;
    if ( count > 512 )
    {
        if ( com_statmon->current.enabled )
            StatMon_Warning(4, 3000, (char*)"code_warning_snapshotents");
        else
            Com_DPrintf(14, "CL_GetSnapshot: truncated %i entities to %i\n", count, 512);
        count = 512;
    }
    snapshot->numEntities = count;
    memset(entityFound, 0, sizeof(entityFound));
    snapshotEntityIndex = 0;
    for ( i = 0; i < count; ++i )
    {
        if ( !Demo_IsPlaying()
            || Demo_ShouldEntityBeVisibleToClient(LocalClientGlobals->parseEntities[((_WORD)i
                                                                                                                                                         + (unsigned __int16)clSnap->parseEntitiesNum)
                                                                                                                                                        & 0x7FF].number) )
        {
            memcpy(
                &snapshot->entities[snapshotEntityIndex],
                &LocalClientGlobals->parseEntities[((_WORD)i + (unsigned __int16)clSnap->parseEntitiesNum) & 0x7FF],
                sizeof(snapshot->entities[snapshotEntityIndex]));
            number = snapshot->entities[snapshotEntityIndex].number;
            if ( number >= 0x400
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_cgame_mp.cpp",
                            289,
                            0,
                            "%s\n\t(number) = %i",
                            "(( number >= 0 && number < (1<<10) ))",
                            number) )
            {
                __debugbreak();
            }
            if ( entityFound[number] )
            {
                v4 = va("EntityNum %i was found twice in this snapshot", number);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_cgame_mp.cpp",
                                290,
                                0,
                                "%s\n\t%s",
                                "!entityFound[ number ]",
                                v4) )
                    __debugbreak();
            }
            entityFound[number] = 1;
            ++snapshotEntityIndex;
        }
        else
        {
            --snapshot->numEntities;
        }
    }
    count = clSnap->numClients;
    if ( count > 32 )
        count = 32;
    memset(snapshot->clients, 0xAAu, sizeof(snapshot->clients));
    snapshot->numClients = count;
    for ( i = 0; i < count; ++i )
        memcpy(
            &snapshot->clients[i],
            &LocalClientGlobals->parseClients[((_WORD)i + (unsigned __int16)clSnap->parseClientsNum) & 0x7FF],
            sizeof(snapshot->clients[i]));
    return 1;
}

void __cdecl CL_SetUserCmdWeapons(int localClientNum, int weapon, int offHandIndex, int lastWeaponForAlt)
{
    clientActive_t *LocalClientGlobals; // eax

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals->cgameUserCmdWeapon = weapon;
    LocalClientGlobals->cgameUserCmdOffHandIndex = offHandIndex;
    LocalClientGlobals->cgameUserCmdLastWeaponForAlt = lastWeaponForAlt;
}

void __cdecl CL_SetUserCmdAimValues(int localClientNum, const float *kickAngles)
{
    clientActive_t *LocalClientGlobals; // [esp+4h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals->cgameKickAngles[0] = *kickAngles;
    LocalClientGlobals->cgameKickAngles[1] = kickAngles[1];
    LocalClientGlobals->cgameKickAngles[2] = kickAngles[2];
}

void __cdecl CL_SetUserCmdOrigin(
                int localClientNum,
                const float *origin,
                const float *velocity,
                const float *viewangles,
                int bobCycle,
                int movementDir)
{
    clientActive_t *LocalClientGlobals; // eax

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals->cgamePredictedDataServerTime = LocalClientGlobals->serverTime;
    LocalClientGlobals->cgameOrigin[0] = *origin;
    LocalClientGlobals->cgameOrigin[1] = origin[1];
    LocalClientGlobals->cgameOrigin[2] = origin[2];
    LocalClientGlobals->cgameVelocity[0] = *velocity;
    LocalClientGlobals->cgameVelocity[1] = velocity[1];
    LocalClientGlobals->cgameVelocity[2] = velocity[2];
    LocalClientGlobals->cgameBobCycle = bobCycle;
    LocalClientGlobals->cgameMovementDir = movementDir;
    LocalClientGlobals->cgameViewangles[0] = *viewangles;
    LocalClientGlobals->cgameViewangles[1] = viewangles[1];
    LocalClientGlobals->cgameViewangles[2] = viewangles[2];
}

void __cdecl CL_SetUserCmdVehicle(
                int localClientNum,
                bool inVehicle,
                const float *origin,
                const float *angles,
                const float *tVelocity,
                const float *aVelocity)
{
    clientActive_t *LocalClientGlobals; // eax

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals->cgameVehicle.inVehicle = inVehicle;
    LocalClientGlobals->cgameVehicle.origin[0] = *origin;
    LocalClientGlobals->cgameVehicle.origin[1] = origin[1];
    LocalClientGlobals->cgameVehicle.origin[2] = origin[2];
    LocalClientGlobals->cgameVehicle.angles[0] = *angles;
    LocalClientGlobals->cgameVehicle.angles[1] = angles[1];
    LocalClientGlobals->cgameVehicle.angles[2] = angles[2];
    LocalClientGlobals->cgameVehicle.tVel[0] = *tVelocity;
    LocalClientGlobals->cgameVehicle.tVel[1] = tVelocity[1];
    LocalClientGlobals->cgameVehicle.tVel[2] = tVelocity[2];
    LocalClientGlobals->cgameVehicle.aVel[0] = *aVelocity;
    LocalClientGlobals->cgameVehicle.aVel[1] = aVelocity[1];
    LocalClientGlobals->cgameVehicle.aVel[2] = aVelocity[2];
}

void __cdecl CL_SetFOVSensitivityScale(int localClientNum, float scale)
{
    CL_GetLocalClientGlobals(localClientNum)->cgameFOVSensitivityScale = scale;
}

void __cdecl CL_SetExtraButtons(int localClientNum, bitarray<51> *button_bits)
{
    bitarray<51> *p_cgameExtraButton_bits; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]

    p_cgameExtraButton_bits = &CL_GetLocalClientGlobals(localClientNum)->cgameExtraButton_bits;
    for ( i = 0; i < 2; ++i )
        p_cgameExtraButton_bits->array[i] |= button_bits->array[i];
}

void __cdecl CL_DumpReliableCommands(int localClientNum)
{
    clientConnection_t *clc; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    for ( i = 0; i < 128; ++i )
        Com_PrintError(1, "cmd %5d: '%s'\n", i, clc->serverCommands[i]);
}

int __cdecl CL_CGameNeedsServerCommand(int localClientNum, int serverCommandNumber)
{
    int result; // eax
    const char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // [esp-4h] [ebp-DCh]
    const char *v10; // [esp-4h] [ebp-DCh]
    clientActive_t *LocalClientGlobals; // [esp+C4h] [ebp-14h]
    clientConnection_t *clc; // [esp+C8h] [ebp-10h]
    char *s; // [esp+CCh] [ebp-Ch]
    const char *sa; // [esp+CCh] [ebp-Ch]
    const char *sb; // [esp+CCh] [ebp-Ch]
    const char *cmd; // [esp+D0h] [ebp-8h]
    int argc; // [esp+D4h] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    if ( serverCommandNumber <= clc->serverCommandSequence - 128 )
    {
        if ( clc->demoplaying )
            return 0;
        if ( Demo_IsPlaying() )
            return 0;
        Com_Printf(14, "===== CL_CGameNeedsServerCommand =====\n");
        Com_Printf(14, "serverCommandNumber: %d\n", serverCommandNumber & 0x7F);
        CL_DumpReliableCommands(localClientNum);
        Com_Error(ERR_DROP, "CL_CGameNeedsServerCommand: EXE_ERR_RELIABLE_CYCLED_OUT");
    }
    if ( serverCommandNumber > clc->serverCommandSequence && !Demo_IsPlaying() )
        Com_Error(ERR_DROP, "CL_CGameNeedsServerCommand: EXE_ERR_NOT_RECEIVED");
    s = clc->serverCommands[serverCommandNumber & 0x7F];
    clc->lastExecutedServerCommand = serverCommandNumber;
    if ( cl_showServerCommands->current.enabled )
        Com_DPrintf(14, "serverCommand: %i : %s\n", serverCommandNumber, s);
    while ( 2 )
    {
        Cmd_TokenizeStringNoEval(s);
        cmd = Cmd_Argv(0);
        argc = Cmd_Argc();
        switch ( *cmd )
        {
            case 'B':
            case 'n':
                Con_ClearNotify(localClientNum);
                LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
                memset(LocalClientGlobals->cmds, 0, sizeof(LocalClientGlobals->cmds));
                return 1;
            case 'd':
                Cmd_EndTokenizedString();
                Cmd_TokenizeStringWithLimit(s, 3);
                CL_ConfigstringModified(localClientNum);
                return 1;
            case 'w':
                if ( argc >= 3 && (v3 = Cmd_Argv(2), !_stricmp(v3, "PB")) )
                {
                    v9 = Cmd_Argv(1);
                    v4 = SEH_SafeTranslateString("EXE_SERVERDISCONNECTREASON");
                    v5 = UI_ReplaceConversionString(v4, v9);
                    v6 = va("%s", v5);
                    Com_Error(ERR_SERVERDISCONNECT, v6);
                }
                else if ( argc >= 2 )
                {
                    v7 = Cmd_Argv(1);
                    CL_DisconnectError(v7);
                }
                else
                {
                    Com_Error(ERR_SERVERDISCONNECT, "EXE_SERVER_DISCONNECTED");
                }
                goto $LN7_31;
            case 'x':
$LN7_31:
                Cmd_EndTokenizedString();
                Cmd_TokenizeStringWithLimit(s, 3);
                v10 = Cmd_Argv(2);
                v8 = Cmd_Argv(1);
                Com_sprintf(bigConfigString, 0x4000u, "%c %s %s", 100, v8, v10);
                Cmd_EndTokenizedString();
                result = 0;
                break;
            case 'y':
                Cmd_EndTokenizedString();
                Cmd_TokenizeStringWithLimit(s, 3);
                sa = Cmd_Argv(2);
                if ( strlen(sa) + strlen(bigConfigString) >= 0x4000 )
                    Com_Error(ERR_DROP, "bcs exceeded BIG_INFO_STRING");
                strcat(bigConfigString, sa);
                Cmd_EndTokenizedString();
                result = 0;
                break;
            case 'z':
                Cmd_EndTokenizedString();
                Cmd_TokenizeStringWithLimit(s, 3);
                sb = Cmd_Argv(2);
                if ( strlen(bigConfigString) + strlen(sb) + 1 >= 0x4000 )
                    Com_Error(ERR_DROP, "bcs exceeded BIG_INFO_STRING");
                strcat(bigConfigString, sb);
                s = bigConfigString;
                Cmd_EndTokenizedString();
                continue;
            default:
                result = 1;
                break;
        }
        return result;
    }
}

void __cdecl CL_ConfigstringModified(int localClientNum)
{
    const char *v2; // eax
    unsigned int v3; // [esp+0h] [ebp-5Ch]
    clientActive_t *LocalClientGlobals; // [esp+34h] [ebp-28h]
    gameState_t *oldGs; // [esp+38h] [ebp-24h]
    char *dup; // [esp+3Ch] [ebp-20h]
    int index; // [esp+4Ch] [ebp-10h]
    const char *s; // [esp+50h] [ebp-Ch]
    int i; // [esp+54h] [ebp-8h]
    const char *old; // [esp+58h] [ebp-4h]

    LargeLocal oldGs_large_local(sizeof(gameState_t)); // [esp+44h] [ebp-18h] BYREF
    //LargeLocal::LargeLocal(&oldGs_large_local, 78584);
    //oldGs = (gameState_t *)LargeLocal::GetBuf(&oldGs_large_local);
    oldGs = (gameState_t *)oldGs_large_local.GetBuf();
    v2 = Cmd_Argv(1);
    index = atoi(v2);
    if ((unsigned int)index >= 0xCBC)
        Com_Error(ERR_DROP, "configstring > MAX_CONFIGSTRINGS");
    s = Cmd_Argv(2);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    old = CL_GetConfigString(index);
    if (!strcmp(old, s))
    {
        if (LocalClientGlobals->serverId != cls.serverId && index == 3)
            CL_ServerIdChanged(localClientNum);
    }
    else
    {
        memcpy(oldGs, &cls.gameState, sizeof(gameState_t));
        memset(cls.gameState.stringOffsets, 0, sizeof(cls.gameState.stringOffsets));
        memset(cls.gameState.stringData, 0, sizeof(cls.gameState.stringData));

        cls.gameState.dataCount = 1;

        for (i = 0; i < MAX_CONFIGSTRINGS; ++i)
        {
            if (i == index)
                dup = (char *)s;
            else
                dup = &oldGs->stringData[oldGs->stringOffsets[i]];
            if (*dup)
            {
                v3 = strlen(dup);
                if ((int)(v3 + cls.gameState.dataCount + 1) > 0x10000)
                    Com_Error(ERR_DROP, "MAX_GAMESTATE_CHARS exceeded");
                cls.gameState.stringOffsets[i] = cls.gameState.dataCount;
                memcpy((unsigned __int8 *)&cls.gameState.stringData[cls.gameState.dataCount], (unsigned __int8 *)dup, v3 + 1);
                cls.gameState.dataCount += v3 + 1;
            }
        }
        if (index == 1
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_cgame_mp.cpp",
                466,
                0,
                "%s",
                "index != CS_SYSTEMINFO"))
        {
            __debugbreak();
        }
        if (index == 3)
            CL_ServerIdChanged(localClientNum);
    }
    //LargeLocal::~LargeLocal(&oldGs_large_local);
}

void __cdecl CL_SetExpectedHunkUsage(const char *mapname)
{
    int handle; // [esp+0h] [ebp-18h] BYREF
    const char *memlistfile; // [esp+4h] [ebp-14h]
    char *buf; // [esp+8h] [ebp-10h]
    int len; // [esp+Ch] [ebp-Ch]
    const char *token; // [esp+10h] [ebp-8h]
    const char *buftrav; // [esp+14h] [ebp-4h] BYREF

    memlistfile = "hunkusage.dat";
    len = FS_FOpenFileByMode((char*)"hunkusage.dat", &handle, FS_READ);
    if ( len >= 0 )
    {
        buf = (char *)Z_Malloc(len + 1, "CL_SetExpectedHunkUsage", 11);
        memset(buf, 0, len + 1);
        FS_Read((unsigned __int8 *)buf, len, handle);
        FS_FCloseFile(handle);
        buftrav = buf;
        while ( 1 )
        {
            token = (const char *)Com_Parse(&buftrav);
            if ( !token
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_cgame_mp.cpp", 643, 0, "%s", "token") )
            {
                __debugbreak();
            }
            if ( !*token )
                break;
            if ( !I_stricmp(token, mapname) )
            {
                token = (const char *)Com_Parse(&buftrav);
                if ( token )
                {
                    if ( *token )
                    {
                        com_expectedHunkUsage = atoi(token);
                        Z_Free(buf, 11);
                        return;
                    }
                }
            }
        }
        Z_Free(buf, 11);
    }
    com_expectedHunkUsage = 0;
}

void __cdecl CL_CM_LoadMap(char *mapname)
{
    int checksum; // [esp+0h] [ebp-4h] BYREF

    if ( !useFastFile->current.enabled )
        Com_LoadBsp(mapname);
    CM_LoadMap(mapname, &checksum);
    if ( !com_sv_running->current.enabled )
    {
        CM_LinkWorld();
        Com_LoadWorld(mapname);
    }
}

void __cdecl CL_ShutdownCGame(int localClientNum)
{
    if ( CL_LocalClient_IsCUIFlagSet(localClientNum, 16) )
    {
        CG_Shutdown(localClientNum);
        CL_LocalClient_ClearCUIFlag(localClientNum, 16);
        CL_LocalClient_ClearCUIFlag(localClientNum, 32);
        track_shutdown(1);
    }
    else if ( CL_LocalClient_IsCUIFlagSet(localClientNum, 32) )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_cgame_mp.cpp",
                        699,
                        0,
                        "%s",
                        "!CL_LocalClient_IsCGameInitialized( localClientNum )") )
            __debugbreak();
    }
}

void __cdecl CL_LocalClient_ClearCUIFlag(int localClientNum, int flag)
{
    if ( localClientNum
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\client_mp\\../client/client.h",
                    300,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum >= 0 && localClientNum < 1)",
                    localClientNum) )
    {
        __debugbreak();
    }
    clientUIActives[localClientNum].flags &= ~flag;
}

void __cdecl CL_ShutdownAllClientsCGame()
{
    int client; // [esp+0h] [ebp-4h]

    for ( client = 0; client >= 0; --client )
        CL_ShutdownCGame(client);
    CG_ShutdownOnceForAllClients();
}

int warnCount;
bool __cdecl CL_DObjCreateSkelForBone(DObj *obj, int boneIndex, DObjAnimMat **pMatOut)
{
    char *buf; // [esp+0h] [ebp-Ch]
    unsigned int len; // [esp+4h] [ebp-8h]
    int timeStamp; // [esp+8h] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_cgame_mp.cpp", 741, 0, "%s", "obj") )
        __debugbreak();
    timeStamp = CL_GetSkelTimeStamp();
    if ( DObjSkelExists(obj, timeStamp) )
    {
        *pMatOut = I_dmaGetDObjSkel(obj);
        return DObjSkelIsBoneUpToDate(obj, boneIndex);
    }
    else
    {
        len = DObjGetAllocSkelSize(obj);
        buf = CL_AllocSkelMemory(len);
        if ( buf )
        {
            *pMatOut = (DObjAnimMat *)buf;
            DObjCreateSkel(obj, buf, timeStamp);
            return 0;
        }
        else
        {
            *pMatOut = 0;
            if ( warnCount != timeStamp )
            {
                warnCount = timeStamp;
                Com_PrintWarning(14, "WARNING: CL_SKEL_MEMORY_SIZE exceeded - not calculating skeleton\n");
            }
            return 1;
        }
    }
}

void __cdecl CL_SubtitlePrint(int localClientNum, const char *text, int duration, int lineWidth)
{
    const char *translation; // [esp+0h] [ebp-4h]
    char *translationa; // [esp+0h] [ebp-4h]

    translation = SEH_StringEd_GetString((char*)text);
    if ( translation )
        goto LABEL_8;
    if ( !loc_warnings->current.enabled )
    {
        translation = text;
LABEL_8:
        CL_ConsolePrint(localClientNum, 4, translation, duration, lineWidth, 0);
        return;
    }
    if ( loc_warningsAsErrors->current.enabled )
        Com_Error(ERR_LOCALIZATION, "Could not translate subtitle text: \"%s\"", text);
    else
        Com_PrintWarning(14, "WARNING: Could not translate subtitle text: \"%s\"\n", text);
    translationa = va("^1UNLOCALIZED(^7%s^1)^7", text);
    CL_ConsolePrint(localClientNum, 4, translationa, duration, lineWidth, 0);
}

char *__cdecl CL_GetConfigString(unsigned int configStringIndex)
{
    bcassert(configStringIndex, MAX_CONFIGSTRINGS);
    
    if ( cls.gameState.stringData[0]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_cgame_mp.cpp",
                    850,
                    0,
                    "%s",
                    "!cls.gameState.stringData[0]") )
    {
        __debugbreak();
    }
    return &cls.gameState.stringData[cls.gameState.stringOffsets[configStringIndex]];
}

void __cdecl CL_FinishLoadingModels()
{
    R_FinishLoadingModels();
}

void __cdecl CL_GetDebugViewForward(float *forward)
{
    *forward = cls.debugRenderForward[0];
    forward[1] = cls.debugRenderForward[1];
    forward[2] = cls.debugRenderForward[2];
}

void __cdecl CL_GetDebugViewPos(float *pos)
{
    Vec3Copy(cls.debugRenderPos, pos);
}

void __cdecl CL_RenderScene(refdef_s *fd, int frameTime)
{
    Vec3Copy(fd->vieworg, cls.debugRenderPos);
    Vec3Copy(fd->viewaxis[0], cls.debugRenderForward);

    R_RenderScene(fd, frameTime);
}

void __cdecl CL_RenderMissileCam(const refdef_s *fd, int frameTime)
{
    Vec3Copy(fd->vieworg, cls.debugRenderPos);
    Vec3Copy(fd->viewaxis[0], cls.debugRenderForward);

    R_RenderMissileCam(fd, frameTime);
}

void __cdecl CL_SetCustomConstant(unsigned int type, const float *vec)
{
    R_AddCmdSetCustomConstant(type, vec);
}

void __cdecl CL_DrawStretchPicPhysical(
                float x,
                float y,
                float w,
                float h,
                float s1,
                float t1,
                float s2,
                float t2,
                const float *color,
                Material *material)
{
    if ( !color || color[3] != 0.0 )
        R_AddCmdDrawStretchPic(x, y, w, h, s1, t1, s2, t2, color, material);
}

void __cdecl CL_DrawStretchPicPhysicalW(
                float x,
                float y,
                float w0,
                float w,
                float h,
                float s1,
                float t1,
                float s2,
                float t2,
                const float *color,
                Material *material)
{
    R_AddCmdDrawStretchPicW(x, y, w0, w, h, s1, t1, s2, t2, color, material);
}

void __cdecl CL_DrawStretchPicPhysicalRotateXY(
                float x,
                float y,
                float w,
                float h,
                float s1,
                float t1,
                float s2,
                float t2,
                float angle,
                const float *color,
                Material *material)
{
    if ( !color || color[3] != 0.0 )
        R_AddCmdDrawStretchPicRotateXY(x, y, w, h, s1, t1, s2, t2, angle, color, material);
}

void __cdecl CL_DrawStretchPicPhysicalRotateXYW(
                float x,
                float y,
                float w0,
                float w,
                float h,
                float s1,
                float t1,
                float s2,
                float t2,
                float angle,
                const float *color,
                Material *material)
{
    if ( !color || color[3] != 0.0 )
        R_AddCmdDrawStretchPicRotateXYW(x, y, w0, w, h, s1, t1, s2, t2, angle, color, material);
}

void __cdecl CL_DrawStretchPicW(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w0,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                float s1,
                float t1,
                float s2,
                float t2,
                const float *color,
                Material *material)
{
    if ( !color || color[3] != 0.0 )
    {
        ScrPlace_ApplyRect(scrPlace, &x, &y, &w, &h, horzAlign, vertAlign);
        CL_DrawStretchPicPhysicalW(x, y, w0, w, h, s1, t1, s2, t2, color, material);
    }
}

void __cdecl CL_DrawStretchPic(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                float s1,
                float t1,
                float s2,
                float t2,
                const float *color,
                Material *material)
{
    if ( !color || color[3] != 0.0 )
    {
        ScrPlace_ApplyRect(scrPlace, &x, &y, &w, &h, horzAlign, vertAlign);
        CL_DrawStretchPicPhysical(x, y, w, h, s1, t1, s2, t2, color, material);
    }
}

void __cdecl CL_DrawStretchPicRotatedST(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                float centerS,
                float centerT,
                float radiusST,
                float scaleFinalS,
                float scaleFinalT,
                float angle,
                const float *color,
                Material *material)
{
    if ( !color || color[3] != 0.0 )
    {
        ScrPlace_ApplyRect(scrPlace, &x, &y, &w, &h, horzAlign, vertAlign);
        R_AddCmdDrawStretchPicRotateST(
            x,
            y,
            w,
            h,
            centerS,
            centerT,
            radiusST,
            scaleFinalS,
            scaleFinalT,
            angle,
            color,
            material);
    }
}

void __cdecl CL_DrawStretchPicRotatedXY(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                float angle,
                const float *color,
                Material *material)
{
    if ( !color || color[3] != 0.0 )
    {
        ScrPlace_ApplyRect(scrPlace, &x, &y, &w, &h, 9, 8);
        R_AddCmdDrawStretchPicRotateXY(x, y, w, h, 0.0, 0.0, 1.0, 1.0, angle, color, material);
    }
}

void __cdecl CL_CapTurnRate(int localClientNum, float maxPitchSpeed, float maxYawSpeed)
{
    clientActive_t *LocalClientGlobals; // eax

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals->cgameMaxPitchSpeed = maxPitchSpeed;
    LocalClientGlobals->cgameMaxYawSpeed = maxYawSpeed;
}

void __cdecl CL_SetViewAngles(int localClientNum, const float *angles)
{
    clientActive_t *LocalClientGlobals; // [esp+4h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals->viewangles[0] = *angles;
    LocalClientGlobals->viewangles[1] = angles[1];
    LocalClientGlobals->viewangles[2] = angles[2];
}

void __cdecl CL_AdjustViewAngles(int localClientNum, const float *adjust)
{
    clientActive_t *LocalClientGlobals; // [esp+8h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals->viewangles[0] = LocalClientGlobals->viewangles[0] + *adjust;
    LocalClientGlobals->viewangles[1] = LocalClientGlobals->viewangles[1] + adjust[1];
    LocalClientGlobals->viewangles[2] = LocalClientGlobals->viewangles[2] + adjust[2];
}

void __cdecl CL_ClampViewAngle(int localClientNum, unsigned int angle, float min, float max)
{
    float v4; // [esp+0h] [ebp-10h]
    float v5; // [esp+4h] [ebp-Ch]
    float v6; // [esp+8h] [ebp-8h]
    clientActive_t *LocalClientGlobals; // [esp+Ch] [ebp-4h]

    if ( max < min
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_cgame_mp.cpp", 1059, 0, "%s", "min <= max") )
    {
        __debugbreak();
    }
    if ( angle >= 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_cgame_mp.cpp",
                    1060,
                    0,
                    "angle doesn't index ARRAY_COUNT(cl->viewangles)\n\t%i not in [0, %i)",
                    angle,
                    3) )
    {
        __debugbreak();
    }
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    v5 = LocalClientGlobals->viewangles[angle];
    if ( (float)(v5 - max) < 0.0 )
        v6 = LocalClientGlobals->viewangles[angle];
    else
        v6 = max;
    if ( (float)(min - v5) < 0.0 )
        v4 = v6;
    else
        v4 = min;
    LocalClientGlobals->viewangles[angle] = v4;
}

void __cdecl CL_SyncTimes(int localClientNum)
{
    if ( CL_GetLocalClientConnectionState(localClientNum) == 10 )
        CL_FirstSnapshot(localClientNum);
}

int __cdecl LoadWorld(char *mapname)
{
    bool IsMenuLevel; // al
    int checksum; // [esp+0h] [ebp-4h] BYREF

    IsMenuLevel = Com_IsMenuLevel(0);
    R_LoadWorld(mapname, &checksum, 0, IsMenuLevel);
    if ( !useFastFile->current.enabled )
        Com_UnloadBsp();
    return checksum;
}

void __cdecl CL_UpdateLevelHunkUsage()
{
    int v0; // eax
    const char *v1; // eax
    unsigned int v2; // eax
    int handle; // [esp+20h] [ebp-130h] BYREF
    clientActive_t *LocalClientGlobals; // [esp+24h] [ebp-12Ch]
    const char *memlistfile; // [esp+28h] [ebp-128h]
    char *buf; // [esp+2Ch] [ebp-124h]
    int localClientNum; // [esp+30h] [ebp-120h]
    int len; // [esp+34h] [ebp-11Ch]
    char *outbuftrav; // [esp+38h] [ebp-118h]
    int memusage; // [esp+3Ch] [ebp-114h]
    char outstr[256]; // [esp+40h] [ebp-110h] BYREF
    const char *token; // [esp+144h] [ebp-Ch]
    char *outbuf; // [esp+148h] [ebp-8h]
    const char *buftrav; // [esp+14Ch] [ebp-4h] BYREF

    memlistfile = "hunkusage.dat";
    memusage = Hunk_Used();
    localClientNum = 0;
    LocalClientGlobals = CL_GetLocalClientGlobals(0);
    len = FS_FOpenFileByMode((char*)"hunkusage.dat", &handle, FS_READ);
    if ( len >= 0 )
    {
        buf = (char *)Z_Malloc(len + 1, "CL_UpdateLevelHunkUsage", 11);
        memset(buf, 0, len + 1);
        outbuf = (char *)Z_Malloc(len + 1, "CL_UpdateLevelHunkUsage", 11);
        memset(outbuf, 0, len + 1);
        FS_Read((unsigned __int8 *)buf, len, handle);
        FS_FCloseFile(handle);
        buftrav = buf;
        outbuftrav = outbuf;
        *outbuf = 0;
        while ( 1 )
        {
            token = (const char *)Com_Parse(&buftrav);
            if ( !token || !*token )
                break;
            if ( I_stricmp(token, LocalClientGlobals->mapname) )
            {
                I_strncat(outbuftrav, len + 1, token);
                I_strncat(outbuftrav, len + 1, " ");
                token = (const char *)Com_Parse(&buftrav);
                if ( token && *token )
                {
                    I_strncat(outbuftrav, len + 1, token);
                    I_strncat(outbuftrav, len + 1, "\n");
                }
                else
                {
                    Com_Error(ERR_DROP, "EXE_ERR_HUNGUSAGE_CORRUPT");
                }
            }
            else
            {
                token = (const char *)Com_Parse(&buftrav);
                if ( token )
                {
                    if ( *token )
                    {
                        v0 = atoi(token);
                        if ( v0 == memusage )
                        {
                            Z_Free(buf, 11);
                            Z_Free(outbuf, 11);
                            return;
                        }
                    }
                }
            }
        }
        handle = FS_FOpenFileWrite((char*)memlistfile);
        if ( !handle )
        {
            v1 = va("EXE_ERR_CANT_CREATE", memlistfile);
            Com_Error(ERR_DROP, v1);
        }
        len = strlen(outbuf);
        v2 = FS_Write(outbuf, len, handle);
        if ( v2 != len )
            Com_Error(ERR_DROP, "EXE_ERR_CANT_WRITE", memlistfile);
        FS_FCloseFile(handle);
        Z_Free(buf, 11);
        Z_Free(outbuf, 11);
    }
    FS_FOpenFileByMode((char*)memlistfile, &handle, FS_APPEND);
    if ( !handle )
        Com_Error(ERR_DROP, "EXE_ERR_HUNKUSAGE_CANT_WRITE");
    Com_sprintf(outstr, 0x100u, "%s %i\n", LocalClientGlobals->mapname, memusage);
    FS_Write(outstr, &outstr[strlen(outstr) + 1] - &outstr[1], handle);
    FS_FCloseFile(handle);
    len = FS_FOpenFileByMode((char *)memlistfile, &handle, FS_READ);
    if ( len >= 0 )
        FS_FCloseFile(handle);
}

void    CL_StartLoading()
{
    if ( CL_AnyLocalClientsRunning() )
    {
        CL_InitRenderer();
        CL_StartHunkUsers();
        SCR_UpdateScreen();
    }
}

void __cdecl CL_InitCGame(int localClientNum)
{
    char *v1; // eax
    double v2; // xmm0_8
    int ControllerIndex; // eax
    clientActive_t *LocalClientGlobals; // [esp+Ch] [ebp-5Ch]
    char *info; // [esp+10h] [ebp-58h]
    unsigned int t1; // [esp+14h] [ebp-54h]
    clientConnection_t *clc; // [esp+18h] [ebp-50h]
    char mapname[68]; // [esp+20h] [ebp-48h] BYREF

    t1 = Sys_Milliseconds();
    Con_Close(localClientNum);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    info = CL_GetConfigString(0);
    v1 = Info_ValueForKey(info, (char *)"mapname");
    I_strncpyz(mapname, v1, 64);
    LiveSteam_CheckAccess();
    Dvar_SetStringByName("mapname", mapname);
    Com_GetBspFilename(LocalClientGlobals->mapname, 0x40u, mapname);
    if ( !CL_WasMapAlreadyLoaded() )
    {
        Com_InitDObj();
        if ( useFastFile->current.enabled )
            Com_LoadLevelFastFiles(mapname);
        else
            CL_SetExpectedHunkUsage(LocalClientGlobals->mapname);
    }
    CL_SetLocalClientConnectionState(localClientNum, CA_LOADING);
    Com_Printf(14, "Setting state to CA_LOADING in CL_InitCGame\n");
    Demo_IsPlaying();
    CL_AllocatePerLocalClientMemory();
    CL_LocalClient_SetCUIFlag(localClientNum, 16);
    cl_serverLoadingMap = 0;
    clc = CL_GetLocalClientConnection(localClientNum);
    CG_Init(localClientNum, clc->serverMessageSequence, clc->lastExecutedServerCommand, clc->clientNum);
    CL_LocalClient_SetCUIFlag(localClientNum, 32);
    R_BeginRemoteScreenUpdate();
    CL_SetLocalClientConnectionState(localClientNum, CA_PRIMED);
    v2 = (double)(int)(Sys_Milliseconds() - t1) / 1000.0;
    Com_Printf(14, "CL_InitCGame: %5.2f seconds\n", v2);
    R_EndRegistration();
    Com_TouchMemory();
    Con_ClearNotify(localClientNum);
    Con_InitMessageBuffer();
    Con_InitGameMsgChannels();
    if ( !useFastFile->current.enabled )
    {
        ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        Cmd_ExecuteSingleCommand(localClientNum, ControllerIndex, (char*)"updatehunkusage");
    }
    R_EndRemoteScreenUpdate(0);
    if ( useFastFile->current.enabled )
        DB_SyncXAssets();
    if ( !CG_HasClientSystemBeenInitialzed() )
    {
        Com_InitDynamicMemorySystems();
        R_InitAssets_PostMapFastfileLoad();
    }
    CG_ClientDoneWithInitialization();
}

const char *connectionString_39[11] =
{
  "CA_DISCONNECTED",
  "CA_CINEMATIC",
  "CA_UICINEMATIC",
  "CA_LOGO",
  "CA_CONNECTING",
  "CA_CHALLENGING",
  "CA_CONNECTED",
  "CA_SENDINGSTATS",
  "CA_LOADING",
  "CA_PRIMED",
  "CA_ACTIVE"
};

void __cdecl CL_SetLocalClientConnectionState(int client, connstate_t state)
{
    DWORD v2; // eax
    const char *v3; // [esp-4h] [ebp-4h]

    if (client
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\client_mp\\../client/client.h",
            242,
            0,
            "client doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            client,
            1))
    {
        __debugbreak();
    }
    clientUIActives[client].connectionState = state;
    v3 = connectionString_39[state];
    v2 = Sys_Milliseconds();
    Com_DPrintf(14, "[%08d] %d: %s\n", v2, client, v3);
}

void __cdecl CL_LocalClient_SetCUIFlag(int localClientNum, int flag)
{
    if ( localClientNum
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\client_mp\\../client/client.h",
                    294,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum >= 0 && localClientNum < 1)",
                    localClientNum) )
    {
        __debugbreak();
    }
    clientUIActives[localClientNum].flags |= flag;
}

void __cdecl CL_RestartCGame(int localClientNum)
{
    cg_s *cgameGlob; // [esp+8h] [ebp-14h]
    centity_s *cent; // [esp+Ch] [ebp-10h]
    clientConnection_t *clc; // [esp+10h] [ebp-Ch]
    cgs_t *cgs; // [esp+14h] [ebp-8h]
    int entnum; // [esp+18h] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgameGlob->newPlayerViewmodel = 1;
    cgs->processedSnapshotNum = clc->serverMessageSequence;
    cgs->serverCommandSequence = clc->lastExecutedServerCommand;
    cgs->localServer = com_sv_running->current.color[0];
    cgameGlob->latestSnapshotNum = 0;
    cgameGlob->snap = 0;
    cgameGlob->nextSnap = 0;
    for ( entnum = 0; entnum < 1024; ++entnum )
    {
        cent = CG_GetEntity(localClientNum, entnum);
        *((unsigned int *)cent + 201) &= ~2u;
        CG_ShutdownEntity(localClientNum, cent, 1);
    }
    CG_ShutdownEntity(localClientNum, &cgameGlob->predictedPlayerEntity, 1);
    CG_SetConfigValues(localClientNum);
    CG_ParseServerInfo(localClientNum);
    CG_ParseCodInfo(localClientNum);
    CG_ParseFog(localClientNum);
    CG_ParsePlayerInfos();
    CG_Veh_RegisterMaterials();
    FX_KillAllEffects(localClientNum);
    CL_SetLocalClientConnectionState(localClientNum, CA_PRIMED);
    Com_Printf(14, "Setting state to CA_PRIMED in CL_RestartCGame\n");
}

void __cdecl CL_FirstSnapshot(int localClientNum)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-8h]
    clientConnection_t *clc; // [esp+4h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if ( (LocalClientGlobals->snap.snapFlags & 2) == 0 )
    {
        CG_RegisterSounds();
        clc = CL_GetLocalClientConnection(localClientNum);
        CL_SetLocalClientConnectionState(localClientNum, CA_ACTIVE);
        clc->isServerRestarting = 0;
        UI_CloseAll(localClientNum);
        LocalClientGlobals->serverTimeDelta = LocalClientGlobals->snap.serverTime - cls.realtime;
        LocalClientGlobals->oldServerTime = LocalClientGlobals->snap.serverTime;
        LocalClientGlobals->serverTime = LocalClientGlobals->snap.serverTime;
        clc->timeDemoBaseTime = LocalClientGlobals->snap.serverTime;
        Con_TimeJumped(localClientNum, LocalClientGlobals->serverTime);
        if ( *(_BYTE *)cl_activeAction->current.integer )
        {
            Cbuf_AddText(localClientNum, cl_activeAction->current.string);
            Cbuf_AddText(localClientNum, "\n");
            Dvar_SetString((dvar_s *)cl_activeAction, "");
        }
    }
}

void __cdecl CL_SetCGameTime(int localClientNum)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-10h]
    connstate_t connstate; // [esp+4h] [ebp-Ch]
    char demoReadResult; // [esp+Bh] [ebp-5h]
    clientConnection_t *clc; // [esp+Ch] [ebp-4h]

    demoReadResult = 1;
    connstate = CL_GetLocalClientConnectionState(localClientNum);
    if ( connstate == CA_ACTIVE )
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
        clc = CL_GetLocalClientConnection(localClientNum);
    }
    else
    {
        if ( connstate != CA_PRIMED )
            return;
        LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
        clc = CL_GetLocalClientConnection(localClientNum);
        if ( clc->demoplaying )
        {
            if ( !clc->firstDemoFrameSkipped )
            {
                clc->firstDemoFrameSkipped = 1;
                return;
            }
            CL_ReadDemoMessage(localClientNum);
        }
        if ( Demo_IsPlaying() && !Demo_IsCompleted() )
        {
            if ( !Demo_IsPlaybackInited() )
            {
                Demo_InitPlaybackData(localClientNum);
                Com_ResetFrametime();
                return;
            }
            Demo_ReadDemoMessage(localClientNum);
        }
        if ( LocalClientGlobals->newSnapshots )
        {
            LocalClientGlobals->newSnapshots = 0;
            CL_FirstSnapshot(localClientNum);
        }
        if ( CL_GetLocalClientConnectionState(localClientNum) != 10 )
            return;
    }
    if ( !LocalClientGlobals->snap.valid )
        Com_Error(ERR_DROP, "CL_SetCGameTime: !cl->snap.valid");
    if ( !sv_paused->current.integer || !cl_paused->current.integer || !com_sv_running->current.enabled )
    {
        if ( LocalClientGlobals->snap.serverTime < LocalClientGlobals->oldFrameServerTime )
        {
            if ( I_stricmp(cls.servername, "localhost") )
                Com_Error(ERR_DROP, "cl->snap.serverTime < cl->oldFrameServerTime");
            else
                CL_FirstSnapshot(localClientNum);
        }
        LocalClientGlobals->oldFrameServerTime = LocalClientGlobals->snap.serverTime;
        if ( Demo_IsPlaying() )
        {
            if ( Demo_IsPaused() || Demo_IsCompleted() )
            {
                if ( Demo_IsClipRecording() )
                    Demo_WriteClipCommandsWhenPaused(localClientNum);
                cls.realtime = LocalClientGlobals->serverTime - LocalClientGlobals->serverTimeDelta - 5;
                return;
            }
            if ( (Demo_IsClipPlaying() || Demo_IsClipPreviewRunning()) && Demo_GetClipPausedState() )
            {
                Demo_ReadDemoMessage(localClientNum);
                cls.realtime = LocalClientGlobals->serverTime - LocalClientGlobals->serverTimeDelta - 5;
                return;
            }
        }
        if ( !clc->demoplaying || !cl_freezeDemo->current.enabled )
        {
            LocalClientGlobals->serverTime = LocalClientGlobals->serverTimeDelta + cls.realtime;
            if ( !Demo_IsPlaying() && LocalClientGlobals->serverTime < LocalClientGlobals->oldServerTime )
                LocalClientGlobals->serverTime = LocalClientGlobals->oldServerTime;
            LocalClientGlobals->oldServerTime = LocalClientGlobals->serverTime;
            if ( LocalClientGlobals->serverTimeDelta + cls.realtime >= LocalClientGlobals->snap.serverTime - 5 )
            {
                LocalClientGlobals->extrapolatedSnapshot = 1;
                if ( cl_showTimeDelta->current.enabled )
                    Com_Printf(14, "Extrapolating snapshot!\n");
            }
        }
        if ( LocalClientGlobals->newSnapshots )
            CL_AdjustTimeDelta(localClientNum);
        if ( clc->demoplaying )
        {
            if ( clc->isTimeDemo )
                CL_UpdateTimeDemo(localClientNum);
            else
                CL_UpdateDemoPlaybackSpeed(localClientNum);
            while ( LocalClientGlobals->serverTime >= LocalClientGlobals->snap.serverTime )
            {
                CL_ReadDemoMessage(localClientNum);
                if ( CL_GetLocalClientConnectionState(localClientNum) != 10 )
                    return;
            }
        }
        do
        {
            if ( !Demo_IsPlaying() )
                break;
            if ( !demoReadResult )
                break;
            if ( Demo_IsCompleted() )
                break;
            if ( LocalClientGlobals->serverTime < LocalClientGlobals->snap.serverTime )
                break;
            demoReadResult = Demo_ReadDemoMessage(localClientNum);
        }
        while ( CL_GetLocalClientConnectionState(localClientNum) == 10 );
    }
}

void __cdecl CL_AdjustTimeDelta(int localClientNum)
{
    clientActive_t *LocalClientGlobals; // [esp+4h] [ebp-14h]
    int idealDelta; // [esp+8h] [ebp-10h]
    unsigned int snapInterval; // [esp+Ch] [ebp-Ch]
    int deltaCorrectionMagnitude; // [esp+10h] [ebp-8h]
    int oldDelta; // [esp+14h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals->newSnapshots = 0;
    if ( !CL_GetLocalClientConnection(localClientNum)->demoplaying && !Demo_IsPlaying() )
    {
        snapInterval = LocalClientGlobals->snap.serverTime - LocalClientGlobals->oldSnapServerTime;
        oldDelta = LocalClientGlobals->serverTimeDelta;
        idealDelta = LocalClientGlobals->snap.serverTime - cls.realtime - snapInterval - 5;
        deltaCorrectionMagnitude = idealDelta - oldDelta;
        if ( idealDelta - oldDelta <= 0 )
        {
            deltaCorrectionMagnitude = oldDelta - idealDelta;
        }
        else if ( snapInterval <= 0x3E8 )
        {
            deltaCorrectionMagnitude -= snapInterval;
            if ( deltaCorrectionMagnitude < 0 )
                deltaCorrectionMagnitude = 0;
        }
        if ( deltaCorrectionMagnitude <= 1000 )
        {
            if ( deltaCorrectionMagnitude <= 100 )
            {
                if ( com_timescaleValue == 1.0 )
                {
                    if ( LocalClientGlobals->extrapolatedSnapshot )
                    {
                        LocalClientGlobals->extrapolatedSnapshot = 0;
                        LocalClientGlobals->serverTimeDelta -= 2;
                    }
                    else if ( idealDelta <= LocalClientGlobals->serverTimeDelta )
                    {
                        if ( idealDelta < LocalClientGlobals->serverTimeDelta )
                            --LocalClientGlobals->serverTimeDelta;
                    }
                    else
                    {
                        ++LocalClientGlobals->serverTimeDelta;
                    }
                }
            }
            else
            {
                if ( cl_showTimeDelta->current.enabled )
                    Com_Printf(14, "<FAST> ");
                LocalClientGlobals->serverTimeDelta = (idealDelta + LocalClientGlobals->serverTimeDelta) >> 1;
            }
        }
        else
        {
            Com_PrintWarning(
                14,
                "Cl_AdjustTimeDelta RESET: snap is %i, last snap was %i, walltime is %i, current delta is %i, old server time was"
                " %i, server time is %i\n",
                LocalClientGlobals->snap.serverTime,
                LocalClientGlobals->oldSnapServerTime,
                cls.realtime,
                LocalClientGlobals->serverTimeDelta,
                LocalClientGlobals->oldServerTime,
                LocalClientGlobals->serverTime);
            LocalClientGlobals->serverTimeDelta = LocalClientGlobals->snap.serverTime - cls.realtime - 5;
            LocalClientGlobals->oldServerTime = LocalClientGlobals->snap.serverTime;
            LocalClientGlobals->serverTime = LocalClientGlobals->snap.serverTime;
            if ( cl_showTimeDelta->current.enabled )
                Com_Printf(14, "<RESET> ");
        }
        if ( LocalClientGlobals->serverTimeDelta != oldDelta )
            Con_TimeNudged(localClientNum, LocalClientGlobals->serverTimeDelta - oldDelta);
        if ( cl_showTimeDelta->current.enabled )
        {
            Com_Printf(
                14,
                "client time: %i, server time: %i\n",
                LocalClientGlobals->serverTimeDelta + cls.realtime,
                LocalClientGlobals->snap.serverTime);
            Com_Printf(14, "ideal delta: %i, current delta: %i\n", idealDelta, LocalClientGlobals->serverTimeDelta);
        }
    }
}

void __cdecl CL_UpdateTimeDemo(int localClientNum)
{
    char *v1; // eax
    char *v2; // eax
    int Int; // [esp-4h] [ebp-10h]
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-Ch]
    clientConnection_t *clc; // [esp+4h] [ebp-8h]
    unsigned int currentTime; // [esp+8h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    clc = CL_GetLocalClientConnection(localClientNum);
    currentTime = Sys_Milliseconds();
    if ( !clc->timeDemoLog )
    {
        Int = Dvar_GetInt("r_mode");
        v1 = CL_TimeDemoLogBaseName(LocalClientGlobals->mapname);
        v2 = va("demos/timedemo_%s_mode_%i.csv", v1, Int);
        clc->timeDemoLog = FS_FOpenFileWrite(v2);
    }
    if ( clc->timeDemoStart )
    {
        if ( clc->timeDemoLog )
            FS_Printf(clc->timeDemoLog, (char*)"%i,%i\n", clc->timeDemoFrames, currentTime - clc->timeDemoPrev);
    }
    else
    {
        clc->timeDemoStart = currentTime;
    }
    clc->timeDemoPrev = currentTime;
    ++clc->timeDemoFrames;
    LocalClientGlobals->serverTime = clc->timeDemoBaseTime + 50 * clc->timeDemoFrames;
}

char *__cdecl CL_TimeDemoLogBaseName(const char *mapname)
{
    char *result; // eax
    const char *pos; // [esp+0h] [ebp-10h]
    const char *start; // [esp+4h] [ebp-Ch]
    const char *end; // [esp+8h] [ebp-8h]

    end = 0;
    start = mapname;
    for ( pos = mapname; *pos; ++pos )
    {
        if ( *pos == 47 || *pos == 92 )
        {
            start = pos + 1;
            end = 0;
        }
        else if ( *pos == 46 )
        {
            end = pos;
        }
    }
    if ( !end )
        return (char *)start;
    result = va("%s", start);
    result[end - start] = 0;
    return result;
}

void __cdecl CL_UpdateDemoPlaybackSpeed(int localClientNum)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-Ch]
    clientConnection_t *clc; // [esp+4h] [ebp-8h]
    int demoSpeed; // [esp+8h] [ebp-4h]

    if ( cl_demoLiveStreaming->current.integer )
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
        clc = CL_GetLocalClientConnection(localClientNum);
        if ( cl_demoSkipToTime->current.integer <= LocalClientGlobals->snap.serverTime )
        {
            if ( cl_demoFFSpeed->current.integer )
                demoSpeed = cl_demoFFSpeed->current.integer;
            else
                demoSpeed = 0;
        }
        else
        {
            demoSpeed = 500;
        }
        if ( demoSpeed )
        {
            if ( clc->demoPrevServerTime )
                LocalClientGlobals->serverTime = clc->demoPrevServerTime + 50 * demoSpeed;
            else
                LocalClientGlobals->serverTime = clc->timeDemoBaseTime + 50;
        }
        else
        {
            LocalClientGlobals->serverTime = LocalClientGlobals->snap.serverTime;
        }
        clc->demoPrevServerTime = LocalClientGlobals->serverTime;
    }
}

void __cdecl CL_SetADS(int localClientNum, bool ads)
{
    CL_GetLocalClientGlobals(localClientNum)->usingAds = ads;
}

void __cdecl CL_DrawString(int x, int y, char *pszString, int bShadow, int iCharHeight)
{
    CG_DrawStringExt(&scrPlaceFull, (float)x, (float)y, pszString, 0, 0, bShadow, (float)iCharHeight);
}

float color_allies[4];
float color_axis[4];
void __cdecl CL_LookupColor(unsigned __int8 c, float *color)
{
    unsigned __int8 v2; // al
    float *v3; // [esp+4h] [ebp-8h]

    if ( c == 56 )
    {
        *color = color_allies[0];
        color[1] = color_allies[1];
        color[2] = color_allies[2];
        color[3] = color_allies[3];
    }
    else if ( c == 57 )
    {
        *color = color_axis[0];
        color[1] = color_axis[1];
        color[2] = color_axis[2];
        color[3] = color_axis[3];
    }
    else
    {
        v2 = ColorIndex(c);
        if ( v2 >= 0x11u )
        {
            *color = 1.0f;
            color[1] = 1.0f;
            color[2] = 1.0f;
            color[3] = 1.0f;
        }
        else
        {
            v3 = (float *)g_color_table[v2];
            *color = *v3;
            color[1] = v3[1];
            color[2] = v3[2];
            color[3] = v3[3];
        }
    }
}

void __cdecl CL_UpdateColor()
{
    Dvar_GetUnpackedColor(cg_TeamColor_Allies, color_allies);
    Dvar_GetUnpackedColor(cg_TeamColor_Axis, color_axis);
    color_allies[3] = 1.0f;
    color_axis[3] = 1.0f;
    R_UpdateTeamColors(color_allies, color_axis);
}

bool __cdecl CL_IsServerRestarting(int localClientNum)
{
    return CL_GetLocalClientConnection(localClientNum)->isServerRestarting;
}

