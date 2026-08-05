#include "live_fileshare.h"
#include <ui/ui_shared.h>
#include "live_win.h"
#include "live_stats.h"
#include "live_fileshare_search.h"
#include <cgame/cg_compass.h>
#include <ui/ui_feeders.h>
#include <universal/com_shared.h>
#include <ui/ui_atoms.h>
#include <client_mp/cl_cgame_mp.h>
#include <qcommon/com_clients.h>
#include <server_mp/sv_main_pc_mp.h>
#include <client/cl_keys.h>

const char *fileShareKeyNames[30] =
{
  "slot",
  "ingameSlot",
  "slotOccupied",
  "datetime",
  "length",
  "size",
  "author",
  "map",
  "mapName",
  "gameType",
  "gameTypeName",
  "fileID",
  "fileSize",
  "fileType",
  "fileName",
  "hasSummary",
  "gameTypeAndMapName",
  "rating",
  "typeIconName",
  "selectedImageName",
  "name",
  "isModifiedName",
  "description",
  "isModifiedDescription",
  "userTag",
  "userTagIndex",
  "gameTypeImage",
  "getTotalVotes",
  "isSummaryCached",
  "isSummaryLoaded"
};


bool(__cdecl *fileShareKeyFunctions[30])(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *) =
{
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetSlot,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetIngameSlot,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_IsSlotOccupied,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetDate,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetLength,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetSize,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetOwnerName,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetMap,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetMapName,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetGameType,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetGameTypeName,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetFileID,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetFileSize,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetFileType,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetFileName,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_HasSummary,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetGameTypeAndMapName,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetRating,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetTypeIconName,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetSelectedImageName,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetName,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetIsModifiedName,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetDescription,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetIsModifiedDescription,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetUserTag,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetUserTagIndex,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetGameTypeImage,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_GetTotalVotes,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_IsSummaryCached,
  (bool(*)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **, float *))Live_FileShare_IsSummaryLoaded
};



const char *s_fsSearchRowTextDvarNames[7] =
{
  NULL,
  "fsSearchRowText1",
  "fsSearchRowText2",
  "fsSearchRowText3",
  "fsSearchRowText4",
  "fsSearchRowText5",
  "fsSearchRowText6"
};

const char *s_fsSearchRowValueDvarNames[7] =
{
  NULL,
  "fsSearchRowValue1",
  "fsSearchRowValue2",
  "fsSearchRowValue3",
  "fsSearchRowValue4",
  "fsSearchRowValue5",
  "fsSearchRowValue6"
};

unsigned int s_fileShareFileRatingPreview;

int dword_A4C8928;

char s_filterList[512];

bdFileMetaData s_theaterFileMetaData;
unsigned int s_fileShareFileRating;
ratingHistory_t s_fileShareRatingHistory;

fileShareLastPlayedGame_t s_lastPlayedGame;
char s_liveBlurb[256];

const dvar_s *fshLiveBlurb;

fileSharePooledDetails_t s_pooledFileDetails;
ddlDef_t *g_fileSharePublicDDL;
ddlState_t g_fileSharePublicRootState;

const dvar_t *fsSearchFileType;
const dvar_t *fsSearchSelectedRow;
const dvar_t *fsShowStreamingGraph;
const dvar_t *fsPrivateSlotCol;
const dvar_t *fsOtherUserPrivateSlotCol;
const dvar_t *fsMaxPrivateSlotRowsOther;
const dvar_t *fsOtherUserSlotSelected;
const dvar_t *fsSlotEmptyHiddenColor;
const dvar_t *fsSlotEmptyShowColorBg;
const dvar_t *fsSlotEmptyMainColor;
const dvar_t *fsSlotEmptyNotSubscribedColor;
const dvar_t *fsSlotNumMainColor;
const dvar_t *fsSlotNumNoSubsribeColor;
const dvar_t *fsSlotEmptyShowColor;
const dvar_t *fsSlotMainColor;
const dvar_t *fsSlotHighlightedColor;
const dvar_t *fsSlotHighlightedColorNoSel;
const dvar_t *fsDebugRatingValue;
const dvar_t *fsStarHighlightColor;
const dvar_t *fsStarPreviewColor;
const dvar_t *fsStarAvgColor;
const dvar_t *fshOldItemColor;
const dvar_t *fsSelectedFileID;
const dvar_t *fsSelectedFileName;
const dvar_t *fsIsSelectedFileNameModified;
const dvar_t *fsSelectedFileDescription;
const dvar_t *fsIsSelectedFileDescriptionModified;
const dvar_t *fsSelectedFileTagIndex;
const dvar_t *fshSelectLastSlotRow;
const dvar_t *fshSelectFirstSlotRow;
const dvar_t *fshDebugFileList;
const dvar_t *fsRecents;
const dvar_t *fsRecentsCount;
const dvar_t *s_fsSearchRowTextDvars[7];
const dvar_t *s_fsSearchRowValueDvars[7];


char s_gamerTag[32];


fileShareLastPlayedGame_t *__cdecl Live_FileShare_GetLastPlayedGame()
{
    return &s_lastPlayedGame;
}

void __cdecl Live_FileShare_SetLiveBlurb(const char *blurb)
{
    I_strncpyz(s_liveBlurb, blurb, 256);
    Dvar_SetString((dvar_s *)fshLiveBlurb, s_liveBlurb);
}

char __cdecl Live_FileShare_GetTag(
                bdFileMetaData *descriptors,
                int itemIndex,
                unsigned __int64 tagType,
                unsigned __int64 *tag)
{
    unsigned int m_numTags; // [esp+Ch] [ebp-10h]
    unsigned int i; // [esp+14h] [ebp-8h]

    if ( (int)descriptors[itemIndex].m_numTags > 40 )
        m_numTags = 40;
    else
        m_numTags = descriptors[itemIndex].m_numTags;
    for ( i = 0; i < m_numTags; ++i )
    {
        if ( tagType == descriptors[itemIndex].m_tags[i].m_priTag )
        {
            *tag = descriptors[itemIndex].m_tags[i].m_secTag;
            return 1;
        }
    }
    *tag = 0;
    return 0;
}

fileSharePooledDetails_t *__cdecl Live_FileShare_GetFilePooledDetailsBuffer()
{
    return &s_pooledFileDetails;
}

bdFileMetaData *__cdecl Live_FileShare_FindFileInPrivateData(
                unsigned __int64 fileID,
                fileShareBufferLocation bufferLocation)
{
    int i; // [esp+8h] [ebp-8h]
    fileSharePrivateData *data; // [esp+Ch] [ebp-4h]

    data = LiveStorage_FileShare_GetFileShareData(bufferLocation);
    for ( i = 0; i < data->descriptorCount; ++i )
    {
        if ( data->descriptors[i].m_fileID == fileID )
            return &data->descriptors[i];
    }
    return 0;
}

bool __cdecl Live_FileShare_ValidatePublicDDLBuffer(char *buffer)
{
    char backup[516]; // [esp+0h] [ebp-208h] BYREF

    return DDL_AssociateBuffer(buffer, 512, g_fileSharePublicDDL)
            || DDL_FixBufferVersion(buffer, g_fileSharePublicDDL, "ddl_mp/file_share_public.ddl", backup, 512);
}

char __cdecl Live_FileShare_ReadPublicMetaDataCreateTime(bdFileMetaData *descriptor, int *createTime)
{
    ddlState_t state; // [esp+0h] [ebp-14h] BYREF
    char *buffer; // [esp+10h] [ebp-4h]

    buffer = (char *)descriptor->m_metaData;
    if ( !Live_FileShare_ValidatePublicDDLBuffer((char *)descriptor->m_metaData)
        || !DDL_MoveToName(&g_fileShareRootState, &state, "createTime") )
    {
        return 0;
    }
    *createTime = DDL_GetInt(&state, buffer);
    return 1;
}

char __cdecl Live_FileShare_WritePublicMetaDataCreateTime(char *buffer, unsigned int createTime)
{
    ddlState_t state; // [esp+0h] [ebp-10h] BYREF

    if ( !Live_FileShare_ValidatePublicDDLBuffer(buffer) || !DDL_MoveToName(&g_fileShareRootState, &state, "createTime") )
        return 0;
    DDL_SetInt(&state, createTime, buffer);
    return 1;
}

char __cdecl Live_FileShare_ReadPublicMetaDataLength(bdFileMetaData *descriptor, int *duration)
{
    ddlState_t state; // [esp+0h] [ebp-14h] BYREF
    char *buffer; // [esp+10h] [ebp-4h]

    buffer = (char *)descriptor->m_metaData;
    if ( !Live_FileShare_ValidatePublicDDLBuffer((char *)descriptor->m_metaData)
        || !DDL_MoveToName(&g_fileShareRootState, &state, "length") )
    {
        return 0;
    }
    *duration = DDL_GetInt(&state, buffer);
    return 1;
}

char __cdecl Live_FileShare_WritePublicMetaDataLength(char *buffer, unsigned int duration)
{
    ddlState_t state; // [esp+0h] [ebp-10h] BYREF

    if ( !Live_FileShare_ValidatePublicDDLBuffer(buffer) || !DDL_MoveToName(&g_fileShareRootState, &state, "length") )
        return 0;
    DDL_SetInt(&state, duration, buffer);
    return 1;
}

char __cdecl Live_FileShare_PublicMetaDataToTags(bdFileMetaData *descriptor)
{
#ifdef KISAK_LIVE_SERVICE
    unsigned int Int; // eax
    unsigned int v2; // edx
    unsigned int Int64; // eax
    unsigned int v4; // ecx
    int v5; // edx
    char *LocalizedGameTypeFromIndex; // eax
    char *LocalizedMapNameFromIndex; // eax
    ddlState_t tagState; // [esp+18h] [ebp-144h] BYREF
    int duration; // [esp+28h] [ebp-134h] BYREF
    char metaData[255]; // [esp+2Ch] [ebp-130h] BYREF
    int createTime; // [esp+130h] [ebp-2Ch] BYREF
    unsigned int i; // [esp+134h] [ebp-28h]
    ddlState_t state; // [esp+138h] [ebp-24h] BYREF
    char *buffer; // [esp+148h] [ebp-14h]
    unsigned __int64 gameTypeIndex; // [esp+14Ch] [ebp-10h]
    unsigned __int64 mapNameIndex; // [esp+154h] [ebp-8h]

    buffer = (char *)descriptor->m_metaData;
    descriptor->m_numTags = 0;
    if ( !Live_FileShare_ValidatePublicDDLBuffer(buffer) )
        return 0;
    if ( DDL_MoveToName(&g_fileSharePublicRootState, &state, "numTags") )
        descriptor->m_numTags = DDL_GetInt(&state, buffer);
    if ( !descriptor->m_numTags
        || !DDL_MoveToName(&g_fileSharePublicRootState, &state, "tags")
        || !DDL_MoveToIndex(&state, &state, 0, 1)
        || !DDL_IterateFirst(&state, &state) )
    {
        return 0;
    }
    i = 0;
    do
    {
        DDL_MoveToName(&state, &tagState, "priTag");
        Int = DDL_GetInt(&tagState, buffer);
        v2 = i;
        LODWORD(descriptor->m_tags[v2].m_priTag) = Int;
        HIDWORD(descriptor->m_tags[v2].m_priTag) = 0;
        DDL_MoveToName(&state, &tagState, "secTag");
        Int64 = DDL_GetInt64(&tagState, buffer);
        v4 = i;
        LODWORD(descriptor->m_tags[v4].m_secTag) = Int64;
        HIDWORD(descriptor->m_tags[v4].m_secTag) = v5;
        if ( LODWORD(descriptor->m_tags[i].m_priTag) != 1 || HIDWORD(descriptor->m_tags[i].m_priTag) )
        {
            if ( LODWORD(descriptor->m_tags[i].m_priTag) == 2 && !HIDWORD(descriptor->m_tags[i].m_priTag) )
                mapNameIndex = descriptor->m_tags[i].m_secTag;
        }
        else
        {
            gameTypeIndex = descriptor->m_tags[i].m_secTag;
        }
        ++i;
    }
    while ( DDL_IterateNext(&state, &state) && i < descriptor->m_numTags );
    duration = 0;
    createTime = 0;
    Live_FileShare_ReadPublicMetaDataLength(descriptor, &duration);
    Live_FileShare_ReadPublicMetaDataCreateTime(descriptor, &createTime);
    memset(metaData, 0, sizeof(metaData));
    DDL_AssociateBuffer(metaData, 255, g_fileshareDDL);
    if ( DDL_MoveToName(&g_fileShareRootState, &state, "name")
        && (LocalizedGameTypeFromIndex = Live_FileShare_GetLocalizedGameTypeFromIndex(gameTypeIndex, 1),
                DDL_SetString(&state, LocalizedGameTypeFromIndex, metaData)) )
    {
        if ( DDL_MoveToName(&g_fileShareRootState, &state, "isModifiedName") && DDL_SetInt(&state, 0, metaData) )
        {
            if ( DDL_MoveToName(&g_fileShareRootState, &state, "description")
                && (LocalizedMapNameFromIndex = Live_FileShare_GetLocalizedMapNameFromIndex(mapNameIndex),
                        DDL_SetString(&state, LocalizedMapNameFromIndex, metaData)) )
            {
                if ( DDL_MoveToName(&g_fileShareRootState, &state, "isModifiedDescription") && DDL_SetInt(&state, 0, metaData) )
                {
                    if ( DDL_MoveToName(&g_fileShareRootState, &state, "createTime") && DDL_SetInt(&state, createTime, metaData) )
                    {
                        if ( DDL_MoveToName(&g_fileShareRootState, &state, "length") && DDL_SetInt(&state, duration, metaData) )
                        {
                            memcpy(descriptor->m_metaData, metaData, 0xFCu);
                            *(_WORD *)&descriptor->m_metaData[252] = *(_WORD *)&metaData[252];
                            descriptor->m_metaData[254] = metaData[254];
                            return 1;
                        }
                        else
                        {
                            Com_PrintError(16, "Could not set 'length' in the file share ddl.\n");
                            return 0;
                        }
                    }
                    else
                    {
                        Com_PrintError(16, "Could not set 'createTime' in the file share ddl.\n");
                        return 0;
                    }
                }
                else
                {
                    Com_PrintError(16, "Could not set 'isModifiedDescription' in the file share ddl.\n");
                    return 0;
                }
            }
            else
            {
                Com_PrintError(16, "Could not set 'description' in the file share ddl.\n");
                return 0;
            }
        }
        else
        {
            Com_PrintError(16, "Could not set 'isModifiedName' in the file share ddl.\n");
            return 0;
        }
    }
    else
    {
        Com_PrintError(16, "Could not set 'name' in the file share ddl.\n");
        return 0;
    }
#else
return 0;
#endif
}

char __cdecl Live_FileShare_WritePublicMetaDataTags(char *buffer, bdTag *tags, int numTags)
{
    ddlState_t tagState; // [esp+0h] [ebp-24h] BYREF
    int i; // [esp+10h] [ebp-14h]
    ddlState_t state; // [esp+14h] [ebp-10h] BYREF

    if ( !Live_FileShare_ValidatePublicDDLBuffer(buffer)
        || !DDL_MoveToName(&g_fileSharePublicRootState, &state, "tags")
        || !DDL_MoveToIndex(&state, &state, 0, 1)
        || !DDL_IterateFirst(&state, &state) )
    {
        return 0;
    }
    i = 0;
    do
    {
        DDL_MoveToName(&state, &tagState, "priTag");
        DDL_SetInt(&tagState, tags[i].m_priTag, buffer);
        DDL_MoveToName(&state, &tagState, "secTag");
        DDL_SetInt64(&tagState, tags[i++].m_secTag, buffer);
    }
    while ( DDL_IterateNext(&state, &state) && i < numTags );
    if ( DDL_MoveToName(&g_fileSharePublicRootState, &state, "numTags") )
        DDL_SetInt(&state, numTags, buffer);
    return 1;
}

bool __cdecl Live_FileShare_ReadFromMetaData(
                int index,
                bdFileMetaData *descriptor,
                char **stringResult,
                int *intResult)
{
    bool result; // al
    char backup[256]; // [esp+4h] [ebp-118h] BYREF
    ddlState_t state; // [esp+108h] [ebp-14h] BYREF
    char *buffer; // [esp+118h] [ebp-4h]

    buffer = (char *)descriptor->m_metaData;
    if ( !DDL_AssociateBuffer((char *)descriptor->m_metaData, 255, g_fileshareDDL)
        && !DDL_FixBufferVersion(buffer, g_fileshareDDL, "ddl_mp/file_share.ddl", backup, 255) )
    {
        return 0;
    }
    switch ( index )
    {
        case 0:
            if ( !DDL_MoveToName(&g_fileShareRootState, &state, "authorName") )
                goto LABEL_24;
            goto LABEL_15;
        case 1:
            if ( DDL_MoveToName(&g_fileShareRootState, &state, "authorXuid") )
                goto LABEL_15;
            goto LABEL_24;
        case 2:
            if ( DDL_MoveToName(&g_fileShareRootState, &state, "createTime") )
                goto LABEL_6;
            goto LABEL_24;
        case 3:
            if ( DDL_MoveToName(&g_fileShareRootState, &state, "length") )
                goto LABEL_6;
            goto LABEL_24;
        case 4:
            if ( !DDL_MoveToName(&g_fileShareRootState, &state, "name") )
                goto LABEL_24;
            goto LABEL_15;
        case 5:
            if ( !DDL_MoveToName(&g_fileShareRootState, &state, "description") )
                goto LABEL_24;
LABEL_15:
            *stringResult = DDL_GetString(&state, buffer);
            return 1;
        case 6:
            if ( !DDL_MoveToName(&g_fileShareRootState, &state, "isModifiedName") )
                goto LABEL_24;
            goto LABEL_6;
        case 7:
            if ( !DDL_MoveToName(&g_fileShareRootState, &state, "isModifiedDescription") )
                goto LABEL_24;
LABEL_6:
            *intResult = DDL_GetInt(&state, buffer);
            result = 1;
            break;
        default:
LABEL_24:
            *intResult = 0;
            *stringResult = (char *)"";
            result = 0;
            break;
    }
    return result;
}

bool __cdecl Live_FileShare_ValidateDDLBuffer(char *buffer)
{
    char backup[260]; // [esp+0h] [ebp-108h] BYREF

    return DDL_AssociateBuffer(buffer, 255, g_fileshareDDL)
            || DDL_FixBufferVersion(buffer, g_fileshareDDL, "ddl_mp/file_share.ddl", backup, 255);
}

void __cdecl Live_FileShare_AddTag(
                unsigned __int64 priTag,
                unsigned __int64 secTag,
                int *numTags,
                bdTag *tags,
                int maxNumTags)
{
    if ( *numTags + 1 > maxNumTags )
    {
        Com_PrintError(16, "Too many tags for file share search while adding a new search tag type.\n");
    }
    else
    {
        tags[*numTags].m_priTag = priTag;
        tags[(*numTags)++].m_secTag = secTag;
    }
}

char *__cdecl Live_FileShare_GetLocalizedUserTagFromIndex(int index)
{
    const char *v1; // eax
    const char *result; // [esp+0h] [ebp-8h]
    const StringTable *fileShareTable; // [esp+4h] [ebp-4h] BYREF

    StringTable_GetAsset("mp/fileShareTags.csv", (XAssetHeader *)&fileShareTable);
    if ( index > 0
        && index <= fileShareTable->rowCount
        && (v1 = va("%d", index), (result = StringTable_Lookup(fileShareTable, 0, v1, 1)) != 0)
        && *result )
    {
        return UI_SafeTranslateString(result);
    }
    else
    {
        return UI_SafeTranslateString("MPUI_FILESHARE_TAG_NONE");
    }
}

const char *__cdecl Live_FileShare_GetGameTypeFromIndex(int index)
{
    const char *v1; // eax
    const char *result; // [esp+0h] [ebp-8h]
    const StringTable *gameTypesTable; // [esp+4h] [ebp-4h] BYREF

    StringTable_GetAsset("mp/gametypesTable.csv", (XAssetHeader *)&gameTypesTable);
    if ( !gameTypesTable
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare.cpp", 677, 0, "%s", "gameTypesTable") )
    {
        __debugbreak();
    }
    if ( index >= 0 && index < gameTypesTable->rowCount )
    {
        v1 = va("%d", index);
        result = StringTable_Lookup(gameTypesTable, 4, v1, 0);
        if ( result && *result )
            return va("%s", result);
        Com_PrintError(16, "Could not index %d in gameTypesTable.csv.\n", index);
    }
    return "";
}

const char *__cdecl Live_FileShare_GetGameTypeImageFromIndex(int index)
{
    const char *v1; // eax
    const char *result; // [esp+0h] [ebp-8h]
    const StringTable *gameTypesTable; // [esp+4h] [ebp-4h] BYREF

    StringTable_GetAsset("mp/gametypesTable.csv", (XAssetHeader *)&gameTypesTable);
    if ( !gameTypesTable
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare.cpp", 697, 0, "%s", "gameTypesTable") )
    {
        __debugbreak();
    }
    if ( index >= 0 && index < gameTypesTable->rowCount )
    {
        v1 = va("%d", index);
        result = StringTable_Lookup(gameTypesTable, 4, v1, 3);
        if ( result && *result )
            return va("%s", result);
        Com_PrintError(16, "Could not index %d in gameTypesTable.csv.\n", index);
    }
    return "";
}

char *__cdecl Live_FileShare_GetLocalizedGameTypeFromIndex(int index, bool upperCase)
{
    const char *v2; // eax
    const char *result; // [esp+0h] [ebp-8h]
    const StringTable *gameTypesTable; // [esp+4h] [ebp-4h] BYREF

    StringTable_GetAsset("mp/gametypesTable.csv", (XAssetHeader *)&gameTypesTable);
    if ( !gameTypesTable
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare.cpp", 717, 0, "%s", "gameTypesTable") )
    {
        __debugbreak();
    }
    if ( index >= 0 && index < gameTypesTable->rowCount )
    {
        v2 = va("%d", index);
        if ( upperCase )
            result = StringTable_Lookup(gameTypesTable, 4, v2, 1);
        else
            result = StringTable_Lookup(gameTypesTable, 4, v2, 7);
        if ( result && *result )
            return UI_SafeTranslateString(result);
        Com_PrintError(16, "Could not index %d in gameTypesTable.csv.\n", index);
    }
    return (char *)"";
}

char *__cdecl Live_FileShare_GetLocalizedMapNameFromIndex(int index)
{
    if ( index < 0 || index >= sharedUiInfo.mapCount )
        return (char *)"";
    else
        return UI_SafeTranslateString(sharedUiInfo.mapList[index].mapName);
}

int __cdecl Live_FileShare_GetGameTypeCount()
{
    const char *v0; // eax
    const StringTable *gameTypesTable; // [esp+0h] [ebp-4h] BYREF

    StringTable_GetAsset("mp/gameTypesTable.csv", (XAssetHeader *)&gameTypesTable);
    v0 = StringTable_Lookup(gameTypesTable, 0, "total_filesharegametype", 1);
    return atoi(v0);
}

int __cdecl Live_FileShare_GetIndexFromGameType(const char *gameType)
{
    const char *v1; // eax
    const StringTable *gameTypesTable; // [esp+0h] [ebp-4h] BYREF

    StringTable_GetAsset("mp/gametypesTable.csv", (XAssetHeader *)&gameTypesTable);
    if ( !gameTypesTable
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare.cpp", 763, 0, "%s", "gameTypesTable") )
    {
        __debugbreak();
    }
    v1 = StringTable_Lookup(gameTypesTable, 0, gameType, 4);
    return atoi(v1);
}

void __cdecl Live_FileShare_PopulateLastPlayedGameDvars()
{
    char *LocalizedGameTypeFromIndex; // eax
    char *LocalizedMapNameFromIndex; // eax
    unsigned int i; // [esp+8h] [ebp-14h]
    int mapName; // [esp+Ch] [ebp-10h]
    int gameType; // [esp+14h] [ebp-8h]

    for ( i = 0; i < s_lastPlayedGame.descriptor.m_numTags; ++i )
    {
        if ( LODWORD(s_lastPlayedGame.descriptor.m_tags[i].m_priTag) != 1
            || HIDWORD(s_lastPlayedGame.descriptor.m_tags[i].m_priTag) )
        {
            if ( LODWORD(s_lastPlayedGame.descriptor.m_tags[i].m_priTag) == 2
                && !HIDWORD(s_lastPlayedGame.descriptor.m_tags[i].m_priTag) )
            {
                mapName = s_lastPlayedGame.descriptor.m_tags[i].m_secTag;
            }
        }
        else
        {
            gameType = s_lastPlayedGame.descriptor.m_tags[i].m_secTag;
        }
    }
    LocalizedGameTypeFromIndex = Live_FileShare_GetLocalizedGameTypeFromIndex(gameType, 1);
    Dvar_SetString((dvar_s *)fsSelectedFileName, LocalizedGameTypeFromIndex);
    LocalizedMapNameFromIndex = Live_FileShare_GetLocalizedMapNameFromIndex(mapName);
    Dvar_SetString((dvar_s *)fsSelectedFileDescription, LocalizedMapNameFromIndex);
}

void __cdecl Live_FileShare_GetLastPlayedGameDetails(int controllerIndex)
{
    const ddlState_t *RootDDLState; // eax
    unsigned __int64 v2; // rax
    ddlState_t localState; // [esp+0h] [ebp-10h] BYREF

    RootDDLState = LiveStats_GetRootDDLState();
    DDL_MoveTo(RootDDLState, &localState, 2, "AfterActionReportStats", "demoFileID");
    LODWORD(v2) = LiveStats_GetDInt64Stat(controllerIndex, &localState);
    s_lastPlayedGame.dwFileID.m_fileID = v2;
    if ( v2 && s_lastPlayedGame.dwFileID.m_fileID == s_lastPlayedGame.descriptor.m_fileID )
    {
        Live_FileShare_PopulateLastPlayedGameDvars();
    }
    else if ( s_lastPlayedGame.dwFileID.m_fileID )
    {
        LiveStorage_FileShare_ReadMetaDataByID(
            controllerIndex,
            &s_lastPlayedGame.dwFileID,
            1u,
            &s_lastPlayedGame.descriptor,
            FILESHARE_LOCATION_POOLEDSTORAGE);
    }
}

char *__cdecl Live_FileShare_GetFilterList()
{
    char *v0; // eax
    bool filterPresent; // [esp+3h] [ebp-1h] BYREF

    filterPresent = 0;
    if ( s_fsSearchRowValueDvars[1]->current.integer )
    {
        v0 = UI_SafeTranslateString("MENU_FILESHARE_MOSTPOPULAR");
        Live_FileShare_AppendToFilterList(1, v0, &filterPresent);
    }
    else
    {
        if ( fsSearchFileType->current.integer != 4 )
            Live_FileShare_AppendToFilterList(
                s_fsSearchRowValueDvars[2]->current.integer,
                s_fsSearchRowTextDvars[2]->current.string,
                &filterPresent);
        Live_FileShare_AppendToFilterList(
            s_fsSearchRowValueDvars[3]->current.integer,
            s_fsSearchRowTextDvars[3]->current.string,
            &filterPresent);
        Live_FileShare_AppendToFilterList(
            s_fsSearchRowValueDvars[4]->current.integer,
            s_fsSearchRowTextDvars[4]->current.string,
            &filterPresent);
        Live_FileShare_AppendToFilterList(
            s_fsSearchRowValueDvars[5]->current.integer,
            s_fsSearchRowTextDvars[5]->current.string,
            &filterPresent);
        Live_FileShare_AppendToFilterList(
            s_fsSearchRowValueDvars[6]->current.integer,
            s_fsSearchRowTextDvars[6]->current.string,
            &filterPresent);
    }
    if ( filterPresent )
        return s_filterList;
    else
        return UI_SafeTranslateString("MENU_NONE");
}

void __cdecl Live_FileShare_AppendToFilterList(int checkValue, const char *textValue, bool *filterPresent)
{
    char *v3; // eax

    if ( checkValue && textValue && *textValue )
    {
        if ( *filterPresent )
        {
            strcat(s_filterList, ", ");
            strcat(s_filterList, UI_SafeTranslateString(textValue));
        }
        else
        {
            v3 = UI_SafeTranslateString(textValue);
            I_strncpyz(s_filterList, v3, 512);
            *filterPresent = 1;
        }
    }
}

unsigned int __cdecl Live_FileShare_FeederToSlot(unsigned int row, unsigned int col)
{
    if ( col > 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare.cpp",
                    856,
                    0,
                    "%s",
                    "col >= 0 && col < FILESHARE_MAX_SLOTS_PER_ROW") )
    {
        __debugbreak();
    }
    if ( row > 6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare.cpp",
                    857,
                    0,
                    "%s",
                    "row >= 0 && row <= FILESHARE_MAX_NUM_ROWS") )
    {
        __debugbreak();
    }
    return 3 * row + col + 1;
}

char __cdecl Live_FileShare_FileIDFromSlot(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int slot,
                unsigned __int64 *fileID)
{
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < descriptorCount; ++i )
    {
        if ( descriptors[i].m_fileSlot == slot )
        {
            *fileID = descriptors[i].m_fileID;
            return 1;
        }
    }
    return 0;
}

static bool __cdecl Live_FileShare_IsSlotOccupied(int slot, fileShareBufferLocation location)
{
    int i; // [esp+0h] [ebp-8h]
    fileSharePrivateData *fileShareData; // [esp+4h] [ebp-4h]

    fileShareData = LiveStorage_FileShare_GetFileShareData(location);
    for ( i = 0; i < fileShareData->descriptorCount; ++i )
    {
        if ( fileShareData->descriptors[i].m_fileSlot == slot )
            return 1;
    }
    return 0;
}

const char *__cdecl Live_FileShare_GetSelectedImage(bdFileMetaData *descriptors, int index)
{
    unsigned __int64 map; // [esp+Ch] [ebp-8h] BYREF

    if ( !descriptors[index].m_category )
        return "";
    if ( descriptors[index].m_category > 3u )
    {
        if ( descriptors[index].m_category == 4 )
            return "playlist_headquarters";
        return "";
    }
    Live_FileShare_GetTag(descriptors, index, 2u, &map);
    if ( map < sharedUiInfo.mapCount )
        return va("menu_%s_map_select_final", sharedUiInfo.mapList[map].mapLoadName);
    Com_PrintError(16, "File share map tag value out of bounds.\n");
    return "";
}

//unsigned int __cdecl Live_FileShare_GetRating()
//{
//    return s_fileShareFileRating;
//}

char *__cdecl Live_FileShare_LocalizedGameTypeAndMap(int gameTypeIndex, int mapIndex)
{
    const char *v2; // eax
    char *v3; // eax
    const char *gameTypeAndMap[2]; // [esp+0h] [ebp-8h] BYREF

    if ( gameTypeIndex < 0
        || gameTypeIndex >= Live_FileShare_GetGameTypeCount()
        || mapIndex < 0
        || mapIndex >= sharedUiInfo.mapCount )
    {
        Com_PrintError(16, "File share game type or map name tag value out of bounds.\n");
        return (char *)"";
    }
    else
    {
        gameTypeAndMap[0] = Live_FileShare_GetLocalizedGameTypeFromIndex(gameTypeIndex, 1);
        v2 = va("%s_CAPS", sharedUiInfo.mapList[mapIndex].mapName);
        gameTypeAndMap[1] = UI_SafeTranslateString(v2);
        v3 = UI_SafeTranslateString("MENU_FILESHARE_GAMEONMAP");
        return UI_ReplaceConversionStrings(v3, 2, gameTypeAndMap);
    }
}

void __cdecl Live_FileShare_InitRatingHistory()
{
    dword_A4C8928 = 0;
}

void __cdecl Live_FileShare_SaveRating(int controllerIndex, unsigned __int64 fileID, unsigned __int8 rating)
{
#ifdef KISAK_LIVE_SERVICE
    int Xuid; // eax
    int v4; // ecx
    int v5; // edx

    *(_QWORD *)&s_fileShareRatingHistory.files[24 * dword_A4C8928 + 8] = fileID;
    s_fileShareRatingHistory.files[24 * dword_A4C8928 + 16] = rating;
    Xuid = Live_GetXuid(controllerIndex);
    v4 = 24 * dword_A4C8928;
    *(unsigned int *)&s_fileShareRatingHistory.files[v4] = Xuid;
    *(unsigned int *)(v4 + 172783212) = v5;
    if ( ++dword_A4C8928 == 200 )
        dword_A4C8928 = 0;
#endif
}

char __cdecl Live_FileShare_IsRated(int controllerIndex, unsigned __int64 fileID, unsigned __int8 *rating)
{
    __int64 v3; // rax
    int i; // [esp+8h] [ebp-Ch]

    LODWORD(v3) = Live_GetXuid(controllerIndex);
    for ( i = 0; i < 200; ++i )
    {
        if ( fileID == __PAIR64__(
                                         *(unsigned int *)&s_fileShareRatingHistory.files[24 * i + 12],
                                         *(unsigned int *)&s_fileShareRatingHistory.files[24 * i + 8])
            && __PAIR64__(
                     *(unsigned int *)&s_fileShareRatingHistory.files[24 * i + 4],
                     *(unsigned int *)&s_fileShareRatingHistory.files[24 * i]) == v3 )
        {
            *rating = s_fileShareRatingHistory.files[24 * i + 16];
            return 1;
        }
    }
    return 0;
}

char __cdecl Live_FileShare_Theater_SetFile(unsigned __int64 fileID, bool isUserFile)
{
    bdFileMetaData *result; // [esp+Ch] [ebp-4h]

    result = 0;
    s_theaterFileMetaData.m_fileID = 0;
    if ( isUserFile )
    {
        result = Live_FileShare_FindFileInPrivateData(fileID, FILESHARE_BUFFER_PRIMARY);
        if ( !result )
            result = Live_FileShare_FindFileInPrivateData(fileID, FILESHARE_BUFFER_SECONDARY);
    }
    if ( !result )
        result = (bdFileMetaData *)RETURN_ZERO32();
    if ( result )
    {
        memcpy(&s_theaterFileMetaData, result, sizeof(s_theaterFileMetaData));
        return 1;
    }
    else
    {
        if ( isUserFile )
            Com_PrintError(16, "File ID %llu not found in %s storage.\n", fileID, "User");
        else
            Com_PrintError(16, "File ID %llu not found in %s storage.\n", fileID, "Pooled");
        return 0;
    }
}

char __cdecl Live_FileShare_Theater_GetSize(int *size)
{
    if ( !s_theaterFileMetaData.m_fileID )
        return 0;
    *size = s_theaterFileMetaData.m_fileSize;
    return 1;
}

char __cdecl Live_FileShare_Theater_GetType(int *type)
{
    if ( !s_theaterFileMetaData.m_fileID )
        return 0;
    *type = s_theaterFileMetaData.m_category;
    return 1;
}

char __cdecl Live_FileShare_Theater_GetName(char **name)
{
    ddlState_t state; // [esp+0h] [ebp-18h] BYREF
    char *buffer; // [esp+10h] [ebp-8h]
    int isModifiedName; // [esp+14h] [ebp-4h]

    buffer = (char *)s_theaterFileMetaData.m_metaData;
    isModifiedName = 0;
    if ( !s_theaterFileMetaData.m_fileID || !Live_FileShare_ValidateDDLBuffer(buffer) )
        return 0;
    if ( DDL_MoveToName(&g_fileShareRootState, &state, "isModifiedName") )
    {
        isModifiedName = DDL_GetInt(&state, buffer);
        if ( !isModifiedName )
        {
            Live_FileShare_Theater_GetMap(name);
            return 1;
        }
        if ( DDL_MoveToName(&g_fileShareRootState, &state, "name") )
        {
            *name = DDL_GetString(&state, buffer);
            return 1;
        }
    }
    return 0;
}

char __cdecl Live_FileShare_Theater_GetLength(int *length)
{
    ddlState_t state; // [esp+0h] [ebp-14h] BYREF
    char *buffer; // [esp+10h] [ebp-4h]

    buffer = (char *)s_theaterFileMetaData.m_metaData;
    if ( !s_theaterFileMetaData.m_fileID || !Live_FileShare_ValidateDDLBuffer(buffer) )
        return 0;
    if ( !DDL_MoveToName(&g_fileShareRootState, &state, "length") )
        return 0;
    *length = DDL_GetInt(&state, buffer);
    return 1;
}

char __cdecl Live_FileShare_Theater_GetDescription(char **description)
{
    ddlState_t state; // [esp+0h] [ebp-14h] BYREF
    char *buffer; // [esp+10h] [ebp-4h]

    buffer = (char *)s_theaterFileMetaData.m_metaData;
    if ( !s_theaterFileMetaData.m_fileID || !Live_FileShare_ValidateDDLBuffer(buffer) )
        return 0;
    if ( !DDL_MoveToName(&g_fileShareRootState, &state, "description") )
        return 0;
    *description = DDL_GetString(&state, buffer);
    return 1;
}

char __cdecl Live_FileShare_Theater_GetMap(char **mapName)
{
    unsigned __int64 mapTag; // [esp+0h] [ebp-8h] BYREF

    mapTag = 0;
    if ( !s_theaterFileMetaData.m_fileID || !Live_FileShare_GetTag(&s_theaterFileMetaData, 0, 2u, &mapTag) )
        return 0;
    *mapName = Live_FileShare_GetLocalizedMapNameFromIndex(mapTag);
    return 1;
}

char __cdecl Live_FileShare_Theater_GetGameType(char **gameTypeName)
{
    unsigned __int64 gameTypeTag; // [esp+0h] [ebp-8h] BYREF

    gameTypeTag = 0;
    if ( !s_theaterFileMetaData.m_fileID || !Live_FileShare_GetTag(&s_theaterFileMetaData, 0, 1u, &gameTypeTag) )
        return 0;
    *gameTypeName = Live_FileShare_GetLocalizedGameTypeFromIndex(gameTypeTag, 1);
    return 1;
}

int __cdecl Live_FileShare_GetSearchResultsCount()
{
    return Live_FileShareSearch_GetTotalFiles();
}

char *__cdecl Live_FileShare_SearchResultsItemText(
                int controllerIndex,
                int startIndex,
                int selectedIndex,
                int itemIndex,
                bool *useOwnerDraw)
{
    const char *v6; // [esp+4h] [ebp-850h]
    char *v7; // [esp+8h] [ebp-84Ch]
    float floatRes; // [esp+Ch] [ebp-848h] BYREF
    const char *stringRes; // [esp+10h] [ebp-844h] BYREF
    bdFileMetaData descriptor; // [esp+14h] [ebp-840h] BYREF

    *useOwnerDraw = 0;
    if ( !Live_FileShareSearch_LoadDescriptor(controllerIndex, startIndex, selectedIndex, itemIndex) )
        goto LABEL_10;
    //bdFileMetaData::bdFileMetaData(&descriptor);
    if ( !Live_FileShareSearch_GetDescriptor(controllerIndex, itemIndex, &descriptor) )
    {
        ////bdFileMetaData::~bdFileMetaData(&descriptor);
LABEL_10:
        *useOwnerDraw = 1;
        return (char *)"";
    }
    if ( fshDebugFileList && fshDebugFileList->current.enabled )
    {
        v7 = va("%lld", descriptor.m_fileID);
        ////bdFileMetaData::~bdFileMetaData(&descriptor);
        return v7;
    }
    else if ( Live_FileShare_GetName(
                            &descriptor,
                            1,
                            0,
                            FILESHARE_INFOLOCATION_SEARCHRESULTS,
                            (char **)&stringRes,
                            &floatRes) )
    {
        v6 = stringRes;
        ////bdFileMetaData::~bdFileMetaData(&descriptor);
        return (char *)v6;
    }
    else
    {
        Live_FileShareSearch_MarkItemCorrupt(controllerIndex, itemIndex);
        ////bdFileMetaData::~bdFileMetaData(&descriptor);
        return (char *)"";
    }
}

int __cdecl Live_FileShare_GetMyPrivateSlotsCount()
{
    if ( CG_IsShowingZombieMap() )
        return 6;
    else
        return 2;
}

int __cdecl Live_FileShare_GetOtherPrivateSlotsCount()
{
    return fsMaxPrivateSlotRowsOther->current.integer;
}

char *__cdecl Live_FileShare_PrivateSlotsText(
                int controllerIndex,
                int feederID,
                unsigned int itemIndex,
                int column,
                Material **handle)
{
#ifdef KISAK_LIVE_SERVIC
    char *result; // eax
    Material *v6; // eax
    char *SelectedImage; // eax
    int j; // [esp+8h] [ebp-14h]
    int k; // [esp+Ch] [ebp-10h]
    int i; // [esp+10h] [ebp-Ch]
    fileSharePrivateData *fileShareData; // [esp+14h] [ebp-8h]
    unsigned int currSlot; // [esp+18h] [ebp-4h]

    currSlot = Live_FileShare_FeederToSlot(itemIndex, column % 3);
    if ( feederID == 80 || feederID == 86 )
    {
        fileShareData = LiveStorage_FileShare_GetFileShareData(FILESHARE_BUFFER_PRIMARY);
    }
    else if ( feederID == 79 )
    {
        fileShareData = LiveStorage_FileShare_GetFileShareData(FILESHARE_BUFFER_SECONDARY);
    }
    else if ( !Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare.cpp",
                             1313,
                             0,
                             "Invalid file share feeder.") )
    {
        __debugbreak();
    }
    if ( !fileShareData
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare.cpp", 1316, 0, "%s", "fileShareData") )
    {
        __debugbreak();
    }
    if ( !fileShareData )
        return (char *)"";
    switch ( column )
    {
        case 0:
        case 1:
        case 2:
            *handle = Material_RegisterHandle("white", 7);
            result = (char *)"";
            break;
        case 3:
        case 4:
        case 5:
            for ( i = 0; ; ++i )
            {
                if ( i >= fileShareData->descriptorCount )
                {
                    *handle = Material_RegisterHandle("white", 7);
                    goto LABEL_40;
                }
                if ( fileShareData->descriptors[i].m_fileSlot == currSlot )
                    break;
            }
            if ( fileShareData->descriptors[i].m_category == 4 )
            {
                v6 = Material_RegisterHandle("menu_mp_fileshare_custom", 7);
            }
            else
            {
                SelectedImage = (char *)Live_FileShare_GetSelectedImage(fileShareData->descriptors, i);
                v6 = Material_RegisterHandle(SelectedImage, 7);
            }
            *handle = v6;
            result = (char *)"";
            break;
        case 6:
        case 7:
        case 8:
            for ( j = 0; ; ++j )
            {
                if ( j >= fileShareData->descriptorCount )
                    goto LABEL_40;
                if ( fileShareData->descriptors[j].m_fileSlot == currSlot )
                    break;
LABEL_31:
                ;
            }
            switch ( fileShareData->descriptors[j].m_category )
            {
                case 1u:
                    *handle = Material_RegisterHandle("menu_mp_lobby_icon_film", 7);
                    result = (char *)"";
                    break;
                case 2u:
                    *handle = Material_RegisterHandle("menu_mp_lobby_icon_clip", 7);
                    result = (char *)"";
                    break;
                case 3u:
                    *handle = Material_RegisterHandle("menu_mp_lobby_icon_screenshot", 7);
                    result = (char *)"";
                    break;
                case 4u:
                    *handle = Material_RegisterHandle("menu_mp_lobby_icon_customgamemode", 7);
                    result = (char *)"";
                    break;
                default:
                    goto LABEL_31;
            }
            break;
        case 9:
        case 10:
        case 11:
            for ( k = 0; k < fileShareData->descriptorCount; ++k )
            {
                if ( fileShareData->descriptors[k].m_fileSlot == currSlot )
                    return (char *)"";
            }
            result = va("%d", currSlot);
            break;
        default:
LABEL_40:
            result = (char *)"";
            break;
    }
    return result;
#else
return NULL;
#endif
}

const char *__cdecl Live_FileShare_PrivateSlotsIngameText(
                int controllerIndex,
                int itemIndex,
                int column,
                Material **handle)
{
    int i; // [esp+4h] [ebp-18h]
    float result; // [esp+8h] [ebp-14h] BYREF
    int descriptorIndex; // [esp+Ch] [ebp-10h]
    fileSharePrivateData *fileShareData; // [esp+10h] [ebp-Ch]
    const char *stringResult; // [esp+14h] [ebp-8h] BYREF
    int curSlot; // [esp+18h] [ebp-4h]

    descriptorIndex = -1;
    curSlot = itemIndex + 1;
    fileShareData = LiveStorage_FileShare_GetFileShareData(FILESHARE_BUFFER_PRIMARY);
    if ( !fileShareData
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare.cpp", 1411, 0, "%s", "fileShareData") )
    {
        __debugbreak();
    }
    if ( !fileShareData )
        return "";
    for ( i = 0; i < fileShareData->descriptorCount; ++i )
    {
        if ( fileShareData->descriptors[i].m_fileSlot == curSlot )
        {
            descriptorIndex = i;
            break;
        }
    }
    if ( !column )
        return va("%d", curSlot);
    if ( column == 1 )
    {
        if ( descriptorIndex >= 0
            && Live_FileShare_GetItemInfo(
                     controllerIndex,
                     FILESHARE_KEY_TYPEICONNAME,
                     itemIndex,
                     FILESHARE_INFOLOCATION_MYFILESHARE_INGAME,
                     &stringResult,
                     &result) )
        {
            *handle = Material_RegisterHandle((char *)stringResult, 7);
            return "";
        }
    }
    else if ( column == 2
                 && descriptorIndex >= 0
                 && Live_FileShare_GetName(
                            fileShareData->descriptors,
                            fileShareData->descriptorCount,
                            descriptorIndex,
                            FILESHARE_INFOLOCATION_MYFILESHARE_INGAME,
                            (char **)&stringResult,
                            &result) )
    {
        return stringResult;
    }
    return "";
}

void __cdecl Live_FileShare_FeederColor(
                listBoxDef_s *listPtr,
                itemDef_s *item,
                int contextIndex,
                unsigned int index,
                int column,
                fileShareBufferLocation bufferLocation,
                DvarValue *color)
{
    unsigned int v7; // eax
    char isSlotOccupied; // [esp+3h] [ebp-Dh]
    unsigned int slotNum; // [esp+4h] [ebp-Ch]
    int fileShareColumn; // [esp+8h] [ebp-8h]
    bool isSlotHighlighted; // [esp+Fh] [ebp-1h]

    if ( column >= 0 )
    {
        fileShareColumn = column % 3;
        slotNum = Live_FileShare_FeederToSlot(index, column % 3);
        v7 = Live_FileShare_FeederToSlot(index, column % 3);
        isSlotOccupied = Live_FileShare_IsSlotOccupied(v7, bufferLocation);
        isSlotHighlighted = 0;
        if ( Window_HasFocus(contextIndex, &item->window) )
        {
            if ( bufferLocation )
            {
                if ( listPtr->cursorPos[contextIndex] == index )
                    isSlotHighlighted = fileShareColumn == fsOtherUserPrivateSlotCol->current.integer;
            }
            else if ( listPtr->cursorPos[contextIndex] == index )
            {
                isSlotHighlighted = fileShareColumn == fsPrivateSlotCol->current.integer;
            }
        }
        if ( isSlotOccupied )
            Live_FileShare_ColorOccupiedSlot(column, color, isSlotHighlighted);
        else
            Live_FileShare_ColorEmptySlot(column, slotNum, color, isSlotHighlighted);
    }
}

void __cdecl Live_FileShare_ColorEmptySlot(int column, int slotNum, DvarValue *color, bool isHighlighted)
{
    switch ( column )
    {
        case 0:
        case 1:
        case 2:
            if ( isHighlighted )
                *color = fsSlotHighlightedColorNoSel->current;
            else
                *color = fsSlotEmptyShowColorBg->current;
            break;
        case 3:
        case 4:
        case 5:
            *color = fsSlotEmptyMainColor->current;
            break;
        case 9:
        case 10:
        case 11:
            *color = fsSlotNumMainColor->current;
            break;
        default:
            *color = fsSlotEmptyHiddenColor->current;
            break;
    }
}

void __cdecl Live_FileShare_ColorOccupiedSlot(unsigned int column, DvarValue *color, bool isHighlighted)
{
    if ( column > 2 )
    {
        *color = fsSlotMainColor->current;
    }
    else if ( isHighlighted )
    {
        *color = fsSlotHighlightedColor->current;
    }
    else
    {
        *color = fsSlotEmptyShowColorBg->current;
    }
}

char __cdecl Live_FileShare_GetDate(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult)
{
    qtime_s qTime; // [esp+0h] [ebp-2Ch] BYREF
    int createTime; // [esp+24h] [ebp-8h] BYREF
    const char *stringRes; // [esp+28h] [ebp-4h] BYREF

    if ( Live_FileShare_ReadFromMetaData(2, &descriptors[index], (char **)&stringRes, &createTime) )
    {
        createTime += LiveStorage_GetUTCOffset();
        Com_GetQTime(createTime, &qTime, 1);
        *stringResult = va(
                                            "%d/%d/%d    %.2d:%.2d",
                                            qTime.tm_mon + 1,
                                            qTime.tm_mday,
                                            qTime.tm_year + 1900,
                                            qTime.tm_hour,
                                            qTime.tm_min);
        return 1;
    }
    else
    {
        *stringResult = "";
        return 0;
    }
}

char __cdecl Live_FileShare_GetLength(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult)
{
    int intRes; // [esp+0h] [ebp-10h] BYREF
    int demoTimeMin; // [esp+4h] [ebp-Ch]
    int demoTimeSec; // [esp+8h] [ebp-8h]
    const char *stringRes; // [esp+Ch] [ebp-4h] BYREF

    if ( Live_FileShare_ReadFromMetaData(3, &descriptors[index], (char **)&stringRes, &intRes) )
    {
        demoTimeMin = intRes / 60000;
        demoTimeSec = intRes % 60000 / 1000;
        *stringResult = va("%02i:%02i", intRes / 60000, demoTimeSec);
        return 1;
    }
    else
    {
        *stringResult = "";
        return 0;
    }
}

char __cdecl Live_FileShare_GetSize(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                char **stringResult)
{
    char *v5; // eax
    char *v6; // eax
    const char *v8; // [esp+4h] [ebp-20h]
    float fileSize; // [esp+20h] [ebp-4h]
    float fileSizea; // [esp+20h] [ebp-4h]

    if ( (double)descriptors[index].m_fileSize < 1048576.0 )
    {
        fileSizea = (double)descriptors[index].m_fileSize / 1024.0;
        v8 = va("%0.2f", fileSizea);
        v5 = UI_SafeTranslateString("MENU_FILESHARE_FILESIZEKB");
    }
    else
    {
        fileSize = (double)descriptors[index].m_fileSize / 1048576.0;
        v8 = va("%0.2f", fileSize);
        v5 = UI_SafeTranslateString("MENU_FILESHARE_FILESIZEMB");
    }
    v6 = UI_ReplaceConversionString(v5, v8);
    *stringResult = v6;
    return 1;
}

char __cdecl Live_FileShare_GetMap(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult)
{
    unsigned __int64 tagValue; // [esp+8h] [ebp-8h] BYREF

    if ( Live_FileShare_GetTag(descriptors, index, 2u, &tagValue) )
    {
        if ( tagValue < sharedUiInfo.mapCount )
        {
            *stringResult = va("%s", sharedUiInfo.mapList[tagValue].mapLoadName);
            return 1;
        }
        Com_PrintError(16, "Fileshare: Value not in sharedUiInfo bounds.\n");
    }
    *stringResult = "";
    return 0;
}

char __cdecl Live_FileShare_GetMapName(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult)
{
    unsigned __int64 tagValue; // [esp+8h] [ebp-8h] BYREF

    if ( Live_FileShare_GetTag(descriptors, index, 2u, &tagValue) )
    {
        if ( tagValue < sharedUiInfo.mapCount )
        {
            *stringResult = va("@%s_CAPS", sharedUiInfo.mapList[tagValue].mapName);
            return 1;
        }
        Com_PrintError(16, "Fileshare: Value not in sharedUiInfo bounds.\n");
    }
    *stringResult = "";
    return 0;
}

char __cdecl Live_FileShare_GetGameType(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult)
{
    unsigned __int64 tagValue; // [esp+0h] [ebp-8h] BYREF

    if ( Live_FileShare_GetTag(descriptors, index, 1u, &tagValue) )
    {
        *stringResult = Live_FileShare_GetGameTypeFromIndex(tagValue);
        return 1;
    }
    else
    {
        *stringResult = "";
        return 0;
    }
}

char __cdecl Live_FileShare_GetGameTypeName(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                char **stringResult)
{
    unsigned __int64 tagValue; // [esp+8h] [ebp-8h] BYREF

    if ( Live_FileShare_GetTag(descriptors, index, 1u, &tagValue) )
    {
        if ( tagValue < Live_FileShare_GetGameTypeCount() )
        {
            *stringResult = Live_FileShare_GetLocalizedGameTypeFromIndex(tagValue, 1);
            return 1;
        }
        Com_PrintError(16, "File share game type tag value out of bounds.\n");
    }
    *stringResult = (char *)"";
    return 0;
}

char __cdecl Live_FileShare_GetGameTypeImage(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult)
{
    unsigned __int64 tagValue; // [esp+8h] [ebp-8h] BYREF

    if ( Live_FileShare_GetTag(descriptors, index, 1u, &tagValue) )
    {
        if ( tagValue < Live_FileShare_GetGameTypeCount() )
        {
            *stringResult = Live_FileShare_GetGameTypeImageFromIndex(tagValue);
            return 1;
        }
        Com_PrintError(16, "File share game type tag value out of bounds.\n");
    }
    *stringResult = "";
    return 0;
}

char __cdecl Live_FileShare_GetGameTypeAndMapName(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                char **stringResult)
{
    unsigned __int64 mapName; // [esp+0h] [ebp-10h] BYREF
    unsigned __int64 gameType; // [esp+8h] [ebp-8h] BYREF

    if ( Live_FileShare_GetTag(descriptors, index, 1u, &gameType)
        && Live_FileShare_GetTag(descriptors, index, 2u, &mapName) )
    {
        *stringResult = Live_FileShare_LocalizedGameTypeAndMap(gameType, mapName);
        return 1;
    }
    else
    {
        *stringResult = (char *)"";
        return 0;
    }
}

char __cdecl Live_FileShare_HasSummary(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult)
{
    if ( descriptors[index].m_summaryFileSize )
    {
        *floatResult = 1.0f;
        if ( stringResult && *stringResult )
            *stringResult = va("%d", 1);
    }
    else
    {
        *floatResult = 0.0f;
        if ( stringResult && *stringResult )
            *stringResult = va("%d", 0);
    }
    return 1;
}

char __cdecl Live_FileShare_IsSummaryCached(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult)
{
    *floatResult = 0.0f;
    return 1;
}

char __cdecl Live_FileShare_IsSummaryLoaded(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult)
{
    if ( Live_FileShareSearch_IsSummaryLoaded() )
        *floatResult = 1.0f;
    else
        *floatResult = 0.0f;
    return 1;
}

bool __cdecl Live_FileShare_GetFileType(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult)
{
    bool result; // al

    switch ( descriptors[index].m_category )
    {
        case 1u:
            *stringResult = "film";
            result = 1;
            break;
        case 2u:
            *stringResult = "clip";
            result = 1;
            break;
        case 3u:
            *stringResult = "screenshot";
            result = 1;
            break;
        case 4u:
            *stringResult = "customgamemode";
            result = 1;
            break;
        default:
            *stringResult = "";
            result = 0;
            break;
    }
    return result;
}

char __cdecl Live_FileShare_GetUserTag(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                char **stringResult)
{
    const char *v5; // eax
    const char *result; // [esp+0h] [ebp-10h]
    const StringTable *fileShareTable; // [esp+4h] [ebp-Ch] BYREF
    unsigned __int64 userTag; // [esp+8h] [ebp-8h] BYREF

    if ( Live_FileShare_GetTag(descriptors, index, 5u, &userTag) )
    {
        StringTable_GetAsset("mp/fileShareTags.csv", (XAssetHeader *)&fileShareTable);
        if ( (int)userTag > 0 && (int)userTag <= fileShareTable->rowCount )
        {
            v5 = va("%llu", userTag);
            result = StringTable_Lookup(fileShareTable, 0, v5, 1);
            if ( result )
            {
                if ( *result )
                {
                    *stringResult = UI_SafeTranslateString(result);
                    return 1;
                }
            }
        }
        Com_PrintError(16, "User tag index out range or tag value invalid.\n");
    }
    *stringResult = UI_SafeTranslateString("MPUI_FILESHARE_TAG_NONE");
    return 1;
}

char __cdecl Live_FileShare_GetUserTagIndex(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult)
{
    const StringTable *fileShareTable; // [esp+10h] [ebp-Ch] BYREF
    unsigned __int64 userTag; // [esp+14h] [ebp-8h] BYREF

    if ( Live_FileShare_GetTag(descriptors, index, 5u, &userTag) )
    {
        StringTable_GetAsset("mp/fileShareTags.csv", (XAssetHeader *)&fileShareTable);
        if ( (int)userTag > 0 && (int)userTag <= fileShareTable->rowCount )
        {
            *floatResult = (float)userTag;
            *stringResult = va("%llu", userTag);
            return 1;
        }
        Com_PrintError(16, "User tag index out range or tag value invalid.\n");
    }
    *floatResult = 0.0f;
    *stringResult = va("0");
    return 1;
}

char __cdecl Live_FileShare_GetFileID(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult)
{
    unsigned __int64 fileID; // [esp+4h] [ebp-8h] BYREF

    if ( location == FILESHARE_INFOLOCATION_SEARCHRESULTS || location == FILESHARE_INFOLOCATION_MYFILESHARE )
    {
        *stringResult = va("%llu", descriptors[index].m_fileID);
        return 1;
    }
    if ( location == FILESHARE_INFOLOCATION_OTHERFILESHARE )
    {
        if ( fsOtherUserSlotSelected
            && fsOtherUserSlotSelected->current.integer > 0
            && Live_FileShare_FileIDFromSlot(descriptors, descriptorCount, fsOtherUserSlotSelected->current.integer, &fileID) )
        {
            *stringResult = va("%llu", fileID);
            return 1;
        }
        Com_PrintError(16, "getFeederData could not get fileID for location %d, index %d.\n", location, index);
    }
    *stringResult = "";
    return 0;
}

char __cdecl Live_FileShare_LookupRating(
                unsigned __int64 fileID,
                bdVoteRankStatsInfo *ratings,
                int numRatings,
                float *rating)
{
    int i; // [esp+10h] [ebp-4h]

    for ( i = 0; i < numRatings; ++i )
    {
        if ( ratings[i].m_entityID == fileID )
        {
            *rating = (double)ratings[i].m_avgVoteValue / 500.0;
            return 1;
        }
    }
    return 0;
}

char __cdecl Live_FileShare_LookupRank(
                unsigned __int64 fileID,
                bdVoteRankStatsInfo *ratings,
                int numRatings,
                int *rank)
{
    int i; // [esp+8h] [ebp-4h]

    for ( i = 0; i < numRatings; ++i )
    {
        if ( ratings[i].m_entityID == fileID )
        {
            *rank = ratings[i].m_rank;
            return 1;
        }
    }
    return 0;
}

double __cdecl Live_FileShare_LookupTotalVotes(unsigned __int64 fileID, bdVoteRankStatsInfo *ratings, int numRatings)
{
    int i; // [esp+10h] [ebp-4h]

    for ( i = 0; i < numRatings; ++i )
    {
        if ( ratings[i].m_entityID == fileID )
            return (double)ratings[i].m_totalVotes;
    }
    return 0.0;
}

char __cdecl Live_FileShare_GetRating(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult)
{
    int numRatings; // [esp+0h] [ebp-8h] BYREF
    bdVoteRankStatsInfo *ratings; // [esp+4h] [ebp-4h] BYREF

    if ( location == FILESHARE_INFOLOCATION_SEARCHRESULTS )
    {
        Live_FileShareSearch_GetRatings(&ratings, &numRatings);
        if ( Live_FileShare_LookupRating(descriptors[index].m_fileID, ratings, numRatings, floatResult) )
            return 1;
    }
    *floatResult = 0.0f;
    return 0;
}

char __cdecl Live_FileShare_GetTotalVotes(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult)
{
    int numRatings; // [esp+0h] [ebp-8h] BYREF
    bdVoteRankStatsInfo *ratings; // [esp+4h] [ebp-4h] BYREF

    if ( location == FILESHARE_INFOLOCATION_SEARCHRESULTS )
    {
        Live_FileShareSearch_GetRatings(&ratings, &numRatings);
        *floatResult = Live_FileShare_LookupTotalVotes(descriptors[index].m_fileID, ratings, numRatings);
        return 1;
    }
    else
    {
        *floatResult = 0.0f;
        return 0;
    }
}

char __cdecl Live_FileShare_GetOwnerName(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                char **stringResult)
{
    int intRes; // [esp+0h] [ebp-4h] BYREF

    if ( Live_FileShare_ReadFromMetaData(0, &descriptors[index], stringResult, &intRes) )
        return 1;
    *stringResult = (char *)"";
    return 0;
}

char __cdecl Live_FileShare_GetFileSize(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult)
{
    *stringResult = va("%d", descriptors[index].m_fileSize);
    return 1;
}

char __cdecl Live_FileShare_GetFileName(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                char **stringResult)
{
    *stringResult = descriptors[index].m_fileName;
    return 1;
}

bool __cdecl Live_FileShare_GetTypeIconName(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult)
{
    bool result; // al

    switch ( descriptors[index].m_category )
    {
        case 1u:
            *stringResult = "menu_mp_lobby_icon_film";
            result = 1;
            break;
        case 2u:
            *stringResult = "menu_mp_lobby_icon_clip";
            result = 1;
            break;
        case 3u:
            *stringResult = "menu_mp_lobby_icon_screenshot";
            result = 1;
            break;
        case 4u:
            *stringResult = "menu_mp_lobby_icon_customgamemode";
            result = 1;
            break;
        default:
            *stringResult = "";
            result = 0;
            break;
    }
    return result;
}

char __cdecl Live_FileShare_GetSlot(
                bdFileMetaData *descriptors,
                int descriptorCount,
                unsigned int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult)
{
    float v6; // xmm0_4

    if ( location == FILESHARE_INFOLOCATION_OTHERFILESHARE )
        v6 = (float)(int)Live_FileShare_FeederToSlot(index, fsOtherUserPrivateSlotCol->current.unsignedInt);
    else
        v6 = (float)(int)Live_FileShare_FeederToSlot(index, fsPrivateSlotCol->current.unsignedInt);
    *floatResult = v6;
    return 1;
}

char __cdecl Live_FileShare_GetIngameSlot(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult)
{
    *floatResult = (float)(index + 1);
    return 1;
}

bool __cdecl Live_FileShare_IsSlotOccupied(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult)
{
    if ( descriptors[index].m_fileSlot && descriptors[index].m_fileSize )
    {
        *floatResult = 1.0f;
        *stringResult = va("1");
        return 1;
    }
    else
    {
        *floatResult = 0.0f;
        *stringResult = va("0");
        return 1;
    }
}

bool __cdecl Live_FileShare_GetSelectedImageName(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult)
{
    *stringResult = Live_FileShare_GetSelectedImage(descriptors, index);
    return **stringResult != 0;
}

char __cdecl Live_FileShare_GetIsModifiedName(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                char **stringResult,
                float *floatResult)
{
    int intRes; // [esp+0h] [ebp-4h] BYREF

    if ( Live_FileShare_ReadFromMetaData(6, &descriptors[index], stringResult, &intRes) )
    {
        *floatResult = (float)intRes;
        return 1;
    }
    else
    {
        *stringResult = (char *)"";
        return 0;
    }
}

char __cdecl Live_FileShare_GetIsModifiedDescription(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                char **stringResult,
                float *floatResult)
{
    int intRes; // [esp+0h] [ebp-4h] BYREF

    if ( Live_FileShare_ReadFromMetaData(7, &descriptors[index], stringResult, &intRes) )
    {
        *floatResult = (float)intRes;
        return 1;
    }
    else
    {
        *stringResult = (char *)"";
        return 0;
    }
}

char __cdecl Live_FileShare_GetName(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                char **stringResult,
                float *floatResult)
{
    int intRes; // [esp+0h] [ebp-8h] BYREF
    int isModified; // [esp+4h] [ebp-4h]

    isModified = 0;
    if ( Live_FileShare_GetIsModifiedName(descriptors, descriptorCount, index, location, stringResult, floatResult) )
        isModified = (int)*floatResult;
    if ( isModified )
    {
        if ( Live_FileShare_ReadFromMetaData(4, &descriptors[index], stringResult, &intRes) )
            return 1;
    }
    else if ( descriptors[index].m_category == 4 )
    {
        if ( Live_FileShare_GetGameTypeName(descriptors, descriptorCount, index, location, stringResult) )
            return 1;
    }
    else if ( Live_FileShare_GetMapName(descriptors, descriptorCount, index, location, (const char **)stringResult) )
    {
        *stringResult = UI_SafeTranslateString(*stringResult + 1);
        return 1;
    }
    *stringResult = (char *)"";
    return 0;
}

char __cdecl Live_FileShare_GetDescription(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                char **stringResult,
                float *floatResult)
{
    int intRes; // [esp+0h] [ebp-8h] BYREF
    int isModified; // [esp+4h] [ebp-4h]

    isModified = 0;
    if ( Live_FileShare_GetIsModifiedDescription(descriptors, descriptorCount, index, location, stringResult, floatResult) )
        isModified = (int)*floatResult;
    if ( isModified )
    {
        if ( Live_FileShare_ReadFromMetaData(5, &descriptors[index], stringResult, &intRes) )
            return 1;
    }
    else
    {
        if ( descriptors[index].m_category == 4 )
        {
            *stringResult = UI_SafeTranslateString("CUSTOM_CUSTOM_GAME_MODE_CAPS");
            return 1;
        }
        if ( Live_FileShare_GetGameTypeName(descriptors, descriptorCount, index, location, stringResult) )
            return 1;
    }
    *stringResult = (char *)"";
    return 0;
}

const char *__cdecl Live_FileShare_GetPooledTeamIcon(int mapID, int team)
{
    const char *v3; // eax
    const StringTable *mapTable; // [esp+0h] [ebp-4h] BYREF

    StringTable_GetAsset("mp/mapsTable.csv", (XAssetHeader *)&mapTable);
    if ( !mapTable
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare.cpp", 2182, 0, "%s", "mapTable") )
    {
        __debugbreak();
    }
    if ( !mapTable )
        return "";
    v3 = va("%d", mapID);
    if ( team == 1 )
        return StringTable_Lookup(mapTable, 5, v3, 15);
    else
        return StringTable_Lookup(mapTable, 5, v3, 14);
}

char *__cdecl Live_FileShare_GetPooledTeamName(int mapID, int team)
{
    const char *v3; // eax
    const char *v4; // eax
    const StringTable *mapTable; // [esp+0h] [ebp-4h] BYREF

    StringTable_GetAsset("mp/mapsTable.csv", (XAssetHeader *)&mapTable);
    if ( !mapTable
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare.cpp", 2234, 0, "%s", "mapTable") )
    {
        __debugbreak();
    }
    if ( !mapTable )
        return (char *)"";
    v3 = va("%d", mapID);
    if ( team == 1 )
        v4 = StringTable_Lookup(mapTable, 5, v3, 13);
    else
        v4 = StringTable_Lookup(mapTable, 5, v3, 12);
    return UI_SafeTranslateString(v4);
}

int __cdecl Live_FileShare_GetSortBy()
{
    return s_fsSearchRowValueDvars[1]->current.integer;
}

void __cdecl Live_FileShare_ExpirePooledFileDetails()
{
    s_pooledFileDetails.isValid = 0;
}

void __cdecl Live_FileShare_SetPooledFileDetailsContext(fileSharePooledFileContext_t context)
{
    if ( s_pooledFileDetails.context != context )
        memset(&s_pooledFileDetails, 0, sizeof(s_pooledFileDetails));
    s_pooledFileDetails.context = context;
}

const char *__cdecl Live_FileShare_GetPooledFileInfo(const char *field)
{
    if ( I_stricmp("isValid", field) )
    {
        if ( I_stricmp("mapKills", field) )
        {
            if ( I_stricmp("mapDeaths", field) )
            {
                if ( I_stricmp("mapTeam", field) )
                {
                    if ( I_stricmp("mapAxisScore", field) )
                    {
                        if ( I_stricmp("mapAxisIcon", field) )
                        {
                            if ( I_stricmp("mapAxisName", field) )
                            {
                                if ( I_stricmp("mapAlliesScore", field) )
                                {
                                    if ( I_stricmp("mapAlliesIcon", field) )
                                    {
                                        if ( I_stricmp("mapAlliesName", field) )
                                        {
                                            if ( I_stricmp("mapVictory", field) )
                                            {
                                                if ( I_stricmp("isTeamBased", field) )
                                                {
                                                    if ( I_stricmp("score", field) )
                                                    {
                                                        if ( I_stricmp("wagerPayout", field) )
                                                        {
                                                            if ( I_stricmp("position", field) )
                                                            {
                                                                if ( I_stricmp("isWager", field) )
                                                                {
                                                                    if ( !Assert_MyHandler(
                                                                                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare.cpp",
                                                                                    2470,
                                                                                    0,
                                                                                    "Invalid key") )
                                                                        __debugbreak();
                                                                    return "";
                                                                }
                                                                else
                                                                {
                                                                    return Live_FileShare_GetPooledIsWager();
                                                                }
                                                            }
                                                            else
                                                            {
                                                                return Live_FileShare_GetPooledPosition();
                                                            }
                                                        }
                                                        else
                                                        {
                                                            return Live_FileShare_GetPooledWagerPayout();
                                                        }
                                                    }
                                                    else
                                                    {
                                                        return Live_FileShare_GetPooledScore();
                                                    }
                                                }
                                                else
                                                {
                                                    return Live_FileShare_IsPooledTeamBased();
                                                }
                                            }
                                            else
                                            {
                                                return Live_FileShare_GetPooledMapVictory();
                                            }
                                        }
                                        else
                                        {
                                            return Live_FileShare_GetPooledMapAlliesName();
                                        }
                                    }
                                    else
                                    {
                                        return Live_FileShare_GetPooledMapAlliesIcon();
                                    }
                                }
                                else
                                {
                                    return Live_FileShare_GetPooledMapAlliesScore();
                                }
                            }
                            else
                            {
                                return Live_FileShare_GetPooledMapAxisName();
                            }
                        }
                        else
                        {
                            return Live_FileShare_GetPooledMapAxisIcon();
                        }
                    }
                    else
                    {
                        return Live_FileShare_GetPooledMapAxisScore();
                    }
                }
                else
                {
                    return Live_FileShare_GetPooledMapTeam();
                }
            }
            else
            {
                return Live_FileShare_GetPooledMapDeaths();
            }
        }
        else
        {
            return Live_FileShare_GetPooledMapKills();
        }
    }
    else if ( s_pooledFileDetails.isValid )
    {
        return va("%d", 1);
    }
    else
    {
        return va("%d", 0);
    }
}

const char *__cdecl Live_FileShare_GetPooledMapKills()
{
    if ( s_pooledFileDetails.isValid )
        return va("%d", s_pooledFileDetails.kills);
    else
        return "";
}

const char *__cdecl Live_FileShare_GetPooledMapDeaths()
{
    if ( s_pooledFileDetails.isValid )
        return va("%d", s_pooledFileDetails.deaths);
    else
        return "";
}

const char *__cdecl Live_FileShare_GetPooledMapTeam()
{
    if ( s_pooledFileDetails.isValid )
        return Live_FileShare_GetPooledTeamIcon(s_pooledFileDetails.mapID, s_pooledFileDetails.team);
    else
        return "";
}

const char *__cdecl Live_FileShare_GetPooledMapAxisScore()
{
    if ( s_pooledFileDetails.isValid )
        return va("%d", s_pooledFileDetails.axisScore);
    else
        return "";
}

const char *__cdecl Live_FileShare_GetPooledMapAxisIcon()
{
    if ( s_pooledFileDetails.isValid )
        return Live_FileShare_GetPooledTeamColorIcon(1);
    else
        return "";
}

const char *__cdecl Live_FileShare_GetPooledTeamColorIcon(int teamID)
{
    const char *teamIcon; // [esp+0h] [ebp-4h]

    teamIcon = Live_FileShare_GetPooledTeamIcon(s_pooledFileDetails.mapID, teamID);
    if ( !I_stricmp(teamIcon, "ops") )
        return "faction_128_specops";
    if ( !I_stricmp(teamIcon, "op40") )
        return "faction_128_op40";
    if ( !I_stricmp(teamIcon, "spets") )
        return "faction_128_spetsnaz";
    if ( !I_stricmp(teamIcon, "sog") )
        return "faction_128_marines";
    if ( !I_stricmp(teamIcon, "tropas") )
        return "faction_128_tropas";
    if ( !I_stricmp(teamIcon, "nva") )
        return "faction_128_nva";
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare.cpp", 2225, 0, "Invalid team ID") )
        __debugbreak();
    return "faction_128_nva";
}

char *__cdecl Live_FileShare_GetPooledMapAxisName()
{
    if ( s_pooledFileDetails.isValid )
        return Live_FileShare_GetPooledTeamName(s_pooledFileDetails.mapID, 1);
    else
        return (char *)"";
}

const char *__cdecl Live_FileShare_GetPooledMapAlliesScore()
{
    if ( s_pooledFileDetails.isValid )
        return va("%d", s_pooledFileDetails.alliesScore);
    else
        return "";
}

const char *__cdecl Live_FileShare_GetPooledMapAlliesIcon()
{
    if ( s_pooledFileDetails.isValid )
        return Live_FileShare_GetPooledTeamColorIcon(2);
    else
        return "";
}

char *__cdecl Live_FileShare_GetPooledMapAlliesName()
{
    if ( s_pooledFileDetails.isValid )
        return Live_FileShare_GetPooledTeamName(s_pooledFileDetails.mapID, 2);
    else
        return (char *)"";
}

const char *__cdecl Live_FileShare_GetPooledMapVictory()
{
    if ( !s_pooledFileDetails.isValid )
        return "";
    if ( s_pooledFileDetails.alliesScore == s_pooledFileDetails.axisScore )
        return "draw";
    if ( s_pooledFileDetails.team == 2 && s_pooledFileDetails.alliesScore > s_pooledFileDetails.axisScore
        || s_pooledFileDetails.team == 1 && s_pooledFileDetails.axisScore > s_pooledFileDetails.alliesScore )
    {
        return "victory";
    }
    return "defeat";
}

const char *__cdecl Live_FileShare_IsPooledTeamBased()
{
    if ( !s_pooledFileDetails.isValid || s_pooledFileDetails.team )
        return "1";
    else
        return "0";
}

const char *__cdecl Live_FileShare_GetPooledScore()
{
    if ( s_pooledFileDetails.isValid )
        return va("%d", s_pooledFileDetails.score);
    else
        return "";
}

const char *__cdecl Live_FileShare_GetPooledWagerPayout()
{
    if ( s_pooledFileDetails.isValid )
        return va("%d", s_pooledFileDetails.wagerPayout);
    else
        return "";
}

const char *__cdecl Live_FileShare_GetPooledPosition()
{
    if ( s_pooledFileDetails.isValid )
        return va("%d", s_pooledFileDetails.position);
    else
        return "";
}

char *__cdecl Live_FileShare_GetPooledIsWager()
{
    if ( s_pooledFileDetails.isValid && s_pooledFileDetails.isWager )
        return va("%d", 1);
    else
        return va("%d", 0);
}

char __cdecl Live_FileShare_GetItemInfo(
                int controllerIndex,
                fileShareKeyIndex keyIndex,
                int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult)
{
#ifdef KISAK_LIVE_SERVICE
    bool v6; // al
    char v8; // [esp+6h] [ebp-85Eh]
    char v9; // [esp+9h] [ebp-85Bh]
    fileShareBufferLocation bufferLocation; // [esp+Ch] [ebp-858h]
    fileSharePrivateData *data; // [esp+10h] [ebp-854h]
    unsigned int slot; // [esp+14h] [ebp-850h]
    bdFileMetaData *descriptors; // [esp+18h] [ebp-84Ch]
    int descriptorCount; // [esp+1Ch] [ebp-848h]
    int i; // [esp+20h] [ebp-844h]
    bdFileMetaData descriptor; // [esp+24h] [ebp-840h] BYREF

    //bdFileMetaData::bdFileMetaData(&descriptor);
    if ( location == FILESHARE_INFOLOCATION_SEARCHRESULTS )
    {
        if ( Live_FileShareSearch_GetDescriptor(controllerIndex, index, &descriptor) )
        {
            descriptors = &descriptor;
            descriptorCount = 1;
            index = 0;
            v6 = Live_FileShare_AreInfoParamsValid(&descriptor, 1, 0, FILESHARE_INFOLOCATION_SEARCHRESULTS);
            goto LABEL_36;
        }
        goto LABEL_16;
    }
    if ( location > FILESHARE_INFOLOCATION_SEARCHRESULTS && location <= FILESHARE_INFOLOCATION_OTHERFILESHARE )
    {
        if ( location == FILESHARE_INFOLOCATION_MYFILESHARE || location == FILESHARE_INFOLOCATION_MYFILESHARE_INGAME )
        {
            data = LiveStorage_FileShare_GetFileShareData(FILESHARE_BUFFER_PRIMARY);
            bufferLocation = FILESHARE_BUFFER_PRIMARY;
        }
        else
        {
            data = LiveStorage_FileShare_GetFileShareData(FILESHARE_BUFFER_SECONDARY);
            bufferLocation = FILESHARE_BUFFER_SECONDARY;
        }
        if ( !data
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare.cpp", 2551, 0, "%s", "data") )
        {
            __debugbreak();
        }
        if ( data )
        {
            if ( location == FILESHARE_INFOLOCATION_MYFILESHARE_INGAME )
            {
                slot = index + 1;
            }
            else if ( location == FILESHARE_INFOLOCATION_OTHERFILESHARE )
            {
                slot = Live_FileShare_FeederToSlot(index, fsOtherUserPrivateSlotCol->current.unsignedInt);
            }
            else
            {
                slot = Live_FileShare_FeederToSlot(index, fsPrivateSlotCol->current.unsignedInt);
            }
            descriptors = data->descriptors;
            descriptorCount = data->descriptorCount;
            if ( (unsigned int)keyIndex <= FILESHARE_KEY_INGAMESLOT )
            {
                v9 = ((char (__cdecl *)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **))fileShareKeyFunctions[keyIndex])(
                             descriptors,
                             descriptorCount,
                             index,
                             location,
                             stringResult);
                ////bdFileMetaData::~bdFileMetaData(&descriptor);
                return v9;
            }
            if ( !Live_FileShare_IsSlotOccupied(slot, bufferLocation) )
                goto LABEL_25;
            for ( i = 0; i < descriptorCount; ++i )
            {
                if ( slot == descriptors[i].m_fileSlot )
                {
                    index = i;
                    break;
                }
            }
            if ( (index >= descriptorCount || index < 0)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare.cpp",
                            2596,
                            0,
                            "%s",
                            "index < descriptorCount && index >= 0") )
            {
                __debugbreak();
            }
            goto LABEL_35;
        }
LABEL_16:
        ////bdFileMetaData::~bdFileMetaData(&descriptor);
        return 0;
    }
LABEL_35:
    v6 = Live_FileShare_AreInfoParamsValid(descriptors, descriptorCount, index, location);
LABEL_36:
    if ( !v6 )
    {
LABEL_25:
        *stringResult = "";
        *floatResult = 0.0f;
        ////bdFileMetaData::~bdFileMetaData(&descriptor);
        return 0;
    }
    v8 = ((char (__cdecl *)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **))fileShareKeyFunctions[keyIndex])(
                 descriptors,
                 descriptorCount,
                 index,
                 location,
                 stringResult);
    ////bdFileMetaData::~bdFileMetaData(&descriptor);
    return v8;
#else
    return 0;
#endif
}

bool __cdecl Live_FileShare_AreInfoParamsValid(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location)
{
    return descriptors && descriptorCount > 0 && location && index >= 0 && index < descriptorCount;
}

char __cdecl Live_FileShare_GetMySlotInfo(
                int controllerIndex,
                int slotNum,
                const char *field,
                const char **stringResult,
                float *floatResult)
{
    int j; // [esp+0h] [ebp-14h]
    int i; // [esp+4h] [ebp-10h]
    fileSharePrivateData *data; // [esp+8h] [ebp-Ch]
    fileShareKeyIndex keyIndex; // [esp+Ch] [ebp-8h]
    int index; // [esp+10h] [ebp-4h]

    *stringResult = "";
    *floatResult = 0.0f;
    for ( i = 0; i < 30; ++i )
    {
        if ( !I_stricmp(field, fileShareKeyNames[i]) )
            keyIndex = (fileShareKeyIndex)i;
    }
    data = LiveStorage_FileShare_GetFileShareData(FILESHARE_BUFFER_PRIMARY);
    if ( !Live_FileShare_IsSlotOccupied(slotNum, FILESHARE_BUFFER_PRIMARY) )
        return 0;
    index = -1;
    for ( j = 0; j < data->descriptorCount; ++j )
    {
        if ( data->descriptors[j].m_fileSlot == slotNum )
        {
            index = j;
            break;
        }
    }
    if ( index < 0 )
        return 0;
    if ( Live_FileShare_AreInfoParamsValid(
                 data->descriptors,
                 data->descriptorCount,
                 index,
                 FILESHARE_INFOLOCATION_MYFILESHARE) )
    {
        return ((char (__cdecl *)(bdFileMetaData *, int, int, fileShareInfoLocation, const char **))fileShareKeyFunctions[keyIndex])(
                         data->descriptors,
                         data->descriptorCount,
                         index,
                         FILESHARE_INFOLOCATION_MYFILESHARE,
                         stringResult);
    }
    return 0;
}

char __cdecl Live_FileShare_GetFeederData(
                int controllerIndex,
                int feederID,
                int index,
                const char *field,
                const char **stringResult,
                float *floatResult)
{
    char result; // al
    int i; // [esp+4h] [ebp-8h]
    fileShareKeyIndex keyIndex; // [esp+8h] [ebp-4h]

    for ( i = 0; i < 30; ++i )
    {
        if ( !I_stricmp(field, fileShareKeyNames[i]) )
            keyIndex = (fileShareKeyIndex)i;
    }
    if ( (unsigned int)keyIndex > FILESHARE_KEY_COUNT )
        return 0;
    switch ( feederID )
    {
        case 'J':
            result = Live_FileShare_GetItemInfo(
                                 controllerIndex,
                                 keyIndex,
                                 index,
                                 FILESHARE_INFOLOCATION_SEARCHRESULTS,
                                 stringResult,
                                 floatResult);
            break;
        case 'O':
            result = Live_FileShare_GetItemInfo(
                                 controllerIndex,
                                 keyIndex,
                                 index,
                                 FILESHARE_INFOLOCATION_OTHERFILESHARE,
                                 stringResult,
                                 floatResult);
            break;
        case 'P':
            result = Live_FileShare_GetItemInfo(
                                 controllerIndex,
                                 keyIndex,
                                 index,
                                 FILESHARE_INFOLOCATION_MYFILESHARE,
                                 stringResult,
                                 floatResult);
            break;
        case 'V':
            result = Live_FileShare_GetItemInfo(
                                 controllerIndex,
                                 keyIndex,
                                 index,
                                 FILESHARE_INFOLOCATION_MYFILESHARE_INGAME,
                                 stringResult,
                                 floatResult);
            break;
        default:
            return 0;
    }
    return result;
}

void __cdecl Live_FileShare_RatingOwnerdraw(
                int contextIndex,
                const rectDef_s *rect,
                const itemDef_s *item,
                const float *color)
{
    unsigned __int64 v4; // rax
    bdVoteRankStatsInfo *v5; // [esp+18h] [ebp-24h]
    int v6; // [esp+1Ch] [ebp-20h]
    int numRatings; // [esp+24h] [ebp-18h] BYREF
    bdVoteRankStatsInfo *ratings; // [esp+28h] [ebp-14h] BYREF
    int starWidth; // [esp+2Ch] [ebp-10h]
    int startHeight; // [esp+30h] [ebp-Ch]
    float rating; // [esp+34h] [ebp-8h] BYREF
    Material *mtl; // [esp+38h] [ebp-4h]

    mtl = Material_RegisterHandle("menu_mp_star_rating", 3);
    starWidth = 12;
    startHeight = 12;
    rating = 0.0f;
    if ( fsDebugRatingValue && fsDebugRatingValue->current.value > 0.0 )
    {
        rating = fsDebugRatingValue->current.value;
    }
    else
    {
        Live_FileShareSearch_GetRatings(&ratings, &numRatings);
        v6 = numRatings;
        v5 = ratings;
        v4 = I_atoi64(fsSelectedFileID->current.string);
        Live_FileShare_LookupRating(v4, v5, v6, &rating);
    }
    Live_FileShare_DrawBackgroundStars(
        contextIndex,
        rect->x,
        rect->y,
        starWidth,
        startHeight,
        rect->horzAlign,
        rect->vertAlign,
        color,
        mtl);
    Live_FileShare_DrawRatingStars(
        contextIndex,
        rating,
        rect->x,
        rect->y,
        starWidth,
        startHeight,
        rect->horzAlign,
        rect->vertAlign,
        &fsStarHighlightColor->current.value,
        mtl);
}

void __cdecl Live_FileShare_DrawBackgroundStars(
                int contextIndex,
                float x,
                float y,
                int w,
                int h,
                int hA,
                int vA,
                const float *color,
                Material *mtl)
{
    int i; // [esp+20h] [ebp-4h]

    for ( i = 0; i < 5; ++i )
        UI_DrawHandlePic(&scrPlaceView[contextIndex], (float)(w * i) + x, y, (float)w, (float)h, hA, vA, color, mtl);
}

void __cdecl Live_FileShare_DrawRatingStars(
                int contextIndex,
                float rating,
                float x,
                float y,
                int w,
                int h,
                int hA,
                int vA,
                const float *color,
                Material *mtl)
{
    float fraction; // [esp+30h] [ebp-8h]
    int i; // [esp+34h] [ebp-4h]

    for ( i = 0; i < 5; ++i )
    {
        fraction = Live_FileShare_GetStarFractionToDraw(rating, i + 1);
        if ( fraction > 0.0 )
            CL_DrawStretchPic(
                &scrPlaceView[contextIndex],
                (float)(w * i) + x,
                y,
                (float)w * fraction,
                (float)h,
                hA,
                vA,
                0.0,
                0.0,
                fraction,
                1.0,
                color,
                mtl);
    }
}

double __cdecl Live_FileShare_GetStarFractionToDraw(float rating, int imageNumber)
{
    int iRating; // [esp+4h] [ebp-8h]

    iRating = (int)(float)(rating * 100.0);
    if ( 20 * imageNumber <= iRating )
        return 1.0;
    if ( 20 * (imageNumber - 1) >= iRating )
        return 0.0;
    return ((double)iRating - (double)(imageNumber - 1) * (double)20) / (double)20;
}

void __cdecl Live_FileShare_PrivateRatingOwnerdraw(
                int contextIndex,
                const rectDef_s *rect,
                const itemDef_s *item,
                const float *color)
{
    unsigned __int64 v4; // rax
    unsigned __int64 v5; // rax
    int descriptorCount; // [esp+1Ch] [ebp-20h]
    int v7; // [esp+1Ch] [ebp-20h]
    fileSharePrivateData *FileShareData; // [esp+24h] [ebp-18h]
    fileSharePrivateData *data; // [esp+28h] [ebp-14h]
    float rating; // [esp+34h] [ebp-8h] BYREF
    Material *mtl; // [esp+38h] [ebp-4h]

    mtl = Material_RegisterHandle("menu_mp_star_rating", 3);
    rating = 0.0f;
    if ( fsDebugRatingValue && fsDebugRatingValue->current.value > 0.0 )
    {
        rating = fsDebugRatingValue->current.value;
    }
    else
    {
        data = LiveStorage_FileShare_GetFileShareData(FILESHARE_BUFFER_PRIMARY);
        descriptorCount = data->descriptorCount;
        v4 = I_atoi64(fsSelectedFileID->current.string);
        if ( !Live_FileShare_LookupRating(v4, data->fileRatings, descriptorCount, &rating) )
        {
            FileShareData = LiveStorage_FileShare_GetFileShareData(FILESHARE_BUFFER_SECONDARY);
            v7 = FileShareData->descriptorCount;
            v5 = I_atoi64(fsSelectedFileID->current.string);
            Live_FileShare_LookupRating(v5, FileShareData->fileRatings, v7, &rating);
        }
    }
    Live_FileShare_DrawBackgroundStars(
        contextIndex,
        rect->x,
        rect->y,
        12,
        12,
        rect->horzAlign,
        rect->vertAlign,
        color,
        mtl);
    Live_FileShare_DrawRatingStars(
        contextIndex,
        rating,
        rect->x,
        rect->y,
        12,
        12,
        rect->horzAlign,
        rect->vertAlign,
        &fsStarHighlightColor->current.value,
        mtl);
}

void __cdecl Live_FileShare_SubmitRatingOwnerdraw(
                int contextIndex,
                const rectDef_s *rect,
                const itemDef_s *item,
                const float *color)
{
    float rating; // [esp+0h] [ebp-40h]
    float ratinga; // [esp+0h] [ebp-40h]
    Material *mtl; // [esp+3Ch] [ebp-4h]

    mtl = Material_RegisterHandle("menu_mp_star_rating", 3);
    Live_FileShare_DrawBackgroundStars(
        contextIndex,
        rect->x,
        rect->y,
        30,
        30,
        rect->horzAlign,
        rect->vertAlign,
        color,
        mtl);
    rating = (double)s_fileShareFileRating / 5.0;
    Live_FileShare_DrawRatingStars(
        contextIndex,
        rating,
        rect->x,
        rect->y,
        30,
        30,
        rect->horzAlign,
        rect->vertAlign,
        &fsStarHighlightColor->current.value,
        mtl);
    ratinga = (double)s_fileShareFileRatingPreview / 5.0;
    Live_FileShare_DrawRatingStars(
        contextIndex,
        ratinga,
        rect->x,
        rect->y,
        30,
        30,
        rect->horzAlign,
        rect->vertAlign,
        &fsStarPreviewColor->current.value,
        mtl);
}

void __cdecl Live_FileShare_MyRatingOwnerdraw(
                int localClientNum,
                int contextIndex,
                const rectDef_s *rect,
                const itemDef_s *item,
                const float *color)
{
    int ControllerIndex; // eax
    unsigned __int64 v6; // [esp+18h] [ebp-24h]
    unsigned __int8 rating; // [esp+2Fh] [ebp-Dh] BYREF
    Material *mtl; // [esp+30h] [ebp-Ch]
    unsigned __int64 fileID; // [esp+34h] [ebp-8h]

    mtl = Material_RegisterHandle("menu_mp_star_rating", 3);
    Live_FileShare_DrawBackgroundStars(
        contextIndex,
        rect->x,
        rect->y,
        30,
        30,
        rect->horzAlign,
        rect->vertAlign,
        color,
        mtl);
    fileID = I_atoi64(fsSelectedFileID->current.string);
    rating = 0;
    if ( fileID )
    {
        v6 = fileID;
        ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        if ( Live_FileShare_IsRated(ControllerIndex, v6, &rating) )
            Live_FileShare_DrawRatingStars(
                contextIndex,
                (float)rating / 5.0,
                rect->x,
                rect->y,
                30,
                30,
                rect->horzAlign,
                rect->vertAlign,
                &fsStarHighlightColor->current.value,
                mtl);
    }
}

void __cdecl Live_FileShare_AvgSubmitRatingOwnerdraw(
                int contextIndex,
                const rectDef_s *rect,
                const itemDef_s *item,
                const float *color)
{
    unsigned __int64 v4; // rax
    bdVoteRankStatsInfo *v5; // [esp+18h] [ebp-24h]
    int v6; // [esp+1Ch] [ebp-20h]
    int numRatings; // [esp+24h] [ebp-18h] BYREF
    bdVoteRankStatsInfo *ratings; // [esp+28h] [ebp-14h] BYREF
    int starWidth; // [esp+2Ch] [ebp-10h]
    int startHeight; // [esp+30h] [ebp-Ch]
    float rating; // [esp+34h] [ebp-8h] BYREF
    Material *mtl; // [esp+38h] [ebp-4h]

    mtl = Material_RegisterHandle("menu_mp_star_rating", 3);
    starWidth = 12;
    startHeight = 12;
    rating = 0.0f;
    if ( fsDebugRatingValue && fsDebugRatingValue->current.value > 0.0 )
    {
        rating = fsDebugRatingValue->current.value;
    }
    else
    {
        Live_FileShareSearch_GetRatings(&ratings, &numRatings);
        v6 = numRatings;
        v5 = ratings;
        v4 = I_atoi64(fsSelectedFileID->current.string);
        Live_FileShare_LookupRating(v4, v5, v6, &rating);
    }
    Live_FileShare_DrawBackgroundStars(
        contextIndex,
        rect->x,
        rect->y,
        starWidth,
        startHeight,
        rect->horzAlign,
        rect->vertAlign,
        color,
        mtl);
    Live_FileShare_DrawRatingStars(
        contextIndex,
        rating,
        rect->x,
        rect->y,
        starWidth,
        startHeight,
        rect->horzAlign,
        rect->vertAlign,
        &fsStarAvgColor->current.value,
        mtl);
}

void __cdecl Live_FileShare_SearchDvarsToTags(int controllerIndex, bdTag *tags, int *numTags, int maxTags)
{
    unsigned __int64 v4; // rax
    int gameModeIndex; // [esp+0h] [ebp-14h]
    int userTag; // [esp+4h] [ebp-10h]
    int fileType; // [esp+8h] [ebp-Ch]
    int playerPreference; // [esp+Ch] [ebp-8h]
    int mapIndex; // [esp+10h] [ebp-4h]

    mapIndex = s_fsSearchRowValueDvars[2]->current.integer;
    gameModeIndex = s_fsSearchRowValueDvars[3]->current.integer;
    userTag = s_fsSearchRowValueDvars[5]->current.integer;
    playerPreference = s_fsSearchRowValueDvars[6]->current.integer;
    fileType = fsSearchFileType->current.integer;
    *numTags = 0;
    if ( mapIndex && fileType != 4 )
        Live_FileShare_AddTag(2u, mapIndex - 1, numTags, tags, maxTags);
    if ( gameModeIndex )
        Live_FileShare_AddTag(1u, gameModeIndex - 1, numTags, tags, maxTags);
    if ( fileType )
        Live_FileShare_AddTag(3u, fileType, numTags, tags, maxTags);
    if ( userTag )
        Live_FileShare_AddTag(5u, userTag, numTags, tags, maxTags);
    if ( playerPreference )
    {
        LODWORD(v4) = Live_GetXuid(controllerIndex);
        Live_FileShare_AddTag(4u, v4, numTags, tags, maxTags);
    }
}

void __cdecl Live_FileShare_ClearSearchState()
{
    const StringTable *fileShareTable; // [esp+4h] [ebp-8h] BYREF
    int i; // [esp+8h] [ebp-4h]
#ifdef KISAK_LIVE_SERVICE
    if ( live_service && live_service->current.enabled )
    {
        for ( i = 0; i < 7; ++i )
        {
            switch ( i )
            {
                case 2:
                    Dvar_SetInt((dvar_s *)s_fsSearchRowValueDvars[2], 0);
                    Live_FileShare_SetSpinnerDvars(
                        0,
                        sharedUiInfo.mapCount,
                        (dvar_s *)s_fsSearchRowValueDvars[2],
                        (dvar_s *)s_fsSearchRowTextDvars[2]);
                    break;
                case 3:
                    Dvar_SetInt((dvar_s *)s_fsSearchRowValueDvars[3], 0);
                    Live_FileShare_SetSpinnerDvars(
                        0,
                        sharedUiInfo.playerClientNums[31],
                        (dvar_s *)s_fsSearchRowValueDvars[3],
                        (dvar_s *)s_fsSearchRowTextDvars[3]);
                    break;
                case 4:
                    Dvar_SetInt((dvar_s *)s_fsSearchRowValueDvars[4], 0);
                    Live_FileShare_SetSpinnerDvars(
                        0,
                        3,
                        (dvar_s *)s_fsSearchRowValueDvars[4],
                        (dvar_s *)s_fsSearchRowTextDvars[4]);
                    break;
                case 5:
                    StringTable_GetAsset("mp/fileShareTags.csv", (XAssetHeader *)&fileShareTable);
                    Dvar_SetInt((dvar_s *)s_fsSearchRowValueDvars[5], 0);
                    Live_FileShare_SetSpinnerDvars(
                        0,
                        fileShareTable->rowCount,
                        (dvar_s *)s_fsSearchRowValueDvars[5],
                        (dvar_s *)s_fsSearchRowTextDvars[5]);
                    break;
                case 6:
                    Dvar_SetInt((dvar_s *)s_fsSearchRowValueDvars[6], 0);
                    Live_FileShare_SetSpinnerDvars(
                        0,
                        1,
                        (dvar_s *)s_fsSearchRowValueDvars[6],
                        (dvar_s *)s_fsSearchRowTextDvars[6]);
                    break;
                default:
                    continue;
            }
        }
    }
#endif
}

int __cdecl Live_FileShare_SetSpinnerDvars(int offset, int max, dvar_s *valueDvar, dvar_s *textDvar)
{
    int value; // [esp+0h] [ebp-4h]

    value = offset + valueDvar->current.integer;
    if ( value >= 0 )
    {
        if ( value > max )
            value = 0;
    }
    else
    {
        value = max;
    }
    if ( !value )
        Dvar_SetString(textDvar, "@MENU_ALL");
    Dvar_SetInt(valueDvar, value);
    return value;
}

cmd_function_s Live_FileShare_Read_f_VAR;
cmd_function_s Live_FileShare_GetSummary_f_VAR;
cmd_function_s Live_FileShare_TransferFromPooled_f_VAR;
cmd_function_s Live_FileShare_TransferFromUser_f_VAR;
cmd_function_s Live_FileShare_TransferFromCommunity_f_VAR;
cmd_function_s Live_FileShare_TransferLastUploaded_f_VAR;
cmd_function_s Live_FileShare_EditInfo_f_VAR;
cmd_function_s Live_FileShare_Remove_f_VAR;
cmd_function_s Live_FileShare_AbortOperation_f_VAR;
cmd_function_s Live_FileShare_GetRecentGames_f_VAR;
cmd_function_s Live_FileShare_ClearSearchDvars_f_VAR;
cmd_function_s Live_FileShare_SearchShiftRight_f_VAR;
cmd_function_s Live_FileShare_SearchShiftLeft_f_VAR;
cmd_function_s Live_FileShare_Search_f_VAR;
cmd_function_s Live_FileShare_DecreaseRating_f_VAR;
cmd_function_s Live_FileShare_IncreaseRating_f_VAR;
cmd_function_s Live_FileShare_ResetRating_f_VAR;
cmd_function_s Live_FileShare_SubmitRating_f_VAR;
cmd_function_s Live_FileShare_GetTopRated_f_VAR;
cmd_function_s Live_FileShare_SetRating_f_VAR;
cmd_function_s Live_FileShare_SetRatingPreview_f_VAR;
cmd_function_s Live_FileShare_ResetRatingPreview_f_VAR;
cmd_function_s Live_FileShare_ReadUserTag_f_VAR;
cmd_function_s Live_FileShare_UserTagLeft_f_VAR;
cmd_function_s Live_FileShare_UserTagRight_f_VAR;
cmd_function_s Live_FileShare_FilterBackup_f_VAR;
cmd_function_s Live_FileShare_FilterRestore_f_VAR;
cmd_function_s Live_FileShare_SetPooledFileContext_f_VAR;
cmd_function_s Live_FileShare_ExpireFileDetails_f_VAR;
cmd_function_s Live_FileShare_GetLastPlayedGame_f_VAR;

void __cdecl Live_FileShare_Init()
{
    char *v0; // eax
    int i; // [esp+20h] [ebp-4h]

    for ( i = 1; i < 7; ++i )
    {
        s_fsSearchRowTextDvars[i] = _Dvar_RegisterString(
                                                                    s_fsSearchRowTextDvarNames[i],
                                                                    "@MENU_ALL",
                                                                    0,
                                                                    "Search text dvar for search spinner rows.");
        s_fsSearchRowValueDvars[i] = _Dvar_RegisterInt(
                                                                     s_fsSearchRowValueDvarNames[i],
                                                                     0,
                                                                     0,
                                                                     0x7FFFFFFF,
                                                                     0,
                                                                     "Search value dvar for search spinner rows.");
    }
    s_lastPlayedGame.descriptor.m_fileID = 0;
    v0 = UI_SafeTranslateString("@MENU_FILESHARE_MOSTRECENT");
    Dvar_SetString((dvar_s *)s_fsSearchRowTextDvars[1], v0);
    fsSearchFileType = _Dvar_RegisterInt("fsSearchFileType", 1, 0, 0x7FFFFFFF, 0, "File type on which to perform search");
    fsSearchSelectedRow = _Dvar_RegisterInt(
                                                    "fsSearchSelectedRow",
                                                    0,
                                                    0,
                                                    0x7FFFFFFF,
                                                    0,
                                                    "Selected search box row in the file share search menu.");
    fsShowStreamingGraph = _Dvar_RegisterBool(
                                                     "fsShowStreamingGraph",
                                                     0,
                                                     0,
                                                     "Shows a graph in-game with streaming history information.");
    fsPrivateSlotCol = _Dvar_RegisterInt(
                                             "fsPrivateSlotCol",
                                             0,
                                             0,
                                             2,
                                             0,
                                             "Selected column in the 2d private slots feeder.");
    fsOtherUserPrivateSlotCol = _Dvar_RegisterInt(
                                                                "fsOtherUserPrivateSlotCol",
                                                                0,
                                                                0,
                                                                2,
                                                                0,
                                                                "Selected column in the 2d private slots feeder for the other user.");
    fsMaxPrivateSlotRowsOther = _Dvar_RegisterInt(
                                                                "fsMaxPrivateSlotRowsOther",
                                                                2,
                                                                0,
                                                                6,
                                                                0,
                                                                "The maximum number of rows for the user's friend's file share while displaying the 2d pr"
                                                                "ivate slots feeder.");
    fsOtherUserSlotSelected = _Dvar_RegisterInt(
                                                            "fsOtherUserSlotSelected",
                                                            0,
                                                            0,
                                                            0x7FFFFFFF,
                                                            0,
                                                            "Other playre's slot selected while transferring to My File Share");
    fsSlotEmptyHiddenColor = _Dvar_RegisterVec4(
                                                         "fsSlotEmptyHiddenColor",
                                                         (0.0),
                                                         (0.0),
                                                         (0.0),
                                                         (0.0),
                                                         0.0,
                                                         1.0,
                                                         0,
                                                         "Color of the material depicting a private slot when not selected.");
    fsSlotEmptyShowColorBg = _Dvar_RegisterVec4(
                                                         "fsSlotEmptyShowColorBg",
                                                         (0.40000001),
                                                         (0.40000001),
                                                         (0.40000001),
                                                         (0.5),
                                                         0.0,
                                                         1.0,
                                                         0,
                                                         "Color of the material depicting a private empty slot background when not selected.");
    fsSlotEmptyMainColor = _Dvar_RegisterVec4(
                                                     "fsSlotEmptyMainColor",
                                                     (0.25),
                                                     (0.25),
                                                     (0.25),
                                                     (0.94999999),
                                                     0.0,
                                                     1.0,
                                                     0,
                                                     "Color of the empty slot. Must be completely opaque to reduce the background to an outline.");
    fsSlotEmptyNotSubscribedColor = _Dvar_RegisterVec4(
                                                                        "fsSlotEmptyNotSubscribedColor",
                                                                         (0.36000001),
                                                                         (0.36000001),
                                                                         (0.36000001),
                                                                         (0.94999999),
                                                                        0.0,
                                                                        1.0,
                                                                        0,
                                                                        "Color of emtpy slot that is also classified.");
    fsSlotNumMainColor = _Dvar_RegisterVec4(
                                                 "fsSlotNumMainColor",
                                                  (0.80000001),
                                                  (0.80000001),
                                                  (0.80000001),
                                                  (0.89999998),
                                                 0.0,
                                                 1.0,
                                                 0,
                                                 "Color of the slot number");
    fsSlotNumNoSubsribeColor = _Dvar_RegisterVec4(
                                                             "fsSlotNumNoSubsribeColor",
                                                              (0.0),
                                                              (0.0),
                                                              (0.0),
                                                              (0.30000001),
                                                             0.0,
                                                             1.0,
                                                             0,
                                                             "Color of the empty slot when not a subscriber");
    fsSlotEmptyShowColor = _Dvar_RegisterVec4(
                                                     "fsSlotEmptyShowColor",
                                                      (0.0),
                                                      (0.0),
                                                      (0.0),
                                                      (0.80000001),
                                                     0.0,
                                                     1.0,
                                                     0,
                                                     "Color of the material depicting a private slot when not selected.");
    fsSlotMainColor = _Dvar_RegisterVec4(
                                            "fsSlotMainColor",
                                             (1.0),
                                             (1.0),
                                             (1.0),
                                             (1.0),
                                            0.0,
                                            1.0,
                                            0,
                                            "Color of the material depicting a private slot when not selected.");
    fsSlotHighlightedColor = _Dvar_RegisterVec4(
                                                         "fsSlotHighlightedColor",
                                                          (0.5),
                                                          (1.0),
                                                          (0.5),
                                                          (0.80000001),
                                                         0.0,
                                                         1.0,
                                                         0,
                                                         "Color of the material depicting a private slot when selected.");
    fsSlotHighlightedColorNoSel = _Dvar_RegisterVec4(
                                                                    "fsSlotHighlightedColorNoSel",
                                                                     (1.0),
                                                                     (1.0),
                                                                     (1.0),
                                                                     (0.89999998),
                                                                    0.0,
                                                                    1.0,
                                                                    0,
                                                                    "Color of the material depicting a private slot when selected but cannot be chosen.");
    fsDebugRatingValue = _Dvar_RegisterFloat(
                                                 "fsDebugRatingValue",
                                                 0.0,
                                                 0.0,
                                                 1.0,
                                                 0x80u,
                                                 "Rating value for debugging the star ownerdraw.");
    fsStarHighlightColor = _Dvar_RegisterVec4(
                                                     "fsStarHighlightColor",
                                                      (0.69999999),
                                                      (0.69999999),
                                                      (0.1),
                                                      (0.80000001),
                                                     0.0,
                                                     1.0,
                                                     0,
                                                     "Highlight color for star ratings.");
    fsStarPreviewColor = _Dvar_RegisterVec4(
                                                 "fsStarPreviewColor",
                                                  (1.0),
                                                  (1.0),
                                                  (1.0),
                                                  (0.5),
                                                 0.0,
                                                 1.0,
                                                 0,
                                                 "Preview color for star ratings.");
    fsStarAvgColor = _Dvar_RegisterVec4(
                                         "fsStarAvgColor",
                                          (0.69999999),
                                          (0.2),
                                          (0.2),
                                          (0.80000001),
                                         0.0,
                                         1.0,
                                         0,
                                         "Highlight color for star ratings average while rating.");
    fshOldItemColor = _Dvar_RegisterVec4(
                                            "fshOldItemColor",
                                             (1.0),
                                             (1.0),
                                             (1.0),
                                             (0.2),
                                            0.0,
                                            1.0,
                                            0,
                                            "Color of old items in search results that can't be selected.");
    fsSelectedFileID = _Dvar_RegisterString(
                                             "fsSelectedFileID",
                                             (char *)"",
                                             0,
                                             "File ID currently selected. Use to lookup up rating in ownerdraw.");
    fsSelectedFileName = _Dvar_RegisterString(
                                                 "fsSelectedFileName",
                                                 (char *)"",
                                                 0,
                                                 "Name when a file is selected to be transferred to My File Share");
    fsIsSelectedFileNameModified = _Dvar_RegisterBool(
                                                                     "fsIsSelectedFileNameModified",
                                                                     0,
                                                                     0,
                                                                     "A flag which gives us information if a selected filename is modified.");
    fsSelectedFileDescription = _Dvar_RegisterString(
                                                                "fsSelectedFileDescription",
                                                                (char *)"",
                                                                0,
                                                                "Description when a file is selected to be transferred to My File Share");
    fsIsSelectedFileDescriptionModified = _Dvar_RegisterBool(
                                                                                    "fsIsSelectedFileDescriptionModified",
                                                                                    0,
                                                                                    0,
                                                                                    "A flag which gives us information if a selected file's description is modified.");
    fsSelectedFileTagIndex = _Dvar_RegisterInt(
                                                         "fsSelectedFileTagIndex",
                                                         0,
                                                         0,
                                                         0x7FFFFFFF,
                                                         0,
                                                         "The tag index of the selected file.");
    fshSelectLastSlotRow = _Dvar_RegisterInt(
                                                     "fshSelectLastSlotRow",
                                                     0,
                                                     0,
                                                     0x7FFFFFFF,
                                                     0,
                                                     "Indicates if the last row in the 2d slot feeder must be selected when focused.");
    fshSelectFirstSlotRow = _Dvar_RegisterInt(
                                                        "fshSelectFirstSlotRow",
                                                        0,
                                                        0,
                                                        0x7FFFFFFF,
                                                        0,
                                                        "Indicates if the first row in the 2d slot feeder must be selected when focused.");
    fshLiveBlurb = _Dvar_RegisterString("fshLiveBlurb", (char *)"", 0, "Live subsription marketing blurb.");
    fshDebugFileList = _Dvar_RegisterBool(
                                             "fshDebugFileList",
                                             0,
                                             0x80u,
                                             "Replaces feeder data with file IDs for file share debugging.");
    fsRecents = _Dvar_RegisterInt(
                                "fsRecents",
                                7,
                                -1,
                                0x7FFFFFFF,
                                0,
                                "Number of days that non-elite users can see their recent game history. Value of -1 clears this filter.");
    fsRecentsCount = _Dvar_RegisterInt(
                                         "fsRecentsCount",
                                         100,
                                         0,
                                         0x7FFFFFFF,
                                         0,
                                         "Number of recent games non-elite users can see.");
    Cmd_AddCommandInternal("readFileShare", Live_FileShare_Read_f, &Live_FileShare_Read_f_VAR);
    Cmd_AddCommandInternal("fileShareGetSummary", BLOPS_NULLSUB, &Live_FileShare_GetSummary_f_VAR);
    Cmd_AddCommandInternal(
        "fileShareTransferFromPooled",
        Live_FileShare_TransferFromPooled_f,
        &Live_FileShare_TransferFromPooled_f_VAR);
    Cmd_AddCommandInternal(
        "fileShareTransferFromUser",
        Live_FileShare_TransferFromUser_f,
        &Live_FileShare_TransferFromUser_f_VAR);
    Cmd_AddCommandInternal(
        "fileShareTransferFromCommunity",
        Live_FileShare_TransferFromCommunity_f,
        &Live_FileShare_TransferFromCommunity_f_VAR);
    Cmd_AddCommandInternal(
        "fileShareTransferLastUploaded",
        Live_FileShare_TransferLastUploaded_f,
        &Live_FileShare_TransferLastUploaded_f_VAR);
    Cmd_AddCommandInternal("fileShareEditInfo", Live_FileShare_EditInfo_f, &Live_FileShare_EditInfo_f_VAR);
    Cmd_AddCommandInternal("fileShareRemove", Live_FileShare_Remove_f, &Live_FileShare_Remove_f_VAR);
    Cmd_AddCommandInternal(
        "fileShareAbortOperation",
        Live_FileShare_AbortOperation_f,
        &Live_FileShare_AbortOperation_f_VAR);
    Cmd_AddCommandInternal(
        "fileShareGetRecentGames",
        Live_FileShare_GetRecentGames_f,
        &Live_FileShare_GetRecentGames_f_VAR);
    Cmd_AddCommandInternal(
        "fileShareClearSearchDvars",
        Live_FileShare_ClearSearchDvars_f,
        &Live_FileShare_ClearSearchDvars_f_VAR);
    Cmd_AddCommandInternal(
        "fileShareSearchShiftRight",
        Live_FileShare_SearchShiftRight_f,
        &Live_FileShare_SearchShiftRight_f_VAR);
    Cmd_AddCommandInternal(
        "fileShareSearchShiftLeft",
        Live_FileShare_SearchShiftLeft_f,
        &Live_FileShare_SearchShiftLeft_f_VAR);
    Cmd_AddCommandInternal("fileSharePerformSearch", Live_FileShare_Search_f, &Live_FileShare_Search_f_VAR);
    Cmd_AddCommandInternal(
        "fileShareDecreaseRating",
        Live_FileShare_DecreaseRating_f,
        &Live_FileShare_DecreaseRating_f_VAR);
    Cmd_AddCommandInternal(
        "fileShareIncreaseRating",
        Live_FileShare_IncreaseRating_f,
        &Live_FileShare_IncreaseRating_f_VAR);
    Cmd_AddCommandInternal("fileShareResetRating", Live_FileShare_ResetRating_f, &Live_FileShare_ResetRating_f_VAR);
    Cmd_AddCommandInternal("fileShareSubmitRating", Live_FileShare_SubmitRating_f, &Live_FileShare_SubmitRating_f_VAR);
    Cmd_AddCommandInternal("fileShareGetTopRated", Live_FileShare_GetTopRated_f, &Live_FileShare_GetTopRated_f_VAR);
    Cmd_AddCommandInternal("fileShareSetRating", Live_FileShare_SetRating_f, &Live_FileShare_SetRating_f_VAR);
    Cmd_AddCommandInternal(
        "fileShareSetRatingPreview",
        Live_FileShare_SetRatingPreview_f,
        &Live_FileShare_SetRatingPreview_f_VAR);
    Cmd_AddCommandInternal(
        "fileShareResetRatingPreview",
        Live_FileShare_ResetRatingPreview_f,
        &Live_FileShare_ResetRatingPreview_f_VAR);
    Cmd_AddCommandInternal("fileShareReadUserTag", Live_FileShare_ReadUserTag_f, &Live_FileShare_ReadUserTag_f_VAR);
    Cmd_AddCommandInternal(
        "fileShareUserTagLeft",
        (void (__cdecl *)())Live_FileShare_UserTagLeft_f,
        &Live_FileShare_UserTagLeft_f_VAR);
    Cmd_AddCommandInternal(
        "fileShareUserTagRight",
        (void (__cdecl *)())Live_FileShare_UserTagRight_f,
        &Live_FileShare_UserTagRight_f_VAR);
    Cmd_AddCommandInternal("fileShareFilterBackup", Live_FileShare_FilterBackup_f, &Live_FileShare_FilterBackup_f_VAR);
    Cmd_AddCommandInternal("fileShareFilterRestore", Live_FileShare_FilterRestore_f, &Live_FileShare_FilterRestore_f_VAR);
    Cmd_AddCommandInternal(
        "fileShareSetPooledFileContext",
        Live_FileShare_SetPooledFileContext_f,
        &Live_FileShare_SetPooledFileContext_f_VAR);
    Cmd_AddCommandInternal(
        "fileShareExpireFileDetails",
        Live_FileShare_ExpireFileDetails_f,
        &Live_FileShare_ExpireFileDetails_f_VAR);
    Cmd_AddCommandInternal(
        "fileShareGetLastPlayedGame",
        Live_FileShare_GetLastPlayedGame_f,
        &Live_FileShare_GetLastPlayedGame_f_VAR);
    g_fileshareDDL = DDL_LoadAsset("ddl_mp/file_share.ddl");
    DDL_Reset(&g_fileShareRootState, g_fileshareDDL);
    g_fileSharePublicDDL = DDL_LoadAsset("ddl_mp/file_share_public.ddl");
    DDL_Reset(&g_fileSharePublicRootState, g_fileSharePublicDDL);
    Live_FileShare_InitRatingHistory();
}

void __cdecl Live_FileShare_Read_f()
{
    unsigned __int64 OwnerID; // rax
    const char *v1; // eax
    unsigned __int64 v2; // rax
    int argCount; // [esp+18h] [ebp-4h]

    argCount = Cmd_Argc();
    if ( SV_GetOwnerID() )
    {
        if ( argCount <= 2 )
        {
            if ( argCount == 1 )
            {
                if ( SV_CanLoadCustomGameType() )
                {
                    OwnerID = SV_GetOwnerID();
                    LiveStorage_FileShare_ReadListing(0, OwnerID, FILESHARE_BUFFER_SECONDARY);
                }
                else
                {
                    Com_PrintError(15, "Can't load custom gametypes when ranked!\n");
                }
            }
            else
            {
                v1 = Cmd_Argv(1);
                v2 = I_atoi64(v1);
                LiveStorage_FileShare_ReadListing(0, v2, FILESHARE_BUFFER_SECONDARY);
            }
        }
        else
        {
            Com_Printf(16, "readFileShare usage: readFileShare OR readFileShare <player XUID>\n");
        }
    }
    else
    {
        Com_PrintError(15, "Must set ownerid first\n");
    }
}

void __cdecl Live_FileShare_ClearSearchDvars_f()
{
    if ( Cmd_Argc() == 1 )
        Live_FileShare_ClearSearchState();
    else
        Com_Printf(16, "fileShareClearSearchDvars usage: fileShareClearSearchDvars\n");
}

void __cdecl Live_FileShare_SearchShiftRight_f()
{
    if ( Cmd_Argc() == 1 )
    {
        if ( !fsSearchSelectedRow
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare.cpp",
                        3168,
                        0,
                        "%s",
                        "fsSearchSelectedRow") )
        {
            __debugbreak();
        }
        Live_FileShare_SetSearchRowOffset(fsSearchSelectedRow->current.integer, 1);
    }
    else
    {
        Com_Printf(16, "fileShareSearchShiftRight usage: fileShareSearchShiftRight\n");
    }
}

char fileShareSortBy[1][64] =
{
  {
    'M',
    'E',
    'N',
    'U',
    '_',
    'F',
    'I',
    'L',
    'E',
    'S',
    'H',
    'A',
    'R',
    'E',
    '_',
    'M',
    'O',
    'S',
    'T',
    'P',
    'O',
    'P',
    'U',
    'L',
    'A',
    'R',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0'
  }
};

char fileShareAgeSearch[3][64] =
{
  {
    'M',
    'E',
    'N',
    'U',
    '_',
    'F',
    'I',
    'L',
    'E',
    'S',
    'H',
    'A',
    'R',
    'E',
    '_',
    'W',
    'E',
    'E',
    'K',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0'
  },
  {
    'M',
    'E',
    'N',
    'U',
    '_',
    'F',
    'I',
    'L',
    'E',
    'S',
    'H',
    'A',
    'R',
    'E',
    '_',
    'M',
    'O',
    'N',
    'T',
    'H',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0'
  },
  {
    'M',
    'E',
    'N',
    'U',
    '_',
    'F',
    'I',
    'L',
    'E',
    'S',
    'H',
    'A',
    'R',
    'E',
    '_',
    'Y',
    'E',
    'A',
    'R',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0'
  }
};


char fshPlayerSearchPreference[1][64] =
{
  {
    'M',
    'E',
    'N',
    'U',
    '_',
    'F',
    'I',
    'L',
    'E',
    'S',
    'H',
    'A',
    'R',
    'E',
    '_',
    'S',
    'E',
    'A',
    'R',
    'C',
    'H',
    'M',
    'E',
    'O',
    'N',
    'L',
    'Y',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0',
    '\0'
  }
};


void __cdecl Live_FileShare_SetSearchRowOffset(int row, int offset)
{
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    int GameTypeCount; // eax
    char *LocalizedGameTypeFromIndex; // eax
    char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    char *v10; // eax
    char *v11; // eax
    dvar_s *v12; // [esp-8h] [ebp-14h]
    dvar_s *v13; // [esp-4h] [ebp-10h]
    const StringTable *fileShareTable; // [esp+4h] [ebp-8h] BYREF
    int value; // [esp+8h] [ebp-4h]

    switch ( row )
    {
        case 1:
            value = Live_FileShare_SetSpinnerDvars(
                                offset,
                                1,
                                (dvar_s *)s_fsSearchRowValueDvars[1],
                                (dvar_s *)s_fsSearchRowTextDvars[1]);
            if ( value )
            {
                if ( value <= 0 || value > 2 )
                {
                    Com_PrintError(16, "Fileshare: Value not in sharedUiInfo bounds.\n");
                }
                else
                {
                    v2 = UI_SafeTranslateString(fileShareSortBy[value - 1]);
                    Dvar_SetString((dvar_s *)s_fsSearchRowTextDvars[1], v2);
                }
            }
            else
            {
                v3 = UI_SafeTranslateString("@MENU_FILESHARE_MOSTRECENT");
                Dvar_SetString((dvar_s *)s_fsSearchRowTextDvars[1], v3);
            }
            break;
        case 2:
            value = Live_FileShare_SetSpinnerDvars(
                                offset,
                                sharedUiInfo.mapCount,
                                (dvar_s *)s_fsSearchRowValueDvars[2],
                                (dvar_s *)s_fsSearchRowTextDvars[2]);
            if ( value )
            {
                if ( value <= 0 || value > sharedUiInfo.mapCount )
                {
                    Com_PrintError(16, "Fileshare: Value not in sharedUiInfo bounds.\n");
                }
                else
                {
                    v4 = UI_SafeTranslateString(&sharedUiInfo.joinGameTypes[29].gameTypeShortName[304 * value]);
                    Dvar_SetString((dvar_s *)s_fsSearchRowTextDvars[2], v4);
                }
            }
            break;
        case 3:
            v13 = (dvar_s *)s_fsSearchRowTextDvars[3];
            v12 = (dvar_s *)s_fsSearchRowValueDvars[3];
            GameTypeCount = Live_FileShare_GetGameTypeCount();
            value = Live_FileShare_SetSpinnerDvars(offset, GameTypeCount, v12, v13);
            if ( value )
            {
                if ( value <= 0 || value > Live_FileShare_GetGameTypeCount() )
                {
                    Com_PrintError(16, "Fileshare: Value not in sharedUiInfo bounds.\n");
                }
                else
                {
                    LocalizedGameTypeFromIndex = Live_FileShare_GetLocalizedGameTypeFromIndex(value - 1, 0);
                    Dvar_SetString((dvar_s *)s_fsSearchRowTextDvars[3], LocalizedGameTypeFromIndex);
                }
            }
            break;
        case 4:
            value = Live_FileShare_SetSpinnerDvars(
                                offset,
                                3,
                                (dvar_s *)s_fsSearchRowValueDvars[4],
                                (dvar_s *)s_fsSearchRowTextDvars[4]);
            if ( value )
            {
                v7 = UI_SafeTranslateString(fileShareAgeSearch[value - 1]);
                Dvar_SetString((dvar_s *)s_fsSearchRowTextDvars[4], v7);
            }
            break;
        case 5:
            StringTable_GetAsset("mp/fileShareTags.csv", (XAssetHeader *)&fileShareTable);
            value = Live_FileShare_SetSpinnerDvars(
                                offset,
                                fileShareTable->rowCount,
                                (dvar_s *)s_fsSearchRowValueDvars[5],
                                (dvar_s *)s_fsSearchRowTextDvars[5]);
            if ( value )
            {
                v8 = va("%d", value);
                v9 = StringTable_Lookup(fileShareTable, 0, v8, 1);
                v10 = UI_SafeTranslateString(v9);
                Dvar_SetString((dvar_s *)s_fsSearchRowTextDvars[5], v10);
            }
            break;
        case 6:
            value = Live_FileShare_SetSpinnerDvars(
                                offset,
                                1,
                                (dvar_s *)s_fsSearchRowValueDvars[6],
                                (dvar_s *)s_fsSearchRowTextDvars[6]);
            if ( value )
            {
                v11 = UI_SafeTranslateString(fshPlayerSearchPreference[value - 1]);
                Dvar_SetString((dvar_s *)s_fsSearchRowTextDvars[6], v11);
            }
            break;
        default:
            return;
    }
}

void __cdecl Live_FileShare_SearchShiftLeft_f()
{
    if ( Cmd_Argc() == 1 )
    {
        if ( !fsSearchSelectedRow
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare.cpp",
                        3181,
                        0,
                        "%s",
                        "fsSearchSelectedRow") )
        {
            __debugbreak();
        }
        Live_FileShare_SetSearchRowOffset(fsSearchSelectedRow->current.integer, -1);
    }
    else
    {
        Com_Printf(16, "fileShareSearchShiftLeft usage: fileShareSearchShiftLeft\n");
    }
}

void __cdecl Live_FileShare_TransferFromPooled_f()
{
    const char *v0; // eax
    const char *v1; // eax
    unsigned __int64 v2; // rax
    unsigned int v3; // [esp-Ch] [ebp-24h]

    if ( Cmd_Argc() == 3 )
    {
        v0 = Cmd_Argv(2);
        v3 = atoi(v0);
        v1 = Cmd_Argv(1);
        v2 = I_atoi64(v1);
        LiveStorage_FileShare_TransferFile(0, v2, v3, FILESHARE_LOCATION_POOLEDSTORAGE, 0);
    }
    else
    {
        Com_Printf(
            16,
            "fileShareTransferFromPooled usage: fileShareTransferFromPooled <pooledStorageFileID> <userStorageSlot> \n");
    }
}

void __cdecl Live_FileShare_TransferFromUser_f()
{
    const char *v0; // eax
    const char *v1; // eax
    unsigned __int64 v2; // rax
    unsigned int v3; // [esp-Ch] [ebp-24h]

    if ( Cmd_Argc() == 3 )
    {
        v0 = Cmd_Argv(2);
        v3 = atoi(v0);
        v1 = Cmd_Argv(1);
        v2 = I_atoi64(v1);
        LiveStorage_FileShare_TransferFile(0, v2, v3, FILESHARE_LOCATION_USERSTORAGE, 0);
    }
    else
    {
        Com_Printf(
            16,
            "fileShareTransferFromUser usage: fileShareTransferFromUser <userStorageFileID> <userStorageSlot> \n");
    }
}

void __cdecl Live_FileShare_TransferFromCommunity_f()
{
    const char *v0; // eax
    const char *v1; // eax
    unsigned __int64 v2; // rax
    unsigned int v3; // [esp-Ch] [ebp-24h]

    if ( Cmd_Argc() == 3 )
    {
        v0 = Cmd_Argv(2);
        v3 = atoi(v0);
        v1 = Cmd_Argv(1);
        v2 = I_atoi64(v1);
        LiveStorage_FileShare_TransferFile(0, v2, v3, FILESHARE_LOCATION_USERSTORAGE, 1);
    }
    else
    {
        Com_Printf(
            16,
            "fileShareTransferFromCommunity usage: fileShareTransferFromCommunity <userStorageFileID> <userStorageSlot> \n");
    }
}

void __cdecl Live_FileShare_TransferLastUploaded_f()
{
    const char *v0; // eax
    unsigned int v1; // eax

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        v1 = atoi(v0);
        LiveStorage_FileShare_TransferLastUploaded(0, v1);
    }
    else
    {
        Com_Printf(16, "fileShareTransferLastUploaded usage: fileShareTransferLastUploaded <userStorageSlot> \n");
    }
}

void __cdecl Live_FileShare_GetRecentGames_f()
{
    unsigned __int64 v0; // rax
    const char *v1; // eax
    __int64 playerXuid; // [esp+10h] [ebp-10h]
    int argCount; // [esp+1Ch] [ebp-4h]

    argCount = Cmd_Argc();
    if ( argCount <= 2 )
    {
        if ( argCount == 1 )
        {
            LODWORD(v0) = Live_GetXuid(0);
            Live_FileShare_GetRecentGames(0, v0);
        }
        else
        {
            v1 = Cmd_Argv(1);
            playerXuid = I_atoi64(v1);
            if ( playerXuid )
                Live_FileShare_GetRecentGames(0, playerXuid);
            else
                Com_PrintError(16, "Invalid XUID.\n");
        }
    }
    else
    {
        Com_Printf(16, "fileShareGetRecentGames usage: fileShareGetRecentGames OR fileShareGetRecentGames <player XUID>\n");
    }
}

void __cdecl Live_FileShare_GetRecentGames(int localControllerIndex, unsigned __int64 playerXuid)
{
    int v2; // [esp+0h] [ebp-2Ch]
    bdTaskResult *j; // [esp+4h] [ebp-28h]
    int v4; // [esp+8h] [ebp-24h]
    bdTag *i; // [esp+Ch] [ebp-20h]
    int numTags; // [esp+10h] [ebp-1Ch] BYREF
    bdTag tags[1]; // [esp+14h] [ebp-18h] BYREF
    int savedregs; // [esp+2Ch] [ebp+0h] BYREF

    //v4 = 1;
    //for ( i = tags; --v4 >= 0; ++i )
    //    bdTag::bdTag(i);
    numTags = 0;
    Live_FileShare_AddTag(4u, playerXuid, &numTags, tags, 40);
    if ( numTags > 0 )
        Live_FileShareSearch_SetContext(localControllerIndex, tags, numTags, FILESHARE_LOCATION_POOLEDSTORAGE, 0, -1);
    v2 = 1;
    //for ( j = (bdTaskResult *)&savedregs; --v2 >= 0; bdTag::~bdTag(j) )
    //    j -= 6;
}

void __cdecl Live_FileShare_Remove_f()
{
    const char *v0; // eax
    unsigned int v1; // eax

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        v1 = atoi(v0);
        LiveStorage_FileShare_RemoveFile(0, v1);
    }
    else
    {
        Com_Printf(16, "fileShareRemove usage: fileShareRemove <fileSlot>\n");
    }
}

void __cdecl Live_FileShare_EditInfo_f()
{
    const char *v0; // eax
    char *ClientName; // eax
    int numTags; // [esp+2Ch] [ebp-20h] BYREF
    unsigned int i; // [esp+30h] [ebp-1Ch]
    fileSharePrivateData *data; // [esp+34h] [ebp-18h]
    int index; // [esp+38h] [ebp-14h]
    unsigned __int64 fileID; // [esp+3Ch] [ebp-10h]
    bdFileMetaData *desc; // [esp+48h] [ebp-4h]

    v0 = Cmd_Argv(1);
    fileID = I_atoi64(v0);
    data = LiveStorage_FileShare_GetFileShareData(FILESHARE_BUFFER_PRIMARY);
    for ( index = 0; index < data->descriptorCount && data->descriptors[index].m_fileID != fileID; ++index )
        ;
    if ( index >= data->descriptorCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare.cpp",
                    3299,
                    0,
                    "%s",
                    "index < data->descriptorCount") )
    {
        __debugbreak();
    }
    if ( index == data->descriptorCount )
    {
        Com_PrintError(16, "File ID %llu not found in file share.\n", fileID);
    }
    else
    {
        ClientName = Live_ControllerIndex_GetClientName(0);
        I_strncpyz(s_gamerTag, ClientName, 32);
        desc = &data->descriptors[index];
        if ( (fileSharePrivateData *)((char *)data + 2104 * index) == (fileSharePrivateData *)-16
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare.cpp", 3313, 0, "%s", "desc") )
        {
            __debugbreak();
        }
        if ( desc )
        {
            for ( i = 0; i < desc->m_numTags; ++i )
            {
                if ( LODWORD(desc->m_tags[i].m_priTag) == 5 && !HIDWORD(desc->m_tags[i].m_priTag) )
                {
                    desc->m_tags[i].m_secTag = fsSelectedFileTagIndex->current.integer;
                    break;
                }
            }
            if ( i == desc->m_numTags )
            {
                numTags = desc->m_numTags;
                Live_FileShare_AddTag(5u, fsSelectedFileTagIndex->current.integer, &numTags, desc->m_tags, 40);
                desc->m_numTags = numTags;
            }
            LiveStorage_FileShare_UpdateMetaData((char *)desc->m_metaData);
            LiveStorage_FileShare_WriteSummary(
                0,
                fileID,
                FILESHARE_LOCATION_USERSTORAGE,
                s_gamerTag,
                strlen(s_gamerTag),
                desc->m_metaData,
                desc->m_metaDataSize,
                desc->m_tags,
                desc->m_numTags,
                1);
        }
    }
}

void __cdecl Live_FileShare_AbortOperation_f()
{
    fileShareLocation location; // [esp+8h] [ebp-4h]

    if ( Cmd_Argc() == 1 )
    {
        location = LiveStorage_FileShare_GetCurrentHTTPLocation(0);
        if ( location )
            LiveStorage_FileShare_AbortOperation(0, location);
    }
    else
    {
        Com_Printf(16, "fileShareAbortOperation usage: fileShareAbortOperation\n");
    }
}

void __cdecl Live_FileShare_Search_f()
{
    if ( Cmd_Argc() == 1 )
        Live_FileShare_DvarSearch(0);
    else
        Com_Printf(16, "fileShareSearchUserGames usage: fileShareSearchUserGames\n");
}

void __cdecl Live_FileShare_DvarSearch(int controllerIndex)
{
    int integer; // [esp+0h] [ebp-3DCh]
    int v2; // [esp+4h] [ebp-3D8h]
    bdTaskResult *j; // [esp+8h] [ebp-3D4h]
    int v4; // [esp+Ch] [ebp-3D0h]
    bdTag *i; // [esp+10h] [ebp-3CCh]
    int numPastDays; // [esp+14h] [ebp-3C8h]
    int numTags; // [esp+18h] [ebp-3C4h] BYREF
    bdTag tags[40]; // [esp+1Ch] [ebp-3C0h] BYREF
    int savedregs; // [esp+3DCh] [ebp+0h] BYREF

    //v4 = 40;
    //for ( i = tags; --v4 >= 0; ++i )
    //    bdTag::bdTag(i);
    if ( s_fsSearchRowValueDvars[1]->current.integer <= 0 )
    {
        numPastDays = -1;
        integer = s_fsSearchRowValueDvars[4]->current.integer;
        switch ( integer )
        {
            case 1:
                numPastDays = 7;
                break;
            case 2:
                numPastDays = 31;
                break;
            case 3:
                numPastDays = 365;
                break;
        }
        Live_FileShare_SearchDvarsToTags(controllerIndex, tags, &numTags, 40);
        if ( numTags <= 0 )
        {
            Com_PrintError(16, "Invalid search. No search tags could be created.\n");
        }
        else
        {
            Com_Printf(16, "Searching for the past %d days.\n", numPastDays);
            Live_FileShareSearch_SetContext(0, tags, numTags, FILESHARE_LOCATION_USERSTORAGE, 0, numPastDays);
        }
    }
    else
    {
        Live_FileShareSearch_SetContext(0, tags, 0, FILESHARE_LOCATION_USERSTORAGE, 1, -1);
    }
    //v2 = 40;
    //for ( j = (bdTaskResult *)&savedregs; --v2 >= 0; bdTag::~bdTag(j) )
    //    j -= 6;
}

void __cdecl Live_FileShare_SubmitRating_f()
{
    const char *v0; // eax
    __int64 fileID; // [esp+10h] [ebp-8h]

    if ( Cmd_Argc() == 2 )
    {
        if ( s_fileShareFileRating )
        {
            v0 = Cmd_Argv(1);
            fileID = I_atoi64(v0);
            Com_Printf(16, "Setting a rating of %d for %llu\n", s_fileShareFileRating, fileID);
            LiveStorage_FileShare_WriteRating(0, fileID, s_fileShareFileRating);
        }
    }
    else
    {
        Com_Printf(16, "fileShareSubmitRating usage: fileShareSubmitRating <fileID>\n");
    }
}

void __cdecl Live_FileShare_ResetRating_f()
{
    if ( Cmd_Argc() == 1 )
        s_fileShareFileRating = 0;
    else
        Com_Printf(16, "fileShareResetRating usage: fileShareResetRating\n");
}

void __cdecl Live_FileShare_IncreaseRating_f()
{
    if ( Cmd_Argc() == 1 )
    {
        if ( s_fileShareFileRating < 5 )
            ++s_fileShareFileRating;
    }
    else
    {
        Com_Printf(16, "fileShareIncreaseRating usage: fileShareIncreaseRating\n");
    }
}

void __cdecl Live_FileShare_DecreaseRating_f()
{
    unsigned int v0; // [esp+0h] [ebp-10h]

    if ( Cmd_Argc() == 1 )
    {
        if ( (int)(s_fileShareFileRating - 1) < 0 )
            v0 = 0;
        else
            v0 = s_fileShareFileRating - 1;
        s_fileShareFileRating = v0;
    }
    else
    {
        Com_Printf(16, "fileShareDecreaseRating usage: fileShareDecreaseRating\n");
    }
}

void __cdecl Live_FileShare_SetRating_f()
{
    const char *v0; // eax
    unsigned int v1; // [esp+0h] [ebp-18h]
    int v2; // [esp+4h] [ebp-14h]

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        v2 = atoi(v0);
        if ( v2 >= 0 )
        {
            if ( v2 <= 5 )
                v1 = v2;
            else
                v1 = 5;
        }
        else
        {
            v1 = 0;
        }
        s_fileShareFileRating = v1;
    }
    else
    {
        Com_Printf(16, "fileShareSetRating usage: fileShareSetRating <rating>\n");
    }
}

void __cdecl Live_FileShare_SetRatingPreview_f()
{
    const char *v0; // eax
    unsigned int v1; // [esp+0h] [ebp-18h]
    int v2; // [esp+4h] [ebp-14h]

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        v2 = atoi(v0);
        if ( v2 >= 0 )
        {
            if ( v2 <= 5 )
                v1 = v2;
            else
                v1 = 5;
        }
        else
        {
            v1 = 0;
        }
        s_fileShareFileRatingPreview = v1;
    }
    else
    {
        Com_Printf(16, "fileShareSetRating usage: fileShareSetRating <rating>\n");
    }
}

void __cdecl Live_FileShare_ResetRatingPreview_f()
{
    if ( Cmd_Argc() == 1 )
        s_fileShareFileRatingPreview = 0;
    else
        Com_Printf(16, "fileShareResetRatingPreview usage: fileShareResetRatingPreview\n");
}

void __cdecl Live_FileShare_ReadUserTag_f()
{
    int i; // [esp+0h] [ebp-Ch]
    const StringTable *fileShareTable; // [esp+4h] [ebp-8h] BYREF
    const char *value; // [esp+8h] [ebp-4h]

    StringTable_GetAsset("mp/fileShareTags.csv", (XAssetHeader *)&fileShareTable);
    for ( i = 0; i < fileShareTable->rowCount; ++i )
        value = StringTable_Lookup(fileShareTable, 0, "1", 1);
}

void Live_FileShare_UserTagLeft_f(const StringTable *table)
{
    const StringTable *fileShareTable; // [esp+0h] [ebp-4h] BYREF

    fileShareTable = table;
    StringTable_GetAsset("mp/fileShareTags.csv", (XAssetHeader *)&fileShareTable);
    if ( fsSelectedFileTagIndex->current.integer )
        Dvar_SetInt((dvar_s *)fsSelectedFileTagIndex, fsSelectedFileTagIndex->current.integer - 1);
    else
        Dvar_SetInt((dvar_s *)fsSelectedFileTagIndex, fileShareTable->rowCount);
}

void Live_FileShare_UserTagRight_f(const StringTable *table)
{
    const StringTable *v1; // ecx
    const StringTable *fileShareTable; // [esp+0h] [ebp-4h] BYREF

    fileShareTable = table;
    if ( Key_IsDown(0, 201) )
    {
        Live_FileShare_UserTagLeft_f(fileShareTable);
    }
    else
    {
        StringTable_GetAsset("mp/fileShareTags.csv", (XAssetHeader *)&fileShareTable);
        Dvar_SetInt((dvar_s *)fsSelectedFileTagIndex, fsSelectedFileTagIndex->current.integer + 1);
        if ( fsSelectedFileTagIndex->current.integer > fileShareTable->rowCount )
            Dvar_SetInt((dvar_s *)fsSelectedFileTagIndex, 0);
    }
}

fileShareFilter s_fileShareFilterBackup;
void __cdecl Live_FileShare_FilterBackup_f()
{
    s_fileShareFilterBackup.mapName = s_fsSearchRowValueDvars[2]->current.integer;
    s_fileShareFilterBackup.gameType = s_fsSearchRowValueDvars[3]->current.integer;
    s_fileShareFilterBackup.age = s_fsSearchRowValueDvars[4]->current.integer;
    s_fileShareFilterBackup.userTag = s_fsSearchRowValueDvars[5]->current.integer;
    s_fileShareFilterBackup.playerPref = s_fsSearchRowValueDvars[6]->current.integer;
    s_fileShareFilterBackup.sortBy = s_fsSearchRowValueDvars[1]->current.integer;
}

void __cdecl Live_FileShare_FilterRestore_f()
{
    Live_FileShare_SetSearchRowOffset(2, s_fileShareFilterBackup.mapName - s_fsSearchRowValueDvars[2]->current.integer);
    Live_FileShare_SetSearchRowOffset(3, s_fileShareFilterBackup.gameType - s_fsSearchRowValueDvars[3]->current.integer);
    Live_FileShare_SetSearchRowOffset(4, s_fileShareFilterBackup.age - s_fsSearchRowValueDvars[4]->current.integer);
    Live_FileShare_SetSearchRowOffset(5, s_fileShareFilterBackup.userTag - s_fsSearchRowValueDvars[5]->current.integer);
    Live_FileShare_SetSearchRowOffset(6, s_fileShareFilterBackup.playerPref - s_fsSearchRowValueDvars[6]->current.integer);
    Live_FileShare_SetSearchRowOffset(1, s_fileShareFilterBackup.sortBy - s_fsSearchRowValueDvars[1]->current.integer);
}

void __cdecl Live_FileShare_GetTopRated_f()
{
    if ( Cmd_Argc() != 2 )
        Com_Printf(16, "fileShareGetTopRated usage: fileShareGetTopRated <file Type>\n");
}

void __cdecl Live_FileShare_ExpireFileDetails_f()
{
    Live_FileShare_ExpirePooledFileDetails();
}

void __cdecl Live_FileShare_GetLastPlayedGame_f()
{
    Live_FileShare_GetLastPlayedGameDetails(0);
}

void __cdecl Live_FileShare_SetPooledFileContext_f()
{
    const char *v0; // eax
    const char *v1; // eax

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        if ( I_stricmp(v0, "self") )
        {
            v1 = Cmd_Argv(1);
            if ( I_stricmp(v1, "friend") )
                Live_FileShare_SetPooledFileDetailsContext(FILESHARE_POOLEDFILEDETAILS_INVALID);
            else
                Live_FileShare_SetPooledFileDetailsContext(FILESHARE_POOLEDFILEDETAILS_FRIEND);
        }
        else
        {
            Live_FileShare_SetPooledFileDetailsContext(FILESHARE_POOLEDFILEDETAILS_SELF);
        }
    }
    else
    {
        Com_Printf(
            16,
            "fileShareSetPooledFileContext usage: fileShareSetPooledFileContext <'self' / 'friend' / 'invalid'>\n");
    }
}

