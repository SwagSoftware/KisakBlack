#include "demo_common.h"

#include <qcommon/cmd.h>
#include <ui/ui_shared.h>
#include <live/live_fileshare.h>
#include <client_mp/sv_client_mp.h>
#include <live/live_win.h>
#include <qcommon/common.h>
#include <live/live_stats.h>
#include <server_mp/sv_main_mp.h>
#include <qcommon/com_clients.h>
#include <DW/MatchRecorder.h>
#include "demo_recording.h"
#include <live/live_storage.h>
#include <universal/com_shared.h>

#include <cstring>
#include <new>
#include <ui_mp/ui_gametype_custom_mp.h>
#include <live/live_sessions.h>
#include <game_mp/g_cmds_mp.h>
#include <client/client.h>
#include "demo_playback.h"
#include <cgame_mp/cg_main_mp.h>
#include <win32/win_shared.h>
#include "demo_files.h"
#include <client/cl_console.h>
#include <sound/snd_public_async.h>
#include <client_mp/cl_cgame_mp.h>
#include "demo_ui.h"
#include <client/cl_keys.h>
#include <ctime>
#include <live/live_counter.h>
#include <game_mp/g_main_mp.h>
#include <qcommon/com_gamemodes.h>
#include <mjpeg/mjpeg.h>
#include "demo_profile.h"
#include <universal/com_workercmds.h>
#include <live/live_sessions_win.h>

const char *demo_tags_enum_string_37[8] =
{
  "MENU_SEGMENT_TAG_KILL",
  "MENU_SEGMENT_TAG_DEATH",
  "MENU_SEGMENT_TAG_KILLSTREAK",
  "MENU_SEGMENT_TAG_PERK",
  "MENU_SEGMENT_TAG_WEAPON",
  "MENU_SEGMENT_TAG_EQUIPMENT",
  "MENU_SEGMENT_TAG_GRENADE",
  "MENU_SEGMENT_TAG_MELEE"
};

const char *demoKeyboardModeNames_69[8] =
{
  "none",
  "segmentName",
  "clipNameIngame",
  "clipDescIngame",
  "screenshotNameIngame",
  "screenshotDescIngame",
  "fileshareFileName",
  "fileshareFileDescription"
};

int s_demoFileHandle;
unsigned __int8 *g_keyframeBuf;

demoClient_s demoClient;
entityState_s g_defaultEntityState;
clientState_s g_defaultClientState;

const dvar_t *demo_enabled;
const dvar_t *demo_recordBasicTraining;
const dvar_t *demo_recordPrivateMatch;
const dvar_t *demo_filesizeLimit;
const dvar_t *demo_debug;
const dvar_t *demo_drawdebuginformation;
const dvar_t *demo_errortitle;
const dvar_t *demo_errormessage;
const dvar_t *demo_client;
const dvar_t *demo_recordingrate;
const dvar_t *demo_keyframerate;
const dvar_t *demo_pause;
const dvar_t *demo_usefilesystem;
const dvar_t *demo_save_smp;
const dvar_t *demo_timescale;
const dvar_t *demo_timeScaleRate;
const dvar_t *demo_cmdNum;
const dvar_t *demo_bookmarkEventThresholdTime;
const dvar_t *demo_controllerConfig;
const dvar_t *demo_defaultSegmentTag;
const dvar_t *demo_selectedSegmentIndex;
const dvar_t *demo_desiredTime;
const dvar_t *demo_desiredClient;
const dvar_t *demo_packetsPerSecondMin;
const dvar_t *demo_bytesPerSecondMin;
const dvar_t *demo_packetsPerSecondMax;
const dvar_t *demo_bytesPerSecondMax;

demoMain demo;
ddlState_t g_fileShareRootState;
ddlDef_t *g_fileshareDDL;
demoRecordedFileUploadInfo s_demoUploadInfo;

void __cdecl Demo_RegisterDvars()
{
    demo_enabled = _Dvar_RegisterBool("demo_enabled", 1, 0, "Used to turn the system on/off.");
    demo_recordBasicTraining = _Dvar_RegisterBool(
                                                             "demo_recordBasicTraining",
                                                             0,
                                                             0,
                                                             "Used to turn the basic training recording on/off.");
    demo_recordPrivateMatch = _Dvar_RegisterBool(
                                                            "demo_recordPrivateMatch",
                                                            0,
                                                            0,
                                                            "Used to turn the private match recording on/off.");
    demo_filesizeLimit = _Dvar_RegisterFloat(
                                                 "demo_filesizeLimit",
                                                 10.0,
                                                 0.1,
                                                 3.4028235e38,
                                                 0,
                                                 "The maximum filesize (in MB) of the demos which we support.");
    demo_debug = _Dvar_RegisterInt("demo_debug", 0, 0, 0x7FFF, 0, "Debug info for the Server Snapshot Demo system");
    demo_drawdebuginformation = _Dvar_RegisterInt(
                                                                "demo_drawdebuginformation",
                                                                0,
                                                                0,
                                                                3,
                                                                0,
                                                                "Used to draw debug information.");
    demo_errortitle = _Dvar_RegisterString(
                                            "demo_errortitle",
                                            (char *)"",
                                            0x40u,
                                            "Most recent demo error's title.");
    demo_errormessage = _Dvar_RegisterString(
                                                "demo_errormessage",
                                                (char *)"",
                                                0x40u,
                                                "Most recent demo error's message.");
    demo_client = _Dvar_RegisterInt("demo_client", 0, 0, 32, 0, "Current viewing player");
    demo_recordingrate = _Dvar_RegisterInt(
                                                 "demo_recordingrate",
                                                 100,
                                                 50,
                                                 0x7FFFFFFF,
                                                 0,
                                                 "Used to tweak the rate(in msec) at which we write a super snapshot");
    demo_keyframerate = _Dvar_RegisterInt(
                                                "demo_keyframerate",
                                                10,
                                                1,
                                                0x7FFFFFFF,
                                                0,
                                                "Used to specify the rate(in sec) at which we generate a keyframe during playback.");
    demo_pause = _Dvar_RegisterBool("demo_pause", 0, 0, "Used to pause a demo playback.");
    demo_usefilesystem = _Dvar_RegisterBool("demo_usefilesystem", 0, 0, "Used to turn HDD write ON or OFF.");
    demo_save_smp = _Dvar_RegisterBool("demo_save_smp", 0, 0, "Used to toggle threaded save for the demo system.");
    demo_timescale = _Dvar_RegisterFloat(
                                         "demo_timescale",
                                         1.0,
                                         0.001,
                                         4.0,
                                         0,
                                         "The rate at which we want to scale time. For slo-mo it will be time/rate, fast mo. it will be time * rate");
    demo_timeScaleRate = _Dvar_RegisterFloat(
                                                 "demo_timeScaleRate",
                                                 4.0,
                                                 1.0,
                                                 4.0,
                                                 0,
                                                 "The rate at which we want to scale time. For slo-mo it will be time/rate, fast mo. it will be time * rate");
    demo_cmdNum = _Dvar_RegisterInt(
                                    "demo_cmdnum",
                                    0,
                                    0,
                                    0x7FFFFFFF,
                                    0,
                                    "The number of the command sent from console/ingame menu. ");
    demo_bookmarkEventThresholdTime = _Dvar_RegisterFloat(
                                                                            "demo_bookmarkEventThresholdTime",
                                                                            2.0,
                                                                            1.0,
                                                                            4.0,
                                                                            0,
                                                                            "The time duration for which we want to show the bookmark event image in the demo timeline.");
    demo_controllerConfig = _Dvar_RegisterInt(
                                                        "demo_controllerConfig",
                                                        0,
                                                        0,
                                                        2,
                                                        0,
                                                        "The number of the command sent from console/ingame menu. ");
    demo_defaultSegmentTag = _Dvar_RegisterString(
                                                         "demo_defaultSegmentTag",
                                                         (char *)"",
                                                         0,
                                                         "The text to be displayed as the default tag for the segment in the UI (save prompt).");
    demo_selectedSegmentIndex = _Dvar_RegisterInt(
                                                                "demo_selectedSegmentIndex",
                                                                0,
                                                                0,
                                                                0x7FFFFFFF,
                                                                0,
                                                                "Used in the Manage Segments UI. This will be used to know which segment index we have selected.");
    demo_desiredTime = _Dvar_RegisterInt(
                                             "demo_desiredtime",
                                             -1,
                                             -1,
                                             0x7FFFFFFF,
                                             0,
                                             "Used to force demo to a certain time");
    demo_desiredClient = _Dvar_RegisterInt(
                                                 "demo_desiredclient",
                                                 -1,
                                                 -1,
                                                 0x7FFFFFFF,
                                                 0,
                                                 "Used to force demo to a certain client");
    demo_packetsPerSecondMin = _Dvar_RegisterInt(
                                                             "demo_packetsPerSecondMin",
                                                             2,
                                                             0,
                                                             0x7FFFFFFF,
                                                             0,
                                                             "Min amount of packets to send per second before throttling.");
    demo_bytesPerSecondMin = _Dvar_RegisterInt(
                                                         "demo_bytesPerSecondMin",
                                                         2048,
                                                         0,
                                                         0x7FFFFFFF,
                                                         0,
                                                         "Min amount of bytes to send per second before throttling.");
    demo_packetsPerSecondMax = _Dvar_RegisterInt(
                                                             "demo_packetsPerSecondMax",
                                                             10,
                                                             0,
                                                             0x7FFFFFFF,
                                                             0,
                                                             "Max amount of packets to send per second before throttling.");
    demo_bytesPerSecondMax = _Dvar_RegisterInt(
                                                         "demo_bytesPerSecondMax",
                                                         0x2000,
                                                         0,
                                                         0x7FFFFFFF,
                                                         0,
                                                         "Max amount of bytes to send per second before throttling.");
}

cmd_function_s Demo_StartRecord_f_VAR;
cmd_function_s Demo_StartRecord_f_VAR_SERVER;
cmd_function_s Demo_StopRecord_f_VAR;
cmd_function_s Demo_StopRecord_f_VAR_SERVER;
cmd_function_s Demo_Play_f_VAR;
cmd_function_s Demo_Play_f_VAR_SERVER;
cmd_function_s Demo_Stop_f_VAR;
cmd_function_s Demo_Stop_f_VAR_SERVER;
cmd_function_s Demo_Forward_f_VAR;
cmd_function_s Demo_Forward_f_VAR_SERVER;
cmd_function_s Demo_Back_f_VAR;
cmd_function_s Demo_Back_f_VAR_SERVER;
cmd_function_s Demo_Pause_f_VAR;
cmd_function_s Demo_Pause_f_VAR_SERVER;
cmd_function_s Demo_JumpToStart_f_VAR;
cmd_function_s Demo_JumpToStart_f_VAR_SERVER;
cmd_function_s Demo_TimeScale_f_VAR;
cmd_function_s Demo_TimeScale_f_VAR_SERVER;
cmd_function_s Demo_ToggleGameHud_f_VAR;
cmd_function_s Demo_ToggleGameHud_f_VAR_SERVER;
cmd_function_s Demo_ToggleDemoHud_f_VAR;
cmd_function_s Demo_ToggleDemoHud_f_VAR_SERVER;
cmd_function_s Demo_SwitchPlayer_f_VAR;
cmd_function_s Demo_SwitchPlayer_f_VAR_SERVER;
cmd_function_s Demo_SwitchCamera_f_VAR;
cmd_function_s Demo_SwitchCamera_f_VAR_SERVER;
cmd_function_s Demo_StartClipRecord_f_VAR;
cmd_function_s Demo_StartClipRecord_f_VAR_SERVER;
cmd_function_s Demo_PauseClipRecord_f_VAR;
cmd_function_s Demo_PauseClipRecord_f_VAR_SERVER;
cmd_function_s Demo_PreviewClip_f_VAR;
cmd_function_s Demo_PreviewClip_f_VAR_SERVER;
cmd_function_s Demo_DeleteClip_f_VAR;
cmd_function_s Demo_DeleteClip_f_VAR_SERVER;
cmd_function_s Demo_SaveAndUploadClip_f_VAR;
cmd_function_s Demo_SaveAndUploadClip_f_VAR_SERVER;
cmd_function_s Demo_SaveSegment_f_VAR;
cmd_function_s Demo_SaveSegment_f_VAR_SERVER;
cmd_function_s Demo_MoveSegment_f_VAR;
cmd_function_s Demo_MoveSegment_f_VAR_SERVER;
cmd_function_s Demo_DeleteSegment_f_VAR;
cmd_function_s Demo_DeleteSegment_f_VAR_SERVER;
cmd_function_s Demo_PreviewSegment_f_VAR;
cmd_function_s Demo_PreviewSegment_f_VAR_SERVER;
cmd_function_s Demo_SwitchDefaultSegmentTag_f_VAR;
cmd_function_s Demo_SwitchDefaultSegmentTag_f_VAR_SERVER;
cmd_function_s Demo_SwitchTransition_f_VAR;
cmd_function_s Demo_SwitchTransition_f_VAR_SERVER;
cmd_function_s Demo_Keyboard_f_VAR;
cmd_function_s Demo_Keyboard_f_VAR_SERVER;
cmd_function_s Demo_Screenshot_f_VAR;
cmd_function_s Demo_Screenshot_f_VAR_SERVER;
cmd_function_s Demo_SaveScreenshot_f_VAR;
cmd_function_s Demo_SaveScreenshot_f_VAR_SERVER;
cmd_function_s Demo_RenderMovie_f_VAR;
cmd_function_s Demo_RenderMovie_f_VAR_SERVER;
cmd_function_s Demo_SwitchControls_f_VAR;
cmd_function_s Demo_SwitchControls_f_VAR_SERVER;
cmd_function_s Demo_AddDemoClient_f_VAR;
cmd_function_s Demo_AddDemoClient_f_VAR_SERVER;
cmd_function_s Demo_RemoveDemoClient_f_VAR;
cmd_function_s Demo_RemoveDemoClient_f_VAR_SERVER;

void __cdecl Demo_RegisterCommands()
{
    Cmd_AddCommandInternal("demo_startrecord", Cbuf_AddServerText_f, &Demo_StartRecord_f_VAR);
    Cmd_AddServerCommandInternal("demo_startrecord", Demo_StartRecord_f, &Demo_StartRecord_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_stoprecord", Cbuf_AddServerText_f, &Demo_StopRecord_f_VAR);
    Cmd_AddServerCommandInternal("demo_stoprecord", Demo_StopRecord_f, &Demo_StopRecord_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_play", Cbuf_AddServerText_f, &Demo_Play_f_VAR);
    Cmd_AddServerCommandInternal("demo_play", Demo_Play_f, &Demo_Play_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_stop", Cbuf_AddServerText_f, &Demo_Stop_f_VAR);
    Cmd_AddServerCommandInternal("demo_stop", Demo_Stop_f, &Demo_Stop_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_forward", Cbuf_AddServerText_f, &Demo_Forward_f_VAR);
    Cmd_AddServerCommandInternal("demo_forward", Demo_Forward_f, &Demo_Forward_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_back", Cbuf_AddServerText_f, &Demo_Back_f_VAR);
    Cmd_AddServerCommandInternal("demo_back", Demo_Back_f, &Demo_Back_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_pause", Cbuf_AddServerText_f, &Demo_Pause_f_VAR);
    Cmd_AddServerCommandInternal("demo_pause", Demo_Pause_f, &Demo_Pause_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_jumptostart", Cbuf_AddServerText_f, &Demo_JumpToStart_f_VAR);
    Cmd_AddServerCommandInternal("demo_jumptostart", Demo_JumpToStart_f, &Demo_JumpToStart_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_timescale", Cbuf_AddServerText_f, &Demo_TimeScale_f_VAR);
    Cmd_AddServerCommandInternal("demo_timescale", Demo_TimeScale_f, &Demo_TimeScale_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_togglegamehud", Cbuf_AddServerText_f, &Demo_ToggleGameHud_f_VAR);
    Cmd_AddServerCommandInternal("demo_togglegamehud", Demo_ToggleGameHud_f, &Demo_ToggleGameHud_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_toggledemohud", Cbuf_AddServerText_f, &Demo_ToggleDemoHud_f_VAR);
    Cmd_AddServerCommandInternal("demo_toggledemohud", Demo_ToggleDemoHud_f, &Demo_ToggleDemoHud_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_switchplayer", Cbuf_AddServerText_f, &Demo_SwitchPlayer_f_VAR);
    Cmd_AddServerCommandInternal("demo_switchplayer", Demo_SwitchPlayer_f, &Demo_SwitchPlayer_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_switchcamera", Cbuf_AddServerText_f, &Demo_SwitchCamera_f_VAR);
    Cmd_AddServerCommandInternal("demo_switchcamera", Demo_SwitchCamera_f, &Demo_SwitchCamera_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_startcliprecord", Cbuf_AddServerText_f, &Demo_StartClipRecord_f_VAR);
    Cmd_AddServerCommandInternal("demo_startcliprecord", Demo_StartClipRecord_f, &Demo_StartClipRecord_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_pausecliprecord", Cbuf_AddServerText_f, &Demo_PauseClipRecord_f_VAR);
    Cmd_AddServerCommandInternal("demo_pausecliprecord", Demo_PauseClipRecord_f, &Demo_PauseClipRecord_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_previewclip", Cbuf_AddServerText_f, &Demo_PreviewClip_f_VAR);
    Cmd_AddServerCommandInternal("demo_previewclip", Demo_PreviewClip_f, &Demo_PreviewClip_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_deleteclip", Cbuf_AddServerText_f, &Demo_DeleteClip_f_VAR);
    Cmd_AddServerCommandInternal("demo_deleteclip", Demo_DeleteClip_f, &Demo_DeleteClip_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_saveanduploadclip", Cbuf_AddServerText_f, &Demo_SaveAndUploadClip_f_VAR);
    Cmd_AddServerCommandInternal("demo_saveanduploadclip", Demo_SaveAndUploadClip_f, &Demo_SaveAndUploadClip_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_savesegment", Cbuf_AddServerText_f, &Demo_SaveSegment_f_VAR);
    Cmd_AddServerCommandInternal("demo_savesegment", Demo_SaveSegment_f, &Demo_SaveSegment_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_movesegment", Cbuf_AddServerText_f, &Demo_MoveSegment_f_VAR);
    Cmd_AddServerCommandInternal("demo_movesegment", Demo_MoveSegment_f, &Demo_MoveSegment_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_deletesegment", Cbuf_AddServerText_f, &Demo_DeleteSegment_f_VAR);
    Cmd_AddServerCommandInternal("demo_deletesegment", Demo_DeleteSegment_f, &Demo_DeleteSegment_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_previewsegment", Cbuf_AddServerText_f, &Demo_PreviewSegment_f_VAR);
    Cmd_AddServerCommandInternal("demo_previewsegment", Demo_PreviewSegment_f, &Demo_PreviewSegment_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_switchdefaultsegmenttag", Cbuf_AddServerText_f, &Demo_SwitchDefaultSegmentTag_f_VAR);
    Cmd_AddServerCommandInternal(
        "demo_switchdefaultsegmenttag",
        Demo_SwitchDefaultSegmentTag_f,
        &Demo_SwitchDefaultSegmentTag_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_switchtransition", Cbuf_AddServerText_f, &Demo_SwitchTransition_f_VAR);
    Cmd_AddServerCommandInternal("demo_switchtransition", Demo_SwitchTransition_f, &Demo_SwitchTransition_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_keyboard", Cbuf_AddServerText_f, &Demo_Keyboard_f_VAR);
    Cmd_AddServerCommandInternal("demo_keyboard", Demo_Keyboard_f, &Demo_Keyboard_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_screenshot", Cbuf_AddServerText_f, &Demo_Screenshot_f_VAR);
    Cmd_AddServerCommandInternal("demo_screenshot", (void (__cdecl *)())Demo_Screenshot_f, &Demo_Screenshot_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_savescreenshot", Cbuf_AddServerText_f, &Demo_SaveScreenshot_f_VAR);
    Cmd_AddServerCommandInternal("demo_savescreenshot", Demo_SaveScreenshot_f, &Demo_SaveScreenshot_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_rendermovie", Cbuf_AddServerText_f, &Demo_RenderMovie_f_VAR);
    Cmd_AddServerCommandInternal("demo_rendermovie", Demo_RenderMovie_f, &Demo_RenderMovie_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_switchcontrols", Cbuf_AddServerText_f, &Demo_SwitchControls_f_VAR);
    Cmd_AddServerCommandInternal("demo_switchcontrols", Demo_SwitchControls_f, &Demo_SwitchControls_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_adddemoclient", Cbuf_AddServerText_f, &Demo_AddDemoClient_f_VAR);
    Cmd_AddServerCommandInternal("demo_adddemoclient", Demo_AddDemoClient_f, &Demo_AddDemoClient_f_VAR_SERVER);
    Cmd_AddCommandInternal("demo_removedemoclient", Cbuf_AddServerText_f, &Demo_RemoveDemoClient_f_VAR);
    Cmd_AddServerCommandInternal("demo_removedemoclient", Demo_RemoveDemoClient_f, &Demo_RemoveDemoClient_f_VAR_SERVER);

    Cmd_SetAutoComplete("demo_play", "demos", "demo");
}

void __cdecl Demo_AddDemoClient_f()
{
    SV_AddDemoClient();
}

void __cdecl Demo_RemoveDemoClient_f()
{
    SV_RemoveDemoClient();
}

void __cdecl Demo_SetTags(
                int controllerIndex,
                int *numTags,
                bdTag *tags,
                demoMetaInfo *info,
                fileShareSearchFileType fileType)
{
    int IndexFromGameType; // eax
    int j; // [esp+0h] [ebp-14h]
    int i; // [esp+10h] [ebp-4h]

    if ( fileType == FILESHARE_FILETYPE_SCREENSHOT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_common.cpp",
                    258,
                    0,
                    "%s",
                    "fileType != FILESHARE_FILETYPE_SCREENSHOT") )
    {
        __debugbreak();
    }
    *numTags = 0;
    for ( i = 0; i < sharedUiInfo.mapCount; ++i )
    {
        if ( !I_strcmp(sharedUiInfo.mapList[i].mapLoadName, info->mapName) )
        {
            Live_FileShare_AddTag(2u, i, numTags, tags, 40);
            break;
        }
    }
    IndexFromGameType = Live_FileShare_GetIndexFromGameType(info->gameType);
    Live_FileShare_AddTag(1u, IndexFromGameType, numTags, tags, 40);
    switch ( fileType )
    {
        case FILESHARE_FILETYPE_CLIP:
        case FILESHARE_FILETYPE_FILM:
            Live_FileShare_AddTag(3u, info->type + 1, numTags, tags, 40);
            break;
        case FILESHARE_PUBLICFILES_START:
            Live_FileShare_AddTag(3u, 0x8000u, numTags, tags, 40);
            break;
        case FILESHARE_FILETYPE_SCREENSHOT:
            Live_FileShare_AddTag(3u, 3u, numTags, tags, 40);
            break;
    }
    if ( fileType != FILESHARE_FILETYPE_SCREENSHOT )
    {
        for ( j = 0; j < info->numConnectedPlayersInfoCount; ++j )
            Live_FileShare_AddTag(4u, info->connectedPlayers[j].xuid, numTags, tags, 40);
    }
}

void __cdecl Demo_SetTags(
                int controllerIndex,
                int *numTags,
                bdTag *tags,
                screenshotMetaInfo *info,
                fileShareSearchFileType fileType)
{
    int IndexFromGameType; // eax
    int j; // [esp+0h] [ebp-10h]
    int i; // [esp+4h] [ebp-Ch]

    if ( fileType != FILESHARE_FILETYPE_SCREENSHOT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_common.cpp",
                    313,
                    0,
                    "%s",
                    "fileType == FILESHARE_FILETYPE_SCREENSHOT") )
    {
        __debugbreak();
    }
    *numTags = 0;
    for ( i = 0; i < sharedUiInfo.mapCount; ++i )
    {
        if ( !I_strcmp(sharedUiInfo.mapList[i].mapLoadName, info->mapName) )
        {
            Live_FileShare_AddTag(2u, i, numTags, tags, 40);
            break;
        }
    }
    IndexFromGameType = Live_FileShare_GetIndexFromGameType(info->gameType);
    Live_FileShare_AddTag(1u, IndexFromGameType, numTags, tags, 40);
    Live_FileShare_AddTag(3u, 3u, numTags, tags, 40);
    for ( j = 0; j < demo.playback->screenshotInfo.screenshotPlayers.count; ++j )
        Live_FileShare_AddTag(4u, demo.playback->screenshotInfo.screenshotPlayers.playerXuids[j], numTags, tags, 40);
}

char __cdecl Demo_SetMetaData(
                int controllerIndex,
                char *metaData,
                int *metaDataSize,
                demoMetaInfo *dInfo,
                screenshotMetaInfo *sInfo,
                fileShareSearchFileType fileType)
{
    char *ClientName; // eax
    unsigned __int64 v8; // rax
    int v9; // eax
    int v10; // eax
    bool v11; // eax
    bool isModifiedDescription; // [esp+7h] [ebp-125h]
    bool isModifiedName; // [esp+Fh] [ebp-11Dh]
    char backup[256]; // [esp+14h] [ebp-118h] BYREF
    ddlState_t state; // [esp+118h] [ebp-14h] BYREF
    int bitSize; // [esp+128h] [ebp-4h]

    if ( fileType == FILESHARE_FILETYPE_SCREENSHOT )
    {
        if ( !sInfo )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_common.cpp",
                            352,
                            0,
                            "Screenshot info expected while setting meta data.") )
                __debugbreak();
            return 0;
        }
        goto LABEL_11;
    }
    if ( dInfo )
    {
LABEL_11:
        if ( !DDL_AssociateBuffer(metaData, 255, g_fileshareDDL)
            && !DDL_FixBufferVersion(metaData, g_fileshareDDL, "ddl_mp/file_share.ddl", backup, 255) )
        {
            return 0;
        }
        bitSize = DDL_GetTotalBufferBitSize(g_fileshareDDL);
        *metaDataSize = bitSize / 8;
        if ( bitSize % 8 )
            ++*metaDataSize;
        if ( DDL_MoveToName(&g_fileShareRootState, &state, "authorName")
            && (ClientName = Live_ControllerIndex_GetClientName(controllerIndex), DDL_SetString(&state, ClientName, metaData)) )
        {
            if ( DDL_MoveToName(&g_fileShareRootState, &state, "authorXuid")
                && (LODWORD(v8) = Live_GetXuid(controllerIndex), DDL_SetInt64(&state, v8, metaData)) )
            {
                if ( DDL_MoveToName(&g_fileShareRootState, &state, "name")
                    && (fileType != FILESHARE_FILETYPE_SCREENSHOT
                        ? (v9 = DDL_SetString(&state, dInfo->name, metaData))
                        : (v9 = DDL_SetString(&state, sInfo->name, metaData)),
                            v9) )
                {
                    if ( DDL_MoveToName(&g_fileShareRootState, &state, "isModifiedName")
                        && (fileType != FILESHARE_FILETYPE_SCREENSHOT
                            ? (isModifiedName = dInfo->isModifiedName)
                            : (isModifiedName = sInfo->isModifiedName),
                                DDL_SetInt(&state, isModifiedName, metaData)) )
                    {
                        if ( DDL_MoveToName(&g_fileShareRootState, &state, "description")
                            && (fileType != FILESHARE_FILETYPE_SCREENSHOT
                                ? (v10 = DDL_SetString(&state, dInfo->description, metaData))
                                : (v10 = DDL_SetString(&state, sInfo->description, metaData)),
                                    v10) )
                        {
                            if ( DDL_MoveToName(&g_fileShareRootState, &state, "isModifiedDescription")
                                && (fileType != FILESHARE_FILETYPE_SCREENSHOT
                                    ? (isModifiedDescription = dInfo->isModifiedDescription)
                                    : (isModifiedDescription = sInfo->isModifiedDescription),
                                        DDL_SetInt(&state, isModifiedDescription, metaData)) )
                            {
                                if ( DDL_MoveToName(&g_fileShareRootState, &state, "createTime")
                                    && (fileType != FILESHARE_FILETYPE_SCREENSHOT
                                        ? (v11 = DDL_SetInt(&state, dInfo->createTime, metaData))
                                        : (v11 = DDL_SetInt(&state, sInfo->createTime, metaData)),
                                            v11) )
                                {
                                    if ( fileType == FILESHARE_FILETYPE_SCREENSHOT
                                        || DDL_MoveToName(&g_fileShareRootState, &state, "length")
                                        && DDL_SetInt(&state, dInfo->endTime - dInfo->startTime, metaData) )
                                    {
                                        return 1;
                                    }
                                    else
                                    {
                                        Com_PrintError(16, "Could not set 'length' in the file share ddl.\n");
                                        return 0;
                                    }
                                }
                                else
                                {
                                    Com_PrintError(16, "Could not set 'createTime' in the file share ddl.\n");
                                    return 0;
                                }
                            }
                            else
                            {
                                Com_PrintError(16, "Could not set 'isModifiedDescription' in the file share ddl.\n");
                                return 0;
                            }
                        }
                        else
                        {
                            Com_PrintError(16, "Could not set 'description' in the file share ddl.\n");
                            return 0;
                        }
                    }
                    else
                    {
                        Com_PrintError(16, "Could not set 'isModifiedName' in the file share ddl.\n");
                        return 0;
                    }
                }
                else
                {
                    Com_PrintError(16, "Could not set 'name' in the file share ddl.\n");
                    return 0;
                }
            }
            else
            {
                Com_PrintError(16, "Could not set 'authorXuid' in the file share ddl.\n");
                return 0;
            }
        }
        else
        {
            Com_PrintError(16, "Could not set 'authorName' in the file share ddl.\n");
            return 0;
        }
    }
    if ( !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_common.cpp",
                    358,
                    0,
                    "Demo info expected while setting meta data.") )
        __debugbreak();
    return 0;
}

char gamerTag_0[32];
void __cdecl Demo_StreamingSuccessCallback(int controllerIndex, unsigned __int64 fileID)
{
    const ddlState_t *RootDDLState; // eax
    int v3; // eax
    char *ClientName; // eax
    int v5; // [esp+18h] [ebp-3F0h]
    bdTaskResult *k; // [esp+1Ch] [ebp-3ECh]
    int v7; // [esp+20h] [ebp-3E8h]
    bdTag *j; // [esp+24h] [ebp-3E4h]
    signed int i; // [esp+28h] [ebp-3E0h]
    int numTags; // [esp+2Ch] [ebp-3DCh] BYREF
    ddlState_t localState; // [esp+30h] [ebp-3D8h] BYREF
    char *matchRecordBuffer; // [esp+40h] [ebp-3C8h] BYREF
    int matchRecordBufferSize; // [esp+44h] [ebp-3C4h] BYREF
    bdTag tags[40]; // [esp+48h] [ebp-3C0h] BYREF
    int savedregs; // [esp+408h] [ebp+0h] BYREF

    RootDDLState = LiveStats_GetRootDDLState();
    DDL_MoveTo(RootDDLState, &localState, 2, "AfterActionReportStats", "demoFileID");
    if ( svs.clients )
    {
        for ( i = 0; i < demo.header.maxClients; ++i )
        {
            if ( svs.clients[i].header.state == CS_ACTIVE )
                SV_SetClientDInt64Stat(i, &localState, fileID);
        }
    }
    matchRecordBuffer = 0;
    matchRecordBufferSize = 0;
    if ( fileID )
    {
#ifdef KISAK_LIVE
        if ( onlinegame->current.enabled )
        {
            v3 = Com_LocalClient_GetControllerIndex(0);
            GenerateRecordedMatchData(v3, &matchRecordBuffer, &matchRecordBufferSize);
        }
        else
#endif
        {
            ClientName = Live_ControllerIndex_GetClientName(controllerIndex);
            I_strncpyz(gamerTag_0, ClientName, 32);
            matchRecordBuffer = gamerTag_0;
            matchRecordBufferSize = strlen(gamerTag_0);
        }
        v7 = 40;
        for (j = tags; --v7 >= 0; ++j)
        {
            //bdTag::bdTag(j);
            new (j) bdTag();
        }
        numTags = 0;
        Demo_SetTags(controllerIndex, &numTags, tags, &demo.info, FILESHARE_FILETYPE_FILM);
        memcpy(s_demoUploadInfo.tags, tags, sizeof(s_demoUploadInfo.tags));
        s_demoUploadInfo.numTags = numTags;
        memset(s_demoUploadInfo.metaData, 0, sizeof(s_demoUploadInfo.metaData));
        Live_FileShare_WritePublicMetaDataTags(s_demoUploadInfo.metaData, tags, numTags);
        Live_FileShare_WritePublicMetaDataLength(s_demoUploadInfo.metaData, demo.info.endTime - demo.info.startTime);
        Live_FileShare_WritePublicMetaDataCreateTime(s_demoUploadInfo.metaData, demo.info.createTime);
        s_demoUploadInfo.metaDataSize = 512;
        Dvar_SetString((dvar_s *)fsSelectedFileName, demo.info.name);
        Dvar_SetString((dvar_s *)fsSelectedFileDescription, demo.info.description);
        Dvar_SetBool((dvar_s *)fsIsSelectedFileNameModified, 0);
        Dvar_SetBool((dvar_s *)fsIsSelectedFileDescriptionModified, 0);
        LiveStorage_FileShare_WriteSummary(
            controllerIndex,
            fileID,
            FILESHARE_LOCATION_POOLEDSTORAGE,
            matchRecordBuffer,
            matchRecordBufferSize,
            s_demoUploadInfo.metaData,
            s_demoUploadInfo.metaDataSize,
            tags,
            numTags,
            0);
        v5 = 40;

        //for ( k = (bdTaskResult *)&savedregs; --v5 >= 0; bdTag::~bdTag(k) )
        // 
        //    k -= 6;

        //for (int i = 0; i < 40; i++)
        //    delete &tags[i];
    }
    else
    {
        Com_PrintError(16, "Trying to write a summary for a file that didn't get uploaded. Bailing out.\n");
    }
}

void __cdecl Demo_StreamingFailureCallback()
{
    Demo_StopStreaming();
}

void __cdecl Demo_StartRecord_f()
{
    const char *v0; // eax
    int v1; // eax
    const char *String; // [esp-18h] [ebp-78h]
    int v3; // [esp-14h] [ebp-74h]
    int tm_mday; // [esp-10h] [ebp-70h]
    int v5; // [esp-Ch] [ebp-6Ch]
    int tm_hour; // [esp-8h] [ebp-68h]
    int tm_min; // [esp-4h] [ebp-64h]
    int i; // [esp+0h] [ebp-60h]
    qtime_s systemTime; // [esp+4h] [ebp-5Ch] BYREF
    fileShareWriteFileInfo fileInfo; // [esp+2Ch] [ebp-34h] BYREF

    if ( Demo_IsIdle() )
    {
        Demo_Init();
        Com_RealTime(&systemTime, 1);
        tm_min = systemTime.tm_min;
        tm_hour = systemTime.tm_hour;
        v5 = systemTime.tm_year + 1900;
        tm_mday = systemTime.tm_mday;
        v3 = systemTime.tm_mon + 1;
        String = Dvar_GetString("mapname");
        v0 = Dvar_GetString("g_gametype");
        Com_sprintf(demo.demoName, 0x80u, "%s_%s_%d_%d_%d_%d_%d", v0, String, v3, tm_mday, v5, tm_hour, tm_min);
        if ( Demo_InitWrite() )
        {
            Demo_PopulateStaticInfoData(&demo.info);
            demo.header.version = 1;
            demo.header.maxClients = com_maxclients->current.integer;
            demo.header.settings.onlinegame = Dvar_GetBool("onlinegame");
            demo.header.settings.xblive_privatematch = Dvar_GetBool("xblive_privatematch");
            demo.header.settings.xblive_wagermatch = Dvar_GetBool("xblive_wagermatch");
            demo.header.settings.systemlink = Dvar_GetBool("systemlink");
            demo.header.settings.combatTraining = Dvar_GetBool("xblive_basictraining");
            demo.header.settings.customGameMode = UI_Gametype_IsUsingCustom();
            fileInfo.category = 1;
            fileInfo.fileName = va("%s.demo", demo.demoName);
            fileInfo.tags = 0;
            fileInfo.location = FILESHARE_LOCATION_POOLEDSTORAGE;
            fileInfo.fileSlot = 6;
            memset(&fileInfo.thumbData, 0, 12);
            fileInfo.fileSize = 0;
            fileInfo.dataCallback = (unsigned int (__cdecl *)(void *, unsigned int, unsigned int))Demo_WriteToStream;
            fileInfo.successCallback = Demo_StreamingSuccessCallback;
            fileInfo.failureCallback = (void (__cdecl *)(int))Demo_StreamingFailureCallback;
            if ( LiveStorage_FileShare_WriteFile(0, &fileInfo) )
                Demo_StartStreaming(0);
            v1 = Session_HostNum(&g_serverSession);
            Demo_SetDemoClientIndex(v1);
            Demo_WriteHeader(&demo.msg, 0);
            Demo_SetDemoState(1);
            Cbuf_AddText(0, "wait 10; demo_adddemoclient\n");
            if ( svs.clients )
            {
                for ( i = 0; i < demo.header.maxClients; ++i )
                {
                    if ( svs.clients[i].header.state == CS_ACTIVE )
                        demo.lastReliableCommandRecorded[i] = svs.clients[i].reliableAcknowledge;
                }
            }
        }
        else
        {
            Com_Printf(0, "Demo Initialization Failed!!. Couldn't start demo recording.");
        }
    }
    else
    {
        Com_Printf(0, "You cant play a demo file when it is recording/playing back.");
    }
}

void __cdecl Demo_StopRecord_f()
{
    Demo_End(0);
}

void __cdecl Demo_Play_f()
{
    const char *v0; // eax
    const char *v1; // eax
    int DemoState; // [esp+50h] [ebp-3Ch]
    char extension[32]; // [esp+58h] [ebp-34h] BYREF
    int len; // [esp+7Ch] [ebp-10h]
    const char *s; // [esp+80h] [ebp-Ch]
    int renderMovie; // [esp+84h] [ebp-8h]
    bool sameDemoName; // [esp+8Bh] [ebp-1h]

    sameDemoName = 0;
    if ( SV_Cmd_Argc() != 2 && SV_Cmd_Argc() != 3 )
    {
        Com_Printf(
            0,
            "Usage: demo_play <demoname> [renderMovie]. RenderMovie is optional and it will be called only through demo_rendermovie command.");
        return;
    }
    if ( CL_GetLocalClientConnectionState(0) < 4 )
    {
        s = SV_Cmd_Argv(1);
        Com_sprintf(extension, 0x20u, ".demo");
        DemoState = Demo_GetDemoState();
        if ( DemoState == 1 )
        {
            Com_Printf(0, "You cant play a demo file when it is recording.");
            return;
        }
        if ( DemoState == 2 )
        {
            if ( I_stricmp(&s[strlen(s) - (&extension[strlen(extension) + 1] - &extension[1])], extension) )
            {
                if ( !I_stricmp(s, demo.demoName) )
                    sameDemoName = 1;
            }
            else
            {
                v0 = va("%s.demo", demo.demoName);
                if ( !I_stricmp(s, v0) )
                    sameDemoName = 1;
            }
            if ( sameDemoName )
            {
                Com_Printf(0, "Already playing demo file %s.", demo.demoName);
                return;
            }
            Demo_End(0);
        }
        if ( !Demo_IsIdle()
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_common.cpp", 709, 0, "%s", "Demo_IsIdle()") )
        {
            __debugbreak();
        }
        Demo_Init();
        if ( SV_Cmd_Argc() == 3 )
        {
            v1 = SV_Cmd_Argv(2);
            renderMovie = atoi(v1);
            demo.renderingMovie = renderMovie != 0;
        }
        len = strlen(s);
        if ( I_stricmp(&s[len - (&extension[strlen(extension) + 1] - &extension[1])], extension) )
            I_strncpyz(demo.demoName, s, len + 1);
        else
            I_strncpyz(demo.demoName, s, len - (&extension[strlen(extension)] - &extension[1]));
        if ( Demo_InitRead() )
        {
            Demo_SetDemoState(2);
            Demo_ProcessPlayback();
        }
        else
        {
            Com_Printf(0, "Demo Initialization Failed!!. Couldnt start demo playback.");
        }
    }
}

void __cdecl Demo_Stop_f()
{
    if ( Demo_IsPlaying() )
    {
        if ( Cmd_Argc() <= 1 )
            Demo_End(0);
        else
            Demo_End(1);
    }
    else
    {
        Com_PrintError(14, "We are not playing a demo.\n");
    }
}

void __cdecl Demo_Forward_f()
{
    const char *v0; // eax
    int v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    const char *v12; // eax
    const char *v13; // eax
    const char *v14; // eax
    int v15; // esi
    clientActive_t *LocalClientGlobals; // [esp+3Ch] [ebp-5Ch]
    msg_t keyframeMsg; // [esp+40h] [ebp-58h] BYREF
    cg_s *cgameGlob; // [esp+70h] [ebp-28h]
    int localClientNum; // [esp+74h] [ebp-24h]
    int keyframeIndex; // [esp+78h] [ebp-20h]
    clientConnection_t *clc; // [esp+7Ch] [ebp-1Ch]
    cgs_t *cgs; // [esp+80h] [ebp-18h]
    int newKeyframeIndex; // [esp+84h] [ebp-14h]
    int msgSequence; // [esp+88h] [ebp-10h]
    int msgType; // [esp+8Ch] [ebp-Ch]
    int newServerTime; // [esp+90h] [ebp-8h]
    int preparse; // [esp+94h] [ebp-4h]

    if ( Demo_IsPlaying() && CL_GetLocalClientConnectionState(0) >= 4 )
    {
        v0 = Cmd_Argv(1);
        v1 = atoi(v0);
        if ( Demo_ShouldProcessCmd(v1) )
        {
            if ( Demo_IsCompleted() )
            {
                Com_PrintError(14, "We cannot forward a demo that is completed\n");
            }
            else if ( Demo_IsClipPreviewRunning() )
            {
                Com_PrintError(14, "We cannot forward a demo that is being previewed\n");
            }
            else
            {
                localClientNum = 0;
                Demo_SetCmdInProgess();
                LocalClientGlobals = CL_GetLocalClientGlobals(0);
                clc = CL_GetLocalClientConnection(0);
                cgs = CG_GetLocalClientStaticGlobals(0);
                cgameGlob = CG_GetLocalClientGlobals(0);
                if ( Cmd_Argc() == 3 )
                {
                    v2 = Cmd_Argv(2);
                    newServerTime = atoi(v2);
                    if ( newServerTime < 0
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_common.cpp",
                                    833,
                                    0,
                                    "%s",
                                    "newServerTime >= 0") )
                    {
                        __debugbreak();
                    }
                    v3 = va("DemoForward - We need to jump back to %d\n", newServerTime);
                    Demo_Printf(128, v3);
                }
                else
                {
                    v4 = va("DemoForward - We need to jump back to the previous keyframe.\n");
                    Demo_Printf(128, v4);
                    newServerTime = -1;
                }
                newKeyframeIndex = Demo_GetKeyFrameForJumpForward(localClientNum, newServerTime);
                if ( newKeyframeIndex >= 0 )
                {
                    if ( demo.playback->keyFrame[newKeyframeIndex].keyframeSnapshotTime < 0
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_common.cpp",
                                    852,
                                    0,
                                    "%s",
                                    "demo.playback->keyFrame[newKeyframeIndex].keyframeSnapshotTime >= 0") )
                    {
                        __debugbreak();
                    }
                    v6 = va(
                                 "DemoForward - newKeyframeIndex: %d currKeyframeIndex: %d\n",
                                 newKeyframeIndex,
                                 demo.playback->keyframeIndex);
                    Demo_Printf(128, v6);
                    keyframeIndex = newKeyframeIndex;
                    v7 = va(
                                 "DemoForward - We need to seeking file to %d block\n",
                                 demo.playback->keyFrame[newKeyframeIndex].demoFileMemoryBlockStart);
                    Demo_Printf(128, v7);
                    v8 = va(
                                 "DemoForward - Block's servertime is %d seq is %d\n",
                                 demo.playback->keyFrame[newKeyframeIndex].keyframeSnapshotTime,
                                 demo.playback->keyFrame[newKeyframeIndex].firstCmdSequence);
                    Demo_Printf(128, v8);
                    preparse = Sys_Milliseconds();

                    {
                        PROF_SCOPED("Forward Logic - File Seek");
                        demo.totalSize = demo.playback->keyFrame[newKeyframeIndex].demoFileMemoryBlockStart;
                        Demo_SeekFile(demo.demoFileHandle, demo.playback->keyFrame[newKeyframeIndex].demoFileMemoryBlockStart, 2);
                        Demo_Load();
                        MSG_GotoBookmark(&demo.msg, &demo.playback->keyFrame[newKeyframeIndex].msgBookmark);
                        demo.playback->lastJumpedKeyframe = newKeyframeIndex;
                    }
                    
                    MSG_Init(&keyframeMsg, demo.msgBuf1, 49152);
                    memcpy(
                        demo.msgBuf1,
                        &g_keyframeBuf[demo.playback->keyFrame[keyframeIndex].keyframeBufferStart],
                        demo.playback->keyFrame[keyframeIndex].size);
                    keyframeMsg.cursize = demo.playback->keyFrame[keyframeIndex].size;
                    v9 = va("Keyframe r - Time: %d\n", demo.playback->keyFrame[keyframeIndex].keyframeSnapshotTime);
                    Demo_Printf(128, v9);
                    v10 = va(
                                    "Keyframe r - File Related: FileStart: %d\n",
                                    demo.playback->keyFrame[keyframeIndex].demoFileMemoryBlockStart);
                    Demo_Printf(128, v10);
                    v11 = va(
                                    "Keyframe r - Demo Msg: Size: %d 1stByte: %d LastByte: %d\n",
                                    demo.msg.cursize,
                                    *demo.msg.data,
                                    demo.msg.data[demo.msg.cursize - 1]);
                    Demo_Printf(128, v11);
                    v12 = va(
                                    "Keyframe r - BookMark: ReadCount: %d Msg: %d\n",
                                    demo.msg.readcount,
                                    demo.msg.data[demo.msg.readcount]);
                    Demo_Printf(128, v12);
                    v13 = va("Keyframe r - BookMark: Bit: %d Msg:%d\n", demo.msg.bit, demo.msg.data[demo.msg.bit >> 3]);
                    Demo_Printf(128, v13);
                    v14 = va(
                                    "Keyframe r - KeyframeMsg: Index: %d BufIndex: %d Time: %d Size: %d 1stByte: %d LastByte: %d\n",
                                    keyframeIndex,
                                    demo.playback->keyFrame[keyframeIndex].keyframeBufferStart,
                                    demo.playback->keyFrame[keyframeIndex].keyframeSnapshotTime,
                                    keyframeMsg.cursize,
                                    *keyframeMsg.data,
                                    keyframeMsg.data[keyframeMsg.cursize - 1]);
                    Demo_Printf(128, v14);
                    Demo_SetCompleted(0);
                    if ( Demo_IsClipPlaying() )
                    {
                        demo.playback->clipTime = demo.playback->keyFrame[keyframeIndex].keyframeSnapshotTime;
                        demo.playback->clipServerTime = demo.playback->keyFrame[keyframeIndex].keyframeClipServerTime - 100;
                    }
                    {
                        PROF_SCOPED("Forward Logic - Process Keyframe");
                        MSG_UnEmbed(&keyframeMsg);
                        msgType = MSG_ReadBits(&keyframeMsg, 2u);
                        Demo_ResetSnapshotData();
                        while ( msgType == 2 )
                        {
                            Demo_ParseClipCommand(localClientNum, &keyframeMsg, 1);
                            msgType = MSG_ReadBits(&keyframeMsg, 2u);
                        }
                        msgSequence = MSG_ReadLong(&keyframeMsg);
                        Demo_ParseServerMessage(localClientNum, &keyframeMsg, msgType, msgSequence);
                    }
                    {
                        PROF_SCOPED("Forward Logic - Resetting World");
                        Demo_ResetTimeValues(localClientNum, LocalClientGlobals->snap.serverTime);
                        Demo_ResetSequenceNumbers(localClientNum, LocalClientGlobals->snap.messageNum - 1);
                        Demo_ResetWorldInformation(localClientNum, LocalClientGlobals->snap.serverTime);
                    }
                    {
                        PROF_SCOPED("Forward Logic - Reading Messages");
                        cgameGlob->latestSnapshotNum = LocalClientGlobals->snap.messageNum;
                        v15 = LocalClientGlobals->snap.serverTime - cls.realtime - 5;
                        LocalClientGlobals->serverTimeDelta = v15 - (Sys_Milliseconds() - preparse);
                        Demo_SetJumpTimeFlag(1);
                    }
                    
                    SND_StopSounds(SND_STOP_ALL);
                    demo.playback->overridePause = 1;
                    CL_SetCGameTime(localClientNum);
                    demo.playback->overridePause = 0;
                    ++demo.playback->keyframeJumpCount;
                    Demo_SetCmdCompleted();
                }
                else
                {
                    v5 = va("DemoForward - We couldnt find a suitable rewind keyframe.\n");
                    Demo_Printf(128, v5);
                    Demo_SetCmdCompleted();
                }
            }
        }
        else
        {
            Com_PrintError(14, "A command is in progress. We cant process this command.\n");
        }
    }
    else
    {
        Com_PrintError(14, "We are not playing a demo.\n");
    }
}

void __cdecl Demo_Back_f()
{
    const char *v0; // eax
    int v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    const char *v12; // eax
    const char *v13; // eax
    const char *v14; // eax
    int v15; // esi
    clientActive_t *LocalClientGlobals; // [esp+44h] [ebp-5Ch]
    msg_t keyframeMsg; // [esp+48h] [ebp-58h] BYREF
    cg_s *cgameGlob; // [esp+78h] [ebp-28h]
    int localClientNum; // [esp+7Ch] [ebp-24h]
    int keyframeIndex; // [esp+80h] [ebp-20h]
    clientConnection_t *clc; // [esp+84h] [ebp-1Ch]
    cgs_t *cgs; // [esp+88h] [ebp-18h]
    int newKeyframeIndex; // [esp+8Ch] [ebp-14h]
    int msgSequence; // [esp+90h] [ebp-10h]
    int msgType; // [esp+94h] [ebp-Ch]
    int newServerTime; // [esp+98h] [ebp-8h]
    int preparse; // [esp+9Ch] [ebp-4h]

    localClientNum = 0;
    if ( Demo_IsPlaying() && CL_GetLocalClientConnectionState(localClientNum) >= 4 )
    {
        v0 = Cmd_Argv(1);
        v1 = atoi(v0);
        if ( Demo_ShouldProcessCmd(v1) )
        {
            if ( Demo_IsClipPreviewRunning() )
            {
                Com_PrintError(14, "We cannot rewind a demo that is being previewed\n");
            }
            else
            {
                Demo_SetCmdInProgess();
                LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
                clc = CL_GetLocalClientConnection(localClientNum);
                cgs = CG_GetLocalClientStaticGlobals(localClientNum);
                cgameGlob = CG_GetLocalClientGlobals(localClientNum);

                PROF_SCOPED("Rewind Logic - Search for Keyframe");
                if ( Cmd_Argc() == 3 )
                {
                    v2 = Cmd_Argv(2);
                    newServerTime = atoi(v2);
                    if ( newServerTime < 0
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_common.cpp",
                                    1008,
                                    0,
                                    "%s",
                                    "newServerTime >= 0") )
                    {
                        __debugbreak();
                    }
                    v3 = va("DemoBack - We need to jump back to %d\n", newServerTime);
                    Demo_Printf(128, v3);
                }
                else
                {
                    v4 = va("DemoBack - We need to jump back to the previous keyframe.\n");
                    Demo_Printf(128, v4);
                    newServerTime = -1;
                }
                keyframeIndex = demo.playback->keyframeIndex;
                newKeyframeIndex = Demo_GetKeyFrameForJumpBack(localClientNum, newServerTime);
                if ( newKeyframeIndex >= 0 )
                {
                    v6 = va(
                                 "DemoBack - newKeyframeIndex: %d currKeyframeIndex: %d\n",
                                 newKeyframeIndex,
                                 demo.playback->keyframeIndex);
                    Demo_Printf(128, v6);
                    keyframeIndex = newKeyframeIndex;
                    v7 = va(
                                 "DemoBack - We need to seeking file to %d block\n",
                                 demo.playback->keyFrame[newKeyframeIndex].demoFileMemoryBlockStart);
                    Demo_Printf(128, v7);
                    v8 = va(
                                 "DemoBack - Block's servertime is %d seq is %d\n",
                                 demo.playback->keyFrame[newKeyframeIndex].keyframeSnapshotTime,
                                 demo.playback->keyFrame[newKeyframeIndex].firstCmdSequence);
                    Demo_Printf(128, v8);

                    preparse = Sys_Milliseconds();

                    {
                        PROF_SCOPED("Rewind Logic - File Seek");
                        demo.totalSize = demo.playback->keyFrame[keyframeIndex].demoFileMemoryBlockStart;
                        Demo_SeekFile(demo.demoFileHandle, demo.playback->keyFrame[keyframeIndex].demoFileMemoryBlockStart, 2);
                        Demo_Load();
                        MSG_GotoBookmark(&demo.msg, &demo.playback->keyFrame[keyframeIndex].msgBookmark);
                        demo.playback->lastJumpedKeyframe = keyframeIndex;
                    }

                    MSG_Init(&keyframeMsg, demo.msgBuf1, 49152);
                    memcpy(
                        demo.msgBuf1,
                        &g_keyframeBuf[demo.playback->keyFrame[keyframeIndex].keyframeBufferStart],
                        demo.playback->keyFrame[keyframeIndex].size);
                    keyframeMsg.cursize = demo.playback->keyFrame[keyframeIndex].size;
                    v9 = va("Keyframe r - Time: %d\n", demo.playback->keyFrame[keyframeIndex].keyframeSnapshotTime);
                    Demo_Printf(128, v9);
                    v10 = va(
                                    "Keyframe r - File Related: FileStart: %d\n",
                                    demo.playback->keyFrame[keyframeIndex].demoFileMemoryBlockStart);
                    Demo_Printf(128, v10);
                    v11 = va(
                                    "Keyframe r - Demo Msg: Size: %d 1stByte: %d LastByte: %d\n",
                                    demo.msg.cursize,
                                    *demo.msg.data,
                                    demo.msg.data[demo.msg.cursize - 1]);
                    Demo_Printf(128, v11);
                    v12 = va(
                                    "Keyframe r - BookMark: ReadCount: %d Msg: %d\n",
                                    demo.msg.readcount,
                                    demo.msg.data[demo.msg.readcount]);
                    Demo_Printf(128, v12);
                    v13 = va("Keyframe r - BookMark: Bit: %d Msg:%d\n", demo.msg.bit, demo.msg.data[demo.msg.bit >> 3]);
                    Demo_Printf(128, v13);
                    v14 = va(
                                    "Keyframe r - KeyframeMsg: Index: %d BufIndex: %d Time: %d Size: %d 1stByte: %d LastByte: %d\n",
                                    keyframeIndex,
                                    demo.playback->keyFrame[keyframeIndex].keyframeBufferStart,
                                    demo.playback->keyFrame[keyframeIndex].keyframeSnapshotTime,
                                    keyframeMsg.cursize,
                                    *keyframeMsg.data,
                                    keyframeMsg.data[keyframeMsg.cursize - 1]);
                    Demo_Printf(128, v14);
                    Demo_SetCompleted(0);
                    if ( Demo_IsClipPlaying() )
                    {
                        demo.playback->clipTime = demo.playback->keyFrame[keyframeIndex].keyframeSnapshotTime;
                        demo.playback->clipServerTime = demo.playback->keyFrame[keyframeIndex].keyframeClipServerTime - 100;
                    }

                    {
                        PROF_SCOPED("Rewind Logic - Process Keyframe");
                        MSG_UnEmbed(&keyframeMsg);
                        msgType = MSG_ReadBits(&keyframeMsg, 2u);
                        Demo_ResetSnapshotData();
                        while (msgType == 2)
                        {
                            Demo_ParseClipCommand(localClientNum, &keyframeMsg, 1);
                            msgType = MSG_ReadBits(&keyframeMsg, 2u);
                        }
                        msgSequence = MSG_ReadLong(&keyframeMsg);
                        Demo_ParseServerMessage(localClientNum, &keyframeMsg, msgType, msgSequence);
                    }
                    {
                        PROF_SCOPED("Rewind Logic - Rewinding World");
                        Demo_ResetTimeValues(localClientNum, LocalClientGlobals->snap.serverTime);
                        Demo_ResetSequenceNumbers(localClientNum, LocalClientGlobals->snap.messageNum - 1);
                        Demo_ResetWorldInformation(localClientNum, LocalClientGlobals->snap.serverTime);
                    }

                    v15 = LocalClientGlobals->snap.serverTime - cls.realtime - 5;
                    LocalClientGlobals->serverTimeDelta = v15 - (Sys_Milliseconds() - preparse);
                    Demo_SetJumpTimeFlag(1);
                    SND_StopSounds(SND_STOP_ALL);

                    {
                        PROF_SCOPED("Rewind Logic - CL_SetCGameTime()");
                        demo.playback->overridePause = 1;
                        CL_SetCGameTime(localClientNum);
                        demo.playback->overridePause = 0;
                    }

                    ++demo.playback->keyframeJumpCount;
                    Demo_SetCmdCompleted();
                }
                else
                {
                    v5 = va("DemoBack - Unable to find a proper previous keyframe.\n");
                    Demo_Printf(128, v5);
                    Demo_SetCmdCompleted();
                }
            }
        }
        else
        {
            Com_PrintError(14, "A command is in progress. We cant process this command.\n");
        }
    }
    else
    {
        Com_PrintError(14, "We are not playing a demo.\n");
    }
}

void __cdecl Demo_JumpToStart_f()
{
    int ControllerIndex; // eax
    int v1; // eax
    char *timescale; // [esp+0h] [ebp-8h]
    char *timescalea; // [esp+0h] [ebp-8h]

    if ( Demo_IsPlaying() && CL_GetLocalClientConnectionState(0) >= 4 )
    {
        if ( !Demo_IsClipPlaying() )
        {
            demo.playback->metaInformationScreenTime = Sys_Milliseconds();
            Com_SetTimeScale(1.0);
        }
        if ( Demo_IsPaused() )
        {
            timescale = va("demo_pause %i\n", demo_cmdNum->current.integer);
            ControllerIndex = Com_LocalClient_GetControllerIndex(0);
            Cmd_ExecuteSingleCommand(0, ControllerIndex, timescale);
        }
        timescalea = va("demo_back %d 0", demo_cmdNum->current.integer);
        v1 = Com_LocalClient_GetControllerIndex(0);
        Cmd_ExecuteSingleCommand(0, v1, timescalea);
    }
    else
    {
        Com_PrintError(14, "We are not playing a demo.\n");
    }
}

void __cdecl Demo_Pause_f()
{
    const char *v0; // eax
    int v1; // eax

    if ( Demo_IsPlaying() )
    {
        v0 = Cmd_Argv(1);
        v1 = atoi(v0);
        if ( Demo_ShouldProcessCmd(v1) )
        {
            Demo_SetCmdInProgess();
            Dvar_SetBool((dvar_s *)demo_pause, !demo_pause->current.enabled);
            Demo_SetCmdCompleted();
        }
        else
        {
            Com_PrintError(14, "A command is in progress. We cant process this command.\n");
        }
    }
    else
    {
        Com_PrintError(14, "We are not playing a demo.\n");
    }
}

void __cdecl Demo_TimeScale_f()
{
    const char *v0; // eax
    int v1; // eax
    const char *v2; // eax
    float timeScale; // [esp+14h] [ebp-4h]

    if ( SV_Cmd_Argc() == 3 )
    {
        if ( Demo_IsPlaying() )
        {
            v0 = Cmd_Argv(1);
            v1 = atoi(v0);
            if ( Demo_ShouldProcessCmd(v1) )
            {
                if ( Demo_IsCompleted() )
                {
                    Com_PrintError(14, "We cannot timescale in a demo that is completed.\n");
                }
                else if ( !Demo_IsClipPlaying() )
                {
                    if ( Demo_IsClipPreviewRunning() )
                    {
                        Com_PrintError(14, "We cannot timescale in a demo that is being previewed\n");
                    }
                    else
                    {
                        Demo_SetCmdInProgess();
                        v2 = Cmd_Argv(2);
                        timeScale = atof(v2);
                        Dvar_SetFloat((dvar_s *)demo_timescale, timeScale);
                        Com_SetTimeScale(timeScale);
                        Demo_SetCmdCompleted();
                    }
                }
            }
            else
            {
                Com_PrintError(14, "A command is in progress. We cant process this command.\n");
            }
        }
        else
        {
            Com_PrintError(14, "We are not playing a demo.\n");
        }
    }
    else
    {
        Com_PrintError(14, "Usage: demo_timescale <cmdNum> <timescale> .\n");
    }
}

void __cdecl Demo_ToggleGameHud_f()
{
    const char *v0; // eax
    int v1; // eax

    if ( SV_Cmd_Argc() == 2 )
    {
        if ( Demo_IsPlaying() )
        {
            v0 = Cmd_Argv(1);
            v1 = atoi(v0);
            if ( Demo_ShouldProcessCmd(v1) )
            {
                if ( !Demo_IsClipPlaying() )
                {
                    Demo_SetCmdInProgess();
                    demo.playback->hideGameHud = !demo.playback->hideGameHud;
                    Demo_SetCmdCompleted();
                }
            }
            else
            {
                Com_PrintError(14, "A command is in progress. We cant process this command.\n");
            }
        }
        else
        {
            Com_PrintError(14, "We are not playing a demo.\n");
        }
    }
    else
    {
        Com_Printf(0, "Usage: demo_togglegamehud <cmdNum>.");
    }
}

void __cdecl Demo_ToggleDemoHud_f()
{
    const char *v0; // eax
    int v1; // eax

    if ( SV_Cmd_Argc() == 2 )
    {
        if ( Demo_IsPlaying() )
        {
            v0 = Cmd_Argv(1);
            v1 = atoi(v0);
            if ( Demo_ShouldProcessCmd(v1) )
            {
                Demo_SetCmdInProgess();
                demo.playback->hideDemoHud = !demo.playback->hideDemoHud;
                Demo_SetCmdCompleted();
            }
            else
            {
                Com_PrintError(14, "A command is in progress. We cant process this command.\n");
            }
        }
        else
        {
            Com_PrintError(14, "We are not playing a demo.\n");
        }
    }
    else
    {
        Com_Printf(0, "Usage: demo_toggledemohud <cmdNum>.");
    }
}

void __cdecl Demo_SwitchPlayer_f()
{
    const char *v0; // eax
    int v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    char result; // [esp+1Bh] [ebp-Dh]
    int direction; // [esp+20h] [ebp-8h]
    int clientNum; // [esp+24h] [ebp-4h]

    clientNum = -1;
    if ( SV_Cmd_Argc() == 3 || SV_Cmd_Argc() == 4 )
    {
        if ( Demo_IsPlaying() )
        {
            v0 = Cmd_Argv(1);
            v1 = atoi(v0);
            if ( Demo_ShouldProcessCmd(v1) )
            {
                if ( Demo_IsMovieCamera() )
                {
                    Com_PrintError(14, "We cannot switch players when in free camera.\n");
                }
                else if ( Demo_IsCompleted() )
                {
                    Com_PrintError(14, "We cannot switch players when the demo is completed.\n");
                }
                else if ( !Demo_IsClipPlaying() )
                {
                    if ( Demo_IsClipPreviewRunning() )
                    {
                        Com_PrintError(14, "We cannot switch players in a demo that is being previewed\n");
                    }
                    else
                    {
                        Demo_SetCmdInProgess();
                        v2 = Cmd_Argv(2);
                        direction = atoi(v2);
                        if ( SV_Cmd_Argc() == 4 )
                        {
                            v3 = Cmd_Argv(3);
                            clientNum = atoi(v3);
                        }
                        result = Demo_SwitchPlayer(0, direction != 0, clientNum, 0);
                        if ( Demo_IsPaused() )
                        {
                            if ( result )
                                Demo_UpdateCurrentSnapshot(0, &demo.ps[demo_client->current.integer]);
                        }
                        Demo_SetCmdCompleted();
                    }
                }
            }
            else
            {
                Com_PrintError(14, "A command is in progress. We cant process this command.\n");
            }
        }
        else
        {
            Com_PrintError(14, "We are not playing a demo.\n");
        }
    }
    else
    {
        Com_Printf(
            0,
            "Usage: demo_switchplayer <cmdNum> <direction> [forcedClient]. <direction> is 0: clockwise 1: anticlockwise.");
    }
}

void __cdecl Demo_SwitchCamera_f()
{
    const char *v0; // eax
    int v1; // eax
    unsigned int cameraMode; // [esp+8h] [ebp-8h]

    if ( SV_Cmd_Argc() == 2 )
    {
        if ( Demo_IsPlaying() )
        {
            v0 = Cmd_Argv(1);
            v1 = atoi(v0);
            if ( Demo_ShouldProcessCmd(v1) )
            {
                if ( !Demo_IsClipPlaying() )
                {
                    if ( Demo_IsClipPreviewRunning() )
                    {
                        Com_PrintError(14, "We cannot switch camera in a demo that is being previewed\n");
                    }
                    else
                    {
                        Demo_SetCmdInProgess();
                        cameraMode = Demo_GetNextCameraMode(demo.playback->cameraMode);
                        Demo_SwitchCameraMode(0, cameraMode);
                        Demo_SetCmdCompleted();
                    }
                }
            }
            else
            {
                Com_PrintError(14, "A command is in progress. We cant process this command.\n");
            }
        }
        else
        {
            Com_PrintError(14, "We are not playing a demo.\n");
        }
    }
    else
    {
        Com_Printf(0, "Usage: demo_switchcamera <cmdNum>.");
    }
}

void __cdecl Demo_StartClipRecord_f()
{
    const char *v0; // eax
    int v1; // eax
    char *v2; // eax
    const char *v3; // eax
    char *v4; // eax
    const char *v5; // eax
    int v6; // [esp-8h] [ebp-1Ch]

    if ( SV_Cmd_Argc() == 2 )
    {
        if ( Demo_IsPlaying() )
        {
            if ( !Demo_GetClipState() || Demo_GetClipState() == 3 )
            {
                v0 = Cmd_Argv(1);
                v1 = atoi(v0);
                if ( Demo_ShouldProcessCmd(v1) )
                {
                    if ( Demo_IsCompleted() )
                    {
                        Com_PrintError(14, "We cannot start record a clip for a demo that is completed\n");
                    }
                    else if ( !Demo_IsClipPlaying() )
                    {
                        if ( Demo_IsClipPreviewRunning() )
                        {
                            Com_PrintError(14, "We cannot start a clip record in a demo that is being previewed\n");
                        }
                        else
                        {
                            if ( demo.playback->pendingSegmentSave
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_common.cpp",
                                            1501,
                                            0,
                                            "%s",
                                            "!demo.playback->pendingSegmentSave") )
                            {
                                __debugbreak();
                            }
                            if ( demo.playback->segmentCount == 10 )
                            {
                                Demo_Error(0, "MPUI_ERROR_CAPS", "MENU_DEMO_MAX_SEGMENTS_REACHED");
                            }
                            else if ( demo.playback->clipRecordBufIndex < 999424 )
                            {
                                Demo_SetCmdInProgess();
                                Dvar_SetInt((dvar_s *)fsSelectedFileTagIndex, 0);
                                demo.playback->prevClipRecordBufIndex = demo.playback->clipRecordBufIndex;
                                demo.playback->prevClipTime = demo.playback->clipTime;
                                CL_GetLocalClientGlobals(0);
                                Demo_InitClipRecord(0, demo.playback->clipRecordBufIndex == 0);
                                demo.playback->segments[demo.playback->segmentCount].clipBufferIndex = demo.playback->clipRecordBufIndex;
                                v6 = demo.playback->segmentCount + 1;
                                v2 = UI_SafeTranslateString("MENU_SEGMENT");
                                v3 = va("%s%04d", v2, v6);
                                I_strncpyz(demo.playback->segments[demo.playback->segmentCount].name, v3, 22);
                                demo.playback->segments[demo.playback->segmentCount].transition = 0;
                                demo.playback->defaultSegmentTag = 0;
                                v4 = UI_SafeTranslateString(demo_tags_enum_string_37[demo.playback->defaultSegmentTag]);
                                Dvar_SetString((dvar_s *)demo_defaultSegmentTag, v4);
                                Demo_ActivateTransitionScreen(1);
                                Demo_WriteDemoPreviewPoint(0, 0);
                                demo.playback->forceWriteClipCommands = 1;
                                demo.playback->pendingSegmentSave = 1;
                                Demo_WriteUncompressedClipSnapshot(0);
                                Demo_SetClipState(2);
                                v5 = va("Clip: Starting clip Recording\n");
                                Demo_Printf(1024, v5);
                                Demo_SetCmdCompleted();
                            }
                            else
                            {
                                Demo_Error(0, "MPUI_ERROR_CAPS", "MENU_DEMO_MAX_CLIP_FILESIZE_REACHED");
                            }
                        }
                    }
                }
                else
                {
                    Com_PrintError(14, "A command is in progress. We cant process this command.\n");
                }
            }
            else
            {
                Com_PrintError(14, "The clip system is doing some process. So, cant start record.\n");
            }
        }
        else
        {
            Com_PrintError(14, "We are not playing a demo.\n");
        }
    }
    else
    {
        Com_Printf(0, "Usage: demo_startcliprecord <cmdNum>.");
    }
}

void __cdecl Demo_PauseClipRecord_f()
{
    const char *v0; // eax
    int v1; // eax
    const char *v2; // eax

    if ( SV_Cmd_Argc() == 2 )
    {
        if ( Demo_IsPlaying() )
        {
            if ( Demo_GetClipState() == 1 || Demo_GetClipState() == 2 )
            {
                v0 = Cmd_Argv(1);
                v1 = atoi(v0);
                if ( Demo_ShouldProcessCmd(v1) )
                {
                    Demo_SetCmdInProgess();
                    Demo_SetClipState(3);
                    MSG_WriteBits(&demo.playback->clipRecordingMsg, 0, 2u);
                    Demo_SaveClipRecordMessageToMainMemory(
                        0,
                        demo.playback->clipRecordingMsg.data,
                        demo.playback->clipRecordingMsg.cursize,
                        0);
                    Demo_WriteDemoPreviewPoint(0, 1);
                    Dvar_SetBool((dvar_s *)demo_pause, 1);
                    Demo_OpenSavePopUp(0, 0);
                    v2 = va("Clip: Paused clip Recording\n");
                    Demo_Printf(1024, v2);
                    Demo_SetCmdCompleted();
                }
                else
                {
                    Com_PrintError(14, "A command is in progress. We cant process this command.\n");
                }
            }
            else
            {
                Com_PrintError(14, "The clip system is not recording.\n");
            }
        }
        else
        {
            Com_PrintError(14, "We are not playing a demo.\n");
        }
    }
    else
    {
        Com_Printf(0, "Usage: demo_pausecliprecord <cmdNum>.");
    }
}

void __cdecl Demo_PreviewClip_f()
{
    const char *v0; // eax
    int ControllerIndex; // eax
    char *v2; // [esp-4h] [ebp-8h]

    if ( SV_Cmd_Argc() == 1 )
    {
        if ( Demo_IsPlaying() )
        {
            Demo_WriteDemoPreviewPoint(0, 0);
            v0 = va("Clip: Starting clip previewing\n");
            Demo_Printf(1024, v0);
            demo.playback->previewAllSegments = 1;
            Demo_SetCompleted(0);
            v2 = va("demo_previewsegment %d 0", demo_cmdNum->current.integer);
            ControllerIndex = Com_LocalClient_GetControllerIndex(0);
            Cmd_ExecuteSingleCommand(0, ControllerIndex, v2);
            Demo_ActivateTransitionScreen(1);
        }
        else
        {
            Com_PrintError(14, "We are not playing a demo.\n");
        }
    }
    else
    {
        Com_Printf(0, "Usage: demo_previewclip\n");
    }
}

void __cdecl Demo_DeleteClip_f()
{
    const char *v0; // eax

    if ( SV_Cmd_Argc() == 1 )
    {
        if ( Demo_IsPlaying() )
        {
            v0 = va("Clip: Deleting clip\n");
            Demo_Printf(1024, v0);
            demo.playback->clipRecordBufIndex = 0;
            demo.playback->segmentCount = 0;
        }
        else
        {
            Com_PrintError(14, "We are not playing a demo.\n");
        }
    }
    else
    {
        Com_Printf(0, "Usage: demo_deleteclip\n");
    }
}

void __cdecl Demo_SaveAndUploadClip_f()
{
    const char *v0; // eax
    int v1; // eax
    const char *v2; // eax
    signed int slot; // [esp+10h] [ebp-8h]

    if ( SV_Cmd_Argc() == 3 )
    {
        if ( Demo_IsPlaybackInited() )
        {
            if ( Demo_GetClipState() )
            {
                v0 = Cmd_Argv(1);
                v1 = atoi(v0);
                if ( Demo_ShouldProcessCmd(v1) )
                {
                    if ( demo.playback->segmentCount )
                    {
                        if ( Demo_GetTotalClipDuration() >= 1000 )
                        {
                            Demo_SetCmdInProgess();
                            v2 = Cmd_Argv(2);
                            slot = atoi(v2);
                            if ( slot && slot <= 3 * Live_FileShare_GetMyPrivateSlotsCount() )
                                Demo_WriteRecordedClip(0, slot);
                            else
                                Com_PrintError(16, "Failed to upload clip. Invalid slot.\n");
                        }
                        else
                        {
                            Demo_Error(0, "MPUI_ERROR_CAPS", "MENU_DEMO_TOTAL_DURATION_LESS_THAN_ONE");
                        }
                    }
                    else
                    {
                        Com_PrintError(14, "Cant save a clip with 0 segments.\n");
                    }
                }
                else
                {
                    Com_PrintError(14, "A command is in progress. We cant process this command.\n");
                }
            }
            else
            {
                Com_PrintError(14, "The clip system is not recording.\n");
            }
        }
        else
        {
            Com_PrintError(14, "Playback memory isnt inited.\n");
        }
    }
    else
    {
        Com_Printf(0, "Usage: demo_saveanduploadclip <cmdNum> <slotNum>.");
    }
}

void __cdecl Demo_SaveSegment_f()
{
    const char *v0; // eax
    int v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    char *v5; // eax
    bool discard; // [esp+1Eh] [ebp-2h]
    bool useDefaultTag; // [esp+1Fh] [ebp-1h]

    useDefaultTag = 0;
    if ( SV_Cmd_Argc() == 3 || SV_Cmd_Argc() == 4 )
    {
        if ( Demo_IsPlaying() )
        {
            v0 = Cmd_Argv(1);
            v1 = atoi(v0);
            if ( Demo_ShouldProcessCmd(v1) )
            {
                Demo_SetCmdInProgess();
                v2 = Cmd_Argv(2);
                discard = atoi(v2) != 0;
                if ( SV_Cmd_Argc() == 4 )
                {
                    v3 = Cmd_Argv(3);
                    useDefaultTag = atoi(v3) != 0;
                }
                v4 = va("Clip: Saving clip\n");
                Demo_Printf(1024, v4);
                if ( discard )
                {
                    demo.playback->clipRecordBufIndex = demo.playback->prevClipRecordBufIndex;
                    demo.playback->clipTime = demo.playback->prevClipTime;
                    Demo_RestoreDemoPreviewPoint(0, 0);
                }
                else
                {
                    if ( useDefaultTag )
                    {
                        v5 = UI_SafeTranslateString(demo_tags_enum_string_37[demo.playback->defaultSegmentTag]);
                        I_strncpyz(demo.playback->segments[demo.playback->segmentCount].name, v5, 288);
                    }
                    demo.playback->segments[demo.playback->segmentCount].size = demo.playback->clipRecordBufIndex
                                                                                                                                        - demo.playback->segments[demo.playback->segmentCount].clipBufferIndex;
                    demo.playback->segments[demo.playback->segmentCount].duration = Demo_GetClipTime()
                                                                                                                                                - demo.playback->prevClipTime;
                    if ( demo.playback->segments[demo.playback->segmentCount].size <= 0
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_common.cpp",
                                    1796,
                                    0,
                                    "%s",
                                    "demo.playback->segments[demo.playback->segmentCount].size > 0") )
                    {
                        __debugbreak();
                    }
                    ++demo.playback->segmentCount;
                    Demo_SetClipModified(1);
                    if ( !Demo_IsCompleted() )
                        Demo_RestoreDemoPreviewPoint(0, 1);
                }
                Demo_ActivateTransitionScreen(1);
                demo.playback->pendingSegmentSave = 0;
                Demo_SetCmdCompleted();
            }
            else
            {
                Com_PrintError(14, "A command is in progress. We cant process this command.\n");
            }
        }
        else
        {
            Com_PrintError(14, "We are not playing a demo.\n");
        }
    }
    else
    {
        Com_Printf(0, "Usage: demo_savesegment <cmdNum> <discard> [useDefaultTag]");
    }
}

void __cdecl Demo_MoveSegment_f()
{
    const char *v0; // eax
    int v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    int newSegmentIndex; // [esp+24h] [ebp-134h]
    clipSegment tempSegment; // [esp+28h] [ebp-130h] BYREF
    bool direction; // [esp+153h] [ebp-5h]
    int segmentIndex; // [esp+154h] [ebp-4h]

    if ( SV_Cmd_Argc() == 4 )
    {
        if ( Demo_IsPlaybackInited() )
        {
            v0 = Cmd_Argv(1);
            v1 = atoi(v0);
            if ( Demo_ShouldProcessCmd(v1) )
            {
                v2 = Cmd_Argv(2);
                segmentIndex = atoi(v2);
                v3 = Cmd_Argv(3);
                direction = atoi(v3) != 0;
                if ( segmentIndex >= 0 && segmentIndex <= demo.playback->segmentCount - 1 )
                {
                    if ( (segmentIndex || direction) && (segmentIndex != demo.playback->segmentCount - 1 || !direction) )
                    {
                        Demo_SetCmdInProgess();
                        if ( direction )
                            newSegmentIndex = segmentIndex + 1;
                        else
                            newSegmentIndex = segmentIndex - 1;
                        if ( (newSegmentIndex < 0 || newSegmentIndex >= demo.playback->segmentCount)
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_common.cpp",
                                        1869,
                                        0,
                                        "%s",
                                        "newSegmentIndex >= 0 && newSegmentIndex < demo.playback->segmentCount") )
                        {
                            __debugbreak();
                        }
                        memcpy(&tempSegment, &demo.playback->segments[segmentIndex], sizeof(tempSegment));
                        memcpy(
                            &demo.playback->segments[segmentIndex],
                            &demo.playback->segments[newSegmentIndex],
                            sizeof(demo.playback->segments[segmentIndex]));
                        memcpy(
                            &demo.playback->segments[newSegmentIndex],
                            &tempSegment,
                            sizeof(demo.playback->segments[newSegmentIndex]));
                        Demo_SetClipModified(1);
                        v4 = va("Clip: Moved segment %d in direction %d\n", segmentIndex, direction);
                        Demo_Printf(3, v4);
                        Demo_SetCmdCompleted();
                    }
                    else
                    {
                        Com_PrintError(14, "Unspported direction %d for index %d\n", direction, segmentIndex);
                    }
                }
                else
                {
                    Com_PrintError(14, "Bad segment index %d\n", segmentIndex);
                }
            }
            else
            {
                Com_PrintError(14, "A command is in progress. We cant process this command.\n");
            }
        }
        else
        {
            Com_PrintError(14, "Playback memory isnt inited.\n");
        }
    }
    else
    {
        Com_Printf(0, "Usage: demo_movesegment <cmdNum> <segmentIndex> <direction>. <Direction> - 0 for up, 1 for down\n");
    }
}

void __cdecl Demo_DeleteSegment_f()
{
    const char *v0; // eax
    int v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    int deletedSegmentSize; // [esp+18h] [ebp-20h]
    int deletedSegmentBufferStartIndex; // [esp+1Ch] [ebp-1Ch]
    int deletedSegmentBufferEndIndex; // [esp+24h] [ebp-14h]
    int i; // [esp+2Ch] [ebp-Ch]
    int ia; // [esp+2Ch] [ebp-Ch]
    signed int sizeOfMemoryBlockToMove; // [esp+30h] [ebp-8h]
    int segmentIndex; // [esp+34h] [ebp-4h]

    if ( SV_Cmd_Argc() == 3 )
    {
        if ( Demo_IsPlaybackInited() )
        {
            v0 = Cmd_Argv(1);
            v1 = atoi(v0);
            if ( Demo_ShouldProcessCmd(v1) )
            {
                v2 = Cmd_Argv(2);
                segmentIndex = atoi(v2);
                if ( segmentIndex >= 0 && segmentIndex <= demo.playback->segmentCount - 1 )
                {
                    Demo_SetCmdInProgess();
                    deletedSegmentBufferStartIndex = demo.playback->segments[segmentIndex].clipBufferIndex;
                    deletedSegmentSize = demo.playback->segments[segmentIndex].size;
                    deletedSegmentBufferEndIndex = deletedSegmentSize + deletedSegmentBufferStartIndex;
                    for ( i = 0; i < demo.playback->segmentCount; ++i )
                    {
                        if ( i != segmentIndex && demo.playback->segments[i].clipBufferIndex >= deletedSegmentBufferEndIndex )
                            demo.playback->segments[i].clipBufferIndex -= deletedSegmentSize;
                    }
                    sizeOfMemoryBlockToMove = demo.playback->clipRecordBufIndex - deletedSegmentBufferEndIndex;
                    if ( sizeOfMemoryBlockToMove > 0 )
                        memmove(
                            &demo.playback->mainClipRecordingBuf[deletedSegmentBufferStartIndex],
                            &demo.playback->mainClipRecordingBuf[deletedSegmentBufferEndIndex],
                            sizeOfMemoryBlockToMove);
                    for ( ia = segmentIndex; ia < demo.playback->segmentCount; ++ia )
                    {
                        if ( ia + 1 >= demo.playback->segmentCount )
                            memset(&demo.playback->segments[ia], 0, sizeof(demo.playback->segments[ia]));
                        else
                            memcpy(
                                &demo.playback->segments[ia],
                                &demo.playback->segments[ia + 1],
                                sizeof(demo.playback->segments[ia]));
                    }
                    --demo.playback->segmentCount;
                    demo.playback->clipRecordBufIndex -= deletedSegmentSize;
                    if ( demo.playback->segmentCount )
                        Demo_SetClipModified(1);
                    v3 = va("Clip: Deleted segment %d \n", segmentIndex);
                    Demo_Printf(3, v3);
                    Demo_SetCmdCompleted();
                }
                else
                {
                    Com_PrintError(14, "Bad segment index %d\n", segmentIndex);
                }
            }
            else
            {
                Com_PrintError(14, "A command is in progress. We cant process this command.\n");
            }
        }
        else
        {
            Com_PrintError(14, "Playback memory isnt inited.\n");
        }
    }
    else
    {
        Com_Printf(0, "Usage: demo_movesegment <cmdNum> <segmentIndex>.\n");
    }
}

void __cdecl Demo_PreviewSegment_f()
{
    const char *v0; // eax
    int v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    clientActive_t *LocalClientGlobals; // [esp+10h] [ebp-18h]
    cg_s *cgameGlob; // [esp+14h] [ebp-14h]
    int segmentIndex; // [esp+24h] [ebp-4h]

    if ( SV_Cmd_Argc() != 2 && SV_Cmd_Argc() != 3 )
    {
        Com_Printf(0, "Usage: demo_previewsegment <cmdNum> [segmentIndex].\n");
        return;
    }
    if ( !Demo_IsPlaying() )
    {
        Com_PrintError(14, "We are not playing a demo.\n");
        return;
    }
    v0 = Cmd_Argv(1);
    v1 = atoi(v0);
    if ( !Demo_ShouldProcessCmd(v1) )
    {
        Com_PrintError(14, "A command is in progress. We cant process this command.\n");
        return;
    }
    if ( SV_Cmd_Argc() == 3 )
    {
        if ( demo.playback->pendingSegmentSave
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_common.cpp",
                        2019,
                        0,
                        "%s",
                        "!demo.playback->pendingSegmentSave") )
        {
            __debugbreak();
        }
        v2 = Cmd_Argv(2);
        segmentIndex = atoi(v2);
        if ( segmentIndex < 0 || segmentIndex >= demo.playback->segmentCount )
        {
            Com_PrintError(14, "Bad segment index specified.\n");
            return;
        }
        if ( !demo.playback->previewAllSegments )
            Demo_WriteDemoPreviewPoint(0, 0);
    }
    else
    {
        if ( !demo.playback->pendingSegmentSave
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_common.cpp",
                        2034,
                        0,
                        "%s",
                        "demo.playback->pendingSegmentSave") )
        {
            __debugbreak();
        }
        segmentIndex = -1;
    }
    demo.playback->clipPreviewSegmentNumber = segmentIndex;
    Demo_SetCmdInProgess();
    LocalClientGlobals = CL_GetLocalClientGlobals(0);
    CL_GetLocalClientConnection(0);
    CG_GetLocalClientStaticGlobals(0);
    cgameGlob = CG_GetLocalClientGlobals(0);
    v3 = va("Clip: Starting clip previewing\n");
    Demo_Printf(1024, v3);
    Dvar_SetBool((dvar_s *)demo_pause, 0);
    demo.playback->clipPausedState = 0;
    if ( !demo.playback->previewAllSegments )
        Demo_SetCompleted(0);
    if ( segmentIndex < 0 )
        demo.playback->clipPreviewBufferIndex = demo.playback->segments[demo.playback->segmentCount].clipBufferIndex;
    else
        demo.playback->clipPreviewBufferIndex = demo.playback->segments[segmentIndex].clipBufferIndex;
    Demo_SetClipState(4);
    if ( demo.playback->clipRecordBufIndex == demo.playback->clipPreviewBufferIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_common.cpp",
                    2068,
                    0,
                    "%s",
                    "demo.playback->clipRecordBufIndex != demo.playback->clipPreviewBufferIndex") )
    {
        __debugbreak();
    }
    Demo_Load();
    Demo_ResetSnapshotData();
    Demo_ReadDemoMessage(0);
    Demo_ResetTimeValues(0, LocalClientGlobals->snap.serverTime);
    Demo_ResetSequenceNumbers(0, LocalClientGlobals->snap.messageNum - 1);
    Demo_ResetWorldInformation(0, LocalClientGlobals->snap.serverTime);
    cgameGlob->latestSnapshotNum = LocalClientGlobals->snap.messageNum;
    LocalClientGlobals->serverTimeDelta = LocalClientGlobals->snap.serverTime - cls.realtime;
    if ( !demo.playback->previewAllSegments )
        Demo_ActivateTransitionScreen(1);
    Demo_SetCmdCompleted();
}

void __cdecl Demo_SwitchDefaultSegmentTag_f()
{
    char *v0; // eax

    if ( SV_Cmd_Argc() == 2 )
    {
        if ( Demo_IsPlaybackInited() )
        {
            if ( Key_IsDown(0, 201) )
            {
                if ( demo.playback->defaultSegmentTag )
                    --demo.playback->defaultSegmentTag;
                else
                    demo.playback->defaultSegmentTag = 7;
            }
            else if ( demo.playback->defaultSegmentTag == 7 )
            {
                demo.playback->defaultSegmentTag = 0;
            }
            else
            {
                ++demo.playback->defaultSegmentTag;
            }
            v0 = UI_SafeTranslateString(demo_tags_enum_string_37[demo.playback->defaultSegmentTag]);
            Dvar_SetString((dvar_s *)demo_defaultSegmentTag, v0);
        }
        else
        {
            Com_Printf(0, "Error: Demo Playback Buffer is not inited.\n");
        }
    }
    else
    {
        Com_Printf(0, "Usage: demo_switchtransition <direction>.");
    }
}

void __cdecl Demo_SwitchTransition_f()
{
    const char *v0; // eax
    const char *v1; // eax
    int index; // [esp+10h] [ebp-8h]
    bool direction; // [esp+17h] [ebp-1h]

    if ( SV_Cmd_Argc() == 3 )
    {
        if ( Demo_IsPlaybackInited() )
        {
            v0 = Cmd_Argv(1);
            index = atoi(v0);
            if ( index == demo.playback->segmentCount - 1 )
            {
                Com_Printf(0, "Cant switch transitions for the last segment\n");
            }
            else
            {
                v1 = Cmd_Argv(2);
                direction = atoi(v1) != 0;
                Demo_SwitchTransition(index, direction);
                Demo_SetClipModified(1);
            }
        }
        else
        {
            Com_Printf(0, "Error: Demo Playback Buffer is not inited.\n");
        }
    }
    else
    {
        Com_Printf(0, "Usage: demo_switchtransition <index> <direction>.");
    }
}

void __cdecl Demo_Keyboard_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // [esp-4h] [ebp-20h]
    int i; // [esp+14h] [ebp-8h]

    if ( SV_Cmd_Argc() == 2 || SV_Cmd_Argc() == 3 )
    {
        for ( i = 0; i < 8; ++i )
        {
            v3 = demoKeyboardModeNames_69[i];
            v0 = Cmd_Argv(1);
            if ( !I_stricmp(v0, v3) )
                demo.keyboard.mode = i;
        }
        if ( demo.keyboard.mode > 0 && demo.keyboard.mode < 8 )
        {
            switch ( demo.keyboard.mode )
            {
                case 1:
                    if ( Demo_IsPlaybackInited() )
                    {
                        if ( SV_Cmd_Argc() == 3 )
                        {
                            v1 = Cmd_Argv(2);
                            demo.keyboard.currentProcessingSegmentIndex = atoi(v1);
                        }
                        else
                        {
                            demo.keyboard.currentProcessingSegmentIndex = demo.playback->segmentCount;
                        }
                        demo.keyboard.title = UI_SafeTranslateString("MENU_SEGMENT");
                        demo.keyboard.defaultText = "";
                        demo.keyboard.textSize = 22;
                        goto LABEL_28;
                    }
                    break;
                case 2:
                    if ( Demo_IsPlaybackInited() )
                        goto LABEL_19;
                    break;
                case 3:
                    if ( Demo_IsPlaybackInited() )
                    {
LABEL_19:
                        demo.keyboard.title = UI_SafeTranslateString("MENU_CLIP");
                        demo.keyboard.defaultText = "";
                        demo.keyboard.textSize = 22;
                        goto LABEL_28;
                    }
                    break;
                case 4:
                    if ( Demo_IsPlaybackInited() )
                        goto LABEL_23;
                    break;
                case 5:
                    if ( Demo_IsPlaybackInited() )
                    {
LABEL_23:
                        demo.keyboard.title = UI_SafeTranslateString("MENU_SCREENSHOT");
                        demo.keyboard.defaultText = "";
                        demo.keyboard.textSize = 22;
                        goto LABEL_28;
                    }
                    break;
                case 6:
                    demo.keyboard.title = UI_SafeTranslateString("MENU_FILESHARE_NEWNAME");
                    demo.keyboard.defaultText = "";
                    demo.keyboard.textSize = 22;
                    goto LABEL_28;
                case 7:
                    demo.keyboard.title = UI_SafeTranslateString("MENU_FILESHARE_NEWDESCRIPTION");
                    demo.keyboard.defaultText = "";
                    demo.keyboard.textSize = 22;
LABEL_28:
                    v2 = va("ui_keyboard_new %d\n", 1);
                    Cbuf_AddText(0, v2);
                    break;
                default:
                    return;
            }
        }
        else
        {
            Com_Printf(0, "Error: Unknown demo keyboard mode.\n");
        }
    }
    else
    {
        Com_Printf(0, "Usage: demo_keyboard <type> [segmentIndex]\n");
    }
}

void    Demo_Screenshot_f()
{
    const char *v1; // eax
    int tiles; // [esp+10h] [ebp-2Ch]
    qtime_s systemTime; // [esp+14h] [ebp-28h] BYREF
    int localClientNum; // [esp+38h] [ebp-4h]

    if ( SV_Cmd_Argc() == 2 )
    {
        if ( Demo_IsPlaying() )
        {
            localClientNum = 0;
            v1 = Cmd_Argv(1);
            tiles = atoi(v1);
            I_strncpyz(demo.playback->screenshotInfo.name, demo.info.name, 22);
            demo.playback->screenshotInfo.isModifiedName = 0;
            I_strncpyz(demo.playback->screenshotInfo.description, demo.info.description, 22);
            demo.playback->screenshotInfo.isModifiedDescription = 0;
            I_strncpyz(demo.playback->screenshotInfo.gameType, demo.info.gameType, 256);
            I_strncpyz(demo.playback->screenshotInfo.mapName, demo.info.mapName, 256);
            Dvar_SetInt((dvar_s *)fsSelectedFileTagIndex, 0);
            Com_RealTime(&systemTime, 1);
            demo.playback->screenshotInfo.week = systemTime.tm_yday / 7;
            demo.playback->screenshotInfo.month = systemTime.tm_mon + 1;
            demo.playback->screenshotInfo.year = systemTime.tm_year + 1900;
            demo.playback->screenshotInfo.day = systemTime.tm_mday;
            demo.playback->screenshotInfo.hour = systemTime.tm_hour;
            demo.playback->screenshotInfo.min = systemTime.tm_min;
            demo.playback->screenshotInfo.createTime = _time64(0);
            Demo_HiResScreenshot(localClientNum, tiles);
        }
        else
        {
            Com_PrintError(14, "The demo system not playing a demo. So, cannot take a screenshot\n");
        }
    }
    else
    {
        Com_Printf(0, "Usage: demo_screenshot <tiles>.");
    }
}

void __cdecl Demo_SaveScreenshot_f()
{
    const char *v0; // eax
    const char *v1; // eax
    int slot; // [esp+10h] [ebp-Ch]
    bool discard; // [esp+1Bh] [ebp-1h]

    slot = 0;
    if ( SV_Cmd_Argc() == 2 || SV_Cmd_Argc() == 3 )
    {
        if ( Demo_IsPlaying() )
        {
            v0 = Cmd_Argv(1);
            discard = atoi(v0) != 0;
            if ( SV_Cmd_Argc() == 3 )
            {
                if ( discard
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_common.cpp", 2368, 0, "%s", "!discard") )
                {
                    __debugbreak();
                }
                v1 = Cmd_Argv(2);
                slot = atoi(v1);
            }
            if ( !discard )
                Demo_SaveScreenshot(0, slot);
            demo.playback->screenshotSize = 0;
        }
        else
        {
            Com_PrintError(14, "The demo system not playing a demo. So, cannot take a screenshot\n");
        }
    }
    else
    {
        Com_Printf(0, "Usage: demo_savescreenshot <discard> [slot].");
    }
}

void __cdecl Demo_RenderMovie_f()
{
    const char *v0; // eax
    const char *v1; // eax

    if ( SV_Cmd_Argc() == 2 )
    {
        if ( Demo_IsIdle() )
        {
            LiveCounter_IncrementCounterValueByName("global_fileshare_rendered", 1u);
            v0 = Cmd_Argv(1);
            v1 = va("demo_play %s 1\n", v0);
            Cbuf_AddText(0, v1);
        }
        else
        {
            Com_PrintError(14, "The demo system is currently running.\n");
        }
    }
    else
    {
        Com_Printf(0, "Usage: demo_rendermovie <demoName>.");
    }
}

void __cdecl Demo_SwitchControls_f()
{
    const char *v0; // eax
    unsigned int currentConfig; // [esp+8h] [ebp-4h]
    int currentConfiga; // [esp+8h] [ebp-4h]

    if ( Demo_IsPlaying() )
    {
        if ( SV_Cmd_Argc() == 2 )
        {
            v0 = Cmd_Argv(1);
            currentConfig = atoi(v0);
            if ( currentConfig > 2
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_common.cpp",
                            2426,
                            0,
                            "%s",
                            "currentConfig >= 0 && currentConfig < DEMO_CONTROLLER_CONFIG_COUNT") )
            {
                __debugbreak();
            }
            Dvar_SetInt((dvar_s *)demo_controllerConfig, currentConfig);
        }
        else
        {
            currentConfiga = Demo_GetCurrentControllerConfig();
            if ( currentConfiga == 2 )
                Dvar_SetInt((dvar_s *)demo_controllerConfig, 0);
            else
                Dvar_SetInt((dvar_s *)demo_controllerConfig, currentConfiga + 1);
        }
    }
    else
    {
        Com_PrintError(14, "The demo system not playing.\n");
    }
}

bool __cdecl Demo_IsEnabled()
{
    if ( G_ExitAfterToolComplete() )
        return 0;
    if ( !onlinegame->current.enabled && !demo_usefilesystem->current.enabled )
        return 0;
    if ( Com_GameMode_IsGameMode(GAMEMODE_BASIC_TRAINING) )
    {
        return demo_enabled->current.enabled && demo_recordBasicTraining->current.enabled;
    }
    else if ( Com_GameMode_IsGameMode(GAMEMODE_FIRST_PRIVATE_ONLINE_GAMEMODE) )
    {
        return demo_enabled->current.enabled && demo_recordPrivateMatch->current.enabled;
    }
    else
    {
        return demo_enabled->current.enabled;
    }
}

void __cdecl Demo_SetDemoState(int state)
{
    demo.state = state;
}

int __cdecl Demo_GetDemoState()
{
    return demo.state;
}

void __cdecl Demo_SetDemoClientState(int state)
{
    demoClient.state = state;
}

int __cdecl Demo_GetDemoClientState()
{
    return demoClient.state;
}

bool __cdecl Demo_IsIdle()
{
    return Demo_GetDemoState() == 0;
}

bool __cdecl Demo_IsRecording()
{
    return Demo_GetDemoState() == 1;
}

bool __cdecl Demo_IsPlaying()
{
    return Demo_GetDemoState() == 2;
}

demoMain *__cdecl Demo_GetDemoName()
{
    return &demo;
}

void __cdecl Demo_Printf(int channel, const char *string)
{
    if ( (channel & demo_debug->current.integer) != 0 )
        Com_Printf(15, string);
}

void __cdecl Demo_Init()
{
    memset(&demo, 0, sizeof(demo));
    g_snapInfo.clientNum = 0;
    g_snapInfo.client = 0;
    g_snapInfo.snapshotDeltaTime = 0;
    *(unsigned int *)&g_snapInfo.fromBaseline = 0;
    g_snapInfo.packetEntityType = ANALYZE_DATATYPE_ENTITYTYPE_GENERALENTITY;
    *(unsigned int *)&g_snapInfo.packetEntityTypeArchived = 0;
    memset(&g_defaultPlayerState, 0, sizeof(g_defaultPlayerState));
    memset(&g_defaultEntityState, 0xFFu, sizeof(g_defaultEntityState));
    memset(&g_defaultClientState, 0xFFu, sizeof(g_defaultClientState));
    MSG_Init(&demo.msg, demo.msgBuf0, 49152);
    memset(demo.ent, 0xFFu, sizeof(demo.ent));
    memset(demo.client, 0xFFu, sizeof(demo.client));
    g_snapInfo.demoSnapshot = 1;
    demo.lastProcessedTime = -1;
    demo.lastProcessedMsgNum = -1;
}

bool __cdecl Demo_InitWrite()
{
    const char *v0; // eax
    char ospath[260]; // [esp+0h] [ebp-108h] BYREF

    memset(ospath, 0, 0x100u);
    Demo_GetDemoPath(ospath);
    v0 = va("%s.demo", demo.demoName);
    demo.demoFileHandle = Demo_OpenFileWrite(v0, ospath, 0);
    return demo.demoFileHandle != 0;
}

bool __cdecl Demo_InitRead()
{
    const char *v0; // eax
    char ospath[260]; // [esp+0h] [ebp-108h] BYREF

    memset(ospath, 0, 0x100u);
    Demo_GetDemoPath(ospath);
    v0 = va("%s.demo", demo.demoName);
    demo.demoFileHandle = Demo_OpenFileRead(v0, ospath, 0);
    return demo.demoFileHandle != 0;
}

void __cdecl Demo_Frame(int msec)
{
    int localClientNum; // [esp+0h] [ebp-4h]

    if ( Demo_IsPlaying() )
    {
        localClientNum = Com_LocalClients_GetPrimary();
        Demo_UpdateClipTime(msec);
        if ( Demo_IsPlaybackInited()
            && demo.playback->holdCmd.active
            && (int)(Sys_Milliseconds() - demo.playback->holdCmd.activatedTime) >= 250 )
        {
            Demo_HandleHoldInput(localClientNum, demo.playback->holdCmd.key, 1);
        }
        Demo_DrawDebugInformation(localClientNum);
    }
}

extern jqWorkerCmd Demo_SaveWorkerCmd;
void __cdecl Demo_End(bool abnormalTermination)
{
    unsigned __int8 *ActiveMemoryBuffer; // eax
    int cursize; // [esp-8h] [ebp-Ch]
    int state; // [esp+0h] [ebp-4h]

    state = Demo_GetDemoState();
    demo.shutdownInProgress = 1;
    if ( state == 1 )
    {
        MSG_WriteBits(&demo.msg, 1, 2u);
        demo.info.endTime = svs.time;
        cursize = demo.msg.cursize;
        ActiveMemoryBuffer = Demo_GetActiveMemoryBuffer();
        Demo_Save(ActiveMemoryBuffer, cursize, 1);
        Sys_WaitWorkerCmdInternal(&Demo_SaveWorkerCmd);
        Cbuf_AddText(0, "wait 10; demo_removedemoclient\n");
    }
    Demo_SetDemoClientState(0);
    Demo_PrintProfileData();
    Demo_StopStreaming();
    Demo_CloseFile(demo.demoFileHandle);
    Demo_SetDemoState(0);
    g_democlientindex = 0;
    if ( state == 2 )
    {
        if ( mjpeg_is_encoding() )
        {
            mjpeg_close();
            if ( abnormalTermination )
                demo.renderingMovie = 0;
        }
        Demo_SetClipState(0);
        Dvar_SetStringByName("ui_lobbypopup", (char*)"none");
        Demo_RestorePreDemoSettings();
        Com_Shutdown("EXE_DEMOFINISHED");
    }
    demo.shutdownInProgress = 0;
    demo.playbackInited = 0;
    num_heli_height_lock_patches = 0;
}

void __cdecl Demo_ReadDataCallback(char *data, unsigned int dataSize)
{
    Demo_Write(data, dataSize, s_demoFileHandle);
}

void __cdecl Demo_ReadDataSuccessPlayIt(dwFileShareReadFileTask *task)
{
    const char *v1; // eax

    Com_Printf(16, "Demo download successful.\n");
    v1 = va("demo_play %s 0\n", task->descriptor.m_fileName);
    Cbuf_AddText(0, v1);
}

void __cdecl Demo_ReadDataSuccessRenderIt(dwFileShareReadFileTask *task)
{
    const char *v1; // eax

    Com_Printf(16, "Demo download successful.\n");
    v1 = va("demo_rendermovie %s\n", task->descriptor.m_fileName);
    Cbuf_AddText(0, v1);
}

void __cdecl Demo_ReadDataFailure()
{
    Com_Printf(16, "Demo download failed.\n");
    if ( Menu_IsMenuOpenAndVisible(0, "popup_downloadingfile") )
    {
        UI_CloseMenuImmediate(0, "popup_downloadingfile");
        UI_OpenMenu(0, "menu_fileshare_error");
    }
}

void __cdecl Demo_DownloadFile(
                int controllerIndex,
                char *filmName,
                unsigned __int64 fileId,
                int fileSize,
                bool isUserFile,
                bool renderIt)
{
    char ospath[260]; // [esp+0h] [ebp-138h] BYREF
    fileShareReadFileInfo fileInfo; // [esp+108h] [ebp-30h] BYREF

    memset(ospath, 0, 0x100u);
    Demo_GetDemoPath(ospath);
    s_demoFileHandle = Demo_OpenFileWrite(filmName, ospath, 0);
    if ( Demo_IsStreamBufferAllocated() )
    {
        Demo_GetCurrentAllocatedFileSize();
        Demo_ReturnStreamBufferMemory();
    }
    Demo_AllocateMemoryFromStreamBuffer(fileSize);
    if ( isUserFile )
        fileInfo.location = FILESHARE_LOCATION_USERSTORAGE;
    else
        fileInfo.location = FILESHARE_LOCATION_POOLEDSTORAGE;
    fileInfo.fileID = fileId;
    fileInfo.fileSize = fileSize;
    fileInfo.isStreamed = 1;
    fileInfo.dataCallback = (void (__cdecl *)(void *, unsigned int, unsigned int, unsigned int))Demo_ReadDataCallback;
    if ( renderIt )
        fileInfo.successCallback = Demo_ReadDataSuccessRenderIt;
    else
        fileInfo.successCallback = Demo_ReadDataSuccessPlayIt;
    fileInfo.failureCallback = (void (__cdecl *)(dwFileShareReadFileTask *))Demo_ReadDataFailure;
    fileInfo.cacheBuffer = Demo_GetStreamAllocatedBuffer();
    LiveStorage_FileShare_ReadFile(controllerIndex, &fileInfo);
}

//void __thiscall demoRecordedFileUploadInfo::~demoRecordedFileUploadInfo(demoRecordedFileUploadInfo *this)
//{
//    int v1; // [esp+4h] [ebp-8h]
//    int *i; // [esp+8h] [ebp-4h]
//
//    v1 = 40;
//    for ( i = &this->numTags; --v1 >= 0; bdTag::~bdTag((bdTaskResult *)i) )
//        i -= 6;
//}
//
