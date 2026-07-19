// gl_query.cpp — GL occlusion / event queries + GLDevice::CreateQuery.
#include "gl_query.h"
#include "gl_d3d9.h"

#include <GL/glew.h>

GLQuery::GLQuery(IDirect3DDevice9 *device, D3DQUERYTYPE type) : device_(device), type_(type) {
    if (type_ == D3DQUERYTYPE_OCCLUSION) glGenQueries(1, &glQuery_);
}

GLQuery::~GLQuery() {
    if (glQuery_) glDeleteQueries(1, &glQuery_);
    if (sync_)    glDeleteSync(static_cast<GLsync>(sync_));
}

HRESULT WINAPI GLQuery::GetDevice(IDirect3DDevice9 **ppDevice) {
    if (!ppDevice) return E_INVALIDARG;
    *ppDevice = device_;
    if (device_) device_->AddRef();
    return D3D_OK;
}

HRESULT WINAPI GLQuery::Issue(DWORD dwIssueFlags) {
    if (type_ == D3DQUERYTYPE_OCCLUSION) {
        if (dwIssueFlags & D3DISSUE_BEGIN) glBeginQuery(GL_SAMPLES_PASSED, glQuery_);
        if (dwIssueFlags & D3DISSUE_END)   glEndQuery(GL_SAMPLES_PASSED);
    } else if (type_ == D3DQUERYTYPE_EVENT) {
        if (dwIssueFlags & D3DISSUE_END) {
            if (sync_) glDeleteSync(static_cast<GLsync>(sync_));
            sync_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        }
    }
    return D3D_OK;
}

HRESULT WINAPI GLQuery::GetData(void *pData, DWORD /*dwSize*/, DWORD dwGetDataFlags) {
    bool flush = (dwGetDataFlags & D3DGETDATA_FLUSH) != 0;

    if (type_ == D3DQUERYTYPE_OCCLUSION) {
        GLint available = 0;
        glGetQueryObjectiv(glQuery_, GL_QUERY_RESULT_AVAILABLE, &available);
        if (!available && !flush) return S_FALSE;       // not ready yet
        GLuint samples = 0;
        glGetQueryObjectuiv(glQuery_, GL_QUERY_RESULT, &samples);  // blocks if flush
        if (pData) *static_cast<DWORD *>(pData) = samples;
        return S_OK;
    }

    if (type_ == D3DQUERYTYPE_EVENT) {
        // No outstanding fence (never Issue'd, or already consumed): real D3D9
        // reports such an event query as signaled. Returning S_FALSE here makes
        // R_FinishGpuFence's `while (GetData == S_FALSE)` spin forever, since the
        // engine waits on dx.flushGpuQuery without ever issuing it.
        if (!sync_) { if (pData) *static_cast<DWORD *>(pData) = TRUE; return S_OK; }
        GLenum r = glClientWaitSync(static_cast<GLsync>(sync_),
                                    flush ? GL_SYNC_FLUSH_COMMANDS_BIT : 0, 0);
        bool done = (r == GL_ALREADY_SIGNALED || r == GL_CONDITION_SATISFIED);
        if (!done) return S_FALSE;
        if (pData) *static_cast<DWORD *>(pData) = TRUE;
        return S_OK;
    }
    return S_OK;
}

HRESULT WINAPI GLDevice::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9 **ppQuery) {
    if (!ppQuery) return E_INVALIDARG;
    *ppQuery = new GLQuery(this, Type);
    return D3D_OK;
}
