#include "xanim.h"
#include "xmodel.h"
#include "xanim_clientnotify.h"

#include <clientscript/cscr_stringlist.h>
#include <universal/com_memory.h>
#include "xanim_load_obj.h"
#include <qcommon/common.h>
#include "xanim_calc.h"
#include <qcommon/threads.h>
#include <cgame_mp/cg_local_mp.h>
#include <clientscript/cscr_vm.h>
#include <database/db_registry.h>
#include <gfx_d3d/r_rendercmds.h>

XAnimInfo g_xAnimInfo[4096];
unsigned int g_endNotetrackName;
bool g_anim_developer;
XAnimClientNotifyList *g_clientList;
XAnimServerNotifyList *g_currList;

int __cdecl XAnimTreeSize()
{
    return 8;
}

bool __cdecl XAnimTreeHasInfo(const XAnimTree_s *tree)
{
    if ( !tree && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 275, 0, "%s", "tree") )
        __debugbreak();
    return tree->children != 0;
}

void __cdecl XAnimInit()
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 4096; ++i )
    {
        g_xAnimInfo[i].prev = (i + 4095) % 4096;
        g_xAnimInfo[i].next = (i + 1) % 4096;
    }
    g_xAnimInfo[0].state.currentAnimTime = 0.0;
    g_xAnimInfo[0].state.oldTime = 0.0;
    *(unsigned int *)&g_xAnimInfo[0].state.cycleCount = 0;
    g_xAnimInfo[0].state.goalTime = 0.0;
    g_xAnimInfo[0].state.goalWeight = 0.0;
    g_xAnimInfo[0].state.weight = 0.0;
    g_xAnimInfo[0].state.rate = 0.0;
    *(unsigned int *)&g_xAnimInfo[0].state.instantWeightChange = 0;
    g_endNotetrackName = SL_GetString_(SCRIPTINSTANCE_SERVER, "end", 0, 3);
    g_anim_developer = com_developer->current.integer != 0;
}

void __cdecl XAnimShutdown()
{
    if ( g_endNotetrackName )
    {
        SL_RemoveRefToString(SCRIPTINSTANCE_SERVER, g_endNotetrackName);
        g_endNotetrackName = 0;
    }
}

void __cdecl XAnimFree(XAnimParts *parts, scriptInstance_t inst)
{
    int size; // [esp+0h] [ebp-14h]
    XAnimNotifyInfo *notify; // [esp+4h] [ebp-10h]
    int i; // [esp+8h] [ebp-Ch]
    __int16 notifyInfoIndex; // [esp+Ch] [ebp-8h]
    unsigned __int16 *boneNames; // [esp+10h] [ebp-4h]

    boneNames = parts->names;
    size = parts->boneCount[9];
    for ( i = 0; i < size; ++i )
        SL_RemoveRefToString(inst, boneNames[i]);
    notifyInfoIndex = 0;
    notify = parts->notify;
    while ( notifyInfoIndex < (int)parts->notifyCount )
    {
        SL_RemoveRefToString(inst, notify->name);
        ++notifyInfoIndex;
        ++notify;
    }
}

void *__cdecl Hunk_AllocXAnimPrecache(unsigned int size)
{
    return Hunk_AllocAlign(size, 4, "XAnimPrecache", 13);
}

XAnimParts *__cdecl XAnimPrecache(char *name, void *(__cdecl *Alloc)(int))
{
    XAnimParts *result; // eax
    XAnimParts *Data_LoadObj; // eax
    XAnimParts *defaultParts; // [esp+Ch] [ebp-8h]
    XAnimParts *parts; // [esp+10h] [ebp-4h]

    if ( useFastFile->current.enabled )
        result = (XAnimParts *)((int (__cdecl *)(char *, unsigned __int8 *(__cdecl *)(unsigned int)))XAnimFindData_FastFile)(
                                                         name,
                                                         (unsigned char*(*)(unsigned int))Hunk_AllocXAnimPrecache);
    else
        result = XAnimFindData_LoadObj(name, (void *(__cdecl *)(int))Hunk_AllocXAnimPrecache);
    if ( !result )
    {
        parts = XAnimLoadFile(name, Alloc, 0);
        if ( !parts )
        {
            Com_PrintWarning(19, "WARNING: Couldn't find xanim '%s', using default xanim '%s' instead\n", name, "void");
            if ( useFastFile->current.enabled )
                Data_LoadObj = (XAnimParts *)((int (__cdecl *)(const char *, unsigned __int8 *(__cdecl *)(unsigned int)))XAnimFindData_FastFile)("void",(unsigned char *(*)(unsigned int))Hunk_AllocXAnimPrecache);
            else
                Data_LoadObj = XAnimFindData_LoadObj((char *)"void", (void *(__cdecl *)(int))Hunk_AllocXAnimPrecache);
            defaultParts = Data_LoadObj;
            if ( !Data_LoadObj )
            {
                defaultParts = XAnimLoadFile((char*)"void", Alloc, 0);
                if ( !defaultParts )
                {
                    Com_Error(ERR_DROP, "Cannot find xanim '%s'", "void");
                    return 0;
                }
                Hunk_SetDataForFile(6, "void", defaultParts, Alloc);
            }
            parts = XAnimClone(defaultParts, Alloc);
            parts->isDefault = 1;
        }
        parts->name = Hunk_SetDataForFile(6, name, parts, Alloc);
        return parts;
    }
    return result;
}

XAnimParts *__cdecl XAnimFindData_LoadObj(char *name, void *(__cdecl *Alloc)(int))
{
    XAnimParts *animParts; // [esp+0h] [ebp-4h]
    XAnimParts *animPartsa; // [esp+0h] [ebp-4h]

    animParts = (XAnimParts *)Hunk_FindDataForFile(6, name);
    if ( animParts )
        return animParts;
    animPartsa = XAnimLoadFile(name, Alloc, 0);
    if ( animPartsa )
        animPartsa->name = Hunk_SetDataForFile(6, name, animPartsa, Alloc);
    return animPartsa;
}

XAnimParts *__cdecl XAnimFindData_FastFile(const char *name)
{
    return DB_FindXAssetHeader(ASSET_TYPE_XANIMPARTS, (char *)name, 1, -1).parts;
}

XAnimParts *__cdecl XAnimClone(XAnimParts *fromParts, void *(__cdecl *Alloc)(int))
{
    XAnimParts *toParts; // [esp+8h] [ebp-18h]
    int size; // [esp+Ch] [ebp-14h]
    XAnimNotifyInfo *notify; // [esp+10h] [ebp-10h]
    int i; // [esp+14h] [ebp-Ch]
    __int16 notifyInfoIndex; // [esp+18h] [ebp-8h]
    unsigned __int16 *boneNames; // [esp+1Ch] [ebp-4h]

    toParts = (XAnimParts *)Alloc(104);
    memcpy(toParts, fromParts, sizeof(XAnimParts));
    boneNames = toParts->names;
    size = toParts->boneCount[9];
    for ( i = 0; i < size; ++i )
        SL_AddRefToString(boneNames[i], SCRIPTINSTANCE_SERVER);
    notifyInfoIndex = 0;
    notify = toParts->notify;
    while ( notifyInfoIndex < (int)toParts->notifyCount )
    {
        SL_AddRefToString(notify->name, SCRIPTINSTANCE_SERVER);
        ++notifyInfoIndex;
        ++notify;
    }
    return toParts;
}

void __cdecl XAnimCreate(XAnim_s *anims, unsigned int animIndex, char *name)
{
    XAnimParts *Data_LoadObj; // eax
    char v4; // [esp+3h] [ebp-31h]
    char *v5; // [esp+8h] [ebp-2Ch]
    char *v6; // [esp+Ch] [ebp-28h]
    char *debugName; // [esp+28h] [ebp-Ch]
    XAnimEntry *anim; // [esp+2Ch] [ebp-8h]
    XAnimParts *parts; // [esp+30h] [ebp-4h]

    if ( useFastFile->current.enabled )
        Data_LoadObj = (XAnimParts *)((int (__cdecl *)(char *, unsigned __int8 *(__cdecl *)(unsigned int)))XAnimFindData_FastFile)(
                                                                     name,
                                                                     (unsigned char *(*)(unsigned int))Hunk_AllocXAnimPrecache);
    else
        Data_LoadObj = XAnimFindData_LoadObj(name, (void *(__cdecl *)(int))Hunk_AllocXAnimPrecache);
    parts = Data_LoadObj;
    if ( useFastFile->current.enabled || Data_LoadObj )
    {
        if ( !Data_LoadObj
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 572, 0, "%s", "parts") )
        {
            __debugbreak();
        }
        anim = &anims->entries[animIndex];
        anim->bCreated = 1;
        anim->numAnims = 0;
        anim->parts = parts;
        if ( anims->debugAnimNames )
        {
            if ( anims->debugAnimNames[animIndex]
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                            584,
                            0,
                            "%s",
                            "!anims->debugAnimNames[animIndex]") )
            {
                __debugbreak();
            }
            debugName = (char *)Hunk_UserAlloc(g_DebugHunkUser, strlen(name) + 1, 4, "XAnimCreate");
            v6 = name;
            v5 = debugName;
            do
            {
                v4 = *v6;
                *v5++ = *v6++;
            }
            while ( v4 );
            anims->debugAnimNames[animIndex] = debugName;
        }
    }
    else
    {
        Com_Error(ERR_DROP, "Cannot find xanim '%s'", name);
    }
}

void __cdecl XAnimBlend(
                XAnim_s *anims,
                unsigned int animIndex,
                const char *name,
                unsigned int children,
                unsigned int num,
                unsigned int flags)
{
    char v6; // [esp+3h] [ebp-31h]
    char *v7; // [esp+8h] [ebp-2Ch]
    const char *v8; // [esp+Ch] [ebp-28h]
    int parentIndex; // [esp+24h] [ebp-10h]
    char *debugName; // [esp+28h] [ebp-Ch]
    unsigned int i; // [esp+30h] [ebp-4h]

    if ( !num && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 604, 0, "%s", "num > 0") )
        __debugbreak();
    anims->entries[animIndex].numAnims = num;
    if ( num != anims->entries[animIndex].numAnims
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 611, 0, "%s", "num == anim->numAnims") )
    {
        __debugbreak();
    }
    anims->entries[animIndex].animParent.flags = flags;
    if ( flags != anims->entries[animIndex].animParent.flags
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    613,
                    0,
                    "%s",
                    "flags == anim->animParent.flags") )
    {
        __debugbreak();
    }
    anims->entries[animIndex].animParent.children = children;
    if ( children != anims->entries[animIndex].animParent.children
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    615,
                    0,
                    "%s",
                    "children == anim->animParent.children") )
    {
        __debugbreak();
    }
    for ( i = 0; i < num; ++i )
    {
        anims->entries[i + anims->entries[animIndex].animParent.children].parent = animIndex;
        if ( animIndex != anims->entries[i + anims->entries[animIndex].animParent.children].parent
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        621,
                        0,
                        "%s",
                        "animIndex == anims->entries[anim->animParent.children + i].parent") )
        {
            __debugbreak();
        }
    }
    if ( IsNodeAdditive(&anims->entries[animIndex]) )
    {
        for ( parentIndex = anims->entries[animIndex].parent; parentIndex; parentIndex = anims->entries[parentIndex].parent )
        {
            if ( IsNodeAdditive(&anims->entries[parentIndex]) )
                Com_Error(ERR_DROP, "Do not nest additives");
        }
    }
    if ( anims->debugAnimNames )
    {
        if ( anims->debugAnimNames[animIndex]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        644,
                        0,
                        "%s",
                        "!anims->debugAnimNames[animIndex]") )
        {
            __debugbreak();
        }
        debugName = (char *)Hunk_UserAlloc(g_DebugHunkUser, strlen(name) + 1, 4, "XAnimBlend");
        v8 = name;
        v7 = debugName;
        do
        {
            v6 = *v8;
            *v7++ = *v8++;
        }
        while ( v6 );
        anims->debugAnimNames[animIndex] = debugName;
    }
}

bool __cdecl IsNodeAdditive(const XAnimEntry *node)
{
    if ( !node && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 290, 0, "%s", "node") )
        __debugbreak();
    return !IsLeafNode(node) && (node->animParent.flags & 0x10) != 0;
}

bool __cdecl IsLeafNode(const XAnimEntry *anim)
{
    return anim->numAnims == 0;
}

void __cdecl XAnimSetParamValue(XAnim_s *anims, unsigned int paramIndex, char *strParamName, float paramValue)
{
    if ( paramIndex >= anims->paramSize
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    654,
                    0,
                    "%s",
                    "paramIndex < anims->paramSize") )
    {
        __debugbreak();
    }
    strncpy(anims->params[paramIndex].strName, strParamName, 0x10u);
    anims->params[paramIndex].fValue = paramValue;
}

void __cdecl XAnimSetValueSection(
                XAnim_s *anims,
                unsigned int animIndex,
                __int16 iFirstValueIndex,
                unsigned __int16 iValueCount)
{
    if ( animIndex >= anims->size
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 662, 0, "%s", "animIndex < anims->size") )
    {
        __debugbreak();
    }
    anims->entries[animIndex].firstParamIxPlusOne = iFirstValueIndex + 1;
    anims->entries[animIndex].paramCount = iValueCount;
}

XAnim_s *__cdecl XAnimCreateAnims(const char *debugName, int size, void *(__cdecl *Alloc)(unsigned int))
{
    return XAnimCreateAnimsWithValues(debugName, size, 0, (void*(*)(int))Alloc);
}

XAnim_s *__cdecl XAnimCreateAnimsWithValues(
                const char *debugName,
                int size,
                unsigned int iTotalValueCount,
                void *(__cdecl *Alloc)(int))
{
    char v5; // [esp+3h] [ebp-29h]
    char *v6; // [esp+8h] [ebp-24h]
    const char *v7; // [esp+Ch] [ebp-20h]
    char *newDebugName; // [esp+20h] [ebp-Ch]
    XAnim_s *anims; // [esp+28h] [ebp-4h]

    if ( !debugName && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 683, 0, "%s", "debugName") )
        __debugbreak();
    if ( !Alloc && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 684, 0, "%s", "Alloc") )
        __debugbreak();
    anims = (XAnim_s *)Alloc(16 * size + 20 * iTotalValueCount + 24);
    memset(anims, 0, 16 * size + 20 * iTotalValueCount + 24);
    anims->size = size;
    anims->paramSize = iTotalValueCount;
    if ( anims->paramSize )
        anims->params = (XAnimParam *)&anims->entries[size];
    if ( g_anim_developer )
    {
        newDebugName = (char *)Hunk_UserAlloc(g_DebugHunkUser, strlen(debugName) + 1, 4, "XAnimCreateAnims");
        v7 = debugName;
        v6 = newDebugName;
        do
        {
            v5 = *v7;
            *v6++ = *v7++;
        }
        while ( v5 );
        anims->debugName = newDebugName;
        anims->debugAnimNames = (const char **)Hunk_UserAlloc(g_DebugHunkUser, 4 * size, 4, "XAnimCreateAnims");
        memset(anims->debugAnimNames, 0, 4 * size);
    }
    anims->wasLoggedIfMissing = (bool *)Hunk_UserAlloc(g_DebugHunkUser, size, 4, "XAnimCreateAnims");
    memset(anims->wasLoggedIfMissing, 0, size);
    if ( Hunk_DataOnHunk((unsigned __int8 *)anims) )
        Hunk_AddData(2, anims, Alloc);
    return anims;
}

void __cdecl XAnimFreeList(XAnim_s *anims)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    if ( anims->debugName )
    {
        Hunk_UserFree(g_DebugHunkUser, (void *)anims->debugName);
        anims->debugName = 0;
    }
    if ( anims->debugAnimNames )
    {
        for ( i = 0; i < anims->size; ++i )
        {
            if ( anims->debugAnimNames[i] )
            {
                Hunk_UserFree(g_DebugHunkUser, (void *)anims->debugAnimNames[i]);
                anims->debugAnimNames[i] = 0;
            }
        }
        Hunk_UserFree(g_DebugHunkUser, anims->debugAnimNames);
        anims->debugAnimNames = 0;
        Hunk_UserFree(g_DebugHunkUser, anims->wasLoggedIfMissing);
        anims->wasLoggedIfMissing = 0;
    }
}

XAnimTree_s *__cdecl XAnimCreateTree(XAnim_s *anims, void *(__cdecl *Alloc)(unsigned int))
{
    XAnimTree_s *tree; // [esp+0h] [ebp-Ch]
    int entrySize; // [esp+8h] [ebp-4h]

    if ( !anims && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 795, 0, "%s", "anims") )
        __debugbreak();
    if ( !anims->size && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 798, 0, "%s", "size") )
        __debugbreak();
    entrySize = XAnimTreeSize();
    tree = (XAnimTree_s *)Alloc(entrySize);
    memset(tree, 0, entrySize);
    tree->anims = anims;
    return tree;
}

void __cdecl XAnimFreeTree(
                XAnimTree_s *tree,
                void (__cdecl *Free)(void *, int, scriptInstance_t),
                scriptInstance_t inst)
{
    int v3; // eax

    if ( !tree && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 824, 0, "%s", "tree") )
        __debugbreak();
    if ( !tree->anims
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 825, 0, "%s", "tree->anims") )
    {
        __debugbreak();
    }
    if ( !tree->anims->size
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 827, 0, "%s", "size") )
    {
        __debugbreak();
    }
    XAnimClearTree(tree);
    if ( Free )
    {
        v3 = XAnimTreeSize();
        Free(tree, v3, inst);
    }
}

XAnim_s *__cdecl XAnimGetAnims(const XAnimTree_s *tree)
{
    if ( !tree && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 883, 0, "%s", "tree") )
        __debugbreak();
    return tree->anims;
}

void __cdecl XAnimInitModelMap(XModel *const *models, unsigned int numModels, XModelNameMap *modelMap)
{
    unsigned int boneIndex; // [esp+0h] [ebp-20h]
    unsigned __int16 boneName; // [esp+4h] [ebp-1Ch]
    unsigned int hash; // [esp+8h] [ebp-18h]
    XModel *model; // [esp+Ch] [ebp-14h]
    unsigned int boneCount; // [esp+10h] [ebp-10h]
    unsigned int localBoneIndex; // [esp+14h] [ebp-Ch]
    unsigned int i; // [esp+18h] [ebp-8h]
    unsigned __int16 *boneNames; // [esp+1Ch] [ebp-4h]

    memset(modelMap, 0, 0x800u);
    boneIndex = 0;
    for ( i = 0; i < numModels; ++i )
    {
        model = models[i];
        boneNames = model->localBoneNames;
        boneCount = model->numBones;
        if ( boneCount >= 0xA0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        931,
                        0,
                        "%s",
                        "boneCount < DOBJ_MAX_PARTS") )
        {
            __debugbreak();
        }
        for ( localBoneIndex = 0; localBoneIndex < boneCount; ++localBoneIndex )
        {
            boneName = boneNames[localBoneIndex];
            iassert(boneName);
            for ( hash = boneName & 0x1FF; modelMap[hash].name; hash = ((_WORD)hash + 1) & 0x1FF )
                ;
            modelMap[hash].index = boneIndex;
            modelMap[hash].name = boneName;
            ++boneIndex;
        }
    }
}

unsigned int __cdecl XAnimGetAnimMap(const XAnimParts *parts, const XModelNameMap *modelMap)
{
    int i; // [esp+0h] [ebp-E0h]
    unsigned int boneIndex; // [esp+4h] [ebp-DCh]
    unsigned int hash; // [esp+Ch] [ebp-D4h]
    unsigned int partIndex; // [esp+10h] [ebp-D0h]
    unsigned int boneCount; // [esp+14h] [ebp-CCh]
    XAnimToXModel animToModel; // [esp+18h] [ebp-C8h] BYREF
    unsigned __int16 *partNames; // [esp+D8h] [ebp-8h]
    unsigned int partName; // [esp+DCh] [ebp-4h]

    for ( i = 0; i < 5; ++i )
        animToModel.partBits.array[i] = 0;
    if ( !parts && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 965, 0, "%s", "parts") )
        __debugbreak();
    memset(&animToModel, 0, 20);
    boneCount = parts->boneCount[9];
    partNames = parts->names;
    for ( partIndex = 0; partIndex < boneCount; ++partIndex )
    {
        partName = partNames[partIndex];
        for ( hash = partName & 0x1FF; ; hash = ((_WORD)hash + 1) & 0x1FF )
        {
            if ( !modelMap[hash].name )
            {
                animToModel.boneIndex[partIndex] = -97;
                goto LABEL_8;
            }
            if ( partName == modelMap[hash].name )
                break;
        }
        boneIndex = modelMap[hash].index;
        animToModel.boneIndex[partIndex] = boneIndex;
        //bitarray<160>::setBit(&animToModel.partBits, boneIndex);
        animToModel.partBits.setBit(boneIndex);
LABEL_8:
        ;
    }
    animToModel.boneCount = boneCount;
    if ( (unsigned __int8)boneCount != boneCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    1005,
                    0,
                    "%s",
                    "animToModel.boneCount == boneCount") )
    {
        __debugbreak();
    }
    return SL_GetStringOfSize(SCRIPTINSTANCE_SERVER, (char *)&animToModel, 0, boneCount + 21, 11);
}

void __cdecl XAnimCalcRelDeltaParts(
                const XAnimParts *parts,
                float weightScale,
                float time1,
                float time2,
                XAnimSimpleRotPos *rotPos,
                int quatIndex)
{
    unsigned __int16 *v6; // [esp+18h] [ebp-ACh]
    unsigned __int16 *pBigTrans; // [esp+1Ch] [ebp-A8h]
    unsigned __int8 *v8; // [esp+20h] [ebp-A4h]
    unsigned __int8 *pSmallTrans; // [esp+24h] [ebp-A0h]
    float sizeVec_4; // [esp+2Ch] [ebp-98h]
    float sizeVec_8; // [esp+30h] [ebp-94h]
    float toVec; // [esp+38h] [ebp-8Ch]
    float toVec_4; // [esp+3Ch] [ebp-88h]
    float toVec_8; // [esp+40h] [ebp-84h]
    float fromVec; // [esp+58h] [ebp-6Ch]
    float fromVec_4; // [esp+5Ch] [ebp-68h]
    float fromVec_8; // [esp+60h] [ebp-64h]
    float fromVec_12; // [esp+64h] [ebp-60h]
    float pos[3]; // [esp+68h] [ebp-5Ch] BYREF
    float rotWeightScale; // [esp+74h] [ebp-50h]
    const XAnimDeltaPart *part; // [esp+78h] [ebp-4Ch]
    const XAnimPartTrans *trans; // [esp+7Ch] [ebp-48h]
    float4 vec1; // [esp+80h] [ebp-44h] BYREF
    float Q[2][2]; // [esp+90h] [ebp-34h] BYREF
    float4 vec2; // [esp+A0h] [ebp-24h] BYREF
    float4 vec; // [esp+B0h] [ebp-14h]

    XAnim_CalcDeltaForTime(parts, time1, Q[0], &vec1);
    XAnim_CalcDeltaForTime(parts, time2, Q[1], &vec2);
    if ( parts->bLoop && time1 > time2 )
    {
        part = parts->deltaPart;
        trans = part->trans;
        if ( trans )
        {
            if ( trans->size )
            {
                if ( trans->smallTrans )
                {
                    pSmallTrans = (unsigned __int8 *)trans->u.frames.frames._1;
                    fromVec = (float)*pSmallTrans;
                    fromVec_4 = (float)pSmallTrans[1];
                    fromVec_8 = (float)pSmallTrans[2];
                    fromVec_12 = 0.0f;

                    v8 = &pSmallTrans[3 * trans->size];
                    toVec = (float)*v8;
                    toVec_4 = (float)v8[1];
                    toVec_8 = (float)v8[2];
                }
                else
                {
                    pBigTrans = (unsigned __int16 *)trans->u.frames.frames._1;
                    fromVec = (float)*pBigTrans;
                    fromVec_4 = (float)pBigTrans[1];
                    fromVec_8 = (float)pBigTrans[2];
                    fromVec_12 = 0.0f;
                    v6 = &pBigTrans[3 * trans->size];
                    toVec = (float)*v6;
                    toVec_4 = (float)v6[1];
                    toVec_8 = (float)v6[2];
                }
                sizeVec_4 = trans->u.frames.size[1];
                sizeVec_8 = trans->u.frames.size[2];
                vec2.v[0] = (float)(trans->u.frames.size[0] * (float)(toVec - fromVec)) + vec2.v[0];
                vec2.v[1] = (float)(sizeVec_4 * (float)(toVec_4 - fromVec_4)) + vec2.v[1];
                vec2.v[2] = (float)(sizeVec_8 * (float)(toVec_8 - fromVec_8)) + vec2.v[2];
                vec2.v[3] = (float)(0.0 * (float)(0.0 - fromVec_12)) + vec2.v[3];
            }
        }
    }
    rotWeightScale = weightScale * 9.3137942e-10;
    rotPos->rot[0] = (float)((float)((float)(Q[1][0] * Q[0][1]) - (float)(Q[1][1] * Q[0][0]))
                                                 * (float)(weightScale * 9.3137942e-10))
                                 + rotPos->rot[0];
    rotPos->rot[1] = (float)((float)((float)(Q[1][0] * Q[0][0]) + (float)(Q[1][1] * Q[0][1])) * rotWeightScale)
                                 + rotPos->rot[1];
    vec.v[0] = weightScale * (float)(vec2.v[0] - vec1.v[0]);
    vec.v[1] = weightScale * (float)(vec2.v[1] - vec1.v[1]);
    vec.v[2] = weightScale * (float)(vec2.v[2] - vec1.v[2]);
    vec.v[3] = weightScale * (float)(vec2.v[3] - vec1.v[3]);
    pos[0] = vec.v[0];
    pos[1] = vec.v[1];
    pos[2] = vec.v[2];
    TransformToQuatRefFrame(Q[quatIndex], pos);
    rotPos->posWeight = rotPos->posWeight + weightScale;
    rotPos->pos[0] = rotPos->pos[0] + pos[0];
    rotPos->pos[1] = rotPos->pos[1] + pos[1];
    rotPos->pos[2] = rotPos->pos[2] + pos[2];
}

void __cdecl TransformToQuatRefFrame(const float *rot, float *trans)
{
    float r; // [esp+0h] [ebp-10h]
    float ra; // [esp+0h] [ebp-10h]
    float zz; // [esp+4h] [ebp-Ch]
    float zza; // [esp+4h] [ebp-Ch]
    float zw; // [esp+8h] [ebp-8h]
    float temp; // [esp+Ch] [ebp-4h]

    zz = *rot * *rot;
    r = (float)(rot[1] * rot[1]) + zz;
    if ( r != 0.0 )
    {
        ra = 2.0 / r;
        zza = zz * ra;
        zw = (float)(*rot * rot[1]) * ra;
        temp = (float)((float)(1.0 - zza) * *trans) + (float)(zw * trans[1]);
        trans[1] = trans[1] - (float)((float)(zw * *trans) + (float)(zza * trans[1]));
        *trans = temp;
    }
}

void __cdecl XAnimCalcAbsDeltaParts(const XAnimParts *parts, float weightScale, float time, XAnimSimpleRotPos *rotPos)
{
    __int64 pos; // [esp+18h] [ebp-28h]
    float pos_8; // [esp+20h] [ebp-20h]
    float Q[2]; // [esp+24h] [ebp-1Ch] BYREF
    float4 vec; // [esp+2Ch] [ebp-14h] BYREF

    XAnim_CalcDeltaForTime(parts, time, Q, &vec);
    rotPos->rot[0] = (float)((float)(weightScale * 0.000030518509) * Q[0]) + rotPos->rot[0];
    rotPos->rot[1] = (float)((float)(weightScale * 0.000030518509) * Q[1]) + rotPos->rot[1];
    vec.v[0] = weightScale * vec.v[0];
    vec.v[1] = weightScale * vec.v[1];
    vec.v[2] = weightScale * vec.v[2];
    vec.v[3] = weightScale * vec.v[3];
    pos = *(_QWORD *)vec.v;
    pos_8 = vec.v[2];
    rotPos->posWeight = rotPos->posWeight + weightScale;
    rotPos->pos[0] = rotPos->pos[0] + *(float *)&pos;
    rotPos->pos[1] = rotPos->pos[1] + *((float *)&pos + 1);
    rotPos->pos[2] = rotPos->pos[2] + pos_8;
}

void __cdecl XAnimAddClientNotify(unsigned int notetrackName, float frac, unsigned int notifyType)
{
    ClientNotifyData ourNote; // [esp+0h] [ebp-10h] BYREF

    if ( Sys_IsMainThread() )
    {
        if ( !Sys_IsMainThread()
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1542, 0, "%s", "Sys_IsMainThread()") )
        {
            __debugbreak();
        }
        if ( g_clientList )
        {
            ourNote.notetrackName = notetrackName;
            ourNote.timeFrac = frac;
            ourNote.notifyName = 0;
            ourNote.notifyType = notifyType;
            AddSignalToNoteList(g_clientList, &ourNote);
        }
    }
}

void __cdecl AddSignalToNoteList(XAnimClientNotifyList *pList, const ClientNotifyData *ourNote)
{
    //XAnimClientNotifyList::AddNotify(pList, ourNote);
    pList->AddNotify(ourNote);
}

void __cdecl XAnimAddClientNotifyNamed(unsigned int notetrackName, float frac, unsigned int notifyName)
{
    ClientNotifyData ourNote; // [esp+0h] [ebp-10h] BYREF

    if ( Sys_IsMainThread() )
    {
        if ( !Sys_IsMainThread()
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1564, 0, "%s", "Sys_IsMainThread()") )
        {
            __debugbreak();
        }
        if ( g_clientList )
        {
            ourNote.notetrackName = notetrackName;
            ourNote.timeFrac = frac;
            ourNote.notifyName = notifyName;
            ourNote.notifyType = 0;
            AddSignalToNoteList(g_clientList, &ourNote);
        }
    }
}

void    XAnimResetAnimMap(const DObj *obj, unsigned int infoIndex)
{
    XModelNameMap modelMap[512]; // [esp+0h] [ebp-800h]
    //XModelNameMap retaddr; // [esp+800h] [ebp+0h]

    //modelMap[509] = a1;
    //modelMap[510] = retaddr;
    XAnimInitModelMap(obj->localModels, obj->numModels, modelMap);
    XAnimResetAnimMap_r(modelMap, infoIndex);
}

void __cdecl XAnimResetAnimMap_r(XModelNameMap *modelMap, unsigned int infoIndex)
{
    XAnimInfo *info; // [esp+0h] [ebp-8h]
    unsigned int childInfoIndex; // [esp+4h] [ebp-4h]

    if ( (!infoIndex || infoIndex >= 0x1000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    2528,
                    0,
                    "%s\n\t(infoIndex) = %i",
                    "(infoIndex && (infoIndex < 4096))",
                    infoIndex) )
    {
        __debugbreak();
    }
    info = &g_xAnimInfo[infoIndex];
    if ( info->animToModel )
    {
        if ( info->children )
        {
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2537, 0, "%s", "!info->children") )
                __debugbreak();
        }
        XAnimResetAnimMapLeaf(modelMap, infoIndex);
    }
    else
    {
        for ( childInfoIndex = info->children; childInfoIndex; childInfoIndex = g_xAnimInfo[childInfoIndex].next )
        {
            if ( childInfoIndex >= 0x1000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                            2549,
                            0,
                            "%s\n\t(childInfoIndex) = %i",
                            "(childInfoIndex && (childInfoIndex < 4096))",
                            childInfoIndex) )
            {
                __debugbreak();
            }
            XAnimResetAnimMap_r(modelMap, childInfoIndex);
        }
    }
}

void __cdecl XAnimResetAnimMapLeaf(const XModelNameMap *modelMap, unsigned int infoIndex)
{
    char *animToModel2; // [esp+4h] [ebp-8h]
    unsigned int animToModel; // [esp+8h] [ebp-4h]

    if ( (!infoIndex || infoIndex >= 0x1000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    2487,
                    0,
                    "%s\n\t(infoIndex) = %i",
                    "(infoIndex && (infoIndex < 4096))",
                    infoIndex) )
    {
        __debugbreak();
    }
    animToModel = g_xAnimInfo[infoIndex].animToModel;
    if ( !g_xAnimInfo[infoIndex].animToModel
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2500, 0, "%s", "animToModel") )
    {
        __debugbreak();
    }
    animToModel2 = SL_ConvertToString(animToModel, SCRIPTINSTANCE_SERVER);
    g_xAnimInfo[infoIndex].animToModel = XAnimGetAnimMap(g_xAnimInfo[infoIndex].parts, modelMap);
    SL_RemoveRefToStringOfSize(SCRIPTINSTANCE_SERVER, animToModel, (unsigned __int8)animToModel2[20] + 21);
}

double __cdecl XAnimGetLength(const XAnim_s *anims, unsigned int animIndex)
{
    XAnimParts *parts; // [esp+8h] [ebp-4h]

    if ( !anims && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2741, 0, "%s", "anims") )
        __debugbreak();
    if ( animIndex >= anims->size
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    2742,
                    0,
                    "animIndex < anims->size\n\t%i, %i",
                    animIndex,
                    anims->size) )
    {
        __debugbreak();
    }
    if ( (const XAnim_s *)((char *)anims + 16 * animIndex) == (const XAnim_s *)-24
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2745, 0, "%s", "entry") )
    {
        __debugbreak();
    }
    if ( !IsLeafNode(&anims->entries[animIndex])
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2746, 0, "%s", "IsLeafNode( entry )") )
    {
        __debugbreak();
    }
    parts = anims->entries[animIndex].parts;
    if ( !parts && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2749, 0, "%s", "parts") )
        __debugbreak();
    return (double)parts->numframes / parts->framerate;
}

int __cdecl XAnimGetFrameCount(const XAnim_s *anims, unsigned int animIndex)
{
    XAnimParts *parts; // [esp+4h] [ebp-4h]

    if ( !anims && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2765, 0, "%s", "anims") )
        __debugbreak();
    if ( animIndex >= anims->size
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    2766,
                    0,
                    "animIndex < anims->size\n\t%i, %i",
                    animIndex,
                    anims->size) )
    {
        __debugbreak();
    }
    if ( (const XAnim_s *)((char *)anims + 16 * animIndex) == (const XAnim_s *)-24
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2769, 0, "%s", "entry") )
    {
        __debugbreak();
    }
    if ( !IsLeafNode(&anims->entries[animIndex])
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2770, 0, "%s", "IsLeafNode( entry )") )
    {
        __debugbreak();
    }
    parts = anims->entries[animIndex].parts;
    if ( !parts && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2773, 0, "%s", "parts") )
        __debugbreak();
    return parts->numframes;
}

int __cdecl XAnimGetLengthMsec(const XAnim_s *anims, unsigned int anim)
{
    return (int)(XAnimGetLength(anims, anim) * 1000.0);
}

unsigned int __cdecl XAnimGetInfoIndex(const XAnimTree_s *tree, unsigned int animIndex)
{
    if ( !tree && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2858, 0, "%s", "tree") )
        __debugbreak();
    if ( tree->children )
        return XAnimGetInfoIndex_r(animIndex, tree->children);
    else
        return 0;
}

unsigned int __cdecl XAnimGetInfoIndex_r(unsigned int animIndex, unsigned int infoIndex)
{
    unsigned int prevAnimIndex; // [esp+4h] [ebp-Ch]
    unsigned int nextAnimIndex; // [esp+8h] [ebp-8h]
    unsigned int resultInfoIndex; // [esp+Ch] [ebp-4h]
    unsigned int infoIndexa; // [esp+1Ch] [ebp+Ch]

    if ( (!infoIndex || infoIndex >= 0x1000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    2806,
                    0,
                    "%s\n\t(infoIndex) = %i",
                    "(infoIndex && (infoIndex < 4096))",
                    infoIndex) )
    {
        __debugbreak();
    }
    if ( g_xAnimInfo[infoIndex].animIndex == animIndex )
        return infoIndex;
    prevAnimIndex = 0;
    for ( infoIndexa = g_xAnimInfo[infoIndex].children; infoIndexa; infoIndexa = g_xAnimInfo[infoIndexa].next )
    {
        nextAnimIndex = g_xAnimInfo[infoIndexa].animIndex;
        if ( nextAnimIndex != prevAnimIndex )
        {
            resultInfoIndex = XAnimGetInfoIndex_r(animIndex, infoIndexa);
            if ( resultInfoIndex )
                return resultInfoIndex;
            prevAnimIndex = nextAnimIndex;
        }
    }
    return 0;
}

double __cdecl XAnimGetTime(const XAnimTree_s *tree, unsigned int animIndex)
{
    unsigned int infoIndex; // [esp+4h] [ebp-4h]

    if ( !tree && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2873, 0, "%s", "tree") )
        __debugbreak();
    if ( !tree->anims
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2874, 0, "%s", "tree->anims") )
    {
        __debugbreak();
    }
    if ( animIndex >= tree->anims->size
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    2875,
                    0,
                    "animIndex < tree->anims->size\n\t%i, %i",
                    animIndex,
                    tree->anims->size) )
    {
        __debugbreak();
    }
    infoIndex = XAnimGetInfoIndex(tree, animIndex);
    if ( infoIndex >= 0x1000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    2878,
                    0,
                    "%s\n\t(infoIndex) = %i",
                    "(infoIndex < 4096)",
                    infoIndex) )
    {
        __debugbreak();
    }
    if ( infoIndex )
        return g_xAnimInfo[infoIndex].state.currentAnimTime;
    else
        return 0.0f;
}

double __cdecl XAnimGetWeight(const XAnimTree_s *tree, unsigned int animIndex)
{
    unsigned int infoIndex; // [esp+4h] [ebp-4h]

    if ( !tree && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2892, 0, "%s", "tree") )
        __debugbreak();
    if ( !tree->anims
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2893, 0, "%s", "tree->anims") )
    {
        __debugbreak();
    }
    if ( animIndex >= tree->anims->size
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    2894,
                    0,
                    "animIndex < tree->anims->size\n\t%i, %i",
                    animIndex,
                    tree->anims->size) )
    {
        __debugbreak();
    }
    infoIndex = XAnimGetInfoIndex(tree, animIndex);
    if ( infoIndex >= 0x1000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    2897,
                    0,
                    "%s\n\t(infoIndex) = %i",
                    "(infoIndex < 4096)",
                    infoIndex) )
    {
        __debugbreak();
    }
    if ( infoIndex )
        return g_xAnimInfo[infoIndex].state.weight;
    else
        return 0.0f;
}

bool __cdecl XAnimHasFinished(const XAnimTree_s *tree, unsigned int animIndex)
{
    XAnimState *state; // [esp+4h] [ebp-8h]
    unsigned int infoIndex; // [esp+8h] [ebp-4h]

    if ( !tree && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2912, 0, "%s", "tree") )
        __debugbreak();
    if ( !tree->anims
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2913, 0, "%s", "tree->anims") )
    {
        __debugbreak();
    }
    if ( animIndex >= tree->anims->size
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    2914,
                    0,
                    "animIndex < tree->anims->size\n\t%i, %i",
                    animIndex,
                    tree->anims->size) )
    {
        __debugbreak();
    }
    infoIndex = XAnimGetInfoIndex(tree, animIndex);
    if ( !infoIndex )
        return 1;
    if ( infoIndex >= 0x1000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    2920,
                    0,
                    "%s\n\t(infoIndex) = %i",
                    "(infoIndex && (infoIndex < 4096))",
                    infoIndex) )
    {
        __debugbreak();
    }
    state = &g_xAnimInfo[infoIndex].state;
    return state->oldTime > state->currentAnimTime
            || state->currentAnimTime == 1.0
            || state->cycleCount > state->oldCycleCount;
}

int __cdecl XAnimGetNumChildren(const XAnim_s *anims, unsigned int animIndex)
{
    if ( !anims && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3061, 0, "%s", "anims") )
        __debugbreak();
    if ( animIndex >= anims->size
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    3062,
                    0,
                    "animIndex < anims->size\n\t%i, %i",
                    animIndex,
                    anims->size) )
    {
        __debugbreak();
    }
    return anims->entries[animIndex].numAnims;
}

unsigned int __cdecl XAnimGetChildAt(const XAnim_s *anims, unsigned int animIndex, unsigned int childIndex)
{
    if ( !anims && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3075, 0, "%s", "anims") )
        __debugbreak();
    if ( animIndex >= anims->size
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    3076,
                    0,
                    "animIndex < anims->size\n\t%i, %i",
                    animIndex,
                    anims->size) )
    {
        __debugbreak();
    }
    if ( childIndex >= anims->entries[animIndex].numAnims
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    3077,
                    0,
                    "childIndex < anims->entries[animIndex].numAnims\n\t%i, %i",
                    childIndex,
                    anims->entries[animIndex].numAnims) )
    {
        __debugbreak();
    }
    return childIndex + anims->entries[animIndex].animParent.children;
}

const char *__cdecl XAnimGetAnimName(const XAnim_s *anims, unsigned int animIndex)
{
    if ( !anims && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3092, 0, "%s", "anims") )
        __debugbreak();
    if ( animIndex >= anims->size
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    3093,
                    0,
                    "animIndex < anims->size\n\t%i, %i",
                    animIndex,
                    anims->size) )
    {
        __debugbreak();
    }
    if ( IsLeafNode(&anims->entries[animIndex]) )
        return anims->entries[animIndex].parts->name;
    else
        return "";
}

char *__cdecl XAnimGetAnimDebugName(const XAnim_s *anims, unsigned int animIndex)
{
    bool isDefault; // [esp+Bh] [ebp-15h]
    XAnimParts *parts; // [esp+Ch] [ebp-14h]
    const char *debugName; // [esp+14h] [ebp-Ch]
    const XAnimEntry *anim; // [esp+18h] [ebp-8h]

    if ( !anims && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3150, 0, "%s", "anims") )
        __debugbreak();
    if ( animIndex >= anims->size
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    3151,
                    0,
                    "animIndex < anims->size\n\t%i, %i",
                    animIndex,
                    anims->size) )
    {
        __debugbreak();
    }
    anim = &anims->entries[animIndex];
    if ( anims->debugAnimNames )
    {
        if ( !anims->debugAnimNames[animIndex]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        3156,
                        0,
                        "%s",
                        "anims->debugAnimNames[animIndex]") )
        {
            __debugbreak();
        }
        debugName = anims->debugAnimNames[animIndex];
        if ( IsLeafNode(anim)
            && ((parts = anims->entries[animIndex].parts, !useFastFile->current.enabled)
                ? (isDefault = parts->isDefault)
                : (isDefault = DB_IsXAssetDefault(ASSET_TYPE_XANIMPARTS, parts->name)),
                    isDefault) )
        {
            return va("^3%s (missing)", debugName);
        }
        else
        {
            return (char *)debugName;
        }
    }
    else if ( IsLeafNode(anim) )
    {
        return (char *)anims->entries[animIndex].parts->name;
    }
    else
    {
        return va("%i", animIndex);
    }
}

unsigned int __cdecl XAnimGetAnimTreeSize(const XAnim_s *anims)
{
    if ( !anims && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3196, 0, "%s", "anims") )
        __debugbreak();
    return anims->size;
}

void __cdecl XAnimInitInfo(XAnimInfo *info)
{
    info->state.currentAnimTime = 0.0;
    info->state.oldTime = 0.0;
    *(unsigned int *)&info->state.cycleCount = 0;
    info->state.goalTime = 0.0;
    info->state.goalWeight = 0.0;
    info->state.weight = 0.0;
    info->state.rate = 0.0;
    *(unsigned int *)&info->state.instantWeightChange = 0;
    info->notifyName = 0;
    info->notifyIndex = -1;
    info->notifyChild = 0;
    info->notifyType = 0;
}

void __cdecl XAnimUpdateOldTime(
                DObj *obj,
                unsigned int infoIndex,
                XAnimState *syncState,
                float dtime,
                bool parentHasWeight,
                bool *childHasTimeForParent)
{
    bool v6; // [esp+10h] [ebp-20h]
    XAnimState *state; // [esp+14h] [ebp-1Ch]
    unsigned int nextInfoIndex; // [esp+18h] [ebp-18h]
    XAnimInfo *info; // [esp+1Ch] [ebp-14h]
    XAnimTree_s *tree; // [esp+20h] [ebp-10h]
    bool childHasTime; // [esp+27h] [ebp-9h] BYREF
    unsigned int childInfoIndex; // [esp+28h] [ebp-8h]
    XAnimParts *parts; // [esp+2Ch] [ebp-4h]

    tree = obj->localTree;
    if ( !obj->localTree
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2308, 0, "%s", "tree") )
    {
        __debugbreak();
    }
    if ( !tree->anims
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2309, 0, "%s", "tree->anims") )
    {
        __debugbreak();
    }
    if ( dtime < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    2310,
                    0,
                    "%s\n\t(dtime) = %g",
                    "(dtime >= 0)",
                    dtime) )
    {
        __debugbreak();
    }
    if ( (!infoIndex || infoIndex >= 0x1000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    2312,
                    0,
                    "%s\n\t(infoIndex) = %i",
                    "(infoIndex && (infoIndex < 4096))",
                    infoIndex) )
    {
        __debugbreak();
    }
    info = &g_xAnimInfo[infoIndex];
    state = &info->state;
    if ( parentHasWeight && dtime < g_xAnimInfo[infoIndex].state.goalTime )
    {
        info->state.weight = (float)((float)((float)(info->state.goalWeight - info->state.weight) * dtime)
                                                             / info->state.goalTime)
                                             + info->state.weight;
        if ( g_xAnimInfo[infoIndex].state.weight < 0.0000010000001 )
            g_xAnimInfo[infoIndex].state.weight = g_xAnimInfo[infoIndex].state.goalWeight * 0.001;
        g_xAnimInfo[infoIndex].state.goalTime = g_xAnimInfo[infoIndex].state.goalTime - dtime;
    }
    else
    {
        g_xAnimInfo[infoIndex].state.weight = g_xAnimInfo[infoIndex].state.goalWeight;
        g_xAnimInfo[infoIndex].state.goalTime = 0.0f;
    }
    v6 = parentHasWeight && g_xAnimInfo[infoIndex].state.weight != 0.0;
    g_xAnimInfo[infoIndex].state.instantWeightChange = 0;
    if ( info->animToModel )
    {
        parts = info->parts;
        if ( !parts && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2342, 0, "%s", "parts") )
            __debugbreak();
        childHasTime = parts->frequency != 0.0;
    }
    else
    {
        childHasTime = 0;
        if ( (info->animParent.flags & 4) != 0 )
            syncState = &g_xAnimInfo[infoIndex].state;
        for ( childInfoIndex = info->children; childInfoIndex; childInfoIndex = nextInfoIndex )
        {
            nextInfoIndex = g_xAnimInfo[childInfoIndex].next;
            XAnimUpdateOldTime(obj, childInfoIndex, syncState, dtime, v6, &childHasTime);
        }
    }
    if ( v6 && childHasTime )
    {
        *childHasTimeForParent = 1;
    }
    else if ( info->animToModel || (info->animParent.flags & 4) == 0 )
    {
        if ( state->currentAnimTime != syncState->currentAnimTime
            || g_xAnimInfo[infoIndex].state.cycleCount != syncState->cycleCount )
        {
            state->currentAnimTime = syncState->currentAnimTime;
            g_xAnimInfo[infoIndex].state.cycleCount = syncState->cycleCount;
            info->notifyIndex = -1;
        }
    }
    else
    {
        XAnimInitTime(tree, infoIndex, 0.0, -1);
    }
    g_xAnimInfo[infoIndex].state.oldTime = g_xAnimInfo[infoIndex].state.currentAnimTime;
    g_xAnimInfo[infoIndex].state.oldCycleCount = g_xAnimInfo[infoIndex].state.cycleCount;
}

unsigned int __cdecl XAnimInitTime(XAnimTree_s *tree, unsigned int infoIndex, float goalTime, int cmdIndex)
{
    unsigned int toInfoIndex; // [esp+14h] [ebp-4h]

    if ( (!infoIndex || infoIndex >= 0x1000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    2267,
                    0,
                    "%s\n\t(infoIndex) = %i",
                    "(infoIndex && (infoIndex < 4096))",
                    infoIndex) )
    {
        __debugbreak();
    }
    if ( g_xAnimInfo[infoIndex].state.currentAnimTime == 0.0 && !g_xAnimInfo[infoIndex].state.cycleCount )
        return infoIndex;
    if ( g_xAnimInfo[infoIndex].state.cycleCount != g_xAnimInfo[infoIndex].state.oldCycleCount )
        return infoIndex;
    if ( goalTime == 0.0 || g_xAnimInfo[infoIndex].state.weight == 0.0 )
    {
        XAnimResetTime(infoIndex);
        return infoIndex;
    }
    else
    {
        toInfoIndex = XAnimCloneInitTime(tree, infoIndex, g_xAnimInfo[infoIndex].parent, cmdIndex);
        XAnimClearTreeGoalWeightsInternal(tree, infoIndex, goalTime, 1, cmdIndex);
        return toInfoIndex;
    }
}

void __cdecl XAnimResetTime(unsigned int infoIndex)
{
    unsigned int childInfoIndex; // [esp+0h] [ebp-4h]

    XAnimResetTimeInternal(infoIndex);
    for ( childInfoIndex = g_xAnimInfo[infoIndex].children; childInfoIndex; childInfoIndex = g_xAnimInfo[childInfoIndex].next )
        XAnimResetTime(childInfoIndex);
}

void __cdecl XAnimResetTimeInternal(unsigned int infoIndex)
{
    XAnimState *state; // [esp+0h] [ebp-8h]

    if ( (!infoIndex || infoIndex >= 0x1000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    2207,
                    0,
                    "%s\n\t(infoIndex) = %i",
                    "(infoIndex && (infoIndex < 4096))",
                    infoIndex) )
    {
        __debugbreak();
    }
    state = &g_xAnimInfo[infoIndex].state;
    state->currentAnimTime = 0.0f;
    state->cycleCount = 0;
    state->oldTime = 0.0f;
    state->oldCycleCount = 0;
    g_xAnimInfo[infoIndex].notifyIndex = -1;
}

unsigned int __cdecl XAnimCloneInitTime(
                XAnimTree_s *tree,
                unsigned int infoIndex,
                unsigned int parentIndex,
                int cmdIndex)
{
    XAnimInfo *toInfo; // [esp+0h] [ebp-14h]
    unsigned int toInfoIndex; // [esp+4h] [ebp-10h]
    XAnimInfo *fromInfo; // [esp+8h] [ebp-Ch]
    unsigned int animToModel; // [esp+Ch] [ebp-8h]
    unsigned int childInfoIndex; // [esp+10h] [ebp-4h]

    if ( (!infoIndex || infoIndex >= 0x1000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    2238,
                    0,
                    "%s\n\t(infoIndex) = %i",
                    "(infoIndex && (infoIndex < 4096))",
                    infoIndex) )
    {
        __debugbreak();
    }
    fromInfo = &g_xAnimInfo[infoIndex];
    animToModel = fromInfo->animToModel;
    if ( fromInfo->animToModel )
        SL_AddRefToString(animToModel, SCRIPTINSTANCE_SERVER);
    toInfoIndex = XAnimAllocInfoWithParent(tree, animToModel, fromInfo->animIndex, parentIndex, 0);
    toInfo = &g_xAnimInfo[toInfoIndex];
    XAnimCloneAnimInfo(tree, fromInfo, toInfo);
    XAnimResetTimeInternal(toInfoIndex);
    toInfo->state.weight = 0.0f;
    toInfo->state.instantWeightChange = 1;
    for ( childInfoIndex = g_xAnimInfo[infoIndex].children; childInfoIndex; childInfoIndex = g_xAnimInfo[childInfoIndex].next )
        XAnimCloneInitTime(tree, childInfoIndex, toInfoIndex, cmdIndex);
    return toInfoIndex;
}

void __cdecl DObjInitServerTime(DObj *obj, float dtime)
{
    XAnimState syncState; // [esp+Ch] [ebp-28h] BYREF
    XAnimTree_s *tree; // [esp+2Ch] [ebp-8h]
    bool childHasTime; // [esp+33h] [ebp-1h] BYREF

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3265, 0, "%s", "obj") )
        __debugbreak();
    tree = obj->localTree;
    if ( tree )
    {
        if ( tree->children )
        {
            syncState.currentAnimTime = 0.0f;
            syncState.cycleCount = 0;
            XAnimUpdateOldTime(obj, tree->children, &syncState, dtime, 1, &childHasTime);
        }
    }
}

void __cdecl DObjUpdateClientInfo(DObj *obj, float dtime, int notifyFlags)
{
    XAnimState syncState; // [esp+Ch] [ebp-28h] BYREF
    XAnimTree_s *tree; // [esp+2Ch] [ebp-8h]
    bool childHasTime; // [esp+33h] [ebp-1h] BYREF

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3296, 0, "%s", "obj") )
        __debugbreak();
    if ( dtime < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3297, 0, "%s", "dtime >= 0") )
    {
        __debugbreak();
    }
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3298, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    tree = obj->localTree;
    if ( tree && tree->children )
    {
        syncState.currentAnimTime = 0.0f;
        syncState.cycleCount = 0;
        XAnimUpdateOldTime(obj, tree->children, &syncState, dtime, 1, &childHasTime);
        if ( tree->children )
            XAnimUpdateTimeAndNotetrack(obj, tree->children, dtime, notifyFlags);
    }
}

void __cdecl XAnimUpdateTimeAndNotetrack(const DObj *obj, unsigned int infoIndex, float dtime, int notifyFlags)
{
    unsigned int nextInfoIndex; // [esp+Ch] [ebp-Ch]
    XAnimInfo *info; // [esp+10h] [ebp-8h]
    XAnimTree_s *tree; // [esp+14h] [ebp-4h]
    unsigned int infoIndexa; // [esp+24h] [ebp+Ch]
    float dtimea; // [esp+28h] [ebp+10h]
    float dtimeb; // [esp+28h] [ebp+10h]

    tree = obj->localTree;
    if ( !obj->localTree
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2001, 0, "%s", "tree") )
    {
        __debugbreak();
    }
    if ( dtime < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    2003,
                    0,
                    "%s\n\t(dtime) = %g",
                    "(dtime >= 0)",
                    dtime) )
    {
        __debugbreak();
    }
    if ( (!infoIndex || infoIndex >= 0x1000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    2004,
                    0,
                    "%s\n\t(infoIndex) = %i",
                    "(infoIndex && (infoIndex < 4096))",
                    infoIndex) )
    {
        __debugbreak();
    }
    info = &g_xAnimInfo[infoIndex];
    if ( info->state.weight == 0.0 )
    {
        XAnimCheckFreeInfo(obj->localTree, infoIndex, 0);
        return;
    }
    if ( g_xAnimInfo[infoIndex].state.goalWeight == 0.0 )
        notifyFlags = 0;
    if ( info->animToModel )
    {
        XAnimUpdateTimeAndNotetrackLeaf(obj, info->parts, infoIndex, dtime, notifyFlags);
        return;
    }
    if ( (info->animParent.flags & 3) != 0 )
    {
        dtimea = XAnimGetAverageRateFrequency(tree, infoIndex) * g_xAnimInfo[infoIndex].state.rate * dtime;
        if ( dtimea == 0.0 )
        {
LABEL_19:
            XAnimCheckFreeInfo(obj->localTree, infoIndex, 1);
            return;
        }
        XAnimUpdateTimeAndNotetrackSyncSubTree(obj, infoIndex, dtimea, notifyFlags);
    }
    else
    {
        if ( (info->animParent.flags & 4) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        2039,
                        0,
                        "%s",
                        "!(info->animParent.flags & XANIM_SYNC_ROOT)") )
        {
            __debugbreak();
        }
        if ( obj->entnum
            && info->notifyName
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        2040,
                        0,
                        "%s",
                        "!(obj->entnum && info->notifyName)") )
        {
            __debugbreak();
        }
        dtimeb = dtime * g_xAnimInfo[infoIndex].state.rate;
        if ( dtimeb == 0.0 )
            goto LABEL_19;
        for ( infoIndexa = info->children; infoIndexa; infoIndexa = nextInfoIndex )
        {
            nextInfoIndex = g_xAnimInfo[infoIndexa].next;
            XAnimUpdateTimeAndNotetrack(obj, infoIndexa, dtimeb, notifyFlags);
        }
    }
}

void __cdecl XAnimCheckFreeInfo(XAnimTree_s *tree, unsigned int infoIndex, int hasWeight)
{
    unsigned int nextInfoIndex; // [esp+4h] [ebp-Ch]
    XAnimInfo *info; // [esp+8h] [ebp-8h]
    unsigned int childInfoIndex; // [esp+Ch] [ebp-4h]

    if ( (!infoIndex || infoIndex >= 0x1000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    1322,
                    0,
                    "%s\n\t(infoIndex) = %i",
                    "(infoIndex && (infoIndex < 4096))",
                    infoIndex) )
    {
        __debugbreak();
    }
    info = &g_xAnimInfo[infoIndex];
    if ( info->state.weight == 0.0 )
        hasWeight = 0;
    for ( childInfoIndex = info->children; childInfoIndex; childInfoIndex = nextInfoIndex )
    {
        nextInfoIndex = g_xAnimInfo[childInfoIndex].next;
        XAnimCheckFreeInfo(tree, childInfoIndex, hasWeight);
    }
    if ( !info->children && !hasWeight && g_xAnimInfo[infoIndex].state.goalWeight == 0.0 )
        XAnimFreeInfo(tree, infoIndex);
}

void __cdecl XAnimFreeInfo(XAnimTree_s *tree, unsigned int infoIndex)
{
    XAnimInfo *info; // [esp+0h] [ebp-14h]
    unsigned int next; // [esp+4h] [ebp-10h]
    char *animToModel; // [esp+8h] [ebp-Ch]
    unsigned int prev; // [esp+Ch] [ebp-8h]

    if (!tree && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1205, 0, "%s", "tree"))
        __debugbreak();
    if ((!infoIndex || infoIndex >= 0x1000)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
            1207,
            0,
            "%s\n\t(infoIndex) = %i",
            "(infoIndex && (infoIndex < 4096))",
            infoIndex))
    {
        __debugbreak();
    }
    info = &g_xAnimInfo[infoIndex];

    if (bgs && bgs->AnimCmdRefCount)
    {
        bgs->AnimCmdRefCount(
            -1,
            infoIndex,
            bgs->time);
    }

    if (info->animToModel)
    {
        if (info->children
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1232, 0, "%s", "!info->children"))
        {
            __debugbreak();
        }
        if (!info->parts
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1235, 0, "%s", "parts"))
        {
            __debugbreak();
        }
        if (!info->animToModel
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1237, 0, "%s", "info->animToModel"))
        {
            __debugbreak();
        }
        animToModel = SL_ConvertToString(info->animToModel, SCRIPTINSTANCE_SERVER);
        SL_RemoveRefToStringOfSize(SCRIPTINSTANCE_SERVER, info->animToModel, (unsigned __int8)animToModel[20] + 21);
        info->animToModel = 0;
    }
    else
    {
        while (info->children)
            XAnimFreeInfo(tree, info->children);
    }
    prev = info->prev;
    next = info->next;
    if (info->prev)
    {
        g_xAnimInfo[prev].next = next;
    }
    else if (info->parent)
    {
        g_xAnimInfo[info->parent].children = next;
    }
    else
    {
        tree->children = next;
    }
    if (next)
        g_xAnimInfo[next].prev = prev;
    XAnimClearServerNotify(info, (scriptInstance_t)tree->inst);
    info->prev = 0;
    Sys_EnterCriticalSection(CRITSECT_XANIM_ALLOC);
    info->next = g_xAnimInfo[0].next;
    g_xAnimInfo[g_xAnimInfo[0].next].prev = infoIndex;
    g_xAnimInfo[0].next = infoIndex;
    Sys_LeaveCriticalSection(CRITSECT_XANIM_ALLOC);
}

void __cdecl XAnimClearServerNotify(XAnimInfo *info, scriptInstance_t inst)
{
    if ( info->notifyName )
    {
        SL_RemoveRefToString(inst, info->notifyName);
        info->notifyName = 0;
    }
    info->notifyIndex = -1;
}

double __cdecl XAnimGetAverageRateFrequency(const XAnimTree_s *tree, unsigned int infoIndex)
{
    const XAnimInfo *info; // [esp+Ch] [ebp-18h]
    const XAnimInfo *infoa; // [esp+Ch] [ebp-18h]
    float totalDtime; // [esp+10h] [ebp-14h]
    float totalWeight; // [esp+14h] [ebp-10h]
    float weight; // [esp+18h] [ebp-Ch]
    float frequency; // [esp+1Ch] [ebp-8h]
    const XAnimParts *parts; // [esp+20h] [ebp-4h]
    unsigned int infoIndexa; // [esp+30h] [ebp+Ch]

    if ( !tree && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1370, 0, "%s", "tree") )
        __debugbreak();
    if ( !tree->anims
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1371, 0, "%s", "tree->anims") )
    {
        __debugbreak();
    }
    info = &g_xAnimInfo[infoIndex];
    if ( info->animToModel )
    {
        parts = info->parts;
        if ( !parts && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1381, 0, "%s", "parts") )
            __debugbreak();
        return parts->frequency;
    }
    else
    {
        totalWeight = 0.0f;
        totalDtime = 0.0f;
        for ( infoIndexa = info->children; infoIndexa; infoIndexa = infoa->next )
        {
            if ( infoIndexa >= 0x1000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                            1390,
                            0,
                            "%s\n\t(infoIndex) = %i",
                            "(infoIndex && (infoIndex < 4096))",
                            infoIndexa) )
            {
                __debugbreak();
            }
            infoa = &g_xAnimInfo[infoIndexa];
            weight = infoa->state.weight;
            if ( weight < 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                            1393,
                            0,
                            "%s\n\t(weight) = %g",
                            "(weight >= 0.0f)",
                            weight) )
            {
                __debugbreak();
            }
            if ( weight != 0.0 )
            {
                frequency = XAnimGetAverageRateFrequency(tree, infoIndexa);
                if ( frequency != 0.0 )
                {
                    totalWeight = totalWeight + weight;
                    totalDtime = (float)((float)(frequency * weight) * infoa->state.rate) + totalDtime;
                }
            }
        }
        if ( totalWeight == 0.0 )
            return 0.0f;
        else
            return (float)(totalDtime / totalWeight);
    }
}

void __cdecl XAnimUpdateTimeAndNotetrackLeaf(
                const DObj *obj,
                const XAnimParts *parts,
                unsigned int infoIndex,
                float dtime,
                char notifyFlags)
{
    const char *v5; // eax
    const char *v6; // eax
    bool v7; // [esp+10h] [ebp-28h]
    bool v8; // [esp+1Ch] [ebp-1Ch]
    float v9; // [esp+24h] [ebp-14h]
    XAnimState *state; // [esp+28h] [ebp-10h]
    XAnimInfo *info; // [esp+2Ch] [ebp-Ch]
    float time; // [esp+30h] [ebp-8h]
    __int16 cycleCount; // [esp+34h] [ebp-4h]
    float dtimea; // [esp+4Ch] [ebp+14h]

    if ( !parts && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1852, 0, "%s", "parts") )
        __debugbreak();
    info = &g_xAnimInfo[infoIndex];
    state = &info->state;
    if ( info->state.oldTime < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1857, 0, "%s", "state->oldTime >= 0") )
    {
        __debugbreak();
    }
    dtimea = (float)(g_xAnimInfo[infoIndex].state.rate * parts->frequency) * dtime;
    if ( dtimea != 0.0 )
    {
        time = g_xAnimInfo[infoIndex].state.oldTime + dtimea;
        cycleCount = g_xAnimInfo[infoIndex].state.cycleCount;
        if ( time < 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        1879,
                        0,
                        "%s\n\t(time) = %g",
                        "(time >= 0)",
                        time) )
        {
            __debugbreak();
        }
        if ( time >= 1.0 )
        {
            if ( parts->bLoop )
            {
                do
                {
                    time = time - (float)(1.0 - parts->loopEntryTime);
                    ++cycleCount;
                }
                while ( time >= 1.0 );
                if ( time < 0.0
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1896, 0, "%s", "time >= 0") )
                {
                    __debugbreak();
                }
            }
            else
            {
                if ( (float)(g_xAnimInfo[infoIndex].state.oldTime - 0.99999988) < 0.0 )
                    v9 = 0.99999988f;
                else
                    v9 = 1.0f;
                time = v9;
            }
        }
        if ( parts->bLoop )
            v8 = time < 1.0;
        else
            v8 = time <= 1.0;
        if ( !v8 )
        {
            v5 = va("time: %f, parts->bLoop: %d", time, parts->bLoop);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                            1900,
                            0,
                            "%s\n\t%s",
                            "parts->bLoop ? (time < 1.f) : (time <= 1.f)",
                            v5) )
                __debugbreak();
        }
        if ( (float)(g_xAnimInfo[infoIndex].state.currentAnimTime - time) <= (float)(cycleCount
                                                                                                                                                             - g_xAnimInfo[infoIndex].state.cycleCount) )
        {
            if ( (notifyFlags & 1) != 0 )
                XAnimProcessServerNotify(obj, info, time);
            state->currentAnimTime = time;
            g_xAnimInfo[infoIndex].state.cycleCount = cycleCount;
            info->notifyIndex = -1;
            if ( (notifyFlags & 2) != 0 )
                XAnimProcessClientNotify(info, dtimea);
            if ( state->currentAnimTime < 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                            1916,
                            0,
                            "%s\n\t(state->currentAnimTime) = %g",
                            "(state->currentAnimTime >= 0)",
                            state->currentAnimTime) )
            {
                __debugbreak();
            }
            if ( parts->bLoop )
                v7 = state->currentAnimTime < 1.0;
            else
                v7 = state->currentAnimTime <= 1.0;
            if ( !v7 )
            {
                v6 = va("time: %f, parts->bLoop: %d", state->currentAnimTime, parts->bLoop);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                                1917,
                                0,
                                "%s\n\t%s",
                                "parts->bLoop ? (state->currentAnimTime < 1.f) : (state->currentAnimTime <= 1.f)",
                                v6) )
                    __debugbreak();
            }
        }
    }
}

void __cdecl XAnimProcessClientNotify(XAnimInfo *info, float dtime)
{
    float frac; // [esp+4h] [ebp-1Ch]
    float fraca; // [esp+4h] [ebp-1Ch]
    float fracb; // [esp+4h] [ebp-1Ch]
    float fracc; // [esp+4h] [ebp-1Ch]
    float fracd; // [esp+4h] [ebp-1Ch]
    float frace; // [esp+4h] [ebp-1Ch]
    float fracf; // [esp+4h] [ebp-1Ch]
    float fracg; // [esp+4h] [ebp-1Ch]
    float frach; // [esp+4h] [ebp-1Ch]
    float fraci; // [esp+4h] [ebp-1Ch]
    float fracj; // [esp+4h] [ebp-1Ch]
    float frack; // [esp+4h] [ebp-1Ch]
    float fracl; // [esp+4h] [ebp-1Ch]
    unsigned int notifyName; // [esp+8h] [ebp-18h]
    unsigned int v16; // [esp+8h] [ebp-18h]
    unsigned int v17; // [esp+8h] [ebp-18h]
    unsigned int v18; // [esp+8h] [ebp-18h]
    unsigned int v19; // [esp+8h] [ebp-18h]
    unsigned int v20; // [esp+8h] [ebp-18h]
    const XAnimState *state; // [esp+Ch] [ebp-14h]
    XAnimNotifyInfo *notifyInfo; // [esp+10h] [ebp-10h]
    XAnimNotifyInfo *notifyInfoa; // [esp+10h] [ebp-10h]
    unsigned __int16 notifyType; // [esp+14h] [ebp-Ch]
    unsigned __int16 notifyIndex; // [esp+18h] [ebp-8h]
    unsigned __int16 notifyIndexa; // [esp+18h] [ebp-8h]
    XAnimParts *parts; // [esp+1Ch] [ebp-4h]

    state = &info->state;
    if ( dtime == 0.0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1615, 0, "%s", "dtime") )
        __debugbreak();
    notifyType = info->notifyType;
    if ( info->state.goalWeight == 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1622, 0, "%s", "state->goalWeight") )
    {
        __debugbreak();
    }
    if ( info->state.oldTime == 1.0 )
    {
        if ( info->notifyName )
        {
LABEL_9:
            notifyName = info->notifyName;
            frac = XAnimGetNotifyFracLeaf(state, state, 1.0, dtime);
            XAnimAddClientNotifyNamed(g_endNotetrackName, frac, notifyName);
            return;
        }
        fraca = XAnimGetNotifyFracLeaf(state, state, 1.0, dtime);
        XAnimAddClientNotify(g_endNotetrackName, fraca, notifyType);
    }
    else if ( info->animToModel )
    {
        parts = info->parts;
        if ( !parts && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1647, 0, "%s", "parts") )
            __debugbreak();
        notifyIndex = XAnimGetNextNotifyIndex(parts, info->state.oldTime);
        if ( notifyIndex >= (int)parts->notifyCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        1650,
                        0,
                        "%s",
                        "notifyIndex < parts->notifyCount") )
        {
            __debugbreak();
        }
        notifyInfo = &parts->notify[notifyIndex];
        if ( info->state.oldTime <= info->state.currentAnimTime )
        {
            if ( state->currentAnimTime == 1.0 )
            {
                if ( parts->bLoop
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1710, 0, "%s", "!parts->bLoop") )
                {
                    __debugbreak();
                }
                if ( info->state.oldTime <= notifyInfo->time )
                {
                    do
                    {
                        if ( notifyInfo->time < info->state.oldTime
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                                        1717,
                                        0,
                                        "%s",
                                        "state->oldTime <= notifyInfo->time") )
                        {
                            __debugbreak();
                        }
                        if ( info->notifyName )
                        {
                            v19 = info->notifyName;
                            fraci = XAnimGetNotifyFracLeaf(state, state, notifyInfo->time, dtime);
                            XAnimAddClientNotifyNamed(notifyInfo->name, fraci, v19);
                        }
                        else
                        {
                            fracj = XAnimGetNotifyFracLeaf(state, state, notifyInfo->time, dtime);
                            XAnimAddClientNotify(notifyInfo->name, fracj, notifyType);
                        }
                        ++notifyInfo;
                        ++notifyIndex;
                    }
                    while ( notifyIndex < (int)parts->notifyCount );
                }
            }
            else if ( notifyInfo->time < state->currentAnimTime && info->state.oldTime <= notifyInfo->time )
            {
                do
                {
                    if ( notifyInfo->time < info->state.oldTime
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                                    1737,
                                    0,
                                    "%s",
                                    "state->oldTime <= notifyInfo->time") )
                    {
                        __debugbreak();
                    }
                    if ( info->notifyName )
                    {
                        v20 = info->notifyName;
                        frack = XAnimGetNotifyFracLeaf(state, state, notifyInfo->time, dtime);
                        XAnimAddClientNotifyNamed(notifyInfo->name, frack, v20);
                    }
                    else
                    {
                        fracl = XAnimGetNotifyFracLeaf(state, state, notifyInfo->time, dtime);
                        XAnimAddClientNotify(notifyInfo->name, fracl, notifyType);
                    }
                    ++notifyInfo;
                    ++notifyIndex;
                }
                while ( notifyIndex < (int)parts->notifyCount && state->currentAnimTime > notifyInfo->time );
            }
        }
        else if ( state->currentAnimTime <= notifyInfo->time )
        {
            if ( info->state.oldTime <= notifyInfo->time )
            {
                do
                {
                    if ( notifyInfo->time < info->state.oldTime
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                                    1680,
                                    0,
                                    "%s",
                                    "state->oldTime <= notifyInfo->time") )
                    {
                        __debugbreak();
                    }
                    if ( info->notifyName )
                    {
                        v17 = info->notifyName;
                        frace = XAnimGetNotifyFracLeaf(state, state, notifyInfo->time, dtime);
                        XAnimAddClientNotifyNamed(notifyInfo->name, frace, v17);
                    }
                    else
                    {
                        fracf = XAnimGetNotifyFracLeaf(state, state, notifyInfo->time, dtime);
                        XAnimAddClientNotify(notifyInfo->name, fracf, notifyType);
                    }
                    ++notifyInfo;
                    ++notifyIndex;
                }
                while ( notifyIndex < (int)parts->notifyCount );
                notifyIndexa = 0;
                for ( notifyInfoa = parts->notify; state->currentAnimTime > notifyInfoa->time; ++notifyInfoa )
                {
                    if ( info->notifyName )
                    {
                        v18 = info->notifyName;
                        fracg = XAnimGetNotifyFracLeaf(state, state, notifyInfoa->time, dtime);
                        XAnimAddClientNotifyNamed(notifyInfoa->name, fracg, v18);
                    }
                    else
                    {
                        frach = XAnimGetNotifyFracLeaf(state, state, notifyInfoa->time, dtime);
                        XAnimAddClientNotify(notifyInfoa->name, frach, notifyType);
                    }
                    if ( ++notifyIndexa >= (int)parts->notifyCount
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                                    1701,
                                    0,
                                    "%s",
                                    "notifyIndex < parts->notifyCount") )
                    {
                        __debugbreak();
                    }
                }
            }
        }
        else
        {
            do
            {
                if ( info->notifyName )
                {
                    v16 = info->notifyName;
                    fracc = XAnimGetNotifyFracLeaf(state, state, notifyInfo->time, dtime);
                    XAnimAddClientNotifyNamed(notifyInfo->name, fracc, v16);
                }
                else
                {
                    fracd = XAnimGetNotifyFracLeaf(state, state, notifyInfo->time, dtime);
                    XAnimAddClientNotify(notifyInfo->name, fracd, notifyType);
                }
                ++notifyInfo;
                ++notifyIndex;
            }
            while ( notifyIndex < (int)parts->notifyCount && state->currentAnimTime > notifyInfo->time );
        }
    }
    else if ( info->state.oldTime > info->state.currentAnimTime || state->currentAnimTime == 1.0 )
    {
        if ( info->notifyName )
            goto LABEL_9;
        fracb = XAnimGetNotifyFracLeaf(state, state, 1.0, dtime);
        XAnimAddClientNotify(g_endNotetrackName, fracb, notifyType);
    }
}

signed int __cdecl XAnimGetNextNotifyIndex(const XAnimParts *parts, float time)
{
    XAnimNotifyInfo *notifyInfo; // [esp+8h] [ebp-14h]
    XAnimNotifyInfo *bestNotifyInfo; // [esp+Ch] [ebp-10h]
    float bestTime; // [esp+10h] [ebp-Ch]
    float testTime; // [esp+14h] [ebp-8h]
    int notifyInfoIndex; // [esp+18h] [ebp-4h]

    if ( !parts && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1424, 0, "%s", "parts") )
        __debugbreak();
    if ( time < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    1426,
                    0,
                    "%s\n\t(time) = %g",
                    "(time >= 0)",
                    time) )
    {
        __debugbreak();
    }
    if ( time >= 1.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    1427,
                    0,
                    "%s\n\t(time) = %g",
                    "(time < 1.f)",
                    time) )
    {
        __debugbreak();
    }
    if ( time == 0.0 )
        return 0;
    bestNotifyInfo = 0;
    bestTime = 2.0f;
    notifyInfo = parts->notify;
    if ( !notifyInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1451, 0, "%s", "notifyInfo") )
    {
        __debugbreak();
    }
    for ( notifyInfoIndex = 0; notifyInfoIndex < parts->notifyCount; ++notifyInfoIndex )
    {
        testTime = notifyInfo->time;
        if ( testTime < 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1455, 0, "%s", "testTime >= 0") )
        {
            __debugbreak();
        }
        if ( time < testTime && bestTime > testTime )
        {
            bestTime = testTime;
            bestNotifyInfo = notifyInfo;
        }
        ++notifyInfo;
    }
    if ( !bestNotifyInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1465, 0, "%s", "bestNotifyInfo") )
    {
        __debugbreak();
    }
    if ( bestNotifyInfo != parts->notify
        && bestNotifyInfo->time <= bestNotifyInfo[-1].time
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    1466,
                    0,
                    "%s",
                    "bestNotifyInfo == parts->notify || bestNotifyInfo->time > (bestNotifyInfo - 1)->time") )
    {
        __debugbreak();
    }
    return bestNotifyInfo - parts->notify;
}

double __cdecl XAnimGetNotifyFracLeaf(const XAnimState *state, const XAnimState *nextState, float time, float dtime)
{
    if ( dtime == 0.0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1476, 0, "%s", "dtime") )
        __debugbreak();
    if ( nextState->oldTime == 1.0 )
        return 1.0;
    if ( nextState->oldTime <= nextState->currentAnimTime )
    {
        if ( (nextState->currentAnimTime > time || nextState->currentAnimTime == 1.0) && time >= nextState->oldTime )
            return ((double)(nextState->oldCycleCount - state->oldCycleCount) + time - state->oldTime) / dtime;
        else
            return 1.0;
    }
    else if ( nextState->currentAnimTime <= time )
    {
        if ( time < nextState->oldTime )
            return 1.0;
        else
            return ((double)(nextState->oldCycleCount - state->oldCycleCount) + time - state->oldTime) / dtime;
    }
    else
    {
        return ((double)(nextState->oldCycleCount + 1 - state->oldCycleCount) + time - state->oldTime) / dtime;
    }
}

void __cdecl XAnimUpdateTimeAndNotetrackSyncSubTree(
                const DObj *obj,
                unsigned int infoIndex,
                float dtime,
                int notifyFlags)
{
    const char *v4; // eax
    const char *v5; // eax
    bool v6; // [esp+1Ch] [ebp-2Ch]
    bool v7; // [esp+28h] [ebp-20h]
    float v8; // [esp+30h] [ebp-18h]
    XAnimState *state; // [esp+34h] [ebp-14h]
    unsigned int nextInfoIndex; // [esp+38h] [ebp-10h]
    XAnimInfo *info; // [esp+3Ch] [ebp-Ch]
    float time; // [esp+40h] [ebp-8h]
    __int16 cycleCount; // [esp+44h] [ebp-4h]
    unsigned int infoIndexa; // [esp+54h] [ebp+Ch]

    info = &g_xAnimInfo[infoIndex];
    state = &info->state;
    if ( (info->animParent.flags & 4) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    1931,
                    0,
                    "%s",
                    "info->animParent.flags & XANIM_SYNC_ROOT") )
    {
        __debugbreak();
    }
    if ( (g_xAnimInfo[infoIndex].state.oldTime < 0.0 || g_xAnimInfo[infoIndex].state.oldTime > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    1933,
                    0,
                    "state->oldTime not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
                    g_xAnimInfo[infoIndex].state.oldTime,
                    0.0,
                    1.0) )
    {
        __debugbreak();
    }
    time = g_xAnimInfo[infoIndex].state.oldTime + dtime;
    cycleCount = g_xAnimInfo[infoIndex].state.oldCycleCount;
    if ( time < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    1944,
                    0,
                    "%s\n\t(time) = %g",
                    "(time >= 0)",
                    time) )
    {
        __debugbreak();
    }
    if ( time >= 1.0 )
    {
        if ( (info->animParent.flags & 2) != 0 )
        {
            if ( (info->animParent.flags & 1) != 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                            1950,
                            0,
                            "%s",
                            "!(info->animParent.flags & XANIM_LOOP_SYNC_TIME)") )
            {
                __debugbreak();
            }
            if ( (float)(g_xAnimInfo[infoIndex].state.oldTime - 0.99999988) < 0.0 )
                v8 = 0.99999988f;
            else
                v8 = 1.0f;
            time = v8;
        }
        else
        {
            if ( (info->animParent.flags & 1) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                            1955,
                            0,
                            "%s",
                            "info->animParent.flags & XANIM_LOOP_SYNC_TIME") )
            {
                __debugbreak();
            }
            do
            {
                time = time - 1.0;
                ++cycleCount;
            }
            while ( time >= 1.0 );
            if ( time < 0.0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1963, 0, "%s", "time >= 0") )
            {
                __debugbreak();
            }
        }
    }
    if ( (info->animParent.flags & 1) != 0 )
        v7 = time < 1.0;
    else
        v7 = time <= 1.0;
    if ( !v7 )
    {
        v4 = va("time: %f, info->animParent.flags & XANIM_LOOP_SYNC_TIME: %d", time, info->animParent.flags & 1);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        1967,
                        0,
                        "%s\n\t%s",
                        "info->animParent.flags & XANIM_LOOP_SYNC_TIME ? (time < 1.f) : (time <= 1.f)",
                        v4) )
            __debugbreak();
    }
    if ( (float)(g_xAnimInfo[infoIndex].state.currentAnimTime - time) <= (float)(cycleCount
                                                                                                                                                         - g_xAnimInfo[infoIndex].state.cycleCount) )
    {
        if ( (notifyFlags & 1) != 0 )
            XAnimProcessServerNotify(obj, info, time);
        state->currentAnimTime = time;
        g_xAnimInfo[infoIndex].state.cycleCount = cycleCount;
        info->notifyIndex = -1;
        if ( (notifyFlags & 2) != 0 )
            XAnimProcessClientNotify(info, dtime);
        if ( state->currentAnimTime < 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        1983,
                        0,
                        "%s\n\t(state->currentAnimTime) = %g",
                        "(state->currentAnimTime >= 0)",
                        state->currentAnimTime) )
        {
            __debugbreak();
        }
        if ( (info->animParent.flags & 1) != 0 )
            v6 = state->currentAnimTime < 1.0;
        else
            v6 = state->currentAnimTime <= 1.0;
        if ( !v6 )
        {
            v5 = va(
                         "time: %f, info->animParent.flags & XANIM_LOOP_SYNC_TIME: %d",
                         state->currentAnimTime,
                         info->animParent.flags & 1);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                            1984,
                            0,
                            "%s\n\t%s",
                            "info->animParent.flags & XANIM_LOOP_SYNC_TIME ? (state->currentAnimTime < 1.f) : (state->currentAnimTime <= 1.f)",
                            v5) )
                __debugbreak();
        }
        for ( infoIndexa = info->children; infoIndexa; infoIndexa = nextInfoIndex )
        {
            nextInfoIndex = g_xAnimInfo[infoIndexa].next;
            XAnimUpdateInfoSync(obj, infoIndexa, notifyFlags, state, dtime);
        }
    }
}

void __cdecl XAnimUpdateInfoSync(
                const DObj *obj,
                unsigned int infoIndex,
                int notifyFlags,
                XAnimState *syncState,
                float dtime)
{
    XAnimState *state; // [esp+4h] [ebp-Ch]
    unsigned int nextInfoIndex; // [esp+8h] [ebp-8h]
    XAnimInfo *info; // [esp+Ch] [ebp-4h]
    unsigned int infoIndexa; // [esp+1Ch] [ebp+Ch]

    if ( dtime <= 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1760, 0, "%s", "dtime > 0") )
    {
        __debugbreak();
    }
    if ( (!infoIndex || infoIndex >= 0x1000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    1761,
                    0,
                    "%s\n\t(infoIndex) = %i",
                    "(infoIndex && (infoIndex < 4096))",
                    infoIndex) )
    {
        __debugbreak();
    }
    info = &g_xAnimInfo[infoIndex];
    state = &info->state;
    if ( info->state.weight == 0.0 )
    {
        XAnimCheckFreeInfo(obj->localTree, infoIndex, 0);
    }
    else
    {
        if ( g_xAnimInfo[infoIndex].state.goalWeight == 0.0 )
            notifyFlags = 0;
        if ( g_xAnimInfo[infoIndex].state.oldTime != syncState->oldTime
            || g_xAnimInfo[infoIndex].state.oldCycleCount != syncState->oldCycleCount )
        {
            state->currentAnimTime = syncState->oldTime;
            g_xAnimInfo[infoIndex].state.cycleCount = syncState->oldCycleCount;
            g_xAnimInfo[infoIndex].state.oldTime = syncState->oldTime;
            g_xAnimInfo[infoIndex].state.oldCycleCount = syncState->oldCycleCount;
            info->notifyIndex = -1;
        }
        if ( (notifyFlags & 1) != 0 )
            XAnimProcessServerNotify(obj, info, syncState->currentAnimTime);
        state->currentAnimTime = syncState->currentAnimTime;
        g_xAnimInfo[infoIndex].state.cycleCount = syncState->cycleCount;
        info->notifyIndex = -1;
        if ( (notifyFlags & 2) != 0 )
            XAnimProcessClientNotify(info, dtime);
        for ( infoIndexa = info->children; infoIndexa; infoIndexa = nextInfoIndex )
        {
            nextInfoIndex = g_xAnimInfo[infoIndexa].next;
            XAnimUpdateInfoSync(obj, infoIndexa, notifyFlags, syncState, dtime);
        }
    }
}

void __cdecl XAnimProcessServerNotify(const DObj *obj, XAnimInfo *info, float time)
{
    int i; // [esp+8h] [ebp-1Ch]
    XAnimTree_s *tree; // [esp+10h] [ebp-14h]
    float fStartTime; // [esp+14h] [ebp-10h]
    XAnimParts *parts; // [esp+1Ch] [ebp-8h]
    int iCurrNotify; // [esp+20h] [ebp-4h]

    if ( info->state.goalWeight == 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2419, 0, "%s", "info->state.goalWeight") )
    {
        __debugbreak();
    }
    if ( time > 1.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2420, 0, "%s", "time <= 1.f") )
    {
        __debugbreak();
    }
    if ( obj->entnum && info->notifyName && info->state.currentAnimTime != time )
    {
        if ( info->state.currentAnimTime == 1.0 )
        {
            NotifyServerNotetrack(obj, info->notifyName, g_endNotetrackName, 0.0);
        }
        else
        {
            tree = obj->localTree;
            if ( !obj->localTree
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2437, 0, "%s", "tree") )
            {
                __debugbreak();
            }
            parts = XAnimGetPartsForNotify(tree, info);
            if ( parts )
            {
                fStartTime = info->state.currentAnimTime;
                if ( fStartTime > time )
                {
                    for ( i = (unsigned __int16)XAnimGetNextNotifyIndex(parts, fStartTime); i < parts->notifyCount; ++i )
                        NotifyServerNotetrack(obj, info->notifyName, parts->notify[i].name, 0.0);
                    fStartTime = 0.0f;
                }
                for ( iCurrNotify = (unsigned __int16)XAnimGetNextNotifyIndex(parts, fStartTime);
                            iCurrNotify < parts->notifyCount && time >= parts->notify[iCurrNotify].time;
                            ++iCurrNotify )
                {
                    NotifyServerNotetrack(obj, info->notifyName, parts->notify[iCurrNotify].name, 0.0);
                }
            }
            else if ( info->state.currentAnimTime > time || time == 1.0 )
            {
                NotifyServerNotetrack(obj, info->notifyName, g_endNotetrackName, 0.0);
            }
        }
    }
}

XAnimParts *__cdecl XAnimGetPartsForNotify(const XAnimTree_s *tree, XAnimInfo *info)
{
    XAnimEntry *anim; // [esp+0h] [ebp-8h]
    XAnimParts *parts; // [esp+4h] [ebp-4h]
    XAnimParts *partsa; // [esp+4h] [ebp-4h]

    if ( info->animToModel )
    {
        parts = info->parts;
        if ( !parts && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1510, 0, "%s", "parts") )
            __debugbreak();
        return parts;
    }
    else if ( info->notifyChild )
    {
        anim = &tree->anims->entries[info->notifyChild];
        if ( !IsLeafNode(anim)
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1518, 0, "%s", "IsLeafNode( anim )") )
        {
            __debugbreak();
        }
        partsa = anim->parts;
        if ( !partsa && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 1521, 0, "%s", "parts") )
            __debugbreak();
        return partsa;
    }
    else
    {
        return 0;
    }
}

void __cdecl NotifyServerNotetrack(
                const DObj *obj,
                unsigned int notifyName,
                unsigned int notetrackName,
                float fPercentage)
{
    XAnimServerNotify *notify; // [esp+0h] [ebp-4h]

    if ( g_currList )
    {
        if ( g_currList->iCurrentElement < g_currList->iNumElements )
        {
            notify = &g_currList->pList[g_currList->iCurrentElement];
            notify->entNum = obj->entnum - 1;
            notify->fPercentTime = fPercentage;
            notify->notetrackName = 0;
            notify->notifyName = 0;
            Scr_SetString(&notify->notetrackName, notetrackName, SCRIPTINSTANCE_SERVER);
            Scr_SetString(&notify->notifyName, notifyName, SCRIPTINSTANCE_SERVER);
            ++g_currList->iCurrentElement;
        }
    }
    else
    {
        Scr_AddConstString(notetrackName, SCRIPTINSTANCE_SERVER);
        Scr_NotifyNum(obj->entnum - 1, 0, notifyName, 1u);
    }
}

void __cdecl DObjClearServerNotifies()
{
    int iLoop; // [esp+0h] [ebp-4h]

    if ( g_currList )
    {
        for ( iLoop = 0; iLoop < g_currList->iCurrentElement; ++iLoop )
        {
            Scr_SetString(&g_currList->pList[iLoop].notetrackName, 0, SCRIPTINSTANCE_SERVER);
            Scr_SetString(&g_currList->pList[iLoop].notifyName, 0, SCRIPTINSTANCE_SERVER);
        }
        g_currList->iCurrentElement = 0;
    }
    g_currList = 0;
}

void __cdecl DObjSetServerNotifies(XAnimServerNotifyList *pNotifyList)
{
    g_currList = pNotifyList;
}

void __cdecl DObjClearClientNotifies()
{
    g_clientList = 0;
}

void __cdecl DObjSetClientNotifies(XAnimClientNotifyList *pNotifyList)
{
    if ( g_clientList
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3344, 0, "%s", "g_clientList == NULL") )
    {
        __debugbreak();
    }
    g_clientList = pNotifyList;
}

const XAnimClientNotifyList *__cdecl DObjGetClientNotifies()
{
    return g_clientList;
}

void __cdecl DObjUpdateServerInfo(DObj *obj, float dtime, int notifyFlags)
{
    XAnimTree_s *tree; // [esp+8h] [ebp-4h]

    if ( dtime < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3366, 0, "%s", "dtime >= 0") )
    {
        __debugbreak();
    }
    tree = obj->localTree;
    if ( obj->localTree )
    {
        if ( tree->children )
            XAnimUpdateTimeAndNotetrack(obj, tree->children, dtime, notifyFlags != 0);
    }
}

void __cdecl DObjDisplayAnimToBuffer(const DObj *obj, const char *header, char *buffer, int bufferSize)
{
    XAnimTree_s *tree; // [esp+0h] [ebp-8h]
    int bufferPos; // [esp+4h] [ebp-4h] BYREF

    if ( !header && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3492, 0, "%s", "header") )
        __debugbreak();
    if ( !buffer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3493, 0, "%s", "buffer") )
        __debugbreak();
    tree = obj->localTree;
    if ( obj->localTree )
    {
        if ( tree->children )
        {
            bufferPos = 0;
            Com_sprintfPos(buffer, bufferSize, &bufferPos, "%s", header);
            XAnimDisplay(tree, tree->children, 0, buffer, bufferSize, &bufferPos);
            Com_sprintfPos(buffer, bufferSize, &bufferPos, "\n");
        }
        else
        {
            Com_sprintf(buffer, bufferSize, "%sNO TREE CHILDREN\n\n", header);
        }
    }
    else
    {
        Com_sprintf(buffer, bufferSize, "%sNO TREE\n\n", header);
    }
}

void __cdecl XAnimDisplay(
                const XAnimTree_s *tree,
                unsigned int infoIndex,
                int depth,
                char *buffer,
                int bufferSize,
                int *bufferPos)
{
    char *v6; // eax
    char *v7; // eax
    char *v8; // eax
    float v9; // [esp+34h] [ebp-30h]
    XAnimInfo *info; // [esp+3Ch] [ebp-28h]
    float delta; // [esp+40h] [ebp-24h]
    float duration; // [esp+44h] [ebp-20h]
    char *debugName; // [esp+48h] [ebp-1Ch]
    scriptInstance_t inst; // [esp+4Ch] [ebp-18h]
    unsigned int animIndex; // [esp+50h] [ebp-14h]
    int i; // [esp+54h] [ebp-10h]
    const XAnimParts *parts; // [esp+58h] [ebp-Ch]
    const char *color; // [esp+5Ch] [ebp-8h]
    unsigned int infoIndexa; // [esp+70h] [ebp+Ch]

    if ( !tree && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3399, 0, "%s", "tree") )
        __debugbreak();
    if ( !tree->anims
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3400, 0, "%s", "tree->anims") )
    {
        __debugbreak();
    }
    if ( !buffer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3401, 0, "%s", "buffer") )
        __debugbreak();
    if ( !bufferPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3402, 0, "%s", "bufferPos") )
    {
        __debugbreak();
    }
    if ( bufferSize <= *bufferPos
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    3403,
                    0,
                    "%s",
                    "bufferSize > *bufferPos") )
    {
        __debugbreak();
    }
    if ( (!infoIndex || infoIndex >= 0x1000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    3405,
                    0,
                    "%s\n\t(infoIndex) = %i",
                    "(infoIndex && (infoIndex < 4096))",
                    infoIndex) )
    {
        __debugbreak();
    }
    info = &g_xAnimInfo[infoIndex];
    if ( info->state.weight < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3409, 0, "%s", "state->weight >= 0") )
    {
        __debugbreak();
    }
    for ( i = 0; i < depth; ++i )
        Com_sprintfPos(buffer, bufferSize, bufferPos, " ");
    animIndex = info->animIndex;
    if ( animIndex >= tree->anims->size
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    3418,
                    0,
                    "%s\n\t(animIndex) = %i",
                    "(animIndex < tree->anims->size)",
                    animIndex) )
    {
        __debugbreak();
    }
    debugName = XAnimGetAnimDebugName(tree->anims, animIndex);
    if ( g_xAnimInfo[infoIndex].state.weight == 0.0 )
    {
        color = "^0";
    }
    else if ( g_xAnimInfo[infoIndex].state.goalWeight <= g_xAnimInfo[infoIndex].state.weight )
    {
        if ( g_xAnimInfo[infoIndex].state.weight <= g_xAnimInfo[infoIndex].state.goalWeight )
            color = "";
        else
            color = "^1";
    }
    else
    {
        color = "^4";
    }
    inst = (scriptInstance_t)(bgs != &level_bgs);
    if ( info->animToModel )
    {
        duration = XAnimGetLength(tree->anims, animIndex);
        parts = info->parts;
        if ( !parts && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3443, 0, "%s", "parts") )
            __debugbreak();
        delta = g_xAnimInfo[infoIndex].state.currentAnimTime - g_xAnimInfo[infoIndex].state.oldTime;
        if ( delta < 0.0 )
            delta = delta + 1.0;
        if ( parts->frequency == 0.0 )
            v9 = 0.0f;
        else
            v9 = delta / parts->frequency;
        if ( info->notifyName )
        {
            v6 = SL_ConvertToString(info->notifyName, inst);
            Com_sprintfPos(
                buffer,
                bufferSize,
                bufferPos,
                "%s%s: (w) %.2f -> %.2f, (r) %.2f, (rt) %.2f / %.2f, (realtd) %.2f, '%s'\n",
                color,
                debugName,
                g_xAnimInfo[infoIndex].state.weight,
                g_xAnimInfo[infoIndex].state.goalWeight,
                g_xAnimInfo[infoIndex].state.rate,
                (float)(g_xAnimInfo[infoIndex].state.currentAnimTime * duration),
                duration,
                v9,
                v6);
        }
        else
        {
            Com_sprintfPos(
                buffer,
                bufferSize,
                bufferPos,
                "%s%s: (w) %.2f -> %.2f, (r) %.2f, (rt) %.2f / %.2f, (realtd) %.2f\n",
                color,
                debugName,
                g_xAnimInfo[infoIndex].state.weight,
                g_xAnimInfo[infoIndex].state.goalWeight,
                g_xAnimInfo[infoIndex].state.rate,
                (float)(g_xAnimInfo[infoIndex].state.currentAnimTime * duration),
                duration,
                v9);
        }
    }
    else
    {
        if ( info->notifyName )
        {
            if ( XAnimHasTime(tree->anims, animIndex) )
            {
                v7 = SL_ConvertToString(info->notifyName, inst);
                Com_sprintfPos(
                    buffer,
                    bufferSize,
                    bufferPos,
                    "%s%s: (w) %.2f -> %.2f, (r) %.2f, (nt) %.2f -> %.2f, '%s'\n",
                    color,
                    debugName,
                    info->state.weight,
                    info->state.goalWeight,
                    info->state.rate,
                    info->state.oldTime,
                    info->state.currentAnimTime,
                    v7);
            }
            else
            {
                v8 = SL_ConvertToString(info->notifyName, inst);
                Com_sprintfPos(
                    buffer,
                    bufferSize,
                    bufferPos,
                    "%s%s: (w) %.2f -> %.2f, '%s'\n",
                    color,
                    debugName,
                    g_xAnimInfo[infoIndex].state.weight,
                    g_xAnimInfo[infoIndex].state.goalWeight,
                    v8);
            }
        }
        else if ( XAnimHasTime(tree->anims, animIndex) )
        {
            Com_sprintfPos(
                buffer,
                bufferSize,
                bufferPos,
                "%s%s: (w) %.2f -> %.2f, (r) %.2f, (nt) %.2f -> %.2f\n",
                color,
                debugName,
                info->state.weight,
                info->state.goalWeight,
                info->state.rate,
                info->state.oldTime,
                info->state.currentAnimTime);
        }
        else
        {
            Com_sprintfPos(
                buffer,
                bufferSize,
                bufferPos,
                "%s%s: (w) %.2f -> %.2f\n",
                color,
                debugName,
                g_xAnimInfo[infoIndex].state.weight,
                g_xAnimInfo[infoIndex].state.goalWeight);
        }
        for ( infoIndexa = info->children; infoIndexa; infoIndexa = g_xAnimInfo[infoIndexa].next )
            XAnimDisplay(
                tree,
                infoIndexa,
                (__PAIR64__(depth, infoIndexa) + 0x100000000LL) >> 32,
                buffer,
                bufferSize,
                bufferPos);
    }
}

void __cdecl DObjDisplayAnim(const DObj *obj, const char *header)
{
    char buffer[2052]; // [esp+0h] [ebp-808h] BYREF

    if ( !header && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3520, 0, "%s", "header") )
        __debugbreak();
    DObjDisplayAnimToBuffer(obj, header, buffer, 2048);
    Com_Printf(19, buffer);
}

void __cdecl XAnimCalcDelta(DObj *obj, unsigned int animIndex, float *rot, float *trans, bool bUseGoalWeight)
{
    XAnimSimpleRotPos rotPos; // [esp+Ch] [ebp-24h] BYREF
    XAnimTree_s *tree; // [esp+24h] [ebp-Ch]
    unsigned int infoIndex; // [esp+28h] [ebp-8h]
    XAnimDeltaInfo deltaInfo; // [esp+2Ch] [ebp-4h]

    tree = obj->localTree;
    if ( !tree && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3542, 0, "%s", "tree") )
        __debugbreak();
    if ( !tree->anims
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3543, 0, "%s", "tree->anims") )
    {
        __debugbreak();
    }
    if ( animIndex >= tree->anims->size
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    3544,
                    0,
                    "%s\n\t(animIndex) = %i",
                    "(animIndex < tree->anims->size)",
                    animIndex) )
    {
        __debugbreak();
    }
    infoIndex = XAnimGetInfoIndex(tree, animIndex);
    if ( infoIndex )
    {
        deltaInfo.bClear = 1;
        deltaInfo.bNormQuat = 0;
        deltaInfo.bAbs = 0;
        deltaInfo.bUseGoalWeight = bUseGoalWeight;
        XAnimCalcDeltaTree(obj, infoIndex, 1.0, deltaInfo, &rotPos);
        if ( rotPos.rot[0] == 0.0 || rotPos.rot[1] == 0.0 )
        {
            *rot = 0.0f;
            rot[1] = 1.0f;
        }
        else
        {
            *rot = rotPos.rot[0];
            rot[1] = rotPos.rot[1];
        }
        *trans = rotPos.pos[0];
        trans[1] = rotPos.pos[1];
        trans[2] = rotPos.pos[2];
    }
    else
    {
        *rot = 0.0f;
        rot[1] = 1.0f;
        *trans = 0.0f;
        trans[1] = 0.0f;
        trans[2] = 0.0f;
    }
}

void __cdecl XAnimCalcDeltaTree(
                const DObj *obj,
                unsigned int infoIndex,
                float weightScale,
                XAnimDeltaInfo deltaInfo,
                XAnimSimpleRotPos *rotPos)
{
    float v5; // [esp+14h] [ebp-B0h]
    XAnimSimpleRotPos *p_newRotPos; // [esp+18h] [ebp-ACh]
    float v7; // [esp+1Ch] [ebp-A8h]
    float goalWeight; // [esp+20h] [ebp-A4h]
    float *pos; // [esp+2Ch] [ebp-98h]
    float v10; // [esp+30h] [ebp-94h]
    float v11; // [esp+38h] [ebp-8Ch]
    float v12; // [esp+54h] [ebp-70h]
    float v13; // [esp+5Ch] [ebp-68h]
    float v14; // [esp+78h] [ebp-4Ch]
    unsigned int infoIndex1; // [esp+84h] [ebp-40h]
    float r; // [esp+8Ch] [ebp-38h]
    XAnimInfo *info; // [esp+90h] [ebp-34h]
    XAnimInfo *infoa; // [esp+90h] [ebp-34h]
    XAnimInfo *infob; // [esp+90h] [ebp-34h]
    XAnimInfo *infoc; // [esp+90h] [ebp-34h]
    XAnimSimpleRotPos newRotPos; // [esp+94h] [ebp-30h] BYREF
    XAnimDeltaInfo childDeltaInfo; // [esp+ACh] [ebp-18h]
    unsigned int infoIndex2; // [esp+B0h] [ebp-14h]
    float weight; // [esp+B4h] [ebp-10h]
    float firstWeight; // [esp+B8h] [ebp-Ch]
    XAnimSimpleRotPos *rotPos2; // [esp+BCh] [ebp-8h]
    const XAnimParts *parts; // [esp+C0h] [ebp-4h]

    if ( (!infoIndex || infoIndex >= 0x1000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    2603,
                    0,
                    "%s\n\t(infoIndex) = %i",
                    "(infoIndex && (infoIndex < 4096))",
                    infoIndex) )
    {
        __debugbreak();
    }
    info = &g_xAnimInfo[infoIndex];
    if ( info->animToModel )
    {
        if ( deltaInfo.bClear )
        {
            rotPos->rot[0] = 0.0f;
            rotPos->rot[1] = 0.0f;
            rotPos->posWeight = 0.0f;
            rotPos->pos[0] = 0.0f;
            rotPos->pos[1] = 0.0f;
            rotPos->pos[2] = 0.0f;
        }
        parts = info->parts;
        if ( !parts && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2624, 0, "%s", "parts") )
            __debugbreak();
        if ( parts->bDelta )
        {
            if ( deltaInfo.bAbs )
                XAnimCalcAbsDeltaParts(parts, weightScale, info->state.currentAnimTime, rotPos);
            else
                XAnimCalcRelDeltaParts(parts, weightScale, info->state.oldTime, info->state.currentAnimTime, rotPos, 1);
        }
    }
    else
    {
        for ( infoIndex1 = info->children; ; infoIndex1 = infoa->next )
        {
            if ( !infoIndex1 )
            {
                if ( deltaInfo.bClear )
                {
                    rotPos->rot[0] = 0.0f;
                    rotPos->rot[1] = 0.0f;
                    rotPos->posWeight = 0.0f;
                    rotPos->pos[0] = 0.0f;
                    rotPos->pos[1] = 0.0f;
                    rotPos->pos[2] = 0.0f;
                }
                return;
            }
            if ( infoIndex1 >= 0x1000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                            2640,
                            0,
                            "%s\n\t(infoIndex1) = %i",
                            "(infoIndex1 && (infoIndex1 < 4096))",
                            infoIndex1) )
            {
                __debugbreak();
            }
            infoa = &g_xAnimInfo[infoIndex1];
            if ( deltaInfo.bUseGoalWeight )
                goalWeight = g_xAnimInfo[infoIndex1].state.goalWeight;
            else
                goalWeight = g_xAnimInfo[infoIndex1].state.weight;
            firstWeight = goalWeight;
            if ( goalWeight < 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                            2644,
                            0,
                            "%s\n\t(firstWeight) = %g",
                            "(firstWeight >= 0.0f)",
                            firstWeight) )
            {
                __debugbreak();
            }
            if ( firstWeight != 0.0 )
                break;
        }
        for ( infoIndex2 = infoa->next; ; infoIndex2 = infob->next )
        {
            if ( !infoIndex2 )
            {
                XAnimCalcDeltaTree(obj, infoIndex1, weightScale, deltaInfo, rotPos);
                return;
            }
            if ( infoIndex2 >= 0x1000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                            2650,
                            0,
                            "%s\n\t(infoIndex2) = %i",
                            "(infoIndex2 && (infoIndex2 < 4096))",
                            infoIndex2) )
            {
                __debugbreak();
            }
            infob = &g_xAnimInfo[infoIndex2];
            if ( deltaInfo.bUseGoalWeight )
                v7 = g_xAnimInfo[infoIndex2].state.goalWeight;
            else
                v7 = g_xAnimInfo[infoIndex2].state.weight;
            weight = v7;
            if ( v7 < 0.0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2654, 0, "%s", "weight >= 0") )
            {
                __debugbreak();
            }
            if ( weight != 0.0 )
                break;
        }
        if ( deltaInfo.bClear )
            p_newRotPos = rotPos;
        else
            p_newRotPos = &newRotPos;
        rotPos2 = p_newRotPos;
        *(_WORD *)&childDeltaInfo.bAbs = *(_WORD *)&deltaInfo.bAbs;
        childDeltaInfo.bClear = 1;
        childDeltaInfo.bNormQuat = 1;
        XAnimCalcDeltaTree(obj, infoIndex1, firstWeight, childDeltaInfo, p_newRotPos);
        childDeltaInfo.bClear = 0;
        XAnimCalcDeltaTree(obj, infoIndex2, weight, childDeltaInfo, rotPos2);
        for ( infoIndex2 = infob->next; infoIndex2; infoIndex2 = infoc->next )
        {
            if ( infoIndex2 >= 0x1000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                            2670,
                            0,
                            "%s\n\t(infoIndex2) = %i",
                            "(infoIndex2 && (infoIndex2 < 4096))",
                            infoIndex2) )
            {
                __debugbreak();
            }
            infoc = &g_xAnimInfo[infoIndex2];
            if ( deltaInfo.bUseGoalWeight )
                v5 = g_xAnimInfo[infoIndex2].state.goalWeight;
            else
                v5 = g_xAnimInfo[infoIndex2].state.weight;
            weight = v5;
            if ( v5 < 0.0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2674, 0, "%s", "weight >= 0") )
            {
                __debugbreak();
            }
            if ( weight != 0.0 )
                XAnimCalcDeltaTree(obj, infoIndex2, weight, childDeltaInfo, rotPos2);
        }
        if ( deltaInfo.bNormQuat )
        {
            if ( deltaInfo.bClear )
            {
                r = (float)(rotPos->rot[0] * rotPos->rot[0]) + (float)(rotPos->rot[1] * rotPos->rot[1]);
                if ( r != 0.0 )
                {
                    v13 = I_rsqrt(r) * weightScale;
                    rotPos->rot[0] = v13 * rotPos->rot[0];
                    rotPos->rot[1] = v13 * rotPos->rot[1];
                }
                if ( rotPos->posWeight != 0.0 )
                {
                    v12 = weightScale / rotPos->posWeight;
                    rotPos->pos[0] = v12 * rotPos->pos[0];
                    rotPos->pos[1] = v12 * rotPos->pos[1];
                    rotPos->pos[2] = v12 * rotPos->pos[2];
                    rotPos->posWeight = weightScale;
                }
            }
            else
            {
                r = (float)(rotPos2->rot[0] * rotPos2->rot[0]) + (float)(rotPos2->rot[1] * rotPos2->rot[1]);
                if ( r != 0.0 )
                {
                    v11 = I_rsqrt(r) * weightScale;
                    rotPos->rot[0] = (float)(v11 * rotPos2->rot[0]) + rotPos->rot[0];
                    rotPos->rot[1] = (float)(v11 * rotPos2->rot[1]) + rotPos->rot[1];
                }
                if ( rotPos2->posWeight != 0.0 )
                {
                    pos = rotPos2->pos;
                    v10 = weightScale / rotPos2->posWeight;
                    rotPos->pos[0] = (float)(v10 * rotPos2->pos[0]) + rotPos->pos[0];
                    rotPos->pos[1] = (float)(v10 * pos[1]) + rotPos->pos[1];
                    rotPos->pos[2] = (float)(v10 * pos[2]) + rotPos->pos[2];
                    rotPos->posWeight = rotPos->posWeight + weightScale;
                }
            }
        }
        else
        {
            if ( !deltaInfo.bClear
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 2683, 0, "%s", "deltaInfo.bClear") )
            {
                __debugbreak();
            }
            if ( rotPos->posWeight != 0.0 )
            {
                v14 = 1.0 / rotPos->posWeight;
                rotPos->pos[0] = v14 * rotPos->pos[0];
                rotPos->pos[1] = v14 * rotPos->pos[1];
                rotPos->pos[2] = v14 * rotPos->pos[2];
            }
        }
    }
}

void __cdecl XAnimCalcAbsDelta(DObj *obj, unsigned int animIndex, float *rot, float *trans)
{
    XAnimSimpleRotPos rotPos; // [esp+Ch] [ebp-24h] BYREF
    XAnimTree_s *tree; // [esp+24h] [ebp-Ch]
    unsigned int infoIndex; // [esp+28h] [ebp-8h]
    XAnimDeltaInfo deltaInfo; // [esp+2Ch] [ebp-4h]

    tree = obj->localTree;
    if ( !tree && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3597, 0, "%s", "tree") )
        __debugbreak();
    if ( !tree->anims
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3598, 0, "%s", "tree->anims") )
    {
        __debugbreak();
    }
    if ( animIndex >= tree->anims->size
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    3599,
                    0,
                    "%s\n\t(animIndex) = %i",
                    "(animIndex < tree->anims->size)",
                    animIndex) )
    {
        __debugbreak();
    }
    infoIndex = XAnimGetInfoIndex(tree, animIndex);
    if ( infoIndex )
    {
        deltaInfo.bClear = 1;
        deltaInfo.bNormQuat = 0;
        deltaInfo.bAbs = 1;
        deltaInfo.bUseGoalWeight = 1;
        XAnimCalcDeltaTree(obj, infoIndex, 1.0, deltaInfo, &rotPos);
        if ( rotPos.rot[0] == 0.0 && rotPos.rot[1] == 0.0 )
        {
            *rot = 0.0f;
            rot[1] = 1.0f;
        }
        else
        {
            *rot = rotPos.rot[0];
            rot[1] = rotPos.rot[1];
        }
        *trans = rotPos.pos[0];
        trans[1] = rotPos.pos[1];
        trans[2] = rotPos.pos[2];
    }
    else
    {
        *rot = 0.0f;
        rot[1] = 1.0f;
        *trans = 0.0f;
        trans[1] = 0.0f;
        trans[2] = 0.0f;
    }
}

void __cdecl XAnimGetRelDelta(
                const XAnim_s *anims,
                unsigned int animIndex,
                float *rot,
                float *trans,
                float time1,
                float time2)
{
    XAnimSimpleRotPos rotPos; // [esp+18h] [ebp-20h] BYREF
    const XAnimEntry *anim; // [esp+30h] [ebp-8h]
    const XAnimParts *parts; // [esp+34h] [ebp-4h]

    anim = &anims->entries[animIndex];
    if ( !IsLeafNode(anim) )
        goto LABEL_11;
    parts = anim->parts;
    if ( !parts && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3655, 0, "%s", "parts") )
        __debugbreak();
    if ( parts->bDelta )
    {
        memset(&rotPos, 0, sizeof(rotPos));
        XAnimCalcRelDeltaParts(parts, 1.0, time1, time2, &rotPos, 0);
        if ( rotPos.rot[0] == 0.0 && rotPos.rot[1] == 0.0 )
        {
            *rot = 0.0f;
            rot[1] = 1.0f;
        }
        else
        {
            *rot = rotPos.rot[0];
            rot[1] = rotPos.rot[1];
        }
        *trans = rotPos.pos[0];
        trans[1] = rotPos.pos[1];
        trans[2] = rotPos.pos[2];
    }
    else
    {
LABEL_11:
        *rot = 0.0f;
        rot[1] = 1.0f;
        *trans = 0.0f;
        trans[1] = 0.0f;
        trans[2] = 0.0f;
    }
}

void __cdecl XAnimGetAbsDelta(const XAnim_s *anims, unsigned int animIndex, float *rot, float *trans, float time)
{
    XAnimSimpleRotPos rotPos; // [esp+10h] [ebp-20h] BYREF
    const XAnimEntry *anim; // [esp+28h] [ebp-8h]
    const XAnimParts *parts; // [esp+2Ch] [ebp-4h]

    anim = &anims->entries[animIndex];
    if ( !IsLeafNode(anim) )
        goto LABEL_11;
    parts = anim->parts;
    if ( !parts && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3713, 0, "%s", "parts") )
        __debugbreak();
    if ( parts->bDelta )
    {
        memset(&rotPos, 0, sizeof(rotPos));
        XAnimCalcAbsDeltaParts(parts, 1.0, time, &rotPos);
        if ( rotPos.rot[0] == 0.0 && rotPos.rot[1] == 0.0 )
        {
            *rot = 0.0f;
            rot[1] = 1.0f;
        }
        else
        {
            *rot = rotPos.rot[0];
            rot[1] = rotPos.rot[1];
        }
        *trans = rotPos.pos[0];
        trans[1] = rotPos.pos[1];
        trans[2] = rotPos.pos[2];
    }
    else
    {
LABEL_11:
        *rot = 0.0f;
        rot[1] = 1.0f;
        *trans = 0.0f;
        trans[1] = 0.0f;
        trans[2] = 0.0f;
    }
}

unsigned int __cdecl XAnimAllocInfoWithParent(
                XAnimTree_s *tree,
                unsigned __int16 animToModel,
                unsigned int animIndex,
                unsigned int parentInfoIndex,
                int after)
{
    bool v6; // [esp+3h] [ebp-21h]
    XAnimParts *parts; // [esp+4h] [ebp-20h]
    XAnimInfo *info; // [esp+10h] [ebp-14h]
    unsigned int next; // [esp+14h] [ebp-10h]
    unsigned int nexta; // [esp+14h] [ebp-10h]
    unsigned int infoIndex; // [esp+18h] [ebp-Ch]
    unsigned int prev; // [esp+1Ch] [ebp-8h]
    XAnimEntry *anim; // [esp+20h] [ebp-4h]

    if ( !tree && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3769, 0, "%s", "tree") )
        __debugbreak();
    if ( !tree->anims
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3770, 0, "%s", "tree->anims") )
    {
        __debugbreak();
    }
    Sys_EnterCriticalSection(CRITSECT_XANIM_ALLOC);
    infoIndex = g_xAnimInfo[0].next;
    if ( g_xAnimInfo[0].next )
    {
        if ( g_xAnimInfo[0].next >= 0x1000u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        3801,
                        0,
                        "%s\n\t(infoIndex) = %i",
                        "(infoIndex && (infoIndex < 4096))",
                        g_xAnimInfo[0].next) )
        {
            __debugbreak();
        }
        g_xAnimInfo[0].next = g_xAnimInfo[infoIndex].next;
        next = g_xAnimInfo[0].next;
        if ( g_xAnimInfo[0].next >= 0x1000u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        3804,
                        0,
                        "%s\n\t(next) = %i",
                        "(next < 4096)",
                        g_xAnimInfo[0].next) )
        {
            __debugbreak();
        }
        g_xAnimInfo[next].prev = 0;
        Sys_LeaveCriticalSection(CRITSECT_XANIM_ALLOC);
        info = &g_xAnimInfo[infoIndex];
        if ( animIndex >= tree->anims->size
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        3813,
                        0,
                        "animIndex < tree->anims->size\n\t%i, %i",
                        animIndex,
                        tree->anims->size) )
        {
            __debugbreak();
        }
        prev = 0;
        if ( after )
        {
            for ( nexta = g_xAnimInfo[parentInfoIndex].children;
                        nexta && g_xAnimInfo[nexta].animIndex <= animIndex;
                        nexta = g_xAnimInfo[nexta].next )
            {
                prev = nexta;
            }
        }
        else
        {
            for ( nexta = g_xAnimInfo[parentInfoIndex].children;
                        nexta && g_xAnimInfo[nexta].animIndex < animIndex;
                        nexta = g_xAnimInfo[nexta].next )
            {
                prev = nexta;
            }
        }
        if ( animIndex >= tree->anims->size
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        3856,
                        0,
                        "%s\n\t(animIndex) = %i",
                        "(animIndex < tree->anims->size)",
                        animIndex) )
        {
            __debugbreak();
        }
        info->prev = prev;
        info->next = nexta;
        info->animIndex = animIndex;
        anim = &tree->anims->entries[animIndex];
        info->children = 0;
        info->parent = parentInfoIndex;
        info->animToModel = animToModel;
        info->streamedParts = 0;
        info->parts = anim->parts;
        if ( info->animParent.children != anim->animParent.children
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        3880,
                        0,
                        "%s",
                        "info->animParent.children == anim->animParent.children") )
        {
            __debugbreak();
        }
        if ( info->animParent.flags != anim->animParent.flags
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        3881,
                        0,
                        "%s",
                        "info->animParent.flags == anim->animParent.flags") )
        {
            __debugbreak();
        }
        if ( animIndex >= tree->anims->size
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        3883,
                        0,
                        "%s\n\t(animIndex) = %i",
                        "(animIndex < tree->anims->size)",
                        animIndex) )
        {
            __debugbreak();
        }
        if ( nexta )
            g_xAnimInfo[nexta].prev = infoIndex;
        if ( prev )
        {
            g_xAnimInfo[prev].next = infoIndex;
        }
        else if ( parentInfoIndex )
        {
            g_xAnimInfo[parentInfoIndex].children = infoIndex;
        }
        else
        {
            tree->children = infoIndex;
        }
        if ( IsLeafNode(anim) )
        {
            parts = anim->parts;
            v6 = useFastFile->current.enabled ? DB_IsXAssetDefault(ASSET_TYPE_XANIMPARTS, parts->name) : parts->isDefault;
            if ( v6 && tree->anims->wasLoggedIfMissing && !tree->anims->wasLoggedIfMissing[animIndex] )
            {
                DB_LogMissingAsset(ASSET_TYPE_XANIMPARTS, anim->parts->name);
                tree->anims->wasLoggedIfMissing[animIndex] = 1;
            }
        }
        if ( info->animToModel )
        {
            if ( info->parts->bStreamable )
                Com_PrintWarning(16, "Cannot play streamed anim '%s' because the anim cache is disabled.", info->parts->name);
        }
        return infoIndex;
    }
    else
    {
        Sys_LeaveCriticalSection(CRITSECT_XANIM_ALLOC);
        Com_Error(ERR_DROP, "exceeded maximum number of anim info");
        return 0;
    }
}

unsigned int XAnimAllocInfoIndex(
                DObj *obj,
                unsigned int animIndex,
                int after,
                int cmdIndex)
{
    unsigned __int16 animToModel; // [esp-Ch] [ebp-820h]
    XModelNameMap modelMap[512]; // [esp+4h] [ebp-810h]
    XAnimTree_s *tree; // [esp+804h] [ebp-10h]
    //unsigned int parentAnimIndex; // [esp+808h] [ebp-Ch]
    //const XAnimEntry *anim; // [esp+80Ch] [ebp-8h]
    //const XAnimEntry *retaddr; // [esp+814h] [ebp+0h]
    const XAnimEntry *animEntry; // [esp+400h] [ebp-14h]
    unsigned int parentAnimIndex; // [esp+3FCh] [ebp-18h]
    unsigned int parentInfoIndex; // [esp+3FCh] [ebp-18h]

    //parentAnimIndex = a1;
    //anim = retaddr;
    tree = obj->localTree;

    iassert(tree);
    iassert(tree->anims);

    iassert(animIndex < tree->anims->size);
    
    if ( animIndex )
    {
        //modelMap[511] = (XModelNameMap)(*(unsigned int *)tree + 16 * animIndex + 24);
        animEntry = &tree->anims->entries[animIndex];
        parentAnimIndex = animEntry->parent;
        parentInfoIndex = XAnimEnsureGoalWeightParent(obj, parentAnimIndex, cmdIndex);
        //modelMap[510] = (XModelNameMap)*(unsigned __int16 *)(*(unsigned int *)&modelMap[511] + 8);
        //modelMap[509] = (XModelNameMap)XAnimEnsureGoalWeightParent(obj, *(unsigned int *)&modelMap[510], cmdIndex);

        if (IsLeafNode(animEntry))
        {
            XAnimInitModelMap(obj->localModels, obj->numModels, modelMap);
            animToModel = XAnimGetAnimMap(animEntry->parts, modelMap);
        }
        else
        {
            animToModel = 0;
        }
    }
    else
    {
        parentInfoIndex = 0;
        animToModel = 0;
    }

    return XAnimAllocInfoWithParent(tree, animToModel, animIndex, parentInfoIndex, after);
}

unsigned int __cdecl XAnimEnsureGoalWeightParent(DObj *obj, unsigned int animIndex, int cmdIndex)
{
    XAnimInfo *info; // [esp+0h] [ebp-14h]
    XAnimTree_s *tree; // [esp+4h] [ebp-10h]
    unsigned int parentInfoIndex; // [esp+8h] [ebp-Ch]
    unsigned int infoIndex; // [esp+Ch] [ebp-8h]
    unsigned int infoIndexa; // [esp+Ch] [ebp-8h]

    tree = obj->localTree;
    if ( !obj->localTree
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 3955, 0, "%s", "tree") )
    {
        __debugbreak();
    }
    if ( animIndex )
    {
        parentInfoIndex = XAnimEnsureGoalWeightParent(obj, tree->anims->entries[animIndex].parent, cmdIndex);
        for ( infoIndexa = g_xAnimInfo[parentInfoIndex].children; infoIndexa; infoIndexa = g_xAnimInfo[infoIndexa].next )
        {
            if ( g_xAnimInfo[infoIndexa].animIndex == animIndex )
                return infoIndexa;
        }
        infoIndex = XAnimAllocInfoWithParent(tree, 0, animIndex, parentInfoIndex, 0);
        XAnimInitInfo(&g_xAnimInfo[infoIndex]);
    }
    else
    {
        if ( tree->children )
            return tree->children;
        infoIndex = XAnimAllocInfoWithParent(tree, 0, 0, 0, 0);
        info = &g_xAnimInfo[infoIndex];
        XAnimInitInfo(info);
        info->state.goalWeight = 1.0f;
        info->state.weight = 1.0f;
        info->state.goalTime = 0.0f;
        info->state.rate = 1.0f;
    }
    return infoIndex;
}

void __cdecl XAnimClearGoalWeight(
                XAnimTree_s *tree,
                unsigned int animIndex,
                float blendTime,
                unsigned __int16 cmdIndex)
{
    unsigned int infoIndex; // [esp+Ch] [ebp-4h]

    infoIndex = XAnimGetInfoIndex(tree, animIndex);
    if ( infoIndex )
        XAnimClearGoalWeightInternal(tree, infoIndex, blendTime, 0, cmdIndex);
}

void __cdecl XAnimClearGoalWeightInternal(
    XAnimTree_s *tree,
    unsigned int infoIndex,
    float blendTime,
    int forceBlendTime,
    unsigned __int16 cmdIndex)
{
    float goalTime; // [esp+0h] [ebp-10h]

    if (!tree && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4099, 0, "%s", "tree"))
        __debugbreak();
    if (!tree->anims
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4100, 0, "%s", "tree->anims"))
    {
        __debugbreak();
    }
    if ((!infoIndex || infoIndex >= 0x1000)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
            4101,
            0,
            "%s\n\t(infoIndex) = %i",
            "(infoIndex && (infoIndex < 4096))",
            infoIndex))
    {
        __debugbreak();
    }
    if (g_xAnimInfo[infoIndex].state.goalWeight == 0.0 && !forceBlendTime)
    {
        if ((float)(blendTime - g_xAnimInfo[infoIndex].state.goalTime) < 0.0)
            goalTime = blendTime;
        else
            goalTime = g_xAnimInfo[infoIndex].state.goalTime;
        g_xAnimInfo[infoIndex].state.goalTime = goalTime;
    }
    else
    {
        g_xAnimInfo[infoIndex].state.goalTime = blendTime;
    }
    g_xAnimInfo[infoIndex].state.goalWeight = 0.0f;
    if (bgs && bgs->AnimCmdRefCount)
    {
        bgs->AnimCmdRefCount(cmdIndex, infoIndex, bgs->time + (int)(float)(blendTime * 1000.0));
    }
    if (blendTime == 0.0)
    {
        g_xAnimInfo[infoIndex].state.oldTime = 0.0f;
        g_xAnimInfo[infoIndex].state.currentAnimTime = 0.0f;
        g_xAnimInfo[infoIndex].state.oldCycleCount = 0;
        g_xAnimInfo[infoIndex].state.cycleCount = 0;
        g_xAnimInfo[infoIndex].state.weight = 0.0f;
        g_xAnimInfo[infoIndex].state.instantWeightChange = 1;
    }
    XAnimClearServerNotify(&g_xAnimInfo[infoIndex], (scriptInstance_t)tree->inst);
}

void __cdecl XAnimClearTreeGoalWeightsInternal(
                XAnimTree_s *tree,
                unsigned int infoIndex,
                float blendTime,
                int forceBlendTime,
                int cmdIndex)
{
    bool v5; // [esp+Ch] [ebp-14h]
    XAnimInfo *childInfo; // [esp+10h] [ebp-10h]
    unsigned int animIndex; // [esp+14h] [ebp-Ch]
    unsigned __int16 myFlags; // [esp+1Ch] [ebp-4h]
    unsigned int infoIndexa; // [esp+2Ch] [ebp+Ch]

    XAnimClearGoalWeightInternal(tree, infoIndex, blendTime, forceBlendTime, cmdIndex);
    if ( g_xAnimInfo[infoIndex].animToModel )
        myFlags = g_xAnimInfo[g_xAnimInfo[infoIndex].parent].animParent.flags;
    else
        myFlags = g_xAnimInfo[infoIndex].animParent.flags;
    animIndex = 0;
    for ( infoIndexa = g_xAnimInfo[infoIndex].children; infoIndexa; infoIndexa = g_xAnimInfo[infoIndexa].next )
    {
        childInfo = &g_xAnimInfo[infoIndexa];
        if ( childInfo->animToModel )
            childInfo = &g_xAnimInfo[childInfo->parent];
        if ( (childInfo->animParent.flags & 0x40) == (myFlags & 0x40) )
        {
            v5 = forceBlendTime && animIndex != g_xAnimInfo[infoIndexa].animIndex;
            XAnimClearTreeGoalWeightsInternal(tree, infoIndexa, blendTime, v5, cmdIndex);
        }
        animIndex = g_xAnimInfo[infoIndexa].animIndex;
    }
}

void __cdecl XAnimClearTreeGoalWeights(XAnimTree_s *tree, unsigned int animIndex, float blendTime, int cmdIndex)
{
    unsigned int infoIndex; // [esp+Ch] [ebp-4h]

    infoIndex = XAnimGetInfoIndex(tree, animIndex);
    if ( infoIndex )
        XAnimClearTreeGoalWeightsInternal(tree, infoIndex, blendTime, 1, cmdIndex);
}

void __cdecl XAnimClearTreeGoalWeightsStrict(XAnimTree_s *tree, unsigned int animIndex, float blendTime, int cmdIndex)
{
    int numAnims; // [esp+8h] [ebp-Ch]
    const XAnimEntry *anim; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    if ( tree )
    {
        if ( !tree->anims
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4203, 0, "%s", "tree->anims") )
        {
            __debugbreak();
        }
        if ( animIndex >= tree->anims->size
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        4204,
                        0,
                        "%s\n\t(animIndex) = %i",
                        "(animIndex < tree->anims->size)",
                        animIndex) )
        {
            __debugbreak();
        }
        anim = &tree->anims->entries[animIndex];
        numAnims = tree->anims->entries[animIndex].numAnims;
        for ( i = 0; i < numAnims; ++i )
            XAnimClearTreeGoalWeights(tree, i + anim->animParent.children, blendTime, cmdIndex);
    }
}

int __cdecl XAnimSetCompleteGoalWeightKnob(
                DObj *obj,
                unsigned int animIndex,
                float goalWeight,
                float goalTime,
                float rate,
                unsigned int notifyName,
                unsigned int notifyType,
                int bRestart,
                int cmdIndex)
{
    XAnimTree_s *tree; // [esp+18h] [ebp-Ch]
    unsigned int infoIndex; // [esp+1Ch] [ebp-8h]
    int savedregs; // [esp+24h] [ebp+0h] BYREF

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4314, 0, "%s", "obj") )
        __debugbreak();
    if ( goalWeight < 0.001 )
        goalWeight = 0.0f;
    tree = obj->localTree;
    infoIndex = XAnimGetInfoIndex(obj->localTree, animIndex);
    if ( infoIndex )
    {
        if ( bRestart )
            infoIndex = XAnimRestart(tree, infoIndex, goalTime, cmdIndex);
    }
    else
    {
        infoIndex = XAnimAllocInfoIndex(obj, animIndex, 0, cmdIndex);
        XAnimInitInfo(&g_xAnimInfo[infoIndex]);
    }
    XAnimClearGoalWeightKnobInternal(tree, infoIndex, goalWeight, goalTime, cmdIndex);
    return XAnimSetCompleteGoalWeightNode(tree, infoIndex, goalWeight, goalTime, rate, notifyName, notifyType, cmdIndex);
}

void __cdecl XAnimClearGoalWeightKnobInternal(
                XAnimTree_s *tree,
                unsigned int infoIndex,
                float goalWeight,
                float goalTime,
                unsigned __int16 cmdIndex)
{
    float v5; // [esp+Ch] [ebp-34h]
    float weight; // [esp+10h] [ebp-30h]
    unsigned __int16 children; // [esp+16h] [ebp-2Ah]
    XAnimInfo *childInfo; // [esp+1Ch] [ebp-24h]
    unsigned __int16 myFlags; // [esp+28h] [ebp-18h]
    float largestWeightDiff; // [esp+34h] [ebp-Ch]
    unsigned int childInfoIndex; // [esp+3Ch] [ebp-4h]
    unsigned int childInfoIndexa; // [esp+3Ch] [ebp-4h]

    if ( !tree && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4229, 0, "%s", "tree") )
        __debugbreak();
    if ( !tree->anims
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4230, 0, "%s", "tree->anims") )
    {
        __debugbreak();
    }
    if ( (!infoIndex || infoIndex >= 0x1000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    4231,
                    0,
                    "%s\n\t(infoIndex) = %i",
                    "(infoIndex && (infoIndex < 4096))",
                    infoIndex) )
    {
        __debugbreak();
    }
    largestWeightDiff = 0.0f;
    if ( g_xAnimInfo[infoIndex].parent )
        children = g_xAnimInfo[g_xAnimInfo[infoIndex].parent].children;
    else
        children = tree->children;
    for ( childInfoIndex = children; childInfoIndex; childInfoIndex = g_xAnimInfo[childInfoIndex].next )
    {
        if ( childInfoIndex >= 0x1000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        4240,
                        0,
                        "%s\n\t(childInfoIndex) = %i",
                        "(childInfoIndex && (childInfoIndex < 4096))",
                        childInfoIndex) )
        {
            __debugbreak();
        }
        if ( childInfoIndex == infoIndex )
            weight = fabs(goalWeight - g_xAnimInfo[childInfoIndex].state.weight);
        else
            weight = g_xAnimInfo[childInfoIndex].state.weight;
        if ( (float)(largestWeightDiff - weight) < 0.0 )
            v5 = weight;
        else
            v5 = largestWeightDiff;
        largestWeightDiff = v5;
    }
    if ( g_xAnimInfo[infoIndex].animToModel )
        myFlags = g_xAnimInfo[g_xAnimInfo[infoIndex].parent].animParent.flags;
    else
        myFlags = g_xAnimInfo[infoIndex].animParent.flags;
    for ( childInfoIndexa = children; childInfoIndexa; childInfoIndexa = g_xAnimInfo[childInfoIndexa].next )
    {
        if ( childInfoIndexa != infoIndex )
        {
            childInfo = &g_xAnimInfo[childInfoIndexa];
            if ( childInfo->animToModel )
                childInfo = &g_xAnimInfo[childInfo->parent];
            if ( (childInfo->animParent.flags & 0x40) == (myFlags & 0x40) )
                XAnimClearGoalWeightInternal(tree, childInfoIndexa, largestWeightDiff * goalTime, 0, cmdIndex);
        }
    }
}

int __cdecl XAnimSetCompleteGoalWeightNode(
                XAnimTree_s *tree,
                unsigned int infoIndex,
                float goalWeight,
                float goalTime,
                float rate,
                unsigned int notifyName,
                unsigned int notifyType,
                unsigned __int16 cmdIndex)
{
    unsigned int parentInfoIndex; // [esp+18h] [ebp-8h]
    int error; // [esp+1Ch] [ebp-4h]

    error = XAnimSetGoalWeightNode(tree, infoIndex, goalWeight, goalTime, rate, notifyName, notifyType, cmdIndex);
    parentInfoIndex = infoIndex;
    while ( 1 )
    {
        parentInfoIndex = g_xAnimInfo[parentInfoIndex].parent;
        if ( !parentInfoIndex )
            break;
        if ( parentInfoIndex >= 0x1000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        4297,
                        0,
                        "%s\n\t(parentInfoIndex) = %i",
                        "(parentInfoIndex && (parentInfoIndex < 4096))",
                        parentInfoIndex) )
        {
            __debugbreak();
        }
        if ( g_xAnimInfo[parentInfoIndex].state.goalWeight == 1.0 || g_xAnimInfo[parentInfoIndex].state.goalWeight == 0.0 )
            XAnimSetGoalWeightNode(tree, parentInfoIndex, 1.0, goalTime, 1.0, 0, 0, cmdIndex);
    }
    return error;
}

int __cdecl XAnimSetGoalWeightKnobAll(
                DObj *obj,
                unsigned int animIndex,
                unsigned int rootIndex,
                float goalWeight,
                float goalTime,
                float rate,
                unsigned int notifyName,
                unsigned int notifyType,
                int bRestart,
                int cmdIndex)
{
    XAnimTree_s *tree; // [esp+1Ch] [ebp-Ch]
    int error; // [esp+20h] [ebp-8h]
    unsigned int infoIndex; // [esp+24h] [ebp-4h]

    if ( animIndex == rootIndex
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4349, 0, "%s", "animIndex != rootIndex") )
    {
        __debugbreak();
    }
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4350, 0, "%s", "obj") )
        __debugbreak();
    error = XAnimSetGoalWeightKnob(obj, animIndex, goalWeight, goalTime, rate, notifyName, notifyType, bRestart, cmdIndex);
    tree = obj->localTree;
    infoIndex = XAnimGetInfoIndex(obj->localTree, animIndex);
    if ( !infoIndex
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4359, 0, "%s", "infoIndex") )
    {
        __debugbreak();
    }
    while ( 1 )
    {
        infoIndex = g_xAnimInfo[infoIndex].parent;
        if ( !infoIndex )
            break;
        if ( g_xAnimInfo[infoIndex].animIndex == rootIndex )
            return error;
        if ( bRestart )
            infoIndex = XAnimRestart(tree, infoIndex, goalTime, cmdIndex);
        XAnimClearGoalWeightKnobInternal(tree, infoIndex, 1.0, goalTime, cmdIndex);
        XAnimSetGoalWeightNode(tree, infoIndex, 1.0, goalTime, 1.0, 0, 0, cmdIndex);
    }
    return 1;
}

int __cdecl XAnimSetCompleteGoalWeightKnobAll(
                DObj *obj,
                unsigned int animIndex,
                unsigned int rootIndex,
                float goalWeight,
                float goalTime,
                float rate,
                unsigned int notifyName,
                unsigned int notifyType,
                int bRestart,
                int cmdIndex)
{
    XAnimTree_s *tree; // [esp+1Ch] [ebp-Ch]
    int error; // [esp+20h] [ebp-8h]
    unsigned int infoIndex; // [esp+24h] [ebp-4h]

    if ( animIndex == rootIndex
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4395, 0, "%s", "animIndex != rootIndex") )
    {
        __debugbreak();
    }
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4396, 0, "%s", "obj") )
        __debugbreak();
    error = XAnimSetGoalWeightKnob(obj, animIndex, goalWeight, goalTime, rate, notifyName, notifyType, bRestart, cmdIndex);
    tree = obj->localTree;
    infoIndex = XAnimGetInfoIndex(obj->localTree, animIndex);
    if ( !infoIndex
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4405, 0, "%s", "infoIndex") )
    {
        __debugbreak();
    }
    while ( 1 )
    {
        infoIndex = g_xAnimInfo[infoIndex].parent;
        if ( !infoIndex )
            break;
        if ( g_xAnimInfo[infoIndex].animIndex == rootIndex )
            return error;
        if ( bRestart )
            infoIndex = XAnimRestart(tree, infoIndex, goalTime, cmdIndex);
        XAnimClearGoalWeightKnobInternal(tree, infoIndex, 1.0, goalTime, cmdIndex);
        XAnimSetCompleteGoalWeightNode(tree, infoIndex, 1.0, goalTime, 1.0, 0, 0, cmdIndex);
    }
    return 1;
}

int __cdecl XAnimSetGoalWeightKnob(
                DObj *obj,
                unsigned int animIndex,
                float goalWeight,
                float goalTime,
                float rate,
                unsigned int notifyName,
                unsigned int notifyType,
                int bRestart,
                int cmdIndex)
{
    XAnimTree_s *tree; // [esp+18h] [ebp-Ch]
    unsigned int infoIndex; // [esp+1Ch] [ebp-8h]
    int savedregs; // [esp+24h] [ebp+0h] BYREF

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4442, 0, "%s", "obj") )
        __debugbreak();
    if ( goalWeight < 0.001 )
        goalWeight = 0.0f;
    tree = obj->localTree;
    infoIndex = XAnimGetInfoIndex(obj->localTree, animIndex);
    if ( infoIndex )
    {
        if ( bRestart )
            infoIndex = XAnimRestart(tree, infoIndex, goalTime, cmdIndex);
    }
    else
    {
        infoIndex = XAnimAllocInfoIndex(obj, animIndex, 0, cmdIndex);
        XAnimInitInfo(&g_xAnimInfo[infoIndex]);
    }
    XAnimClearGoalWeightKnobInternal(tree, infoIndex, goalWeight, goalTime, cmdIndex);
    return XAnimSetGoalWeightNode(tree, infoIndex, goalWeight, goalTime, rate, notifyName, notifyType, cmdIndex);
}

void __cdecl XAnimClearTree(XAnimTree_s *tree)
{
    if ( !tree && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4492, 0, "%s", "tree") )
        __debugbreak();
    if ( tree->children )
    {
        XAnimFreeInfo(tree, tree->children);
        if ( tree->children )
        {
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4497, 0, "%s", "!tree->children") )
                __debugbreak();
        }
    }
}

int __cdecl XAnimSetGoalWeightNode(
                XAnimTree_s *tree,
                unsigned int infoIndex,
                float goalWeight,
                float goalTime,
                float rate,
                unsigned int notifyName,
                unsigned int notifyType,
                unsigned __int16 cmdIndex)
{
    float v9; // [esp+0h] [ebp-20h]
    float v10; // [esp+4h] [ebp-1Ch]
    XAnimInfo *info; // [esp+14h] [ebp-Ch]
    unsigned int animIndex; // [esp+18h] [ebp-8h]
    float weightDiff; // [esp+1Ch] [ebp-4h]

    if ( !tree && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4551, 0, "%s", "tree") )
        __debugbreak();
    if ( !tree->anims
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4552, 0, "%s", "tree->anims") )
    {
        __debugbreak();
    }
    if ( goalWeight != 0.0
        && goalWeight < 0.001
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    4553,
                    0,
                    "%s",
                    "!goalWeight || goalWeight >= WEIGHT_EPSILON") )
    {
        __debugbreak();
    }
    if ( goalTime < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4554, 0, "%s", "goalTime >= 0") )
    {
        __debugbreak();
    }
    if ( rate < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4555, 0, "%s", "rate >= 0") )
    {
        __debugbreak();
    }
    if ( (notifyName || notifyType)
        && goalWeight == 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    4556,
                    0,
                    "%s",
                    "(!notifyName && !notifyType) || goalWeight") )
    {
        __debugbreak();
    }
    if ( (!infoIndex || infoIndex >= 0x1000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    4558,
                    0,
                    "%s\n\t(infoIndex) = %i",
                    "(infoIndex && (infoIndex < 4096))",
                    infoIndex) )
    {
        __debugbreak();
    }
    info = &g_xAnimInfo[infoIndex];
    XAnimClearServerNotify(info, (scriptInstance_t)tree->inst);
    animIndex = info->animIndex;
    if ( animIndex >= tree->anims->size
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    4567,
                    0,
                    "%s\n\t(animIndex) = %i",
                    "(animIndex < tree->anims->size)",
                    animIndex) )
    {
        __debugbreak();
    }
    if ( !animIndex )
    {
        goalWeight = 1.0f;
        goalTime = 0.0f;
        rate = 1.0f;
    }
    if ( goalTime == 0.0 )
    {
        info->state.weight = goalWeight;
        info->state.goalTime = 0.0f;
        info->state.instantWeightChange = 1;
    }
    else
    {
        if ( info->state.weight == 0.0 )
            info->state.weight = goalWeight * 0.001;
        weightDiff = info->state.weight - goalWeight;
        if ( weightDiff == 0.0 || (float)((float)(info->state.goalWeight - goalWeight) * weightDiff) > 0.0 )
        {
            info->state.goalTime = goalTime;
        }
        else
        {
            if ( (float)(goalTime - info->state.goalTime) < 0.0 )
                v10 = goalTime;
            else
                v10 = info->state.goalTime;
            info->state.goalTime = v10;
        }
        if ( goalWeight != 0.0 )
        {
            if ( goalWeight < info->state.weight )
                goalTime = (float)((float)(info->state.weight - goalWeight) / info->state.weight) * goalTime;
            else
                goalTime = (float)((float)(goalWeight - info->state.weight) / goalWeight) * goalTime;
            if ( (float)(info->state.goalTime - goalTime) < 0.0 )
                v9 = goalTime;
            else
                v9 = info->state.goalTime;
            info->state.goalTime = v9;
        }
    }
    if (bgs && bgs->AnimCmdRefCount)
    {
        bgs->AnimCmdRefCount(
            cmdIndex,
            infoIndex,
            bgs->time + (int)(float)(goalTime * 1000.0));
    }
    info->state.goalWeight = goalWeight;
    info->state.rate = rate;
    info->notifyName = notifyName;
    if ( notifyName )
        SL_AddRefToString(notifyName, (scriptInstance_t)tree->inst);
    info->notifyType = notifyType;
    if ( info->notifyIndex != -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    4623,
                    0,
                    "%s",
                    "info->notifyIndex == -1") )
    {
        __debugbreak();
    }
    if ( notifyName && !info->animToModel && (info->animParent.flags & 3) != 0 )
    {
        if ( goalWeight == 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4627, 0, "%s", "goalWeight") )
        {
            __debugbreak();
        }
        info->notifyChild = XAnimGetDescendantWithGreatestWeight(tree, infoIndex);
        if ( !info->notifyChild )
            return 2;
    }
    else
    {
        info->notifyChild = 0;
    }
    return 0;
}

unsigned int __cdecl XAnimGetDescendantWithGreatestWeight(const XAnimTree_s *tree, unsigned int infoIndex)
{
    float testWeight; // [esp+0h] [ebp-14h]
    unsigned int result; // [esp+4h] [ebp-10h]
    XAnimInfo *info; // [esp+8h] [ebp-Ch]
    unsigned int test; // [esp+Ch] [ebp-8h]
    float bestWeight; // [esp+10h] [ebp-4h]
    unsigned int infoIndexa; // [esp+20h] [ebp+Ch]

    info = &g_xAnimInfo[infoIndex];
    if ( info->animToModel )
    {
        if ( info->state.goalWeight == 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        4520,
                        0,
                        "%s",
                        "info->state.goalWeight") )
        {
            __debugbreak();
        }
        return info->animIndex;
    }
    else
    {
        bestWeight = 0.0f;
        result = 0;
        for ( infoIndexa = info->children; infoIndexa; infoIndexa = g_xAnimInfo[infoIndexa].next )
        {
            testWeight = g_xAnimInfo[infoIndexa].state.goalWeight;
            if ( bestWeight < testWeight )
            {
                test = XAnimGetDescendantWithGreatestWeight(tree, infoIndexa);
                if ( test )
                {
                    bestWeight = testWeight;
                    result = test;
                }
            }
        }
        return result;
    }
}

void __cdecl    XAnimSetupSyncNodes(XAnim_s *anims)
{
    XAnimSetupSyncNodes_r(anims, 0, 0);
}

int iFoo;
void __cdecl XAnimSetupSyncNodes_r(XAnim_s *anims, unsigned int animIndex, int parFlags)
{
    XAnimParts *Data_LoadObj; // eax
    char *AnimDebugName; // eax
    char *v5; // eax
    char *v6; // eax
    const char *debugName; // [esp-4h] [ebp-30h]
    const char *v8; // [esp-4h] [ebp-30h]
    bool IsXAssetDefault; // [esp+7h] [ebp-25h]
    XAnimParts *parts; // [esp+8h] [ebp-24h]
    int i; // [esp+10h] [ebp-1Ch]
    int count; // [esp+14h] [ebp-18h]
    int flag; // [esp+1Ch] [ebp-10h]
    int childFlags; // [esp+20h] [ebp-Ch]
    int numAnims; // [esp+24h] [ebp-8h]
    XAnimEntry *anim; // [esp+28h] [ebp-4h]

    anim = &anims->entries[animIndex];
    if ( (parFlags & 3) != 0 )
    {
        if ( IsLeafNode(anim) )
        {
            if ( anims->entries[animIndex].parts->bLoop != ((parFlags & 1) != 0) )
            {
                parts = anims->entries[animIndex].parts;
                if ( useFastFile->current.enabled )
                    IsXAssetDefault = DB_IsXAssetDefault(ASSET_TYPE_XANIMPARTS, parts->name);
                else
                    IsXAssetDefault = parts->isDefault;
                if ( IsXAssetDefault )
                {
                    if ( !useFastFile->current.enabled )
                        XAnimPrecache((char *)"void_loop", (void *(__cdecl *)(int))Hunk_AllocXAnimPrecache);
                    if ( useFastFile->current.enabled )
                        Data_LoadObj = (XAnimParts *)((int (__cdecl *)(const char *, unsigned __int8 *(__cdecl *)(unsigned int)))XAnimFindData_FastFile)(
                                                                                     "void_loop",
                                                                                     (unsigned char*(*)(unsigned int))Hunk_AllocXAnimPrecache);
                    else
                        Data_LoadObj = XAnimFindData_LoadObj((char*)"void_loop", (void *(__cdecl *)(int))Hunk_AllocXAnimPrecache);
                    anims->entries[animIndex].parts = Data_LoadObj;
                    if ( !anims->entries[animIndex].parts )
                        Com_Error(ERR_DROP, "Cannot find 'xanim/%s' that you should have", "void_loop");
                }
                else if ( (parFlags & 1) != 0 )
                {
                    debugName = anims->debugName;
                    AnimDebugName = XAnimGetAnimDebugName(anims, animIndex);
                    Com_Error(ERR_DROP, "animation '%s' in '%s' cannot be sync looping and nonlooping", AnimDebugName, debugName);
                }
                else
                {
                    v8 = anims->debugName;
                    v5 = XAnimGetAnimDebugName(anims, animIndex);
                    Com_Error(ERR_DROP, "animation '%s' in '%s' cannot be sync nonlooping and looping", v5, v8);
                }
            }
            return;
        }
        if ( (anims->entries[animIndex].animParent.flags & 3) != 0 )
        {
            count = 0;
            do
            {
                ++count;
                anim = &anims->entries[anim->animParent.children];
            }
            while ( !IsLeafNode(anim) );
            v6 = XAnimGetAnimDebugName(anims, animIndex);
            Com_Error(ERR_DROP, "duplicate specification of animation sync in '%s' %d nodes above '%s'", anims->debugName, count, v6);
        }
    }
    if ( !IsLeafNode(anim) )
    {
        numAnims = anim->numAnims;
        childFlags = parFlags | anim->animParent.flags & 0x63;
        if ( parFlags & 0x20 | anim->animParent.flags & 0x20 )
            ++iFoo;
        flag = anim->animParent.flags & 3;
        if ( flag )
        {
            if ( flag == 3 )
                Com_Error(ERR_DROP, "animation cannot be sync looping and sync nonlooping");
            anim->animParent.flags |= 4u;
        }
        anim->animParent.flags |= parFlags;
        for ( i = 0; i < numAnims; ++i )
            XAnimSetupSyncNodes_r(anims, i + anim->animParent.children, childFlags);
    }
}

bool __cdecl XAnimHasTime(const XAnim_s *anims, unsigned int animIndex)
{
    return IsLeafNode(&anims->entries[animIndex]) || (anims->entries[animIndex].animParent.flags & 3) != 0;
}

bool __cdecl XAnimIsPrimitive(XAnim_s *anims, unsigned int animIndex)
{
    return anims->entries[animIndex].numAnims == 0;
}

void __cdecl XAnimSetTime(XAnimTree_s *tree, unsigned int animIndex, float time, unsigned __int16 cmdIndex)
{
    char *AnimDebugName; // eax
    const char *v5; // eax
    char *v6; // eax
    const char *v7; // eax
    bool bLoop; // [esp+8h] [ebp-28h]
    bool v9; // [esp+10h] [ebp-20h]
    float v10; // [esp+18h] [ebp-18h]
    float v11; // [esp+1Ch] [ebp-14h]
    XAnimInfo *info; // [esp+24h] [ebp-Ch]
    unsigned int infoIndex; // [esp+28h] [ebp-8h]
    const XAnimEntry *anim; // [esp+2Ch] [ebp-4h]

    if ( !tree && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4807, 0, "%s", "tree") )
        __debugbreak();
    if ( !tree->anims
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4808, 0, "%s", "tree->anims") )
    {
        __debugbreak();
    }
    if ( animIndex >= tree->anims->size
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    4809,
                    0,
                    "%s\n\t(animIndex) = %i",
                    "(animIndex < tree->anims->size)",
                    animIndex) )
    {
        __debugbreak();
    }
    infoIndex = XAnimGetInfoIndex(tree, animIndex);
    if ( infoIndex )
    {
        anim = &tree->anims->entries[animIndex];
        if ( !XAnimHasTime(tree->anims, animIndex) )
        {
            AnimDebugName = XAnimGetAnimDebugName(tree->anims, animIndex);
            v5 = va("Anim name: '%s', time: %f", AnimDebugName, time);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                            4816,
                            0,
                            "%s\n\t%s",
                            "XAnimHasTime( tree->anims, animIndex )",
                            v5) )
                __debugbreak();
        }
        if ( Demo_IsPlaying() )
        {
            if ( (float)(time - 1.0) < 0.0 )
                v11 = time;
            else
                v11 = 1.0f;
            if ( (float)(0.0 - time) < 0.0 )
                v10 = v11;
            else
                v10 = 0.0f;
            time = v10;
        }
        if ( (time < 0.0 || time > 1.0)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        4828,
                        0,
                        "%s\n\t(time) = %g",
                        "(time >= 0.0f && time <= 1.0f)",
                        time) )
        {
            __debugbreak();
        }
        if ( XAnimIsLooped(tree->anims, animIndex) )
            v9 = time < 1.0;
        else
            v9 = time <= 1.0;
        if ( !v9 )
        {
            bLoop = anim->parts->bLoop;
            v6 = XAnimGetAnimDebugName(tree->anims, animIndex);
            v7 = va("name: '%s', time: %f, parts->bLoop: %d", v6, time, bLoop);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                            4829,
                            0,
                            "%s\n\t%s",
                            "XAnimIsLooped( tree->anims, animIndex ) ? (time < 1.0f) : (time <= 1.0f)",
                            v7) )
                __debugbreak();
        }
        if ( infoIndex >= 0x1000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        4831,
                        0,
                        "%s\n\t(infoIndex) = %i",
                        "(infoIndex && (infoIndex < 4096))",
                        infoIndex) )
        {
            __debugbreak();
        }
        info = &g_xAnimInfo[infoIndex];
        if (bgs)
        {
            if (bgs->AnimCmdAddDependent)
                bgs->AnimCmdAddDependent(cmdIndex, infoIndex);
        }
        info->state.currentAnimTime = time;
        info->state.cycleCount = 0;
        info->state.oldTime = time;
        info->state.oldCycleCount = 0;
        info->notifyIndex = -1;
    }
}

unsigned int __cdecl XAnimRestart(XAnimTree_s *tree, unsigned int infoIndex, float goalTime, int cmdIndex)
{
    unsigned int parentInfoIndex; // [esp+Ch] [ebp-10h]
    unsigned int parentAnimIndex; // [esp+14h] [ebp-8h]
    const XAnimEntry *anim; // [esp+18h] [ebp-4h]

    if ( !tree && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4905, 0, "%s", "tree") )
        __debugbreak();
    if ( (!infoIndex || infoIndex >= 0x1000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    4906,
                    0,
                    "%s\n\t(infoIndex) = %i",
                    "(infoIndex && (infoIndex < 4096))",
                    infoIndex) )
    {
        __debugbreak();
    }
    for ( parentInfoIndex = infoIndex; parentInfoIndex; parentInfoIndex = g_xAnimInfo[parentInfoIndex].parent )
    {
        if ( parentInfoIndex >= 0x1000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        4912,
                        0,
                        "%s\n\t(parentInfoIndex) = %i",
                        "(parentInfoIndex && (parentInfoIndex < 4096))",
                        parentInfoIndex) )
        {
            __debugbreak();
        }
        parentAnimIndex = g_xAnimInfo[parentInfoIndex].animIndex;
        if ( parentAnimIndex >= tree->anims->size
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        4919,
                        0,
                        "%s",
                        "parentAnimIndex < tree->anims->size") )
        {
            __debugbreak();
        }
        anim = &tree->anims->entries[parentAnimIndex];
        if ( !IsLeafNode(anim) && (anim->animParent.flags & 4) != 0 )
        {
            XAnimInitTime(tree, parentInfoIndex, goalTime, cmdIndex);
            return XAnimGetInfoIndex(tree, g_xAnimInfo[infoIndex].animIndex);
        }
    }
    if ( g_xAnimInfo[infoIndex].animToModel )
        return XAnimInitTime(tree, infoIndex, goalTime, cmdIndex);
    else
        return infoIndex;
}

int __cdecl XAnimSetGoalWeight(
                DObj *obj,
                unsigned int animIndex,
                float goalWeight,
                float goalTime,
                float rate,
                unsigned int notifyName,
                unsigned int notifyType,
                int bRestart,
                int cmdIndex)
{
    XAnimTree_s *tree; // [esp+18h] [ebp-10h]
    int error; // [esp+1Ch] [ebp-Ch]
    unsigned int infoIndex; // [esp+20h] [ebp-8h]
    bool bBypass; // [esp+27h] [ebp-1h]
    int savedregs; // [esp+28h] [ebp+0h] BYREF

    error = 0;
    bBypass = 0;
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 4989, 0, "%s", "obj") )
        __debugbreak();
    if ( goalWeight < 0.001 )
        goalWeight = 0.0f;
    tree = obj->localTree;
    infoIndex = XAnimGetInfoIndex(obj->localTree, animIndex);
    if ( infoIndex )
    {
        if ( bRestart )
            infoIndex = XAnimRestart(tree, infoIndex, goalTime, cmdIndex);
    }
    else if ( goalWeight == 0.0 )
    {
        error = 0;
        bBypass = 1;
    }
    else
    {
        infoIndex = XAnimAllocInfoIndex(obj, animIndex, 0, cmdIndex);
        XAnimInitInfo(&g_xAnimInfo[infoIndex]);
    }
    if ( !bBypass )
        return XAnimSetGoalWeightNode(tree, infoIndex, goalWeight, goalTime, rate, notifyName, notifyType, cmdIndex);
    return error;
}

void __cdecl XAnimSetAnimRate(XAnimTree_s *tree, unsigned int animIndex, float rate)
{
    unsigned int infoIndex; // [esp+0h] [ebp-4h]

    infoIndex = XAnimGetInfoIndex(tree, animIndex);
    if ( infoIndex )
    {
        if ( !tree && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 5132, 0, "%s", "tree") )
            __debugbreak();
        if ( !tree->anims
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 5133, 0, "%s", "tree->anims") )
        {
            __debugbreak();
        }
        if ( animIndex >= tree->anims->size
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        5134,
                        0,
                        "%s\n\t(animIndex) = %i",
                        "(animIndex < tree->anims->size)",
                        animIndex) )
        {
            __debugbreak();
        }
        if ( rate < 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 5135, 0, "%s", "rate >= 0") )
        {
            __debugbreak();
        }
        if ( infoIndex >= 0x1000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                        5137,
                        0,
                        "%s\n\t(infoIndex) = %i",
                        "(infoIndex && (infoIndex < 4096))",
                        infoIndex) )
        {
            __debugbreak();
        }
        g_xAnimInfo[infoIndex].state.rate = rate;
    }
}

bool __cdecl XAnimIsLooped(const XAnim_s *anims, unsigned int animIndex)
{
    if ( !anims && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 5148, 0, "%s", "anims") )
        __debugbreak();
    if ( IsLeafNode(&anims->entries[animIndex]) )
        return anims->entries[animIndex].parts->bLoop;
    else
        return (anims->entries[animIndex].animParent.flags & 1) != 0;
}

unsigned __int8 __cdecl XAnimGetBoneCount(const XAnim_s *anims, unsigned int animIndex)
{
    if ( !anims && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 5161, 0, "%s", "anims") )
        __debugbreak();
    if ( IsLeafNode(&anims->entries[animIndex]) )
        return anims->entries[animIndex].parts->boneCount[9];
    else
        return 0;
}

int __cdecl XAnimHasBone(const XAnim_s *anims, unsigned int animIndex, unsigned __int16 boneName)
{
    int i; // [esp+8h] [ebp-4h]

    if ( !anims && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 5176, 0, "%s", "anims") )
        __debugbreak();
    for ( i = 0; i < anims->entries[animIndex].parts->boneCount[9]; ++i )
    {
        if ( anims->entries[animIndex].parts->names[i] == boneName )
            return 1;
    }
    return 0;
}

bool __cdecl XAnimIsClientNode(const XAnim_s *anims, unsigned int animIndex)
{
    const XAnimEntry *pAnim; // [esp+0h] [ebp-4h]

    if ( !anims && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 5202, 0, "%s", "anims") )
        __debugbreak();
    if ( animIndex >= anims->size
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    5203,
                    0,
                    "%s",
                    "animIndex < anims->size") )
    {
        __debugbreak();
    }
    pAnim = &anims->entries[animIndex];
    if ( IsLeafNode(pAnim) )
        pAnim = &anims->entries[anims->entries[animIndex].parent];
    return (pAnim->animParent.flags & 0x20) != 0;
}

bool __cdecl XAnimIsClientNode(XAnimTree_s *tree, unsigned int animIndex)
{
    return XAnimIsClientNode(tree->anims, animIndex);
}

char __cdecl XAnimNotetrackExists(const XAnim_s *anims, unsigned int animIndex, unsigned int name)
{
    const XAnimNotifyInfo *notify; // [esp+0h] [ebp-10h]
    int notifyIndex; // [esp+4h] [ebp-Ch]
    XAnimParts *parts; // [esp+Ch] [ebp-4h]

    parts = anims->entries[animIndex].parts;
    if ( !parts && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 5231, 0, "%s", "parts") )
        __debugbreak();
    notify = parts->notify;
    if ( !notify )
        return 0;
    for ( notifyIndex = 0; notifyIndex < parts->notifyCount; ++notifyIndex )
    {
        if ( notify->name == name )
            return 1;
        ++notify;
    }
    return 0;
}

void __cdecl XAnimAddNotetrackTimesToScriptArray(const XAnim_s *anims, unsigned int animIndex, unsigned int name)
{
    const XAnimNotifyInfo *notify; // [esp+8h] [ebp-10h]
    int notifyIndex; // [esp+Ch] [ebp-Ch]
    XAnimParts *parts; // [esp+14h] [ebp-4h]

    parts = anims->entries[animIndex].parts;
    if ( !parts && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 5255, 0, "%s", "parts") )
        __debugbreak();
    notify = parts->notify;
    if ( notify )
    {
        for ( notifyIndex = 0; notifyIndex < parts->notifyCount; ++notifyIndex )
        {
            if ( notify->name == name )
            {
                Scr_AddFloat(notify->time, SCRIPTINSTANCE_SERVER);
                Scr_AddArray(SCRIPTINSTANCE_SERVER);
            }
            ++notify;
        }
    }
}

int __cdecl XAnimSetCompleteGoalWeight(
                DObj *obj,
                unsigned int animIndex,
                float goalWeight,
                float goalTime,
                float rate,
                unsigned int notifyName,
                unsigned int notifyType,
                int bRestart,
                int cmdIndex)
{
    XAnimTree_s *tree; // [esp+18h] [ebp-Ch]
    unsigned int infoIndex; // [esp+1Ch] [ebp-8h]
    int savedregs; // [esp+24h] [ebp+0h] BYREF

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 5322, 0, "%s", "obj") )
        __debugbreak();
    if ( goalWeight < 0.001 )
        goalWeight = 0.0f;
    tree = obj->localTree;
    infoIndex = XAnimGetInfoIndex(obj->localTree, animIndex);
    if ( infoIndex )
    {
        if ( bRestart )
            infoIndex = XAnimRestart(tree, infoIndex, goalTime, cmdIndex);
    }
    else
    {
        infoIndex = XAnimAllocInfoIndex(obj, animIndex, 0, cmdIndex);
        XAnimInitInfo(&g_xAnimInfo[infoIndex]);
    }
    return XAnimSetCompleteGoalWeightNode(tree, infoIndex, goalWeight, goalTime, rate, notifyName, notifyType, cmdIndex);
}

void __cdecl XAnimCloneAnimInfo(XAnimTree_s *toTree, const XAnimInfo *from, XAnimInfo *to)
{
    if ( to->animIndex != from->animIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    5349,
                    0,
                    "%s",
                    "to->animIndex == from->animIndex") )
    {
        __debugbreak();
    }
    memcpy(&to->state, &from->state, sizeof(to->state));
    to->notifyChild = from->notifyChild;
    to->notifyIndex = from->notifyIndex;
    to->notifyName = from->notifyName;
    to->notifyType = from->notifyType;
    if ( to->notifyName )
        SL_AddRefToString(to->notifyName, (scriptInstance_t)toTree->inst);
}

void __cdecl XAnimCloneAnimTree(const XAnimTree_s *from, XAnimTree_s *to)
{
    if ( !from && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 5403, 0, "%s", "from") )
        __debugbreak();
    if ( !from->anims
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 5404, 0, "%s", "from->anims") )
    {
        __debugbreak();
    }
    if ( !from->anims->size
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 5405, 0, "%s", "from->anims->size") )
    {
        __debugbreak();
    }
    if ( !to && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 5406, 0, "%s", "to") )
        __debugbreak();
    if ( to->anims != from->anims
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    5407,
                    0,
                    "%s",
                    "to->anims == from->anims") )
    {
        __debugbreak();
    }
    if ( to->children )
        XAnimFreeInfo(to, to->children);
    if ( to->children
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 5411, 0, "%s", "!to->children") )
    {
        __debugbreak();
    }
    to->inst = from->inst;
    if ( from->children )
        XAnimCloneAnimTree_r(from, to, from->children, 0);
}

void __cdecl XAnimCloneAnimTree_r(
                const XAnimTree_s *from,
                XAnimTree_s *to,
                unsigned int fromInfoIndex,
                unsigned int toInfoParentIndex)
{
    unsigned int toInfoIndex; // [esp+4h] [ebp-10h]
    unsigned int fromChildInfoIndex; // [esp+8h] [ebp-Ch]
    XAnimInfo *fromInfo; // [esp+Ch] [ebp-8h]
    unsigned int animToModel; // [esp+10h] [ebp-4h]

    if ( !from && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 5370, 0, "%s", "from") )
        __debugbreak();
    if ( !from->anims
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 5371, 0, "%s", "from->anims") )
    {
        __debugbreak();
    }
    if ( !from->anims->size
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 5372, 0, "%s", "from->anims->size") )
    {
        __debugbreak();
    }
    if ( !to && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 5373, 0, "%s", "to") )
        __debugbreak();
    if ( to->anims != from->anims
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    5374,
                    0,
                    "%s",
                    "to->anims == from->anims") )
    {
        __debugbreak();
    }
    if ( (!fromInfoIndex || fromInfoIndex >= 0x1000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    5376,
                    0,
                    "%s\n\t(fromInfoIndex) = %i",
                    "(fromInfoIndex && (fromInfoIndex < 4096))",
                    fromInfoIndex) )
    {
        __debugbreak();
    }
    fromInfo = &g_xAnimInfo[fromInfoIndex];
    animToModel = fromInfo->animToModel;
    if ( fromInfo->animToModel )
        SL_AddRefToString(animToModel, SCRIPTINSTANCE_SERVER);
    toInfoIndex = XAnimAllocInfoWithParent(to, animToModel, fromInfo->animIndex, toInfoParentIndex, 1);
    XAnimCloneAnimInfo(to, fromInfo, &g_xAnimInfo[toInfoIndex]);
    for ( fromChildInfoIndex = fromInfo->children;
                fromChildInfoIndex;
                fromChildInfoIndex = g_xAnimInfo[fromChildInfoIndex].next )
    {
        XAnimCloneAnimTree_r(from, to, fromChildInfoIndex, toInfoIndex);
    }
}

XAnimInfo *__cdecl GetAnimInfo(int infoIndex)
{
    if ( (infoIndex <= 0 || infoIndex >= 4096)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    5516,
                    0,
                    "%s\n\t(infoIndex) = %i",
                    "(infoIndex > 0 && infoIndex < 4096)",
                    infoIndex) )
    {
        __debugbreak();
    }
    return &g_xAnimInfo[infoIndex];
}

char __cdecl XAnimGetParamValue(const XAnim_s *anims, unsigned int animIndex, const char *paramName, float *outValue)
{
    const XAnimParam *pParam; // [esp+0h] [ebp-Ch]
    unsigned int iParam; // [esp+4h] [ebp-8h]
    unsigned __int16 iBaseParamIx; // [esp+8h] [ebp-4h]

    if ( animIndex >= anims->size
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    5538,
                    0,
                    "%s",
                    "animIndex < anims->size") )
    {
        __debugbreak();
    }
    if ( !outValue && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 5539, 0, "%s", "outValue") )
        __debugbreak();
    iBaseParamIx = anims->entries[animIndex].firstParamIxPlusOne - 1;
    for ( iParam = 0; iParam < anims->entries[animIndex].paramCount; ++iParam )
    {
        pParam = &anims->params[iParam + iBaseParamIx];
        if ( !strncmp(pParam->strName, paramName, 0x10u) )
        {
            *outValue = pParam->fValue;
            return 1;
        }
    }
    return 0;
}

bool __cdecl XAnimGetParamValue(XAnimTree_s *tree, unsigned int animIndex, const char *paramName, float *outValue)
{
    return XAnimGetParamValueTree(tree->anims, animIndex, paramName, outValue);
}

bool __cdecl XAnimGetParamValueTree(
                const XAnim_s *anims,
                unsigned int animIndex,
                const char *paramName,
                float *outValue)
{
    if ( animIndex >= anims->size
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp",
                    5558,
                    0,
                    "%s",
                    "animIndex < anims->size") )
    {
        __debugbreak();
    }
    if ( !outValue && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim.cpp", 5559, 0, "%s", "outValue") )
        __debugbreak();
    if ( !animIndex )
        return 0;
    if ( XAnimGetParamValue(anims, animIndex, paramName, outValue) )
        return 0;
    return XAnimGetParamValueTree(anims, anims->entries[animIndex].parent, paramName, outValue);
}

//void __thiscall bitarray<160>::setBit(bitarray<160> *this, unsigned int pos)
//{
//    if ( pos >= 0xA0
//        && !Assert_MyHandler(
//                    "c:\\projects_pc\\cod\\codsrc\\src\\universal\\../qcommon/bitarray.h",
//                    115,
//                    0,
//                    "%s",
//                    "pos < BIT_COUNT") )
//    {
//        __debugbreak();
//    }
//    this->array[pos >> 5] |= 0x80000000 >> (pos & 0x1F);
//}

void __cdecl XAnim_SetTime(float time, int frameCount, XAnimTime *animTime)
{
    animTime->time = time;
    animTime->frameFrac = (float)frameCount * time;
    animTime->frameIndex = (int)animTime->frameFrac;
}

