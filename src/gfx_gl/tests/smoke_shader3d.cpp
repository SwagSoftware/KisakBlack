// smoke_shader3d.cpp — vertex shader that transforms position by a constant matrix.
//
// This is the pattern every game vertex shader uses: four dp4s against the four
// rows of a matrix held in vertex-shader constants (c0..c3), with per-component
// write masks. It exercises dp4, write masks, and both constant banks. The matrix
// is identity, so a full-screen triangle stays full-screen; the pixel shader emits
// its own c0 (green), so the centre pixel must be green.
#include <d3d9.h>
#include <GL/glew.h>
#include <cstdio>
#include <vector>
#include <cstring>
#include "dx9asm.h"

using namespace dx9;

int main() {
    // vs_3_0: dcl_position v0; dp4 o0.x,v0,c0; .y,c1; .z,c2; .w,c3
    std::vector<DWORD> vs = {
        vsVersion(),
        instr(OP_DCL, 2), 0u /*POSITION*/, dst(RT_INPUT, 0),
        instr(OP_DP4, 3), dstMask(RT_OUTPUT, 0, WM_X), src(RT_INPUT, 0), src(RT_CONST, 0),
        instr(OP_DP4, 3), dstMask(RT_OUTPUT, 0, WM_Y), src(RT_INPUT, 0), src(RT_CONST, 1),
        instr(OP_DP4, 3), dstMask(RT_OUTPUT, 0, WM_Z), src(RT_INPUT, 0), src(RT_CONST, 2),
        instr(OP_DP4, 3), dstMask(RT_OUTPUT, 0, WM_W), src(RT_INPUT, 0), src(RT_CONST, 3),
        end(),
    };
    // ps_3_0: mov oC0, c0
    std::vector<DWORD> ps = { psVersion(), instr(OP_MOV, 2), dst(RT_COLOROUT, 0), src(RT_CONST, 0), end() };

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
    if (FAILED(dev->CreateVertexShader(vs.data(), &vsh)) || !vsh ||
        FAILED(dev->CreatePixelShader(ps.data(), &psh))  || !psh) {
        printf("FAIL: shader create\n"); return 1;
    }

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

    // c0..c3 = identity (row-major); ps c0 = green.
    float mtx[16] = { 1,0,0,0,  0,1,0,0,  0,0,1,0,  0,0,0,1 };
    float green[4] = { 0, 1, 0, 1 };

    D3DVIEWPORT9 vp = { 0, 0, 64, 64, 0.0f, 1.0f };
    dev->SetViewport(&vp);
    dev->BeginScene();
    dev->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    dev->SetVertexShader(vsh);
    dev->SetPixelShader(psh);
    dev->SetVertexShaderConstantF(0, mtx, 4);
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
    printf(ok ? "SHADER3D: PASS\n" : "SHADER3D: FAIL\n");
    return ok ? 0 : 1;
}
