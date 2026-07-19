// gl_d3d9_draw.cpp — geometry resource creation + the draw path for GLDevice.
//
// The built-in GLSL program here only covers the *pre-transformed* (D3DFVF_XYZRHW /
// D3DDECLUSAGE_POSITIONT) vertex path — i.e. the 2D / fixed-function geometry the
// engine submits in screen space. Programmable 3D shaders go through the DX9
// bytecode→GLSL translator instead (task #5); when a vertex shader is bound this
// built-in is bypassed.
#include "gl_d3d9.h"
#include "gl_resources.h"
#include "gl_shader.h"   // GLAttribLocation / GLAttribName / GLBindAttribLocations

#include <GL/glew.h>
#include <cstdio>

namespace {

// The built-in 2D program consumes the canonical POSITION / COLOR0 / TEXCOORD0
// attribute names (see GLAttribName in gl_shader.cpp), so the same vertex setup
// drives it and the translated programs.
const char *kBuiltinVS =
    "#version 120\n"
    "attribute vec4 aPos;\n"        // POSITIONT: x,y in screen pixels, z depth, w=rhw
    "attribute vec4 aColor0;\n"
    "attribute vec2 aTexCoord0;\n"
    "uniform vec2 uViewport;\n"
    "varying vec4 vColor;\n"
    "varying vec2 vTexCoord;\n"
    "void main() {\n"
    "  float x = (aPos.x / uViewport.x) * 2.0 - 1.0;\n"
    "  float y = 1.0 - (aPos.y / uViewport.y) * 2.0;\n"  // D3D top-left -> GL bottom-left
    "  gl_Position = vec4(x, y, aPos.z, 1.0);\n"
    "  vColor = aColor0;\n"
    "  vTexCoord = aTexCoord0;\n"
    "}\n";

const char *kBuiltinFS =
    "#version 120\n"
    "uniform sampler2D uTex;\n"
    "uniform int uUseTexture;\n"
    "uniform int uColorOp;\n"    // 0 = SELECTARG1 (texture only), 1 = MODULATE (tex*diffuse)
    "uniform int uAlphaFunc;\n"  // GL compare func (GL_NEVER..GL_ALWAYS), 0 = alpha test off
    "uniform float uAlphaRef;\n" // reference in [0,1]
    "varying vec4 vColor;\n"
    "varying vec2 vTexCoord;\n"
    "bool alphaPass(float a) {\n"
    "  if (uAlphaFunc == 0) return true;\n"            // disabled
    "  if (uAlphaFunc == 0x0200) return false;\n"      // GL_NEVER
    "  if (uAlphaFunc == 0x0201) return a <  uAlphaRef;\n"  // GL_LESS
    "  if (uAlphaFunc == 0x0202) return a == uAlphaRef;\n"  // GL_EQUAL
    "  if (uAlphaFunc == 0x0203) return a <= uAlphaRef;\n"  // GL_LEQUAL
    "  if (uAlphaFunc == 0x0204) return a >  uAlphaRef;\n"  // GL_GREATER
    "  if (uAlphaFunc == 0x0205) return a != uAlphaRef;\n"  // GL_NOTEQUAL
    "  if (uAlphaFunc == 0x0206) return a >= uAlphaRef;\n"  // GL_GEQUAL
    "  return true;\n"                                  // GL_ALWAYS (0x0207) / default
    "}\n"
    "void main() {\n"
    "  vec4 c = vColor;\n"
    "  if (uUseTexture != 0) {\n"
    "    vec4 t = texture2D(uTex, vTexCoord);\n"
    "    c = (uColorOp == 0) ? t : (t * vColor);\n"  // SELECTARG1(tex) vs MODULATE(tex*diffuse)
    "  }\n"
    "  if (!alphaPass(c.a)) discard;\n"
    "  gl_FragColor = c;\n"
    "}\n";

unsigned compile(GLenum stage, const char *src) {
    unsigned s = glCreateShader(stage);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        fprintf(stderr, "[gl] built-in shader compile failed: %s\n", log);
    }
    return s;
}

// D3DDECLTYPE -> (component count, GL type, normalized). CoD's vertex formats
// pack normals/tangents (UBYTE4N/DEC3N), colours (UBYTE4N) and texcoords
// (FLOAT16_2/4) — decoding any of these as plain floats yields garbage normals
// (dark/flat lighting, half-black triangles) and garbage UVs (skybox moire).
void declType(BYTE t, GLint *size, GLenum *type, GLboolean *norm) {
    *norm = GL_FALSE;
    switch (t) {
        case D3DDECLTYPE_FLOAT1:    *size = 1; *type = GL_FLOAT;          break;
        case D3DDECLTYPE_FLOAT2:    *size = 2; *type = GL_FLOAT;          break;
        case D3DDECLTYPE_FLOAT3:    *size = 3; *type = GL_FLOAT;          break;
        case D3DDECLTYPE_FLOAT4:    *size = 4; *type = GL_FLOAT;          break;
        case D3DDECLTYPE_D3DCOLOR:  *size = GL_BGRA; *type = GL_UNSIGNED_BYTE; *norm = GL_TRUE; break;
        case D3DDECLTYPE_UBYTE4:    *size = 4; *type = GL_UNSIGNED_BYTE;  break;
        case D3DDECLTYPE_UBYTE4N:   *size = 4; *type = GL_UNSIGNED_BYTE;  *norm = GL_TRUE; break;
        case D3DDECLTYPE_SHORT2:    *size = 2; *type = GL_SHORT;          break;
        case D3DDECLTYPE_SHORT4:    *size = 4; *type = GL_SHORT;          break;
        case D3DDECLTYPE_SHORT2N:   *size = 2; *type = GL_SHORT;          *norm = GL_TRUE; break;
        case D3DDECLTYPE_SHORT4N:   *size = 4; *type = GL_SHORT;          *norm = GL_TRUE; break;
        case D3DDECLTYPE_USHORT2N:  *size = 2; *type = GL_UNSIGNED_SHORT; *norm = GL_TRUE; break;
        case D3DDECLTYPE_USHORT4N:  *size = 4; *type = GL_UNSIGNED_SHORT; *norm = GL_TRUE; break;
        // 3 components packed 10:10:10:2 (low bits = x). REV matches D3D's order.
        case D3DDECLTYPE_UDEC3:     *size = 4; *type = GL_UNSIGNED_INT_2_10_10_10_REV; break;
        case D3DDECLTYPE_DEC3N:     *size = 4; *type = GL_INT_2_10_10_10_REV; *norm = GL_TRUE; break;
        case D3DDECLTYPE_FLOAT16_2: *size = 2; *type = GL_HALF_FLOAT;     break;
        case D3DDECLTYPE_FLOAT16_4: *size = 4; *type = GL_HALF_FLOAT;     break;
        default:                    *size = 4; *type = GL_FLOAT;          break;
    }
}


void primInfo(D3DPRIMITIVETYPE pt, UINT primCount, GLenum *mode, GLsizei *verts) {
    switch (pt) {
        case D3DPT_POINTLIST:     *mode = GL_POINTS;         *verts = primCount;     break;
        case D3DPT_LINELIST:      *mode = GL_LINES;          *verts = primCount * 2; break;
        case D3DPT_LINESTRIP:     *mode = GL_LINE_STRIP;     *verts = primCount + 1; break;
        case D3DPT_TRIANGLELIST:  *mode = GL_TRIANGLES;      *verts = primCount * 3; break;
        case D3DPT_TRIANGLESTRIP: *mode = GL_TRIANGLE_STRIP; *verts = primCount + 2; break;
        case D3DPT_TRIANGLEFAN:   *mode = GL_TRIANGLE_FAN;   *verts = primCount + 2; break;
        default:                  *mode = GL_TRIANGLES;      *verts = primCount * 3; break;
    }
}

} // namespace

// ---- Resource creation ----------------------------------------------------
HRESULT WINAPI GLDevice::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage,
                                       D3DFORMAT Format, D3DPOOL Pool,
                                       IDirect3DTexture9 **ppTexture, HANDLE *) {
    if (!ppTexture) return E_INVALIDARG;
    *ppTexture = new GLTexture(this, Width, Height, Levels, Usage, Format, Pool);
    return D3D_OK;
}

HRESULT WINAPI GLDevice::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage,
                                             D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9 **ppVolumeTexture, HANDLE *) {
    if (!ppVolumeTexture) return E_INVALIDARG;
    *ppVolumeTexture = new GLVolumeTexture(this, Width, Height, Depth, Levels, Usage, Format, Pool);
    return D3D_OK;
}

HRESULT WINAPI GLDevice::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format,
                                           D3DPOOL Pool, IDirect3DCubeTexture9 **ppCubeTexture, HANDLE *) {
    if (!ppCubeTexture) return E_INVALIDARG;
    *ppCubeTexture = new GLCubeTexture(this, EdgeLength, Levels, Usage, Format, Pool);
    return D3D_OK;
}

HRESULT WINAPI GLDevice::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool,
                                            IDirect3DVertexBuffer9 **ppVB, HANDLE *) {
    if (!ppVB) return E_INVALIDARG;
    *ppVB = new GLVertexBuffer(this, Length, Usage, FVF, Pool);
    return D3D_OK;
}

HRESULT WINAPI GLDevice::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
                                           IDirect3DIndexBuffer9 **ppIB, HANDLE *) {
    if (!ppIB) return E_INVALIDARG;
    *ppIB = new GLIndexBuffer(this, Length, Usage, Format, Pool);
    return D3D_OK;
}

HRESULT WINAPI GLDevice::CreateVertexDeclaration(const D3DVERTEXELEMENT9 *pElements,
                                                 IDirect3DVertexDeclaration9 **ppDecl) {
    if (!ppDecl || !pElements) return E_INVALIDARG;
    *ppDecl = new GLVertexDeclaration(this, pElements);
    return D3D_OK;
}

// ---- Geometry binding (device holds non-owning refs; the app owns lifetime) -
HRESULT WINAPI GLDevice::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9 *pStreamData,
                                         UINT OffsetInBytes, UINT Stride) {
    if (StreamNumber >= 4) return D3D_OK;  // TODO: support >4 streams if needed
    streams_[StreamNumber].vb     = static_cast<GLVertexBuffer *>(pStreamData);
    streams_[StreamNumber].offset = OffsetInBytes;
    streams_[StreamNumber].stride = Stride;
    return D3D_OK;
}

HRESULT WINAPI GLDevice::SetIndices(IDirect3DIndexBuffer9 *pIndexData) {
    ib_ = static_cast<GLIndexBuffer *>(pIndexData);
    return D3D_OK;
}

HRESULT WINAPI GLDevice::SetVertexDeclaration(IDirect3DVertexDeclaration9 *pDecl) {
    decl_ = static_cast<GLVertexDeclaration *>(pDecl);
    return D3D_OK;
}

// ---- Draw path ------------------------------------------------------------
void GLDevice::ensureBuiltinProgram() {
    if (builtinProg_) return;
    unsigned vs = compile(GL_VERTEX_SHADER, kBuiltinVS);
    unsigned fs = compile(GL_FRAGMENT_SHADER, kBuiltinFS);
    builtinProg_ = glCreateProgram();
    glAttachShader(builtinProg_, vs);
    glAttachShader(builtinProg_, fs);
    GLBindAttribLocations(builtinProg_);
    glLinkProgram(builtinProg_);
    glDeleteShader(vs);
    glDeleteShader(fs);
    builtinViewportLoc_  = glGetUniformLocation(builtinProg_, "uViewport");
    builtinTexLoc_       = glGetUniformLocation(builtinProg_, "uTex");
    builtinUseTexLoc_    = glGetUniformLocation(builtinProg_, "uUseTexture");
    builtinColorOpLoc_   = glGetUniformLocation(builtinProg_, "uColorOp");
    builtinAlphaFuncLoc_ = glGetUniformLocation(builtinProg_, "uAlphaFunc");
    builtinAlphaRefLoc_  = glGetUniformLocation(builtinProg_, "uAlphaRef");
}

// D3DCMP_* -> the GL compare-func enum (GL_NEVER..GL_ALWAYS). The built-in
// fragment shader emulates the alpha test against these values; 0 means the test
// is disabled.
static GLenum alphaFuncToGL(DWORD d3dCmp) {
    switch (d3dCmp) {
        case D3DCMP_NEVER:        return GL_NEVER;
        case D3DCMP_LESS:         return GL_LESS;
        case D3DCMP_EQUAL:        return GL_EQUAL;
        case D3DCMP_LESSEQUAL:    return GL_LEQUAL;
        case D3DCMP_GREATER:      return GL_GREATER;
        case D3DCMP_NOTEQUAL:     return GL_NOTEQUAL;
        case D3DCMP_GREATEREQUAL: return GL_GEQUAL;
        default:                  return GL_ALWAYS;
    }
}

// Bind the built-in program and set its frame/texture uniforms before a draw.
void GLDevice::bindBuiltinForDraw() {
    ensureBuiltinProgram();
    glUseProgram(builtinProg_);
    if (builtinViewportLoc_ >= 0)
        glUniform2f(builtinViewportLoc_, (float)fbWidth_, (float)fbHeight_);
    bool sampling = applyTextures();
    if (builtinTexLoc_ >= 0)    glUniform1i(builtinTexLoc_, 0);
    if (builtinUseTexLoc_ >= 0) glUniform1i(builtinUseTexLoc_, sampling ? 1 : 0);

    // Fixed-function stage-0 combine: SELECTARG1 (texture only) vs MODULATE
    // (texture * diffuse). Both arg slots default to the (TEXTURE, DIFFUSE) pair,
    // which is all the engine's 2D path uses.
    if (builtinColorOpLoc_ >= 0)
        glUniform1i(builtinColorOpLoc_, texStage0_.colorOp == D3DTOP_SELECTARG1 ? 0 : 1);

    // Alpha test (emulated by discard). Pass the GL compare func, or 0 to disable.
    if (builtinAlphaFuncLoc_ >= 0)
        glUniform1i(builtinAlphaFuncLoc_, alphaTest_.enable ? (int)alphaFuncToGL(alphaTest_.func) : 0);
    if (builtinAlphaRefLoc_ >= 0)
        glUniform1f(builtinAlphaRefLoc_, (float)alphaTest_.ref / 255.0f);
}

void GLDevice::applyVertexState() {
    if (!vao_) glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    for (int i = 0; i < 16; ++i) glDisableVertexAttribArray(i);
    // D3D's default (unspecified) diffuse colour is white; GL's default generic
    // attribute is (0,0,0,1). Without this, a texcoord-only vertex would multiply
    // the texture by black. The array, if the decl supplies COLOR0, overrides this.
    glVertexAttrib4f(GLAttribLocation(D3DDECLUSAGE_COLOR, 0), 1.0f, 1.0f, 1.0f, 1.0f);
    if (!decl_) return;

    for (const D3DVERTEXELEMENT9 &e : decl_->elements()) {
        int loc = GLAttribLocation(e.Usage, e.UsageIndex);
        if (loc < 0 || e.Stream >= 4) continue;
        const Stream &s = streams_[e.Stream];
        if (!s.vb) continue;

        GLint size; GLenum type; GLboolean norm;
        declType(e.Type, &size, &type, &norm);
        glBindBuffer(GL_ARRAY_BUFFER, s.vb->glName());
        glEnableVertexAttribArray(loc);
        glVertexAttribPointer(loc, size, type, norm, s.stride,
                              reinterpret_cast<const void *>(size_t(s.offset + e.Offset)));
    }
}

HRESULT WINAPI GLDevice::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex,
                                       UINT PrimitiveCount) {
    useDrawProgram();
    applyVertexState();

    GLenum mode; GLsizei verts;
    primInfo(PrimitiveType, PrimitiveCount, &mode, &verts);
    glDrawArrays(mode, (GLint)StartVertex, verts);
    return D3D_OK;
}

HRESULT WINAPI GLDevice::DrawIndexedPrimitive(D3DPRIMITIVETYPE Type, INT BaseVertexIndex,
                                              UINT /*MinVertexIndex*/, UINT /*NumVertices*/,
                                              UINT startIndex, UINT primCount) {
    if (!ib_) return D3D_OK;
    useDrawProgram();
    applyVertexState();

    GLenum mode; GLsizei verts;
    primInfo(Type, primCount, &mode, &verts);

    bool is16 = (ib_->format() == D3DFMT_INDEX16);
    GLenum idxType = is16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
    size_t idxSize = is16 ? 2 : 4;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib_->glName());
    const void *offset = reinterpret_cast<const void *>(size_t(startIndex) * idxSize);
    glDrawElementsBaseVertex(mode, verts, idxType, const_cast<void *>(offset), BaseVertexIndex);
    return D3D_OK;
}
