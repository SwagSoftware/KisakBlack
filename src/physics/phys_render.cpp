#include "phys_render.h"
#include <gfx_d3d/r_debug_alloc.h>
#include <qcommon/cm_load.h>
#include <bgame/bg_slidemove.h>
#include <gfx_d3d/rb_backend.h>
#include <gfx_d3d/rb_debug.h>
#include <gfx_d3d/rb_shade.h>
#include <gfx_d3d/r_debug.h>
#include <xanim/xmodel_utils.h>
#include <cgame/cg_drawtools.h>
#include <game/g_debug.h>
#include <tl/tl_system.h>
#include "physics_system.h"
#include "phys_convex_hull.h"

float alpha = 1.0f;
float alpha_0 = 1.0f;
float alpha_1 = 1.0f;
float line_length = 9.0f;
float l2 = 25.0f;
float l1 = 15.0f;
float box_size = 1.0f;
float scale_4 = 20.0f;
float rad = 3.0f;

debug_brush_info_t *debug_brush_info;
debug_patch_info_t *debug_patch_info;

int debugCollisionAabbTreeCount;

// This version takes a Ref (&)
void __cdecl make_rotate(phys_mat44 &m, const phys_vec3 *u, float ca, float sa)
{
    float yy;
    float xsa;
    float xy;
    float zsa;
    float zz;
    float ysa;
    float xz;
    float yz;

    xy = (u->x * u->y) * (1.0f - ca);
    xz = (u->x * u->z) * (1.0f - ca);
    yy = (u->y * u->y) * (1.0f - ca);
    yz = (u->y * u->z) * (1.0f - ca);
    zz = (u->z * u->z) * (1.0f - ca);
    xsa = u->x * sa;
    ysa = u->y * sa;
    zsa = u->z * sa;

    m.x.x = (u->x * u->x) * (1.0f - ca) + ca;
    m.y.x = xy - zsa;
    m.z.x = xz + ysa;
    m.w.x = 0.0f;

    m.x[1] = xy + zsa;
    m.y[1] = yy + ca;
    m.z[1] = yz - xsa;
    m.w[1] = 0.0f;

    m.x[2] = xz - ysa;
    m.y[2] = yz + xsa;
    m.z[2] = zz + ca;
    m.w[2] = 0.0f;

    m.fix_w_column();
}

void    make_rotate(
                phys_mat44 *mat,
                const phys_vec3 *v,
                float theta_factor,
                float max_rotation_radians)
{
    phys_vec3 v8; // [esp+18h] [ebp-3Ch] BYREF
    float len; // [esp+44h] [ebp-10h]

    len = Vec3Length(&v->x);
    if (len >= 0.0000099999997)
    {
        v8.x = (1.0 / len) * v->x;
        v8.y = (1.0 / len) * v->y;
        v8.z = (1.0 / len) * v->z;

        len = len * theta_factor;
        if (len > max_rotation_radians)
            len = max_rotation_radians;

        make_rotate(*mat, &v8, cos(len), sin(len));
    }
    else
    {
        *mat = PHYS_IDENTITY_MATRIX;
        //phys_mat44::operator=(mat, &PHYS_IDENTITY_MATRIX_34);
    }
}

void __cdecl DebugPatchesAndBrushesProlog()
{
    Sys_EnterCriticalSection(CRTISECT_DEBUG_BRUSHES_AND_PATCHES);
}

void __cdecl DebugPatchesAndBrushesEpilog()
{
    Sys_LeaveCriticalSection(CRTISECT_DEBUG_BRUSHES_AND_PATCHES);
}

void __cdecl clear_debug_brushes_and_patches()
{
    debugCollisionAabbTreeCount = 0;
    if ( !debug_brush_info )
    {
        R_DebugAlloc((void **)&debug_brush_info, 490012, "(debug_brush_info)");
        if ( !debug_brush_info )
        {
            Com_Printf(0, "Could not allocate memory for debug_brush_info\n");
            return;
        }
        memset(debug_brush_info, 0, sizeof(debug_brush_info_t));
    }
    debug_brush_info->num_brushes = 0;
    debug_brush_info->num_windings = 0;
    debug_brush_info->num_indices = 0;
    if ( !debug_patch_info )
    {
        R_DebugAlloc((void **)&debug_patch_info, 460008, "(debug_patch_info)");
        if ( !debug_patch_info )
        {
            Com_Printf(0, "Could not allocate memory for debug_patch_info\n");
            return;
        }
        memset(debug_patch_info, 0, sizeof(debug_patch_info_t));
    }
    debug_patch_info->num_patches = 0;
    debug_patch_info->num_indices = 0;
}

void __cdecl add_debug_brush(const cbrush_t *brush, const phys_mat44 *mat)
{
    int savedregs; // [esp+6224h] [ebp+0h] BYREF

    if (debug_brush_info)
    {
        //debug_brush_info_t::add_brush(debug_brush_info, (int)&savedregs, brush, mat);
        debug_brush_info->add_brush(brush, mat);

    }
}

// aislop supervised
void debug_brush_info_t::add_brush(
                const cbrush_t *brush,
                const phys_mat44 *mat)
{
    if ( this->num_brushes >= 5000 )
    {
        Com_Printf(0, "Debug brush limit exceeded, limit=%d\n", 5000);
        return;
    }

    if ( mat )
    {
        Com_Printf(0, "Debug rendering for xformed brushes is not supported at the moment\n");
        return;
    }

    phys_vec3 brushMins, brushMaxs;
    Phys_Vec3ToNitrousVec(brush->mins, &brushMins);
    Phys_Vec3ToNitrousVec(brush->maxs, &brushMaxs);

    phys_static_array<plane_lt, 512> planes;

    for (unsigned int i = 0; i < brush->numsides; ++i)
    {
        const cplane_s *plane = brush->sides[i].plane;
        phys_vec3 n;
        Phys_Vec3ToNitrousVec(plane->normal, &n);

        plane_lt *p = planes.add(0, "phys array add overflow.");
        p->n = n;
        p->d = plane->dist;
    }

    { plane_lt *p = planes.add(0, "phys array add overflow."); p->n = { -1,  0,  0 }; p->d = -brushMins.x; }
    { plane_lt *p = planes.add(0, "phys array add overflow."); p->n = { 0, -1,  0 }; p->d = -brushMins.y; }
    { plane_lt *p = planes.add(0, "phys array add overflow."); p->n = { 0,  0, -1 }; p->d = -brushMins.z; }
    { plane_lt *p = planes.add(0, "phys array add overflow."); p->n = { 1,  0,  0 }; p->d = brushMaxs.x; }
    { plane_lt *p = planes.add(0, "phys array add overflow."); p->n = { 0,  1,  0 }; p->d = brushMaxs.y; }
    { plane_lt *p = planes.add(0, "phys array add overflow."); p->n = { 0,  0,  1 }; p->d = brushMaxs.z; }

    iassert(planes.get_count() < 65536);

    uint16_t nplanes_u16 = (uint16_t)planes.get_count();

    // Allocate a new debug_brush_t entry
    debug_brush_t *db = &brushes[num_brushes++];
    db->brush = brush;
    db->first_winding_index = num_windings;
    db->first_vertex_index = num_indices;
    db->num_windings = nplanes_u16;

    if (num_windings + nplanes_u16 >= 50000)
    {
        Com_Printf(0, "Debug brush windings limit exceeded, limit=%d\n", 50000);
        this->num_brushes--;
        return;
    }

    int saved_num_windings = num_windings;

    for (unsigned int j = 0; j < nplanes_u16; ++j)
    {
        phys_static_array<phys_vec3, 512> winding;
        calc_winding(planes, j, &winding);

        uint8_t winding_count = (uint8_t)winding.m_alloc_count;
        iassert(winding.m_alloc_count < 65536); // "winding.get_count() < 65536"

        windings[num_windings++] = winding_count;

        // Check index budget
        if (num_indices + winding_count >= 360000)
        {
            Com_Printf(0, "Debug brush windings limit exceeded, limit=%d\n", 360000);
            --num_brushes;
            num_windings = saved_num_windings;
            return;
        }

        for (int n = 0; n < winding_count; ++n)
        {
            float v[3];
            Phys_NitrousVecToVec3(winding[n], v);

            unsigned int ind = find_index_in_brush(brush, v);
            iassert(ind < 256); // "ind < 256"

            indices[num_indices++] = (uint8_t)ind;
        }
    }

    db->num_indices = (uint16_t)(num_indices - db->first_vertex_index);

}

__int16 __thiscall debug_brush_info_t::find_index_in_brush(const cbrush_t *brush, const float *v)
{
    float d; // [esp+1Ch] [ebp-10h]
    unsigned int i; // [esp+20h] [ebp-Ch]
    __int16 best_i; // [esp+24h] [ebp-8h]
    float best_d; // [esp+28h] [ebp-4h]

    best_d = Vec3DistanceSq(v, (const float *)brush->verts);
    best_i = 0;
    if ( brush->numverts >= 0x10000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_render.cpp",
                    98,
                    0,
                    "%s",
                    "brush->numverts < 65536") )
    {
        __debugbreak();
    }
    for ( i = 1; i < brush->numverts; ++i )
    {
        d = Vec3DistanceSq(v, brush->verts[i]);
        if ( best_d > d )
        {
            best_d = d;
            best_i = i;
        }
    }
    return best_i;
}

void __cdecl add_debug_patch(const CollisionAabbTree *tree)
{
    if (debug_patch_info)
    {
        //debug_patch_info_t::add_patch(debug_patch_info, tree);
        debug_patch_info->add_patch(tree);
    }
}

void __thiscall debug_patch_info_t::add_patch(const CollisionAabbTree *tree)
{
    float v[3]; // [esp+48h] [ebp-20h] BYREF
    unsigned int ind; // [esp+54h] [ebp-14h]
    int ind_i; // [esp+58h] [ebp-10h]
    unsigned int ntris; // [esp+5Ch] [ebp-Ch]
    debug_patch_t *debug_patch; // [esp+60h] [ebp-8h]
    chull_t *chull; // [esp+64h] [ebp-4h]

    if ( this->num_patches == 5000 )
    {
        Com_Printf(0, "Debug patches limit exceeded, limit=%d\n", 5000);
    }
    else
    {
        chull = get_partition_chull(tree);
        if ( chull && chull->ninds > 0 )
        {
            if ( chull->ninds % 3
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_render.cpp",
                            258,
                            0,
                            "%s",
                            "chull->ninds % 3 == 0") )
            {
                __debugbreak();
            }
            ntris = chull->ninds / 3;
            debug_patch = &this->patches[this->num_patches++];
            debug_patch->tree = tree;
            debug_patch->first_vertex_index = this->num_indices;
            debug_patch->num_indices = 3 * ntris;
            if ( this->num_indices + 3 * ntris < 0x186A0 )
            {
                for ( ind_i = 0; ind_i < chull->ninds; ++ind_i )
                {
                    Phys_NitrousVecToVec3(&chull->verts[chull->inds[ind_i]], v);
                    //ind = debug_patch_info_t::find_index_in_clipmap(this, tree, v);
                    ind = debug_patch_info_t::find_index_in_clipmap(tree, v);
                    this->indices[this->num_indices++] = ind;
                }
            }
            else
            {
                Com_Printf(0, "Debug patches indices limit exceeded, limit=%d\n", 100000);
                --this->num_patches;
            }
        }
    }
}

unsigned int __thiscall debug_patch_info_t::find_index_in_clipmap(
                const CollisionAabbTree *tree,
                const float *v)
{
    int ti; // [esp+34h] [ebp-14h]
    unsigned int best_i; // [esp+38h] [ebp-10h]
    float d; // [esp+3Ch] [ebp-Ch]
    float da; // [esp+3Ch] [ebp-Ch]
    float db; // [esp+3Ch] [ebp-Ch]
    float best_d; // [esp+40h] [ebp-8h]
    const CollisionPartition *partition; // [esp+44h] [ebp-4h]

    partition = &cm.partitions[tree->u.firstChildIndex];
    best_d = Vec3DistanceSq(v, cm.verts[cm.triIndices[3 * partition->firstTri]]);
    best_i = cm.triIndices[3 * partition->firstTri];
    for ( ti = partition->firstTri; ti < partition->firstTri + partition->triCount; ++ti )
    {
        d = Vec3DistanceSq(v, cm.verts[cm.triIndices[3 * ti]]);
        if ( best_d > d )
        {
            best_d = d;
            best_i = cm.triIndices[3 * ti];
        }
        da = Vec3DistanceSq(v, cm.verts[cm.triIndices[3 * ti + 1]]);
        if ( best_d > da )
        {
            best_d = da;
            best_i = cm.triIndices[3 * ti + 1];
        }
        db = Vec3DistanceSq(v, cm.verts[cm.triIndices[3 * ti + 2]]);
        if ( best_d > db )
        {
            best_d = db;
            best_i = cm.triIndices[3 * ti + 2];
        }
    }
    return best_i;
}

void __cdecl render_debug_brushes()
{
    float *v0; // [esp+4h] [ebp-B8h]
    float *v1; // [esp+Ch] [ebp-B0h]
    int v2; // [esp+10h] [ebp-ACh]
    unsigned __int16 i; // [esp+14h] [ebp-A8h]
    int ii; // [esp+1Ch] [ebp-A0h]
    unsigned __int16 ibase; // [esp+20h] [ebp-9Ch]
    unsigned int num_indices; // [esp+24h] [ebp-98h]
    int wi; // [esp+28h] [ebp-94h]
    int first_winding_index; // [esp+2Ch] [ebp-90h]
    debug_brush_t *debug_brush; // [esp+30h] [ebp-8Ch]
    const cbrush_t *brush; // [esp+34h] [ebp-88h]
    int first_vertex_index; // [esp+38h] [ebp-84h]
    int bi; // [esp+3Ch] [ebp-80h]
    float lit_color[4]; // [esp+40h] [ebp-7Ch] BYREF
    GfxColor gfxColor; // [esp+50h] [ebp-6Ch] BYREF
    float verts[3][3]; // [esp+54h] [ebp-68h] BYREF
    materialCommands_t *tess; // [esp+78h] [ebp-44h]
    float player_clip_color[4]; // [esp+7Ch] [ebp-40h]
    float color[4]; // [esp+8Ch] [ebp-30h] BYREF
    float default_color[4]; // [esp+9Ch] [ebp-20h]
    float mantle_color[4]; // [esp+ACh] [ebp-10h]

    default_color[0] = 0.0f;
    default_color[1] = 0.75f;
    default_color[2] = 0.75f;
    default_color[3] = 1.0f;
    player_clip_color[0] = 0.75f;
    player_clip_color[1] = 0.75f;
    player_clip_color[2] = 0.0f;
    player_clip_color[3] = 1.0f;
    mantle_color[0] = 0.5f;
    mantle_color[1] = 0.0f;
    mantle_color[2] = 0.0f;
    mantle_color[3] = 1.0f;
    DebugPatchesAndBrushesProlog();
    if ( debug_brush_info->num_brushes )
    {
        RB_BeginCollisionPolygons(1, 0);
        tess = RB_GetCmd();
        for ( bi = 0; bi < debug_brush_info->num_brushes; ++bi )
        {
            debug_brush = &debug_brush_info->brushes[bi];
            brush = debug_brush->brush;
            first_vertex_index = debug_brush->first_vertex_index;
            first_winding_index = debug_brush->first_winding_index;
            for ( wi = first_winding_index; wi < first_winding_index + debug_brush->num_windings; ++wi )
            {
                num_indices = debug_brush_info->windings[wi];
                if ( num_indices >= 3 )
                {
                    RB_CheckTessOverflow(num_indices, 3 * (num_indices - 2));
                    v2 = (int)brush->verts[debug_brush_info->indices[first_vertex_index]];
                    *(_QWORD *)&verts[0][0] = *(_QWORD *)v2;
                    verts[0][2] = *(float *)(v2 + 8);
                    v1 = brush->verts[debug_brush_info->indices[first_vertex_index + 1]];
                    verts[1][0] = *v1;
                    verts[1][1] = v1[1];
                    verts[1][2] = v1[2];
                    v0 = brush->verts[debug_brush_info->indices[first_vertex_index + 2]];
                    verts[2][0] = *v0;
                    verts[2][1] = v0[1];
                    verts[2][2] = v0[2];
                    if ( (brush->contents & 0x10000) != 0 )
                    {
                        color[0] = player_clip_color[0];
                        color[1] = player_clip_color[1];
                        color[2] = player_clip_color[2];
                        color[3] = player_clip_color[3];
                    }
                    else if ((brush->contents & 0x1000000) != 0)
                    {
                        color[0] = mantle_color[0];
                        color[1] = mantle_color[1];
                        color[2] = mantle_color[2];
                        color[3] = mantle_color[3];
                    }
                    else
                    {
                        color[0] = default_color[0];
                        color[1] = default_color[1];
                        color[2] = default_color[2];
                        color[3] = default_color[3];
                    }
                    RB_ApplySunLight(verts, color, lit_color);
                    R_ConvertColorToBytes(lit_color, (unsigned __int8 *)&gfxColor);
                    ibase = tess->vertexCount;
                    for ( ii = first_vertex_index; ii < (int)(num_indices + first_vertex_index); ++ii )
                    {
                        RB_SetPolyVert(brush->verts[debug_brush_info->indices[ii]], gfxColor, tess->vertexCount);
                        ++tess->vertexCount;
                    }
                    for ( i = 1; i < (int)(num_indices - 1); ++i )
                    {
                        tess->indices[tess->indexCount] = ibase;
                        tess->indices[tess->indexCount + 1] = i + ibase;
                        tess->indices[tess->indexCount + 2] = ibase + i + 1;
                        tess->indexCount += 3;
                    }
                    first_vertex_index += num_indices;
                }
            }
        }
        RB_EndCollisionPolygons();
        DebugPatchesAndBrushesEpilog();
    }
    else
    {
        DebugPatchesAndBrushesEpilog();
    }
}

void __cdecl RB_CheckTessOverflow(int vertexCount, int indexCount)
{
    if ( vertexCount > 5450
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\physics\\../gfx_d3d/rb_backend.h",
                    175,
                    0,
                    "%s\n\t(vertexCount) = %i",
                    "(vertexCount <= 5450)",
                    vertexCount) )
    {
        __debugbreak();
    }
    if ( indexCount > 0x100000
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\physics\\../gfx_d3d/rb_backend.h",
                    176,
                    0,
                    "%s\n\t(indexCount) = %i",
                    "(indexCount <= ((2 * 1024 * 1024) / 2))",
                    indexCount) )
    {
        __debugbreak();
    }
    if ( vertexCount + tess.vertexCount > 5450 || indexCount + tess.indexCount > 0x100000 )
        RB_TessOverflow();
}

unsigned __int8 flip;
void __cdecl render_debug_patches_full()
{
    float *v0; // [esp+0h] [ebp-78h]
    float *v1; // [esp+4h] [ebp-74h]
    float *v2; // [esp+8h] [ebp-70h]
    float *v3; // [esp+Ch] [ebp-6Ch]
    float *v4; // [esp+10h] [ebp-68h]
    unsigned int vi; // [esp+14h] [ebp-64h]
    debug_patch_t *debug_patch; // [esp+38h] [ebp-40h]
    unsigned int i; // [esp+3Ch] [ebp-3Ch]
    const CollisionAabbTree *tree; // [esp+40h] [ebp-38h]
    float polygonVerts[3][3]; // [esp+44h] [ebp-34h] BYREF
    float faceColor[4]; // [esp+68h] [ebp-10h] BYREF

    faceColor[0] = 0.1f;
    faceColor[1] = 0.85f;
    faceColor[2] = 0.4f;
    faceColor[3] = 1.0f;
    DebugPatchesAndBrushesProlog();
    if ( debug_patch_info && debug_patch_info->num_patches )
    {
        RB_BeginCollisionPolygons(1, 0);
        for ( i = 0; i < debug_patch_info->num_patches; ++i )
        {
            debug_patch = &debug_patch_info->patches[i];
            tree = debug_patch->tree;
            faceColor[0] = (float)(0.5 * (float)((float)((((unsigned int)debug_patch->tree >> 4) & 0xF00) >> 4) / 256.0))
                                     + 0.5;
            faceColor[1] = (float)(0.5 * (float)((float)(((unsigned int)tree >> 4) & 0xF0) / 256.0)) + 0.5;
            faceColor[2] = (float)(0.5 * (float)((float)(unsigned __int8)(16 * (((unsigned int)tree >> 4) & 0xF)) / 256.0))
                                     + 0.5;
            if ( tree->u.firstChildIndex == g_debug_partition )
            {
                faceColor[0] = colorWhite[0];
                faceColor[1] = 1.0;
                faceColor[2] = 1.0;
                faceColor[3] = 1.0;
            }
            for ( vi = debug_patch->first_vertex_index; vi < debug_patch->first_vertex_index + debug_patch->num_indices; vi += 3 )
            {
                v4 = cm.verts[debug_patch_info->indices[vi]];
                *(_QWORD *)&polygonVerts[0][0] = *(_QWORD *)v4;
                polygonVerts[0][2] = v4[2];
                v2 = polygonVerts[(flip != 0) + 1];
                v3 = cm.verts[debug_patch_info->indices[vi + 1]];
                *v2 = *v3;
                v2[1] = v3[1];
                v2[2] = v3[2];
                v0 = polygonVerts[2 - (flip != 0)];
                v1 = cm.verts[debug_patch_info->indices[vi + 2]];
                *v0 = *v1;
                v0[1] = v1[1];
                v0[2] = v1[2];
                RB_DrawCollisionPolygon(3, polygonVerts, faceColor, 1, g_debug_partition);
            }
        }
        RB_EndCollisionPolygons();
        DebugPatchesAndBrushesEpilog();
    }
    else
    {
        DebugPatchesAndBrushesEpilog();
    }
}

void __cdecl render_debug_brushes_and_patches()
{
    if ( debug_brush_info )
    {
        render_debug_patches_full();
        render_debug_brushes();
    }
}

void    render_brush(
                const cbrush_t *brush,
                const phys_mat44 *xform,
                const float *color,
                bool bLinesOnly,
                const float *lightPos,
                bool persistent,
                bool ztest,
                const float *edgeColor)
{
    const phys_vec3 *v9; // eax
    float *v10; // [esp-14h] [ebp-580h]
    float v11; // [esp-Ch] [ebp-578h]
    float v12[2]; // [esp-8h] [ebp-574h] BYREF
    float normal[3]; // [esp+4h] [ebp-568h] BYREF
    float p02[4]; // [esp+10h] [ebp-55Ch] BYREF
    float p01[4]; // [esp+20h] [ebp-54Ch] BYREF
    float lightDir[3]; // [esp+30h] [ebp-53Ch]
    float center[4]; // [esp+3Ch] [ebp-530h]
    float *v18; // [esp+4Ch] [ebp-520h]
    float *v19; // [esp+50h] [ebp-51Ch]
    float verts[3][3]; // [esp+54h] [ebp-518h] BYREF
    float *v21; // [esp+78h] [ebp-4F4h]
    int ind_i; // [esp+7Ch] [ebp-4F0h]
    phys_vec3 v23; // [esp+80h] [ebp-4ECh] BYREF
    float vert_list[100][3]; // [esp+98h] [ebp-4D4h] BYREF
    int vi; // [esp+548h] [ebp-24h]
    chull_t *chull; // [esp+54Ch] [ebp-20h]
    float clr[4]; // [esp+550h] [ebp-1Ch] BYREF
    //_UNKNOWN *v28[2]; // [esp+560h] [ebp-Ch] BYREF
    //const float *colora; // [esp+56Ch] [ebp+0h]
    //
    //*(float *)v28 = a1;
    //v28[1] = colora;
    clr[0] = 0.0f;
    clr[1] = 1.0f;
    clr[2] = 1.0f;
    clr[3] = 0.25f;
    if (color)
    {
        clr[0] = *color;
        clr[1] = color[1];
        clr[2] = color[2];
        clr[3] = color[3];
    }

    if (!xform)
        xform = &PHYS_IDENTITY_MATRIX;

    chull = get_brush_chull(brush);

    if (chull)
    {
        iassert(chull->nverts < 100);

        for (vi = 0; vi < chull->nverts; ++vi)
        {
            v10 = vert_list[vi];
            v9 = phys_full_multiply(&v23, xform, &chull->verts[vi]);
            Phys_NitrousVecToVec3(v9, v10);
        }
        for (ind_i = 0; ; ind_i += 3)
        {
            if (ind_i >= chull->ninds)
                return;
            v21 = vert_list[chull->inds[ind_i]];
            verts[0][0] = v21[0];
            verts[0][1] = v21[1];
            verts[0][2] = v21[2];
            v19 = verts[1];
            // lwss add
            if (chull->inds[ind_i + 1] > 99)
                return;
            // lwss end
            v18 = vert_list[chull->inds[ind_i + 1]];
            verts[1][0] = *v18;
            verts[1][1] = v18[1];
            verts[1][2] = v18[2];
            //LODWORD(center[3]) = verts[2];
            //LODWORD(center[2]) = vert_list[chull->inds[ind_i + 2]];
            verts[2][0] = vert_list[chull->inds[ind_i + 2]][0];
            verts[2][1] = vert_list[chull->inds[ind_i + 2]][1];
            verts[2][2] = vert_list[chull->inds[ind_i + 2]][2];
            //verts[2][0] = *(float *)LODWORD(center[2]);
            //verts[2][1] = *(float *)(LODWORD(center[2]) + 4);
            //verts[2][2] = *(float *)(LODWORD(center[2]) + 8);
            if (bLinesOnly)
            {
                render_line(verts[0], verts[1], colorLtGreen, 0, ztest);
                render_line(verts[0], verts[2], colorLtGreen, 0, ztest);
                render_line(verts[1], verts[2], colorLtGreen, 0, ztest);
                continue;
            }
            if (lightPos)
            {
                //LODWORD(center[1]) = verts[1];
                //LODWORD(lightDir[0]) = verts[2];
                lightDir[1] = (float)(verts[0][0] + verts[1][0]) + verts[2][0];
                lightDir[2] = (float)(verts[0][1] + verts[1][1]) + verts[2][1];
                center[0] = (float)(verts[0][2] + verts[1][2]) + verts[2][2];
                lightDir[1] = 0.33333334 * lightDir[1];
                lightDir[2] = 0.33333334 * lightDir[2];
                center[0] = 0.33333334 * center[0];
                p01[1] = lightDir[1] - *lightPos;
                p01[2] = lightDir[2] - lightPos[1];
                p01[3] = center[0] - lightPos[2];
                Vec3Normalize(&p01[1]);
                //LODWORD(p01[0]) = verts[1];
                p02[1] = verts[1][0] - verts[0][0];
                p02[2] = verts[1][1] - verts[0][1];
                p02[3] = verts[1][2] - verts[0][2];
                //LODWORD(p02[0]) = verts[2];
                normal[0] = verts[2][0] - verts[0][0];
                normal[1] = verts[2][1] - verts[0][1];
                normal[2] = verts[2][2] - verts[0][2];
                Vec3Cross(&p02[1], normal, v12);
                Vec3Normalize(v12);
                v11 = 1.0f;
                if (1.0 > 0.0)
                {
                    if (1.0 < 0.60000002)
                        v11 = 0.6f;
                    clr[0] = v11 * clr[0];
                    clr[1] = v11 * clr[1];
                    clr[2] = v11 * clr[2];
                    clr[3] = 0.25f;
                    if (persistent)
                    {
                    LABEL_22:
                        R_AddPersistentDebugPolygon(&frontEndDataOut->debugGlobals, 3, verts, clr, 0, clr, 0);
                        continue;
                    }
                    R_AddDebugPolygon(&frontEndDataOut->debugGlobals, 3, verts, clr, ztest, clr, ztest);
                }
            }
            else
            {
                if (persistent)
                    goto LABEL_22;
                R_AddDebugPolygon(&frontEndDataOut->debugGlobals, 3, verts, color, ztest, edgeColor, ztest);
            }
        }
    }
}

void __cdecl calc_color(unsigned int id, float alpha, float *color)
{
    *color = 0.1f;
    color[1] = 0.85f;
    color[2] = 0.4f;
    color[3] = 1.0f;
    *color = (float)(0.5 * (float)((float)(((id >> 4) & 0xF00) >> 4) / 256.0)) + 0.5;
    color[1] = (float)(0.5 * (float)((float)((id >> 4) & 0xF0) / 256.0)) + 0.5;
    color[2] = (float)(0.5 * (float)((float)(unsigned __int8)(16 * ((unsigned __int8)id >> 4)) / 256.0)) + 0.5;
    color[3] = alpha;
}

void __cdecl render_convex_partition(const CollisionAabbTree *tree)
{
    int ind_i; // [esp+8h] [ebp-4Ch]
    float edgeColor[4]; // [esp+Ch] [ebp-48h] BYREF
    chull_t *chull; // [esp+1Ch] [ebp-38h]
    float polygonVerts[3][3]; // [esp+20h] [ebp-34h] BYREF
    float faceColor[4]; // [esp+44h] [ebp-10h] BYREF

    calc_color((unsigned int)tree, alpha, faceColor);
    memset(edgeColor, 0, 12);
    edgeColor[3] = 1.0f;
    chull = get_partition_chull(tree);
    if ( chull )
    {
        for ( ind_i = 0; ind_i < chull->ninds; ind_i += 3 )
        {
            Phys_NitrousVecToVec3(&chull->verts[chull->inds[ind_i]], polygonVerts[0]);
            Phys_NitrousVecToVec3(&chull->verts[chull->inds[ind_i + 1]], polygonVerts[1]);
            Phys_NitrousVecToVec3(&chull->verts[chull->inds[ind_i + 2]], polygonVerts[2]);
            R_AddDebugPolygon(&frontEndDataOut->debugGlobals, 3, polygonVerts, faceColor, 1, edgeColor, 1);
        }
    }
}

void    render_chull(const chull_t *first, const phys_mat44 *xform, const float *color)
{
    float verts[100][3]; // [esp+38h] [ebp-4D4h] BYREF
    float clr[4]; // [esp+4F0h] [ebp-1Ch] BYREF

    if (first)
    {
        clr[0] = 0.0f;
        clr[1] = 1.0f;
        clr[2] = 1.0f;
        clr[3] = 0.25f;
        if (color)
        {
            clr[0] = color[0];
            clr[1] = color[1];
            clr[2] = color[2];
            clr[3] = color[3];
        }

        if (!xform)
            xform = &PHYS_IDENTITY_MATRIX;

        while (first)
        {
            iassert(first->nverts < 100);

            for (int vi = 0; vi < first->nverts; ++vi)
            {
                phys_vec3 transformedVert;
                phys_full_multiply(&transformedVert, xform, &first->verts[vi]);
                Phys_NitrousVecToVec3(&transformedVert, verts[vi]);
            }

            for (int ii = 0; ii < first->ninds; ii += 3)
            {
                float *v0 = verts[first->inds[ii + 0]];
                float *v1 = verts[first->inds[ii + 1]];
                float *v2 = verts[first->inds[ii + 2]];

                float tverts[3][3]; // [esp+0h] [ebp-50Ch] BYREF
                tverts[0][0] = v0[0]; tverts[0][1] = v0[1]; tverts[0][2] = v0[2];
                tverts[1][0] = v1[0]; tverts[1][1] = v1[1]; tverts[1][2] = v1[2];
                tverts[2][0] = v2[0]; tverts[2][1] = v2[1]; tverts[2][2] = v2[2];

                R_AddDebugPolygon(&frontEndDataOut->debugGlobals, 3, tverts, clr, true, clr, true);
            }

            first = first->next;
        }
    }
}

void __cdecl render_xmodel_chull(const XModel *model, unsigned int key, const phys_mat44 *xform, const float *color)
{
    chull_t *chull; // [esp+0h] [ebp-8h]
    PhysGeomList *geomList; // [esp+4h] [ebp-4h]
    int savedregs; // [esp+8h] [ebp+0h] BYREF

    if ( model && XModelHasCollmap(model) )
    {
        geomList = XModelGetCollmap(model, 0);
        if ( geomList )
        {
            chull = get_collmap_chull(geomList, key);
            render_chull(chull, xform, color);
        }
    }
}

void __cdecl render_brushmodel_chull(
                unsigned int brushmodel,
                unsigned int key,
                const phys_mat44 *xform,
                const float *color)
{
    chull_t *brushmodel_chull; // eax
    int savedregs; // [esp+4h] [ebp+0h] BYREF

    brushmodel_chull = get_brushmodel_chull(brushmodel, key);
    render_chull(brushmodel_chull, xform, color);
}

void __cdecl render_line(const float *p0, const float *p1, const float *color, int duration, int ztest)
{
    CG_DebugLine(p0, p1, color, ztest, duration);
}

void __cdecl render_line(const phys_vec3 *p0, const phys_vec3 *p1, const float *color, int duration, int ztest)
{
    float _p1[3]; // [esp+0h] [ebp-18h] BYREF
    float _p0[3]; // [esp+Ch] [ebp-Ch] BYREF

    Phys_NitrousVecToVec3(p0, _p0);
    Phys_NitrousVecToVec3(p1, _p1);
    render_line(_p0, _p1, color, ztest, duration);
}

void    render_box(
                const phys_vec3 *mins,
                const phys_vec3 *maxs,
                const phys_mat44 *xform,
                const float *color,
                int duration)
{
    const phys_vec3 *v6; // eax
    int m; // [esp-Ch] [ebp-C8h]
    phys_vec3 *v8; // [esp-8h] [ebp-C4h]
    phys_vec3 v9; // [esp+0h] [ebp-BCh] BYREF
    int v10; // [esp+10h] [ebp-ACh]
    int j; // [esp+14h] [ebp-A8h]
    int i; // [esp+18h] [ebp-A4h]
    phys_vec3 *k; // [esp+1Ch] [ebp-A0h]
    phys_vec3 v[8]; // [esp+20h] [ebp-9Ch] BYREF
    int v15; // [esp+ACh] [ebp-10h]
    //_UNKNOWN *v16[2]; // [esp+B0h] [ebp-Ch] BYREF
    //const phys_mat44 *xforma; // [esp+BCh] [ebp+0h]
    //
    //v16[0] = a1;
    //v16[1] = xforma;
    v15 = 8;
    for (k = v; --v15 >= 0; ++k)
        ;
    for (i = 0; i < 8; ++i)
    {
        for (j = 0; j < 3; ++j)
        {
            if ((i & (1 << j)) != 0)
            {
                if (j < 0
                    && _tlAssert(
                        "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\old_phys_math.h",
                        32,
                        "i >= 0 && i < 3",
                        ""))
                {
                    __debugbreak();
                }
                v10 = *((_DWORD *)&maxs->x + j);
            }
            else
            {
                if (j < 0
                    && _tlAssert(
                        "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\old_phys_math.h",
                        32,
                        "i >= 0 && i < 3",
                        ""))
                {
                    __debugbreak();
                }
                v10 = *((_DWORD *)&mins->x + j);
            }
            if ((unsigned int)j > 2)
            {
                if (_tlAssert(
                    "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\old_phys_math.h",
                    34,
                    "i >= 0 && i < 3",
                    ""))
                {
                    __debugbreak();
                }
            }
            *((_DWORD *)&v[i].x + j) = v10;
        }
        v6 = phys_full_multiply(&v9, xform, &v[i]);
        v8 = &v[i];
        v8->x = v6->x;
        v8->y = v6->y;
        v8->z = v6->z;
    }
    for (m = 0; m < 12; ++m)
        render_line(&v[iEdgePairs[m][0]], &v[iEdgePairs[m][1]], color, duration, 0);
}

void    render_box(float *_mn, float *_mx, const float *color, int duration)
{
    phys_vec3 mx; // [esp-Ch] [ebp-2Ch] BYREF
    phys_vec3 mn; // [esp+4h] [ebp-1Ch] BYREF

    Phys_Vec3ToNitrousVec(_mn, &mn);
    Phys_Vec3ToNitrousVec(_mx, &mx);
    render_box(&mn, &mx, &PHYS_IDENTITY_MATRIX, color, duration);
}

void __cdecl render_box(const phys_vec3 *mins, const phys_vec3 *maxs, const float *color, int duration)
{
    render_box(mins, maxs, &PHYS_IDENTITY_MATRIX, color, duration);
}

void    render_gjk_geom(gjk_base_t *geom, const phys_mat44 *cg2w)
{
    float edgeColor[4]; // [esp+34h] [ebp-F0h] BYREF
    float color; // [esp+44h] [ebp-E0h] BYREF
    phys_vec3 mins; // [esp+58h] [ebp-CCh] BYREF
    phys_mat44 mat; // [esp+78h] [ebp-ACh] BYREF
    const gjk_obb_t *obb; // [esp+C4h] [ebp-60h]
    const gjk_partition_t *partition; // [esp+C4h] [ebp-60h]
    float faceColor[4]; // [esp+104h] [ebp-20h] BYREF

    if (geom->get_brush())
    {
        calc_color((unsigned int)geom->get_brush(), alpha_1, faceColor);
        render_brush(geom->get_brush(), cg2w, faceColor, 0, 0, 0, 1, edgeColor);
    }
    else
    {
        switch (geom->get_type())
        {
        case GJK_AABB:
        case GJK_DOUBLE_SPHERE:
        case GJK_CYLINDER:
            render_box(geom->get_aabb_mn(), geom->get_aabb_mx(), cg2w, colorMdRed, 0);
            break;
        case GJK_BRUSH:
            calc_color((unsigned int)geom->get_brush(), alpha_0, &color);
            memset(edgeColor, 0, 12);
            edgeColor[3] = 1.0f;
            render_brush(geom->get_brush(), cg2w, &color, 0, 0, 0, 1, edgeColor);
            break;
        case GJK_PARTITION:
            partition = (gjk_partition_t *)geom;
            render_convex_partition(partition->tree);
            break;
        case GJK_OBB:
            obb = (const gjk_obb_t *)geom;
            phys_full_multiply_mat(&mat, cg2w, &obb->m_xform);
            mins.x = -(obb->m_dims.x);
            mins.y = -(obb->m_dims.y);
            mins.z = -(obb->m_dims.z);
            render_box(&mins, &obb->m_dims, &mat, colorMdRed, 0);
            break;
        default:
            render_box(geom->get_aabb_mn(), geom->get_aabb_mx(), cg2w, colorLtOrange, 0);
            break;
        }
    }
}

void    debug_render(PhysObjUserData *userData)
{
#if 0
    gjk_base_t *i; // [esp-Ch] [ebp-70h]
    float v3; // [esp-4h] [ebp-68h] BYREF
    float pt[3]; // [esp+8h] [ebp-5Ch] BYREF
    phys_mat44 cg2w; // [esp+14h] [ebp-50h]
    rigid_body *body; // [esp+54h] [ebp-10h]
    int v7; // [esp+58h] [ebp-Ch] BYREF
    const phys_mat44 *rb2w; // [esp+5Ch] [ebp-8h]
    const phys_mat44 *retaddr; // [esp+64h] [ebp+0h]

    v7 = a1;
    rb2w = retaddr;
    body = userData->body;
    LODWORD(cg2w.w.w) = &body->m_mat;
    phys_full_multiply_mat((int)&v7, (phys_mat44 *)pt, &body->m_mat, &userData->cg2rb);
    Phys_NitrousVecToVec3((const phys_vec3 *)(LODWORD(cg2w.w.w) + 48), &v3);
    G_DebugStar(&v3, colorYellow, 0);
    if ( userData->m_gjk_geom_list.m_geom_count < 0
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.h",
                 1035,
                 "m_geom_count >= 0",
                 "") )
    {
        __debugbreak();
    }
    for ( i = userData->m_gjk_geom_list.m_first_geom; i; i = i->m_next_geom )
        render_gjk_geom(COERCE_FLOAT(&v7), i, (const phys_mat44 *)pt);
#else // aislop
    rigid_body *body = userData->body;

    phys_mat44 cg2w;
    float starPos[3];

    // Build world matrix
    phys_full_multiply_mat(
        &cg2w,
        &body->m_mat,
        &userData->cg2rb);

    // Convert body position to vec3
    Phys_NitrousVecToVec3(
        (const phys_vec3 *)((char *)&body->m_mat + 48),
        starPos);

    G_DebugStar(starPos, colorYellow, 0);

    // Sanity check
    if (userData->m_gjk_geom_list.m_geom_count < 0)
    {
        if (_tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.h",
            1035,
            "m_geom_count >= 0",
            ""))
        {
            __debugbreak();
        }
    }

    // Render each GJK geometry
    for (gjk_base_t *i = userData->m_gjk_geom_list.m_first_geom;
        i;
        i = i->m_next_geom)
    {
        render_gjk_geom(i, &cg2w);
    }

#endif
    //BLOPS_NULLSUB();
}

// aislop supervised
void    clip_winding(phys_static_array<phys_vec3,512> &winding, const plane_lt &clip)
{
    phys_static_array<float, 512> dists;
    phys_static_array<int, 512>   sides;

    int counts[3] = { 0, 0, 0 }; // [0]=front, [1]=back, [2]=on

    phys_vec3 n = clip.n;
    float     d = clip.d;
    int       num_verts = winding.m_alloc_count;

    // Classify each vertex against the plane
    for (int i = 0; i < num_verts; ++i)
    {
        float dot = winding[i]->dot(n);
        float dist = dot - d;

        *dists.add(0, "phys array add overflow.") = dist;

        int side;
        if (dist > 0.01f)
            side = 0; // front
        else if (dist < -0.01f)
            side = 1; // back
        else
            side = 2; // on

        *sides.add(0, "phys array add overflow.") = side;
        ++counts[side];
    }

    // Wrap: copy first element to end (for edge testing)
    *sides.add(0, "phys array add overflow.") = *sides[0];
    *dists.add(0, "phys array add overflow.") = *dists[0];

    // All verts are behind (or on) the plane — winding is completely clipped
    if (!counts[0])
    {
        winding.m_alloc_count = 0;
        return;
    }

    // All verts are in front — winding is unmodified, nothing to clip
    if (!counts[1])
        return;

    // Mixed — need to clip
    phys_static_array<phys_vec3, 512> clipped;

    for (int j = 0; j < winding.m_alloc_count; ++j)
    {
        phys_vec3 *p = winding[j];

        if (*sides[j] == 2) // on plane — keep as-is
        {
            *clipped.add(0, "phys array add overflow.") = *p;
            continue;
        }

        if (*sides[j] == 0) // front — keep
            *clipped.add(0, "phys array add overflow.") = *p;

        // Check if the next edge crosses a side boundary
        if (*sides[j + 1] == 2)
            continue;

        if (*sides[j + 1] == *sides[j])
            continue;

        // Edge crosses the plane — compute intersection
        phys_vec3 *q = winding[(j + 1) % num_verts];
        float      fj = *dists[j];
        float      fj1 = *dists[j + 1];
        float      t = fj / (fj - fj1);

        phys_vec3 mid;
        for (int k = 0; k < 3; ++k)
        {
            if (n[k] == 1.0f)
            {
                mid[k] = d;
            }
            else if (n[k] == -1.0f)
            {
                mid[k] = -d;
            }
            else
            {
                mid[k] = (*p)[k] + ((*q)[k] - (*p)[k]) * t;
            }
        }

        *clipped.add(0, "phys array add overflow.") = mid;
    }

    // Replace winding with clipped result
    winding.m_alloc_count = 0;
    for (int i = 0; i < clipped.m_alloc_count; ++i)
        *winding.add(0, "phys array add overflow.") = *clipped[i];
}

void    init_winding(const plane_lt *plane, phys_static_array<phys_vec3,512> *winding)
{
    float v3; // xmm0_4
    phys_vec3 *v4; // eax
    phys_vec3 *v5; // eax
    phys_vec3 *v6; // eax
    phys_vec3 *v7; // eax
    phys_vec3 *v8; // eax
    phys_vec3 *v9; // eax
    phys_vec3 *v10; // eax
    phys_vec3 *v11; // eax
    phys_vec3 *v12; // eax
    phys_vec3 *v13; // eax
    phys_vec3 v14; // [esp-6Ch] [ebp-19Ch] BYREF
    phys_vec3 v15; // [esp-5Ch] [ebp-18Ch] BYREF
    phys_vec3 v16; // [esp-4Ch] [ebp-17Ch] BYREF
    phys_vec3 v17; // [esp-3Ch] [ebp-16Ch] BYREF
    phys_vec3 v18; // [esp-2Ch] [ebp-15Ch] BYREF
    phys_vec3 v19; // [esp-1Ch] [ebp-14Ch] BYREF
    phys_vec3 v20; // [esp-Ch] [ebp-13Ch] BYREF
    float p_4; // [esp+4h] [ebp-12Ch]
    float p_8; // [esp+8h] [ebp-128h]
    float p_12; // [esp+Ch] [ebp-124h]
    float v24; // [esp+18h] [ebp-118h]
    float v25; // [esp+1Ch] [ebp-114h]
    float v26; // [esp+20h] [ebp-110h]
    float v27; // [esp+24h] [ebp-10Ch]
    float v28; // [esp+28h] [ebp-108h]
    float v29; // [esp+2Ch] [ebp-104h]
    float v30; // [esp+38h] [ebp-F8h]
    float v31; // [esp+3Ch] [ebp-F4h]
    float v32; // [esp+40h] [ebp-F0h]
    phys_vec3 right; // [esp+44h] [ebp-ECh] BYREF
    phys_vec3 org; // [esp+54h] [ebp-DCh] BYREF
    float v35; // [esp+70h] [ebp-C0h]
    float v36; // [esp+74h] [ebp-BCh]
    float v37; // [esp+78h] [ebp-B8h]
    float v38; // [esp+7Ch] [ebp-B4h]
    float len; // [esp+80h] [ebp-B0h]
    float v40; // [esp+84h] [ebp-ACh]
    float v41; // [esp+88h] [ebp-A8h]
    float v42; // [esp+8Ch] [ebp-A4h]
    float v43; // [esp+94h] [ebp-9Ch]
    float v44; // [esp+98h] [ebp-98h]
    float v45; // [esp+9Ch] [ebp-94h]
    float proj; // [esp+A0h] [ebp-90h]
    int v47; // [esp+A4h] [ebp-8Ch]
    int v48; // [esp+A8h] [ebp-88h]
    float v49; // [esp+ACh] [ebp-84h]
    phys_vec3 up; // [esp+B4h] [ebp-7Ch] BYREF
    float v51; // [esp+C4h] [ebp-6Ch]
    int v52; // [esp+C8h] [ebp-68h]
    int v53; // [esp+CCh] [ebp-64h]
    int axis; // [esp+DCh] [ebp-54h]
    BOOL v55; // [esp+E0h] [ebp-50h]
    phys_vec3 abs_normal; // [esp+E4h] [ebp-4Ch]
    int v57; // [esp+F8h] [ebp-38h]
    float x; // [esp+FCh] [ebp-34h]
    int v59; // [esp+100h] [ebp-30h]
    float y; // [esp+104h] [ebp-2Ch]
    int v61; // [esp+108h] [ebp-28h]
    float z; // [esp+10Ch] [ebp-24h]
    float dist; // [esp+110h] [ebp-20h]
    phys_vec3 normal; // [esp+114h] [ebp-1Ch] BYREF
    //_UNKNOWN *v65; // [esp+124h] [ebp-Ch]
    //const plane_lt *planea; // [esp+128h] [ebp-8h]
    //int arg0; // [esp+130h] [ebp+0h]
    //
    //v65 = a1;
    //planea = (const plane_lt *)arg0;
    normal = plane->n;
    dist = plane->d;
    z = normal.z;
    v61 = fabs(normal.z);
    y = normal.y;
    v59 = fabs(normal.y);
    x = normal.x;
    v57 = fabs(normal.x);
    (abs_normal.x) = fabs(normal.x);
    (abs_normal.y) = fabs(normal.y);
    (abs_normal.z) = fabs(normal.z);
    v55 = (fabs(normal.y)) > (fabs(normal.x));
    axis = v55;
    if ((fabs(normal.z)) <= *(&abs_normal.x + v55))
    {
        v47 = 0.0f;
        v48 = 0.0f;
        v49 = 1.0f;
        up.x = 0.0f;
        up.y = 0.0f;
        v3 = 1.0f;
    }
    else
    {
        axis = 2;
        v51 = 1.0f;
        v52 = 0.0f;
        v53 = 0.0f;
        up.x = 1.0f;
        up.y = 0.0f;
        v3 = 0.0f;
    }
    proj = (float)((float)(up.x * normal.x) + (float)(up.y * normal.y)) + (float)(v3 * normal.z);
    v45 = proj * normal.x;
    v44 = proj * normal.y;
    v43 = proj * normal.z;
    v40 = proj * normal.x;
    v41 = proj * normal.y;
    v42 = proj * normal.z;
    up.x = up.x - (float)(proj * normal.x);
    up.y = up.y - (float)(proj * normal.y);
    up.z = v3 - (float)(proj * normal.z);
    len = Vec3Length(&up.x);
    if (len != 0.0)
    {
        v38 = 1.0 / len;
        up.x = up.x * (float)(1.0 / len);
        up.y = up.y * (float)(1.0 / len);
        up.z = up.z * (float)(1.0 / len);
    }
    v37 = dist * normal.x;
    v36 = dist * normal.y;
    v35 = dist * normal.z;
    org.x = dist * normal.x;
    org.y = dist * normal.y;
    org.z = dist * normal.z;
    phys_cross(&right, &up, &normal);
    up.x = up.x * 131072.0;
    up.y = up.y * 131072.0;
    up.z = up.z * 131072.0;
    right.x = right.x * 131072.0;
    right.y = right.y * 131072.0;
    right.z = right.z * 131072.0;
    v32 = org.x - right.x;
    v31 = org.y - right.y;
    v30 = org.z - right.z;
    v27 = org.x - right.x;
    v28 = org.y - right.y;
    v29 = org.z - right.z;
    v26 = (float)(org.x - right.x) + up.x;
    v25 = (float)(org.y - right.y) + up.y;
    v24 = (float)(org.z - right.z) + up.z;
    p_4 = v26;
    p_8 = v25;
    p_12 = v24;
    v20.x = v26;
    v20.y = v25;
    v20.z = v24;

    //v4 = phys_static_array<phys_vec3, 512>::add(winding, 0, "phys array add overflow.");
    v4 = winding->add(0, "phys array add overflow.");
    //phys_vec3::operator=(v4, &v20);
    *v4 = v20;
    //v5 = operator+(&v19, &org, &right);
    v19 = org + right;
    v5 = &v19;
    //v6 = operator+(&v18, v5, &up);
    v18 = *v5 + up;
    v6 = &v18;
    //phys_vec3::operator=(&v20, v6);
    v20 = *v6;

    //v7 = phys_static_array<phys_vec3, 512>::add(winding, 0, "phys array add overflow.");
    v7 = winding->add(0, "phys array add overflow.");
    //phys_vec3::operator=(v7, &v20);
    *v7 = v20;
    //v8 = operator+(&v17, &org, &right);
    //*v8 = org + right;
    v17 = org + right;
    v8 = &v17;
    //v9 = operator-(&v16, v8, &up);
    v16 = *v8 - up;
    v9 = &v16;
    //phys_vec3::operator=(&v20, v9);
    v20 = *v9;

    //v10 = phys_static_array<phys_vec3, 512>::add(winding, 0, "phys array add overflow.");
    v10 = winding->add(0, "phys array add overflow.");
    //phys_vec3::operator=(v10, &v20);
    *v10 = v20;
    //v11 = operator-(&v15, &org, &right);
    v15 = org - right;
    v11 = &v15;
    //v12 = operator-(&v14, v11, &up);
    v14 = *v11 - up;
    v12 = &v14;
    //phys_vec3::operator=(&v20, v12);
    v20 = *v12;

    //v13 = phys_static_array<phys_vec3, 512>::add(winding, 0, "phys array add overflow.");
    v13 = winding->add(0, "phys array add overflow.");
    //phys_vec3::operator=(v13, &v20);
    *v13 = v20;
}

void    calc_winding(
                const phys_static_array<plane_lt,512> &planes,
                int plane_index,
                phys_static_array<phys_vec3,512> *winding)
{
    const plane_lt *first_plane = planes[plane_index];
    init_winding(first_plane, winding);

    bool past = false;
    int nplanes = planes.m_alloc_count;
    for (int i = 0; i < nplanes; ++i)
    {
        if (i == plane_index)
        {
            past = true;
            continue;
        }

        plane_lt clip = *planes[i];

        float dot = first_plane->n.dot(clip.n);
        float ddelta = first_plane->d - clip.d;

        if (dot > 0.99989998f && fabsf(ddelta) < 0.001f)
        {
            if (past)
            {
                winding->m_alloc_count = 0;
                return;
            }
            continue;
        }

        clip.n.x = -clip.n.x;
        clip.n.y = -clip.n.y;
        clip.n.z = -clip.n.z;
        clip.d = -clip.d;

        clip_winding(*winding, clip);

        if (winding->m_alloc_count < 3)
            return;
    }
}

void __cdecl Phys_DebugPoint(const phys_vec3 *pos, float radius, const float *color)
{
    float point[3]; // [esp+0h] [ebp-Ch] BYREF

    Phys_NitrousVecToVec3(pos, point);
    G_DebugStar(point, color, 0);
}

void __cdecl Phys_DebugBox(const phys_mat44 *mat, const float *color, float scale)
{
    float origin[3]; // [esp+24h] [ebp-4Ch] BYREF
    float mins[3]; // [esp+30h] [ebp-40h] BYREF
    float rotation[3][3]; // [esp+3Ch] [ebp-34h] BYREF
    float size; // [esp+60h] [ebp-10h]
    float maxs[3]; // [esp+64h] [ebp-Ch] BYREF

    size = 7.0f;
    //mins[0] = scale * COERCE_FLOAT(LODWORD(7.0f) ^ _mask__NegFloat_);
    mins[0] = scale * (-(7.0f));
    mins[1] = scale * -3.0;
    mins[2] = scale * -3.0;
    maxs[0] = scale * 7.0;
    maxs[1] = scale * 3.0;
    maxs[2] = scale * 3.0;
    Phys_NitrousVecToVec3(&mat->w, origin);
    Phys_NitrousMat44ToVec33(mat, rotation);
    CG_DebugBoxOriented(origin, mins, maxs, rotation, color, 0, 0);
}

void __cdecl Phys_DebugBox(const phys_mat44 *mat, const phys_vec3 *dims, const float *color)
{
    float origin[3]; // [esp+24h] [ebp-54h] BYREF
    float mins[3]; // [esp+30h] [ebp-48h] BYREF
    float _dims[3]; // [esp+3Ch] [ebp-3Ch] BYREF
    float rotation[3][3]; // [esp+48h] [ebp-30h] BYREF
    float maxs[3]; // [esp+6Ch] [ebp-Ch] BYREF

    Phys_NitrousVecToVec3(dims, _dims);
    maxs[0] = _dims[0];
    maxs[1] = _dims[1];
    maxs[2] = _dims[2];
    mins[0] = -1.0 * _dims[0];
    mins[1] = -1.0 * _dims[1];
    mins[2] = -1.0 * _dims[2];
    Phys_NitrousVecToVec3(&mat->w, origin);
    Phys_NitrousMat44ToVec33(mat, rotation);
    CG_DebugBoxOriented(origin, mins, maxs, rotation, color, 0, 0);
}

void    Phys_RenderBase(const phys_mat44 *mat, float scale)
{
    phys_vec3 v3; // [esp-CCh] [ebp-DCh] BYREF
    float v4; // [esp-B8h] [ebp-C8h]
    float v5; // [esp-B4h] [ebp-C4h]
    float v6; // [esp-B0h] [ebp-C0h]
    float v7; // [esp-ACh] [ebp-BCh]
    float v8; // [esp-A8h] [ebp-B8h]
    float v9; // [esp-A4h] [ebp-B4h]
    float v10; // [esp-9Ch] [ebp-ACh]
    float v11; // [esp-98h] [ebp-A8h]
    float v12; // [esp-94h] [ebp-A4h]
    const phys_vec3 *p_z; // [esp-90h] [ebp-A0h]
    phys_vec3 v14; // [esp-8Ch] [ebp-9Ch] BYREF
    float v15; // [esp-78h] [ebp-88h]
    float v16; // [esp-74h] [ebp-84h]
    float v17; // [esp-70h] [ebp-80h]
    float v18; // [esp-6Ch] [ebp-7Ch]
    float v19; // [esp-68h] [ebp-78h]
    float v20; // [esp-64h] [ebp-74h]
    float v21; // [esp-5Ch] [ebp-6Ch]
    float v22; // [esp-58h] [ebp-68h]
    float v23; // [esp-54h] [ebp-64h]
    const phys_vec3 *p_y; // [esp-50h] [ebp-60h]
    phys_vec3 v25; // [esp-4Ch] [ebp-5Ch] BYREF
    float v26; // [esp-38h] [ebp-48h]
    float v27; // [esp-34h] [ebp-44h]
    float v28; // [esp-30h] [ebp-40h]
    float v29; // [esp-2Ch] [ebp-3Ch]
    float v30; // [esp-28h] [ebp-38h]
    float v31; // [esp-24h] [ebp-34h]
    float v32; // [esp-18h] [ebp-28h]
    float v33; // [esp-14h] [ebp-24h]
    float v34; // [esp-10h] [ebp-20h]
    float x; // [esp-Ch] [ebp-1Ch] BYREF
    float y; // [esp-8h] [ebp-18h]
    float z; // [esp-4h] [ebp-14h]
    //phys_vec3 trans; // [esp+0h] [ebp-10h]
    //float retaddr; // [esp+10h] [ebp+0h]
    //
    //trans.y = a1;
    //trans.z = retaddr;
    x = mat->w.x;
    y = mat->w.y;
    z = mat->w.z;
    //trans.x = mat->w.w;
    v34 = scale * mat->x.x;
    v33 = scale * mat->x.y;
    v32 = scale * mat->x.z;
    v29 = v34;
    v30 = v33;
    v31 = v32;
    v28 = x + v34;
    v27 = y + v33;
    v26 = z + v32;
    v25.x = x + v34;
    v25.y = y + v33;
    v25.z = z + v32;
    render_line((const phys_vec3 *)&x, &v25, colorMdRed, 0, 0);
    p_y = &mat->y;
    v23 = scale * mat->y.x;
    v22 = scale * mat->y.y;
    v21 = scale * mat->y.z;
    v18 = v23;
    v19 = v22;
    v20 = v21;
    v17 = x + v23;
    v16 = y + v22;
    v15 = z + v21;
    v14.x = x + v23;
    v14.y = y + v22;
    v14.z = z + v21;
    render_line((const phys_vec3 *)&x, &v14, colorMdGreen, 0, 0);
    p_z = &mat->z;
    v12 = scale * mat->z.x;
    v11 = scale * mat->z.y;
    v10 = scale * mat->z.z;
    v7 = v12;
    v8 = v11;
    v9 = v10;
    v6 = x + v12;
    v5 = y + v11;
    v4 = z + v10;
    v3.x = x + v12;
    v3.y = y + v11;
    v3.z = z + v10;
    render_line((const phys_vec3 *)&x, &v3, colorMdBlue, 0, 0);
}

void    Phys_JointDebugRender(int id)
{
    // KISAKTODO: arg doesnt make sense, needs rework
#if 0
    const phys_vec3 *v2; // eax
    const phys_vec3 *v3; // eax
    const phys_vec3 *v4; // eax
    const phys_vec3 *v5; // eax
    phys_vec3 *v6; // eax
    const phys_vec3 *v7; // eax
    phys_vec3 *v8; // eax
    const phys_vec3 *v9; // eax
    phys_vec3 *v10; // eax
    const phys_vec3 *v11; // eax
    const phys_vec3 *v12; // eax
    phys_vec3 *v13; // eax
    const phys_vec3 *v14; // eax
    const phys_vec3 *v15; // eax
    phys_vec3 *v16; // eax
    const phys_vec3 *v17; // eax
    phys_vec3 *p_w; // [esp+Ch] [ebp-3C0h]
    phys_vec3 *v19; // [esp+Ch] [ebp-3C0h]
    phys_vec3 *v20; // [esp+Ch] [ebp-3C0h]
    phys_vec3 v21; // [esp+10h] [ebp-3BCh] BYREF
    phys_vec3 v22; // [esp+20h] [ebp-3ACh] BYREF
    phys_vec3 v23; // [esp+30h] [ebp-39Ch] BYREF
    phys_vec3 v24; // [esp+40h] [ebp-38Ch] BYREF
    phys_vec3 v25; // [esp+50h] [ebp-37Ch] BYREF
    phys_vec3 v26; // [esp+60h] [ebp-36Ch] BYREF
    phys_vec3 v27; // [esp+70h] [ebp-35Ch] BYREF
    phys_vec3 v28; // [esp+80h] [ebp-34Ch] BYREF
    phys_vec3 pt2; // [esp+90h] [ebp-33Ch] BYREF
    phys_vec3 pt1; // [esp+A0h] [ebp-32Ch] BYREF
    phys_vec3 v31; // [esp+B0h] [ebp-31Ch] BYREF
    phys_vec3 v32; // [esp+C0h] [ebp-30Ch] BYREF
    phys_vec3 v33; // [esp+D0h] [ebp-2FCh] BYREF
    phys_vec3 v34; // [esp+E0h] [ebp-2ECh] BYREF
    phys_vec3 p2; // [esp+F0h] [ebp-2DCh] BYREF
    phys_vec3 p1; // [esp+100h] [ebp-2CCh] BYREF
    phys_vec3 v37; // [esp+110h] [ebp-2BCh] BYREF
    phys_mat44 *v38; // [esp+12Ch] [ebp-2A0h]
    phys_vec3 v39; // [esp+130h] [ebp-29Ch] BYREF
    phys_vec3 v40; // [esp+140h] [ebp-28Ch] BYREF
    float v41; // [esp+150h] [ebp-27Ch]
    float v42; // [esp+154h] [ebp-278h]
    float v43; // [esp+164h] [ebp-268h]
    float v44; // [esp+168h] [ebp-264h]
    float v45; // [esp+16Ch] [ebp-260h]
    float v46; // [esp+170h] [ebp-25Ch]
    float v47; // [esp+174h] [ebp-258h]
    float v48; // [esp+178h] [ebp-254h]
    float v49; // [esp+184h] [ebp-248h]
    float v50; // [esp+188h] [ebp-244h]
    float v51; // [esp+18Ch] [ebp-240h]
    float v52; // [esp+190h] [ebp-23Ch]
    float v53; // [esp+194h] [ebp-238h]
    float v54; // [esp+198h] [ebp-234h]
    float v55; // [esp+1A4h] [ebp-228h]
    float v56; // [esp+1A8h] [ebp-224h]
    float v57; // [esp+1ACh] [ebp-220h]
    float v58; // [esp+1B0h] [ebp-21Ch]
    float v59; // [esp+1B4h] [ebp-218h]
    float v60; // [esp+1B8h] [ebp-214h]
    float v61; // [esp+1C4h] [ebp-208h]
    float v62; // [esp+1C8h] [ebp-204h]
    float v63; // [esp+1CCh] [ebp-200h]
    float v64; // [esp+1D0h] [ebp-1FCh]
    float v65; // [esp+1D4h] [ebp-1F8h]
    float v66; // [esp+1D8h] [ebp-1F4h]
    float v67; // [esp+1E4h] [ebp-1E8h]
    float v68; // [esp+1E8h] [ebp-1E4h]
    float v69; // [esp+1ECh] [ebp-1E0h]
    float v70; // [esp+1F0h] [ebp-1DCh]
    float v71; // [esp+1F4h] [ebp-1D8h]
    float v72; // [esp+1F8h] [ebp-1D4h]
    float v73; // [esp+204h] [ebp-1C8h]
    float v74; // [esp+208h] [ebp-1C4h]
    float v75; // [esp+20Ch] [ebp-1C0h]
    float v76; // [esp+210h] [ebp-1BCh]
    float v77; // [esp+214h] [ebp-1B8h]
    float v78; // [esp+218h] [ebp-1B4h]
    float v79; // [esp+224h] [ebp-1A8h]
    float v80; // [esp+228h] [ebp-1A4h]
    float v81; // [esp+22Ch] [ebp-1A0h]
    float v82; // [esp+230h] [ebp-19Ch]
    float v83; // [esp+234h] [ebp-198h]
    float v84; // [esp+238h] [ebp-194h]
    float v85; // [esp+244h] [ebp-188h]
    float v86; // [esp+248h] [ebp-184h]
    float v87; // [esp+24Ch] [ebp-180h]
    phys_vec3 phys_vec_end; // [esp+250h] [ebp-17Ch] BYREF
    float v89; // [esp+260h] [ebp-16Ch]
    float v90; // [esp+264h] [ebp-168h]
    float v91; // [esp+268h] [ebp-164h]
    float v92; // [esp+274h] [ebp-158h]
    float v93; // [esp+278h] [ebp-154h]
    float v94; // [esp+27Ch] [ebp-150h]
    float v95; // [esp+280h] [ebp-14Ch]
    float v96; // [esp+284h] [ebp-148h]
    float v97; // [esp+288h] [ebp-144h]
    float v98; // [esp+294h] [ebp-138h]
    float v99; // [esp+298h] [ebp-134h]
    float v100; // [esp+29Ch] [ebp-130h]
    phys_vec3 b2_ref_max; // [esp+2A0h] [ebp-12Ch] BYREF
    phys_vec3 b2_ref_min; // [esp+2B0h] [ebp-11Ch] BYREF
    phys_vec3 b1_ref; // [esp+2C0h] [ebp-10Ch] BYREF
    phys_vec3 v104; // [esp+2D0h] [ebp-FCh] BYREF
    phys_mat44 rv; // [esp+2E0h] [ebp-ECh] BYREF
    float v106[3]; // [esp+324h] [ebp-A8h] BYREF
    phys_vec3 phys_vec_start2; // [esp+330h] [ebp-9Ch] BYREF
    phys_vec3 v108; // [esp+340h] [ebp-8Ch] BYREF
    phys_mat44 *v109; // [esp+35Ch] [ebp-70h]
    phys_vec3 phys_vec_start; // [esp+360h] [ebp-6Ch] BYREF
    phys_vec3 v111; // [esp+370h] [ebp-5Ch] BYREF
    phys_mat44 *p_m_mat; // [esp+38Ch] [ebp-40h]
    phys_vec3 b2_axis; // [esp+390h] [ebp-3Ch] BYREF
    phys_vec3 b1_axis; // [esp+3A0h] [ebp-2Ch] BYREF
    rigid_body *b2; // [esp+3B0h] [ebp-1Ch]
    rigid_body *b1; // [esp+3B4h] [ebp-18h]
    rigid_body_constraint_ragdoll *joint; // [esp+3B8h] [ebp-14h]
    float render_length; // [esp+3BCh] [ebp-10h]
    //_UNKNOWN *v119[2]; // [esp+3C0h] [ebp-Ch] BYREF
    //int vars0; // [esp+3CCh] [ebp+0h]
    //
    //v119[0] = a1;
    //v119[1] = (_UNKNOWN *)vars0;
    render_length = 15.0f;

    iassert(id);

    joint = id;
    b1 = id->b1;
    b2 = id->b2;
    if ((id->m_flags & 8) != 0)
    {
        phys_multiply(&b1_axis, &b1->m_mat, &joint->m_b1_axis_loc);
        phys_multiply(&b2_axis, &b2->m_mat, &joint->m_b2_axis_loc);
        p_m_mat = &b1->m_mat;
        p_w = &b1->m_mat.w;
        v2 = phys_multiply(&v111, &b1->m_mat, &joint->m_b1_r_loc);
        operator+(&phys_vec_start, v2, p_w);
        v109 = &b2->m_mat;
        v19 = &b2->m_mat.w;
        v3 = phys_multiply(&v108, &b2->m_mat, &joint->m_b2_r_loc);
        operator+(&phys_vec_start2, v3, v19);
        Phys_NitrousVecToVec3(&phys_vec_start2, v106);
        G_DebugStar(v106, colorLtGreen, 0);
        make_rotate(COERCE_FLOAT(v119), &rv, &b1_axis, &b2_axis);
        v4 = phys_multiply(&v104, &b1->m_mat, &joint->m_b1_ref_loc);
        phys_multiply(&b1_ref, &rv, v4);
        phys_multiply(&b2_ref_min, &b2->m_mat, &joint->m_b2_ref_min_loc);
        phys_multiply(&b2_ref_max, &b2->m_mat, &joint->m_b2_ref_max_loc);
        v100 = render_length * b1_ref.x;
        v99 = render_length * b1_ref.y;
        v98 = render_length * b1_ref.z;
        v95 = render_length * b1_ref.x;
        v96 = render_length * b1_ref.y;
        v97 = render_length * b1_ref.z;
        v94 = phys_vec_start.x + (float)(render_length * b1_ref.x);
        v93 = phys_vec_start.y + (float)(render_length * b1_ref.y);
        v92 = phys_vec_start.z + (float)(render_length * b1_ref.z);
        v89 = v94;
        v90 = v93;
        v91 = v92;
        phys_vec_end.x = v94;
        phys_vec_end.y = v93;
        phys_vec_end.z = v92;
        render_line(&phys_vec_start, &phys_vec_end, colorMdGreen, 0, 0);
        v87 = render_length * b2_ref_min.x;
        v86 = render_length * b2_ref_min.y;
        v85 = render_length * b2_ref_min.z;
        v82 = render_length * b2_ref_min.x;
        v83 = render_length * b2_ref_min.y;
        v84 = render_length * b2_ref_min.z;
        v81 = phys_vec_start.x + (float)(render_length * b2_ref_min.x);
        v80 = phys_vec_start.y + (float)(render_length * b2_ref_min.y);
        v79 = phys_vec_start.z + (float)(render_length * b2_ref_min.z);
        v76 = v81;
        v77 = v80;
        v78 = v79;
        v75 = render_length * b2_ref_max.x;
        v74 = render_length * b2_ref_max.y;
        v73 = render_length * b2_ref_max.z;
        v70 = render_length * b2_ref_max.x;
        v71 = render_length * b2_ref_max.y;
        v72 = render_length * b2_ref_max.z;
        v69 = phys_vec_start.x + (float)(render_length * b2_ref_max.x);
        v68 = phys_vec_start.y + (float)(render_length * b2_ref_max.y);
        v67 = phys_vec_start.z + (float)(render_length * b2_ref_max.z);
        v64 = v69;
        v65 = v68;
        v66 = v67;
        v63 = render_length * b1_axis.x;
        v62 = render_length * b1_axis.y;
        v61 = render_length * b1_axis.z;
        v58 = render_length * b1_axis.x;
        v59 = render_length * b1_axis.y;
        v60 = render_length * b1_axis.z;
        v57 = phys_vec_start.x + (float)(render_length * b1_axis.x);
        v56 = phys_vec_start.y + (float)(render_length * b1_axis.y);
        v55 = phys_vec_start.z + (float)(render_length * b1_axis.z);
        v52 = v57;
        v53 = v56;
        v54 = v55;
        phys_vec_end.x = v57;
        phys_vec_end.y = v56;
        phys_vec_end.z = v55;
        render_line(&phys_vec_start, &phys_vec_end, colorLtGrey, 0, 0);
        v51 = render_length * b2_axis.x;
        v50 = render_length * b2_axis.y;
        v49 = render_length * b2_axis.z;
        v46 = render_length * b2_axis.x;
        v47 = render_length * b2_axis.y;
        v48 = render_length * b2_axis.z;
        v45 = phys_vec_start.x + (float)(render_length * b2_axis.x);
        v44 = phys_vec_start.y + (float)(render_length * b2_axis.y);
        v43 = phys_vec_start.z + (float)(render_length * b2_axis.z);
        v41 = v45;
        v42 = v44;
        phys_vec_end.x = v45;
        phys_vec_end.y = v44;
        phys_vec_end.z = v43;
        render_line(&phys_vec_start, &phys_vec_end, colorOrange, 0, 0);
        Phys_DebugBox(&b1->m_mat, colorWhiteFaded, 1.0);
        Phys_DebugBox(&b2->m_mat, colorWhiteFaded, 1.0);
    }
    else if ((joint->m_flags & 4) != 0)
    {
        if (render_pivots)
        {
            phys_multiply(&v40, &b1->m_mat, &joint->m_b1_axis_loc);
            phys_multiply(&v39, &b2->m_mat, &joint->m_b2_axis_loc);
            v38 = &b1->m_mat;
            v20 = &b1->m_mat.w;
            v5 = phys_multiply(&v37, &b1->m_mat, &joint->m_b1_r_loc);
            operator+(&p1, v5, v20);
            phys_full_multiply((int)v119, &p2, &b2->m_mat, &joint->m_b2_r_loc);
            v6 = operator*(&v34, &v40, l1);
            v7 = operator+(&v33, &p1, v6);
            render_line(&p1, v7, colorMdGreen, 0, 0);
            v8 = operator*(&v32, &v39, l2);
            v9 = operator+(&v31, &p2, v8);
            render_line(&p2, v9, colorMdGreen, 0, 0);
            Phys_RenderBase(COERCE_FLOAT(v119), &b1->m_mat, l1);
            Phys_RenderBase(COERCE_FLOAT(v119), &b2->m_mat, l2);
        }
        phys_full_multiply((int)v119, &pt1, &b1->m_mat, &joint->m_b1_r_loc);
        phys_multiply(&pt2, &b2->m_mat, &joint->m_b2_ref_min_loc);
        v10 = operator*(&v28, &pt2, line_length);
        v11 = operator+(&v27, &pt1, v10);
        phys_vec3::operator=(&pt2, v11);
        v12 = phys_multiply(&v26, &b2->m_mat, &joint->m_b2_ref_max_loc);
        phys_vec3::operator=(&pt2, v12);
        v13 = operator*(&v25, &pt2, line_length);
        v14 = operator+(&v24, &pt1, v13);
        phys_vec3::operator=(&pt2, v14);
        v15 = phys_multiply(&v23, &b1->m_mat, &joint->m_b1_ref_loc);
        phys_vec3::operator=(&pt2, v15);
        v16 = operator*(&v22, &pt2, line_length);
        v17 = operator+(&v21, &pt1, v16);
        phys_vec3::operator=(&pt2, v17);
        render_line(&pt1, &pt2, colorYellow, 0, 0);
    }
#endif
}

void    make_rotate(phys_mat44 *mat, const phys_vec3 *v1, const phys_vec3 *v2)
{
    float len; // [esp+8h] [ebp-2Ch]
    float co_; // [esp+Ch] [ebp-28h]
    float si_; // [esp+14h] [ebp-20h]
    float si_a; // [esp+14h] [ebp-20h]
    phys_vec3 ud; // [esp+18h] [ebp-1Ch] BYREF
    //_UNKNOWN *v9; // [esp+28h] [ebp-Ch]
    //phys_mat44 *mata; // [esp+2Ch] [ebp-8h]
    //const phys_vec3 *v2a; // [esp+34h] [ebp+0h]
    //
    //v9 = a1;
    //mata = (phys_mat44 *)v2a;
    phys_cross(&ud, v1, v2);
    si_ = (float)((float)(ud.x * ud.x) + (float)(ud.y * ud.y)) + (float)(ud.z * ud.z);
    if (si_ >= 0.0000099999997)
    {
        si_a = sqrtf(si_);
        ud.x = ud.x * (float)(1.0 / si_a);
        ud.y = ud.y * (float)(1.0 / si_a);
        ud.z = ud.z * (float)(1.0 / si_a);
        co_ = (float)((float)(v1->x * v2->x) + (float)(v1->y * v2->y)) + (float)(v1->z * v2->z);
        len = sqrtf((float)(co_ * co_) + (float)(si_a * si_a));
        make_rotate(*mat, &ud, co_ / len, si_a / len);
    }
    else
    {
        //phys_mat44::operator=(mat, &PHYS_IDENTITY_MATRIX_44);
        *mat = PHYS_IDENTITY_MATRIX;
    }
}

void    render_contact(rigid_body_constraint_contact *rbc)
{
    const phys_vec3 *v2; // eax
    const phys_vec3 *p_w; // [esp-50h] [ebp-130h]
    phys_vec3 v4; // [esp-4Ch] [ebp-12Ch] BYREF
    float v5; // [esp-38h] [ebp-118h]
    float v6; // [esp-34h] [ebp-114h]
    float v7; // [esp-30h] [ebp-110h]
    phys_vec3 v8; // [esp-2Ch] [ebp-10Ch] BYREF
    float v9; // [esp-18h] [ebp-F8h]
    float v10; // [esp-14h] [ebp-F4h]
    float v11; // [esp-10h] [ebp-F0h]
    float v12; // [esp-Ch] [ebp-ECh]
    float v13; // [esp-8h] [ebp-E8h]
    float v14; // [esp-4h] [ebp-E4h]
    phys_vec3 dims; // [esp+0h] [ebp-E0h]
    float v16; // [esp+10h] [ebp-D0h]
    phys_vec3 p1; // [esp+14h] [ebp-CCh] BYREF
    float v18; // [esp+28h] [ebp-B8h]
    float v19; // [esp+2Ch] [ebp-B4h]
    float v20; // [esp+30h] [ebp-B0h]
    float v21; // [esp+34h] [ebp-ACh]
    float v22; // [esp+38h] [ebp-A8h]
    float v23; // [esp+3Ch] [ebp-A4h]
    float v24; // [esp+44h] [ebp-9Ch]
    float v25; // [esp+48h] [ebp-98h]
    float v26; // [esp+4Ch] [ebp-94h]
    float v27; // [esp+50h] [ebp-90h]
    int v28; // [esp+54h] [ebp-8Ch]
    int v29; // [esp+58h] [ebp-88h]
    int v30; // [esp+5Ch] [ebp-84h]
    int v31; // [esp+64h] [ebp-7Ch]
    int v32; // [esp+68h] [ebp-78h]
    int v33; // [esp+6Ch] [ebp-74h]
    environment_rigid_body *v34; // [esp+70h] [ebp-70h]
    phys_vec3 hitn; // [esp+74h] [ebp-6Ch]
    phys_vec3 p0; // [esp+84h] [ebp-5Ch] BYREF
    phys_vec3 v37; // [esp+94h] [ebp-4Ch] BYREF
    phys_mat44 *p_m_mat; // [esp+ACh] [ebp-34h]
    const phys_vec3 *v39; // [esp+B0h] [ebp-30h]
    rigid_body *b2; // [esp+B4h] [ebp-2Ch]
    int i; // [esp+B8h] [ebp-28h]
    contact_point_info *cpi; // [esp+BCh] [ebp-24h]
    contact_point_info *m_next_link; // [esp+C0h] [ebp-20h]
    phys_simple_link_list<contact_point_info>::iterator cpi_i_end; // [esp+C4h] [ebp-1Ch]
    phys_simple_link_list<contact_point_info>::iterator cpi_i; // [esp+C8h] [ebp-18h]
    contact_point_info *m_first; // [esp+CCh] [ebp-14h]
    phys_simple_link_list<contact_point_info> *list_cpi; // [esp+D0h] [ebp-10h]
    //_UNKNOWN *v48; // [esp+D4h] [ebp-Ch]
    //rigid_body_constraint_contact *rbca; // [esp+D8h] [ebp-8h]
    //int vars0; // [esp+E0h] [ebp+0h]
    //
    //v48 = a1;
    //rbca = (rigid_body_constraint_contact *)vars0;
    if (rbc)
    {
        list_cpi = &rbc->m_list_contact_point_info_buffer_1;
        m_first = rbc->m_list_contact_point_info_buffer_1.m_first;
        cpi_i.m_ptr = m_first;
        cpi_i_end.m_ptr = 0;
        while (cpi_i_end.m_ptr != cpi_i.m_ptr)
        {
            cpi = cpi_i.m_ptr;
            if (cpi_i.m_ptr->m_point_pair_count <= 0
                && _tlAssert(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_render.cpp",
                    1361,
                    "cpi.m_point_pair_count > 0",
                    ""))
            {
                __debugbreak();
            }
            for (i = 0; i < cpi->m_point_pair_count; ++i)
            {
                b2 = rbc->b2;
                v39 = &cpi->m_list_b2_r_loc[i];
                p_m_mat = &b2->m_mat;
                p_w = &b2->m_mat.w;
                v2 = phys_multiply(&v37, &b2->m_mat, v39);
                //operator+(&p0, v2, p_w);
                p0 = *v2 + *p_w;
                hitn = cpi->m_normal;
                v34 = (environment_rigid_body *)rbc->b2;
                if (v34 == phys_sys::get_environment_rigid_body())
                {
                    v33 = -(hitn.x);
                    v32 = -(hitn.y);
                    v31 = -(hitn.z);
                    v28 = -(hitn.x);
                    v29 = -(hitn.y);
                    v30 = -(hitn.z);
                    (hitn.x) = -(hitn.x);
                    (hitn.y) = -(hitn.y);
                    (hitn.z) = -(hitn.z);
                }
                v27 = scale_4;
                v26 = scale_4 * hitn.x;
                v25 = scale_4 * hitn.y;
                v24 = scale_4 * hitn.z;
                v21 = scale_4 * hitn.x;
                v22 = scale_4 * hitn.y;
                v23 = scale_4 * hitn.z;
                v20 = p0.x + (float)(scale_4 * hitn.x);
                v19 = p0.y + (float)(scale_4 * hitn.y);
                v18 = p0.z + (float)(scale_4 * hitn.z);
                p1.x = v20;
                p1.y = v19;
                p1.z = v18;
                render_line(&p0, &p1, colorWhite, 0, 0);
                v16 = box_size;
                dims.w = box_size;
                dims.z = box_size;
                v12 = box_size;
                v13 = box_size;
                v14 = box_size;
                v11 = p1.x + box_size;
                v10 = p1.y + box_size;
                v9 = p1.z + box_size;
                v8.x = p1.x + box_size;
                v8.y = p1.y + box_size;
                v8.z = p1.z + box_size;
                v7 = p1.x - box_size;
                v6 = p1.y - box_size;
                v5 = p1.z - box_size;
                v4.x = p1.x - box_size;
                v4.y = p1.y - box_size;
                v4.z = p1.z - box_size;
                render_box(&v4, &v8, colorWhite, 0);
            }
            m_next_link = cpi_i.m_ptr->m_next_link;
            cpi_i.m_ptr = m_next_link;
        }
    }
}

void __cdecl render_collision_tree(const CollisionAabbTree *tree, const float *color)
{
    float *var6C; // [esp+10h] [ebp-6Ch]
    float *v3; // [esp+14h] [ebp-68h]
    float *v4; // [esp+18h] [ebp-64h]
    float mins[3]; // [esp+1Ch] [ebp-60h] BYREF
    float maxs[3]; // [esp+28h] [ebp-54h] BYREF
    float v7[4]; // [esp+34h] [ebp-48h] BYREF
    int triIndex; // [esp+44h] [ebp-38h]
    float v0[3]; // [esp+48h] [ebp-34h] BYREF
    float v2[3]; // [esp+54h] [ebp-28h] BYREF
    float v1[3]; // [esp+60h] [ebp-1Ch] BYREF
    const CollisionPartition *partition; // [esp+6Ch] [ebp-10h]
    float center[3]; // [esp+70h] [ebp-Ch] BYREF

    partition = &cm.partitions[tree->u.firstChildIndex];
    for ( triIndex = partition->firstTri; triIndex < partition->firstTri + partition->triCount; ++triIndex )
    {
        v4 = cm.verts[cm.triIndices[3 * triIndex]];
        v0[0] = *v4;
        v0[1] = v4[1];
        v0[2] = v4[2];
        v3 = cm.verts[cm.triIndices[3 * triIndex + 1]];
        v1[0] = *v3;
        v1[1] = v3[1];
        v1[2] = v3[2];
        var6C = cm.verts[cm.triIndices[3 * triIndex + 2]];
        v2[0] = *var6C;
        v2[1] = var6C[1];
        v2[2] = var6C[2];
        CG_DebugLine(v0, v1, color, 1, 0);
        CG_DebugLine(v0, v2, color, 1, 0);
        CG_DebugLine(v1, v2, color, 1, 0);
        center[0] = (float)(v0[0] + v1[0]) + v2[0];
        center[1] = (float)(v0[1] + v1[1]) + v2[1];
        center[2] = (float)(v0[2] + v1[2]) + v2[2];
        center[0] = 0.33333334 * center[0];
        center[1] = 0.33333334 * center[1];
        center[2] = 0.33333334 * center[2];
        mins[0] = -rad;
        mins[1] = -rad;
        mins[2] = -rad;
        maxs[0] = rad;
        maxs[1] = rad;
        maxs[2] = rad;
        v7[0] = 1.0f;
        v7[1] = 0.0f;
        v7[2] = 0.0f;
        v7[3] = 1.0f;
        CG_DebugBox(center, mins, maxs, 0.0, v7, 1, 0);
    }
}

void __cdecl render_prims(col_prim_t *prims, int nprims)
{
    col_prim_t *prim; // [esp+0h] [ebp-8h]
    int pi; // [esp+4h] [ebp-4h]
    int savedregs; // [esp+8h] [ebp+0h] BYREF

    for ( pi = 0; pi < nprims; ++pi )
    {
        prim = &prims[pi];
        if ( prim->type )
            render_brush(prim->brush, 0, 0, 1, 0, 0, 0, 0);
        else
            render_collision_tree(prim->tree, colorWhite);
    }
}

void __cdecl render_debug_draw_gjk_trace_geom()
{
    GjkTraceGeom::Render();
}

GjkTraceGeom GjkTraceGeom::geoms[50];
int GjkTraceGeom::nGeoms;

void __cdecl GjkTraceGeom::Render()
{
    int i; // [esp+0h] [ebp-4h]
    int savedregs; // [esp+4h] [ebp+0h] BYREF

    if (g_debugRenderGjkTraceGeom->current.enabled)
    {
        for (i = 0; i < GjkTraceGeom::nGeoms; ++i)
            render_brush(
                GjkTraceGeom::geoms[i].brush,
                &GjkTraceGeom::geoms[i].mat,
                0,
                1,
                0,
                0,
                0,
                0);
    }
    GjkTraceGeom::nGeoms = 0;
}