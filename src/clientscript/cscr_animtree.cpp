#include "cscr_animtree.h"
#include <universal/q_shared.h>
#include <universal/q_parse.h>
#include "cscr_parser.h"
#include "cscr_variable.h"
#include "cscr_stringlist.h"
#include "cscr_memorytree.h"

#include <Windows.h>
#include <qcommon/common.h>
#include <xanim/xanim.h>
#include <universal/com_memory.h>
#include <bgame/bg_local.h>
#include <server_mp/sv_main_mp.h>

#undef GetObject

const char *VALUES_ROOT_NAME =
"valueRoot";
const char *NODE_ROOT_NAME =
"nodeRoot";
const char *EXPRESSION_ROOT_NAME =
"exprRoot";
const char *PARAMETER_ROOT_NAME =
"paramRoot";
const char *VALUE_PARAM_INDEX =
"valueindex";
const char *PARAM_CLASS =
"class";
const char *PARAM_EXPR_INDEX =
"index";
const char *PARAM_TYPE =
"type";
const char *OPTION_NAME =
"name";
const char *OPTION_EXPR_INDEX =
"index";
const char *EXPR_OPERATOR_NAME =
"command";
const char *EXPR_VALUE_TYPE =
"type";
const char *EXPR_VALUE_NAME =
"value";
const char *EXPR_OPERAND_NAME =
"operand";


const ExpressionOperatorData g_ExprOperatorList[10] =
{
  { 2, "+", false, EqualTypeSameResult, XExpr::MathOpcodes::OP_ADD_VALUES },
  { 2, "-", false, EqualTypeSameResult, XExpr::MathOpcodes::OP_SUBTRACT_2_FROM_1 },
  { 2, "*", false, EqualTypeAllowScalar, XExpr::MathOpcodes::OP_MULTIPLY_VALUES },
  { 2, "/", false, EqualTypeAllowScalar, XExpr::MathOpcodes::OP_DIVIDE_2_FROM_1 },
  { 0, "(", false, NULL, XExpr::MathOpcodes::MAX_NUM_OPCODES },
  { 0, ")", false, NULL, XExpr::MathOpcodes::MAX_NUM_OPCODES },
  { 2, "vec2", true, FloatsToVec, XExpr::MathOpcodes::OP_CONSTRUCT_VEC2 },
  { 3, "vec3", true, FloatsToVec, XExpr::MathOpcodes::OP_CONSTRUCT_VEC3 },
  { 2, "dot", true, VecsToFloat, XExpr::MathOpcodes::OP_DOT_PRODUCT },
  { 2, "cross", true, Vec3ToVec3, XExpr::MathOpcodes::OP_CROSS_PRODUCT }
};


struct StandardParams // sizeof=0x8
{                                       // XREF: .data:g_stdParamList/r
    const char *strParamName;           // XREF: GetStdParamIx+2C/r
    XExpr::MathTypes eType;             // XREF: ParseStdParameter+55/r
};

StandardParams g_stdParamList[1] =
{ { "currTimeSec", XExpr::MathTypes::NUMBER } };


using namespace XExpr;


const char *propertyNames[7] =
{
  "loopsync",
  "nonloopsync",
  "complete",
  "additive",
  "client",
  "separate",
  "forceload"
};




const char *g_strParamTypeToName[8] =
{ "float", "nfloat", "vec2", "nvec2", "uvec2", "vec3", "nvec3", "uvec3" };
const char *g_strAccessClassNames[3] =
{ "external", "inte", "client" };


static thread_local bool g_bChangeChecksum;

ClientTreeStorage *g_pCurrClientData;

ClientTreeStorage gGScrXAnimTreesForClient[2][128];
scrAnimGlob_t gScrAnimGlob[2];
scrAnimPub_t gScrAnimPub[2];

static int iFoo_0;
static int iFoo_1;
static int iFoo_2;

XExpr::MathTypes __cdecl EqualTypeSameResult(const ParseValue *params, int iNumParams, scriptInstance_t inst)
{
    const char *v3; // eax

    if ( iNumParams == 2 )
    {
        if ( params->exprType == params[1].exprType )
            return params->exprType;
        else
            return XExpr::MAX_MATH_TYPES;
    }
    else
    {
        v3 = va("Cannot determine type due to parameter number. Expected %i, received %i.", 2, iNumParams);
        AnimTreeCompileError(inst, v3);
        return XExpr::MAX_MATH_TYPES;
    }
}

void __cdecl AnimTreeCompileError(scriptInstance_t inst, const char *msg)
{
    const char *pos; // [esp+0h] [ebp-4h]

    pos = Com_GetLastTokenPos();
    Com_EndParseSession();
    CompileError(inst, pos - gScrAnimGlob[inst].start, "%s", msg);
}

XExpr::MathTypes __cdecl EqualTypeAllowScalar(const ParseValue *params, int iNumParams, scriptInstance_t inst)
{
    const char *v3; // eax

    if ( iNumParams == 2 )
    {
        if ( params->exprType != params[1].exprType && params->exprType && params[1].exprType )
        {
            return XExpr::MAX_MATH_TYPES;
        }
        else if ( params->exprType == params[1].exprType )
        {
            return params->exprType;
        }
        else if ( params->exprType )
        {
            return params->exprType;
        }
        else
        {
            return params[1].exprType;
        }
    }
    else
    {
        v3 = va("Cannot determine type due to parameter number. Expected %i, received %i.", 2, iNumParams);
        AnimTreeCompileError(inst, v3);
        return XExpr::MAX_MATH_TYPES;
    }
}

XExpr::MathTypes __cdecl FloatsToVec(const ParseValue *params, int iNumParams, scriptInstance_t inst)
{
    const char *v3; // eax
    const char *v5; // eax
    int iLoop; // [esp+0h] [ebp-4h]

    if ( iNumParams >= 2 )
    {
        for ( iLoop = 0; iLoop < iNumParams; ++iLoop )
        {
            if ( params[iLoop].exprType )
            {
                v5 = va("Parameter number %i should be a number. Instead, it is %i.", iLoop, params[iLoop].exprType);
                AnimTreeCompileError(inst, v5);
                return (XExpr::MathTypes)3;
            }
        }
        return (XExpr::MathTypes)(iNumParams - 1);
    }
    else
    {
        v3 = va("Cannot determine type due to parameter number. Expected %i, received %i.", 2, iNumParams);
        AnimTreeCompileError(inst, v3);
        return (XExpr::MathTypes)3;
    }
}

XExpr::MathTypes __cdecl VecsToFloat(const ParseValue *params, int iNumParams, scriptInstance_t inst)
{
    const char *v3; // eax
    const char *v5; // eax
    int iLoop; // [esp+0h] [ebp-8h]
    XExpr::MathTypes eType; // [esp+4h] [ebp-4h]

    if ( iNumParams > 0 )
    {
        eType = params->exprType;
        if ( eType )
        {
            for ( iLoop = 1; iLoop < iNumParams; ++iLoop )
            {
                if ( eType != params[1].exprType )
                {
                    v5 = va(
                                 "Parameter number %i should be of type %i. It is instead of type %i",
                                 iLoop,
                                 eType,
                                 params[1].exprType);
                    AnimTreeCompileError(inst, v5);
                    return (XExpr::MathTypes)3;
                }
            }
            return (XExpr::MathTypes)0;
        }
        else
        {
            AnimTreeCompileError(inst, "Parameter number 0 should be a vector type. It is instead a number.");
            return (XExpr::MathTypes)3;
        }
    }
    else
    {
        v3 = va("Cannot determine type due to parameter number. Expected at least 1, received %i.", iNumParams);
        AnimTreeCompileError(inst, v3);
        return (XExpr::MathTypes)3;
    }
}

XExpr::MathTypes __cdecl Vec3ToVec3(const ParseValue *params, int iNumParams, scriptInstance_t inst)
{
    const char *v3; // eax

    if ( iNumParams == 2 )
    {
        if ( params->exprType == XExpr::VECTOR_3D )
        {
            if ( params[1].exprType == XExpr::VECTOR_3D )
            {
                return (XExpr::MathTypes)2;
            }
            else
            {
                AnimTreeCompileError(inst, "Parameter number 1 should be a vec3 type.");
                return (XExpr::MathTypes)3;
            }
        }
        else
        {
            AnimTreeCompileError(inst, "Parameter number 0 should be a vec3 type.");
            return (XExpr::MathTypes)3;
        }
    }
    else
    {
        v3 = va("Needs 2 parameters, received %i.", iNumParams);
        AnimTreeCompileError(inst, v3);
        return (XExpr::MathTypes)3;
    }
}

void __cdecl SetAnimCheck(int bAnimCheck, scriptInstance_t inst)
{
    gScrAnimGlob[inst].bAnimCheck = bAnimCheck;
}

void __cdecl Scr_EmitAnimation(scriptInstance_t inst, char *pos, unsigned int animName, unsigned int sourcePos)
{
    if (gScrAnimPub[inst].animTreeNames)
        Scr_EmitAnimationInternal(inst, pos, animName, gScrAnimPub[inst].animTreeNames);
    else
        CompileError(inst, sourcePos, "#using_animtree was not specified");
}

void __cdecl Scr_EmitAnimationInternal(scriptInstance_t inst, char *pos, unsigned int animName, unsigned int names)
{
    unsigned int NewVariable; // eax
    VariableValueInternal_u *VariableValueAddress; // eax
    unsigned int animId; // [esp+10h] [ebp-Ch]
    VariableValue tempValue; // [esp+14h] [ebp-8h] BYREF

    if ( !names
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp", 1097, 0, "%s", "names") )
    {
        __debugbreak();
    }
    if ( inst == SCRIPTINSTANCE_CLIENT )
        ++iFoo_0;
    animId = FindVariable(inst, names, animName);
    if ( animId )
    {
        VariableValueAddress = GetVariableValueAddress(inst, animId);
        *(unsigned int *)pos = VariableValueAddress->next;
        VariableValueAddress->next = (unsigned int)pos;
    }
    else
    {
        NewVariable = GetNewVariable(inst, names, animName);
        *(unsigned int *)pos = 0;
        tempValue.type = 7;
        tempValue.u.intValue = (int)pos;
        SetVariableValue(inst, NewVariable, &tempValue);
    }
}

void __cdecl SetVariableValue(scriptInstance_t inst, unsigned int id, VariableValue *value)
{
    VariableValueInternal *entryValue; // [esp+8h] [ebp-4h]

    if ( !id && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h", 587, 0, "%s", "id") )
        __debugbreak();
    if ( value->type >= 13
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    588,
                    0,
                    "%s",
                    "!IsObject( value )") )
    {
        __debugbreak();
    }
    if ( value->type >= 0x16u
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    589,
                    0,
                    "%s",
                    "value->type >= 0 && value->type < VAR_COUNT") )
    {
        __debugbreak();
    }
    if ( value->type == 10
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    590,
                    0,
                    "%s",
                    "value->type != VAR_STACK") )
    {
        __debugbreak();
    }
    entryValue = &gScrVarGlob[inst].variableList[id + 0x8000];
    if ( (entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    593,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) >= 0xD
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    594,
                    0,
                    "%s",
                    "!IsObject( entryValue )") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) == 0xA
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    595,
                    0,
                    "%s",
                    "(entryValue->w.type & VAR_MASK) != VAR_STACK") )
    {
        __debugbreak();
    }
    RemoveRefToValue(inst, entryValue->w.status & 0x1F, entryValue->u.u);
    entryValue->w.status &= 0xFFFFFFE0;
    entryValue->w.status |= value->type;
    entryValue->u.next = value->u.intValue;
}

// KISAKTODO: move to scr_variable
void __cdecl RemoveRefToValue(scriptInstance_t inst, int type, VariableUnion u)
{
    unsigned int value; // [esp+4h] [ebp-4h]

    value = type - 1;
    if ( (unsigned int)(type - 1) < 4 )
    {
        if ( type == 1 )
        {
            RemoveRefToObject(inst, u.stringValue);
        }
        else if ( value > 2 )
        {
            if ( type != 4
                && !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                            446,
                            0,
                            "%s\n\t(value) = %i",
                            "(value == VAR_VECTOR - VAR_BEGIN_REF)",
                            value) )
            {
                __debugbreak();
            }
            RemoveRefToVector(inst, u.vectorValue);
        }
        else
        {
            SL_RemoveRefToString(inst, u.stringValue);
        }
    }
}

void __cdecl RemoveRefToVector(scriptInstance_t inst, const float *vectorValue)
{
    if (!*((_BYTE *)vectorValue - 1))
    {
        _InterlockedExchangeAdd(&gScrVarPub[inst].totalVectorRefCount, 0xFFFFFFFF);
        if (gScrStringDebugGlob[inst])
        {
            if (gScrStringDebugGlob[inst]->refCount[((char *)(vectorValue - 1) - gScrMemTreePub[inst].mt_buffer) / 16] < 0
                && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    388,
                    0,
                    "%s",
                    "gScrStringDebugGlob[inst]->refCount[((char *)vectorValue - 4 - gScrMemTreePub[inst].mt_buffer) / MT_NODE_SIZE] >= 0"))
            {
                __debugbreak();
            }
            _InterlockedExchangeAdd(
                &gScrStringDebugGlob[inst]->refCount[((char *)(vectorValue - 1) - gScrMemTreePub[inst].mt_buffer) / 16],
                0xFFFFFFFF);
        }
        if (*((_WORD *)vectorValue - 2))
            --*((_WORD *)vectorValue - 2);
        else
            MT_Free((_BYTE *)vectorValue - 4, 16, inst);
    }
}

const char *EXPR_TYPE =
"exprType";
const char *EXPR_DATA =
"exprData";

unsigned int __cdecl StoreExprInList(scriptInstance_t inst, unsigned int exprDataArray, ParseValue exprData)
{
    unsigned int iEntryIndex; // [esp+0h] [ebp-Ch]
    unsigned int exprEntry; // [esp+4h] [ebp-8h]
    unsigned int id; // [esp+8h] [ebp-4h]

    iEntryIndex = GetArraySize(inst, exprDataArray);
    id = GetNewArrayVariable(inst, exprDataArray, iEntryIndex);
    exprEntry = GetArray(inst, id);
    SetIntVariable(inst, exprEntry, exprData.exprType, (char *)EXPR_TYPE);
    SetPtrVariable(inst, exprEntry, exprData.scriptExprData, (char *)EXPR_DATA);
    return iEntryIndex;
}

void __cdecl SetIntVariable(scriptInstance_t inst, unsigned int theArray, int integerValue, char *strVarName)
{
    unsigned int String; // eax
    VariableValue theExpr; // [esp+0h] [ebp-Ch] BYREF
    unsigned int theVar; // [esp+8h] [ebp-4h]

    String = SL_GetString_(inst, strVarName, 2u, 4);
    theVar = GetVariable(inst, theArray, String);
    theExpr.type = 6;
    theExpr.u.intValue = integerValue;
    SetNewVariableValue(inst, theVar, &theExpr);
}

void __cdecl SetPtrVariable(scriptInstance_t inst, unsigned int theArray, unsigned int arrayData, char *strVarName)
{
    unsigned int String; // eax
    VariableValue theExpr; // [esp+0h] [ebp-Ch] BYREF
    unsigned int theVar; // [esp+8h] [ebp-4h]

    String = SL_GetString_(inst, strVarName, 2u, 4);
    theVar = GetVariable(inst, theArray, String);
    theExpr.type = 1;
    theExpr.u.intValue = arrayData;
    SetNewVariableValue(inst, theVar, &theExpr);
}

unsigned int __cdecl StoreValueInList(scriptInstance_t inst, unsigned int valueDataArray, unsigned int valueData)
{
    unsigned int iEntryIndex; // [esp+Ch] [ebp-10h]
    VariableValue theValueVarData; // [esp+10h] [ebp-Ch] BYREF
    unsigned int id; // [esp+18h] [ebp-4h]

    iEntryIndex = GetArraySize(inst, valueDataArray);
    id = GetNewArrayVariable(inst, valueDataArray, iEntryIndex);
    theValueVarData.type = 1;
    theValueVarData.u.intValue = valueData;
    SetVariableValue(inst, id, &theValueVarData);
    return iEntryIndex;
}

const char *VALUE_DATA_FIELD =
"valueData";
const char *VALUE_DATA_TYPE =
"valueType";
const char *VALUE_DATA_NAME =
"valueName";

unsigned int __cdecl ParseValueFromValueList(ScriptTokenizer *tokenizer, scriptInstance_t inst, unsigned int valueName)
{
    char *v3; // eax
    unsigned int String; // eax
    unsigned int theValue; // [esp+10h] [ebp-18h]
    unsigned int valueNameVar; // [esp+14h] [ebp-14h]
    VariableValue theVar; // [esp+18h] [ebp-10h] BYREF
    float fValue; // [esp+20h] [ebp-8h]

    if ( DetermineValueType(tokenizer) )
        AnimTreeCompileError(inst, "Values can currently only be numbers.");
    //v3 = tokenizer->PopToken();
    v3 = tokenizer->PopToken();
    fValue = atof(v3);
    theValue = Scr_AllocArray(inst);
    SetFloatVariable(inst, theValue, fValue, (char *)VALUE_DATA_FIELD);
    SetIntVariable(inst, theValue, 0, (char *)VALUE_DATA_TYPE);
    String = SL_GetString_(inst, (char *)VALUE_DATA_NAME, 2u, 4);
    valueNameVar = GetVariable(inst, theValue, String);
    theVar.type = 6;
    theVar.u.intValue = valueName;
    SetNewVariableValue(inst, valueNameVar, &theVar);
    return theValue;
}

char *__thiscall ScriptTokenizer::PopToken()
{
    if ( !this->m_iNumInStack )
        ScriptTokenizer::PushNextToken();
    if ( this->m_iNumInStack )
        return ScriptTokenizer::GetAndRemoveToken();
    else
        return 0;
}

void __thiscall ScriptTokenizer::PushNextToken()
{
    parseInfo_t *strString; // [esp+18h] [ebp-4h]

    if ( this->m_iNumInStack >= 40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
                    209,
                    0,
                    "%s",
                    "m_iNumInStack < MAX_TOKENIZER_STACK") )
    {
        __debugbreak();
    }
    if (gScrAnimGlob[this->inst].pos)
    {
        strString = Com_Parse(&gScrAnimGlob[this->inst].pos);
        ScriptTokenizer::ParseIntoTokens(strString->token);
    }
}

char *__thiscall ScriptTokenizer::GetAndRemoveToken()
{
    if ( this->m_iNumInStack <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
                    219,
                    0,
                    "%s",
                    "m_iNumInStack > 0") )
    {
        __debugbreak();
    }
    return this->m_stack[--this->m_iNumInStack];
}

void __thiscall ScriptTokenizer::ParseIntoTokens(char *strString)
{
    char cPrev; // [esp+Ah] [ebp-12h]
    char cCurr; // [esp+Bh] [ebp-11h]
    ScriptTokenizer::TokenType eCurrType; // [esp+Ch] [ebp-10h]
    const char *strPos; // [esp+10h] [ebp-Ch]
    unsigned int iLen; // [esp+14h] [ebp-8h]
    bool bFoundOneDecimal; // [esp+1Bh] [ebp-1h]

    eCurrType = TOK_UNKNOWN;
    strPos = strString;
    bFoundOneDecimal = 0;
    while ( 2 )
    {
        if ( *strPos )
        {
            cCurr = *strPos;
            switch ( eCurrType )
            {
                case TOK_IDENTIFIER:
                    if ( isalnum(cCurr) || cCurr == 95 )
                        goto LABEL_2;
                    break;
                case TOK_SYMBOL:
                    if ( isdigit(*strPos) )
                    {
                        cPrev = *(strPos - 1);
                        if ( cPrev == 45 || cPrev == 43 || cPrev == 46 )
                        {
                            eCurrType = TOK_NUMBER;
                            if ( cPrev == 46 )
                                bFoundOneDecimal = 1;
                            goto LABEL_2;
                        }
                    }
                    break;
                case TOK_NUMBER:
                    if ( isdigit(cCurr) || !bFoundOneDecimal && cCurr == 46 )
                    {
                        if ( cCurr == 46 )
                            bFoundOneDecimal = 1;
                        goto LABEL_2;
                    }
                    break;
                case TOK_UNKNOWN:
                    if ( isalpha(cCurr) || cCurr == 95 )
                    {
                        eCurrType = TOK_IDENTIFIER;
                    }
                    else if ( isdigit(cCurr) )
                    {
                        eCurrType = TOK_NUMBER;
                    }
                    else
                    {
                        eCurrType = TOK_SYMBOL;
                    }
                    goto LABEL_2;
                default:
LABEL_2:
                    ++strPos;
                    continue;
            }
        }
        break;
    }
    if ( *strPos )
        ScriptTokenizer::ParseIntoTokens((char*)strPos);
    iLen = strPos - strString;
    if ( strPos != strString )
    {
        strncpy(this->m_stack[this->m_iNumInStack], strString, iLen);
        this->m_stack[this->m_iNumInStack][iLen] = 0;
        if ( this->m_stack[this->m_iNumInStack][0] )
            ++this->m_iNumInStack;
    }
}

void __cdecl SetFloatVariable(scriptInstance_t inst, unsigned int theArray, float floatValue, char *strVarName)
{
    unsigned int String; // eax
    VariableValue theExpr; // [esp+0h] [ebp-Ch] BYREF
    unsigned int theVar; // [esp+8h] [ebp-4h]

    String = SL_GetString_(inst, strVarName, 2u, 4);
    theVar = GetVariable(inst, theArray, String);
    theExpr.type = 5;
    theExpr.u.floatValue = floatValue;
    SetNewVariableValue(inst, theVar, &theExpr);
}

int __cdecl DetermineValueType(ScriptTokenizer *tokenizer)
{
    signed int v2; // [esp+0h] [ebp-30h]
    char IsIdentifier; // [esp+13h] [ebp-1Dh]
    const char *token; // [esp+18h] [ebp-18h]
    bool bFoundDecimal; // [esp+23h] [ebp-Dh]
    int iChar; // [esp+28h] [ebp-8h]
    char *strToken; // [esp+2Ch] [ebp-4h]

    strToken = tokenizer->PeekToken();
    v2 = strlen(strToken);
    bFoundDecimal = 0;
    for ( iChar = 0; iChar < v2; ++iChar )
    {
        if ( (iChar || *strToken != 45) && !I_isdigit(strToken[iChar]) )
        {
            if ( bFoundDecimal || strToken[iChar] != 46 )
                break;
            bFoundDecimal = 1;
        }
    }
    if ( iChar == v2 )
        return 0;
    token = tokenizer->PeekToken();
    if ( token )
        IsIdentifier = Scr_IsIdentifier(token);
    else
        IsIdentifier = 0;
    if ( IsIdentifier )
        return 1;
    else
        return 2;
}

char *__thiscall ScriptTokenizer::PeekToken()
{
    if ( !this->m_iNumInStack )
        ScriptTokenizer::PushNextToken();
    if ( this->m_iNumInStack )
        return this->m_stack[this->m_iNumInStack - 1];
    else
        return 0;
}

int __cdecl Scr_GetAnimsIndex(const XAnim_s *anims, scriptInstance_t inst)
{
    int i; // [esp+4h] [ebp-4h]

    for (i = gScrAnimPub[inst].xanim_num[1]; i && gScrAnimPub[inst].xanim_lookup[1][i].anims != anims; --i)
        ;
    return i;
}

XAnim_s *__cdecl Scr_GetClientAnims(unsigned int index, scriptInstance_t inst)
{
    if ((!index || index > gScrAnimPub[inst].xanim_num[0])
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
            1979,
            0,
            "index not in [1, gScrAnimPub[inst].xanim_num[SCR_XANIM_CLIENT]]\n\t%i not in [%i, %i]",
            index,
            1,
            gScrAnimPub[inst].xanim_num[0]))
    {
        __debugbreak();
    }
    return gScrAnimPub[inst].xanim_lookup[0][index].anims;
}

XAnim_s *__cdecl Scr_GetAnims(unsigned int index, scriptInstance_t inst)
{
    if ((!index || index > gScrAnimPub[inst].xanim_num[1])
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
            1987,
            0,
            "index not in [1, gScrAnimPub[inst].xanim_num[SCR_XANIM_SERVER]]\n\t%i not in [%i, %i]",
            index,
            1,
            gScrAnimPub[inst].xanim_num[1]))
    {
        __debugbreak();
    }
    return gScrAnimPub[inst].xanim_lookup[1][index].anims;
}

void __cdecl Scr_UsingTree(scriptInstance_t inst, const char *filename, unsigned int sourcePos)
{
    if (com_sv_running->current.enabled)
    {
        if (inst == SCRIPTINSTANCE_CLIENT)
        {
            Scr_ClientUsingTree(SCRIPTINSTANCE_CLIENT, filename);
        }
        else if (Scr_IsIdentifier(filename))
        {
            gScrAnimPub[inst].animTreeNames = Scr_UsingTreeInternal(inst, filename, &gScrAnimPub[inst].animTreeIndex, 1);
        }
        else
        {
            CompileError(inst, sourcePos, "bad anim tree name");
        }
    }
    else
    {
        Scr_ClientUsingTree(inst, filename);
    }
}

unsigned int __cdecl Scr_UsingTreeInternal(scriptInstance_t inst, const char *filename, unsigned int *index, int user)
{
    unsigned int Variable; // eax
    unsigned int name; // [esp+0h] [ebp-18h]
    unsigned int names; // [esp+8h] [ebp-10h]
    unsigned int i; // [esp+Ch] [ebp-Ch]
    unsigned int fileId; // [esp+10h] [ebp-8h]
    unsigned int id; // [esp+14h] [ebp-4h]
    unsigned int ida; // [esp+14h] [ebp-4h]

    if (!gScrAnimPub[inst].animtree_loading
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
            2335,
            0,
            "%s",
            "gScrAnimPub[inst].animtree_loading"))
    {
        __debugbreak();
    }
    if (!Scr_IsIdentifier(filename)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
            2336,
            0,
            "%s",
            "Scr_IsIdentifier( filename )"))
    {
        __debugbreak();
    }
    name = Scr_CreateCanonicalFilename(inst, filename);
    id = FindVariable(inst, gScrAnimPub[inst].animtrees, name);
    if (id)
    {
        fileId = FindObject(inst, id);
        *index = 0;
        for (i = 1; i <= gScrAnimPub[inst].xanim_num[user]; ++i)
        {
            if (gScrAnimGlob[inst].using_xanim_lookup[user][i] == id)
            {
                *index = i;
                break;
            }
        }
        if (!*index
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
                2367,
                0,
                "%s",
                "*index"))
        {
            __debugbreak();
        }
    }
    else
    {
        ida = GetNewVariable(inst, gScrAnimPub[inst].animtrees, name);
        fileId = GetObject(inst, ida);
        if (++gScrAnimPub[inst].xanim_num[user] >= 0x80
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
                2347,
                0,
                "%s",
                "gScrAnimPub[inst].xanim_num[user] < MAX_XANIMTREE_NUM"))
        {
            __debugbreak();
        }
        gScrAnimGlob[inst].using_xanim_lookup[user][gScrAnimPub[inst].xanim_num[user]] = ida;
        *index = gScrAnimPub[inst].xanim_num[user];
    }
    if (FindVariable(inst, fileId, 1u)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
            2370,
            0,
            "%s",
            "!FindVariable( inst, fileId, ANIMTREE_XANIM )"))
    {
        __debugbreak();
    }
    Variable = GetVariable(inst, fileId, 0);
    names = GetArray(inst, Variable);
    SL_RemoveRefToString(inst, name);
    return names;
}

void __cdecl Scr_ClientUsingTree(scriptInstance_t inst, const char *filename)
{
    if (Scr_IsIdentifier(filename))
        gScrAnimPub[inst].animTreeNames = Scr_UsingTreeInternal(inst, filename, &gScrAnimPub[inst].animTreeIndex, 0);
}

void __cdecl Scr_LoadAnimTreeAtIndex(
    scriptInstance_t inst,
    unsigned int index,
    void *(__cdecl *Alloc)(int),
    int user,
    bool modCheckSum)
{
    ClientTreeStorage *v6; // edx
    char *v7; // eax
    unsigned int Variable; // eax
    unsigned int v13; // eax
    unsigned int animtree_node; // [esp-8h] [ebp-98h]
    VariableValue insertValue; // [esp+4Ch] [ebp-44h] BYREF
    int iValueCount; // [esp+54h] [ebp-3Ch]
    unsigned int nodeVariable; // [esp+58h] [ebp-38h]
    scr_animtree_t animtree; // [esp+5Ch] [ebp-34h] BYREF
    unsigned int rootName; // [esp+60h] [ebp-30h]
    int size2; // [esp+64h] [ebp-2Ch]
    int size; // [esp+68h] [ebp-28h]
    unsigned int name; // [esp+6Ch] [ebp-24h]
    unsigned int filenameId; // [esp+70h] [ebp-20h]
    unsigned int names; // [esp+74h] [ebp-1Ch]
    unsigned int nodeData; // [esp+78h] [ebp-18h]
    const char *strFilename; // [esp+7Ch] [ebp-14h]
    unsigned int fileId; // [esp+80h] [ebp-10h]
    VariableValue tempValue; // [esp+84h] [ebp-Ch] BYREF
    unsigned int id; // [esp+8Ch] [ebp-4h]

    id = gScrAnimGlob[inst].using_xanim_lookup[user][index];

    iassert(gScrAnimPub[inst].animtree_loading);

    Hunk_CheckTempMemoryClear();
    Hunk_CheckTempMemoryHighClear();

    iassert(GetVariableName(inst, id) < SL_MAX_STRING_INDEX);

    filenameId = GetVariableName(inst, id);
    fileId = FindObject(inst, id);
    iassert(fileId);

    if (!FindVariable(inst, fileId, 1u))
    {
        animtree.anims = 0;
        names = FindVariable(inst, fileId, 0);
        if (names)
        {
            names = FindObject(inst, names);
            iassert(names);
            if (inst == SCRIPTINSTANCE_CLIENT)
            {
                v7 = SL_ConvertToString(filenameId, SCRIPTINSTANCE_CLIENT);
                gScrAnimPub[1].xanim_lookup[user][index] = CScr_RetrieveAnimTree(v7, names, filenameId, index);
                insertValue.type = VAR_CODEPOS;
                insertValue.u.intValue = (int)gScrAnimPub[1].xanim_lookup[user][index].anims;
                Variable = GetVariable(SCRIPTINSTANCE_CLIENT, fileId, 1u);
                SetVariableValue(SCRIPTINSTANCE_CLIENT, Variable, &insertValue);
            }
            else
            {
                g_pCurrClientData = &gGScrXAnimTreesForClient[user][index];

                iassert(!gScrAnimPub[inst].animtree_node);
                gScrAnimPub[inst].animtree_node = Scr_AllocArray(inst);
                animtree_node = gScrAnimPub[inst].animtree_node;
                if (!Scr_LoadAnimTreeInternal(inst, SL_ConvertToString(filenameId, inst), animtree_node, names))
                {
                    Com_Error(ERR_DROP, va("unknown anim tree '%s'", SL_ConvertToString(filenameId, inst)));
                }
                Hunk_CheckTempMemoryClear();
                Hunk_CheckTempMemoryHighClear();
                rootName = SL_GetString_(inst, (char *)NODE_ROOT_NAME, 2u, 4);
                nodeVariable = FindVariable(inst, gScrAnimPub[inst].animtree_node, rootName);
                nodeData = GetArray(inst, nodeVariable);
                size = Scr_GetAnimTreeSize(inst, nodeData);
                iassert(size);

                iValueCount = Scr_GetAnimTreeValueCount(inst, gScrAnimPub[inst].animtree_node);
                g_pCurrClientData->pTreeNameMap = (TreeNameMap *)Hunk_Alloc(84 * size, "Client AnimScript", 16);
                memset(g_pCurrClientData->pTreeNameMap, 0, 84 * size);
                g_pCurrClientData->numIndices = size;
                animtree.anims = XAnimCreateAnimsWithValues(SL_ConvertToString(filenameId, inst), size, iValueCount, Alloc);
                name = SL_GetString_(inst, "root", 0, 4);
                ConnectScriptToAnim(inst, names, 0, filenameId, name, index);
                SL_RemoveRefToString(inst, name);
                if (!useFastFile->current.enabled)
                    Scr_PrecacheAnimationTree(inst, nodeData);
                g_bChangeChecksum = sv.state != SS_GAME && modCheckSum;
                size2 = Scr_CreateAllAnimTreeData(
                    inst,
                    gScrAnimPub[inst].animtree_node,
                    names,
                    &animtree,
                    filenameId,
                    index,
                    Alloc);
                //*(_BYTE *)(*((_DWORD *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 8232) = 1;
                g_bChangeChecksum = 1;
                if (size != size2
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
                        2683,
                        0,
                        "%s",
                        "size == size2"))
                {
                    __debugbreak();
                }
                Scr_CheckAnimsDefined(inst, names, filenameId);
                RemoveVariable(inst, fileId, 0);
                RemoveRefToObject(inst, gScrAnimPub[inst].animtree_node);
                gScrAnimPub[inst].animtree_node = 0;
                tempValue.type = 7;
                tempValue.u.intValue = (int)animtree.anims;
                v13 = GetVariable(inst, fileId, 1u);
                SetVariableValue(inst, v13, &tempValue);
                XAnimSetupSyncNodes(animtree.anims);
                gScrAnimPub[inst].xanim_lookup[user][index] = animtree;
                gGScrXAnimTreesForClient[user][index].animTree = animtree;
                strFilename = SL_ConvertToString(filenameId, inst);
                gGScrXAnimTreesForClient[user][index].strName = (char *)Hunk_Alloc(
                    strlen(strFilename) + 1,
                    "Client AnimScript",
                    16);
                I_strncpyz(gGScrXAnimTreesForClient[user][index].strName, strFilename, strlen(strFilename) + 1);
            }
        }
        else
        {
            gScrAnimPub[inst].xanim_lookup[user][index] = animtree;
            if (inst == SCRIPTINSTANCE_SERVER)
            {
                v6 = &gGScrXAnimTreesForClient[user][index];
                v6->strName = 0;
                v6->animTree.anims = 0;
                v6->numIndices = 0;
                v6->pTreeNameMap = 0;
            }
        }
    }
}

unsigned int __cdecl GetVariableName(scriptInstance_t inst, unsigned int id)
{
    VariableValueInternal *v3; // [esp+0h] [ebp-4h]

    v3 = &gScrVarGlob[inst].variableList[id + 0x8000];
    if ( (v3->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (v3->w.status & 0x1F) >= 0xD
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    685,
                    0,
                    "%s",
                    "!IsObject( &gScrVarGlob[inst].variableList[VARIABLELIST_CHILD_BEGIN(inst) + id] )") )
    {
        __debugbreak();
    }
    return gScrVarGlob[inst].variableList[id + 0x8000].w.status >> 8;
}

int __cdecl Scr_GetAnimTreeSize(scriptInstance_t inst, unsigned int parentNode)
{
    unsigned int ValueType; // eax
    unsigned int Object; // eax
    unsigned int flagsId; // [esp+1Ch] [ebp-18h]
    unsigned int arrayNode; // [esp+20h] [ebp-14h]
    bool bHasChildren; // [esp+27h] [ebp-Dh]
    unsigned int node; // [esp+28h] [ebp-Ch]
    int size; // [esp+30h] [ebp-4h]

    size = 0;
    for ( node = FindFirstSibling(inst, parentNode); node; node = FindNextSibling(inst, node) )
    {
        if ( GetVariableName(inst, node) < 0x10000 )
        {
            ValueType = GetValueType(inst, node);
            bHasChildren = ValueType == 1;
            if ( ValueType == 1 )
            {
                arrayNode = FindObject(inst, node);
                flagsId = FindArrayVariable(inst, arrayNode, 1u);
                bHasChildren = flagsId && GetVariableValueAddress(inst, flagsId)->next;
            }
            if ( bHasChildren )
            {
                Object = FindObject(inst, node);
                size += Scr_GetAnimTreeSize(inst, Object);
            }
            else
            {
                ++size;
            }
        }
    }
    if ( size )
        ++size;
    return size;
}

unsigned int __cdecl FindFirstSibling(scriptInstance_t inst, unsigned int id)
{
    VariableValueInternal *entryValue; // [esp+0h] [ebp-4h]

    if ( !id && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h", 632, 0, "%s", "id") )
        __debugbreak();
    entryValue = &gScrVarGlob[inst].variableList[id + 1];
    if ( (entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) < 0xD
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    634,
                    0,
                    "%s",
                    "IsObject( entryValue )") )
    {
        __debugbreak();
    }
    return entryValue->nextSibling;
}

unsigned int __cdecl FindNextSibling(scriptInstance_t inst, unsigned int id)
{
    unsigned int nextSibling; // [esp+4h] [ebp-10h]
    VariableValueInternal *list; // [esp+8h] [ebp-Ch]
    unsigned int childId; // [esp+Ch] [ebp-8h]
    VariableValueInternal *entryValue; // [esp+10h] [ebp-4h]

    list = gScrVarGlob[inst].variableList + 0x8000;
    if ( !id && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h", 645, 0, "%s", "id") )
        __debugbreak();
    entryValue = &list[id];
    if ( (entryValue->w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (entryValue->w.status & 0x1F) >= 0xD
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    647,
                    0,
                    "%s",
                    "!IsObject( entryValue )") )
    {
        __debugbreak();
    }
    nextSibling = entryValue->nextSibling;
    if ( !nextSibling )
        return 0;
    childId = list[nextSibling].hash.id;
    if ( (list[childId].w.status & 0x60) == 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    323,
                    0,
                    "%s",
                    "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE") )
    {
        __debugbreak();
    }
    if ( (list[childId].w.status & 0x1F) >= 0xD
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    653,
                    0,
                    "%s",
                    "!IsObject( &list[childId] )") )
    {
        __debugbreak();
    }
    return childId;
}

unsigned int __cdecl Scr_GetAnimTreeValueCount(scriptInstance_t inst, unsigned int rootData)
{
    unsigned int valueDataArray; // [esp+0h] [ebp-4h]

    valueDataArray = GetNamedArrayVarArray(inst, rootData, (char *)VALUES_ROOT_NAME);
    if ( GetArraySize(inst, valueDataArray) >= 0xFFFF
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
                    1904,
                    0,
                    "%s",
                    "GetArraySize(inst, valueDataArray) < 65535") )
    {
        __debugbreak();
    }
    return GetArraySize(inst, valueDataArray);
}

unsigned int __cdecl GetNamedArrayVarArray(scriptInstance_t inst, unsigned int theArray, char *strVarName)
{
    unsigned int varName; // [esp+0h] [ebp-8h]
    unsigned int theVariable; // [esp+4h] [ebp-4h]

    varName = SL_GetString_(inst, strVarName, 2u, 4);
    theVariable = FindVariable(inst, theArray, varName);
    return GetArray(inst, theVariable);
}

void __cdecl ConnectScriptToAnim(
                scriptInstance_t inst,
                unsigned int names,
                int index,
                unsigned int filename,
                unsigned int name,
                unsigned __int16 treeIndex)
{
    char *v6; // eax
    char *v7; // [esp-4h] [ebp-20h]
    char *strName; // [esp+0h] [ebp-1Ch]
    scr_anim_s anim; // [esp+8h] [ebp-14h]
    char *codePos; // [esp+Ch] [ebp-10h]
    unsigned int animId; // [esp+10h] [ebp-Ch]
    VariableValueInternal_u *value; // [esp+14h] [ebp-8h]
    const char *nextCodePos; // [esp+18h] [ebp-4h]

    animId = FindVariable(inst, names, name);
    if ( animId )
    {
        value = GetVariableValueAddress(inst, animId);
        if ( !value->next )
        {
            v7 = SL_ConvertToString(filename, inst);
            v6 = SL_ConvertToString(name, inst);
            Com_Error(ERR_DROP, "duplicate animation '%s' in 'animtrees/%s.atr'", v6, v7);
        }
        if ( inst == SCRIPTINSTANCE_SERVER )
        {
            strName = SL_ConvertToString(name, SCRIPTINSTANCE_SERVER);
            I_strncpyz(g_pCurrClientData->pTreeNameMap[index].strName, strName, 80);
        }
        anim.index = index;
        anim.tree = treeIndex;
        for ( codePos = (char *)value->next; codePos; codePos = (char *)nextCodePos )
        {
            nextCodePos = *(const char **)codePos;
            *(scr_anim_s *)codePos = anim;
        }
        value->next = 0;
    }
}

void __cdecl Scr_CheckAnimsDefined(scriptInstance_t inst, unsigned int names, unsigned int filename)
{
    char *v3; // eax
    char *v4; // [esp-4h] [ebp-30h]
    unsigned int name; // [esp+1Ch] [ebp-10h]
    char *msg; // [esp+20h] [ebp-Ch]
    unsigned int animId; // [esp+24h] [ebp-8h]
    VariableValueInternal_u *value; // [esp+28h] [ebp-4h]

    for ( animId = FindFirstSibling(inst, names); animId; animId = FindNextSibling(inst, animId) )
    {
        name = GetVariableName(inst, animId);
        if ( name >= 0x10000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
                        2274,
                        0,
                        "%s",
                        "name < SL_MAX_STRING_INDEX") )
        {
            __debugbreak();
        }
        value = GetVariableValueAddress(inst, animId);
        if ( value->next )
        {
            v4 = SL_ConvertToString(filename, inst);
            v3 = SL_ConvertToString(name, inst);
            msg = va("animation '%s' not defined in anim tree '%s'", v3, v4);
            if ( Scr_IsInOpcodeMemory(inst, value->u.codePosValue) )
                CompileError2(inst, (char *)value->next, "%s", msg);
            else
                Com_Error(ERR_DROP, "%s", msg);
        }
    }
}

void __cdecl Scr_PrecacheAnimationTree(scriptInstance_t inst, unsigned int parentNode)
{
    unsigned int ValueType; // eax
    unsigned int Object; // eax
    char *v4; // eax
    unsigned int flagsId; // [esp+1Ch] [ebp-14h]
    unsigned int nodeObj; // [esp+20h] [ebp-10h]
    bool bHasChildren; // [esp+27h] [ebp-9h]
    unsigned int node; // [esp+28h] [ebp-8h]
    unsigned int name; // [esp+2Ch] [ebp-4h]

    for ( node = FindFirstSibling(inst, parentNode); node; node = FindNextSibling(inst, node) )
    {
        name = GetVariableName(inst, node);
        if ( name < 0x10000 )
        {
            ValueType = GetValueType(inst, node);
            bHasChildren = ValueType == 1;
            if ( ValueType == 1 )
            {
                nodeObj = FindObject(inst, node);
                flagsId = FindArrayVariable(inst, nodeObj, 1u);
                bHasChildren = flagsId && GetVariableValueAddress(inst, flagsId)->next;
            }
            if ( bHasChildren )
            {
                Object = FindObject(inst, node);
                Scr_PrecacheAnimationTree(inst, Object);
            }
            else
            {
                v4 = SL_ConvertToString(name, inst);
                XAnimPrecache(v4, (void *(__cdecl *)(int))Hunk_AllocXAnimPrecache);
            }
        }
    }
}

bool __cdecl Scr_LoadAnimTreeInternal(
                scriptInstance_t inst,
                const char *filename,
                unsigned int parentNode,
                unsigned int names)
{
    char extFilename[68]; // [esp+10h] [ebp-58h] BYREF
    bool useArchive; // [esp+5Bh] [ebp-Dh]
    char *sourceBuffer; // [esp+5Ch] [ebp-Ch]
    const char *oldFilename; // [esp+60h] [ebp-8h]
    const char *oldSourceBuf; // [esp+64h] [ebp-4h]

    useArchive = 1;
    if (com_sv_running->current.enabled)
    {
        //BLOPS_NULLSUB();
    }
    if ( strlen(filename) + 15 >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
                    2461,
                    0,
                    "%s",
                    "strlen( filename ) + sizeof( \"animtrees/.atr\" ) < MAX_QPATH") )
    {
        __debugbreak();
    }
    sprintf(extFilename, "animtrees/%s.atr", filename);
    oldSourceBuf = gScrParserPub[inst].sourceBuf;
    sourceBuffer = Scr_AddSourceBuffer(inst, 0, extFilename, 0, useArchive);
    if ( !sourceBuffer )
        return 0;
    oldFilename = gScrParserPub[inst].scriptfilename;
    gScrParserPub[inst].scriptfilename = extFilename;
    Scr_AnimTreeParse(inst, sourceBuffer, parentNode, names);
    gScrParserPub[inst].scriptfilename = oldFilename;
    gScrParserPub[inst].sourceBuf = oldSourceBuf;
    Hunk_ClearTempMemoryHigh();
    return GetArraySize(inst, parentNode) != 0;
}

void __cdecl Scr_AnimTreeParse(scriptInstance_t inst, const char *pos, unsigned int parentNode, unsigned int names)
{
    Com_BeginParseSession("Scr_AnimTreeParse");
    gScrAnimGlob[inst].pos = pos;
    gScrAnimGlob[inst].start = pos;
    if (!AnimTreeParseInternal(inst, parentNode, names, 1, 0, 0))
        AnimTreeCompileError(inst, "bad token");
    Com_EndParseSession();
}

char __cdecl AnimTreeParseInternal(
                scriptInstance_t inst,
                unsigned int parentNode,
                unsigned int names,
                bool bIncludeParent,
                bool bLoop,
                bool bComplete)
{
    unsigned int v6; // eax
    unsigned int stringValue; // [esp+8h] [ebp-2824h]
    unsigned int parentNodea; // [esp+Ch] [ebp-2820h]
    unsigned int unsignedValue; // [esp+10h] [ebp-281Ch]
    unsigned int v11; // [esp+14h] [ebp-2818h]
    unsigned int id; // [esp+18h] [ebp-2814h]
    ScriptTokenizer tokenizer; // [esp+1Ch] [ebp-2810h] BYREF
    unsigned int v14; // [esp+2824h] [ebp-8h]
    unsigned int String; // [esp+2828h] [ebp-4h]

    tokenizer.inst = inst;
    tokenizer.m_iNumInStack = 0;
    unsignedValue = SL_GetString_(inst, (char *)NODE_ROOT_NAME, 2u, 4);
    id = GetVariable(inst, parentNode, unsignedValue);
    parentNodea = GetArray(inst, id);
    String = SL_GetString_(inst, (char *)EXPRESSION_ROOT_NAME, 2u, 4);
    GetVariable(inst, parentNode, String);
    v14 = SL_GetString_(inst, (char *)VALUES_ROOT_NAME, 2u, 4);
    GetVariable(inst, parentNode, v14);
    v11 = SL_GetString_(inst, (char *)PARAMETER_ROOT_NAME, 2u, 4);
    GetVariable(inst, parentNode, v11);

    do
        ParseNode(&tokenizer, inst, parentNodea, parentNode, names, bLoop, bComplete);
    while ( !tokenizer.IsAtEnd() );

    if ( !GetArraySize(inst, parentNodea) )
    {
        if ( bLoop )
            v6 = SL_GetString_(inst, (char*)"void_loop", 0, 4);
        else
            v6 = SL_GetString_(inst, (char*)"void", 0, 4);
        stringValue = v6;
        GetVariable(inst, parentNodea, v6);
        SL_RemoveRefToString(inst, stringValue);
    }
    return 1;
}

bool __thiscall ScriptTokenizer::IsAtEnd()
{
    ScriptTokenizer::PeekToken();
    return !this->m_iNumInStack && gScrAnimGlob[this->inst].pos == 0;
}

char __cdecl ParseNode(
                ScriptTokenizer *tokenizer,
                scriptInstance_t inst,
                unsigned int parentNode,
                unsigned int rootData,
                unsigned int names,
                bool bLoop,
                bool bComplete)
{
    char *v8; // eax
    char *v9; // eax
    char *v10; // eax
    char *v11; // eax
    unsigned int LowercaseString; // eax
    ParseValue v13; // rax
    unsigned int ArraySize; // eax
    char *v15; // eax
    const char *v16; // eax
    bool v17; // al
    bool v18; // al
    unsigned int v19; // eax
    unsigned int v20; // eax
    unsigned int ArrayVariable; // eax
    unsigned int v22; // eax
    unsigned int v23; // eax
    unsigned int v24; // eax
    unsigned int String; // eax
    unsigned int v26; // eax
    char *v27; // eax
    const char *v28; // eax
    ParseValue v29; // [esp-8h] [ebp-2DCh]
    bool v30; // [esp+8h] [ebp-2CCh]
    char *v32; // [esp+18h] [ebp-2BCh]
    char *v34; // [esp+24h] [ebp-2B0h]
    char *v36; // [esp+30h] [ebp-2A4h]
    char *v38; // [esp+3Ch] [ebp-298h]
    char *v40; // [esp+48h] [ebp-28Ch]
    char *v42; // [esp+54h] [ebp-280h]
    char *v44; // [esp+60h] [ebp-274h]
    char v45; // [esp+6Bh] [ebp-269h]
    const char *v46; // [esp+6Ch] [ebp-268h]
    char *v48; // [esp+74h] [ebp-260h]
    char *v50; // [esp+80h] [ebp-254h]
    char *v52; // [esp+8Ch] [ebp-248h]
    char *v54; // [esp+98h] [ebp-23Ch]
    char *v56; // [esp+A4h] [ebp-230h]
    char v57; // [esp+AFh] [ebp-225h]
    const char *v58; // [esp+B0h] [ebp-224h]
    char *v60; // [esp+B8h] [ebp-21Ch]
    char *v62; // [esp+C4h] [ebp-210h]
    char IsIdentifier; // [esp+CFh] [ebp-205h]
    const char *token; // [esp+D0h] [ebp-204h]
    ParseValue v65; // [esp+D8h] [ebp-1FCh]
    int j; // [esp+E0h] [ebp-1F4h]
    int i; // [esp+E4h] [ebp-1F0h]
    unsigned int v68; // [esp+E8h] [ebp-1ECh]
    int iValue; // [esp+ECh] [ebp-1E8h]
    unsigned int valArray; // [esp+F0h] [ebp-1E4h]
    unsigned int valueDataVar; // [esp+F4h] [ebp-1E0h]
    unsigned int iEntryIndex; // [esp+F8h] [ebp-1DCh]
    int iOpt; // [esp+FCh] [ebp-1D8h]
    unsigned int exprArray; // [esp+100h] [ebp-1D4h]
    unsigned int exprDataVar; // [esp+104h] [ebp-1D0h]
    int iCurrIx; // [esp+108h] [ebp-1CCh]
    VariableValue value; // [esp+10Ch] [ebp-1C8h] BYREF
    VariableValue tempValue; // [esp+114h] [ebp-1C0h] BYREF
    unsigned int iInitialSize; // [esp+11Ch] [ebp-1B8h]
    int iValueCount; // [esp+120h] [ebp-1B4h]
    ValueEntry valueData[20]; // [esp+124h] [ebp-1B0h]
    ParseEntry optionData[20]; // [esp+1C4h] [ebp-110h]
    int iNumComponents; // [esp+2B8h] [ebp-1Ch]
    int iNumOptions; // [esp+2BCh] [ebp-18h]
    unsigned int currentAnim; // [esp+2C0h] [ebp-14h]
    unsigned int animName; // [esp+2C4h] [ebp-10h]
    unsigned int currentAnimArray; // [esp+2C8h] [ebp-Ch]
    int flags; // [esp+2CCh] [ebp-8h]
    bool bIgnore; // [esp+2D3h] [ebp-1h]

    currentAnim = 0;
    animName = 0;
    flags = 0;
    bIgnore = 0;
    if ( tokenizer->IsAtEnd() )
        return 1;
    v8 = tokenizer->PeekToken();
    if ( I_strcmp(v8, "param") )
    {
        token = tokenizer->PeekToken();
        if ( token )
            IsIdentifier = Scr_IsIdentifier(token);
        else
            IsIdentifier = 0;
        if ( !IsIdentifier )
        {
            v27 = tokenizer->PeekToken();
            v28 = va("bad token: '%s'", v27);
            AnimTreeCompileError(inst, v28);
            return tokenizer->IsAtEnd();
        }
        v9 = tokenizer->PopToken();
        animName = SL_GetLowercaseString_(v9, 2u, 4, inst);
        if ( FindVariable(inst, parentNode, animName) )
            AnimTreeCompileError(inst, "duplicate animation");
        currentAnim = GetVariable(inst, parentNode, animName);
        v30 = !bComplete && !FindVariable(inst, names, animName) && !gScrAnimGlob[inst].bAnimCheck;
        bIgnore = v30;
        flags = 0;
        iNumOptions = 0;
        iValueCount = 0;
        v62 = tokenizer->PeekToken();
        if ( v62 && *v62 == 91 && !v62[1] )
        {
            tokenizer->PopToken();
            while ( 1 )
            {
                v60 = tokenizer->PeekToken();
                if ( v60 && *v60 == 93 && !v60[1] )
                    break;
                v58 = tokenizer->PeekToken();
                if ( v58 )
                    v57 = Scr_IsIdentifier(v58);
                else
                    v57 = 0;
                if ( !v57 )
                    AnimTreeCompileError(inst, "Operator option not specified.");
                if ( iValueCount >= 20
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
                                1467,
                                0,
                                "%s",
                                "iValueCount < MAX_NUM_OPTIONS") )
                {
                    __debugbreak();
                }
                v10 = tokenizer->PopToken();
                valueData[iValueCount].valueName = SL_GetLowercaseString_(v10, 2u, 4, inst);
                v56 = tokenizer->PeekToken();
                if ( !v56 || *v56 != 61 || v56[1] )
                    AnimTreeCompileError(inst, "Operator options must have an '=' sign after the name.");
                tokenizer->PopToken();
                valueData[iValueCount].theValue = ParseValueFromValueList(tokenizer, inst, valueData[iValueCount].valueName);
                ++iValueCount;
                v54 = tokenizer->PeekToken();
                if ( v54 && *v54 == 44 && !v54[1] )
                {
                    tokenizer->PopToken();
                }
                else
                {
                    v52 = tokenizer->PeekToken();
                    if ( !v52 || *v52 != 93 || v52[1] )
                        AnimTreeCompileError(inst, "The list of values is not properly comma delimited.");
                }
            }
            tokenizer->PopToken();
        }
        v50 = tokenizer->PeekToken();
        if ( v50 && *v50 == 60 && !v50[1] )
        {
            if ( iValueCount )
                AnimTreeCompileError(inst, "An element in the tree cannot have both operators and values.");
LABEL_68:
            tokenizer->PopToken();
            while ( 1 )
            {
                v48 = tokenizer->PeekToken();
                if ( v48 && *v48 == 62 && !v48[1] )
                    break;
                v46 = tokenizer->PeekToken();
                if ( v46 )
                    v45 = Scr_IsIdentifier(v46);
                else
                    v45 = 0;
                if ( !v45 )
                    AnimTreeCompileError(inst, "Operator option not specified.");
                if ( iNumOptions >= 20
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
                                1509,
                                0,
                                "%s",
                                "iNumOptions < MAX_NUM_OPTIONS") )
                {
                    __debugbreak();
                }
                v11 = tokenizer->PopToken();
                LowercaseString = SL_GetLowercaseString_(v11, 2u, 4, inst);
                optionData[iNumOptions].optionName = LowercaseString;
                v44 = tokenizer->PeekToken();
                if ( !v44 || *v44 != 61 || v44[1] )
                    AnimTreeCompileError(inst, "Operator options must have an '=' sign after the name.");
                tokenizer->PopToken();
                v13 = ParseTopRankExpr(tokenizer, inst, rootData);
                v65 = v13;
                v13.exprType = (XExpr::MathTypes)(12 * iNumOptions);
                *(unsigned int *)((char *)&optionData[0].theValue.scriptExprData + v13.exprType) = v13.scriptExprData;
                *(XExpr::MathTypes *)((char *)&optionData[0].theValue.exprType + v13.exprType) = v65.exprType;
                if ( !optionData[iNumOptions].theValue.scriptExprData )
                    AnimTreeCompileError(inst, "No expression provided for the option.");
                ++iNumOptions;
                v42 = tokenizer->PeekToken();
                if ( v42 && *v42 == 44 && !v42[1] )
                    goto LABEL_68;
                v40 = tokenizer->PeekToken();
                if ( !v40 || *v40 != 62 || v40[1] )
                    AnimTreeCompileError(inst, "The list of options is not properly comma delimited.");
            }
            tokenizer->PopToken();
        }
        iNumComponents = 0;
        v38 = tokenizer->PeekToken();
        if ( v38 && *v38 == 58 && !v38[1] )
        {
            tokenizer->PopToken();
            flags = GetAnimTreeParseProperties(tokenizer);
        }
        if ( iNumComponents )
            ++iFoo_1;
        v36 = tokenizer->PeekToken();
        if ( v36 && *v36 == 123 && !v36[1] )
        {
            tokenizer->PopToken();
            currentAnimArray = GetArray(inst, currentAnim);
            ArraySize = GetArraySize(inst, currentAnimArray);
            iInitialSize = ArraySize - iNumComponents;
            while ( 1 )
            {
                v34 = tokenizer->PeekToken();
                if ( v34 && *v34 == 125 && !v34[1] )
                    break;
                if ( tokenizer->IsAtEnd() )
                {
                    v15 = tokenizer->PeekToken();
                    v16 = va("bad token: '%s'", v15);
                    AnimTreeCompileError(inst, v16);
                }
                if ( (flags & 0x80) != 0 )
                {
                    if ( bComplete || (flags & 8) != 0 )
                        v17 = ParseNode(tokenizer, inst, currentAnimArray, rootData, names, (flags & 1) != 0, 1);
                    else
                        v17 = ParseNode(tokenizer, inst, currentAnimArray, rootData, names, (flags & 1) != 0, 0);
                    if ( v17 )
                        AnimTreeCompileError(inst, "unexpected end of file");
                }
                else
                {
                    if ( bComplete || (flags & 8) != 0 && !bIgnore )
                        v18 = ParseNode(tokenizer, inst, currentAnimArray, rootData, names, (flags & 1) != 0, 1);
                    else
                        v18 = ParseNode(tokenizer, inst, currentAnimArray, rootData, names, (flags & 1) != 0, 0);
                    if ( v18 )
                        AnimTreeCompileError(inst, "unexpected end of file");
                }
                v19 = GetArraySize(inst, currentAnimArray);
                if ( iInitialSize != v19 - iNumComponents )
                    bIgnore = 0;
            }
            v32 = tokenizer->PeekToken();
            if ( v32 && *v32 == 125 && !v32[1] )
                tokenizer->PopToken();
            v20 = GetArraySize(inst, currentAnimArray);
            if ( iInitialSize != v20 - iNumComponents )
            {
                tempValue.type = 6;
                tempValue.u.intValue = flags;
                ArrayVariable = GetArrayVariable(inst, currentAnimArray, 0);
                SetVariableValue(inst, ArrayVariable, &tempValue);
                tempValue.type = 6;
                tempValue.u.intValue = 1;
                v22 = GetArrayVariable(inst, currentAnimArray, 1u);
                SetVariableValue(inst, v22, &tempValue);
LABEL_166:
                if ( !bIgnore )
                {
                    iCurrIx = 4;
                    if ( iNumOptions )
                    {
                        String = SL_GetString_(inst, (char *)EXPRESSION_ROOT_NAME, 2u, 4);
                        exprDataVar = FindVariable(inst, rootData, String);
                        if ( !exprDataVar )
                            AnimTreeCompileError(inst, "Expression variable not found.");
                        exprArray = GetArray(inst, exprDataVar);
                        for ( iOpt = 0; iOpt < iNumOptions; ++iOpt )
                        {
                            v29.exprType = optionData[iOpt].theValue.exprType;
                            v29.scriptExprData = optionData[iOpt].theValue.scriptExprData;
                            iEntryIndex = StoreExprInList(inst, exprArray, v29);
                            currentAnimArray = GetArray(inst, currentAnim);
                            StoreOptionsInNode(inst, currentAnimArray, iCurrIx++, optionData[iOpt].optionName, iEntryIndex);
                        }
                    }
                    if ( iValueCount )
                    {
                        v26 = SL_GetString_(inst, (char *)VALUES_ROOT_NAME, 2u, 4);
                        valueDataVar = FindVariable(inst, rootData, v26);
                        if ( !valueDataVar )
                            AnimTreeCompileError(inst, "Value variable not found.");
                        valArray = GetArray(inst, valueDataVar);
                        for ( iValue = 0; iValue < iValueCount; ++iValue )
                        {
                            v68 = StoreValueInList(inst, valArray, valueData[iValue].theValue);
                            currentAnimArray = GetArray(inst, currentAnim);
                            StoreValuesInNode(inst, currentAnimArray, iCurrIx++, v68);
                        }
                    }
                }
                if ( bIgnore )
                {
                    if ( iNumOptions )
                    {
                        for ( i = 0; i < iNumOptions; ++i )
                            RemoveRefToObject(inst, optionData[i].theValue.scriptExprData);
                    }
                    if ( iValueCount )
                    {
                        for ( j = 0; j < iValueCount; ++j )
                            RemoveRefToObject(inst, valueData[j].theValue);
                    }
                }
                return tokenizer->IsAtEnd();
            }
            if ( !bIgnore )
            {
                if ( bLoop )
                    v23 = SL_GetString_(inst, "void_loop", 0, 4);
                else
                    v23 = SL_GetString_(inst, "void", 0, 4);
                animName = v23;
                GetVariable(inst, currentAnimArray, v23);
                SL_RemoveRefToString(inst, animName);
                goto LABEL_166;
            }
        }
        else if ( !bIgnore )
        {
            currentAnimArray = GetArray(inst, currentAnim);
            value.type = 6;
            value.u.intValue = 0;
            v24 = GetArrayVariable(inst, currentAnimArray, 1u);
            SetVariableValue(inst, v24, &value);
            goto LABEL_166;
        }
        RemoveVariable(inst, parentNode, animName);
        goto LABEL_166;
    }
    tokenizer->PopToken();
    ParseParameterDef(tokenizer, inst, rootData);
    return tokenizer->IsAtEnd();
}

ParseValue __cdecl ParseTopRankExpr(ScriptTokenizer *tokenizer, scriptInstance_t inst, unsigned int rootData)
{
    ParseValue params[2]; // [esp+20h] [ebp-1Ch] BYREF
    ExpressionParseTypes eParseType; // [esp+30h] [ebp-Ch]
    ParseValue firstParam; // [esp+34h] [ebp-8h]

    firstParam = ParseRank2Expr(tokenizer, inst, rootData);
    for ( eParseType = DetermineParseType(tokenizer);
                (unsigned int)eParseType <= ET_MINUS;
                eParseType = DetermineParseType(tokenizer) )
    {
        tokenizer->PopToken();
        params[0] = firstParam;
        params[1] = ParseRank2Expr(tokenizer, inst, rootData);
        firstParam = BuildExpr(eParseType, params, 2, inst);
    }
    return firstParam;
}

ExpressionParseTypes __cdecl DetermineParseType(ScriptTokenizer *tokenizer)
{
    char *v2; // eax
    signed int v3; // [esp+0h] [ebp-44h]
    char IsIdentifier; // [esp+17h] [ebp-2Dh]
    const char *token; // [esp+1Ch] [ebp-28h]
    char *v7; // [esp+2Ch] [ebp-18h]
    unsigned int iExpr; // [esp+30h] [ebp-14h]
    bool bFoundDecimal; // [esp+37h] [ebp-Dh]
    int iChar; // [esp+3Ch] [ebp-8h]
    char *strToken; // [esp+40h] [ebp-4h]

    v7 = tokenizer->PeekToken();
    if ( v7 && *v7 == 44 && !v7[1] )
        return ETS_COMMA;
    strToken = tokenizer->PeekToken();
    for ( iExpr = 0; iExpr < 0xA; ++iExpr )
    {
        if ( !I_strcmp(g_ExprOperatorList[iExpr].strValue, strToken) )
            return (ExpressionParseTypes)iExpr;
    }
    v3 = strlen(strToken);
    bFoundDecimal = 0;
    for ( iChar = 0; iChar < v3; ++iChar )
    {
        if ( (iChar || *strToken != 45) && !I_isdigit(strToken[iChar]) )
        {
            if ( bFoundDecimal || strToken[iChar] != 46 )
                break;
            bFoundDecimal = 1;
        }
    }
    if ( iChar == v3 )
        return ET_SINGLE_VALUE;
    token = tokenizer->PeekToken();
    if ( token )
        IsIdentifier = Scr_IsIdentifier(token);
    else
        IsIdentifier = 0;
    if ( !IsIdentifier )
        return NUM_EXPR_TYPES;
    v2 = tokenizer->PeekToken();
    if ( IsStandardParam(v2) )
        return ET_GET_STD_PARAM;
    else
        return ET_GET_PARAMETER;
}

bool __cdecl IsStandardParam(const char *strParamName)
{
    return GetStdParamIx(strParamName) != 1;
}

int __cdecl GetStdParamIx(const char *strParamName)
{
    int iParam; // [esp+0h] [ebp-8h]

    for ( iParam = 0; iParam < 1; ++iParam )
    {
        if ( !I_stricmp(g_stdParamList[iParam].strParamName, strParamName) )
            return iParam;
    }
    return 1;
}

ParseValue __cdecl BuildExpr(ExpressionParseTypes eCurrType, ParseValue *params, int iNumParams, scriptInstance_t inst)
{
    const char *v4; // eax
    const char *v5; // eax
    unsigned int String; // eax
    unsigned int v7; // eax
    unsigned int parameter; // [esp+0h] [ebp-12Ch]
    char strTempBuffer[256]; // [esp+4h] [ebp-128h] BYREF
    VariableValue value; // [esp+108h] [ebp-24h] BYREF
    int iParam; // [esp+110h] [ebp-1Ch]
    VariableValue theVar; // [esp+114h] [ebp-18h] BYREF
    unsigned int cmd; // [esp+11Ch] [ebp-10h]
    ParseValue ret; // [esp+120h] [ebp-Ch]
    const ExpressionOperatorData *currData; // [esp+128h] [ebp-4h]

    currData = &g_ExprOperatorList[eCurrType];
    if ( iNumParams != currData->iArity )
    {
        v4 = va(
                     "Function %i, \"%s\" was given %i parameters, when it takes %i.",
                     eCurrType,
                     currData->strValue,
                     iNumParams,
                     currData->iArity);
        AnimTreeCompileError(inst, v4);
    }
    ret.exprType = currData->FuncTypeTesting(params, iNumParams, inst);
    if ( ret.exprType == MAX_MATH_TYPES )
    {
        v5 = va("Function %i, \"%s\" had a type mismatch between its operands.", eCurrType, currData->strValue);
        AnimTreeCompileError(inst, v5);
    }
    ret.scriptExprData = Scr_AllocArray(inst);
    String = SL_GetString_(inst, (char *)EXPR_OPERATOR_NAME, 2u, 4);
    cmd = GetVariable(inst, ret.scriptExprData, String);
    theVar.type = 6;
    theVar.u.intValue = eCurrType;
    SetNewVariableValue(inst, cmd, &theVar);
    for ( iParam = 0; iParam < iNumParams; ++iParam )
    {
        CreateParameterVarName(strTempBuffer, 0xFFu, iParam);
        v7 = SL_GetString_(inst, strTempBuffer, 2u, 4);
        parameter = GetVariable(inst, ret.scriptExprData, v7);
        value.type = 1;
        value.u.intValue = params[iParam].scriptExprData;
        SetNewVariableValue(inst, parameter, &value);
    }
    return ret;
}

void __cdecl CreateParameterVarName(char *strOutString, unsigned int iMaxNumChars, int iOperandIx)
{
    _snprintf(strOutString, iMaxNumChars, "%s%i", EXPR_OPERAND_NAME, iOperandIx);
}

ParseValue __cdecl ParseRank2Expr(ScriptTokenizer *tokenizer, scriptInstance_t inst, unsigned int rootData)
{
    ParseValue params[2]; // [esp+20h] [ebp-1Ch] BYREF
    ExpressionParseTypes eParseType; // [esp+30h] [ebp-Ch]
    ParseValue firstParam; // [esp+34h] [ebp-8h]

    firstParam = ParseBottomRankExpr(tokenizer, inst, rootData);
    for ( eParseType = DetermineParseType(tokenizer);
                eParseType >= END_OF_RANK1 && eParseType < END_OF_RANK2;
                eParseType = DetermineParseType(tokenizer) )
    {
        tokenizer->PopToken();
        params[0] = firstParam;
        params[1] = ParseBottomRankExpr(tokenizer, inst, rootData);
        firstParam = BuildExpr(eParseType, params, 2, inst);
    }
    return firstParam;
}

ParseValue __cdecl ParseBottomRankExpr(ScriptTokenizer *tokenizer, scriptInstance_t inst, unsigned int rootData)
{
    char *v3; // eax
    const char *v5; // eax
    const char *v6; // eax
    ParseValue v7; // [esp+48h] [ebp-18h]
    ParseValue ret; // [esp+50h] [ebp-10h]
    float fValue; // [esp+58h] [ebp-8h]
    ExpressionParseTypes eParseType; // [esp+5Ch] [ebp-4h]

    eParseType = (ExpressionParseTypes)DetermineParseType(tokenizer);
    if ( eParseType == ET_SINGLE_VALUE )
    {
        v3 = tokenizer->PopToken();
        fValue = atof(v3);
        //ret = (ParseValue)Scr_AllocArray(inst);
        ret.scriptExprData = Scr_AllocArray(inst);
        ret.exprType = NUMBER;
        SetIntVariable(inst, ret.scriptExprData, 10, (char *)EXPR_OPERATOR_NAME);
        SetFloatVariable(inst, ret.scriptExprData, fValue, (char *)EXPR_VALUE_NAME);
        SetIntVariable(inst, ret.scriptExprData, 0, (char *)EXPR_VALUE_TYPE);
        return ret;
    }
    else if ( eParseType < ET_SINGLE_VALUE && g_ExprOperatorList[eParseType].isFunction )
    {
        tokenizer->PopToken();
        return ParseFunctionCall(eParseType, tokenizer, inst, rootData);
    }
    else
    {
        switch ( eParseType )
        {
            case END_OF_RANK2:
                tokenizer->PopToken();
                v7 = ParseTopRankExpr(tokenizer, inst, rootData);
                RequireParseTypePopOrError(tokenizer, inst, ET_RIGHT_PAREN);
                return v7;
            case ET_GET_STD_PARAM:
                return ParseStdParameter(tokenizer, inst);
            case ET_GET_PARAMETER:
                return ParseParameter(tokenizer, inst, rootData);
            default:
                v5 = ConvertParseTypeToStr(eParseType);
                v6 = va("Bad Token in Expression: '%s'", v5);
                AnimTreeCompileError(inst, v6);
                ret.scriptExprData = 0;
                ret.exprType = NUMBER;
                //return 0;
        }
    }
}

const char *__cdecl ConvertParseTypeToStr(ExpressionParseTypes eType)
{
    const char *result; // eax

    if ( (unsigned int)eType < ET_SINGLE_VALUE )
        return g_ExprOperatorList[eType].strValue;
    switch ( eType )
    {
        case ET_SINGLE_VALUE:
            result = "value";
            break;
        case ET_GET_PARAMETER:
            result = "A parameter";
            break;
        case ET_GET_STD_PARAM:
            result = "A standard Param";
            break;
        case END_OF_RANK3:
            result = ",";
            break;
        default:
            result = "unknown";
            break;
    }
    return result;
}

void __cdecl RequireParseTypePopOrError(ScriptTokenizer *tokenizer, scriptInstance_t inst, ExpressionParseTypes eTest)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // [esp-4h] [ebp-10h]
    ExpressionParseTypes eParseType; // [esp+8h] [ebp-4h]

    eParseType = (ExpressionParseTypes)DetermineParseType(tokenizer);
    if ( eTest != eParseType )
    {
        v5 = ConvertParseTypeToStr(eParseType);
        v3 = ConvertParseTypeToStr(eTest);
        v4 = va("Unexpected symbol in expression: Required '%s', got '%s'", v3, v5);
        AnimTreeCompileError(inst, v4);
    }
    tokenizer->PopToken();
}

ParseValue __cdecl ParseFunctionCall(
                ExpressionParseTypes eFuncType,
                ScriptTokenizer *tokenizer,
                scriptInstance_t inst,
                unsigned int rootData)
{
    ParseValue v4; // rax
    int iValue; // [esp+10h] [ebp-F8h]
    const ExpressionOperatorData *currData; // [esp+14h] [ebp-F4h]
    ParseValue params[30]; // [esp+18h] [ebp-F0h] BYREF

    RequireParseTypePopOrError(tokenizer, inst, END_OF_RANK2);
    currData = &g_ExprOperatorList[eFuncType];
    if ( currData->iArity > 30
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
                    804,
                    0,
                    "%s",
                    "currData.iArity <= MAX_PARAMETER_ARITY") )
    {
        __debugbreak();
    }
    for ( iValue = 0; iValue < currData->iArity; ++iValue )
    {
        v4 = ParseTopRankExpr(tokenizer, inst, rootData);
        params[iValue].scriptExprData = v4.scriptExprData;
        params[iValue].exprType = v4.exprType;
        if ( iValue + 1 != currData->iArity )
            RequireParseTypePopOrError(tokenizer, inst, END_OF_RANK3);
    }
    RequireParseTypePopOrError(tokenizer, inst, ET_RIGHT_PAREN);
    return BuildExpr(eFuncType, params, currData->iArity, inst);
}

ParseValue __cdecl ParseStdParameter(ScriptTokenizer *tokenizer, scriptInstance_t inst)
{
    char *v2; // eax
    char *v3; // eax
    const char *v4; // eax
    int iParam; // [esp+8h] [ebp-Ch]
    ParseValue ret; // [esp+Ch] [ebp-8h]

    v2 = tokenizer->PeekToken();
    iParam = GetStdParamIx(v2);
    if ( iParam == 1 )
    {
        v3 = tokenizer->PeekToken();
        v4 = va("The parameter '%s' was supposed to be standard, but isn't.", v3);
        AnimTreeCompileError(inst, v4);
    }
    ret.scriptExprData = Scr_AllocArray(inst);
    ret.exprType = g_stdParamList[iParam].eType;
    SetIntVariable(inst, ret.scriptExprData, 12, (char *)EXPR_OPERATOR_NAME);
    SetIntVariable(inst, ret.scriptExprData, ret.exprType, (char *)EXPR_VALUE_TYPE);
    SetIntVariable(inst, ret.scriptExprData, iParam, (char *)EXPR_VALUE_NAME);
    return ret;
}

ParseValue __cdecl ParseParameter(ScriptTokenizer *tokenizer, scriptInstance_t inst, unsigned int rootData)
{
    char *v3; // eax
    unsigned int String; // eax
    char *v5; // eax
    const char *v6; // eax
    char *v7; // eax
    const char *v8; // eax
    unsigned int v9; // eax
    unsigned int paramData; // [esp+8h] [ebp-2Ch]
    unsigned int paramName; // [esp+Ch] [ebp-28h]
    unsigned int paramDataVar; // [esp+10h] [ebp-24h]
    unsigned int paramArray; // [esp+14h] [ebp-20h]
    unsigned int paramVar; // [esp+18h] [ebp-1Ch]
    ParseValue ret; // [esp+1Ch] [ebp-18h]
    VariableValue theVar; // [esp+24h] [ebp-10h] BYREF
    unsigned int optionNameVar; // [esp+2Ch] [ebp-8h]
    XExpr::ParamTypes eType; // [esp+30h] [ebp-4h]

    v3 = tokenizer->PopToken();
    paramName = SL_GetLowercaseString_(v3, 2u, 4, inst);
    String = SL_GetString_(inst, (char *)PARAMETER_ROOT_NAME, 2u, 4);
    paramDataVar = FindVariable(inst, rootData, String);
    if ( !paramDataVar )
    {
        v5 = SL_ConvertToString(paramName, inst);
        v6 = va("The parameter '%s' was not defined before its use in this expression.", v5);
        AnimTreeCompileError(inst, v6);
    }
    paramArray = GetArray(inst, paramDataVar);
    paramVar = FindVariable(inst, paramArray, paramName);
    if ( !paramVar )
    {
        v7 = SL_ConvertToString(paramName, inst);
        v8 = va("The parameter '%s' was not defined before its use in this expression.", v7);
        AnimTreeCompileError(inst, v8);
    }
    paramData = GetArray(inst, paramVar);
    eType = (XExpr::ParamTypes)GetNamedArrayVarInt(inst, paramData, (char *)PARAM_TYPE);
    ret.scriptExprData = Scr_AllocArray(inst);
    if ( eType >= PT_VEC2 )
    {
        if ( eType >= PT_VEC3 )
            ret.exprType = VECTOR_3D;
        else
            ret.exprType = VECTOR_2D;
    }
    else
    {
        ret.exprType = NUMBER;
    }
    SetIntVariable(inst, ret.scriptExprData, 11, (char *)EXPR_OPERATOR_NAME);
    SetIntVariable(inst, ret.scriptExprData, ret.exprType, (char *)EXPR_VALUE_TYPE);
    v9 = SL_GetString_(inst, (char *)EXPR_VALUE_NAME, 2u, 4);
    optionNameVar = GetVariable(inst, ret.scriptExprData, v9);
    theVar.type = 6;
    theVar.u.intValue = paramName;
    SetNewVariableValue(inst, optionNameVar, &theVar);
    return ret;
}

unsigned int __cdecl GetNamedArrayVarInt_0(scriptInstance_t inst, unsigned int theArray, unsigned int varName)
{
    unsigned int theVariable; // [esp+4h] [ebp-4h]

    theVariable = FindVariable(inst, theArray, varName);
    if (theVariable)
        return GetVariableValueAddress(inst, theVariable)->next;
    else
        return -1;
}

unsigned int __cdecl GetNamedArrayVarInt(scriptInstance_t inst, unsigned int theArray, char *strVarName)
{
    unsigned int varName; // [esp+0h] [ebp-4h]

    varName = SL_GetString_(inst, strVarName, 2u, 4);
    return GetNamedArrayVarInt_0(inst, theArray, varName);
}

int __cdecl GetAnimTreeParseProperties(ScriptTokenizer *tokenizer)
{
    char *v1; // eax
    char *v2; // eax
    const char *v3; // eax
    const char *v5; // [esp-4h] [ebp-4Ch]
    const char *token; // [esp+38h] [ebp-10h]
    scriptInstance_t inst; // [esp+3Ch] [ebp-Ch]
    unsigned int i; // [esp+40h] [ebp-8h]
    int flags; // [esp+44h] [ebp-4h]

    inst = tokenizer->inst;
    flags = 0;
    while ( 1 )
    {
        token = tokenizer->PeekToken();
        if ( !(token ? Scr_IsIdentifier(token) : 0) )
            break;
        for ( i = 0; i < 7; ++i )
        {
            v5 = propertyNames[i];
            v1 = tokenizer->PeekToken();
            if ( !I_stricmp(v1, v5) )
                break;
        }
        switch ( i )
        {
            case 0u:
                flags |= 1u;
                tokenizer->PopToken();
                break;
            case 1u:
                flags |= 2u;
                tokenizer->PopToken();
                break;
            case 2u:
                flags |= 8u;
                tokenizer->PopToken();
                break;
            case 3u:
                flags |= 0x10u;
                tokenizer->PopToken();
                break;
            case 4u:
                flags |= 0x20u;
                tokenizer->PopToken();
                break;
            case 5u:
                flags |= 0x40u;
                tokenizer->PopToken();
                break;
            case 6u:
                flags |= 0x88u;
                tokenizer->PopToken();
                break;
            default:
                v2 = tokenizer->PeekToken();
                v3 = va("Unknown XAnimTree anim property: \"%s\"", v2);
                AnimTreeCompileError(inst, v3);
                break;
        }
    }
    return flags;
}

void __cdecl StoreOptionsInNode(
                scriptInstance_t inst,
                unsigned int currentAnimArray,
                unsigned int iCurrOptionIx,
                unsigned int optionName,
                unsigned int iEntryIndex)
{
    unsigned int String; // eax
    unsigned int optionEntry; // [esp+0h] [ebp-14h]
    VariableValue theExpr; // [esp+4h] [ebp-10h] BYREF
    unsigned int optionNameVar; // [esp+Ch] [ebp-8h]
    unsigned int id; // [esp+10h] [ebp-4h]

    id = GetNewArrayVariable(inst, currentAnimArray, iCurrOptionIx);
    optionEntry = GetArray(inst, id);
    String = SL_GetString_(inst, (char *)OPTION_NAME, 2u, 4);
    optionNameVar = GetVariable(inst, optionEntry, String);
    theExpr.type = 6;
    theExpr.u.intValue = optionName;
    SetNewVariableValue(inst, optionNameVar, &theExpr);
    SetIntVariable(inst, optionEntry, iEntryIndex, (char *)OPTION_EXPR_INDEX);
}

void __cdecl StoreValuesInNode(
                scriptInstance_t inst,
                unsigned int currentAnimArray,
                unsigned int iCurrIx,
                unsigned int iEntryIndex)
{
    unsigned int Array; // eax
    unsigned int id; // [esp+4h] [ebp-4h]

    id = GetNewArrayVariable(inst, currentAnimArray, iCurrIx);
    Array = GetArray(inst, id);
    SetIntVariable(inst, Array, iEntryIndex, (char *)VALUE_PARAM_INDEX);
}

void __cdecl ParseParameterDef(ScriptTokenizer *tokenizer, scriptInstance_t inst, unsigned int rootData)
{
    const char *v3; // eax
    char *v4; // eax
    const char *v5; // eax
    char *v6; // eax
    unsigned int String; // eax
    char *v8; // eax
    const char *v9; // eax
    char *v10; // eax
    const char *v11; // eax
    char *v12; // eax
    const char *v13; // eax
    char *v14; // eax
    const char *v15; // eax
    char *v16; // eax
    const char *v17; // eax
    unsigned int v18; // eax
    char *v20; // [esp+10h] [ebp-7Ch]
    int v21; // [esp+14h] [ebp-78h]
    char *v23; // [esp+28h] [ebp-64h]
    char IsIdentifier; // [esp+37h] [ebp-55h]
    const char *token; // [esp+38h] [ebp-54h]
    int iType; // [esp+54h] [ebp-38h]
    unsigned int iExprIndex; // [esp+58h] [ebp-34h]
    unsigned int paramData; // [esp+5Ch] [ebp-30h]
    char *strType; // [esp+60h] [ebp-2Ch]
    unsigned int paramName; // [esp+64h] [ebp-28h]
    unsigned int paramDataVar; // [esp+68h] [ebp-24h]
    unsigned int paramArray; // [esp+6Ch] [ebp-20h]
    ParseValue theExpr; // [esp+70h] [ebp-1Ch]
    unsigned int exprArray; // [esp+78h] [ebp-14h]
    unsigned int exprDataVar; // [esp+7Ch] [ebp-10h]
    char *strClass; // [esp+80h] [ebp-Ch]
    int iClass; // [esp+84h] [ebp-8h]
    unsigned int theParam; // [esp+88h] [ebp-4h]

    strClass = tokenizer->PeekToken();
    for ( iClass = 0; iClass < 3 && I_strcmp(g_strAccessClassNames[iClass], strClass); ++iClass )
        ;
    if ( iClass == 3 )
        iClass = 0;
    else
        tokenizer->PopToken();
    strType = tokenizer->PeekToken();
    for ( iType = 0; iType < 8 && I_strcmp(g_strParamTypeToName[iType], strType); ++iType )
        ;
    if ( iType == 8 )
    {
        v3 = va("A parameter was given a type '%s' that is not a valid type.", strType);
        AnimTreeCompileError(inst, v3);
    }
    tokenizer->PopToken();
    token = tokenizer->PeekToken();
    if ( token )
        IsIdentifier = Scr_IsIdentifier(token);
    else
        IsIdentifier = 0;
    if ( !IsIdentifier )
    {
        v4 = tokenizer->PeekToken();
        v5 = va("The parameter was given a name '%s' that is not a valid name.", v4);
        AnimTreeCompileError(inst, v5);
    }
    v6 = tokenizer->PopToken();
    paramName = SL_GetLowercaseString_(v6, 2u, 4, inst);
    String = SL_GetString_(inst, (char *)PARAMETER_ROOT_NAME, 2u, 4);
    paramDataVar = FindVariable(inst, rootData, String);
    if ( !paramDataVar )
        AnimTreeCompileError(inst, "Parameter variable not found.");
    paramArray = GetArray(inst, paramDataVar);
    if ( FindVariable(inst, paramArray, paramName) )
    {
        v8 = SL_ConvertToString(paramName, inst);
        v9 = va("The parameter '%s' was defined multiple times.", v8);
        AnimTreeCompileError(inst, v9);
    }
    v23 = tokenizer->PeekToken();
    if ( !v23 || *v23 != 61 || v23[1] )
    {
        v10 = SL_ConvertToString(paramName, inst);
        v11 = va("The parameter '%s' must have an expression.", v10);
        AnimTreeCompileError(inst, v11);
    }
    tokenizer->PopToken();
    theExpr = ParseTopRankExpr(tokenizer, inst, rootData);
    if ( !theExpr.scriptExprData )
    {
        v12 = SL_ConvertToString(paramName, inst);
        v13 = va("The parameter '%s' must have an expression.", v12);
        AnimTreeCompileError(inst, v13);
    }
    if ( iType >= 2 )
    {
        if ( iType >= 5 )
            v21 = 2;
        else
            v21 = 1;
    }
    else
    {
        v21 = 0;
    }
    if ( v21 != theExpr.exprType )
    {
        v14 = SL_ConvertToString(paramName, inst);
        v15 = va("The expression of parameter '%s' does not match its type.", v14);
        AnimTreeCompileError(inst, v15);
    }
    v20 = tokenizer->PeekToken();
    if ( !v20 || *v20 != 59 || v20[1] )
    {
        v16 = SL_ConvertToString(paramName, inst);
        v17 = va("The parameter '%s' must end in a semicolon.", v16);
        AnimTreeCompileError(inst, v17);
    }
    tokenizer->PopToken();
    v18 = SL_GetString_(inst, (char *)EXPRESSION_ROOT_NAME, 2u, 4);
    exprDataVar = FindVariable(inst, rootData, v18);
    if ( !exprDataVar )
        AnimTreeCompileError(inst, "Expression variable not found.");
    exprArray = GetArray(inst, exprDataVar);
    iExprIndex = StoreExprInList(inst, exprArray, theExpr);
    theParam = GetVariable(inst, paramArray, paramName);
    paramData = GetArray(inst, theParam);
    SetIntVariable(inst, paramData, iType, (char *)PARAM_TYPE);
    SetIntVariable(inst, paramData, iClass, (char *)PARAM_CLASS);
    SetIntVariable(inst, paramData, iExprIndex, (char *)PARAM_EXPR_INDEX);
}

int __cdecl Scr_CreateAllAnimTreeData(
                scriptInstance_t inst,
                unsigned int rootData,
                unsigned int names,
                scr_animtree_t *animtree,
                unsigned int filenameId,
                unsigned int index,
                void *(__cdecl *Alloc)(int))
{
    unsigned int NamedArrayVarArray; // eax
    unsigned int v8; // eax
    XAnim_s *anims; // [esp-24h] [ebp-28h]

    NamedArrayVarArray = GetNamedArrayVarArray(inst, rootData, (char *)VALUES_ROOT_NAME);
    CreateValues(inst, animtree->anims, NamedArrayVarArray);
    anims = animtree->anims;
    v8 = GetNamedArrayVarArray(inst, rootData, (char *)NODE_ROOT_NAME);
    return Scr_CreateAnimationTree(inst, v8, rootData, names, anims, 1u, "root", 0, filenameId, index, 0, Alloc, 0);
}

int __cdecl Scr_CreateAnimationTree(
    scriptInstance_t inst,
    unsigned int parentNode,
    unsigned int rootData,
    unsigned int names,
    XAnim_s *anims,
    unsigned int childIndex,
    const char *parentName,
    unsigned int parentIndex,
    unsigned int filename,
    int treeIndex,
    unsigned __int16 flags,
    void *(__cdecl *Alloc)(int),
    unsigned int paramMap)
{
    unsigned int ValueType; // eax
    char *v14; // eax
    char *v15; // eax
    unsigned int next; // [esp+0h] [ebp-60h]
    bool bHasChildren; // [esp+43h] [ebp-1Dh]
    unsigned int nodea; // [esp+44h] [ebp-1Ch]
    unsigned int node; // [esp+44h] [ebp-1Ch]
    unsigned int nodeb; // [esp+44h] [ebp-1Ch]
    unsigned int nodeRef; // [esp+48h] [ebp-18h]
    unsigned int nodeRefa; // [esp+48h] [ebp-18h]
    unsigned int flagsId; // [esp+4Ch] [ebp-14h]
    unsigned int flagsIda; // [esp+4Ch] [ebp-14h]
    unsigned int name; // [esp+50h] [ebp-10h]
    unsigned int size; // [esp+54h] [ebp-Ch]
    unsigned int processAdditive; // [esp+58h] [ebp-8h]
    int childIndexa; // [esp+7Ch] [ebp+1Ch]
    int parentIndexa; // [esp+84h] [ebp+24h]

    size = 0;
    for (nodeRef = FindFirstSibling(inst, parentNode); nodeRef; nodeRef = FindNextSibling(inst, nodeRef))
    {
        if (GetVariableName(inst, nodeRef) < 0x10000)
            ++size;
    }
    if (parentIndex != (unsigned __int16)parentIndex
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
            2044,
            0,
            "%s",
            "parentIndex == (unsigned short)parentIndex"))
    {
        __debugbreak();
    }
    if (childIndex != (unsigned __int16)childIndex
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
            2045,
            0,
            "%s",
            "childIndex == (unsigned short)childIndex"))
    {
        __debugbreak();
    }
    if (size != (unsigned __int16)size
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
            2046,
            0,
            "%s",
            "size == (unsigned short)size"))
    {
        __debugbreak();
    }
    if (g_bChangeChecksum)
    {
        gScrVarPub[inst].checksum *= 31;
        gScrVarPub[inst].checksum += parentIndex;
        gScrVarPub[inst].checksum *= 31;
        gScrVarPub[inst].checksum += childIndex;
        gScrVarPub[inst].checksum *= 31;
        gScrVarPub[inst].checksum += size;
        gScrVarPub[inst].checksum *= 31;
        gScrVarPub[inst].checksum += flags;
    }
    XAnimBlend(anims, parentIndex, parentName, childIndex, size, flags);
    parentIndexa = childIndex;
    childIndexa = size + childIndex;
    processAdditive = 0;
LABEL_18:
    if (processAdditive <= 1)
    {
        for (nodeRefa = FindFirstSibling(inst, parentNode); ; nodeRefa = FindNextSibling(inst, nodeRefa))
        {
            if (!nodeRefa)
            {
                ++processAdditive;
                goto LABEL_18;
            }
            name = GetVariableName(inst, nodeRefa);
            if (name < 0x10000)
            {
                ValueType = GetValueType(inst, nodeRefa);
                bHasChildren = ValueType == 1;
                if (ValueType == 1)
                {
                    nodea = FindObject(inst, nodeRefa);
                    flagsId = FindArrayVariable(inst, nodea, 1u);
                    bHasChildren = flagsId && GetVariableValueAddress(inst, flagsId)->next;
                }
                if (bHasChildren)
                {
                    node = FindObject(inst, nodeRefa);
                    flagsIda = FindArrayVariable(inst, node, 0);

                    if (flagsIda)
                    {
                        next = GetVariableValueAddress(inst, flagsIda)->next;
                    }
                    else
                    {
                        //LOWORD(next) = 0;
                        next = 0;
                    }

                    if (((next & 0x10) != 0) != processAdditive)
                        continue;
                    v14 = SL_ConvertToString(name, inst);
                    childIndexa = Scr_CreateAnimationTree(
                        inst,
                        node,
                        rootData,
                        names,
                        anims,
                        childIndexa,
                        v14,
                        parentIndexa,
                        filename,
                        treeIndex,
                        next,
                        Alloc,
                        paramMap);
                }
                else
                {
                    if (processAdditive)
                        continue;
                    if (parentIndexa != (unsigned __int16)parentIndexa
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
                            2108,
                            0,
                            "%s",
                            "parentIndex == (unsigned short)parentIndex"))
                    {
                        __debugbreak();
                    }
                    if (g_bChangeChecksum)
                    {
                        gScrVarPub[inst].checksum *= 31;
                        gScrVarPub[inst].checksum += parentIndexa;
                    }
                    v15 = SL_ConvertToString(name, inst);
                    XAnimCreate(anims, parentIndexa, v15);
                }
                if (GetValueType(inst, nodeRefa) == 1)
                {
                    nodeb = FindObject(inst, nodeRefa);
                    Scr_ProcessNodeValues(inst, anims, name, rootData, parentIndexa, nodeb);
                    if (FindArrayVariable(inst, nodeb, 2u))
                        ++iFoo_2;
                }
                ConnectScriptToAnim(inst, names, parentIndexa++, filename, (unsigned __int16)name, treeIndex);
            }
        }
    }
    return childIndexa;
}

void __cdecl Scr_ProcessNodeValues(
                scriptInstance_t inst,
                XAnim_s *anims,
                unsigned int name,
                unsigned int rootNode,
                unsigned int animIndex,
                unsigned int arrayNode)
{
    unsigned int theValue; // [esp+0h] [ebp-14h]
    unsigned int currVar; // [esp+4h] [ebp-10h]
    int iValueCount; // [esp+8h] [ebp-Ch]
    unsigned int iFirstValueIndex; // [esp+Ch] [ebp-8h]
    unsigned int iValue; // [esp+10h] [ebp-4h]

    iValue = 4;
    iValueCount = 0;
    iFirstValueIndex = -1;
    for ( currVar = FindArrayVariable(inst, arrayNode, 4u); currVar; currVar = FindArrayVariable(inst, arrayNode, iValue) )
    {
        if ( iFirstValueIndex == -1 )
        {
            theValue = GetArray(inst, currVar);
            iFirstValueIndex = GetNamedArrayVarInt(inst, theValue, (char *)VALUE_PARAM_INDEX);
        }
        ++iValueCount;
        ++iValue;
    }
    if ( iValueCount )
        XAnimSetValueSection(anims, animIndex, iFirstValueIndex, iValueCount);
}

void __cdecl CreateValues(scriptInstance_t inst, XAnim_s *anims, unsigned int valueArray)
{
    char *v3; // eax
    unsigned int theValue; // [esp+4h] [ebp-14h]
    unsigned int strName; // [esp+8h] [ebp-10h]
    float fValue; // [esp+Ch] [ebp-Ch]
    unsigned int currVar; // [esp+10h] [ebp-8h]
    unsigned int iValueNum; // [esp+14h] [ebp-4h]

    iValueNum = 0;
    for ( currVar = FindArrayVariable(inst, valueArray, 0); currVar; currVar = FindArrayVariable(
                                                                                                                                                             inst,
                                                                                                                                                             valueArray,
                                                                                                                                                             iValueNum) )
    {
        theValue = GetArray(inst, currVar);
        strName = GetNamedArrayVarInt(inst, theValue, (char *)VALUE_DATA_NAME);
        fValue = GetNamedArrayVarFloat(inst, theValue, (char *)VALUE_DATA_FIELD);
        v3 = SL_ConvertToString(strName, inst);
        XAnimSetParamValue(anims, iValueNum++, v3, fValue);
    }
}

double __cdecl GetNamedArrayVarFloat(scriptInstance_t inst, unsigned int theArray, char *strVarName)
{
    unsigned int varName; // [esp+0h] [ebp-Ch]
    unsigned int theVariable; // [esp+8h] [ebp-4h]

    varName = SL_GetString_(inst, strVarName, 2u, 4);
    theVariable = FindVariable(inst, theArray, varName);
    return GetVariableValueAddress(inst, theVariable)->u.floatValue;
}

scr_animtree_t __cdecl CScr_RetrieveAnimTree(
    const char *strAnimTreeName,
    unsigned int names,
    unsigned int filename,
    unsigned __int16 iOurTreeIx)
{
    const char *v4; // eax
    const char *v5; // eax
    char *strName; // [esp+1Ch] [ebp-20h]
    int iAnimIx; // [esp+20h] [ebp-1Ch]
    unsigned int name; // [esp+24h] [ebp-18h]
    unsigned int nodeRef; // [esp+28h] [ebp-14h]
    int iSource; // [esp+2Ch] [ebp-10h]
    ClientTreeStorage *pCurrTree; // [esp+30h] [ebp-Ch]
    int iTreeIx; // [esp+34h] [ebp-8h]
    int iNumValues; // [esp+38h] [ebp-4h]
    int iNumValuesa; // [esp+38h] [ebp-4h]

    iSource = 1;
    iNumValues = gScrAnimPub[0].xanim_num[1] + 1;
    for (iTreeIx = 0;
        iTreeIx < iNumValues
        && (!gGScrXAnimTreesForClient[1][iTreeIx].strName
            || I_strcmp(strAnimTreeName, gGScrXAnimTreesForClient[1][iTreeIx].strName));
        ++iTreeIx)
    {
        ;
    }
    if (iTreeIx == iNumValues)
    {
        iSource = 0;
        iNumValuesa = gScrAnimPub[0].xanim_num[0] + 1;
        for (iTreeIx = 0;
            iTreeIx < iNumValuesa
            && (!gGScrXAnimTreesForClient[0][iTreeIx].strName
                || I_strcmp(strAnimTreeName, gGScrXAnimTreesForClient[0][iTreeIx].strName));
            ++iTreeIx)
        {
            ;
        }
        if (iTreeIx == iNumValuesa)
        {
            v4 = va("Couldn't find animtree '%s' on the server.", strAnimTreeName);
            Com_Error(ERR_DROP, v4);
        }
    }
    pCurrTree = &gGScrXAnimTreesForClient[iSource][iTreeIx];
    for (nodeRef = FindFirstSibling(SCRIPTINSTANCE_CLIENT, names);
        nodeRef;
        nodeRef = FindNextSibling(SCRIPTINSTANCE_CLIENT, nodeRef))
    {
        name = GetVariableName(SCRIPTINSTANCE_CLIENT, nodeRef);
        if (name < 0x10000)
        {
            strName = SL_ConvertToString(name, SCRIPTINSTANCE_CLIENT);
            for (iAnimIx = 0;
                iAnimIx < pCurrTree->numIndices
                && (!pCurrTree->pTreeNameMap[iAnimIx].strName[0] || I_stricmp(
                    pCurrTree->pTreeNameMap[iAnimIx].strName,
                    strName));
                ++iAnimIx)
            {
                ;
            }
            if (iAnimIx == pCurrTree->numIndices)
            {
                v5 = va(
                    "Client Script uses anim '%s' from tree '%s' that doesn't exist on the server.",
                    strName,
                    strAnimTreeName);
                Com_Error(ERR_DROP, v5);
            }
            ConnectScriptToAnim(SCRIPTINSTANCE_CLIENT, names, iAnimIx, filename, name, iOurTreeIx);
        }
    }
    return (scr_animtree_t)gGScrXAnimTreesForClient[iSource][iTreeIx].animTree.anims;
}

scr_animtree_t __cdecl Scr_FindAnimTree(scriptInstance_t inst, const char *filename)
{
    unsigned int xanimId; // [esp+2Ch] [ebp-14h]
    unsigned int filenameId; // [esp+30h] [ebp-10h]
    unsigned int fileId; // [esp+34h] [ebp-Ch]
    unsigned int fileIda; // [esp+34h] [ebp-Ch]
    VariableValue tempValue; // [esp+38h] [ebp-8h]

    if ( !gScrAnimPub[inst].animtree_loading
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
                    2716,
                    0,
                    "%s",
                    "gScrAnimPub[inst].animtree_loading") )
    {
        __debugbreak();
    }
    filenameId = Scr_CreateCanonicalFilename(inst, filename);
    fileId = FindVariable(inst, gScrAnimPub[inst].animtrees, filenameId);
    SL_RemoveRefToString(inst, filenameId);
    if ( !fileId )
        return 0;
    if ( GetVariableName(inst, fileId) >= 0x10000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
                    2727,
                    0,
                    "%s",
                    "GetVariableName( inst, fileId ) < SL_MAX_STRING_INDEX") )
    {
        __debugbreak();
    }
    GetVariableName(inst, fileId);
    fileIda = FindObject(inst, fileId);
    if ( !fileIda
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp", 2732, 0, "%s", "fileId") )
    {
        __debugbreak();
    }
    xanimId = FindVariable(inst, fileIda, 1u);
    if ( !xanimId )
        return 0;
    tempValue = Scr_EvalVariable(inst, xanimId);
    if ( tempValue.type != 7
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_animtree.cpp",
                    2739,
                    0,
                    "%s",
                    "tempValue.type == VAR_CODEPOS") )
    {
        __debugbreak();
    }
    return (scr_animtree_t)tempValue.u.intValue;
}

VariableValue __cdecl Scr_EvalVariable(scriptInstance_t inst, unsigned int id)
{
    VariableValueInternal *entryValue; // [esp+Ch] [ebp-Ch]
    VariableValue value; // [esp+10h] [ebp-8h]

    entryValue = &gScrVarGlob[inst].variableList[id + 0x8000];
    if ( (entryValue->w.status & 0x60) == 0
        && id
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    614,
                    0,
                    "%s",
                    "((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE) || !id") )
    {
        __debugbreak();
    }
    value.type = entryValue->w.status & 0x1F;
    value.u.intValue = entryValue->u.u.intValue;
    if ( value.type >= 0xDu
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    619,
                    0,
                    "%s",
                    "!IsObject( &value )") )
    {
        __debugbreak();
    }
    AddRefToValue(inst, value.type, value.u);
    return value;
}

void __cdecl AddRefToValue(scriptInstance_t inst, int type, VariableUnion u)
{
    unsigned int value; // [esp+8h] [ebp-4h]

    value = type - 1;
    if ( (unsigned int)(type - 1) < 4 )
    {
        if ( type == 1 )
        {
            AddRefToObject(inst, u.stringValue);
        }
        else if ( value > 2 )
        {
            if ( type != 4
                && !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                            422,
                            0,
                            "%s\n\t(value) = %i",
                            "(value == VAR_VECTOR - VAR_BEGIN_REF)",
                            value) )
            {
                __debugbreak();
            }
            AddRefToVector(inst, u.vectorValue);
        }
        else
        {
            SL_AddRefToString(u.stringValue, inst);
        }
    }
}

void __cdecl AddRefToObject(scriptInstance_t inst, unsigned int id)
{
    VariableValueInternal *v2; // [esp+0h] [ebp-4h]

    if ((!id || id >= 0x7FFE)
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
            343,
            0,
            "%s",
            "id >= 1 && id < VARIABLELIST_PARENT_SIZE(inst)"))
    {
        __debugbreak();
    }
    ++gScrVarPub[inst].totalObjectRefCount;
    if (gScrVarDebugPub[inst])
    {
        if (!gScrVarDebugPub[inst]->leakCount[id + 1]
            && !Assert_MyHandler(
                "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                348,
                0,
                "%s",
                "(gScrVarDebugPub[inst]->leakCount[VARIABLELIST_PARENT_BEGIN(inst)+ id])"))
        {
            __debugbreak();
        }
        ++gScrVarDebugPub[inst]->leakCount[id + 1];
    }
    if ((gScrVarGlob[inst].variableList[id + 1].w.status & 0x60) == 0
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
            353,
            0,
            "%s",
            "(gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ id].w.status & VAR_STAT_MASK) != VAR_STAT_FREE"))
    {
        __debugbreak();
    }
    v2 = &gScrVarGlob[inst].variableList[id + 1];
    if ((v2->w.status & 0x60) == 0
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
            323,
            0,
            "%s",
            "(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE"))
    {
        __debugbreak();
    }
    if ((v2->w.status & 0x1F) < 0xD
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
            354,
            0,
            "%s",
            "IsObject( &gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ id] )"))
    {
        __debugbreak();
    }
    if (!++gScrVarGlob[inst].variableList[id + 1].u.o.refCount
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
            356,
            0,
            "%s",
            "gScrVarGlob[inst].variableList[VARIABLELIST_PARENT_BEGIN(inst)+ id].u.o.refCount"))
    {
        __debugbreak();
    }
}

void __cdecl AddRefToVector(scriptInstance_t inst, const float *vectorValue)
{
    if (!*((_BYTE *)vectorValue - 1))
    {
        _InterlockedExchangeAdd(&gScrVarPub[inst].totalVectorRefCount, 1u);
        if (gScrStringDebugGlob[inst])
        {
            if (gScrStringDebugGlob[inst]->refCount[((char *)(vectorValue - 1) - gScrMemTreePub[inst].mt_buffer) / 16] < 0
                && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                    371,
                    0,
                    "%s",
                    "gScrStringDebugGlob[inst]->refCount[((char *)vectorValue - 4 - gScrMemTreePub[inst].mt_buffer) / MT_NODE_SIZE] >= 0"))
            {
                __debugbreak();
            }
            _InterlockedExchangeAdd(
                &gScrStringDebugGlob[inst]->refCount[((char *)(vectorValue - 1) - gScrMemTreePub[inst].mt_buffer) / 16],
                1u);
        }
        if (!++ * ((_WORD *)vectorValue - 2)
            && !Assert_MyHandler(
                "c:\\projects_pc\\cod\\codsrc\\src\\clientscript\\scr_variable.h",
                377,
                0,
                "%s",
                "((unsigned short *)vectorValue)[-2]"))
        {
            __debugbreak();
        }
    }
}

void __cdecl Scr_FindAnim(
                scriptInstance_t inst,
                const char *filename,
                const char *animName,
                scr_anim_s *anim,
                int user)
{
    unsigned int v5; // eax
    unsigned int name; // [esp+0h] [ebp-8h]
    unsigned int index; // [esp+4h] [ebp-4h] BYREF

    name = SL_GetLowercaseString_(animName, 0, 4, inst);
    v5 = Scr_UsingTreeInternal(inst, filename, &index, user);
    Scr_EmitAnimationInternal(inst, (char *)anim, name, v5);
    SL_RemoveRefToString(inst, name);
}

void __cdecl Scr_ShutdownAnimTrees(int user)
{
    memset(gGScrXAnimTreesForClient[user], 0, sizeof(ClientTreeStorage[128]));
}

void __cdecl RemoveRefToValue(scriptInstance_t inst, VariableValue *value)
{
    RemoveRefToValue(inst, value->type, value->u);
}

