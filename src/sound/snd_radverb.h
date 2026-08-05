#pragma once

struct snd_radverb // sizeof=0x60
{
    char name[32];
    unsigned int id;
    float smoothing;
    float earlyTime;
    float lateTime;
    float earlyGain;
    float lateGain;
    float returnGain;
    float earlyLpf;
    float lateLpf;
    float inputLpf;
    float dampLpf;
    float wallReflect;
    float dryGain;
    float earlySize;
    float lateSize;
    float diffusion;
};
static_assert(sizeof(snd_radverb) == 96);

struct snd_rv_params
{
    float frameRate;
    float smoothing;
    float earlyTime;
    float lateTime;
    float earlyGain;
    float lateGain;
    float lateGainProx[4];
    float returnGain;
    float earlyLpf;
    float lateLpf;
    float inputLpf;
    float dampLpf;
    float wallReflect[4];
    float dryGain;
    float earlySize;
    float lateSize;
    float diffusion;
    float angle;
    unsigned int delayMatrix;
};

struct __declspec(align(128)) snd_rv_state // sizeof=0x80280
{                                       // XREF: SDXA2RadverbEffect/r
    float delayLine[0x20000];            // XREF: .data:00E1F218/o
                                        // .data:00E1F42C/o ...
    float earlyReflectionCoefs[4][4];
    unsigned int earlyReflectionDelays[4][4];
    float lateReflectionCoefs[4][4];
    unsigned int lateReflectionDelays[4][4];
    float inputLpfState[4];
    float earlyLpfState[4];
    float lateLpfState[4];
    float dampLpfState[4];
    snd_rv_params params;
    unsigned int earlyReflectionDelayBase[4][4];
    unsigned int lateReflectionDelayBase[4][4];
    unsigned int delayIndex;
};


void __cdecl SND_RvParamsDefault(snd_rv_params *params);
double __cdecl SND_RvValidateRange(float value, float min, float max);
void __cdecl SND_RvParamsValidate(snd_rv_params *params);
void __cdecl SND_RvFrameParam(snd_rv_params *params, snd_rv_state *state, unsigned int frameCount);
double __cdecl I_flerp(float a, float b, float w);
void __cdecl SND_RvDelayInit(snd_rv_state *state, unsigned int values);
void __cdecl SND_RvFrame(
                snd_rv_params *params,
                snd_rv_state *state,
                const float *count,
                const float *inLF,
                const float *inRF,
                const float *inLS,
                const float *inRS,
                float *outLF,
                float *outRF,
                float *outLS,
                float *outRS);
