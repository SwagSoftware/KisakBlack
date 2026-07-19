// corecrt_malloc.h — portable stand-in for MSVC's CRT malloc header.
// Declare the allocators directly (NOT via <cstdlib>) to avoid pulling POSIX
// random(), which collides with the engine's own random().
#ifndef KISAK_CORECRT_MALLOC_H
#define KISAK_CORECRT_MALLOC_H
extern "C" {
    void *malloc(size_t) noexcept;
    void *calloc(size_t, size_t) noexcept;
    void *realloc(void *, size_t) noexcept;
    void  free(void *) noexcept;
    void *_aligned_malloc(size_t, size_t) noexcept;
    void  _aligned_free(void *) noexcept;
}
#endif
