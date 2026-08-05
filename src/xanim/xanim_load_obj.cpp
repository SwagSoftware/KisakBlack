#include "xanim_load_obj.h"
#include <universal/com_files.h>
#include <clientscript/cscr_stringlist.h>
#include <qcommon/common.h>

const char *g_highPrecisionParts[18] =
{
  "j_ankle_le",
  "j_ankle_ri",
  "j_clavicle_le",
  "j_clavicle_ri",
  "j_elbow_le",
  "j_elbow_ri",
  "j_hip_le",
  "j_hip_ri",
  "j_knee_le",
  "j_knee_ri",
  "j_mainroot",
  "j_shoulder_le",
  "j_shoulder_ri",
  "j_spine4",
  "j_spinelower",
  "j_spineupper",
  "j_wrist_le",
  "j_wrist_ri"
};

int g_highPrecisionPart[160];
int g_checkBoneOrder;
HunkUser *g_animUser;
XAnimPartQuatPtr *g_partQuatArray;
XAnimPartTransPtr *g_partTransArray;
char *g_simpleQuatBits;

unsigned __int8 *__cdecl GetDeltaQuaternions(
                XAnimDeltaPart *deltaPart,
                void *(__cdecl *Alloc)(int),
                unsigned __int8 *pos,
                unsigned __int16 numloopframes,
                bool useSmallIndices)
{
    unsigned __int16 v6; // [esp+0h] [ebp-34h]
    __int16 *sQ; // [esp+4h] [ebp-30h]
    int m; // [esp+10h] [ebp-24h]
    int k; // [esp+14h] [ebp-20h]
    int j; // [esp+18h] [ebp-1Ch]
    int i; // [esp+1Ch] [ebp-18h]
    __int16 quat[4]; // [esp+20h] [ebp-14h] BYREF
    int size; // [esp+2Ch] [ebp-8h]
    unsigned __int16 numQuatIndices; // [esp+30h] [ebp-4h]

    v6 = *(_WORD *)pos;
    pos += 2;
    numQuatIndices = v6;
    if ( v6 )
    {
        if ( numQuatIndices == 1 )
        {
            ConsumeQuat2((const unsigned __int8 **)&pos, quat);
            deltaPart->quat = (XAnimDeltaPartQuat *)Alloc(8);
            deltaPart->quat->size = 0;
            deltaPart->quat->u.frame0[0] = quat[0];
            deltaPart->quat->u.frame0[1] = quat[1];
        }
        else
        {
            if ( numQuatIndices > (int)numloopframes
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                            204,
                            0,
                            "%s",
                            "numQuatIndices <= numloopframes") )
            {
                __debugbreak();
            }
            if ( useSmallIndices )
            {
                deltaPart->quat = (XAnimDeltaPartQuat *)Alloc(numQuatIndices + 8);
                if ( numQuatIndices >= (int)numloopframes )
                {
                    for ( i = 0; i < numQuatIndices; ++i )
                        deltaPart->quat->u.frames.indices._1[i] = i;
                }
                else
                {
                    size = numQuatIndices;
                    memcpy(deltaPart->quat->u.frames.indices._1, pos, numQuatIndices);
                    pos += numQuatIndices;
                }
            }
            else
            {
                deltaPart->quat = (XAnimDeltaPartQuat *)Alloc(2 * numQuatIndices + 8);
                if ( numQuatIndices >= (int)numloopframes )
                {
                    for ( j = 0; j < numQuatIndices; ++j )
                        deltaPart->quat->u.frames.indices._2[j] = j;
                }
                else
                {
                    size = 2 * numQuatIndices;
                    memcpy(deltaPart->quat->u.frames.indices._1, pos, size);
                    pos += size;
                }
            }
            deltaPart->quat->size = numQuatIndices - 1;
            deltaPart->quat->u.frames.frames = (__int16 (*)[2])Alloc(4 * numQuatIndices);
            for ( k = 0; k < numQuatIndices; ++k )
            {
                ConsumeQuat2((const unsigned __int8 **)&pos, quat);
                deltaPart->quat->u.frames.frames[k][0] = quat[0];
                deltaPart->quat->u.frames.frames[k][1] = quat[1];
            }
            for ( m = 1; m < numQuatIndices; ++m )
            {
                sQ = deltaPart->quat->u.frames.frames[m];
                if ( *(sQ - 1) * sQ[1] + *(sQ - 2) * *sQ < 0 )
                {
                    *sQ = -*sQ;
                    sQ[1] = -sQ[1];
                }
            }
        }
    }
    else
    {
        deltaPart->quat = 0;
    }
    return pos;
}

void __cdecl ConsumeQuat2(const unsigned __int8 **pos, __int16 *out)
{
    int v2; // [esp+8h] [ebp-1Ch]
    float v3; // [esp+Ch] [ebp-18h]
    __int16 v4; // [esp+14h] [ebp-10h]
    int temp; // [esp+20h] [ebp-4h]

    v4 = *(_WORD *)*pos;
    *pos += 2;
    *out = v4;
    temp = 1073676289 - *out * *out;
    if ( temp <= 0 )
    {
        v2 = 0;
    }
    else
    {
        v3 = floor(sqrtf((float)temp) + 0.5);
        v2 = (int)v3;
    }
    if ( v2 != (__int16)v2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                    132,
                    0,
                    "%s",
                    "iQ[1] == (short)iQ[1]") )
    {
        __debugbreak();
    }
    out[1] = v2;
}

unsigned __int8 *__cdecl GetDeltaTranslations(
                const char *name,
                XAnimDeltaPart *deltaPart,
                void *(__cdecl *Alloc)(int),
                const unsigned __int8 *pos,
                unsigned __int16 numloopframes,
                bool useSmallIndices)
{
    XAnimPartTransData *p_u; // edx
    const char *v7; // eax
    float v9; // [esp+4h] [ebp-2Ch]
    float v10; // [esp+8h] [ebp-28h]
    float v11; // [esp+Ch] [ebp-24h]
    unsigned __int16 v12; // [esp+10h] [ebp-20h]
    int j; // [esp+14h] [ebp-1Ch]
    int i; // [esp+18h] [ebp-18h]
    unsigned __int8 *posa; // [esp+44h] [ebp+14h]
    const unsigned __int8 *posb; // [esp+44h] [ebp+14h]

    v12 = *(_WORD *)pos;
    posa = (unsigned __int8 *)(pos + 2);
    if ( v12 )
    {
        if ( v12 == 1 )
        {
            v11 = *(float *)posa;
            posb = posa + 4;
            v10 = *(float *)posb;
            posb += 4;
            v9 = *(float *)posb;
            posa = (unsigned __int8 *)(posb + 4);
            deltaPart->trans = (XAnimPartTrans *)Alloc(16);
            deltaPart->trans->size = 0;
            p_u = &deltaPart->trans->u;
            p_u->frames.mins[0] = v11;
            p_u->frames.mins[1] = v10;
            p_u->frames.mins[2] = v9;
        }
        else
        {
            if ( v12 > (int)numloopframes )
            {
                v7 = va("ERROR: xanim %s numTransIndices > numloopframes\n", name);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                345,
                                0,
                                "%s\n\t%s",
                                "numTransIndices <= numloopframes",
                                v7) )
                    __debugbreak();
            }
            if ( useSmallIndices )
            {
                deltaPart->trans = (XAnimPartTrans *)Alloc(v12 + 32);
                if ( v12 >= (int)numloopframes )
                {
                    for ( i = 0; i < v12; ++i )
                        deltaPart->trans->u.frames.indices._1[i] = i;
                }
                else
                {
                    memcpy(deltaPart->trans->u.frames.indices._1, posa, v12);
                    posa += v12;
                }
            }
            else
            {
                deltaPart->trans = (XAnimPartTrans *)Alloc(2 * v12 + 32);
                if ( v12 >= (int)numloopframes )
                {
                    for ( j = 0; j < v12; ++j )
                        deltaPart->trans->u.frames.indices._2[j] = j;
                }
                else
                {
                    memcpy(deltaPart->trans->u.frames.indices._1, posa, 2 * v12);
                    posa += 2 * v12;
                }
            }
            return LoadTrans(Alloc, posa, v12, deltaPart->trans);
        }
    }
    else
    {
        deltaPart->trans = 0;
    }
    return posa;
}

unsigned __int8 *__cdecl LoadTrans(
                void *(__cdecl *Alloc)(int),
                unsigned __int8 *pos,
                unsigned __int16 numTransIndices,
                XAnimPartTrans *trans)
{
    float v5; // [esp+10h] [ebp-1Ch]
    float v6; // [esp+14h] [ebp-18h]
    float v7; // [esp+18h] [ebp-14h]
    float v8; // [esp+1Ch] [ebp-10h]
    float v9; // [esp+20h] [ebp-Ch]
    float v10; // [esp+24h] [ebp-8h]
    int size; // [esp+28h] [ebp-4h]
    const unsigned __int8 *posb; // [esp+38h] [ebp+Ch]
    unsigned __int8 *posa; // [esp+38h] [ebp+Ch]

    trans->smallTrans = *pos;
    posb = pos + 1;
    v10 = *(float *)posb;
    posb += 4;
    trans->u.frames.mins[0] = v10;
    v9 = *(float *)posb;
    posb += 4;
    trans->u.frames.mins[1] = v9;
    v8 = *(float *)posb;
    posb += 4;
    trans->u.frames.mins[2] = v8;
    v7 = *(float *)posb;
    posb += 4;
    trans->u.frames.size[0] = v7;
    v6 = *(float *)posb;
    posb += 4;
    trans->u.frames.size[1] = v6;
    v5 = *(float *)posb;
    posa = (unsigned __int8 *)(posb + 4);
    trans->u.frames.size[2] = v5;
    if ( trans->smallTrans )
    {
        trans->u.frames.size[0] = 0.0039215689 * trans->u.frames.size[0];
        trans->u.frames.size[1] = 0.0039215689 * trans->u.frames.size[1];
        trans->u.frames.size[2] = 0.0039215689 * trans->u.frames.size[2];
    }
    else
    {
        trans->u.frames.size[0] = 0.000015259022 * trans->u.frames.size[0];
        trans->u.frames.size[1] = 0.000015259022 * trans->u.frames.size[1];
        trans->u.frames.size[2] = 0.000015259022 * trans->u.frames.size[2];
    }
    trans->size = numTransIndices - 1;
    if ( trans->smallTrans )
        size = 3 * numTransIndices;
    else
        size = 6 * numTransIndices;
    trans->u.frames.frames._1 = (unsigned __int8 (*)[3])Alloc(size);
    memcpy((unsigned __int8 *)trans->u.frames.frames._1, posa, size);
    return &posa[size];
}

XAnimParts *__cdecl XAnimLoadFile(char *name, void *(__cdecl *Alloc)(int), bool streamedVersion)
{
    const char *v3; // eax
    __int16 StringOfSize; // ax
    unsigned __int8 *Quaternions; // eax
    int v7; // eax
    unsigned int v8; // ecx
    unsigned __int8 v9; // al
    unsigned __int8 v10; // al
    unsigned __int8 v11; // al
    unsigned __int8 v12; // al
    unsigned __int8 v13; // al
    int v14; // ecx
    unsigned int v15; // edx
    __int16 v16; // ax
    __int16 v17; // ax
    int v18; // edx
    __int16 *v19; // eax
    void *v20; // [esp+8h] [ebp-19C4h]
    void *v21; // [esp+Ch] [ebp-19C0h]
    int *v22; // [esp+10h] [ebp-19BCh]
    unsigned __int8 *v23; // [esp+14h] [ebp-19B8h]
    unsigned __int8 *v24; // [esp+18h] [ebp-19B4h]
    int *v25; // [esp+1Ch] [ebp-19B0h]
    __int16 *v26; // [esp+20h] [ebp-19ACh]
    unsigned __int8 *v27; // [esp+24h] [ebp-19A8h]
    int v28; // [esp+28h] [ebp-19A4h]
    int v29; // [esp+2Ch] [ebp-19A0h]
    int v30; // [esp+30h] [ebp-199Ch]
    int v31; // [esp+34h] [ebp-1998h]
    int v32; // [esp+38h] [ebp-1994h]
    int v33; // [esp+3Ch] [ebp-1990h]
    int v34; // [esp+40h] [ebp-198Ch]
    bool v35; // [esp+44h] [ebp-1988h]
    float v36; // [esp+58h] [ebp-1974h]
    unsigned __int16 v37; // [esp+5Eh] [ebp-196Eh]
    float v38; // [esp+A4h] [ebp-1928h]
    unsigned __int16 v39; // [esp+A8h] [ebp-1924h]
    __int16 numBones; // [esp+ACh] [ebp-1920h]
    unsigned __int16 v41; // [esp+B0h] [ebp-191Ch]
    __int16 v42; // [esp+B4h] [ebp-1918h]
    unsigned __int8 *pos; // [esp+C0h] [ebp-190Ch] BYREF
    bool v44; // [esp+C7h] [ebp-1905h]
    unsigned __int8 dst[20]; // [esp+C8h] [ebp-1904h] BYREF
    unsigned int j; // [esp+DCh] [ebp-18F0h]
    unsigned int v47; // [esp+E0h] [ebp-18ECh]
    char dest[1408]; // [esp+E4h] [ebp-18E8h] BYREF
    __int16 *dataShort; // [esp+664h] [ebp-1368h] BYREF
    unsigned int animPartIndex; // [esp+668h] [ebp-1364h]
    unsigned int v51; // [esp+66Ch] [ebp-1360h]
    unsigned __int8 *dataByte; // [esp+670h] [ebp-135Ch] BYREF
    int *v53; // [esp+674h] [ebp-1358h]
    int v54; // [esp+678h] [ebp-1354h]
    int i; // [esp+67Ch] [ebp-1350h]
    int v56; // [esp+680h] [ebp-134Ch]
    int v57; // [esp+684h] [ebp-1348h]
    int v58; // [esp+688h] [ebp-1344h]
    int v59; // [esp+68Ch] [ebp-1340h]
    int v60; // [esp+690h] [ebp-133Ch]
    int v61; // [esp+694h] [ebp-1338h]
    int v62; // [esp+698h] [ebp-1334h]
    int v63; // [esp+69Ch] [ebp-1330h]
    int v64; // [esp+6A0h] [ebp-132Ch]
    const char *fmt; // [esp+6A4h] [ebp-1328h]
    __int16 v66; // [esp+6A8h] [ebp-1324h]
    unsigned __int16 v67; // [esp+6ACh] [ebp-1320h]
    void *buffer; // [esp+6B0h] [ebp-131Ch] BYREF
    unsigned int count; // [esp+6B4h] [ebp-1318h]
    unsigned int v70; // [esp+6B8h] [ebp-1314h]
    int *v71; // [esp+6BCh] [ebp-1310h]
    int v72; // [esp+6C0h] [ebp-130Ch]
    _WORD base[162]; // [esp+6C4h] [ebp-1308h] BYREF
    unsigned int v74; // [esp+808h] [ebp-11C4h]
    unsigned __int8 *v75; // [esp+80Ch] [ebp-11C0h]
    int v76; // [esp+810h] [ebp-11BCh]
    unsigned __int8 *v77; // [esp+814h] [ebp-11B8h]
    unsigned __int8 *v78; // [esp+818h] [ebp-11B4h]
    unsigned int tableSize; // [esp+81Ch] [ebp-11B0h]
    XAnimIndices indices; // [esp+820h] [ebp-11ACh] BYREF
    XAnimPartQuatPtr part[160]; // [esp+824h] [ebp-11A8h] BYREF
    _WORD v82[160]; // [esp+D24h] [ebp-CA8h]
    bool v83; // [esp+E64h] [ebp-B68h]
    unsigned int PartQuatType; // [esp+E68h] [ebp-B64h]
    unsigned int v85[320]; // [esp+E6Ch] [ebp-B60h]
    unsigned __int8 v86[2]; // [esp+136Ch] [ebp-660h] BYREF
    XAnimPartTransPtr v87[160]; // [esp+14B4h] [ebp-518h] BYREF
    XAnimParts *parts; // [esp+19B4h] [ebp-18h]
    unsigned __int16 v89; // [esp+19B8h] [ebp-14h]
    char v90; // [esp+19BFh] [ebp-Dh]
    int v91; // [esp+19C0h] [ebp-Ch]
    int v92; // [esp+19C4h] [ebp-8h]
    int v93; // [esp+19C8h] [ebp-4h]

    if ( !g_checkBoneOrder )
    {
        g_checkBoneOrder = 1;
        for ( j = 0; j < 0x11; ++j )
        {
            if ( strcmp(g_highPrecisionParts[j], g_highPrecisionParts[j + 1]) >= 0 )
            {
                v3 = va("'%s' '%s'", g_highPrecisionParts[j], g_highPrecisionParts[j + 1]);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                892,
                                0,
                                "%s\n\t%s",
                                "strcmp( g_highPrecisionParts[j], g_highPrecisionParts[j + 1] ) < 0",
                                v3) )
                    __debugbreak();
            }
        }
    }
    fmt = "xanim/%s";
    if ( streamedVersion )
        fmt = "xanim/streamed/%s.ahi";
    if ( Com_sprintf(dest, 0x80u, fmt, name) >= 0 )
    {
        v72 = FS_ReadFile(dest, &buffer);
        if ( v72 >= 0 )
        {
            if ( v72 )
            {
                if ( !buffer
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp", 921, 0, "%s", "buf") )
                {
                    __debugbreak();
                }
                pos = (unsigned __int8 *)buffer;
                v42 = *(_WORD *)buffer;
                pos = (unsigned __int8 *)buffer + 2;
                v66 = v42;
                if ( v42 == 19 )
                {
                    v83 = I_strnicmp(name, "viewmodel_", 10) == 0;
                    v41 = *(_WORD *)pos;
                    pos += 2;
                    v67 = v41;
                    numBones = *(_WORD *)pos;
                    pos += 2;
                    iassert(numBones <= DOBJ_MAX_PARTS);
                    v90 = *pos++;
                    if ( (v90 & 8) != 0 )
                    {
                        FS_FreeFile(buffer);
                        return XAnimLoadFile(name, Alloc, 1);
                    }
                    else
                    {
                        parts = (XAnimParts *)Alloc(104);
                        if ( !parts
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                        953,
                                        0,
                                        "%s",
                                        "parts") )
                        {
                            __debugbreak();
                        }
                        parts->boneCount[9] = numBones;
                        parts->bLoop = (v90 & 1) != 0;
                        parts->bDelta = (v90 & 2) != 0;
                        parts->bLeftHandGripIK = (v90 & 4) != 0;
                        parts->bStreamable = (v90 & 8) != 0;
                        parts->loopEntryTime = 0.0f;
                        parts->assetType = *pos++;
                        v39 = *(_WORD *)pos;
                        pos += 2;
                        parts->framerate = (float)(__int16)v39;
                        parts->primedLength = 0.0f;
                        if ( parts->bStreamable )
                        {
                            v38 = *(float *)pos;
                            pos += 4;
                            parts->primedLength = v38;
                        }
                        if ( !v67
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                        974,
                                        0,
                                        "%s",
                                        "numframes") )
                        {
                            __debugbreak();
                        }
                        if ( parts->bLoop )
                            v37 = v67 + 1;
                        else
                            v37 = v67;
                        v89 = v37;
                        v44 = v37 <= 0x100u;
                        parts->numframes = v37 - 1;
                        if ( parts->numframes )
                            v36 = parts->framerate / (float)parts->numframes;
                        else
                            v36 = 0.0f;
                        parts->frequency = v36;
                        if ( parts->frequency < 0.0
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                        980,
                                        0,
                                        "%s\n\t(parts->frequency) = %g",
                                        "(parts->frequency >= 0)",
                                        parts->frequency) )
                        {
                            __debugbreak();
                        }
                        if ( parts->bDelta )
                        {
                            parts->deltaPart = (XAnimDeltaPart *)Alloc(8);
                            pos = GetDeltaQuaternions(parts->deltaPart, Alloc, pos, v89, v44);
                            pos = GetDeltaTranslations(name, parts->deltaPart, Alloc, pos, v89, v44);
                        }
                        if ( numBones )
                        {
                            count = ((unsigned int)(numBones - 1) >> 3) + 1;
                            v77 = pos;
                            pos += count;
                            memcpy(dst, pos, count);
                            pos += count;
                            memset(part, 0, 8 * numBones);
                            memset(v87, 0, 8 * numBones);
                            for ( j = 0; j < numBones; ++j )
                            {
                                part[j].partIndex = j;
                                v87[j].partIndex = j;
                            }
                        }
                        else
                        {
                            v77 = 0;
                        }
                        for ( j = 0; j < numBones; ++j )
                        {
                            count = strlen((const char *)pos) + 1;
                            StringOfSize = SL_GetStringOfSize(SCRIPTINSTANCE_SERVER, (char *)pos, 0, count, 9);
                            v82[j] = StringOfSize;
                            v35 = v83 || XAnimIsHighPrecisionPart((const char *)pos);
                            g_highPrecisionPart[j] = v35;
                            pos += count;
                        }
                        if ( g_animUser
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                        1021,
                                        0,
                                        "%s",
                                        "!g_animUser") )
                        {
                            __debugbreak();
                        }
                        g_animUser = Hunk_UserCreate(0x40000, HU_SCHEME_DEFAULT, 0, 0, "XAnimLoadFile", 13);
                        for ( j = 0; j < numBones; ++j )
                        {
                            Quaternions = GetQuaternions(
                                                            &part[j],
                                                            pos,
                                                            ((1 << (j & 7)) & v77[j >> 3]) != 0,
                                                            ((1 << (j & 7)) & (char)dst[j >> 3]) != 0,
                                                            v89,
                                                            v44);
                            pos = GetTranslations(&v87[j], Quaternions, v89, v44);
                        }
                        ReadNoteTracks(name, (unsigned __int16 **)&pos, parts, Alloc);
                        if ( (char *)buffer + v72 != (char *)pos
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                        1036,
                                        1,
                                        "%s\n\t(name) = %s",
                                        "(buf + fileSize == pos)",
                                        name) )
                        {
                            __debugbreak();
                        }
                        FS_FreeFile(buffer);
                        if ( numBones )
                        {
                            for ( j = 0; j < numBones; ++j )
                                base[j] = j;
                            g_partQuatArray = part;
                            g_partTransArray = v87;
                            g_simpleQuatBits = (char *)dst;
                            parts->names = (unsigned __int16 *)Alloc(2 * numBones);
                            i = 0;
                            v56 = 0;
                            v57 = 0;
                            v58 = 0;
                            v59 = 0;
                            v60 = 0;
                            v61 = 0;
                            v62 = 0;
                            v63 = 0;
                            v64 = 0;
                            qsort(base, numBones, 2u, (int (__cdecl *)(const void *, const void *))XAnimCompareQuatParts);
                            for ( j = 0; j < numBones; ++j )
                            {
                                animPartIndex = (unsigned __int16)base[j];
                                part[animPartIndex].partIndex = j;
                                v87[animPartIndex].partIndex = j;
                                v7 = *(unsigned int *)&part[animPartIndex].partIndex;
                                v8 = j;
                                v85[2 * j] = (unsigned int)part[animPartIndex].quat;
                                v85[2 * v8 + 1] = v7;
                                parts->names[j] = v82[animPartIndex];
                                PartQuatType = XAnimGetPartQuatType(animPartIndex);
                                *(&i + PartQuatType) = j + 1;
                            }
                            if ( v56 < i )
                                v34 = i;
                            else
                                v34 = v56;
                            v56 = v34;
                            if ( v57 < v34 )
                                v33 = v56;
                            else
                                v33 = v57;
                            v57 = v33;
                            if ( v58 < v33 )
                                v32 = v57;
                            else
                                v32 = v58;
                            v58 = v32;
                            if ( v59 < v32 )
                                v31 = v58;
                            else
                                v31 = v59;
                            v59 = v31;
                            v9 = truncate_cast<unsigned char>(i);
                            parts->boneCount[0] = v9;
                            v10 = truncate_cast<unsigned char>(v56 - i);
                            parts->boneCount[1] = v10;
                            v11 = truncate_cast<unsigned char>(v57 - v56);
                            parts->boneCount[2] = v11;
                            v12 = truncate_cast<unsigned char>(v58 - v57);
                            parts->boneCount[3] = v12;
                            v13 = truncate_cast<unsigned char>(v59 - v58);
                            parts->boneCount[4] = v13;
                            memcpy(v86, (unsigned __int8 *)base, 2 * numBones);
                            qsort(v86, numBones, 2u, (int (__cdecl *)(const void *, const void *))XAnimCompareTransParts);
                            for ( j = 0; j < numBones; ++j )
                            {
                                animPartIndex = *(unsigned __int16 *)&v86[2 * j];
                                v14 = *(unsigned int *)&v87[animPartIndex].partIndex;
                                v15 = j;
                                *(unsigned int *)&dest[8 * j + 128] = (unsigned int)v87[animPartIndex].trans;
                                *(unsigned int *)&dest[8 * v15 + 132] = v14;
                                PartQuatType = XAnimGetPartTransType(animPartIndex);
                                *(&i + PartQuatType) = j + 1;
                            }
                            if ( v61 < v60 )
                                v30 = v60;
                            else
                                v30 = v61;
                            v61 = v30;
                            if ( v62 < v30 )
                                v29 = v61;
                            else
                                v29 = v62;
                            v62 = v29;
                            if ( v63 < v29 )
                                v28 = v62;
                            else
                                v28 = v63;
                            v63 = v28;
                            parts->boneCount[5] = v60;
                            parts->boneCount[6] = v61 - v60;
                            parts->boneCount[7] = v62 - v61;
                            parts->boneCount[8] = v63 - v62;
                            v51 = 0;
                            v91 = 0;
                            v54 = 0;
                            v92 = 0;
                            v74 = 0;
                            v93 = 0;
                            v76 = 0;
                            animPartIndex = 0;
                            v70 = parts->boneCount[0];
                            animPartIndex = parts->boneCount[0];
                            v70 += parts->boneCount[1];
                            while ( animPartIndex < v70 )
                            {
                                tableSize = *(unsigned __int16 *)v85[2 * animPartIndex];
                                ++v91;
                                if ( v44 )
                                {
                                    v51 += tableSize + 1;
                                }
                                else if ( tableSize >= 0x40 )
                                {
                                    v91 += ((tableSize - 1) >> 8) + 1 + 1;
                                    v76 += tableSize + 1;
                                }
                                else
                                {
                                    v91 += tableSize + 1;
                                }
                                ++animPartIndex;
                                v74 += 2 * tableSize + 2;
                            }
                            v70 += parts->boneCount[2];
                            while ( animPartIndex < v70 )
                            {
                                tableSize = *(unsigned __int16 *)v85[2 * animPartIndex];
                                ++v91;
                                if ( v44 )
                                {
                                    v51 += tableSize + 1;
                                }
                                else if ( tableSize >= 0x40 )
                                {
                                    v91 += ((tableSize - 1) >> 8) + 1 + 1;
                                    v76 += tableSize + 1;
                                }
                                else
                                {
                                    v91 += tableSize + 1;
                                }
                                ++animPartIndex;
                                v74 += 4 * tableSize + 4;
                            }
                            v70 += parts->boneCount[3];
                            animPartIndex += parts->boneCount[3];
                            v91 += 2 * parts->boneCount[3];
                            v70 += parts->boneCount[4];
                            animPartIndex += parts->boneCount[4];
                            v91 += 4 * parts->boneCount[4];
                            if ( animPartIndex != parts->boneCount[9]
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                            1187,
                                            0,
                                            "animPartIndex == parts->boneCount[PART_TYPE_ALL]\n\t%i, %i",
                                            animPartIndex,
                                            parts->boneCount[9]) )
                            {
                                __debugbreak();
                            }
                            animPartIndex = 0;
                            v70 = parts->boneCount[5];
                            v51 += parts->boneCount[5];
                            v54 += 6 * parts->boneCount[5];
                            while ( animPartIndex < v70 )
                            {
                                tableSize = **(unsigned __int16 **)&dest[8 * animPartIndex + 128];
                                ++v91;
                                if ( v44 )
                                {
                                    v51 += tableSize + 1;
                                }
                                else if ( tableSize >= 0x40 )
                                {
                                    v91 += ((tableSize - 1) >> 8) + 1 + 1;
                                    v76 += tableSize + 1;
                                }
                                else
                                {
                                    v91 += tableSize + 1;
                                }
                                ++animPartIndex;
                                v92 += 3 * (tableSize + 1);
                            }
                            v70 += parts->boneCount[6];
                            v51 += parts->boneCount[6];
                            v54 += 6 * parts->boneCount[6];
                            while ( animPartIndex < v70 )
                            {
                                tableSize = **(unsigned __int16 **)&dest[8 * animPartIndex + 128];
                                ++v91;
                                if ( v44 )
                                {
                                    v51 += tableSize + 1;
                                }
                                else if ( tableSize >= 0x40 )
                                {
                                    v91 += ((tableSize - 1) >> 8) + 1 + 1;
                                    v76 += tableSize + 1;
                                }
                                else
                                {
                                    v91 += tableSize + 1;
                                }
                                ++animPartIndex;
                                v74 += 3 * (tableSize + 1);
                            }
                            v70 += parts->boneCount[7];
                            animPartIndex += parts->boneCount[7];
                            v51 += parts->boneCount[7];
                            v54 += 3 * parts->boneCount[7];
                            v70 += parts->boneCount[8];
                            animPartIndex += parts->boneCount[8];
                            v51 += parts->boneCount[8];
                            if ( animPartIndex != parts->boneCount[9]
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                            1220,
                                            0,
                                            "%s",
                                            "animPartIndex == parts->boneCount[PART_TYPE_ALL]") )
                            {
                                __debugbreak();
                            }
                            if ( v51 )
                                v27 = (unsigned __int8 *)Alloc(2 * v51);
                            else
                                v27 = 0;
                            dataByte = v27;
                            parts->dataByteCount = v51;
                            if ( parts->dataByteCount != v51
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                            1225,
                                            0,
                                            "%s",
                                            "parts->dataByteCount == dataByteCount") )
                            {
                                __debugbreak();
                            }
                            parts->dataByte = dataByte;
                            if ( v91 )
                                v26 = (__int16 *)Alloc(2 * v91);
                            else
                                v26 = 0;
                            dataShort = v26;
                            parts->dataShortCount = v91;
                            if ( parts->dataShortCount != v91
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                            1230,
                                            0,
                                            "%s",
                                            "parts->dataShortCount == dataShortCount") )
                            {
                                __debugbreak();
                            }
                            parts->dataShort = dataShort;
                            if ( v54 )
                                v25 = (int *)Alloc(4 * v54);
                            else
                                v25 = 0;
                            v71 = v25;
                            parts->dataIntCount = v54;
                            if ( parts->dataIntCount != v54
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                            1235,
                                            0,
                                            "%s",
                                            "parts->dataIntCount == dataIntCount") )
                            {
                                __debugbreak();
                            }
                            parts->dataInt = v71;
                            if ( v92 )
                                v24 = (unsigned __int8 *)Alloc(2 * v92);
                            else
                                v24 = 0;
                            v78 = v24;
                            parts->randomDataByteCount = v92;
                            if ( parts->randomDataByteCount != v92
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                            1241,
                                            0,
                                            "%s",
                                            "parts->randomDataByteCount == randomDataByteCount") )
                            {
                                __debugbreak();
                            }
                            parts->randomDataByte = v78;
                            if ( v74 )
                                v23 = (unsigned __int8 *)Alloc(2 * v74);
                            else
                                v23 = 0;
                            v75 = v23;
                            parts->randomDataShortCount = v74;
                            parts->randomDataShort = (__int16 *)v75;
                            if ( v93 )
                                v22 = (int *)Alloc(4 * v93);
                            else
                                v22 = 0;
                            v53 = v22;
                            parts->randomDataIntCount = v93;
                            if ( parts->randomDataIntCount != v93
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                            1254,
                                            0,
                                            "%s",
                                            "parts->randomDataIntCount == randomDataIntCount") )
                            {
                                __debugbreak();
                            }
                            parts->randomDataInt = v53;
                            if ( v44 )
                            {
                                if ( v76 )
                                    v21 = Alloc(v76);
                                else
                                    v21 = 0;
                                indices._1 = (unsigned __int8 *)v21;
                            }
                            else
                            {
                                if ( v76 )
                                    v20 = Alloc(2 * v76);
                                else
                                    v20 = 0;
                                indices._1 = (unsigned __int8 *)v20;
                            }
                            parts->indices = indices;
                            parts->indexCount = v76;
                            animPartIndex = 0;
                            v70 = parts->boneCount[0];
                            animPartIndex = parts->boneCount[0];
                            v70 += parts->boneCount[1];
                            while ( animPartIndex < v70 )
                            {
                                tableSize = *(unsigned __int16 *)v85[2 * animPartIndex];
                                v16 = truncate_cast<unsigned short>(tableSize);
                                *dataShort++ = v16;
                                memcpy(v75, *(unsigned __int8 **)(v85[2 * animPartIndex] + 4), 2 * (2 * tableSize + 2));
                                v75 += 4 * tableSize + 4;
                                XAnimEmitFrameIndices(
                                    tableSize,
                                    &dataByte,
                                    &dataShort,
                                    &indices,
                                    (XAnimDynamicIndices *)(v85[2 * animPartIndex++] + 8),
                                    v44);
                            }
                            v70 += parts->boneCount[2];
                            while ( animPartIndex < v70 )
                            {
                                tableSize = *(unsigned __int16 *)v85[2 * animPartIndex];
                                v17 = truncate_cast<unsigned short>(tableSize);
                                *dataShort++ = v17;
                                memcpy(v75, *(unsigned __int8 **)(v85[2 * animPartIndex] + 4), 2 * (4 * tableSize + 4));
                                v75 += 8 * tableSize + 8;
                                XAnimEmitFrameIndices(
                                    tableSize,
                                    &dataByte,
                                    &dataShort,
                                    &indices,
                                    (XAnimDynamicIndices *)(v85[2 * animPartIndex++] + 8),
                                    v44);
                            }
                            v70 += parts->boneCount[3];
                            while ( animPartIndex < v70 )
                            {
                                *(unsigned int *)dataShort = *(unsigned int *)(v85[2 * animPartIndex++] + 4);
                                dataShort += 2;
                            }
                            v70 += parts->boneCount[4];
                            while ( animPartIndex < v70 )
                            {
                                v18 = v85[2 * animPartIndex];
                                v19 = dataShort;
                                *(unsigned int *)dataShort = *(unsigned int *)(v18 + 4);
                                *((unsigned int *)v19 + 1) = *(unsigned int *)(v18 + 8);
                                ++animPartIndex;
                                dataShort += 4;
                            }
                            if ( animPartIndex != parts->boneCount[9]
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                            1426,
                                            0,
                                            "animPartIndex == parts->boneCount[PART_TYPE_ALL]\n\t%i, %i",
                                            animPartIndex,
                                            parts->boneCount[9]) )
                            {
                                __debugbreak();
                            }
                            animPartIndex = 0;
                            v70 = parts->boneCount[5];
                            while ( animPartIndex < v70 )
                            {
                                *dataByte++ = dest[8 * animPartIndex + 132];
                                *dataShort = **(_WORD **)&dest[8 * animPartIndex + 128];
                                tableSize = (unsigned __int16)*dataShort++;
                                *v71 = *(unsigned int *)(*(unsigned int *)&dest[8 * animPartIndex + 128] + 4);
                                v71[1] = *(unsigned int *)(*(unsigned int *)&dest[8 * animPartIndex + 128] + 8);
                                v71[2] = *(unsigned int *)(*(unsigned int *)&dest[8 * animPartIndex + 128] + 12);
                                v71[3] = *(unsigned int *)(*(unsigned int *)&dest[8 * animPartIndex + 128] + 16);
                                v71[4] = *(unsigned int *)(*(unsigned int *)&dest[8 * animPartIndex + 128] + 20);
                                v71[5] = *(unsigned int *)(*(unsigned int *)&dest[8 * animPartIndex + 128] + 24);
                                v47 = 0;
                                while ( v47 <= tableSize )
                                {
                                    *v78 = *(_BYTE *)(3 * v47 + *(unsigned int *)(*(unsigned int *)&dest[8 * animPartIndex + 128] + 28));
                                    v78[1] = *(_BYTE *)(*(unsigned int *)(*(unsigned int *)&dest[8 * animPartIndex + 128] + 28) + 3 * v47 + 1);
                                    v78[2] = *(_BYTE *)(*(unsigned int *)(*(unsigned int *)&dest[8 * animPartIndex + 128] + 28) + 3 * v47++ + 2);
                                    v78 += 3;
                                }
                                XAnimEmitFrameIndices(
                                    tableSize,
                                    &dataByte,
                                    &dataShort,
                                    &indices,
                                    (XAnimDynamicIndices *)(*(unsigned int *)&dest[8 * animPartIndex++ + 128] + 32),
                                    v44);
                                v71 += 6;
                            }
                            v70 += parts->boneCount[6];
                            while ( animPartIndex < v70 )
                            {
                                *dataByte++ = dest[8 * animPartIndex + 132];
                                *dataShort = **(_WORD **)&dest[8 * animPartIndex + 128];
                                tableSize = (unsigned __int16)*dataShort++;
                                *v71 = *(unsigned int *)(*(unsigned int *)&dest[8 * animPartIndex + 128] + 4);
                                v71[1] = *(unsigned int *)(*(unsigned int *)&dest[8 * animPartIndex + 128] + 8);
                                v71[2] = *(unsigned int *)(*(unsigned int *)&dest[8 * animPartIndex + 128] + 12);
                                v71[3] = *(unsigned int *)(*(unsigned int *)&dest[8 * animPartIndex + 128] + 16);
                                v71[4] = *(unsigned int *)(*(unsigned int *)&dest[8 * animPartIndex + 128] + 20);
                                v71[5] = *(unsigned int *)(*(unsigned int *)&dest[8 * animPartIndex + 128] + 24);
                                v47 = 0;
                                while ( v47 <= tableSize )
                                {
                                    *(_WORD *)v75 = *(_WORD *)(6 * v47 + *(unsigned int *)(*(unsigned int *)&dest[8 * animPartIndex + 128] + 28));
                                    *((_WORD *)v75 + 1) = *(_WORD *)(*(unsigned int *)(*(unsigned int *)&dest[8 * animPartIndex + 128] + 28)
                                                                                                 + 6 * v47
                                                                                                 + 2);
                                    *((_WORD *)v75 + 2) = *(_WORD *)(*(unsigned int *)(*(unsigned int *)&dest[8 * animPartIndex + 128] + 28)
                                                                                                 + 6 * v47++
                                                                                                 + 4);
                                    v75 += 6;
                                }
                                XAnimEmitFrameIndices(
                                    tableSize,
                                    &dataByte,
                                    &dataShort,
                                    &indices,
                                    (XAnimDynamicIndices *)(*(unsigned int *)&dest[8 * animPartIndex++ + 128] + 32),
                                    v44);
                                v71 += 6;
                            }
                            v70 += parts->boneCount[7];
                            while ( animPartIndex < v70 )
                            {
                                *dataByte = dest[8 * animPartIndex + 132];
                                *v71 = *(unsigned int *)(*(unsigned int *)&dest[8 * animPartIndex + 128] + 4);
                                v71[1] = *(unsigned int *)(*(unsigned int *)&dest[8 * animPartIndex + 128] + 8);
                                v71[2] = *(unsigned int *)(*(unsigned int *)&dest[8 * animPartIndex++ + 128] + 12);
                                ++dataByte;
                                v71 += 3;
                            }
                            v70 += parts->boneCount[8];
                            while ( animPartIndex < v70 )
                                *dataByte++ = dest[8 * animPartIndex++ + 132];
                            if ( animPartIndex != parts->boneCount[9]
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                            1506,
                                            0,
                                            "animPartIndex == parts->boneCount[PART_TYPE_ALL]\n\t%i, %i",
                                            animPartIndex,
                                            parts->boneCount[9]) )
                            {
                                __debugbreak();
                            }
                            if ( v44 )
                            {
                                if ( indices._1 != &parts->indices._1[v76]
                                    && !Assert_MyHandler(
                                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                                1510,
                                                0,
                                                "%s",
                                                "indices._1 == &parts->indices._1[indexCount]") )
                                {
                                    __debugbreak();
                                }
                            }
                            else if ( indices._1 != &parts->indices._1[2 * v76]
                                         && !Assert_MyHandler(
                                                     "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                                     1512,
                                                     0,
                                                     "%s",
                                                     "indices._2 == &parts->indices._2[indexCount]") )
                            {
                                __debugbreak();
                            }
                            if ( dataByte != &parts->dataByte[v51]
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                            1513,
                                            0,
                                            "%s",
                                            "dataByte == &parts->dataByte[dataByteCount]") )
                            {
                                __debugbreak();
                            }
                            if ( dataShort != &parts->dataShort[v91]
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                            1514,
                                            0,
                                            "%s",
                                            "dataShort == &parts->dataShort[dataShortCount]") )
                            {
                                __debugbreak();
                            }
                            if ( v71 != &parts->dataInt[v54]
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                            1515,
                                            0,
                                            "%s",
                                            "dataInt == &parts->dataInt[dataIntCount]") )
                            {
                                __debugbreak();
                            }
                            if ( v78 != &parts->randomDataByte[v92]
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                            1516,
                                            0,
                                            "%s",
                                            "randomDataByte == &parts->randomDataByte[randomDataByteCount]") )
                            {
                                __debugbreak();
                            }
                            if ( v75 != (unsigned __int8 *)&parts->randomDataShort[v74]
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                            1517,
                                            0,
                                            "%s",
                                            "randomDataShort == &parts->randomDataShort[randomDataShortCount]") )
                            {
                                __debugbreak();
                            }
                            if ( v53 != &parts->randomDataInt[v93]
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                            1518,
                                            0,
                                            "%s",
                                            "randomDataInt == &parts->randomDataInt[randomDataIntCount]") )
                            {
                                __debugbreak();
                            }
                            if ( v51 > 0x4000
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                            1520,
                                            0,
                                            "%s\n\t(name) = %s",
                                            "(dataByteCount * sizeof( byte ) <= 16 * 1024)",
                                            name) )
                            {
                                __debugbreak();
                            }
                            if ( (unsigned int)(2 * v91) > 0x4000
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                            1521,
                                            0,
                                            "%s\n\t(name) = %s",
                                            "(dataShortCount * sizeof( ushort ) <= 16 * 1024)",
                                            name) )
                            {
                                __debugbreak();
                            }
                            if ( (unsigned int)(4 * v54) > 0x4000
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                            1522,
                                            0,
                                            "%s\n\t(name) = %s",
                                            "(dataIntCount * sizeof( int ) <= 16 * 1024)",
                                            name) )
                            {
                                __debugbreak();
                            }
                        }
                        if ( !g_animUser
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                                        1525,
                                        0,
                                        "%s",
                                        "g_animUser") )
                        {
                            __debugbreak();
                        }
                        Hunk_UserDestroy(g_animUser);
                        g_animUser = 0;
                        return parts;
                    }
                }
                else
                {
                    FS_FreeFile(buffer);
                    Com_PrintError(19, "ERROR: xanim '%s' out of date (version %d, expecting %d)\n", name, v66, 19);
                    return 0;
                }
            }
            else
            {
                Com_PrintError(19, "ERROR: xanim '%s' has 0 length\n", name);
                FS_FreeFile(buffer);
                return 0;
            }
        }
        else
        {
            if ( buffer
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp", 909, 0, "%s", "!buf") )
            {
                __debugbreak();
            }
            Com_PrintError(19, "ERROR: xanim '%s' not found\n", name);
            return 0;
        }
    }
    else
    {
        Com_PrintError(19, "ERROR: filename '%s' too long\n", dest);
        return 0;
    }
}

void __cdecl ReadNoteTracks(const char *name, unsigned __int16 **pos, XAnimParts *parts, void *(__cdecl *Alloc)(int))
{
    const char *v4; // eax
    float v5; // [esp+8h] [ebp-2Ch]
    unsigned __int16 v6; // [esp+1Ch] [ebp-18h]
    bool customLoopEntry; // [esp+23h] [ebp-11h]
    int numNoteTracks; // [esp+24h] [ebp-10h]
    XAnimNotifyInfo *notify; // [esp+2Ch] [ebp-8h]
    int i; // [esp+30h] [ebp-4h]

    customLoopEntry = 0;
    numNoteTracks = *(unsigned __int8 *)*pos;
    *pos = (unsigned __int16 *)((char *)*pos + 1);
    if ( numNoteTracks >= 255
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                    149,
                    0,
                    "%s",
                    "numNoteTracks < 255") )
    {
        __debugbreak();
    }
    parts->notifyCount = numNoteTracks + 1;
    notify = (XAnimNotifyInfo *)Alloc(8 * parts->notifyCount);
    parts->notify = notify;
    for ( i = 0; i < numNoteTracks; ++i )
    {
        if ( !I_stricmp("loop_begin", (const char *)*pos) )
            customLoopEntry = 1;
        notify->name = SL_GetString_(SCRIPTINSTANCE_SERVER, (char *)*pos, 0, 3);
        *pos = (unsigned __int16 *)((char *)*pos + strlen((const char *)*pos) + 1);
        v6 = *(*pos)++;
        if ( parts->numframes )
            v5 = (float)v6 / (float)parts->numframes;
        else
            v5 = 0.0f;
        notify->time = v5;
        if ( customLoopEntry && parts->bLoop )
            parts->loopEntryTime = notify->time;
        customLoopEntry = 0;
        if ( notify->time < 0.0 || notify->time > 1.0 )
        {
            v4 = va("xanim: '%s', time: %f", name, notify->time);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                            173,
                            0,
                            "%s\n\t%s",
                            "notify->time >= 0.0f && notify->time <= 1.0f",
                            v4) )
                __debugbreak();
        }
        ++notify;
    }
    notify->name = SL_GetString_(SCRIPTINSTANCE_SERVER, "end", 0, 3);
    notify->time = 1.0f;
}

unsigned __int8 *__cdecl GetQuaternions(
                XAnimPartQuatPtr *part,
                unsigned __int8 *pos,
                bool bFlipQuat,
                bool bSimpleQuat,
                unsigned __int16 numloopframes,
                bool useSmallIndices)
{
    unsigned __int16 v7; // [esp+0h] [ebp-48h]
    __int16 *v8; // [esp+4h] [ebp-44h]
    __int16 *v9; // [esp+8h] [ebp-40h]
    int ii; // [esp+10h] [ebp-38h]
    int n; // [esp+14h] [ebp-34h]
    __int16 *sQ; // [esp+18h] [ebp-30h]
    int m; // [esp+24h] [ebp-24h]
    int k; // [esp+28h] [ebp-20h]
    int j; // [esp+2Ch] [ebp-1Ch]
    int i; // [esp+30h] [ebp-18h]
    __int16 quat[4]; // [esp+34h] [ebp-14h] BYREF
    int size; // [esp+40h] [ebp-8h]
    unsigned __int16 numQuatIndices; // [esp+44h] [ebp-4h]

    v7 = *(_WORD *)pos;
    pos += 2;
    numQuatIndices = v7;
    if ( v7 )
    {
        if ( numQuatIndices == 1 )
        {
            if ( bSimpleQuat )
            {
                ConsumeQuat2((const unsigned __int8 **)&pos, quat);
                if ( bFlipQuat )
                {
                    quat[0] = -quat[0];
                    quat[1] = -quat[1];
                }
                part->quat = (XAnimPartQuat *)XAnimTempAlloc(8);
                part->quat->u.frame0[0] = quat[0];
                part->quat->u.frame0[1] = quat[1];
            }
            else
            {
                ConsumeQuat((const unsigned __int8 **)&pos, quat);
                if ( bFlipQuat )
                {
                    quat[0] = -quat[0];
                    quat[1] = -quat[1];
                    quat[2] = -quat[2];
                    quat[3] = -quat[3];
                }
                part->quat = (XAnimPartQuat *)XAnimTempAlloc(12);
                part->quat->u = *(XAnimPartQuatData *)quat;
            }
            part->quat->size = 0;
        }
        else
        {
            if ( numQuatIndices > (int)numloopframes
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                            449,
                            0,
                            "%s",
                            "numQuatIndices <= numloopframes") )
            {
                __debugbreak();
            }
            if ( useSmallIndices )
            {
                part->quat = (XAnimPartQuat *)XAnimTempAlloc(numQuatIndices + 8);
                if ( numQuatIndices >= (int)numloopframes )
                {
                    for ( i = 0; i < numQuatIndices; ++i )
                        part->quat->u.frames.indices._1[i] = i;
                }
                else
                {
                    size = numQuatIndices;
                    memcpy(part->quat->u.frames.indices._1, pos, numQuatIndices);
                    pos += numQuatIndices;
                }
            }
            else
            {
                part->quat = (XAnimPartQuat *)XAnimTempAlloc(2 * numQuatIndices + 8);
                if ( numQuatIndices >= (int)numloopframes )
                {
                    for ( j = 0; j < numQuatIndices; ++j )
                        part->quat->u.frames.indices._2[j] = j;
                }
                else
                {
                    size = 2 * numQuatIndices;
                    memcpy(part->quat->u.frames.indices._1, pos, size);
                    pos += size;
                }
            }
            if ( bSimpleQuat )
            {
                part->quat->u.frames.u.frames = (__int16 (*)[4])XAnimTempAlloc(4 * numQuatIndices);
                ConsumeQuat2((const unsigned __int8 **)&pos, quat);
                if ( bFlipQuat )
                {
                    quat[0] = -quat[0];
                    quat[1] = -quat[1];
                }
                *(_WORD *)part->quat->u.frames.u.frames = quat[0];
                (*part->quat->u.frames.u.frames)[1] = quat[1];
                for ( k = 1; k < numQuatIndices; ++k )
                {
                    ConsumeQuat2((const unsigned __int8 **)&pos, quat);
                    (*part->quat->u.frames.u.frames)[2 * k] = quat[0];
                    (*part->quat->u.frames.u.frames)[2 * k + 1] = quat[1];
                }
                for ( m = 1; m < numQuatIndices; ++m )
                {
                    sQ = &(*part->quat->u.frames.u.frames)[2 * m];
                    if ( *(sQ - 1) * sQ[1] + *(sQ - 2) * *sQ < 0 )
                    {
                        *sQ = -*sQ;
                        sQ[1] = -sQ[1];
                    }
                }
            }
            else
            {
                part->quat->u.frames.u.frames = (__int16 (*)[4])XAnimTempAlloc(8 * numQuatIndices);
                ConsumeQuat((const unsigned __int8 **)&pos, quat);
                if ( bFlipQuat )
                {
                    quat[0] = -quat[0];
                    quat[1] = -quat[1];
                    quat[2] = -quat[2];
                    quat[3] = -quat[3];
                }
                *(XAnimPartQuatData *)&(*part->quat->u.frames.u.frames)[0] = *(XAnimPartQuatData *)quat;
                for ( n = 1; n < numQuatIndices; ++n )
                {
                    ConsumeQuat((const unsigned __int8 **)&pos, quat);
                    part->quat->u.frames.u.frames[n][0] = quat[0];
                    part->quat->u.frames.u.frames[n][1] = quat[1];
                    part->quat->u.frames.u.frames[n][2] = quat[2];
                    part->quat->u.frames.u.frames[n][3] = quat[3];
                }
                for ( ii = 1; ii < numQuatIndices; ++ii )
                {
                    v8 = part->quat->u.frames.u.frames[ii];
                    v9 = v8 - 4;
                    if ( v9[3] * v8[3] + v9[2] * v8[2] + v9[1] * v8[1] + *v9 * *v8 < 0 )
                    {
                        *v8 = -*v8;
                        v8[1] = -v8[1];
                        v8[2] = -v8[2];
                        v8[3] = -v8[3];
                    }
                }
            }
            part->quat->size = numQuatIndices - 1;
        }
    }
    else
    {
        if ( !bSimpleQuat
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp", 407, 0, "%s", "bSimpleQuat") )
        {
            __debugbreak();
        }
        part->quat = 0;
    }
    return pos;
}

void __cdecl ConsumeQuat(const unsigned __int8 **pos, __int16 *out)
{
    int v2; // [esp+8h] [ebp-2Ch]
    float v3; // [esp+Ch] [ebp-28h]
    __int16 v4; // [esp+14h] [ebp-20h]
    __int16 v5; // [esp+18h] [ebp-1Ch]
    __int16 v6; // [esp+1Ch] [ebp-18h]
    int temp; // [esp+30h] [ebp-4h]

    v6 = *(_WORD *)*pos;
    *pos += 2;
    *out = v6;
    v5 = *(_WORD *)*pos;
    *pos += 2;
    out[1] = v5;
    v4 = *(_WORD *)*pos;
    *pos += 2;
    out[2] = v4;
    temp = 1073676289 - (out[2] * out[2] + out[1] * out[1] + *out * *out);
    if ( temp <= 0 )
    {
        v2 = 0;
    }
    else
    {
        v3 = floor(sqrtf((float)temp) + 0.5);
        v2 = (int)v3;
    }
    if ( v2 != (__int16)v2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                    114,
                    0,
                    "%s",
                    "iQ[3] == (short)iQ[3]") )
    {
        __debugbreak();
    }
    out[3] = v2;
}

void *__cdecl XAnimTempAlloc(int size)
{
    if ( !g_animUser
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp", 393, 0, "%s", "g_animUser") )
    {
        __debugbreak();
    }
    return Hunk_UserAlloc(g_animUser, size, 4, 0);
}

unsigned __int8 *__cdecl GetTranslations(
                XAnimPartTransPtr *part,
                const unsigned __int8 *pos,
                unsigned __int16 numloopframes,
                bool useSmallIndices)
{
    XAnimPartTransData *p_u; // edx
    float v6; // [esp+4h] [ebp-2Ch]
    float v7; // [esp+8h] [ebp-28h]
    float v8; // [esp+Ch] [ebp-24h]
    unsigned __int16 v9; // [esp+10h] [ebp-20h]
    int j; // [esp+14h] [ebp-1Ch]
    int i; // [esp+18h] [ebp-18h]
    unsigned __int8 *posa; // [esp+3Ch] [ebp+Ch]
    const unsigned __int8 *posb; // [esp+3Ch] [ebp+Ch]

    v9 = *(_WORD *)pos;
    posa = (unsigned __int8 *)(pos + 2);
    if ( v9 )
    {
        if ( v9 == 1 )
        {
            v8 = *(float *)posa;
            posb = posa + 4;
            v7 = *(float *)posb;
            posb += 4;
            v6 = *(float *)posb;
            posa = (unsigned __int8 *)(posb + 4);
            part->trans = (XAnimPartTrans *)XAnimTempAlloc(16);
            part->trans->size = 0;
            p_u = &part->trans->u;
            p_u->frames.mins[0] = v8;
            p_u->frames.mins[1] = v7;
            p_u->frames.mins[2] = v6;
        }
        else
        {
            if ( v9 > (int)numloopframes
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                            598,
                            0,
                            "%s",
                            "numTransIndices <= numloopframes") )
            {
                __debugbreak();
            }
            if ( useSmallIndices )
            {
                part->trans = (XAnimPartTrans *)XAnimTempAlloc(v9 + 32);
                if ( v9 >= (int)numloopframes )
                {
                    for ( i = 0; i < v9; ++i )
                        part->trans->u.frames.indices._1[i] = i;
                }
                else
                {
                    memcpy(part->trans->u.frames.indices._1, posa, v9);
                    posa += v9;
                }
            }
            else
            {
                part->trans = (XAnimPartTrans *)XAnimTempAlloc(2 * v9 + 32);
                if ( v9 >= (int)numloopframes )
                {
                    for ( j = 0; j < v9; ++j )
                        part->trans->u.frames.indices._2[j] = j;
                }
                else
                {
                    memcpy(part->trans->u.frames.indices._1, posa, 2 * v9);
                    posa += 2 * v9;
                }
            }
            return LoadTrans(XAnimTempAlloc, posa, v9, part->trans);
        }
    }
    else
    {
        part->trans = 0;
    }
    return posa;
}

unsigned int __cdecl XAnimGetPartQuatType(unsigned int animPartIndex)
{
    XAnimPartQuat *quat; // [esp+0h] [ebp-4h]

    quat = g_partQuatArray[animPartIndex].quat;
    if ( ((1 << (animPartIndex & 7)) & g_simpleQuatBits[animPartIndex >> 3]) != 0 )
    {
        if ( quat )
        {
            if ( quat->size )
                return 1;
            else
                return 3;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        if ( !quat
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp", 679, 0, "%s", "quat") )
        {
            __debugbreak();
        }
        if ( quat->size )
            return 2;
        else
            return 4;
    }
}

unsigned int __cdecl XAnimGetPartTransType(unsigned int animPartIndex)
{
    XAnimPartTrans *trans; // [esp+0h] [ebp-4h]

    trans = g_partTransArray[animPartIndex].trans;
    if ( !trans )
        return 8;
    if ( !trans->size )
        return 7;
    if ( trans->smallTrans )
        return 5;
    return 6;
}

int __cdecl XAnimCompareQuatParts(unsigned __int16 *p0, unsigned __int16 *p1)
{
    unsigned int PartQuatType; // eax
    unsigned int partArrayIndices[2]; // [esp+0h] [ebp-18h]
    int diff; // [esp+8h] [ebp-10h]
    unsigned int type[2]; // [esp+Ch] [ebp-Ch]
    unsigned int i; // [esp+14h] [ebp-4h]

    partArrayIndices[0] = *p0;
    partArrayIndices[1] = *p1;
    for ( i = 0; i < 2; ++i )
    {
        PartQuatType = XAnimGetPartQuatType(partArrayIndices[i]);
        type[i] = PartQuatType;
    }
    diff = type[0] - type[1];
    if ( type[0] == type[1] )
        return g_partQuatArray[partArrayIndices[0]].partIndex - g_partQuatArray[partArrayIndices[1]].partIndex;
    else
        return diff;
}

int __cdecl XAnimCompareTransParts(unsigned __int16 *p0, unsigned __int16 *p1)
{
    unsigned int PartTransType; // eax
    unsigned int partArrayIndices[2]; // [esp+0h] [ebp-18h]
    int diff; // [esp+8h] [ebp-10h]
    unsigned int type[2]; // [esp+Ch] [ebp-Ch]
    unsigned int i; // [esp+14h] [ebp-4h]

    partArrayIndices[0] = *p0;
    partArrayIndices[1] = *p1;
    for ( i = 0; i < 2; ++i )
    {
        PartTransType = XAnimGetPartTransType(partArrayIndices[i]);
        type[i] = PartTransType;
    }
    diff = type[0] - type[1];
    if ( type[0] == type[1] )
        return g_partTransArray[partArrayIndices[0]].partIndex - g_partTransArray[partArrayIndices[1]].partIndex;
    else
        return diff;
}

int __cdecl XAnimIsHighPrecisionPart(const char *name)
{
    int v2; // [esp+4h] [ebp-20h]
    unsigned int low; // [esp+18h] [ebp-Ch]
    unsigned int high; // [esp+1Ch] [ebp-8h]

    low = 0;
    high = 18;
    while ( low < high )
    {
        v2 = strcmp(name, g_highPrecisionParts[(high + low) >> 1]);
        if ( v2 >= 0 )
        {
            if ( v2 <= 0 )
                return 1;
            low = ((high + low) >> 1) + 1;
        }
        else
        {
            high = (high + low) >> 1;
        }
    }
    return 0;
}

void __cdecl XAnimEmitFrameIndices(
                unsigned int tableSize,
                unsigned __int8 **dataByte,
                __int16 **dataShort,
                XAnimIndices *indices,
                XAnimDynamicIndices *frameIndices,
                bool useSmallIndices)
{
    unsigned int i; // [esp+4h] [ebp-8h]
    unsigned int longTableSize; // [esp+8h] [ebp-4h]

    if ( useSmallIndices )
    {
        memcpy(*dataByte, frameIndices->_1, tableSize + 1);
        *dataByte += tableSize + 1;
    }
    else if ( tableSize >= 0x40 )
    {
        memcpy(indices->_1, frameIndices->_1, 2 * tableSize + 2);
        indices->_1 += 2 * tableSize + 2;
        longTableSize = ((tableSize - 1) >> 8) + 1;
        if ( frameIndices->_2[0]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_load_obj.cpp",
                        828,
                        0,
                        "%s",
                        "!frameIndices->_2[0]") )
        {
            __debugbreak();
        }
        for ( i = 0; i < longTableSize; ++i )
            (*dataShort)[i] = frameIndices[256 * i]._2[0];
        (*dataShort)[longTableSize] = frameIndices[tableSize]._2[0];
        *dataShort += longTableSize + 1;
    }
    else
    {
        memcpy((unsigned __int8 *)*dataShort, frameIndices->_1, 2 * tableSize + 2);
        *dataShort += tableSize + 1;
    }
}

