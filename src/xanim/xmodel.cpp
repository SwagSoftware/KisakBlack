#include "xmodel.h"
#include <universal/assertive.h>
#include <universal/q_shared.h>
#include <database/db_registry.h>
#include <gfx_d3d/r_model.h>
#include <clientscript/cscr_stringlist.h>
#include <universal/com_memory.h>
#include <qcommon/com_profilemapload.h>
#include "xmodel_load_obj.h"
#include <qcommon/cm_tracebox.h>
#include "xmodel_utils.h"
#include "dobj_utils.h"
#include <cgame/cg_drawtools.h>

Material *g_materials[1];
XModelDefault g_default;

bool __cdecl XModelBad(const XModel *model)
{
    iassert(model);
    if ( useFastFile->current.enabled )
        return DB_IsXAssetDefault(ASSET_TYPE_XMODEL, model->name);
    else
        return model->bad;
}

void __cdecl XModelReleaseResources(XModel *model)
{
    unsigned int surfIndex; // [esp+0h] [ebp-4h]

    if ( !model && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel.cpp", 84, 0, "%s", "model") )
        __debugbreak();
    for ( surfIndex = 0; surfIndex < model->numsurfs; ++surfIndex )
        XSurfaceReleaseResources(&model->surfs[surfIndex]);
}

void __cdecl XModelPartsFree(XModelPartsLoad *modelParts)
{
    int size; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    unsigned __int16 *boneNames; // [esp+8h] [ebp-4h]

    if ( !modelParts
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel.cpp", 100, 0, "%s", "modelParts") )
    {
        __debugbreak();
    }
    boneNames = modelParts->boneNames;
    size = modelParts->numBones;
    for ( i = 0; i < size; ++i )
        SL_RemoveRefToString(SCRIPTINSTANCE_SERVER, boneNames[i]);
}

XModelPartsLoad *__cdecl XModelPartsFindData(const char *name)
{
    return (XModelPartsLoad *)Hunk_FindDataForFile(4, name);
}

void __cdecl XModelPartsSetData(const char *name, XModelPartsLoad *modelParts, void *(__cdecl *Alloc)(int))
{
    Hunk_SetDataForFile(4, name, modelParts, Alloc);
}

XModelSurfs *__cdecl XModelSurfsFindData(const char *name)
{
    return (XModelSurfs *)Hunk_FindDataForFile(3, name);
}

void __cdecl XModelSurfsSetData(const char *name, XModelSurfs *modelSurfs, void *(__cdecl *Alloc)(int))
{
    Hunk_SetDataForFile(3, name, modelSurfs, Alloc);
}

XModel *__cdecl XModelPrecache(char *name, void *(__cdecl *Alloc)(int), void *(__cdecl *AllocColl)(int))
{
    if ( useFastFile->current.enabled )
        return (XModel *)((int (__cdecl *)(char *, void *(__cdecl *)(int), void *(__cdecl *)(int)))XModelPrecache_FastFile)(
                                             name,
                                             Alloc,
                                             AllocColl);
    else
        return XModelPrecache_LoadObj(name, Alloc, AllocColl);
}

XModel *__cdecl XModelPrecache_LoadObj(char *name, void *(__cdecl *Alloc)(int), void *(__cdecl *AllocColl)(int))
{
    XModel *model; // [esp+0h] [ebp-4h]
    XModel *modela; // [esp+0h] [ebp-4h]

    model = (XModel *)Hunk_FindDataForFile(5, name);
    if ( model )
        return model;
    ProfLoad_Begin("Load xmodel");
    modela = XModelLoad(name, Alloc, AllocColl);
    ProfLoad_End();
    if ( modela )
    {
        modela->name = Hunk_SetDataForFile(5, name, modela, Alloc);
        return modela;
    }
    else
    {
        Com_PrintError(19, "ERROR: Cannot find xmodel '%s'.\n", name);
        return XModelDefaultModel(name, Alloc);
    }
}

XModel *__cdecl XModelDefaultModel(const char *name, void *(__cdecl *Alloc)(int))
{
    XModel *model; // [esp+0h] [ebp-4h]

    model = XModelCreateDefault(Alloc);
    Hunk_SetDataForFile(5, name, model, Alloc);
    return model;
}

XModel *__cdecl XModelCreateDefault(void *(__cdecl *Alloc)(int))
{
    XModel *model; // [esp+0h] [ebp-4h]

    model = (XModel *)Alloc(380);
    XModelMakeDefault(model);
    return model;
}

void __cdecl XModelMakeDefault(XModel *model)
{
    const XModelPartsLoad *DefaultParts; // eax

    model->bad = 1;
    DefaultParts = XModelCreateDefaultParts();
    XModelCopyXModelParts(DefaultParts, model);
    memset(model->lodInfo, 0, sizeof(model->lodInfo));
    model->numLods = 1;
    model->collLod = 0;
    model->name = "DEFAULT";
    model->surfs = 0;
    model->materialHandles = g_materials;
    g_materials[0] = Material_RegisterHandle("mc/$default", 8);
    g_default.boneInfo.bounds[0][0] = -16.0f;
    g_default.boneInfo.bounds[0][1] = -16.0f;
    g_default.boneInfo.bounds[0][2] = -16.0f;
    g_default.boneInfo.bounds[1][0] = 16.0f;
    g_default.boneInfo.bounds[1][1] = 16.0f;
    g_default.boneInfo.bounds[1][2] = 16.0f;
    model->localBoneInfo = &g_default.boneInfo;
}

XModelPartsLoad *__cdecl XModelCreateDefaultParts()
{
    g_default.modelParts.parentList = g_default.parentList;
    g_default.modelParts.boneNames = (unsigned __int16 *)&g_default;
    g_default.modelParts.quats = 0;
    g_default.modelParts.trans = 0;
    g_default.modelParts.numBones = 1;
    g_default.modelParts.numRootBones = 1;
    g_default.modelParts.partClassification = g_default.partClassification;
    g_default.partClassification[0] = 0;
    g_default.boneNames[0] = 0;
    return &g_default.modelParts;
}

XModel *__cdecl XModelPrecache_FastFile(const char *name)
{
    return DB_FindXAssetHeader(ASSET_TYPE_XMODEL, (char*)name, 1, -1).model;
}

unsigned __int16 *__cdecl XModelBoneNames(XModel *model)
{
    return model->localBoneNames;
}

double __cdecl XModelGetRadius(const XModel *model)
{
    return model->radius;
}

void __cdecl XModelGetBounds(const XModel *model, float *mins, float *maxs)
{
    *mins = model->mins[0];
    mins[1] = model->mins[1];
    mins[2] = model->mins[2];
    *maxs = model->maxs[0];
    maxs[1] = model->maxs[1];
    maxs[2] = model->maxs[2];
}

int __cdecl XModelGetMemUsage(const XModel *model)
{
    return model->memUsage;
}

void __cdecl XModelRenderString(const float *pos, char *string)
{
    XSurfaceRenderString(pos, string);
}

int __cdecl XModelTraceLine(
                const XModel *model,
                trace_t *results,
                const float *localStart,
                const float *localEnd,
                int contentmask)
{
    float v6; // [esp+8h] [ebp-A8h]
    int v7; // [esp+2Ch] [ebp-84h]
    float endDist; // [esp+34h] [ebp-7Ch]
    int j; // [esp+38h] [ebp-78h]
    TraceExtents boneExtents; // [esp+3Ch] [ebp-74h] BYREF
    float t; // [esp+70h] [ebp-40h]
    float delta[3]; // [esp+78h] [ebp-38h]
    float hitFrac; // [esp+84h] [ebp-2Ch]
    const XModelCollTri_s *ctri; // [esp+88h] [ebp-28h]
    float frac; // [esp+8Ch] [ebp-24h]
    int partIndex; // [esp+90h] [ebp-20h]
    float startDist; // [esp+94h] [ebp-1Ch]
    const XModelCollSurf_s *csurf; // [esp+98h] [ebp-18h]
    float hit[3]; // [esp+9Ch] [ebp-14h]
    float s; // [esp+A8h] [ebp-8h]
    unsigned int i; // [esp+ACh] [ebp-4h]

    PROF_SCOPED("xmodeltraceline");

    //TraceExtents::TraceExtents(&boneExtents);
    if ( model->collLod >= 0 )
    {
        partIndex = -1;
        *(_QWORD *)boneExtents.start.vec.v = *(_QWORD *)localStart;
        boneExtents.start.vec.v[2] = localStart[2];
        *(_QWORD *)boneExtents.end.vec.v = *(_QWORD *)localEnd;
        boneExtents.end.vec.v[2] = localEnd[2];
        CM_CalcTraceExtents(&boneExtents);
        delta[0] = boneExtents.end.vec.v[0] - boneExtents.start.vec.v[0];
        delta[1] = boneExtents.end.vec.v[1] - boneExtents.start.vec.v[1];
        delta[2] = boneExtents.end.vec.v[2] - boneExtents.start.vec.v[2];
        for ( i = 0; i < model->numCollSurfs; ++i )
        {
            csurf = &model->collSurfs[i];
            if ( (contentmask & csurf->contents) != 0
                && intersect_extents_aabb(&boneExtents, csurf->mins, csurf->maxs, results->fraction) )
            {
                for ( j = 0; j < csurf->numCollTris; ++j )
                {
                    ctri = &csurf->collTris[j];
                    endDist = (float)((float)((float)(boneExtents.end.vec.v[0] * ctri->plane[0])
                                                                    + (float)(boneExtents.end.vec.v[1] * ctri->plane[1]))
                                                    + (float)(boneExtents.end.vec.v[2] * ctri->plane[2]))
                                    - ctri->plane[3];
                    if ( endDist < 0.0 )
                    {
                        startDist = (float)((float)((float)(boneExtents.start.vec.v[0] * ctri->plane[0])
                                                                            + (float)(boneExtents.start.vec.v[1] * ctri->plane[1]))
                                                            + (float)(boneExtents.start.vec.v[2] * ctri->plane[2]))
                                            - ctri->plane[3];
                        if ( startDist > 0.0 )
                        {
                            frac = (float)(startDist - 0.125) / (float)(startDist - endDist);
                            v6 = (float)(frac - 0.0) < 0.0 ? 0.0f : frac;
                            frac = v6;
                            if ( v6 < results->fraction )
                            {
                                hitFrac = startDist / (float)(startDist - endDist);
                                hit[0] = (float)(hitFrac * delta[0]) + boneExtents.start.vec.v[0];
                                hit[1] = (float)(hitFrac * delta[1]) + boneExtents.start.vec.v[1];
                                hit[2] = (float)(hitFrac * delta[2]) + boneExtents.start.vec.v[2];
                                s = (float)((float)((float)(hit[0] * ctri->svec[0]) + (float)(hit[1] * ctri->svec[1]))
                                                    + (float)(hit[2] * ctri->svec[2]))
                                    - ctri->svec[3];
                                if ( s >= -0.001 && s <= 1.001 )
                                {
                                    t = (float)((float)((float)(hit[0] * ctri->tvec[0]) + (float)(hit[1] * ctri->tvec[1]))
                                                        + (float)(hit[2] * ctri->tvec[2]))
                                        - ctri->tvec[3];
                                    if ( t >= -0.001 && (float)(s + t) <= 1.001 )
                                    {
                                        partIndex = csurf->boneIdx;
                                        results->startsolid = 0;
                                        results->allsolid = 0;
                                        results->fraction = frac;
                                        if ( (results->fraction < 0.0 || results->fraction > 1.0)
                                            && !Assert_MyHandler(
                                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel.cpp",
                                                        687,
                                                        1,
                                                        "%s\n\t(results->fraction) = %g",
                                                        "(results->fraction >= 0 && results->fraction <= 1.0f)",
                                                        results->fraction) )
                                        {
                                            __debugbreak();
                                        }
                                        results->sflags = csurf->surfFlags;
                                        results->cflags = csurf->contents;
                                        results->normal.vec.v[0] = ctri->plane[0];
                                        results->normal.vec.v[1] = ctri->plane[1];
                                        results->normal.vec.v[2] = ctri->plane[2];
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        v7 = partIndex;
        return v7;
    }
    else
    {
        return -1;
    }
}

int __cdecl XModelTraceLineAnimated(
                const DObj *obj,
                unsigned int modelIndex,
                int baseBoneIndex,
                trace_t *results,
                const DObjAnimMat *boneMtxList,
                float *localStart,
                float *localEnd,
                int contentmask)
{
    double v9; // st7
    double v10; // st7
    double v11; // st7
    float v12; // [esp+Ch] [ebp-21Ch]
    int v13; // [esp+10h] [ebp-218h]
    bool v14; // [esp+14h] [ebp-214h]
    int v15; // [esp+D0h] [ebp-158h]
    float endDist; // [esp+D8h] [ebp-150h]
    int j; // [esp+DCh] [ebp-14Ch]
    TraceExtents boneExtents; // [esp+E0h] [ebp-148h] BYREF
    const DObjAnimMat *baseMatList; // [esp+114h] [ebp-114h]
    float t; // [esp+118h] [ebp-110h]
    float delta[3]; // [esp+120h] [ebp-108h] BYREF
    float invBaseMat[4][3]; // [esp+12Ch] [ebp-FCh] BYREF
    float hitFrac; // [esp+15Ch] [ebp-CCh]
    float localStart2[3]; // [esp+160h] [ebp-C8h] BYREF
    const XModelCollTri_s *ctri; // [esp+16Ch] [ebp-BCh]
    float frac; // [esp+170h] [ebp-B8h]
    const XModel *model; // [esp+174h] [ebp-B4h]
    int partIndex; // [esp+178h] [ebp-B0h]
    float startDist; // [esp+17Ch] [ebp-ACh]
    const XModelCollSurf_s *csurf; // [esp+180h] [ebp-A8h]
    float localEnd2[3]; // [esp+184h] [ebp-A4h] BYREF
    int globalBoneIndex; // [esp+190h] [ebp-98h]
    float hit[3]; // [esp+194h] [ebp-94h] BYREF
    float mat[4][3]; // [esp+1A0h] [ebp-88h] BYREF
    unsigned int hidePartBits[5]; // [esp+1D0h] [ebp-58h] BYREF
    const DObjAnimMat *baseMat; // [esp+1E4h] [ebp-44h]
    float s; // [esp+1E8h] [ebp-40h]
    unsigned int i; // [esp+1ECh] [ebp-3Ch]
    int boneIdx; // [esp+1F0h] [ebp-38h]
    float axis[4][3]; // [esp+1F4h] [ebp-34h] BYREF
    const DObjAnimMat *boneMtx; // [esp+224h] [ebp-4h]

    PROF_SCOPED("xmodeltracelineanimated");

    //TraceExtents::TraceExtents(&boneExtents);
    model = obj->localModels[modelIndex];
    if ( model->collLod >= 0 )
    {
        partIndex = -1;
        baseMatList = XModelGetBasePose(model);
        DObjGetHidePartBits(obj, hidePartBits);
        for ( i = 0; i < model->numCollSurfs; ++i )
        {
            csurf = &model->collSurfs[i];
            if ( (contentmask & csurf->contents) != 0 )
            {
                boneIdx = csurf->boneIdx;
                globalBoneIndex = boneIdx + baseBoneIndex;
                if ( (hidePartBits[(boneIdx + baseBoneIndex) >> 5] & (0x80000000 >> ((boneIdx + baseBoneIndex) & 0x1F))) == 0 )
                {
                    boneMtx = &boneMtxList[boneIdx];
                    baseMat = &baseMatList[boneIdx];
                    v14 = boneMtx->quat[0] == baseMat->quat[0]
                         && boneMtx->quat[1] == baseMat->quat[1]
                         && boneMtx->quat[2] == baseMat->quat[2]
                         && boneMtx->quat[3] == baseMat->quat[3];
                    if ( v14
                        && (boneMtx->trans[0] != baseMat->trans[0]
                         || boneMtx->trans[1] != baseMat->trans[1]
                         || boneMtx->trans[2] != baseMat->trans[2]
                            ? (v13 = 0)
                            : (v13 = 1),
                                v13) )
                    {
                        *(_QWORD *)boneExtents.start.vec.v = *(_QWORD *)localStart;
                        boneExtents.start.vec.v[2] = localStart[2];
                        *(_QWORD *)boneExtents.end.vec.v = *(_QWORD *)localEnd;
                        boneExtents.end.vec.v[2] = localEnd[2];
                    }
                    else
                    {
                        ConvertQuatToInverseMat(baseMat, (float(*)[3])invBaseMat);
                        ConvertQuatToMat(boneMtx, (float(*)[3])mat);
                        mat[3][0] = boneMtx->trans[0];
                        mat[3][1] = boneMtx->trans[1];
                        mat[3][2] = boneMtx->trans[2];
                        MatrixMultiply43(invBaseMat, mat, axis);
                        localStart2[0] = *localStart - axis[3][0];
                        localStart2[1] = localStart[1] - axis[3][1];
                        localStart2[2] = localStart[2] - axis[3][2];
                        localEnd2[0] = *localEnd - axis[3][0];
                        localEnd2[1] = localEnd[1] - axis[3][1];
                        localEnd2[2] = localEnd[2] - axis[3][2];
                        MatrixTransposeTransformVector(localStart2, axis, boneExtents.start.vec.v);
                        MatrixTransposeTransformVector(localEnd2, axis, boneExtents.end.vec.v);
                    }
                    CM_CalcTraceExtents(&boneExtents);
                    if ( intersect_extents_aabb(&boneExtents, csurf->mins, csurf->maxs, results->fraction) )
                    {
                        delta[0] = boneExtents.end.vec.v[0] - boneExtents.start.vec.v[0];
                        delta[1] = boneExtents.end.vec.v[1] - boneExtents.start.vec.v[1];
                        delta[2] = boneExtents.end.vec.v[2] - boneExtents.start.vec.v[2];
                        for ( j = 0; j < csurf->numCollTris; ++j )
                        {
                            ctri = &csurf->collTris[j];
                            endDist = (float)((float)((float)(boneExtents.end.vec.v[0] * ctri->plane[0])
                                                                            + (float)(boneExtents.end.vec.v[1] * ctri->plane[1]))
                                                            + (float)(boneExtents.end.vec.v[2] * ctri->plane[2]))
                                            - ctri->plane[3];
                            if ( endDist < 0.0 )
                            {
                                v9 = phys_dot((const phys_vec3 *)&boneExtents, (const phys_vec3 *)ctri);
                                startDist = v9 - ctri->plane[3];
                                if ( startDist > 0.0 )
                                {
                                    frac = (float)(startDist - 0.125) / (float)(startDist - endDist);
                                    v12 = (float)(frac - 0.0) < 0.0 ? 0.0f : frac;
                                    frac = v12;
                                    if ( v12 < results->fraction )
                                    {
                                        hitFrac = startDist / (float)(startDist - endDist);
                                        Vec3Mad(boneExtents.start.vec.v, hitFrac, delta, hit);
                                        v10 = phys_dot((const phys_vec3 *)hit, (const phys_vec3 *)ctri->svec);
                                        s = v10 - ctri->svec[3];
                                        if ( s >= -0.001 && s <= 1.001 )
                                        {
                                            v11 = phys_dot((const phys_vec3 *)hit, (const phys_vec3 *)ctri->tvec);
                                            t = v11 - ctri->tvec[3];
                                            if ( t >= -0.001 && (float)(s + t) <= 1.001 )
                                            {
                                                partIndex = csurf->boneIdx;
                                                results->startsolid = 0;
                                                results->allsolid = 0;
                                                results->fraction = frac;
                                                if ( (results->fraction < 0.0 || results->fraction > 1.0)
                                                    && !Assert_MyHandler(
                                                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel.cpp",
                                                                928,
                                                                1,
                                                                "%s\n\t(results->fraction) = %g",
                                                                "(results->fraction >= 0 && results->fraction <= 1.0f)",
                                                                results->fraction) )
                                                {
                                                    __debugbreak();
                                                }
                                                results->sflags = csurf->surfFlags;
                                                results->cflags = csurf->contents;
                                                Vec3Copy(ctri->plane, results->normal.vec.v);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        v15 = partIndex;
        return v15;
    }
    else
    {
        return -1;
    }
}

void __cdecl ConvertQuatToInverseMat(const DObjAnimMat *mat, float (*axis)[3])
{
    float transWeight; // [esp+0h] [ebp-48h]
    float yy; // [esp+18h] [ebp-30h]
    float xy; // [esp+1Ch] [ebp-2Ch]
    float zz; // [esp+20h] [ebp-28h]
    float zw; // [esp+24h] [ebp-24h]
    float scaledQuat[3]; // [esp+28h] [ebp-20h]
    float yw; // [esp+34h] [ebp-14h]
    float xz; // [esp+38h] [ebp-10h]
    float yz; // [esp+3Ch] [ebp-Ch]
    float xx; // [esp+40h] [ebp-8h]
    float xw; // [esp+44h] [ebp-4h]

    if ( ((LODWORD(mat->quat[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(mat->quat[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(mat->quat[2]) & 0x7F800000) == 0x7F800000
         || (LODWORD(mat->quat[3]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h",
                    1601,
                    0,
                    "%s",
                    "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])") )
    {
        __debugbreak();
    }
    if ( (LODWORD(mat->transWeight) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h",
                    1602,
                    0,
                    "%s",
                    "!IS_NAN(mat->transWeight)") )
    {
        __debugbreak();
    }
    transWeight = mat->transWeight;
    scaledQuat[0] = transWeight * mat->quat[0];
    scaledQuat[1] = transWeight * mat->quat[1];
    scaledQuat[2] = transWeight * mat->quat[2];
    xx = scaledQuat[0] * mat->quat[0];
    xy = scaledQuat[0] * mat->quat[1];
    xz = scaledQuat[0] * mat->quat[2];
    xw = scaledQuat[0] * mat->quat[3];
    yy = scaledQuat[1] * mat->quat[1];
    yz = scaledQuat[1] * mat->quat[2];
    yw = scaledQuat[1] * mat->quat[3];
    zz = scaledQuat[2] * mat->quat[2];
    zw = scaledQuat[2] * mat->quat[3];
    (*axis)[0] = 1.0 - (float)(yy + zz);
    (*axis)[1] = xy - zw;
    (*axis)[2] = xz + yw;
    (*axis)[3] = xy + zw;
    (*axis)[4] = 1.0 - (float)(xx + zz);
    (*axis)[5] = yz - xw;
    (*axis)[6] = xz - yw;
    (*axis)[7] = yz + xw;
    (*axis)[8] = 1.0 - (float)(xx + yy);
    ((*axis)[9]) =  -((float)((float)(mat->trans[0] * (*axis)[0]) + (float)(mat->trans[1] * (*axis)[3])) + (float)(mat->trans[2] * (*axis)[6]));
    ((*axis)[10]) = -((float)((float)(mat->trans[0] * (*axis)[1]) + (float)(mat->trans[1] * (*axis)[4])) + (float)(mat->trans[2] * (*axis)[7]));
    ((*axis)[11]) = -((float)((float)(mat->trans[0] * (*axis)[2]) + (float)(mat->trans[1] * (*axis)[5])) + (float)(mat->trans[2] * (*axis)[8]));
}

void __cdecl XModelTraceLineAnimatedPartBits(
                const DObj *obj,
                unsigned int modelIndex,
                int baseBoneIndex,
                int contentmask,
                int *partBits)
{
    const XModel *model; // [esp+0h] [ebp-28h]
    const XModelCollSurf_s *csurf; // [esp+4h] [ebp-24h]
    unsigned int hidePartBits[5]; // [esp+Ch] [ebp-1Ch] BYREF
    unsigned int i; // [esp+20h] [ebp-8h]
    int boneIdx; // [esp+24h] [ebp-4h]

    model = obj->localModels[modelIndex];
    if ( model->collLod >= 0 )
    {
        DObjGetHidePartBits(obj, hidePartBits);
        for ( i = 0; i < model->numCollSurfs; ++i )
        {
            csurf = &model->collSurfs[i];
            if ( (contentmask & csurf->contents) != 0 )
            {
                boneIdx = csurf->boneIdx;
                if ( (hidePartBits[(boneIdx + baseBoneIndex) >> 5] & (0x80000000 >> ((boneIdx + baseBoneIndex) & 0x1F))) == 0 )
                    partBits[(boneIdx + baseBoneIndex) >> 5] |= 0x80000000 >> ((boneIdx + baseBoneIndex) & 0x1F);
            }
        }
    }
}

bool __cdecl XSurfaceVisitTrianglesInAabb(
                const XSurface *surface,
                unsigned int vertListIndex,
                const float *aabbMins,
                const float *aabbMaxs,
                bool (__cdecl *visitorFunc)(void *, const unsigned __int8 **, const unsigned __int8 **),
                void *visitorContext)
{
    XSurfaceGetTriCandidatesLocals locals; // [esp+80h] [ebp-2B8h] BYREF
    XRigidVertList *vertList; // [esp+334h] [ebp-4h]

    if ( vertListIndex >= surface->vertListCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel.cpp",
                    1352,
                    0,
                    "%s\n\t(vertListIndex) = %i",
                    "(vertListIndex >= 0 && vertListIndex < surface->vertListCount)",
                    vertListIndex) )
    {
        __debugbreak();
    }
    vertList = &surface->vertList[vertListIndex];
    locals.tree = vertList->collisionTree;
    if ( !locals.tree
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel.cpp", 1355, 0, "%s", "locals.tree") )
    {
        __debugbreak();
    }
    PrefetchArrayIndexed((char *)locals.tree->nodes, 1u, 0, 0x10u);
    locals.visitorFunc = visitorFunc;
    locals.visitorContext = visitorContext;
    locals.inIndices = surface->triIndices;
    locals.inVertices0 = (const unsigned __int8 *)surface->verts0;
    locals.inVertices1 = (const unsigned __int8 *)surface->verts0;
    XSurfaceVisitTrianglesInAabb_ConvertAabb(locals.tree, aabbMins, aabbMaxs, locals.mins, locals.maxs);
    locals.nodeQueue[0].count = 1;
    locals.nodeQueueBegin = 0;
    locals.nodeQueueEnd = 1;
    memset(&locals.leafQueueBegin, 0, 28);
    locals.surfaceFlags = surface->flags;
    locals.vertexSize0 = 32;
    while ( locals.nodeQueueBegin != locals.nodeQueueEnd )
    {
        if ( !XSurfaceVisitTrianglesInAabb_ProcessNode(&locals) )
            return 0;
    }
    while ( locals.leafQueueBegin != locals.leafQueueEnd )
    {
        if ( !XSurfaceVisitTrianglesInAabb_ProcessLeaf(&locals) )
            return 0;
    }
    while ( locals.triangleQueueBegin != locals.triangleQueueEnd )
    {
        if ( !XSurfaceVisitTrianglesInAabb_ProcessTriangles(&locals) )
            return 0;
    }
    while ( locals.vertexQueueBegin != locals.vertexQueueEnd )
    {
        if ( !(unsigned __int8)XSurfaceVisitTrianglesInAabb_ProcessVertices(&locals) )
            return 0;
    }
    return 1;
}

void __cdecl XSurfaceVisitTrianglesInAabb_ConvertAabb(
                const XSurfaceCollisionTree *tree,
                const float *aabbMins,
                const float *aabbMaxs,
                int *mins,
                int *maxs)
{
    float v5; // [esp+60h] [ebp-68h]
    float v6; // [esp+64h] [ebp-64h]
    float v7; // [esp+68h] [ebp-60h]
    float v8; // [esp+6Ch] [ebp-5Ch]
    float v9; // [esp+70h] [ebp-58h]
    float v10; // [esp+74h] [ebp-54h]
    float v11; // [esp+78h] [ebp-50h]
    float v12; // [esp+7Ch] [ebp-4Ch]
    float v13; // [esp+80h] [ebp-48h]
    float v14; // [esp+84h] [ebp-44h]
    float v15; // [esp+88h] [ebp-40h]
    float v16; // [esp+8Ch] [ebp-3Ch]
    float transformedMins_4; // [esp+B4h] [ebp-14h]
    float transformedMins_8; // [esp+B8h] [ebp-10h]
    float transformedMaxs; // [esp+BCh] [ebp-Ch]
    float transformedMaxs_4; // [esp+C0h] [ebp-8h]
    float transformedMaxs_8; // [esp+C4h] [ebp-4h]

    transformedMins_4 = (float)(aabbMins[1] + tree->trans[1]) * tree->scale[1];
    transformedMins_8 = (float)(aabbMins[2] + tree->trans[2]) * tree->scale[2];
    transformedMaxs = (float)(*aabbMaxs + tree->trans[0]) * tree->scale[0];
    transformedMaxs_4 = (float)(aabbMaxs[1] + tree->trans[1]) * tree->scale[1];
    transformedMaxs_8 = (float)(aabbMaxs[2] + tree->trans[2]) * tree->scale[2];
    v16 = (float)((float)(*aabbMins + tree->trans[0]) * tree->scale[0]) - 0.5;
    if ( 1000000.0 <= -1000000.0
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 730, 0, "%s", "min < max") )
    {
        __debugbreak();
    }
    if ( v16 >= -1000000.0 )
    {
        if ( v16 <= 1000000.0 )
            v15 = v16;
        else
            v15 = 1000000.0f;
    }
    else
    {
        v15 = -1000000.0;
    }
    *mins = (__int64)v15;
    v14 = transformedMins_4 - 0.5;
    if ( 1000000.0 <= -1000000.0
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 730, 0, "%s", "min < max") )
    {
        __debugbreak();
    }
    if ( v14 >= -1000000.0 )
    {
        if ( v14 <= 1000000.0 )
            v13 = transformedMins_4 - 0.5;
        else
            v13 = 1000000.0f;
    }
    else
    {
        v13 = -1000000.0;
    }
    mins[1] = (__int64)v13;
    v12 = transformedMins_8 - 0.5;
    if ( 1000000.0 <= -1000000.0
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 730, 0, "%s", "min < max") )
    {
        __debugbreak();
    }
    if ( v12 >= -1000000.0 )
    {
        if ( v12 <= 1000000.0 )
            v11 = transformedMins_8 - 0.5;
        else
            v11 = 1000000.0f;
    }
    else
    {
        v11 = -1000000.0;
    }
    mins[2] = (__int64)v11;
    v10 = transformedMaxs + 0.5;
    if ( 1000000.0 <= -1000000.0
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 730, 0, "%s", "min < max") )
    {
        __debugbreak();
    }
    if ( v10 >= -1000000.0 )
    {
        if ( v10 <= 1000000.0 )
            v9 = transformedMaxs + 0.5;
        else
            v9 = 1000000.0f;
    }
    else
    {
        v9 = -1000000.0;
    }
    *maxs = (__int64)v9;
    v8 = transformedMaxs_4 + 0.5;
    if ( 1000000.0 <= -1000000.0
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 730, 0, "%s", "min < max") )
    {
        __debugbreak();
    }
    if ( v8 >= -1000000.0 )
    {
        if ( v8 <= 1000000.0 )
            v7 = transformedMaxs_4 + 0.5;
        else
            v7 = 1000000.0f;
    }
    else
    {
        v7 = -1000000.0;
    }
    maxs[1] = (__int64)v7;
    v6 = transformedMaxs_8 + 0.5;
    if ( 1000000.0 <= -1000000.0
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 730, 0, "%s", "min < max") )
    {
        __debugbreak();
    }
    if ( v6 >= -1000000.0 )
    {
        if ( v6 <= 1000000.0 )
            v5 = transformedMaxs_8 + 0.5;
        else
            v5 = 1000000.0f;
    }
    else
    {
        v5 = -1000000.0;
    }
    maxs[2] = (__int64)v5;
}

void __cdecl PrefetchArrayIndexed(char *mem, unsigned int elementCount, unsigned int index, unsigned int elementSize)
{
    unsigned int umem; // [esp+4h] [ebp-Ch]
    const unsigned __int8 *memIter; // [esp+8h] [ebp-8h]

    umem = (unsigned int)&mem[elementSize * index];
    if ( !elementCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel.cpp", 1095, 0, "%s", "elementCount") )
    {
        __debugbreak();
    }
    for ( memIter = (const unsigned __int8 *)(umem & 0xFFFFFF80);
                (unsigned int)memIter <= ((umem + elementSize * elementCount - 1) & 0xFFFFFF80);
                memIter += 128 )
    {
        _mm_prefetch((const char *)memIter, 1);
    }
}

int __cdecl XSurfaceVisitTrianglesInAabb_ProcessVertices(XSurfaceGetTriCandidatesLocals *locals)
{
    GfxPackedVertex *pv0; // [esp+0h] [ebp-30h]
    GfxPackedVertex *pv1; // [esp+4h] [ebp-2Ch]
    const unsigned __int8 *verts1[3]; // [esp+8h] [ebp-28h] BYREF
    unsigned __int16 indices[4]; // [esp+14h] [ebp-1Ch]
    const unsigned __int8 *verts0[3]; // [esp+20h] [ebp-10h] BYREF
    unsigned int vertIter; // [esp+2Ch] [ebp-4h]

    for ( vertIter = 0; vertIter < 3; ++vertIter )
        indices[vertIter] = locals->vertexQueue[locals->vertexQueueBegin][vertIter];
    pv0 = (GfxPackedVertex *)locals->inVertices0;
    pv1 = (GfxPackedVertex *)locals->inVertices1;
    for ( vertIter = 0; vertIter != 3; ++vertIter )
    {
        verts0[vertIter] = (const unsigned __int8 *)&pv0[indices[vertIter]];
        verts1[vertIter] = (const unsigned __int8 *)&pv1[indices[vertIter]].normal;
    }
    locals->vertexQueueBegin = ((unsigned __int8)locals->vertexQueueBegin + 1) & 3;
    return ((int (__cdecl *)(void *, const unsigned __int8 **, const unsigned __int8 **))locals->visitorFunc)(
                     locals->visitorContext,
                     verts0,
                     verts1);
}

char __cdecl XSurfaceVisitTrianglesInAabb_ProcessTriangles(XSurfaceGetTriCandidatesLocals *locals)
{
    unsigned __int16 index; // [esp+3Ch] [ebp-18h]
    unsigned int vertIter; // [esp+40h] [ebp-14h]
    unsigned int triangleIter; // [esp+44h] [ebp-10h]
    const unsigned __int16 *indexPtr; // [esp+48h] [ebp-Ch]
    unsigned int triangleCount; // [esp+4Ch] [ebp-8h]
    unsigned int triangleBegin; // [esp+50h] [ebp-4h]

    triangleBegin = locals->triangleQueue[locals->triangleQueueBegin].beginIndex;
    triangleCount = locals->triangleQueue[locals->triangleQueueBegin].count;
    locals->triangleQueueBegin = ((unsigned __int8)locals->triangleQueueBegin + 1) & 3;
    indexPtr = &locals->inIndices[3 * triangleBegin];
    for ( triangleIter = 0; triangleIter != triangleCount; ++triangleIter )
    {
        for ( vertIter = 0; vertIter != 3; ++vertIter )
        {
            index = *indexPtr;
            PrefetchArrayIndexed((char *)locals->inVertices0, 1u, *indexPtr++, locals->vertexSize0);
            locals->vertexQueue[locals->vertexQueueEnd][vertIter] = index;
        }
        locals->vertexQueueEnd = ((unsigned __int8)locals->vertexQueueEnd + 1) & 3;
        if ( locals->vertexQueueBegin == locals->vertexQueueEnd
            && !(unsigned __int8)XSurfaceVisitTrianglesInAabb_ProcessVertices(locals) )
        {
            return 0;
        }
    }
    return 1;
}

char __cdecl XSurfaceVisitTrianglesInAabb_ProcessLeaf(XSurfaceGetTriCandidatesLocals *locals)
{
    XSurfaceCollisionLeaf *leaf; // [esp+64h] [ebp-18h]
    unsigned int indexCount; // [esp+68h] [ebp-14h]
    int indexBeginIndex; // [esp+6Ch] [ebp-10h]
    unsigned int leafBeginIndex; // [esp+70h] [ebp-Ch]
    unsigned int leafIndex; // [esp+74h] [ebp-8h]
    unsigned int leafEndIndex; // [esp+78h] [ebp-4h]

    leafBeginIndex = locals->leafQueue[locals->leafQueueBegin].beginIndex;
    leafEndIndex = locals->leafQueue[locals->leafQueueBegin].count + leafBeginIndex;
    locals->leafQueueBegin = ((unsigned __int8)locals->leafQueueBegin + 1) & 3;
    for ( leafIndex = leafBeginIndex; leafIndex != leafEndIndex; ++leafIndex )
    {
        leaf = &locals->tree->leafs[leafIndex];
        if ( leaf->triangleBeginIndex < 0x8000u )
        {
            indexBeginIndex = leaf->triangleBeginIndex;
            indexCount = 1;
        }
        else
        {
            indexBeginIndex = leaf->triangleBeginIndex - 0x8000;
            indexCount = 2;
        }
        PrefetchArrayIndexed((char *)&locals->inIndices[3 * indexBeginIndex], 3 * indexCount, 0, 2u);
        locals->triangleQueue[locals->triangleQueueEnd].beginIndex = indexBeginIndex;
        locals->triangleQueue[locals->triangleQueueEnd].count = indexCount;
        locals->triangleQueueEnd = ((unsigned __int8)locals->triangleQueueEnd + 1) & 3;
        if ( locals->triangleQueueBegin == locals->triangleQueueEnd
            && !XSurfaceVisitTrianglesInAabb_ProcessTriangles(locals) )
        {
            return 0;
        }
    }
    return 1;
}

char __cdecl XSurfaceVisitTrianglesInAabb_ProcessNode(XSurfaceGetTriCandidatesLocals *locals)
{
    unsigned int childBeginIndex; // [esp+60h] [ebp-1Ch]
    unsigned int childCount; // [esp+64h] [ebp-18h]
    unsigned int childCounta; // [esp+64h] [ebp-18h]
    XSurfaceCollisionNode *node; // [esp+68h] [ebp-14h]
    unsigned int nodeIndex; // [esp+70h] [ebp-Ch]
    unsigned int nodeEndIndex; // [esp+74h] [ebp-8h]
    unsigned int nodeBeginIndex; // [esp+78h] [ebp-4h]

    nodeBeginIndex = locals->nodeQueue[locals->nodeQueueBegin].beginIndex;
    nodeEndIndex = locals->nodeQueue[locals->nodeQueueBegin].count + nodeBeginIndex;
    locals->nodeQueueBegin = ((unsigned __int8)locals->nodeQueueBegin + 1) & 0x3F;
    for ( nodeIndex = nodeBeginIndex; nodeIndex != nodeEndIndex; ++nodeIndex )
    {
        node = &locals->tree->nodes[nodeIndex];
        if ( locals->maxs[0] >= node->aabb.mins[0]
            && locals->mins[0] <= node->aabb.maxs[0]
            && locals->maxs[1] >= node->aabb.mins[1]
            && locals->mins[1] <= node->aabb.maxs[1]
            && locals->maxs[2] >= node->aabb.mins[2]
            && locals->mins[2] <= node->aabb.maxs[2] )
        {
            childBeginIndex = node->childBeginIndex;
            childCount = node->childCount;
            if ( childCount < 0x8000 )
            {
                PrefetchArrayIndexed((char *)&locals->tree->nodes[childBeginIndex], childCount, 0, 0x10u);
                locals->nodeQueue[locals->nodeQueueEnd].beginIndex = childBeginIndex;
                locals->nodeQueue[locals->nodeQueueEnd].count = childCount;
                locals->nodeQueueEnd = ((unsigned __int8)locals->nodeQueueEnd + 1) & 0x3F;
                if ( locals->nodeQueueBegin == locals->nodeQueueEnd )
                    return 0;
            }
            else
            {
                childCounta = childCount - 0x8000;
                PrefetchArrayIndexed((char *)&locals->tree->leafs[childBeginIndex], childCounta, 0, 2u);
                locals->leafQueue[locals->leafQueueEnd].beginIndex = childBeginIndex;
                locals->leafQueue[locals->leafQueueEnd].count = childCounta;
                locals->leafQueueEnd = ((unsigned __int8)locals->leafQueueEnd + 1) & 3;
                if ( locals->leafQueueBegin == locals->leafQueueEnd && !XSurfaceVisitTrianglesInAabb_ProcessLeaf(locals) )
                    return 0;
            }
        }
    }
    return 1;
}

