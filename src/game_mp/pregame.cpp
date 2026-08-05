#include "pregame.h"
#include <server_mp/sv_main_pc_mp.h>
#include <cstring>

pregameStateGlobal pregameG;
const dvar_t *g_pregame_enabled;
const dvar_t *ui_pregame;
const dvar_t *g_pregame_minplayers;

void __cdecl Pregame_SetState(pregameStateEnum s)
{
    pregameG.state = s;
}

pregameStateEnum __cdecl Pregame_GetState()
{
    return pregameG.state;
}

bool __cdecl Pregame_isEnabled()
{
    int LicenseType; // eax

    LicenseType = SV_GetLicenseType();
    if ( SV_IsServerRanked(LicenseType) )
        return 1;
    if ( Dvar_GetBool("xblive_wagermatch") )
        return 1;
    return g_pregame_enabled->current.enabled;
}

void __cdecl Pregame_StartPregame()
{
    Pregame_SetState(PREGAME_INPROGRESS);
}

void __cdecl Pregame_StartGame()
{
    Pregame_SetState(PREGAME_GAMESTARTED);
}

void __cdecl Pregame_Reset()
{
    Pregame_SetState(PREGAME_IDLE);
}

void __cdecl Pregame_ResetData()
{
    memset(pregameG.clients, 0, sizeof(pregameG.clients));
}

void __cdecl Pregame_ResetDataForClient(unsigned int clientNum)
{
    pregameClient *v1; // ecx

    if ( clientNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\pregame.cpp",
                    70,
                    0,
                    "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    clientNum,
                    32) )
    {
        __debugbreak();
    }
    v1 = &pregameG.clients[clientNum];
    v1->pregame_team = TEAM_FREE;
    *(unsigned int *)v1->pregame_class = 0;
    *(unsigned int *)&v1->pregame_class[4] = 0;
    *(unsigned int *)&v1->pregame_class[8] = 0;
    *(unsigned int *)&v1->pregame_class[12] = 0;
    *(unsigned int *)&v1->pregame_class[16] = 0;
    *(unsigned int *)&v1->pregame_class[20] = 0;
    *(unsigned int *)&v1->pregame_class[24] = 0;
    *(unsigned int *)&v1->pregame_class[28] = 0;
}

bool __cdecl Pregame_ShouldLoadPregame()
{
    return Pregame_isEnabled() && Pregame_GetState() < PREGAME_GAMESTARTED;
}

void __cdecl Pregame_RegisterDvars()
{
    g_pregame_enabled = _Dvar_RegisterBool("g_pregame_enabled", 0, 0, "Begin matches with a Pre-Match period");
    ui_pregame = _Dvar_RegisterBool("ui_pregame", 0, 0x80u, "Used by server to let clients know the Pre-Match state");
    g_pregame_minplayers = _Dvar_RegisterInt(
                                                     "party_minplayers",
                                                     2,
                                                     1,
                                                     32,
                                                     0,
                                                     "Minimum number of players needed to start the game");
}

team_t __cdecl Pregame_GetAssignedPregameTeam(unsigned int clientNum)
{
    if ( clientNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\pregame.cpp",
                    90,
                    0,
                    "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    clientNum,
                    32) )
    {
        __debugbreak();
    }
    return pregameG.clients[clientNum].pregame_team;
}

void __cdecl Pregame_SetAssignedPregameClass(unsigned int clientNum, const char *pclass)
{
    if ( clientNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\pregame.cpp",
                    96,
                    0,
                    "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    clientNum,
                    32) )
    {
        __debugbreak();
    }
    I_strncpyz(pregameG.clients[clientNum].pregame_class, pclass, 32);
}

void __cdecl Pregame_SetAssignedPregameTeam(unsigned int clientNum, team_t pteam)
{
    if ( clientNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\pregame.cpp",
                    102,
                    0,
                    "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    clientNum,
                    32) )
    {
        __debugbreak();
    }
    pregameG.clients[clientNum].pregame_team = pteam;
}

char *__cdecl Pregame_GetAssignedPregameClass(unsigned int clientNum)
{
    if ( clientNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\pregame.cpp",
                    108,
                    0,
                    "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    clientNum,
                    32) )
    {
        __debugbreak();
    }
    return pregameG.clients[clientNum].pregame_class;
}

