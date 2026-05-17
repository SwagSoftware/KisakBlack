#include "rb_fog.h"
#include "r_dvars.h"
#include "r_scene.h"
#include "r_state.h"

void    R_SetFrameFog(GfxCmdBufInput *input, const GfxViewInfo *viewInfo)
{
    float parms2[4]; // [esp+34h] [ebp-68h] BYREF
    float fog_maxDensity; // [esp+44h] [ebp-58h]
    float fog_density; // [esp+48h] [ebp-54h]
    float viewerZ; // [esp+4Ch] [ebp-50h]
    float fogColorVec[5]; // [esp+50h] [ebp-4Ch] BYREF
    float parms[4]; // [esp+64h] [ebp-38h] BYREF
    float endAng; // [esp+74h] [ebp-28h]
    float startAng; // [esp+7Ch] [ebp-20h]
    float slopeAng; // [esp+84h] [ebp-18h]
    const GfxFog *fog; // [esp+88h] [ebp-14h]
    const GfxBackEndData *data; // [esp+8Ch] [ebp-10h]
    //_UNKNOWN *v26; // [esp+90h] [ebp-Ch]
    //GfxCmdBufInput *inputa; // [esp+94h] [ebp-8h]
    //int vars0; // [esp+9Ch] [ebp+0h]
    //
    //v26 = a1;
    //inputa = (GfxCmdBufInput *)vars0;

    if (r_fog->current.enabled)
    {
        data = input->data;
        iassert(data);
        fog = &data->fogSettings;
        R_SetInputCodeConstantFromVec4(input, CONST_SRC_CODE_SUN_FOG_COLOR, data->fogSettings.sunFogColor);
        R_SetInputCodeConstantFromVec4(input, CONST_SRC_CODE_SUN_FOG_DIR, data->fogSettings.sunFogDir);
        slopeAng = 1.0e7;
        startAng = cos(data->fogSettings.sunFogStartAng * 0.017453292);
        endAng = cos(data->fogSettings.sunFogEndAng * 0.017453292);

        if ((startAng - endAng) != 0.0)
            slopeAng = 1.0 / (startAng - endAng);

        parms[0] = (-(endAng)) * slopeAng;
        parms[1] = slopeAng;
        parms[2] = 0.0f;
        parms[3] = 0.0f;
        R_SetInputCodeConstantFromVec4(input, CONST_SRC_CODE_SUN_FOG, parms);
        fogColorVec[0] = fog->color[0];
        fogColorVec[1] = fog->color[1];
        fogColorVec[2] = fog->color[2];
        fogColorVec[3] = fog->color[3];
        R_SetInputCodeConstantFromVec4(input, CONST_SRC_CODE_FOG_COLOR, fogColorVec);
        viewerZ = viewInfo->cullViewInfo.viewParms.origin[2] - fog->baseHeight;
        fog_density = fog->density;
        fog_maxDensity = fog->maxDensity;
        if (fog_density == 0.0)
        {
            R_SetInputCodeConstant(input, CONST_SRC_CODE_FOG, 0.0, 0.0, 0.0, 0.0);
            R_SetInputCodeConstant(input, CONST_SRC_CODE_FOG2, 0.0, 0.0, 0.0, 0.0);
        }
        else
        {
            memset(&parms2[1], 0, 12);

            if (fog_maxDensity > (100.0 * fog_density))
                fog_maxDensity = 100.0 * fog_density;
            if (fog_density > fog_maxDensity)
                fog_maxDensity = fog_density;

            parms[0] = log((float)(fog_density / fog_maxDensity)) - (float)((float)(fog->heightDensity * viewerZ) * log(2.0));
            if (parms[0] >= 0.0)
            {
                parms2[0] = parms[0] + 1.0;
            }
            else
            {
                parms2[0] = exp(parms[0]);
            }

            parms[1] = -fog_maxDensity;
            parms[2] = fog_density * fog->fogStart;
            parms[3] = -fog->heightDensity * log(2.0);

            if (fog->heightDensity > 0.0)
            {
                parms[1] *= log(2.0);
            }
            R_SetInputCodeConstantFromVec4(input, CONST_SRC_CODE_FOG, parms);
            R_SetInputCodeConstantFromVec4(input, CONST_SRC_CODE_FOG2, parms2);
        }
    }
    else
    {
        R_SetInputCodeConstant(input, CONST_SRC_CODE_FOG, 0.0, 0.0, 0.0, 0.0);
        R_SetInputCodeConstant(input, CONST_SRC_CODE_FOG2, 0.0, 0.0, 0.0, 0.0);
    }
}

