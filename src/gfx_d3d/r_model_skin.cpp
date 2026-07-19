#include "r_model_skin.h"
#include "r_dvars.h"
#include <win32/win_common.h>
#include "r_dobj_skin.h"
#include <qcommon/threads.h>
#include <EffectsCore/fx_marks.h>
#include <win32/win_main.h>

bool TensionUsage[4];
float TensionBuffer[4][4000];

void __cdecl R_SkinXSurfaceSkinned(
                const XSurface *xsurf,
                const DObjSkelMat *boneMatrix,
                GfxPackedVertex *skinVerticesOut)
{
    if ( (xsurf->flags & 0x80) != 0 )
        R_SkinXSurfaceWeight(xsurf, xsurf->verts0, &xsurf->vertInfo, boneMatrix, skinVerticesOut);
    else
        R_SkinXSurfaceRigid(xsurf, xsurf->vertCount, boneMatrix, skinVerticesOut);
}

void __cdecl R_SkinXSurfaceWeight(
                const XSurface *xsurf,
                const GfxPackedVertex *inVerts,
                const XSurfaceVertexInfo *vertexInfo,
                const DObjSkelMat *boneMatrix,
                GfxPackedVertex *outVerts)
{
    float v5; // [esp+0h] [ebp-28h]
    int i; // [esp+4h] [ebp-24h]
    unsigned int numVerts; // [esp+8h] [ebp-20h]
    int tensionBufferStage; // [esp+Ch] [ebp-1Ch]
    unsigned int iVert; // [esp+10h] [ebp-18h]
    const unsigned __int16 *vertsBlend; // [esp+18h] [ebp-10h]
    const GfxPackedVertex *pverts; // [esp+1Ch] [ebp-Ch]
    int vertIndex; // [esp+20h] [ebp-8h]
    float *vertsTension; // [esp+24h] [ebp-4h]

    vertIndex = 0;
    vertsBlend = vertexInfo->vertsBlend;
    numVerts = vertexInfo->vertCount[3] + vertexInfo->vertCount[2] + vertexInfo->vertCount[1] + vertexInfo->vertCount[0];
    tensionBufferStage = -1;
    vertsTension = 0;
    if ( numVerts <= 0x3E8 && vertexInfo->tensionData && r_tension_enable->current.enabled )
    {
        Sys_EnterCriticalSection(CRITSECT_TENSION);
        for ( i = 0; i < 4; ++i )
        {
            if ( !TensionUsage[i] )
            {
                TensionUsage[i] = 1;
                tensionBufferStage = i;
                vertsTension = TensionBuffer[i];
                break;
            }
        }
        Sys_LeaveCriticalSection(CRITSECT_TENSION);
    }
    pverts = inVerts;
    if ( numVerts )
    {
        if ( vertexInfo->vertCount[0] )
        {
            R_SkinXSurfaceWeight0(inVerts, vertsBlend, vertexInfo->vertCount[0], boneMatrix, outVerts);
            pverts = &inVerts[vertexInfo->vertCount[0]];
            vertIndex = vertexInfo->vertCount[0];
            vertsBlend += vertIndex;
            if ( vertsTension )
                vertsTension += 4 * vertexInfo->vertCount[0];
        }
        if ( vertexInfo->vertCount[1] )
        {
            R_SkinXSurfaceWeight1(pverts, vertsBlend, vertexInfo->vertCount[1], boneMatrix, &outVerts[vertIndex]);
            pverts += vertexInfo->vertCount[1];
            vertIndex += vertexInfo->vertCount[1];
            vertsBlend += 3 * vertexInfo->vertCount[1];
            if ( vertsTension )
                vertsTension += 4 * vertexInfo->vertCount[1];
        }
        if ( vertexInfo->vertCount[2] )
        {
            R_SkinXSurfaceWeight2(pverts, vertsBlend, vertexInfo->vertCount[2], boneMatrix, &outVerts[vertIndex]);
            pverts += vertexInfo->vertCount[2];
            vertIndex += vertexInfo->vertCount[2];
            vertsBlend += 5 * vertexInfo->vertCount[2];
            if ( vertsTension )
                vertsTension += 4 * vertexInfo->vertCount[2];
        }
        if ( vertexInfo->vertCount[3] )
        {
            R_SkinXSurfaceWeight3(pverts, vertsBlend, vertexInfo->vertCount[3], boneMatrix, &outVerts[vertIndex]);
            if ( vertsTension )
                vertsTension += 4 * vertexInfo->vertCount[3];
        }
        if ( numVerts > 0x3E8 || !vertexInfo->tensionData || vertsTension )
        {
            if ( tensionBufferStage != -1 )
            {
                //BLOPS_NULLSUB();
                Sys_EnterCriticalSection(CRITSECT_TENSION);
                TensionUsage[tensionBufferStage] = 0;
                Sys_LeaveCriticalSection(CRITSECT_TENSION);
            }
        }
        else
        {
            for ( iVert = 0; iVert < numVerts; ++iVert )
            {
                if ( outVerts->binormalSign >= 0.0 )
                    v5 = 1.0f;
                else
                    v5 = -1.0f;
                outVerts->binormalSign = v5;
                ++inVerts;
                ++outVerts;
            }
        }
    }
}

void __cdecl R_SkinXSurfaceWeight0(
                const GfxPackedVertex *vertsIn,
                const unsigned __int16 *vertexBlend,
                int vertCount,
                const DObjSkelMat *boneMatrix,
                GfxPackedVertex *vertsOut)
{
    int vertIndex; // [esp+1Ch] [ebp-4h]

    if ( !vertsOut
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp", 1067, 0, "%s", "vertsOut") )
    {
        __debugbreak();
    }
    vertIndex = 0;
    while ( vertIndex < vertCount )
    {
        MatrixTransformVertexAndBasis(
            vertsIn->xyz,
            vertsIn->binormalSign,
            vertsIn->normal,
            vertsIn->tangent,
            (const DObjSkelMat *)((char *)boneMatrix + *vertexBlend),
            vertsOut);
        vertsOut->color.packed = vertsIn->color.packed;
        vertsOut->texCoord.packed = vertsIn->texCoord.packed;
        ++vertIndex;
        ++vertsIn;
        ++vertexBlend;
        ++vertsOut;
    }
}

void __cdecl MatrixTransformVertexAndBasis(
                const float *offset,
                float binormalSign,
                PackedUnitVec normal,
                PackedUnitVec tangent,
                const DObjSkelMat *mat,
                GfxPackedVertex *vert)
{
    PackedUnitVec v6; // [esp+0h] [ebp-78h]
    float v7[3]; // [esp+1Ch] [ebp-5Ch] BYREF
    PackedUnitVec v8; // [esp+28h] [ebp-50h]
    PackedUnitVec v9; // [esp+30h] [ebp-48h]
    PackedUnitVec v10; // [esp+34h] [ebp-44h]
    PackedUnitVec v11; // [esp+38h] [ebp-40h]
    float v12; // [esp+3Ch] [ebp-3Ch]
    float v13; // [esp+40h] [ebp-38h]
    int v14; // [esp+44h] [ebp-34h]
    float out[3]; // [esp+4Ch] [ebp-2Ch] BYREF
    PackedUnitVec in; // [esp+58h] [ebp-20h]
    float rotated[3]; // [esp+60h] [ebp-18h]
    float unpacked[3]; // [esp+6Ch] [ebp-Ch]

    vert->xyz[0] = (float)((float)((float)(*offset * mat->axis[0][0]) + (float)(offset[1] * mat->axis[1][0]))
                                             + (float)(offset[2] * mat->axis[2][0]))
                             + mat->origin[0];
    vert->xyz[1] = (float)((float)((float)(*offset * mat->axis[0][1]) + (float)(offset[1] * mat->axis[1][1]))
                                             + (float)(offset[2] * mat->axis[2][1]))
                             + mat->origin[1];
    vert->xyz[2] = (float)((float)((float)(*offset * mat->axis[0][2]) + (float)(offset[1] * mat->axis[1][2]))
                                             + (float)(offset[2] * mat->axis[2][2]))
                             + mat->origin[2];
    vert->binormalSign = binormalSign;
    out[0] = *(float *)&normal.packed;
    out[1] = *(float *)&normal.packed;
    out[2] = *(float *)&normal.packed;
    in.packed = normal.packed;
    Vec3UnpackUnitVec(normal, out);
    in.packed = 0;
    unpacked[0] = out[0];
    unpacked[1] = out[1];
    unpacked[2] = out[2];
    rotated[0] = (float)((float)(out[0] * mat->axis[0][0]) + (float)(out[1] * mat->axis[1][0]))
                         + (float)(out[2] * mat->axis[2][0]);
    rotated[1] = (float)((float)(out[0] * mat->axis[0][1]) + (float)(out[1] * mat->axis[1][1]))
                         + (float)(out[2] * mat->axis[2][1]);
    rotated[2] = (float)((float)(out[0] * mat->axis[0][2]) + (float)(out[1] * mat->axis[1][2]))
                         + (float)(out[2] * mat->axis[2][2]);
    v12 = rotated[1];
    v13 = rotated[2];
    v14 = 0;
    v9.array[0] = (int)(float)((float)(rotated[0] * 127.0) + 127.5);
    v9.array[1] = (int)(float)((float)(rotated[1] * 127.0) + 127.5);
    v9.array[2] = (int)(float)((float)(rotated[2] * 127.0) + 127.5);
    v9.array[3] = 63;
    v10.packed = v9.packed;
    v11.packed = v9.packed;
    vert->normal = v9;
    v7[0] = *(float *)&tangent.packed;
    v7[1] = *(float *)&tangent.packed;
    v7[2] = *(float *)&tangent.packed;
    v8.packed = tangent.packed;
    Vec3UnpackUnitVec(tangent, v7);
    v8.packed = 0;
    unpacked[0] = v7[0];
    unpacked[1] = v7[1];
    unpacked[2] = v7[2];
    rotated[0] = (float)((float)(v7[0] * mat->axis[0][0]) + (float)(v7[1] * mat->axis[1][0]))
                         + (float)(v7[2] * mat->axis[2][0]);
    rotated[1] = (float)((float)(v7[0] * mat->axis[0][1]) + (float)(v7[1] * mat->axis[1][1]))
                         + (float)(v7[2] * mat->axis[2][1]);
    rotated[2] = (float)((float)(v7[0] * mat->axis[0][2]) + (float)(v7[1] * mat->axis[1][2]))
                         + (float)(v7[2] * mat->axis[2][2]);
    v6.array[0] = (int)(float)((float)(rotated[0] * 127.0) + 127.5);
    v6.array[1] = (int)(float)((float)(rotated[1] * 127.0) + 127.5);
    v6.array[2] = (int)(float)((float)(rotated[2] * 127.0) + 127.5);
    v6.array[3] = 63;
    vert->tangent = v6;
}

void __cdecl R_SkinXSurfaceWeight1(
                const GfxPackedVertex *vertsIn,
                const unsigned __int16 *vertexBlend,
                int vertCount,
                const DObjSkelMat *boneMatrix,
                GfxPackedVertex *vertsOut)
{
    const float4 *matrix; // [esp+14h] [ebp-1Ch]
    float offset[3]; // [esp+1Ch] [ebp-14h] BYREF
    float boneWeight; // [esp+28h] [ebp-8h]
    int vertIndex; // [esp+2Ch] [ebp-4h]

    if ( !vertsOut
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp", 1100, 0, "%s", "vertsOut") )
    {
        __debugbreak();
    }
    vertIndex = 0;
    while ( vertIndex < vertCount )
    {
        MatrixTransformVertexAndBasis(
            vertsIn->xyz,
            vertsIn->binormalSign,
            vertsIn->normal,
            vertsIn->tangent,
            (const DObjSkelMat *)((char *)boneMatrix + *vertexBlend),
            vertsOut);
        matrix = (const float4 *)((char *)boneMatrix + vertexBlend[1]);
        boneWeight = (float)vertexBlend[2] * 0.000015258789;
        R_TransformSkelMat(vertsIn->xyz, matrix, offset);
        offset[0] = boneWeight * offset[0];
        offset[1] = boneWeight * offset[1];
        offset[2] = boneWeight * offset[2];
        boneWeight = 1.0 - boneWeight;
        vertsOut->xyz[0] = (float)(boneWeight * vertsOut->xyz[0]) + offset[0];
        vertsOut->xyz[1] = (float)(boneWeight * vertsOut->xyz[1]) + offset[1];
        vertsOut->xyz[2] = (float)(boneWeight * vertsOut->xyz[2]) + offset[2];
        vertsOut->color.packed = vertsIn->color.packed;
        vertsOut->texCoord.packed = vertsIn->texCoord.packed;
        ++vertIndex;
        ++vertsIn;
        vertexBlend += 3;
        ++vertsOut;
    }
}

void __cdecl R_SkinXSurfaceWeight2(
                const GfxPackedVertex *vertsIn,
                const unsigned __int16 *vertexBlend,
                int vertCount,
                const DObjSkelMat *boneMatrix,
                GfxPackedVertex *vertsOut)
{
    const float4 *matrix; // [esp+14h] [ebp-2Ch]
    const float4 *matrixa; // [esp+14h] [ebp-2Ch]
    float offset[3]; // [esp+1Ch] [ebp-24h] BYREF
    float boneWeight; // [esp+28h] [ebp-18h]
    float totalOffset[3]; // [esp+2Ch] [ebp-14h] BYREF
    int vertIndex; // [esp+38h] [ebp-8h]
    float totalBoneWeight; // [esp+3Ch] [ebp-4h]

    if ( !vertsOut
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp", 1152, 0, "%s", "vertsOut") )
    {
        __debugbreak();
    }
    vertIndex = 0;
    while ( vertIndex < vertCount )
    {
        MatrixTransformVertexAndBasis(
            vertsIn->xyz,
            vertsIn->binormalSign,
            vertsIn->normal,
            vertsIn->tangent,
            (const DObjSkelMat *)((char *)boneMatrix + *vertexBlend),
            vertsOut);
        matrix = (const float4 *)((char *)boneMatrix + vertexBlend[1]);
        totalBoneWeight = (float)vertexBlend[2] * 0.000015258789;
        R_TransformSkelMat(vertsIn->xyz, matrix, totalOffset);
        totalOffset[0] = totalBoneWeight * totalOffset[0];
        totalOffset[1] = totalBoneWeight * totalOffset[1];
        totalOffset[2] = totalBoneWeight * totalOffset[2];
        matrixa = (const float4 *)((char *)boneMatrix + vertexBlend[3]);
        boneWeight = (float)vertexBlend[4] * 0.000015258789;
        totalBoneWeight = totalBoneWeight + boneWeight;
        R_TransformSkelMat(vertsIn->xyz, matrixa, offset);
        totalOffset[0] = (float)(boneWeight * offset[0]) + totalOffset[0];
        totalOffset[1] = (float)(boneWeight * offset[1]) + totalOffset[1];
        totalOffset[2] = (float)(boneWeight * offset[2]) + totalOffset[2];
        boneWeight = 1.0 - totalBoneWeight;
        vertsOut->xyz[0] = (float)((float)(1.0 - totalBoneWeight) * vertsOut->xyz[0]) + totalOffset[0];
        vertsOut->xyz[1] = (float)(boneWeight * vertsOut->xyz[1]) + totalOffset[1];
        vertsOut->xyz[2] = (float)(boneWeight * vertsOut->xyz[2]) + totalOffset[2];
        vertsOut->color.packed = vertsIn->color.packed;
        vertsOut->texCoord.packed = vertsIn->texCoord.packed;
        ++vertIndex;
        ++vertsIn;
        vertexBlend += 5;
        ++vertsOut;
    }
}

void __cdecl R_SkinXSurfaceWeight3(
                const GfxPackedVertex *vertsIn,
                const unsigned __int16 *vertexBlend,
                int vertCount,
                const DObjSkelMat *boneMatrix,
                GfxPackedVertex *vertsOut)
{
    const float4 *matrix; // [esp+14h] [ebp-2Ch]
    const float4 *matrixa; // [esp+14h] [ebp-2Ch]
    const float4 *matrixb; // [esp+14h] [ebp-2Ch]
    float offset[3]; // [esp+1Ch] [ebp-24h] BYREF
    float boneWeight; // [esp+28h] [ebp-18h]
    float totalOffset[3]; // [esp+2Ch] [ebp-14h] BYREF
    int vertIndex; // [esp+38h] [ebp-8h]
    float totalBoneWeight; // [esp+3Ch] [ebp-4h]

    if ( !vertsOut
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp", 1220, 0, "%s", "vertsOut") )
    {
        __debugbreak();
    }
    vertIndex = 0;
    while ( vertIndex < vertCount )
    {
        MatrixTransformVertexAndBasis(
            vertsIn->xyz,
            vertsIn->binormalSign,
            vertsIn->normal,
            vertsIn->tangent,
            (const DObjSkelMat *)((char *)boneMatrix + *vertexBlend),
            vertsOut);
        matrix = (const float4 *)((char *)boneMatrix + vertexBlend[1]);
        totalBoneWeight = (float)vertexBlend[2] * 0.000015258789;
        R_TransformSkelMat(vertsIn->xyz, matrix, totalOffset);
        totalOffset[0] = totalBoneWeight * totalOffset[0];
        totalOffset[1] = totalBoneWeight * totalOffset[1];
        totalOffset[2] = totalBoneWeight * totalOffset[2];
        matrixa = (const float4 *)((char *)boneMatrix + vertexBlend[3]);
        boneWeight = (float)vertexBlend[4] * 0.000015258789;
        totalBoneWeight = totalBoneWeight + boneWeight;
        R_TransformSkelMat(vertsIn->xyz, matrixa, offset);
        totalOffset[0] = (float)(boneWeight * offset[0]) + totalOffset[0];
        totalOffset[1] = (float)(boneWeight * offset[1]) + totalOffset[1];
        totalOffset[2] = (float)(boneWeight * offset[2]) + totalOffset[2];
        matrixb = (const float4 *)((char *)boneMatrix + vertexBlend[5]);
        boneWeight = (float)vertexBlend[6] * 0.000015258789;
        totalBoneWeight = totalBoneWeight + boneWeight;
        R_TransformSkelMat(vertsIn->xyz, matrixb, offset);
        totalOffset[0] = (float)(boneWeight * offset[0]) + totalOffset[0];
        totalOffset[1] = (float)(boneWeight * offset[1]) + totalOffset[1];
        totalOffset[2] = (float)(boneWeight * offset[2]) + totalOffset[2];
        boneWeight = 1.0 - totalBoneWeight;
        vertsOut->xyz[0] = (float)((float)(1.0 - totalBoneWeight) * vertsOut->xyz[0]) + totalOffset[0];
        vertsOut->xyz[1] = (float)(boneWeight * vertsOut->xyz[1]) + totalOffset[1];
        vertsOut->xyz[2] = (float)(boneWeight * vertsOut->xyz[2]) + totalOffset[2];
        vertsOut->color.packed = vertsIn->color.packed;
        vertsOut->texCoord.packed = vertsIn->texCoord.packed;
        ++vertIndex;
        ++vertsIn;
        vertexBlend += 7;
        ++vertsOut;
    }
}

void __cdecl R_SkinXSurfaceRigid(
                const XSurface *surf,
                int totalVertCount,
                const DObjSkelMat *boneMatrix,
                GfxPackedVertex *vertices)
{
    unsigned int i; // [esp+14h] [ebp-1Ch]
    int vertCount; // [esp+18h] [ebp-18h]
    int vertIndex; // [esp+1Ch] [ebp-14h]
    GfxPackedVertex *vertex; // [esp+20h] [ebp-10h]
    const XRigidVertList *vertList; // [esp+24h] [ebp-Ch]
    GfxPackedVertex *v; // [esp+28h] [ebp-8h]
    const DObjSkelMat *bone; // [esp+2Ch] [ebp-4h]

    if ( !vertices
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp", 3906, 0, "%s", "vertices") )
    {
        __debugbreak();
    }
    if ( ((unsigned __int8)vertices & 0xF) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                    3907,
                    0,
                    "%s",
                    "!(reinterpret_cast< unsigned >( vertices ) & 15)") )
    {
        __debugbreak();
    }
    if ( ((unsigned __int8)boneMatrix & 0xF) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                    3908,
                    0,
                    "%s",
                    "!(reinterpret_cast< unsigned >( boneMatrix ) & 15)") )
    {
        __debugbreak();
    }
    v = surf->verts0;
    vertex = vertices;
    for ( i = 0; i < surf->vertListCount; ++i )
    {
        vertList = &surf->vertList[i];
        vertCount = vertList->vertCount;
        bone = (const DObjSkelMat *)((char *)boneMatrix + vertList->boneOffset);
        for ( vertIndex = 0; vertIndex < vertCount; ++vertIndex )
        {
            MatrixTransformVertexAndBasis(v->xyz, v->binormalSign, v->normal, v->tangent, bone, vertex);
            vertex->color.packed = v->color.packed;
            vertex->texCoord.packed = v->texCoord.packed;
            ++v;
            ++vertex;
        }
    }
    if ( vertex - vertices != totalVertCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                    3937,
                    0,
                    "%s",
                    "vertex - vertices == totalVertCount") )
    {
        __debugbreak();
    }
}

void __cdecl R_SkinXSurfaceSkinnedSse(
                const XSurface *xsurf,
                const DObjSkelMat *boneMatrix,
                GfxPackedVertexNormal *skinVertNormalIn,
                GfxPackedVertexNormal *skinVertNormalOut,
                GfxPackedVertex *skinVerticesOut)
{
#ifdef KISAK_SSE_SKINNING
    int savedregs; // [esp+0h] [ebp+0h] BYREF

    if ( skinVertNormalOut )
    {
        if ( skinVertNormalIn )
        {
            if ( (xsurf->flags & 0x80) != 0 )
                R_SkinXSurfaceWeightSseInOut(
                    xsurf->verts0,
                    &xsurf->vertInfo,
                    boneMatrix,
                    skinVertNormalIn,
                    skinVertNormalOut,
                    skinVerticesOut);
            else
                R_SkinXSurfaceRigidSseInOut(
                    xsurf,
                    xsurf->vertCount,
                    boneMatrix,
                    (__m64 *)skinVertNormalIn,
                    (__m64 *)skinVertNormalOut,
                    skinVerticesOut);
        }
        else if ( (xsurf->flags & 0x80) != 0 )
        {
            R_SkinXSurfaceWeightSseOut(xsurf->verts0, &xsurf->vertInfo, boneMatrix, skinVertNormalOut, skinVerticesOut);
        }
        else
        {
            R_SkinXSurfaceRigidSseOut(
                xsurf,
                xsurf->vertCount,
                boneMatrix,
                (__m64 *)skinVertNormalOut,
                skinVerticesOut);
        }
    }
    else
    {
        if ( skinVertNormalIn
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                        4216,
                        0,
                        "%s",
                        "!skinVertNormalIn") )
        {
            __debugbreak();
        }
        if ( (xsurf->flags & 0x80) != 0 )
            R_SkinXSurfaceWeightSse(xsurf->verts0, &xsurf->vertInfo, boneMatrix, skinVerticesOut);
        else
            R_SkinXSurfaceRigidSse(xsurf, xsurf->vertCount, boneMatrix, skinVerticesOut);
    }
#endif
}

void __cdecl R_SkinXSurfaceWeightSse(
                const GfxPackedVertex *inVerts,
                const XSurfaceVertexInfo *vertexInfo,
                const DObjSkelMat *boneMatrix,
                GfxPackedVertex *outVerts)
{
#ifdef KISAK_SSE_SKINNING
    const unsigned __int16 *vertsBlend; // [esp+0h] [ebp-8h]
    int vertIndex; // [esp+4h] [ebp-4h] BYREF
    int savedregs; // [esp+8h] [ebp+0h] BYREF

    vertIndex = 0;
    vertsBlend = vertexInfo->vertsBlend;
    if ( vertexInfo->vertCount[0] )
    {
        R_SkinXSurfaceWeightSseBlock_1_Sse_SkinVertex_0_(
            (const unsigned __int16 *)&savedregs,
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[0],
            boneMatrix,
            outVerts,
            &vertIndex);
        vertsBlend += vertexInfo->vertCount[0];
    }
    if ( vertexInfo->vertCount[1] )
    {
        R_SkinXSurfaceWeightSseBlock_3_Sse_SkinVertex_1_(
            (const unsigned __int16 *)&savedregs,
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[1],
            boneMatrix,
            outVerts,
            &vertIndex);
        vertsBlend += 3 * vertexInfo->vertCount[1];
    }
    if ( vertexInfo->vertCount[2] )
    {
        R_SkinXSurfaceWeightSseBlock_5_Sse_SkinVertex_2_(
            (const unsigned __int16 *)&savedregs,
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[2],
            boneMatrix,
            outVerts,
            &vertIndex);
        vertsBlend += 5 * vertexInfo->vertCount[2];
    }
    if ( vertexInfo->vertCount[3] )
        R_SkinXSurfaceWeightSseBlock_7_Sse_SkinVertex_3_(
            (const unsigned __int16 *)&savedregs,
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[3],
            boneMatrix,
            outVerts,
            &vertIndex);
#endif
}

void __cdecl R_SkinXSurfaceWeightSseOut(
                const GfxPackedVertex *inVerts,
                const XSurfaceVertexInfo *vertexInfo,
                const DObjSkelMat *boneMatrix,
                GfxPackedVertexNormal *outVertNormals,
                GfxPackedVertex *outVerts)
{
#ifdef KISAK_SSE_SKINNING
    const unsigned __int16 *vertsBlend; // [esp+0h] [ebp-8h]
    int vertIndex; // [esp+4h] [ebp-4h] BYREF
    int savedregs; // [esp+8h] [ebp+0h] BYREF

    vertIndex = 0;
    vertsBlend = vertexInfo->vertsBlend;
    if ( vertexInfo->vertCount[0] )
    {
        R_SkinXSurfaceWeightSseBlockOut_1_Sse_SkinVertex_0_(
            (const unsigned __int16 *)&savedregs,
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[0],
            boneMatrix,
            outVertNormals,
            outVerts,
            &vertIndex);
        vertsBlend += vertexInfo->vertCount[0];
    }
    if ( vertexInfo->vertCount[1] )
    {
        R_SkinXSurfaceWeightSseBlockOut_3_Sse_SkinVertex_1_(
            (const unsigned __int16 *)&savedregs,
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[1],
            boneMatrix,
            outVertNormals,
            outVerts,
            &vertIndex);
        vertsBlend += 3 * vertexInfo->vertCount[1];
    }
    if ( vertexInfo->vertCount[2] )
    {
        R_SkinXSurfaceWeightSseBlockOut_5_Sse_SkinVertex_2_(
            (const unsigned __int16 *)&savedregs,
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[2],
            boneMatrix,
            outVertNormals,
            outVerts,
            &vertIndex);
        vertsBlend += 5 * vertexInfo->vertCount[2];
    }
    if ( vertexInfo->vertCount[3] )
        R_SkinXSurfaceWeightSseBlockOut_7_Sse_SkinVertex_3_(
            (const unsigned __int16 *)&savedregs,
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[3],
            boneMatrix,
            outVertNormals,
            outVerts,
            &vertIndex);
#endif
}

void __cdecl R_SkinXSurfaceWeightSseInOut(
                const GfxPackedVertex *inVerts,
                const XSurfaceVertexInfo *vertexInfo,
                const DObjSkelMat *boneMatrix,
                const GfxPackedVertexNormal *inVertNormals,
                GfxPackedVertexNormal *outVertNormals,
                GfxPackedVertex *outVerts)
{
#ifdef KISAK_SSE_SKINNING
    const unsigned __int16 *vertsBlend; // [esp+0h] [ebp-8h]
    int vertIndex; // [esp+4h] [ebp-4h] BYREF
    int savedregs; // [esp+8h] [ebp+0h] BYREF

    vertIndex = 0;
    vertsBlend = vertexInfo->vertsBlend;
    if ( vertexInfo->vertCount[0] )
    {
        R_SkinXSurfaceWeightSseBlockInOut_1_Sse_SkinVertexSimple_0_(
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[0],
            boneMatrix,
            inVertNormals,
            outVertNormals,
            outVerts,
            &vertIndex);
        vertsBlend += vertexInfo->vertCount[0];
    }
    if ( vertexInfo->vertCount[1] )
    {
        R_SkinXSurfaceWeightSseBlockInOut_3_Sse_SkinVertexSimple_1_(
            (const unsigned __int16 *)&savedregs,
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[1],
            boneMatrix,
            inVertNormals,
            outVertNormals,
            outVerts,
            &vertIndex);
        vertsBlend += 3 * vertexInfo->vertCount[1];
    }
    if ( vertexInfo->vertCount[2] )
    {
        R_SkinXSurfaceWeightSseBlockInOut_5_Sse_SkinVertexSimple_2_(
            (const unsigned __int16 *)&savedregs,
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[2],
            boneMatrix,
            inVertNormals,
            outVertNormals,
            outVerts,
            &vertIndex);
        vertsBlend += 5 * vertexInfo->vertCount[2];
    }
    if ( vertexInfo->vertCount[3] )
        R_SkinXSurfaceWeightSseBlockInOut_7_Sse_SkinVertexSimple_3_(
            (const unsigned __int16 *)&savedregs,
            inVerts,
            vertsBlend,
            vertexInfo->vertCount[3],
            boneMatrix,
            inVertNormals,
            outVertNormals,
            outVerts,
            &vertIndex);
#endif
}

void    R_SkinXSurfaceRigidSse(
                const XSurface *surf,
                int totalVertCount,
                const DObjSkelMat *boneMatrix,
                GfxPackedVertex *dstVerts)
{
#ifdef KISAK_SSE_SKINNING
    __m128 v5; // [esp+D4h] [ebp-63Ch]
    __m128 v6; // [esp+1C4h] [ebp-54Ch]
    __m128 v7; // [esp+214h] [ebp-4FCh]
    __m128 v8; // [esp+234h] [ebp-4DCh]
    __m128 v9; // [esp+244h] [ebp-4CCh]
    __m128 v10; // [esp+344h] [ebp-3CCh]
    __m128 v11; // [esp+434h] [ebp-2DCh]
    __m128 v12; // [esp+484h] [ebp-28Ch]
    __m128 v13; // [esp+4A4h] [ebp-26Ch]
    __m128 v14; // [esp+4F4h] [ebp-21Ch]
    __m128 v15; // [esp+564h] [ebp-1ACh]
    __m64 v16; // [esp+66Ch] [ebp-A4h]
    int i; // [esp+6A0h] [ebp-70h]
    __m128 v18; // [esp+6A4h] [ebp-6Ch]
    __m128 matrix_4; // [esp+6B4h] [ebp-5Ch]
    __m128 matrix_20; // [esp+6C4h] [ebp-4Ch]
    __m128 matrix_36; // [esp+6D4h] [ebp-3Ch]
    __m128 *matrix_56; // [esp+6E8h] [ebp-28h]
    int matrix_60; // [esp+6ECh] [ebp-24h]
    XRigidVertList *v24; // [esp+6F0h] [ebp-20h]
    const __m128 *matrixAddress; // [esp+6F4h] [ebp-1Ch]
    __m64 *vertCount; // [esp+6F8h] [ebp-18h]
    __m128 *vertList; // [esp+6FCh] [ebp-14h]
    GfxPackedVertex *vertex; // [esp+704h] [ebp-Ch] BYREF
    const GfxPackedVertex *v; // [esp+708h] [ebp-8h]
    const GfxPackedVertex *retaddr; // [esp+710h] [ebp+0h]

    vertex = a1;
    v = retaddr;
    if ( !dstVerts
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp", 3972, 0, "%s", "dstVerts") )
    {
        __debugbreak();
    }
    if ( ((unsigned __int8)dstVerts & 0xF) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                    3973,
                    0,
                    "%s",
                    "!(reinterpret_cast< unsigned >( dstVerts ) & 15)") )
    {
        __debugbreak();
    }
    if ( ((unsigned __int8)boneMatrix & 0xF) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                    3974,
                    0,
                    "%s",
                    "!(reinterpret_cast< unsigned >( boneMatrix ) & 15)") )
    {
        __debugbreak();
    }
    vertList = (__m128 *)surf->verts0;
    vertCount = (__m64 *)dstVerts;
    for ( matrixAddress = 0;
                (unsigned int)matrixAddress < surf->vertListCount;
                matrixAddress = (const __m128 *)((char *)matrixAddress + 1) )
    {
        v24 = &surf->vertList[(unsigned int)matrixAddress];
        matrix_60 = v24->vertCount;
        matrix_56 = (__m128 *)((char *)boneMatrix + v24->boneOffset);
        v18 = *matrix_56;
        matrix_4 = matrix_56[1];
        matrix_20 = matrix_56[2];
        matrix_36 = matrix_56[3];
        for ( i = 0; i < matrix_60; ++i )
        {
            _mm_prefetch((const char *)&vertList[8], 0);
            v16 = (__m64)vertList[1].m128_u64[0];
            v15 = _mm_add_ps(
                            _mm_add_ps(
                                _mm_mul_ps(v18, _mm_shuffle_ps(*vertList, *vertList, 0)),
                                _mm_mul_ps(matrix_4, _mm_shuffle_ps(*vertList, *vertList, 85))),
                            _mm_add_ps(_mm_mul_ps(matrix_20, _mm_shuffle_ps(*vertList, *vertList, 170)), matrix_36));
            v14 = _mm_shuffle_ps(v15, _mm_unpackhi_ps(v15, *vertList), 196);
            _mm_cvtpu16_ps((int)&vertex);
            v12 = _mm_div_ps(_mm_sub_ps(v14, sse_encodeShift), sse_encodeScale);
            v11 = _mm_mul_ps(v12, _mm_shuffle_ps(v12, v12, 255));
            v10 = _mm_add_ps(
                            _mm_add_ps(
                                _mm_mul_ps(v18, _mm_shuffle_ps(v11, v11, 0)),
                                _mm_mul_ps(matrix_4, _mm_shuffle_ps(v11, v11, 85))),
                            _mm_mul_ps(matrix_20, _mm_shuffle_ps(v11, v11, 170)));
            v13 = _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(v10, _mm_unpackhi_ps(v10, matrix_36), 196), sse_encodeScale),
                            sse_encodeShift);
            _mm_cvtpu16_ps((int)&vertex);
            v9 = _mm_movehl_ps(v13, v13);
            v7 = _mm_div_ps(_mm_sub_ps(v9, sse_encodeShift), sse_encodeScale);
            v6 = _mm_mul_ps(v7, _mm_shuffle_ps(v7, v7, 255));
            v5 = _mm_add_ps(
                         _mm_add_ps(_mm_mul_ps(v18, _mm_shuffle_ps(v6, v6, 0)), _mm_mul_ps(matrix_4, _mm_shuffle_ps(v6, v6, 85))),
                         _mm_mul_ps(matrix_20, _mm_shuffle_ps(v6, v6, 170)));
            v8 = _mm_add_ps(
                         _mm_mul_ps(_mm_shuffle_ps(v5, _mm_unpackhi_ps(v5, matrix_36), 196), sse_encodeScale),
                         sse_encodeShift);
            _mm_stream_ps((float *)vertCount, v14);
            _mm_stream_pi(vertCount + 2, v16);
            _mm_stream_pi(
                vertCount + 3,
                _m_packuswb(
                    _m_packuswb(_mm_cvt_ps2pi(v13), _mm_cvt_ps2pi(v9)),
                    _m_packuswb(_mm_cvt_ps2pi(v8), _mm_cvt_ps2pi(_mm_movehl_ps(v8, v8)))));
            vertList += 2;
            vertCount += 4;
        }
    }
    if ( ((char *)vertCount - (char *)dstVerts) >> 5 != totalVertCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                    4011,
                    0,
                    "%s",
                    "vertex - dstVerts == totalVertCount") )
    {
        __debugbreak();
    }
#endif
}

#ifdef KISAK_SSE_SKINNING
__m128 * _mm_cvtpu16_ps@<eax>(int a1@<ebp>)
{
    __m64 aa; // [esp+50h] [ebp-10h] BYREF
    void *v4; // [esp+58h] [ebp-8h]
    void *retaddr; // [esp+60h] [ebp+0h]

    aa.m64_i32[1] = a1;
    v4 = retaddr;
    return (__m128 *)((unsigned int)&aa.m64_u32[1] ^ __security_cookie);
}
#endif

void    R_SkinXSurfaceRigidSseOut(
                const XSurface *surf,
                int totalVertCount,
                const DObjSkelMat *boneMatrix,
                GfxPackedVertexNormal *dstVertNormals,
                GfxPackedVertex *dstVerts)
{
#ifdef KISAK_SSE_SKINNING
    __m64 v6; // [esp-Ch] [ebp-71Ch]
    __m128 v7; // [esp+D4h] [ebp-63Ch]
    __m128 v8; // [esp+1C4h] [ebp-54Ch]
    __m128 v9; // [esp+214h] [ebp-4FCh]
    __m128 v10; // [esp+234h] [ebp-4DCh]
    __m128 v11; // [esp+244h] [ebp-4CCh]
    __m128 v12; // [esp+344h] [ebp-3CCh]
    __m128 v13; // [esp+434h] [ebp-2DCh]
    __m128 v14; // [esp+484h] [ebp-28Ch]
    __m128 v15; // [esp+4A4h] [ebp-26Ch]
    __m128 v16; // [esp+4F4h] [ebp-21Ch]
    __m128 v17; // [esp+564h] [ebp-1ACh]
    __m64 v18; // [esp+66Ch] [ebp-A4h]
    int v19; // [esp+6A0h] [ebp-70h]
    __m128 v20; // [esp+6A4h] [ebp-6Ch]
    __m128 matrix_4; // [esp+6B4h] [ebp-5Ch]
    __m128 matrix_20; // [esp+6C4h] [ebp-4Ch]
    __m128 matrix_36; // [esp+6D4h] [ebp-3Ch]
    __m128 *matrix_52; // [esp+6E4h] [ebp-2Ch]
    int matrix_56; // [esp+6E8h] [ebp-28h]
    XRigidVertList *matrix_60; // [esp+6ECh] [ebp-24h]
    const __m128 *matrixAddress; // [esp+6F0h] [ebp-20h]
    __m64 *vertCount; // [esp+6F4h] [ebp-1Ch]
    __m64 *vertList; // [esp+6F8h] [ebp-18h]
    __m128 *i; // [esp+6FCh] [ebp-14h]
    GfxPackedVertex *vertex; // [esp+704h] [ebp-Ch] BYREF
    const GfxPackedVertex *v; // [esp+708h] [ebp-8h]
    const GfxPackedVertex *retaddr; // [esp+710h] [ebp+0h]

    vertex = a1;
    v = retaddr;
    if ( !dstVerts
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp", 4044, 0, "%s", "dstVerts") )
    {
        __debugbreak();
    }
    if ( !dstVertNormals
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                    4045,
                    0,
                    "%s",
                    "dstVertNormals") )
    {
        __debugbreak();
    }
    if ( ((unsigned __int8)dstVerts & 0xF) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                    4046,
                    0,
                    "%s",
                    "!(reinterpret_cast< unsigned >( dstVerts ) & 15)") )
    {
        __debugbreak();
    }
    if ( ((unsigned __int8)dstVertNormals & 7) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                    4047,
                    0,
                    "%s",
                    "!(reinterpret_cast< unsigned >( dstVertNormals ) & 7)") )
    {
        __debugbreak();
    }
    if ( ((unsigned __int8)boneMatrix & 0xF) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                    4048,
                    0,
                    "%s",
                    "!(reinterpret_cast< unsigned >( boneMatrix ) & 15)") )
    {
        __debugbreak();
    }
    i = (__m128 *)surf->verts0;
    vertList = (__m64 *)dstVerts;
    vertCount = dstVertNormals;
    for ( matrixAddress = 0;
                (unsigned int)matrixAddress < surf->vertListCount;
                matrixAddress = (const __m128 *)((char *)matrixAddress + 1) )
    {
        matrix_60 = &surf->vertList[(unsigned int)matrixAddress];
        matrix_56 = matrix_60->vertCount;
        matrix_52 = (__m128 *)((char *)boneMatrix + matrix_60->boneOffset);
        v20 = *matrix_52;
        matrix_4 = matrix_52[1];
        matrix_20 = matrix_52[2];
        matrix_36 = matrix_52[3];
        v19 = 0;
        while ( v19 < matrix_56 )
        {
            _mm_prefetch((const char *)&i[8], 0);
            v18 = (__m64)i[1].m128_u64[0];
            v17 = _mm_add_ps(
                            _mm_add_ps(_mm_mul_ps(v20, _mm_shuffle_ps(*i, *i, 0)), _mm_mul_ps(matrix_4, _mm_shuffle_ps(*i, *i, 85))),
                            _mm_add_ps(_mm_mul_ps(matrix_20, _mm_shuffle_ps(*i, *i, 170)), matrix_36));
            v16 = _mm_shuffle_ps(v17, _mm_unpackhi_ps(v17, *i), 196);
            _mm_cvtpu16_ps((int)&vertex);
            v14 = _mm_div_ps(_mm_sub_ps(v16, sse_encodeShift), sse_encodeScale);
            v13 = _mm_mul_ps(v14, _mm_shuffle_ps(v14, v14, 255));
            v12 = _mm_add_ps(
                            _mm_add_ps(
                                _mm_mul_ps(v20, _mm_shuffle_ps(v13, v13, 0)),
                                _mm_mul_ps(matrix_4, _mm_shuffle_ps(v13, v13, 85))),
                            _mm_mul_ps(matrix_20, _mm_shuffle_ps(v13, v13, 170)));
            v15 = _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(v12, _mm_unpackhi_ps(v12, matrix_36), 196), sse_encodeScale),
                            sse_encodeShift);
            _mm_cvtpu16_ps((int)&vertex);
            v11 = _mm_movehl_ps(v15, v15);
            v9 = _mm_div_ps(_mm_sub_ps(v11, sse_encodeShift), sse_encodeScale);
            v8 = _mm_mul_ps(v9, _mm_shuffle_ps(v9, v9, 255));
            v7 = _mm_add_ps(
                         _mm_add_ps(_mm_mul_ps(v20, _mm_shuffle_ps(v8, v8, 0)), _mm_mul_ps(matrix_4, _mm_shuffle_ps(v8, v8, 85))),
                         _mm_mul_ps(matrix_20, _mm_shuffle_ps(v8, v8, 170)));
            v10 = _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(v7, _mm_unpackhi_ps(v7, matrix_36), 196), sse_encodeScale),
                            sse_encodeShift);
            v6 = _m_packuswb(
                         _m_packuswb(_mm_cvt_ps2pi(v15), _mm_cvt_ps2pi(v11)),
                         _m_packuswb(_mm_cvt_ps2pi(v10), _mm_cvt_ps2pi(_mm_movehl_ps(v10, v10))));
            _mm_stream_ps((float *)vertList, v16);
            _mm_stream_pi(vertList + 2, v18);
            _mm_stream_pi(vertList + 3, v6);
            _mm_stream_pi(vertCount, v6);
            ++v19;
            i += 2;
            vertList += 4;
            ++vertCount;
        }
    }
    if ( ((char *)vertList - (char *)dstVerts) >> 5 != totalVertCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                    4086,
                    0,
                    "%s",
                    "vertex - dstVerts == totalVertCount") )
    {
        __debugbreak();
    }
    if ( vertCount - dstVertNormals != totalVertCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                    4087,
                    0,
                    "%s",
                    "vertexNormal - dstVertNormals == totalVertCount") )
    {
        __debugbreak();
    }
#endif
}

void __cdecl R_SkinXSurfaceRigidSseInOut(
                const XSurface *surf,
                int totalVertCount,
                const DObjSkelMat *boneMatrix,
                GfxPackedVertexNormal *srcVertNormals,
                GfxPackedVertexNormal *dstVertNormals,
                GfxPackedVertex *dstVerts)
{
#ifdef KISAK_SSE_SKINNING
    __m128 v6; // [esp+64h] [ebp-1BCh]
    __m64 m64_u64; // [esp+164h] [ebp-BCh]
    __m64 v8; // [esp+16Ch] [ebp-B4h]
    int v9; // [esp+1A0h] [ebp-80h]
    __m128 v10; // [esp+1A4h] [ebp-7Ch]
    __m128 matrix_4; // [esp+1B4h] [ebp-6Ch]
    __m128 matrix_20; // [esp+1C4h] [ebp-5Ch]
    __m128 matrix_36; // [esp+1D4h] [ebp-4Ch]
    __m128 *v14; // [esp+1F0h] [ebp-30h]
    int v15; // [esp+1F4h] [ebp-2Ch]
    XRigidVertList *v16; // [esp+1F8h] [ebp-28h]
    const __m128 *matrixAddress; // [esp+1FCh] [ebp-24h]
    __m64 *vertCount; // [esp+200h] [ebp-20h]
    __m64 *vertList; // [esp+204h] [ebp-1Ch]
    __m128 *vertexNormal; // [esp+20Ch] [ebp-14h]

    if ( !dstVerts
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp", 4121, 0, "%s", "dstVerts") )
    {
        __debugbreak();
    }
    if ( !dstVertNormals
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                    4122,
                    0,
                    "%s",
                    "dstVertNormals") )
    {
        __debugbreak();
    }
    if ( ((unsigned __int8)dstVerts & 0xF) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                    4123,
                    0,
                    "%s",
                    "!(reinterpret_cast< unsigned >( dstVerts ) & 15)") )
    {
        __debugbreak();
    }
    if ( ((unsigned __int8)dstVertNormals & 7) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                    4124,
                    0,
                    "%s",
                    "!(reinterpret_cast< unsigned >( dstVertNormals ) & 7)") )
    {
        __debugbreak();
    }
    if ( ((unsigned __int8)boneMatrix & 0xF) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                    4125,
                    0,
                    "%s",
                    "!(reinterpret_cast< unsigned >( boneMatrix ) & 15)") )
    {
        __debugbreak();
    }
    vertexNormal = (__m128 *)surf->verts0;
    vertList = (__m64 *)dstVerts;
    vertCount = dstVertNormals;
    for ( matrixAddress = 0;
                (unsigned int)matrixAddress < surf->vertListCount;
                matrixAddress = (const __m128 *)((char *)matrixAddress + 1) )
    {
        v16 = &surf->vertList[(unsigned int)matrixAddress];
        v15 = v16->vertCount;
        v14 = (__m128 *)((char *)boneMatrix + v16->boneOffset);
        v10 = *v14;
        matrix_4 = v14[1];
        matrix_20 = v14[2];
        matrix_36 = v14[3];
        v9 = 0;
        while ( v9 < v15 )
        {
            _mm_prefetch((const char *)&vertexNormal[8], 0);
            _mm_prefetch((const char *)&srcVertNormals[16], 0);
            v8 = (__m64)vertexNormal[1].m128_u64[0];
            m64_u64 = (__m64)srcVertNormals->m64_u64;
            v6 = _mm_add_ps(
                         _mm_add_ps(
                             _mm_mul_ps(v10, _mm_shuffle_ps(*vertexNormal, *vertexNormal, 0)),
                             _mm_mul_ps(matrix_4, _mm_shuffle_ps(*vertexNormal, *vertexNormal, 85))),
                         _mm_add_ps(_mm_mul_ps(matrix_20, _mm_shuffle_ps(*vertexNormal, *vertexNormal, 170)), matrix_36));
            _mm_stream_ps((float *)vertList, _mm_shuffle_ps(v6, _mm_unpackhi_ps(v6, *vertexNormal), 196));
            _mm_stream_pi(vertList + 2, v8);
            _mm_stream_pi(vertList + 3, m64_u64);
            _mm_stream_pi(vertCount, m64_u64);
            ++v9;
            vertexNormal += 2;
            ++srcVertNormals;
            vertList += 4;
            ++vertCount;
        }
    }
    if ( ((char *)vertList - (char *)dstVerts) >> 5 != totalVertCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                    4165,
                    0,
                    "%s",
                    "vertex - dstVerts == totalVertCount") )
    {
        __debugbreak();
    }
    if ( vertCount - dstVertNormals != totalVertCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                    4166,
                    0,
                    "%s",
                    "vertexNormal - dstVertNormals == totalVertCount") )
    {
        __debugbreak();
    }
#endif
}

void R_SkinXModelCmd(SkinXModelCmd *data)
{
    GfxPackedVertexNormal *normalOut; // [esp-28D0h] [ebp-28DCh]
    GfxPackedVertexNormal *normalIn; // [esp-28CCh] [ebp-28D8h]
    GfxPackedVertex *skinVerticesOut; // [esp-28C8h] [ebp-28D4h]
    const XSurface *xsurf; // [esp-28C4h] [ebp-28D0h]
    // DObjSkelMat is alignas(16) and R_SkinXSurfaceSkinnedSse does movaps on these,
    // but GCC doesn't 16-align this large stack array for us (no stack realignment);
    // hand-align a pointer into over-allocated storage.
    char boneSkelMatsStorage[DOBJ_MAX_PARTS * sizeof(DObjSkelMat) + 16];
    DObjSkelMat *boneSkelMats = reinterpret_cast<DObjSkelMat *>(
        (reinterpret_cast<uintptr_t>(boneSkelMatsStorage) + 15u) & ~uintptr_t(15u));
    int j; // [esp-30h] [ebp-3Ch]
    GfxModelSkinnedSurface *skinnedSurf; // [esp-24h] [ebp-30h]
    unsigned int i; // [esp-20h] [ebp-2Ch]
    int boneIndex; // [esp-1Ch] [ebp-28h]
    GfxModelSkinnedSurface *surfPos; // [esp-18h] [ebp-24h]
    const DObjAnimMat *mat; // [esp-14h] [ebp-20h]
    GfxModelSkinnedSurface *modelSurfs; // [esp-10h] [ebp-1Ch]
    SkinXModelCmd *skinCmd; // [esp-Ch] [ebp-18h]
    bool fastSkin; // [esp-7h] [ebp-13h]
    char v23; // [esp-6h] [ebp-12h]
    bool useSSE_; // [esp-5h] [ebp-11h]
    BOOL useSSE; // [esp-4h] [ebp-10h]

    if (!Sys_QueryD3DDeviceOKEvent())
    {
        return;
    }

    PROF_SCOPED("R_SkinXModelCmd");

    useSSE = sys_SSE->current.enabled && r_sse_skinning->current.enabled;
    useSSE_ = useSSE;
    v23 = 0;
    fastSkin = gfxBuf.fastSkin;
    skinCmd = data;
    modelSurfs = (GfxModelSkinnedSurface *)data->modelSurfs;
    mat = data->mat;

    iassert(skinCmd);
    iassert(skinCmd->surfCount);
        
    surfPos = modelSurfs;
    boneIndex = -1;
    for (i = 0; i < skinCmd->surfCount; ++i)
    {
        skinnedSurf = surfPos;
        if (surfPos->skinnedCachedOffset == -3)
        {
            surfPos = (GfxModelSkinnedSurface *)((char *)surfPos + 4);
            continue;
        }

        if (boneIndex != skinnedSurf->info.boneIndex)
        {
            boneIndex = skinnedSurf->info.boneIndex;
            const int totalBones = boneIndex + skinnedSurf->info.boneCount;
            const DObjAnimMat *baseMats = &skinnedSurf->info.baseMat[-boneIndex];
            for (j = boneIndex; j < totalBones; ++j)
            {
                if ((skinCmd->surfacePartBits[j >> 5] & (0x80000000 >> (j & 0x1F))) == 0)
                    continue;

                //if (v23)
                //{
                //    v23 = 0;
                //    _m_empty();
                //}

                DObjSkelMat mat0, mat1;

                ConvertQuatToInverseSkelMat(&baseMats[j], &mat0);
                ConvertQuatToSkelMat(&mat[j], &mat1);

                mat1.origin[0] = mat1.origin[0] - skinCmd->viewoffset[0];
                mat1.origin[1] = mat1.origin[1] - skinCmd->viewoffset[1];
                mat1.origin[2] = mat1.origin[2] - skinCmd->viewoffset[2];
                R_MultiplySkelMat(&mat0, &mat1, &boneSkelMats[j]);

                boneSkelMats[j].axis[0][3] = 0.0f;
                boneSkelMats[j].axis[1][3] = 0.0f;
                boneSkelMats[j].axis[2][3] = 0.0f;
                boneSkelMats[j].origin[3] = 1.0f;
            }
        }

        if (skinnedSurf->skinnedCachedOffset == -2)
        {
            surfPos = (GfxModelSkinnedSurface *)((char *)surfPos + 56);
            continue;
        }

        surfPos = skinnedSurf + 1;
        xsurf = skinnedSurf->xsurf;
        iassert(xsurf);

        if (skinnedSurf->skinnedCachedOffset < 0)
        {
            iassert(((reinterpret_cast<uint>(skinnedSurf->skinnedVert) & 15) == 0));
            skinVerticesOut = skinnedSurf->skinnedVert;
        }
        else
        {
            iassert(gfxBuf.skinnedCacheLockAddr);
            iassert((reinterpret_cast<uint>(gfxBuf.skinnedCacheLockAddr) & 15) == 0);
            iassert((skinnedSurf->skinnedCachedOffset & 15) == 0);

            skinVerticesOut = (GfxPackedVertex *)&gfxBuf.skinnedCacheLockAddr[skinnedSurf->skinnedCachedOffset];
        }

        if (useSSE_)
        {
            if (!v23)
            {
                v23 = 1;
                _m_empty();
            }
            normalIn = 0;
            normalOut = 0;
            if (fastSkin)
            {
                if (skinnedSurf->skinnedCachedOffset >= 0)
                    normalOut = &gfxBuf.skinnedCacheNormalsAddr[skinnedSurf->skinnedCachedOffset >> 5];
                if (skinnedSurf->oldSkinnedCachedOffset >= 0)
                    normalIn = &gfxBuf.oldSkinnedCacheNormalsAddr[skinnedSurf->oldSkinnedCachedOffset >> 5];
            }
            R_SkinXSurfaceSkinnedSse(xsurf, &boneSkelMats[boneIndex], normalIn, normalOut, skinVerticesOut);
        }
        else
        {
            R_SkinXSurfaceSkinned(xsurf, &boneSkelMats[boneIndex], skinVerticesOut);
        }
    }

    //if (v23)
    //    _m_empty();
}

void __cdecl R_MultiplySkelMat(const DObjSkelMat *mat0, const DObjSkelMat *mat1, DObjSkelMat *out)
{
    out->axis[0][0] = (float)((float)(mat0->axis[0][0] * mat1->axis[0][0]) + (float)(mat0->axis[0][1] * mat1->axis[1][0]))
                                    + (float)(mat0->axis[0][2] * mat1->axis[2][0]);
    out->axis[0][1] = (float)((float)(mat0->axis[0][0] * mat1->axis[0][1]) + (float)(mat0->axis[0][1] * mat1->axis[1][1]))
                                    + (float)(mat0->axis[0][2] * mat1->axis[2][1]);
    out->axis[0][2] = (float)((float)(mat0->axis[0][0] * mat1->axis[0][2]) + (float)(mat0->axis[0][1] * mat1->axis[1][2]))
                                    + (float)(mat0->axis[0][2] * mat1->axis[2][2]);
    out->axis[1][0] = (float)((float)(mat0->axis[1][0] * mat1->axis[0][0]) + (float)(mat0->axis[1][1] * mat1->axis[1][0]))
                                    + (float)(mat0->axis[1][2] * mat1->axis[2][0]);
    out->axis[1][1] = (float)((float)(mat0->axis[1][0] * mat1->axis[0][1]) + (float)(mat0->axis[1][1] * mat1->axis[1][1]))
                                    + (float)(mat0->axis[1][2] * mat1->axis[2][1]);
    out->axis[1][2] = (float)((float)(mat0->axis[1][0] * mat1->axis[0][2]) + (float)(mat0->axis[1][1] * mat1->axis[1][2]))
                                    + (float)(mat0->axis[1][2] * mat1->axis[2][2]);
    out->axis[2][0] = (float)((float)(mat0->axis[2][0] * mat1->axis[0][0]) + (float)(mat0->axis[2][1] * mat1->axis[1][0]))
                                    + (float)(mat0->axis[2][2] * mat1->axis[2][0]);
    out->axis[2][1] = (float)((float)(mat0->axis[2][0] * mat1->axis[0][1]) + (float)(mat0->axis[2][1] * mat1->axis[1][1]))
                                    + (float)(mat0->axis[2][2] * mat1->axis[2][1]);
    out->axis[2][2] = (float)((float)(mat0->axis[2][0] * mat1->axis[0][2]) + (float)(mat0->axis[2][1] * mat1->axis[1][2]))
                                    + (float)(mat0->axis[2][2] * mat1->axis[2][2]);
    out->origin[0] = (float)((float)((float)(mat0->origin[0] * mat1->axis[0][0])
                                                                 + (float)(mat0->origin[1] * mat1->axis[1][0]))
                                                 + (float)(mat0->origin[2] * mat1->axis[2][0]))
                                 + mat1->origin[0];
    out->origin[1] = (float)((float)((float)(mat0->origin[0] * mat1->axis[0][1])
                                                                 + (float)(mat0->origin[1] * mat1->axis[1][1]))
                                                 + (float)(mat0->origin[2] * mat1->axis[2][1]))
                                 + mat1->origin[1];
    out->origin[2] = (float)((float)((float)(mat0->origin[0] * mat1->axis[0][2])
                                                                 + (float)(mat0->origin[1] * mat1->axis[1][2]))
                                                 + (float)(mat0->origin[2] * mat1->axis[2][2]))
                                 + mat1->origin[2];
}

#ifdef KISAK_SSE_SKINNING
__m128 * _mm_cvtpu8_ps@<eax>(int a1@<ebp>)
{
    __m64 aa; // [esp+0h] [ebp-10h] BYREF
    void *v4; // [esp+8h] [ebp-8h]
    void *retaddr; // [esp+10h] [ebp+0h]

    aa.m64_i32[1] = a1;
    v4 = retaddr;
    return _mm_cvtpu16_ps((int)&aa.m64_i32[1]);
}
#endif
