// gl_d3dx.cpp — GL-backed implementations of the small D3DX surface the renderer
// uses (declared in <d3dx9.h>). Screenshot save + a shader-bytecode buffer.
//
// D3DXSaveSurfaceToFileA reads a surface back via an FBO and writes it out; only
// uncompressed TGA is produced today (the game's screenshot path), and TGA's BGRA
// bottom-up layout matches GL readback. Shader reflection / HLSL compilation
// (D3DXGetShaderConstantTable, D3DXCompileShader) are not implemented: the game
// ships compiled bytecode, and the bytecode→GLSL translator already recovers the
// constant model. They return E_NOTIMPL with a clear marker.
#include "d3dx9.h"
#include "gl_object.h"
#include "gl_resources.h"

#include <GL/glew.h>
#include <cstdio>
#include <vector>

namespace {

class GLD3DXBuffer final : public GLObject<ID3DXBuffer> {
public:
    explicit GLD3DXBuffer(DWORD bytes) : data_(bytes ? bytes : 1) {}
    void *WINAPI GetBufferPointer() override { return data_.data(); }
    DWORD WINAPI GetBufferSize() override { return (DWORD)data_.size(); }
private:
    std::vector<unsigned char> data_;
};

// Read a renderable surface into a BGRA8 CPU buffer via a temporary FBO.
bool ReadSurfaceBGRA(GLSurface *s, int w, int h, std::vector<unsigned char> &out) {
    if (!s || !s->texName()) return false;
    out.assign((size_t)w * h * 4, 0);
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           s->texName(), s->level());
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE, out.data());
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
    return true;
}

}  // namespace

extern "C" {

HRESULT WINAPI D3DXCreateBuffer(DWORD NumBytes, LPD3DXBUFFER *ppBuffer) {
    if (!ppBuffer) return E_INVALIDARG;
    *ppBuffer = new GLD3DXBuffer(NumBytes);
    return D3D_OK;
}

HRESULT WINAPI D3DXSaveSurfaceToFileA(const char *pDestFile, D3DXIMAGE_FILEFORMAT /*fmt*/,
                                      IDirect3DSurface9 *pSrcSurface, const void * /*palette*/,
                                      const RECT * /*srcRect*/) {
    GLSurface *s = static_cast<GLSurface *>(pSrcSurface);
    if (!s || !pDestFile) return E_INVALIDARG;
    int w = (int)s->width(), h = (int)s->height();
    std::vector<unsigned char> bgra;
    if (!ReadSurfaceBGRA(s, w, h, bgra)) return E_FAIL;

    FILE *f = fopen(pDestFile, "wb");
    if (!f) return E_FAIL;
    // 18-byte uncompressed-true-colour TGA header; 32bpp BGRA, 8 alpha bits.
    unsigned char hdr[18] = {0};
    hdr[2]  = 2;
    hdr[12] = (unsigned char)(w & 0xFF); hdr[13] = (unsigned char)((w >> 8) & 0xFF);
    hdr[14] = (unsigned char)(h & 0xFF); hdr[15] = (unsigned char)((h >> 8) & 0xFF);
    hdr[16] = 32; hdr[17] = 8;
    fwrite(hdr, 1, 18, f);
    fwrite(bgra.data(), 1, bgra.size(), f);
    fclose(f);
    return D3D_OK;
}

// --- Not implemented (game ships bytecode; reflection handled by the translator) ---
HRESULT WINAPI D3DXCompileShader(const char *, UINT, const void *, void *, const char *,
                                 const char *, DWORD, LPD3DXBUFFER *, LPD3DXBUFFER *,
                                 LPD3DXCONSTANTTABLE *) { return E_NOTIMPL; }
HRESULT WINAPI D3DXGetShaderConstantTable(const DWORD *, LPD3DXCONSTANTTABLE *pp) { if (pp) *pp = nullptr; return E_NOTIMPL; }
HRESULT WINAPI D3DXGetShaderInputSemantics(const DWORD *, void *, UINT *c) { if (c) *c = 0; return E_NOTIMPL; }
HRESULT WINAPI D3DXGetShaderOutputSemantics(const DWORD *, void *, UINT *c) { if (c) *c = 0; return E_NOTIMPL; }

}  // extern "C"
