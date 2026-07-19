#pragma once

#include <cstdio> // FILE
#include <cmath> // isnan

enum AssertOccurance : __int32
{                                                                             // XREF: AssertNotify/r
        FIRST_TIME = 0x0,
        RECURSIVE = 0x1,
};

struct AddressInfo_s // sizeof=0x110
{                                       // XREF: .data:g_assertAddress/r
                                        // AddressInfo/r
    unsigned int address;               // XREF: Assert_DoStackTrace(char *,int,int,void *)+96/w
    char moduleName[64];
    char bestFunction[64];
    char bestFunctionFilename[64];
    unsigned int bestFunctionAddress;
    char bestLineFilename[64];
    unsigned int bestLineAddress;
    unsigned int bestLineNumber;
};

void __cdecl FixWindowsDesktop();
int __cdecl Assert_DoStackTrace(char *msg, int nIgnore, int type, int *context);
int __cdecl LoadMapFiles(char *msg);
void __cdecl LoadMapFilesForDir(const char *dir);
char __cdecl ParseMapFile(FILE *fp, unsigned int baseAddress, const char *mapName);
void __cdecl ParseError(const char *msg);
char __cdecl ReadLine(FILE *fp);
char __cdecl SkipLines(int lineCount, FILE *fp);
void __cdecl Assert_ResetAddressInfo();
void __cdecl Assert_SetMonkeyCallbackHandler(void (__cdecl *AssertCallbackFunc)(const char *));
bool CopyMessageToClipboard();
char __cdecl AssertNotify(int type, AssertOccurance occurance);
void __cdecl Assert_BuildAssertMessage(char *expr, char *filename, int line, int type, int skipLevels, char *message);
void __cdecl RefreshQuitOnErrorCondition();
bool __cdecl QuitOnError();


bool Assert_MyHandler(const char *filename, int line, int type, const char *fmt, ...);

#define IS_NAN(x) (isnan(x))

#ifdef _DEBUG
// Variadic: some decompiled call sites pass extra file/line/message args after the
// condition; __FILE__/__LINE__ already capture the location, so the extras are
// ignored. (A 1-arg-only macro fails to compile those call sites on any compiler.)
#define iassert(expression, ...) (void)(                                                                                                             \
                        (!!(expression)) ||                                                                                                                    \
                        (Assert_MyHandler(__FILE__, (unsigned)(__LINE__), 0, "%s", #expression), 0) \
                )

#define vassert(expression, fmt, ...)    (void)(                                                                                                             \
                        (!!(expression)) ||                                                                                                                    \
                        (Assert_MyHandler(__FILE__, (unsigned)(__LINE__), 0, "%s\n\t" fmt, #expression, __VA_ARGS__), 0) \
                )

#define bcassert(expression, maxv) vassert(((expression) < (maxv)), #expression "%d does not index [0, %d)", expression, maxv)
#define bcassert2(expression, maxv) vassert(((expression) <= (maxv)), #expression "%d does not index [0, %d]", expression, maxv)
#define rangeassert(expression, minv, maxv) vassert(((expression) <= (maxv) && (expression >= (minv))), #expression "not in [" #minv ", " #maxv "]\n\t%i not in [%i, %i]", expression, minv, maxv)
#define nanassertvec3(vec) iassert( !IS_NAN((vec)[0]) && !IS_NAN((vec)[1]) && !IS_NAN((vec)[2]) )
#define alwaysfails 0
#else
#define iassert(...)
#define vassert(expression, fmt, ...)
#define bcassert(expression, maxv)
#define bcassert2(expression, maxv)
#define rangeassert(expression, minv, maxv)
#define nanassertvec3(vec) 
#define alwaysfails 0
#endif