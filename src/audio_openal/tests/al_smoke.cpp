// al_smoke.cpp — exercises the XAudio2→OpenAL backend the way the engine's SD_Init
// path does: create the engine, enumerate the device, create a mastering voice and a
// source voice, submit a generated sine-wave buffer, play it, and confirm the buffer
// is consumed (OnBufferEnd fires). Prints SMOKE: PASS/FAIL. Runs headless (OpenAL
// falls back to a null device if no audio hardware is present).
#include <XAudio2.h>
#include <cstdio>
#include <cmath>
#include <vector>
#include <unistd.h>

struct Callback : IXAudio2VoiceCallback {
    int ended = 0;
    void WINAPI OnVoiceProcessingPassStart(UINT32) override {}
    void WINAPI OnVoiceProcessingPassEnd() override {}
    void WINAPI OnStreamEnd() override {}
    void WINAPI OnBufferStart(void *) override {}
    void WINAPI OnBufferEnd(void *) override { ++ended; }
    void WINAPI OnLoopEnd(void *) override {}
    void WINAPI OnVoiceError(void *, HRESULT) override {}
};

int main() {
    IXAudio2 *xa2 = nullptr;
    if (XAudio2Create(&xa2, 0, XAUDIO2_ANY_PROCESSOR) < 0 || !xa2) { printf("FAIL: XAudio2Create\n"); return 1; }

    UINT32 count = 0; xa2->GetDeviceCount(&count);
    XAUDIO2_DEVICE_DETAILS dd; xa2->GetDeviceDetails(0, &dd);

    IXAudio2MasteringVoice *master = nullptr;
    if (xa2->CreateMasteringVoice(&master, 2, 48000, 0, 0, nullptr) < 0 || !master) { printf("FAIL: master\n"); return 1; }

    // 0.1s of 440 Hz, 16-bit mono @ 48 kHz.
    const int rate = 48000, samples = rate / 10;
    std::vector<short> pcm(samples);
    for (int i = 0; i < samples; ++i) pcm[i] = (short)(8000.0 * sin(2.0 * 3.14159265 * 440.0 * i / rate));

    WAVEFORMATEX fmt{}; fmt.wFormatTag = WAVE_FORMAT_PCM; fmt.nChannels = 1;
    fmt.nSamplesPerSec = rate; fmt.wBitsPerSample = 16; fmt.nBlockAlign = 2; fmt.nAvgBytesPerSec = rate * 2;

    Callback cb;
    IXAudio2SourceVoice *voice = nullptr;
    if (xa2->CreateSourceVoice(&voice, &fmt, 0, 2.0f, &cb, nullptr, nullptr) < 0 || !voice) { printf("FAIL: source\n"); return 1; }

    XAUDIO2_BUFFER buf{}; buf.AudioBytes = (UINT32)(pcm.size() * sizeof(short));
    buf.pAudioData = (const BYTE *)pcm.data(); buf.Flags = XAUDIO2_END_OF_STREAM; buf.pContext = (void *)0x1234;
    if (voice->SubmitSourceBuffer(&buf, nullptr) < 0) { printf("FAIL: submit\n"); return 1; }
    voice->Start(0, 0);

    XAUDIO2_VOICE_STATE st{}; voice->GetState(&st);
    printf("deviceCount=%u queued=%u\n", count, st.BuffersQueued);

    // Pump for up to ~0.5s waiting for the buffer to finish playing.
    for (int i = 0; i < 50 && cb.ended == 0; ++i) { usleep(20000); voice->GetState(&st); }

    bool pass = (count == 1) && (cb.ended == 1);
    printf("samplesPlayed=%llu callbacks=%d\n", (unsigned long long)st.SamplesPlayed, cb.ended);
    voice->DestroyVoice(); master->DestroyVoice(); xa2->Release();
    printf("SMOKE: %s\n", pass ? "PASS" : "FAIL");
    return pass ? 0 : 1;
}
