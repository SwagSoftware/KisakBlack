// gl_d3d9.h — OpenGL-backed implementations of the D3D9 interfaces.
//
// This is the heart of the translation layer: GLD3D9 implements IDirect3D9 and
// GLDevice implements IDirect3DDevice9 on top of OpenGL. The renderer in
// src/gfx_d3d is unchanged — it still calls Direct3DCreate9()/CreateDevice() and
// drives the returned interfaces.
//
// Methods that are not yet ported are stubbed inline so the layer keeps building
// and the device keeps running while they are filled in:
//   * state setters return D3D_OK (no-op) — wrong pixels, but no crash;
//   * resource creators return E_NOTIMPL until their GL resource classes land.
// Each stub is marked TODO so coverage is greppable.
#ifndef KISAK_GL_D3D9_H
#define KISAK_GL_D3D9_H

#include "gl_object.h"
#include <map>
#include <cstdint>

class GLContext;
class GLVertexBuffer;
class GLIndexBuffer;
class GLVertexDeclaration;
class GLTexture;
class GLVertexShader;
class GLPixelShader;

// D3D sampler state, kept in D3D terms and translated to GL at draw time.
struct GLSamplerState {
    DWORD minFilter = D3DTEXF_LINEAR;
    DWORD magFilter = D3DTEXF_LINEAR;
    DWORD addressU  = D3DTADDRESS_WRAP;
    DWORD addressV  = D3DTADDRESS_WRAP;
};

// ---- IDirect3DDevice9 -> OpenGL -------------------------------------------
class GLDevice final : public GLObject<IDirect3DDevice9> {
public:
    explicit GLDevice(GLContext *ctx, int width, int height);
    ~GLDevice() override;

    // --- Frame / target (gl_d3d9.cpp) ---
    HRESULT WINAPI TestCooperativeLevel() override { return D3D_OK; }
    UINT    WINAPI GetAvailableTextureMem() override { return 256u * 1024 * 1024; }
    HRESULT WINAPI GetDeviceCaps(D3DCAPS9 *pCaps) override;
    HRESULT WINAPI Reset(D3DPRESENT_PARAMETERS *pp) override;
    HRESULT WINAPI Present(const RECT *, const RECT *, HWND, const RGNDATA *) override;
    HRESULT WINAPI BeginScene() override { inScene_ = true;  return D3D_OK; }
    HRESULT WINAPI EndScene() override   { inScene_ = false; return D3D_OK; }
    HRESULT WINAPI Clear(DWORD Count, const D3DRECT *pRects, DWORD Flags, D3DCOLOR Color,
                         float Z, DWORD Stencil) override;
    HRESULT WINAPI SetViewport(const D3DVIEWPORT9 *pViewport) override;

    // --- Geometry resources + draw (gl_d3d9_draw.cpp) ---
    HRESULT WINAPI CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool,
                                      IDirect3DVertexBuffer9 **ppVB, HANDLE *) override;
    HRESULT WINAPI CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
                                     IDirect3DIndexBuffer9 **ppIB, HANDLE *) override;
    HRESULT WINAPI CreateVertexDeclaration(const D3DVERTEXELEMENT9 *pElements,
                                           IDirect3DVertexDeclaration9 **ppDecl) override;
    HRESULT WINAPI SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9 *pStreamData,
                                   UINT OffsetInBytes, UINT Stride) override;
    HRESULT WINAPI SetIndices(IDirect3DIndexBuffer9 *pIndexData) override;
    HRESULT WINAPI SetVertexDeclaration(IDirect3DVertexDeclaration9 *pDecl) override;
    HRESULT WINAPI DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex,
                                 UINT PrimitiveCount) override;
    HRESULT WINAPI DrawIndexedPrimitive(D3DPRIMITIVETYPE Type, INT BaseVertexIndex,
                                        UINT MinVertexIndex, UINT NumVertices,
                                        UINT startIndex, UINT primCount) override;
    HRESULT WINAPI DrawPrimitiveUP(D3DPRIMITIVETYPE, UINT, const void *, UINT) override { return D3D_OK; }

    // --- Render / sampler / texture state (gl_state.cpp) ---
    HRESULT WINAPI SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) override;
    HRESULT WINAPI SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) override;
    HRESULT WINAPI SetTexture(DWORD Stage, IDirect3DBaseTexture9 *pTexture) override;
    HRESULT WINAPI SetScissorRect(const RECT *pRect) override;
    HRESULT WINAPI SetVertexShader(IDirect3DVertexShader9 *pShader) override;
    HRESULT WINAPI SetVertexShaderConstantF(UINT StartRegister, const float *pData, UINT Vec4Count) override;
    HRESULT WINAPI SetPixelShader(IDirect3DPixelShader9 *pShader) override;
    HRESULT WINAPI SetPixelShaderConstantF(UINT StartRegister, const float *pData, UINT Vec4Count) override;
    HRESULT WINAPI SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9 *pRenderTarget) override;
    HRESULT WINAPI SetDepthStencilSurface(IDirect3DSurface9 *) override { return D3D_OK; }
    void    WINAPI SetGammaRamp(UINT, DWORD, const D3DGAMMARAMP *) override {}

    // --- Not yet ported: textures / surfaces / shaders / queries — TODO(task #4/#5) ---
    HRESULT WINAPI GetBackBuffer(UINT, UINT, D3DBACKBUFFER_TYPE, IDirect3DSurface9 **pp) override { return ni(pp); }
    HRESULT WINAPI GetSwapChain(UINT, IDirect3DSwapChain9 **pp) override { return ni(pp); }
    HRESULT WINAPI CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage,
                                 D3DFORMAT Format, D3DPOOL Pool,
                                 IDirect3DTexture9 **ppTexture, HANDLE *) override;
    HRESULT WINAPI CreateVolumeTexture(UINT, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DVolumeTexture9 **pp, HANDLE *) override { return ni(pp); }
    HRESULT WINAPI CreateCubeTexture(UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DCubeTexture9 **pp, HANDLE *) override { return ni(pp); }
    HRESULT WINAPI CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE,
                                      DWORD, BOOL, IDirect3DSurface9 **ppSurface, HANDLE *) override;
    HRESULT WINAPI CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL,
                                               IDirect3DSurface9 **ppSurface, HANDLE *) override;
    HRESULT WINAPI GetRenderTargetData(IDirect3DSurface9 *pRenderTarget,
                                       IDirect3DSurface9 *pDestSurface) override;
    HRESULT WINAPI StretchRect(IDirect3DSurface9 *pSourceSurface, const RECT *pSourceRect,
                               IDirect3DSurface9 *pDestSurface, const RECT *pDestRect,
                               D3DTEXTUREFILTERTYPE Filter) override;
    // Still stubbed (TODO): standalone depth-stencil surfaces, UpdateSurface.
    HRESULT WINAPI CreateDepthStencilSurface(UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, DWORD, BOOL, IDirect3DSurface9 **pp, HANDLE *) override { return ni(pp); }
    HRESULT WINAPI UpdateSurface(IDirect3DSurface9 *, const RECT *, IDirect3DSurface9 *, const POINT *) override { return E_NOTIMPL; }
    HRESULT WINAPI CreateVertexShader(const DWORD *pFunction, IDirect3DVertexShader9 **ppShader) override;
    HRESULT WINAPI CreatePixelShader(const DWORD *pFunction, IDirect3DPixelShader9 **ppShader) override;
    HRESULT WINAPI CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9 **ppQuery) override;

private:
    static const int kMaxStages = 8;

    template <class T> static HRESULT ni(T **pp) { if (pp) *pp = nullptr; return E_NOTIMPL; }
    void ensureBuiltinProgram();  // lazily compile the built-in pre-transformed-vertex shader
    void bindBuiltinForDraw();    // use built-in program + set frame/texture uniforms
    void useDrawProgram();        // pick shader program (if vs+ps bound) or built-in; set uniforms
    void applyVertexState();      // set up VAO attribs from decl_ + streams_
    bool applyTextures();         // bind stage-0 texture + sampler state; returns true if sampling

    GLContext *ctx_ = nullptr;
    int  fbWidth_   = 0;   // current render-target dimensions (back buffer or FBO)
    int  fbHeight_  = 0;
    int  bbWidth_   = 0;   // back-buffer dimensions (restored when RT is unset)
    int  bbHeight_  = 0;
    unsigned fbo_       = 0;  // reused FBO for render-to-texture
    unsigned fboDepth_  = 0;  // its depth-stencil renderbuffer
    int      fboDepthW_ = 0;
    int      fboDepthH_ = 0;
    bool inScene_   = false;

    unsigned vao_                 = 0;
    unsigned builtinProg_         = 0;
    int      builtinViewportLoc_  = -1;
    int      builtinTexLoc_       = -1;
    int      builtinUseTexLoc_    = -1;

    struct Stream { GLVertexBuffer *vb = nullptr; UINT offset = 0; UINT stride = 0; };
    Stream               streams_[4];
    GLIndexBuffer       *ib_   = nullptr;
    GLVertexDeclaration *decl_ = nullptr;

    GLTexture     *boundTex_[kMaxStages] = {};
    GLSamplerState samplers_[kMaxStages];

    // Blend factors are set by two separate render states but applied together.
    DWORD blendSrc_  = D3DBLEND_ONE;
    DWORD blendDest_ = D3DBLEND_ZERO;

    // Programmable shader path: bound shaders, c# constant registers, and a cache
    // of linked (vs,ps) programs keyed by (vsShaderId<<32 | psShaderId).
    GLVertexShader *vs_ = nullptr;
    GLPixelShader  *ps_ = nullptr;
    float           vsConst_[256 * 4] = {};
    float           psConst_[256 * 4] = {};
    struct LinkedProgram { unsigned prog; int vscLoc; int pscLoc; };
    std::map<uint64_t, LinkedProgram> progCache_;
};

// ---- IDirect3D9 -> OpenGL (the factory object) ----------------------------
class GLD3D9 final : public GLObject<IDirect3D9> {
public:
    UINT    WINAPI GetAdapterCount() override { return 1; }
    HRESULT WINAPI GetAdapterIdentifier(UINT, DWORD, D3DADAPTER_IDENTIFIER9 *pIdentifier) override;
    UINT    WINAPI GetAdapterModeCount(UINT, D3DFORMAT) override;
    HRESULT WINAPI EnumAdapterModes(UINT, D3DFORMAT, UINT Mode, D3DDISPLAYMODE *pMode) override;
    HMONITOR WINAPI GetAdapterMonitor(UINT) override { return (HMONITOR)(intptr_t)1; }
    HRESULT WINAPI GetAdapterDisplayMode(UINT, D3DDISPLAYMODE *pMode) override;
    HRESULT WINAPI GetDeviceCaps(UINT, D3DDEVTYPE, D3DCAPS9 *pCaps) override;
    HRESULT WINAPI CheckDeviceType(UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, BOOL) override { return D3D_OK; }
    HRESULT WINAPI CheckDeviceFormat(UINT, D3DDEVTYPE, D3DFORMAT, DWORD, D3DRESOURCETYPE, D3DFORMAT) override { return D3D_OK; }
    HRESULT WINAPI CheckDeviceMultiSampleType(UINT, D3DDEVTYPE, D3DFORMAT, BOOL, D3DMULTISAMPLE_TYPE, DWORD *pQ) override { if (pQ) *pQ = 1; return D3D_OK; }
    HRESULT WINAPI CheckDepthStencilMatch(UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, D3DFORMAT) override { return D3D_OK; }
    HRESULT WINAPI CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow,
                                DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pp,
                                IDirect3DDevice9 **ppReturnedDeviceInterface) override;
};

#endif // KISAK_GL_D3D9_H
