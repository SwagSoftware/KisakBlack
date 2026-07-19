#include "r_cinematic.h"
#include <qcommon/common.h>
#ifdef _WIN32
#include <Windows.h>
#include <mmreg.h>
#endif
#include <dsound.h>
#include <win32/win_wndproc.h>
#include "r_init.h"
#include <qcommon/threads.h>
#include "rb_resource.h"
#include "r_image_load_common.h"
#include <universal/com_memory.h>
#include <universal/com_workercmds.h>
#include <sound/snd_public_async.h>
#include <win32/win_shared.h>
#include <win32/win_net.h>
#include "rb_state.h"
#include "r_material_load_obj.h"

struct CinematicGlob // sizeof=0x980
{                                       // XREF: .data:cinematicGlob/r
    char currentCinematicName[256];     // XREF: R_Cinematic_UpdateFrame_Core+112/r
    char targetCinematicName[256];      // XREF: R_Cinematic_StartPlayback_Internal+16/o
    char nextCinematicName[256];        // XREF: R_Cinematic_StartNextPlayback(void)+2A/o
                                        // R_Cinematic_StartNextPlayback(void)+37/w ...
    unsigned int nextCinematicPlaybackFlags;
    unsigned int playbackFlags;         // XREF: R_Cinematic_StartPlayback_Internal+55/w

    unsigned int timeInMsec;            // XREF: R_Cinematic_UpdateTimeInMsec:loc_AA99B2/w
    unsigned int binkIOSize;            // XREF: R_Cinematic_Advance+5AB/r
                                        // R_Cinematic_GetPercentageFull+7C/r ...
    bool isPreloaded;                   // XREF: R_Cinematic_StopPlayback_Now:loc_AA9C01/w
    bool firstFrameNotify;              // XREF: R_Cinematic_Advance+450/w
    bool lastFrameNotify;               // XREF: R_Cinematic_Advance+4FC/w
    bool targetCinematicChanged;        // XREF: R_Cinematic_StartPlayback_Internal+23/w
    bool cinematicFinished;             // XREF: R_Cinematic_StartPlayback_Internal+2A/w
                                        // R_Cinematic_StopPlayback(void)+25/w ...
    volatile bool fullSyncNextUpdate;
    bool playbackStarted;
    // LWSS: fields around here are inferred (idk why they got cut out)
    char crap[1];                       // XREF: R_Cinematic_Advance+188/w
                                        // R_Cinematic_Advance+195/w ...
    float framerateSpeedFactor;
    int startTimeMS;                   
    int frameNum;
    char crap2[3];
    // padding byte
    // lwss end
    CIN_IOSTATE fileIoState;            // XREF: R_Cinematic_BeginLostDevice(void)+10/r
                                        // R_Cinematic_RelinquishIO+4/r ...
    bool usingAlpha;
    // padding byte
    // padding byte
    // padding byte
    BINK *bink;                        // XREF: R_Cinematic_Advance+195/w
                                        // R_Cinematic_Advance+3B5/w ...
    BINKTEXTURESET binkTextureSet;      // XREF: Unlock_Bink_textures_Callback(void)+3/r
                                        // Unlock_Bink_textures_Callback(void)+9/o ...
    // lwss inferment
    void *YTextures[2];                 // XREF: R_Cinematic_Advance+2D2/r
                                        // R_Cinematic_InitBinkTextures+1A5/w
    void *crTextures[2];                // XREF: R_Cinematic_Advance+2EE/r
                                        // R_Cinematic_InitBinkTextures+1E6/w
    void *cbTextures[2];                // XREF: R_Cinematic_Advance+30A/r
                                        // R_Cinematic_InitBinkTextures+227/w
    void *aTextures[2];                 // XREF: R_Cinematic_Advance+32F/r
                                        // R_Cinematic_InitBinkTextures+277/w ...
    int bufferWidth;  
    // lwss inferment end

    CinematicHunk masterHunk;           // XREF: R_Cinematic_Advance+2D2/r
                                        // R_Cinematic_InitBinkTextures+1A5/w
    CinematicHunk binkHunk;             // XREF: R_Cinematic_Advance+2EE/r
                                        // R_Cinematic_Advance+30A/r ...
    CinematicHunk residentHunk;         // XREF: R_Cinematic_AreHunksOpen+3/o
                                        // R_Cinematic_HunksClose+3/o ...
    int activeImageFrame;
    int framesStopped;                  // XREF: R_Cinematic_AreHunksOpen+17/o
                                        // R_Cinematic_AreHunksOpen:loc_AA8ED1/o ...
    CinematicEnum currentPaused;        // XREF: R_Cinematic_Bink_Free+14/w
    CinematicEnum targetPaused;
    CinematicTextureSet textureSets[2]; // ...
    int activeTextureSet;
    int activeImageFrameTextureSet;
    GfxImage *nextYTexture;
    GfxImage *nextCRTexture;
    GfxImage *nextCBTexture;
    GfxImage *nextATexture;
    void *memPool;
    float playbackVolume;
    bool underrun;
    // padding byte
    // padding byte
    // padding byte
    int lastTime;
    int gap[8];
    int gptr;
} cinematicGlob;

LPDIRECTSOUND8 g_cinematicDS;

int activeTexture;
bool g_cinematicInitialized;

volatile unsigned int cineThreadBlock;
int r_cinematic_disabled;

volatile unsigned int _UpdateFrameLimit = 1;
jqModule _UpdateFrameModule =
{
    .Name = "_UpdateFrame",
    .Type = JQ_WORKER_GENERIC,
    .Code = (int(__cdecl *)(jqBatch *))_UpdateFrameCallback,
    //.Group = 0
};
jqWorkerCmd _UpdateFrameWorkerCmd = { &_UpdateFrameModule, sizeof(CINE_JQ_CMD), 0, 0, &_UpdateFrameLimit, NULL, 0u};

void __cdecl R_CinematicInitSound(const _GUID *guid)
{
    if ( g_cinematicDS )
        g_cinematicDS->Release();
    if ( DirectSoundCreate8(guid, &g_cinematicDS, 0) )
        DirectSoundCreate8(0, &g_cinematicDS, 0);
    if ( g_cinematicDS )
    {
        if ( g_cinematicDS->SetCooperativeLevel(g_wv.hWnd, 2u) )
        {
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp", 495, 0, "%s", "hr == S_OK") )
                __debugbreak();
        }
    }
    else
    {
        Com_PrintError(16, "R_Cinematic_Init: Unable to initialize sound\n");
    }
}

void __cdecl Unlock_Bink_textures2(IDirect3DDevice9 *d3d_device, BINKTEXTURESET *set_textures)
{
    signed int m; // [esp+0h] [ebp-30h]
    signed int k; // [esp+4h] [ebp-2Ch]
    signed int j; // [esp+8h] [ebp-28h]
    int i; // [esp+Ch] [ebp-24h]
    unsigned __int8 *srcBits; // [esp+14h] [ebp-1Ch]
    unsigned __int8 *srcBitsa; // [esp+14h] [ebp-1Ch]
    unsigned __int8 *srcBitsb; // [esp+14h] [ebp-1Ch]
    unsigned __int8 *srcBitsc; // [esp+14h] [ebp-1Ch]
    char *dstBits; // [esp+18h] [ebp-18h]
    char *dstBitsa; // [esp+18h] [ebp-18h]
    char *dstBitsb; // [esp+18h] [ebp-18h]
    char *dstBitsc; // [esp+18h] [ebp-18h]
    int srcWidth; // [esp+1Ch] [ebp-14h]
    int srcWidtha; // [esp+1Ch] [ebp-14h]
    int srcWidthb; // [esp+1Ch] [ebp-14h]
    int srcWidthc; // [esp+1Ch] [ebp-14h]
    signed int srcHeight; // [esp+20h] [ebp-10h]
    signed int srcHeighta; // [esp+20h] [ebp-10h]
    signed int srcHeightb; // [esp+20h] [ebp-10h]
    signed int srcHeightc; // [esp+20h] [ebp-10h]
    BINKFRAMEPLANESET *bp_src; // [esp+24h] [ebp-Ch]
    _D3DLOCKED_RECT lr; // [esp+28h] [ebp-8h] BYREF

    bp_src = &set_textures->bink_buffers.Frames[set_textures->bink_buffers.FrameNum];
    set_textures->tex_draw.Ytexture->LockRect(0, &lr, 0, 0x2000u);
    srcWidth = set_textures->bink_buffers.YABufferWidth;
    srcHeight = set_textures->bink_buffers.YABufferHeight;
    srcBits = (unsigned __int8 *)bp_src->YPlane.Buffer;
    dstBits = (char *)lr.pBits;
    if (lr.Pitch == srcWidth)
    {
        memcpy((unsigned __int8 *)lr.pBits, srcBits, srcHeight * srcWidth);
    }
    else
    {
        for (i = 0; i < srcHeight; ++i)
            memcpy((unsigned __int8 *)&dstBits[lr.Pitch * i], &srcBits[srcWidth * i], srcWidth);
    }
    set_textures->tex_draw.Ytexture->UnlockRect(0);
    set_textures->tex_draw.cRtexture->LockRect(0, &lr, 0, 0x2000u);
    srcWidtha = set_textures->bink_buffers.cRcBBufferWidth;
    srcHeighta = set_textures->bink_buffers.cRcBBufferHeight;
    srcBitsa = (unsigned __int8 *)bp_src->cRPlane.Buffer;
    dstBitsa = (char *)lr.pBits;
    if (lr.Pitch == srcWidtha)
    {
        memcpy((unsigned __int8 *)lr.pBits, srcBitsa, srcHeighta * srcWidtha);
    }
    else
    {
        for (j = 0; j < srcHeighta; ++j)
            memcpy((unsigned __int8 *)&dstBitsa[lr.Pitch * j], &srcBitsa[srcWidtha * j], srcWidtha);
    }
    set_textures->tex_draw.cRtexture->UnlockRect(0);
    set_textures->tex_draw.cBtexture->LockRect(0, &lr, 0, 0x2000u);
    srcWidthb = set_textures->bink_buffers.cRcBBufferWidth;
    srcHeightb = set_textures->bink_buffers.cRcBBufferHeight;
    srcBitsb = (unsigned __int8 *)bp_src->cBPlane.Buffer;
    dstBitsb = (char *)lr.pBits;
    if (lr.Pitch == srcWidthb)
    {
        memcpy((unsigned __int8 *)lr.pBits, srcBitsb, srcHeightb * srcWidthb);
    }
    else
    {
        for (k = 0; k < srcHeightb; ++k)
            memcpy((unsigned __int8 *)&dstBitsb[lr.Pitch * k], &srcBitsb[srcWidthb * k], srcWidthb);
    }
    set_textures->tex_draw.cBtexture->UnlockRect(0);
    if (set_textures->tex_draw.Atexture)
    {
        set_textures->tex_draw.Atexture->LockRect(0, &lr, 0, 0x2000u);
        srcWidthc = set_textures->bink_buffers.YABufferWidth;
        srcHeightc = set_textures->bink_buffers.YABufferHeight;
        srcBitsc = (unsigned __int8 *)bp_src->APlane.Buffer;
        dstBitsc = (char *)lr.pBits;
        if (lr.Pitch == srcWidthc)
        {
            memcpy((unsigned __int8 *)lr.pBits, srcBitsc, srcHeightc * srcWidthc);
        }
        else
        {
            for (m = 0; m < srcHeightc; ++m)
                memcpy((unsigned __int8 *)&dstBitsc[lr.Pitch * m], &srcBitsc[srcWidthc * m], srcWidthc);
        }
        set_textures->tex_draw.Atexture->UnlockRect(0);
    }
}

void __cdecl Unlock_Bink_textures_Callback()
{
    Unlock_Bink_textures2(dx.device, &cinematicGlob.binkTextureSet);
}

void __cdecl R_Cinematic_BeginLostDevice()
{
    unsigned int setIter; // [esp+4h] [ebp-8h]
    CinematicTextureSet *textureSet; // [esp+8h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_CINEMATIC);

    if (cinematicGlob.fileIoState)
        R_Cinematic_RelinquishIO();

    if (R_Cinematic_IsStarted() && cinematicGlob.currentPaused == CINEMATIC_NOT_PAUSED)
    {
        cinematicGlob.currentPaused = CINEMATIC_PAUSED;
        BinkPause(cinematicGlob.bink, 1);
    }

    for (setIter = 0; setIter != 2; ++setIter)
    {
        textureSet = &cinematicGlob.textureSets[setIter];
        Image_Release(&textureSet->drawImageY);
        Image_Release(&textureSet->drawImageCr);
        Image_Release(&textureSet->drawImageCb);
        Image_Release(&textureSet->drawImageA);
    }

    Sys_LeaveCriticalSection(CRITSECT_CINEMATIC);
}

void R_Cinematic_RelinquishIO()
{
    iassert(cinematicGlob.fileIoState != CIN_IOSTATE_RELINQUISHED);
    iassert(cinematicGlob.bink);

    BinkControlBackgroundIO(cinematicGlob.bink, 1);
    R_Cinematic_CheckBinkError();
    Sys_ResumeDiscReads(THREAD_OWNER_CINEMATICS);
    cinematicGlob.fileIoState = CIN_IOSTATE_RELINQUISHED;
}

const char *R_Cinematic_CheckBinkError()
{
    const char *result; // eax
    const char *v1; // eax

    result = (const char *)BinkGetError();
    if ( result && *result )
    {
        v1 = va("BinkGetError(): \"%s\"", result);
        result = (const char *)Assert_MyHandler(
                                                         "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp",
                                                         504,
                                                         0,
                                                         "%s\n\t%s",
                                                         "!binkError || binkError[0] == '\\0'",
                                                         v1);
        if ( !(_BYTE)result )
            __debugbreak();
    }
    return result;
}

void __cdecl R_Cinematic_EndLostDevice()
{
    Sys_EnterCriticalSection(CRITSECT_CINEMATIC);
    if ( cinematicGlob.bink )
    {
        R_Cinematic_MakeBinkDrawTextures();
        R_Cinematic_ClearBinkDrawTextures();
    }
    Sys_LeaveCriticalSection(CRITSECT_CINEMATIC);
}

void R_Cinematic_MakeBinkDrawTextures()
{
    bool useAlpha; // [esp+Bh] [ebp-9h]
    int frameIter; // [esp+Ch] [ebp-8h]
    CinematicTextureSet *textureSet; // [esp+10h] [ebp-4h]

    useAlpha = 0;
    for (frameIter = 0; frameIter != 2; ++frameIter)
    {
        if (cinematicGlob.binkTextureSet.bink_buffers.Frames[frameIter].APlane.Allocate)
            useAlpha = 1;
    }
    textureSet = &cinematicGlob.textureSets[cinematicGlob.activeTextureSet];
    cinematicGlob.binkTextureSet.tex_draw.Ytexture = R_Cinematic_MakeBinkTexture_PC(
        &textureSet->drawImageY,
        cinematicGlob.binkTextureSet.bink_buffers.YABufferWidth,
        cinematicGlob.binkTextureSet.bink_buffers.YABufferHeight,
        0x10000);
    cinematicGlob.binkTextureSet.tex_draw.cRtexture = R_Cinematic_MakeBinkTexture_PC(
        &textureSet->drawImageCr,
        cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferWidth,
        cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferHeight,
        0x10000);
    cinematicGlob.binkTextureSet.tex_draw.cBtexture = R_Cinematic_MakeBinkTexture_PC(
        &textureSet->drawImageCb,
        cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferWidth,
        cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferHeight,
        0x10000);

    if (useAlpha)
        cinematicGlob.binkTextureSet.tex_draw.Atexture = R_Cinematic_MakeBinkTexture_PC(
            &textureSet->drawImageA,
            cinematicGlob.binkTextureSet.bink_buffers.YABufferWidth,
            cinematicGlob.binkTextureSet.bink_buffers.YABufferHeight,
            0x10000);
    else
        cinematicGlob.binkTextureSet.tex_draw.Atexture = 0;

    if (!Sys_IsRenderThread())
    {
        RB_Resource_Flush();
        cinematicGlob.binkTextureSet.tex_draw.Ytexture = textureSet->drawImageY.texture.map;
        cinematicGlob.binkTextureSet.tex_draw.cRtexture = textureSet->drawImageCr.texture.map;
        cinematicGlob.binkTextureSet.tex_draw.cBtexture = textureSet->drawImageCb.texture.map;
        if (useAlpha)
        {
            cinematicGlob.binkTextureSet.tex_draw.Atexture = textureSet->drawImageA.texture.map;
        }
        else
        {
            cinematicGlob.binkTextureSet.tex_draw.Atexture = 0;
        }
    }
}

IDirect3DTexture9 *__cdecl R_Cinematic_MakeBinkTexture_PC(
                GfxImage *image,
                unsigned int width,
                unsigned int height,
                int baseImageFlags)
{
    if ( Sys_IsRenderThread() )
    {
        Image_Setup(image, width, height, 1, baseImageFlags | 3, D3DFMT_L8);
        return (IDirect3DTexture9 *)image->texture.basemap;
    }
    else
    {
        image->width = width;
        image->height = height;
        image->depth = 1;
        image->mapType = 3;
        RB_Resource_CreateTexture(image, 0, 1, baseImageFlags | 3, D3DFMT_L8);
        return 0;
    }
}

void R_Cinematic_ClearBinkDrawTextures()
{
    BINKFRAMETEXTURES *textures = &cinematicGlob.binkTextureSet.tex_draw;

    iassert(textures->Ytexture);
    iassert(textures->cRtexture);
    iassert(textures->cBtexture);
    
    R_Cinematic_ClearTexture(
        cinematicGlob.binkTextureSet.tex_draw.Ytexture,
        cinematicGlob.binkTextureSet.bink_buffers.YABufferWidth,
        cinematicGlob.binkTextureSet.bink_buffers.YABufferHeight,
        0);
    R_Cinematic_ClearTexture(
        cinematicGlob.binkTextureSet.tex_draw.cRtexture,
        cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferWidth,
        cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferHeight,
        0x80u);
    R_Cinematic_ClearTexture(
        cinematicGlob.binkTextureSet.tex_draw.cBtexture,
        cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferWidth,
        cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferHeight,
        0x80u);

    if (cinematicGlob.binkTextureSet.tex_draw.Atexture)
        R_Cinematic_ClearTexture(
            cinematicGlob.binkTextureSet.tex_draw.Atexture,
            cinematicGlob.binkTextureSet.bink_buffers.YABufferWidth,
            cinematicGlob.binkTextureSet.bink_buffers.YABufferHeight,
            0);
}

void __cdecl R_Cinematic_ClearTexture(IDirect3DTexture9 *texture, int width, int height, unsigned __int8 clearValue)
{
    const char *v4; // eax
    int hr; // [esp+0h] [ebp-Ch]
    _D3DLOCKED_RECT lockedRect; // [esp+4h] [ebp-8h] BYREF

    hr = texture->LockRect(0, &lockedRect, 0, 0x2000u);
    if ( hr >= 0 )
    {
        if ( lockedRect.Pitch < width
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp",
                        2172,
                        0,
                        "lockedRect.Pitch >= width\n\t%i, %i",
                        lockedRect.Pitch,
                        width) )
        {
            __debugbreak();
        }
        memset((unsigned __int8 *)lockedRect.pBits, clearValue, lockedRect.Pitch * height);
        texture->UnlockRect(0);
    }
    else
    {
        v4 = R_ErrorDescription(hr);
        Com_PrintError(8, "LockRect failed with error 0x%08x - %s", hr, v4);
    }
}

void __cdecl R_Cinematic_Init()
{
    iassert(!cinematicGlob.memPool);
    iassert(!g_cinematicInitialized);

    memset((unsigned __int8 *)&cinematicGlob, 0, 0x9F4u);
    cinematicGlob.activeImageFrame = -1;
    R_Cinematic_ReserveMemory();

    iassert(cinematicGlob.currentPaused == CINEMATIC_NOT_PAUSED);
    iassert(cinematicGlob.targetPaused == CINEMATIC_NOT_PAUSED);

    g_cinematicInitialized = 1;
    if ( !g_cinematicDS )
        R_CinematicInitSound(0);
}

unsigned int *R_Cinematic_ReserveMemory()
{
    unsigned int *result; // eax

    result = Z_Malloc(0x1800000, "R_Cinematic_ReserveMemory", 19);
    cinematicGlob.memPool = result;
    return result;
}

void __cdecl R_Cinematic_Shutdown()
{
    int setIter; // [esp+0h] [ebp-4h]

    R_Cinematic_StopPlayback();
    g_cinematicInitialized = 0;
    Sys_WaitWorkerCmdInternal(&_UpdateFrameWorkerCmd);
    for (setIter = 0; setIter != 2; ++setIter)
        R_Cinematic_ReleaseImages(&cinematicGlob.textureSets[setIter]);
    Z_Free((char*)cinematicGlob.memPool, 19);
    cinematicGlob.memPool = 0;
    if (g_cinematicDS)
    {
        g_cinematicDS->Release();
        g_cinematicDS = 0;
    }
}

void __cdecl R_Cinematic_ReleaseImages(CinematicTextureSet *textureSet)
{
    int frameIter; // [esp+0h] [ebp-4h]

    for ( frameIter = 0; frameIter != 2; ++frameIter )
    {
        Image_Release(&textureSet->imageY[frameIter]);
        Image_Release(&textureSet->imageCr[frameIter]);
        Image_Release(&textureSet->imageCb[frameIter]);
        Image_Release(&textureSet->imageA[frameIter]);
        Z_VirtualFree(textureSet->Ytexture[frameIter], 23);
        Z_VirtualFree(textureSet->cRtexture[frameIter], 23);
        Z_VirtualFree(textureSet->cBtexture[frameIter], 23);
        Z_VirtualFree(textureSet->Atexture[frameIter], 23);
    }
    Image_Release(&textureSet->drawImageY);
    Image_Release(&textureSet->drawImageCr);
    Image_Release(&textureSet->drawImageCb);
    Image_Release(&textureSet->drawImageA);
}

void __cdecl R_Cinematic_StartPlayback_Internal(const char *name, unsigned int playbackFlags, int volume)
{
    Sys_EnterCriticalSection(CRITSECT_CINEMATIC_TARGET_CHANGE);
    I_strncpyz(cinematicGlob.targetCinematicName, name, 256);
    cinematicGlob.targetCinematicChanged = 1;
    cinematicGlob.cinematicFinished = 0;
    cinematicGlob.targetPaused = (CinematicEnum)((playbackFlags & 0x80u) != 0);
    cinematicGlob.playbackFlags = playbackFlags;
    cinematicGlob.playbackVolume = volume;
    Sys_LeaveCriticalSection(CRITSECT_CINEMATIC_TARGET_CHANGE);
}

void __cdecl R_Cinematic_StartNextPlayback()
{
    Sys_EnterCriticalSection(CRITSECT_CINEMATIC);
    if ( R_Cinematic_IsNextReady_Internal() )
    {
        R_Cinematic_StartPlayback_Internal(
            cinematicGlob.nextCinematicName,
            cinematicGlob.nextCinematicPlaybackFlags,
            cinematicGlob.playbackVolume);
        cinematicGlob.nextCinematicName[0] = 0;
    }
    Sys_LeaveCriticalSection(CRITSECT_CINEMATIC);
}

bool __cdecl R_Cinematic_IsNextReady_Internal()
{
    return cinematicGlob.nextCinematicName[0] != 0;
}

void __cdecl R_Cinematic_StopPlayback()
{
    Sys_EnterCriticalSection(CRITSECT_CINEMATIC);
    Sys_EnterCriticalSection(CRITSECT_CINEMATIC_TARGET_CHANGE);
    cinematicGlob.targetCinematicName[0] = 0;
    cinematicGlob.targetCinematicChanged = 1;
    cinematicGlob.cinematicFinished = 0;
    Sys_LeaveCriticalSection(CRITSECT_CINEMATIC_TARGET_CHANGE);
    Sys_LeaveCriticalSection(CRITSECT_CINEMATIC);
}

int __cdecl _UpdateFrameCallback()
{
    R_Cinematic_UpdateFrame_Core(0);
    _InterlockedExchange(&cineThreadBlock, 0);
    return 0;
}

void __cdecl R_Cinematic_UpdateFrame_Core(bool force_wait)
{
    bool localTargetChanged; // [esp+3h] [ebp-111h]
    char localTargetCinematic[260]; // [esp+4h] [ebp-110h] BYREF
    unsigned int localTargetPlaybackFlags; // [esp+10Ch] [ebp-8h]
    bool isCinematicBeingPlayed; // [esp+113h] [ebp-1h]

    if (!r_cinematic_disabled)
    {
        Sys_EnterCriticalSection(CRITSECT_CINEMATIC_TARGET_CHANGE);
        localTargetChanged = cinematicGlob.targetCinematicChanged;
        if (cinematicGlob.targetCinematicChanged)
        {
            I_strncpyz(localTargetCinematic, cinematicGlob.targetCinematicName, 256);
            cinematicGlob.targetCinematicChanged = 0;
            localTargetPlaybackFlags = cinematicGlob.playbackFlags;
        }
        else
        {
            localTargetCinematic[0] = 0;
            localTargetPlaybackFlags = 0;
        }
        Sys_LeaveCriticalSection(CRITSECT_CINEMATIC_TARGET_CHANGE);
        if (!localTargetChanged
            && localTargetPlaybackFlags
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp",
                2858,
                0,
                "%s",
                "localTargetChanged || !localTargetPlaybackFlags"))
        {
            __debugbreak();
        }
        if (!localTargetChanged
            && localTargetCinematic
            && localTargetCinematic[0]
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp",
                2859,
                0,
                "%s",
                "localTargetChanged || !localTargetCinematic || !localTargetCinematic[0]"))
        {
            __debugbreak();
        }
        cinematicGlob.underrun = 0;
        if (localTargetChanged)
        {
            if (cinematicGlob.currentCinematicName[0])
                R_Cinematic_StopPlayback_Now();
            if (localTargetCinematic[0] && R_Cinematic_StartPlayback_Now(localTargetCinematic, localTargetPlaybackFlags))
            {
                I_strncpyz(cinematicGlob.currentCinematicName, localTargetCinematic, 256);
                cinematicGlob.targetCinematicName[0] = 0;
            }
            else
            {
                cinematicGlob.currentCinematicName[0] = 0;
            }
        }
        isCinematicBeingPlayed = cinematicGlob.currentCinematicName[0] != 0;
        if (cinematicGlob.currentCinematicName[0])
        {
            cinematicGlob.framesStopped = 0;
        }
        else
        {
            if (cinematicGlob.activeImageFrame != -1
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp",
                    2910,
                    0,
                    "%s",
                    "cinematicGlob.activeImageFrame == CINEMATIC_INVALID_IMAGE_FRAME"))
            {
                __debugbreak();
            }
            if (R_Cinematic_AreHunksOpen() && ++cinematicGlob.framesStopped >= 5)
                R_Cinematic_HunksClose();
        }
        if (isCinematicBeingPlayed)
        {
            if (!cinematicGlob.bink
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp",
                    2926,
                    0,
                    "%s",
                    "cinematicGlob.bink"))
            {
                __debugbreak();
            }
            if (!cinematicGlob.cinematicFinished && !R_Cinematic_Advance(force_wait))
            {
                SND_NotifyCinematicEnd();
                cinematicGlob.cinematicFinished = 1;
            }
        }
    }
}

char __cdecl R_Cinematic_AreHunksOpen()
{
    if ( CinematicHunk_IsOpen(&cinematicGlob.masterHunk) )
    {
        iassert(CinematicHunk_IsOpen(&cinematicGlob.binkHunk));
        iassert(CinematicHunk_IsOpen(&cinematicGlob.residentHunk));

        return 1;
    }
    else
    {
        iassert(!CinematicHunk_IsOpen(&cinematicGlob.binkHunk));
        iassert(!CinematicHunk_IsOpen(&cinematicGlob.residentHunk));

        return 0;
    }
}

char __cdecl CinematicHunk_IsOpen(CinematicHunk *hunk)
{
    if ( hunk->base )
    {
        iassert(hunk->atFront);
        iassert(hunk->atBack);
        iassert(hunk->end);
        
        return 1;
    }
    else
    {
        iassert(!hunk->atFront);
        iassert(!hunk->atBack);
        iassert(!hunk->end);
        
        return 0;
    }
}

void R_Cinematic_HunksClose()
{
    CinematicHunk_Close(&cinematicGlob.masterHunk);
    CinematicHunk_Close(&cinematicGlob.binkHunk);
    CinematicHunk_Close(&cinematicGlob.residentHunk);
}

void __cdecl CinematicHunk_Close(CinematicHunk *hunk)
{
    iassert(hunk->base);
    iassert(hunk->atFront);
    iassert(hunk->atBack);
    iassert(hunk->end);
    
    hunk->base = 0;
    hunk->atFront = 0;
    hunk->atBack = 0;
    hunk->end = 0;
    hunk->lastAllocPtr = 0;
    hunk->fragmented = 0;
}

char __cdecl R_Cinematic_Advance(bool force_wait)
{
    //jpeg_decompress_struct *v3; // [esp+0h] [ebp-E0h]
    bool v4; // [esp+20h] [ebp-C0h]
    bool v5; // [esp+24h] [ebp-BCh]
    BINKFRAMETEXTURES *bt; // [esp+8Ch] [ebp-54h]
    BINKFRAMEPLANESET *Frames; // [esp+90h] [ebp-50h]
    int i; // [esp+94h] [ebp-4Ch]
    int maxMsecToWait; // [esp+98h] [ebp-48h]
    CinematicEnum targetPaused; // [esp+9Ch] [ebp-44h]
    bool useCustomSkipLogic; // [esp+A2h] [ebp-3Eh]
    BINKREALTIME binkRealtime; // [esp+A4h] [ebp-3Ch] BYREF
    unsigned int percentageFull; // [esp+DCh] [ebp-4h]

    targetPaused = cinematicGlob.targetPaused;

    iassert((targetPaused == CINEMATIC_PAUSED || targetPaused == CINEMATIC_SCRIPT_PAUSED || targetPaused == CINEMATIC_NOT_PAUSED));
    iassert((cinematicGlob.currentPaused == CINEMATIC_PAUSED || cinematicGlob.currentPaused == CINEMATIC_SCRIPT_PAUSED || cinematicGlob.currentPaused == CINEMATIC_NOT_PAUSED));
    iassert(cinematicGlob.bink);
    iassert(!cinematicGlob.underrun);

    //BG_EvalVehicleName(v3);
    percentageFull = R_Cinematic_GetPercentageFull();
    if (percentageFull < 0x14 && (cinematicGlob.playbackFlags & 1) == 0 && (cinematicGlob.playbackFlags & 0x10) == 0)
    {
        cinematicGlob.underrun = 1;
        targetPaused = CINEMATIC_PAUSED;
    }
    if (targetPaused != cinematicGlob.currentPaused)
    {
        if (targetPaused == CINEMATIC_PAUSED || targetPaused == CINEMATIC_SCRIPT_PAUSED)
            BinkPause(cinematicGlob.bink, 1);
        else
            BinkPause(cinematicGlob.bink, 0);
        R_Cinematic_CheckBinkError();
        cinematicGlob.currentPaused = targetPaused;
        if (targetPaused == CINEMATIC_NOT_PAUSED)
        {
            cinematicGlob.startTimeMS = Sys_Milliseconds();
            cinematicGlob.frameNum = cinematicGlob.bink->FrameNum;
        }
    }
    v5 = force_wait || (cinematicGlob.playbackFlags & 0x40) != 0 && (cinematicGlob.playbackFlags & 1) != 0;
    v4 = (cinematicGlob.playbackFlags & 0x40) != 0 && (cinematicGlob.playbackFlags & 1) != 0;
    useCustomSkipLogic = v4;
    if ((cinematicGlob.playbackFlags & 2) != 0)
        useCustomSkipLogic = 1;
    if (targetPaused == CINEMATIC_NOT_PAUSED)
    {
        if (v5)
        {
            maxMsecToWait = 33;
            while (R_Cinematic_BinkShouldWait(useCustomSkipLogic))
            {
                if (!maxMsecToWait--)
                    break;
                NET_Sleep(1);
            }
        }
        if (force_wait || !R_Cinematic_BinkShouldWait(useCustomSkipLogic))
        {
            bt = cinematicGlob.binkTextureSet.textures;
            Frames = cinematicGlob.binkTextureSet.bink_buffers.Frames;
            for (i = 0; i < cinematicGlob.binkTextureSet.bink_buffers.TotalFrames; ++i)
            {
                Frames->YPlane.Buffer = cinematicGlob.YTextures[i];
                Frames->YPlane.BufferPitch = cinematicGlob.binkTextureSet.bink_buffers.YABufferWidth;
                Frames->cRPlane.Buffer = cinematicGlob.crTextures[i];
                Frames->cRPlane.BufferPitch = cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferWidth;
                Frames->cBPlane.Buffer = cinematicGlob.cbTextures[i];
                Frames->cBPlane.BufferPitch = cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferWidth;
                if (bt->Atexture)
                {
                    Frames->APlane.Buffer = cinematicGlob.aTextures[i];
                    Frames->APlane.BufferPitch = cinematicGlob.binkTextureSet.bink_buffers.YABufferWidth;
                }
                ++bt;
                ++Frames;
            }
            {
                PROF_SCOPED("R_Cinematic_Thread2");
                BinkDoFrame(cinematicGlob.bink);
                while (R_Cinematic_BinkShouldSkip(useCustomSkipLogic)
                    && ((cinematicGlob.playbackFlags & 2) != 0 || cinematicGlob.bink->FrameNum != cinematicGlob.bink->Frames))
                {
                    if (cinematicGlob.bink->FrameNum == 1)
                    {
                        cinematicGlob.startTimeMS = Sys_Milliseconds();
                        cinematicGlob.frameNum = cinematicGlob.bink->FrameNum;
                    }
                    BinkNextFrame(cinematicGlob.bink);
                    BinkDoFrame(cinematicGlob.bink);
                }
            }
            
            if (Sys_IsRenderThread())
            {
                Unlock_Bink_textures2(dx.device, &cinematicGlob.binkTextureSet);
            }
            else
            {
                RB_Resource_Callback(Unlock_Bink_textures_Callback);
                RB_Resource_Flush();
            }
            if (cinematicGlob.activeImageFrame == -1)
                cinematicGlob.firstFrameNotify = 1;
            if (cinematicGlob.binkTextureSet.bink_buffers.FrameNum >= 2
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp",
                    2076,
                    0,
                    "%s\n\t(cinematicGlob.binkTextureSet.bink_buffers.FrameNum) = %i",
                    "(cinematicGlob.binkTextureSet.bink_buffers.FrameNum == 0 || cinematicGlob.binkTextureSet.bink_buffers.FrameNum == 1)",
                    cinematicGlob.binkTextureSet.bink_buffers.FrameNum))
            {
                __debugbreak();
            }
            cinematicGlob.activeImageFrame = cinematicGlob.binkTextureSet.bink_buffers.FrameNum;
            cinematicGlob.activeImageFrameTextureSet = cinematicGlob.activeTextureSet;
            if ((cinematicGlob.playbackFlags & 2) != 0 || cinematicGlob.bink->FrameNum != cinematicGlob.bink->Frames)
                BinkNextFrame(cinematicGlob.bink);
        }
    }
    R_Cinematic_CheckBinkError();
    if ((cinematicGlob.playbackFlags & 2) != 0 || cinematicGlob.bink->FrameNum != cinematicGlob.bink->Frames)
    {
        if (!cinematicGlob.bink
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp",
                2098,
                0,
                "%s",
                "cinematicGlob.bink"))
        {
            __debugbreak();
        }
        BinkGetRealtime(cinematicGlob.bink, &binkRealtime, 0);
        R_Cinematic_UpdateTimeInMsec(&binkRealtime);
        if ((cinematicGlob.playbackFlags & 8) != 0)
        {
            percentageFull = 100;
        }
        else if (binkRealtime.ReadBufferUsed < binkRealtime.ReadBufferSize)
        {
            percentageFull = (__int64)((double)binkRealtime.ReadBufferUsed / (double)cinematicGlob.binkIOSize * 100.0);
        }
        else
        {
            percentageFull = 100;
        }
        if (cinematicGlob.fileIoState && percentageFull > 0x5F)
            R_Cinematic_RelinquishIO();
        if (cinematicGlob.fileIoState != CIN_IOSTATE_SEIZED && percentageFull < 0x32)
        {
            if ((cinematicGlob.playbackFlags & 8) != 0
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp",
                    2114,
                    0,
                    "%s",
                    "!(cinematicGlob.playbackFlags & CINEMATIC_PLAYBACKFLAGS_MEMORY_RESIDENT)"))
            {
                __debugbreak();
            }
            R_Cinematic_SeizeIO();
        }
        return 1;
    }
    else
    {
        cinematicGlob.lastFrameNotify = 1;
        if (cinematicGlob.fileIoState)
            R_Cinematic_RelinquishIO();
        return 0;
    }
}

unsigned int __cdecl R_Cinematic_GetPercentageFull()
{
    BINKREALTIME binkRealtime; // [esp+24h] [ebp-38h] BYREF

    if ( !cinematicGlob.bink
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp",
                    1445,
                    0,
                    "%s",
                    "cinematicGlob.bink") )
    {
        __debugbreak();
    }
    BinkGetRealtime(cinematicGlob.bink, &binkRealtime, 0);
    if ( (cinematicGlob.playbackFlags & 8) != 0 )
        return 100;
    if ( binkRealtime.ReadBufferUsed < binkRealtime.ReadBufferSize )
        return (__int64)((double)binkRealtime.ReadBufferUsed / (double)cinematicGlob.binkIOSize * 100.0);
    return 100;
}

const char *R_Cinematic_SeizeIO()
{
    const char *result; // eax

    iassert(cinematicGlob.fileIoState != CIN_IOSTATE_SEIZED);
    iassert(cinematicGlob.bink);

    if ( !cinematicGlob.fileIoState )
        Sys_SuspendDiscReads(THREAD_OWNER_CINEMATICS);

    BinkControlBackgroundIO(cinematicGlob.bink, 2);
    result = R_Cinematic_CheckBinkError();
    cinematicGlob.fileIoState = CIN_IOSTATE_SEIZED;
    return result;
}

void __cdecl R_Cinematic_UpdateTimeInMsec(const BINKREALTIME *binkRealtime)
{
    const char *v1; // eax
    unsigned __int64 frameNum; // [esp+8h] [ebp-20h]
    unsigned __int64 frameRateDiv; // [esp+10h] [ebp-18h]
    unsigned __int64 timeInMsec; // [esp+18h] [ebp-10h]
    unsigned __int64 frameRate; // [esp+20h] [ebp-8h]

    frameNum = binkRealtime->FrameNum;
    if ( (unsigned int)frameNum < 0x80000000 )
    {
        frameRateDiv = binkRealtime->FrameRateDiv;
        frameRate = binkRealtime->FrameRate;
        timeInMsec = 1000 * frameNum * frameRateDiv / frameRate;
        cinematicGlob.timeInMsec = timeInMsec;
        if ( timeInMsec != (unsigned int)timeInMsec )
        {
            v1 = va(
                         "%08x:%08x, %08x:%08x, %08x:%08x, %08x:%08x",
                         HIDWORD(timeInMsec),
                         (unsigned int)timeInMsec,
                         HIDWORD(frameNum),
                         (unsigned int)frameNum,
                         HIDWORD(frameRate),
                         (unsigned int)frameRate,
                         HIDWORD(frameRateDiv),
                         (unsigned int)frameRateDiv);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp",
                            1518,
                            0,
                            "%s\n\t%s",
                            "cinematicGlob.timeInMsec == timeInMsec",
                            v1) )
                __debugbreak();
        }
    }
    else
    {
        cinematicGlob.timeInMsec = 0;
    }
}

int __cdecl R_Cinematic_BinkShouldSkip(bool useCustomLogic)
{
    float bink; // [esp+14h] [ebp-8h]

    if ( !useCustomLogic )
        return BinkShouldSkip(cinematicGlob.bink);
    bink = (float)(int)cinematicGlob.bink;
    return *(unsigned int *)(cinematicGlob.bink + 12) < (unsigned int)(__int64)((double)(int)(Sys_Milliseconds() - *(unsigned int *)&cinematicGlob.usingAlpha) * *(float *)&cinematicGlob.fileIoState / 1000.0 + bink);
}

int __cdecl R_Cinematic_BinkShouldWait(bool useCustomLogic)
{
    float bink; // [esp+14h] [ebp-8h]

    if ( !useCustomLogic )
        return BinkWait(cinematicGlob.bink);

    bink = (float)(int)cinematicGlob.bink;
    return (unsigned int)(__int64)((double)(int)(Sys_Milliseconds() + 16 - *(unsigned int *)&cinematicGlob.usingAlpha)
                                                             * *(float *)&cinematicGlob.fileIoState
                                                             / 1000.0
                                                             + bink) < *(unsigned int *)(cinematicGlob.bink + 12);
}

void R_Cinematic_StopPlayback_Now()
{
    SND_NotifyCinematicEnd();
    cinematicGlob.activeImageFrame = -1;
    if (cinematicGlob.fileIoState)
        R_Cinematic_RelinquishIO();
    cinematicGlob.isPreloaded = 0;
    BinkClose(cinematicGlob.bink);
    cinematicGlob.bink = 0;
    if (cinematicGlob.binkHunk.base)
        CinematicHunk_Reset(&cinematicGlob.binkHunk);
}

void __cdecl CinematicHunk_Reset(CinematicHunk *hunk)
{
    iassert(hunk->base);
    iassert(hunk->atFront);
    iassert(hunk->atBack);
    iassert(hunk->end);
    
    hunk->atFront = hunk->base;
    hunk->atBack = hunk->end;
    hunk->lastAllocPtr = 0;
    hunk->fragmented = 0;
}

char __cdecl R_Cinematic_StartPlayback_Now(const char *filename, unsigned int playbackFlags)
{
    unsigned int TrackIDsToPlay[5]; // [esp+1Ch] [ebp-9Ch] BYREF
    char errText[132]; // [esp+30h] [ebp-88h] BYREF

    TrackIDsToPlay[0] = 0;
    TrackIDsToPlay[1] = 1;
    TrackIDsToPlay[2] = 2;
    TrackIDsToPlay[3] = 3;
    TrackIDsToPlay[4] = 4;
    if (cinematicGlob.bink
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp",
            2641,
            0,
            "%s",
            "!cinematicGlob.bink"))
    {
        __debugbreak();
    }
    cinematicGlob.activeImageFrame = -1;
    cinematicGlob.activeTextureSet ^= 1u;
    cinematicGlob.firstFrameNotify = 0;
    cinematicGlob.lastFrameNotify = 0;
    if (R_Cinematic_AreHunksOpen())
    {
        R_Cinematic_HunksReset(cinematicGlob.activeTextureSet, playbackFlags);
    }
    else if (!R_Cinematic_HunksOpen(cinematicGlob.activeTextureSet, playbackFlags))
    {
        cinematicGlob.activeTextureSet ^= 1u;
        cinematicGlob.targetCinematicChanged = 1;
        return 0;
    }
    SND_NotifyCinematicStart(cinematicGlob.playbackVolume);
    if (!CinematicHunk_IsEmpty(&cinematicGlob.binkHunk)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp",
            2666,
            0,
            "%s",
            "CinematicHunk_IsEmpty( &cinematicGlob.binkHunk )"))
    {
        __debugbreak();
    }
    R_Cinematic_CheckBinkError();
    BinkSetMemory(R_Cinematic_Bink_Alloc, R_Cinematic_Bink_Free);
    R_Cinematic_CheckBinkError();
    if (g_cinematicDS)
        BinkSetSoundSystem(BinkOpenDirectSound, (unsigned int)g_cinematicDS);
    R_Cinematic_CheckBinkError();
    R_Cinematic_CheckBinkError();
    BinkSetSoundTrack(5, TrackIDsToPlay);
    R_Cinematic_CheckBinkError();
    cinematicGlob.timeInMsec = 0;
    errText[0] = 0;
    if (R_Cinematic_BinkOpen(filename, playbackFlags, errText, 128)
        || (Com_PrintWarning(8, "R_Cinematic_BinkOpen '%s' failed: %s; trying default.\n", filename, errText),
            CinematicHunk_Reset(&cinematicGlob.binkHunk),
            errText[0] = 0,
            R_Cinematic_BinkOpen("default", playbackFlags, errText, 128)))
    {
        if (Sys_IsRenderThread())
        {
            R_Cinematic_BlackRendererImages();
        }
        else
        {
            RB_Resource_Callback(R_Cinematic_BlackRendererImages);
            RB_Resource_Flush();
        }
        R_Cinematic_CheckBinkError();
        if (!cinematicGlob.bink
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp",
                2746,
                0,
                "%s",
                "cinematicGlob.bink"))
        {
            __debugbreak();
        }
        R_Cinematic_InitBinkVolumes();
        memset((unsigned __int8 *)&cinematicGlob.binkTextureSet, 0, 0xD8u);
        BinkGetFrameBuffersInfo(cinematicGlob.bink, &cinematicGlob.binkTextureSet.bink_buffers);
        R_Cinematic_CheckBinkError();
        R_Cinematic_InitBinkTextures();
        BinkRegisterFrameBuffers(cinematicGlob.bink, &cinematicGlob.binkTextureSet.bink_buffers);
        R_Cinematic_CheckBinkError();
        cinematicGlob.currentPaused = CINEMATIC_NOT_PAUSED;
        return 1;
    }
    else
    {
        Com_PrintWarning(8, "R_Cinematic_BinkOpen '%s' failed: %s; not playing movie.\n", "default", errText);
        if (cinematicGlob.activeImageFrame != -1
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp",
                2701,
                0,
                "%s",
                "cinematicGlob.activeImageFrame == CINEMATIC_INVALID_IMAGE_FRAME"))
        {
            __debugbreak();
        }
        CinematicHunk_Reset(&cinematicGlob.binkHunk);
        R_Cinematic_BlackRendererImages();
        return 0;
    }
}

bool __cdecl CinematicHunk_IsEmpty(CinematicHunk *hunk)
{
    if ( !hunk->base
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp", 574, 0, "%s", "hunk->base") )
    {
        __debugbreak();
    }
    if ( !hunk->atFront
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp", 575, 0, "%s", "hunk->atFront") )
    {
        __debugbreak();
    }
    if ( !hunk->atBack
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp", 576, 0, "%s", "hunk->atBack") )
    {
        __debugbreak();
    }
    if ( !hunk->end
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp", 577, 0, "%s", "hunk->end") )
    {
        __debugbreak();
    }
    return hunk->atFront == hunk->base && hunk->atBack == hunk->end;
}

char __cdecl R_Cinematic_HunksOpen(int activeTexture, char playbackFlags)
{
    CinematicHunk_Open(&cinematicGlob.masterHunk, (char*)cinematicGlob.memPool, 0x1800000);
    R_Cinematic_HunksAllocate(activeTexture, playbackFlags);
    return 1;
}

void __cdecl CinematicHunk_Open(CinematicHunk *hunk, char *memory, int size)
{
    iassert(!hunk->base);
    iassert(!hunk->atFront);
    iassert(!hunk->atBack);
    iassert(!hunk->end);

    hunk->base = memory;
    hunk->atFront = memory;
    hunk->end = &memory[size];
    hunk->atBack = hunk->end;
    hunk->lastAllocPtr = 0;
    hunk->fragmented = 0;
}

void __cdecl R_Cinematic_HunksAllocate(int activeTexture, char playbackFlags)
{
    char *residentBufferBase; // [esp+8h] [ebp-1Ch]
    int newResidentBufferSize; // [esp+14h] [ebp-10h]
    int newBinkBufferSize; // [esp+18h] [ebp-Ch]
    char *binkBufferBase; // [esp+1Ch] [ebp-8h]

    newBinkBufferSize = 0x1800000;
    newResidentBufferSize = 0;
    if ((playbackFlags & 8) != 0)
    {
        newResidentBufferSize = 0x1680000;
        newBinkBufferSize = 0x180000;
    }
    binkBufferBase = (char *)CinematicHunk_Alloc(&cinematicGlob.masterHunk, newBinkBufferSize);
    residentBufferBase = (char *)CinematicHunk_Alloc(&cinematicGlob.masterHunk, newResidentBufferSize);
    CinematicHunk_Open(&cinematicGlob.binkHunk, binkBufferBase, newBinkBufferSize);
    CinematicHunk_Open(&cinematicGlob.residentHunk, residentBufferBase, newResidentBufferSize);
}

int __cdecl CinematicHunk_Alloc(CinematicHunk *hunk, int size)
{
    const char *v2; // eax
    char *alloced; // [esp+0h] [ebp-4h]

    if ( !hunk->base
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp", 593, 0, "%s", "hunk->base") )
    {
        __debugbreak();
    }
    if ( !hunk->atFront
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp", 594, 0, "%s", "hunk->atFront") )
    {
        __debugbreak();
    }
    if ( !hunk->atBack
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp", 595, 0, "%s", "hunk->atBack") )
    {
        __debugbreak();
    }
    if ( !hunk->end
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp", 596, 0, "%s", "hunk->end") )
    {
        __debugbreak();
    }
    if ( size < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp", 597, 0, "%s", "size >= 0") )
    {
        __debugbreak();
    }
    alloced = (char *)hunk->atFront;
    hunk->atFront = &alloced[size];
    if ( hunk->atFront <= hunk->atBack )
    {
        if ( hunk->fragmented )
        {
            Com_PrintWarning(0, "Bink allocating from fragmented memory at 0x%x.\n", hunk->atFront);
            hunk->fragmented = 0;
        }
        if ( hunk->atFront > hunk->atBack
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp",
                        613,
                        0,
                        "%s",
                        "hunk->atFront <= hunk->atBack") )
        {
            __debugbreak();
        }
        hunk->lastAllocPtr = alloced;
        return (int)alloced;
    }
    else
    {
        v2 = va("CinematicHunk_Alloc failed: 0x%08x 0x%08x 0x%08x\n", hunk->atFront, size, hunk->atBack);
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp", 603, 0, v2) )
            __debugbreak();
        return -1;
    }
}

void __cdecl R_Cinematic_HunksReset(int activeTexture, char playbackFlags)
{
    CinematicHunk_Close(&cinematicGlob.binkHunk);
    CinematicHunk_Close(&cinematicGlob.residentHunk);
    CinematicHunk_Reset(&cinematicGlob.masterHunk);
    R_Cinematic_HunksAllocate(activeTexture, playbackFlags);
}

void R_Cinematic_InitBinkTextures()
{
    BINKFRAMETEXTURES *textures; // [esp+0h] [ebp-10h]
    int frameIter; // [esp+8h] [ebp-8h]
    int frameItera; // [esp+8h] [ebp-8h]
    CinematicTextureSet *textureSet; // [esp+Ch] [ebp-4h]

    textureSet = &cinematicGlob.textureSets[cinematicGlob.activeTextureSet];
    R_Cinematic_ReleaseImages(textureSet);
    if (cinematicGlob.binkTextureSet.bink_buffers.TotalFrames != 2
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp",
            1214,
            0,
            "%s",
            "cinematicGlob.binkTextureSet.bink_buffers.TotalFrames == CINEMATIC_IMAGES_REQUIRED"))
    {
        __debugbreak();
    }
    for (frameIter = 0; frameIter != 2; ++frameIter)
    {
        textures = &cinematicGlob.binkTextureSet.textures[frameIter];
        textures->Ytexture = R_Cinematic_MakeBinkTexture_PC(
            &textureSet->imageY[frameIter],
            cinematicGlob.binkTextureSet.bink_buffers.YABufferWidth,
            cinematicGlob.binkTextureSet.bink_buffers.YABufferHeight,
            0x40000);
        textures->cRtexture = R_Cinematic_MakeBinkTexture_PC(
            &textureSet->imageCr[frameIter],
            cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferWidth,
            cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferHeight,
            0x40000);
        textures->cBtexture = R_Cinematic_MakeBinkTexture_PC(
            &textureSet->imageCb[frameIter],
            cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferWidth,
            cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferHeight,
            0x40000);
        if (cinematicGlob.binkTextureSet.bink_buffers.Frames[frameIter].APlane.Allocate)
            textures->Atexture = R_Cinematic_MakeBinkTexture_PC(
                &textureSet->imageA[frameIter],
                cinematicGlob.binkTextureSet.bink_buffers.YABufferWidth,
                cinematicGlob.binkTextureSet.bink_buffers.YABufferHeight,
                0x40000);
        else
            textures->Atexture = 0;

        cinematicGlob.bufferWidth = cinematicGlob.binkTextureSet.bink_buffers.YABufferWidth;

        textureSet->Ytexture[frameIter] = Z_VirtualAlloc(
            cinematicGlob.binkTextureSet.bink_buffers.YABufferHeight
            * cinematicGlob.binkTextureSet.bink_buffers.YABufferWidth,
            "BinkBuffers",
            23);
        cinematicGlob.YTextures[frameIter] = textureSet->Ytexture[frameIter];

        textureSet->cRtexture[frameIter] = Z_VirtualAlloc(
            cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferHeight
            * cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferWidth,
            "BinkBuffers",
            23);
        cinematicGlob.crTextures[frameIter] = textureSet->cRtexture[frameIter];
        textureSet->cBtexture[frameIter] = Z_VirtualAlloc(
            cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferHeight
            * cinematicGlob.binkTextureSet.bink_buffers.cRcBBufferWidth,
            "BinkBuffers",
            23);
        cinematicGlob.cbTextures[frameIter] = textureSet->cBtexture[frameIter];


        if (cinematicGlob.binkTextureSet.bink_buffers.Frames[frameIter].APlane.Allocate)
        {
            textureSet->Atexture[frameIter] = Z_VirtualAlloc(
                cinematicGlob.binkTextureSet.bink_buffers.YABufferHeight
                * cinematicGlob.binkTextureSet.bink_buffers.YABufferWidth,
                "BinkBuffers",
                23);
            cinematicGlob.aTextures[frameIter] = textureSet->Atexture[frameIter];
        }
        else
        {
            textureSet->Atexture[frameIter] = 0;
            cinematicGlob.aTextures[frameIter] = 0;
        }
    }
    if (!Sys_IsRenderThread())
    {
        RB_Resource_Flush();
        for (frameItera = 0; frameItera != 2; ++frameItera)
        {
            textures = &cinematicGlob.binkTextureSet.textures[frameItera];
            textures->Ytexture = textureSet->imageY[frameItera].texture.map;
            textures->cRtexture = textureSet->imageCr[frameItera].texture.map;
            textures->cBtexture = textureSet->imageCb[frameItera].texture.map;
            if (cinematicGlob.binkTextureSet.bink_buffers.Frames[frameItera].APlane.Allocate)
                textures->Atexture = textureSet->imageA[frameItera].texture.map;
            else
                textures->Atexture = 0;
        }
    }
    R_Cinematic_MakeBinkDrawTextures();
}

void __cdecl R_Cinematic_BlackRendererImages()
{
    if (Sys_IsRenderThread())
    {
        RB_UnbindAllImages();
    }
    else
    {
        RB_Resource_Callback(RB_UnbindAllImages);
        RB_Resource_Flush();
    }

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_Y] = rgp.blackImage;
    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_CR] = rgp.grayImage;
    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_CB] = rgp.grayImage;
    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_A] = rgp.blackImage;

    iassert(gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_CR]);
    iassert(gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_CB]);
    iassert(gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_A]);
    iassert(gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_Y]);
}

void *__stdcall R_Cinematic_Bink_Alloc(unsigned int bytes)
{
    return malloc(bytes);
}

void __stdcall R_Cinematic_Bink_Free(void *ptr)
{
    char v1; // [esp+3h] [ebp-1h]

    if (cinematicGlob.binkHunk.lastAllocPtr == ptr)
    {
        cinematicGlob.binkHunk.atFront = cinematicGlob.binkHunk.lastAllocPtr;
        cinematicGlob.binkHunk.lastAllocPtr = 0;
        v1 = 1;
    }
    else
    {
        v1 = 0;
    }
    if (!v1)
        cinematicGlob.binkHunk.fragmented = 1;
}

bool __cdecl R_Cinematic_BinkOpen(const char *filename, char playbackFlags, char *errText, unsigned int errTextSize)
{
    char *cwd; // [esp+4h] [ebp-20Ch]
    char filepath[2][256]; // [esp+8h] [ebp-208h] BYREF

    cwd = Sys_Cwd();
    if ( !cwd && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp", 2479, 0, "%s", "cwd") )
        __debugbreak();
    filepath[1][0] = 0;
    if ( (playbackFlags & 8) != 0 )
    {
        _snprintf(filepath[0], 0x100u, "bik/%s.%s", filename, "bik");
    }
    else
    {
        _snprintf(filepath[0], 0x100u, "%s\\main\\video\\%s.%s", cwd, filename, "bik");
        _snprintf(filepath[1], 0x100u, "%s\\raw\\video\\%s.%s", cwd, filename, "bik");
    }
    if ( R_Cinematic_BinkOpenPath(filepath[0], playbackFlags, errText, errTextSize) )
        return 1;
    return filepath[1][0] && R_Cinematic_BinkOpenPath(filepath[1], playbackFlags, errText, errTextSize);
}

char __cdecl R_Cinematic_BinkOpenPath(
    const char *filepath,
    char playbackFlags,
    char *errText,
    unsigned int errTextSize)
{
    const char *Error; // eax
    const char *v6; // eax
    RawFile *rawfile; // [esp+54h] [ebp-Ch]

    if ((playbackFlags & 8) != 0)
    {
        rawfile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, (char*)filepath, 0, 8000).rawfile;
        if (!rawfile)
        {
            if (errText)
                _snprintf(errText, errTextSize, "Couldn't find rawfile '%s' in db", filepath);
            return 0;
        }
        cinematicGlob.bink = (BINK *)BinkOpen(rawfile->buffer, 68174848);
        if (!cinematicGlob.bink)
            CinematicHunk_Reset(&cinematicGlob.residentHunk);
    }
    else
    {
        if (CinematicHunk_GetFreeSpace(&cinematicGlob.binkHunk) <= 1572864
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp",
                2418,
                0,
                "%s",
                "CinematicHunk_GetFreeSpace( &cinematicGlob.binkHunk ) > BINK_MISC_BUFFER_SIZE"))
        {
            __debugbreak();
        }
        cinematicGlob.binkIOSize = CinematicHunk_GetFreeSpace(&cinematicGlob.binkHunk) - 1572864;
        BinkSetIOSize(cinematicGlob.binkIOSize);
        cinematicGlob.bink = (BINK *)BinkOpen(filepath, 0x1104400);
        if (!cinematicGlob.bink)
        {
            Error = (const char *)BinkGetError();
            Com_Printf(16, "BinkOpen failed on %s because: %s\n", filepath, Error);
        }
        cinematicGlob.isPreloaded = 0;
        if (cinematicGlob.bink)
        {
            cinematicGlob.fileIoState = CIN_IOSTATE_RELINQUISHED;
            BinkPause(cinematicGlob.bink, 1);
            R_Cinematic_SeizeIO();
            while (cinematicGlob.fileIoState && R_Cinematic_GetPercentageFull() < 0xA)
                NET_Sleep(1);
            cinematicGlob.isPreloaded = 1;
            BinkPause(cinematicGlob.bink, 0);
            while (BinkWait(cinematicGlob.bink))
                NET_Sleep(1);
        }
    }
    if (cinematicGlob.bink)
    {
        cinematicGlob.framerateSpeedFactor = (double)cinematicGlob.bink->FrameRate / (double)cinematicGlob.bink->FrameRateDiv;
        return 1;
    }
    else
    {
        if (errText)
        {
            v6 = (const char *)BinkGetError();
            _snprintf(errText, errTextSize, "BinkOpen: %s", v6);
        }
        return 0;
    }
}

int __cdecl CinematicHunk_GetFreeSpace(CinematicHunk *hunk)
{
    iassert(hunk->base);
    iassert(hunk->atFront);
    iassert(hunk->atBack);
    iassert(hunk->end);
    
    return (char *)hunk->atBack - (char *)hunk->atFront;
}

void __cdecl R_Cinematic_InitBinkVolumes()
{
    int v0; // [esp+0h] [ebp-20h]
    int v1; // [esp+14h] [ebp-Ch]

    if ( (int)(__int64)(32768.0 * *(float *)&cinematicGlob.playbackVolume * 0.70700002) < 0x8000 )
        v1 = (__int64)(32768.0 * *(float *)&cinematicGlob.playbackVolume * 0.70700002);
    else
        v1 = 0x8000;
    if ( v1 > 0 )
        v0 = v1;
    else
        v0 = 0;
    BinkSetVolume(cinematicGlob.bink, 0, v0);
    BinkSetVolume(cinematicGlob.bink, 1, v0);
    BinkSetVolume(cinematicGlob.bink, 2, 0);
    BinkSetVolume(cinematicGlob.bink, 3, 0);
    BinkSetVolume(cinematicGlob.bink, 4, 0);
}

void __cdecl R_Cinematic_UpdateFrame(bool force_wait)
{
    CINE_JQ_CMD cmd; // [esp+14h] [ebp-8h] BYREF

    PROF_SCOPED("R_Cinematic_UpdateFrame");

    if ( g_cinematicInitialized )
    {
        do
        {
            if ( !_InterlockedExchange(&cineThreadBlock, 1) )
            {
                force_wait = 0;
                R_Cinematic_UpdateRendererImages();
                Sys_AddWorkerCmdInternal(&_UpdateFrameWorkerCmd, (unsigned __int8 *)&cmd, 0);
            }
        }
        while ( force_wait );
    }
}

void R_Cinematic_UpdateRendererImages()
{
    if ( cinematicGlob.activeImageFrame == -1 )
        R_Cinematic_BlackRendererImages();
    else
        R_Cinematic_SetRendererImagesToFrame(cinematicGlob.activeImageFrame);
}

void __cdecl R_Cinematic_SetRendererImagesToFrame(int frameToSetTo)
{
    CinematicTextureSet *textureSet; // [esp+0h] [ebp-4h]

    textureSet = &cinematicGlob.textureSets[cinematicGlob.activeImageFrameTextureSet];

    iassert(frameToSetTo != CINEMATIC_INVALID_IMAGE_FRAME);

    if (Sys_IsRenderThread())
    {
        RB_UnbindAllImages();
    }
    else
    {
        RB_Resource_Callback(RB_UnbindAllImages);
        RB_Resource_Flush();
    }

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_Y] = &textureSet->drawImageY;
    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_CR] = &textureSet->drawImageCr;
    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_CB] = &textureSet->drawImageCb;

    if (textureSet->drawImageA.texture.basemap)
        gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_A] = &textureSet->drawImageA;
    else
        gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_CINEMATIC_A] = rgp.whiteImage;
}

char __cdecl R_Cinematic_GetFilenameAndTimeInMsec(char *outName, int outNameSize, unsigned int *outTimeInMsec)
{
    if ( !outTimeInMsec
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp", 3248, 0, "%s", "outTimeInMsec") )
    {
        __debugbreak();
    }
    if ( !outName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_cinematic.cpp", 3249, 0, "%s", "outName") )
    {
        __debugbreak();
    }
    if ( !R_Cinematic_IsStarted() )
        return 0;
    *outTimeInMsec = cinematicGlob.timeInMsec;
    I_strncpyz(outName, cinematicGlob.currentCinematicName, outNameSize);
    return 1;
}

bool __cdecl R_Cinematic_IsFinished()
{
    return cinematicGlob.cinematicFinished;
}

bool __cdecl R_Cinematic_IsStarted()
{
    return !R_Cinematic_IsFinished() && cinematicGlob.currentCinematicName[0];
}

bool __cdecl R_Cinematic_IsNextReady()
{
    return R_Cinematic_IsNextReady_Internal();
}

char __cdecl R_Cinematic_IsUnderrun()
{
    return cinematicGlob.underrun;
}

void __cdecl R_Cinematic_ForceRelinquishIO()
{
    if (cinematicGlob.fileIoState)
        R_Cinematic_RelinquishIO();
}

