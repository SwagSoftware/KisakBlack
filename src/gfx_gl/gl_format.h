// gl_format.h — D3DFORMAT → OpenGL format mapping (shared by resources + surface ops).
//
// GL enums are returned as plain `unsigned` so callers that don't need GL types
// (and this header) stay free of any GL include.
#ifndef KISAK_GL_FORMAT_H
#define KISAK_GL_FORMAT_H

#include <d3d9.h>

// Fills GL (internalFormat, uploadFormat, uploadType, bytesPerPixel) for `fmt`.
// Returns false for formats not yet handled (e.g. compressed DXT).
bool D3DToGLFormat(D3DFORMAT fmt, unsigned *internalFormat, unsigned *glFormat,
                   unsigned *glType, int *bytesPerPixel);

// Bytes per pixel for `fmt` (4 if unknown).
int D3DFormatBpp(D3DFORMAT fmt);

#endif // KISAK_GL_FORMAT_H
