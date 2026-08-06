#include "snd_bank.h"
#include <win32/win_common.h>

#include <cstring>
#include "snd_utils.h"
#include "snd_log.h"
#include <qcommon/common.h>
#include "snd_db.h"

void *(__cdecl *const SND_FIND_ROW[SND_TABLE_COUNT])(unsigned int) =
{
  (void*(*)(unsigned int))SND_FindRowAlias,
  (void*(*)(unsigned int))SND_FindRowGroup,
  (void*(*)(unsigned int))SND_FindRowCurve,
  (void*(*)(unsigned int))SND_FindRowPan,
  (void*(*)(unsigned int))RETURN_ZERO32,
  (void*(*)(unsigned int))RETURN_ZERO32,
  (void*(*)(unsigned int))SND_FindRowContext,
  (void*(*)(unsigned int))RETURN_ZERO32,
  (void*(*)(unsigned int))SND_FindRowMaster
};

static const char *SND_TABLE_NAMES[SND_TABLE_COUNT] =
{
  "alias",
  "group",
  "curve",
  "pan",
  "snapshot_group",
  "snapshot",
  "context",
  "radverb",
  "master"
};

static unsigned int SND_METADATA_FIELD_COUNT[SND_TABLE_COUNT] = { 53u, 6u, 18u, 8u, 1u, 73u, 9u, 17u, 37u };


static SndBank *g_snd_banks[SND_MAX_BANKS];
static unsigned int g_snd_bankCount;
static SndPatch *g_snd_patches[SND_MAX_PATCHES];
static unsigned int g_snd_patchCount;

void __cdecl SND_AddBank(SndBank *bank)
{
    Sys_EnterCriticalSection(CRITSECT_SOUND_BANK);
    iassert(bank);
    iassert(g_snd_bankCount <= SND_MAX_BANKS);
    if ( !g_snd_bankCount )
        memset(g_snd_banks, 0, sizeof(g_snd_banks));
    g_snd_banks[g_snd_bankCount++] = bank;
    SND_AssertBankIndexValid(bank);
    for ( uint i = 0; i < SND_MAX_PATCHES; ++i )
    {
        if ( g_snd_patches[i] )
            SND_PatchApply(g_snd_patches[i]);
    }
    Sys_LeaveCriticalSection(CRITSECT_SOUND_BANK);
}

void __cdecl SND_AssertBankIndexValid(const SndBank *bank)
{
    for ( uint i = 0; i < bank->aliasCount; ++i )
    {
        iassert(bank->aliasIndex[i].value != SND_BANK_INVALID_VALUE);
        iassert(bank->aliasIndex[i].value != SND_BANK_INVALID_VALUE);
        iassert(bank->aliasIndex[i].value < bank->aliasCount);
    }
    for ( uint i = 0; i < bank->aliasCount; ++i )
    {
        snd_alias_list_t *value = NULL;
        iassert(SND_FindInIndex(bank->alias[i].id, bank, &value));
        iassert(value);
        iassert(value == bank->alias + i);
    }
}

bool __cdecl SND_FindInIndex(unsigned int key, const SndBank *bank, snd_alias_list_t **result)
{
    snd_alias_list_t *list; // [esp+0h] [ebp-8h]
    unsigned int idx; // [esp+4h] [ebp-4h]

    if ( !bank->aliasCount )
        return 0;

    for ( idx = key % bank->aliasCount; idx != SND_BANK_INVALID_VALUE; idx = bank->aliasIndex[idx].next )
    {
        iassert(bank->aliasIndex[idx].value != SND_BANK_INVALID_VALUE);

        list = &bank->alias[bank->aliasIndex[idx].value];
        if ( list->id == key )
        {
            *result = list;
            return true;
        }
    }
    return false;
}

void __cdecl SND_RemoveBank(SndBank *bank)
{
    unsigned int i; // [esp+0h] [ebp-8h]
    bool found; // [esp+7h] [ebp-1h]

    Sys_EnterCriticalSection(CRITSECT_SOUND_BANK);

    iassert(g_snd_bankCount);
    iassert(g_snd_bankCount <= SND_MAX_BANKS);

    found = 0;
    for ( i = 0; i < SND_MAX_BANKS; ++i )
    {
        if ( found || g_snd_banks[i] != bank )
        {
            if ( found )
            {
                //dword_A0B1C6C[i] = (int)g_snd_banks[i];
                g_snd_banks[i - 1] = g_snd_banks[i];
                g_snd_banks[i] = 0;
            }
        }
        else
        {
            g_snd_banks[i] = 0;
            found = 1;
        }
    }
    if ( found )
        --g_snd_bankCount;
    Sys_LeaveCriticalSection(CRITSECT_SOUND_BANK);
}

void __cdecl SND_AddPatch(SndPatch *patch)
{
    Sys_EnterCriticalSection(CRITSECT_SOUND_BANK);

    iassert(patch);
    iassert(g_snd_patchCount <= SND_MAX_PATCHES);

    if ( !g_snd_patchCount )
    {
        g_snd_patches[0] = 0;
        g_snd_patches[1] = 0;
        g_snd_patches[2] = 0;
        g_snd_patches[3] = 0;
        g_snd_patches[4] = 0;
        g_snd_patches[5] = 0;
        g_snd_patches[6] = 0;
        g_snd_patches[7] = 0;
    }

    g_snd_patches[g_snd_patchCount++] = patch;
    Sys_LeaveCriticalSection(CRITSECT_SOUND_BANK);
}

void __cdecl SND_RemovePatch(SndPatch *patch)
{
    unsigned int i; // [esp+0h] [ebp-8h]
    bool found; // [esp+7h] [ebp-1h]

    Sys_EnterCriticalSection(CRITSECT_SOUND_BANK);
    iassert(g_snd_patchCount);
    iassert(g_snd_patchCount <= SND_MAX_PATCHES);

    found = 0;
    for ( i = 0; i < SND_MAX_PATCHES; ++i )
    {
        if ( found || g_snd_patches[i] != patch )
        {
            if ( found )
            {
                g_snd_patches[i - 1] = g_snd_patches[i];
                g_snd_patches[i] = 0;
            }
        }
        else
        {
            g_snd_patches[i] = 0;
            found = 1;
        }
    }
    if ( found )
        --g_snd_patchCount;
    Sys_LeaveCriticalSection(CRITSECT_SOUND_BANK);
}

unsigned int __cdecl SND_AliasCount()
{
    uint total = 0;
    Sys_EnterCriticalSection(CRITSECT_SOUND_BANK);
    for ( int i = 0; i < SND_MAX_BANKS; ++i )
    {
        if ( g_snd_banks[i] )
            total += g_snd_banks[i]->aliasCount;
    }
    Sys_LeaveCriticalSection(CRITSECT_SOUND_BANK);
    return total;
}

bool __cdecl SND_IsAliasNameLooping(const char *name)
{
    snd_alias_list_t *list = SND_FindAlias(name);
    return list && (list->head->flags & 1) != 0;
}

snd_alias_list_t *__cdecl SND_AliasByIndex(unsigned int index)
{
    uint total = 0;
    Sys_EnterCriticalSection(CRITSECT_SOUND_BANK);
    for ( int i = 0; i < SND_MAX_BANKS; ++i )
    {
        if ( g_snd_banks[i] )
        {
            if ( g_snd_banks[i]->aliasCount + total > index )
            {
                snd_alias_list_t *list = &g_snd_banks[i]->alias[index - total];
                Sys_LeaveCriticalSection(CRITSECT_SOUND_BANK);
                return list;
            }
            total += g_snd_banks[i]->aliasCount;
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_SOUND_BANK);
    return 0;
}

snd_alias_list_t *__cdecl SND_FindAlias(const char *name)
{
    if ( !SND_Active() )
        return NULL;

    if ( !name || !*name )
        return NULL;

    int hash = SND_HashName(name);
    snd_alias_list_t *list = SND_FindAliasFromId(hash);

    if ( !list || !list->count )
        return NULL;

    iassert(!I_stricmp(name, list->name));

    return list;
}

snd_alias_list_t *__cdecl SND_FindAliasFromId(unsigned int hash)
{
    if ( !SND_Active() || !hash )
        return NULL;

    Sys_EnterCriticalSection(CRITSECT_SOUND_LOOKUP_CACHE);
    snd_alias_list_t *list = SND_BankAliasLookup(hash);
    Sys_LeaveCriticalSection(CRITSECT_SOUND_LOOKUP_CACHE);
    return list;
}

snd_alias_list_t *__cdecl SND_BankAliasLookup(unsigned int key)
{
    PROF_SCOPED("SND_BankAliasLookup");

    snd_alias_list_t *list = NULL;
    Sys_EnterCriticalSection(CRITSECT_SOUND_BANK);
    for ( uint i = 0; i < g_snd_bankCount && !SND_FindInIndex(key, g_snd_banks[g_snd_bankCount - i - 1], &list); ++i )
        ;
    Sys_LeaveCriticalSection(CRITSECT_SOUND_BANK);
    return list;
}

int __cdecl SND_FindAliasId(const char *name)
{
    int id = SND_HashName(name);
    if ( id && !SND_FindAliasFromId(id) && SND_Active() )
        SND_LogRegisterString(name, id);
    return id;
}

const snd_radverb *__cdecl SND_GetRadverb(unsigned int id)
{
    unsigned int r; // [esp+0h] [ebp-10h]
    SndBank *bank; // [esp+4h] [ebp-Ch]
    unsigned int i; // [esp+8h] [ebp-8h]
    const snd_radverb *radverb; // [esp+Ch] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_SOUND_BANK);
    radverb = 0;
    for ( i = 0; i < g_snd_bankCount && !radverb; ++i )
    {
        bank = g_snd_banks[g_snd_bankCount - i - 1];
        if ( bank )
        {
            for ( r = 0; r < bank->radverbCount; ++r )
            {
                if ( bank->radverbs[r].id == id )
                {
                    radverb = &bank->radverbs[r];
                    break;
                }
            }
        }
    }
    if ( !radverb && g_snd.defaultHash != id )
        radverb = SND_GetRadverb(g_snd.defaultHash);
    Sys_LeaveCriticalSection(CRITSECT_SOUND_BANK);
    return radverb;
}

const snd_snapshot *__cdecl SND_GetSnapshotById(unsigned int id)
{
    unsigned int r; // [esp+0h] [ebp-10h]
    SndBank *bank; // [esp+4h] [ebp-Ch]
    unsigned int i; // [esp+8h] [ebp-8h]
    const snd_snapshot *snapshot; // [esp+Ch] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_SOUND_BANK);
    snapshot = 0;
    for ( i = 0; i < g_snd_bankCount && !snapshot; ++i )
    {
        bank = g_snd_banks[g_snd_bankCount - i - 1];
        if ( bank )
        {
            for ( r = 0; r < bank->snapshotCount; ++r )
            {
                if ( bank->snapshots[r].id == id )
                {
                    snapshot = &bank->snapshots[r];
                    break;
                }
            }
        }
    }
    if ( !snapshot && g_snd.defaultHash != id )
    {
        Com_PrintError(9, "missing snapshot\n");
        snapshot = SND_GetSnapshotById(g_snd.defaultHash);
    }
    Sys_LeaveCriticalSection(CRITSECT_SOUND_BANK);
    return snapshot;
}

const snd_snapshot *__cdecl SND_GetOcclusionSnapshot(const snd_snapshot *snap)
{
    const snd_snapshot *osnap; // [esp+0h] [ebp-4h]

    if ( snap && (osnap = SND_GetSnapshotById(snap->occlusionId)) != 0 )
        return osnap;
    else
        return SND_GetSnapshotById(g_snd.defaultHash);
}

snd_alias_list_t *__cdecl SND_FindRowAlias(unsigned int id)
{
    return SND_FindAliasFromId(id);
}

snd_group *__cdecl SND_FindRowGroup(unsigned int id)
{
    if ( g_snd.global_constants )
    {
        for ( int i = 0; i < g_snd.global_constants->groupCount; ++i )
        {
            if ( g_snd.global_constants->groups[i].id == id )
                return &g_snd.global_constants->groups[i];
        }
    }

    return NULL;
}

snd_curve *__cdecl SND_FindRowCurve(unsigned int id)
{
    if ( g_snd.global_constants )
    {
        for ( int i = 0; i < g_snd.global_constants->curveCount; ++i )
        {
            if ( g_snd.global_constants->curves[i].id == id )
                return &g_snd.global_constants->curves[i];
        }
    }

    return NULL;
}

snd_pan *__cdecl SND_FindRowPan(unsigned int id)
{
    if ( g_snd.global_constants )
    {
        for ( int i = 0; i < g_snd.global_constants->panCount; ++i )
        {
            if ( g_snd.global_constants->pans[i].id == id )
                return &g_snd.global_constants->pans[i];
        }
    }

    return NULL;
}

snd_context *__cdecl SND_FindRowContext(unsigned int id)
{
    for ( int i = 0; i < g_snd.global_constants->contextCount; ++i )
    {
        if ( g_snd.global_constants->contexts[i].type == id )
            return &g_snd.global_constants->contexts[i];
    }

    return NULL;
}

snd_master *__cdecl SND_FindRowMaster(unsigned int id)
{
    if ( g_snd.global_constants )
    {
        for ( int i = 0; i < g_snd.global_constants->masterCount; ++i )
        {
            if ( g_snd.global_constants->masters[i].id == id )
                return &g_snd.global_constants->masters[i];
        }
    }

    return NULL;
}

void *__cdecl SND_FindAsset(unsigned int table, unsigned int id)
{
    iassert(table < SND_TABLE_COUNT);

    if ( table >= SND_TABLE_COUNT )
        return 0;
    else
        return SND_FIND_ROW[table](id);
}

void __cdecl SND_PatchValue(unsigned int table, char *asset, unsigned int field, unsigned int value)
{
    iassert(table < SND_TABLE_COUNT);

    if ( asset )
    {
        const snd_csv_entry_t *meta = &SND_TABLE_METADATA[table][field];
        void *ptr = &asset[meta->offset];
        switch ( meta->type )
        {
            case SND_CSV_FLOAT:
                *(float*)ptr = (float)value / 65535.0;
                break;
            case SND_CSV_INT:
                *(uint *)ptr = value;
                break;
            case SND_CSV_ENUM:
                *(uint *)ptr = value;
                break;
            case SND_CSV_FLAG:
                *(uint *)ptr = value;
                break;
            case SND_CSV_DBSPL:
                *(ushort *)ptr = (int)(SND_dBSPLToLinear(value) * 65535.0);
                break;
            case SND_CSV_HASH:
                *(uint *)ptr = value;
                break;
            case SND_CSV_BYTE:
                *(byte *)ptr = value;
                break;
            case SND_CSV_ENUM_BYTE:
                *(byte *)ptr = value;
                break;
            case SND_CSV_SHORT:
                *(ushort *)ptr = value;
                break;
            case SND_CSV_USHORT:
                *(ushort *)ptr = value;
                break;
            default:
                return;
        }
    }
}

void __cdecl SND_PatchApply(const SndPatch *patch)
{
    int a; // [esp+0h] [ebp-28h]
    char *asset; // [esp+8h] [ebp-20h]
    unsigned int field; // [esp+Ch] [ebp-1Ch]
    unsigned int value; // [esp+10h] [ebp-18h]
    unsigned int f; // [esp+14h] [ebp-14h]
    unsigned int table; // [esp+18h] [ebp-10h]
    unsigned int fieldCount; // [esp+1Ch] [ebp-Ch]
    unsigned int id; // [esp+20h] [ebp-8h]
    unsigned int i; // [esp+24h] [ebp-4h]

    for ( i = 0; i < patch->elementCount; i++)
    {
        table = HIWORD(patch->elements[i]);
        fieldCount = LOWORD(patch->elements[i]);

        iassert(table < SND_TABLE_COUNT);

        i++;
        iassert(i < patch->elementCount);

        if ( i >= patch->elementCount )
            break;

        id = patch->elements[i];

        for ( f = 0; f < fieldCount; ++f )
        {
            i++;
            iassert(i < patch->elementCount);
            if ( i >= patch->elementCount )
                break;

            field = HIWORD(patch->elements[i]);
            value = LOWORD(patch->elements[i]);

            iassert(SND_METADATA_FIELD_COUNT[table]);

            if ( table < SND_TABLE_COUNT && field < SND_METADATA_FIELD_COUNT[table] )
            {
                asset = (char *)SND_FindAsset(table, id);

                snd_alias_list_t *list = (snd_alias_list_t *)asset;
                //if ( table )
                if ( table || !list ) // LWSS: this version of the if() is seen in retail MP exe. The 1st soundbank (table 0) is loaded as all ZERO except the name? 
                {
                    SND_PatchValue(table, asset, field, value);
                }
                else
                {
                    for (a = 0; a < list->count; ++a)
                        SND_PatchValue(0, (char *)&list->head[a], field, value);
                }
            }
        }
    }
}

