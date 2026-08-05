#include "cl_main_mp.h"
#include <cgame_mp/cg_main_mp.h>
#include <qcommon/common.h>
#include <qcommon/sv_msg_write.h>
#include <ui/ui_main.h>
#include <ui/ui_shared.h>
#include <qcommon/com_clients.h>
#include <win32/win_shared.h>
#include <universal/com_files.h>
#include <client/cl_main.h>
#include <qcommon/files.h>
#include <universal/com_memory.h>
#include <universal/com_constantconfigstrings.h>
#include <qcommon/dvar_cmds.h>
#include <server_mp/sv_main_mp.h>
#include <DynEntity/DynEntity_gamestate.h>
#include <physics/rope_gamestate.h>
#include <glass/glass_client.h>
#include "cl_parse_mp.h"
#include <client/cl_devgui.h>
#include <devgui/devgui.h>
#include "cl_cgame_mp.h"
#include "cl_ui_mp.h"
#include <live/live_fileshare.h>
#include <live/live_fileshare_cache.h>
#include <qcommon/threads.h>
#include <gfx_d3d/r_rendercmds.h>
#include <qcommon/mem_track.h>
#include <live/live_storage_win.h>
#include <client/splitscreen.h>
#include <sound/snd_public_async.h>
#include <server_mp/sv_init_mp.h>
#include <qcommon/dl_main.h>
#include <qcommon/dobj_management.h>
#include <qcommon/com_gamemodes.h>
#include <demo/demo_playback.h>
#include <live/live_leaderboard.h>
#include <live/live_stats.h>
#include <live/live_win.h>
#include <cgame_mp/cg_newDraw_mp.h>
#include <cgame_mp/cg_consolecmds_mp.h>
#include <cgame/cg_sound.h>
#include <live/live_steam.h>
#include <qcommon/legacyhacks.h>
#include "cl_main_pc_mp.h"
#include <DynEntity/DynEntity_client.h>
#include <client/cl_cin.h>
#include "cl_input_mp.h"
#include <DW/dwNet.h>
#include <DW/dwMatchMaking.h>
#include <win32/win_gamerprofile.h>
#include <win32/win_voice.h>
#include <live/live_presence_win.h>
#include <ui/ui_playlists.h>
#include "cl_scrn_mp.h"
#include <DW/dwUtils_pc.h>
#include <win32/win_net.h>
#include <stringed/stringed_hooks.h>
#include <client/cl_voice.h>
#include "cl_net_chan_mp.h"
#include <universal/reliablemsg.h>
#include <client/client.h>
#include <server_mp/sv_main_pc_mp.h>
#include <ui_mp/ui_main_mp.h>
#include <client/cl_console.h>
#include <EffectsCore/fx_system.h>
#include <universal/physicalmemory.h>
#include <server_mp/sv_voice_mp.h>
#include <win32/win_input.h>
#include <cgame/cg_compass.h>
#include <client/cl_keys.h>
#include <ui/ui_screenshot.h>
#include <bgame/bg_fire.h>
#include <gfx_d3d/r_ui3d.h>
#include <game_mp/g_main_mp.h>
#include <gfx_d3d/r_dvars.h>
#include <qcommon/com_bsp_load_obj.h>
#include <qcommon/cm_load.h>
#include <win32/win_splash.h>
#include <win32/win_syscon.h>
#include <win32/win_main.h>
#include <ui/ui_emblem.h>
#include <ui/ui_commands.h>
#include <ui/ui_utils.h>
#include <ui/ui_keyboard.h>
#include <ragdoll/ragdoll.h>
#include <ui_mp/ui_gametype_custom_mp.h>
#include <clientscript/cscr_vm.h>
#include <cgame/cg_main.h>
#include <database/db_file_load.h>
#include <live/live_pcache_profile.h>
#include <client/cl_debugdata.h>
#include <win32/win_steam.h>
#include <universal/base64.h>

const dvar_t *cl_noprint;
const dvar_t *playlist;
const dvar_t *category;
const dvar_t *categoryPlaylist;
const dvar_t *wagerCategory;
const dvar_t *wagerCategoryPlaylist;
const dvar_t *customGameMode;
const dvar_t *customclass[10];
const dvar_t *cl_hudDrawsBehindUI;
const dvar_t *cl_voice;
const dvar_t *cl_timeout;
const dvar_t *cl_connectTimeout;
const dvar_t *cl_connectionAttempts;
const dvar_t *cl_shownet;
const dvar_t *cl_shownuments;
const dvar_t *cl_showServerCommands;
const dvar_t *cl_showSend;
const dvar_t *cl_showTimeDelta;
const dvar_t *cl_freezeDemo;
const dvar_t *cl_activeAction;
const dvar_t *cl_avidemo;
const dvar_t *cl_forceavidemo;
const dvar_t *cl_clientDemoName;
const dvar_t *cl_clientDemoUseMemoryBuffer;
const dvar_t *cl_clientDemoType;
const dvar_t *cl_clientDemoMaxRecordTime;
const dvar_t *cl_clientDemoRequestPingTime;
const dvar_t *cl_clientDemoCoolMomentExtendEnabled;
const dvar_t *cl_clientDemoCoolMomentExtendTime;
const dvar_t *cl_yawspeed;
const dvar_t *cl_pitchspeed;
const dvar_t *cl_anglespeedkey;
const dvar_t *cl_maxpackets;
const dvar_t *cl_packetdup;
const dvar_t *cl_sensitivity;
const dvar_t *cl_mouseAccel;
const dvar_t *cl_freelook;
const dvar_t *cl_showMouseRate;
const dvar_t *cl_allowDownload;
const dvar_t *cl_wwwDownload;
const dvar_t *cl_talking;
const dvar_t *cl_inGameVideo;
const dvar_t *cl_serverStatusResendTime;
const dvar_t *cl_maxppf;
const dvar_t *cl_bypassMouseInput;
const dvar_t *m_pitch;
const dvar_t *m_yaw;
const dvar_t *m_forward;
const dvar_t *m_side;
const dvar_t *m_filter;
const dvar_t *ui_custom_newname;
const dvar_t *cl_socketpool_enabled;
const dvar_t *cl_socketpool_size;
const dvar_t *ui_clantag_new;
const dvar_t *cl_demoFFSpeed;
const dvar_t *cl_demoSkipToTime;
const dvar_t *cl_demoLiveStreaming;
const dvar_t *input_invertPitch;
const dvar_t *input_viewSensitivity;
const dvar_t *team_indicator;
const dvar_t *cl_playerRank;
const dvar_t *cl_playerPrestige;
const dvar_t *cl_motdString;
const dvar_t *cl_ingame;
const dvar_t *cl_wasconnected;
const dvar_t *cl_maxPing;
const dvar_t *cl_profileTextHeight;
const dvar_t *cl_profileTextY;
const dvar_t *name;
const dvar_t *clanName;
const dvar_t *rate;
const dvar_t *snaps;
const dvar_t *cl_punkbuster;
const dvar_t *password;
const dvar_t *nextdemo;
const dvar_t *mortarStrikesLeft;
//const dvar_t *hud_enable;
//const dvar_t *cg_blood;
//const dvar_t *cg_allow_mature;
//const dvar_t *cg_mature;
const dvar_t *cl_serverchallenge;
const dvar_t *cl_debugMessageKey;
const dvar_t *motd;

clientActive_t *clients;
UiContext cgDC[1];

clientStatic_t cls;
clientConnection_t *clientConnections;

bool cl_serverLoadingMap;
bool g_waitingForServer;
bool cl_waitingOnServerToLoadMap[1];

char printBuf[2048];
int cl_maxLocalClients;
unsigned int frame_msec;
int old_com_frameTime;

voiceCommunication_t cl_voiceCommunication[1];

void __cdecl CL_AddReliableCommand(int localClientNum, const char *cmd)
{
    int ackIndex; // [esp+4h] [ebp-Ch]
    clientConnection_t *clc; // [esp+8h] [ebp-8h]
    int index; // [esp+Ch] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    if ( clc->reliableSequence - clc->reliableAcknowledge > 128 )
        Com_Error(ERR_DROP, "EXE_ERR_CLIENT_CMD_OVERFLOW");
    index = ++clc->reliableSequence & 0x7F;
    MSG_WriteReliableCommandToBuffer(
        cmd,
        &clc->reliableCommands[index],
        clc->reliableCommandBuffer,
        0x4000,
        &clc->reliableCommandBufferNext);
    ackIndex = ((unsigned __int8)clc->reliableAcknowledge + 1) & 0x7F;
    if ( ackIndex != index
        && clc->reliableCommands[index] <= clc->reliableCommands[ackIndex]
        && clc->reliableCommands[ackIndex] < &clc->reliableCommandBuffer[clc->reliableCommandBufferNext] )
    {
        Com_Error(ERR_DROP, "EXE_ERR_CLIENT_CMD_BUFFER_OVERFLOW");
    }
}

void __cdecl CL_AnimateUI_f()
{
    const char *v0; // eax
    const char *v1; // eax
    uiInfo_s *v2; // eax
    uiInfo_s *Info; // eax
    const char *menuName; // [esp+28h] [ebp-14h]
    const char *itemName; // [esp+2Ch] [ebp-10h]
    int milliseconds; // [esp+30h] [ebp-Ch]
    const char *stateName; // [esp+38h] [ebp-4h]

    menuName = Cmd_Argv(1);
    itemName = Cmd_Argv(2);
    stateName = Cmd_Argv(3);
    v0 = Cmd_Argv(4);
    milliseconds = atoi(v0);
    v1 = Cmd_Argv(5);
    if ( atoi(v1) <= 0 )
    {
        Info = UI_GetInfo(0);
        UI_AnimateItem(0, &Info->uiDC, menuName, itemName, stateName, milliseconds);
        UI_AnimateItem(0, cgDC, menuName, itemName, stateName, milliseconds);
    }
    else
    {
        v2 = UI_GetInfo(0);
        UI_AnimateItemSysTime(0, &v2->uiDC, menuName, itemName, stateName, milliseconds);
        UI_AnimateItemSysTime(0, cgDC, menuName, itemName, stateName, milliseconds);
    }
}

void __cdecl CL_ShowContextualItemUI_f()
{
    const char *v0; // eax
    const char *v1; // eax
    uiInfo_s *Info; // eax
    const char *menuName; // [esp+28h] [ebp-10h]
    float offsetX; // [esp+2Ch] [ebp-Ch]
    const char *itemName; // [esp+30h] [ebp-8h]
    float offsetY; // [esp+34h] [ebp-4h]

    menuName = Cmd_Argv(1);
    itemName = Cmd_Argv(2);
    v0 = Cmd_Argv(3);
    offsetX = atof(v0);
    v1 = Cmd_Argv(4);
    offsetY = atof(v1);
    Info = UI_GetInfo(0);
    UI_UpdateContextItem(&Info->uiDC, menuName, itemName, offsetX, offsetY);
}

void __cdecl CL_ToggleDemoRecording_f()
{
    int ControllerIndex; // eax
    const char *String; // eax
    const char *v2; // eax
    const char *v3; // eax
    int v4; // eax
    unsigned int v5; // [esp-4h] [ebp-10h]
    unsigned int v6; // [esp-4h] [ebp-10h]
    char *v7; // [esp-4h] [ebp-10h]
    clientConnection_t *clc; // [esp+8h] [ebp-4h]

    clc = CL_GetLocalClientConnection(0);
    if ( clc->demoplaying )
    {
        Com_Printf(14, "You cant record when you are playing the demo\n");
    }
    else if ( clc->demorecording )
    {
        if ( !cl_clientDemoType->current.integer )
        {
            ControllerIndex = Com_LocalClient_GetControllerIndex(0);
            Cmd_ExecuteSingleCommand(0, ControllerIndex, (char*)"stoprecord");
        }
    }
    else if ( CL_GetLocalClientConnectionState(0) == 10 )
    {
        v5 = Sys_Milliseconds();
        String = Dvar_GetString("mapname");
        v2 = va("demo_%s%d", String, v5);
        Dvar_SetString((dvar_s *)cl_clientDemoName, v2);
        v6 = Sys_Milliseconds();
        v3 = Dvar_GetString("mapname");
        v7 = va("record demo_%s%d\n", v3, v6);
        v4 = Com_LocalClient_GetControllerIndex(0);
        Cmd_ExecuteSingleCommand(0, v4, v7);
    }
    else
    {
        Com_Printf(14, "You must be in a level to record.\n");
    }
}

void __cdecl CL_WriteDemoEpilog(int localClientNum)
{
    int len; // [esp+10h] [ebp-5Ch] BYREF
    char name[68]; // [esp+14h] [ebp-58h] BYREF
    clientConnection_t *clc; // [esp+5Ch] [ebp-10h]
    int fileHandle; // [esp+60h] [ebp-Ch]
    unsigned __int8 type; // [esp+67h] [ebp-5h] BYREF
    char *buff; // [esp+68h] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    type = 0;
    FS_WriteToDemo((char *)&type, 1u, clc->demofile);
    len = -1;
    FS_WriteToDemo((char *)&len, 4u, clc->demofile);
    FS_WriteToDemo((char *)&len, 4u, clc->demofile);
    FS_FCloseFile(clc->demofile);
    clc->demofile = 0;
    Com_Printf(0, "Finished writing demo file.\n");
    Com_sprintf(name, 0x40u, "demos/demos.dm_%d", 7);
    fileHandle = FS_FOpenFileAppend(name);
    buff = va("%s\n", clc->demoName);
    FS_Write(buff, strlen(buff), fileHandle);
    FS_FCloseFile(fileHandle);
}

void __cdecl CL_WriteDemoClientArchive(int demoFile, unsigned __int8 msgType, ClientArchiveData *archive, int index)
{
    FS_WriteToDemo((char *)&msgType, 1u, demoFile);
    FS_WriteToDemo((char *)&index, 4u, demoFile);
    FS_WriteToDemo((char *)archive->origin, 0xCu, demoFile);
    FS_WriteToDemo((char *)archive->velocity, 0xCu, demoFile);
    FS_WriteToDemo((char *)&archive->movementDir, 4u, demoFile);
    FS_WriteToDemo((char *)&archive->bobCycle, 4u, demoFile);
    FS_WriteToDemo((char *)archive, 4u, demoFile);
    FS_WriteToDemo((char *)archive->viewangles, 0xCu, demoFile);
}

void __cdecl CL_WriteDemoFromContinuousStateBuffer(int localClientNum, int continuousStateBufferIndex)
{
    ClientArchiveData dst; // [esp+8h] [ebp-1080h] BYREF
    unsigned __int8 data[4096]; // [esp+38h] [ebp-1050h] BYREF
    int v4; // [esp+103Ch] [ebp-4Ch]
    msg_t buf; // [esp+1040h] [ebp-48h] BYREF
    int v6; // [esp+1070h] [ebp-18h] BYREF
    clientConnection_t *LocalClientConnection; // [esp+1074h] [ebp-14h]
    int v8; // [esp+1078h] [ebp-10h]
    char buffer; // [esp+107Fh] [ebp-9h] BYREF
    int index; // [esp+1080h] [ebp-8h]
    unsigned int count; // [esp+1084h] [ebp-4h]

    v8 = 0;
    LocalClientConnection = CL_GetLocalClientConnection(localClientNum);
    v4 = (int)&LocalClientConnection->demoContinuousStateBuffer[continuousStateBufferIndex];
    MSG_Init(&buf, data, 4096);
    count = 48;
    index = *(unsigned int *)(v4 + 16);
    while ( index != *(unsigned int *)(v4 + 20) )
    {
        //BLOPS_NULLSUB();
        memcpy((unsigned __int8 *)&dst, (unsigned __int8 *)(*(unsigned int *)(v4 + 12) + count * v8), count);
        CL_WriteDemoClientArchive(LocalClientConnection->demofile, 1u, &dst, index);
        index = (index + 1) % 256;
        ++v8;
    }
    buffer = 0;
    FS_WriteToDemo(&buffer, 1u, LocalClientConnection->demofile);
    v6 = *(unsigned int *)v4;
    FS_WriteToDemo((char *)&v6, 4u, LocalClientConnection->demofile);
    v6 = *(unsigned int *)(v4 + 8);
    FS_WriteToDemo((char *)&v6, 4u, LocalClientConnection->demofile);
    FS_WriteToDemo(*(char **)(v4 + 4), *(unsigned int *)(v4 + 8), LocalClientConnection->demofile);
}

void __cdecl CL_StopRecord_f()
{
    clientConnection_t *clc; // [esp+4h] [ebp-4h]

    clc = CL_GetLocalClientConnection(0);
    if ( clc->demorecording )
    {
        CL_WriteDemoEpilog(0);
        if ( clc->demoLiveStream )
        {
            FS_FCloseFile(clc->demoLiveStream);
            clc->demoLiveStream = 0;
        }
        clc->demorecording = 0;
        Com_Printf(0, "Stopped demo.\n");
    }
    else
    {
        Com_Printf(0, "Not recording a demo.\n");
    }
}

void __cdecl CL_Record_f()
{
    int v0; // eax
    int v1; // eax
    char ospath[260]; // [esp+18h] [ebp-368h] BYREF
    const char *baspath; // [esp+11Ch] [ebp-264h]
    int number; // [esp+120h] [ebp-260h]
    connstate_t connstate; // [esp+124h] [ebp-25Ch]
    char demoName[68]; // [esp+128h] [ebp-258h] BYREF
    int localClientNum; // [esp+16Ch] [ebp-214h]
    char name[256]; // [esp+170h] [ebp-210h] BYREF
    clientConnection_t *clc; // [esp+270h] [ebp-110h]
    const char *s; // [esp+274h] [ebp-10Ch]
    char nameLiveStream[260]; // [esp+278h] [ebp-108h] BYREF

    if ( Cmd_Argc() <= 2 )
    {
        if ( cl_clientDemoUseMemoryBuffer->current.enabled || cl_clientDemoType->current.integer == 2 )
        {
            Com_Printf(
                0,
                "You cannot run this command when cl_clientDemoUseMemoryBuffer is true or cl_clientDemoType is CLIENT_DEMO_CENTER_PIVOT \n");
        }
        else
        {
            localClientNum = 0;
            connstate = CL_GetLocalClientConnectionState(0);
            if ( connstate == CA_ACTIVE )
            {
                clc = CL_GetLocalClientConnection(localClientNum);
                if ( clc->demorecording )
                {
                    Com_Printf(0, "Already recording.\n");
                }
                else
                {
                    if ( Cmd_Argc() == 2 )
                    {
                        s = Cmd_Argv(1);
                        I_strncpyz(demoName, s, 64);
                        Com_sprintf(name, 0x100u, "%s.dm_%d", demoName, 7);
                        Com_sprintf(nameLiveStream, 0x100u, "%s.txt", demoName);
                    }
                    else
                    {
                        for ( number = 0; number <= 9999; ++number )
                        {
                            Com_sprintf(demoName, 0x40u, "demo%04i", number);
                            Com_sprintf(name, 0x100u, "%s.dm_%d", demoName, 7);
                            Com_sprintf(nameLiveStream, 0x100u, "%s.txt", demoName);
                            baspath = fs_homepath->current.string;
                            FS_BuildOSPath((char *)baspath, (char*)"demos", name, ospath);
                            if ( !FS_OSFPathExists(ospath) )
                                break;
                        }
                    }
                    Com_Printf(0, "recording to %s.\n", name);
                    v0 = FS_SV_FOpenFileWrite(name, (char*)"demos");
                    clc->demofile = v0;
                    if ( clc->demofile )
                    {
                        if ( !cl_demoLiveStreaming->current.integer
                            || (v1 = FS_SV_FOpenFileWrite(nameLiveStream, (char *)"demos"), (clc->demoLiveStream = v1) != 0) )
                        {
                            clc->demorecording = 1;
                            clc->demoRecordStartTime = Sys_Milliseconds();
                            clc->demowaiting = 1;
                            clc->demoUseMemoryBuffer = 0;
                            I_strncpyz(clc->demoName, demoName, 64);
                            CL_WriteUncompressedDemoInfo(localClientNum);
                        }
                        else
                        {
                            Com_PrintError(0, "ERROR: couldn't open to write %s.\n", nameLiveStream);
                        }
                    }
                    else
                    {
                        Com_PrintError(0, "ERROR: couldn't open.\n");
                    }
                }
            }
            else
            {
                Com_Printf(0, "You must be in a level to record.\n");
            }
        }
    }
    else
    {
        Com_Printf(0, "record <demoname>\n");
    }
}

void __cdecl CL_WriteInitialStateToDemo(
                int demoFile,
                unsigned __int8 type,
                int serverMessageSequence,
                unsigned __int8 *compressedBuf,
                int compressedBufSize)
{
    int len; // [esp+8h] [ebp-4h] BYREF

    FS_WriteToDemo((char *)&type, 1u, demoFile);
    len = serverMessageSequence;
    FS_WriteToDemo((char *)&len, 4u, demoFile);
    len = compressedBufSize;
    FS_WriteToDemo((char *)&len, 4u, demoFile);
    for ( len = 0; len < compressedBufSize; len += 1024 )
    {
        if ( compressedBufSize - len > 1024 )
            FS_WriteToDemo((char *)&compressedBuf[len], 0x400u, demoFile);
        else
            FS_WriteToDemo((char *)&compressedBuf[len], compressedBufSize - len, demoFile);
    }
}

void __cdecl CL_WriteUncompressedDemoInfo(int localClientNum)
{
    const char *String; // eax
    const char *v2; // eax
    int Checksum; // eax
    const char *v4; // eax
    const char *v5; // eax
    char *v6; // eax
    int UsedBitCount; // eax
    char *v8; // eax
    int v9; // eax
    clientActive_t *LocalClientGlobals; // [esp+28h] [ebp-184h]
    LargeLocal compressedBuf_large_local(0x10000); // [esp+2Ch] [ebp-180h] BYREF
    int configStringCount; // [esp+34h] [ebp-178h]
    int compressedSize; // [esp+38h] [ebp-174h]
    int numConfigStringsWritten; // [esp+3Ch] [ebp-170h]
    unsigned __int8 (*bufData)[65536]; // [esp+40h] [ebp-16Ch]
    int bitsUsed; // [esp+44h] [ebp-168h]
    bool writeConfigString; // [esp+4Bh] [ebp-161h]
    entityState_s nullstate; // [esp+4Ch] [ebp-160h] BYREF
    unsigned __int8 (*compressedBuf)[65536]; // [esp+130h] [ebp-7Ch]
    msg_t buf; // [esp+134h] [ebp-78h] BYREF
    SnapshotInfo_s snapInfo; // [esp+164h] [ebp-48h] BYREF
    int nextConstConfigStringIndex; // [esp+17Ch] [ebp-30h]
    clientConnection_t *clc; // [esp+180h] [ebp-2Ch]
    LargeLocal bufData_large_local(0x10000); // [esp+184h] [ebp-28h] BYREF
    int lastStringIndex; // [esp+18Ch] [ebp-20h]
    int oldSvsHeaderValid; // [esp+190h] [ebp-1Ch]
    const char *constValue; // [esp+194h] [ebp-18h]
    const entityState_s *ent; // [esp+198h] [ebp-14h]
    const char *s; // [esp+19Ch] [ebp-10h]
    int nextConstConfigStringNumber; // [esp+1A0h] [ebp-Ch]
    int i; // [esp+1A4h] [ebp-8h]
    int bitsStart; // [esp+1A8h] [ebp-4h]

    //LargeLocal::LargeLocal(&compressedBuf_large_local, 0x10000);
    compressedBuf = (unsigned __int8 (*)[65536])compressedBuf_large_local.GetBuf();
    //LargeLocal::LargeLocal(&bufData_large_local, 0x10000);
    bufData = (unsigned __int8 (*)[65536])bufData_large_local.GetBuf();
    oldSvsHeaderValid = 0;
    clc = CL_GetLocalClientConnection(localClientNum);
    if ( clc->demoUseMemoryBuffer )
        clc->demoInitialStateBufferIndex = (clc->demoInitialStateBufferIndex + 1) % 3;
    MSG_Init(&buf, (unsigned __int8 *)bufData, 0x10000);
    MSG_WriteLong(&buf, clc->reliableSequence);
    MSG_WriteByte(&buf, 1u);
    MSG_WriteLong(&buf, clc->serverCommandSequence);
    String = Dvar_GetString("mapname");
    MSG_WriteString(&buf, String);
    v2 = Dvar_GetString("g_gametype");
    MSG_WriteString(&buf, v2);
    Checksum = CCS_GetChecksum();
    MSG_WriteLong(&buf, Checksum);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    MSG_WriteByte(&buf, 2u);
    configStringCount = 0;
    nextConstConfigStringIndex = 0;
    nextConstConfigStringNumber = CCS_GetConfigStringNum(0);
    if ( !nextConstConfigStringNumber )
    {
        v4 = va(
                     "ERROR: Could not load the configstring table for the current map and gametype. Make sure that the configstrin"
                     "g table is built for this map and game type.");
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp", 1603, 0, v4) )
            __debugbreak();
    }
    for ( i = 0; i < MAX_CONFIGSTRINGS; ++i )
    {
        writeConfigString = 1;
        if ( i == 1 )
        {
            ++configStringCount;
        }
        else if ( i )
        {
            s = &cls.gameState.stringData[cls.gameState.stringOffsets[i]];
            if ( nextConstConfigStringNumber == i )
            {
                constValue = CCS_GetConfigStringValue(nextConstConfigStringIndex);
                if ( i >= 1547 && I_stricmp(constValue, s) )
                {
                    ++configStringCount;
                }
                else if ( i < 1547 )
                {
                    if ( strcmp(constValue, s) )
                        ++configStringCount;
                }
                nextConstConfigStringNumber = CCS_GetConfigStringNum(++nextConstConfigStringIndex);
            }
            else if ( cls.gameState.stringOffsets[i] )
            {
                ++configStringCount;
            }
        }
        else
        {
            ++configStringCount;
        }
    }
    bitsStart = MSG_GetUsedBitCount(&buf);
    MSG_WriteShort(&buf, configStringCount);
    nextConstConfigStringIndex = 0;
    nextConstConfigStringNumber = CCS_GetConfigStringNum(0);
    numConfigStringsWritten = 0;
    lastStringIndex = -1;
    for ( i = 0; i < MAX_CONFIGSTRINGS; ++i )
    {
        if ( i == 1 )
        {
            s = Dvar_InfoString_Big(8);
            MSG_WriteBit1(&buf);
            MSG_WriteBigString(&buf, (char *)s);
            lastStringIndex = i;
            ++numConfigStringsWritten;
        }
        else
        {
            s = &cls.gameState.stringData[cls.gameState.stringOffsets[i]];
            if ( i )
            {
                if ( nextConstConfigStringNumber )
                {
                    if ( nextConstConfigStringNumber < i )
                    {
                        v5 = va(
                                     "Invalid line in the config string csv - config string indexes must only go up, but it went down and i"
                                     "s now %i (line %i) - check the bottom of the file to see if it's corrupt",
                                     nextConstConfigStringNumber,
                                     nextConstConfigStringIndex);
                        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp", 1688, 0, v5) )
                            __debugbreak();
                    }
                }
                if ( nextConstConfigStringNumber == i )
                {
                    writeConfigString = 0;
                    constValue = CCS_GetConfigStringValue(nextConstConfigStringIndex);
                    if ( i >= 1547 && I_stricmp(constValue, s) )
                    {
                        writeConfigString = 1;
                    }
                    else if ( i >= 1547 || !strcmp(constValue, s) )
                    {
                        if ( !cls.gameState.stringOffsets[i] )
                        {
                            v6 = va("PROCESSING: (<INSENSITIVE) nextConstConfigStringNumber == i -- ConfigString: %d\n", i);
                            Com_Printf(14, v6);
                            writeConfigString = 1;
                        }
                    }
                    else
                    {
                        writeConfigString = 1;
                    }
                    if ( writeConfigString )
                    {
                        if ( cls.gameState.stringOffsets[i] )
                        {
                            if ( i == lastStringIndex + 1 )
                            {
                                MSG_WriteBit1(&buf);
                            }
                            else
                            {
                                MSG_WriteBit0(&buf);
                                MSG_WriteBits(&buf, i, 0xCu);
                            }
                            MSG_WriteBigString(&buf, (char *)s);
                            lastStringIndex = i;
                            ++numConfigStringsWritten;
                        }
                        else
                        {
                            MSG_WriteBit0(&buf);
                            MSG_WriteBits(&buf, i, 0xCu);
                            MSG_WriteBigString(&buf, (char *)"");
                            lastStringIndex = i;
                            ++numConfigStringsWritten;
                        }
                    }
                    nextConstConfigStringNumber = CCS_GetConfigStringNum(++nextConstConfigStringIndex);
                }
                else if ( cls.gameState.stringOffsets[i] )
                {
                    if ( i == lastStringIndex + 1 )
                    {
                        MSG_WriteBit1(&buf);
                    }
                    else
                    {
                        MSG_WriteBit0(&buf);
                        MSG_WriteBits(&buf, i, 0xCu);
                    }
                    MSG_WriteBigString(&buf, (char *)s);
                    ++numConfigStringsWritten;
                }
            }
            else
            {
                MSG_WriteBit1(&buf);
                MSG_WriteBigString(&buf, (char *)s);
                lastStringIndex = i;
                ++numConfigStringsWritten;
            }
        }
    }
    if ( numConfigStringsWritten != configStringCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                    1762,
                    0,
                    "%s",
                    "numConfigStringsWritten == configStringCount") )
    {
        __debugbreak();
    }
    UsedBitCount = MSG_GetUsedBitCount(&buf);
    bitsUsed = UsedBitCount - bitsStart;
    v8 = va(
                 "Demo_WriteConfigStrings() - NumConfigStrings: %d Size: %d bytes\n",
                 configStringCount,
                 (UsedBitCount - bitsStart) / 8);
    Com_Printf(14, v8);
    oldSvsHeaderValid = svsHeaderValid;
    svsHeaderValid = 1;
    svsHeader.mapCenter[0] = cls.mapCenter[0];
    svsHeader.mapCenter[1] = cls.mapCenter[1];
    svsHeader.mapCenter[2] = cls.mapCenter[2];
    memset(&snapInfo, 0, sizeof(snapInfo));
    memset(&nullstate, 0, sizeof(nullstate));
    for ( i = 0; i < 1024; ++i )
    {
        ent = &LocalClientGlobals->entityBaselines[i];
        if ( ent->number )
        {
            MSG_WriteByte(&buf, 4u);
            MSG_WriteEntity(&snapInfo, &buf, -90000, &nullstate, ent, DELTA_FLAGS_FORCE);
        }
    }
    svsHeaderValid = oldSvsHeaderValid;
    MSG_WriteByte(&buf, 3u);
    MSG_WriteLong(&buf, cls.gameState.matchUIVisibilityFlags);
    MSG_WriteByte(&buf, 0xEu);
    MSG_WriteLong(&buf, clc->clientNum);
    MSG_WriteLong(&buf, clc->checksumFeed);
    MSG_WriteByte(&buf, 6u);
    DynEnt_WriteGameState(&buf);
    MSG_WriteByte(&buf, 7u);
    MSG_WriteShort(&buf, 1023);
    MSG_WriteByte(&buf, 8u);
    Rope_WriteGameState(&buf);
    MSG_WriteByte(&buf, 9u);
    GlassCl_WriteDemoSnapshot(&buf);
    MSG_WriteByte(&buf, 0xEu);
    if ( buf.cursize < 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                    1818,
                    0,
                    "%s",
                    "buf.cursize >= CL_DECODE_START") )
    {
        __debugbreak();
    }
    if ( clc->demoUseMemoryBuffer )
    {
        *(unsigned int *)clc->demoInitialStateBuffer[clc->demoInitialStateBufferIndex].compressedBuf = *(unsigned int *)buf.data;
        v9 = MSG_WriteBitsCompress(
                     0,
                     buf.data + 4,
                     buf.cursize - 4,
                     &clc->demoInitialStateBuffer[clc->demoInitialStateBufferIndex].compressedBuf[4],
                     65532);
        clc->demoInitialStateBuffer[clc->demoInitialStateBufferIndex].compressedBufSize = v9 + 4;
        clc->demoInitialStateBuffer[clc->demoInitialStateBufferIndex].serverMessageSequence = clc->serverMessageSequence;
        CL_WriteAllDemoClientArchiveToMemory(localClientNum);
        clc->lastClientArchiveIndex = LocalClientGlobals->clientArchiveIndex;
        clc->demoInitialStateBuffer[clc->demoInitialStateBufferIndex].continuousBufferStartingIndex = (clc->demoContinuousStateBufferIndex
                                                                                                                                                                                                 + 1)
                                                                                                                                                                                                % 200;
    }
    else
    {
        *(unsigned int *)compressedBuf = *(unsigned int *)buf.data;
        compressedSize = MSG_WriteBitsCompress(0, buf.data + 4, buf.cursize - 4, &(*compressedBuf)[4], 65532) + 4;
        CL_WriteInitialStateToDemo(
            clc->demofile,
            0,
            clc->serverMessageSequence,
            (unsigned __int8 *)compressedBuf,
            compressedSize);
        CL_WriteAllDemoClientArchive(localClientNum);
        clc->lastClientArchiveIndex = LocalClientGlobals->clientArchiveIndex;
    }
    //LargeLocal::~LargeLocal(&bufData_large_local);
    //LargeLocal::~LargeLocal(&compressedBuf_large_local);
}

void __cdecl CL_WriteAllDemoClientArchive(int localClientNum)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-1044h]
    unsigned __int8 data[4100]; // [esp+4h] [ebp-1040h] BYREF
    msg_t buf; // [esp+100Ch] [ebp-38h] BYREF
    clientConnection_t *LocalClientConnection; // [esp+103Ch] [ebp-8h]
    int index; // [esp+1040h] [ebp-4h]

    LocalClientConnection = CL_GetLocalClientConnection(localClientNum);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    MSG_Init(&buf, data, 4096);
    for ( index = 0; index < 256; ++index )
    {
        //BLOPS_NULLSUB();
        CL_WriteDemoClientArchive(LocalClientConnection->demofile, 1u, &LocalClientGlobals->clientArchive[index], index);
    }
}

void __cdecl CL_WriteAllDemoClientArchiveToMemory(int localClientNum)
{
    int i; // [esp+0h] [ebp-Ch]
    clientActive_t *LocalClientGlobals; // [esp+4h] [ebp-8h]
    clientConnection_t *clc; // [esp+8h] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    for ( i = 0; i < 256; ++i )
        CL_WriteDemoClientArchiveToMemory(
            LocalClientGlobals,
            &clc->demoInitialStateBuffer[clc->demoInitialStateBufferIndex].archive[i],
            i);
    clc->demoInitialStateBuffer[clc->demoInitialStateBufferIndex].lastClientArchiveIndex = LocalClientGlobals->clientArchiveIndex;
}

void __cdecl CL_WriteDemoClientArchiveToMemory(const clientActive_t *cl, ClientArchiveData *archive, int index)
{
    archive->origin[0] = cl->clientArchive[index].origin[0];
    archive->origin[1] = cl->clientArchive[index].origin[1];
    archive->origin[2] = cl->clientArchive[index].origin[2];
    archive->velocity[0] = cl->clientArchive[index].velocity[0];
    archive->velocity[1] = cl->clientArchive[index].velocity[1];
    archive->velocity[2] = cl->clientArchive[index].velocity[2];
    archive->movementDir = cl->clientArchive[index].movementDir;
    archive->bobCycle = cl->clientArchive[index].bobCycle;
    archive->serverTime = cl->clientArchive[index].serverTime;
    archive->viewangles[0] = cl->clientArchive[index].viewangles[0];
    archive->viewangles[1] = cl->clientArchive[index].viewangles[1];
    archive->viewangles[2] = cl->clientArchive[index].viewangles[2];
}

void __cdecl CL_ReadDemoMessage(int localClientNum)
{
    clientConnection_t *clc; // [esp+8h] [ebp-8h]
    unsigned __int8 s; // [esp+Fh] [ebp-1h] BYREF

    clc = CL_GetLocalClientConnection(localClientNum);
    if ( clc->demofile )
    {
        if ( FS_Read(&s, 1u, clc->demofile) == 1 )
        {
            if ( s )
            {
                if ( s == 1 )
                    CL_ReadDemoClientArchive(localClientNum);
            }
            else
            {
                CL_ReadDemoNetworkPacket(localClientNum);
            }
        }
        else
        {
            CL_DemoCompleted(localClientNum);
        }
    }
    else
    {
        CL_DemoCompleted(localClientNum);
    }
}

void __cdecl CL_DemoCompleted(int localClientNum)
{
    int time; // [esp+10h] [ebp-8h]
    clientConnection_t *clc; // [esp+14h] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    if ( clc->isTimeDemo )
    {
        time = Sys_Milliseconds() - clc->timeDemoStart;
        if ( time > 0 )
            Com_Printf(
                14,
                "%i frames, %3.1f seconds: %3.1f fps\n",
                clc->timeDemoFrames,
                (double)time / 1000.0,
                (double)clc->timeDemoFrames * 1000.0 / (double)time);
    }
    if ( clc->timeDemoLog )
    {
        FS_FCloseFile(clc->timeDemoLog);
        clc->timeDemoLog = 0;
    }
    clc->demoPrevServerTime = 0;
    Dvar_SetIntByName("cl_demoFFSpeed", 0);
    Dvar_SetIntByName("cl_demoSkipToTime", 0);
    CL_Disconnect(0, 1);
    CL_NextDemo(0);
}

void __cdecl CL_ReadDemoClientArchive(int localClientNum)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-14h]
    unsigned __int8 *archive; // [esp+8h] [ebp-Ch]
    clientConnection_t *clc; // [esp+Ch] [ebp-8h]
    int index; // [esp+10h] [ebp-4h] BYREF

    clc = CL_GetLocalClientConnection(localClientNum);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if ( FS_Read((unsigned __int8 *)&index, 4u, clc->demofile) == 4 )
    {
        if ( (unsigned int)index < 0x100 )
        {
            archive = (unsigned __int8 *)&LocalClientGlobals->clientArchive[index];
            FS_Read((unsigned __int8 *)LocalClientGlobals->clientArchive[index].origin, 0xCu, clc->demofile);
            FS_Read(archive + 16, 0xCu, clc->demofile);
            FS_Read(archive + 32, 4u, clc->demofile);
            FS_Read(archive + 28, 4u, clc->demofile);
            FS_Read(archive, 4u, clc->demofile);
            FS_Read(archive + 36, 0xCu, clc->demofile);
            LocalClientGlobals->clientArchiveIndex = index + 1;
        }
        else
        {
            Com_Printf(14, "Demo file was corrupt.\n");
            CL_DemoCompleted(localClientNum);
        }
    }
    else
    {
        CL_DemoCompleted(localClientNum);
    }
}

void __cdecl CL_ReadDemoNetworkPacket(int localClientNum)
{
    unsigned int v1; // ecx
    int v2; // eax
    unsigned __int8 *bufData; // [esp+4h] [ebp-44h]
    msg_t buf; // [esp+8h] [ebp-40h] BYREF
    clientConnection_t *clc; // [esp+38h] [ebp-10h]
    LargeLocal bufData_large_local(0x10000); // [esp+3Ch] [ebp-Ch] BYREF
    int s; // [esp+44h] [ebp-4h] BYREF

    //LargeLocal::LargeLocal(&bufData_large_local, 0x10000);
    bufData = bufData_large_local.GetBuf(); // LargeLocal::GetBuf(&bufData_large_local);
    clc = CL_GetLocalClientConnection(localClientNum);
    if ( FS_Read((unsigned __int8 *)&s, 4u, clc->demofile) == 4 )
    {
        clc->serverMessageSequence = s;
        MSG_Init(&buf, bufData, 0x10000);
        if ( FS_Read((unsigned __int8 *)&buf.cursize, 4u, clc->demofile) != 4 )
            goto LABEL_4;
        if ( buf.cursize < -1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                        1994,
                        0,
                        "%s",
                        "buf.cursize >= -1") )
        {
            __debugbreak();
        }
        if ( buf.cursize == -1 )
        {
LABEL_4:
            CL_DemoCompleted(localClientNum);
            //LargeLocal::~LargeLocal(&bufData_large_local);
        }
        else
        {
            if ( buf.cursize > buf.maxsize )
                Com_Error(ERR_DROP, "CL_ReadDemoMessage: demoMsglen > MAX_MSGLEN");
            v1 = FS_Read(buf.data, buf.cursize, clc->demofile);
            if ( v1 == buf.cursize )
            {
                clc->lastPacketTime = cls.realtime;
                buf.readcount = 0;
                v2 = MSG_ReadLong(&buf);
                clc->reliableAcknowledge = v2;
                if ( clc->reliableAcknowledge >= clc->reliableSequence - 128 )
                    CL_ParseServerMessage(localClientNum, &buf);
                else
                    clc->reliableAcknowledge = clc->reliableSequence;
                //LargeLocal::~LargeLocal(&bufData_large_local);
            }
            else
            {
                Com_Printf(14, "Demo file was truncated.\n");
                CL_DemoCompleted(localClientNum);
                //LargeLocal::~LargeLocal(&bufData_large_local);
            }
        }
    }
    else
    {
        CL_DemoCompleted(localClientNum);
        //LargeLocal::~LargeLocal(&bufData_large_local);
    }
}

void __cdecl CL_NextDemo(int localClientNum)
{
    char v[1028]; // [esp+0h] [ebp-408h] BYREF

    I_strncpyz(v, nextdemo->current.string, 1024);
    Com_DPrintf(14, "CL_NextDemo: %s\n", v);
    if ( v[0] )
    {
        Dvar_SetString((dvar_s *)nextdemo, "");
        Cbuf_AddText(localClientNum, v);
        Cbuf_AddText(localClientNum, "\n");
        Cbuf_Execute(localClientNum, 0);
    }
    else
    {
        Com_Error(ERR_DISCONNECT, "Demo is over");
    }
}

void __cdecl CL_ShutdownDevGui()
{
    CL_DestroyDevGui();
    DevGui_Shutdown();
    Cmd_RemoveCommand("devgui_dvar");
    Cmd_RemoveCommand("devgui_cmd");
    Cmd_RemoveCommand("devgui_open");
}

void __cdecl CL_ShutdownHunkUsers()
{
    Com_SyncThreads();
    if ( cls.hunkUsersStarted )
    {
        CL_ShutdownAllClientsCGame();
        //BLOPS_NULLSUB();
        if ( cls.devGuiStarted )
        {
            CL_ShutdownDevGui();
            cls.devGuiStarted = 0;
        }
        CL_ShutdownUI();
        if ( cls.uiStarted )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                            2360,
                            0,
                            "%s",
                            "!cls.uiStarted") )
                __debugbreak();
        }
        Live_FileShare_ClearSearchState();
        Live_FileShare_CacheShutdown();
        cls.hunkUsersStarted = 0;
    }
}

void __cdecl CL_ShutdownAll()
{
    int depth; // [esp+0h] [ebp-4h]

    depth = 0;
    if ( Sys_IsMainThread() )
        depth = R_PopRemoteScreenUpdate();
    R_SyncRenderThread();
    CL_ShutdownHunkUsers();
    if ( Sys_IsMainThread() )
        R_PushRemoteScreenUpdate(depth);
    if ( cls.rendererStarted )
    {
        CL_ShutdownRenderer(0);
        if ( cls.rendererStarted )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                            2392,
                            0,
                            "%s",
                            "!cls.rendererStarted") )
                __debugbreak();
        }
    }
    track_shutdown(3);
}

void __cdecl CL_ShutdownDemo()
{
    clientConnection_t *clc; // [esp+0h] [ebp-8h]

    clc = CL_GetLocalClientConnection(0);
    if ( clc->demofile )
    {
        FS_FCloseFile(clc->demofile);
        clc->demofile = 0;
        if ( clc->demoLiveStream )
        {
            FS_FCloseFile(clc->demoLiveStream);
            clc->demoLiveStream = 0;
        }
        clc->demoplaying = 0;
        clc->demorecording = 0;
    }
}

void __cdecl CL_MapLoading(const char *mapname)
{
    int localClientNum; // [esp+8h] [ebp-Ch]
    int localClientNuma; // [esp+8h] [ebp-Ch]
    int localClientNumb; // [esp+8h] [ebp-Ch]
    clientConnection_t *clc; // [esp+Ch] [ebp-8h]
    clientConnection_t *clca; // [esp+Ch] [ebp-8h]

    if (CL_AnyLocalClientsRunning())
    {
        g_waitingForServer = 0;
        FS_DisablePureCheck(0);
        for (localClientNum = 0; localClientNum < 1; ++localClientNum)
        {
            Con_Close(localClientNum);
            clientUIActives[localClientNum].keyCatchers = 0;
            CL_LocalClient_ClearCUIFlag(localClientNum, 64);
        }
        //BG_EvalVehicleName();
        LiveStorage_UploadStats();
        UI_CloseAllMenus(0);
        cl_serverLoadingMap = 1;
        if (!com_sv_running->current.enabled)
            Cbuf_ExecuteBuffer(0, 0, (char*)"selectStringTableEntryInDvar mp/didyouknow.csv 0 didyouknow");
        if (clientUIActives[0].connectionState >= CA_CONNECTED && !I_stricmp(cls.servername, "localhost"))
        {
            for (localClientNuma = 0; localClientNuma < 1; ++localClientNuma)
            {
                if (CL_LocalClient_IsActive(localClientNuma))
                {
                    CL_GetLocalClientGlobals(localClientNuma);
                    clc = CL_GetLocalClientConnection(localClientNuma);
                    clientUIActives[localClientNuma].connectionState = CA_CONNECTED;
                    memset(clc->serverMessage, 0, sizeof(clc->serverMessage));
                    memset(&cls.gameState, 0, sizeof(cls.gameState));
                    clc->lastPacketSentTime = -9999;
                    if (!*mapname
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                            2595,
                            0,
                            "%s",
                            "mapname[0]"))
                    {
                        __debugbreak();
                    }
                    cl_waitingOnServerToLoadMap[localClientNuma] = 0;
                }
            }
        }
        else
        {
            Dvar_SetString((dvar_s*)nextmap, "");
            I_strncpyz(cls.servername, "localhost", 256);
            for (localClientNumb = 0; localClientNumb < 1; ++localClientNumb)
            {
                if (CL_LocalClient_IsActive(localClientNumb) && Com_LocalClient_GetControllerIndex(localClientNumb) >= 0)
                {
                    CL_Disconnect(localClientNumb, 0);
                    //BG_EvalVehicleName();
                    UI_CloseAll(localClientNumb);
                    clientUIActives[localClientNumb].connectionState = CA_CHALLENGING;
                    clca = CL_GetLocalClientConnection(localClientNumb);
                    clca->connectTime = -3000;
                    clca->qport = localClientNumb + g_qport;
                    NET_StringToAdr(cls.servername, &clca->serverAddress);
                    CL_CheckForResend(localClientNumb);
                    if (!*mapname
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                            2626,
                            0,
                            "%s",
                            "mapname[0]"))
                    {
                        __debugbreak();
                    }
                    cl_waitingOnServerToLoadMap[localClientNumb] = 0;
                }
            }
        }
        SND_FadeOut();
    }
}

void __cdecl CL_ResetSkeletonCache(unsigned int localClientNum)
{
    clientActive_t *v1; // [esp+0h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                    2639,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( !clients
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp", 2640, 0, "%s", "clients") )
    {
        __debugbreak();
    }
    if ( localClientNum >= cl_maxLocalClients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                    2641,
                    0,
                    "localClientNum doesn't index cl_maxLocalClients\n\t%i not in [0, %i)",
                    localClientNum,
                    cl_maxLocalClients) )
    {
        __debugbreak();
    }
    v1 = &clients[localClientNum];
    if ( !++v1->skelTimeStamp )
        ++v1->skelTimeStamp;
    v1->skelMemoryStart = (char *)((unsigned int)&v1->skelMemory[15] & 0xFFFFFFF0);
    v1->skelMemPos = 0;
}

void __cdecl CL_ClearStaticDownload()
{
    if ( cls.wwwDlDisconnected
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                    2667,
                    0,
                    "%s",
                    "!cls.wwwDlDisconnected") )
    {
        __debugbreak();
    }
    cls.downloadRestart = 0;
    cls.downloadTempName[0] = 0;
    cls.downloadName[0] = 0;
    cls.originalDownloadName[0] = 0;
    cls.wwwDlInProgress = 0;
    cls.wwwDlDisconnected = 0;
    cls.downloadFlags = 0;
    DL_CancelDownload();
}

void __cdecl CL_ClearState(int localClientNum)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if ( LocalClientGlobals )
        memset(LocalClientGlobals, 0, sizeof(clientActive_t));
    Com_ClientDObjClearAllSkel();
}

void __cdecl CL_UploadStatsForController(int localControllerIndex)
{
#ifdef KISAK_LIVE
    if ( !Demo_IsPlaying() && !Demo_IsShutdownInProgress() )
    {
        if ( Com_GameMode_IsPublicOnlineGame() )
            LB_UploadPlayerStats(localControllerIndex);
        LiveStats_CompareStatsVsStableBuffer(localControllerIndex);
        if ( xblive_basictraining )
        {
            if ( xblive_basictraining->current.enabled )
                LiveStorage_UploadStatsForController();
        }
        Session_CleanUpStatsWrites();
    }
#endif
}

void __cdecl CL_Disconnect(unsigned int localClientNum, bool deactivateClient)
{
    int ControllerIndex; // eax
    int v4; // eax
    int v5; // eax
    int v6; // eax
    clientUIActive_t *clientUIActive; // [esp+10h] [ebp-10h]
    connstate_t connstate; // [esp+14h] [ebp-Ch]
    clientConnection_t *clc; // [esp+18h] [ebp-8h]
    bool wasConnected; // [esp+1Fh] [ebp-1h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                    2785,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    clientUIActive = CL_GetLocalClientUIGlobals(localClientNum);
    if ( CL_LocalClient_IsCUIFlagSet(localClientNum, 2) )
    {
        connstate = CL_GetLocalClientConnectionState(localClientNum);
        if ( connstate < CA_CONNECTED )
        {
            clc = 0;
        }
        else
        {
            SND_GameReset();
            CG_SndGameReset();
            clc = CL_GetLocalClientConnection(localClientNum);
        }
        if ( connstate )
            LiveSteam_Client_SteamDisconnect();
        if ( connstate >= CA_CONNECTED && clc->demorecording )
        {
            if ( clc->demoUseMemoryBuffer )
            {
                ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
                Cmd_ExecuteSingleCommand(localClientNum, ControllerIndex, (char*)"togglecoolmomentrecording");
            }
            else
            {
                v4 = Com_LocalClient_GetControllerIndex(localClientNum);
                Cmd_ExecuteSingleCommand(localClientNum, v4, (char *)"stoprecord");
            }
        }
        if ( !cls.wwwDlDisconnected )
        {
            if ( cls.download )
            {
                FS_FCloseFile(cls.download);
                cls.download = 0;
            }
            cls.downloadName[0] = 0;
            cls.downloadTempName[0] = 0;
            legacyHacks.cl_downloadName[0] = 0;
        }
        legacyHacks.cl_downloadName[0] = 0;
        if ( connstate >= CA_CONNECTED && clc->demofile )
        {
            FS_FCloseFile(clc->demofile);
            clc->demofile = 0;
            if ( clc->demoLiveStream )
            {
                FS_FCloseFile(clc->demoLiveStream);
                clc->demoLiveStream = 0;
            }
            clc->demoplaying = 0;
            clc->demorecording = 0;
        }
        SCR_StopCinematic();
        if ( connstate >= CA_CONNECTED && clc->reliableSequence - clc->reliableAcknowledge <= 128 )
        {
            CL_AddReliableCommand(localClientNum, "disconnect");
            CL_WritePacket(localClientNum);
            CL_WritePacket(localClientNum);
            CL_WritePacket(localClientNum);
        }
        wasConnected = connstate >= CA_CONNECTING;
#ifdef KISAK_DW
        if ( connstate >= CA_CONNECTING && clc )
        {
            dwCloseConnection(&clc->serverAddress);
            dwNetPump();
            dwNetPump();
            bdSessionID sessID; // [esp+0h] [ebp-20h] BYREF

            //bdSessionID::bdSessionID(&sessID);
            *(unsigned int *)sessID.m_sessionID.ab = 0;
            *(unsigned int *)&sessID.m_sessionID.ab[4] = 0;
            dwSetSessionID(&sessID);
            //bdTaskResult::~bdTaskResult(&sessID);
        }
#endif
        if ( clc )
        {
            clc->clientChallenge = 0;
            clc->nonce = 0;
            *(&clc->nonce + 1) = 0;
        }
        CL_ClearState(localClientNum);
        CL_ClearMutedList();
        if ( connstate >= CA_CONNECTED )
            memset(clc, 0, sizeof(clientConnection_t));
        clientUIActives[localClientNum].connectionState = CA_DISCONNECTED;
        if ( !cls.wwwDlDisconnected )
            CL_ClearStaticDownload();
        DynEntCl_Shutdown(localClientNum);
        GlassCl_Reset(localClientNum);
        SND_DisconnectListener(localClientNum);
        if ( wasConnected )
            clientUIActive->keyCatchers &= 1u;
        if (!wasConnected && deactivateClient)
        {
            //BLOPS_NULLSUB();
        }
        if ( wasConnected )
        {
            v5 = Com_LocalClient_GetControllerIndex(localClientNum);
            CL_UploadStatsForController(v5);
            v6 = Com_LocalClient_GetControllerIndex(localClientNum);
            CL_UpdateProfileAfterGame(v6);
        }

        // LWSS ADD
        Steam_CancelClientTicket();
        // LWSS END

        if ( CL_AllLocalClientsDisconnected() )
        {
            Dvar_SetBool((dvar_s *)sv_disableClientConsole, 0);
            cl_connectedToPureServer = 0;
            fs_checksumFeed = 0;
            if ( xblive_basictraining && xblive_basictraining->current.enabled )
                LiveStorage_UploadStats();
            LiveStorage_ReadStatsIfDirChanged();
        }
        if ( !Demo_IsIdle() )
            Demo_End(0);
        //BLOPS_NULLSUB();
    }
}

void __cdecl CL_UpdateProfileAfterGame(int controllerIndex)
{
    bool spectateThirdPerson; // [esp+3h] [ebp-1h]

    spectateThirdPerson = Dvar_GetInt(cg_spectateThirdPerson) != 0;
    GamerProfile_UpdateDvarsFromProfile(controllerIndex);
    Dvar_SetBool((dvar_s *)cg_spectateThirdPerson, spectateThirdPerson);
    GamerProfile_UpdateProfileFromDvars(controllerIndex, PROFILE_WRITE_IF_CHANGED);
}

void __cdecl CL_CanNotConnect(int localClientNum, const char *msg)
{
    char *v2; // eax
    char *v3; // eax

    if ( CL_GetLocalClientConnectionState(localClientNum) >= 6 || Dvar_GetBool("cl_wasconnected") )
    {
        Com_Error(ERR_DROP, "%s", msg);
    }
    else
    {
        CL_Disconnect(0, 1);
        UI_CloseMenu(0, "code_statusinfo_popmenu");
        v2 = UI_SafeTranslateString("@MENU_NOTICE_CAPS");
        Dvar_SetStringByName("notice_popmenuTitle", v2);
        v3 = UI_SafeTranslateString(msg);
        Dvar_SetStringByName("notice_popmenuMessage", v3);
        Dvar_SetStringByName("notice_onEscArg", (char *)"");
        UI_OpenMenu(0, "code_notice_popmenu");
    }
}

void __cdecl CL_ForwardCommandToServer(int localClientNum, const char *string)
{
    const char *cmd; // [esp+18h] [ebp-4h]

    cmd = Cmd_Argv(0);
    if ( *cmd != 45 )
    {
        if ( CL_GetLocalClientConnectionState(localClientNum) < 6
            || *cmd == 43
            || CL_GetLocalClientConnection(localClientNum)->demoplaying
            || Demo_IsPlaying() )
        {
            Com_Printf(14, "Unknown command \"%s\"\n", cmd);
        }
        else if ( Cmd_Argc() <= 1 )
        {
            CL_AddReliableCommand(localClientNum, cmd);
        }
        else
        {
            CL_AddReliableCommand(localClientNum, string);
        }
    }
}

void __cdecl CL_ForwardToServer_f()
{
    connstate_t connstate; // [esp+8h] [ebp-414h]
    clientConnection_t *clc; // [esp+10h] [ebp-40Ch]
    char command[1028]; // [esp+14h] [ebp-408h] BYREF

    clc = CL_GetLocalClientConnection(0);
    connstate = CL_GetLocalClientConnectionState(0);
    if ( clc->demoplaying || connstate != CA_ACTIVE )
    {
        Com_Printf(0, "Not connected to a server.\n");
    }
    else if ( Cmd_Argc() > 1 )
    {
        Cmd_ArgsBuffer(1, command, 1024);
        CL_AddReliableCommand(0, command);
    }
}

void __cdecl CL_Setenv_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    char *env; // [esp+30h] [ebp-414h]
    char buffer[1028]; // [esp+34h] [ebp-410h] BYREF
    int i; // [esp+43Ch] [ebp-8h]
    int argc; // [esp+440h] [ebp-4h]

    argc = Cmd_Argc();
    if ( argc <= 2 )
    {
        if ( argc == 2 )
        {
            v2 = Cmd_Argv(1);
            env = getenv(v2);
            if ( env )
            {
                v3 = Cmd_Argv(1);
                Com_Printf(0, "%s=%s\n", v3, env);
            }
            else
            {
                v4 = Cmd_Argv(1);
                Com_Printf(0, "%s undefined\n", v4);
            }
        }
    }
    else
    {
        v0 = Cmd_Argv(1);
        I_strncpyz(buffer, v0, 1024);
        I_strncat(buffer, 1024, "=");
        for ( i = 2; i < argc; ++i )
        {
            v1 = Cmd_Argv(i);
            I_strncat(buffer, 1024, v1);
            I_strncat(buffer, 1024, " ");
        }
        _putenv(buffer);
    }
}

void __cdecl CL_DisconnectLocalClient(int localClientNum)
{
    bool wasConnected; // [esp+7h] [ebp-1h]

    SCR_StopCinematic();
    wasConnected = CL_GetLocalClientConnectionState(localClientNum) >= 6;
    CL_Disconnect(localClientNum, 0);
    if ( Dvar_GetBool("cl_wasconnected") || wasConnected )
    {
        if ( CL_AllLocalClientsDisconnected() )
            Com_Error(ERR_DISCONNECT, "PLATFORM_DISCONNECTED_FROM_SERVER");
    }
}

void __cdecl CL_Reconnect_f()
{
    const char *v0; // eax

    if ( strlen(cls.servername) && strcmp(cls.servername, "localhost") )
    {
        v0 = va("connect %s\n", cls.servername);
        Cbuf_AddText(0, v0);
    }
    else
    {
        Com_Printf(0, "Can't reconnect to localhost.\n");
    }
}

void __cdecl CL_SendPureChecksums(int localClientNum)
{
    char *pChecksums; // [esp+0h] [ebp-40Ch]
    char cMsg[1028]; // [esp+4h] [ebp-408h] BYREF

    pChecksums = FS_ReferencedIwdPureChecksums();
    Com_sprintf(cMsg, 0x400u, "Va ");
    I_strncat(cMsg, 1024, pChecksums);
    cMsg[0] += 13;
    cMsg[1] += 15;
    CL_AddReliableCommand(localClientNum, cMsg);
}

void __cdecl CL_Vid_Restart_f()
{
    R_AbortRenderCommands();
    R_SyncRenderThread();
    R_CheckResizeWindow();
    memcpy(&cls.vidConfig, &vidConfig, sizeof(cls.vidConfig));
    CG_InitScreenDimensions(0);
    CL_SetupViewport();
}

void __cdecl CL_Snd_Restart_f()
{
    int n; // [esp+0h] [ebp-4h]

    if ( !com_sv_running->current.enabled || Com_IsMenuLevel(0) )
    {
        n = Dvar_GetInt("sd_xa2_device_name");
        Dvar_SetIntByName("sd_xa2_device_guid", n);
        Voice_Shutdown();
        SND_Shutdown();
        Com_SyncThreads();
        SND_Init();
        Voice_Init();
        CG_SndGameReset();
    }
    else
    {
        Com_Printf(0, "Listen server cannot sound restart.\n");
    }
}

void __cdecl CL_OpenedIWDList_f()
{
    const char *checksums; // [esp+0h] [ebp-8h] BYREF
    const char *names; // [esp+4h] [ebp-4h] BYREF

    FS_LoadedIwds(&checksums, &names);
    Com_Printf(0, "Opened IWD Names: %s\n", names);
}

void __cdecl CL_ReferencedIWDList_f()
{
    const char *checksums; // [esp+0h] [ebp-8h] BYREF
    const char *names; // [esp+4h] [ebp-4h] BYREF

    FS_ReferencedIwds(&checksums, &names);
    Com_Printf(0, "Referenced IWD Names: %s\n", names);
}

void __cdecl CL_Configstrings_f()
{
    int ofs; // [esp+4h] [ebp-Ch]
    int i; // [esp+Ch] [ebp-4h]

    if ( CL_GetLocalClientConnectionState(0) == 10 )
    {
        CL_GetLocalClientGlobals(0);
        for ( i = 0; i < MAX_CONFIGSTRINGS; ++i )
        {
            ofs = cls.gameState.stringOffsets[i];
            if ( ofs )
                Com_Printf(0, "%4i: %s\n", i, &cls.gameState.stringData[ofs]);
        }
    }
    else
    {
        Com_Printf(0, "Not connected to a server.\n");
    }
}

void __cdecl CL_Clientinfo_f()
{
    int LocalClientConnectionState; // eax
    char *v1; // eax

    Com_Printf(0, "--------- Client Information ---------\n");
    LocalClientConnectionState = CL_GetLocalClientConnectionState(0);
    Com_Printf(0, "state: %i\n", LocalClientConnectionState);
    Com_Printf(0, "Server: %s\n", cls.servername);
    Com_Printf(0, "User info settings:\n");
    v1 = Dvar_InfoString(0, 2);
    Info_Print(v1);
    Com_Printf(0, "--------------------------------------\n");
}

bool __cdecl CL_WasMapAlreadyLoaded()
{
    return com_sv_running->current.enabled;
}

void __cdecl CL_DownloadsComplete(int localClientNum)
{
    const char *v1; // eax
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    int client; // [esp+4h] [ebp-A4h]
    int playlistNum; // [esp+8h] [ebp-A0h]
    int playlistEntry; // [esp+Ch] [ebp-9Ch]
    char *info; // [esp+14h] [ebp-94h]
    char gametype[68]; // [esp+18h] [ebp-90h] BYREF
    clientConnection_t *clc; // [esp+5Ch] [ebp-4Ch]
    char mapname[68]; // [esp+60h] [ebp-48h] BYREF

    clc = CL_GetLocalClientConnection(localClientNum);
    if ( cls.downloadRestart )
    {
        if ( com_sv_running->current.enabled
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                        3898,
                        0,
                        "%s",
                        "!com_sv_running->current.enabled") )
        {
            __debugbreak();
        }
        cls.downloadRestart = 0;
        FS_Restart(localClientNum, clc->checksumFeed);
        if ( !cls.wwwDlDisconnected )
            CL_AddReliableCommand(localClientNum, "donedl");
        cls.wwwDlDisconnected = 0;
        CL_ClearStaticDownload();
    }
    else
    {
        Com_SyncThreads();
        if ( fs_gameDirVar && *(_BYTE *)fs_gameDirVar->current.integer )
        {
            v1 = va("%s/%s", fs_homepath->current.string, fs_gameDirVar->current.string);
            Sys_Mkdir(v1);
        }
        if ( cls.wwwDlDisconnected
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                        3936,
                        0,
                        "%s",
                        "!cls.wwwDlDisconnected") )
        {
            __debugbreak();
        }
        clientUIActives[localClientNum].connectionState = CA_LOADING;
        Com_Printf(14, "Setting state to CA_LOADING in CL_DownloadsComplete\n");
        if ( CL_WasMapAlreadyLoaded() )
        {
            if ( !cls.hunkUsersStarted
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                            4035,
                            0,
                            "%s",
                            "cls.hunkUsersStarted") )
            {
                __debugbreak();
            }
            if ( CL_LocalClient_IsCUIFlagSet(localClientNum, 32) )
            {
                CL_RestartCGame(localClientNum);
                return;
            }
        }
        else
        {
            info = CL_GetConfigString(0);
            v2 = Info_ValueForKey(info, "mapname");
            I_strncpyz(mapname, v2, 64);
            v3 = Info_ValueForKey(info, "g_gametype");
            I_strncpyz(gametype, v3, 64);
            v4 = Info_ValueForKey(info, "playlist_enabled");
            if ( atoi(v4) )
            {
                v5 = Info_ValueForKey(info, "playlist");
                playlistNum = atoi(v5);
                Dvar_SetInt((dvar_s *)playlist, playlistNum);
                Live_SetPlaylistNum(0, playlistNum);
                v6 = Info_ValueForKey(info, "playlist_entry");
                playlistEntry = atoi(v6);
                Dvar_SetInt((dvar_s *)playlist_entry, playlistEntry);
                Playlist_RunRules(0, playlistEntry);
            }
            if ( cls.doVidRestart )
            {
                UI_SetMap((char *)"", "");
                CL_Vid_Restart_f();
            }
            if ( !g_waitingForServer )
                LoadMapLoadscreen(mapname);
            UI_SetMap(mapname, gametype);
            SCR_UpdateScreen();
            CL_ShutdownAll();
            Com_Restart();
            if ( cls.hunkUsersStarted
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                            4013,
                            0,
                            "%s",
                            "!cls.hunkUsersStarted") )
            {
                __debugbreak();
            }
            CL_InitRenderer();
            CL_StartHunkUsers();
            SCR_UpdateScreen();
            for ( client = 0; client < 1; ++client )
                ;
        }
        Dvar_SetInt((dvar_s *)cl_paused, 1);
        CL_InitCGame(localClientNum);
        CL_SendPureChecksums(localClientNum);
        CL_WritePacket(localClientNum);
        CL_WritePacket(localClientNum);
        CL_WritePacket(localClientNum);
    }
}

void __cdecl LoadMapLoadscreen(char *mapname)
{
    DB_AddUserMapDir(mapname);
    Com_LoadMapLoadingScreenFastFile(mapname);
    DB_SyncXAssets();
    DB_UpdateDebugZone();
}

void __cdecl CL_BeginDownload(const char *localName, const char *remoteName)
{
    const char *v2; // eax

    Com_DPrintf(
        14,
        "***** CL_BeginDownload *****\nLocalname: %s\nRemotename: %s\n****************************\n",
        localName,
        remoteName);
    CL_GetLocalClientConnection(0);
    I_strncpyz(cls.downloadName, localName, 256);
    Com_sprintf(cls.downloadTempName, 0x100u, "%s.tmp", localName);
    I_strncpyz(legacyHacks.cl_downloadName, remoteName, 64);
    legacyHacks.cl_downloadSize = 0;
    legacyHacks.cl_downloadCount = 0;
    legacyHacks.cl_downloadTime = cls.realtime;
    cls.downloadBlock = 0;
    cls.downloadCount = 0;
    v2 = va("download %s", remoteName);
    CL_AddReliableCommand(0, v2);
}

void __cdecl CL_NextDownload(int localClientNum)
{
    char *localName; // [esp+24h] [ebp-Ch]
    char *s; // [esp+28h] [ebp-8h]
    char *sa; // [esp+28h] [ebp-8h]
    char *sb; // [esp+28h] [ebp-8h]
    unsigned __int8 *sc; // [esp+28h] [ebp-8h]
    char *remoteName; // [esp+2Ch] [ebp-4h]

    CL_GetLocalClientConnection(localClientNum);
    if (!cls.downloadList[0])
        goto LABEL_11;
    //if (com_sv_running->current.enabled)
    //    MyAssertHandler(".\\client_mp\\cl_main_mp.cpp", 2721, 0, "%s", "!com_sv_running->current.enabled");
    iassert(!com_sv_running->current.enabled);
    s = cls.downloadList;
    if (cls.downloadList[0] == '@')
        s = &cls.downloadList[1];
    remoteName = s;
    sa = strchr(s, '@');
    if (sa)
    {
        *sa = 0;
        localName = sa + 1;
        sb = strchr(sa + 1, '@');
        if (sb)
        {
            *sb = 0;
            sc = (unsigned __int8 *)(sb + 1);
        }
        else
        {
            sc = (unsigned __int8 *)&localName[strlen(localName)];
        }
        CL_BeginDownload(localName, remoteName);
        cls.downloadRestart = 1;
        memmove((unsigned __int8 *)cls.downloadList, sc, strlen((const char *)sc) + 1);
    }
    else
    {
    LABEL_11:
        CL_DownloadsComplete(localClientNum);
    }
}

void __cdecl CL_InitDownloads(int localClientNum)
{
    FS_SERVER_COMPARE_RESULT compareResult; // [esp+0h] [ebp-410h]
    FS_SERVER_COMPARE_RESULT compareResulta; // [esp+0h] [ebp-410h]
    char missingfiles[1028]; // [esp+8h] [ebp-408h] BYREF

    CL_GetLocalClientConnection(localClientNum);
    cls.wwwDlInProgress = 0;
    cls.wwwDlDisconnected = 0;
    CL_ClearStaticDownload();
    if ( !com_sv_running->current.enabled )
    {
        if ( cl_allowDownload->current.enabled )
        {
            compareResulta = FS_CompareWithServerFiles(cls.downloadList, 1024, 1);
            if ( compareResulta == NEED_DOWNLOAD )
            {
                Com_Printf(14, "Need files: %s\n", cls.downloadList);
                if (cls.downloadList[0])
                {
                    clientUIActives[localClientNum].connectionState = CA_CONNECTED;
                    CL_NextDownload(localClientNum);
                    return;
                }
            }
            else if ( compareResulta == NOT_DOWNLOADABLE )
            {
                Com_PrintError(14, "WARNING:: %s is different from the server!!!\n", cls.downloadList);
            }
        }
        else
        {
            compareResult = FS_CompareWithServerFiles(missingfiles, 1024, 0);
            if ( compareResult == NEED_DOWNLOAD )
            {
                Com_Error(ERR_DROP, "You are missing some files referenced by the server: %sGo to the Multiplayer options menu to allow downloads", missingfiles);
            }
            else if ( compareResult == NOT_DOWNLOADABLE )
            {
                Com_Error(ERR_DROP, "%s is different from the server", missingfiles);
            }
        }
    }
    CL_DownloadsComplete(localClientNum);
}

void __cdecl CL_CheckForResend(int localClientNum)
{
    __int64 Uid; // rax
    const char *v6; // eax
    unsigned int RandomUInt; // eax
#ifdef KISAK_DW
    bdTrulyRandomImpl *Instance; // eax
    bdTrulyRandomImpl *v9; // eax
#endif
    unsigned int v10; // eax
    netsrc_t v11; // eax
    netadr_t v12; // [esp-18h] [ebp-D68h]
    int v15; // [esp-4h] [ebp-D54h]
    unsigned __int64 ourUserID; // [esp+D8h] [ebp-C78h] BYREF
    char clientSteamIDStr[32]; // [esp+E4h] [ebp-C6Ch] BYREF
    connstate_t connstate; // [esp+104h] [ebp-C4Ch]
    char info[1028]; // [esp+108h] [ebp-C48h] BYREF
    int pktlen; // [esp+50Ch] [ebp-844h] BYREF
    unsigned __int8 data[1028]; // [esp+510h] [ebp-840h] BYREF
    char md5Str[36]; // [esp+914h] [ebp-43Ch] BYREF
    unsigned int infoLen; // [esp+938h] [ebp-418h]
    clientConnection_t *clc; // [esp+93Ch] [ebp-414h]
    char pkt[1036]; // [esp+940h] [ebp-410h] BYREF

    unsigned char *pSteamClientTicket = NULL;
    uint32 steamClientTicketSize = 0;
    char steamIDbuf[25];
    unsigned char steamTicketBase64[2048]{ 0 };

#ifdef _DEBUG
    unsigned char steamTicketDecodeBuf[1024]{ 0 };
#endif

    connstate = CL_GetLocalClientConnectionState(localClientNum);
    if ( connstate == CA_CONNECTING || connstate == CA_CHALLENGING || connstate == CA_SENDINGSTATS )
    {
        clc = CL_GetLocalClientConnection(localClientNum);
        if ( connstate == CA_SENDINGSTATS )
        {
            if ( cls.realtime - clc->lastPacketSentTime < 100 )
                return;
        }
        else if ( cls.realtime - clc->connectTime < 3000 )
        {
            return;
        }
        if ( !clc->demoplaying && !Demo_IsPlaying() )
        {
            clc->connectTime = cls.realtime;
            ++clc->connectPacketCount;
            if ( connstate == CA_CONNECTING )
            {
#ifdef KISAK_DW
                if ( dwGetAddrHandleConnectionTaskStatus(clc->serverAddress.addrHandleIndex) )
#endif
                {
                    //if (net_lanauthorize->current.enabled || !Sys_IsLANAddress(clc->serverAddress))
                    //{
                    //    //BLOPS_NULLSUB();
                    //    // IN COD4: CL_RequestAuthorization(localClientNum);
                    //}
                    
                    //strcpy(pkt, "getchallenge");
                    //pktlen = &pkt[strlen(pkt) + 1] - &pkt[1];
                    //PbClientConnecting(1, pkt, &pktlen);
                    //CL_BuildMd5StrFromCDKey(md5Str);


                    // NOTE: In Black Ops, the server does not give 2 shits about the CD Key.
                    // Go look at SV_GetChallenge(), it's not even read. This is a leftover.
                    //const char *dummy = "abc";
                    //NET_OutOfBandPrint(Com_LocalClient_GetNetworkID(localClientNum), serverAddress, va("getchallenge 0 \"%s\"", md5Str));
                    //NET_OutOfBandPrint(Com_LocalClient_GetNetworkID(localClientNum), clc->serverAddress, va("getchallenge 0 \"%s\"", dummy));

                    bool got = Steam_GetRawClientTicket(&pSteamClientTicket, &steamClientTicketSize);
                    iassert(got);
                    b64_encode(pSteamClientTicket, steamClientTicketSize, steamTicketBase64);
                    iassert(b64_decode(steamTicketBase64, strlen((char *)steamTicketBase64), steamTicketDecodeBuf) == steamClientTicketSize);

                    NET_OutOfBandPrint(Com_LocalClient_GetNetworkID(localClientNum), clc->serverAddress, va("getchallenge 0 \"%s\" \"%llu\"", steamTicketBase64, Steam_GetClientSteamID64()));

                    *(&clc->nonce + 1) = 0;
                    clc->nonce = 0;
                }
            }
            else if ( connstate == CA_CHALLENGING )
            {
                I_strncpyz(info, Dvar_InfoString(localClientNum, 2), 1024);
                Info_SetValueForKey(info, "protocol", va("%i", 1044));
                Info_SetValueForKey(info, "challenge", va("%i", clc->challenge));
                Uid = LiveSteam_GetUid();
                Int64ToString(Uid, clientSteamIDStr);
                Info_SetValueForKey(info, "steamid", clientSteamIDStr);
                if ( !clc->qport
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                                4414,
                                0,
                                "%s",
                                "clc->qport != 0") )
                {
                    __debugbreak();
                }
                v6 = va("%i", clc->qport);
                Info_SetValueForKey(info, "qport", v6);
#ifdef KISAK_LIVE_SERVICE
                if ( live_service && live_service->current.enabled )
                {
                    memset(temp64buff, 0, sizeof(temp64buff));
                    dwGetOnlineUserID(0, &ourUserID);
                    Com_DPrintf(14, "CHALLENGERESPONSE: Sending bdOnlineUserID %llu to server\n", ourUserID);
                    XUIDToString(&ourUserID, temp64buff);
                    Info_SetValueForKey(info, "bdOnlineUserID", temp64buff);
                    memset(temp64buff, 0, sizeof(temp64buff));
                    if ( !clc->nonce )
                    {
                        Instance = bdSingleton<bdTrulyRandomImpl>::getInstance();
                        RandomUInt = bdTrulyRandomImpl::getRandomUInt(Instance);
                        clc->nonce = RandomUInt;
                        v9 = bdSingleton<bdTrulyRandomImpl>::getInstance();
                        v10 = bdTrulyRandomImpl::getRandomUInt(v9);
                        *(&clc->nonce + 1) = v10;
                    }
                    Com_sprintf(temp64buff, 0xB1u, "%u", *(&clc->nonce + 1));
                    Info_SetValueForKey(info, "nonce", temp64buff);
                    Com_sprintf(temp64buff, 0xB1u, "%u", clc->nonce);
                    Info_SetValueForKey(info, "challengeNum", temp64buff);
                    Com_DPrintf(
                        14,
                        "CHALLENGERESPONSE: Sending nonce %u, challengeNum %u to server\n",
                        *(&clc->nonce + 1),
                        clc->nonce);
                }
#endif
                memcpy(data, "connect \"", 9);
                infoLen = &info[strlen(info) + 1] - &info[1];
                memcpy(&data[9], (unsigned __int8 *)info, infoLen);
                data[infoLen + 9] = 34;
                data[infoLen + 10] = 0;
                pktlen = infoLen + 10;
                memcpy((unsigned __int8 *)pkt, data, infoLen + 10);
                //PbClientConnecting(2, pkt, &pktlen);
                v15 = infoLen + 10;
                v12 = clc->serverAddress;
                v11 = Com_LocalClient_GetNetworkID(localClientNum);
                NET_OutOfBandData(v11, v12, data, v15);
                dvar_modifiedFlags &= ~2u;
            }
            else if ( connstate != CA_SENDINGSTATS )
            {
                Com_Error(ERR_FATAL, "CL_CheckForResend: bad connstate");
            }
        }
    }
}

void __cdecl CL_BuildMd5StrFromCDKey(char *md5Str)
{
#if 0
    int j; // [esp+20h] [ebp-C0h]
    int l; // [esp+24h] [ebp-BCh]
    MD5_CTX m1; // [esp+28h] [ebp-B8h] BYREF
    int i; // [esp+94h] [ebp-4Ch]
    char nums[64]; // [esp+98h] [ebp-48h] BYREF
    char *trimmedNum; // [esp+DCh] [ebp-4h]

    if ( !md5Str
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp", 3157, 0, "%s", "md5Str") )
    {
        __debugbreak();
    }
    memset(nums, 0, sizeof(nums));
    j = 0;
    l = strlen(cl_cdkey);
    if ( l > 32 )
        l = 32;
    for ( i = 0; i < l; ++i )
    {
        if ( cl_cdkey[i] >= 48 && cl_cdkey[i] <= 57
            || cl_cdkey[i] >= 97 && cl_cdkey[i] <= 122
            || cl_cdkey[i] >= 65 && cl_cdkey[i] <= 90 )
        {
            nums[j++] = cl_cdkey[i];
        }
    }
    nums[j] = 0;
    trimmedNum = strtrm(nums);
    strlwr(trimmedNum);
    MD5Init(&m1, (unsigned int)&loc_B684A3);
    MD5Update(&m1, (unsigned __int8 *)trimmedNum, strlen(trimmedNum));
    MD5Final(&m1);
    sprintf(
        md5Str,
        "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
        m1.digest[0],
        m1.digest[1],
        m1.digest[2],
        m1.digest[3],
        m1.digest[4],
        m1.digest[5],
        m1.digest[6],
        m1.digest[7],
        m1.digest[8],
        m1.digest[9],
        m1.digest[10],
        m1.digest[11],
        m1.digest[12],
        m1.digest[13],
        m1.digest[14],
        m1.digest[15]);
#endif
}

char *__cdecl strtrm(char *str)
{
    char *walk; // [esp+0h] [ebp-Ch]
    char *end; // [esp+8h] [ebp-4h]

    while ( *str == 32 || *str == 9 || *str == 10 || *str == 13 )
        ++str;
    end = str;
    for ( walk = str; *walk; ++walk )
    {
        if ( *walk != 32 && *walk != 9 && *walk != 10 && *walk != 13 )
            end = walk + 1;
    }
    *end = 0;
    return str;
}

void __cdecl CL_DisconnectError(const char *message)
{
    char *v1; // eax
    char *v2; // eax
    char *v3; // [esp-4h] [ebp-4h]

    if ( !message
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp", 4651, 0, "%s", "message") )
    {
        __debugbreak();
    }
    v3 = SEH_SafeTranslateString(message);
    v1 = SEH_SafeTranslateString("EXE_SERVERDISCONNECTREASON");
    v2 = UI_ReplaceConversionString(v1, v3);
    Com_Error(ERR_SERVERDISCONNECT, v2);
}

bool __cdecl CL_HandleVoiceTypePacket(int localClientNum, msg_t *msg)
{
    const char *c; // [esp+8h] [ebp-4h]

    c = Cmd_Argv(0);
    if ( I_stricmp(c, "v") )
    {
        return !I_stricmp(c, "vt") || I_stricmp(c, "relay") == 0;
    }
    else
    {
        CL_VoicePacket(localClientNum, msg);
        return 1;
    }
}

char __cdecl CL_ConnectionlessPacket(int localClientNum, netadr_t from, msg_t *msg, int time)
{
    const char *v5; // eax
    bool v6; // bl
    const char *v7; // eax
    int v8; // [esp-8h] [ebp-434h]
    const char *v9; // [esp-4h] [ebp-430h]
    const char *v10; // [esp-4h] [ebp-430h]
    unsigned __int8 highestSequence; // [esp+1Ah] [ebp-412h]
    bool sequenceReceived; // [esp+1Bh] [ebp-411h]
    char strBuf[1027]; // [esp+1Ch] [ebp-410h] BYREF
    bool success; // [esp+41Fh] [ebp-Dh]
    const char *s; // [esp+420h] [ebp-Ch]
    unsigned __int8 sequence; // [esp+427h] [ebp-5h]

    MSG_BeginReading(msg);
    MSG_ReadLong(msg);
    CL_Netchan_AddOOBProfilePacket(localClientNum, msg->cursize);
    //if ( !strnicmp((const char *)msg->data + 4, "PB_", 3u) )
    //{
    //    if ( msg->data[7] == 83 || msg->data[7] == 50 || msg->data[7] == 73 )
    //        PbSvAddEvent(13, -1, msg->cursize - 4, (char *)msg->data + 4);
    //    else
    //        PbClAddEvent(13, msg->cursize - 4, (char *)msg->data + 4);
    //    return 1;
    //}
    //else
    {
        s = MSG_ReadStringLine(msg, strBuf, 0x400u);
        if ( !strcmp(s, "R") )
        {
            success = 0;
            sequenceReceived = 0;
            highestSequence = 0;
            while ( !msg->overflowed )
            {
                sequence = MSG_ReadByte(msg);
                if ( msg->overflowed )
                    break;
                sequenceReceived = 1;
                s = MSG_ReadStringLine(msg, strBuf, 0x400u);
                highestSequence = sequence;
                if ( showpackets->current.integer )
                {
                    v9 = s;
                    v8 = sequence;
                    v5 = NET_AdrToString(from);
                    Com_Printf(16, "reliable recv: %s (%i)->'%s'\n", v5, v8, v9);
                }
                Cmd_TokenizeStringNoEval(s);
                v6 = success;
                success = CL_DispatchConnectionlessPacket(localClientNum, from, msg) || v6;
                Cmd_EndTokenizedString();
            }
            if ( sequenceReceived )
                RMsg_AckSequence(&from, highestSequence);
            return success;
        }
        else
        {
            if ( showpackets->current.integer )
            {
                v10 = s;
                v7 = NET_AdrToString(from);
                Com_Printf(16, "recv: %s->'%s'\n", v7, v10);
            }
            Cmd_TokenizeStringNoEval(s);
            success = CL_DispatchConnectionlessPacket(localClientNum, from, msg);
            Cmd_EndTokenizedString();
            return success;
        }
    }
}

char    CL_DispatchConnectionlessPacket(int localClientNum, netadr_t from, msg_t *msg)
{
    const char *v5; // eax
    unsigned __int8 v6; // al
    const char *v7; // eax
    char *v8; // eax
    netsrc_t v9; // eax
    int ControllerIndex; // eax
    int v11; // eax
    const char *v12; // eax
    int v13; // ecx
    clientConnection_t *v14; // edx
    const char *v15; // eax
    const char *v16; // eax
    netsrc_t NetworkID; // eax
    const char *StringLine; // eax
    const char *v19; // eax
    char *v20; // eax
    netchan_t *p_netchan; // [esp-28h] [ebp-8F0h]
    netadr_t v22; // [esp-24h] [ebp-8ECh]
    int v23; // [esp-14h] [ebp-8DCh]
    netadr_t v24; // [esp-14h] [ebp-8DCh]
    char *netchanOutgoingBuffer; // [esp-10h] [ebp-8D8h]
    char *netchanIncomingBuffer; // [esp-8h] [ebp-8D0h]
    const char *v27; // [esp-8h] [ebp-8D0h]
    char *v28; // [esp-4h] [ebp-8CCh]
    const char *v29; // [esp-4h] [ebp-8CCh]
    connstate_t v30; // [esp-4h] [ebp-8CCh]
    bool v31; // [esp+0h] [ebp-8C8h]
    const char *v32; // [esp+4h] [ebp-8C4h]
    unsigned __int64 serverSteamID; // [esp+50h] [ebp-878h]
    bool isMigration; // [esp+5Bh] [ebp-86Dh]
    char string[1028]; // [esp+60h] [ebp-868h] BYREF
    char *text_in; // [esp+464h] [ebp-464h]
    connstate_t connstate; // [esp+468h] [ebp-460h]
    const char *c; // [esp+46Ch] [ebp-45Ch]
    __int64 statPacketsNeeded; // [esp+470h] [ebp-458h]
    char strBuf[1024]; // [esp+478h] [ebp-450h] BYREF
    clientConnection_t *clc; // [esp+878h] [ebp-50h]
    const char *s; // [esp+87Ch] [ebp-4Ch]
    char mapname[68]; // [esp+880h] [ebp-48h] BYREF

    c = Cmd_Argv(0);
    if ( CL_HandleVoiceTypePacket(localClientNum, msg) )
        return 1;
    if ( !I_strcmp(c, "RA") )
    {
        v5 = Cmd_Argv(1);
        v6 = atoi(v5);
        RMsg_MarkPacketReceived(&from, v6);
        return 1;
    }
    connstate = CL_GetLocalClientConnectionState(localClientNum);
    if ( I_stricmp(c, "challengeResponse") )
    {
        if ( !I_stricmp(c, "rcon") )
        {
            SV_WaitServer();
            msg->overflowed = 0;
            msg->readcount = 0;
            text_in = MSG_ReadStringLine(msg, string, 0x400u);
            SV_Cmd_TokenizeString(text_in);
            SVC_RemoteCommand(from);
            SV_Cmd_EndTokenizedString();
        }
        if ( I_stricmp(c, "connectResponse") && I_stricmp(c, "connectResponseMigration") )
        {
            if ( I_stricmp(c, "statresponse") )
            {
                if ( !I_stricmp(c, "statsOK") )
                {
                    if (clientUIActives[localClientNum].connectionState == CA_SENDINGSTATS)
                        clientUIActives[localClientNum].connectionState = CA_CONNECTED;
                    else
                        Com_DPrintf(14, "Ignoring statsOK, I'm not CA_SENDINGSTATS\n");
                }
                if ( !I_strcmp(c, "steamauthReq") )
                {
                    serverSteamID = MSG_ReadInt64(msg);
                    CL_SteamServerAuth(serverSteamID);
                }
                if ( I_stricmp(c, "disconnect") )
                {
                    if ( !I_stricmp(c, "echo") )
                    {
                        v16 = Cmd_Argv(1);
                        v29 = va("%s", v16);
                        v24 = from;
                        NetworkID = Com_LocalClient_GetNetworkID(localClientNum);
                        NET_OutOfBandPrint(NetworkID, v24, v29);
                        return 1;
                    }
                    if ( !I_stricmp(c, "keyAuthorize") )
                        return 1;
                    if ( !I_stricmp(c, "print") )
                    {
                        clc = CL_GetLocalClientConnection(localClientNum);
                        s = MSG_ReadBigString(msg);
                        I_strncpyz(clc->serverMessage, s, 256);
                        Com_sprintf(printBuf, 0x800u, "%s", s);
                        Com_PrintMessage(14, printBuf, 0);
                        return 1;
                    }
                    if ( I_stricmp(c, "error") )
                    {
                        if ( I_stricmp(c, "loadingnewmap") )
                        {
                            if ( I_stricmp(c, "fastrestart") )
                            {
                                if ( I_stricmp(c, "XX") )
                                {
                                    return 0;
                                }
                                else
                                {
                                    v30 = connstate;
                                    v27 = c;
                                    v20 = NET_AdrToString(from);
                                    Com_DPrintf(14, "%s: Got msg '%s' from server, my connstate is %i.\n", v20, v27, v30);
                                    return 1;
                                }
                            }
                            else
                            {
                                clc = CL_GetLocalClientConnection(localClientNum);
                                v31 = CL_GetLocalClientConnectionState(localClientNum) == 10
                                     && NET_CompareBaseAdr(from, clc->serverAddress);
                                clc->isServerRestarting = v31;
                                return 1;
                            }
                        }
                        else
                        {
                            clc = CL_GetLocalClientConnection(localClientNum);
                            if ( NET_CompareBaseAdr(from, clc->serverAddress) && clc->serverAddress.type != NA_LOOPBACK )
                            {
                                if ( cls.downloadName[0] )
                                    return 1;
                                UI_CloseAllMenus(localClientNum);
                                Cbuf_AddText(localClientNum, "uploadStats\n");
                                StringLine = MSG_ReadStringLine(msg, strBuf, 0x400u);
                                I_strncpyz(mapname, StringLine, 64);
                                clientUIActives[localClientNum].connectionState = CA_CONNECTED;
                                v19 = MSG_ReadStringLine(msg, strBuf, 0x400u);
                                CL_SetupForNewServerMap(localClientNum, mapname, v19);
                            }
                            return 1;
                        }
                    }
                    else if ( connstate
                                 && (clc = CL_GetLocalClientConnection(localClientNum), NET_CompareBaseAdr(from, clc->serverAddress)) )
                    {
                        s = MSG_ReadBigString(msg);
                        CL_CanNotConnect(localClientNum, s);
                        return 1;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    if ( Cmd_Argc() <= 1 )
                    {
                        CL_DisconnectPacket(localClientNum, from, 0);
                    }
                    else
                    {
                        v15 = Cmd_Argv(1);
                        CL_DisconnectPacket(localClientNum, from, v15);
                    }
                    return 1;
                }
            }
            else if ( connstate <= CA_SENDINGSTATS )
            {
                if ( connstate == CA_SENDINGSTATS )
                {
                    clc = CL_GetLocalClientConnection(localClientNum);
                    v12 = Cmd_Argv(1);
                    statPacketsNeeded = I_atoi64(v12);
                    if ( statPacketsNeeded )
                    {
                        v13 = BYTE4(statPacketsNeeded) & 7;
                        v14 = clc;
                        LODWORD(clc->statPacketsToSend) = statPacketsNeeded;
                        HIDWORD(v14->statPacketsToSend) = v13;
                    }
                    else
                    {
                        clientUIActives[localClientNum].connectionState = CA_CONNECTED;
                        clc->statPacketsToSend = 0;
                    }
                    clc->lastPacketTime = cls.realtime;
                    clc->lastPacketSentTime = -9999;
                    return 1;
                }
                else
                {
                    Com_Printf(14, "statresponse packet while not syncing stats.    Ignored.\n");
                    return 0;
                }
            }
            else
            {
                Com_Printf(14, "Dup statresponse received.    Ignored.\n");
                return 0;
            }
        }
        else
        {
            isMigration = I_stricmp(c, "connectResponseMigration") == 0;
            if ( connstate < CA_CONNECTED )
            {
                if ( connstate == CA_CHALLENGING )
                {
                    clc = CL_GetLocalClientConnection(localClientNum);
                    if ( NET_CompareBaseAdr(from, clc->serverAddress) )
                    {
                        netchanIncomingBuffer = clc->netchanIncomingBuffer;
                        netchanOutgoingBuffer = clc->netchanOutgoingBuffer;
                        v23 = localClientNum + g_qport;
                        v22 = from;
                        p_netchan = &clc->netchan;
                        v9 = Com_LocalClient_GetNetworkID(localClientNum);
                        Netchan_Setup(v9, p_netchan, v22, v23, netchanOutgoingBuffer, 2048, netchanIncomingBuffer, 0x10000);
                        if ( Cmd_Argc() <= 1 )
                            v32 = "";
                        else
                            v32 = Cmd_Argv(1);
                        if (I_stricmp(v32, fs_gameDirVar->current.string))
                        {
                            //BLOPS_NULLSUB();
                        }
                        if ( isMigration )
                        {
                            clientUIActives[localClientNum].connectionState = CA_CONNECTED;
                        }
                        else
                        {
                            CL_SetLocalClientConnectionState(localClientNum, CA_SENDINGSTATS);
                            ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
                            LiveStats_MakeStableStatsBuffer(ControllerIndex);
                            if ( !xblive_basictraining->current.enabled )
                            {
                                v11 = Com_LocalClient_GetControllerIndex(localClientNum);
                                LiveStats_MakeStableGlobalStatsBuffer(v11);
                            }
                            memset(clc->statPacketSendTime, 0, 0x8Cu);
                            clc->statPacketsToSend = 0x7FFFFFFFFLL;
                        }
                        clc->lastPacketTime = cls.realtime;
                        clc->lastPacketSentTime = -9999;
                        return 1;
                    }
                    else
                    {
                        Com_Printf(14, "connectResponse from a different address.    Ignored.\n");
                        v28 = NET_AdrToString(clc->serverAddress);
                        v8 = NET_AdrToString(from);
                        Com_Printf(14, "%s should have been %s\n", v8, v28);
                        return 0;
                    }
                }
                else
                {
                    Com_Printf(14, "connectResponse packet while not connecting.    Ignored.\n");
                    return 0;
                }
            }
            else
            {
                Com_Printf(14, "Dup connect received.    Ignored.\n");
                return 0;
            }
        }
    }
    else if ( connstate == CA_CONNECTING )
    {
        clc = CL_GetLocalClientConnection(localClientNum);
        v7 = Cmd_Argv(1);
        clc->challenge = atoi(v7);
        clientUIActives[localClientNum].connectionState = CA_CHALLENGING;
        clc->connectPacketCount = 0;
        clc->connectTime = -99999;
        clc->serverAddress = from;
        Com_DPrintf(14, "challenge: %d\n", clc->challenge);
        return 1;
    }
    else
    {
        Com_Printf(14, "Unwanted challenge response received.    Ignored.\n");
        return 0;
    }
}

void __cdecl CL_DisconnectPacket(int localClientNum, netadr_t from, const char *reason)
{
    clientConnection_t *clc; // [esp+4h] [ebp-8h]

    if ( CL_GetLocalClientConnectionState(localClientNum) >= 4 )
    {
        clc = CL_GetLocalClientConnection(localClientNum);
        if ( NET_CompareAdr(from, clc->netchan.remoteAddress) )
        {
            if ( cls.realtime - clc->lastPacketTime >= 3000 )
            {
                if ( cls.wwwDlDisconnected )
                {
                    CL_Disconnect(localClientNum, 1);
                }
                else if ( reason )
                {
                    CL_DisconnectError(reason);
                }
                else
                {
                    Com_Error(ERR_DROP, "EXE_SERVER_DISCONNECTED");
                }
            }
        }
    }
}

void __cdecl CL_SteamServerAuth(unsigned __int64 serverSteamID)
{
    unsigned __int64 Uid; // rax
    unsigned __int8 data[2052]; // [esp+0h] [ebp-1040h] BYREF
    msg_t buf; // [esp+804h] [ebp-83Ch] BYREF
    clientConnection_t *LocalClientConnection; // [esp+834h] [ebp-80Ch]
    unsigned __int8 authBlob[2048]; // [esp+838h] [ebp-808h] BYREF
    int c; // [esp+103Ch] [ebp-4h]

    c = LiveSteam_Client_ConnectToSteamServer(serverSteamID, authBlob, 0x800u);
    MSG_Init(&buf, data, 2048);
    MSG_WriteString(&buf, "steamAuth");
    Uid = LiveSteam_GetUid();
    MSG_WriteInt64(&buf, Uid);
    MSG_WriteShort(&buf, c);
    MSG_WriteData(&buf, authBlob, c);
    LocalClientConnection = CL_GetLocalClientConnection(0);
    NET_OutOfBandData(NS_CLIENT1, LocalClientConnection->serverAddress, buf.data, buf.cursize);
}

void __cdecl CL_AllocatePerLocalClientMemory()
{
    int maxClients; // [esp+4h] [ebp-8h]

    if (IsDedicatedServer())
    {
        maxClients = sv_dedicatedmaxclients->current.integer;
    }
    else
    {
        maxClients = 32;
    }

    if ( cl_maxLocalClients != MAX_LOCAL_CLIENTS || com_maxclients->current.integer != maxClients )
    {
        AllocatePerLocalClientMemory(MAX_LOCAL_CLIENTS, maxClients, 0);
        Dvar_SetIntIfChanged((dvar_s *)com_maxclients, maxClients);
    }
}

const char *PerLocalClientMemoryName = "PerLocalClient";
unsigned __int8 *perLocalClientMemBuffer;
HunkUser *perLocalClientMemHunk;
int cl_allocatedClients = 32;
unsigned int cl_lastAllocFlags;
void __cdecl AllocatePerLocalClientMemory(int maxLocalClients, int maxClients, unsigned int flags)
{
    int mem_needed; // [esp+0h] [ebp-4h]
    int mem_neededa; // [esp+0h] [ebp-4h]
    int mem_neededb; // [esp+0h] [ebp-4h]

    CL_FreePerLocalClientMemory();
    mem_needed = 2449480 * maxLocalClients;
    if ( (flags & 1) == 0 )
        mem_needed += (int)SV_AllocateClientMemory_SizeRequired(maxLocalClients, maxClients);
    mem_neededa = mem_needed + CG_AllocateClientMemory_SizeRequired(maxLocalClients);
    mem_neededb = mem_neededa + FX_AllocateClientMemory_SizeRequired(maxLocalClients);
    PMem_BeginAlloc(PerLocalClientMemoryName, 1u, TRACK_CLIENT);
    perLocalClientMemBuffer = _PMem_AllocNamed(mem_neededb, 0x80u, 4, 1u, PerLocalClientMemoryName, TRACK_CLIENT);
    memset(perLocalClientMemBuffer, 0, mem_neededb);
    PMem_EndAlloc(PerLocalClientMemoryName, 1u);
    perLocalClientMemHunk = Hunk_UserCreateFromBuffer(
                                                        perLocalClientMemBuffer,
                                                        mem_neededb,
                                                        HU_SCHEME_DEFAULT,
                                                        8u,
                                                        0,
                                                        "PerLocalClient Memory",
                                                        42);
    clients = (clientActive_t *)Hunk_UserAlloc(perLocalClientMemHunk, 1728768 * maxLocalClients, 4, "clients");
    clientConnections = (clientConnection_t *)Hunk_UserAlloc(
                                                                                            perLocalClientMemHunk,
                                                                                            720712 * maxLocalClients,
                                                                                            4,
                                                                                            "clientConnections");
    memset(clients, 0, 1728768 * maxLocalClients);
    memset(clientConnections, 0, 720712 * maxLocalClients);
    memset(&cls.gameState, 0, sizeof(cls.gameState));
    cls.serverId = 0;
    if ( (flags & 1) == 0 )
        SV_AllocateClientMemory(perLocalClientMemHunk, maxLocalClients, maxClients);
    CG_AllocateClientMemory(perLocalClientMemHunk, maxLocalClients);
    FX_AllocateClientMemory(perLocalClientMemHunk, maxLocalClients);
    cl_maxLocalClients = maxLocalClients;
    cl_allocatedClients = maxClients;
    cl_lastAllocFlags = flags;
}

void __cdecl CL_FreePerLocalClientMemory()
{
    Com_UnloadFrontEnd();
    if ( perLocalClientMemHunk )
    {
        if ( (!clients || !clientConnections)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                        5459,
                        0,
                        "%s",
                        "clients && clientConnections") )
        {
            __debugbreak();
        }
        Hunk_UserFree(perLocalClientMemHunk, clients);
        Hunk_UserFree(perLocalClientMemHunk, clientConnections);
    }
    if ( cl_maxLocalClients > 0 )
    {
        PMem_Free(PerLocalClientMemoryName);
        clients = 0;
        clientConnections = 0;
        perLocalClientMemBuffer = 0;
    }
    FX_FreeClientMemory(perLocalClientMemHunk);
    CG_FreeClientMemory(perLocalClientMemHunk, cl_maxLocalClients);
    SV_FreeClientMemory(perLocalClientMemHunk);
    perLocalClientMemHunk = 0;
    cl_maxLocalClients = 0;
    Dvar_SetIntIfChanged((dvar_s *)com_maxclients, 32);
    cl_allocatedClients = 32;
}

void    CL_InitLoad(char *mapname, const char *gametype)
{
    connstate_t connstate; // [esp+0h] [ebp-4h]

    if ( CL_AnyLocalClientsRunning() )
    {
        com_expectedHunkUsage = 0;
        UI_SetMap(mapname, gametype);
        connstate = CL_GetLocalClientConnectionState(0);
        CL_SetLocalClientConnectionState(0, connstate < CA_CONNECTED ? CA_DISCONNECTED : CA_CONNECTED);
        SCR_UpdateScreen();
    }
}

bool __cdecl CL_PacketEvent(int localClientNum, netadr_t from, msg_t *msg, int time, bool connectionlesss_packets_only)
{
    int ControllerIndex; // eax
    clientActive_t *LocalClientGlobals; // [esp+4h] [ebp-1Ch]
    connstate_t connstate; // [esp+8h] [ebp-18h]
    int savedServerMessageSequence; // [esp+Ch] [ebp-14h]
    clientConnection_t *clc; // [esp+10h] [ebp-10h]
    int headerBytes; // [esp+14h] [ebp-Ch]
    int savedReliableAcknowledge; // [esp+18h] [ebp-8h]

    volatile int startingSize = msg->cursize;

    if (msg->cursize >= 4 && *(unsigned int *)msg->data == -1)
    {
        return CL_ConnectionlessPacket(localClientNum, from, msg, time);
    }

    if (connectionlesss_packets_only)
    {
        return false;
    }

    connstate = CL_GetLocalClientConnectionState(localClientNum);

    if (connstate < CA_CONNECTED)
    {
        Com_DPrintf(14, "%s: Got msg sequence %i but connstate (%i) is < CA_CONNECTED\n", NET_AdrToString(from), *(unsigned int *)msg->data, connstate);
        return false;
    }

    clc = CL_GetLocalClientConnection(localClientNum);

    if (msg->cursize < 4)
    {
        Com_Printf(14, "%s: Runt packet\n", NET_AdrToString(from));
        return true;
    }

    if (!NET_CompareAdr(from, clc->netchan.remoteAddress))
    {
        Com_DPrintf(14, "%s:sequenced packet without connection\n", NET_AdrToString(from));
        return false;
    }

    clc->lastPacketTime = cls.realtime;

    if (!Netchan_Process(&clc->netchan, msg))
    {
        return false;
    }

    headerBytes = msg->readcount;
    savedServerMessageSequence = clc->serverMessageSequence;
    savedReliableAcknowledge = clc->reliableAcknowledge;
    clc->serverMessageSequence = *(int *)msg->data;
    clc->reliableAcknowledge = MSG_ReadLong(msg);

    if (clc->reliableAcknowledge < (clc->reliableSequence - 128))
    {
        clc->reliableAcknowledge = clc->reliableSequence;
        return false;
    }

    CL_ParseServerMessage(localClientNum, msg);

    if ( msg->overflowed )
    {
        Com_DPrintf(14, "ignoring illegible message\n");
        clc->serverMessageSequence = savedServerMessageSequence;
        clc->reliableAcknowledge = savedReliableAcknowledge;
        return false;
    }

    if ( clc->demorecording && !clc->demowaiting )
    {
        if ( clc->demoUseMemoryBuffer )
        {
            if ( clc->demoReceivedUncompressedPacket )
            {
                CL_WriteUncompressedDemoInfo(localClientNum);
                clc->demoReceivedUncompressedPacket = 0;
            }
            clc->demoContinuousStateBufferIndex = (clc->demoContinuousStateBufferIndex + 1) % 200;
            CL_WriteNewDemoClientArchiveToMemory(localClientNum);
            CL_WriteDemoMessageToMemory(localClientNum, msg, headerBytes);
            clc->demoContinuousStateBuffer[clc->demoContinuousStateBufferIndex].intialBufferStateIndex = clc->demoInitialStateBufferIndex;
            if ( clc->demoCoolRecordProcessingExtendedCapture )
            {
                if ( Sys_Milliseconds() - clc->demoCoolRecordProcessingTime >= 1000 * cl_clientDemoCoolMomentExtendTime->current.integer )
                {
                    Com_Printf(14, "Writing Epilog through the cool record extended system.\n");
                    CL_WriteDemoEpilog(localClientNum);
                    clc->demoCoolRecordProcessingExtendedCapture = 0;
                }
                else
                {
                    CL_WriteDemoFromContinuousStateBuffer(localClientNum, clc->demoContinuousStateBufferIndex);
                }
            }
            if ( Sys_Milliseconds() - clc->demoRequestUncompressedPacketTime >= 1000 * cl_clientDemoRequestPingTime->current.integer )
                clc->demoRequestUncompressedPacket = 1;
        }
        else
        {
            CL_WriteNewDemoClientArchive(localClientNum);
            CL_WriteDemoMessage(localClientNum, msg, headerBytes);
            if ( Sys_Milliseconds() - clc->demoRecordStartTime >= 1000 * cl_clientDemoMaxRecordTime->current.integer && cl_clientDemoType->current.integer == 1 )
            {
                ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
                Cmd_ExecuteSingleCommand(localClientNum, ControllerIndex, (char*)"stoprecord");
            }
        }
        if ( clc->demoLiveStream )
        {
            LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
            FS_Printf(clc->demoLiveStream, (char *)"%i\n", LocalClientGlobals->serverTime);
            if ( cl_demoLiveStreaming->current.integer > 1 )
                FS_Flush(clc->demoLiveStream);
        }
    }
    return true;
}

void __cdecl CL_WriteNewDemoClientArchive(int localClientNum)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-103Ch]
    unsigned __int8 data[4096]; // [esp+4h] [ebp-1038h] BYREF
    msg_t buf; // [esp+1008h] [ebp-34h] BYREF
    clientConnection_t *LocalClientConnection; // [esp+1038h] [ebp-4h]

    LocalClientConnection = CL_GetLocalClientConnection(localClientNum);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    MSG_Init(&buf, data, 4096);
    while ( LocalClientConnection->lastClientArchiveIndex != LocalClientGlobals->clientArchiveIndex )
    {
        //BLOPS_NULLSUB();
        CL_WriteDemoClientArchive(
            LocalClientConnection->demofile,
            1u,
            &LocalClientGlobals->clientArchive[LocalClientConnection->lastClientArchiveIndex],
            LocalClientConnection->lastClientArchiveIndex);
        LocalClientConnection->lastClientArchiveIndex = (LocalClientConnection->lastClientArchiveIndex + 1) % 256;
    }
}

void __cdecl CL_WriteNewDemoClientArchiveToMemory(int localClientNum)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-18h]
    demoContinuousStateBuffer_t *demoBuffer; // [esp+4h] [ebp-14h]
    int clientArchiveCount; // [esp+8h] [ebp-10h]
    int clientArchiveCounta; // [esp+8h] [ebp-10h]
    clientConnection_t *clc; // [esp+Ch] [ebp-Ch]
    int index; // [esp+10h] [ebp-8h]

    index = 0;
    clc = CL_GetLocalClientConnection(localClientNum);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    demoBuffer = &clc->demoContinuousStateBuffer[clc->demoContinuousStateBufferIndex];
    if ( clc->demoContinuousStateBuffer[clc->demoContinuousStateBufferIndex].archive )
    {
        clientArchiveCount = CL_GetClientArchiveCount(
                                                     demoBuffer->startClientArchiveIndex,
                                                     demoBuffer->endClientArchiveIndex);
        CL_DemoMemoryPoolDeallocate(clc, 48 * clientArchiveCount);
    }
    clientArchiveCounta = CL_GetClientArchiveCount(clc->lastClientArchiveIndex, LocalClientGlobals->clientArchiveIndex);
    demoBuffer->archive = CL_DemoMemoryPoolAllocate(clc, 48 * clientArchiveCounta);
    demoBuffer->startClientArchiveIndex = clc->lastClientArchiveIndex;
    demoBuffer->endClientArchiveIndex = LocalClientGlobals->clientArchiveIndex;
    while ( clc->lastClientArchiveIndex != LocalClientGlobals->clientArchiveIndex )
    {
        memcpy(
            &demoBuffer->archive[48 * index],
            (unsigned __int8 *)&LocalClientGlobals->clientArchive[clc->lastClientArchiveIndex],
            0x30u);
        clc->lastClientArchiveIndex = (clc->lastClientArchiveIndex + 1) % 256;
        ++index;
    }
}

unsigned __int8 *__cdecl CL_DemoMemoryPoolAllocate(clientConnection_t *clc, int size)
{
    unsigned int v2; // edx
    int diff; // [esp+0h] [ebp-Ch]
    unsigned __int8 *address; // [esp+4h] [ebp-8h]
    unsigned int newSize; // [esp+8h] [ebp-4h]

    if ( (unsigned int)&clc->demoMemoryPool[clc->demoMemoryPoolEnd] % 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                    644,
                    0,
                    "%s",
                    "( (unsigned int)&clc->demoMemoryPool[clc->demoMemoryPoolEnd] ) % 4 == 0") )
    {
        __debugbreak();
    }
    newSize = (size + 3) & 0xFFFFFFFC;
    v2 = (size + 3) & 0x80000000;
    if ( size + 3 < 0 )
        v2 = 0;
    if ( v2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                    648,
                    0,
                    "%s",
                    "newSize % 4 == 0") )
    {
        __debugbreak();
    }
    if ( (int)(newSize + clc->demoMemoryPoolEnd) > 0x40000 )
    {
        clc->demoMemoryPoolEnd = 0;
        if ( !clc->demoMemoryPoolBegin
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                        653,
                        0,
                        "%s",
                        "clc->demoMemoryPoolBegin != 0") )
        {
            __debugbreak();
        }
        Com_Printf(14, "The memory pool allocate End position was reset to 0\n");
    }
    address = &clc->demoMemoryPool[clc->demoMemoryPoolEnd];
    diff = clc->demoMemoryPoolEnd - clc->demoMemoryPoolBegin;
    clc->demoMemoryPoolEnd += newSize;
    if ( ((diff >> 31) & 1) != (((clc->demoMemoryPoolEnd - clc->demoMemoryPoolBegin) >> 31) & 1)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                    671,
                    0,
                    "%s",
                    "( (diff>>31) & 0x01 ) == ( ( (clc->demoMemoryPoolEnd - clc->demoMemoryPoolBegin)>>31 ) & 0x01 )") )
    {
        __debugbreak();
    }
    return address;
}

void __cdecl CL_DemoMemoryPoolDeallocate(clientConnection_t *clc, int size)
{
    unsigned int sizea; // [esp+Ch] [ebp+Ch]

    sizea = (size + 3) & 0xFFFFFFFC;
    if ( (int)(sizea + clc->demoMemoryPoolBegin) > 0x40000 )
    {
        clc->demoMemoryPoolBegin = 0;
        Com_Printf(14, "The memory pool allocate Begin position was reset to 0\n");
    }
    clc->demoMemoryPoolBegin += sizea;
}

int __cdecl CL_GetClientArchiveCount(int start, int end)
{
    if ( end < start )
        return end - start + 256;
    else
        return end - start;
}

void __cdecl CL_WriteDemoMessage(int localClientNum, msg_t *msg, int headerBytes)
{
    clientConnection_t *LocalClientConnection; // eax
    unsigned int len; // [esp+0h] [ebp-10h]
    clientConnection_t *clc; // [esp+4h] [ebp-Ch]
    int swlen; // [esp+8h] [ebp-8h] BYREF
    unsigned __int8 networkPacketMarker; // [esp+Fh] [ebp-1h] BYREF

    LocalClientConnection = CL_GetLocalClientConnection(localClientNum);
    networkPacketMarker = 0;
    FS_WriteToDemo((char *)&networkPacketMarker, 1u, LocalClientConnection->demofile);
    clc = CL_GetLocalClientConnection(localClientNum);
    swlen = clc->serverMessageSequence;
    FS_WriteToDemo((char *)&swlen, 4u, clc->demofile);
    swlen = msg->cursize - headerBytes;
    len = swlen;
    if ( swlen < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp", 905, 0, "%s", "len >= 0") )
    {
        __debugbreak();
    }
    FS_WriteToDemo((char *)&swlen, 4u, clc->demofile);
    FS_WriteToDemo((char *)&msg->data[headerBytes], len, clc->demofile);
}

void __cdecl CL_WriteDemoMessageToMemory(int localClientNum, msg_t *msg, int headerBytes)
{
    demoContinuousStateBuffer_t *demoBuffer; // [esp+0h] [ebp-8h]
    clientConnection_t *clc; // [esp+4h] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    demoBuffer = &clc->demoContinuousStateBuffer[clc->demoContinuousStateBufferIndex];
    if ( clc->demoContinuousStateBuffer[clc->demoContinuousStateBufferIndex].msgData )
        CL_DemoMemoryPoolDeallocate(clc, clc->demoContinuousStateBuffer[clc->demoContinuousStateBufferIndex].msgLen);
    demoBuffer->msgLen = msg->cursize - headerBytes;
    demoBuffer->msgData = CL_DemoMemoryPoolAllocate(clc, demoBuffer->msgLen);
    demoBuffer->serverMessageSequence = clc->serverMessageSequence;
    memcpy(demoBuffer->msgData, &msg->data[headerBytes], demoBuffer->msgLen);
}

void __cdecl CL_VoiceTransmit(int localClientNum)
{
    voiceCommunication_t *vc; // [esp+0h] [ebp-4h]

    vc = CL_GetLocalClientVoiceCommunication(localClientNum);
    if ( (int)(Sys_Milliseconds() - vc->voicePacketLastTransmit) >= 200 || vc->voicePacketCount >= 10 )
    {
        if ( vc->voicePacketCount > 0 )
        {
            CL_WriteVoicePacket(localClientNum);
            vc->voicePacketCount = 0;
        }
        vc->voicePacketLastTransmit = Sys_Milliseconds();
    }
}

voiceCommunication_t *__cdecl CL_GetLocalClientVoiceCommunication(int localClientNum)
{
    if ( localClientNum
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\client_mp\\../client/client.h",
                    173,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum == 0)",
                    localClientNum) )
    {
        __debugbreak();
    }
    return cl_voiceCommunication;
}

int __cdecl Client_SendVoiceData(int bytes, char *enc_buffer)
{
    voiceCommunication_t *vc; // [esp+0h] [ebp-4h]

    if ( bytes > 0 )
    {
        vc = CL_GetLocalClientVoiceCommunication(0);
        memcpy(vc->voicePackets[vc->voicePacketCount].data, (unsigned __int8 *)enc_buffer, bytes);
        vc->voicePackets[vc->voicePacketCount++].dataSize = bytes;
        CL_VoiceTransmit(0);
    }
    return bytes;
}

char __cdecl Voice_SendVoiceData()
{
    voiceCommunication_t *vc; // [esp+0h] [ebp-4h]

    if ( !SV_VoiceEnabled() || !cl_voice->current.enabled || Dvar_GetInt("rate") < 5000 )
        return 0;
    if ( CL_GetLocalClientConnectionState(0) == 10 )
    {
        vc = CL_GetLocalClientVoiceCommunication(0);
        if ( cl_talking->current.enabled || IN_IsTalkKeyHeld() || vc->voicePacketCount )
            return 1;
    }
    return 0;
}

void __cdecl CL_RunOncePerClientFrame(int localClientNum, int msec)
{
    int ControllerIndex; // eax

    PROF_SCOPED("CL_RunOncePerClientFrame");

    CL_RunNetworkFrame(localClientNum);
    if ( UI_IsFullscreen(localClientNum) )
        CL_SyncGpu();
    IN_Frame();
    if ( cl_avidemo->current.integer && msec )
    {
        if ( CL_GetLocalClientConnectionState(0) == 10 || cl_forceavidemo->current.enabled )
        {
            ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
            Cmd_ExecuteSingleCommand(0, ControllerIndex, (char*)"screenshot silent\n");
        }
        msec = (int)(float)((float)(1000.0 / (float)cl_avidemo->current.integer) * com_timescaleValue);
        if ( !msec )
            msec = 1;
    }
    cls.realFrametime = msec;
    cls.frametime = msec;
    cls.realtime += msec;
    frame_msec = com_frameTime - old_com_frameTime;
    if ( com_frameTime == old_com_frameTime )
        frame_msec = 1;
    if ( frame_msec > 0xC8 )
        frame_msec = 200;
    old_com_frameTime = com_frameTime;
}

void __cdecl CL_Frame(int localClientNum, int msec)
{
    int UIContextIndex; // eax
    clientActive_t *LocalClientGlobals; // [esp+10h] [ebp-10h]
    const char *buf; // [esp+14h] [ebp-Ch]
    clientUIActive_t *clientUIActive; // [esp+18h] [ebp-8h]
    connstate_t connstate; // [esp+1Ch] [ebp-4h]

    clientUIActive = CL_GetLocalClientUIGlobals(localClientNum);
    connstate = CL_GetLocalClientConnectionState(localClientNum);
    if ( CG_IsShowingZombieMap() || Com_LocalClient_IsBeingUsed(localClientNum) )
    {
        if ( _InterlockedExchange(&cls.scriptError, 0) )
        {
            buf = Dvar_GetString("com_errorMessage");
            if ( strlen(buf) )
            {
                if ( I_stricmp(buf, ";") )
                {
                    UI_OpenMenu(localClientNum, "error_popmenu");
                    if ( !Key_IsCatcherActive(localClientNum, 16) )
                        Key_AddCatcher(localClientNum, 16);
                }
            }
        }
    }
    if ( CL_AllLocalClientsDisconnected() && (clientUIActive->keyCatchers & 0x10) == 0 )
        clientUIActive->keyCatchers |= 0x10u;
    Hunk_CheckTempMemoryClear();
    Hunk_CheckTempMemoryHighClear();
    if ( CL_LocalClient_IsCUIFlagSet(localClientNum, 2) )
    {
        CL_DevGuiFrame(localClientNum);
        UIContextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
        UI_ScreenshotUpdate(localClientNum, UIContextIndex);
        CL_VoiceFrame(localClientNum);
        CL_UpdateColor();
        CL_CheckUserinfo(localClientNum);
        CL_CheckForResend(localClientNum);
        CL_CheckTimeout(localClientNum);
        if ( DL_InProgress() )
            CL_WWWDownload();
        CL_UpdateInGameState(localClientNum);
        CL_SetCGameTime(localClientNum);
        CL_CreateCmdsDuringConnection(localClientNum);
        if ( connstate < CA_ACTIVE && connstate >= CA_CONNECTED )
            CL_SendCmd(localClientNum);
        if ( connstate == CA_ACTIVE )
        {
            LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
            CG_Flame_Age_All_Objects(LocalClientGlobals->serverTime);
            CG_GenerateFireSounds(localClientNum);
        }
        CG_UpdateDestructibles(localClientNum, msec);
    }
}

void __cdecl CL_CheckTimeout(int localClientNum)
{
    clientActive_t *v1; // [esp+0h] [ebp-Ch]
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-Ch]
    connstate_t connstate; // [esp+4h] [ebp-8h]
    clientConnection_t *clc; // [esp+8h] [ebp-4h]

    connstate = CL_GetLocalClientConnectionState(localClientNum);
    if ( connstate >= CA_CONNECTING )
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
        clc = CL_GetLocalClientConnection(localClientNum);
        if ( cl_paused->current.integer && sv_paused->current.integer
            || connstate < CA_PRIMED
            || clc->lastPacketTime <= 0
            || (float)(cls.realtime - clc->lastPacketTime) <= (float)(cl_timeout->current.value * 1000.0) )
        {
            if ( connstate <= CA_CONNECTING
                || connstate >= CA_PRIMED
                || clc->lastPacketTime <= 0
                || (float)(cls.realtime - clc->lastPacketTime) <= (float)(cl_connectTimeout->current.value * 1000.0) )
            {
                if ( connstate != CA_CONNECTING
                    || clc->lastPacketTime
                    || clc->connectPacketCount <= cl_connectionAttempts->current.integer )
                {
                    LocalClientGlobals->timeoutcount = 0;
                }
                else if ( ++LocalClientGlobals->timeoutcount > 5 )
                {
                    CL_ServerTimedOut(localClientNum);
                }
            }
            else if ( ++LocalClientGlobals->timeoutcount > 5 )
            {
                CL_ServerTimedOut(localClientNum);
            }
        }
        else if ( ++LocalClientGlobals->timeoutcount > 5 )
        {
            CL_ServerTimedOut(localClientNum);
        }
    }
    else
    {
        v1 = CL_GetLocalClientGlobals(localClientNum);
        if ( v1 )
            v1->timeoutcount = 0;
    }
}

void __cdecl CL_ServerTimedOut(int localClientNum)
{
    CL_GetLocalClientConnectionState(localClientNum);
    CL_CanNotConnect(localClientNum, "EXE_ERR_SERVER_TIMEOUT");
}

void __cdecl CL_CheckUserinfo(int localClientNum)
{
    char *v1; // eax
    const char *v2; // eax

    if ( CL_GetLocalClientConnectionState(localClientNum) >= 5
        && !cl_paused->current.integer
        && (dvar_modifiedFlags & 2) != 0 )
    {
        v1 = Dvar_InfoString(localClientNum, 2);
        v2 = va("userinfo \"%s\"", v1);
        CL_AddReliableCommand(localClientNum, v2);
    }
}

void __cdecl CL_UpdateInGameState(int localClientNum)
{
    int LocalClientConnectionState; // eax

    LocalClientConnectionState = CL_GetLocalClientConnectionState(localClientNum);
    Dvar_SetBoolIfChanged((dvar_s *)cl_ingame, LocalClientConnectionState == 10);
}

void __cdecl CL_VoiceFrame(int localClientNum)
{
    CL_GetLocalClientVoiceCommunication(localClientNum);
    Voice_GetLocalVoiceData();
    Voice_Playback();
}

char __cdecl CL_IsClientLocal(int clientNum)
{
    int client; // [esp+0h] [ebp-4h]

    for ( client = 0; client < cl_maxLocalClients; ++client )
    {
        if ( CL_GetLocalClientConnectionState(client) > 8 && CG_GetClientNum(client) == clientNum )
            return 1;
    }
    return 0;
}

void __cdecl CL_ParseBadPacket_f()
{
    msg_t msg; // [esp+0h] [ebp-38h] BYREF
    int fileSize; // [esp+30h] [ebp-8h]
    char *file; // [esp+34h] [ebp-4h] BYREF

    fileSize = FS_ReadFile("badpacket.dat", (void **)&file);
    if ( fileSize >= 0 )
    {
        memset(&msg, 0, sizeof(msg));
        msg.cursize = fileSize;
        msg.data = (unsigned __int8 *)file;
        MSG_ReadLong(&msg);
        MSG_ReadLong(&msg);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                        6169,
                        1,
                        "Time to debug this packet, baby!") )
            __debugbreak();
        CL_ParseServerMessage(0, &msg);
        FS_FreeFile(file);
    }
}

void __cdecl CL_ShutdownRef()
{
    R_SyncRenderThread();
    CL_ShutdownRenderer(1);
    track_shutdown(3);
    StatMon_Reset();
}

void __cdecl CL_ShutdownWorld()
{
    R_SyncRenderThread();
    CL_ShutdownRenderer(0);
    track_shutdown(3);
    StatMon_Reset();
}

void __cdecl CL_SetupViewport()
{
    int displayHeight; // [esp+4h] [ebp-10h]
    ScreenPlacement *scrPlace; // [esp+8h] [ebp-Ch]
    int localClientNum; // [esp+Ch] [ebp-8h]
    int displayWidth; // [esp+10h] [ebp-4h]

    displayWidth = cls.vidConfig.displayWidth;
    displayHeight = cls.vidConfig.displayHeight;
    if ( cls.rendererStarted )
    {
        ScrPlace_SetupUnsafeViewport(&scrPlaceFullUnsafe, 0, 0, cls.vidConfig.displayWidth, cls.vidConfig.displayHeight);
        ScrPlace_SetupViewport(&scrPlaceFull, 0, 0, displayWidth, displayHeight);
        for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
            ScrPlace_SetupViewport(&scrPlaceView[localClientNum], 0, 0, displayWidth, displayHeight);
        scrPlace = R_UI3D_ScrPlaceFromTextureWindow(0);
        ScrPlace_SetupUI3DForFullscreen(scrPlace, &scrPlaceFull);
    }
}

void __cdecl CL_InitRenderer()
{
    bool v0; // [esp+0h] [ebp-4h]

    if ( cls.rendererStarted
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                    6237,
                    0,
                    "%s",
                    "!cls.rendererStarted") )
    {
        __debugbreak();
    }
    cls.rendererStarted = 1;
    if ( G_OnlyConnectingPaths() )
        R_AllocateMinimalResources();
    if ( G_OnlyConnectingPaths() )
        Material_SetAlwaysUseDefaultMaterial(0);
    cls.vidConfig.isToolMode = 0;
    cls.vidConfig.isToolMode = r_reflectionProbeGenerate->current.enabled;
    v0 = cls.vidConfig.isToolMode || G_OnlyConnectingPaths();
    cls.vidConfig.isToolMode = v0;
    R_SetIsMultiplayer(1);
    R_BeginRegistration(&cls.vidConfig);
    CL_SetupViewport();
    cls.whiteMaterial = Material_RegisterHandle("white", 3);
    cls.consoleMaterial = Material_RegisterHandle("console", 3);
    cls.spinnerMaterial = Material_RegisterHandle("spinner_wedge", 3);
    cls.consoleFont = R_RegisterFont("fonts/consoleFont", 3);
    g_console_field_width = cls.vidConfig.displayWidth - 32;
    g_consoleField.widthInPixels = cls.vidConfig.displayWidth - 32;
    g_consoleField.charHeight = g_console_char_height;
    g_consoleField.fixedSize = 1;
    StatMon_Reset();
    Con_InitClientAssets();
    if ( G_OnlyConnectingPaths() )
        Material_SetAlwaysUseDefaultMaterial(1);
}

void __cdecl CL_ShutdownRenderer(int destroyWindow)
{
    if ( !cls.rendererStarted
        && !destroyWindow
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                    6312,
                    0,
                    "%s",
                    "cls.rendererStarted || destroyWindow") )
    {
        __debugbreak();
    }
    cls.rendererStarted = 0;
    Com_ShutdownWorld();
    if ( useFastFile->current.enabled && destroyWindow )
        CM_Shutdown();
    R_Shutdown(destroyWindow);
    cls.whiteMaterial = 0;
    cls.consoleMaterial = 0;
    cls.consoleFont = 0;
    cls.spinnerMaterial = 0;
    //BLOPS_NULLSUB();
}

void __cdecl CL_StartHunkUsers()
{
    if ( cls.hunkUsersStarted
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                    6400,
                    0,
                    "%s",
                    "!cls.hunkUsersStarted") )
    {
        __debugbreak();
    }
    if ( CL_AnyLocalClientsRunning() )
    {
        if ( !cls.soundStarted
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                        6405,
                        0,
                        "%s",
                        "cls.soundStarted") )
        {
            __debugbreak();
        }
        if ( !cls.rendererStarted
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                        6406,
                        0,
                        "%s",
                        "cls.rendererStarted") )
        {
            __debugbreak();
        }
        if ( !cls.uiStarted )
        {
            CL_InitUI();
            if ( !cls.uiStarted
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                            6412,
                            0,
                            "%s",
                            "cls.uiStarted") )
            {
                __debugbreak();
            }
        }
        if ( !cls.devGuiStarted )
        {
            cls.devGuiStarted = 1;
            CL_InitDevGui();
        }
        Com_DvarDump(6, 0);
        cls.hunkUsersStarted = 1;
    }
}

cmd_function_s CL_DevGuiDvar_f_VAR;
cmd_function_s CL_DevGuiCmd_f_VAR;
cmd_function_s CL_DevGuiOpen_f_VAR;
void CL_InitDevGui()
{
    DevGui_Init();
    Cmd_AddCommandInternal("devgui_dvar", CL_DevGuiDvar_f, &CL_DevGuiDvar_f_VAR);
    Cmd_AddCommandInternal("devgui_cmd", CL_DevGuiCmd_f, &CL_DevGuiCmd_f_VAR);
    Cmd_AddCommandInternal("devgui_open", CL_DevGuiOpen_f, &CL_DevGuiOpen_f_VAR);
    CL_CreateDevGui();
}

void __cdecl CL_DevGuiDvar_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const dvar_s *dvar; // [esp+28h] [ebp-4h]

    if ( Cmd_Argc() == 3 )
    {
        v1 = Cmd_Argv(2);
        dvar = Dvar_FindVar(v1);
        if ( dvar )
        {
            v3 = Cmd_Argv(1);
            DevGui_AddDvar(v3, dvar);
        }
        else
        {
            v2 = Cmd_Argv(2);
            Com_Printf(11, "dvar '%s' doesn't exist\n", v2);
        }
    }
    else
    {
        v0 = Cmd_Argv(0);
        Com_Printf(0, "USAGE: %s \"devgui path\" dvarName\n", v0);
    }
}

void __cdecl CL_DevGuiCmd_f()
{
    const char *v0; // eax
    const char *v1; // eax
    char *v2; // [esp-4h] [ebp-24h]

    if ( Cmd_Argc() == 3 )
    {
        v2 = (char *)Cmd_Argv(2);
        v1 = Cmd_Argv(1);
        DevGui_AddCommand(v1, v2);
    }
    else
    {
        v0 = Cmd_Argv(0);
        Com_Printf(0, "USAGE: %s \"devgui path\" \"command text\"\n", v0);
    }
}

void __cdecl CL_DevGuiOpen_f()
{
    const char *v0; // eax
    const char *v1; // eax

    if ( Cmd_Argc() == 2 )
    {
        v1 = Cmd_Argv(1);
        DevGui_OpenMenu(v1);
    }
    else
    {
        v0 = Cmd_Argv(0);
        Com_Printf(0, "USAGE: %s \"devgui path\"\n", v0);
    }
}

int __cdecl CL_ScaledMilliseconds()
{
    return cls.realtime;
}

void __cdecl CL_InitRef()
{
    GfxConfiguration config; // [esp+0h] [ebp-34h] BYREF

    Com_Printf(14, "----- Initializing Renderer ----\n");
    SetupGfxConfig(&config);
    R_ConfigureRenderer(&config);
    Dvar_SetInt((dvar_s *)cl_paused, 0);
}

void __cdecl SetupGfxConfig(GfxConfiguration *config)
{
    if ( !config
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp", 6465, 0, "%s", "config") )
    {
        __debugbreak();
    }
    config->maxClientViews = 1;
    config->entCount = 1536;
    config->entnumNone = 1023;
    config->entnumOrdinaryEnd = 1022;
    config->threadContextCount = 15;
    config->critSectCount = 75;
}

void __cdecl CL_InitDedicated()
{
    GfxConfiguration config; // [esp+0h] [ebp-34h] BYREF

    SetupGfxConfig(&config);
    R_MakeDedicated(&config);
    Sys_HideSplashWindow();
    Sys_ShowConsole();
    Sys_NormalExit();
}

void __cdecl CL_startSingleplayer_f()
{
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                    6565,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    Sys_QuitAndStartProcess("CoDSP_rd.exe");
}

void __cdecl CL_DrawLogo(int localClientNum)
{
    float fade; // [esp+40h] [ebp-24h]
    int time; // [esp+44h] [ebp-20h]
    float h1; // [esp+48h] [ebp-1Ch]
    float h0; // [esp+4Ch] [ebp-18h]
    float color[4]; // [esp+50h] [ebp-14h] BYREF
    float w; // [esp+60h] [ebp-4h]

    if ( CL_GetLocalClientConnectionState(localClientNum) != 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                    6714,
                    0,
                    "%s",
                    "CL_GetLocalClientConnectionState( localClientNum ) == CA_LOGO") )
    {
        __debugbreak();
    }
    time = cls.realtime - cls.logo.startTime;
    if ( cls.realtime - cls.logo.startTime >= cls.logo.fadein )
    {
        if ( time <= cls.logo.duration - cls.logo.fadeout )
            fade = 1.0f;
        else
            fade = (float)(cls.logo.duration - time) / (float)cls.logo.fadeout;
    }
    else
    {
        fade = (float)time / (float)cls.logo.fadein;
    }
    if ( fade >= 0.0 )
    {
        if ( fade > 1.0 )
            fade = 1.0f;
    }
    else
    {
        fade = 0.0f;
    }
    color[0] = fade;
    color[1] = fade;
    color[2] = fade;
    color[3] = 1.0f;
    w = (float)cls.vidConfig.displayWidth;
    h0 = ((double)cls.vidConfig.displayHeight + (double)cls.vidConfig.displayHeight) / 3.0;
    h1 = (double)cls.vidConfig.displayHeight - h0;
    R_AddCmdDrawStretchPic(0.0, 0.0, w, h0, 0.0, 0.0, 1.0, 1.0, color, cls.logo.material[0]);
    R_AddCmdDrawStretchPic(0.0, h0, w, h1, 0.0, 0.0, 1.0, 1.0, color, cls.logo.material[1]);
    if ( time > cls.logo.duration )
        CL_StopLogo(localClientNum);
}

void __cdecl CL_StopLogo(int localClientNum)
{
    clientUIActives[localClientNum].connectionState = CA_DISCONNECTED;
}

void __cdecl CL_StopLogoOrCinematic(int localClientNum)
{
    connstate_t clcState; // [esp+0h] [ebp-4h]

    clcState = CL_GetLocalClientConnectionState(localClientNum);
    if ( clcState == CA_CINEMATIC )
    {
        SCR_StopCinematic();
    }
    else
    {
        if ( clcState != CA_LOGO
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                        6814,
                        0,
                        "%s",
                        "clcState == CA_LOGO") )
        {
            __debugbreak();
        }
        CL_StopLogo(localClientNum);
    }
    SND_StopSounds(SND_STOP_ALL);
    if ( clcState )
        UI_SetActiveMenu(localClientNum, UIMENU_NONE);
    else
        UI_SetActiveMenu(localClientNum, UIMENU_MAIN);
}

void __cdecl CL_ToggleMenu_f()
{
    uiMenuCommand_t ActiveMenu; // [esp+0h] [ebp-18h]
    clientUIActive_t *LocalClientUIGlobals; // [esp+4h] [ebp-14h]
    connstate_t connstate; // [esp+8h] [ebp-10h]
    clientConnection_t *clc; // [esp+10h] [ebp-8h]

    LocalClientUIGlobals = CL_GetLocalClientUIGlobals(0);
    clc = CL_GetLocalClientConnection(0);
    connstate = CL_GetLocalClientConnectionState(0);
    if ( (LocalClientUIGlobals->keyCatchers & 0x10) != 0 )
        ActiveMenu = UI_GetActiveMenu(0);
    else
        ActiveMenu = UIMENU_NONE;
    if ( clc->demoplaying )
    {
LABEL_14:
        UI_SetActiveMenu(0, UIMENU_MAIN);
        return;
    }
    if ( connstate != CA_ACTIVE )
    {
        if ( !cl_waitingOnServerToLoadMap[0] )
            return;
        goto LABEL_14;
    }
    if ( (LocalClientUIGlobals->keyCatchers & 0x10) != 0 && ActiveMenu == UIMENU_INGAME )
    {
        UI_SetActiveMenu(0, UIMENU_NONE);
    }
    else if ( !LocalClientUIGlobals->keyCatchers && CL_ShouldAllowInGameMenu(0) )
    {
        UI_SetActiveMenu(0, UIMENU_INGAME);
    }
}

bool __cdecl CL_ShouldAllowInGameMenu(int localClientNum)
{
    if ( !CL_LocalClient_IsCUIFlagSet(localClientNum, 32) )
        return 1;
    if ( Demo_IsPlaying() )
        return 1;
    return (CG_GetLocalClientGlobals(localClientNum)->matchUIVisibilityFlags & 0x1000) == 0;
}

bool warned;
char __cdecl Playlist_ReadFromDisk()
{
    void *playlistBuffer; // [esp+0h] [ebp-4h] BYREF

    if ( FS_ReadFile("playlists_mp.info", &playlistBuffer) )
    {
        if ( playlistBuffer )
        {
            Playlist_ParsePlaylists((const char *)playlistBuffer);
            FS_FreeFile(playlistBuffer);
            return 1;
        }
        if ( !warned )
        {
            Com_PrintError(16, "Unable to find file 'playlists.info'\n");
            warned = 1;
        }
    }
    return 0;
}

void __cdecl CL_PauseGame(bool state)
{
    if ( state )
        Dvar_SetInt((dvar_s *)cl_paused, 1);
    else
        Dvar_SetInt((dvar_s *)cl_paused, 0);
}

cmd_function_s CL_ForwardToServer_f_VAR;
cmd_function_s CL_Configstrings_f_VAR;
cmd_function_s CL_Clientinfo_f_VAR;
cmd_function_s CL_Vid_Restart_f_VAR;
cmd_function_s CL_Vid_Restart_f_VAR_SERVER;
cmd_function_s CL_Snd_Restart_f_VAR;
cmd_function_s CL_Snd_Restart_f_VAR_SERVER;
cmd_function_s CL_Disconnect_f_VAR;
cmd_function_s CL_Disconnect_f_VAR_SERVER;
cmd_function_s CL_Record_f_VAR;
cmd_function_s CL_StopRecord_f_VAR;
cmd_function_s CL_PlayDemo_f_VAR_0;
cmd_function_s CL_PlayDemo_f_VAR_SERVER_0;
cmd_function_s CL_PlayDemo_f_VAR;
cmd_function_s CL_PlayDemo_f_VAR_SERVER;
cmd_function_s CL_ToggleDemoRecording_f_VAR;
cmd_function_s CL_AnimateUI_f_VAR;
cmd_function_s CL_TakeHiResShot_VAR;
cmd_function_s CL_TakeHiResShot2_VAR;
cmd_function_s CL_TakeHiResShot3_VAR;
cmd_function_s CL_TakeHiResShot4_VAR;
cmd_function_s CL_TakeHiResShot8_VAR;
cmd_function_s CL_PlayLogo_f_VAR;
cmd_function_s CL_Connect_f_VAR;
cmd_function_s CL_Connect_f_VAR_SERVER;
cmd_function_s CL_Reconnect_f_VAR;
cmd_function_s CL_Reconnect_f_VAR_SERVER;
cmd_function_s CL_FindServers_f_VAR;
cmd_function_s CL_Rcon_f_VAR;
cmd_function_s CL_ShowContextualItemUI_f_VAR;
cmd_function_s CL_Ping_f_VAR;
cmd_function_s CL_Ping_f_VAR_SERVER;
cmd_function_s CL_ServerStatus_f_VAR;
cmd_function_s CL_ServerStatus_f_VAR_SERVER;
cmd_function_s CL_Setenv_f_VAR;
cmd_function_s CL_ShowIP_f_VAR;
cmd_function_s CL_ToggleMenu_f_VAR;
cmd_function_s CL_OpenedIWDList_f_VAR;
cmd_function_s CL_ReferencedIWDList_f_VAR;
cmd_function_s CL_UpdateLevelHunkUsage_VAR;
cmd_function_s CL_startSingleplayer_f_VAR;
cmd_function_s CL_ParseBadPacket_f_VAR;
cmd_function_s CL_CubemapShot_f_VAR;
cmd_function_s CL_OpenScriptMenu_f_VAR;
cmd_function_s UI_SetClanName_f_VAR;
cmd_function_s UI_ListMenus_f_VAR;
cmd_function_s UI_Keyboard_New_f_VAR;
cmd_function_s UI_Clear_Keyboard_Active_f_VAR;
cmd_function_s UI_Keyboard_Complete_f_VAR;
cmd_function_s UI_UpdateListboxPos_f_VAR;
cmd_function_s UI_KeyClearStates_f_VAR;
cmd_function_s CL_SelectStringTableEntryInDvar_f_VAR;
cmd_function_s CL_ResetStats_f_VAR;
cmd_function_s CL_UploadStats_f_VAR;
cmd_function_s CL_SetupViewport_f_VAR;
cmd_function_s CL_Command_ProvisionallyDisableAllClients_VAR;
cmd_function_s CL_Command_DisableAllButPrimaryClients_VAR;
cmd_function_s CL_Command_DisableAllClients_VAR;
cmd_function_s CL_Command_SetClientBeingUsedAndPrimary_VAR;
cmd_function_s CL_Command_SetClientBeingUsedAndPrimaryAndActive_VAR;
cmd_function_s CL_Command_SetClientBeingUsed_VAR;
cmd_function_s CL_Command_SetClientBeingUsedAndActive_VAR;
cmd_function_s CL_Command_SetClientNotBeingUsed_VAR;
cmd_function_s CL_Command_SetClientPrimary_VAR;
cmd_function_s CL_Command_SignClientOutOfUI_VAR;
cmd_function_s Playlist_CmdReadFromDisk_VAR;
cmd_function_s Playlist_CmdSetCategoryFilter_VAR;
cmd_function_s Playlist_CmdSetPrevPlaylist_VAR;
cmd_function_s Playlist_CmdGetPrevPlaylist_VAR;
cmd_function_s CL_CmdGetOldCustomName_VAR;
cmd_function_s CL_CmdSetNewCustomName_VAR;

void __cdecl CL_InitOnceForAllClients()
{
    srand(Sys_MillisecondsRaw());
    Con_Init();
    CL_InitInput();
    cl_noprint = _Dvar_RegisterBool("cl_noprint", 0, 0, "Print nothing to the console");
    playlist = _Dvar_RegisterInt("playlist", 1, 0, 64, 4u, "The playlist number");
    category = _Dvar_RegisterInt("category", 0, 0, 0x7FFFFFFF, 0, "The category number");
    categoryPlaylist = _Dvar_RegisterInt("categoryPlaylist", 0, 0, 0x7FFFFFFF, 0, "The playlist index in the category");
    wagerCategory = _Dvar_RegisterInt("wagerCategory", 0, 0, 0x7FFFFFFF, 0, "The wager category number");
    wagerCategoryPlaylist = _Dvar_RegisterInt(
                                                        "wagerCategoryPlaylist",
                                                        0,
                                                        0,
                                                        0x7FFFFFFF,
                                                        0,
                                                        "The wager playlist index in the category");
    customGameMode = _Dvar_RegisterInt(
                                         "customGameMode",
                                         0,
                                         0,
                                         3,
                                         0,
                                         "0 for no custom game mode, 1 if built in, 2 if user created custom game mode.");
    for ( int i = 0; i < 10; ++i )
        customclass[i] = _Dvar_RegisterString(customClassDvars[i], (char *)"", 1u, "Custom class name");
    cl_hudDrawsBehindUI = _Dvar_RegisterBool("cl_hudDrawsBehindUI", 1, 0, "Should the HUD draw when the UI is up?");
    cl_voice = _Dvar_RegisterBool("cl_voice", 1, 3u, "Use voice communications");
    cl_timeout = _Dvar_RegisterFloat(
                                 "cl_timeout",
                                 40.0,
                                 0.0,
                                 3600.0,
                                 0,
                                 "Seconds with no received packets until a timeout occurs");
    cl_connectTimeout = _Dvar_RegisterFloat(
                                                "cl_connectTimeout",
                                                200.0,
                                                0.0,
                                                3600.0,
                                                0,
                                                "Timeout time in seconds while connecting to a server");
    cl_connectionAttempts = _Dvar_RegisterInt(
                                                        "cl_connectionAttempts",
                                                        10,
                                                        0,
                                                        0x7FFFFFFF,
                                                        0,
                                                        "Maximum number of connection attempts before aborting");
    cl_shownet = _Dvar_RegisterInt("cl_shownet", 0, -2, 4, 0, "Display network debugging information");
    cl_shownuments = _Dvar_RegisterBool("cl_shownuments", 0, 0, "Show the number of entities");
    cl_showServerCommands = _Dvar_RegisterBool(
                                                        "cl_showServerCommands",
                                                        0,
                                                        0,
                                                        "Enable debugging information for server commands");
    cl_showSend = _Dvar_RegisterBool("cl_showSend", 0, 0, "Enable debugging information for sent commands");
    cl_showTimeDelta = _Dvar_RegisterBool("cl_showTimeDelta", 0, 0, "Enable debugging information for time delta");
    cl_freezeDemo = _Dvar_RegisterBool(
                                        "cl_freezeDemo",
                                        0,
                                        0,
                                        "cl_freezeDemo is used to lock a demo in place for single frame advances");
    cl_activeAction = _Dvar_RegisterString(
                                            "activeAction",
                                            (char *)"",
                                            0,
                                            "Action to execute in first frame");
    cl_avidemo = _Dvar_RegisterInt("cl_avidemo", 0, 0, 0x7FFFFFFF, 0, "AVI demo frames per second");
    cl_forceavidemo = _Dvar_RegisterBool("cl_forceavidemo", 0, 0, "Record AVI demo even if client is not active");
    cl_clientDemoName = _Dvar_RegisterString(
                                                "cl_clientDemoName",
                                                (char *)"",
                                                0,
                                                "Current recording/playing demo name");
    cl_clientDemoUseMemoryBuffer = _Dvar_RegisterBool(
                                                                     "cl_clientDemoUseMemoryBuffer",
                                                                     0,
                                                                     0,
                                                                     "Used to toggle between memory buffer/ continuous file write mode");
    cl_clientDemoType = _Dvar_RegisterInt(
                                                "cl_clientDemoType",
                                                0,
                                                0,
                                                0x7FFFFFFF,
                                                0,
                                                "Client Demo Type where:\n"
                                                " 0:Button Press for start & stop\n"
                                                " 1:Button Press to start. Records till ui_clientDemoMaxRecordTime seconds\n"
                                                " 2: Cool Moment Capture Mode\n");
    cl_clientDemoMaxRecordTime = _Dvar_RegisterInt(
                                                                 "cl_clientDemoMaxRecordTime",
                                                                 10,
                                                                 0,
                                                                 0x7FFFFFFF,
                                                                 0,
                                                                 "Max record time for a client demo");
    cl_clientDemoRequestPingTime = _Dvar_RegisterInt(
                                                                     "cl_clientDemoRequestPingTime",
                                                                     6,
                                                                     0,
                                                                     0x7FFFFFFF,
                                                                     0,
                                                                     "The time (in seconds) between which we request our next uncompressed packet\n");
    cl_clientDemoCoolMomentExtendEnabled = _Dvar_RegisterBool(
                                                                                     "cl_clientDemoCoolMomentExtendEnabled",
                                                                                     0,
                                                                                     0,
                                                                                     "Used to toggle between extended cool capture mode and normal cool capture mode.");
    cl_clientDemoCoolMomentExtendTime = _Dvar_RegisterInt(
                                                                                "cl_clientDemoCoolMomentExtendTime",
                                                                                3,
                                                                                0,
                                                                                0x7FFFFFFF,
                                                                                0,
                                                                                "Max extended record time for the Cool Moment Capture system");
    cl_yawspeed = _Dvar_RegisterFloat(
                                    "cl_yawspeed",
                                    140.0,
                                    -3.4028235e38,
                                    3.4028235e38,
                                    1u,
                                    "Max yaw speed in degrees for game pad and keyboard");
    cl_pitchspeed = _Dvar_RegisterFloat(
                                        "cl_pitchspeed",
                                        140.0,
                                        -3.4028235e38,
                                        3.4028235e38,
                                        1u,
                                        "Max pitch speed in degrees for game pad");
    cl_anglespeedkey = _Dvar_RegisterFloat(
                                             "cl_anglespeedkey",
                                             1.5,
                                             0.0,
                                             3.4028235e38,
                                             0,
                                             "Multiplier for max angle speed for game pad and keyboard");
    cl_maxpackets = _Dvar_RegisterInt("cl_maxpackets", 30, 15, 100, 1u, "Maximum number of packets sent per frame");
    cl_packetdup = _Dvar_RegisterInt("cl_packetdup", 1, 0, 5, 1u, "Enable packet duplication");
    cl_sensitivity = _Dvar_RegisterFloat("sensitivity", 5.0, 0.0099999998, 100.0, 1u, "Mouse sensitivity");
    cl_mouseAccel = _Dvar_RegisterFloat("cl_mouseAccel", 0.0, 0.0, 100.0, 1u, "Mouse acceleration");
    cl_freelook = _Dvar_RegisterBool("cl_freelook", 1, 1u, "Enable looking with mouse");
    cl_showMouseRate = _Dvar_RegisterBool(
                                             "cl_showmouserate",
                                             0,
                                             0,
                                             "Print mouse rate debugging information to the console");
    cl_allowDownload = _Dvar_RegisterBool("cl_allowDownload", 1, 1u, "Allow client downloads from the server");
    cl_wwwDownload = _Dvar_RegisterBool("cl_wwwDownload", 1, 3u, "Download files via HTTP");
    cl_talking = _Dvar_RegisterBool("cl_talking", 0, 0, "Client is talking");
    cl_inGameVideo = _Dvar_RegisterBool("r_inGameVideo", 1, 1u, "Allow in game cinematics");
    cl_serverStatusResendTime = _Dvar_RegisterInt(
                                                                "cl_serverStatusResendTime",
                                                                750,
                                                                0,
                                                                3600,
                                                                0,
                                                                "Time in milliseconds to resend a server status message");
    cl_maxppf = _Dvar_RegisterInt("cl_maxppf", 5, 1, 10, 1u, "Maximum servers to ping per frame in server browser");
    cl_bypassMouseInput = _Dvar_RegisterBool(
                                                    "cl_bypassMouseInput",
                                                    0,
                                                    0,
                                                    "Bypass UI mouse input and send directly to the game");
    m_pitch = _Dvar_RegisterFloat("m_pitch", 0.022, -1.0, 1.0, 1u, "Default pitch");
    m_yaw = _Dvar_RegisterFloat("m_yaw", 0.022, -1.0, 1.0, 1u, "Default yaw");
    m_forward = _Dvar_RegisterFloat("m_forward", 0.25, -1.0, 1.0, 1u, "Forward speed in units per second");
    m_side = _Dvar_RegisterFloat("m_side", 0.25, -1.0, 1.0, 1u, "Sideways motion in units per second");
    m_filter = _Dvar_RegisterBool("m_filter", 0, 1u, "Allow mouse movement smoothing");
    ui_custom_newname = _Dvar_RegisterString(
                                                "ui_custom_newname",
                                                (char *)"",
                                                0,
                                                "New name of custom class entered by player.");
    cl_socketpool_enabled = _Dvar_RegisterBool("cl_socketpool_enabled", 0, 0, "True is socketpool technology is enabled");
    cl_socketpool_size = _Dvar_RegisterInt(
                                                 "cl_socketpool_size",
                                                 10,
                                                 0,
                                                 50,
                                                 0,
                                                 "Number of sockets to use for server browser socket pool");
    ui_clantag_new = _Dvar_RegisterString(
                                         "ui_clantag_new",
                                         (char *)"",
                                         0,
                                         "New name of clan tag entered by player.");
    cl_demoFFSpeed = _Dvar_RegisterInt("cl_demoFFSpeed", 0, 0, 0x7FFFFFFF, 0, "Timedemo playback speed");
    cl_demoSkipToTime = _Dvar_RegisterInt("cl_demoSkipToTime", 0, 0, 0x7FFFFFFF, 0, "Demo skip to servertime");
    cl_demoLiveStreaming = _Dvar_RegisterInt(
                                                     "cl_demoLiveStreaming",
                                                     0,
                                                     0,
                                                     2,
                                                     0,
                                                     "Saves current server time to a file for live streaming");
    input_invertPitch = _Dvar_RegisterBool("input_invertPitch", 0, 1u, "Invert gamepad pitch");
    input_viewSensitivity = _Dvar_RegisterFloat("input_viewSensitivity", 1.0, 0.000099999997, 5.0, 1u, "View Sensitivity");
    team_indicator = _Dvar_RegisterInt(
                                         "team_indicator",
                                         0,
                                         0,
                                         3,
                                         1u,
                                         "The team indicator modes : 0 - Full Name ; 1 - Abbreviated ; 2 - Icon Only");
    cl_playerRank = _Dvar_RegisterInt(
                                        "cl_playerRank",
                                        -1,
                                        -1,
                                        100,
                                        0,
                                        "To toggle the rank of the player which is displayed in his overhead name");
    cl_playerPrestige = _Dvar_RegisterInt(
                                                "cl_playerPrestige",
                                                -1,
                                                -1,
                                                100,
                                                0,
                                                "To toggle the prestige of the player which is displayed in his overhead name");
    cl_motdString = _Dvar_RegisterString("cl_motdString", (char *)"", 0x40u, "Message of the day");
    cl_ingame = _Dvar_RegisterBool("cl_ingame", 0, 0x40u, "True if the game is active");
    cl_wasconnected = _Dvar_RegisterBool(
                                            "cl_wasconnected",
                                            0,
                                            0x40u,
                                            "True if client was connected when we started connecting to a new server");
    _Dvar_RegisterInt("cl_maxPing", 800, 20, 2000, 1u, "Maximum ping for the client");
    cl_profileTextHeight = _Dvar_RegisterInt(
                                                     "cl_profileTextHeight",
                                                     19,
                                                     1,
                                                     100,
                                                     0,
                                                     "Text size to draw the network profile data");
    cl_profileTextY = _Dvar_RegisterInt("cl_profileTextY", 110, 0, 800, 0, "Y position to draw the profile data");
    name = _Dvar_RegisterString("name", "Unknown Soldier", 3u, "Player name");
    clanName = _Dvar_RegisterString("clanName", (char *)"", 3u, "Your clan abbreviation");
    _Dvar_RegisterInt("rate", 25000, 1000, 25000, 3u, "Player's preferred baud rate");
    _Dvar_RegisterInt("snaps", 20, 1, 30, 3u, "Snapshot rate");
    _Dvar_RegisterBool("cl_punkbuster", 0, 0x42u, "Determines whether PunkBuster is enabled");
    _Dvar_RegisterString("password", (char *)"", 2u, "password");
    nextdemo = _Dvar_RegisterString("nextdemo", (char *)"", 0, "The next demo to play");
    mortarStrikesLeft = _Dvar_RegisterInt("mortarStrikesLeft", 0, 0, 3, 0, "Mortar locations not positioned yet");
    _Dvar_RegisterBool("hud_enable", 1, 1u, "Enable the HUD display");
    _Dvar_RegisterBool("cg_blood", 1, 1u, "Show blood");
    _Dvar_RegisterBool("cg_allow_mature", g_allowMature, 0x40u, "Controls Mature Content selectability");
    if ( g_allowMature )
        _Dvar_RegisterBool("cg_mature", 1, 1u, "Show Mature Content");
    else
        _Dvar_RegisterBool("cg_mature", 0, 0x41u, "Show Mature Content");
    cl_serverchallenge = _Dvar_RegisterInt64((char*)"cl_serverchallenge", 0, 0, 0x7FFFFFFF, 0, "Challenge sent to server");
    cl_debugMessageKey = _Dvar_RegisterBool("cl_debugMessageKey", 0, 0, "Enable message key debugging information");
    motd = _Dvar_RegisterString("motd", (char *)"", 0, "Message of the day");
    Cmd_AddCommandInternal("cmd", CL_ForwardToServer_f, &CL_ForwardToServer_f_VAR);
    Cmd_AddCommandInternal("configstrings", CL_Configstrings_f, &CL_Configstrings_f_VAR);
    Cmd_AddCommandInternal("clientinfo", CL_Clientinfo_f, &CL_Clientinfo_f_VAR);
    Cmd_AddCommandInternal("vid_restart", Cbuf_AddServerText_f, &CL_Vid_Restart_f_VAR);
    Cmd_AddServerCommandInternal("vid_restart", CL_Vid_Restart_f, &CL_Vid_Restart_f_VAR_SERVER);
    Cmd_AddCommandInternal("snd_restart", Cbuf_AddServerText_f, &CL_Snd_Restart_f_VAR);
    Cmd_AddServerCommandInternal("snd_restart", CL_Snd_Restart_f, &CL_Snd_Restart_f_VAR_SERVER);
    Cmd_AddCommandInternal("disconnect", Cbuf_AddServerText_f, &CL_Disconnect_f_VAR);
    Cmd_AddServerCommandInternal("disconnect", CL_Disconnect_f, &CL_Disconnect_f_VAR_SERVER);
    Cmd_AddCommandInternal("record", CL_Record_f, &CL_Record_f_VAR);
    Cmd_AddCommandInternal("stoprecord", CL_StopRecord_f, &CL_StopRecord_f_VAR);
    Cmd_AddCommandInternal("demo", Cbuf_AddServerText_f, &CL_PlayDemo_f_VAR_0);
    Cmd_AddServerCommandInternal("demo", CL_PlayDemo_f, &CL_PlayDemo_f_VAR_SERVER_0);
    Cmd_AddCommandInternal("timedemo", Cbuf_AddServerText_f, &CL_PlayDemo_f_VAR);
    Cmd_AddServerCommandInternal("timedemo", CL_PlayDemo_f, &CL_PlayDemo_f_VAR_SERVER);
    Cmd_SetAutoComplete("demo", "demos", "dm_7");
    Cmd_SetAutoComplete("timedemo", "demos", "dm_7");
    Cmd_AddCommandInternal("toggledemorecording", CL_ToggleDemoRecording_f, &CL_ToggleDemoRecording_f_VAR);
    Cmd_AddCommandInternal("ui_animate", CL_AnimateUI_f, &CL_AnimateUI_f_VAR);
    Cmd_AddCommandInternal("hiResScreenshot", (void (__cdecl *)())CL_TakeHiResShot, &CL_TakeHiResShot_VAR);
    Cmd_AddCommandInternal("hiResScreenshot2", (void (__cdecl *)())CL_TakeHiResShot2, &CL_TakeHiResShot2_VAR);
    Cmd_AddCommandInternal("hiResScreenshot3", (void (__cdecl *)())CL_TakeHiResShot3, &CL_TakeHiResShot3_VAR);
    Cmd_AddCommandInternal("hiResScreenshot4", (void (__cdecl *)())CL_TakeHiResShot4, &CL_TakeHiResShot4_VAR);
    Cmd_AddCommandInternal("hiResScreenshot8", (void (__cdecl *)())CL_TakeHiResShot8, &CL_TakeHiResShot8_VAR);
    Cmd_AddCommandInternal("logo", CL_PlayLogo_f, &CL_PlayLogo_f_VAR);
    Cmd_AddCommandInternal("connect", Cbuf_AddServerText_f, &CL_Connect_f_VAR);
    Cmd_AddServerCommandInternal("connect", CL_Connect_f, &CL_Connect_f_VAR_SERVER);
    Cmd_AddCommandInternal("reconnect", Cbuf_AddServerText_f, &CL_Reconnect_f_VAR);
    Cmd_AddServerCommandInternal("reconnect", CL_Reconnect_f, &CL_Reconnect_f_VAR_SERVER);
    Cmd_AddCommandInternal("findservers", CL_FindServers_f, &CL_FindServers_f_VAR);
    CL_RconInit();
    Cmd_AddCommandInternal("rcon", CL_Rcon_f, &CL_Rcon_f_VAR);
    Cmd_AddCommandInternal("showContextItem", CL_ShowContextualItemUI_f, &CL_ShowContextualItemUI_f_VAR);
    Cmd_AddCommandInternal("ping", Cbuf_AddServerText_f, &CL_Ping_f_VAR);
    Cmd_AddServerCommandInternal("ping", BLOPS_NULLSUB, &CL_Ping_f_VAR_SERVER);
    Cmd_AddCommandInternal("serverstatus", Cbuf_AddServerText_f, &CL_ServerStatus_f_VAR);
    Cmd_AddServerCommandInternal("serverstatus", BLOPS_NULLSUB, &CL_ServerStatus_f_VAR_SERVER);
    Cmd_AddCommandInternal("setenv", CL_Setenv_f, &CL_Setenv_f_VAR);
    Cmd_AddCommandInternal("showip", CL_ShowIP_f, &CL_ShowIP_f_VAR);
    Cmd_AddCommandInternal("toggleMenu", CL_ToggleMenu_f, &CL_ToggleMenu_f_VAR);
    Cmd_AddCommandInternal("fs_openedList", CL_OpenedIWDList_f, &CL_OpenedIWDList_f_VAR);
    Cmd_AddCommandInternal("fs_referencedList", CL_ReferencedIWDList_f, &CL_ReferencedIWDList_f_VAR);
    Cmd_AddCommandInternal("updatehunkusage", CL_UpdateLevelHunkUsage, &CL_UpdateLevelHunkUsage_VAR);
    Cmd_AddCommandInternal("startSingleplayer", CL_startSingleplayer_f, &CL_startSingleplayer_f_VAR);
    Cmd_AddCommandInternal("parseBadPacket", CL_ParseBadPacket_f, &CL_ParseBadPacket_f_VAR);
    Cmd_AddCommandInternal("cubemapShot", (void (__cdecl *)())CL_CubemapShot_f, &CL_CubemapShot_f_VAR);
    Cmd_AddCommandInternal("openScriptMenu", CL_OpenScriptMenu_f, &CL_OpenScriptMenu_f_VAR);
    Cmd_AddCommandInternal("setclanname", UI_SetClanName_f, &UI_SetClanName_f_VAR);
    Cmd_AddCommandInternal("listmenus", UI_ListMenus_f, &UI_ListMenus_f_VAR);
    Cmd_AddCommandInternal("ui_keyboard_new", UI_Keyboard_New_f, &UI_Keyboard_New_f_VAR);
    Cmd_AddCommandInternal("ui_clear_keyboard_active", UI_Clear_Keyboard_Active_f, &UI_Clear_Keyboard_Active_f_VAR);
    Cmd_AddCommandInternal("ui_keyboard_complete", UI_Keyboard_Complete_f, &UI_Keyboard_Complete_f_VAR);
    Cmd_AddCommandInternal("updateListboxPos", UI_UpdateListboxPos_f, &UI_UpdateListboxPos_f_VAR);
    Cmd_AddCommandInternal("clearKeyStates", UI_KeyClearStates_f, &UI_KeyClearStates_f_VAR);
    Cmd_AddCommandInternal(
        "selectStringTableEntryInDvar",
        CL_SelectStringTableEntryInDvar_f,
        &CL_SelectStringTableEntryInDvar_f_VAR);
    Cmd_AddCommandInternal("resetStats", CL_ResetStats_f, &CL_ResetStats_f_VAR);
    Cmd_AddCommandInternal("uploadStats", LiveStorage_UploadStatsForController, &CL_UploadStats_f_VAR);
    UI_EmblemRegisterCmds();
    UI_RegisterCmds();
    Cmd_AddCommandInternal("setupviewport", CL_SetupViewport_f, &CL_SetupViewport_f_VAR);
    CL_RegisterDvars();
    CL_RegisterCommands();
    Cmd_AddCommandInternal(
        "provisionallydisableallclients",
        CL_Command_ProvisionallyDisableAllClients,
        &CL_Command_ProvisionallyDisableAllClients_VAR);
    Cmd_AddCommandInternal(
        "disableallbutprimaryclients",
        CL_Command_DisableAllButPrimaryClients,
        &CL_Command_DisableAllButPrimaryClients_VAR);
    Cmd_AddCommandInternal("disableallclients", CL_Command_DisableAllClients, &CL_Command_DisableAllClients_VAR);
    Cmd_AddCommandInternal(
        "setclientbeingusedandprimary",
        CL_Command_SetClientBeingUsedAndPrimary,
        &CL_Command_SetClientBeingUsedAndPrimary_VAR);
    Cmd_AddCommandInternal(
        "setclientbeingusedandprimaryandactive",
        CL_Command_SetClientBeingUsedAndPrimaryAndActive,
        &CL_Command_SetClientBeingUsedAndPrimaryAndActive_VAR);
    Cmd_AddCommandInternal("setclientbeingused", CL_Command_SetClientBeingUsed, &CL_Command_SetClientBeingUsed_VAR);
    Cmd_AddCommandInternal(
        "setclientbeingusedandactive",
        CL_Command_SetClientBeingUsedAndActive,
        &CL_Command_SetClientBeingUsedAndActive_VAR);
    Cmd_AddCommandInternal(
        "setclientnotbeingused",
        CL_Command_SetClientNotBeingUsed,
        &CL_Command_SetClientNotBeingUsed_VAR);
    Cmd_AddCommandInternal("setclientprimary", CL_Command_SetClientPrimary, &CL_Command_SetClientPrimary_VAR);
    Cmd_AddCommandInternal("signclientout", BLOPS_NULLSUB, &CL_Command_SignClientOutOfUI_VAR);
    Cmd_AddCommandInternal("xplaylistreadfromdisk", Playlist_CmdReadFromDisk, &Playlist_CmdReadFromDisk_VAR);
    Cmd_AddCommandInternal("SetCategoryFilter", Playlist_CmdSetCategoryFilter, &Playlist_CmdSetCategoryFilter_VAR);
    Cmd_AddCommandInternal("SetPrevPlaylist", Playlist_CmdSetPrevPlaylist, &Playlist_CmdSetPrevPlaylist_VAR);
    Cmd_AddCommandInternal("GetPrevPlaylist", Playlist_CmdGetPrevPlaylist, &Playlist_CmdGetPrevPlaylist_VAR);
    Cmd_AddCommandInternal("getOldCustomName", CL_CmdGetOldCustomName, &CL_CmdGetOldCustomName_VAR);
    Cmd_AddCommandInternal("setNewCustomName", CL_CmdSetNewCustomName, &CL_CmdSetNewCustomName_VAR);
    CL_InitRef();
    SCR_Init();
    CG_RegisterDvars();
    Flame_Init();
    Phys_Init();
    Ragdoll_Register();
    UI_Gametype_Custom_Init();
    CL_QuickMatch_Init();
    CL_GetLocalClientVoiceCommunication(0)->voicePacketCount = 0;
    Scr_InitVariables(SCRIPTINSTANCE_CLIENT);
    Scr_Init(SCRIPTINSTANCE_CLIENT);
    CG_ResetClientInitializationState();
}

void __cdecl CL_PlayDemo_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    int v4; // eax
    const char *v5; // eax
    char extension[32]; // [esp+58h] [ebp-134h] BYREF
    int localClientNum; // [esp+78h] [ebp-114h]
    char name[256]; // [esp+7Ch] [ebp-110h] BYREF
    clientConnection_t *clc; // [esp+180h] [ebp-Ch]
    const char *arg; // [esp+184h] [ebp-8h]
    int demofile; // [esp+188h] [ebp-4h] BYREF

    if ( Cmd_Argc() == 2 )
    {
        if ( com_sv_running->current.enabled )
        {
            Com_Printf(14, "listen server cannot play a demo.\n");
        }
        else
        {
            localClientNum = 0;
            CL_Disconnect(0, 1);
            arg = Cmd_Argv(1);
            Com_sprintf(extension, 0x20u, ".dm_%d", 7);
            if ( I_stricmp(&arg[strlen(arg) - (&extension[strlen(extension) + 1] - &extension[1])], extension) )
                Com_sprintf(name, 0x100u, "%s.dm_%d", arg, 7);
            else
                Com_sprintf(name, 0x100u, "%s", arg);
            FS_SV_FOpenFileRead(name, (char*)"demos", &demofile);
            if ( !demofile )
            {
                v1 = va("EXE_ERR_NOT_FOUND", name);
                Com_Error(ERR_DROP, v1);
            }
            CL_AllocatePerLocalClientMemory();
            CL_GetLocalClientConnectionState(localClientNum);
            clc = CL_GetLocalClientConnection(localClientNum);
            v2 = Cmd_Argv(1);
            I_strncpyz(clc->demoName, v2, 64);
            Con_Close(localClientNum);
            //BLOPS_NULLSUB();
            clientUIActives[localClientNum].connectionState = CA_CONNECTED;
            clc->demofile = demofile;
            clc->demoplaying = 1;
            v3 = Cmd_Argv(0);
            v4 = I_stricmp(v3, "timedemo");
            clc->isTimeDemo = v4 == 0;
            clc->lastClientArchiveIndex = 0;
            v5 = Cmd_Argv(1);
            I_strncpyz(cls.servername, v5, 256);
            Cbuf_ExecuteBuffer(0, 0, (char*)"selectStringTableEntryInDvar mp/didyouknow.csv 0 didyouknow");
            if ( useFastFile->current.enabled )
                DB_ResetZoneSize(0);
            while (clientUIActives[localClientNum].connectionState >= CA_CONNECTED
                && clientUIActives[localClientNum].connectionState < CA_PRIMED)
                CL_ReadDemoMessage(localClientNum);
            clc->firstDemoFrameSkipped = 0;
            clc->demoPrevServerTime = 0;
            Dvar_SetIntByName("cl_demoFFSpeed", 0);
        }
    }
    else
    {
        v0 = Cmd_Argv(0);
        Com_Printf(14, "%s <demoname>\n", v0);
    }
}

void __cdecl CL_Disconnect_f()
{
    int localClientNum; // [esp+0h] [ebp-4h]

    for ( localClientNum = 0; localClientNum < cl_maxLocalClients; ++localClientNum )
    {
        if ( CL_GetLocalClientConnectionState(localClientNum) )
            CL_DisconnectLocalClient(localClientNum);
    }
}

void __cdecl CL_SetupViewport_f()
{
    CL_SetupViewport();
}

void __cdecl CL_PlayLogo_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    float v5; // [esp+0h] [ebp-70h]
    float v6; // [esp+4h] [ebp-6Ch]
    float v7; // [esp+8h] [ebp-68h]
    float v8; // [esp+Ch] [ebp-64h]
    float v9; // [esp+24h] [ebp-4Ch]
    float v10; // [esp+3Ch] [ebp-34h]
    connstate_t connstate; // [esp+64h] [ebp-Ch]
    const char *name; // [esp+6Ch] [ebp-4h]

    if ( Cmd_Argc() != 5 )
    {
        Com_Printf(0, "USAGE: logo <image name> <fadein seconds> <full duration seconds> <fadeout seconds>\n");
        return;
    }
    Com_DPrintf(0, "CL_PlayLogo_f\n");
    connstate = CL_GetLocalClientConnectionState(0);
    if ( connstate == CA_CINEMATIC )
    {
        SCR_StopCinematic();
    }
    else if ( connstate == CA_LOGO )
    {
        CL_StopLogo(0);
    }
    else if ( connstate )
    {
        return;
    }
    CL_SetLocalClientConnectionState(0, CA_LOGO);
    if ( cls.uiStarted )
        UI_SetActiveMenu(0, UIMENU_NONE);
    SND_StopSounds(SND_STOP_ALL);
    SND_FadeIn();
    name = Cmd_Argv(1);
    v0 = Cmd_Argv(2);
    v7 = atof(v0);
    v10 = v7 * 1000.0;
    cls.logo.fadein = (int)(v10 + 9.313225746154785e-10);
    v1 = Cmd_Argv(3);
    v6 = atof(v1);
    v9 = v6 * 1000.0;
    cls.logo.duration = (int)(v9 + 9.313225746154785e-10);
    v2 = Cmd_Argv(4);
    v5 = atof(v2);
    v8 = v5 * 1000.0;
    cls.logo.fadeout = (int)(v8 + 9.313225746154785e-10);
    cls.logo.duration += cls.logo.fadeout + cls.logo.fadein;
    v3 = va("%s1", name);
    cls.logo.material[0] = Material_RegisterHandle(v3, 3);
    v4 = va("%s2", name);
    cls.logo.material[1] = Material_RegisterHandle(v4, 3);
    cls.logo.startTime = cls.realtime + 100;
}

void __cdecl CL_OpenScriptMenu_f()
{
    const char *v0; // eax
    char *menuName; // [esp+18h] [ebp-10h]
    const char *menuResponse; // [esp+1Ch] [ebp-Ch]
    int menuIndex; // [esp+20h] [ebp-8h]
    const char *parentMenuName; // [esp+24h] [ebp-4h]

    if ( Cmd_Argc() == 3 )
    {
        if ( UI_AllowScriptMenuResponse(0) )
        {
            if ( cls.uiStarted )
            {
                parentMenuName = Cmd_Argv(1);
                menuResponse = Cmd_Argv(2);
                if ( parentMenuName )
                {
                    if ( menuResponse )
                    {
                        for ( menuIndex = 0; menuIndex < 32; ++menuIndex )
                        {
                            menuName = CL_GetConfigString(menuIndex + 2548);
                            if ( *menuName )
                            {
                                if ( !I_stricmp(parentMenuName, menuName) )
                                    break;
                            }
                        }
                        if ( menuIndex == 32 )
                            menuIndex = -1;
                        v0 = va("cmd mr %i %i %s\n", cls.serverId, menuIndex, menuResponse);
                        Cbuf_AddText(0, v0);
                    }
                }
            }
        }
    }
    else
    {
        Com_Printf(0, "USAGE: openscriptmenu <parent menu name> <script menu response>\n");
        Com_Printf(0, "EXAMPLE: openscriptmenu ingame changeweapon\n");
    }
}

void __cdecl Playlist_CmdReadFromDisk()
{
    Playlist_ReadFromDisk();
}

void __cdecl Playlist_CmdSetCategoryFilter()
{
    const char *v0; // eax
    int i; // [esp+10h] [ebp-8h]
    int argc; // [esp+14h] [ebp-4h]

    Playlist_ResetCategoryFilter();
    argc = Cmd_Argc();
    for ( i = 1; i < argc; ++i )
    {
        v0 = Cmd_Argv(i);
        Playlist_EnableCategoryInFilter(v0);
    }
}

void __cdecl Playlist_CmdSetPrevPlaylist()
{
    Playlist_SetPrevPlaylist();
}

void __cdecl Playlist_CmdGetPrevPlaylist()
{
    Playlist_GetPrevPlaylist();
}

void __cdecl CL_CmdGetOldCustomName()
{
    const char *String; // eax
    const char *v1; // eax

    String = Dvar_GetString("ui_custom_name");
    v1 = Dvar_GetString(String);
    Dvar_SetString((dvar_s *)ui_custom_newname, v1);
}

void __cdecl CL_CmdSetNewCustomName()
{
    const char *v0; // eax
    char *String; // [esp-4h] [ebp-4h]

    String = (char *)Dvar_GetString("ui_custom_newname");
    v0 = Dvar_GetString("ui_custom_name");
    Dvar_SetStringByName(v0, String);
    Dvar_SetString((dvar_s *)ui_custom_newname, "");
}

void __cdecl CL_ResetStats_f()
{
    PCache_NukeProfile(0);
    LiveStorage_DeleteGlobalStats(0);
}

void __cdecl CL_Init(int localClientNum)
{
    Com_Printf(14, "----- Client Initialization -----\n");

    CL_ClearState(localClientNum);

    //iassert (CountBitsEnabled( 0xffffffff ) == 32)
    //iassert (CountBitsEnabled( 0x00000000 ) == 0)
    //iassert (CountBitsEnabled( 0x11111111 ) == 8)
    //iassert (CountBitsEnabled( 0x77777777 ) == 24)

    CL_ClearMutedList();

    CL_SetLocalClientConnectionState(localClientNum, CA_DISCONNECTED);

    cls.realtime = 0;

    Com_LocalClient_SetBeingUsed(localClientNum, 1);

    cl_serverLoadingMap = 0;
    cl_waitingOnServerToLoadMap[localClientNum] = 0;
    g_waitingForServer = 0;

    FS_DisablePureCheck(0);
    Cbuf_Execute(localClientNum, Com_LocalClient_GetControllerIndex(localClientNum));
    CL_LocalClient_SetCUIFlag(localClientNum, 2);
    Com_Printf(14, "----- Client Initialization Complete -----\n");
}

int __cdecl CountBitsEnabled(unsigned int num)
{
    unsigned int numa; // [esp+1Ch] [ebp+8h]
    unsigned int numb; // [esp+1Ch] [ebp+8h]

    numa = (((((num >> 1) & 0x55555555) + (num & 0x55555555)) >> 2) & 0x33333333)
        + ((((num >> 1) & 0x55555555) + (num & 0x55555555)) & 0x33333333);
    numb = (((((numa >> 4) & 0xF0F0F0F) + (numa & 0xF0F0F0F)) >> 8) & 0xFF00FF)
        + ((((numa >> 4) & 0xF0F0F0F) + (numa & 0xF0F0F0F)) & 0xFF00FF);
    return HIWORD(numb) + (unsigned __int16)numb;
}

int recursive;
void __cdecl CL_Shutdown(int localClientNum)
{
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                    7747,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    Com_SyncThreads();
    Com_Printf(14, "----- CL_Shutdown -----\n");
    if ( recursive )
    {
        Com_Printf(14, "recursive shutdown\n");
    }
    else
    {
        recursive = 1;
        CL_Disconnect(localClientNum, 1);
        if ( CL_AllLocalClientsDisconnected() )
        {
            CL_ShutdownDebugData();
            CL_ShutdownHunkUsers();
            Voice_Shutdown();
            SND_Shutdown();
            CL_ShutdownInput();
            Cmd_RemoveCommand("cmd");
            Cmd_RemoveCommand("configstrings");
            Cmd_RemoveCommand("clientinfo");
            Cmd_RemoveCommand("vid_restart");
            Cmd_RemoveCommand("snd_restart");
            Cmd_RemoveCommand("disconnect");
            Cmd_RemoveCommand("record");
            Cmd_RemoveCommand("demo");
            Cmd_RemoveCommand("cinematic");
            Cmd_RemoveCommand("logo");
            Cmd_RemoveCommand("stoprecord");
            Cmd_RemoveCommand("connect");
            Cmd_RemoveCommand("reconnect");
            Cmd_RemoveCommand("localservers");
            Cmd_RemoveCommand("globalservers");
            Cmd_RemoveCommand("showContextItem");
            Cmd_RemoveCommand("rcon");
            Cmd_RemoveCommand("setenv");
            Cmd_RemoveCommand("ping");
            Cmd_RemoveCommand("serverstatus");
            Cmd_RemoveCommand("showip");
            Cmd_RemoveCommand("fs_openedList");
            Cmd_RemoveCommand("fs_referencedList");
            Cmd_RemoveCommand("updatehunkusage");
            Cmd_RemoveCommand("SaveTranslations");
            Cmd_RemoveCommand("SaveNewTranslations");
            Cmd_RemoveCommand("LoadTranslations");
            Cmd_RemoveCommand("startSingleplayer");
            Cmd_RemoveCommand("buyNow");
            Cmd_RemoveCommand("singlePlayLink");
            Cmd_RemoveCommand("setRecommended");
            Cmd_RemoveCommand("cubemapShot");
            Cmd_RemoveCommand("openScriptMenu");
            Cmd_RemoveCommand("openmenu");
            Cmd_RemoveCommand("closemenu");
            memset(&cls, 0, sizeof(cls));
        }
        CL_LocalClient_ClearCUIFlag(localClientNum, 2);
        recursive = 0;
        Com_Printf(14, "-----------------------\n");
    }
}

bool __cdecl CL_GetServerList(int source, serverInfo_t **servers, int **count)
{
    bool retval; // [esp+7h] [ebp-1h]

    if ( !servers
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp", 7967, 0, "%s", "servers") )
    {
        __debugbreak();
    }
    if ( !count
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp", 7968, 0, "%s", "count") )
    {
        __debugbreak();
    }
    retval = 0;
    switch ( source )
    {
        case 0:
            *servers = cls.unrankedServers;
            *count = &cls.numunrankedservers;
            retval = 1;
            break;
        case 1:
            *servers = cls.rankedServers;
            *count = &cls.numrankedservers;
            retval = 1;
            break;
        case 2:
            *servers = cls.wagerServers;
            *count = &cls.numwagerservers;
            retval = 1;
            break;
        case 3:
            *servers = cls.favoriteServers;
            *count = &cls.numfavoriteservers;
            retval = 1;
            break;
        case 4:
            *servers = cls.friendsServers;
            *count = &cls.numfriendsservers;
            retval = 1;
            break;
        case 5:
            *servers = cls.recentServers;
            *count = &cls.numrecentservers;
            retval = 1;
            break;
        default:
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                            8008,
                            0,
                            "unknown server type %i\n",
                            source) )
                __debugbreak();
            break;
    }
    return retval;
}

int __cdecl CL_UpdateDirtyPings(int localClientNum, unsigned int source)
{
    int firstPingedIndex; // [esp+0h] [ebp-1Ch]
    int status; // [esp+4h] [ebp-18h]
    unsigned int slot; // [esp+8h] [ebp-14h]
    int latestPingedTime; // [esp+Ch] [ebp-10h]
    int sentCount; // [esp+10h] [ebp-Ch]
    serverInfo_t *servers; // [esp+14h] [ebp-8h] BYREF
    int *count; // [esp+18h] [ebp-4h] BYREF

    status = 0;
    if ( CL_GetLocalClientConnectionState(localClientNum) )
        return 0;
    if ( source >= 6 )
        return 0;
    cls.pingUpdateSource = source;
    CL_GetServerList(source, &servers, &count);
    if ( !*count )
        return 0;
    if ( cls.lastServerPinged >= *count )
        cls.lastServerPinged = 0;
    firstPingedIndex = cls.lastServerPinged;
    if ( cls.lastServerPinged >= (unsigned int)count
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                    8050,
                    0,
                    "cls.lastServerPinged doesn't index count\n\t%i not in [0, %i)",
                    cls.lastServerPinged,
                    count) )
    {
        __debugbreak();
    }
    sentCount = 0;
    latestPingedTime = 0;
    do
    {
        slot = cls.lastServerPinged++;
        if ( cls.lastServerPinged >= *count )
            cls.lastServerPinged = 0;
        if ( slot >= (unsigned int)count
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                        8062,
                        0,
                        "slot doesn't index count\n\t%i not in [0, %i)",
                        slot,
                        count) )
        {
            __debugbreak();
        }
        if ( servers[slot].dirty && servers[slot].ping == -1 )
        {
            if ( !servers[slot].pingedTime )
            {
                servers[slot].pingedTime = Sys_Milliseconds();
                Com_DPrintf(14, "Pinging server %d\n", slot);
                CL_RawPingServer(&servers[slot], 2u);
                ++sentCount;
                ++cls.pingedServerCount;
            }
            if ( servers[slot].pingedTime > latestPingedTime )
                latestPingedTime = servers[slot].pingedTime;
        }
    }
    while ( sentCount < cl_maxppf->current.integer && cls.lastServerPinged != firstPingedIndex );
    if ( sentCount || (int)Sys_Milliseconds() < latestPingedTime + 2000 
#ifdef KISAK_DW
        || dwIsPagedFindInProgress() 
#endif
        )
        return 1;
    return status;
}

void __cdecl CL_ShowIP_f()
{
#if 0 // LWSS: disabled for now due to using the trash bd lib
    unsigned __int16 Port; // ax
    bdAddr *v1; // [esp+Ch] [ebp-18h]
    int v2; // [esp+14h] [ebp-10h]
    bdReference<bdCommonAddr> v3; // [esp+1Ch] [ebp-8h] BYREF
    bdSocketRouter *socketRouter; // [esp+20h] [ebp-4h]

    Sys_ShowIP();
    socketRouter = dwGetSocketRouter();
    v2 = *(unsigned int *)bdSocketRouter::getLocalCommonAddr(socketRouter, (int)&v3);
    v1 = *(bdAddr **)bdCommonAddr::getLocalAddrs(v2);
    Port = bdAddr::getPort(v1);
    Com_Printf(0, "Port is %u\n", Port);
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v3);
#endif
}

char szServerIPAddress[128];
char *__cdecl CL_GetServerIPAddress()
{
    clientConnection_t *clc; // [esp+0h] [ebp-4h]

    if ( CL_GetLocalClientConnectionState(0) >= 6 )
    {
        clc = CL_GetLocalClientConnection(0);
        Com_sprintf(
            szServerIPAddress,
            0x80u,
            "%i.%i.%i.%i:%i",
            clc->serverAddress.ip[0],
            clc->serverAddress.ip[1],
            clc->serverAddress.ip[2],
            clc->serverAddress.ip[3],
            clc->serverAddress.port);
    }
    else
    {
        memset(szServerIPAddress, 0, sizeof(szServerIPAddress));
    }
    return szServerIPAddress;
}

void    CL_SetupForNewServerMap(int localClientNum, char *pszMapName, const char *pszGametype)
{
    int num; // [esp+0h] [ebp-4h]

    Com_Printf(14, "Server changing map %s, gametype %s\n", pszMapName, pszGametype);
    if ( !com_sv_running->current.enabled )
        Cbuf_ExecuteBuffer(0, 0, (char*)"selectStringTableEntryInDvar mp/didyouknow.csv 0 didyouknow");
    if ( (!*pszMapName || !*pszGametype)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                    8225,
                    0,
                    "%s",
                    "pszMapName[0] && pszGametype[0]") )
    {
        __debugbreak();
    }
    cl_serverLoadingMap = 1;
    for ( num = 0; num < 1; ++num )
        cl_waitingOnServerToLoadMap[num] = 0;
    if ( !com_sv_running->current.enabled )
    {
        com_expectedHunkUsage = 0;
        g_waitingForServer = 1;
        FS_DisablePureCheck(1);
        UI_SetMap((char *)"", "");
        LoadMapLoadscreen(pszMapName);
        UI_SetMap(pszMapName, pszGametype);
    }
    SCR_UpdateScreen();
}

bool __cdecl CL_IsServerLoadingMap()
{
    return cl_serverLoadingMap;
}

bool __cdecl CL_IsWaitingOnServerToLoadMap(int localClientNum)
{
    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                    8286,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    return cl_waitingOnServerToLoadMap[localClientNum];
}

void __cdecl CL_SetWaitingOnServerToLoadMap(int localClientNum, bool waiting)
{
    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_mp.cpp",
                    8293,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    cl_waitingOnServerToLoadMap[localClientNum] = waiting;
}

void __cdecl CL_SyncGpu()
{
    R_SyncGpu();
}

void __cdecl CL_DrawTextPhysical(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float w,
                float xScale,
                float yScale,
                const float *color,
                int style)
{
    R_AddCmdDrawTextW(text, maxChars, font, x, y, w, xScale, yScale, 0.0, color, style);
}

void __cdecl CL_DrawTextPhysicalWithEffects(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float w,
                float xScale,
                float yScale,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration)
{
    R_AddCmdDrawTextWithEffects(
        text,
        maxChars,
        font,
        x,
        y,
        w,
        xScale,
        yScale,
        0.0,
        color,
        style,
        glowColor,
        fxMaterial,
        fxMaterialGlow,
        fxBirthTime,
        fxLetterTime,
        fxDecayStartTime,
        fxDecayDuration);
}

void __cdecl CL_DrawText(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                int horzAlign,
                int vertAlign,
                float xScale,
                float yScale,
                const float *color,
                int style)
{
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    R_AddCmdDrawText(text, maxChars, font, x, y, xScale, yScale, 0.0, color, style);
}

void __cdecl CL_DrawTextWithEffects(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float rotation,
                int horzAlign,
                int vertAlign,
                float xScale,
                float yScale,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration)
{
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    R_AddCmdDrawTextWithEffects(
        text,
        maxChars,
        font,
        x,
        y,
        1.0,
        xScale,
        yScale,
        rotation,
        color,
        style,
        glowColor,
        fxMaterial,
        fxMaterialGlow,
        fxBirthTime,
        fxLetterTime,
        fxDecayStartTime,
        fxDecayDuration);
}

void __cdecl CL_DrawTextWithCOD7TypeWriterEffects(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float rotation,
                int horzAlign,
                int vertAlign,
                float xScale,
                float yScale,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration)
{
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    R_AddCmdDrawTextWithCOD7TypeWriterEffects(
        text,
        maxChars,
        font,
        x,
        y,
        xScale,
        yScale,
        rotation,
        color,
        style,
        glowColor,
        fxMaterial,
        fxMaterialGlow,
        fxBirthTime,
        fxLetterTime,
        fxDecayStartTime,
        fxDecayDuration);
}

void __cdecl CL_DrawTextWithRedactEffects(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                int horzAlign,
                int vertAlign,
                float xScale,
                float yScale,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                int fxRedactDecayStartTime,
                int fxRedactDecayDuration)
{
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    R_AddCmdDrawTextWithRedactEffects(
        text,
        maxChars,
        font,
        x,
        y,
        xScale,
        yScale,
        0.0,
        color,
        style,
        glowColor,
        fxMaterial,
        fxMaterialGlow,
        fxBirthTime,
        fxLetterTime,
        fxDecayStartTime,
        fxDecayDuration,
        fxRedactDecayStartTime,
        fxRedactDecayDuration);
}

void __cdecl CL_DrawTextPhysicalWithCOD7DecodeEffects(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration)
{
    R_AddCmdDrawTextWithCOD7DecodeEffects(
        text,
        maxChars,
        font,
        x,
        y,
        xScale,
        yScale,
        0.0,
        color,
        style,
        glowColor,
        fxMaterial,
        fxMaterialGlow,
        fxBirthTime,
        fxLetterTime,
        fxDecayStartTime,
        fxDecayDuration);
}

void __cdecl CL_DrawTextPhysicalWithRedactEffects(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                int fxRedactDecayStartTime,
                int fxRedactDecayDuration)
{
    R_AddCmdDrawTextWithRedactEffects(
        text,
        maxChars,
        font,
        x,
        y,
        xScale,
        yScale,
        0.0,
        color,
        style,
        glowColor,
        fxMaterial,
        fxMaterialGlow,
        fxBirthTime,
        fxLetterTime,
        fxDecayStartTime,
        fxDecayDuration,
        fxRedactDecayStartTime,
        fxRedactDecayDuration);
}

void __cdecl CL_DrawTextWithPopInEffects(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float rotation,
                int horzAlign,
                int vertAlign,
                float xScale,
                float yScale,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration)
{
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    R_AddCmdDrawTextWithPopInEffects(
        text,
        maxChars,
        font,
        x,
        y,
        xScale,
        yScale,
        rotation,
        color,
        style,
        glowColor,
        fxMaterial,
        fxMaterialGlow,
        fxBirthTime,
        fxLetterTime,
        fxDecayStartTime,
        fxDecayDuration);
}

void __cdecl CL_DrawTextRotate(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float rotation,
                int horzAlign,
                int vertAlign,
                float xScale,
                float yScale,
                const float *color,
                int style)
{
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    R_AddCmdDrawText(text, maxChars, font, x, y, xScale, yScale, rotation, color, style);
}

void __cdecl CL_DrawTextPhysicalWithCursor(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                const float *color,
                int style,
                int cursorPos,
                char cursor)
{
    R_AddCmdDrawTextWithCursor(text, maxChars, font, x, y, 1.0, xScale, yScale, 0.0, color, style, cursorPos, cursor);
}

void __cdecl CL_DrawTextWithCursor(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                int horzAlign,
                int vertAlign,
                float xScale,
                float yScale,
                const float *color,
                int style,
                int cursorPos,
                char cursor)
{
    ScrPlace_ApplyRect(scrPlace, &x, &y, &xScale, &yScale, horzAlign, vertAlign);
    R_AddCmdDrawTextWithCursor(text, maxChars, font, x, y, 1.0, xScale, yScale, 0.0, color, style, cursorPos, cursor);
}

bool __cdecl CL_ShouldDisplayHud(int localClientNum)
{
    if ( cl_hudDrawsBehindUI->current.enabled )
        return 1;
    return (CL_GetLocalClientUIGlobals(localClientNum)->keyCatchers & 0x10) == 0
            || CL_LocalClient_IsCUIFlagSet(localClientNum, 64);
}

bool __cdecl CL_IsUIActive(int localClientNum)
{
    return (CL_GetLocalClientUIGlobals(localClientNum)->keyCatchers & 0x10) != 0;
}

Font_s *__cdecl CL_RegisterFont(const char *fontName, int imageTrack)
{
    return R_RegisterFont(fontName, imageTrack);
}

float (*__cdecl CL_GetMapCenter())[3]
{
    return (float (*)[3])cls.mapCenter;
}

void __cdecl CL_SanitizeClanName()
{
    const char *currentName; // [esp+10h] [ebp-20h]
    char saneNameBuf[8]; // [esp+14h] [ebp-1Ch] BYREF
    int nameLen; // [esp+20h] [ebp-10h]
    int curChar; // [esp+24h] [ebp-Ch]
    char *saneName; // [esp+28h] [ebp-8h]
    int i; // [esp+2Ch] [ebp-4h]

    memset(saneNameBuf, 0, 5);
    saneName = saneNameBuf;
    currentName = Dvar_GetString("clanName");
    if ( currentName )
    {
        nameLen = strlen(currentName);
        for ( i = 0; i < nameLen; ++i )
        {
            curChar = CL_FilterChar(currentName[i]);
            if ( curChar > 0 )
                *saneName++ = curChar;
        }
        Dvar_SetStringByName("clanName", saneNameBuf);
    }
}

int __cdecl CL_FilterChar(unsigned __int8 input)
{
    if ( input == 94 )
        return 32;
    if ( input < 0x20u )
        return -1;
    if ( input == 188 || input == 189 )
        return -1;
    return input;
}

//clientStatic_t::clientStatic_t()
//{
//    //int v3; // [esp+4h] [ebp-38h]
//    //char *jj; // [esp+8h] [ebp-34h]
//    //int v5; // [esp+Ch] [ebp-30h]
//    //char *ii; // [esp+10h] [ebp-2Ch]
//    //int v7; // [esp+14h] [ebp-28h]
//    //char *n; // [esp+18h] [ebp-24h]
//    //int v9; // [esp+1Ch] [ebp-20h]
//    //char *m; // [esp+20h] [ebp-1Ch]
//    //int v11; // [esp+24h] [ebp-18h]
//    //serverInfo_t *k; // [esp+28h] [ebp-14h]
//    //int v13; // [esp+2Ch] [ebp-10h]
//    //serverInfo_t *j; // [esp+30h] [ebp-Ch]
//    //int v15; // [esp+34h] [ebp-8h]
//    //serverInfo_t *i; // [esp+38h] [ebp-4h]
//    //
//    //v15 = 128;
//    //for ( i = this->localServers; --v15 >= 0; ++i )
//    //{
//    //    bdSecurityKey::bdSecurityKey(&i->xnkey);
//    //    bdSecurityID::bdSecurityID(&i->xnkid);
//    //}
//    //v13 = 20000;
//    //for ( j = this->rankedServers; --v13 >= 0; ++j )
//    //{
//    //    bdSecurityKey::bdSecurityKey(&j->xnkey);
//    //    bdSecurityID::bdSecurityID(&j->xnkid);
//    //}
//    //v11 = 20000;
//    //for ( k = this->unrankedServers; --v11 >= 0; ++k )
//    //{
//    //    bdSecurityKey::bdSecurityKey(&k->xnkey);
//    //    bdSecurityID::bdSecurityID(&k->xnkid);
//    //}
//    //v9 = 20000;
//    //for ( m = (char *)&s_unlockableItems.itemTable[21].challengeIndices[1] + (unsigned int)this; --v9 >= 0; m += 376 )
//    //{
//    //    bdSecurityKey::bdSecurityKey((bdSecurityKey *)(m + 25));
//    //    bdSecurityID::bdSecurityID((bdSecurityID *)(m + 41));
//    //}
//    //v7 = 256;
//    //for ( n = &cls.rankedServers[16220].hostName[(unsigned int)this + 19]; --v7 >= 0; n += 376 )
//    //{
//    //    bdSecurityKey::bdSecurityKey((bdSecurityKey *)(n + 25));
//    //    bdSecurityID::bdSecurityID((bdSecurityID *)(n + 41));
//    //}
//    //v5 = 20000;
//    //for ( ii = &cls.rankedServers[16476].hostName[(unsigned int)this + 27]; --v5 >= 0; ii += 376 )
//    //{
//    //    bdSecurityKey::bdSecurityKey((bdSecurityKey *)(ii + 25));
//    //    bdSecurityID::bdSecurityID((bdSecurityID *)(ii + 41));
//    //}
//    //v3 = 128;
//    //for ( jj = &cls.unrankedServers[16476].hostName[(unsigned int)this + 27]; --v3 >= 0; jj += 376 )
//    //{
//    //    bdSecurityKey::bdSecurityKey((bdSecurityKey *)(jj + 25));
//    //    bdSecurityID::bdSecurityID((bdSecurityID *)(jj + 41));
//    //}
//    //return this;
//}

bool __cdecl CL_IsLocalClientInGame(int localClientNum)
{
    return CL_GetLocalClientConnectionState(localClientNum) == 10;
}

