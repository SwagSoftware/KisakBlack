// gl_shader.cpp — DX9 bytecode → GLSL 120 translator + GL shader objects.
#include "gl_shader.h"

#include <GL/glew.h>
#include <map>
#include <set>
#include <sstream>
#include <cstdio>
#include <cstring>

namespace {

// --- DX9 bytecode constants (documented format) ---
enum {  // opcodes (token & 0xFFFF)
    OP_MOV = 1, OP_ADD = 2, OP_MAD = 4, OP_MUL = 5, OP_RCP = 6, OP_RSQ = 7,
    OP_DP3 = 8, OP_DP4 = 9, OP_MIN = 10, OP_MAX = 11, OP_FRC = 19,
    OP_TEXLD = 66, OP_DCL = 31, OP_DEF = 81, OP_COMMENT = 0xFFFE, OP_END = 0xFFFF,
};
enum {  // register types
    RT_TEMP = 0, RT_INPUT = 1, RT_CONST = 2, RT_TEXTURE = 3, RT_RASTOUT = 4,
    RT_ATTROUT = 5, RT_OUTPUT = 6, RT_COLOROUT = 8, RT_DEPTHOUT = 9, RT_SAMPLER = 10,
};

struct Operand { int type, reg, swizzle, writemask, mod; };

Operand decodeParam(DWORD t) {
    Operand o;
    o.reg       = (int)(t & 0x7FF);
    o.type      = (int)(((t >> 28) & 0x7) | ((t & 0x1800) >> 8));
    o.writemask = (int)((t >> 16) & 0xF);
    o.swizzle   = (int)((t >> 16) & 0xFF);
    o.mod       = (int)((t >> 24) & 0xF);
    return o;
}

const char *kComp = "xyzw";

std::string maskStr(int m) {  // dest write-mask, e.g. 0b0111 -> ".xyz"
    if (m == 0xF || m == 0) return "";
    std::string s = ".";
    for (int i = 0; i < 4; ++i) if (m & (1 << i)) s += kComp[i];
    return s;
}

std::string swizStr(int sw) {  // src swizzle as a 4-component selector, "" if identity
    if (sw == 0xE4) return "";  // .xyzw
    std::string s = ".";
    for (int i = 0; i < 4; ++i) s += kComp[(sw >> (i * 2)) & 0x3];
    return s;
}

struct Ctx {
    bool isPixel = false;
    const char *cArr() const { return isPixel ? "psc" : "vsc"; }
    std::map<int, std::pair<int,int>> inputs;   // reg -> (usage, usageIndex)
    std::map<int, std::pair<int,int>> outputs;  // reg -> (usage, usageIndex)  (vertex)
    std::set<int> samplers;
    std::map<int, float[4]> defs;
    std::set<int> usedTemps;
    bool usedConst = false;
};

// D3DDECLUSAGE -> a stable varying name shared between the vs and ps stages.
std::string varyingName(int usage, int index) {
    static const char *u[] = {"POSITION","BLENDW","BLENDI","NORMAL","PSIZE","TEXCOORD",
                              "TANGENT","BINORMAL","TESSFACTOR","POSITIONT","COLOR","FOG",
                              "DEPTH","SAMPLE"};
    std::ostringstream s;
    s << "v_" << (usage >= 0 && usage < 14 ? u[usage] : "X") << index;
    return s.str();
}

// Vertex attribute name matching the device's glBindAttribLocation slots.
std::string attribName(int usage) {
    switch (usage) {
        case D3DDECLUSAGE_POSITION: case D3DDECLUSAGE_POSITIONT: return "aPos";
        case D3DDECLUSAGE_COLOR:    return "aColor";
        case D3DDECLUSAGE_TEXCOORD: return "aTexCoord";
        case D3DDECLUSAGE_NORMAL:   return "aNormal";
        default:                    return "aPos";
    }
}

std::string regName(Ctx &c, const Operand &o, bool isDest) {
    switch (o.type) {
        case RT_TEMP:    c.usedTemps.insert(o.reg); { std::ostringstream s; s << "r" << o.reg; return s.str(); }
        case RT_CONST:
            if (c.defs.count(o.reg)) { std::ostringstream s; s << "c" << o.reg << "_def"; return s.str(); }
            c.usedConst = true; { std::ostringstream s; s << c.cArr() << "[" << o.reg << "]"; return s.str(); }
        case RT_INPUT:
            if (c.isPixel) { auto it = c.inputs.find(o.reg);
                             return it != c.inputs.end() ? varyingName(it->second.first, it->second.second) : "vec4(0.0)"; }
            else { auto it = c.inputs.find(o.reg);
                   return it != c.inputs.end() ? attribName(it->second.first) : "aPos"; }
        case RT_OUTPUT: {
            auto it = c.outputs.find(o.reg);
            if (it != c.outputs.end()) {
                if (it->second.first == D3DDECLUSAGE_POSITION || it->second.first == D3DDECLUSAGE_POSITIONT)
                    return "gl_Position";
                return varyingName(it->second.first, it->second.second);
            }
            return o.reg == 0 ? "gl_Position" : "vec4(0.0)";
        }
        case RT_RASTOUT:  return "gl_Position";
        case RT_COLOROUT: return "gl_FragColor";
        case RT_SAMPLER:  { std::ostringstream s; s << "s" << o.reg; return s.str(); }
        default: (void)isDest; return "vec4(0.0)";
    }
}

std::string srcExpr(Ctx &c, const Operand &o) {
    std::string e = regName(c, o, false) + swizStr(o.swizzle);
    if (o.mod == 1) e = "(-" + e + ")";  // D3DSPSM_NEG
    return e;
}

// Emit one instruction as `dst<mask> = (<expr>)<mask>;`.
void emitInstr(Ctx &c, int op, const Operand *src, int nsrc, const Operand &dst,
               std::ostringstream &body) {
    auto s = [&](int i) { return srcExpr(c, src[i]); };
    std::string expr;
    switch (op) {
        case OP_MOV:   expr = s(0); break;
        case OP_ADD:   expr = s(0) + " + " + s(1); break;
        case OP_MUL:   expr = s(0) + " * " + s(1); break;
        case OP_MAD:   expr = s(0) + " * " + s(1) + " + " + s(2); break;
        case OP_MIN:   expr = "min(" + s(0) + ", " + s(1) + ")"; break;
        case OP_MAX:   expr = "max(" + s(0) + ", " + s(1) + ")"; break;
        case OP_FRC:   expr = "fract(" + s(0) + ")"; break;
        case OP_RCP:   expr = "vec4(1.0 / (" + s(0) + ").x)"; break;
        case OP_RSQ:   expr = "vec4(inversesqrt((" + s(0) + ").x))"; break;
        case OP_DP3:   expr = "vec4(dot((" + s(0) + ").xyz, (" + s(1) + ").xyz))"; break;
        case OP_DP4:   expr = "vec4(dot(" + s(0) + ", " + s(1) + "))"; break;
        case OP_TEXLD: expr = "texture2D(" + regName(c, src[1], false) + ", (" + s(0) + ").xy)"; break;
        default: (void)nsrc; return;
    }
    std::string dn = regName(c, dst, true);
    std::string m  = maskStr(dst.writemask);
    body << "  " << dn << m << " = (" << expr << ")" << m << ";\n";
}

} // namespace

bool TranslateD3D9Shader(const DWORD *tok, std::string &out, bool *outIsPixel) {
    Ctx c;
    DWORD ver = *tok++;
    c.isPixel = (ver >> 16) == 0xFFFF;
    if (outIsPixel) *outIsPixel = c.isPixel;

    std::ostringstream body;
    for (;;) {
        DWORD t = *tok++;
        int op = (int)(t & 0xFFFF);
        if (op == OP_END) break;
        if (op == OP_COMMENT) { tok += (t >> 16) & 0x7FFF; continue; }
        int len = (int)((t >> 24) & 0xF);

        if (op == OP_DCL) {
            DWORD usageTok = *tok;
            Operand reg = decodeParam(tok[1]);
            tok += len;
            if (reg.type == RT_SAMPLER) c.samplers.insert(reg.reg);
            else {
                int usage = (int)(usageTok & 0x1F);
                int index = (int)((usageTok >> 16) & 0xF);
                if (reg.type == RT_INPUT)       c.inputs[reg.reg]  = {usage, index};
                else if (reg.type == RT_OUTPUT) c.outputs[reg.reg] = {usage, index};
            }
            continue;
        }
        if (op == OP_DEF) {
            Operand reg = decodeParam(tok[0]);
            float f[4];
            std::memcpy(f, &tok[1], sizeof(f));
            std::memcpy(c.defs[reg.reg], f, sizeof(f));
            tok += len;
            continue;
        }
        // Arithmetic / sample: dst + (len-1) srcs.
        Operand dst = decodeParam(tok[0]);
        Operand src[3];
        int nsrc = len - 1;
        if (nsrc > 3) nsrc = 3;
        for (int i = 0; i < nsrc; ++i) src[i] = decodeParam(tok[1 + i]);
        tok += len;
        emitInstr(c, op, src, nsrc, dst, body);
    }

    // Assemble the GLSL translation unit.
    std::ostringstream o;
    o << "#version 120\n";
    if (c.isPixel) {
        for (auto &in : c.inputs)  o << "varying vec4 " << varyingName(in.second.first, in.second.second) << ";\n";
        for (int s : c.samplers)   o << "uniform sampler2D s" << s << ";\n";
    } else {
        for (auto &in : c.inputs)  o << "attribute vec4 " << attribName(in.second.first) << ";\n";
        for (auto &ou : c.outputs)
            if (ou.second.first != D3DDECLUSAGE_POSITION && ou.second.first != D3DDECLUSAGE_POSITIONT)
                o << "varying vec4 " << varyingName(ou.second.first, ou.second.second) << ";\n";
    }
    if (c.usedConst) o << "uniform vec4 " << c.cArr() << "[256];\n";
    for (auto &d : c.defs)
        o << "const vec4 c" << d.first << "_def = vec4(" << d.second[0] << ", " << d.second[1]
          << ", " << d.second[2] << ", " << d.second[3] << ");\n";
    o << "void main() {\n";
    for (int r : c.usedTemps) o << "  vec4 r" << r << " = vec4(0.0);\n";
    o << body.str();
    o << "}\n";
    out = o.str();
    return true;
}

// ---- GL shader objects ----------------------------------------------------
static unsigned compileGL(GLenum stage, const std::string &src, const char *label) {
    unsigned s = glCreateShader(stage);
    const char *p = src.c_str();
    glShaderSource(s, 1, &p, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        fprintf(stderr, "[gl] %s translate/compile failed:\n%s\nGLSL:\n%s\n", label, log, src.c_str());
        glDeleteShader(s);
        return 0;
    }
    return s;
}

GLVertexShader::GLVertexShader(IDirect3DDevice9 *device, const DWORD *function) : device_(device) {
    bool isPixel = false;
    if (TranslateD3D9Shader(function, glsl_, &isPixel) && !isPixel)
        shader_ = compileGL(GL_VERTEX_SHADER, glsl_, "vertex shader");
}
GLVertexShader::~GLVertexShader() { if (shader_) glDeleteShader(shader_); }
HRESULT WINAPI GLVertexShader::GetDevice(IDirect3DDevice9 **pp) { if (!pp) return E_INVALIDARG; *pp = device_; if (device_) device_->AddRef(); return D3D_OK; }

GLPixelShader::GLPixelShader(IDirect3DDevice9 *device, const DWORD *function) : device_(device) {
    bool isPixel = false;
    if (TranslateD3D9Shader(function, glsl_, &isPixel) && isPixel)
        shader_ = compileGL(GL_FRAGMENT_SHADER, glsl_, "pixel shader");
}
GLPixelShader::~GLPixelShader() { if (shader_) glDeleteShader(shader_); }
HRESULT WINAPI GLPixelShader::GetDevice(IDirect3DDevice9 **pp) { if (!pp) return E_INVALIDARG; *pp = device_; if (device_) device_->AddRef(); return D3D_OK; }
