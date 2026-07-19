#pragma once

#ifdef _WIN32
#include <Windows.h>
#include <mmreg.h>
#endif
#include <XAPOBase.h>
#include <XAudio2.h>
#include <gfx_d3d/r_font.h>
#include "snd.h"
#include "snd_driver_xaudio2_dsp.h"

#define SND_MAX_STREAM_VOICES 10
#define SND_MAX_VOICES 74
#define SND_PLAYBACKID_NOTPLAYED -1


enum snd_stream_status : __int32
{                                       // XREF: ?Snd_StreamStatus@@YA?AW4snd_stream_status@@I@Z/r
                                        // ?Snd_StreamAcquireWindow@@YA?AW4snd_stream_status@@IPAI0PAPAD@Z/r ...
    SND_STREAM_UNUSED   = 0x0,
    SND_STREAM_LOADING  = 0x1,
    SND_STREAM_OK       = 0x2,
    SND_STREAM_STARVING = 0x3,
    SND_STREAM_EOF      = 0x4,
    SND_STREAM_ERROR    = 0x5,
};

//typedef struct ADPCMWAVEFORMAT {
//    WAVEFORMATEX wfx;
//    WORD wSamplesPerBlock;
//    WORD wNumCoef;
//    ADPCMCOEFSET aCoef[1];
//} ADPCMWAVEFORMAT;

// LWSS: the above struct is from the xaudio Sdk and aCoef[] array is variable sized, therefore the dev's just redeclared it with size 7
struct my_adpcmwaveformat_tag // sizeof=0x32
{                                       // XREF: iSND_CreateVoice/r
    WAVEFORMATEX wfx;
    WORD wSamplesPerBlock;
    WORD wNumCoef;
    ADPCMCOEFSET aCoef[7];
};

void __cdecl iSND_ReleaseStreamBuffer(unsigned int streamVoice, unsigned int bufferIndex);

struct StreamVoice : IXAudio2VoiceCallback // sizeof=0x50
{                                       // XREF: SoundState/r
    enum
    {
        BUFFER_COUNT = 2
    };

    XAUDIO2_BUFFER buffers[2];
    unsigned int bufferQueuedCount;

    // Called just before this voice's processing pass begins.
    STDMETHOD_(void, OnVoiceProcessingPassStart) (THIS_ UINT32 BytesRequired)
    {

    }

    // Called just after this voice's processing pass ends.
    STDMETHOD_(void, OnVoiceProcessingPassEnd) (THIS)
    {

    }

    // Called when this voice has just finished playing a buffer stream
    // (as marked with the XAUDIO2_END_OF_STREAM flag on the last buffer).
    STDMETHOD_(void, OnStreamEnd) (THIS)
    {

    }

    // Called when this voice is about to start processing a new buffer.
    STDMETHOD_(void, OnBufferStart) (THIS_ void *pBufferContext)
    {

    }

    // Called when this voice has just finished processing a buffer.
    // The buffer can now be reused or destroyed.
    STDMETHOD_(void, OnBufferEnd) (THIS_ void *pBufferContext)
    {
        iSND_ReleaseStreamBuffer((unsigned int)pBufferContext >> 8, (unsigned __int8)pBufferContext);
    }

    // Called when this voice has just reached the end position of a loop.
    STDMETHOD_(void, OnLoopEnd) (THIS_ void *pBufferContext)
    {

    }

    // Called in the event of a critical error during voice processing,
    // such as a failing xAPO or an error from the hardware XMA decoder.
    // The voice may have to be destroyed and re-created to recover from
    // the error.  The callback arguments report which buffer was being
    // processed when the error occurred, and its HRESULT code.
    STDMETHOD_(void, OnVoiceError) (THIS_ void *pBufferContext, HRESULT Error)
    {
        OutputDebugStringA("StreamVoiceError\n");
    }
};

struct VoiceInfo // sizeof=0x44
{                                       // XREF: SoundState/r
    snd_dsp_source_params params;
    SDXA2SourceEffect *sourceDsp;
};

struct __declspec(align(128)) SoundState // sizeof=0x3EF600
{                                       // XREF: .data:g_sd/r
    int deviceIndex;                    // XREF: SD_XAudio2Done(void)+35/w
    int deviceGUIDValid;                // XREF: SD_XAudio2Done(void)+2B/w
    _GUID deviceGUID;                   // XREF: SD_XAudio2Done(void)+41/w
    IXAudio2 *xa2;                      // XREF: SD_XAudio2Done(void)+3/r
    XAUDIO2_DEVICE_DETAILS details;     // XREF: SD_Init(void)+5D/o
    IXAudio2MasteringVoice *masterVoice; // XREF: SD_Init(void)+6A/w
    XAUDIO2_VOICE_DETAILS masterVoiceDetails; // XREF: SD_Init(void)+76/w
    IXAudio2SubmixVoice *masterBus;     // XREF: iSND_CreateVoice+51F/r
    SDXA2MasterBusEffect masterDsp;     // XREF: SND_InitMasterVoice+122/o
    snd_dsp_master_params masterParams; // XREF: SD_Init(void)+96/o
    IXAudio2SubmixVoice *novoiceBus;    // XREF: iSND_CreateVoice:loc_93EF9C/r
    SDXA2MasterNoVoiceBusEffect novoiceDsp;
    snd_dsp_master_no_voice_params novoiceParams;
    IXAudio2SubmixVoice *radverbBus;    // XREF: iSND_CreateVoice+548/r
    SDXA2RadverbEffect radverbDsp;      // XREF: SND_InitMasterVoice+17C/o
    snd_rv_params radverbParams;        // XREF: SD_Init(void)+E7/o
    SDXA2SourceEffect voiceDsp[148];    // XREF: SDXA2_VoiceDspAllocate(void)+2E/o
    IXAudio2SourceVoice *voices[SND_MAX_VOICES];    // XREF: SD_StopVoice(int)+9/r
    VoiceInfo voiceInfos[SND_MAX_VOICES];           // XREF: iSND_CreateVoice+7D/o
    StreamVoice streamVoices[SND_MAX_STREAM_VOICES];       // XREF: iSND_ReleaseStreamBuffer+64/o
    XAUDIO2_PERFORMANCE_DATA perfData;  // XREF: SD_Init(void)+10C/o

    SoundState() = default;
    ~SoundState() = default;
};

bool __cdecl SD_Xaudio2CanInit();
SDXA2SourceEffect *__cdecl SDXA2_VoiceDspAllocate();
void __cdecl SD_TruncateAudioDeviceNames(Font_s *font, float scale, int size);
void __cdecl SD_XAudio2Done();
unsigned int __cdecl SDXA2_ChannelFixup(unsigned int outputChannels, unsigned int channel);
IXAudio2SubmixVoice *__cdecl SD_CreateBus(
                unsigned int inputChannelCount,
                unsigned int outputChannelCount,
                unsigned int sampleRate,
                unsigned int stage,
                SDXA2Effect *dsp,
                IXAudio2Voice *outputVoice);
void __cdecl SD_StopVoice(int voiceIndex);
unsigned int __cdecl iSND_GetStreamChannel(unsigned int index);
void __cdecl iSND_ReleaseStreamBuffer(unsigned int streamVoice, unsigned int bufferIndex);
int __cdecl SND_StartAliasStream(SndStartAliasInfo *startAliasInfo, unsigned int voiceIndex);
void __cdecl SD_UpdateStreamVoice(int voiceIndex);
int __cdecl iSND_CreateVoice(int voiceIndex, const snd_asset *snd, int isLooping, int bus);
void __cdecl iSND_ChannelError(int voiceIndex);
char __cdecl SD_Init();
void SDXA2_VoiceDspAssertUnused();
bool __cdecl SD_XAudio2Init();
HRESULT __cdecl XAudio2Create(
                IXAudio2 **ppXAudio2,
                unsigned int Flags,
                XAUDIO2_WINDOWS_PROCESSOR_SPECIFIER XAudio2Processor);
char __cdecl SD_XAudio2EnumerateDevices();
char __cdecl SD_XAudio2GetDeviceGUID(wchar_t *inGUID, _GUID *guid, char *outGUID, int size);
char __cdecl SD_XAudio2CheckDevice(
                IXAudio2 *pXAudio2,
                const XAUDIO2_DEVICE_DETAILS *deviceDetails,
                unsigned int deviceIndex);
void SD_SwitchDevice();
void SND_InitMasterVoice();
void SD_Shutdown();
void SND_ShutdownMasterVoice();
void __cdecl SD_PreUpdate();
void __cdecl SD_PauseVoice(int voiceIndex);
void __cdecl SD_UnpauseVoice(int voiceIndex);
void __cdecl SD_UpdateVoice(unsigned int voiceIndex);
void __cdecl SDXA2_UpdateVoiceSends(int voiceIndex);


int __cdecl SD_StartAlias(SndStartAliasInfo *startAliasInfo, unsigned int voice);
int __cdecl SND_StartAliasRam(SndStartAliasInfo *startAliasInfo, int voiceIndex);
