//============================================================================
// stubs_bink_nvapi.cpp
//
// Deferred / unavailable third-party libraries on Linux.
//
// Bink (RAD Game Tools video codec) and NvAPI (NVIDIA driver/stereo API) ship
// only as Windows (and console) binaries -- there are no Linux libraries for
// either. The decompiled engine still references their exported symbols, so to
// let the engine link and run on Linux we provide do-nothing stub definitions
// here that return safe "not available" values.
//
// Consequences:
//   * Bink video playback is disabled. BinkOpen() returns a null HBINK, which
//     is the documented way for callers to detect that a movie could not be
//     opened, so the cinematic code simply skips video. The remaining entry
//     points are no-ops so any incidental calls are harmless.
//   * NvAPI / NVIDIA 3D Vision stereo is reported as unavailable. NvAPI_Initialize
//     returns a non-success status (NVAPI_ERROR), the stereo handle helpers do
//     nothing, and IsActivated reports stereo off, so the renderer runs mono.
//
// If/when real Linux back-ends become available these stubs should be replaced.
//============================================================================

#include <binklib/bink.h>

// nvapi.h gates its stereo/D3D-interop prototypes (including
// NvAPI_Stereo_CreateHandleFromIUnknown, which takes an IUnknown*) behind
// `#if defined(_D3D9_H_)`. The real renderer translation units include <d3d9.h>
// before <nvapi/nvapi.h>, so that block is active and the function is declared
// with C linkage inside nvapi.h's `extern "C"` block. We must do the same here,
// otherwise the declaration is skipped and our definition would get C++ linkage
// (mangled name) and fail to satisfy the call site at link time. Including the
// portable d3d9.h shim both defines _D3D9_H_ and provides the IUnknown type.
#include <d3d9.h>
#include <nvapi/nvapi.h>

//============================================================================
// Bink stubs (C linkage via the RADDEFSTART/extern "C" block in bink.h).
// Signatures are kept identical to the declarations in src/binklib/bink.h by
// reusing the same RADEXPFUNC / RADEXPLINK / PTR4 / HBINK macros.
//============================================================================

RADEXPFUNC char PTR4* RADEXPLINK BinkGetError( void )
{
    // No error string; return an empty (non-null) literal so callers that
    // print or strlen() the result behave sensibly.
    return (char PTR4*)"";
}

RADEXPFUNC HBINK RADEXPLINK BinkOpen( const char PTR4* name, U32 flags )
{
    (void)name;
    (void)flags;
    // Null handle => video unavailable; callers treat this as "movie failed to
    // open" and skip playback.
    return (HBINK)0;
}

RADEXPFUNC void RADEXPLINK BinkGetFrameBuffersInfo( HBINK bink, BINKFRAMEBUFFERS* fbset )
{
    (void)bink;
    (void)fbset;
}

RADEXPFUNC void RADEXPLINK BinkRegisterFrameBuffers( HBINK bink, BINKFRAMEBUFFERS* fbset )
{
    (void)bink;
    (void)fbset;
}

RADEXPFUNC S32 RADEXPLINK BinkDoFrame( HBINK bnk )
{
    (void)bnk;
    return 0;
}

RADEXPFUNC void RADEXPLINK BinkNextFrame( HBINK bnk )
{
    (void)bnk;
}

RADEXPFUNC S32 RADEXPLINK BinkWait( HBINK bnk )
{
    (void)bnk;
    // 0 => "don't wait / not time for next frame".
    return 0;
}

RADEXPFUNC void RADEXPLINK BinkClose( HBINK bnk )
{
    (void)bnk;
}

RADEXPFUNC S32 RADEXPLINK BinkPause( HBINK bnk, S32 pause )
{
    (void)bnk;
    (void)pause;
    return 0;
}

RADEXPFUNC void RADEXPLINK BinkSetVolume( HBINK bnk, U32 trackid, S32 volume )
{
    (void)bnk;
    (void)trackid;
    (void)volume;
}

RADEXPFUNC S32 RADEXPLINK BinkShouldSkip( HBINK bink )
{
    (void)bink;
    // 0 => don't skip.
    return 0;
}

RADEXPFUNC S32 RADEXPLINK BinkControlBackgroundIO( HBINK bink, U32 control )
{
    (void)bink;
    (void)control;
    return 0;
}

RADEXPFUNC void RADEXPLINK BinkGetRealtime( HBINK bink, BINKREALTIME PTR4* run, U32 frames )
{
    (void)bink;
    (void)run;
    (void)frames;
}

RADEXPFUNC void RADEXPLINK BinkSetSoundTrack( U32 total_tracks, U32 PTR4* tracks )
{
    (void)total_tracks;
    (void)tracks;
}

RADEXPFUNC void RADEXPLINK BinkSetIOSize( U32 iosize )
{
    (void)iosize;
}

RADEXPFUNC S32 RADEXPLINK BinkSetSoundSystem( BINKSNDSYSOPEN open, UINTa param )
{
    (void)open;
    (void)param;
    return 0;
}

RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenDirectSound( UINTa param )
{
    (void)param;
    // No sound system available; return a null open callback.
    return (BINKSNDOPEN)0;
}

RADEXPFUNC void RADEXPLINK BinkSetMemory( BINKMEMALLOC a, BINKMEMFREE f )
{
    (void)a;
    (void)f;
}

//============================================================================
// NvAPI stubs (C linkage via the extern "C" block in nvapi.h).
// NVAPI_INTERFACE expands to: extern NvAPI_Status __cdecl
//============================================================================

NVAPI_INTERFACE NvAPI_Initialize()
{
    // NVIDIA driver/NvAPI not available on this platform -> report failure.
    return NVAPI_ERROR;
}

NVAPI_INTERFACE NvAPI_Stereo_CreateHandleFromIUnknown( IUnknown* pDevice, StereoHandle* pStereoHandle )
{
    (void)pDevice;
    if ( pStereoHandle )
        *pStereoHandle = (StereoHandle)0;
    return NVAPI_ERROR;
}

NVAPI_INTERFACE NvAPI_Stereo_DestroyHandle( StereoHandle stereoHandle )
{
    (void)stereoHandle;
    return NVAPI_ERROR;
}

NVAPI_INTERFACE NvAPI_Stereo_IsActivated( StereoHandle stereoHandle, NvU8* pIsStereoOn )
{
    (void)stereoHandle;
    // Stereo is off.
    if ( pIsStereoOn )
        *pIsStereoOn = 0;
    return NVAPI_ERROR;
}

NVAPI_INTERFACE NvAPI_Stereo_SetConvergence( StereoHandle stereoHandle, float newConvergence )
{
    (void)stereoHandle;
    (void)newConvergence;
    return NVAPI_ERROR;
}
