#include "demo_playback.h"
#include "demo_common.h"
#include <universal/physicalmemory.h>
#include "demo_ui.h"
#include <live/live_win.h>
#include <live/live_fileshare.h>
#include <cgame_mp/cg_local_mp.h>
#include <win32/win_shared.h>
#include "demo_files.h"
#include <client/client.h>
#include <gfx_d3d/r_dvars.h>
#include <client_mp/cl_scrn_mp.h>
#include <universal/com_memory.h>
#include <mjpeg/yuv.h>
#include <win32/win_net.h>
#include <cgame/cg_draw_names.h>
#include <qcommon/com_clients.h>
#include <DW/dwRecordEvent.h>
#include <cgame_mp/cg_ui_animate_mp.h>
#include <cgame_mp/cg_newDraw_mp.h>
#include <client/cl_keys.h>
#include "demo_recording.h"
#include <client_mp/cl_cgame_mp.h>
#include <cgame/cg_camerashake.h>
#include <gfx_d3d/r_dpvs.h>
#include <client/cl_console.h>
#include <ik/ik.h>
#include <cgame_mp/cg_scr_main_mp.h>
#include <clientscript/cscr_vm.h>
#include <cgame_mp/cg_consolecmds_mp.h>
#include <EffectsCore/fx_marks.h>
#include "demo_profile.h"
#include <glass/glass_client.h>
#include <universal/com_files.h>
#include <universal/com_files.h>
#include <client_mp/cl_parse_mp.h>
#include <universal/q_parse.h>
#include <server_mp/sv_main_mp.h>
#include <qcommon/msg.h>
#include <qcommon/sv_msg_write.h>
#include <cgame_mp/cg_servercmds_mp.h>
#include <cgame/cg_main.h>
#include <cgame_mp/cg_snapshot_mp.h>
#include <game_mp/g_main_mp.h>
#include <qcommon/dobj_management.h>
#include <cgame/cg_scr_main.h>
#include <universal/com_constantconfigstrings.h>
#include <qcommon/dvar_cmds.h>
#include <cgame/cg_drawtools.h>
#include <ui_mp/ui_main_mp.h>
#include <physics/rope_gamestate.h>
#include <ui_mp/ui_gametype_custom_mp.h>
#include <live/live_counter.h>
#include <sound/snd_public_async.h>
#include <win32/win_gamepad.h>
#include <game/g_scr_helicopter.h>

unsigned __int8 *g_JpegBuf;
char gamerTag_1[32];
streamingState_s streamingState;

void __cdecl Demo_AllocatePlaybackMemory(unsigned int location)
{
    if ( Demo_IsPlaying() )
    {
        PMem_BeginAlloc("SnapshotDemoPlaybackBuffer", location, TRACK_MISC);
        demo.playback = (demoPlayback *)_PMem_AllocNamed(
                                                                            0x12DEE0u,
                                                                            4u,
                                                                            4,
                                                                            location,
                                                                            "SnapshotDemoPlaybackBuffer",
                                                                            TRACK_MISC);
        PMem_EndAlloc("SnapshotDemoPlaybackBuffer", location);
        PMem_BeginAlloc("SnapshotDemoKeyframeBuffer", location, TRACK_MISC);
        g_keyframeBuf = _PMem_AllocNamed(
            0x500000u,
                                            4u,
                                            4,
                                            location,
                                            "SnapshotDemoKeyframeBuffer",
                                            TRACK_MISC);
        PMem_EndAlloc("SnapshotDemoKeyframeBuffer", location);
        PMem_BeginAlloc("SnapshotDemoJpegSaveBuffer", location, TRACK_MISC);
        g_JpegBuf = _PMem_AllocNamed(
            0x500000u,
                                    0x10u,
                                    4,
                                    location,
                                    "SnapshotDemoJpegSaveBuffer",
                                    TRACK_MISC);
        PMem_EndAlloc("SnapshotDemoJpegSaveBuffer", location);
    }
}

void __cdecl Demo_DeallocatePlaybackMemory()
{
    if ( g_JpegBuf )
    {
        PMem_Free("SnapshotDemoJpegSaveBuffer");
        g_JpegBuf = 0;
    }
    if ( g_keyframeBuf )
    {
        PMem_Free("SnapshotDemoKeyframeBuffer");
        g_keyframeBuf = 0;
    }
    if ( demo.playback )
    {
        PMem_Free("SnapshotDemoPlaybackBuffer");
        demo.playback = 0;
    }
}

void __cdecl Demo_Load()
{
    const char *v0; // eax
    unsigned int bytesRead; // [esp+0h] [ebp-4h]
    unsigned int bytesReada; // [esp+0h] [ebp-4h]
    unsigned int bytesReadb; // [esp+0h] [ebp-4h]

    bytesRead = 0;
    memset(demo.memBlock.compressedMsgBuf, 0, sizeof(demo.memBlock.compressedMsgBuf));
    MSG_Init(&demo.msg, demo.msgBuf0, 49152);
    if ( Demo_GetClipState() == 4 )
    {
        demo.memBlock.compressedSize = *(unsigned int *)&demo.playback->mainClipRecordingBuf[demo.playback->clipPreviewBufferIndex];
        demo.playback->clipPreviewBufferIndex += 4;
        demo.memBlock.uncompressedSize = *(unsigned int *)&demo.playback->mainClipRecordingBuf[demo.playback->clipPreviewBufferIndex];
        demo.playback->clipPreviewBufferIndex += 4;
        memcpy(
            demo.memBlock.compressedMsgBuf,
            &demo.playback->mainClipRecordingBuf[demo.playback->clipPreviewBufferIndex],
            demo.memBlock.compressedSize);
        demo.playback->clipPreviewBufferIndex += demo.memBlock.compressedSize;
    }
    else
    {
        bytesReada = Demo_Read((unsigned __int8 *)&demo.memBlock, 4u, demo.demoFileHandle);
        bytesReadb = bytesReada + Demo_Read((unsigned __int8 *)&demo.memBlock.uncompressedSize, 4u, demo.demoFileHandle);
        bytesRead = bytesReadb
                            + Demo_Read(demo.memBlock.compressedMsgBuf, demo.memBlock.compressedSize, demo.demoFileHandle);
        demo.totalSize += bytesRead;
    }
    demo.msg.cursize = MSG_DecompressWithZLib(
                                             demo.memBlock.compressedMsgBuf,
                                             demo.memBlock.compressedSize,
                                             demo.msgBuf0,
                                             demo.memBlock.uncompressedSize);
    demo.msg.data = demo.msgBuf0;
    v0 = va(
                 "DEMO: CurSize: %d CompressedSize: %d. Read %d bytes to demo file %s\n",
                 demo.memBlock.uncompressedSize,
                 demo.memBlock.compressedSize,
                 bytesRead + 8,
                 demo.demoName);
    Demo_Printf(2, v0);
}

void    Demo_HiResScreenshot(int localClientNum, int tiles)
{
#if 0 // KISAKTODO: jpeg bits
    const char *v3; // eax
    int j; // [esp+18h] [ebp-250h]
    int row; // [esp+1Ch] [ebp-24Ch]
    float column; // [esp+20h] [ebp-248h]
    int y; // [esp+24h] [ebp-244h]
    int i; // [esp+28h] [ebp-240h]
    jpeg_exif exif; // [esp+2Ch] [ebp-23Ch] BYREF
    int total; // [esp+3Ch] [ebp-22Ch]
    int now; // [esp+40h] [ebp-228h]
    int iterations; // [esp+44h] [ebp-224h]
    const clientActive_t *LocalClientGlobals; // [esp+48h] [ebp-220h]
    bool streamFreezeState; // [esp+4Fh] [ebp-219h]
    int worstImageQuality; // [esp+50h] [ebp-218h]
    unsigned __int8 *out; // [esp+54h] [ebp-214h]
    int curlod; // [esp+58h] [ebp-210h]
    float yAspect; // [esp+5Ch] [ebp-20Ch]
    cg_s *cgameGlob; // [esp+60h] [ebp-208h]
    int width; // [esp+64h] [ebp-204h]
    int height; // [esp+68h] [ebp-200h]
    float fraction; // [esp+6Ch] [ebp-1FCh]
    unsigned __int8 *tempV; // [esp+70h] [ebp-1F8h]
    int bestImageQuality; // [esp+74h] [ebp-1F4h]
    unsigned __int8 *tempU; // [esp+78h] [ebp-1F0h]
    int maxIterations; // [esp+7Ch] [ebp-1ECh]
    unsigned __int8 *tempY; // [esp+80h] [ebp-1E8h]
    bool hideDemoHud; // [esp+87h] [ebp-1E1h]
    jpeg_encode encode; // [esp+88h] [ebp-1E0h] BYREF
    bool screenshotSuccessful; // [esp+263h] [ebp-5h]
    int imageQuality; // [esp+264h] [ebp-4h]

    maxIterations = 3;
    worstImageQuality = 2048;
    bestImageQuality = 256;
    screenshotSuccessful = 0;
    if ( !tiles
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp", 359, 0, "%s", "tiles != 0") )
    {
        __debugbreak();
    }
    if ( tiles )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
        curlod = r_forceLod->current.integer;
        Dvar_SetInt((dvar_s *)r_forceLod, 4);
        streamFreezeState = r_streamFreezeState->current.enabled;
        Dvar_SetBool((dvar_s *)r_streamFreezeState, 1);
        hideDemoHud = demo.playback->hideDemoHud;
        demo.playback->hideDemoHud = 1;
        CL_UpdateUIVisibilityBits(localClientNum);
        width = cls.vidConfig.displayWidth;
        height = cls.vidConfig.displayHeight;
        yAspect = 1.0f;
        if ( (!cls.vidConfig.displayHeight || !width)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                        380,
                        0,
                        "%s",
                        "height != 0 && width != 0") )
        {
            __debugbreak();
        }
        if ( height && width )
        {
            if ( tiles * width >= 0x10000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                            394,
                            0,
                            "%s",
                            "width*tiles < (1<<16)") )
            {
                __debugbreak();
            }
            imageQuality = bestImageQuality;
            tempY = (unsigned __int8 *)Z_VirtualAlloc(tiles * tiles * (height + 16) * (width + 16), "YUV temp", 23);
            tempU = (unsigned __int8 *)Z_VirtualAlloc(tiles * tiles * (height + 16) * (width + 16) / 4, "YUV temp", 23);
            tempV = (unsigned __int8 *)Z_VirtualAlloc(tiles * tiles * (height + 16) * (width + 16) / 4, "YUV temp", 23);
            yuv_init(width, height, 0);
            for ( iterations = 0; iterations < maxIterations; ++iterations )
            {
                demo.playback->screenshotSize = 0;
                for ( i = 0; i < 2; ++i )
                    SCR_UpdateScreen();
                NET_Sleep(200);
                R_BeginHiResShot(tiles);
                exif.gpsLat = 34.021542f;
                exif.gpsLon = -118.44875;
                exif.gpsOffsetLat = 0.0;
                exif.gpsOffsetLon = 0.0;
                exif.gpsOffsetLon = cgameGlob->refdef.vieworg[0] * 0.0254;
                exif.gpsOffsetLat = cgameGlob->refdef.vieworg[1] * 0.0254;
                out = jpeg_encode::begin(
                                &encode,
                                &g_JpegBuf[demo.playback->screenshotSize],
                                (int)(float)((float)(tiles * width) * yAspect),
                                tiles * height,
                                imageQuality,
                                1,
                                &exif);
                demo.playback->screenshotSize += out - &g_JpegBuf[demo.playback->screenshotSize];
                if ( demo.playback->screenshotSize >= (int)&loc_4FFFFE + 2
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                                427,
                                0,
                                "%s",
                                "demo.playback->screenshotSize < DEMO_JPEG_SAVE_BUFFER_SIZE") )
                {
                    __debugbreak();
                }
                out = &g_JpegBuf[demo.playback->screenshotSize];
                total = tiles * tiles * (height / 16);
                now = 0;
                for ( y = 0; y < tiles; ++y )
                {
                    column = (float)y;
                    for ( row = 0; row < tiles; ++row )
                    {
                        jpeg_screenshot_set_frame_info(
                            width,
                            height,
                            row,
                            y,
                            tiles,
                            COERCE_VOID_((float)row),
                            column,
                            0,
                            SLODWORD(yAspect),
                            COERCE_INT((float)now++ / (float)total));
                        R_UpdateHiResShot((float)row, column);
                        while ( Sys_RendererReady() )
                            ;
                        SCR_UpdateScreen(a1);
                        while ( Sys_RendererReady() )
                            ;
                        jpeg_screenshot_tile(tempY, tempU, tempV);
                    }
                }
                for ( j = 0; j < tiles * height; j += 16 )
                {
                    out = jpeg_screenshot_row(out, tempY, tempU, tempV, j, &encode);
                    if ( out != (unsigned __int8 *)g_JpegBuf[demo.playback->screenshotSize] )
                    {
                        demo.playback->screenshotSize += out - &g_JpegBuf[demo.playback->screenshotSize];
                        if ( demo.playback->screenshotSize >= (int)&loc_4FFFFE + 2
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                                        456,
                                        0,
                                        "%s",
                                        "demo.playback->screenshotSize < DEMO_JPEG_SAVE_BUFFER_SIZE") )
                        {
                            __debugbreak();
                        }
                        out = &g_JpegBuf[demo.playback->screenshotSize];
                    }
                }
                out = jpeg_encode::end(&encode, out);
                if ( out != (unsigned __int8 *)g_JpegBuf[demo.playback->screenshotSize] )
                {
                    demo.playback->screenshotSize += out - &g_JpegBuf[demo.playback->screenshotSize];
                    if ( demo.playback->screenshotSize >= (int)&loc_4FFFFE + 2
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                                    490,
                                    0,
                                    "%s",
                                    "demo.playback->screenshotSize < DEMO_JPEG_SAVE_BUFFER_SIZE") )
                    {
                        __debugbreak();
                    }
                }
                R_EndHiResShot();
                v3 = va("Image Quality: %d Screenshot Size: %d\n", imageQuality, demo.playback->screenshotSize);
                Demo_Printf(3, v3);
                fraction = (float)(demo.playback->screenshotSize - 0x100000) / 1048576.0;
                if ( fraction <= 0.0 )
                {
                    screenshotSuccessful = 1;
                    break;
                }
                if ( imageQuality >= worstImageQuality )
                    break;
                imageQuality += (int)(float)((float)(worstImageQuality - imageQuality) * fraction);
            }
            Z_VirtualFree(tempY, 23);
            Z_VirtualFree(tempU, 23);
            Z_VirtualFree(tempV, 23);
            yuv_shutdown();
            if ( screenshotSuccessful && demo.playback->screenshotSize < 0x100000 )
                Demo_OpenSavePopUp(localClientNum, 1);
            else
                Demo_Error(localClientNum, "MPUI_ERROR_CAPS", "MENU_DEMO_SCREENSHOT_CAPTURE_FAILED");
            cgameGlob->hiResShotMode = 0;
            Dvar_SetInt((dvar_s *)r_forceLod, curlod);
            Dvar_SetBool((dvar_s *)r_streamFreezeState, streamFreezeState);
            demo.playback->hideDemoHud = hideDemoHud;
            CL_UpdateUIVisibilityBits(localClientNum);
            Demo_TagPlayers(localClientNum, &demo.playback->screenshotInfo.screenshotPlayers);
        }
    }
#endif
}

void __cdecl Demo_TagPlayers(int localClientNum, demoTagPlayers *players)
{
    unsigned __int64 v2; // [esp-8h] [ebp-28h]
    unsigned __int64 v3; // [esp-8h] [ebp-28h]
    unsigned __int64 v4; // [esp-8h] [ebp-28h]
    unsigned __int64 v5; // [esp-8h] [ebp-28h]
    int i; // [esp+10h] [ebp-10h]
    int playerIndex; // [esp+14h] [ebp-Ch]
    int playerIndexa; // [esp+14h] [ebp-Ch]
    cg_s *cgameGlob; // [esp+18h] [ebp-8h]
    centity_s *cent; // [esp+1Ch] [ebp-4h]

    if ( players->count < 29 )
    {
        PROF_SCOPED("Demo_TagPlayers - Tagging Players");
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        playerIndex = Demo_GetPlayerIndexForClientNum(cgameGlob->clientNum, cgameGlob->time);
        HIDWORD(v2) = HIDWORD(demo.info.connectedPlayers[playerIndex].xuid);
        LODWORD(v2) = demo.info.connectedPlayers[playerIndex].xuid;
        if ( !Demo_IsPlayerTagged(localClientNum, players, v2) )
        {
            HIDWORD(v3) = HIDWORD(demo.info.connectedPlayers[playerIndex].xuid);
            LODWORD(v3) = demo.info.connectedPlayers[playerIndex].xuid;
            Demo_TagPlayer(players, v3);
        }
        for ( i = 0; i < demo.header.maxClients; ++i )
        {
            cent = CG_GetEntity(localClientNum, i);
            if ( ((*((unsigned int *)cent + 201) >> 1) & 1) != 0 && i != cgameGlob->clientNum )
            {
                if ( CG_CanSeeFriendlyHead(localClientNum, cent) )
                {
                    playerIndexa = Demo_GetPlayerIndexForClientNum(i, cgameGlob->time);
                    if ( playerIndexa >= 0 )
                    {
                        HIDWORD(v4) = HIDWORD(demo.info.connectedPlayers[playerIndexa].xuid);
                        LODWORD(v4) = demo.info.connectedPlayers[playerIndexa].xuid;
                        if ( !Demo_IsPlayerTagged(localClientNum, players, v4) )
                        {
                            HIDWORD(v5) = HIDWORD(demo.info.connectedPlayers[playerIndexa].xuid);
                            LODWORD(v5) = demo.info.connectedPlayers[playerIndexa].xuid;
                            Demo_TagPlayer(players, v5);
                        }
                    }
                }
            }
        }
    }
}

void __cdecl Demo_TagPlayer(demoTagPlayers *players, unsigned __int64 playerXuid)
{
    const char *v2; // eax

    if ( playerXuid )
    {
        players->playerXuids[players->count++] = playerXuid;
        v2 = va(
                     "DEMO Player Tag: Adding player %lld to the tagged players list. Tag players count is now %d\n",
                     playerXuid,
                     players->count);
        Demo_Printf(3, v2);
    }
}

char __cdecl Demo_IsPlayerTagged(int localClientNum, demoTagPlayers *players, unsigned __int64 playerXuid)
{
    int i; // [esp+8h] [ebp-4h]

    for ( i = 0; i < players->count; ++i )
    {
        if ( players->playerXuids[i] == playerXuid )
            return 1;
    }
    return 0;
}

void __cdecl Demo_SaveScreenshotSuccessful(int controllerIndex, unsigned __int64 fileID)
{
    int LocalClientNum; // eax
    int v3; // eax
    char *ClientName; // eax
    int v5; // eax
    char *v6; // [esp-4h] [ebp-50Ch]
    int v7; // [esp+10h] [ebp-4F8h]
    bdTaskResult *k; // [esp+14h] [ebp-4F4h]
    int v9; // [esp+18h] [ebp-4F0h]
    bdTaskResult *m; // [esp+1Ch] [ebp-4ECh]
    int v11; // [esp+20h] [ebp-4E8h]
    bdTag *j; // [esp+24h] [ebp-4E4h]
    int i; // [esp+28h] [ebp-4E0h]
    int numTags; // [esp+2Ch] [ebp-4DCh] BYREF
    char metaData[260]; // [esp+30h] [ebp-4D8h] BYREF
    unsigned __int64 playerXuid; // [esp+138h] [ebp-3D0h]
    int metaDataSize; // [esp+144h] [ebp-3C4h] BYREF
    bdTag tags[40]; // [esp+148h] [ebp-3C0h] BYREF
    int savedregs; // [esp+508h] [ebp+0h] BYREF

    numTags = 0;
    //v11 = 40;
    //for ( j = tags; --v11 >= 0; ++j )
    //    bdTag::bdTag(j);
    metaDataSize = 0;
    LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
    Demo_Error(LocalClientNum, "", "MENU_DEMO_SCREENSHOT_UPLOAD_SUCCESS");
    v3 = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
    UI_CloseMenu(v3, "fileshare_slotselection_scr");
    ClientName = Live_ControllerIndex_GetClientName(controllerIndex);
    I_strncpyz(gamerTag_1, ClientName, 32);
    memset(metaData, 0, 0xFFu);
    Demo_SetTags(controllerIndex, &numTags, tags, &demo.playback->screenshotInfo, FILESHARE_FILETYPE_SCREENSHOT);
    if ( Demo_SetMetaData(
                 controllerIndex,
                 metaData,
                 &metaDataSize,
                 0,
                 &demo.playback->screenshotInfo,
                 FILESHARE_FILETYPE_SCREENSHOT) )
    {
        for ( i = 0; i < demo.playback->screenshotInfo.screenshotPlayers.count; ++i )
        {
            playerXuid = demo.playback->screenshotInfo.screenshotPlayers.playerXuids[i];
            Live_FileShare_AddTag(4u, playerXuid, &numTags, tags, 40);
        }
        Live_FileShare_AddTag(5u, fsSelectedFileTagIndex->current.integer, &numTags, tags, 40);
        LiveStorage_FileShare_WriteSummary(
            controllerIndex,
            fileID,
            FILESHARE_LOCATION_USERSTORAGE,
            gamerTag_1,
            strlen(gamerTag_1),
            metaData,
            metaDataSize,
            tags,
            numTags,
            0);
        v6 = va("userPublishNews TICKER_SCREENSHOTUPLOADED\n");
        v5 = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
        Cmd_ExecuteSingleCommand(v5, controllerIndex, v6);
#ifdef KISAK_DW_TELEMETRY
        dwRecordEventFormat(0, 2u, "JPG{id:%lld location:%d}", fileID, 2);
#endif
        //v7 = 40;
        //for ( k = (bdTaskResult *)&savedregs; --v7 >= 0; bdTag::~bdTag(k) )
        //    k -= 6;
    }
    else
    {
        Com_PrintError(16, "Could not set meta data for uploaded file ID %llu. This is now an untagged file.\n", fileID);
        //v9 = 40;
        //for ( m = (bdTaskResult *)&savedregs; --v9 >= 0; bdTag::~bdTag(m) )
        //    m -= 6;
    }
}

void __cdecl Demo_SaveScreenshot(int localClientNum, unsigned int fileSlot)
{
    const char *v2; // eax
    int ControllerIndex; // eax
    int handle; // [esp+0h] [ebp-13Ch]
    char ospath[256]; // [esp+4h] [ebp-138h] BYREF
    fileShareWriteFileInfo writeFileInfo; // [esp+108h] [ebp-34h] BYREF

    memset(ospath, 0, sizeof(ospath));
    if ( !Demo_IsPlaying() )
    {
        Com_PrintError(0, "ERROR: Cannot save screenshot when we are not playing a demo.\n");
        return;
    }
    if ( !demo.playback->screenshotSize )
    {
        Com_PrintError(0, "ERROR: Cannot save screenshot of size 0.\n");
        return;
    }
    if ( demo_usefilesystem->current.enabled )
    {
        Demo_GetDemoPath(ospath);
        v2 = va("%s.jpeg", demo.demoName);
        handle = Demo_OpenFileWrite(v2, ospath, 0);
        if ( !handle )
        {
            Com_PrintError(0, "ERROR: couldn't open jpg file for write.\n");
            return;
        }
        Demo_Write((char *)g_JpegBuf, demo.playback->screenshotSize, handle);
        Demo_CloseFile(handle);
    }
    writeFileInfo.category = 3;
    writeFileInfo.fileData = g_JpegBuf;
    writeFileInfo.fileSize = demo.playback->screenshotSize;
    writeFileInfo.location = FILESHARE_LOCATION_USERSTORAGE;
    writeFileInfo.fileSlot = fileSlot;
    writeFileInfo.numTags = 0;
    writeFileInfo.tags = 0;
    writeFileInfo.fileName = va("Screenshot_%d.jpg", fileSlot);
    writeFileInfo.thumbData = 0;
    writeFileInfo.thumbDataSize = 0;
    writeFileInfo.dataCallback = 0;
    writeFileInfo.successCallback = Demo_SaveScreenshotSuccessful;
    writeFileInfo.failureCallback = 0;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    LiveStorage_FileShare_WriteFile(ControllerIndex, &writeFileInfo);
}

void __cdecl Demo_SetCmdInProgess()
{
    if ( Demo_IsPlaybackInited() )
        demo.playback->demoCmdInProgress = demo_cmdNum->current.integer;
}

void __cdecl Demo_SetCmdCompleted()
{
    if ( Demo_IsPlaybackInited() )
    {
        Dvar_SetInt((dvar_s *)demo_cmdNum, demo.playback->demoCmdInProgress + 1);
        demo.playback->demoCmdInProgress = -1;
    }
}

bool __cdecl Demo_ShouldProcessCmd(int cmd)
{
    return Demo_IsPlaybackInited() && cmd > demo.playback->demoCmdInProgress;
}

bool __cdecl Demo_IsPlaybackInited()
{
    return demo.playbackInited;
}

bool __cdecl Demo_IsPaused()
{
    if ( !Demo_IsPlaying() )
        return 0;
    if ( !demo.playback )
        return demo_pause->current.enabled;
    return demo_pause->current.enabled && !demo.playback->overridePause;
}

bool __cdecl Demo_KeyframeGenerated()
{
    return Demo_IsPlaybackInited() && demo.playback->keyframeGenerated;
}

void __cdecl Demo_SetCompleted(bool flag)
{
    demo.playback->completed = flag;
}

bool __cdecl Demo_IsCompleted()
{
    if ( !Demo_IsPlaybackInited() )
        return 0;
    if ( Demo_IsShutdownInProgress() )
        return 0;
    return demo.playback->completed;
}

bool __cdecl Demo_IsShutdownInProgress()
{
    return demo.shutdownInProgress;
}

char __cdecl Demo_IsClipRecording()
{
    bool result; // [esp+3h] [ebp-1h]

    result = 0;
    if ( Demo_GetClipState() == 2 || Demo_GetClipState() == 1 )
        return 1;
    return result;
}

bool __cdecl Demo_IsClipPlaying()
{
    return Demo_IsPlaying() && demo.header.isDemoClip;
}

bool __cdecl Demo_GetClipPausedState()
{
    if ( !Demo_IsPlaybackInited() )
        return 0;
    if ( Demo_IsClipPlaying() || Demo_IsClipPreviewRunning() )
        return demo.playback->clipPausedState;
    return 0;
}

void __cdecl Demo_SetJumpTimeFlag(bool flag)
{
    if ( Demo_IsPlaybackInited() )
        demo.playback->jumpTimeFlag = flag;
}

bool __cdecl Demo_GetJumpTimeFlag()
{
    return Demo_IsPlaybackInited() && demo.playback->jumpTimeFlag;
}

void __cdecl Demo_SetSwitchedPlayersFlag(bool flag)
{
    if ( Demo_IsPlaybackInited() )
        demo.playback->switchedPlayers = flag;
}

bool __cdecl Demo_GetSwitchedPlayersFlag()
{
    return Demo_IsPlaybackInited() && demo.playback->switchedPlayers;
}

bool __cdecl Demo_ShouldEntityBeVisibleToClient(int entNum)
{
    return (demo.entClientMask[entNum][demo_client->current.integer >> 5] & (1 << (demo_client->current.integer & 0x1F))) == 0;
}

bool __cdecl Demo_HasClientSpawnedIn(int clientNum)
{
    return demo.ent[clientNum].eType == 1;
}

void __cdecl Demo_SetupForPlayerSwitch(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    CG_ClearPopUpUI(localClientNum);
    EndShellShock(localClientNum);
    cgameGlob->newPlayerViewmodel = 1;
    CG_ResetLowHealthOverlay(cgameGlob);
    cgameGlob->refdef.poisonFx.curAmountTarget = 0.0f;
    cgameGlob->refdef.poisonFx.curAmount = 0.0f;
    cgameGlob->refdef.waterSheetingFx.enabled = 0;
    cgameGlob->refdef.waterSheetingFx.duration = 0;
    cgameGlob->refdef.electrifiedFx.enabled = 0;
    cgameGlob->refdef.electrifiedFx.duration = 0;
    cgameGlob->refdef.flameFx.enabled = 0;
    cgameGlob->refdef.flameFx.duration = 0;
    Demo_SetSwitchedPlayersFlag(1);
}

char __cdecl Demo_SwitchPlayer(int localClientNum, bool reverse, int forceClientNum, bool forceToLivePlayer)
{
    int oldClientNum; // [esp+Ch] [ebp-Ch]
    int i; // [esp+10h] [ebp-8h]
    int clientNum; // [esp+14h] [ebp-4h]

    i = 0;
    if ( forceClientNum < 0 )
    {
        oldClientNum = demo_client->current.integer;
        clientNum = oldClientNum;
        while ( i < demo.header.maxClients )
        {
            if ( reverse )
                --clientNum;
            else
                ++clientNum;
            if ( clientNum < demo.header.maxClients )
            {
                if ( clientNum < 0 )
                    clientNum = demo.header.maxClients - 1;
            }
            else
            {
                clientNum = 0;
            }
            if ( demo.client[clientNum].clientIndex )
            {
                if ( forceToLivePlayer )
                {
                    if ( ((*((unsigned int *)CG_GetEntity(localClientNum, clientNum) + 201) >> 1) & 1) != 0
                        && Demo_HasClientSpawnedIn(clientNum)
                        && oldClientNum != clientNum )
                    {
                        goto LABEL_20;
                    }
                }
                else if ( demo.client[clientNum].clientIndex >= 0
                             && demo.ps[clientNum].commandTime
                             && demo.ps[clientNum].clientNum == clientNum
                             && oldClientNum != clientNum )
                {
LABEL_20:
                    Demo_SetupForPlayerSwitch(localClientNum);
                    Dvar_SetInt((dvar_s *)demo_client, clientNum);
                    return oldClientNum != clientNum;
                }
                ++i;
            }
            else
            {
                ++i;
            }
        }
        return oldClientNum != clientNum;
    }
    else if ( demo_client->current.integer == forceClientNum )
    {
        return 0;
    }
    else
    {
        Demo_SetupForPlayerSwitch(localClientNum);
        Dvar_SetInt((dvar_s *)demo_client, forceClientNum);
        return 1;
    }
}

bool __cdecl Demo_ShouldUpdateViewAngles(int localClientNum)
{
    if ( Demo_IsRenderingScreenshot(localClientNum) )
        return 0;
    if ( Demo_IsErrorDialogOpen(localClientNum) )
        return 0;
    if ( Key_IsCatcherActive(localClientNum, -1) )
        return UI_GetActiveMenu(localClientNum) == UIMENU_SCOREBOARD;
    return 1;
}

int __cdecl Demo_GetKeyFrameTime(int index)
{
    if ( Demo_IsPlaybackInited() )
        return demo.playback->keyFrame[index].keyframeSnapshotTime;
    else
        return -1;
}

int __cdecl Demo_GetCurrentControllerConfig()
{
    return demo_controllerConfig->current.integer;
}

bool __cdecl Demo_IsPublicOnlineMatch()
{
    if ( !Demo_IsPlaying() )
        return 0;
    return demo.header.settings.onlinegame && !demo.header.settings.xblive_privatematch;
}

bool __cdecl Demo_IsCombatTrainingMatch()
{
    return Demo_IsPlaying() && demo.header.settings.combatTraining;
}

bool __cdecl Demo_IsWagerMatch()
{
    return Demo_IsPlaying() && demo.header.settings.xblive_wagermatch;
}

bool __cdecl Demo_IsCustomGameMode()
{
    return Demo_IsPlaying() && demo.header.settings.customGameMode;
}

int __cdecl Demo_GetKeyFrameForJumpBack(int localClientNum, int newServerTime)
{
    int DemoClientIndex; // eax
    int i; // [esp+4h] [ebp-30h]
    clientActive_t *LocalClientGlobals; // [esp+8h] [ebp-2Ch]
    bool checkForFirstKeyframe; // [esp+Fh] [ebp-25h]
    int jumpBackTimeBiasFraction; // [esp+10h] [ebp-24h]
    bool checkForPlayerConnectTime; // [esp+17h] [ebp-1Dh]
    int curKeyframeIndex; // [esp+18h] [ebp-1Ch]
    int numAttempts; // [esp+1Ch] [ebp-18h]
    int keyframeIndex; // [esp+20h] [ebp-14h]
    int disconnectTime; // [esp+24h] [ebp-10h] BYREF
    int newKeyframeIndex; // [esp+28h] [ebp-Ch]
    int connectTime; // [esp+2Ch] [ebp-8h] BYREF
    bool foundOldKeyframeForRewinding; // [esp+33h] [ebp-1h]

    newKeyframeIndex = -1;
    numAttempts = -1;
    connectTime = -1;
    disconnectTime = -1;
    foundOldKeyframeForRewinding = 0;
    checkForPlayerConnectTime = 0;
    checkForFirstKeyframe = 0;
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    keyframeIndex = demo.playback->keyframeIndex;
    if ( Demo_IsClipPlaying() )
    {
        if ( newServerTime >= 0 )
            checkForFirstKeyframe = newServerTime == 0;
        else
            newServerTime = Demo_GetClipTime() - 800;
    }
    else
    {
        checkForPlayerConnectTime = 1;
        Demo_GetConnectionTimeInfoForPlayer(localClientNum, &connectTime, &disconnectTime);
        if ( Demo_IsPaused() || Com_GetTimeScale() <= 2.0 )
            jumpBackTimeBiasFraction = 1;
        else
            jumpBackTimeBiasFraction = 3;
        if ( newServerTime >= 0 )
        {
            if ( newServerTime <= connectTime )
            {
                if ( !newServerTime )
                    checkForFirstKeyframe = 1;
                DemoClientIndex = Demo_GetDemoClientIndex();
                Demo_SwitchPlayer(localClientNum, 0, DemoClientIndex, 1);
            }
        }
        else
        {
            newServerTime = LocalClientGlobals->snap.serverTime - 800 * jumpBackTimeBiasFraction;
        }
    }
    if ( demo.playback->keyFrame[keyframeIndex].keyframeSnapshotTime <= newServerTime )
    {
        if ( checkForPlayerConnectTime && demo.playback->keyFrame[keyframeIndex].keyframeSnapshotTime <= connectTime )
            return -1;
        return keyframeIndex;
    }
    if ( !checkForFirstKeyframe )
    {
        curKeyframeIndex = keyframeIndex;
        while ( !foundOldKeyframeForRewinding && numAttempts < 250 )
        {
            ++numAttempts;
            if ( curKeyframeIndex )
                newKeyframeIndex = (curKeyframeIndex - 1) % 250;
            else
                newKeyframeIndex = 249;
            curKeyframeIndex = newKeyframeIndex;
            if ( demo.playback->keyFrame[newKeyframeIndex].keyframeSnapshotTime / 1000 < newServerTime / 1000
                && demo.playback->keyFrame[newKeyframeIndex].keyframeSnapshotTime >= 0
                && (demo.playback->keyFrame[newKeyframeIndex].keyframeSnapshotTime < demo.playback->keyFrame[keyframeIndex].keyframeSnapshotTime
                 || demo.playback->keyFrame[keyframeIndex].keyframeSnapshotTime <= 0)
                && (!checkForPlayerConnectTime
                 || demo.playback->keyFrame[newKeyframeIndex].keyframeSnapshotTime / 1000 >= connectTime / 1000
                 || !newKeyframeIndex && demo_client->current.integer == Demo_GetDemoClientIndex()) )
            {
                foundOldKeyframeForRewinding = 1;
            }
        }
        if ( !foundOldKeyframeForRewinding )
            return -1;
        return newKeyframeIndex;
    }
    newKeyframeIndex = -1;
    for ( i = 0; i < 250; ++i )
    {
        if ( demo.playback->keyFrame[i].keyframeSnapshotTime >= 0 )
        {
            if ( newKeyframeIndex >= 0 )
            {
                if ( demo.playback->keyFrame[newKeyframeIndex].keyframeSnapshotTime > demo.playback->keyFrame[i].keyframeSnapshotTime )
                    newKeyframeIndex = i;
            }
            else
            {
                newKeyframeIndex = i;
            }
        }
    }
    return newKeyframeIndex;
}

int __cdecl Demo_GetKeyFrameForJumpForward(int localClientNum, int currentTime)
{
    int DemoClientIndex; // eax
    int i; // [esp+0h] [ebp-20h]
    clientActive_t *LocalClientGlobals; // [esp+4h] [ebp-1Ch]
    int newKeyframeTime; // [esp+8h] [ebp-18h]
    bool checkForPlayerConnectTime; // [esp+Fh] [ebp-11h]
    int keyframeIndex; // [esp+10h] [ebp-10h]
    int disconnectTime; // [esp+14h] [ebp-Ch] BYREF
    int newKeyframeIndex; // [esp+18h] [ebp-8h]
    int connectTime; // [esp+1Ch] [ebp-4h] BYREF

    connectTime = -1;
    disconnectTime = -1;
    checkForPlayerConnectTime = 0;
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    keyframeIndex = demo.playback->keyframeIndex;
    if ( Demo_IsClipPlaying() )
    {
        if ( currentTime < 0 )
            currentTime = Demo_GetClipTime();
    }
    else
    {
        checkForPlayerConnectTime = 1;
        Demo_GetConnectionTimeInfoForPlayer(localClientNum, &connectTime, &disconnectTime);
        if ( currentTime >= 0 )
        {
            if ( currentTime >= disconnectTime )
            {
                DemoClientIndex = Demo_GetDemoClientIndex();
                Demo_SwitchPlayer(localClientNum, 0, DemoClientIndex, 1);
            }
        }
        else
        {
            currentTime = LocalClientGlobals->snap.serverTime;
        }
    }
    if ( demo.playback->keyFrame[keyframeIndex].keyframeSnapshotTime < currentTime )
        return -1;

    static const int pausedBuffer = 700;

    if ( Demo_IsPaused() )
        currentTime += pausedBuffer;

    newKeyframeTime = -1;
    newKeyframeIndex = -1;
    for ( i = 0; i < 250; ++i )
    {
        if ( demo.playback->keyFrame[i].keyframeSnapshotTime >= 0
            && demo.playback->keyFrame[i].keyframeSnapshotTime > currentTime
            && (!checkForPlayerConnectTime || demo.playback->keyFrame[i].keyframeSnapshotTime < disconnectTime)
            && (newKeyframeTime < 0 || demo.playback->keyFrame[i].keyframeSnapshotTime < newKeyframeTime) )
        {
            newKeyframeTime = demo.playback->keyFrame[i].keyframeSnapshotTime;
            newKeyframeIndex = i;
        }
    }
    return newKeyframeIndex;
}

void __cdecl Demo_DrawTransitionScreen(int localClientNum)
{
    float color[4]; // [esp+30h] [ebp-14h] BYREF
    int deltaTime; // [esp+40h] [ebp-4h]

    if ( Demo_IsTransitionScreenActive() )
    {
        deltaTime = Sys_Milliseconds() - demo.playback->transitionScreenTime;
        memset(color, 0, 12);
        color[3] = 1.0 - (float)((float)deltaTime / 1000.0);
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            0.0,
            0.0,
            640.0,
            480.0,
            4,
            4,
            0.0,
            0.0,
            1.0,
            1.0,
            color,
            cgMedia.whiteMaterial);
    }
}

void __cdecl Demo_ActivateTransitionScreen(int transition)
{
    demo.playback->transition = transition;
    if ( transition )
        demo.playback->transitionScreenTime = Sys_Milliseconds();
}

bool __cdecl Demo_IsTransitionScreenActive()
{
    if ( !Demo_IsPlaying() || !Demo_IsPlaybackInited() )
        return 0;
    if ( demo.playback->transitionScreenTime >= 0 )
        return Sys_Milliseconds() - demo.playback->transitionScreenTime <= 0x3E8;
    return 0;
}

void __cdecl Demo_DrawMetaInformationScreen(int localClientNum)
{
    float bgColor[4]; // [esp+30h] [ebp-40h] BYREF
    Font_s *font; // [esp+40h] [ebp-30h]
    int textStyle; // [esp+44h] [ebp-2Ch]
    float width; // [esp+48h] [ebp-28h]
    float textColor[4]; // [esp+4Ch] [ebp-24h] BYREF
    const char *s; // [esp+5Ch] [ebp-14h]
    float x; // [esp+60h] [ebp-10h]
    float y; // [esp+64h] [ebp-Ch]
    int deltaTime; // [esp+68h] [ebp-8h]
    bool showMetaInfo; // [esp+6Fh] [ebp-1h]

    showMetaInfo = 0;
    if ( Demo_IsMetaInformationScreenActive() )
    {
        deltaTime = Sys_Milliseconds() - demo.playback->metaInformationScreenTime;
        memset(bgColor, 0, 12);
        bgColor[3] = 1.0;
        textColor[0] = colorWhite[0];
        textColor[1] = 1.0;
        textColor[2] = 1.0;
        textColor[3] = 1.0;
        textStyle = 0;
        font = UI_GetFontHandle(&scrPlaceView[localClientNum], 3, 0.33333334);
        if ( deltaTime >= 5000 )
        {
            if ( deltaTime > 7000 )
            {
                demo.playback->metaInformationScreenTime = -1;
                return;
            }
            bgColor[3] = 1.0 - (float)((float)(deltaTime - 5000) / 2000.0);
            textColor[3] = 1.0 - (float)((float)(3 * (deltaTime - 5000)) / 2000.0);
            if ( Com_GetTimeScale() != 1.0 )
            {
                Com_SetTimeScale(1.0);
                Demo_SetDefaultClient(localClientNum, -1);
            }
        }
        else
        {
            showMetaInfo = 1;
            if ( Com_GetTimeScale() != 2.0 )
                Com_SetTimeScale(2.0);
        }
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            0.0,
            0.0,
            640.0,
            480.0,
            4,
            4,
            0.0,
            0.0,
            1.0,
            1.0,
            bgColor,
            cgMedia.whiteMaterial);
        s = UI_SafeTranslateString("MENU_PREPARING");
        width = (float)UI_TextWidth(s, 0, font, 0.33333334);
        x = (float)(640.0 - width) * 0.5;
        y = 0.0f;
        CL_DrawText(
            &scrPlaceView[localClientNum],
            (char *)s,
            0x7FFFFFFF,
            font,
            x,
            0.0,
            0,
            2,
            0.69999999,
            0.69999999,
            textColor,
            textStyle);
        width = 32.0f;
        x = (float)(640.0 - 32.0) * 0.5;
        y = 10.0f;
        CL_DrawSpinner(&scrPlaceView[localClientNum], x, 10.0, 32.0, 32.0, 0, 2, textColor);
    }
}

bool __cdecl Demo_IsMetaInformationScreenActive()
{
    return Demo_IsPlaying() && Demo_IsPlaybackInited() && demo.playback->metaInformationScreenTime >= 0;
}

void __cdecl Demo_ResetSnapshotData()
{
    memset(demo.ps, 0, sizeof(demo.ps));
    memset(&demo.matchState, 0, sizeof(demo.matchState));
    memset(demo.ent, 0xFFu, sizeof(demo.ent));
    memset(demo.entClientMask, 0, sizeof(demo.entClientMask));
    memset(demo.client, 0xFFu, sizeof(demo.client));
    demo.prevNumParsedEntities = 0;
    demo.prevNumParsedClients = 0;
}

void __cdecl Demo_ResetTimeValues(int localClientNum, int time)
{
    cg_s *v2; // eax
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-Ch]
    clientConnection_t *clc; // [esp+8h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    clc = CL_GetLocalClientConnection(localClientNum);
    v2 = CG_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals->serverTime = time;
    cls.realtime = time;
    LocalClientGlobals->oldFrameServerTime = time;
    LocalClientGlobals->oldServerTime = time;
    clc->lastPacketTime = time;
    v2->time = time;
    v2->landTime = 0;
    v2->damageTime = 0;
    v2->healthOverlayPulseTime = time;
    v2->v_dmg_time = 0;
    v2->crosshairClientLastTime = 0;
    v2->resetWeaponStateFlag = 1;
    v2->stepViewChange = 0.0f;
}

void __cdecl Demo_ResetSequenceNumbers(int localClientNum, int sequence)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]
    cgs_t *cgs; // [esp+4h] [ebp-4h]

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgs->processedSnapshotNum = sequence;
    cgameGlob->latestSnapshotNum = sequence;
}

void __cdecl Demo_ResetWorldInformation(int localClientNum, int time)
{
    int j; // [esp+2Ch] [ebp-18h]
    centity_s *cent; // [esp+30h] [ebp-14h]
    int i; // [esp+34h] [ebp-10h]
    clientActive_t *LocalClientGlobals; // [esp+38h] [ebp-Ch]
    unsigned __int16 t; // [esp+3Ch] [ebp-8h]
    cg_s *cgameGlob; // [esp+40h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    memset(cgameGlob->viewDamage, 0, sizeof(cgameGlob->viewDamage));
    CG_ClearCameraShakes(localClientNum);
    R_InitSceneData(localClientNum);
    cgameGlob->cursorHintFade = 0;
    cgameGlob->cursorHintIcon = 0;
    cgameGlob->cursorHintString = -1;
    cgameGlob->prevViewmodelWeapon = 0;
    Con_Close(localClientNum);

    {
        PROF_SCOPED("Resetting World - FX Rewind");
        CG_RestartSmokeGrenades(localClientNum);
    }
    
    cgameGlob->newPlayerViewmodel = 1;
    cgameGlob->clearMarks = 1;

    {
        PROF_SCOPED("Resetting World - Reset Destructibles & Compass Data");
        CG_RestartDestructibles(localClientNum);
        CG_ResetCompassData(localClientNum, time);
        CG_ClearPopUpUI(localClientNum);
        CG_ResetDirectionalDamageIndicators(localClientNum, time);
        CG_ResetLowHealthOverlay(cgameGlob);
    }
    
    cgameGlob->refdef.poisonFx.curAmountTarget = 0.0f;
    cgameGlob->refdef.poisonFx.curAmount = 0.0f;
    cgameGlob->refdef.waterSheetingFx.enabled = 0;
    cgameGlob->refdef.waterSheetingFx.duration = 0;
    cgameGlob->refdef.electrifiedFx.enabled = 0;
    cgameGlob->refdef.electrifiedFx.duration = 0;
    cgameGlob->refdef.flameFx.enabled = 0;
    cgameGlob->refdef.flameFx.duration = 0;
    CG_ClearOverheadFade();
    CG_ClearPlayerDetails();
    Flame_Reset();
    IK_ResetTime();
    NitrousVehicle::reset_vehicle_physics();

    {
        PROF_SCOPED("Resetting World - Client Script Notify");
        if (cg_scr_mp_data.demo_jump)
        {
            Scr_AddInt(LocalClientGlobals->snap.serverTime, SCRIPTINSTANCE_CLIENT);
            Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
            t = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_mp_data.demo_jump, 2u);
            Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
        }
    }
    
    PROF_SCOPED("Resetting World - Centity Resets");
    for ( i = 0; i < 1024; ++i )
    {
        cent = CG_GetEntity(localClientNum, i);
        if ( cent->vehicle )
        {
            for ( j = 0; j < 4; ++j )
                cent->vehicle->lastGunnerFire[j] = 0;
        }
        if ( cent->nitrousVeh )
        {
            NitrousVehicle::remove_vehicle(cent->nitrousVeh);
            cent->nitrousVeh = 0;
        }
        if ( cent->clientTagCache )
            cent->clientTagCache = 0;
    }
}

bool __cdecl Demo_IsGameHudHidden()
{
    return Demo_IsPlaybackInited() && demo.playback->hideGameHud;
}

bool __cdecl Demo_IsDemoHudHidden()
{
    if ( !Demo_IsPlaybackInited() )
        return 0;
    if ( Demo_IsRenderingMovie() || Demo_IsClipPreviewRunning() )
        return 1;
    return demo.playback->hideDemoHud;
}

int __cdecl Demo_GetCurrentCameraMode()
{
    if ( Demo_IsPlaybackInited() )
        return demo.playback->cameraMode;
    else
        return 0;
}

int __cdecl Demo_GetNextCameraMode(int cameraMode)
{
    if ( cameraMode == 2 )
        return 0;
    else
        return cameraMode + 1;
}

void __cdecl Demo_SwitchCameraMode(int localClientNum, unsigned int newCamMode)
{
    const char *v2; // eax
    const char *v3; // eax
    float *clipCameraAngles; // [esp+8h] [ebp-24h]
    float *clipCameraOrigin; // [esp+10h] [ebp-1Ch]
    clientActive_t *LocalClientGlobals; // [esp+20h] [ebp-Ch]
    cg_s *cgameGlob; // [esp+24h] [ebp-8h]
    unsigned int oldCamMode; // [esp+28h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if ( !CL_LocalClient_IsCUIFlagSet(localClientNum, 32)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                    1952,
                    0,
                    "%s",
                    "CL_LocalClient_IsCGameInitialized( localClientNum )") )
    {
        __debugbreak();
    }
    oldCamMode = demo.playback->cameraMode;
    if ( oldCamMode != newCamMode )
    {
        if ( oldCamMode >= 2 )
        {
            if ( oldCamMode == 2 )
            {
                cgameGlob->cameraMode = 0;
            }
            else
            {
                v2 = va("We are in an unsupported camera mode %d\n", oldCamMode);
                if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp", 1976, 0, v2) )
                    __debugbreak();
            }
        }
        if ( newCamMode >= 2 )
        {
            if ( newCamMode == 2 )
            {
                cgameGlob->movieCameraOrigin[0] = cgameGlob->refdef.vieworg[0];
                cgameGlob->movieCameraOrigin[1] = cgameGlob->refdef.vieworg[1];
                cgameGlob->movieCameraOrigin[2] = cgameGlob->refdef.vieworg[2];
                AxisToAngles(cgameGlob->refdef.viewaxis, LocalClientGlobals->viewangles);
                LocalClientGlobals->viewangles[2] = 0.0f;
                if ( oldCamMode == 1 )
                {
                    clipCameraOrigin = demo.playback->clipCameraOrigin;
                    demo.playback->clipCameraOrigin[0] = cgameGlob->movieCameraOrigin[0];
                    clipCameraOrigin[1] = cgameGlob->movieCameraOrigin[1];
                    clipCameraOrigin[2] = cgameGlob->movieCameraOrigin[2];
                    clipCameraAngles = demo.playback->clipCameraAngles;
                    demo.playback->clipCameraAngles[0] = LocalClientGlobals->viewangles[0];
                    clipCameraAngles[1] = LocalClientGlobals->viewangles[1];
                    clipCameraAngles[2] = LocalClientGlobals->viewangles[2];
                }
                cgameGlob->cameraMode = 1;
                EndShellShock(localClientNum);
            }
            else
            {
                v3 = va("We are in an unsupported camera mode %d\n", newCamMode);
                if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp", 2010, 0, v3) )
                    __debugbreak();
            }
        }
        demo.playback->cameraMode = newCamMode;
        if ( Demo_IsPaused() )
        {
            IK_ResetTime();
            Flame_Reset();
        }
    }
}

void __cdecl Demo_UpdateVisibilityBitsForCameraMode(int localClientNum, int cameraMode)
{
    int v3; // eax
    int v4; // ecx
    char *v5; // eax

    if (Demo_IsPlaying() && Demo_IsPlaybackInited() && CG_GetLocalClientGlobals(localClientNum)->nextSnap)
    {
        if (cameraMode < 0)
            cameraMode = Demo_GetCurrentCameraMode();
        if (cameraMode)
        {
            if (cameraMode == 1)
            {
                v3 = sharedUiInfo.localVisibilityBits[localClientNum];
                HIDWORD(sharedUiInfo.visibilityBits[localClientNum]) |= 0x20000u;
                LODWORD(sharedUiInfo.localVisibilityBits[localClientNum]) = v3;
            }
            else if (cameraMode == 2)
            {
                v4 = sharedUiInfo.localVisibilityBits[localClientNum];
                HIDWORD(sharedUiInfo.visibilityBits[localClientNum]) |= 0x40000u;
                LODWORD(sharedUiInfo.localVisibilityBits[localClientNum]) = v4;
            }
            else
            {
                v5 = va("We are in an unsupported camera mode %d\n", cameraMode);
                if (!Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp", 2060, 0, v5))
                    __debugbreak();
            }
        }
    }
}

bool __cdecl Demo_IsThirdPersonCamera()
{
    return Demo_IsPlaying() && Demo_GetCurrentCameraMode() == 1;
}

bool __cdecl Demo_IsMovieCamera()
{
    return Demo_IsPlaying() && Demo_GetCurrentCameraMode() == 2;
}

bool __cdecl Demo_ShouldUpdateMovieCamera(int localClientNum)
{
    if ( Demo_IsPauseMenuOpen(localClientNum) )
        return 0;
    if ( Demo_IsSaveDialogOpen(localClientNum) )
        return 0;
    if ( Demo_IsErrorDialogOpen(localClientNum) )
        return 0;
    return !Demo_IsRenderingScreenshot(localClientNum);
}

bool __cdecl Demo_IsRenderingScreenshot(int localClientNum)
{
    return CG_GetLocalClientGlobals(localClientNum)->hiResShotMode != 0;
}

bool __cdecl Demo_IsRenderingMovie()
{
    return Demo_IsPlaying() && demo.renderingMovie;
}

void __cdecl Demo_SetClipState(int state)
{
    if ( Demo_IsPlaybackInited() )
        demo.playback->clipState = state;
}

int __cdecl Demo_GetClipState()
{
    if ( Demo_IsPlaybackInited() )
        return demo.playback->clipState;
    else
        return 0;
}

bool __cdecl Demo_IsClipPreviewRunning()
{
    if ( !Demo_IsPlaying() )
        return 0;
    if ( Demo_IsPlaybackInited() )
        return Demo_GetClipState() == 4;
    return 0;
}

void __cdecl Demo_LerpClipCameraValues(int localClientNum)
{
    const char *v1; // eax
    float v2; // [esp+Ch] [ebp-48h]
    float *v3; // [esp+18h] [ebp-3Ch]
    float *v4; // [esp+1Ch] [ebp-38h]
    float *prevClipCameraOrigin; // [esp+20h] [ebp-34h]
    float *clipCameraOrigin; // [esp+24h] [ebp-30h]
    float v7; // [esp+28h] [ebp-2Ch]
    float *prevClipCameraAngles; // [esp+34h] [ebp-20h]
    float *clipCameraAngles; // [esp+38h] [ebp-1Ch]
    int j; // [esp+3Ch] [ebp-18h]
    int i; // [esp+40h] [ebp-14h]
    clientActive_t *LocalClientGlobals; // [esp+44h] [ebp-10h]
    int cameraMode; // [esp+48h] [ebp-Ch]
    cg_s *cgameGlob; // [esp+4Ch] [ebp-8h]
    float f; // [esp+50h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    cameraMode = Demo_GetCurrentCameraMode();
    f = cgameGlob->frameInterpolation;
    if ( cameraMode == 1 )
    {
        if ( demo.playback->forceTeleportClipCamera )
        {
            prevClipCameraAngles = demo.playback->prevClipCameraAngles;
            clipCameraAngles = demo.playback->clipCameraAngles;
            demo.playback->prevClipCameraAngles[0] = demo.playback->clipCameraAngles[0];
            prevClipCameraAngles[1] = clipCameraAngles[1];
            prevClipCameraAngles[2] = clipCameraAngles[2];
            demo.playback->forceTeleportClipCamera = 0;
        }
        for ( i = 0; i < 3; ++i )
        {
            v7 = demo.playback->prevClipCameraAngles[i];
            LocalClientGlobals->viewangles[i] = AngleNormalize180(demo.playback->clipCameraAngles[i] - v7) * f + v7;
        }
    }
    else if ( cameraMode == 2 )
    {
        if ( demo.playback->forceTeleportClipCamera )
        {
            prevClipCameraOrigin = demo.playback->prevClipCameraOrigin;
            clipCameraOrigin = demo.playback->clipCameraOrigin;
            demo.playback->prevClipCameraOrigin[0] = demo.playback->clipCameraOrigin[0];
            prevClipCameraOrigin[1] = clipCameraOrigin[1];
            prevClipCameraOrigin[2] = clipCameraOrigin[2];
            v3 = demo.playback->prevClipCameraAngles;
            v4 = demo.playback->clipCameraAngles;
            demo.playback->prevClipCameraAngles[0] = demo.playback->clipCameraAngles[0];
            v3[1] = v4[1];
            v3[2] = v4[2];
            demo.playback->forceTeleportClipCamera = 0;
        }
        Vec3Lerp(demo.playback->prevClipCameraOrigin, demo.playback->clipCameraOrigin, f, cgameGlob->movieCameraOrigin);
        for ( j = 0; j < 3; ++j )
        {
            v2 = demo.playback->prevClipCameraAngles[j];
            cgameGlob->movieCameraAngles[j] = AngleNormalize180(demo.playback->clipCameraAngles[j] - v2) * f + v2;
        }
    }
    else if ( (cgameGlob->predictedPlayerState.otherFlags & 2) == 0 || cameraMode )
    {
        v1 = va("Unsupported camera mode %d for lerping\n", cameraMode);
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp", 2273, 0, v1) )
            __debugbreak();
    }
}

void __cdecl Demo_UpdateClipTime(int msec)
{
    if ( Demo_IsPlaybackInited()
        && (Demo_IsClipRecording() || Demo_IsClipPlaying())
        && (!Demo_IsClipPlaying() || !Demo_IsPaused() && !Demo_IsCompleted()) )
    {
        demo.playback->clipTime += msec;
    }
}

int __cdecl Demo_GetClipTime()
{
    if ( Demo_IsPlaybackInited() )
        return demo.playback->clipTime;
    else
        return 0;
}

int __cdecl Demo_GetTotalClipDuration()
{
    int i; // [esp+0h] [ebp-8h]
    int totalDuration; // [esp+4h] [ebp-4h]

    totalDuration = 0;
    if ( !Demo_IsPlaybackInited() )
        return 0;
    for ( i = 0; i < demo.playback->segmentCount; ++i )
        totalDuration += demo.playback->segments[i].duration;
    return totalDuration;
}

void __cdecl Demo_SwitchTransition(int index, bool direction)
{
    if ( direction )
    {
        if ( demo.playback->segments[index].transition == 1 )
            demo.playback->segments[index].transition = 0;
        else
            ++demo.playback->segments[index].transition;
    }
    else if ( demo.playback->segments[index].transition )
    {
        --demo.playback->segments[index].transition;
    }
    else
    {
        demo.playback->segments[index].transition = 1;
    }
}

void __cdecl Demo_SetClipModified(bool flag)
{
    if ( Demo_IsPlaybackInited() )
        demo.playback->clipModified = flag;
}

bool __cdecl Demo_IsClipModified()
{
    return Demo_IsPlaybackInited() && demo.playback->clipModified;
}

void __cdecl Demo_ProcessPlayback()
{
    const char *v0; // eax
    int DemoClientIndex; // eax
    int bookmarkOffset; // [esp+8h] [ebp-2Ch]
    int localClientNum; // [esp+10h] [ebp-24h]
    msg_bookmark_t bookmark; // [esp+1Ch] [ebp-18h] BYREF

    CL_SetupClientsForIngame();
    localClientNum = Com_ControllerIndex_GetLocalClientNum(0);
    Com_UnloadFrontEnd();
    CL_AllocatePerLocalClientMemory();
    CL_GetLocalClientConnectionState(localClientNum);
    CL_GetLocalClientConnection(localClientNum);
    Con_Close(localClientNum);
    Demo_SeekFile(demo.demoFileHandle, 0, 2);
    Demo_Load();
    Demo_ReadHeader();
    MSG_SetBookmark(&demo.msg, &bookmark);
    bookmarkOffset = demo.totalSize;
    if ( Demo_ReadInfoData(demo.demoName, demo.demoFileHandle) )
    {
        MSG_GotoBookmark(&demo.msg, &bookmark);
        Demo_SeekFile(demo.demoFileHandle, bookmarkOffset, 2);
        //BG_EvalVehicleName();
        clientUIActives[localClientNum].connectionState = CA_CONNECTED;
        v0 = Cmd_Argv(1);
        I_strncpyz(cls.servername, v0, 256);
        Demo_ReadGamestate(localClientNum);
        DemoClientIndex = Demo_GetDemoClientIndex();
        Demo_SetDefaultClient(localClientNum, DemoClientIndex);
        Demo_SetDefaultVisionSet(localClientNum);
        demo.playbackInited = 0;
    }
    else
    {
        Com_PrintError(0, "ERROR: Error Reading info file\n");
    }
}

void __cdecl Demo_InitPlaybackData(int localClientNum)
{
    int ControllerIndex; // eax

    memset(demo.playback, 0, sizeof(demoPlayback));
    demo.playback->keyframeIndex = -1;
    demo.playback->demoCmdInProgress = -1;
    demo.playback->transitionScreenTime = -1;
    demo.playback->lastJumpedKeyframe = -1;
    demo.playback->metaInformationScreenTime = -1;
    demo.playbackInited = 1;
    if ( Demo_IsClipPlaying() )
        FX_BeginMarks(localClientNum);
    if ( !Demo_IsClipPlaying() )
        demo.playback->metaInformationScreenTime = Sys_Milliseconds();
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    Live_GiveAchievement(ControllerIndex, "MP_FILM_CREATED");
}

void __cdecl Demo_RestorePreDemoSettings()
{
    char *v1; // eax
    int v2; // eax
    int v3; // ecx
    int v4; // edx
    int v5; // eax
    int localClientNum; // [esp+4h] [ebp-4h]

    v1 = va("DEMO: Restoring the pre-demo settings\n");
    Demo_Printf(3, v1);
    Dvar_SetInt(  (dvar_s*)demo_client, 0);
    Dvar_SetInt(  (dvar_s*)demo_cmdNum, 0);
    Dvar_SetFloat((dvar_s*)demo_timescale, 1.0);
    for (localClientNum = 0; localClientNum < 1; ++localClientNum)
    {
        v2 = sharedUiInfo.localVisibilityBits[localClientNum];
        HIDWORD(sharedUiInfo.visibilityBits[localClientNum]) &= ~0x80000u;
        LODWORD(sharedUiInfo.localVisibilityBits[localClientNum]) = v2;
        v3 = sharedUiInfo.localVisibilityBits[localClientNum];
        HIDWORD(sharedUiInfo.visibilityBits[localClientNum]) &= ~0x100000u;
        LODWORD(sharedUiInfo.localVisibilityBits[localClientNum]) = v3;
        v4 = sharedUiInfo.localVisibilityBits[localClientNum];
        HIDWORD(sharedUiInfo.visibilityBits[localClientNum]) &= ~0x20000u;
        LODWORD(sharedUiInfo.localVisibilityBits[localClientNum]) = v4;
        v5 = sharedUiInfo.localVisibilityBits[localClientNum];
        HIDWORD(sharedUiInfo.visibilityBits[localClientNum]) &= ~0x40000u;
        LODWORD(sharedUiInfo.localVisibilityBits[localClientNum]) = v5;
    }
    Dvar_SetBool((dvar_s*)demo_pause, 0);
    Com_SetTimeScale(1.0);
}

void __cdecl Demo_ReadHeader()
{
    int Byte; // eax
    const char *v1; // eax

    demo.header.version = MSG_ReadByte(&demo.msg);
    demo.header.maxClients = MSG_ReadByte(&demo.msg);
    Byte = MSG_ReadByte(&demo.msg);
    Demo_SetDemoClientIndex(Byte);
    demo.header.isDemoClip = MSG_ReadBit(&demo.msg) != 0;
    demo.header.settings.onlinegame = MSG_ReadBit(&demo.msg) != 0;
    demo.header.settings.xblive_privatematch = MSG_ReadBit(&demo.msg) != 0;
    demo.header.settings.xblive_wagermatch = MSG_ReadBit(&demo.msg) != 0;
    demo.header.settings.systemlink = MSG_ReadBit(&demo.msg) != 0;
    demo.header.settings.customGameMode = MSG_ReadBit(&demo.msg) != 0;
    v1 = va("DEMO Header: Version: %d\n", demo.header.version);
    Demo_Printf(3, v1);
}

void __cdecl Demo_ReadGamestate(int localClientNum)
{
    int v1; // eax
    int msgSequence; // [esp+0h] [ebp-8h]
    int msgType; // [esp+4h] [ebp-4h]

    msgType = MSG_ReadBits(&demo.msg, 2u);
    msgSequence = MSG_ReadLong(&demo.msg);
    MSG_UnEmbed(&demo.msg);
    Demo_ParseServerMessage(localClientNum, &demo.msg, msgType, msgSequence);
    v1 = MSG_GetNumBitsRead(&demo.msg);
    Demo_RecordProfileData(0, v1 / 8);
}

char __cdecl Demo_ReadDemoMessage(int localClientNum)
{
    int ClipTime; // eax
    const char *v3; // eax
    int ControllerIndex; // eax
    const char *v5; // eax
    const char *v6; // eax
    char *timescale; // [esp+0h] [ebp-3Ch]
    float *prevClipCameraAngles; // [esp+4h] [ebp-38h]
    float *prevClipCameraOrigin; // [esp+8h] [ebp-34h]
    float movieCamOrigin; // [esp+14h] [ebp-28h]
    float movieCamOrigin_4; // [esp+18h] [ebp-24h]
    float movieCamOrigin_8; // [esp+1Ch] [ebp-20h]
    float movieCamAngles; // [esp+20h] [ebp-1Ch]
    float movieCamAngles_4; // [esp+24h] [ebp-18h]
    float movieCamAngles_8; // [esp+28h] [ebp-14h]
    clientActive_t *LocalClientGlobals; // [esp+2Ch] [ebp-10h]
    int msgSequence; // [esp+34h] [ebp-8h]
    int msgType; // [esp+38h] [ebp-4h]

    if ( Demo_IsCompleted()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                    2666,
                    0,
                    "%s",
                    "!Demo_IsCompleted()") )
    {
        __debugbreak();
    }
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    Demo_RunFirstFrameEvents(localClientNum);
    if ( Demo_IsClipPlaying() )
    {
        if ( CL_LocalClient_IsCUIFlagSet(localClientNum, 32) && demo.playback->clipCameraMode != demo.playback->cameraMode )
        {
            movieCamOrigin = demo.playback->clipCameraOrigin[0];
            movieCamOrigin_4 = demo.playback->clipCameraOrigin[1];
            movieCamOrigin_8 = demo.playback->clipCameraOrigin[2];
            movieCamAngles = demo.playback->clipCameraAngles[0];
            movieCamAngles_4 = demo.playback->clipCameraAngles[1];
            movieCamAngles_8 = demo.playback->clipCameraAngles[2];
            Demo_SwitchCameraMode(localClientNum, demo.playback->clipCameraMode);
            if ( demo.playback->clipCameraMode != demo.playback->cameraMode
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                            2683,
                            0,
                            "%s",
                            "demo.playback->clipCameraMode == demo.playback->cameraMode") )
            {
                __debugbreak();
            }
            if ( demo.playback->cameraMode == 2 )
            {
                prevClipCameraOrigin = demo.playback->prevClipCameraOrigin;
                demo.playback->prevClipCameraOrigin[0] = movieCamOrigin;
                prevClipCameraOrigin[1] = movieCamOrigin_4;
                prevClipCameraOrigin[2] = movieCamOrigin_8;
                prevClipCameraAngles = demo.playback->prevClipCameraAngles;
                demo.playback->prevClipCameraAngles[0] = movieCamAngles;
                prevClipCameraAngles[1] = movieCamAngles_4;
                prevClipCameraAngles[2] = movieCamAngles_8;
            }
        }
        ClipTime = Demo_GetClipTime();
        if ( Demo_ShouldGenerateKeyFrameSnapshot(localClientNum, ClipTime) )
        {
            Demo_GenerateKeyFrameSnapshot(
                localClientNum,
                LocalClientGlobals->snap.messageNum,
                LocalClientGlobals->snap.serverCommandNum,
                demo.playback->originalFilmTime,
                demo.playback->originalFilmTime,
                LocalClientGlobals->snap.snapFlags);
            if ( Demo_KeyframeGenerated() )
                Demo_WriteKeyFrameInformation(localClientNum);
        }
    }
    if ( Demo_IsClipPreviewRunning() && demo.playback->cancelPreview )
    {
        Demo_RestoreUIStateAfterPreview(localClientNum);
        demo.playback->cancelPreview = 0;
        return 0;
    }
    if ( Demo_IsClipRecording() && demo.playback->clipRecordBufIndex >= 999424 )
    {
        v3 = va("demo_pausecliprecord %d\n", demo_cmdNum->current.integer);
        Cbuf_AddText(localClientNum, v3);
        return 0;
    }
    msgType = MSG_ReadBits(&demo.msg, 2u);
    if ( msgType )
        goto LABEL_43;
    if ( Demo_GetClipState() == 4 )
    {
        if ( demo.playback->pendingSegmentSave )
        {
            if ( demo.playback->clipPreviewSegmentNumber != -1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                            2731,
                            0,
                            "%s",
                            "demo.playback->clipPreviewSegmentNumber == -1") )
            {
                __debugbreak();
            }
            if ( demo.playback->clipRecordBufIndex != demo.playback->clipPreviewBufferIndex )
                goto LABEL_28;
        }
        else if ( demo.playback->previewAllSegments )
        {
            if ( demo.playback->clipPreviewBufferIndex != demo.playback->segments[demo.playback->clipPreviewSegmentNumber].size
                                                                                                    + demo.playback->segments[demo.playback->clipPreviewSegmentNumber].clipBufferIndex )
            {
LABEL_28:
                Demo_Load();
LABEL_41:
                msgType = MSG_ReadBits(&demo.msg, 2u);
                goto LABEL_43;
            }
            if ( ++demo.playback->clipPreviewSegmentNumber != demo.playback->segmentCount )
            {
                Demo_ActivateTransitionScreen(*(unsigned int *)&demo.playback->clipRecordInfo.heliPatches[12
                                                                                                                                                                                    * demo.playback->clipPreviewSegmentNumber
                                                                                                                                                                                    + 22].targetname);
                timescale = va(
                                            "demo_previewsegment %d %d",
                                            demo_cmdNum->current.integer,
                                            demo.playback->clipPreviewSegmentNumber);
                ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
                Cmd_ExecuteSingleCommand(localClientNum, ControllerIndex, timescale);
                goto LABEL_41;
            }
        }
        else
        {
            if ( demo.playback->clipPreviewSegmentNumber == -1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                            2765,
                            0,
                            "%s",
                            "demo.playback->clipPreviewSegmentNumber != -1") )
            {
                __debugbreak();
            }
            if ( demo.playback->clipPreviewBufferIndex != demo.playback->segments[demo.playback->clipPreviewSegmentNumber].size
                                                                                                    + demo.playback->segments[demo.playback->clipPreviewSegmentNumber].clipBufferIndex )
                goto LABEL_28;
        }
        Demo_RestoreUIStateAfterPreview(localClientNum);
        return 0;
    }
    Demo_Load();
    msgType = MSG_ReadBits(&demo.msg, 2u);
LABEL_43:
    if ( msgType == 1 )
    {
        if ( Demo_IsClipRecording() )
        {
            v5 = va("demo_pausecliprecord %d\n", demo_cmdNum->current.integer);
            Cbuf_AddText(localClientNum, v5);
        }
        if ( Demo_IsRenderingMovie() )
        {
            v6 = va("demo_stop\n");
            Cbuf_AddText(localClientNum, v6);
        }
        Demo_SetCompleted(1);
        Com_SetTimeScale(1.0);
        return 0;
    }
    else
    {
        while ( msgType == 2 )
        {
            if ( Demo_ParseClipCommand(localClientNum, &demo.msg, 1) == 9 )
                return 0;
            msgType = MSG_ReadBits(&demo.msg, 2u);
        }
        msgSequence = MSG_ReadLong(&demo.msg);
        if ( Demo_GetClipState() == 2 )
        {
            Demo_WriteClipCommands(localClientNum, &demo.playback->clipRecordingMsg, 1);
            MSG_WriteBits(&demo.playback->clipRecordingMsg, msgType, 2u);
            MSG_WriteLong(&demo.playback->clipRecordingMsg, msgSequence);
        }
        Demo_ParseServerMessage(localClientNum, &demo.msg, msgType, msgSequence);
        demo.lastProcessedMsgNum = msgSequence;
        return 1;
    }
}

void __cdecl Demo_RunFirstFrameEvents(int localClientNum)
{
    const char *v1; // eax
    char ospath[260]; // [esp+4h] [ebp-108h] BYREF

    if ( demo.lastProcessedMsgNum == -1 )
    {
        GlassCl_WaitUpdate();
        if ( Demo_IsRenderingMovie() )
        {
            Demo_GetDemoPath(ospath);
            Dvar_SetInt((dvar_s *)r_clipSize, 0);
            Dvar_SetInt((dvar_s *)r_clipCodec, 0);
            Dvar_SetInt((dvar_s *)r_clipFPS, 24);
            FS_BuildOSPath((char *)fs_homepath->current.integer, (char*)"movies", (char *)"", ospath);
            FS_CreatePath(ospath);
            v1 = va("movie_start \"%s%s.avi\"\n", ospath, demo.demoName);
            Cbuf_AddText(localClientNum, v1);
        }
    }
}

void __cdecl Demo_ParseServerMessage(int localClientNum, msg_t *msg, int msgType, int msgSequence)
{
    const char *v4; // eax
    const char *v5; // eax
    clientConnection_t *clc; // [esp+4h] [ebp-8h]
    int cmd; // [esp+8h] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    clc->serverMessageSequence = msgSequence;
    clc->lastPacketTime = cls.realtime;
    v4 = va(
                 "Demo_ParseServerMessage() - msgType: %d msgSeq: %d ClientCmd: %d\n",
                 msgType,
                 msgSequence,
                 clc->reliableAcknowledge);
    Demo_Printf(1, v4);
    clc->reliableAcknowledge = MSG_ReadLong(msg);
    if ( Demo_GetClipState() == 2 )
        MSG_WriteLong(&demo.playback->clipRecordingMsg, clc->reliableAcknowledge);
    if ( clc->reliableAcknowledge >= clc->reliableSequence - 128 )
    {
        while ( 2 )
        {
            if ( msg->overflowed || (cmd = MSG_ReadByte(msg), cmd == 14) )
            {
                if ( !Demo_IsClipPlaying() && Demo_KeyframeGenerated() )
                    Demo_WriteKeyFrameInformation(localClientNum);
                if ( msg->overflowed
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                                2905,
                                0,
                                "%s",
                                "!msg->overflowed") )
                {
                    __debugbreak();
                }
            }
            else
            {
                if ( msg->overflowed
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                                2870,
                                0,
                                "%s",
                                "!msg->overflowed") )
                {
                    __debugbreak();
                }
                switch ( cmd )
                {
                    case 0:
                        continue;
                    case 1:
                        CL_ParseGamestate(localClientNum, msg);
                        continue;
                    case 2:
                        Demo_ReadConfigStrings(localClientNum, msg);
                        Demo_UpdateConfigStrings(localClientNum);
                        continue;
                    case 10:
                        Demo_ParseServerCommands(localClientNum, msg);
                        continue;
                    case 12:
                        Demo_ParseSnapshot(localClientNum, msg);
                        continue;
                    default:
                        v5 = va("CL_ParseDemoServerMessage: Illegible server message %d\n", cmd);
                        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp", 2877, 0, v5) )
                            __debugbreak();
                        break;
                }
            }
            break;
        }
    }
    else
    {
        clc->reliableAcknowledge = clc->reliableSequence;
    }
}

void __cdecl Demo_ParseServerCommands(int localClientNum, msg_t *msg)
{
    const char *v2; // eax
    int cmdMask[2]; // [esp+0h] [ebp-420h]
    char strBuf[1028]; // [esp+8h] [ebp-418h] BYREF
    clientConnection_t *clc; // [esp+410h] [ebp-10h]
    int index; // [esp+414h] [ebp-Ch]
    const char *s; // [esp+418h] [ebp-8h]
    int clientNum; // [esp+41Ch] [ebp-4h]

    clientNum = demo_client->current.integer;
    cmdMask[1] = 0;
    cmdMask[0] = MSG_ReadLong(msg);
    s = MSG_ReadString(msg, strBuf, 0x400u);
    v2 = va("DEMO: r ServerCommands MaskCmdNum: %i %i CmdString: %s\n", cmdMask[0], 0, s);
    Demo_Printf(4, v2);
    if ( Demo_GetClipState() == 2 )
    {
        MSG_WriteByte(&demo.playback->clipRecordingMsg, 0xAu);
        MSG_WriteLong(&demo.playback->clipRecordingMsg, cmdMask[0]);
        MSG_WriteString(&demo.playback->clipRecordingMsg, s);
    }
    if ( (cmdMask[clientNum >> 5] & (1 << (clientNum & 0x1F))) != 0
        && (cmdMask[clientNum >> 5] != 1 << (clientNum & 0x1F) || !Demo_ShouldFilterServerCommandForMovieCamera(s)) )
    {
        clc = CL_GetLocalClientConnection(localClientNum);
        index = ++clc->serverCommandSequence & 0x7F;
        s = Demo_AdjustTimeForReliableCommand(s);
        I_strncpyz(clc->serverCommands[index], s, 1024);
    }
}

bool __cdecl Demo_ShouldFilterServerCommandForMovieCamera(const char *cmd)
{
    const char *s; // [esp+8h] [ebp-4h] BYREF

    if ( !Demo_IsMovieCamera() )
        return 0;
    s = va("%s", cmd);
    return Com_Parse(&s)->token[0] == 115;
}

clSnapshot_t newSnap_0;
void __cdecl Demo_ParseSnapshot(int localClientNum, msg_t *msg)
{
    const char *v2; // eax
    int v3; // eax
    unsigned __int8 *v4; // eax
    int v5; // eax
    int v6; // eax
    int v7; // eax
    int Bit; // eax
    const char *v9; // eax
    playerState_s result; // [esp+0h] [ebp-4D68h] BYREF
    unsigned __int8 dst[9892]; // [esp+26ACh] [ebp-26BCh] BYREF
    clientActive_t *LocalClientGlobals; // [esp+4D50h] [ebp-18h]
    int v13; // [esp+4D54h] [ebp-14h]
    clientConnection_t *LocalClientConnection; // [esp+4D58h] [ebp-10h]
    int c; // [esp+4D5Ch] [ebp-Ch]
    bool v16; // [esp+4D63h] [ebp-5h]
    int v17; // [esp+4D64h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    LocalClientConnection = CL_GetLocalClientConnection(localClientNum);
    v2 = va("Begin Demo Snapshot Read\n");
    Demo_Printf(512, v2);
    Demo_EnableSnapshotProcessing();
    memset(&newSnap_0, 0, sizeof(newSnap_0));
    newSnap_0.serverCommandNum = LocalClientConnection->serverCommandSequence;
    newSnap_0.serverTime = MSG_ReadLong(msg);
    newSnap_0.physicsTime = MSG_ReadLong(msg);
    newSnap_0.messageNum = LocalClientConnection->serverMessageSequence;
    c = MSG_ReadByte(msg);
    if (c)
        newSnap_0.deltaNum = newSnap_0.messageNum - c;
    else
        newSnap_0.deltaNum = -1;
    newSnap_0.snapFlags = MSG_ReadByte(msg);
    if (Demo_IsClipPlaying())
    {
        demo.playback->originalFilmTime = newSnap_0.serverTime;
        demo.playback->clipServerTime += 100;
    }
    if (Demo_GetClipState() == 2)
    {
        MSG_WriteByte(&demo.playback->clipRecordingMsg, 12);
        MSG_WriteLong(&demo.playback->clipRecordingMsg, newSnap_0.serverTime);
        MSG_WriteLong(&demo.playback->clipRecordingMsg, newSnap_0.physicsTime);
        MSG_WriteByte(&demo.playback->clipRecordingMsg, c);
        MSG_WriteByte(&demo.playback->clipRecordingMsg, newSnap_0.snapFlags);
    }
    newSnap_0.valid = 1;
    v13 = MSG_GetNumBitsRead(msg);
    Demo_ReadMatchState(localClientNum, msg, newSnap_0.serverTime);
    newSnap_0.parseMatchStateNum = LocalClientGlobals->parseMatchStateNum - 1;
    CL_UpdateMapCenterFromMatchState(demo.matchState.unarchivedState.mapCenter);
    svs.mapCenter[0] = cls.mapCenter[0];
    svs.mapCenter[1] = cls.mapCenter[1];
    svs.mapCenter[2] = cls.mapCenter[2];
    svsHeader.mapCenter[0] = cls.mapCenter[0];
    svsHeader.mapCenter[1] = cls.mapCenter[1];
    svsHeader.mapCenter[2] = cls.mapCenter[2];
    v3 = MSG_GetNumBitsRead(msg);
    Demo_RecordProfileData(2, (v3 - v13) / 8);
    v13 = MSG_GetNumBitsRead(msg);
    v4 = (unsigned __int8 *)Demo_ReadPlayerStates(&result, localClientNum, msg, newSnap_0.serverTime);
    memcpy(dst, v4, sizeof(dst));
    memcpy((unsigned __int8 *)&newSnap_0.ps, dst, sizeof(newSnap_0.ps));
    v5 = MSG_GetNumBitsRead(msg);
    Demo_RecordProfileData(3, (v5 - v13) / 8);
    v13 = MSG_GetNumBitsRead(msg);
    newSnap_0.numEntities = Demo_ReadPacketEntities(localClientNum, msg, newSnap_0.serverTime, newSnap_0.deltaNum);
    newSnap_0.parseEntitiesNum = LocalClientGlobals->parseEntitiesNum - newSnap_0.numEntities;
    v6 = MSG_GetNumBitsRead(msg);
    Demo_RecordProfileData(4, (v6 - v13) / 8);
    v13 = MSG_GetNumBitsRead(msg);
    newSnap_0.numClients = Demo_ReadPacketClients(localClientNum, msg, newSnap_0.serverTime, newSnap_0.deltaNum);
    newSnap_0.parseClientsNum = LocalClientGlobals->parseClientsNum - newSnap_0.numClients;
    v7 = MSG_GetNumBitsRead(msg);
    Demo_RecordProfileData(5, (v7 - v13) / 8);
    GlassCl_ParseSnapshot(localClientNum, msg);
    Bit = MSG_ReadBit(msg);
    v16 = Bit != 0;
    if (Bit)
    {
        if (msg->overflowed
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                3130,
                0,
                "%s",
                "!msg->overflowed"))
        {
            __debugbreak();
        }
        Demo_ReadCustomSnaphotInformation(localClientNum, msg);
    }
    if (Demo_GetClipState() == 2)
    {
        GlassCl_WriteDemoSnapshot(&demo.playback->clipRecordingMsg);
        if (v16)
        {
            MSG_WriteBit1(&demo.playback->clipRecordingMsg);
            Demo_WriteCustomSnapshotInformation(localClientNum, &demo.playback->clipRecordingMsg);
        }
        else
        {
            MSG_WriteBit0(&demo.playback->clipRecordingMsg);
        }
        MSG_WriteByte(&demo.playback->clipRecordingMsg, 14);
    }
    Demo_DisableSnapshotProcessing();
    if (msg->overflowed
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
            3159,
            0,
            "%s",
            "!msg->overflowed"))
    {
        __debugbreak();
    }
    if (Demo_IsClipPlaying())
    {
        newSnap_0.serverTime = demo.playback->clipServerTime;
        newSnap_0.physicsTime = demo.playback->clipServerTime;
    }
    if (!Demo_IsClipPlaying() && Demo_ShouldGenerateKeyFrameSnapshot(localClientNum, newSnap_0.serverTime))
        Demo_GenerateKeyFrameSnapshot(
            localClientNum,
            newSnap_0.messageNum,
            newSnap_0.serverCommandNum,
            newSnap_0.serverTime,
            newSnap_0.physicsTime,
            newSnap_0.snapFlags);
    Demo_SetupPlaybackForClient(localClientNum, &newSnap_0.ps);
    v9 = va("End Demo Snapshot Read\n");
    Demo_Printf(512, v9);
    v17 = LocalClientGlobals->snap.messageNum + 1;
    if (newSnap_0.messageNum - v17 >= 32)
        v17 = newSnap_0.messageNum - 31;
    while (v17 < newSnap_0.messageNum)
        LocalClientGlobals->snapshots[v17++ & 0x1F].valid = 0;
    LocalClientGlobals->oldSnapServerTime = LocalClientGlobals->snap.serverTime;
    memcpy((unsigned __int8 *)&LocalClientGlobals->snap, (unsigned __int8 *)&newSnap_0, sizeof(LocalClientGlobals->snap));
    LocalClientGlobals->snap.ping = 999;
    memcpy(
        (unsigned __int8 *)&LocalClientGlobals->snapshots[LocalClientGlobals->snap.messageNum & 0x1F],
        (unsigned __int8 *)&LocalClientGlobals->snap,
        sizeof(LocalClientGlobals->snapshots[LocalClientGlobals->snap.messageNum & 0x1F]));
    LocalClientGlobals->newSnapshots = 1;
}

void __cdecl Demo_ReadMatchState(int localClientNum, msg_t *msg, int time)
{
    clientActive_t *LocalClientGlobals; // [esp+8h] [ebp-8h]
    MatchState *matchState; // [esp+Ch] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    matchState = &LocalClientGlobals->parseMatchStates[LocalClientGlobals->parseMatchStateNum & 0x1F];
    Demo_Printf(9, "DEMO: r Type: MatchState\n");
    MSG_ClearLastReferencedEntity(msg);
    MSG_ReadDeltaMatchState(msg, time, &demo.matchState, matchState);
    if ( Demo_GetClipState() == 2 )
    {
        MSG_ClearLastReferencedEntity(&demo.playback->clipRecordingMsg);
        MSG_WriteDeltaMatchState(
            &g_snapInfo,
            &demo.playback->clipRecordingMsg,
            time,
            &demo.matchState,
            matchState);
    }
    memcpy(&demo.matchState, matchState, sizeof(demo.matchState));
    if ( Demo_IsClipPlaying() )
        Demo_AdjustTimeForMatchState(time, demo.playback->clipServerTime, matchState);
    ++LocalClientGlobals->parseMatchStateNum;
}

playerState_s *__cdecl Demo_ReadPlayerStates(playerState_s *result, int localClientNum, msg_t *msg, int time)
{
    const char *v4; // eax
    int v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    int i; // [esp+0h] [ebp-26B4h]
    playerState_s to; // [esp+4h] [ebp-26B0h] BYREF
    int v11; // [esp+26B0h] [ebp-4h]

    for (i = 0; i < demo.header.maxClients; ++i)
    {
        if (MSG_ReadBit(msg))
        {
            v4 = va("Begin Reading PlayerState\n");
            Demo_Printf(512, v4);
            v11 = MSG_GetNumBitsRead(msg);
            MSG_ReadDeltaPlayerstate(localClientNum, msg, time, &demo.ps[i], &to, 0);
            v5 = MSG_GetNumBitsRead(msg);
            v6 = va("DEMO: r Type: PlayerState Client: %d Size: %d bytes\n", i, (v5 - v11) / 8);
            Demo_Printf(17, v6);
            if (Demo_IsClipPlaying())
                to.commandTime = demo.playback->clipServerTime;
            else
                to.commandTime = time;
            memcpy((unsigned __int8 *)&demo.ps[i], (unsigned __int8 *)&to, sizeof(demo.ps[i]));
        }
        if (Demo_GetClipState() == 2)
        {
            if (i == demo_client->current.integer)
            {
                v7 = va("DEMO clip: Writing playerstate for client %d\n", i);
                Demo_Printf(1024, v7);
                MSG_WriteBit1(&demo.playback->clipRecordingMsg);
                MSG_WriteDeltaPlayerstate(
                    &g_snapInfo,
                    &demo.playback->clipRecordingMsg,
                    time,
                    &demo.playback->clipRecordPS,
                    &demo.ps[i]);
                memcpy(
                    (unsigned __int8 *)&demo.playback->clipRecordPS,
                    (unsigned __int8 *)&demo.ps[i],
                    sizeof(demo.playback->clipRecordPS));
            }
            else
            {
                MSG_WriteBit0(&demo.playback->clipRecordingMsg);
            }
        }
    }
    if (Demo_IsClipPlaying())
    {
        memcpy((unsigned __int8 *)&to, (unsigned __int8 *)&demo.ps[demo_client->current.integer], sizeof(to));
        Demo_AdjustTimeForPlayerState(time, demo.playback->clipServerTime, &to);
    }
    else
    {
        memcpy((unsigned __int8 *)&to, (unsigned __int8 *)&demo.ps[demo_client->current.integer], sizeof(to));
    }
    memcpy((unsigned __int8 *)result, (unsigned __int8 *)&to, sizeof(playerState_s));
    return result;
}

int __cdecl Demo_ReadPacketEntities(int localClientNum, msg_t *msg, int time, int deltaNum)
{
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    int v12; // [esp+8h] [ebp-48h]
    int v13; // [esp+Ch] [ebp-44h]
    int number; // [esp+10h] [ebp-40h]
    clientActive_t *LocalClientGlobals; // [esp+14h] [ebp-3Ch]
    int numVisibleEntities; // [esp+18h] [ebp-38h]
    entityState_s *newState; // [esp+1Ch] [ebp-34h]
    unsigned int newnum; // [esp+20h] [ebp-30h]
    int entityClientMaskBit; // [esp+24h] [ebp-2Ch]
    entityState_s *oldState; // [esp+28h] [ebp-28h]
    int oldframeEntityCount; // [esp+2Ch] [ebp-24h]
    int oldnum; // [esp+34h] [ebp-1Ch]
    int oldindex; // [esp+38h] [ebp-18h]
    int clientMask; // [esp+3Ch] [ebp-14h]
    int numEntities; // [esp+40h] [ebp-10h]
    int entityReadResult; // [esp+44h] [ebp-Ch]
    __int16 oldframeEntityBase; // [esp+4Ch] [ebp-4h]

    oldindex = 0;
    numVisibleEntities = 0;
    numEntities = MSG_ReadBits(msg, 0xBu);
    v4 = va("DEMO: r Type: PacketEntities NumEntities: %d FirstEntity: 0\n", numEntities);
    Demo_Printf(33, v4);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    oldframeEntityBase = demo.prevNumParsedEntities;
    oldframeEntityCount = ((unsigned __int16)LocalClientGlobals->parseEntitiesNum - LOWORD(demo.prevNumParsedEntities))
                                            & 0x7FF;
    demo.prevNumParsedEntities = LocalClientGlobals->parseEntitiesNum;
    demo.prevNumEntities = numEntities;
    if ( deltaNum >= 0 )
        oldnum = LocalClientGlobals->parseEntities[oldframeEntityBase & 0x7FF].number;
    else
        oldnum = 9999;
    MSG_ClearLastReferencedEntity(msg);
    if ( Demo_GetClipState() == 2 )
    {
        MSG_WriteBits(&demo.playback->clipRecordingMsg, numEntities, 0xBu);
        MSG_ClearLastReferencedEntity(&demo.playback->clipRecordingMsg);
    }
    while ( !msg->overflowed )
    {
        newnum = MSG_ReadEntityIndex(msg, 0xAu);
        if ( newnum >= 0x400
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                        3384,
                        0,
                        "%s\n\t(newnum) = %i",
                        "(newnum >= 0 && newnum < (1<<10))",
                        newnum) )
        {
            __debugbreak();
        }
        if ( newnum == 1023 )
            break;
        while ( oldnum < (int)newnum )
        {
            v5 = va("EntityState: NewNum (%d) > OldNum (%d) - Copying old data\n", newnum, oldnum);
            Demo_Printf(32, v5);
            memcpy(
                &LocalClientGlobals->parseEntities[LocalClientGlobals->parseEntitiesNum & 0x7FF],
                &LocalClientGlobals->parseEntities[((_WORD)oldindex + oldframeEntityBase) & 0x7FF],
                sizeof(LocalClientGlobals->parseEntities[LocalClientGlobals->parseEntitiesNum & 0x7FF]));
            ++numVisibleEntities;
            ++LocalClientGlobals->parseEntitiesNum;
            if ( ++oldindex < oldframeEntityCount )
                number = LocalClientGlobals->parseEntities[((_WORD)oldindex + oldframeEntityBase) & 0x7FF].number;
            else
                number = 9999;
            oldnum = number;
        }
        if ( oldnum == newnum )
        {
            v6 = va("EntityState: NewNum (%d) == OldNum (%d) - Reading delta\n", newnum, oldnum);
            Demo_Printf(32, v6);
            if ( Demo_IsClipPlaying() )
                oldState = &demo.ent[newnum];
            else
                oldState = &LocalClientGlobals->parseEntities[((_WORD)oldindex + oldframeEntityBase) & 0x7FF];
        }
        else
        {
            v7 = va("EntityState: NewNum (%d) < OldNum (%d) - Reading from baselines\n", newnum, oldnum);
            Demo_Printf(32, v7);
            oldState = &LocalClientGlobals->entityBaselines[newnum];
            if ( Demo_GetClipState() == 2 )
                g_snapInfo.fromBaseline = 1;
        }
        newState = &LocalClientGlobals->parseEntities[LocalClientGlobals->parseEntitiesNum & 0x7FF];
        entityReadResult = MSG_ReadDeltaEntity(msg, time, oldState, newState, newnum);
        entityClientMaskBit = MSG_ReadBit(msg);
        if ( Demo_GetClipState() == 2 )
        {
            if ( entityReadResult )
                MSG_WriteEntity(&g_snapInfo, &demo.playback->clipRecordingMsg, time, oldState, 0, DELTA_FLAGS_FORCE);
            else
                MSG_WriteEntity(&g_snapInfo, &demo.playback->clipRecordingMsg, time, oldState, newState, DELTA_FLAGS_FORCE);
            g_snapInfo.fromBaseline = 0;
        }
        if ( entityReadResult )
        {
            memcpy(&demo.ent[newnum], &g_defaultEntityState, sizeof(demo.ent[newnum]));
        }
        else
        {
            memcpy(&demo.ent[newnum], newState, sizeof(demo.ent[newnum]));
            if ( Demo_IsClipPlaying() )
                Demo_AdjustTimeForEntityState(time, demo.playback->clipServerTime, newState);
        }
        if ( entityClientMaskBit )
        {
            clientMask = MSG_ReadBits(msg, 0x20u);
            demo.entClientMask[newnum][0] = clientMask;
            v8 = va("ClientMask for EntNum: %d is %d.\n", newnum, clientMask);
            Demo_Printf(32, v8);
        }
        else if ( entityReadResult )
        {
            demo.entClientMask[newnum][0] = 0;
            v9 = va("ClientMask for EntNum: %d is set back to 0.\n", newnum);
            Demo_Printf(32, v9);
        }
        if ( Demo_GetClipState() == 2 )
        {
            if ( entityClientMaskBit )
            {
                MSG_WriteBit1(&demo.playback->clipRecordingMsg);
                MSG_WriteBits(&demo.playback->clipRecordingMsg, demo.entClientMask[newnum][0], 0x20u);
            }
            else
            {
                MSG_WriteBit0(&demo.playback->clipRecordingMsg);
            }
        }
        if ( !entityReadResult )
        {
            ++numVisibleEntities;
            ++LocalClientGlobals->parseEntitiesNum;
        }
        if ( oldnum == newnum )
        {
            if ( ++oldindex < oldframeEntityCount )
                v13 = LocalClientGlobals->parseEntities[((_WORD)oldindex + oldframeEntityBase) & 0x7FF].number;
            else
                v13 = 9999;
            oldnum = v13;
        }
    }
    if ( Demo_GetClipState() == 2 )
        MSG_WriteEntityIndex(&g_snapInfo, &demo.playback->clipRecordingMsg, 1023, 10);
    while ( oldnum != 9999 )
    {
        v10 = va("EntityState: OldNum (%d) != DEMO_LARGE_INVALID_STATE_NUMBER - Copying old data\n", oldnum);
        Demo_Printf(32, v10);
        memcpy(
            &LocalClientGlobals->parseEntities[LocalClientGlobals->parseEntitiesNum & 0x7FF],
            &LocalClientGlobals->parseEntities[((_WORD)oldindex + oldframeEntityBase) & 0x7FF],
            sizeof(LocalClientGlobals->parseEntities[LocalClientGlobals->parseEntitiesNum & 0x7FF]));
        ++numVisibleEntities;
        ++LocalClientGlobals->parseEntitiesNum;
        if ( ++oldindex < oldframeEntityCount )
            v12 = LocalClientGlobals->parseEntities[((_WORD)oldindex + oldframeEntityBase) & 0x7FF].number;
        else
            v12 = 9999;
        oldnum = v12;
    }
    return numVisibleEntities;
}

int __cdecl Demo_ReadPacketClients(int localClientNum, msg_t *msg, int time, int deltaNum)
{
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    int v10; // [esp+8h] [ebp-38h]
    int v11; // [esp+Ch] [ebp-34h]
    int clientIndex; // [esp+10h] [ebp-30h]
    clientActive_t *LocalClientGlobals; // [esp+14h] [ebp-2Ch]
    int clientReadResult; // [esp+18h] [ebp-28h]
    clientState_s *newState; // [esp+1Ch] [ebp-24h]
    signed int newnum; // [esp+20h] [ebp-20h]
    clientState_s *oldState; // [esp+24h] [ebp-1Ch]
    int oldnum; // [esp+28h] [ebp-18h]
    int oldindex; // [esp+2Ch] [ebp-14h]
    int numClients; // [esp+34h] [ebp-Ch]
    int oldframeClientCount; // [esp+38h] [ebp-8h]
    __int16 oldframeClientBase; // [esp+3Ch] [ebp-4h]

    oldindex = 0;
    numClients = MSG_ReadBits(msg, 5u);
    v4 = va("DEMO: r Type: PacketClients NumClients: %d FirstClient: 0\n", numClients);
    Demo_Printf(65, v4);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    oldframeClientBase = demo.prevNumParsedClients;
    oldframeClientCount = ((unsigned __int16)LocalClientGlobals->parseClientsNum - LOWORD(demo.prevNumParsedClients))
                                            & 0x7FF;
    demo.prevNumParsedClients = LocalClientGlobals->parseClientsNum;
    demo.prevNumClients = numClients;
    if ( deltaNum >= 0 )
        oldnum = LocalClientGlobals->parseClients[oldframeClientBase & 0x7FF].clientIndex;
    else
        oldnum = 9999;
    if ( Demo_GetClipState() == 2 )
    {
        MSG_WriteBits(&demo.playback->clipRecordingMsg, numClients, 5u);
        MSG_ClearLastReferencedEntity(&demo.playback->clipRecordingMsg);
    }
    MSG_ClearLastReferencedEntity(msg);
    while ( !msg->overflowed && MSG_ReadBit(msg) )
    {
        newnum = MSG_ReadEntityIndex(msg, 5u);
        while ( oldnum < newnum )
        {
            v5 = va("ClientState: NewNum (%d) > OldNum (%d) - Copying old data\n", newnum, oldnum);
            Demo_Printf(64, v5);
            memcpy(
                &LocalClientGlobals->parseClients[LocalClientGlobals->parseClientsNum++ & 0x7FF],
                &LocalClientGlobals->parseClients[((_WORD)oldindex + oldframeClientBase) & 0x7FF],
                sizeof(LocalClientGlobals->parseClients[LocalClientGlobals->parseClientsNum++ & 0x7FF]));
            if ( ++oldindex < oldframeClientCount )
                clientIndex = LocalClientGlobals->parseClients[((_WORD)oldindex + oldframeClientBase) & 0x7FF].clientIndex;
            else
                clientIndex = 9999;
            oldnum = clientIndex;
        }
        if ( oldnum == newnum )
        {
            v6 = va("ClientState: NewNum (%d) == OldNum (%d) - Reading delta\n", newnum, oldnum);
            Demo_Printf(64, v6);
            oldState = &LocalClientGlobals->parseClients[((_WORD)oldindex + oldframeClientBase) & 0x7FF];
        }
        else
        {
            v7 = va("ClientState: NewNum (%d) < OldNum (%d) - Reading from baselines\n", newnum, oldnum);
            Demo_Printf(64, v7);
            oldState = 0;
        }
        newState = &LocalClientGlobals->parseClients[LocalClientGlobals->parseClientsNum & 0x7FF];
        clientReadResult = MSG_ReadDeltaClient(msg, time, oldState, newState, newnum);
        if ( Demo_GetClipState() == 2 )
        {
            if ( clientReadResult )
                MSG_WriteDeltaClient(&g_snapInfo, &demo.playback->clipRecordingMsg, time, oldState, 0, 1);
            else
                MSG_WriteDeltaClient(&g_snapInfo, &demo.playback->clipRecordingMsg, time, oldState, newState, 1);
        }
        if ( clientReadResult )
        {
            if ( !Demo_IsClipPlaying()
                && !Demo_IsClipPreviewRunning()
                && demo_client->current.integer == demo.client[newnum].clientIndex )
            {
                Demo_SwitchPlayer(localClientNum, 0, -1, 1);
            }
            memcpy(&demo.client[newnum], &g_defaultClientState, sizeof(demo.client[newnum]));
        }
        else
        {
            memcpy(&demo.client[newnum], newState, sizeof(demo.client[newnum]));
            ++LocalClientGlobals->parseClientsNum;
        }
        if ( oldnum == newnum )
        {
            if ( ++oldindex < oldframeClientCount )
                v11 = LocalClientGlobals->parseClients[((_WORD)oldindex + oldframeClientBase) & 0x7FF].clientIndex;
            else
                v11 = 9999;
            oldnum = v11;
        }
    }
    if ( Demo_GetClipState() == 2 )
        MSG_WriteBit0(&demo.playback->clipRecordingMsg);
    while ( oldnum != 9999 )
    {
        v8 = va("ClientState: OldNum (%d) != DEMO_LARGE_INVALID_STATE_NUMBER - Copying old data\n", oldnum);
        Demo_Printf(64, v8);
        memcpy(
            &LocalClientGlobals->parseClients[LocalClientGlobals->parseClientsNum++ & 0x7FF],
            &LocalClientGlobals->parseClients[((_WORD)oldindex + oldframeClientBase) & 0x7FF],
            sizeof(LocalClientGlobals->parseClients[LocalClientGlobals->parseClientsNum++ & 0x7FF]));
        if ( ++oldindex < oldframeClientCount )
            v10 = LocalClientGlobals->parseClients[((_WORD)oldindex + oldframeClientBase) & 0x7FF].clientIndex;
        else
            v10 = 9999;
        oldnum = v10;
    }
    return numClients;
}

void __cdecl Demo_SetupPlaybackForClient(int localClientNum, playerState_s *ps)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-Ch]
    cg_s *cgameGlob; // [esp+4h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    BG_GetWeaponDef(ps->weapon);
    ps->pm_flags &= ~0x800u;
    cgameGlob->clientNum = ps->clientNum;
    cgameGlob->weaponSelect = ps->weapon;
    if ( ps->weapon && LocalClientGlobals->snap.ps.weapon != ps->weapon )
    {
        cgameGlob->weaponSelectTime = cgameGlob->time;
        CG_MenuShowNotify(localClientNum, 1);
    }
    CG_SetThirdPerson(0);
    if ( Demo_GetClipState() == 2 )
        Demo_TagPlayers(localClientNum, &demo.playback->segments[demo.playback->segmentCount].taggedPlayers);
}

void __cdecl Demo_RegisterHeliPatches()
{
    demoHeliPatchesInfo *v0; // eax
    heli_height_lock_patches_t *v1; // ecx
    int i; // [esp+0h] [ebp-4h]

    num_heli_height_lock_patches = demo.info.numHeliPatchesCount;
    for ( i = 0; i < demo.info.numHeliPatchesCount; ++i )
    {
        v0 = &demo.info.heliPatches[i];
        v1 = &heli_height_lock_patches[i];
        v1->brushmodel = v0->brushmodel;
        v1->origin[0] = v0->origin[0];
        v1->origin[1] = v0->origin[1];
        v1->origin[2] = v0->origin[2];
        *(unsigned int *)&v1->targetname = *(unsigned int *)&v0->targetname;
        v1->enabled = v0->enabled;
    }
}

char __cdecl Demo_ReadInfoData(char *name, int handle)
{
    const char *v2; // eax
    const char *v4; // eax
    int k; // [esp+0h] [ebp-12Ch]
    int j; // [esp+4h] [ebp-128h]
    int i; // [esp+8h] [ebp-124h]
    unsigned int totalBytesRead; // [esp+10h] [ebp-11Ch]
    int totalBytesReadb; // [esp+10h] [ebp-11Ch]
    int totalBytesReadc; // [esp+10h] [ebp-11Ch]
    int totalBytesReadd; // [esp+10h] [ebp-11Ch]
    int totalBytesReade; // [esp+10h] [ebp-11Ch]
    int totalBytesReadf; // [esp+10h] [ebp-11Ch]
    int totalBytesReadg; // [esp+10h] [ebp-11Ch]
    int totalBytesReadh; // [esp+10h] [ebp-11Ch]
    int totalBytesReadi; // [esp+10h] [ebp-11Ch]
    int totalBytesReadj; // [esp+10h] [ebp-11Ch]
    int totalBytesReadk; // [esp+10h] [ebp-11Ch]
    int totalBytesReadl; // [esp+10h] [ebp-11Ch]
    int totalBytesReadm; // [esp+10h] [ebp-11Ch]
    int totalBytesReadn; // [esp+10h] [ebp-11Ch]
    int totalBytesReado; // [esp+10h] [ebp-11Ch]
    int totalBytesReadp; // [esp+10h] [ebp-11Ch]
    int totalBytesReada; // [esp+10h] [ebp-11Ch]
    int infoDataHandle; // [esp+14h] [ebp-118h]
    int version_start; // [esp+18h] [ebp-114h] BYREF
    char ospath[256]; // [esp+1Ch] [ebp-110h] BYREF
    int offset; // [esp+120h] [ebp-Ch]
    int version_end; // [esp+124h] [ebp-8h] BYREF
    int infoDataSize; // [esp+128h] [ebp-4h] BYREF

    if ( handle )
    {
        infoDataHandle = handle;
    }
    else
    {
        Demo_GetDemoPath(ospath);
        v2 = va("%s.demo", name);
        infoDataHandle = Demo_OpenFileRead(v2, ospath, 1);
        if ( !infoDataHandle )
            return 0;
    }
    offset = -8;
    Demo_SeekFile(infoDataHandle, -8, 1);
    Demo_Read((unsigned __int8 *)&infoDataSize, 4u, infoDataHandle);
    Demo_Read((unsigned __int8 *)&version_end, 4u, infoDataHandle);
    if ( version_end == demo.header.version )
    {
        offset -= infoDataSize;
        Demo_SeekFile(infoDataHandle, offset, 1);
        totalBytesRead = Demo_Read((unsigned __int8 *)&version_start, 4u, infoDataHandle);
        if ( version_start == demo.header.version )
        {
            totalBytesReadb = totalBytesRead + Demo_Read((unsigned __int8 *)&demo.info.startTime, 4u, infoDataHandle);
            totalBytesReadc = totalBytesReadb + Demo_Read((unsigned __int8 *)&demo.info.endTime, 4u, infoDataHandle);
            totalBytesReadd = totalBytesReadc
                                            + Demo_Read((unsigned __int8 *)&demo.info.numConnectedPlayersInfoCount, 4u, infoDataHandle);
            totalBytesReade = totalBytesReadd
                                            + Demo_Read((unsigned __int8 *)&demo.info.numDefaultBookmarksCount, 4u, infoDataHandle);
            totalBytesReadf = totalBytesReade
                                            + Demo_Read((unsigned __int8 *)&demo.info.numHeliPatchesCount, 4u, infoDataHandle);
            totalBytesReadg = totalBytesReadf + Demo_Read((unsigned __int8 *)demo.info.author, 0x20u, infoDataHandle);
            totalBytesReadh = totalBytesReadg + Demo_Read((unsigned __int8 *)demo.info.gameType, 0x100u, infoDataHandle);
            totalBytesReadi = totalBytesReadh + Demo_Read((unsigned __int8 *)demo.info.mapName, 0x100u, infoDataHandle);
            totalBytesReadj = totalBytesReadi + Demo_Read((unsigned __int8 *)demo.info.name, 0x16u, infoDataHandle);
            totalBytesReadk = totalBytesReadj + Demo_Read((unsigned __int8 *)demo.info.description, 0x16u, infoDataHandle);
            totalBytesReadl = totalBytesReadk + Demo_Read((unsigned __int8 *)&demo.info.week, 4u, infoDataHandle);
            totalBytesReadm = totalBytesReadl + Demo_Read((unsigned __int8 *)&demo.info.month, 4u, infoDataHandle);
            totalBytesReadn = totalBytesReadm + Demo_Read((unsigned __int8 *)&demo.info.year, 4u, infoDataHandle);
            totalBytesReado = totalBytesReadn + Demo_Read((unsigned __int8 *)&demo.info.day, 4u, infoDataHandle);
            totalBytesReadp = totalBytesReado + Demo_Read((unsigned __int8 *)&demo.info.hour, 4u, infoDataHandle);
            totalBytesReada = totalBytesReadp + Demo_Read((unsigned __int8 *)&demo.info.min, 4u, infoDataHandle);
            for ( i = 0; i < demo.info.numConnectedPlayersInfoCount; ++i )
                totalBytesReada += Demo_Read(&demo.info.connectedPlayers[i].playerIndex, 0x18u, infoDataHandle);
            for ( j = 0; j < demo.info.numDefaultBookmarksCount; ++j )
                totalBytesReada += Demo_Read(&demo.info.defaultBookmarks[j].type, 0xCu, infoDataHandle);
            for ( k = 0; k < demo.info.numHeliPatchesCount; ++k )
                totalBytesReada += Demo_Read((unsigned __int8 *)&demo.info.heliPatches[k], 0x18u, infoDataHandle);
            Demo_RegisterHeliPatches();
            v4 = va("DEMO Info: Read %d bytes to demo file %s.info\n", totalBytesReada, demo.demoName);
            Demo_Printf(258, v4);
            if ( !handle )
                Demo_CloseFile(infoDataHandle);
            return 1;
        }
        else
        {
            if ( handle )
                Com_Error(ERR_DROP, "EXE_ERR_INVALID_DEMO_FILE");
            else
                Demo_CloseFile(infoDataHandle);
            return 0;
        }
    }
    else
    {
        if ( handle )
            Com_Error(ERR_DROP, "EXE_ERR_INVALID_DEMO_FILE");
        else
            Demo_CloseFile(infoDataHandle);
        return 0;
    }
}

void __cdecl Demo_SetDefaultClient(int localClientNum, int defaultClient)
{
    __int64 v2; // rax
    int i; // [esp+4h] [ebp-24h]
    clientActive_t *LocalClientGlobals; // [esp+10h] [ebp-18h]
    int controllerIndex; // [esp+18h] [ebp-10h]
    bool foundDefaultClient; // [esp+1Fh] [ebp-9h]
    int clientNum; // [esp+20h] [ebp-8h]
    int firstclient; // [esp+24h] [ebp-4h]

    foundDefaultClient = 0;
    clientNum = -1;
    firstclient = 0;
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( !Demo_IsClipPlaying() )
    {
        if ( Live_IsSignedIn(controllerIndex) )
        {
            LODWORD(v2) = Live_GetXuid(controllerIndex);
            for ( i = 0; i < demo.info.numConnectedPlayersInfoCount; ++i )
            {
                if ( !firstclient && v2 )
                    firstclient = i;
                if ( __PAIR64__(HIDWORD(demo.info.connectedPlayers[i].xuid), demo.info.connectedPlayers[i].xuid) == v2 )
                {
                    clientNum = demo.info.connectedPlayers[i].clientNum;
                    foundDefaultClient = 1;
                    break;
                }
            }
        }
        if ( !foundDefaultClient )
            clientNum = firstclient;
        if ( defaultClient >= 0 )
            clientNum = defaultClient;
        if ( LocalClientGlobals->snap.serverTime < demo.info.connectedPlayers[Demo_GetPlayerIndexForClientNum(clientNum, -1)].connectTime )
            clientNum = Demo_GetDemoClientIndex();
        Dvar_SetInt((dvar_s *)demo_client, clientNum);
    }
}

void __cdecl Demo_SetDefaultVisionSet(int localClientNum)
{
    const char *String; // eax
    const char *v2; // eax
    clientConnection_t *clc; // [esp+0h] [ebp-8h]
    int index; // [esp+4h] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    index = ++clc->serverCommandSequence & 0x7F;
    String = Dvar_GetString("mapname");
    v2 = va("%c %i \"%s\" 0", 100, 1550, String);
    I_strncpyz(clc->serverCommands[index], v2, 1024);
}

void __cdecl Demo_UpdateCurrentSnapshot(int localClientNum, playerState_s *ps)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-10h]
    cg_s *cgameGlob; // [esp+4h] [ebp-Ch]
    cgs_t *cgs; // [esp+8h] [ebp-8h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    if ( ps->weapon )
        cgameGlob->weaponSelect = ps->weapon;
    if ( LocalClientGlobals->snap.ps.weapon != ps->weapon )
    {
        cgameGlob->weaponSelectTime = cgameGlob->time;
        CG_MenuShowNotify(localClientNum, 1);
    }
    memcpy((unsigned __int8 *)&LocalClientGlobals->snap.ps, (unsigned __int8 *)ps, sizeof(LocalClientGlobals->snap.ps));
    memcpy(
        (unsigned __int8 *)&cgameGlob->predictedPlayerState,
        (unsigned __int8 *)ps,
        sizeof(cgameGlob->predictedPlayerState));
    memcpy(
        (unsigned __int8 *)&LocalClientGlobals->snapshots[LocalClientGlobals->snap.messageNum & 0x1F],
        (unsigned __int8 *)&LocalClientGlobals->snap,
        sizeof(LocalClientGlobals->snapshots[LocalClientGlobals->snap.messageNum & 0x1F]));
    CG_TransitionSnapshot(localClientNum);
    --cgs->processedSnapshotNum;
    Demo_SetupPlaybackForClient(localClientNum, &LocalClientGlobals->snap.ps);
}

void __cdecl Demo_EnableSnapshotProcessing()
{
    svs.mapCenter[0] = cls.mapCenter[0];
    svs.mapCenter[1] = cls.mapCenter[1];
    svs.mapCenter[2] = cls.mapCenter[2];
    SV_SetServerStaticHeader();
}

void __cdecl Demo_DisableSnapshotProcessing()
{
    SV_GetServerStaticHeader();
}

void    Demo_GenerateUncompressedSnapshot(
                int localClientNum,
                msg_t *msg,
                int msgSequence,
                int cmdSequence,
                int serverTime,
                int physicsTime,
                int snapFlags,
                bool writeAllPlayerStates)
{
    //void *v9; // esp
    const char *v10; // eax
    const char *v11; // eax
    const char *v12; // eax
    int v13; // eax
    const char *v14; // eax
    const char *v15; // eax
    const char *v16; // eax
    int v17; // [esp-28A0h] [ebp-28ACh]
    int v18; // [esp-289Ch] [ebp-28A8h]
    int v19; // [esp-2898h] [ebp-28A4h]
    int v20; // [esp-2894h] [ebp-28A0h]
    clientActive_t *LocalClientGlobals; // [esp-288Ch] [ebp-2898h]
    int i; // [esp-2888h] [ebp-2894h]
    int v23; // [esp-2884h] [ebp-2890h]
    int v24; // [esp-2884h] [ebp-2890h]
    int v25; // [esp-2884h] [ebp-2890h]
    int UsedBitCount; // [esp-2880h] [ebp-288Ch]
    int v27; // [esp-2880h] [ebp-288Ch]
    int v28; // [esp-2880h] [ebp-288Ch]
    int v29; // [esp-2880h] [ebp-288Ch]
    clientState_s v30; // [esp-2878h] [ebp-2884h] BYREF
    playerState_s v31; // [esp-27A8h] [ebp-27B4h] BYREF
    MatchState v32[2]; // [esp-100h] [ebp-10Ch] BYREF
    unsigned int v33[3]; // [esp+0h] [ebp-Ch] BYREF
    //_UNKNOWN *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v33[0] = a1;
    //v33[1] = retaddr;
    //v9 = alloca(10496);
    memset(v32, 0, 0x80u);
    memcpy((unsigned __int8 *)&v31, (unsigned __int8 *)&g_defaultPlayerState, sizeof(v31));
    memset(&v30, 0, sizeof(v30));
    Demo_EnableSnapshotProcessing();
    MSG_GetUsedBitCount(msg);
    v10 = va(
                    "CL_GenerateUncompressedSnapshot() - msgType: %d msgSeq: %d clientCmd: %d ServerTime: %d PhysTime: %d snapFlags: %d\n",
                    3,
                    msgSequence,
                    cmdSequence,
                    serverTime,
                    physicsTime,
                    snapFlags);
    Demo_Printf(131, v10);
    MSG_WriteBits(msg, 3, 2u);
    MSG_ClearLastReferencedEntity(msg);
    MSG_WriteLong(msg, msgSequence);
    MSG_WriteLong(msg, cmdSequence);
    MSG_WriteByte(msg, 0xCu);
    MSG_WriteLong(msg, serverTime);
    MSG_WriteLong(msg, physicsTime);
    MSG_WriteByte(msg, 0);
    MSG_WriteByte(msg, snapFlags);
    UsedBitCount = MSG_GetUsedBitCount(msg);
    MSG_ClearLastReferencedEntity(msg);
    MSG_WriteDeltaMatchState(&g_snapInfo, msg, svsHeader.time, v32, &demo.matchState);
    v23 = MSG_GetUsedBitCount(msg) - UsedBitCount;
    v11 = va("DEMO: UncompressedSnapshot Type: MatchState Size: %d bytes\n", v23 / 8);
    Demo_Printf(9, v11);
    for ( i = 0; i < demo.header.maxClients; ++i )
    {
        g_snapInfo.clientNum = i;
        if ( demo.ps[i].commandTime )
        {
            v27 = MSG_GetUsedBitCount(msg);
            if ( writeAllPlayerStates || demo.ps[i].clientNum == demo_client->current.integer )
            {
                MSG_WriteBit1(msg);
                MSG_WriteDeltaPlayerstate(&g_snapInfo, msg, serverTime, &v31, &demo.ps[i]);
            }
            else
            {
                MSG_WriteBit0(msg);
            }
            v24 = MSG_GetUsedBitCount(msg) - v27;
            v12 = va("DEMO: UncompressedSnapshot Type: PlayerState Client: %d Size: %d bytes\n", i, v24 / 8);
            Demo_Printf(17, v12);
        }
        else
        {
            MSG_WriteBit0(msg);
        }
    }
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    g_snapInfo.clientNum = g_democlientindex;
    g_snapInfo.fromBaseline = 1;
    MSG_ClearLastReferencedEntity(msg);
    MSG_WriteBits(msg, demo.prevNumEntities, 0xBu);
    v20 = 0;
    v19 = -1;
    while ( v20 < demo.prevNumEntities )
    {
        if ( demo.ent[++v19].eType != -1 )
        {
            ++v20;
            v28 = MSG_GetUsedBitCount(msg);
            MSG_WriteEntity(
                &g_snapInfo,
                msg,
                serverTime,
                &LocalClientGlobals->entityBaselines[v19],
                &demo.ent[v19],
                DELTA_FLAGS_FORCE);
            if ( MSG_GetUsedBitCount(msg) == v28
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                            4385,
                            0,
                            "%s",
                            "MSG_GetUsedBitCount( msg ) - bitsStart") )
            {
                __debugbreak();
            }
            if ( demo.entClientMask[v19][0] || demo.entClientMask[v19][1] )
            {
                MSG_WriteBit1(msg);
                MSG_WriteBits(msg, demo.entClientMask[v19][0], 0x20u);
            }
            else
            {
                MSG_WriteBit0(msg);
            }
            v13 = MSG_GetUsedBitCount(msg);
            v14 = va(
                            "DEMO: UncompressedSnapshot Type: PacketEntities Entity: %d Size: %d bits\n",
                            demo.ent[v19].number,
                            v13 - v28);
            Demo_Printf(32, v14);
        }
    }
    g_snapInfo.fromBaseline = 0;
    MSG_WriteEntityIndex(&g_snapInfo, msg, 1023, 10);
    v15 = va("DEMO: UncompressedSnapshot Type: PacketEntities NumEntities: %d FirstEntity: 0\n", demo.prevNumEntities);
    Demo_Printf(33, v15);
    MSG_ClearLastReferencedEntity(msg);
    MSG_WriteBits(msg, demo.prevNumClients, 5u);
    v29 = MSG_GetUsedBitCount(msg);
    v18 = 0;
    v17 = -1;
    while ( v18 < demo.prevNumClients )
    {
        if ( demo.client[++v17].clientIndex >= 0 )
        {
            MSG_WriteDeltaClient(&g_snapInfo, msg, serverTime, &v30, &demo.client[v17], 1);
            ++v18;
        }
    }
    MSG_WriteBit0(msg);
    v25 = MSG_GetUsedBitCount(msg) - v29;
    v16 = va(
                    "DEMO: UncompressedSnapshot Type: PacketClients NumClients: %d FirstClient: 0 Size: %d bytes\n",
                    demo.prevNumClients,
                    v25 / 8);
    Demo_Printf(65, v16);
    GlassCl_WriteDemoSnapshot(msg);
    MSG_WriteBit1(msg);
    Demo_WriteCustomSnapshotInformation(localClientNum, msg);
    Demo_WriteConfigStrings(localClientNum, msg);
    MSG_WriteByte(msg, 0xEu);
    Demo_DisableSnapshotProcessing();
}

void __cdecl Demo_WriteCustomSnapshotInformation(int localClientNum, msg_t *msg)
{
    const XAnim_s *Anims; // eax
    const char *v3; // eax
    const XAnim_s *v4; // eax
    const char *v5; // eax
    float f; // [esp+0h] [ebp-34h]
    float fa; // [esp+0h] [ebp-34h]
    signed int i; // [esp+18h] [ebp-1Ch]
    XAnimTree_s *tree; // [esp+1Ch] [ebp-18h]
    centity_s *cent; // [esp+24h] [ebp-10h]
    unsigned int anim; // [esp+30h] [ebp-4h]
    unsigned int anima; // [esp+30h] [ebp-4h]

    for ( i = 0; i < demo.header.maxClients; ++i )
    {
        if ( demo.ps[i].commandTime > 0
            && (MSG_WriteBit1(msg), cent = CG_GetEntity(localClientNum, i), Com_GetClientDObj(i, localClientNum)) )
        {
            MSG_WriteBit1(msg);
            tree = CScr_GetEntAnimTree(cent);
            anim = demo.ps[i].torsoAnim & 0xFFFFFBFF;
            if ( anim )
            {
                Anims = XAnimGetAnims(tree);
                if ( !XAnimHasTime(Anims, anim) )
                {
                    v3 = va("XAnimHasTime() failed for the torso anim %d\n", anim);
                    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp", 4553, 0, v3) )
                        __debugbreak();
                }
                f = XAnimGetTime(tree, anim);
                MSG_WriteRangedFloatBits(msg, f, 0.0, 1.0, 0x10u);
            }
            else
            {
                MSG_WriteRangedFloatBits(msg, 0.0, 0.0, 1.0, 0x10u);
            }
            anima = demo.ps[i].legsAnim & 0xFFFFFBFF;
            if ( anima )
            {
                v4 = XAnimGetAnims(tree);
                if ( !XAnimHasTime(v4, anima) )
                {
                    v5 = va("XAnimHasTime() failed for the legs anim %d\n", anima);
                    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp", 4570, 0, v5) )
                        __debugbreak();
                }
                fa = XAnimGetTime(tree, anima);
                MSG_WriteRangedFloatBits(msg, fa, 0.0, 1.0, 0x10u);
            }
            else
            {
                MSG_WriteRangedFloatBits(msg, 0.0, 0.0, 1.0, 0x10u);
            }
        }
        else
        {
            MSG_WriteBit0(msg);
        }
    }
    Demo_Destructible_WriteGameState(msg, localClientNum);
}

void __cdecl Demo_ReadCustomSnaphotInformation(int localClientNum, msg_t *msg)
{
    signed int i; // [esp+24h] [ebp-2Ch]
    XAnimTree_s *tree; // [esp+28h] [ebp-28h]
    DObj *obj; // [esp+2Ch] [ebp-24h]
    float animTime; // [esp+30h] [ebp-20h]
    float animTimea; // [esp+30h] [ebp-20h]
    centity_s *cent; // [esp+34h] [ebp-1Ch]
    unsigned int anim; // [esp+40h] [ebp-10h]
    unsigned int anima; // [esp+40h] [ebp-10h]
    float legsWeight; // [esp+48h] [ebp-8h]

    for ( i = 0; i < demo.header.maxClients; ++i )
    {
        if ( MSG_ReadBit(msg) && MSG_ReadBit(msg) )
        {
            cent = CG_GetEntity(localClientNum, i);
            obj = Com_GetClientDObj(i, localClientNum);
            if ( obj )
            {
                tree = CScr_GetEntAnimTree(cent);
                XAnimClearTreeGoalWeights(tree, 0, 0.0, -1);
                anim = demo.ps[i].torsoAnim & 0xFFFFFBFF;
                legsWeight = 1.0f;
                animTime = MSG_ReadRangedFloatBits(msg, 0.0, 1.0, 0x10u);
                if ( anim )
                {
                    XAnimSetCompleteGoalWeight(obj, anim, 1.0, 0.0, 1.0, 0, 0, 0, -1);
                    XAnimSetTime(tree, anim, animTime, 0xFFFFu);
                    legsWeight = 0.01f;
                }
                anima = demo.ps[i].legsAnim & 0xFFFFFBFF;
                animTimea = MSG_ReadRangedFloatBits(msg, 0.0, 1.0, 0x10u);
                if ( anima )
                {
                    XAnimSetCompleteGoalWeight(obj, anima, legsWeight, 0.0, 1.0, 0, 0, 0, -1);
                    XAnimSetTime(tree, anima, animTimea, 0xFFFFu);
                }
            }
            else
            {
                MSG_ReadRangedFloatBits(msg, 0.0, 1.0, 0x10u);
                MSG_ReadRangedFloatBits(msg, 0.0, 1.0, 0x10u);
            }
        }
    }
    Demo_Destructible_ReadGameState(msg, localClientNum);
}

void __cdecl Demo_UpdateDesiredTime(int localClientNum)
{
    char *v1; // eax
    int v2; // eax
    int v3; // eax
    char *v4; // [esp-4h] [ebp-14h]
    char *v5; // [esp-4h] [ebp-14h]
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-10h]
    int client; // [esp+4h] [ebp-Ch]
    int controllerIndex; // [esp+8h] [ebp-8h]
    int serverTime; // [esp+Ch] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if ( Demo_IsPlaybackInited() )
    {
        controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        serverTime = demo_desiredTime->current.integer;
        client = demo_desiredClient->current.integer;
        if ( client >= 0 && serverTime >= 0 )
        {
            if ( demo_client->current.integer != client )
            {
                v1 = va("demo_switchplayer %d 1 %d", demo_cmdNum->current.integer, client);
                Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, v1);
            }
            if ( serverTime < LocalClientGlobals->snap.serverTime - 5000
                || serverTime > LocalClientGlobals->snap.serverTime + 5000 )
            {
                if ( serverTime >= LocalClientGlobals->snap.serverTime )
                {
                    if ( serverTime > LocalClientGlobals->snap.serverTime )
                    {
                        v5 = va("demo_forward %d %d", demo_cmdNum->current.integer, serverTime);
                        v3 = Com_LocalClient_GetControllerIndex(localClientNum);
                        Cmd_ExecuteSingleCommand(localClientNum, v3, v5);
                    }
                }
                else
                {
                    v4 = va("demo_back %d %d", demo_cmdNum->current.integer, serverTime);
                    v2 = Com_LocalClient_GetControllerIndex(localClientNum);
                    Cmd_ExecuteSingleCommand(localClientNum, v2, v4);
                }
            }
            Dvar_SetBool((dvar_s *)demo_pause, 1);
            Dvar_SetInt((dvar_s *)demo_desiredTime, -1);
            Dvar_SetInt((dvar_s *)demo_desiredClient, -1);
        }
    }
}

void __cdecl Demo_WriteConfigStrings(int localClientNum, msg_t *msg)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    int configStringCount; // [esp+34h] [ebp-2Ch]
    int numConfigStringsWritten; // [esp+38h] [ebp-28h]
    int bitsUsed; // [esp+3Ch] [ebp-24h]
    bool writeConfigString; // [esp+42h] [ebp-1Eh]
    int nextConstConfigStringIndex; // [esp+44h] [ebp-1Ch]
    int nextConstConfigStringIndexa; // [esp+44h] [ebp-1Ch]
    int lastStringIndex; // [esp+48h] [ebp-18h]
    const char *constValue; // [esp+4Ch] [ebp-14h]
    const char *constValuea; // [esp+4Ch] [ebp-14h]
    char *s; // [esp+50h] [ebp-10h]
    char *sb; // [esp+50h] [ebp-10h]
    char *sa; // [esp+50h] [ebp-10h]
    int nextConstConfigStringNumber; // [esp+54h] [ebp-Ch]
    int nextConstConfigStringNumbera; // [esp+54h] [ebp-Ch]
    int i; // [esp+58h] [ebp-8h]
    int ia; // [esp+58h] [ebp-8h]
    int bitsStart; // [esp+5Ch] [ebp-4h]

    PROF_SCOPED("Demo - Write Config Strings");

    CL_GetLocalClientGlobals(localClientNum);
    MSG_WriteByte(msg, 2u);
    configStringCount = 0;
    nextConstConfigStringIndex = 0;
    nextConstConfigStringNumber = CCS_GetConfigStringNum(0);
    if ( !nextConstConfigStringNumber )
    {
        v2 = va(
                     "ERROR: Could not load the configstring table for the current map and gametype. Make sure that the configstrin"
                     "g table is built for this map and game type.");
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp", 4737, 0, v2) )
            __debugbreak();
    }
    for ( i = 0; i < MAX_CONFIGSTRINGS; ++i )
    {
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
    bitsStart = MSG_GetUsedBitCount(msg);
    MSG_WriteShort(msg, configStringCount);
    nextConstConfigStringIndexa = 0;
    nextConstConfigStringNumbera = CCS_GetConfigStringNum(0);
    numConfigStringsWritten = 0;
    lastStringIndex = -1;
    for ( ia = 0; ia < MAX_CONFIGSTRINGS; ++ia )
    {
        if ( ia == 1 )
        {
            sb = Dvar_InfoString_Big(8);
            MSG_WriteBit1(msg);
            MSG_WriteBigString(msg, sb);
            lastStringIndex = 1;
            ++numConfigStringsWritten;
        }
        else
        {
            sa = &cls.gameState.stringData[cls.gameState.stringOffsets[ia]];
            if ( ia )
            {
                if ( nextConstConfigStringNumbera )
                {
                    if ( nextConstConfigStringNumbera < ia )
                    {
                        v3 = va(
                                     "Invalid line in the config string csv - config string indexes must only go up, but it went down and i"
                                     "s now %i (line %i) - check the bottom of the file to see if it's corrupt",
                                     nextConstConfigStringNumbera,
                                     nextConstConfigStringIndexa);
                        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp", 4825, 0, v3) )
                            __debugbreak();
                    }
                }
                if ( nextConstConfigStringNumbera == ia )
                {
                    writeConfigString = 0;
                    constValuea = CCS_GetConfigStringValue(nextConstConfigStringIndexa);
                    if ( ia >= 1547 && I_stricmp(constValuea, sa) )
                    {
                        writeConfigString = 1;
                    }
                    else if ( ia >= 1547 || !strcmp(constValuea, sa) )
                    {
                        if ( !cls.gameState.stringOffsets[ia] )
                        {
                            v4 = va("PROCESSING: (<INSENSITIVE) nextConstConfigStringNumber == i -- ConfigString: %d\n", ia);
                            Demo_Printf(1, v4);
                            writeConfigString = 1;
                        }
                    }
                    else
                    {
                        writeConfigString = 1;
                    }
                    if ( writeConfigString )
                    {
                        if ( cls.gameState.stringOffsets[ia] )
                        {
                            if ( ia == lastStringIndex + 1 )
                            {
                                MSG_WriteBit1(msg);
                            }
                            else
                            {
                                MSG_WriteBit0(msg);
                                MSG_WriteBits(msg, ia, 0xCu);
                            }
                            MSG_WriteBigString(msg, sa);
                            lastStringIndex = ia;
                            ++numConfigStringsWritten;
                        }
                        else
                        {
                            MSG_WriteBit0(msg);
                            MSG_WriteBits(msg, ia, 0xCu);
                            MSG_WriteBigString(msg, (char *)"");
                            lastStringIndex = ia;
                            ++numConfigStringsWritten;
                        }
                    }
                    nextConstConfigStringNumbera = CCS_GetConfigStringNum(++nextConstConfigStringIndexa);
                }
                else if ( cls.gameState.stringOffsets[ia] )
                {
                    if ( ia == lastStringIndex + 1 )
                    {
                        MSG_WriteBit1(msg);
                    }
                    else
                    {
                        MSG_WriteBit0(msg);
                        MSG_WriteBits(msg, ia, 0xCu);
                    }
                    MSG_WriteBigString(msg, sa);
                    ++numConfigStringsWritten;
                }
            }
            else
            {
                MSG_WriteBit1(msg);
                MSG_WriteBigString(msg, sa);
                lastStringIndex = 0;
                ++numConfigStringsWritten;
            }
        }
    }
    if ( numConfigStringsWritten != configStringCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                    4899,
                    0,
                    "%s",
                    "numConfigStringsWritten == configStringCount") )
    {
        __debugbreak();
    }
    bitsUsed = MSG_GetUsedBitCount(msg) - bitsStart;
    v5 = va("Demo_WriteConfigStrings() - NumConfigStrings: %d Size: %d bytes\n", configStringCount, bitsUsed / 8);
    Demo_Printf(131, v5);
}

void __cdecl Demo_ReadConfigStrings(int localClientNum, msg_t *msg)
{
    const char *v2; // eax
    unsigned int v3; // [esp+0h] [ebp-68h]
    unsigned int v4; // [esp+10h] [ebp-58h]
    unsigned int v5; // [esp+20h] [ebp-48h]
    unsigned int configStringIndex; // [esp+3Ch] [ebp-2Ch]
    int bitsUsed; // [esp+40h] [ebp-28h]
    int newConfigStringsCount; // [esp+48h] [ebp-20h]
    int nextConstConfigStringIndex; // [esp+4Ch] [ebp-1Ch]
    int lastStringIndex; // [esp+54h] [ebp-14h]
    int numConfigStrings; // [esp+58h] [ebp-10h]
    char *sa; // [esp+5Ch] [ebp-Ch]
    char *sb; // [esp+5Ch] [ebp-Ch]
    char *s; // [esp+5Ch] [ebp-Ch]
    char *sc; // [esp+5Ch] [ebp-Ch]
    int nextConstConfigStringNumber; // [esp+60h] [ebp-8h]
    int bitsStart; // [esp+64h] [ebp-4h]

    PROF_SCOPED("Demo - Read Config Strings");

    CL_GetLocalClientGlobals(localClientNum);
    bitsStart = MSG_GetNumBitsRead(msg);
    numConfigStrings = MSG_ReadShort(msg);
    nextConstConfigStringIndex = 0;
    nextConstConfigStringNumber = CCS_GetConfigStringNum(0);
    newConfigStringsCount = 0;
    while ( !nextConstConfigStringNumber && nextConstConfigStringIndex < MAX_CONFIGSTRINGS)
        nextConstConfigStringNumber = CCS_GetConfigStringNum(++nextConstConfigStringIndex);
    cls.gameState.dataCount = 1;
    lastStringIndex = -1;
    while ( newConfigStringsCount < numConfigStrings )
    {
        if ( MSG_ReadBit(msg) )
            configStringIndex = lastStringIndex + 1;
        else
            configStringIndex = MSG_ReadBits(msg, 0xCu);
        if ( configStringIndex >= 0xCBC
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                        4954,
                        0,
                        "%s",
                        "configStringIndex >= 0 && configStringIndex < MAX_CONFIGSTRINGS") )
        {
            __debugbreak();
        }
        while ( nextConstConfigStringNumber && nextConstConfigStringNumber < (int)configStringIndex )
        {
            sa = (char *)CCS_GetConfigStringValue(nextConstConfigStringIndex);
            v5 = strlen(sa);
            cls.gameState.stringOffsets[nextConstConfigStringNumber] = cls.gameState.dataCount;
            memcpy((unsigned __int8 *)&cls.gameState.stringData[cls.gameState.dataCount], (unsigned __int8 *)sa, v5 + 1);
            cls.gameState.dataCount += v5 + 1;
            nextConstConfigStringNumber = CCS_GetConfigStringNum(++nextConstConfigStringIndex);
        }
        if ( nextConstConfigStringNumber && nextConstConfigStringNumber == configStringIndex )
            nextConstConfigStringNumber = CCS_GetConfigStringNum(++nextConstConfigStringIndex);
        sb = MSG_ReadBigString(msg);
        v4 = strlen(sb);
        s = Demo_AdjustTimeForConfigString(configStringIndex, sb);
        if ( (int)(v4 + cls.gameState.dataCount + 1) >= 0x10000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                        4983,
                        0,
                        "%s",
                        "len + 1 + cls.gameState.dataCount < MAX_GAMESTATE_CHARS") )
        {
            __debugbreak();
        }
        cls.gameState.stringOffsets[configStringIndex] = cls.gameState.dataCount;
        memcpy((unsigned __int8 *)&cls.gameState.stringData[cls.gameState.dataCount], (unsigned __int8 *)s, v4 + 1);
        cls.gameState.dataCount += v4 + 1;
        lastStringIndex = configStringIndex;
        ++newConfigStringsCount;
    }
    while ( nextConstConfigStringNumber )
    {
        sc = (char *)CCS_GetConfigStringValue(nextConstConfigStringIndex);
        v3 = strlen(sc);
        cls.gameState.stringOffsets[nextConstConfigStringNumber] = cls.gameState.dataCount;
        memcpy((unsigned __int8 *)&cls.gameState.stringData[cls.gameState.dataCount], (unsigned __int8 *)sc, v3 + 1);
        cls.gameState.dataCount += v3 + 1;
        nextConstConfigStringNumber = CCS_GetConfigStringNum(++nextConstConfigStringIndex);
    }
    bitsUsed = MSG_GetNumBitsRead(msg) - bitsStart;
    v2 = va("Demo_ReadConfigStrings() - NumConfigStrings: %d Size: %d bytes\n", numConfigStrings, bitsUsed / 8);
    Demo_Printf(131, v2);
}

void __cdecl Demo_UpdateConfigStrings(int localClientNum)
{
    CG_ParseCullDist();
    CG_ParseServerInfo(localClientNum);
    CG_ParseCodInfo(localClientNum);
    CL_ParseMapCenter();
    CG_ParseGameEndTime(localClientNum);
    CG_ParseFog(localClientNum);
    CG_NorthDirectionChanged(localClientNum);
    CG_MiniMapChanged(localClientNum);
}

bool __cdecl Demo_ShouldGenerateKeyFrameSnapshot(int localClientNum, int serverTime)
{
    if ( !Demo_IsEnabled() )
        return 0;
    if ( !Demo_IsPlaybackInited() )
        return 0;
    if ( !CG_GetLocalClientGlobals(localClientNum)->nextSnap )
        return 0;
    if ( demo.lastProcessedTime == -1 )
        return 1;
    return (serverTime - demo.lastProcessedTime) / 1000 >= demo_keyframerate->current.integer;
}

void __cdecl Demo_GenerateKeyFrameSnapshot(
                int localClientNum,
                int msgSequence,
                int cmdSequence,
                int serverTime,
                int physicsTime,
                int snapFlags)
{
    msg_t msg; // [esp+Ch] [ebp-30h] BYREF

    PROF_SCOPED("Keyframe - Generation");

    MSG_Init(&msg, demo.msgBuf1, 49152);
    if ( Demo_IsClipPlaying() )
    {
        demo.playback->forceWriteClipCommands = 1;
        Demo_WriteClipCommandsInternal(localClientNum, &msg, 0);
    }
    Demo_GenerateUncompressedSnapshot(
        localClientNum,
        &msg,
        msgSequence,
        cmdSequence,
        serverTime,
        physicsTime,
        snapFlags,
        1);
    MSG_Init(&demo.playback->keyframeMsg, demo.memBlock.compressedMsgBuf, 49152);
    MSG_Embed(&demo.playback->keyframeMsg, &msg);
    demo.playback->keyframeGenerated = 1;
}

void __cdecl Demo_WriteKeyFrameInformation(int localClientNum)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    int i; // [esp+8h] [ebp-10h]
    clientActive_t *LocalClientGlobals; // [esp+Ch] [ebp-Ch]
    clientConnection_t *clc; // [esp+14h] [ebp-4h]

    PROF_SCOPED("Keyframe - Writing");

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    clc = CL_GetLocalClientConnection(localClientNum);
    if ( demo.playback->keyframeMsg.cursize + demo.playback->keyframeBufferIndex > 0x500000)
    {
        v1 = va("Resetting demo.playback->keyframeBufferIndex since we ran out of keyframe memory\n");
        Demo_Printf(128, v1);
        demo.playback->keyframeBufferIndex = 0;
    }
    for ( i = 0; i < 250; ++i )
    {
        if ( demo.playback->keyFrame[i].keyframeBufferStart < demo.playback->keyframeMsg.cursize
                                                                                                                + demo.playback->keyframeBufferIndex
            && demo.playback->keyFrame[i].keyframeBufferStart >= demo.playback->keyframeBufferIndex
            || demo.playback->keyFrame[i].size + demo.playback->keyFrame[i].keyframeBufferStart > demo.playback->keyframeBufferIndex
            && demo.playback->keyFrame[i].size + demo.playback->keyFrame[i].keyframeBufferStart <= demo.playback->keyframeMsg.cursize
                                                                                                                                                                                     + demo.playback->keyframeBufferIndex )
        {
            demo.playback->keyFrame[i].keyframeSnapshotTime = -1;
            v2 = va(
                         "Keyframe %d is now invalidated since    demo.playback->keyFrame[i].keyframeBufferStart(%d) < demo.playback->k"
                         "eyframeBufferIndex(%d) + demo.playback->keyframeMsg.cursize(%d) \n",
                         i,
                         demo.playback->keyFrame[i].keyframeBufferStart,
                         demo.playback->keyframeBufferIndex,
                         demo.playback->keyframeMsg.cursize);
            Demo_Printf(128, v2);
        }
    }
    demo.playback->keyframeIndex = (demo.playback->keyframeIndex + 1) % 250;
    demo.playback->keyFrame[demo.playback->keyframeIndex].demoFileMemoryBlockStart = demo.totalSize
                                                                                                                                                                 - demo.memBlock.compressedSize
                                                                                                                                                                 - 8;
    demo.playback->keyFrame[demo.playback->keyframeIndex].keyframeBufferStart = demo.playback->keyframeBufferIndex;
    demo.playback->keyFrame[demo.playback->keyframeIndex].firstCmdSequence = demo.playback->lastProcessedCmd;
    demo.playback->keyFrame[demo.playback->keyframeIndex].size = demo.playback->keyframeMsg.cursize;
    MSG_SetBookmark(&demo.msg, &demo.playback->keyFrame[demo.playback->keyframeIndex].msgBookmark);
    memcpy(
        &g_keyframeBuf[demo.playback->keyframeBufferIndex],
        demo.playback->keyframeMsg.data,
        demo.playback->keyframeMsg.cursize);
    demo.playback->keyframeBufferIndex += demo.playback->keyframeMsg.cursize;
    if ( Demo_IsClipPlaying() )
    {
        demo.playback->keyFrame[demo.playback->keyframeIndex].keyframeClipServerTime = demo.playback->clipServerTime;
        demo.playback->keyFrame[demo.playback->keyframeIndex].keyframeSnapshotTime = Demo_GetClipTime();
        demo.lastProcessedTime = Demo_GetClipTime();
    }
    else
    {
        demo.playback->keyFrame[demo.playback->keyframeIndex].keyframeSnapshotTime = LocalClientGlobals->serverTime;
        demo.lastProcessedTime = LocalClientGlobals->serverTime;
    }
    demo.playback->lastProcessedCmd = clc->serverCommandSequence;
    demo.playback->keyframeGenerated = 0;
    v3 = va("Keyframe w - Time: %d\n", demo.playback->keyFrame[demo.playback->keyframeIndex].keyframeSnapshotTime);
    Demo_Printf(128, v3);
    v4 = va(
                 "Keyframe w - File Related: FileStart: %d\n",
                 demo.playback->keyFrame[demo.playback->keyframeIndex].demoFileMemoryBlockStart);
    Demo_Printf(128, v4);
    v5 = va(
                 "Keyframe w - Demo Msg: Size: %d 1stByte: %d LastByte: %d\n",
                 demo.msg.cursize,
                 *demo.msg.data,
                 demo.msg.data[demo.msg.cursize - 1]);
    Demo_Printf(128, v5);
    v6 = va("Keyframe w - BookMark: ReadCount: %d Msg: %d\n", demo.msg.readcount, demo.msg.data[demo.msg.readcount]);
    Demo_Printf(128, v6);
    v7 = va("Keyframe w - BookMark: Bit: %d Msg:%d\n", demo.msg.bit, demo.msg.data[demo.msg.bit >> 3]);
    Demo_Printf(128, v7);
    v8 = va(
                 "Keyframe w - KeyframeMsg: Index: %d BufIndex: %d Time: %d Size: %d 1stByte: %d LastByte: %d\n",
                 demo.playback->keyframeIndex,
                 demo.playback->keyFrame[demo.playback->keyframeIndex].keyframeBufferStart,
                 demo.playback->keyFrame[demo.playback->keyframeIndex].keyframeSnapshotTime,
                 demo.playback->keyframeMsg.cursize,
                 *demo.playback->keyframeMsg.data,
                 demo.playback->keyframeMsg.data[demo.playback->keyframeMsg.cursize - 1]);
    Demo_Printf(128, v8);
}

void __cdecl Demo_InitClipRecord(int localClientNum, bool newClip)
{
    int ControllerIndex; // eax
    char *ClientName; // eax
    char *MapName; // eax
    const char *v5; // eax
    char *v6; // eax
    char *s; // [esp+0h] [ebp-4h]

    MSG_Init(&demo.playback->clipRecordingMsg, demo.playback->subClipRecordingBuf, 49152);
    if ( newClip )
    {
        Demo_WriteHeader(&demo.playback->clipRecordingMsg, 1);
        MSG_WriteBits(&demo.playback->clipRecordingMsg, 3, 2u);
        MSG_WriteLong(&demo.playback->clipRecordingMsg, 0);
        Demo_WriteClipGameState(localClientNum);
        MSG_WriteBits(&demo.playback->clipRecordingMsg, 0, 2u);
        Demo_SaveClipRecordMessageToMainMemory(
            localClientNum,
            demo.playback->clipRecordingMsg.data,
            demo.playback->clipRecordingMsg.cursize,
            0);
        demo.playback->segmentGameStateSize = demo.playback->clipRecordBufIndex;
        demo.playback->clipRecordInfo.startTime = 0;
        ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        ClientName = Live_ControllerIndex_GetClientName(ControllerIndex);
        I_strncpyz(demo.playback->clipRecordInfo.author, ClientName, 32);
        Demo_PopulateStaticInfoData(&demo.playback->clipRecordInfo);
        MapName = UI_GetMapName(demo.playback->clipRecordInfo.mapName, 1);
        v5 = va("%s_CAPS", MapName);
        v6 = UI_SafeTranslateString(v5);
        sprintf(demo.playback->clipRecordInfo.name, "%s", v6);
        demo.playback->clipRecordInfo.isModifiedName = 0;
        s = UI_GetGameTypeName(demo.playback->clipRecordInfo.gameType, 0);
        sprintf(demo.playback->clipRecordInfo.description, "%s", s);
        demo.playback->clipRecordInfo.isModifiedDescription = 0;
    }
}

void __cdecl Demo_WriteUncompressedClipSnapshot(int localClientNum)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-4h]
    int savedregs; // [esp+4h] [ebp+0h] BYREF

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    Demo_WriteClipCommands(localClientNum, &demo.playback->clipRecordingMsg, 1);
    Demo_GenerateUncompressedSnapshot(
        localClientNum,
        &demo.playback->clipRecordingMsg,
        LocalClientGlobals->snap.messageNum,
        LocalClientGlobals->snap.serverCommandNum,
        LocalClientGlobals->snap.serverTime,
        LocalClientGlobals->snap.physicsTime,
        LocalClientGlobals->snap.snapFlags,
        0);
    memcpy(
        (unsigned __int8 *)&demo.playback->clipRecordPS,
        (unsigned __int8 *)&demo.ps[demo_client->current.integer],
        sizeof(demo.playback->clipRecordPS));
}

void __cdecl Demo_WriteClipCommands(int localClientNum, msg_t *msg, bool setClipRecordParams)
{
    if ( Demo_ShouldFlushClipRecordMessage() )
    {
        MSG_WriteBits(msg, 0, 2u);
        Demo_SaveClipRecordMessageToMainMemory(localClientNum, msg->data, msg->cursize, 0);
    }
    Demo_WriteClipCommandsInternal(localClientNum, msg, setClipRecordParams);
}

void __cdecl Demo_WriteClipCommandsInternal(int localClientNum, msg_t *msg, bool setClipRecordParams)
{
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
    bool v13; // [esp+18h] [ebp-68h]
    bool v14; // [esp+1Ch] [ebp-64h]
    bool v15; // [esp+24h] [ebp-5Ch]
    bool v16; // [esp+28h] [ebp-58h]
    float *v17; // [esp+30h] [ebp-50h]
    float *clipCameraAngles; // [esp+40h] [ebp-40h]
    float *v19; // [esp+50h] [ebp-30h]
    float *clipCameraOrigin; // [esp+60h] [ebp-20h]
    clientActive_t *LocalClientGlobals; // [esp+70h] [ebp-10h]
    bool forceWrite; // [esp+77h] [ebp-9h]
    cg_s *cgameGlob; // [esp+78h] [ebp-8h]
    int cameraModeToCheck; // [esp+7Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    forceWrite = demo.playback->forceWriteClipCommands;
    if ( demo.playback->clipRecordClient != demo_client->current.integer || forceWrite )
    {
        MSG_WriteBits(msg, 2, 2u);
        MSG_WriteByte(msg, 0);
        MSG_WriteByte(msg, demo_client->current.integer);
        if ( Demo_IsClipRecording() && Demo_IsPaused() )
        {
            MSG_WriteBit1(msg);
            Demo_EnableSnapshotProcessing();
            MSG_WriteDeltaPlayerstate(
                &g_snapInfo,
                msg,
                0,
                &demo.playback->clipRecordPS,
                &demo.ps[demo_client->current.integer]);
            Demo_DisableSnapshotProcessing();
        }
        else
        {
            MSG_WriteBit0(msg);
        }
        if ( setClipRecordParams )
            demo.playback->clipRecordClient = demo_client->current.color[0];
        v3 = va("DEMO: w Clip Cmd: DEMO_CLIP_CMD_SET_CLIENT client: %d\n", demo_client->current.integer);
        Demo_Printf(1024, v3);
    }
    if ( demo.playback->clipRecordTimescale != demo_timescale->current.value || forceWrite )
    {
        MSG_WriteBits(msg, 2, 2u);
        MSG_WriteByte(msg, 1u);
        MSG_WriteRangedFloatBits(msg, demo_timescale->current.value, 0.0, 4.0, 0x10u);
        if ( setClipRecordParams )
            demo.playback->clipRecordTimescale = demo_timescale->current.value;
        v4 = va("DEMO: w Clip Cmd: DEMO_CLIP_CMD_SET_TIMESCALE Value: %f\n", demo_timescale->current.value);
        Demo_Printf(1024, v4);
    }
    if ( demo.playback->clipCameraMode != demo.playback->cameraMode || forceWrite )
    {
        MSG_WriteBits(msg, 2, 2u);
        MSG_WriteByte(msg, 2u);
        if ( setClipRecordParams )
        {
            MSG_WriteByte(msg, demo.playback->cameraMode);
            demo.playback->clipCameraMode = demo.playback->cameraMode;
            if ( demo.playback->clipCameraMode != demo.playback->cameraMode
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                            5338,
                            0,
                            "%s",
                            "demo.playback->clipCameraMode == demo.playback->cameraMode") )
            {
                __debugbreak();
            }
            v5 = va("DEMO: w Clip Cmd: DEMO_CLIP_CMD_SET_CAMERA_MODE Mode: %d\n", demo.playback->cameraMode);
            Demo_Printf(1024, v5);
        }
        else
        {
            MSG_WriteByte(msg, demo.playback->cameraMode);
            v6 = va("DEMO: w Clip Cmd: DEMO_CLIP_CMD_SET_CAMERA_MODE Mode: %d\n", demo.playback->cameraMode);
            Demo_Printf(1024, v6);
        }
    }
    if ( setClipRecordParams )
        cameraModeToCheck = demo.playback->clipCameraMode;
    else
        cameraModeToCheck = demo.playback->cameraMode;
    if ( !cameraModeToCheck && !forceWrite )
        goto LABEL_80;
    MSG_WriteBits(msg, 2, 2u);
    MSG_WriteByte(msg, 3u);
    if ( cameraModeToCheck >= 0 )
    {
        if ( cameraModeToCheck <= 1 )
        {
            v16 = cgameGlob->refdef.vieworg[0] == demo.playback->clipCameraOrigin[0]
                 && cgameGlob->refdef.vieworg[1] == demo.playback->clipCameraOrigin[1]
                 && cgameGlob->refdef.vieworg[2] == demo.playback->clipCameraOrigin[2];
            if ( v16 && !forceWrite )
                goto LABEL_49;
            MSG_WriteBit1(msg);
            MSG_WriteData(msg, (unsigned __int8 *)cgameGlob->refdef.vieworg, 0xCu);
            if ( setClipRecordParams )
            {
                clipCameraOrigin = demo.playback->clipCameraOrigin;
                demo.playback->clipCameraOrigin[0] = cgameGlob->refdef.vieworg[0];
                clipCameraOrigin[1] = cgameGlob->refdef.vieworg[1];
                clipCameraOrigin[2] = cgameGlob->refdef.vieworg[2];
            }
            v7 = va(
                         "DEMO: w Clip Cmd: DEMO_CLIP_CMD_SET_CAMERA_ORIGIN Value: %d (%f, %f, %f)\n",
                         demo.playback->cameraMode,
                         demo.playback->clipCameraOrigin[0],
                         demo.playback->clipCameraOrigin[1],
                         demo.playback->clipCameraOrigin[2]);
            Demo_Printf(1024, v7);
        }
        else
        {
            if ( cameraModeToCheck != 2 )
                goto LABEL_54;
            v15 = cgameGlob->movieCameraOrigin[0] == demo.playback->clipCameraOrigin[0]
                 && cgameGlob->movieCameraOrigin[1] == demo.playback->clipCameraOrigin[1]
                 && cgameGlob->movieCameraOrigin[2] == demo.playback->clipCameraOrigin[2];
            if ( v15 && !forceWrite )
            {
LABEL_49:
                MSG_WriteBit0(msg);
                goto LABEL_54;
            }
            MSG_WriteBit1(msg);
            MSG_WriteData(msg, (unsigned __int8 *)cgameGlob->movieCameraOrigin, 0xCu);
            if ( setClipRecordParams )
            {
                v19 = demo.playback->clipCameraOrigin;
                demo.playback->clipCameraOrigin[0] = cgameGlob->movieCameraOrigin[0];
                v19[1] = cgameGlob->movieCameraOrigin[1];
                v19[2] = cgameGlob->movieCameraOrigin[2];
                v8 = va(
                             "DEMO: w Clip Cmd: DEMO_CLIP_CMD_SET_CAMERA_ORIGIN Value: %d (%f, %f, %f)\n",
                             demo.playback->cameraMode,
                             demo.playback->clipCameraOrigin[0],
                             demo.playback->clipCameraOrigin[1],
                             demo.playback->clipCameraOrigin[2]);
            }
            else
            {
                v8 = va(
                             "DEMO: w Clip Cmd: DEMO_CLIP_CMD_SET_CAMERA_ORIGIN Value: %d (%f, %f, %f)\n",
                             demo.playback->clipCameraMode,
                             demo.playback->clipCameraOrigin[0],
                             demo.playback->clipCameraOrigin[1],
                             demo.playback->clipCameraOrigin[2]);
            }
            Demo_Printf(1024, v8);
        }
    }
LABEL_54:
    MSG_WriteBits(msg, 2, 2u);
    MSG_WriteByte(msg, 4u);
    if ( cameraModeToCheck >= 0 )
    {
        if ( cameraModeToCheck <= 1 )
        {
            v14 = cgameGlob->refdefViewAngles[0] == demo.playback->clipCameraAngles[0]
                 && cgameGlob->refdefViewAngles[1] == demo.playback->clipCameraAngles[1]
                 && cgameGlob->refdefViewAngles[2] == demo.playback->clipCameraAngles[2];
            if ( v14 && !forceWrite )
            {
LABEL_75:
                MSG_WriteBit0(msg);
                goto LABEL_80;
            }
            MSG_WriteBit1(msg);
            MSG_WriteData(msg, (unsigned __int8 *)LocalClientGlobals->viewangles, 0xCu);
            if ( setClipRecordParams )
            {
                clipCameraAngles = demo.playback->clipCameraAngles;
                demo.playback->clipCameraAngles[0] = LocalClientGlobals->viewangles[0];
                clipCameraAngles[1] = LocalClientGlobals->viewangles[1];
                clipCameraAngles[2] = LocalClientGlobals->viewangles[2];
                v9 = va(
                             "DEMO: w Clip Cmd: DEMO_CLIP_CMD_SET_CAMERA_ANGLES Value: %d (%f, %f, %f)\n",
                             demo.playback->cameraMode,
                             demo.playback->clipCameraAngles[0],
                             demo.playback->clipCameraAngles[1],
                             demo.playback->clipCameraAngles[2]);
LABEL_78:
                Demo_Printf(1024, v9);
                goto LABEL_80;
            }
        }
        else
        {
            if ( cameraModeToCheck != 2 )
                goto LABEL_80;
            v13 = cgameGlob->movieCameraAngles[0] == demo.playback->clipCameraAngles[0]
                 && cgameGlob->movieCameraAngles[1] == demo.playback->clipCameraAngles[1]
                 && cgameGlob->movieCameraAngles[2] == demo.playback->clipCameraAngles[2];
            if ( v13 && !forceWrite )
                goto LABEL_75;
            MSG_WriteBit1(msg);
            MSG_WriteData(msg, (unsigned __int8 *)cgameGlob->movieCameraAngles, 0xCu);
            if ( setClipRecordParams )
            {
                v17 = demo.playback->clipCameraAngles;
                demo.playback->clipCameraAngles[0] = cgameGlob->movieCameraAngles[0];
                v17[1] = cgameGlob->movieCameraAngles[1];
                v17[2] = cgameGlob->movieCameraAngles[2];
                v9 = va(
                             "DEMO: w Clip Cmd: DEMO_CLIP_CMD_SET_CAMERA_ANGLES Value: %d (%f, %f, %f)\n",
                             demo.playback->cameraMode,
                             demo.playback->clipCameraAngles[0],
                             demo.playback->clipCameraAngles[1],
                             demo.playback->clipCameraAngles[2]);
                goto LABEL_78;
            }
        }
        v9 = va(
                     "DEMO: w Clip Cmd: DEMO_CLIP_CMD_SET_CAMERA_ANGLES Value: %d (%f, %f, %f)\n",
                     demo.playback->clipCameraMode,
                     demo.playback->clipCameraAngles[0],
                     demo.playback->clipCameraAngles[1],
                     demo.playback->clipCameraAngles[2]);
        goto LABEL_78;
    }
LABEL_80:
    if ( demo.playback->clipHudHidden != demo.playback->hideGameHud || forceWrite )
    {
        MSG_WriteBits(msg, 2, 2u);
        MSG_WriteByte(msg, 5u);
        if ( setClipRecordParams )
        {
            if ( demo.playback->hideGameHud )
                MSG_WriteBit1(msg);
            else
                MSG_WriteBit0(msg);
            demo.playback->clipHudHidden = demo.playback->hideGameHud;
        }
        else if ( demo.playback->hideGameHud )
        {
            MSG_WriteBit1(msg);
        }
        else
        {
            MSG_WriteBit0(msg);
        }
        v10 = va("DEMO: w Clip Cmd: DEMO_CLIP_CMD_HIDE_GAME_HUD value: %d\n", demo.playback->hideGameHud);
        Demo_Printf(1024, v10);
    }
    if ( forceWrite )
    {
        MSG_WriteBits(msg, 2, 2u);
        MSG_WriteByte(msg, 6u);
        MSG_WriteLong(msg, LocalClientGlobals->snap.messageNum);
        v11 = va("DEMO: w Clip Cmd: DEMO_CLIP_CMD_RESET_SNAPSHOT_DATA\n");
        Demo_Printf(1024, v11);
    }
    if ( demo.playback->clipPausedState != demo_pause->current.color[0] || forceWrite )
    {
        MSG_WriteBits(msg, 2, 2u);
        MSG_WriteByte(msg, 7u);
        if ( setClipRecordParams )
        {
            if ( demo_pause->current.enabled )
                MSG_WriteBit1(msg);
            else
                MSG_WriteBit0(msg);
            demo.playback->clipPausedState = demo_pause->current.enabled;
        }
        else if ( demo.playback->clipPausedState )
        {
            MSG_WriteBit1(msg);
        }
        else
        {
            MSG_WriteBit0(msg);
        }
        v12 = va("DEMO: w Clip Cmd: DEMO_CLIP_CMD_SET_PAUSED_STATE value: %d\n", demo.playback->clipPausedState);
        Demo_Printf(1024, v12);
    }
    demo.playback->forceWriteClipCommands = 0;
}

void __cdecl Demo_WriteEndOfFrameClipCommand(msg_t *msg)
{
    MSG_WriteBits(msg, 2, 2u);
    MSG_WriteByte(msg, 9u);
}

void __cdecl Demo_WriteClipCommandsWhenPaused(int localClientNum)
{
    int ControllerIndex; // eax
    char *v2; // [esp-4h] [ebp-4h]

    if ( demo.playback->clipRecordBufIndex < 999424 )
    {
        Demo_WriteClipCommands(localClientNum, &demo.playback->clipRecordingMsg, 1);
        Demo_WriteEndOfFrameClipCommand(&demo.playback->clipRecordingMsg);
    }
    else
    {
        if ( !Demo_IsClipRecording()
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                        5559,
                        0,
                        "%s",
                        "Demo_IsClipRecording()") )
        {
            __debugbreak();
        }
        v2 = va("demo_pausecliprecord %d", demo_cmdNum->current.integer);
        ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        Cmd_ExecuteSingleCommand(localClientNum, ControllerIndex, v2);
    }
}

int __cdecl Demo_WriteTransitionClipCommandBlockToBuffer(
                unsigned __int8 transition,
                unsigned __int8 *buffer,
                int maxBufferSize)
{
    int compressedSize; // [esp+0h] [ebp-23Ch]
    unsigned __int8 msgBuf[256]; // [esp+4h] [ebp-238h] BYREF
    unsigned __int8 tempMsgBuf[260]; // [esp+104h] [ebp-138h] BYREF
    msg_t msg; // [esp+20Ch] [ebp-30h] BYREF

    memset(msgBuf, 0, sizeof(msgBuf));
    memset(tempMsgBuf, 0, 0x100u);
    MSG_Init(&msg, msgBuf, 256);
    MSG_WriteBits(&msg, 2, 2u);
    MSG_WriteByte(&msg, 8u);
    MSG_WriteByte(&msg, transition);
    Demo_WriteEndOfFrameClipCommand(&msg);
    MSG_WriteBits(&msg, 0, 2u);
    compressedSize = MSG_CompressWithZLib(msg.data, msg.cursize, tempMsgBuf, 0x100u);
    if ( compressedSize + 8 >= maxBufferSize )
        return 0;
    *(unsigned int *)buffer = compressedSize;
    *((unsigned int *)buffer + 1) = msg.cursize;
    memcpy(buffer + 8, tempMsgBuf, compressedSize);
    return compressedSize + 8;
}

unsigned __int8 __cdecl Demo_ParseClipCommand(int localClientNum, msg_t *msg, bool setClipPlaybackParams)
{
    const char *v3; // eax
    double v4; // st7
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
    const char *v15; // eax
    const char *v16; // eax
    const char *v17; // eax
    float *v19; // [esp+1Ch] [ebp-4Ch]
    float *v20; // [esp+20h] [ebp-48h]
    float *prevClipCameraAngles; // [esp+2Ch] [ebp-3Ch]
    float *clipCameraAngles; // [esp+30h] [ebp-38h]
    float *v23; // [esp+34h] [ebp-34h]
    float *v24; // [esp+38h] [ebp-30h]
    float *prevClipCameraOrigin; // [esp+44h] [ebp-24h]
    float *clipCameraOrigin; // [esp+48h] [ebp-20h]
    clientActive_t *LocalClientGlobals; // [esp+4Ch] [ebp-1Ch]
    unsigned __int8 cmdType; // [esp+53h] [ebp-15h]
    cg_s *cgameGlob; // [esp+54h] [ebp-14h]
    float timeScale; // [esp+58h] [ebp-10h]
    int msgNum; // [esp+5Ch] [ebp-Ch]
    int transition; // [esp+60h] [ebp-8h]
    int newClient; // [esp+64h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    cmdType = MSG_ReadByte(msg);
    switch (cmdType)
    {
    case 0u:
        newClient = MSG_ReadByte(msg);
        memcpy(
            (unsigned __int8 *)&demo.ps[newClient],
            (unsigned __int8 *)&demo.ps[demo_client->current.integer],
            sizeof(demo.ps[newClient]));
        memcpy(
            (unsigned __int8 *)&demo.ps[demo_client->current.integer],
            (unsigned __int8 *)&g_defaultPlayerState,
            sizeof(demo.ps[demo_client->current.integer]));
        if (MSG_ReadBit(msg))
        {
            Demo_EnableSnapshotProcessing();
            MSG_ReadDeltaPlayerstate(localClientNum, msg, 0, &demo.ps[newClient], &demo.ps[demo_client->current.integer], 0);
            Demo_DisableSnapshotProcessing();
            if (Demo_IsClipPlaying())
                Demo_AdjustTimeForPlayerState(
                    LocalClientGlobals->snap.serverTime,
                    demo.playback->clipServerTime,
                    &demo.ps[demo_client->current.integer]);
            Demo_UpdateCurrentSnapshot(localClientNum, &demo.ps[demo_client->current.integer]);
            memcpy(
                (unsigned __int8 *)&demo.ps[demo_client->current.integer],
                (unsigned __int8 *)&g_defaultPlayerState,
                sizeof(demo.ps[demo_client->current.integer]));
        }
        Dvar_SetInt((dvar_s*)demo_client, newClient);
        v3 = va("DEMO: r Clip Cmd: DEMO_CLIP_CMD_SET_CLIENT client: %d\n", newClient);
        Demo_Printf(1024, v3);
        break;
    case 1u:
        timeScale = MSG_ReadRangedFloatBits(msg, 0.0, 4.0, 16);
        Com_SetTimeScale(timeScale);
        Dvar_SetFloat((dvar_s*)demo_timescale, timeScale);
        v4 = Com_GetTimeScale();
        v5 = va("DEMO: r Clip Cmd: DEMO_CLIP_CMD_SET_TIMESCALE Value: %f\n", v4);
        Demo_Printf(1024, v5);
        break;
    case 2u:
        demo.playback->clipCameraMode = MSG_ReadByte(msg);
        v6 = va("DEMO: r Clip Cmd: DEMO_CLIP_CMD_SET_CAMERA_MODE Mode: %d\n", demo.playback->cameraMode);
        Demo_Printf(1024, v6);
        if (CL_LocalClient_IsCUIFlagSet(localClientNum, 32))
            Demo_SwitchCameraMode(localClientNum, demo.playback->clipCameraMode);
        break;
    case 3u:
        if (MSG_ReadBit(msg))
        {
            if (setClipPlaybackParams)
            {
                prevClipCameraOrigin = demo.playback->prevClipCameraOrigin;
                clipCameraOrigin = demo.playback->clipCameraOrigin;
                demo.playback->prevClipCameraOrigin[0] = demo.playback->clipCameraOrigin[0];
                prevClipCameraOrigin[1] = clipCameraOrigin[1];
                prevClipCameraOrigin[2] = clipCameraOrigin[2];
                MSG_ReadData(msg, (unsigned char *)demo.playback->clipCameraOrigin, 12);
                v7 = va(
                    "DEMO: r Clip Cmd: DEMO_CLIP_CMD_SET_CAMERA_ORIGIN Value: %d (%f, %f, %f)\n",
                    demo.playback->cameraMode,
                    demo.playback->clipCameraOrigin[0],
                    demo.playback->clipCameraOrigin[1],
                    demo.playback->clipCameraOrigin[2]);
                Demo_Printf(1024, v7);
            }
            else
            {
                MSG_ReadData(msg, (unsigned char *)cgameGlob->refdef.vieworg, 12);
                if (demo.playback->clipCameraMode == 2)
                {
                    cgameGlob->movieCameraOrigin[0] = cgameGlob->refdef.vieworg[0];
                    cgameGlob->movieCameraOrigin[1] = cgameGlob->refdef.vieworg[1];
                    cgameGlob->movieCameraOrigin[2] = cgameGlob->refdef.vieworg[2];
                }
                v8 = va(
                    "DEMO: r Clip Cmd: DEMO_CLIP_CMD_SET_CAMERA_ORIGIN Value: %d (%f, %f, %f)\n",
                    demo.playback->cameraMode,
                    cgameGlob->refdef.vieworg[0],
                    cgameGlob->refdef.vieworg[1],
                    cgameGlob->refdef.vieworg[2]);
                Demo_Printf(1024, v8);
            }
        }
        else
        {
            v23 = demo.playback->prevClipCameraOrigin;
            v24 = demo.playback->clipCameraOrigin;
            demo.playback->prevClipCameraOrigin[0] = demo.playback->clipCameraOrigin[0];
            v23[1] = v24[1];
            v23[2] = v24[2];
            v9 = va(
                "DEMO: r Clip Cmd: DEMO_CLIP_CMD_SET_CAMERA_ORIGIN Value: %d (%f, %f, %f)\n",
                demo.playback->cameraMode,
                demo.playback->clipCameraOrigin[0],
                demo.playback->clipCameraOrigin[1],
                demo.playback->clipCameraOrigin[2]);
            Demo_Printf(1024, v9);
        }
        break;
    case 4u:
        if (MSG_ReadBit(msg))
        {
            if (setClipPlaybackParams)
            {
                prevClipCameraAngles = demo.playback->prevClipCameraAngles;
                clipCameraAngles = demo.playback->clipCameraAngles;
                demo.playback->prevClipCameraAngles[0] = demo.playback->clipCameraAngles[0];
                prevClipCameraAngles[1] = clipCameraAngles[1];
                prevClipCameraAngles[2] = clipCameraAngles[2];
                MSG_ReadData(msg, (unsigned char *)demo.playback->clipCameraAngles, 12);
                v10 = va(
                    "DEMO: r Clip Cmd: DEMO_CLIP_CMD_SET_CAMERA_ANGLES Value: %d (%f, %f, %f)\n",
                    demo.playback->cameraMode,
                    demo.playback->clipCameraAngles[0],
                    demo.playback->clipCameraAngles[1],
                    demo.playback->clipCameraAngles[2]);
                Demo_Printf(1024, v10);
            }
            else
            {
                MSG_ReadData(msg, (unsigned char*)LocalClientGlobals->viewangles, 12);
                if (demo.playback->clipCameraMode == 2)
                {
                    cgameGlob->movieCameraAngles[0] = LocalClientGlobals->viewangles[0];
                    cgameGlob->movieCameraAngles[1] = LocalClientGlobals->viewangles[1];
                    cgameGlob->movieCameraAngles[2] = LocalClientGlobals->viewangles[2];
                }
                v11 = va(
                    "DEMO: r Clip Cmd: DEMO_CLIP_CMD_SET_CAMERA_ANGLES Value: %d (%f, %f, %f)\n",
                    demo.playback->cameraMode,
                    LocalClientGlobals->viewangles[0],
                    LocalClientGlobals->viewangles[1],
                    LocalClientGlobals->viewangles[2]);
                Demo_Printf(1024, v11);
            }
        }
        else
        {
            v19 = demo.playback->prevClipCameraAngles;
            v20 = demo.playback->clipCameraAngles;
            demo.playback->prevClipCameraAngles[0] = demo.playback->clipCameraAngles[0];
            v19[1] = v20[1];
            v19[2] = v20[2];
            v12 = va(
                "DEMO: r Clip Cmd: DEMO_CLIP_CMD_SET_CAMERA_ANGLES Value: %d (%f, %f, %f)\n",
                demo.playback->cameraMode,
                demo.playback->clipCameraAngles[0],
                demo.playback->clipCameraAngles[1],
                demo.playback->clipCameraAngles[2]);
            Demo_Printf(1024, v12);
        }
        break;
    case 5u:
        demo.playback->hideGameHud = MSG_ReadBit(msg) != 0;
        v13 = va("DEMO: r Clip Cmd: DEMO_CLIP_CMD_HIDE_GAME_HUD value: %d\n", demo.playback->hideGameHud);
        Demo_Printf(1024, v13);
        break;
    case 6u:
        msgNum = MSG_ReadLong(msg);
        Demo_ResetSnapshotData();
        Demo_ResetSequenceNumbers(localClientNum, msgNum - 1);
        Demo_ResetWorldInformation(localClientNum, 0);
        Demo_SetJumpTimeFlag(1);
        if (Demo_IsClipPlaying() || Demo_IsClipPreviewRunning())
            demo.playback->forceTeleportClipCamera = 1;
        v14 = va("DEMO: r Clip Cmd: DEMO_CLIP_CMD_RESET_SNAPSHOT_DATA\n");
        Demo_Printf(1024, v14);
        break;
    case 7u:
        demo.playback->clipPausedState = MSG_ReadBit(msg) != 0;
        v15 = va("DEMO: r Clip Cmd: DEMO_CLIP_CMD_SET_PAUSED_STATE value: %d\n", demo.playback->clipPausedState);
        Demo_Printf(1024, v15);
        break;
    case 8u:
        transition = MSG_ReadByte(msg);
        Demo_ActivateTransitionScreen(transition);
        break;
    case 9u:
        v16 = va("DEMO: r Clip Cmd: DEMO_CLIP_CMD_END_OF_FRAME\n");
        Demo_Printf(1024, v16);
        break;
    default:
        v17 = va("Unknown demo cmd type %d\n", cmdType);
        if (!Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp", 5782, 0, v17))
            __debugbreak();
        break;
    }
    return cmdType;
}

void __cdecl Demo_WriteClipGameState(int localClientNum)
{
    int Checksum; // eax
    const char *v2; // eax
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-12Ch]
    entityState_s nullstate; // [esp+4h] [ebp-128h] BYREF
    msg_t msg; // [esp+ECh] [ebp-40h] BYREF
    clientConnection_t *clc; // [esp+11Ch] [ebp-10h]
    int oldSvsHeaderValid; // [esp+120h] [ebp-Ch]
    const entityState_s *ent; // [esp+124h] [ebp-8h]
    int i; // [esp+128h] [ebp-4h]

    oldSvsHeaderValid = 0;
    clc = CL_GetLocalClientConnection(localClientNum);
    memset(demo.msgBuf1, 0, sizeof(demo.msgBuf1));
    MSG_Init(&msg, demo.msgBuf1, 49152);
    MSG_WriteLong(&msg, clc->reliableSequence);
    MSG_WriteByte(&msg, 1u);
    MSG_WriteLong(&msg, clc->serverCommandSequence);
    MSG_WriteString(&msg, demo.info.mapName);
    MSG_WriteString(&msg, demo.info.gameType);
    Checksum = CCS_GetChecksum();
    MSG_WriteLong(&msg, Checksum);
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    Demo_WriteConfigStrings(localClientNum, &msg);
    oldSvsHeaderValid = svsHeaderValid;
    svsHeaderValid = 1;
    svsHeader.mapCenter[0] = cls.mapCenter[0];
    svsHeader.mapCenter[1] = cls.mapCenter[1];
    svsHeader.mapCenter[2] = cls.mapCenter[2];
    memset(&nullstate, 0, sizeof(nullstate));
    for ( i = 0; i < 1024; ++i )
    {
        ent = &LocalClientGlobals->entityBaselines[i];
        if ( ent->number )
        {
            MSG_WriteByte(&msg, 4u);
            MSG_WriteEntity(&g_snapInfo, &msg, -90000, &nullstate, ent, DELTA_FLAGS_FORCE);
        }
    }
    MSG_WriteByte(&msg, 3u);
    MSG_WriteLong(&msg, cls.gameState.matchUIVisibilityFlags);
    svsHeaderValid = oldSvsHeaderValid;
    MSG_WriteByte(&msg, 0xEu);
    MSG_WriteLong(&msg, clc->clientNum);
    MSG_WriteLong(&msg, clc->checksumFeed);
    MSG_WriteByte(&msg, 6u);
    DynEnt_WriteGameState(&msg);
    MSG_WriteByte(&msg, 7u);
    MSG_WriteShort(&msg, 1023);
    MSG_WriteByte(&msg, 8u);
    Rope_WriteGameState(&msg);
    MSG_WriteByte(&msg, 9u);
    GlassCl_WriteDemoSnapshot(&msg);
    UI_Gametype_Custom_WriteClientData(&msg);
    MSG_WriteByte(&msg, 0xEu);
    if ( msg.cursize < 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                    5884,
                    0,
                    "%s",
                    "msg.cursize >= CL_DECODE_START") )
    {
        __debugbreak();
    }
    MSG_Embed(&demo.playback->clipRecordingMsg, &msg);
    v2 = va("DEMO: Clip Gamestate Type: %d Client: %d Size: %d msgSeq: 0\n", 3, clc->clientNum, msg.cursize);
    Demo_Printf(1024, v2);
}

bool __cdecl Demo_ShouldFlushClipRecordMessage()
{
    return demo.playback->clipRecordingMsg.cursize >= 44032;
}

void __cdecl Demo_SaveClipRecordMessageToMainMemory(
                int localClientNum,
                unsigned __int8 *data,
                unsigned int size,
                bool forceWrite)
{
    int compressedSize; // [esp+0h] [ebp-34h]
    msg_t compressedMsg; // [esp+4h] [ebp-30h] BYREF

    memset(demo.msgBuf1, 0, sizeof(demo.msgBuf1));
    MSG_Init(&compressedMsg, demo.msgBuf1, 49152);
    compressedSize = MSG_CompressWithZLib(data, size, demo.msgBuf1, 0xC000u);
    if ( (!compressedSize || !size)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                    5927,
                    0,
                    "%s",
                    "compressedSize != 0 && size != 0") )
    {
        __debugbreak();
    }
    *(unsigned int *)&demo.playback->mainClipRecordingBuf[demo.playback->clipRecordBufIndex] = compressedSize;
    demo.playback->clipRecordBufIndex += 4;
    *(unsigned int *)&demo.playback->mainClipRecordingBuf[demo.playback->clipRecordBufIndex] = size;
    demo.playback->clipRecordBufIndex += 4;
    memcpy(&demo.playback->mainClipRecordingBuf[demo.playback->clipRecordBufIndex], demo.msgBuf1, compressedSize);
    demo.playback->clipRecordBufIndex += compressedSize;
    if ( forceWrite )
        Demo_WriteRecordedClip(localClientNum, 1u);
    MSG_Init(&demo.playback->clipRecordingMsg, demo.playback->subClipRecordingBuf, 49152);
}

void __cdecl Demo_MoveToNextPhase()
{
    switch ( streamingState.phase )
    {
        case 0:
            streamingState.phase = 1;
            break;
        case 1:
            if ( streamingState.segmentNo == demo.playback->segmentCount - 1 )
                streamingState.phase = 3;
            else
                streamingState.phase = 2;
            break;
        case 2:
            ++streamingState.segmentNo;
            streamingState.phase = 1;
            break;
        case 3:
            streamingState.phase = 4;
            break;
        case 4:
            streamingState.phase = 5;
            break;
        default:
            break;
    }
    streamingState.isEnteringState = 1;
}

void __cdecl Demo_SetupClipStreamingState()
{
    int indexBefore; // [esp+4h] [ebp-4h]

    switch ( streamingState.phase )
    {
        case 0:
            streamingState.buffer = demo.playback->mainClipRecordingBuf;
            streamingState.bufferSize = demo.playback->segmentGameStateSize;
            break;
        case 1:
            streamingState.buffer = &demo.playback->mainClipRecordingBuf[demo.playback->segments[streamingState.segmentNo].clipBufferIndex];
            streamingState.bufferSize = demo.playback->segments[streamingState.segmentNo].size;
            streamingState.totalDuration += demo.playback->segments[streamingState.segmentNo].duration;
            break;
        case 2:
            streamingState.bufferSize = Demo_WriteTransitionClipCommandBlockToBuffer(
                                                                        demo.playback->segments[streamingState.segmentNo].transition,
                                                                        demo.msgBuf1,
                                                                        49152);
            streamingState.buffer = demo.msgBuf1;
            break;
        case 3:
            indexBefore = demo.playback->clipRecordBufIndex;
            MSG_WriteBits(&demo.playback->clipRecordingMsg, 1, 2u);
            Demo_SaveClipRecordMessageToMainMemory(
                0,
                demo.playback->clipRecordingMsg.data,
                demo.playback->clipRecordingMsg.cursize,
                0);
            streamingState.buffer = &demo.playback->mainClipRecordingBuf[indexBefore];
            streamingState.bufferSize = demo.playback->clipRecordBufIndex - indexBefore;
            break;
        case 4:
            demo.playback->clipRecordInfo.endTime = streamingState.totalDuration;
            demo.playback->clipRecordInfo.type = 1;
            Demo_PopulateClipPlayerTags(&demo.playback->clipRecordInfo);
            Demo_PopulateHeliPatches(&demo.playback->clipRecordInfo);
            memset(demo.msgBuf1, 0, sizeof(demo.msgBuf1));
            streamingState.bufferSize = Demo_WriteInfoDataToBuffer(&demo.playback->clipRecordInfo, demo.msgBuf1, 49152);
            streamingState.buffer = demo.msgBuf1;
            break;
        default:
            break;
    }
    streamingState.byteOffset = 0;
    streamingState.isEnteringState = 0;
}

void __cdecl Demo_PopulateClipPlayerTags(demoMetaInfo *info)
{
    clipSegment *v1; // eax
    int numConnectedPlayersInfoCount; // edx
    bool foundPlayerTag; // [esp+1Bh] [ebp-Dh]
    int playerIndex; // [esp+1Ch] [ebp-Ch]
    int i; // [esp+20h] [ebp-8h]
    int segmentIndex; // [esp+24h] [ebp-4h]

    for ( segmentIndex = 0; segmentIndex < demo.playback->segmentCount; ++segmentIndex )
    {
        for ( playerIndex = 0; playerIndex < demo.playback->segments[segmentIndex].taggedPlayers.count; ++playerIndex )
        {
            foundPlayerTag = 0;
            for ( i = 0; i < info->numConnectedPlayersInfoCount; ++i )
            {
                if ( LODWORD(info->connectedPlayers[i].xuid) == LODWORD(demo.playback->segments[segmentIndex].taggedPlayers.playerXuids[playerIndex])
                    && HIDWORD(info->connectedPlayers[i].xuid) == HIDWORD(demo.playback->segments[segmentIndex].taggedPlayers.playerXuids[playerIndex]) )
                {
                    foundPlayerTag = 1;
                    break;
                }
            }
            if ( !foundPlayerTag )
            {
                v1 = &demo.playback->segments[segmentIndex];
                numConnectedPlayersInfoCount = info->numConnectedPlayersInfoCount;
                LODWORD(info->connectedPlayers[numConnectedPlayersInfoCount].xuid) = v1->taggedPlayers.playerXuids[playerIndex];
                HIDWORD(info->connectedPlayers[numConnectedPlayersInfoCount].xuid) = HIDWORD(v1->taggedPlayers.playerXuids[playerIndex]);
                ++info->numConnectedPlayersInfoCount;
            }
        }
    }
}

int __cdecl Demo_SendClipToStreamingBuffer(unsigned __int8 *data, int dataSize, unsigned int bytesUploaded)
{
    int size; // [esp+0h] [ebp-4h]

    if ( !bytesUploaded )
    {
        streamingState.buffer = 0;
        streamingState.bufferSize = 0;
        streamingState.phase = 0;
        streamingState.segmentNo = 0;
        streamingState.byteOffset = 0;
        streamingState.isEnteringState = 1;
        streamingState.totalDuration = 0;
    }
    if ( streamingState.phase == 5 && streamingState.byteOffset == streamingState.bufferSize )
        return 0;
    if ( streamingState.isEnteringState )
        Demo_SetupClipStreamingState();
    if ( streamingState.bufferSize - streamingState.byteOffset <= dataSize )
    {
        size = streamingState.bufferSize - streamingState.byteOffset;
        memcpy(
            data,
            &streamingState.buffer[streamingState.byteOffset],
            streamingState.bufferSize - streamingState.byteOffset);
        streamingState.byteOffset += size;
        Demo_MoveToNextPhase();
        return size;
    }
    else
    {
        memcpy(data, &streamingState.buffer[streamingState.byteOffset], dataSize);
        streamingState.byteOffset += dataSize;
        return dataSize;
    }
}

char gamerTag_2[32];
void __cdecl Demo_UploadClipSuccess(int controllerIndex, unsigned __int64 fileID)
{
    int LocalClientNum; // eax
    int v3; // eax
    const char *v4; // eax
    char *ClientName; // eax
    int v6; // eax
    char *v7; // [esp-4h] [ebp-4F8h]
    int v8; // [esp+10h] [ebp-4E4h]
    bdTaskResult *j; // [esp+14h] [ebp-4E0h]
    int v10; // [esp+18h] [ebp-4DCh]
    bdTaskResult *k; // [esp+1Ch] [ebp-4D8h]
    int v12; // [esp+20h] [ebp-4D4h]
    bdTag *i; // [esp+24h] [ebp-4D0h]
    int numTags; // [esp+28h] [ebp-4CCh] BYREF
    char metaData[256]; // [esp+2Ch] [ebp-4C8h] BYREF
    int metaDataSize; // [esp+130h] [ebp-3C4h] BYREF
    bdTag tags[40]; // [esp+134h] [ebp-3C0h] BYREF
    int savedregs; // [esp+4F4h] [ebp+0h] BYREF

    Demo_SetClipState(3);
    Demo_SetClipModified(0);
    LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
    Demo_Error(LocalClientNum, "", "MENU_DEMO_CLIP_UPLOAD_SUCCESS");
    v3 = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
    UI_CloseMenu(v3, "fileshare_slotselection_clip");
    v4 = va("Clip: Clip upload success.\n");
    Demo_Printf(1024, v4);
    Demo_SetCmdCompleted();
    //v12 = 40;
    //for ( i = tags; --v12 >= 0; ++i )
    //    bdTag::bdTag(i);
    numTags = 0;
    metaDataSize = 0;
    ClientName = Live_ControllerIndex_GetClientName(controllerIndex);
    I_strncpyz(gamerTag_2, ClientName, 32);
    memset(metaData, 0, 0xFFu);
    Demo_SetTags(controllerIndex, &numTags, tags, &demo.playback->clipRecordInfo, FILESHARE_FILETYPE_CLIP);
    if ( Demo_SetMetaData(
                 controllerIndex,
                 metaData,
                 &metaDataSize,
                 &demo.playback->clipRecordInfo,
                 0,
                 FILESHARE_FILETYPE_CLIP) )
    {
        Live_FileShare_AddTag(5u, fsSelectedFileTagIndex->current.integer, &numTags, tags, 40);
        LiveStorage_FileShare_WriteSummary(
            controllerIndex,
            fileID,
            FILESHARE_LOCATION_USERSTORAGE,
            gamerTag_2,
            &gamerTag_2[strlen(gamerTag_2) + 1] - (char *)&gamerTag_2[0],
            metaData,
            metaDataSize,
            tags,
            numTags,
            0);
        LiveCounter_IncrementCounterValueByName("global_fileshare_shared", 1u);
        v7 = va("userPublishNews TICKER_CLIPUPLOADED\n");
        v6 = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
        Cmd_ExecuteSingleCommand(v6, controllerIndex, v7);
        //v8 = 40;
        //for ( j = (bdTaskResult *)&savedregs; --v8 >= 0; bdTag::~bdTag(j) )
        //    j -= 6;
    }
    else
    {
        Com_PrintError(16, "Could not set meta data for uploaded file ID %llu. This is now an untagged file.\n", fileID);
        //v10 = 40;
        //for ( k = (bdTaskResult *)&savedregs; --v10 >= 0; bdTag::~bdTag(k) )
        //    k -= 6;
    }
}

void __cdecl Demo_UploadClipFailure(int controllerIndex)
{
    int LocalClientNum; // eax
    const char *v2; // eax

    Demo_SetClipState(3);
    LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
    Demo_Error(LocalClientNum, "", "MENU_DEMO_CLIP_UPLOAD_FAILURE");
    v2 = va("Clip: Clip upload failed.\n");
    Demo_Printf(1024, v2);
    Demo_SetCmdCompleted();
}

void __cdecl Demo_PopulateHeliPatches(demoMetaInfo *info)
{
    info->numHeliPatchesCount = num_heli_height_lock_patches;
    memcpy(
        (unsigned __int8 *)info->heliPatches,
        (unsigned __int8 *)heli_height_lock_patches,
        24 * info->numHeliPatchesCount);
}

void __cdecl Demo_WriteRecordedClip(int localClientNum, unsigned __int16 slot)
{
    demoMain *DemoName; // eax
    int ControllerIndex; // eax
    int v4; // eax
    fileShareWriteFileInfo writeFileInfo; // [esp+0h] [ebp-34h] BYREF

    Demo_SetClipState(5);
    writeFileInfo.fileSlot = slot;
    writeFileInfo.category = 2;
    DemoName = Demo_GetDemoName();
    writeFileInfo.fileName = va("%s_c.demo", DemoName->demoName);
    writeFileInfo.fileData = 0;
    writeFileInfo.fileSize = demo.playback->clipRecordBufIndex;
    writeFileInfo.tags = 0;
    memset(&writeFileInfo.thumbData, 0, 12);
    writeFileInfo.location = FILESHARE_LOCATION_USERSTORAGE;
    writeFileInfo.dataCallback = (unsigned int (__cdecl *)(void *, unsigned int, unsigned int))Demo_SendClipToStreamingBuffer;
    writeFileInfo.successCallback = Demo_UploadClipSuccess;
    writeFileInfo.failureCallback = Demo_UploadClipFailure;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( LiveStorage_FileShare_WriteFile(ControllerIndex, &writeFileInfo) )
    {
        v4 = Com_LocalClient_GetControllerIndex(localClientNum);
        Demo_StartStreaming(v4);
    }
}

char *__cdecl Demo_AdjustTimeForConfigString(int configStringIndex, const char *string)
{
    int modifiedTime; // [esp+4h] [ebp-10h]
    int modifiedTimea; // [esp+4h] [ebp-10h]
    int clipTime; // [esp+8h] [ebp-Ch]
    int currentTime; // [esp+Ch] [ebp-8h]

    if ( !Demo_IsClipPlaying() )
        return (char *)string;
    if ( !Demo_IsPlaybackInited() )
        return (char *)string;
    clipTime = demo.playback->clipServerTime;
    if ( configStringIndex != 12 )
        return (char *)string;
    currentTime = demo.playback->originalFilmTime;
    modifiedTime = atoi(string) - currentTime;
    if ( clipTime + modifiedTime >= 0 )
        return va("%d", clipTime + modifiedTime);
    modifiedTimea = atoi(string);
    return va("%d", modifiedTimea);
}

const char *__cdecl Demo_AdjustTimeForReliableCommand(const char *cmd)
{
    const char *result; // eax
    int modifiedTime; // [esp+4h] [ebp-1Ch]
    int clipTime; // [esp+8h] [ebp-18h]
    parseInfo_t *cmdToken; // [esp+Ch] [ebp-14h]
    parseInfo_t *cmdTokena; // [esp+Ch] [ebp-14h]
    int currentTime; // [esp+14h] [ebp-Ch]
    int cmdValue; // [esp+18h] [ebp-8h]
    const char *s; // [esp+1Ch] [ebp-4h] BYREF

    if ( !Demo_IsClipPlaying() || !Demo_IsPlaybackInited() )
        return cmd;
    s = va("%s", cmd);
    currentTime = demo.playback->originalFilmTime + 100;
    clipTime = demo.playback->clipServerTime + 100;
    if ( Com_Parse(&s)->token[0] != 100 )
        return cmd;
    cmdToken = Com_Parse(&s);
    if ( atoi(cmdToken->token) != 12 )
        return cmd;
    cmdTokena = Com_Parse(&s);
    cmdValue = atoi(cmdTokena->token);
    if ( cmdValue <= 0 )
        modifiedTime = 0;
    else
        modifiedTime = cmdValue - currentTime;
    if ( clipTime + modifiedTime < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                    6341,
                    0,
                    "%s",
                    "modifiedTime + clipTime >= 0") )
    {
        __debugbreak();
    }
    result = va("%c %d %d", 100, 12, clipTime + modifiedTime);
    s = result;
    return result;
}

void __cdecl Demo_AdjustTimeForMatchState(int currentTime, int clipTime, MatchState *matchstate)
{
    int bombTimer; // [esp+4h] [ebp-4h]
    int bombTimera; // [esp+4h] [ebp-4h]

    bombTimer = matchstate->archivedState.bombTimer[0];
    if ( bombTimer )
    {
        if ( clipTime + bombTimer - currentTime < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                        6369,
                        0,
                        "%s",
                        "modifiedTime + clipTime >= 0") )
        {
            __debugbreak();
        }
        matchstate->archivedState.bombTimer[0] = clipTime + bombTimer - currentTime;
    }
    bombTimera = matchstate->archivedState.bombTimer[1];
    if ( bombTimera )
    {
        if ( clipTime + bombTimera - currentTime < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                        6378,
                        0,
                        "%s",
                        "modifiedTime + clipTime >= 0") )
        {
            __debugbreak();
        }
        matchstate->archivedState.bombTimer[1] = clipTime + bombTimera - currentTime;
    }
}

void __cdecl Demo_AdjustTimeForPlayerState(int currentTime, int clipTime, playerState_s *ps)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    int index; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    v3 = va("Playerstate Time Adjustment - currentTime: %d clipTime: %d\n", currentTime, clipTime);
    Demo_Printf(0x4000, v3);
    v4 = va("Archival HUD Elements\n");
    Demo_Printf(0x4000, v4);
    for ( i = 0; i < 31; ++i )
        Demo_AdjustTimeForPlayerStateHudElement(currentTime, clipTime, i, &ps->hud.archival[i]);
    v5 = va("Current HUD Elements\n");
    Demo_Printf(0x4000, v5);
    for ( index = 0; index < 31; ++index )
        Demo_AdjustTimeForPlayerStateHudElement(currentTime, clipTime, index, &ps->hud.current[index]);
}

void __cdecl Demo_AdjustTimeForPlayerStateHudElement(int currentTime, int clipTime, int index, hudelem_s *hudElem)
{
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
    const char *v15; // eax
    const char *v16; // eax
    const char *v17; // eax
    const char *v18; // eax
    const char *v19; // eax
    const char *v20; // eax
    const char *v21; // eax
    const char *v22; // eax
    const char *v23; // eax
    const char *v24; // eax
    const char *v25; // eax
    bool printHudDetails; // [esp+7h] [ebp-5h]
    int deltaTime; // [esp+8h] [ebp-4h]
    int deltaTimea; // [esp+8h] [ebp-4h]
    int deltaTimeb; // [esp+8h] [ebp-4h]
    int deltaTimec; // [esp+8h] [ebp-4h]

    printHudDetails = 0;
    switch ( hudElem->type )
    {
        case 0u:
            return;
        case 1u:
            if ( hudElem->fxBirthTime )
            {
                if ( hudElem->fxBirthTime - currentTime + clipTime < 0 )
                {
                    if ( hudElem->fxBirthTime <= currentTime )
                    {
                        if ( hudElem->fxBirthTime + hudElem->fxDecayStartTime <= currentTime )
                        {
                            if ( hudElem->fxDecayDuration + hudElem->fxDecayStartTime + hudElem->fxBirthTime <= currentTime )
                            {
                                hudElem->fxBirthTime = 0;
                                hudElem->fxDecayStartTime = 0;
                                hudElem->flags |= 0x200u;
                            }
                            else
                            {
                                deltaTimea = hudElem->fxDecayDuration + hudElem->fxDecayStartTime + hudElem->fxBirthTime - currentTime;
                                hudElem->fxDecayStartTime = 0;
                                hudElem->fxBirthTime = clipTime;
                                hudElem->fxDecayDuration = clipTime - deltaTimea;
                            }
                        }
                        else
                        {
                            deltaTime = hudElem->fxDecayStartTime + hudElem->fxBirthTime - currentTime;
                            hudElem->fxBirthTime = clipTime;
                            hudElem->fxDecayStartTime = clipTime - deltaTime;
                        }
                    }
                    else
                    {
                        hudElem->fxBirthTime = hudElem->fxBirthTime - currentTime + clipTime;
                    }
                }
                else if ( hudElem->fxBirthTime <= currentTime )
                {
                    if ( hudElem->fxBirthTime + hudElem->fxDecayStartTime <= currentTime )
                    {
                        if ( hudElem->fxDecayDuration + hudElem->fxDecayStartTime + hudElem->fxBirthTime <= currentTime )
                        {
                            hudElem->fxBirthTime = 0;
                            hudElem->fxDecayStartTime = 0;
                            hudElem->flags |= 0x200u;
                        }
                        else
                        {
                            hudElem->fxBirthTime = hudElem->fxBirthTime - currentTime + clipTime;
                            hudElem->fxDecayStartTime = hudElem->fxDecayStartTime
                                                                                + LOWORD(hudElem->fxBirthTime)
                                                                                - currentTime
                                                                                + clipTime;
                            hudElem->fxDecayDuration = hudElem->fxDecayDuration
                                                                             + hudElem->fxDecayStartTime
                                                                             + LOWORD(hudElem->fxBirthTime)
                                                                             - currentTime;
                        }
                    }
                    else
                    {
                        hudElem->fxBirthTime = hudElem->fxBirthTime - currentTime + clipTime;
                        hudElem->fxDecayStartTime = hudElem->fxDecayStartTime
                                                                            + LOWORD(hudElem->fxBirthTime)
                                                                            - currentTime
                                                                            + clipTime;
                    }
                }
                else
                {
                    hudElem->fxBirthTime = hudElem->fxBirthTime - currentTime + clipTime;
                }
            }
            if ( hudElem->fadeStartTime )
            {
                if ( hudElem->fadeStartTime <= currentTime )
                {
                    if ( hudElem->fadeStartTime + hudElem->fadeTime <= currentTime )
                    {
                        hudElem->fadeStartTime = 0;
                        hudElem->fadeTime = 0;
                    }
                    else
                    {
                        hudElem->fadeStartTime = clipTime + hudElem->fadeStartTime - currentTime;
                    }
                }
                else
                {
                    hudElem->fadeStartTime = clipTime + hudElem->fadeStartTime - currentTime;
                }
            }
            if ( hudElem->moveStartTime )
            {
                if ( hudElem->moveStartTime <= currentTime )
                {
                    if ( hudElem->moveStartTime + hudElem->moveTime <= currentTime )
                    {
                        hudElem->moveStartTime = 0;
                        hudElem->moveTime = 0;
                    }
                    else
                    {
                        hudElem->moveStartTime = clipTime + hudElem->moveStartTime - currentTime;
                    }
                }
                else
                {
                    hudElem->moveStartTime = clipTime + hudElem->moveStartTime - currentTime;
                }
            }
            if ( hudElem->time || hudElem->scaleStartTime )
            {
                v4 = va("ERROR: Need to process Time value for HE_TYPE_TEXT\n");
                Demo_Printf(0x4000, v4);
                printHudDetails = 1;
            }
            goto LABEL_162;
        case 2u:
            if ( hudElem->fxBirthTime )
            {
                if ( hudElem->fxBirthTime - currentTime + clipTime < 0 )
                {
                    if ( hudElem->fxBirthTime <= currentTime )
                    {
                        if ( hudElem->fxBirthTime + hudElem->fxDecayStartTime <= currentTime )
                        {
                            if ( hudElem->fxDecayDuration + hudElem->fxDecayStartTime + hudElem->fxBirthTime <= currentTime )
                            {
                                hudElem->fxBirthTime = 0;
                                hudElem->fxDecayStartTime = 0;
                                hudElem->flags |= 0x200u;
                            }
                            else
                            {
                                deltaTimec = hudElem->fxDecayDuration + hudElem->fxDecayStartTime + hudElem->fxBirthTime - currentTime;
                                hudElem->fxDecayStartTime = 0;
                                hudElem->fxBirthTime = clipTime;
                                hudElem->fxDecayDuration = clipTime - deltaTimec;
                            }
                        }
                        else
                        {
                            deltaTimeb = hudElem->fxDecayStartTime + hudElem->fxBirthTime - currentTime;
                            hudElem->fxBirthTime = clipTime;
                            hudElem->fxDecayStartTime = clipTime - deltaTimeb;
                        }
                    }
                    else
                    {
                        hudElem->fxBirthTime = hudElem->fxBirthTime - currentTime + clipTime;
                    }
                }
                else if ( hudElem->fxBirthTime <= currentTime )
                {
                    if ( hudElem->fxBirthTime + hudElem->fxDecayStartTime <= currentTime )
                    {
                        if ( hudElem->fxDecayDuration + hudElem->fxDecayStartTime + hudElem->fxBirthTime <= currentTime )
                        {
                            hudElem->fxBirthTime = 0;
                            hudElem->fxDecayStartTime = 0;
                            hudElem->flags |= 0x200u;
                        }
                        else
                        {
                            hudElem->fxBirthTime = hudElem->fxBirthTime - currentTime + clipTime;
                            hudElem->fxDecayStartTime = hudElem->fxDecayStartTime
                                                                                + LOWORD(hudElem->fxBirthTime)
                                                                                - currentTime
                                                                                + clipTime;
                            hudElem->fxDecayDuration = hudElem->fxDecayDuration
                                                                             + hudElem->fxDecayStartTime
                                                                             + LOWORD(hudElem->fxBirthTime)
                                                                             - currentTime;
                        }
                    }
                    else
                    {
                        hudElem->fxBirthTime = hudElem->fxBirthTime - currentTime + clipTime;
                        hudElem->fxDecayStartTime = hudElem->fxDecayStartTime
                                                                            + LOWORD(hudElem->fxBirthTime)
                                                                            - currentTime
                                                                            + clipTime;
                    }
                }
                else
                {
                    hudElem->fxBirthTime = hudElem->fxBirthTime - currentTime + clipTime;
                }
            }
            if ( hudElem->time || hudElem->fadeStartTime || hudElem->moveStartTime || hudElem->scaleStartTime )
            {
                v5 = va("ERROR: Need to process Time value for HE_TYPE_VALUE\n");
                Demo_Printf(0x4000, v5);
                printHudDetails = 1;
            }
            goto LABEL_162;
        case 3u:
            if ( hudElem->time
                || hudElem->fxBirthTime
                || hudElem->fadeStartTime
                || hudElem->moveStartTime
                || hudElem->scaleStartTime )
            {
                v6 = va("ERROR: Need to process Time value for HE_TYPE_PLAYERNAME\n");
                Demo_Printf(0x4000, v6);
                printHudDetails = 1;
            }
            goto LABEL_162;
        case 4u:
            if ( hudElem->time
                || hudElem->fxBirthTime
                || hudElem->fadeStartTime
                || hudElem->moveStartTime
                || hudElem->scaleStartTime )
            {
                v7 = va("ERROR: Need to process Time value for HE_TYPE_MAPNAME\n");
                Demo_Printf(0x4000, v7);
                printHudDetails = 1;
            }
            goto LABEL_162;
        case 5u:
            if ( hudElem->time
                || hudElem->fxBirthTime
                || hudElem->fadeStartTime
                || hudElem->moveStartTime
                || hudElem->scaleStartTime )
            {
                v8 = va("ERROR: Need to process Time value for HE_TYPE_GAMETYPE\n");
                Demo_Printf(0x4000, v8);
                printHudDetails = 1;
            }
            goto LABEL_162;
        case 6u:
            if ( hudElem->time
                || hudElem->fxBirthTime
                || hudElem->fadeStartTime
                || hudElem->moveStartTime
                || hudElem->scaleStartTime )
            {
                v9 = va("ERROR: Need to process Time value for HE_TYPE_WAR_GAME_DATA\n");
                Demo_Printf(0x4000, v9);
                printHudDetails = 1;
            }
            goto LABEL_162;
        case 7u:
            if ( hudElem->fadeStartTime )
            {
                if ( hudElem->fadeStartTime <= currentTime )
                {
                    if ( hudElem->fadeStartTime + hudElem->fadeTime <= currentTime )
                    {
                        hudElem->fadeStartTime = 0;
                        hudElem->fadeTime = 0;
                    }
                    else
                    {
                        hudElem->fadeStartTime = clipTime + hudElem->fadeStartTime - currentTime;
                    }
                }
                else
                {
                    hudElem->fadeStartTime = clipTime + hudElem->fadeStartTime - currentTime;
                }
            }
            if ( hudElem->time || hudElem->fxBirthTime || hudElem->moveStartTime || hudElem->scaleStartTime )
            {
                v10 = va("ERROR: Need to process Time value for HE_TYPE_SCORE\n");
                Demo_Printf(0x4000, v10);
                printHudDetails = 1;
            }
            goto LABEL_162;
        case 8u:
            if ( hudElem->fadeStartTime )
            {
                if ( hudElem->fadeStartTime <= currentTime )
                {
                    if ( hudElem->fadeStartTime + hudElem->fadeTime <= currentTime )
                    {
                        hudElem->fadeStartTime = 0;
                        hudElem->fadeTime = 0;
                    }
                    else
                    {
                        hudElem->fadeStartTime = clipTime + hudElem->fadeStartTime - currentTime;
                    }
                }
                else
                {
                    hudElem->fadeStartTime = clipTime + hudElem->fadeStartTime - currentTime;
                }
            }
            if ( hudElem->moveStartTime )
            {
                if ( hudElem->moveStartTime <= currentTime )
                {
                    if ( hudElem->moveStartTime + hudElem->moveTime <= currentTime )
                    {
                        hudElem->moveStartTime = 0;
                        hudElem->moveTime = 0;
                    }
                    else
                    {
                        hudElem->moveStartTime = clipTime + hudElem->moveStartTime - currentTime;
                    }
                }
                else
                {
                    hudElem->moveStartTime = clipTime + hudElem->moveStartTime - currentTime;
                }
            }
            if ( hudElem->scaleStartTime )
            {
                if ( hudElem->scaleStartTime + hudElem->scaleTime <= currentTime )
                {
                    hudElem->scaleStartTime = 0;
                    hudElem->scaleTime = 0;
                }
                else
                {
                    hudElem->scaleStartTime = clipTime + hudElem->scaleStartTime - currentTime;
                }
            }
            if ( hudElem->time || hudElem->fxBirthTime )
            {
                v11 = va("ERROR: Need to process Time value for HE_TYPE_MATERIAL\n");
                Demo_Printf(0x4000, v11);
                printHudDetails = 1;
            }
            goto LABEL_162;
        case 9u:
            if ( hudElem->time )
            {
                if ( hudElem->time <= currentTime )
                    hudElem->time = 0;
                else
                    hudElem->time = clipTime + hudElem->time - currentTime;
            }
            if ( hudElem->fxBirthTime || hudElem->fadeStartTime || hudElem->moveStartTime || hudElem->scaleStartTime )
            {
                v12 = va("ERROR: Need to process Time value for HE_TYPE_TIMER_DOWN\n");
                Demo_Printf(0x4000, v12);
                printHudDetails = 1;
            }
            goto LABEL_162;
        case 0xAu:
            if ( hudElem->time
                || hudElem->fxBirthTime
                || hudElem->fadeStartTime
                || hudElem->moveStartTime
                || hudElem->scaleStartTime )
            {
                v13 = va("ERROR: Need to process Time value for HE_TYPE_TIMER_UP\n");
                Demo_Printf(0x4000, v13);
                printHudDetails = 1;
            }
            goto LABEL_162;
        case 0xBu:
            if ( hudElem->time
                || hudElem->fxBirthTime
                || hudElem->fadeStartTime
                || hudElem->moveStartTime
                || hudElem->scaleStartTime )
            {
                v14 = va("ERROR: Need to process Time value for HE_TYPE_TENTHS_TIMER_DOWN\n");
                Demo_Printf(0x4000, v14);
                printHudDetails = 1;
            }
            goto LABEL_162;
        case 0xCu:
            if ( hudElem->time
                || hudElem->fxBirthTime
                || hudElem->fadeStartTime
                || hudElem->moveStartTime
                || hudElem->scaleStartTime )
            {
                v15 = va("ERROR: Need to process Time value for HE_TYPE_TENTHS_TIMER_UP\n");
                Demo_Printf(0x4000, v15);
                printHudDetails = 1;
            }
            goto LABEL_162;
        case 0xDu:
            if ( hudElem->time
                || hudElem->fxBirthTime
                || hudElem->fadeStartTime
                || hudElem->moveStartTime
                || hudElem->scaleStartTime )
            {
                v16 = va("ERROR: Need to process Time value for HE_TYPE_CLOCK_DOWN\n");
                Demo_Printf(0x4000, v16);
                printHudDetails = 1;
            }
            goto LABEL_162;
        case 0xEu:
            if ( hudElem->time
                || hudElem->fxBirthTime
                || hudElem->fadeStartTime
                || hudElem->moveStartTime
                || hudElem->scaleStartTime )
            {
                v17 = va("ERROR: Need to process Time value for HE_TYPE_CLOCK_UP\n");
                Demo_Printf(0x4000, v17);
                printHudDetails = 1;
            }
            goto LABEL_162;
        case 0xFu:
            goto LABEL_162;
        default:
            Com_PrintError(14, "ERROR: Need to process hud elem type %d\n", hudElem->type);
LABEL_162:
            if ( printHudDetails )
            {
                v18 = va("Hud %d: Type: %d\n", index, hudElem->type);
                Demo_Printf(0x4000, v18);
                v19 = va("\tfadeStartTime: %d fadeTime: %d\n", hudElem->fadeStartTime, hudElem->fadeTime);
                Demo_Printf(0x4000, v19);
                v20 = va("\tfxDecayStartTime: %d fxDecayDuration: %d\n", hudElem->fxDecayStartTime, hudElem->fxDecayDuration);
                Demo_Printf(0x4000, v20);
                v21 = va("\tmoveStartTime: %d moveTime: %d\n", hudElem->moveStartTime, hudElem->moveTime);
                Demo_Printf(0x4000, v21);
                v22 = va("\tscaleStartTime: %d scaleTime: %d\n", hudElem->scaleStartTime, hudElem->scaleTime);
                Demo_Printf(0x4000, v22);
                v23 = va("\ttime: %d\n", hudElem->time);
                Demo_Printf(0x4000, v23);
                v24 = va("\tfxBirthTime: %d\n", hudElem->fxBirthTime);
                Demo_Printf(0x4000, v24);
                v25 = va("\tfxLetterTime: %d\n", hudElem->fxLetterTime);
                Demo_Printf(0x4000, v25);
            }
            return;
    }
}

void __cdecl Demo_AdjustTimeForEntityState(int currentTime, int clipTime, entityState_s *state)
{
    int delta; // [esp+0h] [ebp-4h]
    int deltaa; // [esp+0h] [ebp-4h]
    int deltab; // [esp+0h] [ebp-4h]
    int deltac; // [esp+0h] [ebp-4h]
    int deltad; // [esp+0h] [ebp-4h]
    int deltae; // [esp+0h] [ebp-4h]

    if ( state->time2 )
    {
        delta = clipTime + state->time2 - currentTime;
        if ( delta <= 0 )
            state->time2 = 0;
        else
            state->time2 = delta;
    }
    if ( state->lerp.pos.trTime )
    {
        deltaa = clipTime + state->lerp.pos.trTime - currentTime;
        if ( deltaa <= 0 )
            state->lerp.pos.trTime = 0;
        else
            state->lerp.pos.trTime = deltaa;
    }
    if ( state->lerp.apos.trTime )
    {
        deltab = clipTime + state->lerp.apos.trTime - currentTime;
        if ( deltab <= 0 )
            state->lerp.apos.trTime = 0;
        else
            state->lerp.apos.trTime = deltab;
    }
    if ( !state->eType && state->lerp.u.actor.actorNum )
    {
        deltac = clipTime + state->lerp.u.actor.actorNum - currentTime;
        if ( deltac <= 0 )
            state->lerp.u.actor.actorNum = 0;
        else
            state->lerp.u.actor.actorNum = deltac;
    }
    if ( state->eType == 4 )
    {
        if ( state->lerp.u.actor.team )
        {
            deltad = clipTime + state->lerp.u.actor.team - currentTime;
            if ( deltad <= 0 )
                state->lerp.u.actor.team = 0;
            else
                state->lerp.u.actor.team = deltad;
        }
        if ( state->lerp.u.actor.actorNum )
        {
            deltae = clipTime + state->lerp.u.actor.actorNum - currentTime;
            if ( deltae <= 0 )
                state->lerp.u.actor.actorNum = 0;
            else
                state->lerp.u.actor.actorNum = deltae;
        }
    }
}

void __cdecl Demo_WriteDemoPreviewPoint(int localClientNum, int restorePoint)
{
    clientActive_t *LocalClientGlobals; // [esp+8h] [ebp-6Ch]
    msg_t msg; // [esp+10h] [ebp-64h] BYREF
    clientConnection_t *clc; // [esp+40h] [ebp-34h]
    msg_t previewMsg; // [esp+44h] [ebp-30h] BYREF

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    clc = CL_GetLocalClientConnection(localClientNum);
    demo.playback->demoCompleteStateForRestorationPoint[restorePoint] = Demo_IsCompleted();
    PROF_SCOPED("Preview Keyframe - Generation");

    MSG_Init(&msg, demo.msgBuf1, 49152);
    demo.playback->forceWriteClipCommands = 1;
    Demo_WriteClipCommandsInternal(localClientNum, &msg, 0);
    Demo_GenerateUncompressedSnapshot(
        localClientNum,
        &msg,
        clc->serverMessageSequence,
        clc->serverCommandSequence,
        LocalClientGlobals->serverTime,
        LocalClientGlobals->serverTime,
        2,
        1);
    MSG_Init(&previewMsg, demo.playback->demoPreviewBuf[restorePoint], 49152);
    MSG_Embed(&previewMsg, &msg);
    demo.playback->demoPreview[restorePoint].demoFileMemoryBlockStart = demo.totalSize - demo.memBlock.compressedSize - 8;
    demo.playback->demoPreview[restorePoint].keyframeBufferStart = 0;
    demo.playback->demoPreview[restorePoint].keyframeSnapshotTime = LocalClientGlobals->serverTime;
    demo.playback->demoPreview[restorePoint].firstCmdSequence = demo.playback->lastProcessedCmd;
    demo.playback->demoPreview[restorePoint].size = previewMsg.cursize;
    MSG_SetBookmark(&demo.msg, &demo.playback->demoPreview[restorePoint].msgBookmark);
    if ( demo.playback->demoPreview[restorePoint].size > 0xC000u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                    7020,
                    0,
                    "%s",
                    "demo.playback->demoPreview[restorePoint].size <= sizeof( demo.playback->demoPreviewBuf[restorePoint] )") )
    {
        __debugbreak();
    }
}

void __cdecl Demo_RestoreDemoPreviewPoint(int localClientNum, int restorePoint)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-48h]
    cg_s *cgameGlob; // [esp+4h] [ebp-44h]
    int msgSequence; // [esp+10h] [ebp-38h]
    int msgType; // [esp+14h] [ebp-34h]
    msg_t previewMsg; // [esp+18h] [ebp-30h] BYREF

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    CL_GetLocalClientConnection(localClientNum);
    CG_GetLocalClientStaticGlobals(localClientNum);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( demo.playback->demoCompleteStateForRestorationPoint[restorePoint] )
    {
        Demo_SetCompleted(1);
    }
    else
    {
        demo.totalSize = demo.playback->demoPreview[restorePoint].demoFileMemoryBlockStart;
        Demo_SeekFile(demo.demoFileHandle, demo.playback->demoPreview[restorePoint].demoFileMemoryBlockStart, 2);
        Demo_Load();
        MSG_GotoBookmark(&demo.msg, &demo.playback->demoPreview[restorePoint].msgBookmark);
        MSG_Init(&previewMsg, demo.msgBuf1, 49152);
        memcpy(demo.msgBuf1, demo.playback->demoPreviewBuf[restorePoint], demo.playback->demoPreview[restorePoint].size);
        previewMsg.cursize = demo.playback->demoPreview[restorePoint].size;
        Demo_SetCompleted(0);
        MSG_UnEmbed(&previewMsg);
        Demo_ResetSnapshotData();
        for ( msgType = MSG_ReadBits(&previewMsg, 2u); msgType == 2; msgType = MSG_ReadBits(&previewMsg, 2u) )
            Demo_ParseClipCommand(localClientNum, &previewMsg, 0);
        msgSequence = MSG_ReadLong(&previewMsg);
        Demo_ParseServerMessage(localClientNum, &previewMsg, msgType, msgSequence);
        Demo_ResetTimeValues(localClientNum, LocalClientGlobals->snap.serverTime);
        Demo_ResetSequenceNumbers(localClientNum, LocalClientGlobals->snap.messageNum - 1);
        Demo_ResetWorldInformation(localClientNum, LocalClientGlobals->snap.serverTime);
        cgameGlob->latestSnapshotNum = LocalClientGlobals->snap.messageNum;
        LocalClientGlobals->serverTimeDelta = LocalClientGlobals->snap.serverTime - cls.realtime;
        SND_StopSounds(SND_STOP_ALL);
        demo.playback->overridePause = 1;
        CL_SetCGameTime(localClientNum);
        demo.playback->overridePause = 0;
    }
}

void __cdecl Demo_RestoreUIStateAfterPreview(int localClientNum)
{
    Demo_SetClipState(3);
    Dvar_SetBool((dvar_s *)demo_pause, 1);
    if ( demo.playback->pendingSegmentSave )
    {
        Demo_RestoreDemoPreviewPoint(localClientNum, 1);
        Demo_OpenSavePopUp(localClientNum, 0);
    }
    else if ( demo.playback->previewAllSegments )
    {
        Demo_RestoreDemoPreviewPoint(localClientNum, 0);
        Key_SetCatcher(localClientNum, 16);
        UI_OpenMenu(localClientNum, "demo_manage_segments");
        demo.playback->previewAllSegments = 0;
    }
    else
    {
        Demo_RestoreDemoPreviewPoint(localClientNum, 0);
        Key_SetCatcher(localClientNum, 16);
        UI_OpenMenu(localClientNum, "demo_manage_segments");
    }
}

void __cdecl Demo_HandleInput(int localClientNum, int controllerIndex, int key, int pressed)
{
    const char *v4; // eax
    unsigned int v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    const char *v12; // eax
    const char *v13; // eax
    int v14; // eax
    int v15; // eax
    const char *v16; // eax
    char *v17; // [esp+4h] [ebp-3Ch]
    char *v18; // [esp+4h] [ebp-3Ch]
    float v19; // [esp+8h] [ebp-38h]
    float v20; // [esp+Ch] [ebp-34h]
    float v21; // [esp+14h] [ebp-2Ch]
    int v22; // [esp+1Ch] [ebp-24h]
    float v23; // [esp+20h] [ebp-20h]
    int integer; // [esp+28h] [ebp-18h]
    char isClipRecording; // [esp+2Dh] [ebp-13h]
    bool isClipPlaying; // [esp+2Eh] [ebp-12h]
    char isPauseMenuOpen; // [esp+2Fh] [ebp-11h]
    bool isScoreboardDisplayed; // [esp+30h] [ebp-10h]
    bool isErrorDialogOpen; // [esp+3Ch] [ebp-4h]
    bool isSaveDialogOpen; // [esp+3Dh] [ebp-3h]
    bool isMetaInformationScreenActive; // [esp+3Fh] [ebp-1h]

    if ( Demo_IsPlaying() && !Demo_IsRenderingScreenshot(localClientNum) )
    {
        if ( Demo_IsRenderingMovie() )
        {
            if ( key == 2 )
            {
                v4 = va("demo_stop 1\n");
                Cbuf_AddText(localClientNum, v4);
            }
        }
        else if ( Demo_IsClipPreviewRunning() )
        {
            if ( key == 2 )
                demo.playback->cancelPreview = 1;
        }
        else if ( demo.playback && demo.playback->holdCmd.active && demo.playback->holdCmd.key == key )
        {
            v5 = Sys_Milliseconds();
            Demo_HandleHoldInput(
                localClientNum,
                demo.playback->holdCmd.key,
                (int)(v5 - demo.playback->holdCmd.activatedTime) >= 250);
        }
        else
        {
            isPauseMenuOpen = Demo_IsPauseMenuOpen(localClientNum);
            isScoreboardDisplayed = CG_IsScoreboardDisplayed(localClientNum);
            isSaveDialogOpen = Demo_IsSaveDialogOpen(localClientNum);
            isErrorDialogOpen = Demo_IsErrorDialogOpen(localClientNum);
            isMetaInformationScreenActive = Demo_IsMetaInformationScreenActive();
            if ( CL_GetLocalClientConnectionState(localClientNum) == 10
                && !isPauseMenuOpen
                && !isScoreboardDisplayed
                && !isSaveDialogOpen
                && !isErrorDialogOpen
                && !isMetaInformationScreenActive )
            {
                isClipRecording = Demo_IsClipRecording();
                isClipPlaying = Demo_IsClipPlaying();
                switch ( key )
                {
                    case 1:
                    case 32:
                        if ( pressed )
                        {
                            v8 = va("demo_pause %i\n", demo_cmdNum->current.integer);
                            Cbuf_AddText(localClientNum, v8);
                            Demo_SetButtonPressTime(0);
                        }
                        break;
                    case 2:
                    case 170:
                        if ( !isClipPlaying && pressed )
                        {
                            v9 = va("demo_togglegamehud %i\n", demo_cmdNum->current.integer);
                            Cbuf_AddText(localClientNum, v9);
                            Demo_SetButtonPressTime(4u);
                        }
                        break;
                    case 3:
                    case 114:
                        if ( pressed )
                        {
                            if ( isClipRecording )
                            {
                                v11 = va("demo_pausecliprecord %i\n", demo_cmdNum->current.integer);
                                Cbuf_AddText(localClientNum, v11);
                                Demo_SetButtonPressTime(1u);
                            }
                            else
                            {
                                Demo_ActivateHoldInput(localClientNum, key);
                            }
                        }
                        break;
                    case 4:
                    case 168:
                        if ( !isClipPlaying && pressed )
                        {
                            v10 = va("demo_switchcamera %i\n", demo_cmdNum->current.integer);
                            Cbuf_AddText(localClientNum, v10);
                            Demo_SetButtonPressTime(3u);
                        }
                        break;
                    case 5:
                    case 200:
                        if ( pressed )
                        {
                            v6 = va("demo_switchplayer %i 1\n", demo_cmdNum->current.integer);
                            Cbuf_AddText(localClientNum, v6);
                            Demo_SetButtonPressTime(2u);
                            UI_AnimateItem(localClientNum, &cgDC[localClientNum], "demoPlayers", "CycleNext", "Flash", 1);
                            UI_AnimateItem(localClientNum, &cgDC[localClientNum], "demoPlayers", "SpectatorName", "Flash", 1);
                        }
                        break;
                    case 6:
                    case 201:
                        if ( pressed )
                        {
                            v7 = va("demo_switchplayer %i 0\n", demo_cmdNum->current.integer);
                            Cbuf_AddText(localClientNum, v7);
                            Demo_SetButtonPressTime(2u);
                            UI_AnimateItem(localClientNum, &cgDC[localClientNum], "demoPlayers", "CyclePrev", "Flash", 1);
                            UI_AnimateItem(localClientNum, &cgDC[localClientNum], "demoPlayers", "SpectatorName", "Flash", 1);
                        }
                        break;
                    case 16:
                    case 167:
                        if ( pressed )
                        {
                            v12 = va("demo_toggledemohud %i\n", demo_cmdNum->current.integer);
                            Cbuf_AddText(localClientNum, v12);
                            Demo_SetButtonPressTime(5u);
                        }
                        break;
                    case 154:
                        if ( !isClipPlaying && pressed )
                        {
                            v23 = Com_GetTimeScale() + 0.2;
                            if ( (float)(v23 - demo_timeScaleRate->current.value) < 0.0 )
                                integer = LODWORD(v23);
                            else
                                integer = demo_timeScaleRate->current.integer;
                            if ( (float)(0.2 - v23) < 0.0 )
                                v20 = *(float *)&integer;
                            else
                                v20 = 0.2f;
                            v17 = va("demo_timescale %i %f\n", demo_cmdNum->current.integer, v20);
                            v14 = Com_LocalClient_GetControllerIndex(localClientNum);
                            Cmd_ExecuteSingleCommand(localClientNum, v14, v17);
                            Demo_SetButtonPressTime(9u);
                        }
                        break;
                    case 155:
                        if ( !isClipPlaying && pressed )
                        {
                            v21 = Com_GetTimeScale() - 0.2;
                            if ( (float)(v21 - demo_timeScaleRate->current.value) < 0.0 )
                                v22 = LODWORD(v21);
                            else
                                v22 = demo_timeScaleRate->current.integer;
                            if ( (float)(0.2 - v21) < 0.0 )
                                v19 = *(float *)&v22;
                            else
                                v19 = 0.2f;
                            v18 = va("demo_timescale %i %f\n", demo_cmdNum->current.integer, v19);
                            v15 = Com_LocalClient_GetControllerIndex(localClientNum);
                            Cmd_ExecuteSingleCommand(localClientNum, v15, v18);
                            Demo_SetButtonPressTime(8u);
                        }
                        break;
                    case 156:
                        if ( pressed && !isClipRecording )
                        {
                            v13 = va("demo_back %i\n", demo_cmdNum->current.integer);
                            Cbuf_AddText(localClientNum, v13);
                            Demo_SetButtonPressTime(6u);
                        }
                        break;
                    case 157:
                        if ( !isClipRecording && pressed )
                        {
                            v16 = va("demo_forward %i\n", demo_cmdNum->current.integer);
                            Cbuf_AddText(localClientNum, v16);
                            Demo_SetButtonPressTime(7u);
                        }
                        break;
                    default:
                        Demo_HandleInputForControllerConfig(localClientNum, controllerIndex, key, pressed);
                        break;
                }
            }
        }
    }
}

void __cdecl Demo_HandleInputForControllerConfig(int localClientNum, int controllerIndex, int key, int pressed)
{
    int CurrentControllerConfig; // eax
    const char *v5; // eax
    int v6; // eax
    int v7; // eax
    int v8; // eax
    int v9; // eax
    int v10; // eax
    int v11; // eax
    int v12; // eax
    const char *v13; // eax
    int v14; // eax
    int v15; // eax
    int v16; // eax
    int v17; // eax
    int v18; // eax
    int v19; // eax
    const char *v20; // eax
    const char *v21; // eax
    const char *v22; // eax
    const char *v23; // eax
    int v24; // eax
    int v25; // eax
    char *v26; // [esp+4h] [ebp-44h]
    char *v27; // [esp+4h] [ebp-44h]
    char *v28; // [esp+4h] [ebp-44h]
    char *v29; // [esp+4h] [ebp-44h]
    char *v30; // [esp+4h] [ebp-44h]
    char *v31; // [esp+4h] [ebp-44h]
    char *v32; // [esp+4h] [ebp-44h]
    char *v33; // [esp+4h] [ebp-44h]
    char *v34; // [esp+4h] [ebp-44h]
    char *v35; // [esp+4h] [ebp-44h]
    char *v36; // [esp+4h] [ebp-44h]
    char *v37; // [esp+4h] [ebp-44h]
    char *v38; // [esp+4h] [ebp-44h]
    char *v39; // [esp+4h] [ebp-44h]
    char *v40; // [esp+4h] [ebp-44h]
    float v41; // [esp+8h] [ebp-40h]
    float v42; // [esp+Ch] [ebp-3Ch]
    float v43; // [esp+18h] [ebp-30h]
    float v44; // [esp+20h] [ebp-28h]
    int v45; // [esp+28h] [ebp-20h]
    float v46; // [esp+2Ch] [ebp-1Ch]
    int integer; // [esp+34h] [ebp-14h]
    float timeScale; // [esp+38h] [ebp-10h]
    char isClipRecording; // [esp+3Eh] [ebp-Ah]
    bool isClipPlaying; // [esp+3Fh] [ebp-9h]
    float trigValue; // [esp+40h] [ebp-8h]
    float trigValuea; // [esp+40h] [ebp-8h]
    float trigValueb; // [esp+40h] [ebp-8h]

    isClipRecording = Demo_IsClipRecording();
    isClipPlaying = Demo_IsClipPlaying();
    CurrentControllerConfig = Demo_GetCurrentControllerConfig();
    if ( CurrentControllerConfig )
    {
        if ( CurrentControllerConfig == 1 )
        {
            switch ( key )
            {
                case 18:
                    if ( !isClipPlaying )
                    {
                        trigValuea = GPad_GetButton(controllerIndex, GPAD_L_TRIG);
                        if ( !GPad_IsButtonPressed(controllerIndex, GPAD_R_TRIG) )
                        {
                            if ( trigValuea <= 0.80000001 )
                            {
                                if ( Com_GetTimeScale() != 1.0 )
                                {
                                    v35 = va("demo_timescale %i %f\n", demo_cmdNum->current.integer, 1.0);
                                    v16 = Com_LocalClient_GetControllerIndex(localClientNum);
                                    Cmd_ExecuteSingleCommand(localClientNum, v16, v35);
                                }
                            }
                            else
                            {
                                if ( Demo_IsPaused() )
                                {
                                    v33 = va("demo_pause %i\n", demo_cmdNum->current.integer);
                                    v14 = Com_LocalClient_GetControllerIndex(localClientNum);
                                    Cmd_ExecuteSingleCommand(localClientNum, v14, v33);
                                }
                                if ( Com_GetTimeScale() != 0.30000001 )
                                {
                                    v34 = va("demo_timescale %i %f\n", demo_cmdNum->current.integer, 0.30000001);
                                    v15 = Com_LocalClient_GetControllerIndex(localClientNum);
                                    Cmd_ExecuteSingleCommand(localClientNum, v15, v34);
                                }
                                Demo_SetButtonPressTime(8u);
                            }
                        }
                    }
                    break;
                case 19:
                    if ( !isClipPlaying )
                    {
                        trigValueb = GPad_GetButton(controllerIndex, GPAD_R_TRIG);
                        if ( !GPad_IsButtonPressed(controllerIndex, GPAD_L_TRIG) )
                        {
                            if ( trigValueb <= 0.80000001 )
                            {
                                if ( Com_GetTimeScale() != 1.0 )
                                {
                                    v38 = va("demo_timescale %i %f\n", demo_cmdNum->current.integer, 1.0);
                                    v19 = Com_LocalClient_GetControllerIndex(localClientNum);
                                    Cmd_ExecuteSingleCommand(localClientNum, v19, v38);
                                }
                            }
                            else
                            {
                                if ( Demo_IsPaused() )
                                {
                                    v36 = va("demo_pause %i\n", demo_cmdNum->current.integer);
                                    v17 = Com_LocalClient_GetControllerIndex(localClientNum);
                                    Cmd_ExecuteSingleCommand(localClientNum, v17, v36);
                                }
                                if ( Com_GetTimeScale() != 3.0 )
                                {
                                    v37 = va("demo_timescale %i %f\n", demo_cmdNum->current.integer, 3.0);
                                    v18 = Com_LocalClient_GetControllerIndex(localClientNum);
                                    Cmd_ExecuteSingleCommand(localClientNum, v18, v37);
                                }
                                Demo_SetButtonPressTime(9u);
                            }
                        }
                    }
                    break;
                case 22:
                    if ( !isClipRecording && pressed )
                    {
                        v20 = va("demo_back %i\n", demo_cmdNum->current.integer);
                        Cbuf_AddText(localClientNum, v20);
                        Demo_SetButtonPressTime(6u);
                    }
                    break;
                case 23:
                    if ( !isClipRecording && pressed )
                    {
                        v21 = va("demo_forward %i\n", demo_cmdNum->current.integer);
                        Cbuf_AddText(localClientNum, v21);
                        Demo_SetButtonPressTime(7u);
                    }
                    break;
                default:
                    return;
            }
        }
        else if ( CurrentControllerConfig == 2 )
        {
            switch ( key )
            {
                case 18:
                    if ( !isClipRecording && pressed )
                    {
                        v22 = va("demo_back %i\n", demo_cmdNum->current.integer);
                        Cbuf_AddText(localClientNum, v22);
                        Demo_SetButtonPressTime(6u);
                    }
                    break;
                case 19:
                    if ( !isClipRecording && pressed )
                    {
                        v23 = va("demo_forward %i\n", demo_cmdNum->current.integer);
                        Cbuf_AddText(localClientNum, v23);
                        Demo_SetButtonPressTime(7u);
                    }
                    break;
                case 20:
                    if ( !isClipPlaying && pressed )
                    {
                        v46 = Com_GetTimeScale() + 0.2;
                        if ( (float)(v46 - demo_timeScaleRate->current.value) < 0.0 )
                            integer = LODWORD(v46);
                        else
                            integer = demo_timeScaleRate->current.integer;
                        if ( (float)(0.2 - v46) < 0.0 )
                            v42 = *(float *)&integer;
                        else
                            v42 = 0.2f;
                        v39 = va("demo_timescale %i %f\n", demo_cmdNum->current.integer, v42);
                        v24 = Com_LocalClient_GetControllerIndex(localClientNum);
                        Cmd_ExecuteSingleCommand(localClientNum, v24, v39);
                        Demo_SetButtonPressTime(9u);
                    }
                    break;
                case 21:
                    if ( !isClipPlaying && pressed )
                    {
                        v44 = Com_GetTimeScale() - 0.2;
                        if ( (float)(v44 - demo_timeScaleRate->current.value) < 0.0 )
                            v45 = LODWORD(v44);
                        else
                            v45 = demo_timeScaleRate->current.integer;
                        if ( (float)(0.2 - v44) < 0.0 )
                            v41 = *(float *)&v45;
                        else
                            v41 = 0.2f;
                        v40 = va("demo_timescale %i %f\n", demo_cmdNum->current.integer, v41);
                        v25 = Com_LocalClient_GetControllerIndex(localClientNum);
                        Cmd_ExecuteSingleCommand(localClientNum, v25, v40);
                        Demo_SetButtonPressTime(8u);
                    }
                    break;
                default:
                    return;
            }
        }
    }
    else
    {
        switch ( key )
        {
            case 18:
            case 163:
                if ( pressed )
                {
                    if ( !isClipRecording )
                    {
                        v5 = va("demo_back %i\n", demo_cmdNum->current.integer);
                        Cbuf_AddText(localClientNum, v5);
                        Demo_SetButtonPressTime(6u);
                    }
                }
                break;
            case 19:
                if ( !isClipPlaying )
                {
                    trigValue = GPad_GetButton(controllerIndex, GPAD_R_TRIG);
                    if ( trigValue <= 0.0 )
                    {
                        timeScale = 1.0f;
                        if ( !Demo_IsPaused() )
                        {
                            v27 = va("demo_pause %i\n", demo_cmdNum->current.integer);
                            v7 = Com_LocalClient_GetControllerIndex(localClientNum);
                            Cmd_ExecuteSingleCommand(localClientNum, v7, v27);
                        }
                    }
                    else
                    {
                        if ( Demo_IsPaused() )
                        {
                            v26 = va("demo_pause %i\n", demo_cmdNum->current.integer);
                            v6 = Com_LocalClient_GetControllerIndex(localClientNum);
                            Cmd_ExecuteSingleCommand(localClientNum, v6, v26);
                        }
                        if ( (float)((float)(trigValue * demo_timeScaleRate->current.value) - 0.1) < 0.0 )
                            v43 = 0.1f;
                        else
                            v43 = trigValue * demo_timeScaleRate->current.value;
                        timeScale = v43;
                        if ( v43 >= 1.0 )
                            Demo_SetButtonPressTime(9u);
                        else
                            Demo_SetButtonPressTime(8u);
                    }
                    goto LABEL_33;
                }
                break;
            case 23:
            case 164:
                if ( !isClipRecording && pressed )
                {
                    v13 = va("demo_forward %i\n", demo_cmdNum->current.integer);
                    Cbuf_AddText(localClientNum, v13);
                    Demo_SetButtonPressTime(7u);
                }
                break;
            case 156:
                if ( !isClipPlaying )
                {
                    if ( pressed )
                    {
                        if ( Demo_IsPaused() )
                        {
                            v30 = va("demo_pause %i\n", demo_cmdNum->current.integer);
                            v10 = Com_LocalClient_GetControllerIndex(localClientNum);
                            Cmd_ExecuteSingleCommand(localClientNum, v10, v30);
                        }
                        timeScale = 0.3f;
                        Demo_SetButtonPressTime(8u);
                    }
                    else
                    {
                        timeScale = 1.0f;
                        if ( !Demo_IsPaused() )
                        {
                            v31 = va("demo_pause %i\n", demo_cmdNum->current.integer);
                            v11 = Com_LocalClient_GetControllerIndex(localClientNum);
                            Cmd_ExecuteSingleCommand(localClientNum, v11, v31);
                        }
                    }
                    goto LABEL_33;
                }
                break;
            case 157:
                if ( !isClipPlaying )
                {
                    if ( pressed )
                    {
                        if ( Demo_IsPaused() )
                        {
                            v28 = va("demo_pause %i\n", demo_cmdNum->current.integer);
                            v8 = Com_LocalClient_GetControllerIndex(localClientNum);
                            Cmd_ExecuteSingleCommand(localClientNum, v8, v28);
                        }
                        timeScale = demo_timeScaleRate->current.value;
                        Demo_SetButtonPressTime(9u);
                    }
                    else
                    {
                        timeScale = 1.0f;
                        v29 = va("demo_pause %i\n", demo_cmdNum->current.integer);
                        v9 = Com_LocalClient_GetControllerIndex(localClientNum);
                        Cmd_ExecuteSingleCommand(localClientNum, v9, v29);
                    }
LABEL_33:
                    v32 = va("demo_timescale %i %f\n", demo_cmdNum->current.integer, timeScale);
                    v12 = Com_LocalClient_GetControllerIndex(localClientNum);
                    Cmd_ExecuteSingleCommand(localClientNum, v12, v32);
                }
                break;
            default:
                return;
        }
    }
}

void __cdecl Demo_SetButtonPressTime(unsigned int demoControlButtonType)
{
    if ( demoControlButtonType >= 0xA
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp",
                    7604,
                    0,
                    "%s",
                    "demoControlButtonType >= 0 && demoControlButtonType < DEMO_CONTROLS_BUTTON_COUNT") )
    {
        __debugbreak();
    }
    demo.playback->buttonPressTime[demoControlButtonType] = Sys_Milliseconds();
}

void __cdecl Demo_ActivateHoldInput(int localClientNum, int key)
{
    if ( demo.playback && demo.playback->holdCmd.active )
    {
        Com_PrintError(16, "Cannot activate hold button when theres already an active hold command.");
    }
    else
    {
        demo.playback->holdCmd.activatedTime = Sys_Milliseconds();
        demo.playback->holdCmd.key = key;
        demo.playback->holdCmd.active = 1;
    }
}

void __cdecl Demo_HandleHoldInput(int localClientNum, int key, bool holdResult)
{
    int ControllerIndex; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    char *v7; // [esp-4h] [ebp-18h]
    char isPauseMenuOpen; // [esp+7h] [ebp-Dh]
    bool isScoreboardDisplayed; // [esp+Ch] [ebp-8h]
    bool isErrorDialogOpen; // [esp+11h] [ebp-3h]
    bool isSaveDialogOpen; // [esp+12h] [ebp-2h]
    bool isMetaInformationScreenActive; // [esp+13h] [ebp-1h]

    isPauseMenuOpen = Demo_IsPauseMenuOpen(localClientNum);
    isScoreboardDisplayed = CG_IsScoreboardDisplayed(localClientNum);
    isSaveDialogOpen = Demo_IsSaveDialogOpen(localClientNum);
    isErrorDialogOpen = Demo_IsErrorDialogOpen(localClientNum);
    isMetaInformationScreenActive = Demo_IsMetaInformationScreenActive();
    if ( isPauseMenuOpen
        || isScoreboardDisplayed
        || isSaveDialogOpen
        || isErrorDialogOpen
        || isMetaInformationScreenActive )
    {
        demo.playback->holdCmd.active = 0;
    }
    else
    {
        if ( key == 3 || key == 114 )
        {
            if ( holdResult )
            {
                if ( !Demo_IsPaused() )
                {
                    v7 = va("demo_pause %i\n", demo_cmdNum->current.integer);
                    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
                    Cmd_ExecuteSingleCommand(localClientNum, ControllerIndex, v7);
                }
                CG_GetLocalClientGlobals(localClientNum)->hiResShotMode = 1;
                v4 = va("demo_screenshot 2\n");
                Cbuf_AddText(localClientNum, v4);
            }
            else
            {
                v5 = va("demo_startcliprecord %i\n", demo_cmdNum->current.integer);
                Cbuf_AddText(localClientNum, v5);
                Demo_SetButtonPressTime(1u);
            }
        }
        else
        {
            v6 = va("Unhandled Hold Button Input %d\n", key);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_playback.cpp", 7695, 0, v6) )
                __debugbreak();
        }
        demo.playback->holdCmd.active = 0;
    }
}

