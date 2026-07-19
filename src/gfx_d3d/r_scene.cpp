#include "r_scene.h"

#include <Windows.h>
#include "r_dvars.h"
#include "r_warn.h"
#include "r_stream.h"
#include <universal/com_math_anglevectors.h>
#include <qcommon/threads.h>
#include <xanim/dobj_utils.h>
#include <xanim/xmodel.h>
#include "r_shader_constant_set.h"
#include <cgame_mp/cg_ents_mp.h>
#include <ragdoll/ragdoll_controller.h>
#include "r_drawsurf.h"
#include "r_light.h"
#include "r_draw_method.h"
#include "r_dobj_skin.h"
#include <xanim/xmodel_utils.h>
#include "r_xsurface.h"
#include <win32/win_shared.h>
#include <win32/win_net.h>
#include <client/cl_compositing.h>
#include <EffectsCore/fx_marks.h>
#include "r_state.h"
#include <EffectsCore/fx_beam.h>
#include <qcommon/dobj_management.h>
#include "rb_logfile.h"
#include "r_globalgfxdata.h"
#include "r_model_lighting.h"
#include "rb_fog.h"
#include <glass/glass_client.h>
#include <universal/com_workercmds.h>
#include "r_pretess.h"
#include "r_skybox.h"
#include "r_add_bsp.h"
#include <client/con_channels.h>
#include "r_sunshadow.h"
#include "r_add_staticmodel.h"
#include <DynEntity/DynEntity_pieces.h>
#include "r_spotshadow.h"
#include "r_foliage.h"
#include "r_water_sim.h"
#include <EffectsCore/fx_dvars.h>
#include <cgame/cg_compass.h>
#include "rb_postfx.h"
#include "r_ui3d.h"
#include "r_extracam.h"
#include "r_debug.h"
#include "r_cinematic.h"
#include "r_workercmds_common.h"
#include "r_draw_shadowablelight.h"
#include <DynEntity/DynEntity_client.h>
#include "r_primarylights.h"

GfxScene scene;
GfxViewParms lockPvsViewParms;

bool lastShowDebug;
float debugOrigin[3];
bool g_allowShadowMaps = true;

void __cdecl lerp_0(float (*dest)[4], const float (*from)[4], const float (*to)[4], float t)
{
    (*dest)[0] = Lerp((*from)[0], (*to)[0], t);
    (*dest)[1] = Lerp((*from)[1], (*to)[1], t);
    (*dest)[2] = Lerp((*from)[2], (*to)[2], t);
    (*dest)[3] = Lerp((*from)[3], (*to)[3], t);
}

void __cdecl lerp_1(float (*dest)[3], const float (*from)[3], const float (*to)[3], float t)
{
    (*dest)[0] = Lerp((*from)[0], (*to)[0], t);
    (*dest)[1] = Lerp((*from)[1], (*to)[1], t);
    (*dest)[2] = Lerp((*from)[2], (*to)[2], t);
}

GfxScene *__cdecl R_GetScene()
{
    return &scene;
}

unsigned int __cdecl R_AllocSceneModel()
{
    unsigned int sceneEntIndex; // [esp+0h] [ebp-4h]

    if ( !rg.registered
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 207, 0, "%s", "rg.registered") )
    {
        __debugbreak();
    }
    if ( !rg.inFrame
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 209, 0, "%s", "rg.inFrame") )
    {
        __debugbreak();
    }
    sceneEntIndex = _InterlockedExchangeAdd(&scene.sceneModelCount, 1u);
    if ( sceneEntIndex >= r_modelLimit->current.integer )
    {
        scene.sceneModelCount = r_modelLimit->current.integer;
        R_WarnOncePerFrame(R_WARN_KNOWN_MODELS, r_modelLimit->current.integer);
    }
    return sceneEntIndex;
}

int __cdecl R_AllocTextureOverride(
                unsigned __int16 modelIndexMask,
                const GfxImage *img1,
                GfxImage *img2,
                int prevOverride)
{
    return R_AllocTextureOverride(0, modelIndexMask, img1, img2, prevOverride);
}

int __cdecl R_AllocTextureOverride(
                const Material *material,
                unsigned __int16 modelIndexMask,
                const GfxImage *img1,
                GfxImage *img2,
                int prevOverride)
{
    __int16 texOver; // [esp+4h] [ebp-4h]

    if ( !img1 || !img2 || img1 == img2 )
        return prevOverride;
    if ( !R_StreamTouchImageAndCheck(img2, 0) )
        return prevOverride;
    texOver = _InterlockedExchangeAdd(&frontEndDataOut->textureOverrideCount, 1u);
    if ( (unsigned int)texOver < 0x100 )
    {
        frontEndDataOut->textureOverrides[texOver].material = material;
        frontEndDataOut->textureOverrides[texOver].img1 = img1;
        frontEndDataOut->textureOverrides[texOver].img2 = img2;
        frontEndDataOut->textureOverrides[texOver].dobjModelMask = modelIndexMask;
        frontEndDataOut->textureOverrides[texOver].prev = prevOverride;
        return texOver;
    }
    else
    {
        frontEndDataOut->textureOverrideCount = 256;
        R_WarnOncePerFrame(R_WARN_TEXTURE_OVERRIDES, 256);
        return -1;
    }
}

GfxBrushModel *__cdecl R_GetBrushModel(unsigned int modelIndex)
{
    if ( !rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 309, 0, "%s", "rgp.world") )
    {
        __debugbreak();
    }
    if ( modelIndex >= rgp.world->modelCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    310,
                    0,
                    "modelIndex doesn't index rgp.world->modelCount\n\t%i not in [0, %i)",
                    modelIndex,
                    rgp.world->modelCount) )
    {
        __debugbreak();
    }
    return &rgp.world->models[modelIndex];
}

void __cdecl R_AddBrushModelToSceneFromAngles(
                const GfxBrushModel *bmodel,
                const float *origin,
                const float *angles,
                unsigned __int16 entnum,
                const ShaderConstantSet *constants)
{
    unsigned int sceneEntIndex; // [esp+4h] [ebp-8h]
    GfxSceneBrush *sceneBrush; // [esp+8h] [ebp-4h]

    if ( !bmodel && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 319, 0, "%s", "bmodel") )
        __debugbreak();
    if ( r_drawEntities->current.enabled && r_drawBModels->current.enabled && bmodel->surfaceCount )
    {
        sceneEntIndex = R_AllocSceneBrush();
        if ( sceneEntIndex < 0x400 )
        {
            sceneBrush = &scene.sceneBrush[sceneEntIndex];
            sceneBrush->bmodel = bmodel;
            sceneBrush->entnum = entnum;
            sceneBrush->placement.origin[0] = *origin;
            sceneBrush->placement.origin[1] = origin[1];
            sceneBrush->placement.origin[2] = origin[2];
            AnglesToQuat(angles, sceneBrush->placement.quat);
            sceneBrush->brushConstantSet = r_shader_constant_set_enable->current.enabled ? constants : 0;
        }
    }
}

unsigned int __cdecl R_AllocSceneBrush()
{
    unsigned int sceneEntIndex; // [esp+0h] [ebp-4h]

    if ( !rg.registered
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 293, 0, "%s", "rg.registered") )
    {
        __debugbreak();
    }
    if ( !rg.inFrame
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 295, 0, "%s", "rg.inFrame") )
    {
        __debugbreak();
    }
    sceneEntIndex = _InterlockedExchangeAdd(&scene.sceneBrushCount, 1u);
    if ( sceneEntIndex >= r_brushLimit->current.integer )
    {
        scene.sceneBrushCount = r_brushLimit->current.integer;
        R_WarnOncePerFrame(R_WARN_KNOWN_MODELS, r_brushLimit->current.integer);
    }
    return sceneEntIndex;
}

void __cdecl R_AddGlassBrushToScene(
                const GfxBrushModel *bmodel,
                const float *origin,
                const float *angles,
                Material *altStreamingMaterial)
{
    GfxSceneGlassBrush *glassBrush; // [esp+4h] [ebp-8h]
    unsigned int glassIndex; // [esp+8h] [ebp-4h]

    if ( !bmodel && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 351, 0, "%s", "bmodel") )
        __debugbreak();
    if ( bmodel->surfaceCount )
    {
        if ( !rg.inFrame
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 356, 0, "%s", "rg.inFrame") )
        {
            __debugbreak();
        }
        glassIndex = _InterlockedExchangeAdd(&scene.glassBrushCount, 1u);
        if ( glassIndex < 0x400 )
        {
            glassBrush = &scene.glassBrush[glassIndex];
            glassBrush->bmodel = bmodel;
            glassBrush->altStreamingMaterial = altStreamingMaterial;
            glassBrush->placement.origin[0] = *origin;
            glassBrush->placement.origin[1] = origin[1];
            glassBrush->placement.origin[2] = origin[2];
            AnglesToQuat(angles, glassBrush->placement.quat);
        }
        else
        {
            scene.glassBrushCount = 1024;
            R_WarnOncePerFrame(R_WARN_KNOWN_MODELS, 1024);
        }
    }
}

void __cdecl R_AddDObjToScene(
                const DObj *obj,
                const cpose_t *pose,
                unsigned int entnum,
                unsigned int renderFxFlags,
                float *lightingOrigin,
                float materialTime,
                float materialTime2,
                float burnFraction,
                float wetness,
                char altXModel,
                int textureOverrideIndex,
                const ShaderConstantSet *dobjConstantSet,
                const ShaderConstantSet *dobjConstantSetExtraCam,
                float lightingOriginToleranceSq,
                float scale)
{
    unsigned __int8 v15; // [esp+0h] [ebp-60h]
    unsigned __int8 v16; // [esp+4h] [ebp-5Ch]
    int v17; // [esp+8h] [ebp-58h]
    int integer; // [esp+Ch] [ebp-54h]
    int n; // [esp+30h] [ebp-30h]
    GfxEntity *gfxEnt2; // [esp+34h] [ebp-2Ch]
    GfxSceneModel *sceneModel; // [esp+38h] [ebp-28h]
    XModel *model; // [esp+3Ch] [ebp-24h]
    float radius; // [esp+40h] [ebp-20h]
    float radiusa; // [esp+40h] [ebp-20h]
    GfxEntity *gfxEnt; // [esp+44h] [ebp-1Ch]
    GfxSceneEntity *sceneEnt; // [esp+4Ch] [ebp-14h]
    unsigned int sceneEntIndex; // [esp+50h] [ebp-10h]
    unsigned int sceneEntIndexa; // [esp+50h] [ebp-10h]
    unsigned int gfxEntIndex; // [esp+54h] [ebp-Ch]
    unsigned int gfxEntIndex2; // [esp+58h] [ebp-8h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 402, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 403, 0, "%s", "obj") )
        __debugbreak();
    if ( !pose && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 404, 0, "%s", "pose") )
        __debugbreak();
    if ( entnum >= gfxCfg.entCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    405,
                    0,
                    "entnum doesn't index gfxCfg.entCount\n\t%i not in [0, %i)",
                    entnum,
                    gfxCfg.entCount) )
    {
        __debugbreak();
    }
    if ( r_drawEntities->current.enabled && scene.dpvs.sceneDObjIndex[entnum] == 0xFFFF )
    {
        for ( n = 0; n < DObjGetNumModels(obj); ++n )
        {
            if ( (DObjGetModel(obj, n)->flags & 0x100000) != 0 )
            {
                renderFxFlags |= 0x400000u;
                break;
            }
        }
        if ( scene.dpvs.sceneDObjIndex[entnum] != 0xFFFF
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                        440,
                        0,
                        "%s\n\t(scene.dpvs.sceneDObjIndex[entnum]) = %i",
                        "(scene.dpvs.sceneDObjIndex[entnum] == (65535))",
                        scene.dpvs.sceneDObjIndex[entnum]) )
        {
            __debugbreak();
        }
        if ( materialTime == 0.0 && !renderFxFlags && burnFraction <= 0.0 && wetness == 0.0 && textureOverrideIndex < 0 )
        {
            LOWORD(gfxEntIndex) = 0;
            LOWORD(gfxEntIndex2) = 0;
        }
        else
        {
            gfxEntIndex = _InterlockedExchangeAdd(&frontEndDataOut->gfxEntCount, 1u);
            if ( gfxEntIndex >= 0x100 )
            {
                frontEndDataOut->gfxEntCount = 256;
                R_WarnOncePerFrame(R_WARN_KNOWN_SPECIAL_MODELS, 256);
                return;
            }
            gfxEnt = &frontEndDataOut->gfxEnts[gfxEntIndex];
            frontEndDataOut->gfxEnts[gfxEntIndex].materialTime = materialTime;
            if ( r_swrk_override_enable->current.enabled )
                integer = r_swrk_override_characterCharredAmount->current.integer;
            else
                integer = LODWORD(burnFraction);
            LODWORD(gfxEnt->destructibleBurnAmount) = integer;
            gfxEnt->destructibleFadeAmount = 0.0f;
            gfxEnt->renderFxFlags = renderFxFlags;
            gfxEnt->wetness = wetness;
            gfxEnt->textureOverrideIndex = textureOverrideIndex;
            if ( materialTime2 == 0.0 )
            {
                LOWORD(gfxEntIndex2) = 0;
            }
            else
            {
                gfxEntIndex2 = _InterlockedExchangeAdd(&frontEndDataOut->gfxEntCount, 1u);
                gfxEnt2 = &frontEndDataOut->gfxEnts[gfxEntIndex2];
                frontEndDataOut->gfxEnts[gfxEntIndex2].materialTime = materialTime2;
                if ( r_swrk_override_enable->current.enabled )
                    v17 = r_swrk_override_characterCharredAmount->current.integer;
                else
                    v17 = LODWORD(burnFraction);
                LODWORD(gfxEnt2->destructibleBurnAmount) = v17;
                gfxEnt2->destructibleFadeAmount = 0.0f;
                gfxEnt2->renderFxFlags = renderFxFlags;
                gfxEnt2->wetness = wetness;
                gfxEnt2->textureOverrideIndex = textureOverrideIndex;
            }
        }
        if ( dobjConstantSet && r_shader_constant_set_enable->current.enabled && R_ShaderConstantSetIsUsed(dobjConstantSet) )
            v16 = R_ShaderConstantSet_CopyToFrontEndDataOut(dobjConstantSet);
        else
            v16 = 0;
        if ( dobjConstantSetExtraCam
            && r_shader_constant_set_enable->current.enabled
            && R_ShaderConstantSetIsUsed(dobjConstantSetExtraCam) )
        {
            v15 = R_ShaderConstantSet_CopyToFrontEndDataOut(dobjConstantSetExtraCam);
        }
        else
        {
            v15 = v16;
        }
        if ( (renderFxFlags & 4) != 0 || DObjGetTree(obj) || DObjGetNumModels(obj) != 1 )
        {
            sceneEntIndexa = R_AllocSceneDObj();
            if ( sceneEntIndexa < 0x400 )
            {
                sceneEnt = &scene.sceneDObj[sceneEntIndexa];
                sceneEnt->obj = (DObj*)obj;
                sceneEnt->entnum = entnum;
                scene.dpvs.sceneDObjIndex[entnum] = sceneEntIndexa;
                sceneEnt->info.pose = (cpose_t*)pose;
                if ( sceneEnt->cull.state
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                                540,
                                1,
                                "%s\n\t(sceneEnt->cull.state) = %i",
                                "(sceneEnt->cull.state == CULL_STATE_OUT)",
                                sceneEnt->cull.state) )
                {
                    __debugbreak();
                }
                radiusa = DObjGetRadius(obj);
                CG_GetPoseOrigin(pose, sceneEnt->placement.base.origin);
                CG_GetPoseQuat(pose, sceneEnt->placement.base.quat);
                sceneEnt->placement.scale = scale;
                sceneEnt->cull.mins[0] = sceneEnt->placement.base.origin[0] + -radiusa;
                sceneEnt->cull.mins[1] = sceneEnt->placement.base.origin[1] + -radiusa;
                sceneEnt->cull.mins[2] = sceneEnt->placement.base.origin[2] + -radiusa;
                sceneEnt->cull.maxs[0] = sceneEnt->placement.base.origin[0] + radiusa;
                sceneEnt->cull.maxs[1] = sceneEnt->placement.base.origin[1] + radiusa;
                sceneEnt->cull.maxs[2] = sceneEnt->placement.base.origin[2] + radiusa;
                sceneEnt->lightingOrigin[0] = *lightingOrigin;
                sceneEnt->lightingOrigin[1] = lightingOrigin[1];
                sceneEnt->lightingOrigin[2] = lightingOrigin[2];
                sceneEnt->gfxEntIndex = gfxEntIndex;
                sceneEnt->gfxEntIndex2 = gfxEntIndex2;
                sceneEnt->altXModelIndex = altXModel + 1;
                sceneEnt->entShaderConstantSetIndex = v16;
                sceneEnt->entShaderConstantSetIndexExtraCam = v15;
                sceneEnt->lightingOriginToleranceSq = lightingOriginToleranceSq;
                CG_PredictiveSkinCEntity(sceneEnt);
            }
        }
        else
        {
            sceneEntIndex = R_AllocSceneModel();
            if ( sceneEntIndex < 0x400 )
            {
                sceneModel = &scene.sceneModel[sceneEntIndex];
                model = DObjGetModel(obj, 0);
                sceneModel->model = model;
                sceneModel->obj = obj;
                sceneModel->entnum = entnum;
                scene.dpvs.sceneXModelIndex[entnum] = sceneEntIndex;
                //sceneModel->cachedLightingHandle = (unsigned __int16 *)Ragdoll_HandleBody((int)pose);
                sceneModel->cachedLightingHandle = (unsigned __int16 *)((int)pose);
                sceneModel->lightingOriginToleranceSq = lightingOriginToleranceSq;
                radius = XModelGetRadius(model);
                CG_GetPoseOrigin(pose, sceneModel->placement.base.origin);
                CG_GetPoseQuat(pose, sceneModel->placement.base.quat);
                sceneModel->placement.scale = scale;
                sceneModel->radius = radius;
                sceneModel->lightingOrigin[0] = *lightingOrigin;
                sceneModel->lightingOrigin[1] = lightingOrigin[1];
                sceneModel->lightingOrigin[2] = lightingOrigin[2];
                sceneModel->gfxEntIndex = gfxEntIndex;
                sceneModel->modelShaderConstantSetIndex = v16;
                sceneModel->modelShaderConstantSetIndexExtraCam = v15;
            }
        }
    }
}

unsigned int __cdecl R_AllocSceneDObj()
{
    unsigned int sceneEntIndex; // [esp+0h] [ebp-4h]

    if ( !rg.registered
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 188, 0, "%s", "rg.registered") )
    {
        __debugbreak();
    }
    if ( !rg.inFrame
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 190, 0, "%s", "rg.inFrame") )
    {
        __debugbreak();
    }
    sceneEntIndex = _InterlockedExchangeAdd(&scene.sceneDObjCount, 1u);
    if ( sceneEntIndex >= r_dobjLimit->current.integer )
    {
        scene.sceneDObjCount = r_dobjLimit->current.integer;
        R_WarnOncePerFrame(R_WARN_KNOWN_MODELS, r_dobjLimit->current.integer);
    }
    return sceneEntIndex;
}

GfxParticleCloud *__cdecl R_AddParticleCloudToScene(Material *material)
{
    volatile unsigned int cloudIndex; // [esp+Ch] [ebp-4h]

    cloudIndex = _InterlockedExchangeAdd(&frontEndDataOut->cloudCount, 1u);
    if ( cloudIndex < 0x100 )
    {
        if ( R_AddParticleCloudDrawSurf(cloudIndex, material) )
            return &frontEndDataOut->clouds[cloudIndex];
        else
            return 0;
    }
    else
    {
        _InterlockedExchangeAdd(&frontEndDataOut->cloudCount, 0xFFFFFFFF);
        R_WarnOncePerFrame(R_WARN_MAX_CLOUDS);
        return 0;
    }
}

#if 0
void __cdecl R_AddOmniLightToScene(const float *org, const float (*axis)[3], int radius, float r, float g, float b)
{
    int integer; // xmm0_4
    int v7; // xmm0_4
    float v8; // xmm0_4
    long double v9; // [esp-50h] [ebp-6Ch]
    long double v10; // [esp-48h] [ebp-64h]
    float v11; // [esp-40h] [ebp-5Ch]
    float v12; // [esp-3Ch] [ebp-58h]
    float v13; // [esp-38h] [ebp-54h]
    float v14; // [esp-2Ch] [ebp-48h]
    float v15; // [esp-28h] [ebp-44h]
    float v16; // [esp-24h] [ebp-40h]
    GfxLight *v17; // [esp-Ch] [ebp-28h]
    GfxLight *v18; // [esp-Ch] [ebp-28h]
    int i; // [esp-8h] [ebp-24h]
    float radiusa; // [esp+28h] [ebp+Ch]

    if ( rg.registered && rgp.world )
    {
        if ( !rg.inFrame
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 604, 0, "%s", "rg.inFrame") )
        {
            __debugbreak();
        }
        if ( *(float *)&radius > 0.0 )
        {
            if ( scene.addedLightCount < 32 )
            {
                if ( rg.isNotRenderingFullScreen )
                {
                    if ( (float)(r_dlightMaxNonFullScreenRadius->current.value - *(float *)&radius) < 0.0 )
                        integer = r_dlightMaxNonFullScreenRadius->current.integer;
                    else
                        integer = radius;
                    radiusa = *(float *)&integer;
                }
                else
                {
                    if ( (float)(r_dlightMaxFullScreenRadius->current.value - *(float *)&radius) < 0.0 )
                        v7 = r_dlightMaxFullScreenRadius->current.integer;
                    else
                        v7 = radius;
                    radiusa = *(float *)&v7;
                }
                for ( i = 0; i < scene.addedLightCount; ++i )
                {
                    v17 = &scene.addedLight[i];
                    if ( v17->type == 3
                        && fabs(radiusa - v17->radius) < 10.0
                        && 10.0 * 10.0 > Vec3DistanceSq(org, v17->origin) )
                    {
                        v17->origin[0] = (float)(0.5 * *org) + (float)(0.5 * v17->origin[0]);
                        v17->origin[1] = (float)(0.5 * org[1]) + (float)(0.5 * v17->origin[1]);
                        v17->origin[2] = (float)(0.5 * org[2]) + (float)(0.5 * v17->origin[2]);
                        if ( (float)(radiusa - v17->radius) < 0.0 )
                            v8 = v17->radius;
                        else
                            v8 = radiusa;
                        v17->radius = v8;
                        if ( r <= 0.040449999 )
                        {
                            v16 = r / 12.92;
                        }
                        else
                        {
                            __libm_sse2_pow(v9, v10);
                            v16 = (float)(r + 0.055) / 1.0549999;
                        }
                        v17->diffuseColor[0] = v17->diffuseColor[0] + v16;
                        if ( g <= 0.040449999 )
                        {
                            v15 = g / 12.92;
                        }
                        else
                        {
                            __libm_sse2_pow(v9, v10);
                            v15 = (float)(g + 0.055) / 1.0549999;
                        }
                        v17->diffuseColor[1] = v17->diffuseColor[1] + v15;
                        if ( b <= 0.040449999 )
                        {
                            v14 = b / 12.92;
                        }
                        else
                        {
                            __libm_sse2_pow(v9, v10);
                            v14 = (float)(b + 0.055) / 1.0549999;
                        }
                        v17->diffuseColor[2] = v17->diffuseColor[2] + v14;
                        return;
                    }
                }
                v18 = &scene.addedLight[scene.addedLightCount++];
                memset(&v18->type, 0, sizeof(GfxLight));
                v18->def = rgp.dlightDef;
                v18->type = 3;
                v18->origin[0] = *org;
                v18->origin[1] = org[1];
                v18->origin[2] = org[2];
                v18->radius = radiusa;
                v18->color[0] = r;
                v18->color[1] = g;
                v18->color[2] = b;
                v18->canUseShadowMap = 0;
                v18->spotShadowIndex = -1;
                if ( b <= 0.040449999 )
                {
                    v13 = b / 12.92;
                }
                else
                {
                    __libm_sse2_pow(v9, v10);
                    v13 = (float)(b + 0.055) / 1.0549999;
                }
                if ( g <= 0.040449999 )
                {
                    v12 = g / 12.92;
                }
                else
                {
                    __libm_sse2_pow(v9, v10);
                    v12 = (float)(g + 0.055) / 1.0549999;
                }
                if ( r <= 0.040449999 )
                {
                    v11 = r / 12.92;
                }
                else
                {
                    __libm_sse2_pow(v9, v10);
                    v11 = (float)(r + 0.055) / 1.0549999;
                }
                v18->diffuseColor[0] = v11;
                v18->diffuseColor[1] = v12;
                v18->diffuseColor[2] = v13;
                v18->diffuseColor[3] = 1.0f;
            }
            else
            {
                R_WarnOncePerFrame(R_WARN_MAX_DLIGHTS);
            }
        }
    }
}
#endif

static inline float SRGBToLinear(float c)
{
    if (c <= 0.04045f)
        return c * (1.0f / 12.92f);

    return powf((c + 0.055f) * (1.0f / 1.055f), 2.4f);
}

// aislop
void __cdecl R_AddOmniLightToScene(
    const float *org,
    const float (*axis)[3],
    int radius,
    float r,
    float g,
    float b)
{
    int i;
    float clampedRadius;
    GfxLight *light;

    if (!rg.registered || !rgp.world)
        return;

    if (!rg.inFrame)
    {
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
            604,
            0,
            "%s",
            "rg.inFrame"))
        {
            __debugbreak();
        }
    }

    if (radius <= 0)
        return;

    if (scene.addedLightCount >= 32)
    {
        R_WarnOncePerFrame(R_WARN_MAX_DLIGHTS);
        return;
    }

    /* clamp radius based on fullscreen state */
    if (rg.isNotRenderingFullScreen)
    {
        clampedRadius =
            (radius > r_dlightMaxNonFullScreenRadius->current.value)
            ? (float)r_dlightMaxNonFullScreenRadius->current.integer
            : (float)radius;
    }
    else
    {
        clampedRadius =
            (radius > r_dlightMaxFullScreenRadius->current.value)
            ? (float)r_dlightMaxFullScreenRadius->current.integer
            : (float)radius;
    }

    /* try to merge with existing omni light */
    for (i = 0; i < scene.addedLightCount; ++i)
    {
        light = &scene.addedLight[i];

        if (light->type != 3)
            continue;

        if (fabsf(clampedRadius - light->radius) >= 10.0f)
            continue;

        if (Vec3DistanceSq(org, light->origin) >= 100.0f)
            continue;

        /* merge */
        light->origin[0] = 0.5f * (org[0] + light->origin[0]);
        light->origin[1] = 0.5f * (org[1] + light->origin[1]);
        light->origin[2] = 0.5f * (org[2] + light->origin[2]);

        if (clampedRadius > light->radius)
            light->radius = clampedRadius;

        light->diffuseColor[0] += SRGBToLinear(r);
        light->diffuseColor[1] += SRGBToLinear(g);
        light->diffuseColor[2] += SRGBToLinear(b);

        return;
    }

    /* add new light */
    light = &scene.addedLight[scene.addedLightCount++];
    memset(light, 0, sizeof(*light));

    light->def = rgp.dlightDef;
    light->type = 3;

    light->origin[0] = org[0];
    light->origin[1] = org[1];
    light->origin[2] = org[2];

    light->radius = clampedRadius;

    light->color[0] = r;
    light->color[1] = g;
    light->color[2] = b;

    light->canUseShadowMap = 0;
    light->spotShadowIndex = -1;

    light->diffuseColor[0] = SRGBToLinear(r);
    light->diffuseColor[1] = SRGBToLinear(g);
    light->diffuseColor[2] = SRGBToLinear(b);
    light->diffuseColor[3] = 1.0f;
}

#if 0
void __cdecl R_AddSpotLightToScene(const float *org, const float (*axis)[3], float radius, float r, float g, float b)
{
    double v6; // xmm0_8
    float v7; // xmm0_4
    long double v8; // [esp+10h] [ebp-6Ch]
    long double v9; // [esp+10h] [ebp-6Ch]
    long double v10; // [esp+10h] [ebp-6Ch]
    long double v11; // [esp+10h] [ebp-6Ch]
    long double v12; // [esp+10h] [ebp-6Ch]
    long double v13; // [esp+18h] [ebp-64h]
    float value; // [esp+1Ch] [ebp-60h]
    float v15; // [esp+28h] [ebp-54h]
    float v16; // [esp+2Ch] [ebp-50h]
    float v17; // [esp+30h] [ebp-4Ch]
    float spotLightFovInner; // [esp+54h] [ebp-28h]
    float dir; // [esp+58h] [ebp-24h]
    float dir_4; // [esp+5Ch] [ebp-20h]
    float dir_8; // [esp+60h] [ebp-1Ch]
    float spotLightOffset; // [esp+64h] [ebp-18h]
    GfxLight *light; // [esp+70h] [ebp-Ch]
    int savedregs; // [esp+7Ch] [ebp+0h] BYREF
    float radiusa; // [esp+8Ch] [ebp+10h]

    dir = (*axis)[0];
    dir_4 = (*axis)[1];
    dir_8 = (*axis)[2];
    if ( !r_enableFlashlight->current.integer && rg.registered && rgp.world )
    {
        if ( !rg.inFrame
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 680, 0, "%s", "rg.inFrame") )
        {
            __debugbreak();
        }
        if ( radius > 0.0 )
        {
            if ( scene.addedLightCount < 32 )
            {
                light = &scene.addedLight[scene.addedLightCount++];
                memset(&light->type, 0, sizeof(GfxLight));
                light->exponent = 1;
                light->type = 2;
                radiusa = r_flashLightRange->current.value;
                v6 = (float)((float)(r_flashLightEndRadius->current.value - r_flashLightStartRadius->current.value) / radiusa);
                __libm_sse2_atan(v8);
                v7 = v6;
                spotLightFovInner = v7 * r_spotLightFovInnerFraction->current.value;
                __libm_sse2_tan(v9);
                spotLightOffset = r_flashLightStartRadius->current.value / v7;
                light->canUseShadowMap = 1;
                light->spotShadowIndex = -1;
                light->type = 2;
                light->cullDist = (int)radiusa;
                light->origin[0] = *org;
                light->origin[1] = org[1];
                light->origin[2] = org[2];
                light->dir[0] = -1.0 * dir;
                light->dir[1] = -1.0 * dir_4;
                light->dir[2] = -1.0 * dir_8;
                light->origin[0] = (float)(spotLightOffset * light->dir[0]) + light->origin[0];
                light->origin[1] = (float)(spotLightOffset * light->dir[1]) + light->origin[1];
                light->origin[2] = (float)(spotLightOffset * light->dir[2]) + light->origin[2];
                light->radius = radiusa + spotLightOffset;
                if ( b <= 0.040449999 )
                {
                    v17 = b / 12.92;
                }
                else
                {
                    __libm_sse2_pow(v10, v13);
                    v17 = (float)(b + 0.055) / 1.0549999;
                }
                if ( g <= 0.040449999 )
                {
                    v16 = g / 12.92;
                }
                else
                {
                    __libm_sse2_pow(v10, v13);
                    v16 = (float)(g + 0.055) / 1.0549999;
                }
                if ( r <= 0.040449999 )
                {
                    v15 = r / 12.92;
                }
                else
                {
                    __libm_sse2_pow(v10, v13);
                    v15 = (float)(r + 0.055) / 1.0549999;
                }
                light->diffuseColor[0] = v15;
                light->diffuseColor[1] = v16;
                light->diffuseColor[2] = v17;
                value = r_flashLightBrightness->current.value;
                light->diffuseColor[0] = value * light->diffuseColor[0];
                light->diffuseColor[1] = value * light->diffuseColor[1];
                light->diffuseColor[2] = value * light->diffuseColor[2];
                LODWORD(v11) = light->specularColor;
                HIDWORD(v11) = light->diffuseColor;
                light->specularColor[0] = light->diffuseColor[0];
                light->specularColor[1] = light->diffuseColor[1];
                light->specularColor[2] = light->diffuseColor[2];
                cos(v11);
                light->cosHalfFovInner = spotLightFovInner;
                cos(v12);
                light->cosHalfFovOuter = v7;
                light->angles[0] = 0.0f;
                light->angles[1] = 0.0f;
                light->angles[2] = 0.0f;
                light->attenuation[0] = 1.0f;
                light->attenuation[1] = 0.0f;
                light->attenuation[2] = 0.0f;
                light->attenuation[3] = 1.0f;
                light->aAbB[0] = 0.75f;
                light->aAbB[1] = 1.0f;
                light->aAbB[2] = 0.75f;
                light->aAbB[3] = 1.0f;
                light->falloff[0] = 0.0f;
                light->falloff[1] = radiusa;
                light->falloff[2] = (float)((float)(radiusa - 0.0) * 0.0) + 0.0;
                light->falloff[3] = (float)((float)(radiusa - 0.0) * 0.0) + 0.0;
                SpotLightViewMatrixDir3(
                    (const float *)axis,
                    &(*axis)[3],
                    &(*axis)[6],
                    light->viewMatrix.m);
                SpotLightProjectionMatrix(light->cosHalfFovOuter, light->falloff[0], light->falloff[1], light->projMatrix.m);
                light->def = rgp.flashLightDef;
                scene.dynamicSpotLightNearPlaneOffset = spotLightOffset;
                R_CalcSpotLightPlanes(light, spotLightOffset, scene.dynamicSpotLightPlanes);
            }
            else
            {
                R_WarnOncePerFrame(R_WARN_MAX_DLIGHTS);
            }
        }
    }
}
#endif

void __cdecl R_AddSpotLightToScene(
    const float *org,
    const float (*axis)[3],
    float radius,
    float r,
    float g,
    float b)
{
    GfxLight *light;
    float range;
    float outerHalfAngle;
    float innerHalfAngle;
    float spotOffset;
    float brightness;

    const float *dir = axis[0];   // forward

    if (r_enableFlashlight->current.integer)
        return;

    if (!rg.registered || !rgp.world)
        return;

    if (!rg.inFrame)
    {
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
            680,
            0,
            "%s",
            "rg.inFrame"))
        {
            __debugbreak();
        }
    }

    if (radius <= 0.0f)
        return;

    if (scene.addedLightCount >= 32)
    {
        R_WarnOncePerFrame(R_WARN_MAX_DLIGHTS);
        return;
    }

    light = &scene.addedLight[scene.addedLightCount++];
    memset(light, 0, sizeof(*light));

    /* spotlight type */
    light->type = 2;
    light->exponent = 1;
    light->canUseShadowMap = 1;
    light->spotShadowIndex = -1;

    range = r_flashLightRange->current.value;

    /* compute outer half-angle */
    outerHalfAngle =
        atanf(
            (r_flashLightEndRadius->current.value -
                r_flashLightStartRadius->current.value) /
            range);

    innerHalfAngle =
        outerHalfAngle * r_spotLightFovInnerFraction->current.value;

    spotOffset =
        r_flashLightStartRadius->current.value /
        tanf(outerHalfAngle);

    /* position */
    light->origin[0] = org[0] - dir[0] * spotOffset;
    light->origin[1] = org[1] - dir[1] * spotOffset;
    light->origin[2] = org[2] - dir[2] * spotOffset;

    /* direction */
    light->dir[0] = -dir[0];
    light->dir[1] = -dir[1];
    light->dir[2] = -dir[2];

    light->radius = range + spotOffset;
    light->cullDist = (int)range;

    /* color (linear space) */
    light->diffuseColor[0] = SRGBToLinear(r);
    light->diffuseColor[1] = SRGBToLinear(g);
    light->diffuseColor[2] = SRGBToLinear(b);

    brightness = r_flashLightBrightness->current.value;
    light->diffuseColor[0] *= brightness;
    light->diffuseColor[1] *= brightness;
    light->diffuseColor[2] *= brightness;

    /* specular = diffuse */
    light->specularColor[0] = light->diffuseColor[0];
    light->specularColor[1] = light->diffuseColor[1];
    light->specularColor[2] = light->diffuseColor[2];

    /* spotlight cone */
    light->cosHalfFovInner = cosf(innerHalfAngle);
    light->cosHalfFovOuter = cosf(outerHalfAngle);

    /* defaults */
    light->angles[0] = 0.0f;
    light->angles[1] = 0.0f;
    light->angles[2] = 0.0f;

    light->attenuation[0] = 1.0f;
    light->attenuation[1] = 0.0f;
    light->attenuation[2] = 0.0f;
    light->attenuation[3] = 1.0f;

    light->aAbB[0] = 0.75f;
    light->aAbB[1] = 1.0f;
    light->aAbB[2] = 0.75f;
    light->aAbB[3] = 1.0f;

    /* falloff */
    light->falloff[0] = 0.0f;
    light->falloff[1] = range;
    light->falloff[2] = 0.0f;
    light->falloff[3] = 0.0f;

    /* matrices */
    SpotLightViewMatrixDir3(
        axis[0],
        axis[1],
        axis[2],
        light->viewMatrix.m);

    SpotLightProjectionMatrix(
        light->cosHalfFovOuter,
        light->falloff[0],
        light->falloff[1],
        light->projMatrix.m);

    light->def = rgp.flashLightDef;

    scene.dynamicSpotLightNearPlaneOffset = spotOffset;
    R_CalcSpotLightPlanes(
        light,
        spotOffset,
        scene.dynamicSpotLightPlanes);
}


const float MINIMUM_Z_NEAR = 1.0f;
float __cdecl R_GetDefaultNearClip()
{
    if ( (float)(1.0 - r_znear->current.value) < 0.0 )
        return r_znear->current.value;
    else
        return MINIMUM_Z_NEAR;
}

void __cdecl R_SetupViewProjectionMatrices(GfxViewParms *viewParms, bool offsetmatrix)
{
    if ( rg.renderHiResShot && rg.hiResShotTiles > 0 && offsetmatrix )
    {
        viewParms->projectionMatrix.m[0][0] = (float)rg.hiResShotTiles * viewParms->projectionMatrix.m[0][0];
        viewParms->projectionMatrix.m[1][1] = (float)rg.hiResShotTiles * viewParms->projectionMatrix.m[1][1];
        viewParms->projectionMatrix.m[2][0] = (float)((float)(rg.hiResShotRow / (float)rg.hiResShotTiles)
                                                                                                + (float)((float)(rg.hiResShotRow + 1.0) / (float)rg.hiResShotTiles))
                                                                                / (float)((float)(rg.hiResShotRow / (float)rg.hiResShotTiles)
                                                                                                - (float)((float)(rg.hiResShotRow + 1.0) / (float)rg.hiResShotTiles));
        viewParms->projectionMatrix.m[2][1] = (float)((float)(rg.hiResShotCol / (float)rg.hiResShotTiles)
                                                                                                + (float)((float)(rg.hiResShotCol + 1.0) / (float)rg.hiResShotTiles))
                                                                                / (float)((float)((-(rg.hiResShotCol)) / (float)rg.hiResShotTiles)
                                                                                                + (float)((float)(rg.hiResShotCol + 1.0) / (float)rg.hiResShotTiles));
        viewParms->projectionMatrix.m[2][0] = (float)rg.hiResShotTiles + viewParms->projectionMatrix.m[2][0];
        viewParms->projectionMatrix.m[2][1] = viewParms->projectionMatrix.m[2][1] - (float)rg.hiResShotTiles;
    }
    MatrixMultiply44(viewParms->viewMatrix.m, viewParms->projectionMatrix.m, viewParms->viewProjectionMatrix.m);
    MatrixInverse44(viewParms->viewProjectionMatrix.m, viewParms->inverseViewProjectionMatrix.m);
}

void __cdecl R_AddBModelSurfacesCamera(
                BModelDrawInfo *bmodelInfo,
                const GfxBrushModel *bmodel,
                GfxDrawSurf **drawSurfs,
                GfxDrawSurf **lastDrawSurfs,
                unsigned int reflectionProbeIndex,
                float burnAmt,
                float fadeAmt,
                char visLightsMask)
{
    unsigned __int64 v8; // rax
    unsigned __int64 v9; // rax
    unsigned __int64 v10; // rax
    unsigned int surfId; // [esp+8h] [ebp-28h]
    unsigned int surfIndex; // [esp+Ch] [ebp-24h]
    int drawSurf; // [esp+10h] [ebp-20h]
    int drawSurfa; // [esp+10h] [ebp-20h]
    unsigned __int64 drawSurfb; // [esp+10h] [ebp-20h]
    unsigned int drawSurf_4; // [esp+14h] [ebp-1Ch]
    unsigned int drawSurf_4a; // [esp+14h] [ebp-1Ch]
    const Material *material; // [esp+1Ch] [ebp-14h]
    BModelSurface *modelSurf; // [esp+20h] [ebp-10h]
    const GfxSurface *bspSurf; // [esp+24h] [ebp-Ch]
    unsigned int region; // [esp+28h] [ebp-8h]
    unsigned int count; // [esp+2Ch] [ebp-4h]

    if ( !bmodel && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 994, 0, "%s", "bmodel") )
        __debugbreak();
    surfId = bmodelInfo->surfId;
    modelSurf = (BModelSurface *)((char *)frontEndDataOut + 4 * surfId);
    if ( reflectionProbeIndex >= 8
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    999,
                    0,
                    "reflectionProbeIndex doesn't index 1 << MTL_SORT_ENVMAP_BITS\n\t%i not in [0, %i)",
                    reflectionProbeIndex,
                    8) )
    {
        __debugbreak();
    }
    if ( gfxDrawMethod.emissiveTechType >= 0x82u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    1001,
                    0,
                    "gfxDrawMethod.emissiveTechType doesn't index TECHNIQUE_COUNT\n\t%i not in [0, %i)",
                    gfxDrawMethod.emissiveTechType,
                    130) )
    {
        __debugbreak();
    }
    count = 0;
    surfIndex = bmodel->startSurfIndex;
    while ( count < bmodel->surfaceCount )
    {
        bspSurf = modelSurf->surf;
        material = bspSurf->material;
        if ( rgp.sortedMaterials[(material->info.drawSurf.packed >> 31) & 0xFFF] != material
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                        1007,
                        0,
                        "%s",
                        "rgp.sortedMaterials[material->info.drawSurf.fields.materialSortedIndex] == material") )
        {
            __debugbreak();
        }
        region = material->cameraRegion;
        if ( region != 3 )
        {
            if ( drawSurfs[region] >= lastDrawSurfs[region] )
            {
                R_WarnOncePerFrame(R_WARN_MAX_SCENE_DRAWSURFS, "R_AddBModelSurfacesCamera");
                return;
            }
            modelSurf->bmodelBurnAmt = burnAmt;
            modelSurf->bmodelFadeAmt = fadeAmt;
            if ( surfId >= 0x10000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                            1022,
                            0,
                            "surfId doesn't index 1 << MTL_SORT_OBJECT_ID_BITS\n\t%i not in [0, %i)",
                            surfId,
                            0x10000) )
            {
                __debugbreak();
            }
            v8 = (unsigned __int64)(bspSurf->lightmapIndex & 0x1F) << 20;
            drawSurf_4 = HIDWORD(v8) | HIDWORD(material->info.drawSurf.packed) & 0xFF87FFFF | 0x300000;
            drawSurf = ((reflectionProbeIndex & 7) << 25)
                             | v8 & 0xF1FFFFFF
                             | (unsigned __int16)surfId
                             | *(unsigned int *)&material->info.drawSurf.fields & 0xF00F0000;
            v9 = (unsigned __int64)(visLightsMask & 1) << 29;
            drawSurfa = v9 | drawSurf & 0xDFFFFFFF;
            drawSurf_4a = HIDWORD(v9) | (bspSurf->primaryLightIndex << 11) | drawSurf_4 & 0xFFF807FF;
            v10 = (unsigned __int64)((visLightsMask & 2) != 0) << 30;
            LODWORD(drawSurfb) = v10 | drawSurfa & 0xBFFFFFFF;
            HIDWORD(drawSurfb) = HIDWORD(v10) | drawSurf_4a;
            drawSurfs[region]->packed = drawSurfb;
            ++drawSurfs[region];
        }
        ++count;
        ++surfIndex;
        ++modelSurf;
        surfId += 5;
    }
}

GfxDrawSurf *__cdecl R_AddBModelSurfaces(
                BModelDrawInfo *bmodelInfo,
                const GfxBrushModel *bmodel,
                unsigned __int8 techType,
                GfxDrawSurf *drawSurf,
                GfxDrawSurf *lastDrawSurf)
{
    unsigned int surfId; // [esp+Ch] [ebp-24h]
    unsigned int surfIndex; // [esp+10h] [ebp-20h]
    const Material *material; // [esp+14h] [ebp-1Ch]
    unsigned int newDrawSurf_4; // [esp+1Ch] [ebp-14h]
    BModelSurface *modelSurf; // [esp+24h] [ebp-Ch]
    unsigned int count; // [esp+2Ch] [ebp-4h]

    if ( !bmodel && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 1051, 0, "%s", "bmodel") )
        __debugbreak();
    surfId = bmodelInfo->surfId;
    modelSurf = (BModelSurface *)((char *)frontEndDataOut + 4 * surfId);
    count = 0;
    surfIndex = bmodel->startSurfIndex;
    while ( count < bmodel->surfaceCount )
    {
        if ( drawSurf >= lastDrawSurf )
        {
            R_WarnOncePerFrame(R_WARN_MAX_SCENE_DRAWSURFS, "R_AddBModelSurfaces");
            return drawSurf;
        }
        material = modelSurf->surf->material;
        if ( rgp.sortedMaterials[(material->info.drawSurf.packed >> 31) & 0xFFF] != material
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                        1066,
                        0,
                        "%s",
                        "rgp.sortedMaterials[material->info.drawSurf.fields.materialSortedIndex] == material") )
        {
            __debugbreak();
        }
        if ( Material_GetTechnique(material, techType) )
        {
            if ( surfId >= 0x10000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                            1071,
                            0,
                            "surfId doesn't index 1 << MTL_SORT_OBJECT_ID_BITS\n\t%i not in [0, %i)",
                            surfId,
                            0x10000) )
            {
                __debugbreak();
            }
            newDrawSurf_4 = HIDWORD(material->info.drawSurf.packed) & 0xFF87FFFF | 0x300000;
            *(unsigned int *)&drawSurf->fields = (unsigned __int16)surfId | *(unsigned int *)&material->info.drawSurf.fields & 0xFFFF0000;
            HIDWORD(drawSurf->packed) = newDrawSurf_4;
            ++drawSurf;
        }
        ++count;
        ++surfIndex;
        ++modelSurf;
        surfId += 5;
    }
    return drawSurf;
}

const MaterialTechnique *__cdecl Material_GetTechnique(const Material *material, unsigned __int8 techType)
{
    const char *v2; // eax
    const MaterialTechniqueSet *techSet; // [esp+4h] [ebp-4h]

    techSet = Material_GetTechniqueSet(material);
    if ( !techSet )
    {
        v2 = va(
                     "material '%s' missing techset. %d tech %d",
                     material->info.name,
                     material->localTechniqueSet != 0,
                     techType);
        if ( !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.h",
                        588,
                        0,
                        "%s\n\t%s",
                        "techSet",
                        v2) )
            __debugbreak();
    }
    return techSet->techniques[techType];
}

MaterialTechniqueSet *__cdecl Material_GetTechniqueSet(const Material *material)
{
    const char *v1; // eax

    if ( !material
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.h", 553, 0, "%s", "material") )
    {
        __debugbreak();
    }
    if ( !material->localTechniqueSet )
    {
        v1 = va(
                     "material '%s' missing techset. If you are building fastfile, check Launcher for error messages.",
                     material->info.name);
        if ( !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.h",
                        554,
                        0,
                        "%s\n\t%s",
                        "material->techniqueSet",
                        v1) )
            __debugbreak();
    }
    return material->localTechniqueSet;
}

void __cdecl R_SetEnablePlayerShadowFlag(bool flag)
{
    rg.enablePlayerShadowFlag = flag;
}

void R_AddXModelSurfacesCamera(
    XModelDrawInfo *modelInfo,
    const XModel *model,
    float *origin,
    unsigned int gfxEntIndex,
    unsigned int lightingHandle,
    unsigned __int8 primaryLightIndex,
    int isShadowReceiver,
    int depthHack,
    GfxDrawSurf **drawSurfs,
    GfxDrawSurf **lastDrawSurfs,
    unsigned int reflectionProbeIndex,
    unsigned int constantSetIndex,
    char visLightsMask)
{
    unsigned __int64 v14; // rax
    unsigned __int64 v15; // rax
    unsigned __int64 v16; // rax
    unsigned __int64 v17; // rax
    unsigned __int64 v18; // rax
    unsigned __int64 v19; // rax
    const XSurface *surf; // eax
    int NumTris; // eax
    const XSurface *XSurface; // eax
    char customIndex; // [esp+Ch] [ebp-3Ch]
    unsigned int surfId; // [esp+10h] [ebp-38h]
    int totalVertCount; // [esp+14h] [ebp-34h]
    GfxDrawSurf drawSurf; // [esp+18h] [ebp-30h] BYREF
    int totalTriCount; // [esp+24h] [ebp-24h]
    Material *const *material; // [esp+28h] [ebp-20h]
    unsigned int subMatIndex; // [esp+2Ch] [ebp-1Ch]
    int skinnedCachedOffset; // [esp+30h] [ebp-18h]
    int lod; // [esp+34h] [ebp-14h]
    GfxModelRigidSurface *modelSurf; // [esp+38h] [ebp-10h]
    surfaceType_t surfType; // [esp+3Ch] [ebp-Ch]
    unsigned int region; // [esp+40h] [ebp-8h]
    unsigned int numsurfs; // [esp+44h] [ebp-4h]

    iassert(lightingHandle);
    totalTriCount = 0;
    totalVertCount = 0;
    iassert(model);
    surfId = modelInfo->surfId;
    modelSurf = (GfxModelRigidSurface *)((char *)frontEndDataOut + 4 * surfId);
    lod = modelInfo->lod;
    numsurfs = XModelGetSurfCount(model, lod);
    material = XModelGetSkins(model, lod);
    iassert(material);

    bcassert(reflectionProbeIndex, 1 << 3 /*MTL_SORT_ENVMAP_BITS*/);
    bcassert(gfxDrawMethod.emissiveTechType, 130 /*TECHNIQUE_COUNT*/);

    subMatIndex = 0;
    while (subMatIndex < numsurfs)
    {
        skinnedCachedOffset = modelSurf->surf.skinnedCachedOffset;
        if (skinnedCachedOffset == -3)
        {
            ++surfId;
            modelSurf = (GfxModelRigidSurface *)((char *)modelSurf + 4);
        }
        else
        {
            iassert(*material);
            iassert(rgp.sortedMaterials[(*material)->info.drawSurf.fields.materialSortedIndex] == *material);

            region = (*material)->cameraRegion;
            if (region == 3)
            {
                surfId += 14;
                ++modelSurf;
            }
            else
            {
                if (drawSurfs[region] >= lastDrawSurfs[region])
                {
                    R_WarnOncePerFrame(R_WARN_MAX_SCENE_DRAWSURFS, "R_AddXModelSurfacesCamera");
                    break;
                }

                if (skinnedCachedOffset == -2)
                {
                    surfType = SF_BEGIN_XMODEL;
                }
                else
                {
                    iassert(skinnedCachedOffset == (-1));
                    surfType = SF_XMODEL_RIGID_SKINNED;
                }

                modelSurf->surf.info.gfxEntIndex = gfxEntIndex;
                modelSurf->surf.info.lightingHandle = lightingHandle;
                modelSurf->surf.info.dobjModelIndex = 0;

                bcassert(surfId, 0x10000/*1 << MTL_SORT_OBJECT_ID_BITS*/);

                customIndex = isShadowReceiver != 0;
                if (gfxEntIndex && frontEndDataOut->gfxEnts[gfxEntIndex].destructibleBurnAmount > 0.0)
                    customIndex |= 2u;

                //HIDWORD(v14) = HIDWORD((*material)->info.drawSurf.packed);
                //*(_DWORD *)&drawSurf.fields = (*material)->info.drawSurf.fields;
                //HIDWORD(drawSurf.packed) = HIDWORD(v14);
                //LODWORD(v14) = drawSurf.fields;
                //v14 = drawSurf.packed;
                //v15 = (unsigned __int64)((unsigned int)((v14 >> 58) & 0x3F) - depthHack) << 58;
                //*(_DWORD *)&drawSurf.fields |= v15;
                //HIDWORD(drawSurf.packed) = HIDWORD(v15) | HIDWORD(drawSurf.packed) & 0x3FFFFFF;
                //HIDWORD(drawSurf.packed) = ((surfType & 0xF) << 19) | HIDWORD(drawSurf.packed) & 0xFF87FFFF;
                //*(_DWORD *)&drawSurf.fields = (unsigned __int16)surfId | *(_DWORD *)&drawSurf.fields & 0xFFFF0000;
                //v16 = (unsigned __int64)(reflectionProbeIndex & 7) << 25;
                //*(_DWORD *)&drawSurf.fields = v16 | *(_DWORD *)&drawSurf.fields & 0xF1FFFFFF;
                //HIDWORD(drawSurf.packed) |= HIDWORD(v16);
                //v17 = (unsigned __int64)(customIndex & 0x1F) << 20;
                //*(_DWORD *)&drawSurf.fields = v17 | *(_DWORD *)&drawSurf.fields & 0xFE0FFFFF;
                //HIDWORD(drawSurf.packed) |= HIDWORD(v17);
                //HIDWORD(drawSurf.packed) = (primaryLightIndex << 11) | HIDWORD(drawSurf.packed) & 0xFFF807FF;
                //R_XModelDrawSurfEncodeShaderConstantSet(&drawSurf, constantSetIndex);
                //v18 = (unsigned __int64)(visLightsMask & 1) << 29;
                //*(_DWORD *)&drawSurf.fields = v18 | *(_DWORD *)&drawSurf.fields & 0xDFFFFFFF;
                //HIDWORD(drawSurf.packed) |= HIDWORD(v18);
                //v19 = (unsigned __int64)((visLightsMask & 2) != 0) << 30;
                //*(_DWORD *)&drawSurf.fields = v19 | *(_DWORD *)&drawSurf.fields & 0xBFFFFFFF;
                //HIDWORD(drawSurf.packed) |= HIDWORD(v19);
                //drawSurfs[region]->fields = drawSurf.fields;

                // (aislopped)
                drawSurfs[region]->packed = (*material)->info.drawSurf.packed;

                drawSurfs[region]->fields.objectId = surfId;
                drawSurfs[region]->fields.reflectionProbeIndex = reflectionProbeIndex & 7;
                drawSurfs[region]->fields.customIndex = customIndex & 0x1F;
                drawSurfs[region]->fields.primaryLightIndex = primaryLightIndex;
                drawSurfs[region]->fields.surfType = surfType;

                drawSurfs[region]->fields.primarySortKey -= depthHack;

                R_XModelDrawSurfEncodeShaderConstantSet(drawSurfs[region], constantSetIndex);

                drawSurfs[region]->fields.glightRender = (visLightsMask & 1) != 0;
                drawSurfs[region]->fields.dlightRender = (visLightsMask & 2) != 0;


                ++drawSurfs[region];
                if (r_showTriCounts->current.enabled)
                {
                    surf = R_GetXSurface((unsigned int*)modelSurf, surfType);
                    NumTris = XSurfaceGetNumTris(surf);
                    totalTriCount += NumTris;
                }
                else if (r_showVertCounts->current.enabled)
                {
                    XSurface = R_GetXSurface((unsigned int *)modelSurf, surfType);
                    totalVertCount += XSurfaceGetNumVerts(XSurface);
                }
                surfId += 14;
                ++modelSurf;
            }
        }
        ++subMatIndex;
        ++material;
    }
    if (r_showTriCounts->current.enabled)
    {
        R_AddXModelDebugString(origin, va("%i", totalTriCount));
    }
    else if (r_showVertCounts->current.enabled)
    {
        R_AddXModelDebugString(origin, va("%i", totalVertCount));
    }
    else if (r_showSurfCounts->current.enabled)
    {
        R_AddXModelDebugString(origin, va("%i", numsurfs));
    }
}

const XSurface *__cdecl R_GetXSurface(unsigned int *modelSurf, surfaceType_t surfType)
{
    bool v3; // [esp+0h] [ebp-4h]

    if ( !modelSurf
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 1093, 0, "%s", "modelSurf") )
    {
        __debugbreak();
    }
    v3 = (unsigned int)surfType >= SF_BEGIN_XMODEL && (unsigned int)surfType < SF_END_XMODEL;
    if ( !v3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    1094,
                    0,
                    "%s",
                    "R_IsModelSurfaceType( surfType )") )
    {
        __debugbreak();
    }
    return (const XSurface *)modelSurf[1];
}

GfxDrawSurf *__cdecl R_AddXModelSurfaces(
                XModelDrawInfo *modelInfo,
                const XModel *model,
                unsigned __int8 techType,
                GfxDrawSurf *drawSurf,
                GfxDrawSurf *lastDrawSurf)
{
    unsigned int surfId; // [esp+10h] [ebp-2Ch]
    Material **material; // [esp+14h] [ebp-28h]
    unsigned int subMatIndex; // [esp+18h] [ebp-24h]
    unsigned __int64 newDrawSurf; // [esp+1Ch] [ebp-20h]
    int skinnedCachedOffset; // [esp+28h] [ebp-14h]
    int lod; // [esp+2Ch] [ebp-10h]
    GfxModelRigidSurface *modelSurf; // [esp+30h] [ebp-Ch]
    char surfType; // [esp+34h] [ebp-8h]
    unsigned int numsurfs; // [esp+38h] [ebp-4h]

    if ( !model && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 1310, 0, "%s", "model") )
        __debugbreak();
    surfId = modelInfo->surfId;
    modelSurf = (GfxModelRigidSurface *)((char *)frontEndDataOut + 4 * surfId);
    lod = modelInfo->lod;
    numsurfs = XModelGetSurfCount(model, lod);
    material = XModelGetSkins(model, lod);
    if ( !material
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 1320, 0, "%s", "material") )
    {
        __debugbreak();
    }
    for ( subMatIndex = 0; subMatIndex < numsurfs; ++subMatIndex )
    {
        skinnedCachedOffset = modelSurf->surf.skinnedCachedOffset;
        if ( modelSurf->surf.skinnedCachedOffset == -3 )
        {
            ++surfId;
            modelSurf = (GfxModelRigidSurface *)((char *)modelSurf + 4);
        }
        else
        {
            if ( !*material
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 1333, 0, "%s", "*material") )
            {
                __debugbreak();
            }
            if ( rgp.sortedMaterials[((*material)->info.drawSurf.packed >> 31) & 0xFFF] != *material
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                            1334,
                            0,
                            "%s",
                            "rgp.sortedMaterials[(*material)->info.drawSurf.fields.materialSortedIndex] == *material") )
            {
                __debugbreak();
            }
            if ( Material_GetTechnique(*material, techType) )
            {
                if ( drawSurf >= lastDrawSurf )
                {
                    R_WarnOncePerFrame(R_WARN_MAX_SCENE_DRAWSURFS, "R_AddXModelSurfaces");
                    return drawSurf;
                }
                if ( skinnedCachedOffset == -2 )
                {
                    surfType = 7;
                }
                else
                {
                    if ( skinnedCachedOffset != -1
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                                    1352,
                                    0,
                                    "%s\n\t(skinnedCachedOffset) = %i",
                                    "(skinnedCachedOffset == (-1))",
                                    skinnedCachedOffset) )
                    {
                        __debugbreak();
                    }
                    surfType = 8;
                }
                if ( surfId >= 0x10000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                                1361,
                                0,
                                "surfId doesn't index 1 << MTL_SORT_OBJECT_ID_BITS\n\t%i not in [0, %i)",
                                surfId,
                                0x10000) )
                {
                    __debugbreak();
                }
                HIDWORD(newDrawSurf) = ((surfType & 0xF) << 19) | HIDWORD((*material)->info.drawSurf.packed) & 0xFF87FFFF;
                LODWORD(newDrawSurf) = (unsigned __int16)surfId | *(unsigned int *)&(*material)->info.drawSurf.fields & 0xFFFF0000;
                drawSurf->packed = newDrawSurf;
                ++drawSurf;
                surfId += 14;
                ++modelSurf;
            }
            else
            {
                surfId += 14;
                ++modelSurf;
            }
        }
        ++material;
    }
    return drawSurf;
}

void __cdecl R_AddDObjSurfacesCamera(
                GfxSceneEntity *sceneEnt,
                unsigned __int16 lightingHandle,
                unsigned __int8 primaryLightIndex,
                GfxDrawSurf **drawSurfs,
                GfxDrawSurf **lastDrawSurfs,
                char visLightsMask,
                bool isMissileCamera)
{
    int packed_high; // ecx
    unsigned __int64 v9; // rax
    unsigned __int64 v10; // rax
    unsigned __int64 v11; // rax
    unsigned __int64 v12; // rax
    const XSurface *surf; // eax
    int NumTris; // eax
    const XSurface *v15; // eax
    float val; // [esp+0h] [ebp-B4h]
    float lodDist; // [esp+4h] [ebp-B0h]
    bool lodDist_4; // [esp+8h] [ebp-ACh]
    float v22; // [esp+10h] [ebp-A4h]
    unsigned __int8 entShaderConstantSetIndexExtraCam; // [esp+17h] [ebp-9Dh]
    char customIndex; // [esp+30h] [ebp-84h]
    bool infraredDraw; // [esp+3Bh] [ebp-79h]
    unsigned int surfId; // [esp+3Ch] [ebp-78h]
    unsigned int surfIda; // [esp+3Ch] [ebp-78h]
    DObjAnimMat *boneMatrix; // [esp+40h] [ebp-74h]
    int totalVertCount; // [esp+44h] [ebp-70h]
    const DObj *obj; // [esp+48h] [ebp-6Ch]
    GfxDrawSurf drawSurf; // [esp+4Ch] [ebp-68h] BYREF
    int totalTriCount; // [esp+58h] [ebp-5Ch]
    Material *const *material; // [esp+5Ch] [ebp-58h]
    XModel *model; // [esp+60h] [ebp-54h]
    unsigned int subMatIndex; // [esp+64h] [ebp-50h]
    unsigned int surfSize; // [esp+68h] [ebp-4Ch]
    unsigned int gfxEntIndexToUse; // [esp+6Ch] [ebp-48h]
    int skinnedCachedOffset; // [esp+70h] [ebp-44h]
    const GfxEntity *gfxEnt; // [esp+74h] [ebp-40h]
    int lod; // [esp+78h] [ebp-3Ch]
    unsigned int depthHack; // [esp+7Ch] [ebp-38h]
    void *modelSurf; // [esp+80h] [ebp-34h]
    int isShadowReceiver; // [esp+84h] [ebp-30h]
    surfaceType_t surfType; // [esp+88h] [ebp-2Ch]
    unsigned int region; // [esp+8Ch] [ebp-28h]
    unsigned int gfxEntIndex; // [esp+90h] [ebp-24h]
    unsigned int renderFxFlags; // [esp+94h] [ebp-20h]
    unsigned int numsurfs; // [esp+98h] [ebp-1Ch]
    unsigned int gfxEntIndex2; // [esp+9Ch] [ebp-18h]
    unsigned int modelIndex; // [esp+A0h] [ebp-14h]
    unsigned int modelCount; // [esp+A4h] [ebp-10h]
    GfxModelSkinnedSurface *skinnedSurf; // [esp+A8h] [ebp-Ch]
    bool iAmThePlayer; // [esp+AFh] [ebp-5h]
    int totalSurfCount; // [esp+B0h] [ebp-4h]

    isShadowReceiver = 0;
    modelSurf = sceneEnt->cull.skinnedSurfs.firstSurf;
    if ( modelSurf )
    {
        totalTriCount = 0;
        totalVertCount = 0;
        totalSurfCount = 0;
        obj = sceneEnt->obj;
        iassert(obj);
        iAmThePlayer = DObjIsPlayerShadow(obj);
        if ( !iAmThePlayer || !rg.enablePlayerShadowFlag )
        {
            boneMatrix = DObjGetRotTransArray(obj);
            modelCount = DObjGetNumModels(obj);

            bcassert(sceneEnt->reflectionProbeIndex, 8 /*1 << MTL_SORT_ENVMAP_BITS*/);
            bcassert(gfxDrawMethod.emissiveTechType, 130/*TECHNIQUE_COUNT*/);

            gfxEntIndex = sceneEnt->gfxEntIndex;
            if ( gfxEntIndex )
            {
                gfxEnt = &frontEndDataOut->gfxEnts[gfxEntIndex];
                renderFxFlags = gfxEnt->renderFxFlags;
                depthHack = (renderFxFlags & 2) != 0;
            }
            else
            {
                isShadowReceiver = 0;
                depthHack = 0;
            }
            gfxEntIndex2 = sceneEnt->gfxEntIndex2;
            gfxEntIndexToUse = gfxEntIndex;
            for ( modelIndex = 0; ; ++modelIndex )
            {
                if ( modelIndex >= modelCount )
                {
                    if ( r_showTriCounts->current.enabled )
                    {
                        R_AddXModelDebugString(sceneEnt->placement.base.origin, va("%i", totalTriCount));
                    }
                    else if ( r_showVertCounts->current.enabled )
                    {
                        R_AddXModelDebugString(sceneEnt->placement.base.origin, va("%i", totalVertCount));
                    }
                    else if ( r_showSurfCounts->current.enabled )
                    {
                        R_AddXModelDebugString(sceneEnt->placement.base.origin, va("%i", totalSurfCount));
                    }
                    return;
                }
                if ( sceneEnt->altXModelIndex )
                {
                    if ( sceneEnt->altXModelIndex == modelIndex + 1 )
                        gfxEntIndexToUse = gfxEntIndex2;
                    else
                        gfxEntIndexToUse = gfxEntIndex;
                }
                lod = sceneEnt->cull.lods[modelIndex];
                if ( lod < 0 )
                {
                    boneMatrix += XModelNumBones(DObjGetModel(obj, modelIndex));
                    continue;
                }
                model = DObjGetModel(obj, modelIndex);
                if ( !model
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 1504, 0, "%s", "model") )
                {
                    __debugbreak();
                }
                numsurfs = XModelGetSurfCount(model, lod);
                material = XModelGetSkins(model, lod);
                infraredDraw = 0;
                if ( gfxEntIndexToUse )
                    infraredDraw = (frontEndDataOut->gfxEnts[gfxEntIndexToUse].renderFxFlags & 0x80) != 0;
                if ( !material
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 1518, 0, "%s", "material") )
                {
                    __debugbreak();
                }
                subMatIndex = 0;
                while ( subMatIndex < numsurfs )
                {
                    if ( infraredDraw )
                        material = &rgp.infraredWhite;
                    skinnedSurf = (GfxModelSkinnedSurface *)modelSurf;
                    skinnedCachedOffset = *(unsigned int *)modelSurf;
                    if ( skinnedCachedOffset == -2 )
                    {
                        surfType = SF_BEGIN_XMODEL;
                        surfSize = 56;
                    }
                    else
                    {
                        if ( skinnedCachedOffset == -3 )
                        {
                            surfSize = 4;
                            goto LABEL_34;
                        }
                        surfType = SF_XMODEL_SKINNED;
                        surfSize = 24;
                    }
                    if ( !*material
                        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 1547, 0, "%s", "*material") )
                    {
                        __debugbreak();
                    }
                    if ( rgp.sortedMaterials[((*material)->info.drawSurf.packed >> 31) & 0xFFF] != *material
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                                    1548,
                                    0,
                                    "%s",
                                    "rgp.sortedMaterials[(*material)->info.drawSurf.fields.materialSortedIndex] == *material") )
                    {
                        __debugbreak();
                    }
                    region = (*material)->cameraRegion;
                    if ( region != 3 )
                    {
                        if ( drawSurfs[region] >= lastDrawSurfs[region] )
                        {
                            R_WarnOncePerFrame(R_WARN_MAX_SCENE_DRAWSURFS, "R_AddDObjSurfacesCamera");
                            break;
                        }
                        skinnedSurf->info.gfxEntIndex = gfxEntIndexToUse;
                        skinnedSurf->info.lightingHandle = lightingHandle;
                        skinnedSurf->info.dobjModelIndex = modelIndex;
                        surfId = (_BYTE *)modelSurf - (_BYTE *)frontEndDataOut;
                        if ( (((_BYTE)modelSurf - (_BYTE)frontEndDataOut) & 3) != 0
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                                        1588,
                                        0,
                                        "%s",
                                        "!(surfId & 3)") )
                        {
                            __debugbreak();
                        }
                        surfIda = surfId >> 2;
                        if ( surfIda >= 0x10000
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                                        1590,
                                        0,
                                        "surfId doesn't index 1 << MTL_SORT_OBJECT_ID_BITS\n\t%i not in [0, %i)",
                                        surfIda,
                                        0x10000) )
                        {
                            __debugbreak();
                        }
                        //packed_high = HIDWORD((*material)->info.drawSurf.packed);
                        //*(unsigned int *)&drawSurf.fields = (*material)->info.drawSurf.fields;
                        //HIDWORD(drawSurf.packed) = packed_high;
                        drawSurf.packed = (*material)->info.drawSurf.packed;
                        customIndex = isShadowReceiver != 0;
                        if ( gfxEntIndexToUse && frontEndDataOut->gfxEnts[gfxEntIndexToUse].destructibleBurnAmount > 0.0 )
                        {
                            customIndex |= 2u;
                            HIDWORD(drawSurf.packed) = HIDWORD(drawSurf.packed) & 0xFE7FFFFF | 0x1000000;
                        }
                        HIDWORD(drawSurf.packed) = ((((drawSurf.packed >> 58) & 0x3F) - depthHack) << 26)
                                                                         | HIDWORD(drawSurf.packed) & 0x3FFFFFF;
                        HIDWORD(drawSurf.packed) = ((surfType & 0xF) << 19) | HIDWORD(drawSurf.packed) & 0xFF87FFFF;
                        *(unsigned int *)&drawSurf.fields = (unsigned __int16)surfIda | *(unsigned int *)&drawSurf.fields & 0xFFFF0000;
                        v9 = (unsigned __int64)(sceneEnt->reflectionProbeIndex & 7) << 25;
                        *(unsigned int *)&drawSurf.fields = v9 | *(unsigned int *)&drawSurf.fields & 0xF1FFFFFF;
                        HIDWORD(drawSurf.packed) |= HIDWORD(v9);
                        v10 = (unsigned __int64)(customIndex & 0x1F) << 20;
                        *(unsigned int *)&drawSurf.fields = v10 | *(unsigned int *)&drawSurf.fields & 0xFE0FFFFF;
                        HIDWORD(drawSurf.packed) |= HIDWORD(v10);
                        HIDWORD(drawSurf.packed) = (primaryLightIndex << 11) | HIDWORD(drawSurf.packed) & 0xFFF807FF;
                        if ( isMissileCamera )
                            entShaderConstantSetIndexExtraCam = sceneEnt->entShaderConstantSetIndexExtraCam;
                        else
                            entShaderConstantSetIndexExtraCam = sceneEnt->entShaderConstantSetIndex;
                        R_XModelDrawSurfEncodeShaderConstantSet(&drawSurf, entShaderConstantSetIndexExtraCam);
                        v11 = (unsigned __int64)(visLightsMask & 1) << 29;
                        *(unsigned int *)&drawSurf.fields = v11 | *(unsigned int *)&drawSurf.fields & 0xDFFFFFFF;
                        HIDWORD(drawSurf.packed) |= HIDWORD(v11);
                        v12 = (unsigned __int64)((visLightsMask & 2) != 0) << 30;
                        *(unsigned int *)&drawSurf.fields = v12 | *(unsigned int *)&drawSurf.fields & 0xBFFFFFFF;
                        HIDWORD(drawSurf.packed) |= HIDWORD(v12);
                        drawSurfs[region]->fields = drawSurf.fields;
                        ++drawSurfs[region];
                        if ( r_showTriCounts->current.enabled )
                        {
                            surf = R_GetXSurface((unsigned int*)modelSurf, surfType);
                            NumTris = XSurfaceGetNumTris(surf);
                            totalTriCount += NumTris;
                        }
                        else if ( r_showVertCounts->current.enabled )
                        {
                            v15 = R_GetXSurface((unsigned int*)modelSurf, surfType);
                            totalVertCount += XSurfaceGetNumVerts(v15);
                        }
                    }
LABEL_34:
                    ++subMatIndex;
                    ++material;
                    modelSurf = (char *)modelSurf + surfSize;
                }
                lodDist_4 = (model->flags & 0x80000) != 0;
                lodDist = model->lodInfo[lod].dist;
                v22 = Vec3Distance(boneMatrix->trans, rg.lodParms.origin);
                val = I_fres(sceneEnt->placement.scale) * v22;
                ShodLodInfo(boneMatrix->trans, lod, model->numLods, val, lodDist, lodDist_4);
                boneMatrix += XModelNumBones(model);
                totalSurfCount += numsurfs;
            }
        }
    }
}

GfxDrawSurf *__cdecl R_AddDObjSurfaces(
                GfxSceneEntity *sceneEnt,
                unsigned __int8 techType,
                GfxDrawSurf *drawSurf,
                GfxDrawSurf *lastDrawSurf)
{
    int packed_high; // ecx
    unsigned __int64 v7; // rax
    bool v8; // [esp+7h] [ebp-6Dh]
    char customIndex; // [esp+18h] [ebp-5Ch]
    unsigned int surfId; // [esp+1Ch] [ebp-58h]
    unsigned int surfIda; // [esp+1Ch] [ebp-58h]
    DObj *obj; // [esp+20h] [ebp-54h]
    Material **material; // [esp+24h] [ebp-50h]
    XModel *model; // [esp+28h] [ebp-4Ch]
    unsigned int subMatIndex; // [esp+2Ch] [ebp-48h]
    unsigned int surfSize; // [esp+30h] [ebp-44h]
    GfxDrawSurf newDrawSurf; // [esp+34h] [ebp-40h] BYREF
    int skinnedCachedOffset; // [esp+40h] [ebp-34h]
    bool isShadowMapBuildTech; // [esp+47h] [ebp-2Dh]
    const GfxEntity *gfxEnt; // [esp+48h] [ebp-2Ch]
    int lod; // [esp+4Ch] [ebp-28h]
    unsigned int depthHack; // [esp+50h] [ebp-24h]
    void *modelSurf; // [esp+54h] [ebp-20h]
    surfaceType_t surfType; // [esp+58h] [ebp-1Ch]
    unsigned int gfxEntIndex; // [esp+5Ch] [ebp-18h]
    unsigned int renderFxFlags; // [esp+60h] [ebp-14h]
    unsigned int numsurfs; // [esp+64h] [ebp-10h]
    unsigned int modelIndex; // [esp+68h] [ebp-Ch]
    unsigned int modelCount; // [esp+6Ch] [ebp-8h]
    GfxModelSkinnedSurface *skinnedSurf; // [esp+70h] [ebp-4h]

    modelSurf = sceneEnt->cull.skinnedSurfs.firstSurf;
    if ( !modelSurf )
        return drawSurf;
    obj = Com_GetClientDObj(sceneEnt->entnum, sceneEnt->info.pose->localClientNum);
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 1695, 0, "%s", "obj") )
        __debugbreak();
    modelCount = DObjGetNumModels(obj);
    v8 = techType == 3 || techType == 2;
    isShadowMapBuildTech = v8;
    gfxEntIndex = sceneEnt->gfxEntIndex;
    if ( gfxEntIndex )
    {
        gfxEnt = &frontEndDataOut->gfxEnts[gfxEntIndex];
        renderFxFlags = gfxEnt->renderFxFlags;
        depthHack = (renderFxFlags & 2) != 0;
    }
    else
    {
        depthHack = 0;
    }
    for ( modelIndex = 0; modelIndex < modelCount; ++modelIndex )
    {
        lod = sceneEnt->cull.lods[modelIndex];
        if ( lod >= 0 )
        {
            model = DObjGetModel(obj, modelIndex);
            if ( !model
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 1720, 0, "%s", "model") )
            {
                __debugbreak();
            }
            numsurfs = XModelGetSurfCount(model, lod);
            material = XModelGetSkins(model, lod);
            if ( !material
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 1724, 0, "%s", "material") )
            {
                __debugbreak();
            }
            subMatIndex = 0;
LABEL_25:
            if ( subMatIndex >= numsurfs )
                continue;
            skinnedSurf = (GfxModelSkinnedSurface *)modelSurf;
            skinnedCachedOffset = *(unsigned int *)modelSurf;
            if ( skinnedCachedOffset == -2 )
            {
                surfType = SF_BEGIN_XMODEL;
                surfSize = 56;
            }
            else
            {
                if ( skinnedCachedOffset == -3 )
                {
                    surfSize = 4;
LABEL_24:
                    ++subMatIndex;
                    ++material;
                    modelSurf = (char *)modelSurf + surfSize;
                    goto LABEL_25;
                }
                surfType = SF_XMODEL_SKINNED;
                surfSize = 24;
            }
            if ( !*material
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 1749, 0, "%s", "*material") )
            {
                __debugbreak();
            }
            if ( rgp.sortedMaterials[((*material)->info.drawSurf.packed >> 31) & 0xFFF] != *material
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                            1750,
                            0,
                            "%s",
                            "rgp.sortedMaterials[(*material)->info.drawSurf.fields.materialSortedIndex] == *material") )
            {
                __debugbreak();
            }
            if ( Material_GetTechnique(*material, techType)
                && (!isShadowMapBuildTech || ((*material)->info.gameFlags & 0x40) != 0) )
            {
                if ( drawSurf >= lastDrawSurf )
                {
                    R_WarnOncePerFrame(R_WARN_MAX_SCENE_DRAWSURFS, "R_AddDObjSurfaces");
                    continue;
                }
                customIndex = 0;
                //packed_high = HIDWORD((*material)->info.drawSurf.packed);
                //*(unsigned int *)&newDrawSurf.fields = (*material)->info.drawSurf.fields;
                //HIDWORD(newDrawSurf.packed) = packed_high;
                newDrawSurf.packed = (*material)->info.drawSurf.packed;
                if ( sceneEnt->gfxEntIndex && frontEndDataOut->gfxEnts[sceneEnt->gfxEntIndex].destructibleBurnAmount > 0.0 )
                {
                    customIndex = 2;
                    HIDWORD(newDrawSurf.packed) = HIDWORD(newDrawSurf.packed) & 0xFE7FFFFF | 0x1000000;
                }
                surfId = (_BYTE *)modelSurf - (_BYTE *)frontEndDataOut;
                if ( (((_BYTE)modelSurf - (_BYTE)frontEndDataOut) & 3) != 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                                1779,
                                0,
                                "%s",
                                "!(surfId & 3)") )
                {
                    __debugbreak();
                }
                surfIda = surfId >> 2;
                if ( surfIda >= 0x10000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                                1781,
                                0,
                                "surfId doesn't index 1 << MTL_SORT_OBJECT_ID_BITS\n\t%i not in [0, %i)",
                                surfIda,
                                0x10000) )
                {
                    __debugbreak();
                }
                HIDWORD(newDrawSurf.packed) = ((((newDrawSurf.packed >> 58) & 0x3F) - depthHack) << 26)
                                                                        | HIDWORD(newDrawSurf.packed) & 0x3FFFFFF;
                HIDWORD(newDrawSurf.packed) = ((surfType & 0xF) << 19) | HIDWORD(newDrawSurf.packed) & 0xFF87FFFF;
                *(unsigned int *)&newDrawSurf.fields = (unsigned __int16)surfIda | *(unsigned int *)&newDrawSurf.fields & 0xFFFF0000;
                v7 = (unsigned __int64)(customIndex & 0x1F) << 20;
                *(unsigned int *)&newDrawSurf.fields = v7 | *(unsigned int *)&newDrawSurf.fields & 0xFE0FFFFF;
                HIDWORD(newDrawSurf.packed) |= HIDWORD(v7);
                R_XModelDrawSurfEncodeShaderConstantSet(&newDrawSurf, sceneEnt->entShaderConstantSetIndex);
                drawSurf->fields = newDrawSurf.fields;
                ++drawSurf;
            }
            goto LABEL_24;
        }
    }
    return drawSurf;
}

void __cdecl R_SetCharacterPrimaryLightScaleInfo(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms)
{
    float specularScale; // ecx

    if ( !viewInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 3606, 0, "%s", "viewInfo") )
    {
        __debugbreak();
    }
    if ( !sceneParms
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 3607, 0, "%s", "sceneParms") )
    {
        __debugbreak();
    }
    if ( r_heroLighting->current.enabled )
    {
        if ( r_primaryLightUseTweaks->current.enabled )
        {
            viewInfo->charPrimaryLightScale.diffuseScale = r_primaryLightTweakDiffuseStrength->current.value;
            viewInfo->charPrimaryLightScale.specularScale = r_primaryLightTweakSpecularStrength->current.value;
        }
        else
        {
            specularScale = sceneParms->charPrimaryLightScale.specularScale;
            viewInfo->charPrimaryLightScale.diffuseScale = sceneParms->charPrimaryLightScale.diffuseScale;
            viewInfo->charPrimaryLightScale.specularScale = specularScale;
        }
    }
    else
    {
        viewInfo->charPrimaryLightScale.diffuseScale = 1.0f;
        viewInfo->charPrimaryLightScale.specularScale = 1.0f;
    }
}

bool __cdecl R_EndFencePending()
{
    return R_HW_IsFencePending(&frontEndDataOut->endFence);
}

bool __cdecl R_DynamicFencePending()
{
    volatile unsigned int currentFrame; // [esp+4h] [ebp-4h]

    currentFrame = *frontEndDataOut->dynamicBufferCurrentFrame;
    return currentFrame && currentFrame < frontEndDataOut->frameCount;
}

void __cdecl R_SetEndTime(int endTime)
{
    rg.endTime = endTime;
}

void __cdecl R_WaitEndTime()
{
    PROF_SCOPED("max fps spin");
    while ( (int)(Sys_Milliseconds() - rg.endTime) < 0 )
        NET_Sleep(1u);
}

void __cdecl R_SetSunConstants(GfxCmdBufInput *input)
{
    const GfxLight *sun; // [esp+30h] [ebp-10h]

    sun = &input->data->sunLight;
    if ( sun->type != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    4082,
                    0,
                    "%s",
                    "sun->type == GFX_LIGHT_TYPE_DIR") )
    {
        __debugbreak();
    }
    R_SetInputCodeConstantFromVec4(input, CONST_SRC_CODE_SUN_POSITION, (float*)sun->dir);
    R_SetInputCodeConstant(input, CONST_SRC_CODE_SUN_DIFFUSE, sun->diffuseColor[0], sun->diffuseColor[1], sun->diffuseColor[2], 1.0);
    R_SetInputCodeConstant(input, CONST_SRC_CODE_SUN_SPECULAR, sun->specularColor[0], sun->specularColor[1], sun->specularColor[2], 1.0);
}

void __cdecl R_SetInputCodeConstant(GfxCmdBufInput *input, unsigned int constant, float x, float y, float z, float w)
{
    float *v6; // ecx

    if ( constant >= 0xC5
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.h",
                    1733,
                    0,
                    "constant doesn't index CONST_SRC_CODE_COUNT_FLOAT4\n\t%i not in [0, %i)",
                    constant,
                    197) )
    {
        __debugbreak();
    }
    if ( s_codeConstUpdateFreq[constant] != 2
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.h",
                    1735,
                    0,
                    "%s\n\t(constant) = %i",
                    "(s_codeConstUpdateFreq[constant] == MTL_UPDATE_RARELY)",
                    constant) )
    {
        __debugbreak();
    }
    if ( constant < 0x2F
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.h",
                    1737,
                    0,
                    "%s\n\t(constant) = %i",
                    "(!R_IsChangeablePixelShaderConst( constant ))",
                    constant) )
    {
        __debugbreak();
    }
    input->consts[constant][0] = x;
    v6 = input->consts[constant];
    v6[1] = y;
    v6[2] = z;
    v6[3] = w;
}

void __cdecl R_SetInputCodeConstantFromVec4(GfxCmdBufInput *input, unsigned int constant, const float *value)
{
    bcassert(constant, CONST_SRC_CODE_COUNT_FLOAT4);
    iassert(s_codeConstUpdateFreq[constant] == MTL_UPDATE_RARELY);

    if ( constant < 0x2F
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.h",
                    1748,
                    0,
                    "%s\n\t(constant) = %i",
                    "(!R_IsChangeablePixelShaderConst( constant ))",
                    constant) )
    {
        __debugbreak();
    }

    input->consts[constant][0] = value[0];
    input->consts[constant][1] = value[1];
    input->consts[constant][2] = value[2];
    input->consts[constant][3] = value[3];
}

void    R_SetHDRControlConstants(GfxCmdBufInput *input, const GfxViewInfo *viewInfo)
{
    float v3; // xmm0_4
    float v4; // xmm0_4
    float v5; // xmm0_4
    float v6; // xmm0_4
    float v7; // xmm0_4
    float v8; // xmm0_4
    float v9; // xmm0_4
    float v10; // xmm0_4
    float v11; // xmm0_4
    float v12; // xmm0_4
    float gametime; // [esp+20h] [ebp-17Ch]
    float v14; // [esp+28h] [ebp-174h]
    float v15; // [esp+34h] [ebp-168h]
    float v16; // [esp+40h] [ebp-15Ch]
    float v17; // [esp+4Ch] [ebp-150h]
    float vx; // [esp+58h] [ebp-144h]
    float vxa; // [esp+58h] [ebp-144h]
    float vxb; // [esp+58h] [ebp-144h]
    float vxc; // [esp+58h] [ebp-144h]
    float bpwpscale; // [esp+60h] [ebp-13Ch]
    float rangeC[4]; // [esp+64h] [ebp-138h]
    float rangeB[4]; // [esp+74h] [ebp-128h]
    float rangeA[4]; // [esp+84h] [ebp-118h]
    float Cadd[4]; // [esp+94h] [ebp-108h]
    float Cmul[5]; // [esp+A4h] [ebp-F8h]
    float Badd[4]; // [esp+B8h] [ebp-E4h]
    float Bmul[4]; // [esp+C8h] [ebp-D4h]
    float Amul[6]; // [esp+D8h] [ebp-C4h]
    float v31; // [esp+F0h] [ebp-ACh]
    float v32; // [esp+F4h] [ebp-A8h]
    float v33; // [esp+F8h] [ebp-A4h]
    int i; // [esp+FCh] [ebp-A0h]
    float bloomS; // [esp+100h] [ebp-9Ch]
    float curveE[5]; // [esp+104h] [ebp-98h]
    float curveS[5]; // [esp+118h] [ebp-84h]
    float remapE[5]; // [esp+12Ch] [ebp-70h]
    float remapS[5]; // [esp+140h] [ebp-5Ch]
    float linearE[5]; // [esp+154h] [ebp-48h]
    float linearS[5]; // [esp+168h] [ebp-34h]
    float blackPoint; // [esp+17Ch] [ebp-20h]
    float whitePoint; // [esp+180h] [ebp-1Ch]
    float exposure; // [esp+184h] [ebp-18h]
    float debugLayers; // [esp+188h] [ebp-14h]
    float w; // [esp+18Ch] [ebp-10h]
    //_UNKNOWN *v47; // [esp+190h] [ebp-Ch]
    //GfxCmdBufInput *inputa; // [esp+194h] [ebp-8h]
    //int vars0; // [esp+19Ch] [ebp+0h]
    //
    //v47 = a1;
    //inputa = (GfxCmdBufInput *)vars0;
    if (r_debugLayers->current.enabled)
        w = 1.0f;
    else
        w = 0.0f;
    debugLayers = w;
    R_SetInputCodeConstant(input, 0x89u, viewInfo->exposureRemap.remapMul[0], 0.0, 0.0, 0.0);
    R_SetInputCodeConstant(input, 0x8Au, 1.0, 0.0, 0.0, w);
    if (r_reflectionProbeGenerate->current.enabled)
    {
        R_SetInputCodeConstant(input, 0x89u, 0.0625, 1.0, 1.0, 0.0);
        R_SetInputCodeConstant(input, 0x8Au, 0.0, 0.0, 0.0, 0.0);
    }
    exposure = viewInfo->exposureValue.blackPoint[3];
    whitePoint = viewInfo->exposureValue.whitePoint[0];
    blackPoint = viewInfo->exposureValue.blackPoint[0];

    linearS[0] = viewInfo->exposureValue.linearStart[0];
    linearS[1] = viewInfo->exposureValue.linearStart[1];
    linearS[2] = viewInfo->exposureValue.linearStart[2];
    linearS[3] = viewInfo->exposureValue.linearStart[3];

    linearE[0] = viewInfo->exposureValue.linearEnd[0];
    linearE[1] = viewInfo->exposureValue.linearEnd[1];
    linearE[2] = viewInfo->exposureValue.linearEnd[2];
    linearE[3] = viewInfo->exposureValue.linearEnd[3];

    remapS[0] = viewInfo->exposureValue.remapStart[0];
    remapS[1] = viewInfo->exposureValue.remapStart[1];
    remapS[2] = viewInfo->exposureValue.remapStart[2];
    remapS[3] = viewInfo->exposureValue.remapStart[3];

    remapE[0] = viewInfo->exposureValue.remapEnd[0];
    remapE[1] = viewInfo->exposureValue.remapEnd[1];
    remapE[2] = viewInfo->exposureValue.remapEnd[2];
    remapE[3] = viewInfo->exposureValue.remapEnd[3];

    curveS[0] = viewInfo->exposureValue.scurveStart[0];
    curveS[1] = viewInfo->exposureValue.scurveStart[1];
    curveS[2] = viewInfo->exposureValue.scurveStart[2];
    curveS[3] = viewInfo->exposureValue.scurveStart[3];

    curveE[0] = viewInfo->exposureValue.scurveEnd[0];
    curveE[1] = viewInfo->exposureValue.scurveEnd[1];
    curveE[2] = viewInfo->exposureValue.scurveEnd[2];
    curveE[3] = viewInfo->exposureValue.scurveEnd[3];

    bloomS = 1.0f;
    for (i = 0; i < 3; ++i)
    {
        if (linearS[i] <= 0.99998474)
            v33 = linearS[i];
        else
            v33 = 0.99998474f;
        linearS[i] = v33;
        if (linearS[i] < linearE[i])
            v3 = linearE[i];
        else
            v3 = linearS[i] + 0.000015287891;
        v32 = v3;
        linearE[i] = v3;
        if (remapS[i] <= 0.99998474)
            v31 = remapS[i];
        else
            v31 = 0.99998474f;
        remapS[i] = v31;
        if (remapS[i] < remapE[i])
            v4 = remapE[i];
        else
            v4 = remapS[i] + 0.000015287891;
        Amul[5] = v4;
        remapE[i] = v4;
        if (linearS[i] == 0.0)
            Amul[4] = 0.0f;
        else
            Amul[4] = 1.0 / linearS[i];
        Amul[i] = Amul[4];
        Bmul[i] = 1.0 / (float)(linearE[i] - linearS[i]);
        //Badd[i] = COERCE_FLOAT(LODWORD(linearS[i]) ^ _mask__NegFloat_) * Bmul[i];
        Badd[i] = (-(linearS[i])) * Bmul[i];
        if (linearE[i] == 1.0)
            Cmul[4] = 0.0f;
        else
            Cmul[4] = 1.0 / (float)(1.0 - linearE[i]);
        Cmul[i] = Cmul[4];
        //Cadd[i] = COERCE_FLOAT(LODWORD(linearE[i]) ^ _mask__NegFloat_) * Cmul[i];
        Cadd[i] = (-(linearE[i])) * Cmul[i];
        curveS[i] = curveS[i] * 2.0;
        curveE[i] = curveE[i] * 2.0;
        rangeA[i] = remapS[i];
        rangeB[i] = remapE[i] - remapS[i];
        rangeC[i] = 1.0 - remapE[i];
    }
    bpwpscale = 1.0 / (float)(whitePoint - blackPoint);
    R_SetInputCodeConstant(input, 0x79u, bpwpscale * exposure, 0.0, 1.0 - bloomS, 1.0);
    //R_SetInputCodeConstant(input, 0x7Au, COERCE_FLOAT(LODWORD(blackPoint) ^ _mask__NegFloat_) * bpwpscale, 0.0, 0.0, 0.0);
    R_SetInputCodeConstant(input, 0x7Au, (-(blackPoint)) * bpwpscale, 0.0, 0.0, 0.0);
    R_SetInputCodeConstant(input, 0x7Bu, Amul[0], Amul[1], Amul[2], 1.0);
    R_SetInputCodeConstant(input, 0x7Cu, Bmul[0], Bmul[1], Bmul[2], 0.0);
    R_SetInputCodeConstant(input, 0x7Du, Badd[0], Badd[1], Badd[2], 0.0);
    R_SetInputCodeConstant(input, 0x7Eu, Cmul[0], Cmul[1], Cmul[2], 0.0);
    R_SetInputCodeConstant(input, 0x7Fu, Cadd[0], Cadd[1], Cadd[2], 0.0);
    R_SetInputCodeConstant(input, 0x80u, curveS[0], curveS[1], curveS[2], 0.0);
    R_SetInputCodeConstant(input, 0x81u, curveE[0], curveE[1], curveE[2], 0.0);
    R_SetInputCodeConstant(input, 0x82u, rangeA[0], rangeA[1], rangeA[2], 0.0);
    R_SetInputCodeConstant(input, 0x83u, rangeB[0], rangeB[1], rangeB[2], 0.0);
    R_SetInputCodeConstant(input, 0x84u, rangeC[0], rangeC[1], rangeC[2], 0.0);
    v5 = cos((float)(r_waterWaveAngle->current.value * 0.017453292));
    vx = v5;
    v6 = sin((float)(r_waterWaveAngle->current.value * 0.017453292));
    v17 = 6.2831855 / r_waterWaveWavelength->current.value;
    R_SetInputCodeConstant(input, 0x79u, vx * v17, v6 * v17, vx, v6);
    v7 = cos((float)(r_waterWaveAngle->current.vector[1] * 0.017453292));
    vxa = v7;
    v8 = sin((float)(r_waterWaveAngle->current.vector[1] * 0.017453292));
    v16 = 6.2831855 / r_waterWaveWavelength->current.vector[1];
    R_SetInputCodeConstant(input, 0x7Au, vxa * v16, v8 * v16, vxa, v8);
    v9 = cos((float)(r_waterWaveAngle->current.vector[2] * 0.017453292));
    vxb = v9;
    v10 = sin((float)(r_waterWaveAngle->current.vector[2] * 0.017453292));
    v15 = 6.2831855 / r_waterWaveWavelength->current.vector[2];
    R_SetInputCodeConstant(input, 0x7Bu, vxb * v15, v10 * v15, vxb, v10);
    v11 = cos((float)(r_waterWaveAngle->current.vector[3] * 0.017453292));
    vxc = v11;
    v12 = sin((float)(r_waterWaveAngle->current.vector[3] * 0.017453292));
    v14 = 6.2831855 / r_waterWaveWavelength->current.vector[3];
    R_SetInputCodeConstant(input, 0x7Cu, vxc * v14, v12 * v14, vxc, v12);
    gametime = viewInfo->sceneDef.floatTime;
    R_SetInputCodeConstant(
        input,
        0x7Du,
        (float)((float)(sqrtf(386.22 * v17) * gametime) * r_waterWaveSpeed->current.value) + r_waterWavePhase->current.value,
        (float)((float)(sqrtf(386.22 * v16) * gametime) * r_waterWaveSpeed->current.vector[1])
        + r_waterWavePhase->current.vector[1],
        (float)((float)(sqrtf(386.22 * v15) * gametime) * r_waterWaveSpeed->current.vector[2])
        + r_waterWavePhase->current.vector[2],
        (float)((float)(sqrtf(386.22 * v14) * gametime) * r_waterWaveSpeed->current.vector[3])
        + r_waterWavePhase->current.vector[3]);
    R_SetInputCodeConstant(
        input,
        0x7Eu,
        r_waterWaveAmplitude->current.value,
        r_waterWaveAmplitude->current.vector[1],
        r_waterWaveAmplitude->current.vector[2],
        r_waterWaveAmplitude->current.vector[3]);
    R_SetInputCodeConstant(
        input,
        0x7Fu,
        r_waterWaveSteepness->current.value,
        r_waterWaveSteepness->current.vector[1],
        r_waterWaveSteepness->current.vector[2],
        r_waterWaveSteepness->current.vector[3]);
}

void __cdecl R_GenerateMarkVertsForDynamicModels(const GfxLight *visibleLights, int visibleLightCount)
{
    GfxSceneModel *sceneModel; // [esp+4h] [ebp-24h]
    int dobjIndex; // [esp+8h] [ebp-20h]
    unsigned __int8 reflectionProbeIndex; // [esp+Fh] [ebp-19h]
    unsigned int indexCount; // [esp+10h] [ebp-18h] BYREF
    int brushModelIndex; // [esp+14h] [ebp-14h]
    unsigned __int16 entnum; // [esp+18h] [ebp-10h]
    unsigned __int16 lightHandle; // [esp+1Ch] [ebp-Ch]
    int modelIndex; // [esp+20h] [ebp-8h]
    GfxSceneEntity *sceneEntity; // [esp+24h] [ebp-4h]

    FX_BeginGeneratingMarkVertsForEntModels(scene.dpvs.localClientNum, &indexCount);
    for ( dobjIndex = 0; dobjIndex != scene.sceneDObjCount; ++dobjIndex )
    {
        sceneEntity = &scene.sceneDObj[dobjIndex];
        entnum = sceneEntity->entnum;
        if ( entnum < gfxCfg.entnumOrdinaryEnd && (scene.sceneDObjVisData[0][dobjIndex] & 1) != 0 )
        {
            //lightHandle = Ragdoll_HandleBody(sceneEntity->info.pose)->lightingHandle;
            lightHandle = (sceneEntity->info.pose)->lightingHandle;
            FX_GenerateMarkVertsForEntDObj(
                scene.dpvs.localClientNum,
                entnum,
                &indexCount,
                lightHandle,
                sceneEntity->reflectionProbeIndex,
                (DObj *)sceneEntity->obj,
                sceneEntity->info.pose,
                visibleLights,
                visibleLightCount);
        }
    }
    for ( modelIndex = 0; modelIndex != scene.sceneModelCount; ++modelIndex )
    {
        sceneModel = &scene.sceneModel[modelIndex];
        entnum = sceneModel->entnum;
        if ( entnum < gfxCfg.entnumOrdinaryEnd && (scene.sceneModelVisData[0][modelIndex] & 1) != 0 )
        {
            reflectionProbeIndex = sceneModel->reflectionProbeIndex;
            if ( !sceneModel->cachedLightingHandle
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                            3957,
                            0,
                            "%s",
                            "sceneModel->cachedLightingHandle") )
            {
                __debugbreak();
            }
            if ( sceneModel->obj )
                FX_GenerateMarkVertsForEntXModel(
                    scene.dpvs.localClientNum,
                    entnum,
                    &indexCount,
                    *sceneModel->cachedLightingHandle,
                    reflectionProbeIndex,
                    &sceneModel->placement,
                    visibleLights,
                    visibleLightCount);
        }
    }
    for ( brushModelIndex = 0; brushModelIndex != scene.sceneBrushCount; ++brushModelIndex )
    {
        entnum = scene.sceneBrush[brushModelIndex].entnum;
        if ( entnum < gfxCfg.entnumOrdinaryEnd && (scene.sceneBrushVisData[0][brushModelIndex] & 1) != 0 )
            FX_GenerateMarkVertsForEntBrush(
                scene.dpvs.localClientNum,
                entnum,
                &indexCount,
                scene.sceneBrush[brushModelIndex].reflectionProbeIndex,
                &scene.sceneBrush[brushModelIndex].placement,
                visibleLights,
                visibleLightCount);
    }
    FX_EndGeneratingMarkVertsForEntModels(scene.dpvs.localClientNum);
}

void    R_SetSkyDynamicIntensity(const float *viewForward, GfxCmdBufInput *input)
{
#if 0
    float value; // xmm0_4
    long double v4; // [esp+1Ch] [ebp-8Ch] BYREF
    char textBuff[64]; // [esp+28h] [ebp-80h]
    Font_s *v6; // [esp+68h] [ebp-40h]
    float textY; // [esp+6Ch] [ebp-3Ch]
    float textX; // [esp+70h] [ebp-38h]
    Font_s *font; // [esp+74h] [ebp-34h]
    float skyIntensity; // [esp+78h] [ebp-30h]
    float interp; // [esp+7Ch] [ebp-2Ch]
    float v12; // [esp+80h] [ebp-28h]
    float diff; // [esp+84h] [ebp-24h]
    float dotAngle1; // [esp+88h] [ebp-20h]
    float dotAngle0; // [esp+8Ch] [ebp-1Ch]
    float factor1; // [esp+90h] [ebp-18h]
    float factor0; // [esp+94h] [ebp-14h]
    //float angle0; // [esp+9Ch] [ebp-Ch]
    //float dot; // [esp+A0h] [ebp-8h]
    //float retaddr; // [esp+A8h] [ebp+0h]

    //angle0 = a1;
    //dot = retaddr;
    if ( r_reflectionProbeGenerate->current.enabled )
    {
        R_SetInputCodeConstant(input, 0xC2u, 1.0, 1.0, 1.0, 1.0);
    }
    else
    {
        factor0 = viewForward[2];
        if ( r_sky_intensity_useDebugValues->current.enabled )
        {
            factor1 = r_sky_intensity_angle0->current.value;
            dotAngle0 = r_sky_intensity_angle1->current.value;
            dotAngle1 = r_sky_intensity_factor0->current.value;
            value = r_sky_intensity_factor1->current.value;
        }
        else
        {
            factor1 = rgp.world->skyDynIntensity.angle0;
            dotAngle0 = rgp.world->skyDynIntensity.angle1;
            dotAngle1 = rgp.world->skyDynIntensity.factor0;
            value = rgp.world->skyDynIntensity.factor1;
        }
        diff = value;
        cos(v4);
        v12 = (float)(90.0 - factor1) * 0.017453292;
        cos(v4);
        interp = (float)(90.0 - dotAngle0) * 0.017453292;
        skyIntensity = interp - v12;
        if ( fabs(interp - v12) <= 0.000099999997 )
        {
            textX = 0.0f;
        }
        else
        {
            *(float *)&font = (float)(factor0 - v12) / skyIntensity;
            if ( 1.0 <= 0.0
                && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 730, 0, "%s", "min < max") )
            {
                __debugbreak();
            }
            if ( *(float *)&font >= 0.0 )
            {
                if ( *(float *)&font <= 1.0 )
                    textX = *(float *)&font;
                else
                    textX = 1.0f;
            }
            else
            {
                textX = 0.0f;
            }
            textX = textX * textX;
        }
        textY = (float)((float)(1.0 - textX) * dotAngle1) + (float)(diff * textX);
        if ( r_sky_intensity_showDebugDisplay->current.enabled )
        {
            v6 = R_RegisterFont("fonts/consoleFont", 1);
            *(float *)&textBuff[60] = 100.0f;
            *(float *)&textBuff[56] = 100.0f;
            _snprintf((char *)&v4, 0x40u, "intensity0 angle=%.2f factor=%.2f", factor1, dotAngle1);
            R_AddCmdDrawText(
                (char *)&v4,
                64,
                v6,
                *(float *)&textBuff[60],
                *(float *)&textBuff[56],
                1.5,
                2.0,
                0.0,
                colorRed,
                0);
            *(float *)&textBuff[56] = *(float *)&textBuff[56] + 20.0;
            _snprintf((char *)&v4, 0x40u, "intensity1 angle=%.2f factor=%.2f", dotAngle0, diff);
            R_AddCmdDrawText(
                (char *)&v4,
                64,
                v6,
                *(float *)&textBuff[60],
                *(float *)&textBuff[56],
                1.5,
                2.0,
                0.0,
                colorRed,
                0);
            *(float *)&textBuff[56] = *(float *)&textBuff[56] + 20.0;
            _snprintf((char *)&v4, 0x40u, "intensity=%.2f interp=%.2f", textY, textX);
            R_AddCmdDrawText(
                (char *)&v4,
                64,
                v6,
                *(float *)&textBuff[60],
                *(float *)&textBuff[56],
                1.5,
                2.0,
                0.0,
                colorRed,
                0);
        }
        R_SetInputCodeConstant(input, 0xC2u, textY, textY, textY, textY);
    }
#endif
    float angle0;
    float angle1;
    float factor0;
    float factor1;

    float dotZ;
    float cos0;
    float cos1;
    float t;
    float intensity;

    if (r_reflectionProbeGenerate->current.enabled)
    {
        R_SetInputCodeConstant(input, 0xC2u, 1.0f, 1.0f, 1.0f, 1.0f);
        return;
    }

    dotZ = viewForward[2];

    if (r_sky_intensity_useDebugValues->current.enabled)
    {
        angle0 = r_sky_intensity_angle0->current.value;
        angle1 = r_sky_intensity_angle1->current.value;
        factor0 = r_sky_intensity_factor0->current.value;
        factor1 = r_sky_intensity_factor1->current.value;
    }
    else
    {
        angle0 = rgp.world->skyDynIntensity.angle0;
        angle1 = rgp.world->skyDynIntensity.angle1;
        factor0 = rgp.world->skyDynIntensity.factor0;
        factor1 = rgp.world->skyDynIntensity.factor1;
    }

    /* convert degrees-from-horizon to cosine thresholds */
    cos0 = cosf((90.0f - angle0) * (float)M_PI / 180.0f);
    cos1 = cosf((90.0f - angle1) * (float)M_PI / 180.0f);

    if (fabsf(cos1 - cos0) <= 1e-4f)
    {
        t = 0.0f;
    }
    else
    {
        t = (dotZ - cos0) / (cos1 - cos0);

        /* clamp 0..1 */
        if (t < 0.0f)
            t = 0.0f;
        else if (t > 1.0f)
            t = 1.0f;

        /* smoothstep-style curve */
        t = t * t;
    }

    intensity = (1.0f - t) * factor0 + t * factor1;

    if (r_sky_intensity_showDebugDisplay->current.enabled)
    {
        Font_s *font = R_RegisterFont("fonts/consoleFont", 1);
        float x = 100.0f;
        float y = 100.0f;
        char text[64];

        _snprintf(text, sizeof(text),
            "intensity0 angle=%.2f factor=%.2f", angle0, factor0);
        R_AddCmdDrawText(text, 64, font, x, y, 1.5f, 2.0f, 0.0f, colorRed, 0);
        y += 20.0f;

        _snprintf(text, sizeof(text),
            "intensity1 angle=%.2f factor=%.2f", angle1, factor1);
        R_AddCmdDrawText(text, 64, font, x, y, 1.5f, 2.0f, 0.0f, colorRed, 0);
        y += 20.0f;

        _snprintf(text, sizeof(text),
            "intensity=%.2f interp=%.2f", intensity, t);
        R_AddCmdDrawText(text, 64, font, x, y, 1.5f, 2.0f, 0.0f, colorRed, 0);
    }

    R_SetInputCodeConstant(input, 0xC2u,
        intensity, intensity, intensity, intensity);
}

void __cdecl R_InitScene()
{
    scene.drawSurfs[0] = scene.bspDrawSurfs;
    scene.drawSurfs[1] = scene.smodelDrawSurfsLight;
    scene.drawSurfs[2] = scene.entDrawSurfsLight;
    scene.drawSurfs[9] = scene.codemeshDrawSurfsLight;
    scene.drawSurfs[3] = scene.bspDrawSurfsDecal;
    scene.drawSurfs[4] = scene.smodelDrawSurfsDecal;
    scene.drawSurfs[5] = scene.entDrawSurfsDecal;
    scene.drawSurfs[6] = scene.fxDrawSurfsLight;
    scene.drawSurfs[7] = scene.fxDrawSurfsLightAuto;
    scene.drawSurfs[8] = scene.fxDrawSurfsLightDecal;
    scene.drawSurfs[10] = scene.bspDrawSurfsEmissive;
    scene.drawSurfs[11] = scene.smodelDrawSurfsEmissive;
    scene.drawSurfs[12] = scene.entDrawSurfsEmissive;
    scene.drawSurfs[13] = scene.fxDrawSurfsEmissive;
    scene.drawSurfs[14] = scene.fxDrawSurfsEmissiveAuto;
    scene.drawSurfs[15] = scene.fxDrawSurfsEmissiveDecal;
    scene.drawSurfs[16] = scene.reflectedFxDrawSurfsEmissive;
    scene.drawSurfs[17] = scene.reflectedFxDrawSurfsEmissiveAuto;
    scene.drawSurfs[18] = scene.reflectedFxDrawSurfsEmissiveDecal;
    scene.drawSurfs[19] = scene.bspSunShadowDrawSurfs0;
    scene.drawSurfs[20] = scene.smodelSunShadowDrawSurfs0;
    scene.drawSurfs[21] = scene.entSunShadowDrawSurfs0;
    scene.drawSurfs[22] = scene.codemeshSunShadowDrawSurfs0;
    scene.drawSurfs[23] = scene.bspSunShadowDrawSurfs1;
    scene.drawSurfs[24] = scene.smodelSunShadowDrawSurfs1;
    scene.drawSurfs[25] = scene.entSunShadowDrawSurfs1;
    scene.drawSurfs[26] = scene.codemeshSunShadowDrawSurfs1;
    scene.drawSurfs[27] = scene.bspSpotShadowDrawSurfs0;
    scene.drawSurfs[28] = scene.smodelSpotShadowDrawSurfs0;
    scene.drawSurfs[29] = scene.entSpotShadowDrawSurfs0;
    scene.drawSurfs[30] = scene.bspSpotShadowDrawSurfs1;
    scene.drawSurfs[31] = scene.smodelSpotShadowDrawSurfs1;
    scene.drawSurfs[32] = scene.entSpotShadowDrawSurfs1;
    scene.drawSurfs[33] = scene.bspSpotShadowDrawSurfs2;
    scene.drawSurfs[34] = scene.smodelSpotShadowDrawSurfs2;
    scene.drawSurfs[35] = scene.entSpotShadowDrawSurfs2;
    scene.drawSurfs[36] = scene.bspSpotShadowDrawSurfs3;
    scene.drawSurfs[37] = scene.smodelSpotShadowDrawSurfs3;
    scene.drawSurfs[38] = scene.entSpotShadowDrawSurfs3;
    scene.maxDrawSurfCount[0] = 0x2000;
    scene.maxDrawSurfCount[1] = 0x2000;
    scene.maxDrawSurfCount[2] = 0x2000;
    scene.maxDrawSurfCount[9] = 1024;
    scene.maxDrawSurfCount[3] = 512;
    scene.maxDrawSurfCount[4] = 512;
    scene.maxDrawSurfCount[5] = 512;
    scene.maxDrawSurfCount[6] = 0x2000;
    scene.maxDrawSurfCount[7] = 0x2000;
    scene.maxDrawSurfCount[8] = 0x2000;
    scene.maxDrawSurfCount[10] = 0x2000;
    scene.maxDrawSurfCount[11] = 0x2000;
    scene.maxDrawSurfCount[12] = 0x2000;
    scene.maxDrawSurfCount[13] = 0x2000;
    scene.maxDrawSurfCount[14] = 0x2000;
    scene.maxDrawSurfCount[15] = 0x2000;
    scene.maxDrawSurfCount[16] = 0x2000;
    scene.maxDrawSurfCount[17] = 0x2000;
    scene.maxDrawSurfCount[18] = 0x2000;
    scene.maxDrawSurfCount[19] = 4096;
    scene.maxDrawSurfCount[20] = 4096;
    scene.maxDrawSurfCount[21] = 4096;
    scene.maxDrawSurfCount[22] = 64;
    scene.maxDrawSurfCount[23] = 0x2000;
    scene.maxDrawSurfCount[24] = 0x2000;
    scene.maxDrawSurfCount[25] = 0x2000;
    scene.maxDrawSurfCount[26] = 64;
    scene.maxDrawSurfCount[27] = 256;
    scene.maxDrawSurfCount[28] = 256;
    scene.maxDrawSurfCount[29] = 512;
    scene.maxDrawSurfCount[30] = 256;
    scene.maxDrawSurfCount[31] = 256;
    scene.maxDrawSurfCount[32] = 512;
    scene.maxDrawSurfCount[33] = 256;
    scene.maxDrawSurfCount[34] = 256;
    scene.maxDrawSurfCount[35] = 512;
    scene.maxDrawSurfCount[36] = 256;
    scene.maxDrawSurfCount[37] = 256;
    scene.maxDrawSurfCount[38] = 512;
}

void __cdecl R_ClearScene(int localClientNum)
{
    unsigned int viewIndex; // [esp+4h] [ebp-4h]

    if ( !rg.inFrame
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 6667, 0, "%s", "rg.inFrame") )
    {
        __debugbreak();
    }
    if ( !Sys_IsMainThread()
        && !Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    6668,
                    0,
                    "%s",
                    "Sys_IsMainThread() || Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    scene.dpvs.localClientNum = localClientNum;
    Com_Memset((unsigned int *)scene.sceneDObj, 0, 132 * scene.sceneDObjCount);
    Com_Memset((unsigned int *)&scene.sceneModel[0].info, 0, 76 * scene.sceneModelCount);
    Com_Memset((unsigned int *)&scene.sceneBrush[0].info.surfId, 0, 44 * scene.sceneBrushCount);
    Com_Memset((unsigned int *)scene.glassBrush, 0, 40 * scene.glassBrushCount);
    scene.addedLightCount = 0;
    memset((unsigned __int8 *)scene.drawSurfCount, 0, sizeof(scene.drawSurfCount));
    for ( viewIndex = 0; viewIndex < 7; ++viewIndex )
        Com_Memset((unsigned int *)scene.sceneModelVisData[viewIndex], 1, scene.sceneModelCount);
    scene.sceneDObjCount = 0;
    scene.sceneModelCount = 0;
    scene.sceneBrushCount = 0;
    scene.glassBrushCount = 0;
    if ( rgp.world )
        R_ClearDpvsScene();
    R_ClearSceneDynSModelVisBits();
}

void R_ClearSceneDynSModelVisBits()
{
    memset((unsigned __int8 *)scene.dynSModelVisBitsCamera, 0, sizeof(scene.dynSModelVisBitsCamera));
    memset((unsigned __int8 *)scene.dynSModelVisBitsSunShadow, 0, sizeof(scene.dynSModelVisBitsSunShadow));
}

void __cdecl R_ClearScene_For_ExtraCam(int localClientNum)
{
    unsigned int viewIndex; // [esp+4h] [ebp-4h]

    if ( !rg.inFrame
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 6711, 0, "%s", "rg.inFrame") )
    {
        __debugbreak();
    }
    if ( !Sys_IsMainThread()
        && !Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    6712,
                    0,
                    "%s",
                    "Sys_IsMainThread() || Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    scene.dpvs.localClientNum = localClientNum;
    scene.addedLightCount = 0;
    for ( viewIndex = 0; viewIndex < 7; ++viewIndex )
        Com_Memset((unsigned int *)scene.sceneModelVisData[viewIndex], 1, scene.sceneModelCount);
    scene.drawSurfCount[6] = 0;
    scene.drawSurfCount[7] = 0;
    scene.drawSurfCount[8] = 0;
    scene.drawSurfCount[13] = 0;
    scene.drawSurfCount[14] = 0;
    scene.drawSurfCount[15] = 0;
    scene.drawSurfCount[16] = 0;
    scene.drawSurfCount[17] = 0;
    scene.drawSurfCount[18] = 0;
    if ( rgp.world )
        R_ClearDpvsScene_For_ExtraCam();
    R_ClearSceneDynSModelVisBits();
}

int __cdecl R_GetLocalClientNum()
{
    return scene.dpvs.localClientNum;
}

void __cdecl R_SetLodOrigin(const refdef_s *refdef)
{
    if ( r_lockPvs->modified )
    {
        Dvar_ClearModified(r_lockPvs);
        R_SetViewParmsForScene(refdef, &lockPvsViewParms);
    }
    R_UpdateLodParms(refdef, &rg.lodParms);
    scene.def.viewOffset[0] = refdef->vieworg[0];
    scene.def.viewOffset[1] = refdef->vieworg[1];
    scene.def.viewOffset[2] = refdef->vieworg[2];
    scene.def.time = refdef->time;
    scene.def.floatTime = (float)scene.def.time * 0.001;
    R_UpdateFrameFog(refdef->localClientNum);
    R_UpdateFrameSun();
}

void __cdecl R_UpdateFrameFog(unsigned int localClientNum)
{
    float lerpPos; // [esp+20h] [ebp-Ch]
    int fadeTime; // [esp+24h] [ebp-8h]
    GfxClientFog *f; // [esp+28h] [ebp-4h]

    if ( localClientNum >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    873,
                    0,
                    "localClientNum doesn't index ARRAY_COUNT(rg.clientFogs)\n\t%i not in [0, %i)",
                    localClientNum,
                    4) )
    {
        __debugbreak();
    }
    f = &rg.clientFogs[localClientNum];
    if ( scene.def.time < f->settings[4].finishTime )
    {
        fadeTime = f->settings[4].finishTime - f->settings[4].startTime;
        if ( fadeTime <= 0 )
            fadeTime = 1;
        lerpPos = (float)(scene.def.time - f->settings[4].startTime) / (float)fadeTime;
        if ( lerpPos > 1.0 )
            lerpPos = 1.0f;
        f->settings[2].fogStart = (float)((float)(f->settings[4].fogStart - f->settings[3].fogStart) * lerpPos)
                                                        + f->settings[3].fogStart;
        f->settings[2].heightDensity = (float)((float)(f->settings[4].heightDensity - f->settings[3].heightDensity) * lerpPos)
                                                                 + f->settings[3].heightDensity;
        f->settings[2].density = (float)((float)(f->settings[4].density - f->settings[3].density) * lerpPos)
                                                     + f->settings[3].density;
        f->settings[2].baseHeight = (float)((float)(f->settings[4].baseHeight - f->settings[3].baseHeight) * lerpPos)
                                                            + f->settings[3].baseHeight;
        lerp_0(
            (float (*)[4])f->settings[2].color,
            (const float (*)[4])f->settings[3].color,
            (const float (*)[4])f->settings[4].color,
            lerpPos);
        lerp_0(
            (float (*)[4])f->settings[2].sunFogColor,
            (const float (*)[4])f->settings[3].sunFogColor,
            (const float (*)[4])f->settings[4].sunFogColor,
            lerpPos);
        lerp_1(
            (float (*)[3])f->settings[2].sunFogDir,
            (const float (*)[3])f->settings[3].sunFogDir,
            (const float (*)[3])f->settings[4].sunFogDir,
            lerpPos);
        Vec3Normalize(f->settings[2].sunFogDir);
        f->settings[2].sunFogStartAng = Lerp(f->settings[3].sunFogStartAng, f->settings[4].sunFogStartAng, lerpPos);
        f->settings[2].sunFogEndAng = Lerp(f->settings[3].sunFogEndAng, f->settings[4].sunFogEndAng, lerpPos);
        f->settings[2].maxDensity = (float)((float)(f->settings[4].maxDensity - f->settings[3].maxDensity) * lerpPos)
                                                            + f->settings[3].maxDensity;
    }
    else
    {
        memcpy(&f->settings[2], &f->settings[4], sizeof(f->settings[2]));
    }
    if ( f->index && !r_fog_disable->current.enabled )
        memcpy(&frontEndDataOut->fogSettings, &f->settings[2], sizeof(frontEndDataOut->fogSettings));
    else
        memset((unsigned __int8 *)&frontEndDataOut->fogSettings, 0, sizeof(frontEndDataOut->fogSettings));
}

double __cdecl Lerp(float from, float to, float t)
{
    return (to - from) * t + from;
}

void __cdecl R_SetViewParmsForScene(const refdef_s *refdef, GfxViewParms *viewParms)
{
    float v2; // [esp+Ch] [ebp-28h]
    float DefaultNearClip; // [esp+10h] [ebp-24h]

    memset((unsigned __int8 *)viewParms, 0, sizeof(GfxViewParms));
    viewParms->origin[0] = refdef->vieworg[0];
    viewParms->origin[1] = refdef->vieworg[1];
    viewParms->origin[2] = refdef->vieworg[2];
    viewParms->origin[3] = 1.0f;
    viewParms->axis[0][0] = refdef->viewaxis[0][0];
    viewParms->axis[0][1] = refdef->viewaxis[0][1];
    viewParms->axis[0][2] = refdef->viewaxis[0][2];
    viewParms->axis[1][0] = refdef->viewaxis[1][0];
    viewParms->axis[1][1] = refdef->viewaxis[1][1];
    viewParms->axis[1][2] = refdef->viewaxis[1][2];
    viewParms->axis[2][0] = refdef->viewaxis[2][0];
    viewParms->axis[2][1] = refdef->viewaxis[2][1];
    viewParms->axis[2][2] = refdef->viewaxis[2][2];
    MatrixForViewer(viewParms->origin, viewParms->axis, viewParms->viewMatrix.m);
    if ( refdef->zNear <= 0.0 )
        DefaultNearClip = R_GetDefaultNearClip();
    else
        DefaultNearClip = refdef->zNear;
    if ( (float)(1.0 - DefaultNearClip) < 0.0 )
        v2 = DefaultNearClip;
    else
        v2 = MINIMUM_Z_NEAR;
    viewParms->zNear = v2;
    R_SetupProjection(refdef->tanHalfFovX, refdef->tanHalfFovY, viewParms);
    R_SetupViewProjectionMatrices(viewParms, 1);
}

void __cdecl R_SetupProjection(float tanHalfFovX, float tanHalfFovY, GfxViewParms *viewParms)
{
    InfinitePerspectiveMatrix(tanHalfFovX, tanHalfFovY, viewParms->zNear, viewParms->projectionMatrix.m);
    viewParms->depthHackNearClip = -r_znear_depthhack->current.value;
}

void R_UpdateFrameSun()
{
    BOOL v1; // [esp+Ch] [ebp-38h]
    int v2; // [esp+10h] [ebp-34h]
    int v3; // [esp+14h] [ebp-30h]
    float *specularColor; // [esp+38h] [ebp-Ch]
    float *diffuseColor; // [esp+3Ch] [ebp-8h]
    float *dir; // [esp+40h] [ebp-4h]

    iassert(rgp.world);
    iassert(rgp.world->sunLight);
    
    memcpy(&frontEndDataOut->sunLight, rgp.world->sunLight, sizeof(frontEndDataOut->sunLight));
    if (rg.useSunDirOverride)
    {
        if (rg.useSunDirLerp)
        {
            R_LerpDir(
                rg.sunDirOverride,
                rg.sunDirOverrideTarget,
                rg.sunDirLerpBeginTime,
                rg.sunDirLerpEndTime,
                scene.def.time,
                frontEndDataOut->sunLight.dir);
        }
        else
        {
            dir = frontEndDataOut->sunLight.dir;
            frontEndDataOut->sunLight.dir[0] = rg.sunDirOverride[0];
            dir[1] = rg.sunDirOverride[1];
            dir[2] = rg.sunDirOverride[2];
        }
    }
    if (rg.useSunLightOverride)
    {
        diffuseColor = frontEndDataOut->sunLight.diffuseColor;
        frontEndDataOut->sunLight.diffuseColor[0] = rg.diffuseSunLightOverride[0];
        diffuseColor[1] = rg.diffuseSunLightOverride[1];
        diffuseColor[2] = rg.diffuseSunLightOverride[2];
        specularColor = frontEndDataOut->sunLight.specularColor;
        frontEndDataOut->sunLight.specularColor[0] = rg.specularSunLightOverride[0];
        specularColor[1] = rg.specularSunLightOverride[1];
        specularColor[2] = rg.specularSunLightOverride[2];
    }
    v2 = 0;
    if (sm_enable->current.enabled)
    {
        //if (rg.useSunDirOverride
        //    || (*(float *)(r_lightTweakSunDirection.integer + 24) != *(float *)(r_lightTweakSunDirection.integer + 56)
        //        || *(float *)(r_lightTweakSunDirection.integer + 28) != *(float *)(r_lightTweakSunDirection.integer + 60)
        //        || *(float *)(r_lightTweakSunDirection.integer + 32) != *(float *)(r_lightTweakSunDirection.integer + 64)
        //        ? (v3 = 0)
        //        : (v3 = 1),
        //        !v3))
        //{
        //    v2 = 1;
        //}
        if (rg.useSunDirOverride
            || ((r_lightTweakSunDirection->reset.vector[0] != r_lightTweakSunDirection->current.vector[0]
                || r_lightTweakSunDirection->reset.vector[1] != r_lightTweakSunDirection->current.vector[1]
                || r_lightTweakSunDirection->reset.vector[2] != r_lightTweakSunDirection->current.vector[2])
                ? (v3 = 0)
                : (v3 = 1),
                !v3))
        {
            v2 = 1;
        }
    }
    frontEndDataOut->prim.hasSunDirChanged = v2;
    v1 = rg.useSunDirOverride
        || AngleDelta(r_lightTweakSunDirection->current.vector[0], r_lightTweakSunDirection->reset.vector[0]) > 5.0
        || AngleDelta(r_lightTweakSunDirection->current.vector[1], r_lightTweakSunDirection->reset.vector[1]) > 5.0;
        //|| AngleNormalize180(*(float *)(r_lightTweakSunDirection.integer + 24) - *(float *)(r_lightTweakSunDirection.integer + 56)) > 5.0 
        //|| AngleNormalize180(*(float *)(r_lightTweakSunDirection.integer + 28) - *(float *)(r_lightTweakSunDirection.integer + 60)) > 5.0;
    frontEndDataOut->hasApproxSunDirChanged = v1;
}

void __cdecl R_LerpDir(
                const float *dirBegin,
                const float *dirEnd,
                int beginLerpTime,
                int endLerpTime,
                int currTime,
                float *result)
{
    float fraction; // [esp+8h] [ebp-18h]
    float v7; // [esp+18h] [ebp-8h]
    float lerpFraction; // [esp+1Ch] [ebp-4h]

    if ( endLerpTime <= beginLerpTime
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    6981,
                    0,
                    "%s",
                    "endLerpTime > beginLerpTime") )
    {
        __debugbreak();
    }
    lerpFraction = (float)(currTime - beginLerpTime) / (float)(endLerpTime - beginLerpTime);
    if ( (float)(lerpFraction - 1.0) < 0.0 )
        v7 = (float)(currTime - beginLerpTime) / (float)(endLerpTime - beginLerpTime);
    else
        v7 = 1.0f;
    if ( (float)(0.0 - lerpFraction) < 0.0 )
        fraction = v7;
    else
        fraction = 0.0f;
    Vec3Lerp(dirBegin, dirEnd, fraction, result);
    Vec3Normalize(result);
}

void __cdecl R_UpdateLodParms(const refdef_s *refdef, GfxLodParms *lodParms)
{
    float v2; // [esp+4h] [ebp-2Ch]
    float v3; // [esp+8h] [ebp-28h]
    float value; // [esp+14h] [ebp-1Ch]
    float v5; // [esp+18h] [ebp-18h]
    float invFovScale; // [esp+2Ch] [ebp-4h]

    if ( r_lockPvs->current.enabled )
    {
        lodParms->origin[0] = lockPvsViewParms.origin[0];
        lodParms->origin[1] = lockPvsViewParms.origin[1];
        lodParms->origin[2] = lockPvsViewParms.origin[2];
    }
    else
    {
        lodParms->origin[0] = refdef->vieworg[0];
        lodParms->origin[1] = refdef->vieworg[1];
        lodParms->origin[2] = refdef->vieworg[2];
    }
    lodParms->extraCamOriginValid = refdef->extraCamPosValid;
    lodParms->extraCamOrigin[0] = refdef->extraCamPos[0];
    lodParms->extraCamOrigin[1] = refdef->extraCamPos[1];
    lodParms->extraCamOrigin[2] = refdef->extraCamPos[2];
    invFovScale = refdef->tanHalfFovY * 2.1186731;
    value = r_lodScaleRigid->current.value;
    v5 = r_lodBiasRigid->current.value;
    if ( invFovScale < lodParms->ramp[0].appliedInvScale )
    {
        if ( (float)(lodParms->ramp[0].appliedInvScale / invFovScale) > r_fovScaleThresholdRigid->current.value )
            lodParms->ramp[0].appliedInvScale = invFovScale;
    }
    else
    {
        lodParms->ramp[0].appliedInvScale = invFovScale;
    }
    lodParms->ramp[0].scale = value * lodParms->ramp[0].appliedInvScale;
    lodParms->ramp[0].bias = v5 * lodParms->ramp[0].appliedInvScale;
    v2 = r_lodScaleSkinned->current.value;
    v3 = r_lodBiasSkinned->current.value;
    if ( invFovScale < lodParms->ramp[1].appliedInvScale )
    {
        if ( (float)(lodParms->ramp[1].appliedInvScale / invFovScale) > r_fovScaleThresholdSkinned->current.value )
            lodParms->ramp[1].appliedInvScale = invFovScale;
    }
    else
    {
        lodParms->ramp[1].appliedInvScale = invFovScale;
    }
    lodParms->ramp[1].scale = v2 * lodParms->ramp[1].appliedInvScale;
    lodParms->ramp[1].bias = v3 * lodParms->ramp[1].appliedInvScale;
    lodParms->valid = 1;
}

void __cdecl R_RenderScene(refdef_s *refdef, int frameTime)
{
    GfxViewParms *v2; // [esp+Ch] [ebp-14F8h]
    GfxSceneParms sceneParms; // [esp+1Ch] [ebp-14E8h] BYREF
    GfxViewParms *viewParms; // [esp+1500h] [ebp-4h]

    if ( refdef->tanHalfFovX <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    7168,
                    0,
                    "%s",
                    "refdef->tanHalfFovX > 0") )
    {
        __debugbreak();
    }
    if ( refdef->tanHalfFovY <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    7169,
                    0,
                    "%s",
                    "refdef->tanHalfFovY > 0") )
    {
        __debugbreak();
    }
    if ( !refdef->height
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 7170, 0, "%s", "refdef->height > 0") )
    {
        __debugbreak();
    }
    if ( !refdef->width
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 7171, 0, "%s", "refdef->width > 0") )
    {
        __debugbreak();
    }
    if ( refdef->localClientNum >= gfxCfg.maxClientViews
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    7172,
                    0,
                    "refdef->localClientNum doesn't index gfxCfg.maxClientViews\n\t%i not in [0, %i)",
                    refdef->localClientNum,
                    gfxCfg.maxClientViews) )
    {
        __debugbreak();
    }
    if ( rg.registered && !r_norefresh->current.enabled )
    {
        if ( r_logFile->current.integer )
            RB_LogPrint("====== R_RenderScene ======\n");
        if ( !rgp.world )
            Com_Error(ERR_DROP, "R_RenderScene: NULL worldmodel");
        rg.viewOrg[0] = refdef->vieworg[0];
        rg.viewOrg[1] = refdef->vieworg[1];
        rg.viewOrg[2] = refdef->vieworg[2];
        *(_QWORD *)rg.viewDir = *(_QWORD *)&refdef->viewaxis[0][0];
        rg.viewDir[2] = refdef->viewaxis[0][2];
        R_MaterialParameterTweakUpdate();
        R_UpdateExposureValue(refdef);
        R_CheckShadowMapVolumes(refdef->vieworg);
        viewParms = R_AllocViewParms();
        R_SetViewParmsForScene(refdef, viewParms);
        R_SetSceneParms(refdef, &sceneParms);
        R_CorrectLodScale(refdef);
        R_WorldLod_FrameUpdate(scene.def.floatTime, viewParms->origin, refdef->localClientNum);
        if ( scene.def.time != refdef->time
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                        7213,
                        0,
                        "%s",
                        "scene.def.time == refdef->time") )
        {
            __debugbreak();
        }
        if ( !rg.lodParms.valid
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                        7215,
                        0,
                        "%s",
                        "rg.lodParms.valid") )
        {
            __debugbreak();
        }
        if ( r_lockPvs->current.enabled )
            v2 = &lockPvsViewParms;
        else
            v2 = viewParms;
        frontEndDataOut->hideMatureContent = refdef->hideMatureContent;
        frontEndDataOut->splitscreen = refdef->splitscreen;
        frontEndDataOut->extraCamLargeFrameSize = 1;
        R_GenerateSortedDrawSurfs(&sceneParms, v2, viewParms, frameTime, refdef, 0);
    }
}

void __cdecl R_CheckShadowMapVolumes(const float *position)
{
    unsigned int j; // [esp+0h] [ebp-18h]
    unsigned int control; // [esp+4h] [ebp-14h]
    unsigned int i; // [esp+Ch] [ebp-Ch]
    unsigned int planeIndex; // [esp+10h] [ebp-8h]

    if ( sm_sunShadowSmall->current.enabled )
        Dvar_SetBoolIfChanged((dvar_s *)sm_sunShadowSmallEnable, 1);
    else
        Dvar_SetBoolIfChanged((dvar_s *)sm_sunShadowSmallEnable, 0);
    Dvar_SetBoolIfChanged((dvar_s *)sm_sunEnable, 1);
    Dvar_SetBoolIfChanged((dvar_s *)sm_spotEnable, 1);
    if ( rgp.world->shadowMapVolumeCount )
    {
        i = 0;
        while ( 2 )
        {
            control = rgp.world->shadowMapVolumes[i].control;
            planeIndex = (unsigned __int16)control;
            for ( j = 0; ; ++j )
            {
                if ( j >= (unsigned int)BYTE2(control) + 1 )
                {
                    if ((control & 0x1000000) != 0)
                        Dvar_SetBoolIfChanged((dvar_s *)sm_sunEnable, 0);
                    if ((control & 0x2000000) != 0)
                        Dvar_SetBoolIfChanged((dvar_s *)sm_sunShadowSmallEnable, 1);
                    if ( (control & 0x4000000) != 0 )
                        Dvar_SetBoolIfChanged((dvar_s *)sm_spotEnable, 0);
                    return;
                }
                if ( (float)((float)((float)(rgp.world->shadowMapVolumePlanes[planeIndex].plane[2] * position[2])
                                                     + (float)((float)(rgp.world->shadowMapVolumePlanes[planeIndex].plane[1] * position[1])
                                                                     + (float)(rgp.world->shadowMapVolumePlanes[planeIndex].plane[0] * *position)))
                                     + rgp.world->shadowMapVolumePlanes[planeIndex].plane[3]) < 0.0 )
                    break;
                ++planeIndex;
            }
            if ( ++i < rgp.world->shadowMapVolumeCount )
                continue;
            break;
        }
    }
}

void __cdecl R_UpdateExposureValue(refdef_s *refdef)
{
    float v1; // [esp+0h] [ebp-28h]
    float v2; // [esp+4h] [ebp-24h]
    float v3; // [esp+8h] [ebp-20h]
    float v4; // [esp+Ch] [ebp-1Ch]
    float lerpSpeed; // [esp+10h] [ebp-18h]
    float lerpBaseDecrease; // [esp+14h] [ebp-14h]
    float lerpBaseDecreasea; // [esp+14h] [ebp-14h]
    float feather; // [esp+18h] [ebp-10h] BYREF
    float targetExposure; // [esp+1Ch] [ebp-Ch]
    float lerpBaseIncrease; // [esp+20h] [ebp-8h]
    int exposureId; // [esp+24h] [ebp-4h]

    feather = 50.0f;
    exposureId = R_CheckExposureVolumes(refdef->vieworg, &feather);
    if ( exposureId == -1 )
    {
        targetExposure = rgp.world->sunParse.sunSettings[0].exposure;
        lerpBaseIncrease = 15.0f;
        lerpBaseDecrease = 20.0f;
    }
    else
    {
        if ( rgp.world->exposureVolumes[exposureId].exposure == -1.0 )
            targetExposure = exposureSettings.exposureValues[HIBYTE(rgp.world->exposureVolumes[exposureId].control)].blackPoint[3];
        else
            targetExposure = rgp.world->exposureVolumes[exposureId].exposure;
        feather = feather * rgp.world->exposureVolumes[exposureId].featherRange;
        if ( feather <= 1.0 )
            v4 = feather;
        else
            v4 = 1.0f;
        feather = v4;
        targetExposure = (float)(rgp.world->exposureVolumes[exposureId].featherAdjust * v4) + targetExposure;
        lerpBaseIncrease = 15.0 * rgp.world->exposureVolumes[exposureId].luminanceIncreaseScale;
        lerpBaseDecreasea = 20.0 * rgp.world->exposureVolumes[exposureId].luminanceDecreaseScale;
        if ( lerpBaseIncrease >= 1.0 )
            v3 = lerpBaseIncrease;
        else
            v3 = 1.0f;
        lerpBaseIncrease = v3;
        if ( lerpBaseDecreasea >= 1.0 )
            v2 = lerpBaseDecreasea;
        else
            v2 = 1.0f;
        lerpBaseDecrease = v2;
    }
    if ( refdef->exposure <= targetExposure )
        lerpSpeed = lerpBaseDecrease;
    else
        lerpSpeed = lerpBaseIncrease;
    if ( lerpSpeed >= 1.0 )
        v1 = lerpSpeed;
    else
        v1 = 1.0f;
    refdef->exposure = (float)((float)((float)(v1 - 1.0) * refdef->exposure) + targetExposure) / v1;
    refdef->exposureValue.blackPoint[0] = 0.0f;
    refdef->exposureValue.blackPoint[1] = 0.0f;
    refdef->exposureValue.blackPoint[2] = 0.0f;
    refdef->exposureValue.blackPoint[3] = refdef->exposure;
    refdef->exposureValue.whitePoint[0] = 8.0f;
    refdef->exposureValue.whitePoint[1] = 8.0f;
    refdef->exposureValue.whitePoint[2] = 8.0f;
}

unsigned int __cdecl R_CheckExposureVolumes(float *position, float *feather)
{
    float v3; // [esp+0h] [ebp-1Ch]
    unsigned int j; // [esp+4h] [ebp-18h]
    unsigned int control; // [esp+8h] [ebp-14h]
    unsigned int i; // [esp+10h] [ebp-Ch]
    unsigned int planeIndex; // [esp+14h] [ebp-8h]
    float dot; // [esp+18h] [ebp-4h]

    if ( !rgp.world->exposureVolumeCount )
        return -1;
    i = 0;
    do
    {
        control = rgp.world->exposureVolumes[i].control;
        planeIndex = (unsigned __int16)control;
        for ( j = 0; ; ++j )
        {
            if ( j >= (unsigned int)BYTE2(control) + 1 )
                return i;
            dot = (float)((float)(rgp.world->exposureVolumePlanes[planeIndex].plane[2] * position[2])
                                    + (float)((float)(rgp.world->exposureVolumePlanes[planeIndex].plane[1] * position[1])
                                                    + (float)(rgp.world->exposureVolumePlanes[planeIndex].plane[0] * *position)))
                    + rgp.world->exposureVolumePlanes[planeIndex].plane[3];
            if ( dot < 0.0 )
                break;
            if ( dot <= *feather )
                v3 = (float)((float)(rgp.world->exposureVolumePlanes[planeIndex].plane[2] * position[2])
                                     + (float)((float)(rgp.world->exposureVolumePlanes[planeIndex].plane[1] * position[1])
                                                     + (float)(rgp.world->exposureVolumePlanes[planeIndex].plane[0] * *position)))
                     + rgp.world->exposureVolumePlanes[planeIndex].plane[3];
            else
                v3 = *feather;
            *feather = v3;
            ++planeIndex;
        }
        ++i;
    }
    while ( i < rgp.world->exposureVolumeCount );
    return -1;
}

void __cdecl R_GenerateSortedDrawSurfs(
                const GfxSceneParms *sceneParms,
                const GfxViewParms *viewParmsDpvs,
                const GfxViewParms *viewParmsDraw,
                int frameTime,
                const refdef_s *refdef,
                bool forMissileCamView)
{
    int v6; // eax
    //jpeg_decompress_struct *v7; // [esp+10h] [ebp-280h]
    //jpeg_decompress_struct *v8; // [esp+10h] [ebp-280h]
    bool v9; // [esp+18h] [ebp-278h]
    bool v11; // [esp+23h] [ebp-26Dh]
    unsigned int dynamicSModelCount; // [esp+24h] [ebp-26Ch]
    int VisibleDLights; // [esp+2Ch] [ebp-264h]
    float v14; // [esp+40h] [ebp-250h]
    float v15; // [esp+7Ch] [ebp-214h]
    bool v16; // [esp+C3h] [ebp-1CDh]
    GfxImage *blackImage; // [esp+1BCh] [ebp-D4h]
    GfxImage *v18; // [esp+1C4h] [ebp-CCh]
    GfxImage *image; // [esp+1CCh] [ebp-C4h]
    float v20; // [esp+1D8h] [ebp-B8h]
    float v21; // [esp+1DCh] [ebp-B4h]
    char *Name; // [esp+1E0h] [ebp-B0h]
    int j; // [esp+1E8h] [ebp-A8h]
    int partitionIndex; // [esp+1ECh] [ebp-A4h]
    unsigned int i; // [esp+1F4h] [ebp-9Ch]
    float bestError; // [esp+1F8h] [ebp-98h]
    unsigned int bestNum; // [esp+1FCh] [ebp-94h]
    unsigned int bestDen; // [esp+208h] [ebp-88h]
    unsigned int den; // [esp+20Ch] [ebp-84h]
    GfxDrawSurfListInfo *litPunchThruInfo; // [esp+214h] [ebp-7Ch]
    int pointLightCount; // [esp+218h] [ebp-78h]
    int firstDrawSurfCount; // [esp+21Ch] [ebp-74h]
    GfxDrawSurfListInfo *litPostResolveInfo; // [esp+220h] [ebp-70h]
    GfxDrawSurfListInfo *litInfo; // [esp+224h] [ebp-6Ch]
    bool usePreTess; // [esp+22Bh] [ebp-65h]
    DynSModelClient smodelClient; // [esp+22Ch] [ebp-64h] BYREF
    GfxViewInfo *viewInfo; // [esp+240h] [ebp-50h]
    int viewInfoIndex; // [esp+244h] [ebp-4Ch]
    bool forStereoRightEyeView; // [esp+24Bh] [ebp-45h]
    ShadowType dynamicShadowType; // [esp+250h] [ebp-40h]
    SceneEntCmd sceneEntCmd; // [esp+254h] [ebp-3Ch] BYREF
    FxCmd cmd; // [esp+258h] [ebp-38h] BYREF
    bool extraCamActive; // [esp+28Fh] [ebp-1h]

    Name = va("R_GenerateSortedDrawSurfs (cl=%d v=%d)", sceneParms->localClientNum, rg.viewInfoCount);
    PROF_SCOPED_RUNTIME_NAME(Name);

    forStereoRightEyeView = 0;
    if ( frontEndDataOut->viewInfoCount != rg.viewInfoCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    5435,
                    0,
                    "%s",
                    "frontEndDataOut->viewInfoCount == rg.viewInfoCount") )
    {
        __debugbreak();
    }
    viewInfoIndex = rg.viewInfoCount++;
    frontEndDataOut->viewInfoCount = rg.viewInfoCount;
    if ( (unsigned int)viewInfoIndex >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    5441,
                    1,
                    "viewInfoIndex doesn't index GFX_MAX_CLIENT_VIEWS\n\t%i not in [0, %i)",
                    viewInfoIndex,
                    4) )
    {
        __debugbreak();
    }
    frontEndDataOut->viewInfoIndex = viewInfoIndex;
    viewInfo = &frontEndDataOut->viewInfo[viewInfoIndex];
    viewInfo->hasCmdBuf = viewInfoIndex == 0;

    if ( !forMissileCamView || R_ExtraCam_UseShadowmap() )
        dynamicShadowType = (ShadowType)R_DynamicShadowType();
    else
        dynamicShadowType = SHADOW_NONE;

    viewInfo->dynSModelView = &frontEndDataOut->dynSModelClientViewArray[viewInfoIndex];
    viewInfo->dynSModelState = &frontEndDataOut->dynSModelState;
    //BLOPS_NULLSUB((jpeg_decompress_struct *)viewInfo->dynSModelView);
    rg.isNotRenderingFullScreen = !sceneParms->isRenderingFullScreen;
    rg.sunShadowFull = 1;
    LODWORD(rg.sunShadowmapScale) = sm_sunShadowScale->current.integer;
    bestDen = 1;
    bestNum = 1;
    bestError = 1.0f;
    for ( den = 1; den <= 10; ++den )
    {
        v21 = (double)den * rg.sunShadowmapScale + 0.5;
        v15 = floor(v21);
        v20 = (double)(unsigned int)(__int64)v15 / (double)den - rg.sunShadowmapScale;
        if ( fabs(v20) < bestError )
        {
            bestError = fabs(v20);
            bestDen = den;
            bestNum = (__int64)v15;
        }
    }
    rg.sunShadowmapScale = (double)bestNum / (double)bestDen;
    rg.sunShadowmapScaleNum = (float)bestNum;
    v14 = ceil((float)((float)dx.sunShadowmapSize * rg.sunShadowmapScale));
    rg.sunShadowSize = (__int64)v14;
    rg.sunShadowPartitionRatio = 4.0 / rg.sunShadowmapScale;
    rg.drawSunShadow = 0;

    if ( !rg.lodParms.valid
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 5543, 0, "%s", "rg.lodParms.valid") )
    {
        __debugbreak();
    }
    if ( !rg.correctedLodParms[sceneParms->localClientNum].valid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    5544,
                    0,
                    "%s",
                    "rg.correctedLodParms[sceneParms->localClientNum].valid") )
    {
        __debugbreak();
    }

    memcpy(&viewInfo->input, &gfxCmdBufInput, sizeof(viewInfo->input));
    viewInfo->input.data = frontEndDataOut;
    viewInfo->sceneDef = scene.def;
    memcpy(viewInfo, viewParmsDraw, 0x140u);
    viewInfo->cullViewInfo.sceneViewport = sceneParms->sceneViewport;
    viewInfo->cullViewInfo.displayViewport = sceneParms->displayViewport;
    viewInfo->cullViewInfo.scissorViewport = sceneParms->scissorViewport;
    viewInfo->dynamicShadowType = dynamicShadowType;
    viewInfo->localClientNum = sceneParms->localClientNum;
    viewInfo->isRenderingFullScreen = sceneParms->isRenderingFullScreen;
    viewInfo->playerTeleported = sceneParms->playerTeleported;
    viewInfo->blurRadius = sceneParms->blurRadius;
    viewInfo->noLodCullOut = refdef->noLodCullOut;
    viewInfo->needsFloatZ = 0;
    frontEndDataOut->sunShadow.sunShadowCount = 0;
    frontEndDataOut->spotShadowCount = 0;
    Com_Memcpy(frontEndDataOut->shadowableLights, sceneParms->primaryLights, 368 * rgp.world->primaryLightCount);
    frontEndDataOut->shadowableLightCount = rgp.world->primaryLightCount;
    if ( forMissileCamView )
    {
        viewInfo->isMissileCamera = 1;
        viewInfo->sceneComposition.mainSceneMSAA = R_RENDERTARGET_MISSILE_CAM;

        R_SetInputCodeImage(&viewInfo->input, TEXTURE_SRC_CODE_FLOATZ, gfxRenderTargets[R_RENDERTARGET_FLOAT_Z_MISSILE_CAM].image);
        R_SetInputCodeImage(&viewInfo->input, TEXTURE_SRC_CODE_RESOLVED_POST_SUN, rt.image);
        R_SetInputCodeImage(&viewInfo->input, TEXTURE_SRC_CODE_POST_EFFECT_SRC, rgp.blackImage);

        R_SetInputCodeConstant(&viewInfo->input, 0xC3u, 1.0, 1.0, 1.0, 1.0);
    }
    else
    {
        viewInfo->isMissileCamera = 0;
        viewInfo->sceneComposition.mainSceneMSAA = R_RENDERTARGET_SCENE;
        R_SetInputCodeConstant(&viewInfo->input, 0xC3u, 0.0, 0.0, 0.0, 0.0);
    }
    if ( r_debugShowPrimaryLights->current.enabled )
    {
        for ( i = 0; i < frontEndDataOut->shadowableLightCount; ++i )
            DrawLightDebug(viewInfo, &frontEndDataOut->shadowableLights[i], colorWhite);
    }
    viewInfo->sunVisibility = sceneParms->sunVisibility;
    scene.shadowableLightIsUsed[0] = 0;
    scene.shadowableLightIsUsed[1] = 0;
    scene.shadowableLightIsUsed[2] = 0;
    scene.shadowableLightIsUsed[3] = 0;
    scene.shadowableLightIsUsed[4] = 0;
    scene.shadowableLightIsUsed[5] = 0;
    scene.shadowableLightIsUsed[6] = 0;
    scene.shadowableLightIsUsed[7] = 0;
    R_SetHeroLighting(&viewInfo->input, viewInfo);
    R_SetSkyDynamicIntensity(viewInfo->cullViewInfo.viewParms.axis[0], &viewInfo->input);
    R_SetupDynamicModelLighting(&viewInfo->input);
    R_SetFrameFog(&viewInfo->input, viewInfo);
    R_SetSunConstants(&viewInfo->input);
    DrawSunDirectionDebug(viewParmsDraw->origin, viewParmsDraw->axis[0]);
    DrawOutdoorBoundsVolumeDebug();
    R_SetDepthOfField(viewInfo, sceneParms);
    R_SetFilmInfo(viewInfo, sceneParms);
    R_SetBloomInfo(viewInfo, sceneParms);
    R_SetCharacterPrimaryLightScaleInfo(viewInfo, sceneParms);
    R_DrawCineWarning();
    R_DrawFogParams();
    R_SetInputCodeConstant(&viewInfo->input, 0x31u, 0.984375 * viewParmsDraw->zNear, 0.0, 0.0, 0.0);
    R_SetFullSceneViewMesh(viewInfoIndex, viewInfo);
    R_SetupWorldSurfacesDpvs(viewParmsDpvs, forMissileCamView ? 0x41 : 0);
    R_SetViewFrustumPlanes(viewInfo);
    R_SetDoubleVision(viewInfo, sceneParms);
    R_SetWaterSheetingFx(viewInfo, sceneParms);
    R_SetFlameFx(viewInfo, sceneParms);
    R_SetReviveFx(viewInfo, sceneParms);
    R_SetPoisonFx(viewInfo, sceneParms);
    R_SetGenericFilter(viewInfo, sceneParms);
    R_SetElectrifiedFx(viewInfo, sceneParms);
    R_SetTransportedFx(viewInfo, sceneParms);
    GlassCl_BeginGenerateVerts(sceneParms->localClientNum, viewInfoIndex);
    GlassCl_Update(sceneParms->localClientNum);
    memcpy(&viewInfo->saveScreenFx, &sceneParms->saveScreenFx, sizeof(viewInfo->saveScreenFx));
    R_SetSceneComposition(viewInfo, sceneParms, forMissileCamView);
    R_SetExposure(viewInfo, refdef);
    R_SetHDRControlConstants(&viewInfo->input, viewInfo);

    {
        PROF_SCOPED("R_InitialEntityCulling");
        R_InitialEntityCulling();
    }
    {
        PROF_SCOPED("R_AddWorldSurfacesDpvs");
        v6 = R_CellForPoint(viewParmsDpvs->origin);
        R_AddWorldSurfacesDpvs(viewParmsDpvs, v6);
    }
    {
        PROF_SCOPED("WaitFXSpot");
        Sys_WaitWorkerCmdInternal(&fx_update_spotWorkerCmd);
    }
    
    R_AddEmissiveSpotLight(viewInfo);
    R_WaitForFXUpdateWorkerCmds();
    if ( forMissileCamView )
        VisibleDLights = 0;
    else
        VisibleDLights = R_GetVisibleDLights(viewInfo->visibleLights);
    viewInfo->visibleLightCount = VisibleDLights;
    if ( frontEndDataOut->emissiveSpotLightCount
        && !viewInfo->visibleLightCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    5714,
                    0,
                    "%s",
                    "frontEndDataOut->emissiveSpotLightCount == 0 || viewInfo->visibleLightCount != 0") )
    {
        __debugbreak();
    }
    R_BeginAllStaticModelLighting();
    extraCamActive = refdef->extraCamPosValid;
    if ( Sys_QueryD3DDeviceOKEvent() )
        usePreTess = r_pretess->current.enabled;
    else
        usePreTess = 0;
    if ( usePreTess )
    {
        if ( extraCamActive && viewInfo->isMissileCamera )
            R_BeginPreTess(0);
        else
            R_BeginPreTess(1);
    }

    {
        PROF_SCOPED("wait for r_dpvs_static");
        Sys_WaitWorkerCmdInternal(&r_dpvs_staticWorkerCmd);
    }
    
    R_AddSkyboxModel(viewParmsDraw->origin);

    {
        PROF_SCOPED("bsp surfaces"); 
        if (gfxDrawMethod.drawScene == GFX_DRAW_SCENE_STANDARD)
        {
            R_AddAllBspDrawSurfacesCamera(viewInfo->localClientNum, viewInfo->visibleLights, viewInfo->visibleLightCount);
            if (rgp.world->sunPrimaryLightIndex)
            {
                if (sm_sunEnable->current.enabled)
                {
                    if (sm_sunAlwaysCastsShadow->current.enabled)
                        Com_BitSetAssert(scene.shadowableLightIsUsed, rgp.world->sunPrimaryLightIndex, 128);
                }
                else
                {
                    Com_BitClearAssert(scene.shadowableLightIsUsed, rgp.world->sunPrimaryLightIndex, 128);
                }
            }
            Com_Memset(frontEndDataOut->shadowableLightHasShadowMap, 0, 32);
            if (R_GetAllowShadowMaps())
                R_ChooseShadowedLights(viewInfo);
            R_UpdateDrawMethod(frontEndDataOut);
            if (dynamicShadowType == SHADOW_MAP
                && Com_BitCheckAssert(frontEndDataOut->shadowableLightHasShadowMap, rgp.world->sunPrimaryLightIndex, 32))
            {
                rg.drawSunShadow = 1;
                R_SetupSunShadowMaps(viewParmsDpvs, &frontEndDataOut->sunShadow);
                R_SetSunShadowConstants(&viewInfo->input, &frontEndDataOut->sunShadow.sunProj);
                R_SunShadowMaps();
            }
        }
        else
        {
            R_AddAllBspDrawSurfacesCameraNonlit(rgp.world->dpvs.litSurfsBegin, rgp.world->dpvs.litSurfsEnd, 0);
            R_AddAllBspDrawSurfacesCameraNonlit(rgp.world->dpvs.decalSurfsBegin, rgp.world->dpvs.decalSurfsEnd, 3u);
        }
        R_AddAllBspDrawSurfacesCameraNonlit(rgp.world->dpvs.emissiveSurfsBegin, rgp.world->dpvs.emissiveSurfsEnd, 0xAu);
    }
    
    R_AddAllStaticModelSurfacesCamera(
        viewInfo->localClientNum,
        viewInfo->playerTeleported,
        viewInfo->visibleLights,
        viewInfo->visibleLightCount,
        0);

    {
        PROF_SCOPED("DynEntPieces_AddDrawSurfs");
        DynEntPieces_AddDrawSurfs();
    }
    {
        PROF_SCOPED("wait for r_dpvs_dynmodel");
        Sys_WaitWorkerCmdInternal(&r_dpvs_dynmodelWorkerCmd);
    }
    {
        PROF_SCOPED("wait for r_dpvs_dynbrush");
        Sys_WaitWorkerCmdInternal(&r_dpvs_dynbrushWorkerCmd);
    }
    {
        PROF_SCOPED("draw all dyn ent");
        R_DrawAllDynEnt(viewInfo);
    }

    if ( gfxDrawMethod.drawScene == GFX_DRAW_SCENE_STANDARD
        && dynamicShadowType == SHADOW_MAP
        && Com_BitCheckAssert(frontEndDataOut->shadowableLightHasShadowMap, rgp.world->sunPrimaryLightIndex, 32) )
    {
        {
            PROF_SCOPED("wait for more r_dpvs_static");
            Sys_WaitWorkerCmdInternal(&r_dpvs_staticWorkerCmd);
        }
        {
            PROF_SCOPED("bsp surfaces");
            R_AddAllBspDrawSurfacesSunShadow(viewInfo->localClientNum);
        }
        {
            PROF_SCOPED("static model surfaces");
            R_AddAllStaticModelSurfacesSunShadow(viewInfoIndex);
        }
    }
    FX_BeginMarks(viewInfo->localClientNum);
    memset((unsigned __int8 *)&cmd, 0, sizeof(cmd));
    cmd.localClientNum = viewInfo->localClientNum;
    cmd.visibleLights = viewInfo->visibleLights;
    cmd.visibleLightCount = viewInfo->visibleLightCount;
    Sys_AddWorkerCmdInternal(&fx_marks_drawWorkerCmd, (unsigned char*)& cmd, 0);

    {
        PROF_SCOPED("wait for r_dpvs_sceneent r_dpvs_entity");
        Sys_WaitWorkerCmdInternal(&r_dpvs_sceneentWorkerCmd);
        Sys_WaitWorkerCmdInternal(&r_dpvs_entityWorkerCmd);
    }
    {
        PROF_SCOPED("wait for r_spot_shadow_ent");
        Sys_WaitWorkerCmdInternal(&r_spot_shadow_entWorkerCmd);
    }

    {
        PROF_SCOPED("draw all scene ent");
        R_DrawAllSceneEnt(viewInfo);
    }
    {
        PROF_SCOPED("wait for dobj_skin");
        Sys_WaitWorkerCmdInternal(&dobj_skinWorkerCmd);
    }
    {
        PROF_SCOPED("add workercmd r_add_sceneent");
        sceneEntCmd.viewInfo = viewInfo;
        Sys_AddWorkerCmdInternal(&r_add_sceneentWorkerCmd, (unsigned char *)&sceneEntCmd, 0);
    }
    
    R_SortAllStaticModelSurfacesCamera();
    R_GetLightSurfs(viewInfo->visibleLights, viewInfo->visibleLightCount);
    R_GetPointLightShadowSurfs(viewInfo, scene.visLightShadow, viewInfo->visibleLights, viewInfo->visibleLightCount);
    if ( viewInfo->visibleLightCount )
        R_SetDLightsConstants(&viewInfo->input, viewInfo, viewInfo->visibleLights, viewInfo->visibleLightCount);
    else
        R_SetDLightsConstantsDefaults(&viewInfo->input);
    if ( gfxDrawMethod.drawScene == GFX_DRAW_SCENE_STANDARD )
    {
        PROF_SCOPED("Sun Shadow");
        if ( dynamicShadowType == SHADOW_MAP )
        {
            if ( Com_BitCheckAssert(frontEndDataOut->shadowableLightHasShadowMap, rgp.world->sunPrimaryLightIndex, 32) )
            {
                {
                    PROF_SCOPED("scene ent surfaces");
                    R_AddAllSceneEntSurfacesSunShadow();
                }
                
                R_SortAllStaticModelSurfacesSunShadow();
                R_SortAllCodeMeshSurfacesSunShadow();
                R_GenerateSortedSunShadowDrawSurfs(viewInfo);
            }
            else
            {
                for ( partitionIndex = 0; partitionIndex < 2; ++partitionIndex )
                    R_InitDrawSurfListInfo(&viewInfo->drawList[partitionIndex + 8]);
            }
            R_GenerateAllSortedSpotShadowDrawSurfs(viewInfo, viewInfoIndex);
        }
    }
    R_SetAllStaticModelLighting();
    smodelClient.localClientNum = viewInfo->localClientNum;
    smodelClient.visBitsCamera = scene.dynSModelVisBitsCamera;
    smodelClient.visBitsSunShadow = scene.dynSModelVisBitsSunShadow;
    if ( (int)rgp.world->dpvs.dynamicSModelCount > 2048 )
        dynamicSModelCount = 2048;
    else
        dynamicSModelCount = rgp.world->dpvs.dynamicSModelCount;
    smodelClient.numDynSModel = dynamicSModelCount;
    smodelClient.numDynSModelInMap = rgp.world->dpvs.dynamicSModelCount;
    R_DynSModelBuildClientView(&smodelClient, viewInfo->dynSModelView, &frontEndDataOut->dynSModelState);

    {
        PROF_SCOPED("wait for r_add_sceneent");
        Sys_AssistAndWaitWorkerCmdInternal(&r_model_lightingWorkerCmd);
        Sys_AssistAndWaitWorkerCmdInternal(&r_add_sceneentWorkerCmd);
    }
    
    if ( usePreTess )
    {
        v11 = !extraCamActive || viewInfo->isMissileCamera;
        //BLOPS_NULLSUB((jpeg_decompress_struct *)v11);
    }
    litInfo = viewInfo->drawList;
    R_InitDrawSurfListInfo(viewInfo->drawList);
    litInfo->baseTechType = gfxDrawMethod.baseTechType;
    litInfo->viewInfo = viewInfo;
    litInfo->viewOrigin[0] = viewParmsDraw->origin[0];
    litInfo->viewOrigin[1] = viewParmsDraw->origin[1];
    litInfo->viewOrigin[2] = viewParmsDraw->origin[2];
    litInfo->viewOrigin[3] = viewParmsDraw->origin[3];
    litInfo->cameraView = 1;
    litInfo->shaderHDRmode = viewInfo->sceneComposition.opaqueShaderRemap;
    litPunchThruInfo = &viewInfo->drawList[2];
    R_InitDrawSurfListInfo(&viewInfo->drawList[2]);
    litPunchThruInfo->baseTechType = gfxDrawMethod.baseTechType;
    litPunchThruInfo->viewInfo = viewInfo;
    litPunchThruInfo->viewOrigin[0] = viewParmsDraw->origin[0];
    litPunchThruInfo->viewOrigin[1] = viewParmsDraw->origin[1];
    litPunchThruInfo->viewOrigin[2] = viewParmsDraw->origin[2];
    litPunchThruInfo->viewOrigin[3] = viewParmsDraw->origin[3];
    litPunchThruInfo->cameraView = 1;
    litPunchThruInfo->shaderHDRmode = viewInfo->sceneComposition.opaqueShaderRemap;
    litPostResolveInfo = &viewInfo->drawList[1];
    R_InitDrawSurfListInfo(&viewInfo->drawList[1]);
    litPostResolveInfo->baseTechType = gfxDrawMethod.baseTechType;
    litPostResolveInfo->viewInfo = viewInfo;
    litPostResolveInfo->viewOrigin[0] = viewParmsDraw->origin[0];
    litPostResolveInfo->viewOrigin[1] = viewParmsDraw->origin[1];
    litPostResolveInfo->viewOrigin[2] = viewParmsDraw->origin[2];
    litPostResolveInfo->viewOrigin[3] = viewParmsDraw->origin[3];
    litPostResolveInfo->cameraView = 1;
    litPostResolveInfo->shaderHDRmode = viewInfo->sceneComposition.opaqueShaderRemap;

    {
        PROF_SCOPED("water simulation render");
        if (r_gfxopt_water_simulation->current.enabled && sceneParms->isRenderingFullScreen)
            R_WaterSimulationRender(viewParmsDraw->origin, frameTime, litInfo->viewInfoIndex);
        else
            R_WaterSimulationSwitchOff();
    }
    {
        PROF_SCOPED("set up litInfo");
        firstDrawSurfCount = frontEndDataOut->drawSurfCount;
        R_MergeAndEmitDrawSurfLists(0, 3u, frontEndDataOut);
        litInfo->drawSurfs = &frontEndDataOut->drawSurfs[firstDrawSurfCount];
        litInfo->drawSurfCount = frontEndDataOut->drawSurfCount - firstDrawSurfCount;
    }
    {
        PROF_SCOPED("split litInfo");
        R_SplitDrawSurfacesPrimarySortKey(viewInfo->drawList, &viewInfo->drawList[1], 16);
    }
    
    R_DynSModelWaitWorker();
    if ( r_debugShowDynamicLights->current.enabled )
    {
        for ( j = 0; j < viewInfo->visibleLightCount; ++j )
            DrawLightDebug(viewInfo, &viewInfo->visibleLights[j], colorGreen);
    }
    pointLightCount = 0;
    if ( r_dlightLimit->current.integer && gfxDrawMethod.drawScene == GFX_DRAW_SCENE_STANDARD && !forMissileCamView )
    {
        PROF_SCOPED("emit dlight surfs");
        pointLightCount = R_EmitPointLightPartitionSurfs(
                                                viewInfo,
                                                viewInfo->visibleLights,
                                                viewInfo->visibleLightCount,
                                                viewParmsDpvs->origin);
    }
    viewInfo->pointLightCount = pointLightCount;
    if ( !forMissileCamView && !forStereoRightEyeView )
    {
        if ( !refdef->localClientNum )
            R_StreamUpdate(viewParmsDraw->origin);
        R_StreamUpdatePerClient(viewParmsDraw->origin);
    }

    //if ( !viewInfo->localClientNum && !forMissileCamView && !forStereoRightEyeView )
    //    BLOPS_NULLSUB(v7);

    if ( !forMissileCamView )
    {
        PROF_SCOPED("DynEntCl_ProcessEntities");
        DynEntCl_ProcessEntities(viewInfo->localClientNum, refdef->time);
    }

    {
        PROF_SCOPED("wait for fx_marks_draw");
        Sys_WaitWorkerCmdInternal(&fx_marks_drawWorkerCmd);
    }
    
    if ( Sys_QueryD3DDeviceOKEvent() )
    {
        v9 = fx_marks && fx_marks->current.enabled;
        v16 = v9;
    }
    else
    {
        v16 = 0;
    }
    if ( v16 )
    {
        PROF_SCOPED("Mark verts");
        if ( fx_marks_smodels->current.enabled )
            FX_GenerateMarkVertsForStaticModels(
                viewInfo->localClientNum,
                rgp.world->dpvs.smodelCount,
                rgp.world->dpvs.smodelVisData[0],
                viewInfo->visibleLights,
                viewInfo->visibleLightCount);
        if ( fx_marks_ents->current.enabled )
            R_GenerateMarkVertsForDynamicModels(viewInfo->visibleLights, viewInfo->visibleLightCount);
    }
    R_FinishDecalAndEmissiveDrawSurfs(viewInfo, viewParmsDraw, frontEndDataOut, forMissileCamView);
    R_ShowCull(viewParmsDraw->origin);
    viewInfo->renderSeeThruDecals = CG_IsShowingZombieMap() && FX_GetFrameTotalSeeThruDecalCount(viewInfo->localClientNum);
    //BLOPS_NULLSUB(v8);
}

bool __cdecl R_GetAllowShadowMaps()
{
    return sm_enable->current.enabled && g_allowShadowMaps;
}

bool __cdecl R_DynamicShadowType()
{
    return R_GetAllowShadowMaps();
}

void __cdecl R_SetSceneComposition(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms, bool extraCam)
{
    viewInfo->sceneComposition.ui3d = R_RENDERTARGET_UI3D;
    viewInfo->sceneComposition.seeThruDecal = R_RENDERTARGET_SEETHRU_DECAL;
    viewInfo->sceneComposition.frameBuffer = R_RENDERTARGET_FRAME_BUFFER;
    viewInfo->sceneComposition.renderingMode = 0;
    viewInfo->sceneComposition.extraCam = R_RENDERTARGET_MISSILE_CAM;
    if ( extraCam )
    {
        viewInfo->sceneComposition.mainSceneMSAA = R_RENDERTARGET_MISSILE_CAM;
        viewInfo->sceneComposition.mainScene = R_RENDERTARGET_MISSILE_CAM;
    }
    else
    {
        viewInfo->sceneComposition.mainSceneMSAA = R_RENDERTARGET_SCENE;
        viewInfo->sceneComposition.mainScene = R_RENDERTARGET_SCENE;
    }
}

void __cdecl R_SetExposure(GfxViewInfo *viewInfo, const refdef_s *refdef)
{
    int integer; // [esp+Ch] [ebp-5Ch]

    viewInfo->exposureValue = refdef->exposureValue;
    if ( r_exposureTweak->current.enabled )
    {
        integer = r_exposureValue->current.integer;
        viewInfo->exposureValue.blackPoint[0] = 0.0f;
        viewInfo->exposureValue.blackPoint[1] = 0.0f;
        viewInfo->exposureValue.blackPoint[2] = 0.0f;
        LODWORD(viewInfo->exposureValue.blackPoint[3]) = integer;
        viewInfo->exposureValue.whitePoint[0] = 8.0f;
        viewInfo->exposureValue.whitePoint[1] = 8.0f;
        viewInfo->exposureValue.whitePoint[2] = 8.0f;
        viewInfo->exposureValue.whitePoint[3] = 1.0f;
    }
    else
    {
        Dvar_SetFloatIfChanged((dvar_s *)r_exposureValue, viewInfo->exposureValue.blackPoint[3]);
    }
    viewInfo->postEmissiveBrightening = refdef->postEmissiveBrightening;
    viewInfo->exposureRemap.remapMul[0] = (float)(1.0
                                                                                            / (float)(viewInfo->exposureValue.whitePoint[0]
                                                                                                            - viewInfo->exposureValue.blackPoint[0]))
                                                                            * viewInfo->exposureValue.blackPoint[3];
    viewInfo->exposureRemap.remapMul[1] = (float)(1.0
                                                                                            / (float)(viewInfo->exposureValue.whitePoint[1]
                                                                                                            - viewInfo->exposureValue.blackPoint[1]))
                                                                            * viewInfo->exposureValue.blackPoint[3];
    viewInfo->exposureRemap.remapMul[2] = (float)(1.0
                                                                                            / (float)(viewInfo->exposureValue.whitePoint[2]
                                                                                                            - viewInfo->exposureValue.blackPoint[2]))
                                                                            * viewInfo->exposureValue.blackPoint[3];
    viewInfo->exposureRemap.remapAdd[0] = (-(viewInfo->exposureValue.blackPoint[0]))
                                                                            * (float)(1.0 / (float)(viewInfo->exposureValue.whitePoint[0]
                                                                                                            - viewInfo->exposureValue.blackPoint[0]));
    viewInfo->exposureRemap.remapAdd[1] = (-(viewInfo->exposureValue.blackPoint[1]))
                                                                            * (float)(1.0 / (float)(viewInfo->exposureValue.whitePoint[1]
                                                                                                            - viewInfo->exposureValue.blackPoint[1]));
    viewInfo->exposureRemap.remapAdd[2] = (-(viewInfo->exposureValue.blackPoint[2]))
                                                                            * (float)(1.0 / (float)(viewInfo->exposureValue.whitePoint[2]
                                                                                                            - viewInfo->exposureValue.blackPoint[2]));
    viewInfo->exposureRemap.remapMul[0] = viewInfo->exposureValue.blackPoint[3] / 8.0;
    viewInfo->exposureRemap.remapMul[1] = viewInfo->exposureValue.blackPoint[3] / 8.0;
    viewInfo->exposureRemap.remapMul[2] = viewInfo->exposureValue.blackPoint[3] / 8.0;
    viewInfo->exposureRemap.remapAdd[0] = 0.0f;
    viewInfo->exposureRemap.remapAdd[1] = 0.0f;
    viewInfo->exposureRemap.remapAdd[2] = 0.0f;
}

void __cdecl R_SetDepthOfField(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms)
{
    Font_s *v2; // [esp+24h] [ebp-21Ch]
    Font_s *font; // [esp+28h] [ebp-218h]
    char buff[516]; // [esp+30h] [ebp-210h] BYREF
    float x; // [esp+238h] [ebp-8h]
    float y; // [esp+23Ch] [ebp-4h]

    if ( r_dof_tweak->current.enabled )
    {
        if ( r_dof_nearBlur->current.value < 4.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                        3186,
                        0,
                        "%s\n\t(r_dof_nearBlur->current.value) = %g",
                        "(r_dof_nearBlur->current.value >= 4.0f)",
                        r_dof_nearBlur->current.value) )
        {
            __debugbreak();
        }
        viewInfo->dof.viewModelStart = r_dof_viewModelStart->current.value;
        viewInfo->dof.viewModelEnd = r_dof_viewModelEnd->current.value;
        viewInfo->dof.nearStart = r_dof_nearStart->current.value;
        viewInfo->dof.nearEnd = r_dof_nearEnd->current.value;
        viewInfo->dof.farStart = r_dof_farStart->current.value;
        viewInfo->dof.farEnd = r_dof_farEnd->current.value;
        viewInfo->dof.nearBlur = r_dof_nearBlur->current.value;
        viewInfo->dof.farBlur = r_dof_farBlur->current.value;
    }
    else if ( r_dof_enable->current.enabled )
    {
        memcpy(&viewInfo->dof, &sceneParms->dof, sizeof(viewInfo->dof));
    }
    else
    {
        viewInfo->dof.viewModelStart = 0.0;
        viewInfo->dof.viewModelEnd = 0.0;
        viewInfo->dof.nearStart = 0.0;
        viewInfo->dof.nearEnd = 0.0;
        viewInfo->dof.farStart = 0.0;
        viewInfo->dof.farEnd = 0.0;
        viewInfo->dof.nearBlur = 0.0;
        viewInfo->dof.farBlur = 0.0;
    }
    if ( r_dof_showdebug->current.enabled )
    {
        font = R_RegisterFont("fonts/consoleFont", 1);
        x = 60.0f;
        y = 60.0f;
        _snprintf(buff, 0x200u, "DOF.viewModelStart = %.2f", viewInfo->dof.viewModelStart);
        R_AddCmdDrawText(buff, 0x7FFFFFFF, font, x, y, 1.5, 2.0, 0.0, colorRed, 0);
        y = y + 25.0;
        _snprintf(buff, 0x200u, "DOF.viewModelEnd     = %.2f", viewInfo->dof.viewModelEnd);
        R_AddCmdDrawText(buff, 0x7FFFFFFF, font, x, y, 1.5, 2.0, 0.0, colorRed, 0);
        y = y + 25.0;
        _snprintf(buff, 0x200u, "DOF.nearStart            = %.2f", viewInfo->dof.nearStart);
        R_AddCmdDrawText(buff, 0x7FFFFFFF, font, x, y, 1.5, 2.0, 0.0, colorRed, 0);
        y = y + 25.0;
        _snprintf(buff, 0x200u, "DOF.nearEnd                = %.2f", viewInfo->dof.nearEnd);
        R_AddCmdDrawText(buff, 0x7FFFFFFF, font, x, y, 1.5, 2.0, 0.0, colorRed, 0);
        y = y + 25.0;
        _snprintf(buff, 0x200u, "DOF.farStart             = %.2f", viewInfo->dof.farStart);
        R_AddCmdDrawText(buff, 0x7FFFFFFF, font, x, y, 1.5, 2.0, 0.0, colorRed, 0);
        y = y + 25.0;
        _snprintf(buff, 0x200u, "DOF.farEnd                 = %.2f", viewInfo->dof.farEnd);
        R_AddCmdDrawText(buff, 0x7FFFFFFF, font, x, y, 1.5, 2.0, 0.0, colorRed, 0);
        y = y + 25.0;
        _snprintf(buff, 0x200u, "DOF.nearBlur             = %.2f", viewInfo->dof.nearBlur);
        R_AddCmdDrawText(buff, 0x7FFFFFFF, font, x, y, 1.5, 2.0, 0.0, colorRed, 0);
        y = y + 25.0;
        _snprintf(buff, 0x200u, "DOF.farBlur                = %.2f", viewInfo->dof.farBlur);
        R_AddCmdDrawText(buff, 0x7FFFFFFF, font, x, y, 1.5, 2.0, 0.0, colorRed, 0);
        y = y + 25.0;
        y = y + 25.0;
        _snprintf(buff, 0x200u, "r_dof_tweak                = %d", r_dof_tweak->current.color[0] != 0);
        R_AddCmdDrawText(buff, 0x7FFFFFFF, font, x, y, 1.5, 2.0, 0.0, colorRed, 0);
        y = y + 25.0;
        _snprintf(buff, 0x200u, "r_dof_enable             = %d", r_dof_enable->current.color[0] != 0);
        R_AddCmdDrawText(buff, 0x7FFFFFFF, font, x, y, 1.5, 2.0, 0.0, colorRed, 0);
        y = y + 25.0;
    }
    if ( RB_UsingDepthOfFieldFX(viewInfo) )
    {
        if ( viewInfo->isMissileCamera
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                        3263,
                        0,
                        "%s",
                        "!viewInfo->isMissileCamera") )
        {
            __debugbreak();
        }
        if ( !dx.supportsIntZ )
        {
            if (!gfxRenderTargets[R_RENDERTARGET_FLOAT_Z].surface.color)
                Com_Error(
                    ERR_FATAL,
                    "Depth of field used (enabled via r_dof_enable or r_dof_tweak) with no float-z buffer (r_floatz wasn't enabled "
                    "when the renderer was started.)\n");
            viewInfo->needsFloatZ = 1;
        }
        if ( com_statmon->current.enabled )
        {
            v2 = R_RegisterFont("fonts/consoleFont", 1);
            if ( cg_draw2D->current.enabled )
                R_AddCmdDrawText((char*)"DOF", 0x7FFFFFFF, v2, 0.0, 320.0, 1.5, 2.0, 0.0, colorRedFaded, 0);
        }
    }
}

void __cdecl R_SetDoubleVision(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms)
{
    Font_s *font; // [esp+24h] [ebp-4h]

    memcpy(&viewInfo->doubleVision, &sceneParms->doubleVision, sizeof(viewInfo->doubleVision));
    if ( R_UsingDoubleVision(viewInfo) && com_statmon->current.enabled )
    {
        font = R_RegisterFont("fonts/consoleFont", 1);
        if ( cg_draw2D->current.enabled )
            R_AddCmdDrawText((char*)"DOUBLEVISION", 0x7FFFFFFF, font, 0.0, 320.0, 1.5, 2.0, 0.0, colorRedFaded, 0);
    }
}

void __cdecl R_SetWaterSheetingFx(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms)
{
    memcpy(&viewInfo->waterSheetingFx, &sceneParms->waterSheetingFx, sizeof(viewInfo->waterSheetingFx));
    if ( r_waterSheetingFX_enable->current.enabled )
    {
        viewInfo->waterSheetingFx.blurRadius = r_waterSheetingFX_radius->current.value;
        viewInfo->waterSheetingFx.distortionMagnitude = r_waterSheetingFX_magnitude->current.value;
        viewInfo->waterSheetingFx.distortionScale[0] = r_waterSheetingFX_distortionScaleFactor->current.value;
        viewInfo->waterSheetingFx.distortionScale[1] = r_waterSheetingFX_distortionScaleFactor->current.vector[1];
        viewInfo->waterSheetingFx.duration = 0;
        viewInfo->waterSheetingFx.frameRate = 0.0f;
    }
}

void __cdecl R_SetFlameFx(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms)
{
    memcpy(&viewInfo->flameFx, &sceneParms->flameFx, sizeof(viewInfo->flameFx));
    if ( r_flameFX_enable->current.enabled )
    {
        viewInfo->flameFx.distortionMagnitude = r_flameFX_magnitude->current.value;
        viewInfo->flameFx.distortionScale[0] = r_flameFX_distortionScaleFactor->current.value;
        viewInfo->flameFx.distortionScale[1] = r_flameFX_distortionScaleFactor->current.vector[1];
        viewInfo->flameFx.frameRate = (float)r_flameFX_FPS->current.integer;
    }
}

void __cdecl R_SetElectrifiedFx(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms)
{
    memcpy(&viewInfo->electrifiedFx, &sceneParms->electrifiedFx, sizeof(viewInfo->electrifiedFx));
    if ( r_flameFX_enable->current.enabled )
    {
        viewInfo->electrifiedFx.distortionMagnitude = r_flameFX_magnitude->current.value;
        viewInfo->electrifiedFx.distortionScale[0] = r_flameFX_distortionScaleFactor->current.value;
        viewInfo->electrifiedFx.distortionScale[1] = r_flameFX_distortionScaleFactor->current.vector[1];
        viewInfo->electrifiedFx.frameRate = (float)r_flameFX_FPS->current.integer;
    }
}

void __cdecl R_SetTransportedFx(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms)
{
    memcpy(&viewInfo->transportedFx, &sceneParms->transportedFx, sizeof(viewInfo->transportedFx));
    if ( r_flameFX_enable->current.enabled )
    {
        viewInfo->transportedFx.distortionMagnitude = r_flameFX_magnitude->current.value;
        viewInfo->transportedFx.distortionScale[0] = r_flameFX_distortionScaleFactor->current.value;
        viewInfo->transportedFx.distortionScale[1] = r_flameFX_distortionScaleFactor->current.vector[1];
        viewInfo->transportedFx.frameRate = (float)r_flameFX_FPS->current.integer;
    }
}

void __cdecl R_SetReviveFx(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms)
{
    const DvarValue *v2; // [esp+Ch] [ebp-14h]
    const DvarValue *v3; // [esp+14h] [ebp-Ch]
    const DvarValue *p_current; // [esp+1Ch] [ebp-4h]

    if ( r_reviveFX_debug->current.enabled )
    {
        viewInfo->reviveFx.enabled = 1;
        viewInfo->reviveFx.reviveEdgeColorTemp = r_reviveFX_edgeColorTemp->current.value;
        viewInfo->reviveFx.reviveEdgeSaturation = r_reviveFX_edgeSaturation->current.value;
        p_current = &r_reviveFX_edgeScale->current;
        viewInfo->reviveFx.reviveEdgeScale[0] = r_reviveFX_edgeScale->current.value;
        viewInfo->reviveFx.reviveEdgeScale[1] = p_current->vector[1];
        viewInfo->reviveFx.reviveEdgeScale[2] = p_current->vector[2];
        v3 = &r_reviveFX_edgeContrast->current;
        viewInfo->reviveFx.reviveEdgeContrast[0] = r_reviveFX_edgeContrast->current.value;
        viewInfo->reviveFx.reviveEdgeContrast[1] = v3->vector[1];
        viewInfo->reviveFx.reviveEdgeContrast[2] = v3->vector[2];
        v2 = &r_reviveFX_edgeOffset->current;
        viewInfo->reviveFx.reviveEdgeOffset[0] = r_reviveFX_edgeOffset->current.value;
        viewInfo->reviveFx.reviveEdgeOffset[1] = v2->vector[1];
        viewInfo->reviveFx.reviveEdgeOffset[2] = v2->vector[2];
        viewInfo->reviveFx.reviveEdgeMaskAdjust = r_reviveFX_edgeMaskAdjust->current.value;
        viewInfo->reviveFx.reviveEdgeAmount = r_reviveFX_edgeAmount->current.value;
    }
    else
    {
        memcpy(&viewInfo->reviveFx, &sceneParms->reviveFx, sizeof(viewInfo->reviveFx));
    }
}

void __cdecl R_SetGenericFilter(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms)
{
    float *dir; // [esp+4h] [ebp-8h]

    memcpy(
        (unsigned __int8 *)&viewInfo->genericFilter,
        (unsigned __int8 *)&sceneParms->genericFilter,
        sizeof(viewInfo->genericFilter));
    dir = frontEndDataOut->sunLight.dir;
    viewInfo->genericFilter.sunPosition[0] = (float)(10000.0 * frontEndDataOut->sunLight.dir[0])
                                                                                 + viewInfo->cullViewInfo.viewParms.origin[0];
    viewInfo->genericFilter.sunPosition[1] = (float)(10000.0 * dir[1]) + viewInfo->cullViewInfo.viewParms.origin[1];
    viewInfo->genericFilter.sunPosition[2] = (float)(10000.0 * dir[2]) + viewInfo->cullViewInfo.viewParms.origin[2];
}

#if 0
void __cdecl R_SetPoisonFx(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms)
{
    float curAmount; // edx
    double v3; // xmm0_8
    long double POISON_BLUR_SLOW; // [esp+0h] [ebp-1Ch]
    long double POISON_BLUR_SLOWa; // [esp+0h] [ebp-1Ch]
    float blend; // [esp+4h] [ebp-18h]
    float blenda; // [esp+4h] [ebp-18h]
    float blur1; // [esp+8h] [ebp-14h]
    float range; // [esp+Ch] [ebp-10h]
    float base; // [esp+18h] [ebp-4h]

    curAmount = sceneParms->poisonFx.curAmount;
    viewInfo->poisonFx.curAmountTarget = sceneParms->poisonFx.curAmountTarget;
    viewInfo->poisonFx.curAmount = curAmount;
    if ( r_poisonFX_debug_enable->current.enabled )
        viewInfo->poisonFx.curAmount = r_poisonFX_debug_amount->current.value;
    if ( viewInfo->poisonFx.curAmount > 0.0 )
    {
        blend = r_poisonFX_pulse->current.value - 1.0;
        if ( blend < 0.0 )
            blend = 0.0f;
        *((float *)&POISON_BLUR_SLOW + 1) = (float)((float)((float)(-2.0 * blend) + 3.0) * blend) * blend;
        *(float *)&POISON_BLUR_SLOW = 6.0f;
        blur1 = viewInfo->sceneDef.floatTime / 3.0;
        base = r_poisonFX_blurMin->current.value;
        range = r_poisonFX_blurMax->current.value - base;
        v3 = (float)((float)((float)(viewInfo->sceneDef.floatTime / 6.0) * 3.1415927) * 2.5);
        sin(POISON_BLUR_SLOW);
        *(float *)&v3 = v3;
        sin(POISON_BLUR_SLOWa);
        viewInfo->blurRadius = (float)((float)(1.0 - blenda)
                                                                 * (float)((float)((float)((float)(*(float *)&v3 * 0.5) + 0.5) * range) + base))
                                                 + (float)(blenda
                                                                 * (float)((float)((float)((float)((float)((float)(blur1 * 3.1415927) * 2.5) * 0.5) + 0.5)
                                                                                                 * range)
                                                                                 + base));
        viewInfo->blurRadius = viewInfo->blurRadius * viewInfo->poisonFx.curAmount;
    }
}
#endif

// aislop
void __cdecl R_SetPoisonFx(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms)
{
    float curAmount = sceneParms->poisonFx.curAmount;

    viewInfo->poisonFx.curAmountTarget = sceneParms->poisonFx.curAmountTarget;
    viewInfo->poisonFx.curAmount = curAmount;

    if (r_poisonFX_debug_enable->current.enabled)
        viewInfo->poisonFx.curAmount = r_poisonFX_debug_amount->current.value;

    if (viewInfo->poisonFx.curAmount <= 0.0f)
        return;

    float blend = r_poisonFX_pulse->current.value - 1.0f;
    if (blend < 0.0f)
        blend = 0.0f;

    float blenda = blend * blend * (3.0f - 2.0f * blend);

    float base = r_poisonFX_blurMin->current.value;
    float range = r_poisonFX_blurMax->current.value - base;

    float t0 = viewInfo->sceneDef.floatTime / 6.0f;
    float t1 = viewInfo->sceneDef.floatTime / 3.0f;

    float wave0 = sinf(t0 * 3.1415927f * 2.5f);
    float wave1 = sinf(t1 * 3.1415927f * 2.5f);

    float blur0 = ((wave0 * 0.5f) + 0.5f) * range + base;
    float blur1 = ((wave1 * 0.5f) + 0.5f) * range + base;

    viewInfo->blurRadius =
        ((1.0f - blenda) * blur0 + blenda * blur1) *
        viewInfo->poisonFx.curAmount;
}


void __cdecl R_SetFilmInfo(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms)
{
    iassert(viewInfo);
    iassert(sceneParms);

    if ( r_filmUseTweaks->current.enabled )
    {
        viewInfo->film.enabled = r_filmTweakEnable->current.enabled;

        viewInfo->film.filmHue[0] = r_filmTweakHue->current.vector[0];
        viewInfo->film.filmHue[1] = r_filmTweakHue->current.vector[1];
        viewInfo->film.filmHue[2] = r_filmTweakHue->current.vector[2];

        viewInfo->film.filmSaturation[0] = r_filmTweakSaturation->current.vector[0];
        viewInfo->film.filmSaturation[1] = r_filmTweakSaturation->current.vector[1];
        viewInfo->film.filmSaturation[2] = r_filmTweakSaturation->current.vector[2];

        viewInfo->film.filmColorTemp[0] = r_filmTweakColorTemp->current.vector[0];
        viewInfo->film.filmColorTemp[1] = r_filmTweakColorTemp->current.vector[1];
        viewInfo->film.filmColorTemp[2] = r_filmTweakColorTemp->current.vector[2];

        viewInfo->film.filmDarkTint[0] = r_filmTweakDarkTint->current.vector[0];
        viewInfo->film.filmDarkTint[1] = r_filmTweakDarkTint->current.vector[1];
        viewInfo->film.filmDarkTint[2] = r_filmTweakDarkTint->current.vector[2];

        viewInfo->film.filmMidTint[0] = r_filmTweakMidTint->current.vector[0];
        viewInfo->film.filmMidTint[1] = r_filmTweakMidTint->current.vector[1];
        viewInfo->film.filmMidTint[2] = r_filmTweakMidTint->current.vector[2];

        viewInfo->film.filmLightTint[0] = r_filmTweakLightTint->current.vector[0];
        viewInfo->film.filmLightTint[1] = r_filmTweakLightTint->current.vector[1];
        viewInfo->film.filmLightTint[2] = r_filmTweakLightTint->current.vector[2];

        viewInfo->film.filmContrast[0] = r_filmTweakContrast->current.vector[0];
        viewInfo->film.filmContrast[1] = r_filmTweakContrast->current.vector[1];
        viewInfo->film.filmContrast[2] = r_filmTweakContrast->current.vector[2];

        viewInfo->film.filmBleach[0] = r_filmTweakBleach->current.vector[0];
        viewInfo->film.filmBleach[1] = r_filmTweakBleach->current.vector[1];
        viewInfo->film.filmBleach[2] = r_filmTweakBleach->current.vector[2];
        viewInfo->film.filmMidStart = r_filmTweakMidStart->current.value;
        viewInfo->film.filmMidEnd = r_filmTweakMidEnd->current.value;
        viewInfo->film.filmDarkFeather = r_filmTweakDarkFeather->current.value;
        viewInfo->film.filmLightFeather = r_filmTweakLightFeather->current.value;
        viewInfo->film.sCurveEnable = (float)r_sCurveEnable->current.integer;
        viewInfo->film.sCurveShoulderStrength = r_sCurveShoulderStrength->current.value;
        viewInfo->film.sCurveLinearStrength = r_sCurveLinearStrength->current.value;
        viewInfo->film.sCurveLinearAngle = r_sCurveLinearAngle->current.value;
        viewInfo->film.sCurveToeStrength = r_sCurveToeStrength->current.value;
        viewInfo->film.sCurveToeNumerator = r_sCurveToeNumerator->current.value;
        viewInfo->film.sCurveToeDenominator = r_sCurveToeDenominator->current.value;
    }
    else
    {
        memcpy(&viewInfo->film, &sceneParms->film, sizeof(viewInfo->film));
    }
    if ( r_filmTweakLut->current.integer >= 0 )
        viewInfo->film.filmLut = (float)r_filmTweakLut->current.integer;
}

void __cdecl R_SetBloomInfo(GfxViewInfo *viewInfo, const GfxSceneParms *sceneParms)
{
    float v2; // [esp+8h] [ebp-110h]
    float v3; // [esp+Ch] [ebp-10Ch]
    float v4; // [esp+10h] [ebp-108h]
    float v5; // [esp+14h] [ebp-104h]
    float tx; // [esp+F8h] [ebp-20h]
    float z; // [esp+FCh] [ebp-1Ch]
    float tw; // [esp+104h] [ebp-14h]
    float ty; // [esp+10Ch] [ebp-Ch]
    float x; // [esp+110h] [ebp-8h]
    float xa; // [esp+110h] [ebp-8h]
    float y; // [esp+114h] [ebp-4h]
    float ya; // [esp+114h] [ebp-4h]

    if ( r_bloomTweaks->current.enabled )
    {
        Vec4Copy(r_bloomTintWeights->current.vector, viewInfo->bloom.bloomTintWeights);
        Vec4Copy(r_bloomColorScale->current.vector, viewInfo->bloom.bloomColorScale);
        Vec4Copy(r_bloomTintScale->current.vector, viewInfo->bloom.bloomTintScale);
        Vec4Copy(r_bloomCurveBreakpoint->current.vector, viewInfo->bloom.bloomCurveBreakpoint);
        Vec4Copy(r_bloomCurveLoBlack->current.vector, viewInfo->bloom.bloomCurveLoBlack);
        Vec4Copy(r_bloomCurveLoGamma->current.vector, viewInfo->bloom.bloomCurveLoGamma);
        Vec4Copy(r_bloomCurveLoWhite->current.vector, viewInfo->bloom.bloomCurveLoWhite);
        Vec4Copy(r_bloomCurveLoRemapBlack->current.vector, viewInfo->bloom.bloomCurveLoRemapBlack);
        Vec4Copy(r_bloomCurveLoRemapWhite->current.vector, viewInfo->bloom.bloomCurveLoRemapWhite);
        Vec4Copy(r_bloomCurveHiBlack->current.vector, viewInfo->bloom.bloomCurveHiBlack);
        Vec4Copy(r_bloomCurveHiGamma->current.vector, viewInfo->bloom.bloomCurveHiGamma);
        Vec4Copy(r_bloomCurveHiWhite->current.vector, viewInfo->bloom.bloomCurveHiWhite);
        Vec4Copy(r_bloomCurveHiRemapBlack->current.vector, viewInfo->bloom.bloomCurveHiRemapBlack);
        Vec4Copy(r_bloomCurveHiRemapWhite->current.vector, viewInfo->bloom.bloomCurveHiRemapWhite);
        Vec4Copy(r_bloomExpansionControl->current.vector, viewInfo->bloom.bloomExpansionControl);
        Vec4Copy(r_bloomExpansionWeights->current.vector, viewInfo->bloom.bloomExpansionWeights);

        viewInfo->bloom.bloomBlurRadius = r_bloomBlurRadius->current.value;
        viewInfo->bloom.bloomExpansionSource = r_bloomExpansionSource->current.integer;
        viewInfo->bloom.bloomPersistence = r_bloomPersistence->current.value;

        Vec4Copy(r_bloomStreakXLevels0->current.vector, viewInfo->bloom.bloomStreakXLevels0);
        Vec4Copy(r_bloomStreakXLevels1->current.vector, viewInfo->bloom.bloomStreakXLevels1);

        viewInfo->bloom.bloomStreakXInnerTint[0] = r_bloomStreakXInnerTint->current.vector[0];
        viewInfo->bloom.bloomStreakXInnerTint[1] = r_bloomStreakXInnerTint->current.vector[1];
        viewInfo->bloom.bloomStreakXInnerTint[2] = r_bloomStreakXInnerTint->current.vector[2];

        viewInfo->bloom.bloomStreakXOuterTint[0] = r_bloomStreakXOuterTint->current.vector[0];
        viewInfo->bloom.bloomStreakXOuterTint[0] = r_bloomStreakXOuterTint->current.vector[1];
        viewInfo->bloom.bloomStreakXOuterTint[0] = r_bloomStreakXOuterTint->current.vector[2];

        Vec4Copy(r_bloomStreakXTintControl->current.vector, viewInfo->bloom.bloomStreakXTintControl);

        viewInfo->bloom.bloomStreakXTint[0] = r_bloomStreakXTint->current.vector[0];
        viewInfo->bloom.bloomStreakXTint[1] = r_bloomStreakXTint->current.vector[1];
        viewInfo->bloom.bloomStreakXTint[2] = r_bloomStreakXTint->current.vector[2];

        Vec4Copy(r_bloomStreakYLevels0->current.vector, viewInfo->bloom.bloomStreakYLevels0);
        Vec4Copy(r_bloomStreakYLevels1->current.vector, viewInfo->bloom.bloomStreakYLevels1);

        viewInfo->bloom.bloomStreakYInnerTint[0] = r_bloomStreakYInnerTint->current.vector[0];
        viewInfo->bloom.bloomStreakYInnerTint[1] = r_bloomStreakYInnerTint->current.vector[1];
        viewInfo->bloom.bloomStreakYInnerTint[2] = r_bloomStreakYInnerTint->current.vector[2];

        viewInfo->bloom.bloomStreakYOuterTint[0] = r_bloomStreakYOuterTint->current.vector[0];
        viewInfo->bloom.bloomStreakYOuterTint[1] = r_bloomStreakYOuterTint->current.vector[1];
        viewInfo->bloom.bloomStreakYOuterTint[2] = r_bloomStreakYOuterTint->current.vector[2];

        Vec4Copy(r_bloomStreakYTintControl->current.vector, viewInfo->bloom.bloomStreakYTintControl);

        viewInfo->bloom.bloomStreakYTint[0] = r_bloomStreakYTint->current.vector[0];
        viewInfo->bloom.bloomStreakYTint[1] = r_bloomStreakYTint->current.vector[1];
        viewInfo->bloom.bloomStreakYTint[2] = r_bloomStreakYTint->current.vector[2];
    }
    else
    {
        memcpy(&viewInfo->bloom, &sceneParms->bloom, sizeof(viewInfo->bloom));
    }
    if ( viewInfo->bloom.bloomExpansionSource )
    {
        viewInfo->bloomExpansionXpos = viewInfo->bloom.bloomExpansionControl[2];
        viewInfo->bloomExpansionYpos = viewInfo->bloom.bloomExpansionControl[3];
    }
    else
    {
        x = rgp.world->sun.sunFxPosition[0];
        y = rgp.world->sun.sunFxPosition[1];
        z = rgp.world->sun.sunFxPosition[2];
        tw = (float)((float)(x * viewInfo->cullViewInfo.viewParms.viewProjectionMatrix.m[0][3])
                             + (float)(y * viewInfo->cullViewInfo.viewParms.viewProjectionMatrix.m[1][3]))
             + (float)(z * viewInfo->cullViewInfo.viewParms.viewProjectionMatrix.m[2][3]);
        tx = (float)((float)((float)(x * viewInfo->cullViewInfo.viewParms.viewProjectionMatrix.m[0][0])
                                             + (float)(y * viewInfo->cullViewInfo.viewParms.viewProjectionMatrix.m[1][0]))
                             + (float)(z * viewInfo->cullViewInfo.viewParms.viewProjectionMatrix.m[2][0]))
             / tw;
        ty = (float)((float)((float)(x * viewInfo->cullViewInfo.viewParms.viewProjectionMatrix.m[0][1])
                                             + (float)(y * viewInfo->cullViewInfo.viewParms.viewProjectionMatrix.m[1][1]))
                             + (float)(z * viewInfo->cullViewInfo.viewParms.viewProjectionMatrix.m[2][1]))
             / tw;
        viewInfo->bloomExpansionXpos = (float)(tx * 0.5) + 0.5;
        viewInfo->bloomExpansionYpos = 0.5 - (float)(ty * 0.5);
        if ( (float)((float)((float)(1.0 / (float)(0.75 - 1.0)) * fabs(tx))
                             - (float)((float)(1.0 / (float)(0.75 - 1.0)) * 1.0)) >= 0.0 )
            v5 = (float)((float)(1.0 / (float)(0.75 - 1.0)) * fabs(tx))
                 - (float)((float)(1.0 / (float)(0.75 - 1.0)) * 1.0);
        else
            v5 = 0.0f;
        if ( v5 <= 1.0 )
            v4 = v5;
        else
            v4 = 1.0f;
        if ( (float)((float)((float)(1.0 / (float)(0.75 - 1.0)) * fabs(ty))
                             - (float)((float)(1.0 / (float)(0.75 - 1.0)) * 1.0)) >= 0.0 )
            v3 = (float)((float)(1.0 / (float)(0.75 - 1.0)) * fabs(ty))
                 - (float)((float)(1.0 / (float)(0.75 - 1.0)) * 1.0);
        else
            v3 = 0.0f;
        if ( v3 <= 1.0 )
            v2 = v3;
        else
            v2 = 1.0f;
        xa = (float)(v4 * v4) * (float)(3.0 - (float)(2.0 * v4));
        ya = (float)(v2 * v2) * (float)(3.0 - (float)(2.0 * v2));
        viewInfo->bloom.bloomExpansionWeights[0] = (float)(xa * ya) * viewInfo->bloom.bloomExpansionWeights[0];
        viewInfo->bloom.bloomExpansionWeights[1] = (float)(xa * ya) * viewInfo->bloom.bloomExpansionWeights[1];
        viewInfo->bloom.bloomExpansionWeights[2] = (float)(xa * ya) * viewInfo->bloom.bloomExpansionWeights[2];
        viewInfo->bloom.bloomExpansionWeights[3] = (float)(xa * ya) * viewInfo->bloom.bloomExpansionWeights[3];
    }
}

void __cdecl R_SetFullSceneViewMesh(int viewInfoIndex, GfxViewInfo *viewInfo)
{
    float width; // [esp+24h] [ebp-14h]
    float height; // [esp+28h] [ebp-10h]
    GfxQuadMeshData *quadMesh; // [esp+2Ch] [ebp-Ch]
    float x; // [esp+30h] [ebp-8h]
    float y; // [esp+34h] [ebp-4h]

    quadMesh = &gfxMeshGlob.fullSceneViewMesh[viewInfoIndex];
    viewInfo->fullSceneViewMesh = quadMesh;
    x = (float)viewInfo->cullViewInfo.sceneViewport.x;
    y = (float)viewInfo->cullViewInfo.sceneViewport.y;
    width = (float)viewInfo->cullViewInfo.sceneViewport.width;
    height = (float)viewInfo->cullViewInfo.sceneViewport.height;
    if ( quadMesh->x != x || quadMesh->y != y || quadMesh->width != width || quadMesh->height != height )
    {
        R_SyncRenderThread();
        R_SetQuadMesh(quadMesh, x, y, width, height, 0.0, 0.0, 1.0, 1.0, 0xFFFFFFFF);
        if ( (quadMesh->x != x || quadMesh->y != y || quadMesh->width != width || quadMesh->height != height)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                        3648,
                        1,
                        "%s",
                        "quadMesh->x == x && quadMesh->y == y && quadMesh->width == width && quadMesh->height == height") )
        {
            __debugbreak();
        }
    }
}

void __cdecl R_GenerateSortedSunShadowDrawSurfs(GfxViewInfo *viewInfo)
{
    R_MergeAndEmitSunShadowMapsSurfs(viewInfo);
    iassert(frontEndDataOut);
}

void __cdecl R_AddEmissiveSpotLight(GfxViewInfo *viewInfo)
{
    frontEndDataOut->emissiveSpotLightCount = 0;
    if ( !viewInfo->isMissileCamera )
    {
        if (r_dlightLimit->current.integer && gfxDrawMethod.drawScene == GFX_DRAW_SCENE_STANDARD && !R_CullDynamicSpotLightInCameraView() )
        {
            iassert(scene.addedLightCount > 0);
            iassert(!scene.isAddedLightCulled[0]);

            GfxLight *spotLight = &scene.addedLight[0];
            iassert(spotLight->type == GFX_LIGHT_TYPE_SPOT);

            memcpy(&frontEndDataOut->emissiveSpotLight, spotLight, sizeof(GfxLight));
            frontEndDataOut->emissiveSpotLightIndex = 0;
            frontEndDataOut->emissiveSpotDrawSurfCount = 0;
            frontEndDataOut->emissiveSpotDrawSurfs = 0;
            frontEndDataOut->emissiveSpotLightCount = 1;
            if (spotLight->canUseShadowMap )
                R_AddDynamicShadowableLight(viewInfo, spotLight);
        }
    }
}

int __cdecl R_GetVisibleDLights(GfxLight *visibleLights)
{
    int visibleLightCount; // [esp+14h] [ebp-4h]

    PROF_SCOPED("R_GetVisibleDLights");
    visibleLightCount = 0;
    if ( r_dlightLimit->current.integer && gfxDrawMethod.drawScene == GFX_DRAW_SCENE_STANDARD )
    {
        R_CullDynamicPointLightsInCameraView();
        PROF_SCOPED("lights partition");
        visibleLightCount = R_GetPointLightPartitions(visibleLights);
    }

    return visibleLightCount;
}

void __cdecl R_GetLightSurfs(const GfxLight *visibleLights, int visibleLightCount)
{
    if ( visibleLightCount )
    {
        PROF_SCOPED("R_GetLightSurfs");
        {
            PROF_SCOPED("light surfs bsp");
            R_GetBspLightSurfs(visibleLights, visibleLightCount);
        }
        {
            PROF_SCOPED("light surfs smodel");
            R_GetStaticModelLightSurfs(visibleLights, visibleLightCount);
        }
        {
            PROF_SCOPED("light surfs scene ent");
            R_GetSceneEntLightSurfs(visibleLights, visibleLightCount);
        }
    }
}

void __cdecl R_GetPointLightShadowSurfs(
                GfxViewInfo *viewInfo,
                GfxVisibleLight *visibleLights,
                const GfxLight *lights,
                int numLights)
{
    PROF_SCOPED("R_GetPointLightShadowSurfs");

    if ( frontEndDataOut->emissiveSpotLightCount )
    {
        iassert(frontEndDataOut->emissiveSpotLightCount == 1);
        bcassert(frontEndDataOut->emissiveSpotLightIndex, MAX_VISIBLE_DLIGHTS);
        iassert(frontEndDataOut->emissiveSpotLightIndex == 0);
        iassert(numLights > 0);
        iassert(lights[0].type == GFX_LIGHT_TYPE_SPOT);

        frontEndDataOut->emissiveSpotDrawSurfCount = visibleLights->drawSurfCount;
        frontEndDataOut->emissiveSpotDrawSurfs = visibleLights->drawSurfs;
    }
}

void __cdecl R_SetSunShadowConstants(GfxCmdBufInput *input, const GfxSunShadowProjection *sunProj)
{
    R_SetInputCodeConstantFromVec4(input, CONST_SRC_CODE_SHADOWMAP_SWITCH_PARTITION, (float*)sunProj->switchPartition);
    R_SetInputCodeConstantFromVec4(input, CONST_SRC_CODE_SHADOWMAP_SCALE, (float*)sunProj->shadowmapScale);
}

void    R_SetHeroLighting(GfxCmdBufInput *input, GfxViewInfo *viewInfo)
{
    DvarValue sc; // [esp+9Ch] [ebp-F0h]
    GfxMatrix temp; // [esp+B0h] [ebp-DCh] BYREF
    GfxMatrix color; // [esp+F0h] [ebp-9Ch] BYREF
    float s; // [esp+170h] [ebp-1Ch]
    float bwght; // [esp+174h] [ebp-18h]
    float gwght; // [esp+178h] [ebp-14h]
    float rwght; // [esp+17Ch] [ebp-10h]

    rwght = 0.25f;
    gwght = 0.5f;
    bwght = 0.25f;
    s = r_heroLightSaturation->current.value;
    rwght = (float)(1.0 - s) * 0.25;
    gwght = (float)(1.0 - s) * 0.5;
    bwght = rwght;

    GfxMatrix saturation; // [esp+130h] [ebp-5Ch] BYREF
    saturation.m[0][0] = rwght + s;
    saturation.m[0][1] = rwght;
    saturation.m[0][2] = rwght;
    saturation.m[0][3] = 0.0f;
    saturation.m[1][0] = gwght;
    saturation.m[1][1] = gwght + s;
    saturation.m[1][2] = gwght;
    saturation.m[1][3] = 0.0f;
    saturation.m[2][0] = rwght;
    saturation.m[2][1] = rwght;
    saturation.m[2][2] = rwght + s;
    saturation.m[2][3] = 0.0f;
    saturation.m[3][0] = 0.0f;
    saturation.m[3][1] = 0.0f;
    saturation.m[3][2] = 0.0f;
    saturation.m[3][3] = 1.0f;
    colorTempMatrix(color.m, r_heroLightColorTemp->current.value);
    MatrixMultiply44(color.m, saturation.m, temp.m);
    sc = r_heroLightScale->current;
    R_SetInputCodeConstant(
        input,
        0xA9u,
        viewInfo->charPrimaryLightScale.diffuseScale,
        viewInfo->charPrimaryLightScale.specularScale,
        1.0,
        1.0);

    GfxMatrix scale; // [esp+50h] [ebp-13Ch] BYREF
    scale.m[0][0] = sc.vector[0];
    scale.m[0][1] = 0.0f;
    scale.m[0][2] = 0.0f;
    scale.m[0][3] = 0.0f;
    scale.m[1][0] = 0.0f;
    scale.m[1][1] = sc.vector[1];
    scale.m[1][2] = 0.0f;
    scale.m[1][3] = 0.0f;
    scale.m[2][0] = 0.0f;
    scale.m[2][1] = 0.0f;
    scale.m[2][2] = sc.vector[2];
    scale.m[2][3] = 0.0f;
    scale.m[3][0] = 0.0f;
    scale.m[3][1] = 0.0f;
    scale.m[3][2] = 0.0f;
    scale.m[3][3] = 1.0f;

    GfxMatrix final; // [esp+10h] [ebp-17Ch] BYREF
    MatrixMultiply44(temp.m, scale.m, final.m);
    R_SetInputCodeConstant(input, CONST_SRC_CODE_HERO_LIGHTING_R, final.m[0][0], final.m[1][0], final.m[2][0], 0.0);
    R_SetInputCodeConstant(input, CONST_SRC_CODE_HERO_LIGHTING_G, final.m[0][1], final.m[1][1], final.m[2][1], 0.0);
    R_SetInputCodeConstant(input, CONST_SRC_CODE_HERO_LIGHTING_B, final.m[0][2], final.m[1][2], final.m[2][2], 0.0);
}

void __cdecl R_SetDLightsConstantsDefaults(GfxCmdBufInput *input)
{
    R_SetInputCodeImage(input, TEXTURE_SRC_CODE_DLIGHT_ATTENUATION, rgp.whiteImage);
    R_SetInputCodeImageSamplerState(input, 0x11u, 0x61u);

    R_SetInputCodeImage(input, TEXTURE_SRC_CODE_SHADOWMAP_SPOT, rgp.whiteImage);
    R_SetInputCodeImageSamplerState(input, 7u, 0x62u);
}

// aislop
void R_SetDLightsConstants(
    GfxCmdBufInput *input,
    const GfxViewInfo *viewInfo,
    const GfxLight *visibleLights,
    int visibleLightCount)
{
    // ---- Per-omni SoA arrays ([0] always 0; loop writes [i+1]) ----
    float xs[4]           = {};  // unused; just holds the 0 used as slot 0x8B's W
    float ys[4]           = {};  // omni X positions, view-relative
    float zs[4]           = {};  // omni Y positions
    float falloffs[4]     = {};  // omni Z positions
    float reds[4]         = {};  // omni -1/r^2
    float greens[4]       = {};  // omni R-diffuse scaled
    float blues[4]        = {};  // omni G-diffuse scaled
    float diffuseColor[4] = {};  // [0]=spot lightDef ptr (alias), [1..3]=omni B-diffuse

    // ---- Spot-light scratch (only populated if a spot is processed) ----
    float lightAttentuation[4] = {};                          // [0]=L.attenuation[3], [1..3]=pos view-rel
    float lightDir[3]          = {};                          // = L.attenuation[0..2]
    float specularColor[3]     = {};                          // = L.diffuseColor * diffuseScale
    float lightOrigin[4]       = { 1.0f, 0.0f, 0.0f, 0.0f };  // [2..3] = spec[0..1] scaled
    float v60                  = 0.0f;                        // = spec[2] scaled

    float lightCosHalfFovInner = 0.0f;   // = L.dir[0]
    float spotShadowFade       = 0.0f;   // = L.dir[1]
    float lightRadius          = 0.0f;   // = L.dir[2]
    float lightCosHalfFovOuter = 0.0f;   // = L.radius
    int   lightExponent        = 0;      // = spotShadow->fade bits on shadow-match, else 0
    float cutOn = 0.0f, cutOff = 0.0f;   // = L.cosHalfFovInner / cosHalfFovOuter

    // Spot view/proj matrices and the light's falloff[0..3] kept verbatim.
    float viewMatrix[4][4]    = {};  // = L.viewMatrix.m (4th row reused as scratch below)
    float projMatrix[4][4]    = {};  // = L.projMatrix.m
    float falloffParams[4]    = {};  // = L.falloff[0..3]

    const GfxSpotShadow *spotShadow = nullptr;

    // ===== Accumulate per-light data =====
    // IDA original uses `ys[i + 1]` etc. — indexed by the visible-light index. That's a
    // latent OOB: the arrays are float[4], the upload only consumes slots [1..3], and the
    // loop bound is `visibleLightCount` with no cap. When the caller passes 4+ visible
    // lights (e.g. 1 spot + 3 omnis, or 4 omnis), the last omni's `i+1` lands at array
    // index 4, smashing the next stack local. BO presumably relied on caller-side filtering
    // to never trigger this; the kisak port hits it. We use a dedicated omni counter
    // capped at 3 so writes stay in-range.
    int omniIndex = 1;
    for (int i = 0; i < visibleLightCount; ++i)
    {
        const GfxLight &L = visibleLights[i];

        if (L.type == GFX_LIGHT_TYPE_OMNI)
        {
            if (omniIndex > 3)
                continue;  // only 3 packed omni slots available; ignore the rest

            ys[omniIndex]       = L.origin[0] - viewInfo->cullViewInfo.viewParms.origin[0];
            zs[omniIndex]       = L.origin[1] - viewInfo->cullViewInfo.viewParms.origin[1];
            falloffs[omniIndex] = L.origin[2] - viewInfo->cullViewInfo.viewParms.origin[2];
            reds[omniIndex]     = -1.0f / (L.radius * L.radius);

            const float exposureInv  = 1.0f / viewInfo->exposureRemap.remapMul[0];
            const float diffuseScale = r_diffuseColorScale->current.value;
            greens[omniIndex]       = (L.diffuseColor[0] * diffuseScale) * exposureInv;
            blues[omniIndex]        = (L.diffuseColor[1] * diffuseScale) * exposureInv;
            diffuseColor[omniIndex] = (L.diffuseColor[2] * diffuseScale) * exposureInv;
            ++omniIndex;
        }
        else
        {
            iassert(L.type == GFX_LIGHT_TYPE_SPOT);

            // Stash spot lightDef pointer bit-aliased into diffuseColor[0]. The non-null
            // check below is what triggers the spot's attenuation-image binding.
            *(const GfxLightDef **)&diffuseColor[0] = L.def;

            lightAttentuation[1] = L.origin[0] - viewInfo->cullViewInfo.viewParms.origin[0];
            lightAttentuation[2] = L.origin[1] - viewInfo->cullViewInfo.viewParms.origin[1];
            lightAttentuation[3] = L.origin[2] - viewInfo->cullViewInfo.viewParms.origin[2];
            lightCosHalfFovOuter = L.radius;

            lightDir[0] = L.attenuation[0] + 1.5287891e-5f;  // ~2^-16 epsilon
            lightDir[1] = L.attenuation[1];
            lightDir[2] = L.attenuation[2];
            lightAttentuation[0] = L.attenuation[3];

            lightCosHalfFovInner = L.dir[0];
            spotShadowFade       = L.dir[1];
            lightRadius          = L.dir[2];

            cutOn  = L.cosHalfFovInner;
            cutOff = L.cosHalfFovOuter;
            // IDA also writes `nearEdge = L.exponent;` here; dead — never read.

            falloffParams[0] = L.falloff[0];
            falloffParams[1] = L.falloff[1];
            falloffParams[2] = L.falloff[2];
            falloffParams[3] = L.falloff[3];

            memcpy(viewMatrix, L.viewMatrix.m, sizeof(viewMatrix));
            memcpy(projMatrix, L.projMatrix.m, sizeof(projMatrix));

            // Find a matching shadow record for this spot.
            for (unsigned j = 0; j < frontEndDataOut->shadowableLightCount; ++j)
            {
                const auto &S = frontEndDataOut->shadowableLights[j];
                if (S.type == GFX_LIGHT_TYPE_SPOT
                    && S.spotShadowIndex != -1
                    && L.def == S.def
                    && L.origin[0] == S.origin[0]
                    && L.origin[1] == S.origin[1]
                    && L.origin[2] == S.origin[2])
                {
                    spotShadow   = &frontEndDataOut->spotShadows[S.spotShadowIndex];
                    lightExponent = *(int *)&spotShadow->fade;
                    break;
                }
            }

            // IDA's `specularColor` array actually holds *scaled diffuse*.
            const float diffuseScale = r_diffuseColorScale->current.value;
            specularColor[0] = diffuseScale * L.diffuseColor[0];
            specularColor[1] = diffuseScale * L.diffuseColor[1];
            specularColor[2] = diffuseScale * L.diffuseColor[2];

            // Scaled specular spread across lightOrigin[2..3] + v60.
            const float specScale = r_specularColorScale->current.value;
            lightOrigin[2] = specScale * L.specularColor[0];
            lightOrigin[3] = specScale * L.specularColor[1];
            v60            = specScale * L.specularColor[2];
        }
    }

    // ===== Upload GLIGHT_POSXS..BLUES (slots 0x8B-0x91) =====
    // W of each slot pulls the previous attribute's [0] — always 0.
    R_SetInputCodeConstant(input, CONST_SRC_CODE_GLIGHT_POSXS,    ys[1],       ys[2],       ys[3],       xs[0]);
    R_SetInputCodeConstant(input, CONST_SRC_CODE_GLIGHT_POSYS,    zs[1],       zs[2],       zs[3],       ys[0]);
    R_SetInputCodeConstant(input, CONST_SRC_CODE_GLIGHT_POSZS,    falloffs[1], falloffs[2], falloffs[3], zs[0]);
    R_SetInputCodeConstant(input, CONST_SRC_CODE_GLIGHT_FALLOFFS, reds[1],     reds[2],     reds[3],     falloffs[0]);
    R_SetInputCodeConstant(input, CONST_SRC_CODE_GLIGHT_REDS,     greens[1],   greens[2],   greens[3],   reds[0]);
    R_SetInputCodeConstant(input, CONST_SRC_CODE_GLIGHT_GREENS,   blues[1],    blues[2],    blues[3],    greens[0]);
    // diffuseColor[3] is the slot IDA names `*(float*)&lightDef` (the omni loop OOB-writes here).
    R_SetInputCodeConstant(input, CONST_SRC_CODE_GLIGHT_BLUES,
                           diffuseColor[1], diffuseColor[2], diffuseColor[3], blues[0]);

    // ===== Bind spot attenuation image (codeImages[17]) =====
    if (*(uint32_t *)&diffuseColor[0])
    {
        const GfxLightDef *spotDef = *(const GfxLightDef **)&diffuseColor[0];
        input->codeImages[TEXTURE_SRC_CODE_DLIGHT_ATTENUATION] = spotDef->attenuation.image;
        R_SetInputCodeImageSamplerState(input, 0x11u, spotDef->attenuation.samplerState);
    }
    else
    {
        input->codeImages[TEXTURE_SRC_CODE_DLIGHT_ATTENUATION] = rgp.whiteImage;
        R_SetInputCodeImageSamplerState(input, 0x11u, 0x61u);
    }

    // ===== DLIGHT_SHADOW_LOOKUP_MATRIX_0..3 (slots 0x9D-0xA0) + DLIGHT_SPOT_SHADOWMAP_PIXEL_ADJUST (0x47) =====
    // Build the eye origin (W=1). The viewParms.origin[3] sentinel decides whether xyz
    // is the actual eye position or zeroed (e.g., for shadow-camera projections).
    float eyeOrigin[4];
    if (viewInfo->cullViewInfo.viewParms.origin[3] == 0.0f)
    {
        eyeOrigin[0] = 0.0f;
        eyeOrigin[1] = 0.0f;
        eyeOrigin[2] = 0.0f;
    }
    else
    {
        // IDA pulls XY from viewInfo->viewParms.origin but Z from cullViewInfo.viewParms.origin.
        eyeOrigin[0] = viewInfo->viewParms.origin[0];
        eyeOrigin[1] = viewInfo->viewParms.origin[1];
        eyeOrigin[2] = viewInfo->cullViewInfo.viewParms.origin[2];
    }
    eyeOrigin[3] = 1.0f;

    if (spotShadow)
    {
        float lookupMatrix[4][4];
        memcpy(lookupMatrix, &spotShadow->lookupMatrix, sizeof(lookupMatrix));

        // The 4th row of lookupMatrix is replaced by (eyeOrigin * lookupMatrix), so the
        // shader gets the world eye position pre-transformed into shadow space.
        float transformedEye[4];
        MatrixTransformVector44(eyeOrigin, (const float (*)[4])lookupMatrix, transformedEye);

        R_SetInputCodeConstantFromVec4(input, CONST_SRC_CODE_DLIGHT_SHADOW_LOOKUP_MATRIX_0, lookupMatrix[0]);
        R_SetInputCodeConstantFromVec4(input, CONST_SRC_CODE_DLIGHT_SHADOW_LOOKUP_MATRIX_1, lookupMatrix[1]);
        R_SetInputCodeConstantFromVec4(input, CONST_SRC_CODE_DLIGHT_SHADOW_LOOKUP_MATRIX_2, lookupMatrix[2]);
        R_SetInputCodeConstantFromVec4(input, CONST_SRC_CODE_DLIGHT_SHADOW_LOOKUP_MATRIX_3, transformedEye);

        input->codeImages[TEXTURE_SRC_CODE_SHADOWMAP_SPOT] = spotShadow->image;
        R_SetInputCodeImageSamplerState(input, 7u, 0x62u);
        R_SetInputCodeConstantFromVec4(input, CONST_SRC_CODE_DLIGHT_SPOT_SHADOWMAP_PIXEL_ADJUST,
                                       spotShadow->pixelAdjust);
    }
    else
    {
        input->codeImages[TEXTURE_SRC_CODE_SHADOWMAP_SPOT] = rgp.whiteImage;
        R_SetInputCodeImageSamplerState(input, 7u, 0x62u);
    }

    // ===== DLIGHT_ATTENUATION (slot 0x95) — actually the spot direction =====
    R_SetInputCodeConstantFromVec4(input, CONST_SRC_CODE_DLIGHT_ATTENUATION, lightDir);

    // ===== DLIGHT_FALLOFF (slot 0x96) — outer/inner slope+bias =====
    // Outer edge spans falloff[0] -> falloff[2]; inner edge spans falloff[3] -> falloff[1].
    float outerSlope, outerBias, innerSlope, innerBias;
    if (falloffParams[0] == falloffParams[2]) {
        outerSlope = 1.0f;
        outerBias  = -falloffParams[0];
    } else {
        outerSlope = 1.0f / (falloffParams[2] - falloffParams[0]);
        outerBias  = -falloffParams[0] * outerSlope;
    }
    if (falloffParams[3] == falloffParams[1]) {
        innerSlope = 1.0f;
        innerBias  = -falloffParams[3];
    } else {
        innerSlope = 1.0f / (falloffParams[1] - falloffParams[3]);
        innerBias  = -falloffParams[3] * innerSlope;
    }
    R_SetInputCodeConstant(input, CONST_SRC_CODE_DLIGHT_FALLOFF,
                           outerSlope, innerSlope, outerBias, innerBias);

    // ===== DLIGHT_SPOT_MATRIX_0..3 (slots 0x97-0x9A) — spot view*proj =====
    // 1. Transform light position through the spot's view matrix; the result replaces the
    //    view matrix's translation row (row 3).
    MatrixTransformVector44(&lightAttentuation[1],
                            (const float (*)[4])viewMatrix,
                            viewMatrix[3]);
    // 2. Negate the xyz of that translation row (leave w alone).
    viewMatrix[3][0] = -viewMatrix[3][0];
    viewMatrix[3][1] = -viewMatrix[3][1];
    viewMatrix[3][2] = -viewMatrix[3][2];
    // 3. Compose final = viewMatrix * projMatrix.
    float finalMatrix[4][4];
    MatrixMultiply44((const float (*)[4])viewMatrix,
                     (const float (*)[4])projMatrix,
                     finalMatrix);
    // 4. Upload transposed — each slot gets one column of the row-major finalMatrix.
    R_SetInputCodeConstant(input, CONST_SRC_CODE_DLIGHT_SPOT_MATRIX_0,
                           finalMatrix[0][0], finalMatrix[1][0], finalMatrix[2][0], finalMatrix[3][0]);
    R_SetInputCodeConstant(input, CONST_SRC_CODE_DLIGHT_SPOT_MATRIX_1,
                           finalMatrix[0][1], finalMatrix[1][1], finalMatrix[2][1], finalMatrix[3][1]);
    R_SetInputCodeConstant(input, CONST_SRC_CODE_DLIGHT_SPOT_MATRIX_2,
                           finalMatrix[0][2], finalMatrix[1][2], finalMatrix[2][2], finalMatrix[3][2]);
    R_SetInputCodeConstant(input, CONST_SRC_CODE_DLIGHT_SPOT_MATRIX_3,
                           finalMatrix[0][3], finalMatrix[1][3], finalMatrix[2][3], finalMatrix[3][3]);

    // ===== DLIGHT_POSITION (slot 0x92) — xyz = pos rel view, w = 1/r^2 =====
    R_SetInputCodeConstant(input, CONST_SRC_CODE_DLIGHT_POSITION,
        lightAttentuation[1], lightAttentuation[2], lightAttentuation[3],
        1.0f / (lightCosHalfFovOuter * lightCosHalfFovOuter));

    // ===== DLIGHT_DIFFUSE (slot 0x93) — specularColor[] really holds scaled diffuse =====
    R_SetInputCodeConstant(input, CONST_SRC_CODE_DLIGHT_DIFFUSE,
                           specularColor[0], specularColor[1], specularColor[2], 1.0f);

    // ===== DLIGHT_SPECULAR (slot 0x94) =====
    R_SetInputCodeConstant(input, CONST_SRC_CODE_DLIGHT_SPECULAR,
                           lightOrigin[2], lightOrigin[3], v60, 1.0f);

    // ===== DLIGHT_SPOT_DIR (slot 0x9B) — = L.dir[0..2] =====
    R_SetInputCodeConstant(input, CONST_SRC_CODE_DLIGHT_SPOT_DIR,
                           lightCosHalfFovInner, spotShadowFade, lightRadius, 0.0f);

    // ===== DLIGHT_SPOT_FACTORS (slot 0x9C) — cone dot scale/bias, w = shadow fade bits =====
    const float spotDotScale = 1.0f / (cutOn - cutOff);
    const float spotDotBias  = -cutOff * spotDotScale;
    R_SetInputCodeConstant(input, CONST_SRC_CODE_DLIGHT_SPOT_FACTORS,
                           spotDotScale, spotDotBias, 0.0f, *(float *)&lightExponent);
}


void R_DrawCineWarning()
{
    Font_s *font; // [esp+1Ch] [ebp-8h]
    const char *msg; // [esp+20h] [ebp-4h]

    if ( com_statmon->current.enabled && R_Cinematic_IsStarted() )
    {
        msg = "CINE";
        font = R_RegisterFont("fonts/consoleFont", 1);
        if ( R_Cinematic_IsUnderrun() )
            msg = "CINE UNDERRUN!";
        if ( cg_draw2D->current.enabled )
            R_AddCmdDrawText((char*)msg, 0x7FFFFFFF, font, 0.0, 360.0, 1.5, 2.0, 0.0, colorRedFaded, 0);
    }
}

void __cdecl R_SplitDrawSurfacesPrimarySortKey(
                GfxDrawSurfListInfo *srcList,
                GfxDrawSurfListInfo *destList,
                int sortkeyID)
{
    unsigned __int16 start; // [esp+18h] [ebp-4h]

    for ( start = 0;
                start < srcList->drawSurfCount
         && (sortkeyID < 0
            || (sortkeyID < 0 || sortkeyID >= 0) && ((srcList->drawSurfs[start].packed >> 58) & 0x3F) < sortkeyID);
                ++start )
    {
        ;
    }
    destList->drawSurfCount = srcList->drawSurfCount - start;
    destList->drawSurfs = &srcList->drawSurfs[start];
    srcList->drawSurfCount = start;
}

void __cdecl DrawLightDebug(const GfxViewInfo *viewInfo, const GfxLight *L, const float *debugColor)
{
    float target[3]; // [esp+14h] [ebp-40h] BYREF
    float mins[3]; // [esp+20h] [ebp-34h] BYREF
    float ext[3]; // [esp+2Ch] [ebp-28h]
    float maxs[3]; // [esp+38h] [ebp-1Ch] BYREF
    float lightColor[4]; // [esp+44h] [ebp-10h] BYREF

    ext[0] = 10.0f;
    ext[1] = 10.0f;
    ext[2] = 10.0f;
    maxs[0] = L->origin[0] + 10.0;
    maxs[1] = L->origin[1] + 10.0;
    maxs[2] = L->origin[2] + 10.0;
    mins[0] = (float)(-1.0 * 10.0) + L->origin[0];
    mins[1] = (float)(-1.0 * 10.0) + L->origin[1];
    mins[2] = (float)(-1.0 * 10.0) + L->origin[2];
    lightColor[0] = L->color[0];
    lightColor[1] = L->color[1];
    lightColor[2] = L->color[2];
    lightColor[3] = 1.0f;
    lightColor[0] = (float)(viewInfo->exposureValue.blackPoint[3] / 8.0) * lightColor[0];
    lightColor[1] = (float)(viewInfo->exposureValue.blackPoint[3] / 8.0) * lightColor[1];
    lightColor[2] = (float)(viewInfo->exposureValue.blackPoint[3] / 8.0) * lightColor[2];
    R_AddDebugBox(&frontEndDataOut->debugGlobals, mins, maxs, debugColor);
    if ( L->type == 2 )
    {
        target[0] = (float)(-20.0 * L->dir[0]) + L->origin[0];
        target[1] = (float)(-20.0 * L->dir[1]) + L->origin[1];
        target[2] = (float)(-20.0 * L->dir[2]) + L->origin[2];
        R_AddDebugLine(&frontEndDataOut->debugGlobals, L->origin, target, lightColor, 0);
    }
    else
    {
        R_AddDebugLine(&frontEndDataOut->debugGlobals, mins, maxs, lightColor, 0);
    }
}

void DrawOutdoorBoundsVolumeDebug()
{
    float boxColor[4]; // [esp+0h] [ebp-14h] BYREF

    if ( r_showOutdoorVolumeDebug->current.enabled )
    {
        if ( rgp.world->numOutdoorBounds )
        {
            boxColor[0] = 1.0f;
            boxColor[1] = 1.0f;
            boxColor[2] = 1.0f;
            boxColor[3] = 1.0f;
            R_AddDebugBox(
                &frontEndDataOut->debugGlobals,
                (const float *)rgp.world->outdoorBounds,
                rgp.world->outdoorBounds->bounds[1],
                boxColor);
        }
    }
}

void __cdecl DrawSunDirectionDebug(const float *viewOrg, const float *viewForward)
{
    float boxColor[4]; // [esp+4h] [ebp-A4h] BYREF
    float lineEnd[3]; // [esp+14h] [ebp-94h] BYREF
    float zdir[3]; // [esp+20h] [ebp-88h] BYREF
    float sunColor[4]; // [esp+2Ch] [ebp-7Ch] BYREF
    float mins[3]; // [esp+3Ch] [ebp-6Ch] BYREF
    float zColor[4]; // [esp+48h] [ebp-60h] BYREF
    float xColor[4]; // [esp+58h] [ebp-50h] BYREF
    float ext[3]; // [esp+68h] [ebp-40h]
    float ydir[3]; // [esp+74h] [ebp-34h] BYREF
    float maxs[3]; // [esp+80h] [ebp-28h] BYREF
    float yColor[4]; // [esp+8Ch] [ebp-1Ch] BYREF
    float xdir[3]; // [esp+9Ch] [ebp-Ch] BYREF

    if ( r_showSunDirectionDebug->current.enabled )
    {
        if (!lastShowDebug)
        {
            debugOrigin[0] = (float)(100.0 * *viewForward) + *viewOrg;
            debugOrigin[1] = (float)(100.0 * viewForward[1]) + viewOrg[1];
            debugOrigin[2] = (float)(100.0 * viewForward[2]) + viewOrg[2];
        }
        lineEnd[0] = (float)(200.0 * frontEndDataOut->sunLight.dir[0]) + debugOrigin[0];
        lineEnd[1] = (float)(200.0 * frontEndDataOut->sunLight.dir[1]) + debugOrigin[1];
        lineEnd[2] = (float)(200.0 * frontEndDataOut->sunLight.dir[2]) + debugOrigin[2];
        sunColor[0] = 1.0f;
        sunColor[1] = 1.0f;
        sunColor[2] = 1.0f;
        sunColor[3] = 1.0f;
        R_AddDebugLine(&frontEndDataOut->debugGlobals, debugOrigin, lineEnd, sunColor, 0);
        boxColor[0] = 1.0f;
        boxColor[1] = 1.0f;
        boxColor[2] = 1.0f;
        boxColor[3] = 1.0f;
        ext[0] = 5.0f;
        ext[1] = 5.0f;
        ext[2] = 5.0f;
        maxs[0] = debugOrigin[0] + 5.0;
        maxs[0] = debugOrigin[0] + 5.0;
        maxs[1] = debugOrigin[1] + 5.0;
        maxs[2] = debugOrigin[2] + 5.0;
        mins[0] = (float)(-1.0 * 5.0) + debugOrigin[0];
        mins[1] = (float)(-1.0 * 5.0) + debugOrigin[1];
        mins[2] = (float)(-1.0 * 5.0) + debugOrigin[2];
        R_AddDebugBox(&frontEndDataOut->debugGlobals, mins, maxs, boxColor);
        xColor[0] = 1.0f;
        xColor[1] = 0.0f;
        xColor[2] = 0.0f;
        xColor[3] = 1.0f;
        yColor[0] = 0.0f;
        yColor[1] = 1.0f;
        yColor[2] = 0.0f;
        yColor[3] = 1.0f;
        zColor[0] = 0.0f;
        zColor[1] = 0.0f;
        zColor[2] = 1.0f;
        zColor[3] = 1.0f;
        xdir[0] = (float)(40.0 * 1.0) + debugOrigin[0];
        xdir[1] = (float)(40.0 * 0.0) + debugOrigin[1];
        xdir[2] = (float)(40.0 * 0.0) + debugOrigin[2];
        ydir[0] = (float)(40.0 * 0.0) + debugOrigin[0];
        ydir[1] = (float)(40.0 * 1.0) + debugOrigin[1];
        ydir[2] = xdir[2];
        zdir[0] = ydir[0];
        zdir[1] = xdir[1];
        zdir[2] = (float)(40.0 * 1.0) + debugOrigin[2];
        R_AddDebugLine(&frontEndDataOut->debugGlobals, debugOrigin, xdir, xColor, 0);
        R_AddDebugLine(&frontEndDataOut->debugGlobals, debugOrigin, ydir, yColor, 0);
        R_AddDebugLine(&frontEndDataOut->debugGlobals, debugOrigin, zdir, zColor, 0);
    }
    lastShowDebug = r_showSunDirectionDebug->current.enabled;
}

void R_SortAllCodeMeshSurfacesSunShadow()
{
    PROF_SCOPED("R_SortAllCodeMeshSurfacesSunShadow");

    R_SortDrawSurfs(scene.drawSurfs[22], scene.drawSurfCount[22]);
    R_SortDrawSurfs(scene.drawSurfs[26], scene.drawSurfCount[26]);
}

void R_DrawFogParams()
{
    Font_s *font; // [esp+20h] [ebp-A0h]
    GfxFog *f; // [esp+2Ch] [ebp-94h]
    float y; // [esp+34h] [ebp-8Ch]
    float ya; // [esp+34h] [ebp-8Ch]
    float yb; // [esp+34h] [ebp-8Ch]
    float yc; // [esp+34h] [ebp-8Ch]
    float yd; // [esp+34h] [ebp-8Ch]
    float ye; // [esp+34h] [ebp-8Ch]
    float yf; // [esp+34h] [ebp-8Ch]
    float yg; // [esp+34h] [ebp-8Ch]
    float yh; // [esp+34h] [ebp-8Ch]
    float yi; // [esp+34h] [ebp-8Ch]
    char text[132]; // [esp+38h] [ebp-88h] BYREF

    if ( r_drawDebugFogParams->current.enabled )
    {
        f = &frontEndDataOut->fogSettings;
        font = R_RegisterFont("fonts/consoleFont", 1);
        _snprintf(text, 0x80u, "fog.startTime = %d", f->startTime);
        R_AddCmdDrawText(text, 128, font, 100.0, 150.0, 1.5, 2.0, 0.0, colorWhite, 0);
        _snprintf(text, 0x80u, "fog.finishTime = %d", f->finishTime);
        R_AddCmdDrawText(text, 128, font, 100.0, 25.0 + 150.0, 1.5, 2.0, 0.0, colorWhite, 0);
        y = (float)(25.0 + 150.0) + 25.0;
        _snprintf(text, 0x80u, "fog.fogStart = %.4f", f->fogStart);
        R_AddCmdDrawText(text, 128, font, 100.0, y, 1.5, 2.0, 0.0, colorWhite, 0);
        ya = y + 25.0;
        _snprintf(text, 0x80u, "fog.density = %.4f", f->density);
        R_AddCmdDrawText(text, 128, font, 100.0, ya, 1.5, 2.0, 0.0, colorWhite, 0);
        yb = ya + 25.0;
        _snprintf(text, 0x80u, "fog.maxDensity = %.4f", f->maxDensity);
        R_AddCmdDrawText(text, 128, font, 100.0, yb, 1.5, 2.0, 0.0, colorWhite, 0);
        yc = yb + 25.0;
        _snprintf(text, 0x80u, "fog.heightDensity = %.4f", f->heightDensity);
        R_AddCmdDrawText(text, 128, font, 100.0, yc, 1.5, 2.0, 0.0, colorWhite, 0);
        yd = yc + 25.0;
        _snprintf(text, 0x80u, "fog.baseHeight = %.4f", f->baseHeight);
        R_AddCmdDrawText(text, 128, font, 100.0, yd, 1.5, 2.0, 0.0, colorWhite, 0);
        ye = yd + 25.0;
        _snprintf(text, 0x80u, "fog.colorScale = %.4", f->color[3]);
        R_AddCmdDrawText(text, 128, font, 100.0, ye, 1.5, 2.0, 0.0, colorWhite, 0);
        yf = ye + 25.0;
        _snprintf(text, 0x80u, "fog.color = %.2f, %.2f, %.2f", f->color[0], f->color[1], f->color[2]);
        R_AddCmdDrawText(text, 128, font, 100.0, yf, 1.5, 2.0, 0.0, colorWhite, 0);
        yg = yf + 25.0;
        _snprintf(text, 0x80u, "fog.sunFogColor = %.2f %.2f %.2f", f->sunFogColor[0], f->sunFogColor[1], f->sunFogColor[2]);
        R_AddCmdDrawText(text, 128, font, 100.0, yg, 1.5, 2.0, 0.0, colorWhite, 0);
        yh = yg + 25.0;
        _snprintf(text, 0x80u, "fog.sunFogDir = %.2f %.2f %.2f", f->sunFogDir[0], f->sunFogDir[1], f->sunFogDir[2]);
        R_AddCmdDrawText(text, 128, font, 100.0, yh, 1.5, 2.0, 0.0, colorWhite, 0);
        yi = yh + 25.0;
        _snprintf(text, 0x80u, "fog.sunFogStartAngle = %g", f->sunFogStartAng);
        R_AddCmdDrawText(text, 128, font, 100.0, yi, 1.5, 2.0, 0.0, colorWhite, 0);
        _snprintf(text, 0x80u, "fog.sunFogEndAngle = %g", f->sunFogEndAng);
        R_AddCmdDrawText(text, 128, font, 100.0, yi + 25.0, 1.5, 2.0, 0.0, colorWhite, 0);
    }
}

void R_WaitForFXUpdateWorkerCmds()
{
    PROF_SCOPED("R_WaitForFXUpdateWorkerCmds");

    {
        PROF_SCOPED("wait fx_update");
        Sys_WaitWorkerCmdInternal(&fx_updateWorkerCmd);
    }
    {
        PROF_SCOPED("wait fx_update_remaining");
        Sys_WaitWorkerCmdInternal(&fx_update_remainingWorkerCmd);
    }
    {
        PROF_SCOPED("wait fx_update_remaining_ppu");
        Sys_WaitWorkerCmdInternal(&fx_update_remaining_ppuWorkerCmd);
    }
}

void __cdecl R_FinishDecalAndEmissiveDrawSurfs(
                GfxViewInfo *viewInfo,
                const GfxViewParms *viewParmsDraw,
                GfxBackEndData *backendData,
                bool forMissileCamView)
{
    int firstDrawSurfCount; // [esp+64h] [ebp-14h]
    int firstDrawSurfCounta; // [esp+64h] [ebp-14h]
    int firstDrawSurfCountb; // [esp+64h] [ebp-14h]
    GfxDrawSurfListInfo *emissiveInfo; // [esp+6Ch] [ebp-Ch]

    PROF_SCOPED("R_FinishDecalAndEmissiveDrawSurfs");

    {
        PROF_SCOPED("wait for fx_draw");
        Sys_WaitWorkerCmdInternal(&fx_drawWorkerCmd);
    }
    
    GlassCl_WaitGenerateVerts();

    {
        PROF_SCOPED("R_SortDrawSurfs FX_CAMERA_LIT");
        R_SortDrawSurfs(scene.drawSurfs[6], scene.drawSurfCount[6]);
    }
    {
        PROF_SCOPED("R_SortDrawSurfs CODEMESH_CAMERA_LIT");
        R_SortDrawSurfs(scene.drawSurfs[9], scene.drawSurfCount[9]);
    }
    

    {
        PROF_SCOPED("set up decalInfo");
        R_InitDrawSurfListInfo(&viewInfo->drawList[3]);
        viewInfo->drawList[3].baseTechType = gfxDrawMethod.baseTechType;
        viewInfo->drawList[3].viewInfo = viewInfo;
        viewInfo->drawList[3].viewOrigin[0] = viewParmsDraw->origin[0];
        viewInfo->drawList[3].viewOrigin[1] = viewParmsDraw->origin[1];
        viewInfo->drawList[3].viewOrigin[2] = viewParmsDraw->origin[2];
        viewInfo->drawList[3].viewOrigin[3] = viewParmsDraw->origin[3];
        viewInfo->drawList[3].cameraView = 1;
        viewInfo->drawList[3].shaderHDRmode = viewInfo->sceneComposition.alphaShaderRemap;
        R_InitDrawSurfListInfo(&viewInfo->drawList[6]);
        viewInfo->drawList[6].baseTechType = 10;
        viewInfo->drawList[6].viewInfo = viewInfo;
        viewInfo->drawList[6].viewOrigin[0] = viewParmsDraw->origin[0];
        viewInfo->drawList[6].viewOrigin[1] = viewParmsDraw->origin[1];
        viewInfo->drawList[6].viewOrigin[2] = viewParmsDraw->origin[2];
        viewInfo->drawList[6].viewOrigin[3] = viewParmsDraw->origin[3];
        viewInfo->drawList[6].cameraView = 1;
        viewInfo->drawList[6].shaderHDRmode = 0;
        firstDrawSurfCount = backendData->drawSurfCount;
        R_MergeAndEmitDrawSurfLists(3u, 7u, backendData);
        viewInfo->drawList[3].drawSurfs = &backendData->drawSurfs[firstDrawSurfCount];
        viewInfo->drawList[3].drawSurfCount = backendData->drawSurfCount - firstDrawSurfCount;
        if (!forMissileCamView)
            R_SplitDrawSurfacesPrimarySortKey(&viewInfo->drawList[3], &viewInfo->drawList[6], 60);
    }

    {
        PROF_SCOPED("sort fx surfs");
        R_SortDrawSurfs(scene.drawSurfs[13], scene.drawSurfCount[13]);
        R_SortDrawSurfs(scene.drawSurfs[16], scene.drawSurfCount[16]);
    }
    

    {
        PROF_SCOPED("set up emissiveInfo");
        emissiveInfo = &viewInfo->drawList[4];
        R_InitDrawSurfListInfo(&viewInfo->drawList[4]);
        viewInfo->drawList[4].baseTechType = R_GetEmissiveTechnique(viewInfo, gfxDrawMethod.emissiveTechType);
        viewInfo->drawList[4].viewInfo = viewInfo;
        viewInfo->drawList[4].viewOrigin[0] = viewParmsDraw->origin[0];
        viewInfo->drawList[4].viewOrigin[1] = viewParmsDraw->origin[1];
        viewInfo->drawList[4].viewOrigin[2] = viewParmsDraw->origin[2];
        viewInfo->drawList[4].viewOrigin[3] = viewParmsDraw->origin[3];
        viewInfo->drawList[4].cameraView = 1;
        viewInfo->drawList[4].shaderHDRmode = viewInfo->sceneComposition.emissiveShaderRemap;
        if (backendData->emissiveSpotLightCount)
        {
            if (backendData->emissiveSpotLightCount != 1
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    5268,
                    0,
                    "%s",
                    "backendData->emissiveSpotLightCount == 1"))
            {
                __debugbreak();
            }
            viewInfo->drawList[4].light = &backendData->emissiveSpotLight;
        }
        firstDrawSurfCounta = backendData->drawSurfCount;
        R_MergeAndEmitDrawSurfLists(0xAu, 6u, backendData);
        emissiveInfo->drawSurfs = &backendData->drawSurfs[firstDrawSurfCounta];
        viewInfo->drawList[4].drawSurfCount = backendData->drawSurfCount - firstDrawSurfCounta;
    }
    {
        PROF_SCOPED("set up reflectedInfo");
        R_InitDrawSurfListInfo(&viewInfo->drawList[5]);
        viewInfo->drawList[5].baseTechType = 9;
        viewInfo->drawList[5].viewInfo = viewInfo;
        viewInfo->drawList[5].viewOrigin[0] = viewParmsDraw->origin[0];
        viewInfo->drawList[5].viewOrigin[1] = viewParmsDraw->origin[1];
        viewInfo->drawList[5].viewOrigin[2] = viewParmsDraw->origin[2];
        viewInfo->drawList[5].viewOrigin[3] = viewParmsDraw->origin[3];
        viewInfo->drawList[5].cameraView = 1;
        viewInfo->drawList[5].shaderHDRmode = 0;
        firstDrawSurfCountb = backendData->drawSurfCount;
        R_MergeAndEmitDrawSurfLists(0x10u, 3u, backendData);
        viewInfo->drawList[5].drawSurfs = &backendData->drawSurfs[firstDrawSurfCountb];
        viewInfo->drawList[5].drawSurfCount = backendData->drawSurfCount - firstDrawSurfCountb;
    }

    if ( !viewInfo->needsFloatZ )
        viewInfo->needsFloatZ = R_DoesDrawSurfListInfoNeedFloatz(viewInfo, emissiveInfo);
}

unsigned __int8 __cdecl R_GetEmissiveTechnique(const GfxViewInfo *viewInfo, unsigned __int8 baseTech)
{
    int PrimaryLightCount; // eax

    if ( baseTech != 5 )
        return baseTech;
    if ( r_fullbright->current.enabled )
        return 4;
    if ( !frontEndDataOut->emissiveSpotLightCount )
        return 5;
    if ( !r_spotLightShadows->current.enabled )
        return 5;
    PrimaryLightCount = Com_GetPrimaryLightCount();
    if ( !Com_BitCheckAssert(frontEndDataOut->shadowableLightHasShadowMap, PrimaryLightCount, 32) )
        return 5;
    if ( Com_GetPrimaryLightCount() + 1 != frontEndDataOut->shadowableLightCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    4054,
                    0,
                    "%s",
                    "Com_GetPrimaryLightCount() + GFX_MAX_EMISSIVE_SPOT_LIGHTS == frontEndDataOut->shadowableLightCount") )
    {
        __debugbreak();
    }
    return 6;
}

char __cdecl R_DoesDrawSurfListInfoNeedFloatz(GfxViewInfo *viewInfo, GfxDrawSurfListInfo *emissiveInfo)
{
    const MaterialTechnique *technique; // [esp+8h] [ebp-Ch]
    unsigned int surfIndex; // [esp+Ch] [ebp-8h]

    if ( dx.supportsIntZ && !viewInfo->isMissileCamera )
        return 0;
    for ( surfIndex = 0; ; ++surfIndex )
    {
        if ( surfIndex == emissiveInfo->drawSurfCount )
            return 0;
        technique = Material_GetTechnique(
                                    rgp.sortedMaterials[(emissiveInfo->drawSurfs[surfIndex].packed >> 31) & 0xFFF],
                                    emissiveInfo->baseTechType);
        if ( technique )
        {
            if ( (technique->flags & 0x40) != 0 )
                break;
        }
    }
    if (!gfxRenderTargets[viewInfo->isMissileCamera ? R_RENDERTARGET_FLOAT_Z_MISSILE_CAM : R_RENDERTARGET_FLOAT_Z].surface.color)
        Com_Error(ERR_FATAL, "Renderer attempted to use technique that uses floatz buffer, but it wasn't created.\n");
    return 1;
}

void __cdecl R_SetSceneParms(const refdef_s *refdef, GfxSceneParms *sceneParms)
{
    float specularScale; // eax
    float curAmount; // edx
    bool v4; // [esp+30h] [ebp-Ch]
    unsigned int displayHeight; // [esp+34h] [ebp-8h]
    unsigned int displayWidth; // [esp+38h] [ebp-4h]

    if ( !refdef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 6832, 0, "%s", "refdef") )
        __debugbreak();
    if ( !sceneParms
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 6833, 0, "%s", "sceneParms") )
    {
        __debugbreak();
    }
    if ( refdef->blurRadius < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    6834,
                    0,
                    "%s\n\t(refdef->blurRadius) = %g",
                    "(refdef->blurRadius >= 0.0f)",
                    refdef->blurRadius) )
    {
        __debugbreak();
    }
    sceneParms->localClientNum = refdef->localClientNum;
    sceneParms->blurRadius = refdef->blurRadius;
    memcpy(&sceneParms->dof, &refdef->dof, sizeof(sceneParms->dof));
    memcpy(&sceneParms->film, &refdef->visionset, sizeof(sceneParms->film));
    specularScale = refdef->visionset.charPrimaryLightScale.specularScale;
    sceneParms->charPrimaryLightScale.diffuseScale = refdef->visionset.charPrimaryLightScale.diffuseScale;
    sceneParms->charPrimaryLightScale.specularScale = specularScale;
    memcpy(&sceneParms->bloom, &refdef->visionset.bloom, sizeof(sceneParms->bloom));
    memcpy(&sceneParms->waterSheetingFx, &refdef->waterSheetingFx, sizeof(sceneParms->waterSheetingFx));
    memcpy(&sceneParms->flameFx, &refdef->flameFx, sizeof(sceneParms->flameFx));
    memcpy(&sceneParms->reviveFx, &refdef->visionset.reviveFx, sizeof(sceneParms->reviveFx));
    sceneParms->sunVisibility = refdef->sunVisibility;
    memcpy(&sceneParms->doubleVision, &refdef->doubleVision, sizeof(sceneParms->doubleVision));
    memcpy(
        (unsigned __int8 *)&sceneParms->genericFilter,
        (unsigned __int8 *)&refdef->genericFilter,
        sizeof(sceneParms->genericFilter));
    curAmount = refdef->poisonFx.curAmount;
    sceneParms->poisonFx.curAmountTarget = refdef->poisonFx.curAmountTarget;
    sceneParms->poisonFx.curAmount = curAmount;
    memcpy(&sceneParms->electrifiedFx, &refdef->electrifiedFx, sizeof(sceneParms->electrifiedFx));
    memcpy(&sceneParms->transportedFx, &refdef->transportedFx, sizeof(sceneParms->transportedFx));
    memcpy(&sceneParms->saveScreenFx, &refdef->saveScreenFx, sizeof(sceneParms->saveScreenFx));
    displayWidth = vidConfig.displayWidth;
    displayHeight = vidConfig.displayHeight;
    v4 = refdef->width == vidConfig.displayWidth && refdef->height == vidConfig.displayHeight;
    sceneParms->isRenderingFullScreen = v4;
    sceneParms->playerTeleported = refdef->playerTeleported != 0;
    sceneParms->displayViewport.x = refdef->x;
    sceneParms->displayViewport.y = refdef->y;
    sceneParms->displayViewport.width = refdef->width;
    sceneParms->displayViewport.height = refdef->height;
    sceneParms->sceneViewport.x = (int)((double)(vidConfig.sceneWidth * refdef->x) / (double)displayWidth + 0.5);
    sceneParms->sceneViewport.y = (int)((double)(vidConfig.sceneHeight * refdef->y) / (double)displayHeight + 0.5);
    sceneParms->sceneViewport.width = vidConfig.sceneWidth * (refdef->width + refdef->x) / displayWidth
                                                                    - sceneParms->sceneViewport.x;
    sceneParms->sceneViewport.height = vidConfig.sceneHeight * (refdef->height + refdef->y) / displayHeight
                                                                     - sceneParms->sceneViewport.y;
    if ( refdef->useScissorViewport )
    {
        sceneParms->scissorViewport.x = vidConfig.sceneWidth * refdef->scissorViewport.x / displayWidth;
        sceneParms->scissorViewport.y = vidConfig.sceneHeight * refdef->scissorViewport.y / displayHeight;
        sceneParms->scissorViewport.width = vidConfig.sceneWidth
                                                                            * (refdef->scissorViewport.width + refdef->scissorViewport.x)
                                                                            / displayWidth
                                                                            - sceneParms->scissorViewport.x;
        sceneParms->scissorViewport.height = vidConfig.sceneHeight
                                                                             * (refdef->scissorViewport.height + refdef->scissorViewport.y)
                                                                             / displayHeight
                                                                             - sceneParms->scissorViewport.y;
    }
    else
    {
        sceneParms->scissorViewport = sceneParms->sceneViewport;
    }
    sceneParms->primaryLights = refdef->primaryLights;
}

void __cdecl R_CorrectLodScale(const refdef_s *refdef)
{
    R_UpdateLodParms(refdef, &rg.correctedLodParms[refdef->localClientNum]);
}

void __cdecl R_RenderMissileCam(const refdef_s *refdef, int frameTime)
{
    GfxViewParms *v2; // [esp+Ch] [ebp-14F8h]
    GfxSceneParms sceneParms; // [esp+1Ch] [ebp-14E8h] BYREF
    GfxViewParms *viewParms; // [esp+1500h] [ebp-4h]

    if ( refdef->tanHalfFovX <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    7268,
                    0,
                    "%s",
                    "refdef->tanHalfFovX > 0") )
    {
        __debugbreak();
    }
    if ( refdef->tanHalfFovY <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    7269,
                    0,
                    "%s",
                    "refdef->tanHalfFovY > 0") )
    {
        __debugbreak();
    }
    if ( !refdef->height
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 7270, 0, "%s", "refdef->height > 0") )
    {
        __debugbreak();
    }
    if ( !refdef->width
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 7271, 0, "%s", "refdef->width > 0") )
    {
        __debugbreak();
    }
    if ( refdef->localClientNum >= gfxCfg.maxClientViews
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    7272,
                    0,
                    "refdef->localClientNum doesn't index gfxCfg.maxClientViews\n\t%i not in [0, %i)",
                    refdef->localClientNum,
                    gfxCfg.maxClientViews) )
    {
        __debugbreak();
    }
    if ( rg.registered && !r_norefresh->current.enabled )
    {
        if ( r_logFile->current.integer )
            RB_LogPrint("====== R_RenderMissileCam ======\n");
        if ( !rgp.world )
            Com_Error(ERR_DROP, "R_RenderScene: NULL worldmodel");
        rg.viewOrg[0] = refdef->vieworg[0];
        rg.viewOrg[1] = refdef->vieworg[1];
        rg.viewOrg[2] = refdef->vieworg[2];
        *(_QWORD *)rg.viewDir = *(_QWORD *)&refdef->viewaxis[0][0];
        rg.viewDir[2] = refdef->viewaxis[0][2];
        viewParms = R_AllocViewParms();
        R_SetViewParmsForScene(refdef, viewParms);
        R_MissileCam_SetSceneParms(refdef, &sceneParms);
        R_CorrectLodScale(refdef);
        R_WorldLod_FrameUpdate(scene.def.floatTime, viewParms->origin, refdef->localClientNum);
        if ( scene.def.time != refdef->time
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                        7312,
                        0,
                        "%s",
                        "scene.def.time == refdef->time") )
        {
            __debugbreak();
        }
        if ( !rg.lodParms.valid
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                        7314,
                        0,
                        "%s",
                        "rg.lodParms.valid") )
        {
            __debugbreak();
        }
        if ( r_lockPvs->current.enabled )
            v2 = &lockPvsViewParms;
        else
            v2 = viewParms;
        frontEndDataOut->hideMatureContent = refdef->hideMatureContent;
        frontEndDataOut->splitscreen = refdef->splitscreen;
        frontEndDataOut->extraCamLargeFrameSize = refdef->extraCamLargeFrame;
        R_GenerateSortedDrawSurfs(&sceneParms, v2, viewParms, frameTime, refdef, 1);
        R_UI3D_SetupBackendData_UVConstants(&frontEndDataOut->viewInfo[frontEndDataOut->viewInfoIndex].rbUI3D);
    }
}

void __cdecl R_MissileCam_SetSceneParms(const refdef_s *refdef, GfxSceneParms *sceneParms)
{
    float specularScale; // edx

    if ( !refdef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 6907, 0, "%s", "refdef") )
        __debugbreak();
    if ( !sceneParms
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp", 6908, 0, "%s", "sceneParms") )
    {
        __debugbreak();
    }
    if ( refdef->blurRadius < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_scene.cpp",
                    6909,
                    0,
                    "%s\n\t(refdef->blurRadius) = %g",
                    "(refdef->blurRadius >= 0.0f)",
                    refdef->blurRadius) )
    {
        __debugbreak();
    }
    memset((unsigned __int8 *)sceneParms, 0, sizeof(GfxSceneParms));
    sceneParms->localClientNum = refdef->localClientNum;
    specularScale = refdef->visionset.charPrimaryLightScale.specularScale;
    sceneParms->charPrimaryLightScale.diffuseScale = refdef->visionset.charPrimaryLightScale.diffuseScale;
    sceneParms->charPrimaryLightScale.specularScale = specularScale;
    sceneParms->isRenderingFullScreen = 1;
    sceneParms->playerTeleported = refdef->playerTeleported != 0;
    sceneParms->displayViewport.x = 0;
    sceneParms->displayViewport.y = 0;
    sceneParms->displayViewport.width = R_ExtraCam_RenderTargetWidth();
    sceneParms->displayViewport.height = R_ExtraCam_RenderTargetHeight();
    sceneParms->sceneViewport.x = 0;
    sceneParms->sceneViewport.y = 0;
    sceneParms->sceneViewport.width = R_ExtraCam_RenderTargetWidth();
    sceneParms->sceneViewport.height = R_ExtraCam_RenderTargetHeight();
    sceneParms->scissorViewport = sceneParms->sceneViewport;
    sceneParms->primaryLights = refdef->primaryLights;
}

void __cdecl R_LinkDObjEntity(int localClientNum, unsigned int entnum, float *origin, float radius)
{
    R_FilterDObjIntoCells(localClientNum, entnum, origin, radius);
    R_LinkSphereEntityToPrimaryLights(localClientNum, entnum, origin, radius);
}

void __cdecl R_LinkBModelEntity(int localClientNum, unsigned int entnum, GfxBrushModel *bmodel)
{
    R_FilterBModelIntoCells(localClientNum, entnum, bmodel);
    R_LinkBoxEntityToPrimaryLights(localClientNum, entnum, bmodel->writable.mins, bmodel->writable.maxs);
}

void __cdecl R_UnlinkEntity(int localClientNum, unsigned int entnum)
{
    R_UnfilterEntFromCells(localClientNum, entnum);
    R_UnlinkEntityFromPrimaryLights(localClientNum, entnum);
}

void __cdecl R_LinkDynEnt(unsigned int dynEntId, DynEntityDrawType drawType, float *mins, float *maxs)
{
    R_FilterDynEntIntoCells(dynEntId, drawType, mins, maxs);
    R_LinkDynEntToPrimaryLights(dynEntId, drawType, mins, maxs);
}

void __cdecl R_UnlinkDynEnt(unsigned int dynEntId, DynEntityDrawType drawType)
{
    R_UnfilterDynEntFromCells(dynEntId, drawType);
    R_UnlinkDynEntFromPrimaryLights(dynEntId, drawType);
}

void __cdecl R_PvsLock_GetViewOrigin(float *out)
{
    *out = lockPvsViewParms.origin[0];
    out[1] = lockPvsViewParms.origin[1];
    out[2] = lockPvsViewParms.origin[2];
}

void __cdecl R_PvsLock_GetViewAxis(float (*out)[3])
{
    AxisCopy(lockPvsViewParms.axis, out);
}

const float (*colorMap[4][2])[4] =
{
  { &colorDkRed, &colorRed },
  { &colorDkYellow, &colorYellow },
  { &colorDkGreen, &colorGreen },
  { &colorDkGreen, &colorGreen }
};


void __cdecl ShodLodInfo(
                const float *origin,
                int lod,
                int numLods,
                float curDist,
                float lodDist,
                bool lodDistAutoGenerated)
{
    char *s; // [esp+14h] [ebp-4h]

    if ( r_showLodInfo->current.integer )
    {
        s = 0;
        switch ( r_showLodInfo->current.integer )
        {
            case 1:
                goto LABEL_6;
            case 2:
                s = va("%i/%i", lod, numLods);
                break;
            case 3:
                s = va("%.0f\"", curDist);
                break;
            case 4:
                if ( lodDistAutoGenerated )
LABEL_6:
                    s = va("%i/%i %.0f/%.0f\"", lod, numLods, curDist, lodDist);
                break;
            default:
                break;
        }
        if ( s )
            R_AddScaledDebugString(
                &frontEndDataOut->debugGlobals,
                rg.debugViewParms,
                origin,
                *colorMap[numLods][lodDistAutoGenerated],
                //(const float *)(&(&bad_alloc_Message_482)[2 * numLods])[lodDistAutoGenerated],
                s);
    }
}

void __cdecl R_PerMap_Init()
{
    R_PerMap_DpvsGlobInit();
    r_glob.remoteScreenLastSceneResolveTarget = 0;
}

void __cdecl R_AddXModelDebugString(const float *origin, char *string)
{
    R_AddScaledDebugString(&frontEndDataOut->debugGlobals, rg.debugViewParms, origin, colorCyan, string);
}

