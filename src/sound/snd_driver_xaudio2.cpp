#include "snd_driver_xaudio2.h"
#include <ui/ui_main.h>
#include "snd_stream.h"
#include "snd_db.h"
#include <qcommon/common.h>
#include "snd_public_async.h"
#include <gfx_d3d/r_cinematic.h>
#include "snd_dvar.h"
#include "snd_utils.h"
#include "snd_radverb.h"
#include "snd_bank.h"
#include "snd_globals.h"

const dvar_t *sd_xa2_num_devices;
const dvar_t *sd_xa2_can_switch_device;
const dvar_t *sd_xa2_device_guid;
const dvar_t *sd_xa2_device_name;

SoundState g_sd;
char sd_xa2_device_names[16][256];
const char *sd_xa2_devices_value_list[16];
const char *sd_xa2_device_guids_value_list[16];
int sd_xa2_device_indices[16];
_GUID sd_deviceGUIDs[16];
char sd_xa2_device_guids[16][256];

unsigned int g_xaudio27 = 1;
const int sd_max_num_devices = 16;

bool __cdecl SD_Xaudio2CanInit()
{
    return g_xaudio27 != 0;
}

bool SND_IsVoiceFree(int voiceIndex)
{
    return !g_snd.voiceAliasHash[voiceIndex];
}

SDXA2SourceEffect *__cdecl SDXA2_VoiceDspAllocate()
{
    SDXA2SourceEffect *dsp; // [esp+4h] [ebp-8h]
    unsigned int i; // [esp+8h] [ebp-4h]

    for ( i = 0; i < 148; ++i )
    {
        dsp = &g_sd.voiceDsp[i];
        if ( dsp->AddRef() == 2 )
        {
            DWORD count = dsp->Release();
            iassert(count == 1);
            return dsp;
        }
        dsp->Release();
    }

    iassert(0);
    return 0;
}

void __cdecl SD_TruncateAudioDeviceNames(Font_s *font, float scale, int size)
{
    int j; // [esp+18h] [ebp-Ch]
    char *name; // [esp+1Ch] [ebp-8h]
    int i; // [esp+20h] [ebp-4h]

    for ( i = 0; i < 16; ++i )
    {
        name = sd_xa2_device_names[i];
        for ( j = strlen(name) - 1; j >= 0 && UI_TextWidth(name, 0, font, scale) > size; --j )
            name[j] = 0;
    }
}

void __cdecl SD_XAudio2Done()
{
    if ( g_sd.xa2 )
    {
        g_sd.xa2->Release();
        g_sd.xa2 = 0;
        g_sd.deviceGUIDValid = 0;
        g_sd.deviceIndex = -1;
        memset(&g_sd.deviceGUID, 0, sizeof(g_sd.deviceGUID));
        CoUninitialize();
    }
}

unsigned int __cdecl SDXA2_ChannelFixup(unsigned int outputChannels, unsigned int channel)
{
    if ( outputChannels != 8 )
        return channel;
    switch ( channel )
    {
        case 4u:
            return 6;
        case 5u:
            return 7;
        case 6u:
            return 4;
        case 7u:
            return 5;
        default:
            return channel;
    }
}

IXAudio2SubmixVoice *__cdecl SD_CreateBus(
                unsigned int inputChannelCount,
                unsigned int outputChannelCount,
                unsigned int sampleRate,
                unsigned int stage,
                SDXA2Effect *dsp,
                IXAudio2Voice *outputVoice)
{
    float v7; // [esp+0h] [ebp-140h]
    unsigned int j; // [esp+8h] [ebp-138h]
    unsigned int i; // [esp+Ch] [ebp-134h]
    HRESULT hr; // [esp+10h] [ebp-130h]
    IXAudio2SubmixVoice *voice; // [esp+14h] [ebp-12Ch] BYREF
    float matrix[65]; // [esp+18h] [ebp-128h] BYREF
    XAUDIO2_VOICE_SENDS sends; // [esp+11Ch] [ebp-24h] BYREF
    XAUDIO2_EFFECT_CHAIN effects; // [esp+124h] [ebp-1Ch] BYREF
    XAUDIO2_EFFECT_DESCRIPTOR effectDescriptors[1]; // [esp+12Ch] [ebp-14h] BYREF
    XAUDIO2_SEND_DESCRIPTOR outputSend; // [esp+138h] [ebp-8h] BYREF

    outputSend.Flags = 0;
    outputSend.pOutputVoice = outputVoice;
    sends.SendCount = 1;
    sends.pSends = &outputSend;
    effectDescriptors[0].pEffect = (CXAPOBase*)dsp;
    effectDescriptors[0].InitialState = 1;
    effectDescriptors[0].OutputChannels = inputChannelCount;
    effects.EffectCount = 1;
    effects.pEffectDescriptors = effectDescriptors;

    hr = g_sd.xa2->CreateSubmixVoice(
                 &voice,
                 inputChannelCount,
                 sampleRate,
                 0,
                 stage,
                 &sends,
                 dsp != 0 ? &effects : 0);

    iassert(hr == S_OK);
    iassert(inputChannelCount <= 8);
    iassert(outputChannelCount <= 8);

    for ( i = 0; i < inputChannelCount; ++i )
    {
        for ( j = 0; j < outputChannelCount; ++j )
        {
            if ( i == j )
                v7 = 1.0f;
            else
                v7 = 0.0f;
            matrix[i + j * inputChannelCount] = v7;
        }
    }

    hr = voice->SetOutputMatrix(outputVoice, inputChannelCount, outputChannelCount, matrix, 0);
    iassert(hr == S_OK);

    return voice;
}

void __cdecl SD_StopVoice(int voiceIndex)
{
    unsigned int i; // [esp+0h] [ebp-8h]
    int streamVoice; // [esp+4h] [ebp-4h]

    if ( g_sd.voices[voiceIndex] )
    {
        g_sd.voices[voiceIndex]->Stop(0, 0);
        g_sd.voices[voiceIndex]->DestroyVoice();
        g_sd.voices[voiceIndex] = 0;
    }
    if ( SND_IsStream(voiceIndex) )
    {
        streamVoice = iSND_GetStreamChannel(voiceIndex);
        for ( i = 0; i < 2; ++i )
            iSND_ReleaseStreamBuffer(streamVoice, i);
        if ( Snd_StreamStatus(streamVoice) )
            Snd_StreamClose(streamVoice);
    }
}

unsigned int __cdecl iSND_GetStreamChannel(unsigned int index)
{
    iassert(index >= (0) && index < (0) + 10);
    return index;
}

void __cdecl iSND_ReleaseStreamBuffer(unsigned int streamVoice, unsigned int bufferIndex)
{
    char *releaseWindow; // [esp+4h] [ebp-8h]

    iassert(streamVoice < SND_MAX_STREAM_VOICES);
    iassert(bufferIndex < StreamVoice::BUFFER_COUNT);

    releaseWindow = (char *)InterlockedExchange((volatile unsigned int *)&g_sd.streamVoices[streamVoice].buffers[bufferIndex].pAudioData, 0);

    if (releaseWindow)
    {
        Snd_StreamReleaseWindow(streamVoice, releaseWindow);
    }
}

int __cdecl SND_StartAliasStream(SndStartAliasInfo *startAliasInfo, unsigned int voiceIndex)
{
    unsigned int primeSize; // [esp+8h] [ebp-118h]
    char *primeData; // [esp+Ch] [ebp-114h]
    char filename2[260]; // [esp+10h] [ebp-110h] BYREF
    int streamVoice; // [esp+118h] [ebp-8h]
    const snd_alias_t *alias; // [esp+11Ch] [ebp-4h]

    alias = startAliasInfo->alias;
    streamVoice = iSND_GetStreamChannel(voiceIndex);
    SND_AliasGetFileName(alias, filename2, 256);
    if (alias->soundFile->exists)
    {
        primeSize = 0;
        primeData = 0;
        if ((alias->flags & 0xC000) >> 14 == 3 && alias->soundFile->u.streamSnd->primeSnd)
        {
            primeData = alias->soundFile->u.streamSnd->primeSnd->buffer;
            primeSize = alias->soundFile->u.streamSnd->primeSnd->size;
        }
        Snd_StreamOpen(streamVoice, filename2, (alias->flags & 1) != 0, primeSize, primeData);
        SND_SetVoiceStartInfo(voiceIndex, startAliasInfo);
        SND_SetSoundFileVoiceInfo(voiceIndex, 0, 0, 0, 0, SFLS_LOADING);
        SD_UpdateVoice(voiceIndex);
        if (g_snd.voice[voiceIndex].alias)
            return startAliasInfo->playbackId;
        else
            return -1;
    }
    else
    {
        Com_PrintError(
            1,
            "Tried to play streamed sound '%s' from alias '%s', but it was not found at load time.\n",
            filename2,
            alias->name);
        return SND_SetPlaybackIdNotPlayed(voiceIndex);
    }
}

void __cdecl SD_UpdateStreamVoice(int voiceIndex)
{
    XAUDIO2_BUFFER *buffer; // eax
    unsigned int bufferIndex; // [esp+Ch] [ebp-48h]
    StreamVoice *sv; // [esp+14h] [ebp-40h]
    char *data; // [esp+18h] [ebp-3Ch] BYREF
    XAUDIO2_VOICE_STATE voiceState; // [esp+1Ch] [ebp-38h] BYREF
    unsigned int size; // [esp+2Ch] [ebp-28h] BYREF
    unsigned int position; // [esp+30h] [ebp-24h] BYREF
    unsigned int lengthMS; // [esp+34h] [ebp-20h]
    const snd_asset *header; // [esp+38h] [ebp-1Ch]
    snd_voice_t *voice; // [esp+3Ch] [ebp-18h]
    snd_stream_status status; // [esp+40h] [ebp-14h]
    int streamVoice; // [esp+44h] [ebp-10h]
    XAUDIO2_BUFFER_WMA bufferWMA; // [esp+48h] [ebp-Ch] BYREF
    int isLooping; // [esp+50h] [ebp-4h]
    HRESULT err;

    streamVoice = iSND_GetStreamChannel(voiceIndex);
    status = Snd_StreamStatus(streamVoice);
    voice = &g_snd.voice[voiceIndex];
    isLooping = (voice->alias->flags & 1) != 0;
    header = 0;
    bufferWMA.pDecodedPacketCumulativeBytes = 0;
    bufferWMA.PacketCount = 0;

    if (!status || status == SND_STREAM_STARVING || !Snd_StreamGetHeader(streamVoice))
    {
        return;
    }

    if (status == SND_STREAM_ERROR)
    {
        iSND_ChannelError(voiceIndex);
        return;
    }

    if (g_sd.voices[voiceIndex])
    {
        goto LABEL_11;
    }

    if (status != SND_STREAM_OK)
    {
        return;
    }

    header = Snd_StreamGetHeader(streamVoice);
    if (!iSND_CreateVoice(voiceIndex, header, isLooping, (g_snd.voice[voiceIndex].alias->flags & 0x3000) >> 12))
    {
        iSND_ChannelError(voiceIndex);
        return;

    }
    lengthMS = 1000LL * header->frame_count / header->frame_rate;
    SND_SetSoundFileVoiceInfo(voiceIndex, header->channel_count, header->frame_rate, lengthMS, 0, SFLS_LOADED);
    SND_UpdateVoice(&g_snd.voice[voiceIndex], 0.0);
    if (header->format == SND_ASSET_FORMAT_WMA)
    {
        bufferWMA.PacketCount = header->seek_table_count;
        bufferWMA.pDecodedPacketCumulativeBytes = header->seek_table;
    }
LABEL_11:
    while (Snd_StreamGetFreeWindows(streamVoice))
    {
        g_sd.voices[voiceIndex]->GetState(&voiceState);
        if (voiceState.BuffersQueued >= 2)
            break;
        status = Snd_StreamAcquireWindow(streamVoice, &size, &position, &data);
        switch (status)
        {
        case SND_STREAM_OK:
            if (!header)
            {
                header = Snd_StreamGetHeader(streamVoice);
                if (header->format == SND_ASSET_FORMAT_WMA)
                {
                    bufferWMA.PacketCount = header->seek_table_count;
                    bufferWMA.pDecodedPacketCumulativeBytes = header->seek_table;
                }
            }
            sv = &g_sd.streamVoices[streamVoice];
            bufferIndex = sv->bufferQueuedCount % 2;
            ++sv->bufferQueuedCount;
            buffer = &sv->buffers[bufferIndex];
            memset(buffer, 0, sizeof(XAUDIO2_BUFFER));
            buffer->AudioBytes = size;
            iassert(!buffer->pAudioData);
            buffer->pAudioData = (const unsigned __int8 *)data;
            buffer->pContext = (void *)(bufferIndex | (streamVoice << 8));

            err = g_sd.voices[voiceIndex]->SubmitSourceBuffer(buffer, header->format != SND_ASSET_FORMAT_WMA ? 0 : &bufferWMA);
            iassert(err == S_OK); // LWSS ADD
            break;
        case SND_STREAM_STARVING:
            return;
        case SND_STREAM_EOF:
            if (!voiceState.BuffersQueued)
            {
                SND_StopVoice(voiceIndex);
            }
            return;
        case SND_STREAM_ERROR:
            iSND_ChannelError(voiceIndex);
            return;
        }
    }
}

int __cdecl iSND_CreateVoice(int voiceIndex, const snd_asset *snd, int isLooping, int bus)
{
    IXAudio2SubmixVoice *masterBus; // [esp+1Ch] [ebp-ECh]
    snd_asset_format format; // [esp+20h] [ebp-E8h]
    HRESULT hr; // [esp+38h] [ebp-D0h]
    bool isStreaming; // [esp+3Ch] [ebp-CCh]
    my_adpcmwaveformat_tag AWFT; // [esp+40h] [ebp-C8h] BYREF
    XAUDIO2_EFFECT_DESCRIPTOR effect;        // [esp+98h] [ebp-70h] BYREF
    int streamVoice; // [esp+A4h] [ebp-64h]
    WAVEFORMATEXTENSIBLE WFXE;               // [esp+A8h] [ebp-60h] BYREF
    VoiceInfo *voiceInfo; // [esp+D4h] [ebp-34h]
    XAUDIO2_SEND_DESCRIPTOR outputVoices[2]; // [esp+E0h] [ebp-28h] BYREF
    bool isStereo; // [esp+F6h] [ebp-12h]
    bool isADPCM; // [esp+F7h] [ebp-11h]
    XAUDIO2_VOICE_SENDS sendList;            // [esp+F8h] [ebp-10h] BYREF
    XAUDIO2_EFFECT_CHAIN fxChain;            // [esp+100h] [ebp-8h] BYREF

    isStreaming = SND_IsStream(voiceIndex);
    streamVoice = isStreaming ? iSND_GetStreamChannel(voiceIndex) : -1;
    isStereo = snd->channel_count != 1;
    isADPCM = snd->format == SND_ASSET_FORMAT_MSADPCM;
    voiceInfo = &g_sd.voiceInfos[voiceIndex];

    memset(&WFXE, 0, sizeof(WFXE));
    memset(&AWFT, 0, sizeof(AWFT));

    AWFT.aCoef[0].iCoef1 = 256;
    AWFT.aCoef[0].iCoef2 = 0;
    AWFT.aCoef[1].iCoef1 = 512;
    AWFT.aCoef[1].iCoef2 = -256;
    AWFT.aCoef[2].iCoef1 = 0;
    AWFT.aCoef[2].iCoef2 = 0;
    AWFT.aCoef[3].iCoef1 = 192;
    AWFT.aCoef[3].iCoef2 = 64;
    AWFT.aCoef[4].iCoef1 = 240;
    AWFT.aCoef[4].iCoef2 = 0;
    AWFT.aCoef[5].iCoef1 = 460;
    AWFT.aCoef[5].iCoef2 = -208;
    AWFT.aCoef[6].iCoef1 = 392;
    AWFT.aCoef[6].iCoef2 = -232;

    WAVEFORMATEX *wfx = isADPCM ? &AWFT.wfx : &WFXE.Format;
    wfx->nChannels = snd->channel_count;
    format = snd->format;

    if (format)
    {
        if (format == SND_ASSET_FORMAT_MSADPCM)
        {
            wfx->wFormatTag = 2;
            wfx->nChannels = snd->channel_count;
            wfx->nBlockAlign = 262 * snd->channel_count;
            AWFT.wSamplesPerBlock = 2 * wfx->nBlockAlign / wfx->nChannels - 12;
            AWFT.wNumCoef = 7;
            wfx->nSamplesPerSec = snd->frame_rate;
            wfx->wBitsPerSample = 4;
            wfx->cbSize = 32;
            wfx->nAvgBytesPerSec = (__int64)(wfx->nSamplesPerSec * (unsigned __int64)wfx->nBlockAlign) / AWFT.wSamplesPerBlock;
        }
        else
        {
            if (format != SND_ASSET_FORMAT_WMA)
            {
                Com_PrintError(9, "invalid asset for alias %s\n", g_snd.voice[voiceIndex].alias->name);
                return 0;
            }
            iassert(!isStreaming);
            iassert(snd->seek_table);

            wfx->wFormatTag = 353;
            wfx->nBlockAlign = isStereo ? 4096 : 2230;
            wfx->nSamplesPerSec = snd->frame_rate;
            wfx->nAvgBytesPerSec = 6000 * snd->channel_count;
            wfx->wBitsPerSample = 16;
            wfx->cbSize = 0;
        }
    }
    else
    {
        wfx->wFormatTag = 1;
        wfx->nBlockAlign = (snd->channel_count * wfx->wBitsPerSample) / 8;
        wfx->nSamplesPerSec = snd->frame_rate;
        wfx->nAvgBytesPerSec = wfx->nSamplesPerSec * wfx->nBlockAlign;
        wfx->wBitsPerSample = 16;
        wfx->cbSize = 21072;
    }

    XAUDIO2_BUFFER_WMA bufferWMA;
    XAUDIO2_BUFFER buffer{ 0 };

    bufferWMA.pDecodedPacketCumulativeBytes = 0;
    bufferWMA.PacketCount = 0;
    buffer.Flags = XAUDIO2_END_OF_STREAM; // final buffer, not going to queue anything else

    if (snd->format == SND_ASSET_FORMAT_WMA)
    {
        bufferWMA.PacketCount = snd->seek_table_count;
        bufferWMA.pDecodedPacketCumulativeBytes = snd->seek_table;
        buffer.AudioBytes = bufferWMA.PacketCount * wfx->nBlockAlign;
        iassert(buffer.AudioBytes <= snd->data_size);
    }
    else
    {
        buffer.AudioBytes = snd->data_size;
    }

    iassert(!buffer.pAudioData);

    buffer.pAudioData = (const unsigned __int8 *)snd->data;
    buffer.LoopCount = isLooping != 0 ? 255 : 0;

    outputVoices[0].Flags = 0;

    if (bus == 2)
    {
        masterBus = g_sd.masterBus;
    }
    else
    {
        masterBus = g_sd.novoiceBus;
    }

    outputVoices[0].pOutputVoice = masterBus;
    outputVoices[1].Flags = 0;
    outputVoices[1].pOutputVoice = g_sd.radverbBus;

    sendList.SendCount = 2;
    sendList.pSends = outputVoices;

    voiceInfo->sourceDsp = SDXA2_VoiceDspAllocate();
    effect.pEffect = (CXAPOBase*)voiceInfo->sourceDsp;
    effect.InitialState = 1;
    effect.OutputChannels = snd->channel_count;

    fxChain.EffectCount = 1;
    fxChain.pEffectDescriptors = &effect;

    voiceInfo->sourceDsp->Clear();

    hr = g_sd.xa2->CreateSourceVoice(&g_sd.voices[voiceIndex],
        isADPCM ? &AWFT.wfx : &WFXE.Format,
        0,
        2.0f,
        isStreaming ? &g_sd.streamVoices[streamVoice] : NULL,
        &sendList,
        &fxChain);
    iassert(hr == S_OK);

    if (g_sd.voices[voiceIndex])
    {
        if (isStreaming)
        {
            g_sd.streamVoices[streamVoice].bufferQueuedCount = 0;
        }
        else
        {
            hr = g_sd.voices[voiceIndex]->SubmitSourceBuffer(&buffer, snd->seek_table != 0 ? &bufferWMA : 0);
            iassert(hr == S_OK);
        }

        return 1;
    }
    else
    {
        iassert(0);
        return 0;
    }
}

void __cdecl iSND_ChannelError(int voiceIndex)
{
    const snd_alias_t *alias; // [esp+4h] [ebp-4h]

    alias = g_snd.voice[voiceIndex].alias;
    SND_LengthNotify(voiceIndex, 0);
    SND_StopVoice(voiceIndex);
    alias->soundFile->exists = 0;
}

char __cdecl SD_Init()
{
    if ( g_xaudio27 )
    {
        g_sd.deviceIndex = -1;
        g_sd.deviceGUIDValid = 0;
        memset(&g_sd.deviceGUID, 0, sizeof(g_sd.deviceGUID));
        g_sd.xa2 = 0;
        memset(&g_sd.details, 0, sizeof(g_sd.details));
        g_sd.masterVoice = 0;
        g_sd.masterVoiceDetails.CreationFlags = 0;
        g_sd.masterVoiceDetails.InputChannels = 0;
        g_sd.masterVoiceDetails.InputSampleRate = 0;
        g_sd.masterBus = 0;
        memset(&g_sd.masterParams, 0, sizeof(g_sd.masterParams));
        g_sd.novoiceBus = 0;
        memset(&g_sd.novoiceParams, 0, sizeof(g_sd.novoiceParams));
        g_sd.radverbBus = 0;
        memset(&g_sd.radverbParams, 0, sizeof(g_sd.radverbParams));
        memset(g_sd.voices, 0, sizeof(g_sd.voices));
        memset(&g_sd.perfData, 0, sizeof(g_sd.perfData));

        SDXA2_VoiceDspAssertUnused();
        if ( SD_XAudio2Init() )
        {
            SND_InitMasterVoice();
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_Error(ERR_DROP, "Cannot run with sound built on wrong version of XDK");
        return 0;
    }
}

void SDXA2_VoiceDspAssertUnused()
{
    unsigned int i; // [esp+8h] [ebp-4h]

    for (i = 0; i < 148; ++i)
    {
        DWORD count = g_sd.voiceDsp[i].AddRef();
        iassert(count == 2);
        count = g_sd.voiceDsp[i].Release();
        iassert(count == 1);
    }
}

bool __cdecl SD_XAudio2Init()
{
    HRESULT hr; // [esp+28h] [ebp-Ch]
    unsigned int debug; // [esp+2Ch] [ebp-8h]

    if (g_sd.xa2)
    {
        return 0;
    }

    debug = 0;
    sd_xa2_num_devices = _Dvar_RegisterInt("sd_xa2_num_devices", 0, 0, 16, 0, "Number of xaudio2 devices");
    SD_XAudio2EnumerateDevices();
    if ( sd_xa2_num_devices->current.integer )
    {
        hr = XAudio2Create(&g_sd.xa2, debug, XAUDIO2_ANY_PROCESSOR);
        iassert(hr == S_OK);
        SD_SwitchDevice();
    }
    sd_xa2_can_switch_device = _Dvar_RegisterBool("sd_can_switch_device", sd_xa2_num_devices->current.integer != 0, 0x40u, "Allow Switching sound device");
    return g_sd.xa2 != 0;
}

char __cdecl SD_XAudio2EnumerateDevices()
{
    int v1; // [esp+0h] [ebp-45Ch]
    int n; // [esp+10h] [ebp-44Ch]
    unsigned int deviceIndex; // [esp+14h] [ebp-448h]
    int defaultDevice; // [esp+18h] [ebp-444h]
    XAUDIO2_DEVICE_DETAILS deviceDetails; // [esp+1Ch] [ebp-440h] BYREF
    unsigned int deviceCount; // [esp+450h] [ebp-Ch] BYREF
    HRESULT hr; // [esp+454h] [ebp-8h]
    IXAudio2 *xa2; // [esp+458h] [ebp-4h] BYREF

    xa2 = 0;
    if ( sd_xa2_num_devices->current.integer )
        return 1;
    CoInitializeEx(0, 0);
    hr = XAudio2Create(&xa2, 0, XAUDIO2_ANY_PROCESSOR);
    if ( hr < 0 )
        return 0;
    defaultDevice = 0;
    hr = xa2->GetDeviceCount(&deviceCount);
    if ( hr >= 0 )
    {
        if ( deviceCount >= sd_max_num_devices )
            v1 = sd_max_num_devices;
        else
            v1 = deviceCount;
        deviceCount = v1;
        for ( deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex )
        {
            xa2->GetDeviceDetails(deviceIndex, &deviceDetails);
            if ( SD_XAudio2CheckDevice(xa2, &deviceDetails, deviceIndex) )
            {
                n = sd_xa2_num_devices->current.integer;
                Dvar_SetInt((dvar_s *)sd_xa2_num_devices, n + 1);
                WideCharToMultiByte(0, 0, deviceDetails.DisplayName, -1, sd_xa2_device_names[n], 256, 0, 0);
                SD_XAudio2GetDeviceGUID(deviceDetails.DeviceID, &sd_deviceGUIDs[n], sd_xa2_device_guids[n], 256);
                sd_xa2_devices_value_list[n] = sd_xa2_device_names[n];
                sd_xa2_device_guids_value_list[n] = sd_xa2_device_guids[n];
                sd_xa2_device_indices[n] = deviceIndex;
                if ( deviceDetails.Role == GlobalDefaultDevice )
                    defaultDevice = n;
            }
        }
        sd_xa2_devices_value_list[sd_xa2_num_devices->current.integer] = 0;
        sd_xa2_device_guids_value_list[sd_xa2_num_devices->current.integer] = 0;
        sd_xa2_device_guid = _Dvar_RegisterEnum(
                                                     "sd_xa2_device_guid",
                                                     sd_xa2_device_guids_value_list,
                                                     defaultDevice,
                                                     1u,
                                                     "prefered audio device");
        sd_xa2_device_name = _Dvar_RegisterEnum(
                                                     "sd_xa2_device_name",
                                                     sd_xa2_devices_value_list,
                                                     sd_xa2_device_guid->current.integer,
                                                     0,
                                                     "Available Sound devices");
    }
    //((void (__thiscall *)(IXAudio2 *, IXAudio2 *))xa2->Release)(xa2, xa2);
    xa2->Release();
    return 1;
}

char __cdecl SD_XAudio2GetDeviceGUID(wchar_t *inGUID, _GUID *guid, char *outGUID, int size)
{
    while ( *inGUID )
    {
        if ( !CLSIDFromString(inGUID, guid) )
        {
            WideCharToMultiByte(0, 0, inGUID, -1, outGUID, size, 0, 0);
            return 1;
        }
        ++inGUID;
    }
    return 0;
}

char __cdecl SD_XAudio2CheckDevice(
                IXAudio2 *pXAudio2,
                const XAUDIO2_DEVICE_DETAILS *deviceDetails,
                unsigned int deviceIndex)
{
    IXAudio2MasteringVoice *testVoice; // [esp+4h] [ebp-Ch] BYREF
    HRESULT hr; // [esp+8h] [ebp-8h]
    unsigned int nChannels; // [esp+Ch] [ebp-4h]

    if (deviceDetails->OutputFormat.Format.nChannels < 2u)
    {
        return 0;
    }
    nChannels = deviceDetails->OutputFormat.Format.nChannels <= 8u ? 0 : 8;
    testVoice = 0;
    hr = pXAudio2->CreateMasteringVoice(&testVoice, nChannels, 48000u, 0, deviceIndex, 0);
    if ( hr )
    {
        hr = pXAudio2->CreateMasteringVoice(&testVoice, nChannels, deviceDetails->OutputFormat.Format.nSamplesPerSec > 48000 ? 48000 : 0, 0, deviceIndex, 0);
        if (hr)
        {
            return 0;
        }
    }
    testVoice->DestroyVoice();
    return 1;
}

void SD_SwitchDevice()
{
    if ( sd_xa2_num_devices->current.integer )
    {
        iassert(g_sd.xa2);
        int deviceIndex = sd_xa2_device_indices[sd_xa2_device_guid->current.integer];
        iassert(g_sd.deviceIndex != deviceIndex);
        g_sd.deviceIndex = deviceIndex;
        g_sd.xa2->GetDeviceDetails(deviceIndex, &g_sd.details);
        g_sd.deviceGUID = sd_deviceGUIDs[deviceIndex];
        g_sd.deviceGUIDValid = 1;
        R_CinematicInitSound(&g_sd.deviceGUID);
    }
}

void SND_InitMasterVoice()
{
    unsigned int index; // [esp+8h] [ebp-11Ch]
    int i; // [esp+10h] [ebp-114h]
    HRESULT hr; // [esp+14h] [ebp-110h]
    float matrix[64]; // [esp+1Ch] [ebp-108h] BYREF
    int inputChannels; // [esp+11Ch] [ebp-8h]
    int inputSampleRate; // [esp+120h] [ebp-4h]

    inputChannels = g_sd.details.OutputFormat.Format.nChannels <= 8u ? 0 : 8;
    inputSampleRate = g_sd.details.OutputFormat.Format.nSamplesPerSec > 48000 ? 48000 : 0;

    hr = g_sd.xa2->CreateMasteringVoice(&g_sd.masterVoice, inputChannels, inputSampleRate, 0, g_sd.deviceIndex < 0 ? 0 : g_sd.deviceIndex, 0);

    iassert(hr == S_OK);
    
    if ( g_sd.masterVoice )
    {
        g_sd.masterVoice->GetVoiceDetails(&g_sd.masterVoiceDetails);
        iassert(g_sd.masterVoiceDetails.InputChannels <= 8);

        g_sd.masterBus = SD_CreateBus(
                                             g_sd.masterVoiceDetails.InputChannels,
                                             g_sd.masterVoiceDetails.InputChannels,
                                             48000,
                                             3u,
                                             &g_sd.masterDsp,
                                             g_sd.masterVoice);
        g_sd.novoiceBus = SD_CreateBus(
                                                g_sd.masterVoiceDetails.InputChannels,
                                                g_sd.masterVoiceDetails.InputChannels,
                                                48000,
                                                2u,
                                                &g_sd.novoiceDsp,
                                                g_sd.masterBus);
        g_sd.radverbBus = SD_CreateBus(
                                                4u,
                                                g_sd.masterVoiceDetails.InputChannels,
                                                48000,
                                                1u,
                                                &g_sd.radverbDsp,
                                                g_sd.novoiceBus);
        iassert(g_sd.masterVoiceDetails.InputChannels <= 8);

        for ( i = 0; i < 64; ++i )
            matrix[i] = 0.0f;
        switch ( g_sd.masterVoiceDetails.InputChannels )
        {
            case 1u:
                matrix[0] = sqrtf(0.25);
                matrix[1] = matrix[0];
                matrix[2] = matrix[0];
                matrix[3] = matrix[0];
                break;
            case 2u:
            case 3u:
            case 4u:
            case 5u:
            case 7u:
                memset(matrix, 0, 12);
                matrix[3] = 1.0f;
                matrix[4] = 0.0f;
                matrix[5] = 1.0f;
                matrix[6] = 0.0f;
                matrix[7] = 0.0f;
                break;
            case 6u:
                memset(matrix, 0, 12);
                matrix[3] = 1.0f;
                matrix[4] = 0.0f;
                matrix[5] = 1.0f;
                matrix[6] = 0.0f;
                matrix[7] = 0.0f;
                matrix[16] = 1.0f;
                memset(&matrix[17], 0, 20);
                matrix[22] = 1.0f;
                matrix[23] = 0.0f;
                break;
            case 8u:
                memset(matrix, 0, 12);
                matrix[3] = 1.0f;
                matrix[4] = 0.0f;
                matrix[5] = 1.0f;
                matrix[6] = 0.0f;
                matrix[7] = 0.0f;
                matrix[16] = sqrtf(0.5);
                memset(&matrix[17], 0, 20);
                matrix[22] = matrix[16];
                matrix[23] = 0.0f;
                matrix[24] = matrix[16];
                memset(&matrix[25], 0, 20);
                matrix[30] = matrix[16];
                matrix[31] = 0.0f;
                break;
            default:
                break;
        }
        hr = g_sd.radverbBus->SetOutputMatrix(
                     g_sd.novoiceBus,
                     4u,
                     g_sd.masterVoiceDetails.InputChannels,
                     matrix,
                     0);
        iassert(hr == S_OK);
        
        Snd_SetConfigStringsBySpeakerCount(g_sd.masterVoiceDetails.InputChannels);
        for ( index = 0; index < SND_GetSpeakerConfigCount(); ++index )
        {
            if ( g_sd.masterVoiceDetails.InputChannels == Snd_GetSpeakerConfig(index)->speakerCount )
            {
                Dvar_SetInt((dvar_s *)snd_speakerConfiguration, index);
                return;
            }
        }
    }
    else
    {
        Com_PrintError(9, "Unable to initialize master voice\n");
    }
}

void SD_Shutdown()
{
    SND_ShutdownMasterVoice();
    SD_XAudio2Done();
}

void SND_ShutdownMasterVoice()
{
    g_sd.xa2->StopEngine();

    for (int i = 0; i < SND_MAX_VOICES; i = i + 1)
    {
        SND_StopVoice(i);
    }

    g_sd.radverbBus->DestroyVoice();
    g_sd.radverbBus = 0;

    g_sd.novoiceBus->DestroyVoice();
    g_sd.novoiceBus = 0;

    g_sd.masterBus->DestroyVoice();
    g_sd.masterBus = 0;

    g_sd.masterVoice->DestroyVoice();
    g_sd.masterVoice = 0;

    g_sd.xa2->Release();
    g_sd.xa2 = 0;

    SDXA2_VoiceDspAssertUnused();
}

void __cdecl SD_PreUpdate()
{
    unsigned int CurrentReverb; // eax
    float v2; // xmm0_4
    float v3; // [esp+0h] [ebp-24h]
    float v4; // [esp+8h] [ebp-1Ch]
    unsigned int i; // [esp+18h] [ebp-Ch]
    const snd_radverb *radverb; // [esp+1Ch] [ebp-8h]
    const snd_master *master; // [esp+20h] [ebp-4h]

    SND_RvParamsDefault(&g_sd.radverbParams);
    CurrentReverb = SND_GetCurrentReverb();
    radverb = SND_GetRadverb(CurrentReverb);
    g_sd.radverbParams.frameRate = 48000.0f;
    if (radverb)
    {
        g_sd.radverbParams.smoothing = radverb->smoothing;
        g_sd.radverbParams.earlyTime = radverb->earlyTime;
        g_sd.radverbParams.lateTime = radverb->lateTime;
        g_sd.radverbParams.earlyGain = radverb->earlyGain;
        g_sd.radverbParams.lateGain = radverb->lateGain;
        g_sd.radverbParams.lateGainProx[3] = g_snd.proximityLateModifier[0];
        g_sd.radverbParams.lateGainProx[1] = g_snd.proximityLateModifier[1];
        g_sd.radverbParams.lateGainProx[2] = g_snd.proximityLateModifier[2];
        g_sd.radverbParams.lateGainProx[0] = g_snd.proximityLateModifier[3];
        g_sd.radverbParams.returnGain = radverb->returnGain;
        g_sd.radverbParams.earlyLpf = radverb->earlyLpf;
        g_sd.radverbParams.lateLpf = radverb->lateLpf;
        g_sd.radverbParams.inputLpf = radverb->inputLpf;
        g_sd.radverbParams.dampLpf = radverb->dampLpf;
        for (i = 0; i < 4; ++i)
            g_sd.radverbParams.wallReflect[i] = radverb->wallReflect;
        g_sd.radverbParams.dryGain = radverb->dryGain;
        g_sd.radverbParams.earlySize = radverb->earlySize;
        g_sd.radverbParams.lateSize = radverb->lateSize;
        g_sd.radverbParams.diffusion = radverb->diffusion;
        g_sd.radverbParams.delayMatrix = snd_radverb_matrix->current.color[0];
        if (SND_ActiveListenerCount() == 1)
        {
            //v2 = __libm_sse2_atan2(g_snd.listeners[0].orient.axis[0][0]);
            v2 = atan2(g_snd.listeners[0].orient.axis[0][0], g_snd.listeners[0].orient.axis[0][1]);
            if ((float)((float)(v2 + 3.1415927) - 6.2831855) < 0.0)
                v4 = v2 + 3.1415927;
            else
                v4 = 6.2831855f;
            if ((float)(0.0 - (float)(v2 + 3.1415927)) < 0.0)
                v3 = v4;
            else
                v3 = 0.0f;
            g_sd.radverbParams.angle = v3;
        }
        else
        {
            g_sd.radverbParams.angle = 0.0f;
        }
    }

    iassert(g_sd.radverbParams.frameRate > 1000.0f);
    iassert(g_sd.radverbParams.frameRate < 100000.0f);

    g_sd.radverbBus->SetEffectParameters(0, &g_sd.radverbParams, 100, 0);
    memset(&g_sd.masterParams, 0, sizeof(g_sd.masterParams));
    master = SND_GetMasterCurrent();
    g_sd.masterParams.lowE = master->lowE;
    g_sd.masterParams.lowG = master->lowG;
    g_sd.masterParams.lowF = master->lowF;
    g_sd.masterParams.lowQ = master->lowQ;
    g_sd.masterParams.peak1E = master->peak1E;
    g_sd.masterParams.peak1G = master->peak1G;
    g_sd.masterParams.peak1F = master->peak1F;
    g_sd.masterParams.peak1Q = master->peak1Q;
    g_sd.masterParams.peak2E = master->peak2E;
    g_sd.masterParams.peak2G = master->peak2G;
    g_sd.masterParams.peak2F = master->peak2F;
    g_sd.masterParams.peak2Q = master->peak2Q;
    g_sd.masterParams.hiE = master->hiE;
    g_sd.masterParams.hiG = master->hiG;
    g_sd.masterParams.hiF = master->hiF;
    g_sd.masterParams.hiQ = master->hiQ;
    g_sd.masterParams.eqG = master->eqG;
    g_sd.masterParams.limitE = master->limitE;
    g_sd.masterParams.limitPG = master->limitPG;
    g_sd.masterParams.limitMG = master->limitMG;
    g_sd.masterParams.limitT = master->limitT;
    g_sd.masterParams.limitR = master->limitR;
    g_sd.masterParams.limitTA = master->limitTA;
    g_sd.masterParams.limitTR = master->limitTR;
    g_sd.masterBus->SetEffectParameters(0, &g_sd.masterParams, 96u, 0);
    g_sd.novoiceParams.compE = 0.0;
    g_sd.novoiceParams.compPG = 0.0;
    g_sd.novoiceParams.compMG = 0.0;
    g_sd.novoiceParams.compT = 0.0;
    g_sd.novoiceParams.compR = 0.0;
    g_sd.novoiceParams.compTA = 0.0;
    g_sd.novoiceParams.compTR = 0.0;
    g_sd.novoiceParams = *(snd_dsp_master_no_voice_params *)&master->compE;
    g_sd.novoiceBus->SetEffectParameters(0, &g_sd.novoiceParams, 28u, 0);
}

void __cdecl SD_PauseVoice(int voiceIndex)
{
    g_snd.voice[voiceIndex].paused = 1;
    if (g_snd.voice[voiceIndex].soundFileInfo.loadingState != SFLS_LOADING)
        g_sd.voices[voiceIndex]->Stop(0, 0);
}

void __cdecl SD_UnpauseVoice(int voiceIndex)
{
    if (g_snd.voice[voiceIndex].soundFileInfo.loadingState != SFLS_LOADING)
        g_sd.voices[voiceIndex]->Start(0, 0);
}

void __cdecl SD_UpdateVoice(unsigned int voiceIndex)
{
    HRESULT hr; // [esp+1Ch] [ebp-18h]
    XAUDIO2_VOICE_STATE state; // [esp+20h] [ebp-14h] BYREF
    snd_voice_t *voice; // [esp+30h] [ebp-4h]

    bcassert(voiceIndex, SND_MAX_VOICES);
    iassert(!SND_IsVoiceFree(voiceIndex));

    if (!SND_IsStream(voiceIndex) || (SD_UpdateStreamVoice(voiceIndex), g_snd.voiceAliasHash[voiceIndex]))
    {
        if (g_snd.voice[voiceIndex].soundFileInfo.loadingState != SFLS_LOADING)
        {
            voice = &g_snd.voice[voiceIndex];
            SDXA2_UpdateVoiceSends(voiceIndex);
            g_sd.voices[voiceIndex]->GetState(&state);
            if (!voice->startDelay && !state.SamplesPlayed)
            {
                if (state.BuffersQueued)
                {
                    if (!voice->paused)
                    {
                        hr = g_sd.voices[voiceIndex]->Start(0, 0);
                        iassert(hr == S_OK);
                    }
                }
            }
            if (state.BuffersQueued)
            {
                g_sd.voices[voiceIndex]->SetFrequencyRatio(SND_GetPitch(voice), 0);
            }
            else
            {
                SND_StopVoice(voiceIndex);
            }
        }
    }
}

void __cdecl SDXA2_UpdateVoiceSends(int voiceIndex)
{
    unsigned int out; // [esp+Ch] [ebp-13Ch]
    unsigned int j; // [esp+14h] [ebp-134h]
    unsigned int channel; // [esp+18h] [ebp-130h]
    HRESULT hr; // [esp+1Ch] [ebp-12Ch]
    unsigned int ii; // [esp+20h] [ebp-128h]
    unsigned int n; // [esp+24h] [ebp-124h]
    unsigned int m; // [esp+28h] [ebp-120h]
    unsigned int k; // [esp+2Ch] [ebp-11Ch]
    unsigned int i; // [esp+30h] [ebp-118h]
    unsigned int outputChannels; // [esp+34h] [ebp-114h]
    snd_voice_t *voice; // [esp+38h] [ebp-110h]
    unsigned int inputChannels; // [esp+3Ch] [ebp-10Ch]
    IXAudio2SourceVoice *xvoice; // [esp+40h] [ebp-108h]
    VoiceInfo *params; // [esp+44h] [ebp-104h]
    float volumes[64]; // [esp+48h] [ebp-100h] BYREF

    iassert(!SND_IsVoiceFree(voiceIndex));

    voice = &g_snd.voice[voiceIndex];
    xvoice = g_sd.voices[voiceIndex];
    params = &g_sd.voiceInfos[voiceIndex];

    params->params.lpfAttenuation = SND_GetLpfLevel(voice);
    params->params.lpfRatio = 0.25f;
    params->params.frameRate = 48000.0f;
    params->params.futz.blend = voice->futzBlend;
    params->params.futz.bpfF = snd_futz_bpf_f->current.value;
    params->params.futz.bpfQ = snd_futz_bpf_q->current.value;
    params->params.futz.lsG = snd_futz_ls_g->current.value;
    params->params.futz.lsF = snd_futz_ls_f->current.value;
    params->params.futz.lsQ = snd_futz_ls_q->current.value;
    params->params.futz.distortion = snd_futz_distortion->current.value;
    params->params.futz.pregain = snd_futz_preg->current.value;
    params->params.futz.postgain = snd_futz_postg->current.value;
    params->params.futz.squelch.tg = snd_futz_tg->current.value;
    params->params.futz.squelch.th = snd_futz_th->current.value;
    params->params.futz.preclip = snd_futz_clip_pre->current.value;
    params->params.futz.postclip = snd_futz_clip_post->current.value;

    xvoice->SetEffectParameters(0, params, 64u, 0);

    inputChannels = voice->pan.input_channel_count;
    outputChannels = voice->pan.output_channel_count;

    for (i = 0; i < 64; ++i)
    {
        volumes[i] = 0.0f;
    }

    switch (outputChannels)
    {
    case 1:
        for (k = 0; k < inputChannels; ++k)
        {
            volumes[k] = Snd_SpeakerMapGetVolume( &voice->pan, k, 0) * 0.5 * voice->wetLevel;
            volumes[inputChannels + k] = Snd_SpeakerMapGetVolume( &voice->pan, k, 0) * 0.5 * voice->wetLevel;
            volumes[2 * inputChannels + k] = Snd_SpeakerMapGetVolume( &voice->pan, k, 0) * 0.5 * voice->wetLevel;
            volumes[3 * inputChannels + k] = Snd_SpeakerMapGetVolume( &voice->pan, k, 0) * 0.5 * voice->wetLevel;
        }
        break;
    case 2:
        for (m = 0; m < inputChannels; ++m)
        {
            volumes[m] = Snd_SpeakerMapGetVolume( &voice->pan, m, 0) * 0.70710677 * voice->wetLevel;
            volumes[inputChannels + m] = Snd_SpeakerMapGetVolume( &voice->pan, m, 1) * 0.70710677 * voice->wetLevel;
            volumes[2 * inputChannels + m] = Snd_SpeakerMapGetVolume( &voice->pan, m, 1) * 0.70710677 * voice->wetLevel;
            volumes[3 * inputChannels + m] = Snd_SpeakerMapGetVolume( &voice->pan, m, 0) * 0.70710677 * voice->wetLevel;
        }
        break;
    case 6:
        for (n = 0; n < inputChannels; ++n)
        {
            volumes[n] = Snd_SpeakerMapGetVolume( &voice->pan, n, 4) * voice->wetLevel;
            volumes[inputChannels + n] = Snd_SpeakerMapGetVolume( &voice->pan, n, 1) * voice->wetLevel;
            volumes[2 * inputChannels + n] = Snd_SpeakerMapGetVolume( &voice->pan, n, 5) * voice->wetLevel;
            volumes[3 * inputChannels + n] = Snd_SpeakerMapGetVolume( &voice->pan, n, 0) * voice->wetLevel;
            volumes[3 * inputChannels + n] = Snd_SpeakerMapGetVolume( &voice->pan, n, 2) * 0.70710677 * voice->wetLevel
                + volumes[3 * inputChannels + n];
            volumes[inputChannels + n] = Snd_SpeakerMapGetVolume( &voice->pan, n, 2) * 0.70710677 * voice->wetLevel
                + volumes[inputChannels + n];
        }
        break;
    case 8:
        for (ii = 0; ii < inputChannels; ++ii)
        {
            volumes[inputChannels + ii] = Snd_SpeakerMapGetVolume( &voice->pan, ii, 1) * voice->wetLevel;
            volumes[3 * inputChannels + ii] = Snd_SpeakerMapGetVolume( &voice->pan, ii, 0) * voice->wetLevel;
            volumes[3 * inputChannels + ii] = Snd_SpeakerMapGetVolume( &voice->pan, ii, 2) * 0.70710677 * voice->wetLevel
                + volumes[3 * inputChannels + ii];
            volumes[inputChannels + ii] = Snd_SpeakerMapGetVolume( &voice->pan, ii, 2) * 0.70710677 * voice->wetLevel
                + volumes[inputChannels + ii];
            volumes[2 * inputChannels + ii] = Snd_SpeakerMapGetVolume( &voice->pan, ii, 5) * 0.70710677 * voice->wetLevel;
            volumes[2 * inputChannels + ii] = Snd_SpeakerMapGetVolume( &voice->pan, ii, 7) * 0.70710677 * voice->wetLevel
                + volumes[2 * inputChannels + ii];
            volumes[ii] = Snd_SpeakerMapGetVolume( &voice->pan, ii, 6) * 0.70710677 * voice->wetLevel;
            volumes[ii] = Snd_SpeakerMapGetVolume( &voice->pan, ii, 4) * 0.70710677 * voice->wetLevel + volumes[ii];
        }
        break;
    }
    hr = g_sd.voices[voiceIndex]->SetOutputMatrix(g_sd.radverbBus, inputChannels, 4u, volumes, 0);
    iassert(hr == S_OK);
    
    for (channel = 0; channel < outputChannels; ++channel)
    {
        for (j = 0; j < inputChannels; ++j)
        {
            out = SDXA2_ChannelFixup(outputChannels, channel);
            volumes[j + channel * inputChannels] = Snd_SpeakerMapGetVolume( &voice->pan, j, out) * voice->dryLevel;
        }
    }
    if ((voice->alias->flags & 0x3000) >> 12 == 2)
    {
        hr = xvoice->SetOutputMatrix(g_sd.masterBus, inputChannels, outputChannels, volumes, 0);
    }
    else
    {
        hr = xvoice->SetOutputMatrix(g_sd.novoiceBus, inputChannels, outputChannels, volumes, 0);
    }

    iassert(hr == S_OK);
}

int __cdecl SD_StartAlias(SndStartAliasInfo *startAliasInfo, unsigned int voice)
{
    bcassert(voice, SND_MAX_VOICES);
    
    if ( SND_IsStream(voice) )
        return SND_StartAliasStream(startAliasInfo, voice);
    else
        return SND_StartAliasRam(startAliasInfo, voice);
}

int __cdecl SND_StartAliasRam(SndStartAliasInfo *startAliasInfo, int voiceIndex)
{
    unsigned int total_msec; // [esp+20h] [ebp-18h]
    unsigned int rate; // [esp+28h] [ebp-10h]
    snd_asset *snd; // [esp+30h] [ebp-8h]
    bool isLooping; // [esp+34h] [ebp-4h]

    iassert(SND_IsVoiceFree(voiceIndex));

    isLooping = (startAliasInfo->alias->flags & 1) != 0;
    snd = &startAliasInfo->alias->soundFile->u.loadSnd->sound;
    if (!iSND_CreateVoice(voiceIndex, snd, isLooping, (startAliasInfo->alias->flags & 0x3000) >> 12))
    {
        return -1;
    }
    rate = snd->frame_rate;

    if ( isLooping )
        total_msec = 0;
    else
        total_msec = startAliasInfo->startDelay + (__int64)((double)snd->frame_count * 1000.0 / (double)rate);

    SND_SetSoundFileVoiceInfo(voiceIndex, snd->channel_count, rate, total_msec, 0, SFLS_LOADED);
    SND_SetVoiceStartInfo(voiceIndex, startAliasInfo);

    iassert(startAliasInfo->playbackId != SND_PLAYBACKID_NOTPLAYED);

    return startAliasInfo->playbackId;
}