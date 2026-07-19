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
        default: return false;
    }
}

int D3DFormatBpp(D3DFORMAT fmt) {
    unsigned i, f, t; int bpp;
    return D3DToGLFormat(fmt, &i, &f, &t, &bpp) ? bpp : 4;
}
