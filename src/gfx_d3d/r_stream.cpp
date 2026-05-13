#include "r_stream.h"
#include "r_dvars.h"
#include <xanim/xanim.h>
#include <xanim/xmodel_utils.h>
#include <qcommon/threads.h>
#include <universal/q_shared.h>
#include <xanim/dobj_utils.h>
#include <bgame/bg_local.h>
#include "r_streamalloc.h"
#include <win32/win_common.h>
#include <qcommon/common.h>
#include "r_image_load_obj.h"
#include <win32/win_net.h>
#include <universal/com_memory.h>
#include "r_image.h"
#include "rb_resource.h"
#include <database/db_file_load.h>
#include <universal/com_workercmds.h>
#include <cgame/cg_compass.h>
#include <win32/win_shared.h>
#include <xanim/xmodel.h>

#include <algorithm>
#include <DynEntity/DynEntity_load_obj.h>
#include <DynEntity/DynEntity_client.h>
#include "r_rendercmds.h"

#define TOTAL_IMAGE_PARTS 4608 // Same as POOLSIZE_IMAGE

volatile unsigned int r_stream_sortLimit = 1;
jqModule r_stream_sortModule =
{
    .Name = "r_stream_sort",
    .Type = JQ_WORKER_GENERIC,
    .Code = (int(__cdecl *)(jqBatch *))r_stream_sortCallback,
};
jqWorkerCmd r_stream_sortWorkerCmd = { &r_stream_sortModule, 8u, 0, 0, &r_stream_sortLimit, NULL, 0u };


volatile unsigned int r_stream_updateLimit = 1;
jqModule r_stream_updateModule =
{
    .Name = "r_stream_update",
    .Type = JQ_WORKER_GENERIC,
    .Code = (int(__cdecl *)(jqBatch *))r_stream_updateCallback,
};
jqWorkerCmd r_stream_updateWorkerCmd = { &r_stream_updateModule, 28u, 0, 0, &r_stream_updateLimit, NULL, 0u };


volatile unsigned int r_stream_combineLimit = 1;
jqModule r_stream_combineModule =
{
    .Name = "r_stream_combine",
    .Type = JQ_WORKER_GENERIC,
    .Code = (int(__cdecl *)(jqBatch *))r_stream_combineCallback,
};
jqWorkerCmd r_stream_combineWorkerCmd = { &r_stream_combineModule, 4u, 0, 0, &r_stream_combineLimit, NULL, 0u };


volatile unsigned int r_stream_update_staticmodelsLimit = 1;
jqModule r_stream_update_staticmodelsModule =
{
    .Name = "r_stream_update_staticmodels",
    .Type = JQ_WORKER_GENERIC,
    .Code = r_stream_update_staticmodelsCallback,
};
jqWorkerCmd r_stream_update_staticmodelsWorkerCmd=
{
  &r_stream_update_staticmodelsModule,
  28u,
  0,
  0,
  &r_stream_update_staticmodelsLimit,
  NULL,
  0u
};



volatile unsigned int r_stream_update_staticsurfacesLimit = 1;
jqModule r_stream_update_staticsurfacesModule =
{
    .Name = "r_stream_update_staticsurfaces",
    .Type = JQ_WORKER_GENERIC,
    .Code = (int(__cdecl *)(jqBatch *))r_stream_update_staticsurfacesCallback,

};
jqWorkerCmd r_stream_update_staticsurfacesWorkerCmd =
{
  &r_stream_update_staticsurfacesModule,
  28u,
  0,
  0,
  &r_stream_update_staticsurfacesLimit,
  NULL,
  0u
};



Material *s_preventMaterials[32];

int s_numForcedLoadEntities;
const DObj *s_forcedLoadEntities[4];

StreamFrontendGlob streamFrontendGlob;
volatile unsigned int streamDisabledCount;
bool streamIsInitialized;

char streamUIConfigSet; // not a real name

int s_numStreamHintsActive;
streamerHintInfo s_streamHints[4];

pendingRequest s_pendingRequests[10];
int lastIdx;

bool __cdecl R_StreamIsEnabled()
{
    return streamDisabledCount == 0;
}

void __cdecl R_StreamPushDisable()
{
    _InterlockedExchangeAdd(&streamDisabledCount, 1u);
}

void __cdecl R_StreamPopDisable()
{
    if ( _InterlockedDecrement(&streamDisabledCount) < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp", 279, 0, "%s", "disableCount >= 0") )
    {
        __debugbreak();
    }
}

void __cdecl R_StreamSetInitData(int buffer_size)
{
    streamFrontendGlob.mainBufferSize = buffer_size;
}

void __cdecl R_StreamSetDefaultConfig(bool clear)
{
    streamFrontendGlob.forceDiskOrder = 0;
    streamFrontendGlob.touchedImageImportance = 1.7014117e38f;
    //*(_QWORD *)&streamFrontendGlob.initialImageImportance = __PAIR64__(
    //    LODWORD(FLOAT_1_7014117e38),
    //    LODWORD(FLOAT_8_5070587e37));
    streamFrontendGlob.initialImageImportance = 1.7014117e38f;
    streamFrontendGlob.forcedImageImportance = 8.5070587e37f;
    if (clear)
    {
        memset((unsigned __int8 *)streamFrontendGlob.imageInitialBits, 0, 528u);
        memset((unsigned __int8 *)streamFrontendGlob.imageForceBits, 0, 528u);
        streamFrontendGlob.imageInitialBitsSet = 0;
        streamFrontendGlob.initialLoadAllocFailures = 0;
        streamFrontendGlob.preloadCancelled = 0;
    }
    streamFrontendGlob.diskOrderImagesNeedSorting = 1;
}

void __cdecl R_StreamSetUIConfig(bool clear)
{
    streamFrontendGlob.forceDiskOrder = 1;
    streamFrontendGlob.touchedImageImportance = 1.7014117e38f;
    streamFrontendGlob.forcedImageImportance =  0.0000099999997f;
    streamFrontendGlob.initialImageImportance = 8.5070587e37f;
    if (clear)
    {
        memset((unsigned __int8 *)streamFrontendGlob.imageInitialBits, 0, 528u);
        memset((unsigned __int8 *)streamFrontendGlob.imageForceBits, 0, 528u);
        streamFrontendGlob.imageInitialBitsSet = 0;
        streamFrontendGlob.initialLoadAllocFailures = 0;
        streamFrontendGlob.preloadCancelled = 0;
    }
    streamFrontendGlob.diskOrderImagesNeedSorting = 1;
}

double __cdecl R_Stream_GetProgress()
{
    int complete; // [esp+28h] [ebp-10h]
    int total; // [esp+2Ch] [ebp-Ch]
    unsigned int bits; // [esp+30h] [ebp-8h]
    int idx; // [esp+34h] [ebp-4h]

    if (!com_waitForStreamer->current.integer)
        return 1.0;
    if (streamFrontendGlob.initialLoadAllocFailures > 0)
        return 1.0;
    if (!streamFrontendGlob.imageInitialBitsSet)
        return 0.0;
    total = 0;
    complete = 0;
    for (idx = 0; idx < 132; ++idx)
    {
        bits = streamFrontendGlob.imageInitialBits[idx] | streamFrontendGlob.imageForceBits[idx];
        total += CountBitsEnabled(bits);
        complete += CountBitsEnabled(streamFrontendGlob.imageUseBits[idx - 4] & bits);
    }
    if (total)
        return (double)complete / (double)total + 0.001;
    else
        return 1.0;
}

void __cdecl R_StreamUpdate_ReadTextures()
{
    int idx; // [esp+0h] [ebp-10h]
    int i; // [esp+4h] [ebp-Ch]
    pendingRequest *request; // [esp+8h] [ebp-8h]
    int numRequests; // [esp+Ch] [ebp-4h]

    request = 0;
    numRequests = 0;
    R_StreamAlloc_Lock();
    for ( i = 0; i < 10; ++i )
    {
        idx = (lastIdx + i) % 10;
        if ( s_pendingRequests[idx].status == STREAM_STATUS_QUEUED )
        {
            if ( !request )
            {
                request = &s_pendingRequests[idx];
                request->status = STREAM_STATUS_INPROGRESS;
                lastIdx = (idx + 1) % 10;
            }
            ++numRequests;
        }
    }
    R_StreamAlloc_Unlock();
    if ( request )
    {
        if ( Image_LoadToBuffer(request->image, request->highMip, &request->buffer, &request->bufferSize) )
            request->status = STREAM_STATUS_FINISHED;
        else
            request->status = STREAM_STATUS_CANCELLED;
        NET_Sleep(1u);
    }
    if ( numRequests > 1 )
        Sys_WakeStream();
}

bool __cdecl R_StreamUpdate_ProcessFileCallbacks()
{
    GfxImageFileHeader *fileHeader; // [esp+4h] [ebp-18h]
    GfxImage *image; // [esp+8h] [ebp-14h]
    pendingRequest *request; // [esp+Ch] [ebp-10h]
    int imageIndex; // [esp+10h] [ebp-Ch]
    int i; // [esp+14h] [ebp-8h]
    bool workDone; // [esp+1Bh] [ebp-1h]

    workDone = 0;
    for (i = 0; i < 10; ++i)
    {
        request = &s_pendingRequests[i];
        if (request->status >= (unsigned int)STREAM_STATUS_CANCELLED)
        {
            image = request->image;
            if (!image
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp", 1189, 0, "%s", "image != NULL"))
            {
                __debugbreak();
            }
            imageIndex = DB_GetImageIndex(image);
            if ((streamFrontendGlob.imageLoading[(imageIndex >> 5) - 8] & (1 << (imageIndex & 0x1F))) == 0
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
                    1192,
                    0,
                    "%s",
                    "( streamFrontendGlob.imageLoading[ BIT_INDEX_32( imageIndex ) ] & BIT_MASK_32( imageIndex ) )"))
            {
                __debugbreak();
            }
            if (request->status == STREAM_STATUS_CANCELLED
                || request->status == STREAM_STATUS_READFAILED
                || request->status == STREAM_STATUS_EOF)
            {
                streamFrontendGlob.imageLoading[(imageIndex >> 5) - 8] &= ~(1 << (imageIndex & 0x1F));
                Z_VirtualFree(request->buffer, 20);
                R_Stream_InvalidateRequest(request);
            }
            else
            {
                workDone = 1;
                if (request->status != STREAM_STATUS_FINISHED
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
                        1207,
                        0,
                        "%s\n\t(request->status) = %i",
                        "(request->status == STREAM_STATUS_FINISHED)",
                        request->status))
                {
                    __debugbreak();
                }
                if (Sys_IsRenderThread())
                {
                    fileHeader = (GfxImageFileHeader *)request->buffer;
                    Image_Release(image);
                    if (image->texture.basemap
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
                            1219,
                            0,
                            "%s",
                            "!image->texture.basemap"))
                    {
                        __debugbreak();
                    }
                    Image_LoadFromData(image, fileHeader, (unsigned __int8 *)&fileHeader[1]);
                    Z_VirtualFree(request->buffer, 20);
                }
                else
                {
                    RB_Resource_ReloadTexture(image, request->buffer);
                }
                if (request->highMip)
                {
                    if ((streamFrontendGlob.imageUseBits[(imageIndex >> 5) - 4] & (1 << (imageIndex & 0x1F))) != 0
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
                            1227,
                            0,
                            "%s",
                            "!(streamFrontendGlob.imageUseBits[BIT_INDEX_32(imageIndex)] & BIT_MASK_32(imageIndex))"))
                    {
                        __debugbreak();
                    }
                    streamFrontendGlob.imageUseBits[(imageIndex >> 5) - 4] |= 1 << (imageIndex & 0x1F);
                }
                else
                {
                    if ((streamFrontendGlob.imageUseBits[(imageIndex >> 5) - 4] & (1 << (imageIndex & 0x1F))) == 0
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
                            1232,
                            0,
                            "%s",
                            "streamFrontendGlob.imageUseBits[BIT_INDEX_32(imageIndex)] & BIT_MASK_32(imageIndex)"))
                    {
                        __debugbreak();
                    }
                    streamFrontendGlob.imageUseBits[(imageIndex >> 5) - 4] &= ~(1 << (imageIndex & 0x1F));
                }
                streamFrontendGlob.imageLoading[(imageIndex >> 5) - 8] &= ~(1 << (imageIndex & 0x1F));
                R_Stream_InvalidateRequest(request);
            }
        }
    }
    return workDone;
}
void __cdecl R_Stream_InvalidateRequest(pendingRequest *request)
{
    memset((unsigned __int8 *)request, 0, sizeof(pendingRequest));
    request->status = STREAM_STATUS_INVALID;
}

void __cdecl R_StreamUpdate_SetupInitialImageList()
{
    R_StreamSetDefaultConfig(1);
    if (r_streamLowDetail && r_streamLowDetail->current.enabled)
    {
        //BG_EvalVehicleName();
    }
    else
    {
        memset((unsigned __int8 *)streamFrontendGlob.imageForceBits, 0, 0x210u);
    }
    streamFrontendGlob.diskOrderImagesNeedSorting = 1;
    streamFrontendGlob.imageInitialBitsSet = 1;
    streamFrontendGlob.initialLoadAllocFailures = 0;
    streamFrontendGlob.preloadCancelled = 0;
}

void __cdecl R_Stream_ForceLoadMaterial(Material *material, int part)
{
    GfxImage *image; // [esp+0h] [ebp-10h]
    MaterialTextureDef *texDef; // [esp+4h] [ebp-Ch]
    int textureIter; // [esp+8h] [ebp-8h]
    int textureCount; // [esp+Ch] [ebp-4h]

    if ( material )
    {
        textureCount = material->textureCount;
        for ( textureIter = 0; textureIter != textureCount; ++textureIter )
        {
            texDef = &material->textureTable[textureIter];
            if ( !texDef
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp", 1644, 0, "%s", "texDef") )
            {
                __debugbreak();
            }
            if ( texDef->semantic != 11 )
            {
                image = texDef->u.image;
                if ( !image
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp", 1650, 0, "%s", "image") )
                {
                    __debugbreak();
                }
                R_Stream_ForceLoadImage(image, part);
            }
        }
    }
}

void __cdecl R_Stream_ForceLoadImage(GfxImage *image, int part)
{
    int v2; // eax
    int v3; // eax
    int ImageIndex; // eax
    int v5; // [esp+0h] [ebp-14h]
    int partIndex; // [esp+10h] [ebp-4h]
    int partIndexa; // [esp+10h] [ebp-4h]
    int partIndexb; // [esp+10h] [ebp-4h]

    if (image && image->streaming)
    {
        if (part)
        {
            if (part >= 0)
            {
                for (partIndexb = 0; partIndexb < part && partIndexb < 1; ++partIndexb)
                {
                    ImageIndex = DB_GetImageIndex(image);
                    streamFrontendGlob.imageForceBits[(partIndexb + ImageIndex) >> 5] |= 1 << ((partIndexb + ImageIndex) & 0x1F);
                }
            }
            else
            {
                if (part + 1 < 0)
                    v5 = 0;
                else
                    v5 = part + 1;
                for (partIndexa = v5; partIndexa >= 0; --partIndexa)
                {
                    v3 = DB_GetImageIndex(image);
                    streamFrontendGlob.imageForceBits[(partIndexa + v3) >> 5] |= 1 << ((partIndexa + v3) & 0x1F);
                }
            }
        }
        else
        {
            for (partIndex = 0; partIndex < 1; ++partIndex)
            {
                v2 = DB_GetImageIndex(image);
                streamFrontendGlob.imageForceBits[(partIndex + v2) >> 5] &= ~(1 << ((partIndex + v2) & 0x1F));
            }
        }
        streamFrontendGlob.diskOrderImagesNeedSorting = 1;
    }
}

void __cdecl R_Stream_ForceLoadModel(XModel *model, int part)
{
    int lod; // [esp+0h] [ebp-14h]
    Material *const *material; // [esp+4h] [ebp-10h]
    int surfCount; // [esp+8h] [ebp-Ch]
    XSurface *surfaces; // [esp+Ch] [ebp-8h] BYREF
    int surfIter; // [esp+10h] [ebp-4h]

    if ( model )
    {
        for ( lod = 0; lod < 4; ++lod )
        {
            surfCount = XModelGetSurfaces(model, &surfaces, lod);
            material = XModelGetSkins(model, lod);
            surfIter = 0;
            while ( surfIter != surfCount )
            {
                R_Stream_ForceLoadMaterial(*material, part);
                ++surfIter;
                ++material;
            }
        }
    }
}

char __cdecl R_StreamTouchDObjAndCheck(const DObj *obj, int level)
{
    int i; // [esp+4h] [ebp-8Ch]
    unsigned int boneIndex; // [esp+Ch] [ebp-84h]
    XModel *model; // [esp+10h] [ebp-80h]
    char streamed; // [esp+17h] [ebp-79h]
    unsigned int boneCount; // [esp+18h] [ebp-78h]
    unsigned int surfaceIndex; // [esp+1Ch] [ebp-74h]
    unsigned int surfaceCount; // [esp+20h] [ebp-70h]
    char targBoneIndexLow; // [esp+24h] [ebp-6Ch]
    char invTargBoneIndexLow; // [esp+28h] [ebp-68h]
    Material **materials; // [esp+2Ch] [ebp-64h]
    XSurface *surfaces; // [esp+30h] [ebp-60h] BYREF
    unsigned int hidePartBits[5]; // [esp+34h] [ebp-5Ch] BYREF
    int targBoneIndexHigh; // [esp+48h] [ebp-48h]
    int surfPartBits[9]; // [esp+4Ch] [ebp-44h] BYREF
    XSurface *xsurf; // [esp+70h] [ebp-20h]
    int partBits[5]; // [esp+74h] [ebp-1Ch]
    unsigned int modelCount; // [esp+88h] [ebp-8h]
    unsigned int modelIndex; // [esp+8Ch] [ebp-4h]

    if (!obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp", 1704, 0, "%s", "obj"))
        __debugbreak();
    if (!Sys_IsMainThread()
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
            1705,
            0,
            "%s",
            "Sys_IsMainThread()"))
    {
        __debugbreak();
    }
    if (!useFastFile->current.enabled
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp", 1706, 0, "%s", "IsFastFileLoad()"))
    {
        __debugbreak();
    }
    boneIndex = 0;
    modelCount = DObjGetNumModels(obj);
    if (modelCount > 0x20
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
            1710,
            1,
            "%s",
            "modelCount <= DOBJ_MAX_SUBMODELS"))
    {
        __debugbreak();
    }
    memset(surfPartBits, 0, sizeof(surfPartBits));
    streamed = 1;
    modelIndex = 0;
    while (modelIndex < modelCount)
    {
        model = DObjGetModel(obj, modelIndex);
        if (!model && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp", 1719, 0, "%s", "model"))
            __debugbreak();
        materials = XModelGetSkins(model, 0);
        boneCount = XModelNumBones(model);
        surfaceCount = XModelGetSurfaces(model, &surfaces, 0);
        if (!surfaces
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp", 1726, 0, "%s", "surfaces"))
        {
            __debugbreak();
        }
        if (!surfaceCount
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp", 1727, 0, "%s", "surfaceCount"))
        {
            __debugbreak();
        }
        targBoneIndexHigh = boneIndex >> 5;
        targBoneIndexLow = boneIndex & 0x1F;
        invTargBoneIndexLow = 32 - (boneIndex & 0x1F);
        DObjGetHidePartBits(obj, hidePartBits);
        if (surfaceCount > 0x80
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
                1735,
                0,
                "%s\n\t(surfaceCount) = %i",
                "(surfaceCount <= DOBJ_MAX_SURFS)",
                surfaceCount))
        {
            __debugbreak();
        }
        for (surfaceIndex = 0; surfaceIndex < surfaceCount; ++surfaceIndex)
        {
            xsurf = &surfaces[surfaceIndex];
            for (i = 0; i < 5; ++i)
                surfPartBits[i + 4] = xsurf->partBits[i];
            if ((boneIndex & 0x1F) != 0)
            {
                partBits[0] = (unsigned int)surfPartBits[4 - targBoneIndexHigh] >> targBoneIndexLow;
                partBits[1] = ((unsigned int)surfPartBits[5 - targBoneIndexHigh] >> targBoneIndexLow)
                    | (surfPartBits[4 - targBoneIndexHigh] << invTargBoneIndexLow);
                partBits[2] = ((unsigned int)surfPartBits[6 - targBoneIndexHigh] >> targBoneIndexLow)
                    | (surfPartBits[5 - targBoneIndexHigh] << invTargBoneIndexLow);
                partBits[3] = ((unsigned int)surfPartBits[7 - targBoneIndexHigh] >> targBoneIndexLow)
                    | (surfPartBits[6 - targBoneIndexHigh] << invTargBoneIndexLow);
                partBits[4] = ((unsigned int)surfPartBits[8 - targBoneIndexHigh] >> targBoneIndexLow)
                    | (surfPartBits[7 - targBoneIndexHigh] << invTargBoneIndexLow);
            }
            else
            {
                partBits[0] = surfPartBits[4 - targBoneIndexHigh];
                partBits[1] = surfPartBits[5 - targBoneIndexHigh];
                partBits[2] = surfPartBits[6 - targBoneIndexHigh];
                partBits[3] = surfPartBits[7 - targBoneIndexHigh];
                partBits[4] = surfPartBits[8 - targBoneIndexHigh];
            }
            if (!(partBits[4] & hidePartBits[4]
                | partBits[3] & hidePartBits[3]
                | partBits[2] & hidePartBits[2]
                | partBits[1] & hidePartBits[1]
                | partBits[0] & hidePartBits[0]))
                streamed &= R_StreamTouchMaterialAndCheck(materials[surfaceIndex], level);
        }
        ++modelIndex;
        boneIndex += boneCount;
    }
    return streamed;
}

void __cdecl R_StreamTouchMaterial(Material *material)
{
    GfxImage *Texture; // eax
    int imageCount; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    imageCount = Material_GetTextureCount(material);
    for ( i = 0; i < imageCount; ++i )
    {
        Texture = Material_GetTexture(material, i);
        R_StreamTouchImage(Texture);
    }
}

char __cdecl R_StreamTouchMaterialAndCheck(Material *material, int level)
{
    GfxImage *Texture; // eax
    int imageCount; // [esp+4h] [ebp-Ch]
    char streamed; // [esp+Bh] [ebp-5h]
    int i; // [esp+Ch] [ebp-4h]

    if ( level
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
                    1799,
                    0,
                    "level doesn't index MAX_IMAGE_STREAMED_PARTS\n\t%i not in [0, %i)",
                    level,
                    1) )
    {
        __debugbreak();
    }
    streamed = 1;
    imageCount = Material_GetTextureCount(material);
    for ( i = 0; i < imageCount; ++i )
    {
        Texture = Material_GetTexture(material, i);
        streamed &= R_StreamTouchImageAndCheck(Texture, level);
    }
    return streamed;
}

void __cdecl R_StreamTouchImage(GfxImage *image)
{
    int part; // [esp+4h] [ebp-Ch]
    int imagePartIndex; // [esp+Ch] [ebp-4h]

    if (image->streaming)
    {
        imagePartIndex = DB_GetImageIndex(image);
        for (part = 0; part < 1; ++part)
        {
            streamFrontendGlob.imageTouchBits[streamFrontendGlob.activeImageTouchBits][imagePartIndex >> 5] |= 1 << (imagePartIndex & 0x1F);
            ++imagePartIndex;
        }
    }
}

bool __cdecl R_StreamTouchImageAndCheck(GfxImage *image, int level)
{
    int v3; // [esp+4h] [ebp-18h]
    int part; // [esp+8h] [ebp-14h]
    int imagePartIndex; // [esp+10h] [ebp-Ch]
    int levelPartIndex; // [esp+18h] [ebp-4h]

    if (level
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
            1833,
            0,
            "level doesn't index MAX_IMAGE_STREAMED_PARTS\n\t%i not in [0, %i)",
            level,
            1))
    {
        __debugbreak();
    }
    if (!image->streaming)
        return 1;
    if (level > 0)
        v3 = 0;
    else
        v3 = level;
    imagePartIndex = DB_GetImageIndex(image);
    levelPartIndex = imagePartIndex - v3;
    for (part = 0; part < 1; ++part)
    {
        streamFrontendGlob.imageTouchBits[streamFrontendGlob.activeImageTouchBits][imagePartIndex >> 5] |= 1 << (imagePartIndex & 0x1F);
        ++imagePartIndex;
    }
    return (streamFrontendGlob.imageUseBits[(levelPartIndex >> 5) - 4] & (1 << (levelPartIndex & 0x1F))) != 0;
}

bool __cdecl R_StreamImageCheck(GfxImage *image, int level)
{
    int v3; // [esp+0h] [ebp-14h]
    int imagePartIndex; // [esp+8h] [ebp-Ch]

    if (level
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
            1861,
            0,
            "level doesn't index MAX_IMAGE_STREAMED_PARTS\n\t%i not in [0, %i)",
            level,
            1))
    {
        __debugbreak();
    }
    if (!image->streaming)
        return 1;
    if (level > 0)
        v3 = 0;
    else
        v3 = level;
    imagePartIndex = DB_GetImageIndex(image);
    return (streamFrontendGlob.imageUseBits[((imagePartIndex - v3) >> 5) - 4] & (1 << ((imagePartIndex - v3) & 0x1F))) != 0;
}

void __cdecl R_Stream_ResetHintEntities()
{
    s_numStreamHintsActive = 0;
    memset((unsigned __int8 *)s_streamHints, 0, sizeof(s_streamHints));
}

void __cdecl R_StreamInit()
{
    int i; // [esp+0h] [ebp-4h]

    R_StreamAlloc_InitTempImages();
    streamFrontendGlob.frame = 0;
    streamFrontendGlob.queryInProgress = 0;
    streamFrontendGlob.queryClient = -1;
    for (i = 0; i < 10; ++i)
        R_Stream_InvalidateRequest(&s_pendingRequests[i]);
    memset((unsigned __int8 *)streamFrontendGlob.imageInSortedListBits, 0, 0x210u);
    memset((unsigned __int8 *)s_preventMaterials, 0, sizeof(s_preventMaterials));
    streamFrontendGlob.totalBytesWanted = 0;
    streamFrontendGlob.diskOrderImagesNeedSorting = 1;
    streamIsInitialized = 1;
}

void __cdecl R_StreamShutdown()
{
    R_StreamSetDefaultConfig(1);
    memset((unsigned __int8 *)s_preventMaterials, 0, sizeof(s_preventMaterials));
    memset((unsigned __int8 *)streamFrontendGlob.imageInSortedListBits, 0, 0x210u);
    streamFrontendGlob.totalBytesWanted = 0;
    streamFrontendGlob.diskOrderImagesNeedSorting = 1;
}

void __cdecl R_Stream_ReleaseImage(GfxImage *image, bool lock, bool delayDirty)
{
    int part; // [esp+0h] [ebp-14h]
    int freedSize; // [esp+4h] [ebp-10h] BYREF
    unsigned __int8 *memory; // [esp+8h] [ebp-Ch]
    int imagePartIndex; // [esp+Ch] [ebp-8h]
    int imageIndex; // [esp+10h] [ebp-4h]

    if (!image && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp", 2204, 0, "%s", "image"))
        __debugbreak();
    if (image->streaming)
    {
        if (lock)
            R_StreamAlloc_Lock();
        memory = R_StreamAlloc_FreeImage(image, 0, delayDirty, &freedSize);
        if (freedSize > 0)
        {
            if (!memory
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp", 2224, 0, "%s", "memory"))
            {
                __debugbreak();
            }
            //BG_EvalVehicleName();
        }
        imageIndex = DB_GetImageIndex(image);
        imagePartIndex = imageIndex;
        for (part = 0; part < 1; ++part)
        {
            streamFrontendGlob.imageInitialBits[(imagePartIndex + part) >> 5] &= ~(1 << ((imagePartIndex + part) & 0x1F));
            streamFrontendGlob.imageForceBits[(imagePartIndex + part) >> 5] &= ~(1 << ((imagePartIndex + part) & 0x1F));
        }
        streamFrontendGlob.diskOrderImagesNeedSorting = 1;
        if (lock)
            R_StreamAlloc_Unlock();
    }
}

void __cdecl R_StreamSyncThenFlush(bool flushAll)
{
    int i; // [esp+0h] [ebp-4h]

    if (streamIsInitialized)
    {
        Sys_EnterCriticalSection(CRITSECT_STREAM_SYNC_COMMAND);
        R_StreamPushDisable();
        R_Stream_Sync();
        if (flushAll)
        {
            R_StreamAlloc_Lock();
            R_StreamAlloc_Flush();
            R_StreamAlloc_Unlock();
        }
        streamFrontendGlob.frame = 0;
        streamFrontendGlob.queryInProgress = 0;
        streamFrontendGlob.syncThing = 0;
        Sys_EnterCriticalSection(CRITSECT_STREAM_FORCE_LOAD_COMMAND);
        s_numForcedLoadEntities = 0;
        for (i = 0; i < 4; ++i)
            s_forcedLoadEntities[i] = 0;
        Sys_LeaveCriticalSection(CRITSECT_STREAM_FORCE_LOAD_COMMAND);
        R_StreamPopDisable();
        Sys_LeaveCriticalSection(CRITSECT_STREAM_SYNC_COMMAND);
    }
}

void R_Stream_Sync()
{
    pendingRequest *request; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    int imageIndex; // [esp+8h] [ebp-4h]
    int imageIndexa; // [esp+8h] [ebp-4h]
    int imageIndexb; // [esp+8h] [ebp-4h]

    if (!streamIsInitialized
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
            2279,
            0,
            "%s",
            "streamIsInitialized"))
    {
        __debugbreak();
    }
    if (R_StreamIsEnabled()
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
            2286,
            0,
            "%s",
            "!R_StreamIsEnabled()"))
    {
        __debugbreak();
    }
    if (streamFrontendGlob.queryInProgress)
        R_StreamUpdate_EndQuery();
    if (streamFrontendGlob.queryInProgress
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
            2290,
            0,
            "%s",
            "!streamFrontendGlob.queryInProgress"))
    {
        __debugbreak();
    }
    R_StreamAlloc_Lock();
    for (i = 0; i < 10; ++i)
    {
        request = &s_pendingRequests[i];
        switch (request->status)
        {
        case STREAM_STATUS_PRE:
        case STREAM_STATUS_QUEUED:
            imageIndex = DB_GetImageIndex(request->image);
            streamFrontendGlob.imageLoading[(imageIndex >> 5) - 8] &= ~(1 << (imageIndex & 0x1F));
            R_Stream_InvalidateRequest(request);
            break;
        case STREAM_STATUS_FINISHED:
            imageIndexa = DB_GetImageIndex(request->image);
            streamFrontendGlob.imageLoading[(imageIndexa >> 5) - 8] &= ~(1 << (imageIndexa & 0x1F));
            Z_VirtualFree(request->buffer, 20);
            R_Stream_InvalidateRequest(request);
            break;
        case STREAM_STATUS_INPROGRESS:
            while (s_pendingRequests[i].status < STREAM_STATUS_CANCELLED
                || s_pendingRequests[i].status > STREAM_STATUS_FINISHED)
                NET_Sleep(1u);
            imageIndexb = DB_GetImageIndex(request->image);
            streamFrontendGlob.imageLoading[(imageIndexb >> 5) - 8] &= ~(1 << (imageIndexb & 0x1F));
            Z_VirtualFree(request->buffer, 20);
            R_Stream_InvalidateRequest(request);
            break;
        }
    }
    R_StreamAlloc_Unlock();
}

void __cdecl R_StreamPushSyncDisable()
{
    if ( streamIsInitialized )
    {
        Sys_EnterCriticalSection(CRITSECT_STREAM_SYNC_COMMAND);
        R_StreamPushDisable();
        R_Stream_Sync();
        Sys_LeaveCriticalSection(CRITSECT_STREAM_SYNC_COMMAND);
    }
    else
    {
        R_StreamPushDisable();
    }
}

void __cdecl R_StreamPopSyncDisable()
{
    if ( R_StreamIsEnabled()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
                    2533,
                    0,
                    "%s",
                    "!R_StreamIsEnabled()") )
    {
        __debugbreak();
    }
    if ( streamIsInitialized )
    {
        Sys_EnterCriticalSection(CRITSECT_STREAM_SYNC_COMMAND);
        R_Stream_Sync();
        R_StreamPopDisable();
        Sys_LeaveCriticalSection(CRITSECT_STREAM_SYNC_COMMAND);
    }
    else
    {
        R_StreamPopDisable();
    }
}

void __cdecl R_StreamUpdate_Idle()
{
    if ( DB_FinishedLoadingAssets() )
    {
        if ( R_StreamUpdate(0) )
            R_StreamUpdate_EndQuery();
    }
}

void __cdecl R_StreamUpdate_CompletePreload(void(__cdecl *pumpfunc)())
{
    float progress; // [esp+0h] [ebp-4h]

    R_BeginRemoteScreenUpdate();
    while (R_IsRemoteScreenUpdateActive())
    {
        if (!R_StreamIsEnabled())
            break;
        if (streamFrontendGlob.preloadCancelled)
            break;
        R_StreamAlloc_Lock();
        progress = R_Stream_GetProgress();
        R_StreamAlloc_Unlock();
        if (progress >= 1.0 || streamFrontendGlob.initialLoadAllocFailures > 0)
            break;
        NET_Sleep(0xAu);
    }
    R_EndRemoteScreenUpdate(pumpfunc);
}

char __cdecl R_StreamUpdate(const float *viewPos)
{
    char updateCalled; // [esp+13h] [ebp-1h]

    PROF_SCOPED("stream update");

    updateCalled = 0;
    if (r_streamClear->current.enabled || r_stream->modified)
    {
        R_StreamSyncThenFlush(1);
        Dvar_ClearModified(r_streamSize);
        Dvar_ClearModified(r_stream);
        Dvar_SetBool((dvar_s*)r_streamClear, 0);
    }
    if (r_streamSize->modified)
    {
        R_StreamSyncThenFlush(0);
        Dvar_ClearModified(r_streamSize);
    }
    if (r_streamLowDetail->modified)
    {
        if (r_streamLowDetail->current.enabled)
        {
            //BG_EvalVehicleName();
        }
        else
        {
            memset((unsigned __int8 *)streamFrontendGlob.imageForceBits, 0, 0x210u);
        }
        Dvar_ClearModified(r_streamLowDetail);
        streamFrontendGlob.diskOrderImagesNeedSorting = 1;
    }
    if (r_reflectionProbeGenerate->current.enabled)
    {
        return 0;
    }
    else
    {
        if (r_stream->current.integer > 0)
            updateCalled = R_StreamUpdate_FindImageAndOptimize(viewPos);
        return updateCalled;
    }
}

void __cdecl R_Stream_AddImagePartImportance(int imagePartIndex, float importance)
{
    float v2; // [esp+0h] [ebp-8h]

    bcassert(imagePartIndex, TOTAL_IMAGE_PARTS);

    if ((float)(importance - *(float *)&streamFrontendGlob.imageImportanceBits[imagePartIndex - 4064]) < 0.0)
        v2 = *(float *)&streamFrontendGlob.imageImportanceBits[imagePartIndex - 4064];
    else
        v2 = importance;
    *(float *)&streamFrontendGlob.imageImportanceBits[imagePartIndex - 4064] = v2;
    if ((streamFrontendGlob.dynamicImageImportanceBits[(imagePartIndex >> 5) - 4064] & (1 << (imagePartIndex & 0x1F))) == 0)
    {
        streamFrontendGlob.dynamicImageImportanceBits[(imagePartIndex >> 5) - 4064] |= 1 << (imagePartIndex & 0x1F);
        if ((streamFrontendGlob.imageInSortedListBits[imagePartIndex >> 5] & (1 << (imagePartIndex & 0x1F))) == 0)
        {
            streamFrontendGlob.imageInSortedListBits[imagePartIndex >> 5] |= 1 << (imagePartIndex & 0x1F);
            *(int *)((char *)&streamFrontendGlob.sortedImages[streamFrontendGlob.totalBytesWanted++] + 2) = (__int16)imagePartIndex;
        }
    }
}

void __cdecl importance_swap_func(void **a, void **b)
{
    void *temp; // [esp+0h] [ebp-4h]

    temp = *a;
    *a = *b;
    *b = temp;
}

bool __cdecl importance_compare_func(void *a, void *b)
{
    return (signed int)streamFrontendGlob.imageImportanceBits[(unsigned int)a - 4064] > (signed int)streamFrontendGlob.imageImportanceBits[(unsigned int)b - 4064];
}

void *aux_buffer[2113];
void __cdecl importance_merge_sort(void **list, int list_count)
{
    void **t; // [esp+0h] [ebp-14h]
    void **b; // [esp+8h] [ebp-Ch]
    void **ba; // [esp+8h] [ebp-Ch]
    void **a; // [esp+10h] [ebp-4h]
    void **aa; // [esp+10h] [ebp-4h]

    if ( list_count >= 3 )
    {
        importance_merge_sort(list, list_count / 2);
        importance_merge_sort(&list[list_count / 2], list_count - list_count / 2);
        b = &list[list_count / 2];
        if ( importance_compare_func(*b, *(b - 1)) )
        {
            t = aux_buffer;
            aux_buffer[0] = *(b - 1);
            for ( a = &list[list_count / 2 - 2]; a >= list && importance_compare_func(*b, *a); --a )
                *++t = *a;
            aa = a + 1;
            *aa = *b;
            ba = b + 1;
            while ( t >= aux_buffer && ba < &list[list_count] )
            {
                ++aa;
                if ( importance_compare_func(*t, *ba) )
                    *aa = *t--;
                else
                    *aa = *ba++;
            }
            while ( t >= aux_buffer )
                *++aa = *t--;
        }
    }
    else if ( list_count == 2 )
    {
        if ( importance_compare_func(list[1], *list) )
            importance_swap_func(list, list + 1);
    }
}

void __cdecl R_StreamUpdate_EndQuery()
{
    iassert(streamFrontendGlob.queryInProgress == 1);

    {
        PROF_SCOPED("wait r_stream_sort");
        Sys_WaitWorkerCmdInternal(&r_stream_sortWorkerCmd);
    }
    
    streamFrontendGlob.queryInProgress = 0;
    R_StreamAlloc_Lock();
    R_StreamUpdate_EndQuery_Internal();
    R_StreamAlloc_Unlock();
}

void  R_StreamUpdate_EndQuery_Internal()
{
    unsigned int v0; // eax
    int i; // [esp+28h] [ebp-24h]
    int sortedIndex; // [esp+2Ch] [ebp-20h]
    XAssetPoolEntry<GfxImage> *image; // [esp+30h] [ebp-1Ch]
    signed int imagePartIndex; // [esp+38h] [ebp-14h]
    pendingRequest *request; // [esp+3Ch] [ebp-10h]
    bool movesPending; // [esp+47h] [ebp-5h]
    int imagePart; // [esp+48h] [ebp-4h]

    sortedIndex = 0;

    PROF_SCOPED("R_Stream EndQuery");

    if (Sys_IsRenderThread())
        R_StreamUpdate_ProcessFileCallbacks();
LABEL_3:
    while (sortedIndex != streamFrontendGlob.totalBytesWanted)
    {
        movesPending = CG_IsShowingZombieMap();
        request = 0;
        for (i = 0; i < 10; ++i)
        {
            if (request || s_pendingRequests[i].status)
            {
                if (s_pendingRequests[i].status == STREAM_STATUS_PRE && !movesPending)
                    R_StreamRequestImageRead(&s_pendingRequests[i]);
            }
            else
            {
                request = &s_pendingRequests[i];
            }
        }
        if (!request)
        {
            return;
        }
        while (sortedIndex < streamFrontendGlob.totalBytesWanted)
        {
            imagePartIndex = *(int *)((char *)&streamFrontendGlob.sortedImages[sortedIndex] + 2);
            if ((streamFrontendGlob.imageUseBits[(imagePartIndex >> 5) - 4] & (1 << (imagePartIndex & 0x1F))) == 0)
            {
                v0 = imagePartIndex & 0x80000000;
                if (imagePartIndex < 0)
                    v0 = 0;
                imagePart = v0;
                if ((streamFrontendGlob.imageLoading[(imagePartIndex >> 5) - 8] & (1 << (imagePartIndex & 0x1F))) == 0)
                {
                    image = DB_GetImageAtIndex(imagePartIndex);
                    if (image->entry.streaming)
                    {
                        if (image->entry.streaming != 3 && image->entry.skippedMipLevels)
                        {
                            if (R_StreamRequestImageAllocation(
                                request,
                                &image->entry,
                                1,
                                imagePart,
                                *(float *)&streamFrontendGlob.imageImportanceBits[imagePartIndex - 4064]))
                            {
                                streamFrontendGlob.initialLoadAllocFailures = 0;
                                if (CG_IsShowingZombieMap() || R_StreamRequestImageRead(request))
                                    goto LABEL_3;
                            }
                            else
                            {
                                ++streamFrontendGlob.initialLoadAllocFailures;
                            }
                            return;
                        }
                    }
                }
            }
            ++sortedIndex;
        }
    }
}

char __cdecl R_StreamRequestImageAllocation(
    pendingRequest *request,
    GfxImage *image,
    bool highMip,
    int imagePart,
    float importance)
{
    char *v6; // eax
    int imageIndex; // [esp+Ch] [ebp-4h]

    if (!image && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp", 749, 0, "%s", "image"))
        __debugbreak();
    if (!request
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp", 750, 0, "%s", "request"))
    {
        __debugbreak();
    }
    if (image->streaming == 3
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
            752,
            0,
            "%s",
            "image->streaming != GFX_TEMP_STREAMING"))
    {
        __debugbreak();
    }
    if (!image->streaming)
        return 0;
    if (request->image
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
            758,
            0,
            "%s",
            "request->image == NULL"))
    {
        __debugbreak();
    }
    if (request->status
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
            759,
            0,
            "%s",
            "request->status == STREAM_STATUS_INVALID"))
    {
        __debugbreak();
    }
    request->image = image;
    request->imagePart = imagePart;
    request->importance = importance;
    request->startTime = Sys_Milliseconds();
    request->bufferSize = 0;
    request->buffer = 0;
    request->highMip = highMip;
    if (r_streamLog && r_streamLog->current.value > 0.0)
    {
        v6 = va(
            "-STREAM-allocation complete. bytes=%d,image=%s,imagePart=%d,importance=%f\n",
            request->bufferSize,
            image->name,
            imagePart,
            importance);
        Com_PrintMessage(16, v6, 0);
    }
    imageIndex = DB_GetImageIndex(image);
    if ((streamFrontendGlob.imageLoading[(imageIndex >> 5) - 8] & (1 << (imageIndex & 0x1F))) != 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
            786,
            0,
            "%s",
            "!( streamFrontendGlob.imageLoading[ BIT_INDEX_32( imageIndex ) ] & BIT_MASK_32( imageIndex ) )"))
    {
        __debugbreak();
    }
    streamFrontendGlob.imageLoading[(imageIndex >> 5) - 8] |= 1 << (imageIndex & 0x1F);
    request->status = STREAM_STATUS_PRE;
    request->id[0] = -1;
    return 1;
}

char __cdecl R_StreamRequestImageRead(pendingRequest *request)
{
    int unloadImagePartIndex; // [esp+10h] [ebp-18h]
    int i; // [esp+14h] [ebp-14h]
    pendingRequest *unloadRequest; // [esp+18h] [ebp-10h]
    GfxImage *unloadImage; // [esp+1Ch] [ebp-Ch] BYREF
    GfxImage *image; // [esp+20h] [ebp-8h]
    int imageIndex; // [esp+24h] [ebp-4h]

    image = request->image;
    imageIndex = DB_GetImageIndex(image);
    request->status = STREAM_STATUS_QUEUED;
    if (request->highMip)
    {
        unloadImage = 0;
        if (R_StreamAlloc_CanAllocate(image->baseSize, request->importance, &unloadImage) && unloadImage)
        {
            unloadRequest = 0;
            for (i = 0; i < 10; ++i)
            {
                if (s_pendingRequests[i].status == STREAM_STATUS_INVALID)
                {
                    unloadRequest = &s_pendingRequests[i];
                    break;
                }
            }
            if (unloadRequest)
            {
                unloadImagePartIndex = DB_GetImageIndex(unloadImage);
                if ((streamFrontendGlob.imageUseBits[(unloadImagePartIndex >> 5) - 4] & (1 << (unloadImagePartIndex & 0x1F))) == 0
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
                        890,
                        0,
                        "%s",
                        "streamFrontendGlob.imageUseBits[BIT_INDEX_32(unloadImagePartIndex)] & BIT_MASK_32(unloadImagePartIndex)"))
                {
                    __debugbreak();
                }
                if (R_StreamRequestImageAllocation(
                    unloadRequest,
                    unloadImage,
                    0,
                    0,
                    *(float *)&streamFrontendGlob.imageImportanceBits[unloadImagePartIndex - 4064])
                    && R_StreamRequestImageRead(unloadRequest))
                {
                    unloadImage = 0;
                }
            }
        }
        if (unloadImage)
        {
            streamFrontendGlob.imageLoading[(imageIndex >> 5) - 8] &= ~(1 << (imageIndex & 0x1F));
            R_Stream_InvalidateRequest(request);
            return 0;
        }
        else
        {
            Sys_WakeStream();
            return 1;
        }
    }
    else
    {
        Sys_WakeStream();
        return 1;
    }
}

char __cdecl R_StreamUpdate_FindImageAndOptimize(const float *viewPos)
{
    int hint; // [esp+1Ch] [ebp-2Ch]
    StreamUpdateCmd updateCmd; // [esp+20h] [ebp-28h] BYREF
    float maxDistSq; // [esp+3Ch] [ebp-Ch]
    StreamSortCmd sortCmd; // [esp+40h] [ebp-8h] BYREF

    if (!streamIsInitialized
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
            4536,
            0,
            "%s",
            "streamIsInitialized"))
    {
        __debugbreak();
    }
    if (r_streamFreezeState->current.enabled)
        return 0;
    if (r_streamCheckAabb->current.enabled)
        R_CheckHighmipAabbs();
    Sys_EnterCriticalSection(CRITSECT_STREAM_SYNC_COMMAND);
    if (streamFrontendGlob.queryInProgress)
        R_StreamUpdate_EndQuery();
    if (R_StreamUpdate_TryBeginQuery())
    {
        if (streamFrontendGlob.queryClient != -1
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
                4559,
                0,
                "%s",
                "streamFrontendGlob.queryClient == -1"))
        {
            __debugbreak();
        }
        maxDistSq = r_streamMaxDist->current.value * r_streamMaxDist->current.value;

        PROF_SCOPED("R_Stream update");

        streamFrontendGlob.queryInProgress = 1;
        streamFrontendGlob.queryClient = 0;
        if (rgp.world && viewPos)
        {
            streamFrontendGlob.diskOrderImagesNeedSorting = 1;
            ++streamFrontendGlob.frame;
            streamFrontendGlob.diskOrder = streamFrontendGlob.forceDiskOrder;
            R_StreamUpdatePreventedMaterials();
            R_StreamUpdateForcedModels();
            R_StreamUpdateTouchedModels();
            memset((unsigned __int8 *)streamFrontendGlob.modelTouchBits, 0, sizeof(streamFrontendGlob.modelTouchBits));
            R_StreamUpdate_AddForcedImages(
                streamFrontendGlob.forcedImageImportance,
                streamFrontendGlob.touchedImageImportance);
            updateCmd.frontend = &streamFrontendGlob;
            updateCmd.viewPos[0] = *viewPos;
            updateCmd.viewPos[1] = viewPos[1];
            updateCmd.viewPos[2] = viewPos[2];
            updateCmd.maxDistSq = maxDistSq;
            updateCmd.distanceScale[1] = 1.0f;
            LODWORD(updateCmd.distanceScale[0]) = r_streamHiddenPush->current.integer;
            Sys_AddWorkerCmdInternal(&r_stream_updateWorkerCmd, (unsigned __int8 *)&updateCmd, 0);
            for (hint = 0; hint < 4; ++hint)
            {
                if (s_streamHints[hint].importance > 0.0)
                {
                    updateCmd.distanceScale[1] = 1.0 / s_streamHints[hint].importance;
                    updateCmd.distanceScale[0] = updateCmd.distanceScale[1];
                    updateCmd.viewPos[0] = s_streamHints[hint].origin[0];
                    updateCmd.viewPos[1] = s_streamHints[hint].origin[1];
                    updateCmd.viewPos[2] = s_streamHints[hint].origin[2];
                    Sys_AddWorkerCmdInternal(&r_stream_updateWorkerCmd, (unsigned __int8 *)&updateCmd, 0);
                }
            }
            Sys_LeaveCriticalSection(CRITSECT_STREAM_SYNC_COMMAND);
            return 1;
        }
        else
        {
            ++streamFrontendGlob.frame;
            streamFrontendGlob.diskOrder = 1;
            R_StreamUpdate_AddForcedImages(
                streamFrontendGlob.forcedImageImportance,
                streamFrontendGlob.touchedImageImportance);
            R_StreamUpdate_AddInitialImages(streamFrontendGlob.initialImageImportance);
            sortCmd.frontend = &streamFrontendGlob;
            sortCmd.diskOrder = 1;
            Sys_AddWorkerCmdInternal(&r_stream_sortWorkerCmd, (unsigned __int8 *)&sortCmd, 0);
            streamFrontendGlob.queryClient = -1;
            Sys_LeaveCriticalSection(CRITSECT_STREAM_SYNC_COMMAND);
            return 1;
        }
    }
    else
    {
        Sys_LeaveCriticalSection(CRITSECT_STREAM_SYNC_COMMAND);
        return 0;
    }
}

void __cdecl R_StreamUpdate_AddInitialImages(float importance)
{
    int imagePartIndex; // [esp+4h] [ebp-4h]

    for (imagePartIndex = 0; imagePartIndex < 4224; ++imagePartIndex)
    {
        if ((streamFrontendGlob.imageInitialBits[imagePartIndex >> 5] & (1 << (imagePartIndex & 0x1F))) != 0)
            R_Stream_AddImagePartImportance(imagePartIndex, importance);
    }
}

// aislop to remove bitscan
void R_StreamUpdate_AddForcedImages(float forceImportance, float touchImportance)
{
    streamFrontendGlob.activeImageTouchBits ^= 1u;

    for (int index = 0; index < 132; ++index)
    {
        unsigned int touchBits = streamFrontendGlob.imageTouchBits[1][index] | streamFrontendGlob.imageTouchBits[0][index];
        unsigned int forceBits = streamFrontendGlob.imageForceBits[index] | touchBits;
        unsigned int useBits = streamFrontendGlob.imageUseBits[index - 4] & ~forceBits;

        // process forced/touched bits
        for (int bitIndex = 0; bitIndex < 32; ++bitIndex)
        {
            unsigned int mask = 1u << bitIndex;
            if (forceBits & mask)
            {
                forceBits &= ~mask;

                float importance = (touchBits & mask) ? touchImportance : forceImportance;
                R_Stream_AddImagePartImportance(bitIndex + 32 * index, importance);
            }
        }

        // process remaining useBits
        for (int bitIndex = 0; bitIndex < 32; ++bitIndex)
        {
            unsigned int mask = 1u << bitIndex;
            if (useBits & mask)
            {
                useBits &= ~mask;
                R_Stream_AddImagePartImportance(bitIndex + 32 * index, 0.0f);
            }
        }
    }

    memset(
        streamFrontendGlob.imageTouchBits[streamFrontendGlob.activeImageTouchBits ^ 1],
        0,
        sizeof(streamFrontendGlob.imageTouchBits[streamFrontendGlob.activeImageTouchBits ^ 1])
    );
}


void R_StreamUpdatePreventedMaterials()
{
    unsigned int materialIndex; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    memset((unsigned __int8 *)streamFrontendGlob.materialPreventBits, 0, sizeof(streamFrontendGlob.materialPreventBits));
    for (i = 0; i < 32; ++i)
    {
        if (s_preventMaterials[i])
        {
            materialIndex = DB_GetMaterialIndex(s_preventMaterials[i]);
            streamFrontendGlob.materialPreventBits[materialIndex >> 5] |= 1 << (materialIndex & 0x1F);
        }
    }
}

void R_CheckHighmipAabbs()
{
    float mins[3]; // [esp+0h] [ebp-18h] BYREF
    float maxs[3]; // [esp+Ch] [ebp-Ch] BYREF

    maxs[0] = 131072.0f;
    maxs[1] = 131072.0f;
    maxs[2] = 131072.0f;
    mins[0] = -131072.0f;
    mins[1] = -131072.0f;
    mins[2] = -131072.0f;
    R_CheckHighmipAabbs_r(rgp.world, 0, mins, maxs);
}

void __cdecl R_CheckHighmipAabbs_r(GfxWorld *world, int aabbTreeNode, float *mins, float *maxs)
{
    bool v4; // [esp+0h] [ebp-4Ch]
    bool v5; // [esp+4h] [ebp-48h]
    GfxSurface *surface; // [esp+10h] [ebp-3Ch]
    int ref; // [esp+14h] [ebp-38h]
    int childIter; // [esp+18h] [ebp-34h]
    GfxStreamingAabbTree *tree; // [esp+1Ch] [ebp-30h]
    float myMins[3]; // [esp+20h] [ebp-2Ch] BYREF
    float myMaxs[3]; // [esp+2Ch] [ebp-20h] BYREF
    int childEnd; // [esp+38h] [ebp-14h]
    int childBegin; // [esp+3Ch] [ebp-10h]
    int *leafRefEnd; // [esp+40h] [ebp-Ch]
    int *leafRefBegin; // [esp+44h] [ebp-8h]
    int *leafRefIter; // [esp+48h] [ebp-4h]

    if ( (aabbTreeNode < 0 || aabbTreeNode >= world->streamInfo.aabbTreeCount)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
                    2649,
                    0,
                    "%s\n\t(aabbTreeNode) = %i",
                    "(aabbTreeNode >= 0 && aabbTreeNode < world->streamInfo.aabbTreeCount)",
                    aabbTreeNode) )
    {
        __debugbreak();
    }
    tree = &world->streamInfo.aabbTrees[aabbTreeNode];
    v5 = PointInBounds(tree->mins, mins, maxs) && PointInBounds(tree->maxs, mins, maxs);
    if ( !v5
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
                    2652,
                    0,
                    "%s",
                    "BoundsInBounds( mins, maxs, tree->mins, tree->maxs )") )
    {
        __debugbreak();
    }
    myMins[0] = tree->mins[0];
    myMins[1] = tree->mins[1];
    myMins[2] = tree->mins[2];
    myMaxs[0] = tree->maxs[0];
    myMaxs[1] = tree->maxs[1];
    myMaxs[2] = tree->maxs[2];
    if ( tree->childCount )
    {
        childBegin = tree->firstChild;
        childEnd = tree->childCount + tree->firstChild;
        for ( childIter = childBegin; childIter != childEnd; ++childIter )
            R_CheckHighmipAabbs_r(world, childIter, myMins, myMaxs);
    }
    else
    {
        leafRefBegin = &world->streamInfo.leafRefs[tree->firstItem];
        leafRefEnd = &leafRefBegin[tree->itemCount];
        for ( leafRefIter = leafRefBegin; leafRefIter != leafRefEnd; ++leafRefIter )
        {
            ref = *leafRefIter;
            if ( *leafRefIter >= 0 )
            {
                if ( ref >= world->surfaceCount
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
                                2677,
                                0,
                                "%s\n\t(ref) = %i",
                                "(ref >= 0 && ref < world->surfaceCount)",
                                ref) )
                {
                    __debugbreak();
                }
                surface = &world->dpvs.surfaces[ref];
                v4 = PointInBounds(surface->tris.mins, myMins, myMaxs) && PointInBounds(surface->tris.maxs, myMins, myMaxs);
                if ( !v4
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
                                2679,
                                0,
                                "%s",
                                "BoundsInBounds( myMins, myMaxs, surface->tris.mins, surface->tris.maxs )") )
                {
                    __debugbreak();
                }
            }
        }
    }
}

char __cdecl R_StreamUpdate_TryBeginQuery()
{
    if (streamFrontendGlob.queryInProgress || !R_StreamIsEnabled())
        return 0;
    if (streamFrontendGlob.outputImageList)
    {
        R_ImageList_Output();
        streamFrontendGlob.outputImageList = 0;
    }
    memset((unsigned __int8 *)streamFrontendGlob.materialImportance, 0, sizeof(streamFrontendGlob.materialImportance));
    memset(
        (unsigned __int8 *)streamFrontendGlob.materialImportanceBits,
        0,
        sizeof(streamFrontendGlob.materialImportanceBits));
    memset((unsigned __int8 *)streamFrontendGlob.modelDistance, 0, sizeof(streamFrontendGlob.modelDistance));
    memset((unsigned __int8 *)streamFrontendGlob.modelDistanceBits, 0, sizeof(streamFrontendGlob.modelDistanceBits));
    memset((unsigned __int8 *)&streamFrontendGlob.imageImportance[32], 0, 0x4200u);
    memset((unsigned __int8 *)&streamFrontendGlob.dynamicImageImportance[32], 0, 528u);
    memset((unsigned __int8 *)streamFrontendGlob.dynamicModelDistance, 0, sizeof(streamFrontendGlob.dynamicModelDistance));
    memset(
        (unsigned __int8 *)streamFrontendGlob.dynamicModelDistanceBits,
        0,
        sizeof(streamFrontendGlob.dynamicModelDistanceBits));
    return 1;
}

void R_StreamUpdateTouchedModels()
{
    XAssetPoolEntry<XModel> *model; // [esp+0h] [ebp-8h]
    signed int modelIndex; // [esp+4h] [ebp-4h]

    for (modelIndex = 0; (unsigned int)modelIndex < 0x3E8; ++modelIndex)
    {
        if ((streamFrontendGlob.modelTouchBits[modelIndex >> 5] & (1 << (modelIndex & 0x1F))) != 0)
        {
            model = DB_GetXModelAtIndex(modelIndex);
            if (!model
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp", 3570, 0, "%s", "model"))
            {
                __debugbreak();
            }
            R_StreamUpdateForXModelTouched(&model->entry);
        }
    }
}

void __cdecl R_StreamUpdateForXModelTouched(const XModel *model)
{
    int lod; // [esp+0h] [ebp-14h]
    Material *const *material; // [esp+4h] [ebp-10h]
    int surfCount; // [esp+8h] [ebp-Ch]
    XSurface *surfaces; // [esp+Ch] [ebp-8h] BYREF
    int surfIter; // [esp+10h] [ebp-4h]

    if ( model )
    {
        for ( lod = 0; lod < 4; ++lod )
        {
            surfCount = XModelGetSurfaces(model, &surfaces, lod);
            material = XModelGetSkins(model, lod);
            surfIter = 0;
            while ( surfIter != surfCount )
            {
                R_StreamTouchMaterial(*material);
                ++surfIter;
                ++material;
            }
        }
    }
}

void __cdecl R_StreamUpdateForcedModels()
{
    int lod; // [esp+24h] [ebp-30h]
    float distSq; // [esp+28h] [ebp-2Ch]
    int i; // [esp+30h] [ebp-24h]
    XModel *model; // [esp+34h] [ebp-20h]
    Material **material; // [esp+38h] [ebp-1Ch]
    int surfCount; // [esp+3Ch] [ebp-18h]
    XSurface *surfaces; // [esp+40h] [ebp-14h] BYREF
    int modelIter; // [esp+44h] [ebp-10h]
    int surfIter; // [esp+48h] [ebp-Ch]
    int surfCountPrevLods; // [esp+4Ch] [ebp-8h]
    int modelCount; // [esp+50h] [ebp-4h]

    surfCountPrevLods = 0;
    Sys_EnterCriticalSection(CRITSECT_STREAM_FORCE_LOAD_COMMAND);
    for (i = 0; i < s_numForcedLoadEntities; ++i)
    {
        modelCount = DObjGetNumModels(s_forcedLoadEntities[i]);
        distSq = streamFrontendGlob.forcedImageImportance;
        for (modelIter = 0; modelIter != modelCount; ++modelIter)
        {
            model = DObjGetModel(s_forcedLoadEntities[i], modelIter);
            for (lod = 0; lod < 4; ++lod)
            {
                surfCount = XModelGetSurfaces(model, &surfaces, lod);
                material = XModelGetSkins(model, lod);
                surfIter = 0;
                while (surfIter != surfCount)
                {
                    if ((*material)->textureCount)
                        R_StreamTouchImagesFromMaterial(*material, distSq);
                    ++surfIter;
                    ++material;
                }
                surfCountPrevLods += surfCount;
            }
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_STREAM_FORCE_LOAD_COMMAND);
}

void __cdecl R_StreamTouchImagesFromMaterial(const Material *remoteMaterial, float importance)
{
    GfxImage *image; // [esp+4h] [ebp-20h]
    MaterialTextureDef *texDef; // [esp+Ch] [ebp-18h]
    int textureIter; // [esp+10h] [ebp-14h]
    int imageIndex; // [esp+14h] [ebp-10h]
    MaterialTextureDef *textureTable; // [esp+1Ch] [ebp-8h]
    int textureCount; // [esp+20h] [ebp-4h]

    textureCount = remoteMaterial->textureCount;
    if ( remoteMaterial->textureCount )
    {
        textureTable = remoteMaterial->textureTable;
        for ( textureIter = 0; textureIter != textureCount; ++textureIter )
        {
            texDef = &textureTable[textureIter];
            if ( !texDef
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp", 3395, 0, "%s", "texDef") )
            {
                __debugbreak();
            }
            if ( texDef->semantic != 11 )
            {
                image = texDef->u.image;
                if ( !image
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp", 3401, 0, "%s", "image") )
                {
                    __debugbreak();
                }
                if ( image->streaming )
                {
                    imageIndex = DB_GetImageIndex(image);
                    R_Stream_AddImagePartImportance(imageIndex, importance);
                }
            }
        }
    }
}

void __cdecl R_StreamUpdatePerClient(const float *viewPos)
{
    StreamUpdateCmd updateCmd; // [esp+14h] [ebp-28h] BYREF
    float distanceScale[2]; // [esp+30h] [ebp-Ch] BYREF
    float maxDistSq; // [esp+38h] [ebp-4h]

    if (!streamIsInitialized
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
            4649,
            0,
            "%s",
            "streamIsInitialized"))
    {
        __debugbreak();
    }
    if (!r_streamFreezeState->current.enabled && rgp.world && viewPos)
    {
        Sys_EnterCriticalSection(CRITSECT_STREAM_SYNC_COMMAND);
        if (streamFrontendGlob.queryInProgress && streamFrontendGlob.queryClient != -1)
        {
            PROF_SCOPED("R_Stream update per client");
            maxDistSq = r_streamMaxDist->current.value * r_streamMaxDist->current.value;
            if (streamFrontendGlob.queryClient > 0)
            {
                updateCmd.frontend = &streamFrontendGlob;
                updateCmd.viewPos[0] = *viewPos;
                updateCmd.viewPos[1] = viewPos[1];
                updateCmd.viewPos[2] = viewPos[2];
                updateCmd.maxDistSq = maxDistSq;
                updateCmd.distanceScale[1] = 1.0f;
                LODWORD(updateCmd.distanceScale[0]) = r_streamHiddenPush->current.integer;
                Sys_AddWorkerCmdInternal(&r_stream_updateWorkerCmd, (unsigned __int8 *)&updateCmd, 0);
            }
            distanceScale[1] = 1.0f;
            distanceScale[0] = r_streamHiddenPush->current.value;
            R_StreamUpdateDynamicModels(viewPos, maxDistSq, streamFrontendGlob.frame, distanceScale);
            ++streamFrontendGlob.queryClient;
            Sys_LeaveCriticalSection(CRITSECT_STREAM_SYNC_COMMAND);
        }
        else
        {
            Sys_LeaveCriticalSection(CRITSECT_STREAM_SYNC_COMMAND);
        }
    }
}

void __cdecl R_StreamUpdateDynamicModels(
    const float *viewPos,
    float maxDistSq,
    unsigned int frame,
    float *distanceScale)
{
    unsigned __int16 physicsBrushModel; // [esp+1Ah] [ebp-12Eh]
    volatile int brushModelIndex; // [esp+104h] [ebp-44h]
    DynEntityPose *ClientPose; // [esp+108h] [ebp-40h]
    DynEntityPose *pose; // [esp+10Ch] [ebp-3Ch]
    GfxSceneDynBrush *sceneDynBrush; // [esp+110h] [ebp-38h]
    XModel *model; // [esp+118h] [ebp-30h]
    const DynEntityDef *dynEntDef; // [esp+120h] [ebp-28h]
    const DynEntityDef *dynEntDefa; // [esp+120h] [ebp-28h]
    GfxSceneEntity *sceneEnt; // [esp+124h] [ebp-24h]
    int entIndex; // [esp+128h] [ebp-20h]
    int entIndexa; // [esp+128h] [ebp-20h]
    int entIndexb; // [esp+128h] [ebp-20h]
    int entIndexc; // [esp+128h] [ebp-20h]
    int entIndexd; // [esp+128h] [ebp-20h]
    unsigned __int16 dynEntId; // [esp+12Ch] [ebp-1Ch]
    unsigned __int16 dynEntIda; // [esp+12Ch] [ebp-1Ch]
    int modelIter; // [esp+130h] [ebp-18h]
    int modelCount; // [esp+138h] [ebp-10h]
    GfxBrushModel *bmodel; // [esp+13Ch] [ebp-Ch]
    volatile int entCount; // [esp+144h] [ebp-4h]
    volatile int entCounta; // [esp+144h] [ebp-4h]
    signed int entCountb; // [esp+144h] [ebp-4h]
    signed int entCountc; // [esp+144h] [ebp-4h]
    volatile int entCountd; // [esp+144h] [ebp-4h]

    entCount = scene.sceneDObjCount;
    for (entIndex = 0; entIndex < entCount; ++entIndex)
    {
        sceneEnt = &scene.sceneDObj[entIndex];
        if ((!rg.enablePlayerShadowFlag || !DObjIsPlayerShadow(sceneEnt->obj))
            && (frontEndDataOut->gfxEnts[sceneEnt->gfxEntIndex].renderFxFlags & 1) == 0)
        {
            modelCount = DObjGetNumModels(sceneEnt->obj);
            for (modelIter = 0; modelIter != modelCount; ++modelIter)
            {
                model = DObjGetModel(sceneEnt->obj, modelIter);
                R_StreamUpdate_AddDynamicXModelDistance(
                    model,
                    viewPos,
                    sceneEnt->placement.base.origin,
                    sceneEnt->placement.scale,
                    scene.sceneDObjVisData[0][entIndex] == 1,
                    distanceScale);
            }
        }
    }
    entCounta = scene.sceneModelCount;
    for (entIndexa = 0; entIndexa < entCounta; ++entIndexa)
        R_StreamUpdate_AddDynamicXModelDistance(
            scene.sceneModel[entIndexa].model,
            viewPos,
            scene.sceneModel[entIndexa].placement.base.origin,
            scene.sceneModel[entIndexa].placement.scale,
            scene.sceneModelVisData[0][entIndexa] == 1,
            distanceScale);
    entCountb = scene.sceneDynBrushCount;
    for (entIndexb = 0; entIndexb < entCountb; ++entIndexb)
    {
        sceneDynBrush = &rgp.world->sceneDynBrush[entIndexb];
        dynEntId = sceneDynBrush->dynEntId;
        dynEntDef = DynEnt_GetEntityDef(dynEntId, DYNENT_DRAW_BRUSH);
        if (dynEntDef->physicsBrushModel)
            physicsBrushModel = dynEntDef->physicsBrushModel;
        else
            physicsBrushModel = dynEntDef->brushModel;
        bmodel = R_GetBrushModel(physicsBrushModel);
        pose = DynEnt_GetClientPose(dynEntId, DYNENT_DRAW_BRUSH);
        R_StreamUpdateForBModel(
            viewPos,
            frame,
            sceneDynBrush->info.surfId,
            bmodel,
            pose->pose.origin,
            maxDistSq,
            0,
            1,
            distanceScale);
    }
    entCountc = scene.sceneDynModelCount;
    for (entIndexc = 0; entIndexc < entCountc; ++entIndexc)
    {
        dynEntIda = rgp.world->sceneDynModel[entIndexc].dynEntId;
        dynEntDefa = DynEnt_GetEntityDef(dynEntIda, DYNENT_DRAW_MODEL);
        _mm_prefetch((const char *)dynEntDefa->xModel, 1);
        _mm_prefetch((const char *)dynEntDefa->destroyedxModel, 1);
        ClientPose = DynEnt_GetClientPose(dynEntIda, DYNENT_DRAW_MODEL);
        R_StreamUpdate_AddDynamicXModelDistance(dynEntDefa->xModel, viewPos, ClientPose->pose.origin, 1.0, 1, distanceScale);
        if (dynEntDefa->destroyedxModel)
            R_StreamUpdate_AddDynamicXModelDistance(
                dynEntDefa->destroyedxModel,
                viewPos,
                ClientPose->pose.origin,
                1.0,
                1,
                distanceScale);
    }
    for (brushModelIndex = 0; brushModelIndex < scene.sceneBrushCount; ++brushModelIndex)
    {
        if (scene.sceneBrushVisData[0][brushModelIndex] == 1)
            R_StreamUpdateForBModel(
                viewPos,
                frame,
                scene.sceneBrush[brushModelIndex].info.surfId,
                scene.sceneBrush[brushModelIndex].bmodel,
                scene.sceneBrush[brushModelIndex].placement.origin,
                maxDistSq,
                0,
                1,
                distanceScale);
    }
    entCountd = scene.glassBrushCount;
    for (entIndexd = 0; entIndexd < entCountd; ++entIndexd)
    {
        if (scene.glassBrushVisData[entIndexd] == 1)
            R_StreamUpdateForBModel(
                viewPos,
                frame,
                *(unsigned __int16 *)&scene.glassBrushVisData[40 * entIndexd - 40924],
                *(const GfxBrushModel **)&scene.glassBrushVisData[40 * entIndexd - 40932],
                (const float *)&scene.glassBrushVisData[40 * entIndexd - 40944],
                maxDistSq,
                *(Material **)&scene.glassBrushVisData[40 * entIndexd - 40928],
                1,
                distanceScale);
    }
}

void __cdecl R_StreamUpdateForBModel(
    const float *viewPos,
    unsigned int frame,
    unsigned int surfId,
    const GfxBrushModel *bmodel,
    const float *origin,
    float maxDistSq,
    Material *altMaterial,
    bool isVisible,
    float *distanceScale)
{
    float mip1radiusSq; // [esp+50h] [ebp-44h]
    float v10; // [esp+54h] [ebp-40h]
    float distSq; // [esp+78h] [ebp-1Ch]
    float distSq_4; // [esp+7Ch] [ebp-18h]
    unsigned int surfIndex; // [esp+80h] [ebp-14h]
    Material *material; // [esp+84h] [ebp-10h]
    BModelSurface *modelSurf; // [esp+88h] [ebp-Ch]
    unsigned int count; // [esp+90h] [ebp-4h]

    modelSurf = (BModelSurface *)((char *)frontEndDataOut + 4 * surfId);
    count = 0;
    surfIndex = bmodel->startSurfIndex;
    while (count < bmodel->surfaceCount)
    {
        material = modelSurf->surf->material;
        if (material && material->textureCount)
        {
            _mm_prefetch((const char *)&modelSurf[1], 1);
            if (rg.renderHiResShot && !isVisible)
                return;
            mip1radiusSq = bmodel->writable.mip1radiusSq;
            v10 = distanceScale[isVisible];
            distSq_4 = PointDistSqFromBounds(viewPos, bmodel->writable.mins, bmodel->writable.maxs) * v10;
            distSq = mip1radiusSq / (float)(distSq_4 + 100.0);
            _mm_prefetch((const char *)modelSurf[1].surf, 1);
            R_StreamTouchImagesFromMaterial(material, distSq);
            if (altMaterial)
                R_StreamTouchImagesFromMaterial(altMaterial, distSq);
        }
        ++count;
        ++surfIndex;
        ++modelSurf;
        surfId += 5;
    }
}

double __cdecl PointDistSqFromBounds(const float *v, const float *mins, const float *maxs)
{
    float v5; // [esp+4h] [ebp-38h]
    float v6; // [esp+8h] [ebp-34h]
    float v7; // [esp+Ch] [ebp-30h]
    float v8; // [esp+10h] [ebp-2Ch]
    float v9; // [esp+14h] [ebp-28h]
    float v10; // [esp+18h] [ebp-24h]

    if ( (float)(*mins - *v) < 0.0 )
        v10 = 0.0f;
    else
        v10 = *mins - *v;
    if ( (float)(*v - *maxs) < 0.0 )
        v9 = 0.0f;
    else
        v9 = *v - *maxs;
    if ( (float)(mins[1] - v[1]) < 0.0 )
        v8 = 0.0f;
    else
        v8 = mins[1] - v[1];
    if ( (float)(v[1] - maxs[1]) < 0.0 )
        v7 = 0.0f;
    else
        v7 = v[1] - maxs[1];
    if ( (float)(mins[2] - v[2]) < 0.0 )
        v6 = 0.0f;
    else
        v6 = mins[2] - v[2];
    if ( (float)(v[2] - maxs[2]) < 0.0 )
        v5 = 0.0f;
    else
        v5 = v[2] - maxs[2];
    return (float)((float)(v10 + v9) * (float)(v10 + v9)
                             + (float)(v8 + v7) * (float)(v8 + v7)
                             + (float)(v6 + v5) * (float)(v6 + v5));
}

void __cdecl R_StreamUpdate_AddDynamicXModelDistance(
    const XModel *model,
    const float *viewPos,
    const float *origin,
    float scale,
    bool visible,
    float *distanceScale)
{
    float v6; // [esp+0h] [ebp-1Ch]
    int modelIndex; // [esp+14h] [ebp-8h]
    float distSq; // [esp+18h] [ebp-4h]

    modelIndex = DB_GetXModelIndex(model);
    distSq = Vec3DistanceSq(viewPos, origin) / scale * distanceScale[visible];
    if ((streamFrontendGlob.dynamicModelDistanceBits[modelIndex >> 5] & (1 << (modelIndex & 0x1F))) != 0)
    {
        if ((float)(distSq - streamFrontendGlob.dynamicModelDistance[modelIndex]) < 0.0)
            v6 = distSq;
        else
            v6 = streamFrontendGlob.dynamicModelDistance[modelIndex];
        streamFrontendGlob.dynamicModelDistance[modelIndex] = v6;
    }
    else
    {
        streamFrontendGlob.dynamicModelDistance[modelIndex] = distSq;
        streamFrontendGlob.dynamicModelDistanceBits[modelIndex >> 5] |= 1 << (modelIndex & 0x1F);
    }
}

void __cdecl R_StreamUpdate_End()
{
    StreamCombineCmd combineCmd; // [esp+0h] [ebp-Ch] BYREF
    StreamSortCmd sortCmd; // [esp+4h] [ebp-8h] BYREF

    if (streamFrontendGlob.queryClient <= 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
            4709,
            0,
            "%s",
            "streamFrontendGlob.queryClient > 0"))
    {
        __debugbreak();
    }
    Sys_EnterCriticalSection(CRITSECT_STREAM_SYNC_COMMAND);
    combineCmd.frontend = &streamFrontendGlob;
    Sys_AddWorkerCmdInternal(&r_stream_combineWorkerCmd, (unsigned __int8 *)&combineCmd, 0);
    sortCmd.frontend = &streamFrontendGlob;
    sortCmd.diskOrder = streamFrontendGlob.diskOrder;
    Sys_AddWorkerCmdInternal(&r_stream_sortWorkerCmd, (unsigned __int8 *)&sortCmd, 0);
    streamFrontendGlob.queryClient = -1;
    Sys_LeaveCriticalSection(CRITSECT_STREAM_SYNC_COMMAND);
}

float4 s_viewPos;
void __cdecl R_Stream_UpdateStaticModelsCmd(char *data)
{
    PROF_SCOPED("R_Stream_UpdateStaticModelsCmd");

    StreamUpdateCmd *cmd = (StreamUpdateCmd *)data;

    s_viewPos = *(float4 *)cmd->viewPos;
    for (unsigned int instId = 0; instId < g_worldDpvs->smodelCount; ++instId)
    {
        R_StreamUpdateStaticModel(instId, cmd->viewPos, cmd->maxDistSq, cmd->distanceScale);
    }
}

void __cdecl R_Stream_UpdateStaticSurfacesCmd(char *data)
{
    PROF_SCOPED("R_Stream_UpdateStaticSurfacesCmd");

    StreamUpdateCmd *cmd = (StreamUpdateCmd *)data;
    s_viewPos = *(float4 *)cmd->viewPos;

    for (unsigned int surfId = 0; surfId < g_worldDpvs->staticSurfaceCount; ++surfId)
    {
        R_StreamUpdateWorldSurface(surfId, cmd->viewPos, cmd->maxDistSq, cmd->distanceScale);
    }
}

void __cdecl R_StreamUpdateStaticModel(
                int staticModelIndex,
                const float *viewPos,
                float maxDistSq,
                float *distanceScale)
{
    R_StreamUpdate_AddXModelDistance(
        rgp.world->dpvs.smodelDrawInsts[staticModelIndex].model,
        viewPos,
        rgp.world->dpvs.smodelDrawInsts[staticModelIndex].placement.origin,
        rgp.world->dpvs.smodelDrawInsts[staticModelIndex].placement.scale,
        ((0x80000000 >> (staticModelIndex & 0x1F)) & rgp.world->dpvs.smodelVisDataCameraSaved[staticModelIndex >> 5]) != 0,
        distanceScale);
}

void __cdecl R_StreamUpdate_AddXModelDistance(
    const XModel *model,
    const float *viewPos,
    const float *origin,
    float scale,
    bool visible,
    float *distanceScale)
{
    float v6; // [esp+0h] [ebp-1Ch]
    int modelIndex; // [esp+14h] [ebp-8h]
    float distSq; // [esp+18h] [ebp-4h]

    modelIndex = DB_GetXModelIndex(model);
    distSq = Vec3DistanceSq(viewPos, origin) / scale * distanceScale[visible];
    if ((streamFrontendGlob.modelDistanceBits[modelIndex >> 5] & (1 << (modelIndex & 0x1F))) != 0)
    {
        if ((float)(distSq - streamFrontendGlob.modelDistance[modelIndex]) < 0.0)
            v6 = distSq;
        else
            v6 = streamFrontendGlob.modelDistance[modelIndex];
        streamFrontendGlob.modelDistance[modelIndex] = v6;
    }
    else
    {
        streamFrontendGlob.modelDistance[modelIndex] = distSq;
        streamFrontendGlob.modelDistanceBits[modelIndex >> 5] |= 1 << (modelIndex & 0x1F);
    }
}

void __cdecl R_StreamUpdateWorldSurface(int surfId, const float *viewPos, float maxDistSq, float *distanceScale)
{
    float importance; // [esp+8h] [ebp-120h]
    int materialIndex; // [esp+110h] [ebp-18h]
    const GfxSurface *surface; // [esp+11Ch] [ebp-Ch]
    distance_data distSq; // [esp+120h] [ebp-8h] BYREF

    if ((surfId < 0 || surfId >= rgp.world->surfaceCount)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
            3833,
            0,
            "%s\n\t(surfId) = %i",
            "(surfId >= 0 && surfId < rgp.world->surfaceCount)",
            surfId))
    {
        __debugbreak();
    }
    surface = &rgp.world->dpvs.surfaces[surfId];
    if (!rg.renderHiResShot || rgp.world->dpvs.surfaceVisDataCameraSaved[surfId])
    {
        MultiplePointDistSqFromBounds(
            &distSq,
            viewPos,
            surface->tris.mins,
            surface->tris.maxs,
            surface->tris.himipRadiusSq,
            distanceScale[rgp.world->dpvs.surfaceVisDataCameraSaved[surfId] != 0]);
        materialIndex = DB_GetMaterialIndex(surface->material);
        if ((float)(distSq.importance - streamFrontendGlob.materialImportance[materialIndex]) < 0.0)
            importance = streamFrontendGlob.materialImportance[materialIndex];
        else
            importance = distSq.importance;
        streamFrontendGlob.materialImportance[materialIndex] = importance;
    }
}

void __cdecl MultiplePointDistSqFromBounds(
                distance_data *distances,
                const float *v,
                const float *mip0mins,
                const float *mip0maxs,
                float himipRadiusSq,
                float distanceScale)
{
    float4 v6; // [esp-20h] [ebp-120h]
    float4 v7; // [esp-10h] [ebp-110h]

    if ( ((unsigned int)&s_viewPos & 0xF) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
                    3465,
                    0,
                    "%s",
                    "( (uint)(&s_viewPos.v[0]) & 15 ) == 0") )
    {
        __debugbreak();
    }
    if ( ((unsigned __int8)mip0mins & 0xF) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
                    3469,
                    0,
                    "%s",
                    "((uint)((void*)mip0mins) & 15) == 0") )
    {
        __debugbreak();
    }
    if ( ((unsigned __int8)mip0maxs & 0xF) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
                    3470,
                    0,
                    "%s",
                    "((uint)((void*)mip0maxs) & 15) == 0") )
    {
        __debugbreak();
    }
    *(_QWORD *)v7.v = *(_QWORD *)mip0maxs;
    *(_QWORD *)&v7.unitVec[2].packed = *((_QWORD *)mip0maxs + 1);
    *(_QWORD *)v6.v = *(_QWORD *)mip0mins;
    *(_QWORD *)&v6.unitVec[2].packed = *((_QWORD *)mip0mins + 1);
    distances->distanceForHimip = FastPointDistSqFromBounds(v6, v7) * distanceScale;
    distances->importance = himipRadiusSq / (float)(distances->distanceForHimip + 100.0);
}

double __cdecl FastPointDistSqFromBounds(float4 mins, float4 maxs)
{
    float v3; // [esp+4h] [ebp-120h]
    float v4; // [esp+8h] [ebp-11Ch]
    float v5; // [esp+Ch] [ebp-118h]
    float v6; // [esp+14h] [ebp-110h]
    float v7; // [esp+18h] [ebp-10Ch]
    float v8; // [esp+1Ch] [ebp-108h]

    if ( (float)(s_viewPos.v[0] - mins.v[0]) < 0.0 )
        v8 = mins.v[0];
    else
        v8 = s_viewPos.v[0];
    if ( (float)(s_viewPos.v[1] - mins.v[1]) < 0.0 )
        v7 = mins.v[1];
    else
        v7 = s_viewPos.v[1];
    if ( (float)(s_viewPos.v[2] - mins.v[2]) < 0.0 )
        v6 = mins.v[2];
    else
        v6 = s_viewPos.v[2];
    if ( (float)(maxs.v[0] - v8) < 0.0 )
        v5 = maxs.v[0];
    else
        v5 = v8;
    if ( (float)(maxs.v[1] - v7) < 0.0 )
        v4 = maxs.v[1];
    else
        v4 = v7;
    if ( (float)(maxs.v[2] - v6) < 0.0 )
        v3 = maxs.v[2];
    else
        v3 = v6;
    return (float)((float)((float)((float)(v5 - s_viewPos.v[0]) * (float)(v5 - s_viewPos.v[0]))
                                             + (float)((float)(v4 - s_viewPos.v[1]) * (float)(v4 - s_viewPos.v[1])))
                             + (float)((float)(v3 - s_viewPos.v[2]) * (float)(v3 - s_viewPos.v[2])));
}

void __cdecl R_Stream_SortCmd(_BYTE *data)
{
    PROF_SCOPED("R_Stream_SortCmd");
    StreamSortCmd *cmd = (StreamSortCmd *)data;
    R_StreamUpdate_EndQuerySort(cmd->diskOrder);
}

// aislop
struct importance_and_offset_pred
{
    bool operator()(int a, int b) const
    {
        // Compare based on importance stored in streamFrontendGlob
        return streamFrontendGlob.imageImportanceBits[a - 4064] > streamFrontendGlob.imageImportanceBits[b - 4064];
    }
};

void __cdecl R_StreamUpdate_EndQuerySort(bool diskOrder)
{
    int index; // [esp+A0h] [ebp-8h]
    int imagePartIndex; // [esp+A4h] [ebp-4h]

    PROF_SCOPED("R_Stream EndQuerySort");

    index = 0;
    while (index < streamFrontendGlob.totalBytesWanted)
    {
        imagePartIndex = *(int *)((char *)&streamFrontendGlob.sortedImages[index] + 2);
        if ((streamFrontendGlob.imageInSortedListBits[imagePartIndex >> 5]
            & (1 << (BYTE2(streamFrontendGlob.sortedImages[index]) & 0x1F))) == 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
                4178,
                0,
                "%s",
                "streamFrontendGlob.imageInSortedListBits[ BIT_INDEX_32( imagePartIndex ) ] & BIT_MASK_32( imagePartIndex )"))
        {
            __debugbreak();
        }
        if ((streamFrontendGlob.dynamicImageImportanceBits[(imagePartIndex >> 5) - 4064] & (1 << (imagePartIndex & 0x1F))) != 0)
        {
            ++index;
        }
        else
        {
            *(int *)((char *)&streamFrontendGlob.sortedImages[index] + 2) = *(int *)((char *)&streamFrontendGlob.sortedImages[--streamFrontendGlob.totalBytesWanted]
                + 2);
            streamFrontendGlob.imageInSortedListBits[imagePartIndex >> 5] &= ~(1 << (imagePartIndex & 0x1F));
        }
    }
    if (diskOrder)
    {
        if (streamFrontendGlob.diskOrderImagesNeedSorting || streamFrontendGlob.forceDiskOrder)
        {
            //std::_Sort<int *, int, importance_and_offset_pred>(
            //    (int *)((char *)streamFrontendGlob.sortedImages + 2),
            //    (int *)((char *)&streamFrontendGlob.sortedImages[streamFrontendGlob.totalBytesWanted] + 2),
            //    (4 * streamFrontendGlob.totalBytesWanted) >> 2,
            //    0);

            std::sort((int *)((char *)streamFrontendGlob.sortedImages + 2), (int *)((char *)&streamFrontendGlob.sortedImages[streamFrontendGlob.totalBytesWanted] + 2), importance_and_offset_pred{});

            streamFrontendGlob.diskOrderImagesNeedSorting = 0;
        }
    }
    else
    {
        importance_merge_sort((void **)((char *)streamFrontendGlob.sortedImages + 2), streamFrontendGlob.totalBytesWanted);
    }
}

void __cdecl R_Stream_CombineCmd()
{
    PROF_SCOPED("R_Stream_CombineCmd");
    R_StreamUpdate_CombineImportance();
}

void R_StreamUpdate_CombineImportance()
{
    float distSq; // [esp+4h] [ebp-104h]
    XAssetPoolEntry<Material> *material; // [esp+E8h] [ebp-20h]
    XAssetPoolEntry<XModel> *model; // [esp+ECh] [ebp-1Ch]
    XAssetPoolEntry<XModel> *modela; // [esp+ECh] [ebp-1Ch]
    XAssetPoolEntry<XModel> *modelb; // [esp+ECh] [ebp-1Ch]
    unsigned int materialIndex; // [esp+F0h] [ebp-18h]
    unsigned int staticBit; // [esp+F4h] [ebp-14h]
    unsigned int dynamicBit; // [esp+F8h] [ebp-10h]
    unsigned int mask; // [esp+100h] [ebp-8h]
    unsigned int modelIndex; // [esp+104h] [ebp-4h]

    {
        PROF_SCOPED("R_Stream combine xmodels");

        for (modelIndex = 0; modelIndex < 1000; ++modelIndex)
        {
            mask = 1 << (modelIndex & 0x1F);
            staticBit = mask & streamFrontendGlob.modelDistanceBits[modelIndex >> 5];
            dynamicBit = mask & streamFrontendGlob.dynamicModelDistanceBits[modelIndex >> 5];
            if ((dynamicBit & staticBit) != 0)
            {
                model = DB_GetXModelAtIndex(modelIndex);
                if (!model
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp", 3911, 0, "%s", "model"))
                {
                    __debugbreak();
                }
                if ((float)(streamFrontendGlob.dynamicModelDistance[modelIndex] - streamFrontendGlob.modelDistance[modelIndex]) < 0.0)
                    distSq = streamFrontendGlob.dynamicModelDistance[modelIndex];
                else
                    distSq = streamFrontendGlob.modelDistance[modelIndex];
                R_StreamUpdateForXModel(&model->entry, distSq);
            }
            else if (staticBit)
            {
                modela = DB_GetXModelAtIndex(modelIndex);
                if (!modela
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp", 3917, 0, "%s", "model"))
                {
                    __debugbreak();
                }
                R_StreamUpdateForXModel(&modela->entry, streamFrontendGlob.modelDistance[modelIndex]);
            }
            else if (dynamicBit)
            {
                modelb = DB_GetXModelAtIndex(modelIndex);
                if (!modelb
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp", 3923, 0, "%s", "model"))
                {
                    __debugbreak();
                }
                R_StreamUpdateForXModel(&modelb->entry, streamFrontendGlob.dynamicModelDistance[modelIndex]);
            }
        }
    }

    {
        PROF_SCOPED("R_Stream combine materials");

        for (materialIndex = 0; materialIndex < 0x1000; ++materialIndex)
        {
            if (streamFrontendGlob.materialImportance[materialIndex] != 0.0
                && (streamFrontendGlob.materialPreventBits[materialIndex >> 5] & (1 << (materialIndex & 0x1F))) == 0)
            {
                material = DB_GetMaterialAtIndex(materialIndex);
                if (!material
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp", 3937, 0, "%s", "material"))
                {
                    __debugbreak();
                }
                R_StreamTouchImagesFromMaterial(&material->entry, streamFrontendGlob.materialImportance[materialIndex]);
            }
        }
    }
}

void __cdecl R_StreamUpdateForXModel(const XModel *remoteModel, float distSq)
{
    float v2; // [esp+0h] [ebp-40h]
    float v3; // [esp+4h] [ebp-3Ch]
    float v4; // [esp+10h] [ebp-30h]
    int materialIndex; // [esp+1Ch] [ebp-24h]
    Material **materialHandles; // [esp+20h] [ebp-20h]
    XModelHighMipBounds *highMipBounds; // [esp+24h] [ebp-1Ch]
    XModelHighMipBounds *bounds; // [esp+28h] [ebp-18h]
    float importance; // [esp+2Ch] [ebp-14h]
    int surf; // [esp+34h] [ebp-Ch]
    float distNotSq; // [esp+38h] [ebp-8h]

    distNotSq = sqrtf(distSq);
    materialHandles = remoteModel->materialHandles;
    highMipBounds = remoteModel->streamInfo.highMipBounds;
    for (surf = 0; surf < remoteModel->numsurfs; ++surf)
    {
        if (materialHandles[surf]->textureCount)
        {
            bounds = &highMipBounds[surf];
            if (bounds->himipRadiusSq != 0.0)
            {
                v4 = distNotSq - Abs(bounds->center);
                if ((float)((float)(v4 * v4) - 0.0) < 0.0)
                    v3 = 0.0f;
                else
                    v3 = v4 * v4;
                importance = bounds->himipRadiusSq / (float)(v3 + 100.0);
                materialIndex = DB_GetMaterialIndex(materialHandles[surf]);
                if (materialIndex < 0
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
                        3531,
                        0,
                        "%s",
                        "materialIndex >= 0"))
                {
                    __debugbreak();
                }
                if (materialIndex >= 4096
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
                        3532,
                        0,
                        "%s",
                        "materialIndex < MAX_MATERIAL_POOL_SIZE"))
                {
                    __debugbreak();
                }
                if ((float)(importance - streamFrontendGlob.materialImportance[materialIndex]) < 0.0)
                    v2 = streamFrontendGlob.materialImportance[materialIndex];
                else
                    v2 = importance;
                streamFrontendGlob.materialImportance[materialIndex] = v2;
            }
        }
    }
}

int __cdecl r_stream_update_staticmodelsCallback(jqBatch *batch)
{
    if ( batch->Module->Group.ExecutingBatchCount > 1 )
        return 1;

    char *data = (char *)jqLockData(batch);
    R_Stream_UpdateStaticModelsCmd(data);
    jqUnlockData(batch);
    return 0;
}

int __cdecl r_stream_update_staticsurfacesCallback(jqBatch *batch)
{
    if ( batch->Module->Group.ExecutingBatchCount > 1 )
        return 1;

    char *data = (char *)jqLockData(batch);
    R_Stream_UpdateStaticSurfacesCmd(data);
    jqUnlockData(batch);
    return 0;
}

int __cdecl r_stream_sortCallback(jqBatch *batch)
{
    if ( jqPoll(&r_stream_combineModule.Group) || jqPoll(&r_stream_updateModule.Group) )
        return 1;

    unsigned char *data = (unsigned char *)jqLockData(batch);
    R_Stream_SortCmd(data);
    jqUnlockData(batch);
    return 0;
}

int __cdecl r_stream_combineCallback(jqBatch *batch)
{
    if ( jqPoll(&r_stream_updateModule.Group)
        || jqPoll(&r_stream_update_staticmodelsModule.Group)
        || jqPoll(&r_stream_update_staticsurfacesModule.Group) )
    {
        return 1;
    }
    jqLockData(batch);
    R_Stream_CombineCmd();
    jqUnlockData(batch);
    return 0;
}

int __cdecl r_stream_updateCallback(jqBatch *batch)
{
    StreamUpdateCmd *cmd; // [esp+Ch] [ebp-4h]

    if (batch->Module->Group.ExecutingBatchCount > 1)
        return 1;
    cmd = (StreamUpdateCmd *)jqLockData(batch);
    s_viewPos = *(float4 *)cmd->viewPos;
    R_StreamUpdateStatic(cmd->viewPos, cmd->maxDistSq, cmd->distanceScale);
    jqUnlockData(batch);
    return 0;
}

void __cdecl R_StreamUpdateAabbNode_r_0_(int aabbTreeNode, const float *viewPos, float maxDistSq, float *distanceScale)
{
  int ref; // [esp+4Ch] [ebp-24h]
  int childIter; // [esp+50h] [ebp-20h]
  GfxStreamingAabbTree *tree; // [esp+54h] [ebp-1Ch]
  int childEnd; // [esp+58h] [ebp-18h]
  int *leafRefEnd; // [esp+60h] [ebp-10h]
  int *leafRefIter; // [esp+68h] [ebp-8h]
  float distSq; // [esp+6Ch] [ebp-4h]

  if ( (aabbTreeNode < 0 || aabbTreeNode >= rgp.world->streamInfo.aabbTreeCount)
    && !Assert_MyHandler(
          "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream.cpp",
          3963,
          0,
          "%s\n\t(aabbTreeNode) = %i",
          "(aabbTreeNode >= 0 && aabbTreeNode < rgp.world->streamInfo.aabbTreeCount)",
          aabbTreeNode) )
  {
    __debugbreak();
  }
  tree = &rgp.world->streamInfo.aabbTrees[aabbTreeNode];
  distSq = PointDistSqFromBounds(viewPos, tree->mins, tree->maxs);
  if ( distSq <= maxDistSq )
  {
    if ( tree->childCount )
    {
      childEnd = tree->childCount + tree->firstChild;
      for ( childIter = tree->firstChild; childIter != childEnd; ++childIter )
        R_StreamUpdateAabbNode_r_0_(childIter, viewPos, maxDistSq, distanceScale);
    }
    else
    {
      leafRefEnd = &rgp.world->streamInfo.leafRefs[tree->firstItem + tree->itemCount];
      for ( leafRefIter = &rgp.world->streamInfo.leafRefs[tree->firstItem]; leafRefIter != leafRefEnd; ++leafRefIter )
      {
        ref = *leafRefIter;
        if ( *leafRefIter < 0 )
          R_StreamUpdateStaticModel(~ref, viewPos, maxDistSq, distanceScale);
        else
          R_StreamUpdateWorldSurface(ref, viewPos, maxDistSq, distanceScale);
      }
    }
  }
}

void __cdecl R_StreamUpdateStatic(const float *viewPos, float maxDistSq, float *distanceScale)
{
    iassert(viewPos);

    PROF_SCOPED("R_Stream update static");

    s_viewPos = *(float4 *)viewPos;

    if (rgp.world->streamInfo.aabbTreeCount > 0)
    {
        R_StreamUpdateAabbNode_r_0_(0, viewPos, maxDistSq, distanceScale);
    }
}

