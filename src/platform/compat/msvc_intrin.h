// msvc_intrin.h — MSVC interlocked/barrier intrinsics on GCC/Clang.
//
// The threading/queue code (tl/, qcommon, gfx) uses MSVC's _Interlocked* and
// barrier intrinsics, which MSVC provides as builtins (or via <intrin.h>). Map
// them to GCC/Clang's type-generic __sync/__atomic builtins, preserving MSVC's
// return-value semantics:
//   _InterlockedExchangeAdd  -> fetch_and_add   (returns OLD value)
//   _InterlockedCompareExchange[64] -> val_compare_and_swap (returns OLD value;
//      note the arg reorder: MSVC(dest,exch,comp) vs __sync(ptr,comp,exch))
//   _InterlockedIncrement/Decrement -> add/sub_and_fetch (returns NEW value)
// Macros (not functions) so they stay type-generic across the various pointer
// types the decompiled code passes.
#ifndef KISAK_MSVC_INTRIN_H
#define KISAK_MSVC_INTRIN_H

#if !defined(_MSC_VER)

#define _InterlockedExchangeAdd(p, v)          __sync_fetch_and_add((p), (v))
#define _InterlockedCompareExchange(p, e, c)   __sync_val_compare_and_swap((p), (c), (e))
#define _InterlockedCompareExchange64(p, e, c) __sync_val_compare_and_swap((p), (c), (e))
#define _InterlockedExchange(p, v)             __sync_lock_test_and_set((p), (v))
#define _InterlockedIncrement(p)               __sync_add_and_fetch((p), 1)
#define _InterlockedDecrement(p)               __sync_sub_and_fetch((p), 1)
#define _ReadWriteBarrier()                    __atomic_signal_fence(__ATOMIC_SEQ_CST)
#define _ReadBarrier()                         __atomic_signal_fence(__ATOMIC_SEQ_CST)
#define _WriteBarrier()                        __atomic_signal_fence(__ATOMIC_SEQ_CST)
#define MemoryBarrier()                        __sync_synchronize()

// _BitScanReverse(&index, mask): index <- position of the highest set bit;
// returns 0 if mask == 0. (MSVC bit intrinsic.) Templated on the index type so it
// accepts whatever 32-bit integer pointer the decompiled callers pass — MSVC's
// prototype is `unsigned long*`, but the engine passes DWORD* (unsigned int*), and
// on i386 GCC those are distinct (non-convertible) pointer types.
template <class T>
static inline unsigned char _BitScanReverse(T *Index, unsigned long Mask) {
    if (!Mask) return 0;
    *Index = (T)(31u - (unsigned)__builtin_clz((unsigned int)Mask));
    return 1;
}

// _mm_prefetch(addr, hint): SSE prefetch. GCC's <xmmintrin.h> version takes an
// enum _mm_hint (no implicit int->enum in C++), but the decompiled code passes a
// bare int. Map straight to __builtin_prefetch (the locality arg is advisory).
#ifndef _mm_prefetch
#define _mm_prefetch(addr, hint) __builtin_prefetch((const void *)(addr))
#endif

#endif // !_MSC_VER
#endif // KISAK_MSVC_INTRIN_H
