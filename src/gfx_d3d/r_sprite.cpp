#include "r_sprite.h"
#include <xanim/xanim.h>
#include "fxprimitives.h"
#include "r_drawsurf.h"
#include <EffectsCore/fx_sprite.h>
#include <universal/com_math_anglevectors.h>

void __cdecl R_BuildQuadStampCodeMeshVerts(
                Material *material,
                const float (*viewAxis)[3],
                const float *origin,
                const float *left,
                const float *up,
                const unsigned __int8 *rgbaColor,
                int s0,
                int t0,
                int s1,
                int t1)
{
    __int16 v10; // [esp+10h] [ebp-1A0h]
    __int16 v11; // [esp+14h] [ebp-19Ch]
    __int16 v12; // [esp+18h] [ebp-198h]
    __int16 v13; // [esp+1Ch] [ebp-194h]
    __int16 v14; // [esp+20h] [ebp-190h]
    __int16 v15; // [esp+24h] [ebp-18Ch]
    __int16 v16; // [esp+28h] [ebp-188h]
    __int16 v17; // [esp+2Ch] [ebp-184h]
    int v18; // [esp+34h] [ebp-17Ch]
    int v19; // [esp+48h] [ebp-168h]
    GfxPackedVertex *v20; // [esp+64h] [ebp-14Ch]
    int v21; // [esp+6Ch] [ebp-144h]
    int v22; // [esp+80h] [ebp-130h]
    GfxPackedVertex *v23; // [esp+9Ch] [ebp-114h]
    int v24; // [esp+A4h] [ebp-10Ch]
    int v25; // [esp+B8h] [ebp-F8h]
    GfxPackedVertex *v26; // [esp+D4h] [ebp-DCh]
    int v27; // [esp+DCh] [ebp-D4h]
    int v28; // [esp+F0h] [ebp-C0h]
    PackedUnitVec v29; // [esp+10Ch] [ebp-A4h]
    PackedUnitVec v30; // [esp+12Ch] [ebp-84h]
    float testBinormal[3]; // [esp+178h] [ebp-38h] BYREF
    GfxPackedVertex *verts; // [esp+184h] [ebp-2Ch]
    r_double_index_t *indices; // [esp+188h] [ebp-28h] BYREF
    GfxColor nativeColor; // [esp+18Ch] [ebp-24h] BYREF
    r_double_index_t index; // [esp+190h] [ebp-20h]
    float leftDown[3]; // [esp+194h] [ebp-1Ch]
    unsigned __int16 baseVertex; // [esp+1A0h] [ebp-10h] BYREF
    float leftUp[3]; // [esp+1A4h] [ebp-Ch]

    if ( R_ReserveCodeMeshVerts(4, &baseVertex) && R_ReserveCodeMeshIndices(6, &indices) )
    {
        Vec3Cross((const float *)viewAxis, left, testBinormal);
        if ( (float)((float)((float)(*up * testBinormal[0]) + (float)(up[1] * testBinormal[1]))
                             + (float)(up[2] * testBinormal[2])) <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_sprite.cpp",
                        32,
                        0,
                        "Vec3Dot( up, testBinormal ) > 0.0f\n\t%g, %g",
                        (float)((float)((float)(*up * testBinormal[0]) + (float)(up[1] * testBinormal[1]))
                                    + (float)(up[2] * testBinormal[2])),
                        0.0) )
        {
            __debugbreak();
        }
        index.value[0] = baseVertex;
        index.value[1] = baseVertex + 1;
        *indices = index;
        index.value[0] = baseVertex + 3;
        index.value[1] = baseVertex + 3;
        indices[1] = index;
        index.value[0] = baseVertex + 1;
        index.value[1] = baseVertex + 2;
        indices[2] = index;
        leftUp[0] = *left + *up;
        leftUp[1] = left[1] + up[1];
        leftUp[2] = left[2] + up[2];
        leftDown[0] = *left - *up;
        leftDown[1] = left[1] - up[1];
        leftDown[2] = left[2] - up[2];
        Byte4CopyRgbaToVertexColor(rgbaColor, (unsigned __int8 *)&nativeColor);
        v30.array[0] = (int)(float)((float)((-((*viewAxis)[0])) * 127.0) + 127.5);
        v30.array[1] = (int)(float)((float)((-((*viewAxis)[1])) * 127.0) + 127.5);
        v30.array[2] = (int)(float)((float)((-((*viewAxis)[2])) * 127.0) + 127.5);
        v30.array[3] = 63;
        v29.array[0] = (int)(float)((float)((-*(int *)left) * 127.0) + 127.5);
        v29.array[1] = (int)(float)((float)((-*((int *)left + 1)) * 127.0) + 127.5);
        v29.array[2] = (int)(float)((float)((-*((int *)left + 2)) * 127.0) + 127.5);
        v29.array[3] = 63;
        verts = R_GetCodeMeshVerts(baseVertex);
        verts->xyz[0] = *origin + leftUp[0];
        verts->xyz[1] = origin[1] + leftUp[1];
        verts->xyz[2] = origin[2] + leftUp[2];
        verts->binormalSign = 1.0f;
        verts->normal = v30;
        verts->color = nativeColor;
        if ( (int)((2 * s0) ^ 0x80000000) >> 14 < 0x3FFF )
            v28 = (int)((2 * s0) ^ 0x80000000) >> 14;
        else
            v28 = 0x3FFF;
        if ( v28 > -16384 )
            v17 = v28;
        else
            v17 = -16384;
        if ( (int)((2 * t0) ^ 0x80000000) >> 14 < 0x3FFF )
            v27 = (int)((2 * t0) ^ 0x80000000) >> 14;
        else
            v27 = 0x3FFF;
        if ( v27 > -16384 )
            v16 = v27;
        else
            v16 = -16384;
        verts->texCoord.packed = (v16 & 0x3FFF | (t0 >> 16) & 0xC000) + ((v17 & 0x3FFF | (s0 >> 16) & 0xC000) << 16);
        verts->tangent = v29;
        v26 = verts + 1;
        verts[1].xyz[0] = *origin - leftDown[0];
        v26->xyz[1] = origin[1] - leftDown[1];
        v26->xyz[2] = origin[2] - leftDown[2];
        verts[1].binormalSign = 1.0f;
        verts[1].normal = v30;
        verts[1].color = nativeColor;
        if ( (int)((2 * s1) ^ 0x80000000) >> 14 < 0x3FFF )
            v25 = (int)((2 * s1) ^ 0x80000000) >> 14;
        else
            v25 = 0x3FFF;
        if ( v25 > -16384 )
            v15 = v25;
        else
            v15 = -16384;
        if ( (int)((2 * t0) ^ 0x80000000) >> 14 < 0x3FFF )
            v24 = (int)((2 * t0) ^ 0x80000000) >> 14;
        else
            v24 = 0x3FFF;
        if ( v24 > -16384 )
            v14 = v24;
        else
            v14 = -16384;
        verts[1].texCoord.packed = (v14 & 0x3FFF | (t0 >> 16) & 0xC000) + ((v15 & 0x3FFF | (s1 >> 16) & 0xC000) << 16);
        verts[1].tangent = v29;
        v23 = verts + 2;
        verts[2].xyz[0] = *origin - leftUp[0];
        v23->xyz[1] = origin[1] - leftUp[1];
        v23->xyz[2] = origin[2] - leftUp[2];
        verts[2].binormalSign = 1.0f;
        verts[2].normal = v30;
        verts[2].color = nativeColor;
        if ( (int)((2 * s1) ^ 0x80000000) >> 14 < 0x3FFF )
            v22 = (int)((2 * s1) ^ 0x80000000) >> 14;
        else
            v22 = 0x3FFF;
        if ( v22 > -16384 )
            v13 = v22;
        else
            v13 = -16384;
        if ( (int)((2 * t1) ^ 0x80000000) >> 14 < 0x3FFF )
            v21 = (int)((2 * t1) ^ 0x80000000) >> 14;
        else
            v21 = 0x3FFF;
        if ( v21 > -16384 )
            v12 = v21;
        else
            v12 = -16384;
        verts[2].texCoord.packed = (v12 & 0x3FFF | (t1 >> 16) & 0xC000) + ((v13 & 0x3FFF | (s1 >> 16) & 0xC000) << 16);
        verts[2].tangent = v29;
        v20 = verts + 3;
        verts[3].xyz[0] = *origin + leftDown[0];
        v20->xyz[1] = origin[1] + leftDown[1];
        v20->xyz[2] = origin[2] + leftDown[2];
        verts[3].binormalSign = 1.0f;
        verts[3].normal = v30;
        verts[3].color = nativeColor;
        if ( (int)((2 * s0) ^ 0x80000000) >> 14 < 0x3FFF )
            v19 = (int)((2 * s0) ^ 0x80000000) >> 14;
        else
            v19 = 0x3FFF;
        if ( v19 > -16384 )
            v11 = v19;
        else
            v11 = -16384;
        if ( (int)((2 * t1) ^ 0x80000000) >> 14 < 0x3FFF )
            v18 = (int)((2 * t1) ^ 0x80000000) >> 14;
        else
            v18 = 0x3FFF;
        if ( v18 > -16384 )
            v10 = v18;
        else
            v10 = -16384;
        verts[3].texCoord.packed = (v10 & 0x3FFF | (t1 >> 16) & 0xC000) + ((v11 & 0x3FFF | (s0 >> 16) & 0xC000) << 16);
        verts[3].tangent = v29;
        R_AddCodeMeshDrawSurf(material, indices, 6u, 0, 0, "R_BuildQuadStampCodeMeshVerts", 0xDu);
    }
}

void __cdecl R_GenerateQuadStampCodeMeshVerts(
                Material *material,
                const float *viewAngles,
                const float *origin,
                float worldRadius,
                const unsigned __int8 *rgbaColor,
                float frameFrac)
{
    float v6; // [esp+10h] [ebp-68h]
    int t0; // [esp+1Ch] [ebp-5Ch]
    float dt; // [esp+20h] [ebp-58h]
    int frame; // [esp+24h] [ebp-54h]
    int numFrames; // [esp+2Ch] [ebp-4Ch]
    float left[3]; // [esp+34h] [ebp-44h] BYREF
    float v12; // [esp+40h] [ebp-38h]
    float viewAxis[3][3]; // [esp+44h] [ebp-34h] BYREF
    float up[3]; // [esp+68h] [ebp-10h] BYREF
    int s0; // [esp+74h] [ebp-4h]

    AnglesToAxis(viewAngles, viewAxis);
    left[0] = worldRadius * viewAxis[1][0];
    left[1] = worldRadius * viewAxis[1][1];
    left[2] = worldRadius * viewAxis[1][2];
    up[0] = worldRadius * viewAxis[2][0];
    up[1] = worldRadius * viewAxis[2][1];
    up[2] = worldRadius * viewAxis[2][2];
    numFrames = material->info.textureAtlasRowCount * material->info.textureAtlasColumnCount;
    v6 = floor((double)numFrames * frameFrac);
    frame = (int)v6;
    if ( frame > numFrames - 1 )
        frame = numFrames - 1;
    v12 = 1.0 / (float)material->info.textureAtlasColumnCount;
    *(float *)&s0 = (float)(frame % material->info.textureAtlasColumnCount) * v12;
    dt = 1.0 / (float)material->info.textureAtlasRowCount;
    *(float *)&t0 = (float)(frame / material->info.textureAtlasColumnCount) * dt;
    R_BuildQuadStampCodeMeshVerts(
        material,
        viewAxis,
        origin,
        left,
        up,
        rgbaColor,
        s0,
        t0,
        (*(float *)&s0 + v12),
        (*(float *)&t0 + dt));
}

void __cdecl R_GenerateQuadStampCodeMeshVertsArray(
                Material *material,
                renderQuad_t *quads,
                int numQuads,
                float (*viewAxis)[3])
{
    renderQuad_t *v4; // edx
    GfxPackedVertex *v5; // edx
    __int16 v6; // [esp+Ch] [ebp-2D4h]
    __int16 v7; // [esp+10h] [ebp-2D0h]
    __int16 v8; // [esp+14h] [ebp-2CCh]
    __int16 v9; // [esp+18h] [ebp-2C8h]
    __int16 v10; // [esp+1Ch] [ebp-2C4h]
    __int16 v11; // [esp+20h] [ebp-2C0h]
    __int16 v12; // [esp+24h] [ebp-2BCh]
    __int16 v13; // [esp+28h] [ebp-2B8h]
    float v14; // [esp+2Ch] [ebp-2B4h]
    int v15; // [esp+34h] [ebp-2ACh]
    int v16; // [esp+48h] [ebp-298h]
    int v17; // [esp+68h] [ebp-278h]
    int v18; // [esp+7Ch] [ebp-264h]
    int v19; // [esp+9Ch] [ebp-244h]
    int v20; // [esp+B0h] [ebp-230h]
    int v21; // [esp+D0h] [ebp-210h]
    int v22; // [esp+E4h] [ebp-1FCh]
    PackedUnitVec v23; // [esp+11Ch] [ebp-1C4h]
    PackedUnitVec v24; // [esp+13Ch] [ebp-1A4h]
    float t0; // [esp+17Ch] [ebp-164h]
    __int64 vcenter; // [esp+198h] [ebp-148h]
    __int64 vcenter_8; // [esp+1A0h] [ebp-140h]
    GfxPackedVertex *out; // [esp+1A8h] [ebp-138h]
    float c; // [esp+1ACh] [ebp-134h] BYREF
    PackedTexCoords tex1; // [esp+1B0h] [ebp-130h]
    int t1; // [esp+1B4h] [ebp-12Ch]
    float4 pos3; // [esp+1B8h] [ebp-128h]
    int s1; // [esp+1C8h] [ebp-118h]
    float4 tmp1; // [esp+1CCh] [ebp-114h]
    float angle; // [esp+1DCh] [ebp-104h]
    PackedTexCoords tex2; // [esp+1E0h] [ebp-100h]
    float4 pos0; // [esp+1E4h] [ebp-FCh]
    PackedTexCoords tex3; // [esp+1F4h] [ebp-ECh]
    float s; // [esp+1F8h] [ebp-E8h] BYREF
    float4 vup; // [esp+1FCh] [ebp-E4h]
    float4 pos1; // [esp+20Ch] [ebp-D4h]
    int s0; // [esp+21Ch] [ebp-C4h]
    float4 vleft; // [esp+220h] [ebp-C0h]
    PackedTexCoords tex0; // [esp+230h] [ebp-B0h]
    float4 pos2; // [esp+234h] [ebp-ACh]
    unsigned int value; // [esp+244h] [ebp-9Ch]
    unsigned int add2; // [esp+248h] [ebp-98h]
    float dt; // [esp+24Ch] [ebp-94h]
    unsigned int *indexptr; // [esp+250h] [ebp-90h]
    float numFrames; // [esp+254h] [ebp-8Ch]
    unsigned int add1; // [esp+258h] [ebp-88h]
    float4 viewUp; // [esp+25Ch] [ebp-84h]
    unsigned int start; // [esp+26Ch] [ebp-74h]
    GfxPackedVertex prototypeVert; // [esp+270h] [ebp-70h] BYREF
    float4 prototype; // [esp+290h] [ebp-50h]
    float v56; // [esp+2A0h] [ebp-40h]
    GfxPackedVertex *verts; // [esp+2A4h] [ebp-3Ch]
    unsigned int incadd; // [esp+2A8h] [ebp-38h]
    r_double_index_t *indices; // [esp+2ACh] [ebp-34h] BYREF
    unsigned int add0; // [esp+2B0h] [ebp-30h]
    unsigned int inc; // [esp+2B4h] [ebp-2Ch]
    float4 viewLeft; // [esp+2B8h] [ebp-28h]
    unsigned __int8 rgbaColor[4]; // [esp+2CCh] [ebp-14h] BYREF
    int i; // [esp+2D0h] [ebp-10h]
    float numFramesMax; // [esp+2D4h] [ebp-Ch]
    unsigned __int16 baseVertex; // [esp+2D8h] [ebp-8h] BYREF
    unsigned int count; // [esp+2DCh] [ebp-4h]

    rgbaColor[0] = -1;
    rgbaColor[1] = -1;
    rgbaColor[2] = -1;
    rgbaColor[3] = -1;
    if ( numQuads > 0
        && R_ReserveCodeMeshVerts(4 * numQuads, &baseVertex)
        && R_ReserveCodeMeshIndices(6 * numQuads, &indices) )
    {
        indexptr = (unsigned int *)indices;
        add0 = PackTwoShorts(0, 1u);
        add1 = PackTwoShorts(3u, 3u);
        add2 = PackTwoShorts(1u, 2u);
        incadd = PackTwoShorts(4u, 4u);
        inc = PackTwoShorts(0, 0);
        start = PackTwoShorts(baseVertex, baseVertex);
        count = numQuads;
        --indexptr;
        do
        {
            value = inc + start;
            inc += incadd;
            *++indexptr = add0 + value;
            *++indexptr = add1 + value;
            *++indexptr = add2 + value;
            --count;
        }
        while ( count );
        Byte4CopyRgbaToVertexColor(rgbaColor, (unsigned __int8 *)&prototypeVert.color);
        v24.array[0] = (int)(float)((float)((-((*viewAxis)[0])) * 127.0) + 127.5);
        v24.array[1] = (int)(float)((float)((-((*viewAxis)[1])) * 127.0) + 127.5);
        v24.array[2] = (int)(float)((float)((-((*viewAxis)[2])) * 127.0) + 127.5);
        v24.array[3] = 63;
        prototypeVert.normal = v24;
        v23.array[0] = (int)(float)((float)((-((*viewAxis)[3])) * 127.0) + 127.5);
        v23.array[1] = (int)(float)((float)((-((*viewAxis)[4])) * 127.0) + 127.5);
        v23.array[2] = (int)(float)((float)((-((*viewAxis)[5])) * 127.0) + 127.5);
        v23.array[3] = 63;
        prototypeVert.tangent = v23;
        *(_QWORD *)prototype.v = *(_QWORD *)&prototypeVert.color.packed;
        *(_QWORD *)&prototype.unitVec[2].packed = __PAIR64__(v23.packed, v24.packed);
        verts = R_GetCodeMeshVerts(baseVertex);
        if ( ((unsigned __int8)verts & 0x1F) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_sprite.cpp",
                        364,
                        0,
                        "%s",
                        "(((size_t)verts) & 31) == 0") )
        {
            __debugbreak();
        }
        v56 = 1.0 / (float)material->info.textureAtlasColumnCount;
        dt = 1.0 / (float)material->info.textureAtlasRowCount;
        numFrames = (float)(material->info.textureAtlasRowCount * material->info.textureAtlasColumnCount);
        numFramesMax = numFrames - 1.0;
        *(_QWORD *)viewLeft.v = *(_QWORD *)&(*viewAxis)[3];
        viewLeft.u[2] = LODWORD((*viewAxis)[5]);
        viewLeft.u[3] = 0;
        *(_QWORD *)viewUp.v = *(_QWORD *)&(*viewAxis)[6];
        viewUp.u[2] = LODWORD((*viewAxis)[8]);
        viewUp.u[3] = 0;
        i = 0;
        while ( i < numQuads )
        {
            angle = DEG2RAD(quads[i].rotation);
            really_fast_sincos(angle, &s, &c);
            s = s * quads[i].radius;
            c = c * quads[i].radius;
            vleft.v[0] = (float)(c * viewLeft.v[0]) + (float)(s * viewUp.v[0]);
            vleft.v[1] = (float)(c * viewLeft.v[1]) + (float)(s * viewUp.v[1]);
            vleft.v[2] = (float)(c * viewLeft.v[2]) + (float)(s * viewUp.v[2]);
            vleft.v[3] = (float)(c * viewLeft.v[3]) + (float)(s * viewUp.v[3]);
            tmp1.v[0] = s * viewLeft.v[0];
            tmp1.v[1] = s * viewLeft.v[1];
            tmp1.v[2] = s * viewLeft.v[2];
            tmp1.v[3] = s * viewLeft.v[3];
            vup.v[0] = (float)(c * viewUp.v[0]) - (float)(s * viewLeft.v[0]);
            vup.v[1] = (float)(c * viewUp.v[1]) - (float)(s * viewLeft.v[1]);
            vup.v[2] = (float)(c * viewUp.v[2]) - (float)(s * viewLeft.v[2]);
            vup.v[3] = (float)(c * viewUp.v[3]) - (float)(s * viewLeft.v[3]);
            if ( (float)(numFramesMax - (float)(quads[i].lifeFrac * numFrames)) < 0.0 )
                v14 = numFramesMax;
            else
                v14 = quads[i].lifeFrac * numFrames;
            *(float *)&s0 = (float)((int)v14 % material->info.textureAtlasColumnCount) * v56;
            *(float *)&s1 = *(float *)&s0 + v56;
            t0 = (float)((int)v14 / material->info.textureAtlasColumnCount) * dt;
            *(float *)&t1 = t0 + dt;
            v4 = &quads[i];
            vcenter = *(_QWORD *)v4->pos;
            vcenter_8 = *(_QWORD *)&v4->pos[2];
            pos0.v[0] = (float)(v4->pos[0] + vleft.v[0]) + vup.v[0];
            pos0.v[1] = (float)(*((float *)&vcenter + 1) + vleft.v[1]) + vup.v[1];
            pos0.v[2] = (float)(*(float *)&vcenter_8 + vleft.v[2]) + vup.v[2];
            pos0.v[3] = (float)(*((float *)&vcenter_8 + 1) + vleft.v[3]) + vup.v[3];
            pos1.v[0] = (float)(*(float *)&vcenter - vleft.v[0]) + vup.v[0];
            pos1.v[1] = (float)(*((float *)&vcenter + 1) - vleft.v[1]) + vup.v[1];
            pos1.v[2] = (float)(*(float *)&vcenter_8 - vleft.v[2]) + vup.v[2];
            pos1.v[3] = (float)(*((float *)&vcenter_8 + 1) - vleft.v[3]) + vup.v[3];
            pos2.v[0] = (float)(*(float *)&vcenter - vleft.v[0]) - vup.v[0];
            pos2.v[1] = (float)(*((float *)&vcenter + 1) - vleft.v[1]) - vup.v[1];
            pos2.v[2] = (float)(*(float *)&vcenter_8 - vleft.v[2]) - vup.v[2];
            pos2.v[3] = (float)(*((float *)&vcenter_8 + 1) - vleft.v[3]) - vup.v[3];
            pos3.v[0] = (float)(*(float *)&vcenter + vleft.v[0]) - vup.v[0];
            pos3.v[1] = (float)(*((float *)&vcenter + 1) + vleft.v[1]) - vup.v[1];
            pos3.v[2] = (float)(*(float *)&vcenter_8 + vleft.v[2]) - vup.v[2];
            pos3.v[3] = (float)(*((float *)&vcenter_8 + 1) + vleft.v[3]) - vup.v[3];
            if ( (int)((2 * s0) ^ 0x80000000) >> 14 < 0x3FFF )
                v22 = (int)((2 * s0) ^ 0x80000000) >> 14;
            else
                v22 = 0x3FFF;
            if ( v22 > -16384 )
                v13 = v22;
            else
                v13 = -16384;
            if ( (int)((2 * LODWORD(t0)) ^ 0x80000000) >> 14 < 0x3FFF )
                v21 = (int)((2 * LODWORD(t0)) ^ 0x80000000) >> 14;
            else
                v21 = 0x3FFF;
            if ( v21 > -16384 )
                v12 = v21;
            else
                v12 = -16384;
            tex0.packed = (v12 & 0x3FFF | (SLODWORD(t0) >> 16) & 0xC000) + ((v13 & 0x3FFF | (s0 >> 16) & 0xC000) << 16);
            if ( (int)((2 * s1) ^ 0x80000000) >> 14 < 0x3FFF )
                v20 = (int)((2 * s1) ^ 0x80000000) >> 14;
            else
                v20 = 0x3FFF;
            if ( v20 > -16384 )
                v11 = v20;
            else
                v11 = -16384;
            if ( (int)((2 * LODWORD(t0)) ^ 0x80000000) >> 14 < 0x3FFF )
                v19 = (int)((2 * LODWORD(t0)) ^ 0x80000000) >> 14;
            else
                v19 = 0x3FFF;
            if ( v19 > -16384 )
                v10 = v19;
            else
                v10 = -16384;
            tex1.packed = (v10 & 0x3FFF | (SLODWORD(t0) >> 16) & 0xC000) + ((v11 & 0x3FFF | (s1 >> 16) & 0xC000) << 16);
            if ( (int)((2 * s1) ^ 0x80000000) >> 14 < 0x3FFF )
                v18 = (int)((2 * s1) ^ 0x80000000) >> 14;
            else
                v18 = 0x3FFF;
            if ( v18 > -16384 )
                v9 = v18;
            else
                v9 = -16384;
            if ( (int)((2 * t1) ^ 0x80000000) >> 14 < 0x3FFF )
                v17 = (int)((2 * t1) ^ 0x80000000) >> 14;
            else
                v17 = 0x3FFF;
            if ( v17 > -16384 )
                v8 = v17;
            else
                v8 = -16384;
            tex2.packed = (v8 & 0x3FFF | (t1 >> 16) & 0xC000) + ((v9 & 0x3FFF | (s1 >> 16) & 0xC000) << 16);
            if ( (int)((2 * s0) ^ 0x80000000) >> 14 < 0x3FFF )
                v16 = (int)((2 * s0) ^ 0x80000000) >> 14;
            else
                v16 = 0x3FFF;
            if ( v16 > -16384 )
                v7 = v16;
            else
                v7 = -16384;
            if ( (int)((2 * t1) ^ 0x80000000) >> 14 < 0x3FFF )
                v15 = (int)((2 * t1) ^ 0x80000000) >> 14;
            else
                v15 = 0x3FFF;
            if ( v15 > -16384 )
                v6 = v15;
            else
                v6 = -16384;
            tex3.packed = (v6 & 0x3FFF | (t1 >> 16) & 0xC000) + ((v7 & 0x3FFF | (s0 >> 16) & 0xC000) << 16);
            out = verts;
            v5 = verts;
            *(_QWORD *)verts->xyz = *(_QWORD *)pos0.v;
            *(_QWORD *)&v5->xyz[2] = *(_QWORD *)&pos0.unitVec[2].packed;
            *(_QWORD *)&out->color.packed = *(_QWORD *)prototype.v;
            *(_QWORD *)&out->normal.packed = *(_QWORD *)&prototype.unitVec[2].packed;
            out->texCoord = tex0;
            *(_QWORD *)out[1].xyz = *(_QWORD *)pos1.v;
            *(_QWORD *)&out[1].xyz[2] = *(_QWORD *)&pos1.unitVec[2].packed;
            *(_QWORD *)&out[1].color.packed = *(_QWORD *)prototype.v;
            *(_QWORD *)&out[1].normal.packed = *(_QWORD *)&prototype.unitVec[2].packed;
            out[1].texCoord = tex1;
            *(_QWORD *)out[2].xyz = *(_QWORD *)pos2.v;
            *(_QWORD *)&out[2].xyz[2] = *(_QWORD *)&pos2.unitVec[2].packed;
            *(_QWORD *)&out[2].color.packed = *(_QWORD *)prototype.v;
            *(_QWORD *)&out[2].normal.packed = *(_QWORD *)&prototype.unitVec[2].packed;
            out[2].texCoord = tex2;
            *(_QWORD *)out[3].xyz = *(_QWORD *)pos3.v;
            *(_QWORD *)&out[3].xyz[2] = *(_QWORD *)&pos3.unitVec[2].packed;
            *(_QWORD *)&out[3].color.packed = *(_QWORD *)prototype.v;
            *(_QWORD *)&out[3].normal.packed = *(_QWORD *)&prototype.unitVec[2].packed;
            out[3].texCoord = tex3;
            ++i;
            verts += 4;
        }
        R_AddCodeMeshDrawSurf(material, indices, 6 * numQuads, 0, 0, "R_BuildQuadStampCodeMeshVertsArray", 0xDu);
    }
}

// KISAKTODO: investigate this claim
void __cdecl really_fast_sincos(float in, float *s, float *c)
{
    double v3; // st7
    float v4; // xmm0_4
    double v5; // st7
    float v6; // xmm0_4
    double x; // [esp+20h] [ebp-10h]
    double xa; // [esp+20h] [ebp-10h]

    x = in * 0.1591549386630057 + 100000.0;
    v3 = fast_round(x);
    v4 = ((abs(x - v3 - 0.5)) * 16.0 - 8.0) * (x - v3 - 0.5);
    *s = v4;
    xa = x + 0.25;
    v5 = fast_round(xa);
    v6 = ((abs(xa - v5 - 0.5)) * 16.0 - 8.0) * (xa - v5 - 0.5);
    *c = v6;
}

double __cdecl fast_round(double x)
{
    return floor(x);
}

unsigned int __cdecl PackTwoShorts(unsigned int first, unsigned int second)
{
    return first | (second << 16);
}

