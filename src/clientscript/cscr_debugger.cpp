#include "cscr_debugger.h"
#include <universal/assertive.h>
#include "cscr_compiler.h"
#include "cscr_parser.h"
#include <universal/q_shared.h>
#include <win32/win_net.h>
#include "cscr_vm.h"
#include <universal/com_memory.h>
#include <qcommon/common.h>
#include "cscr_stringlist.h"
#include "cscr_evaluate.h"
#include "cscr_parsetree.h"
#include <win32/win_common.h>
#include <qcommon/threads.h>
#include <qcommon/cmd.h>
#include <client/cl_console.h>

scrDebuggerGlob_t gScrDebuggerGlob[2];
scriptInstance_t gDebuggerInstance;

Scr_Breakpoint *g_breakpointsHead;

scriptInstance_t sortInst;

Scr_Breakpoint g_breakpoints[128];

thread_local int g_breakonExpr;
thread_local unsigned int g_breakonString;
thread_local int g_breakonHit;
thread_local unsigned int g_breakonObject;

void __cdecl Scr_AddManualBreakpoint(scriptInstance_t inst, unsigned __int8 *codePos)
{
    if ( !codePos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp", 214, 0, "%s", "codePos") )
    {
        __debugbreak();
    }
    if ( *codePos == 135 || *codePos == 137 )
    {
        if ( (unsigned __int8 *)gScrDebuggerGlob[inst].nextBreakpointCodePos != codePos
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                        218,
                        0,
                        "%s",
                        "(byte *)gScrDebuggerGlob[inst].nextBreakpointCodePos == codePos") )
        {
            __debugbreak();
        }
        if ( gScrDebuggerGlob[inst].nextBreakpointCodePosMasked
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                        219,
                        0,
                        "%s",
                        "!gScrDebuggerGlob[inst].nextBreakpointCodePosMasked") )
        {
            __debugbreak();
        }
        gScrDebuggerGlob[inst].nextBreakpointCodePosMasked = 1;
    }
    else if ( *codePos == 136 )
    {
        *codePos = -119;
    }
    else
    {
        Scr_AddBreakpoint(inst, codePos);
        *codePos = -121;
    }
}

void __cdecl Scr_AddBreakpoint(scriptInstance_t inst, const unsigned __int8 *codePos)
{
    unsigned __int8 *breakpoint; // [esp+0h] [ebp-4h]

    if ( *codePos == 127
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    165,
                    0,
                    "%s",
                    "*codePos != OP_NOP") )
    {
        __debugbreak();
    }
    breakpoint = (unsigned __int8 *)Scr_FindBreakpointInfo(inst, (const char *)codePos);
    if ( *breakpoint != 127
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    167,
                    0,
                    "%s",
                    "*breakpoint == OP_NOP") )
    {
        __debugbreak();
    }
    *breakpoint = *codePos;
    ++gScrDebuggerGlob[inst].breakpointCount;
}

char *__cdecl Scr_FindBreakpointInfo(scriptInstance_t inst, const char *codePos)
{
    unsigned int index; // [esp+0h] [ebp-4h]

    if (!codePos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp", 142, 0, "%s", "codePos"))
    {
        __debugbreak();
    }
    index = codePos - gScrVarPub[inst].programBuffer;
    if (index >= gScrCompilePub[inst].programLen
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            145,
            0,
            "%s",
            "index < gScrCompilePub[inst].programLen"))
    {
        __debugbreak();
    }
    return &gScrDebuggerGlob[inst].breakpoints[index];
}

Scr_Breakpoint *__cdecl Scr_AllocBreakpoint()
{
    Scr_Breakpoint *breakpoint; // [esp+0h] [ebp-4h]

    breakpoint = g_breakpointsHead;
    if ( !g_breakpointsHead
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    338,
                    0,
                    "%s",
                    "breakpoint") )
    {
        __debugbreak();
    }
    g_breakpointsHead = g_breakpointsHead->next;
    return breakpoint;
}

void __cdecl Scr_FreeBreakpoint(Scr_Breakpoint *breakpoint)
{
    breakpoint->next = g_breakpointsHead;
    g_breakpointsHead = breakpoint;
}

void __thiscall Scr_ScriptWindow::SetScriptFile(scriptInstance_t inst, const char *name)
{
    SourceBufferInfo *sourceBufData; // [esp+4h] [ebp-8h]
    unsigned int i; // [esp+8h] [ebp-4h]

    for ( i = 0; i < gScrParserPub[inst].sourceBufferLookupLen; ++i )
    {
        sourceBufData = &gScrParserPub[inst].sourceBufferLookup[i];
        if ( sourceBufData->buf && !I_stricmp(sourceBufData->buf, name) )
        {
            this->bufferIndex = i;
            Scr_ScriptWindow::Init(inst);
            return;
        }
    }
    this->bufferIndex = -1;
    Scr_ScriptWindow::Init(inst);
}

void __thiscall Scr_ScriptWindow::AddBreakpoint(
                scriptInstance_t inst,
                Scr_Breakpoint **pBreakpoint,
                char *codePos,
                int builtinIndex,
                Scr_WatchElement_s *element,
                unsigned __int8 type)
{
    Scr_Breakpoint *breakpoint; // [esp+8h] [ebp-8h]
    Scr_Breakpoint *newBreakpoint; // [esp+Ch] [ebp-4h]

    breakpoint = *pBreakpoint;
    newBreakpoint = Scr_AllocBreakpoint();
    *pBreakpoint = newBreakpoint;
    newBreakpoint->prev = pBreakpoint;
    newBreakpoint->next = breakpoint;
    if ( breakpoint )
        breakpoint->prev = &newBreakpoint->next;
    newBreakpoint->line = this->selectedLine;
    newBreakpoint->codePos = codePos;
    newBreakpoint->builtinIndex = builtinIndex;
    newBreakpoint->bufferIndex = this->bufferIndex;
    newBreakpoint->element = element;
    if ( element->breakpoint
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    678,
                    0,
                    "%s",
                    "!element->breakpoint") )
    {
        __debugbreak();
    }
    element->breakpoint = newBreakpoint;
    element->breakpointType = type;
    if ( type >= 4u && type <= 5u )
    {
        if ( !codePos
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                        690,
                        0,
                        "%s",
                        "codePos") )
        {
            __debugbreak();
        }
        Scr_AddManualBreakpoint(inst, (unsigned __int8 *)codePos);
    }
}

Scr_WatchElement_s *__cdecl Scr_ReadElement(scriptInstance_t inst)
{
    int id; // [esp+4h] [ebp-4h]

    id = Sys_ReadDebugSocketInt();
    if (id)
    {
        //return Scr_ScriptWatch::GetElementWithId(&gScrDebuggerGlob[inst].scriptWatch, id);
        return gScrDebuggerGlob[inst].scriptWatch.GetElementWithId(id);
    }
    else
    {
        return 0;
    }
}

void __thiscall Scr_ScriptWindow::GetSourcePos(
                scriptInstance_t inst,
                unsigned int *start,
                unsigned int *end)
{
    SourceBufferInfo *sourceBufData; // [esp+4h] [ebp-Ch]
    const char *s; // [esp+8h] [ebp-8h]
    int line; // [esp+Ch] [ebp-4h]

    if ( this->selectedLine < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    782,
                    0,
                    "%s",
                    "selectedLine >= 0") )
    {
        __debugbreak();
    }
    sourceBufData = &gScrParserPub[inst].sourceBufferLookup[this->bufferIndex];
    s = sourceBufData->sourceBuf;
    for ( line = 0; ; ++line )
    {
        if ( s - sourceBufData->sourceBuf > sourceBufData->len
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                        791,
                        0,
                        "%s",
                        "s - sourceBufData->sourceBuf <= sourceBufData->len") )
        {
            __debugbreak();
        }
        if ( line == this->selectedLine )
            break;
        while ( *s )
            ++s;
        ++s;
    }
    *start = s - sourceBufData->sourceBuf;
    while ( *s )
        ++s;
    *end = s - sourceBufData->sourceBuf;
}

char __thiscall Scr_ScriptWindow::AddBreakpointAtSourcePos(
                scriptInstance_t inst,
                Scr_WatchElement_s *element,
                unsigned __int8 breakpointType,
                bool user,
                Scr_Breakpoint **pBreakpoint,
                unsigned int startSourcePos,
                unsigned int endSourcePos)
{
    const char *v8; // eax
    unsigned __int16 v11; // [esp+8h] [ebp-1Ch]
    int builtinIndex; // [esp+Ch] [ebp-18h]
    Scr_WatchElement_s *newElement; // [esp+14h] [ebp-10h]
    bool success; // [esp+1Bh] [ebp-9h]
    char *codePos; // [esp+1Ch] [ebp-8h]
    unsigned __int8 *codePosa; // [esp+1Ch] [ebp-8h]
    unsigned int sourcePos; // [esp+20h] [ebp-4h] BYREF

    builtinIndex = -1;
    success = 0;
    switch ( breakpointType )
    {
        case 4u:
        case 5u:
            codePos = (char *)Scr_GetOpcodePosOfType(inst, this->bufferIndex, startSourcePos, endSourcePos, 1, &sourcePos);
            if ( codePos && (*Scr_FindBreakpointInfo(inst, codePos) != 127 || (unsigned __int8)*codePos != 135) )
                success = 1;
            break;
        case 6u:
        case 7u:
            goto $LN6_152;
        default:
            v8 = va("unreachable: %d", breakpointType);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp", 857, 0, v8) )
                __debugbreak();
$LN6_152:
            codePosa = (unsigned __int8 *)Scr_GetOpcodePosOfType(
                                                                            inst,
                                                                            this->bufferIndex,
                                                                            startSourcePos,
                                                                            endSourcePos,
                                                                            8,
                                                                            &sourcePos);
            if ( codePosa )
            {
                success = 1;
                v11 = *(_WORD *)(codePosa + 1);
                builtinIndex = v11;
                if ( v11 >= gScrCompilePub[inst].func_table_size
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                                869,
                                0,
                                "%s\n\t(builtinIndex) = %i",
                                "(builtinIndex >= 0 && builtinIndex < gScrCompilePub[inst].func_table_size)",
                                v11) )
                {
                    __debugbreak();
                }
                if ( breakpointType == 6 )
                    ++gScrVmDebugPub[inst].func_table[v11].breakpointCount;
            }
            codePos = 0;
            break;
    }
    if ( Sys_IsRemoteDebugServer() )
        Sys_WriteDebugSocketInt(success);
    if ( !success )
        return 0;
    if ( Sys_IsRemoteDebugServer() )
    {
        Sys_WriteDebugSocketInt(this->bufferIndex);
        Sys_WriteDebugSocketInt(sourcePos);
    }
    //newElement = Scr_ScriptWatch::CreateBreakpointElement(
    //                             &gScrDebuggerGlob[inst].scriptWatch,
    //                             inst,
    //                             element,
    //                             this->bufferIndex,
    //                             sourcePos,
    //                             user);
    newElement = gScrDebuggerGlob[inst].scriptWatch.CreateBreakpointElement(inst, element, this->bufferIndex, sourcePos, user);
    Scr_ScriptWindow::AddBreakpoint(inst, pBreakpoint, codePos, builtinIndex, newElement, breakpointType);
    return 1;
}

void __thiscall Scr_ScriptWindow::ToggleBreakpointInternal(
                scriptInstance_t inst,
                Scr_WatchElement_s *element,
                bool force,
                bool overwrite,
                unsigned __int8 breakpointType,
                bool user)
{
    Scr_Breakpoint *breakpoint; // [esp+8h] [ebp-24h]
    unsigned int startSourcePos; // [esp+Ch] [ebp-20h] BYREF
    bool movedSelectedLine; // [esp+13h] [ebp-19h]
    SourceBufferInfo *sourceBufData; // [esp+14h] [ebp-18h]
    const char *s; // [esp+18h] [ebp-14h]
    Scr_WatchElement_s *breakpointElement; // [esp+1Ch] [ebp-10h]
    int line; // [esp+20h] [ebp-Ch]
    Scr_Breakpoint **pBreakpoint; // [esp+24h] [ebp-8h]
    unsigned int endSourcePos; // [esp+28h] [ebp-4h] BYREF

    if ( this->selectedLine < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    919,
                    0,
                    "%s",
                    "selectedLine >= 0") )
    {
        __debugbreak();
    }
    sourceBufData = &gScrParserPub[inst].sourceBufferLookup[this->bufferIndex];
    if ( Sys_IsRemoteDebugServer() )
    {
        Sys_WriteDebugSocketInt(sourceBufData->sortedIndex);
        Sys_WriteDebugSocketInt(this->selectedLine);
        Scr_WriteElement(element);
        Sys_WriteDebugSocketInt(force);
        Sys_WriteDebugSocketInt(overwrite);
        Sys_WriteDebugSocketInt(breakpointType);
        Sys_WriteDebugSocketInt(user);
    }
    if ( breakpointType >= 6u && breakpointType <= 7u )
    {
        Scr_ScriptWindow::GetSourcePos(inst, &startSourcePos, &endSourcePos);
        Scr_ScriptWindow::AddBreakpointAtSourcePos(
            inst,
            element,
            breakpointType,
            user,
            &this->builtinHead,
            startSourcePos,
            endSourcePos);
        return;
    }
    if (overwrite && element)
    {
        //element = Scr_ScriptWatch::DeleteElementInternal(&gScrDebuggerGlob[inst].scriptWatch, inst, element);
        element = gScrDebuggerGlob[inst].scriptWatch.DeleteElementInternal(inst, element);
    }
    s = sourceBufData->sourceBuf;
    line = 0;
    pBreakpoint = &this->breakpointHead;
    movedSelectedLine = 0;
    while ( 1 )
    {
        while ( 1 )
        {
            breakpoint = *pBreakpoint;
            if ( !*pBreakpoint )
                goto LABEL_29;
            if ( breakpoint->line >= this->selectedLine )
                break;
            pBreakpoint = &breakpoint->next;
        }
        if ( breakpoint->line == this->selectedLine )
            break;
LABEL_29:
        if ( !force )
            element = 0;
        while ( 1 )
        {
            if ( s - sourceBufData->sourceBuf > sourceBufData->len
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                            999,
                            0,
                            "%s",
                            "s - sourceBufData->sourceBuf <= sourceBufData->len") )
            {
                __debugbreak();
            }
            if ( line == this->selectedLine )
                break;
            while ( *s )
                ++s;
            ++s;
            ++line;
        }
        startSourcePos = s - sourceBufData->sourceBuf;
        while ( *s )
            ++s;
        endSourcePos = s - sourceBufData->sourceBuf;
        if ( Scr_ScriptWindow::AddBreakpointAtSourcePos(
                     inst,
                     element,
                     breakpointType,
                     user,
                     pBreakpoint,
                     startSourcePos,
                     endSourcePos)
            || this->selectedLine >= this->numLines - 1 )
        {
            return;
        }
        movedSelectedLine = 1;
        if ( Sys_IsRemoteDebugServer() )
            ++this->selectedLine;
        if ( *s )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                            1028,
                            0,
                            "%s",
                            "!(*s)") )
                __debugbreak();
        }
        ++s;
        ++line;
    }
    if ( force )
    {
        if ( element == breakpoint->element )
            element = element->next;
        Scr_FreeLineBreakpoint(inst, breakpoint, 1);
        goto LABEL_29;
    }
    breakpointElement = breakpoint->element;
    if ( !breakpointElement
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    979,
                    0,
                    "%s",
                    "breakpointElement") )
    {
        __debugbreak();
    }
    if ( breakpointElement->breakpointType == breakpointType )
    {
        if ( !movedSelectedLine )
            Scr_FreeLineBreakpoint(inst, breakpoint, 1);
    }
    else
    {
        breakpointElement->breakpointType = breakpointType;
    }
}

void __cdecl Scr_WriteElement(Scr_WatchElement_s *element)
{
    if ( element )
        Sys_WriteDebugSocketInt(element->id);
    else
        Sys_WriteDebugSocketInt(0);
}

void __cdecl Scr_FreeLineBreakpoint(scriptInstance_t inst, Scr_Breakpoint *breakpoint, bool deleteElement)
{
    unsigned __int8 breakpointType; // [esp+7h] [ebp-9h]
    Scr_Breakpoint **pBreakpoint; // [esp+8h] [ebp-8h]
    Scr_WatchElement_s *element; // [esp+Ch] [ebp-4h]

    pBreakpoint = breakpoint->prev;
    element = breakpoint->element;
    if ( !element
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp", 741, 0, "%s", "element") )
    {
        __debugbreak();
    }
    breakpointType = element->breakpointType;
    breakpoint->element = 0;
    if (deleteElement)
    {
        //Scr_ScriptWatch::DeleteElementInternal(&gScrDebuggerGlob[inst].scriptWatch, inst, element);
        gScrDebuggerGlob[inst].scriptWatch.DeleteElementInternal(inst, element);
    }
    if ( breakpointType >= 4u && breakpointType <= 5u )
    {
        if ( !breakpoint->codePos
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                        756,
                        0,
                        "%s",
                        "breakpoint->codePos") )
        {
            __debugbreak();
        }
        Scr_RemoveManualBreakpoint(inst, (unsigned __int8 *)breakpoint->codePos);
    }
    if ( pBreakpoint != breakpoint->prev
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    762,
                    0,
                    "%s",
                    "pBreakpoint == breakpoint->prev") )
    {
        __debugbreak();
    }
    if ( breakpoint->next )
        breakpoint->next->prev = pBreakpoint;
    *pBreakpoint = breakpoint->next;
    Scr_FreeBreakpoint(breakpoint);
}

void __cdecl Scr_RemoveManualBreakpoint(scriptInstance_t inst, unsigned __int8 *codePos)
{
    if ( (unsigned __int8 *)gScrDebuggerGlob[inst].nextBreakpointCodePos == codePos
        && gScrDebuggerGlob[inst].nextBreakpointCodePosMasked )
    {
        gScrDebuggerGlob[inst].nextBreakpointCodePosMasked = 0;
    }
    else if ( *codePos == 135 )
    {
        Scr_RemoveBreakpoint(inst, codePos);
    }
    else
    {
        if ( *codePos != 137
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                        263,
                        0,
                        "%s",
                        "*codePos == OP_manualAndAssignmentBreakpoint") )
        {
            __debugbreak();
        }
        *codePos = -120;
    }
}

void __cdecl Scr_RemoveBreakpoint(scriptInstance_t inst, unsigned __int8 *codePos)
{
    unsigned __int8 *breakpoint; // [esp+0h] [ebp-4h]

    breakpoint = (unsigned __int8 *)Scr_FindBreakpointInfo(inst, (const char *)codePos);
    if ( *breakpoint == 127
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    191,
                    0,
                    "%s",
                    "*breakpoint != OP_NOP") )
    {
        __debugbreak();
    }
    *codePos = *breakpoint;
    *breakpoint = 127;
    if ( !gScrDebuggerGlob[inst].breakpointCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    196,
                    0,
                    "%s",
                    "gScrDebuggerGlob[inst].breakpointCount") )
    {
        __debugbreak();
    }
    --gScrDebuggerGlob[inst].breakpointCount;
}

char *__thiscall Scr_ScriptWindow::GetBreakpointCodePos(scriptInstance_t inst)
{
    unsigned int startSourcePos; // [esp+4h] [ebp-1Ch]
    SourceBufferInfo *sourceBufData; // [esp+8h] [ebp-18h]
    const char *s; // [esp+Ch] [ebp-14h]
    int line; // [esp+10h] [ebp-10h]
    char *codePos; // [esp+14h] [ebp-Ch]
    unsigned int sourcePos; // [esp+18h] [ebp-8h] BYREF
    unsigned int endSourcePos; // [esp+1Ch] [ebp-4h]

    if ( this->selectedLine < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    1063,
                    0,
                    "%s",
                    "selectedLine >= 0") )
    {
        __debugbreak();
    }
    sourceBufData = &gScrParserPub[inst].sourceBufferLookup[this->bufferIndex];
    s = sourceBufData->sourceBuf;
    for ( line = 0; ; ++line )
    {
        if ( s - sourceBufData->sourceBuf > sourceBufData->len
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                        1071,
                        0,
                        "%s",
                        "s - sourceBufData->sourceBuf <= sourceBufData->len") )
        {
            __debugbreak();
        }
        if ( line == this->selectedLine )
            break;
LABEL_19:
        while ( *s )
            ++s;
        ++s;
    }
    startSourcePos = s - sourceBufData->sourceBuf;
    while ( *s )
        ++s;
    endSourcePos = s - sourceBufData->sourceBuf;
    codePos = (char *)Scr_GetOpcodePosOfType(inst, this->bufferIndex, startSourcePos, endSourcePos, 1, &sourcePos);
    if ( codePos )
        return codePos;
    if ( this->selectedLine < this->numLines - 1 )
    {
        ++this->selectedLine;
        if ( *s
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp", 1088, 0, "%s", "!(*s)") )
        {
            __debugbreak();
        }
        goto LABEL_19;
    }
    return 0;
}

void __cdecl Scr_MonitorCommand(const char *text, scriptInstance_t inst)
{
    if ( !text
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp", 1245, 0, "%s", "text") )
    {
        __debugbreak();
    }
    if ( !I_strnicmp(text, "map_restart", 11) && (text[11] <= 32 || text[11] == 59) )
        gScrDebuggerGlob[inst].abort = 1;
}

void __thiscall Scr_ScriptWindow::RunToCursor(scriptInstance_t inst)
{
    char *codePos; // [esp+4h] [ebp-4h]

    if ( Sys_IsRemoteDebugServer() && gScrDebuggerGlob[inst].atBreakpoint )
        Scr_ResumeBreakpoints(inst);
    if (gScrVmPub[inst].function_count)
    {
        codePos = Scr_ScriptWindow::GetBreakpointCodePos(inst);
        Scr_SetTempBreakpoint(inst, codePos, 0);
    }
}

void __cdecl Scr_ResumeBreakpoints(scriptInstance_t inst)
{
    Scr_WatchElement_s *element; // [esp+0h] [ebp-4h]

    gScrDebuggerGlob[inst].atBreakpoint = 0;
    gScrDebuggerGlob[inst].breakpointPos.bufferIndex = -1;
    for ( element = gScrDebuggerGlob[inst].scriptWatch.elementHead; element; element = element->next )
    {
        element->changed = 0;
        element->hitBreakpoint = 0;
    }
}

void __cdecl Scr_SetTempBreakpoint(scriptInstance_t inst, char *codePos, unsigned int threadId)
{
    if ( codePos )
    {
        if ( gScrDebuggerGlob[inst].killThreadCodePos )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                            1265,
                            0,
                            "%s",
                            "!gScrDebuggerGlob[inst].killThreadCodePos") )
                __debugbreak();
        }
        gScrDebuggerGlob[inst].nextBreakpointCodePos = codePos;
        Scr_AddManualBreakpoint(inst, (unsigned __int8 *)gScrDebuggerGlob[inst].nextBreakpointCodePos);
        gScrDebuggerGlob[inst].prevBreakpointLineNum = gScrDebuggerGlob[inst].breakpointPos.lineNum;
        gScrDebuggerGlob[inst].nextBreakpointThreadId = threadId;
    }
}

void __thiscall Scr_ScriptWindow::EnterCallInternal(scriptInstance_t inst)
{
    const char *v3; // [esp+4h] [ebp-30h]
    Scr_SourcePos_t pos; // [esp+Ch] [ebp-28h] BYREF
    Scr_Breakpoint *breakpoint; // [esp+18h] [ebp-1Ch]
    unsigned int startSourcePos; // [esp+1Ch] [ebp-18h] BYREF
    const char *codePos; // [esp+20h] [ebp-14h]
    unsigned int sourcePos; // [esp+24h] [ebp-10h] BYREF
    Scr_Breakpoint **pBreakpoint; // [esp+28h] [ebp-Ch]
    const char *destCodePos; // [esp+2Ch] [ebp-8h]
    unsigned int endSourcePos; // [esp+30h] [ebp-4h] BYREF

    if ( this->selectedLine < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    1479,
                    0,
                    "%s",
                    "selectedLine >= 0") )
    {
        __debugbreak();
    }
    Scr_ScriptWindow::GetSourcePos(inst, &startSourcePos, &endSourcePos);
    codePos = Scr_GetOpcodePosOfType(inst, this->bufferIndex, startSourcePos, endSourcePos, 2, &sourcePos);
    if ( codePos )
    {
        for ( pBreakpoint = &this->breakpointHead; ; pBreakpoint = &breakpoint->next )
        {
            breakpoint = *pBreakpoint;
            if ( !breakpoint )
                break;
            if ( breakpoint->line >= this->selectedLine )
            {
                if ( breakpoint->line != this->selectedLine )
                    breakpoint = 0;
                break;
            }
        }
        if ( breakpoint )
        {
            if ( Sys_IsRemoteDebugServer() )
            {
                Sys_WriteDebugSocketMessageType(0x11u);
                Scr_WriteElement(breakpoint->element);
                Sys_EndWriteDebugSocket();
            }
        }
        else
        {
            if ( Sys_IsRemoteDebugServer() )
                Sys_WriteDebugSocketMessageType(0x15u);
            Scr_ScriptWindow::ToggleBreakpointInternal(inst, 0, 0, 0, 4u, 1);
            if ( Sys_IsRemoteDebugServer() )
                Sys_EndWriteDebugSocket();
        }
        v3 = *(const char **)++codePos;
        codePos += 4;
        destCodePos = v3;
        Scr_GetSourcePosOfType(inst, v3 - 1, 4, &pos);
        if ( Sys_IsRemoteDebugServer() )
        {
            Sys_WriteDebugSocketMessageType(0x28u);
            Sys_WriteDebugSocketInt(pos.bufferIndex);
            Sys_WriteDebugSocketInt(pos.lineNum);
            Sys_EndWriteDebugSocket();
        }
    }
}

void __thiscall Scr_ScriptWindow::Init(scriptInstance_t inst)
{
    int v2; // [esp+0h] [ebp-1Ch]
    SourceBufferInfo *sourceBufData; // [esp+Ch] [ebp-10h]
    const char *s; // [esp+10h] [ebp-Ch]
    int i; // [esp+14h] [ebp-8h]
    int col; // [esp+18h] [ebp-4h]

    this->breakpointHead = 0;
    this->builtinHead = 0;
    this->numLines = 0;
    if ( this->bufferIndex != -1 )
    {
        if ( this->bufferIndex >= gScrParserPub[inst].sourceBufferLookupLen
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                        1858,
                        0,
                        "bufferIndex doesn't index gScrParserPub[inst].sourceBufferLookupLen\n\t%i not in [0, %i)",
                        this->bufferIndex,
                        gScrParserPub[inst].sourceBufferLookupLen) )
        {
            __debugbreak();
        }
        sourceBufData = &gScrParserPub[inst].sourceBufferLookup[this->bufferIndex];
        s = sourceBufData->sourceBuf;
        this->numCols = 0;
        col = 0;
        for ( i = 0; i < sourceBufData->len; ++i )
        {
            if ( *s )
            {
                if ( *s == 9 )
                    v2 = 4 - col % 4;
                else
                    v2 = 1;
                col += v2;
            }
            else
            {
                if ( col > this->numCols )
                    this->numCols = col;
                ++this->numLines;
                col = 0;
            }
            ++s;
        }
        if ( i && *(s - 1) )
            ++this->numLines;
    }
}

void __thiscall Scr_AbstractScriptList::Init(scriptInstance_t inst)
{
    this->numLines = 0;
    this->scriptWindows = 0;
}

void __thiscall Scr_AbstractScriptList::Shutdown(scriptInstance_t inst)
{
    if ( this->scriptWindows )
    {
        Scr_FreeDebugMem(inst, (char *)this->scriptWindows);
        this->scriptWindows = 0;
    }
}

void __cdecl Scr_FreeDebugMem(scriptInstance_t inst, char *ptr)
{
    Z_Free(ptr, 0);
}

void __thiscall Scr_ScriptList::AddFile(
                scriptInstance_t inst,
                const char *filename,
                Scr_AddFileInfo *info)
{
    Scr_ScriptWindow *scriptWindow; // [esp+Ch] [ebp-4h]

    //scriptWindow = (Scr_ScriptWindow *)Scr_ScriptWindow::operator new(0x20u);
    scriptWindow = new Scr_ScriptWindow();
    this->scriptWindows[info->to] = scriptWindow;
    //Scr_ScriptWindow::SetScriptFile(scriptWindow, inst, filename);
    scriptWindow->SetScriptFile(inst, filename);
    if ( scriptWindow->bufferIndex == -1 )
    {
        //Scr_ScriptWindow::operator delete(scriptWindow);
        delete scriptWindow;
        --this->numLines;
    }
    else
    {
        gScrParserPub[inst].sourceBufferLookup[scriptWindow->bufferIndex].sortedIndex = info->to++;
        if ( scriptWindow->numCols > info->maxNumCols )
            info->maxNumCols = scriptWindow->numCols;
    }
    ++info->from;
}

void __thiscall Scr_ScriptList::Init(scriptInstance_t inst)
{
    unsigned int VariableName; // eax
    char *v3; // eax
    char filename[64]; // [esp+50h] [ebp-858h] BYREF
    Scr_AddFileInfo info; // [esp+94h] [ebp-814h] BYREF
    const char *scriptWindowsNames[512]; // [esp+A0h] [ebp-808h] BYREF
    int i; // [esp+8A0h] [ebp-8h]
    unsigned int id; // [esp+8A4h] [ebp-4h]

    Scr_AbstractScriptList::Init(inst);
    ++this->numLines;
    for ( id = FindFirstSibling(inst, gScrCompilePub[inst].loadedscripts); id; id = FindNextSibling(inst, id) )
        ++this->numLines;
    if ( this->numLines > 512 )
        Com_Error(ERR_DROP, "MAX_SCRIPT_WINDOW_NAMES exceeded");
    scriptWindowsNames[0] = "scriptdebugger/help.txt";
    i = 1;
    for ( id = FindFirstSibling(inst, gScrCompilePub[inst].loadedscripts); id; id = FindNextSibling(inst, id) )
    {
        VariableName = GetVariableName(inst, id);
        v3 = SL_ConvertToString(VariableName, inst);
        scriptWindowsNames[i++] = v3;
    }
    qsort(
        &scriptWindowsNames[1],
        this->numLines - 1,
        4u,
        (int (__cdecl *)(const void *, const void *))ConDrawInput_CompareStrings);
    this->scriptWindows = (Scr_ScriptWindow **)Scr_AllocDebugMem(inst, 4 * this->numLines, "Scr_ScriptList::Init2");
    memset(&info, 0, sizeof(info));
    Hunk_CheckTempMemoryHighClear();
    Scr_AddSourceBuffer(inst, 0, (char*)"scriptdebugger/help.txt", 0, 0);
    Hunk_ClearTempMemoryHigh();
    Scr_ScriptList::AddFile(inst, "scriptdebugger/help.txt", &info);
    if ( info.to )
        this->selectedLine = 0;
    for ( id = FindFirstSibling(inst, gScrCompilePub[inst].loadedscripts); id; id = FindNextSibling(inst, id) )
    {
        Com_sprintf(filename, 0x40u, "%s.gsc", scriptWindowsNames[info.from]);
        Scr_ScriptList::AddFile(inst, filename, &info);
    }
    gScrDebuggerGlob[inst].colBuf = (char *)Hunk_UserAlloc(
                                                                                        g_DebugHunkUser,
                                                                                        info.maxNumCols + 1,
                                                                                        4,
                                                                                        "Scr_ScriptList::Init3");
}

unsigned int *__cdecl Scr_AllocDebugMem(scriptInstance_t inst, int size, const char *name)
{
    return Z_Malloc(size, name, 0);
}

void __thiscall Scr_ScriptList::Shutdown(scriptInstance_t inst)
{
    int i; // [esp+8h] [ebp-4h]

    if ( !gScrDebuggerGlob[inst].colBuf
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    2553,
                    0,
                    "%s",
                    "gScrDebuggerGlob[inst].colBuf") )
    {
        __debugbreak();
    }
    Hunk_UserFree(g_DebugHunkUser, gScrDebuggerGlob[inst].colBuf);
    for (i = 0; i < this->numLines; ++i)
    {
        //Scr_ScriptWindow::operator delete(this->scriptWindows[i]);
        delete this->scriptWindows[i];
    }
    Scr_AbstractScriptList::Shutdown(inst);
}

void __thiscall Scr_ScriptCallStack::UpdateStack(scriptInstance_t inst)
{
    Scr_SourcePos2_t *pos; // [esp+4h] [ebp-14h]
    function_frame_t *v4; // [esp+8h] [ebp-10h]
    unsigned int index; // [esp+Ch] [ebp-Ch]
    int i; // [esp+10h] [ebp-8h]
    char *codePos; // [esp+14h] [ebp-4h]

    if (gScrVmPub[inst].function_count)
    {
        this->numLines = gScrVmPub[inst].function_count + 1;
        for (i = 0; i <= gScrVmPub[inst].function_count; ++i)
        {
            if (i)
            {
                v4 = &gScrVmPub[inst].function_frame_start[gScrVmPub[inst].function_count - i];
                codePos = (char *)v4->fs.pos;
                index = v4->fs.localId == 0;
            }
            else
            {
                codePos = (char *)(gScrDebuggerGlob[inst].breakpointCodePos + 1);
                index = 0;
            }
            pos = &this->stack[i];
            if (codePos == &g_EndPos)
            {
                pos->bufferIndex = -1;
                this->stack[i].sourcePos = 0;
            }
            else
            {
                pos->bufferIndex = Scr_GetSourceBuffer(inst, codePos - 1);
                this->stack[i].sourcePos = Scr_GetPrevSourcePos(inst, codePos - 1, index);
            }
        }
    }
    else
    {
        this->numLines = 0;
    }
}

Scr_WatchElement_s *__thiscall Scr_ScriptWatch::GetElementWithId_r(Scr_WatchElement_s *element, int id)
{
    Scr_WatchElement_s *childElement; // [esp+4h] [ebp-4h]

    while ( element )
    {
        if ( element->id == id )
            return element;
        if ( element->childHead )
        {
            childElement = Scr_ScriptWatch::GetElementWithId_r(element->childHead, id);
            if ( childElement )
                return childElement;
        }
        element = element->next;
    }
    return 0;
}

Scr_WatchElement_s *__thiscall Scr_ScriptWatch::GetElementWithId(int id)
{
    return Scr_ScriptWatch::GetElementWithId_r(this->elementHead, id);
}

Scr_WatchElement_s **__thiscall Scr_ScriptWatch::GetElementRef(Scr_WatchElement_s *element)
{
    Scr_WatchElement_s **pElement; // [esp+4h] [ebp-4h]

    for ( pElement = &this->elementHead; *pElement != element; pElement = &(*pElement)->next )
    {
        if ( !*pElement
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                        3386,
                        0,
                        "%s",
                        "*pElement") )
        {
            __debugbreak();
        }
    }
    return pElement;
}

Scr_WatchElement_s *__thiscall Scr_ScriptWatch::GetElementPrev(Scr_WatchElement_s *element)
{
    Scr_WatchElement_s *prevElement; // [esp+4h] [ebp-4h]

    if ( this->elementHead == element )
        return 0;
    for ( prevElement = this->elementHead; prevElement; prevElement = prevElement->next )
    {
        if ( prevElement->next == element )
            return prevElement;
    }
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp", 3410, 0, "unreachable") )
        __debugbreak();
    return 0;
}

void __thiscall Scr_ScriptWatch::ToggleExpandElement(scriptInstance_t inst, Scr_WatchElement_s *element)
{
    element->expand = !element->expand;
    if ( element->expand )
    {
        if ( element->objectType )
            Scr_ScriptWatch::EvaluateWatchChildren(inst, element);
        else
            element->expand = 0;
    }
    if ( !element->expand )
        Scr_FreeWatchElementChildrenStrict(inst, element);
    if ( Sys_IsRemoteDebugServer() )
    {
        Sys_WriteDebugSocketMessageType(0x25u);
        Sys_EndWriteDebugSocket();
    }
}

void __thiscall Scr_ScriptWatch::ExpandElement(
                scriptInstance_t inst,
                Scr_WatchElement_s *element,
                bool expand)
{
    Scr_WatchElement_s *childHead; // [esp+0h] [ebp-Ch]

    if ( element->expand == expand )
    {
        if ( expand )
            childHead = element->childHead;
        else
            childHead = element->parent;
        if ( childHead )
        {
            if ( Sys_IsRemoteDebugServer() )
            {
                Sys_WriteDebugSocketMessageType(0x11u);
                Scr_WriteElement(childHead);
                Sys_EndWriteDebugSocket();
            }
        }
    }
    else
    {
        if ( Sys_IsRemoteDebugServer() )
        {
            Sys_WriteDebugSocketMessageType(0x12u);
            Scr_WriteElement(element);
            Sys_EndWriteDebugSocket();
        }
        Scr_ScriptWatch::ToggleExpandElement(inst, element);
    }
}

Scr_WatchElement_s *__thiscall Scr_ScriptWatch::CreateBreakpointElement(
                scriptInstance_t inst,
                Scr_WatchElement_s *element,
                unsigned int bufferIndex,
                unsigned int sourcePos,
                bool user)
{
    Scr_WatchElement_s **ElementRef; // eax
    Scr_WatchElement_s *ElementRoot; // [esp+0h] [ebp-1ACh]
    Scr_WatchElement_s **pElement; // [esp+8h] [ebp-1A4h]
    char refText[136]; // [esp+Ch] [ebp-1A0h] BYREF
    char valueText[268]; // [esp+94h] [ebp-118h] BYREF
    Scr_WatchElement_s *newElement; // [esp+1A4h] [ebp-8h]
    int lineNum; // [esp+1A8h] [ebp-4h]

    if ( element )
    {
        ElementRoot = Scr_GetElementRoot(element);
        ElementRef = Scr_ScriptWatch::GetElementRef(ElementRoot);
    }
    else
    {
        ElementRef = Scr_ScriptWatch::GetElementRef(0);
    }
    pElement = ElementRef;
    lineNum = Scr_GetSourcePos(inst, bufferIndex, sourcePos, valueText, 0x101u) + 1;
    Com_sprintf(refText, 0x81u, "%i %s", lineNum, gScrParserPub[inst].sourceBufferLookup[bufferIndex].buf);
    newElement = Scr_ScriptWatch::CreateWatchElement(
                                 inst,
                                 refText,
                                 pElement,
                                 "Scr_ScriptWatch::CreateBreakpointElement");
    ReplaceString(&newElement->valueText, valueText, "CreateBreakpointElement", 0, inst);
    return newElement;
}

Scr_WatchElement_s *__cdecl Scr_GetElementRoot(Scr_WatchElement_s *element)
{
    while ( element->parent )
        element = element->parent;
    return element;
}

Scr_WatchElement_s *__thiscall Scr_ScriptWatch::PasteNonBreakpointElement(
                scriptInstance_t inst,
                Scr_WatchElement_s *element,
                char *text,
                bool user)
{
    Scr_WatchElement_s **ElementRef; // eax
    Scr_WatchElement_s *ElementRoot; // [esp+0h] [ebp-10h]
    Scr_WatchElement_s *newElement; // [esp+Ch] [ebp-4h]

    if ( element )
    {
        ElementRoot = Scr_GetElementRoot(element);
        ElementRef = Scr_ScriptWatch::GetElementRef(ElementRoot);
    }
    else
    {
        ElementRef = Scr_ScriptWatch::GetElementRef(0);
    }
    newElement = Scr_ScriptWatch::CreateWatchElement(
                                 inst,
                                 text,
                                 ElementRef,
                                 "Scr_ScriptWatch::PasteNonBreakpointElement");
    if ( newElement->breakpoint
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    3740,
                    0,
                    "%s",
                    "!newElement->breakpoint") )
    {
        __debugbreak();
    }
    Scr_CompileText(inst, text, &newElement->expr);
    Scr_ScriptWatch::EvaluateWatchElement(inst, newElement);
    return newElement;
}

Scr_WatchElement_s *__thiscall Scr_ScriptWatch::DeleteElementInternal(
                scriptInstance_t inst,
                Scr_WatchElement_s *element)
{
    Scr_WatchElement_s **pElement; // [esp+4h] [ebp-4h]

    if ( !element
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp", 4105, 0, "%s", "element") )
    {
        __debugbreak();
    }
    if ( element->parent )
        return element;
    pElement = Scr_ScriptWatch::GetElementRef(element);
    *pElement = element->next;
    Scr_ScriptWatch::FreeWatchElement(inst, element);
    return *pElement;
}

Scr_WatchElement_s *__thiscall Scr_ScriptWatch::BackspaceElementInternal(scriptInstance_t inst, Scr_WatchElement_s *element)
{
    Scr_WatchElement_s **pElement; // [esp+4h] [ebp-8h]
    Scr_WatchElement_s *prevElement; // [esp+8h] [ebp-4h]

    if ( !element
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp", 4195, 0, "%s", "element") )
    {
        __debugbreak();
    }
    if ( element->parent )
        return element;
    pElement = Scr_ScriptWatch::GetElementRef(element);
    prevElement = Scr_ScriptWatch::GetElementPrev(element);
    *pElement = element->next;
    Scr_ScriptWatch::FreeWatchElement(inst, element);
    return prevElement;
}

void __thiscall Scr_ScriptWatch::Init(scriptInstance_t inst)
{
    this->elementHead = 0;
    this->localId = 0;
    this->elementId = 0;
}

void __thiscall Scr_ScriptWatch::Shutdown(scriptInstance_t inst)
{
    Scr_WatchElement_s *next; // [esp+4h] [ebp-4h]

    Scr_ScriptWatch::UpdateBreakpoints(inst, 0);
    Scr_UnbreakAllAssignmentPos(inst);
    while ( this->elementHead )
    {
        next = this->elementHead->next;
        Scr_ScriptWatch::FreeWatchElement(inst, this->elementHead);
        this->elementHead = next;
    }
}

void __cdecl Scr_FreeWatchElementChildrenStrict(scriptInstance_t inst, Scr_WatchElement_s *element)
{
    Scr_WatchElement_s *childElement; // [esp+0h] [ebp-8h]
    Scr_WatchElement_s *nextChildElement; // [esp+4h] [ebp-4h]

    childElement = element->childHead;
    if ( childElement )
    {
        while ( childElement )
        {
            nextChildElement = childElement->next;
            Scr_FreeWatchElementChildren(inst, childElement);
            childElement = nextChildElement;
        }
        if ( !element->childArrayHead
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                        4660,
                        0,
                        "%s",
                        "element->childArrayHead") )
        {
            __debugbreak();
        }
        Scr_FreeDebugMem(inst, (char *)element->childArrayHead);
        element->childCount = 0;
        element->childArrayHead = 0;
        element->childHead = 0;
    }
}

void __cdecl Scr_FreeWatchElementChildren(scriptInstance_t inst, Scr_WatchElement_s *element)
{
    Scr_FreeWatchElementText(inst, element);
    Scr_RemoveValue(inst, element);
    Scr_FreeWatchElementChildrenStrict(inst, element);
}

void __cdecl Scr_RemoveValue(scriptInstance_t inst, Scr_WatchElement_s *element)
{
    if ( element->valueDefined )
    {
        element->valueDefined = 0;
        RemoveRefToValue(inst, element->value.type, element->value.u);
    }
}

void __cdecl Scr_FreeWatchElementText(scriptInstance_t inst, Scr_WatchElement_s *element)
{
    if ( !element->valueText
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    4680,
                    0,
                    "%s",
                    "element->valueText") )
    {
        __debugbreak();
    }
    FreeString(element->valueText, 0, inst);
    element->valueText = 0;
    if ( !element->refText
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    4684,
                    0,
                    "%s",
                    "element->refText") )
    {
        __debugbreak();
    }
    FreeString(element->refText, 0, inst);
    element->refText = 0;
}

void __thiscall Scr_ScriptWatch::FreeWatchElement(
                scriptInstance_t inst,
                Scr_WatchElement_s *element)
{
    Scr_Breakpoint *breakpoint; // [esp+4h] [ebp-8h]

    if ( element->parent
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    4740,
                    0,
                    "%s",
                    "!element->parent") )
    {
        __debugbreak();
    }
    breakpoint = element->breakpoint;
    if ( breakpoint )
    {
        Scr_FreeWatchElementText(inst, element);
        if ( breakpoint->element )
            Scr_FreeLineBreakpoint(inst, breakpoint, 0);
        if ( element->breakpointType == 6 )
        {
            if ( (breakpoint->builtinIndex < 0 || breakpoint->builtinIndex >= gScrCompilePub[inst].func_table_size)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                            4756,
                            0,
                            "%s\n\t(breakpoint->builtinIndex) = %i",
                            "(breakpoint->builtinIndex >= 0 && breakpoint->builtinIndex < gScrCompilePub[inst].func_table_size)",
                            breakpoint->builtinIndex) )
            {
                __debugbreak();
            }
            --gScrVmDebugPub[inst].func_table[breakpoint->builtinIndex].breakpointCount;
        }
    }
    else
    {
        Scr_FreeWatchElementChildren(inst, element);
        if ( element->breakpointType )
            Scr_ScriptWatch::RemoveBreakpoint(inst, element);
        Scr_FreeDebugExpr(inst, &element->expr);
    }
    Scr_FreeDebugMem(inst, (char *)element);
}

void __thiscall Scr_ScriptWatch::EvaluateWatchChildren(
    scriptInstance_t inst,
    Scr_WatchElement_s *parentElement)
{
    unsigned int AllVariableField; // eax
    char *v4; // eax
    char *v5; // eax
    const char *CanonicalString; // eax
    unsigned __int8 v7; // [esp+8h] [ebp-13Ch]
    int v8; // [esp+Ch] [ebp-138h]
    bool v9; // [esp+10h] [ebp-134h]
    int v10; // [esp+14h] [ebp-130h]
    bool v11; // [esp+18h] [ebp-12Ch]
    int(__cdecl * v12)(const void *, const void *); // [esp+1Ch] [ebp-128h]
    int(__cdecl * v13)(unsigned int *, unsigned int *); // [esp+20h] [ebp-124h]
    unsigned __int8 v14; // [esp+24h] [ebp-120h]
    unsigned __int8 objectType; // [esp+28h] [ebp-11Ch]
    bool oldHardcodedField; // [esp+43h] [ebp-101h]
    bool oldHardcodedFielda; // [esp+43h] [ebp-101h]
    char fieldText[136]; // [esp+44h] [ebp-100h] BYREF
    unsigned int hardcodedNames[5]; // [esp+D0h] [ebp-74h] BYREF
    Scr_WatchElement_s *childElement; // [esp+E4h] [ebp-60h]
    unsigned int newIndex; // [esp+E8h] [ebp-5Ch]
    unsigned int oldChildCount; // [esp+ECh] [ebp-58h]
    int(__cdecl * compare)(const void *, const void *); // [esp+F0h] [ebp-54h]
    unsigned __int8 oldObjectType; // [esp+F6h] [ebp-4Eh]
    bool isArray; // [esp+F7h] [ebp-4Dh]
    Scr_WatchElement_s *newElements; // [esp+F8h] [ebp-4Ch]
    Scr_WatchElement_s *oldElements; // [esp+FCh] [ebp-48h]
    unsigned int hardcodedCount; // [esp+100h] [ebp-44h]
    Scr_WatchElement_s *newElement; // [esp+104h] [ebp-40h]
    int function_count; // [esp+108h] [ebp-3Ch]
    unsigned int objectId; // [esp+10Ch] [ebp-38h]
    Scr_WatchElement_s *oldElement; // [esp+110h] [ebp-34h]
    Scr_WatchElement_s **newElementOldRef; // [esp+114h] [ebp-30h]
    int compareResult; // [esp+118h] [ebp-2Ch]
    bool elementChanged; // [esp+11Fh] [ebp-25h]
    unsigned int threadId; // [esp+120h] [ebp-24h]
    unsigned int oldIndex; // [esp+124h] [ebp-20h]
    unsigned int *names; // [esp+128h] [ebp-1Ch]
    bool hardcodedField; // [esp+12Fh] [ebp-15h]
    unsigned int nameIndex; // [esp+130h] [ebp-14h]
    unsigned int count; // [esp+134h] [ebp-10h]
    VariableValue value; // [esp+138h] [ebp-Ch] BYREF
    bool setChildCount; // [esp+142h] [ebp-2h]
    bool sameType; // [esp+143h] [ebp-1h]

    oldObjectType = parentElement->oldObjectType;
    parentElement->oldObjectType = parentElement->objectType;
    if (parentElement->expand && parentElement->objectType)
    {
        if (!gScrVarPub[inst].evaluate
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                5526,
                0,
                "%s",
                "gScrVarPub[inst].evaluate"))
        {
            __debugbreak();
        }
        isArray = parentElement->objectType == 20;
        hardcodedCount = 0;
        if (parentElement->objectType == 23)
        {
            if (!parentElement->parent
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    5533,
                    0,
                    "%s",
                    "parentElement->parent"))
            {
                __debugbreak();
            }
            objectId = parentElement->parent->objectId;
            if (!objectId
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    5535,
                    0,
                    "%s",
                    "objectId"))
            {
                __debugbreak();
            }
            count = Scr_FindAllThreads(inst, objectId, 0, this->localId);
        }
        else if (parentElement->objectType == 24)
        {
            if (!parentElement->parent
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    5541,
                    0,
                    "%s",
                    "parentElement->parent"))
            {
                __debugbreak();
            }
            objectId = parentElement->parent->objectId;
            if (!objectId
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    5543,
                    0,
                    "%s",
                    "objectId"))
            {
                __debugbreak();
            }
            count = Scr_FindAllEndons(inst, objectId, 0);
        }
        else
        {
            objectId = parentElement->objectId;
            if (!objectId
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    5550,
                    0,
                    "%s",
                    "objectId"))
            {
                __debugbreak();
            }
            if (parentElement->directObject)
            {
                objectType = parentElement->objectType;
                if (objectType == 13)
                {
                    threadId = GetSafeParentLocalId(inst, parentElement->objectId);
                    if (!threadId && GetObjectType(inst, parentElement->objectId) == 13)
                    {
                        for (function_count = gScrVmPub[inst].function_count; ; --function_count)
                        {
                            if (!function_count
                                && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                                    5570,
                                    0,
                                    "%s",
                                    "function_count"))
                            {
                                __debugbreak();
                            }
                            if (parentElement->objectId == gScrVmPub[inst].function_frame_start[function_count].fs.localId)
                                break;
                        }
                        do
                            threadId = gScrVmPub[inst].function_frame_start[--function_count].fs.localId;
                        while (!threadId && function_count);
                    }
                    if (threadId)
                        hardcodedNames[hardcodedCount++] = threadId + 5;
                }
                else if (objectType > 0x10u && objectType <= 0x13u)
                {
                    hardcodedNames[hardcodedCount++] = 4;
                }
            }
            else
            {
                hardcodedNames[hardcodedCount++] = 1;
            }
            v14 = parentElement->objectType;
            if (v14 == 13)
            {
                hardcodedNames[hardcodedCount++] = 2;
                hardcodedNames[hardcodedCount++] = 3;
            }
            else if (v14 == 20)
            {
                hardcodedNames[hardcodedCount++] = 0;
            }
            if (hardcodedCount > 5
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    5618,
                    0,
                    "%s",
                    "hardcodedCount <= ARRAY_COUNT( hardcodedNames )"))
            {
                __debugbreak();
            }
            AllVariableField = Scr_FindAllVariableField(inst, objectId, 0);
            count = hardcodedCount + AllVariableField;
        }
        if (count)
        {
            names = Scr_AllocDebugMem(inst, 4 * count, "Scr_ScriptWatch::EvaluateWatchChildren");
            memcpy((unsigned __int8 *)names, (unsigned __int8 *)hardcodedNames, 4 * hardcodedCount);
            if (parentElement->objectType == 23)
            {
                Scr_FindAllThreads(inst, objectId, names, this->localId);
                compare = (int(__cdecl *)(const void *, const void *))CompareThreadIndices;
            }
            else if (parentElement->objectType == 24)
            {
                Scr_FindAllEndons(inst, objectId, names);
                if (inst == SCRIPTINSTANCE_CLIENT)
                    compare = (int(__cdecl *)(const void *, const void *))CompareArrayIndicesClient;
                else
                    compare = (int(__cdecl *)(const void *, const void *))CompareArrayIndices;
            }
            else
            {
                Scr_FindAllVariableField(inst, objectId, &names[hardcodedCount]);
                if (isArray)
                {
                    if (inst == SCRIPTINSTANCE_CLIENT)
                        v13 = CompareArrayIndicesClient;
                    else
                        v13 = CompareArrayIndices;
                    v12 = (int(__cdecl *)(const void *, const void *))v13;
                }
                else
                {
                    v12 = (int(__cdecl *)(const void *, const void *))Scr_CompareCanonicalStrings;
                }
                compare = v12;
            }
            gDebuggerInstance = inst;
            qsort(&names[hardcodedCount], count - hardcodedCount, 4u, compare);
            oldElements = parentElement->childArrayHead;
            oldChildCount = parentElement->childCount;
            newElements = (Scr_WatchElement_s *)Scr_AllocDebugMem(
                inst,
                100 * count,
                "Scr_ScriptWatch::EvaluateWatchChildren3");
            memset(&newElements->expr.parseData, 0, 100 * count);
            newElementOldRef = (Scr_WatchElement_s **)Scr_AllocDebugMem(
                inst,
                4 * count,
                "Scr_ScriptWatch::EvaluateWatchChildren");
            v11 = oldElements && parentElement->objectType == oldObjectType;
            sameType = v11;
            elementChanged = 0;
            oldIndex = 0;
            newIndex = 0;
            for (nameIndex = 0; nameIndex < count; ++nameIndex)
            {
                newElement = &newElements[newIndex];
                v4 = CopyString((char *)"", "EvaluateWatchChildren", 0, inst);
                newElement->valueText = v4;
                v5 = CopyString((char *)"", "EvaluateWatchChildren", 0, inst);
                newElement->refText = v5;
                hardcodedField = newIndex < hardcodedCount;
                if (Scr_ScriptWatch::EvaluateWatchChildElement(
                    inst,
                    parentElement,
                    names[nameIndex],
                    newElement,
                    newIndex < hardcodedCount))
                {
                    newElement->parent = parentElement;
                    if (!++this->elementId
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                            5694,
                            0,
                            "%s",
                            "elementId"))
                    {
                        __debugbreak();
                    }
                    newElement->id = this->elementId;
                    newElementOldRef[newIndex] = 0;
                    if (sameType)
                    {
                        while (oldIndex < oldChildCount)
                        {
                            oldElement = &oldElements[oldIndex];
                            oldHardcodedField = oldIndex < parentElement->hardcodedCount;
                            if (oldHardcodedField == hardcodedField)
                            {
                                if (hardcodedField)
                                    v10 = oldElement->fieldName - newElement->fieldName;
                                else
                                    v10 = compare(&oldElement->fieldName, &newElement->fieldName);
                                compareResult = v10;
                            }
                            else
                            {
                                compareResult = oldHardcodedField - hardcodedField;
                            }
                            if (compareResult >= 0)
                            {
                                Scr_RemoveValue(inst, oldElement);
                                if (compareResult)
                                {
                                    elementChanged = 1;
                                }
                                else
                                {
                                    if (!this->elementId
                                        && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                                            5728,
                                            0,
                                            "%s",
                                            "elementId"))
                                    {
                                        __debugbreak();
                                    }
                                    --this->elementId;
                                    newElement->expand = oldElement->expand;
                                    newElement->childArrayHead = oldElement->childArrayHead;
                                    newElement->childHead = oldElement->childHead;
                                    newElement->childCount = oldElement->childCount;
                                    newElement->hardcodedCount = oldElement->hardcodedCount;
                                    newElement->objectType = oldElement->objectType;
                                    newElement->oldObjectType = oldElement->oldObjectType;
                                    newElement->directObject = oldElement->directObject;
                                    newElement->bufferIndex = oldElement->bufferIndex;
                                    newElement->sourcePos = oldElement->sourcePos;
                                    newElement->changed = oldElement->changed;
                                    newElement->changedTime = oldElement->changedTime;
                                    if (!oldElement->id
                                        && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                                            5742,
                                            0,
                                            "%s",
                                            "oldElement->id"))
                                    {
                                        __debugbreak();
                                    }
                                    newElement->id = oldElement->id;
                                    for (childElement = oldElement->childHead; childElement; childElement = childElement->next)
                                        childElement->parent = newElement;
                                    newElementOldRef[newIndex] = oldElement;
                                    ++oldIndex;
                                }
                                break;
                            }
                            elementChanged = 1;
                            Scr_FreeWatchElementChildren(inst, oldElement);
                            ++oldIndex;
                        }
                    }
                    ++newIndex;
                }
                else
                {
                    Scr_FreeWatchElementText(inst, newElement);
                }
            }
            Scr_FreeDebugMem(inst, (char *)names);
            while (oldIndex < oldChildCount)
            {
                oldElement = &oldElements[oldIndex];
                elementChanged = 1;
                Scr_FreeWatchElementChildren(inst, oldElement);
                ++oldIndex;
            }
            count = newIndex;
            v9 = newIndex && (!sameType || elementChanged || count != oldChildCount);
            setChildCount = v9;
            if (Sys_IsRemoteDebugServer())
            {
                if (setChildCount)
                {
                    Sys_WriteDebugSocketMessageType(0x23u);
                    Scr_WriteElement(parentElement);
                    Sys_WriteDebugSocketInt(count);
                    Sys_WriteDebugSocketInt(sameType);
                    Sys_EndWriteDebugSocket();
                    if (sameType)
                    {
                        oldIndex = 0;
                        newIndex = 0;
                        for (nameIndex = 0; nameIndex < count; ++nameIndex)
                        {
                            newElement = &newElements[newIndex];
                            hardcodedField = newIndex < hardcodedCount;
                            while (oldIndex < oldChildCount)
                            {
                                oldElement = &oldElements[oldIndex];
                                oldHardcodedFielda = oldIndex < parentElement->hardcodedCount;
                                if (oldHardcodedFielda == hardcodedField)
                                {
                                    if (hardcodedField)
                                        v8 = oldElement->fieldName - newElement->fieldName;
                                    else
                                        v8 = compare(&oldElement->fieldName, &newElement->fieldName);
                                    compareResult = v8;
                                }
                                else
                                {
                                    compareResult = oldHardcodedFielda - hardcodedField;
                                }
                                Sys_WriteDebugSocketInt(compareResult);
                                if (compareResult >= 0)
                                {
                                    if (!compareResult)
                                        ++oldIndex;
                                    break;
                                }
                                ++oldIndex;
                            }
                            ++newIndex;
                        }
                    }
                }
            }
            for (newIndex = 0; newIndex < count; ++newIndex)
            {
                newElement = &newElements[newIndex];
                oldElement = newElementOldRef[newIndex];
                hardcodedField = newIndex < hardcodedCount;
                if (newIndex >= hardcodedCount)
                {
                    v7 = parentElement->objectType;
                    if (v7 == 20)
                    {
                        value = Scr_GetArrayIndexValue(inst, newElement->fieldName);
                        Scr_GetValueString(inst, 0, &value, 129, fieldText);
                    }
                    else if (v7 > 0x16u && v7 <= 0x18u)
                    {
                        I_strncpyz(fieldText, newElement->valueText, 129);
                    }
                    else
                    {
                        CanonicalString = Scr_GetCanonicalString(inst, newElement->fieldName);
                        I_strncpyz(fieldText, CanonicalString, 129);
                    }
                    Scr_SetElementRefText(inst, newElement, fieldText);
                    if (Sys_IsRemoteDebugServer())
                    {
                        if (oldElement)
                            Scr_DeltaElementRefText(newElement, oldElement->refText, fieldText);
                        else
                            Scr_DeltaElementRefText(newElement, "", fieldText);
                    }
                }
                else
                {
                    Scr_SetNonFieldElementRefText(inst, newElement);
                    if (Sys_IsRemoteDebugServer())
                    {
                        Sys_WriteDebugSocketMessageType(0x21u);
                        Scr_WriteElement(newElement);
                        Sys_WriteDebugSocketInt(newElement->fieldName);
                        Sys_EndWriteDebugSocket();
                    }
                }
                if (oldElement)
                    Scr_DeltaElementValueText(newElement, oldElement->valueText);
                else
                    Scr_DeltaElementValueText(newElement, "");
                if (oldElement)
                    Scr_FreeWatchElementText(inst, oldElement);
            }
            Scr_FreeDebugMem(inst, (char *)newElementOldRef);
            if (oldElements)
                Scr_FreeDebugMem(inst, (char *)oldElements);
            if (count)
            {
                parentElement->childCount = count;
                parentElement->hardcodedCount = hardcodedCount;
                parentElement->childArrayHead = newElements;
                Scr_ConnectElementChildren(parentElement);
                if (Scr_IsSortWatchElement(parentElement))
                {
                    if (Sys_IsRemoteDebugServer() && setChildCount)
                    {
                        Sys_WriteDebugSocketMessageType(0x29u);
                        Scr_WriteElement(parentElement);
                        Sys_EndWriteDebugSocket();
                    }
                    Scr_SortElementChildren(inst, parentElement);
                }
                for (newElement = parentElement->childHead; newElement; newElement = newElement->next)
                    Scr_ScriptWatch::EvaluateWatchChildren(inst, newElement);
            }
            else
            {
                Scr_FreeDebugMem(inst, (char *)newElements);
                if (parentElement->childCount)
                {
                    if (Sys_IsRemoteDebugServer())
                    {
                        Sys_WriteDebugSocketMessageType(0x1Du);
                        Scr_WriteElement(parentElement);
                        Sys_EndWriteDebugSocket();
                    }
                    Scr_FreeWatchElementChildren(inst, parentElement);
                }
            }
        }
        else if (parentElement->childCount)
        {
            if (Sys_IsRemoteDebugServer())
            {
                Sys_WriteDebugSocketMessageType(0x1Du);
                Scr_WriteElement(parentElement);
                Sys_EndWriteDebugSocket();
            }
            Scr_FreeWatchElementChildren(inst, parentElement);
        }
    }
    else
    {
        Scr_FreeWatchElementChildrenStrict(inst, parentElement);
    }
}

VariableValue __cdecl Scr_GetArrayIndexValue(scriptInstance_t inst, unsigned int name)
{
    VariableValue value; // [esp+0h] [ebp-8h]

    iassert(name);

    if (name < 0x10000)
    {
        // return (VariableValue)((unsigned __int16)name | 0x200000000LL);
        value.type = VAR_STRING;
        value.u.stringValue = (unsigned short)name;
        return value;
    }

    unsigned int limit;
    if (inst == 0)
        limit = 0x17FFE;
    else
        limit = 0x17FFD;

    if (name < limit)
    {
        value.type = VAR_POINTER;
        value.u.pointerValue = name - 0x10000;
        return value;
    }

    value.type = VAR_INTEGER;
    value.u.intValue = name - 0x800000;
    return value;
}

bool __cdecl Scr_IsSortWatchElement(Scr_WatchElement_s *element)
{
    return element->threadList;
}

int __cdecl CompareArrayIndices(unsigned int *arg1, unsigned int *arg2)
{
    int v2; // ecx
    char *v4; // [esp+10h] [ebp-30h]
    VariableValue ArrayIndexValue; // [esp+1Ch] [ebp-24h]
    unsigned int name[2]; // [esp+24h] [ebp-1Ch]
    int i; // [esp+2Ch] [ebp-14h]
    VariableValue value[2]; // [esp+30h] [ebp-10h]

    name[0] = *arg1;
    name[1] = *arg2;
    for ( i = 0; i < 2; ++i )
    {
        ArrayIndexValue = Scr_GetArrayIndexValue(SCRIPTINSTANCE_SERVER, name[i]);
        v2 = i;
        value[i].u.intValue = ArrayIndexValue.u.intValue;
        value[v2].type = ArrayIndexValue.type;
    }
    if ( value[0].type != value[1].type )
        return value[0].type - value[1].type;
    if ( value[0].type == 2 )
    {
        v4 = SL_ConvertToString(value[1].u.stringValue, gDebuggerInstance);
        return strcmp(SL_ConvertToString(value[0].u.stringValue, gDebuggerInstance), v4);
    }
    else
    {
        if ( value[0].type != 6
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                        4833,
                        0,
                        "%s",
                        "value[0].type == VAR_INTEGER") )
        {
            __debugbreak();
        }
        return value[0].u.intValue - value[1].u.intValue;
    }
}

int __cdecl CompareArrayIndicesClient(unsigned int *arg1, unsigned int *arg2)
{
    int v2; // ecx
    char *v4; // [esp+10h] [ebp-30h]
    VariableValue ArrayIndexValue; // [esp+1Ch] [ebp-24h]
    unsigned int name[2]; // [esp+24h] [ebp-1Ch]
    int i; // [esp+2Ch] [ebp-14h]
    VariableValue value[2]; // [esp+30h] [ebp-10h]

    name[0] = *arg1;
    name[1] = *arg2;
    for ( i = 0; i < 2; ++i )
    {
        ArrayIndexValue = Scr_GetArrayIndexValue(SCRIPTINSTANCE_CLIENT, name[i]);
        v2 = i;
        value[i].u.intValue = ArrayIndexValue.u.intValue;
        value[v2].type = ArrayIndexValue.type;
    }
    if ( value[0].type != value[1].type )
        return value[0].type - value[1].type;
    if ( value[0].type == 2 )
    {
        v4 = SL_ConvertToString(value[1].u.stringValue, gDebuggerInstance);
        return strcmp(SL_ConvertToString(value[0].u.stringValue, gDebuggerInstance), v4);
    }
    else
    {
        if ( value[0].type != 6
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                        4859,
                        0,
                        "%s",
                        "value[0].type == VAR_INTEGER") )
        {
            __debugbreak();
        }
        return value[0].u.intValue - value[1].u.intValue;
    }
}

void __cdecl Scr_DeltaElementValueText(Scr_WatchElement_s *element, const char *oldValueText)
{
    if ( strcmp(element->valueText, oldValueText) )
    {
        if ( Sys_IsRemoteDebugServer() )
        {
            Sys_WriteDebugSocketMessageType(0x20u);
            Scr_WriteElement(element);
            Sys_WriteDebugSocketString((char *)element->valueText);
            Sys_EndWriteDebugSocket();
        }
        else if ( *oldValueText )
        {
            element->changed = 1;
            element->changedTime = 0;
        }
    }
}

void __cdecl Scr_SetNonFieldElementRefText(scriptInstance_t inst, Scr_WatchElement_s *element)
{
    char refText[128]; // [esp+18h] [ebp-88h] BYREF
    Scr_WatchElement_s *parentElement; // [esp+9Ch] [ebp-4h]

    parentElement = element->parent;
    switch ( element->fieldName )
    {
        case 0u:
            Com_sprintf(refText, 0x80u, "%s.size", parentElement->refText);
            ReplaceString(&element->refText, refText, "Scr_SetNonFieldElementRefText", 0, inst);
            break;
        case 1u:
            ReplaceString(&element->refText, (char *)parentElement->valueText, "Scr_SetNonFieldElementRefText", 0, inst);
            break;
        case 2u:
            ReplaceString(&element->refText, (char*)"<endons>", "Scr_SetNonFieldElementRefText", 0, inst);
            element->endonList = 1;
            break;
        case 3u:
            if ( !strcmp(parentElement->refText, "<locals>") )
            {
                ReplaceString(&element->refText, (char *)"self", "Scr_SetNonFieldElementRefText", 0, inst);
            }
            else
            {
                Com_sprintf(refText, 0x80u, "%s.self", parentElement->refText);
                ReplaceString(&element->refText, refText, "Scr_SetNonFieldElementRefText", 0, inst);
            }
            break;
        case 4u:
            ReplaceString(&element->refText, (char *)"<threads>", "Scr_SetNonFieldElementRefText", 0, inst);
            element->threadList = 1;
            break;
        default:
            Com_sprintf(refText, 0x80u, "$t%i", element->fieldName - 5);
            ReplaceString(&element->refText, refText, "Scr_SetNonFieldElementRefText", 0, inst);
            break;
    }
    Scr_PostSetText(inst, element);
}

void __cdecl Scr_PostSetText(scriptInstance_t inst, Scr_WatchElement_s *element)
{
    int v2; // [esp+18h] [ebp-14Ch]
    int v3; // [esp+2Ch] [ebp-138h]
    unsigned __int8 ObjectType; // [esp+44h] [ebp-120h]
    unsigned int bufferIndex; // [esp+48h] [ebp-11Ch]
    char valueText[264]; // [esp+4Ch] [ebp-118h] BYREF
    bool directObject; // [esp+15Ah] [ebp-Ah]
    unsigned __int8 type; // [esp+15Bh] [ebp-9h]
    const char *codePos; // [esp+15Ch] [ebp-8h]
    unsigned int sourcePos; // [esp+160h] [ebp-4h]

    if ( element->threadList )
    {
        type = 23;
    }
    else if ( element->endonList )
    {
        type = 24;
    }
    else
    {
        if ( element->objectId )
            ObjectType = GetObjectType(inst, element->objectId);
        else
            ObjectType = 0;
        type = ObjectType;
        if ( ObjectType >= 0xDu && ObjectType <= 0x10u )
            type = Scr_IsEndonThread(inst, element->objectId) ? 0 : 13;
    }
    directObject = 0;
    switch ( type )
    {
        case 0xDu:
            v3 = strcmp(element->refText, element->valueText);
            directObject = v3 == 0;
            if ( !v3 )
                ReplaceString(&element->valueText, (char *)"", "Scr_PostSetText", 0, inst);
            break;
        case 0x11u:
        case 0x12u:
            directObject = strcmp(element->refText, element->valueText) == 0;
            break;
        case 0x13u:
            v2 = strcmp(element->refText, element->valueText);
            directObject = v2 == 0;
            if ( !v2 )
            {
                Scr_GetFieldValue(inst, element->objectId, "classname", 257, valueText);
                ReplaceString(&element->valueText, valueText, "Scr_PostSetText", 0, inst);
            }
            break;
        case 0x14u:
            directObject = 1;
            break;
        default:
            break;
    }
    if ( element->objectType != type || element->directObject != directObject )
    {
        element->objectType = type;
        element->directObject = directObject;
        if ( Sys_IsRemoteDebugServer() )
        {
            Sys_WriteDebugSocketMessageType(0x1Eu);
            Scr_WriteElement(element);
            Sys_WriteDebugSocketInt(type);
            Sys_WriteDebugSocketInt(directObject);
            Sys_EndWriteDebugSocket();
        }
        if ( (type == 13 || type == 21) && element->oldObjectType != 13 && element->oldObjectType != 21 )
        {
            codePos = Scr_GetElementThreadPos(inst, element);
            if ( codePos )
            {
                bufferIndex = Scr_GetSourceBuffer(inst, codePos - 1);
                sourcePos = Scr_GetPrevSourcePos(inst, codePos - 1, 0);
                element->bufferIndex = bufferIndex;
                element->sourcePos = Scr_GetClosestSourcePosOfType(inst, bufferIndex, sourcePos, 4);
            }
            else
            {
                element->bufferIndex = -1;
                element->sourcePos = 0;
            }
            if ( Sys_IsRemoteDebugServer() )
            {
                Sys_WriteDebugSocketMessageType(0x1Fu);
                Scr_WriteElement(element);
                Sys_WriteDebugSocketInt(element->bufferIndex);
                Sys_WriteDebugSocketInt(element->sourcePos);
                Sys_EndWriteDebugSocket();
            }
        }
    }
}

char *__cdecl Scr_GetElementThreadPos(scriptInstance_t inst, Scr_WatchElement_s *element)
{
    char *codePos; // [esp+0h] [ebp-4h]

    if ( element->objectType == 13 && (codePos = Scr_GetThreadPos(inst, element->objectId)) != 0 )
        return codePos;
    else
        return (char *)element->deadCodePos;
}

void __cdecl Scr_SetElementRefText(scriptInstance_t inst, Scr_WatchElement_s *element, char *fieldText)
{
    char refText[128]; // [esp+18h] [ebp-88h] BYREF
    Scr_WatchElement_s *parentElement; // [esp+9Ch] [ebp-4h]

    parentElement = element->parent;
    switch ( parentElement->objectType )
    {
        case 0xDu:
            if ( strcmp(parentElement->refText, "<locals>") )
                goto $LN4_182;
            goto LABEL_3;
        case 0x11u:
        case 0x12u:
        case 0x13u:
$LN4_182:
            Com_sprintf(refText, 0x80u, "%s.%s", parentElement->refText, fieldText);
            ReplaceString(&element->refText, refText, "Scr_SetElementRefText", 0, inst);
            break;
        case 0x14u:
            Com_sprintf(refText, 0x80u, "%s[%s]", parentElement->refText, fieldText);
            ReplaceString(&element->refText, refText, "Scr_SetElementRefText", 0, inst);
            break;
        case 0x17u:
        case 0x18u:
LABEL_3:
            ReplaceString(&element->refText, fieldText, "Scr_SetElementRefText", 0, inst);
            break;
        default:
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                            5259,
                            0,
                            "unreachable") )
                __debugbreak();
            break;
    }
    Scr_PostSetText(inst, element);
}

void __cdecl Scr_DeltaElementRefText(Scr_WatchElement_s *element, const char *oldRefText, char *fieldText)
{
    if ( !Sys_IsRemoteDebugServer()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    5279,
                    0,
                    "%s",
                    "Sys_IsRemoteDebugServer()") )
    {
        __debugbreak();
    }
    if ( strcmp(element->refText, oldRefText) )
    {
        Sys_WriteDebugSocketMessageType(0x22u);
        Scr_WriteElement(element);
        Sys_WriteDebugSocketString(fieldText);
        Sys_EndWriteDebugSocket();
    }
}

void __cdecl Scr_ConnectElementChildren(Scr_WatchElement_s *parentElement)
{
    Scr_WatchElement_s *v1; // [esp+0h] [ebp-10h]
    int newIndex; // [esp+4h] [ebp-Ch]
    Scr_WatchElement_s *newElements; // [esp+8h] [ebp-8h]
    signed int count; // [esp+Ch] [ebp-4h]

    count = parentElement->childCount;
    newElements = parentElement->childArrayHead;
    for ( newIndex = 0; newIndex < count; ++newIndex )
    {
        if ( newIndex >= count - 1 )
            v1 = 0;
        else
            v1 = &newElements[newIndex + 1];
        newElements[newIndex].next = v1;
    }
    parentElement->childHead = newElements;
}

void __cdecl Scr_SortElementChildren(scriptInstance_t inst, Scr_WatchElement_s *parentElement)
{
    unsigned int v2; // [esp+0h] [ebp-14h]
    int newIndex; // [esp+4h] [ebp-10h]
    int newIndexa; // [esp+4h] [ebp-10h]
    Scr_WatchElement_s *newElements; // [esp+8h] [ebp-Ch]
    unsigned int *elementList; // [esp+Ch] [ebp-8h]
    int count; // [esp+10h] [ebp-4h]

    if ( !gScrDebuggerGlob[inst].debugger_inited_system
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    5375,
                    0,
                    "%s",
                    "gScrDebuggerGlob[inst].debugger_inited_system") )
    {
        __debugbreak();
    }
    if ( !Scr_IsSortWatchElement(parentElement)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    5376,
                    0,
                    "%s",
                    "Scr_IsSortWatchElement( parentElement )") )
    {
        __debugbreak();
    }
    count = parentElement->childCount;
    newElements = parentElement->childArrayHead;
    elementList = Scr_AllocDebugMem(inst, 4 * count, "Scr_SortElementChildren");
    for ( newIndex = 0; newIndex < count; ++newIndex )
        elementList[newIndex] = (unsigned int)&newElements[newIndex];
    Sys_EnterCriticalSection(CRITSECT_SCRIPT_DEBUGGER_SORT);
    sortInst = inst;
    qsort(elementList, count, 4u, (int (__cdecl *)(const void *, const void *))CompareThreadElements);
    Sys_LeaveCriticalSection(CRITSECT_SCRIPT_DEBUGGER_SORT);
    for ( newIndexa = 0; newIndexa < count; ++newIndexa )
    {
        if ( newIndexa >= count - 1 )
            v2 = 0;
        else
            v2 = elementList[newIndexa + 1];
        *(unsigned int *)(elementList[newIndexa] + 96) = v2;
    }
    parentElement->childHead = (Scr_WatchElement_s *)*elementList;
    Scr_FreeDebugMem(inst, (char *)elementList);
}

int __cdecl CompareThreadElements(int *arg1, int *arg2)
{
    int elements; // [esp+8h] [ebp-8h]
    int elements_4; // [esp+Ch] [ebp-4h]

    elements = *arg1;
    elements_4 = *arg2;
    if ( gScrParserPub[sortInst].sourceBufferLookup[*(unsigned int *)(*arg1 + 72)].sortedIndex != gScrParserPub[sortInst].sourceBufferLookup[*(unsigned int *)(*arg2 + 72)].sortedIndex )
        return gScrParserPub[sortInst].sourceBufferLookup[*(unsigned int *)(*arg1 + 72)].sortedIndex
                 - gScrParserPub[sortInst].sourceBufferLookup[*(unsigned int *)(*arg2 + 72)].sortedIndex;
    if ( *(unsigned int *)(elements + 76) == *(unsigned int *)(elements_4 + 76) )
        return *(unsigned int *)(elements + 48) - *(unsigned int *)(elements_4 + 48);
    return *(unsigned int *)(elements + 76) - *(unsigned int *)(elements_4 + 76);
}

char __thiscall Scr_ScriptWatch::PostEvaluateWatchElement(
    scriptInstance_t inst,
    Scr_WatchElement_s *element,
    VariableValue *value)
{
    int type; // ecx
    unsigned int intValue; // [esp+0h] [ebp-128h]
    char valueText[268]; // [esp+18h] [ebp-110h] BYREF

    if (!gScrVarPub[inst].evaluate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            6061,
            0,
            "%s",
            "gScrVarPub[inst].evaluate"))
    {
        __debugbreak();
    }
    Scr_RemoveValue(inst, element);
    if (gScrVarPub[inst].error_message)
    {
        Com_sprintf(valueText, 0x101u, "<%s>", gScrVarPub[inst].error_message);
        ReplaceString(&element->valueText, valueText, "PostEvaluateWatchElement", 0, inst);
        element->objectId = 0;
        Scr_ClearErrorMessage(inst);
        RemoveRefToValue(inst, value->type, value->u);
        return 0;
    }
    else
    {
        Scr_GetValueString(inst, this->localId, value, 257, valueText);
        ReplaceString(&element->valueText, valueText, "PostEvaluateWatchElement", 0, inst);
        if (value->type == 1)
            intValue = value->u.intValue;
        else
            intValue = 0;
        element->objectId = intValue;
        if (element->objectId || element->breakpointType == 1 || element->breakpointType == 3)
        {
            element->valueDefined = 1;
            type = value->type;
            element->value.u.intValue = value->u.intValue;
            element->value.type = type;
        }
        else
        {
            RemoveRefToValue(inst, value->type, value->u);
        }
        return 1;
    }
}

char __thiscall Scr_ScriptWatch::EvaluateWatchChildElement(
    scriptInstance_t inst,
    Scr_WatchElement_s *element,
    unsigned int fieldName,
    Scr_WatchElement_s *childElement,
    bool hardcodedField)
{
    unsigned __int8 objectType; // [esp+4h] [ebp-2Ch]
    VariableValue value; // [esp+28h] [ebp-8h] BYREF

    if (element->breakpoint
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            6108,
            0,
            "%s",
            "!element->breakpoint"))
    {
        __debugbreak();
    }
    if (!gScrVarPub[inst].evaluate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            6110,
            0,
            "%s",
            "gScrVarPub[inst].evaluate"))
    {
        __debugbreak();
    }
    childElement->fieldName = fieldName;
    if (hardcodedField)
    {
        value.type = 1;
        value.u.intValue = element->objectId;
        switch (fieldName)
        {
        case 0u:
            AddRefToObject(inst, value.u.stringValue);
            Scr_EvalSizeValue(inst, &value);
            break;
        case 1u:
            break;
        case 2u:
        case 4u:
            value.type = 0;
            break;
        case 3u:
            value.u.stringValue = Scr_GetSelf(inst, value.u.stringValue);
            break;
        default:
            value.u.intValue = fieldName - 5;
            if (fieldName == 5
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    6166,
                    0,
                    "%s",
                    "value.u.pointerValue"))
            {
                __debugbreak();
            }
            break;
        }
        AddRefToValue(inst, value.type, value.u);
    }
    else
    {
        objectType = element->objectType;
        switch (objectType)
        {
        case 0x14u:
            value = Scr_GetArrayIndexValue(inst, fieldName);
            AddRefToValue(inst, value.type, value.u);
            Scr_EvalArrayVariable(inst, element->objectId, &value);
            break;
        case 0x17u:
            value.u.intValue = fieldName;
            value.type = 1;
            AddRefToObject(inst, fieldName);
            break;
        case 0x18u:
            value.u.intValue = fieldName;
            value.type = 2;
            SL_AddRefToString(fieldName, inst);
            break;
        default:
            Scr_EvalFieldVariable(inst, (unsigned __int16)fieldName, &value, element->objectId);
            break;
        }
    }
    return Scr_ScriptWatch::PostEvaluateWatchElement(inst, childElement, &value);
}

void __thiscall Scr_ScriptWatch::EvaluateWatchElementExpression(
    scriptInstance_t inst,
    Scr_WatchElement_s *element,
    VariableValue *value)
{
    if (element->breakpoint
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            6191,
            0,
            "%s",
            "!element->breakpoint"))
    {
        __debugbreak();
    }
    if (!element->expr.exprHead
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            6194,
            0,
            "%s",
            "expr->exprHead"))
    {
        __debugbreak();
    }
    if (!gScrVarPub[inst].evaluate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            6196,
            0,
            "%s",
            "gScrVarPub[inst].evaluate"))
    {
        __debugbreak();
    }
    if (element->valueDefined && (element->breakpointType == 1 || element->breakpointType == 3))
        Scr_EvalScriptExpression(inst, &element->expr, this->localId, value, 1, 0);
    else
        Scr_EvalScriptExpression(inst, &element->expr, this->localId, value, 0, 0);
}

void __thiscall Scr_ScriptWatch::EvaluateWatchElement(
                scriptInstance_t inst,
                Scr_WatchElement_s *element)
{
    char v3; // [esp+3h] [ebp-129h]
    char *v4; // [esp+8h] [ebp-124h]
    const char *valueText; // [esp+Ch] [ebp-120h]
    VariableValue value; // [esp+14h] [ebp-118h] BYREF
    char oldValueText[268]; // [esp+1Ch] [ebp-110h] BYREF

    Scr_ScriptWatch::EvaluateWatchElementExpression(inst, element, &value);
    valueText = element->valueText;
    v4 = oldValueText;
    do
    {
        v3 = *valueText;
        *v4++ = *valueText++;
    }
    while ( v3 );
    Scr_ScriptWatch::PostEvaluateWatchElement(inst, element, &value);
    Scr_PostSetText(inst, element);
    Scr_DeltaElementValueText(element, oldValueText);
    Scr_ScriptWatch::EvaluateWatchChildren(inst, element);
}

Scr_WatchElement_s *__thiscall Scr_ScriptWatch::CreateWatchElement(
                scriptInstance_t inst,
                char *text,
                Scr_WatchElement_s **prevElem,
                const char *name)
{
    Scr_WatchElement_s *element; // [esp+4h] [ebp-4h]

    element = Scr_CreateWatchElement(inst, text, prevElem, name);
    if ( !++this->elementId
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    6252,
                    0,
                    "%s",
                    "elementId") )
    {
        __debugbreak();
    }
    element->id = this->elementId;
    return element;
}

Scr_WatchElement_s *__cdecl Scr_CreateWatchElement(
                scriptInstance_t inst,
                char *text,
                Scr_WatchElement_s **prevElem,
                const char *name)
{
    Scr_WatchElement_s *element; // [esp+0h] [ebp-4h]

    element = (Scr_WatchElement_s *)Scr_AllocDebugMem(inst, 100, name);
    memset(&element->expr.parseData, 0, sizeof(Scr_WatchElement_s));
    element->valueText = CopyString((char *)"", "Scr_CreateWatchElement", 0, inst);
    element->refText = CopyString(text, "Scr_CreateWatchElement", 0, inst);
    element->next = *prevElem;
    *prevElem = element;
    return element;
}

void __thiscall Scr_ScriptWatch::AddElement(
    scriptInstance_t inst,
    Scr_WatchElement_s *element,
    char *text)
{
    unsigned __int8 v4; // [esp+0h] [ebp-18h]
    ScriptExpression_t scriptExpr; // [esp+8h] [ebp-10h] BYREF
    ScriptExpression_t *expr; // [esp+14h] [ebp-4h]

    if (!element
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp", 6347, 0, "%s", "element"))
    {
        __debugbreak();
    }
    if (!element->breakpoint)
    {
        scriptExpr.exprHead = 0;
        Scr_CompileText(inst, text, &scriptExpr);
        v4 = *(_BYTE *)scriptExpr.parseData.stringValue;
        if (*(_BYTE *)scriptExpr.parseData.stringValue == 85)
        {
            if (!gScrVarPub[inst].evaluate
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    6362,
                    0,
                    "%s",
                    "gScrVarPub[inst].evaluate"))
            {
                __debugbreak();
            }
            gScrVarPub[inst].evaluate = 0;
            Scr_ExecCode(inst, *(char **)(scriptExpr.parseData.stringValue + 4), this->localId);
            gScrVarPub[inst].evaluate = 1;
            SL_ShutdownSystem(inst, 2u);
            Scr_FreeDebugExpr(inst, &scriptExpr);
            Scr_ScriptWatch::Evaluate(inst);
        }
        else if (v4 > 0x55u && v4 <= 0x57u)
        {
            Scr_FreeDebugExpr(inst, &scriptExpr);
        }
        else if (element->parent)
        {
            Com_Printf(24, "Cannot change child element\n");
            Scr_FreeDebugExpr(inst, &scriptExpr);
        }
        else
        {
            if (Sys_IsRemoteDebugServer())
            {
                Sys_WriteDebugSocketMessageType(5u);
                Scr_WriteElement(element);
                Sys_WriteDebugSocketString(text);
                Sys_EndWriteDebugSocket();
            }
            expr = &element->expr;
            Scr_FreeDebugExpr(inst, &element->expr);
            ReplaceString(&element->refText, text, "Scr_ScriptWatch::AddElement", 0, inst);
            element->expr = scriptExpr;
            Scr_RemoveValue(inst, element);
            Scr_ScriptWatch::EvaluateWatchElement(inst, element);
            if (Sys_IsRemoteDebugServer())
            {
                Sys_WriteDebugSocketMessageType(0x25u);
                Sys_EndWriteDebugSocket();
            }
        }
    }
}

Scr_WatchElement_s *__thiscall Scr_ScriptWatch::CloneElement(
                scriptInstance_t inst,
                Scr_WatchElement_s *element)
{
    Scr_WatchElement_s *ElementRoot; // eax
    Scr_WatchElement_s **ElementRef; // eax
    Scr_WatchElement_s *newElement; // [esp+4h] [ebp-4h]

    ElementRoot = Scr_GetElementRoot(element);
    ElementRef = Scr_ScriptWatch::GetElementRef(ElementRoot);
    newElement = Scr_ScriptWatch::CreateWatchElement(
                                 inst,
                                 (char *)element->refText,
                                 ElementRef,
                                 "Scr_ScriptWatch::CloneElement");
    Scr_CompileText(inst, element->refText, &newElement->expr);
    return newElement;
}

void __thiscall Scr_ScriptWatch::Evaluate(scriptInstance_t inst)
{
    Scr_WatchElement_s *element; // [esp+4h] [ebp-4h]

    if (!gScrVarPub[inst].evaluate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            6602,
            0,
            "%s",
            "gScrVarPub[inst].evaluate"))
    {
        __debugbreak();
    }
    if (gScrVmPub[inst].outparamcount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            6603,
            0,
            "%s",
            "!gScrVmPub[inst].outparamcount"))
    {
        __debugbreak();
    }
    if (gScrVmPub[inst].inparamcount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            6604,
            0,
            "%s",
            "!gScrVmPub[inst].inparamcount"))
    {
        __debugbreak();
    }
    for (element = this->elementHead; element; element = element->next)
    {
        if (!element->breakpoint)
            Scr_ScriptWatch::EvaluateWatchElement(inst, element);
    }
}

void __cdecl Scr_Evaluate(scriptInstance_t inst)
{
    //Scr_ScriptWatch::Evaluate(&gScrDebuggerGlob[inst].scriptWatch, inst);
    gScrDebuggerGlob[inst].scriptWatch.Evaluate(inst);
}

void __cdecl Scr_CheckBreakonNotify(
    scriptInstance_t inst,
    unsigned int notifyListOwnerId,
    unsigned int stringValue,
    VariableValue *top,
    const char *pos,
    unsigned int localId)
{
    int hitBreakpoint; // [esp+10h] [ebp-18h]
    bool updateBreakpoints; // [esp+17h] [ebp-11h]
    Scr_WatchElement_s *element; // [esp+1Ch] [ebp-Ch]
    VariableValue newValue; // [esp+20h] [ebp-8h] BYREF

    if (gScrVmPub[inst].inparamcount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            6715,
            0,
            "%s",
            "!gScrVmPub[inst].inparamcount"))
    {
        __debugbreak();
    }
    gScrVmPub[inst].top = top;
    gScrDebuggerGlob[inst].scriptWatch.localId = 0;
    g_breakonObject = notifyListOwnerId;
    g_breakonString = stringValue;
    if (gScrVarPub[inst].evaluate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            6723,
            0,
            "%s",
            "!gScrVarPub[inst].evaluate"))
    {
        __debugbreak();
    }
    gScrVarPub[inst].evaluate = 1;
    gScrDebuggerGlob[inst].scriptWatch.localId = 0;
    updateBreakpoints = 0;
    hitBreakpoint = 0;
retry_13:
    for (element = gScrDebuggerGlob[inst].scriptWatch.elementHead; element; element = element->next)
    {
        if (!element->expr.breakonExpr)
            continue;
        if (element->breakpoint
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                6736,
                0,
                "%s",
                "!element->breakpoint"))
        {
            __debugbreak();
        }
        if (!element->expr.exprHead
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                6738,
                0,
                "%s",
                "expr->exprHead"))
        {
            __debugbreak();
        }
        g_breakonHit = 0;
        if (Scr_EvalScriptExpression(inst, &element->expr, 0, &newValue, 1, 1) && !updateBreakpoints)
        {
            Scr_ClearErrorMessage(inst);
            RemoveRefToValue(inst, newValue.type, newValue.u);
        retry2:
            updateBreakpoints = 1;
            //Scr_ScriptWatch::UpdateBreakpoints(&gScrDebuggerGlob[inst].scriptWatch, inst, 0);
            gScrDebuggerGlob[inst].scriptWatch.UpdateBreakpoints(inst, 0);
            goto retry_13;
        }
        if (gScrVarPub[inst].error_message)
        {
            Scr_ClearErrorMessage(inst);
            RemoveRefToValue(inst, newValue.type, newValue.u);
            if (!element->valueDefined)
                continue;
            if (!updateBreakpoints)
                goto retry2;
        }
        //else if (!NtCurrentTeb()->ThreadLocalStoragePointer[_tls_index]->g_breakonHit)
        else if (!g_breakonHit)
        {
            continue;
        }
        if (pos)
        {
            if (element->breakpointType == 1)
            {
                Scr_WatchElementHitBreakpoint(element, 1);
                hitBreakpoint = 1;
            }
        }
    }
    if (updateBreakpoints)
    {
        //Scr_ScriptWatch::UpdateBreakpoints(&gScrDebuggerGlob[inst].scriptWatch, inst, 1);
        gScrDebuggerGlob[inst].scriptWatch.UpdateBreakpoints(inst, 1);
    }
    if (!gScrVarPub[inst].evaluate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            6781,
            0,
            "%s",
            "gScrVarPub[inst].evaluate"))
    {
        __debugbreak();
    }
    gScrVarPub[inst].evaluate = 0;
    //NtCurrentTeb()->ThreadLocalStoragePointer[_tls_index]->g_breakonObject = 0;
    g_breakonObject = 0;
    //NtCurrentTeb()->ThreadLocalStoragePointer[_tls_index]->g_breakonString = 0;
    g_breakonString = 0;
    if (hitBreakpoint)
        Scr_SpecialBreakpoint(inst, top, pos, localId, 121, 16);
}

void __cdecl Scr_SpecialBreakpoint(
    scriptInstance_t inst,
    VariableValue *top,
    const char *pos,
    unsigned int localId,
    int opcode,
    int type)
{
    if (!pos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp", 6678, 0, "%s", "pos"))
    {
        __debugbreak();
    }
    if (gScrVmPub[inst].inparamcount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            6679,
            0,
            "%s",
            "!gScrVmPub[inst].inparamcount"))
    {
        __debugbreak();
    }
    gScrVmPub[inst].outparamcount = 0;
    if (gScrDebuggerGlob[inst].nextBreakpointCodePos)
    {
        Scr_RemoveManualBreakpoint(inst, (unsigned __int8 *)gScrDebuggerGlob[inst].nextBreakpointCodePos);
        gScrDebuggerGlob[inst].nextBreakpointCodePos = 0;
    }
    if (Scr_AllowBreakpoint(inst))
    {
        gScrDebuggerGlob[inst].breakpointTop = top;
        gScrDebuggerGlob[inst].breakpointCodePos = pos - 1;
        gScrDebuggerGlob[inst].breakpointOpcode = opcode;
        gScrDebuggerGlob[inst].scriptWatch.localId = localId;
        if (!Scr_GetSourcePosOfType(inst, pos - 1, type, &gScrDebuggerGlob[inst].breakpointPos)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                6700,
                0,
                "%s",
                "hasSourcePos"))
        {
            __debugbreak();
        }
        Scr_HitBreakpointInternal(inst);
    }
}

bool __cdecl Scr_AllowBreakpoint(scriptInstance_t inst)
{
    return !gScrDebuggerGlob[inst].disableBreakpoints;
}

void __thiscall Scr_ScriptWatch::DisplayThreadPos(
    scriptInstance_t inst,
    Scr_WatchElement_s *element)
{
    unsigned int bufferIndex; // [esp+4h] [ebp-10h]
    unsigned int lineNum; // [esp+8h] [ebp-Ch]
    char *codePos; // [esp+Ch] [ebp-8h]
    unsigned int sourcePos; // [esp+10h] [ebp-4h]

    if (gScrVarPub[inst].evaluate && (element->objectType == 21 || element->objectType == 13 && element->directObject))
    {
        codePos = Scr_GetElementThreadPos(inst, element);
        if (codePos)
        {
            bufferIndex = Scr_GetSourceBuffer(inst, codePos - 1);
            sourcePos = Scr_GetPrevSourcePos(inst, codePos - 1, 0);
            lineNum = Scr_GetLineNum(inst, bufferIndex, sourcePos);
            if (Sys_IsRemoteDebugServer())
            {
                Sys_WriteDebugSocketMessageType(0x28u);
                Sys_WriteDebugSocketInt(bufferIndex);
                Sys_WriteDebugSocketInt(lineNum);
                Sys_EndWriteDebugSocket();
            }
        }
    }
}

void __cdecl Scr_UnbreakAllAssignmentPos(scriptInstance_t inst)
{
    Scr_OpcodeList_s *opcodeElement; // [esp+0h] [ebp-4h]

    if ( gScrDebuggerGlob[inst].assignBreakpointSet )
    {
        gScrDebuggerGlob[inst].assignBreakpointSet = 0;
        for ( opcodeElement = gScrDebuggerGlob[inst].assignHead; opcodeElement; opcodeElement = opcodeElement->next )
            Scr_RemoveAssignmentBreakpoint(inst, (unsigned __int8 *)opcodeElement->codePos);
    }
}

void __cdecl Scr_RemoveAssignmentBreakpoint(scriptInstance_t inst, unsigned __int8 *codePos)
{
    if ( *codePos == 136 )
    {
        Scr_RemoveBreakpoint(inst, codePos);
    }
    else
    {
        if ( *codePos != 137
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                        303,
                        0,
                        "%s",
                        "*codePos == OP_manualAndAssignmentBreakpoint") )
        {
            __debugbreak();
        }
        *codePos = -121;
    }
}

bool __cdecl Scr_RefToVariable(scriptInstance_t inst, unsigned int id, int isObject)
{
    Scr_WatchElementNode_s **pElementNode; // [esp+Ch] [ebp-1Ch]
    Scr_WatchElementNode_s *elementNodeNext; // [esp+10h] [ebp-18h]
    Scr_WatchElementDoubleNode_t *breakpoints; // [esp+14h] [ebp-14h]
    unsigned int *elementNodec; // [esp+18h] [ebp-10h]
    Scr_WatchElementNode_s *elementNode; // [esp+18h] [ebp-10h]
    Scr_WatchElementNode_s *elementNodea; // [esp+18h] [ebp-10h]
    Scr_WatchElementNode_s *elementNodeb; // [esp+18h] [ebp-10h]
    Scr_WatchElement_s *element; // [esp+1Ch] [ebp-Ch]
    VariableValue value; // [esp+20h] [ebp-8h] BYREF
    unsigned int ida; // [esp+34h] [ebp+Ch]

    if (!id)
        return 0;
    if (isObject)
        ida = id + 1;
    else
        ida = id + 0x8000;
    if (gScrDebuggerGlob[inst].removeId)
        return gScrDebuggerGlob[inst].removeId == ida;
    if (!gScrDebuggerGlob[inst].currentElement
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            7005,
            0,
            "%s",
            "gScrDebuggerGlob[inst].currentElement"))
    {
        __debugbreak();
    }
    if (!gScrDebuggerGlob[inst].variableBreakpoints
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            7007,
            0,
            "%s",
            "gScrDebuggerGlob[inst].variableBreakpoints"))
    {
        __debugbreak();
    }
    breakpoints = gScrDebuggerGlob[inst].variableBreakpoints[ida];
    if (!breakpoints)
    {
        if (!gScrDebuggerGlob[inst].add)
            return 0;
        breakpoints = (Scr_WatchElementDoubleNode_t *)Scr_AllocDebugMem(inst, 8, "Scr_RefToVariable1");
        breakpoints->list = 0;
        breakpoints->removedList = 0;
        gScrDebuggerGlob[inst].variableBreakpoints[ida] = breakpoints;
        if (isObject)
            AddRefToObject(inst, ida - 1);
    }
    for (pElementNode = &breakpoints->list;
        *pElementNode && (*pElementNode)->element != gScrDebuggerGlob[inst].currentElement;
        pElementNode = &(*pElementNode)->next)
    {
        ;
    }
    if (gScrDebuggerGlob[inst].add)
    {
        if (*pElementNode)
            return 0;
        elementNodec = Scr_AllocDebugMem(inst, 8, "Scr_RefToVariable2");
        *elementNodec = (unsigned int)gScrDebuggerGlob[inst].currentElement;
        elementNodec[1] = (unsigned int)breakpoints->list;
        breakpoints->list = (Scr_WatchElementNode_s *)elementNodec;
    }
    else
    {
        elementNode = *pElementNode;
        if (!*pElementNode)
            return 0;
        if (elementNode->element != gScrDebuggerGlob[inst].currentElement
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                7045,
                0,
                "%s",
                "elementNode->element == gScrDebuggerGlob[inst].currentElement"))
        {
            __debugbreak();
        }
        *pElementNode = elementNode->next;
        elementNode->next = breakpoints->removedList;
        breakpoints->removedList = elementNode;
        if (!breakpoints->list)
        {
            if (isObject)
            {
                if (gScrVarPub[inst].evaluate && (!Scr_GetRefCountToObject(inst, ida - 1) || !IsFieldObject(inst, ida - 1)))
                {
                    for (elementNodea = breakpoints->removedList; elementNodea; elementNodea = elementNodea->next)
                    {
                        gScrDebuggerGlob[inst].removeId = ida;
                        element = elementNodea->element;
                        if (Scr_RefScriptExpression(inst, &elementNodea->element->expr))
                        {
                            //Scr_ScriptWatch::EvaluateWatchElementExpression(
                            //    &gScrDebuggerGlob[inst].scriptWatch,
                            //    inst,
                            //    element,
                            //    &value);
                                gScrDebuggerGlob[inst].scriptWatch.EvaluateWatchElementExpression(
                                inst,
                                element,
                                &value);
                            if (gScrVarPub[inst].error_message)
                                Scr_ClearErrorMessage(inst);
                            RemoveRefToValue(inst, value.type, value.u);
                        }
                        gScrDebuggerGlob[inst].removeId = 0;
                    }
                }
                RemoveRefToObject(inst, ida - 1);
            }
            for (elementNodeb = breakpoints->removedList; elementNodeb; elementNodeb = elementNodeNext)
            {
                elementNodeNext = elementNodeb->next;
                Scr_FreeDebugMem(inst, (char *)elementNodeb);
            }
            Scr_FreeDebugMem(inst, (char *)breakpoints);
            gScrDebuggerGlob[inst].variableBreakpoints[ida] = 0;
        }
    }
    return 0;
}

void __thiscall Scr_ScriptWatch::UpdateBreakpoints(scriptInstance_t inst, bool add)
{
    Scr_WatchElement_s *element; // [esp+8h] [ebp-4h]
    Scr_WatchElement_s *elementa; // [esp+8h] [ebp-4h]

    for ( element = this->elementHead; element; element = element->next )
    {
        if ( element->breakpointType == 1 || element->breakpointType == 3 )
        {
            if ( element->breakpoint
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                            7104,
                            0,
                            "%s",
                            "!element->breakpoint") )
            {
                __debugbreak();
            }
            if ( !element->expr.exprHead
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                            7106,
                            0,
                            "%s",
                            "expr->exprHead") )
            {
                __debugbreak();
            }
            gScrDebuggerGlob[inst].currentElement = element;
            gScrDebuggerGlob[inst].removeId = 0;
            gScrDebuggerGlob[inst].add = add;
            if ( element->valueDefined && element->value.type == 1 )
                Scr_RefToVariable(inst, element->value.u.stringValue, 1);
            Scr_RefScriptExpression(inst, &element->expr);
        }
    }
    if ( add )
    {
        for ( elementa = this->elementHead; elementa; elementa = elementa->next )
        {
            if ( elementa->breakpointType == 1 )
            {
                Scr_BreakOnAllAssignmentPos(inst);
                return;
            }
        }
        Scr_UnbreakAllAssignmentPos(inst);
    }
}

void __cdecl Scr_BreakOnAllAssignmentPos(scriptInstance_t inst)
{
    Scr_OpcodeList_s *opcodeElement; // [esp+0h] [ebp-4h]

    if ( !gScrDebuggerGlob[inst].assignBreakpointSet )
    {
        gScrDebuggerGlob[inst].assignBreakpointSet = 1;
        gScrDebuggerGlob[inst].objectId = 1;
        for ( opcodeElement = gScrDebuggerGlob[inst].assignHead; opcodeElement; opcodeElement = opcodeElement->next )
            Scr_AddAssignmentBreakpoint(inst, (unsigned __int8 *)opcodeElement->codePos);
    }
}

void __cdecl Scr_AddAssignmentBreakpoint(scriptInstance_t inst, unsigned __int8 *codePos)
{
    if ( *codePos == 135 )
    {
        *codePos = -119;
    }
    else
    {
        Scr_AddBreakpoint(inst, codePos);
        *codePos = -120;
    }
}

Scr_WatchElement_s *__thiscall Scr_ScriptWatch::AddBreakpoint(
                scriptInstance_t inst,
                Scr_WatchElement_s *element,
                unsigned __int8 type)
{
    if ( element->breakpointType == type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    7135,
                    0,
                    "%s",
                    "element->breakpointType != type") )
    {
        __debugbreak();
    }
    if ( element->parent )
        element = Scr_ScriptWatch::CloneElement(inst, element);
    element->breakpointType = type;
    return element;
}

Scr_WatchElement_s *__thiscall Scr_ScriptWatch::RemoveBreakpoint(
                scriptInstance_t inst,
                Scr_WatchElement_s *element)
{
    if ( !element->breakpointType
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    7153,
                    0,
                    "%s",
                    "element->breakpointType != SCR_BREAKPOINT_NONE") )
    {
        __debugbreak();
    }
    element->breakpointType = 0;
    Scr_RemoveValue(inst, element);
    return element;
}

void __thiscall Scr_ScriptWatch::ToggleWatchElementBreakpoint(
                scriptInstance_t inst,
                Scr_WatchElement_s *element,
                unsigned __int8 type)
{
    Scr_WatchElement_s *ElementRoot; // eax
    Scr_WatchElement_s *elementa; // [esp+14h] [ebp+Ch]

    if ( element->breakpointType == type )
    {
        elementa = Scr_ScriptWatch::RemoveBreakpoint(inst, element);
        Scr_FreeDebugExpr(inst, &elementa->expr);
        Scr_CompileText(inst, elementa->refText, &elementa->expr);
    }
    else
    {
        elementa = Scr_ScriptWatch::AddBreakpoint(inst, element, type);
    }
    ElementRoot = Scr_GetElementRoot(elementa);
    Scr_ScriptWatch::EvaluateWatchElement(inst, ElementRoot);
}

void __thiscall Scr_ScriptWatch::ToggleBreakpointInternal(
                scriptInstance_t inst,
                Scr_WatchElement_s *element,
                unsigned __int8 type)
{
    unsigned __int8 breakpointType; // [esp+0h] [ebp-Ch]
    Scr_Breakpoint *breakpoint; // [esp+8h] [ebp-4h]

    if ( !element->threadList && !element->endonList )
    {
        if ( Sys_IsRemoteDebugServer() )
        {
            Sys_WriteDebugSocketMessageType(0x1Bu);
            Scr_WriteElement(element);
            Sys_WriteDebugSocketInt(type);
            Sys_EndWriteDebugSocket();
        }
        breakpoint = element->breakpoint;
        if ( breakpoint )
        {
            breakpointType = element->breakpointType;
            switch ( breakpointType )
            {
                case 4u:
                    if ( type == 1 )
                    {
                        element->breakpointType = 5;
                    }
                    else if ( type == 3 )
                    {
                        Scr_ScriptWatch::DeleteElementInternal(inst, element);
                    }
                    break;
                case 6u:
                    if ( type == 7 )
                    {
                        element->breakpointType = 7;
                        if ( (breakpoint->builtinIndex < 0 || breakpoint->builtinIndex >= gScrCompilePub[inst].func_table_size)
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                                        7249,
                                        0,
                                        "%s\n\t(breakpoint->builtinIndex) = %i",
                                        "(breakpoint->builtinIndex >= 0 && breakpoint->builtinIndex < gScrCompilePub[inst].func_table_size)",
                                        breakpoint->builtinIndex) )
                        {
                            __debugbreak();
                        }
                        --gScrVmDebugPub[inst].func_table[breakpoint->builtinIndex].breakpointCount;
                    }
                    break;
                case 7u:
                    if ( type == 6 )
                    {
                        element->breakpointType = 6;
                        if ( (breakpoint->builtinIndex < 0 || breakpoint->builtinIndex >= gScrCompilePub[inst].func_table_size)
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                                        7232,
                                        0,
                                        "%s\n\t(breakpoint->builtinIndex) = %i",
                                        "(breakpoint->builtinIndex >= 0 && breakpoint->builtinIndex < gScrCompilePub[inst].func_table_size)",
                                        breakpoint->builtinIndex) )
                        {
                            __debugbreak();
                        }
                        ++gScrVmDebugPub[inst].func_table[breakpoint->builtinIndex].breakpointCount;
                    }
                    break;
                default:
                    if ( element->breakpointType != 5
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                                    7265,
                                    0,
                                    "%s\n\t(element->breakpointType) = %i",
                                    "(element->breakpointType == SCR_BREAKPOINT_LINE_NORMAL)",
                                    element->breakpointType) )
                    {
                        __debugbreak();
                    }
                    if ( type == 3 )
                    {
                        element->breakpointType = 4;
                    }
                    else if ( type == 1 )
                    {
                        Scr_ScriptWatch::DeleteElementInternal(inst, element);
                    }
                    break;
            }
        }
        else if ( type != 6 && type != 7 )
        {
            Scr_ScriptWatch::ToggleWatchElementBreakpoint(inst, element, type);
        }
    }
}

void __thiscall Scr_ScriptWatch::SortHitBreakpointsTop(scriptInstance_t inst)
{
    Scr_WatchElement_s **pElement; // [esp+4h] [ebp-14h]
    Scr_WatchElement_s **pInsertPoint; // [esp+8h] [ebp-10h]
    int hitBreakpoint; // [esp+Ch] [ebp-Ch]
    Scr_WatchElement_s *element; // [esp+14h] [ebp-4h]

    gScrDebuggerGlob[inst].atBreakpoint = 1;
    hitBreakpoint = 0;
    for ( pInsertPoint = &this->elementHead;
                *pInsertPoint && (*pInsertPoint)->hitBreakpoint;
                pInsertPoint = &(*pInsertPoint)->next )
    {
        hitBreakpoint = 1;
    }
    pElement = pInsertPoint;
    while ( *pElement )
    {
        element = *pElement;
        if ( (*pElement)->hitBreakpoint )
        {
            hitBreakpoint = 1;
            *pElement = (*pElement)->next;
            element->next = *pInsertPoint;
            *pInsertPoint = element;
            pInsertPoint = &element->next;
        }
        else
        {
            pElement = &(*pElement)->next;
        }
    }
    if ( hitBreakpoint && Sys_IsRemoteDebugServer() )
    {
        Sys_WriteDebugSocketMessageType(0x17u);
        Sys_EndWriteDebugSocket();
    }
}

void __cdecl Scr_InitDebuggerMain(scriptInstance_t inst)
{
    if (gScrVarPub[inst].developer)
    {
        if ( gScrDebuggerGlob[inst].debugger_inited_main
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                        7679,
                        0,
                        "%s",
                        "!gScrDebuggerGlob[inst].debugger_inited_main") )
        {
            __debugbreak();
        }
        gScrDebuggerGlob[inst].variableBreakpoints = (Scr_WatchElementDoubleNode_t **)Hunk_UserAlloc(
                                                                                                                                                                        g_DebugHunkUser,
                                                                                                                                                                        1179640,
                                                                                                                                                                        4,
                                                                                                                                                                        "gScrDebuggerGlob[inst].variableBreakpoints");
        memset(gScrDebuggerGlob[inst].variableBreakpoints, 0, 0x11FFF8u);
        gScrDebuggerGlob[inst].assignHead = 0;
        gScrDebuggerGlob[inst].assignHeadCodePos = 0;
        if ( inst )
            gScrDebuggerGlob[inst].disableBreakpoints = 1;
        else
            gScrDebuggerGlob[0].disableBreakpoints = 0;
        gScrDebuggerGlob[inst].debugger_inited_main = 1;
    }
}

void __cdecl Scr_ShutdownDebuggerMain(scriptInstance_t inst)
{
    unsigned int j; // [esp+0h] [ebp-8h]
    Scr_OpcodeList_s *opcodeElement; // [esp+4h] [ebp-4h]

    if (gScrVarPub[inst].developer && gScrDebuggerGlob[inst].debugger_inited_main)
    {
        gScrDebuggerGlob[inst].debugger_inited_main = 0;
        if ( gScrDebuggerGlob[inst].variableBreakpoints )
        {
            for ( j = 0; j < 0x47FFE; ++j )
            {
                if ( gScrDebuggerGlob[inst].variableBreakpoints[j] )
                {
                    if ( !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                                    7744,
                                    0,
                                    "%s",
                                    "!gScrDebuggerGlob[inst].variableBreakpoints[j]") )
                        __debugbreak();
                }
            }
            Hunk_UserFree(g_DebugHunkUser, gScrDebuggerGlob[inst].variableBreakpoints);
            gScrDebuggerGlob[inst].variableBreakpoints = 0;
        }
        while ( gScrDebuggerGlob[inst].assignHead )
        {
            opcodeElement = gScrDebuggerGlob[inst].assignHead->next;
            Hunk_UserFree(g_DebugHunkUser, gScrDebuggerGlob[inst].assignHead);
            gScrDebuggerGlob[inst].assignHead = opcodeElement;
        }
    }
}

void __cdecl Scr_InitDebugger(scriptInstance_t inst)
{
    if (gScrVarPub[inst].developer && gScrCompilePub[inst].script_loading)
    {
        if ( gScrDebuggerGlob[inst].debugger_inited
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                        7769,
                        0,
                        "%s",
                        "!gScrDebuggerGlob[inst].debugger_inited") )
        {
            __debugbreak();
        }
        gScrDebuggerGlob[inst].breakpoints = (char *)Hunk_UserAlloc(
                                                                                                     g_DebugHunkUser,
                                                                                                     gScrCompilePub[inst].programLen,
                                                                                                     4,
                                                                                                     "gScrDebuggerGlob[inst].breakpoints");
        memset(gScrDebuggerGlob[inst].breakpoints, 0x7Fu, gScrCompilePub[inst].programLen);
        gScrDebuggerGlob[inst].scriptList.Init(inst);
        gScrDebuggerGlob[inst].openScriptList.Init(inst);
        gScrDebuggerGlob[inst].debugger_inited = 1;
    }
}

void __cdecl Scr_ShutdownDebugger(scriptInstance_t inst)
{
    if (gScrVarPub[inst].developer && gScrDebuggerGlob[inst].debugger_inited)
    {
        gScrDebuggerGlob[inst].debugger_inited = 0;
        gScrDebuggerGlob[inst].openScriptList.Shutdown(inst);
        gScrDebuggerGlob[inst].scriptList.Shutdown(inst);
        if ( gScrDebuggerGlob[inst].breakpoints )
        {
            Hunk_UserFree(g_DebugHunkUser, gScrDebuggerGlob[inst].breakpoints);
            gScrDebuggerGlob[inst].breakpoints = 0;
        }
        gScrDebuggerGlob[inst].debugger_inited = 0;
    }
}

void __cdecl Scr_InitDebuggerSystem(scriptInstance_t inst)
{
    if (gScrVarPub[inst].developer)
    {
        if (gScrDebuggerGlob[inst].debugger_inited_system
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                7828,
                0,
                "%s",
                "!gScrDebuggerGlob[inst].debugger_inited_system"))
        {
            __debugbreak();
        }
        Scr_InitBreakpoints();
        gScrDebuggerGlob[inst].nextBreakpointCodePos = 0;
        gScrDebuggerGlob[inst].killThreadCodePos = 0;
        gScrDebuggerGlob[inst].breakpointCount = 0;
        if (gScrVarPub[inst].evaluate
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                7843,
                0,
                "%s",
                "!gScrVarPub[inst].evaluate"))
        {
            __debugbreak();
        }
        gScrVarPub[inst].evaluate = 1;
        gScrDebuggerGlob[inst].assignBreakpointSet = 0;
        gScrDebuggerGlob[inst].breakpointPos.bufferIndex = -1;
        gScrDebuggerGlob[inst].atBreakpoint = 0;
        gScrDebuggerGlob[inst].run_debugger = 0;
        //Scr_ScriptWatch::Init(&gScrDebuggerGlob[inst].scriptWatch, inst);
        gScrDebuggerGlob[inst].scriptWatch.Init(inst);
        if (!gScrVarPub[inst].evaluate
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                7868,
                0,
                "%s",
                "gScrVarPub[inst].evaluate"))
        {
            __debugbreak();
        }
        gScrVarPub[inst].evaluate = 0;
        gScrDebuggerGlob[inst].debugger_inited_system = 1;
        if (Sys_IsRemoteDebugServer() && (Scr_SendSource(inst), Sys_IsRemoteDebugServer()))
        {
            Scr_RunDebugger(inst);
        }
        else
        {
            //Scr_ScriptWatch::UpdateBreakpoints(&gScrDebuggerGlob[inst].scriptWatch, inst, 1);
            gScrDebuggerGlob[inst].scriptWatch.UpdateBreakpoints(inst, 1);
        }
    }
}

void Scr_InitBreakpoints()
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 0x7F; ++i )
        g_breakpoints[i].next = &g_breakpoints[i + 1];
    if ( g_breakpoints[127].next
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    322,
                    0,
                    "%s",
                    "!g_breakpoints[ARRAY_COUNT( g_breakpoints ) - 1].next") )
    {
        __debugbreak();
    }
    g_breakpointsHead = g_breakpoints;
}

void __cdecl Scr_ShutdownDebuggerSystem(scriptInstance_t inst)
{
    if (gScrVarPub[inst].developer && gScrDebuggerGlob[inst].debugger_inited_system)
    {
        gScrDebuggerGlob[inst].debugger_inited_system = 0;
        gScrVarPub[inst].evaluate = 0;
        //Scr_ScriptWatch::Shutdown(&gScrDebuggerGlob[inst].scriptWatch, inst);
        gScrDebuggerGlob[inst].scriptWatch.Shutdown(inst);
        if (gScrDebuggerGlob[inst].nextBreakpointCodePos)
        {
            Scr_RemoveManualBreakpoint(inst, (unsigned __int8 *)gScrDebuggerGlob[inst].nextBreakpointCodePos);
            gScrDebuggerGlob[inst].nextBreakpointCodePos = 0;
        }
        if (gScrDebuggerGlob[inst].killThreadCodePos)
        {
            Scr_RemoveManualBreakpoint(inst, (unsigned __int8 *)gScrDebuggerGlob[inst].killThreadCodePos);
            gScrDebuggerGlob[inst].killThreadCodePos = 0;
        }
        if (gScrDebuggerGlob[inst].breakpointCount)
        {
            if (!Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                7959,
                0,
                "%s",
                "!gScrDebuggerGlob[inst].breakpointCount"))
                __debugbreak();
        }
    }
}

void __cdecl Scr_ConnectRemote(scriptInstance_t inst)
{
    if (Sys_StartRemoteDebugServer() && gScrDebuggerGlob[inst].debugger_inited_system)
    {
        if (gScrVarPub[inst].evaluate
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                7973,
                0,
                "%s",
                "!gScrVarPub[inst].evaluate"))
        {
            __debugbreak();
        }
        Scr_ShutdownDebuggerSystem(inst);
        Scr_InitDebuggerSystem(inst);
    }
}

void __cdecl Scr_AddAssignmentPos(scriptInstance_t inst, char *codePos)
{
    Scr_OpcodeList_s *opcodeElement; // [esp+0h] [ebp-4h]

    if (!gScrVarPub[inst].developer
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            7998,
            0,
            "%s",
            "gScrVarPub[inst].developer"))
    {
        __debugbreak();
    }
    if (gScrCompilePub[inst].developer_statement != 2 && gScrDebuggerGlob[inst].assignHeadCodePos != codePos)
    {
        gScrDebuggerGlob[inst].assignHeadCodePos = codePos;
        opcodeElement = (Scr_OpcodeList_s *)Hunk_UserAlloc(g_DebugHunkUser, 8, 4, "Scr_AddAssignmentPos");
        opcodeElement->codePos = codePos;
        opcodeElement->next = gScrDebuggerGlob[inst].assignHead;
        gScrDebuggerGlob[inst].assignHead = opcodeElement;
    }
}

void __cdecl Scr_ResetAbortDebugger(scriptInstance_t inst)
{
    gScrDebuggerGlob[inst].abort = 0;
}

void __cdecl Scr_RunDebugger(scriptInstance_t inst)
{
    if (gScrVarPub[inst].developer)
    {
        Scr_ResetAbortDebugger(inst);
        if (!Scr_IsStackClear(inst)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                8377,
                0,
                "%s",
                "Scr_IsStackClear(inst)"))
        {
            __debugbreak();
        }
        if (gScrVmPub[inst].function_count
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                8379,
                0,
                "%s",
                "!gScrVmPub[inst].function_count"))
        {
            __debugbreak();
        }
        gScrDebuggerGlob[inst].scriptWatch.localId = 0;
        Scr_DisplayDebugger(inst);
        if (!Scr_IsStackClear(inst)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                8383,
                0,
                "%s",
                "Scr_IsStackClear(inst)"))
        {
            __debugbreak();
        }
    }
}

void __cdecl Scr_DisplayDebugger(scriptInstance_t inst)
{
    if (gScrDebuggerGlob[inst].atBreakpoint)
    {
        //Scr_ScriptCallStack::UpdateStack(&gScrDebuggerGlob[inst].scriptCallStack, inst);
        gScrDebuggerGlob[inst].scriptCallStack.UpdateStack(inst);
    }
    if (!Sys_IsRemoteDebugServer() || (Scr_DisplayDebuggerRemote(inst), !Sys_IsRemoteDebugServer()))
    {
        Scr_ResumeBreakpoints(inst);
    }
}

void __cdecl Scr_DisplayDebuggerRemote(scriptInstance_t inst)
{
    const char *varUsagePos; // [esp+0h] [ebp-8h]
    int line; // [esp+4h] [ebp-4h]

    if (!Sys_IsRemoteDebugServer()
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            8075,
            0,
            "%s",
            "Sys_IsRemoteDebugServer()"))
    {
        __debugbreak();
    }
    varUsagePos = gScrVarPub[inst].varUsagePos;
    gScrVarPub[inst].varUsagePos = "<script debugger variable>";
    if (gScrVarPub[inst].evaluate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            8090,
            0,
            "%s",
            "!gScrVarPub[inst].evaluate"))
    {
        __debugbreak();
    }
    gScrVarPub[inst].evaluate = 1;
    //Scr_ScriptWatch::UpdateBreakpoints(&gScrDebuggerGlob[inst].scriptWatch, inst, 0);
    gScrDebuggerGlob[inst].scriptWatch.UpdateBreakpoints(inst, 0);
    Scr_Evaluate(inst);
    if (gScrDebuggerGlob[inst].atBreakpoint)
    {
        Sys_WriteDebugSocketMessageType(0x24u);
        Sys_WriteDebugSocketInt(*(&gScrDebuggerGlob[0].scriptCallStack.numLines + 113 * inst));
        for (line = 0; line < *(&gScrDebuggerGlob[0].scriptCallStack.numLines + 113 * inst); ++line)
        {
            Sys_WriteDebugSocketInt(gScrDebuggerGlob[inst].scriptCallStack.stack[line].bufferIndex);
            Sys_WriteDebugSocketInt(gScrDebuggerGlob[inst].scriptCallStack.stack[line].sourcePos);
        }
        Sys_EndWriteDebugSocket();
    }
    else
    {
        gScrDebuggerGlob[inst].atBreakpoint = 1;
    }
    if (!gScrVarPub[inst].evaluate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            8113,
            0,
            "%s",
            "gScrVarPub[inst].evaluate"))
    {
        __debugbreak();
    }
    gScrVarPub[inst].evaluate = 0;
    //Scr_ScriptWatch::UpdateBreakpoints(&gScrDebuggerGlob[inst].scriptWatch, inst, 1);
    gScrDebuggerGlob[inst].scriptWatch.UpdateBreakpoints(inst, 1);
    gScrVarPub[inst].varUsagePos = varUsagePos;
    if (Sys_IsMainThread())
    {
        Scr_DisplayDebuggerRemoteInternal(inst);
    }
    else
    {
        gScrDebuggerGlob[inst].disableDebuggerRemote = 1;
        while (gScrDebuggerGlob[inst].disableDebuggerRemote)
            ;
    }
}

void __cdecl Scr_DisplayDebuggerRemoteInternal(scriptInstance_t inst)
{
    const char *varUsagePos; // [esp+0h] [ebp-4h]

    if (!Sys_IsMainThread()
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            8027,
            0,
            "%s",
            "Sys_IsMainThread()"))
    {
        __debugbreak();
    }
    varUsagePos = gScrVarPub[inst].varUsagePos;
    gScrVarPub[inst].varUsagePos = "<script debugger variable>";
    if (gScrVarPub[inst].evaluate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            8034,
            0,
            "%s",
            "!gScrVarPub[inst].evaluate"))
    {
        __debugbreak();
    }
    gScrVarPub[inst].evaluate = 1;
    //Scr_ScriptWatch::UpdateBreakpoints(&gScrDebuggerGlob[inst].scriptWatch, inst, 0);
    gScrDebuggerGlob[inst].scriptWatch.UpdateBreakpoints(inst, 0);
    while (gScrDebuggerGlob[inst].atBreakpoint)
    {
        if (!Sys_IsRemoteDebugServer())
        {
            Scr_ResumeBreakpoints(inst);
            break;
        }
        if (gScrDebuggerGlob[inst].abort)
        {
            gScrDebuggerGlob[inst].step_mode = 0;
            Scr_Step(inst);
        }
        Scr_ProcessDebugMessages(inst);
        Sys_FlushDebugSocketData();
    }
    //Scr_ScriptWatch::UpdateBreakpoints(&gScrDebuggerGlob[inst].scriptWatch, inst, 1);
    gScrDebuggerGlob[inst].scriptWatch.UpdateBreakpoints(inst, 1);
    if (!gScrVarPub[inst].evaluate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            8059,
            0,
            "%s",
            "gScrVarPub[inst].evaluate"))
    {
        __debugbreak();
    }
    gScrVarPub[inst].evaluate = 0;
    gScrVarPub[inst].varUsagePos = varUsagePos;
}

void __cdecl Scr_Step(scriptInstance_t inst)
{
    bool evaluate; // [esp+3h] [ebp-9h]
    unsigned int localId; // [esp+4h] [ebp-8h] BYREF
    const char *codePos; // [esp+8h] [ebp-4h]

    if (Sys_IsRemoteDebugServer() && gScrDebuggerGlob[inst].atBreakpoint)
        Scr_ResumeBreakpoints(inst);
    if (gScrDebuggerGlob[inst].step_mode && gScrVmPub[inst].function_count)
    {
        if (gScrDebuggerGlob[inst].step_mode == 3)
        {
            gScrDebuggerGlob[inst].step_mode = 1;
            codePos = Scr_GetReturnPos(inst, &localId);
        }
        else
        {
            evaluate = gScrVarPub[inst].evaluate;
            gScrVarPub[inst].evaluate = 1;
            if (gScrDebuggerGlob[inst].breakpointOpcode < 0
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    1333,
                    0,
                    "%s\n\t(gScrDebuggerGlob[inst].breakpointOpcode) = %i",
                    "(gScrDebuggerGlob[inst].breakpointOpcode >= 0)",
                    gScrDebuggerGlob[inst].breakpointOpcode))
            {
                __debugbreak();
            }
            codePos = Scr_GetNextCodepos(
                inst,
                gScrDebuggerGlob[inst].breakpointTop,
                gScrDebuggerGlob[inst].breakpointCodePos,
                gScrDebuggerGlob[inst].breakpointOpcode,
                gScrDebuggerGlob[inst].step_mode,
                &localId);
            gScrVarPub[inst].evaluate = evaluate;
        }
        Scr_SetTempBreakpoint(inst, (char *)codePos, localId);
    }
}

void __cdecl Scr_WatchElementHitBreakpoint(Scr_WatchElement_s *element, bool enabled)
{
    if ( Sys_IsRemoteDebugServer() )
    {
        Sys_WriteDebugSocketMessageType(0x1Cu);
        Scr_WriteElement(element);
        Sys_WriteDebugSocketInt(enabled);
        Sys_EndWriteDebugSocket();
    }
    if ( enabled )
    {
        element->hitBreakpoint = 1;
    }
    else
    {
        element->changed = 1;
        element->changedTime = 0;
    }
}

void __cdecl Scr_HitBreakpointInternal(scriptInstance_t inst)
{
    //Scr_ScriptWatch::SortHitBreakpointsTop(&gScrDebuggerGlob[inst].scriptWatch, inst);
    gScrDebuggerGlob[inst].scriptWatch.SortHitBreakpointsTop(inst);
    if ( Sys_IsRemoteDebugServer() )
        Scr_DisplayHitBreakpointRemote(inst);
    Scr_DisplayDebugger(inst);
    Scr_ResetTimeout(inst);
}

void __cdecl Scr_DisplayHitBreakpointRemote(scriptInstance_t inst)
{
    if ( !Sys_IsRemoteDebugServer()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    8406,
                    0,
                    "%s",
                    "Sys_IsRemoteDebugServer()") )
    {
        __debugbreak();
    }
    if ( gScrDebuggerGlob[inst].breakpointPos.bufferIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    8407,
                    0,
                    "%s",
                    "gScrDebuggerGlob[inst].breakpointPos.bufferIndex != NO_BUFFER_INDEX") )
    {
        __debugbreak();
    }
    if ( !gScrDebuggerGlob[inst].atBreakpoint
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    8408,
                    0,
                    "%s",
                    "gScrDebuggerGlob[inst].atBreakpoint") )
    {
        __debugbreak();
    }
    Sys_WriteDebugSocketMessageType(0x16u);
    Sys_WriteDebugSocketInt(gScrDebuggerGlob[inst].breakpointPos.bufferIndex);
    Sys_WriteDebugSocketInt(gScrDebuggerGlob[inst].breakpointPos.lineNum);
    Sys_EndWriteDebugSocket();
}

int __cdecl Scr_HitBreakpoint(
    scriptInstance_t inst,
    VariableValue *top,
    const char *pos,
    unsigned int localId,
    int hitBreakpoint)
{
    Scr_Breakpoint *breakpoint; // [esp+0h] [ebp-24h]
    bool hitStepBreakpoint; // [esp+Bh] [ebp-19h]
    const char *codePos; // [esp+10h] [ebp-14h]
    bool kill_thread; // [esp+17h] [ebp-Dh]
    int opcode; // [esp+18h] [ebp-Ch]
    Scr_WatchElement_s *element; // [esp+1Ch] [ebp-8h]
    bool existsBreakpoint; // [esp+23h] [ebp-1h]

    if (!gScrVarPub[inst].developer
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            8550,
            0,
            "%s",
            "gScrVarPub[inst].developer"))
    {
        __debugbreak();
    }
    if (!gScrVmPub[inst].function_count
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            8554,
            0,
            "%s",
            "gScrVmPub[inst].function_count"))
    {
        __debugbreak();
    }
    if (gScrDebuggerGlob[inst].breakpointPos.bufferIndex != -1
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            8555,
            0,
            "%s",
            "gScrDebuggerGlob[inst].breakpointPos.bufferIndex == NO_BUFFER_INDEX"))
    {
        __debugbreak();
    }
    if (gScrDebuggerGlob[inst].atBreakpoint
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            8556,
            0,
            "%s",
            "!gScrDebuggerGlob[inst].atBreakpoint"))
    {
        __debugbreak();
    }
    codePos = pos - 1;
    opcode = (unsigned __int8)*Scr_FindBreakpointInfo(inst, pos - 1);
    hitStepBreakpoint = 0;
    existsBreakpoint = 0;
    if (gScrDebuggerGlob[inst].nextBreakpointCodePos == pos - 1)
    {
        if (gScrDebuggerGlob[inst].nextBreakpointThreadId && localId != gScrDebuggerGlob[inst].nextBreakpointThreadId)
            existsBreakpoint = 1;
        else
            hitStepBreakpoint = 1;
    }
    kill_thread = gScrDebuggerGlob[inst].kill_thread;
    if (kill_thread)
    {
        gScrDebuggerGlob[inst].kill_thread = 0;
        opcode = 0;
        if (gScrDebuggerGlob[inst].killThreadCodePos)
        {
            if (gScrDebuggerGlob[inst].killThreadCodePos != codePos
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    8581,
                    0,
                    "%s",
                    "gScrDebuggerGlob[inst].killThreadCodePos == codePos"))
            {
                __debugbreak();
            }
            Scr_RemoveManualBreakpoint(inst, (unsigned __int8 *)gScrDebuggerGlob[inst].killThreadCodePos);
            gScrDebuggerGlob[inst].killThreadCodePos = 0;
            existsBreakpoint = 1;
        }
    }
    else if (gScrDebuggerGlob[inst].killThreadCodePos
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            8590,
            0,
            "%s",
            "!gScrDebuggerGlob[inst].killThreadCodePos"))
    {
        __debugbreak();
    }
    gScrVmPub[inst].top = top;
    if (gScrVarPub[inst].evaluate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            8595,
            0,
            "%s",
            "!gScrVarPub[inst].evaluate"))
    {
        __debugbreak();
    }
    gScrVarPub[inst].evaluate = 1;
    for (element = gScrDebuggerGlob[inst].scriptWatch.elementHead; element; element = element->next)
    {
        breakpoint = element->breakpoint;
        if (breakpoint && breakpoint->codePos == codePos)
        {
            existsBreakpoint = 1;
            if (Scr_ConditionalExpression(inst, element, localId))
                hitBreakpoint = 1;
        }
    }
    if (!gScrVarPub[inst].evaluate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            8612,
            0,
            "%s",
            "gScrVarPub[inst].evaluate"))
    {
        __debugbreak();
    }
    gScrVarPub[inst].evaluate = 0;
    if (!hitBreakpoint && !hitStepBreakpoint && existsBreakpoint)
        return opcode;
    if (gScrDebuggerGlob[inst].nextBreakpointCodePos)
    {
        Scr_RemoveManualBreakpoint(inst, (unsigned __int8 *)gScrDebuggerGlob[inst].nextBreakpointCodePos);
        gScrDebuggerGlob[inst].nextBreakpointCodePos = 0;
    }
    if (!Scr_AllowBreakpoint(inst))
        return opcode;
    gScrDebuggerGlob[inst].breakpointTop = top;
    gScrDebuggerGlob[inst].breakpointCodePos = codePos;
    gScrDebuggerGlob[inst].breakpointOpcode = opcode;
    if (Scr_GetSourcePosOfType(inst, codePos, !kill_thread, &gScrDebuggerGlob[inst].breakpointPos)
        && (!hitStepBreakpoint
            || gScrDebuggerGlob[inst].step_mode == 4
            || gScrDebuggerGlob[inst].prevBreakpointLineNum != gScrDebuggerGlob[inst].breakpointPos.lineNum))
    {
        gScrVmPub[inst].function_frame->fs.pos = pos;
        gScrDebuggerGlob[inst].scriptWatch.localId = localId;
        Scr_HitBreakpointInternal(inst);
        return opcode;
    }
    else
    {
        if (!hitStepBreakpoint)
            gScrDebuggerGlob[inst].step_mode = 4;
        Scr_Step(inst);
        gScrDebuggerGlob[inst].breakpointPos.bufferIndex = -1;
        return opcode;
    }
}

bool __cdecl Scr_ConditionalExpression(scriptInstance_t inst, Scr_WatchElement_s *element, unsigned int localId)
{
    unsigned int Self; // eax
    bool v5; // [esp+0h] [ebp-20h]
    Scr_WatchElement_s *conditionalElement; // [esp+8h] [ebp-18h]
    Scr_WatchElement_s *conditionalElementa; // [esp+8h] [ebp-18h]
    VariableValue newValue; // [esp+18h] [ebp-8h] BYREF

    for (conditionalElement = element->next; ; conditionalElement = conditionalElement->next)
    {
        if (!conditionalElement || conditionalElement->breakpointType != 2)
        {
            v5 = element->breakpointType == 1 || element->breakpointType == 5 || element->breakpointType == 6;
            Scr_WatchElementHitBreakpoint(element, v5);
            for (conditionalElementa = element->next;
                conditionalElementa && conditionalElementa->breakpointType == 2;
                conditionalElementa = conditionalElementa->next)
            {
                Scr_WatchElementHitBreakpoint(conditionalElementa, v5);
            }
            return v5;
        }
        if (conditionalElement->breakpoint
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                8452,
                0,
                "%s",
                "!conditionalElement->breakpoint"))
        {
            __debugbreak();
        }
        if (!conditionalElement->expr.exprHead
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                8454,
                0,
                "%s",
                "expr->exprHead"))
        {
            __debugbreak();
        }
        Scr_EvalScriptExpression(inst, &conditionalElement->expr, localId, &newValue, 0, 1);
        if (newValue.type == 1)
            break;
        Scr_CastBool(inst, &newValue);
        if (gScrVarPub[inst].error_message)
        {
            Scr_ClearErrorMessage(inst);
            return 0;
        }
        if (newValue.type != 6
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                8490,
                0,
                "%s",
                "newValue.type == VAR_INTEGER"))
        {
            __debugbreak();
        }
        if (!newValue.u.intValue)
            return 0;
    LABEL_2:
        ;
    }
    RemoveRefToObject(inst, newValue.u.stringValue);
    switch (GetObjectType(inst, newValue.u.stringValue))
    {
    case 0xDu:
    case 0xEu:
    case 0xFu:
    case 0x10u:
        if (newValue.u.intValue == localId)
            goto LABEL_2;
        break;
    case 0x11u:
    case 0x12u:
    case 0x13u:
        if (localId)
        {
            Self = Scr_GetSelf(inst, localId);
            if (newValue.u.intValue == Self)
                goto LABEL_2;
        }
        break;
    default:
        return 0;
    }
    return 0;
}

void __cdecl Scr_HitBuiltinBreakpoint(
    scriptInstance_t inst,
    VariableValue *top,
    const char *pos,
    unsigned int localId,
    int opcode,
    int builtinIndex,
    unsigned int outparamcount)
{
    Scr_Breakpoint *breakpoint; // [esp+0h] [ebp-10h]
    bool hitBreakpoint; // [esp+7h] [ebp-9h]
    Scr_WatchElement_s *element; // [esp+8h] [ebp-8h]
    bool existsBreakpoint; // [esp+Fh] [ebp-1h]

    if (!gScrVarPub[inst].developer
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            8657,
            0,
            "%s",
            "gScrVarPub[inst].developer"))
    {
        __debugbreak();
    }
    if (!gScrVmPub[inst].function_count
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            8661,
            0,
            "%s",
            "gScrVmPub[inst].function_count"))
    {
        __debugbreak();
    }
    if (gScrDebuggerGlob[inst].breakpointPos.bufferIndex != -1
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            8662,
            0,
            "%s",
            "gScrDebuggerGlob[inst].breakpointPos.bufferIndex == NO_BUFFER_INDEX"))
    {
        __debugbreak();
    }
    if (gScrDebuggerGlob[inst].atBreakpoint
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            8663,
            0,
            "%s",
            "!gScrDebuggerGlob[inst].atBreakpoint"))
    {
        __debugbreak();
    }
    hitBreakpoint = 0;
    existsBreakpoint = 0;
    gScrVmPub[inst].top = top;
    gScrVmPub[inst].outparamcount = 0;
    gScrVmPub[inst].breakpointOutparamcount = outparamcount;
    if (gScrVarPub[inst].evaluate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            8672,
            0,
            "%s",
            "!gScrVarPub[inst].evaluate"))
    {
        __debugbreak();
    }
    gScrVarPub[inst].evaluate = 1;
    for (element = gScrDebuggerGlob[inst].scriptWatch.elementHead; element; element = element->next)
    {
        breakpoint = element->breakpoint;
        if (breakpoint && !breakpoint->codePos && breakpoint->builtinIndex == builtinIndex)
        {
            existsBreakpoint = 1;
            if (Scr_ConditionalExpression(inst, element, localId))
                hitBreakpoint = 1;
        }
    }
    if (!gScrVarPub[inst].evaluate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            8691,
            0,
            "%s",
            "gScrVarPub[inst].evaluate"))
    {
        __debugbreak();
    }
    gScrVarPub[inst].evaluate = 0;
    if (!existsBreakpoint
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            8694,
            0,
            "%s",
            "existsBreakpoint"))
    {
        __debugbreak();
    }
    if (hitBreakpoint)
        Scr_SpecialBreakpoint(inst, top, pos, localId, opcode, 8);
}

void __cdecl Scr_DebugKillThread(scriptInstance_t inst, unsigned int threadId, const char *codePos)
{
    bool enabled; // [esp+3h] [ebp-11h]
    int hitBreakpoint; // [esp+4h] [ebp-10h]
    Scr_WatchElementDoubleNode_t *breakpoints; // [esp+8h] [ebp-Ch]
    Scr_WatchElementNode_s *elementNode; // [esp+Ch] [ebp-8h]
    Scr_WatchElement_s *element; // [esp+10h] [ebp-4h]

    if (!gScrDebuggerGlob[inst].variableBreakpoints
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            8710,
            0,
            "%s",
            "gScrDebuggerGlob[inst].variableBreakpoints"))
    {
        __debugbreak();
    }
    breakpoints = gScrDebuggerGlob[inst].variableBreakpoints[threadId + 1];
    if (breakpoints && gScrVarPub[inst].evaluate)
    {
        hitBreakpoint = 0;
        for (elementNode = breakpoints->list; elementNode; elementNode = elementNode->next)
        {
            element = elementNode->element;
            if (!elementNode->element->breakpointType
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    8723,
                    0,
                    "%s",
                    "element->breakpointType != SCR_BREAKPOINT_NONE"))
            {
                __debugbreak();
            }
            if ((element->breakpointType == 1 || element->breakpointType == 3) && element->objectId == threadId)
            {
                enabled = element->breakpointType == 1;
                Scr_WatchElementHitBreakpoint(element, enabled);
                element->deadCodePos = codePos;
                if (enabled)
                    hitBreakpoint = 1;
            }
        }
        if (hitBreakpoint)
        {
            if (gScrVmPub[inst].function_count)
            {
                if (gScrDebuggerGlob[inst].killThreadCodePos)
                {
                    if (gScrDebuggerGlob[inst].killThreadCodePos != gScrVmPub[inst].function_frame->fs.pos
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                            8749,
                            0,
                            "%s",
                            "gScrDebuggerGlob[inst].killThreadCodePos == gScrVmPub[inst].function_frame->fs.pos"))
                    {
                        __debugbreak();
                    }
                    Scr_RemoveManualBreakpoint(inst, (unsigned __int8 *)gScrDebuggerGlob[inst].killThreadCodePos);
                    gScrDebuggerGlob[inst].killThreadCodePos = 0;
                }
                if (gScrDebuggerGlob[inst].nextBreakpointCodePos)
                    Scr_RemoveManualBreakpoint(inst, (unsigned __int8 *)gScrDebuggerGlob[inst].nextBreakpointCodePos);
                gScrDebuggerGlob[inst].nextBreakpointCodePos = (char *)gScrVmPub[inst].function_frame->fs.pos;
                Scr_AddManualBreakpoint(inst, (unsigned __int8 *)gScrDebuggerGlob[inst].nextBreakpointCodePos);
                gScrDebuggerGlob[inst].prevBreakpointLineNum = -1;
                gScrDebuggerGlob[inst].nextBreakpointThreadId = gScrVmPub[inst].function_frame->fs.localId;
            }
            else
            {
                gScrDebuggerGlob[inst].run_debugger = 1;
            }
        }
    }
}

void __cdecl Scr_DebugTerminateThread(scriptInstance_t inst, int topThread)
{
    Scr_DebugKillThread(
        inst,
        gScrVmPub[inst].function_frame_start[topThread].fs.localId,
        gScrVmPub[inst].function_frame_start[topThread].fs.pos);
    if (topThread == gScrVmPub[inst].function_count)
    {
        if (!gScrDebuggerGlob[inst].kill_thread)
        {
            gScrDebuggerGlob[inst].kill_thread = 1;
            if (gScrDebuggerGlob[inst].killThreadCodePos
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    8778,
                    0,
                    "%s",
                    "!gScrDebuggerGlob[inst].killThreadCodePos"))
            {
                __debugbreak();
            }
            if (*gScrVmPub[inst].function_frame->fs.pos != 135 && *gScrVmPub[inst].function_frame->fs.pos != 137)
            {
                gScrDebuggerGlob[inst].killThreadCodePos = (char *)gScrVmPub[inst].function_frame->fs.pos;
                Scr_AddManualBreakpoint(inst, (unsigned __int8 *)gScrDebuggerGlob[inst].killThreadCodePos);
            }
        }
    }
    else
    {
        gScrVmPub[inst].function_frame_start[topThread].fs.pos = &g_EndPos;
    }
}

void __cdecl Scr_ProcessDebugMessages(scriptInstance_t inst)
{
    if ( !Sys_IsRemoteDebugServer()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    8877,
                    0,
                    "%s",
                    "Sys_IsRemoteDebugServer()") )
    {
        __debugbreak();
    }
    while ( 1 )
    {
        switch ( Sys_UpdateDebugSocket() )
        {
            case 4:
                Scr_AddTextRemote(inst);
                break;
            case 6:
                Scr_PasteElementRemote(inst);
                break;
            case 8:
                Scr_InsertElementRemote(inst);
                break;
            case 10:
                Scr_DeleteElementRemote(inst);
                break;
            case 12:
                Scr_BackspaceElementRemote(inst);
                break;
            case 14:
                Scr_CloneElementRemote(inst);
                break;
            case 16:
                Scr_ExpandElementRemote(inst);
                break;
            case 20:
                Scr_ToggleBreakpointRemote(inst);
                break;
            case 24:
                Scr_StepRemote(inst);
                break;
            case 25:
                Scr_RunToCursorRemote(inst);
                break;
            case 26:
                Scr_ToggleWatchElementBreakpointRemote(inst);
                break;
            case 38:
                Scr_KeepAliveRemote(inst);
                break;
            case 39:
                Scr_DisplayThreadPosRemote(inst);
                break;
            case 45:
                Scr_EnterCallRemote(inst);
                break;
            default:
                Scr_UpdateDebugSocket(inst);
                NET_Sleep(1u);
                return;
        }
        Sys_AckDebugSocket();
    }
}

void __cdecl Scr_ToggleBreakpointRemote(scriptInstance_t inst)
{
    bool overwrite; // [esp+5h] [ebp-Fh]
    unsigned __int8 breakpointType; // [esp+6h] [ebp-Eh]
    bool force; // [esp+7h] [ebp-Dh]
    Scr_ScriptWindow *scriptWindow; // [esp+8h] [ebp-Ch]
    bool user; // [esp+Fh] [ebp-5h]
    Scr_WatchElement_s *element; // [esp+10h] [ebp-4h]

    scriptWindow = (&gScrDebuggerGlob[0].scriptList.scriptWindows)[113 * inst][Sys_ReadDebugSocketInt()];
    scriptWindow->selectedLine = Sys_ReadDebugSocketInt();
    element = Scr_ReadElement(inst);
    force = Sys_ReadDebugSocketInt() != 0;
    overwrite = Sys_ReadDebugSocketInt() != 0;
    breakpointType = Sys_ReadDebugSocketInt();
    user = Sys_ReadDebugSocketInt() != 0;
    if (!Sys_IsRemoteDebugServer()
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            1168,
            0,
            "%s",
            "Sys_IsRemoteDebugServer()"))
    {
        __debugbreak();
    }
    Sys_WriteDebugSocketMessageType(0x15u);
    //Scr_ScriptWindow::ToggleBreakpointInternal(scriptWindow, inst, element, force, overwrite, breakpointType, user);
    scriptWindow->ToggleBreakpointInternal(inst, element, force, overwrite, breakpointType, user);
    Sys_EndWriteDebugSocket();
}

void __cdecl Scr_StepRemote(scriptInstance_t inst)
{
    gScrDebuggerGlob[inst].step_mode = Sys_ReadDebugSocketInt();
    Scr_Step(inst);
}

void __cdecl Scr_RunToCursorRemote(scriptInstance_t inst)
{
    Scr_ScriptWindow *scriptWindow; // [esp+4h] [ebp-4h]

    scriptWindow = (&gScrDebuggerGlob[0].scriptList.scriptWindows)[113 * inst][Sys_ReadDebugSocketInt()];
    scriptWindow->selectedLine = Sys_ReadDebugSocketInt();
    //Scr_ScriptWindow::RunToCursor(scriptWindow, inst);
    scriptWindow->RunToCursor(inst);
}

void __cdecl Scr_EnterCallRemote(scriptInstance_t inst)
{
    Scr_ScriptWindow *scriptWindow; // [esp+4h] [ebp-4h]

    scriptWindow = (&gScrDebuggerGlob[0].scriptList.scriptWindows)[113 * inst][Sys_ReadDebugSocketInt()];
    scriptWindow->selectedLine = Sys_ReadDebugSocketInt();
    //Scr_ScriptWindow::EnterCallInternal(scriptWindow, inst);
    scriptWindow->EnterCallInternal(inst);
}

void __cdecl Scr_ExpandElementRemote(scriptInstance_t inst)
{
    bool expand; // [esp+3h] [ebp-5h]
    Scr_WatchElement_s *element; // [esp+4h] [ebp-4h]

    element = Scr_ReadElement(inst);
    expand = Sys_ReadDebugSocketInt() != 0;
    if (element)
    {
        //Scr_ScriptWatch::ExpandElement(&gScrDebuggerGlob[inst].scriptWatch, inst, element, expand);
        gScrDebuggerGlob[inst].scriptWatch.ExpandElement(inst, element, expand);
    }
}

void __cdecl Scr_PasteElementRemote(scriptInstance_t inst)
{
    Scr_WatchElement_s *newElement; // [esp+0h] [ebp-10h]
    bool user; // [esp+6h] [ebp-Ah]
    unsigned __int8 type; // [esp+7h] [ebp-9h]
    Scr_WatchElement_s *element; // [esp+8h] [ebp-8h]
    char *text; // [esp+Ch] [ebp-4h]

    element = Scr_ReadElement(inst);
    text = Sys_ReadDebugSocketString();
    user = Sys_ReadDebugSocketInt() != 0;
    type = 0;
    if ( Sys_IsRemoteDebugServer() )
    {
        type = Sys_ReadDebugSocketInt();
        Sys_WriteDebugSocketMessageType(7u);
        Scr_WriteElement(element);
        Sys_WriteDebugSocketString(text);
        Sys_WriteDebugSocketInt(user);
        Sys_EndWriteDebugSocket();
    }
    newElement = gScrDebuggerGlob[inst].scriptWatch.PasteNonBreakpointElement(
                                 inst,
                                 element,
                                 text,
                                 user);
    if ( Sys_IsRemoteDebugServer() && type )
        gScrDebuggerGlob[inst].scriptWatch.ToggleBreakpointInternal(inst, newElement, type);
    FreeString(text, 0, inst);
}

void __cdecl Scr_InsertElementRemote(scriptInstance_t inst)
{
    Scr_WatchElement_s *element; // [esp+0h] [ebp-4h]

    element = Scr_ReadElement(inst);
    if ( Sys_IsRemoteDebugServer() )
    {
        Sys_WriteDebugSocketMessageType(9u);
        Scr_WriteElement(element);
        Sys_EndWriteDebugSocket();
    }
        gScrDebuggerGlob[inst].scriptWatch.PasteNonBreakpointElement(
        inst,
        element,
        (char *)"",
        1);
}

void __cdecl Scr_DeleteElementRemote(scriptInstance_t inst)
{
    Scr_WatchElement_s *element; // [esp+0h] [ebp-4h]

    element = Scr_ReadElement(inst);
    if ( element )
    {
        if ( Sys_IsRemoteDebugServer() )
        {
            Sys_WriteDebugSocketMessageType(0xBu);
            Scr_WriteElement(element);
            Sys_EndWriteDebugSocket();
        }
        //Scr_ScriptWatch::DeleteElementInternal(&gScrDebuggerGlob[inst].scriptWatch, inst, element);
        gScrDebuggerGlob[inst].scriptWatch.DeleteElementInternal(inst, element);
    }
}

void __cdecl Scr_BackspaceElementRemote(scriptInstance_t inst)
{
    Scr_WatchElement_s *element; // [esp+0h] [ebp-4h]

    element = Scr_ReadElement(inst);
    if ( element )
    {
        if ( Sys_IsRemoteDebugServer() )
        {
            Sys_WriteDebugSocketMessageType(0xDu);
            Scr_WriteElement(element);
            Sys_EndWriteDebugSocket();
        }
        //Scr_ScriptWatch::BackspaceElementInternal(&gScrDebuggerGlob[inst].scriptWatch, inst, element);
        gScrDebuggerGlob[inst].scriptWatch.BackspaceElementInternal(inst, element);
    }
}

void __cdecl Scr_AddTextRemote(scriptInstance_t inst)
{
    Scr_WatchElement_s *element; // [esp+0h] [ebp-8h]
    char *text; // [esp+4h] [ebp-4h]

    element = Scr_ReadElement(inst);
    if ( element )
    {
        text = Sys_ReadDebugSocketString();
        //Scr_ScriptWatch::AddElement(&gScrDebuggerGlob[inst].scriptWatch, inst, element, text);
        gScrDebuggerGlob[inst].scriptWatch.AddElement(inst, element, text);
        FreeString(text, 0, inst);
    }
}

void __cdecl Scr_CloneElementRemote(scriptInstance_t inst)
{
    Scr_WatchElement_s *element; // [esp+0h] [ebp-4h]
    Scr_WatchElement_s *elementa; // [esp+0h] [ebp-4h]

    element = Scr_ReadElement(inst);
    if ( element )
    {
        if ( Sys_IsRemoteDebugServer() )
        {
            Sys_WriteDebugSocketMessageType(0xFu);
            Scr_WriteElement(element);
            Sys_EndWriteDebugSocket();
        }
        //elementa = Scr_ScriptWatch::CloneElement(&gScrDebuggerGlob[inst].scriptWatch, inst, element);
        elementa = gScrDebuggerGlob[inst].scriptWatch.CloneElement(inst, element);
        if (Sys_IsRemoteDebugServer())
        {
            //Scr_ScriptWatch::EvaluateWatchElement(&gScrDebuggerGlob[inst].scriptWatch, inst, elementa);
            gScrDebuggerGlob[inst].scriptWatch.EvaluateWatchElement(inst, elementa);
        }
    }
}

void __cdecl Scr_DisplayThreadPosRemote(scriptInstance_t inst)
{
    Scr_WatchElement_s *element; // [esp+0h] [ebp-4h]

    element = Scr_ReadElement(inst);
    if ( element )
        gScrDebuggerGlob[inst].scriptWatch.DisplayThreadPos(inst, element);
}

void __cdecl Scr_ToggleWatchElementBreakpointRemote(scriptInstance_t inst)
{
    unsigned __int8 type; // [esp+3h] [ebp-5h]
    Scr_WatchElement_s *element; // [esp+4h] [ebp-4h]

    element = Scr_ReadElement(inst);
    type = Sys_ReadDebugSocketInt();
    if ( element )
        gScrDebuggerGlob[inst].scriptWatch.ToggleBreakpointInternal(inst, element, type);
}

void __cdecl Scr_KeepAliveRemote(scriptInstance_t inst)
{
    if ( gScrDebuggerGlob[inst].atBreakpoint )
    {
        Sys_WriteDebugSocketMessageType(0x26u);
        Sys_EndWriteDebugSocket();
    }
}

int __cdecl Scr_UpdateDebugSocket(scriptInstance_t inst)
{
    int newEvent; // [esp+4h] [ebp-Ch]
    int dirty; // [esp+8h] [ebp-8h]

    dirty = gScrDebuggerGlob[inst].scriptWatch.dirty;
    newEvent = 0;
    while ( 2 )
    {
        switch ( Sys_UpdateDebugSocket() )
        {
            case 1:
            case 2:
            case 3:
            case 43:
                goto LABEL_20;
            case 5:
                Scr_AddTextRemote(inst);
                goto LABEL_20;
            case 7:
                Scr_PasteElementRemote(inst);
                goto LABEL_20;
            case 9:
                Scr_InsertElementRemote(inst);
                goto LABEL_20;
            case 11:
                Scr_DeleteElementRemote(inst);
                goto LABEL_20;
            case 13:
                Scr_BackspaceElementRemote(inst);
                goto LABEL_20;
            case 15:
                Scr_CloneElementRemote(inst);
                goto LABEL_20;
            case 17:
            case 18:
            case 22:
            case 23:
            case 28:
            case 29:
            case 30:
            case 31:
            case 32:
            case 33:
            case 34:
            case 35:
            case 36:
            case 37:
            case 40:
                //BLOPS_NULLSUB();
                goto LABEL_20;
            case 21:
                Scr_ToggleBreakpointRemote(inst);
                goto LABEL_20;
            case 27:
                Scr_ToggleWatchElementBreakpointRemote(inst);
                goto LABEL_20;
            case 38:
                Scr_KeepAliveRemote(inst);
                goto LABEL_20;
            case 41:
                //BLOPS_NULLSUB();
                if ( !dirty || gScrDebuggerGlob[inst].scriptWatch.dirty )
                    goto LABEL_20;
                newEvent = 1;
                Sys_AckDebugSocket();
                goto LABEL_19;
            case 42:
                Sys_ConsolePrintRemote(inst, 0);
                goto LABEL_20;
            case 44:
                Cbuf_AddText(0, "toggle cl_paused\n");
LABEL_20:
                newEvent = 1;
                Sys_AckDebugSocket();
                continue;
            default:
LABEL_19:
                Sys_FlushDebugSocketData();
                return newEvent;
        }
    }
}

void __cdecl Sys_ConsolePrintRemote(scriptInstance_t inst, int localClientNum)
{
    char *msg; // [esp+0h] [ebp-4h]

    msg = Sys_ReadDebugSocketString();
    CL_ConsolePrint(localClientNum, 24, msg, 0, 0, 0);
    FreeString(msg, 0, inst);
}

void __cdecl Scr_UpdateRemoteDebugger(scriptInstance_t inst)
{
    if (!Sys_IsMainThread()
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            9144,
            0,
            "%s",
            "Sys_IsMainThread()"))
    {
        __debugbreak();
    }
    if (gScrVarPub[inst].developer && gScrDebuggerGlob[inst].debugger_inited_system && Sys_IsRemoteDebugServer())
    {
        if (gScrDebuggerGlob[inst].disableDebuggerRemote)
        {
            Scr_DisplayDebuggerRemoteInternal(inst);
            gScrDebuggerGlob[inst].disableDebuggerRemote = 0;
        }
    }
}

void __cdecl Scr_UpdateDebugger(scriptInstance_t inst)
{
    int hitBreakpoint; // [esp+10h] [ebp-18h]
    bool updateBreakpoints; // [esp+17h] [ebp-11h]
    Scr_WatchElement_s *element; // [esp+1Ch] [ebp-Ch]
    VariableValue newValue; // [esp+20h] [ebp-8h] BYREF

    if (!gScrVarPub[inst].developer || !gScrDebuggerGlob[inst].debugger_inited_system)
        return;
    if (gScrDebuggerGlob[inst].atBreakpoint
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            9182,
            0,
            "%s",
            "!gScrDebuggerGlob[inst].atBreakpoint"))
    {
        __debugbreak();
    }
    if (!Scr_IsStackClear(inst)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            9183,
            0,
            "%s",
            "Scr_IsStackClear(inst)"))
    {
        __debugbreak();
    }
    if (!Scr_AllowBreakpoint(inst))
        return;
    if (gScrVarPub[inst].evaluate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            9203,
            0,
            "%s",
            "!gScrVarPub[inst].evaluate"))
    {
        __debugbreak();
    }
    gScrVarPub[inst].evaluate = 1;
    gScrDebuggerGlob[inst].scriptWatch.localId = 0;
    updateBreakpoints = 0;
    hitBreakpoint = 0;
retry_14:
    for (element = gScrDebuggerGlob[inst].scriptWatch.elementHead; element; element = element->next)
    {
        if (element->breakpointType == 1 && element->objectType != 13 && element->objectType != 21)
        {
            if (element->breakpoint
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    9218,
                    0,
                    "%s",
                    "!element->breakpoint"))
            {
                __debugbreak();
            }
            if (!element->expr.exprHead
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    9220,
                    0,
                    "%s",
                    "expr->exprHead"))
            {
                __debugbreak();
            }
            if (Scr_EvalScriptExpression(inst, &element->expr, 0, &newValue, 1, 1) && !updateBreakpoints)
            {
                Scr_ClearErrorMessage(inst);
                RemoveRefToValue(inst, newValue.type, newValue.u);
            retry2_0:
                updateBreakpoints = 1;
                //Scr_ScriptWatch::UpdateBreakpoints(&gScrDebuggerGlob[inst].scriptWatch, inst, 0);
                gScrDebuggerGlob[inst].scriptWatch.UpdateBreakpoints(inst, 0);
                goto retry_14;
            }
            if (gScrVarPub[inst].error_message)
            {
                Scr_ClearErrorMessage(inst);
                RemoveRefToValue(inst, newValue.type, newValue.u);
                if (!element->valueDefined)
                    continue;
                if (!updateBreakpoints)
                    goto retry2_0;
            }
            else if (Scr_WatchElementHasSameValue(inst, element, &newValue))
            {
                continue;
            }
            if (Scr_ConditionalExpression(inst, element, 0))
                hitBreakpoint = 1;
        }
    }
    if (Sys_IsRemoteDebugServer())
    {
        if (!updateBreakpoints)
        {
            updateBreakpoints = 1;
            //Scr_ScriptWatch::UpdateBreakpoints(&gScrDebuggerGlob[inst].scriptWatch, inst, 0);
            gScrDebuggerGlob[inst].scriptWatch.UpdateBreakpoints(inst, 0);
        }
        Scr_Evaluate(inst);
        if (Sys_IsRemoteDebugServer())
            Scr_ProcessDebugMessages(inst);
    }
    if (updateBreakpoints)
        gScrDebuggerGlob[inst].scriptWatch.UpdateBreakpoints(inst, 1);
    if (!gScrVarPub[inst].evaluate
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            9274,
            0,
            "%s",
            "gScrVarPub[inst].evaluate"))
    {
        __debugbreak();
    }
    gScrVarPub[inst].evaluate = 0;
    if (hitBreakpoint)
    {
    LABEL_51:
        Scr_StackClear(inst);
        //Scr_ScriptWatch::SortHitBreakpointsTop(&gScrDebuggerGlob[inst].scriptWatch, inst);
        gScrDebuggerGlob[inst].scriptWatch.SortHitBreakpointsTop(inst);
        Scr_RunDebugger(inst);
    }
    else if (gScrDebuggerGlob[inst].run_debugger)
    {
        gScrDebuggerGlob[inst].run_debugger = 0;
        goto LABEL_51;
    }
}

char __cdecl Scr_WatchElementHasSameValue(scriptInstance_t inst, Scr_WatchElement_s *element, VariableValue *newValue)
{
    int type; // eax
    VariableValue oldValue; // [esp+14h] [ebp-8h] BYREF

    if ( !element->valueDefined )
    {
        RemoveRefToValue(inst, newValue->type, newValue->u);
        return 0;
    }
    type = element->value.type;
    oldValue.u.intValue = element->value.u.intValue;
    oldValue.type = type;
    AddRefToValue(inst, type, oldValue.u);
    Scr_EvalEquality(inst, &oldValue, newValue);
    if (gScrVarPub[inst].error_message)
    {
        Scr_ClearErrorMessage(inst);
    }
    else
    {
        if ( oldValue.type != 6
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                        8817,
                        0,
                        "%s",
                        "oldValue.type == VAR_INTEGER") )
        {
            __debugbreak();
        }
        if ( oldValue.u.intValue )
        {
            if ( element->value.type != 1 )
                return 1;
            if ( newValue->type != 1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                            8822,
                            0,
                            "%s",
                            "newValue->type == VAR_POINTER") )
            {
                __debugbreak();
            }
            if ( GetObjectType(inst, newValue->u.intValue) == element->objectType )
                return 1;
        }
    }
    return 0;
}

int __cdecl Scr_HitAssignmentBreakpoint(
    scriptInstance_t inst,
    VariableValue *top,
    const char *pos,
    unsigned int localId,
    int forceBreak)
{
    char *BreakpointInfo; // ecx
    VariableUnion v7; // [esp+4h] [ebp-50h]
    bool enabled; // [esp+27h] [ebp-2Dh]
    int hitBreakpoint; // [esp+28h] [ebp-2Ch]
    Scr_WatchElementDoubleNode_t *breakpoints; // [esp+30h] [ebp-24h]
    Scr_WatchElementDoubleNode_t *breakpointsa; // [esp+30h] [ebp-24h]
    bool updateBreakpoints; // [esp+37h] [ebp-1Dh]
    Scr_WatchElementNode_s *elementNode; // [esp+38h] [ebp-1Ch]
    Scr_WatchElementNode_s *elementNodea; // [esp+38h] [ebp-1Ch]
    int opcode; // [esp+44h] [ebp-10h]
    Scr_WatchElement_s *element; // [esp+48h] [ebp-Ch]
    Scr_WatchElement_s *elementa; // [esp+48h] [ebp-Ch]
    VariableValue newValue; // [esp+4Ch] [ebp-8h] BYREF

    if (!gScrVarPub[inst].developer
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            9307,
            0,
            "%s",
            "gScrVarPub[inst].developer"))
    {
        __debugbreak();
    }
    if (gScrDebuggerGlob[inst].atBreakpoint
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            9308,
            0,
            "%s",
            "!gScrDebuggerGlob[inst].atBreakpoint"))
    {
        __debugbreak();
    }
    gScrDebuggerGlob[inst].scriptWatch.localId = 0;
    updateBreakpoints = 0;
    hitBreakpoint = 0;
retry_15:
    if (!gScrDebuggerGlob[inst].variableBreakpoints
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
            9315,
            0,
            "%s",
            "gScrDebuggerGlob[inst].variableBreakpoints"))
    {
        __debugbreak();
    }
    breakpoints = gScrDebuggerGlob[inst].variableBreakpoints[gScrDebuggerGlob[inst].objectId];
    if (breakpoints)
    {
        if (gScrVarPub[inst].evaluate
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                9319,
                0,
                "%s",
                "!gScrVarPub[inst].evaluate"))
        {
            __debugbreak();
        }
        gScrVarPub[inst].evaluate = 1;
        gScrVmPub[inst].top = top;
        for (elementNode = breakpoints->list; ; elementNode = elementNode->next)
        {
            if (!elementNode)
            {
                if (!gScrVarPub[inst].evaluate
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                        9378,
                        0,
                        "%s",
                        "gScrVarPub[inst].evaluate"))
                {
                    __debugbreak();
                }
                gScrVarPub[inst].evaluate = 0;
                break;
            }
            element = elementNode->element;
            if (!elementNode->element->breakpointType
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                    9327,
                    0,
                    "%s",
                    "element->breakpointType != SCR_BREAKPOINT_NONE"))
            {
                __debugbreak();
            }
            if (element->breakpointType == 1
                && element->objectType != 13
                && element->objectType != 21
                && !element->expr.breakonExpr)
            {
                if (element->breakpoint
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                        9335,
                        0,
                        "%s",
                        "!element->breakpoint"))
                {
                    __debugbreak();
                }
                if (!element->expr.exprHead
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                        9337,
                        0,
                        "%s",
                        "expr->exprHead"))
                {
                    __debugbreak();
                }
                if (Scr_EvalScriptExpression(inst, &element->expr, 0, &newValue, 1, 1) && !updateBreakpoints)
                {
                    Scr_ClearErrorMessage(inst);
                    RemoveRefToValue(inst, newValue.type, newValue.u);
                retry2_1:
                    updateBreakpoints = 1;
                    //Scr_ScriptWatch::UpdateBreakpoints(&gScrDebuggerGlob[inst].scriptWatch, inst, 0);
                    gScrDebuggerGlob[inst].scriptWatch.UpdateBreakpoints(inst, 0);
                    //Scr_ScriptWatch::UpdateBreakpoints(&gScrDebuggerGlob[inst].scriptWatch, inst, 1);
                    gScrDebuggerGlob[inst].scriptWatch.UpdateBreakpoints(inst, 1);
                    if (!gScrVarPub[inst].evaluate
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                            9350,
                            0,
                            "%s",
                            "gScrVarPub[inst].evaluate"))
                    {
                        __debugbreak();
                    }
                    gScrVarPub[inst].evaluate = 0;
                    goto retry_15;
                }
                if (gScrVarPub[inst].error_message)
                {
                    Scr_ClearErrorMessage(inst);
                    RemoveRefToValue(inst, newValue.type, newValue.u);
                    if (!element->valueDefined)
                        continue;
                    if (!updateBreakpoints)
                        goto retry2_1;
                }
                else if (Scr_WatchElementHasSameValue(inst, element, &newValue))
                {
                    continue;
                }
                if (Scr_ConditionalExpression(inst, element, localId))
                    hitBreakpoint = 1;
            }
        }
    }
    BreakpointInfo = Scr_FindBreakpointInfo(inst, pos - 1);
    opcode = (unsigned __int8)*BreakpointInfo;
    switch (*BreakpointInfo)
    {
    case 0:
    case 1:
        gScrDebuggerGlob[inst].objectId = 1;
        if (!gScrDebuggerGlob[inst].variableBreakpoints
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                9391,
                0,
                "%s",
                "gScrDebuggerGlob[inst].variableBreakpoints"))
        {
            __debugbreak();
        }
        breakpointsa = gScrDebuggerGlob[inst].variableBreakpoints[localId + 1];
        if (breakpointsa)
        {
            for (elementNodea = breakpointsa->list; elementNodea; elementNodea = elementNodea->next)
            {
                elementa = elementNodea->element;
                if (!elementNodea->element->breakpointType
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                        9398,
                        0,
                        "%s",
                        "element->breakpointType != SCR_BREAKPOINT_NONE"))
                {
                    __debugbreak();
                }
                if ((elementa->breakpointType == 1 || elementa->breakpointType == 3) && elementa->objectId == localId)
                {
                    enabled = elementa->breakpointType == 1;
                    Scr_WatchElementHitBreakpoint(elementa, enabled);
                    elementa->deadCodePos = pos;
                    if (enabled)
                        hitBreakpoint = 1;
                }
            }
        }
        break;
    case 13:
    case 43:
    case 56:
        gScrDebuggerGlob[inst].objectId = gScrVarPub[inst].levelId + 1;
        break;
    case 14:
    case 44:
    case 58:
        gScrDebuggerGlob[inst].objectId = gScrVarPub[inst].animId + 1;
        break;
    case 20:
        gScrDebuggerGlob[inst].objectId = gScrVarPub[inst].gameId + 0x8000;
        break;
    case 33:
    case 34:
    case 48:
    case 49:
    case 50:
    case 51:
    case 54:
    case 55:
    case 60:
    case 61:
        gScrDebuggerGlob[inst].objectId = localId + 1;
        break;
    case 35:
    case 36:
    case 46:
    case 47:
        break;
    case 38:
    case 45:
    case 59:
        gScrDebuggerGlob[inst].objectId = Scr_GetSelf(inst, localId) + 1;
        break;
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 89:
        if (top->type == 1)
            v7.intValue = (int)top->u.intValue;
        else
            v7.intValue = 0;
        gScrDebuggerGlob[inst].objectId = v7.intValue + 1;
        break;
    case 90:
        if (gScrVarPub[inst].evaluate
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                9451,
                0,
                "%s",
                "!gScrVarPub[inst].evaluate"))
        {
            __debugbreak();
        }
        gScrVarPub[inst].evaluate = 1;
        gScrDebuggerGlob[inst].objectId = Scr_EvalVariableObject(
            inst,
            gScrVmPub[inst].localVars[-*(unsigned __int8 *)pos])
            + 1;
        if (!gScrVarPub[inst].evaluate
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_debugger.cpp",
                9454,
                0,
                "%s",
                "gScrVarPub[inst].evaluate"))
        {
            __debugbreak();
        }
        gScrVarPub[inst].evaluate = 0;
        break;
    default:
        gScrDebuggerGlob[inst].objectId = 1;
        break;
    }
    if (hitBreakpoint || forceBreak)
        return Scr_HitBreakpoint(inst, top, pos, localId, hitBreakpoint);
    else
        return opcode;
}

bool __cdecl Scr_IgnoreErrors(scriptInstance_t inst)
{
    return gScrDebuggerGlob[inst].disableBreakpoints;
}

int __cdecl CompareThreadIndices(unsigned int *arg1, unsigned int *arg2)
{
    return *arg1 - *arg2;
}

