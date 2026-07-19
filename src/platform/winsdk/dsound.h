// dsound.h — portable DirectSound surface for non-Windows builds.
//
// Used by the voice-chat ("groupvoice") capture/playback path and the cinematic
// player. The interfaces are reconstructed as plain abstract classes (only the
// methods the engine calls) to be implemented by the OpenAL backend in
// src/audio_openal — same translation-layer pattern as d3d9.h / XAudio2.h.
#ifndef KISAK_DSOUND_H
#define KISAK_DSOUND_H

#include "windows.h"

// WAVEFORMATEX (shared with XAudio2.h / XAPOBase.h; first include wins).
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

// ---- Buffer descriptors -----------------------------------------------------
typedef struct _DSBUFFERDESC {
    DWORD        dwSize, dwFlags, dwBufferBytes, dwReserved;
    LPWAVEFORMATEX lpwfxFormat;
    GUID         guid3DAlgorithm;
} DSBUFFERDESC, *LPDSBUFFERDESC;

typedef struct _DSCBUFFERDESC {
    DWORD        dwSize, dwFlags, dwBufferBytes, dwReserved;
    LPWAVEFORMATEX lpwfxFormat;
    DWORD        dwFXCount;
    void        *lpDSCFXDesc;
} DSCBUFFERDESC, *LPDSCBUFFERDESC;

// Common DirectSound flags the engine sets (exact SDK values).
#define DSBCAPS_CTRLFREQUENCY 0x00000020
#define DSBCAPS_CTRLPAN       0x00000010
#define DSBCAPS_CTRLVOLUME    0x00000080
#define DSBCAPS_LOCSOFTWARE   0x00008000
#define DSBPLAY_LOOPING       0x00000001
#define DSSCL_NORMAL          0x00000001
#define DSSCL_PRIORITY        0x00000002
#define DSCBSTART_LOOPING     0x00000001

// ---- Playback / capture buffer & device interfaces --------------------------
struct IDirectSoundBuffer : public IUnknown {
    virtual HRESULT WINAPI Lock(DWORD dwOffset, DWORD dwBytes, void **ppvAudioPtr1, DWORD *pdwAudioBytes1, void **ppvAudioPtr2, DWORD *pdwAudioBytes2, DWORD dwFlags) = 0;
    virtual HRESULT WINAPI Unlock(void *pvAudioPtr1, DWORD dwAudioBytes1, void *pvAudioPtr2, DWORD dwAudioBytes2) = 0;
    virtual HRESULT WINAPI Play(DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags) = 0;
    virtual HRESULT WINAPI Stop() = 0;
    virtual HRESULT WINAPI SetCurrentPosition(DWORD dwNewPosition) = 0;
    virtual HRESULT WINAPI SetFrequency(DWORD dwFrequency) = 0;
    virtual HRESULT WINAPI GetCurrentPosition(DWORD *pdwCurrentPlayCursor, DWORD *pdwCurrentWriteCursor) = 0;
};
typedef IDirectSoundBuffer *LPDIRECTSOUNDBUFFER;

struct IDirectSoundCaptureBuffer : public IUnknown {
    virtual HRESULT WINAPI Lock(DWORD dwOffset, DWORD dwBytes, void **ppvAudioPtr1, DWORD *pdwAudioBytes1, void **ppvAudioPtr2, DWORD *pdwAudioBytes2, DWORD dwFlags) = 0;
    virtual HRESULT WINAPI Unlock(void *pvAudioPtr1, DWORD dwAudioBytes1, void *pvAudioPtr2, DWORD dwAudioBytes2) = 0;
    virtual HRESULT WINAPI Start(DWORD dwFlags) = 0;
    virtual HRESULT WINAPI Stop() = 0;
    virtual HRESULT WINAPI GetCurrentPosition(DWORD *pdwCapturePosition, DWORD *pdwReadPosition) = 0;
};
typedef IDirectSoundCaptureBuffer *LPDIRECTSOUNDCAPTUREBUFFER;

struct IDirectSoundCapture : public IUnknown {
    virtual HRESULT WINAPI CreateCaptureBuffer(const DSCBUFFERDESC *pcDSCBufferDesc, IDirectSoundCaptureBuffer **ppDSCBuffer, IUnknown *pUnkOuter) = 0;
};
typedef IDirectSoundCapture *LPDIRECTSOUNDCAPTURE;

struct IDirectSound8 : public IUnknown {
    virtual HRESULT WINAPI CreateSoundBuffer(const DSBUFFERDESC *pcDSBufferDesc, IDirectSoundBuffer **ppDSBuffer, IUnknown *pUnkOuter) = 0;
    virtual HRESULT WINAPI SetCooperativeLevel(HWND hwnd, DWORD dwLevel) = 0;
};
typedef IDirectSound8 *LPDIRECTSOUND8;

// Device factories — implemented by the OpenAL backend; default to "no driver".
static inline HRESULT DirectSoundCreate8(const GUID *, IDirectSound8 **ppDS8, void *) {
    if (ppDS8) *ppDS8 = nullptr;
    return (HRESULT)-1;
}
static inline HRESULT DirectSoundCaptureCreate(const GUID *, IDirectSoundCapture **ppDSC, void *) {
    if (ppDSC) *ppDSC = nullptr;
    return (HRESULT)-1;
}
static inline HRESULT DirectSoundCaptureCreate8(const GUID *lpGUID, IDirectSoundCapture **ppDSC, void *p) {
    return DirectSoundCaptureCreate(lpGUID, ppDSC, p);
}

#endif // KISAK_DSOUND_H
