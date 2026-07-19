// XAPOBase.h — portable XAPO (XAudio2 DSP framework) surface for non-Windows builds.
//
// The engine's audio DSP effects (SDXA2Effect and friends in snd_driver_xaudio2_dsp.*)
// derive from CXAPOBase + IXAPOParameters. This header reconstructs just the slice they
// use — the IXAPO/IXAPOParameters interfaces, the CXAPOBase helper base (ref-counting +
// default IXAPO impl), and the XAPO_* structs/flags — as plain C++ so those classes
// compile and (with the OpenAL backend in src/audio_openal) run on Linux. Mirrors the
// d3d9.h translation-layer pattern.
#ifndef KISAK_XAPOBASE_H
#define KISAK_XAPOBASE_H

#include "windows.h"
#include "../compat/msvc_intrin.h"   // InterlockedIncrement/Decrement

// ---- WAVEFORMATEX (shared with XAudio2.h / dsound.h; first include wins) -----
#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_
typedef struct tWAVEFORMATEX {
    WORD  wFormatTag, nChannels;
    DWORD nSamplesPerSec, nAvgBytesPerSec;
    WORD  nBlockAlign, wBitsPerSample, cbSize;
} WAVEFORMATEX, *LPWAVEFORMATEX;
typedef struct {
    WAVEFORMATEX Format;
    union { WORD wValidBitsPerSample, wSamplesPerBlock, wReserved; } Samples;
    DWORD dwChannelMask;
    GUID  SubFormat;
} WAVEFORMATEXTENSIBLE;
#endif
#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM        0x0001
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE
#endif

// ---- XAPO registration / buffer descriptors --------------------------------
typedef struct XAPO_REGISTRATION_PROPERTIES {
    GUID   clsid;
    WCHAR  FriendlyName[256];
    WCHAR  CopyrightInfo[256];
    UINT32 MajorVersion, MinorVersion;
    UINT32 Flags;
    UINT32 MinInputBufferCount, MaxInputBufferCount;
    UINT32 MinOutputBufferCount, MaxOutputBufferCount;
} XAPO_REGISTRATION_PROPERTIES;

// XAPO_FLAG_* registration flags (exact SDK values).
#define XAPO_FLAG_CHANNELS_MUST_MATCH       0x00000001
#define XAPO_FLAG_FRAMERATE_MUST_MATCH      0x00000002
#define XAPO_FLAG_BITSPERSAMPLE_MUST_MATCH  0x00000004
#define XAPO_FLAG_BUFFERCOUNT_MUST_MATCH    0x00000008
#define XAPO_FLAG_INPLACE_SUPPORTED         0x00000010
#define XAPO_FLAG_INPLACE_REQUIRED          0x00000020

typedef enum XAPO_BUFFER_FLAGS { XAPO_BUFFER_SILENT, XAPO_BUFFER_VALID } XAPO_BUFFER_FLAGS;

typedef struct XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS {
    const WAVEFORMATEX *pFormat;
    UINT32              MaxFrameCount;
} XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS;

typedef struct XAPO_PROCESS_BUFFER_PARAMETERS {
    void             *pBuffer;
    XAPO_BUFFER_FLAGS BufferFlags;
    UINT32            ValidFrameCount;
} XAPO_PROCESS_BUFFER_PARAMETERS;

// ---- IXAPO / IXAPOParameters interfaces ------------------------------------
struct IXAPO : public IUnknown {
    virtual HRESULT WINAPI GetRegistrationProperties(XAPO_REGISTRATION_PROPERTIES **ppRegistrationProperties) = 0;
    virtual HRESULT WINAPI IsInputFormatSupported(const WAVEFORMATEX *pOutputFormat, const WAVEFORMATEX *pRequestedInputFormat, WAVEFORMATEX **ppSupportedInputFormat) = 0;
    virtual HRESULT WINAPI IsOutputFormatSupported(const WAVEFORMATEX *pInputFormat, const WAVEFORMATEX *pRequestedOutputFormat, WAVEFORMATEX **ppSupportedOutputFormat) = 0;
    virtual HRESULT WINAPI Initialize(const void *pData, UINT32 DataByteSize) = 0;
    virtual void    WINAPI Reset() = 0;
    virtual HRESULT WINAPI LockForProcess(UINT32 InputLockedParameterCount, const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pInputLockedParameters, UINT32 OutputLockedParameterCount, const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pOutputLockedParameters) = 0;
    virtual void    WINAPI UnlockForProcess() = 0;
    virtual void    WINAPI Process(UINT32 InputProcessParameterCount, const XAPO_PROCESS_BUFFER_PARAMETERS *pInputProcessParameters, UINT32 OutputProcessParameterCount, XAPO_PROCESS_BUFFER_PARAMETERS *pOutputProcessParameters, int IsEnabled) = 0;
    virtual UINT32  WINAPI CalcInputFrames(UINT32 OutputFrameCount) = 0;
    virtual UINT32  WINAPI CalcOutputFrames(UINT32 InputFrameCount) = 0;
};

struct IXAPOParameters : public IUnknown {
    virtual void WINAPI SetParameters(const void *pParameters, UINT32 ParameterByteSize) = 0;
    virtual void WINAPI GetParameters(void *pParameters, UINT32 ParameterByteSize) = 0;
};

// ---- CXAPOBase: the SDK helper base the effects derive from -----------------
// Provides COM ref-counting and default IXAPO behaviour; the engine's effects
// override Reset/LockForProcess/UnlockForProcess/Process. The format-negotiation
// paths are unused by the audio backend, so they just return success.
class CXAPOBase : public IXAPO {
public:
    LONG m_lReferenceCount;
    const XAPO_REGISTRATION_PROPERTIES *m_pRegistrationProperties;
    bool m_fIsLocked;

    CXAPOBase(const XAPO_REGISTRATION_PROPERTIES *pRegistrationProperties)
        : m_lReferenceCount(1), m_pRegistrationProperties(pRegistrationProperties), m_fIsLocked(false) {}
    virtual ~CXAPOBase() {}

    // IUnknown
    HRESULT WINAPI QueryInterface(REFIID, void **ppvObject) override {
        if (!ppvObject) return (HRESULT)0x80004003;  // E_POINTER
        *ppvObject = static_cast<IXAPO *>(this); AddRef(); return 0;
    }
    ULONG WINAPI AddRef() override  { return (ULONG)InterlockedIncrement(&m_lReferenceCount); }
    ULONG WINAPI Release() override {
        ULONG r = (ULONG)InterlockedDecrement(&m_lReferenceCount);
        if (r == 0) delete this;
        return r;
    }

    // IXAPO defaults (effects override the ones they use).
    HRESULT WINAPI GetRegistrationProperties(XAPO_REGISTRATION_PROPERTIES **pp) override {
        if (pp) *pp = const_cast<XAPO_REGISTRATION_PROPERTIES *>(m_pRegistrationProperties); return 0;
    }
    HRESULT WINAPI IsInputFormatSupported(const WAVEFORMATEX *, const WAVEFORMATEX *, WAVEFORMATEX **) override { return 0; }
    HRESULT WINAPI IsOutputFormatSupported(const WAVEFORMATEX *, const WAVEFORMATEX *, WAVEFORMATEX **) override { return 0; }
    HRESULT WINAPI Initialize(const void *, UINT32) override { return 0; }
    void    WINAPI Reset() override {}
    HRESULT WINAPI LockForProcess(UINT32, const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *, UINT32, const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *) override { m_fIsLocked = true; return 0; }
    void    WINAPI UnlockForProcess() override { m_fIsLocked = false; }
    void    WINAPI Process(UINT32, const XAPO_PROCESS_BUFFER_PARAMETERS *, UINT32, XAPO_PROCESS_BUFFER_PARAMETERS *, int) override {}
    UINT32  WINAPI CalcInputFrames(UINT32 n) override  { return n; }
    UINT32  WINAPI CalcOutputFrames(UINT32 n) override { return n; }

    bool IsLocked() const { return m_fIsLocked; }
};

// MSVC's __uuidof(IUnknown) -> our IID_IUnknown (the effects' QueryInterface uses it).
#ifndef IID_IUnknown_DEFINED
#define IID_IUnknown_DEFINED
static const GUID IID_IUnknown = { 0x00000000, 0x0000, 0x0000, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
#endif
#ifndef __uuidof
#define __uuidof(T) IID_##T
#endif

#endif // KISAK_XAPOBASE_H
