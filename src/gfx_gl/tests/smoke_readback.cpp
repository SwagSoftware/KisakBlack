// smoke_readback.cpp — standalone render target + GPU→CPU readback.
//
// Creates a standalone render-target surface, renders into it (clear red), then
// GetRenderTargetData copies it into a system-memory surface which is LockRect'd
// and inspected on the CPU — the screenshot path. The locked pixel is D3D
// A8R8G8B8 (BGRA bytes in memory), so red is B=0, G=0, R=255.
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

    IDirect3DSurface9 *rt = nullptr, *sys = nullptr;
    if (FAILED(dev->CreateRenderTarget(32, 32, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE, &rt, nullptr)) || !rt ||
        FAILED(dev->CreateOffscreenPlainSurface(32, 32, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &sys, nullptr)) || !sys) {
        printf("FAIL: surface create\n"); return 1;
    }

    // Render into the standalone RT: clear it red.
    dev->SetRenderTarget(0, rt);
    D3DVIEWPORT9 vp = { 0, 0, 32, 32, 0.0f, 1.0f };
    dev->SetViewport(&vp);
    dev->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(255, 0, 0), 1.0f, 0);
    dev->SetRenderTarget(0, nullptr);

    // Read it back into system memory and inspect on the CPU.
    if (FAILED(dev->GetRenderTargetData(rt, sys))) { printf("FAIL: GetRenderTargetData\n"); return 1; }
    D3DLOCKED_RECT lr;
    if (FAILED(sys->LockRect(&lr, nullptr, 0))) { printf("FAIL: LockRect\n"); return 1; }
    unsigned char *px = (unsigned char *)lr.pBits + 16 * lr.Pitch + 16 * 4;
    unsigned char B = px[0], G = px[1], R = px[2];
    sys->UnlockRect();

    printf("readback BGRA pixel R=%d G=%d B=%d (expect R=255,G=0,B=0)\n", R, G, B);
    bool ok = near(R, 255) && near(G, 0) && near(B, 0);

    rt->Release(); sys->Release(); dev->Release(); d3d->Release();
    printf(ok ? "READBACK: PASS\n" : "READBACK: FAIL\n");
    return ok ? 0 : 1;
}
