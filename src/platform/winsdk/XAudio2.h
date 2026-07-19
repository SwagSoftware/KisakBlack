// XAudio2.h — portable XAudio2 surface for non-Windows builds.
//
// The decompiled sound system (snd_driver_xaudio2.*) is built directly on XAudio2:
// it creates an IXAudio2 engine, mastering/submix/source voices, submits buffers, and
// hangs XAPO effect chains off the voices. This header reconstructs that interface
// surface as plain abstract C++ classes (only the methods the engine calls), to be
// implemented by the OpenAL backend in src/audio_openal — the same translation-layer
// pattern as d3d9.h → src/gfx_gl.
#ifndef KISAK_XAUDIO2_H
#define KISAK_XAUDIO2_H

#include "windows.h"
#include "XAPOBase.h"   // WAVEFORMATEX + CXAPOBase (XAUDIO2_EFFECT_DESCRIPTOR.pEffect)

// ---- ADPCM wave format (the sound header redeclares its own tag) ------------
typedef struct { short iCoef1, iCoef2; } ADPCMCOEFSET;
typedef struct {
    WAVEFORMATEX wfx;
    WORD         wSamplesPerBlock, wNumCoef;
    ADPCMCOEFSET aCoef[1];
} ADPCMWAVEFORMAT;

typedef UINT32 XAUDIO2_WINDOWS_PROCESSOR_SPECIFIER;

// ---- Flags / constants the engine references --------------------------------
#define XAUDIO2_END_OF_STREAM   0x0040
#define XAUDIO2_ANY_PROCESSOR   0xFFFFFFFF
#define XAUDIO2_COMMIT_NOW      0
#define XAUDIO2_LOOP_INFINITE   255
#define XAUDIO2_DEFAULT_CHANNELS   0
#define XAUDIO2_DEFAULT_SAMPLERATE 0
enum { NotDefaultDevice = 0, DefaultRenderDevice = 1, DefaultCaptureDevice = 2, GlobalDefaultDevice = 4 };

// ---- Structs ----------------------------------------------------------------
typedef struct XAUDIO2_BUFFER {
    UINT32      Flags, AudioBytes;
    const BYTE *pAudioData;
    UINT32      PlayBegin, PlayLength, LoopBegin, LoopLength, LoopCount;
    void       *pContext;
} XAUDIO2_BUFFER;

typedef struct XAUDIO2_BUFFER_WMA {
    const UINT32 *pDecodedPacketCumulativeBytes;
    UINT32        PacketCount;
} XAUDIO2_BUFFER_WMA;

typedef struct XAUDIO2_VOICE_STATE {
    void  *pCurrentBufferContext;
    UINT32 BuffersQueued;
    UINT64 SamplesPlayed;
} XAUDIO2_VOICE_STATE;

typedef struct XAUDIO2_DEVICE_DETAILS {
    WCHAR  DeviceID[256];
    WCHAR  DisplayName[256];
    int    Role;
    WAVEFORMATEXTENSIBLE OutputFormat;
} XAUDIO2_DEVICE_DETAILS;

typedef struct XAUDIO2_VOICE_DETAILS {
    UINT32 CreationFlags, ActiveFlags, InputChannels, InputSampleRate;
} XAUDIO2_VOICE_DETAILS;

typedef struct XAUDIO2_PERFORMANCE_DATA {
    UINT64 AudioCyclesSinceLastQuery, TotalCyclesSinceLastQuery;
    UINT32 MinimumCyclesPerQuantum, MaximumCyclesPerQuantum;
    UINT32 MemoryUsageInBytes, CurrentLatencyInSamples, GlitchesSinceEngineStarted;
    UINT32 ActiveSourceVoiceCount, TotalSourceVoiceCount;
    UINT32 ActiveSubmixVoiceCount, ActiveResamplerCount, ActiveMatrixMixCount;
    UINT32 ActiveXmaSourceVoices, ActiveXmaStreams;
} XAUDIO2_PERFORMANCE_DATA;

struct IXAudio2Voice;

typedef struct XAUDIO2_SEND_DESCRIPTOR {
    UINT32          Flags;
    IXAudio2Voice  *pOutputVoice;
} XAUDIO2_SEND_DESCRIPTOR;

typedef struct XAUDIO2_VOICE_SENDS {
    UINT32                   SendCount;
    XAUDIO2_SEND_DESCRIPTOR *pSends;
} XAUDIO2_VOICE_SENDS;

typedef struct XAUDIO2_EFFECT_DESCRIPTOR {
    CXAPOBase *pEffect;     // the engine stores its SDXA2 effects here
    BOOL       InitialState;
    UINT32     OutputChannels;
} XAUDIO2_EFFECT_DESCRIPTOR;

typedef struct XAUDIO2_EFFECT_CHAIN {
    UINT32                     EffectCount;
    XAUDIO2_EFFECT_DESCRIPTOR *pEffectDescriptors;
} XAUDIO2_EFFECT_CHAIN;

// ---- Voice callback (used as a base class -> full definition) ---------------
struct IXAudio2VoiceCallback {
    virtual void WINAPI OnVoiceProcessingPassStart(UINT32 BytesRequired) = 0;
    virtual void WINAPI OnVoiceProcessingPassEnd() = 0;
    virtual void WINAPI OnStreamEnd() = 0;
    virtual void WINAPI OnBufferStart(void *pBufferContext) = 0;
    virtual void WINAPI OnBufferEnd(void *pBufferContext) = 0;
    virtual void WINAPI OnLoopEnd(void *pBufferContext) = 0;
    virtual void WINAPI OnVoiceError(void *pBufferContext, HRESULT Error) = 0;
};

// ---- Voice interfaces -------------------------------------------------------
struct IXAudio2Voice {
    virtual void    WINAPI GetVoiceDetails(XAUDIO2_VOICE_DETAILS *pVoiceDetails) = 0;
    virtual HRESULT WINAPI SetOutputMatrix(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, const float *pLevelMatrix, UINT32 OperationSet) = 0;
    virtual HRESULT WINAPI SetEffectParameters(UINT32 EffectIndex, const void *pParameters, UINT32 ParametersByteSize, UINT32 OperationSet) = 0;
    virtual void    WINAPI GetState(XAUDIO2_VOICE_STATE *pVoiceState) = 0;
    virtual void    WINAPI DestroyVoice() = 0;
};

struct IXAudio2SourceVoice : public IXAudio2Voice {
    virtual HRESULT WINAPI Start(UINT32 Flags, UINT32 OperationSet) = 0;
    virtual HRESULT WINAPI Stop(UINT32 Flags, UINT32 OperationSet) = 0;
    virtual HRESULT WINAPI SubmitSourceBuffer(const XAUDIO2_BUFFER *pBuffer, const XAUDIO2_BUFFER_WMA *pBufferWMA) = 0;
    virtual HRESULT WINAPI SetFrequencyRatio(float Ratio, UINT32 OperationSet) = 0;
};

struct IXAudio2SubmixVoice : public IXAudio2Voice {};

struct IXAudio2MasteringVoice : public IXAudio2Voice {};

// ---- The engine object ------------------------------------------------------
struct IXAudio2 : public IUnknown {
    virtual HRESULT WINAPI CreateSourceVoice(IXAudio2SourceVoice **ppSourceVoice, const WAVEFORMATEX *pSourceFormat, UINT32 Flags, float MaxFrequencyRatio, IXAudio2VoiceCallback *pCallback, const XAUDIO2_VOICE_SENDS *pSendList, const XAUDIO2_EFFECT_CHAIN *pEffectChain) = 0;
    virtual HRESULT WINAPI CreateSubmixVoice(IXAudio2SubmixVoice **ppSubmixVoice, UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags, UINT32 ProcessingStage, const XAUDIO2_VOICE_SENDS *pSendList, const XAUDIO2_EFFECT_CHAIN *pEffectChain) = 0;
    virtual HRESULT WINAPI CreateMasteringVoice(IXAudio2MasteringVoice **ppMasteringVoice, UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags, UINT32 DeviceIndex, const XAUDIO2_EFFECT_CHAIN *pEffectChain) = 0;
    virtual HRESULT WINAPI GetDeviceCount(UINT32 *pCount) = 0;
    virtual HRESULT WINAPI GetDeviceDetails(UINT32 Index, XAUDIO2_DEVICE_DETAILS *pDeviceDetails) = 0;
    virtual HRESULT WINAPI StartEngine() = 0;
    virtual void    WINAPI StopEngine() = 0;
};

// Entry point — provided by the OpenAL backend (src/audio_openal).
extern "C" HRESULT WINAPI XAudio2Create(IXAudio2 **ppXAudio2, unsigned int Flags, XAUDIO2_WINDOWS_PROCESSOR_SPECIFIER XAudio2Processor);

#endif // KISAK_XAUDIO2_H
