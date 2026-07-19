// glcontext_sdl.cpp — SDL2 implementation of GLContext.
//
// SDL2 gives us one window/GL-context/input path across Linux, macOS and
// Windows-GL, so this single file is the entire window-system dependency of the
// GL backend. GLEW is initialised here so the rest of src/gfx_gl can call modern
// GL entry points directly.
#include "glcontext.h"

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <cstdio>

namespace {

class SDLGLContext final : public GLContext {
public:
    bool init(const GLContextDesc &desc) {
        if (SDL_WasInit(SDL_INIT_VIDEO) == 0 && SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
            fprintf(stderr, "[gl] SDL_InitSubSystem(VIDEO): %s\n", SDL_GetError());
            return false;
        }
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,   8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   desc.depthStencil ? 24 : 0);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, desc.depthStencil ? 8  : 0);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, desc.doubleBuffer ? 1  : 0);

        Uint32 flags = SDL_WINDOW_OPENGL | (desc.visible ? 0u : Uint32(SDL_WINDOW_HIDDEN));
        win_ = SDL_CreateWindow("KisakBlack", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                desc.width, desc.height, flags);
        if (!win_) { fprintf(stderr, "[gl] SDL_CreateWindow: %s\n", SDL_GetError()); return false; }
        // Request raise + keyboard focus so menu/game input (which needs X input
        // focus, unlike the polled mouse position) reaches the window.
        if (desc.visible) SDL_RaiseWindow(win_);

        ctx_ = SDL_GL_CreateContext(win_);
        if (!ctx_) { fprintf(stderr, "[gl] SDL_GL_CreateContext: %s\n", SDL_GetError()); return false; }

        glewExperimental = GL_TRUE;
        GLenum ge = glewInit();
        if (ge != GLEW_OK) {
            fprintf(stderr, "[gl] glewInit: %s\n", glewGetErrorString(ge));
            return false;
        }
        glGetError();  // GLEW can leave a benign GL_INVALID_ENUM behind on core profiles.
        return true;
    }

    ~SDLGLContext() override {
        if (ctx_) SDL_GL_DeleteContext(ctx_);
        if (win_) SDL_DestroyWindow(win_);
    }

    void  MakeCurrent() override        { SDL_GL_MakeCurrent(win_, ctx_); }
    void  SwapBuffers() override        { SDL_GL_SwapWindow(win_); }
    void  Resize(int w, int h) override { SDL_SetWindowSize(win_, w, h); }
    void *GetProcAddress(const char *n) override { return SDL_GL_GetProcAddress(n); }

private:
    SDL_Window   *win_ = nullptr;
    SDL_GLContext ctx_ = nullptr;
};

} // namespace

GLContext *GLContext::Create(const GLContextDesc &desc) {
    auto *c = new SDLGLContext();
    if (!c->init(desc)) { delete c; return nullptr; }
    return c;
}
