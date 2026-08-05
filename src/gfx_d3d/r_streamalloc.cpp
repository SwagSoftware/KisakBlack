#include "r_streamalloc.h"
#include "r_image.h"
#include "r_stream.h"

StreamAllocGlob s_allocGlob;
int list_count;

void __cdecl R_StreamAlloc_InitTempImages()
{
    int index; // [esp+4h] [ebp-4h]

    if (!s_allocGlob.tempImagesInit)
    {
        for (index = 0; index < 32; ++index)
        {
            s_allocGlob.tempImages[2 * index] = DB_AllocTempImage();
            memset(s_allocGlob.tempImages[2 * index], 0, sizeof(GfxImage));
            LOBYTE(s_allocGlob.tempImages[2 * index + 1]) = 0;
        }
        s_allocGlob.tempImagesInit = 1;
    }
}

GfxImage *__cdecl R_StreamAlloc_SetupTempImage(
    _D3DFORMAT format,
    bool linear,
    unsigned __int16 width,
    unsigned __int16 height)
{
    GfxImage *image; // [esp+0h] [ebp-8h]
    int index; // [esp+4h] [ebp-4h]

    if (!s_allocGlob.tempImagesInit
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_streamalloc.cpp",
            1755,
            0,
            "%s",
            "s_allocGlob.tempImagesInit"))
    {
        __debugbreak();
    }
    R_StreamAlloc_Lock();
    image = 0;
    for (index = 0; index < 32; ++index)
    {
        if (!LOBYTE(s_allocGlob.tempImages[2 * index + 1]))
        {
            LOBYTE(s_allocGlob.tempImages[2 * index + 1]) = 1;
            image = s_allocGlob.tempImages[2 * index];
            break;
        }
    }
    if (image)
    {
        Image_Create2DTexture_PC(image, width, height, 0, 0, format);
        R_StreamAlloc_Unlock();
        return image;
    }
    else
    {
        R_StreamAlloc_Unlock();
        return 0;
    }
}

void __cdecl R_StreamAlloc_ReleaseTempImage(GfxImage *image)
{
    int index; // [esp+0h] [ebp-4h]

    R_StreamAlloc_Lock();
    for (index = 0; index < 32; ++index)
    {
        if (image == s_allocGlob.tempImages[2 * index])
        {
            Image_Release(image);
            LOBYTE(s_allocGlob.tempImages[2 * index + 1]) = 0;
            R_StreamAlloc_Unlock();
            return;
        }
    }
    if (!Assert_MyHandler(
        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_streamalloc.cpp",
        1799,
        0,
        "%s",
        "0 && \"tried to release a non-temp image\""))
        __debugbreak();
    R_StreamAlloc_Unlock();
}

void __cdecl R_StreamAlloc_Lock()
{
    Sys_LockWrite(&s_allocGlob.lock);
}

void __cdecl R_StreamAlloc_Unlock()
{
    Sys_UnlockWrite(&s_allocGlob.lock);
}

char __cdecl R_StreamAlloc_CanAllocate(int size, float importance, GfxImage **unloadImage)
{
    *unloadImage = 0;
    if ( size + Image_TrackBytes <= streamFrontendGlob.mainBufferSize )
        return 1;
    else
        return R_StreamAlloc_FreeImageByImportance(size, importance, unloadImage);
}

char __cdecl R_StreamAlloc_FreeImageByImportance(int size, float importance, GfxImage **unloadImage)
{
    XAssetPoolEntry<GfxImage> *image; // [esp+0h] [ebp-20h]
    GfxImage *sacrificeImage; // [esp+4h] [ebp-1Ch]
    signed int imagePartIndex; // [esp+8h] [ebp-18h]
    int sortIndex; // [esp+Ch] [ebp-14h]

    sacrificeImage = 0;
    for ( sortIndex = streamFrontendGlob.totalBytesWanted - 1; sortIndex >= 0; --sortIndex )
    {
        imagePartIndex = *(int *)((char *)&streamFrontendGlob.sortedImages[sortIndex] + 2);
        if ( (streamFrontendGlob.imageUseBits[(imagePartIndex >> 5) - 4] & (1 << (imagePartIndex & 0x1F))) != 0
            && (importance == 3.4028235e38
             || (streamFrontendGlob.imageForceBits[imagePartIndex >> 5] & (1 << (imagePartIndex & 0x1F))) == 0) )
        {
            if ( *(float *)&streamFrontendGlob.imageImportanceBits[imagePartIndex - 4064] >= importance )
                break;
            if ( (streamFrontendGlob.imageLoading[(imagePartIndex >> 5) - 8] & (1 << (imagePartIndex & 0x1F))) == 0 )
            {
                image = DB_GetImageAtIndex(imagePartIndex);
                if ( image->entry.streaming )
                {
                    if ( !image->entry.skippedMipLevels )
                    {
                        if ( (signed int)image->entry.loadedSize >= size )
                        {
                            *unloadImage = &image->entry;
                            return 1;
                        }
                        if ( !sacrificeImage )
                            sacrificeImage = &image->entry;
                    }
                }
            }
        }
    }
    if ( sacrificeImage )
    {
        *unloadImage = sacrificeImage;
        return 1;
    }
    else
    {
        *unloadImage = 0;
        return 0;
    }
}

unsigned __int8 *__cdecl R_StreamAlloc_FreeImage(GfxImage *image, int imagePart, bool dirty, int *freedSize)
{
    int imagePartIndex; // [esp+0h] [ebp-4h]

    imagePartIndex = DB_GetImageIndex(image);
    streamFrontendGlob.imageUseBits[(imagePartIndex >> 5) - 4] &= ~(1 << (imagePartIndex & 0x1F));
    *freedSize = 0;
    return 0;
}

void __cdecl R_StreamAlloc_Flush()
{
    unsigned int useBits; // [esp+0h] [ebp-10h]
    int bitIndex; // [esp+4h] [ebp-Ch]
    int index; // [esp+Ch] [ebp-4h]

    for ( index = 0; index < 132; ++index )
    {
        useBits = streamFrontendGlob.imageUseBits[index - 4];
        if ( useBits )
        {
            for ( bitIndex = 0; bitIndex < 32; ++bitIndex )
            {
                if ( (useBits & (1 << (bitIndex & 0x1F))) != 0 )
                    streamFrontendGlob.imageUseBits[((bitIndex + 32 * index) >> 5) - 4] &= ~(1 << ((bitIndex + 32 * index) & 0x1F));
            }
        }
    }
}

