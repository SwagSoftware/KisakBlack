#include "cl_rank.h"
#include <database/db_registry.h>
#include <cstring>
#include <universal/com_stringtable.h>


rankData_t s_rankData;

int s_maxxp;
int s_maxprestige;

void __cdecl CL_BuildRankXPTable()
{
    const char *ColumnValueForRow; // eax
    const char *v1; // eax
    const char *v2; // eax
    int row; // [esp+0h] [ebp-8h]
    const StringTable *table; // [esp+4h] [ebp-4h] BYREF

    memset(&s_rankData, 0, sizeof(s_rankData));
    StringTable_GetAsset("mp/rankTable.csv", (XAssetHeader *)&table);
    s_rankData.topRow = table->rowCount - 1;
    if ( table->rowCount >= 96
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_rank.cpp",
                    34,
                    0,
                    "%s\n\t(table->rowCount) = %i",
                    "(table->rowCount < 96)",
                    table->rowCount) )
    {
        __debugbreak();
    }
    if ( table->rowCount <= 96 )
    {
        for ( row = 0; row < table->rowCount; ++row )
        {
            ColumnValueForRow = StringTable_GetColumnValueForRow(table, row, 2);
            s_rankData.minxp[row] = atoi(ColumnValueForRow);
            v1 = StringTable_GetColumnValueForRow(table, row, 7);
            s_rankData.maxxp[row] = atoi(v1);
            v2 = StringTable_GetColumnValueForRow(table, row, 0);
            s_rankData.rankId[row] = atoi(v2);
        }
        s_rankData.isValid = 1;
    }
}

int __cdecl CL_GetMaxXP()
{
    const char *ColumnValueForRow; // eax
    const StringTable *table; // [esp+0h] [ebp-4h] BYREF

    if ( !s_maxxp )
    {
        StringTable_GetAsset("mp/rankTable.csv", (XAssetHeader *)&table);
        ColumnValueForRow = StringTable_GetColumnValueForRow(table, s_rankData.topRow, 7);
        s_maxxp = atoi(ColumnValueForRow);
    }
    return s_maxxp;
}

int __cdecl CL_GetMaxPrestige()
{
    const char *v0; // eax
    const StringTable *table; // [esp+0h] [ebp-4h] BYREF

    if ( !s_maxprestige )
    {
        StringTable_GetAsset("mp/rankIconTable.csv", (XAssetHeader *)&table);
        v0 = StringTable_Lookup(table, 0, "maxprestige", 1);
        s_maxprestige = atoi(v0);
    }
    return s_maxprestige;
}

int __cdecl CL_GetRankForXp(int xp)
{
    int toprow; // [esp+0h] [ebp-14h]
    int maxxp; // [esp+4h] [ebp-10h]
    int row; // [esp+8h] [ebp-Ch]
    int bottomrow; // [esp+Ch] [ebp-8h]
    int minxp; // [esp+10h] [ebp-4h]

    if ( !s_rankData.isValid
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_rank.cpp", 87, 0, "%s", "s_rankData.isValid") )
    {
        __debugbreak();
    }
    if ( !s_rankData.isValid )
        return 0;
    toprow = s_rankData.topRow;
    bottomrow = 0;
    row = s_rankData.topRow / 2;
    while ( toprow > bottomrow )
    {
        minxp = s_rankData.minxp[row];
        maxxp = s_rankData.maxxp[row];
        if ( maxxp < minxp
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_rank.cpp", 112, 0, "%s", "maxxp >= minxp") )
        {
            __debugbreak();
        }
        if ( xp >= minxp )
        {
            if ( xp < maxxp )
                return s_rankData.rankId[row];
            bottomrow = row + 1;
        }
        else
        {
            toprow = row - 1;
        }
        row = bottomrow + (toprow - bottomrow) / 2;
    }
    return s_rankData.rankId[row];
}

const char *__cdecl CL_GetRankData(int rank, rankTableColumns_t column)
{
    const StringTable *table; // [esp+4h] [ebp-8h] BYREF
    char level[4]; // [esp+8h] [ebp-4h] BYREF

    if ( (unsigned int)column >= MP_RANKTABLE_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_rank.cpp",
                    138,
                    0,
                    "column doesn't index MP_RANKTABLE_COUNT\n\t%i not in [0, %i)",
                    column,
                    18) )
    {
        __debugbreak();
    }
    StringTable_GetAsset("mp/rankTable.csv", (XAssetHeader *)&table);
    if ( !table && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_rank.cpp", 142, 0, "%s", "table") )
        __debugbreak();
    Com_sprintf(level, 4u, "%i", rank);
    return StringTable_Lookup(table, 0, level, column);
}

void __cdecl CL_GetRankIcon(int rank, int prestige, Material **handle)
{
    const StringTable *table; // [esp+4h] [ebp-Ch] BYREF
    const char *rankIconName; // [esp+8h] [ebp-8h]
    char id[4]; // [esp+Ch] [ebp-4h] BYREF

    if ( useFastFile->current.enabled )
    {
        if ( rank < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_rank.cpp",
                        164,
                        0,
                        "%s\n\t(rank) = %i",
                        "(rank >= 0)",
                        rank) )
        {
            __debugbreak();
        }
        if ( prestige < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_rank.cpp",
                        165,
                        0,
                        "%s\n\t(prestige) = %i",
                        "(prestige >= 0)",
                        prestige) )
        {
            __debugbreak();
        }
        if ( !handle
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_rank.cpp", 166, 0, "%s", "handle") )
        {
            __debugbreak();
        }
        StringTable_GetAsset("mp/rankIconTable.csv", (XAssetHeader *)&table);
        if ( !table && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_rank.cpp", 170, 0, "%s", "table") )
            __debugbreak();
        Com_sprintf(id, 4u, "%i", rank);
        rankIconName = StringTable_Lookup(table, 0, id, prestige + 1);
        *handle = Material_RegisterHandle((char *)rankIconName, 7);
        if ( Material_IsDefault(*handle) )
            *handle = 0;
    }
}

const char *__cdecl CL_GetRankIconName(int rank, int prestige)
{
    const StringTable *table; // [esp+4h] [ebp-Ch] BYREF
    const char *rankIconName; // [esp+8h] [ebp-8h]
    char id[4]; // [esp+Ch] [ebp-4h] BYREF

    rankIconName = 0;
    if ( !useFastFile->current.enabled )
        return 0;
    if ( rank < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_rank.cpp",
                    191,
                    0,
                    "%s\n\t(rank) = %i",
                    "(rank >= 0)",
                    rank) )
    {
        __debugbreak();
    }
    if ( prestige < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_rank.cpp",
                    192,
                    0,
                    "%s\n\t(prestige) = %i",
                    "(prestige >= 0)",
                    prestige) )
    {
        __debugbreak();
    }
    StringTable_GetAsset("mp/rankIconTable.csv", (XAssetHeader *)&table);
    if ( !table && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_rank.cpp", 196, 0, "%s", "table") )
        __debugbreak();
    Com_sprintf(id, 4u, "%i", rank);
    return StringTable_Lookup(table, 0, id, prestige + 1);
}

