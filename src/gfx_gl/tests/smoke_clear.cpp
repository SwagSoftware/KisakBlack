// smoke_clear.cpp — end-to-end smoke test of the D3D9→GL translation layer.
//
// Drives the exact boot path the game uses (Direct3DCreate9 -> CreateDevice ->
// Clear -> Present) and verifies the cleared colour landed in the GL back buffer.
// This is the first runnable proof that the layer actually drives OpenGL.
#include <d3d9.h>
#include <GL/glew.h>
#include <cstdio>

static bool approx(unsigned char a, int b) { int d = (int)a - b; return d > -2 && d < 2; }

int main() {
    IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d) { printf("FAIL: Direct3DCreate9 returned null\n"); return 1; }

    D3DPRESENT_PARAMETERS pp = {};
    pp.BackBufferWidth        = 64;
    pp.BackBufferHeight       = 64;
    pp.BackBufferFormat       = D3DFMT_X8R8G8B8;
    pp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    pp.Windowed               = TRUE;
    pp.EnableAutoDepthStencil = TRUE;
    pp.AutoDepthStencilFormat = D3DFMT_D24S8;

    IDirect3DDevice9 *dev = nullptr;
    HRESULT hr = d3d->CreateDevice(0, D3DDEVTYPE_HAL, nullptr,
                                   D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &dev);
    if (FAILED(hr) || !dev) { printf("FAIL: CreateDevice hr=0x%08x\n", (unsigned)hr); return 1; }

    D3DCAPS9 caps;
    dev->GetDeviceCaps(&caps);
    printf("caps: VS=0x%08x PS=0x%08x RTs=%u maxTex=%u\n",
           caps.VertexShaderVersion, caps.PixelShaderVersion,
           caps.NumSimultaneousRTs, caps.MaxTextureWidth);

    D3DVIEWPORT9 vp = { 0, 0, 64, 64, 0.0f, 1.0f };
    dev->SetViewport(&vp);
    dev->BeginScene();
    dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
               D3DCOLOR_XRGB(64, 128, 191), 1.0f, 0);
    dev->EndScene();

    glFinish();
    unsigned char px[4] = { 0, 0, 0, 0 };
    glReadPixels(32, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, px);
    dev->Present(nullptr, nullptr, nullptr, nullptr);

    printf("pixel = %d,%d,%d,%d (expect 64,128,191,255)\n", px[0], px[1], px[2], px[3]);
    bool ok = approx(px[0], 64) && approx(px[1], 128) && approx(px[2], 191);

    dev->Release();
    d3d->Release();

    printf(ok ? "SMOKE: PASS\n" : "SMOKE: FAIL\n");
    return ok ? 0 : 1;
}
