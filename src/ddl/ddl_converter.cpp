#include "ddl_converter.h"
#include <universal/q_shared.h>
#include <win32/win_shared.h>
#include <cstring>

bool __cdecl DDL_Converter_IsLeaf(ddlMemberDef_t *member)
{
    return member->type != 6 && member->type != 7;
}

void __cdecl DDL_Converter_CopyStates(
                const ddlState_t *const stateFrom,
                char *bufferFrom,
                const ddlState_t *const stateTo,
                char *bufferTo)
{
    char *String; // eax
    unsigned __int64 v5; // rax
    unsigned int Int; // eax

    if ( stateFrom->member->type == stateTo->member->type )
    {
        switch ( stateFrom->member->type )
        {
            case 0:
            case 1:
            case 2:
                Int = DDL_GetInt(stateFrom, bufferFrom);
                if ( !DDL_SetInt(stateTo, Int, bufferTo) )
                    DDL_PrintError("DDL: Found member '%s' but could not write value.\n", stateFrom->member->name);
                break;
            case 3:
                LODWORD(v5) = DDL_GetInt64(stateFrom, bufferFrom);
                if ( !DDL_SetInt64(stateTo, v5, bufferTo) )
                    DDL_PrintError("DDL: Found member '%s' but could not write value.\n", stateFrom->member->name);
                break;
            case 5:
                String = DDL_GetString(stateFrom, bufferFrom);
                if ( !DDL_SetString(stateTo, String, bufferTo) )
                    DDL_PrintError("DDL: Found member '%s' but could not write value.\n", stateFrom->member->name);
                break;
            default:
                DDL_PrintError("DDL: Invalid type reached during ddl conversion for member '%s'.\n", stateFrom->member->name);
                break;
        }
    }
}

void __cdecl DDL_Converter_ProcessStates(
                const ddlStructDef_t *const structDef,
                int structMemberItr,
                const ddlState_t *const oldState,
                const ddlState_t *const newState,
                char *oldBuffer,
                char *newBuffer)
{
    if ( oldState->member->type == 6 && newState->member->type == 6 )
    {
        DDL_Converter_TraverseStruct(
            &oldState->ddl->structList[structDef->members[structMemberItr].externalIndex],
            oldState,
            newState,
            oldBuffer,
            newBuffer);
    }
    else if ( DDL_Converter_IsLeaf(oldState->member) )
    {
        if ( DDL_Converter_IsLeaf(newState->member) )
            DDL_Converter_CopyStates(oldState, oldBuffer, newState, newBuffer);
    }
}

void __cdecl DDL_Converter_TraverseStruct(
                const ddlStructDef_t *const structDef,
                const ddlState_t *const oldBaseState,
                const ddlState_t *const newBaseState,
                char *oldBuffer,
                char *newBuffer)
{
    int memberArrayItr; // [esp+0h] [ebp-48h]
    int structMemberItr; // [esp+4h] [ebp-44h]
    ddlState_t newState; // [esp+8h] [ebp-40h] BYREF
    ddlState_t oldState; // [esp+18h] [ebp-30h] BYREF
    ddlState_t newArrayState; // [esp+28h] [ebp-20h] BYREF
    ddlState_t oldArrayState; // [esp+38h] [ebp-10h] BYREF

    for ( structMemberItr = 0; structMemberItr < structDef->memberCount; ++structMemberItr )
    {
        if ( DDL_MoveToName(newBaseState, &newState, structDef->members[structMemberItr].name)
            && DDL_MoveToName(oldBaseState, &oldState, structDef->members[structMemberItr].name) )
        {
            for ( memberArrayItr = 0; memberArrayItr < oldState.member->arraySize; ++memberArrayItr )
            {
                if ( oldState.member->arraySize <= 1 || newState.member->arraySize <= 1 )
                {
                    DDL_Converter_ProcessStates(structDef, structMemberItr, &oldState, &newState, oldBuffer, newBuffer);
                }
                else if ( oldState.member->enumIndex == -1 && newState.member->enumIndex == -1 )
                {
                    if ( memberArrayItr < newState.member->arraySize
                        && DDL_MoveToIndex(&oldState, &oldArrayState, memberArrayItr, 1) )
                    {
                        if ( DDL_MoveToIndex(&newState, &newArrayState, memberArrayItr, 1) )
                            DDL_Converter_ProcessStates(
                                structDef,
                                structMemberItr,
                                &oldArrayState,
                                &newArrayState,
                                oldBuffer,
                                newBuffer);
                    }
                }
                else if ( DDL_MoveToName(
                                        &oldState,
                                        &oldArrayState,
                                        oldState.ddl->enumList[oldState.member->enumIndex].members[memberArrayItr])
                             && DDL_MoveToName(
                                        &newState,
                                        &newArrayState,
                                        oldState.ddl->enumList[oldState.member->enumIndex].members[memberArrayItr]) )
                {
                    DDL_Converter_ProcessStates(structDef, structMemberItr, &oldArrayState, &newArrayState, oldBuffer, newBuffer);
                }
            }
        }
    }
}

int __cdecl DDL_Converter_Versions(
                ddlDef_t *ddlVersionFrom,
                ddlDef_t *ddlVersionTo,
                char *buffer,
                char *backupBuffer,
                int bufferSize)
{
    unsigned int v5; // eax
    ddlState_t newRootState; // [esp+0h] [ebp-24h] BYREF
    ddlState_t oldRootState; // [esp+10h] [ebp-14h] BYREF
    int startTime; // [esp+20h] [ebp-4h]

    memset(backupBuffer, 0, bufferSize);
    DDL_AssociateBuffer(backupBuffer, bufferSize, ddlVersionTo);
    DDL_Reset(&oldRootState, ddlVersionFrom);
    DDL_Reset(&newRootState, ddlVersionTo);
    startTime = Sys_Milliseconds();
    DDL_Converter_TraverseStruct(ddlVersionFrom->structList, &oldRootState, &newRootState, buffer, backupBuffer);
    memcpy((unsigned __int8 *)buffer, (unsigned __int8 *)backupBuffer, bufferSize);
    v5 = Sys_Milliseconds();
    DDL_NoCheckPrintf(
        "DDL: Conversion from version %d to version %d completed in %d ms.\n",
        ddlVersionFrom->version,
        ddlVersionTo->version,
        v5 - startTime);
    return 1;
}

