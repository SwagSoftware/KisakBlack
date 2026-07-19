// smoke_savesurface.cpp — D3DXSaveSurfaceToFileA (screenshot path).
//
// Render green into a render-target surface, save it to a TGA via the D3DX shim,
// then read the TGA file back and confirm a pixel is green (TGA stores BGRA).
#include <d3d9.h>
#include <d3dx9.h>
#include <GL/glew.h>
#include <cstdio>

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

    IDirect3DSurface9 *rt = nullptr;
    dev->CreateRenderTarget(8, 8, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE, &rt, nullptr);
    dev->SetRenderTarget(0, rt);
    D3DVIEWPORT9 vp = { 0, 0, 8, 8, 0.0f, 1.0f };
    dev->SetViewport(&vp);
    dev->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 255, 0), 1.0f, 0);  // green
    dev->SetRenderTarget(0, nullptr);

    const char *path = "/tmp/kisak_savesurface.tga";
    if (FAILED(D3DXSaveSurfaceToFileA(path, D3DXIFF_TGA, rt, nullptr, nullptr))) {
        printf("FAIL: D3DXSaveSurfaceToFileA\n"); return 1;
    }

    // Read the TGA back: 18-byte header, then 8x8 BGRA pixels.
    FILE *f = fopen(path, "rb");
    if (!f) { printf("FAIL: reopen tga\n"); return 1; }
    unsigned char hdr[18]; fread(hdr, 1, 18, f);
    int w = hdr[12] | (hdr[13] << 8), h = hdr[14] | (hdr[15] << 8), bpp = hdr[16];
    unsigned char px[4] = {0};
    fseek(f, 18 + (4 * 8 + 4) * 4, SEEK_SET);  // ~centre pixel
    fread(px, 1, 4, f);
    fclose(f);

    printf("tga %dx%d %dbpp, centre BGRA = %d,%d,%d (expect B=0,G=255,R=0)\n",
           w, h, bpp, px[0], px[1], px[2]);
    bool ok = w == 8 && h == 8 && bpp == 32 && px[0] < 8 && px[1] > 247 && px[2] < 8;

    rt->Release(); dev->Release(); d3d->Release();
    printf(ok ? "SAVESURFACE: PASS\n" : "SAVESURFACE: FAIL\n");
    return ok ? 0 : 1;
}
