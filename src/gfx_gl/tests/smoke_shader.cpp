// smoke_shader.cpp — end-to-end test of the DX9-bytecode→GLSL shader path.
//
// Builds minimal vs_3_0 + ps_3_0 token streams (using the documented DX9 encoding,
// co-designed with the parser in gl_shader.cpp), creates them via CreateVertex/
// PixelShader, binds them, sets a pixel-shader constant, and draws a full-screen
// triangle. The pixel shader outputs c0, set to green, so the centre pixel must be
// green — proving translate → compile → link → constant-upload → draw all work.
#include <d3d9.h>
#include <GL/glew.h>
#include <cstdio>
#include <vector>
#include <cstring>
#include "dx9asm.h"

using namespace dx9;

int main() {
    // vs_3_0:  dcl_position v0;  mov o0, v0   (pass clip-space position through)
    std::vector<DWORD> vs = {
        0xFFFE0300u,
        instr(OP_DCL, 2), /*usage POSITION*/ 0u, dst(RT_INPUT, 0),
        instr(OP_MOV, 2), dst(RT_OUTPUT, 0), src(RT_INPUT, 0),
        0x0000FFFFu,
    };
    // ps_3_0:  mov oC0, c0   (output the c0 constant register)
    std::vector<DWORD> ps = {
        0xFFFF0300u,
        instr(OP_MOV, 2), dst(RT_COLOROUT, 0), src(RT_CONST, 0),
        0x0000FFFFu,
    };

    IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
    D3DPRESENT_PARAMETERS pp = {};
    pp.BackBufferWidth = 64; pp.BackBufferHeight = 64;
    pp.BackBufferFormat = D3DFMT_X8R8G8B8; pp.SwapEffect = D3DSWAPEFFECT_DISCARD; pp.Windowed = TRUE;
    IDirect3DDevice9 *dev = nullptr;
    if (FAILED(d3d->CreateDevice(0, D3DDEVTYPE_HAL, nullptr,
                                 D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &dev)) || !dev) {
        printf("FAIL: CreateDevice\n"); return 1;
    }

    IDirect3DVertexShader9 *vsh = nullptr;
    IDirect3DPixelShader9  *psh = nullptr;
    if (FAILED(dev->CreateVertexShader(vs.data(), &vsh)) ||
        FAILED(dev->CreatePixelShader(ps.data(), &psh)) || !vsh || !psh) {
        printf("FAIL: shader create\n"); return 1;
    }

    // Full-screen triangle in clip space (vs passes position straight through).
    struct V { float x, y, z, w; };
    V tri[3] = { {-1,-1,0,1}, {3,-1,0,1}, {-1,3,0,1} };
    IDirect3DVertexBuffer9 *vb = nullptr;
    dev->CreateVertexBuffer(sizeof(tri), 0, 0, D3DPOOL_DEFAULT, &vb, nullptr);
    void *p = nullptr; vb->Lock(0, 0, &p, 0); memcpy(p, tri, sizeof(tri)); vb->Unlock();

    D3DVERTEXELEMENT9 elems[] = {
        { 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        D3DDECL_END()
    };
    IDirect3DVertexDeclaration9 *decl = nullptr;
    dev->CreateVertexDeclaration(elems, &decl);

    float green[4] = { 0.f, 1.f, 0.f, 1.f };
    D3DVIEWPORT9 vp = { 0, 0, 64, 64, 0.0f, 1.0f };
    dev->SetViewport(&vp);
    dev->BeginScene();
    dev->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    dev->SetVertexShader(vsh);
    dev->SetPixelShader(psh);
    dev->SetPixelShaderConstantF(0, green, 1);
    dev->SetVertexDeclaration(decl);
    dev->SetStreamSource(0, vb, 0, sizeof(V));
    dev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);
    dev->EndScene();

    glFinish();
    unsigned char c[4] = {0};
    glReadPixels(32, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, c);
    dev->Present(nullptr, nullptr, nullptr, nullptr);

    printf("pixel = %d,%d,%d (expect 0,255,0)\n", c[0], c[1], c[2]);
    bool ok = c[0] < 8 && c[1] > 247 && c[2] < 8;

    vsh->Release(); psh->Release(); vb->Release(); decl->Release(); dev->Release(); d3d->Release();
    printf(ok ? "SHADER: PASS\n" : "SHADER: FAIL\n");
    return ok ? 0 : 1;
}
