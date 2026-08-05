#include "live_contracts.h"
#include <ui/ui_playlists.h>
#include <universal/q_parse.h>
#include <stringed/stringed_hooks.h>
#include <ui_mp/ui_main_mp.h>
#include "live_stats.h"
#include "live_win.h"
#include <qcommon/com_clients.h>
#include <client/client.h>
#include <server_mp/sv_main_mp.h>
#include <client_mp/sv_client_mp.h>

const char *CONTRACT_STAT_LAST_TIME_DEACTIVATED = "lastTimeDeactivated";
const char *CONTRACT_STAT_INDEX = "contractIndex";
const char *CONTRACT_STAT_PROGRESS = "progress";
const char *CONTRACT_STAT_TIME_PLAYED = "startingTime";
const char *CONTRACT_STAT_NUM_PURCHASED = "CONTRACTS_PURCHASED";
const char *CONTRACT_STAT_CP_SPENT = "CONTRACTS_CP_SPENT";
const char *CONTRACT_STAT_TIMES_PURCHASED = "timesPurchased";

const char *validExpirationTypes[1] =
{ "playtime" };

const char *validRequirementTypes[11] =
{
  "map",
  "gametype",
  "head",
  "body",
  "baseweapon",
  "weapon_substr",
  "perk",
  "kdratio",
  "ads",
  "inventorytype",
  "nonkillstreak"
};

const char *validStatTypes[3] =
{ "player", "gametype", "weapon" };

int s_contractCycleInterval = 86400;
int s_contractCycle = -1;



contractInfo_t contracts[512];
const char *s_contractCycleTickerMessages[32][3];
int s_contractDisplayOrder[512];
unsigned __int8 s_contractsBuffer[65536];
int s_contractFilteredList[512];
int s_numContractCycles;
int s_contractCycleOffset;
int s_numContracts;
unsigned int s_contractFilteredListCount;
int s_contractFilter;
int s_contractsStringBufferLen;
int contractSortControllerIndex;

void __cdecl LiveContracts_ParseContracts(char *buffer)
{
    unsigned int v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    unsigned int v4; // eax
    const char *v5; // eax
    char *v6; // eax
    unsigned int CurrentLanguage; // eax
    const char *LanguageName; // eax
    char *v9; // eax
    int v10; // eax
    char *v11; // eax
    char *v12; // eax
    char *v13; // eax
    char *v14; // eax
    int v15; // eax
    int v16; // eax
    char *v17; // eax
    int v18; // eax
    char *v19; // eax
    int v20; // eax
    int v21; // eax
    int v22; // eax
    int v23; // eax
    int category; // [esp+0h] [ebp-454h]
    int timesPurchasable; // [esp+4h] [ebp-450h]
    int j; // [esp+8h] [ebp-44Ch]
    bool validatedExpirationType; // [esp+Fh] [ebp-445h]
    int v28; // [esp+10h] [ebp-444h]
    char tokenString[1028]; // [esp+14h] [ebp-440h] BYREF
    unsigned int k; // [esp+41Ch] [ebp-38h]
    bool validatedReqType; // [esp+423h] [ebp-31h]
    unsigned int m; // [esp+424h] [ebp-30h]
    bool validatedStatType; // [esp+42Bh] [ebp-29h]
    int n; // [esp+42Ch] [ebp-28h]
    int cycleNumber; // [esp+430h] [ebp-24h]
    int desiredContractCycleInterval; // [esp+434h] [ebp-20h]
    int i; // [esp+438h] [ebp-1Ch]
    parseBlockTypes currentParseBlockType; // [esp+43Ch] [ebp-18h]
    int contractIndex; // [esp+440h] [ebp-14h]
    int requirementIndex; // [esp+444h] [ebp-10h]
    const char *token; // [esp+448h] [ebp-Ch]
    int lineNumber; // [esp+44Ch] [ebp-8h]
    const char *p; // [esp+450h] [ebp-4h] BYREF

    contractIndex = 0;
    requirementIndex = 0;
    memset(contracts, 0, sizeof(contracts));
    for ( i = 0; i < 512; ++i )
        s_contractDisplayOrder[i] = -1;
    lineNumber = 0;
    s_numContracts = 0;
    s_contractsStringBufferLen = 0;
    memset(s_contractCycleTickerMessages, 0, sizeof(s_contractCycleTickerMessages));
    currentParseBlockType = PARSE_UNKNOWN;
    Com_BeginParseSession("contracts");
    Com_SetSpaceDelimited(1);
    p = buffer;
LABEL_5:
    while ( 1 )
    {
        ++lineNumber;
        token = (const char *)Com_ParseOnLine(&p);
        if ( !p )
            break;
        if ( token && *p )
        {
            if ( !*token )
                goto LABEL_212;
            if ( I_stricmp(token, "contract") )
            {
                if ( I_stricmp(token, "cycleInterval") )
                {
                    if ( I_stricmp(token, "cycleOffset") )
                    {
                        if ( I_stricmp(token, "cycleMessage") )
                        {
                            if ( !I_stricmp(token, "visible") )
                            {
                                if ( !s_numContracts
                                    || s_contractCycleTickerMessages[31][s_numContracts + 2] != (const char *)contractIndex )
                                {
                                    s_contractDisplayOrder[s_numContracts++] = contractIndex;
                                }
                                goto LABEL_212;
                            }
                            if ( I_stricmp(token, "name") )
                            {
                                if ( I_stricmp(token, "desc") )
                                {
                                    if ( I_stricmp(token, "cost") )
                                    {
                                        if ( I_stricmp(token, "stat") )
                                        {
                                            if ( I_stricmp(token, "require") )
                                            {
                                                if ( I_stricmp(token, "count") )
                                                {
                                                    if ( I_stricmp(token, "cycle") )
                                                    {
                                                        if ( I_stricmp(token, "progressInterval") )
                                                        {
                                                            if ( I_stricmp(token, "expirationtype") )
                                                            {
                                                                if ( I_stricmp(token, "expirationdata") )
                                                                {
                                                                    if ( I_stricmp(token, "resetOn") )
                                                                    {
                                                                        if ( I_stricmp(token, "rewardxp") )
                                                                        {
                                                                            if ( I_stricmp(token, "rewardcp") )
                                                                            {
                                                                                if ( I_stricmp(token, "inventory") )
                                                                                {
                                                                                    if ( I_stricmp(token, "unlockLevel") )
                                                                                    {
                                                                                        if ( I_stricmp(token, "cooldown") )
                                                                                        {
                                                                                            if ( I_stricmp(token, "category") )
                                                                                            {
                                                                                                Com_Printf(
                                                                                                    16,
                                                                                                    "Contract error: line %i: Unrecognized command %s\n",
                                                                                                    lineNumber,
                                                                                                    token);
                                                                                                Com_SkipRestOfLine(&p);
                                                                                            }
                                                                                            else
                                                                                            {
                                                                                                token = (const char *)Com_ParseOnLine(&p);
                                                                                                if ( token && *p )
                                                                                                {
                                                                                                    category = atoi(token);
                                                                                                    if ( category > 0 && category <= 255 )
                                                                                                        contracts[contractIndex].category = category;
                                                                                                    else
                                                                                                        Com_Printf(
                                                                                                            16,
                                                                                                            "Contract error: line %i: Invalid value for category. Must be betwe"
                                                                                                            "en 1 and 255 inclusive.\n",
                                                                                                            lineNumber);
                                                                                                    goto LABEL_212;
                                                                                                }
                                                                                                Com_Printf(
                                                                                                    16,
                                                                                                    "Contract error: line %i: Expected number after category command\n",
                                                                                                    lineNumber);
                                                                                                Com_SkipRestOfLine(&p);
                                                                                            }
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            token = (const char *)Com_ParseOnLine(&p);
                                                                                            if ( token && *p )
                                                                                            {
                                                                                                v23 = atoi(token);
                                                                                                contracts[contractIndex].cooldown = 60 * v23;
                                                                                                Com_SkipRestOfLine(&p);
                                                                                            }
                                                                                            else
                                                                                            {
                                                                                                Com_Printf(
                                                                                                    16,
                                                                                                    "Contract error: line %i: Expected number after cooldown command\n",
                                                                                                    lineNumber);
                                                                                                Com_SkipRestOfLine(&p);
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        token = (const char *)Com_ParseOnLine(&p);
                                                                                        if ( token && *p )
                                                                                        {
                                                                                            v22 = atoi(token);
                                                                                            contracts[contractIndex].unlockLevel = v22;
                                                                                            Com_SkipRestOfLine(&p);
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            Com_Printf(
                                                                                                16,
                                                                                                "Contract error: line %i: Expected number after unlockLevel command\n",
                                                                                                lineNumber);
                                                                                            Com_SkipRestOfLine(&p);
                                                                                        }
                                                                                    }
                                                                                }
                                                                                else
                                                                                {
                                                                                    token = (const char *)Com_ParseOnLine(&p);
                                                                                    if ( token && *p )
                                                                                    {
                                                                                        timesPurchasable = atoi(token);
                                                                                        if ( timesPurchasable > 0 && timesPurchasable <= 255 )
                                                                                            contracts[contractIndex].timesPurchasable = timesPurchasable;
                                                                                        else
                                                                                            Com_Printf(
                                                                                                16,
                                                                                                "Contract error: line %i: Invalid value for inventory. Must be between 1 "
                                                                                                "and 255 inclusive.\n",
                                                                                                lineNumber);
                                                                                        goto LABEL_212;
                                                                                    }
                                                                                    Com_Printf(
                                                                                        16,
                                                                                        "Contract error: line %i: Expected number after inventory command\n",
                                                                                        lineNumber);
                                                                                    Com_SkipRestOfLine(&p);
                                                                                }
                                                                            }
                                                                            else
                                                                            {
                                                                                token = (const char *)Com_ParseOnLine(&p);
                                                                                if ( token && *p )
                                                                                {
                                                                                    v21 = atoi(token);
                                                                                    contracts[contractIndex].rewardCP = v21;
                                                                                    Com_SkipRestOfLine(&p);
                                                                                }
                                                                                else
                                                                                {
                                                                                    Com_Printf(
                                                                                        16,
                                                                                        "Contract error: line %i: Expected number after rewardcp command\n",
                                                                                        lineNumber);
                                                                                    Com_SkipRestOfLine(&p);
                                                                                }
                                                                            }
                                                                        }
                                                                        else
                                                                        {
                                                                            token = (const char *)Com_ParseOnLine(&p);
                                                                            if ( token && *p )
                                                                            {
                                                                                v20 = atoi(token);
                                                                                contracts[contractIndex].rewardXP = v20;
                                                                                Com_SkipRestOfLine(&p);
                                                                            }
                                                                            else
                                                                            {
                                                                                Com_Printf(
                                                                                    16,
                                                                                    "Contract error: line %i: Expected number after rewardxp command\n",
                                                                                    lineNumber);
                                                                                Com_SkipRestOfLine(&p);
                                                                            }
                                                                        }
                                                                    }
                                                                    else
                                                                    {
                                                                        token = (const char *)Com_ParseOnLine(&p);
                                                                        if ( token && *p )
                                                                        {
                                                                            Com_SkipRestOfLine(&p);
                                                                            v19 = LiveContracts_GetTokenString(token);
                                                                            contracts[contractIndex].resetProgressEvents = v19;
                                                                        }
                                                                        else
                                                                        {
                                                                            Com_Printf(
                                                                                16,
                                                                                "Contract error: line %i: Expected reset conditions after resetOn command\n",
                                                                                lineNumber);
                                                                            Com_SkipRestOfLine(&p);
                                                                        }
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    token = (const char *)Com_ParseOnLine(&p);
                                                                    if ( token && *p )
                                                                    {
                                                                        v18 = atoi(token);
                                                                        contracts[contractIndex].expirationData = v18;
                                                                        Com_SkipRestOfLine(&p);
                                                                    }
                                                                    else
                                                                    {
                                                                        Com_Printf(
                                                                            16,
                                                                            "Contract error: line %i: Expected value after expirationdata command\n",
                                                                            lineNumber);
                                                                        Com_SkipRestOfLine(&p);
                                                                    }
                                                                }
                                                            }
                                                            else
                                                            {
                                                                token = (const char *)Com_ParseOnLine(&p);
                                                                if ( token && *p )
                                                                {
                                                                    Com_SkipRestOfLine(&p);
                                                                    v17 = LiveContracts_GetTokenString(token);
                                                                    contracts[contractIndex].expirationType = v17;
                                                                    validatedExpirationType = 0;
                                                                    for ( j = 0; !j; j = 1 )
                                                                    {
                                                                        if ( !I_strcmp(validExpirationTypes[0], contracts[contractIndex].expirationType) )
                                                                        {
                                                                            validatedExpirationType = 1;
                                                                            break;
                                                                        }
                                                                    }
                                                                    if ( !validatedExpirationType )
                                                                        Com_Printf(
                                                                            16,
                                                                            "Contract error: line %i: Unknown expiration type after expirationtype command\n",
                                                                            lineNumber);
                                                                }
                                                                else
                                                                {
                                                                    Com_Printf(
                                                                        16,
                                                                        "Contract error: line %i: Expected type after expirationtype command\n",
                                                                        lineNumber);
                                                                    Com_SkipRestOfLine(&p);
                                                                }
                                                            }
                                                        }
                                                        else
                                                        {
                                                            token = (const char *)Com_ParseOnLine(&p);
                                                            if ( token && *p )
                                                            {
                                                                v16 = atoi(token);
                                                                contracts[contractIndex].progressInterval = v16;
                                                                if ( contracts[contractIndex].progressInterval <= 0 )
                                                                {
                                                                    contracts[contractIndex].progressInterval = 0;
                                                                    Com_Printf(
                                                                        16,
                                                                        "Contract error: line %i: Progress interval must be a positive integer\n",
                                                                        lineNumber);
                                                                }
LABEL_212:
                                                                Com_SkipRestOfLine(&p);
                                                            }
                                                            else
                                                            {
                                                                Com_Printf(
                                                                    16,
                                                                    "Contract error: line %i: Expected number after progressInterval command\n",
                                                                    lineNumber);
                                                                Com_SkipRestOfLine(&p);
                                                            }
                                                        }
                                                    }
                                                    else
                                                    {
                                                        token = (const char *)Com_ParseOnLine(&p);
                                                        if ( token && *p )
                                                        {
                                                            I_strncpyz(tokenString, token, 1024);
                                                            for ( token = strtok(tokenString, ","); token; token = strtok(0, ",") )
                                                            {
                                                                v28 = atoi(token);
                                                                if ( (unsigned int)v28 >= 0x20
                                                                    && !Assert_MyHandler(
                                                                                "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
                                                                                610,
                                                                                0,
                                                                                "%s",
                                                                                "cycleNumber >= 0 && cycleNumber < MAX_CONTRACT_CYCLES") )
                                                                {
                                                                    __debugbreak();
                                                                }
                                                                if ( v28 >= s_numContractCycles - 1 )
                                                                    s_numContractCycles = v28 + 1;
                                                                contracts[contractIndex].cycle |= 1 << v28;
                                                            }
                                                            goto LABEL_212;
                                                        }
                                                        Com_Printf(
                                                            16,
                                                            "Contract error: line %i: Expected comma-separated list of numbers after cycle command\n",
                                                            lineNumber);
                                                        Com_SkipRestOfLine(&p);
                                                    }
                                                }
                                                else
                                                {
                                                    token = (const char *)Com_ParseOnLine(&p);
                                                    if ( token && *p )
                                                    {
                                                        v15 = atoi(token);
                                                        contracts[contractIndex].count = v15;
                                                        Com_SkipRestOfLine(&p);
                                                    }
                                                    else
                                                    {
                                                        Com_Printf(16, "Contract error: line %i: Expected number after count command\n", lineNumber);
                                                        Com_SkipRestOfLine(&p);
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                token = (const char *)Com_ParseOnLine(&p);
                                                if ( token && *p )
                                                {
                                                    if ( requirementIndex < 5 )
                                                    {
                                                        v13 = LiveContracts_GetTokenString(token);
                                                        contracts[contractIndex].requirements[requirementIndex].reqType = v13;
                                                        ++p;
                                                        validatedReqType = 0;
                                                        for ( k = 0; k < 0xB; ++k )
                                                        {
                                                            if ( !I_strcmp(
                                                                            validRequirementTypes[k],
                                                                            contracts[contractIndex].requirements[requirementIndex].reqType) )
                                                            {
                                                                validatedReqType = 1;
                                                                break;
                                                            }
                                                        }
                                                        if ( !validatedReqType )
                                                            Com_Printf(
                                                                16,
                                                                "Contract error: line %i: Unknown requirement type after require command\n",
                                                                lineNumber);
                                                        token = (const char *)Com_ParseOnLine(&p);
                                                        if ( token && *p )
                                                        {
                                                            Com_SkipRestOfLine(&p);
                                                            v14 = LiveContracts_GetTokenString(token);
                                                            contracts[contractIndex].requirements[requirementIndex++].reqData = v14;
                                                        }
                                                        else
                                                        {
                                                            Com_Printf(
                                                                16,
                                                                "Contract error: line %i: Expected requirement data after name when using require command\n",
                                                                lineNumber);
                                                            Com_SkipRestOfLine(&p);
                                                        }
                                                    }
                                                    else
                                                    {
                                                        Com_Printf(16, "Contract error: line %i: Too many requirements specified\n", lineNumber);
                                                        Com_SkipRestOfLine(&p);
                                                    }
                                                }
                                                else
                                                {
                                                    Com_Printf(16, "Contract error: line %i: Expected name after require command\n", lineNumber);
                                                    Com_SkipRestOfLine(&p);
                                                }
                                            }
                                        }
                                        else
                                        {
                                            token = (const char *)Com_ParseOnLine(&p);
                                            if ( token && *p )
                                            {
                                                v11 = LiveContracts_GetTokenString(token);
                                                contracts[contractIndex].statType = v11;
                                                ++p;
                                                validatedStatType = 0;
                                                for ( m = 0; m < 3; ++m )
                                                {
                                                    if ( !I_strcmp(validStatTypes[m], contracts[contractIndex].statType) )
                                                    {
                                                        validatedStatType = 1;
                                                        break;
                                                    }
                                                }
                                                if ( !validatedStatType )
                                                    Com_Printf(16, "Contract error: line %i: Unknown stat type after stat command\n", lineNumber);
                                                token = (const char *)Com_ParseOnLine(&p);
                                                if ( token && *p )
                                                {
                                                    Com_SkipRestOfLine(&p);
                                                    v12 = LiveContracts_GetTokenString(token);
                                                    contracts[contractIndex].statName = v12;
                                                }
                                                else
                                                {
                                                    Com_Printf(
                                                        16,
                                                        "Contract error: line %i: Expected stat name after type in stat command\n",
                                                        lineNumber);
                                                    Com_SkipRestOfLine(&p);
                                                }
                                            }
                                            else
                                            {
                                                Com_Printf(16, "Contract error: line %i: Expected stat type after stat command\n", lineNumber);
                                                Com_SkipRestOfLine(&p);
                                            }
                                        }
                                    }
                                    else
                                    {
                                        token = (const char *)Com_ParseOnLine(&p);
                                        if ( token && *p )
                                        {
                                            v10 = atoi(token);
                                            contracts[contractIndex].cost = v10;
                                            Com_SkipRestOfLine(&p);
                                        }
                                        else
                                        {
                                            Com_Printf(16, "Contract error: line %i: Expected value after cost command\n", lineNumber);
                                            Com_SkipRestOfLine(&p);
                                        }
                                    }
                                }
                                else
                                {
                                    token = (const char *)Com_ParseOnLine(&p);
                                    if ( token && *p )
                                    {
                                        CurrentLanguage = SEH_GetCurrentLanguage();
                                        LanguageName = SEH_GetLanguageName(CurrentLanguage);
                                        if ( I_stricmp(token, LanguageName) )
                                            goto LABEL_212;
                                        token = (const char *)Com_ParseOnLine(&p);
                                        if ( token && *p )
                                        {
                                            Com_SkipRestOfLine(&p);
                                            v9 = LiveContracts_GetTokenString(token);
                                            contracts[contractIndex].description = v9;
                                        }
                                        else
                                        {
                                            Com_Printf(
                                                16,
                                                "Contract error: line %i: Expected contract description after desc <language> command\n",
                                                lineNumber);
                                            Com_SkipRestOfLine(&p);
                                        }
                                    }
                                    else
                                    {
                                        Com_Printf(16, "Contract error: line %i: Expected language name after desc command\n", lineNumber);
                                        Com_SkipRestOfLine(&p);
                                    }
                                }
                            }
                            else
                            {
                                token = (const char *)Com_ParseOnLine(&p);
                                if ( token && *p )
                                {
                                    v4 = SEH_GetCurrentLanguage();
                                    v5 = SEH_GetLanguageName(v4);
                                    if ( I_stricmp(token, v5) )
                                        goto LABEL_212;
                                    token = (const char *)Com_ParseOnLine(&p);
                                    if ( token && *p )
                                    {
                                        Com_SkipRestOfLine(&p);
                                        v6 = LiveContracts_GetTokenString(token);
                                        contracts[contractIndex].name = v6;
                                    }
                                    else
                                    {
                                        Com_Printf(
                                            16,
                                            "Contract error: line %i: Expected contract name after name <language> command\n",
                                            lineNumber);
                                        Com_SkipRestOfLine(&p);
                                    }
                                }
                                else
                                {
                                    Com_Printf(16, "Contract error: line %i: Expected language name after name command\n", lineNumber);
                                    Com_SkipRestOfLine(&p);
                                }
                            }
                        }
                        else if ( currentParseBlockType == PARSE_CONTRACT )
                        {
                            Com_Printf(
                                16,
                                "Contract error: line %i: cycleMessage command must be used before declaring any contracts\n",
                                lineNumber);
                            Com_SkipRestOfLine(&p);
                        }
                        else
                        {
                            token = (const char *)Com_ParseOnLine(&p);
                            if ( token && *p )
                            {
                                cycleNumber = atoi(token);
                                if ( (unsigned int)cycleNumber < 0x20 )
                                {
                                    token = (const char *)Com_ParseOnLine(&p);
                                    if ( token && *p )
                                    {
                                        v1 = SEH_GetCurrentLanguage();
                                        v2 = SEH_GetLanguageName(v1);
                                        if ( I_stricmp(token, v2) )
                                            goto LABEL_212;
                                        token = (const char *)Com_ParseOnLine(&p);
                                        if ( token && *p )
                                        {
                                            Com_SkipRestOfLine(&p);
                                            for ( n = 0; n < 3; ++n )
                                            {
                                                if ( !s_contractCycleTickerMessages[cycleNumber][n] )
                                                {
                                                    v3 = LiveContracts_GetTokenString(token);
                                                    s_contractCycleTickerMessages[cycleNumber][n] = v3;
                                                    goto LABEL_5;
                                                }
                                                if ( n == 2 )
                                                    Com_Printf(
                                                        16,
                                                        "Contract error: line %i: Too many contract cycle messages specified for cycle %i\n",
                                                        lineNumber,
                                                        cycleNumber);
                                            }
                                        }
                                        else
                                        {
                                            Com_Printf(
                                                16,
                                                "Contract error: line %i: Expected message after cycle number in cycleMessage command\n",
                                                lineNumber);
                                            Com_SkipRestOfLine(&p);
                                        }
                                    }
                                    else
                                    {
                                        Com_Printf(
                                            16,
                                            "Contract error: line %i: Expected language after cycle number in cycleMessage command\n",
                                            lineNumber);
                                        Com_SkipRestOfLine(&p);
                                    }
                                }
                                else
                                {
                                    Com_Printf(16, "Contract error: line %i: Cycle number must be in the range [0,%d)\n", lineNumber, 32);
                                    Com_SkipRestOfLine(&p);
                                }
                            }
                            else
                            {
                                Com_Printf(
                                    16,
                                    "Contract error: line %i: Expected cycle number after cycleMessage command\n",
                                    lineNumber);
                                Com_SkipRestOfLine(&p);
                            }
                        }
                    }
                    else if ( currentParseBlockType == PARSE_CONTRACT )
                    {
                        Com_Printf(
                            16,
                            "Contract error: line %i: cycleOffset command must be used before declaring any contracts\n",
                            lineNumber);
                        Com_SkipRestOfLine(&p);
                    }
                    else
                    {
                        token = (const char *)Com_ParseOnLine(&p);
                        if ( token && *p )
                        {
                            s_contractCycleOffset = atoi(token);
                            Com_SkipRestOfLine(&p);
                        }
                        else
                        {
                            Com_Printf(
                                16,
                                "Contract error: line %i: Expected number of seconds after cycleOffset command\n",
                                lineNumber);
                            Com_SkipRestOfLine(&p);
                        }
                    }
                }
                else if ( currentParseBlockType == PARSE_CONTRACT )
                {
                    Com_Printf(
                        16,
                        "Contract error: line %i: cycleInterval command must be used before declaring any contracts\n",
                        lineNumber);
                    Com_SkipRestOfLine(&p);
                }
                else
                {
                    token = (const char *)Com_ParseOnLine(&p);
                    if ( token && *p )
                    {
                        desiredContractCycleInterval = atoi(token);
                        if ( desiredContractCycleInterval <= 0
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
                                        342,
                                        0,
                                        "%s",
                                        "desiredContractCycleInterval > 0") )
                        {
                            __debugbreak();
                        }
                        if ( desiredContractCycleInterval <= 0 )
                            Com_Printf(
                                16,
                                "Contract error: line %i: Number of seconds for cycleInterval must be a positive integer\n",
                                lineNumber);
                        else
                            s_contractCycleInterval = desiredContractCycleInterval;
                        goto LABEL_212;
                    }
                    Com_Printf(
                        16,
                        "Contract error: line %i: Expected number of seconds after cycleInterval command\n",
                        lineNumber);
                    Com_SkipRestOfLine(&p);
                }
            }
            else if ( s_numContracts < 512 )
            {
                token = (const char *)Com_ParseOnLine(&p);
                if ( token && *p )
                {
                    contractIndex = atoi(token);
                    if ( (unsigned int)contractIndex < 0x200 )
                    {
                        if ( contracts[contractIndex].name )
                            Com_Printf(16, "Contract error: line %i: Contract number %i defined twice.\n", lineNumber, contractIndex);
                        if ( contracts[contractIndex].name )
                        {
                            if ( !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
                                            317,
                                            0,
                                            "%s",
                                            "contracts[contractIndex].name == NULL") )
                                __debugbreak();
                        }
                        currentParseBlockType = PARSE_CONTRACT;
                        requirementIndex = 0;
                        Com_SkipRestOfLine(&p);
                    }
                    else
                    {
                        Com_Printf(
                            16,
                            "Contract error: line %i: Invalid contract number %i after contract command\n",
                            lineNumber,
                            contractIndex);
                        Com_SkipRestOfLine(&p);
                    }
                }
                else
                {
                    Com_Printf(16, "Contract error: line %i: Expected contract number after contract command\n", lineNumber);
                    Com_SkipRestOfLine(&p);
                }
            }
            else
            {
                Com_Printf(16, "Contract error: line %i: Maximum number of contracts exceeded.\n", lineNumber);
                Com_SkipRestOfLine(&p);
            }
        }
    }
    if ( !LiveContracts_ValidateContracts()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
                    798,
                    0,
                    "%s",
                    "LiveContracts_ValidateContracts()") )
    {
        __debugbreak();
    }
    Com_Printf(16, "Contracts finished parsing. String memory required: %d bytes\n", s_contractsStringBufferLen);
    Com_EndParseSession();
}

char *__cdecl LiveContracts_GetTokenString(const char *token)
{
    char v2; // [esp+3h] [ebp-29h]
    char *v3; // [esp+8h] [ebp-24h]
    const char *v4; // [esp+Ch] [ebp-20h]
    int newStringBufferLen; // [esp+20h] [ebp-Ch]
    char *newBufferString; // [esp+24h] [ebp-8h]

    newStringBufferLen = s_contractsStringBufferLen + strlen(token) + 1;
    newBufferString = (char *)&s_contractsBuffer[s_contractsStringBufferLen];
    if ( newStringBufferLen > 0x10000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
                    127,
                    0,
                    "%s",
                    "newStringBufferLen <= CONTRACTS_BUFFER_SIZE") )
    {
        __debugbreak();
    }
    if ( newStringBufferLen <= 0x10000 )
    {
        v4 = token;
        v3 = newBufferString;
        do
        {
            v2 = *v4;
            *v3++ = *v4++;
        }
        while ( v2 );
        s_contractsStringBufferLen = newStringBufferLen;
        return newBufferString;
    }
    else
    {
        Com_PrintError(16, "ERROR: Contract string %s overflows contract string buffer\n", token);
        return (char *)"";
    }
}

char __cdecl LiveContracts_ValidateContracts()
{
    int m; // [esp+0h] [ebp-1Ch]
    int v2; // [esp+4h] [ebp-18h]
    int j; // [esp+8h] [ebp-14h]
    bool foundOneContract[4]; // [esp+Ch] [ebp-10h] BYREF
    int k; // [esp+10h] [ebp-Ch]
    int contractIndex; // [esp+14h] [ebp-8h]
    int i; // [esp+18h] [ebp-4h]

    for ( i = 0; i < s_numContracts; ++i )
    {
        contractIndex = s_contractDisplayOrder[i];
        if ( !contracts[contractIndex].name )
        {
            Com_PrintError(16, "Contract error: contract %i does not have a name\n", contractIndex);
            return 0;
        }
        if ( !contracts[contractIndex].description )
            Com_PrintError(16, "Contract error: contract %i does not have a description\n", contractIndex);
        if ( !contracts[contractIndex].category || contracts[contractIndex].category > 3u )
        {
            Com_PrintError(
                16,
                "Contract error: contract %i has an invalid category of %i. Must be between 1 and %i.\n",
                contractIndex,
                contracts[contractIndex].category,
                3);
            return 0;
        }
        if ( contracts[contractIndex].cost <= 0 )
        {
            Com_PrintError(
                16,
                "Contract error: contract %i has an invalid cost of %i. Must be greater than zero.\n",
                contractIndex,
                contracts[contractIndex].cost);
            return 0;
        }
        if ( !contracts[contractIndex].statType )
        {
            Com_PrintError(16, "Contract error: contract %i does not have a stat type.\n", contractIndex);
            return 0;
        }
        if ( !contracts[contractIndex].statName )
        {
            Com_PrintError(16, "Contract error: contract %i does not have a stat name.\n", contractIndex);
            return 0;
        }
        if ( contracts[contractIndex].count <= 0 )
        {
            Com_PrintError(
                16,
                "Contract error: contract %i has an invalid count of %i. Must be greater than zero.\n",
                contractIndex,
                contracts[contractIndex].count);
            return 0;
        }
        if ( !contracts[contractIndex].expirationType )
        {
            Com_PrintError(16, "Contract error: contract %i does not have an expiration type.\n", contractIndex);
            return 0;
        }
        if ( contracts[contractIndex].expirationData <= 0 )
        {
            Com_PrintError(
                16,
                "Contract error: contract %i has invalid expiration data of %i. Must be greater than zero.\n",
                contractIndex,
                contracts[contractIndex].expirationData);
            return 0;
        }
        if ( contracts[contractIndex].rewardXP < 0 )
        {
            Com_PrintError(
                1,
                "Contract error: contract %i has an invalid rewardxp of %i. Must be greater than or equal to zero.\n",
                contractIndex,
                contracts[contractIndex].rewardXP);
            return 0;
        }
        if ( contracts[contractIndex].rewardCP < 0 )
        {
            Com_PrintError(
                1,
                "Contract error: contract %i has an invalid rewardcp of %i. Must be greater than or equal to zero.\n",
                contractIndex,
                contracts[contractIndex].rewardCP);
            return 0;
        }
        if ( contracts[contractIndex].rewardXP <= 0 && contracts[contractIndex].rewardCP <= 0 )
        {
            Com_PrintError(16, "Contract error: contract %i does not have a reward\n", contractIndex);
            return 0;
        }
    }
    for ( k = 0; k < s_numContractCycles; ++k )
    {
        memset(foundOneContract, 0, 3);
        for ( j = 0; j < s_numContracts; ++j )
        {
            v2 = s_contractDisplayOrder[j];
            if ( (contracts[v2].cycle & (1 << k)) != 0 )
                foundOneContract[contracts[v2].category - 1] = 1;
        }
        for ( m = 0; m < 3; ++m )
        {
            if ( !foundOneContract[m] )
            {
                Com_PrintError(16, "Contract error: cycle %i does not have any contracts in category %i\n", k, m + 1);
                return 0;
            }
        }
    }
    return 1;
}

void LiveContracts_CalculateCurrentContractCycle()
{
    if ( (unsigned int)s_numContractCycles >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
                    809,
                    0,
                    "%s",
                    "( s_numContractCycles >= 0 ) && ( s_numContractCycles < MAX_CONTRACT_CYCLES )") )
    {
        __debugbreak();
    }
    if ( s_numContractCycles )
    {
        s_contractCycle = (int)(s_contractCycleOffset + LiveStorage_GetUTC()) / s_contractCycleInterval
                                        % s_numContractCycles;
        if ( (s_contractCycle < 0 || s_contractCycle >= s_numContractCycles)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
                        816,
                        0,
                        "%s",
                        "( s_contractCycle >= 0 ) && ( s_contractCycle < s_numContractCycles )") )
        {
            __debugbreak();
        }
    }
    else
    {
        s_contractCycle = 0;
    }
}

int __cdecl LiveContracts_GetTimeUntilNextCycle()
{
    if ( s_numContractCycles )
        return s_contractCycleInterval - (int)LiveStorage_GetUTC() % s_contractCycleInterval;
    else
        return -1;
}

unsigned int __cdecl LiveContracts_GetContractsCountForList()
{
    return s_contractFilteredListCount;
}

int __cdecl LiveContracts_GetContractIndexFromListIndex(int controllerIndex, int listIndex)
{
    if ( (listIndex < 0 || listIndex >= (int)s_contractFilteredListCount)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
                    962,
                    0,
                    "%s\n\t(listIndex) = %i",
                    "(( listIndex >= 0 ) && ( listIndex < s_contractFilteredListCount ))",
                    listIndex) )
    {
        __debugbreak();
    }
    return s_contractFilteredList[listIndex];
}

char *__cdecl LiveContracts_GetContractName(int index)
{
    if ( LiveContracts_ValidateContractIndex(index) )
        return contracts[index].name;
    else
        return (char *)"";
}

bool __cdecl LiveContracts_ValidateContractIndex(int contractIndex)
{
    if ( contractIndex < 0 )
        return 0;
    if ( contractIndex < 512 )
        return contracts[contractIndex].name != 0;
    return 0;
}

char *__cdecl LiveContracts_GetContractDesc(int index)
{
    if ( LiveContracts_ValidateContractIndex(index) )
        return contracts[index].description;
    else
        return (char *)"";
}

char *__cdecl LiveContracts_GetContractStatType(int index)
{
    if ( LiveContracts_ValidateContractIndex(index) )
        return contracts[index].statType;
    else
        return (char *)"";
}

char *__cdecl LiveContracts_GetContractStatName(int index)
{
    if ( LiveContracts_ValidateContractIndex(index) )
        return contracts[index].statName;
    else
        return (char *)"";
}

int __cdecl LiveContracts_GetContractCost(int index)
{
    if ( LiveContracts_ValidateContractIndex(index) )
        return contracts[index].cost;
    else
        return 0;
}

char *__cdecl LiveContracts_GetRequirementType(int contractIndex, unsigned int requirementIndex)
{
    if ( !LiveContracts_ValidateContractIndex(contractIndex) )
        return 0;
    if ( requirementIndex > 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
                    1022,
                    0,
                    "%s\n\t(requirementIndex) = %i",
                    "(( requirementIndex >= 0 ) && ( requirementIndex < 5 ))",
                    requirementIndex) )
    {
        __debugbreak();
    }
    if ( requirementIndex > 4 )
        return 0;
    if ( contracts[contractIndex].requirements[requirementIndex].reqType )
        return contracts[contractIndex].requirements[requirementIndex].reqType;
    return 0;
}

char *__cdecl LiveContracts_GetRequirementData(int contractIndex, unsigned int requirementIndex)
{
    if ( !LiveContracts_ValidateContractIndex(contractIndex) )
        return 0;
    if ( requirementIndex > 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
                    1037,
                    0,
                    "%s\n\t(requirementIndex) = %i",
                    "(( requirementIndex >= 0 ) && ( requirementIndex < 5 ))",
                    requirementIndex) )
    {
        __debugbreak();
    }
    if ( requirementIndex > 4 )
        return 0;
    if ( contracts[contractIndex].requirements[requirementIndex].reqData )
        return contracts[contractIndex].requirements[requirementIndex].reqData;
    return 0;
}

char *__cdecl LiveContracts_GetResetConditions(int contractIndex)
{
    if ( LiveContracts_ValidateContractIndex(contractIndex) )
        return contracts[contractIndex].resetProgressEvents;
    else
        return 0;
}

int __cdecl LiveContracts_GetContractRewardXP(int contractIndex)
{
    if ( LiveContracts_ValidateContractIndex(contractIndex) )
        return contracts[contractIndex].rewardXP;
    else
        return 0;
}

int __cdecl LiveContracts_GetContractRewardCP(int contractIndex)
{
    if ( LiveContracts_ValidateContractIndex(contractIndex) )
        return contracts[contractIndex].rewardCP;
    else
        return 0;
}

int __cdecl LiveContracts_GetContractRequiredCount(int index)
{
    if ( LiveContracts_ValidateContractIndex(index) )
        return contracts[index].count;
    else
        return 0;
}

char *__cdecl LiveContracts_GetExpirationType(int index)
{
    if ( LiveContracts_ValidateContractIndex(index) )
        return contracts[index].expirationType;
    else
        return (char *)"";
}

int __cdecl LiveContracts_GetExpirationData(int index)
{
    if ( LiveContracts_ValidateContractIndex(index) )
        return contracts[index].expirationData;
    else
        return 0;
}

int __cdecl LiveContracts_GetTimesPurchasable(int contractIndex)
{
    if ( !LiveContracts_ValidateContractIndex(contractIndex) )
        return 0;
    if ( ui_showAllContracts && ui_showAllContracts->current.enabled )
        return 0;
    return contracts[contractIndex].timesPurchasable;
}

int __cdecl LiveContracts_GetCooldownTime(int contractIndex)
{
    if ( !LiveContracts_ValidateContractIndex(contractIndex) )
        return 0;
    if ( ui_showAllContracts && ui_showAllContracts->current.enabled )
        return 0;
    return contracts[contractIndex].cooldown;
}

int __cdecl LiveContracts_GetUnlockLevel(int contractIndex)
{
    if ( LiveContracts_ValidateContractIndex(contractIndex) )
        return contracts[contractIndex].unlockLevel;
    else
        return 0;
}

bool __cdecl LiveContracts_IsContractLocked(int controllerIndex, int contractIndex)
{
    int UnlockLevel; // esi

    if ( !LiveContracts_ValidateContractIndex(contractIndex) )
        return 0;
    UnlockLevel = LiveContracts_GetUnlockLevel(contractIndex);
    return UnlockLevel > LiveStats_GetRank(controllerIndex) + 1;
}

unsigned int __cdecl LiveContracts_GetTimesPurchased(int controllerIndex, int contractIndex)
{
    persistentStats *statBuffer; // [esp+0h] [ebp-4h]

    if ( !LiveContracts_ValidateContractIndex(contractIndex) )
        return 0;
    statBuffer = LiveContracts_GetStatBuffer(controllerIndex);
    return LiveStats_GetContractStat((char *)statBuffer, contractIndex, CONTRACT_STAT_TIMES_PURCHASED);
}

persistentStats *__cdecl LiveContracts_GetStatBuffer(int controllerIndex)
{
    if ( controllerIndex >= 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
                    1131,
                    0,
                    "%s",
                    "controllerIndex < MAX_GPAD_COUNT") )
    {
        __debugbreak();
    }
    return LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
}

unsigned int __cdecl LiveContracts_GetLastTimeDeactivated(int controllerIndex, int contractIndex)
{
    persistentStats *statBuffer; // [esp+0h] [ebp-4h]

    if ( !LiveContracts_ValidateContractIndex(contractIndex) )
        return 0;
    statBuffer = LiveContracts_GetStatBuffer(controllerIndex);
    return LiveStats_GetContractStat((char *)statBuffer, contractIndex, CONTRACT_STAT_LAST_TIME_DEACTIVATED);
}

int __cdecl LiveContracts_GetTimeUntilCooledDown(int controllerIndex, int contractIndex)
{
    int timeRemaining; // [esp+4h] [ebp-Ch]
    unsigned int lastTimeDeactivated; // [esp+8h] [ebp-8h]
    int cooldown; // [esp+Ch] [ebp-4h]

    if ( !LiveContracts_ValidateContractIndex(contractIndex) )
        return 0;
    cooldown = LiveContracts_GetCooldownTime(contractIndex);
    if ( !cooldown )
        return 0;
    lastTimeDeactivated = LiveContracts_GetLastTimeDeactivated(controllerIndex, contractIndex);
    if ( !lastTimeDeactivated )
        return 0;
    timeRemaining = cooldown + lastTimeDeactivated - LiveStorage_GetUTC();
    if ( timeRemaining >= 0 )
        return timeRemaining;
    else
        return 0;
}

void __cdecl LiveContracts_ActivateContract(int controllerIndex, int index, unsigned int activeContractIndex)
{
    unsigned __int64 v3; // rax
    int ContractCost; // eax
    int v5; // eax
    persistentStats *buffer; // [esp+0h] [ebp-C0h]
    char gamertag[32]; // [esp+4h] [ebp-BCh] BYREF
    int currentCP; // [esp+24h] [ebp-9Ch]
    char xuidString[128]; // [esp+28h] [ebp-98h] BYREF
    int existingActiveContractIndex; // [esp+ACh] [ebp-14h]
    int contractStat; // [esp+B0h] [ebp-10h] BYREF
    int newCP; // [esp+B4h] [ebp-Ch]
    unsigned __int64 xuid; // [esp+B8h] [ebp-8h] BYREF

    buffer = LiveContracts_GetStatBuffer(controllerIndex);
    if ( activeContractIndex > 2 )
    {
        Com_Printf(16, "ERROR: Attempted to activate contract in an invalid slot!");
        return;
    }
    if ( !LiveContracts_ValidateContractIndex(index) )
    {
        Com_Printf(16, "ERROR: Attempted to activate an invalid contract!");
        return;
    }
    existingActiveContractIndex = LiveContracts_GetActiveContractIndex(controllerIndex, index);
    if ( existingActiveContractIndex >= 0 )
    {
        if ( LiveContracts_GetActiveContractStatus(controllerIndex, existingActiveContractIndex) == 1 )
            return;
        LiveContracts_DeactivateContract(controllerIndex, existingActiveContractIndex);
    }
    currentCP = LiveStats_GetCurrency(controllerIndex);
    newCP = currentCP - LiveContracts_GetContractCost(index);
    if ( newCP >= 0 )
    {
        LiveContracts_DeactivateContract(controllerIndex, activeContractIndex);
        Live_UserGetName(controllerIndex, gamertag, 32);
        LODWORD(v3) = Live_GetXuid(controllerIndex);
        xuid = v3;
        XUIDToString(&xuid, xuidString);
        ContractCost = LiveContracts_GetContractCost(index);
        LiveStats_SpendCurrency(controllerIndex, ContractCost, POINTSSPENT_CONTRACT, index);
        LiveStats_SetActiveContractStat((char *)buffer, activeContractIndex, CONTRACT_STAT_INDEX, index + 1);
        LiveStats_SetActiveContractStat((char *)buffer, activeContractIndex, CONTRACT_STAT_PROGRESS, 0);
        LiveStats_SetActiveContractStat((char *)buffer, activeContractIndex, CONTRACT_STAT_TIME_PLAYED, 0);
        contractStat = LiveStats_GetContractStat((char *)buffer, index, CONTRACT_STAT_TIMES_PURCHASED);
        LiveStats_SetContractStat((char *)buffer, index, CONTRACT_STAT_TIMES_PURCHASED, contractStat + 1);
        LiveStats_SetContractStat((char *)buffer, index, CONTRACT_STAT_LAST_TIME_DEACTIVATED, 0);
        LiveStats_GetIntPlayerStat(controllerIndex, &contractStat, CONTRACT_STAT_CP_SPENT);
        v5 = LiveContracts_GetContractCost(index);
        LiveStats_SetIntPlayerStat(controllerIndex, CONTRACT_STAT_CP_SPENT, contractStat + v5);
        LiveStats_GetIntPlayerStat(controllerIndex, &contractStat, CONTRACT_STAT_NUM_PURCHASED);
        LiveStats_SetIntPlayerStat(controllerIndex, CONTRACT_STAT_NUM_PURCHASED, contractStat + 1);
    }
    else
    {
        Com_Printf(16, "ERROR: Not enough CP to activate contract!");
    }
}

void __cdecl LiveContracts_DeactivateContract(int controllerIndex, unsigned int activeContractIndex)
{
    unsigned int UTC; // eax
    persistentStats *buffer; // [esp+0h] [ebp-8h]
    int contractIndex; // [esp+4h] [ebp-4h]

    buffer = LiveContracts_GetStatBuffer(controllerIndex);
    if ( activeContractIndex <= 2 )
    {
        contractIndex = LiveContracts_GetIndexForActiveContract(controllerIndex, activeContractIndex);
        if ( LiveContracts_ValidateContractIndex(contractIndex)
            && !LiveStats_GetContractStat((char *)buffer, contractIndex, CONTRACT_STAT_LAST_TIME_DEACTIVATED) )
        {
            UTC = LiveStorage_GetUTC();
            LiveStats_SetContractStat((char *)buffer, contractIndex, CONTRACT_STAT_LAST_TIME_DEACTIVATED, UTC);
        }
        LiveStats_SetActiveContractStat((char *)buffer, activeContractIndex, CONTRACT_STAT_INDEX, 0);
    }
    else
    {
        Com_Printf(16, "ERROR: Attempted to deactivate a contract with an invalid active contract slot!\n");
    }
}

int __cdecl LiveContracts_GetNumActiveContracts(int controllerIndex)
{
    signed int i; // [esp+4h] [ebp-Ch]
    persistentStats *buffer; // [esp+8h] [ebp-8h]
    int numActiveContracts; // [esp+Ch] [ebp-4h]

    buffer = LiveContracts_GetStatBuffer(controllerIndex);
    numActiveContracts = 0;
    for ( i = 0; i < 3; ++i )
    {
        if ( (int)LiveStats_GetActiveContractStat((char *)buffer, i, CONTRACT_STAT_INDEX) > 0 )
            ++numActiveContracts;
    }
    return numActiveContracts;
}

int __cdecl LiveContracts_GetNumContractsInProgress(int controllerIndex)
{
    signed int i; // [esp+0h] [ebp-8h]
    int numContractsInProgress; // [esp+4h] [ebp-4h]

    numContractsInProgress = 0;
    for ( i = 0; i < 3; ++i )
    {
        if ( LiveContracts_GetActiveContractStatus(controllerIndex, i) == 1 )
            ++numContractsInProgress;
    }
    return numContractsInProgress;
}

int __cdecl LiveContracts_GetActiveContractIndex(int controllerIndex, int contractIndex)
{
    signed int i; // [esp+4h] [ebp-8h]
    persistentStats *buffer; // [esp+8h] [ebp-4h]

    buffer = LiveContracts_GetStatBuffer(controllerIndex);
    if ( !LiveContracts_ValidateContractIndex(contractIndex) )
        return -1;
    for ( i = 0; i < 3; ++i )
    {
        if ( LiveStats_GetActiveContractStat((char *)buffer, i, CONTRACT_STAT_INDEX) == contractIndex + 1 )
            return i;
    }
    return -1;
}

unsigned int __cdecl LiveContracts_GetIndexForActiveContract(int controllerIndex, unsigned int activeContractIndex)
{
    persistentStats *buffer; // [esp+0h] [ebp-4h]

    buffer = LiveContracts_GetStatBuffer(controllerIndex);
    return LiveContracts_GetIndexForActiveContract_Internal((char *)buffer, activeContractIndex);
}

unsigned int __cdecl LiveContracts_GetIndexForActiveContract_Internal(char *buffer, unsigned int activeContractIndex)
{
    unsigned int contractIndex; // [esp+0h] [ebp-4h]

    if ( activeContractIndex <= 2 )
    {
        contractIndex = LiveStats_GetActiveContractStat(buffer, activeContractIndex, CONTRACT_STAT_INDEX) - 1;
        if ( LiveContracts_ValidateContractIndex(contractIndex) )
            return contractIndex;
        else
            return -1;
    }
    else
    {
        Com_Printf(16, "ERROR: Attempted to get contract index for an invalid active contract slot!\n");
        return -1;
    }
}

int __cdecl LiveContracts_GetActiveContractProgress(int controllerIndex, unsigned int activeContractIndex)
{
    int LocalClientNum; // eax
    persistentStats *buffer; // [esp+0h] [ebp-8h]
    bool isInGame; // [esp+7h] [ebp-1h]

    LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
    isInGame = CL_GetLocalClientConnectionState(LocalClientNum) == 10;
    buffer = LiveStorage_GetStatsBuffer(controllerIndex, isInGame ? STATS_LOCATION_GLOBAL : STATS_LOCATION_NORMAL, 1);
    return LiveContracts_GetActiveContractProgress_Internal((char *)buffer, activeContractIndex, 0);
}

int __cdecl LiveContracts_GetActiveContractProgress_Internal(
                char *buffer,
                unsigned int activeContractIndex,
                bool getInternalValue)
{
    int requiredCount; // [esp+0h] [ebp-10h]
    signed int contractIndex; // [esp+4h] [ebp-Ch]
    int progressInterval; // [esp+8h] [ebp-8h]
    signed int progress; // [esp+Ch] [ebp-4h]

    if ( activeContractIndex <= 2 )
    {
        contractIndex = LiveStats_GetActiveContractStat(buffer, activeContractIndex, CONTRACT_STAT_INDEX) - 1;
        if ( contractIndex >= 0 )
        {
            if ( LiveContracts_ValidateContractIndex(contractIndex) )
            {
                requiredCount = LiveContracts_GetContractRequiredCount(contractIndex);
                progressInterval = LiveContracts_GetProgressInterval(contractIndex);
                progress = LiveStats_GetActiveContractStat(buffer, activeContractIndex, CONTRACT_STAT_PROGRESS);
                if ( getInternalValue )
                {
                    if ( !progressInterval )
                        progressInterval = 1;
                    if ( progress > progressInterval * requiredCount )
                        return progressInterval * requiredCount;
                    return progress;
                }
                else
                {
                    if ( progressInterval )
                        progress /= progressInterval;
                    if ( progress > requiredCount )
                        return requiredCount;
                    return progress;
                }
            }
            else
            {
                return 0;
            }
        }
        else
        {
            Com_Printf(16, "ERROR: Attempted to get contract progress for an empty active contract slot!\n");
            return 0;
        }
    }
    else
    {
        Com_Printf(16, "ERROR: Attempted to get contract progress for an invalid active contract slot!\n");
        return 0;
    }
}

int __cdecl LiveContracts_GetProgressInterval(int contractIndex)
{
    if ( LiveContracts_ValidateContractIndex(contractIndex) )
        return contracts[contractIndex].progressInterval;
    else
        return 0;
}

int __cdecl LiveContracts_GetCombatTimeLeft(int controllerIndex, unsigned int activeContractIndex)
{
    persistentStats *buffer; // [esp+0h] [ebp-4h]

    buffer = LiveContracts_GetStatBuffer(controllerIndex);
    return LiveContracts_GetCombatTimeLeft_Internal((char *)buffer, activeContractIndex);
}

int __cdecl LiveContracts_GetCombatTimeLeft_Internal(char *buffer, unsigned int activeContractIndex)
{
    int timeLeft; // [esp+0h] [ebp-Ch]
    unsigned int index; // [esp+8h] [ebp-4h]

    index = LiveContracts_GetIndexForActiveContract_Internal(buffer, activeContractIndex);
    if ( !LiveContracts_ValidateContractIndex(index) )
        return -1;
    if ( I_stricmp(contracts[index].expirationType, validExpirationTypes[0]) )
        return -1;
    timeLeft = contracts[index].expirationData - LiveContracts_GetCombatTimePassed_Internal(buffer, activeContractIndex);
    if ( timeLeft < 0 )
        return 0;
    return timeLeft;
}

int __cdecl LiveContracts_GetCombatTimePassed_Internal(char *buffer, unsigned int activeContractIndex)
{
    signed int timePlayedSinceStart; // [esp+0h] [ebp-4h]

    timePlayedSinceStart = LiveStats_GetActiveContractStat(buffer, activeContractIndex, CONTRACT_STAT_TIME_PLAYED);
    if ( timePlayedSinceStart < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
                    1416,
                    0,
                    "%s",
                    "timePlayedSinceStart >= 0") )
    {
        __debugbreak();
    }
    if ( timePlayedSinceStart < 0 )
        return 0;
    return timePlayedSinceStart;
}

int __cdecl LiveContracts_GetActiveContractStatus_Internal(char *buffer, unsigned int activeContractIndex)
{
    unsigned int v3; // eax
    unsigned int UTC; // eax
    int contractIndex; // [esp+0h] [ebp-Ch]
    int progress; // [esp+4h] [ebp-8h]

    if ( activeContractIndex > 2 )
        return 0;
    contractIndex = LiveStats_GetActiveContractStat(buffer, activeContractIndex, CONTRACT_STAT_INDEX) - 1;
    if ( !LiveContracts_ValidateContractIndex(contractIndex) )
        return 0;
    progress = LiveContracts_GetActiveContractProgress_Internal(buffer, activeContractIndex, 0);
    if ( progress < LiveContracts_GetContractRequiredCount(contractIndex) )
    {
        if ( LiveContracts_IsActiveContractExpired_Internal(buffer, activeContractIndex) )
        {
            if ( !LiveStats_GetContractStat(buffer, contractIndex, CONTRACT_STAT_LAST_TIME_DEACTIVATED) )
            {
                UTC = LiveStorage_GetUTC();
                LiveStats_SetContractStat(buffer, contractIndex, CONTRACT_STAT_LAST_TIME_DEACTIVATED, UTC);
            }
            return 3;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        if ( !LiveStats_GetContractStat(buffer, contractIndex, CONTRACT_STAT_LAST_TIME_DEACTIVATED) )
        {
            v3 = LiveStorage_GetUTC();
            LiveStats_SetContractStat(buffer, contractIndex, CONTRACT_STAT_LAST_TIME_DEACTIVATED, v3);
        }
        return 2;
    }
}

bool __cdecl LiveContracts_IsActiveContractExpired_Internal(char *buffer, unsigned int activeContractIndex)
{
    unsigned int index; // [esp+4h] [ebp-4h]

    index = LiveContracts_GetIndexForActiveContract_Internal(buffer, activeContractIndex);
    if ( !LiveContracts_ValidateContractIndex(index) )
        return 1;
    return !I_stricmp(contracts[index].expirationType, validExpirationTypes[0])
            && LiveContracts_GetCombatTimeLeft_Internal(buffer, activeContractIndex) <= 0;
}

int __cdecl LiveContracts_GetActiveContractStatus(int controllerIndex, unsigned int activeContractIndex)
{
    int LocalClientNum; // eax
    persistentStats *buffer; // [esp+0h] [ebp-8h]
    bool isInGame; // [esp+7h] [ebp-1h]

    LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
    isInGame = CL_GetLocalClientConnectionState(LocalClientNum) == 10;
    buffer = LiveStorage_GetStatsBuffer(controllerIndex, isInGame ? STATS_LOCATION_GLOBAL : STATS_LOCATION_NORMAL, 1);
    return LiveContracts_GetActiveContractStatus_Internal((char *)buffer, activeContractIndex);
}

unsigned int __cdecl LiveContracts_SV_GetIndexForActiveContract(
                unsigned int clientNum,
                unsigned int activeContractIndex)
{
    char *buffer; // [esp+0h] [ebp-4h]

    buffer = LiveContracts_SV_GetStatBuffer(clientNum);
    return LiveContracts_GetIndexForActiveContract_Internal(buffer, activeContractIndex);
}

char *__cdecl LiveContracts_SV_GetStatBuffer(unsigned int clientNum)
{
    if ( clientNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
                    1137,
                    0,
                    "%s\n\t(clientNum) = %i",
                    "(( clientNum >= 0 ) && ( clientNum < 32 ))",
                    clientNum) )
    {
        __debugbreak();
    }
    //iassert(svs.clients[clientNum].statPacketsReceived == ALL_STATS_PACKETS_RECEIVED);
    //if ( (LODWORD(svs.clients[clientNum].statPacketsReceived) != -1 || HIDWORD(svs.clients[clientNum].statPacketsReceived) != 7)
    //    && !Assert_MyHandler(
    //                "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
    //                1138,
    //                0,
    //                "%s",
    //                "svs.clients[clientNum].statPacketsReceived == ALL_STATS_PACKETS_RECEIVED") )
    //{
    //    __debugbreak();
    //}
    iassert(svs.clients[clientNum].header.state >= CS_RECONNECTING);

    return (char *)svs.clients[clientNum].globalStats;
}

int __cdecl LiveContracts_SV_GetActiveContractProgress(unsigned int clientNum, unsigned int activeContractIndex)
{
    char *buffer; // [esp+0h] [ebp-4h]

    buffer = LiveContracts_SV_GetStatBuffer(clientNum);
    return LiveContracts_GetActiveContractProgress_Internal(buffer, activeContractIndex, 0);
}

void __cdecl LiveContracts_SV_IncrementActiveContractProgress(
                unsigned int clientNum,
                unsigned int activeContractIndex,
                int increment)
{
    char *buffer; // [esp+0h] [ebp-18h]
    int currentProgress; // [esp+4h] [ebp-14h]
    ddlState_t searchState; // [esp+8h] [ebp-10h] BYREF

    if ( !com_sv_running->current.enabled
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
                    1522,
                    0,
                    "%s",
                    "com_sv_running->current.enabled") )
    {
        __debugbreak();
    }
    if ( activeContractIndex >= 0xA
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
                    1524,
                    0,
                    "%s\n\t(activeContractIndex) = %i",
                    "(( activeContractIndex >= 0 ) && ( activeContractIndex < 10 ))",
                    activeContractIndex) )
    {
        __debugbreak();
    }
    buffer = LiveContracts_SV_GetStatBuffer(clientNum);
    if ( LiveContracts_GetActiveContractStatus_Internal(buffer, activeContractIndex) == 1 )
    {
        currentProgress = LiveContracts_GetActiveContractProgress_Internal(buffer, activeContractIndex, 1);
        if ( LiveStats_GetActiveContractStatState(activeContractIndex, CONTRACT_STAT_PROGRESS, &searchState) )
            SV_SetClientDIntStat(clientNum, &searchState, increment + currentProgress);
    }
}

void __cdecl LiveContracts_SV_ResetActiveContractProgress(unsigned int clientNum, unsigned int activeContractIndex)
{
    char *buffer; // [esp+0h] [ebp-10h]
    unsigned int contractIndex; // [esp+4h] [ebp-Ch]
    int progressInterval; // [esp+8h] [ebp-8h]
    int progress; // [esp+Ch] [ebp-4h]

    if ( !com_sv_running->current.enabled
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
                    1542,
                    0,
                    "%s",
                    "com_sv_running->current.enabled") )
    {
        __debugbreak();
    }
    if ( activeContractIndex >= 0xA
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
                    1544,
                    0,
                    "%s\n\t(activeContractIndex) = %i",
                    "(( activeContractIndex >= 0 ) && ( activeContractIndex < 10 ))",
                    activeContractIndex) )
    {
        __debugbreak();
    }
    buffer = LiveContracts_SV_GetStatBuffer(clientNum);
    if ( LiveContracts_GetActiveContractStatus_Internal(buffer, activeContractIndex) == 1 )
    {
        contractIndex = LiveContracts_GetIndexForActiveContract_Internal(buffer, activeContractIndex);
        if ( LiveContracts_ValidateContractIndex(contractIndex) )
        {
            progressInterval = LiveContracts_GetProgressInterval(contractIndex);
            if ( progressInterval > 1 )
            {
                progress = LiveContracts_GetActiveContractProgress_Internal(buffer, activeContractIndex, 1);
                LiveContracts_SV_IncrementActiveContractProgress(clientNum, activeContractIndex, -(progress % progressInterval));
            }
        }
    }
}

void __cdecl LiveContracts_SV_IncrementActiveContractTime(
                unsigned int clientNum,
                unsigned int activeContractIndex,
                int increment)
{
    char *buffer; // [esp+0h] [ebp-18h]
    ddlState_t searchState; // [esp+4h] [ebp-14h] BYREF
    int currentTime; // [esp+14h] [ebp-4h]

    if ( !com_sv_running->current.enabled
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
                    1568,
                    0,
                    "%s",
                    "com_sv_running->current.enabled") )
    {
        __debugbreak();
    }
    if ( activeContractIndex >= 0xA
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
                    1570,
                    0,
                    "%s\n\t(activeContractIndex) = %i",
                    "(( activeContractIndex >= 0 ) && ( activeContractIndex < 10 ))",
                    activeContractIndex) )
    {
        __debugbreak();
    }
    if ( increment > 0 )
    {
        buffer = LiveContracts_SV_GetStatBuffer(clientNum);
        if ( LiveContracts_GetActiveContractStatus_Internal(buffer, activeContractIndex) == 1 )
        {
            currentTime = LiveContracts_GetCombatTimePassed_Internal(buffer, activeContractIndex);
            if ( LiveStats_GetActiveContractStatState(activeContractIndex, CONTRACT_STAT_TIME_PLAYED, &searchState) )
                SV_SetClientDIntStat(clientNum, &searchState, increment + currentTime);
        }
    }
}

int __cdecl LiveContracts_SV_GetActiveContractStatus(unsigned int clientNum, unsigned int activeContractIndex)
{
    char *buffer; // [esp+0h] [ebp-4h]

    buffer = LiveContracts_SV_GetStatBuffer(clientNum);
    return LiveContracts_GetActiveContractStatus_Internal(buffer, activeContractIndex);
}

int __cdecl LiveContracts_SV_GetCombatTimePassed(unsigned int clientNum, unsigned int activeContractIndex)
{
    char *buffer; // [esp+0h] [ebp-4h]

    buffer = LiveContracts_SV_GetStatBuffer(clientNum);
    return LiveContracts_GetCombatTimePassed_Internal(buffer, activeContractIndex);
}

void __cdecl LiveContracts_SVMergeBuffers(unsigned __int8 *cacbuffer, unsigned __int8 *globalbuffer)
{
    signed int i; // [esp+0h] [ebp-Ch]
    unsigned int cac_contractidx; // [esp+4h] [ebp-8h]

    if ( (!cacbuffer || !globalbuffer)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
                    1601,
                    0,
                    "%s",
                    "cacbuffer && globalbuffer") )
    {
        __debugbreak();
    }
    if ( cacbuffer && globalbuffer )
    {
        for ( i = 0; i < 3; ++i )
        {
            cac_contractidx = LiveContracts_GetIndexForActiveContract_Internal((char *)cacbuffer, i);
            if ( cac_contractidx != LiveContracts_GetIndexForActiveContract_Internal((char *)globalbuffer, i) )
            {
                Com_DPrintf(15, "CACValidate: contractindex mismatch!\n");
                Com_DPrintf(15, "CACValidate: setting global contractidx to %i\n", cac_contractidx);
                LiveStats_SetActiveContractStat((char *)globalbuffer, i, CONTRACT_STAT_INDEX, cac_contractidx + 1);
                LiveStats_SetActiveContractStat((char *)globalbuffer, i, CONTRACT_STAT_PROGRESS, 0);
                LiveStats_SetActiveContractStat((char *)globalbuffer, i, CONTRACT_STAT_TIME_PLAYED, 0);
            }
        }
    }
}

void __cdecl LiveContracts_CLMergeBuffers(unsigned __int8 *cacbuffer, unsigned __int8 *globalbuffer)
{
    signed int i; // [esp+0h] [ebp-10h]
    int global_progress; // [esp+4h] [ebp-Ch]
    unsigned int global_contractidx; // [esp+8h] [ebp-8h]
    int global_timeplayed; // [esp+Ch] [ebp-4h]

    if ( (!cacbuffer || !globalbuffer)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_contracts.cpp",
                    1636,
                    0,
                    "%s",
                    "cacbuffer && globalbuffer") )
    {
        __debugbreak();
    }
    for ( i = 0; i < 3; ++i )
    {
        global_contractidx = LiveContracts_GetIndexForActiveContract_Internal((char *)globalbuffer, i);
        global_progress = LiveStats_GetActiveContractStat((char *)globalbuffer, i, CONTRACT_STAT_PROGRESS);
        global_timeplayed = LiveStats_GetActiveContractStat((char *)globalbuffer, i, CONTRACT_STAT_TIME_PLAYED);
        LiveStats_SetActiveContractStat((char *)cacbuffer, i, CONTRACT_STAT_INDEX, global_contractidx + 1);
        LiveStats_SetActiveContractStat((char *)cacbuffer, i, CONTRACT_STAT_PROGRESS, global_progress);
        LiveStats_SetActiveContractStat((char *)cacbuffer, i, CONTRACT_STAT_TIME_PLAYED, global_timeplayed);
    }
}

unsigned int __cdecl SV_CACValidate_GetIndexForActiveContract(unsigned __int8 *cacbuffer, unsigned int activeindex)
{
    return LiveContracts_GetIndexForActiveContract_Internal((char *)cacbuffer, activeindex);
}

void __cdecl LiveContracts_SetFilterCmd()
{
    const char *v0; // eax

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        s_contractFilter = atoi(v0);
        LiveContracts_RebuildFilteredList(0);
    }
}

void __cdecl LiveContracts_RebuildFilteredList(int controllerIndex)
{
    bool v1; // [esp+0h] [ebp-1Ch]
    signed int j; // [esp+4h] [ebp-18h]
    bool isInCurrentCycle; // [esp+Bh] [ebp-11h]
    signed int activeContractIndex; // [esp+Ch] [ebp-10h]
    int contractIndex; // [esp+10h] [ebp-Ch]
    int i; // [esp+14h] [ebp-8h]
    int maxContracts; // [esp+18h] [ebp-4h]

    if ( ui_showAllContracts && ui_showAllContracts->current.enabled )
        maxContracts = 512;
    else
        maxContracts = s_numContracts;
    LiveContracts_CalculateCurrentContractCycle();
    s_contractFilteredListCount = 0;
    for ( i = 0; i < maxContracts; ++i )
    {
        if ( ui_showAllContracts && ui_showAllContracts->current.enabled )
        {
            if ( !contracts[i].name )
                continue;
            contractIndex = i;
        }
        else
        {
            contractIndex = s_contractDisplayOrder[i];
        }
        if ( !s_contractFilter || s_contractFilter == contracts[contractIndex].category )
        {
            activeContractIndex = LiveContracts_GetActiveContractIndex(controllerIndex, contractIndex);
            if ( activeContractIndex < 0 || LiveContracts_GetActiveContractStatus(controllerIndex, activeContractIndex) != 1 )
            {
                v1 = !s_numContractCycles || (contracts[contractIndex].cycle & (1 << s_contractCycle)) != 0;
                isInCurrentCycle = v1;
                if ( ui_showAllContracts && ui_showAllContracts->current.enabled )
                    isInCurrentCycle = 1;
                if ( isInCurrentCycle )
                    s_contractFilteredList[s_contractFilteredListCount++] = i;
            }
        }
    }
    if ( !ui_showAllContracts || !ui_showAllContracts->current.enabled )
    {
        contractSortControllerIndex = controllerIndex;
        qsort(
            s_contractFilteredList,
            s_contractFilteredListCount,
            4u,
            (int (__cdecl *)(const void *, const void *))FilteredContractListSort);
        for ( j = 0; j < (int)s_contractFilteredListCount; ++j )
            s_contractFilteredList[j] = s_contractDisplayOrder[s_contractFilteredList[j]];
    }
}

int __cdecl FilteredContractListSort(int *a, int *b)
{
    int UnlockLevel; // esi
    int levelDifference; // [esp+4h] [ebp-14h]
    int contractIndexA; // [esp+8h] [ebp-10h]
    int A; // [esp+Ch] [ebp-Ch]
    int B; // [esp+10h] [ebp-8h]
    int contractIndexB; // [esp+14h] [ebp-4h]

    A = *a;
    B = *b;
    contractIndexA = s_contractDisplayOrder[*a];
    contractIndexB = s_contractDisplayOrder[*b];
    if ( LiveContracts_IsContractLocked(contractSortControllerIndex, contractIndexA)
        && !LiveContracts_IsContractLocked(contractSortControllerIndex, contractIndexB) )
    {
        return 1;
    }
    if ( !LiveContracts_IsContractLocked(contractSortControllerIndex, contractIndexA)
        && LiveContracts_IsContractLocked(contractSortControllerIndex, contractIndexB) )
    {
        return -1;
    }
    if ( !LiveContracts_IsContractLocked(contractSortControllerIndex, contractIndexA)
        || !LiveContracts_IsContractLocked(contractSortControllerIndex, contractIndexB) )
    {
        return A - B;
    }
    UnlockLevel = LiveContracts_GetUnlockLevel(contractIndexA);
    levelDifference = UnlockLevel - LiveContracts_GetUnlockLevel(contractIndexB);
    if ( levelDifference )
        return levelDifference;
    else
        return A - B;
}

