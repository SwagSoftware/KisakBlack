// ShlObj.h — minimal portable stand-in. The engine uses only SHGetFolderPathA, to
// resolve the user's data/documents folder (com_files.cpp). Map the relevant CSIDLs
// onto $HOME so the game writes under the user's home directory on Linux.
#ifndef KISAK_SHLOBJ_H
#define KISAK_SHLOBJ_H

#include "windows.h"

#define CSIDL_PERSONAL      0x0005
#define CSIDL_LOCAL_APPDATA 0x001c
#define CSIDL_FLAG_CREATE   0x8000

extern "C" char *getenv(const char *) noexcept;  // avoid pulling <cstdlib>'s random()

static inline HRESULT SHGetFolderPathA(HWND, int /*csidl*/, HANDLE, DWORD, char *pszPath) {
    if (!pszPath) return (HRESULT)-1;
    const char *home = getenv("HOME"); const char *base = home ? home : "/tmp";
    int n = 0; while (base[n] && n < MAX_PATH - 1) { pszPath[n] = base[n]; ++n; }
    pszPath[n] = '\0';
    return 0;  // S_OK
}
#endif
