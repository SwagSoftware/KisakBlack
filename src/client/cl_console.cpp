#include <qcommon/common.h>
#include "screen_placement.h"
#include "cl_console.h"

#include "cl_main.h"
#include "client.h"
#include <cgame_mp/cg_newDraw_mp.h>
#include <client_mp/cl_cgame_mp.h>
#include "con_channels.h"
#include <stringed/stringed_hooks.h>
#include <win32/win_net.h>
#include <gfx_d3d/r_rendercmds.h>
#include <cgame/cg_sound.h>
#include <universal/com_buildinfo.h>
#include <qcommon/threads.h>
#include <sound/snd_public_async.h>
#include <universal/com_files.h>
#include <client_mp/cl_scrn_mp.h>
#include <cgame_mp/cg_consolecmds_mp.h>
#include <server_mp/sv_main_pc_mp.h>
#include "cl_keys.h"



Console con;
ConDrawInputGlob conDrawInputGlob;

field_t g_consoleField;

s_restricted_single g_restricted[1024];
int g_restricted_count;
e_restricted_initState g_restricted_state;
bool g_restricted_ranked;

int callDepth;

bool con_ignoreMatchPrefixOnly;

int g_console_field_width = 620;
float g_console_char_height = 16.0f;

char con_gameMsgWindowNMsgTime_Descs[3][69];
char con_gameMsgWindowNMsgTime_Names[3][26];
char con_gameMsgWindowNLineCount_Descs[3][73];
char con_gameMsgWindowNLineCount_Names[3][28];
char con_gameMsgWindowNScrollTime_Descs[3][84];
char con_gameMsgWindowNScrollTime_Names[3][29];
char con_gameMsgWindowNFadeInTime_Descs[3][54];
char con_gameMsgWindowNFadeInTime_Names[3][29];
char con_gameMsgWindowNFadeOutTime_Descs[3][55];
char con_gameMsgWindowNFadeOutTime_Names[3][30];
char con_gameMsgWindowNSplitscreenScale_Descs[3][48];
char con_gameMsgWindowNSplitscreenScale_Names[3][35];

const dvar_t *con_inputBoxColor;
const dvar_t *con_inputHintBoxColor;
const dvar_t *con_outputBarColor;
const dvar_t *con_outputSliderColor;
const dvar_t *con_outputWindowColor;

const dvar_t *con_gameMsgWindowNMsgTime[3];
const dvar_t *con_gameMsgWindowNLineCount[3];
const dvar_t *con_gameMsgWindowNScrollTime[3];
const dvar_t *con_gameMsgWindowNFadeInTime[3];
const dvar_t *con_gameMsgWindowNFadeOutTime[3];
const dvar_t *con_gameMsgWindowNSplitscreenScale[3];

const dvar_t *con_errormessagetime;
const dvar_t *con_minicontime;
const dvar_t *con_miniconlines;

const dvar_t *con_typewriterEnabledSounds;
const dvar_t *con_typewriterPrintSpeed;
const dvar_t *con_typewriterDecayStartTime;
const dvar_t *con_typewriterDecayDuration;

const dvar_t *con_typewriterColorBase;
const dvar_t *con_typewriterColorGlowUpdated;
const dvar_t *con_typewriterColorGlowCompleted;
const dvar_t *con_typewriterColorGlowFailed;
const dvar_t *con_typewriterColorGlowCheckpoint;

const dvar_t *con_restricted;
const dvar_t *con_restricted_access;
const dvar_t *con_matchPrefixOnly;

field_t historyEditLines[32];

void __cdecl Con_ToggleConsole()
{
    int localClientNum; // [esp+0h] [ebp-4h]

    Con_Restricted_ResetLists();
    Field_Clear(&g_consoleField);
    Con_CancelAutoComplete();
    g_consoleField.widthInPixels = g_console_field_width;
    g_consoleField.charHeight = g_console_char_height;
    g_consoleField.fixedSize = 1;
    con.outputVisible = 0;
    for (localClientNum = 0; localClientNum < 1; ++localClientNum)
        clientUIActives[localClientNum].keyCatchers ^= 1u;
}

void __cdecl Con_GetTextCopy(char *text, int maxSize)
{
    unsigned int end; // [esp+0h] [ebp-Ch]
    int begin; // [esp+4h] [ebp-8h]
    int totalSize; // [esp+8h] [ebp-4h]

    if ( con.consoleWindow.activeLineCount )
    {
        begin = con.consoleWindow.lines[con.consoleWindow.firstLineIndex].textBufPos;
        end = con.consoleWindow.textBufPos;
        totalSize = con.consoleWindow.textBufPos - begin;
        if ( con.consoleWindow.textBufPos - begin < 0 )
            totalSize += con.consoleWindow.textBufSize;
        if ( totalSize > maxSize - 1 )
        {
            begin += totalSize - (maxSize - 1);
            if ( begin > con.consoleWindow.textBufSize )
                begin -= con.consoleWindow.textBufSize;
            totalSize = maxSize - 1;
        }
        if ( begin >= con.consoleWindow.textBufPos )
        {
            memcpy(
                (unsigned __int8 *)text,
                (unsigned __int8 *)&con.consoleWindow.circularTextBuffer[begin],
                con.consoleWindow.textBufSize - begin);
            memcpy(
                (unsigned __int8 *)&text[con.consoleWindow.textBufSize - begin],
                (unsigned __int8 *)con.consoleWindow.circularTextBuffer,
                end);
        }
        else
        {
            memcpy(
                (unsigned __int8 *)text,
                (unsigned __int8 *)&con.consoleWindow.circularTextBuffer[begin],
                con.consoleWindow.textBufPos - begin);
        }
        text[totalSize] = 0;
    }
    else
    {
        *text = 0;
    }
}

void __cdecl Con_TimeJumped(int localClientNum, int serverTime)
{
    unsigned int gameWindowIndex; // [esp+0h] [ebp-4h]

    Con_ResetMessageWindowTimes(&con.consoleWindow, serverTime);
    for (gameWindowIndex = 0; gameWindowIndex < 3; ++gameWindowIndex)
        Con_ResetMessageWindowTimes(&con.messageBuffer[localClientNum].gamemsgWindows[gameWindowIndex], serverTime);
    Con_ResetMessageWindowTimes(&con.messageBuffer[localClientNum].miniconWindow, serverTime);
    Con_ResetMessageWindowTimes(&con.messageBuffer[localClientNum].errorWindow, serverTime);
}

void __cdecl Con_ResetMessageWindowTimes(MessageWindow *msgwnd, int serverTime)
{
    int duration; // [esp+0h] [ebp-14h]
    Message *message; // [esp+4h] [ebp-10h]
    int lineOffset; // [esp+8h] [ebp-Ch]
    MessageLine *line; // [esp+Ch] [ebp-8h]
    unsigned int lineIndex; // [esp+10h] [ebp-4h]

    for ( lineOffset = 0; lineOffset < msgwnd->activeLineCount; ++lineOffset )
    {
        if ( msgwnd->lineCount <= 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        504,
                        0,
                        "%s\n\t(msgwnd->lineCount) = %i",
                        "(msgwnd->lineCount > 0)",
                        msgwnd->lineCount) )
        {
            __debugbreak();
        }
        lineIndex = (lineOffset + msgwnd->firstLineIndex) % msgwnd->lineCount;
        if ( lineIndex >= msgwnd->lineCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        507,
                        0,
                        "lineIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                        lineIndex,
                        msgwnd->lineCount) )
        {
            __debugbreak();
        }
        line = &msgwnd->lines[lineIndex];
        if ( line->messageIndex >= (unsigned int)msgwnd->lineCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        510,
                        0,
                        "line->messageIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                        line->messageIndex,
                        msgwnd->lineCount) )
        {
            __debugbreak();
        }
        message = &msgwnd->messages[line->messageIndex];
        duration = message->endTime - message->startTime;
        message->startTime = serverTime;
        message->endTime = duration + serverTime;
    }
}

void __cdecl Con_TimeNudged(int localClientNum, int serverTimeNudge)
{
    unsigned int gameWindowIndex; // [esp+0h] [ebp-8h]
    int serverTime; // [esp+4h] [ebp-4h]

    serverTime = CL_GetLocalClientGlobals(localClientNum)->serverTime;
    Con_NudgeMessageWindowTimes(&con.consoleWindow, serverTimeNudge, serverTime);
    for (gameWindowIndex = 0; gameWindowIndex < 3; ++gameWindowIndex)
        Con_NudgeMessageWindowTimes(
            &con.messageBuffer[localClientNum].gamemsgWindows[gameWindowIndex],
            serverTimeNudge,
            serverTime);
    Con_NudgeMessageWindowTimes(&con.messageBuffer[localClientNum].miniconWindow, serverTimeNudge, serverTime);
    Con_NudgeMessageWindowTimes(&con.messageBuffer[localClientNum].errorWindow, serverTimeNudge, serverTime);
}

void __cdecl Con_NudgeMessageWindowTimes(MessageWindow *msgwnd, int serverTimeNudge, int serverTime)
{
    int duration; // [esp+0h] [ebp-18h]
    Message *message; // [esp+4h] [ebp-14h]
    int lineOffset; // [esp+8h] [ebp-10h]
    int lastMessageIndex; // [esp+Ch] [ebp-Ch]
    MessageLine *line; // [esp+10h] [ebp-8h]
    unsigned int lineIndex; // [esp+14h] [ebp-4h]

    lastMessageIndex = -1;
    for ( lineOffset = 0; lineOffset < msgwnd->activeLineCount; ++lineOffset )
    {
        if ( msgwnd->lineCount <= 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        545,
                        0,
                        "%s\n\t(msgwnd->lineCount) = %i",
                        "(msgwnd->lineCount > 0)",
                        msgwnd->lineCount) )
        {
            __debugbreak();
        }
        lineIndex = (lineOffset + msgwnd->firstLineIndex) % msgwnd->lineCount;
        if ( lineIndex >= msgwnd->lineCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        548,
                        0,
                        "lineIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                        lineIndex,
                        msgwnd->lineCount) )
        {
            __debugbreak();
        }
        line = &msgwnd->lines[lineIndex];
        if ( line->messageIndex >= (unsigned int)msgwnd->lineCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        551,
                        0,
                        "line->messageIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                        line->messageIndex,
                        msgwnd->lineCount) )
        {
            __debugbreak();
        }
        if ( line->messageIndex != lastMessageIndex )
        {
            lastMessageIndex = line->messageIndex;
            message = &msgwnd->messages[line->messageIndex];
            message->startTime += serverTimeNudge;
            message->endTime += serverTimeNudge;
            if ( message->startTime < 0 )
            {
                message->endTime -= message->startTime;
                message->startTime = 0;
            }
            if ( message->startTime > serverTime + 1000 )
            {
                duration = message->endTime - message->startTime;
                message->startTime = serverTime + 1000;
                message->endTime = duration + message->startTime;
            }
        }
    }
}

void __cdecl Con_ClearNotify(int localClientNum)
{
    unsigned int gameWindowIndex; // [esp+0h] [ebp-4h]

    for ( gameWindowIndex = 0; gameWindowIndex < 3; ++gameWindowIndex )
        Con_ClearMessageWindow(&con.messageBuffer[localClientNum].gamemsgWindows[gameWindowIndex]);
}

void __cdecl Con_ClearMessageWindow(MessageWindow *msgwnd)
{
    if ( !msgwnd && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 594, 0, "%s", "msgwnd") )
        __debugbreak();
    if ( !msgwnd->messages
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 595, 0, "%s", "msgwnd->messages") )
    {
        __debugbreak();
    }
    if ( !msgwnd->lines
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 596, 0, "%s", "msgwnd->lines") )
    {
        __debugbreak();
    }
    msgwnd->textBufPos = 0;
    msgwnd->messageIndex = 0;
    msgwnd->firstLineIndex = 0;
    msgwnd->activeLineCount = 0;
}

void __cdecl Con_ClearErrors(int localClientNum)
{
    Con_ClearMessageWindow(&con.messageBuffer[localClientNum].errorWindow);
}

void __cdecl Con_CheckResize()
{
    float v0; // [esp+8h] [ebp-20h]
    float v1; // [esp+Ch] [ebp-1Ch]
    float v2; // [esp+10h] [ebp-18h]
    float v3; // [esp+14h] [ebp-14h]
    float v4; // [esp+18h] [ebp-10h]
    float v5; // [esp+1Ch] [ebp-Ch]
    float v6; // [esp+20h] [ebp-8h]
    float v7; // [esp+24h] [ebp-4h]

    v7 = ScrPlace_ApplyX(&scrPlaceFull, 4.0, 1);
    v3 = floor(v7);
    con.screenMin[0] = v3;
    v6 = ScrPlace_ApplyY(&scrPlaceFull, 4.0, 1);
    v2 = floor(v6);
    con.screenMin[1] = v2;
    //v5 = ScrPlace_ApplyX(&scrPlaceFull, COERCE_FLOAT(LODWORD(con_screenPadding) ^ _mask__NegFloat_), 3);
    v5 = ScrPlace_ApplyX(&scrPlaceFull, -con_screenPadding, 3);
    v1 = floor(v5);
    con.screenMax[0] = v1;
    //v4 = ScrPlace_ApplyY(&scrPlaceFull, COERCE_FLOAT(LODWORD(con_screenPadding) ^ _mask__NegFloat_), 3);
    v4 = ScrPlace_ApplyY(&scrPlaceFull, -con_screenPadding, 3);
    v0 = floor(v4);
    con.screenMax[1] = v0;
    if ( cls.consoleFont )
    {
        con.fontHeight = R_TextHeight(cls.consoleFont);
        if ( con.fontHeight <= 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        638,
                        0,
                        "%s",
                        "con.fontHeight > 0") )
        {
            __debugbreak();
        }
        con.visibleLineCount = (int)(float)((float)((float)(con.screenMax[1] - con.screenMin[1])
                                                                                            - (float)(2 * con.fontHeight))
                                                                            - (float)(6.0 * 4.0))
                                                 / con.fontHeight;
        con.visiblePixelWidth = (int)(float)((float)((float)(con.screenMax[0] - con.screenMin[0]) - 10.0)
                                                                             - (float)(6.0 * 3.0));
    }
    else
    {
        con.fontHeight = 0;
        con.visibleLineCount = 0;
        con.visiblePixelWidth = 0;
    }
}

void __cdecl Con_OneTimeInit()
{
    float v0; // [esp+40h] [ebp-B4h]
    float v1; // [esp+64h] [ebp-90h]
    unsigned int gameWindowIndex; // [esp+ECh] [ebp-8h]
    char *dvarDesc; // [esp+F0h] [ebp-4h]
    char *dvarDesca; // [esp+F0h] [ebp-4h]
    char *dvarDescb; // [esp+F0h] [ebp-4h]
    char *dvarDescc; // [esp+F0h] [ebp-4h]
    char *dvarDescd; // [esp+F0h] [ebp-4h]
    char *dvarDesce; // [esp+F0h] [ebp-4h]

    con_inputBoxColor = _Dvar_RegisterVec4(
                                                "con_inputBoxColor",
                                                (0.25),
                                                (0.25),
                                                (0.2),
                                                (1.0),
                                                0.0,
                                                1.0,
                                                1u,
                                                "Color of the console input box");
    con_inputHintBoxColor = _Dvar_RegisterVec4(
                                                        "con_inputHintBoxColor",
                                                        (0.40000001),
                                                        (0.40000001),
                                                        (0.34999999),
                                                        (1.0),
                                                        0.0,
                                                        1.0,
                                                        1u,
                                                        "Color of the console input hint box");
    con_outputBarColor = _Dvar_RegisterVec4(
                                                 "con_outputBarColor",
                                                 (1.0),
                                                 (1.0),
                                                 (0.94999999),
                                                 (0.60000002),
                                                 0.0,
                                                 1.0,
                                                 1u,
                                                 "Color of the console output slider bar");
    con_outputSliderColor = _Dvar_RegisterVec4(
                                                        "con_outputSliderColor",
                                                        (0.15000001),
                                                        (0.15000001),
                                                        (0.1),
                                                        (0.60000002),
                                                        0.0,
                                                        1.0,
                                                        1u,
                                                        "Color of the console slider");
    con_outputWindowColor = _Dvar_RegisterVec4(
                                                        "con_outputWindowColor",
                                                        (0.34999999),
                                                        (0.34999999),
                                                        (0.30000001),
                                                        (0.75),
                                                        0.0,
                                                        1.0,
                                                        1u,
                                                        "Color of the console output");
    for ( gameWindowIndex = 0; gameWindowIndex < 3; ++gameWindowIndex )
    {
        dvarDesc = con_gameMsgWindowNMsgTime_Descs[gameWindowIndex];
        sprintf(con_gameMsgWindowNMsgTime_Names[gameWindowIndex], "con_gameMsgWindow%dMsgTime", gameWindowIndex);
        if ( strlen(con_gameMsgWindowNMsgTime_Names[gameWindowIndex]) != 25
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        715,
                        0,
                        "%s",
                        "strlen( dvarName ) == sizeof( con_gameMsgWindowNMsgTime_Names[gameWindowIndex] ) - 1") )
        {
            __debugbreak();
        }
        sprintf(dvarDesc, "On screen time for game messages in seconds in game message window %d", gameWindowIndex);
        if ( strlen(con_gameMsgWindowNMsgTime_Descs[gameWindowIndex]) != 68
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        715,
                        0,
                        "%s",
                        "strlen( dvarDesc ) == sizeof( con_gameMsgWindowNMsgTime_Descs[gameWindowIndex] ) - 1") )
        {
            __debugbreak();
        }
        con_gameMsgWindowNMsgTime[gameWindowIndex] = _Dvar_RegisterFloat(
                                                                                                     con_gameMsgWindowNMsgTime_Names[gameWindowIndex],
                                                                                                     defaultGameMessageTimes[gameWindowIndex],
                                                                                                     0.0,
                                                                                                     3.4028235e38,
                                                                                                     1u,
                                                                                                     dvarDesc);
        dvarDesca = con_gameMsgWindowNLineCount_Descs[gameWindowIndex];
        sprintf(con_gameMsgWindowNLineCount_Names[gameWindowIndex], "con_gameMsgWindow%dLineCount", gameWindowIndex);
        if ( strlen(con_gameMsgWindowNLineCount_Names[gameWindowIndex]) != 27
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        718,
                        0,
                        "%s",
                        "strlen( dvarName ) == sizeof( con_gameMsgWindowNLineCount_Names[gameWindowIndex] ) - 1") )
        {
            __debugbreak();
        }
        sprintf(dvarDesca, "Maximum number of lines of text visible at once in game message window %d", gameWindowIndex);
        if ( strlen(con_gameMsgWindowNLineCount_Descs[gameWindowIndex]) != 72
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        718,
                        0,
                        "%s",
                        "strlen( dvarDesc ) == sizeof( con_gameMsgWindowNLineCount_Descs[gameWindowIndex] ) - 1") )
        {
            __debugbreak();
        }
        con_gameMsgWindowNLineCount[gameWindowIndex] = _Dvar_RegisterInt(
                                                                                                         con_gameMsgWindowNLineCount_Names[gameWindowIndex],
                                                                                                         defaultGameMessageWindowLineCounts[gameWindowIndex],
                                                                                                         1,
                                                                                                         9,
                                                                                                         1u,
                                                                                                         dvarDesca);
        dvarDescb = con_gameMsgWindowNScrollTime_Descs[gameWindowIndex];
        sprintf(con_gameMsgWindowNScrollTime_Names[gameWindowIndex], "con_gameMsgWindow%dScrollTime", gameWindowIndex);
        if ( strlen(con_gameMsgWindowNScrollTime_Names[gameWindowIndex]) != 28
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        722,
                        0,
                        "%s",
                        "strlen( dvarName ) == sizeof( con_gameMsgWindowNScrollTime_Names[gameWindowIndex] ) - 1") )
        {
            __debugbreak();
        }
        sprintf(
            dvarDescb,
            "Time to scroll messages when the oldest message is removed in game message window %d",
            gameWindowIndex);
        if ( strlen(con_gameMsgWindowNScrollTime_Descs[gameWindowIndex]) != 83
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        722,
                        0,
                        "%s",
                        "strlen( dvarDesc ) == sizeof( con_gameMsgWindowNScrollTime_Descs[gameWindowIndex] ) - 1") )
        {
            __debugbreak();
        }
        con_gameMsgWindowNScrollTime[gameWindowIndex] = _Dvar_RegisterFloat(
                                                                                                            con_gameMsgWindowNScrollTime_Names[gameWindowIndex],
                                                                                                            0.25,
                                                                                                            0.0,
                                                                                                            3.4028235e38,
                                                                                                            1u,
                                                                                                            dvarDescb);
        dvarDescc = con_gameMsgWindowNFadeInTime_Descs[gameWindowIndex];
        sprintf(con_gameMsgWindowNFadeInTime_Names[gameWindowIndex], "con_gameMsgWindow%dFadeInTime", gameWindowIndex);
        if ( strlen(con_gameMsgWindowNFadeInTime_Names[gameWindowIndex]) != 28
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        725,
                        0,
                        "%s",
                        "strlen( dvarName ) == sizeof( con_gameMsgWindowNFadeInTime_Names[gameWindowIndex] ) - 1") )
        {
            __debugbreak();
        }
        sprintf(dvarDescc, "Time to fade in new messages in game message window %d", gameWindowIndex);
        if ( strlen(con_gameMsgWindowNFadeInTime_Descs[gameWindowIndex]) != 53
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        725,
                        0,
                        "%s",
                        "strlen( dvarDesc ) == sizeof( con_gameMsgWindowNFadeInTime_Descs[gameWindowIndex] ) - 1") )
        {
            __debugbreak();
        }
        if ( gameWindowIndex == 2 )
            v1 = 0.75f;
        else
            v1 = 0.25f;
        con_gameMsgWindowNFadeInTime[gameWindowIndex] = _Dvar_RegisterFloat(
                                                                                                            con_gameMsgWindowNFadeInTime_Names[gameWindowIndex],
                                                                                                            v1,
                                                                                                            0.0,
                                                                                                            3.4028235e38,
                                                                                                            1u,
                                                                                                            dvarDescc);
        dvarDescd = con_gameMsgWindowNFadeOutTime_Descs[gameWindowIndex];
        sprintf(con_gameMsgWindowNFadeOutTime_Names[gameWindowIndex], "con_gameMsgWindow%dFadeOutTime", gameWindowIndex);
        if ( strlen(con_gameMsgWindowNFadeOutTime_Names[gameWindowIndex]) != 29
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        728,
                        0,
                        "%s",
                        "strlen( dvarName ) == sizeof( con_gameMsgWindowNFadeOutTime_Names[gameWindowIndex] ) - 1") )
        {
            __debugbreak();
        }
        sprintf(dvarDescd, "Time to fade out old messages in game message window %d", gameWindowIndex);
        if ( strlen(con_gameMsgWindowNFadeOutTime_Descs[gameWindowIndex]) != 54
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        728,
                        0,
                        "%s",
                        "strlen( dvarDesc ) == sizeof( con_gameMsgWindowNFadeOutTime_Descs[gameWindowIndex] ) - 1") )
        {
            __debugbreak();
        }
        if ( gameWindowIndex == 1 )
            v0 = 0.01f;
        else
            v0 = 0.5f;
        con_gameMsgWindowNFadeOutTime[gameWindowIndex] = _Dvar_RegisterFloat(
                                                                                                             con_gameMsgWindowNFadeOutTime_Names[gameWindowIndex],
                                                                                                             v0,
                                                                                                             0.0099999998,
                                                                                                             3.4028235e38,
                                                                                                             1u,
                                                                                                             dvarDescd);
        dvarDesce = con_gameMsgWindowNSplitscreenScale_Descs[gameWindowIndex];
        sprintf(
            con_gameMsgWindowNSplitscreenScale_Names[gameWindowIndex],
            "con_gameMsgWindow%dSplitscreenScale",
            gameWindowIndex);
        if ( strlen(con_gameMsgWindowNSplitscreenScale_Names[gameWindowIndex]) != 34
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        731,
                        0,
                        "%s",
                        "strlen( dvarName ) == sizeof( con_gameMsgWindowNSplitscreenScale_Names[gameWindowIndex] ) - 1") )
        {
            __debugbreak();
        }
        sprintf(dvarDesce, "Scaling of game message window %d in splitscreen", gameWindowIndex);
        if ( strlen(con_gameMsgWindowNSplitscreenScale_Descs[gameWindowIndex]) != 47
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        731,
                        0,
                        "%s",
                        "strlen( dvarDesc ) == sizeof( con_gameMsgWindowNSplitscreenScale_Descs[gameWindowIndex] ) - 1") )
        {
            __debugbreak();
        }
        con_gameMsgWindowNSplitscreenScale[gameWindowIndex] = _Dvar_RegisterFloat(
                                                                                                                        con_gameMsgWindowNSplitscreenScale_Names[gameWindowIndex],
                                                                                                                        1.5,
                                                                                                                        0.0,
                                                                                                                        3.4028235e38,
                                                                                                                        1u,
                                                                                                                        dvarDesce);
    }
    con_errormessagetime = _Dvar_RegisterFloat(
                                                     "con_errormessagetime",
                                                     8.0,
                                                     0.0,
                                                     3.4028235e38,
                                                     1u,
                                                     "Onscreen time for error messages in seconds");
    con_minicontime = _Dvar_RegisterFloat(
                                            "con_minicontime",
                                            4.0,
                                            0.0,
                                            3.4028235e38,
                                            1u,
                                            "Onscreen time for minicon messages in seconds");
    con_miniconlines = _Dvar_RegisterInt(
                                             "con_miniconlines",
                                             5,
                                             0,
                                             100,
                                             1u,
                                             "Number of lines in the minicon message window");
    con_typewriterEnabledSounds = _Dvar_RegisterBool(
                                                                    "con_typewriterEnabledSounds",
                                                                    0,
                                                                    1u,
                                                                    "Enable the typewriter sounds and text effects.    Disabled still uses the typewrite glow settings.");
    con_typewriterPrintSpeed = _Dvar_RegisterInt(
                                                             "con_typewriterPrintSpeed",
                                                             40,
                                                             0,
                                                             0x7FFFFFFF,
                                                             1u,
                                                             "Time (in milliseconds) to print each letter in the line.");
    con_typewriterDecayStartTime = _Dvar_RegisterInt(
                                                                     "con_typewriterDecayStartTime",
                                                                     4000,
                                                                     0,
                                                                     0x7FFFFFFF,
                                                                     1u,
                                                                     "Time (in milliseconds) to spend between the build and disolve phases.");
    con_typewriterDecayDuration = _Dvar_RegisterInt(
                                                                    "con_typewriterDecayDuration",
                                                                    1000,
                                                                    0,
                                                                    0x7FFFFFFF,
                                                                    1u,
                                                                    "Time (in milliseconds) to spend disolving the line away.");
    con_typewriterColorBase = _Dvar_RegisterVec3(
                                                            "con_typewriterColorBase",
                                                            (1.0),
                                                            (1.0),
                                                            (1.0),
                                                            0.0,
                                                            1.0,
                                                            0x1000u,
                                                            "Base color of typewritten objective text.");
    con_typewriterColorGlowUpdated = _Dvar_RegisterVec4(
                                                                         "con_typewriterColorGlowUpdated",
                                                                         (0.0),
                                                                         (0.60000002),
                                                                         (0.18000001),
                                                                         (1.0),
                                                                         0.0,
                                                                         1.0,
                                                                         1u,
                                                                         "Color of typewritten objective text.");
    con_typewriterColorGlowCompleted = _Dvar_RegisterVec4(
                                                                             "con_typewriterColorGlowCompleted",
                                                                             (0.0),
                                                                             (0.30000001),
                                                                             (0.80000001),
                                                                             (1.0),
                                                                             0.0,
                                                                             1.0,
                                                                             1u,
                                                                             "Color of typewritten objective text.");
    con_typewriterColorGlowFailed = _Dvar_RegisterVec4(
                                                                        "con_typewriterColorGlowFailed",
                                                                        (0.80000001),
                                                                        (0.0),
                                                                        (0.0),
                                                                        (1.0),
                                                                        0.0,
                                                                        1.0,
                                                                        1u,
                                                                        "Color of typewritten objective text.");
    con_typewriterColorGlowCheckpoint = _Dvar_RegisterVec4(
                                                                                "con_typewriterColorGlowCheckpoint",
                                                                                (0.60000002),
                                                                                (0.5),
                                                                                (0.60000002),
                                                                                (1.0),
                                                                                0.0,
                                                                                1.0,
                                                                                1u,
                                                                                "Color of typewritten objective text.");
    Con_InitMessageWindow(
        &con.consoleWindow,
        con.consoleMessages,
        con.consoleLines,
        con.consoleText,
        1024,
        0,
        0x8000,
        0,
        1,
        1);
    Con_InitMessageBuffer();
    con.color[0] = colorWhite[0];
    con.color[1] = 1.0;
    con.color[2] = 1.0;
    con.color[3] = 1.0;
    Con_CheckResize();
    con.initialized = 1;
}

void __cdecl Con_InitMessageWindow(
                MessageWindow *msgwnd,
                Message *messages,
                MessageLine *lines,
                char *text,
                int lineCount,
                int padding,
                int textPoolSize,
                int scrollTime,
                int fadeIn,
                int fadeOut)
{
    if ( !msgwnd && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 653, 0, "%s", "msgwnd") )
        __debugbreak();
    if ( !lines && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 654, 0, "%s", "lines") )
        __debugbreak();
    if ( lineCount < padding
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    655,
                    0,
                    "lineCount >= padding\n\t%i, %i",
                    lineCount,
                    padding) )
    {
        __debugbreak();
    }
    msgwnd->lines = lines;
    msgwnd->messages = messages;
    msgwnd->circularTextBuffer = text;
    msgwnd->textBufPos = 0;
    msgwnd->textBufSize = textPoolSize;
    msgwnd->firstLineIndex = 0;
    msgwnd->activeLineCount = 0;
    msgwnd->messageIndex = 0;
    msgwnd->lineCount = lineCount;
    msgwnd->padding = padding;
    msgwnd->scrollTime = scrollTime;
    if ( fadeOut <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    673,
                    0,
                    "%s\n\t(fadeOut) = %i",
                    "(fadeOut > 0)",
                    fadeOut) )
    {
        __debugbreak();
    }
    msgwnd->fadeIn = fadeIn;
    msgwnd->fadeOut = fadeOut;
}

cmd_function_s Con_ChatModePublic_f_VAR;
cmd_function_s Con_ChatModeTeam_f_VAR;
cmd_function_s Con_Clear_f_VAR;
cmd_function_s Con_Echo_f_VAR;

void __cdecl Con_Init()
{
    int i; // [esp+0h] [ebp-4h]

    con_restricted = _Dvar_RegisterBool("monkeytoy", 0, 1u, "Restrict console access");
    con_restricted_access = _Dvar_RegisterBool(
                                                        "con_restricted_access",
                                                        0,
                                                        0,
                                                        "Restricted console access to dvars and commands");
    con_matchPrefixOnly = _Dvar_RegisterBool(
                                                    "con_matchPrefixOnly",
                                                    1,
                                                    1u,
                                                    "Only match the prefix when listing matching Dvars");
    Field_Clear(&g_consoleField);
    g_consoleField.widthInPixels = g_console_field_width;
    g_consoleField.charHeight = g_console_char_height;
    g_consoleField.fixedSize = 1;
    for ( i = 0; i < 32; ++i )
    {
        Field_Clear(&historyEditLines[i]);
        historyEditLines[i].widthInPixels = g_console_field_width;
        historyEditLines[i].charHeight = g_console_char_height;
        historyEditLines[i].fixedSize = 1;
    }
    conDrawInputGlob.matchIndex = -1;
    Cmd_AddCommandInternal("chatmodepublic", Con_ChatModePublic_f, &Con_ChatModePublic_f_VAR);
    Cmd_AddCommandInternal("chatmodeteam", Con_ChatModeTeam_f, &Con_ChatModeTeam_f_VAR);
    Cmd_AddCommandInternal("clear", Con_Clear_f, &Con_Clear_f_VAR);
    Cmd_AddCommandInternal("con_echo", Con_Echo_f, &Con_Echo_f_VAR);
    if ( !con.initialized )
    {
        Con_OneTimeInit();
        if ( !con.initialized
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 805, 0, "%s", "con.initialized") )
        {
            __debugbreak();
        }
    }
}

void __cdecl Con_ChatModePublic_f()
{
    SetupChatField(0, 0, 588);
}

void __cdecl SetupChatField(int localClientNum, int teamChat, int widthInPixels)
{
    clientUIActive_t *LocalClientUIGlobals; // [esp+0h] [ebp-14h]
    PlayerKeyState *chatField; // [esp+4h] [ebp-10h]
    int width; // [esp+8h] [ebp-Ch] BYREF
    int height; // [esp+Ch] [ebp-8h] BYREF
    float aspect; // [esp+10h] [ebp-4h] BYREF

    CL_GetScreenDimensions(&width, &height, &aspect);
    playerKeys[localClientNum].chat_team = teamChat;
    chatField = &playerKeys[localClientNum];
    Field_Clear(&chatField->chatField);
    chatField->chatField.widthInPixels = widthInPixels;
    chatField->chatField.fixedSize = 0;
    if ( height <= 768 )
        chatField->chatField.charHeight = 16.0f;
    else
        chatField->chatField.charHeight = 10.0f;
    LocalClientUIGlobals = CL_GetLocalClientUIGlobals(localClientNum);
    LocalClientUIGlobals->keyCatchers ^= 0x20u;
}

void __cdecl Con_ChatModeTeam_f()
{
    SetupChatField(0, 1, 543);
}

void __cdecl Con_Echo_f()
{
    Con_ToggleConsole();
    I_strncpyz(g_consoleField.buffer, "\\echo ", 256);
    g_consoleField.cursor = strlen(g_consoleField.buffer);
    Field_AdjustScroll(&scrPlaceFull, &g_consoleField);
}

void __cdecl Con_Clear_f()
{
    Con_ClearMessageWindow(&con.consoleWindow);
    con.lineOffset = 0;
    con.displayLineOffset = 0;
}

void __cdecl Con_InitClientAssets()
{
    Con_CheckResize();
}

void __cdecl Con_InitMessageBuffer()
{
    int localClientNum; // [esp+38h] [ebp-14h]
    MessageBuffer *msgBuf; // [esp+40h] [ebp-Ch]
    unsigned int gameWindowIndex; // [esp+48h] [ebp-4h]

    for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
    {
        msgBuf = &con.messageBuffer[localClientNum];
        for ( gameWindowIndex = 0; gameWindowIndex < 3; ++gameWindowIndex )
            Con_InitMessageWindow(
                &msgBuf->gamemsgWindows[gameWindowIndex],
                msgBuf->gamemsgMessages[gameWindowIndex],
                msgBuf->gamemsgLines[gameWindowIndex],
                msgBuf->gamemsgText[gameWindowIndex],
                con_gameMsgWindowNLineCount[gameWindowIndex]->current.integer + 3,
                3,
                2048,
                (int)((float)(con_gameMsgWindowNScrollTime[gameWindowIndex]->current.value * 1000.0) + 9.313225746154785e-10),
                (int)((float)(con_gameMsgWindowNFadeInTime[gameWindowIndex]->current.value * 1000.0) + 9.313225746154785e-10),
                (int)((float)(con_gameMsgWindowNFadeOutTime[gameWindowIndex]->current.value * 1000.0) + 9.313225746154785e-10));
        Con_InitMessageWindow(
            &msgBuf->miniconWindow,
            msgBuf->miniconMessages,
            msgBuf->miniconLines,
            msgBuf->miniconText,
            con_miniconlines->current.integer,
            0,
            4096,
            0,
            0,
            1);
        Con_InitMessageWindow(
            &msgBuf->errorWindow,
            msgBuf->errorMessages,
            msgBuf->errorLines,
            msgBuf->errorText,
            5,
            0,
            1024,
            0,
            1,
            1);
    }
}

void __cdecl CL_ConsolePrint(int localClientNum, int channel, const char *txt, int duration, int pixelWidth, int flags)
{
    iassert(txt);

    if ( cl_noprint && !cl_noprint->current.enabled && channel != 6 )
    {
        if ( !con.initialized )
        {
            Con_OneTimeInit();

            iassert(con.initialized);
        }
        Con_IsChannelVisible(CON_DEST_CONSOLE, channel, flags);
        Sys_EnterCriticalSection(CRITSECT_CONSOLE);
        CL_ConsolePrint_AddLine(localClientNum, channel, txt, duration, pixelWidth, 55, flags);
        Sys_LeaveCriticalSection(CRITSECT_CONSOLE);
    }
}

void CL_ConsolePrint_AddLine(
    int localClientNum,
    int channel,
    const char *txt,
    int duration,
    int pixelWidth,
    char color,
    int flags)
{
    char *v8; // eax
    char *v9; // ecx
    const char *v10; // edx
    int c; // [esp+24h] [ebp-24h]
    int atStartOfBrokenLine; // [esp+28h] [ebp-20h]
    Font_s *font; // [esp+2Ch] [ebp-1Ch]
    float xScale; // [esp+30h] [ebp-18h]
    const char *wrapPosition; // [esp+3Ch] [ebp-Ch]
    const char *text; // [esp+44h] [ebp-4h] BYREF

    iassert(txt);
    rangeassert(color, COLOR_FIRST, COLOR_LAST);

    if (!callDepth)
    {
        callDepth = 1;
        Con_UpdateNotifyMessage(localClientNum, channel, duration, flags);
        if (channel != con.prevChannel && con.lineOffset)
            Con_Linefeed(localClientNum, con.prevChannel, flags);
        if (channel == 2 || channel == 3 || channel == 4)
        {
            font = UI_GetFontHandle(&scrPlaceView[localClientNum], channel != 3 ? 0 : 4, 12.0 / 48.0);
            xScale = R_NormalizedTextScale(font, 12.0 / 48.0);
        }
        else
        {
            font = cls.consoleFont;
            xScale = 1.0f;
        }
        if (!pixelWidth)
            pixelWidth = con.visiblePixelWidth;
        wrapPosition = CL_TextLineWrapPosition(txt, 512 - con.lineOffset, pixelWidth, font, xScale, 0);
        if (txt == wrapPosition && con.lineOffset)
        {
            Con_Linefeed(localClientNum, channel, flags);
            wrapPosition = CL_TextLineWrapPosition(txt, 512 - con.lineOffset, pixelWidth, font, xScale, 0);
        }
        text = txt;
        atStartOfBrokenLine = 0;
        while (1)
        {
            if (!*text)
            {
                if (con.lineOffset)
                {
                    if (channel == 2 || channel == 3 || channel == 4)
                        Con_Linefeed(localClientNum, channel, flags);
                    else
                        Con_UpdateNotifyLine(localClientNum, channel, 0, flags);
                }
                con.prevChannel = channel;
                --callDepth;
                return;
            }
            c = SEH_ReadCharFromString(&text, 0);
            if (c == 10)
            {
                if (wrapPosition)
                {
                    if (font)
                    {
                        if (wrapPosition != text)
                        {
                            if (*text)
                            {
                                v8 = va("font is %s, wrapPosition is %s, text is %s, txt is %s", "valid", wrapPosition, text, txt);
                                if (!Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                                    1323,
                                    0,
                                    "%s\n\t%s",
                                    "!font || wrapPosition == text || !text[0]",
                                    v8))
                                    __debugbreak();
                            }
                        }
                    }
                }
                else
                {
                    wrapPosition = text;
                }
            }
            else
            {
                if (c != 94)
                    goto LABEL_63;
                if (text && *text != 94 && *text >= 48 && *text <= 64 || text && *text == 70)
                {
                    bcassert(con.lineOffset, sizeof(con.textTempLine));
                    color = *text;
                    con.textTempLine[con.lineOffset++] = 94;
                    con.textTempLine[con.lineOffset++] = color;
                    bcassert(con.lineOffset, sizeof(con.textTempLine));
                    ++text;
                    atStartOfBrokenLine = 0;
                }
                else
                {
                    if (*text != 1 && *text != 2)
                    {
                    LABEL_63:
                        if (c != 32 || !atStartOfBrokenLine)
                        {
                            if (c > 255)
                            {
                                bcassert(con.lineOffset, sizeof(con.textTempLine));
                                con.textTempLine[con.lineOffset++] = BYTE1(c);
                                c = (unsigned __int8)c;

                                bcassert(con.lineOffset, sizeof(con.textTempLine));
                            }

                            bcassert(con.lineOffset, sizeof(con.textTempLine));
                            con.textTempLine[con.lineOffset++] = c;
                            bcassert(con.lineOffset, sizeof(con.textTempLine));
                            atStartOfBrokenLine = 0;
                        }
                        goto LABEL_79;
                    }
                    bcassert(con.lineOffset, sizeof(con.textTempLine));
                    if (!IsValidMaterialHandle(*(Material *const *)(text + 3))
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                            1343,
                            0,
                            "%s",
                            "IsValidMaterialHandle( *reinterpret_cast< const MaterialHandle * >( &text[CONTXTCMD_ARG_HUDICON_MATERIAL] ) )"))
                    {
                        __debugbreak();
                    }
                    con.textTempLine[con.lineOffset++] = 94;
                    v9 = &con.textTempLine[con.lineOffset];
                    v10 = text;
                    *(_DWORD *)v9 = *(_DWORD *)text;
                    *((_WORD *)v9 + 2) = *((_WORD *)v10 + 2);
                    v9[6] = v10[6];
                    con.lineOffset += 7;
                    text += 7;

                    bcassert(con.lineOffset, sizeof(con.textTempLine));
                    atStartOfBrokenLine = 0;
                }
            }
        LABEL_79:
            if (text == wrapPosition)
            {
                Con_Linefeed(localClientNum, channel, flags);
                if (c != 10)
                {
                    atStartOfBrokenLine = 1;
                    if (color != 55)
                    {
                        rangeassert(color, COLOR_FIRST, COLOR_LAST);
                        bcassert(con.lineOffset, sizeof(con.textTempLine));
                        
                        con.textTempLine[con.lineOffset] = '^';
                        con.textTempLine[con.lineOffset + 1] = color;
                        con.lineOffset += 2;

                        bcassert(con.lineOffset, sizeof(con.textTempLine));
                    }
                }
                wrapPosition = CL_TextLineWrapPosition(
                    text,
                    512 - con.lineOffset,
                    pixelWidth,
                    font,
                    xScale,
                    atStartOfBrokenLine);
            }
        }
    }
}

void __cdecl Con_UpdateNotifyMessage(int localClientNum, unsigned int channel, int duration, int flags)
{
    print_msg_dest_t dest; // [esp+0h] [ebp-4h]

    if ( !Con_IsChannelOpen(channel)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    1150,
                    0,
                    "%s",
                    "Con_IsChannelOpen(channel)") )
    {
        __debugbreak();
    }
    Con_UpdateNotifyMessageWindow(localClientNum, channel, duration, flags, CON_DEST_MINICON);
    for ( dest = CON_DEST_GAME_FIRST; (unsigned int)dest <= CON_DEST_GAME3; ++dest )
        Con_UpdateNotifyMessageWindow(localClientNum, channel, duration, flags, dest);
    if ( !com_developer
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 1158, 0, "%s", "com_developer") )
    {
        __debugbreak();
    }
    if ( com_developer->current.integer )
        Con_UpdateNotifyMessageWindow(localClientNum, channel, duration, flags, CON_DEST_ERROR);
}

void __cdecl Con_UpdateNotifyMessageWindow(
                int localClientNum,
                unsigned int channel,
                int duration,
                int flags,
                print_msg_dest_t dest)
{
    MessageWindow *DestWindow; // eax

    if ( Con_IsChannelVisible(dest, channel, flags) )
    {
        if ( !duration )
            duration = Con_GetDefaultMsgDuration(dest);
        if ( duration < 0 )
            duration = 0;
        DestWindow = Con_GetDestWindow(localClientNum, dest);
        Con_UpdateMessage(localClientNum, DestWindow, duration);
    }
}

int __cdecl Con_GetDefaultMsgDuration(print_msg_dest_t dest)
{
    float time;
    if (dest == CON_DEST_MINICON)
        time = con_minicontime->current.value * 1000.0f;
    else if (dest == CON_DEST_ERROR)
        time = con_errormessagetime->current.value * 1000.0f;
    else
    {
        iassert(dest >= CON_DEST_GAME_FIRST && dest <= CON_DEST_GAME_LAST);
        time = con_gameMsgWindowNMsgTime[dest - CON_DEST_GAME_FIRST]->current.value * 1000.0f;
    }

    return static_cast<int>(time + 9.313225746154785e-10);
}

void __cdecl Con_UpdateMessage(int localClientNum, MessageWindow *msgwnd, int duration)
{
    Message *message; // [esp+0h] [ebp-4h]

    if ( !msgwnd && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 849, 0, "%s", "msgwnd") )
        __debugbreak();
    if ( msgwnd->messageIndex >= (unsigned int)msgwnd->lineCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    850,
                    0,
                    "msgwnd->messageIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                    msgwnd->messageIndex,
                    msgwnd->lineCount) )
    {
        __debugbreak();
    }
    if ( !msgwnd->lineCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    853,
                    0,
                    "%s",
                    "msgwnd->lineCount != 0") )
    {
        __debugbreak();
    }
    msgwnd->messageIndex = (msgwnd->messageIndex + 1) % msgwnd->lineCount;
    message = &msgwnd->messages[msgwnd->messageIndex];
    message->startTime = 0;
    if ( CL_GetLocalClientGlobals(localClientNum) )
        message->startTime = CL_GetLocalClientGlobals(localClientNum)->serverTime;
    message->endTime = duration + message->startTime;
}

MessageWindow *__cdecl Con_GetDestWindow(int localClientNum, print_msg_dest_t dest)
{
    switch (dest)
    {
    case CON_DEST_CONSOLE:
        return &con.consoleWindow;
    case CON_DEST_MINICON:
        return &con.messageBuffer[localClientNum].miniconWindow;
    case CON_DEST_ERROR:
        return &con.messageBuffer[localClientNum].errorWindow;
    }
    if ((dest < CON_DEST_GAME_FIRST || dest > CON_DEST_GAME3)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
            1125,
            0,
            "%s",
            "dest >= CON_DEST_GAME_FIRST && dest <= CON_DEST_GAME_LAST"))
    {
        __debugbreak();
    }
    return (MessageWindow *)&con.messageBuffer[localClientNum].gamemsgText[2][52 * dest + 1892];
}

void __cdecl Con_UpdateNotifyLine(int localClientNum, unsigned int channel, bool lineFeed, int flags)
{
    print_msg_dest_t dest; // [esp+0h] [ebp-4h]

    if ( !Con_IsChannelOpen(channel)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    1177,
                    0,
                    "%s",
                    "Con_IsChannelOpen(channel)") )
    {
        __debugbreak();
    }
    Con_UpdateNotifyLineWindow(localClientNum, channel, lineFeed, flags, CON_DEST_CONSOLE);
    Con_UpdateNotifyLineWindow(localClientNum, channel, lineFeed, flags, CON_DEST_MINICON);
    for ( dest = CON_DEST_GAME_FIRST; (unsigned int)dest <= CON_DEST_GAME3; ++dest )
        Con_UpdateNotifyLineWindow(localClientNum, channel, lineFeed, flags, dest);
    if ( !com_developer
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 1186, 0, "%s", "com_developer") )
    {
        __debugbreak();
    }
    if ( com_developer->current.integer )
        Con_UpdateNotifyLineWindow(localClientNum, channel, lineFeed, flags, CON_DEST_ERROR);
}

void __cdecl Con_UpdateNotifyLineWindow(
                int localClientNum,
                unsigned int channel,
                bool lineFeed,
                int flags,
                print_msg_dest_t dest)
{
    MessageWindow *DestWindow; // eax

    if ( Con_IsChannelVisible(dest, channel, flags) )
    {
        DestWindow = Con_GetDestWindow(localClientNum, dest);
        Con_UpdateMessageWindowLine(localClientNum, DestWindow, lineFeed, flags);
    }
}

void __cdecl Con_UpdateMessageWindowLine(int localClientNum, MessageWindow *msgwnd, int linefeed, int flags)
{
    int newPadLineOffset; // [esp+0h] [ebp-14h]
    Message *message; // [esp+4h] [ebp-10h]
    unsigned int imod; // [esp+8h] [ebp-Ch]
    MessageLine *line; // [esp+Ch] [ebp-8h]
    MessageLine *linea; // [esp+Ch] [ebp-8h]
    int serverTime; // [esp+10h] [ebp-4h]

    if ( !msgwnd
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 1063, 0, "%s", "msgwnd") )
    {
        __debugbreak();
    }
    if ( msgwnd->firstLineIndex >= (unsigned int)msgwnd->lineCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    1064,
                    0,
                    "msgwnd->firstLineIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                    msgwnd->firstLineIndex,
                    msgwnd->lineCount) )
    {
        __debugbreak();
    }
    if ( msgwnd->messageIndex >= (unsigned int)msgwnd->lineCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    1065,
                    0,
                    "msgwnd->messageIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                    msgwnd->messageIndex,
                    msgwnd->lineCount) )
    {
        __debugbreak();
    }
    serverTime = 0;
    if ( CL_GetLocalClientGlobals(localClientNum) )
        serverTime = CL_GetLocalClientGlobals(localClientNum)->serverTime;
    line = &msgwnd->lines[(msgwnd->activeLineCount + msgwnd->firstLineIndex) % msgwnd->lineCount];
    line->messageIndex = msgwnd->messageIndex;
    line->typingStartTime = 0;
    line->lastTypingSoundTime = 0;
    line->flags = flags;
    line->typingStartTime = GetNextValidPrintTimeForLine(localClientNum, msgwnd, flags);
    if ( line->typingStartTime )
        msgwnd->messages[line->messageIndex].endTime = line->typingStartTime + PrintTimeTotal(msgwnd, line);
    Con_CopyCurrentConsoleLineText(msgwnd, line);
    if ( linefeed )
    {
        if ( msgwnd->activeLineCount == msgwnd->lineCount )
            Con_FreeFirstMessageWindowLine(msgwnd);
        newPadLineOffset = ++msgwnd->activeLineCount - (msgwnd->lineCount - msgwnd->padding);
        if ( newPadLineOffset > 0 )
        {
            imod = (msgwnd->firstLineIndex + newPadLineOffset - 1) % msgwnd->lineCount;
            if ( imod >= msgwnd->lineCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                            1095,
                            0,
                            "imod doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                            imod,
                            msgwnd->lineCount) )
            {
                __debugbreak();
            }
            linea = &msgwnd->lines[imod];
            if ( linea->messageIndex >= (unsigned int)msgwnd->lineCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                            1097,
                            0,
                            "line->messageIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                            linea->messageIndex,
                            msgwnd->lineCount) )
            {
                __debugbreak();
            }
            message = &msgwnd->messages[linea->messageIndex];
            if ( message->endTime - msgwnd->fadeOut > serverTime )
            {
                if ( message->endTime < message->startTime
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                                1102,
                                0,
                                "message->endTime >= message->startTime\n\t%i, %i",
                                message->endTime,
                                message->startTime) )
                {
                    __debugbreak();
                }
                message->endTime = msgwnd->fadeOut + serverTime;
            }
        }
    }
}

void __cdecl Con_FreeFirstMessageWindowLine(MessageWindow *msgwnd)
{
    int activeLineCount; // [esp+0h] [ebp-4h]

    if ( msgwnd->activeLineCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    866,
                    0,
                    "%s",
                    "msgwnd->activeLineCount > 0") )
    {
        __debugbreak();
    }
    --msgwnd->activeLineCount;
    if ( ++msgwnd->firstLineIndex == msgwnd->lineCount )
        msgwnd->firstLineIndex = 0;
    if ( msgwnd == &con.consoleWindow && --con.displayLineOffset < con.visibleLineCount )
    {
        if ( con.consoleWindow.activeLineCount < con.visibleLineCount )
            activeLineCount = con.consoleWindow.activeLineCount;
        else
            activeLineCount = con.visibleLineCount;
        con.displayLineOffset = activeLineCount;
    }
}

void __cdecl Con_CopyCurrentConsoleLineText(MessageWindow *msgwnd, MessageLine *msgLine)
{
    unsigned int poolRemaining; // [esp+0h] [ebp-4h]

    if ( !msgLine
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 906, 0, "%s", "msgLine") )
    {
        __debugbreak();
    }
    while ( Con_NeedToFreeMessageWindowLine(msgwnd, con.lineOffset + 1) )
        Con_FreeFirstMessageWindowLine(msgwnd);
    poolRemaining = msgwnd->textBufSize - msgwnd->textBufPos;
    if ( con.lineOffset > poolRemaining )
    {
        memcpy(
            (unsigned __int8 *)&msgwnd->circularTextBuffer[msgwnd->textBufPos],
            (unsigned __int8 *)con.textTempLine,
            poolRemaining);
        memcpy(
            (unsigned __int8 *)msgwnd->circularTextBuffer,
            (unsigned __int8 *)&con.textTempLine[poolRemaining],
            con.lineOffset - poolRemaining);
    }
    else
    {
        memcpy(
            (unsigned __int8 *)&msgwnd->circularTextBuffer[msgwnd->textBufPos],
            (unsigned __int8 *)con.textTempLine,
            con.lineOffset);
    }
    msgLine->textBufPos = msgwnd->textBufPos;
    msgLine->textBufSize = con.lineOffset;
    if ( (msgwnd->textBufSize & (msgwnd->textBufSize - 1)) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    925,
                    0,
                    "%s",
                    "IsPowerOf2( msgwnd->textBufSize )") )
    {
        __debugbreak();
    }
    msgwnd->textBufPos = (msgwnd->textBufSize - 1) & (con.lineOffset + msgwnd->textBufPos);
    msgwnd->circularTextBuffer[msgwnd->textBufPos] = 10;
    msgwnd->textBufPos = (msgwnd->textBufSize - 1) & (msgwnd->textBufPos + 1);
}

bool __cdecl Con_NeedToFreeMessageWindowLine(MessageWindow *msgwnd, int charCount)
{
    int pastLastChar; // [esp+8h] [ebp-8h]
    MessageLine *line; // [esp+Ch] [ebp-4h]

    if ( !msgwnd->activeLineCount )
        return 0;
    if ( msgwnd->lineCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    890,
                    0,
                    "%s",
                    "msgwnd->lineCount > 0") )
    {
        __debugbreak();
    }
    line = &msgwnd->lines[msgwnd->firstLineIndex];
    if ( (msgwnd->textBufSize & (msgwnd->textBufSize - 1)) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    893,
                    0,
                    "%s",
                    "IsPowerOf2( msgwnd->textBufSize )") )
    {
        __debugbreak();
    }
    pastLastChar = (msgwnd->textBufSize - 1) & (charCount + msgwnd->textBufPos);
    if ( pastLastChar < msgwnd->textBufPos )
        return line->textBufPos >= msgwnd->textBufPos || line->textBufPos < pastLastChar;
    else
        return line->textBufPos >= msgwnd->textBufPos && line->textBufPos < pastLastChar;
}

int __cdecl PrintTimeTotal(MessageWindow *msgwnd, MessageLine *line)
{
    int time; // [esp+4h] [ebp-4h]

    if ( (line->flags & 1) == 0 )
        return 0;
    time = con_typewriterPrintSpeed->current.integer * PrintableCharsCount(msgwnd, line);
    if ( time < con_typewriterDecayStartTime->current.integer )
        time = con_typewriterDecayStartTime->current.integer;
    return con_typewriterDecayDuration->current.integer + time;
}

int __cdecl PrintableCharsCount(const MessageWindow *msgwnd, MessageLine *line)
{
    bool v3; // [esp+0h] [ebp-18h]
    int usedCharCnt; // [esp+4h] [ebp-14h] BYREF
    char c[4]; // [esp+8h] [ebp-10h] BYREF
    int letter; // [esp+Ch] [ebp-Ch]
    int printedCnt; // [esp+10h] [ebp-8h]
    int idx; // [esp+14h] [ebp-4h]

    if ( line->textBufSize )
        v3 = (msgwnd->textBufSize & (msgwnd->textBufSize - 1)) == 0;
    else
        v3 = 1;
    if ( !v3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    969,
                    0,
                    "%s",
                    "line->textBufSize ? IsPowerOf2( msgwnd->textBufSize ) : true") )
    {
        __debugbreak();
    }
    printedCnt = 0;
    idx = 0;
    while ( idx < line->textBufSize )
    {
        c[0] = msgwnd->circularTextBuffer[(msgwnd->textBufSize - 1) & (idx + line->textBufPos)];
        c[1] = msgwnd->circularTextBuffer[(msgwnd->textBufSize - 1) & (line->textBufPos + idx + 1)];
        letter = SEH_DecodeLetter(c[0], c[1], &usedCharCnt, 0);
        idx += usedCharCnt;
        ++printedCnt;
        if ( letter == 94 )
        {
            if ( (c[0] = msgwnd->circularTextBuffer[(msgwnd->textBufSize - 1) & (idx + line->textBufPos)], c)
                && c[0] != 94
                && c[0] >= 48
                && c[0] <= 64
                || c && c[0] == 70 )
            {
                ++idx;
            }
        }
    }
    return printedCnt;
}

int __cdecl GetNextValidPrintTimeForLine(int localClientNum, MessageWindow *msgwnd, char flags)
{
    int nextPrintTime; // [esp+0h] [ebp-10h]
    MessageLine *line; // [esp+4h] [ebp-Ch]
    int lineIdx; // [esp+8h] [ebp-8h]
    int serverTime; // [esp+Ch] [ebp-4h]

    if ( (flags & 1) == 0 )
        return 0;
    serverTime = CL_GetLocalClientGlobals(localClientNum)->serverTime;
    lineIdx = LatestActiveTypewrittenLineIdx(msgwnd);
    if ( lineIdx == -1 )
        return serverTime + 250;
    line = &msgwnd->lines[lineIdx];
    nextPrintTime = line->typingStartTime + PrintTimeWriteOut(msgwnd, line) + 150;
    if ( nextPrintTime - serverTime >= 250 )
        return nextPrintTime;
    else
        return serverTime + 250;
}

int __cdecl LatestActiveTypewrittenLineIdx(MessageWindow *msgwnd)
{
    int idx; // [esp+8h] [ebp-4h]

    if ( !msgwnd->activeLineCount )
        return -1;
    for ( idx = msgwnd->activeLineCount - 1; idx >= 0; --idx )
    {
        if ( (msgwnd->lines[(idx + msgwnd->firstLineIndex) % msgwnd->lineCount].flags & 1) != 0 )
            return (idx + msgwnd->firstLineIndex) % msgwnd->lineCount;
    }
    return -1;
}

int __cdecl PrintTimeWriteOut(MessageWindow *msgwnd, MessageLine *line)
{
    if ( (line->flags & 1) != 0 )
        return con_typewriterPrintSpeed->current.integer * PrintableCharsCount(msgwnd, line);
    else
        return 0;
}

void __cdecl Con_Linefeed(int localClientNum, unsigned int channel, int flags)
{
    Con_UpdateNotifyLine(localClientNum, channel, 1, flags);
    con.lineOffset = 0;
    if ( con.displayLineOffset == con.consoleWindow.activeLineCount - 1 )
        ++con.displayLineOffset;
}

const char *__cdecl CL_TextLineWrapPosition(
                const char *txt,
                int bufferSize,
                int pixelsAvailable,
                Font_s *font,
                float scale,
                int lineBroken)
{
    const char *lineWrapPos; // [esp+4h] [ebp-4h]

    if ( lineBroken )
    {
        while ( *txt == 32 )
            ++txt;
    }
    lineWrapPos = R_TextLineWrapPosition(txt, bufferSize, pixelsAvailable, font, scale);
    if ( !lineWrapPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 1249, 0, "%s", "lineWrapPos") )
    {
        __debugbreak();
    }
    return *lineWrapPos != 0 ? lineWrapPos : 0;
}

void __cdecl CL_ConsoleFixPosition()
{
    CL_ConsolePrint(0, 0, "\n", 0, 0, 0);
    con.displayLineOffset = con.consoleWindow.activeLineCount - 1;
    if ( Sys_IsRemoteDebugServer() )
    {
        if ( Sys_DebugSocketReady(0) )
        {
            Sys_WriteDebugSocketMessageType(0x2Bu);
            Sys_EndWriteDebugSocket();
        }
    }
}

void __cdecl CL_ReviveMessagePrint(
                int localClientNum,
                char *reviveString,
                Material *iconShader,
                float iconWidth,
                float iconHeight,
                bool horzFlipIcon)
{
    int msgLen; // [esp+10h] [ebp-410h]
    unsigned int reviveMsgLen; // [esp+14h] [ebp-40Ch]
    char reviveMsg[1024]; // [esp+18h] [ebp-408h] BYREF
    int pixelWidth; // [esp+41Ch] [ebp-4h]

    msgLen = CL_AddMessageIcon(reviveMsg, 0, 0x400u, iconShader, iconWidth, iconHeight, horzFlipIcon);
    while ( *reviveString )
        msgLen = CL_AddMessageChar(reviveMsg, msgLen, 0x400u, *reviveString++);
    reviveMsgLen = CL_AddMessageChar(reviveMsg, msgLen, 0x400u, 10);
    if ( reviveMsgLen >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    1577,
                    0,
                    "reviveMsgLen < ARRAY_COUNT( reviveMsg )\n\t%i, %i",
                    reviveMsgLen,
                    1024) )
    {
        __debugbreak();
    }
    reviveMsg[reviveMsgLen] = 0;
    pixelWidth = con.visiblePixelWidth;
    CL_ConsolePrint(localClientNum, 5, reviveMsg, 0, con.visiblePixelWidth, 0);
}

int __cdecl CL_AddMessageChar(char *msg, unsigned int msgLen, unsigned int msgMaxLen, char c)
{
    if ( !c && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 1506, 0, "%s", "c != '\\0'") )
        __debugbreak();
    if ( msgLen + 1 > msgMaxLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    1507,
                    0,
                    "%s",
                    "msgLen + 1 <= msgMaxLen") )
    {
        __debugbreak();
    }
    msg[msgLen] = c;
    return msgLen + 1;
}

unsigned int __cdecl CL_AddMessageIcon(
                char *msg,
                unsigned int msgLen,
                unsigned int msgMaxLen,
                Material *iconShader,
                float iconWidth,
                float iconHeight,
                bool horzFlipIcon)
{
    unsigned int v7; // eax
    char v9; // [esp+0h] [ebp-48h]
    char v10; // [esp+4h] [ebp-44h]
    int v11; // [esp+Ch] [ebp-3Ch]
    int v12; // [esp+28h] [ebp-20h]
    int msgLena; // [esp+54h] [ebp+Ch]
    int msgLenb; // [esp+54h] [ebp+Ch]
    unsigned int msgLenc; // [esp+54h] [ebp+Ch]
    unsigned int msgLend; // [esp+54h] [ebp+Ch]

    if ( iconWidth <= 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 1534, 0, "%s", "iconWidth > 0") )
    {
        __debugbreak();
    }
    if ( iconHeight <= 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 1535, 0, "%s", "iconHeight > 0") )
    {
        __debugbreak();
    }
    if ( !IsValidMaterialHandle(iconShader)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    1538,
                    0,
                    "%s",
                    "IsValidMaterialHandle( iconShader )") )
    {
        __debugbreak();
    }
    v7 = CL_AddMessageChar(msg, msgLen, msgMaxLen, 94);
    msgLena = CL_AddMessageChar(msg, v7, msgMaxLen, horzFlipIcon + 1);
    if ( (int)((float)(iconWidth * 32.0) + 9.313225746154785e-10) < 127 )
        v12 = (int)((float)(iconWidth * 32.0) + 9.313225746154785e-10);
    else
        v12 = 127;
    if ( v12 > 16 )
        v10 = v12;
    else
        v10 = 16;
    msgLenb = CL_AddMessageChar(msg, msgLena, msgMaxLen, v10 + 16);
    if ( (int)((float)(iconHeight * 32.0) + 9.313225746154785e-10) < 127 )
        v11 = (int)((float)(iconHeight * 32.0) + 9.313225746154785e-10);
    else
        v11 = 127;
    if ( v11 > 16 )
        v9 = v11;
    else
        v9 = 16;
    msgLenc = CL_AddMessageChar(msg, msgLenb, msgMaxLen, v9 + 16);
    if ( msgLenc + 4 > msgMaxLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    1551,
                    0,
                    "%s",
                    "msgLen + sizeof( iconShader ) <= msgMaxLen") )
    {
        __debugbreak();
    }
    *(unsigned int *)&msg[msgLenc] = (unsigned int)iconShader;
    msgLend = msgLenc + 4;
    if ( msgLend - msgLen != 8
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    1557,
                    0,
                    "%s",
                    "msgLen - startMsgLen == CONTXTCMD_LEN_HUDICON + 1") )
    {
        __debugbreak();
    }
    return msgLend;
}

void __cdecl CL_DeathMessagePrint(
                int localClientNum,
                char *attackerName,
                char attackerColorIndex,
                char *victimName,
                char victimColorIndex,
                Material *iconShader,
                float iconWidth,
                float iconHeight,
                bool horzFlipIcon)
{
    unsigned int v9; // eax
    unsigned int v10; // eax
    unsigned int v11; // eax
    unsigned int v12; // eax
    unsigned int v13; // eax
    char *v14; // [esp+Ch] [ebp-428h]
    int v15; // [esp+10h] [ebp-424h]
    char *v16; // [esp+14h] [ebp-420h]
    int v17; // [esp+18h] [ebp-41Ch]
    char *v18; // [esp+1Ch] [ebp-418h]
    int msgLen; // [esp+20h] [ebp-414h]
    int deathMsgLen; // [esp+24h] [ebp-410h]
    unsigned int deathMsgLena; // [esp+24h] [ebp-410h]
    unsigned int deathMsgLend; // [esp+24h] [ebp-410h]
    unsigned int deathMsgLenb; // [esp+24h] [ebp-410h]
    unsigned int deathMsgLene; // [esp+24h] [ebp-410h]
    unsigned int deathMsgLenc; // [esp+24h] [ebp-410h]
    char deathMsg[1024]; // [esp+2Ch] [ebp-408h] BYREF
    int color; // [esp+430h] [ebp-4h]

    if ( !attackerName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    1600,
                    0,
                    "%s",
                    "attackerName != NULL") )
    {
        __debugbreak();
    }
    if ( !victimName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    1601,
                    0,
                    "%s",
                    "victimName != NULL") )
    {
        __debugbreak();
    }
    if ( (!&victimColorIndex || victimColorIndex == 94 || victimColorIndex < 48 || victimColorIndex > 64)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    1602,
                    0,
                    "%s",
                    "I_IsColorIndex( &victimColorIndex )") )
    {
        __debugbreak();
    }
    if ( (!&attackerColorIndex || attackerColorIndex == 94 || attackerColorIndex < 48 || attackerColorIndex > 64)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    1603,
                    0,
                    "%s",
                    "I_IsColorIndex( &attackerColorIndex )") )
    {
        __debugbreak();
    }
    if ( !cl_noprint || !cl_noprint->current.enabled )
    {
        if ( !con.initialized )
        {
            Con_OneTimeInit();
            if ( !con.initialized
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                            1612,
                            0,
                            "%s",
                            "con.initialized") )
            {
                __debugbreak();
            }
        }
        deathMsgLen = 0;
        if ( con.lineOffset )
            Con_Linefeed(localClientNum, con.prevChannel, 0);
        color = ColorIndex(0x37u);
        if ( *attackerName )
        {
            if ( (attackerColorIndex < 48 || attackerColorIndex > 64)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                            1626,
                            0,
                            "attackerColorIndex not in [COLOR_FIRST, COLOR_LAST]\n\t%i not in [%i, %i]",
                            attackerColorIndex,
                            48,
                            64) )
            {
                __debugbreak();
            }
            v9 = CL_AddMessageChar(deathMsg, 0, 0x400u, 94);
            v18 = attackerName;
            msgLen = CL_AddMessageChar(deathMsg, v9, 0x400u, attackerColorIndex);
            while ( *v18 )
                msgLen = CL_AddMessageChar(deathMsg, msgLen, 0x400u, *v18++);
            v10 = CL_AddMessageChar(deathMsg, msgLen, 0x400u, 94);
            v11 = CL_AddMessageChar(deathMsg, v10, 0x400u, 55);
            deathMsgLen = CL_AddMessageChar(deathMsg, v11, 0x400u, 32);
        }
        deathMsgLena = CL_AddMessageIcon(deathMsg, deathMsgLen, 0x400u, iconShader, iconWidth, iconHeight, horzFlipIcon);
        if ( (victimColorIndex < 48 || victimColorIndex > 64)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        1637,
                        0,
                        "victimColorIndex not in [COLOR_FIRST, COLOR_LAST]\n\t%i not in [%i, %i]",
                        victimColorIndex,
                        48,
                        64) )
        {
            __debugbreak();
        }
        v12 = CL_AddMessageChar(deathMsg, deathMsgLena, 0x400u, 32);
        v13 = CL_AddMessageChar(deathMsg, v12, 0x400u, 94);
        deathMsgLend = CL_AddMessageChar(deathMsg, v13, 0x400u, victimColorIndex);
        v16 = victimName;
        v17 = deathMsgLend;
        while ( *v16 )
            v17 = CL_AddMessageChar(deathMsg, v17, 0x400u, *v16++);
        deathMsgLenb = v17;
        if ( !*attackerName )
        {
            deathMsgLene = CL_AddMessageChar(deathMsg, v17, 0x400u, 32);
            v14 = UI_SafeTranslateString("CGAME_QUAKE_SUICIDE");
            v15 = deathMsgLene;
            while ( *v14 )
                v15 = CL_AddMessageChar(deathMsg, v15, 0x400u, *v14++);
            deathMsgLenb = v15;
        }
        deathMsgLenc = CL_AddMessageChar(deathMsg, deathMsgLenb, 0x400u, 10);
        if ( deathMsgLenc >= 0x400
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        1654,
                        0,
                        "deathMsgLen < ARRAY_COUNT( deathMsg )\n\t%i, %i",
                        deathMsgLenc,
                        1024) )
        {
            __debugbreak();
        }
        deathMsg[deathMsgLenc] = 0;
        CL_ConsolePrint(localClientNum, 5, deathMsg, 0, con.visiblePixelWidth, 0);
    }
}

void __cdecl Con_AutoCompleteFromList(
                const char **strings,
                unsigned int stringCount,
                const char *prefix,
                char *completed,
                unsigned int sizeofCompleted)
{
    int v5; // [esp+0h] [ebp-20h]
    const char *string; // [esp+10h] [ebp-10h]
    unsigned int charIndex; // [esp+14h] [ebp-Ch]
    unsigned int stringIndex; // [esp+1Ch] [ebp-4h]

    v5 = strlen(prefix);
    *completed = 0;
    for ( stringIndex = 0; stringIndex < stringCount; ++stringIndex )
    {
        string = strings[stringIndex];
        if ( !I_strnicmp(prefix, string, v5) )
        {
            if ( *completed )
            {
                for ( charIndex = v5; string[charIndex] == completed[charIndex] && completed[charIndex]; ++charIndex )
                    ;
                completed[charIndex] = 0;
            }
            else
            {
                I_strncpyz(completed, string, sizeofCompleted);
            }
        }
    }
}

const char *__cdecl Con_TokenizeInput()
{
    const char *cmd; // [esp+8h] [ebp-4h]

    Cmd_TokenizeStringNoEval(g_consoleField.buffer);
    cmd = Cmd_Argv(0);
    if ( *cmd == 92 || *cmd == 47 )
        ++cmd;
    while ( isspace(*cmd) )
        ++cmd;
    return cmd;
}

char __cdecl Con_AnySpaceAfterCommand()
{
    int charIndex; // [esp+0h] [ebp-4h]

    for ( charIndex = 0; isspace(g_consoleField.buffer[charIndex]); ++charIndex )
        ;
    while ( g_consoleField.buffer[charIndex] )
    {
        if ( isspace(g_consoleField.buffer[charIndex]) )
            return 1;
        ++charIndex;
    }
    return 0;
}

bool __cdecl Con_IsAutoCompleteMatch(const char *query, const char *matchToText, int matchTextLen)
{
    int matchLetter; // [esp+0h] [ebp-Ch]
    int matchTextPos; // [esp+4h] [ebp-8h]
    const char *queryPos; // [esp+8h] [ebp-4h]

    if ( !query && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 2077, 0, "%s", "query") )
        __debugbreak();
    if ( !matchToText
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 2078, 0, "%s", "matchToText") )
    {
        __debugbreak();
    }
    if ( !matchTextLen
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 2079, 0, "%s", "matchTextLen") )
    {
        __debugbreak();
    }
    if ( !con_ignoreMatchPrefixOnly && con_matchPrefixOnly->current.enabled )
        return I_strnicmp(query, matchToText, matchTextLen) == 0;
    matchTextPos = 0;
    matchLetter = tolower(*matchToText);
    for ( queryPos = query; *queryPos; ++queryPos )
    {
        if ( tolower(*queryPos) == matchLetter )
        {
            if ( ++matchTextPos == matchTextLen )
                return 1;
            matchLetter = tolower(matchToText[matchTextPos]);
        }
        else if ( con_matchPrefixOnly->current.enabled )
        {
            if ( matchTextPos )
            {
                matchTextPos = 0;
                matchLetter = tolower(*matchToText);
            }
        }
    }
    return 0;
}

bool __cdecl Con_HasTooManyMatchesToShow()
{
    return conDrawInputGlob.matchCount > con_inputMaxMatchesShown;
}

bool __cdecl Con_IsDvarCommand(const char *cmd)
{
    if ( !I_stricmp(cmd, "set") )
        return 1;
    if ( !I_stricmp(cmd, "seta") )
        return 1;
    if ( !I_stricmp(cmd, "sets") )
        return 1;
    if ( !I_stricmp(cmd, "reset") )
        return 1;
    if ( I_stricmp(cmd, "toggle") )
        return I_stricmp(cmd, "togglep") == 0;
    return 1;
}

void __cdecl Con_ForEach(
                bool isDvarCommand,
                void (__cdecl *callbackDvar)(const char *),
                void (__cdecl *callbackCmd)(const char *))
{
    Con_Restricted_ForEach(isDvarCommand, callbackDvar, callbackCmd);
}

char __cdecl Con_CycleAutoComplete(int step)
{
    if ( !conDrawInputGlob.mayAutoComplete
        || conDrawInputGlob.matchCount <= 1
        || conDrawInputGlob.matchCount >= con_inputMaxMatchesShown
        || conDrawInputGlob.hasExactMatch && Con_AnySpaceAfterCommand() )
    {
        return 0;
    }
    conDrawInputGlob.matchIndex += step;
    if ( conDrawInputGlob.matchIndex >= 0 )
    {
        if ( conDrawInputGlob.matchIndex >= conDrawInputGlob.matchCount )
            conDrawInputGlob.matchIndex = 0;
    }
    else
    {
        conDrawInputGlob.matchIndex = conDrawInputGlob.matchCount - 1;
    }
    return 1;
}

bool __cdecl Con_HasActiveAutoComplete()
{
    return conDrawInputGlob.matchIndex >= 0 && conDrawInputGlob.autoCompleteChoice[0];
}

char __cdecl Con_CommitToAutoComplete()
{
    char v1; // [esp+13h] [ebp-11h]
    char *buffer; // [esp+18h] [ebp-Ch]
    ConDrawInputGlob *v3; // [esp+1Ch] [ebp-8h]
    const char *originalCommand; // [esp+20h] [ebp-4h]

    if ( !Con_HasActiveAutoComplete() )
        return 0;
    originalCommand = Con_TokenizeInput();
    if ( Con_IsDvarCommand(originalCommand) )
    {
        Com_sprintf(g_consoleField.buffer, 0x100u, "%s %s", originalCommand, conDrawInputGlob.autoCompleteChoice);
    }
    else
    {
        v3 = &conDrawInputGlob;
        buffer = g_consoleField.buffer;
        do
        {
            v1 = v3->autoCompleteChoice[0];
            *buffer = v3->autoCompleteChoice[0];
            v3 = (ConDrawInputGlob *)((char *)v3 + 1);
            ++buffer;
        }
        while ( v1 );
    }
    Cmd_EndTokenizedString();
    g_consoleField.cursor = strlen(g_consoleField.buffer);
    g_consoleField.buffer[g_consoleField.cursor++] = 32;
    g_consoleField.buffer[g_consoleField.cursor] = 0;
    g_consoleField.drawWidth = SEH_PrintStrlen(g_consoleField.buffer);
    Con_CancelAutoComplete();
    return 1;
}

char __cdecl Con_CancelAutoComplete()
{
    if ( !Con_HasActiveAutoComplete() )
        return 0;
    conDrawInputGlob.matchIndex = -1;
    conDrawInputGlob.autoCompleteChoice[0] = 0;
    return 1;
}

void __cdecl Con_AllowAutoCompleteCycling(bool isAllowed)
{
    conDrawInputGlob.mayAutoComplete = isAllowed;
}

void __cdecl Con_DrawGameMessageWindow(
                int localClientNum,
                unsigned int windowIndex,
                int xPos,
                int yPos,
                int horzAlign,
                int vertAlign,
                Font_s *font,
                float fontScale,
                float *color,
                int textStyle,
                char textAlignMode,
                msgwnd_mode_t mode)
{
    if (!cg_paused->current.integer)
    {
        if (windowIndex >= 3
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                3067,
                0,
                "windowIndex doesn't index GAMEMSG_WINDOW_COUNT\n\t%i not in [0, %i)",
                windowIndex,
                3))
        {
            __debugbreak();
        }
        Con_DrawMessageWindow(
            localClientNum,
            &con.messageBuffer[localClientNum].gamemsgWindows[windowIndex],
            xPos,
            yPos,
            (int)((float)(fontScale * 48.0) + 9.313225746154785e-10),
            horzAlign,
            vertAlign,
            font,
            color,
            textStyle,
            1.0,
            mode,
            textAlignMode);
    }
}

void __cdecl Con_DrawMessageWindow(
                int localClientNum,
                MessageWindow *msgwnd,
                int x,
                int y,
                int charHeight,
                int horzAlign,
                int vertAlign,
                Font_s *font,
                float *color,
                int textStyle,
                float msgwndScale,
                msgwnd_mode_t mode,
                char textAlignMode)
{
    if ( !msgwnd
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 3020, 0, "%s", "msgwnd") )
    {
        __debugbreak();
    }
    if ( !CL_ShouldntDrawMessageWindow(localClientNum) )
    {
        switch ( mode )
        {
            case MWM_BOTTOMUP_ALIGN_TOP:
            case MWM_TOPDOWN_ALIGN_BOTTOM:
                Con_DrawMessageWindowOldToNew(
                    localClientNum,
                    msgwnd,
                    x,
                    y,
                    charHeight,
                    horzAlign,
                    vertAlign,
                    mode == MWM_TOPDOWN_ALIGN_BOTTOM,
                    font,
                    color,
                    textStyle,
                    msgwndScale,
                    textAlignMode);
                break;
            case MWM_BOTTOMUP_ALIGN_BOTTOM:
            case MWM_TOPDOWN_ALIGN_TOP:
                Con_DrawMessageWindowNewToOld(
                    localClientNum,
                    msgwnd,
                    x,
                    y,
                    charHeight,
                    horzAlign,
                    vertAlign,
                    mode == MWM_BOTTOMUP_ALIGN_BOTTOM,
                    font,
                    color,
                    textStyle,
                    msgwndScale,
                    textAlignMode);
                break;
            default:
                if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 3038, 0, "unhandled case") )
                    __debugbreak();
                break;
        }
    }
}

void __cdecl Con_DrawMessageWindowNewToOld(
                int localClientNum,
                MessageWindow *msgwnd,
                int x,
                int y,
                int hudCharHeight,
                int horzAlign,
                int vertAlign,
                bool up,
                Font_s *font,
                float *color,
                int textStyle,
                float msgwndScale,
                char textAlignMode)
{
    float v13; // [esp+8h] [ebp-60h]
    float v14; // [esp+34h] [ebp-34h]
    float finalColor[4]; // [esp+38h] [ebp-30h] BYREF
    Message *message; // [esp+48h] [ebp-20h]
    float lerpFactor; // [esp+4Ch] [ebp-1Ch]
    int lineOffset; // [esp+50h] [ebp-18h]
    int imod; // [esp+54h] [ebp-14h]
    int time; // [esp+58h] [ebp-10h]
    MessageLine *line; // [esp+5Ch] [ebp-Ch]
    int charHeight; // [esp+60h] [ebp-8h]
    int serverTime; // [esp+64h] [ebp-4h]

    if ( !msgwnd
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 2862, 0, "%s", "msgwnd") )
    {
        __debugbreak();
    }
    serverTime = CL_GetLocalClientGlobals(localClientNum)->serverTime;
    Con_CullFinishedLines(serverTime, msgwnd);
    charHeight = hudCharHeight;
    if ( !up )
        y -= charHeight;
    for ( lineOffset = 0; lineOffset < msgwnd->activeLineCount; ++lineOffset )
    {
        if ( msgwnd->lineCount <= 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        2873,
                        0,
                        "%s",
                        "msgwnd->lineCount > 0") )
        {
            __debugbreak();
        }
        imod = (lineOffset + msgwnd->firstLineIndex) % msgwnd->lineCount;
        if ( (unsigned int)imod >= msgwnd->lineCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        2875,
                        0,
                        "imod doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                        imod,
                        msgwnd->lineCount) )
        {
            __debugbreak();
        }
        line = &msgwnd->lines[imod];
        if ( line->messageIndex >= (unsigned int)msgwnd->lineCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        2877,
                        0,
                        "line->messageIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                        line->messageIndex,
                        msgwnd->lineCount) )
        {
            __debugbreak();
        }
        message = &msgwnd->messages[line->messageIndex];
        if ( Con_ShouldProcessMessage(message, serverTime) )
        {
            time = serverTime - message->startTime;
            if ( time < msgwnd->scrollTime )
            {
                lerpFactor = 1.0 - (float)((float)time / (float)msgwnd->scrollTime);
                if ( (float)(lerpFactor - 1.0) < 0.0 )
                    v14 = lerpFactor;
                else
                    v14 = 1.0f;
                if ( (float)(0.0 - lerpFactor) < 0.0 )
                    v13 = v14;
                else
                    v13 = 0.0f;
                lerpFactor = v13;
                if ( up )
                    y += (int)((float)((float)charHeight * lerpFactor) + 9.313225746154785e-10);
                else
                    y -= (int)((float)((float)charHeight * lerpFactor) + 9.313225746154785e-10);
            }
        }
    }
    finalColor[0] = *color;
    finalColor[1] = color[1];
    finalColor[2] = color[2];
    finalColor[3] = color[3];
    for ( lineOffset = msgwnd->activeLineCount - 1; lineOffset >= 0; --lineOffset )
    {
        imod = (lineOffset + msgwnd->firstLineIndex) % msgwnd->lineCount;
        if ( (unsigned int)imod >= msgwnd->lineCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        2903,
                        0,
                        "imod doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                        imod,
                        msgwnd->lineCount) )
        {
            __debugbreak();
        }
        line = &msgwnd->lines[imod];
        if ( line->messageIndex >= (unsigned int)msgwnd->lineCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        2906,
                        0,
                        "line->messageIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                        line->messageIndex,
                        msgwnd->lineCount) )
        {
            __debugbreak();
        }
        message = &msgwnd->messages[line->messageIndex];
        if ( up )
            y -= charHeight;
        else
            y += charHeight;
        if ( serverTime - message->endTime < 0 )
        {
            finalColor[3] = Con_GetMessageAlpha(message, msgwnd, serverTime, 1) * color[3];
            Con_DrawMessageLineOnHUD(
                localClientNum,
                &scrPlaceView[localClientNum],
                x,
                y,
                charHeight,
                horzAlign,
                vertAlign,
                font,
                msgwnd,
                imod,
                finalColor,
                textStyle,
                msgwndScale,
                textAlignMode);
        }
    }
}

bool __cdecl Con_ShouldProcessMessage(Message *message, int serverTime)
{
    if ( Demo_IsPlaying() )
        return message->startTime <= serverTime + 1000;
    if ( (message->startTime < 0 || message->startTime > serverTime + 1000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    2618,
                    0,
                    "message->startTime not in [0, serverTime + CON_MSG_TIME_DRIFT_BUFFER]\n\t%i not in [%i, %i]",
                    message->startTime,
                    0,
                    serverTime + 1000) )
    {
        __debugbreak();
    }
    return 1;
}


float MY_GLOWCOLOR[4] = { 0.0, 0.3, 0.0, 1.0 };
void __cdecl Con_DrawMessageLineOnHUD(
                int localClientNum,
                const ScreenPlacement *scrPlace,
                int x,
                int y,
                int charHeight,
                int horzAlign,
                int vertAlign,
                Font_s *font,
                const MessageWindow *msgwnd,
                int lineIdx,
                float *color,
                int textStyle,
                float msgwndScale,
                char textAlignMode)
{
    int v14; // [esp+34h] [ebp-38h]
    int v15; // [esp+38h] [ebp-34h]
    const DvarValue *glowColor; // [esp+3Ch] [ebp-30h]
    float typewriterColor[4]; // [esp+40h] [ebp-2Ch] BYREF
    float xScale; // [esp+50h] [ebp-1Ch] BYREF
    float yAdj; // [esp+54h] [ebp-18h] BYREF
    float xAdj; // [esp+58h] [ebp-14h] BYREF
    int time; // [esp+5Ch] [ebp-10h]
    MessageLine *line; // [esp+60h] [ebp-Ch]
    float fontScale; // [esp+64h] [ebp-8h]
    float yScale; // [esp+68h] [ebp-4h] BYREF

    time = CL_GetLocalClientGlobals(localClientNum)->serverTime;
    if ( LineVisible(msgwnd, lineIdx, time) )
    {
        line = &msgwnd->lines[lineIdx];
        if ( (line->flags & 1) != 0 )
            font = UI_GetFontHandle(0, 1, 1.0);
        fontScale = (float)charHeight / 48.0;
        xScale = R_NormalizedTextScale(font, fontScale * msgwndScale);
        yScale = xScale;
        v15 = textAlignMode & 3;
        if ( v15 == 1 )
        {
            x -= (int)(float)((float)((float)R_ConsoleTextWidth(
                                                                                 msgwnd->circularTextBuffer,
                                                                                 msgwnd->textBufSize,
                                                                                 line->textBufPos,
                                                                                 line->textBufSize,
                                                                                 font)
                                                            * xScale)
                                            * 0.5);
        }
        else if ( v15 == 2 )
        {
            x -= (int)(float)((float)R_ConsoleTextWidth(
                                                                 msgwnd->circularTextBuffer,
                                                                 msgwnd->textBufSize,
                                                                 line->textBufPos,
                                                                 line->textBufSize,
                                                                 font)
                                            * xScale);
        }
        v14 = textAlignMode & 0xC;
        if ( v14 == 4 )
        {
            y += (int)(float)((float)R_TextHeight(font) * yScale);
        }
        else if ( v14 == 8 )
        {
            y += (int)(float)((float)((float)R_TextHeight(font) * yScale) * 0.5);
        }
        xAdj = (float)x;
        yAdj = (float)y;
        ScrPlace_ApplyRect(scrPlace, &xAdj, &yAdj, &xScale, &yScale, horzAlign, vertAlign);
        if ( (line->flags & 1) != 0 )
        {
            if ( (line->flags & 0x40) != 0 )
            {
                typewriterColor[0] = con_typewriterColorBase->current.value;
                typewriterColor[1] = con_typewriterColorBase->current.vector[1];
                typewriterColor[2] = con_typewriterColorBase->current.vector[2];
                typewriterColor[3] = 1.0f;
                if ( (line->flags & 0x10) != 0 )
                {
                    glowColor = &con_typewriterColorGlowCheckpoint->current;
                }
                else if ( (line->flags & 4) != 0 )
                {
                    glowColor = &con_typewriterColorGlowCompleted->current;
                }
                else if ( (line->flags & 8) != 0 )
                {
                    glowColor = &con_typewriterColorGlowFailed->current;
                }
                else
                {
                    glowColor = &con_typewriterColorGlowUpdated->current;
                }
                TypewriterSounds(localClientNum, msgwnd, line);
                R_AddCmdDrawConsoleTextPulseFX(
                    msgwnd->circularTextBuffer,
                    msgwnd->textBufSize,
                    line->textBufPos,
                    line->textBufSize,
                    font,
                    xAdj,
                    yAdj,
                    xScale,
                    yScale,
                    typewriterColor,
                    textStyle,
                    &glowColor->value,
                    line->typingStartTime,
                    con_typewriterPrintSpeed->current.integer,
                    con_typewriterDecayStartTime->current.integer,
                    con_typewriterDecayDuration->current.integer,
                    cgMedia.textDecodeCharacters,
                    cgMedia.textDecodeCharactersGlow);
            }
            else
            {
                TypewriterSounds(localClientNum, msgwnd, line);
                R_AddCmdDrawConsoleTextTypewriterFX(
                    msgwnd->circularTextBuffer,
                    msgwnd->textBufSize,
                    line->textBufPos,
                    line->textBufSize,
                    font,
                    xAdj,
                    yAdj,
                    xScale,
                    yScale,
                    color,
                    textStyle,
                    line->typingStartTime,
                    con_typewriterPrintSpeed->current.integer,
                    con_typewriterDecayStartTime->current.integer,
                    con_typewriterDecayDuration->current.integer);
            }
        }
        else if ( (line->flags & 0x20) != 0 )
        {
            R_AddCmdDrawConsoleTextSubtitle(
                msgwnd->circularTextBuffer,
                msgwnd->textBufSize,
                line->textBufPos,
                line->textBufSize,
                font,
                xAdj,
                yAdj,
                xScale,
                yScale,
                color,
                textStyle,
                MY_GLOWCOLOR);
        }
        else
        {
            R_AddCmdDrawConsoleText(
                msgwnd->circularTextBuffer,
                msgwnd->textBufSize,
                line->textBufPos,
                line->textBufSize,
                font,
                xAdj,
                yAdj,
                xScale,
                yScale,
                color,
                textStyle);
        }
    }
}

bool __cdecl LineVisible(const MessageWindow *msgwnd, int lineIdx, int time)
{
    if ( !msgwnd
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 2634, 0, "%s", "msgwnd") )
    {
        __debugbreak();
    }
    return time >= msgwnd->lines[lineIdx].typingStartTime;
}

void __cdecl TypewriterSounds(int localClientNum, const MessageWindow *msgwnd, MessageLine *line)
{
    SndEntHandle entHandle; // [esp+1Ch] [ebp-20h]
    clientActive_t *LocalClientGlobals; // [esp+24h] [ebp-18h]
    int typeoutTime; // [esp+28h] [ebp-14h]
    int timeElapsed; // [esp+2Ch] [ebp-10h]
    int lastSoundTime; // [esp+34h] [ebp-8h]
    int decayStartTime; // [esp+38h] [ebp-4h]

    if ( con_typewriterEnabledSounds->current.enabled )
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
        timeElapsed = LocalClientGlobals->serverTime - line->typingStartTime;
        lastSoundTime = line->lastTypingSoundTime - line->typingStartTime;
        typeoutTime = con_typewriterPrintSpeed->current.integer * PrintableCharsCount(msgwnd, line);
        decayStartTime = con_typewriterDecayStartTime->current.integer;
        if ( decayStartTime < typeoutTime )
            decayStartTime = typeoutTime;
        if ( timeElapsed <= decayStartTime
            && timeElapsed <= typeoutTime
            && lastSoundTime < con_typewriterPrintSpeed->current.integer
                                             * (timeElapsed
                                                / con_typewriterPrintSpeed->current.integer) )
        {
            entHandle.field = SND_EntHandle(localClientNum, 4095, 0, 0, 1, TEAM_FREE).field;
            SND_Play((char*)"uin_typewriter", 0, 1.0, entHandle, 0, 0, 0);
            line->lastTypingSoundTime = LocalClientGlobals->serverTime;
        }
    }
}

void __cdecl Con_CullFinishedLines(int serverTime, MessageWindow *msgwnd)
{
    const MessageLine *line; // [esp+4h] [ebp-4h]

    if ( !msgwnd
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 2791, 0, "%s", "msgwnd") )
    {
        __debugbreak();
    }
    if ( msgwnd->lineCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    2792,
                    0,
                    "%s",
                    "msgwnd->lineCount > 0") )
    {
        __debugbreak();
    }
    while ( msgwnd->activeLineCount )
    {
        line = &msgwnd->lines[msgwnd->firstLineIndex];
        if ( line->messageIndex >= (unsigned int)msgwnd->lineCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        2798,
                        0,
                        "line->messageIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                        line->messageIndex,
                        msgwnd->lineCount) )
        {
            __debugbreak();
        }
        if ( serverTime - msgwnd->messages[line->messageIndex].endTime < 0 )
            break;
        Con_FreeFirstMessageWindowLine(msgwnd);
    }
}

double __cdecl Con_GetMessageAlpha(Message *message, MessageWindow *msgwnd, int serverTime, bool scrollsIntoPlace)
{
    float curalpha; // [esp+4h] [ebp-4h]

    curalpha = 1.0f;
    if ( !message
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 2812, 0, "%s", "message") )
    {
        __debugbreak();
    }
    if ( !msgwnd
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 2813, 0, "%s", "msgwnd") )
    {
        __debugbreak();
    }
    if ( message->endTime - serverTime < msgwnd->fadeOut )
    {
        if ( msgwnd->fadeOut <= 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        2820,
                        0,
                        "%s\n\t(msgwnd->fadeOut) = %i",
                        "(msgwnd->fadeOut > 0)",
                        msgwnd->fadeOut) )
        {
            __debugbreak();
        }
        curalpha = (float)((float)(message->endTime - serverTime) / (float)msgwnd->fadeOut) * 1.0;
    }
    if ( scrollsIntoPlace && msgwnd->fadeIn < msgwnd->scrollTime )
    {
        if ( serverTime - message->startTime < msgwnd->scrollTime )
        {
            if ( serverTime - message->startTime <= msgwnd->scrollTime - msgwnd->fadeIn )
                return 0.0;
            if ( msgwnd->fadeIn <= 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                            2832,
                            0,
                            "%s\n\t(msgwnd->fadeIn) = %i",
                            "(msgwnd->fadeIn > 0)",
                            msgwnd->fadeIn) )
            {
                __debugbreak();
            }
            curalpha = (float)((float)(serverTime - (msgwnd->scrollTime - msgwnd->fadeIn + message->startTime))
                                             / (float)msgwnd->fadeIn)
                             * curalpha;
        }
    }
    else if ( msgwnd->fadeIn && serverTime - message->startTime < msgwnd->fadeIn )
    {
        curalpha = (float)((float)(serverTime - message->startTime) / (float)msgwnd->fadeIn) * curalpha;
    }
    if ( (float)(0.0 - curalpha) < 0.0 )
        return curalpha;
    else
        return 0.0f;
}

void __cdecl Con_DrawMessageWindowOldToNew(
                int localClientNum,
                MessageWindow *msgwnd,
                int x,
                int y,
                int charHeight,
                int horzAlign,
                int vertAlign,
                bool up,
                Font_s *font,
                float *color,
                int textStyle,
                float msgwndScale,
                char textAlignMode)
{
    clientActive_t *LocalClientGlobals; // [esp+30h] [ebp-34h]
    float finalColor[4]; // [esp+34h] [ebp-30h] BYREF
    Message *message; // [esp+44h] [ebp-20h]
    int lineOffset; // [esp+48h] [ebp-1Ch]
    int imod; // [esp+4Ch] [ebp-18h]
    int time; // [esp+50h] [ebp-14h]
    MessageLine *line; // [esp+54h] [ebp-10h]
    int v; // [esp+58h] [ebp-Ch]
    int groupsize; // [esp+5Ch] [ebp-8h]
    int serverTime; // [esp+60h] [ebp-4h]

    if ( !msgwnd
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 2938, 0, "%s", "msgwnd") )
    {
        __debugbreak();
    }
    serverTime = CL_GetLocalClientGlobals(localClientNum)->serverTime;
    Con_CullFinishedLines(serverTime, msgwnd);
    groupsize = 0;
    if ( zombietron->current.enabled )
        y += 80;
    v = y;
    if ( up )
        v -= charHeight;
    finalColor[0] = *color;
    finalColor[1] = color[1];
    finalColor[2] = color[2];
    finalColor[3] = color[3];
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    for ( lineOffset = 0; lineOffset < msgwnd->activeLineCount; ++lineOffset )
    {
        if ( msgwnd->lineCount <= 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        2960,
                        0,
                        "%s",
                        "msgwnd->lineCount > 0") )
        {
            __debugbreak();
        }
        imod = (lineOffset + msgwnd->firstLineIndex) % msgwnd->lineCount;
        if ( (unsigned int)imod >= msgwnd->lineCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        2962,
                        0,
                        "imod doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                        imod,
                        msgwnd->lineCount) )
        {
            __debugbreak();
        }
        line = &msgwnd->lines[imod];
        if ( line->messageIndex >= (unsigned int)msgwnd->lineCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        2964,
                        0,
                        "line->messageIndex doesn't index msgwnd->lineCount\n\t%i not in [0, %i)",
                        line->messageIndex,
                        msgwnd->lineCount) )
        {
            __debugbreak();
        }
        message = &msgwnd->messages[line->messageIndex];
        if ( Con_ShouldProcessMessage(message, serverTime) && LocalClientGlobals->serverTime <= message->endTime )
        {
            if ( LocalClientGlobals->serverTime > message->endTime - msgwnd->scrollTime )
            {
                time = LocalClientGlobals->serverTime - (message->endTime - msgwnd->scrollTime);
                if ( time > 0 )
                {
                    if ( msgwnd->scrollTime <= 0
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                                    2979,
                                    0,
                                    "%s\n\t(msgwnd->scrollTime) = %i",
                                    "(msgwnd->scrollTime > 0)",
                                    msgwnd->scrollTime) )
                    {
                        __debugbreak();
                    }
                    if ( up )
                        v += (int)((float)((float)((float)time / (float)msgwnd->scrollTime) * (float)charHeight)
                                         + 9.313225746154785e-10);
                    else
                        v -= (int)((float)((float)((float)time / (float)msgwnd->scrollTime) * (float)charHeight)
                                         + 9.313225746154785e-10);
                }
            }
            finalColor[3] = Con_GetMessageAlpha(message, msgwnd, LocalClientGlobals->serverTime, 0) * color[3];
            Con_DrawMessageLineOnHUD(
                localClientNum,
                &scrPlaceView[localClientNum],
                x,
                v,
                charHeight,
                horzAlign,
                vertAlign,
                font,
                msgwnd,
                imod,
                finalColor,
                textStyle,
                msgwndScale,
                textAlignMode);
            if ( up )
                v -= charHeight;
            else
                v += charHeight;
        }
    }
}

bool __cdecl CL_ShouldntDrawMessageWindow(int localClientNum)
{
    return CL_GetLocalClientGlobals(localClientNum)->snap.ps.pm_type != 5 && !CL_ShouldDisplayHud(localClientNum);
}

void __cdecl Con_DrawMiniConsole(int localClientNum, int xPos, int yPos, float alpha)
{
    Font_s *font; // [esp+Ch] [ebp-18h]
    float color[4]; // [esp+14h] [ebp-10h] BYREF

    font = UI_GetFontHandle(&scrPlaceView[localClientNum], 0, 1.0);
    if (con_miniconlines->current.integer > 0x64u
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
            3090,
            0,
            "%s\n\t(con_miniconlines->current.integer) = %i",
            "(con_miniconlines->current.integer >= 0 && con_miniconlines->current.integer <= 100)",
            con_miniconlines->current.integer))
    {
        __debugbreak();
    }
    if (con.messageBuffer[0].miniconWindow.lineCount != con_miniconlines->current.integer)
    {
        con.messageBuffer[0].miniconWindow.lineCount = con_miniconlines->current.integer;
        Con_ClearMiniConsole(localClientNum);
    }
    color[0] = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
    color[3] = alpha;
    Con_DrawMessageWindow(
        localClientNum,
        &con.messageBuffer[localClientNum].miniconWindow,
        xPos,
        yPos,
        12,
        1,
        1,
        font,
        color,
        3,
        1.0,
        MWM_BOTTOMUP_ALIGN_TOP,
        4);
}

void __cdecl Con_ClearMiniConsole(int localClientNum)
{
    Con_ClearMessageWindow(&con.messageBuffer[localClientNum].miniconWindow);
}

void __cdecl Con_DrawErrors(int localClientNum, int xPos, int yPos, float alpha)
{
    Font_s *font; // [esp+Ch] [ebp-14h]
    float color[4]; // [esp+10h] [ebp-10h] BYREF

    font = UI_GetFontHandle(&scrPlaceView[localClientNum], 0, 1.0);
    color[0] = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
    color[3] = alpha;
    Con_DrawMessageWindow(
        localClientNum,
        &con.messageBuffer[localClientNum].errorWindow,
        xPos,
        yPos,
        12,
        1,
        1,
        font,
        color,
        3,
        1.0,
        MWM_BOTTOMUP_ALIGN_TOP,
        4);
}

bool __cdecl Con_IsValidGameMessageWindow(unsigned int windowIndex)
{
    return windowIndex <= 2;
}

bool __cdecl Con_IsGameMessageWindowActive(int localClientNum, unsigned int windowIndex)
{
    if ( windowIndex >= 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    3125,
                    0,
                    "windowIndex doesn't index GAMEMSG_WINDOW_COUNT\n\t%i not in [0, %i)",
                    windowIndex,
                    3) )
    {
        __debugbreak();
    }
    return SLODWORD(con.color[4009 * localClientNum - 2462 + 13 * windowIndex]) > 0;
}

void __cdecl Con_DrawSay(int localClientNum, int x, int y)
{
    char *v3; // eax
    int v4; // eax
    int v5; // eax
    Font_s *font; // [esp+28h] [ebp-14h]
    char *string; // [esp+30h] [ebp-Ch]
    float normalizedScale; // [esp+38h] [ebp-4h]
    float normalizedScalea; // [esp+38h] [ebp-4h]

    if ( Key_IsCatcherActive(localClientNum, 32) )
    {
        if ( playerKeys[localClientNum].chat_team )
            v3 = SEH_SafeTranslateString("EXE_SAYTEAM");
        else
            v3 = SEH_SafeTranslateString("EXE_SAY");
        string = va("%s: ", v3);
        normalizedScale = playerKeys[localClientNum].chatField.charHeight / 48.0;
        font = UI_GetFontHandle(&scrPlaceView[localClientNum], 0, normalizedScale);
        normalizedScalea = R_NormalizedTextScale(font, normalizedScale);
        v4 = R_TextHeight(font);
        CL_DrawText(
            &scrPlaceView[localClientNum],
            string,
            0x7FFFFFFF,
            font,
            (float)x,
            (float)(y + (int)(float)((float)v4 * normalizedScalea)),
            1,
            1,
            normalizedScalea,
            normalizedScalea,
            colorWhite,
            3);
        v5 = R_TextWidth(string, 0, font);
        Field_Draw(
            localClientNum,
            &playerKeys[localClientNum].chatField,
            x + (int)(float)((float)v5 * normalizedScalea),
            y,
            1,
            1);
    }
}

void __cdecl Con_ToggleConsoleOutput()
{
    con.outputVisible = !con.outputVisible;
}

void __cdecl Con_DrawConsole(int localClientNum)
{
    Con_CheckResize();
    if ( Key_IsCatcherActive(localClientNum, 1) )
        Con_DrawSolidConsole(localClientNum);
}

void __cdecl Con_DrawSolidConsole(int localClientNum)
{
    Sys_EnterCriticalSection(CRITSECT_CONSOLE);
    if ( con.lineOffset )
        Con_Linefeed(localClientNum, con.prevChannel, 0);
    Sys_LeaveCriticalSection(CRITSECT_CONSOLE);
    if ( !Key_IsCatcherActive(localClientNum, 1) )
        con.outputVisible = 0;
    if ( con.outputVisible )
        Con_DrawOuputWindow();
    Con_DrawInput(localClientNum);
}


const float con_versionColor[4] = { 1.0, 1.0, 0.0, 1.0 };
const float con_inputDvarMatchColor[4] = { 1.0, 1.0, 0.8, 1.0 };
void __cdecl Con_DrawInput(int localClientNum)
{
    int BuildNumber; // eax
    const char *BuildName; // [esp-4h] [ebp-5Ch]
    bool v3; // [esp+10h] [ebp-48h]
    int matchCount; // [esp+40h] [ebp-18h]
    char *tooManyMatchesStr; // [esp+44h] [ebp-14h]
    int inputTextLenPrev; // [esp+48h] [ebp-10h]
    char *promptString; // [esp+4Ch] [ebp-Ch]
    const char *originalCommand; // [esp+54h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    2427,
                    0,
                    "%s",
                    "Sys_IsMainThread() || Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    if ( Key_IsCatcherActive(localClientNum, 1) )
    {
        BuildName = Com_GetBuildName();
        BuildNumber = Com_GetBuildNumber();
        promptString = va("%s.%s.%d: %s> ", "7", "0", BuildNumber, BuildName);
        conDrawInputGlob.fontHeight = (float)R_TextHeight(cls.consoleFont);
        conDrawInputGlob.x = con.screenMin[0] + 6.0;
        conDrawInputGlob.y = con.screenMin[1] + 6.0;
        conDrawInputGlob.leftX = con.screenMin[0] + 6.0;
        ConDrawInput_Box(1, &con_inputBoxColor->current.value);
        ConDrawInput_TextAndOver(promptString, con_versionColor);
        conDrawInputGlob.leftX = conDrawInputGlob.x;
        g_consoleField.widthInPixels = (int)(float)((float)(con.screenMax[0] - 6.0) - conDrawInputGlob.x);
        inputTextLenPrev = conDrawInputGlob.inputTextLen;
        conDrawInputGlob.inputText = Con_TokenizeInput();
        conDrawInputGlob.inputTextLen = strlen(conDrawInputGlob.inputText);
        conDrawInputGlob.autoCompleteChoice[0] = 0;
        if ( inputTextLenPrev != conDrawInputGlob.inputTextLen )
            Con_CancelAutoComplete();
        if ( conDrawInputGlob.inputTextLen
            && ((originalCommand = conDrawInputGlob.inputText, Cmd_Argc() <= 1)
             || !Con_IsDvarCommand(conDrawInputGlob.inputText)
                ? (v3 = 0)
                : (v3 = 1),
                    !v3
             || (conDrawInputGlob.inputText = Cmd_Argv(1),
                     (conDrawInputGlob.inputTextLen = strlen(conDrawInputGlob.inputText)) != 0)) )
        {
            if ( con_matchPrefixOnly->current.enabled )
            {
                conDrawInputGlob.hasExactMatch = 0;
                conDrawInputGlob.matchCount = 0;
                con_ignoreMatchPrefixOnly = 1;
                Con_ForEach(v3, ConDrawInput_IncrMatchCounter, ConDrawInput_IncrMatchCounter);
                if ( conDrawInputGlob.matchCount > con_inputMaxMatchesShown )
                {
                    conDrawInputGlob.hasExactMatch = 0;
                    conDrawInputGlob.matchCount = 0;
                    con_ignoreMatchPrefixOnly = 0;
                    Con_ForEach(0, ConDrawInput_IncrMatchCounter, ConDrawInput_IncrMatchCounter);
                    if ( !conDrawInputGlob.matchCount )
                    {
                        conDrawInputGlob.hasExactMatch = 0;
                        conDrawInputGlob.matchCount = 0;
                        con_ignoreMatchPrefixOnly = 1;
                        Con_ForEach(v3, ConDrawInput_IncrMatchCounter, ConDrawInput_IncrMatchCounter);
                    }
                }
            }
            else
            {
                conDrawInputGlob.hasExactMatch = 0;
                conDrawInputGlob.matchCount = 0;
                con_ignoreMatchPrefixOnly = 0;
                Con_ForEach(v3, ConDrawInput_IncrMatchCounter, ConDrawInput_IncrMatchCounter);
            }
            matchCount = conDrawInputGlob.matchCount;
            if ( conDrawInputGlob.matchCount )
            {
                if ( conDrawInputGlob.matchIndex >= conDrawInputGlob.matchCount || !conDrawInputGlob.autoCompleteChoice[0] )
                    conDrawInputGlob.matchIndex = -1;
                if ( conDrawInputGlob.matchIndex < 0 )
                    Con_DrawInputPrompt(localClientNum);
                else
                    Con_DrawAutoCompleteChoice(localClientNum, v3, originalCommand);
                conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
                conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
                conDrawInputGlob.x = conDrawInputGlob.leftX;
                if ( matchCount <= con_inputMaxMatchesShown )
                {
                    if ( matchCount == 1 || conDrawInputGlob.hasExactMatch && Con_AnySpaceAfterCommand() )
                    {
                        Con_ForEach(
                            v3,
                            (void (__cdecl *)(const char *))ConDrawInput_DetailedDvarMatch,
                            (void (__cdecl *)(const char *))ConDrawInput_DetailedCmdMatch);
                    }
                    else
                    {
                        ConDrawInput_Box(matchCount, &con_inputHintBoxColor->current.value);
                        Con_ForEach(
                            v3,
                            (void (__cdecl *)(const char *))ConDrawInput_DvarMatch,
                            (void (__cdecl *)(const char *))ConDrawInput_CmdMatch);
                    }
                }
                else
                {
                    tooManyMatchesStr = va(
                                                                "%i matches (too many to show here, press shift+tilde to open full console)",
                                                                matchCount);
                    ConDrawInput_Box(1, &con_inputHintBoxColor->current.value);
                    ConDrawInput_Text(tooManyMatchesStr, con_inputDvarMatchColor);
                }
                Cmd_EndTokenizedString();
            }
            else
            {
                Con_DrawInputPrompt(localClientNum);
                Cmd_EndTokenizedString();
            }
        }
        else
        {
            Con_AllowAutoCompleteCycling(0);
            Con_DrawInputPrompt(localClientNum);
            Cmd_EndTokenizedString();
        }
    }
}

void __cdecl ConDrawInput_Text(char *str, const float *color)
{
    if ( !str && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 1896, 0, "%s", "str") )
        __debugbreak();
    R_AddCmdDrawText(
        str,
        0x7FFFFFFF,
        cls.consoleFont,
        conDrawInputGlob.x,
        conDrawInputGlob.y + conDrawInputGlob.fontHeight,
        1.0,
        1.0,
        0.0,
        color,
        0);
}

void __cdecl ConDrawInput_TextAndOver(char *str, const float *color)
{
    if ( !str && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 1923, 0, "%s", "str") )
        __debugbreak();
    ConDrawInput_Text(str, color);
    conDrawInputGlob.x = (float)ConDrawInput_TextWidth(str) + conDrawInputGlob.x;
}

int __cdecl ConDrawInput_TextWidth(const char *text)
{
    if ( !text && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 1889, 0, "%s", "text") )
        __debugbreak();
    return R_TextWidth(text, 0, cls.consoleFont);
}

void __cdecl ConDrawInput_Box(int lines, const float *color)
{
    ConDraw_Box(
        conDrawInputGlob.x - 6.0,
        conDrawInputGlob.y - 6.0,
        (float)(con.screenMax[0] - con.screenMin[0]) - (float)((float)(conDrawInputGlob.x - 6.0) - con.screenMin[0]),
        (float)((float)lines * conDrawInputGlob.fontHeight) + (float)(2.0 * 6.0),
        color);
}

void __cdecl ConDraw_Box(float x, float y, float w, float h, const float *color)
{
    float darkColor[4]; // [esp+28h] [ebp-10h] BYREF

    if ( color[3] != 0.0 )
    {
        R_AddCmdDrawStretchPic(x, y, w, h, 0.0, 0.0, 0.0, 0.0, color, cls.whiteMaterial);
        darkColor[0] = 0.5 * *color;
        darkColor[1] = 0.5 * color[1];
        darkColor[2] = 0.5 * color[2];
        darkColor[3] = 0.5 * color[3];
        darkColor[3] = color[3];
        R_AddCmdDrawStretchPic(x, y, 2.0, h, 0.0, 0.0, 0.0, 0.0, darkColor, cls.whiteMaterial);
        R_AddCmdDrawStretchPic((float)(x + w) - 2.0, y, 2.0, h, 0.0, 0.0, 0.0, 0.0, darkColor, cls.whiteMaterial);
        R_AddCmdDrawStretchPic(x, y, w, 2.0, 0.0, 0.0, 0.0, 0.0, darkColor, cls.whiteMaterial);
        R_AddCmdDrawStretchPic(x, (float)(y + h) - 2.0, w, 2.0, 0.0, 0.0, 0.0, 0.0, darkColor, cls.whiteMaterial);
    }
}

void __cdecl ConDrawInput_IncrMatchCounter(const char *str)
{
    if ( Con_IsAutoCompleteMatch(str, conDrawInputGlob.inputText, conDrawInputGlob.inputTextLen) )
    {
        if ( conDrawInputGlob.matchCount == conDrawInputGlob.matchIndex )
            I_strncpyz(conDrawInputGlob.autoCompleteChoice, str, 64);
        ++conDrawInputGlob.matchCount;
        if ( !str[conDrawInputGlob.inputTextLen] )
            conDrawInputGlob.hasExactMatch = 1;
    }
}

const float con_inputDvarValueColor[4] = { 1.0, 1.0, 1.0, 1.0 };
void __cdecl ConDrawInput_DvarMatch(char *str)
{
    char *VariantString; // eax

    if ( !str && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 2128, 0, "%s", "str") )
        __debugbreak();
    if ( Con_IsAutoCompleteMatch(str, conDrawInputGlob.inputText, conDrawInputGlob.inputTextLen) )
    {
        ConDrawInput_TextLimitChars(str, 24, con_inputDvarMatchColor);
        conDrawInputGlob.x = conDrawInputGlob.x + 200.0;
        VariantString = (char *)Dvar_GetVariantString(str);
        ConDrawInput_TextLimitChars(VariantString, 40, con_inputDvarValueColor);
        conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
        conDrawInputGlob.x = conDrawInputGlob.leftX;
    }
}

void __cdecl ConDrawInput_TextLimitChars(char *str, int maxChars, const float *color)
{
    if ( !str && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 1903, 0, "%s", "str") )
        __debugbreak();
    R_AddCmdDrawText(
        str,
        maxChars,
        cls.consoleFont,
        conDrawInputGlob.x,
        conDrawInputGlob.y + conDrawInputGlob.fontHeight,
        1.0,
        1.0,
        0.0,
        color,
        0);
}

const float con_inputDvarInactiveValueColor[4] = { 0.8, 0.8, 0.8, 1.0 };
const float con_inputDvarDescriptionColor[4] = { 1.0, 1.0, 1.0, 1.0 };
const float con_inputDvarInfoColor[4] = { 0.8, 0.8, 1.0, 1.0 };
void __cdecl ConDrawInput_DetailedDvarMatch(char *str)
{
    char *v1; // eax
    char *v2; // eax
    char *v3; // eax
    bool hasLatchedValue; // [esp+Fh] [ebp-415h]
    int infoLineCount; // [esp+10h] [ebp-414h] BYREF
    char dvarInfo[1024]; // [esp+14h] [ebp-410h] BYREF
    int descriptionLineCount; // [esp+418h] [ebp-Ch]
    const dvar_s *dvar; // [esp+41Ch] [ebp-8h]
    int lineIndex; // [esp+420h] [ebp-4h]

    if ( !str && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 2253, 0, "%s", "str") )
        __debugbreak();
    if ( Con_IsAutoCompleteMatch(str, conDrawInputGlob.inputText, conDrawInputGlob.inputTextLen)
        && (!conDrawInputGlob.hasExactMatch || !str[conDrawInputGlob.inputTextLen]) )
    {
        dvar = Dvar_FindVar(str);
        if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 2261, 0, "%s", "dvar") )
            __debugbreak();
        hasLatchedValue = Dvar_HasLatchedValue(dvar);
        if ( hasLatchedValue )
            ConDrawInput_Box(3, &con_inputHintBoxColor->current.value);
        else
            ConDrawInput_Box(2, &con_inputHintBoxColor->current.value);
        ConDrawInput_TextLimitChars(str, 24, con_inputDvarMatchColor);
        conDrawInputGlob.x = conDrawInputGlob.x + 200.0;
        v1 = (char *)Dvar_DisplayableValue(dvar);
        ConDrawInput_TextLimitChars(v1, 40, con_inputDvarValueColor);
        conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
        conDrawInputGlob.x = conDrawInputGlob.leftX;
        if ( hasLatchedValue )
        {
            ConDrawInput_Text((char*)"    latched value", con_inputDvarInactiveValueColor);
            conDrawInputGlob.x = conDrawInputGlob.x + 200.0;
            v2 = (char *)Dvar_DisplayableLatchedValue(dvar);
            ConDrawInput_TextLimitChars(v2, 40, con_inputDvarInactiveValueColor);
            conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
            conDrawInputGlob.x = conDrawInputGlob.leftX;
        }
        ConDrawInput_Text((char *)"    default", con_inputDvarInactiveValueColor);
        conDrawInputGlob.x = conDrawInputGlob.x + 200.0;
        v3 = (char *)Dvar_DisplayableResetValue(dvar);
        ConDrawInput_TextLimitChars(v3, 40, con_inputDvarInactiveValueColor);
        conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
        conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
        conDrawInputGlob.x = conDrawInputGlob.leftX;
        Dvar_DomainToString_GetLines(dvar->type, dvar->domain, dvarInfo, 1024, &infoLineCount);
        if ( dvar->description )
            descriptionLineCount = ConDrawInput_GetDvarDescriptionLines(dvar);
        else
            descriptionLineCount = 0;
        ConDrawInput_Box(descriptionLineCount + infoLineCount + 1, &con_inputHintBoxColor->current.value);
        if ( dvar->description )
        {
            ConDrawInput_Text((char *)dvar->description, con_inputDvarDescriptionColor);
            for ( lineIndex = 0; lineIndex < descriptionLineCount; ++lineIndex )
            {
                conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
                conDrawInputGlob.x = conDrawInputGlob.leftX;
            }
        }
        ConDrawInput_Text(dvarInfo, con_inputDvarInfoColor);
        conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
        conDrawInputGlob.x = conDrawInputGlob.leftX;
        if ( dvar->type == DVAR_TYPE_ENUM && Cmd_Argc() == 2 )
            ConDrawInput_AutoCompleteArg(dvar->domain.enumeration.strings, dvar->domain.enumeration.stringCount);
    }
}

void __cdecl ConDrawInput_AutoCompleteArg(const char **stringList, int stringCount)
{
    int ArgChar; // eax
    Font_s *consoleFont; // [esp+10h] [ebp-4B4h]
    int matchIndex; // [esp+4Ch] [ebp-478h]
    signed int matchCount; // [esp+50h] [ebp-474h]
    char matchBuffer[1024]; // [esp+54h] [ebp-470h] BYREF
    int prefixLen; // [esp+45Ch] [ebp-68h]
    int matchLenMax; // [esp+460h] [ebp-64h]
    int matchLen; // [esp+464h] [ebp-60h]
    const char *prefix; // [esp+468h] [ebp-5Ch]
    char *matches[16]; // [esp+46Ch] [ebp-58h] BYREF
    int matchBufferUsed; // [esp+4ACh] [ebp-18h]
    int stringIndex; // [esp+4B0h] [ebp-14h]
    float x; // [esp+4B4h] [ebp-10h]
    float y; // [esp+4B8h] [ebp-Ch]
    float h; // [esp+4BCh] [ebp-8h]
    float w; // [esp+4C0h] [ebp-4h]

    prefix = Cmd_Argv(1);
    prefixLen = strlen(prefix);
    if ( prefixLen )
    {
        matchCount = 0;
        matchBufferUsed = 0;
        matchLenMax = 0;
        for ( stringIndex = 0; stringIndex < stringCount; ++stringIndex )
        {
            if ( !I_strnicmp(prefix, stringList[stringIndex], prefixLen) )
            {
                if ( matchCount == 16 || matchBufferUsed + strlen(stringList[stringIndex]) >= 0x400 )
                    return;
                matches[matchCount] = &matchBuffer[matchBufferUsed];
                Com_StripExtension((char *)stringList[stringIndex], matches[matchCount]);
                matchBufferUsed += strlen(matches[matchCount]) + 1;
                matchLen = ConDrawInput_TextWidth(matches[matchCount]);
                if ( matchLenMax < matchLen )
                    matchLenMax = matchLen;
                ++matchCount;
            }
        }
        if ( matchCount )
        {
            qsort(matches, matchCount, 4u, (int (__cdecl *)(const void *, const void *))ConDrawInput_CompareStrings);
            consoleFont = cls.consoleFont;
            ArgChar = ConDrawInput_TextFieldFirstArgChar();
            x = (float)((float)R_TextWidth(g_consoleField.buffer, ArgChar, consoleFont) + conDrawInputGlob.leftX) - 6.0;
            y = (float)(con.screenMin[1] + conDrawInputGlob.fontHeight) + 6.0;
            w = (float)matchLenMax + (float)(6.0 * 2.0);
            h = (float)((float)matchCount * conDrawInputGlob.fontHeight) + (float)(6.0 * 2.0);
            ConDraw_Box(x, y, w, h, &con_inputHintBoxColor->current.value);
            conDrawInputGlob.x = x + 6.0;
            conDrawInputGlob.y = y + 6.0;
            conDrawInputGlob.leftX = x + 6.0;
            for ( matchIndex = 0; matchIndex < matchCount; ++matchIndex )
            {
                ConDrawInput_Text(matches[matchIndex], con_inputDvarInfoColor);
                conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
                conDrawInputGlob.x = conDrawInputGlob.leftX;
            }
        }
    }
}

int __cdecl ConDrawInput_TextFieldFirstArgChar()
{
    int charIndex; // [esp+0h] [ebp-4h]

    for ( charIndex = 0; isspace(g_consoleField.buffer[charIndex]); ++charIndex )
        ;
    while ( !isspace(g_consoleField.buffer[charIndex]) )
        ++charIndex;
    while ( isspace(g_consoleField.buffer[charIndex]) )
        ++charIndex;
    return charIndex;
}

int __cdecl ConDrawInput_GetDvarDescriptionLines(const dvar_s *dvar)
{
    signed int v1; // kr00_4
    int linecount; // [esp+10h] [ebp-Ch]
    int index; // [esp+14h] [ebp-8h]

    if ( !dvar->description
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    2232,
                    0,
                    "%s",
                    "dvar->description") )
    {
        __debugbreak();
    }
    v1 = strlen(dvar->description);
    linecount = 1;
    for ( index = 0; index < v1; ++index )
    {
        if ( dvar->description[index] == 10 )
            ++linecount;
    }
    return linecount;
}

const float con_inputCommandMatchColor[4] = { 0.8, 0.8, 1.0, 1.0 };
void __cdecl ConDrawInput_DetailedCmdMatch(char *str)
{
    int fileCount; // [esp+8h] [ebp-8h] BYREF
    const char **files; // [esp+Ch] [ebp-4h]

    if ( !str && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 2322, 0, "%s", "str") )
        __debugbreak();
    if ( Con_IsAutoCompleteMatch(str, conDrawInputGlob.inputText, conDrawInputGlob.inputTextLen)
        && (!conDrawInputGlob.hasExactMatch || !str[conDrawInputGlob.inputTextLen]) )
    {
        ConDrawInput_Box(1, &con_inputHintBoxColor->current.value);
        ConDrawInput_Text(str, con_inputCommandMatchColor);
        conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
        conDrawInputGlob.x = conDrawInputGlob.leftX;
        if ( Cmd_Argc() == 2 )
        {
            files = Cmd_GetAutoCompleteFileList(str, &fileCount);
            if ( fileCount )
            {
                ConDrawInput_AutoCompleteArg(files, fileCount);
                FS_FreeFileList(files);
            }
        }
    }
}

void __cdecl ConDrawInput_CmdMatch(char *str)
{
    if ( !str && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 2349, 0, "%s", "str") )
        __debugbreak();
    if ( Con_IsAutoCompleteMatch(str, conDrawInputGlob.inputText, conDrawInputGlob.inputTextLen) )
    {
        ConDrawInput_Text(str, con_inputCommandMatchColor);
        conDrawInputGlob.y = conDrawInputGlob.y + conDrawInputGlob.fontHeight;
        conDrawInputGlob.x = conDrawInputGlob.leftX;
    }
}

void __cdecl Con_DrawAutoCompleteChoice(int localClientNum, bool isDvarCommand, const char *originalCommand)
{
    int drawLen; // [esp+10h] [ebp-114h]
    char colorCodedLine[256]; // [esp+14h] [ebp-110h] BYREF
    int cursorPos; // [esp+118h] [ebp-Ch]
    int x; // [esp+11Ch] [ebp-8h]
    int y; // [esp+120h] [ebp-4h]

    cursorPos = Con_GetAutoCompleteColorCodedString(
                                conDrawInputGlob.autoCompleteChoice,
                                conDrawInputGlob.inputText,
                                conDrawInputGlob.inputTextLen,
                                isDvarCommand,
                                originalCommand,
                                colorCodedLine);
    x = (int)conDrawInputGlob.x;
    y = (int)conDrawInputGlob.y;
    if ( &colorCodedLine[strlen(colorCodedLine) + 1] == &colorCodedLine[1]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    2371,
                    0,
                    "%s",
                    "strlen( colorCodedLine ) > 0") )
    {
        __debugbreak();
    }
    drawLen = SEH_PrintStrlen(colorCodedLine);
    if ( drawLen <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    2373,
                    0,
                    "%s\n\t(colorCodedLine) = %s",
                    "(drawLen > 0)",
                    colorCodedLine) )
    {
        __debugbreak();
    }
    Field_DrawTextOverride(localClientNum, &g_consoleField, x, y, 5, 5, colorCodedLine, drawLen, cursorPos);
}

unsigned int __cdecl Con_GetAutoCompleteColorCodedString(
                char *query,
                const char *matchToText,
                int matchTextLen,
                bool isDvarCommand,
                const char *originalCommand,
                char *colorCoded)
{
    unsigned int prefixLen; // [esp+0h] [ebp-4h]

    if ( isDvarCommand )
        prefixLen = sprintf(colorCoded, "^2%s ", originalCommand);
    else
        prefixLen = 0;
    if ( con_matchPrefixOnly->current.enabled )
        return prefixLen
                 + Con_GetAutoCompleteColorCodedStringContiguous(query, matchToText, matchTextLen, &colorCoded[prefixLen]);
    else
        return prefixLen
                 + Con_GetAutoCompleteColorCodedStringDiscontiguous(query, matchToText, matchTextLen, &colorCoded[prefixLen]);
}

int __cdecl Con_GetAutoCompleteColorCodedStringDiscontiguous(
                const char *query,
                const char *matchToText,
                int matchTextLen,
                char *colorCoded)
{
    int v4; // eax
    char v6; // [esp+13h] [ebp-29h]
    char *v7; // [esp+18h] [ebp-24h]
    const char *v8; // [esp+1Ch] [ebp-20h]
    const char *v9; // [esp+20h] [ebp-1Ch]
    int matchLetter; // [esp+24h] [ebp-18h]
    int colorCodedPos; // [esp+28h] [ebp-14h]
    int colorCodedPosb; // [esp+28h] [ebp-14h]
    int colorCodedPosa; // [esp+28h] [ebp-14h]
    char wasMatching; // [esp+2Fh] [ebp-Dh]
    int matchTextPos; // [esp+30h] [ebp-Ch]
    const char *queryPos; // [esp+34h] [ebp-8h]
    char isMatching; // [esp+3Bh] [ebp-1h]

    if ( !query && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 1975, 0, "%s", "query") )
        __debugbreak();
    if ( !matchToText
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 1976, 0, "%s", "matchToText") )
    {
        __debugbreak();
    }
    if ( !matchTextLen
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 1977, 0, "%s", "matchTextLen") )
    {
        __debugbreak();
    }
    wasMatching = 0;
    matchTextPos = 0;
    colorCodedPos = 0;
    matchLetter = tolower(*matchToText);
    for ( queryPos = query; *queryPos; ++queryPos )
    {
        v4 = tolower(*queryPos);
        isMatching = v4 == matchLetter;
        if ( v4 == matchLetter )
            matchLetter = tolower(matchToText[++matchTextPos]);
        if ( isMatching != wasMatching )
        {
            wasMatching = isMatching;
            if ( isMatching )
                v9 = "^2";
            else
                v9 = "^7";
            v8 = v9;
            v7 = &colorCoded[colorCodedPos];
            do
            {
                v6 = *v8;
                *v7++ = *v8++;
            }
            while ( v6 );
            colorCodedPos += 2;
        }
        colorCoded[colorCodedPos++] = *queryPos;
    }
    strcpy(&colorCoded[colorCodedPos], "^7");
    colorCodedPosb = colorCodedPos + 2;
    colorCoded[colorCodedPosb] = 32;
    colorCodedPosa = colorCodedPosb + 1;
    colorCoded[colorCodedPosa] = 0;
    if ( !strlen(colorCoded)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    2008,
                    0,
                    "%s",
                    "strlen( colorCoded ) > 0") )
    {
        __debugbreak();
    }
    return colorCodedPosa;
}

int __cdecl Con_GetAutoCompleteColorCodedStringContiguous(
                char *query,
                const char *matchToText,
                int matchTextLen,
                char *colorCoded)
{
    char v4; // al
    char v5; // cl
    char *v7; // [esp+18h] [ebp-54h]
    char *v8; // [esp+1Ch] [ebp-50h]
    char *v9; // [esp+48h] [ebp-24h]
    char *v10; // [esp+4Ch] [ebp-20h]
    int colorCodedPos; // [esp+60h] [ebp-Ch]
    int colorCodedPosb; // [esp+60h] [ebp-Ch]
    int colorCodedPosc; // [esp+60h] [ebp-Ch]
    int colorCodedPosa; // [esp+60h] [ebp-Ch]
    int colorCodedPosd; // [esp+60h] [ebp-Ch]
    char *queryPos; // [esp+64h] [ebp-8h]
    unsigned int len; // [esp+68h] [ebp-4h]

    if ( !query && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 2019, 0, "%s", "query") )
        __debugbreak();
    if ( !matchToText
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 2020, 0, "%s", "matchToText") )
    {
        __debugbreak();
    }
    if ( !matchTextLen
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 2021, 0, "%s", "matchTextLen") )
    {
        __debugbreak();
    }
    colorCodedPos = 0;
    queryPos = (char *)I_stristr(query, matchToText);
    if ( queryPos )
    {
        strncpy(colorCoded, query, queryPos - query);
        strcpy(&colorCoded[queryPos - query], "^2");
        colorCodedPosb = queryPos - query + 2;
        len = strlen(matchToText);
        strncpy(&colorCoded[colorCodedPosb], queryPos, len);
        colorCodedPosc = len + colorCodedPosb;
        strcpy(&colorCoded[colorCodedPosc], "^7");
        colorCodedPosa = colorCodedPosc + 2;
        v10 = &queryPos[len];
        v9 = &colorCoded[colorCodedPosa];
        do
        {
            v4 = *v10;
            *v9++ = *v10++;
        }
        while ( v4 );
        colorCodedPosd = strlen(&queryPos[len]) + colorCodedPosa;
        colorCoded[colorCodedPosd] = 32;
        colorCodedPos = colorCodedPosd + 1;
        colorCoded[colorCodedPos] = 0;
    }
    else
    {
        if ( !strlen(query)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        2046,
                        0,
                        "%s",
                        "strlen( query ) > 0") )
        {
            __debugbreak();
        }
        v8 = query;
        v7 = colorCoded;
        do
        {
            v5 = *v8;
            *v7++ = *v8++;
        }
        while ( v5 );
    }
    if ( !strlen(colorCoded)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    2050,
                    0,
                    "%s",
                    "strlen( colorCoded ) > 0") )
    {
        __debugbreak();
    }
    return colorCodedPos;
}

void __cdecl Con_DrawInputPrompt(int localClientNum)
{
    Field_Draw(localClientNum, &g_consoleField, (int)conDrawInputGlob.x, (int)conDrawInputGlob.y, 5, 5);
}

void Con_DrawOuputWindow()
{
    float width; // [esp+14h] [ebp-10h]
    float widtha; // [esp+14h] [ebp-10h]
    float height; // [esp+18h] [ebp-Ch]
    float heighta; // [esp+18h] [ebp-Ch]
    float x; // [esp+1Ch] [ebp-8h]
    float xa; // [esp+1Ch] [ebp-8h]
    float y; // [esp+20h] [ebp-4h]
    float ya; // [esp+20h] [ebp-4h]

    x = con.screenMin[0];
    width = con.screenMax[0] - con.screenMin[0];
    y = con.screenMin[1] + 32.0;
    height = (float)(con.screenMax[1] - con.screenMin[1]) - 32.0;
    ConDraw_Box(
        con.screenMin[0],
        con.screenMin[1] + 32.0,
        con.screenMax[0] - con.screenMin[0],
        height,
        &con_outputWindowColor->current.value);
    xa = x + 6.0;
    ya = y + 6.0;
    widtha = width - (float)(6.0 * 2.0);
    heighta = height - (float)(6.0 * 2.0);
    Con_DrawOutputVersion(xa, ya, widtha, heighta);
    Con_DrawOutputScrollBar(xa, ya, widtha, heighta);
    Con_DrawOutputText(xa, ya);
}

const float con_outputBarSize = 10.0f;
void __cdecl Con_DrawOutputScrollBar(float x, float y, float width, float height)
{
    float h; // [esp+14h] [ebp-28h]
    float v5; // [esp+18h] [ebp-24h]
    float v6; // [esp+1Ch] [ebp-20h]
    float v7; // [esp+28h] [ebp-14h]
    float v8; // [esp+2Ch] [ebp-10h]
    float scale; // [esp+34h] [ebp-8h]
    float xa; // [esp+44h] [ebp+8h]

    xa = (float)(x + width) - 10.0;
    ConDraw_Box(xa, y, 10.0, height, &con_outputBarColor->current.value);
    if ( con.consoleWindow.activeLineCount > con.visibleLineCount )
    {
        scale = 1.0 / (float)(con.consoleWindow.activeLineCount - con.visibleLineCount);
        v7 = (float)(con.displayLineOffset - con.visibleLineCount) * scale;
        if ( (float)(v7 - 1.0) < 0.0 )
            v8 = (float)(con.displayLineOffset - con.visibleLineCount) * scale;
        else
            v8 = 1.0f;
        if ( (float)(0.0 - v7) < 0.0 )
            v6 = v8;
        else
            v6 = 0.0f;
        v5 = ceil((float)((float)((float)con.visibleLineCount * scale) * height));
        if ( (float)(10.0 - v5) < 0.0 )
            h = v5;
        else
            h = con_outputBarSize;
        ConDraw_Box(
            xa,
            (float)((float)((float)((float)(y + height) - h) - y) * v6) + y,
            10.0,
            h,
            &con_outputSliderColor->current.value);
    }
    else
    {
        ConDraw_Box(xa, y, 10.0, height, &con_outputSliderColor->current.value);
    }
}

void __cdecl Con_DrawOutputText(float x, float y)
{
    int rowCount; // [esp+18h] [ebp-24h]
    int firstRow; // [esp+1Ch] [ebp-20h]
    float color[4]; // [esp+24h] [ebp-18h] BYREF
    int lineIndex; // [esp+34h] [ebp-8h]
    int rowIndex; // [esp+38h] [ebp-4h]

    CL_LookupColor(0x37u, color);
    if ( !con.fontHeight
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 3211, 0, "%s", "con.fontHeight") )
    {
        __debugbreak();
    }
    rowCount = con.visibleLineCount;
    firstRow = con.displayLineOffset - con.visibleLineCount;
    if ( con.displayLineOffset - con.visibleLineCount < 0 )
    {
        y = y - (float)(con.fontHeight * firstRow);
        rowCount = con.displayLineOffset;
        firstRow = 0;
    }
    for ( rowIndex = 0; rowIndex < rowCount; ++rowIndex )
    {
        lineIndex = (rowIndex + firstRow + con.consoleWindow.firstLineIndex) % con.consoleWindow.lineCount;
        y = (float)con.fontHeight + y;
        R_AddCmdDrawConsoleText(
            con.consoleWindow.circularTextBuffer,
            con.consoleWindow.textBufSize,
            con.consoleWindow.lines[lineIndex].textBufPos,
            con.consoleWindow.lines[lineIndex].textBufSize,
            cls.consoleFont,
            x,
            y,
            1.0,
            1.0,
            color,
            0);
    }
}

void __cdecl Con_DrawOutputVersion(float x, float y, float width, float height)
{
    char *versionString; // [esp+0h] [ebp-4h]

    versionString = Con_GetVersionString();
    SCR_DrawSmallStringExt((int)x, (int)(float)((float)(height - 16.0) + y), versionString, con_versionColor);
}

char *__cdecl Con_GetVersionString()
{
    int BuildNumber; // eax

    BuildNumber = Com_GetBuildNumber();
    return va("Build %d %s", BuildNumber, "win-x86");
}

void __cdecl Con_PageUp()
{
    int activeLineCount; // [esp+0h] [ebp-4h]

    con.displayLineOffset -= 2;
    if ( con.displayLineOffset < con.visibleLineCount )
    {
        if ( con.consoleWindow.activeLineCount < con.visibleLineCount )
            activeLineCount = con.consoleWindow.activeLineCount;
        else
            activeLineCount = con.visibleLineCount;
        con.displayLineOffset = activeLineCount;
    }
}

void __cdecl Con_PageDown()
{
    int activeLineCount; // [esp+0h] [ebp-8h]

    if ( con.displayLineOffset + 2 < con.consoleWindow.activeLineCount )
        activeLineCount = con.displayLineOffset + 2;
    else
        activeLineCount = con.consoleWindow.activeLineCount;
    con.displayLineOffset = activeLineCount;
}

void __cdecl Con_Top()
{
    int activeLineCount; // [esp+0h] [ebp-4h]

    if ( con.consoleWindow.activeLineCount < con.visibleLineCount )
        activeLineCount = con.consoleWindow.activeLineCount;
    else
        activeLineCount = con.visibleLineCount;
    con.displayLineOffset = activeLineCount;
}

void __cdecl Con_Bottom()
{
    con.displayLineOffset = con.consoleWindow.activeLineCount;
}

void __cdecl Con_Close(int localClientNum)
{
    int client; // [esp+0h] [ebp-4h]

    if (CL_LocalClient_IsCUIFlagSet(localClientNum, 2))
    {
        Field_Clear(&g_consoleField);
        Con_CancelAutoComplete();
        Con_ClearNotify(localClientNum);
        Con_ClearMiniConsole(localClientNum);
        Con_ClearErrors(localClientNum);
        for (client = 0; client < 1; ++client)
            clientUIActives[client].keyCatchers &= ~1u;
    }
}

bool __cdecl Con_IsActive(int localClientNum)
{
    return Key_IsCatcherActive(localClientNum, 1);
}

void __cdecl CL_PlayTextFXPulseSounds(
                int localClientNum,
                int currentTime,
                int strLength,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int *soundTimeKeeper)
{
    SndEntHandle v7; // [esp+24h] [ebp-1Ch]
    SndEntHandle entHandle; // [esp+28h] [ebp-18h]
    int timeElapsed; // [esp+34h] [ebp-Ch]
    int lastSoundTime; // [esp+38h] [ebp-8h]
    int decayStartTime; // [esp+3Ch] [ebp-4h]

    timeElapsed = currentTime - fxBirthTime;
    lastSoundTime = *soundTimeKeeper - fxBirthTime;
    decayStartTime = fxDecayStartTime;
    if ( fxDecayStartTime < fxLetterTime * strLength )
        decayStartTime = fxLetterTime * strLength;
    if ( timeElapsed >= 0 )
    {
        if ( timeElapsed <= decayStartTime )
        {
            if ( timeElapsed < fxLetterTime * strLength )
            {
                if ( !fxLetterTime
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                                3404,
                                0,
                                "%s",
                                "fxLetterTime") )
                {
                    __debugbreak();
                }
                if ( lastSoundTime < fxLetterTime * (timeElapsed / fxLetterTime) )
                {
                    v7.field = SND_EntHandle(localClientNum, 4095, 0, 0, 1, TEAM_FREE).field;
                    SND_Play((char *)"uin_pulse_text_type", 0, 1.0, v7, 0, 0, 0);
                    *soundTimeKeeper = currentTime;
                }
            }
        }
        else if ( lastSoundTime < decayStartTime )
        {
            entHandle.field = SND_EntHandle(localClientNum, 4095, 0, 0, 1, TEAM_FREE).field;
            SND_Play((char*)"uin_pulse_text_delete", 0, 1.0, entHandle, 0, 0, 0);
            *soundTimeKeeper = currentTime;
        }
    }
}

void __cdecl Con_Restricted_Add(const char *name)
{
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    3482,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp", 3483, 0, "%s", "name") )
        __debugbreak();
    if ( Con_Restricted_SigleExist(name) == -1 )
    {
        if ( g_restricted_count >= 1024 )
        {
            Com_DPrintf(16, "ConAccess: Couldn't find cmd: %s: RESTRICTED_MAX_LIST.\n", name);
        }
        else if ( strlen(name) >= 0x40 )
        {
            Com_DPrintf(16, "ConAccess: Couldn't find cmd: %s: RESTRICTED_MAX_NAME_LENGTH.\n", name);
        }
        else
        {
            I_strncpyz(g_restricted[g_restricted_count].name, name, 64);
            g_restricted[g_restricted_count++].valid = 1;
        }
    }
}

int __cdecl Con_Restricted_SigleExist(const char *name)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < g_restricted_count; ++i )
    {
        if ( !I_strcmp(g_restricted[i].name, name) )
            return i;
    }
    return -1;
}

void __cdecl Con_Restricted_SetState(e_restricted_initState state)
{
    if ( (unsigned int)state >= RESTRICTED_STATE_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    3528,
                    0,
                    "state doesn't index RESTRICTED_STATE_COUNT\n\t%i not in [0, %i)",
                    state,
                    3) )
    {
        __debugbreak();
    }
    g_restricted_state = state;
}

void __cdecl Con_Restricted_ShutDown()
{
    memset((unsigned __int8 *)g_restricted, 0, sizeof(g_restricted));
    g_restricted_count = 0;
    g_restricted_ranked = 0;
    Con_Restricted_Reset();
    Con_Restricted_SetState(RESTRICTED_DISABLED);
}

e_restricted_initState __cdecl Con_Restricted_GetState()
{
    return g_restricted_state;
}

void __cdecl Con_Restricted_SecureConfigs()
{
    if ( Con_Restricted_GetState() == RESTRICTED_DISABLED )
        Con_Restricted_SetState(RESTRICTED_CONFIGURING);
}

void __cdecl Con_Restricted_InitLists()
{
    int LicenseType; // eax
    int v1; // eax
    const StringTable *table; // [esp+4h] [ebp-4h] BYREF

    Con_Restricted_ShutDown();
    LicenseType = SV_GetLicenseType();
    if ( SV_IsServerRanked(LicenseType) )
        StringTable_GetAsset("mp/devconsole_restrict_access_dedicated_ranked_mp.csv", (XAssetHeader *)&table);
    else
        StringTable_GetAsset("mp/devconsole_restrict_access_dedicated_mp.csv", (XAssetHeader *)&table);
    if ( table )
    {
        Con_Restricted_LoadTable(table);
        v1 = SV_GetLicenseType();
        g_restricted_ranked = SV_IsServerRanked(v1);
        Con_Restricted_SetState(RESTRICTED_PROTECTED);
    }
    else if ( Dvar_GetBool("con_restricted_access") )
    {
        Com_DPrintf(16, "ConAccess: the table asset was not found.\n");
    }
}

void __cdecl Con_Restricted_LoadTable(const StringTable *table)
{
    int row; // [esp+0h] [ebp-8h]
    const char *name; // [esp+4h] [ebp-4h]

    if ( table )
    {
        for ( row = 0; row < table->rowCount; ++row )
        {
            name = StringTable_GetColumnValueForRow(table, row, 0);
            Con_Restricted_Add(name);
        }
    }
    else if ( Dvar_GetBool("con_restricted_access") )
    {
        Com_DPrintf(16, "ConAccess: no table loaded.\n");
    }
}

void __cdecl Con_Restricted_ResetLists()
{
    bool v0; // esi
    int LicenseType; // eax

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                    3633,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( Dvar_GetBool("con_restricted_access") )
    {
        if ( Con_Restricted_GetState() != RESTRICTED_PROTECTED
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_console.cpp",
                        3640,
                        0,
                        "%s",
                        "Con_Restricted_GetState() == RESTRICTED_PROTECTED") )
        {
            __debugbreak();
        }
        if ( Con_Restricted_GetState() < RESTRICTED_PROTECTED )
            Con_Restricted_InitLists();
        v0 = g_restricted_ranked;
        LicenseType = SV_GetLicenseType();
        if ( v0 != SV_IsServerRanked(LicenseType) )
            Con_Restricted_InitLists();
        Con_Restricted_Reset();
        Con_Restricted_SetLists();
    }
}

void Con_Restricted_SetLists()
{
    int i; // [esp+0h] [ebp-8h]
    const dvar_s *dvar; // [esp+4h] [ebp-4h]

    for ( i = 0; i < g_restricted_count; ++i )
    {
        if ( g_restricted[i].valid )
        {
            dvar = Dvar_FindVar(g_restricted[i].name);
            if ( dvar )
            {
                Dvar_AddFlags(dvar, 0x10000);
            }
            else if ( !Cmd_SetConAccess(g_restricted[i].name) && Dvar_GetBool("con_restricted_access") )
            {
                Com_DPrintf(16, "ConAccess: Couldn't find: %s.\n", g_restricted[i].name);
            }
        }
    }
}

void Con_Restricted_Reset()
{
    Dvar_ResetConAccessDvars();
    Cmd_ResetConAccessCommands();
}

void __cdecl Con_Restricted_ForEach(
                bool isDvarCommand,
                void (__cdecl *callbackDvar)(const char *),
                void (__cdecl *callbackCmd)(const char *))
{
    Dvar_ForEachConsoleAccessName(callbackDvar);
    if ( !isDvarCommand )
        Cmd_ForEachConsoleAccessName(callbackCmd);
}

void __cdecl Con_Restricted_ExecuteBuf(int localClientNum, int controllerIndex, char *buffer)
{
    if ( !Dvar_GetBool("con_restricted_access") || Con_Restricted_GetState() == RESTRICTED_CONFIGURING )
    {
        Cbuf_ExecuteBuffer(localClientNum, controllerIndex, buffer);
    }
    else
    {
        Con_Restricted_ResetLists();
        Cbuf_ExecuteBuffer_Restricted(localClientNum, controllerIndex, buffer);
    }
}

void __cdecl Con_Restricted_AddBuf(char *buf)
{
    int start; // [esp+0h] [ebp-100Ch]
    char buffer[4096]; // [esp+4h] [ebp-1008h] BYREF
    int length; // [esp+1008h] [ebp-4h]

    if ( Dvar_GetBool("con_restricted_access") )
    {
        Con_Restricted_ResetLists();
        //length = strlen_noncrt(buf);
        length = strlen(buf);
        start = 0;
        do
        {
            start = Con_Restricted_RestrictBuf(buf, start, length, buffer, 4096);
            Cbuf_AddText(0, buffer);
            Cbuf_AddText(0, "\n");
        }
        while ( start < length );
    }
    else
    {
        Cbuf_AddText(0, buf);
        Cbuf_AddText(0, "\n");
    }
}

int __cdecl Con_Restricted_RestrictBuf(const char *buf, int start, int length, char *buffer, int buffer_size)
{
    char quotes; // [esp+0h] [ebp-Ch]
    int size; // [esp+4h] [ebp-8h]

    quotes = 0;
    memset((unsigned __int8 *)buffer, 0, buffer_size);
    I_strncat(buffer, buffer_size, "#dcr#");
    size = 5;
    while ( start < length )
    {
        if ( buf[start] == 34 )
            ++quotes;
        if ( buf[start] == 10 || buf[start] == 13 || size + 1 >= buffer_size )
            break;
        buffer[size++] = buf[start];
        if ( (quotes & 1) == 0 && buf[start] == 59 )
        {
            if ( size + 5 >= buffer_size )
                return start;
            I_strncat(buffer, buffer_size, "#dcr#");
            size += 5;
        }
        ++start;
    }
    return start;
}

char __cdecl Con_Restricted_ValidateConfig(const char *fileName)
{
    char *text; // [esp+4h] [ebp-4h] BYREF

    if ( FS_ReadFile(fileName, (void **)&text) < 0 )
        return 0;
    FS_FreeFile(text);
    return 1;
}

int __cdecl ConDrawInput_CompareStrings(const char **e0, const char **e1)
{
    return I_stricmp(*e0, *e1);
}

