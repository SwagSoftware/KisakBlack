// sys_files.cpp — Linux Sys_ListFiles / Sys_ListFilteredFiles (win32/win_common.cpp on
// Windows). Enumerates a directory into the engine's hunk-allocated file list so the FS
// layer can discover .iwd archives and other assets.
#include <win32/win_common.h>
#include <universal/mem_userhunk.h>
#include <qcommon/common.h>   // Com_sprintf

#include <dirent.h>
#include <sys/stat.h>
#include <cstring>
#include <cstdio>

char __cdecl Com_FilterPath(const char *filter, const char *name, int casesensitive);  // qcommon

// Case-insensitive "does name end with .extension" (or extension alone).
bool HasFileExtension(const char *name, const char *extension) {
    if (!name) return false;
    if (!extension || !*extension) return true;
    size_t nl = strlen(name), el = strlen(extension);
    if (el > nl) return false;
    const char *tail = name + nl - el;
    for (size_t i = 0; i < el; ++i) {
        char a = tail[i], b = extension[i];
        if (a >= 'A' && a <= 'Z') a += 32; if (b >= 'A' && b <= 'Z') b += 32;
        if (a != b) return false;
    }
    return true;
}

namespace {
// Normalize backslashes to '/' into a temp buffer.
void Norm(const char *in, char *out, size_t n) {
    size_t i = 0; for (; in && in[i] && i < n - 1; ++i) out[i] = in[i] == '\\' ? '/' : in[i]; out[i] = 0;
}
}

// Recursive filtered listing (the engine passes a wildcard `filter` like "*.cfg").
void Sys_ListFilteredFiles(HunkUser *user, const char *basedir, const char *subdirs,
                           const char *filter, char **list, int *numfiles) {
    char dirpath[1024]; char base[1024]; Norm(basedir, base, sizeof(base));
    if (subdirs && *subdirs) snprintf(dirpath, sizeof(dirpath), "%s/%s", base, subdirs);
    else                     snprintf(dirpath, sizeof(dirpath), "%s", base);
    DIR *d = opendir(dirpath);
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d)) && *numfiles < 16383) {
        if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
        char full[1280]; snprintf(full, sizeof(full), "%s/%s", dirpath, e->d_name);
        struct stat st; if (stat(full, &st) != 0) continue;
        char rel[1024];
        if (subdirs && *subdirs) snprintf(rel, sizeof(rel), "%s/%s", subdirs, e->d_name);
        else                     snprintf(rel, sizeof(rel), "%s", e->d_name);
        if (S_ISDIR(st.st_mode)) {
            Sys_ListFilteredFiles(user, basedir, rel, filter, list, numfiles);
        } else if (Com_FilterPath((char *)filter, rel, 0)) {
            char *copy = (char *)Hunk_UserAlloc(user, (int)strlen(rel) + 1, 1, "");
            strcpy(copy, rel); list[(*numfiles)++] = copy;
        }
    }
    closedir(d);
}

HunkUser **Sys_ListFiles(char *directory, char *extension, char *filter, int *numfiles, int wantsubs) {
    char *list[16384]; int n = 0;
    HunkUser *user = Hunk_UserCreate(0x20000, HU_SCHEME_DEFAULT, 0, 0, "Sys_ListFiles", 3);

    if (filter) {
        Sys_ListFilteredFiles(user, directory, "", filter, list, &n);
    } else {
        bool wantDirs = (extension && extension[0] == '/' && !extension[1]);
        if (wantDirs) extension = (char *)"";
        char dirpath[1024]; Norm(directory, dirpath, sizeof(dirpath));
        DIR *d = opendir(dirpath);
        if (d) {
            struct dirent *e;
            while ((e = readdir(d)) && n < 16383) {
                if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
                char full[1280]; snprintf(full, sizeof(full), "%s/%s", dirpath, e->d_name);
                struct stat st; bool isdir = (stat(full, &st) == 0 && S_ISDIR(st.st_mode));
                if (wantDirs) { if (!isdir) continue; }
                else { if (isdir && !wantsubs) continue;
                       if (extension && *extension && !HasFileExtension(e->d_name, extension)) continue; }
                char *copy = (char *)Hunk_UserAlloc(user, (int)strlen(e->d_name) + 1, 1, "");
                strcpy(copy, e->d_name); list[n++] = copy;
            }
            closedir(d);
        }
    }

    if (numfiles) *numfiles = n;
    if (n == 0) { Hunk_UserDestroy(user); return 0; }
    // Layout the FS expects: [0]=owning HunkUser, [1..n]=names, [n+1]=null; returns &[1].
    HunkUser **v9 = (HunkUser **)Hunk_UserAlloc(user, 4 * n + 8, 4, 0);
    *v9++ = user;
    for (int i = 0; i < n; ++i) v9[i] = (HunkUser *)list[i];
    v9[n] = 0;
    return v9;
}
