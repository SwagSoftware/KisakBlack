// smoke_query.cpp — occlusion + event queries through the D3D9→GL layer.
//
// Occlusion: wrap a full-screen triangle draw in an occlusion query and confirm a
// non-zero sample count comes back. Event: issue an event query and confirm GetData
// reports completion after a flush.
#include <d3d9.h>
#include <GL/glew.h>
#include <cstdio>
#include <cstring>

struct Vtx { float x, y, z, w; float r, g, b, a; };

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

    Vtx tri[3] = {
        {  2.f,  2.f, 0.5f, 1.f, 1,1,1,1 }, { 62.f,  2.f, 0.5f, 1.f, 1,1,1,1 },
        { 32.f, 62.f, 0.5f, 1.f, 1,1,1,1 },
    };
    IDirect3DVertexBuffer9 *vb = nullptr;
    dev->CreateVertexBuffer(sizeof(tri), 0, 0, D3DPOOL_DEFAULT, &vb, nullptr);
    void *p = nullptr; vb->Lock(0, 0, &p, 0); memcpy(p, tri, sizeof(tri)); vb->Unlock();
    D3DVERTEXELEMENT9 elems[] = {
        { 0, 0,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 },
        { 0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,     0 },
        D3DDECL_END()
    };
    IDirect3DVertexDeclaration9 *decl = nullptr;
    dev->CreateVertexDeclaration(elems, &decl);

    IDirect3DQuery9 *occ = nullptr, *evt = nullptr;
    if (FAILED(dev->CreateQuery(D3DQUERYTYPE_OCCLUSION, &occ)) || !occ ||
        FAILED(dev->CreateQuery(D3DQUERYTYPE_EVENT, &evt)) || !evt) {
        printf("FAIL: CreateQuery\n"); return 1;
    }

    D3DVIEWPORT9 vp = { 0, 0, 64, 64, 0.0f, 1.0f };
    dev->SetViewport(&vp);
    dev->BeginScene();
    dev->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    dev->SetVertexDeclaration(decl);
    dev->SetStreamSource(0, vb, 0, sizeof(Vtx));

    occ->Issue(D3DISSUE_BEGIN);
    dev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);
    occ->Issue(D3DISSUE_END);
    evt->Issue(D3DISSUE_END);
    dev->EndScene();

    DWORD samples = 0;
    while (occ->GetData(&samples, sizeof(samples), D3DGETDATA_FLUSH) == S_FALSE) {}
    DWORD done = 0;
    while (evt->GetData(&done, sizeof(done), D3DGETDATA_FLUSH) == S_FALSE) {}
    dev->Present(nullptr, nullptr, nullptr, nullptr);

    printf("occlusion samples = %u (expect > 0), event done = %u\n", samples, done);
    bool ok = samples > 0 && done == TRUE;

    occ->Release(); evt->Release(); vb->Release(); decl->Release(); dev->Release(); d3d->Release();
    printf(ok ? "QUERY: PASS\n" : "QUERY: FAIL\n");
    return ok ? 0 : 1;
}
