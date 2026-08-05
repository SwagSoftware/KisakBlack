#include "dobj.h"
#include <clientscript/cscr_stringlist.h>
#include <qcommon/common.h>
#include "xmodel_utils.h"
#include "xmodel.h"
#include "xanim.h"

#include <bgame/bg_local.h>
#include <cstring>
#include <ik/ik_import.h>
#include <clientscript/cscr_memorytree.h>
#include <universal/com_math_anglevectors.h>
#include "xmodel_load_obj.h"
#include "dobj_utils.h"
#include <cgame_mp/cg_pose_mp.h>
#include <cgame/cg_drawtools.h>
#include "dobj_skel.h"

unsigned int g_empty;

void __cdecl DObjInit()
{
    int duplicatePartBits[6]; // [esp+0h] [ebp-18h] BYREF

    memset(duplicatePartBits, 0, sizeof(duplicatePartBits));
    g_empty = SL_GetStringOfSize(SCRIPTINSTANCE_SERVER, (char *)duplicatePartBits, 0, 0x15u, 12);
}

void __cdecl DObjShutdown()
{
    if ( g_empty )
    {
        SL_RemoveRefToStringOfSize(SCRIPTINSTANCE_SERVER, g_empty, 0x15u);
        g_empty = 0;
    }
}

void __cdecl DObjDumpInfo(const DObj *obj)
{
    char *BoneName; // eax
    char *v2; // eax
    int v3; // [esp-8h] [ebp-28h]
    char *v4; // [esp-4h] [ebp-24h]
    int j; // [esp+0h] [ebp-20h]
    int numBones; // [esp+4h] [ebp-1Ch]
    const unsigned __int8 *pos; // [esp+8h] [ebp-18h]
    int boneIndex; // [esp+Ch] [ebp-14h]
    int numModels; // [esp+10h] [ebp-10h]
    XModel *model; // [esp+14h] [ebp-Ch]
    int i; // [esp+18h] [ebp-8h]
    XModel **models; // [esp+1Ch] [ebp-4h]

    if ( obj )
    {
        Com_Printf(19, "\nModels:\n");
        numModels = obj->numModels;
        boneIndex = 0;
        models = obj->localModels;
        for ( j = 0; j < numModels; ++j )
        {
            model = models[j];
            Com_Printf(19, "%d: '%s'\n", boneIndex, model->name);
            boneIndex += XModelNumBones(model);
        }
        Com_Printf(19, "\nBones:\n");
        numBones = obj->numBones;
        for ( i = 0; i < numBones; ++i )
        {
            BoneName = DObjGetBoneName(obj, i);
            Com_Printf(19, "Bone %d: '%s'\n", i, BoneName);
        }
        if ( obj->duplicateParts )
        {
            Com_Printf(19, "\nPart duplicates:\n");
            for ( pos = (const unsigned __int8 *)(SL_ConvertToString(obj->duplicateParts, SCRIPTINSTANCE_SERVER) + 20);
                        *pos;
                        pos += 2 )
            {
                v4 = DObjGetBoneName(obj, pos[1] - 1);
                v3 = pos[1] - 1;
                v2 = DObjGetBoneName(obj, *pos - 1);
                Com_Printf(19, "%d ('%s') -> %d ('%s')\n", *pos - 1, v2, v3, v4);
            }
        }
        else
        {
            Com_Printf(19, "\nNo part duplicates.\n");
        }
        Com_Printf(19, "\n");
    }
    else
    {
        Com_Printf(19, "No Dobj\n");
    }
}

bool __cdecl DObjIgnoreCollision(const DObj *obj, char modelIndex)
{
    return (obj->ignoreCollision & (1 << modelIndex)) != 0;
}

void __cdecl DObjGetHierarchyBits(DObj *obj, int boneIndex, int *partBits)
{
    int i; // [esp+0h] [ebp-BCh]
    int j; // [esp+4h] [ebp-B8h]
    const unsigned __int8 *pos; // [esp+8h] [ebp-B4h]
    int newBoneIndex; // [esp+Ch] [ebp-B0h]
    int newBoneIndexa; // [esp+Ch] [ebp-B0h]
    const unsigned __int8 *modelParents; // [esp+10h] [ebp-ACh]
    const unsigned __int8 *duplicateParts; // [esp+14h] [ebp-A8h]
    unsigned int bit; // [esp+18h] [ebp-A4h]
    int numModels; // [esp+1Ch] [ebp-A0h]
    XModel *subModel; // [esp+20h] [ebp-9Ch]
    int startIndex[33]; // [esp+24h] [ebp-98h]
    int localBoneIndex; // [esp+A8h] [ebp-14h]
    unsigned __int8 *parentList; // [esp+ACh] [ebp-10h]
    const int *duplicatePartBits; // [esp+B0h] [ebp-Ch]
    XModel **models; // [esp+B4h] [ebp-8h]
    int highBoneIndex; // [esp+B8h] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 490, 0, "%s", "obj") )
        __debugbreak();
    if ( boneIndex >= (unsigned int)obj->numBones
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                    492,
                    0,
                    "boneIndex doesn't index obj->numBones\n\t%i not in [0, %i)",
                    boneIndex,
                    obj->numBones) )
    {
        __debugbreak();
    }
    for ( i = 0; i < 5; ++i )
        partBits[i] = 0;
    numModels = obj->numModels;
    if ( !obj->numModels
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 498, 0, "%s", "numModels > 0") )
    {
        __debugbreak();
    }
    if ( !obj->duplicateParts
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 500, 0, "%s", "obj->duplicateParts") )
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
        if ( newBoneIndex > boneIndex )
            break;
    }
    if ( j != numModels )
    {
        *partBits = 0x80000000;
        parentList = subModel->localParentList;
        if ( IKImport_IsIKEntity((unsigned __int8 *)obj) )
            IKImport_SetIKPartBits(obj, partBits);
LABEL_23:
        localBoneIndex = boneIndex - startIndex[j];
        while ( 1 )
        {
            if ( localBoneIndex < 0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 556, 0, "%s", "localBoneIndex >= 0") )
            {
                __debugbreak();
            }
            bit = 0x80000000 >> (boneIndex & 0x1F);
            highBoneIndex = boneIndex >> 5;
            partBits[boneIndex >> 5] |= bit;
            if ( (bit & duplicatePartBits[highBoneIndex]) != 0 )
            {
                for ( pos = duplicateParts; ; pos += 2 )
                {
                    if ( !*pos && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 587, 0, "%s", "*pos") )
                        __debugbreak();
                    if ( boneIndex == *pos - 1 )
                        break;
                }
                boneIndex = pos[1] - 1;
            }
            else
            {
                newBoneIndexa = localBoneIndex - subModel->numRootBones;
                if ( newBoneIndexa >= 0 )
                {
                    boneIndex -= parentList[newBoneIndexa];
                    goto LABEL_23;
                }
                boneIndex = modelParents[j];
                if ( boneIndex == 255 )
                    return;
            }
            do
            {
                if ( --j < 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                                601,
                                0,
                                "%s\n\t(j) = %i",
                                "(j >= 0)",
                                j) )
                {
                    __debugbreak();
                }
                localBoneIndex = boneIndex - startIndex[j];
            }
            while ( localBoneIndex < 0 );
            subModel = models[j];
            parentList = subModel->localParentList;
        }
    }
}

bool __cdecl DObjSkelIsBoneUpToDate(DObj *obj, int boneIndex)
{
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 630, 0, "%s", "obj") )
        __debugbreak();
    if ( obj == (DObj *)-20
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 633, 0, "%s", "skel") )
    {
        __debugbreak();
    }
    return (obj->skel.partBits.skel[boneIndex >> 5] & (0x80000000 >> (boneIndex & 0x1F))) != 0;
}

void __cdecl DObjSetTree(DObj *obj, XAnimTree_s *tree)
{
    int savedregs; // [esp+0h] [ebp+0h] BYREF

    obj->localTree = tree;
    if ( tree )
    {
        if ( tree->children )
            XAnimResetAnimMap(obj, tree->children);
        tree->inst = !DObjIsServer(obj);
    }
}

void __cdecl DObjCreate(
                DObjModel_s *dobjModels,
                unsigned int numModels,
                XAnimTree_s *tree,
                unsigned __int8 *buf,
                __int16 entnum)
{
    if ( !dobjModels
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 751, 0, "%s", "dobjModels") )
    {
        __debugbreak();
    }
    if ( !numModels
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 752, 0, "%s", "numModels > 0") )
    {
        __debugbreak();
    }
    if ( numModels > 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                    753,
                    0,
                    "%s",
                    "(unsigned)numModels <= DOBJ_MAX_SUBMODELS") )
    {
        __debugbreak();
    }
    if ( !buf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 756, 0, "%s", "obj") )
        __debugbreak();
    memset(buf + 20, 0, 0x44u);
    buf[8] = 0;
    *((_WORD *)buf + 2) = 0;
    *((unsigned int *)buf + 3) = 0;
    *((unsigned int *)buf + 4) = 0;
    *((_WORD *)buf + 3) = entnum;
    *((unsigned int *)buf + 23) = 0;
    *((unsigned int *)buf + 24) = 0;
    *((unsigned int *)buf + 25) = 0;
    *((unsigned int *)buf + 26) = 0;
    *((unsigned int *)buf + 27) = 0;
    DObjCreateDuplicateParts((DObj *)buf, dobjModels, numModels);
    DObjComputeBounds((DObj *)buf);
    DObjSetTree((DObj *)buf, tree);
    buf[112] = -1;
    *((unsigned int *)buf + 29) = 0;
    buf[113] = 0;
}

void __cdecl DObjCreateDuplicateParts(DObj *obj, DObjModel_s *dobjModels, unsigned int numModels)
{
    int numBones; // [esp+0h] [ebp-6BCh]
    unsigned __int8 modelParents[32]; // [esp+4h] [ebp-6B8h] BYREF
    int boneIndex; // [esp+28h] [ebp-694h]
    DObjModel_s *dobjModel; // [esp+2Ch] [ebp-690h]
    unsigned __int8 *duplicateParts; // [esp+30h] [ebp-68Ch]
    bool bRootMeld; // [esp+37h] [ebp-685h]
    int boneCount; // [esp+38h] [ebp-684h]
    XModel *model; // [esp+3Ch] [ebp-680h]
    unsigned int currNumModels; // [esp+40h] [ebp-67Ch]
    unsigned int name; // [esp+44h] [ebp-678h]
    int len; // [esp+48h] [ebp-674h]
    unsigned int size; // [esp+4Ch] [ebp-670h]
    unsigned __int8 parentIndex; // [esp+53h] [ebp-669h] BYREF
    int localBoneIndex; // [esp+54h] [ebp-668h]
    int index; // [esp+58h] [ebp-664h]
    int duplicatePartBits[341]; // [esp+5Ch] [ebp-660h] BYREF
    int matOffset[32]; // [esp+5B4h] [ebp-108h]
    XModel *models[32]; // [esp+634h] [ebp-88h] BYREF
    int modelIndex; // [esp+6B4h] [ebp-8h]
    unsigned __int16 *boneNames; // [esp+6B8h] [ebp-4h]

    duplicateParts = (unsigned __int8 *)&duplicatePartBits[5];
    memset(duplicatePartBits, 0, 20);
    len = 0;
    boneCount = 0;
    dobjModel = dobjModels;
    currNumModels = 0;
    while (currNumModels < numModels)
    {
        boneIndex = boneCount;
        model = dobjModel->model;
        iassert(model);
        boneCount += XModelNumBones(model);
        if (boneCount > 160)
        {
            iassert(currNumModels);
            DObjDumpCreationInfo(dobjModels, numModels);
            Com_Error(ERR_DROP, "dobj for xmodel ',27h,'%s',27h,' has more than %d bones (see console for details)", models[0]->name, 160);
        }
        models[currNumModels] = model;
        modelParents[currNumModels] = -1;
        matOffset[currNumModels] = boneIndex;
        if (dobjModel->ignoreCollision)
            obj->ignoreCollision |= 1 << currNumModels;
        if (currNumModels)
        {
            name = dobjModel->boneName;
            if (name && *SL_ConvertToString(name, SCRIPTINSTANCE_SERVER))
            {
                for (modelIndex = currNumModels - 1; modelIndex >= 0; --modelIndex)
                {
                    if (XModelGetBoneIndex(models[modelIndex], name, matOffset[modelIndex], &modelParents[currNumModels]))
                        goto LABEL_2;
                }

                iassert(currNumModels);
                Com_PrintWarning(19, "WARNING: Part '%s' not found in model '%s' or any of its descendants\n", SL_ConvertToString(name, SCRIPTINSTANCE_SERVER), models[0]->name);
                iassert(modelParents[currNumModels] == NO_BONEINDEX);
            }
            else
            {
                numBones = model->numBones;
                boneNames = model->localBoneNames;
                bRootMeld = 0;
                for (localBoneIndex = 0; localBoneIndex < numBones; ++localBoneIndex)
                {
                    name = boneNames[localBoneIndex];
                    for (modelIndex = currNumModels - 1; modelIndex >= 0; --modelIndex)
                    {
                        if (XModelGetBoneIndex(models[modelIndex], name, matOffset[modelIndex], &parentIndex))
                        {
                            iassert(parentIndex != 255);
                            iassert(parentIndex != 254);
                            iassert(parentIndex < boneIndex + localBoneIndex);
     
                            if (!localBoneIndex)
                                bRootMeld = 1;
                            index = localBoneIndex + boneIndex;
                            duplicateParts[len] = localBoneIndex + boneIndex + 1;
                            duplicatePartBits[index >> 5] |= 0x80000000 >> (index & 0x1F);
                            iassert(duplicateParts[len]);
                   
                            duplicateParts[++len] = parentIndex + 1;

                            iassert(duplicateParts[len]);
            
                            ++len;
                            break;
                        }
                    }
                }
                if (!bRootMeld)
                {
                    iassert(currNumModels);
                    Com_PrintWarning(
                        19,
                        "WARNING: Attempting to meld model, but root part '%s' of model '%s' not found in model '%s' or any of its descendants\n",
                        SL_ConvertToString(*boneNames, SCRIPTINSTANCE_SERVER),
                        model->name,
                        models[0]->name);
                }
            }
        }
    LABEL_2:
        ++currNumModels;
        ++dobjModel;
    }
    if (boneCount != (unsigned __int8)boneCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
            395,
            0,
            "%s",
            "boneCount == (byte)boneCount"))
    {
        __debugbreak();
    }
    obj->numBones = boneCount;
    if (!numModels
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 398, 0, "%s", "numModels > 0"))
    {
        __debugbreak();
    }
    obj->localModels = (XModel **)MT_Alloc(5 * numModels, 13, SCRIPTINSTANCE_SERVER);
    memcpy((unsigned __int8 *)obj->localModels, (unsigned __int8 *)models, 4 * numModels);
    memcpy((unsigned __int8 *)&obj->localModels[numModels], modelParents, numModels);
    if (numModels != (unsigned __int8)numModels
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
            410,
            0,
            "%s",
            "numModels == (byte)numModels"))
    {
        __debugbreak();
    }
    obj->numModels = numModels;
    if (!g_empty && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 419, 0, "%s", "g_empty"))
        __debugbreak();
    if (obj->duplicateParts
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 420, 0, "%s", "!obj->duplicateParts"))
    {
        __debugbreak();
    }
    if (len)
    {
        duplicateParts[len] = 0;
        size = ++len + 20;
        obj->duplicatePartsSize = len + 20;
        if (obj->duplicatePartsSize != size
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                434,
                0,
                "%s",
                "obj->duplicatePartsSize == size"))
        {
            __debugbreak();
        }
        obj->duplicateParts = SL_GetStringOfSize(
            SCRIPTINSTANCE_SERVER,
            (char *)duplicatePartBits,
            0,
            obj->duplicatePartsSize,
            12);
    }
    else
    {
        obj->duplicatePartsSize = 21;
        obj->duplicateParts = g_empty;
    }
}

void __cdecl DObjDumpCreationInfo(DObjModel_s *dobjModels, unsigned int numModels)
{
    const char *Name; // eax
    char *v3; // eax
    unsigned int j; // [esp+0h] [ebp-14h]
    int numBones; // [esp+4h] [ebp-10h]
    unsigned int boneIndex; // [esp+8h] [ebp-Ch]
    XModel *model; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    boneIndex = 0;
    for ( j = 0; j < numModels; ++j )
    {
        model = dobjModels[j].model;
        Name = XModelGetName(model);
        Com_Printf(19, "Model '%s':\n", Name);
        numBones = XModelNumBones(model);
        for ( i = 0; i < numBones; ++i )
        {
            v3 = SL_ConvertToString(model->localBoneNames[i], SCRIPTINSTANCE_SERVER);
            Com_Printf(19, "Bone %d: '%s'\n", boneIndex++, v3);
        }
    }
}

void __cdecl DObjComputeBounds(DObj *obj)
{
    float modelRadius; // [esp+Ch] [ebp-28h]
    unsigned __int8 boneIndex; // [esp+13h] [ebp-21h]
    XModel *model; // [esp+14h] [ebp-20h]
    unsigned int j; // [esp+18h] [ebp-1Ch]
    const unsigned __int8 *modelParents; // [esp+1Ch] [ebp-18h]
    unsigned int numModels; // [esp+20h] [ebp-14h]
    const DObjAnimMat *boneMtxList; // [esp+24h] [ebp-10h]
    float maxRadius; // [esp+28h] [ebp-Ch]
    XModel **models; // [esp+2Ch] [ebp-8h]
    unsigned __int8 baseModelNumBones; // [esp+33h] [ebp-1h]

    maxRadius = 0.0f;
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 675, 0, "%s", "obj") )
        __debugbreak();
    numModels = obj->numModels;
    models = obj->localModels;
    if ( !obj->numModels )
    {
        obj->radius = 0.0f;
        return;
    }
    modelParents = (const unsigned __int8 *)&obj->localModels[obj->numModels];
    baseModelNumBones = (*models)->numBones;
    boneMtxList = XModelGetBasePose(*models);
    for ( j = 0; j < numModels; ++j )
    {
        model = models[j];
        if ( !model && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 697, 0, "%s", "model") )
            __debugbreak();
        modelRadius = XModelGetRadius(model);
        boneIndex = modelParents[j];
        if ( boneIndex != 255 )
        {
            if ( boneIndex >= (int)baseModelNumBones )
                continue;
            modelRadius = Vec3Length(boneMtxList[boneIndex].trans) + modelRadius;
        }
        if ( modelRadius > maxRadius )
            maxRadius = modelRadius;
    }
    obj->radius = maxRadius;
    if ( (LODWORD(obj->radius) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 728, 0, "%s", "!IS_NAN(obj->radius)") )
    {
        __debugbreak();
    }
}

void __cdecl DObjCreateExt(
                DObjModel_s *dobjModels,
                unsigned int numModels,
                XAnimTree_s *tree,
                unsigned __int8 *buf,
                __int16 entnum,
                bool isServer,
                bool isLocalPlayer,
                unsigned __int8 localClientIndex)
{
    if ( !buf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 807, 0, "%s", "obj") )
        __debugbreak();
    DObjCreate(dobjModels, numModels, tree, buf, entnum);
    if ( isServer )
        DObjSetFlag((DObj *)buf, 1u, 1);
    if ( isLocalPlayer )
        DObjSetFlag((DObj *)buf, 2u, 1);
    buf[112] = localClientIndex;
    if ( tree )
        tree->inst = !DObjIsServer((const DObj *)buf);
    *((unsigned int *)buf + 29) = 0;
}

void __cdecl DObjFree(DObj *obj)
{
    unsigned __int16 entnum; // [esp+4h] [ebp-8h]
    XModel **models; // [esp+8h] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 895, 0, "%s", "obj") )
        __debugbreak();
    entnum = obj->entnum;
    models = obj->localModels;
    if ( models )
    {
        MT_Free((unsigned char*)models, 5 * obj->numModels, SCRIPTINSTANCE_SERVER);
        obj->localModels = 0;
    }
    obj->numModels = 0;
    if ( obj->localTree )
    {
        if ( !obj->localTree->anims
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 916, 0, "%s", "tree->anims") )
        {
            __debugbreak();
        }
        obj->localTree = 0;
    }
    if ( !g_empty && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 922, 0, "%s", "g_empty") )
        __debugbreak();
    if ( obj->duplicateParts )
    {
        if ( obj->duplicateParts != g_empty )
            SL_RemoveRefToStringOfSize(SCRIPTINSTANCE_SERVER, obj->duplicateParts, obj->duplicatePartsSize);
        obj->duplicatePartsSize = 0;
        obj->duplicateParts = 0;
    }
    memset(obj, 0x77u, sizeof(DObj));
    obj->entnum = entnum;
}

void __cdecl DObjGetCreateParms(
                const DObj *obj,
                DObjModel_s *dobjModels,
                unsigned __int16 *numModels,
                XAnimTree_s **tree,
                unsigned __int16 *entnum)
{
    const unsigned __int8 *modelParents; // [esp+0h] [ebp-A8h]
    DObjModel_s *dobjModel; // [esp+4h] [ebp-A4h]
    int boneIndex; // [esp+8h] [ebp-A0h]
    XModel *model; // [esp+Ch] [ebp-9Ch]
    int startBoneIndex; // [esp+10h] [ebp-98h]
    int parentModelIndex; // [esp+14h] [ebp-94h]
    int matOffset[33]; // [esp+18h] [ebp-90h]
    XModel **models; // [esp+9Ch] [ebp-Ch]
    int modelIndex; // [esp+A0h] [ebp-8h]
    unsigned __int16 *boneNames; // [esp+A4h] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 967, 0, "%s", "obj") )
        __debugbreak();
    if ( (!obj->numModels || obj->numModels > 0x20u)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                    968,
                    0,
                    "%s",
                    "obj->numModels > 0 && obj->numModels <= DOBJ_MAX_SUBMODELS") )
    {
        __debugbreak();
    }
    if ( !dobjModels
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 969, 0, "%s", "dobjModels") )
    {
        __debugbreak();
    }
    if ( !numModels && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 970, 0, "%s", "numModels") )
        __debugbreak();
    if ( !tree && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 971, 0, "%s", "tree") )
        __debugbreak();
    *numModels = obj->numModels;
    *tree = obj->localTree;
    *entnum = obj->entnum;
    startBoneIndex = 0;
    models = obj->localModels;
    modelParents = (const unsigned __int8 *)&models[obj->numModels];
    modelIndex = 0;
    dobjModel = dobjModels;
    while ( modelIndex < obj->numModels )
    {
        model = models[modelIndex];
        matOffset[modelIndex] = startBoneIndex;
        startBoneIndex += XModelNumBones(model);
        dobjModel->model = models[modelIndex];
        dobjModel->boneName = 0;
        dobjModel->ignoreCollision = (obj->ignoreCollision & (1 << modelIndex)) != 0;
        if ( modelParents[modelIndex] != 255 )
        {
            for ( parentModelIndex = modelIndex - 1; parentModelIndex >= 0; --parentModelIndex )
            {
                if ( modelParents[modelIndex] >= matOffset[parentModelIndex] )
                {
                    boneIndex = modelParents[modelIndex] - matOffset[parentModelIndex];
                    if ( boneIndex >= XModelNumBones(models[parentModelIndex])
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                                    999,
                                    0,
                                    "%s",
                                    "boneIndex < XModelNumBones( models[parentModelIndex] )") )
                    {
                        __debugbreak();
                    }
                    boneNames = XModelBoneNames(models[parentModelIndex]);
                    dobjModel->boneName = boneNames[boneIndex];
                    break;
                }
            }
        }
        ++modelIndex;
        ++dobjModel;
    }
}

void __cdecl DObjCalcBounds(DObj *dobj, float *mins, float *maxs)
{
    int i; // [esp+8h] [ebp-68h]
    float tempBoneMtx[4][3]; // [esp+Ch] [ebp-64h] BYREF
    const DObjAnimMat *worldBoneMtx; // [esp+3Ch] [ebp-34h]
    const DObjAnimMat *boneMtxList; // [esp+40h] [ebp-30h]
    unsigned __int8 boneIndex; // [esp+47h] [ebp-29h]
    float lmaxs[3]; // [esp+48h] [ebp-28h] BYREF
    float lmins[3]; // [esp+54h] [ebp-1Ch] BYREF
    unsigned int j; // [esp+60h] [ebp-10h]
    const unsigned __int8 *modelParents; // [esp+64h] [ebp-Ch]
    unsigned int numModels; // [esp+68h] [ebp-8h]
    XModel *model; // [esp+6Ch] [ebp-4h]

    numModels = dobj->numModels;
    if ( !numModels && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 1019, 0, "%s", "numModels") )
        __debugbreak();
    model = *dobj->localModels;
    *mins = -4.0f;
    mins[1] = -4.0f;
    mins[2] = 0.0f;
    *maxs = 4.0f;
    maxs[1] = 4.0f;
    maxs[2] = 10.0f;
    modelParents = (const unsigned __int8 *)&dobj->localModels[dobj->numModels];
    for ( j = 0; j < numModels; ++j )
    {
        boneIndex = modelParents[j];
        if ( !j || boneIndex == 255 )
        {
            XModelGetBounds(model, lmins, lmaxs);
        }
        else
        {
            if ( boneIndex >= (int)model->numBones )
                continue;
            boneMtxList = XModelGetBasePose(model);
            worldBoneMtx = &boneMtxList[boneIndex];
            QuatToAxis(worldBoneMtx->quat, tempBoneMtx);
            if ( XModelGetStaticBounds(dobj->localModels[j], tempBoneMtx, lmins, lmaxs) )
            {
                lmins[0] = worldBoneMtx->trans[0] + lmins[0];
                lmins[1] = worldBoneMtx->trans[1] + lmins[1];
                lmins[2] = worldBoneMtx->trans[2] + lmins[2];
                lmaxs[0] = worldBoneMtx->trans[0] + lmaxs[0];
                lmaxs[1] = worldBoneMtx->trans[1] + lmaxs[1];
                lmaxs[2] = worldBoneMtx->trans[2] + lmaxs[2];
            }
            else
            {
                lmins[0] = FLT_MAX;
                lmins[1] = FLT_MAX;
                lmins[2] = FLT_MAX;
                lmaxs[0] = -FLT_MAX;
                lmaxs[1] = -FLT_MAX;
                lmaxs[2] = -FLT_MAX;
            }
        }
        for ( i = 0; i < 3; ++i )
        {
            if ( mins[i] > lmins[i] )
                mins[i] = lmins[i];
            if ( lmaxs[i] > maxs[i] )
                maxs[i] = lmaxs[i];
        }
    }
}

void __cdecl DObjArchive(DObj *obj)
{
    DObjModel_s *model; // [esp+8h] [ebp-178h]
    SavedDObj savedObj; // [esp+10h] [ebp-170h] BYREF
    unsigned int modelIndex; // [esp+7Ch] [ebp-104h]
    DObjModel_s dobjModels[32]; // [esp+80h] [ebp-100h] BYREF

    DObjGetCreateParms(obj, dobjModels, &savedObj.numModels, &savedObj.tree, &savedObj.entnum);
    savedObj.ignoreCollision = 0;
    savedObj.models = obj->localModels;
    savedObj.flags = obj->flags;
    savedObj.localClientNum = obj->localClientIndex;
    savedObj.hidePartBits[0] = obj->hidePartBits[0];
    savedObj.hidePartBits[1] = obj->hidePartBits[1];
    savedObj.hidePartBits[2] = obj->hidePartBits[2];
    savedObj.hidePartBits[3] = obj->hidePartBits[3];
    savedObj.hidePartBits[4] = obj->hidePartBits[4];
    for ( modelIndex = 0; modelIndex < savedObj.numModels; ++modelIndex )
    {
        model = &dobjModels[modelIndex];
        savedObj.dobjModels[modelIndex].boneName = model->boneName;
        if ( model->ignoreCollision )
            savedObj.ignoreCollision |= 1 << modelIndex;
    }
    if ( !obj->localModels
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 1154, 0, "%s", "obj->models") )
    {
        __debugbreak();
    }
    obj->localModels = 0;
    DObjFree(obj);
    memcpy(obj, &savedObj, 0x68u);
}

void __cdecl DObjUnarchive(DObj *obj)
{
    DObjModel_s *model; // [esp+8h] [ebp-178h]
    SavedDObj savedObj; // [esp+10h] [ebp-170h] BYREF
    unsigned int modelIndex; // [esp+7Ch] [ebp-104h]
    DObjModel_s dobjModels[32]; // [esp+80h] [ebp-100h] BYREF

    memcpy(&savedObj, obj, sizeof(savedObj));
    for ( modelIndex = 0; modelIndex < savedObj.numModels; ++modelIndex )
    {
        model = &dobjModels[modelIndex];
        model->boneName = savedObj.dobjModels[modelIndex].boneName;
        model->model = savedObj.models[modelIndex];
        model->ignoreCollision = (savedObj.ignoreCollision & (1 << modelIndex)) != 0;
    }
    MT_Free((_BYTE *)savedObj.models, 5 * savedObj.numModels, SCRIPTINSTANCE_SERVER);
    DObjCreateExt(
        dobjModels,
        savedObj.numModels,
        savedObj.tree,
        (unsigned __int8 *)obj,
        savedObj.entnum,
        (savedObj.flags & 1) != 0,
        (savedObj.flags & 2) != 0,
        savedObj.localClientNum);
    DObjSetHidePartBits(obj, savedObj.hidePartBits);
}

void __cdecl DObjSkelClear(const DObj *obj)
{
    memset((void *)&obj->skel, 0, sizeof(obj->skel));
}

bool __cdecl DObjIsServer(const DObj *obj)
{
    return DObjHasFlag(obj, 1u) || obj->localClientIndex < 0;
}

bool __cdecl DObjIsLocalPlayer(const DObj *obj)
{
    return DObjHasFlag(obj, 2u);
}

bool __cdecl DObjIsPlayerShadow(const DObj *obj)
{
    return DObjHasFlag(obj, 0x20u);
}

bool __cdecl DObjIsLeftHandGripIKActive(const DObj *obj)
{
    return DObjHasFlag(obj, 0x10u);
}

int __cdecl DObjGetLocalClientIndex(const DObj *obj)
{
    return obj->localClientIndex;
}

void __cdecl DObjGetBounds(const DObj *obj, float *mins, float *maxs)
{
    iassert(obj);

    mins[0] = -obj->radius;
    mins[1] = -obj->radius;
    mins[2] = -obj->radius;

    maxs[0] = obj->radius;
    maxs[1] = obj->radius;
    maxs[2] = obj->radius;
}

void __cdecl DObjPhysicsGetBounds(const DObj *obj, float *mins, float *maxs)
{
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 1319, 0, "%s", "obj") )
        __debugbreak();
    if ( !*obj->localModels
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 1320, 0, "%s", "obj->models[0]") )
    {
        __debugbreak();
    }
    XModelGetBounds(*(const XModel **)obj->localModels, mins, maxs);
}

double __cdecl DObjGetRadius(const DObj *obj)
{
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 1334, 0, "%s", "obj") )
        __debugbreak();
    return obj->radius;
}

PhysPreset *__cdecl DObjGetPhysPreset(const DObj *obj)
{
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 1344, 0, "%s", "obj") )
        __debugbreak();
    if ( !obj->numModels
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 1345, 0, "%s", "obj->numModels > 0") )
    {
        __debugbreak();
    }
    if ( !*obj->localModels
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 1346, 0, "%s", "obj->models[0]") )
    {
        __debugbreak();
    }
    return (*obj->localModels)->physPreset;
}

const char *__cdecl DObjGetName(const DObj *obj)
{
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 1356, 0, "%s", "obj") )
        __debugbreak();
    if ( !obj->numModels
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 1357, 0, "%s", "obj->numModels > 0") )
    {
        __debugbreak();
    }
    if ( !*obj->localModels
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 1358, 0, "%s", "obj->models[0]") )
    {
        __debugbreak();
    }
    return **(const char ***)obj->localModels;
}

char *__cdecl DObjGetBoneName(const DObj *obj, int boneIndex)
{
    int j; // [esp+0h] [ebp-20h]
    int numBones; // [esp+4h] [ebp-1Ch]
    int baseBoneIndex; // [esp+8h] [ebp-18h]
    int numModels; // [esp+Ch] [ebp-14h]
    XModel *model; // [esp+10h] [ebp-10h]
    int index; // [esp+14h] [ebp-Ch]
    XModel **models; // [esp+18h] [ebp-8h]
    unsigned __int16 *boneNames; // [esp+1Ch] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 1511, 0, "%s", "obj") )
        __debugbreak();
    numModels = obj->numModels;
    baseBoneIndex = 0;
    models = obj->localModels;
    for ( j = 0; j < numModels; ++j )
    {
        model = models[j];
        boneNames = model->localBoneNames;
        numBones = model->numBones;
        index = boneIndex - baseBoneIndex;
        if ( boneIndex - baseBoneIndex < 0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 1525, 0, "%s", "index >= 0") )
        {
            __debugbreak();
        }
        if ( index < numBones )
            return SL_ConvertToString(boneNames[index], SCRIPTINSTANCE_SERVER);
        baseBoneIndex += numBones;
    }
    return 0;
}

char *__cdecl DObjGetModelParentBoneName(const DObj *obj, int modelIndex)
{
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 1552, 0, "%s", "obj") )
        __debugbreak();
    if ( modelIndex >= obj->numModels
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                    1553,
                    0,
                    "%s\n\t(modelIndex) = %i",
                    "(modelIndex < obj->numModels)",
                    modelIndex) )
    {
        __debugbreak();
    }
    return DObjGetBoneName(obj, *((unsigned __int8 *)&obj->localModels[obj->numModels] + modelIndex));
}

XAnimTree_s *__cdecl DObjGetTree(const DObj *obj)
{
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 1571, 0, "%s", "obj") )
        __debugbreak();
    return obj->localTree;
}

int __cdecl DObjBad(const DObj *obj)
{
    int j; // [esp+0h] [ebp-4h]

    for ( j = obj->numModels - 1; j >= 0; --j )
    {
        if ( XModelBad(obj->localModels[j]) )
            return 1;
    }
    return 0;
}

float s_cutOffSq = 4.0f;

void __cdecl DObjTracelinePartBits(DObj *obj, int *partBits)
{
    unsigned int j; // [esp+8h] [ebp-28h]
    XModel *model; // [esp+10h] [ebp-20h]
    unsigned int numModels; // [esp+14h] [ebp-1Ch]
    unsigned int size; // [esp+18h] [ebp-18h]
    unsigned int globalBoneIndex; // [esp+1Ch] [ebp-14h]
    unsigned int localBoneIndex; // [esp+20h] [ebp-10h]
    XBoneInfo *boneInfo; // [esp+24h] [ebp-Ch]
    XModel **models; // [esp+2Ch] [ebp-4h]

    PROF_SCOPED("traceline_bits");

    *partBits = 0;
    partBits[1] = 0;
    partBits[2] = 0;
    partBits[3] = 0;
    partBits[4] = 0;
    globalBoneIndex = 0;
    models = obj->localModels;
    numModels = obj->numModels;
    for ( j = 0; j < numModels; ++j )
    {
        model = models[j];
        size = model->numBones;
        if ( (obj->ignoreCollision & (1 << j)) != 0 )
        {
            globalBoneIndex += size;
        }
        else
        {
            for ( localBoneIndex = 0; localBoneIndex < size; ++localBoneIndex )
            {
                boneInfo = &model->localBoneInfo[localBoneIndex];
                if ( LODWORD(boneInfo->radiusSquared)
                    && s_cutOffSq <= boneInfo->radiusSquared
                    && (obj->hidePartBits[globalBoneIndex >> 5] & (0x80000000 >> (globalBoneIndex & 0x1F))) == 0 )
                {
                    partBits[globalBoneIndex >> 5] |= 0x80000000 >> (globalBoneIndex & 0x1F);
                }
                ++globalBoneIndex;
            }
        }
    }
    DObjCompleteHierarchyBits(obj, partBits);
}

void __cdecl DObjTraceline(DObj *obj, float *start, float *end, unsigned __int8 *priorityMap, DObjTrace_s *trace)
{
    double v5; // st7
    const char *v6; // eax
    float v7; // [esp+20h] [ebp-3DCh]
    unsigned __int16 v8; // [esp+26h] [ebp-3D6h]
    float *v9; // [esp+34h] [ebp-3C8h]
    float v10; // [esp+12Ch] [ebp-2D0h]
    DObjAnimMat *boneMatrix; // [esp+19Ch] [ebp-260h]
    unsigned int j; // [esp+1A0h] [ebp-25Ch]
    const unsigned __int8 *pos; // [esp+1A4h] [ebp-258h]
    const unsigned __int8 *modelParents; // [esp+1A8h] [ebp-254h]
    int jbit; // [esp+1ACh] [ebp-250h]
    float invL2; // [esp+1B0h] [ebp-24Ch]
    int t; // [esp+1B4h] [ebp-248h]
    bool bEndSolid; // [esp+1BBh] [ebp-241h]
    unsigned int cur_mask; // [esp+1BCh] [ebp-240h]
    float delta; // [esp+1C4h] [ebp-238h]
    float delta_4; // [esp+1C8h] [ebp-234h]
    float delta_8; // [esp+1CCh] [ebp-230h]
    DObjAnimMat *hitBoneMatrix; // [esp+1D0h] [ebp-22Ch]
    float sphereFraction; // [esp+1D4h] [ebp-228h]
    float localStart[3]; // [esp+1D8h] [ebp-224h] BYREF
    float enterFrac; // [esp+1E4h] [ebp-218h]
    float startOffset[3]; // [esp+1E8h] [ebp-214h]
    float dist1; // [esp+1F4h] [ebp-208h]
    int hitT; // [esp+1F8h] [ebp-204h]
    unsigned __int16 classificationArray[162]; // [esp+1FCh] [ebp-200h]
    XModel *model; // [esp+344h] [ebp-B8h]
    float dist; // [esp+348h] [ebp-B4h]
    unsigned int numModels; // [esp+34Ch] [ebp-B0h]
    float diff2; // [esp+350h] [ebp-ACh]
    float deltaLengthSq; // [esp+354h] [ebp-A8h]
    float sign; // [esp+358h] [ebp-A4h]
    unsigned int size; // [esp+35Ch] [ebp-A0h]
    float offset[3]; // [esp+360h] [ebp-9Ch]
    unsigned int globalBoneIndex; // [esp+36Ch] [ebp-90h]
    float d2; // [esp+370h] [ebp-8Ch]
    float cappedSphereFraction; // [esp+374h] [ebp-88h]
    float localEnd[3]; // [esp+378h] [ebp-84h] BYREF
    float *bounds; // [esp+384h] [ebp-78h]
    bool bStartSolid; // [esp+38Bh] [ebp-71h]
    unsigned int localBoneIndex; // [esp+38Ch] [ebp-70h]
    unsigned __int8 parentIndex; // [esp+393h] [ebp-69h]
    float solidHitFrac; // [esp+394h] [ebp-68h]
    float dist2; // [esp+398h] [ebp-64h]
    DSkel *skel; // [esp+39Ch] [ebp-60h]
    unsigned __int16 *names; // [esp+3A0h] [ebp-5Ch]
    float leaveFrac; // [esp+3A4h] [ebp-58h]
    int traceHitT; // [esp+3A8h] [ebp-54h]
    unsigned int lowestPriority; // [esp+3ACh] [ebp-50h]
    XBoneInfo *boneInfo; // [esp+3B0h] [ebp-4Ch]
    int ignoreCollision; // [esp+3B4h] [ebp-48h]
    float axis[3][3]; // [esp+3B8h] [ebp-44h] BYREF
    XModel **models; // [esp+3DCh] [ebp-20h]
    unsigned __int16 classification; // [esp+3E0h] [ebp-1Ch]
    int target_mask; // [esp+3E4h] [ebp-18h]
    float center[3]; // [esp+3E8h] [ebp-14h] BYREF
    float hitSign; // [esp+3F4h] [ebp-8h]
    unsigned int currentPriority; // [esp+3F8h] [ebp-4h]

    PROF_SCOPED("dobjtraceline");

    trace->sflags = 0;
    trace->modelIndex = 0;
    trace->partName = 0;
    trace->partGroup = 0;
    trace->localBoneIndex = 254;
    trace->normal[0] = 0.0f;
    trace->normal[1] = 0.0f;
    trace->normal[2] = 0.0f;
    delta = *end - *start;
    delta_4 = end[1] - start[1];
    delta_8 = end[2] - start[2];
    deltaLengthSq = (float)((float)(delta * delta) + (float)(delta_4 * delta_4)) + (float)(delta_8 * delta_8);
    if ( deltaLengthSq == 0.0 )
    {
        return;
    }
    boneMatrix = DObjGetRotTransArray(obj);
    if ( !boneMatrix )
    {
        return;
    }
    invL2 = 1.0 / deltaLengthSq;
    lowestPriority = 2;
    skel = &obj->skel;
    if ( obj == (DObj *)-20
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 1774, 0, "%s", "skel") )
    {
        __debugbreak();
    }
    if ( !obj->duplicateParts
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 1776, 0, "%s", "obj->duplicateParts") )
    {
        __debugbreak();
    }
    pos = (const unsigned __int8 *)(SL_ConvertToString(obj->duplicateParts, SCRIPTINSTANCE_SERVER) + 20);

    nanassertvec3(start);
    nanassertvec3(end);

    globalBoneIndex = 0;
    hitT = -1;
    traceHitT = -1;
    hitSign = 0.0f;
    hitBoneMatrix = 0;
    solidHitFrac = trace->fraction;
    models = obj->localModels;
    modelParents = (const unsigned __int8 *)&models[obj->numModels];
    numModels = obj->numModels;
    target_mask = (1 << numModels) - 1;
    cur_mask = obj->ignoreCollision;
    jbit = 1;
    j = 0;
LABEL_25:
    if ( j < numModels )
    {
        model = models[j];
        names = model->localBoneNames;
        size = model->numBones;
        ignoreCollision = jbit & obj->ignoreCollision;
        cur_mask |= jbit;
        if ( !ignoreCollision || cur_mask != target_mask )
        {
            localBoneIndex = 0;
            while ( 1 )
            {
                if ( localBoneIndex >= size )
                {
                    ++j;
                    jbit *= 2;
                    goto LABEL_25;
                }
                classification = model->partClassification[localBoneIndex];
                currentPriority = priorityMap[classification];
                if ( globalBoneIndex == *pos - 1 )
                {
                    pos += 2;
                    if ( currentPriority == 1 )
                    {
                        classification = classificationArray[*(pos - 1) - 1];
                        currentPriority = priorityMap[classification];
                    }
                }
                else if ( currentPriority == 1 )
                {
                    if ( localBoneIndex >= model->numRootBones )
                    {
                        classification = classificationArray[globalBoneIndex
                                                                                             - model->localParentList[localBoneIndex - model->numRootBones]];
                    }
                    else
                    {
                        parentIndex = modelParents[j];
                        if ( parentIndex == 255 )
                            v8 = 0;
                        else
                            v8 = classificationArray[parentIndex];
                        classification = v8;
                    }
                    currentPriority = priorityMap[classification];
                }
                classificationArray[globalBoneIndex] = classification;
                if ( !ignoreCollision )
                {
                    boneInfo = &model->localBoneInfo[localBoneIndex];
                    if ( LODWORD(boneInfo->radiusSquared) )
                    {
                        if ( s_cutOffSq <= boneInfo->radiusSquared
                            && (obj->hidePartBits[globalBoneIndex >> 5] & (0x80000000 >> (globalBoneIndex & 0x1F))) == 0 )
                        {
                            if ( (skel->partBits.skel[globalBoneIndex >> 5] & (0x80000000 >> (globalBoneIndex & 0x1F))) == 0
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                                            1858,
                                            0,
                                            "%s",
                                            "skel->partBits.skel[globalBoneIndex >> 5] & (HIGH_BIT >> (globalBoneIndex & 31))") )
                            {
                                __debugbreak();
                            }
                            if ( lowestPriority <= currentPriority )
                            {
                                if ( ((LODWORD(boneMatrix->quat[0]) & 0x7F800000) == 0x7F800000
                                     || (LODWORD(boneMatrix->quat[1]) & 0x7F800000) == 0x7F800000
                                     || (LODWORD(boneMatrix->quat[2]) & 0x7F800000) == 0x7F800000
                                     || (LODWORD(boneMatrix->quat[3]) & 0x7F800000) == 0x7F800000)
                                    && !Assert_MyHandler(
                                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                                                1864,
                                                0,
                                                "%s",
                                                "!IS_NAN((boneMatrix->quat)[0]) && !IS_NAN((boneMatrix->quat)[1]) && !IS_NAN((boneMatrix->quat)[2"
                                                "]) && !IS_NAN((boneMatrix->quat)[3])") )
                                {
                                    __debugbreak();
                                }
                                if ( ((LODWORD(boneMatrix->trans[0]) & 0x7F800000) == 0x7F800000
                                     || (LODWORD(boneMatrix->trans[1]) & 0x7F800000) == 0x7F800000
                                     || (LODWORD(boneMatrix->trans[2]) & 0x7F800000) == 0x7F800000)
                                    && !Assert_MyHandler(
                                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                                                1865,
                                                0,
                                                "%s",
                                                "!IS_NAN((boneMatrix->trans)[0]) && !IS_NAN((boneMatrix->trans)[1]) && !IS_NAN((boneMatrix->trans)[2])") )
                                {
                                    __debugbreak();
                                }
                                MatrixTransformVectorQuatTrans(boneInfo->offset, boneMatrix, center);
                                startOffset[0] = *start - center[0];
                                startOffset[1] = start[1] - center[1];
                                startOffset[2] = start[2] - center[2];
                                sphereFraction = (-((float)((float)(startOffset[0] * delta) + (float)(startOffset[1] * delta_4)) + (float)(startOffset[2] * delta_8)))
                                                             * invL2;
                                v10 = (float)(sphereFraction - 1.0) < 0.0 ? (-((float)((float)(startOffset[0] * delta) + (float)(startOffset[1] * delta_4)) + (float)(startOffset[2] * delta_8)))
                                        * invL2
                                        : 1.0f;
                                v7 = (float)(0.0 - sphereFraction) < 0.0 ? v10 : 0.0f;
                                cappedSphereFraction = v7;
                                offset[0] = (float)(v7 * delta) + startOffset[0];
                                offset[1] = (float)(v7 * delta_4) + startOffset[1];
                                offset[2] = (float)(v7 * delta_8) + startOffset[2];
                                d2 = (float)((float)(offset[0] * offset[0]) + (float)(offset[1] * offset[1]))
                                     + (float)(offset[2] * offset[2]);
                                diff2 = boneInfo->radiusSquared - d2;
                                if ( diff2 > 0.0
                                    && (lowestPriority != currentPriority
                                     || (float)(sphereFraction - sqrtf(diff2 * invL2)) < trace->fraction) )
                                {
                                    InvMatrixTransformVectorQuatTrans(start, boneMatrix, localStart);
                                    InvMatrixTransformVectorQuatTrans(end, boneMatrix, localEnd);
                                    if ( ((LODWORD(localStart[0]) & 0x7F800000) == 0x7F800000
                                         || (LODWORD(localStart[1]) & 0x7F800000) == 0x7F800000
                                         || (LODWORD(localStart[2]) & 0x7F800000) == 0x7F800000)
                                        && !Assert_MyHandler(
                                                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                                                    1889,
                                                    0,
                                                    "%s",
                                                    "!IS_NAN((localStart)[0]) && !IS_NAN((localStart)[1]) && !IS_NAN((localStart)[2])") )
                                    {
                                        __debugbreak();
                                    }
                                    if ( ((LODWORD(localEnd[0]) & 0x7F800000) == 0x7F800000
                                         || (LODWORD(localEnd[1]) & 0x7F800000) == 0x7F800000
                                         || (LODWORD(localEnd[2]) & 0x7F800000) == 0x7F800000)
                                        && !Assert_MyHandler(
                                                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                                                    1890,
                                                    0,
                                                    "%s",
                                                    "!IS_NAN((localEnd)[0]) && !IS_NAN((localEnd)[1]) && !IS_NAN((localEnd)[2])") )
                                    {
                                        __debugbreak();
                                    }
                                    enterFrac = 0.0f;
                                    if ( lowestPriority == currentPriority )
                                        leaveFrac = trace->fraction;
                                    else
                                        leaveFrac = solidHitFrac;
                                    bStartSolid = 1;
                                    bEndSolid = 1;
                                    sign = -1.0f;
                                    for ( bounds = (float *)boneInfo; ; bounds += 3 )
                                    {
                                        nanassertvec3(bounds);

                                        for ( t = 0; t < 3; ++t )
                                        {
                                            dist1 = (float)(localStart[t] - bounds[t]) * sign;
                                            dist2 = (float)(localEnd[t] - bounds[t]) * sign;
                                            if ( dist1 <= 0.0 )
                                            {
                                                if ( dist2 > 0.0 )
                                                {
                                                    bEndSolid = 0;
                                                    dist = dist1 - dist2;
                                                    if ( (float)(dist1 - dist2) >= 0.0
                                                        && !Assert_MyHandler(
                                                                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                                                                    1933,
                                                                    0,
                                                                    "%s",
                                                                    "dist < 0") )
                                                    {
                                                        __debugbreak();
                                                    }
                                                    if ( dist1 > (float)(leaveFrac * dist) )
                                                    {
                                                        leaveFrac = dist1 / dist;
                                                        if ( enterFrac >= (float)(dist1 / dist) )
                                                            goto LABEL_29;
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                if ( dist2 > 0.0 )
                                                    goto LABEL_29;
                                                bStartSolid = 0;
                                                dist = dist1 - dist2;
                                                if ( (float)(dist1 - dist2) <= 0.0
                                                    && !Assert_MyHandler(
                                                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                                                                1919,
                                                                0,
                                                                "%s",
                                                                "dist > 0") )
                                                {
                                                    __debugbreak();
                                                }
                                                if ( dist1 > (float)(enterFrac * dist) )
                                                {
                                                    enterFrac = dist1 / dist;
                                                    if ( (float)(dist1 / dist) >= leaveFrac )
                                                        goto LABEL_29;
                                                    hitSign = sign;
                                                    hitT = t;
                                                }
                                            }
                                        }
                                        if ( sign == 1.0 )
                                            break;
                                        sign = 1.0f;
                                    }
                                    if ( bStartSolid )
                                    {
                                        if ( bEndSolid && (float)((float)(delta * *start) + (float)(delta_4 * start[1])) < 0.0 )
                                        {
                                            trace->fraction = 0.0f;
                                            trace->modelIndex = j;
                                            if ( trace->modelIndex != j
                                                && !Assert_MyHandler(
                                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                                                            1962,
                                                            1,
                                                            "%s",
                                                            "trace->modelIndex == j") )
                                            {
                                                __debugbreak();
                                            }
                                            trace->partName = names[localBoneIndex];
                                            trace->partGroup = classification;
                                            trace->localBoneIndex = localBoneIndex;
                                            Vec2NormalizeTo(start, trace->normal);
                                            if ( !Vec3IsNormalized(trace->normal) )
                                            {
                                                v5 = Vec3Length(trace->normal);
                                                v6 = va("(%g %g %g) len %g", trace->normal[0], trace->normal[1], trace->normal[2], v5);
                                                if ( !Assert_MyHandler(
                                                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                                                                1969,
                                                                0,
                                                                "%s\n\t%s",
                                                                "Vec3IsNormalized( trace->normal )",
                                                                v6) )
                                                    __debugbreak();
                                            }
                                            return;
                                        }
                                    }
                                    else
                                    {
                                        if ( lowestPriority == currentPriority )
                                        {
                                            if ( enterFrac >= trace->fraction )
                                                goto LABEL_29;
                                        }
                                        else
                                        {
                                            lowestPriority = currentPriority;
                                        }
                                        trace->fraction = enterFrac;
                                        if ( (trace->fraction < 0.0 || trace->fraction > 1.0)
                                            && !Assert_MyHandler(
                                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                                                        1986,
                                                        1,
                                                        "%s\n\t(trace->fraction) = %g",
                                                        "(trace->fraction >= 0.0f && trace->fraction <= 1.0f)",
                                                        trace->fraction) )
                                        {
                                            __debugbreak();
                                        }
                                        trace->modelIndex = j;
                                        if ( trace->modelIndex != j
                                            && !Assert_MyHandler(
                                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                                                        1988,
                                                        1,
                                                        "%s",
                                                        "trace->modelIndex == j") )
                                        {
                                            __debugbreak();
                                        }
                                        trace->partName = names[localBoneIndex];
                                        trace->partGroup = classification;
                                        trace->localBoneIndex = localBoneIndex;
                                        if ( hitT < 0
                                            && !Assert_MyHandler(
                                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                                                        1992,
                                                        0,
                                                        "%s",
                                                        "hitT >= 0") )
                                        {
                                            __debugbreak();
                                        }
                                        if ( hitT >= 3
                                            && !Assert_MyHandler(
                                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                                                        1993,
                                                        0,
                                                        "%s",
                                                        "hitT < 3") )
                                        {
                                            __debugbreak();
                                        }
                                        traceHitT = hitT;
                                        hitBoneMatrix = boneMatrix;
                                    }
                                }
                            }
                        }
                    }
                }
LABEL_29:
                ++localBoneIndex;
                ++boneMatrix;
                ++globalBoneIndex;
            }
        }
    }
    if ( hitBoneMatrix )
    {
        if ( traceHitT < 0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 2003, 0, "%s", "traceHitT >= 0") )
        {
            __debugbreak();
        }
        if ( traceHitT >= 3
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 2004, 0, "%s", "traceHitT < 3") )
        {
            __debugbreak();
        }
        ConvertQuatToMat(hitBoneMatrix, axis);
        v9 = axis[traceHitT];
        trace->normal[0] = hitSign * *v9;
        trace->normal[1] = hitSign * v9[1];
        trace->normal[2] = hitSign * v9[2];
    }
}

void __cdecl InvMatrixTransformVectorQuatTrans(const float *in, const DObjAnimMat *mat, float *out)
{
    float temp; // [esp+4Ch] [ebp-30h]
    float temp_4; // [esp+50h] [ebp-2Ch]
    float temp_8; // [esp+54h] [ebp-28h]
    float axis[3][3]; // [esp+58h] [ebp-24h] BYREF

    temp = *in - mat->trans[0];
    temp_4 = in[1] - mat->trans[1];
    temp_8 = in[2] - mat->trans[2];
    ConvertQuatToMat(mat, axis);
    *out = (float)((float)(temp * axis[0][0]) + (float)(temp_4 * axis[0][1])) + (float)(temp_8 * axis[0][2]);
    out[1] = (float)((float)(temp * axis[1][0]) + (float)(temp_4 * axis[1][1])) + (float)(temp_8 * axis[1][2]);
    out[2] = (float)((float)(temp * axis[2][0]) + (float)(temp_4 * axis[2][1])) + (float)(temp_8 * axis[2][2]);
}

void __cdecl DObjGeomTraceline(DObj *obj, float *localStart, float *localEnd, int contentmask, DObjTrace_s *results)
{
    unsigned int boneIndex; // [esp+14h] [ebp-5Ch]
    XModel *model; // [esp+1Ch] [ebp-54h]
    int partIndex; // [esp+20h] [ebp-50h]
    unsigned int numModels; // [esp+24h] [ebp-4Ch]
    trace_t trace; // [esp+28h] [ebp-48h] BYREF
    DObjAnimMat *boneMtxList; // [esp+64h] [ebp-Ch]
    unsigned int i; // [esp+68h] [ebp-8h]
    XModel **models; // [esp+6Ch] [ebp-4h]

    PROF_SCOPED("geomtraceline");

    results->modelIndex = 0;
    results->partName = 0;
    results->partGroup = 0;
    results->localBoneIndex = 254;
    trace.fraction = results->fraction;
    if ( (trace.fraction < 0.0 || trace.fraction > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                    2038,
                    1,
                    "%s\n\t(trace.fraction) = %g",
                    "(trace.fraction >= 0.0f && trace.fraction <= 1.0f)",
                    trace.fraction) )
    {
        __debugbreak();
    }
    trace.sflags = 0;
    memset(&trace, 0, 12);
    boneMtxList = DObjGetRotTransArray(obj);
    if ( boneMtxList )
    {
        models = obj->localModels;
        boneIndex = 0;
        numModels = obj->numModels;
        i = 0;
        while ( i < numModels )
        {
            model = models[i];
            partIndex = XModelTraceLineAnimated(obj, i, boneIndex, &trace, boneMtxList, localStart, localEnd, contentmask);
            if ( partIndex >= 0 )
            {
                results->modelIndex = i;
                if ( results->modelIndex != i
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                                2057,
                                1,
                                "%s",
                                "results->modelIndex == i") )
                {
                    __debugbreak();
                }
                results->partName = model->localBoneNames[partIndex];
                results->localBoneIndex = partIndex;
            }
            ++i;
            boneIndex += model->numBones;
            boneMtxList += model->numBones;
        }
    }
    results->fraction = trace.fraction;
    if ( (results->fraction < 0.0 || results->fraction > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                    2064,
                    1,
                    "%s\n\t(results->fraction) = %g",
                    "(results->fraction >= 0.0f && results->fraction <= 1.0f)",
                    results->fraction) )
    {
        __debugbreak();
    }
    results->sflags = trace.sflags;
    *(_QWORD *)results->normal = *(_QWORD *)trace.normal.vec.v;
    LODWORD(results->normal[2]) = trace.normal.vec.u[2];
}

void __cdecl DObjGeomTracelinePartBits(DObj *obj, int contentmask, int *partBits)
{
    unsigned int boneIndex; // [esp+8h] [ebp-18h]
    XModel *model; // [esp+10h] [ebp-10h]
    unsigned int numModels; // [esp+14h] [ebp-Ch]
    unsigned int i; // [esp+18h] [ebp-8h]
    XModel **models; // [esp+1Ch] [ebp-4h]

    PROF_SCOPED("geomtraceline_bits");

    *partBits = 0;
    partBits[1] = 0;
    partBits[2] = 0;
    partBits[3] = 0;
    partBits[4] = 0;
    models = obj->localModels;
    boneIndex = 0;
    numModels = obj->numModels;
    for ( i = 0; i < numModels; ++i )
    {
        model = models[i];
        XModelTraceLineAnimatedPartBits(obj, i, boneIndex, contentmask, partBits);
        boneIndex += model->numBones;
    }
    DObjCompleteHierarchyBits(obj, partBits);
}

int __cdecl DObjHasContents(DObj *obj, int contentmask)
{
    int i; // [esp+8h] [ebp-8h]
    XModel **models; // [esp+Ch] [ebp-4h]

    models = obj->localModels;
    for ( i = 0; i < obj->numModels; ++i )
    {
        if ( (contentmask & XModelGetContents(*models)) != 0 )
            return 1;
    }
    return 0;
}

int __cdecl DObjGetContents(const DObj *obj)
{
    int contents; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    contents = 0;
    for ( i = 0; i < obj->numModels; ++i )
        contents |= XModelGetContents(obj->localModels[i]);
    return contents;
}

bool __cdecl DObjHasCollmap(const DObj *obj)
{
    return obj->numModels && XModelHasCollmap(*(const XModel **)obj->localModels);
}

PhysGeomList *__cdecl DObjGetCollmap(DObj *obj)
{
    if ( obj->numModels )
        return XModelGetCollmap(*(const XModel **)obj->localModels, 0);
    else
        return 0;
}

XModel *__cdecl DObjGetXModel(DObj *obj, int index)
{
    if ( index < 0 || index >= obj->numModels )
        return 0;
    else
        return obj->localModels[index];
}

int __cdecl DObjSetLocalBoneIndex(DObj *obj, int *partBits, int boneIndex, const float *trans, const float *angles)
{
    if ( !DObjSetRotTransIndex(obj, partBits, boneIndex) )
        return 0;
    DObjSetLocalTagInternal(obj, trans, angles, boneIndex);
    return 1;
}

int __cdecl DObjGetModelBoneIndex(const DObj *obj, const char *modelName, unsigned int name, unsigned __int8 *index)
{
    char *v4; // eax
    int j; // [esp+0h] [ebp-18h]
    int ja; // [esp+0h] [ebp-18h]
    unsigned int boneIndex; // [esp+4h] [ebp-14h]
    int numModels; // [esp+8h] [ebp-10h]
    XModel *model; // [esp+Ch] [ebp-Ch]
    XModel *modela; // [esp+Ch] [ebp-Ch]
    unsigned int localBoneIndex; // [esp+10h] [ebp-8h]
    XModel **models; // [esp+14h] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 2354, 0, "%s", "obj") )
        __debugbreak();
    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 2355, 0, "%s", "name") )
        __debugbreak();
    if ( !SL_IsLowercaseString(name, SCRIPTINSTANCE_SERVER) )
    {
        v4 = SL_ConvertToString(name, SCRIPTINSTANCE_SERVER);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                        2357,
                        0,
                        "%s\n\t(SL_ConvertToString( name )) = %s",
                        "(SL_IsLowercaseString( name ))",
                        v4) )
            __debugbreak();
    }
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
                if ( name == model->localBoneNames[localBoneIndex] && !I_stricmp(modelName, model->name) )
                    return 1;
                break;
            }
            localBoneIndex -= model->numBones;
        }
    }
    boneIndex = 0;
    for ( ja = 0; ja < numModels; ++ja )
    {
        modela = models[ja];
        if ( !I_stricmp(modelName, modela->name) && XModelGetBoneIndex(modela, name, boneIndex, index) )
            return 1;
        boneIndex += modela->numBones;
    }
    *index = -1;
    return 0;
}

void __cdecl DObjGetBasePoseMatrix(DObj *obj, unsigned __int8 boneIndex, DObjAnimMat *outMat)
{
    DObjAnimMat mat[160]; // [esp+8h] [ebp-1418h] BYREF
    int partBits[5]; // [esp+140Ch] [ebp-14h] BYREF

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 2427, 0, "%s", "obj") )
        __debugbreak();
    if ( !outMat && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp", 2428, 0, "%s", "outMat") )
        __debugbreak();
    if ( boneIndex >= (unsigned int)obj->numBones
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj.cpp",
                    2429,
                    0,
                    "boneIndex doesn't index obj->numBones\n\t%i not in [0, %i)",
                    boneIndex,
                    obj->numBones) )
    {
        __debugbreak();
    }
    if ( boneIndex >= (int)(*obj->localModels)->numBones )
    {
        DObjGetHierarchyBits(obj, boneIndex, partBits);
        DObjCalcBaseSkel(obj, mat, partBits);
        memcpy(outMat, &mat[boneIndex], sizeof(DObjAnimMat));
    }
    else
    {
        memcpy(outMat, &XModelGetBasePose(*(const XModel **)obj->localModels)[boneIndex], sizeof(DObjAnimMat));
    }
}

void __cdecl DObjSetHidePartBits(DObj *obj, const unsigned int *partBits)
{
    obj->hidePartBits[0] = *partBits;
    obj->hidePartBits[1] = partBits[1];
    obj->hidePartBits[2] = partBits[2];
    obj->hidePartBits[3] = partBits[3];
    obj->hidePartBits[4] = partBits[4];
}

bool __cdecl DObjHasFlag(const DObj *obj, unsigned __int8 flag)
{
    return (unsigned __int8)(flag & obj->flags) != 0;
}

void __cdecl DObjSetFlag(DObj *obj, unsigned __int8 flag, int val)
{
    if ( val )
        obj->flags |= flag;
    else
        obj->flags &= ~flag;
}

bool __cdecl DObjHasIKActiveFlag(const DObj *obj)
{
    return (obj->flags & 8) != 0;
}

void __cdecl DObjSetIKActiveFlag(DObj *obj, int val)
{
    if ( val )
        obj->flags |= 8u;
    else
        obj->flags &= ~8u;
}

void __cdecl DObjSetIKState(DObj *obj, unsigned __int8 *pIKState)
{
    obj->ikState = pIKState;
}