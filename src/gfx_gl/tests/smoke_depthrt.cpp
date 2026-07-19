// smoke_depthrt.cpp — depth testing while rendering to a texture (FBO depth).
//
// Renders two full-screen triangles into a render-target texture with the depth
// test on: a NEAR green one first, then a FAR red one. If depth testing into the
// FBO works, the far red triangle is rejected and the result stays green. (Without
// depth, the later red draw would overwrite green.) The RT is then sampled to the
// back buffer and the centre pixel must be green.
#include <d3d9.h>
#include <GL/glew.h>
#include <cstdio>
#include <cstring>

struct CVtx { float x, y, z, w; float r, g, b, a; };
struct TVtx { float x, y, z, w; float u, v; };

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
    dev->CreateTexture(64, 64, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &rt, nullptr);
    IDirect3DSurface9 *rtSurf = nullptr;
    rt->GetSurfaceLevel(0, &rtSurf);

    // Screen-space (POSITIONT) coords that over-cover the 64x64 viewport.
    auto fullTri = [](float z, float r, float g, float b, CVtx out[3]) {
        out[0] = { -10, -10, z, 1, r, g, b, 1 }; out[1] = { 200, -10, z, 1, r, g, b, 1 };
        out[2] = { -10, 200, z, 1, r, g, b, 1 };
    };
    CVtx nearGreen[3], farRed[3];
    fullTri(0.2f, 0, 1, 0, nearGreen);
    fullTri(0.8f, 1, 0, 0, farRed);

    IDirect3DVertexBuffer9 *vbN = nullptr, *vbF = nullptr;
    dev->CreateVertexBuffer(sizeof(nearGreen), 0, 0, D3DPOOL_DEFAULT, &vbN, nullptr);
    dev->CreateVertexBuffer(sizeof(farRed), 0, 0, D3DPOOL_DEFAULT, &vbF, nullptr);
    void *p; vbN->Lock(0,0,&p,0); memcpy(p, nearGreen, sizeof(nearGreen)); vbN->Unlock();
    vbF->Lock(0,0,&p,0); memcpy(p, farRed, sizeof(farRed)); vbF->Unlock();

    D3DVERTEXELEMENT9 cElems[] = {
        { 0, 0,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 },
        { 0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,     0 },
        D3DDECL_END()
    };
    IDirect3DVertexDeclaration9 *cDecl = nullptr;
    dev->CreateVertexDeclaration(cElems, &cDecl);

    // Pass 1: depth-tested render into the RT.
    dev->SetRenderTarget(0, rtSurf);
    D3DVIEWPORT9 rvp = { 0, 0, 64, 64, 0.0f, 1.0f };
    dev->SetViewport(&rvp);
    dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,0), 1.0f, 0);
    dev->SetRenderState(D3DRS_ZENABLE, TRUE);
    dev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    dev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    dev->SetVertexDeclaration(cDecl);
    dev->SetStreamSource(0, vbN, 0, sizeof(CVtx));
    dev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);   // near green, z=0.2
    dev->SetStreamSource(0, vbF, 0, sizeof(CVtx));
    dev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);   // far red, z=0.8 -> rejected

    // Pass 2: sample the RT to the back buffer.
    dev->SetRenderTarget(0, nullptr);
    dev->SetRenderState(D3DRS_ZENABLE, FALSE);
    TVtx quad[6] = {
        {  0, 0,0.5f,1, 0,0 }, { 64, 0,0.5f,1, 1,0 }, {  0,64,0.5f,1, 0,1 },
        { 64, 0,0.5f,1, 1,0 }, { 64,64,0.5f,1, 1,1 }, {  0,64,0.5f,1, 0,1 },
    };
    IDirect3DVertexBuffer9 *vbQ = nullptr;
    dev->CreateVertexBuffer(sizeof(quad), 0, 0, D3DPOOL_DEFAULT, &vbQ, nullptr);
    vbQ->Lock(0,0,&p,0); memcpy(p, quad, sizeof(quad)); vbQ->Unlock();
    D3DVERTEXELEMENT9 tElems[] = {
        { 0, 0,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 },
        { 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  0 },
        D3DDECL_END()
    };
    IDirect3DVertexDeclaration9 *tDecl = nullptr;
    dev->CreateVertexDeclaration(tElems, &tDecl);

    D3DVIEWPORT9 vp = { 0, 0, 64, 64, 0.0f, 1.0f };
    dev->SetViewport(&vp);
    dev->BeginScene();
    dev->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0);
    dev->SetTexture(0, rt);
    dev->SetVertexDeclaration(tDecl);
    dev->SetStreamSource(0, vbQ, 0, sizeof(TVtx));
    dev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
    dev->EndScene();

    glFinish();
    unsigned char c[4] = {0};
    glReadPixels(32, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, c);
    dev->Present(nullptr, nullptr, nullptr, nullptr);

    printf("pixel = %d,%d,%d (expect ~0,255,0 — far red depth-rejected)\n", c[0], c[1], c[2]);
    bool ok = near(c[0], 0) && near(c[1], 255) && near(c[2], 0);

    rtSurf->Release(); rt->Release(); vbN->Release(); vbF->Release(); vbQ->Release();
    cDecl->Release(); tDecl->Release(); dev->Release(); d3d->Release();
    printf(ok ? "DEPTHRT: PASS\n" : "DEPTHRT: FAIL\n");
    return ok ? 0 : 1;
}
