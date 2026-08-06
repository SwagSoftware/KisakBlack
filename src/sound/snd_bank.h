#pragma once
#include "snd.h"
#include "snd_radverb.h"

#define SND_TABLE_COUNT 9
#define SND_MAX_PATCHES 8
#define SND_MAX_BANKS 32
#define SND_BANK_INVALID_VALUE 0xFFFF

struct SndIndexEntry // sizeof=0x4
{
    unsigned __int16 value;
    unsigned __int16 next;
};

struct SndBank // sizeof=0x28
{                                       // XREF: XAssetPoolEntry<SndBank>/r
    const char *name;
    unsigned int aliasCount;
    snd_alias_list_t *alias;
    SndIndexEntry *aliasIndex;
    unsigned int packHash;
    unsigned int packLocation;
    unsigned int radverbCount;
    snd_radverb *radverbs;
    unsigned int snapshotCount;
    snd_snapshot *snapshots;
};

struct SndPatch // sizeof=0x14
{                                       // XREF: XAssetPoolEntry<SndPatch>/r
    char *name;
    unsigned int elementCount;
    unsigned int *elements;
    unsigned int fileCount;
    SoundFile *files;
};

void __cdecl SND_AddBank(SndBank *bank);
void __cdecl SND_AssertBankIndexValid(const SndBank *bank);
bool __cdecl SND_FindInIndex(unsigned int key, const SndBank *bank, snd_alias_list_t **result);
void __cdecl SND_RemoveBank(SndBank *bank);
void __cdecl SND_AddPatch(SndPatch *patch);
void __cdecl SND_RemovePatch(SndPatch *patch);
unsigned int __cdecl SND_AliasCount();
bool __cdecl SND_IsAliasNameLooping(const char *name);
snd_alias_list_t *__cdecl SND_AliasByIndex(unsigned int index);
snd_alias_list_t *__cdecl SND_FindAlias(const char *name);
snd_alias_list_t *__cdecl SND_FindAliasFromId(unsigned int hash);
snd_alias_list_t *__cdecl SND_BankAliasLookup(unsigned int key);
int __cdecl SND_FindAliasId(const char *name);
const snd_radverb *__cdecl SND_GetRadverb(unsigned int id);
const snd_snapshot *__cdecl SND_GetSnapshotById(unsigned int id);
const snd_snapshot *__cdecl SND_GetOcclusionSnapshot(const snd_snapshot *snap);
snd_alias_list_t *__cdecl SND_FindRowAlias(unsigned int id);
snd_group *__cdecl SND_FindRowGroup(unsigned int id);
snd_curve *__cdecl SND_FindRowCurve(unsigned int id);
snd_pan *__cdecl SND_FindRowPan(unsigned int id);
snd_context *__cdecl SND_FindRowContext(unsigned int id);
snd_master *__cdecl SND_FindRowMaster(unsigned int id);
void *__cdecl SND_FindAsset(unsigned int table, unsigned int id);
void __cdecl SND_PatchValue(unsigned int table, char *asset, unsigned int field, unsigned int value);
void __cdecl SND_PatchApply(const SndPatch *patch);
