#pragma once

struct cg_s;

void __cdecl CG_ParseServerInfo(int localClientNum);
void __cdecl CG_ParseCodInfo(int localClientNum);
void __cdecl CG_ParseCullDist();
void __cdecl CG_ParseTimeScale(int localClientNum);
void __cdecl CG_ParsePlayerInfos();
void __cdecl CG_ParseGameEndTime(int localClientNum);
void __cdecl CG_ParseFog(int localClientNum);
void __cdecl CG_SetConfigValues(int localClientNum);
void __cdecl CG_PrecacheScriptMenu(int localClientNum, int configStringIndex);
void __cdecl CG_RegisterServerMaterial(int configStringIndex);
void __cdecl CG_ParseClientSystemStateChange(int localClientNum, int sysIndex, char *pState);
void __cdecl CG_ParseAirsupport(
                int localClientNum,
                float x,
                float y,
                float z,
                int yaw,
                char *type,
                char *teamFaction,
                char *team,
                int owner,
                char *exitType,
                int serverTime,
                int height);
void __cdecl CG_ClearGenericFilter(cg_s *gameglob);
void __cdecl CG_MapRestart(int localClientNum, int savepersist);
void __cdecl CG_ClearEntityFxHandles(int localClientNum);
void __cdecl CG_CheckOpenWaitingScriptMenu(int localClientNum);
void __cdecl CG_CloseScriptMenu(int localClientNum, bool allowResponse);
void __cdecl CG_MenuShowNotify(int localClientNum, int menuToShow);
void __cdecl CG_ServerCommand(int localClientNum);
void __cdecl CG_DeployServerCommand(int localClientNum);
void __cdecl CG_ConfigStringModified(int localClientNum);
void __cdecl CG_UpdateVoteString(int localClientNum, const char *rawVoteString);
void __cdecl CG_AddToTeamChat(int localClientNum, const char *str);
void __cdecl CG_OpenScriptMenu(int localClientNum);
void __cdecl CG_RemoveChatEscapeChar(char *text);
void __cdecl CG_SetTeamScore(int localClientNum, unsigned int team, int score);
char __cdecl LocalSound(int localClientNum);
void __cdecl LocalSoundStop(int localClientNum);
void __cdecl CG_SetClientDvarFromServer(cg_s *cgameGlob, const char *dvarname, char *value);
void __cdecl CG_SetObjectiveText(cg_s *cgameGlob, const char *text);
void __cdecl CG_SetDrawHud(cg_s *cgameGlob, unsigned int value);
void __cdecl CG_SetScriptMainMenu(cg_s *cgameGlob, const char *text);
void __cdecl CG_BurnServerCommand(int localClientNum);
void __cdecl CG_AnimateUI(int localClientNum);
void __cdecl CG_ElectrifiedServerCommand(int localClientNum);
void __cdecl CG_WaterDropsServerCommand(int localClientNum);
void __cdecl CG_ScrCamera(int localClientNum, int cmd);
void __cdecl CG_ExecuteNewServerCommands(int localClientNum, int latestSequence);

struct GfxFog;
extern GfxFog cg_serverVolFog;