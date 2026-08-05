#include "ik_math.h"
#include <universal/assertive.h>
#include <cstring>
#include <cgame_mp/cg_pose_mp.h>
#include <universal/com_math_anglevectors.h>

float fMaxFootReachSubtract;
float fMaxHandReachSubtract;

void __cdecl ikMatrixIdentity44(float (*out)[4])
{
    if ( !out && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp", 221, 0, "%s", "out") )
        __debugbreak();
    memcpy(out, ikIdentityMatrix44, 0x40u);
}

void __cdecl ikMatrixSet44(float (*out)[4], const float *origin, const float (*axis)[3], float scale)
{
    (*out)[0] = (*axis)[0] * scale;
    (*out)[1] = (float)(*axis)[1] * scale;
    (*out)[2] = (float)(*axis)[2] * scale;
    (*out)[3] = 0.0f;
    (*out)[4] = (float)(*axis)[3] * scale;
    (*out)[5] = (float)(*axis)[4] * scale;
    (*out)[6] = (float)(*axis)[5] * scale;
    (*out)[7] = 0.0f;
    (*out)[8] = (float)(*axis)[6] * scale;
    (*out)[9] = (float)(*axis)[7] * scale;
    (*out)[10] = (float)(*axis)[8] * scale;
    (*out)[11] = 0.0f;
    (*out)[12] = *origin;
    (*out)[13] = origin[1];
    (*out)[14] = origin[2];
    (*out)[15] = 1.0f;
}

void __cdecl ikMatrixTransformVector34(const float *in1, const float (*in2)[4], float *out)
{
    if ( in1 == out && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp", 550, 0, "%s", "in1 != out") )
        __debugbreak();
    *out = (float)((float)(*in1 * (*in2)[0]) + (float)(in1[1] * (float)(*in2)[4])) + (float)(in1[2] * (float)(*in2)[8]);
    out[1] = (float)((float)(*in1 * (float)(*in2)[1]) + (float)(in1[1] * (float)(*in2)[5]))
                 + (float)(in1[2] * (float)(*in2)[9]);
    out[2] = (float)((float)(*in1 * (float)(*in2)[2]) + (float)(in1[1] * (float)(*in2)[6]))
                 + (float)(in1[2] * (float)(*in2)[10]);
}

void __cdecl ikQuatTransToMatrix44(float *quat, const float *trans, float (*out)[4])
{
    float yy; // [esp+0h] [ebp-48h]
    float xy; // [esp+8h] [ebp-40h]
    float z; // [esp+Ch] [ebp-3Ch]
    float zz; // [esp+10h] [ebp-38h]
    float zw; // [esp+14h] [ebp-34h]
    float yw; // [esp+20h] [ebp-28h]
    float xz; // [esp+24h] [ebp-24h]
    float yz; // [esp+28h] [ebp-20h]
    float xx; // [esp+2Ch] [ebp-1Ch]
    float xw; // [esp+30h] [ebp-18h]
    float magSqr; // [esp+38h] [ebp-10h]
    float x; // [esp+3Ch] [ebp-Ch]
    float y; // [esp+40h] [ebp-8h]
    float w; // [esp+44h] [ebp-4h]

    x = *quat;
    y = quat[1];
    z = quat[2];
    w = quat[3];
    magSqr = (float)((float)((float)(x * x) + (float)(y * y)) + (float)(z * z)) + (float)(w * w);
    if ( magSqr <= 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp", 600, 0, "%s", "magSqr > 0.0f") )
    {
        __debugbreak();
    }
    xx = (float)(x * x) * (float)(2.0 / magSqr);
    yy = (float)(y * y) * (float)(2.0 / magSqr);
    zz = (float)(z * z) * (float)(2.0 / magSqr);
    xy = (float)((float)(2.0 / magSqr) * x) * y;
    xz = (float)((float)(2.0 / magSqr) * x) * z;
    xw = (float)((float)(2.0 / magSqr) * x) * w;
    yz = (float)((float)(2.0 / magSqr) * y) * z;
    yw = (float)((float)(2.0 / magSqr) * y) * w;
    zw = (float)((float)(2.0 / magSqr) * z) * w;
    (*out)[0] = 1.0 - (float)(yy + zz);
    (*out)[1] = xy + zw;
    (*out)[2] = xz - yw;
    (*out)[3] = 0.0f;
    (*out)[4] = xy - zw;
    (*out)[5] = 1.0 - (float)(xx + zz);
    (*out)[6] = yz + xw;
    (*out)[7] = 0.0f;
    (*out)[8] = xz + yw;
    (*out)[9] = yz - xw;
    (*out)[10] = 1.0 - (float)(xx + yy);
    (*out)[11] = 0.0f;
    (*out)[12] = *trans;
    (*out)[13] = trans[1];
    (*out)[14] = trans[2];
    (*out)[15] = 1.0f;
}

void __cdecl ikMatrix44ToQuatTrans(float (*mat)[4], float *quat, float *trans)
{
    float axis[3][3]; // [esp+14h] [ebp-24h] BYREF

    *(_QWORD *)&axis[0][0] = *(_QWORD *)&(*mat)[0];
    axis[0][2] = (*mat)[2];
    axis[1][0] = (*mat)[4];
    axis[1][1] = (*mat)[5];
    axis[1][2] = (*mat)[6];
    axis[2][0] = (*mat)[8];
    axis[2][1] = (*mat)[9];
    axis[2][2] = (*mat)[10];
    ikAxisToQuat(axis, quat);
    *trans = (*mat)[12];
    trans[1] = (*mat)[13];
    trans[2] = (*mat)[14];
}

void __cdecl ikAxisToQuat(const float (*mat)[3], float *out)
{
    float *v2; // [esp+8h] [ebp-50h]
    float invLength; // [esp+Ch] [ebp-4Ch]
    float test[4][4]; // [esp+10h] [ebp-48h] BYREF
    int best; // [esp+50h] [ebp-8h]
    float testSizeSq; // [esp+54h] [ebp-4h]

    test[0][0] = (float)(*mat)[5] - (float)(*mat)[7];
    test[0][1] = (float)(*mat)[6] - (float)(*mat)[2];
    test[0][2] = (float)(*mat)[1] - (float)(*mat)[3];
    test[0][3] = (float)((float)((*mat)[0] + (float)(*mat)[4]) + (float)(*mat)[8]) + 1.0;
    testSizeSq = Vec4LengthSq(test[0]);
    if ( testSizeSq < 1.0 )
    {
        test[1][0] = (float)(*mat)[6] + (float)(*mat)[2];
        test[1][1] = (float)(*mat)[7] + (float)(*mat)[5];
        test[1][2] = (float)((float)((float)(*mat)[8] - (float)(*mat)[4]) - (*mat)[0]) + 1.0;
        test[1][3] = test[0][2];
        testSizeSq = Vec4LengthSq(test[1]);
        if ( testSizeSq < 1.0 )
        {
            test[2][0] = (float)((float)((*mat)[0] - (float)(*mat)[4]) - (float)(*mat)[8]) + 1.0;
            test[2][1] = (float)(*mat)[3] + (float)(*mat)[1];
            *(_QWORD *)&test[2][2] = __PAIR64__(LODWORD(test[0][0]), LODWORD(test[1][0]));
            testSizeSq = Vec4LengthSq(test[2]);
            if ( testSizeSq < 1.0 )
            {
                test[3][0] = test[2][1];
                test[3][1] = (float)((float)((float)(*mat)[4] - (*mat)[0]) - (float)(*mat)[8]) + 1.0;
                *(_QWORD *)&test[3][2] = __PAIR64__(LODWORD(test[0][1]), LODWORD(test[1][1]));
                testSizeSq = Vec4LengthSq(test[3]);
                if ( testSizeSq < 1.0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                                704,
                                0,
                                "%s\n\t(testSizeSq) = %g",
                                "(testSizeSq >= 1.0f)",
                                testSizeSq) )
                {
                    __debugbreak();
                }
                best = 3;
            }
            else
            {
                best = 2;
            }
        }
        else
        {
            best = 1;
        }
    }
    else
    {
        best = 0;
    }
    if ( testSizeSq == 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp", 709, 0, "%s", "testSizeSq") )
    {
        __debugbreak();
    }
    invLength = 1.0 / sqrtf(testSizeSq);
    v2 = test[best];
    *out = invLength * *v2;
    out[1] = invLength * v2[1];
    out[2] = invLength * v2[2];
    out[3] = invLength * v2[3];
}

void __cdecl ikMat44Set(float (*mat)[4], const float *v1, const float *v2, const float *v3, const float *pos)
{
    (*mat)[0] = *v1;
    (*mat)[1] = v1[1];
    (*mat)[2] = v1[2];
    (*mat)[4] = *v2;
    (*mat)[5] = v2[1];
    (*mat)[6] = v2[2];
    (*mat)[8] = *v3;
    (*mat)[9] = v3[1];
    (*mat)[10] = v3[2];
    (*mat)[12] = *pos;
    (*mat)[13] = pos[1];
    (*mat)[14] = pos[2];
    (*mat)[3] = 0.0f;
    (*mat)[7] = 0.0f;
    (*mat)[11] = 0.0f;
    (*mat)[15] = 1.0f;
}

// local variable allocation has failed, the output may be wrong!
void    ikCalcBoneModelMatrix_r(
                IKState *ikState,
                int boneNum,
                float *inMatArray,
                float (*out)[4])
{
    float v5; // [esp-218h] [ebp-3D8h]
    float v6; // [esp-214h] [ebp-3D4h]
    float v7; // [esp-210h] [ebp-3D0h]
    float v8; // [esp-20Ch] [ebp-3CCh]
    float v9; // [esp-208h] [ebp-3C8h]
    float v10; // [esp-204h] [ebp-3C4h]
    float v11; // [esp-200h] [ebp-3C0h]
    float v12; // [esp-1FCh] [ebp-3BCh]
    float v13; // [esp-1F8h] [ebp-3B8h]
    float v14; // [esp-1F4h] [ebp-3B4h]
    float v15; // [esp-1F0h] [ebp-3B0h]
    float v16; // [esp-1ECh] [ebp-3ACh]
    float v17; // [esp-1E8h] [ebp-3A8h]
    float v18; // [esp-1E4h] [ebp-3A4h]
    float v19; // [esp-1E0h] [ebp-3A0h]
    float *v20; // [esp-1D4h] [ebp-394h]
    float v21; // [esp-1B0h] [ebp-370h]
    float v22; // [esp-1ACh] [ebp-36Ch]
    float v23; // [esp-1A8h] [ebp-368h]
    float v24; // [esp-1A4h] [ebp-364h]
    float v25; // [esp-1A0h] [ebp-360h]
    float v26; // [esp-19Ch] [ebp-35Ch]
    float v27; // [esp-198h] [ebp-358h]
    float v28; // [esp-194h] [ebp-354h]
    float v29; // [esp-190h] [ebp-350h]
    float v30; // [esp-18Ch] [ebp-34Ch]
    float v31; // [esp-188h] [ebp-348h]
    float v32; // [esp-184h] [ebp-344h]
    float v33; // [esp-180h] [ebp-340h]
    float v34; // [esp-17Ch] [ebp-33Ch]
    float v35; // [esp-178h] [ebp-338h]
    float *outMat; // [esp-16Ch] [ebp-32Ch]
    float v37; // [esp-148h] [ebp-308h]
    float v38; // [esp-144h] [ebp-304h]
    float v39; // [esp-140h] [ebp-300h]
    float v40; // [esp-13Ch] [ebp-2FCh]
    float v41; // [esp-138h] [ebp-2F8h]
    float v42; // [esp-134h] [ebp-2F4h]
    float v43; // [esp-130h] [ebp-2F0h]
    float v44; // [esp-12Ch] [ebp-2ECh]
    float v45; // [esp-128h] [ebp-2E8h]
    float v46; // [esp-124h] [ebp-2E4h]
    float v47; // [esp-120h] [ebp-2E0h]
    float v48; // [esp-11Ch] [ebp-2DCh]
    float v49; // [esp-118h] [ebp-2D8h]
    float v50; // [esp-114h] [ebp-2D4h]
    float v51; // [esp-110h] [ebp-2D0h]
    float *v52; // [esp-108h] [ebp-2C8h]
    float v53; // [esp-F0h] [ebp-2B0h]
    float v54; // [esp-ECh] [ebp-2ACh]
    float v55; // [esp-E8h] [ebp-2A8h]
    float v56; // [esp-E4h] [ebp-2A4h]
    float v57; // [esp-E0h] [ebp-2A0h]
    float v58; // [esp-DCh] [ebp-29Ch]
    float v59; // [esp-D8h] [ebp-298h]
    float v60; // [esp-D4h] [ebp-294h]
    float v61; // [esp-D0h] [ebp-290h]
    float v62; // [esp-CCh] [ebp-28Ch]
    float v63; // [esp-C8h] [ebp-288h]
    float v64; // [esp-C4h] [ebp-284h]
    float v65; // [esp-C0h] [ebp-280h]
    float v66; // [esp-BCh] [ebp-27Ch]
    float v67; // [esp-B8h] [ebp-278h]
    float v68; // [esp-50h] [ebp-210h]
    float v69; // [esp-4Ch] [ebp-20Ch]
    float v70; // [esp-48h] [ebp-208h]
    float v71; // [esp-44h] [ebp-204h]
    float v72; // [esp-40h] [ebp-200h]
    float v73; // [esp-3Ch] [ebp-1FCh]
    float v74; // [esp-38h] [ebp-1F8h]
    float v75; // [esp-34h] [ebp-1F4h]
    float v76; // [esp-30h] [ebp-1F0h]
    float v77; // [esp-2Ch] [ebp-1ECh]
    float v78; // [esp-28h] [ebp-1E8h]
    float v79; // [esp-24h] [ebp-1E4h]
    float v80; // [esp-20h] [ebp-1E0h]
    float v81; // [esp-1Ch] [ebp-1DCh]
    float v82; // [esp-18h] [ebp-1D8h]
    float *pMat; // [esp-10h] [ebp-1D0h]
    float mat[4][4]; // [esp-Ch] [ebp-1CCh] OVERLAPPED BYREF
    float v85; // [esp+50h] [ebp-170h]
    float v86; // [esp+54h] [ebp-16Ch]
    float v87; // [esp+58h] [ebp-168h]
    float v88; // [esp+5Ch] [ebp-164h]
    float v89; // [esp+60h] [ebp-160h]
    float v90; // [esp+64h] [ebp-15Ch]
    float v91; // [esp+68h] [ebp-158h]
    float v92; // [esp+6Ch] [ebp-154h]
    float v93; // [esp+70h] [ebp-150h]
    float v94; // [esp+74h] [ebp-14Ch]
    float v95; // [esp+78h] [ebp-148h]
    float v96; // [esp+7Ch] [ebp-144h]
    float v97; // [esp+80h] [ebp-140h]
    float v98; // [esp+84h] [ebp-13Ch]
    float v99; // [esp+88h] [ebp-138h]
    float *v100; // [esp+90h] [ebp-130h]
    float *v101; // [esp+94h] [ebp-12Ch]
    float *v102; // [esp+98h] [ebp-128h]
    float v103; // [esp+9Ch] [ebp-124h]
    float v104; // [esp+A0h] [ebp-120h]
    float v105; // [esp+A4h] [ebp-11Ch]
    float v106; // [esp+A8h] [ebp-118h]
    float v107; // [esp+ACh] [ebp-114h]
    float v108; // [esp+B0h] [ebp-110h]
    float v109; // [esp+B4h] [ebp-10Ch]
    float v110; // [esp+B8h] [ebp-108h]
    float v111; // [esp+BCh] [ebp-104h]
    float v112; // [esp+C0h] [ebp-100h]
    float v113; // [esp+C4h] [ebp-FCh]
    float v114; // [esp+C8h] [ebp-F8h]
    float v115; // [esp+CCh] [ebp-F4h]
    float v116; // [esp+D0h] [ebp-F0h]
    float v117; // [esp+D4h] [ebp-ECh]
    float v118; // [esp+D8h] [ebp-E8h]
    float *v119; // [esp+DCh] [ebp-E4h]
    float *v120; // [esp+E0h] [ebp-E0h]
    float *v121; // [esp+E4h] [ebp-DCh]
    float *v122; // [esp+E8h] [ebp-D8h]
    float v123; // [esp+ECh] [ebp-D4h]
    float v124; // [esp+F0h] [ebp-D0h]
    float v125; // [esp+F4h] [ebp-CCh]
    float v126; // [esp+F8h] [ebp-C8h]
    float v127; // [esp+FCh] [ebp-C4h]
    float v128; // [esp+100h] [ebp-C0h]
    float v129; // [esp+104h] [ebp-BCh]
    float v130; // [esp+108h] [ebp-B8h]
    float v131; // [esp+10Ch] [ebp-B4h]
    float v132; // [esp+110h] [ebp-B0h]
    float v133; // [esp+114h] [ebp-ACh]
    float v134; // [esp+118h] [ebp-A8h]
    float v135; // [esp+11Ch] [ebp-A4h]
    float v136; // [esp+120h] [ebp-A0h]
    float v137; // [esp+124h] [ebp-9Ch]
    float v138; // [esp+128h] [ebp-98h]
    float *v139; // [esp+130h] [ebp-90h]
    float *v140; // [esp+134h] [ebp-8Ch]
    float *v141; // [esp+138h] [ebp-88h]
    int parentBone; // [esp+13Ch] [ebp-84h]
    float *v143; // [esp+150h] [ebp-70h]
    float *v144; // [esp+154h] [ebp-6Ch]
    float *v145; // [esp+158h] [ebp-68h]
    float v146; // [esp+15Ch] [ebp-64h]
    float v147; // [esp+160h] [ebp-60h]
    float v148; // [esp+164h] [ebp-5Ch]
    float v149; // [esp+168h] [ebp-58h]
    float v150; // [esp+16Ch] [ebp-54h]
    float v151; // [esp+170h] [ebp-50h]
    float v152; // [esp+174h] [ebp-4Ch]
    float v153; // [esp+178h] [ebp-48h]
    float v154; // [esp+17Ch] [ebp-44h]
    float v155; // [esp+180h] [ebp-40h]
    float v156; // [esp+184h] [ebp-3Ch]
    float v157; // [esp+188h] [ebp-38h]
    float v158; // [esp+18Ch] [ebp-34h]
    float v159; // [esp+190h] [ebp-30h]
    float v160; // [esp+194h] [ebp-2Ch]
    float v161; // [esp+198h] [ebp-28h]
    float *v162; // [esp+1A0h] [ebp-20h]
    float *v163; // [esp+1A4h] [ebp-1Ch]
    float (*cache)[4][4]; // [esp+1A8h] [ebp-18h]
    float (*matArray)[4][4]; // [esp+1ACh] [ebp-14h]
    //_UNKNOWN *v166[2]; // [esp+1B4h] [ebp-Ch] BYREF
    //float *inMatArraya; // [esp+1C0h] [ebp+0h]
    //
    //v166[0] = a1;
    //v166[1] = inMatArraya;
    matArray = (float (*)[4][4])inMatArray;
    cache = 0;
    if (inMatArray == (float *)ikState->matArrayPostIK)
    {
        cache = ikState->matArrayCache;
    }
    else if (inMatArray == (float *)ikState->matArrayPreIK)
    {
        cache = ikState->matArrayCachePre;
    }
    if (!cache || cache[boneNum][3][3] == 0.0)
    {
        parentBone = ikState->localIkSystem->ikBoneParents[boneNum];
        if (parentBone == -1)
        {
            outMat = matArray[boneNum][0];
            v21 = outMat[1];
            v22 = outMat[2];
            v23 = outMat[3];
            v24 = outMat[4];
            v25 = outMat[5];
            v26 = outMat[6];
            v27 = outMat[7];
            v28 = outMat[8];
            v29 = outMat[9];
            v30 = outMat[10];
            v31 = outMat[11];
            v32 = outMat[12];
            v33 = outMat[13];
            v34 = outMat[14];
            v35 = outMat[15];
            (*out)[0] = *outMat;
            (*out)[1] = v21;
            (*out)[2] = v22;
            (*out)[3] = v23;
            (*out)[4] = v24;
            (*out)[5] = v25;
            (*out)[6] = v26;
            (*out)[7] = v27;
            (*out)[8] = v28;
            (*out)[9] = v29;
            (*out)[10] = v30;
            (*out)[11] = v31;
            (*out)[12] = v32;
            (*out)[13] = v33;
            (*out)[14] = v34;
            (*out)[15] = v35;
            ikNormalizedMatrixAssert_func(out);
            if (cache)
            {
                v20 = cache[boneNum][0];
                v5 = (*out)[1];
                v6 = (*out)[2];
                v7 = (*out)[3];
                v8 = (*out)[4];
                v9 = (*out)[5];
                v10 = (*out)[6];
                v11 = (*out)[7];
                v12 = (*out)[8];
                v13 = (*out)[9];
                v14 = (*out)[10];
                v15 = (*out)[11];
                v16 = (*out)[12];
                v17 = (*out)[13];
                v18 = (*out)[14];
                v19 = (*out)[15];
                *v20 = (*out)[0];
                v20[1] = v5;
                v20[2] = v6;
                v20[3] = v7;
                v20[4] = v8;
                v20[5] = v9;
                v20[6] = v10;
                v20[7] = v11;
                v20[8] = v12;
                v20[9] = v13;
                v20[10] = v14;
                v20[11] = v15;
                v20[12] = v16;
                v20[13] = v17;
                v20[14] = v18;
                v20[15] = v19;
                ikState->cacheActive = 1;
            }
        }
        else if (!cache || cache[parentBone][3][3] == 0.0)
        {
            ikCalcBoneModelMatrix_r(ikState, parentBone, inMatArray, mat);
            ikNormalizedMatrixAssert_func(mat);
            pMat = matArray[boneNum][0];
            v68 = pMat[1];
            v69 = pMat[2];
            v70 = pMat[3];
            v71 = pMat[4];
            v72 = pMat[5];
            v73 = pMat[6];
            v74 = pMat[7];
            v75 = pMat[8];
            v76 = pMat[9];
            v77 = pMat[10];
            v78 = pMat[11];
            v79 = pMat[12];
            v80 = pMat[13];
            v81 = pMat[14];
            v82 = pMat[15];
            v53 = (float)((float)((float)(*pMat * mat[0][1]) + (float)(v68 * mat[1][1])) + (float)(v69 * mat[2][1]))
                + (float)(v70 * mat[3][1]);
            v54 = (float)((float)((float)(*pMat * mat[0][2]) + (float)(v68 * mat[1][2])) + (float)(v69 * mat[2][2]))
                + (float)(v70 * mat[3][2]);
            v55 = (float)((float)((float)(*pMat * mat[0][3]) + (float)(v68 * mat[1][3])) + (float)(v69 * mat[2][3]))
                + (float)(v70 * mat[3][3]);
            v56 = (float)((float)((float)(v71 * mat[0][0]) + (float)(v72 * mat[1][0])) + (float)(v73 * mat[2][0]))
                + (float)(v74 * mat[3][0]);
            v57 = (float)((float)((float)(v71 * mat[0][1]) + (float)(v72 * mat[1][1])) + (float)(v73 * mat[2][1]))
                + (float)(v74 * mat[3][1]);
            v58 = (float)((float)((float)(v71 * mat[0][2]) + (float)(v72 * mat[1][2])) + (float)(v73 * mat[2][2]))
                + (float)(v74 * mat[3][2]);
            v59 = (float)((float)((float)(v71 * mat[0][3]) + (float)(v72 * mat[1][3])) + (float)(v73 * mat[2][3]))
                + (float)(v74 * mat[3][3]);
            v60 = (float)((float)((float)(v75 * mat[0][0]) + (float)(v76 * mat[1][0])) + (float)(v77 * mat[2][0]))
                + (float)(v78 * mat[3][0]);
            v61 = (float)((float)((float)(v75 * mat[0][1]) + (float)(v76 * mat[1][1])) + (float)(v77 * mat[2][1]))
                + (float)(v78 * mat[3][1]);
            v62 = (float)((float)((float)(v75 * mat[0][2]) + (float)(v76 * mat[1][2])) + (float)(v77 * mat[2][2]))
                + (float)(v78 * mat[3][2]);
            v63 = (float)((float)((float)(v75 * mat[0][3]) + (float)(v76 * mat[1][3])) + (float)(v77 * mat[2][3]))
                + (float)(v78 * mat[3][3]);
            v64 = (float)((float)((float)(v79 * mat[0][0]) + (float)(v80 * mat[1][0])) + (float)(v81 * mat[2][0]))
                + (float)(v82 * mat[3][0]);
            v65 = (float)((float)((float)(v79 * mat[0][1]) + (float)(v80 * mat[1][1])) + (float)(v81 * mat[2][1]))
                + (float)(v82 * mat[3][1]);
            v66 = (float)((float)((float)(v79 * mat[0][2]) + (float)(v80 * mat[1][2])) + (float)(v81 * mat[2][2]))
                + (float)(v82 * mat[3][2]);
            v67 = (float)((float)((float)(v79 * mat[0][3]) + (float)(v80 * mat[1][3])) + (float)(v81 * mat[2][3]))
                + (float)(v82 * mat[3][3]);
            (*out)[0] = (float)((float)((float)(*pMat * mat[0][0]) + (float)(v68 * mat[1][0])) + (float)(v69 * mat[2][0]))
                + (float)(v70 * mat[3][0]);
            (*out)[1] = v53;
            (*out)[2] = v54;
            (*out)[3] = v55;
            (*out)[4] = v56;
            (*out)[5] = v57;
            (*out)[6] = v58;
            (*out)[7] = v59;
            (*out)[8] = v60;
            (*out)[9] = v61;
            (*out)[10] = v62;
            (*out)[11] = v63;
            (*out)[12] = v64;
            (*out)[13] = v65;
            (*out)[14] = v66;
            (*out)[15] = v67;
            ikNormalizedMatrixAssert_func(out);
            if (cache)
            {
                v52 = cache[boneNum][0];
                v37 = (*out)[1];
                v38 = (*out)[2];
                v39 = (*out)[3];
                v40 = (*out)[4];
                v41 = (*out)[5];
                v42 = (*out)[6];
                v43 = (*out)[7];
                v44 = (*out)[8];
                v45 = (*out)[9];
                v46 = (*out)[10];
                v47 = (*out)[11];
                v48 = (*out)[12];
                v49 = (*out)[13];
                v50 = (*out)[14];
                v51 = (*out)[15];
                *v52 = (*out)[0];
                v52[1] = v37;
                v52[2] = v38;
                v52[3] = v39;
                v52[4] = v40;
                v52[5] = v41;
                v52[6] = v42;
                v52[7] = v43;
                v52[8] = v44;
                v52[9] = v45;
                v52[10] = v46;
                v52[11] = v47;
                v52[12] = v48;
                v52[13] = v49;
                v52[14] = v50;
                v52[15] = v51;
                ikState->cacheActive = 1;
            }
        }
        else
        {
            ikNormalizedMatrixAssert_func(cache[parentBone]);
            ikNormalizedMatrixAssert_func(matArray[boneNum]);
            v141 = cache[parentBone][0];
            v140 = matArray[boneNum][0];
            v139 = v140;
            v123 = *v140;
            v124 = v140[1];
            v125 = v140[2];
            v126 = v140[3];
            v122 = v140 + 4;
            v127 = v140[4];
            v128 = v140[5];
            v129 = v140[6];
            v130 = v140[7];
            v121 = v140 + 8;
            v131 = v140[8];
            v132 = v140[9];
            v133 = v140[10];
            v134 = v140[11];
            v120 = v140 + 12;
            v135 = v140[12];
            v136 = v140[13];
            v137 = v140[14];
            v138 = v140[15];
            v119 = v141;
            v103 = *v141;
            v104 = v141[1];
            v105 = v141[2];
            v106 = v141[3];
            v102 = v141 + 4;
            v107 = v141[4];
            v108 = v141[5];
            v109 = v141[6];
            v110 = v141[7];
            v101 = v141 + 8;
            v111 = v141[8];
            v112 = v141[9];
            v113 = v141[10];
            v114 = v141[11];
            v100 = v141 + 12;
            v115 = v141[12];
            v116 = v141[13];
            v117 = v141[14];
            v118 = v141[15];
            v85 = (float)((float)((float)(v123 * v104) + (float)(v124 * v108)) + (float)(v125 * v112)) + (float)(v126 * v116);
            v86 = (float)((float)((float)(v123 * v105) + (float)(v124 * v109)) + (float)(v125 * v113)) + (float)(v126 * v117);
            v87 = (float)((float)((float)(v123 * v106) + (float)(v124 * v110)) + (float)(v125 * v114)) + (float)(v126 * v118);
            v88 = (float)((float)((float)(v127 * v103) + (float)(v128 * v107)) + (float)(v129 * v111)) + (float)(v130 * v115);
            v89 = (float)((float)((float)(v127 * v104) + (float)(v128 * v108)) + (float)(v129 * v112)) + (float)(v130 * v116);
            v90 = (float)((float)((float)(v127 * v105) + (float)(v128 * v109)) + (float)(v129 * v113)) + (float)(v130 * v117);
            v91 = (float)((float)((float)(v127 * v106) + (float)(v128 * v110)) + (float)(v129 * v114)) + (float)(v130 * v118);
            v92 = (float)((float)((float)(v131 * v103) + (float)(v132 * v107)) + (float)(v133 * v111)) + (float)(v134 * v115);
            v93 = (float)((float)((float)(v131 * v104) + (float)(v132 * v108)) + (float)(v133 * v112)) + (float)(v134 * v116);
            v94 = (float)((float)((float)(v131 * v105) + (float)(v132 * v109)) + (float)(v133 * v113)) + (float)(v134 * v117);
            v95 = (float)((float)((float)(v131 * v106) + (float)(v132 * v110)) + (float)(v133 * v114)) + (float)(v134 * v118);
            v96 = (float)((float)((float)(v135 * v103) + (float)(v136 * v107)) + (float)(v137 * v111)) + (float)(v138 * v115);
            v97 = (float)((float)((float)(v135 * v104) + (float)(v136 * v108)) + (float)(v137 * v112)) + (float)(v138 * v116);
            v98 = (float)((float)((float)(v135 * v105) + (float)(v136 * v109)) + (float)(v137 * v113)) + (float)(v138 * v117);
            v99 = (float)((float)((float)(v135 * v106) + (float)(v136 * v110)) + (float)(v137 * v114)) + (float)(v138 * v118);
            (*out)[0] = (float)((float)((float)(v123 * v103) + (float)(v124 * v107)) + (float)(v125 * v111))
                + (float)(v126 * v115);
            (*out)[1] = v85;
            (*out)[2] = v86;
            (*out)[3] = v87;
            (*out)[4] = v88;
            (*out)[5] = v89;
            (*out)[6] = v90;
            (*out)[7] = v91;
            (*out)[8] = v92;
            (*out)[9] = v93;
            (*out)[10] = v94;
            (*out)[11] = v95;
            (*out)[12] = v96;
            (*out)[13] = v97;
            (*out)[14] = v98;
            (*out)[15] = v99;
        }
    }
    else
    {
        ikNormalizedMatrixAssert_func(cache[boneNum]);
        v163 = cache[boneNum][0];
        v162 = v163;
        v146 = *v163;
        v147 = v163[1];
        v148 = v163[2];
        v149 = v163[3];
        v145 = v163 + 4;
        v150 = v163[4];
        v151 = v163[5];
        v152 = v163[6];
        v153 = v163[7];
        v144 = v163 + 8;
        v154 = v163[8];
        v155 = v163[9];
        v156 = v163[10];
        v157 = v163[11];
        v143 = v163 + 12;
        v158 = v163[12];
        v159 = v163[13];
        v160 = v163[14];
        v161 = v163[15];
        (*out)[0] = v146;
        (*out)[1] = v147;
        (*out)[2] = v148;
        (*out)[3] = v149;
        (*out)[4] = v150;
        (*out)[5] = v151;
        (*out)[6] = v152;
        (*out)[7] = v153;
        (*out)[8] = v154;
        (*out)[9] = v155;
        (*out)[10] = v156;
        (*out)[11] = v157;
        (*out)[12] = v158;
        (*out)[13] = v159;
        (*out)[14] = v160;
        (*out)[15] = v161;
    }
}

void __cdecl ikCalcBoneModelMatrix(IKState *ikState, int boneNum, float *inMatArray, float (*out)[4])
{
    int savedregs; // [esp+0h] [ebp+0h] BYREF

    ikCalcBoneModelMatrix_r(ikState, boneNum, inMatArray, out);
}

void __cdecl ikSolve2D(
                const float *BaseJoint,
                const float *ModelTarget,
                float UpperIKc,
                float LowerIKc,
                float UpperIKInvc,
                float LowerIKInvc,
                float *ModelTargetDir,
                float *SinUpper,
                float *CosUpper,
                float *SinLower,
                float *CosLower)
{
    float v11; // [esp+8h] [ebp-1Ch]
    float v12; // [esp+Ch] [ebp-18h]
    float v13; // [esp+10h] [ebp-14h]
    float v14; // [esp+14h] [ebp-10h]
    float tmp_C; // [esp+1Ch] [ebp-8h]

    *ModelTargetDir = *ModelTarget - *BaseJoint;
    ModelTargetDir[1] = ModelTarget[1] - BaseJoint[1];
    ModelTargetDir[2] = ModelTarget[2] - BaseJoint[2];
    tmp_C = sqrtf(
                        (float)((float)(*ModelTargetDir * *ModelTargetDir) + (float)(ModelTargetDir[1] * ModelTargetDir[1]))
                    + (float)(ModelTargetDir[2] * ModelTargetDir[2]));
    *ModelTargetDir = (float)(1.0 / tmp_C) * *ModelTargetDir;
    ModelTargetDir[1] = (float)(1.0 / tmp_C) * ModelTargetDir[1];
    ModelTargetDir[2] = (float)(1.0 / tmp_C) * ModelTargetDir[2];
    *CosUpper = (float)(UpperIKInvc * (float)(1.0 / tmp_C)) + (float)(UpperIKc * tmp_C);
    *CosLower = (float)(LowerIKInvc * (float)(1.0 / tmp_C)) + (float)(LowerIKc * tmp_C);
    v14 = *CosUpper;
    if ( 1.0 <= -1.0
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 730, 0, "%s", "min < max") )
    {
        __debugbreak();
    }
    if ( v14 >= -1.0 )
    {
        if ( v14 <= 1.0 )
            v13 = v14;
        else
            v13 = 1.0f;
    }
    else
    {
        v13 = -1.0f;
    }
    *CosUpper = v13;
    v12 = *CosLower;
    if ( 1.0 <= -1.0
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 730, 0, "%s", "min < max") )
    {
        __debugbreak();
    }
    if ( v12 >= -1.0 )
    {
        if ( v12 <= 1.0 )
            v11 = v12;
        else
            v11 = 1.0f;
    }
    else
    {
        v11 = -1.0f;
    }
    *CosLower = v11;
    *SinUpper = sqrtf(1.0 - (float)(*CosUpper * *CosUpper));
    *SinLower = sqrtf(1.0 - (float)(*CosLower * *CosLower));
}

// local variable allocation has failed, the output may be wrong!
void    ikMap2DTo3D(
                float UpperLimbLength,
                float SinUpper,
                float CosUpper,
                float SinLower,
                float CosLower,
                const float *ModelBaseJoint,
                const float *ModelTargetDir,
                const float *ModelMidJointDir,
                float sinTwist,
                float cosTwist,
                float (*UpperLocalToModel)[4],
                float (*LowerLocalToModel)[4])
{
    float v13; // xmm0_4
    float v14; // [esp-F8h] [ebp-2D8h]
    float v15; // [esp-F4h] [ebp-2D4h]
    float v16; // [esp-F0h] [ebp-2D0h]
    float v17; // [esp-ECh] [ebp-2CCh]
    float v18; // [esp-E8h] [ebp-2C8h]
    float v19; // [esp-E4h] [ebp-2C4h]
    float v20; // [esp-E0h] [ebp-2C0h]
    float v21; // [esp-DCh] [ebp-2BCh]
    float v22; // [esp-D8h] [ebp-2B8h]
    float v23; // [esp-D4h] [ebp-2B4h]
    float v24; // [esp-D0h] [ebp-2B0h]
    float v25; // [esp-CCh] [ebp-2ACh]
    float v26; // [esp-C8h] [ebp-2A8h]
    float v27; // [esp-C4h] [ebp-2A4h]
    float v28; // [esp-58h] [ebp-238h]
    float v29; // [esp-54h] [ebp-234h]
    float v30; // [esp-28h] [ebp-208h]
    float v31; // [esp-24h] [ebp-204h]
    float v32; // [esp+48h] [ebp-198h]
    float v33; // [esp+4Ch] [ebp-194h]
    float v34; // [esp+50h] [ebp-190h]
    float v35; // [esp+54h] [ebp-18Ch]
    float v36; // [esp+58h] [ebp-188h]
    float v37; // [esp+5Ch] [ebp-184h]
    float v38; // [esp+60h] [ebp-180h]
    float v39; // [esp+64h] [ebp-17Ch]
    float v40; // [esp+68h] [ebp-178h]
    float v41; // [esp+6Ch] [ebp-174h]
    float v42; // [esp+70h] [ebp-170h]
    float v43; // [esp+74h] [ebp-16Ch]
    float v44; // [esp+78h] [ebp-168h]
    float v45; // [esp+7Ch] [ebp-164h]
    float v46; // [esp+80h] [ebp-160h]
    float ikToModel[4][4]; // [esp+174h] [ebp-6Ch] BYREF
    float Up[3]; // [esp+1B8h] [ebp-28h] BYREF
    float MidJointAxis[4]; // [esp+1C4h] [ebp-1Ch] BYREF
    //_UNKNOWN *v50; // [esp+1D4h] [ebp-Ch]
    //float UpperLimbLengtha; // [esp+1D8h] [ebp-8h]
    //float CosUppera; // [esp+1E0h] [ebp+0h]
    //
    //v50 = a1;
    //UpperLimbLengtha = CosUppera;
    Vec3Cross(ModelTargetDir, ModelMidJointDir, MidJointAxis);
    if (Vec3Normalize(MidJointAxis) < 0.0099999998)
    {
        MidJointAxis[0] = 0.0f;
        MidJointAxis[1] = 0.0f;
        MidJointAxis[2] = 1.0f;
    }
    Vec3Cross(MidJointAxis, ModelTargetDir, Up);
    ikMat44Set(ikToModel, ModelTargetDir, Up, MidJointAxis, ModelBaseJoint);
    ikNormalizedMatrixAssert_func(ikToModel);
    v32 = (float)((float)((float)(CosUpper * ikToModel[0][1]) + (float)((float)(SinUpper * cosTwist) * ikToModel[1][1]))
        + (float)((float)(SinUpper * sinTwist) * ikToModel[2][1]))
        + (float)(0.0 * ikToModel[3][1]);
    v33 = (float)((float)((float)(CosUpper * ikToModel[0][2]) + (float)((float)(SinUpper * cosTwist) * ikToModel[1][2]))
        + (float)((float)(SinUpper * sinTwist) * ikToModel[2][2]))
        + (float)(0.0 * ikToModel[3][2]);
    v34 = (float)((float)((float)(CosUpper * ikToModel[0][3]) + (float)((float)(SinUpper * cosTwist) * ikToModel[1][3]))
        + (float)((float)(SinUpper * sinTwist) * ikToModel[2][3]))
        + (float)(0.0 * ikToModel[3][3]);
    v35 = (float)((float)((float)((-(SinUpper)) * ikToModel[0][0])
        + (float)((float)(CosUpper * cosTwist) * ikToModel[1][0]))
        + (float)((float)(CosUpper * sinTwist) * ikToModel[2][0]))
        + (float)(0.0 * ikToModel[3][0]);
    v36 = (float)((float)((float)((-(SinUpper)) * ikToModel[0][1])
        + (float)((float)(CosUpper * cosTwist) * ikToModel[1][1]))
        + (float)((float)(CosUpper * sinTwist) * ikToModel[2][1]))
        + (float)(0.0 * ikToModel[3][1]);
    v37 = (float)((float)((float)((-(SinUpper)) * ikToModel[0][2])
        + (float)((float)(CosUpper * cosTwist) * ikToModel[1][2]))
        + (float)((float)(CosUpper * sinTwist) * ikToModel[2][2]))
        + (float)(0.0 * ikToModel[3][2]);
    v38 = (float)((float)((float)((-(SinUpper)) * ikToModel[0][3])
        + (float)((float)(CosUpper * cosTwist) * ikToModel[1][3]))
        + (float)((float)(CosUpper * sinTwist) * ikToModel[2][3]))
        + (float)(0.0 * ikToModel[3][3]);
    v39 = (float)((float)((float)(0.0 * ikToModel[0][0])
        + (float)((-(sinTwist)) * ikToModel[1][0]))
        + (float)(cosTwist * ikToModel[2][0]))
        + (float)(0.0 * ikToModel[3][0]);
    v40 = (float)((float)((float)(0.0 * ikToModel[0][1])
        + (float)((-(sinTwist)) * ikToModel[1][1]))
        + (float)(cosTwist * ikToModel[2][1]))
        + (float)(0.0 * ikToModel[3][1]);
    v41 = (float)((float)((float)(0.0 * ikToModel[0][2])
        + (float)((-(sinTwist)) * ikToModel[1][2]))
        + (float)(cosTwist * ikToModel[2][2]))
        + (float)(0.0 * ikToModel[3][2]);
    v42 = (float)((float)((float)(0.0 * ikToModel[0][3])
        + (float)((-(sinTwist)) * ikToModel[1][3]))
        + (float)(cosTwist * ikToModel[2][3]))
        + (float)(0.0 * ikToModel[3][3]);
    v43 = (float)((float)((float)(0.0 * ikToModel[0][0]) + (float)(0.0 * ikToModel[1][0])) + (float)(0.0 * ikToModel[2][0]))
        + (float)(1.0 * ikToModel[3][0]);
    v44 = (float)((float)((float)(0.0 * ikToModel[0][1]) + (float)(0.0 * ikToModel[1][1])) + (float)(0.0 * ikToModel[2][1]))
        + (float)(1.0 * ikToModel[3][1]);
    v45 = (float)((float)((float)(0.0 * ikToModel[0][2]) + (float)(0.0 * ikToModel[1][2])) + (float)(0.0 * ikToModel[2][2]))
        + (float)(1.0 * ikToModel[3][2]);
    v46 = (float)((float)((float)(0.0 * ikToModel[0][3]) + (float)(0.0 * ikToModel[1][3])) + (float)(0.0 * ikToModel[2][3]))
        + (float)(1.0 * ikToModel[3][3]);
    (*UpperLocalToModel)[0] = (float)((float)((float)(CosUpper * ikToModel[0][0])
        + (float)((float)(SinUpper * cosTwist) * ikToModel[1][0]))
        + (float)((float)(SinUpper * sinTwist) * ikToModel[2][0]))
        + (float)(0.0 * ikToModel[3][0]);
    (*UpperLocalToModel)[1] = v32;
    (*UpperLocalToModel)[2] = v33;
    (*UpperLocalToModel)[3] = v34;
    (*UpperLocalToModel)[4] = v35;
    (*UpperLocalToModel)[5] = v36;
    (*UpperLocalToModel)[6] = v37;
    (*UpperLocalToModel)[7] = v38;
    (*UpperLocalToModel)[8] = v39;
    (*UpperLocalToModel)[9] = v40;
    (*UpperLocalToModel)[10] = v41;
    (*UpperLocalToModel)[11] = v42;
    (*UpperLocalToModel)[12] = v43;
    (*UpperLocalToModel)[13] = v44;
    (*UpperLocalToModel)[14] = v45;
    (*UpperLocalToModel)[15] = v46;
    v28 = (-(SinLower)) * cosTwist;
    v29 = (-(SinLower)) * sinTwist;
    v30 = (float)(UpperLimbLength * SinUpper) * cosTwist;
    v31 = (float)(UpperLimbLength * SinUpper) * sinTwist;
    v14 = (float)((float)((float)(CosLower * ikToModel[0][1]) + (float)(v28 * ikToModel[1][1]))
        + (float)(v29 * ikToModel[2][1]))
        + (float)(0.0 * ikToModel[3][1]);
    v15 = (float)((float)((float)(CosLower * ikToModel[0][2]) + (float)(v28 * ikToModel[1][2]))
        + (float)(v29 * ikToModel[2][2]))
        + (float)(0.0 * ikToModel[3][2]);
    v16 = (float)((float)((float)(CosLower * ikToModel[0][3]) + (float)(v28 * ikToModel[1][3]))
        + (float)(v29 * ikToModel[2][3]))
        + (float)(0.0 * ikToModel[3][3]);
    v17 = (float)((float)((float)(SinLower * ikToModel[0][0]) + (float)((float)(CosLower * cosTwist) * ikToModel[1][0]))
        + (float)((float)(CosLower * sinTwist) * ikToModel[2][0]))
        + (float)(0.0 * ikToModel[3][0]);
    v18 = (float)((float)((float)(SinLower * ikToModel[0][1]) + (float)((float)(CosLower * cosTwist) * ikToModel[1][1]))
        + (float)((float)(CosLower * sinTwist) * ikToModel[2][1]))
        + (float)(0.0 * ikToModel[3][1]);
    v19 = (float)((float)((float)(SinLower * ikToModel[0][2]) + (float)((float)(CosLower * cosTwist) * ikToModel[1][2]))
        + (float)((float)(CosLower * sinTwist) * ikToModel[2][2]))
        + (float)(0.0 * ikToModel[3][2]);
    v20 = (float)((float)((float)(SinLower * ikToModel[0][3]) + (float)((float)(CosLower * cosTwist) * ikToModel[1][3]))
        + (float)((float)(CosLower * sinTwist) * ikToModel[2][3]))
        + (float)(0.0 * ikToModel[3][3]);
    v21 = (float)((float)((float)(0.0 * ikToModel[0][0])
        + (float)((-(sinTwist)) * ikToModel[1][0]))
        + (float)(cosTwist * ikToModel[2][0]))
        + (float)(0.0 * ikToModel[3][0]);
    v22 = (float)((float)((float)(0.0 * ikToModel[0][1])
        + (float)((-(sinTwist)) * ikToModel[1][1]))
        + (float)(cosTwist * ikToModel[2][1]))
        + (float)(0.0 * ikToModel[3][1]);
    v23 = (float)((float)((float)(0.0 * ikToModel[0][2])
        + (float)((-(sinTwist)) * ikToModel[1][2]))
        + (float)(cosTwist * ikToModel[2][2]))
        + (float)(0.0 * ikToModel[3][2]);
    v24 = (float)((float)((float)(0.0 * ikToModel[0][3])
        + (float)((-(sinTwist)) * ikToModel[1][3]))
        + (float)(cosTwist * ikToModel[2][3]))
        + (float)(0.0 * ikToModel[3][3]);
    v25 = (float)((float)((float)((float)(UpperLimbLength * CosUpper) * ikToModel[0][0]) + (float)(v30 * ikToModel[1][0]))
        + (float)(v31 * ikToModel[2][0]))
        + (float)(1.0 * ikToModel[3][0]);
    v26 = (float)((float)((float)((float)(UpperLimbLength * CosUpper) * ikToModel[0][1]) + (float)(v30 * ikToModel[1][1]))
        + (float)(v31 * ikToModel[2][1]))
        + (float)(1.0 * ikToModel[3][1]);
    v27 = (float)((float)((float)((float)(UpperLimbLength * CosUpper) * ikToModel[0][2]) + (float)(v30 * ikToModel[1][2]))
        + (float)(v31 * ikToModel[2][2]))
        + (float)(1.0 * ikToModel[3][2]);
    v13 = (float)((float)((float)((float)(UpperLimbLength * CosUpper) * ikToModel[0][3]) + (float)(v30 * ikToModel[1][3]))
        + (float)(v31 * ikToModel[2][3]))
        + (float)(1.0 * ikToModel[3][3]);
    (*LowerLocalToModel)[0] = (float)((float)((float)(CosLower * ikToModel[0][0]) + (float)(v28 * ikToModel[1][0]))
        + (float)(v29 * ikToModel[2][0]))
        + (float)(0.0 * ikToModel[3][0]);
    (*LowerLocalToModel)[1] = v14;
    (*LowerLocalToModel)[2] = v15;
    (*LowerLocalToModel)[3] = v16;
    (*LowerLocalToModel)[4] = v17;
    (*LowerLocalToModel)[5] = v18;
    (*LowerLocalToModel)[6] = v19;
    (*LowerLocalToModel)[7] = v20;
    (*LowerLocalToModel)[8] = v21;
    (*LowerLocalToModel)[9] = v22;
    (*LowerLocalToModel)[10] = v23;
    (*LowerLocalToModel)[11] = v24;
    (*LowerLocalToModel)[12] = v25;
    (*LowerLocalToModel)[13] = v26;
    (*LowerLocalToModel)[14] = v27;
    (*LowerLocalToModel)[15] = v13;
}

// local variable allocation has failed, the output may be wrong!
void    ikSolveLegJoint(
                IKState *ikState,
                IKJointBones *jointBones,
                IKJointVars *jointVars,
                float *footPos,
                float (*outParentMat)[4],
                float (*outJointMat)[4],
                bool flipHack)
{
    _QWORD basePos[2]; // [esp+BCh] [ebp-230h] BYREF
    float modelMidJointDir[3]; // [esp+CCh] [ebp-220h] BYREF
    float modelTargetDir[3]; // [esp+D8h] [ebp-214h] BYREF
    float sinUpper; // [esp+E4h] [ebp-208h] BYREF
    float cosUpper; // [esp+E8h] [ebp-204h] BYREF
    float sinLower; // [esp+ECh] [ebp-200h] BYREF
    float cosLower; // [esp+F0h] [ebp-1FCh] BYREF
    float *v15; // [esp+F4h] [ebp-1F8h]
    float v16; // [esp+F8h] [ebp-1F4h]
    float footDist; // [esp+FCh] [ebp-1F0h]
    float footVec[4]; // [esp+100h] [ebp-1ECh] BYREF
    float childMat[4][4]; // [esp+110h] [ebp-1DCh] BYREF
    float *v20; // [esp+150h] [ebp-19Ch]
    float *v21; // [esp+154h] [ebp-198h]
    float *v22; // [esp+158h] [ebp-194h]
    float *v23; // [esp+15Ch] [ebp-190h]
    float jointMat[4][4]; // [esp+160h] [ebp-18Ch] BYREF
    float v25; // [esp+1A8h] [ebp-144h]
    float v26; // [esp+1ACh] [ebp-140h]
    float v27; // [esp+1B0h] [ebp-13Ch]
    float v28; // [esp+1B4h] [ebp-138h]
    float v29; // [esp+1B8h] [ebp-134h]
    float v30; // [esp+1BCh] [ebp-130h]
    float v31; // [esp+1C0h] [ebp-12Ch]
    float v32; // [esp+1C4h] [ebp-128h]
    float v33; // [esp+1C8h] [ebp-124h]
    float v34; // [esp+1CCh] [ebp-120h]
    float v35; // [esp+1D0h] [ebp-11Ch]
    float v36; // [esp+1D4h] [ebp-118h]
    float v37; // [esp+1D8h] [ebp-114h]
    float v38; // [esp+1DCh] [ebp-110h]
    float v39; // [esp+1E0h] [ebp-10Ch]
    float v40; // [esp+1E4h] [ebp-108h]
    float *v41; // [esp+1ECh] [ebp-100h]
    float *v42; // [esp+1F0h] [ebp-FCh]
    float *v43; // [esp+1F4h] [ebp-F8h]
    float v44; // [esp+1F8h] [ebp-F4h]
    float v45; // [esp+1FCh] [ebp-F0h]
    float v46; // [esp+200h] [ebp-ECh]
    float v47; // [esp+204h] [ebp-E8h]
    float v48; // [esp+208h] [ebp-E4h]
    float v49; // [esp+20Ch] [ebp-E0h]
    float v50; // [esp+210h] [ebp-DCh]
    float v51; // [esp+214h] [ebp-D8h]
    float v52; // [esp+218h] [ebp-D4h]
    float v53; // [esp+21Ch] [ebp-D0h]
    float v54; // [esp+220h] [ebp-CCh]
    float v55; // [esp+224h] [ebp-C8h]
    float v56; // [esp+228h] [ebp-C4h]
    float v57; // [esp+22Ch] [ebp-C0h]
    float v58; // [esp+230h] [ebp-BCh]
    float v59; // [esp+234h] [ebp-B8h]
    float *v60; // [esp+238h] [ebp-B4h]
    float *v61; // [esp+23Ch] [ebp-B0h]
    float *v62; // [esp+240h] [ebp-ACh]
    float *v63; // [esp+244h] [ebp-A8h]
    __int64 v64; // [esp+248h] [ebp-A4h]
    __int64 v65; // [esp+250h] [ebp-9Ch]
    __int64 v66; // [esp+258h] [ebp-94h]
    __int64 v67; // [esp+260h] [ebp-8Ch]
    __int64 v68; // [esp+268h] [ebp-84h]
    __int64 v69; // [esp+270h] [ebp-7Ch]
    __int64 v70; // [esp+278h] [ebp-74h]
    __int64 v71; // [esp+280h] [ebp-6Ch]
    float *v72; // [esp+288h] [ebp-64h]
    float *v73; // [esp+28Ch] [ebp-60h]
    float parentMat[4][4]; // [esp+290h] [ebp-5Ch] BYREF
    //int v75; // [esp+2D8h] [ebp-14h]
    //_UNKNOWN *v76[2]; // [esp+2E0h] [ebp-Ch] BYREF
    //IKJointVars *jointVarsa; // [esp+2ECh] [ebp+0h]
    //
    //v76[0] = a1;
    //v76[1] = jointVarsa;
    //HIBYTE(v75) = ;
    if (ikState->ikBoneToObjBone[jointBones->childBone] != 161)
    {
        ikCalcBoneModelMatrix(ikState, jointBones->parentBone, (*ikState->matArrayPostIK)[0], parentMat);
        v73 = ikState->matArrayPostIK[jointBones->jointBone][0];
        v72 = parentMat[0];
        v64 = *(_QWORD *)&parentMat[0][0];
        v65 = *(_QWORD *)&parentMat[0][2];
        v63 = parentMat[1];
        v66 = *(_QWORD *)&parentMat[1][0];
        v67 = *(_QWORD *)&parentMat[1][2];
        v62 = parentMat[2];
        v68 = *(_QWORD *)&parentMat[2][0];
        v69 = *(_QWORD *)&parentMat[2][2];
        v61 = parentMat[3];
        v70 = *(_QWORD *)&parentMat[3][0];
        v71 = *(_QWORD *)&parentMat[3][2];
        v60 = v73;
        v44 = *v73;
        v45 = v73[1];
        v46 = v73[2];
        v47 = v73[3];
        v43 = v73 + 4;
        v48 = v73[4];
        v49 = v73[5];
        v50 = v73[6];
        v51 = v73[7];
        v42 = v73 + 8;
        v52 = v73[8];
        v53 = v73[9];
        v54 = v73[10];
        v55 = v73[11];
        v41 = v73 + 12;
        v56 = v73[12];
        v57 = v73[13];
        v58 = v73[14];
        v59 = v73[15];
        v25 = (float)((float)((float)(parentMat[0][0] * v44) + (float)(parentMat[0][1] * v48))
            + (float)(parentMat[0][2] * v52))
            + (float)(parentMat[0][3] * v56);
        v26 = (float)((float)((float)(parentMat[0][0] * v45) + (float)(parentMat[0][1] * v49))
            + (float)(parentMat[0][2] * v53))
            + (float)(parentMat[0][3] * v57);
        v27 = (float)((float)((float)(parentMat[0][0] * v46) + (float)(parentMat[0][1] * v50))
            + (float)(parentMat[0][2] * v54))
            + (float)(parentMat[0][3] * v58);
        v28 = (float)((float)((float)(parentMat[0][0] * v47) + (float)(parentMat[0][1] * v51))
            + (float)(parentMat[0][2] * v55))
            + (float)(parentMat[0][3] * v59);
        v29 = (float)((float)((float)(parentMat[1][0] * v44) + (float)(parentMat[1][1] * v48))
            + (float)(parentMat[1][2] * v52))
            + (float)(parentMat[1][3] * v56);
        v30 = (float)((float)((float)(parentMat[1][0] * v45) + (float)(parentMat[1][1] * v49))
            + (float)(parentMat[1][2] * v53))
            + (float)(parentMat[1][3] * v57);
        v31 = (float)((float)((float)(parentMat[1][0] * v46) + (float)(parentMat[1][1] * v50))
            + (float)(parentMat[1][2] * v54))
            + (float)(parentMat[1][3] * v58);
        v32 = (float)((float)((float)(parentMat[1][0] * v47) + (float)(parentMat[1][1] * v51))
            + (float)(parentMat[1][2] * v55))
            + (float)(parentMat[1][3] * v59);
        v33 = (float)((float)((float)(parentMat[2][0] * v44) + (float)(parentMat[2][1] * v48))
            + (float)(parentMat[2][2] * v52))
            + (float)(parentMat[2][3] * v56);
        v34 = (float)((float)((float)(parentMat[2][0] * v45) + (float)(parentMat[2][1] * v49))
            + (float)(parentMat[2][2] * v53))
            + (float)(parentMat[2][3] * v57);
        v35 = (float)((float)((float)(parentMat[2][0] * v46) + (float)(parentMat[2][1] * v50))
            + (float)(parentMat[2][2] * v54))
            + (float)(parentMat[2][3] * v58);
        v36 = (float)((float)((float)(parentMat[2][0] * v47) + (float)(parentMat[2][1] * v51))
            + (float)(parentMat[2][2] * v55))
            + (float)(parentMat[2][3] * v59);
        v37 = (float)((float)((float)(parentMat[3][0] * v44) + (float)(parentMat[3][1] * v48))
            + (float)(parentMat[3][2] * v52))
            + (float)(parentMat[3][3] * v56);
        v38 = (float)((float)((float)(parentMat[3][0] * v45) + (float)(parentMat[3][1] * v49))
            + (float)(parentMat[3][2] * v53))
            + (float)(parentMat[3][3] * v57);
        v39 = (float)((float)((float)(parentMat[3][0] * v46) + (float)(parentMat[3][1] * v50))
            + (float)(parentMat[3][2] * v54))
            + (float)(parentMat[3][3] * v58);
        v40 = (float)((float)((float)(parentMat[3][0] * v47) + (float)(parentMat[3][1] * v51))
            + (float)(parentMat[3][2] * v55))
            + (float)(parentMat[3][3] * v59);
        v23 = jointMat[0];
        jointMat[0][0] = v25;
        jointMat[0][1] = v26;
        jointMat[0][2] = v27;
        jointMat[0][3] = v28;
        v22 = jointMat[1];
        jointMat[1][0] = v29;
        jointMat[1][1] = v30;
        jointMat[1][2] = v31;
        jointMat[1][3] = v32;
        v21 = jointMat[2];
        jointMat[2][0] = v33;
        jointMat[2][1] = v34;
        jointMat[2][2] = v35;
        jointMat[2][3] = v36;
        v20 = jointMat[3];
        jointMat[3][0] = v37;
        jointMat[3][1] = v38;
        jointMat[3][2] = v39;
        jointMat[3][3] = v40;
        ikCalcBoneModelMatrix(ikState, jointBones->childBone, (*ikState->matArrayPreIK)[0], childMat);
        if (flipHack)
        {
            IK_FlipHack(parentMat);
            IK_FlipHack(jointMat);
            IK_FlipHack(childMat);
        }
        //LODWORD(footVec[3]) = parentMat[3];
        footVec[0] = *footPos - parentMat[3][0];
        footVec[1] = footPos[1] - parentMat[3][1];
        footVec[2] = footPos[2] - parentMat[3][2];
        footDist = Vec3Normalize(footVec);
        if (footDist > (float)((float)(jointVars->UpperLength + jointVars->LowerLength) - fMaxFootReachSubtract))
        {
            v16 = (float)(jointVars->UpperLength + jointVars->LowerLength) - fMaxFootReachSubtract;
            v15 = parentMat[3];
            *footPos = (float)(v16 * footVec[0]) + parentMat[3][0];
            footPos[1] = (float)(v16 * footVec[1]) + v15[1];
            footPos[2] = (float)(v16 * footVec[2]) + v15[2];
        }
        ikSolve2D(
            parentMat[3],
            footPos,
            jointVars->UpperIKc,
            jointVars->LowerIKc,
            jointVars->UpperIKInvc,
            jointVars->LowerIKInvc,
            modelTargetDir,
            &sinUpper,
            &cosUpper,
            &sinLower,
            &cosLower);
        Vec3Cross(parentMat[2], modelTargetDir, modelMidJointDir);
        basePos[0] = *(_QWORD *)&parentMat[3][0];
        basePos[1] = __PAIR64__((float (*)[4])parentMat[3], LODWORD(parentMat[3][2]));
        ikMap2DTo3D(
            jointVars->UpperLength,
            sinUpper,
            cosUpper,
            sinLower,
            cosLower,
            (const float *)basePos,
            modelTargetDir,
            modelMidJointDir,
            0.0,
            1.0,
            outParentMat,
            outJointMat);
        if (((LODWORD((*outParentMat)[0]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[1]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[2]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[3]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                925,
                0,
                "%s",
                "!IS_NAN((outParentMat[0])[0]) && !IS_NAN((outParentMat[0])[1]) && !IS_NAN((outParentMat[0])[2]) && !IS_NAN(("
                "outParentMat[0])[3])"))
        {
            __debugbreak();
        }
        if (((LODWORD((*outParentMat)[4]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[5]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[6]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[7]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                926,
                0,
                "%s",
                "!IS_NAN((outParentMat[1])[0]) && !IS_NAN((outParentMat[1])[1]) && !IS_NAN((outParentMat[1])[2]) && !IS_NAN(("
                "outParentMat[1])[3])"))
        {
            __debugbreak();
        }
        if (((LODWORD((*outParentMat)[8]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[9]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[10]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[11]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                927,
                0,
                "%s",
                "!IS_NAN((outParentMat[2])[0]) && !IS_NAN((outParentMat[2])[1]) && !IS_NAN((outParentMat[2])[2]) && !IS_NAN(("
                "outParentMat[2])[3])"))
        {
            __debugbreak();
        }
        if (((LODWORD((*outParentMat)[12]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[13]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[14]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[15]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                928,
                0,
                "%s",
                "!IS_NAN((outParentMat[3])[0]) && !IS_NAN((outParentMat[3])[1]) && !IS_NAN((outParentMat[3])[2]) && !IS_NAN(("
                "outParentMat[3])[3])"))
        {
            __debugbreak();
        }
        if (((LODWORD((*outJointMat)[0]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[1]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[2]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[3]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                930,
                0,
                "%s",
                "!IS_NAN((outJointMat[0])[0]) && !IS_NAN((outJointMat[0])[1]) && !IS_NAN((outJointMat[0])[2]) && !IS_NAN((out"
                "JointMat[0])[3])"))
        {
            __debugbreak();
        }
        if (((LODWORD((*outJointMat)[4]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[5]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[6]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[7]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                931,
                0,
                "%s",
                "!IS_NAN((outJointMat[1])[0]) && !IS_NAN((outJointMat[1])[1]) && !IS_NAN((outJointMat[1])[2]) && !IS_NAN((out"
                "JointMat[1])[3])"))
        {
            __debugbreak();
        }
        if (((LODWORD((*outJointMat)[8]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[9]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[10]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[11]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                932,
                0,
                "%s",
                "!IS_NAN((outJointMat[2])[0]) && !IS_NAN((outJointMat[2])[1]) && !IS_NAN((outJointMat[2])[2]) && !IS_NAN((out"
                "JointMat[2])[3])"))
        {
            __debugbreak();
        }
        if (((LODWORD((*outJointMat)[12]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[13]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[14]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[15]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                933,
                0,
                "%s",
                "!IS_NAN((outJointMat[3])[0]) && !IS_NAN((outJointMat[3])[1]) && !IS_NAN((outJointMat[3])[2]) && !IS_NAN((out"
                "JointMat[3])[3])"))
        {
            __debugbreak();
        }
        ikNormalizedMatrixAssert_func(outParentMat);
        ikNormalizedMatrixAssert_func(outJointMat);
        if (flipHack)
        {
            IK_FlipHack(outParentMat);
            IK_FlipHack(outJointMat);
        }
    }
}

// local variable allocation has failed, the output may be wrong!
void    ikSolveArmJoint(
                IKState *ikState,
                IKJointBones *jointBones,
                IKJointVars *jointVars,
                float *handPos,
                float (*outParentMat)[4],
                float (*outJointMat)[4],
                bool flipHack)
{
    _QWORD basePos[2]; // [esp+B4h] [ebp-238h] BYREF
    float modelMidJointDir[3]; // [esp+C4h] [ebp-228h] BYREF
    float cosTwist; // [esp+D0h] [ebp-21Ch]
    float sinTwist; // [esp+D4h] [ebp-218h]
    float modelTargetDir[3]; // [esp+D8h] [ebp-214h] BYREF
    float sinUpper; // [esp+E4h] [ebp-208h] BYREF
    float cosUpper; // [esp+E8h] [ebp-204h] BYREF
    float sinLower; // [esp+ECh] [ebp-200h] BYREF
    float cosLower; // [esp+F0h] [ebp-1FCh] BYREF
    float *v17; // [esp+F4h] [ebp-1F8h]
    float v18; // [esp+F8h] [ebp-1F4h]
    float handDist; // [esp+FCh] [ebp-1F0h]
    float handVec[4]; // [esp+100h] [ebp-1ECh] BYREF
    float childMat[4][4]; // [esp+110h] [ebp-1DCh] BYREF
    float *v22; // [esp+150h] [ebp-19Ch]
    float *v23; // [esp+154h] [ebp-198h]
    float *v24; // [esp+158h] [ebp-194h]
    float *v25; // [esp+15Ch] [ebp-190h]
    float jointMat[4][4]; // [esp+160h] [ebp-18Ch] BYREF
    float v27; // [esp+1A8h] [ebp-144h]
    float v28; // [esp+1ACh] [ebp-140h]
    float v29; // [esp+1B0h] [ebp-13Ch]
    float v30; // [esp+1B4h] [ebp-138h]
    float v31; // [esp+1B8h] [ebp-134h]
    float v32; // [esp+1BCh] [ebp-130h]
    float v33; // [esp+1C0h] [ebp-12Ch]
    float v34; // [esp+1C4h] [ebp-128h]
    float v35; // [esp+1C8h] [ebp-124h]
    float v36; // [esp+1CCh] [ebp-120h]
    float v37; // [esp+1D0h] [ebp-11Ch]
    float v38; // [esp+1D4h] [ebp-118h]
    float v39; // [esp+1D8h] [ebp-114h]
    float v40; // [esp+1DCh] [ebp-110h]
    float v41; // [esp+1E0h] [ebp-10Ch]
    float v42; // [esp+1E4h] [ebp-108h]
    float *v43; // [esp+1ECh] [ebp-100h]
    float *v44; // [esp+1F0h] [ebp-FCh]
    float *v45; // [esp+1F4h] [ebp-F8h]
    float v46; // [esp+1F8h] [ebp-F4h]
    float v47; // [esp+1FCh] [ebp-F0h]
    float v48; // [esp+200h] [ebp-ECh]
    float v49; // [esp+204h] [ebp-E8h]
    float v50; // [esp+208h] [ebp-E4h]
    float v51; // [esp+20Ch] [ebp-E0h]
    float v52; // [esp+210h] [ebp-DCh]
    float v53; // [esp+214h] [ebp-D8h]
    float v54; // [esp+218h] [ebp-D4h]
    float v55; // [esp+21Ch] [ebp-D0h]
    float v56; // [esp+220h] [ebp-CCh]
    float v57; // [esp+224h] [ebp-C8h]
    float v58; // [esp+228h] [ebp-C4h]
    float v59; // [esp+22Ch] [ebp-C0h]
    float v60; // [esp+230h] [ebp-BCh]
    float v61; // [esp+234h] [ebp-B8h]
    float *v62; // [esp+238h] [ebp-B4h]
    float *v63; // [esp+23Ch] [ebp-B0h]
    float *v64; // [esp+240h] [ebp-ACh]
    float *v65; // [esp+244h] [ebp-A8h]
    __int64 v66; // [esp+248h] [ebp-A4h]
    __int64 v67; // [esp+250h] [ebp-9Ch]
    __int64 v68; // [esp+258h] [ebp-94h]
    __int64 v69; // [esp+260h] [ebp-8Ch]
    __int64 v70; // [esp+268h] [ebp-84h]
    __int64 v71; // [esp+270h] [ebp-7Ch]
    __int64 v72; // [esp+278h] [ebp-74h]
    __int64 v73; // [esp+280h] [ebp-6Ch]
    float *v74; // [esp+288h] [ebp-64h]
    float *v75; // [esp+28Ch] [ebp-60h]
    float parentMat[4][4]; // [esp+290h] [ebp-5Ch] BYREF
    //int v77; // [esp+2D8h] [ebp-14h]
    //_UNKNOWN *v78[2]; // [esp+2E0h] [ebp-Ch] BYREF
    //IKJointVars *jointVarsa; // [esp+2ECh] [ebp+0h]
    //
    //v78[0] = a1;
    //v78[1] = jointVarsa;
    //HIBYTE(v77) = ;
    if (ikState->ikBoneToObjBone[jointBones->childBone] != 161)
    {
        ikCalcBoneModelMatrix(ikState, jointBones->parentBone, (*ikState->matArrayPostIK)[0], parentMat);
        v75 = ikState->matArrayPostIK[jointBones->jointBone][0];
        v74 = parentMat[0];
        v66 = *(_QWORD *)&parentMat[0][0];
        v67 = *(_QWORD *)&parentMat[0][2];
        v65 = parentMat[1];
        v68 = *(_QWORD *)&parentMat[1][0];
        v69 = *(_QWORD *)&parentMat[1][2];
        v64 = parentMat[2];
        v70 = *(_QWORD *)&parentMat[2][0];
        v71 = *(_QWORD *)&parentMat[2][2];
        v63 = parentMat[3];
        v72 = *(_QWORD *)&parentMat[3][0];
        v73 = *(_QWORD *)&parentMat[3][2];
        v62 = v75;
        v46 = *v75;
        v47 = v75[1];
        v48 = v75[2];
        v49 = v75[3];
        v45 = v75 + 4;
        v50 = v75[4];
        v51 = v75[5];
        v52 = v75[6];
        v53 = v75[7];
        v44 = v75 + 8;
        v54 = v75[8];
        v55 = v75[9];
        v56 = v75[10];
        v57 = v75[11];
        v43 = v75 + 12;
        v58 = v75[12];
        v59 = v75[13];
        v60 = v75[14];
        v61 = v75[15];
        v27 = (float)((float)((float)(parentMat[0][0] * v46) + (float)(parentMat[0][1] * v50))
            + (float)(parentMat[0][2] * v54))
            + (float)(parentMat[0][3] * v58);
        v28 = (float)((float)((float)(parentMat[0][0] * v47) + (float)(parentMat[0][1] * v51))
            + (float)(parentMat[0][2] * v55))
            + (float)(parentMat[0][3] * v59);
        v29 = (float)((float)((float)(parentMat[0][0] * v48) + (float)(parentMat[0][1] * v52))
            + (float)(parentMat[0][2] * v56))
            + (float)(parentMat[0][3] * v60);
        v30 = (float)((float)((float)(parentMat[0][0] * v49) + (float)(parentMat[0][1] * v53))
            + (float)(parentMat[0][2] * v57))
            + (float)(parentMat[0][3] * v61);
        v31 = (float)((float)((float)(parentMat[1][0] * v46) + (float)(parentMat[1][1] * v50))
            + (float)(parentMat[1][2] * v54))
            + (float)(parentMat[1][3] * v58);
        v32 = (float)((float)((float)(parentMat[1][0] * v47) + (float)(parentMat[1][1] * v51))
            + (float)(parentMat[1][2] * v55))
            + (float)(parentMat[1][3] * v59);
        v33 = (float)((float)((float)(parentMat[1][0] * v48) + (float)(parentMat[1][1] * v52))
            + (float)(parentMat[1][2] * v56))
            + (float)(parentMat[1][3] * v60);
        v34 = (float)((float)((float)(parentMat[1][0] * v49) + (float)(parentMat[1][1] * v53))
            + (float)(parentMat[1][2] * v57))
            + (float)(parentMat[1][3] * v61);
        v35 = (float)((float)((float)(parentMat[2][0] * v46) + (float)(parentMat[2][1] * v50))
            + (float)(parentMat[2][2] * v54))
            + (float)(parentMat[2][3] * v58);
        v36 = (float)((float)((float)(parentMat[2][0] * v47) + (float)(parentMat[2][1] * v51))
            + (float)(parentMat[2][2] * v55))
            + (float)(parentMat[2][3] * v59);
        v37 = (float)((float)((float)(parentMat[2][0] * v48) + (float)(parentMat[2][1] * v52))
            + (float)(parentMat[2][2] * v56))
            + (float)(parentMat[2][3] * v60);
        v38 = (float)((float)((float)(parentMat[2][0] * v49) + (float)(parentMat[2][1] * v53))
            + (float)(parentMat[2][2] * v57))
            + (float)(parentMat[2][3] * v61);
        v39 = (float)((float)((float)(parentMat[3][0] * v46) + (float)(parentMat[3][1] * v50))
            + (float)(parentMat[3][2] * v54))
            + (float)(parentMat[3][3] * v58);
        v40 = (float)((float)((float)(parentMat[3][0] * v47) + (float)(parentMat[3][1] * v51))
            + (float)(parentMat[3][2] * v55))
            + (float)(parentMat[3][3] * v59);
        v41 = (float)((float)((float)(parentMat[3][0] * v48) + (float)(parentMat[3][1] * v52))
            + (float)(parentMat[3][2] * v56))
            + (float)(parentMat[3][3] * v60);
        v42 = (float)((float)((float)(parentMat[3][0] * v49) + (float)(parentMat[3][1] * v53))
            + (float)(parentMat[3][2] * v57))
            + (float)(parentMat[3][3] * v61);
        v25 = jointMat[0];
        jointMat[0][0] = v27;
        jointMat[0][1] = v28;
        jointMat[0][2] = v29;
        jointMat[0][3] = v30;
        v24 = jointMat[1];
        jointMat[1][0] = v31;
        jointMat[1][1] = v32;
        jointMat[1][2] = v33;
        jointMat[1][3] = v34;
        v23 = jointMat[2];
        jointMat[2][0] = v35;
        jointMat[2][1] = v36;
        jointMat[2][2] = v37;
        jointMat[2][3] = v38;
        v22 = jointMat[3];
        jointMat[3][0] = v39;
        jointMat[3][1] = v40;
        jointMat[3][2] = v41;
        jointMat[3][3] = v42;
        ikCalcBoneModelMatrix(ikState, jointBones->childBone, (*ikState->matArrayPreIK)[0], childMat);
        if (flipHack)
        {
            IK_FlipHack(parentMat);
            IK_FlipHack(jointMat);
            IK_FlipHack(childMat);
        }
        //LODWORD(handVec[3]) = parentMat[3];
        handVec[0] = *handPos - parentMat[3][0];
        handVec[1] = handPos[1] - parentMat[3][1];
        handVec[2] = handPos[2] - parentMat[3][2];
        handDist = Vec3Normalize(handVec);
        if (handDist > (float)((float)(jointVars->UpperLength + jointVars->LowerLength) - fMaxHandReachSubtract))
        {
            v18 = (float)(jointVars->UpperLength + jointVars->LowerLength) - fMaxHandReachSubtract;
            v17 = parentMat[3];
            *handPos = (float)(v18 * handVec[0]) + parentMat[3][0];
            handPos[1] = (float)(v18 * handVec[1]) + v17[1];
            handPos[2] = (float)(v18 * handVec[2]) + v17[2];
        }
        ikSolve2D(
            parentMat[3],
            handPos,
            jointVars->UpperIKc,
            jointVars->LowerIKc,
            jointVars->UpperIKInvc,
            jointVars->LowerIKInvc,
            modelTargetDir,
            &sinUpper,
            &cosUpper,
            &sinLower,
            &cosLower);
        sinTwist = 0.0f;
        cosTwist = 1.0f;
        Vec3Cross(parentMat[2], modelTargetDir, modelMidJointDir);
        basePos[0] = *(_QWORD *)&parentMat[3][0];
        basePos[1] = __PAIR64__((float (*)[4])parentMat[3], LODWORD(parentMat[3][2]));
        ikMap2DTo3D(
            jointVars->UpperLength,
            sinUpper,
            cosUpper,
            sinLower,
            cosLower,
            (const float *)basePos,
            modelTargetDir,
            modelMidJointDir,
            sinTwist,
            cosTwist,
            outParentMat,
            outJointMat);
        if (((LODWORD((*outParentMat)[0]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[1]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[2]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[3]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                995,
                0,
                "%s",
                "!IS_NAN((outParentMat[0])[0]) && !IS_NAN((outParentMat[0])[1]) && !IS_NAN((outParentMat[0])[2]) && !IS_NAN(("
                "outParentMat[0])[3])"))
        {
            __debugbreak();
        }
        if (((LODWORD((*outParentMat)[4]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[5]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[6]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[7]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                996,
                0,
                "%s",
                "!IS_NAN((outParentMat[1])[0]) && !IS_NAN((outParentMat[1])[1]) && !IS_NAN((outParentMat[1])[2]) && !IS_NAN(("
                "outParentMat[1])[3])"))
        {
            __debugbreak();
        }
        if (((LODWORD((*outParentMat)[8]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[9]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[10]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[11]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                997,
                0,
                "%s",
                "!IS_NAN((outParentMat[2])[0]) && !IS_NAN((outParentMat[2])[1]) && !IS_NAN((outParentMat[2])[2]) && !IS_NAN(("
                "outParentMat[2])[3])"))
        {
            __debugbreak();
        }
        if (((LODWORD((*outParentMat)[12]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[13]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[14]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outParentMat)[15]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                998,
                0,
                "%s",
                "!IS_NAN((outParentMat[3])[0]) && !IS_NAN((outParentMat[3])[1]) && !IS_NAN((outParentMat[3])[2]) && !IS_NAN(("
                "outParentMat[3])[3])"))
        {
            __debugbreak();
        }
        if (((LODWORD((*outJointMat)[0]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[1]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[2]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[3]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                1000,
                0,
                "%s",
                "!IS_NAN((outJointMat[0])[0]) && !IS_NAN((outJointMat[0])[1]) && !IS_NAN((outJointMat[0])[2]) && !IS_NAN((out"
                "JointMat[0])[3])"))
        {
            __debugbreak();
        }
        if (((LODWORD((*outJointMat)[4]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[5]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[6]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[7]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                1001,
                0,
                "%s",
                "!IS_NAN((outJointMat[1])[0]) && !IS_NAN((outJointMat[1])[1]) && !IS_NAN((outJointMat[1])[2]) && !IS_NAN((out"
                "JointMat[1])[3])"))
        {
            __debugbreak();
        }
        if (((LODWORD((*outJointMat)[8]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[9]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[10]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[11]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                1002,
                0,
                "%s",
                "!IS_NAN((outJointMat[2])[0]) && !IS_NAN((outJointMat[2])[1]) && !IS_NAN((outJointMat[2])[2]) && !IS_NAN((out"
                "JointMat[2])[3])"))
        {
            __debugbreak();
        }
        if (((LODWORD((*outJointMat)[12]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[13]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[14]) & 0x7F800000) == 0x7F800000
            || (LODWORD((*outJointMat)[15]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                1003,
                0,
                "%s",
                "!IS_NAN((outJointMat[3])[0]) && !IS_NAN((outJointMat[3])[1]) && !IS_NAN((outJointMat[3])[2]) && !IS_NAN((out"
                "JointMat[3])[3])"))
        {
            __debugbreak();
        }
        ikNormalizedMatrixAssert_func(outParentMat);
        ikNormalizedMatrixAssert_func(outJointMat);
        if (flipHack)
        {
            IK_FlipHack(outParentMat);
            IK_FlipHack(outJointMat);
        }
    }
}

void __cdecl IK_FlipHack(float (*mat)[4])
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 2; ++i )
    {
        ((*mat)[4 * i + 0]) = -((*mat)[4 * i + 0]);
        ((*mat)[4 * i + 1]) = -((*mat)[4 * i + 1]);
        ((*mat)[4 * i + 2]) = -((*mat)[4 * i + 2]);
    }
}

void __cdecl ikNormalizedMatrixAssert_func(float (*mat)[4])
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    float v4; // xmm2_4
    const char *v5; // eax
    int j; // [esp+5Ch] [ebp-Ch]
    int ja; // [esp+5Ch] [ebp-Ch]
    float len; // [esp+60h] [ebp-8h]

    if ( ikSystem.dvars[14] && ikSystem.dvars[14]->current.enabled )
    {
        if ( ((LODWORD((*mat)[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD((*mat)[1]) & 0x7F800000) == 0x7F800000
             || (LODWORD((*mat)[2]) & 0x7F800000) == 0x7F800000
             || (LODWORD((*mat)[3]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                        1049,
                        0,
                        "%s",
                        "!IS_NAN((mat[0])[0]) && !IS_NAN((mat[0])[1]) && !IS_NAN((mat[0])[2]) && !IS_NAN((mat[0])[3])") )
        {
            __debugbreak();
        }
        if ( ((LODWORD((*mat)[4]) & 0x7F800000) == 0x7F800000
             || (LODWORD((*mat)[5]) & 0x7F800000) == 0x7F800000
             || (LODWORD((*mat)[6]) & 0x7F800000) == 0x7F800000
             || (LODWORD((*mat)[7]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                        1050,
                        0,
                        "%s",
                        "!IS_NAN((mat[1])[0]) && !IS_NAN((mat[1])[1]) && !IS_NAN((mat[1])[2]) && !IS_NAN((mat[1])[3])") )
        {
            __debugbreak();
        }
        if ( ((LODWORD((*mat)[8]) & 0x7F800000) == 0x7F800000
             || (LODWORD((*mat)[9]) & 0x7F800000) == 0x7F800000
             || (LODWORD((*mat)[10]) & 0x7F800000) == 0x7F800000
             || (LODWORD((*mat)[11]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                        1051,
                        0,
                        "%s",
                        "!IS_NAN((mat[2])[0]) && !IS_NAN((mat[2])[1]) && !IS_NAN((mat[2])[2]) && !IS_NAN((mat[2])[3])") )
        {
            __debugbreak();
        }
        if ( ((LODWORD((*mat)[12]) & 0x7F800000) == 0x7F800000
             || (LODWORD((*mat)[13]) & 0x7F800000) == 0x7F800000
             || (LODWORD((*mat)[14]) & 0x7F800000) == 0x7F800000
             || (LODWORD((*mat)[15]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                        1052,
                        0,
                        "%s",
                        "!IS_NAN((mat[3])[0]) && !IS_NAN((mat[3])[1]) && !IS_NAN((mat[3])[2]) && !IS_NAN((mat[3])[3])") )
        {
            __debugbreak();
        }
        for ( j = 0; j < 3; ++j )
        {
            if ( (*mat)[4 * j + 3] != 0.0 )
            {
                v1 = va("mat[%i][3] == %f", j, (*mat)[4 * j + 3]);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                                1057,
                                0,
                                "%s\n\t%s",
                                "mat[j][3] == 0.0f",
                                v1) )
                    __debugbreak();
            }
            len = Vec3Length(&(*mat)[4 * j]);
            if ( fabs(1.0 - len) >= 0.0020000001 )
            {
                v2 = va("len == %f", len);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                                1059,
                                0,
                                "%s\n\t%s",
                                "fabs( 1.0f - len ) < IK_SANITYCHECKACCURACY",
                                v2) )
                    __debugbreak();
            }
        }
        if ( fabs(1.0 - (*mat)[15]) >= 0.0020000001 )
        {
            v3 = va("mat[3][3] == %f", (*mat)[15]);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                            1062,
                            0,
                            "%s\n\t%s",
                            "fabs( 1.0f - mat[3][3] ) < IK_SANITYCHECKACCURACY",
                            v3) )
                __debugbreak();
        }
        for ( ja = 0; ja < 3; ++ja )
        {
            if ( ja >= 2 )
                v4 = (float)((float)((*mat)[4 * ja] * (*mat)[0]) + (float)((*mat)[4 * ja + 1] * (*mat)[1]))
                     + (float)((*mat)[4 * ja + 2] * (*mat)[2]);
            else
                v4 = (float)((float)((*mat)[4 * ja] * (*mat)[4 * ja + 4]) + (float)((*mat)[4 * ja + 1] * (*mat)[4 * ja + 5]))
                     + (float)((*mat)[4 * ja + 2] * (*mat)[4 * ja + 6]);
            if ( fabs(v4) >= 0.0020000001 )
            {
                v5 = va("dot == %f", v4);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.cpp",
                                1076,
                                0,
                                "%s\n\t%s",
                                "fabs( dot ) < IK_SANITYCHECKACCURACY",
                                v5) )
                    __debugbreak();
            }
        }
    }
}

// local variable allocation has failed, the output may be wrong!
void    ikRotateBone(IKState *ikState, IKBoneNames boneName, float *rot, bool local)
{
    // this fatass float array was created to cleanup a bunch of m128 shitballs. 
    float v5[64]; // [esp+34h] [ebp-24Ch] BYREF
    float workMat2[4][4]; // [esp+134h] [ebp-14Ch] BYREF
    float workMat[4][4]; // [esp+174h] [ebp-10Ch] BYREF
    float parentMat[4][4]; // [esp+1B4h] [ebp-CCh] BYREF
    float rotMat[4][4]; // [esp+1F4h] [ebp-8Ch] BYREF
    float axis[3][3]; // [esp+240h] [ebp-40h] BYREF
    int v11; // [esp+264h] [ebp-1Ch]
    float (*boneMat)[4][4]; // [esp+268h] [ebp-18h]
    bool isBoneModelSpace; // [esp+26Fh] [ebp-11h]
    //_UNKNOWN *v14; // [esp+274h] [ebp-Ch]
    //IKState *ikStatea; // [esp+278h] [ebp-8h]
    //float *rota; // [esp+280h] [ebp+0h]
    //
    //v14 = a1;
    //ikStatea = (IKState *)rota;
    isBoneModelSpace = 1;
    if (IK_IsCalcBone(ikState, boneName))
    {
        boneMat = (float (*)[4][4])ikState->matArrayPostIK[boneName];
        v11 = boneName - 10;
        switch (boneName)
        {
        case 10:
            boneMat = ikState->handMats;
            break;
        case 15:
            boneMat = (float (*)[4][4])ikState->handMats[1];
            break;
        case 18:
            boneMat = ikState->footMats;
            break;
        case 21:
            boneMat = (float (*)[4][4])ikState->footMats[1];
            break;
        default:
            isBoneModelSpace = 0;
            break;
        }
        AnglesToAxis(rot, axis);
        ikMatrixSet44(rotMat, vec3_origin, axis, 1.0);
        if (local)
        {
            *(_QWORD *)v5 = *(_QWORD *)&(*boneMat)[0][2];
            *(_QWORD *)&v5[2] = *(_QWORD *)&(*boneMat)[1][0];
            *(_QWORD *)&v5[4] = *(_QWORD *)&(*boneMat)[1][2];
            *(_QWORD *)&v5[6] = *(_QWORD *)&(*boneMat)[2][0];
            *(_QWORD *)&v5[8] = *(_QWORD *)&(*boneMat)[2][2];
            *(_QWORD *)&v5[10] = *(_QWORD *)&(*boneMat)[3][0];
            *(_QWORD *)&v5[12] = *(_QWORD *)&(*boneMat)[3][2];
            *(_QWORD *)&workMat[0][0] = *(_QWORD *)boneMat;
            *(_QWORD *)&workMat[0][2] = *(_QWORD *)v5;
            *(_QWORD *)&workMat[1][0] = *(_QWORD *)&v5[2];
            *(_QWORD *)&workMat[1][2] = *(_QWORD *)&v5[4];
            *(_QWORD *)&workMat[2][0] = *(_QWORD *)&v5[6];
            *(_QWORD *)&workMat[2][2] = *(_QWORD *)&v5[8];
            *(_QWORD *)&workMat[3][0] = *(_QWORD *)&v5[10];
            *(_QWORD *)&workMat[3][2] = *(_QWORD *)&v5[12];
            (*boneMat)[0][0] = (float)((float)(workMat[0][0] * rotMat[0][0]) + (float)(workMat[0][1] * rotMat[1][0]))
                + (float)(v5[0] * rotMat[2][0]);
            (*boneMat)[0][1] = (float)((float)(workMat[0][0] * rotMat[0][1]) + (float)(workMat[0][1] * rotMat[1][1]))
                + (float)(workMat[0][2] * rotMat[2][1]);
            (*boneMat)[0][2] = (float)((float)(workMat[0][0] * rotMat[0][2]) + (float)(workMat[0][1] * rotMat[1][2]))
                + (float)(workMat[0][2] * rotMat[2][2]);
            (*boneMat)[0][3] = 0.0f;
            (*boneMat)[1][0] = (float)((float)(workMat[1][0] * rotMat[0][0]) + (float)(workMat[1][1] * rotMat[1][0]))
                + (float)(workMat[1][2] * rotMat[2][0]);
            (*boneMat)[1][1] = (float)((float)(workMat[1][0] * rotMat[0][1]) + (float)(workMat[1][1] * rotMat[1][1]))
                + (float)(workMat[1][2] * rotMat[2][1]);
            (*boneMat)[1][2] = (float)((float)(workMat[1][0] * rotMat[0][2]) + (float)(workMat[1][1] * rotMat[1][2]))
                + (float)(workMat[1][2] * rotMat[2][2]);
            (*boneMat)[1][3] = 0.0f;
            (*boneMat)[2][0] = (float)((float)(workMat[2][0] * rotMat[0][0]) + (float)(workMat[2][1] * rotMat[1][0]))
                + (float)(workMat[2][2] * rotMat[2][0]);
            (*boneMat)[2][1] = (float)((float)(workMat[2][0] * rotMat[0][1]) + (float)(workMat[2][1] * rotMat[1][1]))
                + (float)(workMat[2][2] * rotMat[2][1]);
            (*boneMat)[2][2] = (float)((float)(workMat[2][0] * rotMat[0][2]) + (float)(workMat[2][1] * rotMat[1][2]))
                + (float)(workMat[2][2] * rotMat[2][2]);
            (*boneMat)[2][3] = 0.0f;
        }
        else
        {
            ikCalcBoneModelMatrix(ikState, IKBoneParents[boneName], (*ikState->matArrayPostIK)[0], parentMat);
            if (isBoneModelSpace)
            {
                *(_QWORD *)&v5[24] = *(_QWORD *)&(*boneMat)[0][2];
                *(_QWORD *)&v5[26] = *(_QWORD *)&(*boneMat)[1][0];
                *(_QWORD *)&v5[28] = *(_QWORD *)&(*boneMat)[1][2];
                *(_QWORD *)&v5[30] = *(_QWORD *)&(*boneMat)[2][0];
                *(_QWORD *)&v5[32] = *(_QWORD *)&(*boneMat)[2][2];
                *(_QWORD *)&v5[34] = *(_QWORD *)&(*boneMat)[3][0];
                *(_QWORD *)&v5[36] = *(_QWORD *)&(*boneMat)[3][2];
                *(_QWORD *)&workMat[0][0] = *(_QWORD *)boneMat;
                *(_QWORD *)&workMat[0][2] = *(_QWORD *)&v5[24];
                *(_QWORD *)&workMat[1][0] = *(_QWORD *)&v5[26];
                *(_QWORD *)&workMat[1][2] = *(_QWORD *)&v5[28];
                *(_QWORD *)&workMat[2][0] = *(_QWORD *)&v5[30];
                *(_QWORD *)&workMat[2][2] = *(_QWORD *)&v5[32];
                *(_QWORD *)&workMat[3][0] = *(_QWORD *)&v5[34];
                *(_QWORD *)&workMat[3][2] = *(_QWORD *)&v5[36];
                (*boneMat)[0][0] = (float)((float)(workMat[0][0] * rotMat[0][0]) + (float)(workMat[0][1] * rotMat[1][0]))
                    + (float)(v5[24] * rotMat[2][0]);
                (*boneMat)[0][1] = (float)((float)(workMat[0][0] * rotMat[0][1]) + (float)(workMat[0][1] * rotMat[1][1]))
                    + (float)(workMat[0][2] * rotMat[2][1]);
                (*boneMat)[0][2] = (float)((float)(workMat[0][0] * rotMat[0][2]) + (float)(workMat[0][1] * rotMat[1][2]))
                    + (float)(workMat[0][2] * rotMat[2][2]);
                (*boneMat)[0][3] = 0.0f;
                (*boneMat)[1][0] = (float)((float)(workMat[1][0] * rotMat[0][0]) + (float)(workMat[1][1] * rotMat[1][0]))
                    + (float)(workMat[1][2] * rotMat[2][0]);
                (*boneMat)[1][1] = (float)((float)(workMat[1][0] * rotMat[0][1]) + (float)(workMat[1][1] * rotMat[1][1]))
                    + (float)(workMat[1][2] * rotMat[2][1]);
                (*boneMat)[1][2] = (float)((float)(workMat[1][0] * rotMat[0][2]) + (float)(workMat[1][1] * rotMat[1][2]))
                    + (float)(workMat[1][2] * rotMat[2][2]);
                (*boneMat)[1][3] = 0.0f;
                (*boneMat)[2][0] = (float)((float)(workMat[2][0] * rotMat[0][0]) + (float)(workMat[2][1] * rotMat[1][0]))
                    + (float)(workMat[2][2] * rotMat[2][0]);
                (*boneMat)[2][1] = (float)((float)(workMat[2][0] * rotMat[0][1]) + (float)(workMat[2][1] * rotMat[1][1]))
                    + (float)(workMat[2][2] * rotMat[2][1]);
                (*boneMat)[2][2] = (float)((float)(workMat[2][0] * rotMat[0][2]) + (float)(workMat[2][1] * rotMat[1][2]))
                    + (float)(workMat[2][2] * rotMat[2][2]);
                (*boneMat)[2][3] = 0.0f;
            }
            else
            {
                ikCalcBoneModelMatrix(ikState, boneName, (*ikState->matArrayPostIK)[0], workMat);
                workMat2[0][0] = (float)((float)(workMat[0][0] * rotMat[0][0]) + (float)(workMat[0][1] * rotMat[1][0]))
                    + (float)(workMat[0][2] * rotMat[2][0]);
                workMat2[0][1] = (float)((float)(workMat[0][0] * rotMat[0][1]) + (float)(workMat[0][1] * rotMat[1][1]))
                    + (float)(workMat[0][2] * rotMat[2][1]);
                workMat2[0][2] = (float)((float)(workMat[0][0] * rotMat[0][2]) + (float)(workMat[0][1] * rotMat[1][2]))
                    + (float)(workMat[0][2] * rotMat[2][2]);
                workMat2[0][3] = 0.0f;
                workMat2[1][0] = (float)((float)(workMat[1][0] * rotMat[0][0]) + (float)(workMat[1][1] * rotMat[1][0]))
                    + (float)(workMat[1][2] * rotMat[2][0]);
                workMat2[1][1] = (float)((float)(workMat[1][0] * rotMat[0][1]) + (float)(workMat[1][1] * rotMat[1][1]))
                    + (float)(workMat[1][2] * rotMat[2][1]);
                workMat2[1][2] = (float)((float)(workMat[1][0] * rotMat[0][2]) + (float)(workMat[1][1] * rotMat[1][2]))
                    + (float)(workMat[1][2] * rotMat[2][2]);
                workMat2[1][3] = 0.0f;
                workMat2[2][0] = (float)((float)(workMat[2][0] * rotMat[0][0]) + (float)(workMat[2][1] * rotMat[1][0]))
                    + (float)(workMat[2][2] * rotMat[2][0]);
                workMat2[2][1] = (float)((float)(workMat[2][0] * rotMat[0][1]) + (float)(workMat[2][1] * rotMat[1][1]))
                    + (float)(workMat[2][2] * rotMat[2][1]);
                workMat2[2][2] = (float)((float)(workMat[2][0] * rotMat[0][2]) + (float)(workMat[2][1] * rotMat[1][2]))
                    + (float)(workMat[2][2] * rotMat[2][2]);
                workMat2[2][3] = 0.0f;
                //LODWORD(v5[63]) = workMat2[3];
                //LODWORD(v5[62]) = workMat[3];
                workMat2[3][0] = workMat[3][0];
                workMat2[3][1] = workMat[3][1];
                workMat2[3][2] = workMat[3][2];
                workMat2[3][3] = workMat[3][3];
                if (parentMat == (float (*)[4]) & v5[44]
                    && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.h", 165, 0, "%s", "in != out"))
                {
                    __debugbreak();
                }
                v5[44] = parentMat[0][0];
                v5[45] = parentMat[1][0];
                v5[46] = parentMat[2][0];
                v5[47] = 0.0f;
                v5[48] = parentMat[0][1];
                v5[49] = parentMat[1][1];
                v5[50] = parentMat[2][1];
                v5[51] = 0.0f;
                v5[52] = parentMat[0][2];
                v5[53] = parentMat[1][2];
                v5[54] = parentMat[2][2];
                v5[55] = 0.0f;
                (*boneMat)[0][0] = (float)((float)(workMat2[0][0] * parentMat[0][0]) + (float)(workMat2[0][1] * parentMat[0][1]))
                    + (float)(workMat2[0][2] * parentMat[0][2]);
                (*boneMat)[0][1] = (float)((float)(workMat2[0][0] * v5[45]) + (float)(workMat2[0][1] * v5[49]))
                    + (float)(workMat2[0][2] * v5[53]);
                (*boneMat)[0][2] = (float)((float)(workMat2[0][0] * v5[46]) + (float)(workMat2[0][1] * v5[50]))
                    + (float)(workMat2[0][2] * v5[54]);
                (*boneMat)[0][3] = 0.0f;
                (*boneMat)[1][0] = (float)((float)(workMat2[1][0] * v5[44]) + (float)(workMat2[1][1] * v5[48]))
                    + (float)(workMat2[1][2] * v5[52]);
                (*boneMat)[1][1] = (float)((float)(workMat2[1][0] * v5[45]) + (float)(workMat2[1][1] * v5[49]))
                    + (float)(workMat2[1][2] * v5[53]);
                (*boneMat)[1][2] = (float)((float)(workMat2[1][0] * v5[46]) + (float)(workMat2[1][1] * v5[50]))
                    + (float)(workMat2[1][2] * v5[54]);
                (*boneMat)[1][3] = 0.0f;
                (*boneMat)[2][0] = (float)((float)(workMat2[2][0] * v5[44]) + (float)(workMat2[2][1] * v5[48]))
                    + (float)(workMat2[2][2] * v5[52]);
                (*boneMat)[2][1] = (float)((float)(workMat2[2][0] * v5[45]) + (float)(workMat2[2][1] * v5[49]))
                    + (float)(workMat2[2][2] * v5[53]);
                (*boneMat)[2][2] = (float)((float)(workMat2[2][0] * v5[46]) + (float)(workMat2[2][1] * v5[50]))
                    + (float)(workMat2[2][2] * v5[54]);
                (*boneMat)[2][3] = 0.0f;
                //LODWORD(v5[43]) = parentMat[3];
                //LODWORD(v5[42]) = workMat2[3];
                v5[39] = workMat2[3][0] - parentMat[3][0];
                v5[40] = workMat2[3][1] - parentMat[3][1];
                v5[41] = workMat2[3][2] - parentMat[3][2];
                ikMatrixTransformVector34(&v5[39], (const float (*)[4]) & v5[44], (*boneMat)[3]);
                (*boneMat)[3][3] = 1.0f;
            }
        }
        ikState->bHasActiveLayers = 1;
        if (ikState->cacheActive)
        {
            memset(ikState->matArrayCache, 0, 0x5C0u);
            ikState->cacheActive = 0;
        }
        if (boneName != -1)
            ikState->modifiedIKBones |= 1 << boneName;
    }
}

void __cdecl ikMatrixLerp44(float (*from)[4], float (*to)[4], float lerp, float (*res)[4])
{
    float v4; // [esp+8h] [ebp-98h]
    float v5; // [esp+14h] [ebp-8Ch]
    float v6; // [esp+20h] [ebp-80h]
    float aFrom[3]; // [esp+4Ch] [ebp-54h] BYREF
    float pLerped[3]; // [esp+58h] [ebp-48h] BYREF
    float aLerped[3]; // [esp+64h] [ebp-3Ch] BYREF
    float axis[3][3]; // [esp+70h] [ebp-30h] BYREF
    float aTo[3]; // [esp+94h] [ebp-Ch] BYREF

    *(_QWORD *)&axis[0][0] = *(_QWORD *)&(*from)[0];
    axis[0][2] = (*from)[2];
    axis[1][0] = (*from)[4];
    axis[1][1] = (*from)[5];
    axis[1][2] = (*from)[6];
    axis[2][0] = (*from)[8];
    axis[2][1] = (*from)[9];
    axis[2][2] = (*from)[10];
    AxisToAngles(axis, aFrom);
    *(_QWORD *)&axis[0][0] = *(_QWORD *)&(*to)[0];
    axis[0][2] = (*to)[2];
    axis[1][0] = (*to)[4];
    axis[1][1] = (*to)[5];
    axis[1][2] = (*to)[6];
    axis[2][0] = (*to)[8];
    axis[2][1] = (*to)[9];
    axis[2][2] = (*to)[10];
    AxisToAngles(axis, aTo);
    v6 = aFrom[0];
    aLerped[0] = AngleNormalize180(aTo[0] - aFrom[0]) * lerp + v6;
    v5 = aFrom[1];
    aLerped[1] = AngleNormalize180(aTo[1] - aFrom[1]) * lerp + v5;
    v4 = aFrom[2];
    aLerped[2] = AngleNormalize180(aTo[2] - aFrom[2]) * lerp + v4;
    AnglesToAxis(aLerped, axis);
    Vec3Lerp(&(*from)[12], &(*to)[12], lerp, pLerped);
    ikMatrixSet44(res, pLerped, axis, 1.0);
}

