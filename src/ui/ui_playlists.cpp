#include "ui_playlists.h"
#include <universal/assertive.h>
#include <universal/q_shared.h>
#include <cstring>
#include <universal/q_parse.h>
#include <qcommon/common.h>
#include <stringed/stringed_hooks.h>
#include <live/live_storage_pub.h>
#include <win32/win_gamerprofile.h>
#include "ui_main.h"
#include <universal/memfile.h>
#include <qcommon/com_clients.h>
#include <live/live.h>
#include <live/live_presence_win.h>
#include <win32/win_content.h>
#include <client_mp/cl_main_mp.h>
#include <qcommon/com_gamemodes.h>
#include <server_mp/sv_main_pc_mp.h>
#include <live/live_storage_win.h>

const dvar_t *party_linearMapCycle;
const dvar_t *party_linearMapCycleRandom;
const dvar_t *party_maxplayers;
const dvar_t *playlist_excludeGametype;
const dvar_t *playlist_excludeMap;
const dvar_t *playlist_excludeGametypeMap;
const dvar_t *playlist_enabled;
const dvar_t *playlist_next;
const dvar_t *playlist_entry;

categoryInfo categories[32];
int categoryCount;

int totalEntries;
int s_playlistStringBufferUsed;
int s_playlistRuleBufferUsed;
int s_playlistRuleBufferBase;
int g_currentEntry;

unsigned __int8 s_playlistStringBuffer[32768];
int playlist_versionNum;
playlistEntry playlistEntries[8192];
int prevPlaylist[32];
playlistInfo playlists[64];
bool categoryFilter[32];
char s_playlistRuleBuffer[32768];
int numGametypes;
playlistGametype gametypes[64];

playlistEntry nullPlaylistEntry = { "", "", 0, 0, 0};



int __cdecl Playlist_GetCategoryIdByName(const char *name)
{
    int index; // [esp+0h] [ebp-4h]

    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp", 282, 0, "%s", "name") )
        __debugbreak();
    for ( index = 0; index <= categoryCount; ++index )
    {
        if ( !I_strcmp(categories[index].internalName, name) )
            return index;
    }
    return -1;
}

void __cdecl Playlist_ParsePlaylists(const char *buffer)
{
    int v1; // eax
    int v2; // eax
    int v3; // eax
    int v4; // eax
    int v5; // eax
    int v6; // eax
    int v7; // eax
    int v8; // eax
    int v9; // eax
    int v10; // eax
    int v11; // eax
    int CurrentParseLine; // eax
    unsigned int v13; // eax
    const char *v14; // eax
    int v15; // eax
    char *v16; // eax
    int v17; // eax
    char *v18; // eax
    int v19; // eax
    int v20; // eax
    int v21; // eax
    unsigned int v22; // eax
    const char *v23; // eax
    int v24; // eax
    char *v25; // eax
    int v26; // eax
    unsigned int CurrentLanguage; // eax
    const char *LanguageName; // eax
    int v29; // eax
    char *v30; // eax
    int v31; // eax
    int v32; // eax
    int v33; // eax
    int v34; // eax
    const dvar_s *Var; // eax
    int v36; // eax
    int v37; // eax
    int v38; // eax
    int v39; // eax
    int v40; // eax
    int v41; // eax
    int v42; // eax
    int v43; // eax
    int v44; // eax
    int v45; // eax
    int v46; // eax
    int v47; // eax
    int v48; // eax
    int v49; // eax
    int v50; // eax
    int v51; // eax
    int v52; // eax
    int v53; // esi
    char v54; // al
    int v55; // eax
    int v56; // eax
    int v57; // eax
    int v58; // eax
    int v59; // eax
    int v60; // eax
    char *v61; // eax
    int v62; // eax
    int v63; // eax
    int v64; // eax
    char *v65; // eax
    char *v66; // eax
    int v67; // eax
    int v68; // eax
    int v69; // [esp-4h] [ebp-644h]
    const char *localizedName; // [esp-4h] [ebp-644h]
    const char *internalName; // [esp-4h] [ebp-644h]
    int v72; // [esp-4h] [ebp-644h]
    const char *v73; // [esp-4h] [ebp-644h]
    int v74; // [esp-4h] [ebp-644h]
    int v75; // [esp-4h] [ebp-644h]
    const char *v76; // [esp+8h] [ebp-638h]
    const char *v77; // [esp+10h] [ebp-630h]
    char dest[512]; // [esp+18h] [ebp-628h] BYREF
    char key[512]; // [esp+218h] [ebp-428h] BYREF
    char value[512]; // [esp+418h] [ebp-228h] BYREF
    int index; // [esp+61Ch] [ebp-24h]
    int foundLocalizedName; // [esp+620h] [ebp-20h]
    int playlistNum; // [esp+624h] [ebp-1Ch]
    //parseBlockTypes currentParseBlockType; // [esp+628h] [ebp-18h]
    int currentParseBlockType; // [esp+628h] [ebp-18h]
    playlistEntry *entry; // [esp+62Ch] [ebp-14h]
    int currentCategory; // [esp+630h] [ebp-10h]
    const char *token; // [esp+634h] [ebp-Ch]
    int foundLocalizedDesc; // [esp+638h] [ebp-8h]
    const char *p; // [esp+63Ch] [ebp-4h] BYREF

    totalEntries = 0;
    playlistNum = 0;
    numGametypes = -1;
    categoryCount = -1;
    foundLocalizedName = 0;
    foundLocalizedDesc = 0;
    memset(playlists, 0, sizeof(playlists));
    memset(gametypes, 0, sizeof(gametypes));
    memset(categories, 0, sizeof(categories));
    memset(prevPlaylist, 0, sizeof(prevPlaylist));
    *(unsigned int *)categoryFilter = 0;
    *(unsigned int *)&categoryFilter[4] = 0;
    *(unsigned int *)&categoryFilter[8] = 0;
    *(unsigned int *)&categoryFilter[12] = 0;
    *(unsigned int *)&categoryFilter[16] = 0;
    *(unsigned int *)&categoryFilter[20] = 0;
    *(unsigned int *)&categoryFilter[24] = 0;
    *(unsigned int *)&categoryFilter[28] = 0;
    Playlist_SetPrevPlaylist();
    s_playlistStringBufferUsed = 0;
    s_playlistRuleBufferBase = 0;
    s_playlistRuleBufferUsed = 0;
    currentParseBlockType = PARSE_UNKNOWN;
    Com_BeginParseSession("playlists");
    Com_SetSpaceDelimited(1);
    p = buffer;
    while ( 1 )
    {
        token = (const char *)Com_ParseOnLine(&p);
        if ( !p )
            break;
        if ( token && *p )
        {
            if ( !*token )
                goto LABEL_127;
            if ( I_stricmp(token, "version") )
            {
                if ( I_stricmp(token, "settings") )
                {
                    if ( I_stricmp(token, "gametype") )
                    {
                        if ( I_stricmp(token, "category") || currentParseBlockType == 2 )
                        {
                            if ( I_stricmp(token, "categoryCoop") || currentParseBlockType == 2 )
                            {
                                if ( I_stricmp(token, "playlist") )
                                {
                                    if ( I_stricmp(token, "playlistCoop") )
                                    {
                                        if ( I_stricmp(token, "script") )
                                        {
                                            if ( !I_stricmp(token, "rule") || !I_stricmp(token, "set") )
                                            {
                                                token = Com_ParseRestOfLine(&p);
                                                switch ( currentParseBlockType )
                                                {
                                                    case 1:
                                                        if ( !Playlist_AppendRules(gametypes[numGametypes].rules, token) )
                                                            Com_PrintError(
                                                                16,
                                                                "Error adding '%s' to gametype %s rules\n",
                                                                token,
                                                                gametypes[numGametypes].internalName);
                                                        break;
                                                    case 2:
                                                        if ( !Playlist_AppendRules(playlists[playlistNum].rules, token) )
                                                            Com_PrintError(16, "Error adding '%s' to playlist %i rules\n", token, playlistNum);
                                                        break;
                                                    case 3:
                                                        if ( !Playlist_AppendRules(categories[categoryCount].rules, token) )
                                                            Com_PrintError(
                                                                16,
                                                                "Error adding '%s' to category %s rules\n",
                                                                token,
                                                                categories[categoryCount].internalName);
                                                        break;
                                                    case 4:
                                                        goto LABEL_127;
                                                    default:
                                                        CurrentParseLine = Com_GetCurrentParseLine();
                                                        Com_PrintError(
                                                            16,
                                                            "Playlist error: line %i: 'rule' found outside of any gametype or playlist block\n",
                                                            CurrentParseLine);
                                                        break;
                                                }
                                                goto LABEL_127;
                                            }
                                            if ( I_stricmp(token, "name") )
                                            {
                                                if ( !I_stricmp(token, "icon") )
                                                {
                                                    if ( currentParseBlockType != 4 && currentParseBlockType )
                                                    {
                                                        if ( currentParseBlockType == 2 )
                                                        {
                                                            token = (const char *)Com_ParseOnLine(&p);
                                                            v18 = Playlist_Strdup(token);
                                                            playlists[playlistNum].iconName = v18;
                                                        }
                                                        else if ( currentParseBlockType == 3 )
                                                        {
                                                            token = (const char *)Com_ParseOnLine(&p);
                                                            categories[categoryCount].iconName = Playlist_Strdup(token);
                                                        }
                                                        else
                                                        {
                                                            v19 = Com_GetCurrentParseLine();
                                                            Com_PrintError(
                                                                16,
                                                                "Playlist error: line %i: Stray 'icon' line outside of category/playlist block\n",
                                                                v19);
                                                        }
                                                    }
                                                    goto LABEL_127;
                                                }
                                                if ( I_stricmp(token, "filter") )
                                                {
                                                    if ( I_stricmp(token, "description") )
                                                    {
                                                        if ( I_stricmp(token, "lockdescription") )
                                                        {
                                                            if ( I_stricmp(token, "teambased") )
                                                            {
                                                                if ( !I_stricmp(token, "setdvar") )
                                                                {
                                                                    if ( currentParseBlockType == 5 )
                                                                    {
                                                                        token = (const char *)Com_ParseOnLine(&p);
                                                                        I_strncpyz(key, token, 512);
                                                                        token = (const char *)Com_ParseOnLine(&p);
                                                                        I_strncpyz(value, token, 512);
                                                                        Dvar_SetFromStringByName(key, value);
                                                                    }
                                                                    else
                                                                    {
                                                                        v33 = Com_GetCurrentParseLine();
                                                                        Com_PrintError(
                                                                            16,
                                                                            "Playlist error: line %i: found 'setdvar' flag outside of settings definition.\n",
                                                                            v33);
                                                                    }
                                                                    goto LABEL_127;
                                                                }
                                                                if ( I_stricmp(token, "ranked") )
                                                                {
                                                                    if ( I_stricmp(token, "visible_dvar") )
                                                                    {
                                                                        if ( I_stricmp(token, "collectors") )
                                                                        {
                                                                            if ( I_stricmp(token, "finishedsolo") )
                                                                            {
                                                                                if ( currentParseBlockType == 4 || currentParseBlockType == PARSE_UNKNOWN )
                                                                                    goto LABEL_127;
                                                                                if ( !I_stricmp(token, "unlockxp") )
                                                                                {
                                                                                    if ( currentParseBlockType == 2 )
                                                                                    {
                                                                                        v40 = ParseIntRange("unlockxp", &p, 0x80000000, 0x7FFFFFFF);
                                                                                        playlists[playlistNum].unlockXp = v40;
                                                                                    }
                                                                                    else if ( currentParseBlockType == 3 )
                                                                                    {
                                                                                        categories[categoryCount].unlockXp = ParseIntRange(
                                                                                                                                                                     "unlockxp",
                                                                                                                                                                     &p,
                                                                                                                                                                     0,
                                                                                                                                                                     0x7FFFFFFF);
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        v41 = Com_GetCurrentParseLine();
                                                                                        Com_PrintError(
                                                                                            16,
                                                                                            "Playlist error: line %i: Stray 'unlockxp' line outside of playlist or category block\n",
                                                                                            v41);
                                                                                    }
                                                                                    goto LABEL_127;
                                                                                }
                                                                                if ( !I_stricmp(token, "unlockPLevel") )
                                                                                {
                                                                                    if ( currentParseBlockType == 2 )
                                                                                    {
                                                                                        v42 = ParseIntRange("unlockPLevel", &p, 0x80000000, 0x7FFFFFFF);
                                                                                        playlists[playlistNum].unlockPLevel = v42;
                                                                                    }
                                                                                    else if ( currentParseBlockType == 3 )
                                                                                    {
                                                                                        categories[categoryCount].unlockPLevel = ParseIntRange(
                                                                                                                                                                             "unlockPLevel",
                                                                                                                                                                             &p,
                                                                                                                                                                             0,
                                                                                                                                                                             0x7FFFFFFF);
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        v43 = Com_GetCurrentParseLine();
                                                                                        Com_PrintError(
                                                                                            16,
                                                                                            "Playlist error: line %i: Stray 'unlockPLevel' line outside of playlist or "
                                                                                            "category block\n",
                                                                                            v43);
                                                                                    }
                                                                                    goto LABEL_127;
                                                                                }
                                                                                if ( !I_stricmp(token, "gametypeban") )
                                                                                {
                                                                                    if ( currentParseBlockType == 2 )
                                                                                    {
                                                                                        v44 = ParseIntRange("gametypeban", &p, 0, 0x7FFFFFFF);
                                                                                        playlists[playlistNum].gametypeBan = v44;
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        v45 = Com_GetCurrentParseLine();
                                                                                        Com_PrintError(
                                                                                            16,
                                                                                            "Playlist error: line %i: Stray 'gametypeban' line outside of playlist block\n",
                                                                                            v45);
                                                                                    }
                                                                                    goto LABEL_127;
                                                                                }
                                                                                if ( !I_stricmp(token, "wagerbet") )
                                                                                {
                                                                                    if ( currentParseBlockType == 2 )
                                                                                    {
                                                                                        v46 = ParseIntRange("wagerbet", &p, 0, 0x7FFFFFFF);
                                                                                        playlists[playlistNum].wagerBet = v46;
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        v47 = Com_GetCurrentParseLine();
                                                                                        Com_PrintError(
                                                                                            16,
                                                                                            "Playlist error: line %i: Stray 'wagerbet' line outside of playlist block\n",
                                                                                            v47);
                                                                                    }
                                                                                    goto LABEL_127;
                                                                                }
                                                                                if ( !I_stricmp(token, "wagerentry") )
                                                                                {
                                                                                    if ( currentParseBlockType == 2 )
                                                                                    {
                                                                                        v48 = ParseIntRange("wagerentry", &p, 0, 0x7FFFFFFF);
                                                                                        playlists[playlistNum].wagerEntry = v48;
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        v49 = Com_GetCurrentParseLine();
                                                                                        Com_PrintError(
                                                                                            16,
                                                                                            "Playlist error: line %i: Stray 'wagerentry' line outside of playlist block\n",
                                                                                            v49);
                                                                                    }
                                                                                    goto LABEL_127;
                                                                                }
                                                                                if ( !I_stricmp(token, "maxLocalPlayers") )
                                                                                {
                                                                                    if ( currentParseBlockType == 2 )
                                                                                    {
                                                                                        v50 = ParseIntRange("maxlocalplayers", &p, 0, 0x7FFFFFFF);
                                                                                        playlists[playlistNum].maxLocalPlayers = v50;
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        v51 = Com_GetCurrentParseLine();
                                                                                        Com_PrintError(
                                                                                            16,
                                                                                            "Playlist error: line %i: Stray 'maxlocalplayres' line outside of playlist block\n",
                                                                                            v51);
                                                                                    }
                                                                                    goto LABEL_127;
                                                                                }
                                                                                if ( !I_stricmp(token, "disableGuests") )
                                                                                {
                                                                                    if ( currentParseBlockType == 2 )
                                                                                    {
                                                                                        playlists[playlistNum].disableGuests = 1;
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        v52 = Com_GetCurrentParseLine();
                                                                                        Com_PrintError(
                                                                                            16,
                                                                                            "Playlist error: line %i: Stray 'disableGuests' line outside of playlist block\n",
                                                                                            v52);
                                                                                    }
                                                                                    goto LABEL_127;
                                                                                }
                                                                                if ( !I_stricmp(token, "dlc") )
                                                                                {
                                                                                    if ( currentParseBlockType == 2 )
                                                                                    {
                                                                                        v53 = playlistNum;
                                                                                        v54 = ParseIntRange("dlc", &p, 1, 0x7FFFFFFF);
                                                                                        playlists[playlistNum].requiredDLCMask = playlists[v53].requiredDLCMask
                                                                                                                                                                     | (1 << (v54 + 1));
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        v55 = Com_GetCurrentParseLine();
                                                                                        Com_PrintError(
                                                                                            16,
                                                                                            "Playlist error: line %i: Stray 'requiredDlcPack' line outside of playlist block\n",
                                                                                            v55);
                                                                                    }
                                                                                    goto LABEL_127;
                                                                                }
                                                                                if ( !I_stricmp(token, "maxparty") )
                                                                                {
                                                                                    if ( currentParseBlockType == 2 )
                                                                                    {
                                                                                        v56 = ParseIntRange("maxparty", &p, 1, 32);
                                                                                        playlists[playlistNum].maxPartySize = v56;
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        v57 = Com_GetCurrentParseLine();
                                                                                        Com_PrintError(
                                                                                            16,
                                                                                            "Playlist error: line %i: Stray 'maxparty' line outside of playlist block\n",
                                                                                            v57);
                                                                                    }
                                                                                    goto LABEL_127;
                                                                                }
                                                                                if ( !I_stricmp(token, "minparty") )
                                                                                {
                                                                                    if ( currentParseBlockType == 2 )
                                                                                    {
                                                                                        v58 = ParseIntRange("minparty", &p, 0, 32);
                                                                                        playlists[playlistNum].minPartySize = v58;
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        v59 = Com_GetCurrentParseLine();
                                                                                        Com_PrintError(
                                                                                            16,
                                                                                            "Playlist error: line %i: Stray 'minparty' line outside of playlist block\n",
                                                                                            v59);
                                                                                    }
                                                                                    goto LABEL_127;
                                                                                }
                                                                                if ( I_stricmp(token, "categories") )
                                                                                {
                                                                                    Com_UngetToken();
                                                                                    Com_SetCSV(1);
                                                                                    token = (const char *)Com_ParseOnLine(&p);
                                                                                    if ( token && *p )
                                                                                    {
                                                                                        if ( !*token
                                                                                            && !Assert_MyHandler(
                                                                                                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                                                                                                        1207,
                                                                                                        0,
                                                                                                        "%s",
                                                                                                        "token[0]") )
                                                                                        {
                                                                                            __debugbreak();
                                                                                        }
                                                                                        if ( totalEntries < 0x2000 )
                                                                                        {
                                                                                            entry = &playlistEntries[totalEntries];
                                                                                            v65 = Playlist_Strdup(token);
                                                                                            entry->mapname = v65;
                                                                                            token = (const char *)Com_ParseOnLine(&p);
                                                                                            if ( !token )
                                                                                                goto LABEL_283;
                                                                                            if ( !*p )
                                                                                                goto LABEL_283;
                                                                                            entry->ignore = 0;
                                                                                            v66 = Playlist_Strdup(token);
                                                                                            entry->gametype = v66;
                                                                                            token = (const char *)Com_ParseOnLine(&p);
                                                                                            if ( !token )
                                                                                                goto LABEL_283;
                                                                                            if ( !*p )
                                                                                                goto LABEL_283;
                                                                                            v67 = atoi(token);
                                                                                            entry->fullWeight = v67;
                                                                                            if ( entry->fullWeight <= 0 )
                                                                                                goto LABEL_283;
                                                                                            Com_SkipRestOfLine(&p);
                                                                                            Com_SetCSV(0);
                                                                                            ++playlists[playlistNum].numEntries;
                                                                                            ++totalEntries;
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            v74 = playlistNum;
                                                                                            v64 = Com_GetCurrentParseLine();
                                                                                            Com_PrintError(
                                                                                                16,
                                                                                                "line %i: Exceeded max total entries %d in playlist %i\n",
                                                                                                v64,
                                                                                                0x2000,
                                                                                                v74);
                                                                                            Com_SkipRestOfLine(&p);
                                                                                            Com_SetCSV(0);
                                                                                        }
                                                                                    }
                                                                                    else
                                                                                    {
LABEL_283:
                                                                                        v75 = playlistNum;
                                                                                        v68 = Com_GetCurrentParseLine();
                                                                                        Com_PrintError(16, "line %i: Invalid line in playlist %i\n", v68, v75);
                                                                                        Com_SkipRestOfLine(&p);
                                                                                        Com_SetCSV(0);
                                                                                    }
                                                                                }
                                                                                else
                                                                                {
                                                                                    playlists[playlistNum].categoryCount = 0;
                                                                                    if ( currentParseBlockType == 2 )
                                                                                    {
                                                                                        token = (const char *)Com_ParseOnLine(&p);
                                                                                        if ( token && *p )
                                                                                        {
                                                                                            while ( *token && *p && playlists[playlistNum].categoryCount < 15 )
                                                                                            {
                                                                                                v61 = Playlist_Strdup(token);
                                                                                                playlists[playlistNum].category[playlists[playlistNum].categoryCount++] = v61;
                                                                                                currentCategory = Playlist_GetCategoryIdByName(token);
                                                                                                if ( currentCategory >= 0 )
                                                                                                {
                                                                                                    ++categories[currentCategory].playlistCount;
                                                                                                    token = (const char *)Com_ParseOnLine(&p);
                                                                                                }
                                                                                                else
                                                                                                {
                                                                                                    v73 = token;
                                                                                                    v62 = Com_GetCurrentParseLine();
                                                                                                    Com_Printf(
                                                                                                        16,
                                                                                                        "Playlist error: line %i: There is no existing category named %s.\n",
                                                                                                        v62,
                                                                                                        v73);
                                                                                                    Com_SkipRestOfLine(&p);
                                                                                                }
                                                                                            }
                                                                                            goto LABEL_127;
                                                                                        }
                                                                                        v60 = Com_GetCurrentParseLine();
                                                                                        Com_Printf(
                                                                                            16,
                                                                                            "Playlist error: line %i: Expected at least one category name after category command\n",
                                                                                            v60);
                                                                                        Com_SkipRestOfLine(&p);
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        v63 = Com_GetCurrentParseLine();
                                                                                        Com_PrintError(
                                                                                            16,
                                                                                            "Playlist error: line %i: Stray 'category' line outside of playlist block\n",
                                                                                            v63);
                                                                                        Com_SkipRestOfLine(&p);
                                                                                    }
                                                                                }
                                                                            }
                                                                            else
                                                                            {
                                                                                Com_SkipRestOfLine(&p);
                                                                                if ( currentParseBlockType == 2 )
                                                                                {
                                                                                    playlists[playlistNum].finishedSoloRequired = 1;
                                                                                }
                                                                                else if ( currentParseBlockType == 3 )
                                                                                {
                                                                                    categories[categoryCount].finishedSoloRequired = 1;
                                                                                }
                                                                                else
                                                                                {
                                                                                    v39 = Com_GetCurrentParseLine();
                                                                                    Com_PrintError(
                                                                                        16,
                                                                                        "Playlist error: line %i: found 'finishedsolo' flag outside of playlist or ca"
                                                                                        "tegory definition.\n",
                                                                                        v39);
                                                                                }
                                                                            }
                                                                        }
                                                                        else
                                                                        {
                                                                            Com_SkipRestOfLine(&p);
                                                                            if ( currentParseBlockType == 2 )
                                                                            {
                                                                                playlists[playlistNum].collectorsEditionRequired = 1;
                                                                            }
                                                                            else if ( currentParseBlockType == 3 )
                                                                            {
                                                                                categories[categoryCount].collectorsEditionRequired = 1;
                                                                            }
                                                                            else
                                                                            {
                                                                                v38 = Com_GetCurrentParseLine();
                                                                                Com_PrintError(
                                                                                    16,
                                                                                    "Playlist error: line %i: found 'collectors' flag outside of playlist or catego"
                                                                                    "ry definition.\n",
                                                                                    v38);
                                                                            }
                                                                        }
                                                                    }
                                                                    else
                                                                    {
                                                                        token = (const char *)Com_ParseOnLine(&p);
                                                                        I_strncpyz(dest, token, 512);
                                                                        Com_SkipRestOfLine(&p);
                                                                        if ( currentParseBlockType == 2 )
                                                                        {
                                                                            playlists[playlistNum].visible_dvar = 1;
                                                                            Var = Dvar_FindVar(dest);
                                                                            playlists[playlistNum].visDvar = Var;
                                                                            if ( !playlists[playlistNum].visDvar )
                                                                                goto LABEL_190;
                                                                        }
                                                                        else if ( currentParseBlockType == 3 )
                                                                        {
                                                                            categories[categoryCount].visible_dvar = 1;
                                                                            categories[categoryCount].visDvar = Dvar_FindVar(dest);
                                                                            if ( !categories[categoryCount].visDvar )
                                                                            {
LABEL_190:
                                                                                v36 = Com_GetCurrentParseLine();
                                                                                Com_PrintError(
                                                                                    16,
                                                                                    "Playlist error: line %i: found 'visible_dvar' but no matching DVAR.\n",
                                                                                    v36);
                                                                            }
                                                                        }
                                                                        else
                                                                        {
                                                                            v37 = Com_GetCurrentParseLine();
                                                                            Com_PrintError(
                                                                                16,
                                                                                "Playlist error: line %i: found 'visible_dvar' flag outside of playlist or catego"
                                                                                "ry definition.\n",
                                                                                v37);
                                                                        }
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    Com_SkipRestOfLine(&p);
                                                                    if ( currentParseBlockType == 2 )
                                                                    {
                                                                        playlists[playlistNum].ranked = 1;
                                                                    }
                                                                    else
                                                                    {
                                                                        v34 = Com_GetCurrentParseLine();
                                                                        Com_PrintError(
                                                                            16,
                                                                            "Playlist error: line %i: found 'ranked' flag outside of playlist definition.\n",
                                                                            v34);
                                                                    }
                                                                }
                                                            }
                                                            else
                                                            {
                                                                Com_SkipRestOfLine(&p);
                                                                if ( currentParseBlockType == PARSE_CONTRACT )
                                                                {
                                                                    gametypes[numGametypes].teamBased = 1;
                                                                }
                                                                else
                                                                {
                                                                    v32 = Com_GetCurrentParseLine();
                                                                    Com_PrintError(
                                                                        16,
                                                                        "Playlist error: line %i: found 'teambased' flag outside of gametype definition.\n",
                                                                        v32);
                                                                }
                                                            }
                                                        }
                                                        else
                                                        {
                                                            if ( currentParseBlockType == 4 || currentParseBlockType == PARSE_UNKNOWN )
                                                                goto LABEL_127;
                                                            token = (const char *)Com_ParseOnLine(&p);
                                                            if ( token && *p )
                                                            {
                                                                CurrentLanguage = SEH_GetCurrentLanguage();
                                                                LanguageName = SEH_GetLanguageName(CurrentLanguage);
                                                                if ( I_stricmp(token, LanguageName) )
                                                                {
                                                                    if ( foundLocalizedDesc || I_stricmp("english", token) )
                                                                        goto LABEL_127;
                                                                    token = (const char *)Com_ParseOnLine(&p);
                                                                    if ( token && *p )
                                                                    {
                                                                        if ( currentParseBlockType == 2 )
                                                                        {
LABEL_165:
                                                                            v30 = Playlist_Strdup_Newline(token);
                                                                            playlists[playlistNum].lockdescription = v30;
                                                                            goto LABEL_127;
                                                                        }
                                                                        goto LABEL_173;
                                                                    }
                                                                    v31 = Com_GetCurrentParseLine();
                                                                    Com_Printf(
                                                                        16,
                                                                        "Playlist error: line %i: Expected playlist lockdescription after lockdescription <la"
                                                                        "nguage> command\n",
                                                                        v31);
                                                                    Com_SkipRestOfLine(&p);
                                                                }
                                                                else
                                                                {
                                                                    token = (const char *)Com_ParseOnLine(&p);
                                                                    if ( token && *p )
                                                                    {
                                                                        foundLocalizedDesc = 1;
                                                                        if ( currentParseBlockType == 2 )
                                                                            goto LABEL_165;
LABEL_173:
                                                                        categories[categoryCount].lockdescription = Playlist_Strdup_Newline(token);
                                                                        goto LABEL_127;
                                                                    }
                                                                    v29 = Com_GetCurrentParseLine();
                                                                    Com_Printf(
                                                                        16,
                                                                        "Playlist error: line %i: Expected playlist lockdescription after description <language> command\n",
                                                                        v29);
                                                                    Com_SkipRestOfLine(&p);
                                                                }
                                                            }
                                                            else
                                                            {
                                                                v26 = Com_GetCurrentParseLine();
                                                                Com_Printf(
                                                                    16,
                                                                    "Playlist error: line %i: Expected language name after lockdescription command\n",
                                                                    v26);
                                                                Com_SkipRestOfLine(&p);
                                                            }
                                                        }
                                                    }
                                                    else
                                                    {
                                                        if ( currentParseBlockType == 4 || currentParseBlockType == PARSE_UNKNOWN )
                                                            goto LABEL_127;
                                                        token = (const char *)Com_ParseOnLine(&p);
                                                        if ( token && *p )
                                                        {
                                                            v22 = SEH_GetCurrentLanguage();
                                                            v23 = SEH_GetLanguageName(v22);
                                                            if ( I_stricmp(token, v23) )
                                                            {
                                                                if ( foundLocalizedDesc || I_stricmp("english", token) )
                                                                    goto LABEL_127;
                                                                token = (const char *)Com_ParseOnLine(&p);
                                                                if ( token && *p )
                                                                {
                                                                    if ( currentParseBlockType == 2 )
                                                                    {
LABEL_143:
                                                                        v25 = Playlist_Strdup_Newline(token);
                                                                        playlists[playlistNum].description = v25;
                                                                        goto LABEL_127;
                                                                    }
                                                                    goto LABEL_151;
                                                                }
                                                            }
                                                            else
                                                            {
                                                                token = (const char *)Com_ParseOnLine(&p);
                                                                if ( token && *p )
                                                                {
                                                                    foundLocalizedDesc = 1;
                                                                    if ( currentParseBlockType == 2 )
                                                                        goto LABEL_143;
LABEL_151:
                                                                    categories[categoryCount].description = Playlist_Strdup_Newline(token);
                                                                    goto LABEL_127;
                                                                }
                                                            }
                                                            v24 = Com_GetCurrentParseLine();
                                                            Com_Printf(
                                                                16,
                                                                "Playlist error: line %i: Expected playlist description after description <language> command\n",
                                                                v24);
                                                            Com_SkipRestOfLine(&p);
                                                        }
                                                        else
                                                        {
                                                            v21 = Com_GetCurrentParseLine();
                                                            Com_Printf(
                                                                16,
                                                                "Playlist error: line %i: Expected language name after description command\n",
                                                                v21);
                                                            Com_SkipRestOfLine(&p);
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    if ( currentParseBlockType == 4 || currentParseBlockType == PARSE_UNKNOWN )
                                                        goto LABEL_127;
                                                    if ( currentParseBlockType == 3 )
                                                    {
                                                        token = Com_ParseRestOfLine(&p);
                                                        categories[categoryCount].filter = Playlist_Strdup(token);
                                                    }
                                                    else
                                                    {
                                                        v20 = Com_GetCurrentParseLine();
                                                        Com_Printf(16, "Playlist error: line %i: filter keyword outside of category block\n", v20);
                                                        Com_SkipRestOfLine(&p);
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                token = (const char *)Com_ParseOnLine(&p);
                                                if ( token && *p )
                                                {
                                                    v13 = SEH_GetCurrentLanguage();
                                                    v14 = SEH_GetLanguageName(v13);
                                                    if ( I_stricmp(token, v14) )
                                                    {
                                                        if ( foundLocalizedName || I_stricmp("english", token) )
                                                            goto LABEL_127;
                                                        token = (const char *)Com_ParseOnLine(&p);
                                                        if ( token && *p )
                                                        {
                                                            switch ( currentParseBlockType )
                                                            {
                                                                case 1:
LABEL_97:
                                                                    gametypes[numGametypes].localizedName = Playlist_Strdup(token);
                                                                    break;
                                                                case 2:
LABEL_96:
                                                                    v16 = Playlist_Strdup(token);
                                                                    playlists[playlistNum].name = v16;
                                                                    break;
                                                                case 3:
                                                                    categories[categoryCount].localizedName = Playlist_Strdup(token);
                                                                    break;
                                                            }
LABEL_127:
                                                            Com_SkipRestOfLine(&p);
                                                        }
                                                        else
                                                        {
                                                            if ( currentParseBlockType == 2 )
                                                                v76 = "playlist";
                                                            else
                                                                v76 = "gametype";
                                                            v17 = Com_GetCurrentParseLine();
                                                            Com_Printf(16, "Playlist error: line %i: Expected %s name after name command\n", v17, v76);
                                                            Com_SkipRestOfLine(&p);
                                                        }
                                                    }
                                                    else
                                                    {
                                                        token = (const char *)Com_ParseOnLine(&p);
                                                        if ( token && *p )
                                                        {
                                                            foundLocalizedName = 1;
                                                            switch ( currentParseBlockType )
                                                            {
                                                                case 1:
                                                                    goto LABEL_97;
                                                                case 2:
                                                                    goto LABEL_96;
                                                                case 3:
                                                                    categories[categoryCount].localizedName = Playlist_Strdup(token);
                                                                    break;
                                                            }
                                                            goto LABEL_127;
                                                        }
                                                        if ( currentParseBlockType == 2 )
                                                            v77 = "playlist";
                                                        else
                                                            v77 = "gametype";
                                                        v15 = Com_GetCurrentParseLine();
                                                        Com_Printf(16, "Playlist error: line %i: Expected %s name after name command\n", v15, v77);
                                                        Com_SkipRestOfLine(&p);
                                                    }
                                                }
                                                else
                                                {
                                                    Com_Printf(16, "Playlist error: Expected language name after name command\n");
                                                    Com_SkipRestOfLine(&p);
                                                }
                                            }
                                        }
                                        else if ( currentParseBlockType == PARSE_CONTRACT )
                                        {
                                            token = (const char *)Com_ParseOnLine(&p);
                                            if ( token && *p )
                                            {
                                                gametypes[numGametypes].scriptName = Playlist_Strdup(token);
                                                Com_SkipRestOfLine(&p);
                                            }
                                            else
                                            {
                                                v11 = Com_GetCurrentParseLine();
                                                Com_Printf(16, "Playlist error: line %i: Expected script name after script command\n", v11);
                                                Com_SkipRestOfLine(&p);
                                            }
                                        }
                                        else
                                        {
                                            v10 = Com_GetCurrentParseLine();
                                            Com_Printf(16, "Playlist error: line %i: Stray 'script' line outside of gametype block\n", v10);
                                            Com_SkipRestOfLine(&p);
                                        }
                                    }
                                    else
                                    {
                                        token = (const char *)Com_ParseOnLine(&p);
                                        if ( token && *p )
                                        {
LABEL_45:
                                            currentParseBlockType = 4;
                                            Com_SkipRestOfLine(&p);
                                        }
                                        else
                                        {
LABEL_49:
                                            v8 = Com_GetCurrentParseLine();
                                            Com_Printf(16, "Playlist error: line %i: Expected playlist number after playlist command\n", v8);
                                            Com_SkipRestOfLine(&p);
                                        }
                                    }
                                }
                                else
                                {
                                    token = (const char *)Com_ParseOnLine(&p);
                                    if ( !token || !*p )
                                        goto LABEL_49;
                                    currentParseBlockType = 2;
                                    playlistNum = atoi(token);
                                    if ( (unsigned int)playlistNum < 0x40 )
                                    {
                                        foundLocalizedName = 0;
                                        foundLocalizedDesc = 0;
                                        playlists[playlistNum].rules = Playlist_BeginRules();
                                        playlists[playlistNum].name = "";
                                        playlists[playlistNum].description = "";
                                        playlists[playlistNum].lockdescription = "";
                                        playlists[playlistNum].iconName = "";
                                        playlists[playlistNum].baseEntry = totalEntries;
                                        playlists[playlistNum].numEntries = 0;
                                        playlists[playlistNum].ranked = 0;
                                        playlists[playlistNum].unlockXp = 0;
                                        playlists[playlistNum].unlockPLevel = 0;
                                        playlists[playlistNum].gametypeBan = 0;
                                        playlists[playlistNum].maxPartySize = 1;
                                        playlists[playlistNum].minPartySize = 0;
                                        playlists[playlistNum].categoryCount = 0;
                                        playlists[playlistNum].maxLocalPlayers = 1;
                                        playlists[playlistNum].disableGuests = 0;
                                        for ( index = 0; index < 16; ++index )
                                            playlists[playlistNum].category[index] = "";
                                        goto LABEL_127;
                                    }
                                    v72 = playlistNum;
                                    v9 = Com_GetCurrentParseLine();
                                    Com_Printf(
                                        16,
                                        "Playlist error: line %i: Invalid playlist number %i after playlist command\n",
                                        v9,
                                        v72);
                                    Com_SkipRestOfLine(&p);
                                }
                            }
                            else
                            {
                                token = (const char *)Com_ParseOnLine(&p);
                                if ( token && *p )
                                    goto LABEL_45;
LABEL_35:
                                v7 = Com_GetCurrentParseLine();
                                Com_Printf(16, "Playlist error: line %i: Expected category name after category command\n", v7);
                                Com_SkipRestOfLine(&p);
                            }
                        }
                        else
                        {
                            token = (const char *)Com_ParseOnLine(&p);
                            if ( !token || !*p )
                                goto LABEL_35;
                            if ( (unsigned int)++categoryCount >= 0x20
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                                            464,
                                            0,
                                            "categoryCount doesn't index MAX_CATEGORIES\n\t%i not in [0, %i)",
                                            categoryCount,
                                            32) )
                            {
                                __debugbreak();
                            }
                            currentParseBlockType = 3;
                            categories[categoryCount].internalName = Playlist_Strdup(token);
                            categories[categoryCount].rules = Playlist_BeginRules();
                            categories[categoryCount].filter = "";
                            categories[categoryCount].localizedName = "";
                            categories[categoryCount].description = "";
                            categories[categoryCount].lockdescription = "";
                            categories[categoryCount].iconName = "";
                            categories[categoryCount].playlistCount = 0;
                            Com_SkipRestOfLine(&p);
                        }
                    }
                    else
                    {
                        token = (const char *)Com_ParseOnLine(&p);
                        if ( token && *p )
                        {
                            if ( numGametypes < 63 )
                            {
                                if ( numGametypes >= 0 )
                                {
                                    if ( !*gametypes[numGametypes].scriptName )
                                    {
                                        localizedName = gametypes[numGametypes].localizedName;
                                        v5 = Com_GetCurrentParseLine();
                                        Com_PrintError(
                                            16,
                                            "Playlist error: line %i: gametype %s has no script definition\n",
                                            v5,
                                            localizedName);
                                    }
                                    if ( !*gametypes[numGametypes].localizedName )
                                    {
                                        internalName = gametypes[numGametypes].internalName;
                                        v6 = Com_GetCurrentParseLine();
                                        Com_PrintError(16, "Playlist error: line %i: gametype %s has no localized name\n", v6, internalName);
                                    }
                                }
                                if ( (unsigned int)++numGametypes >= 0x40
                                    && !Assert_MyHandler(
                                                "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                                                426,
                                                0,
                                                "numGametypes doesn't index MAX_PLAYLIST_GAMETYPES\n\t%i not in [0, %i)",
                                                numGametypes,
                                                64) )
                                {
                                    __debugbreak();
                                }
                                currentParseBlockType = PARSE_CONTRACT;
                                foundLocalizedName = 0;
                                gametypes[numGametypes].internalName = Playlist_Strdup(token);
                                gametypes[numGametypes].rules = Playlist_BeginRules();
                                gametypes[numGametypes].scriptName = "";
                                gametypes[numGametypes].localizedName = "";
                                gametypes[numGametypes].teamBased = 0;
                                Com_SkipRestOfLine(&p);
                            }
                            else
                            {
                                v4 = Com_GetCurrentParseLine();
                                Com_Printf(16, "Playlist error: line %i: More than %i gametypes specified in playlist\n", v4, 64);
                                Com_SkipRestOfLine(&p);
                            }
                        }
                        else
                        {
                            v3 = Com_GetCurrentParseLine();
                            Com_Printf(16, "Playlist error: line %i: Expected gametype name after gametype command\n", v3);
                            Com_SkipRestOfLine(&p);
                        }
                    }
                }
                else
                {
                    currentParseBlockType = 5;
                    Com_SkipRestOfLine(&p);
                }
            }
            else
            {
                token = (const char *)Com_ParseOnLine(&p);
                if ( token && *p )
                {
                    playlist_versionNum = atoi(token);
                    if ( playlist_versionNum >= 0 )
                        goto LABEL_127;
                    v69 = playlist_versionNum;
                    v2 = Com_GetCurrentParseLine();
                    Com_Printf(16, "Playlist error: line %i: Invalid version number %i after version command\n", v2, v69);
                    Com_SkipRestOfLine(&p);
                }
                else
                {
                    v1 = Com_GetCurrentParseLine();
                    Com_Printf(16, "Playlist error: line %i: Expected version number after version command\n", v1);
                    Com_SkipRestOfLine(&p);
                }
            }
        }
    }
    Com_EndParseSession();
}

char *__cdecl Playlist_Strdup(const char *src)
{
    char v2; // [esp+3h] [ebp-29h]
    char *v3; // [esp+8h] [ebp-24h]
    const char *v4; // [esp+Ch] [ebp-20h]
    int used; // [esp+20h] [ebp-Ch]
    char *newString; // [esp+28h] [ebp-4h]

    used = s_playlistStringBufferUsed + strlen(src) + 1;
    newString = (char *)&s_playlistStringBuffer[s_playlistStringBufferUsed];
    if ( used > 0x8000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    172,
                    0,
                    "%s",
                    "used <= PLAYLIST_STRING_BUFFER_SIZE") )
    {
        __debugbreak();
    }
    if ( used <= 0x8000 )
    {
        v4 = src;
        v3 = newString;
        do
        {
            v2 = *v4;
            *v3++ = *v4++;
        }
        while ( v2 );
        s_playlistStringBufferUsed = used;
        return newString;
    }
    else
    {
        Com_PrintError(16, "ERROR: Playlist string '%s' overflows playlist string buffer\n", src);
        return (char *)"";
    }
}

char *__cdecl Playlist_Strdup_Newline(const char *src)
{
    int used; // [esp+10h] [ebp-10h]
    char *dest; // [esp+18h] [ebp-8h]
    char *newString; // [esp+1Ch] [ebp-4h]

    used = s_playlistStringBufferUsed + strlen(src) + 1;
    newString = (char *)&s_playlistStringBuffer[s_playlistStringBufferUsed];
    if ( used > 0x8000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    192,
                    0,
                    "%s",
                    "used <= PLAYLIST_STRING_BUFFER_SIZE") )
    {
        __debugbreak();
    }
    if ( used <= 0x8000 )
    {
        dest = newString;
        while ( *src )
        {
            if ( *src == 92 && src[1] == 110 )
            {
                *dest = 10;
                src += 2;
                ++dest;
            }
            else
            {
                *dest++ = *src++;
            }
        }
        *dest = 0;
        s_playlistStringBufferUsed = used;
        return newString;
    }
    else
    {
        Com_PrintError(16, "ERROR: Playlist string '%s' overflows playlist string buffer\n", src);
        return (char *)"";
    }
}

char *__cdecl Playlist_BeginRules()
{
    if ( s_playlistRuleBufferUsed < 0x8000 )
    {
        s_playlistRuleBufferBase = ++s_playlistRuleBufferUsed;
        s_playlistRuleBuffer[s_playlistRuleBufferUsed] = 0;
        return &s_playlistRuleBuffer[s_playlistRuleBufferBase];
    }
    else
    {
        Com_PrintError(16, "ERROR: New playlist rule string overflows playlist rule string buffer\n");
        return (char *)"";
    }
}

char __cdecl Playlist_AppendRules(const char *base, const char *token)
{
    char v3; // cl
    char *v4; // [esp+8h] [ebp-1Ch]
    const char *v5; // [esp+Ch] [ebp-18h]
    int len; // [esp+20h] [ebp-4h]

    len = strlen(token);
    if ( &s_playlistRuleBuffer[s_playlistRuleBufferBase] == base )
    {
        if ( s_playlistRuleBufferUsed + len + 2 <= 0x8000 )
        {
            if ( s_playlistRuleBuffer[s_playlistRuleBufferUsed]
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                            254,
                            0,
                            "%s",
                            "s_playlistRuleBuffer[ s_playlistRuleBufferUsed ] == '\\0'") )
            {
                __debugbreak();
            }
            v5 = token;
            v4 = &s_playlistRuleBuffer[s_playlistRuleBufferUsed];
            do
            {
                v3 = *v5;
                *v4++ = *v5++;
            }
            while ( v3 );
            s_playlistRuleBufferUsed += len;
            if ( s_playlistRuleBuffer[s_playlistRuleBufferUsed]
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                            259,
                            0,
                            "%s",
                            "s_playlistRuleBuffer[ s_playlistRuleBufferUsed ] == '\\0'") )
            {
                __debugbreak();
            }
            s_playlistRuleBuffer[s_playlistRuleBufferUsed] = 59;
            *(_BYTE *)(s_playlistRuleBufferUsed + 160499321) = 0;
            if ( ++s_playlistRuleBufferUsed >= 0x8000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                            264,
                            0,
                            "%s",
                            "s_playlistRuleBufferUsed < PLAYLIST_RULE_BUFFER_SIZE") )
            {
                __debugbreak();
            }
            return 1;
        }
        else
        {
            Com_PrintError(16, "ERROR: Playlist rule string '%s' overflows playlist rule string buffer\n", token);
            return 0;
        }
    }
    else
    {
        Com_PrintError(16, "ERROR: rule buffer de-sync for new rule '%s'\n", token);
        return 0;
    }
}

int __cdecl ParseIntRange(const char *field, const char **pData, int min, int max)
{
    int v4; // eax
    int v6; // eax
    int CurrentParseLine; // eax
    parseInfo_t *token; // [esp+0h] [ebp-8h]
    int value; // [esp+4h] [ebp-4h]

    if ( !field && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp", 302, 0, "%s", "field") )
        __debugbreak();
    if ( !pData && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp", 303, 0, "%s", "pData") )
        __debugbreak();
    token = Com_ParseOnLine(pData);
    if ( token->token[0] )
    {
        value = atoi(token->token);
        if ( value >= min )
        {
            if ( value > max )
            {
                CurrentParseLine = Com_GetCurrentParseLine();
                Com_PrintError(
                    16,
                    "Playlist error: line %i: '%s' number (%i) must be <= %i, \n",
                    CurrentParseLine,
                    field,
                    value,
                    max);
                return max;
            }
        }
        else
        {
            v6 = Com_GetCurrentParseLine();
            Com_PrintError(16, "Playlist error: line %i: '%s' number (%i) must be >= %i, \n", v6, field, value, min);
            return min;
        }
        return value;
    }
    else
    {
        v4 = Com_GetCurrentParseLine();
        Com_PrintError(16, "Playlist error: line %i: Expected '%s' number after '%s' command\n", v4, field, field);
        return min;
    }
}

char __cdecl Playlist_HasCategory(int playlistId, const char *category)
{
    int index; // [esp+0h] [ebp-4h]

    for ( index = 0; index < playlists[playlistId].categoryCount; ++index )
    {
        if ( !I_strcmp(playlists[playlistId].category[index], category) )
            return 1;
    }
    return 0;
}

int __cdecl Playlist_GetPlaylistCount()
{
    int num; // [esp+0h] [ebp-8h]
    int count; // [esp+4h] [ebp-4h]

    count = 0;
    for ( num = 0; num < 64; ++num )
    {
        if ( playlists[num].numEntries )
            ++count;
    }
    return count;
}

int __cdecl Playlist_GetCategoryCount()
{
    int index; // [esp+0h] [ebp-8h]
    int count; // [esp+4h] [ebp-4h]

    count = 0;
    for ( index = 0; index < 32; ++index )
    {
        if ( categories[index].playlistCount )
        {
            if ( categoryFilter[index] )
                ++count;
        }
    }
    return count;
}

int __cdecl Playlist_GetPlaylistCount(int category)
{
    int num; // [esp+0h] [ebp-8h]
    int count; // [esp+4h] [ebp-4h]

    count = 0;
    for ( num = 0; num < 64; ++num )
    {
        if ( playlists[num].numEntries )
        {
            if ( Playlist_HasCategory(num, categories[category].internalName) )
                ++count;
        }
    }
    return count;
}

void __cdecl Playlist_ValidatePlaylistNum()
{
    int controllerIndex; // [esp+0h] [ebp-8h]
    int num; // [esp+4h] [ebp-4h]

    if ( !LiveStorage_DoWeHavePlaylists()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    1323,
                    0,
                    "%s",
                    "LiveStorage_DoWeHavePlaylists()") )
    {
        __debugbreak();
    }
    num = Playlist_GetPlaylistCount();
    for ( controllerIndex = 0; controllerIndex < 1; ++controllerIndex )
    {
        if ( num <= GamerProfile_GetPlaylistNum(controllerIndex) || GamerProfile_GetPlaylistNum(controllerIndex) < 0 )
            GamerProfile_SetPlaylistNum(controllerIndex, 0);
    }
}

int __cdecl Playlist_GetPlaylistIdForNum(int controllerIndex, int num)
{
    if ( LiveStorage_DoWeHavePlaylists() )
    {
        if ( num < 64 && playlists[num].numEntries )
        {
            return num;
        }
        else
        {
            Com_Printf(16, "Warning: Invalid playlist ID found, returning first playlist.\n");
            return 1;
        }
    }
    else
    {
        UI_OpenMenu(0, "popup_gettingdata");
        return 0;
    }
}

int __cdecl Playlist_GetPlaylistIdForNum(int controllerIndex, unsigned int categoryPlaylistNum, int categoryNum)
{
    int index; // [esp+4h] [ebp-Ch]
    unsigned int categoryId; // [esp+8h] [ebp-8h]
    int count; // [esp+Ch] [ebp-4h]

    count = 0;
    categoryId = Playlist_GetCategoryIdForNum(categoryNum);
    if ( categoryPlaylistNum >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    1371,
                    0,
                    "%s\n\t(categoryPlaylistNum) = %i",
                    "(categoryPlaylistNum >= 0 && categoryPlaylistNum < 64)",
                    categoryPlaylistNum) )
    {
        __debugbreak();
    }
    if ( categoryId >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    1372,
                    0,
                    "%s\n\t(categoryId) = %i",
                    "(categoryId >= 0 && categoryId < 32)",
                    categoryId) )
    {
        __debugbreak();
    }
    if ( LiveStorage_DoWeHavePlaylists() )
    {
        for ( index = 0; index < 64; ++index )
        {
            if ( playlists[index].numEntries && Playlist_HasCategory(index, categories[categoryId].internalName) )
            {
                if ( count == categoryPlaylistNum )
                    return index;
                ++count;
            }
        }
        Com_Printf(16, "Warning: Invalid playlist ID found, returning first playlist.\n");
        return 1;
    }
    else
    {
        UI_OpenMenu(0, "popup_gettingdata");
        return 0;
    }
}

int __cdecl Playlist_GetPlaylistNumFromId(int controllerIndex, int id)
{
    int index; // [esp+4h] [ebp-8h]
    int count; // [esp+8h] [ebp-4h]

    count = 0;
    if ( LiveStorage_DoWeHavePlaylists() )
    {
        for ( index = 0; index < 64; ++index )
        {
            if ( playlists[index].numEntries )
            {
                if ( index == id )
                    return count;
                ++count;
            }
        }
        Com_Printf(16, "Warning: Invalid playlist num found, returning first playlist.\n");
        return 0;
    }
    else
    {
        UI_OpenMenu(0, "popup_gettingdata");
        return 0;
    }
}

int __cdecl Playlist_GetCategoryIdForNum(int num)
{
    int index; // [esp+0h] [ebp-8h]
    int count; // [esp+4h] [ebp-4h]

    count = 0;
    for ( index = 0; index < 32; ++index )
    {
        if ( categories[index].playlistCount && categoryFilter[index] )
        {
            if ( count == num )
                return index;
            ++count;
        }
    }
    Com_Printf(16, "Warning: Invalid category ID found, returning first category.\n");
    return 0;
}

int __cdecl Playlist_GetCategoryIdForPlaylistId(int num)
{
    int index; // [esp+0h] [ebp-4h]

    for ( index = 0; index < 32; ++index )
    {
        if ( categories[index].playlistCount && !I_strcmp(categories[index].internalName, playlists[num].category[0]) )
            return index;
    }
    Com_Printf(16, "Warning: Invalid category for playlist %d , returning first category.\n", num);
    return 0;
}

int __cdecl Playlist_GetNumEntries(unsigned int playlistId)
{
    if ( playlistId >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    1454,
                    0,
                    "playlistId doesn't index MAX_PLAYLISTS\n\t%i not in [0, %i)",
                    playlistId,
                    64) )
    {
        __debugbreak();
    }
    if ( playlistId < 0x40 )
        return playlists[playlistId].numEntries;
    else
        return 0;
}

playlistEntry *__cdecl Playlist_GetEntry(unsigned int playlistId, int entry)
{
    int numEntries; // [esp+0h] [ebp-4h]

    numEntries = Playlist_GetNumEntries(playlistId);
    if ( entry >= (unsigned int)numEntries
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    1465,
                    0,
                    "entry doesn't index numEntries\n\t%i not in [0, %i)",
                    entry,
                    numEntries) )
    {
        __debugbreak();
    }
    if ( numEntries <= 0 || entry < 0 || entry >= numEntries )
        return &nullPlaylistEntry;
    else
        return &playlistEntries[entry + playlists[playlistId].baseEntry];
}

void __cdecl Playlist_ResetCategoryFilter()
{
    *(unsigned int *)categoryFilter = 0;
    *(unsigned int *)&categoryFilter[4] = 0;
    *(unsigned int *)&categoryFilter[8] = 0;
    *(unsigned int *)&categoryFilter[12] = 0;
    *(unsigned int *)&categoryFilter[16] = 0;
    *(unsigned int *)&categoryFilter[20] = 0;
    *(unsigned int *)&categoryFilter[24] = 0;
    *(unsigned int *)&categoryFilter[28] = 0;
}

void __cdecl Playlist_EnableCategoryInFilter(const char *filterName)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 32; ++i )
    {
        if ( !categories[i].filter || !*categories[i].filter || I_stristr(categories[i].filter, filterName) )
            categoryFilter[i] = 1;
    }
}

const char *__cdecl Playlist_GetCategoryLocalizedName(int controllerIndex, unsigned int categoryId)
{
    if ( categoryId >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    1493,
                    0,
                    "%s\n\t(categoryId) = %i",
                    "(categoryId >= 0 && categoryId < 32)",
                    categoryId) )
    {
        __debugbreak();
    }
    if ( LiveStorage_DoWeHavePlaylists() )
    {
        if ( Playlist_CategoryIsLocked(controllerIndex, categoryId, 0, 0) == 8 )
        {
            return UI_SafeTranslateString("MPUI_PLAYLIST_FINISHEDSOLO_CAPS");
        }
        else if ( Playlist_CategoryIsLocked(controllerIndex, categoryId, 0, 0) == 12 )
        {
            return UI_SafeTranslateString("UIPLS_PLAYLIST_NOTVISIBLE_CAPS");
        }
        else
        {
            return categories[categoryId].localizedName;
        }
    }
    else
    {
        UI_OpenMenu(0, "popup_gettingdata");
        return 0;
    }
}

const char *__cdecl Playlist_GetPlaylistName(int controllerIndex, unsigned int playlistId)
{
    if ( playlistId >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    1512,
                    0,
                    "%s\n\t(playlistId) = %i",
                    "(playlistId >= 0 && playlistId < 64)",
                    playlistId) )
    {
        __debugbreak();
    }
    if ( LiveStorage_DoWeHavePlaylists() )
    {
        if ( Playlist_IsLocked(controllerIndex, playlistId, 0, 0, 0, 0) == 8 )
        {
            return UI_SafeTranslateString("MPUI_PLAYLIST_CATEGORY_FINISHEDSOLO");
        }
        else if ( Playlist_IsLocked(controllerIndex, playlistId, 0, 0, 0, 0) == 12 )
        {
            return UI_SafeTranslateString("UIPLS_PLAYLIST_CATEGORY_NOTVISIBLE");
        }
        else
        {
            return playlists[playlistId].name;
        }
    }
    else
    {
        UI_OpenMenu(0, "popup_gettingdata");
        return 0;
    }
}

const char *__cdecl Playlist_GetPlaylistDescription(int controllerIndex, unsigned int playlistId)
{
    char *v3; // eax
    char *v4; // eax
    int wagerBet; // [esp-4h] [ebp-8h]
    char *v6; // [esp-4h] [ebp-8h]

    if ( playlistId >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    1544,
                    0,
                    "%s\n\t(playlistId) = %i",
                    "(playlistId >= 0 && playlistId < 64)",
                    playlistId) )
    {
        __debugbreak();
    }
    if ( LiveStorage_DoWeHavePlaylists() )
    {
        if ( playlists[playlistId].wagerBet )
        {
            wagerBet = playlists[playlistId].wagerBet;
            v3 = UI_SafeTranslateString("MENU_POINTS");
            v6 = UI_ReplaceConversionInt(v3, wagerBet);
            v4 = UI_SafeTranslateString("MPUI_WAGERMATCH_BUY_IN");
            return va("%s\n%s %s", playlists[playlistId].description, v4, v6);
        }
        else
        {
            return playlists[playlistId].description;
        }
    }
    else
    {
        UI_OpenMenu(0, "popup_gettingdata");
        return "";
    }
}

const char *__cdecl Playlist_GetPlaylistIconName(int controllerIndex, unsigned int playlistId)
{
    if ( playlistId >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    1600,
                    0,
                    "%s\n\t(playlistId) = %i",
                    "(playlistId >= 0 && playlistId < 64)",
                    playlistId) )
    {
        __debugbreak();
    }
    if ( LiveStorage_DoWeHavePlaylists() )
        return playlists[playlistId].iconName;
    UI_OpenMenu(0, "popup_gettingdata");
    return "";
}

const char *__cdecl Playlist_GetCategoryIconName(int controllerIndex, unsigned int categoryId)
{
    if ( categoryId >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    1610,
                    0,
                    "%s\n\t(categoryId) = %i",
                    "(categoryId >= 0 && categoryId < 32)",
                    categoryId) )
    {
        __debugbreak();
    }
    if ( LiveStorage_DoWeHavePlaylists() )
        return categories[categoryId].iconName;
    UI_OpenMenu(0, "popup_gettingdata");
    return "";
}

int __cdecl Playlist_IsLocked(
                int controllerIndex,
                unsigned int playlistId,
                int xp,
                int prestige,
                int gametypeBan,
                int partyCount)
{
    int Int; // esi
    MemoryFile *v8; // [esp+0h] [ebp-8h]

    if ( playlistId >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    1621,
                    0,
                    "%s\n\t(playlistId) = %i",
                    "(playlistId >= 0 && playlistId < 64)",
                    playlistId) )
    {
        __debugbreak();
    }
    if ( LiveStorage_DoWeHavePlaylists() )
    {
        if ( xp >= playlists[playlistId].unlockXp || prestige )
        {
            if ( partyCount <= playlists[playlistId].maxPartySize )
            {
                if ( partyCount >= playlists[playlistId].minPartySize )
                {
                    if ( playlists[playlistId].gametypeBan == 1 && gametypeBan > 0 )
                    {
                        return 6;
                    }
                    else if ( !playlists[playlistId].visible_dvar
                                 || playlists[playlistId].visDvar && playlists[playlistId].visDvar->current.enabled )
                    {
                        if ( !playlists[playlistId].collectorsEditionRequired || collectors->current.enabled )
                        {
                            if ( playlists[playlistId].finishedSoloRequired
                                && (Int = Dvar_GetInt("mis_01"), Int < Dvar_GetInt("mis_01_unlock")) )
                            {
                                return 8;
                            }
                            else if ( prestige >= playlists[playlistId].unlockPLevel )
                            {
                                if ( Com_LocalClients_GetUsedControllerCount() <= playlists[playlistId].maxLocalPlayers )
                                {
                                    if ( playlists[playlistId].disableGuests && Live_CountGuestsInUse() > 0 )
                                    {
                                        return 14;
                                    }
                                    else
                                    {
                                        Playlist_DoWeHaveRequiredDLC(controllerIndex, playlistId);
                                        return 0;
                                    }
                                }
                                else
                                {
                                    return 13;
                                }
                            }
                            else
                            {
                                return 3;
                            }
                        }
                        else
                        {
                            return 7;
                        }
                    }
                    else
                    {
                        return 12;
                    }
                }
                else
                {
                    return 5;
                }
            }
            else
            {
                return 4;
            }
        }
        else
        {
            return 2;
        }
    }
    else
    {
        UI_OpenMenu(0, "popup_gettingdata");
        return 1;
    }
}

int __cdecl Playlist_CategoryIsLocked(int controllerIndex, unsigned int categoryId, int xp, int prestige)
{
    int Int; // esi

    if ( categoryId >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    1716,
                    0,
                    "%s\n\t(categoryId) = %i",
                    "(categoryId >= 0 && categoryId < 32)",
                    categoryId) )
    {
        __debugbreak();
    }
    if ( LiveStorage_DoWeHavePlaylists() )
    {
        if ( xp >= categories[categoryId].unlockXp || prestige )
        {
            if ( categories[categoryId].unlockPLevel && categories[categoryId].unlockPLevel > prestige )
            {
                return 3;
            }
            else if ( !categories[categoryId].visible_dvar
                         || categories[categoryId].visDvar && categories[categoryId].visDvar->current.enabled )
            {
                if ( !categories[categoryId].collectorsEditionRequired || collectors->current.enabled )
                {
                    if ( categories[categoryId].finishedSoloRequired
                        && (Int = Dvar_GetInt("mis_01"), Int < Dvar_GetInt("mis_01_unlock")) )
                    {
                        return 8;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    return 7;
                }
            }
            else
            {
                return 12;
            }
        }
        else
        {
            return 2;
        }
    }
    else
    {
        UI_OpenMenu(0, "popup_gettingdata");
        return 1;
    }
}

int __cdecl Playlist_GetMapPacksUsedByPlaylist(unsigned int playlistId)
{
    int mapIndex; // [esp+4h] [ebp-10h]
    int entryNum; // [esp+Ch] [ebp-8h]
    int mapPacksUsed; // [esp+10h] [ebp-4h]

    mapPacksUsed = 0;
    if ( playlistId >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    1783,
                    0,
                    "playlistId doesn't index MAX_PLAYLISTS\n\t%i not in [0, %i)",
                    playlistId,
                    64) )
    {
        __debugbreak();
    }
    for ( entryNum = 0; entryNum < Playlist_GetNumEntries(playlistId); ++entryNum )
    {
        Playlist_GetEntry(playlistId, entryNum);
        mapIndex = Live_GetMapIndex();
        if ( mapIndex >= 0 )
        {
            if ( mapIndex >= Live_GetMapCount()
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                            1796,
                            0,
                            "%s\n\t(mapIndex) = %i",
                            "(mapIndex >= 0 && mapIndex < Live_GetMapCount())",
                            mapIndex) )
            {
                __debugbreak();
            }
            mapPacksUsed |= Live_GetMapSource(mapIndex);
        }
        else
        {
            mapPacksUsed |= 2u;
        }
    }
    return mapPacksUsed;
}

bool __cdecl Playlist_DoWeHaveRequiredDLC(int controllerIndex, unsigned int playlistId)
{
    int availablePacks; // [esp+Ch] [ebp-4h]

    if ( playlistId >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    1810,
                    0,
                    "playlistId doesn't index MAX_PLAYLISTS\n\t%i not in [0, %i)",
                    playlistId,
                    64) )
    {
        __debugbreak();
    }
    if ( !LiveStorage_DoWeHavePlaylists() )
    {
        UI_OpenMenu(0, "popup_gettingdata");
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                        1811,
                        0,
                        "%s",
                        "HavePlaylists( controllerIndex )") )
            __debugbreak();
    }
    availablePacks = Content_GetAvailableContentPacks();
    return (playlists[playlistId].requiredDLCMask & ~availablePacks) == 0
            && (Playlist_GetMapPacksUsedByPlaylist(playlistId) & availablePacks) != 0;
}

const char *__cdecl Playlist_GetCategoryInternalName(int categoryId)
{
    return categories[categoryId].internalName;
}

playlistGametype *__cdecl Playlist_FindGametype(const char *gametypeInternalName)
{
    int gametypeNum; // [esp+0h] [ebp-4h]

    for ( gametypeNum = 0; gametypeNum < 64; ++gametypeNum )
    {
        if ( gametypes[gametypeNum].internalName && !I_stricmp(gametypes[gametypeNum].internalName, gametypeInternalName) )
            return &gametypes[gametypeNum];
    }
    Com_Error(ERR_DROP, "Unknown gametype name '%s' in playlist.\n", gametypeInternalName);
    return 0;
}

int __cdecl Playlist_GetGametypeCount()
{
    return numGametypes;
}

const char *__cdecl Playlist_GetGametypeLocalizedName(unsigned int index)
{
    if ( index >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    1949,
                    0,
                    "%s",
                    "index >= 0 && index < MAX_PLAYLIST_GAMETYPES") )
    {
        __debugbreak();
    }
    return gametypes[index].localizedName;
}

const char *__cdecl Playlist_GetGametypeInternalName(unsigned int index)
{
    if ( index >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    1956,
                    0,
                    "%s",
                    "index >= 0 && index < MAX_PLAYLIST_GAMETYPES") )
    {
        __debugbreak();
    }
    return gametypes[index].internalName;
}

int __cdecl Playlist_ChooseEntry(int localControllerIndex, int lastSelection)
{
    __int64 v3; // kr00_8
    int itemSelected; // [esp+4h] [ebp-1Ch]
    int playlistId; // [esp+Ch] [ebp-14h]
    int totalWeight; // [esp+1Ch] [ebp-4h]

    playlistId = Playlist_GetPlaylistIdForNum(localControllerIndex, playlist->current.integer);
    if ( !Playlist_GetNumEntries(playlistId) )
    {
        Com_Error(ERR_DROP, "Playlist %s has no entries\n", playlists[playlistId].name);
        return -1;
    }
    totalWeight = Playlist_CalcTotalWeights(playlistId, lastSelection);
    if ( !totalWeight )
    {
        Playlist_ResetWeights(playlistId);
        totalWeight = Playlist_CalcTotalWeights(playlistId, lastSelection);
    }
    if ( !Dvar_GetBool("party_linearMapCycle") && !Dvar_GetBool("party_linearMapCycleRandom") )
        goto LABEL_15;
    if ( lastSelection >= 0 && lastSelection < Playlist_GetNumEntries(playlistId) )
    {
        if ( lastSelection == Playlist_GetNumEntries(playlistId) - 1 )
            itemSelected = 0;
        else
            itemSelected = lastSelection + 1;
        goto LABEL_16;
    }
    if ( !Dvar_GetBool("party_linearMapCycleRandom") )
    {
        itemSelected = 0;
    }
    else
    {
LABEL_15:
        v3 = rand();
        itemSelected = Playlist_FindEntryByWeight(playlistId, v3 * totalWeight / 0x8000uLL, lastSelection);
    }
LABEL_16:
    if ( itemSelected >= Playlist_GetNumEntries(playlistId)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    2028,
                    0,
                    "%s\n\t(itemSelected) = %i",
                    "(itemSelected < Playlist_GetNumEntries( playlistId ))",
                    itemSelected) )
    {
        __debugbreak();
    }
    Playlist_MarkEntryPlayed(playlistId, itemSelected);
    return itemSelected;
}

void __cdecl Playlist_ResetWeights(int playlistId)
{
    playlistEntry *entry; // [esp+0h] [ebp-8h]
    int entryNum; // [esp+4h] [ebp-4h]

    for ( entryNum = 0; entryNum < Playlist_GetNumEntries(playlistId); ++entryNum )
    {
        entry = Playlist_GetEntry(playlistId, entryNum);
        if ( !entry->ignore )
            entry->remainingWeight = entry->fullWeight;
    }
}

int __cdecl Playlist_CalcTotalWeights(int playlistId, int lastSelection)
{
    playlistEntry *entry; // [esp+0h] [ebp-Ch]
    int weight; // [esp+4h] [ebp-8h]
    int entryNum; // [esp+8h] [ebp-4h]

    weight = 0;
    for ( entryNum = 0; entryNum < Playlist_GetNumEntries(playlistId); ++entryNum )
    {
        if ( entryNum != lastSelection )
        {
            entry = Playlist_GetEntry(playlistId, entryNum);
            if ( !entry->ignore )
                weight += entry->remainingWeight;
        }
    }
    return weight;
}

int __cdecl Playlist_FindEntryByWeight(int playlistId, int selectedWeight, int lastSelection)
{
    int v4; // eax
    playlistEntry *entry; // [esp+0h] [ebp-Ch]
    int weight; // [esp+4h] [ebp-8h]
    int entryNum; // [esp+8h] [ebp-4h]

    weight = 0;
    for ( entryNum = 0; entryNum < Playlist_GetNumEntries(playlistId); ++entryNum )
    {
        if ( entryNum != lastSelection )
        {
            entry = Playlist_GetEntry(playlistId, entryNum);
            if ( !entry->ignore )
            {
                weight += entry->remainingWeight;
                if ( selectedWeight < weight )
                    return entryNum;
            }
        }
    }
    if ( Playlist_CalcTotalWeights(playlistId, lastSelection) )
    {
        v4 = Playlist_CalcTotalWeights(playlistId, lastSelection);
        Com_Error(
            ERR_DROP,
            "Invalid selectedWeight of %i for playlist %i, total weight for this playlist is %i",
            selectedWeight,
            playlistId,
            v4);
        return weight;
    }
    else if ( lastSelection < 0 || lastSelection >= Playlist_GetNumEntries(playlistId) )
    {
        Com_Printf(0, "Playlist: an entry couldn't be found, returning first entry\n");
        return 0;
    }
    else
    {
        Com_Printf(0, "Playlist: all entries where ignored, returning lastSelection entry\n");
        return lastSelection;
    }
}

void __cdecl Playlist_MarkEntryPlayed(int playlistId, int entryNum)
{
    playlistEntry *entry; // [esp+0h] [ebp-4h]

    entry = Playlist_GetEntry(playlistId, entryNum);
    if ( entry->remainingWeight )
        --entry->remainingWeight;
    else
        Com_PrintWarning(
            16,
            "Trying to mark playlistId %i, entry %i as played but it's already reached its quota for this playlist\n",
            playlistId,
            entryNum);
}

void __cdecl Playlist_RunRules(int localControllerIndex, int entryNum)
{
    int LocalClientNum; // eax
    int v3; // eax
    int v4; // eax
    int v5; // eax
    char *v6; // [esp-4h] [ebp-14h]
    char *rules; // [esp-4h] [ebp-14h]
    char *v8; // [esp-4h] [ebp-14h]
    char *v9; // [esp-4h] [ebp-14h]
    playlistEntry *entry; // [esp+0h] [ebp-10h]
    unsigned int playlistId; // [esp+4h] [ebp-Ch]
    int categoryId; // [esp+8h] [ebp-8h]
    playlistGametype *gametypeEntry; // [esp+Ch] [ebp-4h]

    if ( entryNum != 255 )
    {
        playlistId = Playlist_GetPlaylistIdForNum(localControllerIndex, playlist->current.integer);
        categoryId = Playlist_GetCategoryIdForPlaylistId(playlistId);
        entry = Playlist_GetEntry(playlistId, entryNum);
        gametypeEntry = Playlist_FindGametype(entry->gametype);
        Dvar_SetBoolByName("xblive_rankedmatch", playlists[playlistId].ranked);
        Dvar_SetStringByName("ui_mapname", (char *)entry->mapname);
        if ( gametypeEntry )
        {
            Com_DPrintf(
                16,
                "Setting up gametype %s (running script %s)\n",
                gametypeEntry->localizedName,
                gametypeEntry->scriptName);
            Dvar_SetStringByName("ui_gametype", (char *)gametypeEntry->scriptName);
        }
        else
        {
            Dvar_SetStringByName("ui_gametype", (char *)entry->gametype);
        }
        v6 = va("exec %s", "default_xboxlive.cfg");
        LocalClientNum = Com_ControllerIndex_GetLocalClientNum(localControllerIndex);
        Cmd_ExecuteSingleCommand(LocalClientNum, localControllerIndex, v6);
        Com_DPrintf(14, "Setting up rules for the '%s' category\n", categories[categoryId].localizedName);
        rules = (char *)categories[categoryId].rules;
        v3 = Com_ControllerIndex_GetLocalClientNum(localControllerIndex);
        Cmd_ExecuteSingleCommand(v3, localControllerIndex, rules);
        Com_DPrintf(14, "Setting up rules for the '%s' playlist\n", playlists[playlistId].name);
        v8 = (char *)playlists[playlistId].rules;
        v4 = Com_ControllerIndex_GetLocalClientNum(localControllerIndex);
        Cbuf_ExecuteBuffer(v4, localControllerIndex, v8);
        if ( gametypeEntry )
        {
            Com_DPrintf(
                16,
                "Running custom game rules for gametype %s: %s\n",
                gametypeEntry->localizedName,
                gametypeEntry->rules);
            if ( gametypeEntry->rules && *gametypeEntry->rules )
            {
                v9 = (char *)gametypeEntry->rules;
                v5 = Com_ControllerIndex_GetLocalClientNum(localControllerIndex);
                Cbuf_ExecuteBuffer(v5, localControllerIndex, v9);
            }
            Dvar_SetBoolByName("party_teambased", gametypeEntry->teamBased);
        }
        if ( xblive_wagermatch->current.enabled )
            Dvar_SetIntByName("scr_wagerBet", playlists[playlistId].wagerBet);
    }
}

int __cdecl Playlist_GetVersionNumber()
{
    return playlist_versionNum;
}

int __cdecl Playlist_GetUnlockXp(int localControllerIndex, unsigned int playlistId)
{
    if ( playlistId >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    2260,
                    0,
                    "playlistId doesn't index MAX_PLAYLISTS\n\t%i not in [0, %i)",
                    playlistId,
                    64) )
    {
        __debugbreak();
    }
    if ( LiveStorage_DoWeHavePlaylists() )
        return playlists[playlistId].unlockXp;
    UI_OpenMenu(0, "popup_gettingdata");
    return 0x7FFFFFFF;
}

int __cdecl Playlist_GetUnlockPLevel(int localControllerIndex, unsigned int playlistId)
{
    if ( playlistId >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    2271,
                    0,
                    "playlistId doesn't index MAX_PLAYLISTS\n\t%i not in [0, %i)",
                    playlistId,
                    64) )
    {
        __debugbreak();
    }
    if ( LiveStorage_DoWeHavePlaylists() )
        return playlists[playlistId].unlockPLevel;
    UI_OpenMenu(0, "popup_gettingdata");
    return 0x7FFFFFFF;
}

int __cdecl Playlist_GetMaxPartySize(int localControllerIndex, unsigned int playlistId)
{
    if ( playlistId >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    2306,
                    0,
                    "playlistId doesn't index MAX_PLAYLISTS\n\t%i not in [0, %i)",
                    playlistId,
                    64) )
    {
        __debugbreak();
    }
    if ( LiveStorage_DoWeHavePlaylists() )
        return playlists[playlistId].maxPartySize;
    UI_OpenMenu(0, "popup_gettingdata");
    return 0;
}

int __cdecl Playlist_GetMinPartySize(int localControllerIndex, unsigned int playlistId)
{
    if ( playlistId >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    2316,
                    0,
                    "playlistId doesn't index MAX_PLAYLISTS\n\t%i not in [0, %i)",
                    playlistId,
                    64) )
    {
        __debugbreak();
    }
    if ( LiveStorage_DoWeHavePlaylists() )
        return playlists[playlistId].minPartySize;
    UI_OpenMenu(0, "popup_gettingdata");
    return 0;
}

int __cdecl Playlist_GetWagerBet(int localControllerIndex, unsigned int playlistId)
{
    if ( playlistId >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                    2326,
                    0,
                    "playlistId doesn't index MAX_PLAYLISTS\n\t%i not in [0, %i)",
                    playlistId,
                    64) )
    {
        __debugbreak();
    }
    if ( LiveStorage_DoWeHavePlaylists() )
        return playlists[playlistId].wagerBet;
    UI_OpenMenu(0, "popup_gettingdata");
    return 0;
}

void __cdecl Playlist_SetPrevPlaylist()
{
    unsigned int categoryId; // [esp+0h] [ebp-4h]
    unsigned int categoryIda; // [esp+0h] [ebp-4h]

    if ( xblive_wagermatch->current.enabled )
    {
        if ( wagerCategory->current.integer >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                        2350,
                        0,
                        "%s\n\t(wagerCategory->current.integer) = %i",
                        "(wagerCategory->current.integer >= 0 && wagerCategory->current.integer < 32)",
                        wagerCategory->current.integer) )
        {
            __debugbreak();
        }
        categoryId = Playlist_GetCategoryIdForNum(wagerCategory->current.integer);
        if ( categoryId >= 0x20
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                        2353,
                        0,
                        "%s\n\t(categoryId) = %i",
                        "(categoryId >= 0 && categoryId < 32)",
                        categoryId) )
        {
            __debugbreak();
        }
        if ( wagerCategoryPlaylist->current.integer >= 0x40u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                        2354,
                        0,
                        "%s\n\t(wagerCategoryPlaylist->current.integer) = %i",
                        "(wagerCategoryPlaylist->current.integer >= 0 && wagerCategoryPlaylist->current.integer < 64)",
                        wagerCategoryPlaylist->current.integer) )
        {
            __debugbreak();
        }
        prevPlaylist[categoryId] = wagerCategoryPlaylist->current.integer;
    }
    else
    {
        if ( category->current.integer >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                        2360,
                        0,
                        "%s\n\t(category->current.integer) = %i",
                        "(category->current.integer >= 0 && category->current.integer < 32)",
                        category->current.integer) )
        {
            __debugbreak();
        }
        categoryIda = Playlist_GetCategoryIdForNum(category->current.integer);
        if ( categoryIda >= 0x20
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                        2363,
                        0,
                        "%s\n\t(categoryId) = %i",
                        "(categoryId >= 0 && categoryId < 32)",
                        categoryIda) )
        {
            __debugbreak();
        }
        if ( categoryPlaylist->current.integer >= 0x40u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                        2364,
                        0,
                        "%s\n\t(categoryPlaylist->current.integer) = %i",
                        "(categoryPlaylist->current.integer >= 0 && categoryPlaylist->current.integer < 64)",
                        categoryPlaylist->current.integer) )
        {
            __debugbreak();
        }
        prevPlaylist[categoryIda] = categoryPlaylist->current.integer;
    }
}

void __cdecl Playlist_GetPrevPlaylist()
{
    int PlaylistNumFromId; // eax
    unsigned int value; // [esp+0h] [ebp-18h]
    unsigned int oldPlaylist; // [esp+4h] [ebp-14h]
    int playlistId; // [esp+10h] [ebp-8h]
    int playlistIda; // [esp+10h] [ebp-8h]
    unsigned int categoryId; // [esp+14h] [ebp-4h]
    unsigned int categoryIda; // [esp+14h] [ebp-4h]

    if ( xblive_wagermatch->current.enabled )
    {
        if ( wagerCategory->current.integer >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                        2378,
                        0,
                        "%s\n\t(wagerCategory->current.integer) = %i",
                        "(wagerCategory->current.integer >= 0 && wagerCategory->current.integer < 32)",
                        wagerCategory->current.integer) )
        {
            __debugbreak();
        }
        categoryId = Playlist_GetCategoryIdForNum(wagerCategory->current.integer);
        if ( categoryId >= 0x20
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                        2381,
                        0,
                        "%s\n\t(categoryId) = %i",
                        "(categoryId >= 0 && categoryId < 32)",
                        categoryId) )
        {
            __debugbreak();
        }
        oldPlaylist = prevPlaylist[categoryId];
        if ( oldPlaylist >= 0x40
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                        2385,
                        0,
                        "%s\n\t(oldPlaylist) = %i",
                        "(oldPlaylist >= 0 && oldPlaylist < 64)",
                        oldPlaylist) )
        {
            __debugbreak();
        }
        Dvar_SetInt((dvar_s *)wagerCategoryPlaylist, oldPlaylist);
        playlistId = Playlist_GetPlaylistIdForNum(
                                     0,
                                     wagerCategoryPlaylist->current.unsignedInt,
                                     wagerCategory->current.integer);
        PlaylistNumFromId = Playlist_GetPlaylistNumFromId(0, playlistId);
    }
    else
    {
        if ( category->current.integer >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                        2394,
                        0,
                        "%s\n\t(category->current.integer) = %i",
                        "(category->current.integer >= 0 && category->current.integer < 32)",
                        category->current.integer) )
        {
            __debugbreak();
        }
        categoryIda = Playlist_GetCategoryIdForNum(category->current.integer);
        if ( categoryIda >= 0x20
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                        2397,
                        0,
                        "%s\n\t(categoryId) = %i",
                        "(categoryId >= 0 && categoryId < 32)",
                        categoryIda) )
        {
            __debugbreak();
        }
        value = prevPlaylist[categoryIda];
        if ( value >= 0x40
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp",
                        2401,
                        0,
                        "%s\n\t(oldPlaylist) = %i",
                        "(oldPlaylist >= 0 && oldPlaylist < 64)",
                        value) )
        {
            __debugbreak();
        }
        Dvar_SetInt((dvar_s *)categoryPlaylist, value);
        playlistIda = Playlist_GetPlaylistIdForNum(0, categoryPlaylist->current.unsignedInt, category->current.integer);
        PlaylistNumFromId = Playlist_GetPlaylistNumFromId(0, playlistIda);
    }
    Dvar_SetInt((dvar_s *)playlist, PlaylistNumFromId);
}

void __cdecl Playlist_SetPlaylistEntry(int num)
{
    int NumEntries; // eax
    unsigned int playlistId; // [esp+0h] [ebp-4h]

    playlistId = Playlist_GetPlaylistIdForNum(0, playlist->current.integer);
    if ( num < 0 || num >= Playlist_GetNumEntries(playlistId) )
    {
        NumEntries = Playlist_GetNumEntries(playlistId);
        Com_PrintError(16, "Playlist_SetPlaylistEntry error: %d not between (0,%d)\n", num, NumEntries);
    }
    else
    {
        g_currentEntry = num;
        Dvar_SetInt((dvar_s *)playlist_entry, num);
    }
}

void __cdecl Playlist_SetSVMapRotation()
{
    char *String; // eax
    char *v1; // eax
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    playlistEntry *v6; // eax
    char *scriptName; // [esp-4h] [ebp-8ACh]
    char *mapname; // [esp-4h] [ebp-8ACh]
    char *v9; // [esp-4h] [ebp-8ACh]
    char *v11; // [esp+Ch] [ebp-89Ch]
    char *v12; // [esp+20h] [ebp-888h]
    char *v14; // [esp+48h] [ebp-860h]
    char *v16; // [esp+50h] [ebp-858h]
    char *v18; // [esp+58h] [ebp-850h]
    int i; // [esp+84h] [ebp-824h]
    size_t gametype_len; // [esp+88h] [ebp-820h]
    playlistEntry *entry; // [esp+8Ch] [ebp-81Ch] BYREF
    char rotation[1024]; // [esp+90h] [ebp-818h] BYREF
    int playlistId; // [esp+490h] [ebp-418h]
    int total_len; // [esp+494h] [ebp-414h]
    int mapname_len; // [esp+498h] [ebp-410h]
    playlistGametype *gametypeEntryPrevious; // [esp+49Ch] [ebp-40Ch] BYREF
    char token[1024]; // [esp+4A0h] [ebp-408h] BYREF
    playlistGametype *gametypeEntry; // [esp+8A4h] [ebp-4h]

    memset(rotation, 0, sizeof(rotation));
    memset(token, 0, sizeof(token));
    playlistId = Playlist_GetPlaylistIdForNum(0, playlist->current.integer);
    if ( Playlist_GetNumEntries(playlistId) )
    {
        Dvar_SetStringByName("sv_maprotationcurrent", (char *)"");
        Dvar_SetStringByName("sv_maprotation", (char *)"");
        total_len = 0;
        for ( i = 0; i < Playlist_GetNumEntries(playlistId); ++i )
        {
            entry = Playlist_GetEntry(playlistId, i);
            gametypeEntry = Playlist_FindGametype(entry->gametype);
            if ( gametypeEntry )
            {
                mapname_len = strlen(entry->mapname);
                gametype_len = strlen(gametypeEntry->scriptName);
                scriptName = (char*)gametypeEntry->scriptName;
                String = (char*)Dvar_GetString("playlist_excludeGametype");
                v1 = strstr(String, scriptName);
                if ( v1
                    || (mapname = (char *)entry->mapname,
                            v2 = (char *)Dvar_GetString("playlist_excludeMap"),
                            v3 = strstr(v2, mapname),
                            v3)
                    || (v9 = (char *)va("%s %s", gametypeEntry->scriptName, entry->mapname),
                            v4 = (char *)Dvar_GetString("playlist_excludeGametypeMap"),
                            v5 = strstr(v4, v9),
                            v5) )
                {
                    Com_Printf(
                        16,
                        "Playlist %d entry %d excluded: %s %s.\n",
                        playlist->current.integer,
                        i,
                        gametypeEntry->scriptName,
                        entry->mapname);
                    entry->ignore = 1;
                }
                else
                {
                    entry->ignore = 0;
                    if ( i )
                    {
                        v6 = Playlist_GetEntry(playlistId, i - 1);
                        gametypeEntryPrevious = Playlist_FindGametype(v6->gametype);
                        if ( !gametypeEntryPrevious || I_strcmp(gametypeEntryPrevious->scriptName, gametypeEntry->scriptName) )
                        {
                            v16 = (char *)&gametypeEntryPrevious + 3;
                            while ( *++v16 )
                                ;
                            strcpy(v16, " gametype ");
                            strncat(token, gametypeEntry->scriptName, gametype_len);
                        }
                    }
                    else
                    {
                        v18 = (char *)&gametypeEntryPrevious + 3;
                        while ( *++v18 )
                            ;
                        strcpy(v18, "gametype ");
                        strncat(token, gametypeEntry->scriptName, gametype_len);
                    }
                    v14 = (char *)&gametypeEntryPrevious + 3;
                    while ( *++v14 )
                        ;
                    strcpy(v14, " map ");
                    strncat(token, entry->mapname, mapname_len);
                    if ( (unsigned int)(&rotation[strlen(rotation) + 1] - &rotation[1] + &token[strlen(token) + 1] - &token[1]) >= 0x400 )
                        break;
                    v12 = &token[strlen(token) + 1];
                    v11 = (char *)&entry + 3;
                    while ( *++v11 )
                        ;
                    memcpy(v11, token, v12 - token);
                    token[0] = 0;
                }
            }
        }
        Dvar_SetStringByName("sv_maprotation", rotation);
    }
}

void __cdecl Playlist_SetSVMapRotationCurrent(int entryNum)
{
    char *v2; // [esp+18h] [ebp-434h]
    char *v4; // [esp+30h] [ebp-41Ch]
    playlistEntry *entry; // [esp+3Ch] [ebp-410h]
    int playlistId; // [esp+40h] [ebp-40Ch] BYREF
    char token[1024]; // [esp+44h] [ebp-408h] BYREF
    playlistGametype *gametypeEntry; // [esp+448h] [ebp-4h]

    memset(token, 0, sizeof(token));
    playlistId = Playlist_GetPlaylistIdForNum(0, playlist->current.integer);
    if ( Playlist_GetNumEntries(playlistId) )
    {
        entry = Playlist_GetEntry(playlistId, entryNum);
        gametypeEntry = Playlist_FindGametype(entry->gametype);
        if ( gametypeEntry )
        {
            v4 = (char *)&playlistId + 3;
            while ( *++v4 )
                ;
            strcpy(v4, "gametype ");
            strncat(token, gametypeEntry->scriptName, strlen(gametypeEntry->scriptName));
        }
        v2 = (char *)&playlistId + 3;
        while ( *++v2 )
            ;
        strcpy(v2, " map ");
        strncat(token, entry->mapname, strlen(entry->mapname));
        Dvar_SetStringByName("sv_mapRotationCurrent", token);
    }
}

void __cdecl Playlist_SVMapRotate()
{
    int v0; // eax

    v0 = Playlist_ChooseEntry(0, g_currentEntry);
    Playlist_SetPlaylistEntry(v0);
    Playlist_RunRules(0, g_currentEntry);
    Playlist_SetSVMapRotationCurrent(g_currentEntry);
}

cmd_function_s Playlist_Fetch_f_VAR;
cmd_function_s Playlist_Fetch_f_VAR_SERVER;
cmd_function_s Playlist_Pick_f_VAR;
cmd_function_s Playlist_Pick_f_VAR_SERVER;
void __cdecl Playlist_Init()
{
    Cmd_AddCommandInternal("serverplaylistfetch", Cbuf_AddServerText_f, &Playlist_Fetch_f_VAR);
    Cmd_AddServerCommandInternal("serverplaylistfetch", Playlist_Fetch_f, &Playlist_Fetch_f_VAR_SERVER);
    Cmd_AddCommandInternal("serverplaylistpick", Cbuf_AddServerText_f, &Playlist_Pick_f_VAR);
    Cmd_AddServerCommandInternal("serverplaylistpick", Playlist_Pick_f, &Playlist_Pick_f_VAR_SERVER);
    Playlist_RegisterDvars();
}

void Playlist_RegisterDvars()
{
    int LicenseType; // eax

    party_linearMapCycle = _Dvar_RegisterBool("party_linearMapCycle", 0, 0, "We will cycle through the maps linearly if this is true.");
    party_linearMapCycleRandom = _Dvar_RegisterBool(
        "party_linearMapCycleRandom",
        0,
        0,
        "We will cycle through the maps linearly if this is true but picks a random start map.");
    party_maxplayers = _Dvar_RegisterInt("party_maxplayers", 18, 1, 32, 0, "Maximum number of players");
    playlist_excludeMap = _Dvar_RegisterString(
        "playlist_excludeMap",
        "",
        1u,
        "Map,Map to exclude from rotation.");
    playlist_excludeGametype = _Dvar_RegisterString(
        "playlist_excludeGametype",
        "",
        1u,
        "Gametype,Gametype to exclude from rotation.");
    playlist_excludeGametypeMap = _Dvar_RegisterString(
        "playlist_excludeGametypeMap",
        "",
        1u,
        "Gametype Map, Gametype Map pairs to exclude from rotation.");
    LicenseType = SV_GetLicenseType();
    if (SV_IsServerRanked(LicenseType))
        playlist_enabled = _Dvar_RegisterBool("playlist_enabled", 1, 0x44u, "The server is using playlist");
    else
        playlist_enabled = _Dvar_RegisterBool("playlist_enabled", 0, 4u, "The server is using playlist");
    playlist_next = _Dvar_RegisterInt("playlist_next", 1, 1, 64, 0, "The server playlist for next map rotation");
    playlist_entry = _Dvar_RegisterInt("playlist_entry", 0, 0, 0x2000, 4u, "The current server playlist entry.");
}

void __cdecl Playlist_Fetch_f()
{
    LiveStorage_FetchPlaylistsLocal(0);
}

void __cdecl Playlist_Pick_f()
{
    const char *String; // eax

    Playlist_SVMapRotate();
    String = Dvar_GetString("sv_mapRotationCurrent");
    Com_Printf(0, "\n\"sv_mapRotationCurrent\" is:\"%s\"\n\n", String);
}

int __cdecl Playlist_GetWagerAmount(int version, int index, int *wagerAmount)
{
    if ( !wagerAmount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_playlists.cpp", 2628, 0, "%s", "wagerAmount") )
    {
        __debugbreak();
    }
    if ( playlist_versionNum == version && index > 0 && index < totalEntries )
    {
        *wagerAmount = playlists[index].wagerBet;
        return 0;
    }
    else if ( version == playlist_versionNum )
    {
        return 0;
    }
    else
    {
        return 2 * (version >= playlist_versionNum) - 1;
    }
}

