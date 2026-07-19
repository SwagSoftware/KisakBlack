// smoke_texture.cpp — textured + alpha-blended quad through the D3D9→GL layer.
//
// Exercises CreateTexture + LockRect/UnlockRect upload, SetTexture/SetSamplerState,
// and real render-state translation (alpha blending). A green texture is drawn
// over a blue clear with 50% alpha blending, so the result should be ~half green,
// half blue.
#include <d3d9.h>
#include <GL/glew.h>
#include <cstdio>
#include <cstring>

struct Vtx { float x, y, z, w; float u, v; };  // POSITIONT + TEXCOORD

static bool near(unsigned char a, int b) { int d = (int)a - b; return d > -16 && d < 16; }

int main() {
    IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
    D3DPRESENT_PARAMETERS pp = {};
    pp.BackBufferWidth = 64; pp.BackBufferHeight = 64;
    pp.BackBufferFormat = D3DFMT_X8R8G8B8; pp.SwapEffect = D3DSWAPEFFECT_DISCARD; pp.Windowed = TRUE;
    IDirect3DDevice9 *dev = nullptr;
    if (FAILED(d3d->CreateDevice(0, D3DDEVTYPE_HAL, nullptr,
                                 D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &dev)) || !dev) {
        printf("FAIL: CreateDevice\n"); return 1;
    }

    // 2x2 texture, every texel opaque green (A8R8G8B8 = BGRA bytes in memory).
    IDirect3DTexture9 *tex = nullptr;
    if (FAILED(dev->CreateTexture(2, 2, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &tex, nullptr)) || !tex) {
        printf("FAIL: CreateTexture\n"); return 1;
    }
    D3DLOCKED_RECT lr;
    tex->LockRect(0, &lr, nullptr, 0);
    for (int y = 0; y < 2; ++y) {
        unsigned char *row = (unsigned char *)lr.pBits + y * lr.Pitch;
        for (int x = 0; x < 2; ++x) {
            row[x*4+0] = 0;    // B
            row[x*4+1] = 255;  // G
            row[x*4+2] = 0;    // R
            row[x*4+3] = 128;  // A (50%)
        }
    }
    tex->UnlockRect(0);

    Vtx quad[6] = {  // two triangles covering the whole 64x64 viewport
        {  0.f,  0.f, 0.5f, 1.f, 0.f, 0.f }, { 64.f,  0.f, 0.5f, 1.f, 1.f, 0.f },
        {  0.f, 64.f, 0.5f, 1.f, 0.f, 1.f }, { 64.f,  0.f, 0.5f, 1.f, 1.f, 0.f },
        { 64.f, 64.f, 0.5f, 1.f, 1.f, 1.f }, {  0.f, 64.f, 0.5f, 1.f, 0.f, 1.f },
    };
    IDirect3DVertexBuffer9 *vb = nullptr;
    dev->CreateVertexBuffer(sizeof(quad), 0, 0, D3DPOOL_DEFAULT, &vb, nullptr);
    void *p = nullptr; vb->Lock(0, 0, &p, 0); memcpy(p, quad, sizeof(quad)); vb->Unlock();

    D3DVERTEXELEMENT9 elems[] = {
        { 0, 0,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 },
        { 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  0 },
        D3DDECL_END()
    };
    IDirect3DVertexDeclaration9 *decl = nullptr;
    dev->CreateVertexDeclaration(elems, &decl);

    D3DVIEWPORT9 vp = { 0, 0, 64, 64, 0.0f, 1.0f };
    dev->SetViewport(&vp);
    dev->BeginScene();
    dev->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);  // blue
    // Alpha blend: src*srcA + dst*(1-srcA)
    dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    dev->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
    dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    dev->SetTexture(0, tex);
    dev->SetVertexDeclaration(decl);
    dev->SetStreamSource(0, vb, 0, sizeof(Vtx));
    dev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
    dev->EndScene();

    glFinish();
    unsigned char c[4] = {0};
    glReadPixels(32, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, c);
    dev->Present(nullptr, nullptr, nullptr, nullptr);

    // Expect green(0,255,0,a=.5) over blue(0,0,255): R~0, G~128, B~128.
    printf("pixel = %d,%d,%d (expect ~0,128,128)\n", c[0], c[1], c[2]);
    bool ok = near(c[0], 0) && near(c[1], 128) && near(c[2], 128);

    tex->Release(); vb->Release(); decl->Release(); dev->Release(); d3d->Release();
    printf(ok ? "TEXTURE: PASS\n" : "TEXTURE: FAIL\n");
    return ok ? 0 : 1;
}
