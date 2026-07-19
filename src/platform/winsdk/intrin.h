// intrin.h — portable shim so `#include <intrin.h>` resolves on non-Windows.
// The MSVC interlocked/barrier intrinsics are mapped to GCC/Clang builtins in the
// compat header (also force-included build-wide, mirroring MSVC's builtins).
#include "../compat/msvc_intrin.h"
