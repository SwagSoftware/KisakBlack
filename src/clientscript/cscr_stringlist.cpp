#include "cscr_stringlist.h"
#include <universal/assertive.h>
#include "cscr_memorytree.h"
#include <ctype.h>
#include <win32/win_common.h>
#include <cstring>
#include "cscr_instance.h"
#include <qcommon/common.h>

#include <Windows.h>
#include "cscr_variable.h"

// server: 0x8000
// client: 0x4000
HashEntry gScrStringHashTable[0x8000 + 0x4000 /*49152*/];

scrStringGlob_t gScrStringGlob[2]
{
    { &gScrStringHashTable[0] },        // SCRIPTINSTANCE_SERVER
    { &gScrStringHashTable[0x8000] }    // SCRIPTINSTANCE_CLIENT
};

scrStringDebugGlob_t *gScrStringDebugGlob[2];

char *__cdecl SL_ConvertToString(unsigned int stringValue, scriptInstance_t inst)
{
    if ( stringValue
        && gScrStringDebugGlob[inst]
        && !gScrStringDebugGlob[inst]->refCount[stringValue]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                    171,
                    0,
                    "%s\n\t(stringValue) = %i",
                    "(!stringValue || !gScrStringDebugGlob[inst] || gScrStringDebugGlob[inst]->refCount[stringValue])",
                    stringValue) )
    {
        __debugbreak();
    }
    if ( stringValue )
        return GetRefString(inst, stringValue)->str;
    else
        return 0;
}

RefString *__cdecl GetRefString(scriptInstance_t inst, unsigned int stringValue)
{
    if ( !stringValue
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                    153,
                    0,
                    "%s",
                    "stringValue") )
    {
        __debugbreak();
    }
    if ( 16 * stringValue >= 0x100000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                    154,
                    0,
                    "%s",
                    "stringValue * MT_NODE_SIZE < MT_SIZE") )
    {
        __debugbreak();
    }
    return (RefString *)&gScrMemTreePub[inst].mt_buffer[16 * stringValue];
}

const char *__cdecl SL_DebugConvertToString(unsigned int stringValue, scriptInstance_t inst)
{
    int len; // [esp+0h] [ebp-10h]
    int i; // [esp+8h] [ebp-8h]
    RefString *refString; // [esp+Ch] [ebp-4h]

    if ( !stringValue )
        return "<NULL>";
    refString = GetRefString(inst, stringValue);
    len = (unsigned __int8)(HIBYTE(refString->data) - 1);
    if ( refString->str[len] )
        return "<BINARY>";
    for ( i = 0; i < len; ++i )
    {
        if ( !isprint((unsigned __int8)refString->str[i]) )
            return "<BINARY>";
    }
    return refString->str;
}

int __cdecl SL_GetStringLen(scriptInstance_t inst, unsigned int stringValue)
{
    RefString *refString; // [esp+0h] [ebp-4h]

    if ( !stringValue
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                    232,
                    0,
                    "%s",
                    "stringValue") )
    {
        __debugbreak();
    }
    refString = GetRefString(inst, stringValue);
    return SL_GetRefStringLen(refString);
}

int SL_GetRefStringLen(RefString *refString)
{
    int len = refString->byteLen - 1;

    while (refString->str[len])
        len += 256;

    return len;
}

RefString *__cdecl GetRefString_0(scriptInstance_t inst, char *str)
{
    if ((str < gScrMemTreePub[inst].mt_buffer || str >= gScrMemTreePub[inst].mt_buffer + 0x100000)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
            162,
            0,
            "%s",
            "str >= gScrMemTreePub[inst].mt_buffer && str < gScrMemTreePub[inst].mt_buffer + MT_SIZE"))
    {
        __debugbreak();
    }
    return (RefString *)(str - 4);
}

int __cdecl SL_ConvertFromString(scriptInstance_t inst, char *str)
{
    RefString *RefString_0; // eax

    if ( !str
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp", 246, 0, "%s", "str") )
    {
        __debugbreak();
    }
    RefString_0 = GetRefString_0(inst, str);
    return SL_ConvertFromRefString(inst, RefString_0);
}

int __cdecl SL_ConvertFromRefString(scriptInstance_t inst, RefString *refString)
{
    return ((char *)refString - gScrMemTreePub[inst].mt_buffer) / 16;
}

int __cdecl Scr_GetHashCode(scriptInstance_t inst, const char *str, unsigned int len)
{
    return GetHashCode(inst, str, len);
}

int __cdecl GetHashCode(scriptInstance_t inst, const char *str, unsigned int len)
{
    unsigned int hash; // [esp+4h] [ebp-8h]

    if ( len >= 0x100 )
        return (len >> 2) % (inst != SCRIPTINSTANCE_SERVER ? 0x3FFF : 0x7FFF) + 1;
    hash = 0;
    while ( len )
    {
        hash = *str++ + 31 * hash;
        --len;
    }
    return hash % (inst != SCRIPTINSTANCE_SERVER ? 0x3FFF : 0x7FFF) + 1;
}

void __cdecl SL_Init(scriptInstance_t inst)
{
    unsigned int hash; // [esp+0h] [ebp-Ch]
    HashEntry *entry; // [esp+4h] [ebp-8h]
    unsigned int prev; // [esp+8h] [ebp-4h]

    if (gScrStringGlob[inst].inited
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
            292,
            0,
            "%s",
            "!gScrStringGlob[inst].inited"))
    {
        __debugbreak();
    }
    MT_Init(inst);
    Sys_EnterCriticalSection(CRITSECT_SCRIPT_STRING);
    gScrStringGlob[inst].hashTable->status_next = 0;
    prev = 0;
    for (hash = 1; hash < (inst != SCRIPTINSTANCE_SERVER ? 0x4000 : 0x8000); ++hash)
    {
        if ((hash & 0x30000) != 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                313,
                0,
                "%s",
                "!(hash & HASH_STAT_MASK)"))
        {
            __debugbreak();
        }
        entry = &gScrStringGlob[inst].hashTable[hash];
        entry->status_next = 0;
        gScrStringGlob[inst].hashTable[prev].status_next |= hash;
        entry->u.prev = prev;
        prev = hash;
    }
    if ((unsigned __int16)gScrStringGlob[inst].hashTable[prev].status_next
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
            321,
            0,
            "%s",
            "!(gScrStringGlob[inst].hashTable[prev].status_next & HASH_NEXT_MASK)"))
    {
        __debugbreak();
    }
    gScrStringGlob[inst].hashTable->u.prev = prev;
    SL_InitCheckLeaks(inst);
    gScrStringGlob[inst].indentLevel = 0;
    memset(gScrStringGlob[inst].stringsUsed, 0, sizeof(gScrStringGlob[inst].stringsUsed));
    gScrStringGlob[inst].inited = 1;
    Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
}

void __cdecl SL_InitCheckLeaks(scriptInstance_t inst)
{
    Scr_ResetScrStringDebugGlob(inst);
    Com_Memset((unsigned int *)gScrStringDebugGlob[inst], 0, 0x40000);
    gScrStringDebugGlob[inst]->totalRefCount = 0;
}

void __cdecl SL_Shutdown(scriptInstance_t inst)
{
    if (gScrStringGlob[inst].inited)
    {
        gScrStringGlob[inst].inited = 0;
        SL_CheckLeaks(inst);
    }
}

void __cdecl SL_CheckLeaks(scriptInstance_t inst)
{
    volatile int v2; // [esp-8h] [ebp-Ch]
    unsigned int i; // [esp+0h] [ebp-4h]

    if ( gScrStringDebugGlob[inst] )
    {
        if ( !gScrStringDebugGlob[inst]->ignoreLeaks )
        {
            for ( i = 1; i < 0x10000; ++i )
            {
                if ( gScrStringDebugGlob[inst]->refCount[i] )
                {
                    v2 = gScrStringDebugGlob[inst]->refCount[i];
                    Com_PrintError(1, "leaked string = '%s', refcount = %i, i = %i\n", SL_DebugConvertToString(i, inst), v2, i);
                }
            }
            if ( gScrStringDebugGlob[inst]->totalRefCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                            142,
                            0,
                            "%s\n\t(gScrStringDebugGlob[inst]->totalRefCount) = %i",
                            "(!gScrStringDebugGlob[inst]->totalRefCount)",
                            gScrStringDebugGlob[inst]->totalRefCount) )
            {
                __debugbreak();
            }
        }
        Scr_ClearScrStringDebugGlob(inst);
    }
}

unsigned int __cdecl SL_FindStringOfSize(scriptInstance_t inst, const char *str, unsigned int len)
{
    unsigned int stringValue; // [esp+8h] [ebp-20h]
    HashEntry *entry; // [esp+Ch] [ebp-1Ch]
    unsigned int hash; // [esp+10h] [ebp-18h]
    unsigned int newIndex; // [esp+14h] [ebp-14h]
    RefString *refStr; // [esp+1Ch] [ebp-Ch]
    RefString *refStra; // [esp+1Ch] [ebp-Ch]
    unsigned int prev; // [esp+20h] [ebp-8h]
    HashEntry *newEntry; // [esp+24h] [ebp-4h]

    if ( !str
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp", 366, 0, "%s", "str") )
    {
        __debugbreak();
    }
    hash = GetHashCode(inst, str, len);
    Sys_EnterCriticalSection(CRITSECT_SCRIPT_STRING);
    entry = &gScrStringGlob[inst].hashTable[hash];
    if ( (entry->status_next & 0x30000) != 0x20000 )
    {
LABEL_32:
        Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
        return 0;
    }
    refStr = GetRefString(inst, entry->u.prev);
    //if ( (unsigned __int8)HIBYTE(*(unsigned int *)&refStr->0) != (unsigned __int8)len || memcmp(refStr->str, str, len) )
    if ( refStr->byteLen != (unsigned __int8)len || memcmp(refStr->str, str, len) )
    {
        prev = hash;
        newIndex = (unsigned __int16)entry->status_next;
        for ( newEntry = &gScrStringGlob[inst].hashTable[newIndex];
                    newEntry != entry;
                    newEntry = &gScrStringGlob[inst].hashTable[newIndex] )
        {
            if ( (newEntry->status_next & 0x30000) != 0x10000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                            408,
                            0,
                            "%s",
                            "(newEntry->status_next & HASH_STAT_MASK) == HASH_STAT_MOVABLE") )
            {
                __debugbreak();
            }
            refStra = GetRefString(inst, newEntry->u.prev);
            //if ( (unsigned __int8)HIBYTE(*(unsigned int *)&refStra->0) == (unsigned __int8)len && !memcmp(refStra->str, str, len) )
            if ( refStra->byteLen == (unsigned __int8)len && !memcmp(refStra->str, str, len) )
            {
                gScrStringGlob[inst].hashTable[prev].status_next = (unsigned __int16)newEntry->status_next
                                                                                                                 | gScrStringGlob[inst].hashTable[prev].status_next & 0x30000;
                newEntry->status_next = (unsigned __int16)entry->status_next | newEntry->status_next & 0x30000;
                entry->status_next = newIndex | entry->status_next & 0x30000;
                stringValue = newEntry->u.prev;
                newEntry->u.prev = entry->u.prev;
                entry->u.prev = stringValue;
                if ( (newEntry->status_next & 0x30000) == 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                                421,
                                0,
                                "%s",
                                "(newEntry->status_next & HASH_STAT_MASK) != HASH_STAT_FREE") )
                {
                    __debugbreak();
                }
                if ( (entry->status_next & 0x30000) == 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                                422,
                                0,
                                "%s",
                                "(entry->status_next & HASH_STAT_MASK) != HASH_STAT_FREE") )
                {
                    __debugbreak();
                }
                if ( refStra->str != SL_ConvertToString(stringValue, inst)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                                424,
                                0,
                                "%s",
                                "refStr->str == SL_ConvertToString( stringValue, inst )") )
                {
                    __debugbreak();
                }
                goto END_CLEANUP;
            }
            prev = newIndex;
            newIndex = (unsigned __int16)newEntry->status_next;
        }
        goto LABEL_32;
    }
    if ( (entry->status_next & 0x30000) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                    391,
                    0,
                    "%s",
                    "(entry->status_next & HASH_STAT_MASK) != HASH_STAT_FREE") )
    {
        __debugbreak();
    }
    stringValue = entry->u.prev;
    if ( refStr->str != SL_ConvertToString(stringValue, inst)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                    394,
                    0,
                    "%s",
                    "refStr->str == SL_ConvertToString( stringValue, inst )") )
    {
        __debugbreak();
    }
END_CLEANUP:
    Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
    return stringValue;
}

unsigned int __cdecl SL_FindString(const char *str, scriptInstance_t inst)
{
    return SL_FindStringOfSize(inst, str, strlen(str) + 1);
}

unsigned int __cdecl SL_FindLowercaseString(const char *str, scriptInstance_t inst)
{
    char stra[8196]; // [esp+10h] [ebp-2010h] BYREF
    unsigned int len; // [esp+2018h] [ebp-8h]
    signed int i; // [esp+201Ch] [ebp-4h]

    len = strlen(str) + 1;
    if ( (int)len > 0x2000 )
        return 0;
    for ( i = 0; i < (int)len; ++i )
        stra[i] = tolower(str[i]);
    return SL_FindStringOfSize(inst, stra, len);
}

int __cdecl SL_IsLowercaseString(unsigned int stringValue, scriptInstance_t inst)
{
    int v2; // esi
    char *str; // [esp+4h] [ebp-4h]

    if ( !stringValue
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                    484,
                    0,
                    "%s",
                    "stringValue") )
    {
        __debugbreak();
    }
    for ( str = SL_ConvertToString(stringValue, inst); *str; ++str )
    {
        v2 = *str;
        if ( v2 != (char)tolower(v2) )
            return 0;
    }
    return 1;
}

void __cdecl SL_AddUser(unsigned int stringValue, unsigned int user, scriptInstance_t inst)
{
    RefString *RefString; // eax

    RefString = GetRefString(inst, stringValue);
    SL_AddUserInternal(inst, RefString, user);
}

void __cdecl SL_AddUserInternal(scriptInstance_t inst, RefString *refStr, unsigned int user)
{
    char *v4; // eax
    signed __int32 v5; // [esp+4h] [ebp-14h]
    int str; // [esp+14h] [ebp-4h]

    //if ( ((unsigned __int8)user & (unsigned __int8)BYTE2(*(unsigned int *)&refStr->0)) == 0 )
    if (((unsigned __int8)user & refStr->user) == 0)
    {
        str = SL_ConvertFromRefString(inst, refStr);
        if ( gScrStringDebugGlob[inst] )
        {
            if ( gScrStringDebugGlob[inst]->refCount[str] >= 0x10000 )
            {
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                                513,
                                0,
                                "%s\n\t(SL_DebugConvertToString( str, inst )) = %s",
                                "(gScrStringDebugGlob[inst]->refCount[str] < 65536)",
                    SL_DebugConvertToString(str, inst)) )
                    __debugbreak();
            }
            if ( gScrStringDebugGlob[inst]->refCount[str] < 0 )
            {
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                                514,
                                0,
                                "%s\n\t(SL_DebugConvertToString( str, inst )) = %s",
                                "(gScrStringDebugGlob[inst]->refCount[str] >= 0)",
                    SL_DebugConvertToString(str, inst)) )
                    __debugbreak();
            }
            _InterlockedExchangeAdd(&gScrStringDebugGlob[inst]->totalRefCount, 1u);
            _InterlockedExchangeAdd(&gScrStringDebugGlob[inst]->refCount[str], 1u);
        }

        //do
        //    v5 = (signed __int32)refStr->0;
        //while ( _InterlockedCompareExchange((volatile signed __int32 *)refStr, v5 | (user << 16), v5) != v5 );
        //_InterlockedExchangeAdd((volatile signed __int32 *)refStr, 1u);

        volatile int Comperand;
        do
            Comperand = refStr->data;
        while (InterlockedCompareExchange(&refStr->data, Comperand | (user << 16), Comperand) != Comperand);
        InterlockedIncrement(&refStr->data);
    }
}

unsigned int __cdecl SL_GetUser(unsigned int stringValue, scriptInstance_t inst)
{
    //return (unsigned __int8)BYTE2(*(unsigned int *)&GetRefString(inst, stringValue)->0);
    return GetRefString(inst, stringValue)->user;
}

unsigned int __cdecl SL_GetStringOfSize(
                scriptInstance_t inst,
                char *str,
                unsigned int user,
                unsigned int len,
                int type)
{
    unsigned int stringValue; // [esp+10h] [ebp-28h]
    HashEntry *entry; // [esp+14h] [ebp-24h]
    unsigned int newNext; // [esp+18h] [ebp-20h]
    unsigned int newNexta; // [esp+18h] [ebp-20h]
    unsigned int hash; // [esp+1Ch] [ebp-1Ch]
    unsigned int newIndex; // [esp+20h] [ebp-18h]
    unsigned int newIndexa; // [esp+20h] [ebp-18h]
    unsigned int newIndexb; // [esp+20h] [ebp-18h]
    RefString *refStr; // [esp+28h] [ebp-10h]
    RefString *refStr_; // [esp+28h] [ebp-10h]
    RefString *refStrb; // [esp+28h] [ebp-10h]
    unsigned int nexta; // [esp+2Ch] [ebp-Ch]
    unsigned int next; // [esp+2Ch] [ebp-Ch]
    unsigned int prev; // [esp+30h] [ebp-8h]
    unsigned int prevb; // [esp+30h] [ebp-8h]
    unsigned int preva; // [esp+30h] [ebp-8h]
    HashEntry *newEntry; // [esp+34h] [ebp-4h]
    HashEntry *newEntrya; // [esp+34h] [ebp-4h]
    HashEntry *newEntryb; // [esp+34h] [ebp-4h]

    if ( !str
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp", 552, 0, "%s", "str") )
    {
        __debugbreak();
    }
    hash = GetHashCode(inst, str, len);
    Sys_EnterCriticalSection(CRITSECT_SCRIPT_STRING);
    entry = &gScrStringGlob[inst].hashTable[hash];
    if ( (entry->status_next & 0x30000) == 0x20000 )
    {
        refStr = GetRefString(inst, entry->u.prev);
        //if ( (unsigned __int8)HIBYTE(*(unsigned int *)&refStr->0) == (unsigned __int8)len && !memcmp(refStr->str, str, len) )
        if ( refStr->byteLen == (unsigned __int8)len && !memcmp(refStr->str, str, len) )
        {
            SL_AddUserInternal(inst, refStr, user);
            if ( (entry->status_next & 0x30000) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                            580,
                            0,
                            "%s",
                            "(entry->status_next & HASH_STAT_MASK) != HASH_STAT_FREE") )
            {
                __debugbreak();
            }
            stringValue = entry->u.prev;
            if ( refStr->str != SL_ConvertToString(stringValue, inst)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                            583,
                            0,
                            "%s",
                            "refStr->str == SL_ConvertToString( stringValue, inst )") )
            {
                __debugbreak();
            }
            goto END_CLEANUP;
        }
        prev = hash;
        newIndex = (unsigned __int16)entry->status_next;

        for ( newEntry = &gScrStringGlob[inst].hashTable[newIndex];
                    newEntry != entry;
                    newEntry = &gScrStringGlob[inst].hashTable[newIndex] )
        {
            if ( (newEntry->status_next & 0x30000) != 0x10000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                            597,
                            0,
                            "%s",
                            "(newEntry->status_next & HASH_STAT_MASK) == HASH_STAT_MOVABLE") )
            {
                __debugbreak();
            }
            refStr_ = GetRefString(inst, newEntry->u.prev);
            //if ( (unsigned __int8)HIBYTE(*(unsigned int *)&refStr_->0) == (unsigned __int8)len && !memcmp(refStr_->str, str, len) )
            if ( refStr_->byteLen == (unsigned __int8)len && !memcmp(refStr_->str, str, len) )
            {
                gScrStringGlob[inst].hashTable[prev].status_next = (unsigned __int16)newEntry->status_next
                                                                                                                 | gScrStringGlob[inst].hashTable[prev].status_next & 0x30000;
                newEntry->status_next = (unsigned __int16)entry->status_next | newEntry->status_next & 0x30000;
                entry->status_next = newIndex | entry->status_next & 0x30000;
                stringValue = newEntry->u.prev;
                newEntry->u.prev = entry->u.prev;
                entry->u.prev = stringValue;
                SL_AddUserInternal(inst, refStr_, user);
                if ( (newEntry->status_next & 0x30000) == 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                                612,
                                0,
                                "%s",
                                "(newEntry->status_next & HASH_STAT_MASK) != HASH_STAT_FREE") )
                {
                    __debugbreak();
                }
                if ( (entry->status_next & 0x30000) == 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                                613,
                                0,
                                "%s",
                                "(entry->status_next & HASH_STAT_MASK) != HASH_STAT_FREE") )
                {
                    __debugbreak();
                }
                if ( refStr_->str != SL_ConvertToString(stringValue, inst)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                                615,
                                0,
                                "%s",
                                "refStr->str == SL_ConvertToString( stringValue, inst )") )
                {
                    __debugbreak();
                }
                goto END_CLEANUP;
            }
            prev = newIndex;
            newIndex = (unsigned __int16)newEntry->status_next;
        }
        newIndexa = gScrStringGlob[inst].hashTable->status_next;
        if ( !newIndexa )
        {
            Scr_DumpScriptThreads(inst);
            Scr_DumpScriptVariablesDefault(inst);
            Com_Error(ERR_DROP, "exceeded maximum number of script strings (increase STRINGLIST_SIZE)");
        }
        stringValue = MT_AllocIndex(inst, len + 4, type);
        newEntrya = &gScrStringGlob[inst].hashTable[newIndexa];
        if ( (newEntrya->status_next & 0x30000) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                        642,
                        0,
                        "%s",
                        "(newEntry->status_next & HASH_STAT_MASK) == HASH_STAT_FREE") )
        {
            __debugbreak();
        }
        newNext = (unsigned __int16)newEntrya->status_next;
        gScrStringGlob[inst].hashTable->status_next = newNext;
        gScrStringGlob[inst].hashTable[newNext].u.prev = 0;
        newEntrya->status_next = (unsigned __int16)entry->status_next | 0x10000;
        entry->status_next = (unsigned __int16)newIndexa | entry->status_next & 0x30000;
        newEntrya->u.prev = entry->u.prev;
    }
    else
    {
        if ( (entry->status_next & 0x30000) != 0 )
        {
            if ( (entry->status_next & 0x30000) != 0x10000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                            668,
                            0,
                            "%s",
                            "(entry->status_next & HASH_STAT_MASK) == HASH_STAT_MOVABLE") )
            {
                __debugbreak();
            }
            next = (unsigned __int16)entry->status_next;
            for ( preva = next;
                        (unsigned __int16)gScrStringGlob[inst].hashTable[preva].status_next != hash;
                        preva = (unsigned __int16)gScrStringGlob[inst].hashTable[preva].status_next )
            {
                ;
            }
            if ( !preva
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                            675,
                            0,
                            "%s",
                            "prev") )
            {
                __debugbreak();
            }
            newIndexb = gScrStringGlob[inst].hashTable->status_next;
            if ( !newIndexb )
            {
                Scr_DumpScriptThreads(inst);
                Scr_DumpScriptVariablesDefault(inst);
                Com_Error(ERR_DROP, "exceeded maximum number of script strings");
            }
            stringValue = MT_AllocIndex(inst, len + 4, type);
            newEntryb = &gScrStringGlob[inst].hashTable[newIndexb];
            if ( (newEntryb->status_next & 0x30000) != 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                            690,
                            0,
                            "%s",
                            "(newEntry->status_next & HASH_STAT_MASK) == HASH_STAT_FREE") )
            {
                __debugbreak();
            }
            newNexta = (unsigned __int16)newEntryb->status_next;
            gScrStringGlob[inst].hashTable->status_next = newNexta;
            gScrStringGlob[inst].hashTable[newNexta].u.prev = 0;
            gScrStringGlob[inst].hashTable[preva].status_next = newIndexb
                                                                                                                | gScrStringGlob[inst].hashTable[preva].status_next & 0x30000;
            newEntryb->status_next = next | 0x10000;
            newEntryb->u.prev = entry->u.prev;
        }
        else
        {
            stringValue = MT_AllocIndex(inst, len + 4, type);
            prevb = entry->u.prev;
            nexta = (unsigned __int16)entry->status_next;
            gScrStringGlob[inst].hashTable[prevb].status_next = nexta
                                                                                                                | gScrStringGlob[inst].hashTable[prevb].status_next & 0x30000;
            gScrStringGlob[inst].hashTable[nexta].u.prev = prevb;
        }
        if ( (hash & 0x30000) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                        702,
                        0,
                        "%s",
                        "!(hash & HASH_STAT_MASK)") )
        {
            __debugbreak();
        }
        entry->status_next = hash | 0x20000;
    }
    ++gScrStringGlob[inst].stringsUsed[gScrStringGlob[inst].indentLevel];
    if ( !stringValue
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                    710,
                    0,
                    "%s",
                    "stringValue") )
    {
        __debugbreak();
    }
    entry->u.prev = stringValue;
    refStrb = GetRefString(inst, stringValue);
    memcpy((unsigned __int8 *)refStrb->str, (unsigned __int8 *)str, len);
    //refStrb->0 = ($119B815E6C15BED54461C272BD343858)(((unsigned __int8)user << 16) | *(unsigned int *)&refStrb->0 & 0xFF00FFFF);
    refStrb->data = ((unsigned __int8)user << 16) | refStrb->data & 0xFF00FFFF;
    iassert(refStrb->user == user);
    //if ( (unsigned __int8)BYTE2(*(unsigned int *)&refStrb->0) != user
    //    && !Assert_MyHandler(
    //                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
    //                717,
    //                0,
    //                "%s",
    //                "refStr->user == user") )
    //{
    //    __debugbreak();
    //}

    //refStrb->0 = ($119B815E6C15BED54461C272BD343858)(*(unsigned int *)&refStrb->0 & 0xFFFF0000 | 1);
    refStrb->data = refStrb->data & 0xFFFF0000 | 1;
    //refStrb->0 = ($119B815E6C15BED54461C272BD343858)(((unsigned __int8)len << 24) | *(_DWORD *)&refStrb->0 & 0xFFFFFF);
    refStrb->data = ((unsigned __int8)len << 24) | refStrb->data & 0xFFFFFF;

    if ( gScrStringDebugGlob[inst] )
    {
        _InterlockedExchangeAdd(&gScrStringDebugGlob[inst]->totalRefCount, 1u);
        _InterlockedExchangeAdd(&gScrStringDebugGlob[inst]->refCount[stringValue], 1u);
    }
    if ( (entry->status_next & 0x30000) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                    729,
                    0,
                    "%s",
                    "(entry->status_next & HASH_STAT_MASK) != HASH_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( refStrb->str != SL_ConvertToString(stringValue, inst)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                    730,
                    0,
                    "%s",
                    "refStr->str == SL_ConvertToString( stringValue, inst )") )
    {
        __debugbreak();
    }
END_CLEANUP:
    Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
    return stringValue;
}

unsigned int __cdecl SL_GetString_(scriptInstance_t inst, char *str, unsigned int user, int type)
{
    return SL_GetStringOfSize(inst, str, user, strlen(str) + 1, type);
}

unsigned int __cdecl SL_GetString(char *str, unsigned int user, scriptInstance_t inst)
{
    return SL_GetString_(inst, str, user, 6);
}

unsigned int __cdecl SL_GetLowercaseString_(const char *str, unsigned int user, int type, scriptInstance_t inst)
{
    return SL_GetLowercaseStringOfSize(inst, str, user, strlen(str) + 1, type);
}

unsigned int __cdecl SL_GetLowercaseStringOfSize(
                scriptInstance_t inst,
                const char *str,
                unsigned int user,
                unsigned int len,
                int type)
{
    char stra[8192]; // [esp+0h] [ebp-2008h] BYREF
    unsigned int i; // [esp+2004h] [ebp-4h]

    if ( len <= 0x2000 )
    {
        for ( i = 0; i < len; ++i )
            stra[i] = tolower(str[i]);
        return SL_GetStringOfSize(inst, stra, user, len, type);
    }
    else
    {
        Com_Error(ERR_DROP, "max string length exceeded: \"%s\"", str);
        return 0;
    }
}

unsigned int __cdecl SL_GetLowercaseString(const char *str, unsigned int user, scriptInstance_t inst)
{
    return SL_GetLowercaseString_(str, user, 6, inst);
}

unsigned int __cdecl SL_ConvertToLowercase(
                scriptInstance_t inst,
                unsigned int stringValue,
                unsigned int user,
                int type)
{
    char *v5; // [esp+0h] [ebp-2014h]
    char str[8192]; // [esp+4h] [ebp-2010h] BYREF
    unsigned int StringOfSize; // [esp+2008h] [ebp-Ch]
    unsigned int len; // [esp+200Ch] [ebp-8h]
    unsigned int i; // [esp+2010h] [ebp-4h]

    len = SL_GetStringLen(inst, stringValue) + 1;
    if ( len > 0x2000 )
        return stringValue;
    v5 = SL_ConvertToString(stringValue, inst);
    for ( i = 0; i < len; ++i )
        str[i] = tolower(v5[i]);
    StringOfSize = SL_GetStringOfSize(inst, str, user, len, type);
    SL_RemoveRefToString(inst, stringValue);
    return StringOfSize;
}

void __cdecl SL_TransferRefToUser(scriptInstance_t inst, unsigned int stringValue, unsigned int user)
{
    char *v3; // eax
    char *v4; // eax
    signed __int32 v5; // [esp+4h] [ebp-14h]
    RefString *refStr; // [esp+14h] [ebp-4h]
    volatile LONG Comperand; // [esp+20h] [ebp-28h]

    refStr = GetRefString(inst, stringValue);
    //if ( ((unsigned __int8)user & (unsigned __int8)BYTE2(*(unsigned int *)&refStr->0)) != 0 )
    if ((user & refStr->user) != 0)
    {
        iassert(refStr->refCount > 1);

        if ( gScrStringDebugGlob[inst] )
        {
            iassert((gScrStringDebugGlob[inst]->refCount[stringValue]));

            _InterlockedExchangeAdd(&gScrStringDebugGlob[inst]->totalRefCount, 0xFFFFFFFF);
            _InterlockedExchangeAdd(&gScrStringDebugGlob[inst]->refCount[stringValue], 0xFFFFFFFF);
        }
        _InterlockedExchangeAdd(&refStr->data, 0xFFFFFFFF);
    }
    else
    {
        //do
        //    v5 = (signed __int32)refStr->0;
        //while ( _InterlockedCompareExchange((volatile signed __int32 *)refStr, v5 | (user << 16), v5) != v5 );
        do
            Comperand = refStr->data;
        while (InterlockedCompareExchange(&refStr->data, Comperand | (user << 16), Comperand) != Comperand);
    }
}

void __cdecl SL_CheckExists(scriptInstance_t inst, unsigned int stringValue)
{
    const char *v2; // eax
    const char *v3; // eax

    if ( gScrStringDebugGlob[inst] && !gScrStringDebugGlob[inst]->refCount[stringValue] )
    {
        v2 = SL_DebugConvertToString(stringValue, inst);
        v3 = va("%d '%s'", stringValue, v2);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                        856,
                        0,
                        "%s\n\t%s",
                        "!gScrStringDebugGlob[inst] || gScrStringDebugGlob[inst]->refCount[stringValue]",
                        v3) )
            __debugbreak();
    }
}

void __cdecl SL_AddRefToString(unsigned int stringValue, scriptInstance_t inst)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    volatile int v7; // [esp+0h] [ebp-10h]
    RefString *refStr; // [esp+Ch] [ebp-4h]

    if ( gScrStringDebugGlob[inst] )
    {
        if ( !gScrStringDebugGlob[inst]->refCount[stringValue] )
        {
            v2 = SL_DebugConvertToString(stringValue, inst);
            v3 = va("%d '%s'", stringValue, v2);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                            870,
                            0,
                            "%s\n\t%s",
                            "gScrStringDebugGlob[inst]->refCount[stringValue]",
                            v3) )
                __debugbreak();
        }
        if ( gScrStringDebugGlob[inst]->refCount[stringValue] >= 0x10000 )
        {
            v4 = SL_DebugConvertToString(stringValue, inst);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                            871,
                            0,
                            "%s\n\t(SL_DebugConvertToString( stringValue, inst )) = %s",
                            "(gScrStringDebugGlob[inst]->refCount[stringValue] < 65536)",
                            v4) )
                __debugbreak();
        }
        _InterlockedExchangeAdd(&gScrStringDebugGlob[inst]->totalRefCount, 1u);
        _InterlockedExchangeAdd(&gScrStringDebugGlob[inst]->refCount[stringValue], 1u);
    }
    refStr = GetRefString(inst, stringValue);
    _InterlockedExchangeAdd(&refStr->data, 1u);

    iassert(refStr->refCount);
}

void __cdecl SL_RemoveRefToString(scriptInstance_t inst, unsigned int stringValue)
{
    RefString *refStr; // [esp+0h] [ebp-8h]
    int len; // [esp+4h] [ebp-4h]

    refStr = GetRefString(inst, stringValue);
    len = SL_GetRefStringLen(refStr) + 1;
    SL_RemoveRefToStringOfSize(inst, stringValue, len);
}

void __cdecl SL_RemoveRefToStringOfSize(scriptInstance_t inst, unsigned int stringValue, unsigned int len)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    RefString *refStr; // [esp+10h] [ebp-4h]

    refStr = GetRefString(inst, stringValue);
    if ( _InterlockedDecrement(&refStr->data) << 16 )
    {
        if ( gScrStringDebugGlob[inst] )
        {
            if ( !gScrStringDebugGlob[inst]->totalRefCount || !gScrStringDebugGlob[inst]->refCount[stringValue] )
            {
                v3 = SL_DebugConvertToString(stringValue, inst);
                v4 = va(
                             "gScrStringDebugGlob[inst]->totalRefCount: %i, stringValue: %i, gScrStringDebugGlob[inst]->refCount[string"
                             "Value]: %i, string: '%s'",
                             gScrStringDebugGlob[inst]->totalRefCount,
                             stringValue,
                             gScrStringDebugGlob[inst]->refCount[stringValue],
                             v3);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                                1020,
                                0,
                                "%s\n\t%s",
                                "gScrStringDebugGlob[inst]->totalRefCount && gScrStringDebugGlob[inst]->refCount[stringValue]",
                                v4) )
                    __debugbreak();
            }
            if ( !gScrStringDebugGlob[inst]->refCount[stringValue] )
            {
                v5 = SL_DebugConvertToString(stringValue, inst);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                                1021,
                                0,
                                "%s\n\t(SL_DebugConvertToString( stringValue, inst )) = %s",
                                "(gScrStringDebugGlob[inst]->refCount[stringValue])",
                                v5) )
                    __debugbreak();
            }
            _InterlockedExchangeAdd(&gScrStringDebugGlob[inst]->totalRefCount, 0xFFFFFFFF);
            _InterlockedExchangeAdd(&gScrStringDebugGlob[inst]->refCount[stringValue], 0xFFFFFFFF);
        }
    }
    else
    {
        SL_FreeString(inst, stringValue, refStr, len);
        if ( gScrStringDebugGlob[inst] )
        {
            if ( !gScrStringDebugGlob[inst]->totalRefCount || !gScrStringDebugGlob[inst]->refCount[stringValue] )
            {
                v6 = SL_DebugConvertToString(stringValue, inst);
                v7 = va(
                             "gScrStringDebugGlob[inst]->totalRefCount: %i, stringValue: %i, gScrStringDebugGlob[inst]->refCount[string"
                             "Value]: %i, string: '%s'",
                             gScrStringDebugGlob[inst]->totalRefCount,
                             stringValue,
                             gScrStringDebugGlob[inst]->refCount[stringValue],
                             v6);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                                1038,
                                0,
                                "%s\n\t%s",
                                "gScrStringDebugGlob[inst]->totalRefCount && gScrStringDebugGlob[inst]->refCount[stringValue]",
                                v7) )
                    __debugbreak();
            }
            if ( !gScrStringDebugGlob[inst]->refCount[stringValue] )
            {
                v8 = SL_DebugConvertToString(stringValue, inst);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                                1039,
                                0,
                                "%s\n\t(SL_DebugConvertToString( stringValue, inst )) = %s",
                                "(gScrStringDebugGlob[inst]->refCount[stringValue])",
                                v8) )
                    __debugbreak();
            }
            _InterlockedExchangeAdd(&gScrStringDebugGlob[inst]->totalRefCount, 0xFFFFFFFF);
            _InterlockedExchangeAdd(&gScrStringDebugGlob[inst]->refCount[stringValue], 0xFFFFFFFF);
        }
    }
}

void __cdecl SL_FreeString(scriptInstance_t inst, unsigned int stringValue, RefString *refStr, unsigned int len)
{
    char *v4; // eax
    HashEntry *entry; // [esp+4h] [ebp-18h]
    unsigned int newIndex; // [esp+8h] [ebp-14h]
    unsigned int newNext; // [esp+Ch] [ebp-10h]
    unsigned int index; // [esp+10h] [ebp-Ch]
    unsigned int prev; // [esp+14h] [ebp-8h]
    HashEntry *newEntry; // [esp+18h] [ebp-4h]

    index = GetHashCode(inst, refStr->str, len);
    Sys_EnterCriticalSection(CRITSECT_SCRIPT_STRING);
    //if ( (unsigned __int16)*(unsigned int *)&refStr->0 )
    if (refStr->refCount)
    {
        Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
        return;
    }
    else
    {
        entry = &gScrStringGlob[inst].hashTable[index];
        iassert(!refStr->user);
        //if ( (unsigned __int8)BYTE2(*(unsigned int *)&refStr->0) )
        //{
        //    v4 = SL_DebugConvertToString(stringValue, inst);
        //    if ( !Assert_MyHandler(
        //                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
        //                    928,
        //                    0,
        //                    "%s\n\t(SL_DebugConvertToString( stringValue, inst )) = %s",
        //                    "(!refStr->user)",
        //                    v4) )
        //        __debugbreak();
        //}
        MT_FreeIndex(inst, stringValue, len + 4);


        if ( (entry->status_next & 0x30000) != 0x20000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                        932,
                        0,
                        "%s\n\t(entry->status_next) = %i",
                        "((entry->status_next & HASH_STAT_MASK) == HASH_STAT_HEAD)",
                        entry->status_next) )
        {
            __debugbreak();
        }

        newIndex = (unsigned __int16)entry->status_next;
        newEntry = &gScrStringGlob[inst].hashTable[newIndex];
        if (entry->u.prev == stringValue)
        {
            --gScrStringGlob[inst].stringsUsed[gScrStringGlob[inst].indentLevel];
            if (newEntry == entry)
            {
                newEntry = entry;
                newIndex = index;
            }
            else
            {
                entry->status_next = (unsigned __int16)newEntry->status_next | 0x20000;
                entry->u.prev = newEntry->u.prev;
                gScrStringGlob[inst].nextFreeEntry = entry;
            }
        }
        else
        {
            prev = index;
            while ( 1 )
            {
                if ( newEntry == entry
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                                959,
                                0,
                                "%s",
                                "newEntry != entry") )
                {
                    __debugbreak();
                }
                if ( (newEntry->status_next & 0x30000) != 0x10000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                                960,
                                0,
                                "%s",
                                "(newEntry->status_next & HASH_STAT_MASK) == HASH_STAT_MOVABLE") )
                {
                    __debugbreak();
                }
                if ( newEntry->u.prev == stringValue )
                    break;
                prev = newIndex;
                newIndex = (unsigned __int16)newEntry->status_next;
                newEntry = &gScrStringGlob[inst].hashTable[newIndex];
            }
            gScrStringGlob[inst].hashTable[prev].status_next = (unsigned __int16)newEntry->status_next
                                                                                                             | gScrStringGlob[inst].hashTable[prev].status_next & 0x30000;
        }
        if ( (newEntry->status_next & 0x30000) == 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                        974,
                        0,
                        "%s",
                        "(newEntry->status_next & HASH_STAT_MASK) != HASH_STAT_FREE") )
        {
            __debugbreak();
        }
        newNext = gScrStringGlob[inst].hashTable->status_next;
        if ( (newNext & 0x30000) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                        977,
                        0,
                        "%s",
                        "(newNext & HASH_STAT_MASK) == HASH_STAT_FREE") )
        {
            __debugbreak();
        }
        newEntry->status_next = newNext;
        newEntry->u.prev = 0;
        gScrStringGlob[inst].hashTable[newNext].u.prev = newIndex;
        gScrStringGlob[inst].hashTable->status_next = newIndex;
        Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
    }
}

void __cdecl Scr_SetStringLiveUpdateSafe(unsigned __int16 *to, char *value, scriptInstance_t inst)
{
    int substrLen; // [esp+0h] [ebp-94h]
    const char *substr; // [esp+4h] [ebp-90h]
    RefString *refStr; // [esp+8h] [ebp-8Ch] BYREF
    char pfValue[128]; // [esp+Ch] [ebp-88h] BYREF
    const char *destString; // [esp+90h] [ebp-4h]

    if ( *to )
    {
        destString = SL_ConvertToString(*to, SCRIPTINSTANCE_SERVER);
        if ( !destString || I_stricmp(destString, value) )
        {
            if ( *destString == 112 && destString[1] == 102 )
            {
                substr = I_stristr(destString, "_");
                if ( substr )
                {
                    substrLen = substr - destString + 2;
                    I_strncpyz(pfValue, destString, substrLen);
                    I_strncpyz(&pfValue[substrLen - 1], value, 128 - substrLen);
                    if ( !I_stricmp(destString, pfValue) )
                        return;
                    value = pfValue;
                }
            }
            refStr = GetRefString(inst, *to);
            if ( (refStr->data & 0x40000) == 0 )
                SL_RemoveRefToString(inst, *to);
            *to = SL_GetString(value, 0, inst);
        }
    }
    else
    {
        *to = SL_GetString(value, 0, inst);
    }
}

void __cdecl Scr_SetString(unsigned __int16 *to, unsigned int from, scriptInstance_t inst)
{
    if ( from )
        SL_AddRefToString(from, inst);
    if ( *to )
        SL_RemoveRefToString(inst, *to);
    *to = from;
}

void __cdecl Scr_SetStringFromCharString(unsigned __int16 *to, char *from, scriptInstance_t inst)
{
    if ( *to )
        SL_RemoveRefToString(inst, *to);
    *to = SL_GetString(from, 0, SCRIPTINSTANCE_SERVER);
}

unsigned int __cdecl Scr_AllocString(const char *s, int sys, scriptInstance_t inst)
{
    if ( sys != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp",
                    1108,
                    0,
                    "%s",
                    "sys == SCR_SYS_GAME") )
    {
        __debugbreak();
    }
    return SL_GetString((char*)s, 1u, inst);
}

unsigned int __cdecl SL_GetStringForFloat(float f, scriptInstance_t inst)
{
    char tempString[132]; // [esp+8h] [ebp-88h] BYREF

    sprintf(tempString, "%g", f);
    return SL_GetString_(inst, tempString, 0, 15);
}

unsigned int __cdecl SL_GetStringForInt(scriptInstance_t inst, int i)
{
    char tempString[132]; // [esp+0h] [ebp-88h] BYREF

    I_itoa(i, tempString, 128);
    return SL_GetString_(inst, tempString, 0, 15);
}

unsigned int __cdecl SL_GetStringForVector(scriptInstance_t inst, const float *v)
{
    char tempString[132]; // [esp+18h] [ebp-88h] BYREF

    sprintf(tempString, "(%g, %g, %g)", *v, v[1], v[2]);
    return SL_GetString_(inst, tempString, 0, 15);
}

void __cdecl SL_ShutdownSystem(scriptInstance_t inst, unsigned int user)
{
    unsigned int hash; // [esp+0h] [ebp-Ch]
    HashEntry *entry; // [esp+4h] [ebp-8h]
    RefString *refStr; // [esp+8h] [ebp-4h]

    if ( !user
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp", 1147, 0, "%s", "user") )
    {
        __debugbreak();
    }
    Sys_EnterCriticalSection(CRITSECT_SCRIPT_STRING);
    for ( hash = 1; hash < (inst != SCRIPTINSTANCE_SERVER ? 0x4000 : 0x8000); ++hash )
    {
        do
        {
            entry = &gScrStringGlob[inst].hashTable[hash];
            if ( (entry->status_next & 0x30000) == 0 )
                break;
            refStr = GetRefString(inst, entry->u.prev);
            //if ( ((unsigned __int8)user & (unsigned __int8)BYTE2(*(unsigned int *)&refStr->0)) == 0 )
            //    break;
            if (((unsigned __int8)user & refStr->user) == 0)
                break;
            //refStr->0 = ($119B815E6C15BED54461C272BD343858)(((unsigned __int8)(~(_BYTE)user & HIWORD(*(unsigned int *)&refStr->0)) << 16) | *(unsigned int *)&refStr->0 & 0xFF00FFFF);
            refStr->data = ((unsigned __int8)(~(BYTE)user & HIWORD(refStr->data)) << 16) | refStr->data & 0xFF00FFFF;

            gScrStringGlob[inst].nextFreeEntry = 0;
            SL_RemoveRefToString(inst, entry->u.prev);
        } while (gScrStringGlob[inst].nextFreeEntry);
    }
    Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
}

void __cdecl Scr_ShutdownGameStrings(scriptInstance_t inst)
{
    SL_ShutdownSystem(inst, 1u);
}

void __cdecl SL_TransferSystem(unsigned int from, unsigned int to, scriptInstance_t inst)
{
    unsigned int hash; // [esp+0h] [ebp-Ch]
    HashEntry *entry; // [esp+4h] [ebp-8h]
    RefString *refStr; // [esp+8h] [ebp-4h]

    if ( !from
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp", 1191, 0, "%s", "from") )
    {
        __debugbreak();
    }
    if ( !to
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp", 1192, 0, "%s", "to") )
    {
        __debugbreak();
    }
    Sys_EnterCriticalSection(CRITSECT_SCRIPT_STRING);
    for ( hash = 1; hash < (inst != SCRIPTINSTANCE_SERVER ? 0x4000 : 0x8000); ++hash )
    {
        entry = &gScrStringGlob[inst].hashTable[hash];
        if ( (entry->status_next & 0x30000) != 0 )
        {
            refStr = GetRefString(inst, entry->u.prev);
            //if ( ((unsigned __int8)from & (unsigned __int8)BYTE2(*(unsigned int *)&refStr->0)) != 0 )
            if (((unsigned __int8)from & refStr->user) != 0)
            {
                //refStr->0 = ($119B815E6C15BED54461C272BD343858)(((unsigned __int8)(~(_BYTE)from & HIWORD(*(unsigned int *)&refStr->0)) << 16) | *(unsigned int *)&refStr->0 & 0xFF00FFFF);
                refStr->data = ((unsigned __int8)(~(BYTE)from & HIWORD(refStr->data)) << 16) | refStr->data & 0xFF00FFFF;
                //refStr->0 = ($119B815E6C15BED54461C272BD343858)(((unsigned __int8)(to | HIWORD(*(unsigned int *)&refStr->0)) << 16) | *(unsigned int *)&refStr->0 & 0xFF00FFFF);
                refStr->data = ((unsigned __int8)(to | HIWORD(refStr->data)) << 16) | refStr->data & 0xFF00FFFF;
            }
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
}

unsigned int __cdecl Scr_CreateCanonicalFilename(scriptInstance_t inst, const char *filename)
{
    char newFilename[1028]; // [esp+0h] [ebp-408h] BYREF

    CreateCanonicalFilename(newFilename, filename, 1024);
    return SL_GetString_(inst, newFilename, 0, 7);
}

void __cdecl CreateCanonicalFilename(char *newFilename, const char *filename, int count)
{
    unsigned int c; // [esp+0h] [ebp-4h]

    if ( !count
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_stringlist.cpp", 1224, 0, "%s", "count") )
    {
        __debugbreak();
    }
    do
    {
        do
        {
            do
                c = *filename++;
            while ( c == 92 );
        }
        while ( c == 47 );
        while ( c >= 0x20 )
        {
            *newFilename++ = tolower(c);
            if ( !--count )
                Com_Error(ERR_DROP, "Filename '%s' exceeds maximum length of %d", filename, 0);
            if ( c == 47 )
                break;
            c = *filename++;
            if ( c == 92 )
                c = 47;
        }
    }
    while ( c );
    *newFilename = 0;
}

