#include "glass_load_obj.h"
#include "glass.h"
#include <universal/com_memory.h>
#include <universal/com_files.h>

bool glassesInited;
Glasses glasses;

Glasses *__cdecl GetGlasses_LoadObj()
{
    if ( !glassesInited )
    {
        glassesInited = 1;
        memset(&glasses, 0, sizeof(glasses));
        glasses.glasses = (Glass *)operator new[](0x1E460u);
        glasses.numGlasses = 0;
        glasses.name = 0;
    }
    return &glasses;
}

Glasses *__cdecl GetGlasses_FastFile()
{
    XAssetHeader header; // [esp+0h] [ebp-4h] BYREF

    if ( DB_GetAllXAssetOfType(ASSET_TYPE_GLASSES, &header, 1) <= 0 )
        return 0;
    else
        return (Glasses *)header.xmodelPieces;
}

Glasses *__cdecl GetGlasses()
{
    bool v1; // [esp+4h] [ebp-8h]

    v1 = fs_gameDirVar && *(_BYTE *)fs_gameDirVar->current.integer;
    if ( v1 || !useFastFile->current.enabled )
        return (Glasses *)((int (__cdecl *)(Glasses *(__cdecl *)()))GetGlasses_LoadObj)(GetGlasses_LoadObj);
    else
        return (Glasses *)((int (__cdecl *)(Glasses *(__cdecl *)()))GetGlasses_FastFile)(GetGlasses_FastFile);
}

