#include "ui_localvars.h"
#include <universal/com_memory.h>

void __cdecl UILocalVar_Init(UILocalVarContext *context)
{
    if ( !context && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_localvars.cpp", 19, 0, "%s", "context") )
        __debugbreak();
}

void __cdecl UILocalVar_Shutdown(UILocalVarContext *context)
{
    unsigned int hash; // [esp+4h] [ebp-4h]

    if ( !context && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_localvars.cpp", 27, 0, "%s", "context") )
        __debugbreak();
    for ( hash = 0; hash < 0x100; ++hash )
    {
        if ( context->table[hash].name )
        {
            if ( context->table[hash].type == UILOCALVAR_STRING )
                FreeString(context->table[hash].u.string, 36, SCRIPTINSTANCE_SERVER);
            FreeString(context->table[hash].name, 36, SCRIPTINSTANCE_SERVER);
        }
    }
    memset(context, 0, sizeof(UILocalVarContext));
}

UILocalVarContext *__cdecl UILocalVar_Find(UILocalVarContext *context, const char *name)
{
    unsigned int hash; // [esp+0h] [ebp-4h] BYREF

    if ( UILocalVar_FindLocation(context, name, &hash) )
        return (UILocalVarContext *)((char *)context + 12 * hash);
    else
        return 0;
}

char __cdecl UILocalVar_FindLocation(UILocalVarContext *context, const char *name, unsigned int *hashForName)
{
    unsigned int hash; // [esp+1Ch] [ebp-8h]
    unsigned int initialHash; // [esp+20h] [ebp-4h]

    initialHash = UILocalVar_HashName(name);
    hash = initialHash;
    do
    {
        if ( !context->table[hash].name )
            break;
        if ( !strcmp(context->table[hash].name, name) )
        {
            *hashForName = hash;
            return 1;
        }
        hash = (unsigned __int8)(hash + 1);
    }
    while ( hash != initialHash );
    *hashForName = hash;
    return 0;
}

unsigned int __cdecl UILocalVar_HashName(const char *name)
{
    __int16 hash; // [esp+0h] [ebp-8h]
    unsigned int i; // [esp+4h] [ebp-4h]

    hash = 0;
    for ( i = 0; name[i]; ++i )
        hash += (i + 119) * name[i];
    return (unsigned __int8)(hash + HIBYTE(hash));
}

UILocalVarContext *__cdecl UILocalVar_FindOrCreate(UILocalVarContext *context, char *name)
{
    UILocalVar *var; // [esp+0h] [ebp-8h]
    unsigned int hash; // [esp+4h] [ebp-4h] BYREF

    if ( UILocalVar_FindLocation(context, name, &hash) )
        return (UILocalVarContext *)((char *)context + 12 * hash);
    var = &context->table[hash];
    var->name = CopyString(name, "UILocalVar_FindOrCreate", 36, SCRIPTINSTANCE_SERVER);
    var->type = UILOCALVAR_INT;
    var->u.integer = 0;
    return (UILocalVarContext *)var;
}

bool __cdecl UILocalVar_GetBool(const UILocalVar *var)
{
    if ( !var && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_localvars.cpp", 110, 0, "%s", "var") )
        __debugbreak();
    if ( var->type == UILOCALVAR_INT )
        return var->u.integer != 0;
    if ( var->type == UILOCALVAR_FLOAT )
        return var->u.value != 0.0;
    if ( var->type != UILOCALVAR_STRING
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_localvars.cpp",
                    121,
                    0,
                    "var->type == UILOCALVAR_STRING\n\t%i, %i",
                    var->type,
                    2) )
    {
        __debugbreak();
    }
    return atoi(var->u.string) != 0;
}

int __cdecl UILocalVar_GetInt(const UILocalVar *var)
{
    if ( !var && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_localvars.cpp", 129, 0, "%s", "var") )
        __debugbreak();
    if ( var->type )
    {
        if ( var->type == UILOCALVAR_FLOAT )
        {
            return (int)var->u.value;
        }
        else
        {
            if ( var->type != UILOCALVAR_STRING
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_localvars.cpp",
                            140,
                            0,
                            "var->type == UILOCALVAR_STRING\n\t%i, %i",
                            var->type,
                            2) )
            {
                __debugbreak();
            }
            return atoi(var->u.string);
        }
    }
    else
    {
        return var->u.integer;
    }
}

double __cdecl UILocalVar_GetFloat(const UILocalVar *var)
{
    if ( !var && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_localvars.cpp", 148, 0, "%s", "var") )
        __debugbreak();
    if ( var->type == UILOCALVAR_INT )
        return (double)var->u.integer;
    if ( var->type == UILOCALVAR_FLOAT )
        return var->u.value;
    if ( var->type != UILOCALVAR_STRING
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_localvars.cpp",
                    159,
                    0,
                    "var->type == UILOCALVAR_STRING\n\t%i, %i",
                    var->type,
                    2) )
    {
        __debugbreak();
    }
    return (float)atof(var->u.string);
}

char *__cdecl UILocalVar_GetString(const UILocalVar *var, char *stringBuf, unsigned int size)
{
    if ( !var && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_localvars.cpp", 167, 0, "%s", "var") )
        __debugbreak();
    if ( var->type )
    {
        if ( var->type == UILOCALVAR_FLOAT )
        {
            Com_sprintf(stringBuf, size, "%g", var->u.value);
            return stringBuf;
        }
        else
        {
            if ( var->type != UILOCALVAR_STRING
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_localvars.cpp",
                            180,
                            0,
                            "var->type == UILOCALVAR_STRING\n\t%i, %i",
                            var->type,
                            2) )
            {
                __debugbreak();
            }
            return (char *)var->u.integer;
        }
    }
    else
    {
        Com_sprintf(stringBuf, size, "%i", var->u.integer);
        return stringBuf;
    }
}

void __cdecl UILocalVar_SetBool(UILocalVar *var, bool b)
{
    if ( var->type == UILOCALVAR_STRING )
        FreeString(var->u.string, 36, SCRIPTINSTANCE_SERVER);
    var->type = UILOCALVAR_INT;
    var->u.integer = b;
}

void __cdecl UILocalVar_SetInt(UILocalVar *var, int i)
{
    if ( var->type == UILOCALVAR_STRING )
        FreeString(var->u.string, 36, SCRIPTINSTANCE_SERVER);
    var->type = UILOCALVAR_INT;
    var->u.integer = i;
}

void __cdecl UILocalVar_SetFloat(UILocalVar *var, float f)
{
    if ( var->type == UILOCALVAR_STRING )
        FreeString(var->u.string, 36, SCRIPTINSTANCE_SERVER);
    var->type = UILOCALVAR_FLOAT;
    var->u.value = f;
}

void __cdecl UILocalVar_SetString(UILocalVar *var, char *s)
{
    if ( var->type == UILOCALVAR_STRING )
        FreeString(var->u.string, 36, SCRIPTINSTANCE_SERVER);
    var->type = UILOCALVAR_STRING;
    var->u.integer = (int)CopyString(s, "UILocalVar_SetString", 36, SCRIPTINSTANCE_SERVER);
}

