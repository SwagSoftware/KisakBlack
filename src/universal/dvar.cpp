#include "dvar.h"
#include <database/db_registry.h>

#include <algorithm>

#include <gfx_d3d/r_material_load_obj.h>
#include <ctype.h>
#include "assertive.h"
#include "q_shared.h"
#include <qcommon/common.h>
#include <server_mp/sv_bot_mp.h>
#include <win32/win_common.h>
#include <win32/win_net.h>
#include "com_memory.h"
#include "q_parse.h"
#include <qcommon/dvar_cmds.h>

int dvar_modifiedFlags;
FastCriticalSection g_dvarCritSect;
bool areDvarsSorted;

const dvar_s *sortedDvars[4096];
dvar_s *dvarHashTable[1024];
dvar_s dvarPool[4096];
const dvar_s *dvar_cheats;
const dvar_s *dvar_restoreDvarsOnLive;
int dvarCount;
bool isLoadingAutoExecGlobalFlag;
bool isDvarSystemActive;

dvar_s *__cdecl Dvar_FindMalleableVar_0(int dvarHash)
{
    dvar_s *var; // [esp+0h] [ebp-4h]

    _InterlockedExchangeAdd(&g_dvarCritSect.readCount, 1u);
    while (g_dvarCritSect.writeCount)
        NET_Sleep(0);
    for (var = dvarHashTable[dvarHash & 0x3FF]; var; var = var->hashNext)
    {
        if (var->hash == dvarHash)
        {
            Sys_UnlockRead(&g_dvarCritSect);
            return var;
        }
    }
    Sys_UnlockRead(&g_dvarCritSect);
    return 0;
}

bool Material_CachedDvarNameLess(const dvar_s *dvar0, const dvar_s *dvar1)
{
    return I_stricmp(dvar0->name, dvar1->name) < 0;
}

void __cdecl Dvar_Sort()
{
    Sys_LockWrite(&g_dvarCritSect);
    if ( !areDvarsSorted )
    {
        //std::_Sort<Material * *,int,bool (__cdecl *)(Material const *,Material const *)>(
        //    (const GfxStaticModelDrawInst **)sortedDvars,
        //    (const GfxStaticModelDrawInst **)(4 * dvarCount + 161434600),
        //    (4 * dvarCount + 161434600 - (int)sortedDvars) >> 2,
        //    (bool (__cdecl *)(const GfxStaticModelDrawInst *, const GfxStaticModelDrawInst *))Material_CachedShaderTextLess);

        std::sort(
            sortedDvars,
            sortedDvars + dvarCount,
            Material_CachedDvarNameLess
            //Material_CachedShaderTextLess
        );

        areDvarsSorted = 1;
    }
    Sys_UnlockWrite(&g_dvarCritSect);
}

void __cdecl Dvar_ForEach(void (__cdecl *callback)(const dvar_s *, void *), void *userData)
{
    int dvarIter; // [esp+0h] [ebp-4h]

    Dvar_Sort();
    for ( dvarIter = 0; dvarIter < dvarCount; ++dvarIter )
        callback(sortedDvars[dvarIter], userData);
}

void __cdecl Dvar_ForEachName(void (__cdecl *callback)(const char *))
{
    int dvarIter; // [esp+0h] [ebp-4h]

    Dvar_Sort();
    for ( dvarIter = 0; dvarIter < dvarCount; ++dvarIter )
        callback(sortedDvars[dvarIter]->name);
}

void __cdecl Dvar_SetInAutoExec(bool inAutoExec)
{
    isLoadingAutoExecGlobalFlag = inAutoExec;
}

bool __cdecl Dvar_IsSystemActive()
{
    return isDvarSystemActive;
}

char __cdecl Dvar_IsValidName(const char *dvarName)
{
    char nameChar; // [esp+3h] [ebp-5h]
    int index; // [esp+4h] [ebp-4h]

    if ( !dvarName )
        return 0;
    for ( index = 0; dvarName[index]; ++index )
    {
        nameChar = dvarName[index];
        if ( !isalnum(nameChar) && nameChar != 95 )
            return 0;
    }
    return 1;
}

const char *__cdecl Dvar_EnumToString(const dvar_s *dvar)
{
    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 238, 0, "%s", "dvar") )
        __debugbreak();
    if ( !dvar->name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 239, 0, "%s", "dvar->name") )
    {
        __debugbreak();
    }
    if ( dvar->type != DVAR_TYPE_ENUM
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    240,
                    0,
                    "%s\n\t(dvar->name) = %s",
                    "(dvar->type == DVAR_TYPE_ENUM)",
                    dvar->name) )
    {
        __debugbreak();
    }
    if ( !dvar->domain.integer.max
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    241,
                    0,
                    "%s\n\t(dvar->name) = %s",
                    "(dvar->domain.enumeration.strings)",
                    dvar->name) )
    {
        __debugbreak();
    }
    if ( (dvar->current.integer < 0 || dvar->current.integer >= dvar->domain.enumeration.stringCount)
        && dvar->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    242,
                    0,
                    "%s\n\t(dvar->current.integer) = %i",
                    "(dvar->current.integer >= 0 && dvar->current.integer < dvar->domain.enumeration.stringCount || dvar->current.integer == 0)",
                    dvar->current.integer) )
    {
        __debugbreak();
    }
    if ( dvar->domain.enumeration.stringCount )
        return *(const char **)(dvar->domain.integer.max + 4 * dvar->current.integer);
    else
        return "";
}

const char *__cdecl Dvar_IndexStringToEnumString(const dvar_s *dvar, const char *indexString)
{
    signed int v3; // kr00_4
    int enumIndex; // [esp+14h] [ebp-8h]
    int indexStringIndex; // [esp+18h] [ebp-4h]

    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 256, 0, "%s", "dvar") )
        __debugbreak();
    if ( !dvar->name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 257, 0, "%s", "dvar->name") )
    {
        __debugbreak();
    }
    if ( dvar->type != DVAR_TYPE_ENUM
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    258,
                    0,
                    "%s\n\t(dvar->name) = %s",
                    "(dvar->type == DVAR_TYPE_ENUM)",
                    dvar->name) )
    {
        __debugbreak();
    }
    if ( !dvar->domain.integer.max
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    259,
                    0,
                    "%s\n\t(dvar->name) = %s",
                    "(dvar->domain.enumeration.strings)",
                    dvar->name) )
    {
        __debugbreak();
    }
    if ( !indexString
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    260,
                    0,
                    "%s\n\t(dvar->name) = %s",
                    "(indexString)",
                    dvar->name) )
    {
        __debugbreak();
    }
    if ( !dvar->domain.enumeration.stringCount )
        return "";
    v3 = strlen(indexString);
    for ( indexStringIndex = 0; indexStringIndex < v3; ++indexStringIndex )
    {
        if ( !isdigit(indexString[indexStringIndex]) )
            return "";
    }
    enumIndex = atoi(indexString);
    if ( enumIndex >= 0 && enumIndex < dvar->domain.enumeration.stringCount )
        return *(const char **)(dvar->domain.integer.max + 4 * enumIndex);
    else
        return "";
}

const char *__cdecl Dvar_DisplayableValue(const dvar_s *dvar)
{
    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 501, 0, "%s", "dvar") )
        __debugbreak();
    return Dvar_ValueToString(dvar, dvar->current);
}

const char *__cdecl Dvar_ValueToString(const dvar_s *dvar, DvarValue value)
{
    const char *result; // eax
    const char *v3; // eax
    const char *v4; // [esp+20h] [ebp-Ch]

    switch ( dvar->type )
    {
        case DVAR_TYPE_BOOL:
            if ( value.enabled )
                v4 = "1";
            else
                v4 = "0";
            result = v4;
            break;
        case DVAR_TYPE_FLOAT:
            result = va("%g", value.value);
            break;
        case DVAR_TYPE_FLOAT_2:
            result = va("%g %g", value.value, value.vector[1]);
            break;
        case DVAR_TYPE_FLOAT_3:
        case DVAR_TYPE_LINEAR_COLOR_RGB:
        case DVAR_TYPE_COLOR_XYZ:
            result = va("%g %g %g", value.value, value.vector[1], value.vector[2]);
            break;
        case DVAR_TYPE_FLOAT_4:
            result = va("%g %g %g %g", value.value, value.vector[1], value.vector[2], value.vector[3]);
            break;
        case DVAR_TYPE_INT:
            result = va("%i", value.integer);
            break;
        case DVAR_TYPE_ENUM:
            if ( (value.integer < 0 || value.integer >= dvar->domain.enumeration.stringCount)
                && value.integer
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                            311,
                            0,
                            "%s\n\t(value.integer) = %i",
                            "(value.integer >= 0 && value.integer < dvar->domain.enumeration.stringCount || value.integer == 0)",
                            value.integer) )
            {
                __debugbreak();
            }
            if ( dvar->domain.enumeration.stringCount )
                result = *(const char **)(dvar->domain.integer.max + 4 * value.integer);
            else
                result = "";
            break;
        case DVAR_TYPE_STRING:
            if ( !value.integer
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                            317,
                            0,
                            "%s\n\t(dvar->name) = %s",
                            "(value.string)",
                            dvar->name) )
            {
                __debugbreak();
            }
            result = va("%s", value.string);
            break;
        case DVAR_TYPE_COLOR:
            result = va(
                                 "%g %g %g %g",
                                 (float)((float)value.color[0] * 0.0039215689),
                                 (float)((float)value.color[1] * 0.0039215689),
                                 (float)((float)value.color[2] * 0.0039215689),
                                 (float)((float)value.color[3] * 0.0039215689));
            break;
        case DVAR_TYPE_INT64:
            result = va("%lli", value.integer64);
            break;
        default:
            v3 = va("unhandled dvar type '%i'", dvar->type);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 322, 1, v3) )
                __debugbreak();
            result = "";
            break;
    }
    return result;
}

const char *__cdecl Dvar_DisplayableResetValue(const dvar_s *dvar)
{
    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 509, 0, "%s", "dvar") )
        __debugbreak();
    return Dvar_ValueToString(dvar, dvar->reset);
}

const char *__cdecl Dvar_DisplayableLatchedValue(const dvar_s *dvar)
{
    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 517, 0, "%s", "dvar") )
        __debugbreak();
    return Dvar_ValueToString(dvar, dvar->latched);
}

char __cdecl Dvar_ValueInDomain(dvarType_t type, DvarValue value, DvarLimits domain)
{
    char result; // al
    const char *v4; // eax
    bool v5; // [esp+8h] [ebp-Ch]

    switch ( type )
    {
        case DVAR_TYPE_BOOL:
            if ( value.color[0] != 1
                && value.enabled
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                            628,
                            0,
                            "%s",
                            "value.enabled == true || value.enabled == false") )
            {
                __debugbreak();
            }
            result = 1;
            break;
        case DVAR_TYPE_FLOAT:
            result = domain.value.min <= value.value && value.value <= domain.value.max;
            break;
        case DVAR_TYPE_FLOAT_2:
            result = Dvar_VectorInDomain(&value.value, 2, domain.value.min, domain.value.max);
            break;
        case DVAR_TYPE_FLOAT_3:
        case DVAR_TYPE_LINEAR_COLOR_RGB:
        case DVAR_TYPE_COLOR_XYZ:
            result = Dvar_VectorInDomain(&value.value, 3, domain.value.min, domain.value.max);
            break;
        case DVAR_TYPE_FLOAT_4:
            result = Dvar_VectorInDomain(&value.value, 4, domain.value.min, domain.value.max);
            break;
        case DVAR_TYPE_INT:
            if ( domain.enumeration.stringCount > domain.integer.max
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                            632,
                            0,
                            "%s",
                            "domain.integer.min <= domain.integer.max") )
            {
                __debugbreak();
            }
            result = value.integer >= domain.enumeration.stringCount && value.integer <= domain.integer.max;
            break;
        case DVAR_TYPE_ENUM:
            v5 = value.integer >= 0 && value.integer < domain.enumeration.stringCount || !value.integer;
            result = v5;
            break;
        case DVAR_TYPE_STRING:
            result = 1;
            break;
        case DVAR_TYPE_COLOR:
            result = 1;
            break;
        case DVAR_TYPE_INT64:
            if ( domain.integer64.min > domain.integer64.max
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                            640,
                            0,
                            "%s",
                            "domain.integer64.min <= domain.integer64.max") )
            {
                __debugbreak();
            }
            result = value.integer64 >= domain.integer64.min && value.integer64 <= domain.integer64.max;
            break;
        default:
            v4 = va("unhandled dvar type '%i'", type);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 678, 1, v4) )
                __debugbreak();
            result = 0;
            break;
    }
    return result;
}

char __cdecl Dvar_VectorInDomain(const float *vector, int components, float min, float max)
{
    int channel; // [esp+0h] [ebp-4h]

    for ( channel = 0; channel < components; ++channel )
    {
        if ( min > vector[channel] )
            return 0;
        if ( vector[channel] > max )
            return 0;
    }
    return 1;
}

char *__cdecl Dvar_DomainToString(dvarType_t type, DvarLimits domain, char *outBuffer, int outBufferLen)
{
    return Dvar_DomainToString_Internal(type, domain, outBuffer, outBufferLen, 0);
}

char *__cdecl Dvar_DomainToString_Internal(
                dvarType_t type,
                DvarLimits domain,
                char *outBuffer,
                int outBufferLen,
                int *outLineCount)
{
    const char *v5; // eax
    char *outBufferEnd; // [esp+14h] [ebp-10h]
    char *outBufferWalk; // [esp+18h] [ebp-Ch]
    int charsWritten; // [esp+1Ch] [ebp-8h]
    int charsWrittena; // [esp+1Ch] [ebp-8h]
    int stringIndex; // [esp+20h] [ebp-4h]

    if ( outBufferLen <= 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 710, 0, "%s", "outBufferLen > 0") )
    {
        __debugbreak();
    }
    outBufferEnd = &outBuffer[outBufferLen];
    if ( outLineCount )
        *outLineCount = 0;
    switch ( type )
    {
        case DVAR_TYPE_BOOL:
            _snprintf(outBuffer, outBufferLen, "Domain is 0 or 1");
            break;
        case DVAR_TYPE_FLOAT:
            if ( domain.value.min == -3.4028235e38 )
            {
                if ( domain.value.max == 3.4028235e38 )
                    _snprintf(outBuffer, outBufferLen, "Domain is any number");
                else
                    _snprintf(outBuffer, outBufferLen, "Domain is any number %g or smaller", domain.value.max);
            }
            else if ( domain.value.max == 3.4028235e38 )
            {
                _snprintf(outBuffer, outBufferLen, "Domain is any number %g or bigger", domain.value.min);
            }
            else
            {
                _snprintf(outBuffer, outBufferLen, "Domain is any number from %g to %g", domain.value.min, domain.value.max);
            }
            break;
        case DVAR_TYPE_FLOAT_2:
            Dvar_VectorDomainToString(2, domain, outBuffer, outBufferLen);
            break;
        case DVAR_TYPE_FLOAT_3:
        case DVAR_TYPE_LINEAR_COLOR_RGB:
        case DVAR_TYPE_COLOR_XYZ:
            Dvar_VectorDomainToString(3, domain, outBuffer, outBufferLen);
            break;
        case DVAR_TYPE_FLOAT_4:
            Dvar_VectorDomainToString(4, domain, outBuffer, outBufferLen);
            break;
        case DVAR_TYPE_INT:
            if ( domain.enumeration.stringCount == 0x80000000 )
            {
                if ( domain.integer.max == 0x7FFFFFFF )
                    goto LABEL_21;
                _snprintf(outBuffer, outBufferLen, "Domain is any integer %i or smaller", domain.integer.max);
            }
            else if ( domain.integer.max == 0x7FFFFFFF )
            {
                _snprintf(outBuffer, outBufferLen, "Domain is any integer %i or bigger", domain.enumeration.stringCount);
            }
            else
            {
                _snprintf(
                    outBuffer,
                    outBufferLen,
                    "Domain is any integer from %i to %i",
                    domain.enumeration.stringCount,
                    domain.integer.max);
            }
            break;
        case DVAR_TYPE_ENUM:
            charsWritten = _snprintf(outBuffer, outBufferLen, "Domain is one of the following:");
            if ( charsWritten >= 0 )
            {
                outBufferWalk = &outBuffer[charsWritten];
                for ( stringIndex = 0; stringIndex < domain.enumeration.stringCount; ++stringIndex )
                {
                    charsWrittena = _snprintf(
                                                        outBufferWalk,
                                                        outBufferEnd - outBufferWalk,
                                                        "\n    %2i: %s",
                                                        stringIndex,
                                                        *(const char **)(domain.integer.max + 4 * stringIndex));
                    if ( charsWrittena < 0 )
                        break;
                    if ( outLineCount )
                        ++*outLineCount;
                    outBufferWalk += charsWrittena;
                }
            }
            break;
        case DVAR_TYPE_STRING:
            _snprintf(outBuffer, outBufferLen, "Domain is any text");
            break;
        case DVAR_TYPE_COLOR:
            _snprintf(outBuffer, outBufferLen, "Domain is any 4-component color, in RGBA format");
            break;
        case DVAR_TYPE_INT64:
            if ( !domain.enumeration.stringCount && domain.integer.max == 0x80000000 )
            {
                if ( domain.integer64.max == 0x7FFFFFFFFFFFFFFFLL )
LABEL_21:
                    _snprintf(outBuffer, outBufferLen, "Domain is any integer");
                else
                    _snprintf(outBuffer, outBufferLen, "Domain is any integer %lli or smaller", domain.integer64.max);
            }
            else if ( domain.integer64.max == 0x7FFFFFFFFFFFFFFFLL )
            {
                _snprintf(outBuffer, outBufferLen, "Domain is any integer %lli or bigger", domain.integer64.min);
            }
            else
            {
                _snprintf(
                    outBuffer,
                    outBufferLen,
                    "Domain is any integer from %lli to %lli",
                    domain.integer64.min,
                    domain.integer64.max);
            }
            break;
        default:
            v5 = va("unhandled dvar type '%i'", type);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 815, 1, v5) )
                __debugbreak();
            *outBuffer = 0;
            break;
    }
    *(outBufferEnd - 1) = 0;
    return outBuffer;
}

void __cdecl Dvar_VectorDomainToString(int components, DvarLimits domain, char *outBuffer, unsigned int outBufferLen)
{
    if ( domain.value.min == -3.4028235e38 )
    {
        if ( domain.value.max == 3.4028235e38 )
            _snprintf(outBuffer, outBufferLen, "Domain is any %iD vector", components);
        else
            _snprintf(
                outBuffer,
                outBufferLen,
                "Domain is any %iD vector with components %g or smaller",
                components,
                domain.value.max);
    }
    else if ( domain.value.max == 3.4028235e38 )
    {
        _snprintf(
            outBuffer,
            outBufferLen,
            "Domain is any %iD vector with components %g or bigger",
            components,
            domain.value.min);
    }
    else
    {
        _snprintf(
            outBuffer,
            outBufferLen,
            "Domain is any %iD vector with components from %g to %g",
            components,
            domain.value.min,
            domain.value.max);
    }
}

char *__cdecl Dvar_DomainToString_GetLines(
                dvarType_t type,
                DvarLimits domain,
                char *outBuffer,
                int outBufferLen,
                int *outLineCount)
{
    if ( !outLineCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 833, 0, "%s", "outLineCount") )
    {
        __debugbreak();
    }
    return Dvar_DomainToString_Internal(type, domain, outBuffer, outBufferLen, outLineCount);
}

void __cdecl Dvar_PrintDomain(dvarType_t type, DvarLimits domain)
{
    char *v2; // eax
    char domainBuffer[1028]; // [esp+0h] [ebp-408h] BYREF

    v2 = Dvar_DomainToString(type, domain, domainBuffer, 1024);
    Com_Printf(16, "    %s\n", v2);
}

bool __cdecl Dvar_HasLatchedValue(const dvar_s *dvar)
{
    return !Dvar_ValuesEqual(dvar->type, dvar->current, dvar->latched);
}

bool __cdecl Dvar_ValuesEqual(dvarType_t type, DvarValue val0, DvarValue val1)
{
    bool result; // eax
    const char *v4; // eax
    bool v5; // [esp+14h] [ebp-1Ch]
    bool v6; // [esp+18h] [ebp-18h]
    bool dvarValue; // [esp+1Ch] [ebp-14h]

    switch ( type )
    {
        case DVAR_TYPE_BOOL:
            result = val0.color[0] == val1.color[0];
            break;
        case DVAR_TYPE_FLOAT:
            result = val0.value == val1.value;
            break;
        case DVAR_TYPE_FLOAT_2:
            dvarValue = val0.value == val1.value && val0.vector[1] == val1.vector[1];
            result = dvarValue;
            break;
        case DVAR_TYPE_FLOAT_3:
        case DVAR_TYPE_LINEAR_COLOR_RGB:
        case DVAR_TYPE_COLOR_XYZ:
            v6 = val0.value == val1.value && val0.vector[1] == val1.vector[1] && val0.vector[2] == val1.vector[2];
            result = v6;
            break;
        case DVAR_TYPE_FLOAT_4:
            v5 = val0.value == val1.value
                && val0.vector[1] == val1.vector[1]
                && val0.vector[2] == val1.vector[2]
                && val0.vector[3] == val1.vector[3];
            result = v5;
            break;
        case DVAR_TYPE_INT:
            result = val0.integer == val1.integer;
            break;
        case DVAR_TYPE_ENUM:
            result = val0.integer == val1.integer;
            break;
        case DVAR_TYPE_STRING:
            if ( !val0.integer
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 879, 0, "%s", "val0.string") )
            {
                __debugbreak();
            }
            if ( !val1.integer
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 880, 0, "%s", "val1.string") )
            {
                __debugbreak();
            }
            result = strcmp(val0.string, val1.string) == 0;
            break;
        case DVAR_TYPE_COLOR:
            result = val0.integer == val1.integer;
            break;
        case DVAR_TYPE_INT64:
            result = val0.integer64 == val1.integer64;
            break;
        default:
            v4 = va("unhandled dvar type '%i'", type);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 885, 1, v4) )
                __debugbreak();
            result = 0;
            break;
    }
    return result;
}

const dvar_s *__cdecl Dvar_FindVar(const char *dvarName)
{
    if ( !dvarName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 1184, 0, "%s", "dvarName") )
    {
        __debugbreak();
    }
    if ( dvarName && *dvarName )
        return Dvar_FindMalleableVar(dvarName);
    else
        return 0;
}

dvar_s *__cdecl Dvar_FindMalleableVar(const char *dvarName)
{
    int hash; // [esp+Ch] [ebp-4h]

    hash = Com_HashString(dvarName, 0);
    return Dvar_FindMalleableVar_0(hash);
}

void __cdecl Sys_UnlockRead(FastCriticalSection *critSect)
{
    if ( critSect->readCount <= 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\universal\\../qcommon/threads_interlock.h",
                    78,
                    0,
                    "%s",
                    "critSect->readCount > 0") )
    {
        __debugbreak();
    }
    _InterlockedExchangeAdd(&critSect->readCount, 0xFFFFFFFF);
}

const dvar_s *__cdecl Dvar_FindVar(int dvarHash)
{
    return Dvar_FindMalleableVar_0(dvarHash);
}

void __cdecl Dvar_ClearModified(const dvar_s *dvar)
{
    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 1201, 0, "%s", "dvar") )
        __debugbreak();
    ((dvar_s*)dvar)->modified = 0;
}

void __cdecl Dvar_SetModified(const dvar_s *dvar)
{
    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 1208, 0, "%s", "dvar") )
        __debugbreak();
    ((dvar_s *)dvar)->modified = 1;
}

void __cdecl Dvar_UpdateEnumDomain(dvar_s *dvar, const char **stringTable)
{
    const char *v2; // eax
    const char *v3; // eax
    DvarValue result; // [esp+0h] [ebp-28h] BYREF
    DvarValue v5; // [esp+10h] [ebp-18h]
    int stringCount; // [esp+20h] [ebp-8h]
    dvar_s *malleableDvar; // [esp+24h] [ebp-4h]

    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 1218, 0, "%s", "dvar") )
        __debugbreak();
    if ( !dvar->name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 1219, 0, "%s", "dvar->name") )
    {
        __debugbreak();
    }
    if ( !stringTable
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    1220,
                    0,
                    "%s\n\t(dvar->name) = %s",
                    "(stringTable)",
                    dvar->name) )
    {
        __debugbreak();
    }
    if ( dvar->type != DVAR_TYPE_ENUM )
    {
        v2 = va("dvar %s type %i", dvar->name, dvar->type);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                        1221,
                        0,
                        "%s\n\t%s",
                        "dvar->type == DVAR_TYPE_ENUM",
                        v2) )
            __debugbreak();
    }
    for ( stringCount = 0; stringTable[stringCount]; ++stringCount )
        ;
    if ( dvar->reset.integer < 0 || dvar->reset.integer >= stringCount && dvar->reset.integer )
    {
        v3 = va("name %s reset %i count %i", dvar->name, dvar->reset.integer, stringCount);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                        1226,
                        0,
                        "%s\n\t%s",
                        "dvar->reset.integer >= 0 && (dvar->reset.integer < stringCount || dvar->reset.integer == 0)",
                        v3) )
            __debugbreak();
    }
    malleableDvar = dvar;
    dvar->domain.enumeration.stringCount = stringCount;
    malleableDvar->domain.integer.max = (int)stringTable;
    v5 = *Dvar_ClampValueToDomain(&result, dvar->type, dvar->current, dvar->reset, dvar->domain);
    malleableDvar->current = v5;
    malleableDvar->latched = dvar->current;
}

DvarValue *__cdecl Dvar_ClampValueToDomain(
                DvarValue *result,
                dvarType_t type,
                DvarValue value,
                DvarValue resetValue,
                DvarLimits domain)
{
    const char *v5; // eax

    switch ( type )
    {
        case DVAR_TYPE_BOOL:
            value.enabled = value.color[0] != 0;
            break;
        case DVAR_TYPE_FLOAT:
            if ( domain.value.min <= value.value )
            {
                if ( value.value > domain.value.max )
                    value.integer = domain.integer.max;
            }
            else
            {
                value.integer = domain.integer.min;
            }
            break;
        case DVAR_TYPE_FLOAT_2:
            Dvar_ClampVectorToDomain(&value.value, 2, domain.value.min, domain.value.max);
            break;
        case DVAR_TYPE_FLOAT_3:
        case DVAR_TYPE_LINEAR_COLOR_RGB:
        case DVAR_TYPE_COLOR_XYZ:
            Dvar_ClampVectorToDomain(&value.value, 3, domain.value.min, domain.value.max);
            break;
        case DVAR_TYPE_FLOAT_4:
            Dvar_ClampVectorToDomain(&value.value, 4, domain.value.min, domain.value.max);
            break;
        case DVAR_TYPE_INT:
            if (domain.integer.min > domain.integer.max
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                            561,
                            0,
                            "%s",
                            "domain.integer.min <= domain.integer.max") )
            {
                __debugbreak();
            }
            if ( value.integer >= domain.integer.min)
            {
                if ( value.integer > domain.integer.max )
                    value.integer = domain.integer.max;
            }
            else
            {
                value.integer = domain.integer.min;
            }
            break;
        case DVAR_TYPE_ENUM:
            if ( value.integer < 0 || value.integer >= domain.integer.min)
            {
                value.integer = resetValue.integer;
                if ( (resetValue.integer < 0 || value.integer >= domain.enumeration.stringCount)
                    && value.integer
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                                605,
                                0,
                                "%s\n\t(value.integer) = %i",
                                "(value.integer >= 0 && value.integer < domain.enumeration.stringCount || value.integer == 0)",
                                value.integer) )
                {
                    __debugbreak();
                }
            }
            break;
        case DVAR_TYPE_STRING:
        case DVAR_TYPE_COLOR:
            break;
        case DVAR_TYPE_INT64:
            if ( domain.integer64.min > domain.integer64.max
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                            569,
                            0,
                            "%s",
                            "domain.integer64.min <= domain.integer64.max") )
            {
                __debugbreak();
            }
            if ( value.integer64 >= domain.integer64.min )
            {
                if ( value.integer64 > domain.integer64.max )
                    value.integer64 = domain.integer64.max;
            }
            else
            {
                value.integer64 = domain.integer64.min;
            }
            break;
        default:
            v5 = va("unhandled dvar type '%i'", type);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 615, 1, v5) )
                __debugbreak();
            break;
    }
    *result = value;
    return result;
}

void __cdecl Dvar_ClampVectorToDomain(float *vector, int components, float min, float max)
{
    int channel; // [esp+0h] [ebp-4h]

    for ( channel = 0; channel < components; ++channel )
    {
        if ( min <= vector[channel] )
        {
            if ( vector[channel] > max )
                vector[channel] = max;
        }
        else
        {
            vector[channel] = min;
        }
    }
}

bool __cdecl Dvar_GetBool(const char *dvarName)
{
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    if ( !dvar )
        return 0;
    if ( dvar->type
        && (dvar->type != DVAR_TYPE_STRING || (dvar->flags & 0x4000) == 0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    1244,
                    0,
                    "%s\n\t(dvar->type) = %i",
                    "(dvar->type == DVAR_TYPE_BOOL || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))))",
                    dvar->type) )
    {
        __debugbreak();
    }
    if ( dvar->type )
        return Dvar_StringToBool(dvar->current.string);
    else
        return dvar->current.enabled;
}

bool __cdecl Dvar_StringToBool(const char *string)
{
    if ( !string && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 330, 0, "%s", "string") )
        __debugbreak();
    return atoi(string) != 0;
}

int __cdecl Dvar_GetInt(const char *dvarName)
{
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    if ( !dvar )
        return 0;
    if ( dvar->type
        && dvar->type != DVAR_TYPE_INT
        && dvar->type != DVAR_TYPE_FLOAT
        && dvar->type != DVAR_TYPE_ENUM
        && (dvar->type != DVAR_TYPE_STRING || (dvar->flags & 0x4000) == 0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    1260,
                    0,
                    "%s\n\t(dvar->type) = %i",
                    "(dvar->type == DVAR_TYPE_BOOL || dvar->type == DVAR_TYPE_INT || dvar->type == DVAR_TYPE_FLOAT || dvar->type =="
                    " DVAR_TYPE_ENUM || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))))",
                    dvar->type) )
    {
        __debugbreak();
    }
    switch ( dvar->type )
    {
        case DVAR_TYPE_INT:
        case DVAR_TYPE_ENUM:
            return dvar->current.integer;
        case DVAR_TYPE_BOOL:
            return dvar->current.color[0] != 0;
        case DVAR_TYPE_FLOAT:
            return (int)dvar->current.value;
    }
    return Dvar_StringToInt(dvar->current.string);
}

int __cdecl Dvar_StringToInt(const char *string)
{
    if ( !string && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 337, 0, "%s", "string") )
        __debugbreak();
    return atoi(string);
}

int __cdecl Dvar_GetInt(const dvar_s *dvar)
{
    if ( !dvar )
        return 0;
    if ( dvar->type
        && dvar->type != DVAR_TYPE_INT
        && dvar->type != DVAR_TYPE_FLOAT
        && dvar->type != DVAR_TYPE_ENUM
        && (dvar->type != DVAR_TYPE_STRING || (dvar->flags & 0x4000) == 0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    1278,
                    0,
                    "%s\n\t(dvar->type) = %i",
                    "(dvar->type == DVAR_TYPE_BOOL || dvar->type == DVAR_TYPE_INT || dvar->type == DVAR_TYPE_FLOAT || dvar->type =="
                    " DVAR_TYPE_ENUM || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))))",
                    dvar->type) )
    {
        __debugbreak();
    }
    switch ( dvar->type )
    {
        case DVAR_TYPE_INT:
        case DVAR_TYPE_ENUM:
            return dvar->current.integer;
        case DVAR_TYPE_BOOL:
            return dvar->current.color[0] != 0;
        case DVAR_TYPE_FLOAT:
            return (int)dvar->current.value;
    }
    return Dvar_StringToInt(dvar->current.string);
}

int __cdecl Dvar_GetInt(int dvarHash)
{
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = Dvar_FindVar(dvarHash);
    if ( !dvar )
        return 0;
    if ( dvar->type
        && dvar->type != DVAR_TYPE_INT
        && dvar->type != DVAR_TYPE_FLOAT
        && dvar->type != DVAR_TYPE_ENUM
        && (dvar->type != DVAR_TYPE_STRING || (dvar->flags & 0x4000) == 0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    1299,
                    0,
                    "%s\n\t(dvar->type) = %i",
                    "(dvar->type == DVAR_TYPE_BOOL || dvar->type == DVAR_TYPE_INT || dvar->type == DVAR_TYPE_FLOAT || dvar->type =="
                    " DVAR_TYPE_ENUM || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))))",
                    dvar->type) )
    {
        __debugbreak();
    }
    switch ( dvar->type )
    {
        case DVAR_TYPE_INT:
        case DVAR_TYPE_ENUM:
            return dvar->current.integer;
        case DVAR_TYPE_BOOL:
            return dvar->current.color[0] != 0;
        case DVAR_TYPE_FLOAT:
            return (int)dvar->current.value;
    }
    return Dvar_StringToInt(dvar->current.string);
}

__int64 __cdecl Dvar_StringToInt64(const char *string)
{
    if ( !string && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 343, 0, "%s", "string") )
        __debugbreak();
    return _atoi64(string);
}

double __cdecl Dvar_GetFloat(const char *dvarName)
{
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    if ( !dvar )
        return 0.0;
    if ( dvar->type != DVAR_TYPE_FLOAT
        && dvar->type != DVAR_TYPE_INT
        && (dvar->type != DVAR_TYPE_STRING || (dvar->flags & 0x4000) == 0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    1340,
                    0,
                    "%s\n\t(dvar->type) = %i",
                    "(dvar->type == DVAR_TYPE_FLOAT || dvar->type == DVAR_TYPE_INT ||(dvar->type == DVAR_TYPE_STRING && (dvar->flag"
                    "s & (1 << 14))))",
                    dvar->type) )
    {
        __debugbreak();
    }
    if ( dvar->type == DVAR_TYPE_FLOAT )
        return dvar->current.value;
    if ( dvar->type == DVAR_TYPE_INT )
        return (double)dvar->current.integer;
    return Dvar_StringToFloat(dvar->current.string);
}

double __cdecl Dvar_StringToFloat(const char *string)
{
    if ( !string && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 355, 0, "%s", "string") )
        __debugbreak();
    return (float)atof(string);
}

double __cdecl Dvar_GetFloat(int dvarHash)
{
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = Dvar_FindVar(dvarHash);
    if ( !dvar )
        return 0.0;
    if ( dvar->type != DVAR_TYPE_FLOAT
        && dvar->type != DVAR_TYPE_INT
        && (dvar->type != DVAR_TYPE_STRING || (dvar->flags & 0x4000) == 0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    1358,
                    0,
                    "%s\n\t(dvar->type) = %i",
                    "(dvar->type == DVAR_TYPE_FLOAT || dvar->type == DVAR_TYPE_INT || (dvar->type == DVAR_TYPE_STRING && (dvar->fla"
                    "gs & (1 << 14))))",
                    dvar->type) )
    {
        __debugbreak();
    }
    if ( dvar->type == DVAR_TYPE_FLOAT )
        return dvar->current.value;
    if ( dvar->type == DVAR_TYPE_INT )
        return (double)dvar->current.integer;
    return Dvar_StringToFloat(dvar->current.string);
}

const char *__cdecl Dvar_GetString(const char *dvarName)
{
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    if ( !dvar )
        return "";
    if ( dvar->type != DVAR_TYPE_STRING
        && dvar->type != DVAR_TYPE_ENUM
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    1393,
                    0,
                    "%s\n\t(dvar->type) = %i",
                    "(dvar->type == DVAR_TYPE_STRING || dvar->type == DVAR_TYPE_ENUM)",
                    dvar->type) )
    {
        __debugbreak();
    }
    if ( dvar->type == DVAR_TYPE_ENUM )
        return Dvar_EnumToString(dvar);
    else
        return dvar->current.string;
}

const char *__cdecl Dvar_GetVariantString(const char *dvarName)
{
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    if ( dvar )
        return Dvar_ValueToString(dvar, dvar->current);
    else
        return "";
}

const char *__cdecl Dvar_GetVariantString(int dvarHash)
{
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = Dvar_FindVar(dvarHash);
    if ( dvar )
        return Dvar_ValueToString(dvar, dvar->current);
    else
        return "";
}

void __cdecl Dvar_GetUnpackedColor(const dvar_s *dvar, float *expandedColor)
{
    unsigned __int8 color[4]; // [esp+0h] [ebp-4h] BYREF

    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 1426, 0, "%s", "dvar") )
        __debugbreak();
    if ( dvar->type != DVAR_TYPE_COLOR
        && (dvar->type != DVAR_TYPE_STRING || (dvar->flags & 0x4000) == 0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    1427,
                    0,
                    "%s\n\t(dvar->type) = %i",
                    "(dvar->type == DVAR_TYPE_COLOR || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))))",
                    dvar->type) )
    {
        __debugbreak();
    }
    if ( dvar->type == DVAR_TYPE_COLOR )
        *(unsigned int *)color = dvar->current.integer;
    else
        Dvar_StringToColor(dvar->current.string, color);
    *expandedColor = (float)color[0] * 0.0039215689;
    expandedColor[1] = (float)color[1] * 0.0039215689;
    expandedColor[2] = (float)color[2] * 0.0039215689;
    expandedColor[3] = (float)color[3] * 0.0039215689;
}

void __cdecl Dvar_StringToColor(const char *string, unsigned __int8 *color)
{
    float v2; // [esp+0h] [ebp-80h]
    float v3; // [esp+4h] [ebp-7Ch]
    float v4; // [esp+8h] [ebp-78h]
    float v5; // [esp+Ch] [ebp-74h]
    float v6; // [esp+24h] [ebp-5Ch]
    float v7; // [esp+3Ch] [ebp-44h]
    float v8; // [esp+54h] [ebp-2Ch]
    float v9; // [esp+6Ch] [ebp-14h]
    float colorVec[4]; // [esp+70h] [ebp-10h] BYREF

    memset(colorVec, 0, sizeof(colorVec));
    sscanf(string, "%g %g %g %g", colorVec, &colorVec[1], &colorVec[2], &colorVec[3]);
    if ((float)(colorVec[0] - 1.0) < 0.0)
        v9 = colorVec[0];
    else
        v9 = 1.0f;
    if ((float)(0.0 - v9) < 0.0)
        v5 = v9;
    else
        v5 = 0.0f;
    *color = (int)((float)(255.0 * v5) + 9.313225746154785e-10);
    if ((float)(colorVec[1] - 1.0) < 0.0)
        v8 = colorVec[1];
    else
        v8 = 1.0f;
    if ((float)(0.0 - v8) < 0.0)
        v4 = v8;
    else
        v4 = 0.0f;
    color[1] = (int)((float)(255.0 * v4) + 9.313225746154785e-10);
    if ((float)(colorVec[2] - 1.0) < 0.0)
        v7 = colorVec[2];
    else
        v7 = 1.0f;
    if ((float)(0.0 - v7) < 0.0)
        v3 = v7;
    else
        v3 = 0.0f;
    color[2] = (int)((float)(255.0 * v3) + 9.313225746154785e-10);
    if ((float)(colorVec[3] - 1.0) < 0.0)
        v6 = colorVec[3];
    else
        v6 = 1.0f;
    if ((float)(0.0 - v6) < 0.0)
        v2 = v6;
    else
        v2 = 0.0f;
    color[3] = (int)((float)(255.0 * v2) + 9.313225746154785e-10);
}

void __cdecl Dvar_GetUnpackedColorByName(const char *dvarName, float *expandedColor)
{
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    if ( dvar )
    {
        Dvar_GetUnpackedColor(dvar, expandedColor);
    }
    else
    {
        *expandedColor = colorWhite[0];
        expandedColor[1] = 1.0;
        expandedColor[2] = 1.0;
        expandedColor[3] = 1.0;
    }
}

void __cdecl Dvar_GetUnpackedColor(int dvarHash, float *expandedColor)
{
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = Dvar_FindVar(dvarHash);
    if ( dvar )
    {
        Dvar_GetUnpackedColor(dvar, expandedColor);
    }
    else
    {
        *expandedColor = colorWhite[0];
        expandedColor[1] = 1.0;
        expandedColor[2] = 1.0;
        expandedColor[3] = 1.0;
    }
}

double __cdecl Dvar_GetColorRed(const char *dvarName)
{
    float expandedColor[4]; // [esp+0h] [ebp-10h] BYREF

    Dvar_GetUnpackedColorByName(dvarName, expandedColor);
    return expandedColor[0];
}

double __cdecl Dvar_GetColorRed(int dvarHash)
{
    float expandedColor[4]; // [esp+0h] [ebp-10h] BYREF

    Dvar_GetUnpackedColor(dvarHash, expandedColor);
    return expandedColor[0];
}

double __cdecl Dvar_GetColorGreen(const char *dvarName)
{
    float expandedColor[4]; // [esp+0h] [ebp-10h] BYREF

    Dvar_GetUnpackedColorByName(dvarName, expandedColor);
    return expandedColor[1];
}

double __cdecl Dvar_GetColorGreen(int dvarHash)
{
    float expandedColor[4]; // [esp+0h] [ebp-10h] BYREF

    Dvar_GetUnpackedColor(dvarHash, expandedColor);
    return expandedColor[1];
}

double __cdecl Dvar_GetColorBlue(const char *dvarName)
{
    float expandedColor[4]; // [esp+0h] [ebp-10h] BYREF

    Dvar_GetUnpackedColorByName(dvarName, expandedColor);
    return expandedColor[2];
}

double __cdecl Dvar_GetColorBlue(int dvarHash)
{
    float expandedColor[4]; // [esp+0h] [ebp-10h] BYREF

    Dvar_GetUnpackedColor(dvarHash, expandedColor);
    return expandedColor[2];
}

double __cdecl Dvar_GetColorAlpha(const char *dvarName)
{
    float expandedColor[4]; // [esp+0h] [ebp-10h] BYREF

    Dvar_GetUnpackedColorByName(dvarName, expandedColor);
    return expandedColor[3];
}

double __cdecl Dvar_GetColorAlpha(int dvarHash)
{
    float expandedColor[4]; // [esp+0h] [ebp-10h] BYREF

    Dvar_GetUnpackedColor(dvarHash, expandedColor);
    return expandedColor[3];
}

void __cdecl Dvar_Shutdown()
{
    int dvarIter; // [esp+0h] [ebp-8h]
    dvar_s *dvar; // [esp+4h] [ebp-4h]

    Sys_LockWrite(&g_dvarCritSect);
    isDvarSystemActive = 0;
    for ( dvarIter = 0; dvarIter < dvarCount; ++dvarIter )
    {
        dvar = &dvarPool[dvarIter];
        if ( dvar->type == DVAR_TYPE_STRING )
        {
            if ( Dvar_ShouldFreeCurrentString(dvar) )
                Dvar_FreeString(&dvar->current);
            dvar->current.integer = 0;
            if ( Dvar_ShouldFreeResetString(dvar) )
                Dvar_FreeString(&dvar->reset);
            dvar->reset.integer = 0;
            if ( Dvar_ShouldFreeLatchedString(dvar) )
                Dvar_FreeString(&dvar->latched);
            dvar->latched.integer = 0;
        }
        if ( (dvar->flags & 0x4000) != 0 )
            Dvar_FreeNameString(dvar->name);
    }
    dvarCount = 0;
    dvar_cheats = 0;
    dvar_restoreDvarsOnLive = 0;
    dvar_modifiedFlags = 0;
    memset((unsigned __int8 *)dvarHashTable, 0, sizeof(dvarHashTable));
    Sys_UnlockWrite(&g_dvarCritSect);
}

void __cdecl Dvar_FreeNameString(const char *name)
{
    FreeString(name, 11, SCRIPTINSTANCE_SERVER);
}

bool __cdecl Dvar_ShouldFreeCurrentString(dvar_s *dvar)
{
    return dvar->current.integer
            && dvar->current.integer != dvar->latched.integer
            && dvar->current.integer != dvar->reset.integer;
}

bool __cdecl Dvar_ShouldFreeLatchedString(dvar_s *dvar)
{
    return dvar->latched.integer
            && dvar->latched.integer != dvar->current.integer
            && dvar->latched.integer != dvar->reset.integer;
}

bool __cdecl Dvar_ShouldFreeResetString(dvar_s *dvar)
{
    return dvar->reset.integer
            && dvar->reset.integer != dvar->current.integer
            && dvar->reset.integer != dvar->latched.integer;
}

void __cdecl Dvar_FreeString(DvarValue *value)
{
    FreeString(value->string, 11, SCRIPTINSTANCE_SERVER);
    value->integer = 0;
}

void __cdecl Dvar_ChangeResetValue(dvar_s *dvar, DvarValue value)
{
    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 1646, 0, "%s", "dvar") )
        __debugbreak();
    if ( !dvar->name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 1647, 0, "%s", "dvar->name") )
    {
        __debugbreak();
    }
    if ( (dvar->flags & 0x200) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    1648,
                    0,
                    "%s\n\t(dvar->name) = %s",
                    "(dvar->flags & (1 << 9))",
                    dvar->name) )
    {
        __debugbreak();
    }
    Dvar_UpdateResetValue(dvar, value);
}

void __cdecl Dvar_UpdateResetValue(dvar_s *dvar, DvarValue value)
{
    DvarValue oldString; // [esp+10h] [ebp-30h] BYREF
    bool shouldFree; // [esp+27h] [ebp-19h]
    DvarValue resetString; // [esp+28h] [ebp-18h] BYREF

    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 1604, 0, "%s", "dvar") )
        __debugbreak();
    switch ( dvar->type )
    {
        case DVAR_TYPE_FLOAT_2:
            dvar->reset.integer64 = value.integer64;
            break;
        case DVAR_TYPE_FLOAT_3:
        case DVAR_TYPE_LINEAR_COLOR_RGB:
        case DVAR_TYPE_COLOR_XYZ:
            dvar->reset.integer64 = value.integer64;
            dvar->reset.vector[2] = value.vector[2];
            break;
        case DVAR_TYPE_FLOAT_4:
            dvar->reset = value;
            break;
        case DVAR_TYPE_STRING:
            if ( dvar->reset.integer != value.integer )
            {
                shouldFree = Dvar_ShouldFreeResetString(dvar);
                if ( shouldFree )
                    oldString.integer = dvar->reset.integer;
                Dvar_AssignResetStringValue(dvar, &resetString, (char *)value.integer);
                dvar->reset.integer = resetString.integer;
                if ( shouldFree )
                    Dvar_FreeString(&oldString);
            }
            break;
        default:
            dvar->reset = value;
            break;
    }
}

void __cdecl Dvar_AssignResetStringValue(dvar_s *dvar, DvarValue *dest, char *string)
{
    if ( !string && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 226, 0, "%s", "string") )
        __debugbreak();
    if ( dvar->current.integer && (string == (char *)dvar->current.integer || !strcmp(string, dvar->current.string)) )
    {
        Dvar_WeakCopyString(dvar->current.string, dest);
    }
    else if ( dvar->latched.integer && (string == (char *)dvar->latched.integer || !strcmp(string, dvar->latched.string)) )
    {
        Dvar_WeakCopyString(dvar->latched.string, dest);
    }
    else
    {
        Dvar_CopyString(string, dest);
    }
}

void __cdecl Dvar_CopyString(char *string, DvarValue *value)
{
    if ( !string && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 163, 0, "%s", "string") )
        __debugbreak();
    value->integer = (int)CopyString(string, "Dvar_CopyString", 11, SCRIPTINSTANCE_SERVER);
}

void __cdecl Dvar_WeakCopyString(const char *string, DvarValue *value)
{
    if ( !string && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 171, 0, "%s", "string") )
        __debugbreak();
    value->integer = (int)string;
}

void __cdecl Dvar_MakeLatchedValueCurrent(dvar_s *dvar)
{
    Dvar_SetVariant(dvar, dvar->latched, DVAR_SOURCE_INTERNAL);
}

void __cdecl Dvar_SetVariant(dvar_s *dvar, DvarValue value, DvarSetSource source)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *name; // [esp-4h] [ebp-44Ch]
    bool shouldFreeString; // [esp+1Fh] [ebp-429h]
    char string[1024]; // [esp+20h] [ebp-428h] BYREF
    DvarValue oldString; // [esp+420h] [ebp-28h] BYREF
    DvarValue currentString; // [esp+430h] [ebp-18h] BYREF

    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 1005, 0, "%s", "dvar") )
        __debugbreak();
    if ( !dvar->name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 1006, 0, "%s", "dvar->name") )
    {
        __debugbreak();
    }
    if ( dvar && dvar->name && *dvar->name )
    {
        if ( Com_LogFileOpen() && !Dvar_ValuesEqual(dvar->type, dvar->current, value) )
        {
            v3 = Dvar_ValueToString(dvar, value);
            Com_sprintf(string, 0x400u, "            dvar set %s %s\n", dvar->name, v3);
            Com_PrintMessage(6, string, 0);
        }
        if ( !Dvar_ValueInDomain(dvar->type, value, dvar->domain) )
        {
            name = dvar->name;
            v4 = Dvar_ValueToString(dvar, value);
            Com_Printf(1, "'%s' is not a valid value for dvar '%s'\n", v4, name);
            Dvar_PrintDomain(dvar->type, dvar->domain);
            if ( dvar->type == DVAR_TYPE_ENUM )
            {
                if ( !Dvar_ValueInDomain(dvar->type, dvar->reset, dvar->domain)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                                1056,
                                0,
                                "%s\n\t(dvar->name) = %s",
                                "(Dvar_ValueInDomain( dvar->type, dvar->reset, dvar->domain ))",
                                dvar->name) )
                {
                    __debugbreak();
                }
                Dvar_SetVariant(dvar, dvar->reset, source);
            }
            return;
        }
        if ( source == DVAR_SOURCE_EXTERNAL || source == DVAR_SOURCE_SCRIPT )
        {
            if ( !Dvar_CanChangeValue(dvar, value, source) )
                return;
            if ( (dvar->flags & 0x20) != 0 )
            {
                Dvar_SetLatchedValue(dvar, value);
                if ( !Dvar_ValuesEqual(dvar->type, dvar->latched, dvar->current) )
                    Com_Printf(16, "%s will be changed upon restarting.\n", dvar->name);
                return;
            }
        }
        else if ( source == DVAR_SOURCE_DEVGUI && (dvar->flags & 0x800) != 0 )
        {
            Dvar_SetLatchedValue(dvar, value);
            return;
        }
        if ( Dvar_ValuesEqual(dvar->type, dvar->current, value) )
        {
            Dvar_SetLatchedValue(dvar, dvar->current);
        }
        else
        {
            dvar_modifiedFlags |= dvar->flags;
            switch ( dvar->type )
            {
                case DVAR_TYPE_FLOAT_2:
                    Vec2Copy(value.vector, dvar->current.vector);
                    Vec2Copy(value.vector, dvar->latched.vector);
                    break;
                case DVAR_TYPE_FLOAT_3:
                case DVAR_TYPE_LINEAR_COLOR_RGB:
                case DVAR_TYPE_COLOR_XYZ:
                    Vec3Copy(value.vector, dvar->current.vector);
                    Vec3Copy(value.vector, dvar->latched.vector);
                    break;
                case DVAR_TYPE_FLOAT_4:
                    Vec4Copy(value.vector, dvar->current.vector);
                    Vec4Copy(value.vector, dvar->latched.vector);
                    break;
                case DVAR_TYPE_STRING:
                    iassert(dvar->name);
                    iassert(value.string != dvar->current.string || value.string == dvar->latched.string || value.string == dvar->reset.string);
 
                    shouldFreeString = Dvar_ShouldFreeCurrentString(dvar);
                    if ( shouldFreeString )
                        oldString.integer = dvar->current.integer;
                    Dvar_AssignCurrentStringValue(dvar, &currentString, (char*)value.string);
                    dvar->current.string= currentString.string;
                    if ( Dvar_ShouldFreeLatchedString(dvar) )
                        Dvar_FreeString(&dvar->latched);
                    dvar->latched.string = NULL;
                    Dvar_WeakCopyString(dvar->current.string, &dvar->latched);
                    if ( shouldFreeString )
                        Dvar_FreeString(&oldString);
                    break;
                default:
                    dvar->current = value;
                    dvar->latched = value;
                    break;
            }
            dvar->modified = 1;
        }
    }
}

void __cdecl Dvar_AssignCurrentStringValue(dvar_s *dvar, DvarValue *dest, char *string)
{
    if ( !string && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 202, 0, "%s", "string") )
        __debugbreak();
    if ( dvar->latched.integer && (string == (char *)dvar->latched.integer || !strcmp(string, dvar->latched.string)) )
    {
        Dvar_WeakCopyString(dvar->latched.string, dest);
    }
    else if ( dvar->reset.integer && (string == (char *)dvar->reset.integer || !strcmp(string, dvar->reset.string)) )
    {
        Dvar_WeakCopyString(dvar->reset.string, dest);
    }
    else
    {
        Dvar_CopyString(string, dest);
    }
}

void __cdecl Dvar_SetLatchedValue(dvar_s *dvar, DvarValue value)
{
    DvarValue latchedString; // [esp+10h] [ebp-28h] BYREF
    DvarValue oldString; // [esp+20h] [ebp-18h] BYREF
    bool shouldFree; // [esp+33h] [ebp-5h]

    switch ( dvar->type )
    {
        case DVAR_TYPE_FLOAT_2:
            dvar->latched.integer64 = value.integer64;
            break;
        case DVAR_TYPE_FLOAT_3:
        case DVAR_TYPE_LINEAR_COLOR_RGB:
        case DVAR_TYPE_COLOR_XYZ:
            dvar->latched.integer64 = value.integer64;
            dvar->latched.vector[2] = value.vector[2];
            break;
        case DVAR_TYPE_FLOAT_4:
            dvar->latched = value;
            break;
        case DVAR_TYPE_STRING:
            if ( dvar->latched.integer != value.integer )
            {
                shouldFree = Dvar_ShouldFreeLatchedString(dvar);
                if ( shouldFree )
                    oldString.integer = dvar->latched.integer;
                Dvar_AssignLatchedStringValue(dvar, &latchedString, (char *)value.integer);
                dvar->latched.integer = latchedString.integer;
                if ( shouldFree )
                    Dvar_FreeString(&oldString);
            }
            break;
        default:
            dvar->latched = value;
            break;
    }
}

void __cdecl Dvar_AssignLatchedStringValue(dvar_s *dvar, DvarValue *dest, char *string)
{
    if ( !string && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 214, 0, "%s", "string") )
        __debugbreak();
    if ( dvar->current.integer && (string == (char *)dvar->current.integer || !strcmp(string, dvar->current.string)) )
    {
        Dvar_WeakCopyString(dvar->current.string, dest);
    }
    else if ( dvar->reset.integer && (string == (char *)dvar->reset.integer || !strcmp(string, dvar->reset.string)) )
    {
        Dvar_WeakCopyString(dvar->reset.string, dest);
    }
    else
    {
        Dvar_CopyString(string, dest);
    }
}

char __cdecl Dvar_CanChangeValue(const dvar_s *dvar, DvarValue value, DvarSetSource source)
{
    char *reason; // [esp+8h] [ebp-8h]

    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 943, 0, "%s", "dvar") )
        __debugbreak();
    if ( !dvar )
        return 0;
    if ( Dvar_ValuesEqual(dvar->type, value, dvar->reset) )
        return 1;
    reason = 0;
    if ( (dvar->flags & 0x40) != 0 )
    {
        reason = va("%s is read only.\n", dvar->name);
    }
    else if ( (dvar->flags & 0x10) != 0 )
    {
        reason = va("%s is write protected.\n", dvar->name);
    }
    else if ( (dvar->flags & 0x80) != 0
                 && !dvar_cheats->current.enabled
                 && (source == DVAR_SOURCE_EXTERNAL || source == DVAR_SOURCE_SCRIPT) )
    {
        reason = va("%s is cheat protected.\n", dvar->name);
    }
    if ( !reason )
        return 1;
    Com_Printf(1, reason);
    return 0;
}

void __cdecl Dvar_ClearLatchedValue(dvar_s *dvar)
{
    if ( Dvar_HasLatchedValue(dvar) )
        Dvar_SetLatchedValue(dvar, dvar->current);
}

const dvar_s *__cdecl _Dvar_RegisterBool(const char *dvarName, bool value, unsigned __int16 flags, const char *description)
{
    DvarLimits v5; // [esp-14h] [ebp-3Ch] BYREF
    DvarValue dvarValue; // [esp+10h] [ebp-18h]

    dvarValue.integer = value;
    memset(&v5, 0, sizeof(v5));
    return Dvar_RegisterVariant((char*)dvarName, DVAR_TYPE_BOOL, flags, dvarValue, v5, description);
}

const dvar_s *__cdecl Dvar_RegisterVariant(
                char *dvarName,
                dvarType_t type,
                unsigned __int16 flags,
                DvarValue value,
                DvarLimits domain,
                const char *description)
{
    dvar_s *dvar; // [esp+0h] [ebp-8h]

    if ( (flags & 0x4000) == 0
        && !CanKeepStringPointer(dvarName)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    1942,
                    0,
                    "%s\n\t(dvarName) = %s",
                    "((flags & (1 << 14)) || CanKeepStringPointer( dvarName ))",
                    dvarName) )
    {
        __debugbreak();
    }
    dvar = Dvar_FindMalleableVar(dvarName);
    if ( !dvar )
        return Dvar_RegisterNew(dvarName, type, flags, value, domain, description);
    Dvar_Reregister(dvar, dvarName, type, flags, value, domain, description);
    return dvar;
}

void __cdecl Dvar_Reregister(
                dvar_s *dvar,
                const char *dvarName,
                dvarType_t type,
                unsigned __int16 flags,
                DvarValue resetValue,
                DvarLimits domain,
                const char *description)
{
    char *v8; // eax
    char *v9; // eax
    const char *v10; // eax
    char *v11; // eax
    char *v12; // [esp-4h] [ebp-8h]

    if (!dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 1817, 0, "%s", "dvar"))
        __debugbreak();
    if (!dvarName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 1818, 0, "%s", "dvarName"))
    {
        __debugbreak();
    }
    if (dvar->type != type && (dvar->flags & 0x4000) == 0)
    {
        v8 = va("%s: %i != %i", dvarName, dvar->type, type);
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
            1819,
            0,
            "%s\n\t%s",
            "dvar->type == type || (dvar->flags & DVAR_EXTERNAL)",
            v8))
            __debugbreak();
    }
    if (((dvar->flags ^ flags) & 0x4000) != 0)
        Dvar_ReinterpretDvar(dvar, dvarName, type, flags, resetValue, domain);
    if ((dvar->flags & 0x4000) != 0 && dvar->type != type)
    {
        if (dvar->type != DVAR_TYPE_STRING)
        {
            v9 = va("dvar %s, type %i", dvar->name, dvar->type);
            if (!Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                1826,
                0,
                "%s\n\t%s",
                "dvar->type == DVAR_TYPE_STRING",
                v9))
                __debugbreak();
        }
        Dvar_MakeExplicitType(dvar, dvarName, type, flags, resetValue, domain);
    }
    if (dvar->type != type
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
            1830,
            0,
            "%s\n\t(dvarName) = %s",
            "(dvar->type == type)",
            dvarName))
    {
        __debugbreak();
    }
    if ((dvar->flags & 0x9200) == 0 && !Dvar_ValuesEqual(type, dvar->reset, resetValue))
    {
        v12 = (char*)Dvar_ValueToString(dvar, resetValue);
        v10 = Dvar_DisplayableResetValue(dvar);
        v11 = va("dvar %s, %s != %s", dvarName, v10, v12);
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
            1831,
            0,
            "%s\n\t%s",
            "(dvar->flags & (DVAR_CHANGEABLE_RESET|DVAR_SAVED|DVAR_AUTOEXEC)) || Dvar_ValuesEqual( type, dvar->reset, resetValue )",
            v11))
            __debugbreak();
    }
    dvar->flags |= flags;
    if (description)
        dvar->description = description;
    if ((dvar->flags & 0x80) != 0 && dvar_cheats && !dvar_cheats->current.enabled)
    {
        Dvar_SetVariant(dvar, dvar->reset, DVAR_SOURCE_INTERNAL);
        Dvar_SetLatchedValue(dvar, dvar->reset);
    }
    if ((dvar->flags & 0x20) != 0)
        Dvar_MakeLatchedValueCurrent(dvar);
}

void __cdecl Dvar_MakeExplicitType(
                dvar_s *dvar,
                const char *dvarName,
                dvarType_t type,
                unsigned __int16 flags,
                DvarValue resetValue,
                DvarLimits domain)
{
    bool v6; // [esp+0h] [ebp-64h]
    DvarValue v7; // [esp+4h] [ebp-60h] BYREF
    DvarValue v8; // [esp+14h] [ebp-50h]
    DvarValue result; // [esp+24h] [ebp-40h] BYREF
    DvarValue v10; // [esp+34h] [ebp-30h]
    bool wasString; // [esp+4Bh] [ebp-19h]
    DvarValue castValue; // [esp+4Ch] [ebp-18h]

    if (dvar->type != DVAR_TYPE_STRING
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
            1715,
            0,
            "%s\n\t(dvar->type) = %i",
            "(dvar->type == DVAR_TYPE_STRING)",
            dvar->type))
    {
        __debugbreak();
    }
    dvar->type = type;
    dvar->domain = domain;
    if ((flags & 0x40) != 0 || (flags & 0x80) != 0 && dvar_cheats && !dvar_cheats->current.enabled)
    {
        castValue = resetValue;
    }
    else
    {
        v10 = *Dvar_StringToValue(&result, (const dvarType_t)dvar->type, dvar->domain, dvar->current.string);
        castValue = v10;
        v8 = *Dvar_ClampValueToDomain(&v7, type, v10, resetValue, domain);
        castValue = v8;
    }
    v6 = dvar->type == DVAR_TYPE_STRING && castValue.integer;
    wasString = v6;
    if (v6)
        castValue.integer = (int)CopyString((char *)castValue.integer, "Dvar_MakeExplicitType", 11, SCRIPTINSTANCE_SERVER);
    if (dvar->type != DVAR_TYPE_STRING && Dvar_ShouldFreeCurrentString(dvar))
        Dvar_FreeString(&dvar->current);
    dvar->current.integer = 0;
    if (Dvar_ShouldFreeLatchedString(dvar))
        Dvar_FreeString(&dvar->latched);
    dvar->latched.integer = 0;
    if (Dvar_ShouldFreeResetString(dvar))
        Dvar_FreeString(&dvar->reset);
    dvar->reset.integer = 0;
    Dvar_UpdateResetValue(dvar, resetValue);
    Dvar_UpdateValue(dvar, castValue);
    dvar_modifiedFlags |= flags;
    if (wasString)
        FreeString(castValue.string, 11, SCRIPTINSTANCE_SERVER);
}

DvarValue *__cdecl Dvar_StringToValue(DvarValue *result, dvarType_t type, DvarLimits domain, const char *string)
{
    const char *v4; // eax
    DvarValue value; // [esp+4h] [ebp-18h] BYREF

    if ( !string && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 442, 0, "%s", "string") )
        __debugbreak();
    switch ( type )
    {
        case DVAR_TYPE_BOOL:
            value.enabled = Dvar_StringToBool(string);
            break;
        case DVAR_TYPE_FLOAT:
            value.value = Dvar_StringToFloat(string);
            break;
        case DVAR_TYPE_FLOAT_2:
            Dvar_StringToVec2(string, &value.value);
            break;
        case DVAR_TYPE_FLOAT_3:
        case DVAR_TYPE_LINEAR_COLOR_RGB:
        case DVAR_TYPE_COLOR_XYZ:
            Dvar_StringToVec3(string, &value.value);
            break;
        case DVAR_TYPE_FLOAT_4:
            Dvar_StringToVec4(string, &value.value);
            break;
        case DVAR_TYPE_INT:
            value.integer = Dvar_StringToInt(string);
            break;
        case DVAR_TYPE_ENUM:
            value.integer = Dvar_StringToEnum(&domain, string);
            break;
        case DVAR_TYPE_STRING:
            value.integer = (int)string;
            break;
        case DVAR_TYPE_COLOR:
            Dvar_StringToColor(string, (unsigned __int8 *)&value);
            break;
        case DVAR_TYPE_INT64:
            value.integer64 = Dvar_StringToInt64(string);
            break;
        default:
            v4 = va("unhandled dvar type '%i'", type);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 490, 1, v4) )
                __debugbreak();
            value.integer = 0;
            break;
    }
    *result = value;
    return result;
}

void __cdecl Dvar_StringToVec2(const char *string, float *vector)
{
    if ( !string && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 362, 0, "%s", "string") )
        __debugbreak();
    *vector = 0.0f;
    vector[1] = 0.0f;
    sscanf(string, "%g %g", vector, vector + 1);
}

void __cdecl Dvar_StringToVec3(const char *string, float *vector)
{
    if ( !string && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 371, 0, "%s", "string") )
        __debugbreak();
    *vector = 0.0f;
    vector[1] = 0.0f;
    vector[2] = 0.0f;
    if ( *string == 40 )
        sscanf(string, "( %g, %g, %g )", vector, vector + 1, vector + 2);
    else
        sscanf(string, "%g %g %g", vector, vector + 1, vector + 2);
}

void __cdecl Dvar_StringToVec4(const char *string, float *vector)
{
    if ( !string && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 383, 0, "%s", "string") )
        __debugbreak();
    *vector = 0.0f;
    vector[1] = 0.0f;
    vector[2] = 0.0f;
    vector[3] = 0.0f;
    sscanf(string, "%g %g %g %g", vector, vector + 1, vector + 2, vector + 3);
}

int __cdecl Dvar_StringToEnum(const DvarLimits *domain, const char *string)
{
    int v3; // kr00_4
    int stringIndex; // [esp+14h] [ebp-8h]
    int stringIndexa; // [esp+14h] [ebp-8h]
    int stringIndexb; // [esp+14h] [ebp-8h]
    const char *digit; // [esp+18h] [ebp-4h]

    if ( !domain && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 396, 0, "%s", "domain") )
        __debugbreak();
    if ( !string && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 397, 0, "%s", "string") )
        __debugbreak();
    for ( stringIndex = 0; stringIndex < domain->enumeration.stringCount; ++stringIndex )
    {
        if ( !I_stricmp(string, *(const char **)(domain->integer.max + 4 * stringIndex)) )
            return stringIndex;
    }
    stringIndexa = 0;
    for ( digit = string; *digit; ++digit )
    {
        if ( *digit < 48 || *digit > 57 )
            return -1337;
        stringIndexa = 10 * stringIndexa + *digit - 48;
    }
    if ( stringIndexa >= 0 && stringIndexa < domain->enumeration.stringCount )
        return stringIndexa;
    v3 = strlen(string);
    for ( stringIndexb = 0; stringIndexb < domain->enumeration.stringCount; ++stringIndexb )
    {
        if ( !I_strnicmp(string, *(const char **)(domain->integer.max + 4 * stringIndexb), v3) )
            return stringIndexb;
    }
    return -1337;
}

void __cdecl Dvar_UpdateValue(dvar_s *dvar, DvarValue value)
{
    DvarValue oldString; // [esp+1Ch] [ebp-30h] BYREF
    bool shouldFree; // [esp+33h] [ebp-19h]
    DvarValue currentString; // [esp+34h] [ebp-18h] BYREF

    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 1660, 0, "%s", "dvar") )
        __debugbreak();
    switch ( dvar->type )
    {
        case DVAR_TYPE_FLOAT_2:
            dvar->current.integer64 = value.integer64;
            dvar->latched.integer64 = value.integer64;
            break;
        case DVAR_TYPE_FLOAT_3:
        case DVAR_TYPE_LINEAR_COLOR_RGB:
        case DVAR_TYPE_COLOR_XYZ:
            dvar->current.integer = value.integer;
            *(__int64 *)((char *)&dvar->current.integer64 + 4) = *(__int64 *)((char *)&value.integer64 + 4);
            dvar->latched.integer64 = value.integer64;
            dvar->latched.vector[2] = value.vector[2];
            break;
        case DVAR_TYPE_FLOAT_4:
            dvar->current = value;
            dvar->latched = value;
            break;
        case DVAR_TYPE_STRING:
            if ( value.integer != dvar->current.integer )
            {
                shouldFree = Dvar_ShouldFreeCurrentString(dvar);
                if ( shouldFree )
                    oldString.integer = dvar->current.integer;
                Dvar_AssignCurrentStringValue(dvar, &currentString, (char *)value.integer);
                dvar->current.integer = currentString.integer;
                if ( Dvar_ShouldFreeLatchedString(dvar) )
                    Dvar_FreeString(&dvar->latched);
                dvar->latched.integer = 0;
                Dvar_WeakCopyString(dvar->current.string, &dvar->latched);
                if ( shouldFree )
                    Dvar_FreeString(&oldString);
            }
            break;
        default:
            dvar->current = value;
            dvar->latched = value;
            break;
    }
}

void __cdecl Dvar_ReinterpretDvar(
                dvar_s *dvar,
                const char *dvarName,
                dvarType_t type,
                unsigned __int16 flags,
                DvarValue value,
                DvarLimits domain)
{
    DvarValue result; // [esp+0h] [ebp-38h] BYREF
    DvarValue dvarValue; // [esp+10h] [ebp-28h]
    DvarValue resetValue; // [esp+20h] [ebp-18h]

    if ( (dvar->flags & 0x4000) != 0 && (flags & 0x4000) == 0 )
    {
        dvarValue = *Dvar_GetReinterpretedResetValue(&result, dvar, value);
        resetValue = dvarValue;
        Dvar_PerformUnregistration(dvar);
        Dvar_FreeNameString(dvar->name);
        dvar->name = dvarName;
        dvar->flags &= ~0x4000u;
        Dvar_MakeExplicitType(dvar, dvarName, type, flags, resetValue, domain);
    }
}

void __cdecl Dvar_PerformUnregistration(dvar_s *dvar)
{
    char *v1; // eax
    char *v2; // eax
    DvarValue resetString; // [esp+0h] [ebp-18h] BYREF

    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 1569, 0, "%s", "dvar") )
        __debugbreak();
    if ( (dvar->flags & 0x4000) == 0 )
    {
        dvar->flags |= 0x4000u;
        dvar->name = Dvar_AllocNameString((char *)dvar->name);
    }
    if ( dvar->type != DVAR_TYPE_STRING )
    {
        v1 = (char *)Dvar_DisplayableLatchedValue(dvar);
        Dvar_CopyString(v1, &dvar->current);
        if ( Dvar_ShouldFreeLatchedString(dvar) )
            Dvar_FreeString(&dvar->latched);
        dvar->latched.integer = 0;
        Dvar_WeakCopyString(dvar->current.string, &dvar->latched);
        if ( Dvar_ShouldFreeResetString(dvar) )
            Dvar_FreeString(&dvar->reset);
        dvar->reset.integer = 0;
        v2 = (char *)Dvar_DisplayableResetValue(dvar);
        Dvar_AssignResetStringValue(dvar, &resetString, v2);
        dvar->reset.integer = resetString.integer;
        dvar->type = DVAR_TYPE_STRING;
    }
}

char *__cdecl Dvar_AllocNameString(char *name)
{
    return CopyString(name, "Dvar_AllocNameString", 11, SCRIPTINSTANCE_SERVER);
}

DvarValue *__cdecl Dvar_GetReinterpretedResetValue(DvarValue *result, dvar_s *__formal, DvarValue value)
{
    *result = value;
    return result;
}

const dvar_s *__cdecl Dvar_RegisterNew(
                char *dvarName,
                dvarType_t type,
                unsigned __int16 flags,
                DvarValue value,
                DvarLimits domain,
                const char *description)
{
    dvar_s *var; // [esp+34h] [ebp-10h]
    int hash; // [esp+38h] [ebp-Ch]
    dvar_s *dvar; // [esp+3Ch] [ebp-8h]

    Sys_LockWrite(&g_dvarCritSect);
    if ( dvarCount >= 4096 )
    {
        Sys_UnlockWrite(&g_dvarCritSect);
        Com_Error(ERR_FATAL, "Can't create dvar '%s': %i dvars already exist", dvarName, 4096);
    }
    dvar = &dvarPool[dvarCount];
    sortedDvars[dvarCount] = dvar;
    areDvarsSorted = 0;
    ++dvarCount;
    dvar->type = type;
    if ( (flags & 0x4000) != 0 )
        dvar->name = Dvar_AllocNameString(dvarName);
    else
        dvar->name = dvarName;
    switch ( type )
    {
        case DVAR_TYPE_FLOAT_2:
            dvar->current.integer64 = value.integer64;
            dvar->latched.integer64 = value.integer64;
            dvar->reset.integer64 = value.integer64;
            break;
        case DVAR_TYPE_FLOAT_3:
        case DVAR_TYPE_LINEAR_COLOR_RGB:
        case DVAR_TYPE_COLOR_XYZ:
            dvar->current.integer = value.integer;
            *(__int64 *)((char *)&dvar->current.integer64 + 4) = *(__int64 *)((char *)&value.integer64 + 4);
            dvar->latched.integer = value.integer;
            *(__int64 *)((char *)&dvar->latched.integer64 + 4) = *(__int64 *)((char *)&value.integer64 + 4);
            dvar->reset.integer64 = value.integer64;
            dvar->reset.vector[2] = value.vector[2];
            break;
        case DVAR_TYPE_FLOAT_4:
            dvar->current = value;
            dvar->latched = value;
            dvar->reset = value;
            break;
        case DVAR_TYPE_STRING:
            Dvar_CopyString((char *)value.integer, &dvar->current);
            Dvar_WeakCopyString(dvar->current.string, &dvar->latched);
            Dvar_WeakCopyString(dvar->current.string, &dvar->reset);
            break;
        default:
            dvar->current = value;
            dvar->latched = value;
            dvar->reset = value;
            break;
    }
    dvar->domain = domain;
    dvar->modified = 0;
    dvar->flags = flags;
    dvar->description = description;
    hash = Com_HashString(dvarName, 0);
    dvar->hash = hash;
    for ( var = dvarHashTable[hash & 0x3FF]; var; var = var->hashNext )
    {
        if ( I_stricmp(dvarName, var->name) )
        {
            if ( hash == var->hash )
                Com_Error(
                    ERR_FATAL,
                    "dvar name hash collision between '%s' and '%s' Please change one of these names to remove the hash collision\n",
                    dvarName,
                    var->name);
        }
    }
    dvar->hashNext = dvarHashTable[hash & 0x3FF];
    dvarHashTable[hash & 0x3FF] = dvar;
    Sys_UnlockWrite(&g_dvarCritSect);
    return dvar;
}

const dvar_s *__cdecl _Dvar_RegisterInt(
                const char *dvarName,
                int value,
                int min,
                int max,
                unsigned __int16 flags,
                const char *description)
{
    DvarLimits dvarDomain; // [esp-14h] [ebp-3Ch]
    DvarValue dvarValue; // [esp+10h] [ebp-18h]

    dvarValue.integer = value;

    dvarDomain.integer.min = min;
    dvarDomain.integer.max = max;

    return Dvar_RegisterVariant((char*)dvarName, DVAR_TYPE_INT, flags, dvarValue, dvarDomain, description);
}

const dvar_s *__cdecl _Dvar_RegisterInt64(
                char *dvarName,
                __int64 value,
                __int64 min,
                __int64 max,
                unsigned __int16 flags,
                const char *description)
{
    DvarValue dvarValue; // [esp-24h] [ebp-4Ch]
    DvarLimits dvarDomain; // [esp-14h] [ebp-3Ch]

    dvarDomain.integer64.min = min;
    dvarDomain.integer64.max = max;

    dvarValue.integer64 = value;

    return Dvar_RegisterVariant(dvarName, DVAR_TYPE_INT64, flags, dvarValue, dvarDomain, description);
}

const dvar_s *__cdecl _Dvar_RegisterFloat(
                const char *dvarName,
                float value,
                float min,
                float max,
                unsigned __int16 flags,
                const char *description)
{
    DvarLimits dvarDomain; // [esp-14h] [ebp-3Ch]
    DvarValue dvarValue; // [esp+10h] [ebp-18h]

    dvarValue.value = value;

    dvarDomain.value.min = min;
    dvarDomain.value.max = max;

    return Dvar_RegisterVariant((char*)dvarName, DVAR_TYPE_FLOAT, flags, dvarValue, dvarDomain, description);
}

const dvar_s *__cdecl _Dvar_RegisterVec2(
                const char *dvarName,
                float x,
                float y,
                float min,
                float max,
                unsigned __int16 flags,
                const char *description)
{
    DvarValue dvarValue; // [esp-24h] [ebp-4Ch]
    DvarLimits dvarDomain; // [esp-14h] [ebp-3Ch]

    dvarValue.vector[0] = x;
    dvarValue.vector[1] = y;

    dvarDomain.vector.min = min;
    dvarDomain.vector.max = max;

    return Dvar_RegisterVariant((char*)dvarName, DVAR_TYPE_FLOAT_2, flags, dvarValue, dvarDomain, description);
}

const dvar_s *__cdecl _Dvar_RegisterVec3(
                const char *dvarName,
                float x,
                float y,
                float z,
                float min,
                float max,
                unsigned __int16 flags,
                const char *description)
{
    DvarValue dvarValue; // [esp-24h] [ebp-4Ch]
    DvarLimits dvarDomain; // [esp-14h] [ebp-3Ch]

    dvarValue.vector[0] = x;
    dvarValue.vector[1] = y;
    dvarValue.vector[2] = z;

    dvarDomain.vector.min = min;
    dvarDomain.vector.max = max;

    return Dvar_RegisterVariant((char*)dvarName, DVAR_TYPE_FLOAT_3, flags, dvarValue, dvarDomain, description);
}

const dvar_s *__cdecl _Dvar_RegisterVec4(
                const char *dvarName,
                float x,
                float y,
                float z,
                float w,
                float min,
                float max,
                unsigned __int16 flags,
                const char *description)
{
    DvarValue dvarValue; // [esp-24h] [ebp-4Ch]
    DvarLimits dvarDomain; // [esp-14h] [ebp-3Ch]

    dvarValue.vector[0] = x;
    dvarValue.vector[1] = y;
    dvarValue.vector[2] = z;
    dvarValue.vector[3] = w;

    dvarDomain.vector.min = min;
    dvarDomain.vector.max = max;

    return Dvar_RegisterVariant((char*)dvarName, DVAR_TYPE_FLOAT_4, flags, dvarValue, dvarDomain, description);
}

const dvar_s *__cdecl _Dvar_RegisterString(
                const char *dvarName,
                const char *value,
                unsigned __int16 flags,
                const char *description)
{
    DvarLimits v5; // [esp-14h] [ebp-3Ch] BYREF
    DvarValue dvarValue; // [esp+10h] [ebp-18h]

    if ( !dvarName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 2043, 0, "%s", "dvarName") )
    {
        __debugbreak();
    }
    if ( !value && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 2044, 0, "%s", "value") )
        __debugbreak();
    if ( (flags & 0x4000) == 0
        && !CanKeepStringPointer((char*)value)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    2045,
                    0,
                    "%s\n\t(dvarName) = %s",
                    "((flags & (1 << 14)) || CanKeepStringPointer( value ))",
                    dvarName) )
    {
        __debugbreak();
    }
    dvarValue.integer = (int)value;
    memset(&v5, 0, sizeof(v5));
    return Dvar_RegisterVariant((char*)dvarName, DVAR_TYPE_STRING, flags, dvarValue, v5, description);
}

const dvar_s *__cdecl _Dvar_RegisterEnum(
                const char *dvarName,
                const char **valueList,
                int defaultIndex,
                unsigned __int16 flags,
                const char *description)
{
    DvarLimits dvarDomain; // [esp+0h] [ebp-28h]
    DvarValue dvarValue; // [esp+10h] [ebp-18h]

    iassert(dvarName);
    iassert(valueList);


    dvarValue.integer = defaultIndex;

    dvarDomain.enumeration.strings = valueList;

    for (dvarDomain.enumeration.stringCount = 0;
                valueList[dvarDomain.enumeration.stringCount];
                ++dvarDomain.enumeration.stringCount )
    {
        ;
    }

    iassert((defaultIndex >= 0 && defaultIndex < dvarDomain.enumeration.stringCount || defaultIndex == 0));

    return Dvar_RegisterVariant((char*)dvarName, DVAR_TYPE_ENUM, flags, dvarValue, dvarDomain, description);
}

const dvar_s *__cdecl _Dvar_RegisterColor(
                const char *dvarName,
                float r,
                float g,
                float b,
                float a,
                unsigned __int16 flags,
                const char *description)
{
    DvarLimits domain;
    DvarValue dvarValue;

    // Clamp rgba values to [0.0, 1.0]
    r = min(1.0f, max(0.0f, r));
    g = min(1.0f, max(0.0f, g));
    b = min(1.0f, max(0.0f, b));
    a = min(1.0f, max(0.0f, a));

    // Convert to 0-255 range, +.5 for rounding
    dvarValue.color[0] = (int)(r * 255.0f + 0.5f);
    dvarValue.color[1] = (int)(g * 255.0f + 0.5f);
    dvarValue.color[2] = (int)(b * 255.0f + 0.5f);
    dvarValue.color[3] = (int)(a * 255.0f + 0.5f);

    memset(&domain, 0, sizeof(domain));
    return Dvar_RegisterVariant((char*)dvarName, DVAR_TYPE_COLOR, flags, dvarValue, domain, description);
}

const dvar_s *__cdecl _Dvar_RegisterLinearRGB(
                char *dvarName,
                unsigned int x,
                unsigned int y,
                unsigned int z,
                float min,
                float max,
                unsigned __int16 flags,
                const char *description)
{
    DvarValue dvarValue; // [esp-24h] [ebp-4Ch]
    DvarLimits dvarDomain; // [esp-14h] [ebp-3Ch]

    dvarValue.color[0] = x;
    dvarValue.color[1] = y;
    dvarValue.color[2] = z;

    dvarDomain.value.min = min;
    dvarDomain.value.max = max;

    return Dvar_RegisterVariant(dvarName, DVAR_TYPE_LINEAR_COLOR_RGB, flags, dvarValue, dvarDomain, description);
}

const dvar_s *__cdecl _Dvar_RegisterColorXYZ(
                const char *dvarName,
                unsigned int x,
                unsigned int y,
                unsigned int z,
                float min,
                float max,
                unsigned __int16 flags,
                const char *description)
{
    DvarValue dvarValue; // [esp-24h] [ebp-4Ch]
    DvarLimits dvarDomain; // [esp-14h] [ebp-3Ch]

    dvarValue.color[0] = x;
    dvarValue.color[1] = y;
    dvarValue.color[2] = z;

    dvarDomain.value.min = min;
    dvarDomain.value.max = max;

    return Dvar_RegisterVariant((char*)dvarName, DVAR_TYPE_COLOR_XYZ, flags, dvarValue, dvarDomain, description);
}

void __cdecl Dvar_SetDvarValueFromSource(dvar_s *dvar, DvarValue newValue, DvarSetSource source)
{
    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 2111, 0, "%s", "dvar") )
        __debugbreak();
    if ( !dvar->name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 2112, 0, "%s", "dvar->name") )
    {
        __debugbreak();
    }
    if ( dvar )
    {
        if ( dvar->name )
            Dvar_SetVariant(dvar, newValue, source);
    }
}

void __cdecl Dvar_SetBoolFromSource(dvar_s *dvar, bool value, DvarSetSource source)
{
    const char *v3; // [esp+0h] [ebp-1Ch]
    DvarValue newValue; // [esp+4h] [ebp-18h]

    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 2126, 0, "%s", "dvar") )
        __debugbreak();
    if ( !dvar->name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 2127, 0, "%s", "dvar->name") )
    {
        __debugbreak();
    }
    if ( dvar->type
        && (dvar->type != DVAR_TYPE_STRING || (dvar->flags & 0x4000) == 0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    2128,
                    0,
                    "%s\n\t(dvar->name) = %s",
                    "(dvar->type == DVAR_TYPE_BOOL || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))))",
                    dvar->name) )
    {
        __debugbreak();
    }
    if ( dvar && dvar->name )
    {
        if ( dvar->type )
        {
            if ( value )
                v3 = "1";
            else
                v3 = "0";
            newValue.integer = (int)v3;
        }
        else
        {
            newValue.enabled = value;
        }
        Dvar_SetVariant(dvar, newValue, source);
    }
}

void __cdecl Dvar_SetIntFromSource(dvar_s *dvar, int value, DvarSetSource source)
{
    char string[32]; // [esp+0h] [ebp-38h] BYREF
    DvarValue newValue; // [esp+20h] [ebp-18h]

    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 2149, 0, "%s", "dvar") )
        __debugbreak();
    if ( !dvar->name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 2150, 0, "%s", "dvar->name") )
    {
        __debugbreak();
    }
    if ( dvar->type != DVAR_TYPE_INT
        && dvar->type != DVAR_TYPE_ENUM
        && (dvar->type != DVAR_TYPE_STRING || (dvar->flags & 0x4000) == 0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    2151,
                    0,
                    "%s\n\t(dvar->name) = %s",
                    "(dvar->type == DVAR_TYPE_INT || dvar->type == DVAR_TYPE_ENUM || (dvar->type == DVAR_TYPE_STRING && (dvar->flag"
                    "s & (1 << 14))))",
                    dvar->name) )
    {
        __debugbreak();
    }
    if ( dvar && dvar->name )
    {
        if ( dvar->type == DVAR_TYPE_INT || dvar->type == DVAR_TYPE_ENUM )
        {
            newValue.integer = value;
        }
        else
        {
            Com_sprintf(string, 0x20u, "%i", value);
            newValue.integer = (int)string;
        }
        Dvar_SetVariant(dvar, newValue, source);
    }
}

void __cdecl Dvar_SetInt64FromSource(dvar_s *dvar, __int64 value, DvarSetSource source)
{
    char string[32]; // [esp+0h] [ebp-38h] BYREF
    DvarValue newValue; // [esp+20h] [ebp-18h]

    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 2176, 0, "%s", "dvar") )
        __debugbreak();
    if ( !dvar->name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 2177, 0, "%s", "dvar->name") )
    {
        __debugbreak();
    }
    if ( dvar->type != DVAR_TYPE_INT64
        && (dvar->type != DVAR_TYPE_STRING || (dvar->flags & 0x4000) == 0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    2178,
                    0,
                    "%s\n\t(dvar->name) = %s",
                    "(dvar->type == DVAR_TYPE_INT64 || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))))",
                    dvar->name) )
    {
        __debugbreak();
    }
    if ( dvar && dvar->name )
    {
        if ( dvar->type == DVAR_TYPE_INT64 )
        {
            newValue.integer64 = value;
        }
        else
        {
            Com_sprintf(string, 0x20u, "%lli", value);
            newValue.integer = (int)string;
        }
        Dvar_SetVariant(dvar, newValue, source);
    }
}

void __cdecl Dvar_SetFloatFromSource(dvar_s *dvar, float value, DvarSetSource source)
{
    char string[32]; // [esp+8h] [ebp-38h] BYREF
    DvarValue newValue; // [esp+28h] [ebp-18h]

    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 2205, 0, "%s", "dvar") )
        __debugbreak();
    if ( !dvar->name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 2206, 0, "%s", "dvar->name") )
    {
        __debugbreak();
    }
    if ( dvar->type != DVAR_TYPE_FLOAT
        && (dvar->type != DVAR_TYPE_STRING || (dvar->flags & 0x4000) == 0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    2207,
                    0,
                    "%s\n\t(dvar->name) = %s",
                    "(dvar->type == DVAR_TYPE_FLOAT || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))))",
                    dvar->name) )
    {
        __debugbreak();
    }
    if ( dvar && dvar->name )
    {
        if ( dvar->type == DVAR_TYPE_FLOAT )
        {
            newValue.value = value;
        }
        else
        {
            Com_sprintf(string, 0x20u, "%g", value);
            newValue.integer = (int)string;
        }
        Dvar_SetVariant(dvar, newValue, source);
    }
}

void __cdecl Dvar_SetVec2FromSource(dvar_s *dvar, float x, float y, DvarSetSource source)
{
    char string[64]; // [esp+10h] [ebp-58h] BYREF
    DvarValue newValue; // [esp+50h] [ebp-18h]

    iassert(dvar);
    iassert(dvar->name);
    iassert(dvar->type == DVAR_TYPE_FLOAT_2 || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))));

    if ( dvar && dvar->name )
    {
        if ( dvar->type == DVAR_TYPE_FLOAT_2 )
        {
            newValue.vector[0] = x;
            newValue.vector[1] = y;
        }
        else
        {
            Com_sprintf(string, 0x40u, "%g %g", x, y);
            newValue.string = string;
        }
        Dvar_SetVariant(dvar, newValue, source);
    }
}

void __cdecl Dvar_SetVec3FromSource(dvar_s *dvar, float x, float y, float z, DvarSetSource source)
{
    char string[96]; // [esp+18h] [ebp-78h] BYREF
    DvarValue newValue; // [esp+78h] [ebp-18h]

    iassert(dvar);
    iassert(dvar->name);
    iassert(dvar->type == DVAR_TYPE_FLOAT_3 || dvar->type == DVAR_TYPE_COLOR_XYZ || dvar->type == DVAR_TYPE_LINEAR_COLOR_RGB || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))));

    if ( dvar && dvar->name )
    {
        if ( dvar->type == DVAR_TYPE_FLOAT_3
            || dvar->type == DVAR_TYPE_LINEAR_COLOR_RGB
            || dvar->type == DVAR_TYPE_COLOR_XYZ )
        {
            newValue.vector[0] = x;
            newValue.vector[1] = y;
            newValue.vector[2] = z;
        }
        else
        {
            Com_sprintf(string, 0x60u, "%g %g %g", x, y, z);
            newValue.string = string;
        }
        Dvar_SetVariant(dvar, newValue, source);
    }
}

void __cdecl Dvar_SetVec4FromSource(
                dvar_s *dvar,
                float x,
                float y,
                float z,
                float w,
                DvarSetSource source)
{
    char string[128]; // [esp+20h] [ebp-98h] BYREF
    DvarValue newValue; // [esp+A0h] [ebp-18h]

    iassert(dvar);
    iassert(dvar->name);
    iassert((dvar->type == DVAR_TYPE_FLOAT_4 || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14)))));

    if ( dvar && dvar->name )
    {
        if ( dvar->type == DVAR_TYPE_FLOAT_4 )
        {
            Vec4Set(newValue.vector, x, y, z, w);
        }
        else
        {
            Com_sprintf(string, sizeof(string), "%g %g %g %g", x, y, z, w);
            newValue.string = string;
        }
        Dvar_SetVariant(dvar, newValue, source);
    }
}

void __cdecl Dvar_SetColorFromSource(dvar_s *dvar, float r, float g, float b, float a, DvarSetSource source)
{
    float v6; // [esp+20h] [ebp-108h]
    float v7; // [esp+24h] [ebp-104h]
    float v8; // [esp+28h] [ebp-100h]
    float v9; // [esp+2Ch] [ebp-FCh]
    float v10; // [esp+44h] [ebp-E4h]
    float v11; // [esp+5Ch] [ebp-CCh]
    float v12; // [esp+74h] [ebp-B4h]
    float v13; // [esp+8Ch] [ebp-9Ch]
    char string[128]; // [esp+90h] [ebp-98h] BYREF
    DvarValue newValue; // [esp+110h] [ebp-18h]

    if (!dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 2347, 0, "%s", "dvar"))
        __debugbreak();
    if (!dvar->name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 2348, 0, "%s", "dvar->name"))
    {
        __debugbreak();
    }
    if (dvar->type != DVAR_TYPE_COLOR
        && (dvar->type != DVAR_TYPE_STRING || (dvar->flags & 0x4000) == 0)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
            2349,
            0,
            "%s\n\t(dvar->name) = %s",
            "(dvar->type == DVAR_TYPE_COLOR || (dvar->type == DVAR_TYPE_STRING && (dvar->flags & (1 << 14))))",
            dvar->name))
    {
        __debugbreak();
    }
    if (dvar && dvar->name)
    {
        if (dvar->type == DVAR_TYPE_COLOR)
        {
            if ((float)(r - 1.0) < 0.0)
                v13 = r;
            else
                v13 = 1.0f;
            if ((float)(0.0 - v13) < 0.0)
                v9 = v13;
            else
                v9 = 0.0f;
            newValue.enabled = (int)((float)(255.0 * v9) + 9.313225746154785e-10);
            if ((float)(g - 1.0) < 0.0)
                v12 = g;
            else
                v12 = 1.0f;
            if ((float)(0.0 - v12) < 0.0)
                v8 = v12;
            else
                v8 = 0.0f;
            newValue.color[1] = (int)((float)(255.0 * v8) + 9.313225746154785e-10);
            if ((float)(b - 1.0) < 0.0)
                v11 = b;
            else
                v11 = 1.0f;
            if ((float)(0.0 - v11) < 0.0)
                v7 = v11;
            else
                v7 = 0.0f;
            newValue.color[2] = (int)((float)(255.0 * v7) + 9.313225746154785e-10);
            if ((float)(a - 1.0) < 0.0)
                v10 = a;
            else
                v10 = 1.0f;
            if ((float)(0.0 - v10) < 0.0)
                v6 = v10;
            else
                v6 = 0.0f;
            newValue.color[3] = (int)((float)(255.0 * v6) + 9.313225746154785e-10);
        }
        else
        {
            Com_sprintf(string, 0x80u, "%g %g %g %g", r, g, b, a);
            newValue.integer = (int)string;
        }
        Dvar_SetVariant(dvar, newValue, source);
    }
}

void __cdecl Dvar_SetBool(dvar_s *dvar, bool value)
{
    Dvar_SetBoolFromSource(dvar, value, DVAR_SOURCE_INTERNAL);
}

void __cdecl Dvar_SetInt(dvar_s *dvar, int value)
{
    Dvar_SetIntFromSource(dvar, value, DVAR_SOURCE_INTERNAL);
}

void __cdecl Dvar_SetInt64(dvar_s *dvar, __int64 value)
{
    Dvar_SetInt64FromSource(dvar, value, DVAR_SOURCE_INTERNAL);
}

void __cdecl Dvar_SetFloat(dvar_s *dvar, float value)
{
    Dvar_SetFloatFromSource(dvar, value, DVAR_SOURCE_INTERNAL);
}

void __cdecl Dvar_SetVec2(dvar_s *dvar, float x, float y)
{
    Dvar_SetVec2FromSource(dvar, x, y, DVAR_SOURCE_INTERNAL);
}

void __cdecl Dvar_SetVec3(dvar_s *dvar, float x, float y, float z)
{
    Dvar_SetVec3FromSource(dvar, x, y, z, DVAR_SOURCE_INTERNAL);
}

void __cdecl Dvar_SetVec4(dvar_s *dvar, float x, float y, float z, float w)
{
    Dvar_SetVec4FromSource(dvar, x, y, z, w, DVAR_SOURCE_INTERNAL);
}

void __cdecl Dvar_SetString(dvar_s *dvar, const char *value)
{
    Dvar_SetStringFromSource(dvar, value, DVAR_SOURCE_INTERNAL);
}

void __cdecl Dvar_SetStringFromSource(dvar_s *dvar, const char *string, DvarSetSource source)
{
    const char *v3; // eax
    char stringCopy[1024]; // [esp+0h] [ebp-418h] BYREF
    DvarValue newValue; // [esp+400h] [ebp-18h]

    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 2317, 0, "%s", "dvar") )
        __debugbreak();
    if ( !dvar->name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 2318, 0, "%s", "dvar->name") )
    {
        __debugbreak();
    }
    if ( dvar->type != DVAR_TYPE_STRING
        && dvar->type != DVAR_TYPE_ENUM
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    2319,
                    0,
                    "%s\n\t(dvar->name) = %s",
                    "(dvar->type == DVAR_TYPE_STRING || dvar->type == DVAR_TYPE_ENUM)",
                    dvar->name) )
    {
        __debugbreak();
    }
    if ( !string && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 2320, 0, "%s", "string") )
        __debugbreak();
    if ( dvar && dvar->name )
    {
        if ( dvar->type == DVAR_TYPE_STRING )
        {
            I_strncpyz(stringCopy, string, 1024);
            newValue.integer = (int)stringCopy;
        }
        else
        {
            newValue.integer = Dvar_StringToEnum(&dvar->domain, string);
            if ( newValue.integer == -1337 )
            {
                v3 = va("%s doesn't include %s", dvar->name, string);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                                2335,
                                0,
                                "%s\n\t%s",
                                "newValue.integer != DVAR_INVALID_ENUM_INDEX",
                                v3) )
                    __debugbreak();
            }
        }
        Dvar_SetVariant(dvar, newValue, source);
    }
}

void __cdecl Dvar_SetColor(dvar_s *dvar, float r, float g, float b, float a)
{
    Dvar_SetColorFromSource(dvar, r, g, b, a, DVAR_SOURCE_INTERNAL);
}

void __cdecl Dvar_SetFromString(dvar_s *dvar, const char *string)
{
    Dvar_SetFromStringFromSource(dvar, string, DVAR_SOURCE_INTERNAL);
}

void __cdecl Dvar_SetFromStringFromSource(dvar_s *dvar, const char *string, DvarSetSource source)
{
    DvarValue result; // [esp+0h] [ebp-438h] BYREF
    DvarValue v4; // [esp+10h] [ebp-428h]
    char buf[1024]; // [esp+20h] [ebp-418h] BYREF
    DvarValue newValue; // [esp+420h] [ebp-18h]

    if ( dvar && dvar->name )
    {
        I_strncpyz(buf, string, 1024);
        v4 = *Dvar_StringToValue(&result, dvar->type, dvar->domain, buf);
        newValue = v4;
        if ( dvar->type == DVAR_TYPE_ENUM && newValue.integer == -1337 )
        {
            Com_Printf(16, "'%s' is not a valid value for dvar '%s'\n", buf, dvar->name);
            Dvar_PrintDomain(dvar->type, dvar->domain);
            newValue = dvar->reset;
        }
        Dvar_SetVariant(dvar, newValue, source);
    }
}

void __cdecl Dvar_SetBoolByName(const char *dvarName, bool value)
{
    dvar_s *dvar; // [esp+4h] [ebp-4h]

    dvar = (dvar_s *)Dvar_FindVar(dvarName);
    if ( dvar )
    {
        Dvar_SetBool(dvar, value);
    }
    else if ( value )
    {
        _Dvar_RegisterString(dvarName, "1", 0x4000u, "External Dvar");
    }
    else
    {
        _Dvar_RegisterString(dvarName, "0", 0x4000u, "External Dvar");
    }
}

void __cdecl Dvar_SetIntByName(const char *dvarName, int value)
{
    char string[32]; // [esp+0h] [ebp-28h] BYREF
    const dvar_s *dvar; // [esp+24h] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    if ( dvar )
    {
        Dvar_SetInt((dvar_s *)dvar, value);
    }
    else
    {
        Com_sprintf(string, 0x20u, "%i", value);
        _Dvar_RegisterString(dvarName, string, 0x4000u, "External Dvar");
    }
}

void __cdecl Dvar_SetInt64ByName(const char *dvarName, __int64 value)
{
    char string[32]; // [esp+0h] [ebp-28h] BYREF
    const dvar_s *dvar; // [esp+24h] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    if ( dvar )
    {
        Dvar_SetInt64((dvar_s *)dvar, value);
    }
    else
    {
        Com_sprintf(string, 0x20u, "%lli", value);
        _Dvar_RegisterString(dvarName, string, 0x4000u, "External Dvar");
    }
}

void __cdecl Dvar_SetFloatByName(const char *dvarName, float value)
{
    char string[32]; // [esp+8h] [ebp-28h] BYREF
    const dvar_s *dvar; // [esp+2Ch] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    if ( dvar )
    {
        Dvar_SetFloat((dvar_s *)dvar, value);
    }
    else
    {
        Com_sprintf(string, 0x20u, "%g", value);
        _Dvar_RegisterString(dvarName, string, 0x4000u, "External Dvar");
    }
}

void __cdecl Dvar_SetVec2ByName(const char *dvarName, float x, float y)
{
    char string[64]; // [esp+10h] [ebp-48h] BYREF
    const dvar_s *dvar; // [esp+54h] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    if ( dvar )
    {
        Dvar_SetVec2((dvar_s *)dvar, x, y);
    }
    else
    {
        Com_sprintf(string, 0x40u, "%g %g", x, y);
        _Dvar_RegisterString(dvarName, string, 0x4000u, "External Dvar");
    }
}

void __cdecl Dvar_SetVec3ByName(const char *dvarName, float x, float y, float z)
{
    char string[128]; // [esp+18h] [ebp-88h] BYREF
    const dvar_s *dvar; // [esp+9Ch] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    if ( dvar )
    {
        Dvar_SetVec3((dvar_s *)dvar, x, y, z);
    }
    else
    {
        Com_sprintf(string, 0x80u, "%g %g %g", x, y, z);
        _Dvar_RegisterString(dvarName, string, 0x4000u, "External Dvar");
    }
}

void __cdecl Dvar_SetVec4ByName(const char *dvarName, float x, float y, float z, float w)
{
    char string[128]; // [esp+20h] [ebp-88h] BYREF
    const dvar_s *dvar; // [esp+A4h] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    if ( dvar )
    {
        Dvar_SetVec4((dvar_s *)dvar, x, y, z, w);
    }
    else
    {
        Com_sprintf(string, 0x80u, "%g %g %g %g", x, y, z, w);
        _Dvar_RegisterString(dvarName, string, 0x4000u, "External Dvar");
    }
}

void __cdecl Dvar_SetStringByName(const char *dvarName, char *value)
{
    dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = (dvar_s *)Dvar_FindVar(dvarName);
    if ( dvar )
        Dvar_SetString(dvar, value);
    else
        _Dvar_RegisterString(dvarName, value, 0x4000u, "External Dvar");
}

void __cdecl Dvar_SetColorByName(const char *dvarName, float r, float g, float b, float a)
{
    char string[128]; // [esp+20h] [ebp-88h] BYREF
    const dvar_s *dvar; // [esp+A4h] [ebp-4h]

    dvar = Dvar_FindVar(dvarName);
    if ( dvar )
    {
        Dvar_SetColor((dvar_s *)dvar, r, g, b, a);
    }
    else
    {
        Com_sprintf(string, 0x80u, "%g %g %g %g", r, g, b, a);
        _Dvar_RegisterString(dvarName, string, 0x4000u, "External Dvar");
    }
}

const dvar_s *__cdecl Dvar_SetFromStringByNameFromSource(
                const char *dvarName,
                char *string,
                DvarSetSource source,
                unsigned __int16 flags)
{
    dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = (dvar_s *)Dvar_FindVar(dvarName);
    if ( !dvar )
        return _Dvar_RegisterString(dvarName, string, flags | 0x4000, "External Dvar");
    Dvar_SetFromStringFromSource(dvar, string, source);
    return dvar;
}

void __cdecl Dvar_SetFromStringByName(const char *dvarName, char *string)
{
    Dvar_SetFromStringByNameFromSource(dvarName, string, DVAR_SOURCE_INTERNAL, 0);
}

void __cdecl Dvar_SetCommand(const char *dvarName, char *string)
{
    dvar_s *dvar; // [esp+0h] [ebp-4h]

    dvar = (dvar_s *)Dvar_SetFromStringByNameFromSource(dvarName, string, DVAR_SOURCE_EXTERNAL, 0);
    if ( dvar )
    {
        if ( isLoadingAutoExecGlobalFlag )
        {
            Dvar_AddFlags(dvar, 0x8000);
            Dvar_UpdateResetValue(dvar, dvar->current);
        }
    }
}

void __cdecl Dvar_AddFlags(const dvar_s *dvar, int flags)
{
    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 2652, 0, "%s", "dvar") )
        __debugbreak();
    if ( (flags & 0x40F0) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    2653,
                    0,
                    "%s\n\t(flags) = %i",
                    "((flags & ((1 << 7) | (1 << 4) | (1 << 6) | (1 << 14) | (1 << 5))) == 0)",
                    flags) )
    {
        __debugbreak();
    }
    ((dvar_s*)(dvar))->flags |= flags;
}

void __cdecl Dvar_Reset(dvar_s *dvar, DvarSetSource setSource)
{
    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 2660, 0, "%s", "dvar") )
        __debugbreak();
    Dvar_SetVariant(dvar, dvar->reset, setSource);
}

void __cdecl Dvar_SetCheatState()
{
    int dvarIter; // [esp+0h] [ebp-8h]
    dvar_s *dvar; // [esp+4h] [ebp-4h]

    _InterlockedExchangeAdd(&g_dvarCritSect.readCount, 1u);
    while ( g_dvarCritSect.writeCount )
        NET_Sleep(0);
    for ( dvarIter = 0; dvarIter < dvarCount; ++dvarIter )
    {
        dvar = &dvarPool[dvarIter];
        if ( (dvar->flags & 0x80) != 0 )
            Dvar_SetVariant(dvar, dvar->reset, DVAR_SOURCE_INTERNAL);
    }
    Sys_UnlockRead(&g_dvarCritSect);
}

void __cdecl Dvar_Init()
{
    isDvarSystemActive = 1;
    dvar_cheats = _Dvar_RegisterBool("sv_cheats", 1, 0x18u, "External Dvar");
    dvar_restoreDvarsOnLive = _Dvar_RegisterBool(
                                                            "sv_restoreDvars",
                                                            1,
                                                            0,
                                                            "Enable to restore Dvars on entering the Xbox Live menu");
    Dvar_AddCommands();
}

void __cdecl Dvar_ResetScriptInfo()
{
    int dvarIter; // [esp+0h] [ebp-4h]

    _InterlockedExchangeAdd(&g_dvarCritSect.readCount, 1u);
    while ( g_dvarCritSect.writeCount )
        NET_Sleep(0);
    for ( dvarIter = 0; dvarIter < dvarCount; ++dvarIter )
        dvarPool[dvarIter].flags &= ~0x400u;
    Sys_UnlockRead(&g_dvarCritSect);
}

char __cdecl Dvar_AnyLatchedValues()
{
    int dvarIter; // [esp+0h] [ebp-4h]

    _InterlockedExchangeAdd(&g_dvarCritSect.readCount, 1u);
    while ( g_dvarCritSect.writeCount )
        NET_Sleep(0);
    for ( dvarIter = 0; dvarIter < dvarCount; ++dvarIter )
    {
        if ( Dvar_HasLatchedValue(&dvarPool[dvarIter]) )
        {
            Sys_UnlockRead(&g_dvarCritSect);
            return 1;
        }
    }
    Sys_UnlockRead(&g_dvarCritSect);
    return 0;
}

void __cdecl Dvar_RestoreDvars()
{
    dvar_s *dvar; // [esp+0h] [ebp-8h]
    unsigned int hash; // [esp+4h] [ebp-4h]

    if ( dvar_restoreDvarsOnLive && dvar_restoreDvarsOnLive->current.enabled )
    {
        _InterlockedExchangeAdd(&g_dvarCritSect.readCount, 1u);
        while ( g_dvarCritSect.writeCount )
            NET_Sleep(0);
        for ( hash = 0; hash < 0x400; ++hash )
        {
            for ( dvar = dvarHashTable[hash]; dvar; dvar = dvar->hashNext )
            {
                if ( dvar->loadedFromSaveGame )
                {
                    dvar->loadedFromSaveGame = 0;
                    Dvar_SetVariant(dvar, dvar->saved, DVAR_SOURCE_INTERNAL);
                    if ( dvar->type == DVAR_TYPE_STRING )
                        Dvar_FreeString(&dvar->saved);
                }
            }
        }
        Sys_UnlockRead(&g_dvarCritSect);
    }
}

void __cdecl Dvar_ResetDvars(unsigned int filter, DvarSetSource setSource)
{
    int dvarIter; // [esp+0h] [ebp-8h]

    _InterlockedExchangeAdd(&g_dvarCritSect.readCount, 1u);
    while ( g_dvarCritSect.writeCount )
        NET_Sleep(0);
    for ( dvarIter = 0; dvarIter < dvarCount; ++dvarIter )
    {
        if ( (filter & dvarPool[dvarIter].flags) != 0 )
            Dvar_Reset(&dvarPool[dvarIter], setSource);
    }
    Sys_UnlockRead(&g_dvarCritSect);
}

void __cdecl Dvar_ForEachConsoleAccessName(void (__cdecl *callback)(const char *))
{
    int dvarIter; // [esp+0h] [ebp-4h]

    if ( Dvar_GetBool("con_restricted_access") )
    {
        Dvar_Sort();
        for ( dvarIter = 0; dvarIter < dvarCount; ++dvarIter )
        {
            if ( (sortedDvars[dvarIter]->flags & 0x10000) != 0 )
                callback(sortedDvars[dvarIter]->name);
        }
    }
    else
    {
        Dvar_ForEachName(callback);
    }
}

void __cdecl Dvar_ResetConAccessDvars()
{
    int dvarIter; // [esp+0h] [ebp-8h]

    _InterlockedExchangeAdd(&g_dvarCritSect.readCount, 1u);
    while ( g_dvarCritSect.writeCount )
        NET_Sleep(0);
    for ( dvarIter = 0; dvarIter < dvarCount; ++dvarIter )
        dvarPool[dvarIter].flags &= ~0x10000u;
    Sys_UnlockRead(&g_dvarCritSect);
}

int __cdecl Com_SaveDvarsToBuffer(const char **dvarnames, unsigned int numDvars, char *buffer, unsigned int bufsize)
{
    const char *string; // [esp+0h] [ebp-14h]
    int ret; // [esp+4h] [ebp-10h]
    int written; // [esp+8h] [ebp-Ch]
    unsigned int i; // [esp+Ch] [ebp-8h]
    const dvar_s *dvar; // [esp+10h] [ebp-4h]

    ret = 1;
    for ( i = 0; i < numDvars; ++i )
    {
        dvar = Dvar_FindVar(dvarnames[i]);
        if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 3023, 0, "%s", "dvar") )
            __debugbreak();
        string = Dvar_DisplayableValue(dvar);
        written = _snprintf(buffer, bufsize, "%s \"%s\"\n", dvar->name, string);
        if ( written < 0 )
            return 0;
        buffer += written;
        bufsize -= written;
    }
    return ret;
}

int __cdecl Com_LoadDvarsFromBuffer(const char **dvarnames, unsigned int numDvars, char *buffer, char *filename)
{
    return Com_LoadDvarsFromBufferOptional(dvarnames, 0, numDvars, buffer, filename);
}

int __cdecl Com_LoadDvarsFromBufferOptional(
                const char **dvarnames,
                bool *isOptional,
                unsigned int numDvars,
                char *buffer,
                char *filename)
{
    const char *v5; // eax
    unsigned __int8 dst[16384]; // [esp+0h] [ebp-4018h] BYREF
    unsigned int i; // [esp+4004h] [ebp-14h]
    char *s0; // [esp+4008h] [ebp-10h]
    char v10; // [esp+400Fh] [ebp-9h]
    dvar_s *dvar; // [esp+4010h] [ebp-8h]
    int v12; // [esp+4014h] [ebp-4h]

    if ( numDvars >= 0x4000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp",
                    3061,
                    0,
                    "%s",
                    "numDvars < ARRAY_COUNT( wasRead )") )
    {
        __debugbreak();
    }
    memset(dst, 0, numDvars);
    v12 = 0;
    for ( i = 0; i < numDvars; ++i )
    {
        dvar = (dvar_s *)Dvar_FindVar(dvarnames[i]);
        if ( !dvar )
        {
            v5 = va("Unable to find dvar '%s'", dvarnames[i]);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 3068, 0, "%s\n\t%s", "dvar", v5) )
                __debugbreak();
        }
        Dvar_Reset(dvar, DVAR_SOURCE_INTERNAL);
    }
    Com_BeginParseSession(filename);
    while ( 1 )
    {
        s0 = (char *)Com_Parse((const char **)&buffer);
        if ( !*s0 )
            break;
        for ( i = 0; ; ++i )
        {
            if ( i >= numDvars )
            {
                Com_PrintWarning(16, "WARNING: unknown dvar '%s' in file '%s'\n", s0, filename);
                goto next_dvar;
            }
            if ( !I_stricmp(s0, dvarnames[i]) )
                break;
        }
        dvar = (dvar_s *)Dvar_FindVar(dvarnames[i]);
        if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\dvar.cpp", 3084, 0, "%s", "dvar") )
            __debugbreak();
        s0 = (char *)Com_ParseOnLine((const char **)&buffer);
        Dvar_SetFromString(dvar, s0);
        if ( !dst[i] )
        {
            dst[i] = 1;
            ++v12;
        }
next_dvar:
        Com_SkipRestOfLine((const char **)&buffer);
    }
    Com_EndParseSession();
    v10 = 0;
    for ( i = 0; i < numDvars; ++i )
    {
        if ( !dst[i] && isOptional && !isOptional[i] )
        {
            v10 = 1;
            break;
        }
    }
    if ( !v10 )
        return 1;
    Com_PrintError(16, "ERROR: the following dvars were not specified in file '%s'\n", filename);
    for ( i = 0; i < numDvars; ++i )
    {
        if ( !dst[i] && isOptional && !isOptional[i] )
            Com_PrintError(16, "    %s\n", dvarnames[i]);
    }
    return 0;
}

void __cdecl Dvar_SetBoolIfChanged(dvar_s *dvar, bool value)
{
    if ( dvar )
    {
        if ( dvar->current.enabled != value )
            Dvar_SetBool(dvar, value);
    }
}

void __cdecl Dvar_SetIntIfChanged(dvar_s *dvar, int value)
{
    if ( dvar )
    {
        if ( dvar->current.integer != value )
            Dvar_SetInt(dvar, value);
    }
}

void __cdecl Dvar_SetFloatIfChanged(dvar_s *dvar, float value)
{
    if ( dvar )
    {
        if ( dvar->current.value != value )
            Dvar_SetFloat(dvar, value);
    }
}

