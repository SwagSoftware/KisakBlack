// d3d9types.h — portable Direct3D 9 enums / flags / structs for non-Windows builds.
//
// Values match the real Microsoft d3d9types.h so the renderer behaves identically and
// MSDN remains an accurate reference. Only the subset the KisakBlack renderer actually
// uses is declared (surface extracted from `git grep` over src/, excluding vendored libs).
#ifndef KISAK_D3D9TYPES_H
#define KISAK_D3D9TYPES_H

#include "_kisak_wintypes.h"

// ---- Colors ---------------------------------------------------------------
typedef DWORD D3DCOLOR;
#define D3DCOLOR_ARGB(a, r, g, b) \
    ((D3DCOLOR)((((a) & 0xff) << 24) | (((r) & 0xff) << 16) | \
                (((g) & 0xff) << 8)  |  ((b) & 0xff)))
#define D3DCOLOR_RGBA(r, g, b, a) D3DCOLOR_ARGB(a, r, g, b)
#define D3DCOLOR_XRGB(r, g, b)    D3DCOLOR_ARGB(0xff, r, g, b)

typedef struct _D3DCOLORVALUE { float r, g, b, a; } D3DCOLORVALUE;
typedef struct _D3DVECTOR     { float x, y, z; }     D3DVECTOR;

// ---- D3DFORMAT ------------------------------------------------------------
typedef enum _D3DFORMAT {
    D3DFMT_UNKNOWN        = 0,
    D3DFMT_R8G8B8         = 20,
    D3DFMT_A8R8G8B8       = 21,
    D3DFMT_X8R8G8B8       = 22,
    D3DFMT_R5G6B5         = 23,
    D3DFMT_X1R5G5B5       = 24,
    D3DFMT_A1R5G5B5       = 25,
    D3DFMT_A4R4G4B4       = 26,
    D3DFMT_A8             = 28,
    D3DFMT_A8B8G8R8       = 32,
    D3DFMT_G16R16         = 34,
    D3DFMT_A16B16G16R16   = 36,
    D3DFMT_L8             = 50,
    D3DFMT_A8L8           = 51,
    D3DFMT_D16_LOCKABLE   = 70,
    D3DFMT_D15S1          = 73,
    D3DFMT_D24S8          = 75,
    D3DFMT_D24X8          = 77,
    D3DFMT_D16            = 80,
    D3DFMT_D24FS8         = 83,
    D3DFMT_INDEX16        = 101,
    D3DFMT_G16R16F        = 112,
    D3DFMT_A16B16G16R16F  = 113,
    D3DFMT_R32F           = 114,
    D3DFMT_DXT1           = MAKEFOURCC('D', 'X', 'T', '1'),
    D3DFMT_DXT3           = MAKEFOURCC('D', 'X', 'T', '3'),
    D3DFMT_DXT5           = MAKEFOURCC('D', 'X', 'T', '5'),
    D3DFMT_FORCE_DWORD    = 0x7fffffff
} D3DFORMAT;

// ---- D3DRENDERSTATETYPE ---------------------------------------------------
typedef enum _D3DRENDERSTATETYPE {
    D3DRS_ZENABLE                   = 7,
    D3DRS_FILLMODE                  = 8,
    D3DRS_ZWRITEENABLE              = 14,
    D3DRS_ALPHATESTENABLE           = 15,
    D3DRS_SRCBLEND                  = 19,
    D3DRS_DESTBLEND                 = 20,
    D3DRS_CULLMODE                  = 22,
    D3DRS_ZFUNC                     = 23,
    D3DRS_ALPHAREF                  = 24,
    D3DRS_ALPHAFUNC                 = 25,
    D3DRS_ALPHABLENDENABLE          = 27,
    D3DRS_STENCILENABLE             = 52,
    D3DRS_STENCILFAIL               = 53,
    D3DRS_STENCILZFAIL              = 54,
    D3DRS_STENCILPASS               = 55,
    D3DRS_STENCILFUNC               = 56,
    D3DRS_STENCILREF                = 57,
    D3DRS_STENCILMASK               = 58,
    D3DRS_STENCILWRITEMASK          = 59,
    D3DRS_WRAP0                     = 128,
    D3DRS_POINTSIZE                 = 154,
    D3DRS_COLORWRITEENABLE          = 168,
    D3DRS_BLENDOP                   = 171,
    D3DRS_SCISSORTESTENABLE         = 174,
    D3DRS_SLOPESCALEDEPTHBIAS       = 175,
    D3DRS_ADAPTIVETESS_Y            = 181,
    D3DRS_TWOSIDEDSTENCILMODE       = 185,
    D3DRS_CCW_STENCILFAIL           = 186,
    D3DRS_CCW_STENCILZFAIL          = 187,
    D3DRS_CCW_STENCILPASS           = 188,
    D3DRS_CCW_STENCILFUNC           = 189,
    D3DRS_COLORWRITEENABLE1         = 190,
    D3DRS_COLORWRITEENABLE2         = 191,
    D3DRS_COLORWRITEENABLE3         = 192,
    D3DRS_DEPTHBIAS                 = 195,
    D3DRS_SEPARATEALPHABLENDENABLE  = 206,
    D3DRS_SRCBLENDALPHA             = 207,
    D3DRS_DESTBLENDALPHA            = 208,
    D3DRS_BLENDOPALPHA              = 209,
    D3DRS_FORCE_DWORD               = 0x7fffffff
} D3DRENDERSTATETYPE;

// ---- D3DSAMPLERSTATETYPE --------------------------------------------------
typedef enum _D3DSAMPLERSTATETYPE {
    D3DSAMP_ADDRESSU      = 1,
    D3DSAMP_ADDRESSV      = 2,
    D3DSAMP_ADDRESSW      = 3,
    D3DSAMP_MAGFILTER     = 5,
    D3DSAMP_MINFILTER     = 6,
    D3DSAMP_MIPFILTER     = 7,
    D3DSAMP_MAXANISOTROPY = 10,
    D3DSAMP_FORCE_DWORD   = 0x7fffffff
} D3DSAMPLERSTATETYPE;

// ---- D3DTEXTURESTAGESTATETYPE ---------------------------------------------
typedef enum _D3DTEXTURESTAGESTATETYPE {
    D3DTSS_COLOROP        = 1,
    D3DTSS_COLORARG1      = 2,
    D3DTSS_COLORARG2      = 3,
    D3DTSS_ALPHAOP        = 4,
    D3DTSS_ALPHAARG1      = 5,
    D3DTSS_ALPHAARG2      = 6,
    D3DTSS_FORCE_DWORD    = 0x7fffffff
} D3DTEXTURESTAGESTATETYPE;

// ---- Small fixed enums ----------------------------------------------------
typedef enum _D3DTEXTUREOP {
    D3DTOP_SELECTARG1 = 2,
    D3DTOP_MODULATE   = 4
} D3DTEXTUREOP;

typedef enum _D3DTEXTUREADDRESS {
    D3DTADDRESS_WRAP  = 1,
    D3DTADDRESS_CLAMP = 3
} D3DTEXTUREADDRESS;

typedef enum _D3DTEXTUREFILTERTYPE {
    D3DTEXF_NONE   = 0,
    D3DTEXF_POINT  = 1,
    D3DTEXF_LINEAR = 2
} D3DTEXTUREFILTERTYPE;

typedef enum _D3DBLEND {
    D3DBLEND_ZERO        = 1,
    D3DBLEND_ONE         = 2,
    D3DBLEND_SRCALPHA    = 5,
    D3DBLEND_INVSRCALPHA = 6
} D3DBLEND;

typedef enum _D3DBLENDOP {
    D3DBLENDOP_ADD         = 1,
    D3DBLENDOP_SUBTRACT    = 2,
    D3DBLENDOP_REVSUBTRACT = 3,
    D3DBLENDOP_MIN         = 4,
    D3DBLENDOP_MAX         = 5
} D3DBLENDOP;

typedef enum _D3DCMPFUNC {
    D3DCMP_NEVER        = 1,
    D3DCMP_LESS         = 2,
    D3DCMP_EQUAL        = 3,
    D3DCMP_LESSEQUAL    = 4,
    D3DCMP_GREATER      = 5,
    D3DCMP_NOTEQUAL     = 6,
    D3DCMP_GREATEREQUAL = 7,
    D3DCMP_ALWAYS       = 8
} D3DCMPFUNC;

typedef enum _D3DCULL {
    D3DCULL_NONE = 1,
    D3DCULL_CW   = 2,
    D3DCULL_CCW  = 3
} D3DCULL;

typedef enum _D3DFILLMODE {
    D3DFILL_POINT     = 1,
    D3DFILL_WIREFRAME = 2,
    D3DFILL_SOLID     = 3
} D3DFILLMODE;

typedef enum _D3DSTENCILOP {
    D3DSTENCILOP_KEEP    = 1,
    D3DSTENCILOP_ZERO    = 2,
    D3DSTENCILOP_REPLACE = 3,
    D3DSTENCILOP_INCRSAT = 4,
    D3DSTENCILOP_DECRSAT = 5,
    D3DSTENCILOP_INVERT  = 6,
    D3DSTENCILOP_INCR    = 7,
    D3DSTENCILOP_DECR    = 8
} D3DSTENCILOP;

typedef enum _D3DZBUFFERTYPE {
    D3DZB_FALSE = 0,
    D3DZB_TRUE  = 1,
    D3DZB_USEW  = 2
} D3DZBUFFERTYPE;

typedef enum _D3DPRIMITIVETYPE {
    D3DPT_POINTLIST     = 1,
    D3DPT_LINELIST      = 2,
    D3DPT_LINESTRIP     = 3,
    D3DPT_TRIANGLELIST  = 4,
    D3DPT_TRIANGLESTRIP = 5,
    D3DPT_TRIANGLEFAN   = 6
} D3DPRIMITIVETYPE;

typedef enum _D3DPOOL {
    D3DPOOL_DEFAULT   = 0,
    D3DPOOL_MANAGED   = 1,
    D3DPOOL_SYSTEMMEM = 2,
    D3DPOOL_SCRATCH   = 3
} D3DPOOL;

typedef enum _D3DMULTISAMPLE_TYPE {
    D3DMULTISAMPLE_NONE        = 0,
    D3DMULTISAMPLE_NONMASKABLE = 1
} D3DMULTISAMPLE_TYPE;

typedef enum _D3DSWAPEFFECT {
    D3DSWAPEFFECT_DISCARD = 1,
    D3DSWAPEFFECT_FLIP    = 2,
    D3DSWAPEFFECT_COPY    = 3
} D3DSWAPEFFECT;

typedef enum _D3DDEVTYPE {
    D3DDEVTYPE_HAL = 1,
    D3DDEVTYPE_REF = 2,
    D3DDEVTYPE_SW  = 3
} D3DDEVTYPE;

typedef enum _D3DQUERYTYPE {
    D3DQUERYTYPE_EVENT     = 8,
    D3DQUERYTYPE_OCCLUSION = 9
} D3DQUERYTYPE;

typedef enum _D3DRESOURCETYPE {
    D3DRTYPE_SURFACE       = 1,
    D3DRTYPE_VOLUME        = 2,
    D3DRTYPE_TEXTURE       = 3,
    D3DRTYPE_VOLUMETEXTURE = 4,
    D3DRTYPE_CUBETEXTURE   = 5,
    D3DRTYPE_VERTEXBUFFER  = 6,
    D3DRTYPE_INDEXBUFFER   = 7
} D3DRESOURCETYPE;

typedef enum _D3DDECLTYPE {
    D3DDECLTYPE_FLOAT1  = 0,
    D3DDECLTYPE_FLOAT2  = 1,
    D3DDECLTYPE_FLOAT3  = 2,
    D3DDECLTYPE_FLOAT4  = 3,
    D3DDECLTYPE_D3DCOLOR= 4,
    D3DDECLTYPE_UBYTE4  = 5,
    D3DDECLTYPE_UNUSED  = 17
} D3DDECLTYPE;

typedef enum _D3DDECLMETHOD { D3DDECLMETHOD_DEFAULT = 0 } D3DDECLMETHOD;

typedef enum _D3DDECLUSAGE {
    D3DDECLUSAGE_POSITION  = 0,
    D3DDECLUSAGE_BLENDWEIGHT = 1,
    D3DDECLUSAGE_BLENDINDICES = 2,
    D3DDECLUSAGE_NORMAL    = 3,
    D3DDECLUSAGE_TEXCOORD  = 5,
    D3DDECLUSAGE_TANGENT   = 6,
    D3DDECLUSAGE_COLOR     = 10,
    D3DDECLUSAGE_POSITIONT = 9
} D3DDECLUSAGE;

typedef enum _D3DCUBEMAP_FACES {
    D3DCUBEMAP_FACE_POSITIVE_X = 0,
    D3DCUBEMAP_FACE_NEGATIVE_X = 1,
    D3DCUBEMAP_FACE_POSITIVE_Y = 2,
    D3DCUBEMAP_FACE_NEGATIVE_Y = 3,
    D3DCUBEMAP_FACE_POSITIVE_Z = 4,
    D3DCUBEMAP_FACE_NEGATIVE_Z = 5
} D3DCUBEMAP_FACES;

// ---- Flag bitmasks (not enums) --------------------------------------------
#define D3DUSAGE_RENDERTARGET   0x00000001L
#define D3DUSAGE_DEPTHSTENCIL   0x00000002L
#define D3DUSAGE_WRITEONLY      0x00000008L
#define D3DUSAGE_DYNAMIC        0x00000200L

#define D3DLOCK_READONLY        0x00000010L
#define D3DLOCK_NOSYSLOCK       0x00000800L
#define D3DLOCK_NOOVERWRITE     0x00001000L
#define D3DLOCK_DISCARD         0x00002000L

#define D3DCREATE_FPU_PRESERVE                  0x00000002L
#define D3DCREATE_MULTITHREADED                 0x00000004L
#define D3DCREATE_SOFTWARE_VERTEXPROCESSING     0x00000020L
#define D3DCREATE_HARDWARE_VERTEXPROCESSING     0x00000040L

#define D3DCLEAR_TARGET         0x00000001L
#define D3DCLEAR_ZBUFFER        0x00000002L
#define D3DCLEAR_STENCIL        0x00000004L

#define D3DCOLORWRITEENABLE_RED   (1L << 0)
#define D3DCOLORWRITEENABLE_GREEN (1L << 1)
#define D3DCOLORWRITEENABLE_BLUE  (1L << 2)
#define D3DCOLORWRITEENABLE_ALPHA (1L << 3)

#define D3DTA_DIFFUSE   0x00000000
#define D3DTA_TEXTURE   0x00000002

#define D3DISSUE_END    (1L << 0)
#define D3DISSUE_BEGIN  (1L << 1)
#define D3DGETDATA_FLUSH (1L << 0)

#define D3DPRESENT_INTERVAL_DEFAULT   0x00000000L
#define D3DPRESENT_INTERVAL_ONE       0x00000001L
#define D3DPRESENT_INTERVAL_IMMEDIATE 0x80000000L

#define D3DFVF_XYZRHW   0x004
#define D3DFVF_DIFFUSE  0x040
#define D3DFVF_SPECULAR 0x080
#define D3DFVF_TEX1     0x100

#define D3DADAPTER_DEFAULT 0
#define D3DSGR_NO_CALIBRATION 0x00000000L

// ---- HRESULT codes --------------------------------------------------------
#define D3D_OK                  S_OK
#define _D3DFACET(code)         ((HRESULT)(0x88760000L | (code)))
#define D3DERR_NOTAVAILABLE     _D3DFACET(2154)
#define D3DERR_OUTOFVIDEOMEMORY _D3DFACET(380)
#define D3DERR_INVALIDCALL      _D3DFACET(2156)
#define D3DERR_DEVICELOST       _D3DFACET(2152)
#define D3DERR_DEVICENOTRESET   _D3DFACET(2153)
#define D3DERR_DEVICEREMOVED    _D3DFACET(2160)
#define D3DDDIERR_DEVICEREMOVED ((HRESULT)0x88760870L)

// ---- Structs --------------------------------------------------------------
typedef struct _D3DRECT { LONG x1, y1, x2, y2; } D3DRECT;

typedef struct _D3DDISPLAYMODE {
    UINT      Width;
    UINT      Height;
    UINT      RefreshRate;
    D3DFORMAT Format;
} D3DDISPLAYMODE;

typedef struct _D3DVIEWPORT9 {
    DWORD X, Y, Width, Height;
    float MinZ, MaxZ;
} D3DVIEWPORT9;

typedef struct _D3DLOCKED_RECT {
    INT   Pitch;
    void *pBits;
} D3DLOCKED_RECT;

typedef struct _D3DLOCKED_BOX {
    INT   RowPitch;
    INT   SlicePitch;
    void *pBits;
} D3DLOCKED_BOX;

typedef struct _D3DBOX {
    UINT Left, Top, Right, Bottom, Front, Back;
} D3DBOX;

typedef struct _D3DVERTEXELEMENT9 {
    WORD Stream;
    WORD Offset;
    BYTE Type;       // D3DDECLTYPE
    BYTE Method;     // D3DDECLMETHOD
    BYTE Usage;      // D3DDECLUSAGE
    BYTE UsageIndex;
} D3DVERTEXELEMENT9;
#define D3DDECL_END() { 0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0, 0 }

typedef struct _D3DSURFACE_DESC {
    D3DFORMAT           Format;
    D3DRESOURCETYPE     Type;
    DWORD               Usage;
    D3DPOOL             Pool;
    D3DMULTISAMPLE_TYPE MultiSampleType;
    DWORD               MultiSampleQuality;
    UINT                Width;
    UINT                Height;
} D3DSURFACE_DESC;

typedef struct _D3DVOLUME_DESC {
    D3DFORMAT       Format;
    D3DRESOURCETYPE Type;
    DWORD           Usage;
    D3DPOOL         Pool;
    UINT            Width;
    UINT            Height;
    UINT            Depth;
} D3DVOLUME_DESC;

typedef struct _D3DPRESENT_PARAMETERS_ {
    UINT                BackBufferWidth;
    UINT                BackBufferHeight;
    D3DFORMAT           BackBufferFormat;
    UINT                BackBufferCount;
    D3DMULTISAMPLE_TYPE MultiSampleType;
    DWORD               MultiSampleQuality;
    D3DSWAPEFFECT       SwapEffect;
    HWND                hDeviceWindow;
    BOOL                Windowed;
    BOOL                EnableAutoDepthStencil;
    D3DFORMAT           AutoDepthStencilFormat;
    DWORD               Flags;
    UINT                FullScreen_RefreshRateInHz;
    UINT                PresentationInterval;
} D3DPRESENT_PARAMETERS;

// Shader-model caps sub-structs referenced by D3DCAPS9.
typedef struct _D3DVSHADERCAPS2_0 {
    DWORD Caps;
    INT   DynamicFlowControlDepth;
    INT   NumTemps;
    INT   StaticFlowControlDepth;
} D3DVSHADERCAPS2_0;

typedef struct _D3DPSHADERCAPS2_0 {
    DWORD Caps;
    INT   DynamicFlowControlDepth;
    INT   NumTemps;
    INT   StaticFlowControlDepth;
    INT   NumInstructionSlots;
} D3DPSHADERCAPS2_0;

// D3DCAPS9 — the full Microsoft layout. The renderer reads named fields only
// (never relies on byte layout), so additions/reorders are source-safe.
typedef struct _D3DCAPS9 {
    D3DDEVTYPE DeviceType;
    UINT       AdapterOrdinal;
    DWORD      Caps;
    DWORD      Caps2;
    DWORD      Caps3;
    DWORD      PresentationIntervals;
    DWORD      CursorCaps;
    DWORD      DevCaps;
    DWORD      PrimitiveMiscCaps;
    DWORD      RasterCaps;
    DWORD      ZCmpCaps;
    DWORD      SrcBlendCaps;
    DWORD      DestBlendCaps;
    DWORD      AlphaCmpCaps;
    DWORD      ShadeCaps;
    DWORD      TextureCaps;
    DWORD      TextureFilterCaps;
    DWORD      CubeTextureFilterCaps;
    DWORD      VolumeTextureFilterCaps;
    DWORD      TextureAddressCaps;
    DWORD      VolumeTextureAddressCaps;
    DWORD      LineCaps;
    DWORD      MaxTextureWidth;
    DWORD      MaxTextureHeight;
    DWORD      MaxVolumeExtent;
    DWORD      MaxTextureRepeat;
    DWORD      MaxTextureAspectRatio;
    DWORD      MaxAnisotropy;
    float      MaxVertexW;
    float      GuardBandLeft, GuardBandTop, GuardBandRight, GuardBandBottom;
    float      ExtentsAdjust;
    DWORD      StencilCaps;
    DWORD      FVFCaps;
    DWORD      TextureOpCaps;
    DWORD      MaxTextureBlendStages;
    DWORD      MaxSimultaneousTextures;
    DWORD      VertexProcessingCaps;
    DWORD      MaxActiveLights;
    DWORD      MaxUserClipPlanes;
    DWORD      MaxVertexBlendMatrices;
    DWORD      MaxVertexBlendMatrixIndex;
    float      MaxPointSize;
    DWORD      MaxPrimitiveCount;
    DWORD      MaxVertexIndex;
    DWORD      MaxStreams;
    DWORD      MaxStreamStride;
    DWORD      VertexShaderVersion;
    DWORD      MaxVertexShaderConst;
    DWORD      PixelShaderVersion;
    float      PixelShader1xMaxValue;
    DWORD      DevCaps2;
    float      MaxNpatchTessellationLevel;
    DWORD      Reserved5;
    UINT       MasterAdapterOrdinal;
    UINT       AdapterOrdinalInGroup;
    UINT       NumberOfAdaptersInGroup;
    DWORD      DeclTypes;
    DWORD      NumSimultaneousRTs;
    DWORD      StretchRectFilterCaps;
    D3DVSHADERCAPS2_0 VS20Caps;
    D3DPSHADERCAPS2_0 PS20Caps;
    DWORD      VertexTextureFilterCaps;
    DWORD      MaxVShaderInstructionsExecuted;
    DWORD      MaxPShaderInstructionsExecuted;
    DWORD      MaxVertexShader30InstructionSlots;
    DWORD      MaxPixelShader30InstructionSlots;
} D3DCAPS9;

#endif // KISAK_D3D9TYPES_H
