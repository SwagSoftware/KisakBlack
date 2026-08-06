#pragma once

void __cdecl SND_InitLog();
void __cdecl SND_LogLookupAlias(unsigned int hash, char *string);
void __cdecl SND_LogRegisterString(const char *name, unsigned int hash);
char __cdecl SND_LogSkip(unsigned int hash, unsigned int skipCount, unsigned int *skipHash, unsigned int *skipTime);
void __cdecl SND_LogMissingAliasId(unsigned int hash);
void __cdecl SND_LogPlayedAliasId(unsigned int hash);
void __cdecl SND_LogCommandQHWM(unsigned int event_count);
void __cdecl SND_LogNotifyQHWM(unsigned int event_count);
