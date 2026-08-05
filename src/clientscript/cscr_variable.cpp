#include "cscr_variable.h"
#include <universal/com_memory.h>
#include <qcommon/common.h>
#include "cscr_parser.h"
#include "cscr_debugger.h"
#include "cscr_animtree.h"
#include "cscr_vm.h"
#include <universal/physicalmemory.h>
#include "cscr_instance.h"
#include "cscr_stringlist.h"
#include "cscr_readwrite.h"
#include "cscr_memorytree.h"
#include <universal/com_files.h>
#include "cscr_tempmemory.h"
#include <universal/q_parse.h>
#include <database/db_registry.h>

#undef GetObject // windows aids

scrVarGlob_t gScrVarGlob[2];
scrVarPub_t gScrVarPub[2];
scrVarDebugPub_t *gScrVarDebugPub[2];

scr_classStruct_t gServerClassMap[5] =
{
  { 0u, 0u, 'e', "entity" },
  { 0u, 0u, 'h', "hudelem" },
  { 0u, 0u, 'p', "pathnode" },
  { 0u, 0u, 'v', "vehiclenode" },
  { 0u, 0u, 'd', "dynentity" }
};

scr_classStruct_t gClientClassMap[5] =
{
  { 0u, 0u, 'e', "entity" },
  { 0u, 0u, 'h', "hudelem" },
  { 0u, 0u, 'p', "pathnode" },
  { 0u, 0u, 'v', "vehiclenode" },
  { 0u, 0u, 'd', "dynentity" }
};



scr_classStruct_t *gScrClassMap[2] = { gServerClassMap, gClientClassMap };

void __cdecl Scr_DumpScriptThreads(scriptInstance_t inst)
{
    double ThreadUsage; // st7
    double ObjectUsage; // st7
    int NumScriptVars; // eax
    int NumScriptVarsParent; // eax
    int NumScriptVarsChild; // eax
    int NumScriptThreads; // eax
    int j; // [esp+18h] [ebp-DCh]
    int ja; // [esp+18h] [ebp-DCh]
    unsigned int classnum; // [esp+1Ch] [ebp-D8h]
    const char *pos; // [esp+20h] [ebp-D4h]
    ThreadDebugInfo info; // [esp+24h] [ebp-D0h]
    const char *buf; // [esp+B8h] [ebp-3Ch]
    int size; // [esp+BCh] [ebp-38h]
    VariableValueInternal *entryValue; // [esp+C0h] [ebp-34h]
    ThreadDebugInfo *pInfo; // [esp+C4h] [ebp-30h]
    int num; // [esp+C8h] [ebp-2Ch]
    unsigned __int8 type; // [esp+CFh] [ebp-25h]
    VariableUnion u; // [esp+D0h] [ebp-24h]
    int i; // [esp+D4h] [ebp-20h]
    const VariableStackBuffer *stackBuf; // [esp+D8h] [ebp-1Ch]
    unsigned int entId; // [esp+DCh] [ebp-18h]
    ThreadDebugInfo *infoArray; // [esp+E0h] [ebp-14h]
    int count; // [esp+E4h] [ebp-10h]
    float endonUsage; // [esp+E8h] [ebp-Ch]
    unsigned int id; // [esp+ECh] [ebp-8h]
    float varUsage; // [esp+F0h] [ebp-4h]

    num = 0;
    for ( id = 1; id < 0x3FFFE; ++id )
    {
        entryValue = &gScrVarGlob[inst].variableList[id + 0x8000];
        if ( (entryValue->w.status & 0x60) != 0 && (entryValue->w.status & 0x1F) == 0xA )
            ++num;
    }
    if ( num )
    {
        infoArray = (ThreadDebugInfo *)Z_TryVirtualAlloc(140 * num, "Scr_DumpScriptThreads", 0);
        if ( infoArray )
        {
            num = 0;
            for ( id = 1; id < 0x3FFFE; ++id )
            {
                entryValue = &gScrVarGlob[inst].variableList[id + 0x8000];
                if ( (entryValue->w.status & 0x60) != 0 && (entryValue->w.status & 0x1F) == 0xA )
                {
                    pInfo = &infoArray[num++];
                    info.posSize = 0;
                    stackBuf = entryValue->u.u.stackValue;
                    size = stackBuf->size;
                    pos = stackBuf->pos;
                    buf = stackBuf->buf;
                    while ( size )
                    {
                        --size;
                        type = *buf++;
                        u.intValue = *(int *)buf;
                        buf += 4;
                        if ( type == 7 )
                            info.pos[info.posSize++] = u.codePosValue;
                    }
                    info.pos[info.posSize++] = pos;
                    ThreadUsage = Scr_GetThreadUsage(inst, stackBuf, &pInfo->endonUsage);
                    pInfo->varUsage = ThreadUsage;
                    pInfo->posSize = info.posSize--;
                    for ( j = 0; j < pInfo->posSize; ++j )
                        pInfo->pos[j] = info.pos[info.posSize - j];
                }
            }
            qsort(infoArray, num, 0x8Cu, (int (__cdecl *)(const void *, const void *))ThreadInfoCompare);
            Com_Printf(24, "********************************\n");
            varUsage = 0.0f;
            endonUsage = 0.0f;
            i = 0;
            while ( i < num )
            {
                pInfo = &infoArray[i];
                count = 0;
                info.varUsage = 0.0f;
                info.endonUsage = 0.0f;
                do
                {
                    ++count;
                    info.varUsage = info.varUsage + infoArray[i].varUsage;
                    info.endonUsage = info.endonUsage + infoArray[i++].endonUsage;
                }
                while ( i < num && !ThreadInfoCompare((unsigned int*)pInfo->pos, (unsigned int *)infoArray[i].pos) );
                varUsage = varUsage + info.varUsage;
                endonUsage = endonUsage + info.endonUsage;
                Com_Printf(24, "count: %d, var usage: %d, endon usage: %d\n", count, (int)info.varUsage, (int)info.endonUsage);
                Scr_PrintPrevCodePos(inst, 24, (char *)pInfo->pos[0], 0);
                for ( ja = 1; ja < pInfo->posSize; ++ja )
                {
                    Com_Printf(24, "called from:\n");
                    Scr_PrintPrevCodePos(inst, 24, (char *)pInfo->pos[ja], 0);
                }
            }
            Z_VirtualFree(infoArray, 0);
            Com_Printf(24, "********************************\n");
            Com_Printf(24, "var usage: %d, endon usage: %d\n", (int)varUsage, (int)endonUsage);
            Com_Printf(24, "\n");
            for ( classnum = 0; classnum < 5; ++classnum )
            {
                if ( gScrClassMap[inst][classnum].entArrayId )
                {
                    info.varUsage = 0.0f;
                    count = 0;
                    for ( entId = FindFirstSibling(inst, gScrClassMap[inst][classnum].entArrayId);
                                entId;
                                entId = FindNextSibling(inst, entId) )
                    {
                        ++count;
                        if ( (gScrVarGlob[inst].variableList[entId + 0x8000].w.status & 0x1F) == 1 )
                        {
                            ObjectUsage = Scr_GetObjectUsage(inst, gScrVarGlob[inst].variableList[entId + 0x8000].u.next);
                            info.varUsage = ObjectUsage + info.varUsage;
                        }
                    }
                    Com_Printf(
                        24,
                        "ent type '%s'... count: %d, var usage: %d\n",
                        gScrClassMap[inst][classnum].name,
                        count,
                        (int)info.varUsage);
                }
            }
            Com_Printf(24, "********************************\n");
            NumScriptVars = Scr_GetNumScriptVars(inst);
            Com_Printf(24, "num vars:        %d\n", NumScriptVars);
            NumScriptVarsParent = Scr_GetNumScriptVarsParent(inst);
            Com_Printf(24, "parent:        %d\n", NumScriptVarsParent);
            NumScriptVarsChild = Scr_GetNumScriptVarsChild(inst);
            Com_Printf(24, "child:        %d\n", NumScriptVarsChild);
            NumScriptThreads = Scr_GetNumScriptThreads(inst);
            Com_Printf(24, "num threads: %d\n", NumScriptThreads);
            Com_Printf(24, "********************************\n");
        }
        else
        {
            Com_Printf(24, "Cannot dump script threads: out of memory\n");
        }
    }
}

int __cdecl ThreadInfoCompare(unsigned int *info1, unsigned int *info2)
{
    const char *pos1; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    const char *pos2; // [esp+8h] [ebp-4h]

    for ( i = 0; ; ++i )
    {
        if ( i >= info1[32] || i >= info2[32] )
            return info1[32] - info2[32];
        pos1 = (const char *)info1[i];
        pos2 = (const char *)info2[i];
        if ( pos1 != pos2 )
            break;
    }
    return pos1 - pos2;
}

void __cdecl Scr_DumpScriptVariables(
    scriptInstance_t inst,
    bool spreadsheet,
    bool summary,
    bool total,
    bool functionSummary,
    bool lineSort,
    const char *fileName,
    const char *functionName,
    int minCount)
{
    unsigned int NumScriptVars; // eax
    char *pos; // [esp+0h] [ebp-24h]
    int(__cdecl * VariableInfoCompareCallBack)(const void *, const void *); // [esp+4h] [ebp-20h]
    unsigned int index; // [esp+8h] [ebp-1Ch]
    VariableDebugInfo *pInfo; // [esp+Ch] [ebp-18h]
    VariableDebugInfo *pInfoa; // [esp+Ch] [ebp-18h]
    VariableDebugInfo *pInfob; // [esp+Ch] [ebp-18h]
    signed int num; // [esp+10h] [ebp-14h]
    int filteredCount; // [esp+14h] [ebp-10h]
    int i; // [esp+18h] [ebp-Ch]
    int ia; // [esp+18h] [ebp-Ch]
    char *infoArray; // [esp+1Ch] [ebp-8h]
    int count; // [esp+20h] [ebp-4h]

    if (gScrVarDebugPub[inst]
        && (gScrVarPub[inst].developer
            || !spreadsheet && !fileName && !functionName && !lineSort && !functionSummary && !minCount))
    {
        infoArray = Z_TryVirtualAlloc(4718560, "Scr_DumpScriptVariables", 0);
        if (infoArray)
        {
            num = 0;
            for (index = 0; index < 0x47FFE; ++index)
            {
                pos = (char *)gScrVarDebugPub[inst]->varUsage[index];
                if (pos)
                {
                    pInfo = (VariableDebugInfo *)&infoArray[16 * num];
                    if (!fileName || Scr_PrevCodePosFileNameMatches(inst, pos, fileName))
                    {
                        if (functionName || functionSummary)
                            pInfo->functionName = Scr_PrevCodePosFunctionName(inst, pos);
                        else
                            pInfo->functionName = 0;
                        if (!functionName || pInfo->functionName && I_stristr(pInfo->functionName, functionName))
                        {
                            pInfo->pos = pos;
                            pInfo->fileName = Scr_PrevCodePosFileName(inst, pos);
                            pInfo->varUsage = 1;
                            ++num;
                        }
                    }
                }
            }
            if (total)
            {
                Com_Printf(0, "num vars:          %d\n", num);
                Z_VirtualFree(infoArray, 0);
            }
            else
            {
                if (summary)
                {
                    VariableInfoCompareCallBack = (int(__cdecl *)(const void *, const void *))VariableInfoFileNameCompare;
                    qsort(infoArray, num, 0x10u, (int(__cdecl *)(const void *, const void *))VariableInfoFileNameCompare);
                }
                else if (functionSummary)
                {
                    VariableInfoCompareCallBack = (int(__cdecl *)(const void *, const void *))VariableInfoFunctionCompare;
                    qsort(infoArray, num, 0x10u, (int(__cdecl *)(const void *, const void *))VariableInfoFunctionCompare);
                }
                else
                {
                    VariableInfoCompareCallBack = (int(__cdecl *)(const void *, const void *))CompareThreadIndices;
                    qsort(infoArray, num, 0x10u, (int(__cdecl *)(const void *, const void *))CompareThreadIndices);
                }
                i = 0;
                while (i < num)
                {
                    pInfoa = (VariableDebugInfo *)&infoArray[16 * i];
                    do
                    {
                        ++pInfoa->varUsage;
                        --*(_DWORD *)&infoArray[16 * i++ + 12];
                    } while (i < num && !VariableInfoCompareCallBack(pInfoa, &infoArray[16 * i]));
                }
                if (lineSort)
                    qsort(infoArray, num, 0x10u, (int(__cdecl *)(const void *, const void *))VariableInfoFileLineCompare);
                else
                    qsort(infoArray, num, 0x10u, (int(__cdecl *)(const void *, const void *))VariableInfoCountCompare);
                Com_Printf(24, "********************************\n");
                if (spreadsheet)
                {
                    if (summary)
                    {
                        Com_Printf(0, "count\tfile\n");
                    }
                    else if (functionSummary)
                    {
                        Com_Printf(0, "count\tfile\tfunction\n");
                    }
                    else
                    {
                        Com_Printf(0, "count\tfile\tline\tsource\tcol\n");
                    }
                }
                count = 0;
                filteredCount = 0;
                for (ia = 0; ia < num; ++ia)
                {
                    pInfob = (VariableDebugInfo *)&infoArray[16 * ia];
                    if (pInfob->varUsage)
                    {
                        count += pInfob->varUsage;
                        if (pInfob->varUsage >= minCount)
                        {
                            filteredCount += pInfob->varUsage;
                            if (spreadsheet)
                            {
                                Com_Printf(0, "%d\t", pInfob->varUsage);
                                Scr_PrintPrevCodePosSpreadSheet(inst, 0, (char *)pInfob->pos, summary, functionSummary);
                            }
                            else
                            {
                                if (summary
                                    && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                                        751,
                                        0,
                                        "%s",
                                        "!summary"))
                                {
                                    __debugbreak();
                                }
                                Com_Printf(0, "count: %d\n", pInfob->varUsage);
                                Scr_PrintPrevCodePos(inst, 0, (char *)pInfob->pos, 0);
                            }
                        }
                    }
                }
                if (num != count
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        758,
                        0,
                        "%s",
                        "num == count"))
                {
                    __debugbreak();
                }
                Com_Printf(0, "********************************\n");
                Com_Printf(0, "num vars:          %d\n", filteredCount);
                NumScriptVars = Scr_GetNumScriptVars(inst);
                Com_Printf(0, "num unlisted vars: %d\n", NumScriptVars - filteredCount);
                Com_Printf(0, "********************************\n");
                Z_VirtualFree(infoArray, 0);
            }
        }
        else
        {
            Com_Printf(24, "Cannot dump script variables: out of memory\n");
        }
    }
}

int __cdecl VariableInfoFileNameCompare(unsigned int *info1, unsigned int *info2)
{
    const char *fileName1; // [esp+0h] [ebp-8h]
    const char *fileName2; // [esp+4h] [ebp-4h]

    fileName1 = (const char *)info1[1];
    fileName2 = (const char *)info2[1];
    if ( !fileName1 )
        return 1;
    if ( fileName2 )
        return I_stricmp(fileName1, fileName2);
    return -1;
}

int __cdecl VariableInfoFunctionCompare(unsigned int *info1, unsigned int *info2)
{
    const char *functionName2; // [esp+0h] [ebp-Ch]
    const char *functionName1; // [esp+4h] [ebp-8h]
    int fileNameCompare; // [esp+8h] [ebp-4h]

    fileNameCompare = VariableInfoFileNameCompare(info1, info2);
    if ( fileNameCompare )
        return fileNameCompare;
    functionName1 = (const char *)info1[2];
    functionName2 = (const char *)info2[2];
    if ( !functionName1 )
        return 1;
    if ( functionName2 )
        return I_stricmp(functionName1, functionName2);
    return -1;
}

int __cdecl VariableInfoCountCompare(unsigned int *info1, unsigned int *info2)
{
    return info2[3] - info1[3];
}

int __cdecl VariableInfoFileLineCompare(unsigned int *info1, unsigned int *info2)
{
    int fileCompare; // [esp+0h] [ebp-4h]

    fileCompare = VariableInfoFileNameCompare(info1, info2);
    if ( fileCompare )
        return fileCompare;
    else
        return CompareThreadIndices(info1, info2);
}

void __cdecl Scr_DumpScriptVariablesDefault(scriptInstance_t inst)
{
    Scr_DumpScriptVariables(inst, 0, 0, 0, 0, 0, 0, 0, 0);
}

const char *gScriptVarsAlloc = "ScriptVars";

void __cdecl Scr_InitVariables(scriptInstance_t inst)
{
    gScrVarGlob[inst].variableList = (VariableValueInternal *)_PMem_AllocNamed(
        0x7DFFC8u,
        0x80u,
        4,
        1u,
        gScriptVarsAlloc,
        TRACK_SCRIPT);
    gScrVarPub[inst].saveIdMap = (unsigned __int16 *)_PMem_AllocNamed(0xFFFCu, 4u, 4, 1u, gScriptVarsAlloc, TRACK_SCRIPT);
    gScrVarPub[inst].saveIdMapRev = (unsigned __int16 *)_PMem_AllocNamed(
        0xFFFCu,
        4u,
        4,
        1u,
        gScriptVarsAlloc,
        TRACK_SCRIPT);
    if (!gScrVarDebugPub[inst])
        Scr_ResetScrVarDebugPub(inst);
    if (gScrVarDebugPub[inst])
    {
        gScrVarDebugPub[inst]->leakCount = (int *)_PMem_AllocNamed(0x11FFF8u, 4u, 4, 1u, gScriptVarsAlloc, TRACK_SCRIPT);
        memset(gScrVarDebugPub[inst]->leakCount, 0, 0x11FFF8u);
    }
    gScrVarPub[inst].totalObjectRefCount = 0;
    gScrVarPub[inst].totalVectorRefCount = 0;
    if (gScrVarDebugPub[inst])
    {
        gScrVarDebugPub[inst]->extRefCount = (unsigned __int16 *)_PMem_AllocNamed(
            0xFFFCu,
            4u,
            4,
            1u,
            gScriptVarsAlloc,
            TRACK_SCRIPT);
        memset(gScrVarDebugPub[inst]->extRefCount, 0, 0xFFFCu);
    }
    gScrVarPub[inst].numScriptValues = 0;
    gScrVarPub[inst].numScriptObjects = 0;
    if (gScrVarDebugPub[inst])
    {
        gScrVarDebugPub[inst]->varUsage = (const char **)_PMem_AllocNamed(
            0x11FFF8u,
            4u,
            4,
            1u,
            gScriptVarsAlloc,
            TRACK_SCRIPT);
        memset(gScrVarDebugPub[inst]->varUsage, 0, 0x11FFF8u);
    }
    Scr_InitVariableRange(inst, 1u, 0x7FFFu);
    Scr_InitVariableRange(inst, 0x8000u, 0x47FFEu);
}

void __cdecl Scr_InitVariableRange(scriptInstance_t inst, unsigned int begin, unsigned int end)
{
    unsigned int index; // [esp+4h] [ebp-8h]
    VariableValueInternal *value; // [esp+8h] [ebp-4h]
    VariableValueInternal *valuea; // [esp+8h] [ebp-4h]

    for ( index = begin + 1; index < end; ++index )
    {
        value = &gScrVarGlob[inst].variableList[index];
        value->w.status = 0;
        if ( (value->w.status & 0x1F) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        800,
                        0,
                        "%s",
                        "!(value->w.type & VAR_MASK)") )
        {
            __debugbreak();
        }
        value->hash.id = index - begin;
        value->v.next = index - begin;
        value->u.next = index - begin + 1;
        value->hash.u.prev = index - begin - 1;
    }
    valuea = &gScrVarGlob[inst].variableList[begin];
    valuea->w.status = 0;
    if ( (valuea->w.status & 0x1F) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    809,
                    0,
                    "%s",
                    "!(value->w.type & VAR_MASK)") )
    {
        __debugbreak();
    }
    valuea->w.status = valuea->w.status;
    valuea->hash.id = 0;
    valuea->v.next = 0;
    valuea->u.next = 1;
    gScrVarGlob[inst].variableList[begin + 1].hash.u.prev = 0;
    valuea->hash.u.prev = end - begin - 1;
    gScrVarGlob[inst].variableList[end - 1].u.next = 0;
}

void __cdecl Scr_InitClassMap(scriptInstance_t inst)
{
    int classnum; // [esp+0h] [ebp-4h]

    for ( classnum = 0; classnum < 5; ++classnum )
    {
        gScrClassMap[inst][classnum].entArrayId = 0;
        gScrClassMap[inst][classnum].id = 0;
    }
}

void __cdecl Scr_ShutdownVariables(scriptInstance_t inst)
{
    if (gScrVarPub[inst].gameId)
    {
        FreeValue(inst, gScrVarPub[inst].gameId);
        gScrVarPub[inst].gameId = 0;
    }
    if (!gScrStringDebugGlob[inst] || !gScrStringDebugGlob[inst]->ignoreLeaks)
    {
        if (gScrVarPub[inst].numScriptValues
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                927,
                0,
                "%s\n\t(gScrVarPub[inst].numScriptValues) = %i",
                "(!gScrVarPub[inst].numScriptValues)",
                gScrVarPub[inst].numScriptValues))
        {
            __debugbreak();
        }
        if (gScrVarPub[inst].numScriptObjects
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                928,
                0,
                "%s\n\t(gScrVarPub[inst].numScriptObjects) = %i",
                "(!gScrVarPub[inst].numScriptObjects)",
                gScrVarPub[inst].numScriptObjects))
        {
            __debugbreak();
        }
    }
    Scr_CheckLeaks(inst);
}

void __cdecl Scr_CheckLeaks(scriptInstance_t inst)
{
    bool bLeak; // [esp+3h] [ebp-5h]
    unsigned int id; // [esp+4h] [ebp-4h]
    unsigned int ida; // [esp+4h] [ebp-4h]

    if (!gScrStringDebugGlob[inst] || !gScrStringDebugGlob[inst]->ignoreLeaks)
    {
        Scr_CheckLeakRange(inst, 1u, 0x7FFFu);
        Scr_CheckLeakRange(inst, 0x8000u, 0x47FFEu);
        if (gScrVarPub[inst].totalObjectRefCount
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                172,
                0,
                "%s",
                "!gScrVarPub[inst].totalObjectRefCount"))
        {
            __debugbreak();
        }
        if (gScrVarPub[inst].totalVectorRefCount
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                173,
                0,
                "%s",
                "!gScrVarPub[inst].totalVectorRefCount"))
        {
            __debugbreak();
        }
        if (gScrVarPub[inst].ext_threadcount
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                174,
                0,
                "%s",
                "!gScrVarPub[inst].ext_threadcount"))
        {
            __debugbreak();
        }
    }
    if (gScrVarDebugPub[inst])
    {
        bLeak = 0;
        for (id = 0; id < 0x47FFE; ++id)
        {
            if (gScrVarDebugPub[inst]->leakCount[id])
                bLeak = 1;
        }
        if (bLeak)
        {
            Com_Printf(24, "leak:\n");
            for (ida = 0; ida < 0x47FFE; ++ida)
            {
                if (gScrVarDebugPub[inst]->leakCount[ida])
                    Com_Printf(24, "%d, %d\n", ida, gScrVarDebugPub[inst]->leakCount[ida]);
            }
            Com_Printf(24, "\n");
            if ((!gScrStringDebugGlob[inst] || !gScrStringDebugGlob[inst]->ignoreLeaks)
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp", 198, 0, "leak"))
            {
                __debugbreak();
            }
        }
        Scr_ClearScrVarDebugPub(inst);
    }
}

void __cdecl Scr_CheckLeakRange(scriptInstance_t inst, unsigned int begin, unsigned int end)
{
    VariableValueInternal *entry; // [esp+0h] [ebp-10h]
    int index; // [esp+8h] [ebp-8h]
    VariableValueInternal *value; // [esp+Ch] [ebp-4h]

    for ( index = 1; index < (int)(end - begin); ++index )
    {
        entry = &gScrVarGlob[inst].variableList[index + begin];
        value = &gScrVarGlob[inst].variableList[entry->hash.id + begin];
        switch ( value->w.status & 0x60 )
        {
            case ' ':
                Com_Printf(
                    24,
                    "move: %d -> %d\n",
                    entry->hash.id + begin,
                    gScrVarGlob[inst].variableList[value->v.next + begin].hash.id + begin);
LABEL_11:
                Com_Printf(
                    24,
                    "%d -> %d\n",
                    entry->hash.id + begin,
                    gScrVarGlob[inst].variableList[value->nextSibling + begin].hash.id);
                Com_Printf(
                    24,
                    "%d <- %d\n",
                    entry->hash.id + begin,
                    gScrVarGlob[inst].variableList[entry->hash.u.prev + begin].hash.id);
                continue;
            case '@':
                Com_Printf(
                    24,
                    "head: %d -> %d\n",
                    entry->hash.id + begin,
                    gScrVarGlob[inst].variableList[value->v.next + begin].hash.id + begin);
                goto LABEL_11;
            case '`':
                Com_Printf(
                    24,
                    "ext: %d %d\n",
                    entry->hash.id + begin,
                    gScrVarGlob[inst].variableList[value->v.next + begin].hash.id + begin);
                goto LABEL_11;
        }
    }
}

unsigned int __cdecl Scr_GetNumScriptVars(scriptInstance_t inst)
{
    return gScrVarPub[inst].numScriptObjects + gScrVarPub[inst].numScriptValues;
}

unsigned int __cdecl Scr_GetNumScriptVarsParent(scriptInstance_t inst)
{
    return gScrVarPub[inst].numScriptObjects;
}

unsigned int __cdecl Scr_GetNumScriptVarsChild(scriptInstance_t inst)
{
    return gScrVarPub[inst].numScriptValues;
}

unsigned int __cdecl GetVariableKeyObject(scriptInstance_t inst, unsigned int id)
{
    VariableValueInternal *v3; // [esp+0h] [ebp-4h]

    v3 = &gScrVarGlob[inst].variableList[id + 0x8000];
    if ( (v3->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (v3->w.status & 0x1F) >= 0xD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    967,
                    0,
                    "%s",
                    "!IsObject( &gScrVarGlob[inst].variableList[VARIABLELIST_CHILD_BEGIN(inst) + id] )") )
    {
        __debugbreak();
    }
    return (gScrVarGlob[inst].variableList[id + 0x8000].w.status >> 8) - 0x10000;
}

unsigned int __cdecl GetVariableIndexInternal(scriptInstance_t inst, unsigned int parentId, unsigned int name)
{
    VariableValueInternal *parentValue; // [esp+1Ch] [ebp-Ch]
    unsigned int newIndex; // [esp+20h] [ebp-8h]

    if ( !parentId
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1350,
                    0,
                    "%s",
                    "parentId") )
    {
        __debugbreak();
    }
    parentValue = &gScrVarGlob[inst].variableList[parentId + 1];
    if ( (parentValue->w.status & 0x60) != 0x60
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1352,
                    0,
                    "%s",
                    "(parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL") )
    {
        __debugbreak();
    }
    if ( (parentValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (parentValue->w.status & 0x1F) < 0xD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1353,
                    0,
                    "%s",
                    "IsObject( parentValue )") )
    {
        __debugbreak();
    }
    newIndex = FindVariableIndexInternal2(inst, name, (parentId + name) % 0x3FFFD + 1);
    if ( newIndex )
        return newIndex;
    else
        return GetNewVariableIndexInternal2(inst, parentId, name, (parentId + name) % 0x3FFFD + 1);
}

unsigned int __cdecl GetNewVariableIndexInternal2(
                scriptInstance_t inst,
                unsigned int parentId,
                unsigned int name,
                unsigned int index)
{
    unsigned int siblingId; // [esp+20h] [ebp-20h]
    VariableValueInternal *parentValue; // [esp+24h] [ebp-1Ch]
    VariableValueInternal *entry; // [esp+28h] [ebp-18h]
    VariableValueInternal *siblingValue; // [esp+2Ch] [ebp-14h]
    unsigned int siblingIndex; // [esp+30h] [ebp-10h]
    unsigned int id; // [esp+3Ch] [ebp-4h]
    unsigned int indexa; // [esp+54h] [ebp+14h]

    indexa = GetNewVariableIndexInternal3(inst, parentId, name, index);
    parentValue = &gScrVarGlob[inst].variableList[parentId + 1];
    if ( (parentValue->w.status & 0x60) != 0x60
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1224,
                    0,
                    "%s\n\t(parentValue->w.status & VAR_STAT_MASK) = %i",
                    "((parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL)",
                    parentValue->w.status & 0x60) )
    {
        __debugbreak();
    }
    entry = &gScrVarGlob[inst].variableList[indexa + 0x8000];
    id = entry->hash.id;
    siblingId = parentValue->nextSibling;
    if ( siblingId )
    {
        siblingValue = &gScrVarGlob[inst].variableList[siblingId + 0x8000];
        if ( (siblingValue->w.status & 0x60) == 0
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                        323,
                        0,
                        "%s",
                        "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
        {
            __debugbreak();
        }
        if ( (siblingValue->w.status & 0x1F) >= 0xD
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        1233,
                        0,
                        "%s",
                        "!IsObject( siblingValue )") )
        {
            __debugbreak();
        }
        siblingIndex = FindVariableIndexInternal(inst, parentId, siblingValue->w.status >> 8);
        if ( !siblingIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        1236,
                        0,
                        "%s",
                        "siblingIndex") )
        {
            __debugbreak();
        }
        gScrVarGlob[inst].variableList[siblingIndex + 0x8000].hash.u.prev = indexa;
    }
    else
    {
        siblingIndex = 0;
        gScrVarGlob[inst].variableList[parentValue->v.next + 1].hash.u.prev = id;
    }
    parentValue->nextSibling = id;
    entry->hash.u.prev = 0;
    gScrVarGlob[inst].variableList[id + 0x8000].nextSibling = siblingIndex;
    return indexa;
}

unsigned int __cdecl GetNewVariableIndexInternal3(
                scriptInstance_t inst,
                unsigned int parentId,
                unsigned int name,
                unsigned int index)
{
    const char *v4; // eax
    const char *v5; // eax
    VariableValueInternal *parentValue; // [esp+20h] [ebp-44h]
    VariableValueInternal *entry; // [esp+24h] [ebp-40h]
    unsigned int newIndex; // [esp+28h] [ebp-3Ch]
    unsigned int newIndexb; // [esp+28h] [ebp-3Ch]
    unsigned int newIndexa; // [esp+28h] [ebp-3Ch]
    unsigned int prevId; // [esp+2Ch] [ebp-38h]
    unsigned int nextSiblingIndex; // [esp+30h] [ebp-34h]
    unsigned int prevSiblingIndex; // [esp+34h] [ebp-30h]
    unsigned int next; // [esp+38h] [ebp-2Ch]
    unsigned int nexta; // [esp+38h] [ebp-2Ch]
    unsigned int nextb; // [esp+38h] [ebp-2Ch]
    unsigned int nextc; // [esp+38h] [ebp-2Ch]
    unsigned int nextd; // [esp+38h] [ebp-2Ch]
    VariableValueInternal *entryValue; // [esp+3Ch] [ebp-28h]
    unsigned int prev; // [esp+44h] [ebp-20h]
    unsigned int preva; // [esp+44h] [ebp-20h]
    unsigned int prevb; // [esp+44h] [ebp-20h]
    VariableValueInternal *newEntryValue; // [esp+48h] [ebp-1Ch]
    int type; // [esp+4Ch] [ebp-18h]
    VariableValueInternal *newEntrya; // [esp+50h] [ebp-14h]
    VariableValueInternal *newEntry; // [esp+50h] [ebp-14h]
    VariableValue value; // [esp+58h] [ebp-Ch]
    unsigned int id; // [esp+60h] [ebp-4h]

    if ( (name & 0xFF000000) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    992,
                    0,
                    "%s",
                    "!(name & ~VAR_NAME_LOW_MASK)") )
    {
        __debugbreak();
    }
    entry = &gScrVarGlob[inst].variableList[index + 0x8000];
    entryValue = &gScrVarGlob[inst].variableList[entry->hash.id + 0x8000];
    type = entryValue->w.status & 0x60;
    if ( !type )
    {
        newIndex = entry->v.next;
        next = entryValue->u.next;
        if ( newIndex == entry->hash.id || (entry->w.status & 0x60) != 0 )
        {
            newEntryValue = &gScrVarGlob[inst].variableList[entry->hash.id + 0x8000];
        }
        else
        {
            gScrVarGlob[inst].variableList[newIndex + 0x8000].hash.id = entry->hash.id;
            entry->hash.id = index;
            entryValue->v.next = newIndex;
            entryValue->u.next = entry->u.next;
            newEntryValue = entry;
        }
        prev = entry->hash.u.prev;
        if ( gScrVarGlob[inst].variableList[prev + 0x8000].hash.id )
        {
            if ( (gScrVarGlob[inst].variableList[gScrVarGlob[inst].variableList[prev + 0x8000].hash.id + 0x8000].w.status
                    & 0x60) != 0 )
            {
                v4 = va(
                             "%d, %d, %d",
                             prev,
                             gScrVarGlob[inst].variableList[prev + 0x8000].hash.id,
                             gScrVarGlob[inst].variableList[gScrVarGlob[inst].variableList[prev + 0x8000].hash.id + 0x8000].w.status
                         & 0x60);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                                1020,
                                0,
                                "%s\n\t%s",
                                "!gScrVarGlob[inst].variableList[begin + prev].hash.id || (gScrVarGlob[inst].variableList[begin+ gScrVarG"
                                "lob[inst].variableList[begin+ prev].hash.id].w.status & VAR_STAT_MASK) == VAR_STAT_FREE",
                                v4) )
                    __debugbreak();
            }
        }
        if ( gScrVarGlob[inst].variableList[next + 0x8000].hash.id )
        {
            if ( (gScrVarGlob[inst].variableList[gScrVarGlob[inst].variableList[next + 0x8000].hash.id + 0x8000].w.status
                    & 0x60) != 0 )
            {
                v5 = va(
                             "%d, %d, %d",
                             next,
                             gScrVarGlob[inst].variableList[next + 0x8000].hash.id,
                             gScrVarGlob[inst].variableList[gScrVarGlob[inst].variableList[next + 0x8000].hash.id + 0x8000].w.status
                         & 0x60);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                                1021,
                                0,
                                "%s\n\t%s",
                                "!gScrVarGlob[inst].variableList[begin + next].hash.id || (gScrVarGlob[inst].variableList[begin+ gScrVarG"
                                "lob[inst].variableList[begin+ next].hash.id].w.status & VAR_STAT_MASK) == VAR_STAT_FREE",
                                v5) )
                    __debugbreak();
            }
        }
        gScrVarGlob[inst].variableList[gScrVarGlob[inst].variableList[prev + 0x8000].hash.id + 0x8000].u.next = next;
        gScrVarGlob[inst].variableList[next + 0x8000].hash.u.prev = prev;
        newEntryValue->w.status = 64;
        newEntryValue->v.next = index;
        goto LABEL_66;
    }
    if ( type == 64 )
    {
        if ( (entry->w.status & 0x60) == 0 )
        {
            newIndexb = entry->v.next;
            newEntrya = &gScrVarGlob[inst].variableList[newIndexb + 0x8000];
            newEntryValue = &gScrVarGlob[inst].variableList[index + 0x8000];
            preva = newEntrya->hash.u.prev;
            nexta = entry->u.next;
            gScrVarGlob[inst].variableList[gScrVarGlob[inst].variableList[preva + 0x8000].hash.id + 0x8000].u.next = nexta;
            gScrVarGlob[inst].variableList[nexta + 0x8000].hash.u.prev = preva;
            newEntrya->hash.id = entry->hash.id;
            entry->hash.id = index;
            newEntrya->hash.u.prev = entry->hash.u.prev;
            gScrVarGlob[inst].variableList[gScrVarGlob[inst].variableList[newEntrya->hash.u.prev + 0x8000].hash.id + 0x8000].nextSibling = newIndexb;
            gScrVarGlob[inst].variableList[entryValue->nextSibling + 0x8000].hash.u.prev = newIndexb;
            entryValue->w.status &= 0xFFFFFF9F;
            entryValue->w.status |= 0x20u;
            entry->w.status = 64;
            goto LABEL_66;
        }
        index = gScrVarGlob[inst].variableList[0x8000].u.next;
        if ( index )
        {
            if ( index < 0x3FFFE )
            {
LABEL_26:
                entry = &gScrVarGlob[inst].variableList[index + 0x8000];
                newEntryValue = &gScrVarGlob[inst].variableList[entry->hash.id + 0x8000];
                if ( (newEntryValue->w.status & 0x60) != 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                                1067,
                                0,
                                "%s",
                                "(newEntryValue->w.status & VAR_STAT_MASK) == VAR_STAT_FREE") )
                {
                    __debugbreak();
                }
                nextb = newEntryValue->u.next;
                gScrVarGlob[inst].variableList[0x8000].u.next = nextb;
                gScrVarGlob[inst].variableList[nextb + 0x8000].hash.u.prev = 0;
                newEntryValue->w.status = 32;
                newEntryValue->v.next = entryValue->v.next;
                entryValue->v.next = index;
                goto LABEL_66;
            }
        }
        else
        {
            Scr_TerminalError("exceeded maximum number of script variables", inst);
        }
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        1063,
                        0,
                        "%s\n\t(index) = %i",
                        "(index > 0 && index < ((inst==SCRIPTINSTANCE_CLIENT)?(unsigned)CLIENT_VARIABLELIST_CHILD_SIZE:(unsigned)SERV"
                        "ER_VARIABLELIST_CHILD_SIZE))",
                        index) )
            __debugbreak();
        goto LABEL_26;
    }
    if ( type != 32
        && type != 96
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1081,
                    0,
                    "%s",
                    "type == VAR_STAT_MOVABLE || type == VAR_STAT_EXTERNAL") )
    {
        __debugbreak();
    }
    if ( (entry->w.status & 0x60) != 0 )
    {
        newIndexa = gScrVarGlob[inst].variableList[0x8000].u.next;
        if ( !newIndexa )
            Scr_TerminalError("exceeded maximum number of script variables", inst);
        newEntry = &gScrVarGlob[inst].variableList[newIndexa + 0x8000];
        newEntryValue = &gScrVarGlob[inst].variableList[newEntry->hash.id + 0x8000];
        if ( (newEntryValue->w.status & 0x60) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        1107,
                        0,
                        "%s",
                        "(newEntryValue->w.status & VAR_STAT_MASK) == VAR_STAT_FREE") )
        {
            __debugbreak();
        }
        nextd = newEntryValue->u.next;
        gScrVarGlob[inst].variableList[0x8000].u.next = nextd;
        gScrVarGlob[inst].variableList[nextd + 0x8000].hash.u.prev = 0;
    }
    else
    {
        if ( entry == entryValue
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        1086,
                        0,
                        "%s",
                        "entry != entryValue") )
        {
            __debugbreak();
        }
        newIndexa = entry->v.next;
        newEntry = &gScrVarGlob[inst].variableList[newIndexa + 0x8000];
        newEntryValue = entry;
        prevb = newEntry->hash.u.prev;
        nextc = entry->u.next;
        gScrVarGlob[inst].variableList[gScrVarGlob[inst].variableList[prevb + 0x8000].hash.id + 0x8000].u.next = nextc;
        gScrVarGlob[inst].variableList[nextc + 0x8000].hash.u.prev = prevb;
    }
    prevSiblingIndex = entry->hash.u.prev;
    if ( prevSiblingIndex )
        gScrVarGlob[inst].variableList[gScrVarGlob[inst].variableList[prevSiblingIndex + 0x8000].hash.id + 0x8000].nextSibling = newIndexa;
    nextSiblingIndex = entryValue->nextSibling;
    if ( nextSiblingIndex )
        gScrVarGlob[inst].variableList[nextSiblingIndex + 0x8000].hash.u.prev = newIndexa;
    if ( type == 32 )
    {
        prevId = gScrVarGlob[inst].variableList[entryValue->v.next + 0x8000].hash.id;
        if ( (gScrVarGlob[inst].variableList[prevId + 0x8000].w.status & 0x60) != 0x20
            && (gScrVarGlob[inst].variableList[prevId + 0x8000].w.status & 0x60) != 0x40
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        1135,
                        0,
                        "%s",
                        "(gScrVarGlob[inst].variableList[begin + prevId].w.status & VAR_STAT_MASK) == VAR_STAT_MOVABLE || (gScrVarGlo"
                        "b[inst].variableList[begin + prevId].w.status & VAR_STAT_MASK) == VAR_STAT_HEAD") )
        {
            __debugbreak();
        }
        while ( gScrVarGlob[inst].variableList[prevId + 0x8000].v.next != index )
        {
            prevId = gScrVarGlob[inst].variableList[gScrVarGlob[inst].variableList[prevId + 0x8000].v.next + 0x8000].hash.id;
            if ( (gScrVarGlob[inst].variableList[prevId + 0x8000].w.status & 0x60) != 0x20
                && (gScrVarGlob[inst].variableList[prevId + 0x8000].w.status & 0x60) != 0x40
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                            1140,
                            0,
                            "%s",
                            "(gScrVarGlob[inst].variableList[begin + prevId].w.status & VAR_STAT_MASK) == VAR_STAT_MOVABLE || (gScrVarG"
                            "lob[inst].variableList[begin + prevId].w.status & VAR_STAT_MASK) == VAR_STAT_HEAD") )
            {
                __debugbreak();
            }
        }
        gScrVarGlob[inst].variableList[prevId + 0x8000].v.next = newIndexa;
    }
    else
    {
        if ( type != 96
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        1148,
                        0,
                        "%s",
                        "type == VAR_STAT_EXTERNAL") )
        {
            __debugbreak();
        }
        entryValue->v.next = newIndexa;
    }
    newEntry->hash.u.prev = entry->hash.u.prev;
    id = newEntry->hash.id;
    newEntry->hash.id = entry->hash.id;
    entry->hash.id = id;
    newEntryValue->w.status = 64;
    newEntryValue->v.next = index;
LABEL_66:
    if ( entry != &gScrVarGlob[inst].variableList[index + 0x8000]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1166,
                    0,
                    "%s",
                    "entry == &gScrVarGlob[inst].variableList[begin + index]") )
    {
        __debugbreak();
    }
    if ( newEntryValue != &gScrVarGlob[inst].variableList[entry->hash.id + 0x8000]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1167,
                    0,
                    "%s",
                    "newEntryValue == &gScrVarGlob[inst].variableList[begin + entry->hash.id]") )
    {
        __debugbreak();
    }
    if ( (newEntryValue->w.status & 0x1F) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1168,
                    0,
                    "%s",
                    "(newEntryValue->w.type & VAR_MASK) == VAR_UNDEFINED") )
    {
        __debugbreak();
    }
    newEntryValue->w.status = (unsigned __int8)newEntryValue->w.status;
    newEntryValue->w.status |= name << 8;
    if ( (!entry->hash.id || entry->hash.id >= 0x3FFFE)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1173,
                    0,
                    "%s\n\t(entry->hash.id) = %i",
                    "(entry->hash.id > 0 && entry->hash.id < ((inst==SCRIPTINSTANCE_CLIENT)?(unsigned)CLIENT_VARIABLELIST_CHILD_SIZ"
                    "E:(unsigned)SERVER_VARIABLELIST_CHILD_SIZE))",
                    entry->hash.id) )
    {
        __debugbreak();
    }
    ++gScrVarPub[inst].totalObjectRefCount;
    if ( gScrVarDebugPub[inst] )
    {
        if ( gScrVarDebugPub[inst]->leakCount[entry->hash.id + 0x8000]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        1178,
                        0,
                        "%s",
                        "!gScrVarDebugPub[inst]->leakCount[begin + entry->hash.id]") )
        {
            __debugbreak();
        }
        ++gScrVarDebugPub[inst]->leakCount[entry->hash.id + 0x8000];
    }
    ++gScrVarPub[inst].numScriptValues;
    if (!gScrVarPub[inst].varUsagePos
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1185,
            0,
            "%s",
            "gScrVarPub[inst].varUsagePos"))
    {
        __debugbreak();
    }
    if ( gScrVarDebugPub[inst] )
    {
        if ( gScrVarDebugPub[inst]->varUsage[entry->hash.id + 0x8000]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        1188,
                        0,
                        "%s",
                        "!gScrVarDebugPub[inst]->varUsage[begin + entry->hash.id]") )
        {
            __debugbreak();
        }
        gScrVarDebugPub[inst]->varUsage[entry->hash.id + 0x8000] = gScrVarPub[inst].varUsagePos;
    }
    parentValue = &gScrVarGlob[inst].variableList[parentId + 1];
    if ( (parentValue->w.status & 0x60) != 0x60
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1194,
                    0,
                    "%s",
                    "(parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL") )
    {
        __debugbreak();
    }
    if ( (parentValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (parentValue->w.status & 0x1F) < 0xD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1195,
                    0,
                    "%s",
                    "IsObject( parentValue )") )
    {
        __debugbreak();
    }
    if ( (parentValue->w.status & 0x1F) == 0x14 )
    {
        ++parentValue->u.o.u.entnum;
        value = Scr_GetArrayIndexValue(inst, name);
        AddRefToValue(inst, value.type, value.u);
    }
    return index;
}

void __cdecl ClearObject(scriptInstance_t inst, unsigned int parentId)
{
    if ( (gScrVarGlob[inst].variableList[parentId + 1].w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1590,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst) + parentId].w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    AddRefToObject(inst, parentId);
    ClearObjectInternal(inst, parentId);
    RemoveRefToEmptyObject(inst, parentId);
}

void __cdecl ClearObjectInternal(scriptInstance_t inst, unsigned int parentId)
{
    unsigned int nextId; // [esp+20h] [ebp-18h]
    unsigned int nextSibling; // [esp+24h] [ebp-14h]
    unsigned int nextSiblinga; // [esp+24h] [ebp-14h]
    VariableValueInternal *parentValue; // [esp+28h] [ebp-10h]
    VariableValueInternal *entryValue; // [esp+30h] [ebp-8h]
    unsigned int id; // [esp+34h] [ebp-4h]
    unsigned int ida; // [esp+34h] [ebp-4h]

    parentValue = &gScrVarGlob[inst].variableList[parentId + 1];
    if ( (parentValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (parentValue->w.status & 0x1F) < 0xD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1557,
                    0,
                    "%s",
                    "IsObject( parentValue )") )
    {
        __debugbreak();
    }
    if ( parentValue->nextSibling )
    {
        entryValue = &gScrVarGlob[inst].variableList[parentValue->nextSibling + 0x8000];
        if ( (entryValue->w.status & 0x60) == 0
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                        323,
                        0,
                        "%s",
                        "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
        {
            __debugbreak();
        }
        if ( (entryValue->w.status & 0x1F) >= 0xD
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        1563,
                        0,
                        "%s",
                        "!IsObject( entryValue )") )
        {
            __debugbreak();
        }
        nextSibling = FindVariableIndexInternal(inst, parentId, entryValue->w.status >> 8);
        if ( !nextSibling
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        1567,
                        0,
                        "%s",
                        "nextSibling") )
        {
            __debugbreak();
        }
        do
        {
            id = gScrVarGlob[inst].variableList[nextSibling + 0x8000].hash.id;
            MakeVariableExternal(inst, nextSibling, parentValue);
            nextSibling = gScrVarGlob[inst].variableList[id + 0x8000].nextSibling;
        }
        while ( nextSibling );
        nextId = parentValue->nextSibling;
        do
        {
            ida = nextId;
            nextSiblinga = gScrVarGlob[inst].variableList[nextId + 0x8000].nextSibling;
            nextId = gScrVarGlob[inst].variableList[nextSiblinga + 0x8000].hash.id;
            FreeChildValue(inst, parentId, ida);
        }
        while ( nextSiblinga );
    }
}

void __cdecl MakeVariableExternal(scriptInstance_t inst, unsigned int index, VariableValueInternal *parentValue)
{
    Variable_u v3; // edx
    VariableValueInternal *entry; // [esp+1Ch] [ebp-3Ch]
    unsigned int oldPrevSiblingIndex; // [esp+20h] [ebp-38h]
    unsigned int nextSiblingIndex; // [esp+24h] [ebp-34h]
    unsigned int prevSiblingIndex; // [esp+28h] [ebp-30h]
    unsigned int oldIndex; // [esp+30h] [ebp-28h]
    VariableValueInternal *entryValue; // [esp+34h] [ebp-24h]
    unsigned int tempEntry; // [esp+38h] [ebp-20h]
    Variable_u tempEntry_4; // [esp+3Ch] [ebp-1Ch]
    VariableValueInternal *oldEntry; // [esp+40h] [ebp-18h]
    VariableValueInternal *oldEntrya; // [esp+40h] [ebp-18h]
    Variable *prev; // [esp+44h] [ebp-14h]
    unsigned int oldNextSiblingIndex; // [esp+48h] [ebp-10h]
    VariableValue value; // [esp+4Ch] [ebp-Ch]
    VariableValueInternal *oldEntryValue; // [esp+54h] [ebp-4h]
    VariableValueInternal *oldEntryValuea; // [esp+54h] [ebp-4h]

    entry = &gScrVarGlob[inst].variableList[index + 0x8000];
    entryValue = &gScrVarGlob[inst].variableList[entry->hash.id + 0x8000];
    if ( (entryValue->w.status & 0x60) != 0x20
        && (entryValue->w.status & 0x60) != 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1389,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_MOVABLE || (entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_HEAD") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) >= 0xD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1390,
                    0,
                    "%s",
                    "!IsObject( entryValue )") )
    {
        __debugbreak();
    }
    if ( (parentValue->w.status & 0x1F) == 0x14 )
    {
        --parentValue->u.o.u.entnum;
        if ( (entryValue->w.status & 0x60) == 0
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                        323,
                        0,
                        "%s",
                        "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
        {
            __debugbreak();
        }
        if ( (entryValue->w.status & 0x1F) >= 0xD
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        1395,
                        0,
                        "%s",
                        "!IsObject( entryValue )") )
        {
            __debugbreak();
        }
        value = Scr_GetArrayIndexValue(inst, entryValue->w.status >> 8);
        RemoveRefToValue(inst, value.type, value.u);
    }
    if ( (entryValue->w.status & 0x60) == 0x40 )
    {
        oldIndex = entryValue->v.next;
        oldEntry = &gScrVarGlob[inst].variableList[oldIndex + 0x8000];
        oldEntryValue = &gScrVarGlob[inst].variableList[oldEntry->hash.id + 0x8000];
        if ( oldEntry != entry )
        {
            if ( (oldEntryValue->w.status & 0x60) != 0x20
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                            1409,
                            0,
                            "%s",
                            "(oldEntryValue->w.status & VAR_STAT_MASK) == VAR_STAT_MOVABLE") )
            {
                __debugbreak();
            }
            oldEntryValue->w.status &= 0xFFFFFF9F;
            oldEntryValue->w.status |= 0x40u;
            prevSiblingIndex = entry->hash.u.prev;
            nextSiblingIndex = entryValue->nextSibling;
            oldPrevSiblingIndex = oldEntry->hash.u.prev;
            oldNextSiblingIndex = oldEntryValue->nextSibling;
            if ( oldNextSiblingIndex )
                gScrVarGlob[inst].variableList[oldNextSiblingIndex + 0x8000].hash.u.prev = index;
            if ( oldPrevSiblingIndex )
                gScrVarGlob[inst].variableList[gScrVarGlob[inst].variableList[oldPrevSiblingIndex + 0x8000].hash.id + 0x8000].nextSibling = index;
            if ( nextSiblingIndex )
                gScrVarGlob[inst].variableList[nextSiblingIndex + 0x8000].hash.u.prev = oldIndex;
            if ( prevSiblingIndex )
                gScrVarGlob[inst].variableList[gScrVarGlob[inst].variableList[prevSiblingIndex + 0x8000].hash.id + 0x8000].nextSibling = oldIndex;
            tempEntry = entry->hash.id;
            tempEntry_4.prev = entry->hash.u.prev;
            v3.prev = oldEntry->hash.u.prev;
            entry->hash.id = oldEntry->hash.id;
            entry->hash.u = v3;
            oldEntry->hash.id = tempEntry;
            oldEntry->hash.u = tempEntry_4;
            entry = oldEntry;
            index = oldIndex;
        }
    }
    else
    {
        oldEntrya = entry;
        oldEntryValuea = entryValue;
        do
        {
            if ( (oldEntryValuea->w.status & 0x60) != 0x20
                && (oldEntryValuea->w.status & 0x60) != 0x40
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                            1442,
                            0,
                            "%s",
                            "(oldEntryValue->w.status & VAR_STAT_MASK) == VAR_STAT_MOVABLE || (oldEntryValue->w.status & VAR_STAT_MASK)"
                            " == VAR_STAT_HEAD") )
            {
                __debugbreak();
            }
            prev = &oldEntrya->hash;
            oldEntrya = &gScrVarGlob[inst].variableList[oldEntryValuea->v.next + 0x8000];
            oldEntryValuea = &gScrVarGlob[inst].variableList[oldEntrya->hash.id + 0x8000];
        }
        while ( oldEntrya != entry );
        gScrVarGlob[inst].variableList[prev->id + 0x8000].v.next = entryValue->v.next;
    }
    if ( entryValue != &gScrVarGlob[inst].variableList[entry->hash.id + 0x8000]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1452,
                    0,
                    "%s",
                    "entryValue == &gScrVarGlob[inst].variableList[VARIABLELIST_CHILD_BEGIN(inst)+ entry->hash.id]") )
    {
        __debugbreak();
    }
    entryValue->w.status &= 0xFFFFFF9F;
    entryValue->w.status |= 0x60u;
    entryValue->v.next = index;
}

void __cdecl FreeChildValue(scriptInstance_t inst, unsigned int parentId, unsigned int id)
{
    VariableValueInternal *entry; // [esp+Ch] [ebp-20h]
    unsigned int nextSiblingIndex; // [esp+10h] [ebp-1Ch]
    unsigned int prevSiblingIndex; // [esp+14h] [ebp-18h]
    unsigned int parentIndex; // [esp+18h] [ebp-14h]
    VariableValueInternal *entryValue; // [esp+1Ch] [ebp-10h]
    unsigned int index; // [esp+20h] [ebp-Ch]

    entryValue = &gScrVarGlob[inst].variableList[id + 0x8000];
    if ((entryValue->w.status & 0x60) != 0x60
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1475,
            0,
            "%s\n\t(entryValue->w.status & VAR_STAT_MASK) = %i",
            "((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL)",
            entryValue->w.status & 0x60))
    {
        __debugbreak();
    }
    if ((entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
            323,
            0,
            "%s",
            "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE"))
    {
        __debugbreak();
    }
    if ((entryValue->w.status & 0x1F) >= 0xD
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1476,
            0,
            "%s",
            "!IsObject( entryValue )"))
    {
        __debugbreak();
    }
    if (gScrVarGlob[inst].variableList[entryValue->v.next + 0x8000].hash.id != id
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1477,
            0,
            "%s",
            "gScrVarGlob[inst].variableList[VARIABLELIST_CHILD_BEGIN(inst)+ entryValue->v.index].hash.id == id"))
    {
        __debugbreak();
    }
    RemoveRefToValue(inst, entryValue->w.status & 0x1F, entryValue->u.u);
    if ((!id || id >= 0x3FFFE)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1483,
            0,
            "%s",
            "id > 0 && id < VARIABLELIST_CHILD_SIZE(inst)"))
    {
        __debugbreak();
    }
    --gScrVarPub[inst].totalObjectRefCount;
    if (gScrVarDebugPub[inst])
    {
        if (!gScrVarDebugPub[inst]->leakCount[id + 0x8000]
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                1488,
                0,
                "%s",
                "gScrVarDebugPub[inst]->leakCount[VARIABLELIST_CHILD_BEGIN(inst)+ id]"))
        {
            __debugbreak();
        }
        if (--gScrVarDebugPub[inst]->leakCount[id + 0x8000]
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                1490,
                0,
                "%s",
                "!gScrVarDebugPub[inst]->leakCount[VARIABLELIST_CHILD_BEGIN(inst)+ id]"))
        {
            __debugbreak();
        }
    }
    --gScrVarPub[inst].numScriptValues;
    if (gScrVarDebugPub[inst])
    {
        if (!gScrVarDebugPub[inst]->varUsage[id + 0x8000]
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                1498,
                0,
                "%s",
                "gScrVarDebugPub[inst]->varUsage[VARIABLELIST_CHILD_BEGIN(inst)+ id]"))
        {
            __debugbreak();
        }
        gScrVarDebugPub[inst]->varUsage[id + 0x8000] = 0;
    }
    if ((entryValue->w.status & 0x60) != 0x60
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1503,
            0,
            "%s\n\t(entryValue->w.status & VAR_STAT_MASK) = %i",
            "((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL)",
            entryValue->w.status & 0x60))
    {
        __debugbreak();
    }
    index = entryValue->v.next;
    entry = &gScrVarGlob[inst].variableList[index + 0x8000];
    if (entry->hash.id != id
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1507,
            0,
            "%s",
            "entry->id == id"))
    {
        __debugbreak();
    }
    nextSiblingIndex = entryValue->nextSibling;
    prevSiblingIndex = entry->hash.u.prev;
    if (prevSiblingIndex)
    {
        gScrVarGlob[inst].variableList[gScrVarGlob[inst].variableList[prevSiblingIndex + 0x8000].hash.id + 0x8000].nextSibling = nextSiblingIndex;
    }
    else
    {
        if (gScrVarGlob[inst].variableList[0x8000].hash.id
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                1519,
                0,
                "%s",
                "!gScrVarGlob[inst].variableList[VARIABLELIST_CHILD_BEGIN(inst)].hash.id"))
        {
            __debugbreak();
        }
        gScrVarGlob[inst].variableList[parentId + 1].nextSibling = gScrVarGlob[inst].variableList[nextSiblingIndex + 0x8000].hash.id;
    }
    if (nextSiblingIndex)
    {
        gScrVarGlob[inst].variableList[nextSiblingIndex + 0x8000].hash.u.prev = prevSiblingIndex;
    }
    else
    {
        parentIndex = gScrVarGlob[inst].variableList[parentId + 1].v.next;
        if (gScrVarGlob[inst].variableList[0x8000].hash.id
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                1531,
                0,
                "%s",
                "!gScrVarGlob[inst].variableList[VARIABLELIST_CHILD_BEGIN(inst)].hash.id"))
        {
            __debugbreak();
        }
        gScrVarGlob[inst].variableList[parentIndex + 1].hash.u.prev = gScrVarGlob[inst].variableList[prevSiblingIndex
            + 0x8000].hash.id;
    }
    entryValue->w.status = 0;
    entryValue->u.next = gScrVarGlob[inst].variableList[0x8000].u.next;
    entry->hash.u.prev = 0;
    gScrVarGlob[inst].variableList[gScrVarGlob[inst].variableList[0x8000].u.next + 0x8000].hash.u.prev = index;
    gScrVarGlob[inst].variableList[0x8000].u.next = index;
}

void __cdecl Scr_SetThreadNotifyName(scriptInstance_t inst, unsigned int startLocalId, unsigned int stringValue)
{
    VariableValueInternal *entryValue; // [esp+0h] [ebp-4h]

    entryValue = &gScrVarGlob[inst].variableList[startLocalId + 1];
    if ( (entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1607,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) != 0xD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1608,
                    0,
                    "%s\n\t(entryValue->w.type) = %i",
                    "((entryValue->w.type & VAR_MASK) == VAR_THREAD)",
                    entryValue->w.status) )
    {
        __debugbreak();
    }
    entryValue->w.status &= 0xFFFFFFE0;
    entryValue->w.status = (unsigned __int8)entryValue->w.status;
    entryValue->w.status |= 0xEu;
    entryValue->w.status |= stringValue << 8;
}

void __cdecl Scr_StopThread(scriptInstance_t inst, unsigned int threadId)
{
    if (!threadId
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1636,
            0,
            "%s",
            "threadId"))
    {
        __debugbreak();
    }
    Scr_ClearThread(inst, threadId);
    gScrVarGlob[inst].variableList[threadId + 1].u.o.u.nextEntId = gScrVarPub[inst].levelId;
    AddRefToObject(inst, gScrVarPub[inst].levelId);
}

void __cdecl Scr_ClearThread(scriptInstance_t inst, unsigned int parentId)
{
    VariableValueInternal *parentValue; // [esp+0h] [ebp-4h]

    if ( !parentId
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1620,
                    0,
                    "%s",
                    "parentId") )
    {
        __debugbreak();
    }
    parentValue = &gScrVarGlob[inst].variableList[parentId + 1];
    if ( (parentValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1623,
                    0,
                    "%s",
                    "(parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( ((parentValue->w.status & 0x1F) < 0xD || (parentValue->w.status & 0x1F) > 0x10)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1624,
                    0,
                    "%s",
                    "((parentValue->w.type & VAR_MASK) >= VAR_THREAD) && ((parentValue->w.type & VAR_MASK) <= VAR_CHILD_THREAD)") )
    {
        __debugbreak();
    }
    if ( FindVariable(inst, parentId, 0x17FFFu)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1625,
                    0,
                    "%s",
                    "!FindVariable( inst, parentId, OBJECT_STACK )") )
    {
        __debugbreak();
    }
    if ( parentValue->nextSibling )
        ClearObjectInternal(inst, parentId);
    RemoveRefToObject(inst, parentValue->u.o.u.nextEntId);
}

void __cdecl Scr_RemoveThreadNotifyName(scriptInstance_t inst, unsigned int startLocalId)
{
    unsigned __int16 stringValue; // [esp+0h] [ebp-8h]
    VariableValueInternal *entryValue; // [esp+4h] [ebp-4h]

    entryValue = &gScrVarGlob[inst].variableList[startLocalId + 1];
    if ( (entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1650,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) != 0xE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1651,
                    0,
                    "%s",
                    "(entryValue->w.type & VAR_MASK) == VAR_NOTIFY_THREAD") )
    {
        __debugbreak();
    }
    stringValue = Scr_GetThreadNotifyName(inst, startLocalId);
    if ( !stringValue
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1653,
                    0,
                    "%s",
                    "stringValue") )
    {
        __debugbreak();
    }
    SL_RemoveRefToString(inst, stringValue);
    entryValue->w.status &= 0xFFFFFFE0;
    entryValue->w.status |= 0xDu;
}

unsigned int __cdecl Scr_GetThreadNotifyName(scriptInstance_t inst, unsigned int startLocalId)
{
    if ( (gScrVarGlob[inst].variableList[startLocalId + 1].w.status & 0x60) != 0x60
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1676,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst) + startLocalId].w.type & VAR_STAT_MASK) == VAR_STAT_EXTERNAL") )
    {
        __debugbreak();
    }
    if ( (gScrVarGlob[inst].variableList[startLocalId + 1].w.status & 0x1F) != 0xE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1677,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst) + startLocalId].w.type & VAR_MASK) == VAR_NOTIFY_THREAD") )
    {
        __debugbreak();
    }
    if ( gScrVarGlob[inst].variableList[startLocalId + 1].w.status >> 8 >= 0x10000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1678,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst) + startLocalId].w.notifyName >> VAR_NAME_BITS) < ( 1 << 16 )") )
    {
        __debugbreak();
    }
    return gScrVarGlob[inst].variableList[startLocalId + 1].w.status >> 8;
}

void __cdecl Scr_SetThreadWaitTime(scriptInstance_t inst, unsigned int startLocalId, unsigned int waitTime)
{
    VariableValueInternal *entryValue; // [esp+0h] [ebp-4h]

    entryValue = &gScrVarGlob[inst].variableList[startLocalId + 1];
    if ( (entryValue->w.status & 0x60) != 0x60
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1688,
                    0,
                    "%s\n\t(entryValue->w.status & VAR_STAT_MASK) = %i",
                    "((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL)",
                    entryValue->w.status & 0x60) )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) != 0xD
        && (unsigned __int16)Scr_GetThreadNotifyName(inst, startLocalId)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1689,
                    0,
                    "%s",
                    "((entryValue->w.type & VAR_MASK) == VAR_THREAD) || !Scr_GetThreadNotifyName( inst, startLocalId )") )
    {
        __debugbreak();
    }
    entryValue->w.status &= 0xFFFFFFE0;
    entryValue->w.status = (unsigned __int8)entryValue->w.status;
    entryValue->w.status |= 0xFu;
    gScrVarGlob[inst].variableList[startLocalId + 1].w.status |= waitTime << 8;
}

void __cdecl Scr_ClearWaitTime(scriptInstance_t inst, unsigned int startLocalId)
{
    VariableValueInternal *entryValue; // [esp+0h] [ebp-4h]

    entryValue = &gScrVarGlob[inst].variableList[startLocalId + 1];
    if ( (entryValue->w.status & 0x60) != 0x60
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1703,
                    0,
                    "%s\n\t(entryValue->w.status & VAR_STAT_MASK) = %i",
                    "((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL)",
                    entryValue->w.status & 0x60) )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) != 0xF
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1704,
                    0,
                    "%s",
                    "(entryValue->w.type & VAR_MASK) == VAR_TIME_THREAD") )
    {
        __debugbreak();
    }
    entryValue->w.status &= 0xFFFFFFE0;
    entryValue->w.status |= 0xDu;
}

unsigned int __cdecl Scr_GetThreadWaitTime(scriptInstance_t inst, unsigned int startLocalId)
{
    if ( (gScrVarGlob[inst].variableList[startLocalId + 1].w.status & 0x60) != 0x60
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1712,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst) + startLocalId].w.status & VAR_STAT_MASK) == V"
                    "AR_STAT_EXTERNAL") )
    {
        __debugbreak();
    }
    if ( (gScrVarGlob[inst].variableList[startLocalId + 1].w.status & 0x1F) != 0xF
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1713,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst) + startLocalId].w.type & VAR_MASK) == VAR_TIME_THREAD") )
    {
        __debugbreak();
    }
    return gScrVarGlob[inst].variableList[startLocalId + 1].w.status >> 8;
}

unsigned int __cdecl GetParentLocalId(scriptInstance_t inst, unsigned int threadId)
{
    if ( (gScrVarGlob[inst].variableList[threadId + 1].w.status & 0x60) != 0x60
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1720,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ threadId].w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL") )
    {
        __debugbreak();
    }
    if ( (gScrVarGlob[inst].variableList[threadId + 1].w.status & 0x1F) != 0x10
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1721,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ threadId].w.type & VAR_MASK) == VAR_CHILD_THREAD") )
    {
        __debugbreak();
    }
    return gScrVarGlob[inst].variableList[threadId + 1].w.status >> 8;
}

unsigned int __cdecl GetSafeParentLocalId(scriptInstance_t inst, unsigned int threadId)
{
    if ( (gScrVarGlob[inst].variableList[threadId + 1].w.status & 0x60) == 0 )
        return 0;
    if ( (gScrVarGlob[inst].variableList[threadId + 1].w.status & 0x60) != 0x60
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1731,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ threadId].w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL") )
    {
        __debugbreak();
    }
    if ( ((gScrVarGlob[inst].variableList[threadId + 1].w.status & 0x1F) < 0xD
         || (gScrVarGlob[inst].variableList[threadId + 1].w.status & 0x1F) > 0x10)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1732,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ threadId].w.type & VAR_MASK) >= VAR_THREAD &&"
                    " (gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ threadId].w.type & VAR_MASK) <= VAR_CHILD_THREAD") )
    {
        __debugbreak();
    }
    if ( (gScrVarGlob[inst].variableList[threadId + 1].w.status & 0x1F) == 0x10 )
        return gScrVarGlob[inst].variableList[threadId + 1].w.status >> 8;
    else
        return 0;
}

unsigned int __cdecl GetStartLocalId(scriptInstance_t inst, unsigned int threadId)
{
    if ( (gScrVarGlob[inst].variableList[threadId + 1].w.status & 0x60) != 0x60
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1739,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ threadId].w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL") )
    {
        __debugbreak();
    }
    if ( ((gScrVarGlob[inst].variableList[threadId + 1].w.status & 0x1F) < 0xD
         || (gScrVarGlob[inst].variableList[threadId + 1].w.status & 0x1F) > 0x10)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1740,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ threadId].w.type & VAR_MASK) >= VAR_THREAD &&"
                    " (gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ threadId].w.type & VAR_MASK) <= VAR_CHILD_THREAD") )
    {
        __debugbreak();
    }
    while ( (gScrVarGlob[inst].variableList[threadId + 1].w.status & 0x1F) == 0x10 )
        threadId = gScrVarGlob[inst].variableList[threadId + 1].w.status >> 8;
    if ( (gScrVarGlob[inst].variableList[threadId + 1].w.status & 0x60) != 0x60
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1745,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ threadId].w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL") )
    {
        __debugbreak();
    }
    if ( ((gScrVarGlob[inst].variableList[threadId + 1].w.status & 0x1F) < 0xD
         || (gScrVarGlob[inst].variableList[threadId + 1].w.status & 0x1F) > 0xF)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1746,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ threadId].w.type & VAR_MASK) >= VAR_THREAD &&"
                    " (gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ threadId].w.type & VAR_MASK) <= VAR_TIME_THREAD") )
    {
        __debugbreak();
    }
    return threadId;
}

void __cdecl Scr_KillThread(scriptInstance_t inst, unsigned int parentId)
{
    unsigned int ObjectVariable; // eax
    VariableValueInternal_u *VariableValueAddress; // eax
    VariableValueInternal *parentValue; // [esp+4h] [ebp-18h]
    unsigned int selfNameId; // [esp+8h] [ebp-14h]
    unsigned int name; // [esp+Ch] [ebp-10h]
    unsigned int id; // [esp+14h] [ebp-8h]
    unsigned int notifyListEntry; // [esp+18h] [ebp-4h]

    if (!parentId
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1760,
            0,
            "%s",
            "parentId"))
    {
        __debugbreak();
    }
    parentValue = &gScrVarGlob[inst].variableList[parentId + 1];
    if ((parentValue->w.status & 0x60) != 0x60
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1763,
            0,
            "%s",
            "(parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL"))
    {
        __debugbreak();
    }
    if (((parentValue->w.status & 0x1F) < 0xD || (parentValue->w.status & 0x1F) > 0x10)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1764,
            0,
            "%s",
            "((parentValue->w.type & VAR_MASK) >= VAR_THREAD) && ((parentValue->w.type & VAR_MASK) <= VAR_CHILD_THREAD)"))
    {
        __debugbreak();
    }
    Scr_ClearThread(inst, parentId);
    id = FindObjectVariable(inst, gScrVarPub[inst].pauseArrayId, parentId);
    if (id)
    {
        for (selfNameId = FindObject(inst, id); ; RemoveObjectVariable(inst, selfNameId, name - 0x10000))
        {
            notifyListEntry = FindFirstSibling(inst, selfNameId);
            if (!notifyListEntry)
                break;
            if ((gScrVarGlob[inst].variableList[notifyListEntry + 0x8000].w.status & 0x1F) != 1
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1779,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_CHILD_BEGIN(inst)+ notifyListEntry].w.type & VAR_MASK) == VAR_POINTER"))
            {
                __debugbreak();
            }
            name = gScrVarGlob[inst].variableList[notifyListEntry + 0x8000].w.status >> 8;
            if (name - 0x10000 >= 0x10000
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1781,
                    0,
                    "%s",
                    "(name - SL_MAX_STRING_INDEX) < (1 << 16)"))
            {
                __debugbreak();
            }
            ObjectVariable = FindObjectVariable(inst, selfNameId, name - 0x10000);
            VariableValueAddress = GetVariableValueAddress(inst, ObjectVariable);
            VM_CancelNotify(inst, VariableValueAddress->next, name - 0x10000);
            Scr_KillEndonThread(inst, name - 0x10000);
        }
        if (GetArraySize(inst, selfNameId)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                1790,
                0,
                "%s",
                "!GetArraySize( inst, selfNameId )"))
        {
            __debugbreak();
        }
        RemoveObjectVariable(inst, gScrVarPub[inst].pauseArrayId, parentId);
    }
    parentValue->w.status &= 0xFFFFFFE0;
    parentValue->w.status |= 0x15u;
}

void __cdecl Scr_KillEndonThread(scriptInstance_t inst, unsigned int threadId)
{
    VariableValueInternal *parentValue; // [esp+0h] [ebp-4h]

    parentValue = &gScrVarGlob[inst].variableList[threadId + 1];
    if ((parentValue->w.status & 0x60) != 0x60
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1805,
            0,
            "%s",
            "(parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL"))
    {
        __debugbreak();
    }
    if ((parentValue->w.status & 0x1F) != 0xD
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1806,
            0,
            "%s",
            "(parentValue->w.type & VAR_MASK) == VAR_THREAD"))
    {
        __debugbreak();
    }
    if (parentValue->nextSibling
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1807,
            0,
            "%s",
            "!parentValue->nextSibling"))
    {
        __debugbreak();
    }
    RemoveRefToObject(inst, parentValue->u.o.u.nextEntId);
    if (FindObjectVariable(inst, gScrVarPub[inst].pauseArrayId, threadId)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1811,
            0,
            "%s",
            "!FindObjectVariable( inst, gScrVarPub[inst].pauseArrayId, threadId )"))
    {
        __debugbreak();
    }
    parentValue->w.status &= 0xFFFFFFE0;
    parentValue->w.status |= 0x15u;
}

unsigned int __cdecl AllocValue(scriptInstance_t inst)
{
    VariableValueInternal *entry; // [esp+4h] [ebp-14h]
    unsigned int newIndex; // [esp+8h] [ebp-10h]
    unsigned int next; // [esp+Ch] [ebp-Ch]
    VariableValueInternal *entryValue; // [esp+10h] [ebp-8h]
    unsigned int index; // [esp+14h] [ebp-4h]

    index = gScrVarGlob[inst].variableList[0x8000].u.next;
    if (!index)
        Scr_TerminalError("exceeded maximum number of script variables", inst);
    entry = &gScrVarGlob[inst].variableList[index + 0x8000];
    entryValue = &gScrVarGlob[inst].variableList[entry->hash.id + 0x8000];
    if ((entryValue->w.status & 0x60) != 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1946,
            0,
            "%s",
            "(entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_FREE"))
    {
        __debugbreak();
    }
    next = entryValue->u.next;
    if (entry != entryValue && (entry->w.status & 0x60) == 0)
    {
        newIndex = entry->v.next;
        if (newIndex == index
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                1954,
                0,
                "%s",
                "newIndex != index"))
        {
            __debugbreak();
        }
        gScrVarGlob[inst].variableList[newIndex + 0x8000].hash.id = entry->hash.id;
        entry->hash.id = index;
        entryValue->v.next = newIndex;
        entryValue->u.next = entry->u.next;
        entryValue = entry;
    }
    gScrVarGlob[inst].variableList[0x8000].u.next = next;
    gScrVarGlob[inst].variableList[next + 0x8000].hash.u.prev = 0;
    entryValue->v.next = index;
    entryValue->nextSibling = 0;
    entry->hash.u.prev = 0;
    if ((!entry->hash.id || entry->hash.id >= 0x3FFFE)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1970,
            0,
            "%s",
            "entry->hash.id > 0 && entry->hash.id < VARIABLELIST_CHILD_SIZE(inst)"))
    {
        __debugbreak();
    }
    ++gScrVarPub[inst].totalObjectRefCount;
    if (gScrVarDebugPub[inst])
    {
        if (gScrVarDebugPub[inst]->leakCount[entry->hash.id + 0x8000]
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                1975,
                0,
                "%s",
                "!gScrVarDebugPub[inst]->leakCount[VARIABLELIST_CHILD_BEGIN(inst)+ entry->hash.id]"))
        {
            __debugbreak();
        }
        ++gScrVarDebugPub[inst]->leakCount[entry->hash.id + 0x8000];
    }
    ++gScrVarPub[inst].numScriptValues;
    if (!gScrVarPub[inst].varUsagePos
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1982,
            0,
            "%s",
            "gScrVarPub[inst].varUsagePos"))
    {
        __debugbreak();
    }
    if (gScrVarDebugPub[inst])
    {
        if (gScrVarDebugPub[inst]->varUsage[entry->hash.id + 0x8000]
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                1985,
                0,
                "%s",
                "!gScrVarDebugPub[inst]->varUsage[VARIABLELIST_CHILD_BEGIN(inst)+ entry->hash.id]"))
        {
            __debugbreak();
        }
        gScrVarDebugPub[inst]->varUsage[entry->hash.id + 0x8000] = gScrVarPub[inst].varUsagePos;
    }
    entryValue->w.status = 96;
    if ((entryValue->w.status & 0x1F) != 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1991,
            0,
            "%s",
            "!(entryValue->w.type & VAR_MASK)"))
    {
        __debugbreak();
    }
    entryValue->w.status = entryValue->w.status;
    return entry->hash.id;
}

unsigned int __cdecl AllocObject(scriptInstance_t inst)
{
    VariableValueInternal *entryValue; // [esp+0h] [ebp-8h]
    unsigned int id; // [esp+4h] [ebp-4h]

    id = AllocVariable(inst);
    entryValue = &gScrVarGlob[inst].variableList[id + 1];
    entryValue->w.status = 96;
    if ( (entryValue->w.status & 0x1F) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2008,
                    0,
                    "%s",
                    "!(entryValue->w.type & VAR_MASK)") )
    {
        __debugbreak();
    }
    entryValue->w.status |= 0x11u;
    entryValue->u.o.refCount = 0;
    return id;
}

unsigned int __cdecl AllocVariable(scriptInstance_t inst)
{
    VariableValueInternal *entry; // [esp+4h] [ebp-14h]
    unsigned int newIndex; // [esp+8h] [ebp-10h]
    unsigned int next; // [esp+Ch] [ebp-Ch]
    unsigned int index; // [esp+10h] [ebp-8h]
    VariableValueInternal *entryValue; // [esp+14h] [ebp-4h]

    index = gScrVarGlob[inst].variableList[1].u.next;
    if (!index)
        Scr_TerminalError("exceeded maximum number of script variables", inst);
    entry = &gScrVarGlob[inst].variableList[index + 1];
    entryValue = &gScrVarGlob[inst].variableList[entry->hash.id + 1];
    if ((entryValue->w.status & 0x60) != 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1835,
            0,
            "%s",
            "(entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_FREE"))
    {
        __debugbreak();
    }
    next = entryValue->u.next;
    if (entry != entryValue && (entry->w.status & 0x60) == 0)
    {
        newIndex = entry->v.next;
        if (newIndex == index
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                1843,
                0,
                "%s",
                "newIndex != index"))
        {
            __debugbreak();
        }
        gScrVarGlob[inst].variableList[newIndex + 1].hash.id = entry->hash.id;
        entry->hash.id = index;
        entryValue->v.next = newIndex;
        entryValue->u.next = entry->u.next;
        entryValue = entry;
    }
    gScrVarGlob[inst].variableList[1].u.next = next;
    gScrVarGlob[inst].variableList[next + 1].hash.u.prev = 0;
    entryValue->v.next = index;
    entryValue->nextSibling = 0;
    entry->hash.u.prev = 0;
    if ((!entry->hash.id || entry->hash.id >= 0x7FFE)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1859,
            0,
            "%s",
            "entry->hash.id > 0 && entry->hash.id < VARIABLELIST_PARENT_SIZE(inst)"))
    {
        __debugbreak();
    }
    ++gScrVarPub[inst].totalObjectRefCount;
    if (gScrVarDebugPub[inst])
    {
        if (gScrVarDebugPub[inst]->leakCount[entry->hash.id + 1]
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                1864,
                0,
                "%s",
                "!gScrVarDebugPub[inst]->leakCount[VARIABLELIST_PARENT_BEGIN(inst)+ entry->hash.id]"))
        {
            __debugbreak();
        }
        ++gScrVarDebugPub[inst]->leakCount[entry->hash.id + 1];
    }
    ++gScrVarPub[inst].numScriptObjects;
    if (!gScrVarPub[inst].varUsagePos
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1871,
            0,
            "%s",
            "gScrVarPub[inst].varUsagePos"))
    {
        __debugbreak();
    }
    if (gScrVarDebugPub[inst])
    {
        if (gScrVarDebugPub[inst]->varUsage[entry->hash.id + 1]
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                1874,
                0,
                "%s",
                "!gScrVarDebugPub[inst]->varUsage[VARIABLELIST_PARENT_BEGIN(inst)+ entry->hash.id]"))
        {
            __debugbreak();
        }
        gScrVarDebugPub[inst]->varUsage[entry->hash.id + 1] = gScrVarPub[inst].varUsagePos;
    }
    return entry->hash.id;
}

unsigned int __cdecl Scr_AllocArray(scriptInstance_t inst)
{
    const char *varUsagePos; // [esp+0h] [ebp-Ch]
    VariableValueInternal *entryValue; // [esp+4h] [ebp-8h]
    unsigned int id; // [esp+8h] [ebp-4h]

    varUsagePos = gScrVarPub[inst].varUsagePos;
    if (!varUsagePos)
        gScrVarPub[inst].varUsagePos = "<script array variable>";
    id = AllocVariable(inst);
    entryValue = &gScrVarGlob[inst].variableList[id + 1];
    entryValue->w.status = 96;
    if ((entryValue->w.status & 0x1F) != 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2054,
            0,
            "%s",
            "!(entryValue->w.type & VAR_MASK)"))
    {
        __debugbreak();
    }
    entryValue->w.status |= 0x14u;
    entryValue->u.o.refCount = 0;
    entryValue->u.o.u.entnum = 0;
    gScrVarPub[inst].varUsagePos = varUsagePos;
    return id;
}

unsigned int __cdecl AllocThread(scriptInstance_t inst, unsigned int self)
{
    VariableValueInternal *entryValue; // [esp+0h] [ebp-8h]
    unsigned int id; // [esp+4h] [ebp-4h]

    id = AllocVariable(inst);
    entryValue = &gScrVarGlob[inst].variableList[id + 1];
    entryValue->w.status = 96;
    if ( (entryValue->w.status & 0x1F) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2075,
                    0,
                    "%s",
                    "!(entryValue->w.type & VAR_MASK)") )
    {
        __debugbreak();
    }
    entryValue->w.status |= 0xDu;
    entryValue->u.o.refCount = 0;
    entryValue->u.o.u.nextEntId = self;
    return id;
}

unsigned int __cdecl AllocChildThread(scriptInstance_t inst, unsigned int self, unsigned int parentLocalId)
{
    VariableValueInternal *entryValue; // [esp+0h] [ebp-8h]
    unsigned int id; // [esp+4h] [ebp-4h]

    id = AllocVariable(inst);
    entryValue = &gScrVarGlob[inst].variableList[id + 1];
    entryValue->w.status = 96;
    if ( (entryValue->w.status & 0x1F) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2092,
                    0,
                    "%s",
                    "!(entryValue->w.type & VAR_MASK)") )
    {
        __debugbreak();
    }
    entryValue->w.status |= 0x10u;
    if ( (entryValue->w.status & 0xFFFFFF00) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2094,
                    0,
                    "%s",
                    "!(entryValue->w.parentLocalId & VAR_NAME_HIGH_MASK)") )
    {
        __debugbreak();
    }
    entryValue->w.status |= parentLocalId << 8;
    entryValue->u.o.refCount = 0;
    entryValue->u.o.u.nextEntId = self;
    return id;
}

unsigned int __cdecl Scr_GetSelf(scriptInstance_t inst, unsigned int threadId)
{
    if ( (gScrVarGlob[inst].variableList[threadId + 1].w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2105,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ threadId].w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( ((gScrVarGlob[inst].variableList[threadId + 1].w.status & 0x1F) < 0xD
         || (gScrVarGlob[inst].variableList[threadId + 1].w.status & 0x1F) > 0x10)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2106,
                    0,
                    "%s",
                    "((gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ threadId].w.type & VAR_MASK) >= VAR_THREAD) "
                    "&& ((gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ threadId].w.type & VAR_MASK) <= VAR_CHILD_THREAD)") )
    {
        __debugbreak();
    }
    return gScrVarGlob[inst].variableList[threadId + 1].u.o.u.self;
}

void __cdecl FreeValue(scriptInstance_t inst, unsigned int id)
{
    VariableValueInternal *entry; // [esp+Ch] [ebp-Ch]
    VariableValueInternal *entryValue; // [esp+10h] [ebp-8h]
    unsigned int index; // [esp+14h] [ebp-4h]

    entryValue = &gScrVarGlob[inst].variableList[id + 0x8000];
    if ((entryValue->w.status & 0x60) != 0x60
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2119,
            0,
            "%s\n\t(entryValue->w.status & VAR_STAT_MASK) = %i",
            "((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL)",
            entryValue->w.status & 0x60))
    {
        __debugbreak();
    }
    if ((entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
            323,
            0,
            "%s",
            "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE"))
    {
        __debugbreak();
    }
    if ((entryValue->w.status & 0x1F) >= 0xD
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2120,
            0,
            "%s",
            "!IsObject( entryValue )"))
    {
        __debugbreak();
    }
    if (gScrVarGlob[inst].variableList[entryValue->v.next + 0x8000].hash.id != id
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2121,
            0,
            "%s",
            "gScrVarGlob[inst].variableList[VARIABLELIST_CHILD_BEGIN(inst)+ entryValue->v.index].hash.id == id"))
    {
        __debugbreak();
    }
    RemoveRefToValue(inst, entryValue->w.status & 0x1F, entryValue->u.u);
    if ((!id || id >= 0x3FFFE)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2127,
            0,
            "%s",
            "id > 0 && id < VARIABLELIST_CHILD_SIZE(inst)"))
    {
        __debugbreak();
    }
    --gScrVarPub[inst].totalObjectRefCount;
    if (gScrVarDebugPub[inst])
    {
        if (!gScrVarDebugPub[inst]->leakCount[id + 0x8000]
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                2132,
                0,
                "%s",
                "gScrVarDebugPub[inst]->leakCount[VARIABLELIST_CHILD_BEGIN(inst)+ id]"))
        {
            __debugbreak();
        }
        if (--gScrVarDebugPub[inst]->leakCount[id + 0x8000]
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                2134,
                0,
                "%s",
                "!gScrVarDebugPub[inst]->leakCount[VARIABLELIST_CHILD_BEGIN(inst)+ id]"))
        {
            __debugbreak();
        }
    }
    --gScrVarPub[inst].numScriptValues;
    if (gScrVarDebugPub[inst])
    {
        if (!gScrVarDebugPub[inst]->varUsage[id + 0x8000]
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                2142,
                0,
                "%s",
                "gScrVarDebugPub[inst]->varUsage[VARIABLELIST_CHILD_BEGIN(inst)+ id]"))
        {
            __debugbreak();
        }
        gScrVarDebugPub[inst]->varUsage[id + 0x8000] = 0;
    }
    if ((entryValue->w.status & 0x60) != 0x60
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2147,
            0,
            "%s\n\t(entryValue->w.status & VAR_STAT_MASK) = %i",
            "((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL)",
            entryValue->w.status & 0x60))
    {
        __debugbreak();
    }
    index = entryValue->v.next;
    entry = &gScrVarGlob[inst].variableList[index + 0x8000];
    if (entry->hash.id != id
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2151,
            0,
            "%s",
            "entry->id == id"))
    {
        __debugbreak();
    }
    if (entry->hash.u.prev
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2152,
            0,
            "%s",
            "!entry->u.prevSibling"))
    {
        __debugbreak();
    }
    if (entryValue->nextSibling
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2153,
            0,
            "%s",
            "!entryValue->nextSibling"))
    {
        __debugbreak();
    }
    entryValue->w.status = 0;
    entryValue->u.next = gScrVarGlob[inst].variableList[0x8000].u.next;
    entry->hash.u.prev = 0;
    gScrVarGlob[inst].variableList[gScrVarGlob[inst].variableList[0x8000].u.next + 0x8000].hash.u.prev = index;
    gScrVarGlob[inst].variableList[0x8000].u.next = index;
}
void __cdecl RemoveRefToObject(scriptInstance_t inst, unsigned int id)
{
    int unsignedValue; // [esp+0h] [ebp-10h]
    unsigned int classnum; // [esp+4h] [ebp-Ch]
    unsigned int entArrayId; // [esp+8h] [ebp-8h]
    VariableValueInternal *entryValue; // [esp+Ch] [ebp-4h]

    if ((!id || id >= 0x7FFE)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2176,
            0,
            "%s",
            "id >= 1 && id < VARIABLELIST_PARENT_SIZE(inst)"))
    {
        __debugbreak();
    }
    entryValue = &gScrVarGlob[inst].variableList[id + 1];
    if ((entryValue->w.status & 0x60) != 0x60
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2179,
            0,
            "%s\n\t(entryValue->w.status & VAR_STAT_MASK) = %i",
            "((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL)",
            entryValue->w.status & 0x60))
    {
        __debugbreak();
    }
    if ((entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
            323,
            0,
            "%s",
            "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE"))
    {
        __debugbreak();
    }
    if ((entryValue->w.status & 0x1F) < 0xD
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2180,
            0,
            "%s",
            "IsObject( entryValue )"))
    {
        __debugbreak();
    }
    if (entryValue->u.o.refCount)
    {
        --gScrVarPub[inst].totalObjectRefCount;
        if (gScrVarDebugPub[inst])
        {
            if (!gScrVarDebugPub[inst]->leakCount[id + 1]
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2188,
                    0,
                    "%s",
                    "gScrVarDebugPub[inst]->leakCount[VARIABLELIST_PARENT_BEGIN(inst)+ id]"))
            {
                __debugbreak();
            }
            --gScrVarDebugPub[inst]->leakCount[id + 1];
        }
        if (!--entryValue->u.o.refCount && (entryValue->w.status & 0x1F) == 0x13 && !entryValue->nextSibling)
        {
            entryValue->w.status &= 0xFFFFFFE0;
            entryValue->w.status |= 0x12u;
            classnum = entryValue->w.status >> 8;
            if (classnum >= 5
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2203,
                    0,
                    "%s",
                    "classnum < CLASS_NUM_COUNT"))
            {
                __debugbreak();
            }
            entArrayId = gScrClassMap[inst][classnum].entArrayId;
            if (!entArrayId
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2206,
                    0,
                    "%s",
                    "entArrayId"))
            {
                __debugbreak();
            }
            unsignedValue = entryValue->u.o.u.entnum & 0x3FFF;
            if (inst == SCRIPTINSTANCE_CLIENT)
            {
                if ((int)entryValue->u.o.u.entnum >> 14)
                    unsignedValue += 1536 * ((int)entryValue->u.o.u.entnum >> 14);
            }
            RemoveArrayVariable(inst, entArrayId, unsignedValue);
        }
    }
    else
    {
        if (entryValue->nextSibling)
            ClearObject(inst, id);
        FreeVariable(inst, id);
    }
}
void __cdecl FreeVariable(scriptInstance_t inst, unsigned int id)
{
    VariableValueInternal *entry; // [esp+4h] [ebp-Ch]
    VariableValueInternal *entryValue; // [esp+8h] [ebp-8h]
    unsigned int index; // [esp+Ch] [ebp-4h]

    if ((!id || id >= 0x7FFE)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1890,
            0,
            "%s",
            "id > 0 && id < VARIABLELIST_PARENT_SIZE(inst)"))
    {
        __debugbreak();
    }
    --gScrVarPub[inst].totalObjectRefCount;
    if (gScrVarDebugPub[inst])
    {
        if (!gScrVarDebugPub[inst]->leakCount[id + 1]
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                1895,
                0,
                "%s",
                "gScrVarDebugPub[inst]->leakCount[VARIABLELIST_PARENT_BEGIN(inst)+ id]"))
        {
            __debugbreak();
        }
        if (--gScrVarDebugPub[inst]->leakCount[id + 1]
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                1897,
                0,
                "%s",
                "!gScrVarDebugPub[inst]->leakCount[VARIABLELIST_PARENT_BEGIN(inst)+ id]"))
        {
            __debugbreak();
        }
    }
    --gScrVarPub[inst].numScriptObjects;
    if (gScrVarDebugPub[inst])
    {
        if (!gScrVarDebugPub[inst]->varUsage[id + 1]
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                1905,
                0,
                "%s",
                "gScrVarDebugPub[inst]->varUsage[VARIABLELIST_PARENT_BEGIN(inst)+ id]"))
        {
            __debugbreak();
        }
        gScrVarDebugPub[inst]->varUsage[id + 1] = 0;
    }
    entryValue = &gScrVarGlob[inst].variableList[id + 1];
    if ((entryValue->w.status & 0x60) != 0x60
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1912,
            0,
            "%s\n\t(entryValue->w.status & VAR_STAT_MASK) = %i",
            "((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL)",
            entryValue->w.status & 0x60))
    {
        __debugbreak();
    }
    index = entryValue->v.next;
    entry = &gScrVarGlob[inst].variableList[index + 1];
    if (entry->hash.id != id
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1916,
            0,
            "%s",
            "entry->id == id"))
    {
        __debugbreak();
    }
    if (entry->hash.u.prev
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1917,
            0,
            "%s",
            "!entry->u.prevSibling"))
    {
        __debugbreak();
    }
    if (entryValue->nextSibling
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            1918,
            0,
            "%s",
            "!entryValue->nextSibling"))
    {
        __debugbreak();
    }
    entryValue->w.status = 0;
    entryValue->u.next = gScrVarGlob[inst].variableList[1].u.next;
    entry->hash.u.prev = 0;
    gScrVarGlob[inst].variableList[gScrVarGlob[inst].variableList[1].u.next + 1].hash.u.prev = index;
    gScrVarGlob[inst].variableList[1].u.next = index;
}

void __cdecl RemoveRefToEmptyObject(scriptInstance_t inst, unsigned int id)
{
    VariableValueInternal *entryValue; // [esp+0h] [ebp-4h]

    entryValue = &gScrVarGlob[inst].variableList[id + 1];
    if ((entryValue->w.status & 0x60) != 0x60
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2232,
            0,
            "%s\n\t(entryValue->w.status & VAR_STAT_MASK) = %i",
            "((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL)",
            entryValue->w.status & 0x60))
    {
        __debugbreak();
    }
    if ((entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
            323,
            0,
            "%s",
            "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE"))
    {
        __debugbreak();
    }
    if ((entryValue->w.status & 0x1F) < 0xD
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2233,
            0,
            "%s",
            "IsObject( entryValue )"))
    {
        __debugbreak();
    }
    if (entryValue->nextSibling
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2234,
            0,
            "%s",
            "!entryValue->nextSibling"))
    {
        __debugbreak();
    }
    if (entryValue->u.o.refCount)
    {
        if ((!id || id >= 0x7FFE)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                2238,
                0,
                "%s",
                "id >= 1 && id < VARIABLELIST_PARENT_SIZE(inst)"))
        {
            __debugbreak();
        }
        --gScrVarPub[inst].totalObjectRefCount;
        if (gScrVarDebugPub[inst])
        {
            if (!gScrVarDebugPub[inst]->leakCount[id + 1]
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2243,
                    0,
                    "%s",
                    "gScrVarDebugPub[inst]->leakCount[VARIABLELIST_PARENT_BEGIN(inst)+ id]"))
            {
                __debugbreak();
            }
            --gScrVarDebugPub[inst]->leakCount[id + 1];
        }
        --entryValue->u.o.refCount;
    }
    else
    {
        FreeVariable(inst, id);
    }
}

int __cdecl Scr_GetRefCountToObject(scriptInstance_t inst, unsigned int id)
{
    VariableValueInternal *entryValue; // [esp+0h] [ebp-4h]

    entryValue = &gScrVarGlob[inst].variableList[id + 1];
    if ( (entryValue->w.status & 0x60) != 0x60
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2262,
                    0,
                    "%s\n\t(entryValue->w.status & VAR_STAT_MASK) = %i",
                    "((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL)",
                    entryValue->w.status & 0x60) )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) < 0xD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2263,
                    0,
                    "%s",
                    "IsObject( entryValue )") )
    {
        __debugbreak();
    }
    return entryValue->u.o.refCount;
}

float *__cdecl Scr_AllocVector(scriptInstance_t inst, float *v)
{
    float *result; // eax

    result = Scr_AllocVector(inst);
    *result = *v;
    result[1] = v[1];
    result[2] = v[2];
    return result;
}

float *__cdecl Scr_AllocVector(scriptInstance_t inst)
{
    char *v1; // eax

    v1 = MT_Alloc(16, 2, inst);
    *(_DWORD *)v1 = 0;
    _InterlockedExchangeAdd(&gScrVarPub[inst].totalVectorRefCount, 1u);
    if (gScrStringDebugGlob[inst])
        _InterlockedExchangeAdd(&gScrStringDebugGlob[inst]->refCount[(v1 - gScrMemTreePub[inst].mt_buffer) / 16], 1u);
    return (float *)(v1 + 4);
}

bool __cdecl IsValidArrayIndex(scriptInstance_t inst, unsigned int unsignedValue)
{
    return unsignedValue + 0x7E8002 <= 0xFE8001;
}

unsigned int __cdecl GetInternalVariableIndex(scriptInstance_t inst, unsigned int unsignedValue)
{
    if (!IsValidArrayIndex(inst, unsignedValue)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2306,
            0,
            "%s",
            "IsValidArrayIndex( inst, unsignedValue )"))
    {
        __debugbreak();
    }
    return (unsignedValue + 0x800000) & 0xFFFFFF;
}
unsigned int __cdecl FindArrayVariable(scriptInstance_t inst, unsigned int parentId, unsigned int intValue)
{
    return gScrVarGlob[inst].variableList[FindArrayVariableIndex(inst, parentId, intValue) + 0x8000].hash.id;
}

unsigned int __cdecl FindArrayVariableIndex(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue)
{
    if (!IsValidArrayIndex(inst, unsignedValue)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2314,
            0,
            "%s",
            "IsValidArrayIndex( inst, unsignedValue )"))
    {
        __debugbreak();
    }
    return FindVariableIndexInternal(inst, parentId, (unsignedValue + 0x800000) & 0xFFFFFF);
}

unsigned int __cdecl FindVariable(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue)
{
    return gScrVarGlob[inst].variableList[FindVariableIndexInternal(inst, parentId, unsignedValue) + 0x8000].hash.id;
}

unsigned int __cdecl FindObjectVariable(scriptInstance_t inst, unsigned int parentId, unsigned int id)
{
    return gScrVarGlob[inst].variableList[FindVariableIndexInternal(inst, parentId, id + 0x10000) + 0x8000].hash.id;
}

unsigned int __cdecl GetArrayVariableIndex(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue)
{
    if (!IsValidArrayIndex(inst, unsignedValue)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2340,
            0,
            "%s",
            "IsValidArrayIndex( inst, unsignedValue )"))
    {
        __debugbreak();
    }
    return GetVariableIndexInternal(inst, parentId, (unsignedValue + 0x800000) & 0xFFFFFF);
}

unsigned int __cdecl Scr_GetVariableFieldIndex(scriptInstance_t inst, unsigned int parentId, int name)
{
    const char *v4; // eax
    VariableValueInternal *v5; // [esp+20h] [ebp-10h]
    unsigned int index; // [esp+28h] [ebp-8h]
    unsigned int type; // [esp+2Ch] [ebp-4h]

    if (!parentId
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2360,
            0,
            "%s",
            "parentId"))
    {
        __debugbreak();
    }
    v5 = &gScrVarGlob[inst].variableList[parentId + 1];
    if ((v5->w.status & 0x60) == 0
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
            323,
            0,
            "%s",
            "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE"))
    {
        __debugbreak();
    }
    if ((v5->w.status & 0x1F) < 0xD
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2361,
            0,
            "%s",
            "IsObject( &gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ parentId] )"))
    {
        __debugbreak();
    }
    type = gScrVarGlob[inst].variableList[parentId + 1].w.status & 0x1F;
    if (type <= 0x11)
        return GetVariableIndexInternal(inst, parentId, name);
    if (type == 19)
    {
        index = FindVariableIndexInternal(inst, parentId, name);
        if (index)
        {
            return index;
        }
        else
        {
            gScrVarPub[inst].entId = parentId;
            gScrVarPub[inst].entFieldName = name;
            return 0;
        }
    }
    else
    {
        v4 = va("cannot set field of %s", var_typename[type]);
        Scr_Error(inst, v4, 0);
        return 0;
    }
}

VariableValue __cdecl Scr_FindVariableField(scriptInstance_t inst, unsigned int parentId, unsigned int name)
{
    VariableValueInternal *entryValue; // [esp+18h] [ebp-24h]
    VariableValue value; // [esp+30h] [ebp-Ch]
    unsigned int id; // [esp+38h] [ebp-4h]

    if ( !parentId
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2393,
                    0,
                    "%s",
                    "parentId") )
    {
        __debugbreak();
    }
    entryValue = &gScrVarGlob[inst].variableList[parentId + 1];
    if ( (entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) < 0xD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2394,
                    0,
                    "%s",
                    "IsObject( &gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ parentId] )") )
    {
        __debugbreak();
    }

    if ( ((gScrVarGlob[inst].variableList[parentId + 1].w.status & 0x1F) < 0xD
         || (gScrVarGlob[inst].variableList[parentId + 1].w.status & 0x1F) >= 0x14)
        && (gScrVarGlob[inst].variableList[parentId + 1].w.status & 0x1F) < 0x15
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2395,
                    0,
                    "%s",
                    "((gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ parentId].w.type & VAR_MASK) >= FIRST_OBJECT"
                    " && (gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ parentId].w.type & VAR_MASK) < FIRST_NONF"
                    "IELD_OBJECT) || ((gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ parentId].w.type & VAR_MASK)"
                    " >= FIRST_DEAD_OBJECT)") )
    {
        __debugbreak();
    }

    id = FindVariable(inst, parentId, name);
    if ( id )
    {
        return Scr_EvalVariable(inst, id);
    }
    else if ( (gScrVarGlob[inst].variableList[parentId + 1].w.status & 0x1F) == 0x13 )
    {
        return Scr_EvalVariableEntityField(inst, parentId, name);
    }
    else
    {
        value.type = VAR_UNDEFINED;
        value.u.intValue = 0;
        return value;
    }
}

unsigned int __cdecl Scr_FindAllVariableField(scriptInstance_t inst, unsigned int parentId, unsigned int *names)
{
    VariableValueInternal *v3; // ecx
    unsigned int classnum; // [esp+4Ch] [ebp-1Ch]
    VariableValueInternal *parentValue; // [esp+50h] [ebp-18h]
    unsigned int name; // [esp+58h] [ebp-10h]
    unsigned int namea; // [esp+58h] [ebp-10h]
    unsigned int nameb; // [esp+58h] [ebp-10h]
    unsigned int count; // [esp+60h] [ebp-8h]
    unsigned int id; // [esp+64h] [ebp-4h]
    unsigned int ida; // [esp+64h] [ebp-4h]
    unsigned int idb; // [esp+64h] [ebp-4h]

    parentValue = &gScrVarGlob[inst].variableList[parentId + 1];
    if ((parentValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2423,
            0,
            "%s",
            "(parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE"))
    {
        __debugbreak();
    }
    if ((parentValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
            323,
            0,
            "%s",
            "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE"))
    {
        __debugbreak();
    }
    if ((parentValue->w.status & 0x1F) < 0xD
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2424,
            0,
            "%s",
            "IsObject( parentValue )"))
    {
        __debugbreak();
    }
    count = 0;
    switch (parentValue->w.status & 0x1F)
    {
    case 0xDu:
    case 0xEu:
    case 0xFu:
    case 0x10u:
    case 0x11u:
    case 0x12u:
        goto $LN12_71;
    case 0x13u:
        classnum = parentValue->w.status >> 8;
        if (classnum >= 5
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                2432,
                0,
                "%s",
                "classnum < CLASS_NUM_COUNT"))
        {
            __debugbreak();
        }
        for (id = FindFirstSibling(inst, gScrClassMap[inst][classnum].id); id; id = FindNextSibling(inst, id))
        {
            v3 = &gScrVarGlob[inst].variableList[id + 0x8000];
            name = (v3->w.status >> 8) - 0x800000;
            if (v3->w.status >> 8 == 0x800000
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2440,
                    0,
                    "%s",
                    "name"))
            {
                __debugbreak();
            }
            if (name <= gScrVarPub[inst].canonicalStrCount && !FindVariable(inst, parentId, name))
            {
                if (names)
                    names[count] = name;
                ++count;
            }
        }
    $LN12_71:
        for (ida = FindFirstSibling(inst, parentId); ida; ida = FindNextSibling(inst, ida))
        {
            namea = gScrVarGlob[inst].variableList[ida + 0x8000].w.status >> 8;
            if (!namea
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2463,
                    0,
                    "%s",
                    "name"))
            {
                __debugbreak();
            }
            if (namea != 98302 && namea != 98303)
            {
                if (namea > gScrVarPub[inst].canonicalStrCount
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        2466,
                        0,
                        "%s",
                        "name <= gScrVarPub[inst].canonicalStrCount"))
                {
                    __debugbreak();
                }
                if (names)
                    names[count] = namea;
                ++count;
            }
        }
        break;
    case 0x14u:
        for (idb = FindFirstSibling(inst, parentId); idb; idb = FindNextSibling(inst, idb))
        {
            nameb = gScrVarGlob[inst].variableList[idb + 0x8000].w.status >> 8;
            if (!nameb
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2478,
                    0,
                    "%s",
                    "name"))
            {
                __debugbreak();
            }
            if (names)
                names[count] = nameb;
            ++count;
        }
        break;
    default:
        return count;
    }
    return count;
}

void __cdecl ClearVariableField(scriptInstance_t inst, unsigned int parentId, unsigned int name, VariableValue *value)
{
    VariableValueInternal *v4; // [esp+20h] [ebp-14h]
    unsigned int classnum; // [esp+24h] [ebp-10h]
    VariableValueInternal *parentValue; // [esp+28h] [ebp-Ch]
    unsigned int fieldId; // [esp+30h] [ebp-4h]
    VariableValue *valuea; // [esp+48h] [ebp+14h]

    v4 = &gScrVarGlob[inst].variableList[parentId + 1];
    if ( (v4->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (v4->w.status & 0x1F) < 0xD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2499,
                    0,
                    "%s",
                    "IsObject( &gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ parentId] )") )
    {
        __debugbreak();
    }
    if ( ((gScrVarGlob[inst].variableList[parentId + 1].w.status & 0x1F) < 0xD
         || (gScrVarGlob[inst].variableList[parentId + 1].w.status & 0x1F) >= 0x14)
        && (gScrVarGlob[inst].variableList[parentId + 1].w.status & 0x1F) < 0x15
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2500,
                    0,
                    "%s",
                    "((gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ parentId].w.type & VAR_MASK) >= FIRST_OBJECT"
                    " && (gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ parentId].w.type & VAR_MASK) < FIRST_NONF"
                    "IELD_OBJECT) || ((gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ parentId].w.type & VAR_MASK)"
                    " >= FIRST_DEAD_OBJECT)") )
    {
        __debugbreak();
    }
    if ( FindVariableIndexInternal(inst, parentId, name) )
    {
        RemoveVariable(inst, parentId, name);
    }
    else
    {
        parentValue = &gScrVarGlob[inst].variableList[parentId + 1];
        if ( (parentValue->w.status & 0x1F) == 0x13 )
        {
            if ( parentValue->w.status >> 8 >= 5
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                            2513,
                            0,
                            "%s",
                            "(parentValue->w.classnum >> VAR_NAME_BITS) < CLASS_NUM_COUNT") )
            {
                __debugbreak();
            }
            classnum = parentValue->w.status >> 8;
            fieldId = FindArrayVariable(inst, gScrClassMap[inst][classnum].id, name);
            if ( fieldId )
            {
                valuea = value + 1;
                valuea->type = 0;
                SetEntityFieldValue(
                    inst,
                    classnum,
                    parentValue->u.o.u.entnum & 0x3FFF,
                    (int)parentValue->u.o.u.entnum >> 14,
                    gScrVarGlob[inst].variableList[fieldId + 0x8000].u.next,
                    valuea);
            }
        }
    }
}

unsigned int __cdecl GetArrayVariable(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue)
{
    return gScrVarGlob[inst].variableList[GetArrayVariableIndex(inst, parentId, unsignedValue) + 0x8000].hash.id;
}

unsigned int __cdecl GetNewArrayVariable(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue)
{
    return gScrVarGlob[inst].variableList[GetNewArrayVariableIndex(inst, parentId, unsignedValue) + 0x8000].hash.id;
}

unsigned int __cdecl GetNewArrayVariableIndex(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue)
{
    if (!IsValidArrayIndex(inst, unsignedValue)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2348,
            0,
            "%s",
            "IsValidArrayIndex( inst, unsignedValue )"))
    {
        __debugbreak();
    }
    return GetNewVariableIndexInternal(inst, parentId, (unsignedValue + 0x800000) & 0xFFFFFF);
}

unsigned int __cdecl GetNewVariableIndexInternal(scriptInstance_t inst, unsigned int parentId, unsigned int name)
{
    if ( FindVariableIndexInternal(inst, parentId, name)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1312,
                    0,
                    "%s",
                    "!FindVariableIndexInternal( inst, parentId, name )") )
    {
        __debugbreak();
    }
    return GetNewVariableIndexInternal2(inst, parentId, name, (parentId + name) % 0x3FFFD + 1);
}

unsigned int __cdecl GetVariable(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue)
{
    return gScrVarGlob[inst].variableList[GetVariableIndexInternal(inst, parentId, unsignedValue) + 0x8000].hash.id;
}

unsigned int __cdecl GetNewVariable(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue)
{
    return gScrVarGlob[inst].variableList[GetNewVariableIndexInternal(inst, parentId, unsignedValue) + 0x8000].hash.id;
}

unsigned int __cdecl GetObjectVariable(scriptInstance_t inst, unsigned int parentId, unsigned int id)
{
    if ( (gScrVarGlob[inst].variableList[parentId + 1].w.status & 0x1F) != 0x14
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2551,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ parentId].w.type & VAR_MASK) == VAR_ARRAY") )
    {
        __debugbreak();
    }
    return gScrVarGlob[inst].variableList[GetVariableIndexInternal(inst, parentId, id + 0x10000) + 0x8000].hash.id;
}

unsigned int __cdecl GetNewObjectVariable(scriptInstance_t inst, unsigned int parentId, unsigned int id)
{
    if ( (gScrVarGlob[inst].variableList[parentId + 1].w.status & 0x1F) != 0x14
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2559,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ parentId].w.type & VAR_MASK) == VAR_ARRAY") )
    {
        __debugbreak();
    }
    return gScrVarGlob[inst].variableList[GetNewVariableIndexInternal(inst, parentId, id + 0x10000) + 0x8000].hash.id;
}

unsigned int __cdecl GetNewObjectVariableReverse(scriptInstance_t inst, unsigned int parentId, unsigned int id)
{
    if ( (gScrVarGlob[inst].variableList[parentId + 1].w.status & 0x1F) != 0x14
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2567,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ parentId].w.type & VAR_MASK) == VAR_ARRAY") )
    {
        __debugbreak();
    }
    return gScrVarGlob[inst].variableList[GetNewVariableIndexReverseInternal(inst, parentId, id + 0x10000) + 0x8000].hash.id;
}

unsigned int __cdecl GetNewVariableIndexReverseInternal(
                scriptInstance_t inst,
                unsigned int parentId,
                unsigned int name)
{
    if ( FindVariableIndexInternal(inst, parentId, name)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1330,
                    0,
                    "%s",
                    "!FindVariableIndexInternal( inst, parentId, name )") )
    {
        __debugbreak();
    }
    return GetNewVariableIndexReverseInternal2(inst, parentId, name, (parentId + name) % 0x3FFFD + 1);
}

unsigned int __cdecl GetNewVariableIndexReverseInternal2(
                scriptInstance_t inst,
                unsigned int parentId,
                unsigned int name,
                unsigned int index)
{
    VariableValueInternal *parentValue; // [esp+20h] [ebp-20h]
    unsigned int siblingId; // [esp+24h] [ebp-1Ch]
    VariableValueInternal *entry; // [esp+28h] [ebp-18h]
    unsigned int siblingIndex; // [esp+2Ch] [ebp-14h]
    VariableValueInternal *siblingValue; // [esp+30h] [ebp-10h]
    VariableValueInternal *parent; // [esp+34h] [ebp-Ch]
    unsigned int id; // [esp+3Ch] [ebp-4h]
    unsigned int indexa; // [esp+54h] [ebp+14h]

    indexa = GetNewVariableIndexInternal3(inst, parentId, name, index);
    parentValue = &gScrVarGlob[inst].variableList[parentId + 1];
    if ( (parentValue->w.status & 0x60) != 0x60
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    1273,
                    0,
                    "%s\n\t(parentValue->w.status & VAR_STAT_MASK) = %i",
                    "((parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL)",
                    parentValue->w.status & 0x60) )
    {
        __debugbreak();
    }
    parent = &gScrVarGlob[inst].variableList[parentValue->v.next + 1];
    entry = &gScrVarGlob[inst].variableList[indexa + 0x8000];
    id = entry->hash.id;
    siblingId = parent->hash.u.prev;
    if ( siblingId )
    {
        siblingValue = &gScrVarGlob[inst].variableList[siblingId + 0x8000];
        if ( (siblingValue->w.status & 0x60) == 0
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                        323,
                        0,
                        "%s",
                        "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
        {
            __debugbreak();
        }
        if ( (siblingValue->w.status & 0x1F) >= 0xD
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        1284,
                        0,
                        "%s",
                        "!IsObject( siblingValue )") )
        {
            __debugbreak();
        }
        siblingValue->nextSibling = indexa;
        siblingIndex = FindVariableIndexInternal(inst, parentId, siblingValue->w.status >> 8);
        if ( !siblingIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        1289,
                        0,
                        "%s",
                        "siblingIndex") )
        {
            __debugbreak();
        }
    }
    else
    {
        siblingIndex = 0;
        parentValue->nextSibling = id;
    }
    parent->hash.u.prev = id;
    entry->hash.u.prev = siblingIndex;
    gScrVarGlob[inst].variableList[id + 0x8000].nextSibling = 0;
    return indexa;
}

void __cdecl RemoveVariable(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue)
{
    unsigned int index; // [esp+20h] [ebp-8h]
    unsigned int id; // [esp+24h] [ebp-4h]

    index = FindVariableIndexInternal(inst, parentId, unsignedValue);
    if ( !index
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp", 2579, 0, "%s", "index") )
    {
        __debugbreak();
    }
    id = gScrVarGlob[inst].variableList[index + 0x8000].hash.id;
    MakeVariableExternal(inst, index, &gScrVarGlob[inst].variableList[parentId + 1]);
    FreeChildValue(inst, parentId, id);
}

void __cdecl RemoveNextVariable(scriptInstance_t inst, unsigned int parentId)
{
    unsigned int index; // [esp+20h] [ebp-8h]
    unsigned int id; // [esp+24h] [ebp-4h]

    if ( (gScrVarGlob[inst].variableList[parentId + 1].w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2593,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ parentId].w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    id = gScrVarGlob[inst].variableList[parentId + 1].nextSibling;
    if ( !id
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp", 2596, 0, "%s", "id") )
    {
        __debugbreak();
    }
    index = FindVariableIndexInternal(inst, parentId, gScrVarGlob[inst].variableList[id + 0x8000].w.status >> 8);
    if ( !index
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp", 2599, 0, "%s", "index") )
    {
        __debugbreak();
    }
    if ( id != gScrVarGlob[inst].variableList[index + 0x8000].hash.id
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2601,
                    0,
                    "%s",
                    "id == gScrVarGlob[inst].variableList[VARIABLELIST_CHILD_BEGIN(inst)+ index].hash.id") )
    {
        __debugbreak();
    }
    MakeVariableExternal(inst, index, &gScrVarGlob[inst].variableList[parentId + 1]);
    FreeChildValue(inst, parentId, id);
}

void __cdecl RemoveObjectVariable(scriptInstance_t inst, unsigned int parentId, unsigned int id)
{
    if ( (gScrVarGlob[inst].variableList[parentId + 1].w.status & 0x1F) != 0x14
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2610,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ parentId].w.type & VAR_MASK) == VAR_ARRAY") )
    {
        __debugbreak();
    }
    RemoveVariable(inst, parentId, id + 0x10000);
}

void __cdecl RemoveArrayVariable(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue)
{
    if (!IsValidArrayIndex(inst, unsignedValue)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2626,
            0,
            "%s",
            "IsValidArrayIndex( inst, unsignedValue )"))
    {
        __debugbreak();
    }
    RemoveVariable(inst, parentId, (unsignedValue + 0x800000) & 0xFFFFFF);
}

void __cdecl SafeRemoveVariable(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue)
{
    VariableValueInternal *v3; // [esp+0h] [ebp-2Ch]
    unsigned int index; // [esp+24h] [ebp-8h]
    unsigned int id; // [esp+28h] [ebp-4h]

    index = FindVariableIndexInternal(inst, parentId, unsignedValue);
    if ( index )
    {
        id = gScrVarGlob[inst].variableList[index + 0x8000].hash.id;
        v3 = &gScrVarGlob[inst].variableList[id + 0x8000];
        if ( (v3->w.status & 0x60) == 0
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                        323,
                        0,
                        "%s",
                        "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
        {
            __debugbreak();
        }
        if ( (v3->w.status & 0x1F) >= 0xD
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        2642,
                        0,
                        "%s",
                        "!IsObject( &gScrVarGlob[inst].variableList[VARIABLELIST_CHILD_BEGIN(inst)+ id] )") )
        {
            __debugbreak();
        }
        MakeVariableExternal(inst, index, &gScrVarGlob[inst].variableList[parentId + 1]);
        FreeChildValue(inst, parentId, id);
    }
}

void __cdecl RemoveVariableValue(scriptInstance_t inst, unsigned int parentId, unsigned int index)
{
    unsigned int id; // [esp+0h] [ebp-4h]

    if ( !index
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp", 2719, 0, "%s", "index") )
    {
        __debugbreak();
    }
    id = gScrVarGlob[inst].variableList[index + 0x8000].hash.id;
    if ( !id
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp", 2722, 0, "%s", "id") )
    {
        __debugbreak();
    }
    MakeVariableExternal(inst, index, &gScrVarGlob[inst].variableList[parentId + 1]);
    FreeChildValue(inst, parentId, id);
}

void __cdecl SetNewVariableValue(scriptInstance_t inst, unsigned int id, VariableValue *value)
{
    VariableValueInternal *entryValue; // [esp+0h] [ebp-4h]

    if ( value->type >= 13
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2733,
                    0,
                    "%s",
                    "!IsObject( value )") )
    {
        __debugbreak();
    }
    entryValue = &gScrVarGlob[inst].variableList[id + 0x8000];
    if ( (entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2736,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) >= 0xD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2737,
                    0,
                    "%s",
                    "!IsObject( entryValue )") )
    {
        __debugbreak();
    }
    if ( value->type >= 0x16u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2738,
                    0,
                    "%s",
                    "value->type >= 0 && value->type < VAR_COUNT") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2739,
                    0,
                    "%s",
                    "(entryValue->w.type & VAR_MASK) == VAR_UNDEFINED") )
    {
        __debugbreak();
    }
    if ( value->type == 1
        && (entryValue->w.status & 0x1F) >= 0x15
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2741,
                    0,
                    "%s",
                    "(value->type != VAR_POINTER) || ((entryValue->w.type & VAR_MASK) < FIRST_DEAD_OBJECT)") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2743,
                    0,
                    "%s",
                    "!(entryValue->w.type & VAR_MASK)") )
    {
        __debugbreak();
    }
    entryValue->w.status |= value->type;
    entryValue->u.next = value->u.intValue;
}

VariableValueInternal_u *__cdecl GetVariableValueAddress(scriptInstance_t inst, unsigned int id)
{
    VariableValueInternal *entryValue; // [esp+0h] [ebp-4h]

    if ( !id
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp", 2753, 0, "%s", "id") )
    {
        __debugbreak();
    }
    entryValue = &gScrVarGlob[inst].variableList[id + 0x8000];
    if ( (entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2755,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2756,
                    0,
                    "%s",
                    "(entryValue->w.type & VAR_MASK) != VAR_UNDEFINED") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) >= 0xD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2757,
                    0,
                    "%s",
                    "!IsObject( entryValue )") )
    {
        __debugbreak();
    }
    return &entryValue->u;
}

void __cdecl SetVariableEntityFieldValue(
                scriptInstance_t inst,
                unsigned int entId,
                unsigned int fieldName,
                VariableValue *value)
{
    VariableValueInternal *entValue; // [esp+8h] [ebp-Ch]
    VariableValueInternal *entryValue; // [esp+Ch] [ebp-8h]
    unsigned int fieldId; // [esp+10h] [ebp-4h]

    if ( value->type >= 13
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2769,
                    0,
                    "%s",
                    "!IsObject( value )") )
    {
        __debugbreak();
    }
    if ( value->type == 10
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2770,
                    0,
                    "%s",
                    "value->type != VAR_STACK") )
    {
        __debugbreak();
    }
    entValue = &gScrVarGlob[inst].variableList[entId + 1];
    if ( (entValue->w.status & 0x1F) != 0x13
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2773,
                    0,
                    "%s",
                    "(entValue->w.type & VAR_MASK) == VAR_ENTITY") )
    {
        __debugbreak();
    }
    if ( entValue->w.status >> 8 >= 5
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2774,
                    0,
                    "%s",
                    "(entValue->w.classnum >> VAR_NAME_BITS) < CLASS_NUM_COUNT") )
    {
        __debugbreak();
    }
    fieldId = FindArrayVariable(inst, gScrClassMap[inst][entValue->w.status >> 8].id, fieldName);
    if ( !fieldId
        || !SetEntityFieldValue(
                    inst,
                    entValue->w.status >> 8,
                    entValue->u.o.u.entnum & 0x3FFF,
                    (int)entValue->u.o.u.entnum >> 14,
                    gScrVarGlob[inst].variableList[fieldId + 0x8000].u.next,
                    value) )
    {
        entryValue = &gScrVarGlob[inst].variableList[GetNewVariable(inst, entId, fieldName) + 0x8000];
        if ( (entryValue->w.status & 0x1F) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        2783,
                        0,
                        "%s",
                        "!(entryValue->w.type & VAR_MASK)") )
        {
            __debugbreak();
        }
        entryValue->w.status |= value->type;
        entryValue->u.next = value->u.intValue;
    }
}

void __cdecl ClearVariableValue(scriptInstance_t inst, unsigned int id)
{
    VariableValueInternal *entryValue; // [esp+8h] [ebp-4h]

    if ( !id
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp", 2793, 0, "%s", "id") )
    {
        __debugbreak();
    }
    entryValue = &gScrVarGlob[inst].variableList[id + 0x8000];
    if ( (entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2796,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) >= 0xD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2797,
                    0,
                    "%s",
                    "!IsObject( entryValue )") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) == 0xA
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2798,
                    0,
                    "%s",
                    "(entryValue->w.type & VAR_MASK) != VAR_STACK") )
    {
        __debugbreak();
    }
    RemoveRefToValue(inst, entryValue->w.status & 0x1F, entryValue->u.u);
    entryValue->w.status &= 0xFFFFFFE0;
    if ( (entryValue->w.status & 0x1F) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2803,
                    0,
                    "%s",
                    "(entryValue->w.type & VAR_MASK) == VAR_UNDEFINED") )
    {
        __debugbreak();
    }
}

void __cdecl SetVariableFieldValue(scriptInstance_t inst, unsigned int id, VariableValue *value)
{
    if (id)
        SetVariableValue(inst, id, value);
    else
        SetVariableEntityFieldValue(inst, gScrVarPub[inst].entId, gScrVarPub[inst].entFieldName, value);
}

unsigned int __cdecl Scr_EvalVariableObject(scriptInstance_t inst, unsigned int id)
{
    const char *v3; // eax
    VariableValueInternal *entryValue; // [esp+0h] [ebp-8h]
    unsigned int type; // [esp+4h] [ebp-4h]

    entryValue = &gScrVarGlob[inst].variableList[id + 0x8000];
    if ( (entryValue->w.status & 0x60) == 0
        && id
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2826,
                    0,
                    "%s",
                    "((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE) || !id") )
    {
        __debugbreak();
    }
    type = entryValue->w.status & 0x1F;
    if ( type == 1 && (type = gScrVarGlob[inst].variableList[entryValue->u.next + 1].w.status & 0x1F, type < 0x14) )
    {
        if ( (gScrVarGlob[inst].variableList[entryValue->u.next + 1].w.status & 0x1F) < 0xD
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        2834,
                        0,
                        "%s",
                        "type >= FIRST_OBJECT") )
        {
            __debugbreak();
        }
        return entryValue->u.next;
    }
    else
    {
        v3 = va("%s is not a field object", var_typename[type]);
        Scr_Error(inst, v3, 0);
        return 0;
    }
}

VariableValue __cdecl Scr_EvalVariableEntityField(scriptInstance_t inst, unsigned int entId, unsigned int fieldName)
{
    VariableValueInternal *entValue; // [esp+8h] [ebp-14h]
    unsigned int fieldId; // [esp+10h] [ebp-Ch]
    int value; // [esp+14h] [ebp-8h]
    VariableValue result;

    entValue = &gScrVarGlob[inst].variableList[entId + VARIABLELIST_PARENT_BEGIN];

    iassert((entValue->w.type & VAR_MASK) == VAR_ENTITY);
    iassert((entValue->w.classnum >> VAR_NAME_BITS) < CLASS_NUM_COUNT);


    fieldId = FindArrayVariable(inst, gScrClassMap[inst][entValue->w.classnum >> VAR_NAME_BITS].id, fieldName);

    if (!fieldId)
    {
        result.type = VAR_UNDEFINED;
        result.u.intValue = 0;
        return result;
    }

    result = GetEntityFieldValue(
        inst,
        entValue->w.classnum >> VAR_NAME_BITS,
        entValue->u.o.u.entnum & 0x3FFF,
        (int)entValue->u.o.u.entnum >> 14,
        gScrVarGlob[inst].variableList[fieldId + 0x8000].u.next);

    //var_pointer check is optimized out halfway, does nothing

    return result;
}

VariableValue __cdecl Scr_EvalVariableField(scriptInstance_t inst, unsigned int id)
{
    if (id)
        return Scr_EvalVariable(inst, id);
    else
        return Scr_EvalVariableEntityField(inst, gScrVarPub[inst].entId, gScrVarPub[inst].entFieldName);
}

void __cdecl Scr_EvalSizeValue(scriptInstance_t inst, VariableValue *value)
{
    VariableUnion v2; // [esp+10h] [ebp-1Ch]
    VariableUnion stringValue; // [esp+1Ch] [ebp-10h]
    VariableValueInternal *entryValue; // [esp+20h] [ebp-Ch]
    char *error_message; // [esp+24h] [ebp-8h]
    VariableUnion id; // [esp+28h] [ebp-4h]

    if ( value->type == 1 )
    {
        id.intValue = value->u.intValue;
        entryValue = &gScrVarGlob[inst].variableList[value->u.intValue + 1];
        value->type = 6;
        if ( (entryValue->w.status & 0x1F) == 0x14 )
            v2.intValue = entryValue->u.o.u.entnum;
        else
            v2.intValue = 1;
        value->u = v2;
        RemoveRefToObject(inst, id.stringValue);
    }
    else if ( value->type == 2 )
    {
        value->type = 6;
        stringValue.intValue = value->u.intValue;
        value->u.intValue = strlen(SL_ConvertToString(value->u.intValue, inst));
        SL_RemoveRefToString(inst, stringValue.stringValue);
    }
    else
    {
        if ( value->type == 10
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        2958,
                        0,
                        "%s",
                        "value->type != VAR_STACK") )
        {
            __debugbreak();
        }
        error_message = va("size cannot be applied to %s", var_typename[value->type]);
        RemoveRefToValue(inst, value->type, value->u);
        value->type = 0;
        Scr_Error(inst, error_message, 0);
    }
}

unsigned int __cdecl GetArraySize(scriptInstance_t inst, unsigned int id)
{
    VariableValueInternal *entryValue; // [esp+0h] [ebp-4h]

    if ( !id
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp", 2972, 0, "%s", "id") )
    {
        __debugbreak();
    }
    entryValue = &gScrVarGlob[inst].variableList[id + 1];
    if ( (entryValue->w.status & 0x1F) != 0x14
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2975,
                    0,
                    "%s",
                    "(entryValue->w.type & VAR_MASK) == VAR_ARRAY") )
    {
        __debugbreak();
    }
    return entryValue->u.o.u.entnum;
}

unsigned int __cdecl GetObject(scriptInstance_t inst, unsigned int id)
{
    VariableValueInternal *entryValue; // [esp+0h] [ebp-4h]

    if ( !id
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp", 2984, 0, "%s", "id") )
    {
        __debugbreak();
    }
    entryValue = &gScrVarGlob[inst].variableList[id + 0x8000];
    if ( (entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2987,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2988,
                    0,
                    "%s",
                    "(entryValue->w.type & VAR_MASK) == VAR_UNDEFINED || (entryValue->w.type & VAR_MASK) == VAR_POINTER") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) == 0 )
    {
        entryValue->w.status |= 1u;
        entryValue->u.next = AllocObject(inst);
    }
    if ( (entryValue->w.status & 0x1F) != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2997,
                    0,
                    "%s",
                    "(entryValue->w.type & VAR_MASK) == VAR_POINTER") )
    {
        __debugbreak();
    }
    return entryValue->u.next;
}

unsigned int __cdecl GetArray(scriptInstance_t inst, unsigned int id)
{
    VariableValueInternal *entryValue; // [esp+0h] [ebp-4h]

    if ( !id
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp", 3006, 0, "%s", "id") )
    {
        __debugbreak();
    }
    entryValue = &gScrVarGlob[inst].variableList[id + 0x8000];
    if ( (entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    3009,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    3010,
                    0,
                    "%s",
                    "(entryValue->w.type & VAR_MASK) == VAR_UNDEFINED || (entryValue->w.type & VAR_MASK) == VAR_POINTER") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) == 0 )
    {
        entryValue->w.status |= 1u;
        entryValue->u.next = Scr_AllocArray(inst);
    }
    if ( (entryValue->w.status & 0x1F) != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    3019,
                    0,
                    "%s",
                    "(entryValue->w.type & VAR_MASK) == VAR_POINTER") )
    {
        __debugbreak();
    }
    return entryValue->u.next;
}

unsigned int __cdecl FindObject(scriptInstance_t inst, unsigned int id)
{
    VariableValueInternal *entryValue; // [esp+0h] [ebp-4h]

    if ( !id
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp", 3028, 0, "%s", "id") )
    {
        __debugbreak();
    }
    entryValue = &gScrVarGlob[inst].variableList[id + 0x8000];
    if ( (entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    3031,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    3032,
                    0,
                    "%s",
                    "(entryValue->w.type & VAR_MASK) == VAR_POINTER") )
    {
        __debugbreak();
    }
    return entryValue->u.next;
}

bool __cdecl IsFieldObject(scriptInstance_t inst, unsigned int id)
{
    VariableValueInternal *entryValue; // [esp+0h] [ebp-4h]

    if ( !id
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp", 3042, 0, "%s", "id") )
    {
        __debugbreak();
    }
    entryValue = &gScrVarGlob[inst].variableList[id + 1];
    if ( (entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) < 0xD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    3045,
                    0,
                    "%s",
                    "IsObject( entryValue )") )
    {
        __debugbreak();
    }
    return (entryValue->w.status & 0x1F) < 0x14;
}

bool __cdecl Scr_IsThreadAlive(unsigned int thread, scriptInstance_t inst)
{
    VariableValueInternal *entryValue; // [esp+0h] [ebp-4h]

    if (!gScrVarPub[inst].timeArrayId
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            3055,
            0,
            "%s",
            "gScrVarPub[inst].timeArrayId"))
    {
        __debugbreak();
    }
    entryValue = &gScrVarGlob[inst].variableList[thread + 1];
    if ((entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            3058,
            0,
            "%s",
            "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE"))
    {
        __debugbreak();
    }
    if (((entryValue->w.status & 0x1F) < 0xD || (entryValue->w.status & 0x1F) > 0x10)
        && (entryValue->w.status & 0x1F) != 0x15
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            3059,
            0,
            "%s",
            "((entryValue->w.type & VAR_MASK) >= VAR_THREAD && (entryValue->w.type & VAR_MASK) <= VAR_CHILD_THREAD) || (ent"
            "ryValue->w.type & VAR_MASK) == VAR_DEAD_THREAD"))
    {
        __debugbreak();
    }
    return (entryValue->w.status & 0x1F) != 21;
}

void __cdecl Scr_EvalBoolNot(scriptInstance_t inst, VariableValue *value)
{
    Scr_CastBool(inst, value);
    if ( value->type == 6 )
        value->u.intValue = value->u.intValue == 0;
}

void __cdecl Scr_EvalBoolComplement(scriptInstance_t inst, VariableValue *value)
{
    const char *v2; // eax
    int type; // [esp+8h] [ebp-4h]

    if ( value->type == 6 )
    {
        value->u.intValue = ~value->u.intValue;
    }
    else
    {
        type = value->type;
        RemoveRefToValue(inst, type, value->u);
        value->type = 0;
        v2 = va("~ cannot be applied to \"%s\"", var_typename[type]);
        Scr_Error(inst, v2, 0);
    }
}

void __cdecl Scr_CastBool(scriptInstance_t inst, VariableValue *value)
{
    const char *v2; // eax
    int type; // [esp+Ch] [ebp-4h]

    if ( value->type == 6 )
    {
        value->u.intValue = value->u.intValue != 0;
    }
    else if ( value->type == 5 )
    {
        value->type = 6;
        value->u.intValue = value->u.floatValue != 0.0;
    }
    else
    {
        type = value->type;
        RemoveRefToValue(inst, type, value->u);
        value->type = 0;
        v2 = va("cannot cast %s to bool", var_typename[type]);
        Scr_Error(inst, v2, 0);
    }
}

char __cdecl Scr_CastString(scriptInstance_t inst, VariableValue *value)
{
    const float *constTempVector; // [esp+14h] [ebp-4h]

    switch (value->type)
    {
    case 2:
        return 1;
    case 6:
        value->type = 2;
        value->u.intValue = SL_GetStringForInt(inst, value->u.intValue);
        return 1;
    case 5:
        value->type = 2;
        value->u.intValue = SL_GetStringForFloat(value->u.floatValue, inst);
        return 1;
    case 4:
        value->type = 2;
        constTempVector = (const float *)value->u.intValue;
        value->u.intValue = SL_GetStringForVector(inst, value->u.vectorValue);
        RemoveRefToVector(inst, constTempVector);
        return 1;
    default:
        gScrVarPub[inst].error_message = va("cannot cast %s to string", var_typename[value->type]);
        RemoveRefToValue(inst, value->type, value->u);
        value->type = 0;
        return 0;
    }
}

void __cdecl Scr_CastDebugString(scriptInstance_t inst, VariableValue *value)
{
    const XAnim_s *Anims; // eax
    unsigned int String; // eax
    unsigned int intValue; // [esp-4h] [ebp-20h]
    unsigned int stringValue; // [esp+10h] [ebp-Ch]
    char *s; // [esp+18h] [ebp-4h]
    char *sa; // [esp+18h] [ebp-4h]

    switch ( value->type )
    {
        case 1:
            sa = (char *)var_typename[GetObjectType(inst, value->u.intValue)];
            String = SL_GetString_(inst, sa, 0, 15);
            goto LABEL_6;
        case 2:
        case 4:
        case 5:
        case 6:
            Scr_CastString(inst, value);
            return;
        case 3:
            value->type = 2;
            return;
        case 0xB:
            intValue = (unsigned __int16)value->u.intValue;
            Anims = Scr_GetAnims(HIWORD(value->u.intValue), SCRIPTINSTANCE_SERVER);
            s = XAnimGetAnimDebugName(Anims, intValue);
            String = SL_GetString_(inst, s, 0, 15);
            goto LABEL_6;
        default:
            String = SL_GetString_(inst, (char *)var_typename[value->type], 0, 15);
LABEL_6:
            stringValue = String;
            RemoveRefToValue(inst, value->type, value->u);
            value->type = 2;
            value->u.intValue = stringValue;
            return;
    }
}

char __cdecl Scr_GetEntClassId(scriptInstance_t inst, unsigned int id)
{
    if ( GetObjectType(inst, id) != 19
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    3193,
                    0,
                    "%s",
                    "GetObjectType( inst, id ) == VAR_ENTITY") )
    {
        __debugbreak();
    }
    return gScrClassMap[inst][gScrVarGlob[inst].variableList[id + 1].w.status >> 8].charId;
}

int __cdecl Scr_GetEntNum(scriptInstance_t inst, unsigned int id)
{
    if ( GetObjectType(inst, id) != 19
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    3200,
                    0,
                    "%s",
                    "GetObjectType( inst, id ) == VAR_ENTITY") )
    {
        __debugbreak();
    }
    return gScrVarGlob[inst].variableList[id + 1].u.o.u.entnum & 0x3FFF;
}

void __cdecl Scr_ClearVector(scriptInstance_t inst, VariableValue *value)
{
    int i; // [esp+Ch] [ebp-4h]

    for ( i = 2; i >= 0; --i )
        RemoveRefToValue(inst, value[i].type, value[i].u);
    value->type = 0;
}

void __cdecl Scr_CastVector(scriptInstance_t inst, VariableValue *value)
{
    const char *v2; // eax
    int type; // [esp+0h] [ebp-14h]
    int i; // [esp+4h] [ebp-10h]
    float vec[3]; // [esp+8h] [ebp-Ch] BYREF

    for (i = 2; i >= 0; --i)
    {
        type = value[i].type;
        if (type == 5)
        {
            vec[2 - i] = value[i].u.floatValue;
        }
        else
        {
            if (type != 6)
            {
                gScrVarPub[inst].error_index = i + 1;
                Scr_ClearVector(inst, value);
                v2 = va("type %s is not a float", var_typename[type]);
                Scr_Error(inst, v2, 0);
                return;
            }
            vec[2 - i] = (float)value[i].u.intValue;
        }
    }
    value->type = 4;
    value->u.intValue = (int)Scr_AllocVector(inst, vec);
}

unsigned int __cdecl Scr_EvalFieldObject(scriptInstance_t inst, unsigned int tempVariable, VariableValue *value)
{
    unsigned int type; // [esp+14h] [ebp-Ch]
    VariableValue tempValue; // [esp+18h] [ebp-8h] BYREF

    type = value->type;
    if ( type == 1 && (type = gScrVarGlob[inst].variableList[value->u.intValue + 1].w.status & 0x1F, type < 0x14) )
    {
        if ( (gScrVarGlob[inst].variableList[value->u.intValue + 1].w.status & 0x1F) < 0xD
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        3259,
                        0,
                        "%s",
                        "type >= FIRST_OBJECT") )
        {
            __debugbreak();
        }
        tempValue.type = 1;
        tempValue.u.intValue = value->u.intValue;
        SetVariableValue(inst, tempVariable, &tempValue);
        return tempValue.u.pointerValue;
    }
    else
    {
        RemoveRefToValue(inst, value->type, value->u);
        Scr_Error(inst, va("%s is not a field object", var_typename[type]), 0);
        return 0;
    }
}

void __cdecl Scr_UnmatchingTypesError(scriptInstance_t inst, VariableValue *value1, VariableValue *value2)
{
    char *v3; // eax
    char *v4; // [esp-Ch] [ebp-28h]
    const char *v5; // [esp-8h] [ebp-24h]
    const char *v6; // [esp-4h] [ebp-20h]
    int type1; // [esp+10h] [ebp-Ch]
    int type2; // [esp+14h] [ebp-8h]
    char *error_message; // [esp+18h] [ebp-4h]

    if ( gScrVarPub[inst].error_message )
    {
        error_message = 0;
    }
    else
    {
        type1 = value1->type;
        type2 = value2->type;
        Scr_CastDebugString(inst, value1);
        Scr_CastDebugString(inst, value2);
        if ( value1->type != 2
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        3288,
                        0,
                        "%s",
                        "value1->type == VAR_STRING") )
        {
            __debugbreak();
        }
        if ( value2->type != 2
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        3289,
                        0,
                        "%s",
                        "value2->type == VAR_STRING") )
        {
            __debugbreak();
        }
        v6 = var_typename[type2];
        v5 = var_typename[type1];
        v4 = SL_ConvertToString(value2->u.intValue, inst);
        v3 = SL_ConvertToString(value1->u.intValue, inst);
        error_message = va("pair '%s' and '%s' has unmatching types '%s' and '%s'", v3, v4, v5, v6);
    }
    RemoveRefToValue(inst, value1->type, value1->u);
    value1->type = 0;
    RemoveRefToValue(inst, value2->type, value2->u);
    value2->type = 0;
    Scr_Error(inst, error_message, 0);
}

void __cdecl Scr_EvalOr(scriptInstance_t inst, VariableValue *value1, VariableValue *value2)
{
    if ( value1->type == 6 && value2->type == 6 )
        value1->u.intValue |= value2->u.intValue;
    else
        Scr_UnmatchingTypesError(inst, value1, value2);
}

void __cdecl Scr_EvalExOr(scriptInstance_t inst, VariableValue *value1, VariableValue *value2)
{
    if ( value1->type == 6 && value2->type == 6 )
        value1->u.intValue ^= value2->u.intValue;
    else
        Scr_UnmatchingTypesError(inst, value1, value2);
}

void __cdecl Scr_EvalAnd(scriptInstance_t inst, VariableValue *value1, VariableValue *value2)
{
    if ( value1->type == 6 && value2->type == 6 )
        value1->u.intValue &= value2->u.intValue;
    else
        Scr_UnmatchingTypesError(inst, value1, value2);
}

void __cdecl Scr_EvalEquality(scriptInstance_t inst, VariableValue *value1, VariableValue *value2)
{
    bool v3; // [esp+0h] [ebp-1Ch]
    bool tempInt; // [esp+18h] [ebp-4h]
    bool tempInta; // [esp+18h] [ebp-4h]

    Scr_CastWeakerPair(inst, value1, value2);
    if ( value1->type != value2->type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    3521,
                    0,
                    "%s",
                    "value1->type == value2->type") )
    {
        __debugbreak();
    }
    switch ( value1->type )
    {
        case 0:
            value1->type = 6;
            value1->u.intValue = 1;
            break;
        case 1:
            if ( ((gScrVarGlob[inst].variableList[value1->u.intValue + 0x8000].w.status & 0x1F) == 0x14
                 || (gScrVarGlob[inst].variableList[value2->u.intValue + 0x8000].w.status & 0x1F) == 0x14)
                && !gScrVarPub[inst].evaluate) 
            {
                goto LABEL_21;
            }
            value1->type = 6;
            tempInta = value1->u.intValue == value2->u.intValue;
            RemoveRefToObject(inst, value1->u.intValue);
            RemoveRefToObject(inst, value2->u.intValue);
            value1->u.intValue = tempInta;
            break;
        case 2:
        case 3:
            value1->type = 6;
            tempInt = value1->u.intValue == value2->u.intValue;
            SL_RemoveRefToString(inst, value1->u.intValue);
            SL_RemoveRefToString(inst, value2->u.intValue);
            value1->u.intValue = tempInt;
            break;
        case 4:
            value1->type = 6;
            v3 = *(float *)value1->u.intValue == *(float *)value2->u.intValue
                && *(float *)(value1->u.intValue + 4) == *(float *)(value2->u.intValue + 4)
                && *(float *)(value1->u.intValue + 8) == *(float *)(value2->u.intValue + 8);
            RemoveRefToVector(inst, value1->u.vectorValue);
            RemoveRefToVector(inst, value2->u.vectorValue);
            value1->u.intValue = v3;
            break;
        case 5:
            value1->type = 6;
            value1->u.intValue = fabs(value1->u.floatValue - value2->u.floatValue) < 0.000001;
            break;
        case 6:
            value1->u.intValue = value1->u.intValue == value2->u.intValue;
            break;
        case 9:
            value1->type = 6;
            value1->u.intValue = value1->u.intValue == value2->u.intValue;
            break;
        case 0xB:
            value1->type = 6;
            value1->u.intValue = value1->u.intValue == value2->u.intValue;
            break;
        default:
LABEL_21:
            Scr_UnmatchingTypesError(inst, value1, value2);
            break;
    }
}

void __cdecl Scr_CastWeakerPair(scriptInstance_t inst, VariableValue *value1, VariableValue *value2)
{
    float *v3; // eax
    int type1; // [esp+0h] [ebp-Ch]
    float *tempVector; // [esp+4h] [ebp-8h]
    float *tempVectora; // [esp+4h] [ebp-8h]
    float *tempVectorb; // [esp+4h] [ebp-8h]
    int type2; // [esp+8h] [ebp-4h]

    type1 = value1->type;
    type2 = value2->type;
    if ( type1 != type2 )
    {
        if ( type1 == 5 && type2 == 6 )
        {
            value2->type = 5;
            value2->u.floatValue = (float)value2->u.intValue;
            return;
        }
        if ( type1 == 6 && type2 == 5 )
        {
            value1->type = 5;
            value1->u.floatValue = (float)value1->u.intValue;
            return;
        }
        if ( type1 == 4 )
        {
            if ( type2 == 5 )
            {
                tempVector = Scr_AllocVector(inst);
                *tempVector = value2->u.floatValue;
                tempVector[1] = value2->u.floatValue;
                tempVector[2] = value2->u.floatValue;
                value2->u.intValue = (int)tempVector;
                value2->type = 4;
                return;
            }
            if ( type2 == 6 )
            {
                v3 = Scr_AllocVector(inst);
                *v3 = (float)value2->u.intValue;
                v3[1] = (float)value2->u.intValue;
                v3[2] = (float)value2->u.intValue;
                value2->u.intValue = (int)v3;
                value2->type = 4;
                return;
            }
        }
        if ( type2 != 4 )
            goto LABEL_18;
        if ( type1 == 5 )
        {
            tempVectora = Scr_AllocVector(inst);
            *tempVectora = value1->u.floatValue;
            tempVectora[1] = value1->u.floatValue;
            tempVectora[2] = value1->u.floatValue;
            value1->u.intValue = (int)tempVectora;
            value1->type = 4;
            return;
        }
        if ( type1 == 6 )
        {
            tempVectorb = Scr_AllocVector(inst);
            *tempVectorb = (float)value1->u.intValue;
            tempVectorb[1] = (float)value1->u.intValue;
            tempVectorb[2] = (float)value1->u.intValue;
            value1->u.intValue = (int)tempVectorb;
            value1->type = 4;
        }
        else
        {
LABEL_18:
            Scr_UnmatchingTypesError(inst, value1, value2);
        }
    }
}

void __cdecl Scr_EvalInequality(scriptInstance_t inst, VariableValue *value1, VariableValue *value2)
{
    Scr_EvalEquality(inst, value1, value2);
    if ( value1->type != 6
        && value1->type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    3588,
                    0,
                    "%s",
                    "(value1->type == VAR_INTEGER) || (value1->type == VAR_UNDEFINED)") )
    {
        __debugbreak();
    }
    value1->u.intValue = value1->u.intValue == 0;
}

void __cdecl Scr_EvalLess(scriptInstance_t inst, VariableValue *value1, VariableValue *value2)
{
    int type; // [esp+4h] [ebp-4h]

    Scr_CastWeakerPair(inst, value1, value2);
    if ( value1->type != value2->type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    3597,
                    0,
                    "%s",
                    "value1->type == value2->type") )
    {
        __debugbreak();
    }
    type = value1->type;
    if ( type == 5 )
    {
        value1->type = 6;
        value1->u.intValue = value2->u.floatValue > value1->u.floatValue;
    }
    else if ( type == 6 )
    {
        value1->u.intValue = value1->u.intValue < value2->u.intValue;
    }
    else
    {
        Scr_UnmatchingTypesError(inst, value1, value2);
    }
}

void __cdecl Scr_EvalGreaterEqual(scriptInstance_t inst, VariableValue *value1, VariableValue *value2)
{
    Scr_EvalLess(inst, value1, value2);
    if ( value1->type != 6
        && value1->type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    3619,
                    0,
                    "%s",
                    "(value1->type == VAR_INTEGER) || (value1->type == VAR_UNDEFINED)") )
    {
        __debugbreak();
    }
    value1->u.intValue = value1->u.intValue == 0;
}

void __cdecl Scr_EvalGreater(scriptInstance_t inst, VariableValue *value1, VariableValue *value2)
{
    int type; // [esp+4h] [ebp-4h]

    Scr_CastWeakerPair(inst, value1, value2);
    if ( value1->type != value2->type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    3628,
                    0,
                    "%s",
                    "value1->type == value2->type") )
    {
        __debugbreak();
    }
    type = value1->type;
    if ( type == 5 )
    {
        value1->type = 6;
        value1->u.intValue = value1->u.floatValue > value2->u.floatValue;
    }
    else if ( type == 6 )
    {
        value1->u.intValue = value1->u.intValue > value2->u.intValue;
    }
    else
    {
        Scr_UnmatchingTypesError(inst, value1, value2);
    }
}

void __cdecl Scr_EvalLessEqual(scriptInstance_t inst, VariableValue *value1, VariableValue *value2)
{
    Scr_EvalGreater(inst, value1, value2);
    if ( value1->type != 6
        && value1->type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    3650,
                    0,
                    "%s",
                    "(value1->type == VAR_INTEGER) || (value1->type == VAR_UNDEFINED)") )
    {
        __debugbreak();
    }
    value1->u.intValue = value1->u.intValue == 0;
}

void __cdecl Scr_EvalShiftLeft(scriptInstance_t inst, VariableValue *value1, VariableValue *value2)
{
    if ( value1->type == 6 && value2->type == 6 )
        value1->u.intValue <<= value2->u.intValue;
    else
        Scr_UnmatchingTypesError(inst, value1, value2);
}

void __cdecl Scr_EvalShiftRight(scriptInstance_t inst, VariableValue *value1, VariableValue *value2)
{
    if ( value1->type == 6 && value2->type == 6 )
        value1->u.intValue >>= value2->u.intValue;
    else
        Scr_UnmatchingTypesError(inst, value1, value2);
}

void __cdecl Scr_EvalPlus(scriptInstance_t inst, VariableValue *value1, VariableValue *value2)
{
    const char *v3; // eax
    char v4; // [esp+3h] [ebp-2045h]
    char *v5; // [esp+8h] [ebp-2040h]
    char *v6; // [esp+Ch] [ebp-203Ch]
    char v7; // [esp+13h] [ebp-2035h]
    char *v8; // [esp+18h] [ebp-2030h]
    char *v9; // [esp+1Ch] [ebp-202Ch]
    unsigned int StringOfSize; // [esp+2Ch] [ebp-201Ch]
    int StringLen; // [esp+30h] [ebp-2018h]
    float *v12; // [esp+34h] [ebp-2014h]
    char *v13; // [esp+38h] [ebp-2010h]
    char *v14; // [esp+3Ch] [ebp-200Ch]
    char str[8192]; // [esp+40h] [ebp-2008h] BYREF
    unsigned int len; // [esp+2044h] [ebp-4h]

    Scr_CastWeakerStringPair(inst, value1, value2);
    if ( value1->type != value2->type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    3694,
                    0,
                    "%s",
                    "value1->type == value2->type") )
    {
        __debugbreak();
    }
    switch ( value1->type )
    {
        case 2:
            v13 = SL_ConvertToString(value1->u.intValue, inst);
            v14 = SL_ConvertToString(value2->u.intValue, inst);
            StringLen = SL_GetStringLen(inst, value1->u.intValue);
            len = StringLen + SL_GetStringLen(inst, value2->u.intValue) + 1;
            if ( (int)len <= 0x2000 )
            {
                v9 = v13;
                v8 = str;
                do
                {
                    v7 = *v9;
                    *v8++ = *v9++;
                }
                while ( v7 );
                v6 = v14;
                v5 = &str[StringLen];
                do
                {
                    v4 = *v6;
                    *v5++ = *v6++;
                }
                while ( v4 );
                StringOfSize = SL_GetStringOfSize(inst, str, 0, len, 15);
                SL_RemoveRefToString(inst, value1->u.intValue);
                SL_RemoveRefToString(inst, value2->u.intValue);
                value1->u.intValue = StringOfSize;
            }
            else
            {
                SL_RemoveRefToString(inst, value1->u.intValue);
                SL_RemoveRefToString(inst, value2->u.intValue);
                value1->type = 0;
                value2->type = 0;
                v3 = va("cannot concat \"%s\" and \"%s\" - max string length exceeded", v13, v14);
                Scr_Error(inst, v3, 0);
            }
            break;
        case 4:
            v12 = Scr_AllocVector(inst);
            *v12 = *(float *)value1->u.intValue + *(float *)value2->u.intValue;
            v12[1] = *(float *)(value1->u.intValue + 4) + *(float *)(value2->u.intValue + 4);
            v12[2] = *(float *)(value1->u.intValue + 8) + *(float *)(value2->u.intValue + 8);
            RemoveRefToVector(inst, value1->u.vectorValue);
            RemoveRefToVector(inst, value2->u.vectorValue);
            value1->u.intValue = (int)v12;
            break;
        case 5:
            value1->u.floatValue = value1->u.floatValue + value2->u.floatValue;
            break;
        case 6:
            value1->u.intValue += value2->u.intValue;
            break;
        default:
            Scr_UnmatchingTypesError(inst, value1, value2);
            break;
    }
}

void __cdecl Scr_CastWeakerStringPair(scriptInstance_t inst, VariableValue *value1, VariableValue *value2)
{
    int type1; // [esp+20h] [ebp-Ch]
    int type2; // [esp+24h] [ebp-8h]
    const float *constTempVector; // [esp+28h] [ebp-4h]
    const float *constTempVectora; // [esp+28h] [ebp-4h]

    type1 = value1->type;
    type2 = value2->type;
    if ( type1 != type2 )
    {
        if ( type1 < type2 )
        {
            if ( type1 == 2 )
            {
                switch ( type2 )
                {
                    case 4:
                        value2->type = 2;
                        constTempVector = (const float *)value2->u.intValue;
                        value2->u.intValue = SL_GetStringForVector(inst, value2->u.vectorValue);
                        RemoveRefToVector(inst, constTempVector);
                        return;
                    case 5:
                        value2->type = 2;
                        value2->u.intValue = SL_GetStringForFloat(value2->u.floatValue, inst);
                        return;
                    case 6:
                        value2->type = 2;
                        value2->u.intValue = SL_GetStringForInt(inst, value2->u.intValue);
                        return;
                }
            }
            else if ( type1 != 5 )
            {
LABEL_28:
                Scr_UnmatchingTypesError(inst, value1, value2);
                return;
            }
            if ( type2 == 6 )
            {
                value2->type = 5;
                value2->u.floatValue = (float)value2->u.intValue;
                return;
            }
            goto LABEL_28;
        }
        if ( type2 == 2 )
        {
            switch ( type1 )
            {
                case 4:
                    value1->type = 2;
                    constTempVectora = (const float *)value1->u.intValue;
                    value1->u.intValue = SL_GetStringForVector(inst, value1->u.vectorValue);
                    RemoveRefToVector(inst, constTempVectora);
                    return;
                case 5:
                    value1->type = 2;
                    value1->u.intValue = SL_GetStringForFloat(value1->u.floatValue, inst);
                    return;
                case 6:
                    value1->type = 2;
                    value1->u.intValue = SL_GetStringForInt(inst, value1->u.intValue);
                    return;
            }
        }
        else if ( type2 != 5 )
        {
            goto LABEL_28;
        }
        if ( type1 == 6 )
        {
            value1->type = 5;
            value1->u.floatValue = (float)value1->u.intValue;
            return;
        }
        goto LABEL_28;
    }
}

void __cdecl Scr_EvalMinus(scriptInstance_t inst, VariableValue *value1, VariableValue *value2)
{
    int type; // [esp+0h] [ebp-10h]
    float *tempVector; // [esp+Ch] [ebp-4h]

    Scr_CastWeakerPair(inst, value1, value2);
    if ( value1->type != value2->type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    3749,
                    0,
                    "%s",
                    "value1->type == value2->type") )
    {
        __debugbreak();
    }
    type = value1->type;
    switch ( type )
    {
        case 4:
            tempVector = Scr_AllocVector(inst);
            *tempVector = *(float *)value1->u.intValue - *(float *)value2->u.intValue;
            tempVector[1] = *(float *)(value1->u.intValue + 4) - *(float *)(value2->u.intValue + 4);
            tempVector[2] = *(float *)(value1->u.intValue + 8) - *(float *)(value2->u.intValue + 8);
            RemoveRefToVector(inst, value1->u.vectorValue);
            RemoveRefToVector(inst, value2->u.vectorValue);
            value1->u.intValue = (int)tempVector;
            break;
        case 5:
            value1->u.floatValue = value1->u.floatValue - value2->u.floatValue;
            break;
        case 6:
            value1->u.intValue -= value2->u.intValue;
            break;
        default:
            Scr_UnmatchingTypesError(inst, value1, value2);
            break;
    }
}

void __cdecl Scr_EvalMultiply(scriptInstance_t inst, VariableValue *value1, VariableValue *value2)
{
    int type; // [esp+0h] [ebp-10h]
    float *tempVector; // [esp+Ch] [ebp-4h]

    Scr_CastWeakerPair(inst, value1, value2);
    if ( value1->type != value2->type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    3782,
                    0,
                    "%s",
                    "value1->type == value2->type") )
    {
        __debugbreak();
    }
    type = value1->type;
    switch ( type )
    {
        case 4:
            tempVector = Scr_AllocVector(inst);
            *tempVector = *(float *)value1->u.intValue * *(float *)value2->u.intValue;
            tempVector[1] = *(float *)(value1->u.intValue + 4) * *(float *)(value2->u.intValue + 4);
            tempVector[2] = *(float *)(value1->u.intValue + 8) * *(float *)(value2->u.intValue + 8);
            RemoveRefToVector(inst, value1->u.vectorValue);
            RemoveRefToVector(inst, value2->u.vectorValue);
            value1->u.intValue = (int)tempVector;
            break;
        case 5:
            value1->u.floatValue = value1->u.floatValue * value2->u.floatValue;
            break;
        case 6:
            value1->u.intValue *= value2->u.intValue;
            break;
        default:
            Scr_UnmatchingTypesError(inst, value1, value2);
            break;
    }
}

void __cdecl Scr_EvalDivide(scriptInstance_t inst, VariableValue *value1, VariableValue *value2)
{
    int type; // [esp+0h] [ebp-18h]
    float *tempVector; // [esp+14h] [ebp-4h]

    Scr_CastWeakerPair(inst, value1, value2);
    if ( value1->type != value2->type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    3815,
                    0,
                    "%s",
                    "value1->type == value2->type") )
    {
        __debugbreak();
    }
    type = value1->type;
    switch ( type )
    {
        case 4:
            tempVector = Scr_AllocVector(inst);
            if ( *(float *)value2->u.intValue == 0.0
                || *(float *)(value2->u.intValue + 4) == 0.0
                || *(float *)(value2->u.intValue + 8) == 0.0 )
            {
                *tempVector = 0.0f;
                tempVector[1] = 0.0f;
                tempVector[2] = 0.0f;
                RemoveRefToVector(inst, value1->u.vectorValue);
                RemoveRefToVector(inst, value2->u.vectorValue);
                value1->u.intValue = (int)tempVector;
                Scr_Error(inst, "divide by 0", 0);
            }
            else
            {
                *tempVector = *(float *)value1->u.intValue / *(float *)value2->u.intValue;
                tempVector[1] = *(float *)(value1->u.intValue + 4) / *(float *)(value2->u.intValue + 4);
                tempVector[2] = *(float *)(value1->u.intValue + 8) / *(float *)(value2->u.intValue + 8);
                RemoveRefToVector(inst, value1->u.vectorValue);
                RemoveRefToVector(inst, value2->u.vectorValue);
                value1->u.intValue = (int)tempVector;
            }
            break;
        case 5:
            if ( value2->u.floatValue == 0.0 )
            {
LABEL_9:
                value1->u.intValue = 0;
                Scr_Error(inst, "divide by 0", 0);
                return;
            }
            value1->u.floatValue = value1->u.floatValue / value2->u.floatValue;
            break;
        case 6:
            value1->type = 5;
            if ( value2->u.intValue )
            {
                value1->u.floatValue = (float)value1->u.intValue / (float)value2->u.intValue;
                return;
            }
            goto LABEL_9;
        default:
            Scr_UnmatchingTypesError(inst, value1, value2);
            break;
    }
}

void __cdecl Scr_EvalMod(scriptInstance_t inst, VariableValue *value1, VariableValue *value2)
{
    if ( value1->type == 6 && value2->type == 6 )
    {
        if ( value2->u.intValue )
        {
            value1->u.intValue %= value2->u.intValue;
        }
        else
        {
            value1->u.intValue = 0;
            Scr_Error(inst, "divide by 0", 0);
        }
    }
    else
    {
        Scr_UnmatchingTypesError(inst, value1, value2);
    }
}

void __cdecl Scr_EvalBinaryOperator(scriptInstance_t inst, int op, VariableValue *value1, VariableValue *value2)
{
    switch ( op )
    {
        case 'f':
            Scr_EvalOr(inst, value1, value2);
            break;
        case 'g':
            Scr_EvalExOr(inst, value1, value2);
            break;
        case 'h':
            Scr_EvalAnd(inst, value1, value2);
            break;
        case 'i':
            Scr_EvalEquality(inst, value1, value2);
            break;
        case 'j':
            Scr_EvalInequality(inst, value1, value2);
            break;
        case 'k':
            Scr_EvalLess(inst, value1, value2);
            break;
        case 'l':
            Scr_EvalGreater(inst, value1, value2);
            break;
        case 'm':
            Scr_EvalLessEqual(inst, value1, value2);
            break;
        case 'n':
            Scr_EvalGreaterEqual(inst, value1, value2);
            break;
        case 'o':
            Scr_EvalShiftLeft(inst, value1, value2);
            break;
        case 'p':
            Scr_EvalShiftRight(inst, value1, value2);
            break;
        case 'q':
            Scr_EvalPlus(inst, value1, value2);
            break;
        case 'r':
            Scr_EvalMinus(inst, value1, value2);
            break;
        case 's':
            Scr_EvalMultiply(inst, value1, value2);
            break;
        case 't':
            Scr_EvalDivide(inst, value1, value2);
            break;
        case 'u':
            Scr_EvalMod(inst, value1, value2);
            break;
        default:
            return;
    }
}

bool __cdecl IsObjectFree(scriptInstance_t inst, unsigned int id)
{
    return (gScrVarGlob[inst].variableList[id + 1].w.status & 0x60) == 0;
}

unsigned int __cdecl GetValueType(scriptInstance_t inst, unsigned int id)
{
    if ( (gScrVarGlob[inst].variableList[id + 0x8000].w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    3964,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_CHILD_BEGIN(inst)+ id].w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    return gScrVarGlob[inst].variableList[id + 0x8000].w.status & 0x1F;
}

unsigned int __cdecl GetObjectType(scriptInstance_t inst, unsigned int id)
{
    if ( (gScrVarGlob[inst].variableList[id + 1].w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    3971,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ id].w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    return gScrVarGlob[inst].variableList[id + 1].w.status & 0x1F;
}

void __cdecl Scr_FreeEntityNum(unsigned int entnum, unsigned int classnum, scriptInstance_t inst)
{
    unsigned int entArrayId; // [esp+4h] [ebp-10h]
    unsigned int entnumId; // [esp+8h] [ebp-Ch]
    VariableValueInternal *entryValue; // [esp+Ch] [ebp-8h]
    unsigned int entId; // [esp+10h] [ebp-4h]

    if (gScrVarPub[inst].bInited)
    {
        entArrayId = gScrClassMap[inst][classnum].entArrayId;
        if (!entArrayId
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                3989,
                0,
                "%s",
                "entArrayId"))
        {
            __debugbreak();
        }
        entnumId = FindArrayVariable(inst, entArrayId, entnum);
        if (entnumId)
        {
            entId = FindObject(inst, entnumId);
            if (!entId
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    3997,
                    0,
                    "%s",
                    "entId"))
            {
                __debugbreak();
            }
            entryValue = &gScrVarGlob[inst].variableList[entId + 1];
            if ((entryValue->w.status & 0x1F) != 0x13
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4000,
                    0,
                    "%s",
                    "(entryValue->w.type & VAR_MASK) == VAR_ENTITY"))
            {
                __debugbreak();
            }
            if ((entryValue->u.o.u.entnum & 0x3FFF) != entnum - 1536 * ((int)entryValue->u.o.u.entnum >> 14)
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4002,
                    0,
                    "%s",
                    "SCR_GET_ENTITY_FROM_ENTCLIENT(entryValue->u.o.u.entnum) == entnum - (SCR_GET_CLIENT_FROM_ENTCLIENT(entryVa"
                    "lue->u.o.u.entnum) * MAX_LOCAL_CENTITIES)"))
            {
                __debugbreak();
            }
            if (entryValue->w.status >> 8 != classnum
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4003,
                    0,
                    "%s",
                    "(entryValue->w.classnum >> VAR_NAME_BITS) == classnum"))
            {
                __debugbreak();
            }
            entryValue->w.status &= 0xFFFFFFE0;
            entryValue->w.status |= 0x12u;
            AddRefToObject(inst, entId);
            entryValue->u.o.u.nextEntId = gScrVarPub[inst].freeEntList;
            if (gScrVarDebugPub[inst])
                --gScrVarDebugPub[inst]->extRefCount[gScrVarPub[inst].freeEntList];
            gScrVarPub[inst].freeEntList = entId;
            if (gScrVarDebugPub[inst])
                ++gScrVarDebugPub[inst]->extRefCount[gScrVarPub[inst].freeEntList];
            RemoveArrayVariable(inst, entArrayId, entnum);
        }
    }
}

void __cdecl Scr_FreeEntityList(scriptInstance_t inst)
{
    VariableValueInternal *entryValue; // [esp+0h] [ebp-8h]
    unsigned int entId; // [esp+4h] [ebp-4h]

    if (gScrVarDebugPub[inst])
        --gScrVarDebugPub[inst]->extRefCount[gScrVarPub[inst].freeEntList];
    while (gScrVarPub[inst].freeEntList)
    {
        entId = gScrVarPub[inst].freeEntList;
        entryValue = &gScrVarGlob[inst].variableList[entId + 1];
        gScrVarPub[inst].freeEntList = entryValue->u.o.u.nextEntId;
        entryValue->u.o.u.nextEntId = 0;
        Scr_CancelNotifyList(inst, entId);
        if (entryValue->nextSibling)
            ClearObjectInternal(inst, entId);
        RemoveRefToObject(inst, entId);
    }
}

void __cdecl Scr_FreeObjects(scriptInstance_t inst)
{
    VariableValueInternal *entryValue; // [esp+0h] [ebp-8h]
    unsigned int id; // [esp+4h] [ebp-4h]

    for ( id = 1; id < 0x7FFE; ++id )
    {
        entryValue = &gScrVarGlob[inst].variableList[id + 1];
        if ( (entryValue->w.status & 0x60) != 0
            && ((entryValue->w.status & 0x1F) == 0x11 || (entryValue->w.status & 0x1F) == 0x12) )
        {
            Scr_CancelNotifyList(inst, id);
            ClearObject(inst, id);
        }
    }
}

void __cdecl Scr_SetClassMap(scriptInstance_t inst, unsigned int classnum)
{
    if ( gScrClassMap[inst][classnum].entArrayId
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4077,
                    0,
                    "%s",
                    "!gScrClassMap[inst][classnum].entArrayId") )
    {
        __debugbreak();
    }
    if ( gScrClassMap[inst][classnum].id
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4078,
                    0,
                    "%s",
                    "!gScrClassMap[inst][classnum].id") )
    {
        __debugbreak();
    }
    gScrClassMap[inst][classnum].entArrayId = Scr_AllocArray(inst);
    if ( gScrVarDebugPub[inst] )
        ++gScrVarDebugPub[inst]->extRefCount[gScrClassMap[inst][classnum].entArrayId];
    gScrClassMap[inst][classnum].id = Scr_AllocArray(inst);
    if ( gScrVarDebugPub[inst] )
        ++gScrVarDebugPub[inst]->extRefCount[gScrClassMap[inst][classnum].id];
}

void __cdecl Scr_RemoveClassMap(scriptInstance_t inst, unsigned int classnum)
{
    if (gScrVarPub[inst].bInited)
    {
        if (gScrClassMap[inst][classnum].entArrayId)
        {
            if (gScrVarDebugPub[inst])
                --gScrVarDebugPub[inst]->extRefCount[gScrClassMap[inst][classnum].entArrayId];
            RemoveRefToObject(inst, gScrClassMap[inst][classnum].entArrayId);
            gScrClassMap[inst][classnum].entArrayId = 0;
        }
        if (gScrClassMap[inst][classnum].id)
        {
            if (gScrVarDebugPub[inst])
                --gScrVarDebugPub[inst]->extRefCount[gScrClassMap[inst][classnum].id];
            RemoveRefToObject(inst, gScrClassMap[inst][classnum].id);
            gScrClassMap[inst][classnum].id = 0;
        }
    }
}

void __cdecl Scr_AddClassField(unsigned int classnum, char *name, unsigned int offset, scriptInstance_t inst)
{
    unsigned int str; // [esp+0h] [ebp-14h]
    unsigned int stra; // [esp+0h] [ebp-14h]
    unsigned int classId; // [esp+4h] [ebp-10h]
    VariableValueInternal *entryValue; // [esp+8h] [ebp-Ch]
    VariableValueInternal *entryValuea; // [esp+8h] [ebp-Ch]
    unsigned int fieldId; // [esp+Ch] [ebp-8h]
    const char *namePos; // [esp+10h] [ebp-4h]

    if ( offset >= 0x10000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4133,
                    0,
                    "%s",
                    "offset < (1 << 16)") )
    {
        __debugbreak();
    }
    for ( namePos = name; *namePos; ++namePos )
    {
        if ( *namePos >= 65
            && *namePos <= 90
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        4137,
                        0,
                        "%s\n\t(name) = %s",
                        "(*namePos < 'A' || *namePos > 'Z')",
                        name) )
        {
            __debugbreak();
        }
    }
    classId = gScrClassMap[inst][classnum].id;
    str = SL_GetCanonicalString(inst, name);
    if ( FindArrayVariable(inst, classId, str)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4144,
                    0,
                    "%s",
                    "!FindArrayVariable( inst, classId, (unsigned)str )") )
    {
        __debugbreak();
    }
    entryValue = &gScrVarGlob[inst].variableList[GetNewArrayVariable(inst, classId, str) + 0x8000];
    entryValue->w.status &= 0xFFFFFFE0;
    entryValue->w.status |= 6u;
    entryValue->u.next = (unsigned __int16)offset;
    stra = SL_GetString_(inst, name, 0, 16);
    if ( FindVariable(inst, classId, stra)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4155,
                    0,
                    "%s",
                    "!FindVariable( inst, classId, str )") )
    {
        __debugbreak();
    }
    fieldId = GetNewVariable(inst, classId, stra);
    SL_RemoveRefToString(inst, stra);
    entryValuea = &gScrVarGlob[inst].variableList[fieldId + 0x8000];
    entryValuea->w.status &= 0xFFFFFFE0;
    entryValuea->w.status |= 6u;
    entryValuea->u.next = (unsigned __int16)offset;
}

int __cdecl Scr_GetOffset(unsigned int classnum, char *name, scriptInstance_t inst)
{
    unsigned int v3; // eax
    unsigned int classId; // [esp+0h] [ebp-8h]
    unsigned int fieldId; // [esp+4h] [ebp-4h]

    classId = gScrClassMap[inst][classnum].id;
    v3 = SL_ConvertFromString(inst, name);
    fieldId = FindVariable(inst, classId, v3);
    if ( fieldId )
        return gScrVarGlob[inst].variableList[fieldId + 0x8000].u.u.entityOffset;
    else
        return -1;
}

unsigned int __cdecl FindEntityId(scriptInstance_t inst, unsigned int entnum, unsigned int classnum, int clientNum)
{
    unsigned int entArrayId; // [esp+0h] [ebp-Ch]
    VariableValueInternal *entryValue; // [esp+4h] [ebp-8h]
    unsigned int id; // [esp+8h] [ebp-4h]

    if ( clientNum && inst == SCRIPTINSTANCE_CLIENT )
        entnum += 1536 * clientNum;
    if ( entnum >= 0x10000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4192,
                    0,
                    "%s",
                    "(unsigned)entnum < (1 << 16)") )
    {
        __debugbreak();
    }
    entArrayId = gScrClassMap[inst][classnum].entArrayId;
    if ( !entArrayId
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4195,
                    0,
                    "%s",
                    "entArrayId") )
    {
        __debugbreak();
    }
    id = FindArrayVariable(inst, entArrayId, entnum);
    if ( !id )
        return 0;
    entryValue = &gScrVarGlob[inst].variableList[id + 0x8000];
    if ( (entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4202,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4203,
                    0,
                    "%s",
                    "(entryValue->w.type & VAR_MASK) == VAR_POINTER") )
    {
        __debugbreak();
    }
    if ( !entryValue->u.next
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4204,
                    0,
                    "%s",
                    "entryValue->u.u.pointerValue") )
    {
        __debugbreak();
    }
    return entryValue->u.next;
}

unsigned int __cdecl Scr_GetEntityId(scriptInstance_t inst, unsigned int entnum, unsigned int classnum, int clientNum)
{
    unsigned int entArrayId; // [esp+0h] [ebp-14h]
    unsigned __int16 actualEntNum; // [esp+4h] [ebp-10h]
    VariableValueInternal *entryValue; // [esp+8h] [ebp-Ch]
    unsigned int entId; // [esp+Ch] [ebp-8h]
    unsigned int id; // [esp+10h] [ebp-4h]

    actualEntNum = entnum;
    if ( clientNum && inst == SCRIPTINSTANCE_CLIENT )
        entnum += 1536 * clientNum;
    if ( entnum >= 0x10000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4224,
                    0,
                    "%s",
                    "(unsigned)entnum < (1 << 16)") )
    {
        __debugbreak();
    }
    entArrayId = gScrClassMap[inst][classnum].entArrayId;
    if ( !entArrayId
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4227,
                    0,
                    "%s",
                    "entArrayId") )
    {
        __debugbreak();
    }
    id = GetArrayVariable(inst, entArrayId, entnum);
    if ( !id
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp", 4230, 0, "%s", "id") )
    {
        __debugbreak();
    }
    entryValue = &gScrVarGlob[inst].variableList[id + 0x8000];
    if ( (entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4232,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) != 0 )
    {
        if ( (entryValue->w.status & 0x1F) != 1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        4236,
                        0,
                        "%s",
                        "(entryValue->w.type & VAR_MASK) == VAR_POINTER") )
        {
            __debugbreak();
        }
        return entryValue->u.next;
    }
    else
    {
        entId = AllocEntity(inst, classnum, actualEntNum, clientNum);
        if ( (entryValue->w.status & 0x1F) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        4242,
                        0,
                        "%s",
                        "!(entryValue->w.type & VAR_MASK)") )
        {
            __debugbreak();
        }
        entryValue->w.status |= 1u;
        entryValue->u.next = entId;
        return entId;
    }
}

unsigned int __cdecl AllocEntity(
                scriptInstance_t inst,
                unsigned int classnum,
                unsigned __int16 entnum,
                unsigned __int8 clientNum)
{
    VariableValueInternal *entryValue; // [esp+0h] [ebp-8h]
    unsigned int id; // [esp+4h] [ebp-4h]

    id = AllocVariable(inst);
    entryValue = &gScrVarGlob[inst].variableList[id + 1];
    entryValue->w.status = 96;
    if ( (entryValue->w.status & 0x1F) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2024,
                    0,
                    "%s",
                    "!(entryValue->w.type & VAR_MASK)") )
    {
        __debugbreak();
    }
    entryValue->w.status |= 0x13u;
    if ( (entryValue->w.status & 0xFFFFFF00) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2026,
                    0,
                    "%s",
                    "!(entryValue->w.classnum & VAR_NAME_HIGH_MASK)") )
    {
        __debugbreak();
    }
    entryValue->w.status |= classnum << 8;
    entryValue->u.o.refCount = 0;
    entryValue->u.o.u.entnum = entnum;
    entryValue->u.o.u.entnum |= clientNum << 14;
    return id;
}

void __cdecl Scr_EvalArray(scriptInstance_t inst, VariableValue *value, VariableValue *index)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    unsigned int ArrayIndex; // eax
    const char *v9; // eax
    int type; // [esp+10h] [ebp-34h]
    char c[4]; // [esp+38h] [ebp-Ch] BYREF
    const char *s; // [esp+3Ch] [ebp-8h]
    VariableValueInternal *entryValue; // [esp+40h] [ebp-4h]

    if (value == index
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            4287,
            0,
            "%s",
            "value != index"))
    {
        __debugbreak();
    }
    type = value->type;
    switch (type)
    {
    case 1:
        entryValue = &gScrVarGlob[inst].variableList[value->u.intValue + 1];
        if ((entryValue->w.status & 0x60) == 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                4339,
                0,
                "%s",
                "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE"))
        {
            __debugbreak();
        }
        if ((entryValue->w.status & 0x60) == 0
            && !Assert_MyHandler(
                "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                323,
                0,
                "%s",
                "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE"))
        {
            __debugbreak();
        }
        if ((entryValue->w.status & 0x1F) < 0xD
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                4340,
                0,
                "%s",
                "IsObject( entryValue )"))
        {
            __debugbreak();
        }
        if ((entryValue->w.status & 0x1F) == 0x14)
        {
            ArrayIndex = Scr_FindArrayIndex(inst, value->u.intValue, index);
            *index = Scr_EvalVariable(inst, ArrayIndex);
            RemoveRefToObject(inst, value->u.intValue);
        }
        else
        {
            gScrVarPub[inst].error_index = 1;
            v7 = va("%s is not an array", var_typename[entryValue->w.status & 0x1F]);
            Scr_Error(inst, v7, 0);
        }
        break;
    case 2:
        if (index->type == 6)
        {
            if (index->u.intValue < 0
                || (s = SL_ConvertToString(value->u.intValue, inst), index->u.intValue >= (signed int)strlen(s)))
            {
                v3 = va("string index %d out of range", index->u.intValue);
                Scr_Error(inst, v3, 0);
            }
            else
            {
                index->type = 2;
                c[0] = s[index->u.intValue];
                c[1] = 0;
                index->u.intValue = SL_GetStringOfSize(inst, c, 0, 2u, 15);
                SL_RemoveRefToString(inst, value->u.intValue);
            }
        }
        else
        {
            v4 = va("%s is not a string index", var_typename[index->type]);
            Scr_Error(inst, v4, 0);
        }
        break;
    case 4:
        if (index->type == 6)
        {
            if (index->u.intValue >= 3u)
            {
                v5 = va("vector index %d out of range", index->u.intValue);
                Scr_Error(inst, v5, 0);
            }
            else
            {
                index->type = 5;
                index->u.floatValue = *(float *)(value->u.intValue + 4 * index->u.intValue);
                RemoveRefToVector(inst, value->u.vectorValue);
            }
        }
        else
        {
            v6 = va("%s is not a vector index", var_typename[index->type]);
            Scr_Error(inst, v6, 0);
        }
        break;
    default:
        if (value->type == 10
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                4354,
                0,
                "%s",
                "value->type != VAR_STACK"))
        {
            __debugbreak();
        }
        gScrVarPub[inst].error_index = 1;
        v9 = va("%s is not an array, string, or vector", var_typename[value->type]);
        Scr_Error(inst, v9, 0);
        break;
    }
}

unsigned int __cdecl Scr_FindArrayIndex(scriptInstance_t inst, unsigned int parentId, VariableValue *index)
{
    const char *v4; // eax
    const char *v5; // eax
    unsigned int id; // [esp+8h] [ebp-4h]

    if ( index->type == 6 )
    {
        if ( IsValidArrayIndex(inst, index->u.intValue) )
        {
            return FindArrayVariable(inst, parentId, index->u.intValue);
        }
        else
        {
            v4 = va("array index %d out of range", index->u.intValue);
            Scr_Error(inst, v4, 0);
            AddRefToObject(inst, parentId);
            return 0;
        }
    }
    else if ( index->type == 2 )
    {
        id = FindVariable(inst, parentId, index->u.intValue);
        SL_RemoveRefToString(inst, index->u.intValue);
        return id;
    }
    else
    {
        v5 = va("%s is not an array index", var_typename[index->type]);
        Scr_Error(inst, v5, 0);
        AddRefToObject(inst, parentId);
        return 0;
    }
}

unsigned int __cdecl Scr_EvalArrayRef(scriptInstance_t inst, unsigned int parentId)
{
    const char *v3; // eax
    const char *v4; // eax
    VariableValueInternal *parentValue; // [esp+24h] [ebp-1Ch]
    VariableValueInternal *entValue; // [esp+28h] [ebp-18h]
    VariableValue varValue; // [esp+2Ch] [ebp-14h]
    VariableValueInternal *entryValue; // [esp+34h] [ebp-Ch]
    unsigned int fieldId; // [esp+38h] [ebp-8h]
    VariableUnion id; // [esp+3Ch] [ebp-4h]

    if (parentId)
    {
        parentValue = &gScrVarGlob[inst].variableList[parentId + 0x8000];
        if ((parentValue->w.status & 0x60) == 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                4405,
                0,
                "%s",
                "(parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE"))
        {
            __debugbreak();
        }
        varValue.type = parentValue->w.status & 0x1F;
        if (varValue.type)
        {
            varValue.u.intValue = parentValue->u.u.intValue;
        add_array:
            if (varValue.type == 1)
            {
                entryValue = &gScrVarGlob[inst].variableList[varValue.u.intValue + 1];
                if ((entryValue->w.status & 0x60) == 0
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        4446,
                        0,
                        "%s",
                        "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE"))
                {
                    __debugbreak();
                }
                if ((entryValue->w.status & 0x60) == 0
                    && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                        323,
                        0,
                        "%s",
                        "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE"))
                {
                    __debugbreak();
                }
                if ((entryValue->w.status & 0x1F) < 0xD
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        4447,
                        0,
                        "%s",
                        "IsObject( entryValue )"))
                {
                    __debugbreak();
                }
                if ((entryValue->w.status & 0x1F) == 0x14)
                {
                    if (entryValue->u.o.refCount)
                    {
                        id.intValue = varValue.u.intValue;
                        RemoveRefToObject(inst, varValue.u.stringValue);
                        varValue.u.intValue = Scr_AllocArray(inst);
                        CopyArray(inst, id.stringValue, varValue.u.stringValue);
                        iassert(parentValue);
                        parentValue->u.next = varValue.u.intValue;
                    }
                    return varValue.u.pointerValue;
                }
                else
                {
                    gScrVarPub[inst].error_index = 1;
                    v4 = va("%s is not an array", var_typename[entryValue->w.status & 0x1F]);
                    Scr_Error(inst, v4, 0);
                    return 0;
                }
            }
            else
            {
                if (varValue.type == 10
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        4424,
                        0,
                        "%s",
                        "varValue.type != VAR_STACK"))
                {
                    __debugbreak();
                }
                gScrVarPub[inst].error_index = 1;
                if (varValue.type == 2)
                {
                    Scr_Error(inst, "string characters cannot be individually changed", 0);
                    return 0;
                }
                else
                {
                    if (varValue.type == 4)
                    {
                        Scr_Error(inst, "vector components cannot be individually changed", 0);
                    }
                    else
                    {
                        v3 = va("%s is not an array", var_typename[varValue.type]);
                        Scr_Error(inst, v3, 0);
                    }
                    return 0;
                }
            }
        }
    }
    else
    {
        entValue = &gScrVarGlob[inst].variableList[gScrVarPub[inst].entId + 1];
        if ((entValue->w.status & 0x1F) != 0x13
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                4373,
                0,
                "%s",
                "(entValue->w.type & VAR_MASK) == VAR_ENTITY"))
        {
            __debugbreak();
        }
        if (entValue->w.status >> 8 >= 5
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                4374,
                0,
                "%s",
                "(entValue->w.classnum >> VAR_NAME_BITS) < CLASS_NUM_COUNT"))
        {
            __debugbreak();
        }
        fieldId = FindArrayVariable(inst, gScrClassMap[inst][entValue->w.status >> 8].id, gScrVarPub[inst].entFieldName);
        if (fieldId)
        {
            varValue = GetEntityFieldValue(
                inst,
                entValue->w.status >> 8,
                entValue->u.o.u.entnum & 0x3FFF,
                (int)entValue->u.o.u.entnum >> 14,
                gScrVarGlob[inst].variableList[fieldId + 0x8000].u.next);
            if (varValue.type)
            {
                if (varValue.type == 1 && !gScrVarGlob[inst].variableList[varValue.u.intValue + 1].u.o.refCount)
                {
                    RemoveRefToValue(inst, 1, varValue.u);
                    gScrVarPub[inst].error_index = 1;
                    Scr_Error(inst, "read-only array cannot be changed", 0);
                    return 0;
                }
                RemoveRefToValue(inst, varValue.type, varValue.u);
                if (varValue.type == 1
                    && gScrVarGlob[inst].variableList[varValue.u.intValue + 1].u.o.refCount
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        4393,
                        0,
                        "%s",
                        "(varValue.type != VAR_POINTER) || !gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ varVa"
                        "lue.u.pointerValue].u.o.refCount"))
                {
                    __debugbreak();
                }
                parentValue = 0;
                goto add_array;
            }
        }
        parentValue = &gScrVarGlob[inst].variableList[GetNewVariable(
            inst,
            gScrVarPub[inst].entId,
            gScrVarPub[inst].entFieldName)
            + 0x8000];
    }
    if ((parentValue->w.status & 0x1F) != 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            4412,
            0,
            "%s",
            "!(parentValue->w.type & VAR_MASK)"))
    {
        __debugbreak();
    }
    parentValue->w.status |= 1u;
    parentValue->u.next = Scr_AllocArray(inst);
    return parentValue->u.u.pointerValue;
}

void __cdecl CopyArray(scriptInstance_t inst, unsigned int parentId, unsigned int newParentId)
{
    ObjectInfo_u u; // edx
    unsigned int nextSibling; // [esp+18h] [ebp-20h]
    VariableValueInternal *parentValue; // [esp+1Ch] [ebp-1Ch]
    VariableValueInternal *entryValue; // [esp+24h] [ebp-14h]
    int type; // [esp+28h] [ebp-10h]
    VariableValueInternal *newEntryValue; // [esp+2Ch] [ebp-Ch]
    unsigned int id; // [esp+34h] [ebp-4h]

    parentValue = &gScrVarGlob[inst].variableList[parentId + 1];
    if ( (parentValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (parentValue->w.status & 0x1F) < 0xD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2662,
                    0,
                    "%s",
                    "IsObject( parentValue )") )
    {
        __debugbreak();
    }
    if ( (parentValue->w.status & 0x1F) != 0x14
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    2663,
                    0,
                    "%s",
                    "(parentValue->w.type & VAR_MASK) == VAR_ARRAY") )
    {
        __debugbreak();
    }
    id = gScrVarGlob[inst].variableList[parentId + 1].nextSibling;
    if ( id )
    {
        while ( 1 )
        {
            entryValue = &gScrVarGlob[inst].variableList[id + 0x8000];
            type = entryValue->w.status & 0x1F;
            if ( (entryValue->w.status & 0x60) == 0
                && !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                            323,
                            0,
                            "%s",
                            "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
            {
                __debugbreak();
            }
            if ( (entryValue->w.status & 0x1F) >= 0xD
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                            2674,
                            0,
                            "%s",
                            "!IsObject( entryValue )") )
            {
                __debugbreak();
            }
            newEntryValue = &gScrVarGlob[inst].variableList[gScrVarGlob[inst].variableList[GetVariableIndexInternal(
                                                                                                                                                                             inst,
                                                                                                                                                                             newParentId,
                                                                                                                                                                             entryValue->w.status >> 8)
                                                                                                                                                                     + 0x8000].hash.id
                                                                                                        + 0x8000];
            if ( (newEntryValue->w.status & 0x1F) != 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                            2679,
                            0,
                            "%s",
                            "(newEntryValue->w.type & VAR_MASK) == VAR_UNDEFINED") )
            {
                __debugbreak();
            }
            if ( (newEntryValue->w.status & 0x1F) != 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                            2681,
                            0,
                            "%s",
                            "!(newEntryValue->w.type & VAR_MASK)") )
            {
                __debugbreak();
            }
            newEntryValue->w.status |= type;
            if ( type == 1 )
            {
                if ( (gScrVarGlob[inst].variableList[entryValue->u.next + 1].w.status & 0x1F) == 0x14 )
                {
                    newEntryValue->u.next = Scr_AllocArray(inst);
                    CopyArray(inst, entryValue->u.next, newEntryValue->u.next);
                }
                else
                {
                    newEntryValue->u.next = entryValue->u.next;
                    AddRefToObject(inst, entryValue->u.next);
                }
            }
            else
            {
                if ( type == 10
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                                2686,
                                0,
                                "%s",
                                "type != VAR_STACK") )
                {
                    __debugbreak();
                }
                u = entryValue->u.o.u;
                newEntryValue->u.next = entryValue->u.next;
                newEntryValue->u.o.u = u;
                AddRefToValue(inst, type, entryValue->u.u);
            }
            nextSibling = gScrVarGlob[inst].variableList[id + 0x8000].nextSibling;
            if ( !nextSibling )
                break;
            id = gScrVarGlob[inst].variableList[nextSibling + 0x8000].hash.id;
            if ( !id
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp", 2710, 0, "%s", "id") )
            {
                __debugbreak();
            }
        }
    }
}

void __cdecl ClearArray(scriptInstance_t inst, unsigned int parentId, VariableValue *value)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    VariableValueInternal *parentValue; // [esp+18h] [ebp-1Ch]
    VariableValueInternal *entValue; // [esp+1Ch] [ebp-18h]
    VariableValue varValue; // [esp+20h] [ebp-14h]
    VariableValueInternal *entryValue; // [esp+28h] [ebp-Ch]
    unsigned int fieldId; // [esp+2Ch] [ebp-8h]
    VariableUnion id; // [esp+30h] [ebp-4h]

    if (parentId)
    {
        parentValue = &gScrVarGlob[inst].variableList[parentId + 0x8000];
        if ((parentValue->w.status & 0x60) == 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                4515,
                0,
                "%s",
                "(parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE"))
        {
            __debugbreak();
        }
        varValue.type = parentValue->w.status & 0x1F;
        varValue.u.intValue = parentValue->u.u.intValue;
    }
    else
    {
        entValue = &gScrVarGlob[inst].variableList[gScrVarPub[inst].entId + 1];
        if ((entValue->w.status & 0x1F) != 0x13
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                4483,
                0,
                "%s",
                "(entValue->w.type & VAR_MASK) == VAR_ENTITY"))
        {
            __debugbreak();
        }
        if (entValue->w.status >> 8 >= 5
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                4484,
                0,
                "%s",
                "(entValue->w.classnum >> VAR_NAME_BITS) < CLASS_NUM_COUNT"))
        {
            __debugbreak();
        }
        fieldId = FindArrayVariable(inst, gScrClassMap[inst][entValue->w.status >> 8].id, gScrVarPub[inst].entFieldName);
        if (!fieldId
            || (varValue = GetEntityFieldValue(
                inst,
                entValue->w.status >> 8,
                entValue->u.o.u.entnum & 0x3FFF,
                (int)entValue->u.o.u.entnum >> 14,
                gScrVarGlob[inst].variableList[fieldId + 0x8000].u.next),
                !varValue.type))
        {
            varValue.type = 0;
        error_0:
            gScrVarPub[inst].error_index = 1;
            v3 = va("%s is not an array", var_typename[varValue.type]);
            Scr_Error(inst, v3, 0);
            return;
        }
        if (varValue.type == 1 && !gScrVarGlob[inst].variableList[varValue.u.intValue + 1].u.o.refCount)
        {
            RemoveRefToValue(inst, 1, varValue.u);
            gScrVarPub[inst].error_index = 1;
            Scr_Error(inst, "read-only array cannot be changed", 0);
            return;
        }
        RemoveRefToValue(inst, varValue.type, varValue.u);
        if (varValue.type == 1
            && gScrVarGlob[inst].variableList[varValue.u.intValue + 1].u.o.refCount
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                4503,
                0,
                "%s",
                "(varValue.type != VAR_POINTER) || !gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ varValue."
                "u.pointerValue].u.o.refCount"))
        {
            __debugbreak();
        }
        parentValue = 0;
    }
    if (varValue.type != 1)
    {
        if (varValue.type == 10
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                4524,
                0,
                "%s",
                "varValue.type != VAR_STACK"))
        {
            __debugbreak();
        }
        goto error_0;
    }
    entryValue = &gScrVarGlob[inst].variableList[varValue.u.intValue + 1];
    if ((entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            4534,
            0,
            "%s",
            "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE"))
    {
        __debugbreak();
    }
    if ((entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
            323,
            0,
            "%s",
            "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE"))
    {
        __debugbreak();
    }
    if ((entryValue->w.status & 0x1F) < 0xD
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            4535,
            0,
            "%s",
            "IsObject( entryValue )"))
    {
        __debugbreak();
    }
    if ((entryValue->w.status & 0x1F) == 0x14)
    {
        if (entryValue->u.o.refCount)
        {
            id.intValue = (int)varValue.u.pointerValue;
            RemoveRefToObject(inst, varValue.u.stringValue);
            varValue.u.intValue = Scr_AllocArray(inst);
            CopyArray(inst, id.stringValue, varValue.u.stringValue);
            if (!parentValue
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4550,
                    0,
                    "%s",
                    "parentValue"))
            {
                __debugbreak();
            }
            parentValue->u.next = varValue.u.intValue;
        }
        if (value->type == 6)
        {
            if (IsValidArrayIndex(inst, value->u.intValue))
            {
                SafeRemoveArrayVariable(inst, varValue.u.stringValue, value->u.intValue);
            }
            else
            {
                v5 = va("array index %d out of range", value->u.intValue);
                Scr_Error(inst, v5, 0);
            }
        }
        else if (value->type == 2)
        {
            SL_RemoveRefToString(inst, value->u.intValue);
            SafeRemoveVariable(inst, varValue.u.stringValue, value->u.intValue);
        }
        else
        {
            v6 = va("%s is not an array index", var_typename[value->type]);
            Scr_Error(inst, v6, 0);
        }
    }
    else
    {
        gScrVarPub[inst].error_index = 1;
        v4 = va("%s is not an array", var_typename[entryValue->w.status & 0x1F]);
        Scr_Error(inst, v4, 0);
    }
}

void __cdecl SafeRemoveArrayVariable(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue)
{
    if (!IsValidArrayIndex(inst, unsignedValue)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            2618,
            0,
            "%s",
            "IsValidArrayIndex( inst, unsignedValue )"))
    {
        __debugbreak();
    }
    SafeRemoveVariable(inst, parentId, (unsignedValue + 0x800000) & 0xFFFFFF);
}

void __cdecl SetEmptyArray(scriptInstance_t inst, unsigned int parentId)
{
    VariableValue tempValue; // [esp+Ch] [ebp-8h] BYREF

    tempValue.type = 1;
    tempValue.u.intValue = Scr_AllocArray(inst);
    SetVariableValue(inst, parentId, &tempValue);
}

void __cdecl Scr_AddArrayKeys(unsigned int parentId, scriptInstance_t inst)
{
    VariableValue indexValue; // [esp+2Ch] [ebp-14h]
    VariableValueInternal *entryValue; // [esp+38h] [ebp-8h]
    unsigned int id; // [esp+3Ch] [ebp-4h]

    if ( !parentId
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4593,
                    0,
                    "%s",
                    "parentId") )
    {
        __debugbreak();
    }
    if ( GetObjectType(inst, parentId) != 20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4594,
                    0,
                    "%s",
                    "GetObjectType( inst, parentId ) == VAR_ARRAY") )
    {
        __debugbreak();
    }
    Scr_MakeArray(inst);
    for ( id = FindFirstSibling(inst, parentId); id; id = FindNextSibling(inst, id) )
    {
        entryValue = &gScrVarGlob[inst].variableList[id + 0x8000];
        if ( ((entryValue->w.status & 0x60) == 0 || (entryValue->w.status & 0x60) == 0x60)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        4602,
                        0,
                        "%s",
                        "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE && (entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_EXTERNAL") )
        {
            __debugbreak();
        }
        if ( (entryValue->w.status & 0x60) == 0
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                        323,
                        0,
                        "%s",
                        "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
        {
            __debugbreak();
        }
        if ( (entryValue->w.status & 0x1F) >= 0xD
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        4603,
                        0,
                        "%s",
                        "!IsObject( entryValue )") )
        {
            __debugbreak();
        }
        indexValue = Scr_GetArrayIndexValue(inst, entryValue->w.status >> 8);
        if ( indexValue.type == 2 )
        {
            Scr_AddConstString(indexValue.u.stringValue, inst);
        }
        else if ( indexValue.type == 6 )
        {
            Scr_AddInt(indexValue.u.intValue, inst);
        }
        else if ( !Assert_MyHandler(
                                 "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                                 4619,
                                 1,
                                 "bad case") )
        {
            __debugbreak();
        }
        Scr_AddArray(inst);
    }
}

scr_entref_t __cdecl Scr_GetEntityIdRef(scriptInstance_t inst, unsigned int entId)
{
    //int v3; // eax
    //int entref; // [esp+0h] [ebp-Ch]
    //VariableValueInternal *entValue; // [esp+8h] [ebp-4h]
    //
    //entValue = &gScrVarGlob[inst].variableList[entId + 1];
    //if ( (entValue->w.status & 0x1F) != 0x13
    //    && !Assert_MyHandler(
    //                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
    //                4635,
    //                0,
    //                "%s",
    //                "(entValue->w.type & VAR_MASK) == VAR_ENTITY") )
    //{
    //    __debugbreak();
    //}
    //if ( entValue->w.status >> 8 >= 5
    //    && !Assert_MyHandler(
    //                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
    //                4636,
    //                0,
    //                "%s",
    //                "(entValue->w.name >> VAR_NAME_BITS) < CLASS_NUM_COUNT") )
    //{
    //    __debugbreak();
    //}
    //LOWORD(entref) = entValue->u.o.u.entnum & 0x3FFF;
    //HIWORD(entref) = entValue->w.status >> 8;
    //v3 = (int)entValue->u.o.u.entnum >> 14;
    //*(unsigned int *)&retstr->entnum = entref;
    //retstr->client = v3;
    //return retstr;

    scr_entref_t entref; // [esp+0h] [ebp-8h]
    VariableValueInternal *entValue; // [esp+4h] [ebp-4h]

    entValue = &gScrVarGlob[inst].variableList[entId + 1];

    iassert((entValue->w.type & VAR_MASK) == VAR_ENTITY);
    iassert((entValue->w.name >> VAR_NAME_BITS) < CLASS_NUM_COUNT);

    entref.entnum = entValue->u.o.u.entnum & 0x3FFF;
    entref.classnum = entValue->w.classnum >> VAR_NAME_BITS;
    entref.client = entValue->u.o.u.entnum >> 14;

    return entref;
}

void __cdecl Scr_CopyEntityNum(
                unsigned int fromEntnum,
                unsigned int toEntnum,
                unsigned int classnum,
                scriptInstance_t inst)
{
    unsigned int EntityId; // eax
    unsigned int fromEntId; // [esp+4h] [ebp-4h]

    fromEntId = FindEntityId(inst, fromEntnum, classnum, 0);
    if ( fromEntId && FindFirstSibling(inst, fromEntId) )
    {
        if ( FindEntityId(inst, toEntnum, classnum, 0) )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                            4708,
                            0,
                            "%s",
                            "!FindEntityId( inst, toEntnum, classnum )") )
                __debugbreak();
        }
        EntityId = Scr_GetEntityId(inst, toEntnum, classnum, 0);
        CopyEntity(inst, fromEntId, EntityId);
    }
}

void __cdecl CopyEntity(scriptInstance_t inst, unsigned int parentId, unsigned int newParentId)
{
    VariableValueInternal *v3; // [esp+4Ch] [ebp-1Ch]
    VariableValueInternal *parentValue; // [esp+50h] [ebp-18h]
    unsigned int name; // [esp+54h] [ebp-14h]
    VariableValueInternal *entryValue; // [esp+58h] [ebp-10h]
    VariableValueInternal *newEntryValue; // [esp+5Ch] [ebp-Ch]
    int type; // [esp+60h] [ebp-8h]
    unsigned int id; // [esp+64h] [ebp-4h]

    iassert(parentId);
    iassert(newParentId);

    parentValue = &gScrVarGlob[inst].variableList[parentId + VARIABLELIST_PARENT_BEGIN];

    if ( (parentValue->w.status & 0x60) != 0x60
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4659,
                    0,
                    "%s",
                    "(parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL") )
    {
        __debugbreak();
    }
    if ( (parentValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (parentValue->w.status & 0x1F) < 0xD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4660,
                    0,
                    "%s",
                    "IsObject( parentValue )") )
    {
        __debugbreak();
    }
    if ( (parentValue->w.status & 0x1F) != 0x13
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4661,
                    0,
                    "%s",
                    "(parentValue->w.type & VAR_MASK) == VAR_ENTITY") )
    {
        __debugbreak();
    }
    if ( (gScrVarGlob[inst].variableList[newParentId + 1].w.status & 0x60) != 0x60
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4663,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ newParentId].w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL") )
    {
        __debugbreak();
    }
    v3 = &gScrVarGlob[inst].variableList[newParentId + 1];
    if ( (v3->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (v3->w.status & 0x1F) < 0xD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4664,
                    0,
                    "%s",
                    "IsObject( &gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ newParentId] )") )
    {
        __debugbreak();
    }
    if ( (gScrVarGlob[inst].variableList[newParentId + 1].w.status & 0x1F) != 0x13
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4665,
                    0,
                    "%s",
                    "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ newParentId].w.type & VAR_MASK) == VAR_ENTITY") )
    {
        __debugbreak();
    }
    for ( id = FindFirstSibling(inst, parentId); id; id = FindNextSibling(inst, id) )
    {
        entryValue = &gScrVarGlob[inst].variableList[id + 0x8000];
        if ( ((entryValue->w.status & 0x60) == 0 || (entryValue->w.status & 0x60) == 0x60)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        4671,
                        0,
                        "%s",
                        "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE && (entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_EXTERNAL") )
        {
            __debugbreak();
        }
        if ( (entryValue->w.status & 0x60) == 0
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                        323,
                        0,
                        "%s",
                        "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
        {
            __debugbreak();
        }
        if ( (entryValue->w.status & 0x1F) >= 0xD
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        4672,
                        0,
                        "%s",
                        "!IsObject( entryValue )") )
        {
            __debugbreak();
        }
        name = entryValue->w.status >> 8;
        if ( name == 98303
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        4677,
                        0,
                        "%s",
                        "name != OBJECT_STACK") )
        {
            __debugbreak();
        }
        if ( name != 98302 )
        {
            if ( FindVariableIndexInternal(inst, newParentId, name)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                            4681,
                            0,
                            "%s",
                            "!FindVariableIndexInternal( inst, newParentId, name )") )
            {
                __debugbreak();
            }

            newEntryValue = &gScrVarGlob[inst].variableList[GetVariable(inst, newParentId, name) + 0x8000];

            if ( ((newEntryValue->w.status & 0x60) == 0 || (newEntryValue->w.status & 0x60) == 0x60)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                            4684,
                            0,
                            "%s",
                            "(newEntryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE && (newEntryValue->w.status & VAR_STAT_MASK) !="
                            " VAR_STAT_EXTERNAL") )
            {
                __debugbreak();
            }
            if ( (newEntryValue->w.status & 0x1F) != 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                            4685,
                            0,
                            "%s",
                            "(newEntryValue->w.type & VAR_MASK) == VAR_UNDEFINED") )
            {
                __debugbreak();
            }
            type = entryValue->w.status & 0x1F;
            if ( (newEntryValue->w.status & 0x1F) != 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                            4689,
                            0,
                            "%s",
                            "!(newEntryValue->w.type & VAR_MASK)") )
            {
                __debugbreak();
            }
            newEntryValue->w.status |= type;
            if ( newEntryValue->w.status >> 8 != name
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                            4692,
                            0,
                            "%s",
                            "(newEntryValue->w.name >> VAR_NAME_BITS) == name") )
            {
                __debugbreak();
            }
            newEntryValue->u.next = entryValue->u.next;
            AddRefToValue(inst, type, newEntryValue->u.u);
        }
    }
}

double __cdecl Scr_GetObjectUsage(scriptInstance_t inst, unsigned int parentId)
{
    VariableValueInternal *parentValue; // [esp+18h] [ebp-Ch]
    float usage; // [esp+1Ch] [ebp-8h]
    unsigned int id; // [esp+20h] [ebp-4h]

    parentValue = &gScrVarGlob[inst].variableList[parentId + 1];
    if ( (parentValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4764,
                    0,
                    "%s",
                    "(parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (parentValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (parentValue->w.status & 0x1F) < 0xD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4765,
                    0,
                    "%s",
                    "IsObject( parentValue )") )
    {
        __debugbreak();
    }
    usage = 1.0f;
    for ( id = FindFirstSibling(inst, parentId); id; id = FindNextSibling(inst, id) )
        usage = Scr_GetEntryUsage(inst, &gScrVarGlob[inst].variableList[id + 0x8000]) + usage;
    return usage;
}

double __cdecl Scr_GetEntryUsage_0(scriptInstance_t inst, unsigned int type, VariableUnion u)
{
  VariableValueInternal *parentValue; // [esp+8h] [ebp-4h]

  if ( type != 1 )
    return 0.0;
  parentValue = &gScrVarGlob[inst].variableList[u.intValue + 1];
  if ( (parentValue->w.status & 0x60) == 0
    && !Assert_MyHandler(
          "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
          4722,
          0,
          "%s",
          "(parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
  {
    __debugbreak();
  }
  if ( (parentValue->w.status & 0x60) == 0
    && !Assert_MyHandler(
          "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
          323,
          0,
          "%s",
          "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
  {
    __debugbreak();
  }
  if ( (parentValue->w.status & 0x1F) < 0xD
    && !Assert_MyHandler(
          "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
          4723,
          0,
          "%s",
          "IsObject( parentValue )") )
  {
    __debugbreak();
  }
  if ( (parentValue->w.status & 0x1F) == 0x14 )
    return Scr_GetObjectUsage(inst, u.stringValue) / ((double)parentValue->u.o.refCount + 1.0);
  else
    return 0.0;
}

double __cdecl Scr_GetEntryUsage(scriptInstance_t inst, VariableValueInternal *entryValue)
{
    return Scr_GetEntryUsage_0(inst, entryValue->w.status & 0x1F, entryValue->u.u) + 1.0;
}

double __cdecl Scr_GetThreadUsage(scriptInstance_t inst, const VariableStackBuffer *stackBuf, float *endonUsage)
{
    const char *buf; // [esp+0h] [ebp-18h]
    const char *bufa; // [esp+0h] [ebp-18h]
    unsigned int localId; // [esp+4h] [ebp-14h]
    int size; // [esp+8h] [ebp-10h]
    float usage; // [esp+Ch] [ebp-Ch]
    VariableUnion u; // [esp+10h] [ebp-8h]

    size = stackBuf->size;
    buf = &stackBuf->buf[5 * size];
    usage = Scr_GetObjectUsage(inst, stackBuf->localId);
    *endonUsage = Scr_GetEndonUsage(inst, stackBuf->localId);
    localId = stackBuf->localId;
    while ( size )
    {
        bufa = buf - 4;
        u.intValue = *(int *)bufa;
        buf = bufa - 1;
        --size;
        if ( *buf == 7 )
        {
            localId = GetParentLocalId(inst, localId);
            usage = Scr_GetObjectUsage(inst, localId) + usage;
            *endonUsage = Scr_GetEndonUsage(inst, localId) + *endonUsage;
        }
        else
        {
            usage = Scr_GetEntryUsage_0(inst, *(unsigned __int8 *)buf, u) + usage;
        }
    }
    return usage;
}

double __cdecl Scr_GetEndonUsage(scriptInstance_t inst, unsigned int parentId)
{
    unsigned int Object; // eax
    VariableValueInternal *parentValue; // [esp+0h] [ebp-8h]
    unsigned int id; // [esp+4h] [ebp-4h]

    parentValue = &gScrVarGlob[inst].variableList[parentId + 1];
    if ((parentValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            4746,
            0,
            "%s",
            "(parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE"))
    {
        __debugbreak();
    }
    if ((parentValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
            323,
            0,
            "%s",
            "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE"))
    {
        __debugbreak();
    }
    if ((parentValue->w.status & 0x1F) < 0xD
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            4747,
            0,
            "%s",
            "IsObject( parentValue )"))
    {
        __debugbreak();
    }
    id = FindObjectVariable(inst, gScrVarPub[inst].pauseArrayId, parentId);
    if (!id)
        return 0.0;
    Object = FindObject(inst, id);
    return Scr_GetObjectUsage(inst, Object);
}

unsigned int __cdecl Scr_FindField(const char *name, int *type, scriptInstance_t inst)
{
    const char *pos; // [esp+10h] [ebp-Ch]
    const char *posa; // [esp+10h] [ebp-Ch]
    int len; // [esp+14h] [ebp-8h]
    unsigned int index; // [esp+18h] [ebp-4h]

    if ( !gScrVarPub[inst].fieldBuffer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    4825,
                    0,
                    "%s",
                    "gScrVarPub[inst].fieldBuffer") )
    {
        __debugbreak();
    }
    for ( pos = gScrVarPub[inst].fieldBuffer; *pos; pos += len + 3 )
    {
        len = strlen(pos) + 1;
        if ( !I_stricmp(name, pos) )
        {
            posa = &pos[len];
            index = *(unsigned __int16 *)posa;
            *type = posa[2];
            return index;
        }
    }
    return 0;
}

void __cdecl Scr_AddFields(const char *path, char *extension, scriptInstance_t inst)
{
    if ( useFastFile->current.enabled )
        Scr_AddFields_FastFile(inst, path, extension);
    else
        Scr_AddFields_LoadObj(inst, path, extension);
}

void __cdecl Scr_AddFields_LoadObj(scriptInstance_t inst, const char *path, char *extension)
{
    char filename[64]; // [esp+30h] [ebp-50h] BYREF
    int numFiles; // [esp+74h] [ebp-Ch] BYREF
    int i; // [esp+78h] [ebp-8h]
    const char **files; // [esp+7Ch] [ebp-4h]

    files = FS_ListFiles(path, extension, FS_LIST_PURE_ONLY, &numFiles);
    gScrVarPub[inst].fieldBuffer = TempMallocAlignStrict(1);
    *gScrVarPub[inst].fieldBuffer = 0;
    for ( i = 0; i < numFiles; ++i )
    {
        if ( (inst || strlen(files[i]) <= 6 || I_strncmp(files[i], "client", 6))
            && (inst == SCRIPTINSTANCE_SERVER || strlen(files[i]) > 6 && !I_strncmp(files[i], "client", 6)) )
        {
            sprintf(filename, "%s/%s", path, files[i]);
            if ( strlen(filename) >= 0x40
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                            4996,
                            0,
                            "%s",
                            "strlen( filename ) < MAX_QPATH") )
            {
                __debugbreak();
            }
            Scr_AddFieldsForFile(inst, filename);
        }
    }
    if ( files )
        FS_FreeFileList(files);
}

void __cdecl Scr_AddFieldsForFile(scriptInstance_t inst, char *filename)
{
    char *SourceFile_FastFile; // eax
    const char *v3; // eax
    const char *v4; // eax
    char v5; // [esp+3h] [ebp-9Dh]
    char *v6; // [esp+8h] [ebp-98h]
    char *v7; // [esp+Ch] [ebp-94h]
    int v8; // [esp+10h] [ebp-90h]
    int tempType; // [esp+78h] [ebp-28h] BYREF
    char *sourceBuffer; // [esp+7Ch] [ebp-24h]
    int len; // [esp+80h] [ebp-20h]
    int size; // [esp+84h] [ebp-1Ch]
    char *targetPos; // [esp+88h] [ebp-18h]
    unsigned int index; // [esp+8Ch] [ebp-14h]
    int type; // [esp+90h] [ebp-10h]
    const char *sourcePos; // [esp+94h] [ebp-Ch] BYREF
    char *token; // [esp+98h] [ebp-8h]
    int i; // [esp+9Ch] [ebp-4h]

    Hunk_CheckTempMemoryHighClear();
    if ( useFastFile->current.enabled )
        SourceFile_FastFile = (char *)Scr_GetSourceFile_FastFile(filename);
    else
        SourceFile_FastFile = Scr_GetSourceFile_LoadObj(filename);
    sourceBuffer = SourceFile_FastFile;
    sourcePos = SourceFile_FastFile;
    Com_BeginParseSession("Scr_AddFields");
    for ( targetPos = TempMallocAlignStrict(0) - 1; ; *targetPos = 0 )
    {
        token = (char *)Com_Parse(&sourcePos);
        if ( !sourcePos )
            break;
        if ( !strcmp(token, "float") )
        {
            type = 5;
        }
        else if ( !strcmp(token, "int") )
        {
            type = 6;
        }
        else if ( !strcmp(token, "string") )
        {
            type = 2;
        }
        else
        {
            if ( strcmp(token, "vector") )
            {
                v3 = va("unknown type %s in %s", token, filename);
                Com_Error(ERR_DROP, v3);
                return;
            }
            type = 4;
        }
        token = (char *)Com_Parse(&sourcePos);
        if ( !sourcePos )
        {
            v4 = va("missing field name in %s", filename);
            Com_Error(ERR_DROP, v4);
        }
        v8 = strlen(token);
        len = v8 + 1;
        for ( i = v8; i >= 0; --i )
            token[i] = tolower(token[i]);
        index = SL_GetCanonicalString(inst, token);
        if ( Scr_FindField(token, &tempType, inst) )
            Com_Error(ERR_DROP, "duplicate key %s in %s", token, filename);
        if ( targetPos != TempMallocAlignStrict(0) - 1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        4948,
                        0,
                        "%s",
                        "targetPos == TempMalloc( 0 ) - 1") )
        {
            __debugbreak();
        }
        TempMemorySetPos(targetPos);
        size = len + 4;
        targetPos = TempMallocAlignStrict(len + 4);
        v7 = token;
        v6 = targetPos;
        do
        {
            v5 = *v7;
            *v6++ = *v7++;
        }
        while ( v5 );
        targetPos += len;
        *(_WORD *)targetPos = index;
        targetPos += 2;
        *targetPos++ = type;
    }
    Com_EndParseSession();
    Hunk_ClearTempMemoryHigh();
}

char *__cdecl Scr_GetSourceFile_LoadObj(char *filename)
{
    const char *v1; // eax
    unsigned __int8 *sourceBuffer; // [esp+0h] [ebp-Ch]
    int len; // [esp+4h] [ebp-8h]
    int f; // [esp+8h] [ebp-4h] BYREF

    len = FS_FOpenFileByMode(filename, &f, FS_READ);
    if ( len < 0 )
    {
        v1 = va("cannot find %s", filename);
        Com_Error(ERR_DROP, v1);
    }
    sourceBuffer = (unsigned __int8 *)Hunk_AllocateTempMemoryHigh(len + 1, "Scr_LoadAnimTreeInternal");
    FS_Read(sourceBuffer, len, f);
    sourceBuffer[len] = 0;
    FS_FCloseFile(f);
    return (char *)sourceBuffer;
}

char *__cdecl Scr_GetSourceFile_FastFile(const char *filename)
{
    const char *v1; // eax
    RawFile *rawfile; // [esp+4h] [ebp-4h]

    rawfile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, (char*)filename, 1, -1).rawfile;
    if ( !rawfile )
    {
        v1 = va("cannot find %s", filename);
        Com_Error(ERR_DROP, v1);
    }
    return (char*)rawfile->buffer;
}

void __cdecl Scr_AddFields_FastFile(scriptInstance_t inst, const char *path, const char *extension)
{
    char filename[68]; // [esp+4h] [ebp-48h] BYREF

    gScrVarPub[inst].fieldBuffer = TempMallocAlignStrict(1);
    *gScrVarPub[inst].fieldBuffer = 0;
    if ( inst )
        sprintf(filename, "%s/%s.%s", path, "clientkeys", extension);
    else
        sprintf(filename, "%s/%s.%s", path, "keys", extension);
    Scr_AddFieldsForFile(inst, filename);
}

void __cdecl Scr_FreeValue(scriptInstance_t inst, unsigned int id)
{
    if ( !id
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp", 5029, 0, "%s", "id") )
    {
        __debugbreak();
    }
    RemoveRefToObject(inst, id);
}

void __cdecl Scr_AllocGameVariable(scriptInstance_t inst)
{
    if (!gScrVarPub[inst].gameId)
    {
        if (gScrVarPub[inst].varUsagePos)
        {
            if (!Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                5103,
                0,
                "%s",
                "!gScrVarPub[inst].varUsagePos"))
                __debugbreak();
        }
        gScrVarPub[inst].varUsagePos = "<game variable>";
        gScrVarPub[inst].gameId = AllocValue(inst);
        SetEmptyArray(inst, gScrVarPub[inst].gameId);
        gScrVarPub[inst].varUsagePos = 0;
        Scr_UpdateDebugger(inst);
    }
}

void __cdecl Scr_FreeGameVariable(scriptInstance_t inst, int bComplete)
{
    VariableValueInternal *entryValue; // [esp+0h] [ebp-4h]

    if (!gScrVarPub[inst].gameId
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            5124,
            0,
            "%s",
            "gScrVarPub[inst].gameId"))
    {
        __debugbreak();
    }
    if (bComplete)
    {
        FreeValue(inst, gScrVarPub[inst].gameId);
        gScrVarPub[inst].gameId = 0;
    }
    else
    {
        entryValue = &gScrVarGlob[inst].variableList[gScrVarPub[inst].gameId + 0x8000];
        if ((entryValue->w.status & 0x1F) != 1
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                5134,
                0,
                "%s",
                "(entryValue->w.type & VAR_MASK) == VAR_POINTER"))
        {
            __debugbreak();
        }
        Scr_MakeValuePrimitive(inst, entryValue->u.next);
    }
}

int __cdecl Scr_MakeValuePrimitive(scriptInstance_t inst, unsigned int parentId)
{
    VariableValueInternal *parentValue; // [esp+1Ch] [ebp-10h]
    unsigned int name; // [esp+20h] [ebp-Ch]
    VariableValueInternal *entryValue; // [esp+24h] [ebp-8h]
    unsigned int id; // [esp+28h] [ebp-4h]

    parentValue = &gScrVarGlob[inst].variableList[parentId + 1];
    if ( (parentValue->w.status & 0x60) != 0x60
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    5043,
                    0,
                    "%s",
                    "(parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL") )
    {
        __debugbreak();
    }
    if ( (parentValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (parentValue->w.status & 0x1F) < 0xD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    5044,
                    0,
                    "%s",
                    "IsObject( parentValue )") )
    {
        __debugbreak();
    }
    if ( (parentValue->w.status & 0x1F) == 0xD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    5046,
                    0,
                    "%s",
                    "(parentValue->w.type & VAR_MASK) != VAR_THREAD") )
    {
        __debugbreak();
    }
    if ( (parentValue->w.status & 0x1F) == 0xE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    5047,
                    0,
                    "%s",
                    "(parentValue->w.type & VAR_MASK) != VAR_NOTIFY_THREAD") )
    {
        __debugbreak();
    }
    if ( (parentValue->w.status & 0x1F) == 0xF
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    5048,
                    0,
                    "%s",
                    "(parentValue->w.type & VAR_MASK) != VAR_TIME_THREAD") )
    {
        __debugbreak();
    }
    if ( (parentValue->w.status & 0x1F) == 0x10
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                    5049,
                    0,
                    "%s",
                    "(parentValue->w.type & VAR_MASK) != VAR_CHILD_THREAD") )
    {
        __debugbreak();
    }
    if ( (parentValue->w.status & 0x1F) != 0x14 )
        return 0;
    while ( 2 )
    {
        id = FindFirstSibling(inst, parentId);
LABEL_26:
        if ( id )
        {
            entryValue = &gScrVarGlob[inst].variableList[id + 0x8000];
            if ( ((entryValue->w.status & 0x60) == 0 || (entryValue->w.status & 0x60) == 0x60)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                            5059,
                            0,
                            "%s",
                            "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE && (entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_EXTERNAL") )
            {
                __debugbreak();
            }
            if ( (entryValue->w.status & 0x60) == 0
                && !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                            323,
                            0,
                            "%s",
                            "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
            {
                __debugbreak();
            }
            if ( (entryValue->w.status & 0x1F) >= 0xD
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                            5060,
                            0,
                            "%s",
                            "!IsObject( entryValue )") )
            {
                __debugbreak();
            }
            name = entryValue->w.status >> 8;
            switch ( entryValue->w.status & 0x1F )
            {
                case 0u:
                case 2u:
                case 3u:
                case 4u:
                case 5u:
                case 6u:
                    goto LABEL_25;
                case 1u:
                    if ( Scr_MakeValuePrimitive(inst, entryValue->u.next) )
                        goto LABEL_25;
                    goto $LN4_201;
                case 7u:
                case 8u:
                case 9u:
                case 0xAu:
                case 0xBu:
$LN4_201:
                    RemoveVariable(inst, parentId, name);
                    continue;
                default:
                    if ( !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                                    5088,
                                    0,
                                    "unknown type") )
                        __debugbreak();
LABEL_25:
                    id = FindNextSibling(inst, id);
                    goto LABEL_26;
            }
        }
        return 1;
    }
}

int __cdecl Scr_GetClassnumForCharId(scriptInstance_t inst, char charId)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 5; ++i )
    {
        if ( gScrClassMap[inst][i].charId == charId )
            return i;
    }
    return -1;
}

int __cdecl Scr_FindAllThreads(scriptInstance_t inst, unsigned int selfId, unsigned int *threads, unsigned int localId)
{
    unsigned int Object; // eax
    unsigned int stackId; // [esp+34h] [ebp-1Ch]
    unsigned int timeId; // [esp+38h] [ebp-18h]
    unsigned int notifyListId; // [esp+3Ch] [ebp-14h]
    unsigned int notifyListIda; // [esp+3Ch] [ebp-14h]
    unsigned int threadId; // [esp+40h] [ebp-10h]
    unsigned int threadIda; // [esp+40h] [ebp-10h]
    VariableValueInternal *entryValue; // [esp+44h] [ebp-Ch]
    int count; // [esp+48h] [ebp-8h]
    unsigned int id; // [esp+4Ch] [ebp-4h]

    count = 0;
    if ( localId && selfId == Scr_GetSelf(inst, localId) )
    {
        if ( threads )
            *threads = localId;
        count = 1;
    }
    for ( id = 1; id < 0x3FFFE; ++id )
    {
        entryValue = &gScrVarGlob[inst].variableList[id + 0x8000];
        if ( (entryValue->w.status & 0x60) != 0 && (entryValue->w.status & 0x1F) == 0xA )
        {
            for ( threadId = *(unsigned int *)(entryValue->u.next + 8); threadId; threadId = GetSafeParentLocalId(inst, threadId) )
            {
                if ( selfId == Scr_GetSelf(inst, threadId) )
                {
                    if ( threads )
                        threads[count] = threadId;
                    ++count;
                    break;
                }
            }
        }
    }
    notifyListId = FindVariable(inst, selfId, 0x17FFEu);
    if ( notifyListId )
    {
        Object = FindObject(inst, notifyListId);
        for ( notifyListIda = FindFirstSibling(inst, Object);
                    notifyListIda;
                    notifyListIda = FindNextSibling(inst, notifyListIda) )
        {
            timeId = FindObject(inst, notifyListIda);
            if ( !timeId
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                            5250,
                            0,
                            "%s",
                            "timeId") )
            {
                __debugbreak();
            }
            for ( stackId = FindFirstSibling(inst, timeId); stackId; stackId = FindNextSibling(inst, stackId) )
            {
                if ( GetValueType(inst, stackId) == 10 )
                {
                    for ( threadIda = *(unsigned int *)(GetVariableValueAddress(inst, stackId)->next + 8);
                                threadIda;
                                threadIda = GetSafeParentLocalId(inst, threadIda) )
                    {
                        if ( selfId != Scr_GetSelf(inst, threadIda) )
                        {
                            if ( threads )
                                threads[count] = threadIda;
                            ++count;
                            break;
                        }
                    }
                }
            }
        }
    }
    return count;
}

unsigned int __cdecl Scr_FindAllEndons(scriptInstance_t inst, unsigned int threadId, unsigned int *names)
{
    unsigned int localId; // [esp+18h] [ebp-20h]
    unsigned int selfNameId; // [esp+1Ch] [ebp-1Ch]
    unsigned int name; // [esp+20h] [ebp-18h]
    VariableValueInternal *threadValue; // [esp+24h] [ebp-14h]
    unsigned int count; // [esp+2Ch] [ebp-Ch]
    unsigned int id; // [esp+30h] [ebp-8h]
    unsigned int notifyListEntry; // [esp+34h] [ebp-4h]

    if (!threadId
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            5286,
            0,
            "%s",
            "threadId"))
    {
        __debugbreak();
    }
    threadValue = &gScrVarGlob[inst].variableList[threadId + 1];
    if ((threadValue->w.status & 0x60) != 0x60
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            5289,
            0,
            "%s",
            "(threadValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL"))
    {
        __debugbreak();
    }
    if (((threadValue->w.status & 0x1F) < 0xD || (threadValue->w.status & 0x1F) > 0x10)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
            5290,
            0,
            "%s",
            "((threadValue->w.type & VAR_MASK) >= VAR_THREAD) && ((threadValue->w.type & VAR_MASK) <= VAR_CHILD_THREAD)"))
    {
        __debugbreak();
    }
    count = 0;
    for (localId = threadId; localId; localId = GetSafeParentLocalId(inst, localId))
    {
        id = FindObjectVariable(inst, gScrVarPub[inst].pauseArrayId, localId);
        if (id)
        {
            selfNameId = FindObject(inst, id);
            for (notifyListEntry = FindFirstSibling(inst, selfNameId);
                notifyListEntry;
                notifyListEntry = FindNextSibling(inst, notifyListEntry))
            {
                if ((gScrVarGlob[inst].variableList[notifyListEntry + 0x8000].w.status & 0x1F) != 1
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        5303,
                        0,
                        "%s",
                        "(gScrVarGlob[inst].variableList[VARIABLELIST_CHILD_BEGIN(inst)+ notifyListEntry].w.type & VAR_MASK) == VAR_POINTER"))
                {
                    __debugbreak();
                }
                name = gScrVarGlob[inst].variableList[notifyListEntry + 0x8000].w.status >> 8;
                if (name - 0x10000 >= 0x10000
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_variable.cpp",
                        5305,
                        0,
                        "%s",
                        "(name - SL_MAX_STRING_INDEX) < (1 << 16)"))
                {
                    __debugbreak();
                }
                if (names)
                    names[count] = (unsigned __int16)Scr_GetThreadNotifyName(inst, name - 0x10000);
                ++count;
            }
        }
    }
    return count;
}
