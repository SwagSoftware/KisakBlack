// gl_object.h — COM refcount base shared by every GL-backed D3D9 object.
//
// CRTP-free mixin: provides IUnknown for any D3D9 interface. QueryInterface is
// permissive (returns self) — the COD renderer never queries for a different
// interface, it only AddRef/Releases.
#ifndef KISAK_GL_OBJECT_H
#define KISAK_GL_OBJECT_H

#include <d3d9.h>

template <class IFACE>
class GLObject : public IFACE {
public:
    HRESULT WINAPI QueryInterface(REFIID, void **ppv) override {
        if (ppv) *ppv = this;
        ++ref_;
        return S_OK;
    }
    ULONG WINAPI AddRef() override  { return ++ref_; }
    ULONG WINAPI Release() override { ULONG r = --ref_; if (r == 0) delete this; return r; }
protected:
    virtual ~GLObject() {}
    ULONG ref_ = 1;
};

#endif // KISAK_GL_OBJECT_H
