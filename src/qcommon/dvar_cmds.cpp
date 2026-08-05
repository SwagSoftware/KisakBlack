#include "dvar_cmds.h"
#include "cmd.h"
#include "common.h"
#include <cstring>
#include <universal/com_files.h>
#include <universal/com_shared.h>
#include <server_mp/sv_init_mp.h>
#include <stringed/stringed_hooks.h>
#include <win32/win_shared.h>
#include <DW/dwUtils_pc.h>
#include <live/live_win.h>
#include <cgame_mp/cg_main_mp.h>
#include <client/splitscreen.h>
#include <live/live_pcache.h>
#include <live/live_steam.h>
#include <live/live_pcache_profile.h>
#include <bgame/bg_emblems.h>

char info1[1024];
char info2[16384];

int __cdecl Dvar_Command(bool restricted)
{
    const char *v1; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // [esp-4h] [ebp-1024h]
    char combined[4096]; // [esp+18h] [ebp-1008h] BYREF
    dvar_s *dvar; // [esp+101Ch] [ebp-4h]

    v1 = Cmd_Argv(0);
    dvar = (dvar_s *)Dvar_FindVar(v1);
    if ( !dvar )
        return 0;
    if ( Dvar_GetBool("con_restricted_access") && restricted && (dvar->flags & 0x10000) == 0 )
    {
        Com_DPrintf(16, "ConAccess: dvar not set: %s.\n", dvar->name);
        return 0;
    }
    else if ( Cmd_Argc() == 1 )
    {
        v6 = Dvar_DisplayableResetValue(dvar);
        v3 = Dvar_DisplayableValue(dvar);
        Com_Printf(0, "\"%s\" is: \"%s^7\" default: \"%s^7\"\n", dvar->name, v3, v6);
        if ( Dvar_HasLatchedValue(dvar) )
        {
            v4 = Dvar_DisplayableLatchedValue(dvar);
            Com_Printf(0, "latched: \"%s\"\n", v4);
        }
        Dvar_PrintDomain(dvar->type, dvar->domain);
        return 1;
    }
    else
    {
        Dvar_GetCombinedString(combined, 1);
        v5 = Cmd_Argv(0);
        Dvar_SetCommand(v5, combined);
        return 1;
    }
}

void __cdecl Dvar_GetCombinedString(char *combined, int first)
{
    const char *v2; // eax
    int c; // [esp+28h] [ebp-10h]
    int l; // [esp+2Ch] [ebp-Ch]
    int len; // [esp+30h] [ebp-8h]

    c = Cmd_Argc();
    *combined = 0;
    l = 0;
    while ( first < c )
    {
        len = strlen(Cmd_Argv(first)) + 1;
        if ( len + l >= 4094 )
            break;
        v2 = Cmd_Argv(first);
        I_strncat(combined, 4096, v2);
        if ( first != c - 1 )
            I_strncat(combined, 4096, " ");
        l += len;
        ++first;
    }
}

void __cdecl Dvar_Toggle_f()
{
    Dvar_ToggleInternal();
}

bool __cdecl Dvar_ToggleInternal()
{
    const char *v0; // eax
    char *v2; // eax
    const char *string; // [esp+6Ch] [ebp-18h]
    int argIndex; // [esp+70h] [ebp-14h]
    const char *argString; // [esp+74h] [ebp-10h]
    char *argStringa; // [esp+74h] [ebp-10h]
    const char *dvarName; // [esp+78h] [ebp-Ch]
    dvar_s *dvar; // [esp+7Ch] [ebp-8h]
    const char *enumString; // [esp+80h] [ebp-4h]
    const char *enumStringa; // [esp+80h] [ebp-4h]

    if ( Cmd_Argc() >= 2 )
    {
        dvarName = Cmd_Argv(1);
        if ( !dvarName
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dvar_cmds.cpp", 257, 0, "%s", "dvarName") )
        {
            __debugbreak();
        }
        dvar = (dvar_s *)Dvar_FindVar(dvarName);
        if ( dvar )
        {
            if ( Cmd_Argc() == 2 )
            {
                return Dvar_ToggleSimple(dvar);
            }
            else
            {
                string = Dvar_DisplayableValue(dvar);
                for ( argIndex = 2; argIndex + 1 < Cmd_Argc(); ++argIndex )
                {
                    argString = Cmd_Argv(argIndex);
                    if ( dvar->type == DVAR_TYPE_ENUM )
                    {
                        enumString = Dvar_IndexStringToEnumString(dvar, argString);
                        if ( strlen(enumString) )
                            argString = enumString;
                    }
                    if ( !I_stricmp(string, argString) )
                    {
                        v2 = (char *)Cmd_Argv(argIndex + 1);
                        Dvar_SetCommand(dvarName, v2);
                        return 1;
                    }
                }
                argStringa = (char *)Cmd_Argv(2);
                if ( dvar->type == DVAR_TYPE_ENUM )
                {
                    enumStringa = Dvar_IndexStringToEnumString(dvar, argStringa);
                    if ( strlen(enumStringa) )
                        argStringa = (char *)enumStringa;
                }
                Dvar_SetCommand(dvarName, argStringa);
                return 1;
            }
        }
        else
        {
            Com_Printf(0, "toggle failed: dvar '%s' not found.\n", dvarName);
            return 0;
        }
    }
    else
    {
        if ( !Cmd_Argv(0)
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dvar_cmds.cpp", 251, 0, "%s", "Cmd_Argv( 0 )") )
        {
            __debugbreak();
        }
        v0 = Cmd_Argv(0);
        Com_Printf(0, "USAGE: %s <variable> <optional value sequence>\n", v0);
        return 0;
    }
}

bool __cdecl Dvar_ToggleSimple(dvar_s *dvar)
{
    bool result; // al
    const char *v2; // eax

    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dvar_cmds.cpp", 158, 0, "%s", "dvar") )
        __debugbreak();
    switch ( dvar->type )
    {
        case DVAR_TYPE_BOOL:
            Dvar_SetBoolFromSource(dvar, !dvar->current.enabled, DVAR_SOURCE_EXTERNAL);
            result = 1;
            break;
        case DVAR_TYPE_FLOAT:
            if ( dvar->domain.value.min > 0.0 || dvar->domain.value.max < 1.0 )
            {
                if ( dvar->current.value == dvar->domain.value.min )
                    Dvar_SetFloatFromSource(dvar, dvar->domain.value.max, DVAR_SOURCE_EXTERNAL);
                else
                    Dvar_SetFloatFromSource(dvar, dvar->domain.value.min, DVAR_SOURCE_EXTERNAL);
            }
            else if ( dvar->current.value == 0.0 )
            {
                Dvar_SetFloatFromSource(dvar, 1.0, DVAR_SOURCE_EXTERNAL);
            }
            else
            {
                Dvar_SetFloatFromSource(dvar, 0.0, DVAR_SOURCE_EXTERNAL);
            }
            result = 1;
            break;
        case DVAR_TYPE_FLOAT_2:
        case DVAR_TYPE_FLOAT_3:
        case DVAR_TYPE_FLOAT_4:
        case DVAR_TYPE_STRING:
        case DVAR_TYPE_COLOR:
        case DVAR_TYPE_LINEAR_COLOR_RGB:
        case DVAR_TYPE_COLOR_XYZ:
            Com_Printf(0, "'toggle' with no arguments makes no sense for dvar '%s'\n", dvar->name);
            result = 0;
            break;
        case DVAR_TYPE_INT:
            if ( dvar->domain.enumeration.stringCount > 0 || dvar->domain.integer.max < 1 )
            {
                if ( dvar->current.integer == dvar->domain.enumeration.stringCount )
                    Dvar_SetIntFromSource(dvar, dvar->domain.integer.max, DVAR_SOURCE_EXTERNAL);
                else
                    Dvar_SetIntFromSource(dvar, dvar->domain.enumeration.stringCount, DVAR_SOURCE_EXTERNAL);
            }
            else if ( dvar->current.integer )
            {
                Dvar_SetIntFromSource(dvar, 0, DVAR_SOURCE_EXTERNAL);
            }
            else
            {
                Dvar_SetIntFromSource(dvar, 1, DVAR_SOURCE_EXTERNAL);
            }
            result = 1;
            break;
        case DVAR_TYPE_ENUM:
            if ( dvar->domain.enumeration.stringCount )
                Dvar_SetIntFromSource(
                    dvar,
                    (dvar->current.integer + 1) % dvar->domain.enumeration.stringCount,
                    DVAR_SOURCE_EXTERNAL);
            result = 1;
            break;
        case DVAR_TYPE_INT64:
            if ( dvar->domain.integer64.min > 0 || dvar->domain.integer64.max <= 0 )
            {
                if ( dvar->current.integer == dvar->domain.enumeration.stringCount
                    && LODWORD(dvar->current.vector[1]) == dvar->domain.integer.max )
                {
                    Dvar_SetInt64FromSource(dvar, dvar->domain.integer64.max, DVAR_SOURCE_EXTERNAL);
                }
                else
                {
                    Dvar_SetInt64FromSource(dvar, dvar->domain.integer64.min, DVAR_SOURCE_EXTERNAL);
                }
            }
            else if ( dvar->current.integer64 )
            {
                Dvar_SetInt64FromSource(dvar, 0, DVAR_SOURCE_EXTERNAL);
            }
            else
            {
                Dvar_SetInt64FromSource(dvar, 1, DVAR_SOURCE_EXTERNAL);
            }
            result = 1;
            break;
        default:
            v2 = va("unhandled case %i", dvar->type);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dvar_cmds.cpp", 234, 1, v2) )
                __debugbreak();
            result = 0;
            break;
    }
    return result;
}

void __cdecl Dvar_TogglePrint_f()
{
    int v0; // eax
    const char *string; // [esp+18h] [ebp-Ch]
    const char *dvarName; // [esp+1Ch] [ebp-8h]
    const dvar_s *dvar; // [esp+20h] [ebp-4h]

    if ( Dvar_ToggleInternal() )
    {
        if ( Cmd_Argc() < 2 )
        {
            v0 = Cmd_Argc();
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dvar_cmds.cpp",
                            331,
                            0,
                            "%s\n\t(Cmd_Argc()) = %i",
                            "(Cmd_Argc() >= 2)",
                            v0) )
                __debugbreak();
        }
        dvarName = Cmd_Argv(1);
        if ( !dvarName
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dvar_cmds.cpp", 333, 1, "%s", "dvarName") )
        {
            __debugbreak();
        }
        dvar = Dvar_FindVar(dvarName);
        if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dvar_cmds.cpp", 335, 1, "%s", "dvar") )
            __debugbreak();
        string = Dvar_DisplayableValue(dvar);
        if ( !string
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dvar_cmds.cpp", 337, 1, "%s", "string") )
        {
            __debugbreak();
        }
        Com_Printf(0, "%s toggled to %s\n", dvarName, string);
    }
}

void __cdecl Dvar_Set_f()
{
    const char *v0; // eax
    const char *v1; // eax
    char combined[4096]; // [esp+24h] [ebp-1008h] BYREF
    char *dvarName; // [esp+1028h] [ebp-4h]

    if ( Cmd_Argc() >= 3 )
    {
        dvarName = (char *)Cmd_Argv(1);
        if ( Dvar_IsValidName(dvarName) )
        {
            Dvar_GetCombinedString(combined, 2);
            v1 = Cmd_Argv(1);
            Dvar_SetCommand(v1, combined);
        }
        else
        {
            v0 = Cmd_Argv(1);
            Com_Printf(0, "invalid variable name: %s\n", v0);
        }
    }
    else
    {
        Com_Printf(0, "USAGE: set <variable> <value>\n");
    }
}

void __cdecl Dvar_SetU_f()
{
    const char *v0; // eax
    const dvar_s *dvar; // [esp+10h] [ebp-4h]

    if ( Cmd_Argc() >= 3 )
    {
        Dvar_Set_f();
        v0 = Cmd_Argv(1);
        dvar = Dvar_FindVar(v0);
        if ( dvar )
            Dvar_AddFlags(dvar, 2);
    }
    else
    {
        Com_Printf(0, "USAGE: setu <variable> <value>\n");
    }
}

void __cdecl Dvar_SetS_f()
{
    const char *v0; // eax
    const dvar_s *dvar; // [esp+10h] [ebp-4h]

    if ( Cmd_Argc() >= 3 )
    {
        Dvar_Set_f();
        v0 = Cmd_Argv(1);
        dvar = Dvar_FindVar(v0);
        if ( dvar )
            Dvar_AddFlags(dvar, 4);
    }
    else
    {
        Com_Printf(0, "USAGE: sets <variable> <value>\n");
    }
}

void __cdecl Dvar_SetA_f()
{
    const char *v0; // eax
    const dvar_s *dvar; // [esp+10h] [ebp-4h]

    if ( Cmd_Argc() >= 3 )
    {
        Dvar_Set_f();
        v0 = Cmd_Argv(1);
        dvar = Dvar_FindVar(v0);
        if ( dvar )
            Dvar_AddFlags(dvar, 1);
    }
    else
    {
        Com_Printf(0, "USAGE: seta <variable> <value>\n");
    }
}

void __cdecl Dvar_SetAdmin_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const dvar_s *dvar; // [esp+18h] [ebp-4h]

    if ( Cmd_Argc() >= 3 )
    {
        v0 = Cmd_Argv(1);
        dvar = Dvar_FindVar(v0);
        if ( dvar )
        {
            if ( (dvar->flags & 0x10000) != 0 )
                Dvar_AddFlags(dvar, 1);
            Dvar_Set_f();
        }
        else
        {
            v1 = Cmd_Argv(1);
            Com_Printf(0, "setadmindvar failed: dvar '%s' not found.\n", v1);
        }
    }
    else
    {
        Com_Printf(0, "USAGE: setadmindvar <variable> <value>\n");
    }
}

void __cdecl Dvar_SetFromDvar_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    char *v3; // [esp-4h] [ebp-28h]
    const dvar_s *dvarSrc; // [esp+20h] [ebp-4h]

    if ( Cmd_Argc() == 3 )
    {
        v0 = Cmd_Argv(2);
        dvarSrc = Dvar_FindVar(v0);
        if ( dvarSrc )
        {
            v3 = (char *)Dvar_DisplayableValue(dvarSrc);
            v2 = Cmd_Argv(1);
            Dvar_SetCommand(v2, v3);
        }
        else
        {
            v1 = Cmd_Argv(2);
            Com_Printf(0, "dvar '%s' doesn't exist\n", v1);
        }
    }
    else
    {
        Com_Printf(0, "USAGE: setfromdvar <dest_dvar> <source_dvar>\n");
    }
}

void __cdecl Dvar_Reset_f()
{
    const char *v0; // eax
    dvar_s *dvar; // [esp+10h] [ebp-4h]

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        dvar = (dvar_s *)Dvar_FindVar(v0);
        if ( dvar )
            Dvar_Reset(dvar, DVAR_SOURCE_EXTERNAL);
    }
    else
    {
        Com_Printf(0, "USAGE: reset <variable>\n");
    }
}

void __cdecl Dvar_WriteVariables(int f)
{
    Dvar_ForEach((void (__cdecl *)(const dvar_s *, void *))Dvar_WriteSingleVariable, &f);
}

void __cdecl Dvar_WriteSingleVariable(const dvar_s *dvar, int *userData)
{
    const char *v2; // eax
    int f; // [esp+0h] [ebp-4h]

    if ( I_stricmp(dvar->name, "cl_cdkey") )
    {
        if ( (dvar->flags & 1) != 0 )
        {
            f = *userData;
            v2 = Dvar_DisplayableLatchedValue(dvar);
            FS_Printf(f, (char*)"seta %s \"%s\"\n", dvar->name, v2);
        }
    }
}

void __cdecl Dvar_WriteDefaults(int f)
{
    Dvar_ForEach((void (__cdecl *)(const dvar_s *, void *))Dvar_WriteSingleDefault, &f);
}

void __cdecl Dvar_WriteSingleDefault(const dvar_s *dvar, int *userData)
{
    const char *v2; // eax
    int f; // [esp+0h] [ebp-4h]

    if ( I_stricmp(dvar->name, "cl_cdkey") )
    {
        if ( (dvar->flags & 0x40C0) == 0 )
        {
            f = *userData;
            v2 = Dvar_DisplayableResetValue(dvar);
            FS_Printf(f, (char *)"set %s \"%s\"\n", dvar->name, v2);
        }
    }
}

int dvar_count_local;
void __cdecl Dvar_List_f()
{
    char *match; // [esp+10h] [ebp-4h]

    dvar_count_local = 0;
    if ( Cmd_Argc() <= 1 )
    {
        Dvar_ForEach((void (__cdecl *)(const dvar_s *, void *))Dvar_ListSingle, 0);
    }
    else
    {
        match = (char *)Cmd_Argv(1);
        Dvar_ForEach((void (__cdecl *)(const dvar_s *, void *))Dvar_ListSingle, match);
    }
    Com_Printf(0, "\n%i total dvars\n", dvar_count_local);
}

void __cdecl Dvar_ListSingle(const dvar_s *dvar, const char *userData)
{
    const char *v2; // eax

    if ( ((dvar->flags & 0x10000) != 0 || !Dvar_GetBool("con_restricted_access"))
        && (!userData || Com_Filter(userData, (char *)dvar->name, 0)) )
    {
        if ( (dvar->flags & 0x404) != 0 )
            Com_Printf(0, "S");
        else
            Com_Printf(0, " ");
        if ( (dvar->flags & 2) != 0 )
            Com_Printf(0, "U");
        else
            Com_Printf(0, " ");
        if ( (dvar->flags & 0x40) != 0 )
            Com_Printf(0, "R");
        else
            Com_Printf(0, " ");
        if ( (dvar->flags & 0x10) != 0 )
            Com_Printf(0, "I");
        else
            Com_Printf(0, " ");
        if ( (dvar->flags & 1) != 0 )
            Com_Printf(0, "A");
        else
            Com_Printf(0, " ");
        if ( (dvar->flags & 0x20) != 0 )
            Com_Printf(0, "L");
        else
            Com_Printf(0, " ");
        if ( (dvar->flags & 0x80) != 0 )
            Com_Printf(0, "C");
        else
            Com_Printf(0, " ");
        if ( (dvar->flags & 8) != 0 )
            Com_Printf(0, "Y");
        else
            Com_Printf(0, " ");
        if ( (dvar->flags & 0x100) != 0 )
            Com_Printf(0, "D");
        else
            Com_Printf(0, " ");
        Com_Printf(0, " ");
        if ( (dvar->flags & 0x8000) != 0 )
            Com_Printf(0, "X");
        else
            Com_Printf(0, " ");
        if ( (dvar->flags & 0x4000) != 0 )
            Com_Printf(0, "E");
        else
            Com_Printf(0, " ");
        if ( (dvar->flags & 0x1000) != 0 )
            Com_Printf(0, "V");
        else
            Com_Printf(0, " ");
        v2 = Dvar_DisplayableValue(dvar);
        Com_Printf(0, " %s \"%s\"\n", dvar->name, v2);
        ++dvar_count_local;
    }
}

void __cdecl Com_DvarDump(int channel, const char *match)
{
    DvarDumpInfo dumpInfo; // [esp+0h] [ebp-94h] BYREF
    char summary[132]; // [esp+Ch] [ebp-88h] BYREF

    if ( channel != 6 || com_logfile && com_logfile->current.integer )
    {
        Com_PrintMessage(channel, (char*)"=============================== DVAR DUMP ========================================\n", 0);
        dumpInfo.count = 0;
        dumpInfo.channel = channel;
        dumpInfo.match = match;
        Dvar_ForEach(Com_DvarDumpSingle, &dumpInfo);
        Com_sprintf(summary, 0x80u, "\n%i total dvars\n%i dvar indexes\n", dumpInfo.count, dvarCount);
        Com_PrintMessage(channel, summary, 0);
        Com_PrintMessage(
            channel,
            (char*)"=============================== END DVAR DUMP =====================================\n",
            0);
    }
}

void __cdecl Com_DvarDumpSingle(const dvar_s *dvar, void *userData)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // [esp-4h] [ebp-810h]
    char message[2052]; // [esp+4h] [ebp-808h] BYREF

    ++*(unsigned int *)userData;
    if ( !*((unsigned int *)userData + 2) || Com_Filter(*((const char **)userData + 2), (char *)dvar->name, 0) )
    {
        if ( Dvar_HasLatchedValue(dvar) )
        {
            v4 = Dvar_DisplayableLatchedValue(dvar);
            v2 = Dvar_DisplayableValue(dvar);
            Com_sprintf(message, 0x800u, "            %s \"%s\" -- latched \"%s\"\n", dvar->name, v2, v4);
        }
        else
        {
            v3 = Dvar_DisplayableValue(dvar);
            Com_sprintf(message, 0x800u, "            %s \"%s\"\n", dvar->name, v3);
        }
        Com_PrintMessage(*((unsigned int *)userData + 1), message, 0);
    }
}

void __cdecl Dvar_Dump_f()
{
    const char *match; // [esp+10h] [ebp-4h]

    if ( Cmd_Argc() <= 1 )
    {
        Com_DvarDump(0, 0);
    }
    else
    {
        match = Cmd_Argv(1);
        Com_DvarDump(0, match);
    }
}

void __cdecl SV_SetConfig(int start, int max, int bit)
{
    DvarSetConfigInfo info; // [esp+0h] [ebp-Ch] BYREF

    info.start = start;
    info.max = max;
    info.bit = bit;
    Dvar_ForEach((void (__cdecl *)(const dvar_s *, void *))SV_SetConfigDvar, &info);
}

void __cdecl SV_SetConfigDvar(const dvar_s *dvar, int *userData)
{
    char *v2; // eax

    if ( (userData[2] & dvar->flags) != 0 )
    {
        v2 = (char *)Dvar_DisplayableValue(dvar);
        SV_SetConfigValueForKey(*userData, userData[1], (char *)dvar->name, v2);
    }
}

char *__cdecl Dvar_InfoString(int localClientNum, int bit)
{
    const char *Username; // eax
    unsigned __int64 v3; // rax
    char *v4; // eax
    __int64 Uid; // rax
    const char *v6; // eax

    unsigned int RandomUInt; // eax
    unsigned int v10; // eax
    unsigned __int64 ourUserID; // [esp+80h] [ebp-288h] BYREF
    char tempbuf[8]; // [esp+90h] [ebp-278h] BYREF
    char temp64buff[184]; // [esp+98h] [ebp-270h] BYREF
    char clientSteamIDStr[32]; // [esp+150h] [ebp-1B8h] BYREF
    PCachePublicProfile *profile; // [esp+174h] [ebp-194h]
    CompositeEmblemLayer layers[12]; // [esp+178h] [ebp-190h] BYREF
    __int16 backgroundID; // [esp+2FCh] [ebp-Ch] BYREF
    clientConnection_t *clc; // [esp+300h] [ebp-8h]
    int index; // [esp+304h] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    info1[0] = 0;
    Dvar_ForEach((void (__cdecl *)(const dvar_s *, void *))Dvar_InfoStringSingle, &bit);
    if ( (bit & 2) != 0 )
    {
        index = localClientNum;
        Username = CL_ControllerIndex_GetUsername();
        Info_SetValueForKey(info1, (char*)"name", Username);
        if ( clanName && *(_BYTE *)clanName->current.integer )
        {
            I_strncpyz(tempbuf, clanName->current.string, 5);
            I_CleanStr(tempbuf);
            Info_SetValueForKey(info1, (char *)"clanAbbrev", tempbuf);
        }
        else
        {
            Info_SetValueForKey(info1, (char *)"clanAbbrev", "");
        }
        if ( Live_IsSignedInToLive() )
        {
            PCache_Lock();
            LODWORD(v3) = Live_GetXuid(index);
            profile = (PCachePublicProfile *)PCache_GetComponent(index, v3, 0);
            if ( PCache_TouchComponent(&profile->c) )
            {
                PCache_GetProfileEmblem(profile, layers, 12, &backgroundID);
                v4 = BG_EmblemsWriteString(backgroundID, layers, 12);
                Info_SetValueForKey(info1, (char *)"emblem", v4);
            }
            PCache_Unlock();
        }
        Uid = LiveSteam_GetUid();
        Int64ToString(Uid, clientSteamIDStr);
        Info_SetValueForKey(info1, (char *)"steamid", clientSteamIDStr);
        if ( !clc->qport
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\dvar_cmds.cpp",
                        1125,
                        0,
                        "%s",
                        "clc->qport != 0") )
        {
            __debugbreak();
        }
        v6 = va("%i", clc->qport);
        Info_SetValueForKey(info1, (char *)"qport", v6);
#ifdef KISAK_LIVE_SERVICE
        bdTrulyRandomImpl *Instance; // eax
        bdTrulyRandomImpl *v9; // eax
        if ( live_service && live_service->current.enabled )
        {
            memset(temp64buff, 0, 0xB1u);
            dwGetOnlineUserID(0, &ourUserID);
            Com_DPrintf(14, "CHALLENGERESPONSE: Sending bdOnlineUserID %llu to server\n", ourUserID);
            XUIDToString(&ourUserID, temp64buff);
            Info_SetValueForKey(info1, "bdOnlineUserID", temp64buff);
            memset(temp64buff, 0, 0xB1u);
            if ( !clc->nonce )
            {
                Instance = bdSingleton<bdTrulyRandomImpl>::getInstance();
                RandomUInt = bdTrulyRandomImpl::getRandomUInt(Instance);
                clc->nonce = RandomUInt;
                v9 = bdSingleton<bdTrulyRandomImpl>::getInstance();
                v10 = bdTrulyRandomImpl::getRandomUInt(v9);
                *(&clc->nonce + 1) = v10;
            }
            Com_sprintf(temp64buff, 0xB1u, "%u", *(&clc->nonce + 1));
            Info_SetValueForKey(info1, "nonce", temp64buff);
            Com_sprintf(temp64buff, 0xB1u, "%u", clc->nonce);
            Info_SetValueForKey(info1, "challengeNum", temp64buff);
            Com_DPrintf(
                14,
                "CHALLENGERESPONSE: Sending nonce %u, challengeNum %u to server\n",
                *(&clc->nonce + 1),
                clc->nonce);
        }
#endif
    }
    return info1;
}

void __cdecl Dvar_InfoStringSingle(const dvar_s *dvar, unsigned int *userData)
{
    const char *v2; // eax

    if ( (*userData & dvar->flags) != 0 )
    {
        v2 = Dvar_DisplayableValue(dvar);
        Info_SetValueForKey(info1, (char *)dvar->name, v2);
    }
}

char *__cdecl Dvar_InfoString_Big(int bit)
{
    info2[0] = 0;
    Dvar_ForEach((void (__cdecl *)(const dvar_s *, void *))Dvar_InfoStringSingle_Big, &bit);
    return info2;
}

void __cdecl Dvar_InfoStringSingle_Big(const dvar_s *dvar, unsigned int *userData)
{
    const char *v2; // eax

    if ( (*userData & dvar->flags) != 0 )
    {
        v2 = Dvar_DisplayableValue(dvar);
        Info_SetValueForKey_Big(info2, (char *)dvar->name, v2);
    }
}

cmd_function_s Dvar_Toggle_f_VAR;
cmd_function_s Dvar_TogglePrint_f_VAR;
cmd_function_s Dvar_Set_f_VAR;
cmd_function_s Dvar_SetS_f_VAR;
cmd_function_s Dvar_SetA_f_VAR;
cmd_function_s Dvar_SetAdmin_f_VAR;
cmd_function_s Dvar_SetFromDvar_f_VAR;
cmd_function_s Dvar_SetFromLocalizedStr_f_VAR;
cmd_function_s Dvar_SetToTime_f_VAR;
cmd_function_s Dvar_Reset_f_VAR;
cmd_function_s Dvar_List_f_VAR;
cmd_function_s Dvar_Dump_f_VAR;
cmd_function_s Dvar_RegisterBool_f_VAR;
cmd_function_s Dvar_RegisterInt_f_VAR;
cmd_function_s Dvar_RegisterFloat_f_VAR;
cmd_function_s Dvar_RegisterColor_f_VAR;
cmd_function_s Dvar_SetU_f_VAR;
cmd_function_s Dvar_RestoreDvars_VAR;

void __cdecl Dvar_AddCommands()
{
    Cmd_AddCommandInternal("toggle", Dvar_Toggle_f, &Dvar_Toggle_f_VAR);
    Cmd_AddCommandInternal("togglep", Dvar_TogglePrint_f, &Dvar_TogglePrint_f_VAR);
    Cmd_AddCommandInternal("set", Dvar_Set_f, &Dvar_Set_f_VAR);
    Cmd_AddCommandInternal("sets", Dvar_SetS_f, &Dvar_SetS_f_VAR);
    Cmd_AddCommandInternal("seta", Dvar_SetA_f, &Dvar_SetA_f_VAR);
    Cmd_AddCommandInternal("setadmindvar", Dvar_SetAdmin_f, &Dvar_SetAdmin_f_VAR);
    Cmd_AddCommandInternal("setfromdvar", Dvar_SetFromDvar_f, &Dvar_SetFromDvar_f_VAR);
    Cmd_AddCommandInternal("setfromlocString", Dvar_SetFromLocalizedStr_f, &Dvar_SetFromLocalizedStr_f_VAR);
    Cmd_AddCommandInternal("setdvartotime", Dvar_SetToTime_f, &Dvar_SetToTime_f_VAR);
    Cmd_AddCommandInternal("reset", Dvar_Reset_f, &Dvar_Reset_f_VAR);
    Cmd_AddCommandInternal("dvarlist", Dvar_List_f, &Dvar_List_f_VAR);
    Cmd_AddCommandInternal("dvardump", Dvar_Dump_f, &Dvar_Dump_f_VAR);
    Cmd_AddCommandInternal("dvar_bool", Dvar_RegisterBool_f, &Dvar_RegisterBool_f_VAR);
    Cmd_AddCommandInternal("dvar_int", Dvar_RegisterInt_f, &Dvar_RegisterInt_f_VAR);
    Cmd_AddCommandInternal("dvar_float", Dvar_RegisterFloat_f, &Dvar_RegisterFloat_f_VAR);
    Cmd_AddCommandInternal("dvar_color", Dvar_RegisterColor_f, &Dvar_RegisterColor_f_VAR);
    Cmd_AddCommandInternal("setu", Dvar_SetU_f, &Dvar_SetU_f_VAR);
    Cmd_AddCommandInternal("restoreDvars", Dvar_RestoreDvars, &Dvar_RestoreDvars_VAR);
}

void __cdecl Dvar_RegisterBool_f()
{
    const char *v0; // eax
    const char *v1; // eax
    char *dvarName; // [esp+20h] [ebp-10h]
    const dvar_s *dvar; // [esp+24h] [ebp-Ch]
    bool value; // [esp+2Fh] [ebp-1h]

    if ( Cmd_Argc() == 3 )
    {
        dvarName = (char *)Cmd_Argv(1);
        v1 = Cmd_Argv(2);
        value = atoi(v1) != 0;
        dvar = Dvar_FindVar(dvarName);
        if ( !dvar || dvar->type == DVAR_TYPE_STRING && (dvar->flags & 0x4000) != 0 )
        {
            _Dvar_RegisterBool(dvarName, value, 0x4000u, "External Dvar");
        }
        else if ( dvar->type )
        {
            Com_Printf(0, "dvar '%s' is not a boolean dvar\n", dvar->name);
        }
    }
    else
    {
        v0 = Cmd_Argv(0);
        Com_Printf(0, "USAGE: %s <name> <default>\n", v0);
    }
}

void __cdecl Dvar_RegisterInt_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    int max; // [esp+30h] [ebp-18h]
    const char *dvarName; // [esp+34h] [ebp-14h]
    int min; // [esp+38h] [ebp-10h]
    const dvar_s *dvar; // [esp+3Ch] [ebp-Ch]
    int value; // [esp+44h] [ebp-4h]

    if ( Cmd_Argc() == 5 )
    {
        dvarName = Cmd_Argv(1);
        v1 = Cmd_Argv(2);
        value = atoi(v1);
        v2 = Cmd_Argv(3);
        min = atoi(v2);
        v3 = Cmd_Argv(4);
        max = atoi(v3);
        if ( min <= max )
        {
            dvar = Dvar_FindVar(dvarName);
            if ( !dvar || dvar->type == DVAR_TYPE_STRING && (dvar->flags & 0x4000) != 0 )
            {
                _Dvar_RegisterInt(dvarName, value, min, max, 0x4000u, "External Dvar");
            }
            else if ( dvar->type != DVAR_TYPE_INT && dvar->type != DVAR_TYPE_ENUM )
            {
                Com_Printf(0, "dvar '%s' is not an integer dvar\n", dvar->name);
            }
        }
        else
        {
            Com_Printf(0, "dvar %s: min %i should not be greater than max %i\n", dvarName, min, max);
        }
    }
    else
    {
        v0 = Cmd_Argv(0);
        Com_Printf(0, "USAGE: %s <name> <default> <min> <max>\n", v0);
    }
}

void __cdecl Dvar_RegisterFloat_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    float max; // [esp+44h] [ebp-18h]
    const char *dvarName; // [esp+48h] [ebp-14h]
    float min; // [esp+4Ch] [ebp-10h]
    const dvar_s *dvar; // [esp+50h] [ebp-Ch]
    float value; // [esp+58h] [ebp-4h]

    if ( Cmd_Argc() == 5 )
    {
        dvarName = Cmd_Argv(1);
        v1 = Cmd_Argv(2);
        value = atof(v1);
        v2 = Cmd_Argv(3);
        min = atof(v2);
        v3 = Cmd_Argv(4);
        max = atof(v3);
        if ( min <= max )
        {
            dvar = Dvar_FindVar(dvarName);
            if ( !dvar || dvar->type == DVAR_TYPE_STRING && (dvar->flags & 0x4000) != 0 )
            {
                _Dvar_RegisterFloat(dvarName, value, min, max, 0x4000u, "External Dvar");
            }
            else if ( dvar->type != DVAR_TYPE_FLOAT )
            {
                Com_Printf(0, "dvar '%s' is not an integer dvar\n", dvar->name);
            }
        }
        else
        {
            Com_Printf(0, "dvar %s: min %g should not be greater than max %g\n", dvarName, min, max);
        }
    }
    else
    {
        v0 = Cmd_Argv(0);
        Com_Printf(0, "USAGE: %s <name> <default> <min> <max>\n", v0);
    }
}

void __cdecl Dvar_RegisterColor_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    float r; // [esp+50h] [ebp-1Ch]
    float g; // [esp+54h] [ebp-18h]
    float b; // [esp+58h] [ebp-14h]
    const char *dvarName; // [esp+5Ch] [ebp-10h]
    const dvar_s *dvar; // [esp+60h] [ebp-Ch]
    float a; // [esp+64h] [ebp-8h]
    int argc; // [esp+68h] [ebp-4h]

    argc = Cmd_Argc();
    if ( argc == 5 || argc == 6 )
    {
        dvarName = Cmd_Argv(1);
        v1 = Cmd_Argv(2);
        r = atof(v1);
        v2 = Cmd_Argv(3);
        g = atof(v2);
        v3 = Cmd_Argv(4);
        b = atof(v3);
        if ( argc == 6 )
        {
            v4 = Cmd_Argv(5);
            a = atof(v4);
        }
        else
        {
            a = 1.0f;
        }
        dvar = Dvar_FindVar(dvarName);
        if ( !dvar || dvar->type == DVAR_TYPE_STRING && (dvar->flags & 0x4000) != 0 )
            _Dvar_RegisterColor(dvarName, r, g, b, a, 0x4000u, "External Dvar");
    }
    else
    {
        v0 = Cmd_Argv(0);
        Com_Printf(0, "USAGE: %s <name> <r> <g> <b> [a]\n", v0);
    }
}

void __cdecl Dvar_SetFromLocalizedStr_f()
{
    const char *v0; // eax
    const char *v1; // eax
    char combined; // [esp+24h] [ebp-1010h] BYREF
    char pszInputBuffer[4099]; // [esp+25h] [ebp-100Fh] BYREF
    char *dvarName; // [esp+102Ch] [ebp-8h]
    char *src; // [esp+1030h] [ebp-4h]

    if ( Cmd_Argc() >= 3 )
    {
        dvarName = (char *)Cmd_Argv(1);
        if ( Dvar_IsValidName(dvarName) )
        {
            Dvar_GetCombinedString(&combined, 2);
            if ( combined == 64 )
            {
                src = SEH_LocalizeTextMessage(pszInputBuffer, "dvar string", LOCMSG_NOERR);
                if ( src )
                {
                    if ( *src )
                        I_strncpyz(&combined, src, 4096);
                }
            }
            v1 = Cmd_Argv(1);
            Dvar_SetCommand(v1, &combined);
        }
        else
        {
            v0 = Cmd_Argv(1);
            Com_Printf(0, "invalid variable name: %s\n", v0);
        }
    }
    else
    {
        Com_Printf(0, "USAGE: setFromLocalizedString <variable> <string>\n");
    }
}

void __cdecl Dvar_SetToTime_f()
{
    const char *v0; // eax
    unsigned int v1; // eax
    const char *v2; // eax
    char *v3; // [esp-4h] [ebp-2Ch]
    const char *dvarName; // [esp+24h] [ebp-4h]

    if ( Cmd_Argc() >= 2 )
    {
        dvarName = Cmd_Argv(1);
        if ( Dvar_IsValidName(dvarName) )
        {
            v1 = Sys_Milliseconds();
            v3 = va("%i", v1);
            v2 = Cmd_Argv(1);
            Dvar_SetCommand(v2, v3);
        }
        else
        {
            v0 = Cmd_Argv(1);
            Com_Printf(0, "invalid variable name: %s\n", v0);
        }
    }
    else
    {
        Com_Printf(0, "USAGE: set <variable>\n");
    }
}

