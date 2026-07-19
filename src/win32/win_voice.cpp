#include "win_voice.h"
#include <qcommon/common.h>
#include <Windows.h>
#include <mmsystem.h>
#include <cstring>
#include <groupvoice/encode.h>
#include <groupvoice/record.h>
#include <groupvoice/decode.h>
#include <groupvoice/play.h>
#include <client/cl_main.h>
#include "win_shared.h"
#include <live/live_sessions_win.h>

const dvar_t *winvoice_mic_mute;
const dvar_t *winvoice_mic_reclevel;
const dvar_t *winvoice_save_voice;
const dvar_t *winvoice_mic_scaler;

char old_rec_source[256];
int mic_old_reclevel;
int mic_current_reclevel;
dsound_sample_t *s_clientSamples[32];
int s_clientTalkTime[32];
int g_voice_initialized;
float levelSamples[6];
int sampleCount;
bool recording;
float voice_current_scaler = 1.0f;


bool __cdecl Voice_Init()
{
    HWND__ *handle; // [esp+1Ch] [ebp-8h]
    int client; // [esp+20h] [ebp-4h]

    Com_Printf(16, "--- Initializing Voice ---\n");
    winvoice_mic_mute = _Dvar_RegisterBool("winvoice_mic_mute", 1, 1u, "Mute the microphone");
    winvoice_mic_reclevel = _Dvar_RegisterFloat(
                                                        "winvoice_mic_reclevel",
                                                        65535.0,
                                                        0.0,
                                                        65535.0,
                                                        1u,
                                                        "Microphone recording level");
    winvoice_save_voice = _Dvar_RegisterBool("winvoice_save_voice", 0, 1u, "Write voice data to a file");
    winvoice_mic_scaler = _Dvar_RegisterFloat("winvoice_mic_scaler", 1.0, 0.25, 2.0, 1u, "Microphone scaler value");
    if ( !mixerGetRecordSource(old_rec_source) )
        strcpy(old_rec_source, "microphone-not-present");
    mixerSetRecordSource(old_rec_source);
    mic_old_reclevel = mixerGetRecordLevel(old_rec_source);
    mic_current_reclevel = (unsigned __int16)(int)winvoice_mic_reclevel->current.value;
    mixerSetRecordLevel(old_rec_source, (int)winvoice_mic_reclevel->current.value);
    mixerSetMicrophoneMute(winvoice_mic_mute->current.color[0]);
    g_current_bandwidth_setting = 0;
    handle = GetDesktopWindow();
    Record_Init();
    Decode_Init(g_current_bandwidth_setting);
    Encode_Init(g_current_bandwidth_setting);
    if ( Sound_Init(handle) )
        g_voice_initialized = 1;
    memset((unsigned __int8 *)s_clientTalkTime, 0, sizeof(s_clientTalkTime));
    for ( client = 0; client < 32; ++client )
        s_clientSamples[client] = Sound_NewSample();
    return 0;
}

void __cdecl Voice_StopClientSamples()
{
    int client; // [esp+0h] [ebp-4h]

    for ( client = 0; client < 32; ++client )
        Sound_StopSample(s_clientSamples[client]);
}

void __cdecl Voice_Shutdown()
{
    int client; // [esp+0h] [ebp-4h]

    if ( g_voice_initialized )
    {
        for ( client = 0; client < 32; ++client )
            Sound_DestroySample(s_clientSamples[client]);
        Voice_StopRecording();
        Record_Shutdown();
        Encode_Shutdown();
        Decode_Shutdown();
        Sound_Shutdown();
        mixerSetMicrophoneMute(1u);
        mixerSetRecordLevel(old_rec_source, mic_old_reclevel);
        mixerSetRecordSource(old_rec_source);
        g_voice_initialized = 0;
    }
}

double __cdecl Voice_GetVoiceLevel()
{
    int ii; // [esp+0h] [ebp-8h]
    float avgLvl; // [esp+4h] [ebp-4h]

    if ( !g_voice_initialized )
        return 0.0;
    levelSamples[sampleCount % 6] = (float)(voice_current_voicelevel / 32767.0) / 6.0;
    ++sampleCount;
    avgLvl = 0.0f;
    for ( ii = 0; ii < 6; ++ii )
        avgLvl = avgLvl + levelSamples[ii];
    return avgLvl;
}

void __cdecl Voice_Playback()
{
    int client; // [esp+0h] [ebp-4h]

    if ( g_voice_initialized )
    {
        if ( mic_current_reclevel != (unsigned __int16)(int)winvoice_mic_reclevel->current.value )
        {
            mic_current_reclevel = (unsigned __int16)(int)winvoice_mic_reclevel->current.value;
            mixerSetRecordLevel(old_rec_source, (int)winvoice_mic_reclevel->current.value);
        }
        for ( client = 0; client < 32; ++client )
            Sound_SampleFrame(s_clientSamples[client]);
        CL_RegisterDvars();
    }
}

int __cdecl Voice_GetLocalVoiceData()
{
    if ( !g_voice_initialized )
        return 0;
    if ( !recording )
        Voice_StartRecording();
    if ( voice_current_scaler != winvoice_mic_scaler->current.value )
    {
        voice_current_scaler = winvoice_mic_scaler->current.value;
        if ( voice_current_scaler >= 0.5 )
        {
            if ( voice_current_scaler > 1.5 )
                voice_current_scaler = 1.5f;
        }
        else
        {
            voice_current_scaler = 0.5f;
        }
    }
    Record_Frame();
    return 0;
}

bool playing;
void __cdecl Voice_IncomingVoiceData(unsigned __int8 talker, unsigned __int8 *data, int packetDataSize)
{
    int v3; // [esp+0h] [ebp-201Ch]
    _iobuf *stream; // [esp+8h] [ebp-2014h]
    int v5; // [esp+10h] [ebp-200Ch]
    __int16 out[4096]; // [esp+14h] [ebp-2008h] BYREF
    unsigned int data_len; // [esp+2018h] [ebp-4h]

    if ( g_voice_initialized )
    {
        if ( !playing )
            playing = 1;
        data_len = 0;
        v5 = 0;
        if ( talker >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_voice.cpp",
                        243,
                        0,
                        "%s\n\t(talker) = %i",
                        "(talker >= 0 && talker < 32)",
                        talker) )
        {
            __debugbreak();
        }
        s_clientTalkTime[talker] = Sys_Milliseconds();
        while ( v5 < packetDataSize )
        {
            if ( packetDataSize - v5 < g_decode_frame_size )
                v3 = packetDataSize - v5;
            else
                v3 = g_decode_frame_size;
            data_len = Decode_Sample((char *)&data[v5], v3, out, g_decode_frame_size);
            if ( (int)data_len > 0 )
                Sound_UpdateSample(s_clientSamples[talker], (char *)out, data_len);
            v5 += v3;
        }
        if ( winvoice_save_voice->current.enabled )
        {
            stream = fopen("voice.wav", "ab");
            if ( stream )
            {
                fwrite(out, data_len, 1u, stream);
                fclose(stream);
            }
        }
    }
}

bool __cdecl Voice_IsClientTalking(unsigned int clientNum)
{
    if ( clientNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_voice.cpp",
                    272,
                    0,
                    "%s\n\t(clientNum) = %i",
                    "(clientNum >= 0 && clientNum < 32)",
                    clientNum) )
    {
        __debugbreak();
    }
    return (int)(Sys_Milliseconds() - s_clientTalkTime[clientNum]) < 300;
}

dsound_sample_t *currentRecordingSample;
int count_0;
char __cdecl Voice_StartRecording()
{
    if ( !recording )
    {
        currentRecordingSample = Record_NewSample();
        Record_Start(currentRecordingSample);
        recording = 1;
        ++count_0;
    }
    return 1;
}

char __cdecl Voice_StopRecording()
{
    if ( !recording )
        return 0;
    Record_Stop(currentRecordingSample);
    Record_DestroySample(currentRecordingSample);
    recording = 0;
    return 1;
}

unsigned int __cdecl mixerGetRecordLevel(char *SrcName)
{
    tagMIXERCONTROLA mxc; // [esp+0h] [ebp-188h] BYREF
    unsigned int jj; // [esp+98h] [ebp-F0h]
    tagMIXERLINECONTROLSA mxlc; // [esp+9Ch] [ebp-ECh] BYREF
    HMIXER__ *phmx; // [esp+B8h] [ebp-D0h] BYREF
    unsigned int ii; // [esp+BCh] [ebp-CCh]
    tagMIXERLINEA mixerline; // [esp+C0h] [ebp-C8h] BYREF
    tMIXERCONTROLDETAILS_UNSIGNED newSetting; // [esp+16Ch] [ebp-1Ch] BYREF
    tMIXERCONTROLDETAILS mxcd; // [esp+170h] [ebp-18h] BYREF

    if ( !waveInGetNumDevs() )
        return -1;
    if ( !mixerGetNumDevs() )
        return -1;
    if ( mixerOpen(&phmx, 0, 0, 0, 0) )
        return -1;
    memset((unsigned __int8 *)&mixerline, 0, sizeof(mixerline));
    mixerline.cbStruct = 168;
    mixerline.dwComponentType = 7;
    mixerGetLineInfoA((HMIXEROBJ)phmx, &mixerline, 3u);
    jj = mixerline.cConnections;
    for ( ii = 0; ii < jj; ++ii )
    {
        mixerline.dwSource = ii;
        mixerGetLineInfoA((HMIXEROBJ)phmx, &mixerline, 1u);
        if ( !I_stricmp(mixerline.szName, SrcName) )
        {
            mxlc.cbStruct = 24;
            mxlc.dwLineID = mixerline.dwLineID;
            mxlc.dwControlID = 1342373889;
            mxlc.cControls = 1;
            mxlc.cbmxctrl = 148;
            mxlc.pamxctrl = &mxc;
            if ( !mixerGetLineControlsA((HMIXEROBJ)phmx, &mxlc, 2u) )
            {
                mxcd.cbStruct = 24;
                mxcd.cChannels = 1;
                mxcd.cbDetails = 4;
                mxcd.paDetails = &newSetting;
                mxcd.cMultipleItems = 0;
                mxcd.dwControlID = mxc.dwControlID;
                mixerGetControlDetailsA((HMIXEROBJ)phmx, &mxcd, 0);
                Com_Printf(16, "Line Control %3d: %22s", mxc.dwControlID, mxlc.pamxctrl->szName);
                Com_Printf(
                    16,
                    "\tcur: %6d\tbounds (%6d-%6d)\n",
                    newSetting.dwValue,
                    mxlc.pamxctrl->Bounds.lMinimum,
                    mxlc.pamxctrl->Bounds.lMaximum);
                mixerClose(phmx);
                return newSetting.dwValue;
            }
            break;
        }
    }
    mixerClose(phmx);
    return -1;
}

int __cdecl mixerSetRecordLevel(char *SrcName, unsigned __int16 newLevel)
{
    tagMIXERCONTROLA mxc; // [esp+0h] [ebp-188h] BYREF
    unsigned int jj; // [esp+98h] [ebp-F0h]
    tagMIXERLINECONTROLSA mxlc; // [esp+9Ch] [ebp-ECh] BYREF
    unsigned int err; // [esp+B4h] [ebp-D4h]
    HMIXER__ *phmx; // [esp+B8h] [ebp-D0h] BYREF
    unsigned int ii; // [esp+BCh] [ebp-CCh]
    tagMIXERLINEA mixerline; // [esp+C0h] [ebp-C8h] BYREF
    tMIXERCONTROLDETAILS_UNSIGNED newSetting; // [esp+16Ch] [ebp-1Ch] BYREF
    tMIXERCONTROLDETAILS mxcd; // [esp+170h] [ebp-18h] BYREF

    if ( !waveInGetNumDevs() )
        return 0;
    if ( !mixerGetNumDevs() )
        return 0;
    if ( mixerOpen(&phmx, 0, 0, 0, 0) )
        return 0;
    mixerline.cbStruct = 168;
    mixerline.dwComponentType = 7;
    if ( mixerGetLineInfoA((HMIXEROBJ)phmx, &mixerline, 3u) )
        return 0;
    jj = mixerline.cConnections;
    for ( ii = 0; ii < jj; ++ii )
    {
        mixerline.dwSource = ii;
        if ( !mixerGetLineInfoA((HMIXEROBJ)phmx, &mixerline, 1u) && !I_stricmp(mixerline.szName, SrcName) )
        {
            mxlc.cbStruct = 24;
            mxlc.dwLineID = mixerline.dwLineID;
            mxlc.dwControlID = 1342373889;
            mxlc.cControls = 1;
            mxlc.cbmxctrl = 148;
            mxlc.pamxctrl = &mxc;
            err = mixerGetLineControlsA((HMIXEROBJ)phmx, &mxlc, 2u);
            if ( !err )
            {
                mxcd.cbStruct = 24;
                mxcd.cChannels = 1;
                mxcd.cbDetails = 4;
                mxcd.paDetails = &newSetting;
                mxcd.cMultipleItems = 0;
                mxcd.dwControlID = mxc.dwControlID;
                err = mixerGetControlDetailsA((HMIXEROBJ)phmx, &mxcd, 0);
                if ( !err )
                {
                    newSetting.dwValue = newLevel;
                    mixerSetControlDetails((HMIXEROBJ)phmx, &mxcd, 0);
                }
            }
            break;
        }
    }
    mixerClose(phmx);
    return 0;
}

int __cdecl mixerGetRecordSource(char *srcName)
{
    int jj; // [esp+4h] [ebp-100h]
    tagMIXERLINECONTROLSA mxlc; // [esp+8h] [ebp-FCh] BYREF
    unsigned int err; // [esp+20h] [ebp-E4h]
    int iNumChannels; // [esp+24h] [ebp-E0h]
    int iMultipleItems; // [esp+28h] [ebp-DCh]
    tMIXERCONTROLDETAILS_BOOLEAN *lpListBool; // [esp+30h] [ebp-D4h]
    int ii; // [esp+34h] [ebp-D0h]
    tagMIXERCONTROLDETAILS_LISTTEXTA *lpListText; // [esp+38h] [ebp-CCh]
    tagMIXERLINEA mixerline; // [esp+3Ch] [ebp-C8h] BYREF
    tMIXERCONTROLDETAILS mxcd; // [esp+E8h] [ebp-1Ch] BYREF
    tagMIXERCONTROLA *lpmxc; // [esp+100h] [ebp-4h]

    HMIXER mixerHandle;

    if ( !waveInGetNumDevs() )
        return 0;
    if ( !mixerGetNumDevs() )
        return 0;
    if ( mixerOpen(&mixerHandle, 0, 0, 0, 0) )
        return 0;

    HMIXEROBJ phmx = (HMIXEROBJ)mixerHandle;

    lpmxc = 0;
    lpListText = 0;
    lpListBool = 0;
    mixerline.cbStruct = 168;
    mixerline.dwComponentType = 7;

    if (mixerGetLineInfoA(phmx, &mixerline, 3u) != MMSYSERR_NOERROR)
    {
        mixerClose(mixerHandle); // KISAKTODO: this fails always for some reason.
        return 0;
    }

    mixerGetLineInfoA((HMIXEROBJ)phmx, &mixerline, 3u);
    lpmxc = (tagMIXERCONTROLA *)calloc(148 * mixerline.cControls, 1u);
    mxlc.cbStruct = 24;
    mxlc.dwLineID = mixerline.dwLineID;
    mxlc.dwControlID = 0;
    mxlc.cControls = mixerline.cControls;
    mxlc.cbmxctrl = 148;
    mxlc.pamxctrl = lpmxc;
    err = mixerGetLineControlsA((HMIXEROBJ)phmx, &mxlc, 0);
    if ( !err )
    {
        for ( ii = 0; ii < (int)mixerline.cControls; ++ii )
        {
            if ( (lpmxc[ii].dwControlType & 0xF0000000) == 0x70000000 )
            {
                iNumChannels = mixerline.cChannels;
                iMultipleItems = 0;
                if ( (lpmxc[ii].fdwControl & 1) != 0 )
                    iNumChannels = 1;
                if ( (lpmxc[ii].fdwControl & 2) != 0 )
                    iMultipleItems = lpmxc[ii].cMultipleItems;
                lpListText = (tagMIXERCONTROLDETAILS_LISTTEXTA *)calloc(72 * iMultipleItems * iNumChannels, 1u);
                mxcd.cbStruct = 24;
                mxcd.dwControlID = lpmxc[ii].dwControlID;
                mxcd.cChannels = iNumChannels;
                mxcd.cMultipleItems = iMultipleItems;
                mxcd.cbDetails = 72;
                mxcd.paDetails = lpListText;
                err = mixerGetControlDetailsA((HMIXEROBJ)phmx, &mxcd, 1u);
                if ( !err )
                {
                    lpListBool = (tMIXERCONTROLDETAILS_BOOLEAN *)calloc(4 * iMultipleItems * iNumChannels, 1u);
                    mxcd.cbDetails = 4;
                    mxcd.paDetails = lpListBool;
                    err = mixerGetControlDetailsA((HMIXEROBJ)phmx, &mxcd, 0);
                    if ( !err )
                    {
                        for ( jj = 0; jj < iMultipleItems; ++jj )
                        {
                            if ( lpListBool[jj].fValue == 1 )
                                strncpy(srcName, lpListText[jj].szName, 0xFFu);
                        }
                        if ( lpmxc )
                            free(lpmxc);
                        lpmxc = 0;
                        if ( lpListText )
                            free(lpListText);
                        lpListText = 0;
                        if ( lpListBool )
                            free(lpListBool);
                        lpListBool = 0;
                        if ( !err )
                            return 1;
                    }
                }
            }
        }
    }
    if ( lpmxc )
        free(lpmxc);
    lpmxc = 0;
    if ( lpListText )
        free(lpListText);
    lpListText = 0;
    if ( lpListBool )
        free(lpListBool);
    return 0;
}

int __cdecl mixerSetRecordSource(char *SrcName)
{
    int jj; // [esp+0h] [ebp-100h]
    tagMIXERLINECONTROLSA mxlc; // [esp+4h] [ebp-FCh] BYREF
    unsigned int err; // [esp+1Ch] [ebp-E4h]
    int iNumChannels; // [esp+20h] [ebp-E0h]
    int iMultipleItems; // [esp+24h] [ebp-DCh]
    tMIXERCONTROLDETAILS_BOOLEAN *lpListBool; // [esp+2Ch] [ebp-D4h]
    int ii; // [esp+30h] [ebp-D0h]
    tagMIXERCONTROLDETAILS_LISTTEXTA *lpListText; // [esp+34h] [ebp-CCh]
    tagMIXERLINEA mixerline; // [esp+38h] [ebp-C8h] BYREF
    tMIXERCONTROLDETAILS mxcd; // [esp+E4h] [ebp-1Ch] BYREF
    tagMIXERCONTROLA *lpmxc; // [esp+FCh] [ebp-4h]

    HMIXER mixerHandle;

    if ( !waveInGetNumDevs() )
        return 0;
    if ( !mixerGetNumDevs() )
        return 0;
    if ( mixerOpen(&mixerHandle, 0, 0, 0, 0) )
        return 0;

    HMIXEROBJ phmx = (HMIXEROBJ)mixerHandle;

    lpmxc = 0;
    lpListText = 0;
    lpListBool = 0;
    mixerline.cbStruct = 168;
    mixerline.dwComponentType = 7;

    if (mixerGetLineInfoA(phmx, &mixerline, 3u) != MMSYSERR_NOERROR)
    {
        mixerClose(mixerHandle); // KISAKTODO: this fails always for some reason.
        return 0;
    }

    mixerGetLineInfoA((HMIXEROBJ)phmx, &mixerline, 3u);
    lpmxc = (tagMIXERCONTROLA *)calloc(148 * mixerline.cControls, 1u);
    mxlc.cbStruct = 24;
    mxlc.dwLineID = mixerline.dwLineID;
    mxlc.dwControlID = 0;
    mxlc.cControls = mixerline.cControls;
    mxlc.cbmxctrl = 148;
    mxlc.pamxctrl = lpmxc;
    err = mixerGetLineControlsA((HMIXEROBJ)phmx, &mxlc, 0);
    if ( !err )
    {
        for ( ii = 0; ii < (int)mixerline.cControls; ++ii )
        {
            if ( (lpmxc[ii].dwControlType & 0xF0000000) == 0x70000000 )
            {
                iNumChannels = mixerline.cChannels;
                iMultipleItems = 0;
                if ( (lpmxc[ii].fdwControl & 1) != 0 )
                    iNumChannels = 1;
                if ( (lpmxc[ii].fdwControl & 2) != 0 )
                    iMultipleItems = lpmxc[ii].cMultipleItems;
                lpListText = (tagMIXERCONTROLDETAILS_LISTTEXTA *)calloc(72 * iMultipleItems * iNumChannels, 1u);
                mxcd.cbStruct = 24;
                mxcd.dwControlID = lpmxc[ii].dwControlID;
                mxcd.cChannels = iNumChannels;
                mxcd.cMultipleItems = iMultipleItems;
                mxcd.cbDetails = 72;
                mxcd.paDetails = lpListText;
                err = mixerGetControlDetailsA((HMIXEROBJ)phmx, &mxcd, 1u);
                if ( !err )
                {
                    lpListBool = (tMIXERCONTROLDETAILS_BOOLEAN *)calloc(4 * iMultipleItems * iNumChannels, 1u);
                    mxcd.cbDetails = 4;
                    mxcd.paDetails = lpListBool;
                    err = mixerGetControlDetailsA((HMIXEROBJ)phmx, &mxcd, 0);
                    if ( !err )
                    {
                        for ( jj = 0; jj < iMultipleItems; ++jj )
                        {
                            if ( I_stricmp(lpListText[jj].szName, SrcName) )
                                lpListBool[jj].fValue = 0;
                            else
                                lpListBool[jj].fValue = 1;
                        }
                        err = mixerSetControlDetails((HMIXEROBJ)phmx, &mxcd, 0);
                        if ( lpmxc )
                            free(lpmxc);
                        lpmxc = 0;
                        if ( lpListText )
                            free(lpListText);
                        lpListText = 0;
                        if ( lpListBool )
                            free(lpListBool);
                        lpListBool = 0;
                        if ( !err )
                            return 1;
                    }
                }
            }
        }
    }
    if ( lpmxc )
        free(lpmxc);
    lpmxc = 0;
    if ( lpListText )
        free(lpListText);
    lpListText = 0;
    if ( lpListBool )
        free(lpListBool);
    return 0;
}

int __cdecl mixerSetMicrophoneMute(unsigned __int8 bMute)
{
    tagMIXERCONTROLA mxc; // [esp+0h] [ebp-188h] BYREF
    unsigned int jj; // [esp+98h] [ebp-F0h]
    tagMIXERLINECONTROLSA mxlc; // [esp+9Ch] [ebp-ECh] BYREF
    unsigned int err; // [esp+B4h] [ebp-D4h]
    HMIXER__ *phmx; // [esp+B8h] [ebp-D0h] BYREF
    unsigned int ii; // [esp+BCh] [ebp-CCh]
    tagMIXERLINEA mixerline; // [esp+C0h] [ebp-C8h] BYREF
    tMIXERCONTROLDETAILS_BOOLEAN newSetting; // [esp+16Ch] [ebp-1Ch] BYREF
    tMIXERCONTROLDETAILS mxcd; // [esp+170h] [ebp-18h] BYREF

    if ( !waveInGetNumDevs() )
        return 0;
    if ( !mixerGetNumDevs() )
        return 0;
    if ( mixerOpen(&phmx, 0, 0, 0, 0) )
        return 0;
    mixerline.cbStruct = 168;
    mixerline.dwComponentType = 4;
    err = mixerGetLineInfoA((HMIXEROBJ)phmx, &mixerline, 3u);
    if ( !err )
    {
        jj = mixerline.cConnections;
        for ( ii = 0; ii < jj; ++ii )
        {
            mixerline.dwSource = ii;
            mixerGetLineInfoA((HMIXEROBJ)phmx, &mixerline, 1u);
            if ( !I_stricmp(mixerline.szName, old_rec_source) )
            {
                mxlc.cbStruct = 24;
                mxlc.dwLineID = mixerline.dwLineID;
                mxlc.dwControlID = 536936450;
                mxlc.cControls = 1;
                mxlc.cbmxctrl = 148;
                mxlc.pamxctrl = &mxc;
                err = mixerGetLineControlsA((HMIXEROBJ)phmx, &mxlc, 2u);
                if ( !err )
                {
                    mxcd.cbStruct = 24;
                    mxcd.cChannels = 1;
                    mxcd.cbDetails = 4;
                    mxcd.paDetails = &newSetting;
                    mxcd.cMultipleItems = 0;
                    mxcd.dwControlID = mxc.dwControlID;
                    err = mixerGetControlDetailsA((HMIXEROBJ)phmx, &mxcd, 0);
                    if ( !err )
                    {
                        newSetting.fValue = bMute;
                        err = mixerSetControlDetails((HMIXEROBJ)phmx, &mxcd, 0);
                    }
                }
                break;
            }
        }
    }
    mixerClose(phmx);
    return 0;
}

int __cdecl Live_GetClientNumForXuid(const SessionData_s *session, unsigned __int64 xuid)
{
    return Session_FindRegisteredUser(session, xuid);
}

