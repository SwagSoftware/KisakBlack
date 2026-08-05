#include "com_expressions_eval.h"
#include "assertive.h"
#include <ui/ui_localvars.h>
#include <qcommon/com_clients.h>
#include <win32/win_shared.h>
#include <cgame_mp/cg_players_mp.h>
#include <client/cl_rank.h>
#include <client_mp/cl_ui_mp.h>
#include <bgame/bg_misc.h>
#include <client/splitscreen.h>
#include <live/live_win.h>
#include <cgame/cg_compass.h>
#include <client/client.h>
#include <bgame/bg_unlockable_items.h>
#include <live/live_fileshare.h>
#include <live/live_fileshare_search.h>
#include <live/live_counter.h>
#include <ui/ui_utils.h>
#include <client_mp/cl_ui_pc_mp.h>
#include <cgame/cg_ammocounter.h>
#include <cgame_mp/cg_consolecmds_mp.h>
#include <ui/ui_playlists.h>
#include <game_mp/g_main_mp.h>
#include <client/cl_console.h>
#include <live/live_friends_pc.h>
#include <ui/ui_emblem.h>
#include <ui_mp/ui_main_mp.h>
#include <demo/demo_playback.h>
#include <demo/demo_ui.h>
#include <ui_mp/ui_gametype_custom_mp.h>
#include <live/live_groups_dw.h>
#include <live/live_storage_win.h>
#include <live/live_meetplayer.h>
#include <live/live_contracts.h>
#include "com_tasks.h"
#include <live/live_clans.h>
#include <live/live_pcache_profile.h>
#include <client_mp/cl_main_pc_mp.h>
#include <win32/win_gamerprofile.h>
#include <live/live_storage_pub.h>
#include <ui/ui_feeders.h>
#include <cgame/cg_drawtools.h>
#include <stringed/stringed_hooks.h>

int s_indexToFunctionMap[1024];

void __cdecl GetToastPopupTitle(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    uiInfo_s *uiInfo; // [esp+8h] [ebp-4h]

    uiInfo = UI_GetInfo(localClientNum);
    result.dataType = VAL_STRING;
    result.internals.string = uiInfo->toastPopupTitle;
    if (uiscript_debug && uiscript_debug->current.integer)
        Expression_TraceInternal("GetToastPopupTitle() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void IsVisible_0(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *SourceString; // eax
    int UIContextIndex; // eax
    itemDef_s *v6; // [esp-4h] [ebp-18h]
    Operand result; // [esp+0h] [ebp-14h] BYREF
    Operand data; // [esp+8h] [ebp-Ch] BYREF
    itemDef_s *actualItem; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &data);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    SourceString = GetSourceString(data);
    actualItem = Menu_GetMatchingItemByNumber(item->parent, 0, SourceString);
    if (actualItem)
    {
        v6 = actualItem;
        UIContextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
        result.internals.intVal = Window_IsVisible(UIContextIndex, &v6->window);
    }
    AddOperandToStack(dataStack, &result);
}

void(__cdecl *rpnFunctions[480])(const int, itemDef_s *, OperandStack *) =
{
  &RPN_OP_NOOP,
  NULL,
  &RPN_OP_MULTIPLY,
  &RPN_OP_DIVIDE,
  &RPN_OP_MODULUS,
  &RPN_OP_ADD,
  &RPN_OP_SUBTRACT,
  &RPN_OP_NEGATE,
  &RPN_OP_NOT,
  &RPN_OP_LESSTHAN,
  &RPN_OP_LESSTHANEQUALTO,
  &RPN_OP_GREATERTHAN,
  &RPN_OP_GREATERTHANEQUALTO,
  &RPN_OP_EQUALS,
  &RPN_OP_NOTEQUAL,
  &RPN_OP_AND,
  &RPN_OP_OR,
  NULL,
  &RPN_OP_COMMA,
  &RPN_OP_BITWISEAND,
  &RPN_OP_BITWISEOR,
  &RPN_OP_BITWISENOT,
  &RPN_OP_BITSHIFTLEFT,
  &RPN_OP_BITSHIFTRIGHT,
  &RPN_FUNC_INVALID,
  &GetSinValue,
  &GetCosValue,
  &MinValue,
  &MaxValue,
  &GetMilliseconds,
  &GetDvarValue,
  &GetDvarIntValue,
  &GetDvarBoolValue,
  &GetDvarFloatValue,
  &GetDvarStringValue,
  &GetUIActive,
  &GetFlashbanged,
  &GetScoped,
  &GetScoreboardVisible,
  &InKillcam,
  &IsDualWield,
  &IsFuelWeapon,
  &GetPlayerField,
  &GetSelectingLocation,
  &GetTeamField,
  &GetOtherTeamField,
  &GetTeamMarinesField,
  &GetTeamOpForField,
  &IsMenuOpen,
  &WritingData,
  &InLobby,
  &InPrivateParty,
  &PrivatePartyHost,
  &PrivatePartyHostInLobby,
  &AloneInPrivateParty,
  &IsGroup,
  &AloneInLobby,
  &GetAdsJavelin,
  &GetWeapLockBlink,
  &GetWeapAttackTop,
  &GetWeapAttackDirect,
  &SecondsAsTimeDisplay,
  &TableLookup,
  &StatsTableLookup,
  &GetClassBonus,
  &LocalizeString,
  &GetLocalVarIntValue,
  &GetLocalVarBoolValue,
  &GetLocalVarFloatValue,
  &GetLocalVarStringValue,
  &GetTimeLeft,
  &SecondsAsCountdownDisplay,
  &GetGameMessageWindowActive,
  &RPN_FUNC_TOINT,
  &RPN_FUNC_TOSTRING,
  &RPN_FUNC_TOFLOAT,
  &GetGametypeName,
  &GetGametypeInternal,
  &GetGametypeObjective,
  &GetScore,
  &UI_GetOnlineFriendCount,
  &GetFollowing,
  &GetKeyBinding,
  &GetActionSlotUsable,
  &GetHudFade,
  &IsGroup,
  &IsGroup,
  &GameHost,
  &IsVisibilityBitSet,
  &Splitscreen,
  &SplitscreenHost,
  &SplitscreenNum,
  &IsCinematicFinished,
  &IsSelectingLocationalKillstreak,
  &IsSelectingAirstrike,
  &IsSelectingArtillery,
  &IsSelectingNapalm,
  &IsSelectingMortar,
  &IsSelectingComlink,
  &IsGroup,
  &IsGroup,
  &GetGameInvitesCount,
  &IsGroup,
  &GetPartyMissingMapPackError,
  &IsGroup,
  &GetIsIntermission,
  &GetIsSuperUser,
  &GetIsAutoJoinDevUser,
  &IsItemLocked,
  &IsItemDualWieldLocked,
  &IsItemDualWieldPurchased,
  &IsItemNew,
  &GetItemRef,
  &GetItemName,
  &GetItemImage,
  &GetItemUnlockLevel,
  &GetItemUnlockPLevel,
  &GetItemDesc,
  &GetItemIndex,
  &GetItemDualWieldIndex,
  &GetItemDualWieldBaseIndex,
  &GetItemCost,
  &GetItemDualWieldCost,
  &GetItemSellPrice,
  &GetItemCount,
  &GetItemGroup,
  &GetPlayerStatByName,
  &RPN_FUNC_ISCLANMEMBER,
  &RPN_FUNC_ISCLANMEMBER,
  &RPN_FUNC_ISCLANMEMBER,
  &GetClanDateFounded,
  &GetXUID,
  &GetSelfGamertag,
  &GetRankByXUID,
  &GetPrestigeByXUID,
  &GetCodpointsByXUID,
  &GetDisplayLevelByXUID,
  &GetClanDateFounded,
  &GetClanDateFounded,
  &GetClanDateFounded,
  &ClanMemberRankIsAtleast,
  &IsGroup,
  &GetClanName,
  &GetClanMOTD,
  &GetClanTagAndName,
  &IsGroup,
  &GetMutedStatus,
  &GetUIRect,
  &GetFeederData,
  &HasFocus,
  &IsVisible_0,
  &GetRank,
  &GetPrestige,
  &GetPlayerCardTitle,
  &GetDefaultClassSlot,
  &GetCacFactionNameWithButtons,
  &GetItemAttachment,
  &GetStatForFriendOrSelf,
  &GetSortedStatsForFriendOrSelf,
  &GetFloatAsFormattedString,
  &ChangeSortedStatsPivot,
  &CanScrollUpOrDown,
  &GetCurrentScrollBarPosition,
  &GetCombatRecordInfoBarWidth,
  &GetCombatRecordInfoBarText,
  &GetCombatRecordInfoBarTagText,
  &GetCombatRecordHistogramHeight,
  &GetCombatRecordPieChartText,
  &GetCombatRecordMinMaxScore,
  &GetCombatRecordFailedContracts,
  &GetCombatRecordLockedString,
  &GetNumWagerMatchesPlayed,
  &GetNumWagerMatchesWon,
  &GetAfterActionReportAwardsInfo,
  &GetNumPersonalBests,
  &GetPersonalBestName,
  &GetPersonalBestValue,
  &GetPersonalBestDelta,
  &GetPersonalBestPrefix,
  &GetNumWeaponUnlocks,
  &GetNumFeatureUnlocks,
  &GetUnlockedWeaponItemIndex,
  &GetUnlockedFeatureItemIndex,
  &GetNumStatsMilestones,
  &GetStatsMilestoneValue,
  &GetStatsMilestoneName,
  &GetCurrentChallengeXpReward,
  &GetCurrentChallengeCpReward,
  &GetCurrentChallengeProgress,
  &IsCurrentChallengeItemClassified,
  &GetNumChallenges,
  &GetTotalMatchesPlayed,
  &GetCopyClassConfirmationText,
  &GetCopyClassDialogTitle,
  &IsFileshareDataSummaryValid,
  &GetFileshareRecentGamesCount,
  &GetFileshareGameMap,
  &GetFileshareGameMapName,
  &GetFileshareGameType,
  &GetFileshareGameTypeName,
  &GetFileshareGameDate,
  &GetFileshareFileName,
  &GetFileshareFileId,
  &GetFileshareFileSize,
  &GetImageWidth,
  &GetTextWidth,
  &GetTextHeight,
  &GetComposite,
  &IsCompositeValid,
  &IsSignedIn,
  &IsProfileSignedIn,
  &GetToastPopupWidth,
  &GetToastPopupIcon,
  &GetToastPopupTitle,
  &GetToastPopupDescription,
  &GetNumLives,
  &PlayersAlive,
  &GetPlaylistMaxPartySize,
  &GetGameTime,
  &GetBombTime,
  &CanSpecCycle,
  &CanSpecFree,
  &GetDStat,
  &GetDStatForPreviousMatch,
  &IsStableStatsBufferInitialized,
  &GetClientInPlace,
  &GetClientName,
  &ToOrdinal,
  &GetScoreForClient,
  &IsDemoPlaying,
  &IsDemoClipRecording,
  &IsDemoClipPlaying,
  &IsDemoMovieRendering,
  &IsDemoThirdPersonCamera,
  &IsDemoMovieCamera,
  &IsDisplayingPartyScoreboard,
  &GetDemoTitleName,
  &GetDemoTitleDescription,
  &GetDemoAuthor,
  &GetDemoTimeInfo,
  &GetDemoDuration,
  &GetDemoSegmentCount,
  &GetDemoSegmentInformation,
  &IsClipModified,
  &CanStartDemoPlayback,
  &GetDemoSaveScreenName,
  &GetDemoSaveScreenDescription,
  &GetTheaterFilmNotSelectedMessage,
  &IsItemPurchased,
  &IsCurrentItemPurchased,
  &IsCurrentItemAttachmentPurchased,
  &IsCurrentItemOptionPurchased,
  &IsItemAttachmentPointPurchased,
  &GetCurrentItemCost,
  &GetCurrentItemSellPrice,
  &GetCurrentItemAttachmentCost,
  &GetCurrentItemName,
  &GetCurrentItemIndex,
  &GetCurrentItemAttachmentName,
  &GetCurrentItemAttachmentDesc,
  &GetCurrentItemNumAttachments,
  &GetCurrentItemAttachmentNum,
  &GetCurrentItemAttachmentPoint,
  &GetItemNumAttachmentsEquipped,
  &GetItemEquippedAttachment,
  &PlaylistPlayerCount,
  &CategoryPlayerCountForPlaylist,
  &TotalPlayersInPlaylists,
  &GetPlayersRegisteredOnline,
  &GetPreviousGameType,
  &GetLeaderboardMinReqText,
  &GetBaseLbMenuName,
  &GetPreviousGameType,
  &GetPreviousGameType,
  &GetPreviousGameType,
  &IsGroup,
  &IsExtraCamActive,
  &GetCurrentWeapon,
  &ShowZombieMap,
  &GetAttachmentName,
  &GetAttachmentImage,
  &GetAttachmentDesc,
  &GetWeaponOptionImage,
  &GetWeaponOptionName,
  &IsPlayerJoinable,
  &IsPlayerInvitable,
  &ClanMemberRankIsAtleast,
  &IsExtraCamStatic,
  &GetNumItemAttachmentsWithAttachPoint,
  &GetCurrentItemOption,
  &GetWeaponOptionCost,
  &IsItemGroupNew,
  &Select,
  &Choose,
  &HasTacticalMaskOverlay,
  &InVehicle,
  &GetNumActiveContracts,
  &GetCurrentContractIndex,
  &GetContractName,
  &GetContractDesc,
  &GetContractProgress,
  &GetContractRequiredCount,
  &GetContractCost,
  &IsContractActive,
  &GetIndexForActiveContract,
  &GetIndexForNthActiveContract,
  &IsContractInProgress,
  &IsContractExpired,
  &IsContractComplete,
  &GetContractCombatTimeLeft,
  &MenuHasFocus,
  &GetCustomClassLoadoutItem,
  &GetCustomClassModifier,
  &GetCustomClassName,
  &GetCACItemIndex,
  &GetMachineID,
  &ApproxEquals,
  &GetLeaderboardValue,
  &GetStatValue,
  &AreStatsFetched,
  &IsSignedInToLive,
  &GetLocalClientNum,
  &IsPrimaryLocalClient,
  &GetSellText,
  &GetItemPrice,
  &IsGroup,
  &IsFriendFromXuid,
  &RandomIntRange,
  &GetSelectedEmblemLayer,
  &IsGroup,
  &IsGroup,
  &GetWeaponOptionGroupIndex,
  &GetDownloadProgress,
  &GetUploadProgress,
  &GetUploadTimeRemaining,
  &IsCurrentSortedItemEquipped,
  &IsCurrentItemEquippedInAnyCustomClass,
  &IsItemEquipped,
  &GetFeederCount,
  &IsCurrentItemClassified,
  &GetCurrentItemClassifiedHintText,
  &AreContractsFetched,
  &IsTimeSynced,
  &IsContractLocked,
  &GetTimesContractPurchased,
  &GetTimesContractPurchasable,
  &GetContractCooldownTime,
  &GetRemainingContractCooldownTime,
  &GetContractUnlockLevel,
  &GetContractRewardText,
  &GetContractExpirationType,
  &GetContractExpirationData,
  &IsTaskInProgress,
  &GetPlaceWithTiesForScore,
  &GetIsSuperUser,
  &isInGuidedMissile,
  &IsInGame,
  &EmblemLayerState,
  &EmblemLayerName,
  &EmblemLayerCanOutline,
  &EmblemLayerCanDuplicate,
  &EmblemSelectedLayer,
  &EmblemLayerCost,
  &EmblemLayerUnlockLevel,
  &EmblemIconName,
  &EmblemIconUnlockDesc,
  &EmblemIconName,
  &EmblemIconCost,
  &EmblemIconState,
  &EmblemIsModified,
  &EmblemPurchasedLayerCount,
  &EmblemBackgroundCount,
  &EmblemBackgroundIsLocked,
  &EmblemBackgroundIsClassified,
  &EmblemBackgroundIsPurchased,
  &EmblemBackgroundMaterial,
  &EmblemPlayerBackgroundMaterial,
  &EmblemSelectedBackground,
  &EmblemBackgroundCost,
  &EmblemBackgroundName,
  &EmblemBackgroundUnlockDesc,
  &EmblemStateDisplay,
  &EmblemCategoryDisplay,
  &EmblemFilterCount,
  &EmblemFilterIconID,
  &ItemHasDualWield,
  &ItemIsDualWield,
  &GetDemoFileID,
  &GetFileShareRating,
  &GetAutoJoinLobbyStatus,
  &GetCounterTotal,
  &ShowBusyDotsIndicator,
  &CanRateFilmInTheater,
  &GetAttachmentsFormatted,
  &IsClanTagFeatureLocked,
  &IsClanTagFeaturePurchased,
  &GetCurrentClanTagFeature,
  &GetClanTagFeatureCost,
  &GetClanTagFeatureName,
  &IsItemOptionPurchasedByName,
  &GetItemOptionByName,
  &GetFaceCamoIndex,
  &HowManyReadiesNeeded,
  &GetLiveGroupCount,
  &GetUserTagFromIndex,
  &GetFileShareFilterList,
  &CanRenderClip,
  &CanShowContentFromUser,
  &IsContentRatingAllowed,
  &IsDemonwareFetchingDone,
  &GetIndexIntoMatchScoreboard,
  &GetWagerPlaceForMatchScoreboard,
  &GetWagerGametypeNameFromEnum,
  &GetClanMOTD,
  &AnySignedIn,
  &AnySignedInToLiveAndStatsFetched,
  &AnySignedInToLiveAndStatsFetched,
  &IsProItemVersionUnlocked,
  &IsProItemVersion,
  &GetFlagCarrierForTeam,
  &GetFlagStatusForTeam,
  &GetChallengeProgressString,
  &GetChallengeName,
  &GetName,
  &GetProgressString,
  &GetXpReward,
  &GetCpReward,
  &GetDescription,
  &GetChallengeDescription,
  &ToUpper,
  &GetPlaylistName,
  &GetTimeUntilNewContracts,
  &NeedToPerformCommunitySearch,
  &GetLbTypeWithButtons,
  &GetLBFilter,
  &GetLBTypeByDuration,
  &GetScoreboardColumnHeader,
  &GetProItemVersionCost,
  &GetNumSortedItemsEquipped,
  &GetGameInvitesCount,
  &GetMySlotInfo,
  &IsAttachmentAllowedOnItemIndex,
  &IsGroup,
  &GetFileShareTotalVotes,
  &GridMove,
  &GetPooledFileDetails,
  &GetClanTagFeaturePlevel,
  &GetWeaponOptionUnlockLvl,
  &GetWeaponOptionUnlockPLevel,
  &Add64,
  &Sub64,
  &Div64,
  &Mul64,
  &IsPremiumSubscriber,
  &GetUserFileRating,
  &HostMigrationWaitingForPlayers,
  &IsItemUnlocked,
  &IsWeaponItemUnlocked,
  &IsWeaponItemPurchased,
  &IsWeaponItemClassified,
  &GetWeaponName,
  &IsGroup,
  &CanSwitchToLobby,
  &GetMapIndexByName,
  &GetGamemodeIndexByName,
  &AloneInPrivatePartyIgnoreSplitscreen,
  &AloneInLobbyIgnoreSplitscreen,
  &ServerSort,
  &ServerSortDirection,
  &IsViewportLarge,
  &IsChallengeItemPurchased,
  &GetChallengeAttachmentName,
  &IsItemLockedForAll,
  &GetServerCounts,
  &GetLowestLocalCP,
  &IsWagerServer,
  &GetContextHeight,
  &GetCurrentItemMultiText,
  &GetCopyCustomGametypeClassConfirmationText,
  &GetCopyCustomGametypeClassDialogTitle
};



int s_functionToIndexMap[1024] =
{
  -1,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0
};

int currentTempOperand;
char s_tempOperandValueAsString[32][256];
char localVarStrings[4][32];

int localVarStringIndex;
int secondsRemaining;
int lastPoll;
char resultString[256];
char resultString_0[128];
char resultString_1[128];
int endIndex_0;
int startIndex_0;
int currIndex_0;


char __cdecl GetTwoOperands(OperandStack *dataStack, Operand *data1, Operand *data2)
{
    GetOperand(dataStack, data2);
    GetOperand(dataStack, data1);
    return 1;
}

char __cdecl GetOperand(OperandStack *dataStack, Operand *data)
{
    operandInternalDataUnion v2; // edx
    operandInternalDataUnion v4; // eax
    OperandList *list; // [esp+0h] [ebp-4h]

    if ( dataStack->numOperandLists >= 1 )
    {
        list = &dataStack->stack[dataStack->numOperandLists - 1];
        if ( list->operandCount == 1 )
        {
            v4.intVal = list->operands[0].internals.intVal;
            data->dataType = list->operands[0].dataType;
            data->internals = v4;
            --dataStack->numOperandLists;
            if ( data->dataType == VAL_STRING
                && !data->internals.intVal
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                            191,
                            0,
                            "%s",
                            "data->dataType != VAL_STRING || data->internals.string") )
            {
                __debugbreak();
            }
            return 1;
        }
        else
        {
            Expression_Error(
                " Invalid operand count - expected to find one operand but instead found %i\n",
                list->operandCount);
            data->dataType = VAL_INT;
            data->internals.intVal = 0;
            return 0;
        }
    }
    else
    {
        Expression_Error(" Invalid operation - missing parameter inside function or parenthesis\n");
        dataStack->numOperandLists = 1;
        dataStack->stack[0].operandCount = 1;
        v2.intVal = (int)dataStack->stack[0].operands[0].internals;
        data->dataType = dataStack->stack[0].operands[0].dataType;
        data->internals = v2;
        data->dataType = VAL_INT;
        data->internals.intVal = 0;
        return 0;
    }
}

char __cdecl GetOperandList(OperandStack *dataStack, OperandList *list)
{
    if ( dataStack->numOperandLists >= 1 )
    {
        memcpy(list, &dataStack->stack[--dataStack->numOperandLists], sizeof(OperandList));
        return 1;
    }
    else
    {
        Expression_Error(" Invalid operation - missing parameter inside function or parenthesis\n");
        dataStack->numOperandLists = 1;
        memcpy(list, dataStack, sizeof(OperandList));
        list->operandCount = 1;
        list->operands[0].dataType = VAL_INT;
        list->operands[0].internals.intVal = 0;
        return 0;
    }
}

char *__cdecl GetSourceString(Operand operand)
{
    char *result; // [esp+8h] [ebp-4h]

    if ( operand.dataType == VAL_STRING )
        return (char *)operand.internals.intVal;
    if ( (unsigned int)currentTempOperand >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    274,
                    0,
                    "currentTempOperand doesn't index NUM_OPERAND_STRINGS\n\t%i not in [0, %i)",
                    currentTempOperand,
                    32) )
    {
        __debugbreak();
    }
    result = s_tempOperandValueAsString[currentTempOperand];
    currentTempOperand = (currentTempOperand + 1) % 32;
    if ( operand.dataType )
    {
        if ( operand.dataType == VAL_FLOAT )
            Com_sprintf(result, 0x100u, "%f", operand.internals.floatVal);
    }
    else
    {
        I_itoa(operand.internals.intVal, result, 256);
    }
    return result;
}

double __cdecl GetSourceFloat(Operand *source)
{
    if ( source->dataType == VAL_FLOAT )
        return source->internals.floatVal;
    if ( source->dataType == VAL_INT )
        return (double)source->internals.intVal;
    if ( source->dataType != VAL_STRING
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    303,
                    0,
                    "%s",
                    "source->dataType == VAL_STRING") )
    {
        __debugbreak();
    }
    return (float)atof(source->internals.string);
}

operandInternalDataUnion __cdecl GetSourceInt(Operand *source)
{
    if ( source->dataType == VAL_FLOAT )
    {
        return (operandInternalDataUnion)(int)source->internals.floatVal;
    }
    else if ( source->dataType )
    {
        if ( source->dataType != VAL_STRING
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                        314,
                        0,
                        "%s",
                        "source->dataType == VAL_STRING") )
        {
            __debugbreak();
        }
        return (operandInternalDataUnion)atoi(source->internals.string);
    }
    else
    {
        return source->internals;
    }
}

void __cdecl GetDvarValue(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *NameForValueType; // eax
    const char *v4; // eax
    const char *name; // [esp+0h] [ebp-18h]
    Operand source; // [esp+4h] [ebp-14h] BYREF
    Operand result; // [esp+Ch] [ebp-Ch] BYREF
    const dvar_s *dvar; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    if ( source.dataType == VAL_STRING )
    {
        dvar = Dvar_FindVar(source.internals.string);
    }
    else
    {
        if ( source.dataType )
        {
            NameForValueType = GetNameForValueType(source.dataType);
            Expression_Error("Must use a string as the name of a dvar, not a %s\n", NameForValueType);
            result.dataType = VAL_STRING;
            result.internals.intVal = (int)"";
            AddOperandToStack(dataStack, &result);
            return;
        }
        dvar = Dvar_FindVar(source.internals.intVal);
    }
    result.dataType = VAL_STRING;
    if ( dvar )
    {
        if ( dvar->type )
        {
            switch ( dvar->type )
            {
                case DVAR_TYPE_FLOAT:
                    result.dataType = VAL_FLOAT;
                    result.internals.intVal = dvar->current.integer;
                    break;
                case DVAR_TYPE_INT:
                    result.dataType = VAL_INT;
                    result.internals.intVal = dvar->current.integer;
                    break;
                case DVAR_TYPE_STRING:
                    result.internals.intVal = (int)CopyTempString(dvar->current.string);
                    break;
                default:
                    result.internals.intVal = (int)Dvar_DisplayableValue(dvar);
                    break;
            }
        }
        else
        {
            result.dataType = VAL_INT;
            result.internals.intVal = dvar->current.color[0] != 0;
        }
    }
    else
    {
        result.internals.intVal = (int)"";
    }
    AddOperandToStack(dataStack, &result);
    if ( uiscript_debug && uiscript_debug->current.integer )
    {
        if ( dvar )
            name = dvar->name;
        else
            name = "NULL";
        v4 = Dvar_DisplayableValue(dvar);
        Expression_TraceInternal("dvar( %s ) = %s\n", name, v4);
    }
}

void __cdecl AddOperandToStack(OperandStack *dataStack, Operand *data)
{
    operandInternalDataUnion v2; // eax
    int numOperandLists; // ecx

    if ( data->dataType == VAL_STRING
        && !data->internals.intVal
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    225,
                    0,
                    "%s",
                    "data->dataType != VAL_STRING || data->internals.string") )
    {
        __debugbreak();
    }
    v2.intVal = (int)data->internals;
    numOperandLists = dataStack->numOperandLists;
    dataStack->stack[numOperandLists].operands[0].dataType = data->dataType;
    dataStack->stack[numOperandLists].operands[0].internals = v2;
    dataStack->stack[dataStack->numOperandLists++].operandCount = 1;
}

char *__cdecl CopyTempString(const char *string)
{
    char *result; // [esp+0h] [ebp-4h]

    if ( (unsigned int)currentTempOperand >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    289,
                    0,
                    "currentTempOperand doesn't index NUM_OPERAND_STRINGS\n\t%i not in [0, %i)",
                    currentTempOperand,
                    32) )
    {
        __debugbreak();
    }
    result = s_tempOperandValueAsString[currentTempOperand];
    currentTempOperand = (currentTempOperand + 1) % 32;
    Com_sprintf(result, 0x100u, "%s", string);
    return result;
}

const char *__cdecl GetNameForValueType(expDataType valType)
{
    const char *v2; // eax

    switch ( valType )
    {
        case VAL_INT:
            return "Int";
        case VAL_FLOAT:
            return "Float";
        case VAL_STRING:
            return "String";
    }
    v2 = va("Unknown value type %i", valType);
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp", 329, 0, v2) )
        __debugbreak();
    return "";
}

void __cdecl GetDvarStringValue(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *NameForValueType; // eax
    Operand source; // [esp+4h] [ebp-14h] BYREF
    Operand result; // [esp+Ch] [ebp-Ch] BYREF
    const dvar_s *dvar; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    if ( source.dataType == VAL_STRING )
    {
        dvar = Dvar_FindVar(source.internals.string);
    }
    else
    {
        if ( source.dataType )
        {
            NameForValueType = GetNameForValueType(source.dataType);
            Expression_Error("Must use a string as the name of a dvar, not a %s\n", NameForValueType);
            result.dataType = VAL_STRING;
            result.internals.intVal = (int)"";
            AddOperandToStack(dataStack, &result);
            return;
        }
        dvar = Dvar_FindVar(source.internals.intVal);
    }
    result.dataType = VAL_STRING;
    if ( dvar )
    {
        if ( dvar->type == DVAR_TYPE_STRING )
            result.internals.intVal = (int)CopyTempString(dvar->current.string);
        else
            result.internals.intVal = (int)Dvar_DisplayableValue(dvar);
    }
    else
    {
        result.internals.intVal = (int)"";
    }
    if ( uiscript_debug && uiscript_debug->current.integer )
    {
        if ( dvar )
            Expression_TraceInternal("dvarstring( %s ) = %s\n", dvar->name, result.internals.string);
        else
            Expression_TraceInternal("dvarstring( %s ) = %s\n", "NULL", result.internals.string);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetDvarBoolValue(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *NameForValueType; // eax
    const char *v4; // eax
    Operand source; // [esp+4h] [ebp-14h] BYREF
    Operand result; // [esp+Ch] [ebp-Ch] BYREF
    const dvar_s *dvar; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    if ( source.dataType == VAL_STRING )
    {
        dvar = Dvar_FindVar(source.internals.string);
    }
    else
    {
        if ( source.dataType )
        {
            NameForValueType = GetNameForValueType(source.dataType);
            Expression_Error("Must use a string as the name of a dvar, not a %s\n", NameForValueType);
            result.dataType = VAL_STRING;
            result.internals.intVal = (int)"";
            AddOperandToStack(dataStack, &result);
            return;
        }
        dvar = Dvar_FindVar(source.internals.intVal);
    }
    result.dataType = VAL_INT;
    if ( dvar )
    {
        if ( dvar->type )
        {
            if ( dvar->type == DVAR_TYPE_FLOAT )
            {
                result.internals.intVal = (int)dvar->current.value;
            }
            else if ( dvar->type == DVAR_TYPE_INT )
            {
                result.internals.intVal = dvar->current.integer;
            }
            else
            {
                v4 = Dvar_DisplayableValue(dvar);
                result.internals.intVal = atoi(v4);
            }
        }
        else
        {
            result.internals.intVal = dvar->current.color[0] != 0;
        }
    }
    else
    {
        result.internals.intVal = 0;
    }
    if ( uiscript_debug && uiscript_debug->current.integer )
    {
        if ( dvar )
            Expression_TraceInternal("dvarbool( %s ) = %i\n", dvar->name, result.internals.intVal);
        else
            Expression_TraceInternal("dvarbool( %s ) = %i\n", "NULL", result.internals.intVal);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetDvarIntValue(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *NameForValueType; // eax
    const char *v4; // eax
    Operand source; // [esp+4h] [ebp-14h] BYREF
    Operand result; // [esp+Ch] [ebp-Ch] BYREF
    const dvar_s *dvar; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    if ( source.dataType == VAL_STRING )
    {
        dvar = Dvar_FindVar(source.internals.string);
    }
    else
    {
        if ( source.dataType )
        {
            NameForValueType = GetNameForValueType(source.dataType);
            Expression_Error("Must use a string as the name of a dvar, not a %s\n", NameForValueType);
            result.dataType = VAL_STRING;
            result.internals.intVal = (int)"";
            AddOperandToStack(dataStack, &result);
            return;
        }
        dvar = Dvar_FindVar(source.internals.intVal);
    }
    result.dataType = VAL_INT;
    if ( dvar )
    {
        if ( dvar->type )
        {
            switch ( dvar->type )
            {
                case DVAR_TYPE_FLOAT:
                    result.internals.intVal = (int)dvar->current.value;
                    break;
                case DVAR_TYPE_INT:
                    result.internals.intVal = dvar->current.integer;
                    break;
                case DVAR_TYPE_ENUM:
                    result.internals.intVal = dvar->current.integer;
                    break;
                default:
                    v4 = Dvar_DisplayableValue(dvar);
                    result.internals.intVal = atoi(v4);
                    break;
            }
        }
        else
        {
            result.internals.intVal = dvar->current.color[0] != 0;
        }
    }
    else
    {
        result.internals.intVal = 0;
    }
    if ( uiscript_debug && uiscript_debug->current.integer )
    {
        if ( dvar )
            Expression_TraceInternal("dvarint( %s ) = %i\n", dvar->name, result.internals.intVal);
        else
            Expression_TraceInternal("dvarint( %s ) = %i\n", "NULL", result.internals.intVal);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetDvarFloatValue(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *NameForValueType; // eax
    const char *v4; // eax
    const char *name; // [esp+8h] [ebp-18h]
    Operand source; // [esp+Ch] [ebp-14h] BYREF
    Operand result; // [esp+14h] [ebp-Ch] BYREF
    const dvar_s *dvar; // [esp+1Ch] [ebp-4h]

    GetOperand(dataStack, &source);
    if ( source.dataType == VAL_STRING )
    {
        dvar = Dvar_FindVar(source.internals.string);
    }
    else
    {
        if ( source.dataType )
        {
            NameForValueType = GetNameForValueType(source.dataType);
            Expression_Error("Must use a string as the name of a dvar, not a %s\n", NameForValueType);
            result.dataType = VAL_STRING;
            result.internals.intVal = (int)"";
            AddOperandToStack(dataStack, &result);
            return;
        }
        dvar = Dvar_FindVar(source.internals.intVal);
    }
    result.dataType = VAL_FLOAT;
    if ( dvar )
    {
        if ( dvar->type )
        {
            if ( dvar->type == DVAR_TYPE_FLOAT )
            {
                result.internals.intVal = dvar->current.integer;
            }
            else if ( dvar->type == DVAR_TYPE_INT )
            {
                result.internals.floatVal = (float)dvar->current.integer;
            }
            else
            {
                v4 = Dvar_DisplayableValue(dvar);
                result.internals.floatVal = atof(v4);
            }
        }
        else
        {
            result.internals.floatVal = (float)dvar->current.enabled;
        }
    }
    else
    {
        result.internals.intVal = 0;
    }
    if ( uiscript_debug && uiscript_debug->current.integer )
    {
        if ( dvar )
            name = dvar->name;
        else
            name = "NULL";
        Expression_TraceInternal("dvarfloat( %s ) = %f\n", name, result.internals.floatVal);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetLocalVarStringValue(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-1Ch] BYREF
    const UILocalVar *var; // [esp+8h] [ebp-14h]
    Operand result; // [esp+Ch] [ebp-10h] BYREF
    int size; // [esp+14h] [ebp-8h]
    char *stringBuf; // [esp+18h] [ebp-4h]

    stringBuf = localVarStrings[localVarStringIndex];
    size = 32;
    localVarStringIndex = (localVarStringIndex + 1) % 4u;
    GetOperand(dataStack, &source);
    var = (const UILocalVar *)GetLocalVar(localClientNum, &source);
    if ( var )
    {
        result.dataType = VAL_STRING;
        result.internals.intVal = (int)UILocalVar_GetString(var, stringBuf, size);
        if ( uiscript_debug )
        {
            if ( uiscript_debug->current.integer )
                Expression_TraceInternal("localVarString( %s ) = %s\n", source.internals.string, result.internals.string);
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        result.dataType = VAL_STRING;
        result.internals.intVal = (int)"";
        AddOperandToStack(dataStack, &result);
    }
}

UILocalVarContext *__cdecl GetLocalVar(int localClientNum, Operand *source)
{
    const char *NameForValueType; // eax
    UILocalVarContext *LocalVarsContext; // eax
    const char *string; // [esp-4h] [ebp-8h]
    int contextIndex; // [esp+0h] [ebp-4h]

    if ( source->dataType == VAL_STRING )
    {
        contextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
        if ( contextIndex == -1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                        553,
                        0,
                        "%s",
                        "contextIndex != INVALID_UI_CONTEXT") )
        {
            __debugbreak();
        }
        string = source->internals.string;
        LocalVarsContext = UI_UIContext_GetLocalVarsContext(contextIndex);
        return UILocalVar_Find(LocalVarsContext, string);
    }
    else
    {
        NameForValueType = GetNameForValueType(source->dataType);
        Expression_Error("Must use a string as the name of a localVar, not a %s\n", NameForValueType);
        return 0;
    }
}

void __cdecl GetLocalVarBoolValue(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    const UILocalVar *var; // [esp+8h] [ebp-Ch]
    Operand result; // [esp+Ch] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    var = (const UILocalVar *)GetLocalVar(localClientNum, &source);
    if ( var )
    {
        result.dataType = VAL_INT;
        result.internals.intVal = UILocalVar_GetBool(var);
        if ( uiscript_debug )
        {
            if ( uiscript_debug->current.integer )
                Expression_TraceInternal("localVarBool( %s ) = %i\n", source.internals.string, result.internals.intVal);
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        result.dataType = VAL_INT;
        result.internals.intVal = 0;
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetLocalVarIntValue(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    const UILocalVar *var; // [esp+8h] [ebp-Ch]
    Operand result; // [esp+Ch] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    var = (const UILocalVar *)GetLocalVar(localClientNum, &source);
    if (var)
    {
        result.dataType = VAL_INT;
        result.internals.intVal = UILocalVar_GetInt(var);
        if (uiscript_debug)
        {
            if (uiscript_debug->current.integer)
                Expression_TraceInternal("localVarInt( %s ) = %i\n", source.internals.string, result.internals.intVal);
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        result.dataType = VAL_INT;
        result.internals.intVal = 0;
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetLocalVarFloatValue(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+8h] [ebp-14h] BYREF
    const UILocalVar *var; // [esp+10h] [ebp-Ch]
    Operand result; // [esp+14h] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    var = (const UILocalVar *)GetLocalVar(localClientNum, &source);
    if ( var )
    {
        result.dataType = VAL_FLOAT;
        result.internals.floatVal = UILocalVar_GetFloat(var);
        if ( uiscript_debug )
        {
            if ( uiscript_debug->current.integer )
                Expression_TraceInternal("localVarFloat( %s ) = %f\n", source.internals.string, result.internals.floatVal);
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        result.dataType = VAL_FLOAT;
        result.internals.intVal = 0;
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetSinValue(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    float v4; // xmm0_4
    Operand source; // [esp+10h] [ebp-14h] BYREF
    Operand result; // [esp+18h] [ebp-Ch] BYREF
    float val; // [esp+20h] [ebp-4h]

    GetOperand(dataStack, &source);
    val = GetSourceFloat(&source);
    result.dataType = VAL_FLOAT;
    //v4 = __libm_sse2_sin(val);
    v4 = sin(val);
    result.internals.floatVal = v4;
    if (uiscript_debug && uiscript_debug->current.integer)
        Expression_TraceInternal("sin( %f ) = %f\n", val, result.internals.floatVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCosValue(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    float v4; // xmm0_4
    Operand source; // [esp+10h] [ebp-14h] BYREF
    Operand result; // [esp+18h] [ebp-Ch] BYREF
    float val; // [esp+20h] [ebp-4h]

    GetOperand(dataStack, &source);
    val = GetSourceFloat(&source);
    result.dataType = VAL_FLOAT;
    //v4 = __libm_sse2_cos(val);
    v4 = cos(val);
    result.internals.floatVal = v4;
    if (uiscript_debug && uiscript_debug->current.integer)
        Expression_TraceInternal("cos( %f ) = %f\n", val, result.internals.floatVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetMilliseconds(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = Sys_Milliseconds();
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetPlayerField(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *NameForValueType; // eax
    int clientNum; // eax
    char xuidStr[36]; // [esp+0h] [ebp-110h] BYREF
    cg_s *LocalClientGlobals; // [esp+24h] [ebp-ECh]
    unsigned __int64 xuid; // [esp+28h] [ebp-E8h]
    cg_s *v8; // [esp+30h] [ebp-E0h]
    cg_s *v9; // [esp+34h] [ebp-DCh]
    Material *material; // [esp+38h] [ebp-D8h] BYREF
    cg_s *v11; // [esp+3Ch] [ebp-D4h]
    char string[32]; // [esp+40h] [ebp-D0h] BYREF
    const char *clanAbbrev; // [esp+60h] [ebp-B0h]
    cg_s *v14; // [esp+64h] [ebp-ACh]
    char clanTag[8]; // [esp+68h] [ebp-A8h] BYREF
    cg_s *v16; // [esp+70h] [ebp-A0h]
    const char *rankText; // [esp+74h] [ebp-9Ch]
    operandInternalDataUnion *OurClientScore; // [esp+78h] [ebp-98h]
    const score_s *nextHighestScore; // [esp+7Ch] [ebp-94h]
    int i; // [esp+80h] [ebp-90h]
    cg_s *v21; // [esp+84h] [ebp-8Ch]
    int place; // [esp+88h] [ebp-88h]
    const score_s *ClientScore; // [esp+8Ch] [ebp-84h]
    cg_s *v24; // [esp+90h] [ebp-80h]
    operandInternalDataUnion *v25; // [esp+94h] [ebp-7Ch]
    operandInternalDataUnion *v26; // [esp+98h] [ebp-78h]
    const score_s *score; // [esp+9Ch] [ebp-74h]
    operandInternalDataUnion *v28; // [esp+A0h] [ebp-70h]
    cg_s *v29; // [esp+A4h] [ebp-6Ch]
    char buf[40]; // [esp+A8h] [ebp-68h] BYREF
    cg_s *cgameGlob; // [esp+D0h] [ebp-40h]
    char clientName[40]; // [esp+D4h] [ebp-3Ch] BYREF
    Operand source; // [esp+100h] [ebp-10h] BYREF
    Operand result; // [esp+108h] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    if ( source.dataType == VAL_STRING )
    {
        if ( I_stricmp(source.internals.string, "name") )
        {
            if ( I_stricmp(source.internals.string, "localname") )
            {
                if ( I_stricmp(source.internals.string, "teamname") )
                {
                    if ( I_stricmp(source.internals.string, "otherteamname") )
                    {
                        if ( I_stricmp(source.internals.string, "dead") )
                        {
                            if ( I_stricmp(source.internals.string, "clipAmmo") )
                            {
                                if ( I_stricmp(source.internals.string, "nightvision") )
                                {
                                    if ( I_stricmp(source.internals.string, "number") )
                                    {
                                        if ( I_stricmp(source.internals.string, "score") )
                                        {
                                            if ( I_stricmp(source.internals.string, "deaths") )
                                            {
                                                if ( I_stricmp(source.internals.string, "kills") )
                                                {
                                                    if ( I_stricmp(source.internals.string, "place") )
                                                    {
                                                        if ( I_stricmp(source.internals.string, "ping") )
                                                        {
                                                            if ( I_stricmp(source.internals.string, "rank") )
                                                            {
                                                                if ( I_stricmp(source.internals.string, "clan") )
                                                                {
                                                                    if ( I_stricmp(source.internals.string, "tag") )
                                                                    {
                                                                        if ( I_stricmp(source.internals.string, "rankicon") )
                                                                        {
                                                                            if ( I_stricmp(source.internals.string, "currentteamname") )
                                                                            {
                                                                                if ( I_stricmp(source.internals.string, "xuid") )
                                                                                {
                                                                                    Expression_Error("Unknown player field '%s'\n", source.internals.string);
                                                                                    result.dataType = VAL_INT;
                                                                                    result.internals.intVal = 0;
                                                                                }
                                                                                else
                                                                                {
                                                                                    result.dataType = VAL_STRING;
                                                                                    result.internals.intVal = (int)"";
                                                                                    LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
                                                                                    if ( LocalClientGlobals->predictedPlayerState.clientNum >= 0x20u
                                                                                        && !Assert_MyHandler(
                                                                                                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                                                                                                    972,
                                                                                                    0,
                                                                                                    "cgameGlob->predictedPlayerState.clientNum doesn't index MAX_CLIENTS\n"
                                                                                                    "\t%i not in [0, %i)",
                                                                                                    LocalClientGlobals->predictedPlayerState.clientNum,
                                                                                                    32) )
                                                                                    {
                                                                                        __debugbreak();
                                                                                    }
                                                                                    if ( LocalClientGlobals->predictedPlayerState.clientNum < 0x20u )
                                                                                    {
                                                                                        xuid = LocalClientGlobals->bgs.clientinfo[LocalClientGlobals->predictedPlayerState.clientNum].xuid;
                                                                                        Com_sprintf(xuidStr, 0x22u, "%lld", xuid);
                                                                                        result.dataType = VAL_STRING;
                                                                                        result.internals.intVal = (int)CopyTempString(xuidStr);
                                                                                    }
                                                                                    if ( uiscript_debug && uiscript_debug->current.integer )
                                                                                        Expression_TraceInternal(
                                                                                            "player( %s ) = %s\n",
                                                                                            source.internals.string,
                                                                                            result.internals.string);
                                                                                }
                                                                            }
                                                                            else
                                                                            {
                                                                                result.dataType = VAL_STRING;
                                                                                result.internals.intVal = 0;
                                                                                v8 = CG_GetLocalClientGlobals(localClientNum);
                                                                                result.dataType = VAL_STRING;
                                                                                result.internals.intVal = (int)CG_GetTeamName(v8->bgs.clientinfo[v8->predictedPlayerState.clientNum].team);
                                                                                if ( uiscript_debug && uiscript_debug->current.integer )
                                                                                    Expression_TraceInternal(
                                                                                        "player( %s ) = %s\n",
                                                                                        source.internals.string,
                                                                                        result.internals.string);
                                                                            }
                                                                        }
                                                                        else
                                                                        {
                                                                            result.dataType = VAL_STRING;
                                                                            result.internals.intVal = (int)"";
                                                                            v9 = CG_GetLocalClientGlobals(localClientNum);
                                                                            CL_GetRankIcon(
                                                                                v9->bgs.clientinfo[v9->predictedPlayerState.clientNum].rank,
                                                                                v9->bgs.clientinfo[v9->predictedPlayerState.clientNum].prestige,
                                                                                &material);
                                                                            if ( material )
                                                                            {
                                                                                result.dataType = VAL_STRING;
                                                                                result.internals.intVal = (int)CopyTempString(material->info.name);
                                                                            }
                                                                            if ( uiscript_debug && uiscript_debug->current.integer )
                                                                                Expression_TraceInternal(
                                                                                    "player( %s ) = %s\n",
                                                                                    source.internals.string,
                                                                                    result.internals.string);
                                                                        }
                                                                    }
                                                                    else
                                                                    {
                                                                        result.dataType = VAL_STRING;
                                                                        result.internals.intVal = (int)"";
                                                                        v11 = CG_GetLocalClientGlobals(localClientNum);
                                                                        CL_GetClientName(localClientNum, v11->predictedPlayerState.clientNum, string, 32, 0);
                                                                        result.dataType = VAL_STRING;
                                                                        result.internals.intVal = (int)CopyTempString(string);
                                                                        if ( uiscript_debug && uiscript_debug->current.integer )
                                                                            Expression_TraceInternal(
                                                                                "player( %s ) = %s\n",
                                                                                source.internals.string,
                                                                                result.internals.string);
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    result.dataType = VAL_STRING;
                                                                    result.internals.intVal = (int)"";
                                                                    memset(clanTag, 0, 7);
                                                                    v14 = CG_GetLocalClientGlobals(localClientNum);
                                                                    clientNum = v14->predictedPlayerState.clientNum;
                                                                    clanAbbrev = v14->bgs.clientinfo[clientNum].clanAbbrev;
                                                                    if ( (cg_s *)((char *)v14 + clientNum * 1480) != (cg_s *)-389184 && *clanAbbrev )
                                                                        Com_sprintf(clanTag, 7u, "[%s]", clanAbbrev);
                                                                    result.dataType = VAL_STRING;
                                                                    result.internals.intVal = (int)CopyTempString(clanTag);
                                                                    if ( uiscript_debug && uiscript_debug->current.integer )
                                                                        Expression_TraceInternal(
                                                                            "player( %s ) = %s\n",
                                                                            source.internals.string,
                                                                            result.internals.string);
                                                                }
                                                            }
                                                            else
                                                            {
                                                                result.dataType = VAL_STRING;
                                                                result.internals.intVal = (int)"";
                                                                v16 = CG_GetLocalClientGlobals(localClientNum);
                                                                rankText = CL_GetRankData(
                                                                                         v16->bgs.clientinfo[v16->predictedPlayerState.clientNum].rank,
                                                                                         MP_RANKTABLE_DISPLAYLEVEL);
                                                                result.dataType = VAL_STRING;
                                                                result.internals.intVal = (int)rankText;
                                                                if ( uiscript_debug && uiscript_debug->current.integer )
                                                                    Expression_TraceInternal(
                                                                        "player( %s ) = %s\n",
                                                                        source.internals.string,
                                                                        result.internals.string);
                                                            }
                                                        }
                                                        else
                                                        {
                                                            OurClientScore = (operandInternalDataUnion *)UI_GetOurClientScore(localClientNum);
                                                            if ( OurClientScore )
                                                                result.internals = (operandInternalDataUnion)OurClientScore->intVal;
                                                            else
                                                                result.internals.intVal = 0;
                                                            result.dataType = VAL_INT;
                                                            if ( uiscript_debug && uiscript_debug->current.integer )
                                                                Expression_TraceInternal(
                                                                    "player( %s ) = %i\n",
                                                                    source.internals.string,
                                                                    result.internals.intVal);
                                                        }
                                                    }
                                                    else
                                                    {
                                                        result.dataType = VAL_INT;
                                                        place = 1;
                                                        v21 = CG_GetLocalClientGlobals(localClientNum);
                                                        ClientScore = GetClientScore(localClientNum, v21->predictedPlayerState.clientNum);
                                                        if ( ClientScore )
                                                        {
                                                            place = ClientScore->place;
                                                            for ( i = place - 1; i >= 1; --i )
                                                            {
                                                                nextHighestScore = UI_GetScoreAtRank(localClientNum, i);
                                                                if ( !nextHighestScore || nextHighestScore->score != ClientScore->score )
                                                                    break;
                                                                place = i;
                                                            }
                                                        }
                                                        if ( !place )
                                                            place = 1;
                                                        result.internals.intVal = place;
                                                        if ( uiscript_debug && uiscript_debug->current.integer )
                                                            Expression_TraceInternal(
                                                                "player( %s ) = %i\n",
                                                                source.internals.string,
                                                                result.internals.intVal);
                                                    }
                                                }
                                                else
                                                {
                                                    v24 = CG_GetLocalClientGlobals(localClientNum);
                                                    v25 = (operandInternalDataUnion *)GetClientScore(
                                                                                                                            localClientNum,
                                                                                                                            v24->predictedPlayerState.clientNum);
                                                    if ( v25 )
                                                        result.internals = v25[4];
                                                    else
                                                        result.internals.intVal = 0;
                                                    result.dataType = VAL_INT;
                                                    if ( uiscript_debug && uiscript_debug->current.integer )
                                                        Expression_TraceInternal(
                                                            "player( %s ) = %i\n",
                                                            source.internals.string,
                                                            result.internals.intVal);
                                                }
                                            }
                                            else
                                            {
                                                v26 = (operandInternalDataUnion *)UI_GetOurClientScore(localClientNum);
                                                if ( v26 )
                                                    result.internals = v26[6];
                                                else
                                                    result.internals.intVal = 0;
                                                result.dataType = VAL_INT;
                                                if ( uiscript_debug && uiscript_debug->current.integer )
                                                    Expression_TraceInternal(
                                                        "player( %s ) = %i\n",
                                                        source.internals.string,
                                                        result.internals.intVal);
                                            }
                                        }
                                        else
                                        {
                                            score = UI_GetOurClientScore(localClientNum);
                                            if ( score )
                                                result.internals.intVal = score->score;
                                            else
                                                result.internals.intVal = 0;
                                            result.dataType = VAL_INT;
                                            if ( uiscript_debug && uiscript_debug->current.integer )
                                                Expression_TraceInternal(
                                                    "player( %s ) = %i\n",
                                                    source.internals.string,
                                                    result.internals.intVal);
                                        }
                                    }
                                    else
                                    {
                                        v28 = (operandInternalDataUnion *)CG_GetLocalClientGlobals(localClientNum);
                                        result.dataType = VAL_INT;
                                        result.internals = (operandInternalDataUnion)v28->intVal;
                                        if ( uiscript_debug && uiscript_debug->current.integer )
                                            Expression_TraceInternal("player( %s ) = %i\n", source.internals.string, result.internals.intVal);
                                    }
                                }
                                else
                                {
                                    result.dataType = VAL_INT;
                                    result.internals.intVal = (unsigned __int8)CG_LookingThroughNightVision(localClientNum);
                                    if ( uiscript_debug && uiscript_debug->current.integer )
                                        Expression_TraceInternal("player( %s ) = %i\n", source.internals.string, result.internals.intVal);
                                }
                            }
                            else
                            {
                                result.dataType = VAL_INT;
                                result.internals.intVal = CG_GetPlayerClipAmmoCount(localClientNum);
                                if ( uiscript_debug && uiscript_debug->current.integer )
                                    Expression_TraceInternal("player( %s ) = %s\n", source.internals.string, result.internals.string);
                            }
                        }
                        else
                        {
                            result.dataType = VAL_INT;
                            result.internals.intVal = CG_IsPlayerDead(localClientNum);
                            if ( uiscript_debug && uiscript_debug->current.integer )
                                Expression_TraceInternal("player( %s ) = %i\n", source.internals.string, result.internals.intVal);
                        }
                    }
                    else
                    {
                        result.dataType = VAL_STRING;
                        result.internals.intVal = (int)CG_GetPlayerOpposingTeamName(localClientNum);
                        if ( uiscript_debug && uiscript_debug->current.integer )
                            Expression_TraceInternal("player( %s ) = %s\n", source.internals.string, result.internals.string);
                    }
                }
                else
                {
                    result.dataType = VAL_STRING;
                    result.internals.intVal = (int)CG_GetPlayerTeamName(localClientNum);
                    if ( uiscript_debug && uiscript_debug->current.integer )
                        Expression_TraceInternal("player( %s ) = %s\n", source.internals.string, result.internals.string);
                }
            }
            else
            {
                result.dataType = VAL_STRING;
                result.internals.intVal = (int)"";
                v29 = CG_GetLocalClientGlobals(localClientNum);
                CL_GetClientName(localClientNum, v29->clientNum, buf, 38, 1);
                result.dataType = VAL_STRING;
                result.internals.intVal = (int)CopyTempString(buf);
                if ( uiscript_debug && uiscript_debug->current.integer )
                    Expression_TraceInternal("player( %s ) = %s\n", source.internals.string, result.internals.string);
            }
        }
        else
        {
            result.dataType = VAL_STRING;
            result.internals.intVal = (int)"";
            cgameGlob = CG_GetLocalClientGlobals(localClientNum);
            CL_GetClientName(localClientNum, cgameGlob->predictedPlayerState.clientNum, clientName, 38, 1);
            result.dataType = VAL_STRING;
            result.internals.intVal = (int)CopyTempString(clientName);
            if ( uiscript_debug )
            {
                if ( uiscript_debug->current.integer )
                    Expression_TraceInternal("player( %s ) = %s\n", source.internals.string, result.internals.string);
            }
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        NameForValueType = GetNameForValueType(source.dataType);
        Expression_Error("Must use a string as the name of a player field, not a %s\n", NameForValueType);
        result.dataType = VAL_STRING;
        result.internals.intVal = (int)"";
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetOtherTeamField(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+4h] [ebp-1Ch] BYREF
    team_t otherteam; // [esp+Ch] [ebp-14h]
    Operand result; // [esp+10h] [ebp-10h] BYREF
    team_t team; // [esp+18h] [ebp-8h]
    cg_s *cgameGlob; // [esp+1Ch] [ebp-4h]

    GetOperand(dataStack, &source);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->bgs.clientinfo[cgameGlob->clientNum].infoValid )
        team = cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team;
    else
        team = TEAM_FREE;
    switch ( team )
    {
        case TEAM_FREE:
            otherteam = TEAM_FREE;
            break;
        case TEAM_AXIS:
            otherteam = TEAM_ALLIES;
            break;
        case TEAM_ALLIES:
            otherteam = TEAM_AXIS;
            break;
        case TEAM_SPECTATOR:
            otherteam = TEAM_SPECTATOR;
            break;
        default:
            otherteam = TEAM_FREE;
            break;
    }
    GetFieldForTeam(localClientNum, otherteam, &source, &result);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetFieldForTeam(int localClientNum, team_t team, Operand *fieldName, Operand *result)
{
    const char *NameForValueType; // eax
    cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( fieldName->dataType == VAL_STRING )
    {
        if ( I_stricmp(fieldName->internals.string, "score") )
        {
            if ( I_stricmp(fieldName->internals.string, "name") )
            {
                if ( I_stricmp(fieldName->internals.string, "ffaname") )
                {
                    Expression_Error("Unknown team field '%s'\n", fieldName->internals.string);
                    result->dataType = VAL_INT;
                    result->internals.intVal = 0;
                }
                else
                {
                    result->dataType = VAL_STRING;
                    result->internals.intVal = (int)CG_GetFFATeamName(cgameGlob->bgs.clientinfo[cgameGlob->clientNum].ffaTeam);
                    if ( uiscript_debug && uiscript_debug->current.integer )
                        Expression_TraceInternal(
                            "team(%i)( %s ) = %s\n",
                            team,
                            fieldName->internals.string,
                            result->internals.string);
                }
            }
            else
            {
                result->dataType = VAL_STRING;
                result->internals.intVal = (int)CG_GetTeamName(team);
                if ( uiscript_debug && uiscript_debug->current.integer )
                    Expression_TraceInternal("team(%i)( %s ) = %s\n", team, fieldName->internals.string, result->internals.string);
            }
        }
        else
        {
            result->dataType = VAL_INT;
            result->internals.intVal = cgameGlob->teamScores[team];
            if ( uiscript_debug )
            {
                if ( uiscript_debug->current.integer )
                    Expression_TraceInternal("team(%i)( %s ) = %i\n", team, fieldName->internals.string, result->internals.intVal);
            }
        }
    }
    else
    {
        NameForValueType = GetNameForValueType(fieldName->dataType);
        Expression_Error(" Must use a string as the name of a team parameter, not a %s\n", NameForValueType);
        result->dataType = VAL_STRING;
        result->internals.intVal = (int)"";
    }
}

void __cdecl GetTeamField(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-18h] BYREF
    Operand result; // [esp+8h] [ebp-10h] BYREF
    team_t team; // [esp+10h] [ebp-8h]
    cg_s *cgameGlob; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->bgs.clientinfo[cgameGlob->clientNum].infoValid )
        team = cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team;
    else
        team = TEAM_FREE;
    GetFieldForTeam(localClientNum, team, &source, &result);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetTeamMarinesField(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-10h] BYREF
    Operand result; // [esp+8h] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    GetFieldForTeam(localClientNum, TEAM_ALLIES, &source, &result);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetTeamOpForField(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-10h] BYREF
    Operand result; // [esp+8h] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    GetFieldForTeam(localClientNum, TEAM_AXIS, &source, &result);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetUIActive(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = CL_IsUIActive(localClientNum);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("ui_active() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsViewportLarge(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 1;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsViewportLarge() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetFlashbanged(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    bool v3; // [esp+0h] [ebp-Ch]
    Operand result; // [esp+4h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    v3 = CG_Flashbanged(localClientNum) || CG_Flared(localClientNum);
    result.internals.intVal = v3;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("flashbanged() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetScoped(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = CG_ScopeIsOverlayed(localClientNum);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("scoped() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl InKillcam(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = CG_GetLocalClientGlobals(localClientNum)->inKillCam;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("InKillcam() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsDualWield(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+Ch] [ebp-10h] BYREF
    cg_s *cgameGlob; // [esp+14h] [ebp-8h]
    unsigned int weaponIndex; // [esp+18h] [ebp-4h]

    result.dataType = VAL_INT;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->weaponSelect < BG_GetNumWeapons()
        && BG_PlayerHasWeapon(&cgameGlob->predictedPlayerState, cgameGlob->weaponSelect) )
    {
        weaponIndex = cgameGlob->weaponSelect;
    }
    else
    {
        weaponIndex = cgameGlob->predictedPlayerState.weapon;
    }
    result.internals.intVal = BG_GetWeaponDef(weaponIndex)->bDualWield;
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsFuelWeapon(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+Ch] [ebp-10h] BYREF
    cg_s *cgameGlob; // [esp+14h] [ebp-8h]
    unsigned int weaponIndex; // [esp+18h] [ebp-4h]

    result.dataType = VAL_INT;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->weaponSelect < BG_GetNumWeapons()
        && BG_PlayerHasWeapon(&cgameGlob->predictedPlayerState, cgameGlob->weaponSelect) )
    {
        weaponIndex = cgameGlob->weaponSelect;
    }
    else
    {
        weaponIndex = cgameGlob->predictedPlayerState.weapon;
    }
    result.internals.intVal = BG_GetWeaponDef(weaponIndex)->fuelTankWeapon;
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetPartyMissingMapPackError(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand operandResult; // [esp+0h] [ebp-8h] BYREF

    operandResult.internals.intVal = (int)"";
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl IsSelectingLocationalKillstreak(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    bool v3; // [esp+0h] [ebp-18h]
    Operand result; // [esp+4h] [ebp-14h] BYREF
    int mtlIndex; // [esp+Ch] [ebp-Ch]
    cg_s *cgameGlob; // [esp+10h] [ebp-8h]
    int locationSelectorActive; // [esp+14h] [ebp-4h]

    result.dataType = VAL_INT;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    mtlIndex = cgameGlob->predictedPlayerState.locationSelectionInfo & 0xF;
    v3 = mtlIndex < 15 && cgameGlob->predictedPlayerState.locationSelectionInfo;
    locationSelectorActive = v3;
    result.internals.intVal = v3;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsSelectingLocationalKillstreak() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl isInGuidedMissile(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    centity_s *cent; // [esp+10h] [ebp-14h]
    Operand result; // [esp+14h] [ebp-10h] BYREF
    cg_s *cgameGlob; // [esp+1Ch] [ebp-8h]
    playerState_s *ps; // [esp+20h] [ebp-4h]

    result.dataType = VAL_INT;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    result.internals.intVal = 0;
    if ( BG_GetWeaponDef(cgameGlob->predictedPlayerState.weapon)->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED
        && ps->fWeaponPosFrac >= 0.94999999
        || (ps->eFlags2 & 0x40000) != 0 )
    {
        if ( ps->fWeaponPosFrac >= 0.94999999 )
            result.internals.intVal = 1;
        if ( ps->viewlocked_entNum != 1023 || !BG_GetAmmoInClip(ps, ps->weapon) )
            result.internals.intVal = 1;
    }
    else if ( (ps->eFlags & 0x300) != 0 )
    {
        cent = CG_GetEntity(localClientNum, ps->viewlocked_entNum);
        if ( BG_GetWeaponDef(cent->nextState.weapon)->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED )
            result.internals.intVal = 1;
    }
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("isInGuidedMissile() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsInGame(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = CL_AllLocalClientsDisconnected() == 0;
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsSelectingAirstrike(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    result.internals.intVal = cgameGlob->predictedPlayerState.locationSelectionType == LOC_SEL_AIRSTRIKE;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsSelectingAirstrike() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsSelectingArtillery(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    result.internals.intVal = cgameGlob->predictedPlayerState.locationSelectionType == LOC_SEL_ARTILLERY;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsSelectingArtillery() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsSelectingNapalm(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    result.internals.intVal = cgameGlob->predictedPlayerState.locationSelectionType == LOC_SEL_NAPALM;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsSelectingNapalm() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsSelectingMortar(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    result.internals.intVal = cgameGlob->predictedPlayerState.locationSelectionType == LOC_SEL_MORTAR;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsSelectingMortar() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsSelectingComlink(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    result.internals.intVal = cgameGlob->predictedPlayerState.locationSelectionType == LOC_SEL_COMLINK;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsSelectingComlink() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetScoreboardVisible(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = CG_IsScoreboardDisplayed(localClientNum);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("scoreboard_visible() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetIsIntermission(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = CG_IsIntermission(localClientNum);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("isIntermission() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetIsSuperUser(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    EUserTier tier; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    tier = (EUserTier)Live_GetTier(ControllerIndex);
    result.internals.intVal = tier == USER_TIER_SUPER;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("issuperuser() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetIsAutoJoinDevUser(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("isautojoindevuser() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetSelectingLocation(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = (unsigned __int8)CG_IsSelectingLocation(localClientNum);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("selecting_location() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl ShowZombieMap(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = CG_IsShowingZombieMap();
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("ShowZombieMap() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl PrivatePartyHostInLobby(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.internals.intVal = 0;
    result.dataType = VAL_INT;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("PrivatePartyHostInLobby() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl AloneInPrivateParty(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.internals.intVal = 0;
    result.dataType = VAL_INT;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("AloneInPrivateParty() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl AloneInPrivatePartyIgnoreSplitscreen(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-10h] BYREF
    Operand result; // [esp+8h] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    result.internals.intVal = 0;
    result.dataType = VAL_INT;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("AloneInPartyIgnoreSplitscreen() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl AloneInLobby(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.internals.intVal = 0;
    result.dataType = VAL_INT;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("AloneInLobby() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl AloneInLobbyIgnoreSplitscreen(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-10h] BYREF
    Operand result; // [esp+8h] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    result.internals.intVal = 0;
    result.dataType = VAL_INT;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("AloneInLobbyIgnoreSplitscreen() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GameHost(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GameHost() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl InLobby(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = CL_GetLocalClientConnectionState(localClientNum) == 10;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("InLobby() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl InPrivateParty(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = com_sv_running->current.enabled;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("InPrivateParty() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsVisibilityBitSet(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    __int64 v4; // [esp+4h] [ebp-1Ch]
    Operand source; // [esp+Ch] [ebp-14h] BYREF
    int checkBit; // [esp+14h] [ebp-Ch]
    Operand result; // [esp+18h] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    checkBit = GetSourceInt(&source).intVal;
    LODWORD(v4) = LODWORD(sharedUiInfo.visibilityBits[localClientNum]) & (1LL << checkBit);
    HIDWORD(v4) = HIDWORD(sharedUiInfo.visibilityBits[localClientNum]) & ((1LL << checkBit) >> 32);
    result.internals.intVal = v4 != 0;
    if (uiscript_debug && uiscript_debug->current.integer)
        Expression_TraceInternal("IsVisiblityBitSet() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl PrivatePartyHost(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("PrivatePartyHost() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl Splitscreen(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("Splitscreen() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl SplitscreenHost(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = Com_LocalClient_IsPrimary(localClientNum);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("SplitscreenHost() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl SplitscreenNum(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 1;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("SplitscreenNum() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsCinematicFinished(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsCinematicFinished() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsItemLocked(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    int v4; // [esp-4h] [ebp-18h]
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    v4 = index;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = BG_UnlockablesIsItemLocked(ControllerIndex, v4);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsItemLocked() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsItemLockedForAll(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    int v4; // [esp-4h] [ebp-1Ch]
    int otherLocalClientNum; // [esp+0h] [ebp-18h]
    Operand source; // [esp+4h] [ebp-14h] BYREF
    Operand result; // [esp+Ch] [ebp-Ch] BYREF
    int index; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 1;
    index = GetSourceInt(&source).intVal;
    for ( otherLocalClientNum = 0; otherLocalClientNum < 1; ++otherLocalClientNum )
    {
        if ( Com_LocalClient_IsBeingUsed(otherLocalClientNum) )
        {
            v4 = index;
            ControllerIndex = Com_LocalClient_GetControllerIndex(otherLocalClientNum);
            if ( !BG_UnlockablesIsItemLocked(ControllerIndex, v4) )
            {
                result.internals.intVal = 0;
                break;
            }
        }
    }
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsItemLockedForAll() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCurrentClanTagFeature(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int index; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    index = sharedUiInfo.clanTagFeature;
    if ( sharedUiInfo.clanTagFeature == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    1801,
                    0,
                    "%s",
                    "index != ENTRY_NOT_FOUND") )
    {
        __debugbreak();
    }
    result.internals.intVal = index;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetCurrentClanTagFeature() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsClanTagFeatureLocked(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    unsigned int v4; // [esp-4h] [ebp-18h]
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    v4 = index;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = BG_UnlockablesIsClanTagFeatureLocked(ControllerIndex, v4);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsClanTagFeatureLocked() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetClanTagFeatureCost(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *ClanTagFeature; // eax
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    ClanTagFeature = BG_UnlockablesGetClanTagFeature(index, CLANTAG_COL_COST);
    result.internals.intVal = atoi(ClanTagFeature);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetClanTagFeatureCost() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetClanTagFeatureName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    index = GetSourceInt(&source).intVal;
    result.internals.intVal = (int)BG_UnlockablesGetClanTagFeature(index, CLANTAG_COL_NAME);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetClanTagFeatureName() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsClanTagFeaturePurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    unsigned int v4; // [esp-4h] [ebp-18h]
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    v4 = index;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = BG_UnlockablesIsClanTagFeaturePurchased(ControllerIndex, v4);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsClanTagFeaturePurchased() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetClanTagFeaturePlevel(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    result.internals.intVal = BG_UnlockablesGetClanTagFeaturePLvlUnlock(index);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetClanTagFeaturePlevel() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsItemDualWieldLocked(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    int v4; // [esp-4h] [ebp-18h]
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    v4 = index;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = BG_UnlockablesIsItemDualWieldLocked(ControllerIndex, v4);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsItemDualWieldLocked() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsItemDualWieldPurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    int v4; // [esp-4h] [ebp-18h]
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    v4 = index;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = BG_UnlockablesIsItemDualWieldPurchased(ControllerIndex, v4);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsItemDualWieldPurchased() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsItemNew(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    int v4; // [esp-4h] [ebp-18h]
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    v4 = index;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = BG_UnlockablesIsItemNew(ControllerIndex, v4);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsItemNew() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsItemGroupNew(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    const char *v4; // [esp-4h] [ebp-18h]
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    const char *groupName; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    groupName = GetSourceString(source);
    v4 = groupName;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = (unsigned __int8)BG_UnlockablesIsItemGroupNew(ControllerIndex, v4);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsItemGroupNew() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetAttachmentsFormatted(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int attachments; // [esp+14h] [ebp-174h]
    Operand result; // [esp+18h] [ebp-170h] BYREF
    OperandList list; // [esp+20h] [ebp-168h] BYREF
    int attachmentNum; // [esp+7Ch] [ebp-10Ch]
    char fullWeaponName[256]; // [esp+80h] [ebp-108h] BYREF
    const char *currentAttachmentName; // [esp+184h] [ebp-4h]

    memset(fullWeaponName, 0, sizeof(fullWeaponName));
    attachmentNum = 24;
    GetOperandList(dataStack, &list);
    if ( list.operandCount >= 1 )
    {
        for ( attachments = 0; attachments < list.operandCount; ++attachments )
        {
            attachmentNum = GetSourceInt(&list.operands[attachments]).intVal;
            if ( (unsigned int)attachmentNum >= 0x18
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                            1993,
                            0,
                            "attachmentNum doesn't index ATTACHMENT_COUNT\n\t%i not in [0, %i)",
                            attachmentNum,
                            24) )
            {
                __debugbreak();
            }
            currentAttachmentName = BG_GetAttachmentDisplayName((eAttachment)attachmentNum);
            if ( currentAttachmentName )
            {
                if ( strcmp(currentAttachmentName, "MPUI_NO_ATTACHMENT") )
                {
                    I_strncat(fullWeaponName, 64, " / ");
                    currentAttachmentName = SEH_LocalizeTextMessage(currentAttachmentName, "ui string", LOCMSG_NOERR);
                    I_strncat(fullWeaponName, 64, currentAttachmentName);
                }
            }
        }
        result.dataType = VAL_STRING;
        result.internals.intVal = (int)fullWeaponName;
        if ( uiscript_debug && uiscript_debug->current.integer )
            Expression_TraceInternal("GetWeaponFullName() = %s\n", result.internals.string);
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Expression_Error("Expected at least one GetWeaponOptionGroupIndex()\n");
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetAttachmentName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int attachmentNum; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    attachmentNum = GetSourceInt(&source).intVal;
    if ( (unsigned int)attachmentNum < 0x18 )
        result.internals.intVal = (int)BG_GetAttachmentDisplayName((eAttachment)attachmentNum);
    else
        result.internals.intVal = (int)"";
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetAttachmentName() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetAttachmentImage(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int attachmentNum; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    attachmentNum = GetSourceInt(&source).intVal;
    if ( (unsigned int)attachmentNum >= 0x18
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    2040,
                    0,
                    "attachmentNum doesn't index ATTACHMENT_COUNT\n\t%i not in [0, %i)",
                    attachmentNum,
                    24) )
    {
        __debugbreak();
    }
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)BG_GetAttachmentImage((eAttachment)attachmentNum);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetAttachmentImage() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetAttachmentDesc(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int attachmentNum; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    attachmentNum = GetSourceInt(&source).intVal;
    if ( (unsigned int)attachmentNum >= 0x18
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    2056,
                    0,
                    "attachmentNum doesn't index ATTACHMENT_COUNT\n\t%i not in [0, %i)",
                    attachmentNum,
                    24) )
    {
        __debugbreak();
    }
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)BG_GetAttachmentDesc((eAttachment)attachmentNum);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetAttachmentName() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsAttachmentAllowedOnItemIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *ItemAttachmentRef; // eax
    int attachmentNum; // [esp+0h] [ebp-74h]
    int i; // [esp+4h] [ebp-70h]
    int numAttachments; // [esp+8h] [ebp-6Ch]
    Operand result; // [esp+Ch] [ebp-68h] BYREF
    OperandList operandList; // [esp+14h] [ebp-60h] BYREF
    const char *attachmentRef; // [esp+6Ch] [ebp-8h]
    int itemIndex; // [esp+70h] [ebp-4h]

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    GetOperandList(dataStack, &operandList);
    if ( operandList.operandCount == 2 )
    {
        itemIndex = GetSourceInt(operandList.operands).intVal;
        attachmentRef = GetSourceString(operandList.operands[1]);
        numAttachments = BG_UnlockablesGetNumItemAttachmentsWithAttachPoint(itemIndex, 0);
        for ( i = 0; i < numAttachments; ++i )
        {
            attachmentNum = BG_UnlockablesGetItemAttachmentNumWithAttachPoint(itemIndex, i, 0);
            ItemAttachmentRef = BG_UnlockablesGetItemAttachmentRef(itemIndex, attachmentNum);
            if ( !I_stricmp(attachmentRef, ItemAttachmentRef) )
            {
                result.internals.intVal = 1;
                break;
            }
        }
        if ( uiscript_debug )
        {
            if ( uiscript_debug->current.integer )
                Expression_TraceInternal("IsAttachmentAllowedOnItemIndex() = %d\n", result.internals.intVal);
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Expression_Error("Expected 2 parameters to IsAttachmentAllowedOnItemIndex()\n");
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetFaceCamoIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int faceCamoIndex; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    faceCamoIndex = GetSourceInt(&source).intVal;
    result.internals.intVal = BG_UnlockablesGetFaceCamoItemIndex(faceCamoIndex);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetFaceCamoIndex() = %d\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetWeaponOptionImage(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int weaponOption; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    weaponOption = GetSourceInt(&source).intVal;
    result.internals.intVal = (int)BG_GetWeaponOptionImage(weaponOption);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetWeaponOptionImage() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetWeaponOptionName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int weaponOption; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    weaponOption = GetSourceInt(&source).intVal;
    result.internals.intVal = (int)BG_GetWeaponOptionDisplayName(weaponOption);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetWeaponOptionName() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetWeaponOptionGroupIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-68h] BYREF
    OperandList list; // [esp+8h] [ebp-60h] BYREF
    eWeaponOptionGroup weaponOptionGroup; // [esp+60h] [ebp-8h]
    int index; // [esp+64h] [ebp-4h]

    GetOperandList(dataStack, &list);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( list.operandCount == 2 )
    {
        index = GetSourceInt(list.operands).intVal;
        weaponOptionGroup = (eWeaponOptionGroup)GetSourceInt(&list.operands[1]).intVal;
        result.internals.intVal = BG_GetWeaponOptionNumFromIndexAndGroup(index, weaponOptionGroup);
        if ( uiscript_debug )
        {
            if ( uiscript_debug->current.integer )
                Expression_TraceInternal("GetWeaponOptionGroupIndex() = %i\n", result.internals.intVal);
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Expression_Error("Expected 2 parameters to GetWeaponOptionGroupIndex()\n");
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetPooledFileDetails(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-64h] BYREF
    const char *field; // [esp+8h] [ebp-5Ch]
    OperandList list; // [esp+Ch] [ebp-58h] BYREF

    GetOperandList(dataStack, &list);
    result.dataType = VAL_STRING;
    field = list.operands[0].internals.string;
    result.internals.intVal = (int)Live_FileShare_GetPooledFileInfo(list.operands[0].internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl CanRateFilmInTheater(int locaClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("CanRateFileInTheater() = %d\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetDemoFileID(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetDemoFileID() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetFileShareRating(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
#ifdef KISAK_LIVE
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = Live_FileShare_GetRating();
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetFileShareRating() = %d\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
#endif
}

void __cdecl GetFileShareTotalVotes(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    unsigned __int64 v3; // rax
    bdVoteRankStatsInfo *v4; // [esp-8h] [ebp-18h]
    int v5; // [esp-4h] [ebp-14h]
    int numRatings; // [esp+0h] [ebp-10h] BYREF
    Operand result; // [esp+4h] [ebp-Ch] BYREF
    bdVoteRankStatsInfo *ratings; // [esp+Ch] [ebp-4h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    Live_FileShareSearch_GetRatings(&ratings, &numRatings);
    v5 = numRatings;
    v4 = ratings;
    v3 = I_atoi64(fsSelectedFileID->current.string);
    result.internals.intVal = (int)Live_FileShare_LookupTotalVotes(v3, v4, v5);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetFileShareTotalVotes() = %d\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetAutoJoinLobbyStatus(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetAutoJoinLobbyStatus() = %d\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCounterTotal(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *SourceString; // eax
    __int64 v4; // rax
    char *counterType; // [esp+8h] [ebp-68h]
    int counterId; // [esp+Ch] [ebp-64h]
    Operand result; // [esp+10h] [ebp-60h] BYREF
    OperandList list; // [esp+18h] [ebp-58h] BYREF

    GetOperandList(dataStack, &list);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( list.operandCount == 1 )
    {
        counterType = GetSourceString(list.operands[0]);
        counterId = LiveCounter_CounterStringToID(counterType);
        if ( counterId == -1 )
        {
            SourceString = GetSourceString(list.operands[0]);
            Com_PrintError(16, "Invalid counter string id: %s\n", SourceString);
        }
        else
        {
            LODWORD(v4) = LiveCounter_GetCounterTotalValue(counterId);
            if ( v4 != -1 )
                result.internals.intVal = (int)va("%llu", v4);
        }
    }
    else
    {
        Com_PrintError(16, "Invalid param count. usage: <string returned> getCounterTotal( <counter id> ).\n");
    }
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetCounterTotal() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetDownloadProgress(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    int v4; // eax
    Operand result; // [esp+10h] [ebp-14h] BYREF
    unsigned int bytes; // [esp+18h] [ebp-Ch] BYREF
    float dataRate; // [esp+1Ch] [ebp-8h] BYREF
    unsigned int totalSize; // [esp+20h] [ebp-4h]

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    totalSize = LiveStorage_FileShare_GetDownloadingFileTotalSize(ControllerIndex);
    v4 = Com_LocalClient_GetControllerIndex(localClientNum);
    LiveStorage_FileShare_GetDownloadProgress(v4, &bytes, &dataRate);
    if ( totalSize && totalSize >= bytes )
        result.internals.intVal = (int)((double)bytes / (double)totalSize * 100.0);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetDownloadProgress() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetUploadProgress(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    int v4; // eax
    Operand result; // [esp+10h] [ebp-14h] BYREF
    unsigned int bytes; // [esp+18h] [ebp-Ch] BYREF
    float dataRate; // [esp+1Ch] [ebp-8h] BYREF
    unsigned int totalSize; // [esp+20h] [ebp-4h]

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    totalSize = LiveStorage_FileShare_GetUploadingFileTotalSize(ControllerIndex);
    v4 = Com_LocalClient_GetControllerIndex(localClientNum);
    LiveStorage_FileShare_GetUploadProgress(v4, &bytes, &dataRate);
    if ( totalSize < bytes )
        totalSize = bytes;
    if ( totalSize && totalSize >= bytes )
        result.internals.intVal = (int)((double)bytes / (double)totalSize * 100.0);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetUploadProgress() = %d\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsPremiumSubscriber(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = CG_IsShowingZombieMap();
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsPremiumSubscriber() = %d\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetUploadTimeRemaining(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    int v4; // eax
    Operand result; // [esp+8h] [ebp-14h] BYREF
    unsigned int bytes; // [esp+10h] [ebp-Ch] BYREF
    float dataRate; // [esp+14h] [ebp-8h] BYREF
    unsigned int totalSize; // [esp+18h] [ebp-4h]

    result.dataType = VAL_STRING;
    result.internals.intVal = 0;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    totalSize = LiveStorage_FileShare_GetUploadingFileTotalSize(ControllerIndex);
    v4 = Com_LocalClient_GetControllerIndex(localClientNum);
    LiveStorage_FileShare_GetUploadProgress(v4, &bytes, &dataRate);
    if ( (int)(Sys_Milliseconds() - lastPoll) > 1000 )
    {
        if ( totalSize && totalSize >= bytes )
            secondsRemaining = (int)((double)(totalSize - bytes) / dataRate);
        else
            secondsRemaining = 0;
        lastPoll = Sys_Milliseconds();
    }
    if ( secondsRemaining >= 86400 )
        result.internals.intVal = (int)"";
    else
        result.internals.intVal = (int)GetLocalizedTimeRemaining(secondsRemaining);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetUploadProgress() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

char *__cdecl GetLocalizedTimeRemaining(int secondsRemaining)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    char *v12; // eax
    const char *v13; // eax
    int hours; // [esp+0h] [ebp-80h]
    int seconds; // [esp+4h] [ebp-7Ch]
    int minutes; // [esp+8h] [ebp-78h]
    int days; // [esp+Ch] [ebp-74h]
    char output[68]; // [esp+10h] [ebp-70h] BYREF
    ConversionArguments convArgs; // [esp+58h] [ebp-28h] BYREF

    days = 0;
    hours = 0;
    minutes = 0;
    seconds = secondsRemaining;
    if ( secondsRemaining <= 86400 )
    {
        if ( secondsRemaining <= 3600 )
        {
            if ( secondsRemaining > 60 )
            {
                minutes = secondsRemaining / 60;
                seconds = secondsRemaining % 60;
            }
        }
        else
        {
            hours = secondsRemaining / 3600;
            seconds = secondsRemaining % 3600;
        }
    }
    else
    {
        days = secondsRemaining / 86400;
        seconds = secondsRemaining % 86400;
    }
    convArgs.argCount = 0;
    memset(output, 0, 0x40u);
    if ( days <= 0 )
    {
        if ( hours <= 0 )
        {
            if ( minutes <= 0 )
            {
                v13 = va("%d", seconds);
                convArgs.args[convArgs.argCount++] = v13;
                v12 = UI_SafeTranslateString("MENU_TIMEREMAINING_S");
            }
            else
            {
                v10 = va("%d", minutes);
                convArgs.args[convArgs.argCount++] = v10;
                v11 = va("%d", seconds);
                convArgs.args[convArgs.argCount++] = v11;
                v12 = UI_SafeTranslateString("MENU_TIMEREMAINING_MS");
            }
            UI_ReplaceConversions(v12, &convArgs, output, 64);
        }
        else
        {
            v6 = va("%d", hours);
            convArgs.args[convArgs.argCount++] = v6;
            v7 = va("%d", minutes);
            convArgs.args[convArgs.argCount++] = v7;
            v8 = va("%d", seconds);
            convArgs.args[convArgs.argCount++] = v8;
            v9 = UI_SafeTranslateString("MENU_TIMEREMAINING_HMS");
            UI_ReplaceConversions(v9, &convArgs, output, 64);
        }
    }
    else
    {
        v1 = va("%d", days);
        convArgs.args[convArgs.argCount++] = v1;
        v2 = va("%d", hours);
        convArgs.args[convArgs.argCount++] = v2;
        v3 = va("%d", minutes);
        convArgs.args[convArgs.argCount++] = v3;
        v4 = va("%d", seconds);
        convArgs.args[convArgs.argCount++] = v4;
        v5 = UI_SafeTranslateString("MENU_TIMEREMAINING_DHMS");
        UI_ReplaceConversions(v5, &convArgs, output, 64);
    }
    return va("%s", output);
}

void __cdecl ShowBusyDotsIndicator(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)UI_GetBusyDotsIndicator();
    AddOperandToStack(dataStack, &result);
}

void __cdecl HostMigrationWaitingForPlayers(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("HostMigrationWaitingForPlayers() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCurrentItemOption(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    eWeaponOptionGroup OptionFromIndexAndGroupDvar; // eax
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    OptionFromIndexAndGroupDvar = (eWeaponOptionGroup)GetOptionFromIndexAndGroupDvar();
    result.internals.intVal = BG_GetWeaponOptionNumFromIndexAndGroup(
                                                            sharedUiInfo.sortedItemPivot,
                                                            OptionFromIndexAndGroupDvar);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetCurrentItemOption() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

int __cdecl GetOptionFromIndexAndGroupDvar()
{
    int currentWeaponGroup; // [esp+0h] [ebp-8h]

    currentWeaponGroup = 0;
    if ( ui_currentWeaponOptionGroup )
        return ui_currentWeaponOptionGroup->current.integer;
    return currentWeaponGroup;
}

void __cdecl GetWeaponOptionCost(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    Operand source; // [esp+0h] [ebp-10h] BYREF
    Operand result; // [esp+8h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    GetOperand(dataStack, &source);
    v3.intVal = GetSourceInt(&source).intVal;
    result.internals.intVal = BG_GetWeaponOptionCost(v3.intVal);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetWeaponOptionCost() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetWeaponOptionUnlockLvl(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    Operand source; // [esp+0h] [ebp-10h] BYREF
    Operand result; // [esp+8h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    GetOperand(dataStack, &source);
    v3.intVal = GetSourceInt(&source).intVal;
    result.internals.intVal = BG_GetWeaponOptionUnlockLvl(v3.intVal);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetWeaponOptionUnlockLvl() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetWeaponOptionUnlockPLevel(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    Operand source; // [esp+0h] [ebp-10h] BYREF
    Operand result; // [esp+8h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    GetOperand(dataStack, &source);
    v3.intVal = GetSourceInt(&source).intVal;
    result.internals.intVal = BG_GetWeaponOptionUnlockPLevel(v3.intVal);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetWeaponOptionUnlockPLevel() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetItemRef(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    index = GetSourceInt(&source).intVal;
    result.internals.intVal = (int)BG_UnlockablesGetItemRef(index);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetItemRef() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetItemName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    Operand source; // [esp+0h] [ebp-10h] BYREF
    Operand result; // [esp+8h] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    v3.intVal = GetSourceInt(&source).intVal;
    result.internals.intVal = (int)BG_UnlockablesGetItemName(v3.intVal);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetItemName() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetItemImage(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    index = GetSourceInt(&source).intVal;
    result.internals.intVal = (int)BG_UnlockablesGetItemImage(index);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetItemImage() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetItemUnlockLevel(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    result.internals.intVal = BG_UnlockablesGetItemUnlockLevel(index);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetItemUnlockLevel() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetItemUnlockPLevel(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    result.internals.intVal = BG_UnlockablesGetItemUnlockPLevel(index);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetItemUnlockLevel() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetItemCost(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    result.internals.intVal = BG_UnlockablesGetItemCost(index);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetItemCost() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetItemDualWieldCost(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    result.internals.intVal = BG_UnlockablesGetItemDualWieldCost(index);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetCurrentItemDualWieldCost() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetItemSellPrice(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    result.internals.intVal = BG_UnlockablesGetItemSellPrice(index);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetItemSellPrice() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetItemCount(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    result.internals.intVal = BG_UnlockablesGetItemCount(index);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetItemCount() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetItemGroup(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    Operand source; // [esp+0h] [ebp-10h] BYREF
    Operand result; // [esp+8h] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    v3.intVal = GetSourceInt(&source).intVal;
    result.internals.intVal = (int)BG_UnlockablesGetItemGroup(v3.intVal);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetItemGroup() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetItemDesc(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    index = GetSourceInt(&source).intVal;
    result.internals.intVal = (int)BG_UnlockablesGetItemDesc(index);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetItemDesc() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetItemIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    const char *itemName; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    itemName = GetSourceString(source);
    result.internals.intVal = BG_UnlockablesGetItemIndexFromName(itemName);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetItemIndex() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetItemDualWieldIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    result.internals.intVal = BG_UnlockablesGetItemDualWieldIndex(index);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetItemDualWieldIndex() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetItemDualWieldBaseIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    result.internals.intVal = BG_UnlockablesGetItemDualWieldBaseIndex(index);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetItemDualWieldBaseIndex() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetDefaultClassSlot(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-68h] BYREF
    OperandList list; // [esp+8h] [ebp-60h] BYREF
    const char *className; // [esp+60h] [ebp-8h]
    const char *slotName; // [esp+64h] [ebp-4h]

    GetOperandList(dataStack, &list);
    if ( list.operandCount == 2 )
    {
        result.dataType = VAL_INT;
        className = GetSourceString(list.operands[0]);
        slotName = GetSourceString(list.operands[1]);
        result.internals.intVal = BG_UnlockablesGetDefaultClassSlotIndexFromName(className, slotName);
        if ( uiscript_debug )
        {
            if ( uiscript_debug->current.integer )
                Expression_TraceInternal("GetDefaultClassSlot() = %i\n", result.internals.intVal);
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Expression_Error("Expected 2 parameters to GetDefaultClassSlot()\n");
        result.dataType = VAL_INT;
        result.internals.intVal = 0;
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetItemAttachment(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-68h] BYREF
    OperandList list; // [esp+8h] [ebp-60h] BYREF
    int attachmentNum; // [esp+60h] [ebp-8h]
    int itemIndex; // [esp+64h] [ebp-4h]

    GetOperandList(dataStack, &list);
    result.dataType = VAL_INT;
    if ( list.operandCount == 2 )
    {
        itemIndex = GetOperandValueInt(list.operands).intVal;
        attachmentNum = GetOperandValueInt(&list.operands[1]).intVal;
        result.internals.intVal = BG_UnlockablesGetItemAttachment(itemIndex, attachmentNum);
        if ( uiscript_debug )
        {
            if ( uiscript_debug->current.integer )
                Expression_TraceInternal("GetItemAttachment() = %d\n", result.internals.intVal);
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Expression_Error("Expected 2 parameters to GetItemAttachment()\n");
        result.internals.intVal = 0;
        AddOperandToStack(dataStack, &result);
    }
}

operandInternalDataUnion __cdecl GetOperandValueInt(Operand *source)
{
    if ( source->dataType == VAL_STRING )
    {
        return (operandInternalDataUnion)atoi(source->internals.string);
    }
    else if ( source->dataType == VAL_FLOAT )
    {
        return (operandInternalDataUnion)(int)source->internals.floatVal;
    }
    else
    {
        return source->internals;
    }
}

void __cdecl GetCurrentItemMultiText(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *VariantString; // eax
    int j; // [esp+0h] [ebp-1Ch]
    int i; // [esp+4h] [ebp-18h]
    multiDef_s *multiPtr; // [esp+8h] [ebp-14h]
    Operand result; // [esp+Ch] [ebp-10h] BYREF
    const char *string; // [esp+14h] [ebp-8h]
    float value; // [esp+18h] [ebp-4h]

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( !item
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    2911,
                    0,
                    "%s",
                    "item") )
    {
        __debugbreak();
    }
    if ( !item->parent
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    2912,
                    0,
                    "%s",
                    "item->parent") )
    {
        __debugbreak();
    }
    if ( item->dataType == 10 )
    {
        multiPtr = Item_GetMultiDef(item);
        if ( multiPtr->strDef )
        {
            string = Dvar_GetVariantString(item->dvar);
            for ( i = 0; i < multiPtr->count; ++i )
            {
                if ( !I_stricmp(string, multiPtr->dvarStr[i]) )
                {
                    result.internals.intVal = (int)multiPtr->dvarList[i];
                    break;
                }
            }
        }
        else
        {
            VariantString = Dvar_GetVariantString(item->dvar);
            value = atof(VariantString);
            for ( j = 0; j < multiPtr->count; ++j )
            {
                if ( multiPtr->dvarValue[j] == value )
                {
                    result.internals.intVal = (int)multiPtr->dvarList[j];
                    break;
                }
            }
        }
        if ( uiscript_debug )
        {
            if ( uiscript_debug->current.integer )
                Expression_TraceInternal("GetCurrentItemMultiText() = %d\n", result.internals.intVal);
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Expression_Error("GetCurrentItemMultiText() called on an item that is not an ITEM_TYPE_MULTI\n");
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetClanMOTD(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetNumLives(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+4h] [ebp-18h] BYREF
    cg_s *cgameGlob; // [esp+Ch] [ebp-10h]
    int numDeaths; // [esp+10h] [ebp-Ch]
    const playerState_s *ps; // [esp+14h] [ebp-8h]
    int numLives; // [esp+18h] [ebp-4h]

    numDeaths = -1;
    numLives = -1;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = CG_GetPredictedPlayerState(localClientNum);
    if ( cgameGlob->bgs.clientinfo[ps->clientNum].infoValid )
        numDeaths = cgameGlob->bgs.clientinfo[ps->clientNum].score.deaths;
    if ( numDeaths >= 0 )
    {
        numLives = Dvar_GetInt("scr_numLives") - numDeaths;
        if ( numLives < 0 )
            numLives = -1;
    }
    result.dataType = VAL_INT;
    result.internals.intVal = numLives;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetNumLives() = %d\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl PlayersAlive(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int clientNum; // [esp+4h] [ebp-20h]
    Operand source; // [esp+8h] [ebp-1Ch] BYREF
    Operand result; // [esp+10h] [ebp-14h] BYREF
    int team; // [esp+18h] [ebp-Ch]
    cg_s *cgameGlob; // [esp+1Ch] [ebp-8h]
    int count; // [esp+20h] [ebp-4h]

    GetOperand(dataStack, &source);
    count = 0;
    team = 6;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( I_stricmp(source.internals.string, "TEAM_ALLIES") )
    {
        if ( !I_stricmp(source.internals.string, "TEAM_AXIS") )
            team = 1;
    }
    else
    {
        team = 2;
    }
    for ( clientNum = 0; clientNum < 32; ++clientNum )
    {
        if ( cgameGlob->bgs.clientinfo[clientNum].infoValid
            && (team == 6 || team == cgameGlob->bgs.clientinfo[clientNum].team)
            && !cgameGlob->bgs.clientinfo[clientNum].score.status_icon )
        {
            ++count;
        }
    }
    result.dataType = VAL_INT;
    result.internals.intVal = count;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("PlayersAlive() = %d\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetPlayerStatByName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    char *v4; // eax
    char *v5; // eax
    char *SourceString; // [esp-4h] [ebp-14h]
    operandInternalDataUnion v7; // [esp-4h] [ebp-14h]
    Operand source; // [esp+0h] [ebp-10h] BYREF
    Operand result; // [esp+8h] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    result.internals.intVal = 0;
    result.dataType = VAL_INT;
    SourceString = GetSourceString(source);
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( !LiveStats_GetIntPlayerStat(ControllerIndex, &result.internals.intVal, SourceString) )
    {
        v4 = GetSourceString(source);
        Expression_Error("UI Expression Error: Error retrieving PlayerStat %s\n", v4);
    }
    if ( uiscript_debug && uiscript_debug->current.integer )
    {
        v7.intVal = (int)result.internals;
        v5 = GetSourceString(source);
        Expression_TraceInternal("getstatbyname( %s ) = %i\n", v5, v7.intVal);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetLowestLocalCP(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.internals.intVal = 0;
    result.dataType = VAL_INT;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( !LiveStats_GetIntPlayerStatByKey(ControllerIndex, &result.internals.intVal, MP_PLAYERSTATSKEY_CODPOINTS) )
        Expression_Error("UI Expression Error: Error retrieving CP for local client %d\n", localClientNum);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("getlowestlocalcp() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsWagerServer(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *v3; // eax
    Operand result; // [esp+0h] [ebp-410h] BYREF
    char buff[1028]; // [esp+8h] [ebp-408h] BYREF

    LAN_GetServerInfo(
        ui_netSource->current.integer,
        sharedUiInfo.serverStatus.displayServers[sharedUiInfo.serverStatus.currentServer],
        buff,
        1024);
    result.dataType = VAL_INT;
    v3 = Info_ValueForKey(buff, "wagerbet");
    result.internals.intVal = atoi(v3);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetContextHeight(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *v3; // eax
    Operand result; // [esp+0h] [ebp-410h] BYREF
    char buff[1028]; // [esp+8h] [ebp-408h] BYREF

    LAN_GetServerInfo(
        ui_netSource->current.integer,
        sharedUiInfo.serverStatus.displayServers[sharedUiInfo.serverStatus.currentServer],
        buff,
        1024);
    result.dataType = VAL_INT;
    v3 = Info_ValueForKey(buff, "wagerbet");
    result.internals.intVal = atoi(v3) != 0 ? 62 : 80;
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetKeyBinding(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *NameForValueType; // eax
    Operand source; // [esp+0h] [ebp-10h] BYREF
    Operand result; // [esp+8h] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    if ( source.dataType == VAL_STRING )
    {
        UI_GetKeyBindingLocalizedString(localClientNum, source.internals.string, resultString, 0);
        result.dataType = VAL_STRING;
        result.internals.intVal = (int)resultString;
    }
    else
    {
        NameForValueType = GetNameForValueType(source.dataType);
        Expression_Error(" Must use a string as KeyBinding() parameter, not a %s\n", NameForValueType);
        result.dataType = VAL_STRING;
        result.internals.intVal = (int)"";
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetActionSlotUsable(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int slotId; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    slotId = GetOperandValueInt(&source).intVal;
    if ( slotId >= 1 && slotId <= 4 )
    {
        --slotId;
        result.dataType = VAL_INT;
        result.internals.intVal = CG_ActionSlotIsUsable(localClientNum, slotId) != 0;
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Expression_Error("UI Expression Error: ActionSlot() slot ID should be in range (1,%i) not %i.\n", 4, slotId);
        result.dataType = VAL_INT;
        result.internals.intVal = 0;
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetHudFade(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *v3; // eax
    const char *NameForValueType; // eax
    Operand source; // [esp+0h] [ebp-10h] BYREF
    Operand result; // [esp+8h] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    result.dataType = VAL_FLOAT;
    if ( CL_LocalClient_IsCUIFlagSet(localClientNum, 32) )
    {
        if ( source.dataType == VAL_STRING )
        {
            if ( I_stricmp(source.internals.string, "dpad") )
            {
                if ( I_stricmp(source.internals.string, "weapon") )
                {
                    if ( I_stricmp(source.internals.string, "compass") )
                    {
                        if ( I_stricmp(source.internals.string, "scoreboard") )
                        {
                            NameForValueType = GetNameForValueType(source.dataType);
                            Expression_Error(
                                " Argument to HudFade() must be \"dpad\", \"compass\", \"scoreboard\", or \"weapon\".\n",
                                NameForValueType);
                            result.internals.intVal = 0;
                        }
                        else
                        {
                            result.internals.floatVal = 1.0f;
                        }
                    }
                    else
                    {
                        result.internals.floatVal = CG_GetHudAlphaCompass(localClientNum);
                    }
                }
                else
                {
                    result.internals.floatVal = CG_GetHudAlphaAmmoCounter(localClientNum);
                }
            }
            else
            {
                result.internals.floatVal = CG_GetHudAlphaDPad(localClientNum);
            }
            AddOperandToStack(dataStack, &result);
        }
        else
        {
            v3 = GetNameForValueType(source.dataType);
            Expression_Error(" Must use a string as HudFade() parameter, not a %s\n", v3);
            result.internals.intVal = 0;
            AddOperandToStack(dataStack, &result);
        }
    }
    else
    {
        result.internals.intVal = 0;
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl IsMenuOpen(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *NameForValueType; // eax
    Operand source; // [esp+0h] [ebp-10h] BYREF
    Operand result; // [esp+8h] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    if ( source.dataType == VAL_STRING )
    {
        result.dataType = VAL_INT;
        result.internals.intVal = Menu_IsMenuOpenAndVisible(localClientNum, source.internals.string);
        if ( uiscript_debug )
        {
            if ( uiscript_debug->current.integer )
                Expression_TraceInternal("ismenuopen( %s ) = %i\n", source.internals.string, result.internals.intVal);
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        NameForValueType = GetNameForValueType(source.dataType);
        Expression_Error(" Must use a string as the name of a menu, not a %s\n", NameForValueType);
        result.dataType = VAL_INT;
        result.internals.intVal = 0;
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl WritingData(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("writingdata() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetAdsJavelin(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = CL_GetLocalClientConnectionState(localClientNum) >= 8 && CG_JavelinADS(localClientNum);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("adsjavelin() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetWeapLockBlink(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+Ch] [ebp-14h] BYREF
    Operand result; // [esp+14h] [ebp-Ch] BYREF
    float bps; // [esp+1Ch] [ebp-4h]

    GetOperand(dataStack, &source);
    bps = GetSourceFloat(&source);
    result.dataType = VAL_INT;
    result.internals.intVal = CL_GetLocalClientConnectionState(localClientNum) >= 8
                                                 && CG_UICheckWeapLockBlink(localClientNum, bps);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("weaplockblink( %.2f ) = %i\n", bps, result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetWeapAttackTop(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = CL_GetLocalClientConnectionState(localClientNum) >= 8
                                                 && CG_UICheckWeapLockAttackTop(localClientNum);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("weapattacktop() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetWeapAttackDirect(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = CL_GetLocalClientConnectionState(localClientNum) >= 8
                                                 && CG_UICheckWeapLockAttackDirect(localClientNum);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("weapattackdirect() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl ToUpper(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    const char *origString; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    origString = GetSourceString(source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)UI_ToUpper(origString);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("toUpper() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetPlaylistName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    unsigned int PlaylistIdForNum; // eax
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int controllerIndex; // [esp+8h] [ebp-4h]

    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.dataType = VAL_STRING;
    PlaylistIdForNum = Playlist_GetPlaylistIdForNum(controllerIndex, playlist->current.integer);
    result.internals.intVal = (int)Playlist_GetPlaylistName(controllerIndex, PlaylistIdForNum);
    if ( !result.internals.intVal || !*(_BYTE *)result.internals.intVal )
        result.internals.intVal = (int)"";
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("getPlaylistName() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl SecondsAsTimeDisplay(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    char *v8; // eax
    char *v9; // eax
    char *v10; // eax
    char *v11; // eax
    char *v12; // eax
    char *v13; // eax
    int v14; // [esp-4h] [ebp-24h]
    int v15; // [esp-4h] [ebp-24h]
    int v16; // [esp-4h] [ebp-24h]
    int v17; // [esp-4h] [ebp-24h]
    Operand source; // [esp+0h] [ebp-20h] BYREF
    Operand result; // [esp+8h] [ebp-18h] BYREF
    int hours; // [esp+10h] [ebp-10h]
    int seconds; // [esp+14h] [ebp-Ch]
    int minutes; // [esp+18h] [ebp-8h]
    int days; // [esp+1Ch] [ebp-4h]

    GetOperand(dataStack, &source);
    resultString_0[0] = 0;
    v3.intVal = GetSourceInt(&source).intVal;
    days = v3.intVal / 86400;
    hours = v3.intVal % 86400 / 3600;
    minutes = v3.intVal % 3600 / 60;
    seconds = v3.intVal % 60;
    if ( v3.intVal / 86400 )
    {
        v14 = days;
        v4 = UI_SafeTranslateString("MPUI_TIMEDISPLAY_DAYS");
        v5 = UI_ReplaceConversionInt(v4, v14);
        I_strncat(resultString_0, 128, v5);
        I_strncat(resultString_0, 128, " ");
    }
    if ( hours )
    {
        v15 = hours;
        v6 = UI_SafeTranslateString("MPUI_TIMEDISPLAY_HOURS");
        v7 = UI_ReplaceConversionInt(v6, v15);
        I_strncat(resultString_0, 128, v7);
        I_strncat(resultString_0, 128, " ");
    }
    if ( minutes )
    {
        v16 = minutes;
        v8 = UI_SafeTranslateString("MPUI_TIMEDISPLAY_MINUTES");
        v9 = UI_ReplaceConversionInt(v8, v16);
        I_strncat(resultString_0, 128, v9);
        I_strncat(resultString_0, 128, " ");
    }
    if ( seconds )
    {
        v17 = seconds;
        v10 = UI_SafeTranslateString("MPUI_TIMEDISPLAY_SECONDS");
        v11 = UI_ReplaceConversionInt(v10, v17);
        I_strncat(resultString_0, 128, v11);
        I_strncat(resultString_0, 128, " ");
    }
    if ( !days && !hours && !minutes && !seconds )
    {
        I_strncat(resultString_0, 128, "< ");
        v12 = UI_SafeTranslateString("MPUI_TIMEDISPLAY_SECONDS");
        v13 = UI_ReplaceConversionInt(v12, 1);
        I_strncat(resultString_0, 128, v13);
        I_strncat(resultString_0, 128, " ");
    }
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)resultString_0;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("secondsToTime() = %s\n", resultString_0);
    AddOperandToStack(dataStack, &result);
}

void __cdecl SecondsAsCountdownDisplay(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int timeInSec; // [esp+0h] [ebp-70h]
    Operand result; // [esp+4h] [ebp-6Ch] BYREF
    int msec; // [esp+Ch] [ebp-64h]
    OperandList list; // [esp+10h] [ebp-60h] BYREF
    bool showMs; // [esp+6Bh] [ebp-5h]
    int sec; // [esp+6Ch] [ebp-4h]

    GetOperandList(dataStack, &list);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)resultString_1;
    if ( list.operandCount <= 2 )
    {
        if ( list.operandCount == 2 )
        {
            timeInSec = GetOperandValueInt(list.operands).intVal;
            showMs = GetOperandValueInt(&list.operands[1]).intVal != 0;
        }
        else
        {
            timeInSec = GetOperandValueInt(list.operands).intVal;
            showMs = 0;
        }
        if ( timeInSec < 0 )
            timeInSec = 0;
        sec = timeInSec / 1000;
        msec = timeInSec % 1000;
        if ( showMs )
            _snprintf(resultString_1, 0x80u, "%2i:%02i.%i", sec / 60, sec % 60, msec / 100);
        else
            _snprintf(resultString_1, 0x80u, "%2i:%02i", sec / 60, sec % 60);
        if ( uiscript_debug )
        {
            if ( uiscript_debug->current.integer )
                Expression_TraceInternal("secondsToCountdown() = %s\n", resultString_1);
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        result.internals.intVal = (int)"";
        Expression_Error("Expected 1 or 2 parameters to secondsAsCountdown()\n");
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetTimeLeft(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int timeLeft; // [esp+0h] [ebp-14h]
    Operand result; // [esp+4h] [ebp-10h] BYREF
    cg_s *cgameGlob; // [esp+Ch] [ebp-8h]
    cgs_t *cgs; // [esp+10h] [ebp-4h]

    if ( CL_GetLocalClientConnectionState(localClientNum) >= 8 )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        timeLeft = cgs->gameEndTime - cgameGlob->nextSnap->serverTime;
        result.dataType = VAL_INT;
        result.internals.intVal = timeLeft;
    }
    else
    {
        result.dataType = VAL_INT;
        result.internals.intVal = 0;
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetGameTime(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-10h] BYREF
    cg_s *cgameGlob; // [esp+8h] [ebp-8h]
    cgs_t *cgs; // [esp+Ch] [ebp-4h]

    if ( CL_GetLocalClientConnectionState(localClientNum) < 8 )
    {
        result.dataType = VAL_INT;
        result.internals.intVal = 0;
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    result.dataType = VAL_INT;
    result.internals.intVal = cgameGlob->nextSnap->serverTime;
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetBombTime(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-18h] BYREF
    Operand result; // [esp+8h] [ebp-10h] BYREF
    cg_s *cgameGlob; // [esp+10h] [ebp-8h]
    int bombIndex; // [esp+14h] [ebp-4h]

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    GetOperand(dataStack, &source);
    bombIndex = GetSourceInt(&source).intVal;
    if ( bombIndex < 2 )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        result.internals.intVal = cgameGlob->bombTimer[bombIndex];
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Expression_Error(" The bomb index %d is not less than BOMB_TIMER_COUNT\n", bombIndex);
    }
}

void __cdecl GetGametypeObjective(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    result.dataType = VAL_STRING;
    if ( CL_GetLocalClientConnectionState(localClientNum) >= 8 )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        result.internals.intVal = (int)CG_GetGametypeDescription(localClientNum);
        if ( !result.internals.intVal )
            result.internals.intVal = (int)"";
    }
    else
    {
        result.internals.intVal = (int)"";
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetPlaylistMaxPartySize(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    int v4; // eax
    int integer; // [esp-4h] [ebp-10h]
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int playlistId; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    integer = playlist->current.integer;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    playlistId = Playlist_GetPlaylistIdForNum(ControllerIndex, integer);
    v4 = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = Playlist_GetMaxPartySize(v4, playlistId);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetPlaylistMaxPartySize() = %d\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetGametypeName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    cgs_t *cgs; // [esp+8h] [ebp-4h]

    result.dataType = VAL_STRING;
    if ( CL_GetLocalClientConnectionState(localClientNum) >= 8 )
    {
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        result.internals.intVal = (int)UI_GetGameTypeDisplayName(cgs->gametype);
    }
    else if ( g_gametype )
    {
        result.internals.intVal = (int)UI_GetGameTypeDisplayName(g_gametype->current.string);
    }
    else
    {
        result.internals.intVal = (int)"";
    }
    if ( !result.internals.intVal )
        result.internals.intVal = (int)"";
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetGametypeInternal(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-10h] BYREF
    cg_s *cgameGlob; // [esp+8h] [ebp-8h]
    cgs_t *cgs; // [esp+Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    result.dataType = VAL_STRING;
    if ( CL_GetLocalClientConnectionState(localClientNum) >= 8 )
        result.internals.intVal = (int)cgs->gametype;
    else
        result.internals.intVal = g_gametype->current.integer;
    if ( !result.internals.intVal )
        result.internals.intVal = (int)"";
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetScore(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *NameForValueType; // eax
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    const score_s *score; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( source.dataType )
    {
        NameForValueType = GetNameForValueType(source.dataType);
        Expression_Error(" Must use an integer for the rank: %s\n", NameForValueType);
        AddOperandToStack(dataStack, &result);
    }
    else if ( source.internals.intVal > 0 )
    {
        score = UI_GetScoreAtRank(localClientNum, source.internals.intVal);
        if ( score )
            result.internals.intVal = score->score;
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Expression_Error(" rank must be > 0: %i\n", source.internals.intVal);
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetGameMessageWindowActive(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int windowIndex; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    windowIndex = GetSourceInt(&source).intVal;
    if ( Con_IsValidGameMessageWindow(windowIndex) )
        result.internals.intVal = Con_IsGameMessageWindowActive(localClientNum, windowIndex);
    else
        Expression_Error("UI Script error: gameMsgWndActive was passed an invalid window index\n");
    AddOperandToStack(dataStack, &result);
}

void __cdecl UI_GetOnlineFriendCount(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int controllerIndex; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = Friends_GetCount(controllerIndex, 4);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetFollowing(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    result.internals.intVal = (cgameGlob->predictedPlayerState.otherFlags & 0x1A) != 0;
    if ( (cgameGlob->predictedPlayerState.otherFlags & 0x18) != 0 )
        result.internals.intVal = 1;
    AddOperandToStack(dataStack, &result);
}

void __cdecl CanSpecCycle(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    result.internals.intVal = (cgameGlob->predictedPlayerState.otherFlags & 8) != 0;
    AddOperandToStack(dataStack, &result);
}

void __cdecl CanSpecFree(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    result.internals.intVal = (cgameGlob->predictedPlayerState.otherFlags & 0x10) != 0;
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetClientInPlace(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-1Ch] BYREF
    Operand result; // [esp+8h] [ebp-14h] BYREF
    cg_s *cgameGlob; // [esp+10h] [ebp-Ch]
    int place; // [esp+14h] [ebp-8h]
    int clientNum; // [esp+18h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = -1;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    place = GetSourceInt(&source).intVal;
    if ( (unsigned int)(place - 1) >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    3983,
                    0,
                    "place-1 doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    place - 1,
                    32) )
    {
        __debugbreak();
    }
    clientNum = cgameGlob->teamPlayers[place + 3];
    if ( (unsigned int)clientNum < 0x20 )
        result.internals.intVal = clientNum;
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetPlaceWithTiesForScore(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int score; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    score = GetSourceInt(&source).intVal;
    result.dataType = VAL_INT;
    result.internals.intVal = CG_GetPlaceWithTiesForScore(localClientNum, score);
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemLayerName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    const char *LayerName; // eax
    Operand source; // [esp+0h] [ebp-10h] BYREF
    Operand result; // [esp+8h] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    v3.intVal = GetOperandValueInt(&source).intVal;
    LayerName = UI_EmblemGetLayerName(v3.intVal);
    result.internals.intVal = (int)CopyTempString(LayerName);
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemLayerState(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int UnlockedLayerCount; // eax
    int PurchasedLayerCount; // eax
    Operand source; // [esp+0h] [ebp-18h] BYREF
    Operand result; // [esp+8h] [ebp-10h] BYREF
    int layer; // [esp+10h] [ebp-8h]
    int controllerIndex; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    layer = GetOperandValueInt(&source).intVal;
    UnlockedLayerCount = BG_EmblemsGetUnlockedLayerCount(controllerIndex);
    if ( layer < UnlockedLayerCount )
    {
        PurchasedLayerCount = BG_EmblemsGetPurchasedLayerCount(controllerIndex);
        if ( layer < PurchasedLayerCount )
        {
            if ( *UI_EmblemGetLayerName(layer) )
                result.internals.intVal = 3;
            else
                result.internals.intVal = 2;
        }
        else
        {
            result.internals.intVal = 1;
        }
    }
    else
    {
        result.internals.intVal = 0;
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemLayerCanOutline(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int layer; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    layer = GetOperandValueInt(&source).intVal;
    result.internals.intVal = UI_EmblemCanOutlineLayer(layer);
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemLayerCanDuplicate(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-18h] BYREF
    Operand result; // [esp+8h] [ebp-10h] BYREF
    int layer; // [esp+10h] [ebp-8h]
    int controllerIndex; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    layer = GetOperandValueInt(&source).intVal;
    result.internals.intVal = UI_EmblemCanDuplicateLayer(controllerIndex);
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemSelectedLayer(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = UI_EmblemGetSelectedLayer();
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemLayerCost(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int layer; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    layer = GetOperandValueInt(&source).intVal;
    result.internals.intVal = BG_EmblemsGetLayerCost(layer);
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemLayerUnlockLevel(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int layer; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    layer = GetOperandValueInt(&source).intVal;
    result.internals.intVal = BG_EmblemsGetLayerUnlockLevel(layer);
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemIconName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    __int16 floatVal; // ax
    const char *IconDesc; // eax
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int controllerIndex; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    floatVal = (unsigned __int16)GetOperandValueInt(&source).floatVal;
    IconDesc = BG_EmblemsGetIconDesc(controllerIndex, floatVal);
    result.internals.intVal = (int)CopyTempString(IconDesc);
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemIconUnlockDesc(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    __int16 floatVal; // ax
    const char *IconUnlockDesc; // eax
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int controllerIndex; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    floatVal = (unsigned __int16)GetOperandValueInt(&source).floatVal;
    IconUnlockDesc = BG_EmblemsGetIconUnlockDesc(controllerIndex, floatVal);
    result.internals.intVal = (int)CopyTempString(IconUnlockDesc);
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemIconCost(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    __int16 floatVal; // ax
    Operand source; // [esp+0h] [ebp-10h] BYREF
    Operand result; // [esp+8h] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    floatVal = (unsigned __int16)GetOperandValueInt(&source).floatVal;
    result.internals.intVal = BG_EmblemsGetIconCost(floatVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemIconState(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-18h] BYREF
    Operand result; // [esp+8h] [ebp-10h] BYREF
    int controllerIndex; // [esp+10h] [ebp-8h]
    int id; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    id = GetOperandValueInt(&source).intVal;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( id == -1 )
    {
        result.internals.intVal = 4;
    }
    else if ( BG_EmblemsIsIconClassified(controllerIndex, id) )
    {
        result.internals.intVal = 0;
    }
    else if ( BG_EmblemsIsIconLocked(controllerIndex, id) )
    {
        result.internals.intVal = 1;
    }
    else if ( BG_EmblemsIsIconPurchased(controllerIndex, id) )
    {
        result.internals.intVal = 3;
    }
    else
    {
        result.internals.intVal = 2;
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemPurchasedLayerCount(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int controllerIndex; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = BG_EmblemsGetPurchasedLayerCount(controllerIndex);
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemIsModified(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int controllerIndex; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = UI_EmblemIsModified(controllerIndex);
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemBackgroundCount(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = BG_EmblemsGetBackgroundCount();
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemBackgroundIsLocked(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    Operand source; // [esp+0h] [ebp-18h] BYREF
    Operand result; // [esp+8h] [ebp-10h] BYREF
    int controllerIndex; // [esp+10h] [ebp-8h]
    __int16 id; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    v3.intVal = GetOperandValueInt(&source).intVal;
    id = BG_EmblemsGetBackgroundID(v3.intVal);
    result.internals.intVal = BG_EmblemsIsBackgroundLocked(controllerIndex, id) != 0;
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemBackgroundIsClassified(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    Operand source; // [esp+0h] [ebp-18h] BYREF
    Operand result; // [esp+8h] [ebp-10h] BYREF
    int controllerIndex; // [esp+10h] [ebp-8h]
    __int16 id; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    v3.intVal = GetOperandValueInt(&source).intVal;
    id = BG_EmblemsGetBackgroundID(v3.intVal);
    result.internals.intVal = BG_EmblemsIsBackgroundClassified(controllerIndex, id);
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemBackgroundIsPurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    Operand source; // [esp+0h] [ebp-18h] BYREF
    Operand result; // [esp+8h] [ebp-10h] BYREF
    int controllerIndex; // [esp+10h] [ebp-8h]
    __int16 id; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    v3.intVal = GetOperandValueInt(&source).intVal;
    id = BG_EmblemsGetBackgroundID(v3.intVal);
    result.internals.intVal = BG_EmblemsIsBackgroundPurchased(controllerIndex, id);
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemBackgroundMaterial(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    Operand source; // [esp+0h] [ebp-18h] BYREF
    Operand result; // [esp+8h] [ebp-10h] BYREF
    Material *mat; // [esp+10h] [ebp-8h]
    __int16 id; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    v3.intVal = GetOperandValueInt(&source).intVal;
    id = BG_EmblemsGetBackgroundID(v3.intVal);
    mat = BG_EmblemsGetBackgroundMaterial(id);
    if ( mat )
        result.internals.intVal = (int)mat->info.name;
    else
        result.internals.intVal = (int)"";
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemPlayerBackgroundMaterial(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *SourceString; // eax
    Operand result; // [esp+0h] [ebp-7Ch] BYREF
    int controllerIndex; // [esp+8h] [ebp-74h]
    OperandList list; // [esp+Ch] [ebp-70h] BYREF
    int useServer; // [esp+68h] [ebp-14h]
    Material *mat; // [esp+6Ch] [ebp-10h]
    __int16 id; // [esp+70h] [ebp-Ch]
    unsigned __int64 xuid; // [esp+74h] [ebp-8h]

    GetOperandList(dataStack, &list);
    useServer = GetSourceInt(&list.operands[1]).intVal;
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( list.operandCount >= 2 )
    {
        controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        SourceString = GetSourceString(list.operands[0]);
        xuid = I_atoi64(SourceString);
        if ( useServer )
            id = PCache_GetPlayerEmblemBackgroundID(-1, xuid);
        else
            id = PCache_GetPlayerEmblemBackgroundID(controllerIndex, xuid);
        mat = BG_EmblemsGetBackgroundMaterial(id);
        if ( mat )
            result.internals.intVal = (int)mat->info.name;
        else
            result.internals.intVal = (int)"emblem_bg_nocod";
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemSelectedBackground(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    __int16 id; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    id = UI_EmblemGetSelectedBackgroundID();
    result.internals.intVal = BG_EmblemsGetBackgroundIndex(id);
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemBackgroundName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    const char *BackgroundDesc; // eax
    Operand source; // [esp+0h] [ebp-18h] BYREF
    Operand result; // [esp+8h] [ebp-10h] BYREF
    int controllerIndex; // [esp+10h] [ebp-8h]
    __int16 id; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    v3.intVal = GetOperandValueInt(&source).intVal;
    id = BG_EmblemsGetBackgroundID(v3.intVal);
    BackgroundDesc = BG_EmblemsGetBackgroundDesc(controllerIndex, id);
    result.internals.intVal = (int)CopyTempString(BackgroundDesc);
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemBackgroundUnlockDesc(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    const char *BackgroundUnlockDesc; // eax
    Operand source; // [esp+0h] [ebp-18h] BYREF
    Operand result; // [esp+8h] [ebp-10h] BYREF
    int controllerIndex; // [esp+10h] [ebp-8h]
    __int16 id; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    v3.intVal = GetOperandValueInt(&source).intVal;
    id = BG_EmblemsGetBackgroundID(v3.intVal);
    BackgroundUnlockDesc = BG_EmblemsGetBackgroundUnlockDesc(controllerIndex, id);
    result.internals.intVal = (int)CopyTempString(BackgroundUnlockDesc);
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemStateDisplay(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    char *StateDisplay; // eax
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int controllerIndex; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    v3.intVal = GetOperandValueInt(&source).intVal;
    StateDisplay = UI_EmblemGetStateDisplay(controllerIndex, v3.intVal);
    result.internals.intVal = (int)CopyTempString(StateDisplay);
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemCategoryDisplay(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    char *CategoryDisplay; // eax
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int controllerIndex; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    v3.intVal = GetOperandValueInt(&source).intVal;
    CategoryDisplay = UI_EmblemGetCategoryDisplay(controllerIndex, v3.intVal);
    result.internals.intVal = (int)CopyTempString(CategoryDisplay);
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemFilterCount(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    int intVal; // [esp-4h] [ebp-68h]
    int controllerIndex; // [esp+0h] [ebp-64h]
    Operand result; // [esp+4h] [ebp-60h] BYREF
    OperandList list; // [esp+Ch] [ebp-58h] BYREF

    GetOperandList(dataStack, &list);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( list.operandCount == 2 )
    {
        controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        intVal = GetOperandValueInt(&list.operands[1]).intVal;
        v3.intVal = GetOperandValueInt(list.operands).intVal;
        result.internals.intVal = UI_EmblemGetFilterCount(controllerIndex, v3.intVal, intVal);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemFilterIconID(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    int v4; // [esp-8h] [ebp-6Ch]
    int intVal; // [esp-4h] [ebp-68h]
    int controllerIndex; // [esp+0h] [ebp-64h]
    Operand result; // [esp+4h] [ebp-60h] BYREF
    OperandList list; // [esp+Ch] [ebp-58h] BYREF

    GetOperandList(dataStack, &list);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( list.operandCount == 3 )
    {
        controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        intVal = GetOperandValueInt(&list.operands[2]).intVal;
        v4 = GetOperandValueInt(&list.operands[1]).intVal;
        v3.intVal = GetOperandValueInt(list.operands).intVal;
        result.internals.intVal = UI_EmblemGetFilterIconID(controllerIndex, v3.intVal, v4, intVal);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl EmblemBackgroundCost(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    __int16 id; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    v3.intVal = GetOperandValueInt(&source).intVal;
    id = BG_EmblemsGetBackgroundID(v3.intVal);
    result.internals.intVal = BG_EmblemsGetBackgroundCost(id);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GridMove(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int v3; // [esp+0h] [ebp-8Ch]
    Operand result; // [esp+8h] [ebp-84h] BYREF
    int pages; // [esp+10h] [ebp-7Ch]
    OperandList list; // [esp+14h] [ebp-78h] BYREF
    int page; // [esp+6Ch] [ebp-20h]
    int idx; // [esp+70h] [ebp-1Ch]
    int row; // [esp+74h] [ebp-18h]
    int rows; // [esp+78h] [ebp-14h]
    const char *direction; // [esp+7Ch] [ebp-10h]
    int count; // [esp+80h] [ebp-Ch]
    int cols; // [esp+84h] [ebp-8h]
    int col; // [esp+88h] [ebp-4h]

    GetOperandList(dataStack, &list);
    rows = GetOperandValueInt(list.operands).intVal;
    cols = GetOperandValueInt(&list.operands[1]).intVal;
    count = GetOperandValueInt(&list.operands[2]).intVal;
    idx = GetOperandValueInt(&list.operands[3]).intVal;
    direction = GetSourceString(list.operands[4]);
    pages = (count + cols * rows - 1) / (cols * rows);
    page = idx / (cols * rows);
    col = idx % cols;
    row = idx / cols % rows;
    result.dataType = VAL_INT;
    result.internals.intVal = idx;
    if ( I_stricmp(direction, "up") )
    {
        if ( I_stricmp(direction, "down") )
        {
            if ( I_stricmp(direction, "left") )
            {
                if ( I_stricmp(direction, "right") )
                {
                    if ( I_stricmp(direction, "page_left") )
                    {
                        if ( !I_stricmp(direction, "page_right") )
                            ++page;
                    }
                    else
                    {
                        --page;
                    }
                }
                else if ( col >= cols - 1 || idx >= count - 1 )
                {
                    col = 0;
                    ++page;
                }
                else
                {
                    ++col;
                }
            }
            else if ( col <= 0 )
            {
                col = cols - 1;
                --page;
            }
            else
            {
                --col;
            }
        }
        else if ( row < rows - 1 )
        {
            ++row;
        }
    }
    else if ( row > 0 )
    {
        --row;
    }
    if ( pages )
    {
        if ( page < 0 )
            page += pages;
        if ( page >= pages )
            page -= pages;
        idx = col + page * cols * rows + cols * row;
        if ( count - 1 < idx )
            v3 = count - 1;
        else
            v3 = idx;
        idx = v3;
    }
    else
    {
        page = 0;
        idx = 0;
    }
    result.internals.intVal = idx;
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetClientName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-44h] BYREF
    Operand result; // [esp+8h] [ebp-3Ch] BYREF
    cg_s *cgameGlob; // [esp+10h] [ebp-34h]
    char clientName[40]; // [esp+14h] [ebp-30h] BYREF
    int clientNum; // [esp+40h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    clientNum = GetSourceInt(&source).intVal;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    CL_GetClientName(localClientNum, clientNum, clientName, 38, 1);
    result.internals.intVal = (int)CopyTempString(clientName);
    AddOperandToStack(dataStack, &result);
}

void __cdecl ToOrdinal(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    int number; // [esp+8h] [ebp-Ch]
    Operand result; // [esp+Ch] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    number = GetSourceInt(&source).intVal;
    result.internals.intVal = (int)UI_TranslateIntegerToOrdinal(number);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetScoreForClient(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-18h] BYREF
    Operand result; // [esp+8h] [ebp-10h] BYREF
    cg_s *cgameGlob; // [esp+10h] [ebp-8h]
    int clientNum; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    clientNum = GetSourceInt(&source).intVal;
    if ( (unsigned int)clientNum < 0x20 && cgameGlob->bgs.clientinfo[clientNum].infoValid )
        result.internals.intVal = cgameGlob->bgs.clientinfo[clientNum].score.score;
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsDemoPlaying(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = Demo_IsPlaying();
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsDemoClipRecording(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = Demo_IsClipRecording() != 0;
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsDemoClipPlaying(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int type; // [esp+0h] [ebp-Ch] BYREF
    Operand result; // [esp+4h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( Demo_IsPlaying() )
    {
        result.internals.intVal = Demo_IsClipPlaying();
    }
    else if ( Live_FileShare_Theater_GetType(&type) && type == 2 )
    {
        result.internals.intVal = 1;
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsDemoMovieRendering(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = Demo_IsRenderingMovie();
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsDemoThirdPersonCamera(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( Demo_IsPlaying() )
        result.internals.intVal = Demo_IsThirdPersonCamera();
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsDemoMovieCamera(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( Demo_IsPlaying() )
        result.internals.intVal = Demo_IsMovieCamera();
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsDisplayingPartyScoreboard(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( Demo_IsPlaying() )
        result.internals.intVal = 0;
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetDemoTitleName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( !Live_FileShare_Theater_GetName((char **)&result.internals) )
    {
        if ( Demo_IsPlaying() )
            result.internals.intVal = (int)Demo_GetTitleName();
        else
            result.internals.intVal = (int)"";
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetDemoTitleDescription(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( !Live_FileShare_Theater_GetDescription((char **)&result.internals) )
    {
        if ( Demo_IsPlaying() )
            result.internals.intVal = (int)Demo_GetTitleDescription();
        else
            result.internals.intVal = (int)"";
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetDemoAuthor(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( Demo_IsPlaying() )
        result.internals.intVal = (int)Demo_GetAuthor();
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetDemoTimeInfo(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( Demo_IsPlaying() )
        result.internals.intVal = (int)Demo_GetTimeInfo();
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetDemoDuration(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( Demo_IsPlaying() )
        result.internals.intVal = (int)Demo_GetDuration();
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetDemoSegmentCount(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = Demo_GetSegmentCount();
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetDemoSegmentInformation(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-68h] BYREF
    OperandList list; // [esp+8h] [ebp-60h] BYREF
    int segmentIndex; // [esp+60h] [ebp-8h]
    const char *infoKey; // [esp+64h] [ebp-4h]

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    GetOperandList(dataStack, &list);
    if ( list.operandCount == 2 )
    {
        segmentIndex = GetOperandValueInt(list.operands).intVal;
        infoKey = GetSourceString(list.operands[1]);
        result.internals.intVal = (int)Demo_GetSegmentInformation(segmentIndex, infoKey);
    }
    else
    {
        result.internals.intVal = (int)"";
        Expression_Error("Expected 2 parameters to getDemoSegmentInformation()\n");
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsClipModified(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = Demo_IsClipModified();
    AddOperandToStack(dataStack, &result);
}

void __cdecl CanStartDemoPlayback(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = (int)"";
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetDemoSaveScreenName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    const char *type; // [esp+10h] [ebp-4h]

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    GetOperand(dataStack, &source);
    type = GetSourceString(source);
    result.internals.intVal = (int)Demo_GetSaveScreenName(type);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetDemoSaveScreenDescription(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    const char *type; // [esp+10h] [ebp-4h]

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    GetOperand(dataStack, &source);
    type = GetSourceString(source);
    result.internals.intVal = (int)Demo_GetSaveScreenDescription(type);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetTheaterFilmNotSelectedMessage(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetTheaterFilmNotSelectedMessage() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsCurrentSortedItemEquipped(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *v3; // eax
    int currSlot; // [esp+0h] [ebp-1Ch]
    Operand result; // [esp+4h] [ebp-18h] BYREF
    int controllerIndex; // [esp+Ch] [ebp-10h]
    int slotCount; // [esp+10h] [ebp-Ch]
    const char *groupName; // [esp+14h] [ebp-8h]
    int index; // [esp+18h] [ebp-4h]

    result.dataType = VAL_INT;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    index = sharedUiInfo.itemIndex;
    if ( sharedUiInfo.itemIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    5016,
                    0,
                    "%s",
                    "index != ENTRY_NOT_FOUND") )
    {
        __debugbreak();
    }
    result.internals.intVal = 0;
    groupName = Dvar_GetString("selected_group");
    slotCount = Dvar_GetInt("ui_slot_count");
    for ( currSlot = 0; currSlot < slotCount; ++currSlot )
    {
        v3 = va("%s%d", groupName, currSlot + 1);
        if ( index == BG_UnlockablesGetEquippedItemInSlot(controllerIndex, 0, v3) )
            result.internals.intVal = 1;
    }
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsCurrentItemEquipped() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetNumSortedItemsEquipped(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *v3; // eax
    unsigned int EquippedItemInSlot; // eax
    int currSlot; // [esp+0h] [ebp-18h]
    Operand result; // [esp+4h] [ebp-14h] BYREF
    int controllerIndex; // [esp+Ch] [ebp-Ch]
    int slotCount; // [esp+10h] [ebp-8h]
    const char *groupName; // [esp+14h] [ebp-4h]

    result.dataType = VAL_INT;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = 0;
    groupName = Dvar_GetString("selected_group");
    slotCount = Dvar_GetInt("ui_slot_count");
    for ( currSlot = 0; currSlot < slotCount; ++currSlot )
    {
        v3 = va("%s%d", groupName, currSlot + 1);
        EquippedItemInSlot = BG_UnlockablesGetEquippedItemInSlot(controllerIndex, 0, v3);
        if ( BG_UnlockablesIsItemValidNotNull(EquippedItemInSlot) )
            ++result.internals.intVal;
    }
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetNumSortedItemsEquipped() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsCurrentItemEquippedInAnyCustomClass(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-14h] BYREF
    int controllerIndex; // [esp+8h] [ebp-Ch]
    bool isEquipped; // [esp+Fh] [ebp-5h]
    int index; // [esp+10h] [ebp-4h]

    result.dataType = VAL_INT;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    index = sharedUiInfo.itemIndex;
    if ( sharedUiInfo.itemIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    5065,
                    0,
                    "%s",
                    "index != ENTRY_NOT_FOUND") )
    {
        __debugbreak();
    }
    isEquipped = BG_UnlockablesIsItemEquippedInAnyCustomClass(controllerIndex, index);
    result.internals.intVal = isEquipped;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsCurrentItemEquippedInAnyCustomClass() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsItemEquipped(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-70h] BYREF
    const char *customClassName; // [esp+8h] [ebp-68h]
    int controllerIndex; // [esp+Ch] [ebp-64h]
    OperandList list; // [esp+10h] [ebp-60h] BYREF
    int itemIndex; // [esp+6Ch] [ebp-4h]

    GetOperandList(dataStack, &list);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( list.operandCount == 2 )
    {
        controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        itemIndex = list.operands[0].internals.intVal;
        customClassName = list.operands[1].internals.string;
        result.internals.intVal = BG_UnlockablesIsItemEquipped(
                                                                controllerIndex,
                                                                list.operands[1].internals.string,
                                                                list.operands[0].internals.intVal);
        if ( uiscript_debug )
        {
            if ( uiscript_debug->current.integer )
                Expression_TraceInternal("IsItemEquipped() = %i\n", result.internals.intVal);
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Expression_Error("UI Expression Error: Expected 2 parameters to IsItemEquipped()\n");
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl IsCurrentItemPurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int index; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    index = sharedUiInfo.itemIndex;
    if ( sharedUiInfo.itemIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    5122,
                    0,
                    "%s",
                    "index != ENTRY_NOT_FOUND") )
    {
        __debugbreak();
    }
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = BG_UnlockablesIsItemPurchased(ControllerIndex, index);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsCurrentItemPurchased() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsProItemVersionUnlocked(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    int v4; // [esp-4h] [ebp-18h]
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    v4 = index;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = BG_UnlockablesIsProItemUnlocked(ControllerIndex, v4);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsProItemVersionUnlocked() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsProItemVersion(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    result.internals.intVal = BG_UnlockablesIsProItem(index);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsProItemVersion() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetProItemVersionCost(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-18h] BYREF
    Operand result; // [esp+8h] [ebp-10h] BYREF
    int proItemIndex; // [esp+10h] [ebp-8h]
    int index; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    proItemIndex = BG_UnlockablesGetProItem(index);
    if ( proItemIndex == -1 )
        result.internals.intVal = 0;
    else
        result.internals.intVal = BG_UnlockablesGetItemCost(proItemIndex);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetProItemVersionCost() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsItemPurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    int v4; // [esp-4h] [ebp-18h]
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    v4 = index;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = BG_UnlockablesIsItemPurchased(ControllerIndex, v4);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsItemPurchased() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsItemAttachmentPointPurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    int v4; // [esp-8h] [ebp-74h]
    Operand result; // [esp+0h] [ebp-6Ch] BYREF
    const char *attachmentPoint; // [esp+8h] [ebp-64h]
    OperandList list; // [esp+Ch] [ebp-60h] BYREF
    int itemIndex; // [esp+68h] [ebp-4h]

    GetOperandList(dataStack, &list);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( list.operandCount == 2 )
    {
        itemIndex = GetOperandValueInt(list.operands).intVal;
        attachmentPoint = GetSourceString(list.operands[1]);
        if ( itemIndex == -1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                        5230,
                        0,
                        "%s",
                        "itemIndex != ENTRY_NOT_FOUND") )
        {
            __debugbreak();
        }
        v4 = itemIndex;
        ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        result.internals.intVal = BG_UnlockablesIsItemAttachmentPointPurchased(ControllerIndex, v4, attachmentPoint);
        if ( uiscript_debug )
        {
            if ( uiscript_debug->current.integer )
                Expression_TraceInternal("IsCurrentItemAttachmentPointPurchased() = %i\n", result.internals.intVal);
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Expression_Error("Expected 2 parameters to IsItemAttachmentPointPurchased()\n");
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl IsCurrentItemAttachmentPurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    int v4; // [esp-4h] [ebp-10h]
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int index; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    index = sharedUiInfo.itemIndex;
    if ( sharedUiInfo.itemIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    5248,
                    0,
                    "%s",
                    "index != ENTRY_NOT_FOUND") )
    {
        __debugbreak();
    }
    v4 = sharedUiInfo.attachmentNum;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = BG_UnlockablesIsItemAttachmentPurchased(ControllerIndex, index, v4);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsCurrentItemAttachmentPurchased() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsCurrentItemOptionPurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    eWeaponOptionGroup OptionFromIndexAndGroupDvar; // eax
    int ControllerIndex; // eax
    int WeaponOptionNumFromIndexAndGroup; // [esp-4h] [ebp-10h]
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int index; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    index = sharedUiInfo.itemIndex;
    if ( sharedUiInfo.itemIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    5265,
                    0,
                    "%s",
                    "index != ENTRY_NOT_FOUND") )
    {
        __debugbreak();
    }
    OptionFromIndexAndGroupDvar = (eWeaponOptionGroup)GetOptionFromIndexAndGroupDvar();
    WeaponOptionNumFromIndexAndGroup = BG_GetWeaponOptionNumFromIndexAndGroup(
                                                                             sharedUiInfo.sortedItemPivot,
                                                                             OptionFromIndexAndGroupDvar);
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = BG_UnlockablesIsItemOptionPurchased(
                                                            ControllerIndex,
                                                            index,
                                                            WeaponOptionNumFromIndexAndGroup);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsCurrentItemOptionPurchased() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetItemOptionByName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    const char *optionName; // [esp+10h] [ebp-4h]

    result.dataType = VAL_INT;
    GetOperand(dataStack, &source);
    optionName = GetSourceString(source);
    result.internals.intVal = BG_UnlockablesGetWeaponOptionFromName(optionName);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetItemOptionByName() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsItemOptionPurchasedByName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    int v4; // [esp-8h] [ebp-20h]
    int WeaponOptionFromName; // [esp-4h] [ebp-1Ch]
    Operand source; // [esp+0h] [ebp-18h] BYREF
    Operand result; // [esp+8h] [ebp-10h] BYREF
    int index; // [esp+10h] [ebp-8h]
    const char *optionName; // [esp+14h] [ebp-4h]

    result.dataType = VAL_INT;
    GetOperand(dataStack, &source);
    index = sharedUiInfo.itemIndex;
    if ( sharedUiInfo.itemIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    5302,
                    0,
                    "%s",
                    "index != ENTRY_NOT_FOUND") )
    {
        __debugbreak();
    }
    optionName = GetSourceString(source);
    WeaponOptionFromName = BG_UnlockablesGetWeaponOptionFromName(optionName);
    v4 = index;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = BG_UnlockablesIsItemOptionPurchased(ControllerIndex, v4, WeaponOptionFromName);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsItemOptionPurchasedByName() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl ItemHasDualWield(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    result.internals.intVal = BG_UnlockablesItemHasDualWield(index);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("ItemHasDualWield() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl ItemIsDualWield(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    index = GetSourceInt(&source).intVal;
    result.internals.intVal = BG_UnlockablesItemIsDualWield(index);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("ItemIsDualWield() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCurrentItemCost(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int index; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    index = sharedUiInfo.itemIndex;
    if ( sharedUiInfo.itemIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    5356,
                    0,
                    "%s",
                    "index != ENTRY_NOT_FOUND") )
    {
        __debugbreak();
    }
    result.internals.intVal = BG_UnlockablesGetItemCost(index);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetCurrentItemCost() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCurrentItemSellPrice(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int index; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    index = sharedUiInfo.itemIndex;
    if ( sharedUiInfo.itemIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    5373,
                    0,
                    "%s",
                    "index != ENTRY_NOT_FOUND") )
    {
        __debugbreak();
    }
    result.internals.intVal = BG_UnlockablesGetItemSellPrice(index);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetCurrentItemSellPrice() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsCurrentItemClassified(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int index; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    index = sharedUiInfo.itemIndex;
    result.internals.intVal = BG_UnlockablesIsItemClassified(sharedUiInfo.itemIndex);
    if ( Dvar_GetInt((dvar_s*)ui_useCustomClassInfo) )
        result.internals.intVal = 0;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsCurrentItemClassified() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCurrentItemClassifiedHintText(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *ItemGroup; // eax
    const char *v4; // eax
    const char *v5; // eax
    char *v6; // eax
    int purchasesNeeded; // [esp+0h] [ebp-43Ch]
    Operand result; // [esp+4h] [ebp-438h] BYREF
    char outputString[1024]; // [esp+Ch] [ebp-430h] BYREF
    const StringTable *attributesTable; // [esp+410h] [ebp-2Ch] BYREF
    ConversionArguments convArgs; // [esp+414h] [ebp-28h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    StringTable_GetAsset("mp/attributesTable.csv", (XAssetHeader *)&attributesTable);
    purchasesNeeded = BG_UnlockablesGetNumPurchasesBeforeDeclassified(sharedUiInfo.itemIndex);
    memset(convArgs.args, 0, sizeof(convArgs.args));
    convArgs.argCount = 2;
    convArgs.args[0] = va("%i", purchasesNeeded);
    ItemGroup = BG_UnlockablesGetItemGroup(sharedUiInfo.itemIndex);
    v4 = StringTable_Lookup(attributesTable, 0, ItemGroup, 2, "name", 1);
    v5 = va("%s%s", v4, "_LOWER");
    convArgs.args[1] = UI_SafeTranslateString(v5);
    if ( purchasesNeeded == 1 )
        v6 = UI_SafeTranslateString("MPUI_CLASSIFIED_TOOLTIP_SING");
    else
        v6 = UI_SafeTranslateString("MPUI_CLASSIFIED_TOOLTIP");
    UI_ReplaceConversions(v6, &convArgs, outputString, 1024);
    result.internals.intVal = (int)CopyTempString(outputString);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCurrentItemAttachmentCost(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int index; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    index = sharedUiInfo.itemIndex;
    if ( sharedUiInfo.itemIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    5441,
                    0,
                    "%s",
                    "index != ENTRY_NOT_FOUND") )
    {
        __debugbreak();
    }
    result.internals.intVal = BG_UnlockablesGetItemAttachmentCost(index, sharedUiInfo.attachmentNum);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetCurrentItemAttachmentCost() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCurrentItemName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int index; // [esp+8h] [ebp-4h]

    result.dataType = VAL_STRING;
    index = sharedUiInfo.itemIndex;
    if ( sharedUiInfo.itemIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    5458,
                    0,
                    "%s",
                    "index != ENTRY_NOT_FOUND") )
    {
        __debugbreak();
    }
    result.internals.intVal = (int)BG_UnlockablesGetItemName(index);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetCurrentItemName() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCurrentItemIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int index; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    index = sharedUiInfo.itemIndex;
    if ( sharedUiInfo.itemIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    5475,
                    0,
                    "%s",
                    "index != ENTRY_NOT_FOUND") )
    {
        __debugbreak();
    }
    result.internals.intVal = index;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetCurrentItemIndex() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCurrentItemAttachmentName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int index; // [esp+8h] [ebp-4h]

    result.dataType = VAL_STRING;
    index = sharedUiInfo.itemIndex;
    if ( sharedUiInfo.itemIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    5493,
                    0,
                    "%s",
                    "index != ENTRY_NOT_FOUND") )
    {
        __debugbreak();
    }
    result.internals.intVal = (int)BG_UnlockablesGetItemAttachmentDisplayName(index, sharedUiInfo.attachmentNum);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetCurrentItemAttachmentName() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCurrentItemAttachmentDesc(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int index; // [esp+8h] [ebp-4h]

    result.dataType = VAL_STRING;
    index = sharedUiInfo.itemIndex;
    if ( sharedUiInfo.itemIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    5511,
                    0,
                    "%s",
                    "index != ENTRY_NOT_FOUND") )
    {
        __debugbreak();
    }
    result.internals.intVal = (int)BG_UnlockablesGetItemAttachmentDesc(index, sharedUiInfo.attachmentNum);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetCurrentItemAttachmentDesc() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCurrentItemAttachmentNum(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int attachmentNum; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    attachmentNum = sharedUiInfo.attachmentNum;
    if ( sharedUiInfo.attachmentNum == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    5529,
                    0,
                    "%s",
                    "attachmentNum != ENTRY_NOT_FOUND") )
    {
        __debugbreak();
    }
    result.internals.intVal = attachmentNum;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetCurrentItemAttachmentNum() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCurrentItemAttachmentPoint(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    eAttachment ItemAttachment; // eax
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    ItemAttachment = BG_UnlockablesGetItemAttachment(sharedUiInfo.itemIndex, sharedUiInfo.attachmentNum);
    result.internals.intVal = BG_GetAttachmentPointIndexFromAttachment(ItemAttachment);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetCurrentItemAttachmentPoint() = %d\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCurrentItemNumAttachments(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int index; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    index = sharedUiInfo.itemIndex;
    if ( sharedUiInfo.itemIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    5561,
                    0,
                    "%s",
                    "index != ENTRY_NOT_FOUND") )
    {
        __debugbreak();
    }
    result.internals.intVal = BG_UnlockablesGetNumItemAttachments(index);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetCurrentItemNumAttachments() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetItemNumAttachmentsEquipped(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    loadoutSlot_t ItemLoadoutSlot; // eax
    const char *AttachmentPointName; // eax
    const char *v5; // eax
    eAttachmentPoint currAttachPoint; // [esp+0h] [ebp-80h]
    const char *loadoutName; // [esp+4h] [ebp-7Ch]
    int controllerIndex; // [esp+8h] [ebp-78h]
    int numAttachmentsEquipped; // [esp+Ch] [ebp-74h]
    Operand result; // [esp+10h] [ebp-70h] BYREF
    const char *customClassName; // [esp+18h] [ebp-68h]
    int equippedAttachmentNum; // [esp+1Ch] [ebp-64h]
    OperandList list; // [esp+20h] [ebp-60h] BYREF
    int itemIndex; // [esp+7Ch] [ebp-4h]

    GetOperandList(dataStack, &list);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( list.operandCount == 2 )
    {
        numAttachmentsEquipped = 0;
        customClassName = GetSourceString(list.operands[0]);
        itemIndex = GetSourceInt(&list.operands[1]).intVal;
        if ( BG_UnlockablesIsItemValidNotNull(itemIndex) )
        {
            ItemLoadoutSlot = (loadoutSlot_t)BG_UnlockablesGetItemLoadoutSlot(itemIndex);
            loadoutName = BG_UnlockablesGetLoadoutSlotName(ItemLoadoutSlot);
            controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
            if ( loadoutName )
            {
                if ( *loadoutName )
                {
                    for ( currAttachPoint = ATTACHMENT_POINT_TOP; currAttachPoint < ATTACHMENT_POINT_COUNT; ++currAttachPoint )
                    {
                        AttachmentPointName = BG_GetAttachmentPointName(currAttachPoint);
                        v5 = va("%sattachment%s", loadoutName, AttachmentPointName);
                        equippedAttachmentNum = BG_UnlockablesGetEquippedItemInSlot(controllerIndex, customClassName, v5);
                        if ( BG_UnlockablesGetItemAttachment(itemIndex, equippedAttachmentNum) )
                            ++numAttachmentsEquipped;
                    }
                }
            }
        }
        result.internals.intVal = numAttachmentsEquipped;
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Expression_Error("Expected 2 parameters to GetCurrentItemNumAttachmentsEquipped()\n");
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetItemEquippedAttachment(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    loadoutSlot_t ItemLoadoutSlot; // eax
    const char *AttachmentPointName; // eax
    const char *v5; // eax
    eAttachmentPoint currAttachPoint; // [esp+0h] [ebp-80h]
    const char *loadoutName; // [esp+4h] [ebp-7Ch]
    int controllerIndex; // [esp+8h] [ebp-78h]
    int returnAttachmentIndex; // [esp+Ch] [ebp-74h]
    Operand result; // [esp+10h] [ebp-70h] BYREF
    const char *customClassName; // [esp+18h] [ebp-68h]
    int equippedAttachmentNum; // [esp+1Ch] [ebp-64h]
    OperandList list; // [esp+20h] [ebp-60h] BYREF
    eAttachment equippedAttachmentIndex; // [esp+78h] [ebp-8h]
    int itemIndex; // [esp+7Ch] [ebp-4h]

    GetOperandList(dataStack, &list);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( list.operandCount == 3 )
    {
        result.internals.intVal = -1;
        customClassName = GetSourceString(list.operands[0]);
        itemIndex = GetSourceInt(&list.operands[1]).intVal;
        if ( BG_UnlockablesIsItemValidNotNull(itemIndex) )
        {
            returnAttachmentIndex = GetSourceInt(&list.operands[2]).intVal;
            ItemLoadoutSlot = (loadoutSlot_t)BG_UnlockablesGetItemLoadoutSlot(itemIndex);
            loadoutName = BG_UnlockablesGetLoadoutSlotName(ItemLoadoutSlot);
            controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
            if ( loadoutName )
            {
                if ( *loadoutName )
                {
                    for ( currAttachPoint = ATTACHMENT_POINT_TOP; currAttachPoint < ATTACHMENT_POINT_COUNT; ++currAttachPoint )
                    {
                        AttachmentPointName = BG_GetAttachmentPointName(currAttachPoint);
                        v5 = va("%sattachment%s", loadoutName, AttachmentPointName);
                        equippedAttachmentNum = BG_UnlockablesGetEquippedItemInSlot(controllerIndex, customClassName, v5);
                        equippedAttachmentIndex = BG_UnlockablesGetItemAttachment(itemIndex, equippedAttachmentNum);
                        if ( equippedAttachmentIndex )
                        {
                            if ( !returnAttachmentIndex )
                            {
                                result.internals.intVal = equippedAttachmentIndex;
                                AddOperandToStack(dataStack, &result);
                                return;
                            }
                            --returnAttachmentIndex;
                        }
                    }
                }
            }
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Expression_Error("Expected 3 parameters to GetCurrentItemNumAttachmentsEquipped()\n");
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetNumItemAttachmentsWithAttachPoint(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-68h] BYREF
    OperandList list; // [esp+8h] [ebp-60h] BYREF
    int attachPoint; // [esp+60h] [ebp-8h]
    int itemIndex; // [esp+64h] [ebp-4h]

    result.internals.intVal = 0;
    result.dataType = VAL_INT;
    GetOperandList(dataStack, &list);
    if ( list.operandCount == 2 )
    {
        itemIndex = GetOperandValueInt(list.operands).intVal;
        attachPoint = GetOperandValueInt(&list.operands[1]).intVal;
        result.internals.intVal = BG_UnlockablesGetNumItemAttachmentsWithAttachPoint(itemIndex, attachPoint);
        if ( uiscript_debug )
        {
            if ( uiscript_debug->current.integer )
                Expression_TraceInternal("GetNumItemAttachmentsWithAttachPoint() = %i\n", result.internals.intVal);
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Expression_Error("Expected 2 parameters to GetNumItemAttachmentsWithAttachPoint()\n");
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl PlaylistPlayerCount(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *v3; // eax
    Operand source; // [esp+0h] [ebp-18h] BYREF
    Operand result; // [esp+8h] [ebp-10h] BYREF
    int controllerIndex; // [esp+10h] [ebp-8h]
    int playlistId; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    playlistId = Playlist_GetPlaylistIdForNum(controllerIndex, source.internals.intVal);
    v3 = va("playlist/%d", playlistId);
    result.internals.intVal = LiveGroups_GetCount(v3);
    AddOperandToStack(dataStack, &result);
}

void __cdecl CategoryPlayerCountForPlaylist(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int CategoryIdForNum; // eax
    char *v4; // eax
    unsigned int Count; // eax
    int index; // [esp+0h] [ebp-18h]
    Operand source; // [esp+4h] [ebp-14h] BYREF
    Operand result; // [esp+Ch] [ebp-Ch] BYREF
    const char *category; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    CategoryIdForNum = Playlist_GetCategoryIdForNum(source.internals.intVal);
    category = Playlist_GetCategoryInternalName(CategoryIdForNum);
    for ( index = 0; index < 64; ++index )
    {
        if ( Playlist_HasCategory(index, category) )
        {
            v4 = va("playlist/%d", index);
            Count = LiveGroups_GetCount(v4);
            result.internals.intVal += Count;
        }
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl TotalPlayersInPlaylists(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = LiveGroups_GetCount((char*)"playlist");
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetPlayersRegisteredOnline(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = LiveGroups_GetCount((char *)"online");
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetUserTagFromIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    Operand source; // [esp+0h] [ebp-10h] BYREF
    Operand result; // [esp+8h] [ebp-8h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    GetOperand(dataStack, &source);
    v3.intVal = GetSourceInt(&source).intVal;
    result.internals.intVal = (int)Live_FileShare_GetLocalizedUserTagFromIndex(v3.intVal);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetUserTagFromIndex() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl CanShowContentFromUser(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-10h] BYREF
    Operand result; // [esp+8h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 1;
    GetOperand(dataStack, &source);
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("CanShowContentFromUser() = %d\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetUserFileRating(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *SourceString; // eax
    int ControllerIndex; // eax
    unsigned __int64 v5; // [esp-Ch] [ebp-20h]
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    unsigned __int8 rating; // [esp+13h] [ebp-1h] BYREF

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    SourceString = GetSourceString(source);
    v5 = I_atoi64(SourceString);
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( Live_FileShare_IsRated(ControllerIndex, v5, &rating) )
        result.internals.intVal = rating;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetUserFileRating() = %d\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl CanRenderClip(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 1;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("CanRenderClip() = %d\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetFileShareFilterList(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)Live_FileShare_GetFilterList();
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetFileShareFilterList() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetLiveGroupCount(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    const char *groupName; // [esp+10h] [ebp-4h]

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    GetOperand(dataStack, &source);
    groupName = GetSourceString(source);
    result.internals.intVal = LiveGroups_GetCount((char *)groupName);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetBaseLbMenuName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *v3; // eax
    const char *v4; // eax
    int v5; // [esp+0h] [ebp-70h]
    const StringTable *gameTypesTable; // [esp+4h] [ebp-6Ch] BYREF
    Operand result; // [esp+8h] [ebp-68h] BYREF
    int lbType; // [esp+10h] [ebp-60h]
    const char *lbMenuPrefix; // [esp+14h] [ebp-5Ch]
    OperandList list; // [esp+18h] [ebp-58h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    GetOperandList(dataStack, &list);
    lbType = list.operands[0].internals.intVal;
    lbMenuPrefix = list.operands[1].internals.string;
    if ( list.operands[0].internals.intVal < 8 )
        v5 = lbType;
    else
        v5 = lbType - 8;
    lbType = v5;
    StringTable_GetAsset("mp/gametypesTable.csv", (XAssetHeader *)&gameTypesTable);
    if ( !gameTypesTable
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    5921,
                    0,
                    "%s",
                    "gameTypesTable") )
    {
        __debugbreak();
    }
    v3 = va("%d", lbType);
    v4 = StringTable_Lookup(gameTypesTable, 5, v3, 1);
    result.internals.intVal = (int)va("%s%s", lbMenuPrefix, v4);
    AddOperandToStack(dataStack, &result);
}

const int lbWriteMinRequirementValues[3] =
{ 50, 5, 25 };


void __cdecl GetLeaderboardMinReqText(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    char *v4; // eax
    const char *v5; // [esp-4h] [ebp-49Ch]
    char outputString[1024]; // [esp+0h] [ebp-498h] BYREF
    ConversionArguments convArgs; // [esp+404h] [ebp-94h] BYREF
    int lbTypeByResetPeriod; // [esp+42Ch] [ebp-6Ch]
    Operand result; // [esp+430h] [ebp-68h] BYREF
    OperandList list; // [esp+438h] [ebp-60h] BYREF
    int totalMatchesPlayed; // [esp+490h] [ebp-8h]
    const char *gameModePrefix; // [esp+494h] [ebp-4h]

    GetOperandList(dataStack, &list);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    gameModePrefix = GetSourceString(list.operands[0]);
    lbTypeByResetPeriod = GetSourceInt(&list.operands[1]).intVal;
    if ( lbTypeByResetPeriod )
    {
        totalMatchesPlayed = LiveStorage_GetMatchesPlayed(gameModePrefix, lbTypeByResetPeriod);
    }
    else
    {
        v5 = gameModePrefix;
        ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        totalMatchesPlayed = LiveStats_GetTotalMatchesPlayedByGameMode(ControllerIndex, v5);
    }
    if ( totalMatchesPlayed < lbWriteMinRequirementValues[lbTypeByResetPeriod] )
    {
        memset(convArgs.args, 0, sizeof(convArgs.args));
        convArgs.argCount = 2;
        convArgs.args[0] = va("%d", lbWriteMinRequirementValues[lbTypeByResetPeriod]);
        convArgs.args[1] = va("%d", totalMatchesPlayed);
        v4 = UI_SafeTranslateString("MPUI_LEADERBOARD_MIN_REQ_TEXT");
        UI_ReplaceConversions(v4, &convArgs, outputString, 1024);
        result.internals.intVal = (int)CopyTempString(outputString);
    }
    else
    {
        result.internals.intVal = (int)"";
    }
    if ( !result.internals.intVal )
        result.internals.intVal = (int)"";
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetPreviousGameType(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( !"" )
        result.internals.intVal = (int)"";
    AddOperandToStack(dataStack, &result);
}

void __cdecl HowManyReadiesNeeded(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = -1;
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsExtraCamActive(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = CG_ExtraCamIsActive(localClientNum);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCurrentWeapon(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    const cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( cgameGlob->predictedPlayerState.weaponstate < 20 || cgameGlob->predictedPlayerState.weaponstate > 25 )
        result.internals.intVal = (int)BG_WeaponName(cgameGlob->predictedPlayerState.weapon);
    else
        result.internals.intVal = (int)BG_WeaponName(cgameGlob->predictedPlayerState.offHandIndex);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsPlayerJoinable(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int v3; // edx
    char *SourceString; // eax
    int v5; // edx
    unsigned __int64 v6; // [esp+10h] [ebp-2Ch]
    Operand source; // [esp+18h] [ebp-24h] BYREF
    Operand result; // [esp+20h] [ebp-1Ch] BYREF
    int controllerIndex; // [esp+28h] [ebp-14h]
    unsigned __int64 selectedPlayerXuid; // [esp+2Ch] [ebp-10h]
    unsigned __int64 myXuid; // [esp+34h] [ebp-8h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( Live_IsSignedIn(controllerIndex) )
    {
        //LODWORD(v6) = Live_GetXuid(controllerIndex);
        //HIDWORD(v6) = v3;
        myXuid = Live_GetXuid(controllerIndex);
    }
    else
    {
        //v6 = 0;
        myXuid = (unsigned long long)0;
    }
    //myXuid = v6;
    SourceString = GetSourceString(source);
    selectedPlayerXuid = I_atoi64(SourceString);
#ifdef KISAK_LIVE
    if ( selectedPlayerXuid && selectedPlayerXuid != myXuid )
    {
        if ( Dvar_GetBool("ui_friendsListOpen") )
        {
            if ( Live_GetServerForFriend(selectedPlayerXuid) )
                result.internals.intVal = 1;
        }
        else if ( Dvar_GetBool("ui_playerListOpen") && (LiveMeetPlayer_GetPlayerSessionByID(selectedPlayerXuid) || v5) )
        {
            result.internals.intVal = 1;
        }
    }
#endif
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsPlayerInvitable(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int v3; // edx
    char *SourceString; // eax
    unsigned __int64 v5; // [esp+0h] [ebp-2Ch]
    Operand source; // [esp+8h] [ebp-24h] BYREF
    Operand result; // [esp+10h] [ebp-1Ch] BYREF
    int controllerIndex; // [esp+18h] [ebp-14h]
    unsigned __int64 selectedPlayerXuid; // [esp+1Ch] [ebp-10h]
    unsigned __int64 myXuid; // [esp+24h] [ebp-8h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( Live_IsSignedIn(controllerIndex) )
    {
        //LODWORD(v5) = Live_GetXuid(controllerIndex);
        //HIDWORD(v5) = v3;
        myXuid = Live_GetXuid(controllerIndex);
    }
    else
    {
        //v5 = 0;
        myXuid = (unsigned long long)0;
    }
    //myXuid = v5;
    SourceString = GetSourceString(source);
    selectedPlayerXuid = I_atoi64(SourceString);
    if ( selectedPlayerXuid && selectedPlayerXuid != myXuid )
        result.internals.intVal = 1;
    AddOperandToStack(dataStack, &result);
}

void __cdecl ClanMemberRankIsAtleast(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-10h] BYREF
    Operand result; // [esp+8h] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsExtraCamStatic(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    const cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->extraCamStatic
        || cgameGlob->extraCamActive && (cgameGlob->predictedPlayerState.weapFlags & 0x200000) == 0 )
    {
        result.internals.intVal = 1;
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetFlagCarrierForTeam(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *v3; // eax
    char clientName[40]; // [esp+8h] [ebp-54h] BYREF
    centity_s *cent; // [esp+34h] [ebp-28h]
    const clientInfo_t *clientInfo; // [esp+38h] [ebp-24h]
    int i; // [esp+3Ch] [ebp-20h]
    Operand source; // [esp+40h] [ebp-1Ch] BYREF
    Operand result; // [esp+48h] [ebp-14h] BYREF
    const char *teamName; // [esp+50h] [ebp-Ch]
    team_t team; // [esp+54h] [ebp-8h]
    const cg_s *cgameGlob; // [esp+58h] [ebp-4h]

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    GetOperand(dataStack, &source);
    teamName = GetSourceString(source);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    team = TEAM_ALLIES;
    v3 = CG_GetTeamName(TEAM_AXIS);
    if ( !I_stricmp(teamName, v3) )
        team = TEAM_AXIS;
    for ( i = 0; i < com_maxclients->current.integer; ++i )
    {
        clientInfo = &cgameGlob->bgs.clientinfo[i];
        if ( clientInfo->infoValid )
        {
            if ( clientInfo->team == team )
            {
                cent = CG_GetEntity(localClientNum, i);
                if ( ((*((unsigned int *)cent + 201) >> 1) & 1) != 0 && (cent->nextState.lerp.eFlags2 & 1) != 0 )
                {
                    CL_GetClientName(localClientNum, i, clientName, 38, 0);
                    result.internals.intVal = (int)CopyTempString(clientName);
                    break;
                }
            }
        }
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetFlagStatusForTeam(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *v3; // eax
    Operand source; // [esp+0h] [ebp-1Ch] BYREF
    Operand result; // [esp+8h] [ebp-14h] BYREF
    const char *teamName; // [esp+10h] [ebp-Ch]
    int isAway; // [esp+14h] [ebp-8h]
    const cg_s *cgameGlob; // [esp+18h] [ebp-4h]

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    GetOperand(dataStack, &source);
    teamName = GetSourceString(source);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    isAway = cgameGlob->alliesFlagAway;
    v3 = CG_GetTeamName(TEAM_AXIS);
    if ( !I_stricmp(teamName, v3) )
        isAway = cgameGlob->axisFlagAway;
    if ( isAway )
        result.internals.intVal = (int)UI_SafeTranslateString("OBJECTIVES_FLAG_AWAY_CAPS");
    else
        result.internals.intVal = (int)UI_SafeTranslateString("OBJECTIVES_FLAG_HOME_CAPS");
    AddOperandToStack(dataStack, &result);
}

void __cdecl HasTacticalMaskOverlay(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = CG_IsShowingZombieMap();
    AddOperandToStack(dataStack, &result);
}

void __cdecl InVehicle(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    const cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    result.internals.intVal = (cgameGlob->predictedPlayerState.eFlags & 0x4000) != 0;
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetNumActiveContracts(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = LiveContracts_GetNumActiveContracts(ControllerIndex);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCurrentContractIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    uiInfo_s *uiInfo; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    uiInfo = UI_GetInfo(localClientNum);
    if ( !uiInfo
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    6475,
                    0,
                    "%s",
                    "uiInfo") )
    {
        __debugbreak();
    }
    result.internals.intVal = uiInfo->contractIndex;
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetContractName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    index = GetSourceInt(&source).intVal;
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( index >= 0 )
        result.internals.intVal = (int)LiveContracts_GetContractName(index);
    else
        result.internals.intVal = (int)UI_SafeTranslateString("MPUI_CONTRACT_EMPTY_CAPS");
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetContractDesc(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    index = GetSourceInt(&source).intVal;
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    result.internals.intVal = (int)LiveContracts_GetContractDesc(index);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetContractProgress(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-1Ch] BYREF
    Operand result; // [esp+8h] [ebp-14h] BYREF
    int controllerIndex; // [esp+10h] [ebp-Ch]
    int index; // [esp+14h] [ebp-8h]
    int activeContractIndex; // [esp+18h] [ebp-4h]

    GetOperand(dataStack, &source);
    index = GetSourceInt(&source).intVal;
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    activeContractIndex = LiveContracts_GetActiveContractIndex(controllerIndex, index);
    if ( activeContractIndex >= 0 )
        result.internals.intVal = LiveContracts_GetActiveContractProgress(controllerIndex, activeContractIndex);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetContractRequiredCount(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    index = GetSourceInt(&source).intVal;
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    result.internals.intVal = LiveContracts_GetContractRequiredCount(index);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetContractCost(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    index = GetSourceInt(&source).intVal;
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    result.internals.intVal = LiveContracts_GetContractCost(index);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetTimeUntilNewContracts(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = LiveContracts_GetTimeUntilNextCycle();
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsContractActive(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-1Ch] BYREF
    Operand result; // [esp+8h] [ebp-14h] BYREF
    int controllerIndex; // [esp+10h] [ebp-Ch]
    int index; // [esp+14h] [ebp-8h]
    int activeContractIndex; // [esp+18h] [ebp-4h]

    GetOperand(dataStack, &source);
    index = GetSourceInt(&source).intVal;
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    activeContractIndex = LiveContracts_GetActiveContractIndex(controllerIndex, index);
    if ( activeContractIndex >= 0 )
        result.internals.intVal = 1;
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsContractInProgress(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-1Ch] BYREF
    Operand result; // [esp+8h] [ebp-14h] BYREF
    int controllerIndex; // [esp+10h] [ebp-Ch]
    int index; // [esp+14h] [ebp-8h]
    int activeContractIndex; // [esp+18h] [ebp-4h]

    GetOperand(dataStack, &source);
    index = GetSourceInt(&source).intVal;
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    activeContractIndex = LiveContracts_GetActiveContractIndex(controllerIndex, index);
    if ( LiveContracts_GetActiveContractStatus(controllerIndex, activeContractIndex) == 1 )
        result.internals.intVal = 1;
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsContractExpired(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-1Ch] BYREF
    Operand result; // [esp+8h] [ebp-14h] BYREF
    int controllerIndex; // [esp+10h] [ebp-Ch]
    int index; // [esp+14h] [ebp-8h]
    int activeContractIndex; // [esp+18h] [ebp-4h]

    GetOperand(dataStack, &source);
    index = GetSourceInt(&source).intVal;
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    activeContractIndex = LiveContracts_GetActiveContractIndex(controllerIndex, index);
    if ( LiveContracts_GetActiveContractStatus(controllerIndex, activeContractIndex) == 3 )
        result.internals.intVal = 1;
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsContractComplete(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-1Ch] BYREF
    Operand result; // [esp+8h] [ebp-14h] BYREF
    int controllerIndex; // [esp+10h] [ebp-Ch]
    int index; // [esp+14h] [ebp-8h]
    int activeContractIndex; // [esp+18h] [ebp-4h]

    GetOperand(dataStack, &source);
    index = GetSourceInt(&source).intVal;
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    activeContractIndex = LiveContracts_GetActiveContractIndex(controllerIndex, index);
    if ( LiveContracts_GetActiveContractStatus(controllerIndex, activeContractIndex) == 2 )
        result.internals.intVal = 1;
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsContractLocked(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-18h] BYREF
    Operand result; // [esp+8h] [ebp-10h] BYREF
    int controllerIndex; // [esp+10h] [ebp-8h]
    int contractIndex; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    contractIndex = GetSourceInt(&source).intVal;
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( LiveContracts_IsContractLocked(controllerIndex, contractIndex) )
        result.internals.intVal = 1;
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetTimesContractPurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-18h] BYREF
    Operand result; // [esp+8h] [ebp-10h] BYREF
    int controllerIndex; // [esp+10h] [ebp-8h]
    int contractIndex; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    contractIndex = GetSourceInt(&source).intVal;
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = LiveContracts_GetTimesPurchased(controllerIndex, contractIndex);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetTimesContractPurchasable(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int contractIndex; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    contractIndex = GetSourceInt(&source).intVal;
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    result.internals.intVal = LiveContracts_GetTimesPurchasable(contractIndex);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetContractCooldownTime(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int contractIndex; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    contractIndex = GetSourceInt(&source).intVal;
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    result.internals.intVal = LiveContracts_GetCooldownTime(contractIndex);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetRemainingContractCooldownTime(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-18h] BYREF
    Operand result; // [esp+8h] [ebp-10h] BYREF
    int controllerIndex; // [esp+10h] [ebp-8h]
    int contractIndex; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    contractIndex = GetSourceInt(&source).intVal;
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = LiveContracts_GetTimeUntilCooledDown(controllerIndex, contractIndex);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetContractUnlockLevel(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int contractIndex; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    contractIndex = GetSourceInt(&source).intVal;
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    result.internals.intVal = LiveContracts_GetUnlockLevel(contractIndex);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetContractRewardText(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    char *v8; // eax
    int v9; // [esp-8h] [ebp-7Ch]
    char *v10; // [esp-4h] [ebp-78h]
    int v11; // [esp-4h] [ebp-78h]
    int xpReward; // [esp+0h] [ebp-74h]
    Operand result; // [esp+4h] [ebp-70h] BYREF
    int contractIndex; // [esp+Ch] [ebp-68h]
    const char *xpStringRef; // [esp+10h] [ebp-64h]
    OperandList list; // [esp+14h] [ebp-60h] BYREF
    int cpReward; // [esp+6Ch] [ebp-8h]
    bool notGreyedOut; // [esp+73h] [ebp-1h]

    GetOperandList(dataStack, &list);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    xpStringRef = 0;
    contractIndex = GetSourceInt(list.operands).intVal;
    v3.intVal = GetSourceInt(&list.operands[1]).intVal;
    notGreyedOut = v3.intVal > 0;
    if ( v3.intVal <= 0 )
        xpStringRef = "MPUI_CONTRACT_REWARD_XP";
    else
        xpStringRef = "MPUI_CONTRACT_REWARD_XP_YELLOW";
    cpReward = LiveContracts_GetContractRewardCP(contractIndex);
    xpReward = LiveContracts_GetContractRewardXP(contractIndex);
    if ( cpReward <= 0 || xpReward <= 0 )
    {
        if ( cpReward <= 0 )
        {
            if ( xpReward > 0 )
            {
                v8 = UI_SafeTranslateString(xpStringRef);
                result.internals.intVal = (int)UI_ReplaceConversionInt(v8, xpReward);
            }
        }
        else
        {
            v11 = cpReward;
            v7 = UI_SafeTranslateString("MENU_POINTS");
            result.internals.intVal = (int)UI_ReplaceConversionInt(v7, v11);
        }
    }
    else
    {
        v4 = UI_SafeTranslateString(xpStringRef);
        v10 = UI_ReplaceConversionInt(v4, xpReward);
        v9 = cpReward;
        v5 = UI_SafeTranslateString("MENU_POINTS");
        v6 = UI_ReplaceConversionInt(v5, v9);
        result.internals.intVal = (int)va("%s        %s", v6, v10);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetContractExpirationType(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int contractIndex; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    contractIndex = GetSourceInt(&source).intVal;
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    result.internals.intVal = (int)LiveContracts_GetExpirationType(contractIndex);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsTaskInProgress(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    const char *taskName; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    taskName = GetSourceString(source);
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = TaskManger2_TaskGetInProgressForControllerByName(taskName, ControllerIndex) != 0;
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetContractExpirationData(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int contractIndex; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    contractIndex = GetSourceInt(&source).intVal;
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    result.internals.intVal = LiveContracts_GetExpirationData(contractIndex);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetContractCombatTimeLeft(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-1Ch] BYREF
    Operand result; // [esp+8h] [ebp-14h] BYREF
    int controllerIndex; // [esp+10h] [ebp-Ch]
    int index; // [esp+14h] [ebp-8h]
    int activeContractIndex; // [esp+18h] [ebp-4h]

    GetOperand(dataStack, &source);
    index = GetSourceInt(&source).intVal;
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    activeContractIndex = LiveContracts_GetActiveContractIndex(controllerIndex, index);
    result.internals.intVal = LiveContracts_GetCombatTimeLeft(controllerIndex, activeContractIndex);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetIndexForActiveContract(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-18h] BYREF
    Operand result; // [esp+8h] [ebp-10h] BYREF
    int controllerIndex; // [esp+10h] [ebp-8h]
    int activeContractIndex; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    activeContractIndex = GetSourceInt(&source).intVal;
    result.dataType = VAL_INT;
    result.internals.intVal = -1;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = LiveContracts_GetIndexForActiveContract(controllerIndex, activeContractIndex);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetIndexForNthActiveContract(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    signed int i; // [esp+0h] [ebp-20h]
    Operand source; // [esp+4h] [ebp-1Ch] BYREF
    int indexForActiveContract; // [esp+Ch] [ebp-14h]
    Operand result; // [esp+10h] [ebp-10h] BYREF
    int controllerIndex; // [esp+18h] [ebp-8h]
    int activeContractIndex; // [esp+1Ch] [ebp-4h]

    GetOperand(dataStack, &source);
    activeContractIndex = GetSourceInt(&source).intVal;
    result.dataType = VAL_INT;
    result.internals.intVal = -1;
    indexForActiveContract = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    for ( i = 0; i < 3; ++i )
    {
        indexForActiveContract = LiveContracts_GetIndexForActiveContract(controllerIndex, i);
        if ( indexForActiveContract != -1 && !--activeContractIndex )
        {
            result.internals.intVal = indexForActiveContract;
            AddOperandToStack(dataStack, &result);
            return;
        }
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetClanDateFounded(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-10h] BYREF
    Operand result; // [esp+8h] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetClanName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *Name; // eax
    int v4; // [esp+0h] [ebp-1Ch]
    Operand result; // [esp+4h] [ebp-18h] BYREF
    int clanTagColor; // [esp+Ch] [ebp-10h]
    int controllerIndex; // [esp+10h] [ebp-Ch]
    int currEquippedClanTagFeatureIndex; // [esp+14h] [ebp-8h]
    const char *clanName; // [esp+18h] [ebp-4h]

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    clanName = Clan_GetName(controllerIndex);
    currEquippedClanTagFeatureIndex = BG_UnlockablesGetEquippedClanTagFeatureIndex(controllerIndex);
    if ( currEquippedClanTagFeatureIndex )
        v4 = currEquippedClanTagFeatureIndex;
    else
        v4 = 7;
    clanTagColor = v4;
    if ( clanName && *clanName )
    {
        Name = Clan_GetName(controllerIndex);
        result.internals.intVal = (int)va("[^%i%s^7]", clanTagColor, Name);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsGroup(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetPlayerCardTitle(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *v3; // eax
    Operand source; // [esp+0h] [ebp-444h] BYREF
    const char *playerName; // [esp+8h] [ebp-43Ch]
    Operand result; // [esp+Ch] [ebp-438h] BYREF
    char outputString[1028]; // [esp+14h] [ebp-430h] BYREF
    ConversionArguments convArgs; // [esp+41Ch] [ebp-28h] BYREF

    result.dataType = VAL_STRING;
    GetOperand(dataStack, &source);
    playerName = GetSourceString(source);
    memset(&convArgs.args[1], 0, 32);
    convArgs.argCount = 1;
    convArgs.args[0] = playerName;
    v3 = UI_SafeTranslateString("MENU_PLAYERCARD_TITLE_CAPS");
    UI_ReplaceConversions(v3, &convArgs, outputString, 1024);
    result.internals.intVal = (int)CopyTempString(outputString);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetClanTagAndName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *Name; // eax
    char *v4; // eax
    const char *v5; // eax
    const char *playerName; // [esp+0h] [ebp-14h]
    Operand result; // [esp+4h] [ebp-10h] BYREF
    int controllerIndex; // [esp+Ch] [ebp-8h]
    int feature; // [esp+10h] [ebp-4h]

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    playerName = CL_ControllerIndex_GetUsername();
    feature = LiveStats_GetClanTagFeature(controllerIndex);
    Name = Clan_GetName(controllerIndex);
    if ( I_strcmp(Name, "") )
    {
        v4 = Clan_GetName(controllerIndex);
        v5 = va("[%s] %s", v4, playerName);
        result.internals.intVal = (int)CopyTempString(v5);
    }
    else
    {
        result.internals.intVal = (int)CopyTempString(playerName);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetXUID(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
#ifdef KISAK_LIVE
    __int64 v3; // rax
    const char *v4; // eax
    Operand result; // [esp+Ch] [ebp-Ch] BYREF
    int controllerIndex; // [esp+14h] [ebp-4h]

    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( Live_IsSignedIn(controllerIndex) )
    {
        LODWORD(v3) = Live_GetXuid(controllerIndex);
        v4 = va("%lld", v3);
        result.internals.intVal = (int)CopyTempString(v4);
    }
    else
    {
        result.internals.intVal = (int)CopyTempString("0");
    }
    AddOperandToStack(dataStack, &result);
#else
    __int64 v3; // rax
    const char *v4; // eax
    Operand result; // [esp+Ch] [ebp-Ch] BYREF
    int controllerIndex; // [esp+14h] [ebp-4h]

    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    //if (Live_IsSignedIn(controllerIndex))
    //{
    //    LODWORD(v3) = Live_GetXuid(controllerIndex);
    //    v4 = va("%lld", v3);
    //    result.internals.intVal = (int)CopyTempString(v4);
    //}
    //else
    {
        result.internals.intVal = (int)CopyTempString("0");
    }
    AddOperandToStack(dataStack, &result);
#endif
}

void __cdecl GetSellText(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int controllerIndex; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetItemPrice(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int controllerIndex; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetSelfGamertag(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
#ifdef KISAK_LIVE
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int controllerIndex; // [esp+8h] [ebp-4h]

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( Live_IsSignedIn(controllerIndex) )
        result.internals.intVal = (int)Live_ControllerIndex_GetClientName(controllerIndex);
    else
        result.internals.intVal = (int)"";
    AddOperandToStack(dataStack, &result);
#else
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int controllerIndex; // [esp+8h] [ebp-4h]

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    //if (Live_IsSignedIn(controllerIndex))
    //    result.internals.intVal = (int)Live_ControllerIndex_GetClientName(controllerIndex);
    //else
        result.internals.intVal = (int)"";
    AddOperandToStack(dataStack, &result);
#endif
}

void __cdecl GetRankByXUID(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *SourceString; // eax
    int ControllerIndex; // eax
    unsigned __int64 v5; // [esp-8h] [ebp-20h]
    Operand source; // [esp+0h] [ebp-18h] BYREF
    Operand result; // [esp+8h] [ebp-10h] BYREF
    unsigned __int64 xuid; // [esp+10h] [ebp-8h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    SourceString = GetSourceString(source);
    xuid = I_atoi64(SourceString);
    v5 = xuid;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = GetRankByXUID_Internal(ControllerIndex, v5);
    if ( result.internals.intVal < 0 )
        result.internals.intVal = 0;
    AddOperandToStack(dataStack, &result);
}

int __cdecl GetRankByXUID_Internal(int controllerIndex, unsigned __int64 xuid)
{
    int prestige; // [esp+0h] [ebp-8h] BYREF
    int rank; // [esp+4h] [ebp-4h] BYREF

    if ( xuid && (rank = 0, prestige = 0, PCache_GetRank(controllerIndex, xuid, &rank, &prestige)) )
        return rank;
    else
        return -1;
}

void __cdecl GetPrestigeByXUID(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *SourceString; // eax
    int prestige; // [esp+0h] [ebp-24h] BYREF
    int controllerIndex; // [esp+4h] [ebp-20h]
    int rank; // [esp+8h] [ebp-1Ch] BYREF
    Operand source; // [esp+Ch] [ebp-18h] BYREF
    Operand result; // [esp+14h] [ebp-10h] BYREF
    unsigned __int64 xuid; // [esp+1Ch] [ebp-8h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    SourceString = GetSourceString(source);
    xuid = I_atoi64(SourceString);
    if ( xuid )
    {
        rank = 0;
        prestige = 0;
        controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        if ( PCache_GetRank(controllerIndex, xuid, &rank, &prestige) )
            result.internals.intVal = prestige;
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCodpointsByXUID(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *SourceString; // eax
    int controllerIndex; // [esp+0h] [ebp-20h]
    int codpoints; // [esp+4h] [ebp-1Ch] BYREF
    Operand source; // [esp+8h] [ebp-18h] BYREF
    Operand result; // [esp+10h] [ebp-10h] BYREF
    unsigned __int64 xuid; // [esp+18h] [ebp-8h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    SourceString = GetSourceString(source);
    xuid = I_atoi64(SourceString);
    if ( xuid )
    {
        codpoints = 0;
        controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        if ( PCache_GetCodpoints(controllerIndex, xuid, &codpoints) )
            result.internals.intVal = codpoints;
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetDisplayLevelByXUID(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *SourceString; // eax
    int ControllerIndex; // eax
    const char *RankData; // eax
    unsigned __int64 v6; // [esp-8h] [ebp-24h]
    Operand source; // [esp+0h] [ebp-1Ch] BYREF
    Operand result; // [esp+8h] [ebp-14h] BYREF
    int rank; // [esp+10h] [ebp-Ch]
    unsigned __int64 xuid; // [esp+14h] [ebp-8h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    SourceString = GetSourceString(source);
    xuid = I_atoi64(SourceString);
    v6 = xuid;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    rank = GetRankByXUID_Internal(ControllerIndex, v6);
    if ( rank >= 0 )
    {
        RankData = CL_GetRankData(rank, MP_RANKTABLE_DISPLAYLEVEL);
        result.internals.intVal = (int)CopyTempString(RankData);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetMutedStatus(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *SourceString; // eax
    Operand source; // [esp+4h] [ebp-2Ch] BYREF
    const clientActive_t *LocalClientGlobals; // [esp+Ch] [ebp-24h]
    int playerIndex; // [esp+10h] [ebp-20h]
    Operand result; // [esp+14h] [ebp-1Ch] BYREF
    const clientState_s *pState; // [esp+1Ch] [ebp-14h]
    unsigned __int64 playerXuid; // [esp+20h] [ebp-10h]
    int i; // [esp+28h] [ebp-8h]
    int localControllerIndex; // [esp+2Ch] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    SourceString = GetSourceString(source);
    playerXuid = I_atoi64(SourceString);
    localControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    playerIndex = -1;
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if ( LocalClientGlobals->snap.valid )
    {
        for ( i = 0; i < LocalClientGlobals->snap.numClients; ++i )
        {
            pState = &LocalClientGlobals->parseClients[((_WORD)i + (unsigned __int16)LocalClientGlobals->snap.parseClientsNum)
                                                                                             & 0x7FF];
            if ( pState->xuid == playerXuid )
            {
                playerIndex = pState->clientIndex;
                break;
            }
        }
    }
    result.internals.intVal = playerIndex > -1 && CL_IsPlayerMuted(localClientNum, playerIndex);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsProfileSignedIn(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int controllerIndex; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( controllerIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    7486,
                    0,
                    "%s",
                    "controllerIndex != INVALID_CONTROLLER_PORT") )
    {
        __debugbreak();
    }
    result.internals.intVal = controllerIndex < 0 || GamerProfile_IsProfileLoggedIn(controllerIndex);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsSignedIn(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
#ifdef KISAK_LIVE
    int ControllerIndex; // eax
    Operand result; // [esp+0h] [ebp-8h] BYREF

    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = Live_IsSignedIn(ControllerIndex);
    result.dataType = VAL_INT;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsSignedIn() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
#else
    int ControllerIndex; // eax
    Operand result; // [esp+0h] [ebp-8h] BYREF

    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = 0;// Live_IsSignedIn(ControllerIndex);
    result.dataType = VAL_INT;
    if (uiscript_debug && uiscript_debug->current.integer)
        Expression_TraceInternal("IsSignedIn() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
#endif
}

void __cdecl IsSignedInToLive(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
#ifdef KISAK_LIVE
    Operand result; // [esp+0h] [ebp-8h] BYREF

    Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = Live_IsSignedInToLive();
    result.dataType = VAL_INT;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsSignedInToLive() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
#else
    Operand result; // [esp+0h] [ebp-8h] BYREF

    Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = 0;// Live_IsSignedInToLive();
    result.dataType = VAL_INT;
    if (uiscript_debug && uiscript_debug->current.integer)
        Expression_TraceInternal("IsSignedInToLive() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
#endif
}

void __cdecl AnySignedIn(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.internals.intVal = 0;
    result.dataType = VAL_INT;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsSignedIn() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl AnySignedInToLiveAndStatsFetched(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.internals.intVal = 0;
    result.dataType = VAL_INT;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsSignedInToLive() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl AreStatsFetched(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int localControllerIndex; // [esp+8h] [ebp-4h]

    localControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = Live_IsUserSignedInToDemonware(localControllerIndex)
                                                 && LiveStorage_DoWeHaveAllStats(localControllerIndex);
    result.dataType = VAL_INT;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("AreStatsFetched() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl AreContractsFetched(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = LiveStorage_DoWeHaveContracts();
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("AreContractsFetched() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsTimeSynced(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = LiveStorage_IsTimeSynced();
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsTimeSynced() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsContentRatingAllowed(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 1;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsContentRatingAllowed() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsDemonwareFetchingDone(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+20h] [ebp-18h] BYREF
    bool isTimeSynced; // [esp+2Bh] [ebp-Dh]
    int controllerIndex; // [esp+2Ch] [ebp-Ch]
    bool weHaveContracts; // [esp+31h] [ebp-7h]
    bool weHavePlaylist; // [esp+32h] [ebp-6h]
    bool isUserSignedInToLive; // [esp+33h] [ebp-5h]
    bool isUserSignedInToDemonware; // [esp+34h] [ebp-4h]
    bool weHaveStats; // [esp+35h] [ebp-3h]
    bool hasMultiplayerPrivileges; // [esp+36h] [ebp-2h]
    bool isUserGuest; // [esp+37h] [ebp-1h]

    result.dataType = VAL_INT;
    result.internals.intVal = 1;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    isUserGuest = CG_IsShowingZombieMap();
    isUserSignedInToLive = Live_IsUserSignedInToLive();
    isUserSignedInToDemonware = Live_IsUserSignedInToDemonware(controllerIndex);
    weHaveStats = LiveStorage_DoWeHaveAllStats(controllerIndex);
    weHavePlaylist = LiveStorage_DoWeHavePlaylists();
    hasMultiplayerPrivileges = Flame_GetLocalClientSourceRange();
    if ( !isUserSignedInToDemonware || !weHaveStats || !weHavePlaylist || !hasMultiplayerPrivileges )
        result.internals.intVal = 0;
    isTimeSynced = LiveStorage_IsTimeSynced();
    weHaveContracts = LiveStorage_DoWeHaveContracts();
    if ( !isTimeSynced || !weHaveContracts )
        result.internals.intVal = 0;
    if ( !result.internals.intVal )
    {
        Com_Printf(16, "Can play online (controller: %d): %s\n", controllerIndex, "false");
        if ( isUserGuest )
            Com_Printf(16, "\t%s - Live_UserIsGuest( controller: %d )\n", "true", controllerIndex);
        else
            Com_Printf(16, "\t%s - Live_UserIsGuest( controller: %d )\n", "false", controllerIndex);
        if ( isUserSignedInToLive )
            Com_Printf(16, "\t%s - Live_IsUserSignedInToLive( controller: %d )\n", "true", controllerIndex);
        else
            Com_Printf(16, "\t%s - Live_IsUserSignedInToLive( controller: %d )\n", "false", controllerIndex);
        if ( isUserSignedInToDemonware )
            Com_Printf(16, "\t%s - Live_IsUserSignedInToDemonware( controller: %d )\n", "true", controllerIndex);
        else
            Com_Printf(16, "\t%s - Live_IsUserSignedInToDemonware( controller: %d )\n", "false", controllerIndex);
        if ( weHavePlaylist )
            Com_Printf(16, "\t%s - LiveStorage_DoWeHavePlaylists()\n", "true");
        else
            Com_Printf(16, "\t%s - LiveStorage_DoWeHavePlaylists()\n", "false");
        if ( weHaveStats )
            Com_Printf(16, "\t%s - LiveStorage_DoWeHaveAllStats( controller: %d )\n", "true", controllerIndex);
        else
            Com_Printf(16, "\t%s - LiveStorage_DoWeHaveAllStats( controller: %d )\n", "false", controllerIndex);
        if ( isTimeSynced )
            Com_Printf(16, "\t%s - LiveStorage_IsTimeSynced()\n", "true");
        else
            Com_Printf(16, "\t%s - LiveStorage_IsTimeSynced()\n", "false");
        if ( weHaveContracts )
            Com_Printf(16, "\t%s - LiveStorage_DoWeHaveContracts()\n", "true");
        else
            Com_Printf(16, "\t%s - LiveStorage_DoWeHaveContracts()\n", "false");
    }
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("IsDemonwareFetchingDone() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetUIRect(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *SourceString; // eax
    Operand result; // [esp+0h] [ebp-6Ch] BYREF
    const char *field; // [esp+8h] [ebp-64h]
    OperandList list; // [esp+Ch] [ebp-60h] BYREF
    itemDef_s *actualItem; // [esp+68h] [ebp-4h]

    field = "";
    GetOperandList(dataStack, &list);
    result.dataType = VAL_FLOAT;
    result.internals.intVal = 0;
    if ( list.operandCount >= 1 )
    {
        if ( list.operandCount == 1 )
        {
            actualItem = item;
            if ( list.operands[0].dataType == VAL_STRING )
                field = list.operands[0].internals.string;
        }
        else
        {
            SourceString = GetSourceString(list.operands[0]);
            actualItem = Menu_GetMatchingItemByNumber(item->parent, 0, SourceString);
            if ( list.operands[1].dataType == VAL_STRING )
                field = list.operands[1].internals.string;
        }
        if ( actualItem )
        {
            if ( I_stricmp(field, "x") )
            {
                if ( I_stricmp(field, "y") )
                {
                    if ( I_stricmp(field, "w") )
                    {
                        if ( !I_stricmp(field, "h") )
                            result.internals.intVal = LODWORD(actualItem->window.rect.h);
                    }
                    else
                    {
                        result.internals.intVal = LODWORD(actualItem->window.rect.w);
                    }
                }
                else
                {
                    result.internals.intVal = LODWORD(actualItem->window.rect.y);
                }
            }
            else
            {
                result.internals.intVal = LODWORD(actualItem->window.rect.x);
            }
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Com_PrintError(13, "UI Expression Error: Expected at least 1 parameter to getUIRect()\n");
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetDStat(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    __int64 v3; // rax
    Operand result; // [esp+0h] [ebp-74h] BYREF
    int controllerIndex; // [esp+8h] [ebp-6Ch]
    OperandList list; // [esp+Ch] [ebp-68h] BYREF
    ddlState_t searchState; // [esp+64h] [ebp-10h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
#ifdef KISAK_LIVE
    searchState = *LiveStats_GetRootDDLState();
    GetOperandList(dataStack, &list);
    if ( GetSearchState(&searchState, &list, 0) )
    {
        controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        if ( !searchState.member
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                        7875,
                        0,
                        "%s",
                        "searchState.member") )
        {
            __debugbreak();
        }
        if ( searchState.member->type == 5 )
        {
            result.dataType = VAL_STRING;
            result.internals.intVal = (int)LiveStats_GetDStringStat(controllerIndex, &searchState);
        }
        else if ( searchState.member->type == 3 )
        {
            result.dataType = VAL_STRING;
            LODWORD(v3) = LiveStats_GetDInt64Stat(controllerIndex, &searchState);
            result.internals.intVal = (int)va("%llu", v3);
        }
        else
        {
            result.dataType = VAL_INT;
            result.internals.intVal = LiveStats_GetDIntStat(controllerIndex, &searchState);
        }
    }
#endif
    AddOperandToStack(dataStack, &result);
}

char __cdecl GetSearchState(ddlState_t *searchState, OperandList *operandList, int listStartIndex)
{
    expDataType dataType; // [esp+8h] [ebp-60h]
    int i; // [esp+Ch] [ebp-5Ch]
    OperandList list; // [esp+10h] [ebp-58h] BYREF

    memcpy(&list, operandList, sizeof(list));
    if ( list.operandCount - listStartIndex <= 8 )
    {
        for ( i = listStartIndex; i < list.operandCount; ++i )
        {
            dataType = list.operands[i].dataType;
            if ( dataType )
            {
                if ( dataType != VAL_STRING )
                {
                    DDL_PrintError("DDL: Expected strings or integers only.");
                    return 0;
                }
                if ( searchState->member
                    && searchState->arrayIndex == -1
                    && searchState->member->arraySize > 1
                    && searchState->member->enumIndex == -1 )
                {
                    DDL_PrintError("DDL: array index (integer) expected. Received a string instead.");
                    return 0;
                }
                if ( !DDL_MoveToName(searchState, searchState, list.operands[i].internals.string) )
                {
                    DDL_PrintError("DDL: Could not find member name.");
                    return 0;
                }
            }
            else
            {
                if ( !searchState->member
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                                7833,
                                0,
                                "%s",
                                "searchState->member") )
                {
                    __debugbreak();
                }
                if ( searchState->member->arraySize == 1 )
                {
                    DDL_PrintError("DDL: Member name (string) expected. Received an integer instead.");
                    return 0;
                }
                if ( !DDL_MoveToIndex(searchState, searchState, list.operands[i].internals.intVal, 1) )
                {
                    DDL_PrintError("DDL: Could not find member array index number.");
                    return 0;
                }
            }
        }
        return 1;
    }
    else
    {
        DDL_PrintError("DDL: path should be at most %d element deep", 8);
        return 0;
    }
}

void __cdecl GetDStatForPreviousMatch(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    unsigned int DIntStatFromBase; // esi
    Operand result; // [esp+4h] [ebp-7Ch] BYREF
    char *liveStatsBuffer; // [esp+Ch] [ebp-74h]
    int controllerIndex; // [esp+10h] [ebp-70h]
    char *oldStatsBuffer; // [esp+14h] [ebp-6Ch]
    OperandList list; // [esp+18h] [ebp-68h] BYREF
    ddlState_t searchState; // [esp+70h] [ebp-10h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    searchState = *LiveStats_GetRootDDLState();
    GetOperandList(dataStack, &list);
    if ( GetSearchState(&searchState, &list, 0) )
    {
        controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        liveStatsBuffer = (char *)LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        oldStatsBuffer = (char *)LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_STABLE, 1);
        if ( !searchState.member
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                        7920,
                        0,
                        "%s",
                        "searchState.member") )
        {
            __debugbreak();
        }
        DIntStatFromBase = LiveStats_GetDIntStatFromBase(controllerIndex, &searchState, (persistentStats *)liveStatsBuffer);
        result.internals.intVal = DIntStatFromBase
                                                        - LiveStats_GetDIntStatFromBase(
                                                                controllerIndex,
                                                                &searchState,
                                                                (persistentStats *)oldStatsBuffer);
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl IsStableStatsBufferInitialized(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int controllerIndex; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = LiveStats_IsStableStatsBufferInitialized(controllerIndex);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetStatForFriendOrSelf(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    __int64 v3; // rax
    persistentStats *buffer; // [esp+0h] [ebp-80h]
    Operand result; // [esp+4h] [ebp-7Ch] BYREF
    int controllerIndex; // [esp+Ch] [ebp-74h]
    OperandList list; // [esp+10h] [ebp-70h] BYREF
    ddlState_t searchState; // [esp+6Ch] [ebp-14h] BYREF
    int forFriend; // [esp+7Ch] [ebp-4h]

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    searchState = *LiveStats_GetRootDDLState();
    GetOperandList(dataStack, &list);
    forFriend = list.operands[0].internals.intVal;
    if ( GetSearchState(&searchState, &list, 1) )
    {
        controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        if ( forFriend )
            buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_OTHERPLAYER, 1);
        else
            buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_GLOBAL, 1);
        if ( !searchState.member
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                        7984,
                        0,
                        "%s",
                        "searchState.member") )
        {
            __debugbreak();
        }
        if ( searchState.member->type == 5 )
        {
            result.dataType = VAL_STRING;
            result.internals.intVal = (int)LiveStats_GetDStringStatFromForcedBase(
                                                                             controllerIndex,
                                                                             &searchState,
                                                                             (char *)buffer);
        }
        else if ( searchState.member->type == 3 )
        {
            result.dataType = VAL_STRING;
            LODWORD(v3) = LiveStats_GetDInt64StatFromForcedBase(controllerIndex, &searchState, (char *)buffer);
            result.internals.intVal = (int)va("%llu", v3);
        }
        else
        {
            result.dataType = VAL_INT;
            result.internals.intVal = LiveStats_GetDIntStatFromForcedBase(controllerIndex, &searchState, (char *)buffer);
        }
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetSortedStatsForFriendOrSelf(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int param; // [esp+4h] [ebp-80h]
    Operand result; // [esp+8h] [ebp-7Ch] BYREF
    int returnStatsforFriend; // [esp+10h] [ebp-74h]
    OperandList list; // [esp+14h] [ebp-70h] BYREF
    bool forFriend; // [esp+73h] [ebp-11h]
    int currIndex; // [esp+74h] [ebp-10h]
    int index; // [esp+78h] [ebp-Ch]
    int getAbsoluteIndex; // [esp+7Ch] [ebp-8h]
    int itemIndex; // [esp+80h] [ebp-4h]

    GetOperandList(dataStack, &list);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( list.operandCount >= 4 )
    {
        returnStatsforFriend = list.operands[0].internals.intVal;
        index = list.operands[1].internals.intVal;
        getAbsoluteIndex = list.operands[2].internals.intVal;
        param = list.operands[3].internals.intVal;
        forFriend = list.operands[0].internals.intVal != 0;
        if ( list.operands[2].internals.intVal && index >= 0 && index < sharedUiInfo.clanTagFeature )
        {
            if ( LiveCombatRecord_GetSortedItemData(index, forFriend, SORT_KEY) == 2147483600.0 )
            {
                while ( LiveCombatRecord_GetSortedItemData(index, forFriend, SORT_KEY) == 2147483600.0
                         && index < sharedUiInfo.clanTagFeature )
                    ++index;
            }
            itemIndex = (int)LiveCombatRecord_GetSortedItemData(index, forFriend, ITEM_INDEX);
            currIndex = index;
        }
        if ( !getAbsoluteIndex )
        {
            currIndex = sharedUiInfo.sortedItemPivot + index - 3;
            if ( sharedUiInfo.sortedItemPivot + index == 2 )
                currIndex = 0;
            if ( currIndex >= sharedUiInfo.clanTagFeature )
                currIndex = sharedUiInfo.clanTagFeature - 1;
            itemIndex = (int)LiveCombatRecord_GetSortedItemData(currIndex, forFriend, ITEM_INDEX);
        }
        switch ( param )
        {
            case 0:
                result.dataType = VAL_INT;
                result.internals.intVal = itemIndex;
                break;
            case 1:
                result.dataType = VAL_INT;
                result.internals.intVal = (int)LiveCombatRecord_GetSortedItemData(currIndex, forFriend, PARAM1);
                break;
            case 2:
                result.dataType = VAL_INT;
                result.internals.intVal = (int)LiveCombatRecord_GetSortedItemData(currIndex, forFriend, PARAM2);
                break;
            case 4:
                result.dataType = VAL_FLOAT;
                result.internals.floatVal = LiveCombatRecord_GetSortedItemData(currIndex, forFriend, SORT_KEY);
                break;
            default:
                break;
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Com_PrintError(13, "UI Expression Error: Expected 4 parameters to getSortedStatsForFriendOrSelf()\n");
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetToastPopupWidth(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // [esp+4h] [ebp-24h]
    int v4; // [esp+8h] [ebp-20h]
    int v5; // [esp+Ch] [ebp-1Ch]
    Operand result; // [esp+10h] [ebp-18h] BYREF
    Font_s *font; // [esp+18h] [ebp-10h]
    const ScreenPlacement *scrPlace; // [esp+1Ch] [ebp-Ch]
    uiInfo_s *uiInfo; // [esp+20h] [ebp-8h]
    float scale; // [esp+24h] [ebp-4h]

    scale = 0.315f;
    scrPlace = &scrPlaceView[localClientNum];
    font = UI_GetFontHandle(scrPlace, 0, 0.315);
    uiInfo = UI_GetInfo(localClientNum);
    result.dataType = VAL_INT;
    v4 = UI_TextWidth(uiInfo->toastPopupDesc, 0, font, 0.315);
    v5 = UI_TextWidth(uiInfo->toastPopupTitle, 0, font, 0.315);
    if ( v5 < v4 )
        v3.intVal = v4;
    else
        v3.intVal = v5;
    result.internals = v3;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetToastPopupWidth() = %i\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetToastPopupIcon(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    uiInfo_s *uiInfo; // [esp+8h] [ebp-4h]

    uiInfo = UI_GetInfo(localClientNum);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)uiInfo->toastPopupIconName;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetToastPopupIcon() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetToastPopupDescription(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    uiInfo_s *uiInfo; // [esp+8h] [ebp-4h]

    uiInfo = UI_GetInfo(localClientNum);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)uiInfo->toastPopupDesc;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("GetToastPopupDescription() = %s\n", result.internals.string);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetFloatAsFormattedString(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+Ch] [ebp-68h] BYREF
    OperandList list; // [esp+14h] [ebp-60h] BYREF
    float value; // [esp+70h] [ebp-4h]

    GetOperandList(dataStack, &list);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( list.operandCount >= 2 )
    {
        value = list.operands[0].internals.floatVal;
        result.internals.intVal = (int)va("%.*f", list.operands[1].internals.intVal, list.operands[0].internals.floatVal);
    }
    else
    {
        Com_PrintError(13, "UI Expression Error: Expected 2 parameters to GetFloatAsFormattedString()\n");
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl ChangeSortedStatsPivot(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-18h] BYREF
    int changeValue; // [esp+8h] [ebp-10h]
    Operand result; // [esp+Ch] [ebp-Ch] BYREF
    int newPivot; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 1;
    changeValue = GetSourceInt(&source).intVal;
    newPivot = changeValue + sharedUiInfo.sortedItemPivot;
    if ( changeValue + sharedUiInfo.sortedItemPivot > 2 || newPivot < sharedUiInfo.clanTagFeature - 2 )
        sharedUiInfo.sortedItemPivot = newPivot;
    AddOperandToStack(dataStack, &result);
}

void __cdecl CanScrollUpOrDown(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int wantToScrollUp; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    wantToScrollUp = GetSourceInt(&source).intVal;
    result.internals.intVal = (!wantToScrollUp || sharedUiInfo.sortedItemPivot > 2)
                                                 && (wantToScrollUp || sharedUiInfo.sortedItemPivot < sharedUiInfo.clanTagFeature - 3);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCurrentScrollBarPosition(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // [esp+0h] [ebp-20h]
    Operand source; // [esp+8h] [ebp-18h] BYREF
    Operand result; // [esp+10h] [ebp-10h] BYREF
    int currPos; // [esp+18h] [ebp-8h]
    float totalHeightOfScrollBar; // [esp+1Ch] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    totalHeightOfScrollBar = GetSourceFloat(&source);
    result.dataType = VAL_FLOAT;
    currPos = Dvar_GetInt("ui_combatCurrScrollBarPos");
    if ( (float)(totalHeightOfScrollBar
                         - (float)((float)currPos * (float)(totalHeightOfScrollBar / (float)(sharedUiInfo.clanTagFeature - 5)))) < 0.0 )
        v3.floatVal = totalHeightOfScrollBar;
    else
        v3.floatVal = (float)currPos * (float)(totalHeightOfScrollBar / (float)(sharedUiInfo.clanTagFeature - 5));
    result.internals = v3;
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCombatRecordInfoBarWidth(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    double SortedItemData; // st7
    bool v4; // [esp-4h] [ebp-B0h]
    float v5; // [esp+0h] [ebp-ACh]
    float v6; // [esp+4h] [ebp-A8h]
    float v7; // [esp+8h] [ebp-A4h]
    float v8; // [esp+Ch] [ebp-A0h]
    float topKDRforComparedPlayer; // [esp+14h] [ebp-98h]
    float topKDR; // [esp+18h] [ebp-94h]
    int infoBarNumber; // [esp+1Ch] [ebp-90h]
    Operand result; // [esp+20h] [ebp-8Ch] BYREF
    float infoBarMaxWidth; // [esp+28h] [ebp-84h]
    int currentWeaponStatView; // [esp+2Ch] [ebp-80h]
    float topSortKeyForComparedPlayer; // [esp+30h] [ebp-7Ch]
    int topIndexWithDefinedKDR; // [esp+34h] [ebp-78h]
    float width; // [esp+38h] [ebp-74h]
    float topSortKey; // [esp+3Ch] [ebp-70h]
    int infoBarForOtherPlayer; // [esp+40h] [ebp-6Ch]
    OperandList list; // [esp+44h] [ebp-68h] BYREF
    bool forFriend; // [esp+A3h] [ebp-9h]
    int currIndex; // [esp+A4h] [ebp-8h]
    int topIndexWithDefinedKDRforOtherPlayer; // [esp+A8h] [ebp-4h]

    GetOperandList(dataStack, &list);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( list.operandCount == 3 )
    {
        result.dataType = VAL_FLOAT;
        infoBarForOtherPlayer = GetSourceInt(list.operands).intVal;
        infoBarMaxWidth = GetSourceFloat(&list.operands[1]);
        infoBarNumber = GetSourceInt(&list.operands[2]).intVal;
        currentWeaponStatView = Dvar_GetInt("ui_combatCurrViewNum");
        forFriend = Dvar_GetBool("ui_showFriendsCombatRecord");
        currIndex = 0;
        width = 0.0f;
        topIndexWithDefinedKDR = 0;
        topIndexWithDefinedKDRforOtherPlayer = 0;
        if ( Dvar_GetBool("ui_combatStatsMilestonesVis") && (currentWeaponStatView == 7 || currentWeaponStatView == 9)
            || infoBarForOtherPlayer && !Dvar_GetBool("ui_combatComparisonModeOn") )
        {
            result.internals.intVal = 0;
            AddOperandToStack(dataStack, &result);
        }
        else
        {
            currIndex = sharedUiInfo.sortedItemPivot + infoBarNumber - 3;
            if ( sharedUiInfo.sortedItemPivot + infoBarNumber == 2 )
                currIndex = 0;
            if ( currIndex >= sharedUiInfo.clanTagFeature )
                --currIndex;
            if ( infoBarForOtherPlayer )
            {
                v4 = !forFriend;
                SortedItemData = LiveCombatRecord_GetSortedItemData(currIndex, forFriend, ITEM_INDEX);
                currIndex = LiveCombatRecord_GetSortedItemPositionByItemIndex((int)SortedItemData, v4);
                forFriend = !forFriend;
            }
            topSortKey = LiveCombatRecord_GetSortedItemData(0, forFriend, SORT_KEY);
            topSortKeyForComparedPlayer = LiveCombatRecord_GetSortedItemData(0, !forFriend, SORT_KEY);
            if ( currentWeaponStatView == 2 )
            {
                currIndex = infoBarNumber - 1;
                topSortKey = LiveCombatRecord_GetSortedItemData(infoBarNumber - 1, forFriend, PARAM1);
                topSortKeyForComparedPlayer = LiveCombatRecord_GetSortedItemData(currIndex, !forFriend, PARAM1);
                if ( topSortKey == 0.0 )
                {
                    width = 0.0f;
                }
                else if ( Dvar_GetBool("ui_combatComparisonModeOn") )
                {
                    if ( (float)(topSortKey - topSortKeyForComparedPlayer) < 0.0 )
                        v7 = topSortKeyForComparedPlayer;
                    else
                        v7 = topSortKey;
                    width = (float)(topSortKey / v7) * infoBarMaxWidth;
                }
                else
                {
                    width = infoBarMaxWidth;
                }
            }
            else if ( currentWeaponStatView == 5 )
            {
                if ( (int)LiveCombatRecord_GetSortedItemData(currIndex, forFriend, PARAM1) )
                {
                    if ( (int)LiveCombatRecord_GetSortedItemData(currIndex, forFriend, PARAM2) )
                    {
                        if ( LiveCombatRecord_GetSortedItemData(topIndexWithDefinedKDR, forFriend, SORT_KEY) == 2147483600.0 )
                        {
                            while ( LiveCombatRecord_GetSortedItemData(topIndexWithDefinedKDR, forFriend, SORT_KEY) == 2147483600.0 )
                                ++topIndexWithDefinedKDR;
                        }
                        topKDR = LiveCombatRecord_GetSortedItemData(topIndexWithDefinedKDR, forFriend, SORT_KEY);
                        if ( Dvar_GetBool("ui_combatComparisonModeOn") )
                        {
                            if ( LiveCombatRecord_GetSortedItemData(topIndexWithDefinedKDRforOtherPlayer, !forFriend, SORT_KEY) == 2147483600.0 )
                            {
                                while ( LiveCombatRecord_GetSortedItemData(topIndexWithDefinedKDRforOtherPlayer, !forFriend, SORT_KEY) == 2147483600.0 )
                                    ++topIndexWithDefinedKDRforOtherPlayer;
                            }
                            topKDRforComparedPlayer = LiveCombatRecord_GetSortedItemData(
                                                                                    topIndexWithDefinedKDRforOtherPlayer,
                                                                                    !forFriend,
                                                                                    SORT_KEY);
                            if ( (float)(topKDR - topKDRforComparedPlayer) < 0.0 )
                                v8 = topKDRforComparedPlayer;
                            else
                                v8 = topKDR;
                            width = LiveCombatRecord_GetSortedItemData(currIndex, forFriend, SORT_KEY) / v8 * infoBarMaxWidth;
                        }
                        else
                        {
                            width = LiveCombatRecord_GetSortedItemData(currIndex, forFriend, SORT_KEY) / topKDR * infoBarMaxWidth;
                        }
                    }
                    else
                    {
                        width = infoBarMaxWidth;
                    }
                }
                else
                {
                    width = 0.0f;
                }
            }
            else if ( Dvar_GetBool("ui_combatComparisonModeOn") )
            {
                if ( (float)(topSortKey - topSortKeyForComparedPlayer) < 0.0 )
                    v6 = topSortKeyForComparedPlayer;
                else
                    v6 = topSortKey;
                if ( v6 == 0.0 )
                {
                    width = 0.0f;
                }
                else
                {
                    if ( (float)(topSortKey - topSortKeyForComparedPlayer) < 0.0 )
                        v5 = topSortKeyForComparedPlayer;
                    else
                        v5 = topSortKey;
                    width = LiveCombatRecord_GetSortedItemData(currIndex, forFriend, SORT_KEY) / v5 * infoBarMaxWidth;
                }
            }
            else if ( topSortKey == 0.0 )
            {
                width = 0.0f;
            }
            else
            {
                width = LiveCombatRecord_GetSortedItemData(currIndex, forFriend, SORT_KEY) / topSortKey * infoBarMaxWidth;
            }
            result.internals.floatVal = width;
            AddOperandToStack(dataStack, &result);
        }
    }
    else
    {
        Expression_Error("Expected 3 parameters to GetCombatRecordInfoBarWidth()\n");
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetCombatRecordInfoBarText(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    double SortedItemData; // st7
    double v4; // st7
    const char *v5; // eax
    const char *v6; // eax
    const char *ColumnValueForRow; // eax
    double v8; // st7
    double v9; // st7
    double v10; // st7
    double v11; // st7
    double v12; // st7
    const char *v13; // eax
    double v14; // st7
    int v15; // [esp-4h] [ebp-4ECh]
    double v16; // [esp+0h] [ebp-4E8h]
    bool v17; // [esp+4h] [ebp-4E4h]
    int v18; // [esp+4h] [ebp-4E4h]
    int currValue; // [esp+Ch] [ebp-4DCh]
    int requiredValue; // [esp+14h] [ebp-4D4h]
    const char *currItemGroup; // [esp+18h] [ebp-4D0h]
    const StringTable *nextStatsMilestonesTable; // [esp+1Ch] [ebp-4CCh] BYREF
    const char *currStatsMilestoneString; // [esp+20h] [ebp-4C8h]
    const char *currMilestoneStat; // [esp+24h] [ebp-4C4h]
    int nextMilestoneRowNum; // [esp+28h] [ebp-4C0h]
    int currMilestone; // [esp+2Ch] [ebp-4BCh]
    const StringTable *statsMilestonesTable; // [esp+30h] [ebp-4B8h] BYREF
    int currMilestoneRowNum; // [esp+34h] [ebp-4B4h]
    int infoBarNumber; // [esp+38h] [ebp-4B0h]
    int comparisonMode; // [esp+3Ch] [ebp-4ACh]
    Operand result; // [esp+40h] [ebp-4A8h] BYREF
    int currentWeaponStatView; // [esp+48h] [ebp-4A0h]
    const char *itemName; // [esp+4Ch] [ebp-49Ch]
    char outputString[1024]; // [esp+50h] [ebp-498h] BYREF
    const char *suffix; // [esp+454h] [ebp-94h]
    OperandList list; // [esp+458h] [ebp-90h] BYREF
    bool forFriend; // [esp+4B3h] [ebp-35h]
    int currIndex; // [esp+4B4h] [ebp-34h]
    ConversionArguments convArgs; // [esp+4B8h] [ebp-30h] BYREF
    bool statsMilestonesVisible; // [esp+4E3h] [ebp-5h]
    const char *equipmentUsageString; // [esp+4E4h] [ebp-4h]

    GetOperandList(dataStack, &list);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( list.operandCount != 2 )
    {
        Com_PrintError(13, "UI Expression Error: Expected 2 parameters to GetCombatRecordInfoBarText()\n");
        AddOperandToStack(dataStack, &result);
        return;
    }
    comparisonMode = GetSourceInt(list.operands).intVal;
    infoBarNumber = GetSourceInt(&list.operands[1]).intVal;
    forFriend = Dvar_GetBool("ui_showFriendsCombatRecord");
    currentWeaponStatView = Dvar_GetInt("ui_combatCurrViewNum");
    currIndex = sharedUiInfo.sortedItemPivot + infoBarNumber - 3;
    if ( sharedUiInfo.sortedItemPivot + infoBarNumber == 2 )
        currIndex = 0;
    if ( currIndex >= sharedUiInfo.clanTagFeature )
        --currIndex;
    if ( comparisonMode )
    {
        v17 = !forFriend;
        SortedItemData = LiveCombatRecord_GetSortedItemData(currIndex, forFriend, ITEM_INDEX);
        currIndex = LiveCombatRecord_GetSortedItemPositionByItemIndex((int)SortedItemData, v17);
        forFriend = !forFriend;
    }
    statsMilestonesVisible = Dvar_GetBool("ui_combatStatsMilestonesVis");
    if ( !statsMilestonesVisible || currentWeaponStatView != 7 && currentWeaponStatView != 9 )
    {
        switch ( currentWeaponStatView )
        {
            case 5:
                if ( (int)LiveCombatRecord_GetSortedItemData(currIndex, forFriend, PARAM2) )
                {
                    v16 = LiveCombatRecord_GetSortedItemData(currIndex, forFriend, SORT_KEY);
                    v15 = (int)LiveCombatRecord_GetSortedItemData(currIndex, forFriend, PARAM2);
                    v9 = LiveCombatRecord_GetSortedItemData(currIndex, forFriend, PARAM1);
                    result.internals.intVal = (int)va("%d/%d ( %.2f )", (int)v9, v15, v16);
                }
                else
                {
                    v18 = (int)LiveCombatRecord_GetSortedItemData(currIndex, forFriend, PARAM2);
                    v8 = LiveCombatRecord_GetSortedItemData(currIndex, forFriend, PARAM1);
                    result.internals.intVal = (int)va("%d/%d", (int)v8, v18);
                }
                break;
            case 6:
                v10 = LiveCombatRecord_GetSortedItemData(currIndex, forFriend, SORT_KEY);
                result.internals.intVal = (int)va("%.2f %%", v10 * 100.0);
                break;
            case 13:
                memset(convArgs.args, 0, sizeof(convArgs.args));
                convArgs.argCount = 1;
                v11 = LiveCombatRecord_GetSortedItemData(currIndex, forFriend, PARAM1);
                convArgs.args[0] = va("%d", (int)v11);
                v12 = LiveCombatRecord_GetSortedItemData(currIndex, forFriend, ITEM_INDEX);
                itemName = BG_UnlockablesGetItemName((int)v12);
                suffix = LiveCombatRecord_GetEquipmentUsageStringSuffix(itemName);
                v13 = va("MPUI_CR_EQUIPMENT_%s", suffix);
                equipmentUsageString = UI_SafeTranslateString(v13);
                UI_ReplaceConversions((char *)equipmentUsageString, &convArgs, outputString, 1024);
                result.internals.intVal = (int)CopyTempString(outputString);
                break;
            default:
                v14 = LiveCombatRecord_GetSortedItemData(currIndex, forFriend, PARAM1);
                result.internals.intVal = (int)va("%d", (int)v14);
                break;
        }
LABEL_31:
        AddOperandToStack(dataStack, &result);
        return;
    }
    currMilestoneStat = 0;
    if ( currentWeaponStatView == 9 )
        currMilestoneStat = "headshots";
    else
        currMilestoneStat = "kills";
    if ( !currMilestoneStat )
        goto LABEL_31;
    v4 = LiveCombatRecord_GetSortedItemData(currIndex, forFriend, ITEM_INDEX);
    currItemGroup = BG_UnlockablesGetItemGroup((int)v4);
    currMilestone = (int)LiveCombatRecord_GetSortedItemData(currIndex, forFriend, PARAM3);
    v5 = va("%d", currMilestone);
    currMilestoneRowNum = CL_GetMilestoneRowNum(
                                                    (XAssetHeader *)&statsMilestonesTable,
                                                    3,
                                                    currItemGroup,
                                                    4,
                                                    currMilestoneStat,
                                                    1,
                                                    v5);
    v6 = va("%d", currMilestone + 1);
    nextMilestoneRowNum = CL_GetMilestoneRowNum(
                                                    (XAssetHeader *)&nextStatsMilestonesTable,
                                                    3,
                                                    currItemGroup,
                                                    4,
                                                    currMilestoneStat,
                                                    1,
                                                    v6);
    if ( nextMilestoneRowNum == -1 )
    {
        result.internals.intVal = (int)UI_SafeTranslateString("STATS_NO_MORE_MILESTONES");
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        currValue = (int)LiveCombatRecord_GetSortedItemData(currIndex, forFriend, PARAM1);
        ColumnValueForRow = StringTable_GetColumnValueForRow(statsMilestonesTable, currMilestoneRowNum, 2);
        requiredValue = atoi(ColumnValueForRow) - currValue;
        memset(convArgs.args, 0, sizeof(convArgs.args));
        convArgs.argCount = 4;
        convArgs.args[0] = va("%d", requiredValue);
        convArgs.args[1] = StringTable_GetColumnValueForRow(statsMilestonesTable, currMilestoneRowNum, 6);
        convArgs.args[2] = UI_SafeTranslateString("MENU_XP_CAPS");
        convArgs.args[3] = StringTable_GetColumnValueForRow(statsMilestonesTable, currMilestoneRowNum, 7);
        if ( currentWeaponStatView == 7 )
            currStatsMilestoneString = UI_SafeTranslateString("STATS_MILESTONES_COMBAT_RECORD_KILLS");
        else
            currStatsMilestoneString = UI_SafeTranslateString("STATS_MILESTONES_COMBAT_RECORD_HEADSHOTS");
        UI_ReplaceConversions((char *)currStatsMilestoneString, &convArgs, outputString, 1024);
        result.internals.intVal = (int)CopyTempString(outputString);
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetCopyClassDialogTitle(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *v3; // eax
    Operand source; // [esp+0h] [ebp-444h] BYREF
    Operand result; // [esp+8h] [ebp-43Ch] BYREF
    int classIndexToCopy; // [esp+10h] [ebp-434h]
    char outputString[1028]; // [esp+14h] [ebp-430h] BYREF
    ConversionArguments convArgs; // [esp+41Ch] [ebp-28h] BYREF

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    classIndexToCopy = GetSourceInt(&source).intVal;
    memset(convArgs.args, 0, sizeof(convArgs.args));
    convArgs.argCount = 1;
    convArgs.args[0] = Dvar_GetString(customClassDvars[classIndexToCopy]);
    v3 = UI_SafeTranslateString("MPUI_COPY_CLASS_TITLE");
    UI_ReplaceConversions(v3, &convArgs, outputString, 1024);
    result.internals.intVal = (int)CopyTempString(outputString);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCopyClassConfirmationText(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *v3; // eax
    Operand result; // [esp+0h] [ebp-4A0h] BYREF
    int classIndexToCopy; // [esp+8h] [ebp-498h]
    int classIndexToReplace; // [esp+Ch] [ebp-494h]
    char outputString[1028]; // [esp+10h] [ebp-490h] BYREF
    OperandList list; // [esp+418h] [ebp-88h] BYREF
    int selectedClassIndex; // [esp+474h] [ebp-2Ch]
    ConversionArguments convArgs; // [esp+478h] [ebp-28h] BYREF

    GetOperandList(dataStack, &list);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( list.operandCount >= 2 )
    {
        classIndexToCopy = GetSourceInt(list.operands).intVal;
        classIndexToReplace = GetSourceInt(&list.operands[1]).intVal;
        selectedClassIndex = classIndexToReplace;
        if ( classIndexToReplace >= classIndexToCopy )
            ++selectedClassIndex;
        memset(convArgs.args, 0, sizeof(convArgs.args));
        convArgs.argCount = 2;
        convArgs.args[0] = Dvar_GetString(customClassDvars[selectedClassIndex]);
        convArgs.args[1] = Dvar_GetString(customClassDvars[classIndexToCopy]);
        v3 = UI_SafeTranslateString("MPUI_COPY_CLASS_CONFIRMATION");
        UI_ReplaceConversions(v3, &convArgs, outputString, 1024);
        result.internals.intVal = (int)CopyTempString(outputString);
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Com_PrintError(13, "UI Expression Error: Expected 2 parameters to GetCopyClassDialogTitle()\n");
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetCopyCustomGametypeClassDialogTitle(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *v3; // eax
    Operand source; // [esp+0h] [ebp-444h] BYREF
    Operand result; // [esp+8h] [ebp-43Ch] BYREF
    int classIndexToCopy; // [esp+10h] [ebp-434h]
    char outputString[1028]; // [esp+14h] [ebp-430h] BYREF
    ConversionArguments convArgs; // [esp+41Ch] [ebp-28h] BYREF

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    memset(&convArgs, 0, sizeof(convArgs));
    classIndexToCopy = GetSourceInt(&source).intVal;
    convArgs.argCount = 1;
    convArgs.args[0] = g_customGameModeClassDescriptions[classIndexToCopy].name;
    v3 = UI_SafeTranslateString("MPUI_COPY_CLASS_TITLE");
    UI_ReplaceConversions(v3, &convArgs, outputString, 1024);
    result.internals.intVal = (int)CopyTempString(outputString);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCopyCustomGametypeClassConfirmationText(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *v3; // eax
    Operand result; // [esp+0h] [ebp-4A0h] BYREF
    int classIndexToCopy; // [esp+8h] [ebp-498h]
    int classIndexToReplace; // [esp+Ch] [ebp-494h]
    char outputString[1028]; // [esp+10h] [ebp-490h] BYREF
    OperandList list; // [esp+418h] [ebp-88h] BYREF
    int selectedClassIndex; // [esp+474h] [ebp-2Ch]
    ConversionArguments convArgs; // [esp+478h] [ebp-28h] BYREF

    GetOperandList(dataStack, &list);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( list.operandCount >= 2 )
    {
        classIndexToCopy = GetSourceInt(list.operands).intVal;
        classIndexToReplace = GetSourceInt(&list.operands[1]).intVal;
        selectedClassIndex = classIndexToReplace;
        if ( classIndexToReplace >= classIndexToCopy )
            ++selectedClassIndex;
        memset(&convArgs.args[2], 0, 28);
        convArgs.argCount = 2;
        convArgs.args[0] = g_customGameModeClassDescriptions[selectedClassIndex].name;
        convArgs.args[1] = g_customGameModeClassDescriptions[classIndexToCopy].name;
        v3 = UI_SafeTranslateString("MPUI_COPY_CLASS_CONFIRMATION");
        UI_ReplaceConversions(v3, &convArgs, outputString, 1024);
        result.internals.intVal = (int)CopyTempString(outputString);
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Com_PrintError(13, "UI Expression Error: Expected 2 parameters to GetCopyClassDialogTitle()\n");
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetCombatRecordInfoBarTagText(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *PersonalBestStatNameByIndex; // eax
    Operand source; // [esp+0h] [ebp-14h] BYREF
    int infoBarNumber; // [esp+8h] [ebp-Ch]
    Operand result; // [esp+Ch] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    infoBarNumber = GetSourceInt(&source).intVal;
    if ( Dvar_GetInt("ui_combatCurrViewNum") == 2 )
    {
        PersonalBestStatNameByIndex = LiveCombatRecord_GetPersonalBestStatNameByIndex(infoBarNumber);
        result.internals.intVal = (int)va("MPUI_%s", PersonalBestStatNameByIndex);
    }
    else
    {
        result.internals.intVal = (int)BG_UnlockablesGetItemName(infoBarNumber);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCombatRecordHistogramHeight(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+4h] [ebp-8Ch] BYREF
    float minScore; // [esp+Ch] [ebp-84h]
    float currScore; // [esp+10h] [ebp-80h]
    int numRecentValues; // [esp+14h] [ebp-7Ch]
    OperandList list; // [esp+18h] [ebp-78h] BYREF
    bool forFriend; // [esp+77h] [ebp-19h]
    int currIndex; // [esp+78h] [ebp-18h]
    int isValid; // [esp+7Ch] [ebp-14h]
    int itemIndex; // [esp+80h] [ebp-10h]
    int gameTypeIndex; // [esp+84h] [ebp-Ch]
    float maxScore; // [esp+88h] [ebp-8h]
    float fullHeight; // [esp+8Ch] [ebp-4h]

    GetOperandList(dataStack, &list);
    result.dataType = VAL_FLOAT;
    result.internals.intVal = 0;
    if ( list.operandCount == 2 )
    {
        itemIndex = GetSourceInt(list.operands).intVal;
        gameTypeIndex = GetSourceInt(&list.operands[1]).intVal;
        minScore = 0.0f;
        maxScore = 0.0f;
        currScore = 0.0f;
        currIndex = 0;
        forFriend = Dvar_GetBool("ui_showFriendsCombatRecord");
        isValid = 0;
        numRecentValues = 0;
        fullHeight = 1.0f;
        if ( gameTypeIndex == -1 )
        {
            numRecentValues = (int)LiveCombatRecord_GetSortedItemData(10, forFriend, ITEM_INDEX);
            minScore = LiveCombatRecord_GetSortedItemData(10, forFriend, PARAM1);
            maxScore = LiveCombatRecord_GetSortedItemData(10, forFriend, PARAM2);
            if ( itemIndex <= numRecentValues - 1 )
            {
                currScore = LiveCombatRecord_GetSortedItemData(numRecentValues - 1 - itemIndex, forFriend, PARAM1);
                isValid = (int)LiveCombatRecord_GetSortedItemData(numRecentValues - 1 - itemIndex, forFriend, ITEM_INDEX);
            }
        }
        else
        {
            numRecentValues = (int)LiveCombatRecord_GetSortedItemData(10 * gameTypeIndex, forFriend, ITEM_INDEX);
            minScore = LiveCombatRecord_GetSortedItemData(10 * gameTypeIndex, forFriend, PARAM1);
            maxScore = LiveCombatRecord_GetSortedItemData(10 * gameTypeIndex, forFriend, PARAM2);
            if ( itemIndex <= numRecentValues - 1 )
            {
                currIndex = 10 * gameTypeIndex + numRecentValues - 1 - itemIndex;
                currScore = LiveCombatRecord_GetSortedItemData(currIndex, forFriend, SORT_KEY);
                isValid = (int)LiveCombatRecord_GetSortedItemData(currIndex, forFriend, ITEM_INDEX);
            }
        }
        if ( isValid && numRecentValues > 1 )
        {
            if ( minScore >= 0.0 )
            {
                if ( maxScore == 0.0 )
                {
                    result.internals.floatVal = 0.05f;
                }
                else if ( currScore == 0.0 )
                {
                    result.internals.floatVal = 0.05f;
                }
                else
                {
                    result.internals.floatVal = currScore / maxScore;
                }
            }
            else if ( currScore == minScore || maxScore == minScore )
            {
                result.internals.floatVal = 0.05f;
            }
            else if ( currScore == maxScore )
            {
                result.internals.floatVal = fullHeight;
            }
            else
            {
                result.internals.floatVal = (float)(currScore - minScore) / (float)(maxScore - minScore);
            }
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Com_PrintError(13, "UI Expression Error: Expected 2 parameters to GetCombatRecordHistogramHeight()\n");
        AddOperandToStack(dataStack, &result);
    }
}

const char *lbWagerGameModeEnum_4[5] =
{ "oic", "hlnd", "gun", "shrp", NULL };

const char *lbTypeEnum_4[17] =
{
  "tdm",
  "dm",
  "ctf",
  "dom",
  "sab",
  "sd",
  "koth",
  "dem",
  "hctdm",
  "hcdm",
  "hcctf",
  "hcdom",
  "hcsab",
  "hcsd",
  "hckoth",
  "hcdem",
  NULL
};


void __cdecl GetCombatRecordPieChartText(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *v3; // eax
    char *v4; // eax
    const char *v5; // eax
    char *v6; // eax
    const char *v7; // eax
    char *v8; // eax
    char *v9; // eax
    char *v10; // eax
    int v11; // [esp-8h] [ebp-A8h]
    int v12; // [esp-8h] [ebp-A8h]
    char *v13; // [esp-4h] [ebp-A4h]
    char *v14; // [esp-4h] [ebp-A4h]
    char *v15; // [esp-4h] [ebp-A4h]
    char *itemName; // [esp+8h] [ebp-98h]
    int numMinutes; // [esp+Ch] [ebp-94h]
    int numHours; // [esp+10h] [ebp-90h]
    Operand result; // [esp+14h] [ebp-8Ch] BYREF
    int totalItemTypes; // [esp+1Ch] [ebp-84h]
    OperandList list; // [esp+20h] [ebp-80h] BYREF
    int totalItems; // [esp+78h] [ebp-28h]
    bool forFriend; // [esp+7Fh] [ebp-21h]
    int pieChartType; // [esp+80h] [ebp-20h]
    int itemTypes[3]; // [esp+84h] [ebp-1Ch]
    int numItems[3]; // [esp+90h] [ebp-10h]
    int textPosition; // [esp+9Ch] [ebp-4h]

    GetOperandList(dataStack, &list);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( list.operandCount != 2 )
    {
        Com_PrintError(13, "UI Expression Error: Expected 2 parameters to GetCombatRecordPieChartText()\n");
        AddOperandToStack(dataStack, &result);
        return;
    }
    pieChartType = GetSourceInt(list.operands).intVal;
    textPosition = GetSourceInt(&list.operands[1]).intVal;
    forFriend = Dvar_GetBool("ui_showFriendsCombatRecord");
    currIndex_0 = 0;
    startIndex_0 = 0;
    endIndex_0 = 0;
    totalItemTypes = 0;
    totalItems = 0;
    if ( !pieChartType )
    {
LABEL_6:
        startIndex_0 = 0;
        endIndex_0 = 3;
        goto LABEL_8;
    }
    if ( pieChartType != 1 )
    {
        if ( pieChartType != 2 )
            goto LABEL_8;
        goto LABEL_6;
    }
    startIndex_0 = 3;
    endIndex_0 = 6;
LABEL_8:
    while ( startIndex_0 < endIndex_0 )
    {
        itemTypes[currIndex_0] = (int)LiveCombatRecord_GetSortedItemData(startIndex_0, forFriend, ITEM_INDEX);
        numItems[currIndex_0] = (int)LiveCombatRecord_GetSortedItemData(startIndex_0, forFriend, SORT_KEY);
        if ( !numItems[currIndex_0] )
            break;
        ++totalItemTypes;
        totalItems += numItems[currIndex_0];
        ++startIndex_0;
        ++currIndex_0;
    }
    currIndex_0 = -1;
    if ( textPosition )
    {
        if ( textPosition == 1 )
        {
            if ( totalItemTypes >= 2 )
                currIndex_0 = 1;
        }
        else if ( textPosition == 2 && totalItemTypes >= 3 )
        {
            currIndex_0 = 2;
        }
    }
    else if ( totalItemTypes >= 1 )
    {
        currIndex_0 = 0;
    }
    if ( currIndex_0 != -1 )
    {
        if ( pieChartType )
        {
            if ( pieChartType == 1 )
            {
                v14 = UI_SafeTranslateString("MPUI_GAMES");
                v12 = numItems[currIndex_0];
                v5 = va("MPUI_%s_CAPS", lbWagerGameModeEnum_4[itemTypes[currIndex_0]]);
                v6 = UI_SafeTranslateString(v5);
                result.internals.intVal = (int)va("%s ( %d %s )", v6, v12, v14);
            }
            else
            {
                v7 = BG_UnlockablesGetItemName(itemTypes[currIndex_0]);
                itemName = UI_SafeTranslateString(v7);
                numHours = numItems[currIndex_0] / 3600;
                numItems[currIndex_0] %= 3600;
                numMinutes = numItems[currIndex_0] / 60;
                if ( numHours >= 1 && numMinutes )
                {
                    v15 = UI_SafeTranslateString("MPUI_MINUTES");
                    v8 = UI_SafeTranslateString("MPUI_HOURS");
                    result.internals.intVal = (int)va("%s ( %d %s %d %s )", itemName, numHours, v8, numMinutes, v15);
                }
                else if ( numHours < 1 || numMinutes )
                {
                    if ( numHours < 1 && numMinutes >= 0 )
                    {
                        v10 = UI_SafeTranslateString("MPUI_MINUTES");
                        result.internals.intVal = (int)va("%s ( %d %s )", itemName, numMinutes, v10);
                    }
                }
                else
                {
                    v9 = UI_SafeTranslateString("MPUI_HOURS");
                    result.internals.intVal = (int)va("%s ( %d %s )", itemName, numHours, v9);
                }
            }
        }
        else
        {
            v13 = UI_SafeTranslateString("MPUI_GAMES");
            v11 = numItems[currIndex_0];
            v3 = va("MPUI_%s_CAPS", lbTypeEnum_4[itemTypes[currIndex_0]]);
            v4 = UI_SafeTranslateString(v3);
            result.internals.intVal = (int)va("%s ( %d %s )", v4, v11, v13);
        }
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCombatRecordMinMaxScore(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int Int; // eax
    int v4; // eax
    int v5; // eax
    bool v6; // [esp-8h] [ebp-84h]
    bool v7; // [esp-8h] [ebp-84h]
    bool v8; // [esp-8h] [ebp-84h]
    Operand result; // [esp+4h] [ebp-78h] BYREF
    int minScore; // [esp+Ch] [ebp-70h]
    int graphType; // [esp+10h] [ebp-6Ch]
    OperandList list; // [esp+14h] [ebp-68h] BYREF
    bool forFriend; // [esp+6Fh] [ebp-Dh]
    int returnMinScore; // [esp+70h] [ebp-Ch]
    int maxScore; // [esp+74h] [ebp-8h]
    int count; // [esp+78h] [ebp-4h]

    GetOperandList(dataStack, &list);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( list.operandCount == 2 )
    {
        minScore = 0;
        maxScore = 0;
        count = 0;
        forFriend = Dvar_GetBool("ui_showFriendsCombatRecord");
        graphType = GetSourceInt(list.operands).intVal;
        returnMinScore = GetSourceInt(&list.operands[1]).intVal;
        if ( graphType )
        {
            if ( graphType == 1 )
            {
                v6 = forFriend;
                Int = Dvar_GetInt("ui_combatHistogramCurrGametype");
                minScore = (int)LiveCombatRecord_GetSortedItemData(10 * Int, v6, PARAM1);
                v7 = forFriend;
                v4 = Dvar_GetInt("ui_combatHistogramCurrGametype");
                maxScore = (int)LiveCombatRecord_GetSortedItemData(10 * v4, v7, PARAM2);
                v8 = forFriend;
                v5 = Dvar_GetInt("ui_combatHistogramCurrGametype");
                count = (int)LiveCombatRecord_GetSortedItemData(10 * v5, v8, ITEM_INDEX);
            }
            else if ( graphType == 2 )
            {
                minScore = (int)LiveCombatRecord_GetSortedItemData(10, forFriend, PARAM1);
                maxScore = (int)LiveCombatRecord_GetSortedItemData(10, forFriend, PARAM2);
            }
        }
        else
        {
            minScore = (int)LiveCombatRecord_GetSortedItemData(10, forFriend, PARAM1);
            maxScore = (int)LiveCombatRecord_GetSortedItemData(10, forFriend, PARAM2);
            count = (int)LiveCombatRecord_GetSortedItemData(10, forFriend, ITEM_INDEX);
        }
        if ( count >= 2 )
        {
            if ( returnMinScore )
            {
                if ( minScore >= 0 )
                    result.internals.intVal = (int)va("0");
                else
                    result.internals.intVal = (int)va("%d", minScore);
            }
            else if ( maxScore != minScore || maxScore > 0 )
            {
                result.internals.intVal = (int)va("%d", maxScore);
            }
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Com_PrintError(13, "UI Expression Error: Expected 2 parameters to GetCombatRecordMinMaxScore()\n");
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetCombatRecordFailedContracts(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    signed int activeContractIndex; // [esp+0h] [ebp-28h]
    signed int i; // [esp+4h] [ebp-24h]
    Operand result; // [esp+Ch] [ebp-1Ch] BYREF
    int numPurchasedContracts; // [esp+14h] [ebp-14h] BYREF
    int controllerIndex; // [esp+18h] [ebp-10h]
    int numCompletedContracts; // [esp+20h] [ebp-8h] BYREF
    int numContractsInProgress; // [esp+24h] [ebp-4h]

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    numCompletedContracts = 0;
    numPurchasedContracts = 0;
    numContractsInProgress = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( Dvar_GetBool("ui_showFriendsCombatRecord") )
    {
        LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_OTHERPLAYER, 1);
        LiveStats_GetIntOtherPlayerStatByKey(controllerIndex, &numCompletedContracts, MP_PLAYERSTATSKEY_CONTRACTS_COMPLETED);
        LiveStats_GetIntOtherPlayerStatByKey(controllerIndex, &numPurchasedContracts, MP_PLAYERSTATSKEY_CONTRACTS_PURCHASED);
        for ( i = 0; i < 3; ++i )
        {
            if ( LiveContracts_GetActiveContractStatus(controllerIndex, i) == 1 )
                ++numContractsInProgress;
        }
    }
    else
    {
        LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        LiveStats_GetIntPlayerStatByKey(controllerIndex, &numCompletedContracts, MP_PLAYERSTATSKEY_CONTRACTS_COMPLETED);
        LiveStats_GetIntPlayerStatByKey(controllerIndex, &numPurchasedContracts, MP_PLAYERSTATSKEY_CONTRACTS_PURCHASED);
        for ( activeContractIndex = 0; activeContractIndex < 3; ++activeContractIndex )
        {
            if ( LiveContracts_GetActiveContractStatus(controllerIndex, activeContractIndex) == 1 )
                ++numContractsInProgress;
        }
    }
    result.internals.intVal = numPurchasedContracts - (numContractsInProgress + numCompletedContracts);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCombatRecordLockedString(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *v3; // eax
    Operand source; // [esp+0h] [ebp-440h] BYREF
    Operand result; // [esp+8h] [ebp-438h] BYREF
    char outputString[1024]; // [esp+10h] [ebp-430h] BYREF
    const char *name; // [esp+414h] [ebp-2Ch]
    ConversionArguments convArgs; // [esp+418h] [ebp-28h] BYREF

    GetOperand(dataStack, &source);
    name = GetSourceString(source);
    result.dataType = VAL_STRING;
    memset(convArgs.args, 0, sizeof(convArgs.args));
    convArgs.argCount = 1;
    convArgs.args[0] = va("%s", name);
    v3 = UI_SafeTranslateString("MPUI_COMBAT_RECORD_LOCKED_FOR_FRIEND");
    UI_ReplaceConversions(v3, &convArgs, outputString, 1024);
    result.internals.intVal = (int)CopyTempString(outputString);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetNumWagerMatchesPlayed(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *PlayerStatStringByKey; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    int gameType; // [esp+0h] [ebp-20h]
    int losses; // [esp+4h] [ebp-1Ch] BYREF
    Operand result; // [esp+8h] [ebp-18h] BYREF
    int controllerIndex; // [esp+10h] [ebp-10h]
    int wins; // [esp+14h] [ebp-Ch] BYREF
    bool forFriend; // [esp+1Bh] [ebp-5h]
    int numMatches; // [esp+1Ch] [ebp-4h]

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    numMatches = 0;
    wins = 0;
    losses = 0;
    forFriend = Dvar_GetBool("ui_showFriendsCombatRecord");
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    for ( gameType = 0; gameType < 4; ++gameType )
    {
        if ( forFriend )
        {
            PlayerStatStringByKey = LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_WINS);
            LiveStats_GetIntOtherPlayerStatByGameMode(
                controllerIndex,
                &wins,
                lbWagerGameModeEnum_4[gameType],
                PlayerStatStringByKey);
            v4 = LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_LOSSES);
            LiveStats_GetIntOtherPlayerStatByGameMode(controllerIndex, &losses, lbWagerGameModeEnum_4[gameType], v4);
        }
        else
        {
            v5 = LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_WINS);
            LiveStats_GetIntPlayerStatByGameMode(controllerIndex, &wins, lbWagerGameModeEnum_4[gameType], v5);
            v6 = LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_LOSSES);
            LiveStats_GetIntPlayerStatByGameMode(controllerIndex, &losses, lbWagerGameModeEnum_4[gameType], v6);
        }
        numMatches += losses + wins;
    }
    result.internals.intVal = numMatches;
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetNumWagerMatchesWon(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *PlayerStatStringByKey; // eax
    const char *v4; // eax
    int gameType; // [esp+0h] [ebp-20h]
    Operand result; // [esp+8h] [ebp-18h] BYREF
    int controllerIndex; // [esp+10h] [ebp-10h]
    int wins; // [esp+14h] [ebp-Ch] BYREF
    bool forFriend; // [esp+1Bh] [ebp-5h]
    int numMatches; // [esp+1Ch] [ebp-4h]

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    numMatches = 0;
    wins = 0;
    forFriend = Dvar_GetBool("ui_showFriendsCombatRecord");
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    for ( gameType = 0; gameType < 4; ++gameType )
    {
        if ( forFriend )
        {
            PlayerStatStringByKey = LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_WINS);
            LiveStats_GetIntOtherPlayerStatByGameMode(
                controllerIndex,
                &wins,
                lbWagerGameModeEnum_4[gameType],
                PlayerStatStringByKey);
        }
        else
        {
            v4 = LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_WINS);
            LiveStats_GetIntPlayerStatByGameMode(controllerIndex, &wins, lbWagerGameModeEnum_4[gameType], v4);
        }
        numMatches += wins;
    }
    result.internals.intVal = numMatches;
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetIndexIntoMatchScoreboard(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int controllerIndex; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = CG_GetIndexIntoMatchScoreboard(localClientNum, controllerIndex);
    if ( result.internals.intVal < 0 )
        result.internals.intVal = 0;
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetScoreboardColumnHeader(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int columnNumber; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    columnNumber = GetSourceInt(&source).intVal;
    result.internals.intVal = (int)CG_GetNameForScoreboardColumn(localClientNum, columnNumber);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetWagerPlaceForMatchScoreboard(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int matchScoreboardIndex; // [esp+10h] [ebp-4h]

    result.dataType = VAL_INT;
    GetOperand(dataStack, &source);
    matchScoreboardIndex = GetSourceInt(&source).intVal;
    result.internals.intVal = CG_GetWagerPlaceForMatchScoreboard(localClientNum, matchScoreboardIndex);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetWagerGametypeNameFromEnum(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int gametypeIndex; // [esp+10h] [ebp-4h]

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    GetOperand(dataStack, &source);
    gametypeIndex = GetSourceInt(&source).intVal;
    if ( (unsigned int)gametypeIndex < 4 )
        result.internals.intVal = (int)lbWagerGameModeEnum_4[gametypeIndex];
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetAfterActionReportAwardsInfo(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    signed int currIndex; // [esp+0h] [ebp-44h]
    Operand source; // [esp+4h] [ebp-40h] BYREF
    Operand result; // [esp+Ch] [ebp-38h] BYREF
    char *liveStatsBuffer; // [esp+14h] [ebp-30h]
    int controllerIndex; // [esp+18h] [ebp-2Ch]
    int numContractsCompletedAfterMatch; // [esp+1Ch] [ebp-28h] BYREF
    char *oldStatsBuffer; // [esp+20h] [ebp-24h]
    int awardIndex; // [esp+24h] [ebp-20h]
    int totalPersonalBests; // [esp+28h] [ebp-1Ch]
    int numNewPersonalBests; // [esp+2Ch] [ebp-18h]
    int numAwards[3]; // [esp+30h] [ebp-14h]
    int numContractsCompleted; // [esp+3Ch] [ebp-8h]
    int numContractsCompletedBeforeMatch; // [esp+40h] [ebp-4h] BYREF

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    awardIndex = GetSourceInt(&source).intVal;
    if ( (unsigned int)awardIndex <= 2 )
    {
        numNewPersonalBests = 0;
        numContractsCompleted = 0;
        totalPersonalBests = LiveStats_GetNumPersonalBests(controllerIndex);
        for ( currIndex = 0; currIndex < totalPersonalBests; ++currIndex )
        {
            if ( LiveStats_GetPersonalBestDelta(controllerIndex, currIndex) > 0 )
                ++numNewPersonalBests;
        }
        numAwards[0] = numNewPersonalBests;
        numAwards[1] = LiveStats_GetNumStatsMilestones(controllerIndex);
        liveStatsBuffer = (char *)LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        oldStatsBuffer = (char *)LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_STABLE, 1);
        numContractsCompletedBeforeMatch = 0;
        numContractsCompletedAfterMatch = 0;
        LiveStats_GetIntPlayerStatFromBase(&numContractsCompletedBeforeMatch, "CONTRACTS_COMPLETED", oldStatsBuffer);
        LiveStats_GetIntPlayerStatFromBase(&numContractsCompletedAfterMatch, "CONTRACTS_COMPLETED", liveStatsBuffer);
        numContractsCompleted = numContractsCompletedAfterMatch - numContractsCompletedBeforeMatch;
        numAwards[2] = numContractsCompletedAfterMatch - numContractsCompletedBeforeMatch;
        result.internals.intVal = numAwards[awardIndex];
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetNumPersonalBests(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = LiveStats_GetNumPersonalBests(ControllerIndex);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetPersonalBestName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    char *PersonalBestName; // eax
    unsigned int v5; // [esp-4h] [ebp-18h]
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    index = GetSourceInt(&source).intVal;
    v5 = index;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    PersonalBestName = LiveStats_GetPersonalBestName(ControllerIndex, v5);
    result.internals.intVal = (int)CopyTempString(PersonalBestName);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetPersonalBestPrefix(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    unsigned int v4; // [esp-4h] [ebp-1Ch]
    Operand source; // [esp+0h] [ebp-18h] BYREF
    Operand result; // [esp+8h] [ebp-10h] BYREF
    int index; // [esp+10h] [ebp-8h]
    const char *prefix; // [esp+14h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    index = GetSourceInt(&source).intVal;
    v4 = index;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    prefix = LiveStats_GetPersonalBestPrefix(ControllerIndex, v4);
    if ( prefix && *prefix )
        result.internals.intVal = (int)CopyTempString(prefix);
    else
        result.internals.intVal = (int)"NULL";
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetPersonalBestValue(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *PlayerStatStringByKey; // eax
    const char *v6; // eax
    float v7; // [esp+8h] [ebp-34h]
    __int64 PersonalBestValue; // [esp+14h] [ebp-28h]
    float v9; // [esp+1Ch] [ebp-20h]
    Operand source; // [esp+20h] [ebp-1Ch] BYREF
    Operand result; // [esp+28h] [ebp-14h] BYREF
    int controllerIndex; // [esp+30h] [ebp-Ch]
    const char *personalBestName; // [esp+34h] [ebp-8h]
    int index; // [esp+38h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    index = GetSourceInt(&source).intVal;
    personalBestName = LiveStats_GetPersonalBestName(controllerIndex, index);
    if ( !personalBestName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    9283,
                    0,
                    "%s",
                    "personalBestName") )
    {
        __debugbreak();
    }
    if ( (!personalBestName
         || (v3 = LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_KDRATIO), I_stricmp(personalBestName, v3)))
        && (v4 = LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_HIGHEST_KDRATIO), I_stricmp(personalBestName, v4)) )
    {
        if ( (!personalBestName
             || (PlayerStatStringByKey = LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_ACCURACY),
                     I_stricmp(personalBestName, PlayerStatStringByKey)))
            && (v6 = LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_HIGHEST_ACCURACY), I_stricmp(personalBestName, v6)) )
        {
            result.internals.intVal = LiveStats_GetPersonalBestValue(controllerIndex, index);
        }
        else
        {
            result.dataType = VAL_STRING;
            v9 = (double)(unsigned int)LiveStats_GetPersonalBestValue(controllerIndex, index) / 10000.0 * 100.0;
            if ( (float)(v9 - 100.0) < 0.0 )
                v7 = v9;
            else
                v7 = 100.0f;
            result.internals.intVal = (int)va("%.2f %%", v7);
        }
    }
    else
    {
        result.dataType = VAL_STRING;
        PersonalBestValue = (unsigned int)LiveStats_GetPersonalBestValue(controllerIndex, index);
        result.internals.intVal = (int)va("%.2f", (double)PersonalBestValue / 1000.0);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetPersonalBestDelta(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    unsigned int v4; // [esp-4h] [ebp-18h]
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int index; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    index = GetSourceInt(&source).intVal;
    v4 = index;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = LiveStats_GetPersonalBestDelta(ControllerIndex, v4);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetNumWeaponUnlocks(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    signed int currItem; // [esp+0h] [ebp-20h]
    Operand result; // [esp+4h] [ebp-1Ch] BYREF
    int controllerIndex; // [esp+Ch] [ebp-14h]
    loadoutSlot_t currLoadoutSlot; // [esp+10h] [ebp-10h]
    int itemIndex; // [esp+14h] [ebp-Ch]
    int totalWeaponsUnlocked; // [esp+18h] [ebp-8h]
    int totalItemsUnlocked; // [esp+1Ch] [ebp-4h]

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    totalItemsUnlocked = LiveStats_GetNumRecentlyUnlockedItems(controllerIndex);
    totalWeaponsUnlocked = 0;
    itemIndex = 0;
    for ( currItem = 0; currItem < totalItemsUnlocked; ++currItem )
    {
        itemIndex = LiveStats_GetRecentlyUnlockedItemIndex(controllerIndex, currItem);
        currLoadoutSlot = (loadoutSlot_t)BG_UnlockablesGetItemLoadoutSlot(itemIndex);
        if ( currLoadoutSlot == LOADOUTSLOT_FIRST
            || currLoadoutSlot == LOADOUTSLOT_SECONDARY_WEAPON
            || BG_UnlockablesGetItemGroupEnum(itemIndex) == 14 && BG_UnlockablesIsProItem(itemIndex) )
        {
            ++totalWeaponsUnlocked;
        }
    }
    result.internals.intVal = totalWeaponsUnlocked;
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetNumFeatureUnlocks(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    signed int currItem; // [esp+0h] [ebp-20h]
    Operand result; // [esp+8h] [ebp-18h] BYREF
    int controllerIndex; // [esp+10h] [ebp-10h]
    int totalFeaturesUnlocked; // [esp+14h] [ebp-Ch]
    int itemIndex; // [esp+18h] [ebp-8h]
    int totalItemsUnlocked; // [esp+1Ch] [ebp-4h]

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    totalItemsUnlocked = LiveStats_GetNumRecentlyUnlockedItems(controllerIndex);
    totalFeaturesUnlocked = 0;
    itemIndex = 0;
    for ( currItem = 0; currItem < totalItemsUnlocked; ++currItem )
    {
        itemIndex = LiveStats_GetRecentlyUnlockedItemIndex(controllerIndex, currItem);
        if ( BG_UnlockablesGetItemGroupEnum(itemIndex) == 21 )
            ++totalFeaturesUnlocked;
    }
    result.internals.intVal = totalFeaturesUnlocked;
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetNumStatsMilestones(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int ControllerIndex; // eax
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = LiveStats_GetNumStatsMilestones(ControllerIndex);
    AddOperandToStack(dataStack, &result);
}

const char *__cdecl GetStatsMilestoneData(
                milestoneTableColumns_t comparisonColumn1,
                const char *value1,
                milestoneTableColumns_t comparisonColumn2,
                const char *value2,
                milestoneTableColumns_t comparisonColumn3,
                const char *value3,
                milestoneTableColumns_t returnColumn)
{
    int rowNum; // [esp+0h] [ebp-8h]
    const StringTable *statsMilestonestable; // [esp+4h] [ebp-4h] BYREF

    rowNum = CL_GetMilestoneRowNum(
                         (XAssetHeader *)&statsMilestonestable,
                         comparisonColumn1,
                         value1,
                         comparisonColumn2,
                         value2,
                         comparisonColumn3,
                         value3);
    if ( rowNum == -1 )
        return "";
    else
        return StringTable_GetColumnValueForRow(statsMilestonestable, rowNum, returnColumn);
}

void __cdecl GetStatsMilestoneValue(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    unsigned int NumStatsMilestones; // eax
    const char *v4; // eax
    const char *StatsMilestoneData; // eax
    const char *v6; // eax
    Operand source; // [esp+4h] [ebp-38h] BYREF
    Operand result; // [esp+Ch] [ebp-30h] BYREF
    statsMilestoneTypes_t statsMilestoneType; // [esp+14h] [ebp-28h]
    int controllerIndex; // [esp+18h] [ebp-24h]
    const char *statsMilestoneItemGroup; // [esp+1Ch] [ebp-20h]
    const StringTable *statsTable; // [esp+20h] [ebp-1Ch] BYREF
    int index; // [esp+24h] [ebp-18h]
    const char *statsMilestoneTierString; // [esp+28h] [ebp-14h]
    const char *statsMilestoneName; // [esp+2Ch] [ebp-10h]
    int statsMilestoneItemIndex; // [esp+30h] [ebp-Ch]
    const char *statType; // [esp+34h] [ebp-8h]
    int statsMilestoneTier; // [esp+38h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    index = GetSourceInt(&source).intVal;
    if ( index >= 0
        && (NumStatsMilestones = LiveStats_GetNumStatsMilestones(controllerIndex), index <= (int)(NumStatsMilestones - 1)) )
    {
        StringTable_GetAsset("mp/statsTable.csv", (XAssetHeader *)&statsTable);
        statsMilestoneTier = LiveStats_GetStatsMilestoneTier(controllerIndex, index);
        statsMilestoneTierString = va("%d", statsMilestoneTier - 1);
        statsMilestoneType = (statsMilestoneTypes_t)LiveStats_GetStatsMilestoneType(controllerIndex, index);
        statsMilestoneName = LiveStats_GetStatsMilestoneName(controllerIndex, index);
        statType = 0;
        switch ( statsMilestoneType )
        {
            case MILESTONE_GLOBAL:
                statType = "global";
                goto LABEL_12;
            case MILESTONE_WEAPON:
                statsMilestoneItemIndex = LiveStats_GetStatsMilestoneItemIndex(controllerIndex, index);
                if ( statsMilestoneItemIndex == -1 )
                    goto LABEL_12;
                v4 = va("%d", statsMilestoneItemIndex);
                statsMilestoneItemGroup = StringTable_Lookup(statsTable, 0, v4, 2);
                StatsMilestoneData = GetStatsMilestoneData(
                                                             MILESTONE_COLUMN_TIERID,
                                                             statsMilestoneTierString,
                                                             MILESTONE_COLUMN_STATTYPE,
                                                             statsMilestoneItemGroup,
                                                             MILESTONE_COLUMN_STATNAME,
                                                             statsMilestoneName,
                                                             MILESTONE_COLUMN_TARGETVALUE);
                result.internals.intVal = atoi(StatsMilestoneData);
                AddOperandToStack(dataStack, &result);
                break;
            case MILESTONE_GAMEMODE:
                statType = "gamemode";
                goto LABEL_12;
            case MILESTONE_GROUP:
                statType = "group";
                goto LABEL_12;
            case MILESTONE_ATTACHMENTS:
                statType = "attachment";
                goto LABEL_12;
            default:
LABEL_12:
                if ( statType )
                {
                    if ( *statType )
                    {
                        v6 = GetStatsMilestoneData(
                                     MILESTONE_COLUMN_TIERID,
                                     statsMilestoneTierString,
                                     MILESTONE_COLUMN_STATTYPE,
                                     statType,
                                     MILESTONE_COLUMN_STATNAME,
                                     statsMilestoneName,
                                     MILESTONE_COLUMN_TARGETVALUE);
                        result.internals.intVal = atoi(v6);
                    }
                }
                AddOperandToStack(dataStack, &result);
                break;
        }
    }
    else
    {
        result.dataType = VAL_STRING;
        result.internals.intVal = (int)"";
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetStatsMilestoneName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    unsigned int NumStatsMilestones; // eax
    const char *ItemGroupName; // eax
    const char *v5; // eax
    char *v6; // eax
    Operand source; // [esp+0h] [ebp-42Ch] BYREF
    const char *statMilestoneName; // [esp+8h] [ebp-424h]
    Operand result; // [esp+Ch] [ebp-420h] BYREF
    statsMilestoneTypes_t statsMilestoneType; // [esp+14h] [ebp-418h]
    int controllerIndex; // [esp+18h] [ebp-414h]
    char outputString[1024]; // [esp+1Ch] [ebp-410h] BYREF
    int milestoneTier; // [esp+420h] [ebp-Ch]
    int index; // [esp+424h] [ebp-8h]
    int statsMilestoneItemIndex; // [esp+428h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    memset(outputString, 0, sizeof(outputString));
    index = GetSourceInt(&source).intVal;
    if ( index >= 0
        && (NumStatsMilestones = LiveStats_GetNumStatsMilestones(controllerIndex), index <= (int)(NumStatsMilestones - 1)) )
    {
        statsMilestoneItemIndex = LiveStats_GetStatsMilestoneItemIndex(controllerIndex, index);
        if ( statsMilestoneItemIndex == -1 )
        {
            result.internals.intVal = (int)"";
            AddOperandToStack(dataStack, &result);
        }
        else
        {
            statsMilestoneType = (statsMilestoneTypes_t)LiveStats_GetStatsMilestoneType(controllerIndex, index);
            statMilestoneName = LiveStats_GetStatsMilestoneName(controllerIndex, index);
            milestoneTier = LiveStats_GetStatsMilestoneTier(controllerIndex, index) - 1;
            if ( statsMilestoneType == MILESTONE_GROUP )
            {
                ItemGroupName = BG_UnlockablesGetItemGroupName((itemGroup_t)statsMilestoneItemIndex);
                v5 = va("STATS_TYPE_%s", ItemGroupName);
                v6 = UI_SafeTranslateString(v5);
                CL_GetMilestoneLocalizedName(
                    outputString,
                    256,
                    statMilestoneName,
                    statsMilestoneType,
                    (eAttachment)statsMilestoneItemIndex,
                    milestoneTier,
                    0,
                    v6);
            }
            else
            {
                CL_GetMilestoneLocalizedName(
                    outputString,
                    256,
                    statMilestoneName,
                    statsMilestoneType,
                    (eAttachment)statsMilestoneItemIndex,
                    milestoneTier,
                    0,
                    0);
            }
            result.internals.intVal = (int)CopyTempString(outputString);
            AddOperandToStack(dataStack, &result);
        }
    }
    else
    {
        AddOperandToStack(dataStack, &result);
    }
}

char __cdecl GetCurrentIndex(
                int localClientNum,
                itemDef_s *item,
                OperandStack *dataStack,
                Operand *result,
                int *challengeNum,
                operandInternalDataUnion *milestoneType,
                char *challengeType)
{
    char *SourceString; // eax
    const char *stringResult; // [esp+0h] [ebp-6Ch] BYREF
    OperandList list; // [esp+4h] [ebp-68h] BYREF
    float currIndex; // [esp+60h] [ebp-Ch] BYREF
    itemDef_s *actualItem; // [esp+64h] [ebp-8h]
    const char *feederName = NULL; // [esp+68h] [ebp-4h]

    currIndex = 0.0f;
    GetOperandList(dataStack, &list);
    if ( list.operandCount >= 2 )
    {
        if ( list.operandCount == 2 )
        {
            feederName = GetSourceString(list.operands[0]);
            milestoneType->intVal = GetOperandValueInt(&list.operands[1]).intVal;
        }
        else if ( list.operandCount == 3 && challengeType )
        {
            feederName = GetSourceString(list.operands[0]);
            milestoneType->intVal = GetOperandValueInt(&list.operands[1]).intVal;
            SourceString = GetSourceString(list.operands[2]);
            Com_sprintf(challengeType, 0x100u, "%s", SourceString);
        }
        if ( feederName && *feederName )
        {
            actualItem = Menu_GetMatchingItemByNumber(item->parent, 0, (char *)feederName);
            if ( UI_FeederData(localClientNum, actualItem, "selection", (char **)&stringResult, &currIndex) )
            {
                *challengeNum = (int)currIndex;
                return 1;
            }
            else
            {
                AddOperandToStack(dataStack, result);
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Expression_Error("Expected 2 parameters to GetName()\n");
        AddOperandToStack(dataStack, result);
        return 0;
    }
}

void __cdecl GetCurrentChallengeXpReward(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int xpReward; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+4h] [ebp-10h] BYREF
    int milestoneType; // [esp+Ch] [ebp-8h] BYREF
    int challengeNum; // [esp+10h] [ebp-4h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    challengeNum = 0;
    xpReward = 0;
    milestoneType = 0;
    if ( GetCurrentIndexOfHighlightedFeeder(localClientNum, item, dataStack, &result, &challengeNum, &milestoneType, 0) )
    {
        if ( LiveStats_GetChallengeInfoParam(CHALLENGE_PARAM_XP_REWARD, challengeNum, 0, &xpReward, milestoneType) )
            result.internals.intVal = xpReward;
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        AddOperandToStack(dataStack, &result);
    }
}

char __cdecl GetCurrentIndexOfHighlightedFeeder(
                int localClientNum,
                itemDef_s *item,
                OperandStack *dataStack,
                Operand *result,
                int *challengeNum,
                int *milestoneType,
                char *name)
{
    const char *stringResult; // [esp+0h] [ebp-10h] BYREF
    float currIndex; // [esp+4h] [ebp-Ch] BYREF
    itemDef_s *actualItem; // [esp+8h] [ebp-8h]
    int contextIndex; // [esp+Ch] [ebp-4h]

    currIndex = 0.0f;
    if ( !name )
        name = (char*)"stats_milestones_list";
    contextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
    actualItem = Menu_GetMatchingItemByNumber(item->parent, 0, name);
    *milestoneType = 1;
    if ( !Window_HasFocus(contextIndex, &actualItem->window) )
    {
        actualItem = Menu_GetMatchingItemByNumber(item->parent, 0, (char*)"stats_milestones_weapongrouplist");
        *milestoneType = 4;
    }
    if ( actualItem && UI_FeederData(localClientNum, actualItem, "selection", (char **)&stringResult, &currIndex) )
    {
        *challengeNum = (int)currIndex;
        return 1;
    }
    else
    {
        AddOperandToStack(dataStack, result);
        return 0;
    }
}

void __cdecl IsCurrentChallengeItemClassified(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int isItemClassified; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+4h] [ebp-10h] BYREF
    int milestoneType; // [esp+Ch] [ebp-8h] BYREF
    int challengeNum; // [esp+10h] [ebp-4h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    challengeNum = 0;
    milestoneType = 0;
    isItemClassified = 0;
    if ( GetCurrentIndexOfHighlightedFeeder(localClientNum, item, dataStack, &result, &challengeNum, &milestoneType, 0) )
    {
        if ( LiveStats_GetChallengeInfoParam(
                     CHALLENGE_PARAM_IS_ITEM_CLASSIFIED,
                     challengeNum,
                     0,
                     &isItemClassified,
                     milestoneType) )
        {
            result.internals.intVal = isItemClassified;
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetXpReward(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int xpReward; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+4h] [ebp-10h] BYREF
    int milestoneType; // [esp+Ch] [ebp-8h] BYREF
    int challengeNum; // [esp+10h] [ebp-4h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    challengeNum = 0;
    xpReward = 0;
    milestoneType = 0;
    if ( GetCurrentIndex(
                 localClientNum,
                 item,
                 dataStack,
                 &result,
                 &challengeNum,
                 (operandInternalDataUnion *)&milestoneType,
                 0) )
    {
        if ( LiveStats_GetChallengeInfoParam(CHALLENGE_PARAM_XP_REWARD, challengeNum, 0, &xpReward, milestoneType) )
            result.internals.intVal = xpReward;
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetCurrentChallengeCpReward(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-14h] BYREF
    int milestoneType; // [esp+8h] [ebp-Ch] BYREF
    int challengeNum; // [esp+Ch] [ebp-8h] BYREF
    int cpReward; // [esp+10h] [ebp-4h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    challengeNum = 0;
    cpReward = 0;
    milestoneType = 0;
    if ( GetCurrentIndexOfHighlightedFeeder(localClientNum, item, dataStack, &result, &challengeNum, &milestoneType, 0)
        && LiveStats_GetChallengeInfoParam(CHALLENGE_PARAM_CP_REWARD, challengeNum, 0, &cpReward, milestoneType) )
    {
        result.internals.intVal = cpReward;
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCpReward(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-14h] BYREF
    int milestoneType; // [esp+8h] [ebp-Ch] BYREF
    int challengeNum; // [esp+Ch] [ebp-8h] BYREF
    int cpReward; // [esp+10h] [ebp-4h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    challengeNum = 0;
    cpReward = 0;
    milestoneType = 0;
    if ( GetCurrentIndex(
                 localClientNum,
                 item,
                 dataStack,
                 &result,
                 &challengeNum,
                 (operandInternalDataUnion *)&milestoneType,
                 0)
        && LiveStats_GetChallengeInfoParam(CHALLENGE_PARAM_CP_REWARD, challengeNum, 0, &cpReward, milestoneType) )
    {
        result.internals.intVal = cpReward;
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCurrentChallengeProgress(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *PlayerStatStringByKey; // eax
    const char *name; // [esp-4h] [ebp-48h]
    Operand result; // [esp+20h] [ebp-24h] BYREF
    int milestoneType; // [esp+28h] [ebp-1Ch] BYREF
    const char *targetTime; // [esp+2Ch] [ebp-18h]
    challengeInfo_t *challenge; // [esp+30h] [ebp-14h] BYREF
    const char *currTime; // [esp+34h] [ebp-10h]
    int currMinutes; // [esp+38h] [ebp-Ch]
    int challengeNum; // [esp+3Ch] [ebp-8h] BYREF
    int targetMinutes; // [esp+40h] [ebp-4h]

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    challengeNum = 0;
    milestoneType = 0;
    if ( GetCurrentIndexOfHighlightedFeeder(localClientNum, item, dataStack, &result, &challengeNum, &milestoneType, 0)
        && LiveStats_GetChallengeInfo(&challenge, challengeNum, milestoneType) )
    {
        name = challenge->name;
        PlayerStatStringByKey = LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_TIMEPLAYEDTOTAL);
        if ( !I_stricmp(PlayerStatStringByKey, name) )
        {
            currMinutes = (int)((float)((float)challenge->currentValue / 60.0) + 9.313225746154785e-10);
            currTime = va("%id %ih %im", currMinutes / 1440, currMinutes % 1440 / 60, currMinutes % 60);
            targetMinutes = (int)((float)((float)challenge->targetValue / 60.0) + 9.313225746154785e-10);
            targetTime = va("%id %ih %im", targetMinutes / 1440, targetMinutes % 1440 / 60, targetMinutes % 60);
            result.internals.intVal = (int)va("%s / %s", currTime, targetTime);
            AddOperandToStack(dataStack, &result);
            return;
        }
        result.internals.intVal = (int)va("%d / %d", challenge->currentValue, challenge->targetValue);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetNumChallenges(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-14h] BYREF
    Operand result; // [esp+8h] [ebp-Ch] BYREF
    int type; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    type = GetSourceInt(&source).intVal;
    result.internals.intVal = LiveStats_GetChallengeCount(type);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetGameInvitesCount(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = Live_GetInvitesCount();
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetChallengeProgressString(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    char *v6; // eax
    float v7; // [esp+8h] [ebp-44Ch]
    Operand result; // [esp+10h] [ebp-444h] BYREF
    int milestoneType; // [esp+18h] [ebp-43Ch] BYREF
    char outputString[1024]; // [esp+1Ch] [ebp-438h] BYREF
    float percentage; // [esp+420h] [ebp-34h]
    challengeInfo_t *challenge; // [esp+424h] [ebp-30h] BYREF
    int challengeNum; // [esp+428h] [ebp-2Ch] BYREF
    ConversionArguments convArgs; // [esp+42Ch] [ebp-28h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    challengeNum = 0;
    memset(outputString, 0, sizeof(outputString));
    memset(&convArgs, 0, sizeof(convArgs));
    milestoneType = 0;
    if ( GetCurrentIndexOfHighlightedFeeder(localClientNum, item, dataStack, &result, &challengeNum, &milestoneType, 0)
        && LiveStats_GetChallengeInfo(&challenge, challengeNum, milestoneType)
        && challenge->targetValue )
    {
        if ( challenge->targetValue == 1 )
        {
            result.internals.intVal = (int)UI_SafeTranslateString("MPUI_CONTRACT_NOT_COMPLETE");
            AddOperandToStack(dataStack, &result);
            return;
        }
        percentage = (float)(100 * challenge->currentValue) / (float)challenge->targetValue;
        v7 = floor(percentage);
        v3 = va("%d", (int)v7);
        convArgs.args[convArgs.argCount++] = v3;
        v4 = va("%d", challenge->currentValue);
        convArgs.args[convArgs.argCount++] = v4;
        v5 = va("%d", challenge->targetValue);
        convArgs.args[convArgs.argCount++] = v5;
        v6 = UI_SafeTranslateString("STATS_STATSMILESTONE_PROGRESS");
        UI_ReplaceConversions(v6, &convArgs, outputString, 1024);
        result.internals.intVal = (int)CopyTempString(outputString);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetLbTypeWithButtons(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *String; // eax
    const char *v4; // eax
    const StringTable *gameTypesTable; // [esp+0h] [ebp-440h] BYREF
    Operand result; // [esp+4h] [ebp-43Ch] BYREF
    const char *gameTypeName; // [esp+Ch] [ebp-434h]
    char outputString[1024]; // [esp+10h] [ebp-430h] BYREF
    char binding[4]; // [esp+414h] [ebp-2Ch] BYREF
    ConversionArguments convArgs; // [esp+418h] [ebp-28h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    StringTable_GetAsset("mp/gametypesTable.csv", (XAssetHeader *)&gameTypesTable);
    if ( !gameTypesTable
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    9889,
                    0,
                    "%s",
                    "gameTypesTable") )
    {
        __debugbreak();
    }
    String = Dvar_GetString("lb_type");
    v4 = StringTable_Lookup(gameTypesTable, 1, String, 2);
    gameTypeName = UI_SafeTranslateString(v4);
    binding[0] = 5;
    binding[1] = 0;
    binding[2] = 6;
    binding[3] = 0;
    memset(&convArgs.args[2], 0, 28);
    convArgs.args[0] = binding;
    convArgs.args[1] = &binding[2];
    convArgs.argCount = 2;
    UI_ReplaceConversions((char *)gameTypeName, &convArgs, outputString, 1024);
    result.internals.intVal = (int)CopyTempString(outputString);
    AddOperandToStack(dataStack, &result);
}

void __cdecl PrepareProgressDescription(int milestoneType, char *outputString, challengeInfo_t *challenge)
{
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    float v7; // [esp+8h] [ebp-40h]
    float v8; // [esp+Ch] [ebp-3Ch]
    ConversionArguments convArgs; // [esp+20h] [ebp-28h] BYREF

    memset(&convArgs, 0, sizeof(convArgs));
    if ( challenge->targetValue != 1 || challenge->currentValue )
    {
        if ( challenge->targetValue == 1 && challenge->currentValue == 1 )
        {
            v4 = UI_SafeTranslateString("STATS_CHALLENGE_COMPLETED");
            UI_ReplaceConversions(v4, &convArgs, outputString, 1024);
        }
        else if ( milestoneType )
        {
            if ( milestoneType == 5 || milestoneType == 6 || milestoneType == 7 )
            {
                v7 = floor((float)((float)(100 * challenge->currentValue) / (float)challenge->targetValue));
                convArgs.args[convArgs.argCount++] = va("%d", (int)v7);
                convArgs.args[convArgs.argCount++] = va("%d", challenge->currentValue);
                convArgs.args[convArgs.argCount++] = va("%d", challenge->targetValue);
                v6 = UI_SafeTranslateString("STATS_STATSMILESTONE_PROGRESS");
                UI_ReplaceConversions(v6, &convArgs, outputString, 1024);
            }
        }
        else
        {
            v8 = floor((float)((float)(100 * challenge->currentValue) / (float)challenge->targetValue));
            convArgs.args[convArgs.argCount++] = va("%d", (int)v8);
            if ( I_stricmp(challenge->name, "PERKS_MARATHON_MILE") )
            {
                if ( I_stricmp(challenge->name, "time_played_total") )
                {
                    convArgs.args[convArgs.argCount++] = va("%d", challenge->currentValue);
                    convArgs.args[convArgs.argCount] = va("%d", challenge->targetValue);
                }
                else
                {
                    convArgs.args[convArgs.argCount++] = va("%.1f", (float)((float)challenge->currentValue / 3600.0));
                    convArgs.args[convArgs.argCount] = va("%.1f", (float)((float)challenge->targetValue / 3600.0));
                }
                ++convArgs.argCount;
            }
            else
            {
                convArgs.args[convArgs.argCount++] = va("%.1f", (float)((float)challenge->currentValue / 633.0));
                convArgs.args[convArgs.argCount++] = va("%.1f", (float)((float)challenge->targetValue / 633.0));
            }
            v5 = UI_SafeTranslateString("STATS_STATSMILESTONE_PROGRESS");
            UI_ReplaceConversions(v5, &convArgs, outputString, 1024);
        }
    }
    else
    {
        v3 = UI_SafeTranslateString("MPUI_CONTRACT_NOT_COMPLETE");
        UI_ReplaceConversions(v3, &convArgs, outputString, 1024);
    }
}

void __cdecl IsItemUnlocked(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-14h] BYREF
    int milestoneType; // [esp+8h] [ebp-Ch] BYREF
    challengeInfo_t *challenge; // [esp+Ch] [ebp-8h] BYREF
    int challengeNum; // [esp+10h] [ebp-4h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    challengeNum = 0;
    milestoneType = 0;
    if ( GetCurrentIndex(
                 localClientNum,
                 item,
                 dataStack,
                 &result,
                 &challengeNum,
                 (operandInternalDataUnion *)&milestoneType,
                 0) )
    {
        if ( LiveStats_GetChallengeInfo(&challenge, challengeNum, milestoneType) && (challenge->flags & 2) == 0 )
            result.internals.intVal = 1;
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl IsChallengeItemPurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-14h] BYREF
    int milestoneType; // [esp+8h] [ebp-Ch] BYREF
    challengeInfo_t *challenge; // [esp+Ch] [ebp-8h] BYREF
    int challengeNum; // [esp+10h] [ebp-4h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    challengeNum = 0;
    milestoneType = 0;
    if ( GetCurrentIndex(
                 localClientNum,
                 item,
                 dataStack,
                 &result,
                 &challengeNum,
                 (operandInternalDataUnion *)&milestoneType,
                 0) )
    {
        if ( LiveStats_GetChallengeInfo(&challenge, challengeNum, milestoneType) )
        {
            if ( !challenge
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                            10064,
                            0,
                            "%s",
                            "challenge") )
            {
                __debugbreak();
            }
            if ( milestoneType == 7 || milestoneType == 5 )
            {
                if ( challenge )
                {
                    if ( !challenge->flags )
                        result.internals.intVal = 1;
                }
            }
        }
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl IsWeaponItemPurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-14h] BYREF
    int milestoneType; // [esp+8h] [ebp-Ch] BYREF
    challengeInfo_t *challenge; // [esp+Ch] [ebp-8h] BYREF
    int challengeNum; // [esp+10h] [ebp-4h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    challengeNum = 0;
    milestoneType = 0;
    if ( GetCurrentIndexOfHighlightedFeeder(localClientNum, item, dataStack, &result, &challengeNum, &milestoneType, 0)
        && LiveStats_GetChallengeInfo(&challenge, challengeNum, milestoneType) )
    {
        if ( milestoneType == 1 )
        {
            if ( !challenge->flags )
                result.internals.intVal = 1;
        }
        else if ( milestoneType == 4 )
        {
            result.internals.intVal = 1;
        }
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsWeaponItemClassified(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-14h] BYREF
    int milestoneType; // [esp+8h] [ebp-Ch] BYREF
    challengeInfo_t *challenge; // [esp+Ch] [ebp-8h] BYREF
    int challengeNum; // [esp+10h] [ebp-4h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    challengeNum = 0;
    milestoneType = 0;
    if ( GetCurrentIndexOfHighlightedFeeder(localClientNum, item, dataStack, &result, &challengeNum, &milestoneType, 0)
        && LiveStats_GetChallengeInfo(&challenge, challengeNum, milestoneType) )
    {
        if ( milestoneType == 1 )
        {
            if ( (challenge->flags & 4) != 0 )
                result.internals.intVal = 1;
        }
        else if ( milestoneType == 4 )
        {
            result.internals.intVal = 1;
        }
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsWeaponItemUnlocked(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-14h] BYREF
    int milestoneType; // [esp+8h] [ebp-Ch] BYREF
    challengeInfo_t *challenge; // [esp+Ch] [ebp-8h] BYREF
    int challengeNum; // [esp+10h] [ebp-4h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    challengeNum = 0;
    milestoneType = 0;
    if ( GetCurrentIndexOfHighlightedFeeder(localClientNum, item, dataStack, &result, &challengeNum, &milestoneType, 0)
        && LiveStats_GetChallengeInfo(&challenge, challengeNum, milestoneType) )
    {
        if ( milestoneType == 1 )
        {
            if ( (challenge->flags & 2) == 0 )
                result.internals.intVal = 1;
        }
        else if ( milestoneType == 4 )
        {
            result.internals.intVal = 1;
        }
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetProgressString(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-41Ch] BYREF
    int milestoneType; // [esp+8h] [ebp-414h] BYREF
    char outputString[1028]; // [esp+Ch] [ebp-410h] BYREF
    challengeInfo_t *challenge; // [esp+414h] [ebp-8h] BYREF
    int challengeNum; // [esp+418h] [ebp-4h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    challengeNum = 0;
    memset(outputString, 0, 0x400u);
    milestoneType = 0;
    if ( GetCurrentIndex(
                 localClientNum,
                 item,
                 dataStack,
                 &result,
                 &challengeNum,
                 (operandInternalDataUnion *)&milestoneType,
                 0) )
    {
        if ( LiveStats_GetChallengeInfo(&challenge, challengeNum, milestoneType) )
        {
            if ( challenge->targetValue )
            {
                PrepareProgressDescription(milestoneType, outputString, challenge);
                result.internals.intVal = (int)CopyTempString(outputString);
            }
        }
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-23Ch] BYREF
    int milestoneType; // [esp+8h] [ebp-234h] BYREF
    char outputString[256]; // [esp+Ch] [ebp-230h] BYREF
    char challengeType[256]; // [esp+10Ch] [ebp-130h] BYREF
    int challengeNum; // [esp+210h] [ebp-2Ch] BYREF
    ConversionArguments convArgs; // [esp+214h] [ebp-28h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    challengeNum = 0;
    memset(outputString, 0, sizeof(outputString));
    memset(challengeType, 0, sizeof(challengeType));
    memset(&convArgs, 0, sizeof(convArgs));
    milestoneType = 0;
    if ( GetCurrentIndex(
                 localClientNum,
                 item,
                 dataStack,
                 &result,
                 &challengeNum,
                 (operandInternalDataUnion *)&milestoneType,
                 challengeType) )
    {
        CL_GetFrontEndMilestoneLocalizedName(
            outputString,
            256,
            challengeNum,
            (statsMilestoneTypes_t)milestoneType,
            challengeType);
        result.internals.intVal = (int)CopyTempString(outputString);
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetDescription(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-23Ch] BYREF
    int milestoneType; // [esp+8h] [ebp-234h] BYREF
    char outputString[256]; // [esp+Ch] [ebp-230h] BYREF
    char challengeType[256]; // [esp+10Ch] [ebp-130h] BYREF
    int challengeNum; // [esp+210h] [ebp-2Ch] BYREF
    ConversionArguments convArgs; // [esp+214h] [ebp-28h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    challengeNum = 0;
    memset(outputString, 0, sizeof(outputString));
    memset(challengeType, 0, sizeof(challengeType));
    memset(&convArgs, 0, sizeof(convArgs));
    milestoneType = 0;
    if ( GetCurrentIndex(
                 localClientNum,
                 item,
                 dataStack,
                 &result,
                 &challengeNum,
                 (operandInternalDataUnion *)&milestoneType,
                 challengeType) )
    {
        CL_GetFrontEndMilestoneLocalizedDesc(
            outputString,
            256,
            challengeNum,
            (statsMilestoneTypes_t)milestoneType,
            challengeType);
        result.internals.intVal = (int)CopyTempString(outputString);
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetWeaponName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-80h] BYREF
    int milestoneType; // [esp+8h] [ebp-78h]
    const char *itemName; // [esp+Ch] [ebp-74h]
    const char *stringResult; // [esp+10h] [ebp-70h] BYREF
    challengeInfo_t *challenge; // [esp+14h] [ebp-6Ch] BYREF
    OperandList list; // [esp+18h] [ebp-68h] BYREF
    float currIndex; // [esp+74h] [ebp-Ch] BYREF
    itemDef_s *actualItem = NULL; // [esp+78h] [ebp-8h]
    const char *feederName; // [esp+7Ch] [ebp-4h]

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    itemName = 0;
    milestoneType = 0;
    currIndex = 0.0f;
    GetOperandList(dataStack, &list);
    if ( list.operandCount == 2 )
    {
        feederName = GetSourceString(list.operands[0]);
        milestoneType = GetOperandValueInt(&list.operands[1]).intVal;
        if ( feederName && *feederName )
            actualItem = Menu_GetMatchingItemByNumber(item->parent, 0, (char *)feederName);
        if ( UI_FeederData(localClientNum, actualItem, "selection", (char **)&stringResult, &currIndex) )
        {
            if ( LiveStats_GetChallengeInfo(&challenge, (int)currIndex, milestoneType) )
            {
                itemName = BG_UnlockablesGetItemName(challenge->index);
                result.internals.intVal = (int)CopyTempString(itemName);
            }
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Expression_Error("Expected 2 parameters to GetName()\n");
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetChallengeAttachmentName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-1Ch] BYREF
    const char *itemName; // [esp+8h] [ebp-14h]
    const char *stringResult; // [esp+Ch] [ebp-10h] BYREF
    challengeInfo_t *challenge; // [esp+10h] [ebp-Ch] BYREF
    float currIndex; // [esp+14h] [ebp-8h] BYREF
    itemDef_s *actualItem; // [esp+18h] [ebp-4h]

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    itemName = 0;
    currIndex = 0.0f;
    actualItem = Menu_GetMatchingItemByNumber(item->parent, 0, (char*)"stats_milestones_attachments");
    if ( UI_FeederData(localClientNum, actualItem, "selection", (char **)&stringResult, &currIndex)
        && LiveStats_GetChallengeInfo(&challenge, (int)currIndex, 5) )
    {
        if ( challenge->index >= 0x18u )
            itemName = BG_UnlockablesGetItemName(challenge->index);
        else
            itemName = BG_GetAttachmentDisplayName((eAttachment)challenge->index);
        result.internals.intVal = (int)CopyTempString(itemName);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetChallengeName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-13Ch] BYREF
    int milestoneType; // [esp+8h] [ebp-134h] BYREF
    char outputString[256]; // [esp+Ch] [ebp-130h] BYREF
    int challengeNum; // [esp+110h] [ebp-2Ch] BYREF
    ConversionArguments convArgs; // [esp+114h] [ebp-28h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    challengeNum = 0;
    memset(outputString, 0, sizeof(outputString));
    memset(&convArgs, 0, sizeof(convArgs));
    milestoneType = 0;
    if ( GetCurrentIndexOfHighlightedFeeder(localClientNum, item, dataStack, &result, &challengeNum, &milestoneType, 0) )
    {
        CL_GetFrontEndMilestoneLocalizedName(outputString, 256, challengeNum, (statsMilestoneTypes_t)milestoneType, 0);
        result.internals.intVal = (int)CopyTempString(outputString);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetChallengeDescription(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-114h] BYREF
    int milestoneType; // [esp+8h] [ebp-10Ch] BYREF
    char outputString[256]; // [esp+Ch] [ebp-108h] BYREF
    int challengeNum; // [esp+110h] [ebp-4h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    challengeNum = 0;
    memset(outputString, 0, sizeof(outputString));
    milestoneType = 0;
    if ( GetCurrentIndexOfHighlightedFeeder(localClientNum, item, dataStack, &result, &challengeNum, &milestoneType, 0) )
    {
        CL_GetFrontEndMilestoneLocalizedDesc(outputString, 256, challengeNum, (statsMilestoneTypes_t)milestoneType, 0);
        result.internals.intVal = (int)CopyTempString(outputString);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetLBFilter(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    if ( Dvar_GetInt("lb_filter") )
    {
        if ( Dvar_GetInt("lb_filter") == 1 )
        {
            result.internals.intVal = (int)UI_SafeTranslateString("MPUI_LB_FRIENDS_ONLY_CAPS");
        }
        else if ( Dvar_GetInt("lb_filter") == 2 )
        {
            result.internals.intVal = (int)UI_SafeTranslateString("MPUI_LB_PLAYERS_IN_LOBBY_CAPS");
        }
    }
    else
    {
        result.internals.intVal = (int)UI_SafeTranslateString("MPUI_LB_EVERYONE_CAPS");
    }
    AddOperandToStack(dataStack, &result);
}

const char *lbResetPeriodStrings_4[4] =
{ "MPUI_LB_ALL_TIME", "MPUI_LB_WEEKLY", "MPUI_LB_MONTHLY", NULL };

void __cdecl GetLBTypeByDuration(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int currResetPeriod; // [esp+8h] [ebp-4h]

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    currResetPeriod = Dvar_GetInt("lb_typeByResetPeriod");
    result.internals.intVal = (int)UI_SafeTranslateString(lbResetPeriodStrings_4[currResetPeriod]);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetUnlockedWeaponItemIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    signed int currItem; // [esp+0h] [ebp-28h]
    Operand source; // [esp+4h] [ebp-24h] BYREF
    Operand result; // [esp+Ch] [ebp-1Ch] BYREF
    int controllerIndex; // [esp+14h] [ebp-14h]
    int itemNum; // [esp+18h] [ebp-10h]
    loadoutSlot_t currLoadoutSlot; // [esp+1Ch] [ebp-Ch]
    int itemIndex; // [esp+20h] [ebp-8h]
    int totalItemsUnlocked; // [esp+24h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    itemNum = GetSourceInt(&source).intVal;
    totalItemsUnlocked = LiveStats_GetNumRecentlyUnlockedItems(controllerIndex);
    itemIndex = 0;
    for ( currItem = 0; currItem < totalItemsUnlocked; ++currItem )
    {
        itemIndex = LiveStats_GetRecentlyUnlockedItemIndex(controllerIndex, currItem);
        currLoadoutSlot = (loadoutSlot_t)BG_UnlockablesGetItemLoadoutSlot(itemIndex);
        if ( currLoadoutSlot == LOADOUTSLOT_FIRST
            || currLoadoutSlot == LOADOUTSLOT_SECONDARY_WEAPON
            || BG_UnlockablesGetItemGroupEnum(itemIndex) == 14 && BG_UnlockablesIsProItem(itemIndex) )
        {
            if ( !itemNum )
            {
                result.internals.intVal = itemIndex;
                AddOperandToStack(dataStack, &result);
                return;
            }
            --itemNum;
        }
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetUnlockedFeatureItemIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    signed int currItem; // [esp+0h] [ebp-28h]
    Operand source; // [esp+4h] [ebp-24h] BYREF
    itemGroup_t currItemGroup; // [esp+Ch] [ebp-1Ch]
    Operand result; // [esp+10h] [ebp-18h] BYREF
    int controllerIndex; // [esp+18h] [ebp-10h]
    int itemNum; // [esp+1Ch] [ebp-Ch]
    int itemIndex; // [esp+20h] [ebp-8h]
    int totalItemsUnlocked; // [esp+24h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    itemNum = GetSourceInt(&source).intVal;
    totalItemsUnlocked = LiveStats_GetNumRecentlyUnlockedItems(controllerIndex);
    itemIndex = 0;
    for ( currItem = 0; currItem < totalItemsUnlocked; ++currItem )
    {
        itemIndex = LiveStats_GetRecentlyUnlockedItemIndex(controllerIndex, currItem);
        currItemGroup = (itemGroup_t)BG_UnlockablesGetItemGroupEnum(itemIndex);
        if ( currItemGroup == ITEMGROUP_FEATURE )
        {
            if ( !itemNum )
            {
                result.internals.intVal = itemIndex;
                AddOperandToStack(dataStack, &result);
                return;
            }
            --itemNum;
        }
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetTotalMatchesPlayed(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *PlayerStatStringByKey; // eax
    const char *v4; // eax
    int gameType; // [esp+0h] [ebp-20h]
    int losses; // [esp+4h] [ebp-1Ch] BYREF
    Operand result; // [esp+8h] [ebp-18h] BYREF
    int controllerIndex; // [esp+10h] [ebp-10h]
    int wins; // [esp+14h] [ebp-Ch] BYREF
    bool forFriend; // [esp+1Bh] [ebp-5h]
    int numMatches; // [esp+1Ch] [ebp-4h]

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    numMatches = 0;
    wins = 0;
    losses = 0;
    forFriend = Dvar_GetBool("ui_showFriendsCombatRecord");
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( forFriend )
    {
        for ( gameType = 0; gameType < 16; ++gameType )
        {
            PlayerStatStringByKey = LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_WINS);
            LiveStats_GetIntOtherPlayerStatByGameMode(controllerIndex, &wins, lbTypeEnum_4[gameType], PlayerStatStringByKey);
            v4 = LiveStats_GetPlayerStatStringByKey(MP_PLAYERSTATSKEY_LOSSES);
            LiveStats_GetIntOtherPlayerStatByGameMode(controllerIndex, &losses, lbTypeEnum_4[gameType], v4);
            numMatches += losses + wins;
        }
        result.internals.intVal = numMatches;
    }
    else
    {
        result.internals.intVal = LiveStats_GetTotalMatchesPlayed(controllerIndex);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl NeedToPerformCommunitySearch(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 1;
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsFileshareDataSummaryValid(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-1Ch] BYREF
    Operand result; // [esp+8h] [ebp-14h] BYREF
    int controllerIndex; // [esp+10h] [ebp-Ch]
    float isValid; // [esp+14h] [ebp-8h] BYREF
    int index; // [esp+18h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    index = GetSourceInt(&source).intVal;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( Live_FileShare_GetItemInfo(
                 controllerIndex,
                 FILESHARE_KEY_HASSUMMARY,
                 index,
                 FILESHARE_INFOLOCATION_SEARCHRESULTS,
                 0,
                 &isValid)
        && isValid > 0.0 )
    {
        result.internals.intVal = 1;
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetFileshareRecentGamesCount(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int v3; // eax
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int controllerIndex; // [esp+8h] [ebp-4h]

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    v3 = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( TaskManger2_TaskGetInProgressForControllerByName("LiveFileShareSearch", v3) )
        result.internals.intVal = 0;
    else
        result.internals.intVal = Live_FileShare_GetSearchResultsCount();
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetFileshareGameType(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-1Ch] BYREF
    Operand result; // [esp+8h] [ebp-14h] BYREF
    int controllerIndex; // [esp+10h] [ebp-Ch]
    int index; // [esp+14h] [ebp-8h]
    const char *gameType; // [esp+18h] [ebp-4h] BYREF

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    index = GetSourceInt(&source).intVal;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( Live_FileShare_GetItemInfo(
                 controllerIndex,
                 FILESHARE_KEY_GAMETYPE,
                 index,
                 FILESHARE_INFOLOCATION_SEARCHRESULTS,
                 &gameType,
                 0) )
    {
        result.internals.intVal = (int)CopyTempString(gameType);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetFileshareGameTypeName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-1Ch] BYREF
    Operand result; // [esp+8h] [ebp-14h] BYREF
    int controllerIndex; // [esp+10h] [ebp-Ch]
    const char *gameTypeName; // [esp+14h] [ebp-8h] BYREF
    int index; // [esp+18h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    index = GetSourceInt(&source).intVal;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( Live_FileShare_GetItemInfo(
                 controllerIndex,
                 FILESHARE_KEY_GAMETYPENAME,
                 index,
                 FILESHARE_INFOLOCATION_SEARCHRESULTS,
                 &gameTypeName,
                 0) )
    {
        result.internals.intVal = (int)CopyTempString(gameTypeName);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetFileshareGameMap(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-1Ch] BYREF
    Operand result; // [esp+8h] [ebp-14h] BYREF
    int controllerIndex; // [esp+10h] [ebp-Ch]
    const char *map; // [esp+14h] [ebp-8h] BYREF
    int index; // [esp+18h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    index = GetSourceInt(&source).intVal;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( Live_FileShare_GetItemInfo(
                 controllerIndex,
                 FILESHARE_KEY_MAP,
                 index,
                 FILESHARE_INFOLOCATION_SEARCHRESULTS,
                 &map,
                 0) )
    {
        result.internals.intVal = (int)CopyTempString(map);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetFileshareGameMapName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-1Ch] BYREF
    Operand result; // [esp+8h] [ebp-14h] BYREF
    int controllerIndex; // [esp+10h] [ebp-Ch]
    const char *mapName; // [esp+14h] [ebp-8h] BYREF
    int index; // [esp+18h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    index = GetSourceInt(&source).intVal;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( Live_FileShare_GetItemInfo(
                 controllerIndex,
                 FILESHARE_KEY_MAPNAME,
                 index,
                 FILESHARE_INFOLOCATION_SEARCHRESULTS,
                 &mapName,
                 0) )
    {
        result.internals.intVal = (int)CopyTempString(mapName);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetFileshareGameDate(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-1Ch] BYREF
    Operand result; // [esp+8h] [ebp-14h] BYREF
    int controllerIndex; // [esp+10h] [ebp-Ch]
    int index; // [esp+14h] [ebp-8h]
    const char *gameDate; // [esp+18h] [ebp-4h] BYREF

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    index = GetSourceInt(&source).intVal;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( Live_FileShare_GetItemInfo(
                 controllerIndex,
                 FILESHARE_KEY_DATETIME,
                 index,
                 FILESHARE_INFOLOCATION_SEARCHRESULTS,
                 &gameDate,
                 0) )
    {
        result.internals.intVal = (int)CopyTempString(gameDate);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetFileshareFileName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-1Ch] BYREF
    const char *fileName; // [esp+8h] [ebp-14h] BYREF
    Operand result; // [esp+Ch] [ebp-10h] BYREF
    int controllerIndex; // [esp+14h] [ebp-8h]
    int index; // [esp+18h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    index = GetSourceInt(&source).intVal;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( Live_FileShare_GetItemInfo(
                 controllerIndex,
                 FILESHARE_KEY_FILENAME,
                 index,
                 FILESHARE_INFOLOCATION_SEARCHRESULTS,
                 &fileName,
                 0) )
    {
        result.internals.intVal = (int)CopyTempString(fileName);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetCacFactionNameWithButtons(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *v3; // eax
    Operand source; // [esp+0h] [ebp-448h] BYREF
    Operand result; // [esp+8h] [ebp-440h] BYREF
    char outputString[1028]; // [esp+10h] [ebp-438h] BYREF
    const char *factionName; // [esp+418h] [ebp-30h]
    char binding[4]; // [esp+41Ch] [ebp-2Ch] BYREF
    ConversionArguments convArgs; // [esp+420h] [ebp-28h] BYREF

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    factionName = GetSourceString(source);
    binding[0] = 5;
    binding[1] = 0;
    binding[2] = 6;
    binding[3] = 0;
    memset(&convArgs.args[1], 0, 32);
    convArgs.argCount = 3;
    convArgs.args[0] = binding;
    convArgs.args[1] = UI_SafeTranslateString(factionName);
    convArgs.args[2] = &binding[2];
    v3 = UI_SafeTranslateString("MPUI_CAC_FACTION_NAME_WITH_BUTTONS");
    UI_ReplaceConversions(v3, &convArgs, outputString, 1024);
    result.internals.intVal = (int)CopyTempString(outputString);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetFileshareFileSize(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-1Ch] BYREF
    Operand result; // [esp+8h] [ebp-14h] BYREF
    int controllerIndex; // [esp+10h] [ebp-Ch]
    const char *fileSize; // [esp+14h] [ebp-8h] BYREF
    int index; // [esp+18h] [ebp-4h]

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    index = GetSourceInt(&source).intVal;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( Live_FileShare_GetItemInfo(
                 controllerIndex,
                 FILESHARE_KEY_FILESIZE,
                 index,
                 FILESHARE_INFOLOCATION_SEARCHRESULTS,
                 &fileSize,
                 0) )
    {
        result.internals.intVal = (int)CopyTempString(fileSize);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetFileshareFileId(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand source; // [esp+0h] [ebp-1Ch] BYREF
    Operand result; // [esp+8h] [ebp-14h] BYREF
    int controllerIndex; // [esp+10h] [ebp-Ch]
    int index; // [esp+14h] [ebp-8h]
    const char *fileId; // [esp+18h] [ebp-4h] BYREF

    GetOperand(dataStack, &source);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    index = GetSourceInt(&source).intVal;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( Live_FileShare_GetItemInfo(
                 controllerIndex,
                 FILESHARE_KEY_FILEID,
                 index,
                 FILESHARE_INFOLOCATION_SEARCHRESULTS,
                 &fileId,
                 0) )
    {
        result.internals.intVal = (int)CopyTempString(fileId);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetMySlotInfo(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-78h] BYREF
    const char *field; // [esp+8h] [ebp-70h]
    int controllerIndex; // [esp+Ch] [ebp-6Ch]
    const char *stringResult; // [esp+10h] [ebp-68h] BYREF
    float floatResult; // [esp+14h] [ebp-64h] BYREF
    OperandList list; // [esp+18h] [ebp-60h] BYREF
    int slotNum; // [esp+74h] [ebp-4h]

    GetOperandList(dataStack, &list);
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    slotNum = GetOperandValueInt(list.operands).intVal;
    field = list.operands[1].internals.string;
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( Live_FileShare_GetMySlotInfo(controllerIndex, slotNum, field, &stringResult, &floatResult) )
        result.internals.intVal = (int)CopyTempString(stringResult);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetImageWidth(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Material *v3; // eax
    Operand source; // [esp+0h] [ebp-14h] BYREF
    const char *imageName; // [esp+8h] [ebp-Ch]
    Operand result; // [esp+Ch] [ebp-8h] BYREF

    GetOperand(dataStack, &source);
    imageName = GetSourceString(source);
    result.dataType = VAL_FLOAT;
    v3 = Material_RegisterHandle((char *)imageName, 3);
    result.internals.floatVal = CG_GetPicWidth(v3);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetTextWidth(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+4h] [ebp-74h] BYREF
    Font_s *font; // [esp+Ch] [ebp-6Ch]
    int fontInt; // [esp+10h] [ebp-68h]
    const ScreenPlacement *scrPlace; // [esp+14h] [ebp-64h]
    OperandList list; // [esp+18h] [ebp-60h] BYREF
    float scale; // [esp+70h] [ebp-8h]
    const char *text; // [esp+74h] [ebp-4h]

    GetOperandList(dataStack, &list);
    if ( list.operandCount == 3 )
    {
        result.dataType = VAL_INT;
        text = GetSourceString(list.operands[0]);
        fontInt = GetOperandValueInt(&list.operands[1]).intVal;
        scale = GetSourceFloat(&list.operands[2]);
        scrPlace = &scrPlaceView[localClientNum];
        font = UI_GetFontHandle(scrPlace, fontInt, scale);
        result.internals.intVal = UI_TextWidth(text, 0, font, scale);
    }
    else
    {
        Expression_Error("Expected 3 parameters to GetTextWidth()\n");
        result.dataType = VAL_INT;
        result.internals.intVal = 0;
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetTextHeight(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int lineCount; // [esp+4h] [ebp-80h]
    Operand result; // [esp+8h] [ebp-7Ch] BYREF
    Font_s *font; // [esp+10h] [ebp-74h]
    int fontInt; // [esp+14h] [ebp-70h]
    const ScreenPlacement *scrPlace; // [esp+18h] [ebp-6Ch]
    OperandList list; // [esp+1Ch] [ebp-68h] BYREF
    float scale; // [esp+78h] [ebp-Ch]
    const char *text; // [esp+7Ch] [ebp-8h]
    const char *p; // [esp+80h] [ebp-4h]

    GetOperandList(dataStack, &list);
    if ( list.operandCount == 3 )
    {
        result.dataType = VAL_INT;
        text = GetSourceString(list.operands[0]);
        fontInt = GetOperandValueInt(&list.operands[1]).intVal;
        scale = GetSourceFloat(&list.operands[2]);
        p = text;
        lineCount = 1;
        scrPlace = &scrPlaceView[localClientNum];
        font = UI_GetFontHandle(scrPlace, fontInt, scale);
        while ( p && *p )
        {
            if ( *p == 10 )
                ++lineCount;
            ++p;
        }
        result.internals.intVal = lineCount * UI_TextHeight(font, scale);
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Expression_Error("Expected 3 parameters to GetTextHeight()\n");
        result.dataType = VAL_INT;
        result.internals.intVal = 0;
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl HasFocus(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *SourceString; // eax
    int UIContextIndex; // eax
    itemDef_s *v5; // [esp-4h] [ebp-18h]
    Operand result; // [esp+0h] [ebp-14h] BYREF
    Operand data; // [esp+8h] [ebp-Ch] BYREF
    itemDef_s *actualItem; // [esp+10h] [ebp-4h]

    GetOperand(dataStack, &data);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    SourceString = GetSourceString(data);
    actualItem = Menu_GetMatchingItemByNumber(item->parent, 0, SourceString);
    if ( actualItem )
    {
        v5 = actualItem;
        UIContextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
        result.internals.intVal = Window_HasFocus(UIContextIndex, &v5->window);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl MenuHasFocus(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int UIContextIndex; // eax
    menuDef_t *parent; // [esp-4h] [ebp-Ch]
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( !item
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    11027,
                    0,
                    "%s",
                    "item") )
    {
        __debugbreak();
    }
    if ( !item->parent
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    11028,
                    0,
                    "%s",
                    "item->parent") )
    {
        __debugbreak();
    }
    parent = item->parent;
    UIContextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
    if ( Window_HasFocus(UIContextIndex, &parent->window) )
        result.internals.intVal = 1;
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetFeederData(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *SourceString; // eax
    char *v4; // eax
    char *v5; // eax
    uiInfo_s *uiInfo; // [esp+0h] [ebp-7Ch]
    menuDef_t *menu; // [esp+4h] [ebp-78h]
    Operand result; // [esp+8h] [ebp-74h] BYREF
    const char *field; // [esp+10h] [ebp-6Ch]
    const char *stringResult; // [esp+14h] [ebp-68h] BYREF
    float floatResult; // [esp+18h] [ebp-64h] BYREF
    OperandList list; // [esp+1Ch] [ebp-60h] BYREF
    itemDef_s *actualItem = NULL; // [esp+78h] [ebp-4h]

    field = "";
    GetOperandList(dataStack, &list);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    if ( list.operandCount >= 1 )
    {
        if ( list.operandCount == 1 )
        {
            actualItem = item;
            if ( list.operands[0].dataType == VAL_STRING )
                field = list.operands[0].internals.string;
        }
        else if ( list.operandCount == 2 )
        {
            SourceString = GetSourceString(list.operands[0]);
            actualItem = Menu_GetMatchingItemByNumber(item->parent, 0, SourceString);
            if ( list.operands[1].dataType == VAL_STRING )
                field = list.operands[1].internals.string;
        }
        else
        {
            uiInfo = UI_GetInfo(localClientNum);
            v4 = GetSourceString(list.operands[0]);
            menu = Menus_FindByName(&uiInfo->uiDC, v4);
            if ( menu )
            {
                if ( list.operands[1].dataType == VAL_STRING )
                {
                    Operand tmp;
                    tmp.dataType = VAL_STRING;
                    tmp.internals.string = (char*)list.operands[1].internals.intVal;
                    //v5 = GetSourceString((Operand)__PAIR64__(list.operands[1].internals.intVal, 2));
                    v5 = GetSourceString(tmp);
                    actualItem = Menu_GetMatchingItemByNumber(menu, 0, v5);
                    if ( list.operands[2].dataType == VAL_STRING )
                        field = list.operands[2].internals.string;
                }
            }
        }
        if ( actualItem )
        {
            floatResult = 0.0f;
            if ( UI_FeederData(localClientNum, actualItem, field, (char **)&stringResult, &floatResult) )
            {
                if ( stringResult )
                {
                    result.dataType = VAL_STRING;
                    result.internals.intVal = (int)CopyTempString(stringResult);
                }
                else
                {
                    result.dataType = VAL_FLOAT;
                    result.internals.floatVal = floatResult;
                }
            }
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Com_PrintError(13, "UI Expression Error: Expected at least 1 parameter to getFeederData()\n");
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetRank(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int Rank; // eax
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int controllerIndex; // [esp+8h] [ebp-4h]

    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.dataType = VAL_STRING;
    Rank = LiveStats_GetRank(controllerIndex);
    result.internals.intVal = (int)CL_GetRankData(Rank, MP_RANKTABLE_FULLRANK);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetPrestige(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-Ch] BYREF
    int controllerIndex; // [esp+8h] [ebp-4h]

    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.dataType = VAL_INT;
    result.internals.intVal = LiveStats_GetPrestige(controllerIndex);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetComposite(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-10h] BYREF
    Operand data; // [esp+8h] [ebp-8h] BYREF

    GetOperand(dataStack, &data);
    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsCompositeValid(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-10h] BYREF
    Operand data; // [esp+8h] [ebp-8h] BYREF

    GetOperand(dataStack, &data);
    result.dataType = VAL_INT;
    AddOperandToStack(dataStack, &result);
}

void __cdecl TableLookup(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *SourceString; // eax
    operandInternalDataUnion v4; // eax
    char *v5; // eax
    operandInternalDataUnion v6; // [esp-10h] [ebp-74h]
    char *v7; // [esp-Ch] [ebp-70h]
    char *v8; // [esp-8h] [ebp-6Ch]
    operandInternalDataUnion v9; // [esp-8h] [ebp-6Ch]
    int intVal; // [esp-4h] [ebp-68h]
    const char *string; // [esp-4h] [ebp-68h]
    const StringTable *tablePtr; // [esp+0h] [ebp-64h] BYREF
    OperandList list; // [esp+4h] [ebp-60h] BYREF
    Operand operandResult; // [esp+5Ch] [ebp-8h] BYREF

    GetOperandList(dataStack, &list);
    if ( useFastFile->current.enabled )
    {
        if ( list.operandCount == 4 )
        {
            SourceString = GetSourceString(list.operands[0]);
            StringTable_GetAsset(SourceString, (XAssetHeader *)&tablePtr);
            operandResult.dataType = VAL_STRING;
            intVal = GetSourceInt(&list.operands[3]).intVal;
            v8 = GetSourceString(list.operands[2]);
            v4.intVal = GetSourceInt(&list.operands[1]).intVal;
            operandResult.internals.intVal = (int)StringTable_Lookup(tablePtr, v4.intVal, v8, intVal);
            if ( uiscript_debug )
            {
                if ( uiscript_debug->current.integer )
                {
                    string = operandResult.internals.string;
                    v9.intVal = GetSourceInt(&list.operands[3]).intVal;
                    v7 = GetSourceString(list.operands[2]);
                    v6.intVal = GetSourceInt(&list.operands[1]).intVal;
                    v5 = GetSourceString(list.operands[0]);
                    Expression_TraceInternal("tablelookup( %s, %i, %s, %i ) == %s\n", v5, v6.intVal, v7, v9.intVal, string);
                }
            }
            AddOperandToStack(dataStack, &operandResult);
        }
        else
        {
            Expression_Error(
                "UI Expression Error: Expected 4 params to function StringTableLookup, found %i\n",
                list.operandCount);
            operandResult.dataType = VAL_STRING;
            operandResult.internals.intVal = (int)"";
            AddOperandToStack(dataStack, &operandResult);
        }
    }
    else
    {
        operandResult.dataType = VAL_STRING;
        operandResult.internals.intVal = (int)"";
        AddOperandToStack(dataStack, &operandResult);
    }
}

void __cdecl StatsTableLookup(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    operandInternalDataUnion v4; // eax
    char *v5; // [esp-Ch] [ebp-70h]
    char *SourceString; // [esp-8h] [ebp-6Ch]
    operandInternalDataUnion v7; // [esp-8h] [ebp-6Ch]
    int intVal; // [esp-4h] [ebp-68h]
    const char *string; // [esp-4h] [ebp-68h]
    const StringTable *tablePtr; // [esp+0h] [ebp-64h] BYREF
    OperandList list; // [esp+4h] [ebp-60h] BYREF
    Operand operandResult; // [esp+5Ch] [ebp-8h] BYREF

    GetOperandList(dataStack, &list);
    if ( useFastFile->current.enabled )
    {
        if ( list.operandCount == 3 )
        {
            StringTable_GetAsset("mp/statstable.csv", (XAssetHeader *)&tablePtr);
            operandResult.dataType = VAL_STRING;
            intVal = GetSourceInt(&list.operands[2]).intVal;
            SourceString = GetSourceString(list.operands[1]);
            v3.intVal = GetSourceInt(list.operands).intVal;
            operandResult.internals.intVal = (int)StringTable_Lookup(tablePtr, v3.intVal, SourceString, intVal);
            if ( uiscript_debug )
            {
                if ( uiscript_debug->current.integer )
                {
                    string = operandResult.internals.string;
                    v7.intVal = GetSourceInt(&list.operands[2]).intVal;
                    v5 = GetSourceString(list.operands[1]);
                    v4.intVal = GetSourceInt(list.operands).intVal;
                    Expression_TraceInternal("statstablelookup( %i, %s, %i ) == %s\n", v4.intVal, v5, v7.intVal, string);
                }
            }
            AddOperandToStack(dataStack, &operandResult);
        }
        else
        {
            Expression_Error(
                "UI Expression Error: Expected 3 params to function StringTableLookup, found %i\n",
                list.operandCount);
            operandResult.dataType = VAL_STRING;
            operandResult.internals.intVal = (int)"";
            AddOperandToStack(dataStack, &operandResult);
        }
    }
    else
    {
        operandResult.dataType = VAL_STRING;
        operandResult.internals.intVal = (int)"";
        AddOperandToStack(dataStack, &operandResult);
    }
}

void __cdecl GetClassBonus(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *SourceString; // eax
    char *v4; // eax
    operandInternalDataUnion v5; // [esp-4h] [ebp-68h]
    const StringTable *tablePtr; // [esp+0h] [ebp-64h] BYREF
    OperandList list; // [esp+4h] [ebp-60h] BYREF
    Operand operandResult; // [esp+5Ch] [ebp-8h] BYREF

    GetOperandList(dataStack, &list);
    operandResult.dataType = VAL_INT;
    operandResult.internals.intVal = 0;
    if ( useFastFile->current.enabled )
    {
        if ( list.operandCount == 1 )
        {
            StringTable_GetAsset("mp/classbonustable.csv", (XAssetHeader *)&tablePtr);
            SourceString = GetSourceString(list.operands[0]);
            operandResult.internals.intVal = GetClassBonusInternal(localClientNum, tablePtr, SourceString);
            if ( uiscript_debug )
            {
                if ( uiscript_debug->current.integer )
                {
                    v5.intVal = (int)operandResult.internals;
                    v4 = GetSourceString(list.operands[0]);
                    Expression_TraceInternal("getclassbonus( %s ) == %d\n", v4, v5.intVal);
                }
            }
            AddOperandToStack(dataStack, &operandResult);
        }
        else
        {
            Expression_Error(
                "UI Expression Error: Expected 1 params to function GetClassBonus, found %i\n",
                list.operandCount);
            AddOperandToStack(dataStack, &operandResult);
        }
    }
    else
    {
        AddOperandToStack(dataStack, &operandResult);
    }
}

int __cdecl GetClassBonusInternal(int localClientNum, const StringTable *table, const char *className)
{
    int defaultRow; // [esp+0h] [ebp-9Ch]
    int slot; // [esp+8h] [ebp-94h]
    int statRow; // [esp+Ch] [ebp-90h]
    int numMatches; // [esp+14h] [ebp-88h]
    int column; // [esp+18h] [ebp-84h]
    int rowMatches[32]; // [esp+1Ch] [ebp-80h] BYREF

    numMatches = 0;
    statRow = StringTable_LookupRowNumForValue(table, 0, "stats");
    if ( statRow == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    11370,
                    0,
                    "%s",
                    "statRow != STRINGTABLE_ROW_NOT_FOUND") )
    {
        __debugbreak();
    }
    defaultRow = StringTable_LookupRowNumForValue(table, 0, "0");
    if ( defaultRow == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    11373,
                    0,
                    "%s",
                    "defaultRow != STRINGTABLE_ROW_NOT_FOUND") )
    {
        __debugbreak();
    }
    for ( column = 1; column < table->columnCount; ++column )
    {
        slot = column + table->columnCount * statRow;
        if ( table->values[slot].hash )
        {
            numMatches = FindClassBonusMatches(
                                         localClientNum,
                                         table,
                                         rowMatches,
                                         numMatches,
                                         table->values[slot].string,
                                         className,
                                         column,
                                         defaultRow);
            if ( !numMatches )
                return 0;
        }
    }
    if ( numMatches != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    11392,
                    0,
                    "%s",
                    "numMatches == 1") )
    {
        __debugbreak();
    }
    return atoi(table->values[table->columnCount * rowMatches[0]].string);
}

int __cdecl FindClassBonusMatches(
                int localClientNum,
                const StringTable *table,
                int *rowMatches,
                int numRows,
                const char *statName,
                const char *className,
                int column,
                int ignoreRow)
{
    int ControllerIndex; // eax
    int row; // [esp+8h] [ebp-Ch]
    int rowa; // [esp+8h] [ebp-Ch]
    int numMatches; // [esp+Ch] [ebp-8h]
    const char *playerData; // [esp+10h] [ebp-4h]

    numMatches = 0;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    playerData = ClassBonusStatQuery(ControllerIndex, statName, className);
    if ( numRows )
    {
        for ( rowa = 0; rowa < numRows; ++rowa )
        {
            if ( IsClassBonusMatch(table->values[column + table->columnCount * rowMatches[rowa]].string, playerData) )
                rowMatches[numMatches++] = rowMatches[rowa];
        }
    }
    else
    {
        for ( row = 0; row <= table->rowCount - 1; ++row )
        {
            if ( row != ignoreRow && IsClassBonusMatch(table->values[column + table->columnCount * row].string, playerData) )
                rowMatches[numMatches++] = row;
        }
    }
    return numMatches;
}

bool __cdecl IsClassBonusMatch(const char *tableData, const char *playerData)
{
    char slotData[1024]; // [esp+0h] [ebp-408h] BYREF
    char *token; // [esp+404h] [ebp-4h]

    if ( !I_stricmp(tableData, "") )
        return 1;
    if ( !I_stricmp(tableData, "<none>") )
        return I_stristr(playerData, "_null") != 0;
    I_strncpyz(slotData, tableData, 1024);
    for ( token = strtok(slotData, "|"); token; token = strtok(0, "|") )
    {
        if ( !I_stricmp(token, playerData) )
            return 1;
    }
    return 0;
}

const char *__cdecl ClassBonusStatQuery(int controllerIndex, const char *statName, const char *className)
{
    unsigned int DIntStat; // eax
    unsigned int v5; // eax
    ddlState_t searchState; // [esp+14h] [ebp-14h] BYREF
    bool group; // [esp+27h] [ebp-1h]

    searchState = *LiveStats_GetRootDDLState();
    if ( Dvar_GetInt(ui_useCustomClassInfo) )
        return "";
    group = 0;
    if ( I_stristr(statName, "<group>") )
    {
        statName += strlen("<group>");
        group = 1;
    }
    if ( DDL_MoveTo(&searchState, &searchState, 3, "cacLoadouts", className, statName) )
    {
        if ( group )
        {
            DIntStat = LiveStats_GetDIntStat(controllerIndex, &searchState);
            return BG_UnlockablesGetItemGroup(DIntStat);
        }
        else
        {
            v5 = LiveStats_GetDIntStat(controllerIndex, &searchState);
            return BG_UnlockablesGetItemRef(v5);
        }
    }
    else
    {
        DDL_PrintError("DDL:Error getting player stat value for %s.\n", statName);
        return "";
    }
}

void __cdecl GetCustomClassLoadoutItem(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *itemName; // [esp+4h] [ebp-6Ch]
    OperandList list; // [esp+8h] [ebp-68h] BYREF
    Operand operandResult; // [esp+64h] [ebp-Ch] BYREF
    int slotNum; // [esp+6Ch] [ebp-4h]

    GetOperandList(dataStack, &list);
    if ( list.operandCount == 2 )
    {
        slotNum = GetSourceInt(list.operands).intVal;
        itemName = GetSourceString(list.operands[1]);
        operandResult.dataType = VAL_INT;
        operandResult.internals.intVal = GetCustomClassLoadoutItemForSlot(0, slotNum, itemName);
    }
    else
    {
        Expression_Error("UI Expression Error: Expected at 2 parameters to GetCustomClassLoadoutItem()\n");
        operandResult.dataType = VAL_INT;
        operandResult.internals.intVal = 0;
    }
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl GetCustomClassModifier(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *itemName; // [esp+4h] [ebp-6Ch]
    OperandList list; // [esp+8h] [ebp-68h] BYREF
    Operand operandResult; // [esp+64h] [ebp-Ch] BYREF
    int slotNum; // [esp+6Ch] [ebp-4h]

    GetOperandList(dataStack, &list);
    operandResult.dataType = VAL_INT;
    operandResult.internals.intVal = 0;
    if ( list.operandCount == 2 )
    {
        slotNum = GetSourceInt(list.operands).intVal;
        itemName = GetSourceString(list.operands[1]);
        operandResult.dataType = VAL_INT;
        operandResult.internals.intVal = GetCustomClassModifierForClass(0, slotNum, itemName);
    }
    else
    {
        Expression_Error("UI Expression Error: Expected at 2 parameters to GetCustomClassModifier()\n");
    }
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl GetCustomClassName(int localClienTNum, itemDef_s *item, OperandStack *dataStack)
{
    char *v3; // eax
    int classNum; // [esp+0h] [ebp-64h]
    OperandList list; // [esp+4h] [ebp-60h] BYREF
    Operand operandResult; // [esp+5Ch] [ebp-8h] BYREF

    GetOperandList(dataStack, &list);
    operandResult.dataType = VAL_STRING;
    operandResult.internals.intVal = (int)"";
    if ( list.operandCount == 1 )
    {
        classNum = GetSourceInt(list.operands).intVal;
        if ( classNum <= 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                        11510,
                        0,
                        "%s",
                        "classNum > 0") )
        {
            __debugbreak();
        }
        if ( !g_customGameModeClassDescriptions[classNum - 1].name[0] )
        {
            v3 = UI_SafeTranslateString("MPUI_CUSTOM_CLASS_NAME");
            Com_sprintf(g_customGameModeClassDescriptions[classNum - 1].name, 0x10u, "%s %d", v3, classNum);
        }
        operandResult.internals.intVal = (int)g_customGameModeClassDescriptions[classNum - 1].name;
        AddOperandToStack(dataStack, &operandResult);
    }
    else
    {
        Expression_Error("UI Expression Error: Expected 1 parameter to GetCustomClassName()\n");
        AddOperandToStack(dataStack, &operandResult);
    }
}

void __cdecl GetCACItemIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *v3; // eax
    const dvar_s *Var; // eax
    ddlState_t *CacDDLState; // eax
    Operand result; // [esp+14h] [ebp-7Ch] BYREF
    int controllerIndex; // [esp+1Ch] [ebp-74h]
    OperandList list; // [esp+20h] [ebp-70h] BYREF
    ddlState_t searchState; // [esp+7Ch] [ebp-14h] BYREF
    bool isKillstreakSlot; // [esp+8Fh] [ebp-1h]

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    GetOperandList(dataStack, &list);
    if ( list.operandCount == 2 )
    {
        controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        isKillstreakSlot = I_stristr(list.operands[1].internals.string, "killstreak") != 0;
        if ( Dvar_GetInt(custom_killstreak_mode) == 2 && isKillstreakSlot )
        {
            v3 = va(
                         "custom_killstreak_%c",
                         *(char *)(strlen(list.operands[1].internals.string) + list.operands[1].internals.intVal - 1));
            Var = Dvar_FindVar(v3);
            result.internals.intVal = atoi(Var->current.string);
            AddOperandToStack(dataStack, &result);
        }
        else if ( !Dvar_GetInt(ui_useCustomClassInfo) || isKillstreakSlot )
        {
            if ( I_stricmp(list.operands[0].internals.string, "cacLoadouts") )
                CacDDLState = LiveStats_GetCacDDLState();
            else
                CacDDLState = LiveStats_GetRootDDLState();
            searchState = *CacDDLState;
            if ( DDL_MoveTo(
                         &searchState,
                         &searchState,
                         2,
                         list.operands[0].internals.intVal,
                         list.operands[1].internals.intVal) )
            {
                if ( !searchState.member
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                                11592,
                                0,
                                "%s",
                                "searchState.member") )
                {
                    __debugbreak();
                }
                result.internals.intVal = LiveStats_GetDIntStat(controllerIndex, &searchState);
                AddOperandToStack(dataStack, &result);
            }
            else
            {
                DDL_PrintError("DDL: Could not find member name.");
                AddOperandToStack(dataStack, &result);
            }
        }
        else
        {
            result.internals.intVal = GetCustomClassLoadoutItemForClassName(
                                                                    controllerIndex,
                                                                    list.operands[0].internals.string,
                                                                    list.operands[1].internals.string);
            AddOperandToStack(dataStack, &result);
        }
    }
    else
    {
        Expression_Error("Expected 2 parameters to getCACItemIndex()\n");
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetMachineID(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"????????????????????";
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetLocalClientNum(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals = localClientNum;
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsPrimaryLocalClient(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = Com_LocalClient_IsPrimary(localClientNum);
    AddOperandToStack(dataStack, &result);
}

void __cdecl IsFriendFromXuid(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *String; // eax
    int ControllerIndex; // eax
    unsigned __int64 v5; // [esp-Ch] [ebp-DCh]
    Operand result; // [esp+0h] [ebp-D0h] BYREF
    FriendInfo info; // [esp+8h] [ebp-C8h] BYREF
    unsigned __int64 selectedPlayerXuid; // [esp+C8h] [ebp-8h]

    String = Dvar_GetString("selectedPlayerXuid");
    selectedPlayerXuid = I_atoi64(String);
    result.dataType = VAL_INT;
    v5 = selectedPlayerXuid;
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    result.internals.intVal = Friends_GetByID(ControllerIndex, v5, &info) != 0;
    AddOperandToStack(dataStack, &result);
}

void __cdecl CanSwitchToLobby(int locaClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = 1;
    if ( uiscript_debug && uiscript_debug->current.integer )
        Expression_TraceInternal("CanSwitchToLobby() = %d\n", result.internals.intVal);
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetFeederCount(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int UIContextIndex; // eax
    float v4; // [esp+0h] [ebp-28h]
    listBoxDef_s *v5; // [esp+4h] [ebp-24h]
    Operand source; // [esp+8h] [ebp-20h] BYREF
    Operand result; // [esp+10h] [ebp-18h] BYREF
    float feederID; // [esp+18h] [ebp-10h]
    listBoxDef_s *listPtr; // [esp+1Ch] [ebp-Ch]
    itemDef_s *feeder; // [esp+20h] [ebp-8h]
    const char *feederName; // [esp+24h] [ebp-4h]

    result.dataType = VAL_INT;
    GetOperand(dataStack, &source);
    feederName = GetSourceString(source);
    feeder = Menu_FindItemByName(item->parent, feederName);
    if ( feeder )
    {
        listPtr = Item_GetListBoxDef(feeder);
        if ( listPtr )
        {
            feederID = Item_GetListBoxFeederID(feeder);
            v5 = listPtr;
            v4 = feederID;
            UIContextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
            result.internals.intVal = (int)UI_FeederCount(localClientNum, UIContextIndex, v4, v5);
        }
        else
        {
            Expression_Error("UI Expression Error: Item %s is not a listbox in GetFeederCount()\n", feederName);
            result.internals.intVal = 0;
        }
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        Expression_Error("UI Expression Error: Unable to find item %s in GetFeederCount()\n", feederName);
        result.internals.intVal = 0;
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetMapIndexByName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int UIContextIndex; // eax
    int v4; // eax
    const char *v5; // eax
    char *v6; // eax
    itemDef_s *v7; // [esp-4h] [ebp-A4h]
    float v8; // [esp+0h] [ebp-A0h]
    itemGroup_t v9; // [esp+4h] [ebp-9Ch]
    _CustomClassDescription *v10; // [esp+14h] [ebp-8Ch]
    int mapCount; // [esp+18h] [ebp-88h]
    bool useOwnerdraw; // [esp+1Fh] [ebp-81h] BYREF
    Operand result; // [esp+20h] [ebp-80h] BYREF
    float feederID; // [esp+28h] [ebp-78h]
    Material *material; // [esp+2Ch] [ebp-74h] BYREF
    listBoxDef_s *listPtr; // [esp+30h] [ebp-70h]
    float angle; // [esp+34h] [ebp-6Ch] BYREF
    OperandList list; // [esp+38h] [ebp-68h] BYREF
    const char *mapName; // [esp+90h] [ebp-10h]
    itemDef_s *feeder; // [esp+94h] [ebp-Ch]
    int mapIndex; // [esp+98h] [ebp-8h]
    const char *feederName; // [esp+9Ch] [ebp-4h]

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    GetOperandList(dataStack, &list);
    if ( list.operandCount == 2 )
    {
        feederName = GetSourceString(list.operands[0]);
        mapName = GetSourceString(list.operands[1]);
        feeder = Menu_FindItemByName(item->parent, feederName);
        if ( feeder )
        {
            listPtr = Item_GetListBoxDef(feeder);
            if ( listPtr )
            {
                feederID = Item_GetListBoxFeederID(feeder);
                UIContextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
                mapCount = UI_FeederCount(localClientNum, UIContextIndex, feederID, listPtr);
                for ( mapIndex = 0; mapIndex < (int)mapCount; ++mapIndex )
                {
                    v9 = (itemGroup_t)mapIndex;
                    v8 = feederID;
                    v7 = feeder;
                    v4 = Com_LocalClient_GetUIContextIndex(localClientNum);
                    v10 = UI_FeederItemText(localClientNum, v4, v7, v8, v9, 1, &material, &angle, &useOwnerdraw);
                    v5 = va("%s_CAPS", mapName);
                    v6 = UI_SafeTranslateString(v5);
                    if ( !I_strcmp(v6, v10->name) )
                        break;
                }
                if ( mapIndex >= (int)mapCount || mapIndex < 0 )
                    result.internals.intVal = 0;
                else
                    result.internals.intVal = mapIndex;
                AddOperandToStack(dataStack, &result);
            }
            else
            {
                Expression_Error("UI Expression Error: Item %s is not a listbox in GetFeederCount()\n", feederName);
                result.internals.intVal = 0;
                AddOperandToStack(dataStack, &result);
            }
        }
        else
        {
            Expression_Error("UI Expression Error: Unable to find item %s in GetFeederCount()\n", feederName);
            result.internals.intVal = 0;
            AddOperandToStack(dataStack, &result);
        }
    }
    else
    {
        Expression_Error("UI Expression Error: Expected 2 parameters to GetMapIndexByName( <feeder name>, <map name> )\n");
        result.internals.intVal = 0;
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl GetGamemodeIndexByName(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int UIContextIndex; // eax
    int v4; // eax
    char *v5; // eax
    itemDef_s *v6; // [esp-4h] [ebp-A4h]
    float v7; // [esp+0h] [ebp-A0h]
    itemGroup_t v8; // [esp+4h] [ebp-9Ch]
    _CustomClassDescription *v9; // [esp+14h] [ebp-8Ch]
    bool useOwnerdraw; // [esp+1Bh] [ebp-85h] BYREF
    Operand result; // [esp+1Ch] [ebp-84h] BYREF
    float feederID; // [esp+24h] [ebp-7Ch]
    Material *material; // [esp+28h] [ebp-78h] BYREF
    const char *gamemodeName; // [esp+2Ch] [ebp-74h]
    listBoxDef_s *listPtr; // [esp+30h] [ebp-70h]
    float angle; // [esp+34h] [ebp-6Ch] BYREF
    OperandList list; // [esp+38h] [ebp-68h] BYREF
    itemDef_s *feeder; // [esp+90h] [ebp-10h]
    int gamemodeIndex; // [esp+94h] [ebp-Ch]
    int gamemodeCount; // [esp+98h] [ebp-8h]
    const char *feederName; // [esp+9Ch] [ebp-4h]

    result.dataType = VAL_INT;
    result.internals.intVal = 0;
    GetOperandList(dataStack, &list);
    if ( list.operandCount == 2 )
    {
        feederName = GetSourceString(list.operands[0]);
        gamemodeName = GetSourceString(list.operands[1]);
        feeder = Menu_FindItemByName(item->parent, feederName);
        if ( feeder )
        {
            listPtr = Item_GetListBoxDef(feeder);
            if ( listPtr )
            {
                feederID = Item_GetListBoxFeederID(feeder);
                UIContextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
                gamemodeCount = (int)UI_FeederCount(localClientNum, UIContextIndex, feederID, listPtr);
                for ( gamemodeIndex = 0; gamemodeIndex < gamemodeCount; ++gamemodeIndex )
                {
                    v8 = (itemGroup_t)gamemodeIndex;
                    v7 = feederID;
                    v6 = feeder;
                    v4 = Com_LocalClient_GetUIContextIndex(localClientNum);
                    v9 = UI_FeederItemText(localClientNum, v4, v6, v7, v8, 1, &material, &angle, &useOwnerdraw);
                    v5 = UI_SafeTranslateString(gamemodeName);
                    if ( !I_strcmp(v5, v9->name) )
                        break;
                }
                if ( gamemodeIndex >= gamemodeCount || gamemodeIndex < 0 )
                    result.internals.intVal = 0;
                else
                    result.internals.intVal = gamemodeIndex;
                AddOperandToStack(dataStack, &result);
            }
            else
            {
                Expression_Error("UI Expression Error: Item %s is not a listbox in GetFeederCount()\n", feederName);
                result.internals.intVal = 0;
                AddOperandToStack(dataStack, &result);
            }
        }
        else
        {
            Expression_Error("UI Expression Error: Unable to find item %s in GetFeederCount()\n", feederName);
            result.internals.intVal = 0;
            AddOperandToStack(dataStack, &result);
        }
    }
    else
    {
        Expression_Error("UI Expression Error: Expected 2 parameters to GetMapIndexByName( <feeder name>, <map name> )\n");
        result.internals.intVal = 0;
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl ServerSort(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = sharedUiInfo.serverStatus.sortKey;
    AddOperandToStack(dataStack, &result);
}

void __cdecl ServerSortDirection(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = sharedUiInfo.serverStatus.sortDir;
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetServerCounts(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char *v4; // eax
    int numDisplayServers; // [esp-8h] [ebp-18h]
    Operand result; // [esp+4h] [ebp-Ch] BYREF
    int count; // [esp+Ch] [ebp-4h]

    count = 0;
    if (!ui_netSource
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
            11947,
            0,
            "%s",
            "ui_netSource"))
    {
        __debugbreak();
    }
    switch (ui_netSource->current.integer)
    {
    case 0:
        count = cls.numunrankedservers;
        break;
    case 1:
        count = cls.numrankedservers;
        break;
    case 2:
        count = cls.numwagerservers;
        break;
    case 3:
        count = cls.numfavoriteservers;
        break;
    case 4:
        count = cls.numfriendsservers;
        break;
    case 5:
        count = sharedUiInfo.serverStatus.numDisplayServers;
        break;
    default:
        break;
    }
    result.dataType = VAL_STRING;
    numDisplayServers = sharedUiInfo.serverStatus.numDisplayServers;
    v4 = UI_SafeTranslateString("MENU_SERVER_CAPS");
    result.internals.intVal = (int)va("%s %d(%d)", v4, numDisplayServers, count);
    AddOperandToStack(dataStack, &result);
}

void __cdecl RandomIntRange(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    int intVal; // [esp-4h] [ebp-64h]
    Operand result; // [esp+0h] [ebp-60h] BYREF
    OperandList list; // [esp+8h] [ebp-58h] BYREF

    GetOperandList(dataStack, &list);
    if ( list.operandCount == 2 )
    {
        result.dataType = VAL_INT;
        intVal = GetSourceInt(&list.operands[1]).intVal;
        v3.intVal = GetSourceInt(list.operands).intVal;
        result.internals.intVal = irand(v3.intVal, intVal);
    }
    else
    {
        Expression_Error("UI Expression Error: Expected at 2 parameters to RandomIntRange()\n");
        result.dataType = VAL_INT;
        result.internals.intVal = 0;
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl GetSelectedEmblemLayer(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    result.dataType = VAL_INT;
    result.internals.intVal = UI_EmblemGetSelectedLayer();
    AddOperandToStack(dataStack, &result);
}

void __cdecl MinValue(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    float val; // [esp+0h] [ebp-6Ch]
    OperandList list; // [esp+4h] [ebp-68h] BYREF
    int operandNum; // [esp+5Ch] [ebp-10h]
    Operand operandResult; // [esp+60h] [ebp-Ch] BYREF
    float min; // [esp+68h] [ebp-4h]

    GetOperandList(dataStack, &list);
    if ( list.operandCount >= 1 )
    {
        min = GetSourceFloat(list.operands);
        for ( operandNum = 1; operandNum < list.operandCount; ++operandNum )
        {
            val = GetSourceFloat(&list.operands[operandNum]);
            if ( min > val )
                min = val;
        }
        operandResult.dataType = VAL_FLOAT;
        operandResult.internals.floatVal = min;
        AddOperandToStack(dataStack, &operandResult);
    }
    else
    {
        Expression_Error("UI Expression Error: Expected at least 1 parameter to min()\n");
        operandResult.dataType = VAL_FLOAT;
        operandResult.internals.intVal = 0;
        AddOperandToStack(dataStack, &operandResult);
    }
}

void __cdecl MaxValue(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion max; // [esp+0h] [ebp-70h]
    float val; // [esp+4h] [ebp-6Ch]
    OperandList list; // [esp+8h] [ebp-68h] BYREF
    int operandNum; // [esp+64h] [ebp-Ch]
    Operand operandResult; // [esp+68h] [ebp-8h] BYREF

    GetOperandList(dataStack, &list);
    if ( list.operandCount >= 1 )
    {
        max.floatVal = GetSourceFloat(list.operands);
        for ( operandNum = 1; operandNum < list.operandCount; ++operandNum )
        {
            val = GetSourceFloat(&list.operands[operandNum]);
            if ( val > max.floatVal )
                max.floatVal = val;
        }
        operandResult.dataType = VAL_FLOAT;
        operandResult.internals = max;
        AddOperandToStack(dataStack, &operandResult);
    }
    else
    {
        Expression_Error("UI Expression Error: Expected at least 1 parameter to max()\n");
        operandResult.dataType = VAL_FLOAT;
        operandResult.internals.intVal = 0;
        AddOperandToStack(dataStack, &operandResult);
    }
}

void __cdecl Select(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand result; // [esp+0h] [ebp-64h] BYREF
    OperandList list; // [esp+Ch] [ebp-58h] BYREF

    GetOperandList(dataStack, &list);
    if ( list.operandCount == 3 )
    {
        if ( GetOperandValueInt(list.operands).intVal )
            AddOperandToStack(dataStack, &list.operands[1]);
        else
            AddOperandToStack(dataStack, &list.operands[2]);
    }
    else
    {
        Expression_Error("Expected 3 parameters to Select()\n");
        result.internals.intVal = 0;
        result.dataType = VAL_INT;
        AddOperandToStack(dataStack, &result);
    }
}

void __cdecl Choose(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    int intVal; // [esp+0h] [ebp-78h]
    operandInternalDataUnion v4; // [esp+4h] [ebp-74h]
    operandInternalDataUnion v5; // [esp+8h] [ebp-70h]
    operandInternalDataUnion v6; // [esp+Ch] [ebp-6Ch]
    Operand result; // [esp+10h] [ebp-68h] BYREF
    OperandList list; // [esp+18h] [ebp-60h] BYREF
    int sel; // [esp+74h] [ebp-4h]

    GetOperandList(dataStack, &list);
    if ( list.operandCount == 1 )
    {
        Expression_Error("Expected 3 parameters to Select()\n");
        result.internals.intVal = 0;
        result.dataType = VAL_INT;
        AddOperandToStack(dataStack, &result);
    }
    else
    {
        v4.intVal = list.operandCount - 1;
        v5.intVal = GetOperandValueInt(list.operands).intVal + 1;
        if ( v5.intVal < v4.intVal )
            v6.intVal = v5.intVal;
        else
            v6.intVal = v4.intVal;
        if ( v6.intVal > 1 )
            intVal = v6.intVal;
        else
            intVal = 1;
        sel = intVal;
        AddOperandToStack(dataStack, &list.operands[intVal]);
    }
}

void __cdecl LocalizeString(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    const char *v3; // eax
    Operand result; // [esp+20h] [ebp-488h] BYREF
    char string[1024]; // [esp+28h] [ebp-480h] BYREF
    bool useLocalization; // [esp+42Fh] [ebp-79h]
    Operand *operand; // [esp+430h] [ebp-78h]
    unsigned int charIndex; // [esp+434h] [ebp-74h]
    OperandList list; // [esp+438h] [ebp-70h] BYREF
    unsigned int tokenLen; // [esp+490h] [ebp-18h]
    bool enableLocalization; // [esp+497h] [ebp-11h]
    expDataType type; // [esp+498h] [ebp-10h]
    int parmIndex; // [esp+49Ch] [ebp-Ch]
    const char *token; // [esp+4A0h] [ebp-8h]
    unsigned int stringLen; // [esp+4A4h] [ebp-4h]

    GetOperandList(dataStack, &list);
    useLocalization = 1;
    stringLen = 0;
    for ( parmIndex = 0; parmIndex < list.operandCount; ++parmIndex )
    {
        operand = &list.operands[parmIndex];
        type = operand->dataType;
        enableLocalization = 0;
        if ( type == VAL_STRING )
        {
            token = GetSourceString(*operand);
            tokenLen = strlen(token);
            if ( tokenLen <= 1 )
                continue;
            ValidateLocalizedStringRef(token, tokenLen);
            if ( *token == 64 )
            {
                ++token;
                --tokenLen;
            }
            if ( stringLen + tokenLen + 1 >= 0x400 )
                Com_Error(ERR_LOCALIZATION, "Error: %s is too long. Max length is %i\n", token, 1024);
            if ( stringLen )
                string[stringLen++] = 20;
            useLocalization = 1;
        }
        else
        {
            token = GetSourceString(*operand);
            tokenLen = strlen(token);
            for ( charIndex = 0; charIndex < tokenLen; ++charIndex )
            {
                if ( token[charIndex] == 20 || token[charIndex] == 21 || token[charIndex] == 22 )
                    Expression_Error(" bad escape character (%i) present in string", token[charIndex]);
                if ( isalpha(token[charIndex]) )
                {
                    v3 = va("Non-localized UI strings are not allowed to have letters in them: \"%s\"", token);
                    LocalizationError(v3);
                    break;
                }
            }
            if ( stringLen + tokenLen + 1 >= 0x400 )
                Com_Error(ERR_LOCALIZATION, "Error: %s is too long. Max length is %i\n", token, 1024);
            if ( tokenLen )
                string[stringLen++] = 21;
            useLocalization = 0;
        }
        for ( charIndex = 0; charIndex < tokenLen; ++charIndex )
        {
            if ( token[charIndex] == 20 || token[charIndex] == 21 || token[charIndex] == 22 )
                string[stringLen] = 46;
            else
                string[stringLen] = token[charIndex];
            ++stringLen;
        }
    }
    string[stringLen] = 0;
    result.dataType = VAL_STRING;
    result.internals.intVal = (int)SEH_LocalizeTextMessage(string, "ui string", LOCMSG_NOERR);
    if ( !result.internals.intVal )
        result.internals.intVal = (int)"";
    AddOperandToStack(dataStack, &result);
}

void __cdecl LocalizationError(const char *errorMessage)
{
    if ( Dvar_GetBool("loc_warnings") )
    {
        if ( !errorMessage
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                        12168,
                        0,
                        "%s",
                        "errorMessage") )
        {
            __debugbreak();
        }
        if ( Dvar_GetBool("loc_warningsAsErrors") )
            Com_Error(ERR_LOCALIZATION, "Error: %s", errorMessage);
        else
            Com_PrintWarning(13, "WARNING: %s\n", errorMessage);
    }
}

void __cdecl ValidateLocalizedStringRef(const char *token, int tokenLen)
{
    const char *v2; // eax
    int charIter; // [esp+0h] [ebp-4h]

    if ( !token
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    12182,
                    0,
                    "%s",
                    "token") )
    {
        __debugbreak();
    }
    if ( tokenLen <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    12183,
                    0,
                    "%s",
                    "tokenLen > 0") )
    {
        __debugbreak();
    }
    for ( charIter = 1; charIter < tokenLen; ++charIter )
    {
        if ( !isalnum(token[charIter]) && token[charIter] != 95 )
        {
            v2 = va(
                         "Illegal localized string reference: %s must contain only alpha-numeric characters and underscores",
                         token);
            LocalizationError(v2);
        }
    }
}

void __cdecl RPN_FUNC_TOINT(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand operandResult; // [esp+0h] [ebp-10h] BYREF
    Operand data1; // [esp+8h] [ebp-8h] BYREF

    GetOperand(dataStack, &data1);
    operandResult.dataType = VAL_INT;
    operandResult.internals = GetSourceInt(&data1);
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl RPN_FUNC_TOSTRING(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand operandResult; // [esp+0h] [ebp-10h] BYREF
    Operand data1; // [esp+8h] [ebp-8h] BYREF

    GetOperand(dataStack, &data1);
    operandResult.dataType = VAL_STRING;
    operandResult.internals.intVal = (int)GetSourceString(data1);
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl RPN_FUNC_TOFLOAT(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand operandResult; // [esp+0h] [ebp-10h] BYREF
    Operand data1; // [esp+8h] [ebp-8h] BYREF

    GetOperand(dataStack, &data1);
    operandResult.dataType = VAL_FLOAT;
    operandResult.internals.floatVal = GetSourceFloat(&data1);
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl RPN_FUNC_ISCLANMEMBER(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    ClanMemberRankIsAtleast(localClientNum, item, dataStack);
}

void __cdecl RPN_OP_NOOP(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Expression_Error("Invalid operator NOOP made it into the stack!\n");
}

void __cdecl RPN_FUNC_INVALID(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Expression_Error("Invalid functionmade it into the stack!\n");
}

void __cdecl RPN_OP_NOT(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand operandResult; // [esp+0h] [ebp-10h] BYREF
    Operand data1; // [esp+8h] [ebp-8h] BYREF

    GetOperand(dataStack, &data1);
    LogicalNot(localClientNum, &data1, &operandResult);
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl LogicalNot(int localClientNum, Operand *source, Operand *result)
{
    const char *NameForValueType; // eax
    char *SourceString; // [esp-4h] [ebp-Ch]

    result->dataType = VAL_INT;
    if ( source->dataType == VAL_STRING )
    {
        SourceString = GetSourceString(*source);
        NameForValueType = GetNameForValueType(source->dataType);
        Expression_Error("You cannot ! a '%s' (%s)\n", NameForValueType, SourceString);
        result->internals.intVal = 0;
    }
    else if ( source->dataType )
    {
        if ( source->dataType == VAL_FLOAT )
            result->internals.intVal = source->internals.floatVal == 0.0;
        else
            Expression_Error("Unknown datatype %i in LogicalNot()\n", source->dataType);
    }
    else
    {
        result->internals.intVal = source->internals.intVal == 0;
    }
}

void __cdecl RPN_OP_BITWISENOT(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand operandResult; // [esp+0h] [ebp-10h] BYREF
    Operand data1; // [esp+8h] [ebp-8h] BYREF

    GetOperand(dataStack, &data1);
    BitwiseNot(localClientNum, &data1, &operandResult);
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl BitwiseNot(int localClientNum, Operand *source, Operand *result)
{
    const char *NameForValueType; // eax
    char *SourceString; // [esp-4h] [ebp-Ch]
    int val; // [esp+4h] [ebp-4h]

    result->dataType = VAL_INT;
    if ( source->dataType == VAL_STRING )
    {
        SourceString = GetSourceString(*source);
        NameForValueType = GetNameForValueType(source->dataType);
        Expression_Error(" You cannot ~ a '%s' (%s)\n", NameForValueType, SourceString);
        result->internals.intVal = 0;
    }
    else
    {
        if ( source->dataType )
        {
            if ( source->dataType == VAL_FLOAT )
            {
                val = (int)source->internals.floatVal;
            }
            else
            {
                val = 0;
                Expression_Error("Unknown datatype %i in LogicalNot()\n", source->dataType);
            }
        }
        else
        {
            val = source->internals.intVal;
        }
        result->internals.intVal = ~val;
        if ( uiscript_debug )
        {
            if ( uiscript_debug->current.integer )
                Expression_TraceInternal("~%i = %i\n", val, result->internals.intVal);
        }
    }
}

void __cdecl RPN_OP_BITSHIFTLEFT(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand data2; // [esp+0h] [ebp-18h] BYREF
    Operand operandResult; // [esp+8h] [ebp-10h] BYREF
    Operand data1; // [esp+10h] [ebp-8h] BYREF

    GetTwoOperands(dataStack, &data1, &data2);
    BitShiftLeft(localClientNum, &data1, &data2, &operandResult);
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl BitShiftLeft(int localClientNum, Operand *source, Operand *bitsSource, Operand *result)
{
    int val; // [esp+0h] [ebp-8h]
    int bits; // [esp+4h] [ebp-4h]

    val = GetSourceInt(source).intVal;
    bits = GetSourceInt(bitsSource).intVal;
    result->dataType = VAL_INT;
    result->internals.intVal = val << bits;
    if ( uiscript_debug )
    {
        if ( uiscript_debug->current.integer )
            Expression_TraceInternal("%i << %i = %i\n", val, bits, result->internals.intVal);
    }
}

void __cdecl RPN_OP_BITSHIFTRIGHT(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand data2; // [esp+0h] [ebp-18h] BYREF
    Operand operandResult; // [esp+8h] [ebp-10h] BYREF
    Operand data1; // [esp+10h] [ebp-8h] BYREF

    GetTwoOperands(dataStack, &data1, &data2);
    BitShiftRight(localClientNum, &data1, &data2, &operandResult);
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl BitShiftRight(int localClientNum, Operand *source, Operand *bitsSource, Operand *result)
{
    int val; // [esp+0h] [ebp-8h]
    int bits; // [esp+4h] [ebp-4h]

    val = GetSourceInt(source).intVal;
    bits = GetSourceInt(bitsSource).intVal;
    result->dataType = VAL_INT;
    result->internals.intVal = val >> bits;
    if ( uiscript_debug )
    {
        if ( uiscript_debug->current.integer )
            Expression_TraceInternal("%i >> %i = %i\n", val, bits, result->internals.intVal);
    }
}

void __cdecl RPN_OP_NEGATE(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand data1; // [esp+0h] [ebp-8h] BYREF

    GetOperand(dataStack, &data1);
    if ( data1.dataType )
    {
        if ( data1.dataType != VAL_FLOAT )
        {
            Expression_Error(" trying to negate a string: %s\n", data1.internals.string);
            return;
        }
        //data1.internals.intVal ^= _mask__NegFloat_;
        data1.internals.intVal = -data1.internals.intVal;
    }
    else
    {
        data1.internals.intVal = -data1.internals.intVal;
    }
    AddOperandToStack(dataStack, &data1);
}

void __cdecl RPN_OP_MULTIPLY(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand data2; // [esp+14h] [ebp-18h] BYREF
    Operand operandResult; // [esp+1Ch] [ebp-10h] BYREF
    Operand data1; // [esp+24h] [ebp-8h] BYREF

    GetTwoOperands(dataStack, &data1, &data2);
    MakeStringOperandNumber(&data1);
    MakeStringOperandNumber(&data2);
    switch ( data1.dataType | (4 * data2.dataType) )
    {
        case 0:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = data2.internals.intVal * data1.internals.intVal;
            break;
        case 1:
            operandResult.dataType = VAL_FLOAT;
            operandResult.internals.floatVal = (float)data2.internals.intVal * data1.internals.floatVal;
            break;
        case 4:
            operandResult.dataType = VAL_FLOAT;
            operandResult.internals.floatVal = (float)data1.internals.intVal * data2.internals.floatVal;
            break;
        case 5:
            operandResult.dataType = VAL_FLOAT;
            operandResult.internals.floatVal = data1.internals.floatVal * data2.internals.floatVal;
            break;
        default:
            break;
    }
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl MakeStringOperandNumber(Operand *op)
{
    float f; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if ( op->dataType == VAL_STRING )
    {
        f = atof(op->internals.string);
        i = atoi(op->internals.string);
        if ( f == (float)i )
        {
            op->dataType = VAL_INT;
            op->internals.intVal = i;
        }
        else
        {
            op->dataType = VAL_FLOAT;
            op->internals.floatVal = f;
        }
    }
}

void __cdecl RPN_OP_DIVIDE(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    float floatVal; // [esp+0h] [ebp-1Ch]
    Operand data2; // [esp+4h] [ebp-18h] BYREF
    Operand operandResult; // [esp+Ch] [ebp-10h] BYREF
    Operand data1; // [esp+14h] [ebp-8h] BYREF

    GetTwoOperands(dataStack, &data1, &data2);
    if ( data1.dataType )
    {
        if ( data1.dataType == VAL_STRING )
        {
            data1.dataType = VAL_FLOAT;
            data1.internals.floatVal = atof(data1.internals.string);
        }
    }
    else
    {
        data1.dataType = VAL_FLOAT;
        data1.internals.floatVal = (float)data1.internals.intVal;
    }
    if ( data2.dataType )
    {
        if ( data2.dataType == VAL_STRING )
        {
            data2.dataType = VAL_FLOAT;
            data2.internals.floatVal = atof(data2.internals.string);
        }
    }
    else
    {
        data2.dataType = VAL_FLOAT;
        data2.internals.floatVal = (float)data2.internals.intVal;
    }
    operandResult.dataType = VAL_FLOAT;
    if ( data2.internals.floatVal == 0.0 )
        floatVal = 1.0f;
    else
        floatVal = data2.internals.floatVal;
    operandResult.internals.floatVal = data1.internals.floatVal / floatVal;
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl RPN_OP_MODULUS(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // [esp+0h] [ebp-34h]
    Operand data2; // [esp+1Ch] [ebp-18h] BYREF
    Operand operandResult; // [esp+24h] [ebp-10h] BYREF
    Operand data1; // [esp+2Ch] [ebp-8h] BYREF

    GetTwoOperands(dataStack, &data1, &data2);
    if ( data1.dataType == VAL_FLOAT )
    {
        data1.dataType = VAL_INT;
        data1.internals.intVal = (int)(data1.internals.floatVal + 9.313225746154785e-10);
    }
    else if ( data1.dataType == VAL_STRING )
    {
        data1.dataType = VAL_INT;
        data1.internals.intVal = atoi(data1.internals.string);
    }
    if ( data2.dataType == VAL_FLOAT )
    {
        data2.dataType = VAL_INT;
        data2.internals.intVal = (int)(data2.internals.floatVal + 9.313225746154785e-10);
    }
    else if ( data2.dataType == VAL_STRING )
    {
        data2.dataType = VAL_INT;
        data2.internals.intVal = atoi(data2.internals.string);
    }
    if ( (data1.dataType || data2.dataType)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    12511,
                    0,
                    "%s",
                    "data1.dataType == VAL_INT && data2.dataType == VAL_INT") )
    {
        __debugbreak();
    }
    operandResult.dataType = VAL_INT;
    if ( data2.internals.intVal )
        v3.intVal = (int)data2.internals;
    else
        v3.intVal = 1;
    operandResult.internals.intVal = data1.internals.intVal % v3.intVal;
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl RPN_OP_ADD(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    char rightSideStr[256]; // [esp+10h] [ebp-320h] BYREF
    Operand data2; // [esp+110h] [ebp-220h] BYREF
    Operand operandResult; // [esp+118h] [ebp-218h] BYREF
    char leftSideStr[256]; // [esp+120h] [ebp-210h] BYREF
    char resultStr[260]; // [esp+220h] [ebp-110h] BYREF
    Operand data1; // [esp+328h] [ebp-8h] BYREF

    GetTwoOperands(dataStack, &data1, &data2);
    switch ( data1.dataType | (4 * data2.dataType) )
    {
        case 0:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = data2.internals.intVal + data1.internals.intVal;
            break;
        case 1:
            operandResult.dataType = VAL_FLOAT;
            operandResult.internals.floatVal = (float)data2.internals.intVal + data1.internals.floatVal;
            break;
        case 2:
            operandResult.dataType = VAL_STRING;
            I_strncpyz(leftSideStr, data1.internals.string, 256);
            Com_sprintf(resultStr, 0x100u, "%s%i", leftSideStr, data2.internals.intVal);
            operandResult.internals.intVal = (int)CopyTempString(resultStr);
            break;
        case 4:
            operandResult.dataType = VAL_FLOAT;
            operandResult.internals.floatVal = (float)data1.internals.intVal + data2.internals.floatVal;
            break;
        case 5:
            operandResult.dataType = VAL_FLOAT;
            operandResult.internals.floatVal = data1.internals.floatVal + data2.internals.floatVal;
            break;
        case 6:
            operandResult.dataType = VAL_STRING;
            I_strncpyz(leftSideStr, data1.internals.string, 256);
            Com_sprintf(resultStr, 0x100u, "%s%f", leftSideStr, data2.internals.floatVal);
            operandResult.internals.intVal = (int)CopyTempString(resultStr);
            break;
        case 8:
            operandResult.dataType = VAL_STRING;
            I_strncpyz(rightSideStr, data2.internals.string, 256);
            Com_sprintf(resultStr, 0x100u, "%i%s", data1.internals.intVal, rightSideStr);
            operandResult.internals.intVal = (int)CopyTempString(resultStr);
            break;
        case 9:
            operandResult.dataType = VAL_STRING;
            I_strncpyz(rightSideStr, data2.internals.string, 256);
            Com_sprintf(resultStr, 0x100u, "%f%s", data1.internals.floatVal, rightSideStr);
            operandResult.internals.intVal = (int)CopyTempString(resultStr);
            break;
        case 0xA:
            operandResult.dataType = VAL_STRING;
            I_strncpyz(leftSideStr, data1.internals.string, 256);
            I_strncpyz(rightSideStr, data2.internals.string, 256);
            Com_sprintf(resultStr, 0x100u, "%s%s", leftSideStr, rightSideStr);
            operandResult.internals.intVal = (int)CopyTempString(resultStr);
            break;
        default:
            break;
    }
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl RPN_OP_LESSTHAN(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand data2; // [esp+20h] [ebp-18h] BYREF
    Operand operandResult; // [esp+28h] [ebp-10h] BYREF
    Operand data1; // [esp+30h] [ebp-8h] BYREF

    GetTwoOperands(dataStack, &data1, &data2);
    MakeStringOperandNumber(&data1);
    MakeStringOperandNumber(&data2);
    switch ( data1.dataType | (4 * data2.dataType) )
    {
        case 0:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = data1.internals.intVal < data2.internals.intVal;
            break;
        case 1:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = (float)data2.internals.intVal > data1.internals.floatVal;
            break;
        case 4:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = data2.internals.floatVal > (float)data1.internals.intVal;
            break;
        case 5:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = data2.internals.floatVal > data1.internals.floatVal;
            break;
        default:
            break;
    }
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl RPN_OP_LESSTHANEQUALTO(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand data2; // [esp+20h] [ebp-18h] BYREF
    Operand operandResult; // [esp+28h] [ebp-10h] BYREF
    Operand data1; // [esp+30h] [ebp-8h] BYREF

    GetTwoOperands(dataStack, &data1, &data2);
    MakeStringOperandNumber(&data1);
    MakeStringOperandNumber(&data2);
    switch ( data1.dataType | (4 * data2.dataType) )
    {
        case 0:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = data1.internals.intVal <= data2.internals.intVal;
            break;
        case 1:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = (float)data2.internals.intVal >= data1.internals.floatVal;
            break;
        case 4:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = data2.internals.floatVal >= (float)data1.internals.intVal;
            break;
        case 5:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = data2.internals.floatVal >= data1.internals.floatVal;
            break;
        default:
            break;
    }
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl RPN_OP_GREATERTHAN(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand data2; // [esp+20h] [ebp-18h] BYREF
    Operand operandResult; // [esp+28h] [ebp-10h] BYREF
    Operand data1; // [esp+30h] [ebp-8h] BYREF

    GetTwoOperands(dataStack, &data1, &data2);
    MakeStringOperandNumber(&data1);
    MakeStringOperandNumber(&data2);
    switch ( data1.dataType | (4 * data2.dataType) )
    {
        case 0:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = data1.internals.intVal > data2.internals.intVal;
            break;
        case 1:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = data1.internals.floatVal > (float)data2.internals.intVal;
            break;
        case 4:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = (float)data1.internals.intVal > data2.internals.floatVal;
            break;
        case 5:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = data1.internals.floatVal > data2.internals.floatVal;
            break;
        default:
            break;
    }
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl RPN_OP_GREATERTHANEQUALTO(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand data2; // [esp+20h] [ebp-18h] BYREF
    Operand operandResult; // [esp+28h] [ebp-10h] BYREF
    Operand data1; // [esp+30h] [ebp-8h] BYREF

    GetTwoOperands(dataStack, &data1, &data2);
    MakeStringOperandNumber(&data1);
    MakeStringOperandNumber(&data2);
    switch ( data1.dataType | (4 * data2.dataType) )
    {
        case 0:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = data1.internals.intVal >= data2.internals.intVal;
            break;
        case 1:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = data1.internals.floatVal >= (float)data2.internals.intVal;
            break;
        case 4:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = (float)data1.internals.intVal >= data2.internals.floatVal;
            break;
        case 5:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = data1.internals.floatVal >= data2.internals.floatVal;
            break;
        default:
            break;
    }
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl RPN_OP_EQUALS(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand data2; // [esp+20h] [ebp-18h] BYREF
    Operand operandResult; // [esp+28h] [ebp-10h] BYREF
    Operand data1; // [esp+30h] [ebp-8h] BYREF

    GetTwoOperands(dataStack, &data1, &data2);
    if ( data1.dataType != data2.dataType )
    {
        MakeStringOperandNumber(&data1);
        MakeStringOperandNumber(&data2);
    }
    switch ( data1.dataType | (4 * data2.dataType) )
    {
        case 0:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = data1.internals.intVal == data2.internals.intVal;
            break;
        case 1:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = data1.internals.floatVal == (float)data2.internals.intVal;
            break;
        case 4:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = (float)data1.internals.intVal == data2.internals.floatVal;
            break;
        case 5:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = data1.internals.floatVal == data2.internals.floatVal;
            break;
        case 0xA:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = I_stricmp(data1.internals.string, data2.internals.string) == 0;
            break;
        default:
            break;
    }
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl RPN_OP_NOTEQUAL(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand data2; // [esp+20h] [ebp-18h] BYREF
    Operand operandResult; // [esp+28h] [ebp-10h] BYREF
    Operand data1; // [esp+30h] [ebp-8h] BYREF

    GetTwoOperands(dataStack, &data1, &data2);
    if ( data1.dataType != data2.dataType )
    {
        MakeStringOperandNumber(&data1);
        MakeStringOperandNumber(&data2);
    }
    switch ( data1.dataType | (4 * data2.dataType) )
    {
        case 0:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = data1.internals.intVal != data2.internals.intVal;
            break;
        case 1:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = data1.internals.floatVal != (float)data2.internals.intVal;
            break;
        case 4:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = (float)data1.internals.intVal != data2.internals.floatVal;
            break;
        case 5:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = data1.internals.floatVal != data2.internals.floatVal;
            break;
        case 0xA:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = I_stricmp(data1.internals.string, data2.internals.string) != 0;
            break;
        default:
            break;
    }
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl RPN_OP_AND(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    bool v3; // [esp+0h] [ebp-2Ch]
    bool v4; // [esp+4h] [ebp-28h]
    bool v5; // [esp+8h] [ebp-24h]
    bool v6; // [esp+Ch] [ebp-20h]
    Operand data2; // [esp+14h] [ebp-18h] BYREF
    Operand operandResult; // [esp+1Ch] [ebp-10h] BYREF
    Operand data1; // [esp+24h] [ebp-8h] BYREF

    GetTwoOperands(dataStack, &data1, &data2);
    if ( data1.dataType == VAL_STRING )
    {
        data1.dataType = VAL_INT;
        data1.internals.intVal = *(char *)data1.internals.intVal;
    }
    if ( data2.dataType == VAL_STRING )
    {
        data2.dataType = VAL_INT;
        data2.internals.intVal = *(char *)data2.internals.intVal;
    }
    switch ( data1.dataType | (4 * data2.dataType) )
    {
        case 0:
            operandResult.dataType = VAL_INT;
            v3 = data1.internals.intVal && data2.internals.intVal;
            operandResult.internals.intVal = v3;
            break;
        case 1:
            operandResult.dataType = VAL_INT;
            v5 = data1.internals.floatVal != 0.0 && data2.internals.intVal;
            operandResult.internals.intVal = v5;
            break;
        case 4:
            operandResult.dataType = VAL_INT;
            v4 = data1.internals.intVal && data2.internals.floatVal != 0.0;
            operandResult.internals.intVal = v4;
            break;
        case 5:
            operandResult.dataType = VAL_INT;
            v6 = data1.internals.floatVal != 0.0 && data2.internals.floatVal != 0.0;
            operandResult.internals.intVal = v6;
            break;
        default:
            break;
    }
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl RPN_OP_OR(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    bool v3; // [esp+0h] [ebp-2Ch]
    bool v4; // [esp+4h] [ebp-28h]
    bool v5; // [esp+8h] [ebp-24h]
    bool v6; // [esp+Ch] [ebp-20h]
    Operand data2; // [esp+14h] [ebp-18h] BYREF
    Operand operandResult; // [esp+1Ch] [ebp-10h] BYREF
    Operand data1; // [esp+24h] [ebp-8h] BYREF

    GetTwoOperands(dataStack, &data1, &data2);
    if ( data1.dataType == VAL_STRING )
    {
        data1.dataType = VAL_INT;
        data1.internals.intVal = *(char *)data1.internals.intVal;
    }
    if ( data2.dataType == VAL_STRING )
    {
        data2.dataType = VAL_INT;
        data2.internals.intVal = *(char *)data2.internals.intVal;
    }
    switch ( data1.dataType | (4 * data2.dataType) )
    {
        case 0:
            operandResult.dataType = VAL_INT;
            v3 = data1.internals.intVal || data2.internals.intVal;
            operandResult.internals.intVal = v3;
            break;
        case 1:
            operandResult.dataType = VAL_INT;
            v5 = data1.internals.floatVal != 0.0 || data2.internals.intVal;
            operandResult.internals.intVal = v5;
            break;
        case 4:
            operandResult.dataType = VAL_INT;
            v4 = data1.internals.intVal || data2.internals.floatVal != 0.0;
            operandResult.internals.intVal = v4;
            break;
        case 5:
            operandResult.dataType = VAL_INT;
            v6 = data1.internals.floatVal != 0.0 || data2.internals.floatVal != 0.0;
            operandResult.internals.intVal = v6;
            break;
        default:
            break;
    }
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl RPN_OP_BITWISEAND(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // esi
    Operand data2; // [esp+4h] [ebp-18h] BYREF
    Operand operandResult; // [esp+Ch] [ebp-10h] BYREF
    Operand data1; // [esp+14h] [ebp-8h] BYREF

    GetTwoOperands(dataStack, &data1, &data2);
    operandResult.dataType = VAL_INT;
    v3.intVal = GetSourceInt(&data1).intVal;
    operandResult.internals.intVal = GetSourceInt(&data2).intVal & v3.intVal;
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl RPN_OP_BITWISEOR(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // esi
    Operand data2; // [esp+4h] [ebp-18h] BYREF
    Operand operandResult; // [esp+Ch] [ebp-10h] BYREF
    Operand data1; // [esp+14h] [ebp-8h] BYREF

    GetTwoOperands(dataStack, &data1, &data2);
    operandResult.dataType = VAL_INT;
    v3.intVal = GetSourceInt(&data1).intVal;
    operandResult.internals.intVal = GetSourceInt(&data2).intVal | v3.intVal;
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl RPN_OP_SUBTRACT(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand data2; // [esp+14h] [ebp-18h] BYREF
    Operand operandResult; // [esp+1Ch] [ebp-10h] BYREF
    Operand data1; // [esp+24h] [ebp-8h] BYREF

    GetTwoOperands(dataStack, &data1, &data2);
    MakeStringOperandNumber(&data1);
    MakeStringOperandNumber(&data2);
    switch ( data1.dataType | (4 * data2.dataType) )
    {
        case 0:
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = data1.internals.intVal - data2.internals.intVal;
            break;
        case 1:
            operandResult.dataType = VAL_FLOAT;
            operandResult.internals.floatVal = data1.internals.floatVal - (float)data2.internals.intVal;
            break;
        case 4:
            operandResult.dataType = VAL_FLOAT;
            operandResult.internals.floatVal = (float)data1.internals.intVal - data2.internals.floatVal;
            break;
        case 5:
            operandResult.dataType = VAL_FLOAT;
            operandResult.internals.floatVal = data1.internals.floatVal - data2.internals.floatVal;
            break;
        default:
            break;
    }
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl RPN_OP_COMMA(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    int v4; // ecx
    Operand *v5; // esi
    operandInternalDataUnion v6; // ecx
    int v7; // edx
    Operand *v8; // esi
    Operand operandResult; // [esp+4h] [ebp-CCh] BYREF
    int list1Operand; // [esp+Ch] [ebp-C4h]
    OperandList list2; // [esp+10h] [ebp-C0h] BYREF
    Operand *finalList; // [esp+6Ch] [ebp-64h]
    OperandList list1; // [esp+70h] [ebp-60h] BYREF
    int operand; // [esp+C8h] [ebp-8h]
    int list2Operand; // [esp+CCh] [ebp-4h]

    if ( GetOperandList(dataStack, &list2) && GetOperandList(dataStack, &list1) )
    {
        if ( list2.operandCount + list1.operandCount <= 10 )
        {
            dataStack->stack[dataStack->numOperandLists].operandCount = list2.operandCount + list1.operandCount;
            finalList = dataStack->stack[dataStack->numOperandLists].operands;
            operand = 0;
            for ( list1Operand = 0; list1Operand < list1.operandCount; ++list1Operand )
            {
                v3.intVal = (int)list1.operands[list1Operand].internals;
                v4 = operand;
                v5 = finalList;
                finalList[operand].dataType = list1.operands[list1Operand].dataType;
                v5[v4].internals = v3;
                if ( finalList[operand].dataType == VAL_STRING
                    && !finalList[operand].internals.intVal
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                                12922,
                                0,
                                "%s",
                                "finalList[operand].dataType != VAL_STRING || finalList[operand].internals.string") )
                {
                    __debugbreak();
                }
                ++operand;
            }
            for ( list2Operand = 0; list2Operand < list2.operandCount; ++list2Operand )
            {
                v6.intVal = (int)list2.operands[list2Operand].internals;
                v7 = operand;
                v8 = finalList;
                finalList[operand].dataType = list2.operands[list2Operand].dataType;
                v8[v7].internals = v6;
                if ( finalList[operand].dataType == VAL_STRING
                    && !finalList[operand].internals.intVal
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                                12928,
                                0,
                                "%s",
                                "finalList[operand].dataType != VAL_STRING || finalList[operand].internals.string") )
                {
                    __debugbreak();
                }
                ++operand;
            }
            ++dataStack->numOperandLists;
        }
        else
        {
            Expression_Error("UI Script error: No function takes %i parameters\n", list2.operandCount + list1.operandCount);
            operandResult.dataType = VAL_INT;
            operandResult.internals.intVal = 0;
            AddOperandToStack(dataStack, &operandResult);
        }
    }
}

void __cdecl ApproxEquals(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    float valueB; // [esp+0h] [ebp-70h]
    float tolerance; // [esp+4h] [ebp-6Ch]
    float valueA; // [esp+8h] [ebp-68h]
    OperandList list; // [esp+10h] [ebp-60h] BYREF
    Operand operandResult; // [esp+68h] [ebp-8h] BYREF

    operandResult.dataType = VAL_INT;
    operandResult.internals.intVal = 0;
    GetOperandList(dataStack, &list);
    if ( list.operandCount == 3 )
    {
        valueA = GetSourceFloat(list.operands);
        valueB = GetSourceFloat(&list.operands[1]);
        tolerance = GetSourceFloat(&list.operands[2]);
        if ( tolerance > fabs(valueA - valueB) )
            operandResult.internals.intVal = 1;
    }
    else
    {
        Expression_Error("UI Expression Error: Expected at least 3 parameters to approxEquals( valueA, valueB, tolerance )\n");
    }
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl GetLeaderboardValue(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand operandResult; // [esp+4h] [ebp-8h] BYREF

    operandResult.dataType = VAL_FLOAT;
    operandResult.internals.intVal = 0;
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl GetStatValue(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    Operand operandResult; // [esp+0h] [ebp-Ch] BYREF
    int statValue; // [esp+8h] [ebp-4h]

    statValue = 0;
    operandResult.dataType = VAL_INT;
    operandResult.internals.intVal = 0;
    AddOperandToStack(dataStack, &operandResult);
}

void __cdecl Add64(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    unsigned __int64 op2; // [esp+0h] [ebp-70h]
    unsigned __int64 op1; // [esp+8h] [ebp-68h]
    Operand result; // [esp+10h] [ebp-60h] BYREF
    OperandList list; // [esp+18h] [ebp-58h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    GetOperandList(dataStack, &list);
    if ( list.operandCount == 2 )
    {
        op1 = I_atoi64(list.operands[0].internals.string);
        op2 = I_atoi64(list.operands[1].internals.string);
        result.internals.intVal = (int)va("%llu", op2 + op1);
    }
    else
    {
        Expression_Error("Binary operator function Add64() needs 2 operand parameters. Got %d\n", list.operandCount);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl Sub64(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    unsigned __int64 op2; // [esp+0h] [ebp-70h]
    unsigned __int64 op1; // [esp+8h] [ebp-68h]
    Operand result; // [esp+10h] [ebp-60h] BYREF
    OperandList list; // [esp+18h] [ebp-58h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    GetOperandList(dataStack, &list);
    if ( list.operandCount == 2 )
    {
        op1 = I_atoi64(list.operands[0].internals.string);
        op2 = I_atoi64(list.operands[1].internals.string);
        result.internals.intVal = (int)va("%llu", op1 - op2);
    }
    else
    {
        Expression_Error("Binary operator function Sub64() needs 2 operand parameters. Got %d\n", list.operandCount);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl Div64(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    unsigned __int64 op2; // [esp+0h] [ebp-70h]
    unsigned __int64 op1; // [esp+8h] [ebp-68h]
    Operand result; // [esp+10h] [ebp-60h] BYREF
    OperandList list; // [esp+18h] [ebp-58h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    GetOperandList(dataStack, &list);
    if ( list.operandCount == 2 )
    {
        op1 = I_atoi64(list.operands[0].internals.string);
        op2 = I_atoi64(list.operands[1].internals.string);
        if ( op2 )
            result.internals.intVal = (int)va("%llu", op1 / op2);
    }
    else
    {
        Expression_Error("Binary operator function Div64() needs 2 operand parameters. Got %d\n", list.operandCount);
    }
    AddOperandToStack(dataStack, &result);
}

void __cdecl Mul64(int localClientNum, itemDef_s *item, OperandStack *dataStack)
{
    __int64 v3; // rax
    unsigned __int64 op1; // [esp+8h] [ebp-68h]
    Operand result; // [esp+10h] [ebp-60h] BYREF
    OperandList list; // [esp+18h] [ebp-58h] BYREF

    result.dataType = VAL_STRING;
    result.internals.intVal = (int)"";
    GetOperandList(dataStack, &list);
    if ( list.operandCount == 2 )
    {
        op1 = I_atoi64(list.operands[0].internals.string);
        v3 = op1 * I_atoi64(list.operands[1].internals.string);
        result.internals.intVal = (int)va("%llu", v3);
    }
    else
    {
        Expression_Error("Binary operator function Mul64() needs 2 operand parameters. Got %d\n", list.operandCount);
    }
    AddOperandToStack(dataStack, &result);
}

int __cdecl Expression_GetIndexForFunction(int func)
{
    return s_functionToIndexMap[func];
}

int __cdecl Expression_GetFunctionForOp(int op)
{
    if (op < NUM_EXPRESSION_OPERATORS)
    {
        return op;
    }

    return s_indexToFunctionMap[op - NUM_EXPRESSION_OPERATORS] + NUM_EXPRESSION_OPERATORS;
}

char resultString_2[256];
char *__cdecl GetExpressionResultString(int localClientNum, itemDef_s *item, ExpressionStatement *statement)
{
    const char *v4; // eax
    Operand result; // [esp+Ch] [ebp-Ch] BYREF
    int len; // [esp+14h] [ebp-4h]

    if ( !EvaluateExpression(localClientNum, item, statement, &result, 1) )
        return (char *)"";
    len = 0;
    if ( result.dataType )
    {
        if ( result.dataType == VAL_FLOAT )
        {
            len = Com_sprintf(resultString_2, 0x100u, "%f", result.internals.floatVal);
        }
        else
        {
            if ( result.dataType != VAL_STRING )
            {
                v4 = va("Unknown result datatype of %i", result.dataType);
                if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp", 13338, 0, v4) )
                    __debugbreak();
                return (char *)"";
            }
            len = Com_sprintf(resultString_2, 0x100u, "%s", result.internals.string);
        }
    }
    else
    {
        len = Com_sprintf(resultString_2, 0x100u, "%i", result.internals.intVal);
    }
    if ( len < 0 )
        Expression_Warn(
            "Expression result string has been truncated, longer than %d characters: %s...\n",
            256,
            resultString_2);
    return resultString_2;
}

const ExpressionStatement *s_currentStatement;
char __cdecl EvaluateExpression(
                int localClientNum,
                itemDef_s *item,
                ExpressionStatement *statement,
                Operand *results,
                int resultCount)
{
    int type; // [esp+0h] [ebp-13C4h]
    expressionRpn *rpn; // [esp+8h] [ebp-13BCh]

    OperandStack dataStack; // [esp+Ch] [ebp-13B8h] BYREF

    rpn = statement->rpn;
    s_currentStatement = statement;

    //if ( uiscript_debug && uiscript_debug->current.integer )
        //BLOPS_NULLSUB();

    memset(&dataStack, 0, sizeof(dataStack));

    for (expressionRpn *i = rpn; i->type != 3; ++i )
    {
        type = i->type;
        if ( !type )
        {
            AddOperandToStack(&dataStack, &i->data.constant);
            continue;
        }
        if ( type == 1 )
        {
            i->type = 2;
            i->data.cmd = rpnFunctions[Expression_GetFunctionForOp(i->data.cmdIdx)];
            ((void (__cdecl *)(const int, itemDef_s *, OperandStack *))i->data.cmd)(localClientNum, item, &dataStack);
            continue;
        }
        if ( type == 2 )
            ((void(__cdecl *)(const int, itemDef_s *, OperandStack *))i->data.cmd)(localClientNum, item, &dataStack);
    }


    if ( dataStack.numOperandLists <= 1 )
    {
        if ( dataStack.numOperandLists )
        {
            if ( dataStack.stack[0].operandCount == resultCount )
            {
                memcpy(results, dataStack.stack, sizeof(Operand) * resultCount);
                s_currentStatement = 0;
                return 1;
            }
            else
            {
                Expression_Error("Incorrect number of results in expression\n");
                s_currentStatement = 0;
                return 0;
            }
        }
        else
        {
            Expression_Error("At the end of EvaluateExpression, we have no operands!");
            s_currentStatement = 0;
            return 0;
        }
    }
    else
    {
        Expression_Error("stray operands in expression\n");
        s_currentStatement = 0;
        return 0;
    }
}

bool __cdecl IsExpressionTrue(int localClientNum, itemDef_s *item, ExpressionStatement *statement)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    return EvaluateExpression(localClientNum, item, statement, &result, 1) && GetSourceInt(&result).intVal != 0;
}

operandInternalDataUnion __cdecl GetExpressionInt(int localClientNum, itemDef_s *item, ExpressionStatement *statement)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    if ( EvaluateExpression(localClientNum, item, statement, &result, 1) )
        return GetSourceInt(&result);
    return 0;
}

double __cdecl GetExpressionFloat(int localClientNum, itemDef_s *item, ExpressionStatement *statement)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    if ( EvaluateExpression(localClientNum, item, statement, &result, 1) )
        return GetSourceFloat(&result);
    else
        return 0.0;
}

char __cdecl GetExpressionStringInt(
                int localClientNum,
                itemDef_s *item,
                ExpressionStatement *statement,
                char **outString,
                operandInternalDataUnion *outInt)
{
    Operand results[2]; // [esp+0h] [ebp-10h] BYREF

    if ( !EvaluateExpression(localClientNum, item, statement, results, 2) )
        return 0;
    *outString = GetSourceString(results[0]);
    outInt->intVal = GetSourceInt(&results[1]).intVal;
    return 1;
}

char *__cdecl GetExpressionResultStringCompile(
                int localClientNum,
                itemDef_s *item,
                const char **text,
                void *compileBuffer,
                int compileBufferSize)
{
    ExpressionStatement statement; // [esp+0h] [ebp-10h] BYREF

    statement.filename = (char*)"runtime";
    statement.line = 0;
    if ( Expression_Parse(text, &statement, compileBuffer, compileBufferSize) )
        return GetExpressionResultString(localClientNum, item, &statement);
    else
        return (char *)"";
}


void __cdecl Expression_MapIndexToFunction(int index, const char *function)
{
    int i; // [esp+0h] [ebp-4h]

    if ( s_indexToFunctionMap[index]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions_eval.cpp",
                    13443,
                    0,
                    "%s",
                    "s_indexToFunctionMap[ index ] == FUNC_INVALID") )
    {
        __debugbreak();
    }
    for ( i = 0; g_expFunctionNames[i]; ++i )
    {
        if ( !I_stricmp(g_expFunctionNames[i], function) )
        {
            s_indexToFunctionMap[index] = i;
            s_functionToIndexMap[i] = index;
        }
    }
}

int s_countErrorOutput;
int s_lastErrorOutputMS;
char __cdecl Expression_Throttle()
{
    unsigned int ms; // [esp+0h] [ebp-4h]

    ms = Sys_Milliseconds();
    if ( s_countErrorOutput > 10 && (int)(ms - s_lastErrorOutputMS) < 5000 )
        return 1;
    if ( (int)(ms - s_lastErrorOutputMS) > 5000 )
        s_countErrorOutput = 0;
    ++s_countErrorOutput;
    s_lastErrorOutputMS = ms;
    return 0;
}

void Expression_Error(const char *str, ...)
{
    char buf[1024]; // [esp+14h] [ebp-408h] BYREF
    int offset; // [esp+418h] [ebp-4h]
    va_list va; // [esp+428h] [ebp+Ch] BYREF

    va_start(va, str);
    if ( !Expression_Throttle() )
    {
        _snprintf(buf, 0x400u, "^1Expression Error (%s:%d): ", s_currentStatement->filename, s_currentStatement->line);
        offset = &buf[strlen(buf) + 1] - &buf[1];
        _vsnprintf(&buf[offset], 0x400u, str, va);
        buf[1023] = 0;
        //BLOPS_NULLSUB();
        Com_PrintMessage(16, buf, 3);
    }
}

void Expression_Warn(const char *str, ...)
{
    char buf[1024]; // [esp+14h] [ebp-408h] BYREF
    int offset; // [esp+418h] [ebp-4h]
    va_list va; // [esp+428h] [ebp+Ch] BYREF

    va_start(va, str);
    if ( !Expression_Throttle() )
    {
        _snprintf(buf, 0x400u, "^3Expression Warning (%s:%d): ", s_currentStatement->filename, s_currentStatement->line);
        offset = &buf[strlen(buf) + 1] - &buf[1];
        _vsnprintf(&buf[offset], 0x400u, str, va);
        buf[1023] = 0;
        //BLOPS_NULLSUB();
        Com_PrintMessage(16, buf, 2);
    }
}

void Expression_TraceInternal(const char *str, ...)
{
    char buf[1024]; // [esp+14h] [ebp-408h] BYREF
    int offset; // [esp+418h] [ebp-4h]
    va_list va; // [esp+428h] [ebp+Ch] BYREF

    va_start(va, str);
    I_strncpyz(buf, "^2Expression Trace: ", 1024);
    offset = &buf[strlen(buf) + 1] - &buf[1];
    _vsnprintf(&buf[offset], 0x400u, str, va);
    buf[1023] = 0;
    Com_PrintMessage(16, buf, 0);
}

