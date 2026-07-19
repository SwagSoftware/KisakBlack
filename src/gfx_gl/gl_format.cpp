// gl_format.cpp — D3DFORMAT → OpenGL format mapping.
#include "gl_format.h"

#include <GL/glew.h>

bool D3DToGLFormat(D3DFORMAT fmt, unsigned *internal, unsigned *format, unsigned *type, int *bpp) {
    // D3D's *A8R8G8B8 is BGRA byte order in memory, so upload it as GL_BGRA.
    switch (fmt) {
        case D3DFMT_A8R8G8B8: *internal = GL_RGBA8; *format = GL_BGRA; *type = GL_UNSIGNED_INT_8_8_8_8_REV; *bpp = 4; return true;
        case D3DFMT_X8R8G8B8: *internal = GL_RGB8;  *format = GL_BGRA; *type = GL_UNSIGNED_INT_8_8_8_8_REV; *bpp = 4; return true;
        case D3DFMT_A8B8G8R8: *internal = GL_RGBA8; *format = GL_RGBA; *type = GL_UNSIGNED_INT_8_8_8_8_REV; *bpp = 4; return true;
        case D3DFMT_R5G6B5:   *internal = GL_RGB5;  *format = GL_RGB;  *type = GL_UNSIGNED_SHORT_5_6_5;     *bpp = 2; return true;
        case D3DFMT_A8:       *internal = GL_R8;    *format = GL_RED;  *type = GL_UNSIGNED_BYTE;            *bpp = 1; return true;
        case D3DFMT_L8:       *internal = GL_R8;    *format = GL_RED;  *type = GL_UNSIGNED_BYTE;            *bpp = 1; return true;
        // A8L8: luminance+alpha. The compatibility context (#version 120 GLSL with
        // gl_FragColor/texture2D) keeps GL_LUMINANCE_ALPHA, which samples as (L,L,L,A)
        // exactly like D3D — so shaders reading .rgb (luminance) and .a (alpha) match.
        case D3DFMT_A8L8:     *internal = GL_LUMINANCE8_ALPHA8; *format = GL_LUMINANCE_ALPHA; *type = GL_UNSIGNED_BYTE; *bpp = 2; return true;
        // Float / deep render-target formats (HDR scene, bloom, depth resolves). These
        // are rendered to and sampled entirely on the GPU, so only the GL internal
        // format matters; an unmapped HDR target gets no storage and samples as garbage
        // (which composites a colour cast onto the 3D scene). D3D's *16R16F etc. are
        // R-first in memory, matching GL_RGBA/RG/RED order.
        case D3DFMT_A16B16G16R16F: *internal = GL_RGBA16F; *format = GL_RGBA; *type = GL_HALF_FLOAT;     *bpp = 8; return true;
        case D3DFMT_A16B16G16R16:  *internal = GL_RGBA16;  *format = GL_RGBA; *type = GL_UNSIGNED_SHORT;  *bpp = 8; return true;
        case D3DFMT_G16R16F:       *internal = GL_RG16F;   *format = GL_RG;   *type = GL_HALF_FLOAT;      *bpp = 4; return true;
        case D3DFMT_G16R16:        *internal = GL_RG16;    *format = GL_RG;   *type = GL_UNSIGNED_SHORT;   *bpp = 4; return true;
        case D3DFMT_R32F:          *internal = GL_R32F;    *format = GL_RED;  *type = GL_FLOAT;            *bpp = 4; return true;
        default: return false;
    }
}

int D3DFormatBpp(D3DFORMAT fmt) {
    unsigned i, f, t; int bpp;
    return D3DToGLFormat(fmt, &i, &f, &t, &bpp) ? bpp : 4;
}

unsigned D3DCompressedGLFormat(D3DFORMAT fmt, int *blockBytes) {
    switch ((unsigned)fmt) {
        case 0x31545844: if (blockBytes) *blockBytes = 8;  return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;  // 'DXT1'
        case 0x33545844: if (blockBytes) *blockBytes = 16; return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;  // 'DXT3'
        case 0x35545844: if (blockBytes) *blockBytes = 16; return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;  // 'DXT5'
        default: return 0;
    }
}
