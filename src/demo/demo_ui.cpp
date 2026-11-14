#include "demo_ui.h"
#include "demo_common.h"
#include <qcommon/common.h>
#include <ui/ui_main.h>
#include <qcommon/com_clients.h>
#include <client/cl_keys.h>
#include <universal/offline.h>
#include "demo_playback.h"
#include <client/client.h>
#include <win32/win_shared.h>
#include <client_mp/cl_cgame_mp.h>


const char *demo_transition_enum_string_40[2] = { "MENU_DEMO_TRANSITION_CUT", "MENU_DEMO_TRANSITION_FADE" };


const char *__cdecl Demo_GetKeyboardTitle()
{
    if ( demo.keyboard.title )
        return demo.keyboard.title;
    else
        return "";
}

const char *__cdecl Demo_GetKeyboardDefaultText()
{
    if ( demo.keyboard.defaultText )
        return demo.keyboard.defaultText;
    else
        return "";
}

int __cdecl Demo_GetKeyboardTextSize()
{
    if ( demo.keyboard.textSize > 0 )
        return demo.keyboard.textSize;
    else
        return 0;
}

void __cdecl Demo_KeyboardComplete(int localClientNum, char *string, bool errorReported, char *errorString)
{
    int ControllerIndex; // eax
    int v5; // eax
    char *v6; // [esp-4h] [ebp-Ch]

    if ( errorReported )
    {
        if ( demo.keyboard.mode <= 0 || demo.keyboard.mode > 5 )
        {
            Com_SetErrorMessage(errorString);
            UI_OpenMenu(localClientNum, "error_popmenu_keyboard");
        }
        else
        {
            Demo_Error(localClientNum, "MPUI_ERROR_CAPS", errorString);
        }
    }
    else
    {
        switch ( demo.keyboard.mode )
        {
            case 1:
                I_strncpyz(
                    demo.playback->segments[demo.keyboard.currentProcessingSegmentIndex].name,
                    string,
                    demo.keyboard.textSize);
                ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
                if ( Menu_IsMenuOpenAndVisible(ControllerIndex, "demo_save_clip") )
                {
                    v6 = va("demo_savesegment %d 0", demo_cmdNum->current.integer);
                    v5 = Com_LocalClient_GetControllerIndex(localClientNum);
                    Cmd_ExecuteSingleCommand(localClientNum, v5, v6);
                    Key_RemoveCatcher(localClientNum, -17);
                    UI_CloseMenu(localClientNum, "demo_save_clip");
                }
                break;
            case 2:
                I_strncpyz(demo.playback->clipRecordInfo.name, string, demo.keyboard.textSize);
                demo.playback->clipRecordInfo.isModifiedName = 1;
                break;
            case 3:
                I_strncpyz(demo.playback->clipRecordInfo.description, string, demo.keyboard.textSize);
                demo.playback->clipRecordInfo.isModifiedDescription = 1;
                break;
            case 4:
                I_strncpyz(demo.playback->screenshotInfo.name, string, demo.keyboard.textSize);
                demo.playback->screenshotInfo.isModifiedName = 1;
                break;
            case 5:
                I_strncpyz(demo.playback->screenshotInfo.description, string, demo.keyboard.textSize);
                demo.playback->screenshotInfo.isModifiedDescription = 1;
                break;
            case 6:
                Dvar_SetString((dvar_s *)fsSelectedFileName, string);
                Dvar_SetBool((dvar_s *)fsIsSelectedFileNameModified, 1);
                break;
            case 7:
                Dvar_SetString((dvar_s *)fsSelectedFileDescription, string);
                Dvar_SetBool((dvar_s *)fsIsSelectedFileDescriptionModified, 1);
                break;
            default:
                break;
        }
    }
    demo.keyboard.mode = 0;
    demo.keyboard.title = "";
    demo.keyboard.defaultText = "";
    demo.keyboard.textSize = 0;
}

void __cdecl Demo_GetStartAndEndTime(int *startTime, int *endTime)
{
    *startTime = demo.info.startTime;
    *endTime = demo.info.endTime;
}

void __cdecl Demo_GetDuration(char *outputText, int duration)
{
    sprintf(outputText, "%02i:%02i", duration / 60000, duration % 60000 / 1000);
}

char *__cdecl Demo_GetAuthor()
{
    return demo.info.author;
}

char *__cdecl Demo_GetTitleName()
{
    return demo.info.name;
}

char *__cdecl Demo_GetTitleDescription()
{
    return demo.info.description;
}

char *__cdecl Demo_GetTimeInfo()
{
    const char *MonthAbbrev; // eax
    char *s; // [esp+0h] [ebp-4h]

    MonthAbbrev = UI_GetMonthAbbrev(demo.info.month - 1);
    s = UI_SafeTranslateString(MonthAbbrev);
    return va("%s %d, %d    %02d:%02d", s, demo.info.day, demo.info.year, demo.info.hour, demo.info.min);
}

char *__cdecl Demo_GetDuration()
{
    return va(
                     "%02i:%02i",
                     (demo.info.endTime - demo.info.startTime) / 60000,
                     (demo.info.endTime - demo.info.startTime) % 60000 / 1000);
}

char *__cdecl Demo_GetSaveScreenName(const char *type)
{
    if ( !Demo_IsPlaybackInited() )
        return (char *)"";
    if ( !I_stricmp(type, "clip") )
        return demo.playback->clipRecordInfo.name;
    if ( I_stricmp(type, "screenshot") )
        return (char *)"";
    return demo.playback->screenshotInfo.name;
}

char *__cdecl Demo_GetSaveScreenDescription(const char *type)
{
    if ( !Demo_IsPlaybackInited() )
        return (char *)"";
    if ( !I_stricmp(type, "clip") )
        return demo.playback->clipRecordInfo.description;
    if ( I_stricmp(type, "screenshot") )
        return (char *)"";
    return demo.playback->screenshotInfo.description;
}

void __cdecl Demo_GetConnectionTimeInfoForPlayer(int localClientNum, int *connectTime, int *disconnectTime)
{
    int snapTime; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]
    int clientNum; // [esp+Ch] [ebp-4h]

    clientNum = demo_client->current.integer;
    snapTime = CL_GetLocalClientGlobals(localClientNum)->snap.serverTime;
    for ( i = 0; i < demo.info.numConnectedPlayersInfoCount; ++i )
    {
        if ( demo.info.connectedPlayers[i].clientNum == clientNum
            && demo.info.connectedPlayers[i].connectTime <= snapTime
            && demo.info.connectedPlayers[i].disconnectTime >= snapTime )
        {
            *connectTime = demo.info.connectedPlayers[i].connectTime;
            *disconnectTime = demo.info.connectedPlayers[i].disconnectTime;
            return;
        }
    }
}

int __cdecl Demo_GetPlayerIndexForClientNum(int clientNum, int serverTime)
{
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < demo.info.numConnectedPlayersInfoCount; ++i )
    {
        if ( demo.info.connectedPlayers[i].clientNum == clientNum
            && (demo.info.connectedPlayers[i].connectTime <= serverTime
             && demo.info.connectedPlayers[i].disconnectTime >= serverTime
             || serverTime < 0) )
        {
            return i;
        }
    }
    return -1;
}

int __cdecl Demo_GetNextDefaultBookmarkForPlayer(
                int localClientNum,
                int index,
                int *type,
                int *time,
                float (*color)[4])
{
    const char *v5; // eax
    unsigned __int8 v7; // [esp+0h] [ebp-18h]
    int playerIndex; // [esp+8h] [ebp-10h]
    int snapTime; // [esp+Ch] [ebp-Ch]
    int i; // [esp+10h] [ebp-8h]
    int clientNum; // [esp+14h] [ebp-4h]

    clientNum = demo_client->current.integer;
    snapTime = CL_GetLocalClientGlobals(localClientNum)->snap.serverTime;
    playerIndex = Demo_GetPlayerIndexForClientNum(clientNum, snapTime);
    for ( i = index + 1; i < demo.info.numDefaultBookmarksCount; ++i )
    {
        if ( playerIndex >= 0
            && demo.info.connectedPlayers[playerIndex].connectTime <= demo.info.defaultBookmarks[i].time
            && demo.info.connectedPlayers[playerIndex].disconnectTime >= demo.info.defaultBookmarks[i].time )
        {
            v7 = demo.info.defaultBookmarks[i].type;
            if ( v7 )
            {
                if ( v7 == 1 )
                {
                    (*color)[0] = colorBlue[0];
                    (*color)[1] = 0.0;
                    (*color)[2] = 1.0;
                    (*color)[3] = 1.0;
                    goto LABEL_23;
                }
                if ( v7 != 2 )
                {
                    v5 = va("Unknown bookmark type %d\n", demo.info.defaultBookmarks[i].type);
                    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_ui.cpp", 569, 0, v5) )
                        __debugbreak();
                    goto LABEL_23;
                }
                if ( demo.info.defaultBookmarks[i].clientNum1 == clientNum )
                {
                    (*color)[0] = colorYellow[0];
                    (*color)[1] = 1.0;
                    (*color)[2] = 0.0;
                    (*color)[3] = 1.0;
                    goto LABEL_23;
                }
            }
            else
            {
                if ( demo.info.defaultBookmarks[i].clientNum2 == clientNum
                    && demo.info.defaultBookmarks[i].clientNum2 != demo.info.defaultBookmarks[i].clientNum1 )
                {
                    (*color)[0] = colorGreen[0];
                    (*color)[1] = 1.0;
                    (*color)[2] = 0.0;
                    (*color)[3] = 1.0;
LABEL_23:
                    *type = demo.info.defaultBookmarks[i].type;
                    *time = demo.info.defaultBookmarks[i].time;
                    return i;
                }
                if ( demo.info.defaultBookmarks[i].clientNum1 == clientNum )
                {
                    (*color)[0] = colorRed[0];
                    (*color)[1] = 0.0;
                    (*color)[2] = 0.0;
                    (*color)[3] = 1.0;
                    goto LABEL_23;
                }
            }
        }
    }
    return -1;
}

int __cdecl Demo_GetClientNumForXuid(int localClientNum, unsigned __int64 xuid)
{
    int i; // [esp+4h] [ebp-10h]
    bool ignoreServerTime; // [esp+Fh] [ebp-5h]
    int serverTime; // [esp+10h] [ebp-4h]

    serverTime = CL_GetLocalClientGlobals(localClientNum)->snap.serverTime;
    if ( serverTime <= 0 )
        ignoreServerTime = 1;
    for ( i = 0; i < demo.info.numConnectedPlayersInfoCount; ++i )
    {
        if ( __PAIR64__(HIDWORD(demo.info.connectedPlayers[i].xuid), demo.info.connectedPlayers[i].xuid) == xuid
            && (demo.info.connectedPlayers[i].connectTime <= serverTime
             && demo.info.connectedPlayers[i].disconnectTime >= serverTime
             || ignoreServerTime) )
        {
            return demo.info.connectedPlayers[i].clientNum;
        }
    }
    return -1;
}

char __cdecl Demo_GetActivatedColorForButton(int demoControlButtonType, float (*color)[4])
{
    if ( !Demo_IsPlaybackInited() )
        return 0;
    if ( !demo.playback->buttonPressTime[demoControlButtonType] )
        return 0;
    if ( (int)(Sys_Milliseconds() - demo.playback->buttonPressTime[demoControlButtonType]) > 100 )
        return 0;
    (*color)[0] = colorYellow[0];
    (*color)[1] = 1.0;
    (*color)[2] = 0.0;
    (*color)[3] = 1.0;
    return 1;
}

void __cdecl Demo_DrawVerticalLine(
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
    float originalWidth; // [esp+38h] [ebp-4h]

    originalWidth = w;
    ScrPlace_ApplyRect(scrPlace, &x, &y, &w, &h, horzAlign, vertAlign);
    x = (float)SnapFloatToInt(x);
    w = originalWidth;
    CL_DrawStretchPicPhysical(x, y, originalWidth, h, s1, t1, s2, t2, color, material);
}

int __cdecl Demo_GetSegmentCount()
{
    if ( Demo_IsPlaybackInited() )
        return demo.playback->segmentCount;
    else
        return 0;
}

clipSegment *__cdecl Demo_GetSegmentName(int index)
{
    if ( Demo_IsPlaybackInited() )
        return &demo.playback->segments[index];
    else
        return (clipSegment *)"";
}

char *__cdecl Demo_GetSegmentTransition(int index)
{
    char *s; // [esp+4h] [ebp-4h]

    if ( !Demo_IsPlaybackInited() )
        return (char *)"";
    if ( index == demo.playback->segmentCount - 1 )
        s = UI_SafeTranslateString("MENU_NONE");
    else
        s = UI_SafeTranslateString(demo_transition_enum_string_40[demo.playback->segments[index].transition]);
    return va("%s", s);
}

clipSegment *__cdecl Demo_GetSegmentInformation(int index, const char *s)
{
    int i; // [esp+0h] [ebp-1Ch]
    int total; // [esp+4h] [ebp-18h]
    char output[12]; // [esp+8h] [ebp-14h] BYREF
    const char *outputStr; // [esp+18h] [ebp-4h]

    memset(output, 0, sizeof(output));
    outputStr = "";
    total = 0;
    if ( !Demo_IsPlaybackInited() )
        return (clipSegment *)"";
    if ( !I_stricmp(s, "name") )
        return Demo_GetSegmentName(index);
    if ( I_stricmp(s, "duration") )
    {
        if ( I_stricmp(s, "durationLocString") )
        {
            if ( I_stricmp(s, "transition") )
            {
                if ( I_stricmp(s, "totalClipDurationLocString") )
                {
                    return (clipSegment *)"";
                }
                else
                {
                    for ( i = 0; i < Demo_GetSegmentCount(); ++i )
                        total += demo.playback->segments[i].duration;
                    outputStr = UI_SafeTranslateString("MENU_TOTAL_CLIP_DURATION");
                    Demo_GetDuration(output, total);
                    outputStr = UI_ReplaceConversionString((char *)outputStr, output);
                    return (clipSegment *)va("%s", outputStr);
                }
            }
            else
            {
                return (clipSegment *)Demo_GetSegmentTransition(index);
            }
        }
        else
        {
            outputStr = UI_SafeTranslateString("MENU_DEMO_DURATION");
            Demo_GetDuration(output, demo.playback->segments[index].duration);
            outputStr = UI_ReplaceConversionString((char *)outputStr, output);
            return (clipSegment *)va("%s", outputStr);
        }
    }
    else
    {
        Demo_GetDuration(output, demo.playback->segments[index].duration);
        return (clipSegment *)va("%s", output);
    }
}

char __cdecl Demo_IsPauseMenuOpen(int localClientNum)
{
    int menuCount; // [esp+0h] [ebp-30h]
    const char *ingameMenuNames[11]; // [esp+4h] [ebp-2Ch]

    ingameMenuNames[0] = "demo_ingame";
    ingameMenuNames[1] = "demo_manage_segments";
    ingameMenuNames[2] = "demo_button_layout";
    ingameMenuNames[3] = "ingameoptions";
    ingameMenuNames[4] = "ingameoptions_sensitivity";
    ingameMenuNames[5] = "ingameoptions_team_indicator";
    ingameMenuNames[6] = "ingameoptions_volume";
    ingameMenuNames[7] = "ingameoptions_brightness";
    ingameMenuNames[8] = "options_new_pc";
    ingameMenuNames[9] = "options_controls_new_pc";
    ingameMenuNames[10] = 0;
    if ( Key_IsCatcherActive(localClientNum, 16) )
    {
        for ( menuCount = 0; ingameMenuNames[menuCount]; ++menuCount )
        {
            if ( Menu_IsMenuOpenAndVisible(localClientNum, ingameMenuNames[menuCount]) )
                return 1;
        }
    }
    return 0;
}

void __cdecl Demo_OpenSavePopUp(int localClientNum, int type)
{
    uiInfo_s *uiInfo; // [esp+4h] [ebp-4h]

    uiInfo = UI_GetInfo(localClientNum);
    Key_SetCatcher(localClientNum, 16);
    if ( type )
    {
        if ( type == 1 )
            Menus_OpenByName(localClientNum, &uiInfo->uiDC, "demo_save_screenshot");
    }
    else
    {
        Menus_OpenByName(localClientNum, &uiInfo->uiDC, "demo_save_clip");
    }
}

bool __cdecl Demo_IsSaveDialogOpen(int localClientNum)
{
    if ( Menu_IsMenuOpenAndVisible(localClientNum, "demo_save_clip") )
        return 1;
    if ( Menu_IsMenuOpenAndVisible(localClientNum, "demo_save_screenshot") )
        return 1;
    if ( Menu_IsMenuOpenAndVisible(localClientNum, "screenshot_save_screen") )
        return 1;
    return Menu_IsMenuOpenAndVisible(localClientNum, "fileshare_slotselection_scr");
}

void __cdecl Demo_Error(int localClientNum, const char *errorTitle, const char *errorMessage)
{
    char *v3; // eax
    char *v4; // eax
    uiInfo_s *uiInfo; // [esp+0h] [ebp-4h]

    v3 = UI_SafeTranslateString(errorTitle);
    Dvar_SetString((dvar_s *)demo_errortitle, v3);
    v4 = UI_SafeTranslateString(errorMessage);
    Dvar_SetString((dvar_s *)demo_errormessage, v4);
    if ( !Demo_IsPaused() )
        Dvar_SetBool((dvar_s *)demo_pause, 1);
    uiInfo = UI_GetInfo(localClientNum);
    Key_SetCatcher(localClientNum, 16);
    Menus_OpenByName(localClientNum, &uiInfo->uiDC, "demo_error");
}

bool __cdecl Demo_IsErrorDialogOpen(int localClientNum)
{
    return Menu_IsMenuOpenAndVisible(localClientNum, "demo_error");
}

