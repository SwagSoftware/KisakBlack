#include "snd_radverb.h"

#include <universal/assertive.h>
#include <universal/com_math.h>
#include "snd_dsp.h"

void __cdecl SND_RvParamsDefault(snd_rv_params *params)
{
    params->earlySize = 1.0f;
    params->lateSize = 1.0f;
    params->earlyGain = 1.0f;
    params->lateGain = 1.0f;
    params->smoothing = 0.5f;
    params->diffusion = 0.5f;
    params->dryGain = 0.0;
    params->earlyTime = 0.8f;
    params->lateTime = 0.8f;
    params->earlyLpf = 0.0;
    params->lateGainProx[0] = 1.0f;
    params->lateGainProx[1] = 1.0f;
    params->lateGainProx[2] = 1.0f;
    params->lateGainProx[3] = 1.0f;
    params->returnGain = 1.0f;
    params->lateLpf = 0.0;
    params->inputLpf = 0.0;
    params->dampLpf = 0.0;
    params->wallReflect[0] = 0.5f;
    params->wallReflect[1] = 0.5f;
    params->wallReflect[2] = 0.5f;
    params->wallReflect[3] = 0.5f;
    params->frameRate = 48000.0f;
    params->delayMatrix = 0;
}

double __cdecl SND_RvValidateRange(float value, float min, float max)
{
    iassert(!IS_NAN(value));
    iassert(value >= min);
    iassert(value <= max);

    return I_fclamp(value, min, max);
}

void __cdecl SND_RvParamsValidate(snd_rv_params *params)
{
    float *lateGainProx; // edi
    int v2; // ebx
    float *wallReflect; // edi
    int v4; // ebx

    params->smoothing = SND_RvValidateRange(params->smoothing, 0.0, 1.0);
    params->earlyTime = SND_RvValidateRange(params->earlyTime, 0.0, 1.0);
    params->lateTime = SND_RvValidateRange(params->lateTime, 0.0, 1.0);
    params->earlyGain = SND_RvValidateRange(params->earlyGain, 0.0, 4.0);
    params->lateGain = SND_RvValidateRange(params->lateGain, 0.0, 4.0);
    lateGainProx = params->lateGainProx;
    v2 = 4;
    do
    {
        *lateGainProx = SND_RvValidateRange(*lateGainProx, 0.0, 4.0);
        ++lateGainProx;
        --v2;
    }
    while ( v2 );
    params->returnGain = SND_RvValidateRange(params->returnGain, 0.0, 4.0);
    params->earlyLpf = SND_RvValidateRange(params->earlyLpf, 0.0, 1.0);
    params->lateLpf = SND_RvValidateRange(params->lateLpf, 0.0, 1.0);
    params->inputLpf = SND_RvValidateRange(params->inputLpf, 0.0, 1.0);
    params->dampLpf = SND_RvValidateRange(params->dampLpf, 0.0, 1.0);
    wallReflect = params->wallReflect;
    v4 = 4;
    do
    {
        *wallReflect = SND_RvValidateRange(*wallReflect, 0.0, 1.0);
        ++wallReflect;
        --v4;
    }
    while ( v4 );
    params->dryGain = SND_RvValidateRange(params->dryGain, 0.0, 1.0);
    params->earlySize = SND_RvValidateRange(params->earlySize, 0.1, 16.0);
    params->lateSize = SND_RvValidateRange(params->lateSize, 0.1, 16.0);
    params->diffusion = SND_RvValidateRange(params->diffusion, 0.0, 1.0);

    iassert(!IS_NAN(params->angle));
}

// aislop
void __cdecl SND_RvFrameParam(snd_rv_params *params, snd_rv_state *state, unsigned int frameCount)
{
    SND_RvParamsValidate(params);

    state->params.frameRate = params->frameRate;
    iassert(params->frameRate > 1000.0f);
    iassert(params->frameRate < 100000.0f);

    const float smooth = (1.0f - params->smoothing) / (params->frameRate / (float)frameCount * 0.1f);
    iassert(!IS_NAN(smooth));

    state->params.earlyTime  = I_flerp(state->params.earlyTime,  params->earlyTime,  smooth);
    state->params.lateTime   = I_flerp(state->params.lateTime,   params->lateTime,   smooth);
    state->params.earlyGain  = I_flerp(state->params.earlyGain,  params->earlyGain,  smooth);
    state->params.lateGain   = I_flerp(state->params.lateGain,   params->lateGain,   smooth);
    for (int i = 0; i < 4; ++i)
        state->params.lateGainProx[i] = params->lateGainProx[i] * state->params.lateGain;
    state->params.returnGain = I_flerp(state->params.returnGain, params->returnGain, smooth);
    state->params.earlyLpf   = I_flerp(state->params.earlyLpf,   params->earlyLpf,   smooth);
    state->params.lateLpf    = I_flerp(state->params.lateLpf,    params->lateLpf,    smooth);
    state->params.inputLpf   = I_flerp(state->params.inputLpf,   params->inputLpf,   smooth);
    state->params.dampLpf    = I_flerp(state->params.dampLpf,    params->dampLpf,    smooth);
    for (int i = 0; i < 4; ++i)
        state->params.wallReflect[i] = I_flerp(state->params.wallReflect[i], params->wallReflect[i], smooth);
    state->params.dryGain   = I_flerp(state->params.dryGain,   params->dryGain,   smooth);
    state->params.diffusion = I_flerp(state->params.diffusion, params->diffusion, smooth);
    state->params.earlySize = I_flerp(state->params.earlySize, params->earlySize, smooth);
    state->params.lateSize  = I_flerp(state->params.lateSize,  params->lateSize,  smooth);
    state->params.delayMatrix = params->delayMatrix;
    SND_RvDelayInit(state, params->delayMatrix);

    const float angle = state->params.diffusion * 1.5707964f;
    const float c = cosf(angle);
    const float s = sinf(angle);
    iassert(!IS_NAN(c));
    iassert(!IS_NAN(s));
    state->lateReflectionCoefs[0][0] =  c;
    state->lateReflectionCoefs[0][1] =  s;
    state->lateReflectionCoefs[1][0] =  s;
    state->lateReflectionCoefs[1][1] = -c;
    state->lateReflectionCoefs[2][2] = -c;
    state->lateReflectionCoefs[2][3] =  s;
    state->lateReflectionCoefs[3][2] =  s;
    state->lateReflectionCoefs[3][3] =  c;

    float coef[4];
    for (int i = 0; i < 4; ++i)
    {
        coef[i] = I_fmax(0.0f, (state->params.wallReflect[i] - 1.0f) * (float)i * 0.35355338f + 0.70710677f);
        iassert(!IS_NAN(coef[i]));
    }
    static const int earlyPerm[4][4] = {
        { 0, 3, 2, 1 },
        { 3, 0, 1, 2 },
        { 2, 1, 0, 3 },
        { 1, 2, 3, 0 },
    };
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            state->earlyReflectionCoefs[i][j] = coef[earlyPerm[i][j]];

    // Direct (unsigned int)(double * float) — Hex-Rays originally typed the
    // intermediate as _QWORD and earlier port retyped it as float, turning
    // the int64-truncate-low-32 idiom into a denormal float→uint conversion
    // that clamped every tap delay to 0.
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            state->earlyReflectionDelays[i][j] =
                (unsigned int)((double)state->earlyReflectionDelayBase[i][j] * params->earlySize);
            state->lateReflectionDelays[i][j] =
                (unsigned int)((double)state->lateReflectionDelayBase[i][j] * params->lateSize);
            iassert(state->earlyReflectionDelays[i][j] < 0x8000);
            iassert(state->lateReflectionDelays[i][j] < 0x8000);
        }
    }
}

double __cdecl I_flerp(float a, float b, float w)
{
    return (1.0 - w) * a + b * w;
}

void __cdecl SND_RvDelayInit(snd_rv_state *state, unsigned int values)
{

    //*(_QWORD *)&state->lateReflectionDelayBase[0][2] = 0;
    //*(_QWORD *)&state->lateReflectionDelayBase[1][2] = 0;
    //*(_QWORD *)&state->lateReflectionDelayBase[2][0] = 0;
    //*(_QWORD *)&state->lateReflectionDelayBase[3][0] = 0;
    state->lateReflectionDelayBase[0][2] = 0;
    state->lateReflectionDelayBase[0][3] = 0;
    state->lateReflectionDelayBase[1][2] = 0;
    state->lateReflectionDelayBase[1][3] = 0;
    state->lateReflectionDelayBase[2][0] = 0;
    state->lateReflectionDelayBase[2][1] = 0;
    state->lateReflectionDelayBase[3][0] = 0;
    state->lateReflectionDelayBase[3][1] = 0;
    if ( values )
    {
        state->earlyReflectionDelayBase[0][0] = 587;
        state->earlyReflectionDelayBase[0][1] = 1439;
        state->earlyReflectionDelayBase[0][2] = 1373;
        state->earlyReflectionDelayBase[0][3] = 643;
        state->earlyReflectionDelayBase[1][0] = 1493;
        state->earlyReflectionDelayBase[1][1] = 373;
        state->earlyReflectionDelayBase[1][2] = 911;
        state->earlyReflectionDelayBase[1][3] = 1091;
        state->earlyReflectionDelayBase[2][0] = 1201;
        state->earlyReflectionDelayBase[2][1] = 773;
        state->earlyReflectionDelayBase[2][2] = 521;
        state->earlyReflectionDelayBase[2][3] = 1637;
        state->earlyReflectionDelayBase[3][0] = 997;
        state->earlyReflectionDelayBase[3][1] = 1297;
        state->earlyReflectionDelayBase[3][2] = 1579;
        state->earlyReflectionDelayBase[3][3] = 439;
        state->lateReflectionDelayBase[0][0] = 293;
        state->lateReflectionDelayBase[0][1] = 479;
        state->lateReflectionDelayBase[1][0] = 1051;
        state->lateReflectionDelayBase[1][1] = 1237;
        state->lateReflectionDelayBase[2][2] = 1169;
        state->lateReflectionDelayBase[2][3] = 1663;
        state->lateReflectionDelayBase[3][2] = 113;
        state->lateReflectionDelayBase[3][3] = 607;
    }
    else
    {
        state->earlyReflectionDelayBase[0][0] = 941;
        state->earlyReflectionDelayBase[0][1] = 149;
        state->earlyReflectionDelayBase[0][2] = 673;
        state->earlyReflectionDelayBase[0][3] = 277;
        state->earlyReflectionDelayBase[1][0] = 211;
        state->earlyReflectionDelayBase[1][1] = 797;
        state->earlyReflectionDelayBase[1][2] = 421;
        state->earlyReflectionDelayBase[1][3] = 577;
        state->earlyReflectionDelayBase[2][0] = 751;
        state->earlyReflectionDelayBase[2][1] = 349;
        state->earlyReflectionDelayBase[2][2] = 1009;
        state->earlyReflectionDelayBase[2][3] = 173;
        state->earlyReflectionDelayBase[3][0] = 499;
        state->earlyReflectionDelayBase[3][1] = 613;
        state->earlyReflectionDelayBase[3][2] = 239;
        state->earlyReflectionDelayBase[3][3] = 877;
        state->lateReflectionDelayBase[0][0] = 1103;
        state->lateReflectionDelayBase[0][1] = 1283;
        state->lateReflectionDelayBase[1][0] = 1439;
        state->lateReflectionDelayBase[1][1] = 1619;
        state->lateReflectionDelayBase[2][2] = 1759;
        state->lateReflectionDelayBase[2][3] = 1949;
        state->lateReflectionDelayBase[3][2] = 751;
        state->lateReflectionDelayBase[3][3] = 941;
    }
}

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
                float *outRS)
{
    float *earlyLpfState; // edi
    float *v13; // eax
    float *v14; // ecx
    unsigned int delayIndex; // ebx
    float v16; // xmm4_4
    float v17; // xmm5_4
    float v18; // xmm0_4
    float v19; // xmm2_4
    snd_rv_state *v20; // edx
    float v21; // xmm7_4
    bool v22; // zf
    float v23; // xmm7_4
    float v24; // xmm0_4
    float v25; // xmm0_4
    float v26; // xmm2_4
    float v27; // xmm0_4
    float v28; // xmm0_4
    float v29; // xmm2_4
    float v30; // xmm0_4
    float v31; // xmm4_4
    float v32; // xmm0_4
    float v33; // xmm2_4
    float v34; // xmm0_4
    float v35; // xmm0_4
    float v36; // xmm2_4
    int v37; // [esp+10h] [ebp-10h]
    int v38; // [esp+14h] [ebp-Ch]
    int v39; // [esp+18h] [ebp-8h]
    int v40; // [esp+1Ch] [ebp-4h]
    unsigned int counta; // [esp+30h] [ebp+10h]
    unsigned int countb; // [esp+30h] [ebp+10h]
    const float *inRFa; // [esp+38h] [ebp+18h]
    int inLSa; // [esp+3Ch] [ebp+1Ch]
    const float *inRSa; // [esp+40h] [ebp+20h]
    float *outRFa; // [esp+48h] [ebp+28h]
    float *outLSa; // [esp+4Ch] [ebp+2Ch]
    float *outRSa; // [esp+50h] [ebp+30h]

    SND_RvFrameParam(params, state, (unsigned int)count);
    earlyLpfState = state->earlyLpfState;
    counta = 4;
    do
    {
        if ( IS_NAN(*(earlyLpfState - 4))
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_radverb.cpp",
                        298,
                        0,
                        "%s",
                        "!IS_NAN(state->inputLpfState[k])") )
        {
            __debugbreak();
        }
        if ( IS_NAN(*earlyLpfState)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_radverb.cpp",
                        299,
                        0,
                        "%s",
                        "!IS_NAN(state->earlyLpfState[k])") )
        {
            __debugbreak();
        }
        if ( IS_NAN(earlyLpfState[4])
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_radverb.cpp",
                        300,
                        0,
                        "%s",
                        "!IS_NAN(state->lateLpfState[k])") )
        {
            __debugbreak();
        }
        if ( IS_NAN(earlyLpfState[8])
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_radverb.cpp",
                        301,
                        0,
                        "%s",
                        "!IS_NAN(state->dampLpfState[k])") )
        {
            __debugbreak();
        }
        ++earlyLpfState;
        --counta;
    }
    while ( counta );
    if ( count )
    {
        v13 = outLS;
        v39 = (char *)inLS - (char *)outLS;
        v38 = (char *)inRF - (char *)outLS;
        v40 = (char *)outRF - (char *)outLS;
        v37 = (char *)inRS - (char *)outLS;
        inRSa = (const float *)((char *)outRS - (char *)outLS);
        outRSa = (float *)((char *)inLF - (char *)outLS);
        outRFa = (float *)((char *)outLF - (char *)outLS);
        inRFa = count;
        do
        {
            outLSa = state->dampLpfState;
            v14 = state->earlyReflectionCoefs[0];
            inLSa = 4;
            do
            {
                delayIndex = state->delayIndex;
                v16 = 0.0;
                v17 = 0.0;
                v18 = 0.0;
                v19 = 0.0;
                v20 = state;
                countb = 2;
                do
                {
                    v17 = (float)(v20->delayLine[delayIndex
                                                                         - *((unsigned int *)v14 + 16)
                                                                         + ((int)(delayIndex - *((unsigned int *)v14 + 16)) < 0 ? 0x8000 : 0)]
                                            * *v14)
                            + v17;
                    v16 = (float)(v20->delayLine[delayIndex
                                                                         - *((unsigned int *)v14 + 48)
                                                                         + ((int)(delayIndex - *((unsigned int *)v14 + 48)) < 0 ? 0x8000 : 0)]
                                            * v14[32])
                            + v16;
                    v19 = (float)(v20->delayLine[delayIndex
                                                                         - *((unsigned int *)v14 + 17)
                                                                         + 0x8000
                                                                         + ((int)(delayIndex - *((unsigned int *)v14 + 17)) < 0 ? 0x8000 : 0)]
                                            * v14[1])
                            + v19;
                    v21 = v20->delayLine[delayIndex
                                                         - *((unsigned int *)v14 + 49)
                                                         + 0x8000
                                                         + ((int)(delayIndex - *((unsigned int *)v14 + 49)) < 0 ? 0x8000 : 0)]
                            * v14[33];
                    v14 += 2;
                    v20 = (snd_rv_state *)((char *)v20 + 0x40000);
                    v22 = countb-- == 1;
                    v23 = v21 + v18;
                    v18 = v23;
                }
                while ( !v22 );
                *(outLSa - 8) = (float)((float)((float)(1.0 - state->params.earlyLpf) * state->params.earlyTime)
                                                            * (float)(v19 + v17))
                                            + (float)(state->params.earlyLpf * *(outLSa - 8));
                *outLSa = (float)((float)((float)(1.0 - state->params.dampLpf) * state->params.lateTime) * (float)(v23 + v16))
                                + (float)(state->params.dampLpf * *outLSa);
                v22 = inLSa-- == 1;
                ++outLSa;
            }
            while ( !v22 );
            *(float *)((char *)v13 + (unsigned int)outRFa) = (float)(state->dampLpfState[1] + state->dampLpfState[0]) * 0.70710677;
            *(float *)((char *)v13 + (unsigned int)inRSa) = (float)(state->dampLpfState[0] - state->dampLpfState[1]) * 0.70710677;
            *(float *)((char *)v13 + v40) = (float)(state->dampLpfState[3] - state->dampLpfState[2]) * 0.70710677;
            *v13 = (float)(state->dampLpfState[3] + state->dampLpfState[2]) * 0.70710677;
            v24 = (float)((float)(1.0 - state->params.inputLpf) * *(float *)((char *)v13 + v39))
                    + (float)(state->params.inputLpf * state->inputLpfState[0]);
            state->inputLpfState[0] = v24;
            state->delayLine[state->delayIndex] = v24 + *v13;
            v25 = (float)((float)(1.0 - state->params.lateLpf) * *v13)
                    + (float)(state->params.lateLpf * state->lateLpfState[0]);
            v26 = state->inputLpfState[0];
            state->lateLpfState[0] = v25;
            *v13 = (float)((float)((float)(v26 * state->params.dryGain) + (float)(v25 * state->params.lateGainProx[0]))
                                     + (float)(state->params.earlyGain * state->earlyLpfState[0]))
                     * state->params.returnGain;
            v27 = (float)((float)(1.0 - state->params.inputLpf) * *(float *)((char *)v13 + v38))
                    + (float)(state->params.inputLpf * state->inputLpfState[1]);
            state->inputLpfState[1] = v27;
            state->delayLine[state->delayIndex + 0x8000] = v27 + *(float *)((char *)v13 + v40);
            v28 = (float)((float)(1.0 - state->params.lateLpf) * *(float *)((char *)v13 + v40))
                    + (float)(state->params.lateLpf * state->lateLpfState[1]);
            v29 = state->params.lateGainProx[1] * v28;
            state->lateLpfState[1] = v28;
            *(float *)((char *)v13 + v40) = (float)((float)(v29 + (float)(state->inputLpfState[1] * state->params.dryGain))
                                                                                        + (float)(state->params.earlyGain * state->earlyLpfState[1]))
                                                                        * state->params.returnGain;
            v30 = (float)((float)(1.0 - state->params.inputLpf) * *(float *)((char *)v13 + v37))
                    + (float)(state->params.inputLpf * state->inputLpfState[2]);
            state->inputLpfState[2] = v30;
            state->delayLine[state->delayIndex + 0x10000] = v30 + *(float *)((char *)v13 + (unsigned int)inRSa);
            v31 = state->earlyLpfState[2];
            v32 = (float)((float)(1.0 - state->params.lateLpf) * *(float *)((char *)v13 + (unsigned int)inRSa))
                    + (float)(state->params.lateLpf * state->lateLpfState[2]);
            v33 = state->inputLpfState[2];
            state->lateLpfState[2] = v32;
            *(float *)((char *)v13 + (unsigned int)inRSa) = (float)((float)((float)(v33 * state->params.dryGain)
                                                                                                                            + (float)(v31 * state->params.earlyGain))
                                                                                                            + (float)(v32 * state->params.lateGainProx[2]))
                                                                                            * state->params.returnGain;
            v34 = (float)((float)(1.0 - state->params.inputLpf) * *(float *)((char *)v13 + (unsigned int)outRSa))
                    + (float)(state->params.inputLpf * state->inputLpfState[3]);
            state->inputLpfState[3] = v34;
            state->delayLine[state->delayIndex + 98304] = v34 + *(float *)((char *)v13 + (unsigned int)outRFa);
            v35 = (float)((float)(1.0 - state->params.lateLpf) * *(float *)((char *)v13 + (unsigned int)outRFa))
                    + (float)(state->params.lateLpf * state->lateLpfState[3]);
            v36 = state->params.lateGainProx[3];
            state->lateLpfState[3] = v35;
            *(float *)((char *)v13 + (unsigned int)outRFa) = (float)((float)((float)(v36 * v35)
                                                                                                                             + (float)(state->params.earlyGain
                                                                                                                                             * state->earlyLpfState[3]))
                                                                                                             + (float)(state->inputLpfState[3] * state->params.dryGain))
                                                                                             * state->params.returnGain;
            if ( ++state->delayIndex >= 0x8000 )
                state->delayIndex = 0;
            ++v13;
            inRFa = (const float *)((char *)inRFa - 1);
        }
        while ( inRFa );
    }
}

