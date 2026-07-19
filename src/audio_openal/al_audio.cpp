// al_audio.cpp — OpenAL implementation of the XAudio2 interfaces (see al_audio.h).
#include "al_audio.h"

#include <deque>
#include <cstdio>

namespace kisak_al {

ALenum AlFormat(const WAVEFORMATEX *fmt) {
    if (!fmt || fmt->wFormatTag != WAVE_FORMAT_PCM) return 0;  // only raw PCM here
    if (fmt->nChannels == 1) return fmt->wBitsPerSample == 8 ? AL_FORMAT_MONO8   : AL_FORMAT_MONO16;
    if (fmt->nChannels == 2) return fmt->wBitsPerSample == 8 ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
    return 0;  // >2ch (e.g. multichannel output formats) are not source formats here
}

// ---- ALSourceVoice ---------------------------------------------------------
// Per-voice queue of the pContext values for buffers still in flight (kept here,
// out of the header, so <deque> doesn't leak into it).
struct VoiceCtx { std::deque<void *> contexts; };

ALSourceVoice::ALSourceVoice(const WAVEFORMATEX *fmt, IXAudio2VoiceCallback *cb) : cb_(cb) {
    if (fmt) { format_ = AlFormat(fmt); rate_ = (ALsizei)fmt->nSamplesPerSec; channels_ = fmt->nChannels; }
    alGenSources(1, &source_);
    ctxQueue_ = new VoiceCtx();
}

void ALSourceVoice::Service() {
    if (!source_) return;
    ALint processed = 0;
    alGetSourcei(source_, AL_BUFFERS_PROCESSED, &processed);
    while (processed-- > 0) {
        ALuint buf = 0;
        alSourceUnqueueBuffers(source_, 1, &buf);
        if (buf) { ALint bits = 16, ch = 1, size = 0;
            alGetBufferi(buf, AL_BITS, &bits); alGetBufferi(buf, AL_CHANNELS, &ch); alGetBufferi(buf, AL_SIZE, &size);
            if (bits && ch) samplesPlayed_ += (UINT64)size / ((bits / 8) * ch);
            alDeleteBuffers(1, &buf);
        }
        VoiceCtx *vc = (VoiceCtx *)ctxQueue_;
        void *context = nullptr;
        if (vc && !vc->contexts.empty()) { context = vc->contexts.front(); vc->contexts.pop_front(); }
        if (cb_) cb_->OnBufferEnd(context);
    }
}

void WINAPI ALSourceVoice::GetVoiceDetails(XAUDIO2_VOICE_DETAILS *d) {
    if (!d) return; d->CreationFlags = 0; d->ActiveFlags = 0;
    d->InputChannels = channels_; d->InputSampleRate = (UINT32)rate_;
}

HRESULT WINAPI ALSourceVoice::SetOutputMatrix(IXAudio2Voice *, UINT32 srcCh, UINT32 dstCh, const float *matrix, UINT32) {
    if (source_ && matrix) {                       // approximate the level matrix as an overall gain
        float g = 0.0f; UINT32 n = srcCh * dstCh; if (!n) n = 1;
        for (UINT32 i = 0; i < n; ++i) { float a = matrix[i] < 0 ? -matrix[i] : matrix[i]; if (a > g) g = a; }
        alSourcef(source_, AL_GAIN, g);
    }
    return S_OK;
}

void WINAPI ALSourceVoice::GetState(XAUDIO2_VOICE_STATE *s) {
    Service();
    if (!s) return;
    ALint queued = 0; if (source_) alGetSourcei(source_, AL_BUFFERS_QUEUED, &queued);
    s->pCurrentBufferContext = lastContext_;
    s->BuffersQueued = (UINT32)queued;
    s->SamplesPlayed = samplesPlayed_;
}

void WINAPI ALSourceVoice::DestroyVoice() {
    if (source_) { alSourceStop(source_);
        ALint q = 0; alGetSourcei(source_, AL_BUFFERS_QUEUED, &q);
        while (q-- > 0) { ALuint b = 0; alSourceUnqueueBuffers(source_, 1, &b); if (b) alDeleteBuffers(1, &b); }
        alDeleteSources(1, &source_); source_ = 0;
    }
    delete (VoiceCtx *)ctxQueue_;
    delete this;
}

HRESULT WINAPI ALSourceVoice::Start(UINT32, UINT32) { if (source_) alSourcePlay(source_); return S_OK; }
HRESULT WINAPI ALSourceVoice::Stop(UINT32, UINT32)  { if (source_) alSourceStop(source_); return S_OK; }

HRESULT WINAPI ALSourceVoice::SubmitSourceBuffer(const XAUDIO2_BUFFER *buf, const XAUDIO2_BUFFER_WMA *) {
    Service();
    if (!buf || !source_) return S_OK;
    lastContext_ = buf->pContext;
    if (format_ && buf->pAudioData && buf->AudioBytes) {
        ALuint b = 0; alGenBuffers(1, &b);
        alBufferData(b, format_, buf->pAudioData, (ALsizei)buf->AudioBytes, rate_);
        alSourceQueueBuffers(source_, 1, &b);
        ((VoiceCtx *)ctxQueue_)->contexts.push_back(buf->pContext);
        // Keep playing if the source drained while we were between buffers.
        ALint state = 0; alGetSourcei(source_, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING) alSourcePlay(source_);
    } else {
        // Unsupported (e.g. ADPCM) — report completion immediately so streaming progresses.
        if (cb_) cb_->OnBufferEnd(buf->pContext);
    }
    if (buf->Flags & XAUDIO2_END_OF_STREAM) { endOfStream_ = true; if (cb_) cb_->OnStreamEnd(); }
    return S_OK;
}

HRESULT WINAPI ALSourceVoice::SetFrequencyRatio(float ratio, UINT32) {
    if (source_ && ratio > 0) alSourcef(source_, AL_PITCH, ratio);
    return S_OK;
}

// ---- Bus / submix / mastering voices ---------------------------------------
static void FillDetails(XAUDIO2_VOICE_DETAILS *d, UINT32 ch, UINT32 rate) {
    if (!d) return; d->CreationFlags = 0; d->ActiveFlags = 0; d->InputChannels = ch; d->InputSampleRate = rate;
}
void WINAPI ALBusVoice::GetVoiceDetails(XAUDIO2_VOICE_DETAILS *d)        { FillDetails(d, channels_, rate_); }
void WINAPI ALBusVoice::GetState(XAUDIO2_VOICE_STATE *s)                 { if (s) { s->pCurrentBufferContext = nullptr; s->BuffersQueued = 0; s->SamplesPlayed = 0; } }
void WINAPI ALSubmixVoice::GetVoiceDetails(XAUDIO2_VOICE_DETAILS *d)     { FillDetails(d, channels_, rate_); }
void WINAPI ALSubmixVoice::GetState(XAUDIO2_VOICE_STATE *s)             { if (s) { s->pCurrentBufferContext = nullptr; s->BuffersQueued = 0; s->SamplesPlayed = 0; } }
void WINAPI ALMasteringVoice::GetVoiceDetails(XAUDIO2_VOICE_DETAILS *d)  { FillDetails(d, channels_, rate_); }
void WINAPI ALMasteringVoice::GetState(XAUDIO2_VOICE_STATE *s)          { if (s) { s->pCurrentBufferContext = nullptr; s->BuffersQueued = 0; s->SamplesPlayed = 0; } }

// ---- ALXAudio2 -------------------------------------------------------------
ALXAudio2::ALXAudio2() {
    dev_ = alcOpenDevice(nullptr);
    if (dev_) { ctx_ = alcCreateContext(dev_, nullptr); if (ctx_) alcMakeContextCurrent(ctx_); }
    if (!ctx_) fprintf(stderr, "[al] OpenAL device init failed\n");
}
ALXAudio2::~ALXAudio2() {
    if (ctx_) { alcMakeContextCurrent(nullptr); alcDestroyContext(ctx_); }
    if (dev_) alcCloseDevice(dev_);
}

HRESULT WINAPI ALXAudio2::CreateSourceVoice(IXAudio2SourceVoice **ppv, const WAVEFORMATEX *fmt, UINT32, float, IXAudio2VoiceCallback *cb, const XAUDIO2_VOICE_SENDS *, const XAUDIO2_EFFECT_CHAIN *) {
    if (!ppv) return E_POINTER;
    *ppv = new ALSourceVoice(fmt, cb);   // effect chain/sends ignored (OpenAL mixes to output)
    return S_OK;
}
HRESULT WINAPI ALXAudio2::CreateSubmixVoice(IXAudio2SubmixVoice **ppv, UINT32 ch, UINT32 rate, UINT32, UINT32, const XAUDIO2_VOICE_SENDS *, const XAUDIO2_EFFECT_CHAIN *) {
    if (!ppv) return E_POINTER;
    *ppv = new ALSubmixVoice(ch ? ch : 2, rate ? rate : 48000);
    return S_OK;
}
HRESULT WINAPI ALXAudio2::CreateMasteringVoice(IXAudio2MasteringVoice **ppv, UINT32 ch, UINT32 rate, UINT32, UINT32, const XAUDIO2_EFFECT_CHAIN *) {
    if (!ppv) return E_POINTER;
    *ppv = new ALMasteringVoice(ch ? ch : 2, rate ? rate : 48000);
    return S_OK;
}
HRESULT WINAPI ALXAudio2::GetDeviceDetails(UINT32 index, XAUDIO2_DEVICE_DETAILS *d) {
    if (!d || index != 0) return E_INVALIDARG;
    const char name[] = "OpenAL Default Device";
    for (int i = 0; name[i]; ++i) { d->DeviceID[i] = name[i]; d->DisplayName[i] = name[i]; d->DeviceID[i + 1] = 0; d->DisplayName[i + 1] = 0; }
    d->Role = GlobalDefaultDevice;
    WAVEFORMATEX &f = d->OutputFormat.Format;
    f.wFormatTag = WAVE_FORMAT_PCM; f.nChannels = 2; f.nSamplesPerSec = 48000;
    f.wBitsPerSample = 16; f.nBlockAlign = 4; f.nAvgBytesPerSec = 48000 * 4; f.cbSize = 0;
    return S_OK;
}

} // namespace kisak_al

// ---- Entry point (matches the XAudio2Create declaration in XAudio2.h) -------
extern "C" HRESULT WINAPI XAudio2Create(IXAudio2 **ppXAudio2, unsigned int, XAUDIO2_WINDOWS_PROCESSOR_SPECIFIER) {
    if (!ppXAudio2) return E_POINTER;
    kisak_al::ALXAudio2 *o = new kisak_al::ALXAudio2();
    if (!o->Ok()) { o->Release(); *ppXAudio2 = nullptr; return E_FAIL; }
    *ppXAudio2 = o;
    return S_OK;
}
