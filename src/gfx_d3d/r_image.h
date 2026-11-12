#pragma once
#include <database/db_registry.h>

struct GfxImage;
union GfxTexture;

enum MapType : __int32
{                                       // ...
    MAPTYPE_NONE = 0x0,
    MAPTYPE_INVALID1 = 0x1,
    MAPTYPE_INVALID2 = 0x2,
    MAPTYPE_2D = 0x3,
    MAPTYPE_3D = 0x4,
    MAPTYPE_CUBE = 0x5,
    MAPTYPE_COUNT = 0x6,
};

struct ImageList // sizeof=0x4004
{                                       // XREF: ?R_SumOfUsedImages@@YAXPAUImage_MemUsage@@@Z/r
                                        // ?R_ImageList_Output@@YAXXZ/r ...
    unsigned int count;                 // XREF: R_SumOfUsedImages(Image_MemUsage *)+B1/r
                                        // R_ImageList_Output(void)+73/r ...
    GfxImage *image[4608];              // Same as POOLSIZE_IMAGE (retail: 4096, smaller than its own 4224 pool)
                                        // R_ImageList_Output(void)+A8/w ...
};

struct Image_MemUsage;

void __cdecl Image_TrackTotalMemory(GfxImage *image, int platform, int memory);
bool __cdecl Image_IsCodeImage(int track);
void __cdecl R_GetImageList(ImageList *imageList);
void __cdecl R_AddImageToList(XAssetHeader header, XAssetHeader *data);
void __cdecl R_SumOfUsedImages(Image_MemUsage *usage);
void __cdecl Image_Release(GfxImage *image);
GfxImage *__cdecl Image_AllocProg(int imageProgType, unsigned __int8 category, unsigned __int8 semantic);
unsigned int __cdecl Image_GetAvailableHashLocation(const char *name);
GfxImage *__cdecl Image_Alloc(
                char *name,
                unsigned __int8 category,
                unsigned __int8 semantic,
                unsigned __int8 imageTrack);
void __cdecl Image_Construct(
                char *name,
                int nameSize,
                unsigned __int8 category,
                unsigned __int8 semantic,
                unsigned __int8 imageTrack,
                GfxImage *image);
void __cdecl R_ShutdownImages();
bool __cdecl Image_IsProg(GfxImage *image);
void __cdecl Image_Free(GfxImage *image);
void __cdecl Image_Create2DTexture_PC(
                GfxImage *image,
                unsigned __int16 width,
                unsigned __int16 height,
                unsigned int mipmapCount,
                int imageFlags,
                _D3DFORMAT imageFormat);
unsigned int __cdecl Image_GetUsage(int imageFlags, _D3DFORMAT imageFormat);
void __cdecl Image_Create3DTexture_PC(
                GfxImage *image,
                unsigned __int16 width,
                unsigned __int16 height,
                unsigned __int16 depth,
                unsigned int mipmapCount,
                int imageFlags,
                _D3DFORMAT imageFormat);
void __cdecl Image_CreateCubeTexture_PC(
                GfxImage *image,
                unsigned __int16 edgeLen,
                unsigned int mipmapCount,
                _D3DFORMAT imageFormat);
void __cdecl Image_SetupRenderTarget(
                GfxImage *image,
                unsigned __int16 width,
                unsigned __int16 height,
                _D3DFORMAT imageFormat);
void __cdecl Image_PicmipForSemantic(unsigned __int8 semantic, Picmip *picmip);
void __cdecl R_DelayLoadImage(XAssetHeader header, void *data);
char __cdecl Image_AssignDefaultTexture(GfxImage *image);
char __cdecl R_DuplicateTexture(GfxImage *dstImage, const GfxImage *srcImage);
void __cdecl Load_Texture(GfxTexture *remoteLoadDef, GfxImage *image);
GfxImage *__cdecl Image_FindExisting(const char *name);
GfxImage *__cdecl Image_FindExisting_LoadObj(const char *name);
GfxImage *__cdecl Image_FindExisting_FastFile(const char *name);
GfxImage *__cdecl Image_Register(char *imageName, unsigned __int8 semantic, int imageTrack);
inline GfxImage *__cdecl Image_Register(const char *imageName, unsigned __int8 semantic, int imageTrack)
{
    return Image_Register((char *)imageName, semantic, imageTrack);
}
GfxImage *__cdecl Image_Register_LoadObj(char *imageName, unsigned __int8 semantic, unsigned __int8 imageTrack);
GfxImage *__cdecl Image_Register_FastFile(const char *imageName);
void __cdecl R_ReleaseLostImages();
void __cdecl R_FreeLostImage(XAssetHeader header);
void __cdecl R_ReloadLostImages();
void __cdecl R_RebuildLostImage(XAssetHeader header);
void __cdecl Image_Rebuild(GfxImage *image);
void __cdecl Image_UpdatePicmip(GfxImage *image);
void __cdecl Image_Reload(GfxImage *image);
void __cdecl R_EnumImages(void (__cdecl *func)(struct GfxImage *, void *), void *data);
IDirect3DSurface9 *__cdecl Image_GetSurface(GfxImage *image);
void __cdecl R_SetPicmip();
void __cdecl R_InitImages();
GfxImage *R_InitRawImage();
GfxImage *R_InitCodeImages();
void __cdecl R_ImageList_Output();
bool __cdecl imagecompare(GfxImage *image1, GfxImage *image2);
_D3DFORMAT __cdecl R_ImagePixelFormat(GfxImage *image);
void __cdecl R_DownsampleMipMapBilinear(
                const unsigned __int8 *src,
                int srcWidth,
                int srcHeight,
                int texelPitch,
                unsigned __int8 *dst);

extern int Image_TrackBytes;