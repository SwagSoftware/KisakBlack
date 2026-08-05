#include "DynEntity_coll.h"

#include <string.h>
#include <qcommon/cm_load.h>
#include "DynEntity_load_obj.h"
#include "DynEntity_client.h"

DynEntityCollWorld dynEntCollWorlds[4];

DynEntityCollSector *__cdecl DynEnt_GetCollSector(DynEntityCollType collType, unsigned int sectorIndex)
{
    if ( !sectorIndex
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 66, 0, "%s", "sectorIndex") )
    {
        __debugbreak();
    }
    if ( sectorIndex >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                    67,
                    0,
                    "%s\n\t(sectorIndex) = %i",
                    "(sectorIndex < 1024)",
                    sectorIndex) )
    {
        __debugbreak();
    }
    return &DynEnt_GetCollWorld(collType)->sectors[sectorIndex];
}

DynEntityCollWorld *__cdecl DynEnt_GetCollWorld(DynEntityCollType collType)
{
    if ( (unsigned int)collType >= DYNENT_COLL_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                    54,
                    0,
                    "collType doesn't index DYNENT_COLL_COUNT\n\t%i not in [0, %i)",
                    collType,
                    4) )
    {
        __debugbreak();
    }
    return &dynEntCollWorlds[collType];
}

void __cdecl DynEnt_ClearCollWorld(DynEntityCollType collType)
{
    unsigned __int16 sectorIndex; // [esp+18h] [ebp-8h]
    DynEntityCollWorld *world; // [esp+1Ch] [ebp-4h]

    if ( cm.isInUse )
    {
        world = DynEnt_GetCollWorld(collType);
        memset(world, 0, sizeof(DynEntityCollWorld));
        CM_ModelBounds(0, world->mins, world->maxs);
        world->freeHead = 2;
        for ( sectorIndex = 2; sectorIndex < 0x3FFu; ++sectorIndex )
            world->sectors[sectorIndex].tree.u.parent = sectorIndex + 1;
        world->sectors[1].tree.axis = (float)(world->maxs[1] - world->mins[1]) >= (float)(world->maxs[0] - world->mins[0]);
        world->sectors[1].tree.dist = (float)(world->maxs[world->sectors[1].tree.axis]
                                                                                + world->mins[world->sectors[1].tree.axis])
                                                                * 0.5;
    }
}

void __cdecl DynEnt_UnlinkEntity(DynEntityCollType collType, unsigned __int16 dynEntId)
{
    unsigned __int16 EntityCount; // ax
    DynEntityServer *ServerEntity; // edx
    unsigned __int16 *p_flags; // [esp+8h] [ebp-28h]
    int contents; // [esp+Ch] [ebp-24h]
    DynEntityCollSector *sector; // [esp+10h] [ebp-20h]
    DynEntityDrawType drawType; // [esp+14h] [ebp-1Ch]
    DynEntityColl *scan; // [esp+18h] [ebp-18h]
    DynEntityColl *scana; // [esp+18h] [ebp-18h]
    unsigned __int16 parentSectorIndex; // [esp+1Ch] [ebp-14h]
    unsigned __int16 parentSectorIndexa; // [esp+1Ch] [ebp-14h]
    DynEntityColl *next; // [esp+20h] [ebp-10h]
    unsigned __int16 sectorIndex; // [esp+24h] [ebp-Ch]
    DynEntityCollWorld *world; // [esp+28h] [ebp-8h]
    DynEntityColl *dynEntColl; // [esp+2Ch] [ebp-4h]

    if ( dynEntId == 0xFFFF
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                    408,
                    0,
                    "%s",
                    "dynEntId != DYNENT_INVALID_ID") )
    {
        __debugbreak();
    }
    if ( dynEntId >= (unsigned int)DynEnt_GetEntityCount(collType) )
    {
        EntityCount = DynEnt_GetEntityCount(collType);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                        409,
                        0,
                        "dynEntId doesn't index DynEnt_GetEntityCount( collType )\n\t%i not in [0, %i)",
                        dynEntId,
                        EntityCount) )
            __debugbreak();
    }
    world = DynEnt_GetCollWorld(collType);
    dynEntColl = DynEnt_GetEntityColl(collType, dynEntId);
    sectorIndex = dynEntColl->sector;
    if ( dynEntColl->sector )
    {
        sector = &world->sectors[sectorIndex];
        dynEntColl->sector = 0;
        if ( !world->sectors[sectorIndex].entListHead
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                        421,
                        0,
                        "%s",
                        "sector->entListHead") )
        {
            __debugbreak();
        }
        if ( world->sectors[sectorIndex].entListHead - 1 == dynEntId )
        {
            world->sectors[sectorIndex].entListHead = dynEntColl->nextEntInSector;
        }
        else
        {
            for ( scan = DynEnt_GetEntityColl(collType, world->sectors[sectorIndex].entListHead - 1); ; scan = next )
            {
                next = DynEnt_GetEntityColl(collType, scan->nextEntInSector - 1);
                if ( next == dynEntColl )
                    break;
                if ( !scan->nextEntInSector
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                                439,
                                0,
                                "%s",
                                "scan->nextEntInSector") )
                {
                    __debugbreak();
                }
            }
            scan->nextEntInSector = dynEntColl->nextEntInSector;
        }
        while ( !sector->entListHead && !sector->tree.child[0] && !sector->tree.child[1] )
        {
            sector->contents = 0;
            if ( !sector->tree.u.parent )
            {
                if ( sectorIndex != 1
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                                449,
                                0,
                                "%s",
                                "sectorIndex == DYNENT_SECTOR_HEAD") )
                {
                    __debugbreak();
                }
                goto LABEL_35;
            }
            parentSectorIndex = sector->tree.u.parent;
            sector->tree.u.parent = world->freeHead;
            world->freeHead = sectorIndex;
            sector = &world->sectors[parentSectorIndex];
            if ( world->sectors[parentSectorIndex].tree.child[0] == sectorIndex )
            {
                world->sectors[parentSectorIndex].tree.child[0] = 0;
            }
            else
            {
                if ( world->sectors[parentSectorIndex].tree.child[1] != sectorIndex
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                                466,
                                0,
                                "%s",
                                "sector->tree.child[1] == sectorIndex") )
                {
                    __debugbreak();
                }
                world->sectors[parentSectorIndex].tree.child[1] = 0;
            }
            sectorIndex = parentSectorIndex;
        }
        while ( 1 )
        {
LABEL_35:
            contents = world->sectors[sector->tree.child[1]].contents | world->sectors[sector->tree.child[0]].contents;
            if ( sector->entListHead )
            {
                for ( scana = DynEnt_GetEntityColl(collType, sector->entListHead - 1);
                            ;
                            scana = DynEnt_GetEntityColl(collType, scana->nextEntInSector - 1) )
                {
                    contents |= scana->contents;
                    if ( !scana->nextEntInSector )
                        break;
                }
            }
            sector->contents = contents;
            parentSectorIndexa = sector->tree.u.parent;
            if ( !parentSectorIndexa )
                break;
            sector = &world->sectors[parentSectorIndexa];
        }
        drawType = (DynEntityDrawType)(collType & 1);
        if ( collType >= DYNENT_COLL_SERVER_FIRST )
        {
            ServerEntity = DynEnt_GetServerEntity(dynEntId, drawType);
            ServerEntity->flags &= ~2u;
        }
        else
        {
            p_flags = &DynEnt_GetClientEntity(dynEntId, drawType)->flags;
            *p_flags &= ~4u;
        }
    }
}

void __cdecl DynEnt_LinkEntity(
                DynEntityCollType collType,
                unsigned __int16 dynEntId,
                const float *absMins,
                const float *absMaxs)
{
    unsigned __int16 EntityCount; // ax
    DynEntityClient *ClientEntity; // eax
    DynEntityServer *ServerEntity; // edx
    float *v7; // [esp+Ch] [ebp-48h]
    float *v8; // [esp+10h] [ebp-44h]
    float *linkMaxs; // [esp+14h] [ebp-40h]
    float *linkMins; // [esp+18h] [ebp-3Ch]
    int contents; // [esp+20h] [ebp-34h]
    DynEntityCollSector *sector; // [esp+24h] [ebp-30h]
    DynEntityDrawType drawType; // [esp+28h] [ebp-2Ch]
    float dist; // [esp+2Ch] [ebp-28h]
    float mins[2]; // [esp+30h] [ebp-24h] BYREF
    const DynEntityDef *dynEntDef; // [esp+38h] [ebp-1Ch]
    unsigned __int16 sectorIndex; // [esp+3Ch] [ebp-18h]
    float maxs[2]; // [esp+40h] [ebp-14h] BYREF
    DynEntityCollWorld *world; // [esp+48h] [ebp-Ch]
    int axis; // [esp+4Ch] [ebp-8h]
    DynEntityColl *dynEntColl; // [esp+50h] [ebp-4h]

    if ( dynEntId == 0xFFFF
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                    526,
                    0,
                    "%s",
                    "dynEntId != DYNENT_INVALID_ID") )
    {
        __debugbreak();
    }
    if ( dynEntId >= (unsigned int)DynEnt_GetEntityCount(collType) )
    {
        EntityCount = DynEnt_GetEntityCount(collType);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                        527,
                        0,
                        "dynEntId doesn't index DynEnt_GetEntityCount( collType )\n\t%i not in [0, %i)",
                        dynEntId,
                        EntityCount) )
            __debugbreak();
    }
    if ( !absMins
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 528, 0, "%s", "absMins") )
    {
        __debugbreak();
    }
    if ( !absMaxs
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 529, 0, "%s", "absMaxs") )
    {
        __debugbreak();
    }
    world = DynEnt_GetCollWorld(collType);
    drawType = (DynEntityDrawType)(collType & 1);
    dynEntDef = DynEnt_GetEntityDef(dynEntId, drawType);
    dynEntColl = DynEnt_GetEntityColl(collType, dynEntId);
    contents = DynEnt_GetContents(dynEntDef);
    dynEntColl->contents = contents;
    while ( 1 )
    {
        mins[0] = world->mins[0];
        mins[1] = world->mins[1];
        maxs[0] = world->maxs[0];
        maxs[1] = world->maxs[1];
        for ( sectorIndex = 1; ; sectorIndex = sector->tree.child[1] )
        {
            while ( 1 )
            {
                sector = &world->sectors[sectorIndex];
                axis = world->sectors[sectorIndex].tree.axis;
                dist = sector->tree.dist;
                world->sectors[sectorIndex].contents |= contents;
                if ( absMins[axis] <= dist )
                    break;
                mins[axis] = dist;
                if ( !sector->tree.child[0] )
                    goto LABEL_25;
                sectorIndex = sector->tree.child[0];
            }
            if ( dist <= absMaxs[axis] )
                break;
            maxs[axis] = dist;
            if ( !sector->tree.child[1] )
                goto LABEL_25;
        }
        if ( sectorIndex == dynEntColl->sector )
        {
            linkMins = dynEntColl->linkMins;
            dynEntColl->linkMins[0] = *absMins;
            linkMins[1] = absMins[1];
            linkMins[2] = absMins[2];
            linkMaxs = dynEntColl->linkMaxs;
            dynEntColl->linkMaxs[0] = *absMaxs;
            linkMaxs[1] = absMaxs[1];
            linkMaxs[2] = absMaxs[2];
            return;
        }
LABEL_25:
        if ( !dynEntColl->sector )
            break;
        if ( sectorIndex == dynEntColl->sector )
            goto LABEL_30;
        DynEnt_UnlinkEntity(collType, dynEntId);
    }
    DynEnt_AddToCollSector(collType, dynEntId, sectorIndex);
LABEL_30:
    v8 = dynEntColl->linkMins;
    dynEntColl->linkMins[0] = *absMins;
    v8[1] = absMins[1];
    v8[2] = absMins[2];
    v7 = dynEntColl->linkMaxs;
    dynEntColl->linkMaxs[0] = *absMaxs;
    v7[1] = absMaxs[1];
    v7[2] = absMaxs[2];
    DynEnt_SortCollSector(collType, sectorIndex, mins, maxs);
    if ( collType >= DYNENT_COLL_SERVER_FIRST )
    {
        ServerEntity = DynEnt_GetServerEntity(dynEntId, drawType);
        ServerEntity->flags |= 2u;
    }
    else
    {
        ClientEntity = DynEnt_GetClientEntity(dynEntId, drawType);
        ClientEntity->flags |= 4u;
    }
}

void __cdecl DynEnt_AddToCollSector(
                DynEntityCollType collType,
                unsigned __int16 dynEntId,
                unsigned __int16 sectorIndex)
{
    unsigned __int16 EntityCount; // ax
    DynEntityCollSector *sector; // [esp+4h] [ebp-18h]
    unsigned __int16 *prevListIndex; // [esp+8h] [ebp-14h]
    DynEntityColl *dynEntColl; // [esp+18h] [ebp-4h]

    if ( dynEntId == 0xFFFF
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                    272,
                    0,
                    "%s",
                    "dynEntId != DYNENT_INVALID_ID") )
    {
        __debugbreak();
    }
    if ( dynEntId >= (unsigned int)DynEnt_GetEntityCount(collType) )
    {
        EntityCount = DynEnt_GetEntityCount(collType);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                        273,
                        0,
                        "dynEntId doesn't index DynEnt_GetEntityCount( collType )\n\t%i not in [0, %i)",
                        dynEntId,
                        EntityCount) )
            __debugbreak();
    }
    if ( !sectorIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                    275,
                    0,
                    "%s",
                    "sectorIndex") )
    {
        __debugbreak();
    }
    if ( sectorIndex >= 0x400u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                    276,
                    0,
                    "%s\n\t(sectorIndex) = %i",
                    "(sectorIndex < 1024)",
                    sectorIndex) )
    {
        __debugbreak();
    }
    sector = &DynEnt_GetCollWorld(collType)->sectors[sectorIndex];
    dynEntColl = DynEnt_GetEntityColl(collType, dynEntId);
    for ( prevListIndex = &sector->entListHead;
                (unsigned int)*prevListIndex - 1 <= dynEntId;
                prevListIndex = &DynEnt_GetEntityColl(collType, *prevListIndex - 1)->nextEntInSector )
    {
        ;
    }
    dynEntColl->sector = sectorIndex;
    dynEntColl->nextEntInSector = *prevListIndex;
    *prevListIndex = dynEntId + 1;
}

void __cdecl DynEnt_SortCollSector(
                DynEntityCollType collType,
                unsigned __int16 sectorIndex,
                const float *mins,
                const float *maxs)
{
    unsigned __int16 listIndex; // [esp+8h] [ebp-28h]
    float dist; // [esp+10h] [ebp-20h]
    const DynEntityDef *dynEntDef; // [esp+14h] [ebp-1Ch]
    DynEntityCollWorld *world; // [esp+18h] [ebp-18h]
    unsigned __int16 dynEntId; // [esp+1Ch] [ebp-14h]
    DynEntityColl *prevDynEntColl; // [esp+20h] [ebp-10h]
    int axis; // [esp+24h] [ebp-Ch]
    DynEntityColl *dynEntColl; // [esp+28h] [ebp-8h]
    unsigned __int16 childSectorIndex; // [esp+2Ch] [ebp-4h]

    if ( !sectorIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                    319,
                    0,
                    "%s",
                    "sectorIndex") )
    {
        __debugbreak();
    }
    if ( sectorIndex >= 0x400u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                    320,
                    0,
                    "%s\n\t(sectorIndex) = %i",
                    "(sectorIndex < 1024)",
                    sectorIndex) )
    {
        __debugbreak();
    }
    if ( !mins
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 322, 0, "%s", "mins") )
    {
        __debugbreak();
    }
    if ( !maxs
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 323, 0, "%s", "maxs") )
    {
        __debugbreak();
    }
    world = DynEnt_GetCollWorld(collType);
    axis = world->sectors[sectorIndex].tree.axis;
    dist = world->sectors[sectorIndex].tree.dist;
    prevDynEntColl = 0;
    listIndex = world->sectors[sectorIndex].entListHead;
    while ( listIndex )
    {
        dynEntId = listIndex - 1;
        dynEntDef = DynEnt_GetEntityDef(listIndex - 1, (DynEntityDrawType)(collType & 1));
        dynEntColl = DynEnt_GetEntityColl(collType, listIndex - 1);
        if ( dynEntColl->linkMins[axis] <= dist )
        {
            if ( dist > dynEntColl->linkMaxs[axis] )
            {
                childSectorIndex = world->sectors[sectorIndex].tree.child[1];
                if ( childSectorIndex )
                    goto LABEL_26;
                childSectorIndex = DynEnt_AllocCollSector(collType, mins, maxs);
                if ( childSectorIndex )
                {
                    world->sectors[sectorIndex].tree.child[1] = childSectorIndex;
                    world->sectors[childSectorIndex].tree.u.parent = sectorIndex;
                    goto LABEL_26;
                }
            }
            goto skipEntity_0;
        }
        childSectorIndex = world->sectors[sectorIndex].tree.child[0];
        if ( childSectorIndex )
        {
LABEL_26:
            listIndex = dynEntColl->nextEntInSector;
            if ( !prevDynEntColl
                && DynEnt_GetEntityColl(collType, world->sectors[sectorIndex].entListHead - 1) != dynEntColl
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                            378,
                            0,
                            "%s",
                            "prevDynEntColl || (DynEnt_GetEntityColl( collType, sector->entListHead - 1 ) == dynEntColl)") )
            {
                __debugbreak();
            }
            if ( prevDynEntColl
                && DynEnt_GetEntityColl(collType, prevDynEntColl->nextEntInSector - 1) != dynEntColl
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                            379,
                            0,
                            "%s",
                            "!prevDynEntColl || (DynEnt_GetEntityColl( collType, prevDynEntColl->nextEntInSector - 1) == dynEntColl)") )
            {
                __debugbreak();
            }
            DynEnt_AddToCollSector(collType, dynEntId, childSectorIndex);
            world->sectors[childSectorIndex].contents |= DynEnt_GetContents(dynEntDef);
            if ( prevDynEntColl )
                prevDynEntColl->nextEntInSector = listIndex;
            else
                world->sectors[sectorIndex].entListHead = listIndex;
        }
        else
        {
            childSectorIndex = DynEnt_AllocCollSector(collType, mins, maxs);
            if ( childSectorIndex )
            {
                world->sectors[sectorIndex].tree.child[0] = childSectorIndex;
                world->sectors[childSectorIndex].tree.u.parent = sectorIndex;
                goto LABEL_26;
            }
skipEntity_0:
            prevDynEntColl = dynEntColl;
            listIndex = dynEntColl->nextEntInSector;
        }
    }
}

unsigned __int16 __cdecl DynEnt_AllocCollSector(DynEntityCollType collType, const float *mins, const float *maxs)
{
    DynEntityCollSector *sector; // [esp+8h] [ebp-18h]
    float size[2]; // [esp+Ch] [ebp-14h]
    unsigned __int16 sectorIndex; // [esp+14h] [ebp-Ch]
    DynEntityCollWorld *world; // [esp+18h] [ebp-8h]
    unsigned __int16 axis; // [esp+1Ch] [ebp-4h]

    if ( !mins
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 228, 0, "%s", "mins") )
    {
        __debugbreak();
    }
    if ( !maxs
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 229, 0, "%s", "maxs") )
    {
        __debugbreak();
    }
    world = DynEnt_GetCollWorld(collType);
    sectorIndex = world->freeHead;
    if ( !sectorIndex )
        return 0;
    size[0] = *maxs - *mins;
    size[1] = maxs[1] - mins[1];
    axis = size[1] >= size[0];
    if ( size[size[1] >= size[0]] <= 512.0 )
        return 0;
    sector = &world->sectors[sectorIndex];
    if ( world->sectors[sectorIndex].contents
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                    245,
                    0,
                    "%s",
                    "!sector->contents") )
    {
        __debugbreak();
    }
    if ( sector->entListHead
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                    246,
                    0,
                    "%s",
                    "!sector->entListHead") )
    {
        __debugbreak();
    }
    world->freeHead = sector->tree.u.parent;
    sector->tree.axis = axis;
    sector->tree.dist = (float)(maxs[axis] + mins[axis]) * 0.5;
    if ( sector->tree.child[0]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                    253,
                    0,
                    "%s",
                    "!sector->tree.child[0]") )
    {
        __debugbreak();
    }
    if ( sector->tree.child[1] )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                        254,
                        0,
                        "%s",
                        "!sector->tree.child[1]") )
            __debugbreak();
    }
    return sectorIndex;
}

int __cdecl DynEnt_GetContents(const DynEntityDef *dynEntDef)
{
    if ( !dynEntDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 622, 0, "%s", "dynEntDef") )
    {
        __debugbreak();
    }
    return dynEntDef->contents;
}

void __cdecl DynEnt_GetLocalBounds(const DynEntityDef *dynEntDef, float *mins, float *maxs)
{
    const char *v3; // eax

    if ( !dynEntDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 633, 0, "%s", "dynEntDef") )
    {
        __debugbreak();
    }
    if ( !dynEntDef->xModel
        && !dynEntDef->brushModel
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                    634,
                    0,
                    "%s",
                    "dynEntDef->xModel || dynEntDef->brushModel") )
    {
        __debugbreak();
    }
    if ( !mins
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 635, 0, "%s", "mins") )
    {
        __debugbreak();
    }
    if ( !maxs
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 636, 0, "%s", "maxs") )
    {
        __debugbreak();
    }
    if ( dynEntDef->xModel )
    {
        XModelGetBounds(dynEntDef->xModel, mins, maxs);
        if ( *maxs <= *mins || maxs[1] <= mins[1] || maxs[2] <= mins[2] )
        {
            v3 = va("Bad bounds on xmodel %s", dynEntDef->xModel->name);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                            643,
                            0,
                            "%s\n\t%s",
                            "mins[0] < maxs[0] && mins[1] < maxs[1] && mins[2] < maxs[2]",
                            v3) )
                __debugbreak();
        }
    }
    else
    {
        CM_ModelBounds(dynEntDef->brushModel, mins, maxs);
    }
}

double __cdecl DynEnt_GetRadiusDistSqr(const DynEntityColl *dynEntColl, const float *origin)
{
    float absMaxs[3]; // [esp+8h] [ebp-28h]
    float offset[3]; // [esp+14h] [ebp-1Ch]
    int vecIndex; // [esp+20h] [ebp-10h]
    float absMins[3]; // [esp+24h] [ebp-Ch]

    if ( !dynEntColl
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 682, 0, "%s", "dynEntColl") )
    {
        __debugbreak();
    }
    if ( !origin
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 683, 0, "%s", "origin") )
    {
        __debugbreak();
    }
    absMins[0] = dynEntColl->linkMins[0];
    absMins[1] = dynEntColl->linkMins[1];
    absMins[2] = dynEntColl->linkMins[2];
    absMaxs[0] = dynEntColl->linkMaxs[0];
    absMaxs[1] = dynEntColl->linkMaxs[1];
    absMaxs[2] = dynEntColl->linkMaxs[2];
    for ( vecIndex = 0; vecIndex < 3; ++vecIndex )
    {
        if ( absMins[vecIndex] <= origin[vecIndex] )
        {
            if ( origin[vecIndex] <= absMaxs[vecIndex] )
                offset[vecIndex] = 0.0f;
            else
                offset[vecIndex] = origin[vecIndex] - absMaxs[vecIndex];
        }
        else
        {
            offset[vecIndex] = absMins[vecIndex] - origin[vecIndex];
        }
    }
    return offset[2] * offset[2] + offset[1] * offset[1] + offset[0] * offset[0];
}

bool __cdecl DynEnt_IsInCone(
                const DynEntityDef *dynEntDef,
                const GfxPlacement *pose,
                const float *origin,
                float coneAngleCos,
                const float *coneDir)
{
    float dir[3]; // [esp+14h] [ebp-Ch] BYREF

    if ( !coneDir || coneAngleCos == -1.0 )
        return 1;
    dir[0] = pose->origin[0] - *origin;
    dir[1] = pose->origin[1] - origin[1];
    dir[2] = pose->origin[2] - origin[2];
    return Vec3Normalize(dir) < 0.00001
            || (float)((float)((float)(dir[0] * *coneDir) + (float)(dir[1] * coneDir[1])) + (float)(dir[2] * coneDir[2])) > coneAngleCos;
}

double __cdecl DynEnt_GetCylindricalRadiusDistSqr(const DynEntityColl *dynEntColl, const float *origin)
{
    float absMaxs[3]; // [esp+8h] [ebp-24h]
    float offset[2]; // [esp+14h] [ebp-18h]
    int vecIndex; // [esp+1Ch] [ebp-10h]
    float absMins[3]; // [esp+20h] [ebp-Ch]

    if ( !dynEntColl
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 724, 0, "%s", "dynEntColl") )
    {
        __debugbreak();
    }
    if ( !origin
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 725, 0, "%s", "origin") )
    {
        __debugbreak();
    }
    absMins[0] = dynEntColl->linkMins[0];
    absMins[1] = dynEntColl->linkMins[1];
    absMins[2] = dynEntColl->linkMins[2];
    absMaxs[0] = dynEntColl->linkMaxs[0];
    absMaxs[1] = dynEntColl->linkMaxs[1];
    absMaxs[2] = dynEntColl->linkMaxs[2];
    for ( vecIndex = 0; vecIndex < 2; ++vecIndex )
    {
        if ( absMins[vecIndex] <= origin[vecIndex] )
        {
            if ( origin[vecIndex] <= absMaxs[vecIndex] )
                offset[vecIndex] = 0.0f;
            else
                offset[vecIndex] = origin[vecIndex] - absMaxs[vecIndex];
        }
        else
        {
            offset[vecIndex] = absMins[vecIndex] - origin[vecIndex];
        }
    }
    return offset[1] * offset[1] + offset[0] * offset[0];
}

bool __cdecl DynEnt_EntityInArea(
                const DynEntityDef *dynEntDef,
                const DynEntityColl *dynEntColl,
                const float *mins,
                const float *maxs,
                int contentMask)
{
    if ( !dynEntDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 750, 0, "%s", "dynEntDef") )
    {
        __debugbreak();
    }
    if ( !dynEntColl
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 751, 0, "%s", "dynEntColl") )
    {
        __debugbreak();
    }
    if ( !mins
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 752, 0, "%s", "mins") )
    {
        __debugbreak();
    }
    if ( !maxs
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 753, 0, "%s", "maxs") )
    {
        __debugbreak();
    }
    if ( (contentMask & DynEnt_GetContents(dynEntDef)) == 0 )
        return 0;
    if ( dynEntColl->linkMins[0] > maxs[0] || dynEntColl->linkMins[1] > maxs[1] || dynEntColl->linkMins[2] > maxs[2] )
        return 0;
    return *mins <= dynEntColl->linkMaxs[0] && mins[1] <= dynEntColl->linkMaxs[1] && mins[2] <= dynEntColl->linkMaxs[2];
}

void __cdecl DynEnt_PointTraceToModel(
                const DynEntityDef *dynEntDef,
                const GfxPlacement *pose,
                const pointtrace_t *clip,
                trace_t *results)
{
    unsigned __int16 Id; // [esp+Ah] [ebp-6Ah]
    float dynEntAxis[4][3]; // [esp+20h] [ebp-54h] BYREF
    float localStart[3]; // [esp+50h] [ebp-24h] BYREF
    float normal[3]; // [esp+5Ch] [ebp-18h] BYREF
    float localEnd[3]; // [esp+68h] [ebp-Ch] BYREF

    if ( !dynEntDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 781, 0, "%s", "dynEntDef") )
    {
        __debugbreak();
    }
    if ( !pose
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 782, 0, "%s", "pose") )
    {
        __debugbreak();
    }
    if ( !clip
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 783, 0, "%s", "clip") )
    {
        __debugbreak();
    }
    if ( !results
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 784, 0, "%s", "results") )
    {
        __debugbreak();
    }
    if ( !dynEntDef->xModel
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                    785,
                    0,
                    "%s",
                    "dynEntDef->xModel") )
    {
        __debugbreak();
    }
    if ( results->fraction <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                    787,
                    0,
                    "%s\n\t(results->fraction) = %g",
                    "(results->fraction > 0.0f)",
                    results->fraction) )
    {
        __debugbreak();
    }
    if ( results->fraction > 1.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                    788,
                    0,
                    "%s\n\t(results->fraction) = %g",
                    "(results->fraction <= 1.0f)",
                    results->fraction) )
    {
        __debugbreak();
    }
    if ( clip->bLocational )
    {
        dynEntAxis[3][0] = pose->origin[0];
        dynEntAxis[3][1] = pose->origin[1];
        dynEntAxis[3][2] = pose->origin[2];
        UnitQuatToAxis(pose->quat, dynEntAxis);
        MatrixTransposeTransformVector43(clip->extents.start.vec.v, dynEntAxis, localStart);
        MatrixTransposeTransformVector43(clip->extents.end.vec.v, dynEntAxis, localEnd);
        if ( !XModelTraceLine(dynEntDef->xModel, results, localStart, localEnd, clip->contentmask) )
        {
            MatrixTransformVector(results->normal.vec.v, dynEntAxis, normal);
            Vec3NormalizeTo(normal, results->normal.vec.v);
            Id = DynEnt_GetId(dynEntDef, DYNENT_DRAW_MODEL);
            if ( !results
                && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_public.h", 175, 0, "%s", "trace") )
            {
                __debugbreak();
            }
            results->hitType = TRACE_HITTYPE_DYNENT_MODEL;
            results->hitId = Id;
        }
    }
}

void __cdecl DynEnt_PointTraceToBrush(
                const DynEntityDef *dynEntDef,
                const GfxPlacement *pose,
                const pointtrace_t *clip,
                trace_t *results)
{
    unsigned __int16 Id; // [esp+Ah] [ebp-2Ah]
    float oldFraction; // [esp+Ch] [ebp-28h]
    float axis[3][3]; // [esp+10h] [ebp-24h] BYREF

    if ( !dynEntDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 817, 0, "%s", "dynEntDef") )
    {
        __debugbreak();
    }
    if ( !pose
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 818, 0, "%s", "pose") )
    {
        __debugbreak();
    }
    if ( !clip
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 819, 0, "%s", "clip") )
    {
        __debugbreak();
    }
    if ( !results
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp", 820, 0, "%s", "results") )
    {
        __debugbreak();
    }
    if ( dynEntDef->xModel
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                    821,
                    0,
                    "%s",
                    "!dynEntDef->xModel") )
    {
        __debugbreak();
    }
    if ( !dynEntDef->brushModel
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                    822,
                    0,
                    "%s",
                    "dynEntDef->brushModel") )
    {
        __debugbreak();
    }
    if ( results->fraction <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                    824,
                    0,
                    "%s\n\t(results->fraction) = %g",
                    "(results->fraction > 0.0f)",
                    results->fraction) )
    {
        __debugbreak();
    }
    if ( results->fraction > 1.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_coll.cpp",
                    825,
                    0,
                    "%s\n\t(results->fraction) = %g",
                    "(results->fraction <= 1.0f)",
                    results->fraction) )
    {
        __debugbreak();
    }
    oldFraction = results->fraction;
    UnitQuatToAxis(pose->quat, axis);
    CM_TransformedBoxTraceRotated(
        results,
        clip->extents.start.vec.v,
        clip->extents.end.vec.v,
        vec3_origin,
        vec3_origin,
        dynEntDef->brushModel,
        clip->contentmask,
        pose->origin,
        axis);
    if ( results->fraction < oldFraction )
    {
        Id = DynEnt_GetId(dynEntDef, DYNENT_DRAW_BRUSH);
        if ( !results && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_public.h", 175, 0, "%s", "trace") )
            __debugbreak();
        results->hitType = TRACE_HITTYPE_DYNENT_BRUSH;
        results->hitId = Id;
    }
}

