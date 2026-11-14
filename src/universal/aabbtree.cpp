#include "aabbtree.h"

#include <cstring>
#include "assertive.h"
#include "com_math.h"
#include <cstdlib>
#include <cfloat>
#include <qcommon/common.h>

float *sortedMins;
float *sortedMaxs;
float *sortedCoplanar;
int aabbTreeCount;

int __cdecl BuildAabbTree(const GenericAabbTreeOptions *options)
{
    float *v2; // [esp+4h] [ebp-454h]
    float *v3; // [esp+8h] [ebp-450h]
    float *v4; // [esp+Ch] [ebp-44Ch]
    float *v5; // [esp+10h] [ebp-448h]
    unsigned __int8 *boundCopies; // [esp+44h] [ebp-414h]
    int *remap; // [esp+48h] [ebp-410h]
    int itemIndex; // [esp+4Ch] [ebp-40Ch]
    int itemIndexa; // [esp+4Ch] [ebp-40Ch]
    int itemIndexb; // [esp+4Ch] [ebp-40Ch]
    int itemIndexc; // [esp+4Ch] [ebp-40Ch]
    int remapBuffer[64]; // [esp+50h] [ebp-408h] BYREF
    float sortedBounds[3][64]; // [esp+150h] [ebp-308h] BYREF
    unsigned __int8 *itemCopies; // [esp+454h] [ebp-4h]

    if ( options->itemCount > 0x40u )
    {
        remap = (int *)operator new(4 * options->itemCount);
        sortedMins = (float *)operator new(4 * options->itemCount);
        sortedMaxs = (float *)operator new(4 * options->itemCount);
        sortedCoplanar = (float *)operator new(4 * options->itemCount);
    }
    else
    {
        remap = remapBuffer;
        sortedMins = sortedBounds[0];
        sortedMaxs = sortedBounds[1];
        sortedCoplanar = sortedBounds[2];
    }
    for ( itemIndex = 0; itemIndex < options->itemCount; ++itemIndex )
        remap[itemIndex] = itemIndex;
    options->treeNodePool->firstItem = 0;
    options->treeNodePool->itemCount = options->itemCount;
    aabbTreeCount = 1;
    BuildAabbTree_r(options->treeNodePool, options, remap);
    itemCopies = (unsigned __int8 *)operator new(options->itemSize * options->itemCount);
    memcpy(itemCopies, (unsigned __int8 *)options->items, options->itemSize * options->itemCount);
    for ( itemIndexa = 0; itemIndexa < options->itemCount; ++itemIndexa )
        memcpy(
            (unsigned __int8 *)options->items + options->itemSize * itemIndexa,
            &itemCopies[options->itemSize * remap[itemIndexa]],
            options->itemSize);
    operator delete(itemCopies);
    if ( options->maintainValidBounds )
    {
        boundCopies = (unsigned __int8 *)operator new(
                                                                             4
                                                                         * ((3 * (unsigned __int64)(unsigned int)options->itemCount) >> 32 != 0
                                                                            ? -1
                                                                            : 3 * options->itemCount));
        memcpy(boundCopies, (unsigned __int8 *)options->mins, 12 * options->itemCount);
        for ( itemIndexb = 0; itemIndexb < options->itemCount; ++itemIndexb )
        {
            v4 = options->mins[itemIndexb];
            v5 = (float *)&boundCopies[12 * remap[itemIndexb]];
            *v4 = *v5;
            v4[1] = v5[1];
            v4[2] = v5[2];
        }
        memcpy(boundCopies, (unsigned __int8 *)options->maxs, 12 * options->itemCount);
        for ( itemIndexc = 0; itemIndexc < options->itemCount; ++itemIndexc )
        {
            v2 = options->maxs[itemIndexc];
            v3 = (float *)&boundCopies[12 * remap[itemIndexc]];
            *v2 = *v3;
            v2[1] = v3[1];
            v2[2] = v3[2];
        }
        operator delete(boundCopies);
    }
    if ( remap != remapBuffer )
    {
        operator delete(remap);
        operator delete(sortedMins);
        operator delete(sortedMaxs);
        operator delete(sortedCoplanar);
    }
    return aabbTreeCount;
}

void __cdecl BuildAabbTree_r(GenericAabbTree *tree, const GenericAabbTreeOptions *options, int *remap)
{
    int midStart; // [esp+0h] [ebp-10h] BYREF
    int childIndex; // [esp+4h] [ebp-Ch]
    int lastStart; // [esp+8h] [ebp-8h] BYREF
    GenericAabbTree *subtree; // [esp+Ch] [ebp-4h]

    if ( !tree->itemCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\aabbtree.cpp", 388, 0, "%s", "tree->itemCount") )
    {
        __debugbreak();
    }
    tree->firstChild = aabbTreeCount;
    tree->childCount = 0;
    if ( tree->itemCount > options->maxItemsPerLeaf
        && SplitAabbTree(tree->itemCount, options, remap, &midStart, &lastStart) )
    {
        subtree = &options->treeNodePool[aabbTreeCount];
        if ( tree->firstChild != aabbTreeCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\aabbtree.cpp",
                        400,
                        1,
                        "%s",
                        "tree->firstChild == aabbTreeCount") )
        {
            __debugbreak();
        }
        CreateAabbSubTrees(tree, options, remap, 0, midStart);
        if ( midStart < lastStart )
            CreateAabbSubTrees(tree, options, remap, midStart, lastStart - midStart);
        CreateAabbSubTrees(tree, options, remap, lastStart, tree->itemCount - lastStart);
        tree->childCount = aabbTreeCount - tree->firstChild;
        for ( childIndex = 0; childIndex < tree->childCount; ++childIndex )
            BuildAabbTree_r(&subtree[childIndex], options, &remap[subtree[childIndex].firstItem - tree->firstItem]);
    }
}

int __cdecl SplitAabbTree(int count, const GenericAabbTreeOptions *options, int *remap, int *midStart, int *lastStart)
{
    float v6; // [esp+4h] [ebp-58h]
    float v7; // [esp+8h] [ebp-54h]
    int top; // [esp+Ch] [ebp-50h]
    float *mins; // [esp+10h] [ebp-4Ch]
    int splitAxis; // [esp+14h] [ebp-48h] BYREF
    int bot; // [esp+18h] [ebp-44h]
    float bounds[2][2][3]; // [esp+1Ch] [ebp-40h] BYREF
    float (*maxs)[3]; // [esp+4Ch] [ebp-10h]
    float splitDist; // [esp+50h] [ebp-Ch] BYREF
    int swapCache; // [esp+54h] [ebp-8h]
    int mid; // [esp+58h] [ebp-4h]

    mins = (float *)options->mins;
    maxs = options->maxs;
    if ( !PickAabbSplitPlane((float (*)[3])mins, maxs, remap, count, &splitAxis, &splitDist) )
        return 0;
    ClearBounds(bounds[0][0], bounds[0][1]);
    ClearBounds(bounds[1][0], bounds[1][1]);
    bot = 0;
    top = count - 1;
    while ( bot <= top )
    {
        while ( bot <= top && splitDist >= maxs[remap[bot]][splitAxis] && splitDist > mins[3 * remap[bot] + splitAxis] )
        {
            ExpandBounds(&mins[3 * remap[bot]], maxs[remap[bot]], bounds[0][0], bounds[0][1]);
            ++bot;
        }
        while ( bot <= top && mins[3 * remap[top] + splitAxis] >= splitDist && maxs[remap[top]][splitAxis] > splitDist )
        {
            ExpandBounds(&mins[3 * remap[top]], maxs[remap[top]], bounds[1][0], bounds[1][1]);
            --top;
        }
        if ( bot > top )
            break;
        if ( (mins[3 * remap[bot] + splitAxis] < splitDist || maxs[remap[bot]][splitAxis] <= splitDist)
            && (splitDist < maxs[remap[top]][splitAxis] || splitDist <= mins[3 * remap[top] + splitAxis]) )
        {
            for ( mid = bot; mid < top; ++mid )
            {
                if ( mins[3 * remap[mid] + splitAxis] >= splitDist && maxs[remap[mid]][splitAxis] > splitDist )
                {
                    swapCache = remap[mid];
                    remap[mid] = remap[top];
                    remap[top] = swapCache;
                    break;
                }
                if ( splitDist >= maxs[remap[mid]][splitAxis] && splitDist > mins[3 * remap[mid] + splitAxis] )
                {
                    swapCache = remap[mid];
                    remap[mid] = remap[bot];
                    remap[bot] = swapCache;
                    break;
                }
            }
            if ( mid == top )
                break;
        }
        else
        {
            swapCache = remap[bot];
            remap[bot] = remap[top];
            remap[top] = swapCache;
        }
    }
    if ( bot <= top
        && (bot < options->minItemsPerLeaf
         || top - bot + 1 < options->minItemsPerLeaf
         || count - top - 1 < options->minItemsPerLeaf) )
    {
        while ( bot <= top )
        {
            while ( bot <= top )
            {
                v7 = AddedVolume(&mins[3 * remap[bot]], maxs[remap[bot]], bounds[0][0], bounds[0][1]);
                if ( AddedVolume(&mins[3 * remap[bot]], maxs[remap[bot]], bounds[1][0], bounds[1][1]) < v7 )
                    break;
                ExpandBounds(&mins[3 * remap[bot]], maxs[remap[bot]], bounds[0][0], bounds[0][1]);
                ++bot;
            }
            while ( bot <= top )
            {
                v6 = AddedVolume(&mins[3 * remap[top]], maxs[remap[top]], bounds[1][0], bounds[1][1]);
                if ( AddedVolume(&mins[3 * remap[top]], maxs[remap[top]], bounds[0][0], bounds[0][1]) < v6 )
                    break;
                ExpandBounds(&mins[3 * remap[top]], maxs[remap[top]], bounds[1][0], bounds[1][1]);
                --top;
            }
            if ( bot >= top )
            {
                if ( bot == top )
                {
                    if ( 2 * bot >= count )
                        --top;
                    else
                        ++bot;
                }
            }
            else
            {
                swapCache = remap[bot];
                remap[bot] = remap[top];
                remap[top] = swapCache;
                ++bot;
                --top;
            }
        }
    }
    if ( !bot || bot == count )
        return 0;
    *midStart = bot;
    *lastStart = top + 1;
    return 1;
}

bool __cdecl PickAabbSplitPlane(
                float (*mins)[3],
                float (*maxs)[3],
                int *remap,
                int count,
                int *chosenAxis,
                float *chosenDist)
{
    float v7; // [esp+4h] [ebp-A0h]
    int sideSplitCount; // [esp+38h] [ebp-6Ch]
    float nextDist; // [esp+3Ch] [ebp-68h]
    int prevMinCount; // [esp+40h] [ebp-64h]
    int prevOnCount; // [esp+44h] [ebp-60h]
    float dist; // [esp+48h] [ebp-5Ch]
    signed int minMaxCount; // [esp+4Ch] [ebp-58h]
    signed int coplanarCount; // [esp+50h] [ebp-54h]
    int axisIndex; // [esp+54h] [ebp-50h]
    signed int bestHeuristic; // [esp+58h] [ebp-4Ch]
    int smallestAxis; // [esp+5Ch] [ebp-48h]
    int maxIndex; // [esp+60h] [ebp-44h]
    float globalMaxs[3]; // [esp+64h] [ebp-40h] BYREF
    int sideFrontCount; // [esp+70h] [ebp-34h]
    int i; // [esp+74h] [ebp-30h]
    int sideOnCount; // [esp+78h] [ebp-2Ch]
    float globalMins[3]; // [esp+7Ch] [ebp-28h] BYREF
    int axisBias[3]; // [esp+88h] [ebp-1Ch]
    int minIndex; // [esp+94h] [ebp-10h]
    int onIndex; // [esp+98h] [ebp-Ch]
    int heuristic; // [esp+9Ch] [ebp-8h]
    int sideBackCount; // [esp+A0h] [ebp-4h]

    ClearBounds(globalMins, globalMaxs);
    for ( i = 0; i < count; ++i )
        ExpandBounds(&(*mins)[3 * remap[i]], &(*maxs)[3 * remap[i]], globalMins, globalMaxs);
    smallestAxis = (float)(globalMaxs[0] - globalMins[0]) > (float)(globalMaxs[1] - globalMins[1]);
    if ( (float)(globalMaxs[smallestAxis] - globalMins[smallestAxis]) > (float)(globalMaxs[2] - globalMins[2]) )
        smallestAxis = 2;
    for ( i = 0; i < 3; ++i )
        axisBias[i] = (int)((float)((float)((float)((float)(globalMaxs[i] - globalMins[i]) + 1.0) * 10.0)
                                                            / (float)((float)(globalMaxs[smallestAxis] - globalMins[smallestAxis]) + 1.0))
                                            + 0.4999999990686774);
    bestHeuristic = 0x80000000;
    for ( axisIndex = 0; axisIndex < 3; ++axisIndex )
    {
        minMaxCount = 0;
        coplanarCount = 0;
        for ( i = 0; i < count; ++i )
        {
            if ( (*mins)[3 * remap[i] + axisIndex] == (*maxs)[3 * remap[i] + axisIndex] )
            {
                sortedCoplanar[coplanarCount++] = (*mins)[3 * remap[i] + axisIndex];
            }
            else
            {
                sortedMins[minMaxCount] = (*mins)[3 * remap[i] + axisIndex];
                sortedMaxs[minMaxCount++] = (*maxs)[3 * remap[i] + axisIndex];
            }
        }
        qsort(sortedMins, minMaxCount, 4u, (int (__cdecl *)(const void *, const void *))compare_floats);
        qsort(sortedMaxs, minMaxCount, 4u, (int (__cdecl *)(const void *, const void *))compare_floats);
        qsort(sortedCoplanar, coplanarCount, 4u, (int (__cdecl *)(const void *, const void *))compare_floats);
        sideFrontCount = 0;
        sideBackCount = count;
        sideSplitCount = 0;
        sideOnCount = 0;
        minIndex = 0;
        maxIndex = 0;
        onIndex = 0;
        prevMinCount = 0;
        prevOnCount = 0;
        if ( (float)(*sortedCoplanar - *sortedMins) < 0.0 )
            v7 = *sortedCoplanar;
        else
            v7 = *sortedMins;
        nextDist = v7;
        while ( nextDist < 3.4028235e38 )
        {
            dist = nextDist;
            nextDist = FLT_MAX;
            sideSplitCount += prevMinCount;
            sideBackCount -= prevMinCount;
            prevMinCount = 0;
            while ( minIndex < minMaxCount && sortedMins[minIndex] == dist )
            {
                ++prevMinCount;
                ++minIndex;
            }
            if ( minIndex < minMaxCount && sortedMins[minIndex] < 3.4028235e38 )
                nextDist = sortedMins[minIndex];
            while ( maxIndex < minMaxCount && sortedMaxs[maxIndex] == dist )
            {
                ++sideFrontCount;
                --sideSplitCount;
                ++maxIndex;
            }
            if ( maxIndex < minMaxCount && nextDist > sortedMaxs[maxIndex] )
                nextDist = sortedMaxs[maxIndex];
            sideFrontCount += prevOnCount;
            sideOnCount -= prevOnCount;
            prevOnCount = 0;
            while ( onIndex < coplanarCount && sortedCoplanar[onIndex] == dist )
            {
                ++prevOnCount;
                ++onIndex;
            }
            sideOnCount += prevOnCount;
            sideBackCount -= prevOnCount;
            if ( onIndex < coplanarCount && nextDist > sortedCoplanar[onIndex] )
                nextDist = sortedCoplanar[onIndex];
            if ( sideOnCount + sideSplitCount + sideBackCount + sideFrontCount != count
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\aabbtree.cpp",
                            163,
                            0,
                            "%s",
                            "sideFrontCount + sideBackCount + sideSplitCount + sideOnCount == count") )
            {
                __debugbreak();
            }
            if ( sideFrontCount < 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\aabbtree.cpp",
                            164,
                            0,
                            "%s",
                            "sideFrontCount >= 0") )
            {
                __debugbreak();
            }
            if ( sideBackCount < 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\aabbtree.cpp",
                            165,
                            0,
                            "%s",
                            "sideBackCount >= 0") )
            {
                __debugbreak();
            }
            if ( sideSplitCount < 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\aabbtree.cpp",
                            166,
                            0,
                            "%s",
                            "sideSplitCount >= 0") )
            {
                __debugbreak();
            }
            if ( sideOnCount < 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\aabbtree.cpp",
                            167,
                            0,
                            "%s",
                            "sideOnCount >= 0") )
            {
                __debugbreak();
            }
            if ( sideFrontCount > 1 && sideBackCount > 1 )
            {
                heuristic = axisBias[axisIndex]
                                    + count
                                    - abs(sideFrontCount - sideBackCount)
                                    - sideOnCount
                                    - 4 * sideSplitCount;
                if ( !sideOnCount && !sideSplitCount && !prevMinCount )
                    heuristic += SnapFloatToInt((float)(nextDist - dist));
                if ( heuristic > bestHeuristic )
                {
                    bestHeuristic = heuristic;
                    *chosenAxis = axisIndex;
                    if ( sideOnCount || sideSplitCount || prevMinCount )
                        *chosenDist = dist;
                    else
                        *chosenDist = (float)(dist + nextDist) * 0.5;
                }
            }
        }
    }
    return bestHeuristic != 0x80000000;
}

int __cdecl compare_floats(float *e0, float *e1)
{
    float diff; // [esp+4h] [ebp-4h]

    diff = *e0 - *e1;
    if ( diff >= 0.0 )
        return diff > 0.0;
    else
        return -1;
}

double __cdecl AddedVolume(const float *addedmins, const float *addedmaxs, const float *mins, const float *maxs)
{
    float expandedMins[3]; // [esp+0h] [ebp-20h] BYREF
    float expandedVolume; // [esp+Ch] [ebp-14h]
    float expandedMaxs[3]; // [esp+14h] [ebp-Ch] BYREF

    expandedMins[0] = *mins;
    expandedMins[1] = mins[1];
    expandedMins[2] = mins[2];
    expandedMaxs[0] = *maxs;
    expandedMaxs[1] = maxs[1];
    expandedMaxs[2] = maxs[2];
    ExpandBounds(addedmins, addedmaxs, expandedMins, expandedMaxs);
    expandedVolume = (float)((float)(expandedMaxs[0] - expandedMins[0]) * (float)(expandedMaxs[1] - expandedMins[1]))
                                 * (float)(expandedMaxs[2] - expandedMins[2]);
    return expandedVolume
             - (float)((float)((float)(*maxs - *mins) * (float)(maxs[1] - mins[1])) * (float)(maxs[2] - mins[2]));
}

void __cdecl CreateAabbSubTrees(
                GenericAabbTree *tree,
                const GenericAabbTreeOptions *options,
                int *remap,
                int firstIndex,
                int count)
{
    int midStart; // [esp+0h] [ebp-Ch] BYREF
    int lastStart; // [esp+4h] [ebp-8h] BYREF
    GenericAabbTree *subtree; // [esp+8h] [ebp-4h]

    if ( count > options->maxItemsPerLeaf && SplitAabbTree(count, options, &remap[firstIndex], &midStart, &lastStart) )
    {
        subtree = AllocAabbTreeNode(options);
        subtree->firstItem = firstIndex + tree->firstItem;
        subtree->itemCount = midStart;
        if ( midStart < lastStart )
        {
            subtree = AllocAabbTreeNode(options);
            subtree->firstItem = midStart + firstIndex + tree->firstItem;
            subtree->itemCount = lastStart - midStart;
        }
        subtree = AllocAabbTreeNode(options);
        subtree->firstItem = lastStart + firstIndex + tree->firstItem;
        subtree->itemCount = count - lastStart;
    }
    else
    {
        subtree = AllocAabbTreeNode(options);
        subtree->firstItem = firstIndex + tree->firstItem;
        subtree->itemCount = count;
    }
}

GenericAabbTree *__cdecl AllocAabbTreeNode(const GenericAabbTreeOptions *options)
{
    if ( aabbTreeCount == options->treeNodeLimit )
        Com_Error(ERR_DROP, "More than %i AABB nodes needed\n", options->treeNodeLimit);
    return &options->treeNodePool[aabbTreeCount++];
}

