// gl_resources.cpp — GL-backed vertex/index buffers + vertex declaration.
#include "gl_resources.h"
#include "gl_format.h"

#include <GL/glew.h>
#include <cstdio>
#include <cstring>

// ---- GLVertexBuffer -------------------------------------------------------
GLVertexBuffer::GLVertexBuffer(IDirect3DDevice9 *device, UINT length, DWORD usage,
                               DWORD fvf, D3DPOOL pool)
    : device_(device), length_(length), usage_(usage), fvf_(fvf), pool_(pool),
      shadow_(length, 0) {
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, length_,
                 nullptr, (usage_ & D3DUSAGE_DYNAMIC) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLVertexBuffer::~GLVertexBuffer() { if (vbo_) glDeleteBuffers(1, &vbo_); }

HRESULT WINAPI GLVertexBuffer::GetDevice(IDirect3DDevice9 **ppDevice) {
    if (!ppDevice) return E_INVALIDARG;
    *ppDevice = device_;
    if (device_) device_->AddRef();
    return D3D_OK;
}

HRESULT WINAPI GLVertexBuffer::Lock(UINT OffsetToLock, UINT SizeToLock, void **ppbData, DWORD) {
    if (!ppbData) return E_INVALIDARG;
    if (SizeToLock == 0) SizeToLock = length_ - OffsetToLock;  // 0 means "to end" in D3D
    lockOffset_ = OffsetToLock;
    lockSize_   = SizeToLock;
    dirty_      = true;
    *ppbData    = shadow_.data() + OffsetToLock;
    return D3D_OK;
}

HRESULT WINAPI GLVertexBuffer::Unlock() {
    if (dirty_) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferSubData(GL_ARRAY_BUFFER, lockOffset_, lockSize_, shadow_.data() + lockOffset_);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        dirty_ = false;
    }
    return D3D_OK;
}

HRESULT WINAPI GLVertexBuffer::GetDesc(D3DVERTEXBUFFER_DESC *pDesc) {
    if (!pDesc) return E_INVALIDARG;
    *pDesc = D3DVERTEXBUFFER_DESC{};
    pDesc->Format = D3DFMT_UNKNOWN; pDesc->Type = D3DRTYPE_VERTEXBUFFER;
    pDesc->Usage = usage_; pDesc->Pool = pool_; pDesc->Size = length_; pDesc->FVF = fvf_;
    return D3D_OK;
}

// ---- GLIndexBuffer --------------------------------------------------------
GLIndexBuffer::GLIndexBuffer(IDirect3DDevice9 *device, UINT length, DWORD usage,
                             D3DFORMAT format, D3DPOOL pool)
    : device_(device), length_(length), usage_(usage), format_(format), pool_(pool),
      shadow_(length, 0) {
    glGenBuffers(1, &ibo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, length_,
                 nullptr, (usage_ & D3DUSAGE_DYNAMIC) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

GLIndexBuffer::~GLIndexBuffer() { if (ibo_) glDeleteBuffers(1, &ibo_); }

HRESULT WINAPI GLIndexBuffer::GetDevice(IDirect3DDevice9 **ppDevice) {
    if (!ppDevice) return E_INVALIDARG;
    *ppDevice = device_;
    if (device_) device_->AddRef();
    return D3D_OK;
}

HRESULT WINAPI GLIndexBuffer::Lock(UINT OffsetToLock, UINT SizeToLock, void **ppbData, DWORD) {
    if (!ppbData) return E_INVALIDARG;
    if (SizeToLock == 0) SizeToLock = length_ - OffsetToLock;
    lockOffset_ = OffsetToLock;
    lockSize_   = SizeToLock;
    dirty_      = true;
    *ppbData    = shadow_.data() + OffsetToLock;
    return D3D_OK;
}

HRESULT WINAPI GLIndexBuffer::Unlock() {
    if (dirty_) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, lockOffset_, lockSize_, shadow_.data() + lockOffset_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        dirty_ = false;
    }
    return D3D_OK;
}

HRESULT WINAPI GLIndexBuffer::GetDesc(D3DINDEXBUFFER_DESC *pDesc) {
    if (!pDesc) return E_INVALIDARG;
    *pDesc = D3DINDEXBUFFER_DESC{};
    pDesc->Format = format_; pDesc->Type = D3DRTYPE_INDEXBUFFER;
    pDesc->Usage = usage_; pDesc->Pool = pool_; pDesc->Size = length_;
    return D3D_OK;
}

// ---- GLTexture ------------------------------------------------------------
GLTexture::GLTexture(IDirect3DDevice9 *device, UINT width, UINT height, UINT levels,
                     DWORD usage, D3DFORMAT format, D3DPOOL pool)
    : device_(device), width_(width), height_(height),
      levels_(levels ? levels : 1), usage_(usage), format_(format), pool_(pool) {
    glGenTextures(1, &tex_);
    glBindTexture(GL_TEXTURE_2D, tex_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, levels_ - 1);
    levelShadow_.resize(levels_);
    // A render-target texture is never Lock/Unlocked, so allocate level-0 storage
    // now to make it complete for FBO colour attachment.
    if (usage_ & D3DUSAGE_RENDERTARGET) {
        unsigned internal, fmt, type; int bpp;
        if (D3DToGLFormat(format_, &internal, &fmt, &type, &bpp))
            glTexImage2D(GL_TEXTURE_2D, 0, internal, width_, height_, 0, fmt, type, nullptr);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLTexture::~GLTexture() { if (tex_) glDeleteTextures(1, &tex_); }

HRESULT WINAPI GLTexture::GetDevice(IDirect3DDevice9 **ppDevice) {
    if (!ppDevice) return E_INVALIDARG;
    *ppDevice = device_;
    if (device_) device_->AddRef();
    return D3D_OK;
}

HRESULT WINAPI GLTexture::GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) {
    if (!pDesc || Level >= levels_) return E_INVALIDARG;
    *pDesc = D3DSURFACE_DESC{};
    pDesc->Format = format_; pDesc->Type = D3DRTYPE_SURFACE; pDesc->Usage = usage_; pDesc->Pool = pool_;
    pDesc->MultiSampleType = D3DMULTISAMPLE_NONE;
    pDesc->Width  = width_  >> Level ? width_  >> Level : 1;
    pDesc->Height = height_ >> Level ? height_ >> Level : 1;
    return D3D_OK;
}

HRESULT WINAPI GLTexture::GetSurfaceLevel(UINT Level, IDirect3DSurface9 **ppSurfaceLevel) {
    if (!ppSurfaceLevel || Level >= levels_) return E_INVALIDARG;
    *ppSurfaceLevel = new GLSurface(this, Level);
    return D3D_OK;
}

HRESULT WINAPI GLTexture::LockRect(UINT Level, D3DLOCKED_RECT *pLockedRect, const RECT *, DWORD) {
    if (!pLockedRect || Level >= levels_) return E_INVALIDARG;
    UINT w = width_  >> Level ? width_  >> Level : 1;
    UINT h = height_ >> Level ? height_ >> Level : 1;
    int  bpp = D3DFormatBpp(format_);
    levelShadow_[Level].assign((size_t)w * h * bpp, 0);
    lockLevel_ = Level;
    dirty_     = true;
    pLockedRect->Pitch = (int)(w * bpp);
    pLockedRect->pBits = levelShadow_[Level].data();
    return D3D_OK;
}

HRESULT WINAPI GLTexture::UnlockRect(UINT Level) {
    if (Level >= levels_ || !dirty_) return D3D_OK;
    unsigned internal, format, type; int bpp;
    if (!D3DToGLFormat(format_, &internal, &format, &type, &bpp)) {
        fprintf(stderr, "[gl] GLTexture: unsupported format 0x%x (level not uploaded)\n", format_);
        dirty_ = false;
        return D3D_OK;
    }
    UINT w = width_  >> Level ? width_  >> Level : 1;
    UINT h = height_ >> Level ? height_ >> Level : 1;
    glBindTexture(GL_TEXTURE_2D, tex_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, Level, internal, w, h, 0, format, type, levelShadow_[Level].data());
    glBindTexture(GL_TEXTURE_2D, 0);
    dirty_ = false;
    return D3D_OK;
}

// ---- GLSurface ------------------------------------------------------------
GLSurface::GLSurface(GLTexture *owner, UINT level)
    : owner_(owner), level_(level),
      width_(owner->width()  >> level ? owner->width()  >> level : 1),
      height_(owner->height() >> level ? owner->height() >> level : 1),
      format_(owner->format()) {}

GLSurface::GLSurface(IDirect3DDevice9 *device, UINT width, UINT height, D3DFORMAT format, bool sysmem)
    : device_(device), width_(width), height_(height), format_(format), sysmem_(sysmem) {
    if (sysmem_) {
        shadow_.assign((size_t)width_ * height_ * D3DFormatBpp(format_), 0);
    } else {
        // Standalone render target: an immutable-storage GL texture.
        unsigned internal, fmt, type; int bpp;
        glGenTextures(1, &ownTex_);
        glBindTexture(GL_TEXTURE_2D, ownTex_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (D3DToGLFormat(format_, &internal, &fmt, &type, &bpp))
            glTexImage2D(GL_TEXTURE_2D, 0, internal, width_, height_, 0, fmt, type, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

GLSurface::~GLSurface() { if (ownTex_) glDeleteTextures(1, &ownTex_); }

unsigned GLSurface::texName() const { return owner_ ? owner_->glName() : ownTex_; }

HRESULT WINAPI GLSurface::GetDevice(IDirect3DDevice9 **ppDevice) {
    if (owner_) return owner_->GetDevice(ppDevice);
    if (!ppDevice) return E_INVALIDARG;
    *ppDevice = device_;
    if (device_) device_->AddRef();
    return D3D_OK;
}

HRESULT WINAPI GLSurface::GetDesc(D3DSURFACE_DESC *pDesc) {
    if (owner_) return owner_->GetLevelDesc(level_, pDesc);
    if (!pDesc) return E_INVALIDARG;
    *pDesc = D3DSURFACE_DESC{};
    pDesc->Format = format_; pDesc->Type = D3DRTYPE_SURFACE;
    pDesc->Usage = sysmem_ ? 0 : D3DUSAGE_RENDERTARGET;
    pDesc->Pool = sysmem_ ? D3DPOOL_SYSTEMMEM : D3DPOOL_DEFAULT;
    pDesc->MultiSampleType = D3DMULTISAMPLE_NONE;
    pDesc->Width = width_; pDesc->Height = height_;
    return D3D_OK;
}

HRESULT WINAPI GLSurface::LockRect(D3DLOCKED_RECT *lr, const RECT *r, DWORD f) {
    if (owner_) return owner_->LockRect(level_, lr, r, f);
    if (!lr) return E_INVALIDARG;
    if (!sysmem_) return E_FAIL;  // only system-memory surfaces are CPU-lockable here
    lr->Pitch = (int)(width_ * D3DFormatBpp(format_));
    lr->pBits = shadow_.data();
    return D3D_OK;
}

HRESULT WINAPI GLSurface::UnlockRect() {
    if (owner_) return owner_->UnlockRect(level_);
    return D3D_OK;  // sysmem: nothing to flush
}

HRESULT WINAPI GLSurface::GetContainer(REFIID, void **ppContainer) {
    if (!ppContainer) return E_INVALIDARG;
    if (owner_) { owner_->AddRef(); *ppContainer = owner_; }
    else        { AddRef();        *ppContainer = this; }
    return D3D_OK;
}

// ---- GLVertexDeclaration --------------------------------------------------
GLVertexDeclaration::GLVertexDeclaration(IDirect3DDevice9 *device,
                                         const D3DVERTEXELEMENT9 *elements)
    : device_(device) {
    // Copy up to (and excluding) the D3DDECL_END terminator (Stream == 0xFF).
    for (const D3DVERTEXELEMENT9 *e = elements; e && e->Stream != 0xFF; ++e)
        elements_.push_back(*e);
}

HRESULT WINAPI GLVertexDeclaration::GetDevice(IDirect3DDevice9 **ppDevice) {
    if (!ppDevice) return E_INVALIDARG;
    *ppDevice = device_;
    if (device_) device_->AddRef();
    return D3D_OK;
}

HRESULT WINAPI GLVertexDeclaration::GetDeclaration(D3DVERTEXELEMENT9 *pElement, UINT *pNumElements) {
    UINT n = (UINT)elements_.size() + 1;  // +1 for the terminator D3D reports
    if (pNumElements) *pNumElements = n;
    if (pElement) {
        for (size_t i = 0; i < elements_.size(); ++i) pElement[i] = elements_[i];
        pElement[elements_.size()] = D3DVERTEXELEMENT9 D3DDECL_END();
    }
    return D3D_OK;
}
