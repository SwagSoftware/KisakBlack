#include "dobj_utils.h"
#include "dobj.h"
#include "xmodel.h"

#include <universal/assertive.h>
#include <clientscript/cscr_stringlist.h>
#include <universal/q_shared.h>
#include "xmodel_utils.h"
#include <EffectsCore/fx_beam.h>
#include <gfx_d3d/r_dvars.h>
#include <ik/ik_import.h>

DObjAnimMat *__cdecl DObjGetRotTransArray(const DObj *obj)
{
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 37, 0, "%s", "obj") )
        __debugbreak();
    return obj->skel.mat;
}

int __cdecl DObjGetNumModels(const DObj *obj)
{
    return obj->numModels;
}

unsigned __int16 __cdecl DObjGetEntNum(const DObj *obj)
{
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 73, 0, "%s", "obj") )
        __debugbreak();
    return obj->entnum;
}

int __cdecl DObjGetSurfaces(const DObj *obj, int *partBits, const char *lods)
{
    int n; // [esp+4h] [ebp-64h]
    int m; // [esp+8h] [ebp-60h]
    int k; // [esp+Ch] [ebp-5Ch]
    int i; // [esp+10h] [ebp-58h]
    int j; // [esp+14h] [ebp-54h]
    int numBones; // [esp+18h] [ebp-50h]
    int numBonesa; // [esp+18h] [ebp-50h]
    int boneIndex; // [esp+1Ch] [ebp-4Ch]
    int numModels; // [esp+24h] [ebp-44h]
    XModel *model; // [esp+28h] [ebp-40h]
    XModel *modela; // [esp+28h] [ebp-40h]
    int surfaceCount; // [esp+2Ch] [ebp-3Ch]
    char targBoneIndexLow; // [esp+30h] [ebp-38h]
    int lod; // [esp+38h] [ebp-30h]
    int loda; // [esp+38h] [ebp-30h]
    int targBoneIndexHigh; // [esp+3Ch] [ebp-2Ch]
    int surfPartBits[9]; // [esp+40h] [ebp-28h]
    XModel **models; // [esp+64h] [ebp-4h]

    numModels = obj->numModels;
    if ( !obj->numModels
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 107, 0, "%s", "numModels") )
    {
        __debugbreak();
    }
    models = obj->localModels;
    model = *models;
    numBones = XModelNumBones(*models);
    lod = *lods;
    if ( lod < 0 )
    {
        surfaceCount = 0;
        for ( k = 0; k < 5; ++k )
            partBits[k] = 0;
    }
    else
    {
        surfaceCount = model->lodInfo[lod].numsurfs;
        for ( i = 0; i < 5; ++i )
            partBits[i] = model->lodInfo[lod].partBits[i];
    }
    boneIndex = numBones;
    for ( m = 0; m < 4; ++m )
        surfPartBits[m] = 0;
    for ( j = 1; j < numModels; ++j )
    {
        modela = models[j];
        numBonesa = XModelNumBones(modela);
        loda = lods[j];
        if ( loda >= 0 )
        {
            surfaceCount += modela->lodInfo[loda].numsurfs;
            for ( n = 0; n < 5; ++n )
                surfPartBits[n + 4] = modela->lodInfo[loda].partBits[n];
            targBoneIndexHigh = boneIndex >> 5;
            targBoneIndexLow = boneIndex & 0x1F;
            if ( (boneIndex & 0x1F) != 0 )
            {
                *partBits |= (unsigned int)surfPartBits[4 - targBoneIndexHigh] >> targBoneIndexLow;
                partBits[1] |= ((unsigned int)surfPartBits[5 - targBoneIndexHigh] >> targBoneIndexLow)
                                         | (surfPartBits[4 - targBoneIndexHigh] << (32 - targBoneIndexLow));
                partBits[2] |= ((unsigned int)surfPartBits[6 - targBoneIndexHigh] >> targBoneIndexLow)
                                         | (surfPartBits[5 - targBoneIndexHigh] << (32 - targBoneIndexLow));
                partBits[3] |= ((unsigned int)surfPartBits[7 - targBoneIndexHigh] >> targBoneIndexLow)
                                         | (surfPartBits[6 - targBoneIndexHigh] << (32 - targBoneIndexLow));
                partBits[4] |= ((unsigned int)surfPartBits[8 - targBoneIndexHigh] >> targBoneIndexLow)
                                         | (surfPartBits[7 - targBoneIndexHigh] << (32 - targBoneIndexLow));
            }
            else
            {
                *partBits |= surfPartBits[4 - targBoneIndexHigh];
                partBits[1] |= surfPartBits[5 - targBoneIndexHigh];
                partBits[2] |= surfPartBits[6 - targBoneIndexHigh];
                partBits[3] |= surfPartBits[7 - targBoneIndexHigh];
                partBits[4] |= surfPartBits[8 - targBoneIndexHigh];
            }
        }
        boneIndex += numBonesa;
    }
    return surfaceCount;
}

void __cdecl DObjGetSurfaceData(const DObj *obj, const float *origin, float scale, bool offscreen, char *lods)
{
    float BaseLodDist; // [esp+8h] [ebp-34h]
    XModelLodRampType lodRampType; // [esp+20h] [ebp-1Ch]
    XModel *model; // [esp+24h] [ebp-18h]
    float adjustedDist; // [esp+28h] [ebp-14h]
    char lod; // [esp+2Ch] [ebp-10h]
    float baseDist; // [esp+30h] [ebp-Ch]
    int modelCount; // [esp+34h] [ebp-8h]
    int modelIndex; // [esp+38h] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 220, 0, "%s", "obj") )
        __debugbreak();

    nanassertvec3(origin);

    modelCount = DObjGetNumModels(obj);
    if ( modelCount > 32
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp",
                    224,
                    1,
                    "%s",
                    "modelCount <= DOBJ_MAX_SUBMODELS") )
    {
        __debugbreak();
    }
    BaseLodDist = R_GetBaseLodDist(origin);
    baseDist = I_fres(scale) * BaseLodDist;

    iassert(!IS_NAN(scale));
    iassert(!IS_NAN(baseDist));
    
    for ( modelIndex = 0; modelIndex < modelCount; ++modelIndex )
    {
        model = DObjGetModel(obj, modelIndex);
        lod = -1;
        if ( model )
        {
            lodRampType = XModelGetLodRampType(model);
            adjustedDist = R_GetAdjustedLodDist(baseDist, lodRampType);
            if ( offscreen )
                adjustedDist = adjustedDist * r_offscreenCasterLodScale->current.value;
            lod = DObjGetLodForDist(obj, modelIndex, adjustedDist, baseDist);
        }
        lods[modelIndex] = lod;
    }
}

void __cdecl DObjGetBoneInfo(const DObj *obj, XBoneInfo **boneInfo)
{
    int j; // [esp+0h] [ebp-14h]
    XModel *model; // [esp+4h] [ebp-10h]
    int size; // [esp+8h] [ebp-Ch]
    int i; // [esp+Ch] [ebp-8h]
    XModel **models; // [esp+10h] [ebp-4h]

    models = obj->localModels;
    for ( j = 0; j < obj->numModels; ++j )
    {
        model = models[j];
        size = model->numBones;
        for ( i = 0; i < size; ++i )
            *boneInfo++ = &model->localBoneInfo[i];
    }
}

int __cdecl DObjNumBones(const DObj *obj)
{
    return obj->numBones;
}

int __cdecl DObjGetLodForDist(const DObj *obj, int modelIndex, float dist, float base_dist)
{
    int lod; // [esp+Ch] [ebp-4h]

    lod = XModelGetLodForDist(obj->localModels[modelIndex], dist, base_dist, 0);
    if ( r_warm_dobj->current.enabled && lod < 0 )
        return 0;
    return lod;
}

void __cdecl DObjGetSetBones(const DObj *obj, int *setPartBits)
{
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 5; ++i )
        setPartBits[i] = obj->skel.partBits.anim[i];
}

unsigned int __cdecl DObjGetRootBoneCount(const DObj *obj)
{
    XModel *model; // [esp+0h] [ebp-4h]

    model = DObjGetModel(obj, 0);
    if ( !model->numRootBones
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp",
                    372,
                    0,
                    "%s",
                    "model->numRootBones") )
    {
        __debugbreak();
    }
    return model->numRootBones;
}

int __cdecl DObjSetRotTransIndex(const DObj *obj, const int *partBits, int boneIndex)
{
    DSkel *skel; // [esp+0h] [ebp-Ch]
    int boneIndexHigh; // [esp+4h] [ebp-8h]
    unsigned int boneIndexLow; // [esp+8h] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 394, 0, "%s", "obj") )
        __debugbreak();
    if ( !obj->skel.mat
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 395, 0, "%s", "obj->skel.mat") )
    {
        __debugbreak();
    }
    if ( boneIndex < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp",
                    396,
                    0,
                    "%s\n\t(boneIndex) = %i",
                    "(boneIndex >= 0)",
                    boneIndex) )
    {
        __debugbreak();
    }
    if ( boneIndex >= obj->numBones
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp",
                    397,
                    0,
                    "%s\n\t(boneIndex) = %i",
                    "(boneIndex < obj->numBones)",
                    boneIndex) )
    {
        __debugbreak();
    }
    boneIndexHigh = boneIndex >> 5;
    boneIndexLow = 0x80000000 >> (boneIndex & 0x1F);
    if ( (boneIndexLow & partBits[boneIndex >> 5]) == 0 )
        return 0;
    skel = (DSkel*)&obj->skel;
    if ( (boneIndexLow & obj->skel.partBits.anim[boneIndexHigh]) != 0 )
        return 0;
    if ( (boneIndexLow & skel->partBits.skel[boneIndexHigh]) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp",
                    414,
                    0,
                    "%s",
                    "!(skel->partBits.skel[boneIndexHigh] & boneIndexLow)") )
    {
        __debugbreak();
    }
    skel->partBits.anim[boneIndexHigh] |= boneIndexLow;
    return 1;
}

char __cdecl DObjSetSkelRotTransIndex(const DObj *obj, const int *partBits, int boneIndex)
{
    DSkel *skel; // [esp+0h] [ebp-Ch]
    int boneIndexHigh; // [esp+4h] [ebp-8h]
    unsigned int boneIndexLow; // [esp+8h] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 437, 0, "%s", "obj") )
        __debugbreak();
    if ( boneIndex < 0 || boneIndex >= obj->numBones )
        return 0;
    boneIndexHigh = boneIndex >> 5;
    boneIndexLow = 0x80000000 >> (boneIndex & 0x1F);
    if ( (boneIndexLow & partBits[boneIndex >> 5]) == 0 )
        return 1;
    skel = (DSkel*)&obj->skel;
    if ( (boneIndexLow & obj->skel.partBits.anim[boneIndexHigh]) != 0 )
        return 0;
    if ( (boneIndexLow & skel->partBits.skel[boneIndexHigh]) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp",
                    463,
                    0,
                    "%s",
                    "!(skel->partBits.skel[boneIndexHigh] & boneIndexLow)") )
    {
        __debugbreak();
    }
    skel->partBits.anim[boneIndexHigh] |= boneIndexLow;
    skel->partBits.skel[boneIndexHigh] |= boneIndexLow;
    return 1;
}

void __cdecl DObjSetControlTagAngles(const DObj *obj, int *partBits, unsigned int boneIndex, float *angles)
{
    if ( boneIndex < 0xFE )
    {
        if ( DObjSetControlRotTransIndex(obj, partBits, boneIndex) )
            DObjSetLocalTagInternal(obj, vec3_origin, angles, boneIndex);
    }
    else if ( boneIndex != 254
                 && boneIndex != 255
                 && !Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp",
                             483,
                             0,
                             "%s\n\t(boneIndex) = %i",
                             "((boneIndex == 254) || (boneIndex == 255))",
                             boneIndex) )
    {
        __debugbreak();
    }
}

XModel *__cdecl DObjGetModel(const DObj *obj, int modelIndex)
{
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 505, 0, "%s", "obj") )
        __debugbreak();
    if ( modelIndex < 0 || modelIndex >= obj->numModels )
        return 0;
    else
        return obj->localModels[modelIndex];
}

int __cdecl DObjGetModelBoneIndex(const DObj *obj, int model_index, unsigned int name, unsigned __int8 *index)
{
    int j; // [esp+0h] [ebp-14h]
    unsigned int boneIndex; // [esp+4h] [ebp-10h]
    XModel *model; // [esp+Ch] [ebp-8h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 569, 0, "%s", "obj") )
        __debugbreak();
    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 570, 0, "%s", "name") )
        __debugbreak();
    boneIndex = 0;
    for ( j = 0; j < obj->numModels; ++j )
    {
        model = obj->localModels[j];
        if ( j == model_index )
        {
            if ( XModelGetBoneIndex(model, name, boneIndex, index) )
                return 1;
            break;
        }
        boneIndex += model->numBones;
    }
    *index = -1;
    return 0;
}

void __cdecl DObjSetLocalTag(
                const DObj *obj,
                int *partBits,
                unsigned int boneIndex,
                const float *trans,
                const float *angles)
{
    if ( boneIndex < 0xFE )
    {
        if ( DObjSetRotTransIndex(obj, partBits, boneIndex) )
            DObjSetLocalTagInternal(obj, trans, angles, boneIndex);
    }
    else if ( boneIndex != 254
                 && boneIndex != 255
                 && !Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp",
                             613,
                             0,
                             "%s\n\t(boneIndex) = %i",
                             "((boneIndex == 254) || (boneIndex == 255))",
                             boneIndex) )
    {
        __debugbreak();
    }
}

void __cdecl DObjSetLocalTagInternal(const DObj *obj, const float *trans, const float *angles, int boneIndex)
{
    DObjAnimMat *rotTrans; // [esp+0h] [ebp-4h]
    DObjAnimMat *rotTransa; // [esp+0h] [ebp-4h]

    rotTrans = DObjGetRotTransArray(obj);
    if ( rotTrans )
    {
        rotTransa = &rotTrans[boneIndex];
        if ( angles )
            DObjSetAngles(rotTransa, angles);
        else
            DObjClearAngles(rotTransa);
        DObjSetTrans(rotTransa, trans);
    }
}

void __cdecl DObjSetAngles(DObjAnimMat *rotTrans, const float *angles)
{
    float v2; // [esp+8h] [ebp-44h]
    float v3; // [esp+14h] [ebp-38h]
    float v4; // [esp+20h] [ebp-2Ch]
    float yawQuat; // [esp+24h] [ebp-28h]
    float yawQuat_4; // [esp+28h] [ebp-24h]
    float rollQuat; // [esp+2Ch] [ebp-20h]
    float rollQuat_4; // [esp+30h] [ebp-1Ch]
    float pitchQuat; // [esp+34h] [ebp-18h]
    float pitchQuat_4; // [esp+38h] [ebp-14h]
    float tempQuat; // [esp+3Ch] [ebp-10h]

    v4 = angles[1] * 0.0087266462;
    yawQuat_4 = cos(v4);
    yawQuat = sin(v4);
    v3 = *angles * 0.0087266462;
    pitchQuat_4 = cos(v3);
    pitchQuat = sin(v3);
    v2 = angles[2] * 0.0087266462;
    rollQuat_4 = cos(v2);
    rollQuat = sin(v2);
    tempQuat = (-(pitchQuat)) * yawQuat;
    rotTrans->quat[0] = (float)(rollQuat * (float)(pitchQuat_4 * yawQuat_4)) + (float)(rollQuat_4 * tempQuat);
    rotTrans->quat[1] = (float)(rollQuat_4 * (float)(pitchQuat * yawQuat_4)) + (float)(rollQuat * (float)(pitchQuat_4 * yawQuat));
    rotTrans->quat[2] = (float)((-(rollQuat)) * (float)(pitchQuat * yawQuat_4)) + (float)(rollQuat_4 * (float)(pitchQuat_4 * yawQuat));
    rotTrans->quat[3] = (float)(rollQuat_4 * (float)(pitchQuat_4 * yawQuat_4)) - (float)(rollQuat * tempQuat);
}

void __cdecl DObjClearAngles(DObjAnimMat *rotTrans)
{
    rotTrans->quat[0] = 0.0f;
    rotTrans->quat[1] = 0.0f;
    rotTrans->quat[2] = 0.0f;
    rotTrans->quat[3] = 1.0f;
}

void __cdecl DObjSetTrans(DObjAnimMat *rotTrans, const float *trans)
{
    rotTrans->transWeight = 0.0f;
    rotTrans->trans[0] = *trans;
    rotTrans->trans[1] = trans[1];
    rotTrans->trans[2] = trans[2];
}

void __cdecl DObjCompleteHierarchyBits(DObj *obj, int *partBits)
{
    int j; // [esp+8h] [ebp-B4h]
    const unsigned __int8 *pos; // [esp+Ch] [ebp-B0h]
    int newBoneIndex; // [esp+10h] [ebp-ACh]
    int newBoneIndexa; // [esp+10h] [ebp-ACh]
    int newBoneIndexb; // [esp+10h] [ebp-ACh]
    const unsigned __int8 *modelParents; // [esp+14h] [ebp-A8h]
    const unsigned __int8 *duplicateParts; // [esp+18h] [ebp-A4h]
    int numModels; // [esp+1Ch] [ebp-A0h]
    XModel *subModel; // [esp+20h] [ebp-9Ch]
    int startIndex[33]; // [esp+24h] [ebp-98h]
    int localBoneIndex; // [esp+A8h] [ebp-14h]
    unsigned __int8 *parentList; // [esp+ACh] [ebp-10h]
    int objBoneIndex; // [esp+B0h] [ebp-Ch]
    const int *duplicatePartBits; // [esp+B4h] [ebp-8h]
    XModel **models; // [esp+B8h] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 767, 0, "%s", "obj") )
        __debugbreak();
    if ( !obj->numBones
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 768, 0, "%s", "obj->numBones > 0") )
    {
        __debugbreak();
    }
    objBoneIndex = obj->numBones - 1;
    numModels = obj->numModels;
    if ( !obj->numModels
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 773, 0, "%s", "numModels > 0") )
    {
        __debugbreak();
    }
    if ( !obj->duplicateParts
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp",
                    775,
                    0,
                    "%s",
                    "obj->duplicateParts") )
    {
        __debugbreak();
    }
    duplicatePartBits = (const int *)SL_ConvertToString(obj->duplicateParts, SCRIPTINSTANCE_SERVER);
    duplicateParts = (const unsigned __int8 *)(duplicatePartBits + 5);
    newBoneIndex = 0;
    subModel = 0;
    models = obj->localModels;
    modelParents = (const unsigned __int8 *)&models[numModels];
    for ( j = 0; j < numModels; ++j )
    {
        startIndex[j] = newBoneIndex;
        subModel = models[j];
        newBoneIndex = startIndex[j] + subModel->numBones;
        if ( newBoneIndex > objBoneIndex )
            break;
    }
    if ( j == numModels
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 797, 0, "%s", "j != numModels") )
    {
        __debugbreak();
    }
    *partBits |= 0x80000000;
    parentList = subModel->localParentList;
    if ( IKImport_IsIKEntity((unsigned __int8 *)obj) )
        IKImport_SetIKPartBits(obj, partBits);
    while ( 1 )
    {
        while ( 1 )
        {
            localBoneIndex = objBoneIndex - startIndex[j];
            if ( localBoneIndex < 0 )
                break;
            if ( (partBits[objBoneIndex >> 5] & (0x80000000 >> (objBoneIndex & 0x1F))) != 0 )
            {
                if ( (duplicatePartBits[objBoneIndex >> 5] & (0x80000000 >> (objBoneIndex & 0x1F))) != 0 )
                {
                    for ( pos = duplicateParts; ; pos += 2 )
                    {
                        if ( !*pos
                            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 891, 0, "%s", "*pos") )
                        {
                            __debugbreak();
                        }
                        if ( objBoneIndex == *pos - 1 )
                            break;
                    }
                    newBoneIndexb = pos[1] - 1;
                    goto LABEL_40;
                }
                newBoneIndexa = localBoneIndex - subModel->numRootBones;
                if ( newBoneIndexa >= 0 )
                {
                    newBoneIndexb = objBoneIndex - parentList[newBoneIndexa];
                    goto LABEL_40;
                }
                newBoneIndexb = modelParents[j];
                if ( newBoneIndexb == 255 )
                {
                    --objBoneIndex;
                }
                else
                {
LABEL_40:
                    if ( newBoneIndexb >= (unsigned int)obj->numBones
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp",
                                    904,
                                    0,
                                    "%s",
                                    "(unsigned)newBoneIndex < obj->numBones") )
                    {
                        __debugbreak();
                    }
                    partBits[newBoneIndexb >> 5] |= 0x80000000 >> (newBoneIndexb & 0x1F);
                    --objBoneIndex;
                }
            }
            else
            {
                --objBoneIndex;
            }
        }
        if ( --j < 0 )
            break;
        subModel = models[j];
        parentList = subModel->localParentList;
    }
}

int __cdecl DObjSetControlRotTransIndex(const DObj *obj, const int *partBits, int boneIndex)
{
    DSkel *skel; // [esp+0h] [ebp-Ch]
    int boneIndexHigh; // [esp+4h] [ebp-8h]
    unsigned int boneIndexLow; // [esp+8h] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 930, 0, "%s", "obj") )
        __debugbreak();
    if ( !obj->skel.mat
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 931, 0, "%s", "obj->skel.mat") )
    {
        __debugbreak();
    }
    if ( boneIndex < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 932, 0, "%s", "boneIndex >= 0") )
    {
        __debugbreak();
    }
    if ( boneIndex >= obj->numBones
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp",
                    933,
                    0,
                    "%s",
                    "boneIndex < obj->numBones") )
    {
        __debugbreak();
    }
    boneIndexHigh = boneIndex >> 5;
    boneIndexLow = 0x80000000 >> (boneIndex & 0x1F);
    if ( (boneIndexLow & partBits[boneIndex >> 5]) == 0 )
        return 0;
    skel = (DSkel*)&obj->skel;
    if ( (boneIndexLow & obj->skel.partBits.anim[boneIndexHigh]) != 0 )
        return 0;
    if ( (boneIndexLow & skel->partBits.skel[boneIndexHigh]) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp",
                    951,
                    0,
                    "%s",
                    "!(skel->partBits.skel[boneIndexHigh] & boneIndexLow)") )
    {
        __debugbreak();
    }
    skel->partBits.control[boneIndexHigh] |= boneIndexLow;
    skel->partBits.anim[boneIndexHigh] |= boneIndexLow;
    return 1;
}

bool __cdecl DObjSkelExists(const DObj *obj, int timeStamp)
{
    if ( obj->skel.timeStamp == timeStamp )
        return obj->skel.mat != 0;
    memset((void *)&obj->skel, 0, sizeof(obj->skel));
    return 0;
}

void __cdecl DObjClearSkel(const DObj *obj)
{
    if ( !obj->locked
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 1001, 0, "%s", "obj->locked") )
    {
        __debugbreak();
    }
    memset((void *)&obj->skel, 0, 0x3Cu);
}

int __cdecl DObjSkelAreBonesUpToDate(const DObj *obj, int *partBits)
{
    int i; // [esp+4h] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 1019, 0, "%s", "obj") )
        __debugbreak();
    if ( obj == (const DObj *)-20
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 1022, 0, "%s", "skel") )
    {
        __debugbreak();
    }
    for ( i = 0; i < 5; ++i )
    {
        if ( (partBits[i] & ~obj->skel.partBits.skel[i]) != 0 )
            return 0;
    }
    return 1;
}

void __cdecl DObjSkelSetSkel(const DObj *obj, int *partBits)
{
    DSkel *skel; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 1051, 0, "%s", "obj") )
        __debugbreak();
    skel = (DSkel*)&obj->skel;
    if ( obj == (const DObj *)-20
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 1054, 0, "%s", "skel") )
    {
        __debugbreak();
    }
    for ( i = 0; i < 5; ++i )
    {
        skel->partBits.skel[i] |= partBits[i];
        skel->partBits.anim[i] |= partBits[i];
    }
}

int __cdecl DObjGetAllocSkelSize(const DObj *obj)
{
    return 32 * obj->numBones;
}

void __cdecl DObjCreateSkel(DObj *obj, char *buf, int timeStamp)
{
    int i; // [esp+0h] [ebp-4h]

    obj->skel.mat = (DObjAnimMat *)buf;
    obj->skel.timeStamp = timeStamp;
    for ( i = 0; i < 5; ++i )
    {
        if ( obj->skel.partBits.anim[i]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp",
                        1103,
                        0,
                        "%s",
                        "!obj->skel.partBits.anim[i]") )
        {
            __debugbreak();
        }
        if ( obj->skel.partBits.control[i]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp",
                        1104,
                        0,
                        "%s",
                        "!obj->skel.partBits.control[i]") )
        {
            __debugbreak();
        }
        if ( obj->skel.partBits.skel[i] )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp",
                            1105,
                            0,
                            "%s",
                            "!obj->skel.partBits.skel[i]") )
                __debugbreak();
        }
    }
}

DObjAnimMat *__cdecl I_dmaGetDObjSkel(const DObj *obj)
{
    if ( !obj->skel.mat
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 1123, 0, "%s", "obj->skel.mat") )
    {
        __debugbreak();
    }
    return obj->skel.mat;
}

void __cdecl DObjGetHidePartBits(const DObj *obj, unsigned int *partBits)
{
    *partBits = obj->hidePartBits[0];
    partBits[1] = obj->hidePartBits[1];
    partBits[2] = obj->hidePartBits[2];
    partBits[3] = obj->hidePartBits[3];
    partBits[4] = obj->hidePartBits[4];
}

int __cdecl DObjGetBoneIndex(const DObj *obj, unsigned int name, unsigned __int8 *index, int modelNum)
{
    int j; // [esp+0h] [ebp-18h]
    int ja; // [esp+0h] [ebp-18h]
    unsigned int boneIndex; // [esp+4h] [ebp-14h]
    int numModels; // [esp+8h] [ebp-10h]
    XModel *model; // [esp+Ch] [ebp-Ch]
    XModel *modela; // [esp+Ch] [ebp-Ch]
    unsigned int localBoneIndex; // [esp+10h] [ebp-8h]
    XModel **models; // [esp+14h] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 1180, 0, "%s", "obj") )
        __debugbreak();
    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 1181, 0, "%s", "name") )
        __debugbreak();
    localBoneIndex = *index;
    if ( localBoneIndex == 255 )
        return 0;
    numModels = obj->numModels;
    models = obj->localModels;
    if ( localBoneIndex < obj->numBones )
    {
        for ( j = 0; j < numModels; ++j )
        {
            model = models[j];
            if ( localBoneIndex < model->numBones )
            {
                if ( name != model->localBoneNames[localBoneIndex] )
                    break;
                return 1;
            }
            localBoneIndex -= model->numBones;
        }
    }
    boneIndex = 0;
    for ( ja = 0; ja < numModels; ++ja )
    {
        modela = models[ja];
        if ( (modelNum == -1 || modelNum == ja) && XModelGetBoneIndex(modela, name, boneIndex, index) )
            return 1;
        boneIndex += modela->numBones;
    }
    *index = -1;
    return 0;
}

int __cdecl DObjGetChildBones(const DObj *obj, unsigned __int8 parentBone, unsigned __int8 *children, int maxChildren)
{
    unsigned __int8 child_index; // [esp+7h] [ebp-1Dh]
    int j; // [esp+8h] [ebp-1Ch]
    unsigned __int8 modelBoneIndex; // [esp+Fh] [ebp-15h]
    XModel *model; // [esp+14h] [ebp-10h]
    int numChildBones; // [esp+18h] [ebp-Ch]
    unsigned __int8 childBoneIndexStart; // [esp+1Fh] [ebp-5h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 1303, 0, "%s", "obj") )
        __debugbreak();
    if ( parentBone >= (int)obj->numBones
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp",
                    1304,
                    0,
                    "%s",
                    "parentBone < obj->numBones") )
    {
        __debugbreak();
    }
    if ( !children
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_utils.cpp", 1305, 0, "%s", "children") )
    {
        __debugbreak();
    }
    numChildBones = 0;
    modelBoneIndex = parentBone;
    childBoneIndexStart = 0;
    for ( j = 0; ; ++j )
    {
        if ( j >= obj->numModels )
            return 0;
        model = obj->localModels[j];
        if ( modelBoneIndex < (int)model->numBones )
            break;
        modelBoneIndex -= model->numBones;
        childBoneIndexStart += model->numBones;
    }
    for ( child_index = 0; child_index < (int)model->numBones; ++child_index )
    {
        if ( modelBoneIndex == child_index - model->localParentList[child_index - model->numRootBones] )
        {
            children[numChildBones] = child_index + childBoneIndexStart;
            if ( ++numChildBones == maxChildren )
                return numChildBones;
        }
    }
    return numChildBones;
}

void __cdecl DObjLock(const DObj *obj)
{
    volatile unsigned int *p_locked; // [esp+0h] [ebp-4h]

    p_locked = &(((DObj*)obj)->locked);
    do
    {
        while ( *p_locked )
            ;
    }
    while ( _InterlockedCompareExchange(p_locked, 1, 0) );
}

void __cdecl DObjUnlock(const DObj *obj)
{
    iassert(obj->locked);

    //obj->locked = 0;
    ((DObj *)obj)->locked = 0;
}

bool __cdecl DObjIsRecorded(const DObj *obj)
{
    return (obj->flags & 4) != 0;
}

