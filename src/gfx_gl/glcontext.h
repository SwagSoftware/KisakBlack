// glcontext.h — platform-abstracted OpenGL context/surface.
//
// The D3D9→GL translation layer talks to the GPU through one of these. The
// concrete implementation is chosen per platform (GLX/Xlib on Linux today;
// CGL/NSOpenGL on macOS and WGL on Windows-GL can be added behind the same
// factory). This is the ONLY window-system dependency in the GL backend — keeping
// it isolated here is what lets the rest of src/gfx_gl stay platform-neutral.
#ifndef KISAK_GLCONTEXT_H
#define KISAK_GLCONTEXT_H

struct GLContextDesc {
    int  width        = 640;
    int  height       = 480;
    bool doubleBuffer = true;
    bool depthStencil = true;  // request a 24/8 depth-stencil
    bool visible      = true;  // false → offscreen-style (used by headless tests)
};

class GLContext {
public:
    // Creates and makes-current a context for the running platform, or returns
    // nullptr on failure. Caller owns the result.
    static GLContext *Create(const GLContextDesc &desc);

    virtual ~GLContext() {}
    virtual void  MakeCurrent()                 = 0;
    virtual void  SwapBuffers()                 = 0;
    virtual void  Resize(int width, int height) = 0;
    // Resolve a GL entry point for the loader (modern GL beyond 1.x).
    virtual void *GetProcAddress(const char *name) = 0;
};

#endif // KISAK_GLCONTEXT_H
