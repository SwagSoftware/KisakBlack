#include "snd_dsp.h"
#include <universal/com_math.h>
#include "snd.h"
#include "snd_occlusion.h"

void __cdecl SND_DspMul(unsigned int count, const float *a, const float *b, float *c)
{
    iassert((count & 3) == 0);
    iassert(((uintptr_t)a & 15) == 0);
    iassert(((uintptr_t)b & 15) == 0);
    iassert(((uintptr_t)c & 15) == 0);

    for (unsigned int i = 0; i < count; i += 4)
    {
        c[i + 0] = a[i + 0] * b[i + 0];
        c[i + 1] = a[i + 1] * b[i + 1];
        c[i + 2] = a[i + 2] * b[i + 2];
        c[i + 3] = a[i + 3] * b[i + 3];
    }
}

void __cdecl SND_DspScale(unsigned int count, float a, float *c)
{
    float *v3; // eax
    unsigned int v4; // ecx

    iassert((count & 3) == 0);
    iassert((((int)c) & 15) == 0);

    if ( count )
    {
        v3 = c + 2;
        v4 = ((count - 1) >> 2) + 1;
        do
        {
            *(v3 - 2) = *(v3 - 2) * a;
            *(v3 - 1) = *(v3 - 1) * a;
            *v3 = a * *v3;
            v3[1] = v3[1] * a;
            v3 += 4;
            --v4;
        }
        while ( v4 );
    }
}

void __cdecl SND_DspScaleCache(unsigned int count, float a, float *c)
{
    float *v3; // ebx
    unsigned int v4; // esi
    int v5; // eax

    v3 = c;

    iassert((count & 31) == 0);
    iassert((((int)c) & 31) == 0);

    if ( count >> 5 )
    {
        v4 = count >> 5;
        do
        {
            I_dcbt(128, (const char *)v3);
            v5 = 8;
            do
            {
                *v3 = *v3 * a;
                v3[1] = a * v3[1];
                v3[2] = a * v3[2];
                v3[3] = a * v3[3];
                v3 += 4;
                --v5;
            }
            while ( v5 );
            --v4;
        }
        while ( v4 );
    }
}

void __cdecl I_dcbt(int offset, const char *base)
{
    _mm_prefetch(&base[offset], 1);
}

void __cdecl SND_DspScale(unsigned int count, float a, const float *b, float *c)
{
    float *v4; // ebx
    const float *v5; // eax
    const float *v6; // ecx

    v4 = c;

    iassert((count & 3) == 0);
    iassert((((int)c) & 15) == 0);
    v5 = b;
    v6 = &b[count];
    if ( b < v6 )
    {
        do
        {
            *v4 = *v5 * a;
            v4[1] = v5[1] * a;
            v4[2] = v5[2] * a;
            v4[3] = v5[3] * a;
            v5 += 4;
            v4 += 4;
        }
        while ( v5 < v6 );
    }
}

void __cdecl SND_DspSum(unsigned int count, const float *a, float *c)
{
    iassert((count & 3) == 0);
    iassert(((uintptr_t)a & 15) == 0);
    iassert(((uintptr_t)c & 15) == 0);
    iassert(a != c);

    for (unsigned int i = 0; i < count; i += 4)
    {
        c[i + 0] += a[i + 0];
        c[i + 1] += a[i + 1];
        c[i + 2] += a[i + 2];
        c[i + 3] += a[i + 3];
    }
}

void __cdecl SND_DspOnePoleFilterMono(unsigned int count, float *frames, float b0, float a1, float *y1)
{
    float *v5; // ecx
    float *v6; // edx

    v5 = frames + 1;
    v6 = &frames[count];
    *frames = (float)(*frames * b0) - (float)(*y1 * a1);
    if ( frames + 1 != v6 )
    {
        do
        {
            *v5 = (float)(*v5 * b0) - (float)(*(v5 - 1) * a1);
            ++v5;
        }
        while ( v5 != v6 );
    }
    *y1 = *(v6 - 1);
}

void __cdecl SND_OcclusionLpfCoef(float occlusionLevel, float occlusionRatio, float sampleRate, float *b0, float *a1)
{
    double v5; // st7
    double v6; // st7
    float *v7; // eax
    float *v8; // ebx
    double v9; // st7
    float max; // [esp+8h] [ebp-1Ch]
    float maxa; // [esp+8h] [ebp-1Ch]
    float maxb; // [esp+8h] [ebp-1Ch]
    float k; // [esp+18h] [ebp-Ch]
    float y; // [esp+1Ch] [ebp-8h]
    float ya; // [esp+1Ch] [ebp-8h]
    float x; // [esp+20h] [ebp-4h]
    float sampleRatea; // [esp+34h] [ebp+10h]

    iassert(sampleRate > 100.0f);
    iassert(sampleRate < 100000.0f);

    x = I_fclamp(occlusionLevel, -80.0, 0.0);
    if ( x >= -0.000001 )
    {
        v7 = b0;
        v8 = a1;
        *b0 = 1.0f;
        *a1 = 0.0;
    }
    else
    {
        y = I_fclamp(occlusionRatio, 0.0, 0.99000001);
        max = y * x * 0.05f;
        k = powf(10.0, max);
        maxa = (1.0 - y) * x * 0.1;
        ya = powf(10.0, maxa);
        v5 = cosf(31415.928 / sampleRate);
        sampleRatea = 1.0 - v5 * ya;
        maxb = (1.0 - v5) * ya + (1.0 - v5) * ya - (1.0 - v5 * v5) * ya * ya;
        v6 = sqrtf(maxb);
        v7 = b0;
        v8 = a1;
        v9 = (sampleRatea - v6) / (1.0 - ya);
        *b0 = (1.0 - v9) * k;
        *a1 = -v9;
    }
    iassert(!IS_NAN(*b0));
    iassert(!IS_NAN(*a1));
}

double __cdecl I_fclamp(float val, float min, float max)
{
    float v5; // [esp+4h] [ebp-4h]

    if ( (float)(val - max) < 0.0 )
        v5 = val;
    else
        v5 = max;
    if ( (float)(min - val) < 0.0 )
        return v5;
    else
        return min;
}

void __cdecl SND_DspBiquadInPlace(
                const snd_dsp_biquad_coef *coef,
                snd_dsp_biquad_state *state,
                unsigned int count,
                float *input)
{
    unsigned int v5; // edx
    float v7; // xmm3_4
    float v8; // xmm1_4
    float v9; // xmm2_4
    float v10; // xmm0_4
    float *v11; // ecx
    float v12; // xmm4_4
    float v13; // xmm6_4
    float v14; // [esp+0h] [ebp-10h]
    float v15; // [esp+4h] [ebp-Ch]
    float v16; // [esp+8h] [ebp-8h]
    float v17; // [esp+Ch] [ebp-4h]
    float coefa; // [esp+18h] [ebp+8h]
    float statea; // [esp+1Ch] [ebp+Ch]

    v5 = 0;
    if ( count )
    {
        v7 = state->x[1];
        v8 = state->x[0];
        v9 = state->y[1];
        v17 = coef->s[2];
        statea = coef->s[1];
        coefa = coef->s[0];
        v16 = coef->s[3];
        v15 = coef->s[4];
        v10 = state->y[0];
        do
        {
            v11 = &input[v5];
            v12 = v7;
            v7 = v8;
            v8 = *v11;
            v14 = v9;
            v9 = v10;
            ++v5;
            v13 = (float)((float)((float)((float)(statea * v7) + (float)(coefa * *v11)) + (float)(v17 * v12))
                                    - (float)(v16 * v10))
                    - (float)(v15 * v14);
            v10 = v13;
            *v11 = v13;
        }
        while ( v5 < count );
        state->x[1] = v7;
        state->x[2] = v12;
        state->x[0] = v8;
        state->y[1] = v9;
        state->y[2] = v14;
        state->y[0] = v13;
    }
}

void __cdecl SND_DspBiquadNormalize(float *a, float *b, snd_dsp_biquad_coef *coef)
{
    iassert(!IS_NAN(b[0]));
    iassert(!IS_NAN(b[1]));
    iassert(!IS_NAN(b[2]));
    iassert(!IS_NAN(a[0]));
    iassert(!IS_NAN(a[1]));
    iassert(!IS_NAN(a[2]));

    coef->s[0] = b[0] / a[0];
    coef->s[1] = b[1] / a[0];
    coef->s[2] = b[2] / a[0];
    coef->s[3] = a[1] / a[0];
    coef->s[4] = a[2] / a[0];

    iassert(!IS_NAN(coef->s[0]));
    iassert(!IS_NAN(coef->s[1]));
    iassert(!IS_NAN(coef->s[2]));
    iassert(!IS_NAN(coef->s[3]));
    iassert(!IS_NAN(coef->s[4]));
}

void __cdecl SND_DspBiquadHShelve(float Fs, float db, float Fhz, float q, snd_dsp_biquad_coef *coef)
{
    float a[3];
    float b[3];
    float f0;
    float A;
    float w0;
    float sinW0;
    float cosW0;
    float alpha;
    float sqrtA;
    float twoSqrtAAlpha;
    float Ap1;
    float Am1;

    iassert(Fs > 1000.0f);
    iassert(Fs < 100000.0f);

    f0 = (Fhz / Fs) * 2.0f;

    iassert(f0 >= 0.0f);
    iassert(!IS_NAN(f0));
    iassert(q > 0.0f);
    iassert(q < 32.0f);
    iassert(!IS_NAN(q));
    iassert(db < 50.0f);
    iassert(db > -1000.0f);
    iassert(!IS_NAN(db));

    f0 = I_fmax(f0, 0.000001f);

    A = powf(10.0f, db * 0.025f);
    w0 = f0 * 3.1415927f;
    sinW0 = sinf(w0);
    cosW0 = cosf(w0);
    alpha = sinW0 / (q + q);
    sqrtA = sqrtf(A);

    twoSqrtAAlpha = 2.0f * sqrtA * alpha;
    Ap1 = A + 1.0f;
    Am1 = A - 1.0f;

    b[0] = A * (Ap1 + Am1 * cosW0 + twoSqrtAAlpha);
    b[1] = -2.0f * A * (Am1 + Ap1 * cosW0);
    b[2] = A * (Ap1 + Am1 * cosW0 - twoSqrtAAlpha);

    a[0] = Ap1 - Am1 * cosW0 + twoSqrtAAlpha;
    a[1] = 2.0f * (Am1 - Ap1 * cosW0);
    a[2] = Ap1 - Am1 * cosW0 - twoSqrtAAlpha;

    SND_DspBiquadNormalize(a, b, coef);
}

void __cdecl SND_DspBiquadLShelve(
    float Fs,
    float db,
    float Fhz,
    float q,
    snd_dsp_biquad_coef *coef)
{
    float a[3];
    float b[3];
    float f0;
    float A;
    float w0;
    float alpha;
    float cosW0;
    float sqrtA;
    float twoSqrtAAlpha;
    float Ap1;
    float Am1;

    iassert(Fs > 1000.0f);
    iassert(Fs < 100000.0f);

    f0 = (Fhz / Fs) * 2.0f;

    iassert(f0 >= 0.0f);
    iassert(!IS_NAN(f0));
    iassert(q > 0.0f);
    iassert(q < 32.0f);
    iassert(!IS_NAN(q));
    iassert(db < 50.0f);
    iassert(db > -1000.0f);
    iassert(!IS_NAN(db));

    f0 = I_fmax(f0, 0.000001f);

    A = powf(10.0f, db * 0.025f);
    w0 = f0 * 3.1415927f;
    alpha = sinf(w0) / (q + q);
    cosW0 = cosf(w0);
    sqrtA = sqrtf(A);

    twoSqrtAAlpha = 2.0f * sqrtA * alpha;
    Ap1 = A + 1.0f;
    Am1 = A - 1.0f;

    b[0] = A * (Ap1 - Am1 * cosW0 + twoSqrtAAlpha);
    b[1] = 2.0f * A * (Am1 - Ap1 * cosW0);
    b[2] = A * (Ap1 - Am1 * cosW0 - twoSqrtAAlpha);

    a[0] = Ap1 + Am1 * cosW0 + twoSqrtAAlpha;
    a[1] = -2.0f * (Am1 + Ap1 * cosW0);
    a[2] = Ap1 + Am1 * cosW0 - twoSqrtAAlpha;

    SND_DspBiquadNormalize(a, b, coef);
}

void __cdecl SND_DspBiquadPeak(
    float Fs,
    float db,
    float Fhz,
    float q,
    snd_dsp_biquad_coef *coef)
{
    float a[3];
    float b[3];
    float f0;
    float A;
    float w0;
    float alpha;
    float cosW0;
    float alphaA;
    float alphaOverA;

    iassert(Fs > 1000.0f);
    iassert(Fs < 100000.0f);

    f0 = (Fhz / Fs) * 2.0f;

    iassert(f0 >= 0.0f);
    iassert(!IS_NAN(f0));
    iassert(q > 0.0f);
    iassert(q < 32.0f);
    iassert(!IS_NAN(q));
    iassert(db < 50.0f);
    iassert(db > -1000.0f);
    iassert(!IS_NAN(db));

    f0 = I_fmax(f0, 0.000001f);

    A = powf(10.0f, db * 0.025f);
    w0 = f0 * 3.1415927f;
    alpha = sinf(w0) / (q + q);
    cosW0 = cosf(w0);

    alphaA = alpha * A;
    alphaOverA = alpha / A;

    b[0] = 1.0f + alphaA;
    b[1] = -2.0f * cosW0;
    b[2] = 1.0f - alphaA;

    a[0] = 1.0f + alphaOverA;
    a[1] = -2.0f * cosW0;
    a[2] = 1.0f - alphaOverA;

    SND_DspBiquadNormalize(a, b, coef);
}

void __cdecl SND_DspBiquadBpf(
    float Fs,
    float Fhz,
    float q,
    snd_dsp_biquad_coef *coef)
{
    float a[3];
    float b[3];
    float f0;
    float w0;
    float alpha;
    float cosW0;

    iassert(Fs > 1000.0f);
    iassert(Fs < 100000.0f);

    f0 = (Fhz / Fs) * 2.0f;

    iassert(f0 >= 0.0f);
    iassert(!IS_NAN(f0));
    iassert(q > 0.0f);
    iassert(q <= 32.0f);
    iassert(!IS_NAN(q));

    f0 = I_fmax(f0, 0.000001f);

    w0 = f0 * 3.1415927f;
    alpha = sinf(w0) / (q + q);
    cosW0 = cosf(w0);

    b[0] = alpha;
    b[1] = 0.0f;
    b[2] = -alpha;

    a[0] = 1.0f + alpha;
    a[1] = -2.0f * cosW0;
    a[2] = 1.0f - alpha;

    SND_DspBiquadNormalize(a, b, coef);
}

void __cdecl SND_DspPolyDistortion(unsigned int count, float amount, float *frame, float *temp)
{
    float min; // [esp+4h] [ebp-10h]
    float mina; // [esp+4h] [ebp-10h]
    float amounta; // [esp+20h] [ebp+Ch]
    float amountb; // [esp+20h] [ebp+Ch]

    amounta = I_fclamp(amount, 0.0, 1.0);
    SND_DspMul(count, frame, frame, temp);
    amountb = amounta * 1.5707964;
    min = cosf(amountb);
    SND_DspScale(count, min, frame);
    mina = sinf(amountb);
    SND_DspScale(count, mina, temp);
    SND_DspSum(count, temp, frame);
}

void __cdecl SND_DspClip(unsigned int count, float *c, float minimum, float maximum)
{
    float *v4; // esi
    unsigned int v5; // edi
    float *v6; // esi
    unsigned int v7; // edi

    if ( count )
    {
        v4 = c + 2;
        v5 = ((count - 1) >> 2) + 1;
        do
        {
            *(v4 - 2) = I_fmin(maximum, *(v4 - 2));
            *(v4 - 1) = I_fmin(maximum, *(v4 - 1));
            *v4 = I_fmin(maximum, *v4);
            v4[1] = I_fmin(maximum, v4[1]);
            v4 += 4;
            --v5;
        }
        while ( v5 );
        v6 = c + 2;
        v7 = ((count - 1) >> 2) + 1;
        do
        {
            *(v6 - 2) = I_fmax(minimum, *(v6 - 2));
            *(v6 - 1) = I_fmax(minimum, *(v6 - 1));
            *v6 = I_fmax(minimum, *v6);
            v6[1] = I_fmax(minimum, v6[1]);
            v6 += 4;
            --v7;
        }
        while ( v7 );
    }
}

void __cdecl SND_DspSquelch(
                const snd_dsp_squelch_param *param,
                snd_dsp_squelch_state *state,
                float frameRate,
                unsigned int frameCount,
                float *input)
{
    unsigned int v6; // ebx
    float *v7; // esi
    float v8; // xmm0_4
    float v9; // xmm1_4
    float dt; // [esp+4h] [ebp-1Ch]
    float ITG; // [esp+18h] [ebp-8h]
    float TH; // [esp+28h] [ebp+8h]
    float statea; // [esp+2Ch] [ebp+Ch]
    float TG; // [esp+30h] [ebp+10h]

    TG = SND_DspDecayConstant(param->tg, 1.0 / frameRate, 0.0056234132);
    v6 = 0;
    for ( TH = param->th; v6 < frameCount; *v7 = v9 )
    {
        v7 = &input[v6];
        dt = fabs(*v7);
        ++v6;
        statea = 1.0 - I_fmax(dt, TH);
        ITG = 1.0 - TG;
        v8 = (float)((float)((float)((float)((float)((float)((float)((float)(statea * statea) * statea) * statea) * statea)
                                                                             * statea)
                                                             * (float)(1.0 / TH))
                                             + 1.0)
                             * TG)
             + (float)(state->g * ITG);
        v9 = *v7 * v8;
        state->g = v8;
    }

    iassert(!IS_NAN(state->g));
}

double __cdecl SND_DspDecayConstant(float time, float dt, float targetAmplitude)
{
    iassert(time >= 0);
    iassert(!IS_NAN(time));
    iassert(dt >= 0);
    iassert(!IS_NAN(dt));

    if ( dt < time )
        return -(logf(targetAmplitude) * (dt / time));
    else
        return 0.0;
}

void __cdecl SND_DspFutzMono(
                const snd_dsp_futz_param *param,
                snd_dsp_futz_state *state,
                float rate,
                unsigned int count,
                float *input,
                float *tempa,
                float *tempb)
{
    snd_dsp_biquad_coef ls; // [esp+20h] [ebp-28h] BYREF
    snd_dsp_biquad_coef bpf; // [esp+34h] [ebp-14h] BYREF

    if ( param->blend >= SND_EPSILON )
    {
        iassert(param->blend >= 0.0f);
        iassert(param->blend <= 1.0f);

        SND_DspScale(count, param->pregain, input, tempa);
        SND_DspPolyDistortion(count, param->distortion, tempa, tempb);
        //SND_DspClip(count, tempa, COERCE_FLOAT(LODWORD(param->preclip) ^ _mask__NegFloat_), param->preclip);
        SND_DspClip(count, tempa, -param->preclip, param->preclip);
        SND_DspBiquadBpf(rate, param->bpfF, param->bpfQ, &bpf);
        SND_DspBiquadInPlace(&bpf, &state->bpf, count, tempa);
        SND_DspBiquadLShelve(rate, param->lsG, param->lsF, param->lsQ, &ls);
        SND_DspBiquadInPlace(&ls, &state->ls, count, tempa);
        SND_DspSquelch(&param->squelch, &state->squelch, rate, count, tempa);
        SND_DspScale(count, param->postgain * param->blend, tempa);
        SND_DspScale(count, 1.0 - param->blend, input);
        SND_DspSum(count, tempa, input);
        //SND_DspClip(count, input, COERCE_FLOAT(LODWORD(param->postclip) ^ _mask__NegFloat_), param->postclip);
        SND_DspClip(count, input, -param->postclip, param->postclip);
    }
}

void __cdecl SND_DspInterleave(unsigned int channel_count, unsigned int frame_count, float *in, float *out)
{
    // aislopped for correctness
    for (unsigned int ch = 0; ch < channel_count; ch++)
    {
        float *src = in + ch * frame_count;
        float *dst = out + ch;

        for (unsigned int i = 0; i < frame_count; i++)
        {
            dst[i * channel_count] = src[i];
        }
    }
}

void __cdecl SND_DspUninterleave(unsigned int channel_count, unsigned int frame_count, float *in, float *out)
{
    // aislopped for correctness
    for (unsigned int ch = 0; ch < channel_count; ch++)
    {
        float *dst = out + ch * frame_count;
        float *src = in + ch;

        for (unsigned int i = 0; i < frame_count; i++)
        {
            dst[i] = src[i * channel_count];
        }
    }
}

float SND_DspLog(float m)
{
    float x = (m - 1.0f) / (m + 1.0f);
    float x3 = x * x * x;

    return x + x3 * (1.0f / 3.0f);
}

float SND_DspExp(float x)
{
    float x2 = x * x;
    float x3 = x2 * x;
    float x4 = x3 * x;
    float x5 = x4 * x;

    return 1.0f
        + x
        + x2 / 2.0f
        + x3 / 6.0f
        + x4 / 24.0f
        + x5 / 120.0f;
}

void __cdecl SND_DspDynamoDenormal(snd_dsp_dynamo_state *state)
{
    if ( state->yfilter < SND_EPSILON )
        state->yfilter = 0.0;
    if ( state->gfilter < SND_EPSILON )
        state->gfilter = 0.0;
    if ( state->g < SND_EPSILON )
        state->g = 0.0;
}

void __cdecl SND_DspDynamo(
                unsigned int frameCount,
                float sampleRate,
                float TH,
                float R,
                float TA,
                float TR,
                snd_dsp_dynamo_state *state,
                float *input,
                float *gain)
{
    unsigned int *p_offset; // ebx
    unsigned int v11; // edx
    float *v12; // ebx
    float g; // xmm0_4
    float gfilter; // xmm3_4
    float v15; // xmm2_4
    float *v16; // ebx
    double v17; // st7
    float v18; // xmm0_4
    float dt_4; // [esp+14h] [ebp-2Ch]
    float dt_4a; // [esp+14h] [ebp-2Ch]
    float dt_4b; // [esp+14h] [ebp-2Ch]
    float *end; // [esp+24h] [ebp-1Ch]
    float v23; // [esp+30h] [ebp-10h]
    float y; // [esp+34h] [ebp-Ch]
    float *output; // [esp+38h] [ebp-8h]
    unsigned int T; // [esp+3Ch] [ebp-4h]
    float iR; // [esp+48h] [ebp+8h]
    float SA; // [esp+4Ch] [ebp+Ch]
    float SR; // [esp+54h] [ebp+14h]
    float *lookahead; // [esp+60h] [ebp+20h]
    float *lookaheada; // [esp+60h] [ebp+20h]
    float Ma; // [esp+64h] [ebp+24h]
    float M; // [esp+64h] [ebp+24h]

    iassert(SND_DSP_DYNAMO_LOOKAHEAD_FRAMES % frameCount == 0);

    T = frameCount;
    
    iassert((frameCount * sizeof(float)) % 128 == 0);

    p_offset = &state->offset;
    output = input;
    lookahead = &state->lookahead[state->offset];

    iassert(lookahead < state->lookahead + SND_DSP_DYNAMO_LOOKAHEAD_FRAMES);
    iassert(lookahead >= state->lookahead);

    memcpy((unsigned __int8 *)lookahead, (unsigned __int8 *)input, T * 4);
    v11 = (frameCount + *p_offset) % 0x3C0;
    *p_offset = v11;
    v12 = &state->lookahead[v11];
    iassert(TH != 0);
    Ma = 1.0 / sampleRate;
    iR = 1.0 / R;
    SR = SND_DspDecayConstant(TR, Ma, 0.0056234132);
    SA = SND_DspDecayConstant(TA, Ma, 0.0056234132);
    dt_4 = SND_DspLog(TH) * (iR - 1.0);
    M = SND_DspExp(dt_4);

    iassert(!IS_NAN(iR));
    iassert(!IS_NAN(M));
    rangeassert(SA, 0.0f, 1.0f);
    rangeassert(SR, 0.0f, 1.0f);

    *gain = M;
    for ( end = &v12[T]; v12 != end; output += 2 )
    {
        y = fabs(*lookahead);
        lookaheada = lookahead + 1;
        v23 = (float)(state->yfilter * 0.99000001) + (float)(y * 0.0099999905);
        state->yfilter = v23;
        if ( v23 <= TH )
        {
            state->g = M;
        }
        else
        {
            dt_4a = v23 * (float)(1.0 / TH);
            dt_4b = SND_DspLog(dt_4a) * iR;
            state->g = SND_DspExp(dt_4b) / state->yfilter * M * TH;
        }
        *output = state->gfilter * *v12;
        *gain = I_fmin(*gain, state->gfilter);
        g = state->g;
        gfilter = state->gfilter;
        if ( gfilter <= g )
            v15 = SR;
        else
            v15 = SA;
        state->gfilter = (float)(g * v15) + (float)((float)(1.0 - v15) * gfilter);
        v16 = v12 + 1;
        v17 = fabs(*lookaheada);
        v18 = state->gfilter;
        lookahead = lookaheada + 1;
        state->yfilter = state->yfilter * 0.99000001 + v17 * 0.0099999905;
        output[1] = v18 * *v16;
        v12 = v16 + 1;
        state->gfilter = (float)((float)(1.0 - v15) * state->gfilter) + (float)(state->g * v15);
    }
    SND_DspDynamoDenormal(state);
}

void __cdecl SND_DspFxSourceMono(
                const snd_dsp_futz_param *params,
                snd_dsp_source_state *state,
                unsigned int frameCount,
                float *frames,
                float *tempa,
                float *tempb)
{
    snd_dsp_source_state *v6; // ebx
    const snd_dsp_source_params *v7; // esi
    float blend; // xmm0_4
    float b0; // [esp+20h] [ebp-4h] BYREF

    v6 = state;
    v7 = (const snd_dsp_source_params *)params;
    if ( params->bpfF < -SND_EPSILON )
    {
        PROF_SCOPED("lpf");
        SND_OcclusionLpfCoef(v7->lpfAttenuation, v7->lpfRatio, v7->frameRate, &b0, (float *)&params);
        SND_DspOnePoleFilterMono(frameCount, frames, b0, *(float *)&params, &v6->lpfy);
    }
    blend = v7->futz.blend;
    params = &v7->futz;
    if ( blend > SND_EPSILON )
    {
        PROF_SCOPED("futz");
        SND_DspFutzMono(params, &v6->futz, v7->frameRate, frameCount, frames, tempa, tempb);
    }
}

void __cdecl SND_DspBiquadDenormal(snd_dsp_biquad_state *state)
{
    int i; // ebx
    float *v2; // esi
    float f; // [esp+0h] [ebp-18h]
    float fa; // [esp+0h] [ebp-18h]
    float valGE; // [esp+4h] [ebp-14h]
    float valGEa; // [esp+4h] [ebp-14h]

    for ( i = 0; i < 3; ++i )
    {
        v2 = &state->y[i];
        valGE = *v2;
        f = fabs(valGE) - 5.9604643e-12;
        *v2 = I_fsel(f, valGE, 0.0);
        v2 += 4;
        valGEa = *v2;
        fa = fabs(valGEa) - 5.9604643e-12;
        *v2 = I_fsel(fa, valGEa, 0.0);
    }
}

double __cdecl I_fsel(float comparand, float valGE, float valLT)
{
    if ( comparand < 0.0 )
        return valLT;
    return valGE;
}

void __cdecl SND_DspBiquadNanCheck(snd_dsp_biquad_state *state)
{
    for ( int i = 0; i < 3; ++i )
    {
        iassert(!IS_NAN(state->y[i]));
        iassert(!IS_NAN(state->x[i]));
    }
}

void __cdecl SND_DspFxMasterNoVoiceSingleChannel(
                unsigned int frameCount,
                float frameRate,
                float *frames,
                snd_dsp_master_no_voice_params *params,
                snd_dsp_master_no_voice_state *state,
                snd_dsp_meters *meters)
{
    PROF_SCOPED("comp");

    if ( params->compE > SND_EPSILON )
    {
        if ( fabs(params->compPG - 1.0) > SND_EPSILON )
            SND_DspScale(frameCount, params->compPG, frames);
        SND_DspDynamo(
            frameCount,
            frameRate,
            params->compT,
            params->compR,
            params->compTA,
            params->compTR,
            &state->comp,
            frames,
            meters != 0 ? &meters->dyn1Gain : 0);
        if ( fabs(params->compMG - 1.0) > SND_EPSILON )
            SND_DspScale(frameCount, params->compMG, frames);
    }
}

void __cdecl SND_DspFxMasterSingleChannel(
                unsigned int frameCount,
                float frameRate,
                float *frames,
                snd_dsp_master_params *params,
                snd_dsp_master_state *state,
                snd_dsp_meters *meters)
{
    snd_dsp_biquad_coef hi; // [esp+2Ch] [ebp-14h] BYREF

    iassert(params);
    iassert(state);
    iassert(meters);

    SND_DspScaleCache(frameCount, params->eqG, frames);
    
    {
        PROF_SCOPED("eq");

        if (params->lowE <= SND_EPSILON)
        {
            memset(state, 0, 0x20u);
        }
        else
        {
            SND_DspBiquadLShelve(frameRate, params->lowG, params->lowF, params->lowQ, &hi);
            SND_DspBiquadInPlace(&hi, &state->low, frameCount, frames);
            SND_DspBiquadDenormal(&state->low);
            SND_DspBiquadNanCheck(&state->low);
        }

        if (params->peak1E <= SND_EPSILON)
        {
            memset(&state->peak1, 0, sizeof(state->peak1));
        }
        else
        {
            SND_DspBiquadPeak(frameRate, params->peak1G, params->peak1F, params->peak1Q, &hi);
            SND_DspBiquadInPlace(&hi, &state->peak1, frameCount, frames);
            SND_DspBiquadDenormal(&state->peak1);
            SND_DspBiquadNanCheck(&state->peak1);
        }
        if (params->peak2E <= SND_EPSILON)
        {
            memset(&state->peak2, 0, sizeof(state->peak2));
        }
        else
        {
            SND_DspBiquadPeak(frameRate, params->peak2G, params->peak2F, params->peak2Q, &hi);
            SND_DspBiquadInPlace(&hi, &state->peak2, frameCount, frames);
            SND_DspBiquadDenormal(&state->peak2);
            SND_DspBiquadNanCheck(&state->peak2);
        }
        if (params->hiE <= SND_EPSILON)
        {
            memset(&state->hi, 0, sizeof(state->hi));
        }
        else
        {
            SND_DspBiquadHShelve(frameRate, params->hiG, params->hiF, params->hiQ, &hi);
            SND_DspBiquadInPlace(&hi, &state->hi, frameCount, frames);
            SND_DspBiquadDenormal(&state->hi);
            SND_DspBiquadNanCheck(&state->hi);
        }
    }
    
    {
        PROF_SCOPED("limit");
        if (params->limitE > SND_EPSILON)
        {
            if (fabs(params->limitPG - 1.0) > SND_EPSILON)
                SND_DspScale(frameCount, params->limitPG, frames);
            SND_DspDynamo(
                frameCount,
                frameRate,
                params->limitT,
                params->limitR,
                params->limitTA,
                params->limitTR,
                &state->limit,
                frames,
                meters != 0 ? &meters->dyn2Gain : 0);
            if (fabs(params->limitMG - 1.0) > SND_EPSILON)
                SND_DspScale(frameCount, params->limitMG, frames);
        }
    }
}

