// smoke_rendertarget.cpp — render-to-texture (FBO) through the D3D9→GL layer.
//
// Creates a render-target texture, binds it via SetRenderTarget and clears it red,
// restores the back buffer, then draws a full-screen quad sampling that texture.
// The centre pixel must read back red — proving CreateTexture(RENDERTARGET) +
// GetSurfaceLevel + SetRenderTarget (FBO) + sample-the-result all work. A solid
// clear is used into the RT so the result is orientation-independent.
#include <d3d9.h>
#include <GL/glew.h>
#include <cstdio>
#include <cstring>

struct Vtx { float x, y, z, w; float u, v; };

static bool near(unsigned char a, int b) { int d = (int)a - b; return d > -8 && d < 8; }

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

    IDirect3DTexture9 *rt = nullptr;
    if (FAILED(dev->CreateTexture(32, 32, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8,
                                  D3DPOOL_DEFAULT, &rt, nullptr)) || !rt) {
        printf("FAIL: CreateTexture(RENDERTARGET)\n"); return 1;
    }
    IDirect3DSurface9 *rtSurf = nullptr;
    rt->GetSurfaceLevel(0, &rtSurf);

    // Pass 1: render into the RT texture — clear it red.
    dev->SetRenderTarget(0, rtSurf);
    D3DVIEWPORT9 rvp = { 0, 0, 32, 32, 0.0f, 1.0f };
    dev->SetViewport(&rvp);
    dev->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(255, 0, 0), 1.0f, 0);

    // Pass 2: back to the back buffer; draw a full-screen quad sampling the RT.
    dev->SetRenderTarget(0, nullptr);
    Vtx quad[6] = {
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
    dev->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    dev->SetTexture(0, rt);
    dev->SetVertexDeclaration(decl);
    dev->SetStreamSource(0, vb, 0, sizeof(Vtx));
    dev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
    dev->EndScene();

    glFinish();
    unsigned char c[4] = {0};
    glReadPixels(32, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, c);
    dev->Present(nullptr, nullptr, nullptr, nullptr);

    printf("pixel = %d,%d,%d (expect 255,0,0)\n", c[0], c[1], c[2]);
    bool ok = near(c[0], 255) && near(c[1], 0) && near(c[2], 0);

    rtSurf->Release(); rt->Release(); vb->Release(); decl->Release(); dev->Release(); d3d->Release();
    printf(ok ? "RENDERTARGET: PASS\n" : "RENDERTARGET: FAIL\n");
    return ok ? 0 : 1;
}
