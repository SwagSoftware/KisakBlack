// d3d9.h — portable Direct3D 9 COM interfaces for non-Windows builds.
//
// The interfaces are declared as plain abstract C++ classes (rather than the real SDK's
// DECLARE_INTERFACE macro soup) for readability. We need source-compatibility with the
// renderer's call sites, NOT binary compatibility with Microsoft's d3d9.dll — these
// interfaces are implemented by our own OpenGL backend in src/gfx_gl, so vtable layout
// only has to be self-consistent.
//
// Only the methods the KisakBlack renderer actually calls are declared (extracted by
// grepping every `->Method(` call site over src/). Unused methods are intentionally
// omitted; add them here (with the standard MSDN signature) if a new call site appears.
#ifndef KISAK_D3D9_H
#define KISAK_D3D9_H

// Also announce the real SDK's include-guard macro: third-party vendor headers
// (e.g. nvapi.h) gate their D3D9-interop sections on `#if defined(_D3D9_H_)`.
#ifndef _D3D9_H_
#define _D3D9_H_
#endif

#include "d3d9types.h"

#define D3D_SDK_VERSION 32

// Forward declarations for the whole interface family.
struct IDirect3D9;
struct IDirect3DDevice9;
struct IDirect3DResource9;
struct IDirect3DBaseTexture9;
struct IDirect3DTexture9;
struct IDirect3DCubeTexture9;
struct IDirect3DVolumeTexture9;
struct IDirect3DSurface9;
struct IDirect3DVertexBuffer9;
struct IDirect3DIndexBuffer9;
struct IDirect3DVertexDeclaration9;
struct IDirect3DVertexShader9;
struct IDirect3DPixelShader9;
struct IDirect3DQuery9;
struct IDirect3DSwapChain9;
struct RGNDATA;   // opaque; the renderer only ever passes NULL

// d3d9-specific small types (mirror the real SDK).
typedef enum _D3DBACKBUFFER_TYPE { D3DBACKBUFFER_TYPE_MONO = 0 } D3DBACKBUFFER_TYPE;
typedef struct _D3DGAMMARAMP { WORD red[256], green[256], blue[256]; } D3DGAMMARAMP;

typedef struct _D3DVERTEXBUFFER_DESC {
    D3DFORMAT       Format;
    D3DRESOURCETYPE Type;
    DWORD           Usage;
    D3DPOOL         Pool;
    UINT            Size;
    DWORD           FVF;
} D3DVERTEXBUFFER_DESC;

typedef struct _D3DINDEXBUFFER_DESC {
    D3DFORMAT       Format;
    D3DRESOURCETYPE Type;
    DWORD           Usage;
    D3DPOOL         Pool;
    UINT            Size;
} D3DINDEXBUFFER_DESC;

// ---------------------------------------------------------------------------
struct IDirect3DResource9 : public IUnknown {
    virtual HRESULT WINAPI GetDevice(IDirect3DDevice9 **ppDevice) = 0;
    virtual D3DRESOURCETYPE WINAPI GetType() = 0;
    virtual DWORD   WINAPI SetPriority(DWORD PriorityNew) = 0;
    virtual DWORD   WINAPI GetPriority() = 0;
    virtual void    WINAPI PreLoad() = 0;
};

struct IDirect3DBaseTexture9 : public IDirect3DResource9 {
    virtual DWORD WINAPI SetLOD(DWORD LODNew) = 0;
    virtual DWORD WINAPI GetLOD() = 0;
    virtual DWORD WINAPI GetLevelCount() = 0;
};

struct IDirect3DTexture9 : public IDirect3DBaseTexture9 {
    virtual HRESULT WINAPI GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) = 0;
    virtual HRESULT WINAPI GetSurfaceLevel(UINT Level, IDirect3DSurface9 **ppSurfaceLevel) = 0;
    virtual HRESULT WINAPI LockRect(UINT Level, D3DLOCKED_RECT *pLockedRect,
                                    const RECT *pRect, DWORD Flags) = 0;
    virtual HRESULT WINAPI UnlockRect(UINT Level) = 0;
    virtual HRESULT WINAPI AddDirtyRect(const RECT *pDirtyRect) = 0;
};

struct IDirect3DCubeTexture9 : public IDirect3DBaseTexture9 {
    virtual HRESULT WINAPI GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) = 0;
    virtual HRESULT WINAPI GetCubeMapSurface(D3DCUBEMAP_FACES FaceType, UINT Level,
                                             IDirect3DSurface9 **ppCubeMapSurface) = 0;
    virtual HRESULT WINAPI LockRect(D3DCUBEMAP_FACES FaceType, UINT Level,
                                    D3DLOCKED_RECT *pLockedRect, const RECT *pRect,
                                    DWORD Flags) = 0;
    virtual HRESULT WINAPI UnlockRect(D3DCUBEMAP_FACES FaceType, UINT Level) = 0;
};

struct IDirect3DVolumeTexture9 : public IDirect3DBaseTexture9 {
    virtual HRESULT WINAPI GetLevelDesc(UINT Level, D3DVOLUME_DESC *pDesc) = 0;
    virtual HRESULT WINAPI LockBox(UINT Level, D3DLOCKED_BOX *pLockedVolume,
                                   const D3DBOX *pBox, DWORD Flags) = 0;
    virtual HRESULT WINAPI UnlockBox(UINT Level) = 0;
};

struct IDirect3DSurface9 : public IDirect3DResource9 {
    virtual HRESULT WINAPI GetContainer(REFIID riid, void **ppContainer) = 0;
    virtual HRESULT WINAPI GetDesc(D3DSURFACE_DESC *pDesc) = 0;
    virtual HRESULT WINAPI LockRect(D3DLOCKED_RECT *pLockedRect, const RECT *pRect,
                                    DWORD Flags) = 0;
    virtual HRESULT WINAPI UnlockRect() = 0;
};

struct IDirect3DVertexBuffer9 : public IDirect3DResource9 {
    virtual HRESULT WINAPI Lock(UINT OffsetToLock, UINT SizeToLock, void **ppbData,
                                DWORD Flags) = 0;
    virtual HRESULT WINAPI Unlock() = 0;
    virtual HRESULT WINAPI GetDesc(D3DVERTEXBUFFER_DESC *pDesc) = 0;
};

struct IDirect3DIndexBuffer9 : public IDirect3DResource9 {
    virtual HRESULT WINAPI Lock(UINT OffsetToLock, UINT SizeToLock, void **ppbData,
                                DWORD Flags) = 0;
    virtual HRESULT WINAPI Unlock() = 0;
    virtual HRESULT WINAPI GetDesc(D3DINDEXBUFFER_DESC *pDesc) = 0;
};

struct IDirect3DVertexDeclaration9 : public IUnknown {
    virtual HRESULT WINAPI GetDevice(IDirect3DDevice9 **ppDevice) = 0;
    virtual HRESULT WINAPI GetDeclaration(D3DVERTEXELEMENT9 *pElement, UINT *pNumElements) = 0;
};

struct IDirect3DVertexShader9 : public IUnknown {
    virtual HRESULT WINAPI GetDevice(IDirect3DDevice9 **ppDevice) = 0;
    virtual HRESULT WINAPI GetFunction(void *pData, UINT *pSizeOfData) = 0;
};

struct IDirect3DPixelShader9 : public IUnknown {
    virtual HRESULT WINAPI GetDevice(IDirect3DDevice9 **ppDevice) = 0;
    virtual HRESULT WINAPI GetFunction(void *pData, UINT *pSizeOfData) = 0;
};

struct IDirect3DQuery9 : public IUnknown {
    virtual HRESULT WINAPI GetDevice(IDirect3DDevice9 **ppDevice) = 0;
    virtual D3DQUERYTYPE WINAPI GetType() = 0;
    virtual DWORD   WINAPI GetDataSize() = 0;
    virtual HRESULT WINAPI Issue(DWORD dwIssueFlags) = 0;
    virtual HRESULT WINAPI GetData(void *pData, DWORD dwSize, DWORD dwGetDataFlags) = 0;
};

struct IDirect3DSwapChain9 : public IUnknown {
    virtual HRESULT WINAPI Present(const RECT *pSourceRect, const RECT *pDestRect,
                                   HWND hDestWindowOverride, const RGNDATA *pDirtyRegion,
                                   DWORD dwFlags) = 0;
    // GetFrontBufferData must stay at vtable index 4 (the slot real D3D9 uses): the
    // screenshot path calls it through a raw vtable offset, so its position matters.
    virtual HRESULT WINAPI GetFrontBufferData(IDirect3DSurface9 *pDestSurface) = 0;
    virtual HRESULT WINAPI GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type,
                                         IDirect3DSurface9 **ppBackBuffer) = 0;
};

// ---------------------------------------------------------------------------
struct IDirect3DDevice9 : public IUnknown {
    virtual HRESULT WINAPI TestCooperativeLevel() = 0;
    virtual UINT    WINAPI GetAvailableTextureMem() = 0;
    virtual HRESULT WINAPI GetDeviceCaps(D3DCAPS9 *pCaps) = 0;
    virtual HRESULT WINAPI Reset(D3DPRESENT_PARAMETERS *pPresentationParameters) = 0;
    virtual HRESULT WINAPI Present(const RECT *pSourceRect, const RECT *pDestRect,
                                   HWND hDestWindowOverride, const RGNDATA *pDirtyRegion) = 0;
    virtual HRESULT WINAPI GetBackBuffer(UINT iSwapChain, UINT iBackBuffer,
                                         D3DBACKBUFFER_TYPE Type,
                                         IDirect3DSurface9 **ppBackBuffer) = 0;
    virtual HRESULT WINAPI GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9 **ppSwapChain) = 0;

    // Resource creation
    virtual HRESULT WINAPI CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage,
                                         D3DFORMAT Format, D3DPOOL Pool,
                                         IDirect3DTexture9 **ppTexture, HANDLE *pSharedHandle) = 0;
    virtual HRESULT WINAPI CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels,
                                               DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
                                               IDirect3DVolumeTexture9 **ppVolumeTexture,
                                               HANDLE *pSharedHandle) = 0;
    virtual HRESULT WINAPI CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage,
                                             D3DFORMAT Format, D3DPOOL Pool,
                                             IDirect3DCubeTexture9 **ppCubeTexture,
                                             HANDLE *pSharedHandle) = 0;
    virtual HRESULT WINAPI CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool,
                                              IDirect3DVertexBuffer9 **ppVertexBuffer,
                                              HANDLE *pSharedHandle) = 0;
    virtual HRESULT WINAPI CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format,
                                             D3DPOOL Pool, IDirect3DIndexBuffer9 **ppIndexBuffer,
                                             HANDLE *pSharedHandle) = 0;
    virtual HRESULT WINAPI CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format,
                                              D3DMULTISAMPLE_TYPE MultiSample,
                                              DWORD MultisampleQuality, BOOL Lockable,
                                              IDirect3DSurface9 **ppSurface,
                                              HANDLE *pSharedHandle) = 0;
    virtual HRESULT WINAPI CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format,
                                                     D3DMULTISAMPLE_TYPE MultiSample,
                                                     DWORD MultisampleQuality, BOOL Discard,
                                                     IDirect3DSurface9 **ppSurface,
                                                     HANDLE *pSharedHandle) = 0;
    virtual HRESULT WINAPI CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format,
                                                       D3DPOOL Pool, IDirect3DSurface9 **ppSurface,
                                                       HANDLE *pSharedHandle) = 0;

    // Surface transfer
    virtual HRESULT WINAPI UpdateSurface(IDirect3DSurface9 *pSourceSurface, const RECT *pSourceRect,
                                         IDirect3DSurface9 *pDestinationSurface,
                                         const POINT *pDestPoint) = 0;
    virtual HRESULT WINAPI GetRenderTargetData(IDirect3DSurface9 *pRenderTarget,
                                               IDirect3DSurface9 *pDestSurface) = 0;
    virtual HRESULT WINAPI StretchRect(IDirect3DSurface9 *pSourceSurface, const RECT *pSourceRect,
                                       IDirect3DSurface9 *pDestSurface, const RECT *pDestRect,
                                       D3DTEXTUREFILTERTYPE Filter) = 0;

    // Targets / frame
    virtual HRESULT WINAPI SetRenderTarget(DWORD RenderTargetIndex,
                                           IDirect3DSurface9 *pRenderTarget) = 0;
    virtual HRESULT WINAPI SetDepthStencilSurface(IDirect3DSurface9 *pNewZStencil) = 0;
    virtual HRESULT WINAPI BeginScene() = 0;
    virtual HRESULT WINAPI EndScene() = 0;
    virtual HRESULT WINAPI Clear(DWORD Count, const D3DRECT *pRects, DWORD Flags, D3DCOLOR Color,
                                 float Z, DWORD Stencil) = 0;
    virtual HRESULT WINAPI SetViewport(const D3DVIEWPORT9 *pViewport) = 0;

    // Fixed/programmable state
    virtual HRESULT WINAPI SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) = 0;
    virtual HRESULT WINAPI SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) = 0;
    virtual HRESULT WINAPI SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type,
                                                DWORD Value) = 0;
    virtual HRESULT WINAPI SetTexture(DWORD Stage, IDirect3DBaseTexture9 *pTexture) = 0;
    virtual HRESULT WINAPI SetScissorRect(const RECT *pRect) = 0;

    // Geometry binding
    virtual HRESULT WINAPI SetStreamSource(UINT StreamNumber,
                                           IDirect3DVertexBuffer9 *pStreamData,
                                           UINT OffsetInBytes, UINT Stride) = 0;
    virtual HRESULT WINAPI SetIndices(IDirect3DIndexBuffer9 *pIndexData) = 0;
    virtual HRESULT WINAPI CreateVertexDeclaration(const D3DVERTEXELEMENT9 *pVertexElements,
                                                   IDirect3DVertexDeclaration9 **ppDecl) = 0;
    virtual HRESULT WINAPI SetVertexDeclaration(IDirect3DVertexDeclaration9 *pDecl) = 0;

    // Shaders
    virtual HRESULT WINAPI CreateVertexShader(const DWORD *pFunction,
                                              IDirect3DVertexShader9 **ppShader) = 0;
    virtual HRESULT WINAPI SetVertexShader(IDirect3DVertexShader9 *pShader) = 0;
    virtual HRESULT WINAPI SetVertexShaderConstantF(UINT StartRegister,
                                                    const float *pConstantData,
                                                    UINT Vector4fCount) = 0;
    virtual HRESULT WINAPI CreatePixelShader(const DWORD *pFunction,
                                             IDirect3DPixelShader9 **ppShader) = 0;
    virtual HRESULT WINAPI SetPixelShader(IDirect3DPixelShader9 *pShader) = 0;
    virtual HRESULT WINAPI SetPixelShaderConstantF(UINT StartRegister,
                                                   const float *pConstantData,
                                                   UINT Vector4fCount) = 0;

    // Draw
    virtual HRESULT WINAPI DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex,
                                         UINT PrimitiveCount) = 0;
    virtual HRESULT WINAPI DrawIndexedPrimitive(D3DPRIMITIVETYPE Type, INT BaseVertexIndex,
                                                UINT MinVertexIndex, UINT NumVertices,
                                                UINT startIndex, UINT primCount) = 0;
    virtual HRESULT WINAPI DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount,
                                           const void *pVertexStreamZeroData,
                                           UINT VertexStreamZeroStride) = 0;

    // Misc
    virtual HRESULT WINAPI CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9 **ppQuery) = 0;
    virtual void    WINAPI SetGammaRamp(UINT iSwapChain, DWORD Flags, const D3DGAMMARAMP *pRamp) = 0;
};

// ---------------------------------------------------------------------------
// Adapter identification (D3DADAPTER_IDENTIFIER9). Layout matches the real SDK;
// the renderer reads Description (GPU name), VendorId, and DriverVersion.
#define MAX_DEVICE_IDENTIFIER_STRING 512
typedef struct _D3DADAPTER_IDENTIFIER9 {
    char          Driver[MAX_DEVICE_IDENTIFIER_STRING];
    char          Description[MAX_DEVICE_IDENTIFIER_STRING];
    char          DeviceName[32];
    LARGE_INTEGER DriverVersion;
    DWORD         VendorId, DeviceId, SubSysId, Revision;
    GUID          DeviceIdentifier;
    DWORD         WHQLLevel;
} D3DADAPTER_IDENTIFIER9, *LPD3DADAPTER_IDENTIFIER9;

struct IDirect3D9 : public IUnknown {
    virtual UINT    WINAPI GetAdapterCount() = 0;
    virtual HRESULT WINAPI GetAdapterIdentifier(UINT Adapter, DWORD Flags,
                                                D3DADAPTER_IDENTIFIER9 *pIdentifier) = 0;
    virtual UINT    WINAPI GetAdapterModeCount(UINT Adapter, D3DFORMAT Format) = 0;
    virtual HRESULT WINAPI EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode,
                                            D3DDISPLAYMODE *pMode) = 0;
    virtual HMONITOR WINAPI GetAdapterMonitor(UINT Adapter) = 0;
    virtual HRESULT WINAPI GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode) = 0;
    virtual HRESULT WINAPI GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9 *pCaps) = 0;
    virtual HRESULT WINAPI CheckDeviceType(UINT Adapter, D3DDEVTYPE DevType,
                                           D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
                                           BOOL bWindowed) = 0;
    virtual HRESULT WINAPI CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType,
                                             D3DFORMAT AdapterFormat, DWORD Usage,
                                             D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) = 0;
    virtual HRESULT WINAPI CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType,
                                                      D3DFORMAT SurfaceFormat, BOOL Windowed,
                                                      D3DMULTISAMPLE_TYPE MultiSampleType,
                                                      DWORD *pQualityLevels) = 0;
    virtual HRESULT WINAPI CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType,
                                                  D3DFORMAT AdapterFormat,
                                                  D3DFORMAT RenderTargetFormat,
                                                  D3DFORMAT DepthStencilFormat) = 0;
    virtual HRESULT WINAPI CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow,
                                        DWORD BehaviorFlags,
                                        D3DPRESENT_PARAMETERS *pPresentationParameters,
                                        IDirect3DDevice9 **ppReturnedDeviceInterface) = 0;
};

// Library entry point. On Windows this lives in d3d9.dll; here it is provided by
// src/gfx_gl (it instantiates our GL-backed IDirect3D9).
extern "C" IDirect3D9 *WINAPI Direct3DCreate9(UINT SDKVersion);

// PIX debug-event markers (exported from d3d9.dll on Windows). No-ops here; a GL
// build can later route these to GL_KHR_debug push/pop groups.
static inline int WINAPI D3DPERF_BeginEvent(D3DCOLOR, const wchar_t *) { return 0; }
static inline int WINAPI D3DPERF_EndEvent(void) { return 0; }

// Convenience typedefs used by the renderer.
typedef IDirect3D9                 *LPDIRECT3D9;
typedef IDirect3DDevice9           *LPDIRECT3DDEVICE9;
typedef IDirect3DTexture9          *LPDIRECT3DTEXTURE9;
typedef IDirect3DSurface9          *LPDIRECT3DSURFACE9;
typedef IDirect3DVertexBuffer9     *LPDIRECT3DVERTEXBUFFER9;
typedef IDirect3DIndexBuffer9      *LPDIRECT3DINDEXBUFFER9;
typedef IDirect3DVertexDeclaration9 *LPDIRECT3DVERTEXDECLARATION9;
typedef IDirect3DVertexShader9     *LPDIRECT3DVERTEXSHADER9;
typedef IDirect3DPixelShader9      *LPDIRECT3DPIXELSHADER9;
typedef IDirect3DQuery9            *LPDIRECT3DQUERY9;
typedef IDirect3DSwapChain9        *LPDIRECT3DSWAPCHAIN9;

#endif // KISAK_D3D9_H
