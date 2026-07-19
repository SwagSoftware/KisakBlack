// gl_d3d9.cpp — OpenGL implementations of IDirect3D9 / IDirect3DDevice9.
#include "gl_d3d9.h"
#include "glcontext.h"
#include "gl_resources.h"

#include <GL/glew.h>
#include <SDL2/SDL.h>   // adapter display-mode queries (EnumAdapterModes etc.)
#include <cstdio>

// Default device caps, shared by GLDevice::GetDeviceCaps and GLD3D9::GetDeviceCaps.
// These advertise an SM3.0-class GPU, which is what the Black Ops renderer expects.
static void FillDefaultCaps(D3DCAPS9 *c) {
    *c = D3DCAPS9{};
    c->DeviceType              = D3DDEVTYPE_HAL;
    c->MaxTextureWidth         = 8192;
    c->MaxTextureHeight        = 8192;
    c->MaxAnisotropy           = 16;
    c->MaxSimultaneousTextures = 8;
    c->MaxTextureBlendStages   = 8;
    c->NumSimultaneousRTs      = 4;
    c->VertexShaderVersion     = 0xFFFE0300;  // vs_3_0
    c->PixelShaderVersion      = 0xFFFF0300;  // ps_3_0
    c->MaxVertexShaderConst    = 256;
    c->MaxPrimitiveCount       = 0x00FFFFFF;
    c->MaxVertexIndex          = 0x00FFFFFF;
    c->MaxStreams              = 16;
}

// ---------------------------------------------------------------------------
// GLDevice
// ---------------------------------------------------------------------------
GLDevice::GLDevice(GLContext *ctx, int width, int height)
    : ctx_(ctx), fbWidth_(width), fbHeight_(height), bbWidth_(width), bbHeight_(height) {}

GLDevice::~GLDevice() {
    if (swapChain_)   swapChain_->Release();   // swap chain references the back buffer; drop it first
    if (backBuffer_)  backBuffer_->Release();
    if (builtinProg_) glDeleteProgram(builtinProg_);
    if (vao_)         glDeleteVertexArrays(1, &vao_);
    if (fbo_)         glDeleteFramebuffers(1, &fbo_);
    if (fboDepth_)    glDeleteRenderbuffers(1, &fboDepth_);
    delete ctx_;
}

HRESULT WINAPI GLDevice::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9 *pRenderTarget) {
    if (RenderTargetIndex != 0) return D3D_OK;  // single render target for now (MRT: TODO)
    GLSurface *s = static_cast<GLSurface *>(pRenderTarget);
    // A null target, or the back-buffer surface itself, means the default framebuffer.
    if (!s || s->isBackbuffer()) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        fbWidth_ = bbWidth_; fbHeight_ = bbHeight_;
        return D3D_OK;
    }
    if (!fbo_) glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s->texName(), s->level());

    // Provide a matching depth-stencil buffer so depth testing works when
    // rendering to a texture. (Honoring an explicit SetDepthStencilSurface is a
    // TODO; for now the FBO owns an auto-sized depth-stencil renderbuffer.)
    int w = (int)s->width(), h = (int)s->height();
    if (fboDepthW_ != w || fboDepthH_ != h) {
        if (!fboDepth_) glGenRenderbuffers(1, &fboDepth_);
        glBindRenderbuffer(GL_RENDERBUFFER, fboDepth_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        fboDepthW_ = w; fboDepthH_ = h;
    }
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fboDepth_);

    fbWidth_ = w; fbHeight_ = h;
    return D3D_OK;
}

HRESULT WINAPI GLDevice::Reset(D3DPRESENT_PARAMETERS *pp) {
    if (pp && pp->BackBufferWidth && pp->BackBufferHeight) {
        fbWidth_  = (int)pp->BackBufferWidth;
        fbHeight_ = (int)pp->BackBufferHeight;
        if (ctx_) ctx_->Resize(fbWidth_, fbHeight_);
    }
    return D3D_OK;
}

HRESULT WINAPI GLDevice::Present(const RECT *, const RECT *, HWND, const RGNDATA *) {
    if (ctx_) ctx_->SwapBuffers();
    return D3D_OK;
}

HRESULT WINAPI GLDevice::Clear(DWORD /*Count*/, const D3DRECT * /*pRects*/, DWORD Flags,
                               D3DCOLOR Color, float Z, DWORD Stencil) {
    GLbitfield mask = 0;
    if (Flags & D3DCLEAR_TARGET) {
        const float inv = 1.0f / 255.0f;
        glClearColor(((Color >> 16) & 0xff) * inv,   // R
                     ((Color >>  8) & 0xff) * inv,   // G
                     ((Color      ) & 0xff) * inv,   // B
                     ((Color >> 24) & 0xff) * inv);  // A
        mask |= GL_COLOR_BUFFER_BIT;
    }
    if (Flags & D3DCLEAR_ZBUFFER)  { glClearDepth(Z);          mask |= GL_DEPTH_BUFFER_BIT; }
    if (Flags & D3DCLEAR_STENCIL)  { glClearStencil((GLint)Stencil); mask |= GL_STENCIL_BUFFER_BIT; }

    // D3D's Clear ignores scissor (when no rects) and the write masks; GL's does
    // not. Force the affected state for the clear, then restore.
    GLboolean scissor = glIsEnabled(GL_SCISSOR_TEST);
    if (scissor) glDisable(GL_SCISSOR_TEST);
    if (mask & GL_COLOR_BUFFER_BIT) glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    if (mask & GL_DEPTH_BUFFER_BIT) glDepthMask(GL_TRUE);

    glClear(mask);

    if (scissor) glEnable(GL_SCISSOR_TEST);
    return D3D_OK;
}

HRESULT WINAPI GLDevice::SetViewport(const D3DVIEWPORT9 *vp) {
    if (!vp) return E_INVALIDARG;
    // D3D viewport origin is top-left; GL is bottom-left — flip Y.
    glViewport((GLint)vp->X, fbHeight_ - (GLint)(vp->Y + vp->Height),
               (GLsizei)vp->Width, (GLsizei)vp->Height);
    glDepthRange(vp->MinZ, vp->MaxZ);
    return D3D_OK;
}

HRESULT WINAPI GLDevice::GetDeviceCaps(D3DCAPS9 *pCaps) {
    if (!pCaps) return E_INVALIDARG;
    FillDefaultCaps(pCaps);
    return D3D_OK;
}

// ---------------------------------------------------------------------------
// GLD3D9 (factory)
// ---------------------------------------------------------------------------
HRESULT WINAPI GLD3D9::GetAdapterIdentifier(UINT, DWORD, D3DADAPTER_IDENTIFIER9 *pIdentifier) {
    if (!pIdentifier) return E_INVALIDARG;
    *pIdentifier = D3DADAPTER_IDENTIFIER9{};
    // GL_VENDOR/GL_RENDERER need a current context; fall back to a neutral name.
    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *vendor   = glGetString(GL_VENDOR);
    snprintf(pIdentifier->Description, sizeof(pIdentifier->Description), "%s",
             renderer ? (const char *)renderer : "OpenGL Renderer");
    snprintf(pIdentifier->Driver, sizeof(pIdentifier->Driver), "%s",
             vendor ? (const char *)vendor : "OpenGL");
    return D3D_OK;
}

HRESULT WINAPI GLD3D9::GetAdapterDisplayMode(UINT, D3DDISPLAYMODE *pMode) {
    if (!pMode) return E_INVALIDARG;
    SDL_DisplayMode dm;
    if (SDL_WasInit(SDL_INIT_VIDEO) && SDL_GetDesktopDisplayMode(0, &dm) == 0) {
        pMode->Width = (UINT)dm.w; pMode->Height = (UINT)dm.h;
        pMode->RefreshRate = (UINT)dm.refresh_rate;
    } else {
        pMode->Width = 1920; pMode->Height = 1080; pMode->RefreshRate = 60;
    }
    pMode->Format = D3DFMT_X8R8G8B8;
    return D3D_OK;
}

UINT WINAPI GLD3D9::GetAdapterModeCount(UINT, D3DFORMAT) {
    int n = SDL_WasInit(SDL_INIT_VIDEO) ? SDL_GetNumDisplayModes(0) : 0;
    return n > 0 ? (UINT)n : 1;  // always offer at least the fallback desktop mode
}

HRESULT WINAPI GLD3D9::EnumAdapterModes(UINT, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE *pMode) {
    if (!pMode) return E_INVALIDARG;
    SDL_DisplayMode dm;
    if (SDL_WasInit(SDL_INIT_VIDEO) && SDL_GetDisplayMode(0, (int)Mode, &dm) == 0) {
        pMode->Width = (UINT)dm.w; pMode->Height = (UINT)dm.h;
        pMode->RefreshRate = (UINT)dm.refresh_rate;
    } else {
        if (Mode != 0) return D3DERR_INVALIDCALL;
        pMode->Width = 1920; pMode->Height = 1080; pMode->RefreshRate = 60;
    }
    pMode->Format = Format;
    return D3D_OK;
}

HRESULT WINAPI GLD3D9::GetDeviceCaps(UINT, D3DDEVTYPE, D3DCAPS9 *pCaps) {
    if (!pCaps) return E_INVALIDARG;
    FillDefaultCaps(pCaps);  // same caps the device reports
    return D3D_OK;
}

HRESULT WINAPI GLD3D9::CreateDevice(UINT /*Adapter*/, D3DDEVTYPE /*DeviceType*/,
                                    HWND /*hFocusWindow*/, DWORD /*BehaviorFlags*/,
                                    D3DPRESENT_PARAMETERS *pp,
                                    IDirect3DDevice9 **ppReturnedDeviceInterface) {
    if (!pp || !ppReturnedDeviceInterface) return E_INVALIDARG;
    *ppReturnedDeviceInterface = nullptr;

    GLContextDesc desc;
    desc.width        = pp->BackBufferWidth  ? (int)pp->BackBufferWidth  : 640;
    desc.height       = pp->BackBufferHeight ? (int)pp->BackBufferHeight : 480;
    desc.doubleBuffer = true;
    desc.depthStencil = pp->EnableAutoDepthStencil ? true : true;
    desc.visible      = pp->Windowed ? true : true;

    GLContext *ctx = GLContext::Create(desc);
    if (!ctx) { fprintf(stderr, "[gl] CreateDevice: GL context creation failed\n"); return E_FAIL; }

    *ppReturnedDeviceInterface = new GLDevice(ctx, desc.width, desc.height);
    return D3D_OK;
}

// ---------------------------------------------------------------------------
// Library entry point (replaces d3d9.dll's Direct3DCreate9 on non-Windows).
// ---------------------------------------------------------------------------
extern "C" IDirect3D9 *WINAPI Direct3DCreate9(UINT /*SDKVersion*/) {
    return new GLD3D9();
}
