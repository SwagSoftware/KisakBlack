// d3dx9.h — portable D3DX9 surface for non-Windows builds.
//
// The renderer uses only a small slice of the D3DX helper library: screenshot
// saving and shader reflection/compilation. These are declared here and backed by
// the GL layer (implemented in src/gfx_gl). Math helpers (D3DXMATRIX/D3DXVECTOR)
// are declared as plain structs; add operations as call sites require them.
#ifndef KISAK_D3DX9_H
#define KISAK_D3DX9_H

#include "d3d9.h"

typedef struct D3DXVECTOR2 { float x, y; }       D3DXVECTOR2;
typedef struct D3DXVECTOR3 { float x, y, z; }    D3DXVECTOR3;
typedef struct D3DXVECTOR4 { float x, y, z, w; } D3DXVECTOR4;

typedef struct D3DXMATRIX {
    union {
        struct { float _11,_12,_13,_14, _21,_22,_23,_24,
                       _31,_32,_33,_34, _41,_42,_43,_44; };
        float m[4][4];
    };
} D3DXMATRIX;

typedef enum _D3DXIMAGE_FILEFORMAT {
    D3DXIFF_BMP = 0, D3DXIFF_JPG = 1, D3DXIFF_TGA = 2, D3DXIFF_PNG = 3, D3DXIFF_DDS = 4,
} D3DXIMAGE_FILEFORMAT;

typedef char *D3DXHANDLE;

// Raw on-disk constant-table (CTAB) layout the renderer walks by hand to map shader
// constants to registers (r_material_load_obj.cpp). Offsets are byte offsets into
// the table blob; sizeof(_D3DXSHADER_CONSTANTINFO) is exactly 20. Matches d3dx9shader.h.
typedef struct _D3DXSHADER_CONSTANTTABLE {
    DWORD Size;
    DWORD Creator;       // offset to creator string
    DWORD Version;
    DWORD Constants;     // number of constants
    DWORD ConstantInfo;  // offset to the _D3DXSHADER_CONSTANTINFO array
    DWORD Flags;
    DWORD Target;        // offset to target string
} D3DXSHADER_CONSTANTTABLE;

typedef struct _D3DXSHADER_CONSTANTINFO {
    DWORD Name;          // offset to constant name
    WORD  RegisterSet;
    WORD  RegisterIndex;
    WORD  RegisterCount;
    WORD  Reserved;
    DWORD TypeInfo;
    DWORD DefaultValue;
} D3DXSHADER_CONSTANTINFO;

// Buffer of compiled-shader / error bytes.
struct ID3DXBuffer : public IUnknown {
    virtual void *WINAPI GetBufferPointer() = 0;
    virtual DWORD WINAPI GetBufferSize() = 0;
};
typedef ID3DXBuffer *LPD3DXBUFFER;

// Vertex-shader input semantic (D3DXSEMANTIC): {Usage, UsageIndex}. The material
// loader reads these to map shader inputs to vertex streams.
typedef struct _D3DXSEMANTIC {
    UINT Usage;
    UINT UsageIndex;
} D3DXSEMANTIC;

// Reflection over a compiled shader's constant table (register assignments). The
// real ID3DXConstantTable also exposes the raw table blob via GetBufferPointer/
// GetBufferSize (the loader walks it by hand — see _D3DXSHADER_CONSTANTTABLE).
struct ID3DXConstantTable : public IUnknown {
    virtual void      *WINAPI GetBufferPointer() = 0;
    virtual DWORD      WINAPI GetBufferSize() = 0;
    virtual D3DXHANDLE WINAPI GetConstantByName(D3DXHANDLE hConstant, const char *pName) = 0;
    virtual UINT       WINAPI GetSamplerIndex(D3DXHANDLE hConstant) = 0;
};
typedef ID3DXConstantTable *LPD3DXCONSTANTTABLE;

extern "C" {

HRESULT WINAPI D3DXCompileShader(const char *pSrcData, UINT srcDataLen,
                                 const void *pDefines, void *pInclude,
                                 const char *pFunctionName, const char *pProfile, DWORD Flags,
                                 LPD3DXBUFFER *ppShader, LPD3DXBUFFER *ppErrorMsgs,
                                 LPD3DXCONSTANTTABLE *ppConstantTable);

HRESULT WINAPI D3DXCreateBuffer(DWORD NumBytes, LPD3DXBUFFER *ppBuffer);

HRESULT WINAPI D3DXSaveSurfaceToFileA(const char *pDestFile, D3DXIMAGE_FILEFORMAT DestFormat,
                                      IDirect3DSurface9 *pSrcSurface, const void *pSrcPalette,
                                      const RECT *pSrcRect);

HRESULT WINAPI D3DXGetShaderConstantTable(const DWORD *pFunction, LPD3DXCONSTANTTABLE *ppConstantTable);
HRESULT WINAPI D3DXGetShaderInputSemantics(const DWORD *pFunction, void *pSemantics, UINT *pCount);
HRESULT WINAPI D3DXGetShaderOutputSemantics(const DWORD *pFunction, void *pSemantics, UINT *pCount);

}  // extern "C"

#endif // KISAK_D3DX9_H
