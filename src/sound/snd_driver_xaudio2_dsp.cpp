#include "snd_driver_xaudio2_dsp.h"
#include "snd_dsp.h"
#include "snd.h"
#include <mjpeg/mjpeg.h>
#include <mjpeg/avi.h>
#include "snd_radverb.h"

XAPO_REGISTRATION_PROPERTIES g_sourceEffectProps = {
    /* clsid                */ { 0u, 1u, 2u, { 3u, 0u, 0u, 0u, 0u, 0u, 0u, 0u } },
    /* FriendlyName[256]    */ L"Source DSP",
    /* CopyrightInfo[256]   */ L"Treyarch",
    /* MajorVersion         */ 1u,
    /* MinorVersion         */ 1u,
    /* Flags                */ XAPO_FLAG_CHANNELS_MUST_MATCH
                                | XAPO_FLAG_FRAMERATE_MUST_MATCH
                                | XAPO_FLAG_BITSPERSAMPLE_MUST_MATCH
                                | XAPO_FLAG_BUFFERCOUNT_MUST_MATCH
                                | XAPO_FLAG_INPLACE_SUPPORTED
                                | XAPO_FLAG_INPLACE_REQUIRED,
    /* MinInputBufferCount  */ 0u,
    /* MaxInputBufferCount  */ 1u,
    /* MinOutputBufferCount */ 0u,
    /* MaxOutputBufferCount */ 1u,
};

XAPO_REGISTRATION_PROPERTIES g_masterNoVoiceEffectProps = {
    /* clsid                */ { 0u, 6u, 1u, { 3u, 0u, 0u, 0u, 0u, 0u, 0u, 0u } },
    /* FriendlyName[256]    */ L"WorldBus DSP",
    /* CopyrightInfo[256]   */ L"Treyarch",
    /* MajorVersion         */ 1u,
    /* MinorVersion         */ 1u,
    /* Flags                */ XAPO_FLAG_CHANNELS_MUST_MATCH
                                | XAPO_FLAG_FRAMERATE_MUST_MATCH
                                | XAPO_FLAG_BITSPERSAMPLE_MUST_MATCH
                                | XAPO_FLAG_BUFFERCOUNT_MUST_MATCH
                                | XAPO_FLAG_INPLACE_SUPPORTED
                                | XAPO_FLAG_INPLACE_REQUIRED,
    /* MinInputBufferCount  */ 0u,
    /* MaxInputBufferCount  */ 1u,
    /* MinOutputBufferCount */ 0u,
    /* MaxOutputBufferCount */ 1u,
};

XAPO_REGISTRATION_PROPERTIES g_masterEffectProps = {
    /* clsid                */ { 0u, 6u, 2u, { 3u, 0u, 0u, 0u, 0u, 0u, 0u, 0u } },
    /* FriendlyName[256]    */ L"Master Bus DSP",
    /* CopyrightInfo[256]   */ L"Treyarch",
    /* MajorVersion         */ 1u,
    /* MinorVersion         */ 1u,
    /* Flags                */ XAPO_FLAG_CHANNELS_MUST_MATCH
                                | XAPO_FLAG_FRAMERATE_MUST_MATCH
                                | XAPO_FLAG_BITSPERSAMPLE_MUST_MATCH
                                | XAPO_FLAG_BUFFERCOUNT_MUST_MATCH
                                | XAPO_FLAG_INPLACE_SUPPORTED
                                | XAPO_FLAG_INPLACE_REQUIRED,
    /* MinInputBufferCount  */ 0u,
    /* MaxInputBufferCount  */ 1u,
    /* MinOutputBufferCount */ 0u,
    /* MaxOutputBufferCount */ 1u,
};

XAPO_REGISTRATION_PROPERTIES g_RadverbEffectProps = {
    /* clsid                */ { 5u, 4u, 3u, { 2u, 0u, 0u, 0u, 0u, 0u, 0u, 0u } },
    /* FriendlyName[256]    */ L"Radverb DSP",
    /* CopyrightInfo[256]   */ L"Treyarch",
    /* MajorVersion         */ 1u,
    /* MinorVersion         */ 1u,
    /* Flags                */ XAPO_FLAG_CHANNELS_MUST_MATCH
                                | XAPO_FLAG_FRAMERATE_MUST_MATCH
                                | XAPO_FLAG_BITSPERSAMPLE_MUST_MATCH
                                | XAPO_FLAG_BUFFERCOUNT_MUST_MATCH
                                | XAPO_FLAG_INPLACE_SUPPORTED
                                | XAPO_FLAG_INPLACE_REQUIRED,
    /* MinInputBufferCount  */ 0u,
    /* MaxInputBufferCount  */ 1u,
    /* MinOutputBufferCount */ 0u,
    /* MaxOutputBufferCount */ 1u,
};



SDXA2Effect::SDXA2Effect(XAPO_REGISTRATION_PROPERTIES *props) : CXAPOBase(props)
//SDXA2Effect::SDXA2Effect(XAPO_REGISTRATION_PROPERTIES *pRegistrationProperties, BYTE *pParameterBlocks, UINT32 uParameterBlockByteSize, BOOL fProducer) 
//    : CXAPOParametersBase(pRegistrationProperties, pParameterBlocks, uParameterBlockByteSize, fProducer)
{
    //CXAPOBase::CXAPOBase(this, props);
    //this->CXAPOBase::IXAPO::IUnknown::__vftable = (SDXA2Effect_vtbl *)&SDXA2Effect::`vftable'{for `CXAPOBase'};
    //this->IXAPOParameters::IUnknown::__vftable = (IXAPOParameters_vtbl *)&SDXA2Effect::`vftable'{for `IXAPOParameters'};
    this->locked = 0;
    this->started = 0;
    memset((unsigned __int8 *)this->interleave, 0, sizeof(this->interleave));
    //return this;
}

SDXA2Effect::~SDXA2Effect()
{
    //this->CXAPOBase::IXAPO::IUnknown::__vftable = (SDXA2Effect_vtbl *)&SDXA2Effect::`vftable'{for `CXAPOBase'};
    //this->IXAPOParameters::IUnknown::__vftable = (IXAPOParameters_vtbl *)&SDXA2Effect::`vftable'{for `IXAPOParameters'};
    //CXAPOBase::~CXAPOBase(this);
}

void STDMETHODCALLTYPE SDXA2Effect::Reset()
{
    iassert(!locked);
    locked = 0;
    started = 0;
}

void STDMETHODCALLTYPE SDXA2Effect::UnlockForProcess()
{
    iassert(locked);
    locked = 0;
}

HRESULT STDMETHODCALLTYPE SDXA2Effect::LockForProcess(
                unsigned int InputLockedParameterCount,
                const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *in,
                unsigned int OutputLockedParameterCount,
                const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *out)
{
    iassert(!locked);

    this->locked = 1;
    this->started = 0;

    iassert(in->MaxFrameCount == out->MaxFrameCount);
    iassert(in->pFormat->wBitsPerSample == out->pFormat->wBitsPerSample);
    iassert(in->pFormat->wFormatTag == out->pFormat->wFormatTag);
    iassert(in->pFormat->nSamplesPerSec == out->pFormat->nSamplesPerSec);
    iassert(in->pFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE);
    iassert(in->pFormat->wBitsPerSample == 32);

    this->frameRate = in->pFormat->nSamplesPerSec;
    this->frameCount = in->MaxFrameCount;
    this->channelCount = in->pFormat->nChannels;

    return 0;
}

void __stdcall SDXA2Effect::Process(
                unsigned int InputProcessParameterCount,
                const XAPO_PROCESS_BUFFER_PARAMETERS *pInputProcessParameters,
                unsigned int OutputProcessParameterCount,
                XAPO_PROCESS_BUFFER_PARAMETERS *pOutputProcessParameters,
                int IsEnabled)
{
    float *input; // [esp+0h] [ebp-8h]
    float *output; // [esp+4h] [ebp-4h]

    iassert(locked);
    iassert(InputProcessParameterCount == 1);
    iassert(OutputProcessParameterCount == 1);
    iassert(pInputProcessParameters[0].ValidFrameCount == frameCount);
    iassert(SDXA2_MAX_FRAME_COUNT >= frameCount);

    if (pInputProcessParameters->BufferFlags == XAPO_BUFFER_SILENT)
    {
        memset(pInputProcessParameters->pBuffer, 0, 4 * this->frameCount * this->channelCount);
    }
    pOutputProcessParameters->BufferFlags = XAPO_BUFFER_VALID;

    input = (float *)pInputProcessParameters->pBuffer;
    output = (float *)pOutputProcessParameters->pBuffer;

    iassert(frameCount == 480);

    SND_DspUninterleave(this->channelCount, this->frameCount, input, this->interleave);
    this->Process(this->channelCount, this->frameCount, this->interleave);
    SND_DspInterleave(this->channelCount, this->frameCount, this->interleave, output);
}

SDXA2SourceEffect::SDXA2SourceEffect() 
    //: SDXA2Effect(&g_sourceEffectProps, (BYTE *)&this->params, sizeof(params), TRUE)
    : SDXA2Effect(&g_sourceEffectProps)
{
    //SDXA2Effect::SDXA2Effect(this, &g_sourceEffectProps);
    //this->SDXA2Effect::CXAPOBase::IXAPO::IUnknown::__vftable = (SDXA2SourceEffect_vtbl *)&SDXA2SourceEffect::`vftable'{for `CXAPOBase'};
    //this->SDXA2Effect::IXAPOParameters::IUnknown::__vftable = (IXAPOParameters_vtbl *)&SDXA2SourceEffect::`vftable'{for `IXAPOParameters'};
    SDXA2SourceEffect::Clear();
    //return this;
}

void SDXA2SourceEffect::Clear()
{
    iassert(!locked);
    iassert(!started);

    memset((unsigned __int8 *)&this->params, 0, sizeof(this->params));
    memset((unsigned __int8 *)this->state, 0, sizeof(this->state));
}

void __thiscall SDXA2SourceEffect::Process(
                unsigned int channelCount,
                unsigned int frameCount,
                float *data)
{
    iassert(channelCount <= SDXA2_MAX_SOURCE_CHANNELS);

    for ( int i = 0; i < channelCount; ++i )
        SND_DspFxSourceMono(
            (const snd_dsp_futz_param *)&this->params,
            &this->state[i],
            frameCount,
            &data[frameCount * i],
            this->tempa,
            this->tempb);
}

void STDMETHODCALLTYPE SDXA2SourceEffect::SetParameters(const void *pParams, unsigned int cbParams)
{
    iassert(cbParams == sizeof(params));
    memcpy(&params, pParams, sizeof(params));
}

SDXA2MasterNoVoiceBusEffect::SDXA2MasterNoVoiceBusEffect() 
    //: SDXA2Effect(&g_masterNoVoiceEffectProps, (BYTE*)&this->params, sizeof(params), TRUE)
    : SDXA2Effect(&g_masterNoVoiceEffectProps)
{
    //SDXA2Effect::SDXA2Effect(this, &g_masterNoVoiceEffectProps);
    //this->SDXA2Effect::CXAPOBase::IXAPO::IUnknown::__vftable = (SDXA2MasterNoVoiceBusEffect_vtbl *)&SDXA2MasterNoVoiceBusEffect::`vftable'{for `CXAPOBase'};
    //this->SDXA2Effect::IXAPOParameters::IUnknown::__vftable = (IXAPOParameters_vtbl *)&SDXA2MasterNoVoiceBusEffect::`vftable'{for `IXAPOParameters'};
    //return this;
}

void __thiscall SDXA2MasterNoVoiceBusEffect::Process(
                unsigned int channelCount,
                unsigned int frameCount,
                float *data)
{
    for ( int i = 0; i < channelCount; ++i )
    {
        SND_DspFxMasterNoVoiceSingleChannel(
            frameCount,
            this->frameRate,
            &data[frameCount * i],
            &this->params,
            &this->state[i],
            &g_snd.meters[i]);
    }
}

void STDMETHODCALLTYPE SDXA2MasterNoVoiceBusEffect::SetParameters(
                const void *pParams,
                unsigned int cbParams)
{
    iassert(cbParams == sizeof(params));
    memcpy(&params, pParams, sizeof(params));
}

SDXA2MasterBusEffect::SDXA2MasterBusEffect() 
    //: SDXA2Effect(&g_masterEffectProps, (BYTE*)&this->params, sizeof(params), TRUE)
    : SDXA2Effect(&g_masterEffectProps)
{
    //SDXA2Effect::SDXA2Effect(this, &g_masterEffectProps);
    //this->SDXA2Effect::CXAPOBase::IXAPO::IUnknown::__vftable = (SDXA2MasterBusEffect_vtbl *)&SDXA2MasterBusEffect::`vftable'{for `CXAPOBase'};
    //this->SDXA2Effect::IXAPOParameters::IUnknown::__vftable = (IXAPOParameters_vtbl *)&SDXA2MasterBusEffect::`vftable'{for `IXAPOParameters'};
    memset((unsigned __int8 *)&this->params, 0, sizeof(this->params));
    memset((unsigned __int8 *)this->state, 0, sizeof(this->state));
    //return this;
}

void __thiscall SDXA2MasterBusEffect::Process(
                unsigned int channelCount,
                unsigned int frameCount,
                float *data)
{
    float frameRate; // [esp+0h] [ebp-24Ch]
    float v5; // [esp+14h] [ebp-238h]
    float v6; // [esp+18h] [ebp-234h]
    unsigned int v7; // [esp+1Ch] [ebp-230h]
    unsigned int channel; // [esp+2Ch] [ebp-220h]
    float frameSample; // [esp+30h] [ebp-21Ch]
    unsigned int frame; // [esp+34h] [ebp-218h]
    __int16 shorts[258]; // [esp+3Ch] [ebp-210h] BYREF
    unsigned int framesProcessed; // [esp+244h] [ebp-8h]
    unsigned int i; // [esp+248h] [ebp-4h]

    for ( i = 0; i < channelCount; ++i )
    {
        frameRate = (float)this->frameRate;
        SND_DspFxMasterSingleChannel(
            frameCount,
            frameRate,
            &data[frameCount * i],
            &this->params,
            &this->state[i],
            &g_snd.meters[i]);
    }
    // lwss: seems like some kind of cinematic hack
    if ( mjpeg_is_encoding() )
    {
        for ( framesProcessed = 0; framesProcessed < frameCount; framesProcessed += v7 )
        {
            if ( frameCount - framesProcessed >= 0x100 )
                v7 = 256;
            else
                v7 = frameCount - framesProcessed;
            for ( frame = 0; frame < v7; ++frame )
            {
                frameSample = 0.0f;
                for ( channel = 0; channel < channelCount; ++channel )
                    frameSample = frameSample + data[frameCount * channel + framesProcessed + frame];
                if ( frameSample >= 1.0 )
                    v6 = 1.0f;
                else
                    v6 = frameSample;
                if ( v6 <= -1.0 )
                    v5 = -1.0f;
                else
                    v5 = v6;
                shorts[frame] = (int)(float)(v5 * 32767.0);
            }
            mjpeg_add_samples(shorts, v7);
        }
    }
}

void STDMETHODCALLTYPE SDXA2MasterBusEffect::SetParameters(const void *pParams, unsigned int cbParams)
{
    iassert(cbParams == sizeof(params));
    memcpy(&params, pParams, 96);
}

SDXA2RadverbEffect::SDXA2RadverbEffect() 
    //: SDXA2Effect(&g_RadverbEffectProps, (BYTE*)&this->params, 96, TRUE)
    : SDXA2Effect(&g_RadverbEffectProps)
{
    //SDXA2Effect::SDXA2Effect(this, &g_RadverbEffectProps);
    //this->SDXA2Effect::CXAPOBase::IXAPO::IUnknown::__vftable = (SDXA2RadverbEffect_vtbl *)&SDXA2RadverbEffect::`vftable'{for `CXAPOBase'};
    //this->SDXA2Effect::IXAPOParameters::IUnknown::__vftable = (IXAPOParameters_vtbl *)&SDXA2RadverbEffect::`vftable'{for `IXAPOParameters'};
    SND_RvParamsDefault(&this->params);
    //return this;
}

void __thiscall SDXA2RadverbEffect::Process(
                unsigned int channelCount,
                unsigned int frameCount,
                float *data)
{
    iassert(channelCount == 4);
    iassert(params.earlySize > 0);

    SND_RvFrame(
        &this->params,
        &this->state,
        (const float *)frameCount,
        &data[3 * frameCount],
        &data[frameCount],
        data,
        &data[2 * frameCount],
        &this->temp[3 * frameCount],
        &this->temp[frameCount],
        this->temp,
        &this->temp[2 * frameCount]);

    memcpy(data, this->temp, frameCount * 4 * channelCount);
}

void STDMETHODCALLTYPE SDXA2RadverbEffect::SetParameters(const void *pParams, unsigned int cbParams)
{
    iassert(cbParams == sizeof(params));
    memcpy(&params, pParams, sizeof(params));
    iassert(params.frameRate > 1000.0f);
    iassert(params.frameRate < 100000.0f);
}

