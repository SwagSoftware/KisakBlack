#include "r_xsurface_load_obj.h"

#include <universal/aabbtree.h>
#include <universal/com_memory.h>
#include "r_xsurface.h"
#include "r_dvars.h"
#include <qcommon/com_bsp_load_obj.h>
#include <set>
#include <algorithm>  // std::min/std::max (MSVC <windows.h> would supply bare macros)

#if 0
void __cdecl XModelReadSurface(XModel *model, unsigned __int8 **pos, void *(__cdecl *Alloc)(int), XSurface *surface)
{
    int vertCount; // edx
    __int64 v5; // rax
    __int64 v6; // rax
    __int64 v7; // rax
    unsigned __int16 *v8; // [esp+8h] [ebp-D54h]
    unsigned __int8 v9; // [esp+Ch] [ebp-D50h]
    XRigidVertList *v10; // [esp+10h] [ebp-D4Ch]
    float v11; // [esp+18h] [ebp-D44h]
    std::_Tree<std::_Tset_traits<unsigned __int64,std::less<unsigned __int64>,std::allocator<unsigned __int64>,0> >::iterator *v12; // [esp+2E0h] [ebp-A7Ch]
    unsigned __int16 v13; // [esp+350h] [ebp-A0Ch]
    float v14; // [esp+354h] [ebp-A08h]
    float v15; // [esp+358h] [ebp-A04h]
    float v16; // [esp+35Ch] [ebp-A00h]
    float v17; // [esp+360h] [ebp-9FCh]
    float v18; // [esp+364h] [ebp-9F8h]
    float v19; // [esp+368h] [ebp-9F4h]
    __int16 v20; // [esp+36Ch] [ebp-9F0h]
    float v21; // [esp+380h] [ebp-9DCh]
    float v22; // [esp+384h] [ebp-9D8h]
    float v23; // [esp+388h] [ebp-9D4h]
    float v24; // [esp+38Ch] [ebp-9D0h]
    float v25; // [esp+390h] [ebp-9CCh]
    float v26; // [esp+394h] [ebp-9C8h]
    float v27; // [esp+398h] [ebp-9C4h]
    float v28; // [esp+39Ch] [ebp-9C0h]
    float v29; // [esp+3A0h] [ebp-9BCh]
    float v30; // [esp+3A4h] [ebp-9B8h]
    float v31; // [esp+3B4h] [ebp-9A8h]
    float v32; // [esp+3B8h] [ebp-9A4h]
    float v33; // [esp+3BCh] [ebp-9A0h]
    __int16 v34; // [esp+3C0h] [ebp-99Ch]
    __int16 v35; // [esp+3C4h] [ebp-998h]
    unsigned __int16 v36; // [esp+3C8h] [ebp-994h]
    unsigned __int16 triCount_; // [esp+3CCh] [ebp-990h]
    unsigned __int16 vertCount_; // [esp+3D0h] [ebp-98Ch]
    std::pair<std::_Tree<std::_Tset_traits<unsigned __int64,std::less<unsigned __int64>,std::allocator<unsigned __int64>,0> >::iterator,bool> v39; // [esp+3D8h] [ebp-984h] BYREF
    std::_Aux_cont *Myownedaux; // [esp+3E4h] [ebp-978h]
    std::_Tree_nod<std::_Tset_traits<unsigned __int64,std::less<unsigned __int64>,std::allocator<unsigned __int64>,0> >::_Node *Myhead; // [esp+3E8h] [ebp-974h]
    std::_Tree<std::_Tset_traits<unsigned __int64,std::less<unsigned __int64>,std::allocator<unsigned __int64>,0> >::iterator result; // [esp+3ECh] [ebp-970h] BYREF
    XVertexInfo_s tmp[2]; // [esp+3F4h] [ebp-968h] BYREF
    int db; // [esp+478h] [ebp-8E4h]
    int src; // [esp+47Ch] [ebp-8E0h]
    int dst; // [esp+480h] [ebp-8DCh]
    int kk; // [esp+484h] [ebp-8D8h]
    int idx; // [esp+488h] [ebp-8D4h]
    int jj; // [esp+48Ch] [ebp-8D0h]
    int v50; // [esp+490h] [ebp-8CCh]
    int v51; // [esp+494h] [ebp-8C8h]
    int v52; // [esp+498h] [ebp-8C4h]
    int score; // [esp+49Ch] [ebp-8C0h]
    int k; // [esp+4A0h] [ebp-8BCh]
    unsigned __int16 v55; // [esp+4A4h] [ebp-8B8h]
    int best_k; // [esp+4A8h] [ebp-8B4h]
    unsigned __int16 v57; // [esp+4ACh] [ebp-8B0h]
    int best_score; // [esp+4B0h] [ebp-8ACh]
    unsigned __int16 v59; // [esp+4B4h] [ebp-8A8h]
    int ii; // [esp+4B8h] [ebp-8A4h]
    unsigned __int16 *vertFlags; // [esp+4BCh] [ebp-8A0h]
    int move_count; // [esp+4C0h] [ebp-89Ch]
    XVertexBuffer *v63; // [esp+4C4h] [ebp-898h]
    unsigned __int16 *indices; // [esp+4C8h] [ebp-894h]
    int num_indices; // [esp+4CCh] [ebp-890h]
    unsigned __int16 next_index; // [esp+4D0h] [ebp-88Ch]
    int c; // [esp+4D4h] [ebp-888h]
    int b; // [esp+4D8h] [ebp-884h]
    unsigned __int64 normTri; // [esp+4DCh] [ebp-880h] BYREF
    int a; // [esp+4E4h] [ebp-878h]
    int n; // [esp+4E8h] [ebp-874h]
    int num_tris; // [esp+4ECh] [ebp-870h]
    int write_i; // [esp+4F0h] [ebp-86Ch]
    std::set<unsigned __int64> tris; // [esp+4F4h] [ebp-868h] BYREF
    int num_dupes; // [esp+514h] [ebp-848h]
    float check[3]; // [esp+518h] [ebp-844h]
    float Value; // [esp+524h] [ebp-838h]
    int m; // [esp+528h] [ebp-834h]
    float MagicNumber; // [esp+52Ch] [ebp-830h]
    int j; // [esp+530h] [ebp-82Ch]
    unsigned __int8 numWeights; // [esp+537h] [ebp-825h]
    int weightCount[4]; // [esp+538h] [ebp-824h] BYREF
    XVertexInfo0 *vert0Out; // [esp+548h] [ebp-814h]
    XVertexInfo2 *vert2Out; // [esp+54Ch] [ebp-810h]
    int boneIndex; // [esp+550h] [ebp-80Ch]
    int numblends; // [esp+554h] [ebp-808h]
    XBlendLoadInfo *blendOut; // [esp+558h] [ebp-804h]
    int allocCount; // [esp+55Ch] [ebp-800h]
    int blendBoneOffset; // [esp+560h] [ebp-7FCh]
    XRigidVertList *rigidVertList; // [esp+564h] [ebp-7F8h]
    int rigidVertCount; // [esp+568h] [ebp-7F4h]
    int vertexBytes; // [esp+56Ch] [ebp-7F0h]
    int size; // [esp+570h] [ebp-7ECh]
    bool deformed; // [esp+577h] [ebp-7E5h]
    XVertexInfo2 *verts2; // [esp+578h] [ebp-7E4h]
    XVertexInfo3 *verts3; // [esp+57Ch] [ebp-7E0h]
    int startTriIndex; // [esp+580h] [ebp-7DCh]
    unsigned int vertListIter; // [esp+584h] [ebp-7D8h]
    int localBoneIndex; // [esp+588h] [ebp-7D4h]
    XVertexInfo1 *verts1; // [esp+58Ch] [ebp-7D0h]
    int vertListCount; // [esp+590h] [ebp-7CCh]
    XRigidVertList rigidVertListArray[161]; // [esp+594h] [ebp-7C8h] BYREF
    XVertexInfo0 *verts0; // [esp+D28h] [ebp-34h]
    XVertexInfo_s *verts; // [esp+D2Ch] [ebp-30h]
    int blendBoneIndex; // [esp+D30h] [ebp-2Ch]
    XVertexInfo1 *vert1Out; // [esp+D34h] [ebp-28h]
    int endIndex; // [esp+D38h] [ebp-24h]
    int i; // [esp+D3Ch] [ebp-20h]
    unsigned __int16 *vertsBlendOut; // [esp+D40h] [ebp-1Ch]
    int triIndex; // [esp+D44h] [ebp-18h]
    int vertIndex; // [esp+D48h] [ebp-14h]
    XVertexInfo0 *vertOut; // [esp+D4Ch] [ebp-10h]
    int sizeInBytes; // [esp+D50h] [ebp-Ch]
    XVertexBuffer *surfVerts; // [esp+D54h] [ebp-8h]
    XVertexInfo3 *vert3Out; // [esp+D58h] [ebp-4h]

    memset(weightCount, 0, sizeof(weightCount));
    surface->flags = 0;
    surface->tileMode = *(*pos)++;
    *pos += 2;
    vertCount_ = *(_WORD *)*pos;
    *pos += 2;
    surface->vertCount = vertCount_;
    triCount_ = *(_WORD *)*pos;
    *pos += 2;
    surface->triCount = triCount_;
    if ( !surface->triCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                    966,
                    0,
                    "%s\n\t(surface->triCount) = %i",
                    "(surface->triCount > 0)",
                    surface->triCount) )
    {
        __debugbreak();
    }
    vertListCount = 0;
    rigidVertCount = 0;
    while ( 1 )
    {
        if ( (unsigned int)vertListCount >= 161
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                        973,
                        0,
                        "%s",
                        "vertListCount < ARRAY_COUNT( rigidVertListArray )") )
        {
            __debugbreak();
        }
        rigidVertList = &rigidVertListArray[vertListCount];
        v36 = *(_WORD *)*pos;
        *pos += 2;
        rigidVertList->vertCount = v36;
        if ( !rigidVertList->vertCount )
            break;
        v35 = *(_WORD *)*pos;
        *pos += 2;
        localBoneIndex = v35;
        rigidVertList->boneOffset = v35 << 6;
        rigidVertCount += rigidVertList->vertCount;
        ++vertListCount;
    }
    vertCount = surface->vertCount;
    deformed = rigidVertCount != vertCount;
    if ( rigidVertCount != vertCount )
    {
        vertListCount = 0;
        surface->flags |= 0x82u;
    }
    if ( vertListCount == 1 )
    {
        numblends = 0;
        boneIndex = rigidVertListArray[0].boneOffset >> 6;
        surface->partBits[rigidVertListArray[0].boneOffset >> 11] |= 0x80000000 >> (boneIndex & 0x1F);
    }
    else
    {
        surface->flags |= 2u;
        v34 = *(_WORD *)*pos;
        *pos += 2;
        numblends = v34;
    }
    size = (surface->vertCount << 6) + 4 * numblends;
    surfVerts = (XVertexBuffer *)Hunk_AllocateTempMemory(size, "XModelReadSurface");
    verts = &surfVerts->v;
    for ( j = 0; j < surface->vertCount; ++j )
    {
        v33 = *(float *)*pos;
        *pos += 4;
        verts->normal[0] = v33;

        v32 = *(float *)*pos;
        *pos += 4;
        verts->normal[1] = v32;

        v31 = *(float *)*pos;
        *pos += 4;
        verts->normal[2] = v31;

        if ( r_modelVertColor->current.enabled )
            Byte4CopyBgraToVertexColor(*pos, verts->color);
        else
            *(unsigned int *)verts->color = -1;
        *pos += 4;

        v30 = *(float *)*pos;
        *pos += 4;
        verts->texCoordX = v30;

        v29 = *(float *)*pos;
        *pos += 4;
        verts->texCoordY = v29;

        v28 = *(float *)*pos;
        *pos += 4;
        verts->binormal[0] = v28;

        v27 = *(float *)*pos;
        *pos += 4;
        verts->binormal[1] = v27;

        v26 = *(float *)*pos;
        *pos += 4;
        verts->binormal[2] = v26;

        v25 = *(float *)*pos;
        *pos += 4;
        verts->tangent[0] = v25;

        v24 = *(float *)*pos;
        *pos += 4;
        verts->tangent[1] = v24;

        v23 = *(float *)*pos;
        *pos += 4;
        verts->tangent[2] = v23;

        v22 = *(float *)*pos;
        *pos += 4;
        MagicNumber = v22;

        if ( v22 == 27397.0 )
        {
            if ( !surface->vertInfo.tensionData )
            {
                surface->vertInfo.tensionData = (float *)Alloc(48 * surface->vertCount);
                memset((unsigned __int8 *)surface->vertInfo.tensionData, 0, 48 * surface->vertCount);
            }
            for ( m = 0; m < 12; ++m )
            {
                v21 = *(float *)*pos;
                *pos += 4;
                Value = v21;
                if ( v21 == 0.0 )
                    v11 = 1.0f;
                else
                    v11 = 1.0 / Value;
                surface->vertInfo.tensionData[12 * j + m] = v11;
            }
        }
        else if ( MagicNumber != 27398.0 )
        {
            *pos -= 4;
        }
        check[0] = (float)((float)(verts->normal[0] * verts->tangent[0]) + (float)(verts->normal[1] * verts->tangent[1]))
                         + (float)(verts->normal[2] * verts->tangent[2]);
        check[1] = (float)((float)(verts->tangent[0] * verts->binormal[0]) + (float)(verts->tangent[1] * verts->binormal[1]))
                         + (float)(verts->tangent[2] * verts->binormal[2]);
        check[2] = (float)((float)(verts->binormal[0] * verts->normal[0]) + (float)(verts->binormal[1] * verts->normal[1]))
                         + (float)(verts->binormal[2] * verts->normal[2]);
        if ( vertListCount == 1 )
        {
            if ( deformed
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                            1114,
                            0,
                            "%s",
                            "!deformed") )
            {
                __debugbreak();
            }
            verts->numWeights = 0;
            verts->boneOffset = rigidVertListArray[0].boneOffset;

            v16 = *(float *)*pos;
            *pos += 4;
            verts->offset[0] = v16;

            v15 = *(float *)*pos;
            *pos += 4;
            verts->offset[1] = v15;

            v14 = *(float *)*pos;
            *pos += 4;
            verts->offset[2] = v14;

            ++verts;
        }
        else
        {
            numWeights = *(*pos)++;
            verts->numWeights = numWeights;
            if ( numWeights >= 4u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                            1086,
                            0,
                            "%s\n\t(numWeights) = %i",
                            "(numWeights < 4)",
                            numWeights) )
            {
                __debugbreak();
            }
            ++weightCount[numWeights];
            v20 = *(_WORD *)*pos;
            *pos += 2;
            blendBoneIndex = v20;
            surface->partBits[v20 >> 5] |= 0x80000000 >> (v20 & 0x1F);
            blendBoneOffset = blendBoneIndex << 6;
            verts->boneOffset = (_WORD)blendBoneIndex << 6;
            if ( blendBoneOffset != verts->boneOffset
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                            1093,
                            0,
                            "%s",
                            "blendBoneOffset == verts->boneOffset") )
            {
                __debugbreak();
            }
            v19 = *(float *)*pos;
            *pos += 4;
            verts->offset[0] = v19;
            v18 = *(float *)*pos;
            *pos += 4;
            verts->offset[1] = v18;
            v17 = *(float *)*pos;
            *pos += 4;
            verts->offset[2] = v17;
            ++verts;
            if ( numWeights )
            {
                if ( !deformed
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                                1103,
                                0,
                                "%s",
                                "deformed") )
                {
                    __debugbreak();
                }
                for ( i = 0; i < numWeights; ++i )
                {
                    ReadBlend(surface, surface->partBits, (XBlendLoadInfo *)verts, pos);
                    verts = (XVertexInfo_s *)((char *)verts + 4);
                }
            }
        }
    }
    allocCount = (surface->triCount + 1) & 0xFFFFFFFE;
    sizeInBytes = 6 * (surface->triCount + 1);
    surface->triIndices = (unsigned __int16 *)Alloc(sizeInBytes);
    if ( !surface->triIndices
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                    1161,
                    0,
                    "%s",
                    "surface->triIndices") )
    {
        __debugbreak();
    }
    for ( vertIndex = 0; vertIndex < 3 * surface->triCount; ++vertIndex )
    {
        v13 = *(_WORD *)*pos;
        *pos += 2;
        surface->triIndices[vertIndex] = v13;
        if ( surface->triIndices[vertIndex] >= (int)surface->vertCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                        1166,
                        0,
                        "%s",
                        "surface->triIndices[vertIndex] < surface->vertCount") )
        {
            __debugbreak();
        }
    }
    if ( vertListCount == 1 )
    {
        num_dupes = 0;
        num_tris = vertIndex / 3;
        write_i = 0;
        std::set<unsigned __int64>::set<unsigned __int64>(&tris);
        for ( n = 0; n < vertIndex; n += 3 )
        {
            a = surface->triIndices[n];
            b = surface->triIndices[n + 1];
            c = surface->triIndices[n + 2];
            if ( a >= b || a >= c )
            {
                if ( b >= c || b >= a )
                {
                    v7 = (__int64)a << 16;
                    LODWORD(normTri) = v7 | c;
                    HIDWORD(normTri) = b | HIDWORD(v7) | (c >> 31);
                }
                else
                {
                    v6 = (__int64)c << 16;
                    LODWORD(normTri) = v6 | b;
                    HIDWORD(normTri) = a | HIDWORD(v6) | (b >> 31);
                }
            }
            else
            {
                v5 = (__int64)b << 16;
                LODWORD(normTri) = v5 | a;
                HIDWORD(normTri) = c | HIDWORD(v5) | (a >> 31);
            }
            Myownedaux = 0;
            Myhead = tris._Myhead;
            if ( !&tris )
                _invalid_parameter_noinfo();
            Myownedaux = tris._Myownedaux;
            v12 = std::_Tree<std::_Tset_traits<unsigned __int64,std::less<unsigned __int64>,std::allocator<unsigned __int64>,0>>::find(
                            &tris,
                            &result,
                            &normTri);
            if ( !v12->_Myaux || v12->_Myaux != Myownedaux )
                _invalid_parameter_noinfo();
            if ( v12->_Ptr == Myhead )
            {
                surface->triIndices[write_i++] = a;
                surface->triIndices[write_i++] = b;
                surface->triIndices[write_i++] = c;
                std::_Tree<std::_Tset_traits<unsigned __int64,std::less<unsigned __int64>,std::allocator<unsigned __int64>,0>>::insert(
                    &tris,
                    &v39,
                    &normTri);
            }
            else
            {
                ++num_dupes;
            }
        }
        if ( num_dupes )
        {
            surface->triCount -= num_dupes;
            vertIndex = 3 * surface->triCount;
            if ( model->name )
                Com_Printf(16, "deleted %d duplicate polys from %d poly mesh %s\n", num_dupes, num_tris, model->name);
            else
                Com_Printf(16, "deleted %d duplicate polys from %d poly mesh %s\n", num_dupes, num_tris, "(null)");
        }
        std::_Tree<std::_Tset_traits<unsigned __int64,std::less<unsigned __int64>,std::allocator<unsigned __int64>,0>>::~_Tree<std::_Tset_traits<unsigned __int64,std::less<unsigned __int64>,std::allocator<unsigned __int64>,0>>(&tris);
    }
    triIndex = 0;
    endIndex = 0;
    for ( j = 0; j < vertListCount; ++j )
    {
        startTriIndex = triIndex;
        rigidVertListArray[j].triOffset = triIndex;
        if ( rigidVertListArray[j].triOffset != startTriIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                        1220,
                        0,
                        "%s",
                        "rigidVertListArray[j].triOffset == startTriIndex") )
        {
            __debugbreak();
        }
        endIndex += rigidVertListArray[j].vertCount;
        while ( triIndex < surface->triCount && surface->triIndices[3 * triIndex] < endIndex )
            ++triIndex;
        rigidVertListArray[j].triCount = triIndex - startTriIndex;
        if ( rigidVertListArray[j].triCount != triIndex - startTriIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                        1225,
                        0,
                        "%s",
                        "rigidVertListArray[j].triCount == triIndex - startTriIndex") )
        {
            __debugbreak();
        }
    }
    if ( vertListCount == 1 )
    {
        vertFlags = (unsigned __int16 *)Hunk_AllocateTempMemory(2 * surface->vertCount, "XModelReadSurface");
        memset((unsigned __int8 *)vertFlags, 0, 2 * surface->vertCount);
        indices = surface->triIndices;
        vertFlags[*indices] = 1;
        vertFlags[indices[1]] = 1;
        vertFlags[indices[2]] = 1;
        num_indices = 3 * surface->triCount;
        for ( ii = 3; ii < num_indices; ii += 3 )
        {
            best_k = ii;
            best_score = 0;
            for ( k = ii; k < num_indices; k += 3 )
            {
                v52 = indices[k];
                v51 = indices[k + 1];
                v50 = indices[k + 2];
                score = (vertFlags[v51] + vertFlags[v52] + vertFlags[v50] + 1) << 24;
                score -= v50 + v51 + v52;
                if ( score > best_score )
                {
                    best_score = score;
                    best_k = k;
                }
            }
            v59 = indices[best_k];
            v57 = indices[best_k + 1];
            v55 = indices[best_k + 2];
            vertFlags[v59] = 1;
            vertFlags[v57] = 1;
            vertFlags[v55] = 1;
            if ( best_k != ii )
            {
                indices[best_k] = indices[ii];
                indices[best_k + 1] = indices[ii + 1];
                indices[best_k + 2] = indices[ii + 2];
                indices[ii] = v59;
                indices[ii + 1] = v57;
                indices[ii + 2] = v55;
            }
        }
        memset((unsigned __int8 *)vertFlags, 0, 2 * surface->vertCount);
        next_index = 1;
        for ( jj = 0; jj < num_indices; ++jj )
        {
            idx = indices[jj];
            if ( !vertFlags[idx] )
                vertFlags[idx] = next_index++;
            indices[jj] = vertFlags[idx] - 1;
        }
        if ( next_index != surface->vertCount + 1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                        1307,
                        1,
                        "%s",
                        "next_index == surface->vertCount + 1") )
        {
            __debugbreak();
        }
        v63 = surfVerts;
        move_count = 0;
        for ( kk = 0; kk < surface->vertCount; ++kk )
        {
            src = kk;
            dst = vertFlags[kk] - 1;
            if ( kk != dst )
            {
                db = 0;
                memcpy(&tmp[1], &v63->v + src, sizeof(XVertexInfo_s));
                while ( src != dst )
                {
                    memcpy(&tmp[db], &v63->v + dst, sizeof(XVertexInfo_s));
                    db = 1 - db;
                    memcpy(&v63->v + dst, &tmp[db], sizeof(XVertexInfo_s));
                    vertFlags[src] = src + 1;
                    src = dst;
                    dst = vertFlags[dst] - 1;
                    ++move_count;
                }
            }
        }
        if ( move_count > surface->vertCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                        1334,
                        1,
                        "%s",
                        "move_count <= surface->vertCount") )
        {
            __debugbreak();
        }
        Hunk_FreeTempMemory((char *)vertFlags);
    }
    if ( (surface->triCount & 1) != 0 )
    {
        if ( allocCount <= surface->triCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                        1344,
                        1,
                        "%s",
                        "allocCount > surface->triCount") )
        {
            __debugbreak();
        }
        surface->triIndices[vertIndex] = surface->triIndices[vertIndex - 1];
        surface->triIndices[vertIndex + 1] = surface->triIndices[vertIndex - 1];
        surface->triIndices[vertIndex + 2] = surface->triIndices[vertIndex - 1];
        ++surface->triCount;
    }
    if ( vertListCount > 255 )
        Com_Error(ERR_FATAL, "vertListCount > 255 !");

    if ( vertListCount )
        v10 = (XRigidVertList *)Alloc(12 * vertListCount);
    else
        v10 = 0;

    surface->vertList = v10;
    surface->vertListCount = vertListCount;
    memcpy((unsigned __int8 *)surface->vertList, (unsigned __int8 *)rigidVertListArray, 12 * vertListCount);
    vertexBytes = 32;
    vertexBytes = 32 * surface->vertCount;
    surface->verts0 = (GfxPackedVertex *)Alloc(vertexBytes);
    memset((unsigned __int8 *)surface->verts0, 0, vertexBytes);
    model->memUsage += vertexBytes;
    XSurfaceTransfer(surfVerts, surface->verts0, surface->verts0, surface->vertCount);
    if ( deformed )
    {
        if ( XModelNumBones((const cpose_t *)model) <= 1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                        1479,
                        0,
                        "%s",
                        "XModelNumBones( model ) > 1") )
        {
            __debugbreak();
        }
        verts0 = 0;
        verts1 = 0;
        verts2 = 0;
        verts3 = 0;
        if ( weightCount[0] )
        {
            size = 2 * weightCount[0];
            verts0 = (XVertexInfo0 *)Alloc(2 * weightCount[0]);
        }
        if ( weightCount[1] )
        {
            size = 6 * weightCount[1];
            verts1 = (XVertexInfo1 *)Alloc(6 * weightCount[1]);
        }
        if ( weightCount[2] )
        {
            size = 10 * weightCount[2];
            verts2 = (XVertexInfo2 *)Alloc(10 * weightCount[2]);
        }
        if ( weightCount[3] )
        {
            size = 14 * weightCount[3];
            verts3 = (XVertexInfo3 *)Alloc(14 * weightCount[3]);
        }
        for ( i = 0; i < 4; ++i )
        {
            surface->vertInfo.vertCount[i] = weightCount[i];
            if ( surface->vertInfo.vertCount[i] != weightCount[i]
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                            1513,
                            0,
                            "%s",
                            "surface->vertInfo.vertCount[i] == weightCount[i]") )
            {
                __debugbreak();
            }
        }
        verts = &surfVerts->v;
        vert0Out = verts0;
        vert1Out = verts1;
        vert2Out = verts2;
        vert3Out = verts3;
        for ( j = 0; j < surface->vertCount; ++j )
        {
            v9 = verts->numWeights;
            if ( v9 )
            {
                if ( v9 == 1 )
                {
                    if ( !vert1Out
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                                    1534,
                                    0,
                                    "%s",
                                    "vert1Out") )
                    {
                        __debugbreak();
                    }
                    vertOut = &vert1Out->vert0;
                    blendOut = vert1Out->blend;
                    ++vert1Out;
                }
                else if ( v9 == 2 )
                {
                    if ( !vert2Out
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                                    1541,
                                    0,
                                    "%s",
                                    "vert2Out") )
                    {
                        __debugbreak();
                    }
                    vertOut = &vert2Out->vert0;
                    blendOut = vert2Out->blend;
                    ++vert2Out;
                }
                else
                {
                    if ( verts->numWeights != 3
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                                    1548,
                                    0,
                                    "%s\n\t(verts->numWeights) = %i",
                                    "(verts->numWeights == 3)",
                                    verts->numWeights) )
                    {
                        __debugbreak();
                    }
                    if ( !vert3Out
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                                    1549,
                                    0,
                                    "%s",
                                    "vert3Out") )
                    {
                        __debugbreak();
                    }
                    vertOut = &vert3Out->vert0;
                    blendOut = vert3Out->blend;
                    ++vert3Out;
                }
            }
            else
            {
                if ( !vert0Out
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                                1527,
                                0,
                                "%s",
                                "vert0Out") )
                {
                    __debugbreak();
                }
                vertOut = vert0Out;
                blendOut = 0;
                ++vert0Out;
            }
            vertOut->boneOffset = verts->boneOffset;
            numWeights = verts->numWeights;
            ++verts;
            if ( numWeights )
            {
                if ( !blendOut
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                                1563,
                                0,
                                "%s",
                                "blendOut") )
                {
                    __debugbreak();
                }
                for ( i = 0; i < numWeights; ++i )
                {
                    blendOut->boneOffset = LOWORD(verts->normal[0]);
                    blendOut->boneWeight = HIWORD(verts->normal[0]);
                    ++blendOut;
                    verts = (XVertexInfo_s *)((char *)verts + 4);
                }
            }
        }
        if ( vert0Out != &verts0[surface->vertInfo.vertCount[0]]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                        1575,
                        0,
                        "%s",
                        "vert0Out == verts0 + surface->vertInfo.vertCount[0]") )
        {
            __debugbreak();
        }
        if ( vert1Out != &verts1[surface->vertInfo.vertCount[1]]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                        1576,
                        0,
                        "%s",
                        "vert1Out == verts1 + surface->vertInfo.vertCount[1]") )
        {
            __debugbreak();
        }
        if ( vert2Out != &verts2[surface->vertInfo.vertCount[2]]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                        1577,
                        0,
                        "%s",
                        "vert2Out == verts2 + surface->vertInfo.vertCount[2]") )
        {
            __debugbreak();
        }
        if ( vert3Out != &verts3[surface->vertInfo.vertCount[3]]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                        1578,
                        0,
                        "%s",
                        "vert3Out == verts3 + surface->vertInfo.vertCount[3]") )
        {
            __debugbreak();
        }
        size = 2
                 * (7 * surface->vertInfo.vertCount[3]
                    + 5 * surface->vertInfo.vertCount[2]
                    + 3 * surface->vertInfo.vertCount[1]
                    + surface->vertInfo.vertCount[0]);
        if ( size )
            v8 = (unsigned __int16 *)Alloc(size);
        else
            v8 = 0;
        vertsBlendOut = v8;
        model->memUsage += size;
        surface->vertInfo.vertsBlend = vertsBlendOut;
        if ( surface->vertInfo.vertCount[0] )
        {
            vert0Out = verts0;
            j = 0;
            while ( j < surface->vertInfo.vertCount[0] )
            {
                *vertsBlendOut++ = vert0Out->boneOffset;
                ++j;
                ++vert0Out;
            }
        }
        if ( surface->vertInfo.vertCount[1] )
        {
            vert1Out = verts1;
            j = 0;
            while ( j < surface->vertInfo.vertCount[1] )
            {
                *vertsBlendOut++ = vert1Out->vert0.boneOffset;
                *vertsBlendOut++ = vert1Out->blend[0].boneOffset;
                *vertsBlendOut++ = vert1Out->blend[0].boneWeight;
                ++j;
                ++vert1Out;
            }
        }
        if ( surface->vertInfo.vertCount[2] )
        {
            vert2Out = verts2;
            j = 0;
            while ( j < surface->vertInfo.vertCount[2] )
            {
                *vertsBlendOut++ = vert2Out->vert0.boneOffset;
                *vertsBlendOut++ = vert2Out->blend[0].boneOffset;
                *vertsBlendOut++ = vert2Out->blend[0].boneWeight;
                *vertsBlendOut++ = vert2Out->blend[1].boneOffset;
                *vertsBlendOut++ = vert2Out->blend[1].boneWeight;
                ++j;
                ++vert2Out;
            }
        }
        if ( surface->vertInfo.vertCount[3] )
        {
            vert3Out = verts3;
            j = 0;
            while ( j < surface->vertInfo.vertCount[3] )
            {
                *vertsBlendOut++ = vert3Out->vert0.boneOffset;
                *vertsBlendOut++ = vert3Out->blend[0].boneOffset;
                *vertsBlendOut++ = vert3Out->blend[0].boneWeight;
                *vertsBlendOut++ = vert3Out->blend[1].boneOffset;
                *vertsBlendOut++ = vert3Out->blend[1].boneWeight;
                *vertsBlendOut++ = vert3Out->blend[2].boneOffset;
                *vertsBlendOut++ = vert3Out->blend[2].boneWeight;
                ++j;
                ++vert3Out;
            }
        }
        if ( (char *)vertsBlendOut - (char *)surface->vertInfo.vertsBlend != size
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                        1665,
                        0,
                        "%s",
                        "(byte *)vertsBlendOut - (byte *)(surface->vertInfo.vertsBlend) == size") )
        {
            __debugbreak();
        }
    }
    if ( ((surface->flags & 0x80) != 0) != (surface->vertListCount == 0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                    1668,
                    0,
                    "%s",
                    "(0 != (surface->flags & XSURFACE_FLAG_DEFORMED)) == (surface->vertListCount == 0)") )
    {
        __debugbreak();
    }
    for ( vertListIter = 0; vertListIter != surface->vertListCount; ++vertListIter )
        XModelReadSurface_BuildCollisionTree(surface, vertListIter, Alloc);
    Hunk_FreeTempMemory((char *)surfVerts);
}
#endif

// aislop
void XModelReadSurface(
    XModel *model,
    unsigned char **pos,
    void *(*Alloc)(int),
    XSurface *surface)
{
    unsigned int i, j;

    memset(surface->vertInfo.vertCount, 0, sizeof(surface->vertInfo.vertCount));

    surface->flags = 0;
    surface->tileMode = *(*pos)++;
    *pos += 2; /* unused */

    surface->vertCount = *(uint16_t *)*pos; *pos += 2;
    surface->triCount = *(uint16_t *)*pos; *pos += 2;

    iassert(surface->triCount > 0);

    /* ------------------------------------------------------------
       Read rigid vertex lists
       ------------------------------------------------------------ */
    XRigidVertList rigidLists[161];
    unsigned int vertListCount = 0;
    unsigned int rigidVertCount = 0;

    for (;;)
    {
        iassert(vertListCount < ARRAY_COUNT(rigidLists));

        XRigidVertList *list = &rigidLists[vertListCount];

        list->vertCount = *(uint16_t *)*pos; *pos += 2;
        if (!list->vertCount)
            break;

        uint16_t boneIndex = *(uint16_t *)*pos; *pos += 2;
        list->boneOffset = boneIndex << 6;

        rigidVertCount += list->vertCount;
        vertListCount++;
    }

    bool deformed = (rigidVertCount != surface->vertCount);
    if (deformed)
    {
        surface->flags |= XSURFACE_FLAG_DEFORMED | XSURFACE_FLAG_SKINNED;
        vertListCount = 0;
    }

    int numBlends = 0;

    if (vertListCount == 1)
    {
        int bone = rigidLists[0].boneOffset >> 6;
        surface->partBits[bone >> 5] |= 0x80000000 >> (bone & 31);
    }
    else
    {
        surface->flags |= XSURFACE_FLAG_SKINNED;
        numBlends = *(uint16_t *)*pos; *pos += 2;
    }

    /* ------------------------------------------------------------
       Load packed vertex stream
       ------------------------------------------------------------ */
    int surfSize = (surface->vertCount << 6) + 4 * numBlends;
    XVertexBuffer *surfVerts = (XVertexBuffer *)Hunk_AllocateTempMemory(surfSize, "XModelReadSurface");

    XVertexInfo_s *v = &surfVerts->v;

    for (i = 0; i < surface->vertCount; i++, v++)
    {
        v->normal[0] = *(float *)*pos; *pos += 4;
        v->normal[1] = *(float *)*pos; *pos += 4;
        v->normal[2] = *(float *)*pos; *pos += 4;

        if (r_modelVertColor->current.enabled)
            Byte4CopyBgraToVertexColor(*pos, v->color);
        else
            *(uint32_t *)v->color = 0xFFFFFFFF;
        *pos += 4;

        v->texCoordX = *(float *)*pos; *pos += 4;
        v->texCoordY = *(float *)*pos; *pos += 4;

        v->binormal[0] = *(float *)*pos; *pos += 4;
        v->binormal[1] = *(float *)*pos; *pos += 4;
        v->binormal[2] = *(float *)*pos; *pos += 4;

        v->tangent[0] = *(float *)*pos; *pos += 4;
        v->tangent[1] = *(float *)*pos; *pos += 4;
        v->tangent[2] = *(float *)*pos; *pos += 4;

        float marker = *(float *)*pos; *pos += 4;

        if (marker == 27397.0f)
        {
            if (!surface->vertInfo.tensionData)
            {
                surface->vertInfo.tensionData =
                    (float *)Alloc(48 * surface->vertCount);
                memset(surface->vertInfo.tensionData, 0, 48 * surface->vertCount);
            }

            for (j = 0; j < 12; j++)
            {
                float val = *(float *)*pos; *pos += 4;
                surface->vertInfo.tensionData[i * 12 + j] =
                    (val == 0.0f) ? 1.0f : (1.0f / val);
            }
        }
        else if (marker != 27398.0f)
        {
            *pos -= 4;
        }

        if (vertListCount == 1)
        {
            iassert(!deformed);

            v->numWeights = 0;
            v->boneOffset = rigidLists[0].boneOffset;

            v->offset[0] = *(float *)*pos; *pos += 4;
            v->offset[1] = *(float *)*pos; *pos += 4;
            v->offset[2] = *(float *)*pos; *pos += 4;
        }
        else
        {
            v->numWeights = *(*pos)++;
            iassert(v->numWeights < 4);

            surface->vertInfo.vertCount[v->numWeights]++;

            uint16_t bone = *(uint16_t *)*pos; *pos += 2;
            v->boneOffset = bone << 6;
            surface->partBits[bone >> 5] |= 0x80000000 >> (bone & 31);

            v->offset[0] = *(float *)*pos; *pos += 4;
            v->offset[1] = *(float *)*pos; *pos += 4;
            v->offset[2] = *(float *)*pos; *pos += 4;

            for (j = 0; j < v->numWeights; j++)
            {
                ReadBlend(surface, surface->partBits,
                    (XBlendLoadInfo *)v, pos);
                v = (XVertexInfo_s *)((char *)v + 4);
            }
        }
    }

    /* ------------------------------------------------------------
       Triangle indices
       ------------------------------------------------------------ */
    int allocCount = (surface->triCount + 1) & ~1;
    surface->triIndices =
        (uint16_t *)Alloc(sizeof(uint16_t) * 3 * allocCount);

    for (i = 0; i < surface->triCount * 3; i++)
    {
        surface->triIndices[i] = *(uint16_t *)*pos;
        *pos += 2;

        iassert(surface->triIndices[i] < surface->vertCount);
    }

    /* ------------------------------------------------------------
       Duplicate triangle removal (rigid only)
       ------------------------------------------------------------ */
    if (vertListCount == 1)
    {
        std::set<uint64_t> triSet;
        int write = 0;

        for (i = 0; i < surface->triCount * 3; i += 3)
        {
            uint16_t a = surface->triIndices[i + 0];
            uint16_t b = surface->triIndices[i + 1];
            uint16_t c = surface->triIndices[i + 2];

            uint16_t lo = std::min(a, std::min(b, c));
            uint16_t hi = std::max(a, std::max(b, c));
            uint16_t mid = a ^ b ^ c ^ lo ^ hi;

            uint64_t key =
                ((uint64_t)lo << 32) |
                ((uint64_t)mid << 16) |
                hi;

            if (triSet.insert(key).second)
            {
                surface->triIndices[write++] = a;
                surface->triIndices[write++] = b;
                surface->triIndices[write++] = c;
            }
        }

        surface->triCount = write / 3;
    }

    /* ------------------------------------------------------------
       Final surface buffers
       ------------------------------------------------------------ */
    surface->vertListCount = vertListCount;
    surface->vertList =
        vertListCount ? (XRigidVertList *)Alloc(12 * vertListCount) : NULL;

    memcpy(surface->vertList, rigidLists, 12 * vertListCount);

    surface->verts0 =
        (GfxPackedVertex *)Alloc(32 * surface->vertCount);

    memset(surface->verts0, 0, 32 * surface->vertCount);
    model->memUsage += 32 * surface->vertCount;

    XSurfaceTransfer(surfVerts, surface->verts0,
        surface->verts0, surface->vertCount);

    for (i = 0; i < surface->vertListCount; i++)
        XModelReadSurface_BuildCollisionTree(surface, i, Alloc);

    Hunk_FreeTempMemory((char *)surfVerts);
}

void __cdecl ReadBlend(XSurface *surface, int *partBits, XBlendLoadInfo *blend, unsigned __int8 **pos)
{
    unsigned __int16 v4; // [esp+0h] [ebp-10h]
    __int16 boner; // [esp+4h] [ebp-Ch]

    boner = *(_WORD *)*pos;
    *pos += 2;
    partBits[boner >> 5] |= 0x80000000 >> (boner & 0x1F);
    blend->boneOffset = boner << 6;
    if ( boner << 6 != blend->boneOffset
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                    81,
                    0,
                    "%s",
                    "boneOffset == blend->boneOffset") )
    {
        __debugbreak();
    }
    v4 = *(_WORD *)*pos;
    *pos += 2;
    blend->boneWeight = v4;
}

void __cdecl XModelReadSurface_BuildCollisionTree(
                XSurface *surface,
                unsigned int vertListIndex,
                void *(__cdecl *Alloc)(int))
{
    unsigned __int8 *v3; // eax
    int v4; // [esp+60h] [ebp-138h]
    int v5; // [esp+64h] [ebp-134h]
    int v6; // [esp+68h] [ebp-130h]
    int v7; // [esp+6Ch] [ebp-12Ch]
    int v8; // [esp+70h] [ebp-128h]
    int v9; // [esp+74h] [ebp-124h]
    int v10; // [esp+78h] [ebp-120h]
    int v11; // [esp+7Ch] [ebp-11Ch]
    int v12; // [esp+80h] [ebp-118h]
    int v13; // [esp+84h] [ebp-114h]
    int v14; // [esp+88h] [ebp-110h]
    int v15; // [esp+8Ch] [ebp-10Ch]
    float *v16; // [esp+90h] [ebp-108h]
    float *v17; // [esp+94h] [ebp-104h]
    float *v18; // [esp+98h] [ebp-100h]
    float *v19; // [esp+9Ch] [ebp-FCh]
    GenericAabbTree *builtNode; // [esp+A0h] [ebp-F8h]
    unsigned int leafIndex; // [esp+A4h] [ebp-F4h]
    float nodeMins[3]; // [esp+A8h] [ebp-F0h] BYREF
    XSurfaceCollisionNode *outNode; // [esp+B4h] [ebp-E4h]
    unsigned int leafEnd; // [esp+B8h] [ebp-E0h]
    float nodeMaxs[3]; // [esp+BCh] [ebp-DCh] BYREF
    unsigned int allocSize; // [esp+C8h] [ebp-D0h]
    unsigned int alignedAddr; // [esp+CCh] [ebp-CCh]
    unsigned __int8 *alloced; // [esp+D0h] [ebp-C8h]
    float combinedVolume; // [esp+D4h] [ebp-C4h]
    float thisVolume; // [esp+D8h] [ebp-C0h]
    float tmp[3]; // [esp+DCh] [ebp-BCh]
    float prevVolume; // [esp+E8h] [ebp-B0h]
    bool merge; // [esp+EFh] [ebp-A9h]
    float triMins[3]; // [esp+F0h] [ebp-A8h] BYREF
    float triMaxs[3]; // [esp+FCh] [ebp-9Ch] BYREF
    XSurfaceCollisionTree *tree; // [esp+108h] [ebp-90h]
    bool generateLeafsPass; // [esp+10Fh] [ebp-89h]
    unsigned int nodeIndex; // [esp+110h] [ebp-88h]
    unsigned int triEndIndex; // [esp+114h] [ebp-84h]
    float prevMins[3]; // [esp+118h] [ebp-80h] BYREF
    unsigned int leafCount; // [esp+124h] [ebp-74h]
    GenericAabbTreeOptions options; // [esp+128h] [ebp-70h] BYREF
    unsigned int nodeCount; // [esp+150h] [ebp-48h]
    float globalMaxs[3]; // [esp+154h] [ebp-44h] BYREF
    unsigned int triIndex; // [esp+160h] [ebp-38h]
    float prevMaxs[3]; // [esp+164h] [ebp-34h] BYREF
    bool lastMergeable; // [esp+173h] [ebp-25h]
    float globalMins[3]; // [esp+174h] [ebp-24h] BYREF
    float globalDelta[3]; // [esp+180h] [ebp-18h]
    unsigned int triBeginIndex; // [esp+18Ch] [ebp-Ch]
    XRigidVertList *vertList; // [esp+190h] [ebp-8h]
    unsigned int allocedLeafCount; // [esp+194h] [ebp-4h]

    if ( (surface->flags & 0x80) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                    334,
                    0,
                    "%s",
                    "0 == (surface->flags & XSURFACE_FLAG_DEFORMED)") )
    {
        __debugbreak();
    }
    if ( vertListIndex >= surface->vertListCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                    335,
                    0,
                    "%s\n\t(vertListIndex) = %i",
                    "(vertListIndex >= 0 && vertListIndex < surface->vertListCount)",
                    vertListIndex) )
    {
        __debugbreak();
    }
    vertList = &surface->vertList[vertListIndex];
    tree = (XSurfaceCollisionTree *)Alloc(40);
    vertList->collisionTree = tree;
    if ( !surface->triCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                    340,
                    0,
                    "%s",
                    "surface->triCount > 0") )
    {
        __debugbreak();
    }
    memset(&options, 0, 12);
    options.mins = 0;
    options.maxs = 0;
    options.maintainValidBounds = 1;
    options.treeNodePool = (GenericAabbTree *)malloc(0x20000u);
    options.treeNodeLimit = 0x2000;
    options.minItemsPerLeaf = 1;
    options.maxItemsPerLeaf = 16;
    ClearBounds(globalMins, globalMaxs);
    tree->leafs = 0;
    generateLeafsPass = 0;
    allocedLeafCount = 0;
    triBeginIndex = vertList->triOffset;
    triEndIndex = triBeginIndex + vertList->triCount;
    while ( 1 )
    {
        leafCount = 0;
        ClearBounds(prevMins, prevMaxs);
        lastMergeable = 0;
        for ( triIndex = triBeginIndex; triIndex != triEndIndex; ++triIndex )
        {
            ClearBounds(triMins, triMaxs);
            AddPointToBounds(surface->verts0[surface->triIndices[3 * triIndex]].xyz, triMins, triMaxs);
            AddPointToBounds(surface->verts0[surface->triIndices[3 * triIndex + 1]].xyz, triMins, triMaxs);
            AddPointToBounds(surface->verts0[surface->triIndices[3 * triIndex + 2]].xyz, triMins, triMaxs);
            ExpandBounds(triMins, triMaxs, globalMins, globalMaxs);
            merge = 0;
            if ( lastMergeable )
            {
                prevVolume = (float)((float)(prevMaxs[0] - prevMins[0]) * (float)(prevMaxs[1] - prevMins[1]))
                                     * (float)(prevMaxs[2] - prevMins[2]);
                thisVolume = (float)((float)(triMaxs[0] - triMins[0]) * (float)(triMaxs[1] - triMins[1]))
                                     * (float)(triMaxs[2] - triMins[2]);
                ExpandBounds(triMins, triMaxs, prevMins, prevMaxs);
                tmp[0] = prevMaxs[0] - prevMins[0];
                tmp[1] = prevMaxs[1] - prevMins[1];
                tmp[2] = prevMaxs[2] - prevMins[2];
                combinedVolume = (float)((float)(prevMaxs[0] - prevMins[0]) * (float)(prevMaxs[1] - prevMins[1]))
                                             * (float)(prevMaxs[2] - prevMins[2]);
                merge = (float)(prevVolume + thisVolume) >= combinedVolume;
            }
            if ( merge )
            {
                if ( generateLeafsPass )
                {
                    if ( !leafCount
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                                    426,
                                    0,
                                    "%s",
                                    "leafCount > 0") )
                    {
                        __debugbreak();
                    }
                    if ( leafCount - 1 >= allocedLeafCount
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                                    427,
                                    0,
                                    "%s",
                                    "(leafCount - 1) < allocedLeafCount") )
                    {
                        __debugbreak();
                    }
                    v19 = options.mins[leafCount - 1];
                    *v19 = prevMins[0];
                    v19[1] = prevMins[1];
                    v19[2] = prevMins[2];
                    v18 = options.maxs[leafCount - 1];
                    *v18 = prevMaxs[0];
                    v18[1] = prevMaxs[1];
                    v18[2] = prevMaxs[2];
                    if ( tree->leafs[leafCount - 1].triangleBeginIndex >= 0x8000u
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                                    430,
                                    0,
                                    "%s\n\t(tree->leafs[leafCount - 1].triangleBeginIndex) = %i",
                                    "(tree->leafs[leafCount - 1].triangleBeginIndex < 0x8000)",
                                    tree->leafs[leafCount - 1].triangleBeginIndex) )
                    {
                        __debugbreak();
                    }
                    tree->leafs[leafCount - 1].triangleBeginIndex += 0x8000;
                }
                lastMergeable = 0;
            }
            else
            {
                if ( generateLeafsPass )
                {
                    if ( leafCount >= allocedLeafCount
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                                    439,
                                    0,
                                    "%s",
                                    "leafCount < allocedLeafCount") )
                    {
                        __debugbreak();
                    }
                    if ( triIndex >= 0x8000
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                                    440,
                                    0,
                                    "%s",
                                    "triIndex < XSURFACE_COLLISION_LEAF_TWO_TRIANGLES") )
                    {
                        __debugbreak();
                    }
                    tree->leafs[leafCount].triangleBeginIndex = triIndex;
                    if ( tree->leafs[leafCount].triangleBeginIndex != triIndex
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                                    442,
                                    0,
                                    "%s\n\t(triIndex) = %i",
                                    "(tree->leafs[leafCount].triangleBeginIndex == triIndex)",
                                    triIndex) )
                    {
                        __debugbreak();
                    }
                    v17 = options.mins[leafCount];
                    *v17 = triMins[0];
                    v17[1] = triMins[1];
                    v17[2] = triMins[2];
                    v16 = options.maxs[leafCount];
                    *v16 = triMaxs[0];
                    v16[1] = triMaxs[1];
                    v16[2] = triMaxs[2];
                }
                ++leafCount;
                lastMergeable = 1;
                prevMins[0] = triMins[0];
                prevMins[1] = triMins[1];
                prevMins[2] = triMins[2];
                prevMaxs[0] = triMaxs[0];
                prevMaxs[1] = triMaxs[1];
                prevMaxs[2] = triMaxs[2];
            }
        }
        if ( generateLeafsPass )
            break;
        generateLeafsPass = 1;
        tree->leafs = (XSurfaceCollisionLeaf *)Alloc(2 * leafCount);
        tree->leafCount = leafCount;
        options.mins = (float (*)[3])malloc(12 * leafCount);
        options.maxs = (float (*)[3])malloc(12 * leafCount);
        options.items = tree->leafs;
        options.itemCount = leafCount;
        options.itemSize = 2;
        allocedLeafCount = leafCount;
    }
    if ( leafCount != allocedLeafCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                    455,
                    0,
                    "%s",
                    "leafCount == allocedLeafCount") )
    {
        __debugbreak();
    }
    tree->trans[0] = -globalMins[0];
    tree->trans[1] = -globalMins[1];
    tree->trans[2] = -globalMins[2];
    globalDelta[0] = globalMaxs[0] - globalMins[0];
    globalDelta[1] = globalMaxs[1] - globalMins[1];
    globalDelta[2] = globalMaxs[2] - globalMins[2];
    tree->scale[0] = 65535.0 / (float)(globalMaxs[0] - globalMins[0]);
    tree->scale[1] = 65535.0 / globalDelta[1];
    tree->scale[2] = 65535.0 / globalDelta[2];
    nodeCount = BuildAabbTree(&options);
    tree->nodeCount = nodeCount;
    allocSize = 16 * nodeCount + 15;
    v3 = (unsigned __int8 *)Alloc(allocSize);
    alloced = v3;
    alignedAddr = (unsigned int)(v3 + 15) & 0xFFFFFFF0;
    tree->nodes = (XSurfaceCollisionNode *)alignedAddr;
    if ( ((int)tree->nodes & 0xF) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                    494,
                    0,
                    "%s",
                    "!(reinterpret_cast< uint32_t >( tree->nodes ) & 0x0F)") )
    {
        __debugbreak();
    }
    for ( nodeIndex = 0; nodeIndex != nodeCount; ++nodeIndex )
    {
        outNode = &tree->nodes[nodeIndex];
        builtNode = &options.treeNodePool[nodeIndex];
        leafEnd = builtNode->itemCount + builtNode->firstItem;
        ClearBounds(nodeMins, nodeMaxs);
        for ( leafIndex = builtNode->firstItem; leafIndex != leafEnd; ++leafIndex )
            ExpandBounds(options.mins[leafIndex], options.maxs[leafIndex], nodeMins, nodeMaxs);
        v15 = (__int64)((nodeMins[0] + tree->trans[0]) * tree->scale[0] - 0.5);
        if ( v15 >= 0 )
        {
            if ( v15 <= 0xFFFF )
                v14 = (__int64)((nodeMins[0] + tree->trans[0]) * tree->scale[0] - 0.5);
            else
                LOWORD(v14) = -1;
        }
        else
        {
            LOWORD(v14) = 0;
        }
        outNode->aabb.mins[0] = v14;
        v13 = (__int64)((nodeMins[1] + tree->trans[1]) * tree->scale[1] - 0.5);
        if ( v13 >= 0 )
        {
            if ( v13 <= 0xFFFF )
                v12 = (__int64)((nodeMins[1] + tree->trans[1]) * tree->scale[1] - 0.5);
            else
                LOWORD(v12) = -1;
        }
        else
        {
            LOWORD(v12) = 0;
        }
        outNode->aabb.mins[1] = v12;
        v11 = (__int64)((nodeMins[2] + tree->trans[2]) * tree->scale[2] - 0.5);
        if ( v11 >= 0 )
        {
            if ( v11 <= 0xFFFF )
                v10 = (__int64)((nodeMins[2] + tree->trans[2]) * tree->scale[2] - 0.5);
            else
                LOWORD(v10) = -1;
        }
        else
        {
            LOWORD(v10) = 0;
        }
        outNode->aabb.mins[2] = v10;
        v9 = (__int64)((nodeMaxs[0] + tree->trans[0]) * tree->scale[0] + 0.5);
        if ( v9 >= 0 )
        {
            if ( v9 <= 0xFFFF )
                v8 = (__int64)((nodeMaxs[0] + tree->trans[0]) * tree->scale[0] + 0.5);
            else
                LOWORD(v8) = -1;
        }
        else
        {
            LOWORD(v8) = 0;
        }
        outNode->aabb.maxs[0] = v8;
        v7 = (__int64)((nodeMaxs[1] + tree->trans[1]) * tree->scale[1] + 0.5);
        if ( v7 >= 0 )
        {
            if ( v7 <= 0xFFFF )
                v6 = (__int64)((nodeMaxs[1] + tree->trans[1]) * tree->scale[1] + 0.5);
            else
                LOWORD(v6) = -1;
        }
        else
        {
            LOWORD(v6) = 0;
        }
        outNode->aabb.maxs[1] = v6;
        v5 = (__int64)((nodeMaxs[2] + tree->trans[2]) * tree->scale[2] + 0.5);
        if ( v5 >= 0 )
        {
            if ( v5 <= 0xFFFF )
                v4 = (__int64)((nodeMaxs[2] + tree->trans[2]) * tree->scale[2] + 0.5);
            else
                LOWORD(v4) = -1;
        }
        else
        {
            LOWORD(v4) = 0;
        }
        outNode->aabb.maxs[2] = v4;
        if ( builtNode->childCount )
        {
            outNode->childBeginIndex = builtNode->firstChild;
            if ( outNode->childBeginIndex != builtNode->firstChild
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                            524,
                            0,
                            "%s\n\t(builtNode->firstChild) = %i",
                            "(outNode->childBeginIndex == builtNode->firstChild)",
                            builtNode->firstChild) )
            {
                __debugbreak();
            }
            outNode->childCount = builtNode->childCount;
            if ( outNode->childCount != builtNode->childCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                            526,
                            0,
                            "%s\n\t(outNode->childCount) = %i",
                            "(outNode->childCount == builtNode->childCount)",
                            outNode->childCount) )
            {
                __debugbreak();
            }
        }
        else
        {
            if ( !builtNode->itemCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                            530,
                            0,
                            "%s",
                            "builtNode->itemCount") )
            {
                __debugbreak();
            }
            outNode->childBeginIndex = builtNode->firstItem;
            if ( outNode->childBeginIndex != builtNode->firstItem
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                            532,
                            0,
                            "%s\n\t(builtNode->firstItem) = %i",
                            "(outNode->childBeginIndex == builtNode->firstItem)",
                            builtNode->firstItem) )
            {
                __debugbreak();
            }
            outNode->childCount = builtNode->itemCount + 0x8000;
            if ( outNode->childCount != builtNode->itemCount + 0x8000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface_load_obj.cpp",
                            534,
                            0,
                            "%s\n\t(builtNode->itemCount) = %i",
                            "(outNode->childCount == builtNode->itemCount + 0x8000)",
                            builtNode->itemCount) )
            {
                __debugbreak();
            }
        }
    }
    free(options.mins);
    free(options.maxs);
    free(options.treeNodePool);
}

