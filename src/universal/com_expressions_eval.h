#pragma once

#include "com_expressions.h"
#include <ddl/ddl_api.h>
#include <universal/offline.h>
#include "com_stringtable.h"
#include <ui/ui_main.h>
#include <client/cl_milestone.h>

struct OperandList // sizeof=0x54
{                                       // XREF: OperandStack/r
    Operand operands[10];               // XREF: IsAttachmentAllowedOnItemIndex+5B/r
    int operandCount;                   // XREF: GetAttachmentsFormatted+4D/r
};

struct OperandStack // sizeof=0x13B4
{
    OperandList stack[60];
    int numOperandLists;
};

bool __cdecl IsVisible(char flags);
char __cdecl GetTwoOperands(OperandStack *dataStack, Operand *data1, Operand *data2);
char __cdecl GetOperand(OperandStack *dataStack, Operand *data);
char __cdecl GetOperandList(OperandStack *dataStack, OperandList *list);
char *__cdecl GetSourceString(Operand operand);
double __cdecl GetSourceFloat(Operand *source);
operandInternalDataUnion __cdecl GetSourceInt(Operand *source);
void __cdecl GetDvarValue(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl AddOperandToStack(OperandStack *dataStack, Operand *data);
char *__cdecl CopyTempString(const char *string);
const char *__cdecl GetNameForValueType(expDataType valType);
void __cdecl GetDvarStringValue(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetDvarBoolValue(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetDvarIntValue(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetDvarFloatValue(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetLocalVarStringValue(int localClientNum, itemDef_s *item, OperandStack *dataStack);
UILocalVarContext *__cdecl GetLocalVar(int localClientNum, Operand *source);
void __cdecl GetLocalVarBoolValue(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetLocalVarIntValue(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetLocalVarFloatValue(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetSinValue(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCosValue(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetMilliseconds(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetPlayerField(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetOtherTeamField(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetFieldForTeam(int localClientNum, team_t team, Operand *fieldName, Operand *result);
void __cdecl GetTeamField(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetTeamMarinesField(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetTeamOpForField(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetUIActive(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsViewportLarge(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetFlashbanged(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetScoped(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl InKillcam(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsDualWield(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsFuelWeapon(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetPartyMissingMapPackError(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsSelectingLocationalKillstreak(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl isInGuidedMissile(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsInGame(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsSelectingAirstrike(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsSelectingArtillery(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsSelectingNapalm(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsSelectingMortar(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsSelectingComlink(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetScoreboardVisible(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetIsIntermission(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetIsSuperUser(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetIsAutoJoinDevUser(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetSelectingLocation(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl ShowZombieMap(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl PrivatePartyHostInLobby(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl AloneInPrivateParty(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl AloneInPrivatePartyIgnoreSplitscreen(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl AloneInLobby(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl AloneInLobbyIgnoreSplitscreen(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GameHost(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl InLobby(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl InPrivateParty(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsVisibilityBitSet(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl PrivatePartyHost(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl Splitscreen(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl SplitscreenHost(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl SplitscreenNum(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsCinematicFinished(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsItemLocked(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsItemLockedForAll(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCurrentClanTagFeature(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsClanTagFeatureLocked(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetClanTagFeatureCost(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetClanTagFeatureName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsClanTagFeaturePurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetClanTagFeaturePlevel(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsItemDualWieldLocked(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsItemDualWieldPurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsItemNew(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsItemGroupNew(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetAttachmentsFormatted(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetAttachmentName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetAttachmentImage(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetAttachmentDesc(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsAttachmentAllowedOnItemIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetFaceCamoIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetWeaponOptionImage(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetWeaponOptionName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetWeaponOptionGroupIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetPooledFileDetails(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl CanRateFilmInTheater(int locaClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetDemoFileID(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetFileShareRating(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetFileShareTotalVotes(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetAutoJoinLobbyStatus(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCounterTotal(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetDownloadProgress(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetUploadProgress(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsPremiumSubscriber(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetUploadTimeRemaining(int localClientNum, itemDef_s *item, OperandStack *dataStack);
char *__cdecl GetLocalizedTimeRemaining(int secondsRemaining);
void __cdecl ShowBusyDotsIndicator(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl HostMigrationWaitingForPlayers(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCurrentItemOption(int localClientNum, itemDef_s *item, OperandStack *dataStack);
int __cdecl GetOptionFromIndexAndGroupDvar();
void __cdecl GetWeaponOptionCost(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetWeaponOptionUnlockLvl(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetWeaponOptionUnlockPLevel(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetItemRef(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetItemName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetItemImage(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetItemUnlockLevel(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetItemUnlockPLevel(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetItemCost(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetItemDualWieldCost(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetItemSellPrice(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetItemCount(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetItemGroup(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetItemDesc(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetItemIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetItemDualWieldIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetItemDualWieldBaseIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetDefaultClassSlot(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetItemAttachment(int localClientNum, itemDef_s *item, OperandStack *dataStack);
operandInternalDataUnion __cdecl GetOperandValueInt(Operand *source);
void __cdecl GetCurrentItemMultiText(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetClanMOTD(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetNumLives(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl PlayersAlive(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetPlayerStatByName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetLowestLocalCP(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsWagerServer(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetContextHeight(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetKeyBinding(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetActionSlotUsable(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetHudFade(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsMenuOpen(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl WritingData(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetAdsJavelin(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetWeapLockBlink(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetWeapAttackTop(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetWeapAttackDirect(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl ToUpper(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetPlaylistName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl SecondsAsTimeDisplay(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl SecondsAsCountdownDisplay(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetTimeLeft(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetGameTime(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetBombTime(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetGametypeObjective(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetPlaylistMaxPartySize(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetGametypeName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetGametypeInternal(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetScore(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetGameMessageWindowActive(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl UI_GetOnlineFriendCount(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetFollowing(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl CanSpecCycle(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl CanSpecFree(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetClientInPlace(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetPlaceWithTiesForScore(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemLayerName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemLayerState(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemLayerCanOutline(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemLayerCanDuplicate(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemSelectedLayer(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemLayerCost(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemLayerUnlockLevel(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemIconName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemIconUnlockDesc(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemIconCost(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemIconState(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemPurchasedLayerCount(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemIsModified(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemBackgroundCount(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemBackgroundIsLocked(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemBackgroundIsClassified(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemBackgroundIsPurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemBackgroundMaterial(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemPlayerBackgroundMaterial(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemSelectedBackground(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemBackgroundName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemBackgroundUnlockDesc(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemStateDisplay(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemCategoryDisplay(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemFilterCount(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemFilterIconID(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl EmblemBackgroundCost(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GridMove(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetClientName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl ToOrdinal(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetScoreForClient(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsDemoPlaying(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsDemoClipRecording(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsDemoClipPlaying(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsDemoMovieRendering(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsDemoThirdPersonCamera(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsDemoMovieCamera(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsDisplayingPartyScoreboard(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetDemoTitleName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetDemoTitleDescription(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetDemoAuthor(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetDemoTimeInfo(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetDemoDuration(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetDemoSegmentCount(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetDemoSegmentInformation(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsClipModified(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl CanStartDemoPlayback(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetDemoSaveScreenName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetDemoSaveScreenDescription(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetTheaterFilmNotSelectedMessage(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsCurrentSortedItemEquipped(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetNumSortedItemsEquipped(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsCurrentItemEquippedInAnyCustomClass(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsItemEquipped(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsCurrentItemPurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsProItemVersionUnlocked(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsProItemVersion(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetProItemVersionCost(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsItemPurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsItemAttachmentPointPurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsCurrentItemAttachmentPurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsCurrentItemOptionPurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetItemOptionByName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsItemOptionPurchasedByName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl ItemHasDualWield(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl ItemIsDualWield(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCurrentItemCost(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCurrentItemSellPrice(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsCurrentItemClassified(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCurrentItemClassifiedHintText(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCurrentItemAttachmentCost(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCurrentItemName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCurrentItemIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCurrentItemAttachmentName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCurrentItemAttachmentDesc(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCurrentItemAttachmentNum(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCurrentItemAttachmentPoint(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCurrentItemNumAttachments(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetItemNumAttachmentsEquipped(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetItemEquippedAttachment(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetNumItemAttachmentsWithAttachPoint(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl PlaylistPlayerCount(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl CategoryPlayerCountForPlaylist(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl TotalPlayersInPlaylists(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetPlayersRegisteredOnline(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetUserTagFromIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl CanShowContentFromUser(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetUserFileRating(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl CanRenderClip(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetFileShareFilterList(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetLiveGroupCount(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetBaseLbMenuName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetLeaderboardMinReqText(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetPreviousGameType(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl HowManyReadiesNeeded(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsExtraCamActive(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCurrentWeapon(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsPlayerJoinable(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsPlayerInvitable(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl ClanMemberRankIsAtleast(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsExtraCamStatic(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetFlagCarrierForTeam(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetFlagStatusForTeam(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl HasTacticalMaskOverlay(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl InVehicle(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetNumActiveContracts(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCurrentContractIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetContractName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetContractDesc(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetContractProgress(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetContractRequiredCount(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetContractCost(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetTimeUntilNewContracts(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsContractActive(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsContractInProgress(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsContractExpired(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsContractComplete(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsContractLocked(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetTimesContractPurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetTimesContractPurchasable(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetContractCooldownTime(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetRemainingContractCooldownTime(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetContractUnlockLevel(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetContractRewardText(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetContractExpirationType(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsTaskInProgress(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetContractExpirationData(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetContractCombatTimeLeft(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetIndexForActiveContract(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetIndexForNthActiveContract(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetClanDateFounded(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetClanName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsGroup(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetPlayerCardTitle(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetClanTagAndName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetXUID(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetSellText(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetItemPrice(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetSelfGamertag(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetRankByXUID(int localClientNum, itemDef_s *item, OperandStack *dataStack);
int __cdecl GetRankByXUID_Internal(int controllerIndex, unsigned __int64 xuid);
void __cdecl GetPrestigeByXUID(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCodpointsByXUID(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetDisplayLevelByXUID(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetMutedStatus(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsProfileSignedIn(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsSignedIn(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsSignedInToLive(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl AnySignedIn(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl AnySignedInToLiveAndStatsFetched(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl AreStatsFetched(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl AreContractsFetched(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsTimeSynced(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsContentRatingAllowed(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsOnlineFetchingDone(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetUIRect(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetDStat(int localClientNum, itemDef_s *item, OperandStack *dataStack);
char __cdecl GetSearchState(ddlState_t *searchState, OperandList *operandList, int listStartIndex);
void __cdecl GetDStatForPreviousMatch(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsStableStatsBufferInitialized(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetStatForFriendOrSelf(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetSortedStatsForFriendOrSelf(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetToastPopupWidth(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetToastPopupIcon(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetToastPopupTitle(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetToastPopupDescription(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetFloatAsFormattedString(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl ChangeSortedStatsPivot(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl CanScrollUpOrDown(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCurrentScrollBarPosition(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCombatRecordInfoBarWidth(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCombatRecordInfoBarText(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCopyClassDialogTitle(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCopyClassConfirmationText(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCopyCustomGametypeClassDialogTitle(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCopyCustomGametypeClassConfirmationText(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCombatRecordInfoBarTagText(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCombatRecordHistogramHeight(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCombatRecordPieChartText(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCombatRecordMinMaxScore(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCombatRecordFailedContracts(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCombatRecordLockedString(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetNumWagerMatchesPlayed(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetNumWagerMatchesWon(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetIndexIntoMatchScoreboard(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetScoreboardColumnHeader(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetWagerPlaceForMatchScoreboard(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetWagerGametypeNameFromEnum(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetAfterActionReportAwardsInfo(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetNumPersonalBests(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetPersonalBestName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetPersonalBestPrefix(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetPersonalBestValue(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetPersonalBestDelta(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetNumWeaponUnlocks(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetNumFeatureUnlocks(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetNumStatsMilestones(int localClientNum, itemDef_s *item, OperandStack *dataStack);
const char *__cdecl GetStatsMilestoneData(
                milestoneTableColumns_t comparisonColumn1,
                const char *value1,
                milestoneTableColumns_t comparisonColumn2,
                const char *value2,
                milestoneTableColumns_t comparisonColumn3,
                const char *value3,
                milestoneTableColumns_t returnColumn);
void __cdecl GetStatsMilestoneValue(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetStatsMilestoneName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
char __cdecl GetCurrentIndex(
                int localClientNum,
                itemDef_s *item,
                OperandStack *dataStack,
                Operand *result,
                int *challengeNum,
                operandInternalDataUnion *milestoneType,
                char *challengeType);
void __cdecl GetCurrentChallengeXpReward(int localClientNum, itemDef_s *item, OperandStack *dataStack);
char __cdecl GetCurrentIndexOfHighlightedFeeder(
                int localClientNum,
                itemDef_s *item,
                OperandStack *dataStack,
                Operand *result,
                int *challengeNum,
                int *milestoneType,
                char *name);
void __cdecl IsCurrentChallengeItemClassified(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetXpReward(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCurrentChallengeCpReward(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCpReward(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCurrentChallengeProgress(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetNumChallenges(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetGameInvitesCount(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetChallengeProgressString(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetLbTypeWithButtons(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl PrepareProgressDescription(int milestoneType, char *outputString, challengeInfo_t *challenge);
void __cdecl IsItemUnlocked(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsChallengeItemPurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsWeaponItemPurchased(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsWeaponItemClassified(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsWeaponItemUnlocked(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetProgressString(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetDescription(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetWeaponName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetChallengeAttachmentName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetChallengeName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetChallengeDescription(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetLBFilter(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetLBTypeByDuration(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetUnlockedWeaponItemIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetUnlockedFeatureItemIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetTotalMatchesPlayed(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl NeedToPerformCommunitySearch(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsFileshareDataSummaryValid(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetFileshareRecentGamesCount(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetFileshareGameType(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetFileshareGameTypeName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetFileshareGameMap(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetFileshareGameMapName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetFileshareGameDate(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetFileshareFileName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCacFactionNameWithButtons(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetFileshareFileSize(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetFileshareFileId(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetMySlotInfo(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetImageWidth(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetTextWidth(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetTextHeight(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl HasFocus(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl MenuHasFocus(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetFeederData(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetRank(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetPrestige(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetComposite(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsCompositeValid(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl TableLookup(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl StatsTableLookup(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetClassBonus(int localClientNum, itemDef_s *item, OperandStack *dataStack);
int __cdecl GetClassBonusInternal(int localClientNum, const StringTable *table, const char *className);
int __cdecl FindClassBonusMatches(
                int localClientNum,
                const StringTable *table,
                int *rowMatches,
                int numRows,
                const char *statName,
                const char *className,
                int column,
                int ignoreRow);
bool __cdecl IsClassBonusMatch(const char *tableData, const char *playerData);
const char *__cdecl ClassBonusStatQuery(int controllerIndex, const char *statName, const char *className);
void __cdecl GetCustomClassLoadoutItem(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCustomClassModifier(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCustomClassName(int localClienTNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetCACItemIndex(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetMachineID(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetLocalClientNum(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsPrimaryLocalClient(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl IsFriendFromXuid(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl CanSwitchToLobby(int locaClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetFeederCount(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetMapIndexByName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetGamemodeIndexByName(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl ServerSort(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl ServerSortDirection(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetServerCounts(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RandomIntRange(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetSelectedEmblemLayer(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl MinValue(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl MaxValue(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl Select(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl Choose(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl LocalizeString(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl LocalizationError(const char *errorMessage);
void __cdecl ValidateLocalizedStringRef(const char *token, int tokenLen);
void __cdecl RPN_FUNC_TOINT(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RPN_FUNC_TOSTRING(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RPN_FUNC_TOFLOAT(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RPN_FUNC_ISCLANMEMBER(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RPN_OP_NOOP(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RPN_FUNC_INVALID(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RPN_OP_NOT(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl LogicalNot(int localClientNum, Operand *source, Operand *result);
void __cdecl RPN_OP_BITWISENOT(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl BitwiseNot(int localClientNum, Operand *source, Operand *result);
void __cdecl RPN_OP_BITSHIFTLEFT(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl BitShiftLeft(int localClientNum, Operand *source, Operand *bitsSource, Operand *result);
void __cdecl RPN_OP_BITSHIFTRIGHT(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl BitShiftRight(int localClientNum, Operand *source, Operand *bitsSource, Operand *result);
void __cdecl RPN_OP_NEGATE(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RPN_OP_MULTIPLY(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl MakeStringOperandNumber(Operand *op);
void __cdecl RPN_OP_DIVIDE(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RPN_OP_MODULUS(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RPN_OP_ADD(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RPN_OP_LESSTHAN(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RPN_OP_LESSTHANEQUALTO(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RPN_OP_GREATERTHAN(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RPN_OP_GREATERTHANEQUALTO(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RPN_OP_EQUALS(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RPN_OP_NOTEQUAL(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RPN_OP_AND(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RPN_OP_OR(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RPN_OP_BITWISEAND(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RPN_OP_BITWISEOR(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RPN_OP_SUBTRACT(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl RPN_OP_COMMA(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl ApproxEquals(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetLeaderboardValue(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl GetStatValue(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl Add64(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl Sub64(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl Div64(int localClientNum, itemDef_s *item, OperandStack *dataStack);
void __cdecl Mul64(int localClientNum, itemDef_s *item, OperandStack *dataStack);
int __cdecl Expression_GetIndexForFunction(int func);
int __cdecl Expression_GetFunctionForOp(int op);
char *__cdecl GetExpressionResultString(int localClientNum, itemDef_s *item, ExpressionStatement *statement);
char __cdecl EvaluateExpression(
                int localClientNum,
                itemDef_s *item,
                ExpressionStatement *statement,
                Operand *results,
                int resultCount);
bool __cdecl IsExpressionTrue(int localClientNum, itemDef_s *item, ExpressionStatement *statement);
operandInternalDataUnion __cdecl GetExpressionInt(int localClientNum, itemDef_s *item, ExpressionStatement *statement);
double __cdecl GetExpressionFloat(int localClientNum, itemDef_s *item, ExpressionStatement *statement);
char __cdecl GetExpressionStringInt(
                int localClientNum,
                itemDef_s *item,
                ExpressionStatement *statement,
                char **outString,
                operandInternalDataUnion *outInt);
char *__cdecl GetExpressionResultStringCompile(
                int localClientNum,
                itemDef_s *item,
                const char **text,
                void *compileBuffer,
                int compileBufferSize);
void __cdecl Expression_MapIndexToFunction(int index, const char *function);
char __cdecl Expression_Throttle();
void Expression_Error(const char *str, ...);
void Expression_Warn(const char *str, ...);
void Expression_TraceInternal(const char *str, ...);
