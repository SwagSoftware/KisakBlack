#include "snd_log.h"

#include <cstring>
#include "snd.h"
#include <win32/win_common.h>
#include <qcommon/common.h>

void __cdecl SND_InitLog()
{
    memset(g_snd.logSkipPlayedHash, 0, sizeof(g_snd.logSkipPlayedHash));
    memset(g_snd.logSkipPlayedTimestamp, 0, sizeof(g_snd.logSkipPlayedTimestamp));
    memset(g_snd.logSkipMissingHash, 0, sizeof(g_snd.logSkipMissingHash));
    memset(g_snd.logSkipMissingTimestamp, 0, sizeof(g_snd.logSkipMissingTimestamp));
    memset(g_snd.logReverseHash, 0, sizeof(g_snd.logReverseHash));
    g_snd.logTimestamp = 1;
}

void __cdecl SND_LogLookupAlias(unsigned int hash, char *string)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    *string = 0;
    Sys_EnterCriticalSection(CRITSECT_SOUND_LOOKUP_CACHE);
    for ( i = 0; i < 0x80; ++i )
    {
        if ( g_snd.logReverseHash[i].hash == hash )
        {
            g_snd.logReverseHash[i].timestamp = g_snd.logTimestamp++;
            strncpy(string, (char*)& g_snd.logReverseHash[i], 0x30u);
            break;
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_SOUND_LOOKUP_CACHE);
}

void __cdecl SND_LogRegisterString(const char *name, unsigned int hash)
{
    unsigned int i; // [esp+0h] [ebp-Ch]
    unsigned int oldest; // [esp+4h] [ebp-8h]
    unsigned int oldestTime; // [esp+8h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_SOUND_LOOKUP_CACHE);
    oldest = 0;
    oldestTime = g_snd.logReverseHash[0].timestamp;
    for ( i = 0; i < 0x80; ++i )
    {
        if ( g_snd.logReverseHash[i].hash == hash )
        {
            g_snd.logReverseHash[oldest].timestamp = g_snd.logTimestamp++;
            Sys_LeaveCriticalSection(CRITSECT_SOUND_LOOKUP_CACHE);
            return;
        }
        if ( !g_snd.logReverseHash[i].hash )
        {
            oldest = i;
            break;
        }
        if ( oldestTime > g_snd.logReverseHash[i].timestamp )
        {
            oldest = i;
            oldestTime = g_snd.logReverseHash[i].timestamp;
        }
    }
    g_snd.logReverseHash[oldest].hash = hash;
    g_snd.logReverseHash[oldest].timestamp = g_snd.logTimestamp++;
    strncpy((char*)&g_snd.logReverseHash[oldest], (char *)name, 0x30u);
    Sys_LeaveCriticalSection(CRITSECT_SOUND_LOOKUP_CACHE);
}

char __cdecl SND_LogSkip(unsigned int hash, unsigned int skipCount, unsigned int *skipHash, unsigned int *skipTime)
{
    unsigned int i; // [esp+0h] [ebp-Ch]
    unsigned int oldest; // [esp+4h] [ebp-8h]
    unsigned int oldestTime; // [esp+8h] [ebp-4h]

    oldest = 0;
    oldestTime = *skipTime;
    for ( i = 0; i < skipCount; ++i )
    {
        if ( skipHash[i] == hash )
        {
            skipTime[oldest] = g_snd.logTimestamp++;
            return 1;
        }
        if ( skipHash[i] )
        {
            if ( oldestTime > skipTime[i] )
            {
                oldest = i;
                oldestTime = skipTime[i];
            }
        }
        else
        {
            oldest = i;
            oldestTime = 0;
        }
    }
    skipHash[oldest] = hash;
    skipTime[oldest] = g_snd.logTimestamp++;
    return 0;
}

void __cdecl SND_LogMissingAliasId(unsigned int hash)
{
    char reverse[48]; // [esp+0h] [ebp-34h] BYREF

    iassert(hash != SND_INVALID_HASH);
    SND_LogSkip(hash, 0x80u, g_snd.logSkipMissingHash, g_snd.logSkipMissingTimestamp);
    if ( !SND_LogSkip(hash, 0x80u, g_snd.logSkipMissingHash, g_snd.logSkipMissingTimestamp) )
    {
        SND_LogLookupAlias(hash, reverse);
        Com_Printf(9, "missing alias 0x%x %s\n", hash, reverse);
    }
}

void __cdecl SND_LogPlayedAliasId(unsigned int hash)
{
    iassert(hash != SND_INVALID_HASH);
    SND_LogSkip(hash, 0x200u, g_snd.logSkipPlayedHash, g_snd.logSkipPlayedTimestamp);
}

void __cdecl SND_LogCommandQHWM(unsigned int event_count)
{
    if ( g_snd.logCommandCount < event_count )
        g_snd.logCommandCount = event_count;
}

void __cdecl SND_LogNotifyQHWM(unsigned int event_count)
{
    if ( g_snd.logNotifyCount < event_count )
        g_snd.logNotifyCount = event_count;
}

