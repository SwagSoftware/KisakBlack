#include "l_script.h"
#include "l_memory.h"

#include <cstring>
#include <qcommon/common.h>
#include <universal/com_files.h>
#include <universal/q_parse.h>

punctuation_s default_punctuations[53] =
{
  { (char*)">>=", 1, NULL },
  { (char*)"<<=", 2, NULL },
  { (char*)"...", 3, NULL },
  { (char*)"##", 4, NULL },
  { (char*)"&&", 5, NULL },
  { (char*)"||", 6, NULL },
  { (char*)">=", 7, NULL },
  { (char*)"<=", 8, NULL },
  { (char*)"==", 9, NULL },
  { (char*)"!=", 10, NULL },
  { (char*)"*=", 11, NULL },
  { (char*)"/=", 12, NULL },
  { (char*)"%=", 13, NULL },
  { (char*)"+=", 14, NULL },
  { (char*)"-=", 15, NULL },
  { (char*)"++", 16, NULL },
  { (char*)"--", 17, NULL },
  { (char*)"&=", 18, NULL },
  { (char*)"|=", 19, NULL },
  { (char*)"^=", 20, NULL },
  { (char*)">>", 21, NULL },
  { (char*)"<<", 22, NULL },
  { (char*)"->", 23, NULL },
  { (char*)"::", 24, NULL },
  { (char*)".*", 25, NULL },
  { (char*)"*", 26, NULL },
  { (char*)"/", 27, NULL },
  { (char*)"%", 28, NULL },
  { (char*)"+", 29, NULL },
  { (char*)"-", 30, NULL },
  { (char*)"=", 31, NULL },
  { (char*)"&", 32, NULL },
  { (char*)"|", 33, NULL },
  { (char*)"^", 34, NULL },
  { (char*)"~", 35, NULL },
  { (char*)"!", 36, NULL },
  { (char*)">", 37, NULL },
  { (char*)"<", 38, NULL },
  { (char*)".", 39, NULL },
  { (char*)",", 40, NULL },
  { (char*)";", 41, NULL },
  { (char*)":", 42, NULL },
  { (char*)"?", 43, NULL },
  { (char*)"(", 44, NULL },
  { (char*)")", 45, NULL },
  { (char*)"{", 46, NULL },
  { (char*)"}", 47, NULL },
  { (char*)"[", 48, NULL },
  { (char*)"]", 49, NULL },
  { (char*)"\\", 50, NULL },
  { (char*)"#", 51, NULL },
  { (char*)"$", 52, NULL },
  { NULL, 0, NULL }
};



void __cdecl PS_CreatePunctuationTable(script_s *script, punctuation_s *punctuations)
{
    punctuation_s *lastp; // [esp+20h] [ebp-10h]
    int i; // [esp+24h] [ebp-Ch]
    punctuation_s *newp; // [esp+28h] [ebp-8h]
    punctuation_s *p; // [esp+2Ch] [ebp-4h]

    if ( !script->punctuationtable )
        script->punctuationtable = (punctuation_s **)GetMemory(0x400u);
    memset(script->punctuationtable, 0, 0x400u);
    for ( i = 0; punctuations[i].p; ++i )
    {
        newp = &punctuations[i];
        lastp = 0;
        for ( p = script->punctuationtable[*newp->p]; p; p = p->next )
        {
            if ( strlen(p->p) < strlen(newp->p) )
            {
                newp->next = p;
                if ( lastp )
                    lastp->next = newp;
                else
                    script->punctuationtable[*newp->p] = newp;
                break;
            }
            lastp = p;
        }
        if ( !p )
        {
            newp->next = 0;
            if ( lastp )
                lastp->next = newp;
            else
                script->punctuationtable[*newp->p] = newp;
        }
    }
}

void ScriptError(script_s *script, const char *str, ...)
{
    char text[1028]; // [esp+4h] [ebp-408h] BYREF
    va_list va; // [esp+41Ch] [ebp+10h] BYREF

    va_start(va, str);
    if ( (script->flags & 1) == 0 )
    {
        _vsnprintf(text, 0x400u, str, va);
        Com_PrintError(24, "Error: file %s, line %d: %s\n", script->filename, script->line, text);
    }
}

void ScriptWarning(script_s *script, const char *str, ...)
{
    char text[1028]; // [esp+4h] [ebp-408h] BYREF
    va_list va; // [esp+41Ch] [ebp+10h] BYREF

    va_start(va, str);
    if ( (script->flags & 2) == 0 )
    {
        _vsnprintf(text, 0x400u, str, va);
        Com_PrintWarning(24, "Warning: file %s, line %d: %s\n", script->filename, script->line, text);
    }
}

int __cdecl PS_ReadWhiteSpace(script_s *script)
{
    while ( 1 )
    {
        while ( *script->script_p <= 32 )
        {
            if ( !*script->script_p )
                return 0;
            if ( *script->script_p == 10 )
                ++script->line;
            ++script->script_p;
        }
        if ( *script->script_p != 47 )
            break;
        if ( script->script_p[1] == 47 )
        {
            ++script->script_p;
            do
            {
                if ( !*++script->script_p )
                    return 0;
            }
            while ( *script->script_p != 10 );
            ++script->line;
            if ( !*++script->script_p )
                return 0;
        }
        else
        {
            if ( script->script_p[1] != 42 )
                return 1;
            ++script->script_p;
            do
            {
                if ( !*++script->script_p )
                    return 0;
                if ( *script->script_p == 10 )
                    ++script->line;
            }
            while ( *script->script_p != 42 || script->script_p[1] != 47 );
            if ( !*++script->script_p )
                return 0;
            if ( !*++script->script_p )
                return 0;
        }
    }
    return 1;
}

int __cdecl PS_ReadEscapeCharacter(script_s *script, char *ch)
{
    char c; // [esp+4h] [ebp-Ch]
    int ca; // [esp+4h] [ebp-Ch]
    int cb; // [esp+4h] [ebp-Ch]
    int cc; // [esp+4h] [ebp-Ch]
    int val; // [esp+8h] [ebp-8h]
    int vala; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]
    int ia; // [esp+Ch] [ebp-4h]

    switch ( *++script->script_p )
    {
        case '"':
            c = 34;
            goto LABEL_38;
        case '\'':
            c = 39;
            goto LABEL_38;
        case '?':
            c = 63;
            goto LABEL_38;
        case '\\':
            c = 92;
            goto LABEL_38;
        case 'a':
            c = 7;
            goto LABEL_38;
        case 'b':
            c = 8;
            goto LABEL_38;
        case 'f':
            c = 12;
            goto LABEL_38;
        case 'n':
            c = 10;
            goto LABEL_38;
        case 'r':
            c = 13;
            goto LABEL_38;
        case 't':
            c = 9;
            goto LABEL_38;
        case 'v':
            c = 11;
            goto LABEL_38;
        case 'x':
            ++script->script_p;
            i = 0;
            val = 0;
            break;
        default:
            if ( *script->script_p < 48 || *script->script_p > 57 )
                ScriptError(script, "unknown escape char");
            ia = 0;
            vala = 0;
            while ( 1 )
            {
                cc = *script->script_p;
                if ( cc < 48 || cc > 57 )
                    break;
                vala = cc - 48 + 10 * vala;
                ++ia;
                ++script->script_p;
            }
            --script->script_p;
            if ( vala > 255 )
            {
                ScriptWarning(script, "too large value in escape character");
                LOBYTE(vala) = -1;
            }
            c = vala;
            goto LABEL_38;
    }
    while ( 1 )
    {
        ca = *script->script_p;
        if ( ca >= 48 && ca <= 57 )
        {
            cb = ca - 48;
            goto LABEL_24;
        }
        if ( ca >= 65 && ca <= 90 )
        {
            cb = ca - 55;
            goto LABEL_24;
        }
        if ( ca < 97 || ca > 122 )
            break;
        cb = ca - 87;
LABEL_24:
        val = cb + 16 * val;
        ++i;
        ++script->script_p;
    }
    --script->script_p;
    if ( val > 255 )
    {
        ScriptWarning(script, "too large value in escape character");
        LOBYTE(val) = -1;
    }
    c = val;
LABEL_38:
    ++script->script_p;
    *ch = c;
    return 1;
}

int __cdecl PS_ReadString(script_s *script, token_s *token, int quote)
{
    int tmpline; // [esp+0h] [ebp-Ch]
    int len; // [esp+4h] [ebp-8h]
    int lena; // [esp+4h] [ebp-8h]
    char *tmpscript_p; // [esp+8h] [ebp-4h]

    switch ( quote )
    {
        case '"':
            token->type = 1;
            break;
        case '\'':
            token->type = 2;
            break;
        case '#':
            token->type = 6;
            ++script->script_p;
            quote = 34;
            break;
    }
    if ( token->type == 6 )
        token->string[0] = -1;
    else
        token->string[0] = *script->script_p;
    len = 1;
    ++script->script_p;
    while ( 1 )
    {
        while ( 1 )
        {
            while ( 1 )
            {
                if ( len >= 1022 )
                {
                    ScriptError(script, "string longer than MAX_TOKEN = %d", 1024);
                    return 0;
                }
                if ( *script->script_p != 92 || (script->flags & 8) != 0 )
                    break;
                if ( !PS_ReadEscapeCharacter(script, &token->string[len]) )
                {
                    token->string[len] = 0;
                    return 0;
                }
                ++len;
            }
            if ( *script->script_p == quote )
                break;
            if ( !*script->script_p )
            {
                token->string[len] = 0;
                ScriptError(script, "missing trailing quote");
                return 0;
            }
            if ( *script->script_p == 10 )
            {
                token->string[len] = 0;
                ScriptError(script, "newline inside string %s", token->string);
                return 0;
            }
            token->string[len++] = *script->script_p++;
        }
        ++script->script_p;
        if ( (script->flags & 4) != 0 )
            break;
        tmpscript_p = script->script_p;
        tmpline = script->line;
        if ( !PS_ReadWhiteSpace(script) )
        {
            script->script_p = tmpscript_p;
            script->line = tmpline;
            break;
        }
        if ( *script->script_p != quote )
        {
            script->script_p = tmpscript_p;
            script->line = tmpline;
            break;
        }
        ++script->script_p;
    }
    if ( token->type == 6 )
        token->string[len] = -1;
    else
        token->string[len] = quote;
    lena = len + 1;
    token->string[lena] = 0;
    token->subtype = lena;
    return 1;
}

int __cdecl PS_ReadName(script_s *script, token_s *token)
{
    char c; // [esp+3h] [ebp-5h]
    int len; // [esp+4h] [ebp-4h]

    len = 0;
    token->type = 4;
    do
    {
        token->string[len++] = *script->script_p++;
        if ( len >= 1024 )
        {
            ScriptError(script, "name longer than MAX_TOKEN = %d", 1024);
            return 0;
        }
        c = *script->script_p;
    }
    while ( c >= 97 && c <= 122 || c >= 65 && c <= 90 || c >= 48 && c <= 57 || c == 95 );
    token->string[len] = 0;
    token->subtype = len;
    return 1;
}

void __cdecl NumberValue(char *string, __int16 subtype, unsigned int *intvalue, long double *floatvalue)
{
    unsigned int dotfound; // [esp+3Ch] [ebp-4h]
    char *stringa; // [esp+48h] [ebp+8h]
    char *stringb; // [esp+48h] [ebp+8h]
    char *stringc; // [esp+48h] [ebp+8h]

    dotfound = 0;
    *intvalue = 0;
    *floatvalue = 0.0;
    if ( (subtype & 0x800) != 0 )
    {
        while ( *string )
        {
            if ( *string == 46 )
            {
                if ( dotfound )
                    return;
                dotfound = 10;
                ++string;
            }
            if ( dotfound )
            {
                *floatvalue = (double)(*string - 48) / (double)dotfound + *floatvalue;
                dotfound *= 10;
            }
            else
            {
                *floatvalue = *floatvalue * 10.0 + (double)(*string - 48);
            }
            ++string;
        }
        *intvalue = (__int64)*floatvalue;
    }
    else if ( (subtype & 8) != 0 )
    {
        while ( *string )
            *intvalue = 10 * *intvalue + *string++ - 48;
        *floatvalue = (double)*intvalue;
    }
    else if ( (subtype & 0x100) != 0 )
    {
        for ( stringa = string + 2; *stringa; ++stringa )
        {
            *intvalue *= 16;
            if ( *stringa < 97 || *stringa > 102 )
            {
                if ( *stringa < 65 || *stringa > 70 )
                    *intvalue = *stringa + *intvalue - 48;
                else
                    *intvalue = *stringa + *intvalue - 55;
            }
            else
            {
                *intvalue = *stringa + *intvalue - 87;
            }
        }
        *floatvalue = (double)*intvalue;
    }
    else if ( (subtype & 0x200) != 0 )
    {
        for ( stringb = string + 1; *stringb; ++stringb )
            *intvalue = *stringb + 8 * *intvalue - 48;
        *floatvalue = (double)*intvalue;
    }
    else if ( (subtype & 0x400) != 0 )
    {
        for ( stringc = string + 2; *stringc; ++stringc )
            *intvalue = *stringc + 2 * *intvalue - 48;
        *floatvalue = (double)*intvalue;
    }
}

int __cdecl PS_ReadNumber(script_s *script, token_s *token)
{
    int v3; // edx
    bool octal; // [esp+0h] [ebp-14h]
    char c; // [esp+7h] [ebp-Dh]
    char ca; // [esp+7h] [ebp-Dh]
    char cb; // [esp+7h] [ebp-Dh]
    char cc; // [esp+7h] [ebp-Dh]
    int len; // [esp+8h] [ebp-Ch]
    int i; // [esp+Ch] [ebp-8h]
    int dot; // [esp+10h] [ebp-4h]

    len = 0;
    token->type = 3;
    if ( *script->script_p == 48 && (script->script_p[1] == 120 || script->script_p[1] == 88) )
    {
        token->string[0] = *script->script_p++;
        token->string[1] = *script->script_p;
        len = 2;
        for ( c = *++script->script_p; c >= 48 && c <= 57 || c >= 97 && c <= 102 || c == 65; c = *script->script_p )
        {
            token->string[len++] = *script->script_p++;
            if ( len >= 1024 )
            {
                ScriptError(script, "hexadecimal number longer than MAX_TOKEN = %d", 1024);
                return 0;
            }
        }
        token->subtype |= 0x100u;
        goto LABEL_41;
    }
    if ( *script->script_p == 48 && (script->script_p[1] == 98 || script->script_p[1] == 66) )
    {
        token->string[0] = *script->script_p++;
        token->string[1] = *script->script_p;
        len = 2;
        for ( ca = *++script->script_p; ca == 48 || ca == 49; ca = *script->script_p )
        {
            token->string[len++] = *script->script_p++;
            if ( len >= 1024 )
            {
                ScriptError(script, "binary number longer than MAX_TOKEN = %d", 1024);
                return 0;
            }
        }
        token->subtype |= 0x400u;
        goto LABEL_41;
    }
    dot = 0;
    octal = *script->script_p == 48;
    while ( 1 )
    {
        cb = *script->script_p;
        if ( cb != 46 )
            break;
        dot = 1;
LABEL_33:
        token->string[len++] = *script->script_p++;
        if ( len >= 1023 )
        {
            ScriptError(script, "number longer than MAX_TOKEN = %d", 1024);
            return 0;
        }
    }
    if ( cb == 56 || cb == 57 )
    {
        octal = 0;
        goto LABEL_33;
    }
    if ( cb >= 48 && cb <= 57 )
        goto LABEL_33;
    if ( octal )
        v3 = token->subtype | 0x200;
    else
        v3 = token->subtype | 8;
    token->subtype = v3;
    if ( dot )
        token->subtype |= 0x800u;
LABEL_41:
    for ( i = 0; i < 2; ++i )
    {
        cc = *script->script_p;
        if ( (cc == 108 || cc == 76) && (token->subtype & 0x2000) == 0 )
        {
            ++script->script_p;
            token->subtype |= 0x2000u;
        }
        else if ( (cc == 117 || cc == 85) && (token->subtype & 0x4800) == 0 )
        {
            ++script->script_p;
            token->subtype |= 0x4000u;
        }
    }
    token->string[len] = 0;
    NumberValue(token->string, token->subtype, &token->intvalue, &token->floatvalue);
    if ( (token->subtype & 0x800) == 0 )
        token->subtype |= 0x1000u;
    return 1;
}

int __cdecl PS_ReadPunctuation(script_s *script, token_s *token)
{
    punctuation_s *punc; // [esp+10h] [ebp-Ch]
    unsigned int len; // [esp+14h] [ebp-8h]
    char *p; // [esp+18h] [ebp-4h]

    for ( punc = script->punctuationtable[*script->script_p]; punc; punc = punc->next )
    {
        p = punc->p;
        len = strlen(punc->p);
        if ( &script->script_p[len] <= script->end_p && !strncmp(script->script_p, p, len) )
        {
            strncpy((char*)token, p, 0x400u);
            script->script_p += len;
            token->type = 5;
            token->subtype = punc->n;
            return 1;
        }
    }
    return 0;
}

int __cdecl PS_ReadPrimitive(script_s *script, token_s *token)
{
    int len; // [esp+8h] [ebp-4h]

    len = 0;
    while ( *script->script_p > 32 && *script->script_p != 59 )
    {
        if ( len >= 1024 )
        {
            ScriptError(script, "primitive token longer than MAX_TOKEN = %d", 1024);
            return 0;
        }
        token->string[len++] = *script->script_p++;
    }
    token->string[len] = 0;
    memcpy(&script->token, token, sizeof(script->token));
    return 1;
}

int __cdecl PS_ReadToken(script_s *script, token_s *token)
{
    if ( script->tokenavailable )
    {
        script->tokenavailable = 0;
        memcpy(token, &script->token, sizeof(token_s));
        return 1;
    }
    script->lastscript_p = script->script_p;
    script->lastline = script->line;
    memset(token, 0, sizeof(token_s));
    script->whitespace_p = script->script_p;
    token->whitespace_p = script->script_p;
    if ( !PS_ReadWhiteSpace(script) )
        return 0;
    script->endwhitespace_p = script->script_p;
    token->endwhitespace_p = script->script_p;
    token->line = script->line;
    token->linescrossed = script->line - script->lastline;
    if ( *script->script_p == 34 )
    {
        if ( !PS_ReadString(script, token, 34) )
            return 0;
    }
    else if ( *script->script_p == 35 && script->script_p[1] == 34 )
    {
        if ( !PS_ReadString(script, token, 35) )
            return 0;
    }
    else if ( *script->script_p == 39 )
    {
        if ( !PS_ReadString(script, token, 39) )
            return 0;
    }
    else if ( *script->script_p >= 48 && *script->script_p <= 57
                 || *script->script_p == 46 && script->script_p[1] >= 48 && script->script_p[1] <= 57 )
    {
        if ( !PS_ReadNumber(script, token) )
            return 0;
    }
    else
    {
        if ( (script->flags & 0x10) != 0 )
            return PS_ReadPrimitive(script, token);
        if ( *script->script_p >= 97 && *script->script_p <= 122
            || *script->script_p >= 65 && *script->script_p <= 90
            || *script->script_p == 95 )
        {
            if ( !PS_ReadName(script, token) )
                return 0;
        }
        else if ( !PS_ReadPunctuation(script, token) )
        {
            ScriptError(script, "can't read token");
            return 0;
        }
    }
    memcpy(&script->token, token, sizeof(script->token));
    return 1;
}

void __cdecl StripDoubleQuotes(char *string)
{
    char v1; // dl
    char *v2; // [esp+28h] [ebp-8h]
    char *v3; // [esp+2Ch] [ebp-4h]

    if ( *string == 34 )
    {
        v3 = string + 1;
        v2 = string;
        do
        {
            v1 = *v3;
            *v2++ = *v3++;
        }
        while ( v1 );
    }
    if ( string[strlen(string) - 1] == 34 )
        string[strlen(string) - 1] = 0;
}

bool __cdecl EndOfScript(script_s *script)
{
    return script->script_p >= script->end_p;
}

script_s *__cdecl LoadScriptFile(const char *filename)
{
    char v2; // [esp+3h] [ebp-61h]
    script_s *v3; // [esp+8h] [ebp-5Ch]
    const char *v4; // [esp+Ch] [ebp-58h]
    script_s *buffer; // [esp+10h] [ebp-54h]
    int fp; // [esp+18h] [ebp-4Ch] BYREF
    char pathname[64]; // [esp+1Ch] [ebp-48h] BYREF
    int length; // [esp+60h] [ebp-4h]

    Com_sprintf(pathname, 0x40u, "%s", filename);
    length = FS_FOpenFileRead(pathname, &fp);
    if ( !fp )
        return 0;
    buffer = (script_s *)GetClearedMemory(length + 1201);
    v4 = filename;
    v3 = buffer;
    do
    {
        v2 = *v4;
        v3->filename[0] = *v4++;
        v3 = (script_s *)((char *)v3 + 1);
    }
    while ( v2 );
    buffer->buffer = buffer[1].filename;
    buffer->buffer[length] = 0;
    buffer->length = length;
    buffer->script_p = buffer->buffer;
    buffer->lastscript_p = buffer->buffer;
    buffer->end_p = &buffer->buffer[length];
    buffer->tokenavailable = 0;
    buffer->line = 1;
    buffer->lastline = 1;
    SetScriptPunctuations(buffer);
    FS_Read((unsigned __int8 *)buffer->buffer, length, fp);
    FS_FCloseFile(fp);
    buffer->length = Com_Compress(buffer->buffer);
    return buffer;
}

void __cdecl SetScriptPunctuations(script_s *script)
{
    PS_CreatePunctuationTable(script, default_punctuations);
    script->punctuations = default_punctuations;
}

script_s *__cdecl LoadScriptMemory(char *ptr, unsigned int length, const char *name)
{
    char v4; // [esp+3h] [ebp-15h]
    script_s *v5; // [esp+8h] [ebp-10h]
    script_s *buffer; // [esp+10h] [ebp-8h]

    buffer = (script_s *)GetClearedMemory(length + 1201);
    v5 = buffer;
    do
    {
        v4 = *name;
        v5->filename[0] = *name++;
        v5 = (script_s *)((char *)v5 + 1);
    }
    while ( v4 );
    buffer->buffer = buffer[1].filename;
    buffer->buffer[length] = 0;
    buffer->length = length;
    buffer->script_p = buffer->buffer;
    buffer->lastscript_p = buffer->buffer;
    buffer->end_p = &buffer->buffer[length];
    buffer->tokenavailable = 0;
    buffer->line = 1;
    buffer->lastline = 1;
    SetScriptPunctuations(buffer);
    memcpy((unsigned __int8 *)buffer->buffer, (unsigned __int8 *)ptr, length);
    return buffer;
}

void __cdecl FreeScript(script_s *script)
{
    if ( script->punctuationtable )
        FreeMemory((char *)script->punctuationtable);
    FreeMemory(script->filename);
}

