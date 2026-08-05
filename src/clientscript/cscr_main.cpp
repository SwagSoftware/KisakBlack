#include "cscr_main.h"
#include "cscr_variable.h"
#include <universal/assertive.h>
#include "cscr_compiler.h"
#include <universal/q_shared.h>
#include <string.h>
#include "cscr_stringlist.h"
#include "cscr_animtree.h"
#include "cscr_parser.h"
#include "cscr_vm.h"
#include "cscr_tempmemory.h"
#include "cscr_evaluate.h"
#include <universal/com_memory.h>
#include <qcommon/com_profilemapload.h>
#include <qcommon/common.h>
#include "cscr_parsetree.h"
#include "cscr_yacc.h"
#include <game/sentient_fields.h>

#undef GetObject

thread_local unsigned int gScriptHighRefMark;
thread_local unsigned int gScriptIndentLevel;
thread_local unsigned int gScriptHighIndentMark;

int scrShowStrUsageStringCount;
int oldNum;

bool __cdecl Scr_IsInOpcodeMemory(scriptInstance_t inst, const char *pos)
{
    if ( !gScrVarPub[inst].programBuffer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
                    35,
                    0,
                    "%s",
                    "gScrVarPub[inst].programBuffer") )
    {
        __debugbreak();
    }
    if ( !pos && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp", 36, 0, "%s", "pos") )
        __debugbreak();
    return pos - gScrVarPub[inst].programBuffer < gScrCompilePub[inst].programLen;
}

char __cdecl Scr_IsIdentifier(const char *token)
{
    while ( *token )
    {
        if ( !I_iscsym(*token) )
            return 0;
        ++token;
    }
    return 1;
}

int __cdecl Scr_GetFunctionHandle(scriptInstance_t inst, const char *filename, const char *name)
{
    VariableValue pos; // [esp+28h] [ebp-2Ch]
    unsigned int str; // [esp+30h] [ebp-24h]
    int result; // [esp+3Ch] [ebp-18h]
    unsigned int name2; // [esp+40h] [ebp-14h]
    unsigned int posId; // [esp+44h] [ebp-10h]
    unsigned int filePosId; // [esp+48h] [ebp-Ch]
    unsigned int id; // [esp+50h] [ebp-4h]

    if ( !gScrCompilePub[inst].scriptsPos
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
                    76,
                    0,
                    "%s",
                    "gScrCompilePub[inst].scriptsPos") )
    {
        __debugbreak();
    }
    if ( strlen(filename) >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
                    77,
                    0,
                    "%s",
                    "strlen( filename ) < MAX_QPATH") )
    {
        __debugbreak();
    }
    name2 = Scr_CreateCanonicalFilename(inst, filename);
    filePosId = FindVariable(inst, gScrCompilePub[inst].scriptsPos, name2);
    SL_RemoveRefToString(inst, name2);
    if ( !filePosId )
        return 0;

    id = FindObject(inst, filePosId);
    if ( !id && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp", 87, 0, "%s", "id") )
        __debugbreak();
    str = SL_FindLowercaseString(name, inst);
    if ( !str )
        return 0;

    posId = FindVariable(inst, id, str);
    if ( !posId )
        return 0;
    pos = Scr_EvalVariable(inst, posId);
    if ( pos.type != 7
        && pos.type != 12
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
                    98,
                    0,
                    "%s",
                    "pos.type == VAR_CODEPOS || pos.type == VAR_DEVELOPER_CODEPOS") )
    {
        __debugbreak();
    }
    if ( !Scr_IsInOpcodeMemory(inst, pos.u.codePosValue) )
        return 0;
    result = pos.u.intValue - (unsigned int)gScrVarPub[inst].programBuffer;
    if ( !result
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp", 105, 0, "%s", "result") )
    {
        __debugbreak();
    }
    return result;
}

unsigned int __cdecl SL_TransferToCanonicalString(scriptInstance_t inst, unsigned int stringValue)
{
    if (!stringValue
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp", 126, 0, "%s", "stringValue"))
    {
        __debugbreak();
    }
    SL_TransferRefToUser(inst, stringValue, 2u);
    if (gScrCompilePub[inst].canonicalStrings[stringValue])
        return gScrCompilePub[inst].canonicalStrings[stringValue];
    gScrCompilePub[inst].canonicalStrings[stringValue] = ++gScrVarPub[inst].canonicalStrCount;
    return gScrVarPub[inst].canonicalStrCount;
}

unsigned int __cdecl SL_GetCanonicalString(scriptInstance_t inst, char *str)
{
    unsigned int String; // eax
    unsigned int v4; // eax

    String = SL_FindString(str, inst);
    if ( gScrCompilePub[inst].canonicalStrings[String] )
        return gScrCompilePub[inst].canonicalStrings[String];
    v4 = SL_GetString_(inst, str, 0, 16);
    return SL_TransferToCanonicalString(inst, v4);
}

void __cdecl Scr_BeginLoadScripts(scriptInstance_t inst, int user)
{
    gScrVarPub[inst].varUsagePos = "<script compile variable>";

    gScriptIndentLevel = 0;
    gScriptHighRefMark = 0;

    if ( gScrCompilePub[inst].script_loading
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
                    175,
                    0,
                    "%s",
                    "!gScrCompilePub[inst].script_loading") )
    {
        __debugbreak();
    }
    gScrCompilePub[inst].script_loading = 1;

    Scr_InitOpcodeLookup(inst);
    Scr_InitDebuggerMain(inst);

    if ( gScrCompilePub[inst].loadedscripts
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
                    183,
                    0,
                    "%s",
                    "!gScrCompilePub[inst].loadedscripts") )
    {
        __debugbreak();
    }
    gScrCompilePub[inst].loadedscripts = Scr_AllocArray(inst);
    if ( gScrVarDebugPub[inst] )
        ++gScrVarDebugPub[inst]->extRefCount[gScrCompilePub[inst].loadedscripts];
    gScrVmPub[inst].showError = gScrVarPub[inst].developer;
    if ( gScrCompilePub[inst].scriptsPos
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
                    204,
                    0,
                    "%s",
                    "!gScrCompilePub[inst].scriptsPos") )
    {
        __debugbreak();
    }
    gScrCompilePub[inst].scriptsPos = Scr_AllocArray(inst);
    if ( gScrVarDebugPub[inst] )
        ++gScrVarDebugPub[inst]->extRefCount[gScrCompilePub[inst].scriptsPos];
    if ( gScrCompilePub[inst].scriptsCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
                    212,
                    0,
                    "%s",
                    "!gScrCompilePub[inst].scriptsCount") )
    {
        __debugbreak();
    }
    gScrCompilePub[inst].scriptsCount = Scr_AllocArray(inst);
    if ( gScrVarDebugPub[inst] )
        ++gScrVarDebugPub[inst]->extRefCount[gScrCompilePub[inst].scriptsCount];
    if ( gScrCompilePub[inst].builtinFunc
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
                    220,
                    0,
                    "%s",
                    "!gScrCompilePub[inst].builtinFunc") )
    {
        __debugbreak();
    }
    gScrCompilePub[inst].builtinFunc = Scr_AllocArray(inst);
    if ( gScrVarDebugPub[inst] )
        ++gScrVarDebugPub[inst]->extRefCount[gScrCompilePub[inst].builtinFunc];
    if ( gScrCompilePub[inst].builtinMeth
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
                    227,
                    0,
                    "%s",
                    "!gScrCompilePub[inst].builtinMeth") )
    {
        __debugbreak();
    }
    gScrCompilePub[inst].builtinMeth = Scr_AllocArray(inst);
    if ( gScrVarDebugPub[inst] )
        ++gScrVarDebugPub[inst]->extRefCount[gScrCompilePub[inst].builtinMeth];
    gScrVarPub[inst].programHunkUser = Hunk_UserCreate(0x100000, HU_SCHEME_DEFAULT, 4u, 0, "Scr_BeginLoadScripts", 7);
    TempMemoryReset(gScrVarPub[inst].programHunkUser);
    gScrVarPub[inst].programBuffer = TempMallocAlignStrict(0);
    if ( ((int)gScrVarPub[inst].programBuffer & 0x1F) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
                    242,
                    0,
                    "%s\n\t((int)gScrVarPub[inst].programBuffer) = %i",
                    "(!((int)gScrVarPub[inst].programBuffer & 31))",
                    gScrVarPub[inst].programBuffer) )
    {
        __debugbreak();
    }
    gScrCompilePub[inst].programLen = 0;
    gScrVarPub[inst].endScriptBuffer = 0;
    SL_BeginLoadScripts(inst);
    Scr_InitEvaluate(inst);
    gScrVarPub[inst].fieldBuffer = 0;
    gScrCompilePub[inst].value_count = 0;
    Scr_ClearErrorMessage(inst);
    gScrCompilePub[inst].func_table_size = 0;
    gScrAnimPub[inst].animTreeNames = 0;
    Scr_SetLoadedImpureScript(0);
    Scr_BeginLoadAnimTrees(inst, user);
}

void __cdecl SL_BeginLoadScripts(scriptInstance_t inst)
{
    memset(gScrCompilePub[inst].canonicalStrings, 0, sizeof(gScrCompilePub[inst].canonicalStrings));
    gScrVarPub[inst].canonicalStrCount = 0;
}

void __cdecl Scr_BeginLoadAnimTrees(scriptInstance_t inst, int user)
{
    gScrVarPub[inst].varUsagePos = "<script compile variable>";
    if (gScrAnimPub[inst].animtree_loading
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
            294,
            0,
            "%s",
            "!gScrAnimPub[inst].animtree_loading"))
    {
        __debugbreak();
    }
    gScrAnimPub[inst].animtree_loading = 1;
    gScrAnimPub[inst].xanim_num[user] = 0;
    gScrAnimPub[inst].xanim_lookup[user][0].anims = 0;
    if (gScrAnimPub[inst].animtrees
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
            300,
            0,
            "%s",
            "!gScrAnimPub[inst].animtrees"))
    {
        __debugbreak();
    }
    gScrAnimPub[inst].animtrees = Scr_AllocArray(inst);
    if (gScrVarDebugPub[inst])
        ++gScrVarDebugPub[inst]->extRefCount[gScrAnimPub[inst].animtrees];
    gScrAnimPub[inst].animtree_node = 0;
    gScrCompilePub[inst].developer_statement = 0;
}

int __cdecl Scr_ScanFile(scriptInstance_t inst, char *buf, int max_size)
{
    char c; // [esp+3h] [ebp-5h]
    int n; // [esp+4h] [ebp-4h]

    c = 42;
    for ( n = 0; n < max_size; ++n )
    {
        c = *gScrCompilePub[inst].in_ptr++;
        if ( !c || c == 10 )
            break;
        buf[n] = c;
    }
    if ( c == 10 )
    {
        buf[n++] = c;
    }
    else if ( !c )
    {
        if ( gScrCompilePub[inst].parseBuf )
        {
            gScrCompilePub[inst].in_ptr = gScrCompilePub[inst].parseBuf;
            gScrCompilePub[inst].parseBuf = 0;
        }
        else
        {
            --gScrCompilePub[inst].in_ptr;
        }
    }
    return n;
}

unsigned int __cdecl Scr_LoadScriptInternal(
    scriptInstance_t inst,
    const char *filename,
    PrecacheEntry *entries,
    int entriesCount)
{
    char *v5; // eax
    char *v6; // eax
    unsigned int Variable; // eax
    unsigned int v8; // eax
    const char *v9; // [esp-8h] [ebp-94h]
    char extFilename[68]; // [esp+14h] [ebp-78h] BYREF
    unsigned int fileCountId; // [esp+5Ch] [ebp-30h]
    unsigned int filePosPtr; // [esp+60h] [ebp-2Ch]
    char *sourceBuffer; // [esp+64h] [ebp-28h]
    const char *oldFilename; // [esp+68h] [ebp-24h]
    unsigned int name; // [esp+6Ch] [ebp-20h]
    unsigned int oldAnimTreeNames; // [esp+70h] [ebp-1Ch]
    const char *oldSourceBuf; // [esp+74h] [ebp-18h]
    unsigned int scriptId; // [esp+78h] [ebp-14h]
    unsigned int filePosId; // [esp+7Ch] [ebp-10h]
    int oldStrCount; // [esp+80h] [ebp-Ch]
    const char *formatExtString; // [esp+84h] [ebp-8h]
    sval_u parseData; // [esp+88h] [ebp-4h] BYREF

    oldStrCount = 0;
    if (!gScrCompilePub[inst].script_loading
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
            360,
            0,
            "%s",
            "gScrCompilePub[inst].script_loading"))
    {
        __debugbreak();
    }
    if (strlen(filename) >= 0x40
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
            361,
            0,
            "%s",
            "strlen( filename ) < MAX_QPATH"))
    {
        __debugbreak();
    }
    Hunk_CheckTempMemoryHighClear();
    name = Scr_CreateCanonicalFilename(inst, filename);
    if (FindVariable(inst, gScrCompilePub[inst].loadedscripts, name))
    {
        SL_RemoveRefToString(inst, name);
        filePosPtr = FindVariable(inst, gScrCompilePub[inst].scriptsPos, name);
        if (filePosPtr)
            return FindObject(inst, filePosPtr);
        else
            return 0;
    }
    else
    {
        scriptId = GetNewVariable(inst, gScrCompilePub[inst].loadedscripts, name);
        SL_RemoveRefToString(inst, name);
        formatExtString = "%s.gsc";
        if (inst == SCRIPTINSTANCE_CLIENT
            && (!I_strncmp(filename, "clientscripts", 13) || !I_strncmp(filename, "character/clientscripts", 23)))
        {
            formatExtString = "%s.csc";
        }
        v5 = SL_ConvertToString(name, inst);
        Com_sprintf(extFilename, 0x40u, formatExtString, v5);
        oldSourceBuf = gScrParserPub[inst].sourceBuf;
        if (scrShowStrUsage->current.enabled)
            oldStrCount = gScrStringGlob[inst].stringsUsed[++gScrStringGlob[inst].indentLevel];
        ProfLoad_Begin("Scr_AddSourceBuffer");
        v9 = TempMallocAlignStrict(0);
        v6 = SL_ConvertToString(name, inst);
        sourceBuffer = Scr_AddSourceBuffer(inst, v6, extFilename, v9, 1);
        ProfLoad_End();
        if (sourceBuffer)
        {
            //NtCurrentTeb()->ThreadLocalStoragePointer[_tls_index]->gScriptIndentLevel = NtCurrentTeb()->ThreadLocalStoragePointer[_tls_index]->gScriptIndentLevel + 1;
            gScriptIndentLevel++;

            //if (NtCurrentTeb()->ThreadLocalStoragePointer[_tls_index]->gScriptIndentLevel > NtCurrentTeb()->ThreadLocalStoragePointer[_tls_index]->gScriptHighIndentMark)
            //    NtCurrentTeb()->ThreadLocalStoragePointer[_tls_index]->gScriptHighIndentMark = NtCurrentTeb()->ThreadLocalStoragePointer[_tls_index]->gScriptIndentLevel;

            if (gScriptIndentLevel > gScriptHighIndentMark)
            {
                gScriptHighIndentMark = gScriptIndentLevel;
            }

            if (scrShowVarUseage->current.enabled)
                Com_Printf(
                    25,
                    "Entering file %s (%i)\n",
                    filename,
                    gScriptIndentLevel);

            oldAnimTreeNames = gScrAnimPub[inst].animTreeNames;
            gScrAnimPub[inst].animTreeNames = 0;
            gScrCompilePub[inst].far_function_count = 0;
            Scr_InitAllocNode(inst);
            oldFilename = gScrParserPub[inst].scriptfilename;
            gScrParserPub[inst].scriptfilename = extFilename;
            gScrCompilePub[inst].in_ptr = "+";
            gScrCompilePub[inst].parseBuf = sourceBuffer;
            ScriptParse(inst, &parseData, 0);
            Variable = GetVariable(inst, gScrCompilePub[inst].scriptsPos, name);
            filePosId = GetObject(inst, Variable);
            v8 = GetVariable(inst, gScrCompilePub[inst].scriptsCount, name);
            fileCountId = GetObject(inst, v8);
            ProfLoad_Begin("ScriptCompile");
            ScriptCompile(inst, parseData, filePosId, fileCountId, scriptId, entries, entriesCount);
            ProfLoad_End();
            if (scrShowStrUsage->current.enabled)
            {
                Com_Printf(
                    0,
                    "---Script File (%i) : %s -> %i\n",
                    gScrStringGlob[inst].indentLevel,
                    filename,
                    gScrStringGlob[inst].stringsUsed[gScrStringGlob[inst].indentLevel] - oldStrCount);
                scrShowStrUsageStringCount += gScrStringGlob[inst].stringsUsed[gScrStringGlob[inst].indentLevel--] - oldStrCount;
                if (!gScrStringGlob[inst].indentLevel)
                    Com_Printf(0, "******Current compilation string total: %i\n", scrShowStrUsageStringCount);
            }
            //if (gScrVarPub[inst].numScriptValues > NtCurrentTeb()->ThreadLocalStoragePointer[_tls_index]->gScriptHighRefMark)
            //    NtCurrentTeb()->ThreadLocalStoragePointer[_tls_index]->gScriptHighRefMark = gScrVarPub[inst].numScriptValues;
            if (gScrVarPub[inst].numScriptValues > gScriptHighRefMark)
            {
                gScriptHighRefMark = gScrVarPub[inst].numScriptValues;
            }
            if (scrShowVarUseage->current.enabled)
                Com_Printf(
                    25,
                    "---\nScript File (%i) : %s\nCur var useage %i\nChange %i\n---\n",
                    //NtCurrentTeb()->ThreadLocalStoragePointer[_tls_index]->gScriptIndentLevel,
                    gScriptIndentLevel,
                    filename,
                    gScrVarPub[inst].numScriptValues,
                    gScrVarPub[inst].numScriptValues - oldNum);
            oldNum = gScrVarPub[inst].numScriptValues;
            gScrParserPub[inst].scriptfilename = oldFilename;
            gScrParserPub[inst].sourceBuf = oldSourceBuf;
            gScrAnimPub[inst].animTreeNames = oldAnimTreeNames;
            Hunk_CheckTempMemoryHighClear();
            if (scrShowVarUseage->current.enabled)
                Com_Printf(
                    19,
                    "Leaving file %s (%i)\n",
                    filename,
                    //NtCurrentTeb()->ThreadLocalStoragePointer[_tls_index]->gScriptIndentLevel);
                    gScriptIndentLevel);
            //NtCurrentTeb()->ThreadLocalStoragePointer[_tls_index]->gScriptIndentLevel = NtCurrentTeb()->ThreadLocalStoragePointer[_tls_index]->gScriptIndentLevel - 1;
            gScriptIndentLevel = gScriptIndentLevel - 1;
            return filePosId;
        }
        else
        {
            return 0;
        }
    }
}


unsigned int __cdecl Scr_LoadScript(scriptInstance_t inst, char *filename)
{
    PrecacheEntry entries[MAX_PRECACHE_ENTRIES]; // [esp+0h] [ebp-3800h] BYREF

    return Scr_LoadScriptInternal(inst, filename, &entries[0], 0);
}

void __cdecl Scr_PostCompileScripts(scriptInstance_t inst)
{
    if (gScrCompilePub[inst].programLen != TempMallocAlignStrict(0) - gScrVarPub[inst].programBuffer
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
            489,
            0,
            "%s",
            "gScrCompilePub[inst].programLen == static_cast< uint >( TempMalloc( 0 ) - gScrVarPub[inst].programBuffer )"))
    {
        __debugbreak();
    }
    if (gScrAnimPub[inst].animTreeNames)
    {
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
            490,
            0,
            "%s",
            "!gScrAnimPub[inst].animTreeNames"))
            __debugbreak();
    }
}

void __cdecl Scr_EndLoadScripts(scriptInstance_t inst)
{
    if (scrShowVarUseage->current.enabled)
        Com_Printf(
            25,
            "---\nHigh ref count : %i\nDeepest include level %i\n",
            gScriptHighRefMark,
            gScriptHighIndentMark);
    Scr_EndLoadEvaluate(inst);
    //BG_EvalVehicleName();
    SL_ShutdownSystem(inst, 2u);
    if (com_sv_running->current.enabled)
        Scr_InitDebugger(inst);
    gScrCompilePub[inst].script_loading = 0;
    if (!gScrCompilePub[inst].loadedscripts
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
            523,
            0,
            "%s",
            "gScrCompilePub[inst].loadedscripts"))
    {
        __debugbreak();
    }
    ClearObject(inst, gScrCompilePub[inst].loadedscripts);
    if (gScrVarDebugPub[inst])
        --gScrVarDebugPub[inst]->extRefCount[gScrCompilePub[inst].loadedscripts];
    RemoveRefToObject(inst, gScrCompilePub[inst].loadedscripts);
    gScrCompilePub[inst].loadedscripts = 0;

    if (!gScrCompilePub[inst].scriptsPos
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
            537,
            0,
            "%s",
            "gScrCompilePub[inst].scriptsPos"))
    {
        __debugbreak();
    }
    ClearObject(inst, gScrCompilePub[inst].scriptsPos);
    if (gScrVarDebugPub[inst])
        --gScrVarDebugPub[inst]->extRefCount[gScrCompilePub[inst].scriptsPos];
    RemoveRefToObject(inst, gScrCompilePub[inst].scriptsPos);
    gScrCompilePub[inst].scriptsPos = 0;

    if (!gScrCompilePub[inst].scriptsCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
            547,
            0,
            "%s",
            "gScrCompilePub[inst].scriptsCount"))
    {
        __debugbreak();
    }
    ClearObject(inst, gScrCompilePub[inst].scriptsCount);
    if (gScrVarDebugPub[inst])
        --gScrVarDebugPub[inst]->extRefCount[gScrCompilePub[inst].scriptsCount];
    RemoveRefToObject(inst, gScrCompilePub[inst].scriptsCount);
    gScrCompilePub[inst].scriptsCount = 0;

    if (!gScrCompilePub[inst].builtinFunc
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
            556,
            0,
            "%s",
            "gScrCompilePub[inst].builtinFunc"))
    {
        __debugbreak();
    }
    ClearObject(inst, gScrCompilePub[inst].builtinFunc);
    if (gScrVarDebugPub[inst])
        --gScrVarDebugPub[inst]->extRefCount[gScrCompilePub[inst].builtinFunc];
    RemoveRefToObject(inst, gScrCompilePub[inst].builtinFunc);
    gScrCompilePub[inst].builtinFunc = 0;
    if (!gScrCompilePub[inst].builtinMeth
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
            565,
            0,
            "%s",
            "gScrCompilePub[inst].builtinMeth"))
    {
        __debugbreak();
    }
    ClearObject(inst, gScrCompilePub[inst].builtinMeth);
    if (gScrVarDebugPub[inst])
        --gScrVarDebugPub[inst]->extRefCount[gScrCompilePub[inst].builtinMeth];
    RemoveRefToObject(inst, gScrCompilePub[inst].builtinMeth);
    gScrCompilePub[inst].builtinMeth = 0;
}

void __cdecl Scr_PrecacheAnimTrees(scriptInstance_t inst, void *(__cdecl *Alloc)(int), int user, bool modChecksum)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for (i = 1; i <= gScrAnimPub[inst].xanim_num[user]; ++i)
        Scr_LoadAnimTreeAtIndex(inst, i, Alloc, user, modChecksum);
}

void __cdecl Scr_EndLoadAnimTrees(scriptInstance_t inst)
{
    if (!gScrAnimPub[inst].animtrees
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
            589,
            0,
            "%s",
            "gScrAnimPub[inst].animtrees"))
    {
        __debugbreak();
    }
    ClearObject(inst, gScrAnimPub[inst].animtrees);
    if (gScrVarDebugPub[inst])
        --gScrVarDebugPub[inst]->extRefCount[gScrAnimPub[inst].animtrees];
    RemoveRefToObject(inst, gScrAnimPub[inst].animtrees);
    gScrAnimPub[inst].animtrees = 0;
    if (gScrAnimPub[inst].animtree_node)
        RemoveRefToObject(inst, gScrAnimPub[inst].animtree_node);
    SL_ShutdownSystem(inst, 2u);
    if (gScrVarPub[inst].programBuffer && !gScrVarPub[inst].endScriptBuffer)
        gScrVarPub[inst].endScriptBuffer = TempMallocAlignStrict(0);
    gScrAnimPub[inst].animtree_loading = 0;
    gScrVarPub[inst].varUsagePos = 0;
}

void __cdecl Scr_FreeScripts(scriptInstance_t inst, unsigned __int8 sys)
{
    unsigned int *xanim_num; // ecx

    if (sys != 1
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_main.cpp",
            616,
            0,
            "%s",
            "sys == SCR_SYS_GAME"))
    {
        __debugbreak();
    }
    Hunk_CheckTempMemoryClear();
    if (gScrCompilePub[inst].script_loading)
    {
        gScrCompilePub[inst].script_loading = 0;
        Scr_EndLoadScripts(inst);
    }
    if (gScrAnimPub[inst].animtree_loading)
    {
        gScrAnimPub[inst].animtree_loading = 0;
        Scr_EndLoadAnimTrees(inst);
    }
    Scr_ShutdownDebugger(inst);
    Scr_ShutdownDebuggerMain(inst);
    Scr_ShutdownEvaluate(inst);
    SL_ShutdownSystem(inst, 1u);
    Scr_ShutdownOpcodeLookup(inst);
    if (gScrVarPub[inst].programHunkUser)
    {
        Hunk_UserDestroy(gScrVarPub[inst].programHunkUser);
        gScrVarPub[inst].programHunkUser = 0;
    }
    gScrVarPub[inst].programBuffer = 0;
    gScrCompilePub[inst].programLen = 0;
    gScrVarPub[inst].endScriptBuffer = 0;
    gScrVarPub[inst].checksum = 0;
    xanim_num = gScrAnimPub[inst].xanim_num;
    *xanim_num = 0;
    xanim_num[1] = 0;
}

void __cdecl Scr_GetGenericField(
                unsigned __int8 *b,
                fieldtype_t type,
                int ofs,
                scriptInstance_t inst,
                unsigned int whichbits)
{
    unsigned __int16 str; // [esp+Ch] [ebp-14h]
    float vec[3]; // [esp+14h] [ebp-Ch] BYREF

    switch ( type )
    {
        case F_INT:
            Scr_AddInt(*(unsigned int *)&b[ofs], inst);
            break;
        case F_SHORT:
            Scr_AddInt(*(__int16 *)&b[ofs], inst);
            break;
        case F_BYTE:
            Scr_AddInt(b[ofs], inst);
            break;
        case F_FLOAT:
            Scr_AddFloat(*(float *)&b[ofs], inst);
            break;
        case F_LSTRING:
            Scr_AddString((char *)&b[ofs], inst);
            break;
        case F_STRING:
            str = *(_WORD *)&b[ofs];
            if ( str )
                Scr_AddConstString(str, inst);
            break;
        case F_VECTOR:
            Scr_AddVector((float *)&b[ofs], inst);
            break;
        case F_VECTORHACK:
            vec[0] = 0.0f;
            vec[1] = *(float *)&b[ofs];
            vec[2] = 0.0f;
            Scr_AddVector(vec, inst);
            break;
        case F_BITFLAG:
            if ( (whichbits & *(unsigned int *)&b[ofs]) != 0 )
                Scr_AddInt(1, inst);
            else
                Scr_AddInt(0, inst);
            break;
        default:
            return;
    }
}

void __cdecl Scr_SetGenericField(
                unsigned __int8 *b,
                fieldtype_t type,
                int ofs,
                scriptInstance_t inst,
                unsigned int whichbits)
{
    float vec[3]; // [esp+4h] [ebp-Ch] BYREF

    switch ( type )
    {
        case F_INT:
            *(int *)&b[ofs] = Scr_GetInt(0, inst);
            break;
        case F_SHORT:
            *(short *)&b[ofs] = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
            break;
        case F_BYTE:
            *(uint8 *)&b[ofs] = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
            break;
        case F_FLOAT:
            *(float *)&b[ofs] = Scr_GetFloat(0, inst);
            break;
        case F_STRING:
            Scr_SetString((unsigned short *)&b[ofs], Scr_GetConstStringIncludeNull(0, inst), inst);
            break;
        case F_VECTOR:
            Scr_GetVector(0, vec, inst);
            *(float *)&b[ofs] = vec[0];
            *(float *)&b[ofs + 4] = vec[1];
            *(float *)&b[ofs + 8] = vec[2];
            break;
        case F_VECTORHACK:
            Scr_GetVector(0, vec, inst);
            *(float *)&b[ofs] = vec[1];
            break;
        case F_BITFLAG:
            if (Scr_GetInt(0, inst))
            {
                *(int *)&b[ofs] |= whichbits;
            }
            else
            {
                *(int *)&b[ofs] &= ~whichbits;
            }
            break;
        default:
            return;
    }
}

