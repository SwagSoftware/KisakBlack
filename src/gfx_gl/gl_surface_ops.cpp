// gl_surface_ops.cpp — standalone surface creation + resolve/readback.
//
// CreateRenderTarget / CreateOffscreenPlainSurface build standalone GLSurfaces;
// GetRenderTargetData reads a GPU render target back into a system-memory surface
// (the screenshot path); StretchRect blits between renderable surfaces (resolve /
// downsample) via glBlitFramebuffer.
#include "gl_d3d9.h"
#include "gl_resources.h"
#include "gl_format.h"

#include <GL/glew.h>

// --- Back buffer / swap chain ----------------------------------------------
//
// The GL backend renders straight into the window's default framebuffer, so the
// back buffer is a single GLSurface tagged as such (binding it as a render
// target restores FBO 0), and the swap chain is a thin shim whose Present()
// swaps the GL window. Both are created lazily and owned by the device.
GLSurface *GLDevice::backBufferSurface() {
    if (!backBuffer_)
        backBuffer_ = new GLSurface(this, (UINT)bbWidth_, (UINT)bbHeight_,
                                    D3DFMT_A8R8G8B8, GLBackbufferTag{});
    return backBuffer_;
}

HRESULT WINAPI GLDevice::GetBackBuffer(UINT, UINT, D3DBACKBUFFER_TYPE, IDirect3DSurface9 **pp) {
    if (!pp) return E_INVALIDARG;
    GLSurface *bb = backBufferSurface();
    bb->AddRef();
    *pp = bb;
    return D3D_OK;
}

HRESULT WINAPI GLDevice::GetSwapChain(UINT, IDirect3DSwapChain9 **pp) {
    if (!pp) return E_INVALIDARG;
    if (!swapChain_)
        swapChain_ = new GLSwapChain(this, backBufferSurface());
    swapChain_->AddRef();
    *pp = swapChain_;
    return D3D_OK;
}

// Read the just-rendered frame (default framebuffer, GL_BACK) into a system-memory
// surface as BGRA8 — the screenshot path: R_TakeScreenshot creates an offscreen
// A8R8G8B8 surface, calls this, then D3DXSaveSurfaceToFileA writes a TGA. glReadPixels
// is bottom-up and so is the TGA we emit, so no vertical flip is needed.
HRESULT WINAPI GLSwapChain::GetFrontBufferData(IDirect3DSurface9 *pDestSurface) {
    GLSurface *dst = static_cast<GLSurface *>(pDestSurface);
    if (!dst) return E_FAIL;
    UINT w = dst->width(), h = dst->height();
    std::vector<unsigned char> &shadow = dst->shadow();
    if (shadow.size() < (size_t)w * h * 4) shadow.assign((size_t)w * h * 4, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glReadBuffer(GL_BACK);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, (GLsizei)w, (GLsizei)h, GL_BGRA, GL_UNSIGNED_BYTE, shadow.data());
    return D3D_OK;
}

HRESULT WINAPI GLDevice::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format,
                                            D3DMULTISAMPLE_TYPE, DWORD, BOOL,
                                            IDirect3DSurface9 **ppSurface, HANDLE *) {
    if (!ppSurface) return E_INVALIDARG;
    *ppSurface = new GLSurface(this, Width, Height, Format, /*sysmem=*/false);
    return D3D_OK;
}

HRESULT WINAPI GLDevice::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format,
                                                     D3DPOOL, IDirect3DSurface9 **ppSurface, HANDLE *) {
    if (!ppSurface) return E_INVALIDARG;
    *ppSurface = new GLSurface(this, Width, Height, Format, /*sysmem=*/true);
    return D3D_OK;
}

HRESULT WINAPI GLDevice::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format,
                                                   D3DMULTISAMPLE_TYPE, DWORD, BOOL,
                                                   IDirect3DSurface9 **ppSurface, HANDLE *) {
    if (!ppSurface) return E_INVALIDARG;
    *ppSurface = new GLSurface(this, Width, Height, Format, GLDepthStencilTag{});
    return D3D_OK;
}

// Read a render target back into a system-memory surface for CPU access.
HRESULT WINAPI GLDevice::GetRenderTargetData(IDirect3DSurface9 *pRenderTarget,
                                             IDirect3DSurface9 *pDestSurface) {
    GLSurface *src = static_cast<GLSurface *>(pRenderTarget);
    GLSurface *dst = static_cast<GLSurface *>(pDestSurface);
    if (!src || !dst || !src->texName()) return E_FAIL;

    unsigned internal, fmt, type; int bpp;
    if (!D3DToGLFormat(dst->format(), &internal, &fmt, &type, &bpp)) return E_FAIL;

    UINT w = src->width() < dst->width() ? src->width() : dst->width();
    UINT h = src->height() < dst->height() ? src->height() : dst->height();
    if (dst->shadow().size() < (size_t)dst->width() * dst->height() * bpp)
        dst->shadow().assign((size_t)dst->width() * dst->height() * bpp, 0);

    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           src->texName(), src->level());
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, fmt, type, dst->shadow().data());
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
    return D3D_OK;
}

// Blit between two renderable surfaces (resolve / scale). Either side may be the
// back buffer (FBO 0) — e.g. capturing the frame-buffer render target into an image
// for refraction/feedback — so the back buffer binds framebuffer 0 directly instead
// of a texture-attachment FBO.
HRESULT WINAPI GLDevice::StretchRect(IDirect3DSurface9 *pSourceSurface, const RECT *pSourceRect,
                                     IDirect3DSurface9 *pDestSurface, const RECT *pDestRect,
                                     D3DTEXTUREFILTERTYPE Filter) {
    GLSurface *src = static_cast<GLSurface *>(pSourceSurface);
    GLSurface *dst = static_cast<GLSurface *>(pDestSurface);
    if (!src || !dst) return E_FAIL;
    if ((!src->isBackbuffer() && !src->texName()) ||
        (!dst->isBackbuffer() && !dst->texName())) return E_FAIL;

    int sx0 = 0, sy0 = 0, sx1 = (int)src->width(), sy1 = (int)src->height();
    int dx0 = 0, dy0 = 0, dx1 = (int)dst->width(), dy1 = (int)dst->height();
    if (pSourceRect) { sx0 = pSourceRect->left; sy0 = pSourceRect->top; sx1 = pSourceRect->right; sy1 = pSourceRect->bottom; }
    if (pDestRect)   { dx0 = pDestRect->left;   dy0 = pDestRect->top;   dx1 = pDestRect->right;   dy1 = pDestRect->bottom; }

    GLuint fbos[2] = {0, 0};
    if (src->isBackbuffer()) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glReadBuffer(GL_BACK);
    } else {
        glGenFramebuffers(1, &fbos[0]);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbos[0]);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, src->texName(), src->level());
        glReadBuffer(GL_COLOR_ATTACHMENT0);
    }
    if (dst->isBackbuffer()) {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glDrawBuffer(GL_BACK);
    } else {
        glGenFramebuffers(1, &fbos[1]);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbos[1]);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dst->texName(), dst->level());
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
    }
    glBlitFramebuffer(sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1, GL_COLOR_BUFFER_BIT,
                      Filter == D3DTEXF_NONE ? GL_NEAREST : GL_LINEAR);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    if (fbos[0]) glDeleteFramebuffers(1, &fbos[0]);
    if (fbos[1]) glDeleteFramebuffers(1, &fbos[1]);
    return D3D_OK;
}
