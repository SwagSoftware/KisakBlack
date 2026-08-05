#include "r_xsurface_optimize.h"
#include <qcommon/threads.h>
#include "rb_resource.h"
#include <xanim/xmodel_utils.h>
#include "r_buffers.h"
#include "r_init.h"

void __cdecl XModelOptimizeCallback(XModel *data)
{
    _XModelOptimize(data);
}

void __cdecl XModelOptimize(XModel *model)
{
    if ( XModelBad(model)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_optimize.cpp",
                    118,
                    0,
                    "%s",
                    "!XModelBad( model )") )
    {
        __debugbreak();
    }
    if ( Sys_IsRenderThread() )
    {
        _XModelOptimize(model);
    }
    else
    {
        RB_Resource_CallbackParam((void (__cdecl *)(void *))XModelOptimizeCallback, model);
        RB_Resource_Flush();
    }
}

void __cdecl _XModelOptimize(XModel *model)
{
    int lodIndex; // [esp+0h] [ebp-14h]
    int surfIndex; // [esp+4h] [ebp-10h]
    int surfCount; // [esp+8h] [ebp-Ch]
    int lodCount; // [esp+Ch] [ebp-8h]
    XSurface *surfaces; // [esp+10h] [ebp-4h] BYREF

    lodCount = XModelGetNumLods(model);
    for ( lodIndex = 0; lodIndex < lodCount; ++lodIndex )
    {
        surfCount = XModelGetSurfaces(model, &surfaces, lodIndex);
        for ( surfIndex = 0; surfIndex < surfCount; ++surfIndex )
            XSurfaceOptimize(model, &surfaces[surfIndex]);
    }
}

void __cdecl XSurfaceOptimize(XModel *model, XSurface *surf)
{
    void *indexBuffer; // [esp+0h] [ebp-Ch]
    int rawIndexBytes; // [esp+4h] [ebp-8h]
    signed int indexBytes; // [esp+8h] [ebp-4h]

    if ( surf->indexBuffer )
        goto LABEL_10;
    if ( !surf->triIndices
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_optimize.cpp",
                    71,
                    0,
                    "%s",
                    "surf->triIndices") )
    {
        __debugbreak();
    }
    rawIndexBytes = 6 * surf->triCount;
    indexBytes = (rawIndexBytes + 31) & 0xFFFFFFE0;
    if ( indexBytes > 0x200000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_optimize.cpp",
                    75,
                    0,
                    "%s",
                    "indexBytes <= MATERIAL_MAX_INDEX_BUFFER_SIZE") )
    {
        __debugbreak();
    }
    indexBuffer = R_AllocStaticIndexBuffer(&surf->indexBuffer, indexBytes);
    if ( indexBuffer )
    {
        Com_Memcpy(indexBuffer, surf->triIndices, rawIndexBytes);
        memset((unsigned char *)indexBuffer + rawIndexBytes, 0, indexBytes - rawIndexBytes);
        R_FinishStaticIndexBuffer(surf->indexBuffer);
LABEL_10:
        if ( (surf->flags & 0x80) == 0 )
            XSurfaceOptimizeRigid(model, surf);
    }
}

void __cdecl XSurfaceOptimizeRigid(XModel *model, XSurface *surface)
{
    IDirect3DVertexBuffer9 *vb; // [esp+0h] [ebp-10h] BYREF
    HRESULT hr; // [esp+4h] [ebp-Ch]
    void *vertexBuffer; // [esp+8h] [ebp-8h] BYREF
    int vertexBytes; // [esp+Ch] [ebp-4h]

    if ( (surface->flags & 0x80) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_optimize.cpp",
                    22,
                    0,
                    "%s",
                    "!(surface->flags & XSURFACE_FLAG_DEFORMED)") )
    {
        __debugbreak();
    }
    if ( !surface->vb0 )
    {
        vertexBytes = 32 * surface->vertCount;
        hr = dx.device->CreateVertexBuffer(vertexBytes, 8u, 0, D3DPOOL_DEFAULT, &vb, 0);
        if ( hr >= 0 )
        {
            //hr = ((int (__thiscall *)(IDirect3DVertexBuffer9 *, IDirect3DVertexBuffer9 *, unsigned int, unsigned int, void **, unsigned int))vb->Lock)(
            //             vb,
            //             vb,
            //             0,
            //             0,
            //             &vertexBuffer,
            //             0);
            hr = vb->Lock(0, 0, &vertexBuffer, 0);
            if ( hr >= 0 )
            {
                surface->vb0 = vb;
                memcpy((unsigned __int8 *)vertexBuffer, (unsigned __int8 *)surface->verts0, vertexBytes);
                R_FinishStaticVertexBuffer(vb);
            }
            else
            {
                vb->Release();
            }
        }
    }
}

void __cdecl XModelUnoptimizeCallback(XModel *data)
{
    XModelUnoptimize(data);
}

void __cdecl XModelUnoptimize(XModel *model)
{
    int lodIndex; // [esp+0h] [ebp-14h]
    int surfIndex; // [esp+4h] [ebp-10h]
    int surfCount; // [esp+8h] [ebp-Ch]
    int lodCount; // [esp+Ch] [ebp-8h]
    XSurface *surfaces; // [esp+10h] [ebp-4h] BYREF

    if ( Sys_IsRenderThread() )
    {
        if ( XModelBad(model)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_optimize.cpp",
                        179,
                        0,
                        "%s",
                        "!XModelBad( model )") )
        {
            __debugbreak();
        }
        lodCount = XModelGetNumLods(model);
        for ( lodIndex = 0; lodIndex < lodCount; ++lodIndex )
        {
            surfCount = XModelGetSurfaces(model, &surfaces, lodIndex);
            for ( surfIndex = 0; surfIndex < surfCount; ++surfIndex )
                XSurfaceUnoptimize(&surfaces[surfIndex]);
        }
    }
    else
    {
        RB_Resource_CallbackParam((void (__cdecl *)(void *))XModelUnoptimizeCallback, model);
        RB_Resource_Flush();
    }
}

void __cdecl XSurfaceUnoptimize(XSurface *surf)
{
    if ( surf->indexBuffer )
    {
        R_FreeStaticIndexBuffer(surf->indexBuffer);
        surf->indexBuffer = 0;
    }
    if ( (surf->flags & 0x80) == 0 )
    {
        if ( surf->vb0 )
            XSurfaceUnoptimizeRigid(surf);
    }
}

void __cdecl XSurfaceUnoptimizeRigid(XSurface *surf)
{
    if ( (surf->flags & 0x80) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_optimize.cpp",
                    56,
                    0,
                    "%s",
                    "!(surf->flags & XSURFACE_FLAG_DEFORMED)") )
    {
        __debugbreak();
    }
    if ( !surf->vb0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_optimize.cpp",
                    57,
                    0,
                    "%s",
                    "surf->vb0") )
    {
        __debugbreak();
    }
    R_FreeStaticVertexBuffer(surf->vb0);
    surf->vb0 = 0;
}

