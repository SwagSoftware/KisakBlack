#include "live_news.h"
#include <win32/win_shared.h>
#include <universal/q_shared.h>
#include <ddl/ddl_api.h>
#include <cstring>
#include "live_pcache.h"
#include "live_pcache_profile.h"
#include <cgame/cg_main.h>
#include "live_contracts.h"
#include <client_mp/cl_ui_pc_mp.h>
#include "live_friends_pc.h"
#include "live_ticker.h"
#include "live_win.h"
#include <universal/com_expressions_eval.h>
#include <game_mp/g_utils_mp.h>

const dvar_t *liveNewsMaxCounters;
countersNewsState_t s_countersNewsState;
TickerMessageQueue s_tickerMessageQueue;
char outputString[128];
unsigned __int64 friendXuid[5];
bool indexOfXuidAddedToTicker[5];

cmd_function_s LiveNews_PublishNews_f_VAR;
cmd_function_s LiveNews_GetOwnNews_f_VAR;

const dvar_t *friendsNewsLastFetchTime;

char __cdecl LiveNews_NeedToGetFriendNews()
{
    unsigned int v0; // eax

    if ( (!friendsNewsLastFetchTime || (int)(Sys_Milliseconds() - friendsNewsLastFetchTime->current.integer) <= 180000)
        && ((int)Sys_Milliseconds() >= 180000 || friendsNewsLastFetchTime->current.integer) )
    {
        return 0;
    }
    v0 = Sys_Milliseconds();
    Dvar_SetInt((dvar_s *)friendsNewsLastFetchTime, v0);
    return 1;
}

void __cdecl LiveNews_PopulateTickerQueueMessages()
{
    int v0; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; ; ++i )
    {
        v0 = s_tickerMessageQueue.newsItemCount > 10 ? 10 : s_tickerMessageQueue.newsItemCount;
        if ( i >= v0 )
            break;
        LiveTicker_AddMessages(s_tickerMessageQueue.messageToBeAddedToTicker[i], TICKER_STREAM_FRIENDS, 0);
    }
}

int __cdecl LiveNews_FindDuplicateNewsItemIndex(char *outputString)
{
    int v2; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; ; ++i )
    {
        v2 = s_tickerMessageQueue.newsItemCount > 10 ? 10 : s_tickerMessageQueue.newsItemCount;
        if ( i >= v2 )
            break;
        if ( !I_strcmp(s_tickerMessageQueue.messageToBeAddedToTicker[i], outputString) )
            return i;
    }
    return -1;
}

void __cdecl LiveNews_PopulateFriendNews(int controllerIndex, unsigned __int64 xuid, char *name)
{
    char *v3; // eax
    char *v4; // eax
    ddlState_t v5; // [esp+0h] [ebp-1D0h] BYREF
    ddlState_t v6; // [esp+10h] [ebp-1C0h] BYREF
    ddlState_t searchState; // [esp+20h] [ebp-1B0h] BYREF
    ddlState_t resultState; // [esp+30h] [ebp-1A0h] BYREF
    int newsItemQueuePosition; // [esp+40h] [ebp-190h]
    int newsTypeHash; // [esp+44h] [ebp-18Ch]
    bool isValidToken; // [esp+4Bh] [ebp-185h]
    const char *newsType; // [esp+50h] [ebp-180h]
    int tokenType; // [esp+54h] [ebp-17Ch]
    int randomlyPickedNewsItemIndex; // [esp+58h] [ebp-178h]
    ddlState_t g_tickerMessage; // [esp+5Ch] [ebp-174h] BYREF
    int totalNumNewsItem; // [esp+70h] [ebp-160h]
    bool isEmptyToken; // [esp+77h] [ebp-159h]
    PCachePublicProfile *profile; // [esp+78h] [ebp-158h]
    int tickerMessageIndex; // [esp+7Ch] [ebp-154h]
    char localized_string_newstype[64]; // [esp+80h] [ebp-150h] BYREF
    char gamerTag[32]; // [esp+C0h] [ebp-110h] BYREF
    ddlState_t tickerMessageNumber; // [esp+E0h] [ebp-F0h] BYREF
    char localized_string[68]; // [esp+F0h] [ebp-E0h] BYREF
    ddlDef_t *playerDDL; // [esp+134h] [ebp-9Ch]
    const char *token; // [esp+138h] [ebp-98h]
    int tokenIndex; // [esp+13Ch] [ebp-94h]
    ddlState_t tickerMessageToken; // [esp+140h] [ebp-90h] BYREF
    ConversionArguments convArgs; // [esp+150h] [ebp-80h] BYREF
    char tempValueString[64]; // [esp+178h] [ebp-58h] BYREF
    int value; // [esp+1BCh] [ebp-14h]
    ddlState_t tickerMessageTokenNumber; // [esp+1C0h] [ebp-10h] BYREF

    newsType = 0;
    token = 0;
    tokenIndex = 0;
    tickerMessageIndex = 0;
    totalNumNewsItem = 0;
    memset(&convArgs, 0, sizeof(convArgs));
    PCache_Lock();
    profile = (PCachePublicProfile *)PCache_GetComponent(controllerIndex, xuid, 0);
    playerDDL = PCache_GetPublicProfileDDL();
    DDL_Reset(&g_tickerMessage, playerDDL);
    DDL_MoveToName(&g_tickerMessage, &g_tickerMessage, "tickerMessage");
    DDL_MoveToIndex(&g_tickerMessage, &tickerMessageNumber, tickerMessageIndex, 1);
    I_strncpyz(gamerTag, name, 32);
    convArgs.args[convArgs.argCount++] = gamerTag;
    do
    {
        DDL_MoveToName(&tickerMessageNumber, &resultState, "timestamp");
        if ( !DDL_GetInt(&resultState, profile->ddl) )
            break;
        ++totalNumNewsItem;
    }
    while ( DDL_IterateNext(&tickerMessageNumber, &tickerMessageNumber) );
    if ( totalNumNewsItem <= 0 )
        goto LABEL_26;
    randomlyPickedNewsItemIndex = CG_rand() % totalNumNewsItem;
    DDL_MoveToIndex(&g_tickerMessage, &tickerMessageNumber, randomlyPickedNewsItemIndex, 1);
    DDL_MoveToName(&tickerMessageNumber, &tickerMessageToken, "token");
    DDL_MoveToName(&tickerMessageNumber, &searchState, "messageType");
    newsTypeHash = DDL_GetInt(&searchState, profile->ddl);
    newsType = DB_FindXAssetNameFromHash(ASSET_TYPE_LOCALIZE_ENTRY, newsTypeHash);
    if ( !newsType )
    {
        Com_Printf(16, "%s's one of the ticker news is corrupted \n", gamerTag);
        PCache_Unlock();
        return;
    }
    I_strncpyz(localized_string_newstype, newsType, 64);
    while ( 1 )
    {
        DDL_MoveToIndex(&tickerMessageToken, &tickerMessageTokenNumber, tokenIndex, 1);
        DDL_MoveToName(&tickerMessageTokenNumber, &v6, "tokenType");
        tokenType = DDL_GetInt(&v6, profile->ddl);
        DDL_MoveToName(&tickerMessageTokenNumber, &v5, "value");
        value = DDL_GetInt(&v5, profile->ddl);
        if ( !tokenType && !value )
        {
            isValidToken = 1;
            isEmptyToken = 1;
            goto LABEL_23;
        }
        if ( tokenType == 1 )
        {
            sprintf(tempValueString, "%d", value);
            convArgs.args[convArgs.argCount++] = tempValueString;
            isValidToken = 1;
            isEmptyToken = 0;
            goto LABEL_21;
        }
        if ( tokenType )
            break;
        token = DB_FindXAssetNameFromHash(ASSET_TYPE_LOCALIZE_ENTRY, value);
        if ( !token )
        {
            isValidToken = 0;
            Com_Printf(16, "%s has corrupted token in one of the messages. \n", gamerTag);
            goto LABEL_23;
        }
        I_strncpyz(localized_string, token, 64);
        v3 = UI_SafeTranslateString(localized_string);
        convArgs.args[convArgs.argCount++] = v3;
        isValidToken = 1;
        isEmptyToken = 0;
LABEL_21:
        ++tokenIndex;
        if ( !isValidToken || isEmptyToken )
            goto LABEL_23;
    }
    if ( tokenType != 2 )
        goto LABEL_21;
    token = LiveContracts_GetContractName(value);
    if ( I_strcmp(token, "") )
    {
        convArgs.args[convArgs.argCount++] = token;
        isValidToken = 1;
        isEmptyToken = 0;
        goto LABEL_21;
    }
    isValidToken = 0;
    Com_Printf(16, "%s has corrupted contract token in one of the messages. \n", gamerTag);
LABEL_23:
    if ( isValidToken )
    {
        v4 = UI_SafeTranslateString(localized_string_newstype);
        UI_ReplaceConversions(v4, &convArgs, outputString, 128);
        if ( LiveNews_FindDuplicateNewsItemIndex(outputString) == -1 )
        {
            newsItemQueuePosition = s_tickerMessageQueue.newsItemCount++ % 10;
            I_strncpyz(s_tickerMessageQueue.messageToBeAddedToTicker[newsItemQueuePosition], outputString, 128);
        }
    }
LABEL_26:
    PCache_Unlock();
}

void __cdecl LiveNews_GetFriendsXuids(int controllerIndex)
{
    int v1; // eax
    int v2; // eax
    int v3; // [esp+0h] [ebp-A4Ch]
    int k; // [esp+4h] [ebp-A48h]
    int temp; // [esp+Ch] [ebp-A40h]
    int temp_4; // [esp+10h] [ebp-A3Ch]
    int j; // [esp+18h] [ebp-A34h]
    int index; // [esp+1Ch] [ebp-A30h]
    int i; // [esp+20h] [ebp-A2Ch]
    FriendInfo friendInfo; // [esp+24h] [ebp-A28h] BYREF
    int minCount; // [esp+E0h] [ebp-96Ch]
    unsigned __int64 allFriendsXuids[300]; // [esp+E4h] [ebp-968h]
    int friendCount; // [esp+A48h] [ebp-4h]

    friendCount = Friends_GetCount(controllerIndex, 0);
    if ( friendCount > 5 )
        v3 = 5;
    else
        v3 = friendCount;
    minCount = v3;
    if ( v3 == friendCount )
    {
        for ( i = 0; i < friendCount; ++i )
        {
            Friends_GetByIndex(controllerIndex, 0, i, &friendInfo);
            LODWORD(friendXuid[i]) = friendInfo.dwid;
            HIDWORD(friendXuid[i]) = HIDWORD(friendInfo.dwid);
        }
    }
    else
    {
        for ( index = 0; index < friendCount; ++index )
        {
            Friends_GetByIndex(controllerIndex, 0, index, &friendInfo);
            LODWORD(allFriendsXuids[index]) = friendInfo.dwid;
            HIDWORD(allFriendsXuids[index]) = HIDWORD(friendInfo.dwid);
        }
        for ( j = 0; j < friendCount; ++j )
        {
            v1 = CG_rand();
            temp = allFriendsXuids[j];
            temp_4 = HIDWORD(allFriendsXuids[j]);
            v2 = j + v1 % (friendCount - j);
            LODWORD(allFriendsXuids[j]) = allFriendsXuids[v2];
            HIDWORD(allFriendsXuids[j]) = HIDWORD(allFriendsXuids[v2]);
            LODWORD(allFriendsXuids[v2]) = temp;
            HIDWORD(allFriendsXuids[v2]) = temp_4;
        }
        for ( k = 0; k < 5; ++k )
        {
            LODWORD(friendXuid[k]) = allFriendsXuids[k];
            HIDWORD(friendXuid[k]) = HIDWORD(allFriendsXuids[k]);
        }
    }
    LiveTicker_ReplaceMessages("", TICKER_STREAM_FRIENDS, 0);
}

char __cdecl LiveNews_GetFriendsPublicProfile(int controllerIndex, unsigned __int64 xuid)
{
    PCachePublicProfile *friendPublicProfile; // [esp+4h] [ebp-4h]

    PCache_Lock();
    friendPublicProfile = (PCachePublicProfile *)PCache_GetComponent(controllerIndex, xuid, 0);
    if ( !friendPublicProfile
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_news.cpp", 366, 0, "%s", "friendPublicProfile") )
    {
        __debugbreak();
    }
    if ( PCache_TouchComponent(&friendPublicProfile->c) )
    {
        PCache_Unlock();
        return 1;
    }
    else
    {
        PCache_Unlock();
        return 0;
    }
}

void __cdecl LiveNews_UpdateFriendsNewsTicker(int localControllerIndex)
{
    unsigned __int64 v1; // [esp-Ch] [ebp-D8h]
    unsigned __int64 v2; // [esp-Ch] [ebp-D8h]
    unsigned __int64 v3; // [esp-8h] [ebp-D4h]
    int j; // [esp+4h] [ebp-C8h]
    int i; // [esp+8h] [ebp-C4h]
    FriendInfo friendInfo; // [esp+Ch] [ebp-C0h] BYREF

    if ( LiveNews_NeedToGetFriendNews() )
    {
        for ( i = 0; i < 5; ++i )
        {
            indexOfXuidAddedToTicker[i] = 0;
            LODWORD(friendXuid[i]) = 0;
            HIDWORD(friendXuid[i]) = 0;
        }
        LiveNews_GetFriendsXuids(localControllerIndex);
        LiveNews_PopulateTickerQueueMessages();
    }
    for ( j = 0; j < 5 && HIDWORD(friendXuid[j]) | LODWORD(friendXuid[j]); ++j )
    {
        if ( !indexOfXuidAddedToTicker[j] )
        {
            HIDWORD(v3) = HIDWORD(friendXuid[j]);
            LODWORD(v3) = friendXuid[j];
            if ( LiveNews_GetFriendsPublicProfile(localControllerIndex, v3) )
            {
                HIDWORD(v1) = HIDWORD(friendXuid[j]);
                LODWORD(v1) = friendXuid[j];
                if ( Friends_GetByID(localControllerIndex, v1, &friendInfo) )
                {
                    HIDWORD(v2) = HIDWORD(friendXuid[j]);
                    LODWORD(v2) = friendXuid[j];
                    LiveNews_PopulateFriendNews(localControllerIndex, v2, friendInfo.name);
                }
                indexOfXuidAddedToTicker[j] = 1;
            }
        }
    }
}

void __cdecl LiveNews_GetOwnNews(int controllerIndex)
{
    char *ClientName; // eax
    unsigned __int64 v2; // rax
    const char *XAssetNameFromHash; // eax
    char *v4; // eax
    char *v5; // eax
    ddlState_t v6; // [esp+0h] [ebp-248h] BYREF
    ddlState_t v7; // [esp+10h] [ebp-238h] BYREF
    ddlState_t searchState; // [esp+20h] [ebp-228h] BYREF
    ddlState_t resultState; // [esp+30h] [ebp-218h] BYREF
    const char *contractName; // [esp+40h] [ebp-208h]
    bool isValidToken; // [esp+47h] [ebp-201h]
    int tokenType; // [esp+4Ch] [ebp-1FCh]
    const char *newsType; // [esp+50h] [ebp-1F8h]
    ddlState_t g_tickerMessage; // [esp+54h] [ebp-1F4h] BYREF
    char outputString[135]; // [esp+68h] [ebp-1E0h] BYREF
    bool isEmptyToken; // [esp+EFh] [ebp-159h]
    PCachePublicProfile *profile; // [esp+F0h] [ebp-158h]
    int tickerMessageIndex; // [esp+F4h] [ebp-154h]
    char localized_string_newstype[64]; // [esp+F8h] [ebp-150h] BYREF
    char gamerTag[32]; // [esp+138h] [ebp-110h] BYREF
    ddlState_t tickerMessageNumber; // [esp+158h] [ebp-F0h] BYREF
    char localized_string[64]; // [esp+168h] [ebp-E0h] BYREF
    ddlDef_t *playerDDL; // [esp+1A8h] [ebp-A0h]
    int tokenIndex; // [esp+1ACh] [ebp-9Ch]
    ddlState_t tickerMessageToken; // [esp+1B0h] [ebp-98h] BYREF
    ConversionArguments convArgs; // [esp+1C0h] [ebp-88h] BYREF
    char tempValueString[68]; // [esp+1E8h] [ebp-60h] BYREF
    int value; // [esp+230h] [ebp-18h]
    ddlState_t tickerMessageTokenNumber; // [esp+234h] [ebp-14h] BYREF
    int newsTypeHash; // [esp+244h] [ebp-4h]

    tokenIndex = 0;
    tickerMessageIndex = 0;
    newsType = 0;
    memset(&convArgs, 0, sizeof(convArgs));
    ClientName = Live_ControllerIndex_GetClientName(controllerIndex);
    I_strncpyz(gamerTag, ClientName, 32);
    convArgs.args[convArgs.argCount++] = gamerTag;
    PCache_Lock();
    LODWORD(v2) = Live_GetXuid(0);
    profile = (PCachePublicProfile *)PCache_GetComponent(0, v2, 0);
    playerDDL = PCache_GetPublicProfileDDL();
    DDL_Reset(&g_tickerMessage, playerDDL);
    DDL_MoveToName(&g_tickerMessage, &g_tickerMessage, "tickerMessage");
    DDL_MoveToIndex(&g_tickerMessage, &tickerMessageNumber, tickerMessageIndex, 1);
    while ( 1 )
    {
        DDL_MoveToName(&tickerMessageNumber, &resultState, "timestamp");
        if ( !DDL_GetInt(&resultState, profile->ddl) )
            break;
        DDL_MoveToName(&tickerMessageNumber, &tickerMessageToken, "token");
        DDL_MoveToName(&tickerMessageNumber, &searchState, "messageType");
        newsTypeHash = DDL_GetInt(&searchState, profile->ddl);
        newsType = DB_FindXAssetNameFromHash(ASSET_TYPE_LOCALIZE_ENTRY, newsTypeHash);
        if ( newsType )
        {
            I_strncpyz(localized_string_newstype, newsType, 64);
            while ( 1 )
            {
                DDL_MoveToIndex(&tickerMessageToken, &tickerMessageTokenNumber, tokenIndex, 1);
                DDL_MoveToName(&tickerMessageTokenNumber, &v7, "tokenType");
                tokenType = DDL_GetInt(&v7, profile->ddl);
                DDL_MoveToName(&tickerMessageTokenNumber, &v6, "value");
                value = DDL_GetInt(&v6, profile->ddl);
                if ( !tokenType && !value )
                    break;
                if ( tokenType == 1 )
                {
                    sprintf(tempValueString, "%d", value);
                    convArgs.args[convArgs.argCount++] = tempValueString;
                    isValidToken = 1;
                    isEmptyToken = 0;
                }
                else if ( tokenType )
                {
                    if ( tokenType == 2 )
                    {
                        contractName = LiveContracts_GetContractName(value);
                        if ( I_strcmp(contractName, "") )
                        {
                            convArgs.args[convArgs.argCount++] = contractName;
                            isValidToken = 1;
                        }
                        else
                        {
                            isValidToken = 0;
                        }
                        isEmptyToken = 0;
                    }
                }
                else
                {
                    newsType = DB_FindXAssetNameFromHash(ASSET_TYPE_LOCALIZE_ENTRY, newsTypeHash);
                    if ( newsType )
                    {
                        XAssetNameFromHash = DB_FindXAssetNameFromHash(ASSET_TYPE_LOCALIZE_ENTRY, value);
                        I_strncpyz(localized_string, XAssetNameFromHash, 64);
                        v4 = UI_SafeTranslateString(localized_string);
                        convArgs.args[convArgs.argCount++] = v4;
                        isValidToken = 1;
                    }
                    else
                    {
                        isValidToken = 0;
                    }
                    isEmptyToken = 0;
                }
                ++tokenIndex;
                if ( !isValidToken || isEmptyToken )
                    goto LABEL_24;
            }
            isValidToken = 1;
            isEmptyToken = 1;
LABEL_24:
            if ( isValidToken )
            {
                v5 = UI_SafeTranslateString(localized_string_newstype);
                UI_ReplaceConversions(v5, &convArgs, outputString, 128);
            }
            convArgs.argCount = 1;
            tokenIndex = 0;
            if ( !DDL_IterateNext(&tickerMessageNumber, &tickerMessageNumber) )
                break;
        }
        else
        {
            Com_Printf(16, "%s's one of the ticker news is corrupted \n", gamerTag);
            if ( !DDL_IterateNext(&tickerMessageNumber, &tickerMessageNumber) )
                break;
        }
    }
    PCache_Unlock();
}

int __cdecl LiveNews_GetNextIndexForPublishingNews(PCachePublicProfile *profile, CmdArgs *cmd_args)
{
    const char *v2; // eax
    int v4; // eax
    ddlState_t v5; // [esp+0h] [ebp-124h] BYREF
    ddlState_t v6; // [esp+10h] [ebp-114h] BYREF
    ddlState_t searchState; // [esp+20h] [ebp-104h] BYREF
    ddlState_t resultState; // [esp+30h] [ebp-F4h] BYREF
    int j; // [esp+40h] [ebp-E4h]
    int i; // [esp+44h] [ebp-E0h]
    int timestamp; // [esp+48h] [ebp-DCh]
    ddlState_t g_tickerMessage; // [esp+4Ch] [ebp-D8h] BYREF
    char newsType[68]; // [esp+5Ch] [ebp-C8h] BYREF
    int firstTickerMessageIndex; // [esp+A0h] [ebp-84h]
    const StringTable *table; // [esp+A4h] [ebp-80h] BYREF
    const char *name; // [esp+A8h] [ebp-7Ch]
    int priority; // [esp+ACh] [ebp-78h]
    int timeSinceNewsPublished; // [esp+B0h] [ebp-74h]
    char localized_string_newstype[68]; // [esp+B4h] [ebp-70h] BYREF
    ddlState_t tickerMessageNumber; // [esp+FCh] [ebp-28h] BYREF
    ddlDef_t *playerDDL; // [esp+10Ch] [ebp-18h]
    int lowestPriority; // [esp+110h] [ebp-14h]
    const char *weight; // [esp+114h] [ebp-10h]
    int lowestPriorityIndex; // [esp+118h] [ebp-Ch]
    int maximumNumMessagesAllowed; // [esp+11Ch] [ebp-8h]
    int newsTypeHash; // [esp+120h] [ebp-4h]

    firstTickerMessageIndex = 0;
    lowestPriority = 0;
    lowestPriorityIndex = 0;
    name = 0;
    weight = 0;
    StringTable_GetAsset("mp/tickerWeights.csv", (XAssetHeader *)&table);
    I_strncpyz(newsType, cmd_args->argv[cmd_args->nesting][1], 64);
    v2 = StringTable_Lookup(table, 0, newsType, 2);
    maximumNumMessagesAllowed = atoi(v2);
    playerDDL = PCache_GetPublicProfileDDL();
    DDL_Reset(&g_tickerMessage, playerDDL);
    DDL_MoveToName(&g_tickerMessage, &g_tickerMessage, "tickerMessage");
    DDL_MoveToIndex(&g_tickerMessage, &tickerMessageNumber, firstTickerMessageIndex, 1);
    for ( i = 0; i < 3; ++i )
    {
        DDL_MoveToName(&tickerMessageNumber, &resultState, "timestamp");
        timestamp = DDL_GetInt(&resultState, profile->ddl);
        if ( timestamp <= 0 )
            return i;
        DDL_MoveToName(&tickerMessageNumber, &searchState, "messageType");
        newsTypeHash = DDL_GetInt(&searchState, profile->ddl);
        name = DB_FindXAssetNameFromHash(ASSET_TYPE_LOCALIZE_ENTRY, newsTypeHash);
        if ( !name )
            return i;
        I_strncpyz(localized_string_newstype, name, 64);
        if ( !I_stricmp(newsType, localized_string_newstype) && maximumNumMessagesAllowed == 1 )
            return i;
        if ( !DDL_IterateNext(&tickerMessageNumber, &tickerMessageNumber) )
            break;
    }
    DDL_MoveToIndex(&g_tickerMessage, &tickerMessageNumber, firstTickerMessageIndex, 1);
    for ( j = 0; j < 3; ++j )
    {
        DDL_MoveToName(&tickerMessageNumber, &v6, "messageType");
        newsTypeHash = DDL_GetInt(&v6, profile->ddl);
        name = DB_FindXAssetNameFromHash(ASSET_TYPE_LOCALIZE_ENTRY, newsTypeHash);
        if ( name )
        {
            I_strncpyz(localized_string_newstype, name, 64);
            weight = StringTable_Lookup(table, 0, localized_string_newstype, 1);
            DDL_MoveToName(&tickerMessageNumber, &v5, "timestamp");
            timestamp = DDL_GetInt(&v5, profile->ddl);
            timeSinceNewsPublished = LiveStorage_GetUTC() - timestamp;
            v4 = atoi(weight);
            priority = timeSinceNewsPublished * v4;
            if ( timeSinceNewsPublished * v4 > lowestPriority )
            {
                lowestPriority = priority;
                lowestPriorityIndex = j;
            }
        }
        if ( !DDL_IterateNext(&tickerMessageNumber, &tickerMessageNumber) )
            break;
    }
    return lowestPriorityIndex;
}

void __cdecl LiveNews_PublishNews(int controllerIndex, CmdArgs *cmd_args)
{
    unsigned __int64 v2; // rax
    int v3; // eax
    ddlState_t v4; // [esp+0h] [ebp-190h] BYREF
    ddlState_t v5; // [esp+10h] [ebp-180h] BYREF
    ddlState_t v6; // [esp+20h] [ebp-170h] BYREF
    ddlState_t v7; // [esp+30h] [ebp-160h] BYREF
    unsigned int value; // [esp+40h] [ebp-150h]
    ddlState_t v9; // [esp+44h] [ebp-14Ch] BYREF
    ddlState_t v10; // [esp+54h] [ebp-13Ch] BYREF
    ddlState_t searchState; // [esp+64h] [ebp-12Ch] BYREF
    ddlState_t resultState; // [esp+74h] [ebp-11Ch] BYREF
    unsigned int hash; // [esp+84h] [ebp-10Ch]
    bool isNumber; // [esp+8Bh] [ebp-105h]
    int number; // [esp+8Ch] [ebp-104h] BYREF
    char newsType[68]; // [esp+90h] [ebp-100h] BYREF
    ddlState_t g_tickerMessage; // [esp+D4h] [ebp-BCh] BYREF
    int tokenType; // [esp+E4h] [ebp-ACh]
    int argIndex; // [esp+ECh] [ebp-A4h]
    int messageNumber; // [esp+F0h] [ebp-A0h]
    PCachePublicProfile *profile; // [esp+F4h] [ebp-9Ch]
    int time; // [esp+F8h] [ebp-98h]
    ddlState_t tickerMessageNumber; // [esp+FCh] [ebp-94h] BYREF
    ddlDef_t *playerDDL; // [esp+10Ch] [ebp-84h]
    char token[68]; // [esp+110h] [ebp-80h] BYREF
    int tokenIndex; // [esp+158h] [ebp-38h]
    ddlState_t tickerMessageToken; // [esp+15Ch] [ebp-34h] BYREF
    ddlState_t tickerMessageTokenNumber; // [esp+16Ch] [ebp-24h] BYREF
    int argc; // [esp+17Ch] [ebp-14h]
    unsigned __int64 xuid; // [esp+180h] [ebp-10h]
    unsigned int newsTypeHash; // [esp+18Ch] [ebp-4h]

    PCache_Lock();
    LODWORD(v2) = Live_GetXuid(controllerIndex);
    xuid = v2;
    profile = (PCachePublicProfile *)PCache_GetComponent(controllerIndex, v2, 0);
    if ( (profile->c.state & 2) != 0 )
    {
        playerDDL = PCache_GetPublicProfileDDL();
        DDL_Reset(&g_tickerMessage, playerDDL);
        DDL_MoveToName(&g_tickerMessage, &g_tickerMessage, "tickerMessage");
        messageNumber = LiveNews_GetNextIndexForPublishingNews(profile, cmd_args);
        DDL_MoveToIndex(&g_tickerMessage, &tickerMessageNumber, messageNumber, 1);
        DDL_MoveToName(&tickerMessageNumber, &tickerMessageToken, "token");
        I_strncpyz(newsType, cmd_args->argv[cmd_args->nesting][1], 64);
        newsTypeHash = DB_HashForName(newsType, ASSET_TYPE_LOCALIZE_ENTRY);
        DDL_MoveToName(&tickerMessageNumber, &resultState, "messageType");
        DDL_SetInt(&resultState, newsTypeHash, profile->ddl);
        time = LiveStorage_GetUTC();
        DDL_MoveToName(&tickerMessageNumber, &searchState, "timestamp");
        DDL_SetInt(&searchState, time, profile->ddl);
        argc = cmd_args->argc[cmd_args->nesting];
        argIndex = 2;
        for ( tokenIndex = 0; argIndex < argc; ++tokenIndex )
        {
            I_strncpyz(token, cmd_args->argv[cmd_args->nesting][argIndex], 64);
            v3 = sscanf(token, "%d", &number);
            isNumber = v3 != 0;
            if ( !v3 || I_stricmp("TICKER_CONTRACTCOMPLETED", newsType) )
                tokenType = isNumber;
            else
                tokenType = 2;
            if ( tokenType == 1 || tokenType == 2 )
            {
                DDL_MoveToIndex(&tickerMessageToken, &tickerMessageTokenNumber, tokenIndex, 1);
                DDL_MoveToName(&tickerMessageTokenNumber, &v10, "tokenType");
                DDL_SetInt(&v10, tokenType, profile->ddl);
                value = number;
                DDL_MoveToName(&tickerMessageTokenNumber, &v9, "value");
                DDL_SetInt(&v9, value, profile->ddl);
            }
            else
            {
                DDL_MoveToIndex(&tickerMessageToken, &tickerMessageTokenNumber, tokenIndex, 1);
                DDL_MoveToName(&tickerMessageTokenNumber, &v7, "tokenType");
                DDL_SetInt(&v7, tokenType, profile->ddl);
                hash = DB_HashForName(token, ASSET_TYPE_LOCALIZE_ENTRY);
                DDL_MoveToName(&tickerMessageTokenNumber, &v6, "value");
                DDL_SetInt(&v6, hash, profile->ddl);
            }
            if ( !DDL_IterateNext(&tickerMessageTokenNumber, &tickerMessageTokenNumber) )
                break;
            ++argIndex;
        }
        DDL_MoveToIndex(&tickerMessageToken, &tickerMessageTokenNumber, tokenIndex, 1);
        DDL_MoveToName(&tickerMessageTokenNumber, &v5, "tokenType");
        DDL_SetInt(&v5, 0, profile->ddl);
        DDL_MoveToName(&tickerMessageTokenNumber, &v4, "value");
        DDL_SetInt(&v4, 0, profile->ddl);
        profile->c.state |= 1u;
        PCache_Unlock();
    }
    else
    {
        PCache_Unlock();
    }
}

void __cdecl LiveNews_PublishNews_f()
{
    CmdArgs *v0; // eax

    if ( Cmd_Argc() < 2 )
        Com_Printf(0, "USAGE: userPublishNews <newstype> <tokens(0..5)> \n");
    v0 = Cmd_Args();
    LiveNews_PublishNews(0, v0);
}

void __cdecl LiveNews_GetOwnNews_f()
{
    LiveNews_GetOwnNews(0);
}

void __cdecl LiveNews_Init()
{
    Cmd_AddCommandInternal("userPublishNews", LiveNews_PublishNews_f, &LiveNews_PublishNews_f_VAR);
    Cmd_AddCommandInternal("userGetOwnNews", LiveNews_GetOwnNews_f, &LiveNews_GetOwnNews_f_VAR);
    friendsNewsLastFetchTime = _Dvar_RegisterInt(
                                                             "friendsNewsLastFetchTime",
                                                             0,
                                                             0,
                                                             0x7FFFFFFF,
                                                             0,
                                                             "Time in milliseconds of the last time we got friends' news updates.");
    LiveNews_InitCountersTicker();
}

void __cdecl formatStringWithCommas(char *outputString, int stringLength, unsigned __int64 value)
{
    int i; // [esp+10h] [ebp-410h]
    int intermediateStringLength; // [esp+14h] [ebp-40Ch]
    char intermediateString[1028]; // [esp+18h] [ebp-408h] BYREF

    if ( !outputString
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_news.cpp", 754, 0, "%s", "outputString") )
    {
        __debugbreak();
    }
    if ( outputString )
    {
        if ( !stringLength
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_news.cpp", 758, 0, "%s", "stringLength") )
        {
            __debugbreak();
        }
        if ( stringLength )
        {
            memset(outputString, 0, stringLength);
            Com_sprintf(intermediateString, 0x400u, "%llu", value);
            intermediateStringLength = &intermediateString[strlen(intermediateString) + 1] - &intermediateString[1];
            if ( intermediateStringLength > stringLength
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_news.cpp",
                            768,
                            0,
                            "%s",
                            "intermediateStringLength <= stringLength") )
            {
                __debugbreak();
            }
            if ( intermediateStringLength <= stringLength )
            {
                for ( i = 0; i < intermediateStringLength; ++i )
                {
                    if ( !((intermediateStringLength - i) % 3) )
                    {
                        if ( i )
                            Com_sprintf(outputString, 0x400u, "%s%c", outputString, 44);
                    }
                    Com_sprintf(outputString, 0x400u, "%s%c", outputString, intermediateString[i]);
                }
            }
        }
    }
}

bool __cdecl LiveNews_EvaluateExpression(const char *expr, char *outputString, int stringLength)
{
    float v4; // [esp+8h] [ebp-42Ch]
    char compileBuffer[1024]; // [esp+Ch] [ebp-428h] BYREF
    ExpressionStatement statement; // [esp+410h] [ebp-24h] BYREF
    float floatValue; // [esp+420h] [ebp-14h]
    bool validNumber; // [esp+427h] [ebp-Dh]
    const char *expressionResult; // [esp+428h] [ebp-Ch]
    unsigned __int64 value; // [esp+42Ch] [ebp-8h]

    if ( !expr && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_news.cpp", 784, 0, "%s", "expr") )
        __debugbreak();
    if ( !expr )
        return 0;
    if ( !outputString
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_news.cpp", 788, 0, "%s", "outputString") )
    {
        __debugbreak();
    }
    if ( !outputString )
        return 0;
    if ( !stringLength
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_news.cpp", 792, 0, "%s", "stringLength") )
    {
        __debugbreak();
    }
    if ( !stringLength )
        return 0;
    validNumber = 0;
    floatValue = 0.0f;
    Expression_Parse(&expr, &statement, compileBuffer, 1024);
    expressionResult = GetExpressionResultString(0, 0, &statement);
    value = I_atoi64(expressionResult);
    if ( value < 0xA )
    {
        floatValue = atof(expressionResult);
        if ( floatValue > 0.01 && floatValue < 10.0 )
        {
            v4 = floor(floatValue);
            if ( floatValue == v4 )
                Com_sprintf(outputString, stringLength, "%llu", value);
            else
                Com_sprintf(outputString, stringLength, "%.2f", floatValue);
            return 1;
        }
    }
    else
    {
        formatStringWithCommas(outputString, stringLength, value);
        return 1;
    }
    return validNumber;
}

char __cdecl LiveNews_EvaluateCounter(int row, char *outputString, int stringLength)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *v6; // eax
    const char *v7; // eax
    char *v8; // eax
    char expressionString[1028]; // [esp+0h] [ebp-440h] BYREF
    const char *expression; // [esp+408h] [ebp-38h]
    const StringTable *countersTable; // [esp+40Ch] [ebp-34h] BYREF
    const char *stringRef; // [esp+410h] [ebp-30h]
    int weight; // [esp+414h] [ebp-2Ch]
    ConversionArguments convArgs; // [esp+418h] [ebp-28h] BYREF

    StringTable_GetAsset("mp/globalCounterRefs.csv", (XAssetHeader *)&countersTable);
    v3 = va("%d", row);
    v4 = StringTable_Lookup(countersTable, 0, v3, 1);
    weight = atoi(v4);
    if ( weight < G_rand() % 100 + 1 )
        return 0;
    v6 = va("%d", row);
    expression = StringTable_Lookup(countersTable, 0, v6, 3);
    if ( !expression || !*expression || !LiveNews_EvaluateExpression(expression, expressionString, 1024) )
        return 0;
    v7 = va("%d", row);
    stringRef = StringTable_Lookup(countersTable, 0, v7, 2);
    if ( !stringRef || !*stringRef )
        return 0;
    if ( !expressionString )
        return 0;
    convArgs.args[0] = expressionString;
    convArgs.argCount = 1;
    v8 = UI_SafeTranslateString(stringRef);
    UI_ReplaceConversions(v8, &convArgs, outputString, stringLength);
    return 1;
}

char __cdecl LiveNews_ProcessCounter()
{
    int i; // [esp+0h] [ebp-110h]
    int row; // [esp+4h] [ebp-10Ch]
    char stringRef[260]; // [esp+8h] [ebp-108h] BYREF

    row = G_rand() % s_countersNewsState.total + 1;
    for ( i = 0; i < s_countersNewsState.numProcessed; ++i )
    {
        if ( row == s_countersNewsState.counters[i] )
            return 0;
    }
    if ( !LiveNews_EvaluateCounter(row, stringRef, 256) )
        return 0;
    s_countersNewsState.counters[s_countersNewsState.numProcessed] = row;
    LiveTicker_AddMessages(stringRef, TICKER_STREAM_EVENTS, 0);
    return 1;
}

void __cdecl LiveNews_UpdateCountersTicker()
{
    if ( (int)(Sys_Milliseconds() - s_countersNewsState.lastFlushTime) >= 300000 )
    {
        s_countersNewsState.numProcessed = 0;
        s_countersNewsState.lastFlushTime = Sys_Milliseconds();
        LiveTicker_RemoveStream(TICKER_STREAM_EVENTS);
    }
    if ( s_countersNewsState.numProcessed < liveNewsMaxCounters->current.integer
        && LiveTicker_IsStreamPopulated(TICKER_STREAM_COD) )
    {
        if ( LiveNews_ProcessCounter() )
            ++s_countersNewsState.numProcessed;
    }
}

void __cdecl LiveNews_InitCountersTicker()
{
    const dvar_s *v0; // eax
    int max; // [esp+4h] [ebp-8h]
    const StringTable *countersTable; // [esp+8h] [ebp-4h] BYREF

    s_countersNewsState.numProcessed = 0;
    s_countersNewsState.lastFlushTime = Sys_Milliseconds();
    StringTable_GetAsset("mp/globalCounterRefs.csv", (XAssetHeader *)&countersTable);
    s_countersNewsState.total = StringTable_RowCount(countersTable);
    if ( s_countersNewsState.total < 10 )
        max = s_countersNewsState.total;
    else
        max = 10;
    if ( s_countersNewsState.total < 10 )
        v0 = _Dvar_RegisterInt(
                     "liveNewsMaxCounters",
                     s_countersNewsState.total,
                     0,
                     max,
                     0,
                     "Maximum global counters in the counters stream");
    else
        v0 = _Dvar_RegisterInt("liveNewsMaxCounters", 10, 0, max, 0, "Maximum global counters in the counters stream");
    liveNewsMaxCounters = v0;
}

