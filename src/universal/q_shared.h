#pragma once

// this is the define for determining if we have an asm version of a C function
#if (defined _M_IX86 || defined __i386__) && !defined __sun__    && !defined __LCC__
#define id386	1
#else
#define id386	0
#endif

// for windows fastcall option
#define	QDECL

#include <universal/dvar.h>
#include <universal/com_math.h>
#include <universal/assertive.h>
#include <universal/profile.h>

#include <cstdint>
#include <cmath>

//======================= WIN32 DEFINES =================================
#ifdef WIN32

#define	MAC_STATIC

#undef QDECL
#define	QDECL	__cdecl

// buildstring will be incorporated into the version string
#ifdef NDEBUG
#ifdef _M_IX86
#define	CPUSTRING	"win-x86"
#elif defined _M_ALPHA
#define	CPUSTRING	"win-AXP"
#endif
#else
#ifdef _M_IX86
#define	CPUSTRING	"win-x86-debug"
#elif defined _M_ALPHA
#define	CPUSTRING	"win-AXP-debug"
#endif
#endif

// angle indexes
#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

#define ID_INLINE __inline 

int __cdecl ShortSwap(__int16 l);
int __cdecl LongSwap(int l);

static ID_INLINE short BigShort(short l) { return (short)ShortSwap(l); }
#define LittleShort
static ID_INLINE int BigLong(int l) { return LongSwap(l); }
#define LittleLong
//static ID_INLINE float BigFloat(const float *l) { FloatSwap(l); }
#define LittleFloat

#define	PATH_SEP '\\'

#else // !WIN32 — portable (Linux/POSIX) equivalents of the block above

#define	MAC_STATIC

#undef QDECL
#define	QDECL

#define	CPUSTRING	"linux-x86"

// angle indexes
#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

#define ID_INLINE inline

int ShortSwap(__int16 l);
int LongSwap(int l);

static ID_INLINE short BigShort(short l) { return (short)ShortSwap(l); }
#define LittleShort
static ID_INLINE int BigLong(int l) { return LongSwap(l); }
#define LittleLong
#define LittleFloat

#define	PATH_SEP '/'

#endif // WIN32


#define MAX_LOCAL_CENTITIES 0x600

#define	MAX_GENTITIES		(1024) // 0x400

#define ENTITYNUM_WORLD (MAX_GENTITIES - 2)
#define ENTITYNUM_NONE (MAX_GENTITIES - 1)

//=============================================
void I_strncat(char *dest, int size, const char *src);
void I_strncpyz(char *dest, const char *src, int destsize);
int I_stricmp(const char *s0, const char *s1);
int I_strnicmp(const char *s0, const char *s1, int n);
const char *__cdecl I_stristr(const char *s0, const char *substr);

bool I_islower(int c);
bool I_isupper(int c);
bool I_isalpha(int c);

bool I_iscsym(int c);

bool __cdecl I_isdigit(int c);
bool __cdecl I_isalnum(int c);
bool __cdecl I_isforfilename(int c);
int __cdecl I_strncmp(const char *s0, const char *s1, int n);
int __cdecl I_strcmp(const char *s0, const char *s1);
int __cdecl I_stricmpwild(const char *wild, const char *s);
char *__cdecl I_strlwr(char *s);
char *__cdecl I_strupr(char *s);
int __cdecl I_DrawStrlen(const char *str);
char *__cdecl I_CleanStr(char *string);
unsigned __int8 __cdecl I_CleanChar(unsigned __int8 character);

// lwss: fcking winblows!
#undef MAKEWORD
#undef MAKELONG
#undef LOWORD
#undef HIWORD
#undef LOBYTE
#undef HIBYTE

#if defined(__GNUC__)
typedef                    long long ll;
typedef unsigned long long ull;
#define __int64 long long
#define __int32 int
#define __int16 short
#define __int8    char
#define MAKELL(num) num ## LL
#define FMT_64 "ll"
#elif defined(_MSC_VER)
typedef                    __int64 ll;
typedef unsigned __int64 ull;
#define MAKELL(num) num ## i64
#define FMT_64 "I64"
#elif defined (__BORLANDC__)
typedef                    __int64 ll;
typedef unsigned __int64 ull;
#define MAKELL(num) num ## i64
#define FMT_64 "L"
#else
#error "unknown compiler"
#endif
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

typedef     signed char     int8; // LWSS: slight changes here made to conform with steam api...
typedef     signed char     sint8;
typedef unsigned char     uint8;
typedef                    short    int16;
typedef     signed short    sint16;
typedef unsigned short    uint16;
typedef __int32				int32;
typedef unsigned __int32 uint32;
typedef signed long			sint32;
typedef ll                            int64;
typedef ll                            sint64;
typedef ull                         uint64;

// Partially defined types. They are used when the decompiler does not know
// anything about the type except its size.
#define _BYTE    uint8
#define _WORD    uint16
#define _DWORD uint32
#define _QWORD uint64
#if !defined(_MSC_VER)
#define _LONGLONG __int128
#endif

// Cod4 specific types
#define psize_int uintptr_t

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

#ifndef M_2_PI
#define M_2_PI (M_PI * 2.0)
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

#define M_PI_HALF (M_PI / 2.0) // LWSS ADD

#define PI_DIV_180		0.017453292519943295769236907684886
#define INV_PI_DIV_180	57.295779513082320876798154814105

// Punish Aurelio if you don't like these performance enhancements. :-)
#define DEG2RAD( a ) ( ( (a) * PI_DIV_180 ) )
#define RAD2DEG( a ) ( ( (a) * INV_PI_DIV_180 ) )

// Some convenience macros to make partial accesses nicer
#define LAST_IND(x,part_type)        (sizeof(x)/sizeof(part_type) - 1)
#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN
#    define LOW_IND(x,part_type)     LAST_IND(x,part_type)
#    define HIGH_IND(x,part_type)    0
#else
#    define HIGH_IND(x,part_type)    LAST_IND(x,part_type)
#    define LOW_IND(x,part_type)     0
#endif
// first unsigned macros:
#define BYTEn(x, n)     (*((_BYTE*)&(x)+n))
#define WORDn(x, n)     (*((_WORD*)&(x)+n))
#define DWORDn(x, n)    (*((_DWORD*)&(x)+n))

#define LOBYTE_NAMEHACK(x)    BYTEn(x,LOW_IND(x,_BYTE))
#define LOBYTE(x)    BYTEn(x,LOW_IND(x,_BYTE))
#define LOWORD(x)    WORDn(x,LOW_IND(x,_WORD))
#define LOWORD_NAMEHACK(x)    WORDn(x,LOW_IND(x,_WORD)) // windows. 
#define LODWORD(x) DWORDn(x,LOW_IND(x,_DWORD))
#define HIBYTE(x)    BYTEn(x,HIGH_IND(x,_BYTE))
#define HIBYTE_NAMEHACK(x)    BYTEn(x,HIGH_IND(x,_BYTE))
#define HIWORD(x)    WORDn(x,HIGH_IND(x,_WORD))
#define HIDWORD(x) DWORDn(x,HIGH_IND(x,_DWORD))
#define BYTE1(x)     BYTEn(x,    1)                 // byte 1 (counting from 0)
#define BYTE2(x)     BYTEn(x,    2)
#define BYTE3(x)     BYTEn(x,    3)
#define BYTE4(x)     BYTEn(x,    4)
#define BYTE5(x)     BYTEn(x,    5)
#define BYTE6(x)     BYTEn(x,    6)
#define BYTE7(x)     BYTEn(x,    7)
#define BYTE8(x)     BYTEn(x,    8)
#define BYTE9(x)     BYTEn(x,    9)
#define BYTE10(x)    BYTEn(x, 10)
#define BYTE11(x)    BYTEn(x, 11)
#define BYTE12(x)    BYTEn(x, 12)
#define BYTE13(x)    BYTEn(x, 13)
#define BYTE14(x)    BYTEn(x, 14)
#define BYTE15(x)    BYTEn(x, 15)
#define WORD1(x)     WORDn(x,    1)
#define WORD2(x)     WORDn(x,    2)                 // third word of the object, unsigned
#define WORD3(x)     WORDn(x,    3)
#define WORD4(x)     WORDn(x,    4)
#define WORD5(x)     WORDn(x,    5)
#define WORD6(x)     WORDn(x,    6)
#define WORD7(x)     WORDn(x,    7)

// now signed macros (the same but with sign extension)
#define SBYTEn(x, n)     (*((int8*)&(x)+n))
#define SWORDn(x, n)     (*((int16*)&(x)+n))
#define SDWORDn(x, n)    (*((int32*)&(x)+n))

#define SLOBYTE(x)    SBYTEn(x,LOW_IND(x,int8))
#define SLOWORD(x)    SWORDn(x,LOW_IND(x,int16))
#define SLODWORD(x) SDWORDn(x,LOW_IND(x,int32))
#define SHIBYTE(x)    SBYTEn(x,HIGH_IND(x,int8))
#define SHIWORD(x)    SWORDn(x,HIGH_IND(x,int16))
#define SHIDWORD(x) SDWORDn(x,HIGH_IND(x,int32))
#define SBYTE1(x)     SBYTEn(x,    1)
#define SBYTE2(x)     SBYTEn(x,    2)
#define SBYTE3(x)     SBYTEn(x,    3)
#define SBYTE4(x)     SBYTEn(x,    4)
#define SBYTE5(x)     SBYTEn(x,    5)
#define SBYTE6(x)     SBYTEn(x,    6)
#define SBYTE7(x)     SBYTEn(x,    7)
#define SBYTE8(x)     SBYTEn(x,    8)
#define SBYTE9(x)     SBYTEn(x,    9)
#define SBYTE10(x)    SBYTEn(x, 10)
#define SBYTE11(x)    SBYTEn(x, 11)
#define SBYTE12(x)    SBYTEn(x, 12)
#define SBYTE13(x)    SBYTEn(x, 13)
#define SBYTE14(x)    SBYTEn(x, 14)
#define SBYTE15(x)    SBYTEn(x, 15)
#define SWORD1(x)     SWORDn(x,    1)
#define SWORD2(x)     SWORDn(x,    2)
#define SWORD3(x)     SWORDn(x,    3)
#define SWORD4(x)     SWORDn(x,    4)
#define SWORD5(x)     SWORDn(x,    5)
#define SWORD6(x)     SWORDn(x,    6)
#define SWORD7(x)     SWORDn(x,    7)

// Generate a pair of operands. S stands for 'signed'
#define __SPAIR16__(high, low)    (((int16)    (high) <<    8) | (uint8) (low))
#define __SPAIR32__(high, low)    (((int32)    (high) << 16) | (uint16)(low))
#define __SPAIR64__(high, low)    (((int64)    (high) << 32) | (uint32)(low))
#define __SPAIR128__(high, low) (((int128) (high) << 64) | (uint64)(low))
#define __PAIR16__(high, low)     (((uint16) (high) <<    8) | (uint8) (low))
#define __PAIR32__(high, low)     (((uint32) (high) << 16) | (uint16)(low))
#define __PAIR64__(high, low)     (((uint64) (high) << 32) | (uint32)(low))
#define __PAIR128__(high, low)    (((uint128)(high) << 64) | (uint64)(low))

#define arr_esize(a) (sizeof((a)[0]))
#define arr_cnt(a) (sizeof(a)/arr_esize(a))
#define ARRAY_COUNT(a) arr_cnt(a)

union FloatWriteSwap_union // sizeof=0x4
{                                       // ...
    float f;
    int n;
    unsigned __int8 b[4];
};

union FloatReadSwap_union // sizeof=0x4
{                                       // ...
    float f;
    int n;
    unsigned __int8 b[4];
};

// rotate left
template<class T> T __ROL__(T value, int count)
{
        const uint nbits = sizeof(T) * 8;

        if (count > 0)
        {
                count %= nbits;
                T high = value >> (nbits - count);
                if (T(-1) < 0) // signed value
                        high &= ~((T(-1) << count));
                value <<= count;
                value |= high;
        }
        else
        {
                count = -count % nbits;
                T low = value << (nbits - count);
                value >>= count;
                value |= low;
        }
        return value;
}

inline uint8    __ROL1__(uint8    value, int count) { return __ROL__((uint8)value, count); }
inline uint16 __ROL2__(uint16 value, int count) { return __ROL__((uint16)value, count); }
inline uint32 __ROL4__(uint32 value, int count) { return __ROL__((uint32)value, count); }
inline uint64 __ROL8__(uint64 value, int count) { return __ROL__((uint64)value, count); }
inline uint8    __ROR1__(uint8    value, int count) { return __ROL__((uint8)value, -count); }
inline uint16 __ROR2__(uint16 value, int count) { return __ROL__((uint16)value, -count); }
inline uint32 __ROR4__(uint32 value, int count) { return __ROL__((uint32)value, -count); }
inline uint64 __ROR8__(uint64 value, int count) { return __ROL__((uint64)value, -count); }


//=============================================
void Com_Memset(void *dest, const int val, const size_t count);
void Com_Memcpy(void *dest, const void *src, const size_t count);

char *QDECL va(const char *format, ...);

inline void BLOPS_NULLSUB(void *fakearg = nullptr)
{
}
inline void BLOPS_NULLSUB()
{
}

inline int RETURN_ZERO32()
{
    return 0;
}

//#define COERCE_FLOAT(x) (*(float*)&(x))
// KISAK ADDITION: pray that the optimizer doesn't shit the bed
__forceinline static float COERCE_FLOAT(unsigned val) {
    union {
        unsigned v;
        float f;
    } lol = { val };
    return lol.f;
}

__forceinline static unsigned int COERCE_UNSIGNED_INT(float val) {
    union {
        float f;
        unsigned v;
    } lol = { val };
    return lol.v;
}

__forceinline static int COERCE_INT(float val) {
    union {
        float f;
        int v;
    } lol = { val };
    return lol.v;
}


//=============================================

enum trType_t : __int32
{                                       // XREF: ?Actor_BecomeCorpse@@YAHPAUgentity_s@@@Z/r
    TR_STATIONARY = 0x0,
    TR_INTERPOLATE = 0x1,
    TR_INTERPOLATE_MOVER = 0x2,
    TR_LINEAR = 0x3,
    TR_LINEAR_STOP = 0x4,
    TR_SINE = 0x5,
    TR_GRAVITY = 0x6,
    TR_LOW_GRAVITY = 0x7,
    TR_ACCELERATE = 0x8,
    TR_DECELERATE = 0x9,
    TR_PHYSICS = 0xA,
    TR_XDOLL = 0xB,
    TR_FIRST_RAGDOLL = 0xC,
    TR_RAGDOLL = 0xC,
    TR_RAGDOLL_GRAVITY = 0xD,
    TR_RAGDOLL_INTERPOLATE = 0xE,
    TR_LAST_RAGDOLL = 0xE,
    NUM_TRTYPES = 0xF,
};

struct cspField_t // sizeof=0xC
{                                                                             // XREF: .data:weaponDefFields/r
                                                                                // .data:destructiblePieceFields/r ...
        const char *szName;                                 // XREF: BG_ParsePenetrationDepthTable+13D/w
                                                                                // G_ParseHitLocDmgTable(void)+5D/w ...
        int iOffset;                                                // XREF: BG_ParsePenetrationDepthTable+156/w
                                                                                // G_ParseHitLocDmgTable(void)+70/w
        int iFieldType;                                         // XREF: BG_ParsePenetrationDepthTable+166/w
                                                                                // G_ParseHitLocDmgTable(void)+7D/w
};

struct orientation_t;

struct trajectory_t // sizeof=0x24
{                                                                             // XREF: LerpEntityState/r
                                                                                // LerpEntityState/r ...
        unsigned __int8 trType; // KISAKTODO: trType_t                         // XREF: CScriptMover_SetupMoveSpeed+6AD/w
                                                                                // CScr_LaunchRagdoll+147/w ...
        // padding byte
        // padding byte
        // padding byte
        int trTime;                                                 // XREF: CScriptMover_SetupMoveSpeed+6C9/w
                                                                                // CScr_LaunchRagdoll+14E/w ...
        int trDuration;                                         // XREF: CScriptMover_SetupMoveSpeed+6DF/w
                                                                                // CScriptMover_SetupMoveSpeed+7E0/r ...
        float trBase[3];                                        // XREF: CScriptMover_SetupMoveSpeed+6E9/w
                                                                                // CScriptMover_SetupMoveSpeed+6F6/w ...
        float trDelta[3];                                     // XREF: CScriptMover_SetupMoveSpeed+70F/w
                                                                                // CScriptMover_SetupMoveSpeed+71C/w ...
};

struct qtime_s // sizeof=0x24
{                                       // XREF: SaveHeader/r qtime_t/r ...
    int tm_sec;                         // XREF: CG_DrawWristWatch+18B/r
    int tm_min;                         // XREF: CG_DrawWristWatch+8A/r
    int tm_hour;                        // XREF: CG_DrawWristWatch+97/r
    int tm_mday;                        // XREF: UI_StartServerRefresh(int,int,int)+C2/r
    int tm_mon;                         // XREF: UI_StartServerRefresh(int,int,int)+C6/r
    int tm_year;                        // XREF: UI_StartServerRefresh(int,int,int)+B9/r
    int tm_wday;
    int tm_yday;                        // XREF: Demo_Screenshot_f(void)+10C/r
    int tm_isdst;
};

static const float colorBlack[4] = { 0.0, 0.0, 0.0, 1.0 }; // idb
static const float colorBlackBlank[4] = { 0.0, 0.0, 0.0, 0.0 };
static const float colorRed[4] = { 1.0, 0.0, 0.0, 1.0 }; // idb
static const float colorGreen[4] = { 0.0, 1.0, 0.0, 1.0 }; // idb
static const float colorLtGreen[4] = { 0.0, 0.69999999f, 0.0, 1.0 }; // idb
static const float colorMdGreen[4] = { 0.0, 0.5, 0.0, 1.0 };
static const float colorBlue[4] = { 0.0, 0.0, 1.0, 1.0 }; // idb
static const float colorLtBlue[4] = { 0.5, 0.5, 1.0, 1.0 }; // idb
static const float colorMdBlue[4] = { 0.0, 0.0, 0.5, 1.0 };
static const float colorYellow[4] = { 1.0, 1.0, 0.0, 1.0 }; // idb
static const float colorLtYellow[4] = { 0.75, 0.75f, 0.0f, 1.0f };
static const float colorMdYellow[4] = { 0.5, 0.5, 0.0, 1.0 }; // idb
static const float colorMagenta[4] = { 1.0, 0.0, 1.0, 1.0 }; // idb
static const float colorCyan[4] = { 0.0f, 1.0f, 1.0f, 1.0f };
static const float colorLtCyan[4] = { 0.0, 0.75, 0.75, 1.0 }; // idb
static const float colorMdCyan[4] = { 0.0f, 0.5f, 0.5f, 1.0f };
static const float colorDkCyan[4] = { 0.0f, 0.25f, 0.25f, 1.0f };
static const float colorWhite[4] = { 1.0, 1.0, 1.0, 1.0 }; // idb
static const float colorLtGrey[4] = { 0.75, 0.75, 0.75, 1.0 }; // idb
static const float colorMdGrey[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
static const float colorDkGrey[4] = { 0.25f, 0.25f, 0.25f, 1.0f };
static const float colorOrange[4] = { 1.0, 0.69999999f, 0.0, 1.0 }; // idb
static const float colorLtOrange[4] = { 0.75, 0.52499998f, 0.0, 1.0 }; // idb
static const float colorWhiteFaded[4] = { 1.0, 1.0, 1.0, 0.75 }; // idb
static const float colorGreenFaded[4] = { 0.0, 1.0, 0.0, 0.75 }; // idb
static const float colorRedFaded[4] = { 0.75, 0.25, 0.0, 0.75 }; // idb
static const float colorMdRed[4] = { 0.5, 0.0, 0.0, 1.0 };
static const float colorDkRed[4] = { 0.25, 0.0, 0.0, 1.0 };
static const float colorDkYellow[4] = { 0.25, 0.25, 0.0, 1.0 };
static const float colorDkGreen[4] = { 0.0, 0.25, 0.0, 1.0 };




struct usercmd_s;

unsigned __int8 __cdecl ColorIndex(unsigned __int8 c);
const char *__cdecl Com_GetFilenameSubString(const char *pathname);
const char *__cdecl Com_GetExtensionSubString(const char *filename);
void __cdecl Com_StripExtension(char *in, char *out);
void __cdecl Com_DefaultExtension(char *path, unsigned int maxSize, const char *extension);
__int16 __cdecl BigShort(__int16 l);
unsigned __int64 __cdecl LittleLong64(unsigned __int64 l);
int __cdecl ShortSwap(__int16 l);
__int16 __cdecl ShortNoSwap(__int16 l);
int __cdecl LongSwap(int l);
unsigned __int64 __cdecl Long64Swap(unsigned __int64 l);
unsigned __int64 __cdecl Long64NoSwap(unsigned __int64 ll);
double __cdecl FloatReadSwap(int n);
double __cdecl FloatReadNoSwap(int n);
FloatWriteSwap_union __cdecl FloatWriteSwap(float f);
void __cdecl Swap_InitLittleEndian();
void __cdecl Swap_InitBigEndian();
void __cdecl Swap_Init();
__int64 __cdecl I_atoi64(const char *str);
bool __cdecl I_islower(int c);
bool __cdecl I_isupper(int c);
bool __cdecl I_isalpha(int c);
bool __cdecl I_isdigit(int c);
bool __cdecl I_isalnum(int c);
bool __cdecl I_isforfilename(int c);
void __cdecl I_strncpyz(char *dest, const char *src, int destsize);
int __cdecl I_strnicmp(const char *s0, const char *s1, int n);
int __cdecl I_strncmp(const char *s0, const char *s1, int n);
const char *__cdecl I_stristr(const char *s0, const char *substr);
int __cdecl I_stricmp(const char *s0, const char *s1);
int __cdecl I_strcmp(const char *s0, const char *s1);
int __cdecl I_stricmpwild(const char *wild, const char *s);
char *__cdecl I_strlwr(char *s);
char *__cdecl I_strupr(char *s);
bool __cdecl I_iscsym(int c);
void __cdecl I_strncat(char *dest, int size, const char *src);
int __cdecl I_DrawStrlen(const char *str);
char *__cdecl I_CleanStr(char *string);
unsigned __int8 __cdecl I_CleanChar(unsigned __int8 character);
int Com_sprintf(char *dest, unsigned int size, const char *fmt, ...);
int Com_sprintfPos(char *dest, int destSize, int *destPos, const char *fmt, ...);
bool __cdecl CanKeepStringPointer(char *string);
char *__cdecl I_itoa(int value, char *buf, int bufsize);
void __cdecl strreverse(char *begin, char *end);
char *va(const char *format, ...);
void __cdecl Com_InitThreadData(int threadContext);
char *__cdecl Info_ValueForKey(char *s, char *key);
inline char *__cdecl Info_ValueForKey(char *s, const char *key)
{
    return Info_ValueForKey(s, (char *)key);
}
void __cdecl Info_NextPair(const char **head, char *key, char *value);
void __cdecl Info_RemoveKey(char *s, char *key);
void __cdecl Info_RemoveKey_Big(char *s, char *key);
bool __cdecl Info_Validate(char *s);
void __cdecl Info_SetValueForKey(char *s, char *key, const char *value);
inline void __cdecl Info_SetValueForKey(char *s, const char *key, const char *value)
{
    Info_SetValueForKey(s, (char *)key, value);
}
void __cdecl Info_SetValueForKey_Big(char *s, char *key, const char *value);
bool __cdecl ParseConfigStringToStruct(
                unsigned __int8 *pStruct,
                const cspField_t *pFieldList,
                int iNumFields,
                char *pszBuffer,
                int iMaxFieldTypes,
                int (__cdecl *parseSpecialFieldType)(unsigned __int8 *, const char *, const int, const int),
                void (__cdecl *parseStrCpy)(unsigned __int8 *, const char *));
bool __cdecl KeyValueToField(
                unsigned __int8 *pStruct,
                const cspField_t *pField,
                char *pszKeyValue,
                int iMaxFieldTypes,
                int (__cdecl *parseSpecialFieldType)(unsigned __int8 *, const char *, const int, const int),
                void (__cdecl *parseStrcpy)(unsigned __int8 *, const char *));
bool __cdecl ParseConfigStringToStructMerged(
                unsigned __int8 *pStruct,
                cspField_t *pFieldList,
                int iNumFields,
                const char *mergedName,
                const char **pszBuffer,
                const char **sourceName,
                char *pszMergedBuffer,
                int iMaxFieldTypes,
                int (__cdecl *parseSpecialFieldType)(unsigned __int8 *, const char *, const int, const int),
                void (__cdecl *parseStrCpy)(unsigned __int8 *, const char *),
                int (__cdecl *parseMergeSpecialCase)(const char *, char **, char *, int));
double __cdecl GetLeanFraction(float fFrac);
double __cdecl UnGetLeanFraction(float fFrac);
void __cdecl AddLeanToPosition(float *position, float fViewYaw, float fLeanFrac, float fViewRoll, float fLeanDist);
void __cdecl OrientationPosToWorldPos(const orientation_t *orient, const float *pos, float *out);
void __cdecl OrientationDirToWorldDir(const orientation_t *orient, const float *dir, float *out);
void __cdecl OrientationConcatenate(const orientation_t *orFirst, const orientation_t *orSecond, orientation_t *out);
void __cdecl OrientationInvert(const orientation_t *orient, orientation_t *out);
bool __cdecl Com_IsLegacyXModelName(const char *name);
double __cdecl CMD_GetAnalogButtonValue(usercmd_s *cmd, unsigned int whichBit);

static const int iEdgePairs[12][2] =
{
  { 0, 1 },
  { 0, 2 },
  { 0, 4 },
  { 1, 3 },
  { 1, 5 },
  { 2, 3 },
  { 2, 6 },
  { 3, 7 },
  { 4, 5 },
  { 4, 6 },
  { 5, 7 },
  { 6, 7 }
};

extern const dvar_s *useFastFile;
extern const dvar_s *sys_smp_allowed;

inline bool IsFastFileLoad()
{
#ifdef KISAK_NO_FASTFILES
    return false;
#endif
    return useFastFile->current.enabled;
}

#define IsObjFileLoad() IsFastFileLoad()

extern const dvar_t *dedicated;
inline bool IsDedicatedServer()
{
#ifdef KISAK_DEDICATED
    return true;
#else
    return dedicated && dedicated->current.integer;
#endif
}

#ifndef KISAK_DEDICATED
    #ifdef KISAK_SP
        #define DEFAULT_CFG "default.cfg"
    #else
        #define DEFAULT_CFG "default_mp.cfg"
    #endif
#else
    #define DEFAULT_CFG "default_dedicated.cfg"
#endif

template <typename T>
inline void AssignToSmallerType(T *dest, int src)
{
    *dest = (T)src;
    iassert((int)*dest == src);
}

static inline unsigned int abs8(int8_t v)
{
    int x = v;
    int mask = x >> 31;
    return (unsigned int)((x + mask) ^ mask);
}


// KISAKTODO: remove
template <typename T>
__forceinline T _Pow_int(T base, int exp)
{
    return (T)powf(base, exp);
}

template <typename T, typename U>
inline constexpr T truncate_cast(U value)
{
    static_assert(std::is_integral_v<T>, "truncate_cast target must be integral");
    static_assert(std::is_integral_v<U>, "truncate_cast source must be integral");

    iassert(value == (T)value);

    return static_cast<T>(value);
}