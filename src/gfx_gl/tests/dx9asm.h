// dx9asm.h — tiny DX9 shader-bytecode assembler for the GL-layer tests.
//
// Emits token streams in the documented DX9 encoding (the same encoding the
// translator in gl_shader.cpp parses), so tests can build real vs_3_0/ps_3_0
// shaders without fxc. Test-only helper.
#ifndef KISAK_TEST_DX9ASM_H
#define KISAK_TEST_DX9ASM_H

#include <d3d9.h>

namespace dx9 {

enum {  // register types
    RT_TEMP = 0, RT_INPUT = 1, RT_CONST = 2, RT_TEXTURE = 3,
    RT_OUTPUT = 6, RT_COLOROUT = 8, RT_SAMPLER = 10,
};
enum {  // opcodes
    OP_MOV = 1, OP_ADD = 2, OP_MAD = 4, OP_MUL = 5, OP_DP3 = 8, OP_DP4 = 9,
    OP_TEXLD = 66, OP_DCL = 31, OP_DEF = 81,
};
enum { WM_X = 1, WM_Y = 2, WM_Z = 4, WM_W = 8, WM_ALL = 0xF };
enum { SWZ_XYZW = 0xE4 };  // identity src swizzle

inline DWORD param(int type, int reg, int field16, int mod) {
    DWORD t = 0x80000000u | (DWORD(reg) & 0x7FF);
    t |= (DWORD(type) & 7) << 28;
    t |= ((DWORD(type) >> 3) & 3) << 11;
    t |= (DWORD(field16) & 0xFF) << 16;
    t |= (DWORD(mod) & 0xF) << 24;
    return t;
}
inline DWORD dst(int type, int reg)              { return param(type, reg, WM_ALL, 0); }
inline DWORD dstMask(int type, int reg, int mask){ return param(type, reg, mask, 0); }
inline DWORD src(int type, int reg)              { return param(type, reg, SWZ_XYZW, 0); }
inline DWORD instr(int op, int len)              { return DWORD(op) | (DWORD(len) << 24); }
inline DWORD vsVersion()                         { return 0xFFFE0300u; }
inline DWORD psVersion()                         { return 0xFFFF0300u; }
inline DWORD end()                               { return 0x0000FFFFu; }

}  // namespace dx9

#endif // KISAK_TEST_DX9ASM_H
