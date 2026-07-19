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
// D3D's CreateTexture(Levels=0) means "full mip chain"; compute it from the size.
static UINT FullMipCount(UINT w, UINT h) {
    UINT m = (w > h) ? w : h, n = 1;
    while (m > 1) { m >>= 1; ++n; }
    return n;
}

GLTexture::GLTexture(IDirect3DDevice9 *device, UINT width, UINT height, UINT levels,
                     DWORD usage, D3DFORMAT format, D3DPOOL pool)
    : device_(device), width_(width), height_(height),
      levels_(levels ? levels : FullMipCount(width, height)), usage_(usage), format_(format), pool_(pool) {
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
    } else {
        // Give every texture a neutral 1x1 level-0 immediately so it is COMPLETE even
        // before (or if) the engine uploads its pixels — otherwise a not-yet-uploaded
        // (e.g. streamed) texture is incomplete and samples as a debug colour (magenta
        // on Mesa). The real LockRect/UnlockRect redefines level 0 with the true data.
        static const unsigned char gray[4] = { 128, 128, 128, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, gray);
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
    int blockBytes = 0;
    if (D3DCompressedGLFormat(format_, &blockBytes)) {
        // DXT/BC: the lock surface is a grid of 4x4 blocks; Pitch is bytes per block row.
        UINT bw = (w + 3) / 4, bh = (h + 3) / 4;
        levelShadow_[Level].assign((size_t)bw * bh * blockBytes, 0);
        pLockedRect->Pitch = (int)(bw * blockBytes);
    } else {
        int bpp = D3DFormatBpp(format_);
        levelShadow_[Level].assign((size_t)w * h * bpp, 0);
        pLockedRect->Pitch = (int)(w * bpp);
    }
    lockLevel_ = Level;
    dirty_     = true;
    pLockedRect->pBits = levelShadow_[Level].data();
    return D3D_OK;
}

HRESULT WINAPI GLTexture::UnlockRect(UINT Level) {
    if (Level >= levels_ || !dirty_) return D3D_OK;
    UINT w = width_  >> Level ? width_  >> Level : 1;
    UINT h = height_ >> Level ? height_ >> Level : 1;
    // One-time GPU capability report — tells us if S3TC (DXT) uploads can work.
    static bool reported = false;
    if (!reported) {
        reported = true;
        const char *ext = (const char *)glGetString(GL_EXTENSIONS);
        bool s3tc = ext && strstr(ext, "texture_compression_s3tc");
        fprintf(stderr, "[gl] renderer=%s | GL=%s | S3TC=%s\n",
                glGetString(GL_RENDERER), glGetString(GL_VERSION), s3tc ? "YES" : "NO");
    }
    glBindTexture(GL_TEXTURE_2D, tex_);
    while (glGetError() != GL_NO_ERROR) {}  // drain prior errors
    int blockBytes = 0; unsigned cfmt = D3DCompressedGLFormat(format_, &blockBytes);
    if (cfmt) {
        glCompressedTexImage2D(GL_TEXTURE_2D, Level, cfmt, w, h, 0,
                               (GLsizei)levelShadow_[Level].size(), levelShadow_[Level].data());
    } else {
        unsigned internal, format, type; int bpp;
        if (!D3DToGLFormat(format_, &internal, &format, &type, &bpp)) {
            fprintf(stderr, "[gl] GLTexture: unsupported format 0x%x (level not uploaded)\n", format_);
            glBindTexture(GL_TEXTURE_2D, 0); dirty_ = false; return D3D_OK;
        }
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, Level, internal, w, h, 0, format, type, levelShadow_[Level].data());
    }
    GLenum uerr = glGetError();
    glBindTexture(GL_TEXTURE_2D, 0);
    dirty_ = false;
    if (uerr != GL_NO_ERROR)
        fprintf(stderr, "[gl] texture upload error 0x%x: L%u fmt=0x%x %ux%u %s\n",
                uerr, Level, (unsigned)format_, w, h, cfmt ? "DXT" : "raw");
    return D3D_OK;
}

// ---- GLVolumeTexture (GL_TEXTURE_3D) --------------------------------------
GLVolumeTexture::GLVolumeTexture(IDirect3DDevice9 *device, UINT w, UINT h, UINT d, UINT levels,
                                 DWORD usage, D3DFORMAT format, D3DPOOL pool)
    : device_(device), width_(w), height_(h), depth_(d),
      levels_(levels ? levels : 1), usage_(usage), format_(format), pool_(pool) {
    glGenTextures(1, &tex_);
    glBindTexture(GL_TEXTURE_3D, tex_);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, levels_ - 1);
    levelShadow_.resize(levels_);
    glBindTexture(GL_TEXTURE_3D, 0);
}
GLVolumeTexture::~GLVolumeTexture() { if (tex_) glDeleteTextures(1, &tex_); }

HRESULT WINAPI GLVolumeTexture::GetDevice(IDirect3DDevice9 **ppDevice) {
    if (!ppDevice) return E_INVALIDARG;
    *ppDevice = device_; if (device_) device_->AddRef(); return D3D_OK;
}
HRESULT WINAPI GLVolumeTexture::GetLevelDesc(UINT Level, D3DVOLUME_DESC *pDesc) {
    if (!pDesc || Level >= levels_) return E_INVALIDARG;
    *pDesc = D3DVOLUME_DESC{};
    pDesc->Format = format_; pDesc->Type = D3DRTYPE_VOLUME; pDesc->Usage = usage_; pDesc->Pool = pool_;
    pDesc->Width  = width_  >> Level ? width_  >> Level : 1;
    pDesc->Height = height_ >> Level ? height_ >> Level : 1;
    pDesc->Depth  = depth_  >> Level ? depth_  >> Level : 1;
    return D3D_OK;
}
HRESULT WINAPI GLVolumeTexture::LockBox(UINT Level, D3DLOCKED_BOX *pLockedVolume, const D3DBOX *, DWORD) {
    if (!pLockedVolume || Level >= levels_) return E_INVALIDARG;
    UINT w = width_  >> Level ? width_  >> Level : 1;
    UINT h = height_ >> Level ? height_ >> Level : 1;
    UINT d = depth_  >> Level ? depth_  >> Level : 1;
    int bpp = D3DFormatBpp(format_);
    levelShadow_[Level].assign((size_t)w * h * d * bpp, 0);
    dirty_ = true;
    pLockedVolume->RowPitch   = (int)(w * bpp);
    pLockedVolume->SlicePitch = (int)(w * h * bpp);
    pLockedVolume->pBits      = levelShadow_[Level].data();
    return D3D_OK;
}
HRESULT WINAPI GLVolumeTexture::UnlockBox(UINT Level) {
    if (Level >= levels_ || !dirty_) return D3D_OK;
    UINT w = width_  >> Level ? width_  >> Level : 1;
    UINT h = height_ >> Level ? height_ >> Level : 1;
    UINT d = depth_  >> Level ? depth_  >> Level : 1;
    unsigned internal, format, type; int bpp;
    if (D3DToGLFormat(format_, &internal, &format, &type, &bpp)) {
        glBindTexture(GL_TEXTURE_3D, tex_);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage3D(GL_TEXTURE_3D, Level, internal, w, h, d, 0, format, type, levelShadow_[Level].data());
        glBindTexture(GL_TEXTURE_3D, 0);
    } else {
        fprintf(stderr, "[gl] GLVolumeTexture: unsupported format 0x%x\n", format_);
    }
    dirty_ = false;
    return D3D_OK;
}

// ---- GLCubeTexture (GL_TEXTURE_CUBE_MAP) ----------------------------------
GLCubeTexture::GLCubeTexture(IDirect3DDevice9 *device, UINT edgeLen, UINT levels,
                             DWORD usage, D3DFORMAT format, D3DPOOL pool)
    : device_(device), edge_(edgeLen),
      levels_(levels ? levels : FullMipCount(edgeLen, edgeLen)), usage_(usage), format_(format), pool_(pool) {
    glGenTextures(1, &tex_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex_);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                    levels_ > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, levels_ - 1);
    for (auto &face : levelShadow_) face.resize(levels_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
GLCubeTexture::~GLCubeTexture() { if (tex_) glDeleteTextures(1, &tex_); }

HRESULT WINAPI GLCubeTexture::GetDevice(IDirect3DDevice9 **ppDevice) {
    if (!ppDevice) return E_INVALIDARG;
    *ppDevice = device_; if (device_) device_->AddRef(); return D3D_OK;
}
HRESULT WINAPI GLCubeTexture::GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) {
    if (!pDesc || Level >= levels_) return E_INVALIDARG;
    *pDesc = D3DSURFACE_DESC{};
    pDesc->Format = format_; pDesc->Type = D3DRTYPE_SURFACE; pDesc->Usage = usage_; pDesc->Pool = pool_;
    pDesc->MultiSampleType = D3DMULTISAMPLE_NONE;
    UINT e = edge_ >> Level ? edge_ >> Level : 1;
    pDesc->Width = e; pDesc->Height = e;
    return D3D_OK;
}
HRESULT WINAPI GLCubeTexture::GetCubeMapSurface(D3DCUBEMAP_FACES face, UINT level,
                                                IDirect3DSurface9 **ppSurface) {
    if (!ppSurface || (unsigned)face >= 6 || level >= levels_) return E_INVALIDARG;
    *ppSurface = new GLSurface(this, face, level);
    return D3D_OK;
}
HRESULT WINAPI GLCubeTexture::LockRect(D3DCUBEMAP_FACES FaceType, UINT Level,
                                       D3DLOCKED_RECT *pLockedRect, const RECT *, DWORD) {
    if (!pLockedRect || (unsigned)FaceType >= 6 || Level >= levels_) return E_INVALIDARG;
    UINT e = edge_ >> Level ? edge_ >> Level : 1;
    std::vector<unsigned char> &shadow = levelShadow_[FaceType][Level];
    int blockBytes = 0;
    if (D3DCompressedGLFormat(format_, &blockBytes)) {
        UINT bw = (e + 3) / 4, bh = (e + 3) / 4;
        shadow.assign((size_t)bw * bh * blockBytes, 0);
        pLockedRect->Pitch = (int)(bw * blockBytes);
    } else {
        int bpp = D3DFormatBpp(format_);
        shadow.assign((size_t)e * e * bpp, 0);
        pLockedRect->Pitch = (int)(e * bpp);
    }
    dirty_ = true;
    pLockedRect->pBits = shadow.data();
    return D3D_OK;
}
HRESULT WINAPI GLCubeTexture::UnlockRect(D3DCUBEMAP_FACES FaceType, UINT Level) {
    if ((unsigned)FaceType >= 6 || Level >= levels_ || !dirty_) return D3D_OK;
    UINT e = edge_ >> Level ? edge_ >> Level : 1;
    std::vector<unsigned char> &shadow = levelShadow_[FaceType][Level];
    GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + (unsigned)FaceType;
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex_);
    int blockBytes = 0; unsigned cfmt = D3DCompressedGLFormat(format_, &blockBytes);
    if (cfmt) {
        glCompressedTexImage2D(target, Level, cfmt, e, e, 0,
                               (GLsizei)shadow.size(), shadow.data());
    } else {
        unsigned internal, format, type; int bpp;
        if (!D3DToGLFormat(format_, &internal, &format, &type, &bpp)) {
            fprintf(stderr, "[gl] GLCubeTexture: unsupported format 0x%x (face %d level %d not uploaded)\n",
                    format_, (int)FaceType, (int)Level);
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0); dirty_ = false; return D3D_OK;
        }
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(target, Level, internal, e, e, 0, format, type, shadow.data());
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    dirty_ = false;
    return D3D_OK;
}

// ---- GLSurface ------------------------------------------------------------
GLSurface::GLSurface(GLTexture *owner, UINT level)
    : owner_(owner), level_(level),
      width_(owner->width()  >> level ? owner->width()  >> level : 1),
      height_(owner->height() >> level ? owner->height() >> level : 1),
      format_(owner->format()) {}

GLSurface::GLSurface(GLCubeTexture *owner, D3DCUBEMAP_FACES face, UINT level)
    : cubeOwner_(owner), cubeFace_(face), level_(level),
      width_(owner->edgeLength() >> level ? owner->edgeLength() >> level : 1),
      height_(owner->edgeLength() >> level ? owner->edgeLength() >> level : 1),
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

GLSurface::GLSurface(IDirect3DDevice9 *device, UINT width, UINT height, D3DFORMAT format, GLBackbufferTag)
    : device_(device), width_(width), height_(height), format_(format), backbuffer_(true) {}

GLSurface::GLSurface(IDirect3DDevice9 *device, UINT width, UINT height, D3DFORMAT format, GLDepthStencilTag)
    : device_(device), width_(width), height_(height), format_(format), depthStencil_(true) {}

GLSurface::~GLSurface() { if (ownTex_) glDeleteTextures(1, &ownTex_); }

unsigned GLSurface::texName() const {
    if (owner_) return owner_->glName();
    if (cubeOwner_) return cubeOwner_->glName();
    return ownTex_;
}

HRESULT WINAPI GLSurface::GetDevice(IDirect3DDevice9 **ppDevice) {
    if (owner_) return owner_->GetDevice(ppDevice);
    if (cubeOwner_) return cubeOwner_->GetDevice(ppDevice);
    if (!ppDevice) return E_INVALIDARG;
    *ppDevice = device_;
    if (device_) device_->AddRef();
    return D3D_OK;
}

HRESULT WINAPI GLSurface::GetDesc(D3DSURFACE_DESC *pDesc) {
    if (owner_) return owner_->GetLevelDesc(level_, pDesc);
    if (cubeOwner_) return cubeOwner_->GetLevelDesc(level_, pDesc);
    if (!pDesc) return E_INVALIDARG;
    *pDesc = D3DSURFACE_DESC{};
    pDesc->Format = format_; pDesc->Type = D3DRTYPE_SURFACE;
    pDesc->Usage = depthStencil_ ? D3DUSAGE_DEPTHSTENCIL : (sysmem_ ? 0 : D3DUSAGE_RENDERTARGET);
    pDesc->Pool = sysmem_ ? D3DPOOL_SYSTEMMEM : D3DPOOL_DEFAULT;
    pDesc->MultiSampleType = D3DMULTISAMPLE_NONE;
    pDesc->Width = width_; pDesc->Height = height_;
    return D3D_OK;
}

HRESULT WINAPI GLSurface::LockRect(D3DLOCKED_RECT *lr, const RECT *r, DWORD f) {
    if (owner_) return owner_->LockRect(level_, lr, r, f);
    if (cubeOwner_) return cubeOwner_->LockRect(cubeFace_, level_, lr, r, f);
    if (!lr) return E_INVALIDARG;
    if (!sysmem_) return E_FAIL;  // only system-memory surfaces are CPU-lockable here
    lr->Pitch = (int)(width_ * D3DFormatBpp(format_));
    lr->pBits = shadow_.data();
    return D3D_OK;
}

HRESULT WINAPI GLSurface::UnlockRect() {
    if (owner_) return owner_->UnlockRect(level_);
    if (cubeOwner_) return cubeOwner_->UnlockRect(cubeFace_, level_);
    return D3D_OK;  // sysmem: nothing to flush
}

HRESULT WINAPI GLSurface::GetContainer(REFIID, void **ppContainer) {
    if (!ppContainer) return E_INVALIDARG;
    if (owner_) { owner_->AddRef(); *ppContainer = owner_; }
    else if (cubeOwner_) { cubeOwner_->AddRef(); *ppContainer = cubeOwner_; }
    else                 { AddRef();             *ppContainer = this; }
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
