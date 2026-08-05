#include "r_material.h"
#include <universal/com_memory.h>
#include "r_dvars.h"
#include <qcommon/com_profilemapload.h>
#include <qcommon/threads.h>
#include "r_init.h"
#include "rb_resource.h"
#include <database/db_file_load.h>
#include "rb_logfile.h"
#include "r_utils.h"
#include "r_material_load_obj.h"
#include <xanim/xmodel.h>
#include <cgame/cg_drawtools.h>
#include "r_image.h"
#include "rb_state.h"
#include <universal/com_files.h>
#include <algorithm>
#include <qcommon/common.h>
#include "r_rendercmds.h"

const stream_source_info_t s_streamSourceInfo[18][STREAM_SRC_COUNT] =
{
  {
    { 0u, 0u, 3u },
    { 0u, 16u, 4u },
    { 0u, 20u, 1u },
    { 0u, 28u, 5u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u }
  },
  {
    { 0u, 0u, 3u },
    { 0u, 16u, 4u },
    { 0u, 20u, 5u },
    { 0u, 24u, 5u },
    { 0u, 28u, 5u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u }
  },
  {
    { 0u, 0u, 3u },
    { 0u, 16u, 4u },
    { 0u, 20u, 3u },
    { 0u, 36u, 5u },
    { 0u, 40u, 5u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 2u, 0u, 15u }
  },
  {
    { 0u, 0u, 3u },
    { 0u, 16u, 4u },
    { 0u, 20u, 3u },
    { 0u, 36u, 5u },
    { 0u, 40u, 5u },
    { 1u, 0u, 1u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 2u, 0u, 15u }
  },
  {
    { 0u, 0u, 3u },
    { 0u, 16u, 4u },
    { 0u, 20u, 3u },
    { 0u, 36u, 5u },
    { 0u, 40u, 5u },
    { 1u, 0u, 1u },
    { 255u, 0u, 0u },
    { 1u, 8u, 8u },
    { 255u, 0u, 0u },
    { 2u, 0u, 15u }
  },
  {
    { 0u, 0u, 3u },
    { 0u, 16u, 4u },
    { 0u, 20u, 3u },
    { 0u, 36u, 5u },
    { 0u, 40u, 5u },
    { 1u, 0u, 3u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 2u, 0u, 15u }
  },
  {
    { 0u, 0u, 3u },
    { 0u, 16u, 4u },
    { 0u, 20u, 3u },
    { 0u, 36u, 5u },
    { 0u, 40u, 5u },
    { 1u, 0u, 3u },
    { 255u, 0u, 0u },
    { 1u, 16u, 8u },
    { 255u, 0u, 0u },
    { 2u, 0u, 15u }
  },
  {
    { 0u, 0u, 3u },
    { 0u, 16u, 4u },
    { 0u, 20u, 3u },
    { 0u, 36u, 5u },
    { 0u, 40u, 5u },
    { 1u, 0u, 3u },
    { 255u, 0u, 0u },
    { 1u, 16u, 8u },
    { 1u, 20u, 8u },
    { 2u, 0u, 15u }
  },
  {
    { 0u, 0u, 3u },
    { 0u, 16u, 4u },
    { 0u, 20u, 3u },
    { 0u, 36u, 5u },
    { 0u, 40u, 5u },
    { 1u, 0u, 3u },
    { 1u, 16u, 1u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 2u, 0u, 15u }
  },
  {
    { 0u, 0u, 3u },
    { 0u, 16u, 4u },
    { 0u, 20u, 3u },
    { 0u, 36u, 5u },
    { 0u, 40u, 5u },
    { 1u, 0u, 3u },
    { 1u, 16u, 1u },
    { 1u, 24u, 8u },
    { 255u, 0u, 0u },
    { 2u, 0u, 15u }
  },
  {
    { 0u, 0u, 3u },
    { 0u, 16u, 4u },
    { 0u, 20u, 3u },
    { 0u, 36u, 5u },
    { 0u, 40u, 5u },
    { 1u, 0u, 3u },
    { 1u, 16u, 1u },
    { 1u, 24u, 8u },
    { 1u, 28u, 8u },
    { 2u, 0u, 15u }
  },
  {
    { 0u, 0u, 3u },
    { 0u, 16u, 4u },
    { 0u, 20u, 3u },
    { 0u, 36u, 5u },
    { 0u, 40u, 5u },
    { 1u, 0u, 3u },
    { 1u, 16u, 3u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 2u, 0u, 15u }
  },
  {
    { 0u, 0u, 3u },
    { 0u, 16u, 4u },
    { 0u, 20u, 3u },
    { 0u, 36u, 5u },
    { 0u, 40u, 5u },
    { 1u, 0u, 3u },
    { 1u, 16u, 3u },
    { 1u, 32u, 8u },
    { 255u, 0u, 0u },
    { 2u, 0u, 15u }
  },
  {
    { 0u, 0u, 3u },
    { 0u, 16u, 4u },
    { 0u, 20u, 3u },
    { 0u, 36u, 5u },
    { 0u, 40u, 5u },
    { 1u, 0u, 3u },
    { 1u, 16u, 3u },
    { 1u, 32u, 8u },
    { 1u, 36u, 8u },
    { 2u, 0u, 15u }
  },
  {
    { 0u, 0u, 3u },
    { 255u, 0u, 0u },
    { 0u, 12u, 1u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u }
  },
  {
    { 0u, 0u, 2u },
    { 0u, 12u, 4u },
    { 0u, 16u, 5u },
    { 0u, 20u, 5u },
    { 0u, 24u, 5u },
    { 0u, 28u, 5u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u }
  },
  {
    { 0u, 0u, 5u },
    { 0u, 8u, 4u },
    { 0u, 12u, 5u },
    { 0u, 4u, 5u },
    { 255u, 0u, 0u },
    { 1u, 0u, 5u },
    { 255u, 0u, 0u },
    { 1u, 4u, 5u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u }
  },
  {
    { 0u, 0u, 10u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u },
    { 255u, 0u, 0u }
  }
};

const stream_dest_info_t s_streamDestInfo[20] =
{
  { 0u, 0u },
  { 3u, 0u },
  { 10u, 0u },
  { 10u, 1u },
  { 12u, 0u },
  { 5u, 0u },
  { 5u, 1u },
  { 5u, 2u },
  { 5u, 3u },
  { 5u, 4u },
  { 5u, 5u },
  { 5u, 6u },
  { 5u, 7u },
  { 5u, 8u },
  { 5u, 9u },
  { 5u, 10u },
  { 5u, 11u },
  { 5u, 12u },
  { 5u, 13u },
  { 1u, 0u }
};


const PerMapMaterialTable s_permapMaterials[3] =
{
  { "flamethrowerfx_color_distort_overlay_bloom", &rgp.permapAssets.flameThrowerFXMaterial },
  { "zombie_electric_shock_overlay", &rgp.permapAssets.electrifiedFXMaterial },
  { "zombie_transporter_overlay", &rgp.permapAssets.transportedFXMaterial }
};

// generated with aislop from rdata
const BuiltInMaterialTable s_builtInMaterials[122] =
{
    { "$default", &rgp.defaultMaterial },
    { "white", &rgp.whiteMaterial },
    { "$additive", &rgp.additiveMaterial },
    { "$additive_nodepth", &rgp.additiveMaterialNoDepth },
    { "$glare_blind", &rgp.glareBlindMaterial },
    { "$point", &rgp.pointMaterial },
    { "$line", &rgp.lineMaterial },
    { "$line_nodepth", &rgp.lineMaterialNoDepth },
    { "$blend", &rgp.blendMaterial },
    { "$blend_nodepth", &rgp.blendMaterialNoDepth },
    { "debug_sphere_exterior", &rgp.whiteDebugExteriorMaterial },
    { "debug_sphere_interior", &rgp.whiteDepthInteriorMaterial },
    { "light_corona", &rgp.lightCoronaMaterial },
    { "clear_alpha", &rgp.clearAlphaMaterial },
    { "clear_alpha_stencil", &rgp.clearAlphaStencilMaterial },
    { "set_alpha", &rgp.setAlphaMaterial },
    { "depthprepass", &rgp.depthPrepassMaterial },
    { "shadowclear", &rgp.shadowClearMaterial },
    { "shadowcaster", &rgp.shadowCasterMaterial },
    { "shadowoverlay", &rgp.shadowOverlayMaterial },
    { "stencilshadow", &rgp.stencilShadowMaterial },
    { "stencildisplay", &rgp.stencilDisplayMaterial },
    { "floatz_display", &rgp.floatZDisplayMaterial },

    { "$floatz_donotremove", &rgp.dummyMaterial },
    { "$processed_floatz_donotremove", &rgp.dummyMaterial },

    { "shellshock", &rgp.shellShockBlurredMaterial },
    { "shellshock_flashed", &rgp.shellShockFlashedMaterial },

    { "color_channel_mixer", &rgp.colorChannelMixerMaterial },
    { "frame_color_debug", &rgp.frameColorDebugMaterial },
    { "frame_alpha_debug", &rgp.frameAlphaDebugMaterial },

    { "feedbackblend", &rgp.feedbackBlendMaterial },
    { "feedbackfilmblend", &rgp.feedbackFilmBlendMaterial },
    { "feedbackreplace", &rgp.feedbackReplaceMaterial },

    { "cinematic", &rgp.cinematicMaterial },

    { "$ps3_aadownsample_donotremove", &rgp.dummyMaterial },

    { "dof_downsample", &rgp.dofDownsampleMaterial },
    { "dof_downsample_nv_intz", &rgp.dofDownsampleNvIntzMaterial },
    { "dof_near_coc", &rgp.dofNearCocMaterial },
    { "small_blur", &rgp.smallBlurMaterial },
    { "postfx_dof", &rgp.postFxDofMaterial },
    { "postfx_dof_nv_intz", &rgp.postFxDofNvIntzMaterial },
    { "postfx_dof_color", &rgp.postFxDofColorMaterial },
    { "postfx_dof_color_nv_intz", &rgp.postFxDofColorNvIntzMaterial },

    { "motionblurfilter1", &rgp.motionblur1Material },
    { "motionblurfilter2", &rgp.motionblur2Material },
    { "motionblurfilter3", &rgp.motionblur3Material },
    { "motionblurfilter4", &rgp.motionblur4Material },
    { "motionblurfilter5", &rgp.motionblur5Material },
    { "motionblurfilter6", &rgp.motionblur6Material },
    { "motionblurfilter7", &rgp.motionblur7Material },
    { "motionblurfilter8", &rgp.motionblur8Material },
    { "motionblurradial", &rgp.motionblurRadialMaterial },
    { "motionblurframebased", &rgp.motionblurFrameBasedMaterial },

    { "particle_blend_donotremove", &rgp.dummyMaterial },
    { "particle_zdownsample_donotremove", &rgp.dummyMaterial },

    { "watersheeting_color_distort_blur", &rgp.waterSheetingFXMaterial },
    { "revivefx", &rgp.reviveFXMaterial },

    { "postfx_color", &rgp.postFxColorMaterial },

    { "postfx_lut_0", &rgp.postFxLutMaterial[0] },
    { "postfx_lut_1", &rgp.postFxLutMaterial[1] },
    { "postfx_lut_2", &rgp.postFxLutMaterial[2] },
    { "postfx_lut_3", &rgp.postFxLutMaterial[3] },
    { "postfx_lut_4", &rgp.postFxLutMaterial[4] },
    { "postfx_lut_5", &rgp.postFxLutMaterial[5] },
    { "postfx_lut_6", &rgp.postFxLutMaterial[6] },
    { "postfx_lut_7", &rgp.postFxLutMaterial[7] },
    { "postfx_lut_8", &rgp.postFxLutMaterial[8] },
    { "postfx_lut_9", &rgp.postFxLutMaterial[9] },

    { "postfx", &rgp.postFxMaterial },

    { "glow_consistent_setup", &rgp.glowConsistentSetupMaterial },
    { "glow_apply_bloom", &rgp.glowApplyBloomMaterial },
    { "glow_godrays", &rgp.godRaysFilterMaterial },

    { "filter_symmetric_1", &rgp.symmetricFilterMaterial[0] },
    { "filter_symmetric_2", &rgp.symmetricFilterMaterial[1] },
    { "filter_symmetric_3", &rgp.symmetricFilterMaterial[2] },
    { "filter_symmetric_4", &rgp.symmetricFilterMaterial[3] },
    { "filter_symmetric_5", &rgp.symmetricFilterMaterial[4] },
    { "filter_symmetric_6", &rgp.symmetricFilterMaterial[5] },
    { "filter_symmetric_7", &rgp.symmetricFilterMaterial[6] },
    { "filter_symmetric_8", &rgp.symmetricFilterMaterial[7] },

    { "pixel_cost_add_depth_always", &rgp.pixelCostAddDepthAlwaysMaterial },
    { "pixel_cost_add_depth_disable", &rgp.pixelCostAddDepthDisableMaterial },
    { "pixel_cost_add_depth_equal", &rgp.pixelCostAddDepthEqualMaterial },
    { "pixel_cost_add_depth_less", &rgp.pixelCostAddDepthLessMaterial },
    { "pixel_cost_add_depthwrite", &rgp.pixelCostAddDepthWriteMaterial },
    { "pixel_cost_add_nodepthwrite", &rgp.pixelCostAddNoDepthWriteMaterial },
    { "pixel_cost_color_code", &rgp.pixelCostColorCodeMaterial },

    { "poison", &rgp.poisonFXMaterial },
    { "rope", &rgp.ropeMaterial },

    { "redact1", &rgp.redactMaterial1 },
    { "redact2", &rgp.redactMaterial2 },
    { "redact3", &rgp.redactMaterial3 },

    { "resample_final", &rgp.resampleFinal },
    { "resample_shift", &rgp.resampleShift },

    { "bloom_downsample_ldr", &rgp.bloomDownsampleLDR },
    { "bloom_downsample_hdr", &rgp.bloomDownsampleHDR },
    { "bloom_downsample_convolution", &rgp.bloomDownsampleConvolution },
    { "bloom_remap", &rgp.bloomRemap },
    { "bloom_expansion", &rgp.bloomExpansion },
    { "bloom_remap_streak", &rgp.bloomRemapStreak },
    { "bloom_streak_x", &rgp.bloomStreakX },
    { "bloom_streak_y", &rgp.bloomStreakY },
    { "bloom_apply_streak", &rgp.bloomApplyStreak },
    { "bloom_blur_x", &rgp.bloomBlurX },
    { "bloom_blur_y", &rgp.bloomBlurY },
    { "bloom_mip3_blur", &rgp.bloomMip3Blur },
    { "bloom_apply_hdr", &rgp.bloomApplyHDR },
    { "bloom_apply_ldr", &rgp.bloomApplyLDR },
    { "bloom_smooth_a", &rgp.bloomSmoothA },
    { "bloom_smooth_b", &rgp.bloomSmoothB },

    { "generic_filter_superflare_apply", &rgp.superFlareApply },
    { "generic_filter_superflare_occluder", &rgp.superFlareOccluder },
    { "generic_filter_superflare_occluder_debug", &rgp.superFlareOccluderDebug },
    { "generic_filter_superflare_occluder_noz", &rgp.superFlareOccluderNoZ },
    { "generic_filter_superflare_occluder_noz_debug", &rgp.superFlareOccluderNoZDebug },

    { "create_lut2d", &rgp.createLut2d },
    { "create_lut2dv", &rgp.createLut2dv },
    { "apply_lut2d", &rgp.applyLut2d },
    { "apply_lut3d", &rgp.applyLut3d },

    { "$ps3_reload_zcull_donotremove", &rgp.dummyMaterial },

    { "composite_result", &rgp.compositeResult },
    { "infrared_white", &rgp.infraredWhite },
};


struct $5803DA4EE1143C7EE9D14DA3FE11ED2E // sizeof=0x2004
{                                       // XREF: .data:materialGlobals/r
    int techniqueSetCount;
    MaterialTechniqueSet *techniqueSetHashTable[2048];
};
$5803DA4EE1143C7EE9D14DA3FE11ED2E materialGlobals;

bool g_alwaysUseDefaultMaterial;
bool skipShaderCreationHack;
bool g_generateOverrideTechniques = true;
HRESULT g_hr;
int mem;
Material *Current_Edit_Material;
MaterialConstantDef *MaterialParameterConstantEntry;
dvar_s *MaterialParameterDvar;
MaterialConstantDef MaterialParameterConstantEntry_save;

char desc[256];

void __cdecl Material_FreeTechniqueSet(XAssetHeader header)
{
    Material_ReleaseTechniqueSetResources(header.techniqueSet);
}

unsigned __int8 *__cdecl Material_Alloc(unsigned int size)
{
    return Hunk_Alloc(size, "Material_Alloc", 23);
}

void __cdecl Load_CreateMaterialPixelShader(GfxPixelShaderLoadDef *loadDef, MaterialPixelShader *mtlShader)
{
    HRESULT hr; // [esp+0h] [ebp-8h]

    if ( loadDef != &mtlShader->prog.loadDef
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp",
                    763,
                    0,
                    "%s",
                    "loadDef == &mtlShader->prog.loadDef") )
    {
        __debugbreak();
    }
    if ( r_loadForRenderer && r_loadForRenderer->current.enabled )
    {
        ProfLoad_Begin("Create pixel shader");
        if ( Sys_IsRenderThread() )
        {
            hr = dx.device->CreatePixelShader((DWORD*)loadDef->program, (IDirect3DPixelShader9 **)&mtlShader->prog);
            if ( hr < 0 )
            {
                g_hr = hr;
                mem = dx.device->GetAvailableTextureMem();
            }
        }
        else
        {
            RB_Resource_CreatePixelShader((unsigned int*)loadDef->program, &mtlShader->prog.ps);
            RB_Resource_Flush();
        }
        DB_LoadedExternalData(0x2710u);
        ProfLoad_End();
    }
    else
    {
        mtlShader->prog.ps = 0;
    }
}

void __cdecl Load_CreateMaterialVertexShader(GfxVertexShaderLoadDef *loadDef, MaterialVertexShader *mtlShader)
{
    const char *v2; // eax
    int hr; // [esp+4h] [ebp-8h]

    if ( loadDef != &mtlShader->prog.loadDef
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp",
                    808,
                    0,
                    "%s",
                    "loadDef == &mtlShader->prog.loadDef") )
    {
        __debugbreak();
    }
    if ( r_loadForRenderer && r_loadForRenderer->current.enabled && !skipShaderCreationHack )
    {
        ProfLoad_Begin("Create vertex shader");
        if ( Sys_IsRenderThread() )
        {
            if ( r_logFile && r_logFile->current.integer )
                RB_LogPrint("dx.device->CreateVertexShader( loadDef->program, &mtlShader->prog.vs )\n");
            hr = dx.device->CreateVertexShader(loadDef->program, (IDirect3DVertexShader9 **)&mtlShader->prog);
            if ( hr < 0 )
            {
                ++g_disableRendering;
                v2 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp (%i) dx.device->CreateVertexShader( loadDef->progra"
                    "m, &mtlShader->prog.vs ) failed: %s\n",
                    829,
                    v2);
            }
        }
        else
        {
            RB_Resource_CreateVertexShader((unsigned int*)loadDef->program, &mtlShader->prog.vs);
            RB_Resource_Flush();
        }
        DB_LoadedExternalData(0x2710u);
        ProfLoad_End();
    }
    else
    {
        mtlShader->prog.vs = 0;
    }
}

void __cdecl Load_BuildVertexDecl(MaterialVertexDeclaration **mtlVertDecl)
{
    IDirect3DVertexDeclaration9 *v1; // eax
    MaterialStreamRouting data[16]; // [esp+8h] [ebp-28h] BYREF
    int vertDeclType; // [esp+2Ch] [ebp-4h]

    memcpy(data, &(*mtlVertDecl)->routing, sizeof(data));
    for ( vertDeclType = 0; vertDeclType < 18; ++vertDeclType )
    {
        if ( r_loadForRenderer && r_loadForRenderer->current.enabled )
        {
            v1 = Material_BuildVertexDecl(data, (*mtlVertDecl)->streamCount, s_streamSourceInfo[vertDeclType]);
            (*mtlVertDecl)->routing.decl[vertDeclType] = v1;
        }
        else
        {
            (*mtlVertDecl)->routing.decl[vertDeclType] = 0;
        }
    }
    (*mtlVertDecl)->isLoaded = 1;
}

IDirect3DVertexDeclaration9 *__cdecl Material_BuildVertexDecl(
                const MaterialStreamRouting *routingData,
                int streamCount,
                const stream_source_info_t *sourceTable)
{
    int v4; // edx
    int v5; // edx
    const char *v6; // eax
    int hr; // [esp+4h] [ebp-828h]
    int elemIndexInsert; // [esp+Ch] [ebp-820h]
    const stream_source_info_t *sourceInfo; // [esp+10h] [ebp-81Ch]
    _D3DVERTEXELEMENT9 declEnd; // [esp+14h] [ebp-818h]
    _D3DVERTEXELEMENT9 elemTable[256]; // [esp+1Ch] [ebp-810h] BYREF
    IDirect3DVertexDeclaration9 *decl; // [esp+820h] [ebp-Ch] BYREF
    const stream_dest_info_t *destInfo; // [esp+824h] [ebp-8h]
    int elemIndex; // [esp+828h] [ebp-4h]

    decl = NULL;

    declEnd.Stream = 255;
    declEnd.Offset = 0;
    declEnd.Type = 17;
    declEnd.Method = 0;
    declEnd.Usage = 0;
    declEnd.UsageIndex = 0;

    elemIndex = 0;

    while ( streamCount )
    {
        bcassert(routingData->source, STREAM_SRC_COUNT);

        sourceInfo = &sourceTable[routingData->source];

        if ( sourceInfo->Stream == 255 )
            return 0;
        
        destInfo = &s_streamDestInfo[routingData->dest];

        bcassert(routingData->dest, ARRAY_COUNT(s_streamDestInfo)); // LWSS ADD
        const stream_dest_info_t *destInfo = &s_streamDestInfo[routingData->dest];

        for (elemIndexInsert = elemIndex;
            elemIndexInsert > 0 && elemTable[elemIndexInsert - 1].Stream > sourceInfo->Stream;
            --elemIndexInsert)
        {
            elemTable[elemIndexInsert] = elemTable[elemIndexInsert - 1];
        }

        elemTable[elemIndexInsert].Stream = sourceInfo->Stream;
        elemTable[elemIndexInsert].Offset = sourceInfo->Offset;
        elemTable[elemIndexInsert].Type = sourceInfo->Type;
        elemTable[elemIndexInsert].Method = 0;
        elemTable[elemIndexInsert].Usage = destInfo->Usage;
        elemTable[elemIndexInsert].UsageIndex = destInfo->UsageIndex;

        elemIndex++;
        routingData++;
        streamCount--;
    }

    elemTable[elemIndex] = declEnd;

    if ( dx.device )
    {
        if ( Sys_IsRenderThread() )
        {
            if ( r_logFile && r_logFile->current.integer )
                RB_LogPrint("dx.device->CreateVertexDeclaration( elemTable, &decl )\n");
            hr = dx.device->CreateVertexDeclaration(elemTable, &decl);
            if ( hr < 0 )
            {
                ++g_disableRendering;
                v6 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp (%i) dx.device->CreateVertexDeclaration( elemTable,"
                    " &decl ) failed: %s\n",
                    952,
                    v6);
            }
        }
        else
        {
            RB_Resource_CreateVertexDeclaration(elemTable, &decl);
            RB_Resource_Flush();
        }

        iassert(decl);
    }
    return decl;
}

MaterialTechniqueSet *__cdecl Material_FindTechniqueSet(const char *name, MtlTechSetNotFoundBehavior notFoundBehavior)
{
    if ( useFastFile->current.enabled )
        return Material_FindTechniqueSet_FastFile(name, notFoundBehavior);
    else
        return Material_FindTechniqueSet_LoadObj(name, notFoundBehavior);
}

MaterialTechniqueSet *__cdecl Material_FindTechniqueSet_LoadObj(
                const char *name,
                MtlTechSetNotFoundBehavior notFoundBehavior)
{
    MaterialTechniqueSet *defaultTechSet; // [esp+0h] [ebp-8h]
    int hashIndex; // [esp+4h] [ebp-4h] BYREF

    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 1068, 0, "%s", "name") )
        __debugbreak();
    if ( MaterialTechniqueSet_FindHashLocation(name, &hashIndex) )
        return materialGlobals.techniqueSetHashTable[hashIndex];
    if ( notFoundBehavior == MTL_TECHSET_NOT_FOUND_RETURN_NULL )
        return 0;
    defaultTechSet = Material_RegisterTechniqueSet((char*)"default");
    if ( !defaultTechSet
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 1076, 0, "%s", "defaultTechSet") )
    {
        __debugbreak();
    }
    return defaultTechSet;
}

char __cdecl MaterialTechniqueSet_FindHashLocation(const char *name, int *foundHashIndex)
{
    int hashIndex; // [esp+0h] [ebp-4h]

    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 1045, 0, "%s", "name") )
        __debugbreak();
    for ( hashIndex = R_HashAssetName(name) & 0x7FF;
                materialGlobals.techniqueSetHashTable[hashIndex];
                hashIndex = ((_WORD)hashIndex + 1) & 0x7FF )
    {
        if ( !I_stricmp(materialGlobals.techniqueSetHashTable[hashIndex]->name, name) )
        {
            *foundHashIndex = hashIndex;
            return 1;
        }
    }
    *foundHashIndex = hashIndex;
    return 0;
}

MaterialTechniqueSet *__cdecl Material_FindTechniqueSet_FastFile(
                const char *name,
                MtlTechSetNotFoundBehavior notFoundBehavior)
{
    XAssetHeader header; // [esp+4h] [ebp-4h]

    header.xmodelPieces = DB_FindXAssetHeader(ASSET_TYPE_TECHNIQUE_SET, (char*)name, 1, -1).xmodelPieces;
    if ( notFoundBehavior || !DB_IsXAssetDefault(ASSET_TYPE_TECHNIQUE_SET, name) )
        return (MaterialTechniqueSet *)header.xmodelPieces;
    else
        return 0;
}

void __cdecl Material_SetAlwaysUseDefaultMaterial(bool alwaysUseDefaultMaterial)
{
    g_alwaysUseDefaultMaterial = alwaysUseDefaultMaterial;
}

void __cdecl Material_PreventOverrideTechniqueGeneration()
{
    g_generateOverrideTechniques = 0;
}

MaterialTechniqueSet *__cdecl Material_RegisterTechniqueSet(char *name)
{
    MaterialTechniqueSet *techniqueSet; // [esp+0h] [ebp-4h]
    MaterialTechniqueSet *techniqueSeta; // [esp+0h] [ebp-4h]

    techniqueSet = Material_FindTechniqueSet(name, MTL_TECHSET_NOT_FOUND_RETURN_NULL);
    if ( techniqueSet )
        return techniqueSet;
    ProfLoad_Begin("Load technique set");
    techniqueSeta = Material_LoadTechniqueSet(name);
    ProfLoad_End();
    if ( !techniqueSeta )
        return 0;
    Material_SetTechniqueSet(name, techniqueSeta);
    return techniqueSeta;
}

void __cdecl Material_SetTechniqueSet(const char *name, MaterialTechniqueSet *techniqueSet)
{
    int hashIndex; // [esp+0h] [ebp-4h] BYREF

    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 1106, 0, "%s", "name") )
        __debugbreak();
    MaterialTechniqueSet_FindHashLocation(name, &hashIndex);
    if ( !techniqueSet->name
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp",
                    1108,
                    0,
                    "%s",
                    "techniqueSet->name") )
    {
        __debugbreak();
    }
    materialGlobals.techniqueSetHashTable[hashIndex] = techniqueSet;
}

void __cdecl Material_ReleaseTechniqueSetResources(MaterialTechniqueSet *techniqueSet)
{
    MaterialTechnique *technique; // [esp+10h] [ebp-Ch]
    int techType; // [esp+14h] [ebp-8h]
    int passIndex; // [esp+18h] [ebp-4h]

    for ( techType = 0; techType < 130; ++techType )
    {
        technique = techniqueSet->techniques[techType];
        if ( technique )
        {
            for ( passIndex = 0; passIndex < technique->passCount; ++passIndex )
                Material_ReleasePassResources(&technique->passArray[passIndex]);
        }
    }
}

void __cdecl Material_ReleasePassResources(MaterialPass *pass)
{
    IDirect3DSurface9 *v1; // [esp+0h] [ebp-10h]
    IDirect3DSurface9 *var; // [esp+4h] [ebp-Ch]
    IDirect3DPixelShader9 *varCopy; // [esp+8h] [ebp-8h]
    int declIndex; // [esp+Ch] [ebp-4h]
    int declIndexa; // [esp+Ch] [ebp-4h]

    if ( r_loadForRenderer && r_loadForRenderer->current.enabled )
    {
        if ( Sys_IsRenderThread() )
        {
            if ( !pass->pixelShader
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp",
                            1201,
                            0,
                            "%s",
                            "pass->pixelShader") )
            {
                __debugbreak();
            }
            if ( pass->pixelShader->prog.ps )
            {
                do
                {
                    if ( r_logFile && r_logFile->current.integer )
                        RB_LogPrint("pass->pixelShader->prog.ps->Release()\n");
                    varCopy = pass->pixelShader->prog.ps;
                    pass->pixelShader->prog.ps = 0;
                    R_ReleaseAndSetNULL<IDirect3DIndexBuffer9>(
                        (IDirect3DSurface9 *)varCopy,
                        "pass->pixelShader->prog.ps",
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp",
                        1203);
                }
                while ( alwaysfails );
            }
            if ( !pass->vertexShader
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp",
                            1205,
                            0,
                            "%s",
                            "pass->vertexShader") )
            {
                __debugbreak();
            }
            if ( pass->vertexShader->prog.vs )
            {
                do
                {
                    if ( r_logFile && r_logFile->current.integer )
                        RB_LogPrint("pass->vertexShader->prog.vs->Release()\n");
                    var = (IDirect3DSurface9 *)pass->vertexShader->prog.vs;
                    pass->vertexShader->prog.vs = 0;
                    R_ReleaseAndSetNULL<IDirect3DIndexBuffer9>(
                        var,
                        "pass->vertexShader->prog.vs",
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp",
                        1207);
                }
                while ( alwaysfails );
            }
            if ( !pass->vertexDecl
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp",
                            1209,
                            0,
                            "%s",
                            "pass->vertexDecl") )
            {
                __debugbreak();
            }
            if ( pass->vertexDecl->isLoaded )
            {
                pass->vertexDecl->isLoaded = 0;
                for ( declIndexa = 0; declIndexa < 18; ++declIndexa )
                {
                    if ( pass->vertexDecl->routing.decl[declIndexa] )
                    {
                        do
                        {
                            if ( r_logFile && r_logFile->current.integer )
                                RB_LogPrint("pass->vertexDecl->routing.decl[declIndex]->Release()\n");
                            v1 = (IDirect3DSurface9 *)pass->vertexDecl->routing.decl[declIndexa];
                            pass->vertexDecl->routing.decl[declIndexa] = 0;
                            R_ReleaseAndSetNULL<IDirect3DIndexBuffer9>(
                                v1,
                                "pass->vertexDecl->routing.decl[declIndex]",
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp",
                                1216);
                        }
                        while ( alwaysfails );
                    }
                }
            }
        }
        else
        {
            if ( !pass->pixelShader
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp",
                            1169,
                            0,
                            "%s",
                            "pass->pixelShader") )
            {
                __debugbreak();
            }
            if ( pass->pixelShader->prog.ps )
            {
                RB_Resource_Release(pass->pixelShader->prog.ps);
                pass->pixelShader->prog.ps = 0;
            }
            if ( !pass->vertexShader
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp",
                            1176,
                            0,
                            "%s",
                            "pass->vertexShader") )
            {
                __debugbreak();
            }
            if ( pass->vertexShader->prog.vs )
            {
                RB_Resource_Release(pass->vertexShader->prog.vs);
                pass->vertexShader->prog.vs = 0;
            }
            if ( !pass->vertexDecl
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp",
                            1183,
                            0,
                            "%s",
                            "pass->vertexDecl") )
            {
                __debugbreak();
            }
            if ( pass->vertexDecl->isLoaded )
            {
                pass->vertexDecl->isLoaded = 0;
                for ( declIndex = 0; declIndex < 18; ++declIndex )
                {
                    if ( pass->vertexDecl->routing.decl[declIndex] )
                    {
                        RB_Resource_Release(pass->vertexDecl->routing.decl[declIndex]);
                        pass->vertexDecl->routing.decl[declIndex] = 0;
                    }
                }
            }
        }
    }
}

void __cdecl Material_FreeAllTechniqueSets()
{
    DB_EnumXAssets(ASSET_TYPE_TECHNIQUE_SET, (void (__cdecl *)(XAssetHeader, void *))Material_FreeTechniqueSet, 0, 1);
    if ( !useFastFile->current.enabled )
        memset(materialGlobals.techniqueSetHashTable, 0, sizeof(materialGlobals.techniqueSetHashTable));
}

void __cdecl Material_DirtySort()
{
    rgp.needSortMaterials = 1;
}

Material *__cdecl Material_Duplicate(Material *mtlCopy, char *name)
{
    unsigned int v3; // [esp+8h] [ebp-30h]
    const char *nameBackup; // [esp+18h] [ebp-20h]
    Material *mtlNewa; // [esp+1Ch] [ebp-1Ch]
    unsigned __int8 *mtlNew; // [esp+1Ch] [ebp-1Ch]
    int constantTableSize; // [esp+24h] [ebp-14h]
    unsigned __int16 hashIndex[3]; // [esp+28h] [ebp-10h] BYREF
    bool exists; // [esp+2Fh] [ebp-9h] BYREF
    unsigned int textureTableSize; // [esp+30h] [ebp-8h]
    unsigned int stateBitsTableSize; // [esp+34h] [ebp-4h]

    if ( !mtlCopy
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 1397, 0, "%s", "mtlCopy") )
    {
        __debugbreak();
    }
    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 1398, 0, "%s", "name") )
        __debugbreak();
    Material_GetHashIndex(name, hashIndex, &exists);
    if ( exists )
    {
        mtlNewa = rg.materialHashTable[hashIndex[0]];
        nameBackup = mtlNewa->info.name;
        memcpy(mtlNewa, mtlCopy, sizeof(Material));
        mtlNewa->info.name = nameBackup;
        rgp.needSortMaterials = 1;
        return mtlNewa;
    }
    else
    {
        v3 = strlen(name);
        mtlNew = Material_Alloc(v3 + 193);
        memcpy(mtlNew, mtlCopy, 0xC0u);
        *(unsigned int *)mtlNew = (unsigned int)(mtlNew + 192);
        memcpy(*(unsigned __int8 **)mtlNew, (unsigned __int8 *)name, v3 + 1);
        stateBitsTableSize = 8 * mtlCopy->stateBitsCount;
        *((unsigned int *)mtlNew + 47) = (unsigned int)Material_Alloc(stateBitsTableSize);
        memcpy(*((unsigned __int8 **)mtlNew + 47), (unsigned __int8 *)mtlCopy->stateBitsTable, stateBitsTableSize);
        if ( mtlCopy->textureTable )
        {
            textureTableSize = 16 * mtlCopy->textureCount;
            *((unsigned int *)mtlNew + 45) = (unsigned int)Material_Alloc(textureTableSize);
            memcpy(*((unsigned __int8 **)mtlNew + 45), (unsigned __int8 *)mtlCopy->textureTable, textureTableSize);
        }
        if ( mtlCopy->localConstantTable )
        {
            constantTableSize = 32 * mtlCopy->constantCount;
            *((unsigned int *)mtlNew + 46) = (unsigned int)Material_Alloc(constantTableSize);
            memcpy(*((unsigned __int8 **)mtlNew + 46), (unsigned __int8 *)mtlCopy->localConstantTable, constantTableSize);
        }
        Material_Add((Material *)mtlNew, hashIndex[0]);
        return (Material *)mtlNew;
    }
}

void __cdecl Material_Add(Material *material, unsigned __int16 hashIndex)
{
    unsigned __int64 v2; // rax
    unsigned int v3; // ecx

    if ( !material
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 1365, 0, "%s", "material") )
    {
        __debugbreak();
    }
    rgp.needSortMaterials = 1;
    if ( rg.materialHashTable[hashIndex]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp",
                    1369,
                    1,
                    "%s",
                    "rg.materialHashTable[hashIndex] == NULL") )
    {
        __debugbreak();
    }
    material->info.hashIndex = hashIndex;
    v2 = (unsigned __int64)(rgp.materialCount & 0xFFF) << 31;
    v3 = HIDWORD(v2) | HIDWORD(material->info.drawSurf.packed) & 0xFFFFF800;
    *(unsigned int *)&material->info.drawSurf.fields = v2 | *(unsigned int *)&material->info.drawSurf.fields & 0x7FFFFFFF;
    HIDWORD(material->info.drawSurf.packed) = v3;
    rgp.sortedMaterials[rgp.materialCount] = material;
    rg.materialHashTable[hashIndex] = material;
    if ( ++rgp.materialCount == 4096 )
        Com_Error(ERR_FATAL, "Too many unique materials (%i or more)\n", 4096);
}

bool __cdecl Material_IsDefault(const Material *material)
{
    if ( !material
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 1462, 0, "%s", "material") )
    {
        __debugbreak();
    }
    if ( !rgp.defaultMaterial
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp",
                    1463,
                    0,
                    "%s",
                    "rgp.defaultMaterial") )
    {
        __debugbreak();
    }
    if ( material->textureTable != rgp.defaultMaterial->textureTable )
        return 0;
    if ( material->textureTable->nameHash == rgp.defaultMaterial->textureTable->nameHash )
        return 1;
    if ( material->localConstantTable == rgp.defaultMaterial->localConstantTable )
        return material->localTechniqueSet == rgp.defaultMaterial->localTechniqueSet;
    return 0;
}

void __cdecl Material_GetHashIndex(const char *name, unsigned __int16 *hashIndex, bool *exists)
{
    char *v3; // eax
    int v4; // [esp+4h] [ebp-1Ch]
    unsigned __int16 beginHashIndex; // [esp+1Ch] [ebp-4h]

    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 1483, 0, "%s", "name") )
        __debugbreak();
    if ( !hashIndex
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 1484, 0, "%s", "hashIndex") )
    {
        __debugbreak();
    }
    if ( !exists
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 1485, 0, "%s", "exists") )
    {
        __debugbreak();
    }
    beginHashIndex = R_HashAssetName(name) % 0xFFF;
    *hashIndex = beginHashIndex;
    do
    {
        if ( !rg.materialHashTable[*hashIndex] )
            break;
        v3 = (char*)strchr(rg.materialHashTable[*hashIndex]->info.name, 0x28u);
        if ( v3 )
            *v3 = 0;
        v4 = strcmp(rg.materialHashTable[*hashIndex]->info.name, name);
        if ( v3 )
            *v3 = 40;
        if ( !v4 )
        {
            *exists = 1;
            return;
        }
        *hashIndex = ((unsigned int)*hashIndex + 1) % 0xFFF;
    }
    while ( *hashIndex != beginHashIndex );
    *exists = 0;
}

Material *__cdecl Material_Register(char *name, int imageTrack)
{
    if ( useFastFile->current.enabled )
        return (Material *)((int (__cdecl *)(char *, int))Material_Register_FastFile)(name, imageTrack);
    else
        return Material_Register_LoadObj(name, imageTrack);
}

Material *__cdecl Material_Register_LoadObj(char *name, int imageTrack)
{
    Material *material; // [esp+0h] [ebp-Ch]
    bool exists; // [esp+7h] [ebp-5h] BYREF
    unsigned __int16 hashIndex; // [esp+8h] [ebp-4h] BYREF

    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 1525, 0, "%s", "name") )
        __debugbreak();
    if ( !*name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 1526, 0, "%s", "name[0]") )
    {
        __debugbreak();
    }
    Material_GetHashIndex(name, &hashIndex, &exists);
    if ( exists )
        return rg.materialHashTable[hashIndex];
    ProfLoad_Begin("Load material");
    material = Material_Load(name, imageTrack);
    ProfLoad_End();
    if ( !material )
        return Material_MakeDefault(name);
    Material_GetHashIndex(name, &hashIndex, &exists);
    if ( exists
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 1539, 1, "%s", "!exists") )
    {
        __debugbreak();
    }
    Material_Add(material, hashIndex);
    return material;
}

Material *__cdecl Material_MakeDefault(char *name)
{
    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 1445, 0, "%s", "name") )
        __debugbreak();
    if ( !rgp.defaultMaterial )
    {
        if ( strcmp(name, "$default")
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp",
                        1449,
                        0,
                        "%s",
                        "!strcmp( name, MATERIAL_DEFAULT_NAME )") )
        {
            __debugbreak();
        }
        Com_Error(ERR_FATAL, "couldn't load material '$default'");
    }
    Com_PrintWarning(8, "WARNING: Could not find material '%s'\n", name);
    return Material_Duplicate(rgp.defaultMaterial, name);
}

Material *__cdecl Material_Register_FastFile(const char *name)
{
    return DB_FindXAssetHeader(ASSET_TYPE_MATERIAL, (char*)name, 1, -1).material;
}

Material *__cdecl Material_RegisterHandle(char *name, int imageTrack)
{
    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 1631, 0, "%s", "name") )
        __debugbreak();
    if ( !rgp.defaultMaterial
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp",
                    1632,
                    0,
                    "%s",
                    "rgp.defaultMaterial") )
    {
        __debugbreak();
    }
    if ( g_alwaysUseDefaultMaterial && rgp.defaultMaterial )
        return rgp.defaultMaterial;
    if ( *name )
        return Material_Register(name, imageTrack);
    else
        return rgp.defaultMaterial;
}

void __cdecl R_MaterialList_f()
{
    const char *fmt; // [esp+8h] [ebp-8150h]
    unsigned int i; // [esp+138h] [ebp-8020h]
    const char **p_name; // [esp+13Ch] [ebp-801Ch]
    int v3; // [esp+140h] [ebp-8018h]
    MaterialMemory *v4; // [esp+144h] [ebp-8014h]
    unsigned int inData; // [esp+148h] [ebp-8010h] BYREF
    MaterialMemory v6[4097]; // [esp+14Ch] [ebp-800Ch] BYREF
    float v7; // [esp+8154h] [ebp-4h]

    v3 = 0;
    Com_Printf(8, "-----------------------\n");
    inData = 0;
    DB_EnumXAssets(ASSET_TYPE_MATERIAL, (void (__cdecl *)(XAssetHeader, void *))R_GetMaterialList, &inData, 0);

    //std::_Sort<RagdollSortStruct *,int,bool (__cdecl *)(RagdollSortStruct const &,RagdollSortStruct const &)>(
    //    v6,
    //    &v6[inData],
    //    (int)(8 * inData) >> 3,
    //    (bool (__cdecl *)(const MaterialMemory *, const MaterialMemory *))R_MaterialCompare);

    std::sort(v6, v6 + inData, R_MaterialCompare);

    Com_Printf(8, "geo KB     name\n");
    for ( i = 0; i < inData; ++i )
    {
        v4 = &v6[i];
        p_name = &v4->material->info.name;
        if ( !v4->material
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 1733, 0, "%s", "material") )
        {
            __debugbreak();
        }
        v3 += v4->memory;
        v7 = (float)v4->memory / 1024.0;
        if ( v7 >= 10.0 )
            fmt = "%6.0f";
        else
            fmt = "%6.1f";
        Com_Printf(8, fmt, v7);
        Com_Printf(8, "     %s\n", *p_name);
    }
    Com_Printf(8, "-----------------------\n");
    Com_Printf(8, "current total    %5.1f MB\n", (float)((float)v3 / 1048576.0));
    Com_Printf(8, "%i total geometry materials\n", inData);
    Com_Printf(8, "Related commands: meminfo, imagelist, gfx_world, gfx_model, cg_drawfps, com_statmon, tempmeminfo\n");
}

void __cdecl R_GetMaterialList(XAssetHeader header, char *data)
{
    int memory; // [esp+0h] [ebp-Ch]
    XAssetHeader *materialMemory; // [esp+4h] [ebp-8h]

    memory = R_GetMaterialMemory(header.material);
    if ( memory )
    {
        if ( *(unsigned int *)data >= 0x1000u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp",
                        1697,
                        0,
                        "%s",
                        "materialList->count < ARRAY_COUNT( materialList->sorted )") )
        {
            __debugbreak();
        }
        materialMemory = (XAssetHeader *)&data[8 * *(unsigned int *)data + 4];
        materialMemory->xmodelPieces = header.xmodelPieces;
        materialMemory[1].xmodelPieces = (XModelPieces *)memory;
        ++*(unsigned int *)data;
    }
}

int __cdecl R_GetMaterialMemory(Material *material)
{
    int i; // [esp+0h] [ebp-4h]

    if ( !rgp.world )
        return 0;
    for ( i = 0; i < rgp.world->materialMemoryCount; ++i )
    {
        if ( material == rgp.world->materialMemory[i].material )
            return rgp.world->materialMemory[i].memory;
    }
    return 0;
}

bool __cdecl R_MaterialCompare(const MaterialMemory &material0, const MaterialMemory &material1)
{
    return material0.memory < material1.memory;
}

void __cdecl R_MaterialEdit_f()
{
    const char *v0; // eax
    char name[16]; // [esp+50h] [ebp-24h] BYREF
    MaterialConstantDef *ct; // [esp+64h] [ebp-10h]
    const char *search; // [esp+68h] [ebp-Ch]
    int i; // [esp+6Ch] [ebp-8h]
    int count; // [esp+70h] [ebp-4h]

    search = "";
    if ( Cmd_Argc() >= 2 )
        search = Cmd_Argv(1);
    Current_Edit_Material = 0;
    count = 0;
    for ( i = 0; i < rgp.materialCount; ++i )
    {
        if ( *rgp.sortedMaterials[i]->info.name != 42 )
        {
            v0 = strstr(rgp.sortedMaterials[i]->info.name, search);
            if ( v0 )
            {
                Com_Printf(8, "%s\n", rgp.sortedMaterials[i]->info.name);
                if ( !Current_Edit_Material
                    || strlen(rgp.sortedMaterials[i]->info.name) < strlen(Current_Edit_Material->info.name) )
                {
                    Current_Edit_Material = rgp.sortedMaterials[i];
                }
                ++count;
            }
        }
    }
    if ( count )
    {
        if ( count != 1 )
            Com_Printf(8, "Selecting %s.\n", Current_Edit_Material->info.name);
        for ( i = 0; i < Current_Edit_Material->constantCount; ++i )
        {
            ct = &Current_Edit_Material->localConstantTable[i];
            strncpy(name, ct->name, 0xCu);
            name[12] = 0;
            Com_Printf(
                8,
                "    %d: %12s = %g %g %g %g\n",
                i + 1,
                name,
                ct->literal[0],
                ct->literal[1],
                ct->literal[2],
                ct->literal[3]);
        }
    }
    else
    {
        Com_Printf(8, "No materials matching %s were found.\n", search);
    }
}

void __cdecl R_MaterialParameterEdit_f()
{
    const char *v0; // eax
    float max; // [esp+60h] [ebp-50h]
    float min; // [esp+64h] [ebp-4Ch]
    dvar_s *hack; // [esp+68h] [ebp-48h]
    MaterialConstantDef *v4; // [esp+6Ch] [ebp-44h]
    unsigned int hash; // [esp+70h] [ebp-40h]
    char dest[16]; // [esp+74h] [ebp-3Ch] BYREF
    MaterialConstantDef *ct; // [esp+84h] [ebp-2Ch]
    const char *param; // [esp+88h] [ebp-28h]
    const char *name; // [esp+8Ch] [ebp-24h]
    int index; // [esp+90h] [ebp-20h]
    int i; // [esp+94h] [ebp-1Ch]
    MaterialConstantDef *cte; // [esp+98h] [ebp-18h]
    char NameBuf[16]; // [esp+9Ch] [ebp-14h] BYREF

    if ( Current_Edit_Material )
    {
        if ( Cmd_Argc() >= 2 )
        {
            name = Cmd_Argv(1);
            param = name;
            index = atoi(name);
            if ( index && index <= Current_Edit_Material->constantCount )
            {
                strncpy(NameBuf, Current_Edit_Material->localConstantTable[index - 1].name, 0xCu);
                NameBuf[12] = 0;
                param = (const char *)NameBuf;
            }
            else
            {
                hash = R_HashString(name);
                for ( i = 0; i < Current_Edit_Material->constantCount; ++i )
                {
                    v4 = &Current_Edit_Material->localConstantTable[i];
                    if ( !strncmp(v4->name, name, 0xCu) && v4->nameHash == hash )
                    {
                        index = i + 1;
                        break;
                    }
                }
            }
            if ( index )
            {
                cte = &Current_Edit_Material->localConstantTable[index - 1];
                Com_Printf(8, "%s:\n", Current_Edit_Material->info.name);
                if ( Cmd_Argc() == 2 )
                {
                    Com_Printf(
                        8,
                        "    %s = %g %g %g %g\n",
                        param,
                        cte->literal[0],
                        cte->literal[1],
                        cte->literal[2],
                        cte->literal[3]);
                }
                else
                {
                    Com_Printf(
                        8,
                        "    Before: %s = %g %g %g %g\n",
                        param,
                        cte->literal[0],
                        cte->literal[1],
                        cte->literal[2],
                        cte->literal[3]);
                    for ( i = 2; i < Cmd_Argc() && i < 6; ++i )
                    {
                        v0 = Cmd_Argv(i);
                        *(float *)&cte->name[4 * i + 4] = atof(v0);
                    }
                    Com_Printf(
                        8,
                        "    After: %s = %g %g %g %g\n",
                        param,
                        cte->literal[0],
                        cte->literal[1],
                        cte->literal[2],
                        cte->literal[3]);
                }
                if ( cte == MaterialParameterConstantEntry && MaterialParameterDvar )
                {
                    hack = (dvar_s *)MaterialParameterDvar;
                    MaterialParameterDvar->modified = 0;
                    hack->current = *(DvarValue *)cte->literal;
                    hack->latched = *(DvarValue *)cte->literal;
                    hack->reset = *(DvarValue *)cte->literal;
                    min = hack->domain.value.min;
                    max = hack->domain.value.max;
                    if ( min > cte->literal[0] )
                        min = cte->literal[0];
                    if ( min > cte->literal[1] )
                        min = cte->literal[1];
                    if ( min > cte->literal[2] )
                        min = cte->literal[2];
                    if ( min > cte->literal[3] )
                        min = cte->literal[3];
                    if ( cte->literal[0] > max )
                        max = cte->literal[0];
                    if ( cte->literal[1] > max )
                        max = cte->literal[1];
                    if ( cte->literal[2] > max )
                        max = cte->literal[2];
                    if ( cte->literal[3] > max )
                        max = cte->literal[3];
                    hack->domain.value.min = min;
                    hack->domain.value.max = max;
                }
            }
            else
            {
                Com_Printf(8, "%s is not a parameter of %s.\n", name, Current_Edit_Material->info.name);
            }
        }
        else
        {
            Com_Printf(8, "%s:\n", Current_Edit_Material->info.name);
            for ( i = 0; i < Current_Edit_Material->constantCount; ++i )
            {
                ct = &Current_Edit_Material->localConstantTable[i];
                strncpy(dest, ct->name, 0xCu);
                dest[12] = 0;
                Com_Printf(
                    8,
                    "    %d: %12s = %g %g %g %g\n",
                    i + 1,
                    (const char *)dest,
                    ct->literal[0],
                    ct->literal[1],
                    ct->literal[2],
                    ct->literal[3]);
            }
        }
    }
    else
    {
        Com_Printf(8, "No material selected for editing.\n");
    }
}

void __cdecl R_MaterialParameterTweak_f()
{
    const char *v0; // eax
    const char *v1; // eax
    dvar_s *v2; // eax
    __int64 dvarDomain_4; // [esp+74h] [ebp-64h]
    int dvarDomain_12; // [esp+7Ch] [ebp-5Ch]
    dvar_s *hack; // [esp+80h] [ebp-58h]
    float v6; // [esp+84h] [ebp-54h]
    float val; // [esp+88h] [ebp-50h]
    MaterialConstantDef *v8; // [esp+8Ch] [ebp-4Ch]
    unsigned int hash; // [esp+90h] [ebp-48h]
    char dest[16]; // [esp+94h] [ebp-44h] BYREF
    MaterialConstantDef *ct; // [esp+A4h] [ebp-34h]
    float max; // [esp+A8h] [ebp-30h]
    const char *param; // [esp+ACh] [ebp-2Ch]
    const char *name; // [esp+B0h] [ebp-28h]
    int index; // [esp+B4h] [ebp-24h]
    float min; // [esp+B8h] [ebp-20h]
    int i; // [esp+BCh] [ebp-1Ch]
    MaterialConstantDef *cte; // [esp+C0h] [ebp-18h]
    char NameBuf[16]; // [esp+C4h] [ebp-14h] BYREF

    MaterialParameterConstantEntry = 0;
    if ( Current_Edit_Material )
    {
        if ( Cmd_Argc() >= 2 )
        {
            name = Cmd_Argv(1);
            param = name;
            index = atoi(name);
            if ( index && index <= Current_Edit_Material->constantCount )
            {
                strncpy(NameBuf, Current_Edit_Material->localConstantTable[index - 1].name, 0xCu);
                NameBuf[12] = 0;
                param = (const char *)NameBuf;
            }
            else
            {
                hash = R_HashString(name);
                for ( i = 0; i < Current_Edit_Material->constantCount; ++i )
                {
                    v8 = &Current_Edit_Material->localConstantTable[i];
                    if ( !strncmp(v8->name, name, 0xCu) && v8->nameHash == hash )
                    {
                        index = i + 1;
                        break;
                    }
                }
            }
            if ( index )
            {
                cte = &Current_Edit_Material->localConstantTable[index - 1];
                min = cte->literal[0];
                max = cte->literal[0];
                if ( min > cte->literal[1] )
                    min = cte->literal[1];
                if ( min > cte->literal[2] )
                    min = cte->literal[2];
                if ( min > cte->literal[3] )
                    min = cte->literal[3];
                if ( cte->literal[1] > max )
                    max = cte->literal[1];
                if ( cte->literal[2] > max )
                    max = cte->literal[2];
                if ( cte->literal[3] > max )
                    max = cte->literal[3];
                if ( Cmd_Argc() < 3 )
                {
                    if ( min > 0.0 )
                        min = 0.0f;
                }
                else
                {
                    v0 = Cmd_Argv(2);
                    val = atof(v0);
                    if ( min > val )
                        min = val;
                }
                if ( Cmd_Argc() < 4 )
                {
                    if ( max < 1.0 )
                        max = 1.0f;
                }
                else
                {
                    v1 = Cmd_Argv(3);
                    v6 = atof(v1);
                    if ( v6 > max )
                        max = v6;
                }
                v2 = (dvar_s*)_Dvar_RegisterVec4(
                             "r_MaterialParameterTweak",
                             (cte->literal[0]),
                             (cte->literal[1]),
                             (cte->literal[2]),
                             (cte->literal[3]),
                             min,
                             max,
                             0x280u,
                             "Modifies currently selected material parameter");
                MaterialParameterDvar = v2;
                hack = (dvar_s *)v2;
                v2->modified = 0;
                v2->current = *(DvarValue *)cte->literal;
                v2->latched = *(DvarValue *)cte->literal;
                v2->reset = *(DvarValue *)cte->literal;
                //*(float *)&dvarDomain_4 = max;
                v2->domain.value.min = min;
                v2->domain.value.max = max;
                //*(_QWORD *)&v2->domain.value.max = dvarDomain_4;
                //*((unsigned int *)&v2->domain.vector + 3) = dvarDomain_12;
                sprintf(desc, "%s . %s parameter tweak", Current_Edit_Material->info.name, param);
                hack->description = desc;
                MaterialParameterConstantEntry = cte;
                memcpy(&MaterialParameterConstantEntry_save, cte, sizeof(MaterialParameterConstantEntry_save));
                Com_Printf(8, "%s:\n", Current_Edit_Material->info.name);
                Com_Printf(8, "    %s = %g %g %g %g\n", param, cte->literal[0], cte->literal[1], cte->literal[2], cte->literal[3]);
            }
            else
            {
                Com_Printf(8, "%s is not a parameter of %s.\n", name, Current_Edit_Material->info.name);
            }
        }
        else
        {
            Com_Printf(8, "%s:\n", Current_Edit_Material->info.name);
            for ( i = 0; i < Current_Edit_Material->constantCount; ++i )
            {
                ct = &Current_Edit_Material->localConstantTable[i];
                strncpy(dest, ct->name, 0xCu);
                dest[12] = 0;
                Com_Printf(
                    8,
                    "    %d: %12s = %g %g %g %g\n",
                    i + 1,
                    (const char *)dest,
                    ct->literal[0],
                    ct->literal[1],
                    ct->literal[2],
                    ct->literal[3]);
            }
        }
    }
    else
    {
        Com_Printf(8, "No material selected for tweaking.\n");
    }
}

void __cdecl R_MaterialParameterTweakUpdate()
{
    if ( MaterialParameterConstantEntry
        && MaterialParameterConstantEntry->nameHash == MaterialParameterConstantEntry_save.nameHash
        && MaterialParameterDvar
        && MaterialParameterDvar->modified )
    {
        *(DvarValue *)MaterialParameterConstantEntry->literal = MaterialParameterDvar->current;
        Dvar_ClearModified(MaterialParameterDvar);
    }
}

void __cdecl R_EnumTechniqueSets(void (__cdecl *func)(MaterialTechniqueSet *, void *), void *data)
{
    MaterialTechniqueSet *header; // [esp+0h] [ebp-8h]
    unsigned int hashIndex; // [esp+4h] [ebp-4h]

    for ( hashIndex = 0; hashIndex < 0x800; ++hashIndex )
    {
        header = materialGlobals.techniqueSetHashTable[hashIndex];
        if ( header )
            func(header, data);
    }
}

void __cdecl R_EnumMaterials(void (__cdecl *func)(Material *, void *), void *data)
{
    Material *header; // [esp+0h] [ebp-8h]
    unsigned int hashIndex; // [esp+4h] [ebp-4h]

    for ( hashIndex = 0; hashIndex < 0x1000; ++hashIndex )
    {
        header = rg.materialHashTable[hashIndex];
        if ( header )
            func(header, data);
    }
}

const char *__cdecl Material_GetName(Material *handle)
{
    if ( !handle
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 2051, 0, "%s", "handle") )
    {
        __debugbreak();
    }
    return Material_FromHandle(handle)->info.name;
}

void __cdecl R_InitAssets_PostMapFastfileLoad()
{
    rgp.permapAssets.flameThrowerFXMaterial = 0;
    rgp.permapAssets.electrifiedFXMaterial = 0;
    rgp.permapAssets.transportedFXMaterial = 0;
    Material_LoadPerMap(s_permapMaterials, 3);
}

void __cdecl Material_LoadPerMap(const PerMapMaterialTable *mtlTable, int mtlTableCount)
{
    int permapMtlIndex; // [esp+0h] [ebp-4h]

    for ( permapMtlIndex = 0; permapMtlIndex < mtlTableCount; ++permapMtlIndex )
    {
        if ( *mtlTable[permapMtlIndex].material
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp",
                        2095,
                        0,
                        "%s",
                        "!*mtlTable[permapMtlIndex].material") )
        {
            __debugbreak();
        }
        *mtlTable[permapMtlIndex].material = Material_Register((char *)mtlTable[permapMtlIndex].name, 0);
        if ( !*mtlTable[permapMtlIndex].material )
            Com_Error(ERR_FATAL, "Could not find permap material '%s'", mtlTable[permapMtlIndex].name);
    }
}

void __cdecl Material_Init()
{
    if ( !useFastFile->current.enabled )
    {
        memset(&materialGlobals, 0, sizeof(materialGlobals));
        Material_PreLoadAllShaderText();
    }
    Material_LoadBuiltIn(s_builtInMaterials, 122);
    Material_Register((char*)"statmon_warning_tris", 1);
}

void __cdecl Material_LoadBuiltIn(const BuiltInMaterialTable *mtlTable, int mtlTableCount)
{
    int builtInMtlIndex; // [esp+0h] [ebp-4h]

    for ( builtInMtlIndex = 0; builtInMtlIndex < mtlTableCount; ++builtInMtlIndex )
    {
        if ( *mtlTable[builtInMtlIndex].material
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp",
                        2077,
                        0,
                        "%s",
                        "!*mtlTable[builtInMtlIndex].material") )
        {
            __debugbreak();
        }
        *mtlTable[builtInMtlIndex].material = Material_Register((char *)mtlTable[builtInMtlIndex].name, 0);
        if ( !*mtlTable[builtInMtlIndex].material )
            Com_Error(ERR_FATAL, "Could not find material '%s'", mtlTable[builtInMtlIndex].name);
        rgp.dummyMaterial = 0;
    }
}

void __cdecl Material_Shutdown()
{
    Material_FreeAll();
    memset(&materialGlobals, 0, sizeof(materialGlobals));
    memset(rg.materialHashTable, 0, sizeof(rg.materialHashTable));
    rgp.materialCount = 0;
}

void __cdecl Material_UpdatePicmipAll()
{
    R_SyncRenderThread();
    R_SetPicmip();
    DB_EnumXAssets(ASSET_TYPE_MATERIAL, (void (__cdecl *)(XAssetHeader, void *))Material_UpdatePicmipSingle, 0, 1);
}

void __cdecl Material_UpdatePicmipSingle(XAssetHeader header)
{
    int textureIndex; // [esp+4h] [ebp-4h]

    for ( textureIndex = 0; textureIndex < BYTE2(header.xmodelPieces[14].name); ++textureIndex )
        Material_UpdatePicmipForTexdef((const MaterialTextureDef *)&header.xmodelPieces[15].name[16 * textureIndex]);
}

void __cdecl Material_UpdatePicmipForTexdef(const MaterialTextureDef *texdef)
{
    if ( !texdef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 2252, 0, "%s", "texdef") )
    {
        __debugbreak();
    }
    if ( texdef->semantic != 11 )
    {
        if ( texdef->u.image )
            Image_UpdatePicmip(texdef->u.image);
    }
}

void __cdecl R_Cmd_ReloadMaterialTextures()
{
    char *v0; // eax
    Material *material; // [esp+10h] [ebp-8h]
    const char *name; // [esp+14h] [ebp-4h]

    if ( Cmd_Argc() == 2 )
    {
        name = Cmd_Argv(1);
        if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 2303, 0, "%s", "name") )
            __debugbreak();
        material = Material_Find(name);
        if ( material )
        {
            RB_BindDefaultImages();
            Material_ReloadTextures(material);
        }
        else
        {
            v0 = va("ReloadMaterialTextures: Material '%s' is not currently loaded.\n", name);
            Com_Printf(8, v0);
        }
    }
    else
    {
        Com_Printf(8, "Usage: reloadmaterialtextures <materialname>\n");
    }
}

Material *__cdecl Material_Find(const char *name)
{
    Material *material; // [esp+0h] [ebp-Ch]
    unsigned __int16 hashIndex[3]; // [esp+4h] [ebp-8h] BYREF
    bool exists; // [esp+Bh] [ebp-1h] BYREF

    Material_GetHashIndex(name, hashIndex, &exists);
    if ( !exists )
        return 0;
    material = rg.materialHashTable[hashIndex[0]];
    if ( !material
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 1620, 1, "%s", "material") )
    {
        __debugbreak();
    }
    return material;
}

void __cdecl Material_ReloadTextures(const Material *material)
{
    GfxImage *image; // [esp+0h] [ebp-18h]
    GfxImage *maxConvert; // [esp+4h] [ebp-14h]
    GfxImage *lastConverted; // [esp+8h] [ebp-10h]
    int textureIter; // [esp+Ch] [ebp-Ch]
    int textureCount; // [esp+10h] [ebp-8h]
    const MaterialTextureDef *texture; // [esp+14h] [ebp-4h]

    if ( !material
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp", 2218, 0, "%s", "material") )
    {
        __debugbreak();
    }
    textureCount = material->textureCount;
    lastConverted = 0;
    while ( 1 )
    {
        maxConvert = 0;
        for ( textureIter = 0; textureIter != textureCount; ++textureIter )
        {
            texture = &material->textureTable[textureIter];
            if ( texture->semantic != 11 )
            {
                image = texture->u.image;
                if ( image > maxConvert && (!lastConverted || image < lastConverted) )
                    maxConvert = texture->u.image;
            }
        }
        lastConverted = maxConvert;
        if ( !maxConvert )
            break;
        Image_Reload(maxConvert);
    }
}

unsigned int __cdecl Material_LoadFile(const char *filename, int *file)
{
    char fullFilename[68]; // [esp+0h] [ebp-48h] BYREF

    Com_sprintf(fullFilename, 0x40u, "materials/%s", filename);
    return FS_FOpenFileReadCurrentThread(fullFilename, file);
}

bool __cdecl IsValidMaterialHandle(Material *const handle)
{
    if ( ((unsigned __int8)handle & 3) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp",
                    2407,
                    0,
                    "%s",
                    "( 0x0003 & reinterpret_cast<int>( handle ) ) == 0x0") )
    {
        __debugbreak();
    }
    return handle && handle->info.name && *handle->info.name;
}

int __cdecl GetCodeConst_GenericParam0()
{
    return 183;
}

int __cdecl GetCodeConst_GenericParam1()
{
    return 184;
}

int __cdecl GetCodeConst_GenericParam2()
{
    return 185;
}

int __cdecl GetCodeConst_GenericParam3()
{
    return 186;
}

int __cdecl GetCodeConst_GenericParamA()
{
    return 180;
}

int __cdecl GetCodeConst_GenericParamB()
{
    return 181;
}

int __cdecl GetCodeConst_GameTimeParam()
{
    return 32;
}

int __cdecl Material_GetTextureCount(Material *handle)
{
    return handle->textureCount;
}

GfxImage *__cdecl Material_GetTexture(Material *handle, int index)
{
    if ( index >= Material_GetTextureCount(handle)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material.cpp",
                    2507,
                    0,
                    "%s",
                    "index < Material_GetTextureCount( handle )") )
    {
        __debugbreak();
    }
    return handle->textureTable[index].u.image;
}

