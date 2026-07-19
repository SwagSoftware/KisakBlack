#pragma once

#undef GetObject

#define MAX_PRECACHE_ENTRIES 0x700 /*1792*/

enum scriptInstance_t : __int32
{                                                                             // XREF: ScriptTokenizer/r
                                                                                // ?Path_CallFunctionForNodes@@YAXW4scriptInstance_t@@P6AX0PAUpathnode_t@@@Z@Z/r ...
        SCRIPTINSTANCE_SERVER = 0x0,
        SCRIPTINSTANCE_CLIENT = 0x1,
        SCRIPT_INSTANCE_MAX     = 0x2,
};

struct ScriptFunctions // sizeof=0xC
{                                                                             // XREF: ?CGScr_LoadClientScriptsAndAnims@@YAXXZ/r
                                                                                // CGScr_LoadScriptsAndAnims/r
        int maxSize;                                                // XREF: CGScr_LoadClientScriptsAndAnims(void)+9/w
                                                                                // CGScr_LoadClientScriptsAndAnims(void)+43/w ...
        int count;                                                    // XREF: CGScr_LoadClientScriptsAndAnims(void)+10/w
                                                                                // CGScr_LoadClientScriptsAndAnims(void)+40/r ...
        int *address;                                             // XREF: CGScr_LoadClientScriptsAndAnims(void)+1D/w
                                                                                // CGScr_LoadScriptsAndAnims+1D/w
};

struct PrecacheEntry // sizeof=0x8
{                                                                             // XREF: ?Scr_LoadScript@@YAIW4scriptInstance_t@@PBD@Z/r
        unsigned __int16 filename;
        bool include;
        // padding byte
        unsigned int sourcePos;
};

enum fieldtype_t : __int32;




struct __declspec(align(4)) RefString // sizeof=0x8
{
        //$DA181ABD9DD4DBD447601B1FE2E6E195 ___u0;
        union //$DA181ABD9DD4DBD447601B1FE2E6E195 // sizeof=0x4
        {                                                                             // XREF: RefString/r
                //$119B815E6C15BED54461C272BD343858 __s0;
                struct //$119B815E6C15BED54461C272BD343858 // sizeof=0x4
                {                                                                             // XREF: $DA181ABD9DD4DBD447601B1FE2E6E195/r
                        unsigned __int32 refCount : 16;
                        unsigned __int32 user : 8;
                        unsigned __int32 byteLen : 8;
                };
                //volatile int data;
                volatile unsigned int data;
        };
        char str[1];
        // padding byte
        // padding byte
        // padding byte
};

namespace XExpr
{
        enum MathTypes : __int32
        {                                                                             // XREF: ParseValue/r
                                                                                        // StandardParams/r ...
                NUMBER = 0x0,                             // XREF: .data:g_stdParamList/s
                VECTOR_2D = 0x1,
                VECTOR_3D = 0x2,
                MAX_MATH_TYPES = 0x3,
        };

        enum MathOpcodes : __int32
        {                                                                             // XREF: ExpressionOperatorData/r
                OP_PUSH_VALUE = 0x0,
                OP_PUSH_PARAM = 0x1,
                OP_PUSH_STD_PARAM = 0x2,
                OP_ADD_VALUES = 0x3,                 // XREF: .rdata:g_ExprOperatorList/s
                OP_SUBTRACT_2_FROM_1 = 0x4,                 // XREF: .rdata:g_ExprOperatorList/s
                OP_MULTIPLY_VALUES = 0x5,                 // XREF: .rdata:g_ExprOperatorList/s
                OP_DIVIDE_2_FROM_1 = 0x6,                 // XREF: .rdata:g_ExprOperatorList/s
                OP_CONSTRUCT_VEC2 = 0x7,                 // XREF: .rdata:g_ExprOperatorList/s
                OP_CONSTRUCT_VEC3 = 0x8,                 // XREF: .rdata:g_ExprOperatorList/s
                OP_DOT_PRODUCT = 0x9,                 // XREF: .rdata:g_ExprOperatorList/s
                OP_CROSS_PRODUCT = 0xA,                 // XREF: .rdata:g_ExprOperatorList/s
                MAX_NUM_OPCODES = 0xB,                 // XREF: .rdata:g_ExprOperatorList/s
        };

        enum ParamTypes : __int32
        {                                                                             // XREF: ParseParameter/r
                PT_FLOAT = 0x0,
                PT_NORM_FLOAT = 0x1,
                PT_VEC2 = 0x2,
                PT_NORM_VEC2 = 0x3,
                PT_UNIT_VEC2 = 0x4,
                PT_VEC3 = 0x5,
                PT_NORM_VEC3 = 0x6,
                PT_UNIT_VEC3 = 0x7,
                PT_MAX_PARAM_TYPES = 0x8,
        };
}

enum ExpressionParseTypes : __int32
{                                       // XREF: ParseTopRankExpr/r
                                        // BuildExpr/r ...
    ET_PLUS = 0x0,
    ET_MINUS = 0x1,
    END_OF_RANK1 = 0x2,
    ET_MULTIPLY = 0x2,
    ET_DIVIDE = 0x3,
    END_OF_RANK2 = 0x4,
    ET_LEFT_PAREN = 0x4,
    ET_RIGHT_PAREN = 0x5,
    ET_VEC2_CONST = 0x6,
    ET_VEC3_CONST = 0x7,
    ET_DOT_PROD_FUNC = 0x8,
    ET_CROSS_PROD_FUNC = 0x9,
    ET_SINGLE_VALUE = 0xA,
    ET_GET_PARAMETER = 0xB,
    ET_GET_STD_PARAM = 0xC,
    END_OF_RANK3 = 0xD,
    ETS_COMMA = 0xD,
    NUM_EXPR_TYPES = 0xE,
};

struct ParseValue;
struct ExpressionOperatorData // sizeof=0x14
{                                       // XREF: .rdata:g_ExprOperatorList/r
    int iArity;
    const char *strValue;               // XREF: DetermineParseType+76/r
    // ConvertParseTypeToStr+10/r
    bool isFunction;                    // XREF: ParseBottomRankExpr+B5/r
    // padding byte
    // padding byte
    // padding byte
    XExpr::MathTypes(__cdecl *FuncTypeTesting)(const ParseValue *, int, scriptInstance_t);
    XExpr::MathOpcodes eMathType;
};

struct ParseValue // sizeof=0x8
{                                                                             // XREF: ParseTopRankExpr+89/w
                                                                                // ParseRank2Expr+89/w ...
        unsigned int scriptExprData;                // XREF: StoreExprInList(scriptInstance_t,uint,ParseValue)+65/r
        // ParseNode+637/w ...
        XExpr::MathTypes exprType;                    // XREF: StoreExprInList(scriptInstance_t,uint,ParseValue)+4A/r
        // ParseNode+644/w ...
};

struct ValueEntry // sizeof=0x8
{                                       // XREF: ParseNode/r
    unsigned int valueName;             // XREF: ParseNode+2CB/w
    // ParseNode+343/r
    unsigned int theValue;              // XREF: ParseNode+361/w
    // ParseNode+D72/r ...
};

struct ParseEntry // sizeof=0xC
{                                       // XREF: ParseNode/r
    ParseValue theValue;                // XREF: ParseNode+637/w
    // ParseNode+644/w ...
    unsigned int optionName;            // XREF: ParseNode+59D/w
    // ParseNode+CA2/r
};

union HashEntry_u // sizeof=0x4
{                                       // XREF: HashEntry/r
    unsigned int prev;
    unsigned int str;
};

struct HashEntry // sizeof=0x8
{                                       // XREF: .data:HashEntry * gScrStringHashTable/r
    unsigned int status_next;           // XREF: _Com_InitHunkMemory+95/o
                                        // .data:scrStringGlob_t * gScrStringGlob/o
    HashEntry_u u;
};

struct __declspec(align(128)) scrStringGlob_t // sizeof=0x100
{                                       // XREF: .data:scrStringGlob_t * gScrStringGlob/r
    HashEntry *hashTable;               // XREF: SL_Init(scriptInstance_t)+59/r
                                        // SL_Init(scriptInstance_t)+D0/r ...
    bool inited;                        // XREF: SL_Init(scriptInstance_t)+C/r
                                        // SL_Init(scriptInstance_t)+1B2/w ...
    // padding byte
    // padding byte
    // padding byte
    HashEntry *nextFreeEntry;           // XREF: SL_FreeString+198/w
                                        // SL_ShutdownSystem(scriptInstance_t,uint)+EF/w ...
    int indentLevel;                    // XREF: Scr_LoadScriptInternal(scriptInstance_t,char const *,PrecacheEntry *,int)+212/r
                                        // Scr_LoadScriptInternal(scriptInstance_t,char const *,PrecacheEntry *,int)+221/w ...
    int stringsUsed[32];                // XREF: Scr_LoadScriptInternal(scriptInstance_t,char const *,PrecacheEntry *,int)+239/r
                                        // Scr_LoadScriptInternal(scriptInstance_t,char const *,PrecacheEntry *,int)+4A0/r ...
};

bool __cdecl Scr_IsInOpcodeMemory(scriptInstance_t inst, const char *pos);
char __cdecl Scr_IsIdentifier(const char *token);
int __cdecl Scr_GetFunctionHandle(scriptInstance_t inst, const char *filename, const char *name);
unsigned int __cdecl SL_TransferToCanonicalString(scriptInstance_t inst, unsigned int stringValue);
unsigned int __cdecl SL_GetCanonicalString(scriptInstance_t inst, char *str);
void __cdecl Scr_BeginLoadScripts(scriptInstance_t inst, int user);
void __cdecl SL_BeginLoadScripts(scriptInstance_t inst);
void __cdecl Scr_BeginLoadAnimTrees(scriptInstance_t inst, int user);
int __cdecl Scr_ScanFile(scriptInstance_t inst, char *buf, int max_size);
unsigned int __cdecl Scr_LoadScriptInternal(
                scriptInstance_t inst,
                const char *filename,
                PrecacheEntry *entries,
                int entriesCount);
unsigned int __cdecl Scr_LoadScript(scriptInstance_t inst, char *filename);
void __cdecl Scr_PostCompileScripts(scriptInstance_t inst);
void __cdecl Scr_EndLoadScripts(scriptInstance_t inst);
void __cdecl Scr_PrecacheAnimTrees(scriptInstance_t inst, void *(__cdecl *Alloc)(int), int user, bool modChecksum);
void __cdecl Scr_EndLoadAnimTrees(scriptInstance_t inst);
void __cdecl Scr_FreeScripts(scriptInstance_t inst, unsigned __int8 sys);
void __cdecl Scr_GetGenericField(
                unsigned __int8 *b,
                fieldtype_t type,
                int ofs,
                scriptInstance_t inst,
                unsigned int whichbits);
void __cdecl Scr_SetGenericField(
                unsigned __int8 *b,
                fieldtype_t type,
                int ofs,
                scriptInstance_t inst,
                unsigned int whichbits);
