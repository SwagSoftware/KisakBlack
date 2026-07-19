// smoke_voltexture.cpp — volume (3D) texture creation + LockBox/UnlockBox upload.
//
// Exercises CreateVolumeTexture (GL_TEXTURE_3D), GetLevelDesc, and uploading a
// known colour to a level via LockBox/UnlockBox (glTexImage3D). It is enough to
// verify creation + upload succeed without any GL error.
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

    const UINT W = 4, H = 4, D = 4;
    IDirect3DVolumeTexture9 *vol = nullptr;
    if (FAILED(dev->CreateVolumeTexture(W, H, D, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &vol, nullptr))
        || !vol) {
        printf("FAIL: CreateVolumeTexture\n"); return 1;
    }

    if (vol->GetLevelCount() != 1) { printf("FAIL: GetLevelCount\n"); return 1; }

    D3DVOLUME_DESC desc;
    if (FAILED(vol->GetLevelDesc(0, &desc)) || desc.Width != W || desc.Height != H ||
        desc.Depth != D || desc.Format != D3DFMT_A8R8G8B8) {
        printf("FAIL: GetLevelDesc (%ux%ux%u fmt=%d)\n", desc.Width, desc.Height, desc.Depth, desc.Format);
        return 1;
    }

    D3DLOCKED_BOX lb;
    if (FAILED(vol->LockBox(0, &lb, nullptr, 0)) || !lb.pBits) {
        printf("FAIL: LockBox\n"); return 1;
    }
    if (lb.RowPitch != (int)(W * 4) || lb.SlicePitch != (int)(W * H * 4)) {
        printf("FAIL: LockBox pitches (row=%d slice=%d)\n", lb.RowPitch, lb.SlicePitch); return 1;
    }
    // Fill every voxel with a gradient by slice (A8R8G8B8 = BGRA bytes).
    for (UINT z = 0; z < D; ++z) {
        unsigned char *slice = (unsigned char *)lb.pBits + z * lb.SlicePitch;
        for (UINT y = 0; y < H; ++y) {
            unsigned char *row = slice + y * lb.RowPitch;
            for (UINT x = 0; x < W; ++x) {
                row[x*4+0] = (unsigned char)(z * 60);  // B
                row[x*4+1] = 200;                       // G
                row[x*4+2] = (unsigned char)(x * 50);  // R
                row[x*4+3] = 255;                       // A
            }
        }
    }
    if (FAILED(vol->UnlockBox(0))) { printf("FAIL: UnlockBox\n"); return 1; }

    glFinish();
    bool ok = checkGL("volume upload");

    vol->Release(); dev->Release(); d3d->Release();
    printf(ok ? "VOLTEXTURE: PASS\n" : "VOLTEXTURE: FAIL\n");
    return ok ? 0 : 1;
}
