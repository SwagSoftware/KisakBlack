#include "cg_servercmds_mp.h"
#include "cg_actors_mp.h"
#include <client_mp/cl_cgame_mp.h>
#include <universal/q_shared.h>
#include <client_mp/cl_main_mp.h>
#include <universal/com_files.h>
#include <server_mp/sv_main_pc_mp.h>
#include <gfx_d3d/r_dpvs.h>
#include <live/live_win.h>
#include <universal/q_parse.h>
#include <gfx_d3d/r_fog.h>
#include <client_mp/cl_parse_mp.h>
#include <cgame/cg_scr_main.h>
#include <clientscript/cscr_vm.h>
#include "cg_scr_main_mp.h"
#include <bgame/bg_slidemove.h>
#include "cg_main_mp.h"
#include "cg_draw_mp.h"
#include "cg_snapshot_mp.h"
#include <cgame/cg_localents.h>
#include <EffectsCore/fx_system.h>
#include <DynEntity/DynEntity_client.h>
#include <glass/glass_client.h>
#include <gfx_d3d/r_bsp_load_obj.h>
#include <client/splitscreen.h>
#include <gfx_d3d/r_stream.h>
#include <gfx_d3d/r_water_sim.h>
#include <sound/snd_public_async.h>
#include <cgame/cg_sound.h>
#include <cgame/cg_main.h>
#include <client_mp/cl_input_mp.h>
#include "cg_consolecmds_mp.h"
#include <gfx_d3d/r_cinematic.h>
#include <cgame/cg_spawn.h>
#include <cgame/cg_bolt.h>
#include <qcommon/com_clients.h>
#include <cgame/offhandweapons.h>
#include <client_mp/cl_main_pc_mp.h>
#include <live/live_leaderboard.h>
#include <live/live_stats.h>
#include <live/live_storage_win.h>
#include <cgame/cg_hudelem.h>
#include <stringed/stringed_hooks.h>
#include <cgame/cg_drawtools.h>
#include <bgame/bg_misc.h>
#include <EffectsCore/fx_load_obj.h>
#include <gfx_d3d/r_model.h>
#include "cg_ui_animate_mp.h"

GfxFog cg_serverVolFog;

struct //__declspec(align(4)) $59835072FC2CD3936CE4A4C9F556010B // sizeof=0x48
{                                       // XREF: .data:cg_waitingScriptMenu/r
    char name[64];                      // XREF: CG_CheckOpenWaitingScriptMenu(int)+A/r
                                        // CG_CheckOpenWaitingScriptMenu(int)+4D/w ...
    int index;                          // XREF: CG_OpenScriptMenu+13F/r
                                        // CG_OpenScriptMenu+1DE/w
    bool useMouse;                      // XREF: CG_CheckOpenWaitingScriptMenu(int)+1D/r
                                        // CG_OpenScriptMenu+1ED/w
    // padding byte
    // padding byte
    // padding byte
} cg_waitingScriptMenu[1];

void __cdecl CG_ParseServerInfo(int localClientNum)
{
    char *v1; // eax
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    int v5; // eax
    bool IsServerRanked; // al
    char *v7; // eax
    int v8; // eax
    char *v9; // eax
    int v10; // eax
    char *v11; // eax
    int v12; // eax
    char *info; // [esp+0h] [ebp-Ch]
    cgs_t *cgs; // [esp+4h] [ebp-8h]
    char *mapname; // [esp+8h] [ebp-4h]

    info = CL_GetConfigString(0);
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    v1 = (char *)Info_ValueForKey(info, "sv_hostname");
    strncpy((char *)cgs->szHostName, v1, 0x100u);
    v2 = (char *)Info_ValueForKey(info, "g_gametype");
    strncpy((char *)cgs->gametype, v2, 0x20u);
    if ( !cgs->localServer )
        Dvar_SetStringByName("g_gametype", cgs->gametype);
    v3 = Info_ValueForKey(info, "com_maxclients");
    cgs->maxclients = atoi(v3);
    if ( (cgs->maxclients < 1 || cgs->maxclients > 32)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_servercmds_mp.cpp",
                    142,
                    0,
                    "%s\n\t(cgs->maxclients) = %i",
                    "(cgs->maxclients >= 1 && cgs->maxclients <= 32)",
                    cgs->maxclients) )
    {
        __debugbreak();
    }
    mapname = Info_ValueForKey(info, "mapname");
    Com_GetBspFilename(cgs->mapname, 0x40u, mapname);
    v4 = Info_ValueForKey(info, "sv_ranked");
    v5 = atoi(v4);
    IsServerRanked = SV_IsServerRanked(v5);
    Dvar_SetBoolByName("ui_ranked", IsServerRanked);
    v7 = Info_ValueForKey(info, "xblive_privatematch");
    v8 = atoi(v7);
    Dvar_SetBoolByName("xblive_privatematch", v8 != 0);
    v9 = Info_ValueForKey(info, "xblive_wagermatch");
    v10 = atoi(v9);
    Dvar_SetBoolByName("xblive_wagermatch", v10 != 0);
    v11 = Info_ValueForKey(info, "xblive_basictraining");
    v12 = atoi(v11);
    Dvar_SetBoolByName("xblive_basictraining", v12 != 0);
}

void __cdecl CG_ParseCodInfo(int localClientNum)
{
    char *key; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]
    char *value; // [esp+Ch] [ebp-4h]

    if ( !CG_GetLocalClientStaticGlobals(localClientNum)->localServer )
    {
        for ( i = 0; i < 150; ++i )
        {
            key = CL_GetConfigString(i + 23);
            if ( !*key )
                break;
            value = CL_GetConfigString(i + 173);
            Dvar_SetFromStringByName(key, value);
        }
    }
}

void __cdecl CG_ParseCullDist()
{
    char *info; // [esp+4h] [ebp-8h]
    float dist; // [esp+8h] [ebp-4h]

    info = CL_GetConfigString(7u);
    dist = atof(info);
    R_SetCullDist(dist);
}

void __cdecl CG_ParseTimeScale(int localClientNum)
{
    char *info; // [esp+0h] [ebp-8h]
    cg_s *cgameGlob; // [esp+4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgameGlob->timeScaleStart = Com_GetTimeScale();
    cgameGlob->timeScaleTimeStart = cgameGlob->time;
    info = CL_GetConfigString(0xEu);
    sscanf(info, "%g %d", &cgameGlob->timeScaleEnd, &cgameGlob->timeScaleTimeEnd);
}

void __cdecl CG_ParsePlayerInfos()
{
    int i; // [esp+10h] [ebp-44h]
    const char *nameList; // [esp+14h] [ebp-40h] BYREF
    char filteredName[32]; // [esp+18h] [ebp-3Ch] BYREF
    int clientNum; // [esp+3Ch] [ebp-18h]
    const char *playerName; // [esp+40h] [ebp-14h]
    const char *xuidStr; // [esp+44h] [ebp-10h]
    const char *playerInfo; // [esp+48h] [ebp-Ch]
    unsigned __int64 uid; // [esp+4Ch] [ebp-8h] BYREF

    for ( clientNum = 0; clientNum < 32; ++clientNum )
    {
        playerInfo = CL_GetConfigString(clientNum + 323);
        if ( *playerInfo )
        {
            Cmd_TokenizeStringNoEval(playerInfo);
            xuidStr = Cmd_Argv(0);
            playerName = Cmd_Argv(1);
            StringToXUID(xuidStr, &uid);
            Cmd_EndTokenizedString();
            I_strncpyz(filteredName, playerName, 32);
            for ( i = 0; filteredName[i] && i < 32; ++i )
            {
                if ( filteredName[i] == 1 )
                    filteredName[i] = 32;
            }
            nameList = filteredName;
            Live_AddRecentPlayers(&uid, &nameList, 1);
        }
    }
}

void __cdecl CG_ParseGameEndTime(int localClientNum)
{
    char *ConfigString; // eax
    cgs_t *cgs; // [esp+0h] [ebp-4h]

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    ConfigString = CL_GetConfigString(0xCu);
    cgs->gameEndTime = atoi(ConfigString);
}

void __cdecl CG_ParseFog(int localClientNum)
{
    parseInfo_t *v1; // eax
    parseInfo_t *v2; // eax
    parseInfo_t *v3; // eax
    parseInfo_t *v4; // eax
    parseInfo_t *v5; // eax
    parseInfo_t *v6; // eax
    parseInfo_t *v7; // eax
    parseInfo_t *v8; // eax
    parseInfo_t *v9; // eax
    parseInfo_t *v10; // eax
    parseInfo_t *v11; // eax
    parseInfo_t *v12; // eax
    parseInfo_t *v13; // eax
    parseInfo_t *v14; // eax
    parseInfo_t *v15; // eax
    parseInfo_t *v16; // eax
    float sunDirY; // [esp+44h] [ebp-54h]
    const char *info; // [esp+48h] [ebp-50h] BYREF
    float r; // [esp+4Ch] [ebp-4Ch]
    int transitionTime; // [esp+50h] [ebp-48h]
    float sunEndAng; // [esp+54h] [ebp-44h]
    float baseHeight; // [esp+58h] [ebp-40h]
    float sunStartAng; // [esp+5Ch] [ebp-3Ch]
    float fogColorScale; // [esp+60h] [ebp-38h]
    float start; // [esp+64h] [ebp-34h]
    float g; // [esp+68h] [ebp-30h]
    float b; // [esp+6Ch] [ebp-2Ch]
    float maxFogOpacity; // [esp+70h] [ebp-28h]
    float density; // [esp+74h] [ebp-24h]
    int time; // [esp+78h] [ebp-20h]
    float sunColorB; // [esp+7Ch] [ebp-1Ch]
    float sunDirZ; // [esp+80h] [ebp-18h]
    const char *token; // [esp+84h] [ebp-14h]
    float sunColorR; // [esp+88h] [ebp-10h]
    float sunColorG; // [esp+8Ch] [ebp-Ch]
    float sunDirX; // [esp+90h] [ebp-8h]
    float heightDensity; // [esp+94h] [ebp-4h]

    time = CG_GetLocalClientGlobals(localClientNum)->time;
    info = CL_GetConfigString(0xAu);
    token = (const char *)Com_Parse(&info);
    start = atof(token);
    token = (const char *)Com_Parse(&info);
    if ( token && *token )
    {
        density = atof(token);
        v1 = Com_Parse(&info);
        heightDensity = atof(v1->token);
        v2 = Com_Parse(&info);
        baseHeight = atof(v2->token);
        v3 = Com_Parse(&info);
        r = atof(v3->token);
        v4 = Com_Parse(&info);
        g = atof(v4->token);
        v5 = Com_Parse(&info);
        b = atof(v5->token);
        v6 = Com_Parse(&info);
        transitionTime = atoi(v6->token);
        v7 = Com_Parse(&info);
        fogColorScale = atof(v7->token);
        v8 = Com_Parse(&info);
        sunColorR = atof(v8->token);
        v9 = Com_Parse(&info);
        sunColorG = atof(v9->token);
        v10 = Com_Parse(&info);
        sunColorB = atof(v10->token);
        v11 = Com_Parse(&info);
        sunDirX = atof(v11->token);
        v12 = Com_Parse(&info);
        sunDirY = atof(v12->token);
        v13 = Com_Parse(&info);
        sunDirZ = atof(v13->token);
        v14 = Com_Parse(&info);
        sunStartAng = atof(v14->token);
        v15 = Com_Parse(&info);
        sunEndAng = atof(v15->token);
        v16 = Com_Parse(&info);
        maxFogOpacity = atof(v16->token);
        R_SetFogFromServer(
            localClientNum,
            start,
            r,
            g,
            b,
            density,
            heightDensity,
            baseHeight,
            fogColorScale,
            sunColorR,
            sunColorG,
            sunColorB,
            sunDirX,
            sunDirY,
            sunDirZ,
            sunStartAng,
            sunEndAng,
            maxFogOpacity);
        R_SwitchFog(localClientNum, 1u, time, transitionTime);

        cg_serverVolFog.fogStart = start;
        cg_serverVolFog.color[0] = r;
        cg_serverVolFog.color[1] = g;
        cg_serverVolFog.color[2] = b;
        cg_serverVolFog.density = density;
        cg_serverVolFog.heightDensity = heightDensity;
        cg_serverVolFog.baseHeight = baseHeight;
        cg_serverVolFog.color[3] = fogColorScale;
        cg_serverVolFog.sunFogColor[0] = sunColorR;
        cg_serverVolFog.sunFogColor[1] = sunColorG;
        cg_serverVolFog.sunFogColor[2] = sunColorB;
        cg_serverVolFog.sunFogDir[0] = sunDirX;
        cg_serverVolFog.sunFogDir[1] = sunDirY;
        cg_serverVolFog.sunFogDir[2] = sunDirZ;
        cg_serverVolFog.sunFogStartAng = sunStartAng;
        cg_serverVolFog.sunFogEndAng = sunEndAng;
        cg_serverVolFog.sunFogColor[3] = maxFogOpacity;
    }
    else
    {
        R_SwitchFog(localClientNum, 0, time, (int)start);
    }
}

void __cdecl CG_SetConfigValues(int localClientNum)
{
    cg_s *LocalClientGlobals; // eax
    char *ConfigString; // eax
    char *v3; // eax
    int i; // [esp+8h] [ebp-4h]
    signed int ia; // [esp+8h] [ebp-4h]
    signed int ib; // [esp+8h] [ebp-4h]
    int ic; // [esp+8h] [ebp-4h]

    CG_GetLocalClientGlobals(localClientNum);
    CL_ParseMapCenter();
    CG_ParseCullDist();
    LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
    R_SwitchFog(localClientNum, 0, LocalClientGlobals->time, 0);
    for ( i = 2548; i < 2580; ++i )
        CG_PrecacheScriptMenu(localClientNum, i);
    for ( ia = 3092; ia < 3100; ++ia )
    {
        ConfigString = CL_GetConfigString(ia);
        Material_RegisterHandle(ConfigString, 7);
    }
    for ( ib = 3100; ib < 3115; ++ib )
    {
        v3 = CL_GetConfigString(ib);
        Material_RegisterHandle(v3, 7);
    }
    for ( ic = 2581; ic < 2836; ++ic )
        CG_RegisterServerMaterial(ic);
    CG_ParseGameEndTime(localClientNum);
    CG_VisionSetConfigString_Naked(localClientNum);
    CG_VisionSetConfigString_Night(localClientNum);
}

void __cdecl CG_PrecacheScriptMenu(int localClientNum, int configStringIndex)
{
    char *configString; // [esp+0h] [ebp-4h]

    if ( (configStringIndex < 2548 || configStringIndex >= 2580)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_servercmds_mp.cpp",
                    598,
                    0,
                    "%s",
                    "(configStringIndex >= CS_SCRIPT_MENUS) && (configStringIndex < CS_SCRIPT_MENUS + MAX_SCRIPT_MENUS)") )
    {
        __debugbreak();
    }
    configString = CL_GetConfigString(configStringIndex);
    if ( *configString )
    {
        if ( !Load_ScriptMenu(localClientNum, configString, 7) )
            Com_Error(ERR_DROP, "Could not load script menu file '%s'", configString);
    }
}

void __cdecl CG_RegisterServerMaterial(int configStringIndex)
{
    char *materialName; // [esp+0h] [ebp-4h]

    if ( (configStringIndex < 2580 || configStringIndex >= 2836)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_servercmds_mp.cpp",
                    613,
                    0,
                    "%s\n\t(configStringIndex) = %i",
                    "(configStringIndex >= CS_SERVER_MATERIALS && configStringIndex < CS_SERVER_MATERIALS + 256)",
                    configStringIndex) )
    {
        __debugbreak();
    }
    materialName = CL_GetConfigString(configStringIndex);
    if ( *materialName )
        Material_RegisterHandle(materialName, 7);
}

void __cdecl CG_ParseClientSystemStateChange(int localClientNum, int sysIndex, char *pState)
{
    unsigned int j; // [esp+10h] [ebp-420h]
    char *state; // [esp+14h] [ebp-41Ch]
    char str[1028]; // [esp+18h] [ebp-418h] BYREF
    unsigned __int16 t; // [esp+420h] [ebp-10h]
    const char *sysName; // [esp+424h] [ebp-Ch]
    cg_s *cgs; // [esp+428h] [ebp-8h]
    const char *configString; // [esp+42Ch] [ebp-4h]

    if (cg_scr_data.clientsysstatechange && gScrVarPub[1].timeArrayId)
    {
        cgs = CG_GetLocalClientGlobals(localClientNum);
        state = Info_ValueForKey(pState, "s");
        str[0] = 0;
        if ( state )
        {
            I_strncpyz(str, state, 1024);
            for ( j = 0; j < &str[strlen(str) + 1] - &str[1]; ++j )
            {
                if ( str[j] == 33 )
                    str[j] = 32;
            }
        }
        Scr_AddString(str, SCRIPTINSTANCE_CLIENT);
        configString = CL_GetConfigString(sysIndex + 1538);
        sysName = Info_ValueForKey((char *)configString, "n");
        Scr_AddString((char *)sysName, SCRIPTINSTANCE_CLIENT);
        Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
        t = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_data.clientsysstatechange, 3u);
        Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
    }
}

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
                int height)
{
    unsigned __int16 t; // [esp+8h] [ebp-4h]

    if ( cg_scr_mp_data.airsupport )
    {
        Scr_AddInt(height, SCRIPTINSTANCE_CLIENT);
        Scr_AddInt(serverTime, SCRIPTINSTANCE_CLIENT);
        Scr_AddString(exitType, SCRIPTINSTANCE_CLIENT);
        Scr_AddInt(owner, SCRIPTINSTANCE_CLIENT);
        Scr_AddString(teamFaction, SCRIPTINSTANCE_CLIENT);
        Scr_AddString(team, SCRIPTINSTANCE_CLIENT);
        Scr_AddInt(yaw, SCRIPTINSTANCE_CLIENT);
        Scr_AddString(type, SCRIPTINSTANCE_CLIENT);
        Scr_AddFloat(z, SCRIPTINSTANCE_CLIENT);
        Scr_AddFloat(y, SCRIPTINSTANCE_CLIENT);
        Scr_AddFloat(x, SCRIPTINSTANCE_CLIENT);
        Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
        t = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_mp_data.airsupport, 0xCu);
        Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
    }
}

void __cdecl CG_ClearGenericFilter(cg_s *gameglob)
{
    int iADSParam; // [esp+0h] [ebp-18h]
    int i; // [esp+4h] [ebp-14h]
    int iOverlay; // [esp+8h] [ebp-10h]
    int iParam; // [esp+Ch] [ebp-Ch]
    int iPass; // [esp+10h] [ebp-8h]
    int iFilter; // [esp+14h] [ebp-4h]

    for ( iFilter = 0; iFilter < 3; ++iFilter )
    {
        for ( iPass = 0; iPass < 16; ++iPass )
        {
            gameglob->genericFilterPassEnabled[iFilter][iPass] = 0;
            gameglob->genericFilterPassMaterial[iFilter][iPass] = 0;
            gameglob->genericFilterPassTarget[iFilter][iPass] = 0;
            gameglob->genericFilterPassSampler0[iFilter][iPass] = 0;
            gameglob->genericFilterPassSampler1[iFilter][iPass] = 0;
            gameglob->genericFilterPassQuads[iFilter][iPass] = 0;
            gameglob->genericFilterPassFlareOcclusion[iFilter][iPass] = 0.0f;
            for ( iParam = 0; iParam < 16; ++iParam )
                gameglob->genericFilterPassParam[iFilter][iPass][iParam] = 0.0f;
        }
    }
    for ( iOverlay = 0; iOverlay < 3; ++iOverlay )
    {
        gameglob->genericOverlayMaterial[iOverlay] = 0;
        gameglob->genericOverlayEnabled[iOverlay] = 0;
        gameglob->genericOverlaySortIndex[iOverlay] = 0;
        for ( i = 0; i < 16; ++i )
            gameglob->genericOverlayParam[iOverlay][i] = 0.0f;
    }
    for ( iADSParam = 0; iADSParam < 8; ++iADSParam )
        gameglob->genericOverlayParamADS[iADSParam] = 0.0f;
    gameglob->genericOverlayEnableADS = 0;
}

void __cdecl CG_MapRestart(int localClientNum, int savepersist)
{
    unsigned __int16 v2; // [esp+0h] [ebp-14h]
    int clientIndex; // [esp+4h] [ebp-10h]
    unsigned __int16 t; // [esp+8h] [ebp-Ch]
    cg_s *cgameGlob; // [esp+Ch] [ebp-8h]
    cgs_t *cgs; // [esp+10h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    CG_ClearGenericFilter(cgameGlob);
    destroy_client_gjkcc_info(localClientNum);
    if ( cg_showmiss->current.integer )
        Com_Printf(14, "CG_MapRestart\n");
    CG_ClearCenterPrint(localClientNum);
    cgameGlob->cursorHintFade = 0;
    cgameGlob->lastHealthLerpDelay = 1;
    CG_ShutdownEntity(localClientNum, &cgameGlob->predictedPlayerEntity, 1);
    CG_InitLocalEntities(localClientNum);
    FX_KillAllEffects(localClientNum);
    FX_ShutdownSystem(localClientNum);
    DynEntCl_Shutdown(localClientNum);
    GlassCl_Reset(localClientNum);
    if ( Flame_GetLocalClientSourceRange() )
    {
        //BLOPS_NULLSUB();
        Phys_Init();
    }
    DynEntCl_InitEntities(localClientNum);
    //BLOPS_NULLSUB();
    R_InitPrimaryLights(cgameGlob->refdef.primaryLights);
    R_ClearShadowedPrimaryLightHistory(localClientNum);
    if ( CL_LocalClient_IsFirstActive(localClientNum) )
    {
        R_Stream_ResetHintEntities();
        R_PerMap_Init();
    }
    FX_InitSystem(localClientNum);
    CG_ClearEntityFxHandles(localClientNum);
    R_WaterSimulationRestart();
    if ( !Demo_IsPlaying() )
    {
        CG_VisionSetConfigString_Naked(localClientNum);
        CG_VisionSetConfigString_Night(localClientNum);
    }
    cgs->voteTime = 0;
    cgameGlob->mapRestart = 1;
    SND_GameReset();
    CG_SndGameReset();
    cgameGlob->v_dmg_time = 0;
    memset((unsigned __int8 *)cgameGlob->viewDamage, 0, sizeof(cgameGlob->viewDamage));
    CG_SetThirdPerson(0);
    CL_SetStance(localClientNum, CL_STANCE_STAND);
    CL_SetADS(localClientNum, 0);
    if ( !savepersist )
    {
        CG_CloseScriptMenu(localClientNum, 0);
        UI_CloseAllMenus(localClientNum);
        for ( clientIndex = 0; clientIndex < 32; ++clientIndex )
            cgameGlob->scoreOrder[clientIndex] = 32;
        cgameGlob->teamScores[0] = 0;
        cgameGlob->teamScores[1] = 0;
        cgameGlob->teamScores[2] = 0;
        cgameGlob->teamScores[3] = 0;
    }
    CG_ScoresUp(localClientNum);
    cgameGlob->objectiveText[0] = 0;
    CL_SyncTimes(localClientNum);
    if ( CL_LocalClient_IsFirstActive(localClientNum) )
    {
        R_Cinematic_StopPlayback();
        Scr_ShutdownSystem(SCRIPTINSTANCE_CLIENT, 1u, 0);
        Scr_InitSystem(SCRIPTINSTANCE_CLIENT, 1);
        Scr_SetLoading(1, SCRIPTINSTANCE_CLIENT);
        Scr_AllocGameVariable(SCRIPTINSTANCE_CLIENT);
        CScr_LoadStructs();
        CG_InitFakeEntities(localClientNum, 1);
        CScr_LoadLevel();
        Scr_SetLoading(0, SCRIPTINSTANCE_CLIENT);
        if ( cg_scr_data.maprestart )
        {
            v2 = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_data.maprestart, 0);
            Scr_FreeThread(v2, SCRIPTINSTANCE_CLIENT);
        }
    }
    else
    {
        CG_InitFakeEntities(localClientNum, 1);
    }
    Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
    t = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_data.localclientconnect, 1u);
    Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
    CG_InitBolt(localClientNum);
    CreateRopes(localClientNum);
    cgameGlob->poisoned = 0;
    cgameGlob->swimming = 0;
}

void __cdecl CG_ClearEntityFxHandles(int localClientNum)
{
    cg_s *cgameGlob; // [esp+8h] [ebp-14h]
    centity_s *cent; // [esp+Ch] [ebp-10h]
    int num; // [esp+14h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    for ( num = 0; num < cgameGlob->snap->numEntities; ++num )
    {
        cent = CG_GetEntity(localClientNum, cgameGlob->snap->entities[num].number);
        if ( cent->nextState.eType == 8 || cent->nextState.eType == 9 )
        {
            cent->pose.fx.effect = 0;
            cent->pose.fx.triggerTime = 0;
        }
    }
}

void __cdecl CG_CheckOpenWaitingScriptMenu(int localClientNum)
{
    if ( cg_waitingScriptMenu[localClientNum].name[0] )
    {
        if ( UI_PopupScriptMenu(
                     localClientNum,
                     cg_waitingScriptMenu[localClientNum].name,
                     cg_waitingScriptMenu[localClientNum].useMouse) )
        {
            cg_waitingScriptMenu[localClientNum].name[0] = 0;
        }
    }
}

void __cdecl CG_CloseScriptMenu(int localClientNum, bool allowResponse)
{
    UI_ClosePopupScriptMenu(localClientNum, allowResponse);
    cg_waitingScriptMenu[localClientNum].name[0] = 0;
}

void __cdecl CG_MenuShowNotify(int localClientNum, int menuToShow)
{
    cg_s *cgameGlob; // [esp+18h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    switch ( menuToShow )
    {
        case 0:
            if ( cgameGlob->healthFadeTime < cgameGlob->time )
            {
                cgameGlob->healthFadeTime = cgameGlob->time;
                if ( CL_LocalClient_GetActiveCount() == 1 )
                    Menus_ShowByName(&cgDC[localClientNum], "Health");
                else
                    Menus_ShowByName(&cgDC[localClientNum], "Health_mp");
            }
            break;
        case 1:
        case 5:
            if ( cgameGlob->ammoFadeTime < cgameGlob->time )
            {
                cgameGlob->ammoFadeTime = cgameGlob->time;
                if ( CL_LocalClient_GetActiveCount() == 1 )
                {
                    Menus_ShowByName(&cgDC[localClientNum], "weaponinfo");
                    Menus_ShowByName(&cgDC[localClientNum], "weaponinfo_lowdef");
                }
                else
                {
                    Menus_ShowByName(&cgDC[localClientNum], "weaponinfo_mp");
                }
            }
            if ( cgameGlob->offhandFadeTime < cgameGlob->time )
            {
                cgameGlob->offhandFadeTime = cgameGlob->time;
                if ( CL_LocalClient_GetActiveCount() == 1 )
                    Menus_ShowByName(&cgDC[localClientNum], "offhandinfo");
                else
                    Menus_ShowByName(&cgDC[localClientNum], "offhandinfo_mp");
            }
            break;
        case 3:
            if ( cgameGlob->compassFadeTime < cgameGlob->time )
            {
                cgameGlob->compassFadeTime = cgameGlob->time;
                if ( CL_LocalClient_GetActiveCount() == 1 )
                    Menus_ShowByName(&cgDC[localClientNum], "Compass");
                else
                    Menus_ShowByName(&cgDC[localClientNum], "Compass_mp");
            }
            break;
        case 4:
            if ( cgameGlob->stanceFadeTime < cgameGlob->time )
            {
                cgameGlob->stanceFadeTime = cgameGlob->time;
                if ( CL_LocalClient_GetActiveCount() == 1 )
                    Menus_ShowByName(&cgDC[localClientNum], "stance");
                else
                    Menus_ShowByName(&cgDC[localClientNum], "stance_mp");
            }
            break;
        case 6:
            if ( cgameGlob->scoreFadeTime < cgameGlob->time )
            {
                cgameGlob->scoreFadeTime = cgameGlob->time;
                Menus_ShowByName(&cgDC[localClientNum], "objectiveinfo");
            }
            break;
        case 7:
            if ( cgameGlob->sprintFadeTime < cgameGlob->time )
            {
                cgameGlob->sprintFadeTime = cgameGlob->time;
                if ( CL_LocalClient_GetActiveCount() == 1 )
                    Menus_ShowByName(&cgDC[localClientNum], "sprintMeter");
                else
                    Menus_ShowByName(&cgDC[localClientNum], "sprintMeter_mp");
            }
            break;
        case 8:
            if ( cgameGlob->vehicleControlsFadeTime < cgameGlob->time )
            {
                cgameGlob->vehicleControlsFadeTime = cgameGlob->time;
                Menus_ShowByName(&cgDC[localClientNum], "RCBombHints");
            }
            break;
        default:
            return;
    }
}

void __cdecl CG_ServerCommand(int localClientNum)
{
    CG_DeployServerCommand(localClientNum);
    Cmd_EndTokenizedString();
}

void __cdecl CG_DeployServerCommand(int localClientNum)
{
    const char *v1; // eax
    unsigned int v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    int v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    const char *v12; // eax
    const char *v13; // eax
    const char *v14; // eax
    const char *v15; // eax
    const char *v16; // eax
    const char *v17; // eax
    const char *v18; // eax
    char *v19; // eax
    const char *v20; // eax
    const char *v21; // eax
    unsigned int v22; // eax
    const char *v23; // eax
    int v24; // eax
    const char *v25; // eax
    int v26; // eax
    const char *v27; // eax
    int v28; // eax
    int v29; // eax
    int v30; // eax
    const char *v31; // eax
    const char *v32; // eax
    const char *v33; // eax
    const char *v34; // eax
    const char *v35; // eax
    const char *v36; // eax
    const char *v37; // eax
    const char *v38; // eax
    const char *v39; // eax
    const char *v40; // eax
    const char *v41; // eax
    int ControllerIndex; // eax
    const char *v43; // eax
    int v44; // eax
    const char *v45; // eax
    int v46; // eax
    const char *v47; // eax
    const char *v48; // eax
    int v49; // [esp+Ch] [ebp-280h]
    int v50; // [esp+1Ch] [ebp-270h]
    int v51; // [esp+24h] [ebp-268h]
    const char *v52; // [esp+28h] [ebp-264h]
    int v53; // [esp+28h] [ebp-264h]
    const char *v54; // [esp+28h] [ebp-264h]
    char *v55; // [esp+28h] [ebp-264h]
    float x; // [esp+2Ch] [ebp-260h]
    float y; // [esp+30h] [ebp-25Ch]
    float z; // [esp+34h] [ebp-258h]
    float v59; // [esp+44h] [ebp-248h]
    float v60; // [esp+48h] [ebp-244h]
    float v61; // [esp+4Ch] [ebp-240h]
    float v62; // [esp+5Ch] [ebp-230h]
    float v63; // [esp+60h] [ebp-22Ch]
    float v64; // [esp+64h] [ebp-228h]
    char team[4]; // [esp+A4h] [ebp-1E8h] BYREF
    char type[4]; // [esp+A8h] [ebp-1E4h] BYREF
    char teamFaction[4]; // [esp+ACh] [ebp-1E0h] BYREF
    char exitType[4]; // [esp+B0h] [ebp-1DCh] BYREF
    float pos[3]; // [esp+B4h] [ebp-1D8h] BYREF
    float dir[3]; // [esp+C0h] [ebp-1CCh] BYREF
    unsigned int drawType; // [esp+CCh] [ebp-1C0h]
    unsigned __int16 id; // [esp+D0h] [ebp-1BCh]
    char hudElemString[260]; // [esp+D4h] [ebp-1B8h] BYREF
    cg_s *cgameGlob; // [esp+1D8h] [ebp-B4h]
    const char *s; // [esp+1DCh] [ebp-B0h]
    int weapIndex; // [esp+1E0h] [ebp-ACh]
    int i; // [esp+1E4h] [ebp-A8h]
    const char *cmd; // [esp+1E8h] [ebp-A4h]
    char text[152]; // [esp+1ECh] [ebp-A0h] BYREF
    int argc; // [esp+288h] [ebp-4h]

    cmd = Cmd_Argv(0);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    switch ( *cmd )
    {
        case 0:
            return;
        case 0x28:
            CG_ElectrifiedServerCommand(localClientNum);
            break;
        case 0x29:
            v45 = Cmd_Argv(1);
            v46 = atoi(v45);
            CG_ScrCamera(localClientNum, v46);
            break;
        case 0x30:
            CG_WaterDropsServerCommand(localClientNum);
            break;
        case 0x38:
            v54 = Cmd_Argv(1);
            ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
            Live_GiveAchievement(ControllerIndex, v54);
            break;
        case 0x39:
            v55 = (char *)Cmd_Argv(2);
            v43 = Cmd_Argv(1);
            v44 = atoi(v43);
            CG_ParseClientSystemStateChange(localClientNum, v44, v55);
            break;
        case 0x42:
            CG_MapRestart(localClientNum, 0);
            break;
        case 0x43:
            v20 = Cmd_Argv(1);
            weapIndex = atoi(v20);
            if ( !weapIndex || BG_GetWeaponDef(weapIndex)->offhandClass )
                CG_SetEquippedOffHand(localClientNum, weapIndex);
            break;
        case 0x47:
            v23 = Cmd_Argv(1);
            v24 = atoi(v23);
            CG_SetTeamScore(localClientNum, 1u, v24);
            break;
        case 0x48:
            v25 = Cmd_Argv(1);
            v26 = atoi(v25);
            CG_SetTeamScore(localClientNum, 2u, v26);
            break;
        case 0x4A:
            v27 = Cmd_Argv(1);
            v28 = atoi(v27);
            CG_MenuShowNotify(localClientNum, v28);
            break;
        case 0x4B:
            v21 = Cmd_Argv(1);
            v22 = atoi(v21);
            CL_ResetPlayerMuting(v22);
            break;
        case 0x4C:
            UI_CloseInGameMenu(localClientNum);
            break;
        case 0x4E:
            v52 = Cmd_Argv(1);
            v29 = Com_LocalClient_GetControllerIndex(localClientNum);
            LiveStats_SetStatChanged(v29, v52);
            break;
        case 0x50:
            v30 = Com_LocalClient_GetControllerIndex(localClientNum);
            if ( !LB_UploadPlayerStats(v30) )
                Com_PrintError(
                    22,
                    "there was an error uploading player stats for local client on %i Demonware Leaderboard for PS3\n",
                    localClientNum);
            break;
        case 0x51:
            LiveStorage_UploadStats();
            break;
        case 0x52:
            v31 = Cmd_Argv(5);
            I_strncpyz(type, v31, 2);
            v32 = Cmd_Argv(6);
            I_strncpyz(teamFaction, v32, 2);
            v33 = Cmd_Argv(7);
            I_strncpyz(team, v33, 2);
            v34 = Cmd_Argv(9);
            I_strncpyz(exitType, v34, 2);
            v35 = Cmd_Argv(11);
            v53 = atoi(v35);
            v36 = Cmd_Argv(10);
            v51 = atoi(v36);
            v37 = Cmd_Argv(8);
            v50 = atoi(v37);
            v38 = Cmd_Argv(4);
            v49 = atoi(v38);
            v39 = Cmd_Argv(3);
            z = atof(v39);
            v40 = Cmd_Argv(2);
            y = atof(v40);
            v41 = Cmd_Argv(1);
            x = atof(v41);
            CG_ParseAirsupport(localClientNum, x, y, z, v49, type, teamFaction, team, v50, exitType, v51, v53);
            break;
        case 0x57:
            CG_BurnServerCommand(localClientNum);
            break;
        case 0x5B:
            CG_AnimateUI(localClientNum);
            break;
        case 0x61:
            v1 = Cmd_Argv(1);
            v2 = atoi(v1);
            CG_SelectWeaponIndex(localClientNum, &cgameGlob->nextSnap->ps, v2);
            break;
        case 0x63:
            v3 = Cmd_Argv(1);
            CG_TranslateHudElemMessage(localClientNum, v3, "announcement message", hudElemString);
            if ( Cmd_Argc() <= 2 )
            {
                CG_BoldGameMessage(localClientNum, hudElemString, 0);
            }
            else
            {
                v4 = Cmd_Argv(2);
                v5 = atoi(v4);
                CG_BoldGameMessage(localClientNum, hudElemString, v5);
            }
            break;
        case 0x64:
            CG_ConfigStringModified(localClientNum);
            break;
        case 0x65:
        case 0x66:
            v6 = Cmd_Argv(1);
            CG_TranslateHudElemMessage(localClientNum, v6, "game message", hudElemString);
            CG_GameMessage(localClientNum, hudElemString);
            break;
        case 0x67:
            v7 = Cmd_Argv(1);
            CG_TranslateHudElemMessage(localClientNum, v7, "bold game message", hudElemString);
            CG_BoldGameMessage(localClientNum, hudElemString, 0);
            break;
        case 0x68:
            if ( !cg_teamChatsOnly->current.enabled )
            {
                v8 = Cmd_Argv(1);
                s = SEH_LocalizeTextMessage(v8, "chat message", LOCMSG_SAFE);
                I_strncpyz(text, s, 150);
                CG_RemoveChatEscapeChar(text);
                CG_AddToTeamChat(localClientNum, text);
                Com_Printf(14, "%s\n", text);
            }
            break;
        case 0x69:
            v9 = Cmd_Argv(1);
            s = SEH_LocalizeTextMessage(v9, "team chat message", LOCMSG_SAFE);
            I_strncpyz(text, s, 150);
            CG_RemoveChatEscapeChar(text);
            CG_AddToTeamChat(localClientNum, text);
            Com_Printf(14, "%s\n", text);
            break;
        case 0x6A:
            v10 = Cmd_Argv(1);
            id = atoi(v10);
            v11 = Cmd_Argv(2);
            drawType = atoi(v11);
            v12 = Cmd_Argv(3);
            v62 = atof(v12);
            v13 = Cmd_Argv(4);
            v63 = atof(v13);
            v14 = Cmd_Argv(5);
            v64 = atof(v14);
            pos[0] = v62;
            pos[1] = v63;
            pos[2] = v64;
            v15 = Cmd_Argv(6);
            v59 = atof(v15);
            v16 = Cmd_Argv(7);
            v60 = atof(v16);
            v17 = Cmd_Argv(8);
            v61 = atof(v17);
            dir[0] = v59;
            dir[1] = v60;
            dir[2] = v61;
            DynEntCl_DestroyEvent(localClientNum, id, (DynEntityCollType)drawType, pos, dir);
            break;
        case 0x6B:
            LocalSoundStop(localClientNum);
            break;
        case 0x6E:
            CG_MapRestart(localClientNum, 1);
            break;
        case 0x73:
            if ( !LocalSound(localClientNum) )
                CL_DumpReliableCommands(localClientNum);
            break;
        case 0x74:
            CG_OpenScriptMenu(localClientNum);
            break;
        case 0x75:
            CG_CloseScriptMenu(localClientNum, 1);
            break;
        case 0x76:
            for ( i = 1; i < Cmd_Argc(); i += 2 )
            {
                v18 = Cmd_Argv(i);
                I_strncpyz(text, v18, 150);
                v19 = (char *)Cmd_Argv(i + 1);
                CG_SetClientDvarFromServer(cgameGlob, text, v19);
            }
            break;
        default:
            v47 = Cmd_Argv(0);
            Com_Printf(14, "Unknown client game command: %s\n", v47);
            argc = Cmd_Argc();
            if ( argc > 1 )
            {
                Com_Printf(14, "Arguments(%i):", argc - 1);
                for ( i = 1; i < argc; ++i )
                {
                    v48 = Cmd_Argv(i);
                    Com_Printf(14, " %s", v48);
                }
                Com_Printf(14, "\n");
            }
            break;
    }
}

void __cdecl CG_ConfigStringModified(int localClientNum)
{
    const char *v1; // eax
    shellshock_parms_t *ShellshockParms; // eax
    char *ConfigString; // eax
    char *str; // [esp+8h] [ebp-10h]
    cgs_t *cgs; // [esp+10h] [ebp-8h]
    int num; // [esp+14h] [ebp-4h]

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    CG_GetLocalClientGlobals(localClientNum);
    v1 = Cmd_Argv(1);
    num = atoi(v1);
    str = CL_GetConfigString(num);
    if ( num < 2836 || num > 3091 )
    {
        if ( num == 3147 )
        {
            CG_RegisterItems(localClientNum);
        }
        else if ( num )
        {
            if ( num < 23 || num >= 323 )
            {
                if ( num < 323 || num >= 355 )
                {
                    switch ( num )
                    {
                        case 7:
                            CG_ParseCullDist();
                            break;
                        case 14:
                            CG_ParseTimeScale(localClientNum);
                            break;
                        case 15:
                            cgs->voteTime = atoi(str);
                            break;
                        case 17:
                            cgs->voteYes = atoi(str);
                            break;
                        case 18:
                            cgs->voteNo = atoi(str);
                            break;
                        case 16:
                            CG_UpdateVoteString(localClientNum, str);
                            break;
                        case 13:
                            CL_ParseMapCenter();
                            break;
                        case 12:
                            CG_ParseGameEndTime(localClientNum);
                            break;
                        case 10:
                            CG_ParseFog(localClientNum);
                            break;
                        default:
                            if ( num < 1568 || num >= 2080 )
                            {
                                if ( num < 2080 || num >= 2276 )
                                {
                                    if ( num < 2532 || num >= 2548 )
                                    {
                                        if ( num >= 3092 && num < 3100 || num >= 3100 && num < 3115 )
                                        {
                                            ConfigString = CL_GetConfigString(num);
                                            Material_RegisterHandle(ConfigString, 7);
                                        }
                                        else if ( num < 2580 || num >= 2836 )
                                        {
                                            switch ( num )
                                            {
                                                case 1548:
                                                    CG_NorthDirectionChanged(localClientNum);
                                                    break;
                                                case 1549:
                                                    CG_MiniMapChanged(localClientNum);
                                                    break;
                                                case 1550:
                                                    CG_VisionSetConfigString_Naked(localClientNum);
                                                    break;
                                                case 1551:
                                                    CG_VisionSetConfigString_Night(localClientNum);
                                                    break;
                                            }
                                        }
                                        else
                                        {
                                            CG_RegisterServerMaterial(num);
                                        }
                                    }
                                    else if ( *str && BG_LoadShellShockDvars(str) )
                                    {
                                        ShellshockParms = BG_GetShellshockParms(num - 2532);
                                        BG_SetShellShockParmsFromDvars(ShellshockParms);
                                    }
                                }
                                else
                                {
                                    *((unsigned int *)cgs + num - 1403) = (unsigned int)FX_Register(str);
                                    if ( !*((unsigned int *)cgs + num - 1403)
                                        && !Assert_MyHandler(
                                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_servercmds_mp.cpp",
                                                    879,
                                                    0,
                                                    "%s",
                                                    "cgs->fxs[num - CS_EFFECT_NAMES]") )
                                    {
                                        __debugbreak();
                                    }
                                }
                            }
                            else
                            {
                                *((unsigned int *)cgs + num - 1403) = (unsigned int)R_RegisterModel(str);
                            }
                            break;
                    }
                }
                else
                {
                    CG_ParsePlayerInfos();
                }
            }
            else
            {
                CG_ParseCodInfo(localClientNum);
            }
        }
        else
        {
            CG_ParseServerInfo(localClientNum);
        }
    }
    else if ( !Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_servercmds_mp.cpp",
                             811,
                             0,
                             "%s\n\t%s",
                             "0",
                             "CG_ConfigStringModified:    We do not support modified weapon files") )
    {
        __debugbreak();
    }
}

void __cdecl CG_UpdateVoteString(int localClientNum, const char *rawVoteString)
{
    const char *v2; // eax
    int mapNameIndex; // [esp+0h] [ebp-120h]
    int loadMapNameLength; // [esp+4h] [ebp-11Ch] BYREF
    int srcIndex; // [esp+8h] [ebp-118h]
    cgs_t *cgs; // [esp+Ch] [ebp-114h]
    char voteStringWithLongMapNames[260]; // [esp+10h] [ebp-110h] BYREF
    const char *mapName; // [esp+118h] [ebp-8h]
    int dstIndex; // [esp+11Ch] [ebp-4h]

    dstIndex = 0;
    for ( srcIndex = 0; srcIndex < 256; ++srcIndex )
    {
        mapName = UI_GetMapDisplayNameFromPartialLoadNameMatch(&rawVoteString[srcIndex], &loadMapNameLength);
        if ( mapName )
        {
            for ( mapNameIndex = 0; srcIndex < 256 && mapName[mapNameIndex]; ++mapNameIndex )
                voteStringWithLongMapNames[dstIndex++] = mapName[mapNameIndex];
            srcIndex += loadMapNameLength;
        }
        voteStringWithLongMapNames[dstIndex++] = rawVoteString[srcIndex];
        if ( !rawVoteString[srcIndex] )
            break;
    }
    voteStringWithLongMapNames[255] = 0;
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    v2 = SEH_LocalizeTextMessage(voteStringWithLongMapNames, "vote string", LOCMSG_SAFE);
    I_strncpyz(cgs->voteString, v2, 256);
}

void __cdecl CG_AddToTeamChat(int localClientNum, const char *str)
{
    char *ls; // [esp+8h] [ebp-18h]
    int len; // [esp+Ch] [ebp-14h]
    cgs_t *cgs; // [esp+10h] [ebp-10h]
    char lastcolor; // [esp+14h] [ebp-Ch]
    char *p; // [esp+18h] [ebp-8h]
    char *pa; // [esp+18h] [ebp-8h]
    char *pb; // [esp+18h] [ebp-8h]
    int chatHeight; // [esp+1Ch] [ebp-4h]
    const char *stra; // [esp+2Ch] [ebp+Ch]

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    chatHeight = cg_chatHeight->current.integer;
    if ( chatHeight && cg_chatTime->current.integer > 0 )
    {
        len = 0;
        p = cgs->teamChatMsgs[cgs->teamChatPos % chatHeight];
        *p = 0;
        lastcolor = 55;
        ls = 0;
        while ( *str )
        {
            if ( len > 79 )
            {
                if ( ls )
                {
                    str = &str[-(p - ls) + 1];
                    p = ls;
                }
                *p = 0;
                cgs->teamChatMsgTimes[cgs->teamChatPos++ % chatHeight] = CG_GetLocalClientGlobals(localClientNum)->time;
                pa = cgs->teamChatMsgs[cgs->teamChatPos % chatHeight];
                *pa = 0;
                *pa = 94;
                pa[1] = lastcolor;
                p = pa + 2;
                len = 0;
                ls = 0;
            }
            if ( str && *str == 94 && str[1] && str[1] != 94 && str[1] >= 48 && str[1] <= 64 )
            {
                *p = *str;
                pb = p + 1;
                stra = str + 1;
                lastcolor = *stra;
                *pb = *stra;
                p = pb + 1;
                str = stra + 1;
            }
            else
            {
                if ( *str == 32 )
                    ls = p;
                *p++ = *str++;
                ++len;
            }
        }
        *p = 0;
        cgs->teamChatMsgTimes[cgs->teamChatPos++ % chatHeight] = CG_GetLocalClientGlobals(localClientNum)->time;
        if ( cgs->teamChatPos - cgs->teamLastChatPos > chatHeight )
            cgs->teamLastChatPos = cgs->teamChatPos - chatHeight;
    }
    else
    {
        cgs->teamLastChatPos = 0;
        cgs->teamChatPos = 0;
    }
}

void __cdecl CG_OpenScriptMenu(int localClientNum)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    int v4; // eax
    const char *v5; // eax
    char *menuName; // [esp+30h] [ebp-10h]
    unsigned int menuIndex; // [esp+34h] [ebp-Ch]
    bool useMouse; // [esp+3Bh] [ebp-5h]

    v1 = Cmd_Argv(1);
    menuIndex = atoi(v1);
    if ( menuIndex >= 0x20 )
    {
        Com_Printf(14, "Server tried to open a bad script menu index: %i\n", menuIndex);
        v2 = va("cmd mr %i bad\n", menuIndex);
        Cbuf_AddText(localClientNum, v2);
        return;
    }
    menuName = CL_GetConfigString(menuIndex + 2548);
    if ( !*menuName )
    {
        Com_Printf(14, "Server tried to open a non-loaded script menu index: %i\n", menuIndex);
        v3 = va("cmd mr %i bad\n", menuIndex);
        Cbuf_AddText(localClientNum, v3);
        return;
    }
    if ( Cmd_Argc() > 2 && Cmd_Argv(2) && *Cmd_Argv(2) )
    {
        useMouse = 0;
        v4 = UI_PopupScriptMenu(localClientNum, menuName, 0);
    }
    else
    {
        useMouse = 1;
        v4 = UI_PopupScriptMenu(localClientNum, menuName, 1);
    }
    if ( !v4 )
    {
        if ( cg_waitingScriptMenu[localClientNum].name[0] )
        {
            if ( !I_stricmp(menuName, cg_waitingScriptMenu[localClientNum].name) )
                return;
            v5 = va("cmd mr %i noop\n", cg_waitingScriptMenu[localClientNum].index);
            Cbuf_AddText(localClientNum, v5);
        }
        if ( strlen(menuName) >= 0x40
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_servercmds_mp.cpp",
                        1299,
                        0,
                        "%s\n\t(menuName) = %s",
                        "(I_strlen( menuName ) < (sizeof( cg_waitingScriptMenu[localClientNum].name ) / (sizeof( cg_waitingScriptMenu"
                        "[localClientNum].name[0] ) * (sizeof( cg_waitingScriptMenu[localClientNum].name ) != 4 || sizeof( cg_waiting"
                        "ScriptMenu[localClientNum].name[0] ) <= 4))))",
                        menuName) )
        {
            __debugbreak();
        }
        I_strncpyz(cg_waitingScriptMenu[localClientNum].name, menuName, 64);
        cg_waitingScriptMenu[localClientNum].index = menuIndex;
        cg_waitingScriptMenu[localClientNum].useMouse = useMouse;
    }
}

void __cdecl CG_RemoveChatEscapeChar(char *text)
{
    int l; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    l = 0;
    for ( i = 0; text[i]; ++i )
    {
        if ( text[i] != 25 )
            text[l++] = text[i];
    }
    text[l] = 0;
}

void __cdecl CG_SetTeamScore(int localClientNum, unsigned int team, int score)
{
    if ( team >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_servercmds_mp.cpp",
                    1500,
                    0,
                    "%s\n\t(team) = %i",
                    "(team >= 0 && team < TEAM_NUM_TEAMS)",
                    team) )
    {
        __debugbreak();
    }
    CG_GetLocalClientGlobals(localClientNum)->teamScores[team] = score;
}

char __cdecl LocalSound(int localClientNum)
{
    const char *v2; // eax
    int ClientNumForLocalClient; // eax
    unsigned int v4; // [esp+4h] [ebp-18h]
    int argc; // [esp+18h] [ebp-4h]

    argc = Cmd_Argc();
    if ( argc == 2 )
    {
        v2 = Cmd_Argv(1);
        v4 = atoi(v2);
        ClientNumForLocalClient = CG_GetClientNumForLocalClient(localClientNum);
        CG_PlaySound(localClientNum, ClientNumForLocalClient, 0, 0, 0, 1.0, v4);
        return 1;
    }
    else
    {
        Com_PrintError(9, "ERROR: LocalSound() called with %i args (should be 2)\n", argc);
        return 0;
    }
}

void __cdecl LocalSoundStop(int localClientNum)
{
    const char *v1; // eax
    cg_s *LocalClientGlobals; // eax
    unsigned int v3; // [esp-4h] [ebp-14h]

    if ( Cmd_Argc() == 2 )
    {
        v1 = Cmd_Argv(1);
        v3 = atoi(v1);
        LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
        CG_StopSoundAlias(localClientNum, LocalClientGlobals->nextSnap->ps.clientNum, v3);
    }
    else
    {
        Com_PrintError(9, "ERROR: LocalSoundStop(), should be called with 2 arguments.\n");
    }
}

void __cdecl CG_SetClientDvarFromServer(cg_s *cgameGlob, const char *dvarname, char *value)
{
    unsigned int v3; // eax

    if ( I_stricmp(dvarname, "cg_objectiveText") )
    {
        if ( I_stricmp(dvarname, "hud_drawHud") )
        {
            if ( I_stricmp(dvarname, "g_scriptMainMenu") )
                Dvar_SetFromStringByName(dvarname, value);
            else
                CG_SetScriptMainMenu(cgameGlob, value);
        }
        else
        {
            v3 = atoi(value);
            CG_SetDrawHud(cgameGlob, v3);
        }
    }
    else
    {
        CG_SetObjectiveText(cgameGlob, value);
    }
}

void __cdecl CG_SetObjectiveText(cg_s *cgameGlob, const char *text)
{
    I_strncpyz(cgameGlob->objectiveText, text, 1024);
}

void __cdecl CG_SetDrawHud(cg_s *cgameGlob, unsigned int value)
{
    if ( value >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_servercmds_mp.cpp",
                    1543,
                    0,
                    "%s\n\t(value) = %i",
                    "((value == 0 || value == 1))",
                    value) )
    {
        __debugbreak();
    }
    cgameGlob->drawHud = value;
}

void __cdecl CG_SetScriptMainMenu(cg_s *cgameGlob, const char *text)
{
    if ( !text
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_servercmds_mp.cpp", 1550, 0, "%s", "text") )
    {
        __debugbreak();
    }
    I_strncpyz(cgameGlob->scriptMainMenu, text, 256);
}

void __cdecl CG_BurnServerCommand(int localClientNum)
{
    const char *v1; // eax
    int time; // [esp+8h] [ebp-4h]

    if ( localClientNum >= 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_servercmds_mp.cpp",
                    1570,
                    0,
                    "%s",
                    "localClientNum < MAX_LOCAL_CLIENTS") )
    {
        __debugbreak();
    }
    v1 = Cmd_Argv(1);
    time = atoi(v1);
    if ( time )
        CG_Burn(localClientNum, time);
    else
        CG_ClearBurn(localClientNum);
}

void __cdecl CG_AnimateUI(int localClientNum)
{
    switch ( *Cmd_Argv(1) )
    {
        case 'a':
            CG_DisplayHudAnim(localClientNum);
            break;
        case 'b':
            CG_DisplayContract(localClientNum);
            break;
        case 'c':
            CG_DisplayGameModeMessage(localClientNum);
            break;
        case 'd':
            CG_DisplayMilestoneComplete(localClientNum);
            break;
        case 'e':
            CG_DisplayKillstreak(localClientNum);
            break;
        case 'f':
            if ( Dvar_GetBool("xblive_wagermatch") )
                CG_DisplayWagerMedal(localClientNum);
            else
                CG_DisplayMedal(localClientNum);
            break;
        case 'g':
            CG_DisplayRankUp(localClientNum);
            break;
        case 'h':
            CG_DisplayTeamMessage(localClientNum);
            break;
        case 'i':
            CG_ClearPopUpUI(localClientNum);
            break;
        case 'j':
            CG_DisplayWagerPopup(localClientNum);
            break;
        case 'k':
            CG_ClearEndGame(localClientNum);
            break;
        case 'l':
            CG_SetupMilestoneEndGame(localClientNum);
            break;
        case 'm':
            CG_SetupEndGame(localClientNum);
            break;
        case 'n':
            CG_ClearCenterPopUpUI(localClientNum);
            break;
        default:
            return;
    }
}

void __cdecl CG_ElectrifiedServerCommand(int localClientNum)
{
    const char *v1; // eax
    int time; // [esp+8h] [ebp-4h]

    if ( localClientNum >= 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_servercmds_mp.cpp",
                    1653,
                    0,
                    "%s",
                    "localClientNum < MAX_LOCAL_CLIENTS") )
    {
        __debugbreak();
    }
    v1 = Cmd_Argv(1);
    time = atoi(v1);
    if ( time )
        CG_Electrified(localClientNum, time);
    else
        CG_ClearElectrified(localClientNum);
}

void __cdecl CG_WaterDropsServerCommand(int localClientNum)
{
    const char *v1; // eax
    cg_s *cgameGlob; // [esp+8h] [ebp-8h]
    int count; // [esp+Ch] [ebp-4h]

    v1 = Cmd_Argv(1);
    count = atoi(v1);
    if ( count < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_servercmds_mp.cpp",
                    1671,
                    0,
                    "%s",
                    "count >= 0") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( count <= 0 )
    {
        cgameGlob->waterDropStopTime = cgameGlob->time;
    }
    else
    {
        cgameGlob->waterDropCount = count;
        cgameGlob->waterDropStartTime = cgameGlob->time;
        cgameGlob->waterDropStopTime = 0x7FFFFFFF;
    }
}

void __cdecl CG_ScrCamera(int localClientNum, int cmd)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    const char *v12; // eax
    ScriptCamera *cam; // [esp+30h] [ebp-4h]

    cam = &CG_GetLocalClientGlobals(localClientNum)->cameraData.scriptCam;
    cam->flags |= cmd;
    switch ( cmd )
    {
        case 0:
            cam->flags &= ~1u;
            break;
        case 2:
            cam->flags &= ~4u;
            v2 = Cmd_Argv(2);
            cam->origin[0] = atof(v2);
            v3 = Cmd_Argv(3);
            cam->origin[1] = atof(v3);
            v4 = Cmd_Argv(4);
            cam->origin[2] = atof(v4);
            v5 = Cmd_Argv(5);
            cam->angles[0] = atof(v5);
            v6 = Cmd_Argv(6);
            cam->angles[1] = atof(v6);
            v7 = Cmd_Argv(7);
            cam->angles[2] = atof(v7);
            break;
        case 4:
            cam->flags &= ~2u;
            v8 = Cmd_Argv(2);
            cam->originCent = atoi(v8);
            break;
        case 8:
            cam->flags &= ~0x10u;
            cam->flags &= ~0x20u;
            v9 = Cmd_Argv(2);
            cam->lookat[0] = atof(v9);
            v10 = Cmd_Argv(3);
            cam->lookat[1] = atof(v10);
            v11 = Cmd_Argv(4);
            cam->lookat[2] = atof(v11);
            break;
        case 16:
            cam->flags &= ~8u;
            cam->flags &= ~0x20u;
            v12 = Cmd_Argv(2);
            cam->lookAtCent = atoi(v12);
            break;
        case 32:
            cam->flags &= ~8u;
            cam->flags &= ~0x10u;
            break;
        default:
            return;
    }
}

void __cdecl CG_ExecuteNewServerCommands(int localClientNum, int latestSequence)
{
    cgs_t *cgs; // [esp+8h] [ebp-8h]
    int nesting; // [esp+Ch] [ebp-4h]

    nesting = Cmd_Args()->nesting;
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    while ( cgs->serverCommandSequence < latestSequence )
    {
        if ( !CG_GetLocalClientGlobals(localClientNum)->nextSnap
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_servercmds_mp.cpp",
                        2002,
                        0,
                        "%s",
                        "CG_GetLocalClientGlobals( localClientNum )->nextSnap") )
        {
            __debugbreak();
        }
        if ( CL_CGameNeedsServerCommand(localClientNum, ++cgs->serverCommandSequence) )
            CG_ServerCommand(localClientNum);
        if ( nesting != Cmd_Args()->nesting
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_servercmds_mp.cpp",
                        2008,
                        0,
                        "%s",
                        "nesting == Cmd_Args()->nesting") )
        {
            __debugbreak();
        }
    }
}

