// msvc_rand.cpp — MSVC-compatible rand()/srand() for the decompiled engine.
//
// The engine was built against the Microsoft CRT, whose rand() returns a value
// in [0, RAND_MAX] with RAND_MAX == 0x7FFF (32767). The code relies on that
// range pervasively: `rand() / 32767.0` to map into [0,1], Com_Random() /
// Com_CRandom(), sound pitch/volume jitter, debris directions, and so on.
//
// glibc's rand() instead returns [0, 2^31-1], so every one of those
// normalizations is ~65536x too large. The most visible symptom is a hang:
// GaussianRandom() samples a point in the unit disc with the rejection loop
//     do { x = Com_CRandom(); y = Com_CRandom(); w = x*x + y*y; } while (w > 1);
// and with glibc's range w is never <= 1, so R_CreateParticleCloudBuffer() (and
// any other Gaussian sampling) spins forever.
//
// Override rand()/srand() with the exact MSVC linear congruential generator so
// the engine sees the range — and sequence — it was written against. These are
// strong symbols in the main executable, which the dynamic loader resolves ahead
// of libc's for all engine calls. The seed is thread-local, matching the MSVC
// CRT where each thread keeps an independent sequence (srand() on one thread does
// not perturb another's).

namespace {
    __thread unsigned long g_holdrand = 1;  // MSVC's initial _Holdrand
}

extern "C" int rand() noexcept {
    g_holdrand = g_holdrand * 214013ul + 2531011ul;
    return (int)((g_holdrand >> 16) & 0x7FFF);
}

extern "C" void srand(unsigned int seed) noexcept {
    g_holdrand = seed;
}
