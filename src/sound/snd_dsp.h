#pragma once
#include "snd_driver_xaudio2_dsp.h"

#define SND_DSP_DYNAMO_LOOKAHEAD_FRAMES 960

struct snd_dsp_meters // sizeof=0x18
{                                                                             // XREF: snd_local_t/r
    float p;
    float vu;
    float dyn1Gain;
    float dyn1Level;
    float dyn2Gain;
    float dyn2Level;
};

void __cdecl SND_DspMul(unsigned int count, const float *a, const float *b, float *c);
void __cdecl SND_DspScale(unsigned int count, float a, float *c);
void __cdecl SND_DspScaleCache(unsigned int count, float a, float *c);
void __cdecl I_dcbt(int offset, const char *base);
void __cdecl SND_DspScale(unsigned int count, float a, const float *b, float *c);
void __cdecl SND_DspSum(unsigned int count, const float *a, float *c);
void __cdecl SND_DspOnePoleFilterMono(unsigned int count, float *frames, float b0, float a1, float *y1);
void __cdecl SND_OcclusionLpfCoef(float occlusionLevel, float occlusionRatio, float sampleRate, float *b0, float *a1);
double __cdecl I_fclamp(float val, float min, float max);
void __cdecl SND_DspBiquadInPlace(
                const snd_dsp_biquad_coef *coef,
                snd_dsp_biquad_state *state,
                unsigned int count,
                float *input);
void __cdecl SND_DspBiquadNormalize(float *a, float *b, snd_dsp_biquad_coef *coef);
void __cdecl SND_DspBiquadHShelve(float Fs, float db, float Fhz, float q, snd_dsp_biquad_coef *coef);
void __cdecl SND_DspBiquadLShelve(float Fs, float db, float Fhz, float q, snd_dsp_biquad_coef *coef);
void __cdecl SND_DspBiquadPeak(float Fs, float db, float Fhz, float q, snd_dsp_biquad_coef *coef);
void __cdecl SND_DspBiquadBpf(float Fs, float Fhz, float q, snd_dsp_biquad_coef *coef);
void __cdecl SND_DspPolyDistortion(unsigned int count, float amount, float *frame, float *temp);
void __cdecl SND_DspClip(unsigned int count, float *c, float minimum, float maximum);
void __cdecl SND_DspSquelch(
                const snd_dsp_squelch_param *param,
                snd_dsp_squelch_state *state,
                float frameRate,
                unsigned int frameCount,
                float *input);
double __cdecl SND_DspDecayConstant(float time, float dt, float targetAmplitude);
void __cdecl SND_DspFutzMono(
                const snd_dsp_futz_param *param,
                snd_dsp_futz_state *state,
                float rate,
                unsigned int count,
                float *input,
                float *tempa,
                float *tempb);
void __cdecl SND_DspInterleave(unsigned int channel_count, unsigned int frame_count, float *in, float *out);
void __cdecl SND_DspUninterleave(unsigned int channel_count, unsigned int frame_count, float *in, float *out);
float SND_DspLog(float m);
float SND_DspExp(float x);
void __cdecl SND_DspDynamoDenormal(snd_dsp_dynamo_state *state);
void __cdecl SND_DspDynamo(
                unsigned int frameCount,
                float sampleRate,
                float TH,
                float R,
                float TA,
                float TR,
                snd_dsp_dynamo_state *state,
                float *input,
                float *gain);
void __cdecl SND_DspFxSourceMono(
                const snd_dsp_futz_param *params,
                snd_dsp_source_state *state,
                unsigned int frameCount,
                float *frames,
                float *tempa,
                float *tempb);
void __cdecl SND_DspBiquadDenormal(snd_dsp_biquad_state *state);
double __cdecl I_fsel(float comparand, float valGE, float valLT);
void __cdecl SND_DspBiquadNanCheck(snd_dsp_biquad_state *state);
void __cdecl SND_DspFxMasterNoVoiceSingleChannel(
                unsigned int frameCount,
                float frameRate,
                float *frames,
                snd_dsp_master_no_voice_params *params,
                snd_dsp_master_no_voice_state *state,
                snd_dsp_meters *meters);
void __cdecl SND_DspFxMasterSingleChannel(
                unsigned int frameCount,
                float frameRate,
                float *frames,
                snd_dsp_master_params *params,
                snd_dsp_master_state *state,
                snd_dsp_meters *meters);
