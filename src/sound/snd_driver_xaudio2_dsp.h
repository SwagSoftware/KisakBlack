#pragma once

#include <universal/assertive.h>
#include "snd_radverb.h"

#define SDXA2_MAX_FRAME_COUNT 480

// The DSP effects derive from CXAPOBase / IXAPOParameters — reconstructed portably in
// XAPOBase.h — so they now compile on every platform; the OpenAL backend
// (src/audio_openal) supplies the XAudio2/XAPO runtime that drives them.
#include <XAPOBase.h>
#include <XAudio2.h>

static const GUID HACK_IID_IXAPOParameters = { 0xA90BC001, 0xE897, 0xE897, { 0x55, 0xE4, 0x9E, 0x47, 0x00, 0x00, 0x00, 0x01 } };

//struct SDXA2Effect : public CXAPOParametersBase  // sizeof=0x3C80
struct SDXA2Effect : public CXAPOBase, public IXAPOParameters // sizeof=0x3C80
{                                       // XREF: SDXA2MasterNoVoiceBusEffect/r
    int locked;
    int started;
    unsigned int frameRate;
    unsigned int frameCount;
    unsigned int channelCount;
    __declspec(align(32)) float interleave[3840];             // XREF: .rdata:stru_D50258/o

    SDXA2Effect(XAPO_REGISTRATION_PROPERTIES *props);
    //SDXA2Effect(XAPO_REGISTRATION_PROPERTIES *pRegistrationProperties, BYTE *pParameterBlocks, UINT32 uParameterBlockByteSize, BOOL fProducer);
    ~SDXA2Effect();

    // IUnknown CANCER

    HRESULT __stdcall QueryInterface(REFIID riid,void **ppvObject)
    {
        if (!ppvObject)
            return E_POINTER;

        *ppvObject = nullptr;

        // IUnknown
        if (riid == __uuidof(IUnknown))
        {
            *ppvObject = static_cast<IUnknown *>(static_cast<CXAPOBase *>(this));
            AddRef();
            return S_OK;
        }

        // IXAPOParameters (your custom IID)
        //if (riid == IID_IXAPOParameters)
        if (riid == HACK_IID_IXAPOParameters)
        {
            *ppvObject = static_cast<IXAPOParameters *>(this);
            AddRef();
            return S_OK;
        }

        // fallback to base implementation
        return CXAPOBase::QueryInterface(riid, ppvObject);
    }

    ULONG __stdcall AddRef()
    {
        return InterlockedIncrement(&this->m_lReferenceCount);
    }

    ULONG __stdcall Release()
    {
        return CXAPOBase::Release();
    }

    HRESULT STDMETHODCALLTYPE LockForProcess(
        unsigned int InputLockedParameterCount,
        const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *in,
        unsigned int OutputLockedParameterCount,
        const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *out);;

    //int __stdcall SDXA2Effect::Release(); CXAPOBase
    void STDMETHODCALLTYPE Reset();
    void STDMETHODCALLTYPE UnlockForProcess();

    void STDMETHODCALLTYPE GetParameters(void *, UINT32)
    {
        iassert(0);
    }

    virtual void __stdcall Process(
        unsigned int InputProcessParameterCount,
        const XAPO_PROCESS_BUFFER_PARAMETERS *pInputProcessParameters,
        unsigned int OutputProcessParameterCount,
        XAPO_PROCESS_BUFFER_PARAMETERS *pOutputProcessParameters,
        int IsEnabled);
    
    virtual void Process(unsigned int channelCount, unsigned int frameCount, float *data) = 0;
};


struct snd_dsp_master_params // sizeof=0x60
{                                       // XREF: SDXA2MasterBusEffect/r
                                        // SoundState/r
    float lowE;                         // XREF: SD_PreUpdate(int)+314/w
    float lowG;                         // XREF: SD_PreUpdate(int)+324/w
    float lowF;                         // XREF: SD_PreUpdate(int)+334/w
    float lowQ;                         // XREF: SD_PreUpdate(int)+344/w
    float peak1E;                       // XREF: SD_PreUpdate(int)+354/w
    float peak1G;                       // XREF: SD_PreUpdate(int)+364/w
    float peak1F;                       // XREF: SD_PreUpdate(int)+374/w
    float peak1Q;                       // XREF: SD_PreUpdate(int)+384/w
    float peak2E;                       // XREF: SD_PreUpdate(int)+394/w
    float peak2G;                       // XREF: SD_PreUpdate(int)+3A4/w
    float peak2F;                       // XREF: SD_PreUpdate(int)+3B4/w
    float peak2Q;                       // XREF: SD_PreUpdate(int)+3C4/w
    float hiE;                          // XREF: SD_PreUpdate(int)+3D4/w
    float hiG;                          // XREF: SD_PreUpdate(int)+3E4/w
    float hiF;                          // XREF: SD_PreUpdate(int)+3F4/w
    float hiQ;                          // XREF: SD_PreUpdate(int)+404/w
    float eqG;                          // XREF: SD_PreUpdate(int)+414/w
    float limitE;                       // XREF: SD_PreUpdate(int)+427/w
    float limitPG;                      // XREF: SD_PreUpdate(int)+43A/w
    float limitMG;                      // XREF: SD_PreUpdate(int)+44D/w
    float limitT;                       // XREF: SD_PreUpdate(int)+460/w
    float limitR;                       // XREF: SD_PreUpdate(int)+473/w
    float limitTA;                      // XREF: SD_PreUpdate(int)+486/w
    float limitTR;                      // XREF: SD_PreUpdate(int)+499/w
};

struct __declspec(align(128)) snd_dsp_dynamo_state // sizeof=0xF80
{                                       // XREF: snd_dsp_master_no_voice_state/r
                                        // snd_dsp_master_state/r
    float lookahead[960];
    unsigned int offset;
    float yfilter;
    float gfilter;
    float g;
};

struct snd_dsp_master_no_voice_state // sizeof=0xF80
{                                       // XREF: SDXA2MasterNoVoiceBusEffect/r
    snd_dsp_dynamo_state comp;
};

struct snd_dsp_biquad_state // sizeof=0x20
{                                       // XREF: snd_dsp_futz_state/r
                                        // snd_dsp_futz_state/r ...
    float y[4];
    float x[4];
};

struct snd_dsp_squelch_state // sizeof=0x8
{                                       // XREF: snd_dsp_futz_state/r
    float l;
    float g;
};

struct snd_dsp_futz_state // sizeof=0x48
{                                       // XREF: snd_dsp_source_state/r
    snd_dsp_biquad_state bpf;
    snd_dsp_biquad_state ls;
    snd_dsp_squelch_state squelch;
};

struct snd_dsp_master_no_voice_params // sizeof=0x1C
{                                       // XREF: SDXA2MasterNoVoiceBusEffect/r
                                        // SoundState/r
    float compE;                        // XREF: SD_Init(void)+AF/w
                                        // SD_PreUpdate(int)+4C1/w ...
    float compPG;                       // XREF: SD_Init(void)+B5/w
                                        // SD_PreUpdate(int)+4C7/w ...
    float compMG;                       // XREF: SD_Init(void)+BB/w
                                        // SD_PreUpdate(int)+4CD/w ...
    float compT;                        // XREF: SD_Init(void)+C1/w
                                        // SD_PreUpdate(int)+4D3/w ...
    float compR;                        // XREF: SD_Init(void)+C7/w
                                        // SD_PreUpdate(int)+4D9/w ...
    float compTA;                       // XREF: SD_Init(void)+CD/w
                                        // SD_PreUpdate(int)+4DF/w ...
    float compTR;                       // XREF: SD_Init(void)+D3/w
                                        // SD_PreUpdate(int)+4E5/w ...
};

struct snd_dsp_biquad_coef // sizeof=0x14
{                                       // XREF: ?SND_DspFutzMono@@YAXPIBUsnd_dsp_futz_param@@PIAUsnd_dsp_futz_state@@MIPIAM22@Z/r
                                        // ?SND_DspFutzMono@@YAXPIBUsnd_dsp_futz_param@@PIAUsnd_dsp_futz_state@@MIPIAM22@Z/r ...
    float s[5];
};

struct snd_dsp_master_state // sizeof=0x1000
{                                       // XREF: SDXA2MasterBusEffect/r
    snd_dsp_biquad_state low;
    snd_dsp_biquad_state peak1;
    snd_dsp_biquad_state peak2;
    snd_dsp_biquad_state hi;
    snd_dsp_dynamo_state limit;
};

struct SDXA2MasterBusEffect : SDXA2Effect // sizeof=0xBD00
{                                       // XREF: SoundState/r
    snd_dsp_master_params params;
    snd_dsp_master_state state[8];

    SDXA2MasterBusEffect();

    void Process(
        unsigned int channelCount,
        unsigned int frameCount,
        float *data);
    void STDMETHODCALLTYPE SetParameters(const void *pParams, unsigned int cbParams); 
};

struct SDXA2RadverbEffect : SDXA2Effect // sizeof=0x87B80
{                                       // XREF: SoundState/r
    snd_rv_state state;                 // XREF: .data:00E1F218/o
                                        // .data:00E1F42C/o ...
    snd_rv_params params;

    float temp[3840];

    SDXA2RadverbEffect();

    void Process(
        unsigned int channelCount,
        unsigned int frameCount,
        float *data);

    void STDMETHODCALLTYPE SetParameters(const void *pParams, unsigned int cbParams);
};


struct snd_dsp_squelch_param // sizeof=0x8
{                                       // XREF: snd_dsp_futz_param/r
    float tg;
    float th;
};

struct snd_dsp_futz_param // sizeof=0x34
{                                       // XREF: snd_dsp_source_params/r
    float blend;
    float bpfF;
    float bpfQ;
    float lsG;
    float lsF;
    float lsQ;
    float distortion;
    float pregain;
    float postgain;
    float preclip;
    float postclip;
    snd_dsp_squelch_param squelch;
};

struct snd_dsp_source_params // sizeof=0x40
{                                       // XREF: SDXA2SourceEffect/r
                                        // VoiceInfo/r
    float frameRate;
    float lpfAttenuation;
    float lpfRatio;
    snd_dsp_futz_param futz;
};

struct snd_dsp_source_state // sizeof=0x4C
{                                       // XREF: SDXA2SourceEffect/r
    float lpfy;
    snd_dsp_futz_state futz;
};

struct SDXA2SourceEffect : public SDXA2Effect // sizeof=0x5B80
{                                       // XREF: SoundState/r
    snd_dsp_source_params params;
    snd_dsp_source_state state[2];

    float tempa[960];
    float tempb[960];

    SDXA2SourceEffect();
    void Clear();
    void Process(
        unsigned int channelCount,
        unsigned int frameCount,
        float *data);
    void STDMETHODCALLTYPE SetParameters(const void *pParams, unsigned int cbParams);
};

struct SDXA2MasterNoVoiceBusEffect : SDXA2Effect // sizeof=0xB900
{                                       // XREF: SoundState/r
    snd_dsp_master_no_voice_params params;
    snd_dsp_master_no_voice_state state[8];

    SDXA2MasterNoVoiceBusEffect();

    void Process(
        unsigned int channelCount,
        unsigned int frameCount,
        float *data);
    void STDMETHODCALLTYPE SetParameters(const void *pParams, unsigned int cbParams);
};










