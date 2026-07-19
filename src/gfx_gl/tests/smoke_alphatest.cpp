// smoke_alphatest.cpp — fixed-function alpha test + the stage-0 color combine.
//
// Core GL removed the fixed-function alpha test; the built-in fragment shader
// emulates it via discard. This test draws a textured quad whose texel alpha is
// below the alpha-test reference, so the alpha test fails for every fragment and
// the blue background shows through (discard). It then lowers the reference so the
// alpha test passes and the textured colour appears. Finally it checks that
// D3DTSS_COLOROP = D3DTOP_MODULATE multiplies the texture by the diffuse colour,
// while D3DTOP_SELECTARG1 selects the texture only.
#include <d3d9.h>
#include <GL/glew.h>
#include <cstdio>
#include <cstring>

struct Vtx { float x, y, z, w; unsigned int color; float u, v; };  // POSITIONT + D3DCOLOR + TEXCOORD

static bool near(unsigned char a, int b) { int d = (int)a - b; return d > -16 && d < 16; }

static IDirect3DDevice9 *g_dev;
static IDirect3DVertexBuffer9 *g_vb;
static IDirect3DVertexDeclaration9 *g_decl;

// Draw the full-screen quad and read back the centre pixel.
static void drawAndRead(unsigned char out[4]) {
    g_dev->SetVertexDeclaration(g_decl);
    g_dev->SetStreamSource(0, g_vb, 0, sizeof(Vtx));
    g_dev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
    glFinish();
    glReadPixels(32, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, out);
}

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
    g_dev = dev;

    // 2x2 texture: opaque-ish red (R=255) but alpha = 64 (below the test ref below).
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
            row[x*4+1] = 0;    // G
            row[x*4+2] = 255;  // R
            row[x*4+3] = 64;   // A (low)
        }
    }
    tex->UnlockRect(0);

    // Full-screen quad; diffuse = half-green (0,128,0) so MODULATE is observable.
    const unsigned int diffuse = D3DCOLOR_ARGB(255, 0, 128, 0);
    Vtx quad[6] = {
        {  0.f,  0.f, 0.5f, 1.f, diffuse, 0.f, 0.f }, { 64.f,  0.f, 0.5f, 1.f, diffuse, 1.f, 0.f },
        {  0.f, 64.f, 0.5f, 1.f, diffuse, 0.f, 1.f }, { 64.f,  0.f, 0.5f, 1.f, diffuse, 1.f, 0.f },
        { 64.f, 64.f, 0.5f, 1.f, diffuse, 1.f, 1.f }, {  0.f, 64.f, 0.5f, 1.f, diffuse, 0.f, 1.f },
    };
    IDirect3DVertexBuffer9 *vb = nullptr;
    dev->CreateVertexBuffer(sizeof(quad), 0, 0, D3DPOOL_DEFAULT, &vb, nullptr);
    void *p = nullptr; vb->Lock(0, 0, &p, 0); memcpy(p, quad, sizeof(quad)); vb->Unlock();
    g_vb = vb;

    D3DVERTEXELEMENT9 elems[] = {
        { 0, 0,  D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 },
        { 0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,     0 },
        { 0, 20, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  0 },
        D3DDECL_END()
    };
    IDirect3DVertexDeclaration9 *decl = nullptr;
    dev->CreateVertexDeclaration(elems, &decl);
    g_decl = decl;

    D3DVIEWPORT9 vp = { 0, 0, 64, 64, 0.0f, 1.0f };
    dev->SetViewport(&vp);
    dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    dev->SetTexture(0, tex);

    bool ok = true;
    unsigned char c[4];

    // --- Pass 1: alpha test rejects the texture (texel A=64 < ref=128) -> blue bg.
    dev->BeginScene();
    dev->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);  // blue
    dev->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);  // texture only
    dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    dev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    dev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
    dev->SetRenderState(D3DRS_ALPHAREF, 128);
    drawAndRead(c);
    dev->EndScene();
    printf("pass1 (alpha-test discard) = %d,%d,%d (expect blue 0,0,255)\n", c[0], c[1], c[2]);
    ok = ok && near(c[0], 0) && near(c[1], 0) && near(c[2], 255);

    // --- Pass 2: lower the ref so the texel passes (A=64 >= ref=32) -> red texture.
    dev->BeginScene();
    dev->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);  // blue
    dev->SetRenderState(D3DRS_ALPHAREF, 32);
    drawAndRead(c);
    dev->EndScene();
    printf("pass2 (alpha-test pass, SELECTARG1) = %d,%d,%d (expect red 255,0,0)\n", c[0], c[1], c[2]);
    ok = ok && near(c[0], 255) && near(c[1], 0) && near(c[2], 0);

    // --- Pass 3: MODULATE multiplies texture(255,0,0) by diffuse(0,128,0) -> ~0.
    // R = 255*0 = 0, G = 0*128 = 0, so the modulated colour is black here. Switch
    // the texture to white to make MODULATE observable: white*diffuse = diffuse.
    tex->LockRect(0, &lr, nullptr, 0);
    for (int y = 0; y < 2; ++y) {
        unsigned char *row = (unsigned char *)lr.pBits + y * lr.Pitch;
        for (int x = 0; x < 2; ++x) { row[x*4+0]=255; row[x*4+1]=255; row[x*4+2]=255; row[x*4+3]=255; }
    }
    tex->UnlockRect(0);
    dev->BeginScene();
    dev->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);
    dev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);  // tex * diffuse
    drawAndRead(c);
    dev->EndScene();
    printf("pass3 (MODULATE white*diffuse) = %d,%d,%d (expect ~0,128,0)\n", c[0], c[1], c[2]);
    ok = ok && near(c[0], 0) && near(c[1], 128) && near(c[2], 0);

    // --- Pass 4: SELECTARG1 ignores diffuse -> white texture shows white.
    dev->BeginScene();
    dev->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);
    dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    drawAndRead(c);
    dev->EndScene();
    printf("pass4 (SELECTARG1 white tex) = %d,%d,%d (expect ~255,255,255)\n", c[0], c[1], c[2]);
    ok = ok && near(c[0], 255) && near(c[1], 255) && near(c[2], 255);

    dev->Present(nullptr, nullptr, nullptr, nullptr);

    tex->Release(); vb->Release(); decl->Release(); dev->Release(); d3d->Release();
    printf(ok ? "ALPHATEST: PASS\n" : "ALPHATEST: FAIL\n");
    return ok ? 0 : 1;
}
