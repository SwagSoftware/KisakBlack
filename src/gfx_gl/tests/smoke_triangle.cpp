// smoke_triangle.cpp — draws a triangle through the D3D9→GL layer and verifies it.
//
// Exercises the geometry pipeline: CreateVertexBuffer + Lock/Unlock, a vertex
// declaration, SetStreamSource/SetVertexDeclaration, and DrawPrimitive. The
// vertices are pre-transformed (D3DDECLUSAGE_POSITIONT, screen-space), which the
// built-in program handles. A red triangle is drawn over a black clear; the
// centre pixel must come back red.
#include <d3d9.h>
#include <GL/glew.h>
#include <cstdio>

struct Vtx { float x, y, z, w; float r, g, b, a; };  // POSITIONT(float4) + COLOR(float4)

static bool approx(unsigned char a, int b) { int d = (int)a - b; return d > -3 && d < 3; }

int main() {
    IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
    D3DPRESENT_PARAMETERS pp = {};
    pp.BackBufferWidth = 64; pp.BackBufferHeight = 64;
    pp.BackBufferFormat = D3DFMT_X8R8G8B8; pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.Windowed = TRUE; pp.EnableAutoDepthStencil = TRUE; pp.AutoDepthStencilFormat = D3DFMT_D24S8;
    IDirect3DDevice9 *dev = nullptr;
    if (FAILED(d3d->CreateDevice(0, D3DDEVTYPE_HAL, nullptr,
                                 D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &dev)) || !dev) {
        printf("FAIL: CreateDevice\n"); return 1;
    }

    // Red triangle, screen-space coords inside a 64x64 viewport.
    Vtx verts[3] = {
        { 32.f,  8.f, 0.5f, 1.f, 1.f, 0.f, 0.f, 1.f },
        {  8.f, 56.f, 0.5f, 1.f, 1.f, 0.f, 0.f, 1.f },
        { 56.f, 56.f, 0.5f, 1.f, 1.f, 0.f, 0.f, 1.f },
    };
    IDirect3DVertexBuffer9 *vb = nullptr;
    if (FAILED(dev->CreateVertexBuffer(sizeof(verts), 0, 0, D3DPOOL_DEFAULT, &vb, nullptr)) || !vb) {
        printf("FAIL: CreateVertexBuffer\n"); return 1;
    }
    void *p = nullptr;
    vb->Lock(0, 0, &p, 0);
    for (int i = 0; i < 3; ++i) static_cast<Vtx *>(p)[i] = verts[i];
    vb->Unlock();

    D3DVERTEXELEMENT9 elems[] = {
        { 0, 0,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 },
        { 0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,     0 },
        D3DDECL_END()
    };
    IDirect3DVertexDeclaration9 *decl = nullptr;
    dev->CreateVertexDeclaration(elems, &decl);

    D3DVIEWPORT9 vp = { 0, 0, 64, 64, 0.0f, 1.0f };
    dev->SetViewport(&vp);
    dev->BeginScene();
    dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    dev->SetVertexDeclaration(decl);
    dev->SetStreamSource(0, vb, 0, sizeof(Vtx));
    dev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);
    dev->EndScene();

    glFinish();
    unsigned char c[4] = {0}, corner[4] = {0};
    glReadPixels(32, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, c);       // centre: inside triangle
    glReadPixels(2,  2,  1, 1, GL_RGBA, GL_UNSIGNED_BYTE, corner);  // corner: background
    dev->Present(nullptr, nullptr, nullptr, nullptr);

    printf("centre = %d,%d,%d  corner = %d,%d,%d\n", c[0],c[1],c[2], corner[0],corner[1],corner[2]);
    bool ok = approx(c[0], 255) && approx(c[1], 0) && approx(c[2], 0)      // red inside
              && approx(corner[0], 0) && approx(corner[1], 0) && approx(corner[2], 0);  // black outside

    vb->Release(); decl->Release(); dev->Release(); d3d->Release();
    printf(ok ? "TRIANGLE: PASS\n" : "TRIANGLE: FAIL\n");
    return ok ? 0 : 1;
}
