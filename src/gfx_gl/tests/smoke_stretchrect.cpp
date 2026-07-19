// smoke_stretchrect.cpp — StretchRect blit between render targets (resolve/scale).
//
// Render red into a small RT, StretchRect it (upscaled) into a larger RT, then read
// the larger RT back and confirm the centre is red.
#include <d3d9.h>
#include <GL/glew.h>
#include <cstdio>

static bool near(unsigned char a, int b) { int d = (int)a - b; return d > -4 && d < 4; }

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

    IDirect3DSurface9 *small = nullptr, *big = nullptr, *sys = nullptr;
    dev->CreateRenderTarget(16, 16, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE, &small, nullptr);
    dev->CreateRenderTarget(64, 64, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE, &big, nullptr);
    dev->CreateOffscreenPlainSurface(64, 64, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &sys, nullptr);
    if (!small || !big || !sys) { printf("FAIL: surface create\n"); return 1; }

    dev->SetRenderTarget(0, small);
    D3DVIEWPORT9 vp = { 0, 0, 16, 16, 0.0f, 1.0f };
    dev->SetViewport(&vp);
    dev->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(255, 0, 0), 1.0f, 0);
    dev->SetRenderTarget(0, nullptr);

    if (FAILED(dev->StretchRect(small, nullptr, big, nullptr, D3DTEXF_LINEAR))) {
        printf("FAIL: StretchRect\n"); return 1;
    }

    dev->GetRenderTargetData(big, sys);
    D3DLOCKED_RECT lr;
    sys->LockRect(&lr, nullptr, 0);
    unsigned char *px = (unsigned char *)lr.pBits + 32 * lr.Pitch + 32 * 4;
    unsigned char B = px[0], G = px[1], R = px[2];
    sys->UnlockRect();

    printf("blit BGRA pixel R=%d G=%d B=%d (expect R=255,G=0,B=0)\n", R, G, B);
    bool ok = near(R, 255) && near(G, 0) && near(B, 0);

    small->Release(); big->Release(); sys->Release(); dev->Release(); d3d->Release();
    printf(ok ? "STRETCHRECT: PASS\n" : "STRETCHRECT: FAIL\n");
    return ok ? 0 : 1;
}
