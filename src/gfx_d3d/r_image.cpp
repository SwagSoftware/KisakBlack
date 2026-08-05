#include "r_image.h"
#include <universal/com_memory.h>
#include <xanim/xmodel.h>
#include <qcommon/threads.h>
#include "rb_resource.h"
#include "r_dvars.h"
#include "rb_logfile.h"
#include "r_image_load_common.h"
#include "r_image_load_obj.h"
#include <database/db_file_load.h>
#include <qcommon/com_profilemapload.h>
#include "r_texturemem.h"
#include "r_init.h"

#include <algorithm>
#include "rb_state.h"
#include <qcommon/common.h>

const char *g_imageProgNames[29] =
{
  "$shadowmap_sun",
  "$shadowmap_spot",
  "$floatz",
  "$post_effect_src",
  "$post_effect_godrays",
  "$post_effect_0",
  "$post_effect_1",
  "$pingpong_0",
  "$pingpong_1",
  "$resolved_scene",
  "$savedscreen",
  "$raw",
  "$model_lighting",
  "$8bit_swapchain",
  "$16bit_system",
  "$8bit_system",
  "$seethru_decal",
  "$lut",
  "$ui3d",
  "$ui3d_ping_pong",
  "$missilecam",
  "$floatz_missilecam",
  "$composite",
  "$bloom_mip1",
  "$bloom_mip2",
  "$bloom_mip3",
  "$bloom_mip3_ping",
  "$bloom_mip3_pong",
  "$bloom_streak1"
};


//$240FF632105687D15E57E43F247C9BAA imageGlobals;
struct //$240FF632105687D15E57E43F247C9BAA // sizeof=0x2014
{                                       // XREF: .data:imageGlobals/r
    GfxImage *imageHashTable[2048];     // XREF: Image_AllocProg(int,uchar,uchar)+AE/w
    int picmip;                         // XREF: Image_PicmipForSemantic(uchar,Picmip *):$LN6_197/r
    int picmipBump;                     // XREF: Image_PicmipForSemantic(uchar,Picmip *):$LN5_202/r
    int picmipSpec;                     // XREF: Image_PicmipForSemantic(uchar,Picmip *):$LN4_244/r
    CardMemory totalMemory;             // XREF: Image_TrackTotalMemory(GfxImage *,int,int)+2A/r
} imageGlobals;

int Image_TrackBytes;
GfxImage g_imageProgs[29];

void __cdecl Image_TrackTotalMemory(GfxImage *image, int platform, int memory)
{
    if ( !Image_IsCodeImage(image->track) )
        imageGlobals.totalMemory.platform[platform] += memory - image->cardMemory.platform[platform];
}

bool __cdecl Image_IsCodeImage(int track)
{
    return track >= 0 && (track <= 1 || track == 4);
}

void __cdecl R_GetImageList(ImageList *imageList)
{
    if ( !imageList
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 232, 0, "%s", "imageList") )
    {
        __debugbreak();
    }
    imageList->count = 0;
    DB_EnumXAssets(ASSET_TYPE_IMAGE, (void (__cdecl *)(XAssetHeader, void *))R_AddImageToList, imageList, 1);
}

void __cdecl R_AddImageToList(XAssetHeader header, XAssetHeader *data)
{
    if ( data->xmodelPieces >= (XModelPieces *)0x1080
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    225,
                    0,
                    "%s",
                    "imageList->count < ARRAY_COUNT( imageList->image )") )
    {
        __debugbreak();
    }
    data[(int)data->xmodelPieces++ + 1] = header;
}

void __cdecl R_SumOfUsedImages(Image_MemUsage *usage)
{
    const char *v1; // eax
    GfxImage *v2; // [esp+0h] [ebp-4240h]
    unsigned int v3[4]; // [esp+4h] [ebp-423Ch] BYREF
    int v4; // [esp+14h] [ebp-422Ch]
    int v5; // [esp+18h] [ebp-4228h]
    int v6; // [esp+1Ch] [ebp-4224h]
    int v7; // [esp+20h] [ebp-4220h]
    int v8; // [esp+24h] [ebp-421Ch]
    int v9; // [esp+28h] [ebp-4218h]
    int v10; // [esp+2Ch] [ebp-4214h]
    unsigned int i; // [esp+30h] [ebp-4210h]
    int v12; // [esp+34h] [ebp-420Ch]
    ImageList imageList; // [esp+38h] [ebp-4208h] BYREF

    if ( !usage && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 248, 0, "%s", "usage") )
        __debugbreak();
    R_GetImageList(&imageList);
    memset(v3, 0, sizeof(v3));
    v4 = 0;
    v5 = 0;
    v6 = 0;
    v7 = 0;
    v8 = 0;
    v9 = 0;
    v12 = 0;
    for ( i = 0; i < imageList.count; ++i )
    {
        v2 = imageList.image[i];
        if ( !v2 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 260, 0, "%s", "image") )
            __debugbreak();
        v10 = v2->cardMemory.platform[0];
        v3[v2->track] += v10;
        if ( !Image_IsCodeImage(v2->track) )
            v12 += v10;
    }
    usage->total = v12;
    usage->lightmap = v4;
    if ( usage->total != imageGlobals.totalMemory.platform[0] )
    {
        v1 = va("%i != %i", usage->total, imageGlobals.totalMemory.platform[0]);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                        281,
                        0,
                        "%s\n\t%s",
                        "usage->total == imageGlobals.totalMemory.platform[PICMIP_PLATFORM_USED]",
                        v1) )
            __debugbreak();
    }
    usage->minspec = imageGlobals.totalMemory.platform[1];
}

void __cdecl Image_Release(GfxImage *image)
{
    int platform; // [esp+0h] [ebp-4h]

    if ( !image && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 314, 0, "%s", "image") )
        __debugbreak();
    if ( !Image_IsCodeImage(image->track) )
    {
        for ( platform = 0; platform < 2; ++platform )
            imageGlobals.totalMemory.platform[platform] -= image->cardMemory.platform[platform];
    }
    if ( image->texture.basemap )
    {
        if ( Sys_IsRenderThread() )
            image->texture.basemap->Release();
        else
            RB_Resource_Release(image->texture.basemap);
        image->texture.basemap = 0;
        image->cardMemory.platform[0] = 0;
        image->cardMemory.platform[1] = 0;
    }
    else if ( r_loadForRenderer->current.enabled
                 && image->cardMemory.platform[0]
                 && !Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                             359,
                             0,
                             "%s",
                             "!image->cardMemory.platform[PICMIP_PLATFORM_USED]") )
    {
        __debugbreak();
    }
    Image_TrackBytes -= image->loadedSize;
    image->loadedSize = 0;
    image->mapType = 0;
}

GfxImage *__cdecl Image_AllocProg(int imageProgType, unsigned __int8 category, unsigned __int8 semantic)
{
    GfxImage *image; // [esp+0h] [ebp-Ch]
    const char *name; // [esp+4h] [ebp-8h]

    image = &g_imageProgs[imageProgType];
    if ( !image && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 445, 1, "%s", "image") )
        __debugbreak();
    name = g_imageProgNames[imageProgType];
    image->name = name;
    if ( !category
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    450,
                    0,
                    "%s",
                    "category != IMG_CATEGORY_UNKNOWN") )
    {
        __debugbreak();
    }
    image->category = category;
    image->semantic = semantic;
    image->track = 0;
    imageGlobals.imageHashTable[Image_GetAvailableHashLocation(name)] = image;
    return &g_imageProgs[imageProgType];
}

unsigned int __cdecl Image_GetAvailableHashLocation(const char *name)
{
    unsigned int stuck; // [esp+4h] [ebp-8h]
    unsigned int hashIndex; // [esp+8h] [ebp-4h]

    hashIndex = R_HashAssetName(name) & 0x7FF;
    for ( stuck = 0; stuck <= 0x800; ++stuck )
    {
        if ( !imageGlobals.imageHashTable[hashIndex] )
            break;
        hashIndex = ((_WORD)hashIndex + 1) & 0x7FF;
    }
    return hashIndex;
}

GfxImage *__cdecl Image_Alloc(
                char *name,
                unsigned __int8 category,
                unsigned __int8 semantic,
                unsigned __int8 imageTrack)
{
    unsigned int v5; // [esp+0h] [ebp-20h]
    GfxImage *image; // [esp+10h] [ebp-10h]

    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 512, 0, "%s", "name") )
        __debugbreak();
    v5 = strlen(name);
    image = (GfxImage *)Hunk_Alloc(v5 + 53, "Image_Alloc", 23);
    if ( !image && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 521, 1, "%s", "image") )
        __debugbreak();
    image->name = (const char *)&image[1];
    Image_Construct(name, v5 + 1, category, semantic, imageTrack, image);
    imageGlobals.imageHashTable[Image_GetAvailableHashLocation(name)] = image;
    return image;
}

void __cdecl Image_Construct(
                char *name,
                int nameSize,
                unsigned __int8 category,
                unsigned __int8 semantic,
                unsigned __int8 imageTrack,
                GfxImage *image)
{
    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 407, 0, "%s", "name") )
        __debugbreak();
    if ( nameSize <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    408,
                    0,
                    "%s\n\t(nameSize) = %i",
                    "(nameSize > 0)",
                    nameSize) )
    {
        __debugbreak();
    }
    if ( !image && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 409, 0, "%s", "image") )
        __debugbreak();
    memcpy((unsigned __int8 *)image->name, (unsigned __int8 *)name, nameSize);
    if ( !category
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    415,
                    0,
                    "%s",
                    "category != IMG_CATEGORY_UNKNOWN") )
    {
        __debugbreak();
    }
    image->category = category;
    image->semantic = semantic;
    if ( image->noPicmip
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    419,
                    1,
                    "%s",
                    "image->noPicmip == false") )
    {
        __debugbreak();
    }
    if ( image->picmip.platform[0]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    420,
                    1,
                    "%s",
                    "image->picmip.platform[PICMIP_PLATFORM_USED] == 0") )
    {
        __debugbreak();
    }
    if ( image->picmip.platform[1]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    422,
                    1,
                    "%s",
                    "image->picmip.platform[PICMIP_PLATFORM_MINSPEC] == 0") )
    {
        __debugbreak();
    }
    image->track = imageTrack;
}

void __cdecl R_ShutdownImages()
{
    GfxImage *image; // [esp+0h] [ebp-2014h]
    GfxImage *imagea; // [esp+0h] [ebp-2014h]
    int v2; // [esp+4h] [ebp-2010h]
    unsigned int i; // [esp+8h] [ebp-200Ch]
    unsigned int v4[2049]; // [esp+Ch] [ebp-2008h]
    int j; // [esp+2010h] [ebp-4h]

    RB_UnbindAllImages();
    v2 = 0;
    for ( i = 0; i < 0x800; ++i )
    {
        image = imageGlobals.imageHashTable[i];
        if ( image )
        {
            if ( Image_IsProg(image) )
                v4[v2++] = (unsigned int)image;
            else
                Image_Free(imageGlobals.imageHashTable[i]);
        }
    }
    memset(&imageGlobals, 0, 0x2000u);
    for ( j = 0; j < v2; ++j )
    {
        imagea = (GfxImage *)v4[j];
        imageGlobals.imageHashTable[Image_GetAvailableHashLocation(imagea->name)] = imagea;
    }
}

bool __cdecl Image_IsProg(GfxImage *image)
{
    return image >= g_imageProgs && image < &g_imageProgs[30];
}

void __cdecl Image_Free(GfxImage *image)
{
    Image_Release(image);
}

void __cdecl Image_Create2DTexture_PC(
                GfxImage *image,
                unsigned __int16 width,
                unsigned __int16 height,
                unsigned int mipmapCount,
                int imageFlags,
                _D3DFORMAT imageFormat)
{
    const char *v6; // eax
    int hr; // [esp+4h] [ebp-10h]
    unsigned int usage; // [esp+8h] [ebp-Ch]
    _D3DPOOL memPool; // [esp+10h] [ebp-4h]

    if ( !image && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 646, 0, "%s", "image") )
        __debugbreak();
    if ( image->texture.basemap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    647,
                    0,
                    "%s",
                    "!image->texture.basemap") )
    {
        __debugbreak();
    }
    image->width = width;
    image->height = height;
    image->depth = 1;
    image->mapType = 3;
    usage = Image_GetUsage(imageFlags, imageFormat);
    if ( (imageFlags & 0x40000) != 0 )
        memPool = D3DPOOL_SYSTEMMEM;
    else
        memPool = (_D3DPOOL)(usage == 0);
    if ( (imageFlags & 0x100) != 0 )
        memPool = D3DPOOL_SYSTEMMEM;
    if ( !Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    675,
                    0,
                    "%s",
                    "Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("dx.device->CreateTexture( width, height, mipmapCount, usage, imageFormat, memPool, &image->texture.map, 0 )\n");
    hr = dx.device->CreateTexture(
                 width,
                 height,
                 mipmapCount,
                 usage,
                 imageFormat,
                 memPool,
                 (IDirect3DTexture9 **)image,
                 0);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v6 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp (%i) dx.device->CreateTexture( width, height, mipmapCount,"
            " usage, imageFormat, memPool, &image->texture.map, 0 ) failed: %s\n",
            679,
            v6);
    }
}

unsigned int __cdecl Image_GetUsage(int imageFlags, _D3DFORMAT imageFormat)
{
    if ( (imageFlags & 0x20000) != 0 )
    {
        if ( imageFormat == D3DFMT_D24S8 || imageFormat == D3DFMT_D24X8 || imageFormat == D3DFMT_D16 )
            return 2;
        else
            return 1;
    }
    else if ( (imageFlags & 0x10000) != 0 )
    {
        return 512;
    }
    else
    {
        return 0;
    }
}

void __cdecl Image_Create3DTexture_PC(
                GfxImage *image,
                unsigned __int16 width,
                unsigned __int16 height,
                unsigned __int16 depth,
                unsigned int mipmapCount,
                int imageFlags,
                _D3DFORMAT imageFormat)
{
    const char *v7; // eax
    int hr; // [esp+4h] [ebp-10h]

    if ( !image && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 701, 0, "%s", "image") )
        __debugbreak();
    if ( image->texture.basemap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    702,
                    0,
                    "%s",
                    "!image->texture.basemap") )
    {
        __debugbreak();
    }
    image->width = width;
    image->height = height;
    image->depth = depth;
    image->mapType = 4;
    Image_GetUsage(imageFlags, imageFormat);
    if ( !Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    717,
                    0,
                    "%s",
                    "Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint(
            "dx.device->CreateVolumeTexture( width, height, depth, mipmapCount, 0, imageFormat, D3DPOOL_MANAGED, &image->texture.volmap, 0 )\n");
    hr = dx.device->CreateVolumeTexture(
                 width,
                 height,
                 depth,
                 mipmapCount,
                 0,
                 imageFormat,
                 D3DPOOL_MANAGED,
                 (IDirect3DVolumeTexture9 **)image,
                 0);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v7 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp (%i) dx.device->CreateVolumeTexture( width, height, depth,"
            " mipmapCount, 0, imageFormat, D3DPOOL_MANAGED, &image->texture.volmap, 0 ) failed: %s\n",
            721,
            v7);
    }
}

void __cdecl Image_CreateCubeTexture_PC(
                GfxImage *image,
                unsigned __int16 edgeLen,
                unsigned int mipmapCount,
                _D3DFORMAT imageFormat)
{
    const char *v4; // eax
    int hr; // [esp+4h] [ebp-8h]

    if ( !image && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 741, 0, "%s", "image") )
        __debugbreak();
    if ( image->texture.basemap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    742,
                    0,
                    "%s",
                    "!image->texture.basemap") )
    {
        __debugbreak();
    }
    image->width = edgeLen;
    image->height = edgeLen;
    image->depth = 1;
    image->mapType = 5;
    if ( !gfxMetrics.canMipCubemaps )
        mipmapCount = 1;
    if ( !Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    754,
                    0,
                    "%s",
                    "Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint(
            "dx.device->CreateCubeTexture( edgeLen, mipmapCount, 0, imageFormat, D3DPOOL_MANAGED, &image->texture.cubemap, 0 )\n");
    hr = dx.device->CreateCubeTexture(
                 edgeLen,
                 mipmapCount,
                 0,
                 imageFormat,
                 D3DPOOL_MANAGED,
                 (IDirect3DCubeTexture9 **)image,
                 0);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v4 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp (%i) dx.device->CreateCubeTexture( edgeLen, mipmapCount, 0"
            ", imageFormat, D3DPOOL_MANAGED, &image->texture.cubemap, 0 ) failed: %s\n",
            758,
            v4);
    }
}

void __cdecl Image_SetupRenderTarget(
                GfxImage *image,
                unsigned __int16 width,
                unsigned __int16 height,
                _D3DFORMAT imageFormat)
{
    if ( !image && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 769, 0, "%s", "image") )
        __debugbreak();
    if ( image->semantic
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    770,
                    0,
                    "%s",
                    "image->semantic == TS_2D") )
    {
        __debugbreak();
    }
    if ( !_stricmp(image->name, "$16bit_system") || !_stricmp(image->name, "$8bit_system") )
        Image_SetupAndLoad(image, width, height, 1, 259, imageFormat);
    else
        Image_SetupAndLoad(image, width, height, 1, 131075, imageFormat);
}

void __cdecl Image_PicmipForSemantic(unsigned __int8 semantic, Picmip *picmip)
{
    const char *v2; // eax
    int picmipUsed; // [esp+4h] [ebp-4h]

    switch ( semantic )
    {
        case 0u:
        case 1u:
            goto $LN7_167;
        case 2u:
        case 0xBu:
        case 0xCu:
        case 0xDu:
        case 0xEu:
        case 0xFu:
        case 0x10u:
        case 0x11u:
        case 0x12u:
        case 0x13u:
        case 0x14u:
        case 0x15u:
        case 0x16u:
        case 0x17u:
        case 0x18u:
        case 0x19u:
        case 0x1Au:
        case 0x1Bu:
        case 0x1Cu:
            picmipUsed = imageGlobals.picmip;
            goto LABEL_8;
        case 5u:
            picmipUsed = imageGlobals.picmipBump;
            goto LABEL_8;
        case 8u:
            picmipUsed = imageGlobals.picmipSpec;
LABEL_8:
            picmip->platform[1] = 2;
            if ( picmipUsed >= 0 )
            {
                if (picmipUsed > 3)
                {
                    //LOBYTE(picmipUsed) = 3;
                    (picmipUsed) = 3;
                }
            }
            else
            {
                //LOBYTE(picmipUsed) = 0;
                (picmipUsed) = 0;
            }
            picmip->platform[0] = picmipUsed;
            break;
        default:
            v2 = va("unhandled case: %d", semantic);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 798, 1, v2) )
                __debugbreak();
$LN7_167:
            //*picmip = 0;
            // mov [ecx], ax
            picmip->platform[0] = 0;
            picmip->platform[1] = 0;
            break;
    }
}

void __cdecl R_DelayLoadImage(XAssetHeader header, void *data)
{
    const char *externalDataSize; // [esp+4h] [ebp-4h]

    GfxImage *image = header.image;


    if (image->delayLoadPixels)
    {
        image->delayLoadPixels = 0;
        externalDataSize = (const char *)image->cardMemory.platform[0];
        image->cardMemory.platform[0] = 0;
        image->cardMemory.platform[1] = 0;
        if (r_loadForRenderer->current.enabled && !Image_LoadFromFile(image, 0))
            Image_AssignDefaultTexture(image);
        DB_LoadedExternalData((unsigned int)externalDataSize);
    }
}

char __cdecl Image_AssignDefaultTexture(GfxImage *image)
{
    if ( image->mapType != 3 )
        return 0;
    if ( image->semantic == 5 )
        return R_DuplicateTexture(image, rgp.identityNormalMapImage);
    if ( image->semantic == 8 )
        return R_DuplicateTexture(image, rgp.blackImage);
    return R_DuplicateTexture(image, rgp.whiteImage);
}

char __cdecl R_DuplicateTexture(GfxImage *dstImage, const GfxImage *srcImage)
{
    if ( !srcImage || !srcImage->texture.basemap )
        return 0;
    dstImage->texture.basemap = srcImage->texture.basemap;
    dstImage->texture.basemap->AddRef();
    return 1;
}

void __cdecl Load_Texture(GfxTexture *remoteLoadDef, GfxImage *image)
{
    unsigned int v2; // [esp+0h] [ebp-68h]
    unsigned int v3; // [esp+4h] [ebp-64h]
    unsigned int v4; // [esp+8h] [ebp-60h]
    unsigned __int8 mapType; // [esp+10h] [ebp-58h]
    unsigned __int16 v6; // [esp+14h] [ebp-54h]
    int v7; // [esp+18h] [ebp-50h]
    GfxImageLoadDef *loadDef; // [esp+34h] [ebp-34h]
    int externalDataSize; // [esp+38h] [ebp-30h]
    signed int mipCount; // [esp+3Ch] [ebp-2Ch]
    const unsigned __int8 *data; // [esp+40h] [ebp-28h]
    unsigned __int8 *imageBuffer; // [esp+48h] [ebp-20h]
    int faceCount; // [esp+58h] [ebp-10h]
    signed int faceIndex; // [esp+5Ch] [ebp-Ch]
    _D3DFORMAT imageFormat; // [esp+60h] [ebp-8h]
    int mipLevel; // [esp+64h] [ebp-4h]

    loadDef = (GfxImageLoadDef *)remoteLoadDef->basemap;
    if ( remoteLoadDef->basemap != image->texture.basemap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    945,
                    0,
                    "%s",
                    "loadDef == image->texture.loadDef") )
    {
        __debugbreak();
    }
    image->texture.basemap = 0;
    if ( r_loadForRenderer && r_loadForRenderer->current.enabled )
    {
        imageFormat = (_D3DFORMAT)loadDef->format;
        if ( loadDef->resourceSize )
        {
            image->delayLoadPixels = 0;
            mapType = image->mapType;
            if ( mapType == 3 )
            {
                faceCount = 1;
            }
            else if ( mapType == 4 )
            {
                faceCount = 1;
            }
            else
            {
                if ( image->mapType != 5
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                                996,
                                0,
                                "%s\n\t(image->mapType) = %i",
                                "(image->mapType == MAPTYPE_CUBE)",
                                image->mapType) )
                {
                    __debugbreak();
                }
                faceCount = 6;
            }
            data = loadDef->data;
            mipCount = Image_CountMipmaps(loadDef->flags, image->width, image->height, image->depth);
            for ( faceIndex = 0; faceIndex < faceCount; ++faceIndex )
            {
                if ( faceCount != 1 )
                    Image_CubemapFace(faceIndex);
                for ( mipLevel = 0; mipLevel < mipCount; ++mipLevel )
                {
                    if ( image->width >> mipLevel > 1 )
                        v4 = image->width >> mipLevel;
                    else
                        v4 = 1;
                    if ( image->height >> mipLevel > 1 )
                        v3 = image->height >> mipLevel;
                    else
                        v3 = 1;
                    if ( image->depth >> mipLevel > 1 )
                        v2 = image->depth >> mipLevel;
                    else
                        v2 = 1;
                    data += Image_GetCardMemoryAmountForMipLevel(imageFormat, v4, v3, v2);
                }
            }
            imageBuffer = (unsigned __int8 *)Z_VirtualAlloc(data - loadDef->data, "Image_AllocTempMemory", 20);
            memcpy(imageBuffer, loadDef->data, data - loadDef->data);
            RB_Resource_CreateTexture(image, imageBuffer, mipCount, 0, imageFormat);
            if ( data != &loadDef->data[loadDef->resourceSize]
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                            1059,
                            0,
                            "data == &loadDef->data[loadDef->resourceSize]\n\t%i, %i",
                            data,
                            &loadDef->data[loadDef->resourceSize]) )
            {
                __debugbreak();
            }
        }
        else if ( image->category == 5 )
        {
            image->delayLoadPixels = 0;
            if ((int)image->width >> r_picmip_water->current.integer < 4)
            {
                //LOWORD(v7) = 4;
                (v7) = 4;
            }
            else
                v7 = (int)image->width >> r_picmip_water->current.integer;
            if ( (int)image->height >> r_picmip_water->current.integer < 4 )
                v6 = 4;
            else
                v6 = (int)image->height >> r_picmip_water->current.integer;
            image->cardMemory.platform[0] = 0;
            image->cardMemory.platform[1] = 0;
            Image_Create2DTexture_PC(image, v7, v6, loadDef->levelCount, 0x10000, imageFormat);
        }
        else
        {
            if ( image->cardMemory.platform[0] != Image_GetCardMemoryAmount(
                                                                                            loadDef->flags,
                                                                                            (_D3DFORMAT)loadDef->format,
                                                                                            image->width,
                                                                                            image->height,
                                                                                            image->depth)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                            965,
                            1,
                            "%s\n\t(image->name) = %s",
                            "(static_cast< uint >( image->cardMemory.platform[PICMIP_PLATFORM_USED] ) == Image_GetCardMemoryAmount( loa"
                            "dDef->flags, static_cast< GfxPixelFormat >( loadDef->format ), image->width, image->height, image->depth ))",
                            image->name) )
            {
                __debugbreak();
            }
            if ( image->texture.basemap
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                            966,
                            1,
                            "%s\n\t(image->name) = %s",
                            "(image->texture.basemap == 0)",
                            image->name) )
            {
                __debugbreak();
            }
            image->delayLoadPixels = 0;
            if ( !image->delayLoadPixels )
            {
                externalDataSize = image->cardMemory.platform[0];
                image->cardMemory.platform[0] = 0;
                image->cardMemory.platform[1] = 0;
                if ( !Image_LoadFromFile(image, 0) )
                    Com_Error(ERR_DROP, "Couldn't load image '%s'\n", image->name);
                DB_LoadedExternalData(externalDataSize);
            }
        }
    }
}

GfxImage *__cdecl Image_FindExisting(const char *name)
{
    if ( useFastFile->current.enabled )
        return Image_FindExisting_FastFile(name);
    else
        return Image_FindExisting_LoadObj(name);
}

GfxImage *__cdecl Image_FindExisting_LoadObj(const char *name)
{
    GfxImage *image; // [esp+14h] [ebp-118h]
    unsigned int stuck; // [esp+1Ch] [ebp-110h]
    unsigned int hashIndex; // [esp+20h] [ebp-10Ch]
    char error_string[260]; // [esp+24h] [ebp-108h] BYREF

    hashIndex = R_HashAssetName(name) & 0x7FF;
    image = imageGlobals.imageHashTable[hashIndex];
    stuck = 0;
    while ( image && strcmp(name, image->name) )
    {
        hashIndex = ((_WORD)hashIndex + 1) & 0x7FF;
        image = imageGlobals.imageHashTable[hashIndex];
        if ( ++stuck > 0x800 )
        {
            sprintf(error_string, "stuck %s name %x\n", name, hashIndex);
            Com_Printf(16, "%s", error_string);
            return !Image_IsProg(image) ? image : 0;
        }
    }
    return !Image_IsProg(image) ? image : 0;
}

GfxImage *__cdecl Image_FindExisting_FastFile(const char *name)
{
    return DB_FindXAssetHeader(ASSET_TYPE_IMAGE, (char*)name, 1, -1).image;
}

GfxImage *__cdecl Image_Register(char *imageName, unsigned __int8 semantic, int imageTrack)
{
    if ( useFastFile->current.enabled )
        return (GfxImage *)((int (__cdecl *)(char *, unsigned int, int))Image_Register_FastFile)(imageName, semantic, imageTrack);
    else
        return Image_Register_LoadObj(imageName, semantic, imageTrack);
}

GfxImage *__cdecl Image_Register_LoadObj(char *imageName, unsigned __int8 semantic, unsigned __int8 imageTrack)
{
    GfxImage *image; // [esp+0h] [ebp-4h]
    GfxImage *imagea; // [esp+0h] [ebp-4h]

    image = Image_FindExisting(imageName);
    if ( image )
        return image;
    ProfLoad_Begin("Load image");
    imagea = Image_Load(imageName, semantic, imageTrack);
    ProfLoad_End();
    if ( !imagea )
        Com_PrintError(8, "ERROR: failed to load image '%s'\n", imageName);
    return imagea;
}

GfxImage *__cdecl Image_Register_FastFile(const char *imageName)
{
    return Image_FindExisting(imageName);
}

void __cdecl R_ReleaseLostImages()
{
    DB_EnumXAssets(ASSET_TYPE_IMAGE, (void (__cdecl *)(XAssetHeader, void *))R_FreeLostImage, 0, 1);
}

void __cdecl R_FreeLostImage(XAssetHeader header)
{
    if ( !header.xmodelPieces
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 1293, 0, "%s", "image") )
    {
        __debugbreak();
    }
    if ( !BYTE2(header.xmodelPieces->numpieces)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    1294,
                    0,
                    "%s",
                    "image->category != IMG_CATEGORY_UNKNOWN") )
    {
        __debugbreak();
    }
    if ( BYTE2(header.xmodelPieces->numpieces) >= 5u )
        Image_Release(header.image);
}

void __cdecl R_ReloadLostImages()
{
    DB_EnumXAssets(ASSET_TYPE_IMAGE, (void (__cdecl *)(XAssetHeader, void *))R_RebuildLostImage, 0, 1);
}

void __cdecl R_RebuildLostImage(XAssetHeader header)
{
    if ( !header.xmodelPieces
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 1309, 0, "%s", "image") )
    {
        __debugbreak();
    }
    if ( !BYTE2(header.xmodelPieces->numpieces)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    1310,
                    0,
                    "%s",
                    "image->category != IMG_CATEGORY_UNKNOWN") )
    {
        __debugbreak();
    }
    if ( !header.xmodelPieces->name )
    {
        if ( BYTE2(header.xmodelPieces->numpieces) < 5u )
        {
            if ( BYTE2(header.xmodelPieces->numpieces) == 3 )
            {
                if ( !HIBYTE(header.xmodelPieces->numpieces)
                    && !Image_LoadFromFile(header.image, LOBYTE(header.xmodelPieces[3].numpieces) == 0)
                    && !Image_AssignDefaultTexture(header.image) )
                {
                    Com_Error(
                        ERR_DROP,
                        "Couldn't load image '%s' to recover from a lost device",
                        (const char *)header.xmodelPieces[3].pieces);
                }
            }
            else
            {
                Com_Error(
                    ERR_DROP,
                    "No way to recover image '%s' from a lost device",
                    (const char *)header.xmodelPieces[3].pieces);
            }
        }
        else if ( !Image_IsProg(header.image) )
        {
            Image_Rebuild(header.image);
        }
    }
}

void __cdecl Image_Rebuild(GfxImage *image)
{
    const char *v1; // eax
    unsigned __int8 category; // [esp+0h] [ebp-4h]

    if ( !image && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 1177, 0, "%s", "image") )
        __debugbreak();
    if ( !image->category
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    1178,
                    0,
                    "%s",
                    "image->category != IMG_CATEGORY_UNKNOWN") )
    {
        __debugbreak();
    }
    if ( image->category < 5u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    1181,
                    0,
                    "%s",
                    "image->category >= IMG_CATEGORY_FIRST_UNMANAGED") )
    {
        __debugbreak();
    }
    if ( image->texture.basemap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    1183,
                    0,
                    "%s",
                    "!image->texture.basemap") )
    {
        __debugbreak();
    }
    category = image->category;
    if ( category == 5 )
    {
        Image_BuildWaterMap(image);
    }
    else if ( category == 6 )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                        1192,
                        1,
                        "non-prog image cannot be a render target") )
            __debugbreak();
    }
    else
    {
        v1 = va("unhandled case %i", image->category);
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 1196, 1, v1) )
            __debugbreak();
    }
}

void __cdecl Image_UpdatePicmip(GfxImage *image)
{
    Picmip picmip; // [esp+0h] [ebp-4h] BYREF

    if ( !image && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 1389, 0, "%s", "image") )
        __debugbreak();
    if ( image->category == 3 && !image->noPicmip )
    {
        Image_GetPicmip(image, &picmip);
        if ( image->picmip.platform[0] != picmip.platform[0] )
            Image_Reload(image);
    }
}

void __cdecl Image_Reload(GfxImage *image)
{
    if ( !image && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 1402, 0, "%s", "image") )
        __debugbreak();
    Image_Release(image);
    if ( !Image_LoadFromFile(image, image->skippedMipLevels == 0) )
        Com_Error(ERR_DROP, "failed to load image '%s'", image->name);
}

void __cdecl R_EnumImages(void (__cdecl *func)(GfxImage *, void *), void *data)
{
    GfxImage *header; // [esp+0h] [ebp-8h]
    unsigned int imageIndex; // [esp+4h] [ebp-4h]

    for ( imageIndex = 0; imageIndex < 0x800; ++imageIndex )
    {
        header = imageGlobals.imageHashTable[imageIndex];
        if ( header )
        {
            if ( !Image_IsProg(header) )
                func(header, data);
        }
    }
}

IDirect3DSurface9 *__cdecl Image_GetSurface(GfxImage *image)
{
    const char *v1; // eax
    int hr; // [esp+4h] [ebp-8h]
    IDirect3DSurface9 *surface; // [esp+8h] [ebp-4h] BYREF

    if ( !image && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 1455, 0, "%s", "image") )
        __debugbreak();
    if ( image->mapType != 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    1456,
                    0,
                    "%s",
                    "image->mapType == MAPTYPE_2D") )
    {
        __debugbreak();
    }
    if ( !image->texture.basemap
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 1457, 0, "%s", "image->texture.map") )
    {
        __debugbreak();
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("image->texture.map->GetSurfaceLevel( 0, &surface )\n");
    //hr = ((int (__stdcall *)(unsigned int, unsigned int, unsigned int))image->texture.basemap->__vftable[1].AddRef)(
    //             (GfxTexture)image->texture.basemap,
    //             0,
    //             &surface);
    hr = image->texture.map->GetSurfaceLevel(0, &surface);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v1 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp (%i) image->texture.map->GetSurfaceLevel( 0, &surface ) failed: %s\n",
            1459,
            v1);
    }
    return surface;
}

void __cdecl R_SetPicmip()
{
    unsigned int texMemInMegs; // [esp+0h] [ebp-10h]
    unsigned int sysMemInMegs; // [esp+4h] [ebp-Ch]
    bool cappedPicmip; // [esp+Bh] [ebp-5h]
    int minPicmip; // [esp+Ch] [ebp-4h]

    if ( !dx.device
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 1598, 0, "%s", "dx.device") )
    {
        __debugbreak();
    }
    texMemInMegs = R_AvailableTextureMemory();
    sysMemInMegs = Dvar_GetInt("sys_sysMB");
    if ( !r_reflectionProbeGenerate
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    1608,
                    0,
                    "%s",
                    "r_reflectionProbeGenerate") )
    {
        __debugbreak();
    }
    if ( r_reflectionProbeGenerate->current.enabled )
    {
        Com_Printf(8, "Picmip is set to lowest quality for generating reflections.\n");
        imageGlobals.picmip = 2;
        imageGlobals.picmipBump = 2;
        imageGlobals.picmipSpec = 2;
    }
    else
    {
        if ( r_picmip_manual->current.enabled )
        {
            Com_Printf(8, "Picmip is set manually.\n");
            imageGlobals.picmip = r_picmip->current.integer;
            imageGlobals.picmipBump = r_picmip_bump->current.integer;
            imageGlobals.picmipSpec = r_picmip_spec->current.integer;
        }
        else
        {
            Com_Printf(8, "Texture detail is set automatically.\n");
            if ( texMemInMegs < 0x1C2 )
            {
                if ( texMemInMegs < 0x12C )
                {
                    imageGlobals.picmip = texMemInMegs < 0xC8;
                    imageGlobals.picmipBump = 1;
                }
                else
                {
                    imageGlobals.picmip = 0;
                    imageGlobals.picmipBump = 0;
                }
                imageGlobals.picmipSpec = 1;
            }
            else
            {
                imageGlobals.picmip = 0;
                imageGlobals.picmipBump = 0;
                imageGlobals.picmipSpec = 0;
            }
            if ( sysMemInMegs > 0x180 )
                minPicmip = sysMemInMegs <= 0x280;
            else
                minPicmip = 2;
            if ( minPicmip )
            {
                cappedPicmip = 0;
                if ( imageGlobals.picmip < minPicmip )
                {
                    imageGlobals.picmip = minPicmip;
                    cappedPicmip = 1;
                }
                if ( imageGlobals.picmipBump < minPicmip )
                {
                    imageGlobals.picmipBump = minPicmip;
                    cappedPicmip = 1;
                }
                if ( imageGlobals.picmipSpec < minPicmip )
                {
                    imageGlobals.picmipSpec = minPicmip;
                    cappedPicmip = 1;
                }
                if ( cappedPicmip )
                    Com_Printf(
                        8,
                        "Reducing texture detail based on total system memory of %i MB to improve load times.\n",
                        sysMemInMegs);
            }
            Dvar_SetInt((dvar_s *)r_picmip, imageGlobals.picmip);
            Dvar_SetInt((dvar_s *)r_picmip_bump, imageGlobals.picmipBump);
            Dvar_SetInt((dvar_s *)r_picmip_spec, imageGlobals.picmipSpec);
        }
        Com_Printf(
            8,
            "Using picmip %i on most textures, %i on normal maps, and %i on specular maps\n",
            imageGlobals.picmip,
            imageGlobals.picmipBump,
            imageGlobals.picmipSpec);
    }
}

void __cdecl R_InitImages()
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 2; ++i )
    {
        if ( imageGlobals.totalMemory.platform[i] )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                            1703,
                            0,
                            "%s",
                            "imageGlobals.totalMemory.platform[i] == 0") )
                __debugbreak();
        }
    }
    R_SetPicmip();
    R_InitCodeImages();
    RB_InitImages();
    R_InitRawImage();
}

GfxImage *R_InitRawImage()
{
    GfxImage *result; // eax

    result = Image_AllocProg(11, 4u, 0);
    rgp.rawImage = result;
    if ( !result )
    {
        result = (GfxImage *)Assert_MyHandler(
                                                     "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                                                     1466,
                                                     1,
                                                     "%s",
                                                     "rgp.rawImage");
        if ( !(_BYTE)result )
            __debugbreak();
    }
    return result;
}

GfxImage *R_InitCodeImages()
{
    GfxImage *result; // eax

    rgp.whiteImage = Image_Register("$white", 1u, 0);
    if ( !rgp.whiteImage
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 1511, 1, "%s", "rgp.whiteImage") )
    {
        __debugbreak();
    }
    rgp.blackImage = Image_Register("$black", 1u, 0);
    if ( !rgp.blackImage
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 1514, 1, "%s", "rgp.blackImage") )
    {
        __debugbreak();
    }
    rgp.blankImage = Image_Register("$blank", 1u, 0);
    if ( !rgp.blankImage
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 1517, 1, "%s", "rgp.blankImage") )
    {
        __debugbreak();
    }
    rgp.g16r16Image = Image_Register("$g16r16", 1u, 0);
    if ( !rgp.g16r16Image
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 1520, 1, "%s", "rgp.g16r16Image") )
    {
        __debugbreak();
    }
    rgp.r5g6b5Image = Image_Register("$r5g6b5", 1u, 0);
    if ( !rgp.r5g6b5Image
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 1523, 1, "%s", "rgp.r5g6b5Image") )
    {
        __debugbreak();
    }
    rgp.grayImage = Image_Register("$gray", 1u, 0);
    if ( !rgp.grayImage
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 1534, 1, "%s", "rgp.grayImage") )
    {
        __debugbreak();
    }
    rgp.identityNormalMapImage = Image_Register("$identitynormalmap", 1u, 0);
    if ( !rgp.identityNormalMapImage
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    1537,
                    1,
                    "%s",
                    "rgp.identityNormalMapImage") )
    {
        __debugbreak();
    }
    rgp.whiteTransparentImage = Image_Register("$whitetransparent", 1u, 0);
    if ( !rgp.whiteTransparentImage
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    1540,
                    1,
                    "%s",
                    "rgp.whiteTransparentImage") )
    {
        __debugbreak();
    }
    rgp.blackTransparentImage = Image_Register("$blacktransparent", 1u, 0);
    if ( !rgp.blackTransparentImage
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    1543,
                    1,
                    "%s",
                    "rgp.blackTransparentImage") )
    {
        __debugbreak();
    }
    rgp.pixelCostColorCodeImage = Image_Register("$pixelcostcolorcode", 1u, 0);
    if ( !rgp.pixelCostColorCodeImage
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    1547,
                    1,
                    "%s",
                    "rgp.pixelCostColorCodeImage") )
    {
        __debugbreak();
    }
    result = Image_Register("$white_r32f", 1u, 0);
    rgp.r32fWhiteImage = result;
    if ( !result )
    {
        result = (GfxImage *)Assert_MyHandler(
                                                     "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                                                     1568,
                                                     1,
                                                     "%s",
                                                     "rgp.r32fWhiteImage");
        if ( !(_BYTE)result )
            __debugbreak();
    }
    rgp.heatMapImage = 0;
    return result;
}

const char *g_platform_name[2] = { "current", " min_pc" };
const char *imageTypeName[10] =
{
  "misc  ",
  "debug ",
  "$tex+?",
  "ui    ",
  "lmap  ",
  "light ",
  "f/x   ",
  "hud   ",
  "model ",
  "world "
};


void __cdecl R_ImageList_Output()
{
    const char *v0; // eax
    const char *fmt; // [esp+Ch] [ebp-4318h]
    GfxImage *image; // [esp+A8h] [ebp-427Ch]
    int v3; // [esp+ACh] [ebp-4278h]
    unsigned __int8 dst[80]; // [esp+B4h] [ebp-4270h] BYREF
    unsigned int v5[2]; // [esp+104h] [ebp-4220h]
    _D3DFORMAT v6; // [esp+10Ch] [ebp-4218h]
    unsigned int j; // [esp+110h] [ebp-4214h]
    ImageList imageList; // [esp+114h] [ebp-4210h] BYREF
    int i; // [esp+431Ch] [ebp-8h]
    float v10; // [esp+4320h] [ebp-4h]

    v5[0] = 0;
    v5[1] = 0;
    memset(dst, 0, sizeof(dst));
    R_GetImageList(&imageList);
    //std::_Sort<GfxImage * *,int,int (__cdecl *)(GfxImage *,GfxImage *)>(
    //    imageList.image,
    //    &imageList.image[imageList.count],
    //    (signed int)(4 * imageList.count) >> 2,
    //    imagecompare);
    std::sort(&imageList.image[0], &imageList.image[imageList.count], imagecompare);
    Com_Printf(8, "\n-reqrd w*h-");
    Com_Printf(8, "-fmt-    ");
    for ( i = 0; i < 2; ++i )
        Com_Printf(8, "%s", g_platform_name[i]);
    Com_Printf(8, "    --name-------\n");
    for ( j = 0; j < imageList.count; ++j )
    {
        image = imageList.image[j];
        Com_Printf(8, "%4i x %-4i ", image->width, image->height);
        v6 = R_ImagePixelFormat(image);
        if ( v6 > D3DFMT_L8 )
        {
            if ( v6 > D3DFMT_DXT1 )
            {
                if ( v6 == D3DFMT_DXT3 )
                {
                    Com_Printf(8, "DXT3    ");
                    goto LABEL_30;
                }
                if ( v6 == D3DFMT_DXT5 )
                {
                    Com_Printf(8, "DXT5    ");
                    goto LABEL_30;
                }
            }
            else
            {
                switch ( v6 )
                {
                    case D3DFMT_DXT1:
                        Com_Printf(8, "DXT1    ");
                        goto LABEL_30;
                    case D3DFMT_A8L8:
                        Com_Printf(8, "AL16    ");
                        goto LABEL_30;
                    case D3DFMT_R32F:
                        Com_Printf(8, "R32F    ");
                        goto LABEL_30;
                }
            }
LABEL_28:
            v0 = va("unhandled case: %d", v6);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 1994, 1, v0) )
                __debugbreak();
        }
        else if ( v6 == D3DFMT_L8 )
        {
            Com_Printf(8, "L8        ");
        }
        else
        {
            switch ( v6 )
            {
                case D3DFMT_A8R8G8B8:
                    Com_Printf(8, "RGBA32");
                    break;
                case D3DFMT_X8R8G8B8:
                    Com_Printf(8, "RGB32 ");
                    break;
                case D3DFMT_R5G6B5:
                    Com_Printf(8, "RGB565");
                    break;
                case D3DFMT_A8:
                    Com_Printf(8, "A8        ");
                    break;
                case D3DFMT_G16R16:
                    Com_Printf(8, "G16R16");
                    break;
                default:
                    goto LABEL_28;
            }
        }
LABEL_30:
        Com_Printf(8, "    %s", imageTypeName[image->track]);
        for ( i = 0; i < 2; ++i )
        {
            v3 = image->cardMemory.platform[i];
            v10 = (float)v3 / 1024.0;
            if ( v10 >= 10.0 )
                fmt = "%7.0fk";
            else
                fmt = "%7.1fk";
            Com_Printf(8, fmt, v10);
            if ( !useFastFile->current.enabled )
            {
                *(unsigned int *)&dst[8 * image->track + 4 * i] += v3;
                if ( Image_IsCodeImage(image->track) )
                    continue;
            }
            v5[i] += v3;
        }
        Com_Printf(8, "    %s\t\t%x\n", image->name, image);
    }
    Com_Printf(8, " ---------\n");
    Com_Printf(8, " %i total images\n", imageList.count);
    for ( i = 0; i < 2; ++i )
        Com_Printf(
            8,
            " %5.1f MB %s total non-streamed image size\n",
            (float)((float)(int)v5[i] / 1048576.0),
            g_platform_name[i]);
    if ( !useFastFile->current.enabled )
    {
        Com_Printf(8, "\n");
        Com_Printf(8, "             ");
        for ( i = 0; i < 2; ++i )
            Com_Printf(8, "%s", g_platform_name[i]);
        Com_Printf(8, "\n");
        for ( j = 0; j < 0xA; ++j )
        {
            Com_Printf(8, "%s:", imageTypeName[j]);
            for ( i = 0; i < 2; ++i )
                Com_Printf(8, "    %5.1f", (float)((float)*(int *)&dst[8 * j + 4 * i] / 1048576.0));
            Com_Printf(8, "    MB\n");
        }
    }
    Com_Printf(8, "Related commands: meminfo, imagelist, gfx_world, gfx_model, cg_drawfps, com_statmon, tempmeminfo\n");
}

bool __cdecl imagecompare(GfxImage *image1, GfxImage *image2)
{
    if ( image1->track > (int)image2->track )
        return 0;
    if ( image1->track >= (int)image2->track )
        return image1->cardMemory.platform[0] < image2->cardMemory.platform[0];
    return 1;
}

_D3DFORMAT __cdecl R_ImagePixelFormat(GfxImage *image)
{
    MapType mapType; // [esp+0h] [ebp-40h]
    _D3DSURFACE_DESC surfaceDesc; // [esp+4h] [ebp-3Ch] BYREF
    _D3DVOLUME_DESC volumeDesc; // [esp+24h] [ebp-1Ch] BYREF

    mapType = (MapType)image->mapType;

    if (image->mapType == MAPTYPE_2D)
    {
        iassert(image->texture.map);

        image->texture.map->GetLevelDesc(0, &surfaceDesc);
        return surfaceDesc.Format;
    }

    if (mapType == MAPTYPE_3D)
    {
        iassert(image->texture.volmap);
        image->texture.volmap->GetLevelDesc(0, &volumeDesc);
        return volumeDesc.Format;
    }
    if (mapType == MAPTYPE_CUBE)
    {
        iassert(image->texture.cubemap);
        image->texture.cubemap->GetLevelDesc(0, &surfaceDesc);
        return surfaceDesc.Format;
    }

    iassert(0);
    //if (!alwaysfails)
    //{
    //    MyAssertHandler(".\\r_image.cpp", 1403, 1, va("unhandled case %i for %s", image->mapType, image->name));
    //}

    return (_D3DFORMAT)0;
}

void __cdecl R_DownsampleMipMapBilinear(
                const unsigned __int8 *src,
                int srcWidth,
                int srcHeight,
                int texelPitch,
                unsigned __int8 *dst)
{
    int srcRowPitch; // [esp+0h] [ebp-24h]
    int channelIndex; // [esp+4h] [ebp-20h]
    int channelIndexa; // [esp+4h] [ebp-20h]
    unsigned __int8 *dstPos; // [esp+8h] [ebp-1Ch]
    unsigned __int8 *dstPosa; // [esp+8h] [ebp-1Ch]
    int dstWidth; // [esp+Ch] [ebp-18h]
    int dstWidtha; // [esp+Ch] [ebp-18h]
    int dstHeight; // [esp+10h] [ebp-14h]
    int colIndex; // [esp+14h] [ebp-10h]
    int colIndexa; // [esp+14h] [ebp-10h]
    const unsigned __int8 *srcPos; // [esp+1Ch] [ebp-8h]
    const unsigned __int8 *srcPosa; // [esp+1Ch] [ebp-8h]
    int rowIndex; // [esp+20h] [ebp-4h]

    if ( !src && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 2149, 0, "%s", "src") )
        __debugbreak();
    if ( srcWidth <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    2150,
                    0,
                    "%s\n\t(srcWidth) = %i",
                    "(srcWidth > 0)",
                    srcWidth) )
    {
        __debugbreak();
    }
    if ( srcHeight <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    2151,
                    0,
                    "%s\n\t(srcHeight) = %i",
                    "(srcHeight > 0)",
                    srcHeight) )
    {
        __debugbreak();
    }
    if ( texelPitch <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp",
                    2152,
                    0,
                    "%s\n\t(texelPitch) = %i",
                    "(texelPitch > 0)",
                    texelPitch) )
    {
        __debugbreak();
    }
    if ( !dst && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image.cpp", 2154, 0, "%s", "dst") )
        __debugbreak();
    if ( srcWidth != 1 || srcHeight != 1 )
    {
        srcPos = src;
        dstPos = dst;
        dstWidth = srcWidth >> 1;
        dstHeight = srcHeight >> 1;
        srcRowPitch = texelPitch * srcWidth;
        if ( srcWidth >> 1 && dstHeight )
        {
            for ( rowIndex = 0; rowIndex < dstHeight; ++rowIndex )
            {
                srcPosa = &src[2 * rowIndex * srcRowPitch];
                dstPosa = &dst[rowIndex * texelPitch * (srcWidth >> 1)];
                for ( colIndexa = 0; colIndexa < dstWidth; ++colIndexa )
                {
                    for ( channelIndexa = 0; channelIndexa < texelPitch; ++channelIndexa )
                        dstPosa[channelIndexa] = (srcPosa[texelPitch + channelIndexa + srcRowPitch]
                                                                        + srcPosa[channelIndexa + srcRowPitch]
                                                                        + srcPosa[texelPitch + channelIndexa]
                                                                        + srcPosa[channelIndexa]) >> 2;
                    dstPosa += texelPitch;
                    srcPosa += 2 * texelPitch;
                }
            }
        }
        else
        {
            dstWidtha = dstHeight + dstWidth;
            for ( colIndex = 0; colIndex < dstWidtha; ++colIndex )
            {
                for ( channelIndex = 0; channelIndex < texelPitch; ++channelIndex )
                    dstPos[channelIndex] = (srcPos[texelPitch + channelIndex] + srcPos[channelIndex]) >> 1;
                dstPos += texelPitch;
                srcPos += 2 * texelPitch;
            }
        }
    }
}

