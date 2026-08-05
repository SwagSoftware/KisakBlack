#include <Windows.h>

#include "win_gamerprofile.h"
#include <universal/dvar.h>
#include <universal/assertive.h>
#include <stringed/stringed_hooks.h>
#include <qcommon/com_clients.h>
#include <cstring>
#include <qcommon/cmd.h>
#include <gfx_d3d/r_dvars.h>
#include <sound/snd_dvar.h>
#include <qcommon/common.h>
#include "win_gamepad.h"
#include <client_mp/cl_main_mp.h>
#include <cgame_mp/cg_local_mp.h>
#include <live/live_win.h>
#include <ui/ui_playlists.h>
#include <client_mp/cl_input_mp.h>

const char *commonDvarList[19] =
{
  "gpad_buttonsConfig",
  "gpad_button_lstick_deflect_max",
  "gpad_button_rstick_deflect_max",
  "r_gamma",
  "snd_menu_voice",
  "snd_menu_music",
  "snd_menu_sfx",
  "snd_menu_master",
  "snd_menu_cinematic",
  "snd_menu_speaker_setup",
  "snd_menu_listen_level",
  "snd_menu_left_channel",
  "snd_menu_right_channel",
  "snd_menu_center_channel",
  "snd_menu_left_surround",
  "snd_menu_right_surround",
  "snd_menu_lfe",
  "demo_controllerConfig",
  NULL
};

GamerSettingState gamerSettings[1];
const char *mpDvarList[1];
const char *nonResetMPDvarList[1];
const char *commonXProfileDvarList[1];

extern const dvar_t *sv_botUseFriendNames; // KISAKTODO: remove later

static void __cdecl SetDvarFromLocString(int controllerIndex, const char *dvarName, char *preLocalizedText)
{
    char *localizedText; // [esp+0h] [ebp-4h]

    if ( !Dvar_IsValidName(dvarName)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                    414,
                    0,
                    "%s",
                    "Dvar_IsValidName( dvarName )") )
    {
        __debugbreak();
    }
    localizedText = SEH_LocalizeTextMessage(preLocalizedText, "dvar string", LOCMSG_NOERR);
    if ( localizedText && *localizedText )
        Dvar_SetCommand(dvarName, localizedText);
    else
        Dvar_SetCommand(dvarName, preLocalizedText);
}

static void __cdecl ResetCreateAClassNames(int controllerIndex)
{
    SetDvarFromLocString(controllerIndex, "customclass1", (char *)"CLASS_SLOT1_CAPS");
    SetDvarFromLocString(controllerIndex, "customclass2", (char *)"CLASS_SLOT2_CAPS");
    SetDvarFromLocString(controllerIndex, "customclass3", (char *)"CLASS_SLOT3_CAPS");
    SetDvarFromLocString(controllerIndex, "customclass4", (char *)"CLASS_SLOT4_CAPS");
    SetDvarFromLocString(controllerIndex, "customclass5", (char *)"CLASS_SLOT5_CAPS");
    SetDvarFromLocString(controllerIndex, "prestigeclass1", (char *)"CLASS_PRESTIGE1");
    SetDvarFromLocString(controllerIndex, "prestigeclass2", (char *)"CLASS_PRESTIGE2");
    SetDvarFromLocString(controllerIndex, "prestigeclass3", (char *)"CLASS_PRESTIGE3");
    SetDvarFromLocString(controllerIndex, "prestigeclass4", (char *)"CLASS_PRESTIGE4");
    SetDvarFromLocString(controllerIndex, "prestigeclass5", (char *)"CLASS_PRESTIGE5");
}

GamerSettingState *__cdecl GamerProfile_GetProfileSettings(int controllerIndex)
{
    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp",
                    345,
                    0,
                    "%s\n\t(controllerIndex) = %i",
                    "((controllerIndex >= 0) && (controllerIndex < 1))",
                    controllerIndex) )
    {
        __debugbreak();
    }
    return &gamerSettings[controllerIndex];
}

bool __cdecl GamerProfile_IsProfileLoggedIn(int controllerIndex)
{
    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp",
                    355,
                    0,
                    "%s\n\t(controllerIndex) = %i",
                    "((controllerIndex >= 0) && (controllerIndex < 1))",
                    controllerIndex) )
    {
        __debugbreak();
    }
    return gamerSettings[controllerIndex].isProfileLoggedIn;
}

bool __cdecl GamerProfile_WasErrorOnRead(int controllerIndex)
{
    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp",
                    365,
                    0,
                    "%s\n\t(controllerIndex) = %i",
                    "((controllerIndex >= 0) && (controllerIndex < 1))",
                    controllerIndex) )
    {
        __debugbreak();
    }
    return gamerSettings[controllerIndex].errorOnRead;
}

int __cdecl GamerProfile_GetPlaylistNum(int controllerIndex)
{
    return gamerSettings[controllerIndex].playlist;
}

void __cdecl GamerProfile_SetPlaylistNum(int controllerIndex, int playlistNum)
{
    gamerSettings[controllerIndex].playlist = playlistNum;
}

void __cdecl GamerProfile_ExecControllerBindings(int controllerIndex)
{
    char *v1; // eax
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    int localClientNum; // [esp+4h] [ebp-4h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp",
                    937,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    localClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
    if ( gamerSettings[controllerIndex].gpadEnabled )
    {
        if ( gamerSettings[controllerIndex].gpadButtonsConfig[0] )
        {
            v1 = strstr(gamerSettings[controllerIndex].gpadButtonsConfig, "buttons_");
            if ( v1 == gamerSettings[controllerIndex].gpadButtonsConfig )
            {
                v2 = va("exec %s\n", gamerSettings[controllerIndex].gpadButtonsConfig);
                Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, v2);
            }
        }
        if ( gamerSettings[controllerIndex].gpadSticksConfig[0] )
        {
            v3 = strstr(gamerSettings[controllerIndex].gpadSticksConfig, "thumbstick_");
            if ( v3 == gamerSettings[controllerIndex].gpadSticksConfig )
            {
                v4 = va("exec %s\n", gamerSettings[controllerIndex].gpadSticksConfig);
                Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, v4);
            }
        }
        Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, (char*)"updatevehiclebindings");
    }
}

void __cdecl GamerProfile_UpdateDvarsFromProfile(int controllerIndex)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *String; // eax
    const char *v4; // eax
    int i; // [esp+8h] [ebp-4h]
    int ia; // [esp+8h] [ebp-4h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp",
                    1088,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( !r_gamma
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp", 1089, 0, "%s", "r_gamma") )
    {
        __debugbreak();
    }
    if ( !snd_menu_master
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp",
                    1091,
                    0,
                    "%s",
                    "snd_menu_master") )
    {
        __debugbreak();
    }
    v1 = va("Updating dvars from profile %i.\n", controllerIndex);
    Com_Printf(16, "^3%s\n", v1);
    if ( gamerSettings[controllerIndex].isInitialized )
    {
        Dvar_SetInt((dvar_s *)com_first_time, gamerSettings[controllerIndex].firstTime);
        Dvar_SetBool((dvar_s *)zombietron_discovered, gamerSettings[controllerIndex].zombietron_discovered);
        Dvar_SetBool((dvar_s *)zombiefive_discovered, gamerSettings[controllerIndex].zombiefive_discovered);
        Dvar_SetBool((dvar_s *)gpad_enabled, gamerSettings[controllerIndex].gpadEnabled);
        Dvar_SetBool((dvar_s *)input_invertPitch, gamerSettings[controllerIndex].invertPitch);
        Dvar_SetFloat((dvar_s *)input_viewSensitivity, gamerSettings[controllerIndex].viewSensitivity);
        Dvar_SetInt((dvar_s *)team_indicator, gamerSettings[controllerIndex].team_indicator);
        Dvar_SetBool((dvar_s *)cg_mature, gamerSettings[controllerIndex].mature_content);
        Dvar_SetBool((dvar_s *)cg_blood, gamerSettings[controllerIndex].show_blood);
        Dvar_SetString((dvar_s *)gpad_buttonsConfig, gamerSettings[controllerIndex].gpadButtonsConfig);
        if ( gamerSettings[controllerIndex].gpadSticksConfig[0] )
            Dvar_SetString((dvar_s *)gpad_sticksConfig, gamerSettings[controllerIndex].gpadSticksConfig);
        Dvar_SetFloat((dvar_s *)gpad_button_lstick_deflect_max, gamerSettings[controllerIndex].gpadButtonLStickDeflect);
        Dvar_SetFloat((dvar_s *)gpad_button_rstick_deflect_max, gamerSettings[controllerIndex].gpadButtonRStickDeflect);
        Dvar_SetFloat((dvar_s *)r_gamma, gamerSettings[controllerIndex].gamma);
        Dvar_SetFloat((dvar_s *)snd_menu_voice, gamerSettings[controllerIndex].snd_menu_voice);
        Dvar_SetFloat((dvar_s *)snd_menu_music, gamerSettings[controllerIndex].snd_menu_music);
        Dvar_SetFloat((dvar_s *)snd_menu_sfx, gamerSettings[controllerIndex].snd_menu_sfx);
        Dvar_SetFloat((dvar_s *)snd_menu_master, gamerSettings[controllerIndex].snd_menu_master);
        Dvar_SetFloat((dvar_s *)snd_menu_cinematic, gamerSettings[controllerIndex].snd_cinematic_volume);
        Dvar_SetInt((dvar_s *)snd_menu_speaker_setup, gamerSettings[controllerIndex].snd_menu_speaker_setup);
        Dvar_SetInt((dvar_s *)snd_menu_listen_level, gamerSettings[controllerIndex].snd_menu_listen_level);
        Dvar_SetFloat((dvar_s *)snd_menu_left_channel, gamerSettings[controllerIndex].snd_menu_left_channel);
        Dvar_SetFloat((dvar_s *)snd_menu_right_channel, gamerSettings[controllerIndex].snd_menu_right_channel);
        Dvar_SetFloat((dvar_s *)snd_menu_center_channel, gamerSettings[controllerIndex].snd_menu_center_channel);
        Dvar_SetFloat((dvar_s *)snd_menu_left_surround, gamerSettings[controllerIndex].snd_menu_left_surround);
        Dvar_SetFloat((dvar_s *)snd_menu_right_surround, gamerSettings[controllerIndex].snd_menu_right_surround);
        Dvar_SetFloat((dvar_s *)snd_menu_lfe, gamerSettings[controllerIndex].snd_menu_lfe);
        Dvar_SetBool((dvar_s *)xblive_basictraining_popup, gamerSettings[controllerIndex].basicTrainingPopup);
        Dvar_SetInt((dvar_s *)bot_friends, gamerSettings[controllerIndex].friends);
        Dvar_SetInt((dvar_s *)bot_enemies, gamerSettings[controllerIndex].enemies);
        Dvar_SetInt((dvar_s *)bot_difficulty, gamerSettings[controllerIndex].difficulty);
        Dvar_SetBool((dvar_s *)bot_tips, gamerSettings[controllerIndex].tips);
        Dvar_SetBool((dvar_s *)sv_botUseFriendNames, gamerSettings[controllerIndex].gamertags);
        Dvar_SetInt((dvar_s *)demo_controllerConfig, gamerSettings[controllerIndex].demoControllerConfig);
        Com_ControllerIndex_GetLocalClientNum(controllerIndex);
        Dvar_SetString((dvar_s *)motd, gamerSettings[controllerIndex].motd);
        Dvar_SetBool((dvar_s *)cg_spectateThirdPerson, gamerSettings[controllerIndex].lastSpectatorSelectedThirdPerson);
        Dvar_SetInt((dvar_s *)playlist, gamerSettings[controllerIndex].playlist);
        Dvar_SetInt((dvar_s *)category, gamerSettings[controllerIndex].category);
        Dvar_SetInt((dvar_s *)categoryPlaylist, gamerSettings[controllerIndex].categoryPlaylist);
        Dvar_SetInt((dvar_s *)wagerCategory, gamerSettings[controllerIndex].wagerCategory);
        Dvar_SetInt((dvar_s *)wagerCategoryPlaylist, gamerSettings[controllerIndex].wagerCategoryPlaylist);
        Playlist_SetPrevPlaylist();
        if ( !gamerSettings[controllerIndex].customClass[0][0] )
        {
            ResetCreateAClassNames(controllerIndex);
            for ( i = 0; i < 10; ++i )
                I_strncpyz(gamerSettings[controllerIndex].customClass[i], customclass[i]->current.string, 16);
        }
        for ( ia = 0; ia < 10; ++ia )
            Dvar_SetString((dvar_s *)customclass[ia], gamerSettings[controllerIndex].customClass[ia]);
        Dvar_SetString((dvar_s *)clanName, gamerSettings[controllerIndex].clanPrefix);
        CL_SanitizeClanName();
        String = Dvar_GetString("clanName");
        I_strncpyz(gamerSettings[controllerIndex].clanPrefix, String, 5);
        v4 = va("GamerProfile_UpdateDvarsFromProfile(%i):", controllerIndex);
        DebugReportProfileDVars(v4);
    }
    else
    {
        v2 = va("Profile %i not initialized yet.\n", controllerIndex);
        Com_Printf(16, "^3%s\n", v2);
    }
}

const char *dvarNameList[10] =
{
  "mis_01",
  "mis_difficulty",
  "takeCoverWarnings",
  "r_gamma",
  "snd_menu_voice",
  "snd_menu_music",
  "snd_menu_sfx",
  "snd_menu_master",
  "snd_menu_cinematic",
  NULL
};

void __cdecl DebugReportProfileDVars(const char *headerMsg)
{
    const char *VariantString; // eax
    const char **listDvar; // [esp+0h] [ebp-4h]

    Com_Printf(16, "--------------\n%s\n", headerMsg);
    for ( listDvar = dvarNameList; *listDvar; ++listDvar )
    {
        VariantString = Dvar_GetVariantString(*listDvar);
        Com_Printf(16, " \"%s\" = \"%s\".\n", *listDvar, VariantString);
    }
}

void __cdecl SetDvarFromLocString_0(int controllerIndex, const char *dvarName, char *preLocalizedText)
{
    char *localizedText; // [esp+0h] [ebp-4h]

    if (!Dvar_IsValidName(dvarName)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp",
            961,
            0,
            "%s",
            "Dvar_IsValidName( dvarName )"))
    {
        __debugbreak();
    }
    localizedText = SEH_LocalizeTextMessage(preLocalizedText, "dvar string", LOCMSG_NOERR);
    if (localizedText && *localizedText)
        Dvar_SetCommand(dvarName, localizedText);
    else
        Dvar_SetCommand(dvarName, preLocalizedText);
}

void __cdecl GamerProfile_UpdateProfileFromDvars(int controllerIndex, profileWriteState_t profileWriteState)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    cg_s *cgameGlob; // [esp+Ch] [ebp-CE4h]
    int v6; // [esp+10h] [ebp-CE0h]
    int localClientNum; // [esp+14h] [ebp-CDCh]
    GamerSettingState oldSettings; // [esp+18h] [ebp-CD8h] BYREF
    bool writeProfile; // [esp+CEBh] [ebp-5h]
    int i; // [esp+CECh] [ebp-4h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp",
                    1248,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( !r_gamma
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp", 1249, 0, "%s", "r_gamma") )
    {
        __debugbreak();
    }
    if ( !snd_menu_master
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp",
                    1251,
                    0,
                    "%s",
                    "snd_menu_master") )
    {
        __debugbreak();
    }
    v2 = va("GamerProfile_UpdateProfileFromDvars(%i):", controllerIndex);
    DebugReportProfileDVars(v2);
    memcpy(&oldSettings, &gamerSettings[controllerIndex], sizeof(oldSettings));
    v3 = va("Updating profile %i from dvars.\n", controllerIndex);
    Com_Printf(16, "^3%s\n", v3);
    if ( I_stricmp(gpad_buttonsConfig->current.string, oldSettings.gpadButtonsConfig) )
    {
        localClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
        CL_ClearKeys(localClientNum);
    }
    gamerSettings[controllerIndex].firstTime = com_first_time->current.integer;
    gamerSettings[controllerIndex].zombietron_discovered = zombietron_discovered->current.enabled;
    gamerSettings[controllerIndex].zombiefive_discovered = zombiefive_discovered->current.enabled;
    gamerSettings[controllerIndex].gpadEnabled = gpad_enabled->current.enabled;
    gamerSettings[controllerIndex].invertPitch = input_invertPitch->current.enabled;
    gamerSettings[controllerIndex].viewSensitivity = input_viewSensitivity->current.value;
    gamerSettings[controllerIndex].team_indicator = team_indicator->current.integer;
    gamerSettings[controllerIndex].mature_content = cg_mature->current.enabled;
    gamerSettings[controllerIndex].show_blood = cg_blood->current.enabled;
    if ( _stricmp(gpad_buttonsConfig->current.string, "BUTTONS_ZOMBIETRON") )
        I_strncpyz(gamerSettings[controllerIndex].gpadButtonsConfig, gpad_buttonsConfig->current.string, 256);
    if ( !*(_BYTE *)gpad_buttonsConfig->current.integer )
        Dvar_SetString((dvar_s *)gpad_buttonsConfig, "buttons_default");
    if ( !*(_BYTE *)gpad_sticksConfig->current.integer )
        Dvar_SetString((dvar_s *)gpad_sticksConfig, "thumbstick_default");
    if ( Dvar_GetBool("gpad_enabled") )
    {
        if ( Dvar_GetBool("zombietron") )
        {
            Cmd_ExecuteSingleCommand(0, controllerIndex, (char*)"exec thumbstick_default.cfg\n");
            Cmd_ExecuteSingleCommand(0, controllerIndex, (char*)"exec buttons_default.cfg\n");
            Cmd_ExecuteSingleCommand(0, controllerIndex, (char*)"exec buttons_zombietron\n");
        }
        else
        {
            I_strncpyz(gamerSettings[controllerIndex].gpadSticksConfig, gpad_sticksConfig->current.string, 256);
            GamerProfile_ExecControllerBindings(controllerIndex);
            gamerSettings[controllerIndex].gpadButtonLStickDeflect = gpad_button_lstick_deflect_max->current.value;
            gamerSettings[controllerIndex].gpadButtonRStickDeflect = gpad_button_rstick_deflect_max->current.value;
            gamerSettings[controllerIndex].demoControllerConfig = demo_controllerConfig->current.integer;
        }
    }
    gamerSettings[controllerIndex].gamma = r_gamma->current.value;
    gamerSettings[controllerIndex].snd_menu_voice = snd_menu_voice->current.value;
    gamerSettings[controllerIndex].snd_menu_music = snd_menu_music->current.value;
    gamerSettings[controllerIndex].snd_menu_sfx = snd_menu_sfx->current.value;
    gamerSettings[controllerIndex].snd_menu_master = snd_menu_master->current.value;
    gamerSettings[controllerIndex].snd_cinematic_volume = snd_menu_cinematic->current.value;
    gamerSettings[controllerIndex].snd_menu_speaker_setup = snd_menu_speaker_setup->current.integer;
    gamerSettings[controllerIndex].snd_menu_listen_level = snd_menu_listen_level->current.integer;
    gamerSettings[controllerIndex].snd_menu_left_channel = snd_menu_left_channel->current.value;
    gamerSettings[controllerIndex].snd_menu_right_channel = snd_menu_right_channel->current.value;
    gamerSettings[controllerIndex].snd_menu_center_channel = snd_menu_center_channel->current.value;
    gamerSettings[controllerIndex].snd_menu_left_surround = snd_menu_left_surround->current.value;
    gamerSettings[controllerIndex].snd_menu_right_surround = snd_menu_right_surround->current.value;
    gamerSettings[controllerIndex].snd_menu_lfe = snd_menu_lfe->current.value;
    gamerSettings[controllerIndex].basicTrainingPopup = xblive_basictraining_popup->current.enabled;
    gamerSettings[controllerIndex].friends = bot_friends->current.integer;
    gamerSettings[controllerIndex].enemies = bot_enemies->current.integer;
    gamerSettings[controllerIndex].difficulty = bot_difficulty->current.integer;
    gamerSettings[controllerIndex].tips = bot_tips->current.enabled;
    gamerSettings[controllerIndex].gamertags = sv_botUseFriendNames->current.enabled;
    I_strncpyz(gamerSettings[controllerIndex].motd, motd->current.string, 256);
    gamerSettings[controllerIndex].lastSpectatorSelectedThirdPerson = cg_spectateThirdPerson->current.enabled;
    gamerSettings[controllerIndex].playlist = playlist->current.integer;
    gamerSettings[controllerIndex].category = category->current.integer;
    gamerSettings[controllerIndex].categoryPlaylist = categoryPlaylist->current.integer;
    gamerSettings[controllerIndex].wagerCategory = wagerCategory->current.integer;
    gamerSettings[controllerIndex].wagerCategoryPlaylist = wagerCategoryPlaylist->current.integer;
    if ( !*(_BYTE *)customclass[0]->current.integer )
        ResetCreateAClassNames(controllerIndex);
    for ( i = 0; i < 10; ++i )
        I_strncpyz(gamerSettings[controllerIndex].customClass[i], customclass[i]->current.string, 16);
    CL_SanitizeClanName();
    I_strncpyz(gamerSettings[controllerIndex].clanPrefix, clanName->current.string, 5);
    writeProfile = 0;
    writeProfile = gamerSettings[controllerIndex].loadoutDirty;
    if ( profileWriteState )
    {
        if ( profileWriteState == PROFILE_WRITE_IF_CHANGED )
        {
            if ( memcmp(&oldSettings, &gamerSettings[controllerIndex], 0xCC4u) )
                writeProfile = 1;
        }
        else
        {
            v4 = va("Invalid profileWriteState: %i", profileWriteState);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp", 1423, 0, v4) )
                __debugbreak();
        }
    }
    else
    {
        writeProfile = 0;
    }
    if ( gamerSettings[controllerIndex].isProfileLoggedIn && !gamerSettings[controllerIndex].errorOnRead )
    {
        if ( !writeProfile )
            writeProfile = GamerProfile_IsNewLocalProfile(controllerIndex);
        if ( writeProfile )
        {
            GamerProfile_SaveProfile(controllerIndex);
            gamerSettings[controllerIndex].loadoutDirty = 0;
        }
    }
    if ( cgArray )
    {
        v6 = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
        cgameGlob = CG_GetLocalClientGlobals(v6);
        if ( cgameGlob )
        {
            if ( cgameGlob->localClientNum == v6 && (cgameGlob->predictedPlayerState.eFlags & 0x4000) != 0 )
                cgameGlob->vehicleInitView = 1;
        }
    }
    gamerSettings[controllerIndex].isInitialized = 1;
}

void __cdecl GamerProfile_SaveProfile(int controllerIndex)
{
    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp",
                    825,
                    0,
                    "%s\n\t(controllerIndex) = %i",
                    "((controllerIndex >= 0) && (controllerIndex < 1))",
                    controllerIndex) )
    {
        __debugbreak();
    }
    if ( !gamerSettings[controllerIndex].isProfileLoggedIn
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp",
                    826,
                    0,
                    "%s",
                    "gamerSettings[controllerIndex].isProfileLoggedIn") )
    {
        __debugbreak();
    }
    if ( gamerSettings[controllerIndex].errorOnRead
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp",
                    827,
                    0,
                    "%s",
                    "!gamerSettings[controllerIndex].errorOnRead") )
    {
        __debugbreak();
    }
    GamerProfile_CreateExeBindingsConfig(controllerIndex);
    GamerProfile_CreateCommonBindingsConfig(controllerIndex);
    //BLOPS_NULLSUB();
}

void __cdecl GamerProfile_CreateExeBindingsConfig(int controllerIndex)
{
    const char *v1; // eax

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp",
                    538,
                    0,
                    "%s\n\t(controllerIndex) = %i",
                    "((controllerIndex >= 0) && (controllerIndex < 1))",
                    controllerIndex) )
    {
        __debugbreak();
    }
    if ( !gamerSettings[controllerIndex].isProfileLoggedIn
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp",
                    539,
                    0,
                    "%s",
                    "gamerSettings[controllerIndex].isProfileLoggedIn") )
    {
        __debugbreak();
    }
    v1 = va("GamerProfile_CreateExeBindingsConfig( %i ):", controllerIndex);
    DebugReportProfileDVars(v1);
    memset(
        (unsigned __int8 *)gamerSettings[controllerIndex].exeSettingsBuffer,
        0,
        sizeof(gamerSettings[controllerIndex].exeSettingsBuffer));
    GamerProfile_AppendDvarListToConfig(gamerSettings[controllerIndex].exeSettingsBuffer, 0x3E8u, mpDvarList);
    GamerProfile_AppendDvarListToConfig(gamerSettings[controllerIndex].exeSettingsBuffer, 0x3E8u, nonResetMPDvarList);
    GamerProfile_AppendDvarListToConfig(gamerSettings[controllerIndex].exeSettingsBuffer, 0x3E8u, customClassDvars);
    GamerProfile_AppendByteListToConfig(
        gamerSettings[controllerIndex].exeSettingsBuffer,
        0x3E8u,
        gamerSettings[controllerIndex].classLoadouts[0],
        0xAAu);
    GamerProfile_AppendByteListToConfig(
        gamerSettings[controllerIndex].exeSettingsBuffer,
        0x3E8u,
        gamerSettings[controllerIndex].killstreaks,
        3u);
    gamerSettings[controllerIndex].exeSettingsBuffer[strlen(gamerSettings[controllerIndex].exeSettingsBuffer)] = 0;
}

void __cdecl GamerProfile_AppendDvarListToConfig(char *buffer, unsigned int size, const char **dvarNameList)
{
    const char *v3; // eax
    char *stringToCopy; // [esp+50h] [ebp-21Ch]
    char tempString[260]; // [esp+54h] [ebp-218h] BYREF
    char *loc; // [esp+158h] [ebp-114h]
    char buf[256]; // [esp+15Ch] [ebp-110h] BYREF
    int len; // [esp+260h] [ebp-Ch]
    const char **dvarName; // [esp+264h] [ebp-8h]
    const dvar_s *dvar; // [esp+268h] [ebp-4h]

    if ( !buffer
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp", 399, 0, "%s", "buffer") )
    {
        __debugbreak();
    }
    if ( !dvarNameList
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp", 400, 0, "%s", "dvarNameList") )
    {
        __debugbreak();
    }
    loc = &buffer[strlen(buffer)];
    for ( dvarName = dvarNameList; *dvarName; ++dvarName )
    {
        dvar = Dvar_FindVar(*dvarName);
        if ( dvar )
        {
            stringToCopy = (char *)Dvar_DisplayableLatchedValue(dvar);
            if ( dvar == motd && strlen(stringToCopy) > 0x40 )
            {
                I_strncpyz(tempString, stringToCopy, 64);
                stringToCopy = tempString;
            }
            sprintf(buf, "set %s \"%s\"\n", dvar->name, stringToCopy);
            len = &buf[strlen(buf) + 1] - &buf[1];
            I_strncat(loc, len + 1, buf);
            loc += len;
        }
        else
        {
            Com_Printf(16, "Couldn't find profile dvar: %s.\n", *dvarName);
        }
    }
    if ( size <= strlen(buffer) )
    {
        v3 = va("sizeof( buffer ) = %i, strlen( buffer ) = %i\n", size, strlen(buffer));
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp",
                        430,
                        1,
                        "%s\n\t%s",
                        "size > strlen( buffer )",
                        v3) )
            __debugbreak();
    }
}

void __cdecl GamerProfile_AppendByteListToConfig(
                char *buffer,
                unsigned int bufSize,
                unsigned __int8 *data,
                unsigned __int16 dataSize)
{
    const char *v4; // eax
    unsigned int v5; // [esp+20h] [ebp-18h]
    unsigned __int16 existingSize; // [esp+34h] [ebp-4h]
    char *buffera; // [esp+40h] [ebp+8h]
    unsigned __int16 dataSizea; // [esp+4Ch] [ebp+14h]

    if ( !buffer
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp", 443, 0, "%s", "buffer") )
    {
        __debugbreak();
    }
    if ( !data
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp", 444, 0, "%s", "data") )
    {
        __debugbreak();
    }
    v5 = strlen(buffer);
    if ( v5 + dataSize + 2 >= bufSize
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp",
                    447,
                    0,
                    "%s",
                    "bufLen + dataSize + sizeof( dataSize ) < bufSize") )
    {
        __debugbreak();
    }
    buffera = &buffer[v5 + 1];
    existingSize = *(_WORD *)buffera;
    memcpy((unsigned __int8 *)&buffera[*(unsigned __int16 *)buffera + 2], data, dataSize);
    dataSizea = existingSize + dataSize;
    *(_WORD *)buffera = dataSizea;
    if ( bufSize <= strlen(buffera) + dataSizea + 2 )
    {
        v4 = va("sizeof( buffer ) = %i, strlen( buffer ) = %i\n", bufSize, strlen(buffera));
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp",
                        460,
                        1,
                        "%s\n\t%s",
                        "bufSize > strlen( buffer ) + dataSize + sizeof( dataSize )",
                        v4) )
            __debugbreak();
    }
}

void __cdecl GamerProfile_CreateCommonBindingsConfig(int controllerIndex)
{
    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp",
                    573,
                    0,
                    "%s\n\t(controllerIndex) = %i",
                    "((controllerIndex >= 0) && (controllerIndex < 1))",
                    controllerIndex) )
    {
        __debugbreak();
    }
    if ( !gamerSettings[controllerIndex].isProfileLoggedIn
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp",
                    574,
                    0,
                    "%s",
                    "gamerSettings[controllerIndex].isProfileLoggedIn") )
    {
        __debugbreak();
    }
    memset(
        (unsigned __int8 *)gamerSettings[controllerIndex].commonSettingsBuffer,
        0,
        sizeof(gamerSettings[controllerIndex].commonSettingsBuffer));
    GamerProfile_AppendDvarListToConfig(gamerSettings[controllerIndex].commonSettingsBuffer, 0x3E8u, commonDvarList);
    GamerProfile_AppendDvarListToConfig(
        gamerSettings[controllerIndex].commonSettingsBuffer,
        0x3E8u,
        commonXProfileDvarList);
    gamerSettings[controllerIndex].commonSettingsBuffer[strlen(gamerSettings[controllerIndex].commonSettingsBuffer)] = 0;
}

bool __cdecl GamerProfile_IsNewLocalProfile(int controllerIndex)
{
    return &gamerSettings[controllerIndex].exeSettingsBuffer[strlen(gamerSettings[controllerIndex].exeSettingsBuffer) + 1] == &gamerSettings[controllerIndex].exeSettingsBuffer[1]
            || &gamerSettings[controllerIndex].commonSettingsBuffer[strlen(gamerSettings[controllerIndex].commonSettingsBuffer)
                                                                                                                        + 1] == &gamerSettings[controllerIndex].commonSettingsBuffer[1];
}

void __cdecl GamerProfile_InitAllProfiles()
{
    memset(gamerSettings, 0, sizeof(gamerSettings));
    //BLOPS_NULLSUB();
    //BLOPS_NULLSUB();
    //BLOPS_NULLSUB();
    GamerProfile_UpdateProfileFromDvars(0, PROFILE_NO_WRITE);
}

void __cdecl GamerProfile_LogInProfile(int controllerIndex)
{
    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp",
                    1841,
                    0,
                    "%s\n\t(controllerIndex) = %i",
                    "((controllerIndex >= 0) && (controllerIndex < 1))",
                    controllerIndex) )
    {
        __debugbreak();
    }
    gamerSettings[controllerIndex].isProfileLoggedIn = 1;
    gamerSettings[controllerIndex].errorOnRead = 0;
}

void __cdecl GamerProfile_LogOutProfile(int controllerIndex)
{
    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_gamerprofile.cpp",
                    1861,
                    0,
                    "%s\n\t(controllerIndex) = %i",
                    "((controllerIndex >= 0) && (controllerIndex < 1))",
                    controllerIndex) )
    {
        __debugbreak();
    }
    Com_Printf(16, "Logging out gamerprofile on controller %i\n", controllerIndex);
    //BLOPS_NULLSUB();
    GamerProfile_UpdateProfileFromDvars(controllerIndex, PROFILE_NO_WRITE);
}

