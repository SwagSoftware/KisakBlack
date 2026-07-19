// gl_shader.h — DX9 shader bytecode → GLSL translation + GL shader objects.
//
// The game ships compiled DX9 SM3 vertex/pixel shader bytecode (in fastfiles).
// TranslateD3D9Shader() parses that token stream and emits GLSL 120; GLVertexShader
// / GLPixelShader wrap the compiled GL shader. At draw time the device links the
// bound (vs, ps) pair into a program and uploads the c# constant registers as the
// `vsc[]` / `psc[]` uniform arrays.
//
// Opcode coverage is the common arithmetic + sampling subset (mov/add/sub/mad/mul/
// rcp/rsq/dp3/dp4/min/max/frc/texld, plus dcl/def); unsupported opcodes make the
// translator return false with a diagnostic so gaps are visible, not silent.
//
// NOTE: token encodings follow the documented DX9 bytecode format and are verified
// end-to-end by tests/smoke_shader.cpp. They still need validation against real
// fastfile shaders once those are available (tracked in the README).
#ifndef KISAK_GL_SHADER_H
#define KISAK_GL_SHADER_H

#include "gl_object.h"
#include <string>

// Translate a DX9 shader token stream to GLSL 120. On success fills `glslOut` and
// `*outIsPixel`; on an unsupported token returns false (glslOut holds a comment).
bool TranslateD3D9Shader(const DWORD *tokens, std::string &glslOut, bool *outIsPixel);

class GLVertexShader final : public GLObject<IDirect3DVertexShader9> {
public:
    GLVertexShader(IDirect3DDevice9 *device, const DWORD *function);
    ~GLVertexShader() override;
    HRESULT WINAPI GetDevice(IDirect3DDevice9 **ppDevice) override;
    HRESULT WINAPI GetFunction(void *, UINT *pSize) override { if (pSize) *pSize = 0; return D3D_OK; }
    unsigned glShader() const { return shader_; }
    bool ok() const { return shader_ != 0; }
private:
    IDirect3DDevice9 *device_;
    unsigned          shader_ = 0;
    std::string       glsl_;
};

class GLPixelShader final : public GLObject<IDirect3DPixelShader9> {
public:
    GLPixelShader(IDirect3DDevice9 *device, const DWORD *function);
    ~GLPixelShader() override;
    HRESULT WINAPI GetDevice(IDirect3DDevice9 **ppDevice) override;
    HRESULT WINAPI GetFunction(void *, UINT *pSize) override { if (pSize) *pSize = 0; return D3D_OK; }
    unsigned glShader() const { return shader_; }
    bool ok() const { return shader_ != 0; }
private:
    IDirect3DDevice9 *device_;
    unsigned          shader_ = 0;
    std::string       glsl_;
};

#endif // KISAK_GL_SHADER_H
