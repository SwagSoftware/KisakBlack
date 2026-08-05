#include "l_precomp.h"
#include <qcommon/common.h>
#include "l_memory.h"
#include <time.h>

struct directive_s // sizeof=0x8
{                                       // XREF: .rdata:directives/r
    const char *name;                         // XREF: PC_ReadDirective(source_s *)+92/r
    int (__cdecl *func)(source_s *);    // XREF: PC_ReadDirective(source_s *)+141/r
};

const directive_s directives[15] =
{
  { "if", &PC_Directive_if },
  { "ifdef", &PC_Directive_ifdef },
  { "ifndef", &PC_Directive_ifndef },
  { "elif", &PC_Directive_elif },
  { "else", &PC_Directive_else },
  { "endif", &PC_Directive_endif },
  { "include", &PC_Directive_include },
  { "define", &PC_Directive_define },
  { "undef", &PC_Directive_undef },
  { "line", &PC_Directive_line },
  { "error", &PC_Directive_error },
  { "pragma", &PC_Directive_pragma },
  { "eval", &PC_Directive_eval },
  { "evalfloat", &PC_Directive_evalfloat },
  { NULL, NULL }
};

directive_s dollardirectives[20] =
{
  { "evalint", &PC_DollarDirective_evalint },
  { "evalfloat", &PC_DollarDirective_evalfloat },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL }
};

source_s *sourceFiles[64];

void __cdecl PrintSourceStack(const script_s *scriptstack)
{
    script_s *scriptstacka; // [esp+8h] [ebp+8h]

    for ( scriptstacka = scriptstack->next; scriptstacka; scriptstacka = scriptstacka->next )
        Com_PrintWarning(24, "    From file %s, line %d\n", scriptstacka->filename, scriptstacka->line);
}

void SourceError(source_s *source, const char *str, ...)
{
    char text[1028]; // [esp+4h] [ebp-408h] BYREF
    va_list va; // [esp+41Ch] [ebp+10h] BYREF

    va_start(va, str);
    _vsnprintf(text, 0x400u, str, va);
    Com_PrintError(24, "Error: file %s, line %d: %s\n", source->scriptstack->filename, source->scriptstack->line, text);
    PrintSourceStack(source->scriptstack);
}

void SourceWarning(source_s *source, const char *str, ...)
{
    char text[1028]; // [esp+4h] [ebp-408h] BYREF
    va_list va; // [esp+41Ch] [ebp+10h] BYREF

    va_start(va, str);
    _vsnprintf(text, 0x400u, str, va);
    Com_PrintWarning(
        24,
        "Warning: file %s, line %d: %s\n",
        source->scriptstack->filename,
        source->scriptstack->line,
        text);
    PrintSourceStack(source->scriptstack);
}

void __cdecl PC_PushScript(source_s *source, script_s *script)
{
    script_s *s; // [esp+0h] [ebp-4h]

    for ( s = source->scriptstack; s; s = s->next )
    {
        if ( !I_stricmp(s->filename, script->filename) )
        {
            SourceError(source, (char*)"%s recursively included", script->filename);
            return;
        }
    }
    script->next = source->scriptstack;
    source->scriptstack = script;
}

int numtokens;
token_s *__cdecl PC_CopyToken(token_s *token)
{
    unsigned int *t; // [esp+8h] [ebp-4h]

    t = GetMemory(0x430u);
    if ( t )
    {
        memcpy(t, token, 0x430u);
        t[266] = 0;
        ++numtokens;
        return (token_s *)t;
    }
    else
    {
        Com_Error(ERR_FATAL, "EXE_ERR_OUT_OF_MEMORY");
        return 0;
    }
}

void __cdecl PC_FreeToken(token_s *token)
{
    FreeMemory(token->string);
    --numtokens;
}

int __cdecl PC_ReadSourceToken(source_s *source, token_s *token)
{
    token_s *t; // [esp+8h] [ebp-10h]
    script_s *script; // [esp+Ch] [ebp-Ch]
    parseSkip_t skip; // [esp+10h] [ebp-8h] BYREF
    int type; // [esp+14h] [ebp-4h] BYREF

    while ( !source->tokens )
    {
        if ( PS_ReadToken(source->scriptstack, token) )
            return 1;
        if ( EndOfScript(source->scriptstack) )
        {
            while ( source->indentstack && source->indentstack->script == source->scriptstack )
            {
                SourceWarning(source, (char*)"missing #endif");
                PC_PopIndent(source, &type, &skip);
            }
        }
        if ( !source->scriptstack->next )
            return 0;
        script = source->scriptstack;
        source->scriptstack = script->next;
        FreeScript(script);
    }
    memcpy(token, source->tokens, sizeof(token_s));
    t = source->tokens;
    source->tokens = t->next;
    PC_FreeToken(t);
    return 1;
}

void __cdecl PC_PopIndent(source_s *source, int *type, parseSkip_t *skip)
{
    indent_s *indent; // [esp+0h] [ebp-4h]

    *type = 0;
    *skip = SKIP_NO;
    indent = source->indentstack;
    if ( indent )
    {
        if ( source->indentstack->script == source->scriptstack )
        {
            *type = indent->type;
            *skip = indent->skip;
            source->indentstack = source->indentstack->next;
            source->skip -= indent->skip != SKIP_NO;
            FreeMemory((char *)indent);
        }
    }
}

int __cdecl PC_UnreadSourceToken(source_s *source, token_s *token)
{
    token_s *v2; // eax

    v2 = PC_CopyToken(token);
    v2->next = source->tokens;
    source->tokens = v2;
    return 1;
}

int __cdecl PC_ReadDefineParms(source_s *source, define_s *define, token_s **parms, int maxparms)
{
    token_s *t; // [esp+50h] [ebp-454h]
    int done; // [esp+54h] [ebp-450h]
    int lastcomma; // [esp+58h] [ebp-44Ch]
    int indent; // [esp+5Ch] [ebp-448h]
    int numparms; // [esp+60h] [ebp-444h]
    token_s token; // [esp+64h] [ebp-440h] BYREF
    int i; // [esp+49Ch] [ebp-8h]
    token_s *last; // [esp+4A0h] [ebp-4h]

    if ( !PC_ReadSourceToken(source, &token) )
    {
        SourceError(source, "define %s missing parms", define->name);
        return 0;
    }
    if ( define->numparms > maxparms )
    {
        SourceError(source, "define with more than %d parameters", maxparms);
        return 0;
    }
    for ( i = 0; i < define->numparms; ++i )
        parms[i] = 0;
    if ( strcmp(token.string, "(") )
    {
        PC_UnreadSourceToken(source, &token);
        SourceError(source, "define %s missing parms", define->name);
        return 0;
    }
    done = 0;
    numparms = 0;
    indent = 0;
LABEL_11:
    if ( !done )
    {
        if ( numparms >= maxparms )
        {
            SourceError(source, "define %s with too many parms", define->name);
            return 0;
        }
        if ( numparms >= define->numparms )
        {
            SourceWarning(source, "define %s has too many parms", define->name);
            return 0;
        }
        parms[numparms] = 0;
        lastcomma = 1;
        last = 0;
        while ( 1 )
        {
            if ( !PC_ReadSourceToken(source, &token) )
            {
                SourceError(source, "define %s incomplete", define->name);
                return 0;
            }
            if ( !strcmp(token.string, ",") && !indent )
            {
                if ( lastcomma )
                    SourceWarning(source, "too many comma's");
                goto LABEL_38;
            }
            lastcomma = 0;
            if ( !strcmp(token.string, "(") )
            {
                ++indent;
            }
            else if ( !strcmp(token.string, ")") )
            {
                if ( !indent )
                {
                    if ( !parms[define->numparms - 1] )
                        SourceWarning(source, "too few define parms to %s", define->name);
                    done = 1;
LABEL_38:
                    ++numparms;
                    goto LABEL_11;
                }
                --indent;
            }
            if ( numparms < define->numparms )
            {
                t = PC_CopyToken(&token);
                t->next = 0;
                if ( last )
                    last->next = t;
                else
                    parms[numparms] = t;
                last = t;
            }
        }
    }
    return 1;
}

int __cdecl PC_StringizeTokens(token_s *tokens, token_s *token)
{
    token->type = 1;
    token->whitespace_p = 0;
    token->endwhitespace_p = 0;
    token->string[0] = 0;
    strcat(token->string, "\"");
    while ( tokens )
    {
        strncat(token->string, tokens->string, 1024 - strlen(token->string));
        tokens = tokens->next;
    }
    strncat(token->string, "\"", 1024 - strlen(token->string));
    return 1;
}

int __cdecl PC_MergeTokens(token_s *t1, token_s *t2)
{
    if ( t1->type == 4 && (t2->type == 4 || t2->type == 3) )
    {
        strcat(t1->string, t2->string);
        return 1;
    }
    else if ( t1->type == 1 && t2->type == 1 )
    {
        t1->string[strlen(t1->string) - 1] = 0;
        strcat(t1->string, &t2->string[1]);
        return 1;
    }
    else
    {
        return 0;
    }
}

int __cdecl PC_NameHash(char *name)
{
    int hash; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    hash = 0;
    for ( i = 0; name[i]; ++i )
        hash += (i + 119) * name[i];
    return ((unsigned __int16)(hash >> 20) ^ (unsigned __int16)(hash ^ (hash >> 10))) & 0x3FF;
}

void __cdecl PC_AddDefineToHash(define_s *define, define_s **definehash)
{
    int hash; // [esp+0h] [ebp-4h]

    hash = PC_NameHash(define->name);
    define->hashnext = definehash[hash];
    definehash[hash] = define;
}

define_s *__cdecl PC_FindHashedDefine(define_s **definehash, char *name)
{
    define_s *d; // [esp+14h] [ebp-8h]

    for ( d = definehash[PC_NameHash(name)]; d; d = d->hashnext )
    {
        if ( !strcmp(d->name, name) )
            return d;
    }
    return 0;
}

int __cdecl PC_FindDefineParm(define_s *define, const char *name)
{
    int i; // [esp+14h] [ebp-8h]
    token_s *p; // [esp+18h] [ebp-4h]

    i = 0;
    for ( p = define->parms; p; p = p->next )
    {
        if ( !strcmp(p->string, name) )
            return i;
        ++i;
    }
    return -1;
}

void __cdecl PC_FreeDefine(define_s *define)
{
    token_s *t; // [esp+0h] [ebp-8h]
    token_s *ta; // [esp+0h] [ebp-8h]
    token_s *next; // [esp+4h] [ebp-4h]
    token_s *nexta; // [esp+4h] [ebp-4h]

    for ( t = define->parms; t; t = next )
    {
        next = t->next;
        PC_FreeToken(t);
    }
    for ( ta = define->tokens; ta; ta = nexta )
    {
        nexta = ta->next;
        PC_FreeToken(ta);
    }
    FreeMemory((char *)define);
}

int __cdecl PC_ExpandBuiltinDefine(
                source_s *source,
                token_s *deftoken,
                define_s *define,
                token_s **firsttoken,
                token_s **lasttoken)
{
    char v6; // [esp+47h] [ebp-21h]
    token_s *v7; // [esp+4Ch] [ebp-1Ch]
    script_s *scriptstack; // [esp+50h] [ebp-18h]
    __int64 t; // [esp+58h] [ebp-10h] BYREF
    char *curtime; // [esp+60h] [ebp-8h]
    token_s *token; // [esp+64h] [ebp-4h]

    token = PC_CopyToken(deftoken);
    switch ( define->builtin )
    {
        case 1:
            sprintf(token->string, "%d", deftoken->line);
            token->intvalue = deftoken->line;
            token->floatvalue = (double)deftoken->line;
            token->type = 3;
            token->subtype = 4104;
            *firsttoken = token;
            *lasttoken = token;
            return 1;
        case 2:
            scriptstack = source->scriptstack;
            v7 = token;
            do
            {
                v6 = scriptstack->filename[0];
                v7->string[0] = scriptstack->filename[0];
                scriptstack = (script_s *)((char *)scriptstack + 1);
                v7 = (token_s *)((char *)v7 + 1);
            }
            while ( v6 );
            token->type = 4;
            goto LABEL_8;
        case 3:
            t = _time64(0);
            curtime = _ctime64(&t);
            strcpy(token->string, "\"");
            strncat(token->string, curtime + 4, 7u);
            strncat(&token->string[7], curtime + 20, 4u);
            strcat(token->string, "\"");
            free(curtime);
            token->type = 4;
            goto LABEL_8;
        case 4:
            t = _time64(0);
            curtime = _ctime64(&t);
            strcpy(token->string, "\"");
            strncat(token->string, curtime + 11, 8u);
            strcat(token->string, "\"");
            free(curtime);
            token->type = 4;
LABEL_8:
            token->subtype = strlen(token->string);
            *firsttoken = token;
            *lasttoken = token;
            break;
        default:
            *firsttoken = 0;
            *lasttoken = 0;
            break;
    }
    return 1;
}

int __cdecl PC_ExpandDefine(
                source_s *source,
                token_s *deftoken,
                define_s *define,
                token_s **firsttoken,
                token_s **lasttoken)
{
    int parmnum; // [esp+0h] [ebp-65Ch]
    int parmnuma; // [esp+0h] [ebp-65Ch]
    token_s *dt; // [esp+4h] [ebp-658h]
    token_s *t; // [esp+8h] [ebp-654h]
    token_s *ta; // [esp+8h] [ebp-654h]
    token_s *tb; // [esp+8h] [ebp-654h]
    token_s *nextpt; // [esp+10h] [ebp-64Ch]
    token_s *parms[128]; // [esp+14h] [ebp-648h] BYREF
    token_s *t2; // [esp+214h] [ebp-448h]
    token_s *pt; // [esp+218h] [ebp-444h]
    token_s token; // [esp+21Ch] [ebp-440h] BYREF
    int i; // [esp+650h] [ebp-Ch]
    token_s *last; // [esp+654h] [ebp-8h]
    token_s *first; // [esp+658h] [ebp-4h]

    if ( define->builtin )
        return PC_ExpandBuiltinDefine(source, deftoken, define, firsttoken, lasttoken);
    if ( define->numparms && !PC_ReadDefineParms(source, define, parms, 128) )
        return 0;
    first = 0;
    last = 0;
    for ( dt = define->tokens; dt; dt = dt->next )
    {
        parmnum = -1;
        if ( dt->type == 4 )
            parmnum = PC_FindDefineParm(define, dt->string);
        if ( parmnum < 0 )
        {
            if ( dt->string[0] != 35 || dt->string[1] )
            {
                ta = PC_CopyToken(dt);
            }
            else
            {
                if ( dt->next )
                    parmnuma = PC_FindDefineParm(define, dt->next->string);
                else
                    parmnuma = -1;
                if ( parmnuma < 0 )
                {
                    SourceWarning(source, "stringizing operator without define parameter");
                    continue;
                }
                dt = dt->next;
                if ( !PC_StringizeTokens(parms[parmnuma], &token) )
                {
                    SourceError(source, "can't stringize tokens");
                    return 0;
                }
                ta = PC_CopyToken(&token);
            }
            ta->next = 0;
            if ( last )
                last->next = ta;
            else
                first = ta;
            last = ta;
        }
        else
        {
            for ( pt = parms[parmnum]; pt; pt = pt->next )
            {
                t = PC_CopyToken(pt);
                t->next = 0;
                if ( last )
                    last->next = t;
                else
                    first = t;
                last = t;
            }
        }
    }
    tb = first;
    while ( tb )
    {
        if ( tb->next && tb->next->string[0] == 35 && tb->next->string[1] == 35 && (t2 = tb->next->next) != 0 )
        {
            if ( !PC_MergeTokens(tb, t2) )
            {
                SourceError(source, "can't merge %s with %s", tb->string, t2->string);
                return 0;
            }
            PC_FreeToken(tb->next);
            tb->next = t2->next;
            if ( t2 == last )
                last = tb;
            PC_FreeToken(t2);
        }
        else
        {
            tb = tb->next;
        }
    }
    *firsttoken = first;
    *lasttoken = last;
    for ( i = 0; i < define->numparms; ++i )
    {
        for ( pt = parms[i]; pt; pt = nextpt )
        {
            nextpt = pt->next;
            PC_FreeToken(pt);
        }
    }
    return 1;
}

void __cdecl PC_ConvertPath(char *path)
{
    char v1; // dl
    char *v2; // [esp+8h] [ebp-Ch]
    char *v3; // [esp+Ch] [ebp-8h]
    char *ptr; // [esp+10h] [ebp-4h]
    char *ptra; // [esp+10h] [ebp-4h]

    ptr = path;
    while ( *ptr )
    {
        if ( (*ptr == 92 || *ptr == 47) && (ptr[1] == 92 || ptr[1] == 47) )
        {
            v3 = ptr + 1;
            v2 = ptr;
            do
            {
                v1 = *v3;
                *v2++ = *v3++;
            }
            while ( v1 );
        }
        else
        {
            ++ptr;
        }
    }
    for ( ptra = path; *ptra; ++ptra )
    {
        if ( *ptra == 47 || *ptra == 92 )
            *ptra = 92;
    }
}

int __cdecl PC_Directive_include(source_s *source)
{
    char v2; // [esp+1Bh] [ebp-4B9h]
    char *v3; // [esp+20h] [ebp-4B4h]
    char *v4; // [esp+24h] [ebp-4B0h]
    char v5; // [esp+2Bh] [ebp-4A9h]
    token_s *v6; // [esp+2Ch] [ebp-4A8h]
    char *v7; // [esp+40h] [ebp-494h]
    char v8; // [esp+47h] [ebp-48Dh]
    char *v9; // [esp+4Ch] [ebp-488h]
    char *includepath; // [esp+50h] [ebp-484h] BYREF
    char path[68]; // [esp+54h] [ebp-480h] BYREF
    script_s *script; // [esp+98h] [ebp-43Ch]
    token_s token; // [esp+9Ch] [ebp-438h] BYREF

    if ( source->skip > 0 )
        return 1;
    if ( !PC_ReadSourceToken(source, &token) || token.linescrossed > 0 )
        goto LABEL_4;
    if ( token.type == 1 )
    {
        StripDoubleQuotes(token.string);
        PC_ConvertPath(token.string);
        script = LoadScriptFile(token.string);
        if ( !script )
        {
            includepath = source->includepath;
            v9 = path;
            do
            {
                v8 = *includepath;
                *v9++ = *includepath++;
            }
            while ( v8 );
            v7 = &token.string[strlen(token.string) + 1];
            v6 = (token_s *)((char *)&includepath + 3);
            do
            {
                v5 = v6->string[1];
                v6 = (token_s *)((char *)v6 + 1);
            }
            while ( v5 );
            memcpy(v6, &token, v7 - (char *)&token);
            script = LoadScriptFile(path);
        }
        goto LABEL_30;
    }
    if ( token.type != 5 || token.string[0] != 60 )
    {
LABEL_4:
        SourceError(source, "#include without file name");
        return 0;
    }
    v4 = source->includepath;
    v3 = path;
    do
    {
        v2 = *v4;
        *v3++ = *v4++;
    }
    while ( v2 );
    while ( PC_ReadSourceToken(source, &token) )
    {
        if ( token.linescrossed > 0 )
        {
            PC_UnreadSourceToken(source, &token);
            break;
        }
        if ( token.type == 5 && token.string[0] == 62 )
            break;
        strncat(path, token.string, 0x40u);
    }
    if ( token.string[0] != 62 )
        SourceWarning(source, "#include missing trailing >");
    if ( &path[strlen(path) + 1] == &path[1] )
    {
        SourceError(source, "#include without file name between < >");
        return 0;
    }
    PC_ConvertPath(path);
    script = LoadScriptFile(path);
LABEL_30:
    if ( script )
    {
        PC_PushScript(source, script);
        return 1;
    }
    else
    {
        SourceError(source, "file %s not found", path);
        return 0;
    }
}

bool __cdecl PC_WhiteSpaceBeforeToken(token_s *token)
{
    return token->endwhitespace_p - token->whitespace_p > 0;
}

void __cdecl PC_ClearTokenWhiteSpace(token_s *token)
{
    token->whitespace_p = 0;
    token->endwhitespace_p = 0;
    token->linescrossed = 0;
}

int __cdecl PC_Directive_undef(source_s *source)
{
    define_s *lastdefine; // [esp+14h] [ebp-444h]
    int hash; // [esp+18h] [ebp-440h]
    define_s *define; // [esp+1Ch] [ebp-43Ch]
    token_s token; // [esp+20h] [ebp-438h] BYREF

    if ( source->skip > 0 )
        return 1;
    if ( PC_ReadLine(source, &token, 0) )
    {
        if ( token.type == 4 )
        {
            hash = PC_NameHash(token.string);
            lastdefine = 0;
            for ( define = source->definehash[hash]; define; define = define->hashnext )
            {
                if ( !strcmp(define->name, token.string) )
                {
                    if ( (define->flags & 1) != 0 )
                    {
                        SourceWarning(source, "can't undef %s", token.string);
                    }
                    else
                    {
                        if ( lastdefine )
                            lastdefine->hashnext = define->hashnext;
                        else
                            source->definehash[hash] = define->hashnext;
                        PC_FreeDefine(define);
                    }
                    return 1;
                }
                lastdefine = define;
            }
            return 1;
        }
        else
        {
            PC_UnreadSourceToken(source, &token);
            SourceError(source, "expected name, found %s", token.string);
            return 0;
        }
    }
    else
    {
        SourceError(source, "undef without name");
        return 0;
    }
}

int __cdecl PC_ReadLine(source_s *source, token_s *token, bool expandDefines)
{
    int crossline; // [esp+14h] [ebp-8h]
    define_s *define; // [esp+18h] [ebp-4h]

    for ( crossline = 0; ; crossline = 1 )
    {
        while ( 1 )
        {
            if ( !PC_ReadSourceToken(source, token) )
                return 0;
            if ( token->linescrossed > crossline )
            {
                PC_UnreadSourceToken(source, token);
                return 0;
            }
            if ( token->type != 4 )
                break;
            if ( !expandDefines )
                break;
            define = PC_FindHashedDefine(source->definehash, token->string);
            if ( !define )
                break;
            if ( !PC_ExpandDefineIntoSource(source, token, define) )
                return 0;
        }
        if ( strcmp(token->string, "\\") )
            break;
    }
    return 1;
}

int __cdecl PC_ExpandDefineIntoSource(source_s *source, token_s *deftoken, define_s *define)
{
    token_s *firsttoken; // [esp+0h] [ebp-8h] BYREF
    token_s *lasttoken; // [esp+4h] [ebp-4h] BYREF

    if ( !PC_ExpandDefine(source, deftoken, define, &firsttoken, &lasttoken) )
        return 0;
    if ( firsttoken )
    {
        if ( lasttoken )
        {
            lasttoken->next = source->tokens;
            source->tokens = firsttoken;
        }
    }
    return 1;
}

int __cdecl PC_Directive_define(source_s *source)
{
    char v2; // [esp+7Bh] [ebp-45Dh]
    char *name; // [esp+80h] [ebp-458h]
    token_s *p_token; // [esp+84h] [ebp-454h]
    token_s *t; // [esp+98h] [ebp-440h]
    token_s *ta; // [esp+98h] [ebp-440h]
    define_s *define; // [esp+9Ch] [ebp-43Ch]
    define_s *definea; // [esp+9Ch] [ebp-43Ch]
    token_s token; // [esp+A0h] [ebp-438h] BYREF
    token_s *last; // [esp+4D4h] [ebp-4h]

    if ( source->skip > 0 )
        return 1;
    if ( !PC_ReadLine(source, &token, 0) )
    {
        SourceError(source, "#define without name");
        return 0;
    }
    if ( token.type != 4 )
    {
        PC_UnreadSourceToken(source, &token);
        SourceError(source, "expected name after #define, found %s", token.string);
        return 0;
    }
    define = PC_FindHashedDefine(source->definehash, token.string);
    if ( define )
    {
        if ( (define->flags & 1) != 0 )
        {
            SourceError(source, "can't redefine %s", token.string);
            return 0;
        }
        SourceWarning(source, "redefinition of %s", token.string);
        PC_UnreadSourceToken(source, &token);
        if ( !PC_Directive_undef(source) )
            return 0;
        PC_FindHashedDefine(source->definehash, token.string);
    }
    definea = (define_s *)GetMemory(&token.string[strlen(token.string) + 1] - &token.string[1] + 33);
    definea->name = 0;
    definea->flags = 0;
    definea->builtin = 0;
    definea->numparms = 0;
    definea->parms = 0;
    definea->tokens = 0;
    definea->next = 0;
    definea->hashnext = 0;
    definea->name = (char *)&definea[1];
    p_token = &token;
    name = definea->name;
    do
    {
        v2 = p_token->string[0];
        *name = p_token->string[0];
        p_token = (token_s *)((char *)p_token + 1);
        ++name;
    }
    while ( v2 );
    PC_AddDefineToHash(definea, source->definehash);
    if ( !PC_ReadLine(source, &token, 0) )
        return 1;
    if ( PC_WhiteSpaceBeforeToken(&token) || strcmp(token.string, "(") )
    {
LABEL_37:
        last = 0;
        do
        {
            ta = PC_CopyToken(&token);
            if ( ta->type == 4 && !strcmp(ta->string, definea->name) )
            {
                SourceError(source, "recursive define (removed recursion)");
            }
            else
            {
                PC_ClearTokenWhiteSpace(ta);
                ta->next = 0;
                if ( last )
                    last->next = ta;
                else
                    definea->tokens = ta;
                last = ta;
            }
        }
        while ( PC_ReadLine(source, &token, 0) );
        if ( !last || strcmp(definea->tokens->string, "##") && strcmp(last->string, "##") )
            return 1;
        SourceError(source, "define with misplaced ##");
        return 0;
    }
    last = 0;
    if ( PC_CheckTokenString(source, ")") )
    {
LABEL_35:
        if ( !PC_ReadLine(source, &token, 0) )
            return 1;
        goto LABEL_37;
    }
    do
    {
        if ( !PC_ReadLine(source, &token, 0) )
        {
            SourceError(source, "expected define parameter");
            return 0;
        }
        if ( token.type != 4 )
        {
            SourceError(source, "invalid define parameter");
            return 0;
        }
        if ( PC_FindDefineParm(definea, token.string) >= 0 )
        {
            SourceError(source, "two of the same define parameters");
            return 0;
        }
        t = PC_CopyToken(&token);
        PC_ClearTokenWhiteSpace(t);
        t->next = 0;
        if ( last )
            last->next = t;
        else
            definea->parms = t;
        last = t;
        ++definea->numparms;
        if ( !PC_ReadLine(source, &token, 0) )
        {
            SourceError(source, "define parameters not terminated");
            return 0;
        }
        if ( !strcmp(token.string, ")") )
            goto LABEL_35;
    }
    while ( !strcmp(token.string, ",") );
    SourceError(source, "define not terminated");
    return 0;
}

define_s *__cdecl PC_DefineFromString(char *string)
{
    source_s src; // [esp+10h] [ebp-4E8h] BYREF
    token_s *t; // [esp+4E4h] [ebp-14h]
    define_s *def; // [esp+4E8h] [ebp-10h]
    script_s *script; // [esp+4ECh] [ebp-Ch]
    int i; // [esp+4F0h] [ebp-8h]
    int res; // [esp+4F4h] [ebp-4h]

    script = LoadScriptMemory(string, strlen(string), "*extern");
    memset(&src, 0, sizeof(src));
    strncpy((char *)&src, "*extern", 0x40u);
    src.scriptstack = script;
    src.definehash = (define_s **)GetClearedMemory(0x1000u);
    res = PC_Directive_define(&src);
    for ( t = src.tokens; t; t = src.tokens )
    {
        src.tokens = src.tokens->next;
        PC_FreeToken(t);
    }
    def = 0;
    for ( i = 0; i < 1024; ++i )
    {
        if ( src.definehash[i] )
        {
            def = src.definehash[i];
            break;
        }
    }
    FreeMemory((char *)src.definehash);
    FreeScript(script);
    if ( res > 0 )
        return def;
    if ( src.defines )
        PC_FreeDefine(def);
    return 0;
}

int __cdecl PC_AddDefine(source_s *source, char *string)
{
    define_s *define; // [esp+0h] [ebp-4h]

    define = PC_DefineFromString(string);
    if ( !define )
        return 0;
    PC_AddDefineToHash(define, source->definehash);
    return 1;
}

define_s *__cdecl PC_CopyDefine(source_s *source, define_s *define)
{
    char v2; // dl
    _BYTE *v4; // [esp+8h] [ebp-28h]
    char *name; // [esp+Ch] [ebp-24h]
    unsigned int *newdefine; // [esp+20h] [ebp-10h]
    token_s *newtoken; // [esp+24h] [ebp-Ch]
    token_s *newtokena; // [esp+24h] [ebp-Ch]
    token_s *token; // [esp+28h] [ebp-8h]
    token_s *tokena; // [esp+28h] [ebp-8h]
    token_s *lasttoken; // [esp+2Ch] [ebp-4h]
    token_s *lasttokena; // [esp+2Ch] [ebp-4h]

    newdefine = GetMemory(strlen(define->name) + 33);
    *newdefine = (unsigned int)(newdefine + 8);
    name = define->name;
    v4 = (_BYTE *)*newdefine;
    do
    {
        v2 = *name;
        *v4++ = *name++;
    }
    while ( v2 );
    newdefine[1] = define->flags;
    newdefine[2] = define->builtin;
    newdefine[3] = define->numparms;
    newdefine[6] = 0;
    newdefine[7] = 0;
    newdefine[5] = 0;
    lasttoken = 0;
    for ( token = define->tokens; token; token = token->next )
    {
        newtoken = PC_CopyToken(token);
        newtoken->next = 0;
        if ( lasttoken )
            lasttoken->next = newtoken;
        else
            newdefine[5] = (unsigned int)newtoken;
        lasttoken = newtoken;
    }
    newdefine[4] = 0;
    lasttokena = 0;
    for ( tokena = define->parms; tokena; tokena = tokena->next )
    {
        newtokena = PC_CopyToken(tokena);
        newtokena->next = 0;
        if ( lasttokena )
            lasttokena->next = newtokena;
        else
            newdefine[4] = (unsigned int)newtokena;
        lasttokena = newtokena;
    }
    return (define_s *)newdefine;
}

define_s *globaldefines;
void __cdecl PC_AddGlobalDefinesToSource(source_s *source)
{
    define_s *newdefine; // [esp+0h] [ebp-8h]
    define_s *define; // [esp+4h] [ebp-4h]

    for ( define = globaldefines; define; define = define->next )
    {
        newdefine = PC_CopyDefine(source, define);
        PC_AddDefineToHash(newdefine, source->definehash);
    }
}

int __cdecl PC_Directive_if_def(source_s *source, int type)
{
    define_s *d; // [esp+0h] [ebp-440h]
    token_s token; // [esp+8h] [ebp-438h] BYREF

    if ( PC_ReadLine(source, &token, 0) )
    {
        if ( token.type == 4 )
        {
            d = PC_FindHashedDefine(source->definehash, token.string);
            PC_PushIndent(source, type, (parseSkip_t)((type == 8) == (d == 0)));
            return 1;
        }
        else
        {
            PC_UnreadSourceToken(source, &token);
            SourceError(source, "expected name after #ifdef, found %s", token.string);
            return 0;
        }
    }
    else
    {
        SourceError(source, "#ifdef without name");
        return 0;
    }
}

void __cdecl PC_PushIndent(source_s *source, int type, parseSkip_t skip)
{
    indent_s *indent; // [esp+0h] [ebp-4h]

    indent = (indent_s *)GetMemory(0x10u);
    indent->type = type;
    indent->script = source->scriptstack;
    indent->skip = skip;
    source->skip += indent->skip != SKIP_NO;
    indent->next = source->indentstack;
    source->indentstack = indent;
}

int __cdecl PC_Directive_ifdef(source_s *source)
{
    return PC_Directive_if_def(source, 8);
}

int __cdecl PC_Directive_ifndef(source_s *source)
{
    return PC_Directive_if_def(source, 16);
}

int __cdecl PC_Directive_else(source_s *source)
{
    parseSkip_t skip; // [esp+0h] [ebp-8h] BYREF
    int type; // [esp+4h] [ebp-4h] BYREF

    PC_PopIndent(source, &type, &skip);
    if ( type )
    {
        if ( type == 2 )
        {
            SourceError(source, "#else after #else");
            return 0;
        }
        else
        {
            PC_PushIndent(source, 2, (parseSkip_t)(skip != SKIP_YES));
            return 1;
        }
    }
    else
    {
        SourceError(source, "misplaced #else");
        return 0;
    }
}

int __cdecl PC_Directive_endif(source_s *source)
{
    parseSkip_t skip; // [esp+0h] [ebp-8h] BYREF
    int type; // [esp+4h] [ebp-4h] BYREF

    PC_PopIndent(source, &type, &skip);
    if ( type )
        return 1;
    SourceError(source, "misplaced #endif");
    return 0;
}

int __cdecl PC_OperatorPriority(int op)
{
    int result; // eax

    switch ( op )
    {
        case 5:
            result = 7;
            break;
        case 6:
            result = 6;
            break;
        case 7:
            result = 12;
            break;
        case 8:
            result = 12;
            break;
        case 9:
            result = 11;
            break;
        case 10:
            result = 11;
            break;
        case 21:
            result = 13;
            break;
        case 22:
            result = 13;
            break;
        case 26:
            result = 15;
            break;
        case 27:
            result = 15;
            break;
        case 28:
            result = 15;
            break;
        case 29:
            result = 14;
            break;
        case 30:
            result = 14;
            break;
        case 32:
            result = 10;
            break;
        case 33:
            result = 8;
            break;
        case 34:
            result = 9;
            break;
        case 35:
            result = 16;
            break;
        case 36:
            result = 16;
            break;
        case 37:
            result = 12;
            break;
        case 38:
            result = 12;
            break;
        case 42:
            result = 5;
            break;
        case 43:
            result = 5;
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

int __cdecl PC_EvaluateTokens(source_s *source, token_s *tokens, int *intvalue, long double *floatvalue, int integer)
{
    bool v6; // [esp+18h] [ebp-DC0h]
    bool v7; // [esp+1Ch] [ebp-DBCh]
    bool v8; // [esp+20h] [ebp-DB8h]
    bool v9; // [esp+24h] [ebp-DB4h]
    int type; // [esp+74h] [ebp-D64h]
    operator_s *o; // [esp+78h] [ebp-D60h]
    operator_s *oa; // [esp+78h] [ebp-D60h]
    operator_s *ob; // [esp+78h] [ebp-D60h]
    double questmarkfloatvalue; // [esp+80h] [ebp-D58h]
    operator_s *lastoperator; // [esp+8Ch] [ebp-D4Ch]
    value_s value_heap[64]; // [esp+90h] [ebp-D48h] BYREF
    operator_s *firstoperator; // [esp+894h] [ebp-544h]
    int lastwasvalue; // [esp+898h] [ebp-540h]
    value_s *v2; // [esp+89Ch] [ebp-53Ch]
    int error; // [esp+8A0h] [ebp-538h]
    int negativevalue; // [esp+8A4h] [ebp-534h]
    operator_s operator_heap[64]; // [esp+8A8h] [ebp-530h] BYREF
    value_s *firstvalue; // [esp+DACh] [ebp-2Ch]
    value_s *v1; // [esp+DB0h] [ebp-28h]
    int numvalues; // [esp+DB4h] [ebp-24h]
    int parentheses; // [esp+DB8h] [ebp-20h]
    int numoperators; // [esp+DBCh] [ebp-1Ch]
    value_s *v; // [esp+DC0h] [ebp-18h]
    value_s *lastvalue; // [esp+DC4h] [ebp-14h]
    int lastoperatortype; // [esp+DC8h] [ebp-10h]
    int brace; // [esp+DCCh] [ebp-Ch]
    int questmarkintvalue; // [esp+DD0h] [ebp-8h]
    int gotquestmarkvalue; // [esp+DD4h] [ebp-4h]

    brace = 0;
    parentheses = 0;
    error = 0;
    lastwasvalue = 0;
    negativevalue = 0;
    questmarkintvalue = 0;
    gotquestmarkvalue = 0;
    lastoperatortype = 0;
    numoperators = 0;
    numvalues = 0;
    lastoperator = 0;
    firstoperator = 0;
    lastvalue = 0;
    firstvalue = 0;
    if ( intvalue )
        *intvalue = 0;
    if ( floatvalue )
        *floatvalue = 0.0;
    while ( tokens )
    {
        type = tokens->type;
        switch ( type )
        {
            case 3:
                if ( lastwasvalue )
                    goto LABEL_37;
                if ( numvalues >= 64 )
                {
LABEL_24:
                    SourceError(source, "out of value space\n");
                    error = 1;
                    break;
                }
                v = &value_heap[numvalues++];
                if ( negativevalue )
                {
                    v->intvalue = -(int)tokens->intvalue;
                    v->floatvalue = -tokens->floatvalue;
                }
                else
                {
                    v->intvalue = tokens->intvalue;
                    v->floatvalue = tokens->floatvalue;
                }
                v->parentheses = parentheses;
                v->next = 0;
                v->prev = lastvalue;
                if ( lastvalue )
                    lastvalue->next = v;
                else
                    firstvalue = v;
                lastvalue = v;
                lastwasvalue = 1;
                negativevalue = 0;
                break;
            case 4:
                if ( lastwasvalue || negativevalue )
                {
LABEL_37:
                    SourceError(source, "syntax error in #if/#elif");
                    error = 1;
                    break;
                }
                if ( strcmp(tokens->string, "defined") )
                {
                    SourceError(source, "undefined name %s in #if/#elif", tokens->string);
                    error = 1;
                    break;
                }
                tokens = tokens->next;
                if ( !strcmp(tokens->string, "(") )
                {
                    brace = 1;
                    tokens = tokens->next;
                }
                if ( tokens && tokens->type == 4 )
                {
                    if ( numvalues >= 64 )
                        goto LABEL_24;
                    v = &value_heap[numvalues++];
                    if ( PC_FindHashedDefine(source->definehash, tokens->string) )
                    {
                        v->intvalue = 1;
                        v->floatvalue = 1.0;
                    }
                    else
                    {
                        v->intvalue = 0;
                        v->floatvalue = 0.0;
                    }
                    v->parentheses = parentheses;
                    v->next = 0;
                    v->prev = lastvalue;
                    if ( lastvalue )
                        lastvalue->next = v;
                    else
                        firstvalue = v;
                    lastvalue = v;
                    if ( !brace || (tokens = tokens->next) != 0 && !strcmp(tokens->string, ")") )
                    {
                        brace = 0;
                        lastwasvalue = 1;
                    }
                    else
                    {
                        SourceError(source, "defined without ) in #if/#elif");
                        error = 1;
                    }
                }
                else
                {
                    if ( tokens )
                        SourceError(source, "defined without name in #if/#elif; got %s", tokens->string);
                    else
                        SourceError(source, "defined without name in #if/#elif; got %s", "end-of-file");
                    error = 1;
                }
                break;
            case 5:
                if ( negativevalue )
                {
                    SourceError(source, "misplaced minus sign in #if/#elif");
                    error = 1;
                }
                else if ( tokens->subtype == 44 )
                {
                    ++parentheses;
                }
                else if ( tokens->subtype == 45 )
                {
                    if ( --parentheses < 0 )
                    {
                        SourceError(source, "too many ) in #if/#elsif");
                        error = 1;
                    }
                }
                else if ( !integer
                             && (tokens->subtype == 35
                                || tokens->subtype == 28
                                || tokens->subtype == 21
                                || tokens->subtype == 22
                                || tokens->subtype == 32
                                || tokens->subtype == 33
                                || tokens->subtype == 34) )
                {
                    SourceError(source, "illigal operator %s on floating point operands\n", tokens->string);
                    error = 1;
                }
                else
                {
                    switch ( tokens->subtype )
                    {
                        case 5:
                        case 6:
                        case 7:
                        case 8:
                        case 9:
                        case 0xA:
                        case 0x15:
                        case 0x16:
                        case 0x1A:
                        case 0x1B:
                        case 0x1C:
                        case 0x1D:
                        case 0x20:
                        case 0x21:
                        case 0x22:
                        case 0x25:
                        case 0x26:
                        case 0x2A:
                        case 0x2B:
                            goto $LN82;
                        case 0x10:
                        case 0x11:
                            SourceError(source, "++ or -- used in #if/#elif");
                            break;
                        case 0x1E:
                            if ( lastwasvalue )
                            {
$LN82:
                                if ( !lastwasvalue )
                                {
                                    SourceError(source, "operator %s after operator in #if/#elif", tokens->string);
                                    error = 1;
                                }
                            }
                            else
                            {
                                negativevalue = 1;
                            }
                            break;
                        case 0x23:
                        case 0x24:
                            if ( lastwasvalue )
                            {
                                SourceError(source, "! or ~ after value in #if/#elif");
                                error = 1;
                            }
                            break;
                        default:
                            SourceError(source, "invalid operator %s in #if/#elif", tokens->string);
                            error = 1;
                            break;
                    }
                    if ( !error && !negativevalue )
                    {
                        if ( numoperators < 64 )
                        {
                            o = &operator_heap[numoperators++];
                            o->op = tokens->subtype;
                            o->priority = PC_OperatorPriority(tokens->subtype);
                            o->parentheses = parentheses;
                            o->next = 0;
                            o->prev = lastoperator;
                            if ( lastoperator )
                                lastoperator->next = o;
                            else
                                firstoperator = o;
                            lastoperator = o;
                            lastwasvalue = 0;
                        }
                        else
                        {
                            SourceError(source, "out of operator space\n");
                            error = 1;
                        }
                    }
                }
                break;
            default:
                SourceError(source, "unknown %s in #if/#elif", tokens->string);
                error = 1;
                break;
        }
        if ( error )
            break;
        tokens = tokens->next;
    }
    if ( !error )
    {
        if ( lastwasvalue )
        {
            if ( parentheses )
            {
                SourceError(source, "too many ( in #if/#elif");
                error = 1;
            }
        }
        else
        {
            SourceError(source, "trailing operator in #if/#elif");
            error = 1;
        }
    }
    gotquestmarkvalue = 0;
    questmarkintvalue = 0;
    questmarkfloatvalue = 0.0;
    while ( !error && firstoperator )
    {
        v = firstvalue;
        for ( oa = firstoperator;
                    oa->next
             && oa->parentheses <= oa->next->parentheses
             && (oa->parentheses != oa->next->parentheses || oa->priority < oa->next->priority);
                    oa = oa->next )
        {
            if ( oa->op != 36 && oa->op != 35 )
                v = v->next;
            if ( !v )
            {
                SourceError(source, "mising values in #if/#elif");
                error = 1;
                break;
            }
        }
        if ( error )
            break;
        v1 = v;
        v2 = v->next;
        switch ( oa->op )
        {
            case 5:
                v9 = v1->intvalue && v2->intvalue;
                v1->intvalue = v9;
                v8 = v1->floatvalue != 0.0 && v2->floatvalue != 0.0;
                v1->floatvalue = (double)v8;
                break;
            case 6:
                v7 = v1->intvalue || v2->intvalue;
                v1->intvalue = v7;
                v6 = v1->floatvalue != 0.0 || v2->floatvalue != 0.0;
                v1->floatvalue = (double)v6;
                break;
            case 7:
                v1->intvalue = v1->intvalue >= v2->intvalue;
                v1->floatvalue = (double)(v1->floatvalue >= v2->floatvalue);
                break;
            case 8:
                v1->intvalue = v1->intvalue <= v2->intvalue;
                v1->floatvalue = (double)(v2->floatvalue >= v1->floatvalue);
                break;
            case 9:
                v1->intvalue = v1->intvalue == v2->intvalue;
                v1->floatvalue = (double)(v1->floatvalue == v2->floatvalue);
                break;
            case 0xA:
                v1->intvalue = v1->intvalue != v2->intvalue;
                v1->floatvalue = (double)(v1->floatvalue != v2->floatvalue);
                break;
            case 0x15:
                v1->intvalue >>= v2->intvalue;
                break;
            case 0x16:
                v1->intvalue <<= v2->intvalue;
                break;
            case 0x1A:
                v1->intvalue *= v2->intvalue;
                v1->floatvalue = v1->floatvalue * v2->floatvalue;
                break;
            case 0x1B:
                if ( !v2->intvalue || v2->floatvalue == 0.0 )
                    goto LABEL_113;
                v1->intvalue /= v2->intvalue;
                v1->floatvalue = v1->floatvalue / v2->floatvalue;
                break;
            case 0x1C:
                if ( v2->intvalue )
                {
                    v1->intvalue %= v2->intvalue;
                }
                else
                {
LABEL_113:
                    SourceError(source, "divide by zero in #if/#elif\n");
                    error = 1;
                }
                break;
            case 0x1D:
                v1->intvalue += v2->intvalue;
                v1->floatvalue = v1->floatvalue + v2->floatvalue;
                break;
            case 0x1E:
                v1->intvalue -= v2->intvalue;
                v1->floatvalue = v1->floatvalue - v2->floatvalue;
                break;
            case 0x20:
                v1->intvalue &= v2->intvalue;
                break;
            case 0x21:
                v1->intvalue |= v2->intvalue;
                break;
            case 0x22:
                v1->intvalue ^= v2->intvalue;
                break;
            case 0x23:
                v1->intvalue = ~v1->intvalue;
                break;
            case 0x24:
                v1->intvalue = v1->intvalue == 0;
                v1->floatvalue = (double)(v1->floatvalue == 0.0);
                break;
            case 0x25:
                v1->intvalue = v1->intvalue > v2->intvalue;
                v1->floatvalue = (double)(v1->floatvalue > v2->floatvalue);
                break;
            case 0x26:
                v1->intvalue = v1->intvalue < v2->intvalue;
                v1->floatvalue = (double)(v2->floatvalue > v1->floatvalue);
                break;
            case 0x2A:
                if ( gotquestmarkvalue )
                {
                    if ( integer )
                    {
                        if ( !questmarkintvalue )
                            v1->intvalue = v2->intvalue;
                    }
                    else if ( questmarkfloatvalue == 0.0 )
                    {
                        v1->floatvalue = v2->floatvalue;
                    }
                    gotquestmarkvalue = 0;
                }
                else
                {
                    SourceError(source, ": without ? in #if/#elif");
                    error = 1;
                }
                break;
            case 0x2B:
                if ( gotquestmarkvalue )
                {
                    SourceError(source, "? after ? in #if/#elif");
                    error = 1;
                }
                else
                {
                    questmarkintvalue = v1->intvalue;
                    questmarkfloatvalue = v1->floatvalue;
                    gotquestmarkvalue = 1;
                }
                break;
            default:
                break;
        }
        if ( error )
            break;
        lastoperatortype = oa->op;
        if ( oa->op != 36 && oa->op != 35 )
        {
            if ( oa->op != 43 )
                v = v->next;
            if ( v->prev )
                v->prev->next = v->next;
            else
                firstvalue = v->next;
            if ( v->next )
                v->next->prev = v->prev;
            else
                lastvalue = v->prev;
        }
        if ( oa->prev )
            oa->prev->next = oa->next;
        else
            firstoperator = oa->next;
        if ( oa->next )
            oa->next->prev = oa->prev;
    }
    if ( firstvalue )
    {
        if ( intvalue )
            *intvalue = firstvalue->intvalue;
        if ( floatvalue )
            *floatvalue = firstvalue->floatvalue;
    }
    for ( ob = firstoperator; ob; ob = ob->next )
        ;
    for ( v = firstvalue; v; v = lastvalue )
        lastvalue = v->next;
    if ( !error )
        return 1;
    if ( intvalue )
        *intvalue = 0;
    if ( floatvalue )
        *floatvalue = 0.0;
    return 0;
}

int __cdecl PC_DollarEvaluate(source_s *source, int *intvalue, long double *floatvalue, int integer)
{
    token_s *t; // [esp+14h] [ebp-454h]
    token_s *ta; // [esp+14h] [ebp-454h]
    token_s *tb; // [esp+14h] [ebp-454h]
    token_s *tc; // [esp+14h] [ebp-454h]
    define_s *define; // [esp+18h] [ebp-450h]
    int defined; // [esp+1Ch] [ebp-44Ch]
    token_s *nexttoken; // [esp+20h] [ebp-448h]
    int indent; // [esp+24h] [ebp-444h]
    token_s token; // [esp+28h] [ebp-440h] BYREF
    token_s *firsttoken; // [esp+460h] [ebp-8h]
    token_s *lasttoken; // [esp+464h] [ebp-4h]

    defined = 0;
    if ( intvalue )
        *intvalue = 0;
    if ( floatvalue )
        *floatvalue = 0.0;
    if ( !PC_ReadSourceToken(source, &token) )
    {
        SourceError(source, "no leading ( after $evalint/$evalfloat");
        return 0;
    }
    if ( !PC_ReadSourceToken(source, &token) )
    {
        SourceError(source, "nothing to evaluate");
        return 0;
    }
    indent = 1;
    firsttoken = 0;
    lasttoken = 0;
    do
    {
        if ( token.type == 4 )
        {
            if ( defined )
            {
                defined = 0;
                t = PC_CopyToken(&token);
                t->next = 0;
                if ( lasttoken )
                    lasttoken->next = t;
                else
                    firsttoken = t;
                lasttoken = t;
            }
            else if ( !strcmp(token.string, "defined") )
            {
                defined = 1;
                ta = PC_CopyToken(&token);
                ta->next = 0;
                if ( lasttoken )
                    lasttoken->next = ta;
                else
                    firsttoken = ta;
                lasttoken = ta;
            }
            else
            {
                define = PC_FindHashedDefine(source->definehash, token.string);
                if ( !define )
                {
                    SourceError(source, "can't evaluate %s, not defined", token.string);
                    return 0;
                }
                if ( !PC_ExpandDefineIntoSource(source, &token, define) )
                    return 0;
            }
            continue;
        }
        if ( token.type != 3 && token.type != 5 )
        {
            SourceError(source, "can't evaluate %s", token.string);
            return 0;
        }
        if ( token.string[0] == 40 )
        {
            ++indent;
        }
        else if ( token.string[0] == 41 )
        {
            --indent;
        }
        if ( indent <= 0 )
            break;
        tb = PC_CopyToken(&token);
        tb->next = 0;
        if ( lasttoken )
            lasttoken->next = tb;
        else
            firsttoken = tb;
        lasttoken = tb;
    }
    while ( PC_ReadSourceToken(source, &token) );
    if ( !PC_EvaluateTokens(source, firsttoken, intvalue, floatvalue, integer) )
        return 0;
    for ( tc = firsttoken; tc; tc = nexttoken )
    {
        nexttoken = tc->next;
        PC_FreeToken(tc);
    }
    return 1;
}

int __cdecl PC_Directive_elif(source_s *source)
{
    parseSkip_t skip; // [esp+0h] [ebp-Ch] BYREF
    int type; // [esp+4h] [ebp-8h] BYREF
    int value; // [esp+8h] [ebp-4h] BYREF

    PC_PopIndent(source, &type, &skip);
    if ( type && type != 2 )
    {
        if ( PC_Evaluate(source, &value, 0, 1) )
        {
            if ( skip == SKIP_YES )
                skip = (parseSkip_t)(value == 0);
            else
                skip = SKIP_ALL_ELIFS;
            PC_PushIndent(source, 4, skip);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        SourceError(source, "misplaced #elif");
        return 0;
    }
}

int __cdecl PC_Evaluate(source_s *source, int *intvalue, long double *floatvalue, int integer)
{
    token_s *t; // [esp+14h] [ebp-450h]
    token_s *ta; // [esp+14h] [ebp-450h]
    token_s *tb; // [esp+14h] [ebp-450h]
    token_s *tc; // [esp+14h] [ebp-450h]
    define_s *define; // [esp+18h] [ebp-44Ch]
    token_s *nexttoken; // [esp+1Ch] [ebp-448h]
    int defined; // [esp+20h] [ebp-444h]
    token_s token; // [esp+24h] [ebp-440h] BYREF
    token_s *firsttoken; // [esp+45Ch] [ebp-8h]
    token_s *lasttoken; // [esp+460h] [ebp-4h]

    defined = 0;
    if ( intvalue )
        *intvalue = 0;
    if ( floatvalue )
        *floatvalue = 0.0;
    if ( PC_ReadLine(source, &token, 1) )
    {
        firsttoken = 0;
        lasttoken = 0;
        do
        {
            if ( token.type == 4 )
            {
                if ( defined )
                {
                    defined = 0;
                    t = PC_CopyToken(&token);
                    t->next = 0;
                    if ( lasttoken )
                        lasttoken->next = t;
                    else
                        firsttoken = t;
                    lasttoken = t;
                }
                else if ( !strcmp(token.string, "defined") )
                {
                    defined = 1;
                    ta = PC_CopyToken(&token);
                    ta->next = 0;
                    if ( lasttoken )
                        lasttoken->next = ta;
                    else
                        firsttoken = ta;
                    lasttoken = ta;
                }
                else
                {
                    define = PC_FindHashedDefine(source->definehash, token.string);
                    if ( !define )
                    {
                        SourceError(source, "can't evaluate %s, not defined", token.string);
                        return 0;
                    }
                    if ( !PC_ExpandDefineIntoSource(source, &token, define) )
                        return 0;
                }
            }
            else
            {
                if ( token.type != 3 && token.type != 5 )
                {
                    SourceError(source, "can't evaluate %s", token.string);
                    return 0;
                }
                tb = PC_CopyToken(&token);
                tb->next = 0;
                if ( lasttoken )
                    lasttoken->next = tb;
                else
                    firsttoken = tb;
                lasttoken = tb;
            }
        }
        while ( PC_ReadLine(source, &token, defined == 0) );
        if ( PC_EvaluateTokens(source, firsttoken, intvalue, floatvalue, integer) )
        {
            for ( tc = firsttoken; tc; tc = nexttoken )
            {
                nexttoken = tc->next;
                PC_FreeToken(tc);
            }
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        SourceError(source, "no value after #if/#elif");
        return 0;
    }
}

int __cdecl PC_Directive_if(source_s *source)
{
    int value; // [esp+4h] [ebp-4h] BYREF

    if ( !PC_Evaluate(source, &value, 0, 1) )
        return 0;
    PC_PushIndent(source, 1, (parseSkip_t)(value == 0));
    return 1;
}

int __cdecl PC_Directive_line(source_s *source)
{
    SourceError(source, "#line directive not supported");
    return 0;
}

int __cdecl PC_Directive_error(source_s *source)
{
    token_s token; // [esp+0h] [ebp-438h] BYREF

    token.string[0] = 0;
    PC_ReadSourceToken(source, &token);
    if ( source->skip )
        return 1;
    SourceError(source, "#error directive: %s", token.string);
    return 0;
}

int __cdecl PC_Directive_pragma(source_s *source)
{
    token_s token; // [esp+0h] [ebp-438h] BYREF

    SourceWarning(source, "#pragma directive not supported");
    while ( PC_ReadLine(source, &token, 0) )
        ;
    return 1;
}

void __cdecl UnreadSignToken(source_s *source)
{
    token_s token; // [esp+0h] [ebp-438h] BYREF

    token.line = source->scriptstack->line;
    token.whitespace_p = source->scriptstack->script_p;
    token.endwhitespace_p = source->scriptstack->script_p;
    token.linescrossed = 0;
    strcpy(token.string, "-");
    token.type = 5;
    token.subtype = 30;
    PC_UnreadSourceToken(source, &token);
}

int __cdecl PC_Directive_eval(source_s *source)
{
    token_s token; // [esp+0h] [ebp-438h] BYREF
    int value; // [esp+434h] [ebp-4h] BYREF

    if ( !PC_Evaluate(source, &value, 0, 1) )
        return 0;
    token.line = source->scriptstack->line;
    token.whitespace_p = source->scriptstack->script_p;
    token.endwhitespace_p = source->scriptstack->script_p;
    token.linescrossed = 0;
    sprintf(token.string, "%d", abs(value));
    token.type = 3;
    token.subtype = 12296;
    PC_UnreadSourceToken(source, &token);
    if ( value < 0 )
        UnreadSignToken(source);
    return 1;
}

int __cdecl PC_Directive_evalfloat(source_s *source)
{
    float v2; // xmm0_4
    token_s token; // [esp+Ch] [ebp-440h] BYREF
    long double value; // [esp+444h] [ebp-8h] BYREF

    if ( !PC_Evaluate(source, 0, &value, 0) )
        return 0;
    token.line = source->scriptstack->line;
    token.whitespace_p = source->scriptstack->script_p;
    token.endwhitespace_p = source->scriptstack->script_p;
    token.linescrossed = 0;
    v2 = value;
    sprintf(token.string, "%1.2f", fabs(v2));
    token.type = 3;
    token.subtype = 10248;
    PC_UnreadSourceToken(source, &token);
    if ( value < 0.0 )
        UnreadSignToken(source);
    return 1;
}

int __cdecl PC_ReadDirective(source_s *source)
{
    token_s token; // [esp+14h] [ebp-438h] BYREF
    int i; // [esp+448h] [ebp-4h]

    if ( PC_ReadSourceToken(source, &token) )
    {
        if ( token.linescrossed <= 0 )
        {
            if ( token.type == 4 )
            {
                for ( i = 0; directives[i].name; ++i )
                {
                    if ( !strcmp(directives[i].name, token.string) )
                        return directives[i].func(source);
                }
            }
            SourceError(source, "unknown precompiler directive %s", token.string);
            return 0;
        }
        else
        {
            PC_UnreadSourceToken(source, &token);
            SourceError(source, "found # at end of line");
            return 0;
        }
    }
    else
    {
        SourceError(source, "found # without name");
        return 0;
    }
}

int __cdecl PC_DollarDirective_evalint(source_s *source)
{
    token_s token; // [esp+0h] [ebp-438h] BYREF
    int value; // [esp+434h] [ebp-4h] BYREF

    if ( !PC_DollarEvaluate(source, &value, 0, 1) )
        return 0;
    token.line = source->scriptstack->line;
    token.whitespace_p = source->scriptstack->script_p;
    token.endwhitespace_p = source->scriptstack->script_p;
    token.linescrossed = 0;
    sprintf(token.string, "%d", abs(value));
    token.type = 3;
    token.subtype = 12296;
    token.intvalue = value;
    token.floatvalue = (double)value;
    PC_UnreadSourceToken(source, &token);
    if ( value < 0 )
        UnreadSignToken(source);
    return 1;
}

int __cdecl PC_DollarDirective_evalfloat(source_s *source)
{
    float v2; // xmm0_4
    token_s token; // [esp+1Ch] [ebp-440h] BYREF
    long double value; // [esp+454h] [ebp-8h] BYREF

    if ( !PC_DollarEvaluate(source, 0, &value, 0) )
        return 0;
    token.line = source->scriptstack->line;
    token.whitespace_p = source->scriptstack->script_p;
    token.endwhitespace_p = source->scriptstack->script_p;
    token.linescrossed = 0;
    v2 = value;
    sprintf(token.string, "%1.2f", fabs(v2));
    token.type = 3;
    token.subtype = 10248;
    token.intvalue = (__int64)value;
    token.floatvalue = value;
    PC_UnreadSourceToken(source, &token);
    if ( value < 0.0 )
        UnreadSignToken(source);
    return 1;
}

int __cdecl PC_ReadDollarDirective(source_s *source)
{
    token_s token; // [esp+14h] [ebp-438h] BYREF
    int i; // [esp+448h] [ebp-4h]

    if (PC_ReadSourceToken(source, &token))
    {
        if (token.linescrossed <= 0)
        {
            if (token.type == 4)
            {
                for (i = 0; dollardirectives[i].name; ++i)
                {
                    if (!strcmp(dollardirectives[i].name, token.string))
                        return dollardirectives[i].func(source);
                }
            }
            PC_UnreadSourceToken(source, &token);
            SourceError(source, "unknown precompiler directive %s", token.string);
            return 0;
        }
        else
        {
            PC_UnreadSourceToken(source, &token);
            SourceError(source, "found $ at end of line");
            return 0;
        }
    }
    else
    {
        SourceError(source, "found $ without name");
        return 0;
    }
}

int __cdecl PC_ReadToken(source_s *source, token_s *token)
{
    token_s newtoken; // [esp+54h] [ebp-438h] BYREF
    define_s *define; // [esp+488h] [ebp-4h]

    while ( 1 )
    {
        do
        {
            while ( 1 )
            {
                while ( 1 )
                {
                    if ( !PC_ReadSourceToken(source, token) )
                        return 0;
                    if ( token->type != 5 || token->string[0] != 35 || token->string[1] )
                        break;
                    if ( !PC_ReadDirective(source) )
                        return 0;
                }
                if ( token->type != 5 || token->string[0] != 36 )
                    break;
                if ( !PC_ReadDollarDirective(source) )
                    return 0;
            }
        }
        while ( source->skip );
        if ( token->type == 1 && PC_ReadToken(source, &newtoken) )
        {
            if ( newtoken.type == 1 )
            {
                token->string[strlen(token->string) - 1] = 0;
                if ( strlen(token->string) + &newtoken.string[strlen(&newtoken.string[1]) + 2] - &newtoken.string[2] + 1 >= 0x400 )
                {
                    SourceError(source, "string longer than MAX_TOKEN %d\n", 1024);
                    return 0;
                }
                memcpy(
                    &token->string[strlen(token->string)],
                    &newtoken.string[1],
                    &newtoken.string[strlen(&newtoken.string[1]) + 2] - &newtoken.string[1]);
            }
            else
            {
                PC_UnreadToken(source, &newtoken);
            }
        }
        if ( token->type != 4 )
            break;
        define = PC_FindHashedDefine(source->definehash, token->string);
        if ( !define )
            break;
        if ( !PC_ExpandDefineIntoSource(source, token, define) )
            return 0;
    }
    memcpy(&source->token, token, sizeof(source->token));
    return 1;
}

int __cdecl PC_CheckTokenString(source_s *source, char *string)
{
    token_s tok; // [esp+14h] [ebp-438h] BYREF

    if ( !PC_ReadToken(source, &tok) )
        return 0;
    if ( !strcmp(tok.string, string) )
        return 1;
    PC_UnreadSourceToken(source, &tok);
    return 0;
}

void __cdecl PC_UnreadLastToken(source_s *source)
{
    PC_UnreadSourceToken(source, &source->token);
}

void __cdecl PC_UnreadToken(source_s *source, token_s *token)
{
    PC_UnreadSourceToken(source, token);
}

source_s *__cdecl LoadSourceFile(char *filename)
{
    source_s *source; // [esp+0h] [ebp-8h]
    script_s *script; // [esp+4h] [ebp-4h]

    script = LoadScriptFile(filename);
    if ( !script )
        return 0;
    script->next = 0;
    source = (source_s *)GetMemory(0x4D0u);
    memset(source, 0, sizeof(source_s));
    strncpy((char*)source, filename, 0x40u);
    source->scriptstack = script;
    source->tokens = 0;
    source->defines = 0;
    source->indentstack = 0;
    source->skip = 0;
    source->definehash = (define_s **)GetClearedMemory(0x1000u);
    PC_AddGlobalDefinesToSource(source);
    return source;
}

void __cdecl FreeSource(source_s *source)
{
    script_s *script; // [esp+0h] [ebp-14h]
    define_s *define; // [esp+4h] [ebp-10h]
    indent_s *indent; // [esp+8h] [ebp-Ch]
    token_s *token; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    while ( source->scriptstack )
    {
        script = source->scriptstack;
        source->scriptstack = script->next;
        FreeScript(script);
    }
    while ( source->tokens )
    {
        token = source->tokens;
        source->tokens = token->next;
        PC_FreeToken(token);
    }
    for ( i = 0; i < 1024; ++i )
    {
        while ( source->definehash[i] )
        {
            define = source->definehash[i];
            source->definehash[i] = define->hashnext;
            PC_FreeDefine(define);
        }
    }
    while ( source->indentstack )
    {
        indent = source->indentstack;
        source->indentstack = indent->next;
        FreeMemory((char *)indent);
    }
    if ( source->definehash )
        FreeMemory((char *)source->definehash);
    FreeMemory(source->filename);
}

int __cdecl PC_LoadSourceHandle(char *filename, const char **builtinDefines)
{
    source_s *source; // [esp+0h] [ebp-Ch]
    int defineIter; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    for ( i = 1; i < 64 && sourceFiles[i]; ++i )
        ;
    if ( i >= 64 )
        return 0;
    source = LoadSourceFile(filename);
    if ( !source )
        return 0;
    if ( builtinDefines )
    {
        for ( defineIter = 0; builtinDefines[defineIter]; ++defineIter )
            PC_AddDefine(source, (char *)builtinDefines[defineIter]);
    }
    sourceFiles[i] = source;
    return i;
}

int __cdecl PC_FreeSourceHandle(int handle)
{
    if ( handle < 1 || handle >= 64 )
        return 0;
    if ( !sourceFiles[handle] )
        return 0;
    FreeSource(sourceFiles[handle]);
    sourceFiles[handle] = 0;
    return 1;
}

int __cdecl PC_ReadTokenHandle(int handle, pc_token_s *pc_token)
{
    float floatvalue; // xmm0_4
    char v4; // [esp+3h] [ebp-449h]
    char *string; // [esp+8h] [ebp-444h]
    token_s *p_token; // [esp+Ch] [ebp-440h]
    int ret; // [esp+10h] [ebp-43Ch]
    token_s token; // [esp+14h] [ebp-438h] BYREF

    if ( handle < 1 || handle >= 64 )
        return 0;
    if ( !sourceFiles[handle] )
        return 0;
    ret = PC_ReadToken(sourceFiles[handle], &token);
    p_token = &token;
    string = pc_token->string;
    do
    {
        v4 = p_token->string[0];
        *string = p_token->string[0];
        p_token = (token_s *)((char *)p_token + 1);
        ++string;
    }
    while ( v4 );
    pc_token->type = token.type;
    pc_token->subtype = token.subtype;
    pc_token->intvalue = token.intvalue;
    floatvalue = token.floatvalue;
    pc_token->floatvalue = floatvalue;
    if ( pc_token->type == 1 )
        StripDoubleQuotes(pc_token->string);
    return ret;
}

int __cdecl PC_ReadLineHandle(int handle, pc_token_s *pc_token)
{
    float floatvalue; // xmm0_4
    char v4; // [esp+3h] [ebp-449h]
    char *string; // [esp+8h] [ebp-444h]
    token_s *p_token; // [esp+Ch] [ebp-440h]
    int ret; // [esp+10h] [ebp-43Ch]
    token_s token; // [esp+14h] [ebp-438h] BYREF

    if ( handle < 1 || handle >= 64 )
        return 0;
    if ( !sourceFiles[handle] )
        return 0;
    ret = PC_ReadLine(sourceFiles[handle], &token, 1);
    p_token = &token;
    string = pc_token->string;
    do
    {
        v4 = p_token->string[0];
        *string = p_token->string[0];
        p_token = (token_s *)((char *)p_token + 1);
        ++string;
    }
    while ( v4 );
    pc_token->type = token.type;
    pc_token->subtype = token.subtype;
    pc_token->intvalue = token.intvalue;
    floatvalue = token.floatvalue;
    pc_token->floatvalue = floatvalue;
    if ( pc_token->type == 1 )
        StripDoubleQuotes(pc_token->string);
    return ret;
}

void __cdecl PC_UnreadLastTokenHandle(int handle)
{
    if ( handle >= 1 && handle < 64 )
    {
        if ( sourceFiles[handle] )
            PC_UnreadLastToken(sourceFiles[handle]);
    }
}

int __cdecl PC_SourceFileAndLine(int handle, char *filename, int *line)
{
    char v4; // cl
    char v5; // dl
    char *v6; // [esp+8h] [ebp-1Ch]
    source_s *v7; // [esp+Ch] [ebp-18h]
    char *v8; // [esp+18h] [ebp-Ch]
    script_s *v9; // [esp+1Ch] [ebp-8h]
    script_s *scriptstack; // [esp+20h] [ebp-4h]

    if ( handle < 1 || handle >= 64 )
        return 0;
    if ( !sourceFiles[handle] )
        return 0;
    scriptstack = sourceFiles[handle]->scriptstack;
    if ( scriptstack )
    {
        v9 = sourceFiles[handle]->scriptstack;
        v8 = filename;
        do
        {
            v4 = v9->filename[0];
            *v8 = v9->filename[0];
            v9 = (script_s *)((char *)v9 + 1);
            ++v8;
        }
        while ( v4 );
        *line = scriptstack->line;
    }
    else
    {
        v7 = sourceFiles[handle];
        v6 = filename;
        do
        {
            v5 = v7->filename[0];
            *v6 = v7->filename[0];
            v7 = (source_s *)((char *)v7 + 1);
            ++v6;
        }
        while ( v5 );
        *line = 0;
    }
    return 1;
}

