// types.h — fixed-width scalar aliases for the vendored CubeMapGen library.
//
// AMD's CubeMapGen ships its own types.h (not included in this tree). The library
// uses int32/uint32/float32/etc. throughout; reconstruct that small set here. The
// aliases match the engine's own widths, so coexisting typedefs are benign.
#ifndef KISAK_CUBEMAPGEN_TYPES_H
#define KISAK_CUBEMAPGEN_TYPES_H

#include <cstdint>

typedef int8_t   int8;
typedef uint8_t  uint8;
typedef int16_t  int16;
typedef uint16_t uint16;
typedef int32_t  int32;
typedef uint32_t uint32;
typedef int64_t  int64;
typedef uint64_t uint64;
typedef float    float32;
typedef double   float64;

#endif // KISAK_CUBEMAPGEN_TYPES_H
