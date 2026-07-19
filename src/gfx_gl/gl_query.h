// gl_query.h — GL-backed IDirect3DQuery9 (occlusion + event queries).
//
// OCCLUSION → a GL occlusion query object (GL_SAMPLES_PASSED); GetData returns the
// sample count. EVENT → a GL fence sync; GetData reports whether the GPU has passed
// the Issue(END) point. Matches D3D's Issue(BEGIN/END) + GetData(FLUSH) protocol.
#ifndef KISAK_GL_QUERY_H
#define KISAK_GL_QUERY_H

#include "gl_object.h"

class GLQuery final : public GLObject<IDirect3DQuery9> {
public:
    GLQuery(IDirect3DDevice9 *device, D3DQUERYTYPE type);
    ~GLQuery() override;

    HRESULT WINAPI GetDevice(IDirect3DDevice9 **ppDevice) override;
    D3DQUERYTYPE WINAPI GetType() override { return type_; }
    DWORD   WINAPI GetDataSize() override { return sizeof(DWORD); }
    HRESULT WINAPI Issue(DWORD dwIssueFlags) override;
    HRESULT WINAPI GetData(void *pData, DWORD dwSize, DWORD dwGetDataFlags) override;

private:
    IDirect3DDevice9 *device_;
    D3DQUERYTYPE      type_;
    unsigned          glQuery_ = 0;        // occlusion query object
    void             *sync_    = nullptr;  // GLsync for event queries
};

#endif // KISAK_GL_QUERY_H
