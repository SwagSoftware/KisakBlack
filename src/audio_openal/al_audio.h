// al_audio.h — OpenAL implementations of the XAudio2 interfaces.
//
// The decompiled sound system (src/sound/snd_driver_xaudio2.*) is written against
// XAudio2; this backend implements those interfaces on OpenAL so the engine runs with
// real audio on Linux — the audio analogue of src/gfx_gl (D3D9→OpenGL).
//
// Scope of this first cut: source voices play through OpenAL sources with a streaming
// buffer queue and working OnBufferEnd callbacks. The submix/mastering bus graph and
// the XAPO DSP effect chains are accepted but not run — OpenAL mixes the sources
// straight to the output device, so sound plays without the reverb/EQ/compressor DSP
// (those are a later enhancement). Mirrors the gfx_gl object/ownership pattern.
#ifndef KISAK_AL_AUDIO_H
#define KISAK_AL_AUDIO_H

#include <XAudio2.h>
#include <AL/al.h>
#include <AL/alc.h>

namespace kisak_al {

// Map a WAVEFORMATEX (PCM) to an OpenAL buffer format; 0 if unsupported.
ALenum AlFormat(const WAVEFORMATEX *fmt);

// ---- Source voice: an OpenAL source + a queue of streamed buffers -----------
class ALSourceVoice final : public IXAudio2SourceVoice {
public:
    ALSourceVoice(const WAVEFORMATEX *fmt, IXAudio2VoiceCallback *cb);

    // IXAudio2Voice
    void    WINAPI GetVoiceDetails(XAUDIO2_VOICE_DETAILS *d) override;
    HRESULT WINAPI SetOutputMatrix(IXAudio2Voice *, UINT32 srcCh, UINT32 dstCh, const float *matrix, UINT32) override;
    HRESULT WINAPI SetEffectParameters(UINT32, const void *, UINT32, UINT32) override { return S_OK; }
    void    WINAPI GetState(XAUDIO2_VOICE_STATE *s) override;
    void    WINAPI DestroyVoice() override;
    // IXAudio2SourceVoice
    HRESULT WINAPI Start(UINT32, UINT32) override;
    HRESULT WINAPI Stop(UINT32, UINT32) override;
    HRESULT WINAPI SubmitSourceBuffer(const XAUDIO2_BUFFER *buf, const XAUDIO2_BUFFER_WMA *) override;
    HRESULT WINAPI SetFrequencyRatio(float ratio, UINT32) override;

private:
    void Service();   // recycle finished buffers, fire OnBufferEnd

    ALuint  source_ = 0;
    ALenum  format_ = 0;
    ALsizei rate_ = 48000;
    UINT32  channels_ = 1;
    IXAudio2VoiceCallback *cb_ = nullptr;
    UINT64  samplesPlayed_ = 0;
    void   *lastContext_ = nullptr;
    void   *ctxQueue_ = nullptr;   // heap std::deque<void*> of per-buffer contexts (kept out of this header)
    bool    endOfStream_ = false;
};

// ---- Submix / mastering voices: routing stubs (OpenAL mixes to the device) --
class ALBusVoice : public IXAudio2Voice {
public:
    ALBusVoice(UINT32 channels, UINT32 rate) : channels_(channels), rate_(rate) {}
    void    WINAPI GetVoiceDetails(XAUDIO2_VOICE_DETAILS *d) override;
    HRESULT WINAPI SetOutputMatrix(IXAudio2Voice *, UINT32, UINT32, const float *, UINT32) override { return S_OK; }
    HRESULT WINAPI SetEffectParameters(UINT32, const void *, UINT32, UINT32) override { return S_OK; }
    void    WINAPI GetState(XAUDIO2_VOICE_STATE *s) override;
    void    WINAPI DestroyVoice() override { delete this; }
protected:
    UINT32 channels_, rate_;
};

class ALSubmixVoice final : public IXAudio2SubmixVoice {
public:
    ALSubmixVoice(UINT32 channels, UINT32 rate) : channels_(channels), rate_(rate) {}
    void    WINAPI GetVoiceDetails(XAUDIO2_VOICE_DETAILS *d) override;
    HRESULT WINAPI SetOutputMatrix(IXAudio2Voice *, UINT32, UINT32, const float *, UINT32) override { return S_OK; }
    HRESULT WINAPI SetEffectParameters(UINT32, const void *, UINT32, UINT32) override { return S_OK; }
    void    WINAPI GetState(XAUDIO2_VOICE_STATE *s) override;
    void    WINAPI DestroyVoice() override { delete this; }
private:
    UINT32 channels_, rate_;
};

class ALMasteringVoice final : public IXAudio2MasteringVoice {
public:
    ALMasteringVoice(UINT32 channels, UINT32 rate) : channels_(channels), rate_(rate) {}
    void    WINAPI GetVoiceDetails(XAUDIO2_VOICE_DETAILS *d) override;
    HRESULT WINAPI SetOutputMatrix(IXAudio2Voice *, UINT32, UINT32, const float *, UINT32) override { return S_OK; }
    HRESULT WINAPI SetEffectParameters(UINT32, const void *, UINT32, UINT32) override { return S_OK; }
    void    WINAPI GetState(XAUDIO2_VOICE_STATE *s) override;
    void    WINAPI DestroyVoice() override { delete this; }
private:
    UINT32 channels_, rate_;
};

// ---- The engine object: owns the OpenAL device/context ----------------------
class ALXAudio2 final : public IXAudio2 {
public:
    ALXAudio2();
    bool Ok() const { return ctx_ != nullptr; }

    // IUnknown
    HRESULT WINAPI QueryInterface(REFIID, void **ppv) override { if (ppv) *ppv = this; ++ref_; return S_OK; }
    ULONG   WINAPI AddRef() override  { return ++ref_; }
    ULONG   WINAPI Release() override { ULONG r = --ref_; if (r == 0) delete this; return r; }

    // IXAudio2
    HRESULT WINAPI CreateSourceVoice(IXAudio2SourceVoice **ppv, const WAVEFORMATEX *fmt, UINT32, float, IXAudio2VoiceCallback *cb, const XAUDIO2_VOICE_SENDS *, const XAUDIO2_EFFECT_CHAIN *) override;
    HRESULT WINAPI CreateSubmixVoice(IXAudio2SubmixVoice **ppv, UINT32 ch, UINT32 rate, UINT32, UINT32, const XAUDIO2_VOICE_SENDS *, const XAUDIO2_EFFECT_CHAIN *) override;
    HRESULT WINAPI CreateMasteringVoice(IXAudio2MasteringVoice **ppv, UINT32 ch, UINT32 rate, UINT32, UINT32, const XAUDIO2_EFFECT_CHAIN *) override;
    HRESULT WINAPI GetDeviceCount(UINT32 *count) override { if (count) *count = 1; return S_OK; }
    HRESULT WINAPI GetDeviceDetails(UINT32 index, XAUDIO2_DEVICE_DETAILS *d) override;
    HRESULT WINAPI StartEngine() override { return S_OK; }
    void    WINAPI StopEngine() override {}

private:
    ~ALXAudio2();
    ALCdevice  *dev_ = nullptr;
    ALCcontext *ctx_ = nullptr;
    ULONG ref_ = 1;
};

} // namespace kisak_al

#endif // KISAK_AL_AUDIO_H
