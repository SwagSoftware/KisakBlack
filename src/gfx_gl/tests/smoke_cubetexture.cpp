// smoke_cubetexture.cpp — cube texture creation + per-face LockRect/UnlockRect.
//
// Exercises CreateCubeTexture (GL_TEXTURE_CUBE_MAP), GetCubeMapSurface, and
// uploading a known colour to each of the six faces via LockRect/UnlockRect. It
// is enough to verify creation + upload succeed without any GL error; sampling a
// cube in a draw is not required here.
#include <d3d9.h>
#include <GL/glew.h>
#include <cstdio>

static bool checkGL(const char *where) {
    GLenum e = glGetError();
    if (e != GL_NO_ERROR) { printf("FAIL: GL error 0x%04x at %s\n", e, where); return false; }
    return true;
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

    // 4x4x4-cube of A8R8G8B8.
    const UINT edge = 4;
    IDirect3DCubeTexture9 *cube = nullptr;
    if (FAILED(dev->CreateCubeTexture(edge, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &cube, nullptr))
        || !cube) {
        printf("FAIL: CreateCubeTexture\n"); return 1;
    }

    if (cube->GetLevelCount() != 1) { printf("FAIL: GetLevelCount\n"); return 1; }

    D3DSURFACE_DESC desc;
    if (FAILED(cube->GetLevelDesc(0, &desc)) || desc.Width != edge || desc.Height != edge ||
        desc.Format != D3DFMT_A8R8G8B8) {
        printf("FAIL: GetLevelDesc (%ux%u fmt=%d)\n", desc.Width, desc.Height, desc.Format);
        return 1;
    }

    // Distinct colour per face; verify the upload path on every face target.
    bool ok = true;
    for (int f = 0; f < 6; ++f) {
        D3DLOCKED_RECT lr;
        if (FAILED(cube->LockRect((D3DCUBEMAP_FACES)f, 0, &lr, nullptr, 0)) || !lr.pBits) {
            printf("FAIL: LockRect face %d\n", f); ok = false; break;
        }
        for (UINT y = 0; y < edge; ++y) {
            unsigned char *row = (unsigned char *)lr.pBits + y * lr.Pitch;
            for (UINT x = 0; x < edge; ++x) {
                row[x*4+0] = (unsigned char)(f * 40);   // B
                row[x*4+1] = (unsigned char)(255 - f);  // G
                row[x*4+2] = (unsigned char)(f * 30);   // R
                row[x*4+3] = 255;                        // A
            }
        }
        if (FAILED(cube->UnlockRect((D3DCUBEMAP_FACES)f, 0))) {
            printf("FAIL: UnlockRect face %d\n", f); ok = false; break;
        }
    }
    glFinish();
    ok = ok && checkGL("cube upload");

    // GetCubeMapSurface returns a usable surface view that locks the same storage.
    IDirect3DSurface9 *surf = nullptr;
    if (FAILED(cube->GetCubeMapSurface(D3DCUBEMAP_FACE_POSITIVE_X, 0, &surf)) || !surf) {
        printf("FAIL: GetCubeMapSurface\n"); ok = false;
    } else {
        D3DLOCKED_RECT lr;
        if (FAILED(surf->LockRect(&lr, nullptr, 0)) || !lr.pBits) {
            printf("FAIL: surface LockRect\n"); ok = false;
        } else {
            surf->UnlockRect();
        }
        surf->Release();
    }
    glFinish();
    ok = ok && checkGL("cube surface");

    cube->Release(); dev->Release(); d3d->Release();
    printf(ok ? "CUBETEXTURE: PASS\n" : "CUBETEXTURE: FAIL\n");
    return ok ? 0 : 1;
}
