// Psapi.h — portable stand-in for the Process Status API header.
#ifndef KISAK_PSAPI_H
#define KISAK_PSAPI_H
#include "windows.h"
typedef struct _PROCESS_MEMORY_COUNTERS {
    DWORD cb, PageFaultCount;
    SIZE_T PeakWorkingSetSize, WorkingSetSize, QuotaPeakPagedPoolUsage, QuotaPagedPoolUsage,
           QuotaPeakNonPagedPoolUsage, QuotaNonPagedPoolUsage, PagefileUsage, PeakPagefileUsage;
} PROCESS_MEMORY_COUNTERS, *PPROCESS_MEMORY_COUNTERS;
static inline BOOL GetProcessMemoryInfo(HANDLE, PROCESS_MEMORY_COUNTERS *c, DWORD) { if (c) memset(c, 0, sizeof(*c)); return TRUE; }
#endif
