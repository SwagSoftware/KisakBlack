#include "r_material_load_obj.h"
#include <universal/q_shared.h>
#include <qcommon/common.h>
#include <universal/q_parse.h>
#include <universal/com_files.h>
#include <qcommon/com_profilemapload.h>
#include "r_utils.h"
#include "r_init.h"
#include <universal/com_memory.h>
#include <qcommon/md4.h>
#include <qcommon/com_bsp_load_obj.h>
#include "r_bsp_load_obj.h"
#include "r_dvars.h"
#include "r_water_load_obj.h"

#include <algorithm>
#include <cgame/cg_drawtools.h>
#include "r_material_consts.h"
#include "r_state.h"
#include "r_image.h"

#include <d3dx9.h>

MaterialTypeInfo g_materialTypeInfo[5] =
{
  { "", "", 0u },
  { "m/", "m_", 2u },
  { "mc/", "mc_", 3u },
  { "w/", "w_", 2u },
  { "wc/", "wc_", 3u }
};

const LayeredTechniqueSetName s_lyrTechSetNames[121] =
{
  { "l_sm_a0c0", "l_sm_", "l_[hsm|sm]_", "a0c0" },
  { "l_sm_a0c0d0", "l_sm_", "l_[hsm|sm]_", "a0c0d0" },
  { "l_sm_a0c0d0n0", "l_sm_", "l_[hsm|sm]_", "a0c0d0n0" },
  { "l_sm_a0c0d0n0s0", "l_sm_", "l_[hsm|sm]_", "a0c0d0n0s0" },
  { "l_sm_a0c0d0s0", "l_sm_", "l_[hsm|sm]_", "a0c0d0s0" },
  { "l_sm_a0c0n0", "l_sm_", "l_[hsm|sm]_", "a0c0n0" },
  { "l_sm_a0c0n0s0", "l_sm_", "l_[hsm|sm]_", "a0c0n0s0" },
  { "l_sm_a0c0s0", "l_sm_", "l_[hsm|sm]_", "a0c0s0" },
  { "l_sm_b0c0", "l_sm_", "l_[hsm|sm]_", "b0c0" },
  { "l_sm_b0c0_sco", "l_sm_", "l_[hsm|sm]_", "b0c0_sco" },
  { "l_sm_b0c0_seethru", "l_sm_", "l_[hsm|sm]_", "b0c0_seethru" },
  { "l_sm_b0c0d0", "l_sm_", "l_[hsm|sm]_", "b0c0d0" },
  { "l_sm_b0c0d0_sco", "l_sm_", "l_[hsm|sm]_", "b0c0d0_sco" },
  { "l_sm_b0c0d0_seethru", "l_sm_", "l_[hsm|sm]_", "b0c0d0_seethru" },
  { "l_sm_b0c0d0n0", "l_sm_", "l_[hsm|sm]_", "b0c0d0n0" },
  { "l_sm_b0c0d0n0_sco", "l_sm_", "l_[hsm|sm]_", "b0c0d0n0_sco" },
  { "l_sm_b0c0d0n0_seethru", "l_sm_", "l_[hsm|sm]_", "b0c0d0n0_seethru" },
  { "l_sm_b0c0d0n0s0", "l_sm_", "l_[hsm|sm]_", "b0c0d0n0s0" },
  { "l_sm_b0c0d0n0s0_sco", "l_sm_", "l_[hsm|sm]_", "b0c0d0n0s0_sco" },
  { "l_sm_b0c0d0n0s0_seethru", "l_sm_", "l_[hsm|sm]_", "b0c0d0n0s0_seethru" },
  { "l_sm_b0c0d0n0s0sc0x0", "l_sm_", "l_[hsm|sm]_", "b0c0d0n0s0sc0x0" },
  { "l_sm_b0c0d0n0s0x0", "l_sm_", "l_[hsm|sm]_", "b0c0d0n0s0x0" },
  { "l_sm_b0c0d0n0x0", "l_sm_", "l_[hsm|sm]_", "b0c0d0n0x0" },
  { "l_sm_b0c0d0s0", "l_sm_", "l_[hsm|sm]_", "b0c0d0s0" },
  { "l_sm_b0c0d0s0_sco", "l_sm_", "l_[hsm|sm]_", "b0c0d0s0_sco" },
  { "l_sm_b0c0d0s0_seethru", "l_sm_", "l_[hsm|sm]_", "b0c0d0s0_seethru" },
  { "l_sm_b0c0d0s0sc0x0", "l_sm_", "l_[hsm|sm]_", "b0c0d0s0sc0x0" },
  { "l_sm_b0c0d0s0x0", "l_sm_", "l_[hsm|sm]_", "b0c0d0s0x0" },
  { "l_sm_b0c0d0x0", "l_sm_", "l_[hsm|sm]_", "b0c0d0x0" },
  { "l_sm_b0c0n0", "l_sm_", "l_[hsm|sm]_", "b0c0n0" },
  { "l_sm_b0c0n0_sco", "l_sm_", "l_[hsm|sm]_", "b0c0n0_sco" },
  { "l_sm_b0c0n0_seethru", "l_sm_", "l_[hsm|sm]_", "b0c0n0_seethru" },
  { "l_sm_b0c0n0s0", "l_sm_", "l_[hsm|sm]_", "b0c0n0s0" },
  { "l_sm_b0c0n0s0_sco", "l_sm_", "l_[hsm|sm]_", "b0c0n0s0_sco" },
  { "l_sm_b0c0n0s0_seethru", "l_sm_", "l_[hsm|sm]_", "b0c0n0s0_seethru" },
  { "l_sm_b0c0n0s0sc0x0", "l_sm_", "l_[hsm|sm]_", "b0c0n0s0sc0x0" },
  { "l_sm_b0c0n0s0x0", "l_sm_", "l_[hsm|sm]_", "b0c0n0s0x0" },
  { "l_sm_b0c0n0x0", "l_sm_", "l_[hsm|sm]_", "b0c0n0x0" },
  { "l_sm_b0c0s0", "l_sm_", "l_[hsm|sm]_", "b0c0s0" },
  { "l_sm_b0c0s0_sco", "l_sm_", "l_[hsm|sm]_", "b0c0s0_sco" },
  { "l_sm_b0c0s0_seethru", "l_sm_", "l_[hsm|sm]_", "b0c0s0_seethru" },
  { "l_sm_b0c0s0sc0x0", "l_sm_", "l_[hsm|sm]_", "b0c0s0sc0x0" },
  { "l_sm_b0c0s0x0", "l_sm_", "l_[hsm|sm]_", "b0c0s0x0" },
  { "l_sm_b0c0x0", "l_sm_", "l_[hsm|sm]_", "b0c0x0" },
  { "l_sm_r0c0", "l_sm_", "l_[hsm|sm]_", "r0c0" },
  { "l_sm_r0c0_sco", "l_sm_", "l_[hsm|sm]_", "r0c0_sco" },
  { "l_sm_r0c0_seethru", "l_sm_", "l_[hsm|sm]_", "r0c0_seethru" },
  { "l_sm_r0c0d0", "l_sm_", "l_[hsm|sm]_", "r0c0d0" },
  { "l_sm_r0c0d0_sco", "l_sm_", "l_[hsm|sm]_", "r0c0d0_sco" },
  { "l_sm_r0c0d0_seethru", "l_sm_", "l_[hsm|sm]_", "r0c0d0_seethru" },
  { "l_sm_r0c0d0n0", "l_sm_", "l_[hsm|sm]_", "r0c0d0n0" },
  { "l_sm_r0c0d0n0_sco", "l_sm_", "l_[hsm|sm]_", "r0c0d0n0_sco" },
  { "l_sm_r0c0d0n0_seethru", "l_sm_", "l_[hsm|sm]_", "r0c0d0n0_seethru" },
  { "l_sm_r0c0d0n0s0", "l_sm_", "l_[hsm|sm]_", "r0c0d0n0s0" },
  { "l_sm_r0c0d0n0s0_sco", "l_sm_", "l_[hsm|sm]_", "r0c0d0n0s0_sco" },
  { "l_sm_r0c0d0n0s0_seethru", "l_sm_", "l_[hsm|sm]_", "r0c0d0n0s0_seethru" },
  { "l_sm_r0c0d0n0s0sc0x0", "l_sm_", "l_[hsm|sm]_", "r0c0d0n0s0sc0x0" },
  { "l_sm_r0c0d0n0s0x0", "l_sm_", "l_[hsm|sm]_", "r0c0d0n0s0x0" },
  { "l_sm_r0c0d0n0x0", "l_sm_", "l_[hsm|sm]_", "r0c0d0n0x0" },
  { "l_sm_r0c0d0s0", "l_sm_", "l_[hsm|sm]_", "r0c0d0s0" },
  { "l_sm_r0c0d0s0_sco", "l_sm_", "l_[hsm|sm]_", "r0c0d0s0_sco" },
  { "l_sm_r0c0d0s0_seethru", "l_sm_", "l_[hsm|sm]_", "r0c0d0s0_seethru" },
  { "l_sm_r0c0d0s0sc0x0", "l_sm_", "l_[hsm|sm]_", "r0c0d0s0sc0x0" },
  { "l_sm_r0c0d0s0x0", "l_sm_", "l_[hsm|sm]_", "r0c0d0s0x0" },
  { "l_sm_r0c0d0x0", "l_sm_", "l_[hsm|sm]_", "r0c0d0x0" },
  { "l_sm_r0c0n0", "l_sm_", "l_[hsm|sm]_", "r0c0n0" },
  { "l_sm_r0c0n0_sco", "l_sm_", "l_[hsm|sm]_", "r0c0n0_sco" },
  { "l_sm_r0c0n0_seethru", "l_sm_", "l_[hsm|sm]_", "r0c0n0_seethru" },
  { "l_sm_r0c0n0s0", "l_sm_", "l_[hsm|sm]_", "r0c0n0s0" },
  { "l_sm_r0c0n0s0_sco", "l_sm_", "l_[hsm|sm]_", "r0c0n0s0_sco" },
  { "l_sm_r0c0n0s0_seethru", "l_sm_", "l_[hsm|sm]_", "r0c0n0s0_seethru" },
  { "l_sm_r0c0n0s0sc0x0", "l_sm_", "l_[hsm|sm]_", "r0c0n0s0sc0x0" },
  { "l_sm_r0c0n0s0x0", "l_sm_", "l_[hsm|sm]_", "r0c0n0s0x0" },
  { "l_sm_r0c0n0x0", "l_sm_", "l_[hsm|sm]_", "r0c0n0x0" },
  { "l_sm_r0c0s0", "l_sm_", "l_[hsm|sm]_", "r0c0s0" },
  { "l_sm_r0c0s0_sco", "l_sm_", "l_[hsm|sm]_", "r0c0s0_sco" },
  { "l_sm_r0c0s0_seethru", "l_sm_", "l_[hsm|sm]_", "r0c0s0_seethru" },
  { "l_sm_r0c0s0sc0x0", "l_sm_", "l_[hsm|sm]_", "r0c0s0sc0x0" },
  { "l_sm_r0c0s0x0", "l_sm_", "l_[hsm|sm]_", "r0c0s0x0" },
  { "l_sm_r0c0x0", "l_sm_", "l_[hsm|sm]_", "r0c0x0" },
  { "l_sm_t0c0", "l_sm_", "l_[hsm|sm]_", "t0c0" },
  { "l_sm_t0c0_sco", "l_sm_", "l_[hsm|sm]_", "t0c0_sco" },
  { "l_sm_t0c0_seethru", "l_sm_", "l_[hsm|sm]_", "t0c0_seethru" },
  { "l_sm_t0c0d0", "l_sm_", "l_[hsm|sm]_", "t0c0d0" },
  { "l_sm_t0c0d0_sco", "l_sm_", "l_[hsm|sm]_", "t0c0d0_sco" },
  { "l_sm_t0c0d0_seethru", "l_sm_", "l_[hsm|sm]_", "t0c0d0_seethru" },
  { "l_sm_t0c0d0n0", "l_sm_", "l_[hsm|sm]_", "t0c0d0n0" },
  { "l_sm_t0c0d0n0_sco", "l_sm_", "l_[hsm|sm]_", "t0c0d0n0_sco" },
  { "l_sm_t0c0d0n0_seethru", "l_sm_", "l_[hsm|sm]_", "t0c0d0n0_seethru" },
  { "l_sm_t0c0d0n0s0", "l_sm_", "l_[hsm|sm]_", "t0c0d0n0s0" },
  { "l_sm_t0c0d0n0s0_sco", "l_sm_", "l_[hsm|sm]_", "t0c0d0n0s0_sco" },
  { "l_sm_t0c0d0n0s0_seethru", "l_sm_", "l_[hsm|sm]_", "t0c0d0n0s0_seethru" },
  { "l_sm_t0c0d0n0s0sc0x0", "l_sm_", "l_[hsm|sm]_", "t0c0d0n0s0sc0x0" },
  { "l_sm_t0c0d0n0s0x0", "l_sm_", "l_[hsm|sm]_", "t0c0d0n0s0x0" },
  { "l_sm_t0c0d0n0x0", "l_sm_", "l_[hsm|sm]_", "t0c0d0n0x0" },
  { "l_sm_t0c0d0s0", "l_sm_", "l_[hsm|sm]_", "t0c0d0s0" },
  { "l_sm_t0c0d0s0_sco", "l_sm_", "l_[hsm|sm]_", "t0c0d0s0_sco" },
  { "l_sm_t0c0d0s0_seethru", "l_sm_", "l_[hsm|sm]_", "t0c0d0s0_seethru" },
  { "l_sm_t0c0d0s0sc0x0", "l_sm_", "l_[hsm|sm]_", "t0c0d0s0sc0x0" },
  { "l_sm_t0c0d0s0x0", "l_sm_", "l_[hsm|sm]_", "t0c0d0s0x0" },
  { "l_sm_t0c0d0x0", "l_sm_", "l_[hsm|sm]_", "t0c0d0x0" },
  { "l_sm_t0c0n0", "l_sm_", "l_[hsm|sm]_", "t0c0n0" },
  { "l_sm_t0c0n0_sco", "l_sm_", "l_[hsm|sm]_", "t0c0n0_sco" },
  { "l_sm_t0c0n0_seethru", "l_sm_", "l_[hsm|sm]_", "t0c0n0_seethru" },
  { "l_sm_t0c0n0s0", "l_sm_", "l_[hsm|sm]_", "t0c0n0s0" },
  { "l_sm_t0c0n0s0_sco", "l_sm_", "l_[hsm|sm]_", "t0c0n0s0_sco" },
  { "l_sm_t0c0n0s0_seethru", "l_sm_", "l_[hsm|sm]_", "t0c0n0s0_seethru" },
  { "l_sm_t0c0n0s0sc0x0", "l_sm_", "l_[hsm|sm]_", "t0c0n0s0sc0x0" },
  { "l_sm_t0c0n0s0x0", "l_sm_", "l_[hsm|sm]_", "t0c0n0s0x0" },
  { "l_sm_t0c0n0x0", "l_sm_", "l_[hsm|sm]_", "t0c0n0x0" },
  { "l_sm_t0c0s0", "l_sm_", "l_[hsm|sm]_", "t0c0s0" },
  { "l_sm_t0c0s0_sco", "l_sm_", "l_[hsm|sm]_", "t0c0s0_sco" },
  { "l_sm_t0c0s0_seethru", "l_sm_", "l_[hsm|sm]_", "t0c0s0_seethru" },
  { "l_sm_t0c0s0sc0x0", "l_sm_", "l_[hsm|sm]_", "t0c0s0sc0x0" },
  { "l_sm_t0c0s0x0", "l_sm_", "l_[hsm|sm]_", "t0c0s0x0" },
  { "l_sm_t0c0x0", "l_sm_", "l_[hsm|sm]_", "t0c0x0" },
  { "unlit_add", NULL, NULL, "a0c0" },
  { "unlit_blend", NULL, NULL, "b0c0" },
  { "unlit_multiply", NULL, NULL, "m0c0" },
  { "unlitdecalblend_add", NULL, NULL, "a0c0" },
  { "unlitdecalblend_multiply", NULL, NULL, "m0c0" }
};

const bool g_useTechnique[130] =
{
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  false,
  false,
  false,
  false,
  true,
  false,
  true,
  true,
  true
};

const CodeSamplerSource s_lightSamplers[2] =
{
  { "attenuation", TEXTURE_SRC_CODE_LIGHT_ATTENUATION, NULL, 0, 0 },
  { NULL, TEXTURE_SRC_CODE_BLACK, NULL, 0, 0 }
};

const CodeSamplerSource s_lightmapSamplers[4] =
{
  { "primary", TEXTURE_SRC_CODE_LIGHTMAP_PRIMARY, NULL, 0, 0 },
  { "secondary", TEXTURE_SRC_CODE_LIGHTMAP_SECONDARY, NULL, 0, 0 },
  { "secondaryb", TEXTURE_SRC_CODE_LIGHTMAP_SECONDARYB, NULL, 0, 0 },
  { NULL, TEXTURE_SRC_CODE_BLACK, NULL, 0, 0 }
};

const CodeSamplerSource s_codeSamplers[28] =
{
  { "white", TEXTURE_SRC_CODE_WHITE, NULL, 0, 0 },
  { "black", TEXTURE_SRC_CODE_BLACK, NULL, 0, 0 },
  { "identityNormalMap", TEXTURE_SRC_CODE_IDENTITY_NORMAL_MAP, NULL, 0, 0 },
  { "lightmap", TEXTURE_SRC_CODE_LIGHTMAP_PRIMARY, s_lightmapSamplers, 0, 0 },
  { "outdoor", TEXTURE_SRC_CODE_OUTDOOR, NULL, 0, 0 },
  { "shadowmapSun", TEXTURE_SRC_CODE_SHADOWMAP_SUN, NULL, 0, 0 },
  { "shadowmapSpot", TEXTURE_SRC_CODE_SHADOWMAP_SPOT, NULL, 0, 0 },
  { "feedback", TEXTURE_SRC_CODE_FEEDBACK, NULL, 0, 0 },
  { "resolvedPostSun", TEXTURE_SRC_CODE_RESOLVED_POST_SUN, NULL, 0, 0 },
  { "resolvedScene", TEXTURE_SRC_CODE_RESOLVED_SCENE, NULL, 0, 0 },
  { "postEffectSrc", TEXTURE_SRC_CODE_POST_EFFECT_SRC, NULL, 0, 0 },
  { "postEffectGodRays", TEXTURE_SRC_CODE_POST_EFFECT_GODRAYS, NULL, 0, 0 },
  { "postEffect0", TEXTURE_SRC_CODE_POST_EFFECT_0, NULL, 0, 0 },
  { "postEffect1", TEXTURE_SRC_CODE_POST_EFFECT_1, NULL, 0, 0 },
  { "sky", TEXTURE_SRC_CODE_SKY, NULL, 0, 0 },
  { "light", TEXTURE_SRC_CODE_LIGHT_ATTENUATION, s_lightSamplers, 0, 0 },
  { "floatZ", TEXTURE_SRC_CODE_FLOATZ, NULL, 0, 0 },
  { "processedFloatZ", TEXTURE_SRC_CODE_PROCESSED_FLOATZ, NULL, 0, 0 },
  { "rawFloatZ", TEXTURE_SRC_CODE_RAW_FLOATZ, NULL, 0, 0 },
  { "codeTexture0", TEXTURE_SRC_CODE_TEXTURE_0, NULL, 0, 0 },
  { "codeTexture1", TEXTURE_SRC_CODE_TEXTURE_1, NULL, 0, 0 },
  { "codeTexture2", TEXTURE_SRC_CODE_TEXTURE_2, NULL, 0, 0 },
  { "codeTexture3", TEXTURE_SRC_CODE_TEXTURE_3, NULL, 0, 0 },
  { "impactMask", TEXTURE_SRC_CODE_IMPACT_MASK, NULL, 0, 0 },
  { "ui3d", TEXTURE_SRC_CODE_UI3D, NULL, 0, 0 },
  { "missileCam", TEXTURE_SRC_CODE_MISSILE_CAM, NULL, 0, 0 },
  { "compositeResult", TEXTURE_SRC_CODE_COMPOSITE_RESULT, NULL, 0, 0 },
  { NULL, TEXTURE_SRC_CODE_BLACK, NULL, 0, 0 }
};

const CodeSamplerSource s_defaultCodeSamplers[27] =
{
  { "shadowmapSamplerSun", TEXTURE_SRC_CODE_SHADOWMAP_SUN, NULL, 0, 0 },
  { "shadowmapSamplerSpot", TEXTURE_SRC_CODE_SHADOWMAP_SPOT, NULL, 0, 0 },
  { "feedbackSampler", TEXTURE_SRC_CODE_FEEDBACK, NULL, 0, 0 },
  { "floatZSampler", TEXTURE_SRC_CODE_FLOATZ, NULL, 0, 0 },
  { "processedFloatZSampler", TEXTURE_SRC_CODE_PROCESSED_FLOATZ, NULL, 0, 0 },
  { "rawFloatZSampler", TEXTURE_SRC_CODE_RAW_FLOATZ, NULL, 0, 0 },
  { "featherFloatZSampler", TEXTURE_SRC_CODE_FEATHER_FLOAT_Z, NULL, 0, 0 },
  { "attenuationSampler", TEXTURE_SRC_CODE_LIGHT_ATTENUATION, NULL, 0, 0 },
  {
    "dlightAttenuationSampler",
    TEXTURE_SRC_CODE_DLIGHT_ATTENUATION,
    NULL,
    0,
    0
  },
  { "lightmapSamplerPrimary", TEXTURE_SRC_CODE_LIGHTMAP_PRIMARY, NULL, 0, 0 },
  {
    "lightmapSamplerSecondary",
    TEXTURE_SRC_CODE_LIGHTMAP_SECONDARY,
    NULL,
    0,
    0
  },
  {
    "lightmapSamplerSecondaryB",
    TEXTURE_SRC_CODE_LIGHTMAP_SECONDARYB,
    NULL,
    0,
    0
  },
  { "modelLightingSampler", TEXTURE_SRC_CODE_MODEL_LIGHTING, NULL, 0, 0 },
  { "cinematicYSampler", TEXTURE_SRC_CODE_CINEMATIC_Y, NULL, 0, 0 },
  { "cinematicCrSampler", TEXTURE_SRC_CODE_CINEMATIC_CR, NULL, 0, 0 },
  { "cinematicCbSampler", TEXTURE_SRC_CODE_CINEMATIC_CB, NULL, 0, 0 },
  { "cinematicASampler", TEXTURE_SRC_CODE_CINEMATIC_A, NULL, 0, 0 },
  { "reflectionProbeSampler", TEXTURE_SRC_CODE_REFLECTION_PROBE, NULL, 0, 0 },
  {
    "terrainScorchTextureSampler0",
    TEXTURE_SRC_CODE_TERRAIN_SCORCH_TEXTURE_0,
    NULL,
    0,
    0
  },
  {
    "terrainScorchTextureSampler1",
    TEXTURE_SRC_CODE_TERRAIN_SCORCH_TEXTURE_1,
    NULL,
    0,
    0
  },
  {
    "terrainScorchTextureSampler2",
    TEXTURE_SRC_CODE_TERRAIN_SCORCH_TEXTURE_2,
    NULL,
    0,
    0
  },
  {
    "terrainScorchTextureSampler3",
    TEXTURE_SRC_CODE_TERRAIN_SCORCH_TEXTURE_3,
    NULL,
    0,
    0
  },
  { "impactMaskSampler", TEXTURE_SRC_CODE_IMPACT_MASK, NULL, 0, 0 },
  { "ui3dSampler", TEXTURE_SRC_CODE_UI3D, NULL, 0, 0 },
  { "missileCamSampler", TEXTURE_SRC_CODE_MISSILE_CAM, NULL, 0, 0 },
  { "heatmapSampler", TEXTURE_SRC_CODE_HEATMAP, NULL, 0, 0 },
  { NULL, TEXTURE_SRC_CODE_BLACK, NULL, 0, 0 }
};

const CodeConstantSource s_sunConsts[4] =
{
  { "position", 50u, NULL, 0, 0 },
  { "diffuse", 51u, NULL, 0, 0 },
  { "specular", 52u, NULL, 0, 0 },
  { NULL, 0u, NULL, 0, 0 }
};

const CodeConstantSource s_lightConsts[18] =
{
  { "position", 0u, NULL, 0, 0 },
  { "diffuse", 1u, NULL, 0, 0 },
  { "specular", 2u, NULL, 0, 0 },
  { "spotDir", 3u, NULL, 0, 0 },
  { "spotFactors", 4u, NULL, 0, 0 },
  { "falloffPlacement", 25u, NULL, 0, 0 },
  { "attenuation", 5u, NULL, 0, 0 },
  { "fallOffA", 6u, NULL, 0, 0 },
  { "fallOffB", 7u, NULL, 0, 0 },
  { "spotMatrix0", 8u, NULL, 0, 0 },
  { "spotMatrix1", 9u, NULL, 0, 0 },
  { "spotMatrix2", 10u, NULL, 0, 0 },
  { "spotMatrix3", 11u, NULL, 0, 0 },
  { "spotAABB", 12u, NULL, 0, 0 },
  { "coneControl1", 13u, NULL, 0, 0 },
  { "coneControl2", 14u, NULL, 0, 0 },
  { "spotCookieSlideControl", 15u, NULL, 0, 0 },
  { NULL, 0u, NULL, 0, 0 }
};

const CodeConstantSource s_nearPlaneConsts[4] =
{
  { "org", 16u, NULL, 0, 0 },
  { "dx", 17u, NULL, 0, 0 },
  { "dy", 18u, NULL, 0, 0 },
  { NULL, 0u, NULL, 0, 0 }
};

const CodeConstantSource s_codeConsts[185] =
{
  { "nearPlane", 230u, s_nearPlaneConsts, 0, 0 },
  { "sun", 230u, s_sunConsts, 0, 0 },
  { "light", 230u, s_lightConsts, 0, 0 },
  { "baseLightingCoords", 78u, NULL, 0, 0 },
  { "lightingLookupScale", 53u, NULL, 0, 0 },
  { "debugBumpmap", 54u, NULL, 0, 0 },
  { "pixelCostFracs", 34u, NULL, 0, 0 },
  { "pixelCostDecode", 35u, NULL, 0, 0 },
  { "materialColor", 55u, NULL, 0, 0 },
  { "fogConsts", 56u, NULL, 0, 0 },
  { "fogConsts2", 57u, NULL, 0, 0 },
  { "fogColor", 58u, NULL, 0, 0 },
  { "sunFogColor", 61u, NULL, 0, 0 },
  { "sunFogDir", 60u, NULL, 0, 0 },
  { "sunFog", 59u, NULL, 0, 0 },
  { "glowSetup", 62u, NULL, 0, 0 },
  { "glowApply", 63u, NULL, 0, 0 },
  { "filterTap", 36u, NULL, 8, 1 },
  { "codeMeshArg", 76u, NULL, 2, 1 },
  { "renderTargetSize", 21u, NULL, 0, 0 },
  { "vposx_to_world", 22u, NULL, 0, 0 },
  { "vposy_to_world", 23u, NULL, 0, 0 },
  { "vpos1_to_world", 24u, NULL, 0, 0 },
  { "shadowmapSwitchPartition", 47u, NULL, 0, 0 },
  { "shadowmapScale", 48u, NULL, 0, 0 },
  { "shadowmapPolygonOffset", 20u, NULL, 0, 0 },
  { "shadowParms", 19u, NULL, 0, 0 },
  { "zNear", 49u, NULL, 0, 0 },
  { "clipSpaceLookupScale", 72u, NULL, 0, 0 },
  { "clipSpaceLookupOffset", 73u, NULL, 0, 0 },
  { "dofEquationViewModelAndFarBlur", 26u, NULL, 0, 0 },
  { "dofEquationScene", 27u, NULL, 0, 0 },
  { "dofLerpScale", 28u, NULL, 0, 0 },
  { "dofLerpBias", 29u, NULL, 0, 0 },
  { "dofRowDelta", 30u, NULL, 0, 0 },
  { "depthFromClip", 75u, NULL, 0, 0 },
  { "outdoorFeatherParms", 67u, NULL, 0, 0 },
  { "skyTransition", 68u, NULL, 0, 0 },
  { "envMapParms", 69u, NULL, 0, 0 },
  { "waterParms", 80u, NULL, 0, 0 },
  { "colorMatrixR", 44u, NULL, 0, 0 },
  { "colorMatrixG", 45u, NULL, 0, 0 },
  { "colorMatrixB", 46u, NULL, 0, 0 },
  { "colorBias", 64u, NULL, 0, 0 },
  { "colorTintBase", 65u, NULL, 0, 0 },
  { "colorTintDelta", 66u, NULL, 0, 0 },
  { "gameTime", 32u, NULL, 0, 0 },
  { "alphaFade", 33u, NULL, 0, 0 },
  { "destructibleParms", 114u, NULL, 0, 0 },
  { "particleCloudColor", 31u, NULL, 0, 0 },
  { "particleCloudMatrix", 74u, NULL, 0, 0 },
  { "worldMatrix", 197u, NULL, 0, 0 },
  { "inverseWorldMatrix", 198u, NULL, 0, 0 },
  { "transposeWorldMatrix", 199u, NULL, 0, 0 },
  { "inverseTransposeWorldMatrix", 200u, NULL, 0, 0 },
  { "viewMatrix", 201u, NULL, 0, 0 },
  { "inverseViewMatrix", 202u, NULL, 0, 0 },
  { "transposeViewMatrix", 203u, NULL, 0, 0 },
  { "inverseTransposeViewMatrix", 204u, NULL, 0, 0 },
  { "projectionMatrix", 205u, NULL, 0, 0 },
  { "inverseProjectionMatrix", 206u, NULL, 0, 0 },
  { "transposeProjectionMatrix", 207u, NULL, 0, 0 },
  { "inverseTransposeProjectionMatrix", 208u, NULL, 0, 0 },
  { "worldViewMatrix", 209u, NULL, 0, 0 },
  { "inverseWorldViewMatrix", 210u, NULL, 0, 0 },
  { "transposeWorldViewMatrix", 211u, NULL, 0, 0 },
  { "inverseTransposeWorldViewMatrix", 212u, NULL, 0, 0 },
  { "viewProjectionMatrix", 213u, NULL, 0, 0 },
  { "inverseViewProjectionMatrix", 214u, NULL, 0, 0 },
  { "transposeViewProjectionMatrix", 215u, NULL, 0, 0 },
  { "inverseTransposeViewProjectionMatrix", 216u, NULL, 0, 0 },
  { "worldViewProjectionMatrix", 217u, NULL, 0, 0 },
  { "inverseWorldViewProjectionMatrix", 218u, NULL, 0, 0 },
  { "transposeWorldViewProjectionMatrix", 219u, NULL, 0, 0 },
  { "inverseTransposeWorldViewProjectionMatrix", 220u, NULL, 0, 0 },
  { "shadowLookupMatrix", 221u, NULL, 0, 0 },
  { "inverseShadowLookupMatrix", 222u, NULL, 0, 0 },
  { "transposeShadowLookupMatrix", 223u, NULL, 0, 0 },
  { "inverseTransposeShadowLookupMatrix", 224u, NULL, 0, 0 },
  { "worldOutdoorLookupMatrix", 225u, NULL, 0, 0 },
  { "inverseWorldOutdoorLookupMatrix", 226u, NULL, 0, 0 },
  { "transposeWorldOutdoorLookupMatrix", 227u, NULL, 0, 0 },
  { "inverseTransposeWorldOutdoorLookupMatrix", 228u, NULL, 0, 0 },
  { "windDirection", 79u, NULL, 0, 0 },
  { "variantWindSpring", 98u, NULL, 16, 1 },
  { "u_customWindCenter", 192u, NULL, 0, 0 },
  { "u_customWindSpring", 193u, NULL, 0, 0 },
  { "grassParms", 81u, NULL, 0, 0 },
  { "grassForce0", 82u, NULL, 0, 0 },
  { "grassForce1", 83u, NULL, 0, 0 },
  { "grassWindForce0", 84u, NULL, 0, 0 },
  { "cloudWorldArea", 115u, NULL, 0, 0 },
  { "waterScroll", 116u, NULL, 0, 0 },
  { "motionblurDirectionAndMagnitude", 85u, NULL, 0, 0 },
  { "flameDistortion", 86u, NULL, 0, 0 },
  { "bloomScale", 87u, NULL, 0, 0 },
  { "overlayTexCoord", 88u, NULL, 0, 0 },
  { "colorBias1", 89u, NULL, 0, 0 },
  { "colorTintBase1", 90u, NULL, 0, 0 },
  { "colorTintDelta1", 91u, NULL, 0, 0 },
  { "fadeEffect", 92u, NULL, 0, 0 },
  { "viewportDimensions", 93u, NULL, 0, 0 },
  { "framebufferRead", 94u, NULL, 0, 0 },
  { "resizeParams1", 95u, NULL, 0, 0 },
  { "resizeParams2", 96u, NULL, 0, 0 },
  { "resizeParams3", 97u, NULL, 0, 0 },
  { "crossFadeAlpha", 117u, NULL, 0, 0 },
  { "__characterCharredAmount", 118u, NULL, 0, 0 },
  { "treeCanopyParms", 119u, NULL, 0, 0 },
  { "marksHitNormal", 120u, NULL, 0, 0 },
  { "postFxControl0", 121u, NULL, 0, 0 },
  { "postFxControl1", 122u, NULL, 0, 0 },
  { "postFxControl2", 123u, NULL, 0, 0 },
  { "postFxControl3", 124u, NULL, 0, 0 },
  { "postFxControl4", 125u, NULL, 0, 0 },
  { "postFxControl5", 126u, NULL, 0, 0 },
  { "postFxControl6", 127u, NULL, 0, 0 },
  { "postFxControl7", 128u, NULL, 0, 0 },
  { "postFxControl8", 129u, NULL, 0, 0 },
  { "postFxControl9", 130u, NULL, 0, 0 },
  { "postFxControlA", 131u, NULL, 0, 0 },
  { "postFxControlB", 132u, NULL, 0, 0 },
  { "postFxControlC", 133u, NULL, 0, 0 },
  { "postFxControlD", 134u, NULL, 0, 0 },
  { "postFxControlE", 135u, NULL, 0, 0 },
  { "postFxControlF", 136u, NULL, 0, 0 },
  { "hdrControl0", 137u, NULL, 0, 0 },
  { "hdrControl1", 138u, NULL, 0, 0 },
  { "glightPosXs", 139u, NULL, 0, 0 },
  { "glightPosYs", 140u, NULL, 0, 0 },
  { "glightPosZs", 141u, NULL, 0, 0 },
  { "glightFallOffs", 142u, NULL, 0, 0 },
  { "glightReds", 143u, NULL, 0, 0 },
  { "glightGreens", 144u, NULL, 0, 0 },
  { "glightBlues", 145u, NULL, 0, 0 },
  { "dlightPosition", 146u, NULL, 0, 0 },
  { "dlightDiffuse", 147u, NULL, 0, 0 },
  { "dlightSpecular", 148u, NULL, 0, 0 },
  { "dlightAttenuation", 149u, NULL, 0, 0 },
  { "dlightFallOff", 150u, NULL, 0, 0 },
  { "dlightSpotMatrix0", 151u, NULL, 0, 0 },
  { "dlightSpotMatrix1", 152u, NULL, 0, 0 },
  { "dlightSpotMatrix2", 153u, NULL, 0, 0 },
  { "dlightSpotMatrix3", 154u, NULL, 0, 0 },
  { "dlightSpotDir", 155u, NULL, 0, 0 },
  { "dlightSpotFactors", 156u, NULL, 0, 0 },
  { "dlightShadowLookupMatrix0", 157u, NULL, 0, 0 },
  { "dlightShadowLookupMatrix1", 158u, NULL, 0, 0 },
  { "dlightShadowLookupMatrix2", 159u, NULL, 0, 0 },
  { "dlightShadowLookupMatrix3", 160u, NULL, 0, 0 },
  { "cloudLayerControl0", 161u, NULL, 0, 0 },
  { "cloudLayerControl1", 162u, NULL, 0, 0 },
  { "cloudLayerControl2", 163u, NULL, 0, 0 },
  { "cloudLayerControl3", 164u, NULL, 0, 0 },
  { "cloudLayerControl4", 165u, NULL, 0, 0 },
  { "heroLightingR", 166u, NULL, 0, 0 },
  { "heroLightingG", 167u, NULL, 0, 0 },
  { "heroLightingB", 168u, NULL, 0, 0 },
  { "lightHeroScale", 169u, NULL, 0, 0 },
  { "cinematicBlurBox", 170u, NULL, 0, 0 },
  { "cinematicBlurBox2", 171u, NULL, 0, 0 },
  { "adsZScale", 172u, NULL, 0, 0 },
  { "ui3dUVSetup0", 173u, NULL, 0, 0 },
  { "ui3dUVSetup1", 174u, NULL, 0, 0 },
  { "ui3dUVSetup2", 175u, NULL, 0, 0 },
  { "ui3dUVSetup3", 176u, NULL, 0, 0 },
  { "ui3dUVSetup4", 177u, NULL, 0, 0 },
  { "ui3dUVSetup5", 178u, NULL, 0, 0 },
  { "__characterDissolveColor", 179u, NULL, 0, 0 },
  { "cameraLook", 180u, NULL, 0, 0 },
  { "cameraUp", 181u, NULL, 0, 0 },
  { "cameraSide", 182u, NULL, 0, 0 },
  { "scriptVector0", 183u, NULL, 0, 0 },
  { "scriptVector1", 184u, NULL, 0, 0 },
  { "scriptVector2", 185u, NULL, 0, 0 },
  { "scriptVector3", 186u, NULL, 0, 0 },
  { "scriptVector4", 187u, NULL, 0, 0 },
  { "scriptVector5", 188u, NULL, 0, 0 },
  { "scriptVector6", 189u, NULL, 0, 0 },
  { "scriptVector7", 190u, NULL, 0, 0 },
  { "eyeOffset", 191u, NULL, 0, 0 },
  { "skyColorMultiplier", 194u, NULL, 0, 0 },
  { "extraCamParam", 195u, NULL, 0, 0 },
  { "emblemLUTSelector", 196u, NULL, 0, 0 },
  { NULL, 0u, NULL, 0, 0 }
};

const CodeConstantSource s_defaultCodeConsts[26] =
{
  { "nearPlaneOrg", 16u, NULL, 0, 0 },
  { "nearPlaneDx", 17u, NULL, 0, 0 },
  { "nearPlaneDy", 18u, NULL, 0, 0 },
  { "sunPosition", 50u, NULL, 0, 0 },
  { "sunDiffuse", 51u, NULL, 0, 0 },
  { "sunSpecular", 52u, NULL, 0, 0 },
  { "lightPosition", 0u, NULL, 0, 0 },
  { "lightDiffuse", 1u, NULL, 0, 0 },
  { "lightSpecular", 2u, NULL, 0, 0 },
  { "lightSpotDir", 3u, NULL, 0, 0 },
  { "lightSpotFactors", 4u, NULL, 0, 0 },
  { "lightFalloffPlacement", 25u, NULL, 0, 0 },
  { "lightAttenuation", 5u, NULL, 0, 0 },
  { "lightFallOffA", 6u, NULL, 0, 0 },
  { "lightFallOffB", 7u, NULL, 0, 0 },
  { "lightSpotMatrix0", 8u, NULL, 0, 0 },
  { "lightSpotMatrix1", 9u, NULL, 0, 0 },
  { "lightSpotMatrix2", 10u, NULL, 0, 0 },
  { "lightSpotMatrix3", 11u, NULL, 0, 0 },
  { "lightSpotAABB", 12u, NULL, 0, 0 },
  { "lightConeControl1", 13u, NULL, 0, 0 },
  { "lightConeControl2", 14u, NULL, 0, 0 },
  { "lightSpotCookieSlideControl", 15u, NULL, 0, 0 },
  { "spotShadowmapPixelAdjust", 70u, NULL, 0, 0 },
  { "dlightSpotShadowmapPixelAdjust", 71u, NULL, 0, 0 },
  { NULL, 0u, NULL, 0, 0 }
};

const MtlStateMapBitName s_alphaTestBitNames[5] =
{
  { "Always", 2048 },
  { "GE128", 12288 },
  { "GT0", 4096 },
  { "GE255", 8192 },
  { NULL, 0 }
};

const MtlStateMapBitName s_blendOpRgbBitNames[7] =
{
  { "Disable", 0 },
  { "Add", 256 },
  { "Subtract", 512 },
  { "RevSubtract", 768 },
  { "Min", 1024 },
  { "Max", 1280 },
  { NULL, 0 }
};

const MtlStateMapBitName s_srcBlendRgbBitNames[11] =
{
  { "Zero", 1 },
  { "One", 2 },
  { "SrcColor", 3 },
  { "InvSrcColor", 4 },
  { "SrcAlpha", 5 },
  { "InvSrcAlpha", 6 },
  { "DestAlpha", 7 },
  { "InvDestAlpha", 8 },
  { "DestColor", 9 },
  { "InvDestColor", 10 },
  { NULL, 0 }
};

const MtlStateMapBitName s_dstBlendRgbBitNames[11] =
{
  { "Zero", 16 },
  { "One", 32 },
  { "SrcColor", 48 },
  { "InvSrcColor", 64 },
  { "SrcAlpha", 80 },
  { "InvSrcAlpha", 96 },
  { "DestAlpha", 112 },
  { "InvDestAlpha", 128 },
  { "DestColor", 144 },
  { "InvDestColor", 160 },
  { NULL, 0 }
};

const MtlStateMapBitName s_blendOpAlphaBitNames[7] =
{
  { "Disable", 0 },
  { "Add", 16777216 },
  { "Subtract", 33554432 },
  { "RevSubtract", 50331648 },
  { "Min", 67108864 },
  { "Max", 83886080 },
  { NULL, 0 }
};

const MtlStateMapBitName s_srcBlendAlphaBitNames[11] =
{
  { "Zero", 65536 },
  { "One", 131072 },
  { "SrcColor", 196608 },
  { "InvSrcColor", 262144 },
  { "SrcAlpha", 327680 },
  { "InvSrcAlpha", 393216 },
  { "DestAlpha", 458752 },
  { "InvDestAlpha", 524288 },
  { "DestColor", 589824 },
  { "InvDestColor", 655360 },
  { NULL, 0 }
};

const MtlStateMapBitName s_dstBlendAlphaBitNames[11] =
{
  { "Zero", 1048576 },
  { "One", 2097152 },
  { "SrcColor", 3145728 },
  { "InvSrcColor", 4194304 },
  { "SrcAlpha", 5242880 },
  { "InvSrcAlpha", 6291456 },
  { "DestAlpha", 7340032 },
  { "InvDestAlpha", 8388608 },
  { "DestColor", 9437184 },
  { "InvDestColor", 10485760 },
  { NULL, 0 }
};

const MtlStateMapBitName s_cullFaceBitNames[4] =
{ { "None", 16384 }, { "Back", 32768 }, { "Front", 49152 }, { NULL, 0 } };

const MtlStateMapBitName s_depthTestBitNames[6] =
{
  { "Disable", 2 },
  { "Less", 4 },
  { "LessEqual", 12 },
  { "Equal", 8 },
  { "Always", 0 },
  { NULL, 0 }
};

const MtlStateMapBitName s_depthWriteBitNames[5] =
{ { "Enable", 1 }, { "Disable", 0 }, { "On", 1 }, { "Off", 0 }, { NULL, 0 } };

const MtlStateMapBitName s_colorWriteRgbBitNames[3] =
{ { "Enable", 134217728 }, { "Disable", 0 }, { NULL, 0 } };

const MtlStateMapBitName s_colorWriteAlphaBitNames[3] =
{ { "Enable", 268435456 }, { "Disable", 0 }, { NULL, 0 } };

const MtlStateMapBitName s_polygonOffsetBitNames[5] =
{ { "0", 0 }, { "1", 16 }, { "2", 32 }, { "shadowmap", 48 }, { NULL, 0 } };

const MtlStateMapBitName s_wireframeBitNames[3] =
{ { "Enable", 2147483648 }, { "Disable", 0 }, { NULL, 0 } };

const MtlStateMapBitName s_stencilBitNames[4] =
{ { "Disable", 0 }, { "OneSided", 64 }, { "TwoSided", 192 }, { NULL, 0 } };

const MtlStateMapBitName s_stencilOpFrontPassBitNames[9] =
{
  { "Keep", 0 },
  { "Zero", 256 },
  { "Replace", 512 },
  { "IncrSat", 768 },
  { "DecrSat", 1024 },
  { "Invert", 1280 },
  { "Incr", 1536 },
  { "Decr", 1792 },
  { NULL, 0 }
};

const MtlStateMapBitName s_stencilOpFrontFailBitNames[9] =
{
  { "Keep", 0 },
  { "Zero", 2048 },
  { "Replace", 4096 },
  { "IncrSat", 6144 },
  { "DecrSat", 8192 },
  { "Invert", 10240 },
  { "Incr", 12288 },
  { "Decr", 14336 },
  { NULL, 0 }
};

const MtlStateMapBitName s_stencilOpFrontZFailBitNames[9] =
{
  { "Keep", 0 },
  { "Zero", 16384 },
  { "Replace", 32768 },
  { "IncrSat", 49152 },
  { "DecrSat", 65536 },
  { "Invert", 81920 },
  { "Incr", 98304 },
  { "Decr", 114688 },
  { NULL, 0 }
};

const MtlStateMapBitName s_stencilFuncFrontBitNames[9] =
{
  { "Never", 0 },
  { "Less", 131072 },
  { "Equal", 262144 },
  { "LessEqual", 393216 },
  { "Greater", 524288 },
  { "NotEqual", 655360 },
  { "GreaterEqual", 786432 },
  { "Always", 917504 },
  { NULL, 0 }
};

const MtlStateMapBitName s_stencilOpBackPassBitNames[9] =
{
  { "Keep", 0 },
  { "Zero", 1048576 },
  { "Replace", 2097152 },
  { "IncrSat", 3145728 },
  { "DecrSat", 4194304 },
  { "Invert", 5242880 },
  { "Incr", 6291456 },
  { "Decr", 7340032 },
  { NULL, 0 }
};

const MtlStateMapBitName s_stencilOpBackFailBitNames[9] =
{
  { "Keep", 0 },
  { "Zero", 8388608 },
  { "Replace", 16777216 },
  { "IncrSat", 25165824 },
  { "DecrSat", 33554432 },
  { "Invert", 41943040 },
  { "Incr", 50331648 },
  { "Decr", 58720256 },
  { NULL, 0 }
};

const MtlStateMapBitName s_stencilOpBackZFailBitNames[9] =
{
  { "Keep", 0 },
  { "Zero", 67108864 },
  { "Replace", 134217728 },
  { "IncrSat", 201326592 },
  { "DecrSat", 268435456 },
  { "Invert", 335544320 },
  { "Incr", 402653184 },
  { "Decr", 469762048 },
  { NULL, 0 }
};

const MtlStateMapBitName s_stencilFuncBackBitNames[9] =
{
  { "Never", 0 },
  { "Less", 536870912 },
  { "Equal", 1073741824 },
  { "LessEqual", 1610612736 },
  { "Greater", 2147483648 },
  { "NotEqual", -1610612736 },
  { "GreaterEqual", -1073741824 },
  { "Always", -536870912 },
  { NULL, 0 }
};

const MtlStateMapBitGroup s_stateMapSrcBitGroup[23] =
{
  { "mtlAlphaTest", s_alphaTestBitNames, { 14336, 0 } },
  { "mtlBlendOp", s_blendOpRgbBitNames, { 1792, 0 } },
  { "mtlSrcBlend", s_srcBlendRgbBitNames, { 15, 0 } },
  { "mtlDestBlend", s_dstBlendRgbBitNames, { 240, 0 } },
  { "mtlBlendOpAlpha", s_blendOpAlphaBitNames, { 117440512, 0 } },
  { "mtlSrcBlendAlpha", s_srcBlendAlphaBitNames, { 983040, 0 } },
  { "mtlDestBlendAlpha", s_dstBlendAlphaBitNames, { 15728640, 0 } },
  { "mtlCullFace", s_cullFaceBitNames, { 49152, 0 } },
  { "mtlColorWriteRgb", s_colorWriteRgbBitNames, { 134217728, 0 } },
  { "mtlColorWriteAlpha", s_colorWriteAlphaBitNames, { 268435456, 0 } },
  { "mtlDepthTest", s_depthTestBitNames, { 0, 14 } },
  { "mtlDepthWrite", s_depthWriteBitNames, { 0, 1 } },
  { "mtlPolygonOffset", s_polygonOffsetBitNames, { 0, 48 } },
  { "mtlStencil", s_stencilBitNames, { 0, 192 } },
  { "mtlStencilFuncFront", s_stencilFuncFrontBitNames, { 0, 917504 } },
  { "mtlStencilOpFrontPass", s_stencilOpFrontPassBitNames, { 0, 1792 } },
  { "mtlStencilOpFrontFail", s_stencilOpFrontFailBitNames, { 0, 14336 } },
  { "mtlStencilOpFrontZFail", s_stencilOpFrontZFailBitNames, { 0, 114688 } },
  { "mtlStencilFuncBack", s_stencilFuncBackBitNames, { 0, 917504 } },
  { "mtlStencilOpBackPass", s_stencilOpBackPassBitNames, { 0, 1792 } },
  { "mtlStencilOpBackFail", s_stencilOpBackFailBitNames, { 0, 14336 } },
  { "mtlStencilOpBackZFail", s_stencilOpBackZFailBitNames, { 0, 114688 } },
  { NULL, NULL, { 0, 0 } }
};

const MtlStateMapBitGroup s_stateMapDstAlphaTestBitGroup[2] =
{
  { "alphaTest", s_alphaTestBitNames, { 14336, 0 } },
  { NULL, NULL, { 0, 0 } }
};

const MtlStateMapBitGroup s_stateMapDstBlendFuncRgbBitGroup[4] =
{
  { "blendFuncRgb", s_blendOpRgbBitNames, { 1792, 0 } },
  { "blendFuncRgb", s_srcBlendRgbBitNames, { 1807, 0 } },
  { "blendFuncRgb", s_dstBlendRgbBitNames, { 2032, 0 } },
  { NULL, NULL, { 0, 0 } }
};

const MtlStateMapBitGroup s_stateMapDstBlendFuncAlphaBitGroup[4] =
{
  { "blendFuncAlpha", s_blendOpAlphaBitNames, { 117440512, 0 } },
  { "blendFuncAlpha", s_srcBlendAlphaBitNames, { 983040, 0 } },
  { "blendFuncAlpha", s_dstBlendAlphaBitNames, { 15728640, 0 } },
  { NULL, NULL, { 0, 0 } }
};

const MtlStateMapBitGroup s_stateMapDstCullFaceBitGroup[2] =
{ { "cullFace", s_cullFaceBitNames, { 49152, 0 } }, { NULL, NULL, { 0, 0 } } };

const MtlStateMapBitGroup s_stateMapDstDepthTestBitGroup[2] =
{ { "depthTest", s_depthTestBitNames, { 0, 14 } }, { NULL, NULL, { 0, 0 } } };

const MtlStateMapBitGroup s_stateMapDstDepthWriteBitGroup[2] =
{ { "depthWrite", s_depthWriteBitNames, { 0, 1 } }, { NULL, NULL, { 0, 0 } } };

const MtlStateMapBitGroup s_stateMapDstColorWriteBitGroup[3] =
{
  { "colorWrite", s_colorWriteRgbBitNames, { 134217728, 0 } },
  { "colorWrite", s_colorWriteAlphaBitNames, { 268435456, 0 } },
  { NULL, NULL, { 0, 0 } }
};

const MtlStateMapBitGroup s_stateMapDstPolygonOffsetBitGroup[2] =
{
  { "polygonOffset", s_polygonOffsetBitNames, { 0, 48 } },
  { NULL, NULL, { 0, 0 } }
};

const MtlStateMapBitGroup s_stateMapDstWireframeBitGroup[2] =
{
  { "wireframe", s_wireframeBitNames, { 2147483648, 0 } },
  { NULL, NULL, { 0, 0 } }
};

const MtlStateMapBitGroup s_stateMapDstStencilBitGroup[10] =
{
  { "stencil", s_stencilBitNames, { 0, 192 } },
  { "stencil", s_stencilFuncFrontBitNames, { 0, 917504 } },
  { "stencil", s_stencilOpFrontPassBitNames, { 0, 1792 } },
  { "stencil", s_stencilOpFrontFailBitNames, { 0, 14336 } },
  { "stencil", s_stencilOpFrontZFailBitNames, { 0, 114688 } },
  { "stencil", s_stencilFuncBackBitNames, { 0, 917504 } },
  { "stencil", s_stencilOpBackPassBitNames, { 0, 1792 } },
  { "stencil", s_stencilOpBackFailBitNames, { 0, 14336 } },
  { "stencil", s_stencilOpBackZFailBitNames, { 0, 114688 } },
  { NULL, NULL, { 0, 0 } }
};

const MaterialWorldVertexFormat s_worldVertFormatForLayerCount[4] =
{
  MTL_WORLDVERT_TEX_1_NRM_1,
  MTL_WORLDVERT_TEX_2_NRM_1,
  MTL_WORLDVERT_TEX_3_NRM_1,
  MTL_WORLDVERT_TEX_4_NRM_1
};


// *WARNING* One or more selections were skipped as they could not be interpreted as c data



unsigned int g_customSamplerSrc[4] = { 27u, 4u, 5u, 33u };
unsigned int g_customSamplerDest[4] = { 15u, 12u, 13u, 14u };

ScriptableConstant g_scriptableConstantArray[13] =
{
  { 118u, NULL },
  { 179u, NULL },
  { 183u, NULL },
  { 184u, NULL },
  { 185u, NULL },
  { 186u, NULL },
  { 187u, NULL },
  { 188u, NULL },
  { 189u, NULL },
  { 190u, NULL },
  { 166u, NULL },
  { 167u, NULL },
  { 168u, NULL }
};



struct //$54435CF730F84DB67694F69169881761 // sizeof=0x2CE28
{                                       // XREF: .data:mtlLoadGlob/r
    unsigned int cachedShaderCount;     // XREF: Material_FindCachedShaderText+D/r
                                        // Material_PreLoadAllShaderText(void)+6F/w ...
    GfxCachedShaderText *cachedShaderText;
                                        // XREF: Material_FindCachedShaderText+31/r
                                        // Material_FindCachedShaderText+59/r ...
    unsigned int vertexDeclCount;       // XREF: Material_FreeAll(void)+60/w
                                        // Material_AllocVertexDecl:loc_ACE889/r ...
    MaterialVertexDeclaration vertexDeclHashTable[64];
                                        // XREF: Material_FreeAll(void)+53/o
                                        // Material_AllocVertexDecl+4D/o ...
    unsigned int literalCount;          // XREF: Material_FreeAllLiterals+3/w
                                        // Material_RegisterLiteral+1B/r ...
    float literalTable[32][4];          // XREF: Material_RegisterLiteral+2D/o
                                        // Material_RegisterLiteral+A4/o ...
    unsigned int stringCount;           // XREF: Material_FreeAllStrings+17/w
                                        // Material_RegisterString:loc_AD2296/r ...
    MaterialString stringHashTable[1024];
                                        // XREF: Material_FreeAllStrings+A/o
                                        // Material_RegisterString+24/r ...
    unsigned int vertexShaderCount;     // XREF: Material_FreeAll(void)+7E/w
                                        // Material_RegisterVertexShader+CC/r ...
    MaterialVertexShader *vertexShaderHashTable[5120];
                                        // XREF: Material_FreeAll(void)+71/o
                                        // Material_RegisterVertexShader+24/r ...
    unsigned int pixelShaderCount;      // XREF: Material_FreeAll(void)+9C/w
                                        // Material_RegisterPixelShader+CC/r ...
    MaterialPixelShader *pixelShaderHashTable[20480];
                                        // XREF: Material_FreeAll(void)+8F/o
                                        // Material_RegisterPixelShader+24/r ...
    unsigned int stateMapCount;         // XREF: Material_FreeAllStateMaps+17/w
    MaterialStateMap *stateMapHashTable[64];
                                        // XREF: Material_FreeAllStateMaps+A/o
                                        // Material_FindStateMap+22/r ...
    unsigned int techniqueCount;        // XREF: Material_FreeAll(void)+42/w
                                        // Material_SetTechnique+4/r ...
    MaterialTechnique *techniqueHashTable[16384];
                                        // XREF: Material_FreeAll(void)+35/o
                                        // Material_FindTechnique+4E/r ...
    const MaterialRaw *sortMtlRaw;      // XREF: Material_FinishLoadingInstance+D1/w
} mtlLoadGlob;

bool gIsPIMPEnabled;
bool gIsSW2Material;
bool gCheckedForPimp;

bool __cdecl Material_CachedShaderTextLess(const GfxCachedShaderText &cached0, const GfxCachedShaderText &cached1)
{
    return I_stricmp(cached0.name, cached1.name) < 0;
}

void __cdecl Material_FreeAll()
{
    Material_FreeAllLiterals();
    Material_FreeAllStrings();
    Material_FreeAllStateMaps();
    Material_FreeAllTechniqueSets();
    if ( !useFastFile->current.enabled )
    {
        memset(mtlLoadGlob.techniqueHashTable, 0, sizeof(mtlLoadGlob.techniqueHashTable));
        mtlLoadGlob.techniqueCount = 0;
        memset(&mtlLoadGlob.vertexDeclHashTable[0].streamCount, 0, 0x1B00u);
        mtlLoadGlob.vertexDeclCount = 0;
        memset(mtlLoadGlob.vertexShaderHashTable, 0, sizeof(mtlLoadGlob.vertexShaderHashTable));
        mtlLoadGlob.vertexShaderCount = 0;
        memset(mtlLoadGlob.pixelShaderHashTable, 0, sizeof(mtlLoadGlob.pixelShaderHashTable));
        mtlLoadGlob.pixelShaderCount = 0;
    }
}

void Material_FreeAllLiterals()
{
    mtlLoadGlob.literalCount = 0;
}

void Material_FreeAllStrings()
{
    memset(mtlLoadGlob.stringHashTable, 0, sizeof(mtlLoadGlob.stringHashTable));
    mtlLoadGlob.stringCount = 0;
}

void Material_FreeAllStateMaps()
{
    memset(mtlLoadGlob.stateMapHashTable, 0, sizeof(mtlLoadGlob.stateMapHashTable));
    mtlLoadGlob.stateMapCount = 0;
}

MaterialTechniqueSet *__cdecl Material_LoadTechniqueSet(char *name)
{
    char *v1; // eax
    char *v2; // eax
    char *v3; // eax
    unsigned int v5; // [esp+0h] [ebp-1ECh]
    bool v6; // [esp+10h] [ebp-1DCh]
    bool v7; // [esp+18h] [ebp-1D4h]
    MaterialTechnique *technique; // [esp+30h] [ebp-1BCh]
    int techTypeCount; // [esp+34h] [ebp-1B8h]
    char filename[260]; // [esp+3Ch] [ebp-1B0h] BYREF
    int techTypeIndex; // [esp+140h] [ebp-ACh]
    unsigned __int8 techType[136]; // [esp+144h] [ebp-A8h]
    bool usingTechnique; // [esp+1D3h] [ebp-19h]
    int nameSize; // [esp+1D4h] [ebp-18h]
    int fileSize; // [esp+1D8h] [ebp-14h]
    void *file; // [esp+1DCh] [ebp-10h] BYREF
    const char *token; // [esp+1E0h] [ebp-Ch]
    MaterialTechniqueSet *techniqueSet; // [esp+1E4h] [ebp-8h]
    const char *text; // [esp+1E8h] [ebp-4h] BYREF

    if ( !gCheckedForPimp )
    {
        v7 = getenv("IW_PIMP") && !strcmp(getenv("IW_PIMP"), "1");
        gIsPIMPEnabled = v7;
        if ( v7 )
            Com_Printf(0, "^0PIMP (%s)\n", "enabled");
        else
            Com_Printf(0, "^0PIMP (%s)\n", "disabled");
        gCheckedForPimp = 1;
    }
    v1 = strstr(name, "sw_");
    v6 = 1;
    if ( !v1 )
    {
        v2 = strstr(name, "swb_");
        if ( !v2 )
        {
            v3 = strstr(name, "swl_");
            if ( !v3 )
                v6 = 0;
        }
    }
    gIsSW2Material = v6;
    if ( !gIsPIMPEnabled || gIsSW2Material )
        Com_sprintf(filename, 0x100u, "techsets/%s.techset", name);
    else
        Com_sprintf(filename, 0x100u, "pimp/techsets/%s.techset", name);
    fileSize = FS_ReadFile(filename, &file);
    if ( fileSize >= 0 )
    {
        v5 = strlen(name);
        nameSize = v5 + 1;
        techniqueSet = (MaterialTechniqueSet *)Material_Alloc(v5 + 529);
        techniqueSet->name = (const char *)&techniqueSet[1];
        techniqueSet->worldVertFormat = 0;
        memcpy((unsigned __int8 *)techniqueSet->name, (unsigned __int8 *)name, nameSize);
        text = (const char *)file;
        Com_BeginParseSession(filename);
        Com_SetScriptWarningPrefix("^1ERROR: ");
        Com_SetSpaceDelimited(0);
        Com_SetKeepStringQuotes(1);
        techTypeCount = 0;
        usingTechnique = 0;
        while ( 1 )
        {
            token = (const char *)Com_Parse(&text);
            if ( !*token )
                break;
            if ( *token == 34 )
            {
                if ( techTypeCount == 130 )
                {
                    Com_ScriptError("Too many labels in technique set\n");
                    techniqueSet = 0;
                    break;
                }
                if ( !Material_IgnoreTechnique(token) )
                {
                    techType[techTypeCount] = Material_TechniqueTypeForName(token);
                    if ( techType[techTypeCount] == 130 )
                        goto LABEL_28;
                    if ( Material_UsingTechnique(techType[techTypeCount]) )
                        usingTechnique = 1;
                    ++techTypeCount;
                }
                if ( !Material_MatchToken(&text, ":") )
                {
                    techniqueSet = 0;
                    break;
                }
            }
            else
            {
                if ( usingTechnique )
                {
                    if ( !techTypeCount )
                    {
LABEL_28:
                        Com_ScriptError("Unknown technique type '%s'\n", token);
                        techniqueSet = 0;
                        break;
                    }
                    technique = Material_RegisterTechnique((char *)token);
                    if ( !technique )
                    {
                        Com_ScriptError("Couldn't register technique '%s'\n", token);
                        techniqueSet = 0;
                        break;
                    }
                    for ( techTypeIndex = 0; techTypeIndex < techTypeCount; ++techTypeIndex )
                        techniqueSet->techniques[techType[techTypeIndex]] = technique;
                }
                techTypeCount = 0;
                usingTechnique = 0;
                if ( !Material_MatchToken(&text, ";") )
                {
                    techniqueSet = 0;
                    break;
                }
            }
        }
        Com_EndParseSession();
        FS_FreeFile(file);
        return techniqueSet;
    }
    else
    {
        Com_PrintError(8, "^1ERROR: Couldn't open techniqueSet '%s'\n", filename);
        return 0;
    }
}

bool __cdecl Material_UsingTechnique(unsigned int techType)
{
    if ( techType >= 0x82
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    2820,
                    0,
                    "techType doesn't index TECHNIQUE_COUNT\n\t%i not in [0, %i)",
                    techType,
                    130) )
    {
        __debugbreak();
    }
    return g_useTechnique[techType];
}

bool __cdecl Material_MatchToken(const char **text, const char *match)
{
    return Com_MatchToken(text, match, 1) != 0;
}

MaterialTechnique *__cdecl Material_RegisterTechnique(char *name)
{
    MaterialTechnique *technique; // [esp+0h] [ebp-4h]
    MaterialTechnique *techniquea; // [esp+0h] [ebp-4h]

    technique = Material_FindTechnique(name);
    if ( technique )
        return technique;
    ProfLoad_Begin("Load technique");
    techniquea = Material_LoadTechnique(name);
    ProfLoad_End();
    if ( techniquea )
        Material_SetTechnique(name, techniquea);
    return techniquea;
}

MaterialTechnique *__cdecl Material_FindTechnique(const char *name)
{
    unsigned int hashIndex; // [esp+0h] [ebp-4h] BYREF

    if ( !name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp", 2342, 0, "%s", "name") )
    {
        __debugbreak();
    }
    if ( MaterialTechnique_FindHashLocation(name, &hashIndex) )
        return mtlLoadGlob.techniqueHashTable[hashIndex];
    else
        return 0;
}

char __cdecl MaterialTechnique_FindHashLocation(const char *name, unsigned int *foundHashIndex)
{
    unsigned int hashIndex; // [esp+0h] [ebp-8h]

    for ( hashIndex = R_HashAssetName(name) & 0x3FFF;
                mtlLoadGlob.techniqueHashTable[hashIndex];
                hashIndex = ((_WORD)hashIndex + 1) & 0x3FFF )
    {
        if ( !I_stricmp(mtlLoadGlob.techniqueHashTable[hashIndex]->name, name) )
        {
            *foundHashIndex = hashIndex;
            return 1;
        }
    }
    *foundHashIndex = hashIndex;
    return 0;
}

void __cdecl Material_SetTechnique(const char *name, MaterialTechnique *technique)
{
    unsigned int hashIndex; // [esp+0h] [ebp-4h] BYREF

    if ( mtlLoadGlob.techniqueCount == 0x3FFF )
        Com_Error(ERR_DROP, "More than %i techniques in use", 0x3FFF);
    MaterialTechnique_FindHashLocation(name, &hashIndex);
    ++mtlLoadGlob.techniqueCount;
    mtlLoadGlob.techniqueHashTable[hashIndex] = technique;
}

MaterialTechnique *__cdecl Material_LoadTechnique(char *name)
{
    unsigned __int8 *technique; // [esp+24h] [ebp-1A0h]
    int stateMapSize; // [esp+28h] [ebp-19Ch]
    MaterialStateMap *stateMap[4]; // [esp+2Ch] [ebp-198h] BYREF
    char filename[260]; // [esp+3Ch] [ebp-188h] BYREF
    MaterialVertexDeclaration *vertexDecl; // [esp+144h] [ebp-80h]
    unsigned int nameSize; // [esp+148h] [ebp-7Ch]
    unsigned __int16 techFlags; // [esp+14Ch] [ebp-78h] BYREF
    int fileSize; // [esp+150h] [ebp-74h]
    void *file; // [esp+154h] [ebp-70h] BYREF
    bool error; // [esp+15Bh] [ebp-69h]
    MaterialPass passes[4]; // [esp+15Ch] [ebp-68h] BYREF
    const char *token; // [esp+1ACh] [ebp-18h]
    MaterialStateMap **stateMapForPass; // [esp+1B0h] [ebp-14h]
    unsigned __int16 passCount; // [esp+1B4h] [ebp-10h]
    const char *formatString; // [esp+1B8h] [ebp-Ch]
    const char *text; // [esp+1BCh] [ebp-8h] BYREF
    int passIndex; // [esp+1C0h] [ebp-4h]

    if ( !gIsPIMPEnabled || gIsSW2Material )
    {
        formatString = "techniques/%s.tech";
        Com_sprintf(filename, 0x100u, "techniques/%s.tech", name);
    }
    else
    {
        formatString = "pimp/techniques/%s.tech";
        Com_sprintf(filename, 0x100u, "pimp/techniques/%s.tech", name);
    }
    fileSize = FS_ReadFile(filename, &file);
    if ( fileSize >= 0 )
    {
        text = (const char *)file;
        Com_BeginParseSession(filename);
        Com_SetScriptWarningPrefix("^1ERROR: ");
        Com_SetSpaceDelimited(0);
        error = 0;
        techFlags = 0;
        for ( passCount = 0; passCount < 4u; ++passCount )
        {
            token = (const char *)Com_Parse(&text);
            if ( !*token )
                break;
            if ( *token != 123 )
            {
                Com_ScriptError("expected '{' but found '%s'\n", token);
                error = 1;
                break;
            }
            if ( !Material_LoadPass((char **)&text, &techFlags, &passes[passCount], &stateMap[passCount]) )
            {
                Com_ScriptError("Error loading pass for dx9 technique '%s'\n", name);
                error = 1;
                break;
            }
            if ( !Material_MatchToken(&text, "}") )
            {
                error = 1;
                break;
            }
        }
        Com_EndParseSession();
        FS_FreeFile(file);
        if ( error )
        {
            return 0;
        }
        else if ( passCount )
        {
            stateMapSize = 4 * passCount;
            nameSize = strlen(name) + 1;
            technique = Material_Alloc(nameSize + 24 * passCount + 8);
            stateMapForPass = (MaterialStateMap **)&technique[20 * passCount + 8];
            *(unsigned int *)technique = (unsigned int)&stateMapForPass[passCount];
            memcpy(*(unsigned __int8 **)technique, (unsigned __int8 *)name, nameSize);

            *((_WORD *)technique + 2) = techFlags;
            if ( !strcmp(*(const char **)technique, "zprepass")
                || !strncmp(*(const char **)technique, "pimp_technique_zprepass_", 0x18u)
                || !strncmp(*(const char **)technique, "pimp_technique_layer_zprepass_", 0x1Eu) )
            {
                *((_WORD *)technique + 2) |= 4u;
            }
            for ( passIndex = 0; passIndex < passCount; ++passIndex )
            {
                vertexDecl = passes[passIndex].vertexDecl;
                if ( !vertexDecl
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                                8604,
                                0,
                                "%s",
                                "vertexDecl") )
                {
                    __debugbreak();
                }
                if ( vertexDecl->hasOptionalSource )
                {
                    *((_WORD *)technique + 2) |= 8u;
                    break;
                }
            }
            *((_WORD *)technique + 3) = passCount;
            memcpy(technique + 8, (unsigned __int8 *)passes, 20 * passCount);
            memcpy((unsigned __int8 *)stateMapForPass, (unsigned __int8 *)stateMap, stateMapSize);
            return (MaterialTechnique *)technique;
        }
        else
        {
            Com_ScriptError(
                "Technique '%s' has no passes.    The technique should be left blank in the techniqueset instead.\n",
                name);
            return 0;
        }
    }
    else
    {
        Com_ScriptError("Couldn't open technique '%s'\n", filename);
        return 0;
    }
}

char __cdecl Material_LoadPass(
                char **text,
                unsigned __int16 *techFlags,
                MaterialPass *pass,
                MaterialStateMap **stateMap)
{
    char *v5; // eax
    MaterialShaderArgument *v6; // [esp+14h] [ebp-6550h]
    int j; // [esp+1Ch] [ebp-6548h]
    int k; // [esp+1Ch] [ebp-6548h]
    unsigned int updated; // [esp+20h] [ebp-6544h]
    ShaderParameterSet v10; // [esp+24h] [ebp-6540h] BYREF
    char PassVertexDecl; // [esp+30BBh] [ebp-34A9h]
    ShaderParameterSet paramSet; // [esp+30BCh] [ebp-34A8h] BYREF
    unsigned int argCount; // [esp+6150h] [ebp-414h] BYREF
    unsigned int firstArg; // [esp+6154h] [ebp-410h] BYREF
    unsigned int v15; // [esp+6158h] [ebp-40Ch]
    unsigned int i; // [esp+615Ch] [ebp-408h]
    MaterialShaderArgument *localArgs; // [esp+6160h] [ebp-404h]
    MaterialShaderArgument args[128]; // [esp+6164h] [ebp-400h] BYREF

    PassVertexDecl = 1;
    pass->vertexDecl = 0;
    pass->vertexShader = 0;
    pass->pixelShader = 0;
    *(unsigned int *)&pass->perPrimArgCount = 0;
    pass->localArgs = 0;
    if ( !Material_LoadPassStateMap((const char **)text, stateMap) )
        return 0;
    argCount = 0;
    if ( Material_LoadPassVertexShader((const char **)text, techFlags, &paramSet, pass, 0x80u, &argCount, args) )
    {
        if ( argCount )
        {
            if ( !Material_LoadPassPixelShader((const char **)text, techFlags, &v10, pass, 0x80u, &argCount, args) )
                goto LABEL_8;
            qsort(args, argCount, 8u, (int (__cdecl *)(const void *, const void *))Material_CompareShaderArgumentsForRuntime);
            firstArg = 0;
            pass->perPrimArgCount = Material_CountArgsWithUpdateFrequency(MTL_UPDATE_PER_PRIM, args, argCount, &firstArg);
            pass->perObjArgCount = Material_CountArgsWithUpdateFrequency(MTL_UPDATE_PER_OBJECT, args, argCount, &firstArg);
            pass->stableArgCount = Material_CountArgsWithUpdateFrequency(MTL_UPDATE_RARELY, args, argCount, &firstArg);
            v6 = &args[firstArg];
            updated = Material_CountArgsWithUpdateFrequency(MTL_UPDATE_CUSTOM, args, argCount, &firstArg);
            pass->customSamplerFlags = 0;
            v15 = 0;
            while ( v15 < updated )
            {
                if ( v6->type != 4
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                                8438,
                                0,
                                "%s\n\t(customArg->type) = %i",
                                "(customArg->type == MTL_ARG_CODE_PIXEL_SAMPLER)",
                                v6->type) )
                {
                    __debugbreak();
                }
                if ( v6->dest == 255
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                                8439,
                                0,
                                "%s",
                                "customArg->dest != SAMPLER_INDEX_INVALID") )
                {
                    __debugbreak();
                }
                for ( i = 0; i < 4; ++i )
                {
                    if ( v6->u.codeSampler == g_customSamplerSrc[i] )
                    {
                        if ( ((1 << i) & pass->customSamplerFlags) != 0
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                                        8446,
                                        0,
                                        "%s",
                                        "!(pass->customSamplerFlags & (1 << customSamplerIndex))") )
                        {
                            __debugbreak();
                        }
                        if ( v6->dest != g_customSamplerDest[i]
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                                        8447,
                                        0,
                                        "%s",
                                        "customArg->dest == g_customSamplerDest[customSamplerIndex]") )
                        {
                            __debugbreak();
                        }
                        pass->customSamplerFlags |= 1 << i;
                        break;
                    }
                }
                if ( i == 4
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                                8452,
                                0,
                                "%s",
                                "customSamplerIndex != CUSTOM_SAMPLER_COUNT") )
                {
                    __debugbreak();
                }
                ++v15;
                ++v6;
            }
            argCount = pass->stableArgCount + pass->perObjArgCount + pass->perPrimArgCount;
            pass->localArgs = (MaterialShaderArgument *)Material_Alloc(8 * argCount);
            memcpy((unsigned __int8 *)pass->localArgs, (unsigned __int8 *)args, 8 * argCount);
            localArgs = pass->localArgs;
            for ( j = 0; j < pass->perPrimArgCount; ++j )
            {
                if ( localArgs->type < 3u
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                                8464,
                                0,
                                "%s",
                                "arg->type >= MTL_ARG_CODE_PRIM_BEGIN") )
                {
                    __debugbreak();
                }
                if ( localArgs->type >= 6u
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                                8465,
                                0,
                                "%s",
                                "arg->type < MTL_ARG_CODE_PRIM_END") )
                {
                    __debugbreak();
                }
                ++localArgs;
            }
            localArgs = &pass->localArgs[pass->perPrimArgCount];
            for ( k = 0; k < pass->perObjArgCount; ++k )
            {
                if ( localArgs->type < 3u
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                                8471,
                                0,
                                "%s",
                                "arg->type >= MTL_ARG_CODE_PRIM_BEGIN") )
                {
                    __debugbreak();
                }
                if ( localArgs->type >= 6u
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                                8472,
                                0,
                                "%s",
                                "arg->type < MTL_ARG_CODE_PRIM_END") )
                {
                    __debugbreak();
                }
                ++localArgs;
            }
            if ( Material_ValidateShaderLinkage(
                         paramSet.outputs,
                         paramSet.outputCount,
                         v10.varyingInputs,
                         v10.varyingInputCount) )
            {
                PassVertexDecl = Material_LoadPassVertexDecl(
                                                     (const char **)text,
                                                     paramSet.varyingInputs,
                                                     paramSet.varyingInputCount,
                                                     pass);
                //BLOPS_NULLSUB();
                //BLOPS_NULLSUB();
                Material_LoadDeclTypes((const char **)text, pass);
                v5 = strstr(*text, "vertexDef");
                if ( v5 )
                {
                    while ( strcmp(";", Com_Parse((const char **)text)->token) )
                        ;
                }
                return PassVertexDecl;
            }
            else
            {
LABEL_8:
                //BLOPS_NULLSUB();
                //BLOPS_NULLSUB();
                return 0;
            }
        }
        else
        {
            Com_ScriptError("material has no vertex arguments; it should at least have a transform matrix.\n");
            return 0;
        }
    }
    else
    {
        //BLOPS_NULLSUB();
        return 0;
    }
}

bool __cdecl Material_LoadPassStateMap(const char **text, MaterialStateMap **stateMap)
{
    parseInfo_t *token; // [esp+0h] [ebp-4h]

    if ( !Material_MatchToken(text, "stateMap") )
        return 0;
    token = Com_Parse(text);
    if ( token->token[0] && token->token[0] != 59 )
    {
        *stateMap = Material_RegisterStateMap(token->token);
        return *stateMap && Material_MatchToken(text, ";");
    }
    else
    {
        Com_ScriptError("missing stateMap filename\n");
        return 0;
    }
}

MaterialStateMap *__cdecl Material_RegisterStateMap(char *name)
{
    MaterialStateMap *stateMap; // [esp+0h] [ebp-4h]
    MaterialStateMap *stateMapa; // [esp+0h] [ebp-4h]

    stateMap = Material_FindStateMap(name);
    if ( stateMap )
        return stateMap;
    stateMapa = Material_LoadStateMap(name);
    if ( !stateMapa )
        return 0;
    Material_SetStateMap(name, stateMapa);
    return stateMapa;
}

MaterialStateMap *__cdecl Material_FindStateMap(const char *name)
{
    unsigned int hashIndex; // [esp+0h] [ebp-4h] BYREF

    if ( Material_HashStateMap(name, &hashIndex) )
        return mtlLoadGlob.stateMapHashTable[hashIndex];
    else
        return 0;
}

char __cdecl Material_HashStateMap(const char *name, unsigned int *foundHashIndex)
{
    unsigned int initialHashIndex; // [esp+14h] [ebp-8h]
    unsigned int hashIndex; // [esp+18h] [ebp-4h]

    initialHashIndex = R_HashAssetName(name) & 0x3F;
    hashIndex = initialHashIndex;
    while ( mtlLoadGlob.stateMapHashTable[hashIndex] )
    {
        if ( !strcmp(mtlLoadGlob.stateMapHashTable[hashIndex]->name, name) )
        {
            *foundHashIndex = hashIndex;
            return 1;
        }
        hashIndex = ((_BYTE)hashIndex + 1) & 0x3F;
        if ( hashIndex == initialHashIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                        2262,
                        0,
                        "%s\n\t(\"Increase the size of the state map hash table\") = %s",
                        "(hashIndex != initialHashIndex)",
                        "Increase the size of the state map hash table") )
        {
            __debugbreak();
        }
    }
    *foundHashIndex = hashIndex;
    return 0;
}

void __cdecl Material_SetStateMap(const char *name, MaterialStateMap *stateMap)
{
    unsigned int hashIndex; // [esp+0h] [ebp-4h] BYREF

    Material_HashStateMap(name, &hashIndex);
    mtlLoadGlob.stateMapHashTable[hashIndex] = stateMap;
}

MaterialStateMap *__cdecl Material_LoadStateMap(char *name)
{
    unsigned int v2; // [esp+0h] [ebp-12Ch]
    MaterialStateMap *stateMap; // [esp+10h] [ebp-11Ch]
    char filename[260]; // [esp+14h] [ebp-118h] BYREF
    int nameSize; // [esp+11Ch] [ebp-10h]
    int fileSize; // [esp+120h] [ebp-Ch]
    void *file; // [esp+124h] [ebp-8h] BYREF
    const char *text; // [esp+128h] [ebp-4h] BYREF

    Com_sprintf(filename, 0x100u, "statemaps/%s.sm", name);
    fileSize = FS_ReadFile(filename, &file);
    if ( fileSize >= 0 )
    {
        text = (const char *)file;
        Com_BeginParseSession(filename);
        Com_SetScriptWarningPrefix("^1ERROR: ");
        Com_SetSpaceDelimited(0);
        v2 = strlen(name);
        nameSize = v2 + 1;
        stateMap = (MaterialStateMap *)Material_Alloc(v2 + 45);
        stateMap->name = (const char *)&stateMap[1];
        memcpy((unsigned __int8 *)stateMap->name, (unsigned __int8 *)name, nameSize);
        if ( !Material_ParseStateMap((char **)&text, stateMap) )
            stateMap = 0;
        Com_EndParseSession();
        FS_FreeFile(file);
        return stateMap;
    }
    else
    {
        Com_ScriptError("Couldn't open statemap '%s'\n", filename);
        return 0;
    }
}

bool __cdecl Material_ParseStateMap(char **text, MaterialStateMap *stateMap)
{
    if ( !Material_ParseRuleSet(text, "alphaTest", s_stateMapDstAlphaTestBitGroup, stateMap->ruleSet) )
        return 0;
    if ( !Material_ParseRuleSet(text, "blendFunc", s_stateMapDstBlendFuncRgbBitGroup, &stateMap->ruleSet[1]) )
        return 0;
    if ( !Material_ParseRuleSet(
                    text,
                    "separateAlphaBlendFunc",
                    s_stateMapDstBlendFuncAlphaBitGroup,
                    &stateMap->ruleSet[2]) )
        return 0;
    if ( !Material_ParseRuleSet(text, "cullFace", s_stateMapDstCullFaceBitGroup, &stateMap->ruleSet[3]) )
        return 0;
    if ( !Material_ParseRuleSet(text, "depthTest", s_stateMapDstDepthTestBitGroup, &stateMap->ruleSet[4]) )
        return 0;
    if ( !Material_ParseRuleSet(text, "depthWrite", s_stateMapDstDepthWriteBitGroup, &stateMap->ruleSet[5]) )
        return 0;
    if ( !Material_ParseRuleSet(text, "colorWrite", s_stateMapDstColorWriteBitGroup, &stateMap->ruleSet[6]) )
        return 0;
    if ( !Material_ParseRuleSet(text, "polygonOffset", s_stateMapDstPolygonOffsetBitGroup, &stateMap->ruleSet[7]) )
        return 0;
    if ( Material_ParseRuleSet(text, "stencil", s_stateMapDstStencilBitGroup, &stateMap->ruleSet[8]) )
        return Material_ParseRuleSet(text, "wireframe", s_stateMapDstWireframeBitGroup, &stateMap->ruleSet[9]) != 0;
    return 0;
}

char __cdecl Material_ParseRuleSet(
                char **text,
                const char *ruleSetName,
                const MtlStateMapBitGroup *stateSet,
                MaterialStateMapRuleSet **ruleSet)
{
    int v5; // edx
    int v6; // eax
    int v7; // [esp+0h] [ebp-2034h]
    MaterialStateMapRule dst[257]; // [esp+4h] [ebp-2030h] BYREF
    int v9; // [esp+2024h] [ebp-10h]
    int ruleCount; // [esp+2028h] [ebp-Ch]
    char *token; // [esp+202Ch] [ebp-8h]
    int i; // [esp+2030h] [ebp-4h]

    if ( !Material_MatchToken((const char **)text, ruleSetName) )
        return 0;
    if ( !Material_MatchToken((const char **)text, "{") )
        return 0;
    memset(dst, 0, sizeof(dst));
    v7 = 0;
    ruleCount = 0;
    while ( 1 )
    {
        if ( ruleCount > 256 )
        {
            Com_ScriptError("state %s has more than %i rules\n", ruleSetName, 256);
            return 0;
        }
        token = (char *)Com_Parse((const char **)text);
        if ( *token == 125 )
            break;
        v9 = Material_ParseRuleSetCondition((const char **)text, token, &dst[ruleCount]);
        if ( v9 )
        {
            if ( v9 == 2 )
                return 0;
            if ( v7 == ruleCount )
            {
                Com_ScriptError("missing rule condition for state %s\n", ruleSetName);
                return 0;
            }
            if ( !Material_ParseRuleSetValue((const char **)text, token, stateSet, &dst[v7]) )
                return 0;
            for ( i = v7 + 1; i < ruleCount; ++i )
            {
                v5 = i;
                dst[v5].stateBitsSet[0] = dst[v7].stateBitsSet[0];
                dst[v5].stateBitsSet[1] = dst[v7].stateBitsSet[1];
                v6 = i;
                dst[v6].stateBitsClear[0] = dst[v7].stateBitsClear[0];
                dst[v6].stateBitsClear[1] = dst[v7].stateBitsClear[1];
            }
            v7 = ruleCount;
        }
        else
        {
            ++ruleCount;
        }
    }
    if ( ruleCount )
    {
        if ( v7 == ruleCount )
        {
            *ruleSet = Material_AssembleRuleSet(ruleCount, dst);
            return 1;
        }
        else
        {
            Com_ScriptError("missing value for state %s\n", ruleSetName);
            return 0;
        }
    }
    else
    {
        Com_ScriptError("no entries for state %s: you may want to do 'default: passthrough;'\n", ruleSetName);
        return 0;
    }
}

int __cdecl Material_ParseRuleSetCondition(const char **text, const char *token, MaterialStateMapRule *rule)
{
    MtlParseSuccess success; // [esp+3Ch] [ebp-4h]
    MtlParseSuccess successa; // [esp+3Ch] [ebp-4h]
    parseInfo_t *tokena; // [esp+4Ch] [ebp+Ch]
    parseInfo_t *tokenb; // [esp+4Ch] [ebp+Ch]

    if ( rule->stateBitsMask[0]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    2893,
                    1,
                    "%s\n\t(rule->stateBitsMask[0]) = %i",
                    "(rule->stateBitsMask[0] == 0)",
                    rule->stateBitsMask[0]) )
    {
        __debugbreak();
    }
    if ( rule->stateBitsMask[1]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    2894,
                    1,
                    "%s\n\t(rule->stateBitsMask[1]) = %i",
                    "(rule->stateBitsMask[1] == 0)",
                    rule->stateBitsMask[1]) )
    {
        __debugbreak();
    }
    if ( rule->stateBitsValue[0]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    2895,
                    1,
                    "%s\n\t(rule->stateBitsValue[0]) = %i",
                    "(rule->stateBitsValue[0] == 0)",
                    rule->stateBitsValue[0]) )
    {
        __debugbreak();
    }
    if ( rule->stateBitsValue[1]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    2896,
                    1,
                    "%s\n\t(rule->stateBitsValue[1]) = %i",
                    "(rule->stateBitsValue[1] == 0)",
                    rule->stateBitsValue[1]) )
    {
        __debugbreak();
    }
    if ( !strcmp(token, "default") )
        return Material_MatchToken(text, ":") ? 0 : 2;
    success = (MtlParseSuccess)Material_ParseRuleSetConditionTest(text, token, rule);
    if ( success )
        return success;
    do
    {
        tokena = Com_Parse(text);
        if ( !strcmp(tokena->token, ":") )
            return 0;
        if ( strcmp(tokena->token, "&&"))
        {
            Com_ScriptError("expected ':' or '&&', found '%s'\n", tokena->token);
            return 2;
        }
        tokenb = Com_Parse(text);
        successa = (MtlParseSuccess)Material_ParseRuleSetConditionTest(text, tokenb->token, rule);
    }
    while ( successa == MTL_PARSE_SUCCESS );
    if ( successa == MTL_PARSE_NO_MATCH )
        Com_ScriptError("failed parsing conditional after '&&'\n");
    return 2;
}

int __cdecl Material_ParseRuleSetConditionTest(const char **text, const char *token, MaterialStateMapRule *rule)
{
    int sourceIndex; // [esp+18h] [ebp-Ch]
    const MtlStateMapBitName *bitName; // [esp+1Ch] [ebp-8h]
    int stateBitsIndex; // [esp+20h] [ebp-4h]

    for ( sourceIndex = 0; ; ++sourceIndex )
    {
        if ( !s_stateMapSrcBitGroup[sourceIndex].name )
            return 1;
        if ( !strcmp(token, s_stateMapSrcBitGroup[sourceIndex].name) )
            break;
    }
    if ( !Material_MatchToken(text, "==") )
        return 2;
    bitName = Material_ParseValueForState(text, s_stateMapSrcBitGroup[sourceIndex].bitNames);
    if ( !bitName )
        return 2;
    stateBitsIndex = 0;
    while ( !s_stateMapSrcBitGroup[sourceIndex].stateBitsMask[stateBitsIndex] )
    {
        if ( ++stateBitsIndex >= 2
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                        2882,
                        0,
                        "%s\n\t(s_stateMapSrcBitGroup[sourceIndex].name) = %s",
                        "(stateBitsIndex < 2)",
                        s_stateMapSrcBitGroup[sourceIndex].name) )
        {
            __debugbreak();
        }
    }
    rule->stateBitsMask[stateBitsIndex] |= s_stateMapSrcBitGroup[sourceIndex].stateBitsMask[stateBitsIndex];
    rule->stateBitsValue[stateBitsIndex] |= bitName->bits;
    return 0;
}

const MtlStateMapBitName *__cdecl Material_ParseValueForState(const char **text, const MtlStateMapBitName *bitNames)
{
    parseInfo_t *v2; // eax
    int valueIndex; // [esp+14h] [ebp-110h]
    parseInfo_t *token; // [esp+18h] [ebp-10Ch]
    char signedToken[260]; // [esp+1Ch] [ebp-108h] BYREF

    token = Com_Parse(text);
    if ( token->token[0] == 45 )
    {
        v2 = Com_Parse(text);
        Com_sprintf(signedToken, 0x100u, "-%s", v2->token);
        token = (parseInfo_t *)signedToken;
    }
    for ( valueIndex = 0; bitNames[valueIndex].name; ++valueIndex )
    {
        if ( !strcmp(token->token, bitNames[valueIndex].name) )
            return &bitNames[valueIndex];
    }
    Com_ScriptError("%s is not a valid state value\n", token->token);
    return 0;
}

bool __cdecl Material_ParseRuleSetValue(
                const char **text,
                const char *token,
                const MtlStateMapBitGroup *stateSet,
                MaterialStateMapRule *rule)
{
    int stateIndex; // [esp+18h] [ebp-Ch]
    const MtlStateMapBitName *bitName; // [esp+1Ch] [ebp-8h]
    int stateBitsIndex; // [esp+20h] [ebp-4h]

    if ( rule->stateBitsClear[0]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    2931,
                    1,
                    "%s\n\t(rule->stateBitsClear[0]) = %i",
                    "(rule->stateBitsClear[0] == 0)",
                    rule->stateBitsClear[0]) )
    {
        __debugbreak();
    }
    if ( rule->stateBitsClear[1]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    2932,
                    1,
                    "%s\n\t(rule->stateBitsClear[1]) = %i",
                    "(rule->stateBitsClear[1] == 0)",
                    rule->stateBitsClear[1]) )
    {
        __debugbreak();
    }
    if ( rule->stateBitsSet[0]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    2933,
                    1,
                    "%s\n\t(rule->stateBitsSet[0]) = %i",
                    "(rule->stateBitsSet[0] == 0)",
                    rule->stateBitsSet[0]) )
    {
        __debugbreak();
    }
    if ( rule->stateBitsSet[1]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    2934,
                    1,
                    "%s\n\t(rule->stateBitsSet[1]) = %i",
                    "(rule->stateBitsSet[1] == 0)",
                    rule->stateBitsSet[1]) )
    {
        __debugbreak();
    }
    if ( !strcmp(token, "passthrough") )
        return Material_MatchToken(text, ";");
    Com_UngetToken();
    stateIndex = 0;
    do
    {
        bitName = Material_ParseValueForState(text, stateSet[stateIndex].bitNames);
        if ( !bitName )
            return 0;
        stateBitsIndex = 0;
        while ( !stateSet[stateIndex].stateBitsMask[stateBitsIndex] )
        {
            if ( ++stateBitsIndex >= 2
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                            2957,
                            0,
                            "%s\n\t(stateSet[stateIndex].name) = %s",
                            "(stateBitsIndex < 2)",
                            stateSet[stateIndex].name) )
            {
                __debugbreak();
            }
        }
        rule->stateBitsSet[stateBitsIndex] |= bitName->bits;
        rule->stateBitsClear[stateBitsIndex] |= stateSet[stateIndex++].stateBitsMask[stateBitsIndex];
        if ( !stateSet[stateIndex].name )
            return Material_MatchToken(text, ";");
    }
    while ( Material_MatchToken(text, ",") );
    return 0;
}

MaterialStateMapRuleSet *__cdecl Material_AssembleRuleSet(int ruleCount, const MaterialStateMapRule *rules)
{
    int stateBitsIndex; // [esp+0h] [ebp-Ch]
    unsigned __int8 *ruleSet; // [esp+4h] [ebp-8h]
    int ruleIndex; // [esp+8h] [ebp-4h]

    ruleSet = Material_Alloc(32 * ruleCount + 4);
    *(unsigned int *)ruleSet = ruleCount;
    Com_Memcpy(ruleSet + 4, rules, 32 * ruleCount);
    for ( ruleIndex = 0; ruleIndex < ruleCount; ++ruleIndex )
    {
        for ( stateBitsIndex = 0; stateBitsIndex < 2; ++stateBitsIndex )
            *(unsigned int *)&ruleSet[32 * ruleIndex + 28 + 4 * stateBitsIndex] = ~*(unsigned int *)&ruleSet[32 * ruleIndex
                                                                                                                                                                                 + 28
                                                                                                                                                                                 + 4 * stateBitsIndex];
    }
    return (MaterialStateMapRuleSet *)ruleSet;
}

char __cdecl Material_LoadPassVertexDecl(
                const char **text,
                ShaderVaryingDef *inputTable,
                unsigned int inputCount,
                MaterialPass *pass)
{
    unsigned __int8 source; // [esp+16h] [ebp-3Ah] BYREF
    unsigned __int8 resourceDest; // [esp+17h] [ebp-39h] BYREF
    int insertIndex; // [esp+18h] [ebp-38h]
    bool existing; // [esp+1Fh] [ebp-31h] BYREF
    const char *token; // [esp+20h] [ebp-30h]
    int routingIndex; // [esp+24h] [ebp-2Ch]
    unsigned __int8 dest[2]; // [esp+2Ah] [ebp-26h] BYREF
    MaterialStreamRouting routing[16]; // [esp+2Ch] [ebp-24h] BYREF

    for ( routingIndex = 0; ; ++routingIndex )
    {
        if ( (unsigned int)routingIndex >= 0x10 )
        {
            Com_ScriptError("More than %i vertex mappings\n", routingIndex);
            return 0;
        }
        token = (const char *)Com_Parse(text);
        if ( strcmp(token, "vertex") )
            break;
        if ( !Material_MatchToken(text, ".") )
            return 0;
        token = (const char *)Com_Parse(text);
        if ( !Material_StreamDestForName(text, token, &dest[1]) )
            return 0;
        if ( !Material_ResourceDestForStreamDest(dest[1], inputTable, inputCount, &resourceDest) )
            return 0;
        if ( !Material_MatchToken(text, "=") )
            return 0;
        if ( !Material_MatchToken(text, "code") )
            return 0;
        if ( !Material_MatchToken(text, ".") )
            return 0;
        token = (const char *)Com_Parse(text);
        if ( !Material_StreamSourceForName(text, token, &source) )
            return 0;
        if ( !Material_MatchToken(text, ";") )
            return 0;
        for ( insertIndex = routingIndex;
                    insertIndex > 0
             && dest[2 * insertIndex] >= (int)source
             && (dest[2 * insertIndex] != source || dest[2 * insertIndex + 1] >= (int)resourceDest);
                    --insertIndex )
        {
            routing[insertIndex] = *(MaterialStreamRouting *)&dest[2 * insertIndex];
        }
        routing[insertIndex].source = source;
        routing[insertIndex].dest = resourceDest;
    }
    Com_UngetToken();
    if ( !Material_CheckUnspecifiedVertexInputs(inputTable, inputCount) )
        return 0;
    pass->vertexDecl = Material_AllocVertexDecl(routing, routingIndex, &existing);
    if ( !existing )
        Load_BuildVertexDecl(&pass->vertexDecl);
    return 1;
}

MaterialVertexDeclaration *__cdecl Material_AllocVertexDecl(
                MaterialStreamRouting *routingData,
                unsigned int streamCount,
                bool *existing)
{
    unsigned int hashIndex; // [esp+8h] [ebp-Ch]
    MaterialVertexDeclaration *mvd; // [esp+Ch] [ebp-8h]
    int routingIndex; // [esp+10h] [ebp-4h]

    if ( !streamCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    2120,
                    0,
                    "%s",
                    "streamCount") )
    {
        __debugbreak();
    }
    hashIndex = Material_HashVertexDecl(routingData, streamCount);
    for ( mvd = &mtlLoadGlob.vertexDeclHashTable[hashIndex];
                mvd->streamCount;
                mvd = &mtlLoadGlob.vertexDeclHashTable[hashIndex] )
    {
        if ( mvd->streamCount == streamCount && !memcmp(&mvd->routing, routingData, 2 * streamCount) )
        {
            *existing = 1;
            return mvd;
        }
        hashIndex = ((_BYTE)hashIndex + 1) & 0x3F;
    }
    if ( mtlLoadGlob.vertexDeclCount == 63 )
        Com_Error(ERR_DROP, "More than %i vertex declarations in use", 63);
    ++mtlLoadGlob.vertexDeclCount;
    memset(&mvd->streamCount, 0, sizeof(MaterialVertexDeclaration));
    memcpy((unsigned __int8 *)&mvd->routing, &routingData->source, 2 * streamCount);
    if ( streamCount >= 0x10
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    2141,
                    0,
                    "streamCount doesn't index ARRAY_COUNT( mvd->routing.data )\n\t%i not in [0, %i)",
                    streamCount,
                    16) )
    {
        __debugbreak();
    }
    mvd->streamCount = streamCount;
    if ( mvd->streamCount != streamCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    2143,
                    0,
                    "%s\n\t(streamCount) = %i",
                    "(mvd->streamCount == streamCount)",
                    streamCount) )
    {
        __debugbreak();
    }
    for ( routingIndex = 0; routingIndex < (int)streamCount; ++routingIndex )
    {
        if ( mvd->routing.data[routingIndex].source >= 5u )
        {
            mvd->hasOptionalSource = 1;
            break;
        }
    }
    *existing = 0;
    return mvd;
}

int __cdecl Material_HashVertexDecl(const MaterialStreamRouting *routingData, int streamCount)
{
    char hash; // [esp+0h] [ebp-10h]
    unsigned int byteIndex; // [esp+Ch] [ebp-4h]

    hash = 0;
    for ( byteIndex = 0; byteIndex < 2 * streamCount; ++byteIndex )
        hash += (byteIndex + 119) * *(&routingData->source + byteIndex);
    return hash & 0x3F;
}

char __cdecl Material_StreamDestForName(const char **text, const char *destName, unsigned __int8 *dest)
{
    int index; // [esp+78h] [ebp-4h] BYREF

    if ( !strcmp(destName, "position") )
    {
        *dest = 0;
        return 1;
    }
    else if ( !strcmp(destName, "normal") )
    {
        *dest = 1;
        return 1;
    }
    else if ( !strcmp(destName, "color") )
    {
        if ( Material_ParseIndex(text, 2, &index) )
        {
            *dest = index + 2;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if ( !strcmp(destName, "depth") )
    {
        *dest = 4;
        return 1;
    }
    else if ( !strcmp(destName, "texcoord") )
    {
        if ( Material_ParseIndex(text, 14, &index) )
        {
            *dest = index + 5;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if ( !strcmp(destName, "blendweight") )
    {
        *dest = 19;
        return 1;
    }
    else
    {
        Com_ScriptError("unknown stream destination '%s'\n", destName);
        return 0;
    }
}

bool __cdecl Material_ParseIndex(const char **text, int indexCount, int *index)
{
    if ( !Material_MatchToken(text, "[") )
        return 0;
    *index = Com_ParseInt(text);
    if ( *index < 0 || *index >= indexCount )
        Com_ScriptError("index '%i' is not in the range [0, %i]\n", *index, indexCount - 1);
    return Material_MatchToken(text, "]");
}

char __cdecl Material_StreamSourceForName(const char **text, const char *sourceName, unsigned __int8 *source)
{
    int index; // [esp+8Ch] [ebp-4h] BYREF

    if ( !strcmp(sourceName, "position") )
    {
        *source = 0;
        return 1;
    }
    else if ( !strcmp(sourceName, "normal") )
    {
        *source = 3;
        return 1;
    }
    else if ( !strcmp(sourceName, "tangent") )
    {
        *source = 4;
        return 1;
    }
    else if ( !strcmp(sourceName, "color") )
    {
        *source = 1;
        return 1;
    }
    else if ( !strcmp(sourceName, "texcoord") )
    {
        if ( Material_ParseIndex(text, 3, &index) )
        {
            if ( index )
                *source = index + 4;
            else
                *source = 2;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if ( !strcmp(sourceName, "normalTransform") )
    {
        if ( Material_ParseIndex(text, 2, &index) )
        {
            *source = index + 7;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if ( !strcmp(sourceName, "blendweight") )
    {
        *source = 9;
        return 1;
    }
    else
    {
        Com_ScriptError("unknown stream source '%s'\n", sourceName);
        return 0;
    }
}

char __cdecl Material_ResourceDestForStreamDest(
                unsigned __int8 streamDest,
                ShaderVaryingDef *inputTable,
                unsigned int inputCount,
                unsigned __int8 *resourceDest)
{
    const char *v5; // eax
    unsigned int inputIndex; // [esp+0h] [ebp-4h]

    for ( inputIndex = 0; ; ++inputIndex )
    {
        if ( inputIndex >= inputCount )
        {
            v5 = Material_NameForStreamDest(streamDest);
            Com_ScriptError("vertex shader doesn't use input '%s'.\n", v5);
            return 0;
        }
        if ( inputTable[inputIndex].streamDest == streamDest )
            break;
    }
    if ( inputTable[inputIndex].isAssigned )
    {
        Com_ScriptError("vertex input '%s' specified more than once.\n", inputTable[inputIndex].name);
        return 0;
    }
    else
    {
        inputTable[inputIndex].isAssigned = 1;
        *resourceDest = inputTable[inputIndex].resourceDest;
        return 1;
    }
}

const char *__cdecl Material_NameForStreamDest(unsigned __int8 dest)
{
    const char *result; // eax
    const char *v2; // eax

    switch ( dest )
    {
        case 0u:
            result = "position";
            break;
        case 1u:
            result = "normal";
            break;
        case 2u:
            result = "color[0]";
            break;
        case 3u:
            result = "color[1]";
            break;
        case 4u:
            result = "depth";
            break;
        case 5u:
            result = "texcoord[0]";
            break;
        case 6u:
            result = "texcoord[1]";
            break;
        case 7u:
            result = "texcoord[2]";
            break;
        case 8u:
            result = "texcoord[3]";
            break;
        case 9u:
            result = "texcoord[4]";
            break;
        case 0xAu:
            result = "texcoord[5]";
            break;
        case 0xBu:
            result = "texcoord[6]";
            break;
        case 0xCu:
            result = "texcoord[7]";
            break;
        case 0xDu:
            result = "texcoord[8]";
            break;
        case 0xEu:
            result = "texcoord[9]";
            break;
        case 0xFu:
            result = "texcoord[10]";
            break;
        case 0x10u:
            result = "texcoord[11]";
            break;
        case 0x11u:
            result = "texcoord[12]";
            break;
        case 0x12u:
            result = "texcoord[13]";
            break;
        case 0x13u:
            result = "blendweight";
            break;
        default:
            v2 = va("unhandled case %i", dest);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp", 3215, 0, v2) )
                __debugbreak();
            result = "";
            break;
    }
    return result;
}

bool __cdecl Material_CheckUnspecifiedVertexInputs(const ShaderVaryingDef *inputTable, unsigned int inputCount)
{
    unsigned int inputIndex; // [esp+0h] [ebp-8h]
    bool isValid; // [esp+7h] [ebp-1h]

    isValid = 1;
    for ( inputIndex = 0; inputIndex < inputCount; ++inputIndex )
    {
        if ( !inputTable[inputIndex].isAssigned )
        {
            Com_ScriptError("vertex input '%s' is not specified.\n", inputTable[inputIndex].name);
            isValid = 0;
        }
    }
    return isValid;
}

char __cdecl Material_LoadPassVertexShader(
                const char **text,
                unsigned __int16 *techFlags,
                ShaderParameterSet *paramSet,
                MaterialPass *pass,
                unsigned int argLimit,
                unsigned int *argCount,
                MaterialShaderArgument *args)
{
    parseInfo_t *shaderName; // [esp+8h] [ebp-8h]
    MaterialVertexShader *mtlShader; // [esp+Ch] [ebp-4h]

    memset(paramSet, 0, sizeof(ShaderParameterSet));
    if ( !Material_MatchToken(text, "vertexShader") )
        return 0;
    Material_ParseShaderVersion(text);
    shaderName = Com_Parse(text);
    mtlShader = Material_RegisterVertexShader(shaderName->token);
    if ( !mtlShader )
        return 0;
    pass->vertexShader = mtlShader;
    return Material_SetPassShaderArguments_DX(
                     (char **)text,
                     (char *)mtlShader->name,
                     MTL_VERTEX_SHADER,
                     (const DWORD *)&mtlShader[1],
                     techFlags,
                     paramSet,
                     argLimit,
                     argCount,
                     args);
}

MaterialVertexShader *__cdecl Material_RegisterVertexShader(char *shaderName)
{
    unsigned int hashIndex; // [esp+14h] [ebp-Ch] BYREF
    MaterialVertexShader *mtlShader; // [esp+1Ch] [ebp-4h]

    if ( Material_GetVertexShaderHashIndex(shaderName, &hashIndex) )
        return mtlLoadGlob.vertexShaderHashTable[hashIndex];
    ProfLoad_Begin("Load vertex shader");
    mtlShader = Material_LoadVertexShader(shaderName);
    ProfLoad_End();
    if ( mtlShader && !strcmp(mtlShader->name, shaderName) )
    {
        mtlLoadGlob.vertexShaderHashTable[hashIndex] = mtlShader;
        if ( ++mtlLoadGlob.vertexShaderCount == 5120 )
            Com_Error(ERR_DROP, "More than %i unique vertex shaders", 5119);
    }
    return mtlShader;
}

MaterialVertexShader *__cdecl Material_LoadVertexShader(char *shaderName)
{
    unsigned __int8 *v2; // eax
    const char *v3; // eax
    ID3DXBuffer *v4; // [esp-8h] [ebp-50h]
    unsigned int programSize; // [esp+10h] [ebp-38h]
    int hr; // [esp+18h] [ebp-30h]
    char target[16]; // [esp+1Ch] [ebp-2Ch] BYREF
    DWORD *program; // [esp+30h] [ebp-18h]
    unsigned int nameSize; // [esp+34h] [ebp-14h]
    ID3DXBuffer *shader; // [esp+3Ch] [ebp-Ch]
    unsigned int totalSize; // [esp+40h] [ebp-8h]
    MaterialVertexShader *mtlShader; // [esp+44h] [ebp-4h]

    Material_GetShaderTargetString(target, 0x10u, "vs");
    shader = Material_CompileShader(shaderName, MTL_VERTEX_SHADER, (char*)"vs_main", target);
    if ( !shader )
        return 0;
    programSize = shader->GetBufferSize();
    if ( !programSize
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    5154,
                    0,
                    "%s\n\t(programSize) = %i",
                    "(programSize > 0)",
                    0) )
    {
        __debugbreak();
    }
    nameSize = strlen(shaderName) + 1;
    totalSize = programSize + nameSize + 16;
    mtlShader = (MaterialVertexShader *)Material_Alloc(totalSize);
    program = (DWORD*)&mtlShader[1];
    mtlShader->name = (char *)&mtlShader[1] + programSize;
    memcpy((unsigned __int8 *)mtlShader->name, (unsigned __int8 *)shaderName, nameSize);
    v4 = shader;
    v2 = (unsigned __int8 *)shader->GetBufferPointer();
    //v2 = (unsigned __int8 *)((int (__thiscall *)(ID3DXBuffer *))shader->GetBufferPointer)(shader);
    memcpy((unsigned __int8 *)program, v2, (unsigned int)v4);
    hr = dx.device->CreateVertexShader((const DWORD*)program, &mtlShader->prog.vs);
    //hr = ((int (__stdcall *)(IDirect3DDevice9 *, unsigned int *, MaterialVertexShaderProgram *, unsigned int))dx.device->CreateVertexShader)(
    //             dx.device,
    //             program,
    //             &mtlShader->prog,
    //             programSize);
    if ( hr >= 0 )
    {
        mtlShader->prog.loadDef.programSize = programSize >> 2;
        if ( 4 * mtlShader->prog.loadDef.programSize != programSize
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                        5205,
                        1,
                        "mtlShader->prog.loadDef.programSize * sizeof( mtlShader->prog.loadDef.program[0] ) == programSize\n"
                        "\t%i, %i",
                        4 * mtlShader->prog.loadDef.programSize,
                        programSize) )
        {
            __debugbreak();
        }
        mtlShader->prog.loadDef.program = program;
        shader->Release();
        return mtlShader;
    }
    else
    {
        v3 = R_ErrorDescription(hr);
        Com_ScriptError("vertex shader creation failed for %s: %s\n", shaderName, v3);
        return 0;
    }
}

ID3DXBuffer *__cdecl Material_CompileShader(
                char *shaderName,
                MaterialShaderType shaderType,
                char *entryPoint,
                char *target)
{
    char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    char *ScriptErrorPrefix; // [esp+0h] [ebp-8E64h]
    _BYTE v9[2052]; // [esp+4h] [ebp-8E60h] BYREF
    char *prefix; // [esp+808h] [ebp-865Ch]
    char *fileName; // [esp+80Ch] [ebp-8658h] BYREF
    void (*v12)(const char *, ...); // [esp+810h] [ebp-8654h]
    char v13; // [esp+817h] [ebp-864Dh]
    char *errorMessage; // [esp+818h] [ebp-864Ch]
    unsigned int lineNumber[2]; // [esp+81Ch] [ebp-8648h] BYREF
    char dest[516]; // [esp+824h] [ebp-8640h] BYREF
    unsigned int shaderTextLen; // [esp+A28h] [ebp-843Ch]
    int v18; // [esp+A2Ch] [ebp-8438h] BYREF
    HRESULT hr; // [esp+A30h] [ebp-8434h]
    GfxAssembledShaderText prog; // [esp+A34h] [ebp-8430h] BYREF
    char *shaderString; // [esp+8E50h] [ebp-14h]
    unsigned int v22[2]; // [esp+8E54h] [ebp-10h] BYREF
    ID3DXBuffer *shader; // [esp+8E5Ch] [ebp-8h] BYREF
    int v24; // [esp+8E60h] [ebp-4h]

    shader = 0;
    v22[0] = 0;
    v18 = 0;
    lineNumber[1] = 0;
    v22[1] = 0;
    Com_sprintf(dest, 0x200u, "shaders/%s", shaderName);
    shaderString = (char *)Hunk_AllocateTempMemory(0x80000, "Material_CompileShader");
    shaderTextLen = Material_GenerateShaderString(&prog, shaderName, shaderType, shaderString, 0x80000u);
    if ( !shaderTextLen )
    {
        Hunk_FreeTempMemory(shaderString);
        return 0;
    }
    if ( Material_FindCachedShaderDX(shaderString, shaderTextLen, entryPoint, target, &shader, shaderName, 0) )
    {
LABEL_22:
        Hunk_FreeTempMemory(shaderString);
        return shader;
    }
    v24 = 2;
    v5 = strstr(shaderName, "pimp_shader_sw4");
    if ( !v5 )
        v24 |= 0x10000u;
    hr = D3DXCompileShader(shaderString, shaderTextLen, 0, 0, entryPoint, target, v24, &shader, NULL, NULL);
    //hr = D3DXCompileShader(shaderString, shaderTextLen, 0, 0, entryPoint, target, v24, &shader, v22, &v18);
    //__asm { fnclex }
    if ( v22[0] )
    {
        v13 = 0;
        v6 = (const char *)(*(int (__stdcall **)(unsigned int))(*(unsigned int *)v22[0] + 12))(v22[0]);
        if ( I_stristr(v6, "error") )
            v12 = Com_ScriptError;
        else
            v12 = Com_ScriptWarning;
        errorMessage = (char *)(*(int (__stdcall **)(unsigned int))(*(unsigned int *)v22[0] + 12))(v22[0]);
        Material_FileIncludeFileAndLineNumber(&prog, errorMessage, &fileName, lineNumber);
        if ( v12 )
        {
            ScriptErrorPrefix = (char *)Com_GetScriptErrorPrefix();
            prefix = (char *)Com_GetScriptWarningPrefix();
            Com_SetScriptWarningPrefix("^3WARNING: ");
            Com_SetScriptErrorPrefix("^1ERROR: ");
            v9[0] = 0;
            v12(
                (char*)"compiler message(s) for %s (entryPoint=\"%s\", target=\"%s\"):\n%s\n%s\n%s(%d)\n",
                dest,
                entryPoint,
                target,
                errorMessage,
                v9,
                fileName,
                lineNumber[0]);
            Com_SetScriptWarningPrefix(prefix);
            Com_SetScriptErrorPrefix(ScriptErrorPrefix);
        }
        (*(void (__thiscall **)(unsigned int, unsigned int))(*(unsigned int *)v22[0] + 8))(v22[0], v22[0]);
    }
    if ( hr < 0 )
    {
        v7 = R_ErrorDescription(hr);
        Com_ScriptError("%s compilation failed - %s\n", dest, v7);
        Hunk_FreeTempMemory(shaderString);
        return 0;
    }
    if ( shader )
    {
        if ( !v18 )
        {
            shader->Release();
            Com_ScriptError("%s compilation failed - NULL constants\n", dest);
            Hunk_FreeTempMemory(shaderString);
            return 0;
        }
        (*(void (__stdcall **)(int))(*(unsigned int *)v18 + 8))(v18);
        Material_CacheShaderDX(shaderString, shaderTextLen, entryPoint, target, shader, 0);
        goto LABEL_22;
    }
    if ( v18 )
        (*(void (__stdcall **)(int))(*(unsigned int *)v18 + 8))(v18);
    Com_ScriptError("%s compilation failed - NULL shader\n", dest);
    Hunk_FreeTempMemory(shaderString);
    return 0;
}

unsigned int __cdecl Material_GenerateShaderString(
                GfxAssembledShaderText *prog,
                const char *shaderName,
                MaterialShaderType shaderType,
                char *shaderString,
                unsigned int sizeofShaderString)
{
    char shaderFilename[268]; // [esp+4h] [ebp-228h] BYREF
    const char *iwProjectGameDir; // [esp+110h] [ebp-11Ch]
    bool wasGenerated; // [esp+117h] [ebp-115h]
    int textSize; // [esp+118h] [ebp-114h]
    char filepath[260]; // [esp+11Ch] [ebp-110h] BYREF
    const char *formatString; // [esp+224h] [ebp-8h]
    char *text; // [esp+228h] [ebp-4h] BYREF

    if ( !gIsPIMPEnabled || gIsSW2Material )
    {
        formatString = "shaders/%s";
        Com_sprintf(filepath, 0x100u, "shaders/%s", shaderName);
    }
    else
    {
        formatString = "pimp/shaders/%s";
        Com_sprintf(filepath, 0x100u, "pimp/shaders/%s", shaderName);
    }
    textSize = FS_ReadFile(filepath, (void **)&text);
    if ( textSize >= 0 )
    {
        prog->string = shaderString;
        prog->used = 0;
        prog->currentDestLine = 1;
        prog->fileCount = 0;
        prog->total = sizeofShaderString;
        prog->overflowed = 0;
        iwProjectGameDir = getenv("IW_PROJECT_GAMEDIR");
        if ( iwProjectGameDir && isalpha(*iwProjectGameDir) && iwProjectGameDir[1] == 58 )
            iwProjectGameDir += 2;
        if ( iwProjectGameDir )
            Com_sprintf(shaderFilename, 0x104u, "%s/%s/main/%s", iwProjectGameDir, "pc", filepath);
        else
            Com_sprintf(shaderFilename, 0x104u, "%s/%s/main/%s", ".", "pc", filepath);
        FS_ReplaceSeparators(shaderFilename);
        Material_EmitShaderString(prog, "#define SET_TARGET 1\n");
        Material_EmitShaderString(prog, "#define viewProj\t\t\t\tviewProjectionMatrix\n");
        Material_EmitShaderString(prog, "#define worldIT    \t\t\t\tworldMatrix\n");
        Material_EmitShaderString(prog, "#define world\t\t\t\t\tworldMatrix\n");
        Material_EmitShaderString(prog, "#define view \t\t\t\t\tviewMatrix\n");
        Material_EmitShaderString(prog, "#define viewInverse    \t\t\tinverseViewMatrix\n");
        Material_EmitShaderString(prog, "#define Projection    \t\t\tprojectionMatrix\n");
        Material_EmitShaderString(prog, "#define PC\n");
        Material_EmitShaderString(prog, "#define NGL_PC 1\n");
        Material_EmitShaderString(prog, "#define CONFIG_RELATIVE_WORLD    1\n");
        if ( shaderType )
        {
            Material_EmitShaderString(prog, "#define IS_VERTEX_SHADER 0\n");
            Material_EmitShaderString(prog, "#define IS_PIXEL_SHADER 1\n");
        }
        else
        {
            Material_EmitShaderString(prog, "#define IS_VERTEX_SHADER 1\n");
            Material_EmitShaderString(prog, "#define IS_PIXEL_SHADER 0\n");
        }
        Material_AddShaderFile(prog, shaderName, 1u);
        wasGenerated = Material_GenerateShaderString_r(prog, shaderName, text, textSize, 0);
        FS_FreeFile(text);
        if ( wasGenerated )
        {
            if ( prog->overflowed )
            {
                return 0;
            }
            else
            {
                prog->string[prog->used] = 0;
                return prog->used;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_PrintWarning(8, "Couldn't read shader '%s'\n", filepath);
        return 0;
    }
}

void __cdecl Material_EmitShaderString(GfxAssembledShaderText *prog, const char *string)
{
    int v2; // [esp+0h] [ebp-14h]

    if ( !prog->overflowed )
    {
        v2 = strlen(string);
        if ( v2 + prog->used < prog->total )
        {
            prog->currentDestLine += Material_MemCopyAndReturnLines(&prog->string[prog->used], string, v2);
            prog->used += v2;
        }
        else
        {
            Com_ScriptError("Shader text overflowed. You may need to increase R_SHADER_TEXT_SIZE_LIMIT.\n");
            prog->overflowed = 1;
        }
    }
}

int __cdecl Material_MemCopyAndReturnLines(char *destString, const char *srcString, int len)
{
    int i; // [esp+0h] [ebp-8h]
    int iLines; // [esp+4h] [ebp-4h]

    iLines = 0;
    for ( i = 0; i < len; ++i )
    {
        destString[i] = srcString[i];
        if ( srcString[i] == 10 )
            ++iLines;
    }
    return iLines;
}

void __cdecl Material_AddShaderFile(GfxAssembledShaderText *prog, const char *shaderFileName, unsigned int srcLine)
{
    if ( prog->fileCount < 0x80 )
    {
        //BLOPS_NULLSUB();
        I_strncpyz(prog->files[prog->fileCount].fileName, shaderFileName, 256);
        prog->files[prog->fileCount].srcLine = srcLine;
        prog->files[prog->fileCount++].destLine = prog->currentDestLine;
    }
    else
    {
        Com_ScriptError("Max shader include files exceeded.    Increase R_SHADER_MAX_INCLUDE_FILES.\n");
    }
}

bool __cdecl Material_GenerateShaderString_r(
                GfxAssembledShaderText *prog,
                const char *shaderName,
                const char *file,
                unsigned int fileSize,
                bool isInLibDir)
{
    bool atStartOfLine; // [esp+3h] [ebp-411h]
    char includeName[1024]; // [esp+4h] [ebp-410h] BYREF
    const char *parse; // [esp+408h] [ebp-Ch]
    int includeNameLen; // [esp+40Ch] [ebp-8h]
    unsigned int includeLine; // [esp+410h] [ebp-4h]

    atStartOfLine = 1;
    parse = file;
    while ( *parse )
    {
        if ( *parse == 47 && parse[1] == 42 )
        {
            Material_EmitShaderChar(prog, 32);
            for ( parse += 2; *parse != 42 || parse[1] != 47; ++parse )
            {
                if ( *parse == 10 )
                    Material_EmitShaderString(prog, "\n");
                if ( !*parse )
                    return 0;
            }
            parse += 2;
        }
        else if ( *parse == 47 && parse[1] == 47 )
        {
            Material_EmitShaderChar(prog, 10);
            for ( parse += 2; *parse != 10; ++parse )
            {
                if ( !*parse )
                    return !prog->overflowed;
            }
            ++parse;
            atStartOfLine = 1;
        }
        else if ( atStartOfLine && !strncmp(parse, "#include", 8u) && isspace(parse[8]) )
        {
            for ( parse += 9; isspace(*parse); ++parse )
                ;
            if ( *parse != 34 )
                Com_Error(ERR_DROP, "Found '%c' instead of '\"' in #include for shader %s\n", *parse, shaderName);
            ++parse;
            for ( includeNameLen = 0; parse[includeNameLen] != 34; ++includeNameLen )
            {
                if ( parse[includeNameLen] == 10 || !parse[includeNameLen] )
                    Com_Error(ERR_DROP, "Missing ending '\"' in #include for shader %s\n", shaderName);
                includeName[includeNameLen] = parse[includeNameLen];
            }
            includeName[includeNameLen] = 0;
            includeLine = prog->files[prog->fileCount - 1].srcLine
                                    + prog->currentDestLine
                                    - prog->files[prog->fileCount - 1].destLine;
            Material_AddShaderFile(prog, includeName, 1u);
            if ( !Material_IncludeShader(prog, includeName, isInLibDir) )
                return 0;
            Material_AddShaderFile(prog, shaderName, includeLine);
            Material_EmitShaderChar(prog, 10);
            for ( parse += includeNameLen + 1; *parse != 10; ++parse )
            {
                if ( !*parse )
                    return !prog->overflowed;
            }
            ++parse;
            atStartOfLine = 1;
        }
        else
        {
            Material_EmitShaderChar(prog, *parse);
            if ( *parse == 10 )
            {
                atStartOfLine = 1;
            }
            else if ( !isspace(*parse) )
            {
                atStartOfLine = 0;
            }
            ++parse;
        }
    }
    Material_EmitShaderChar(prog, 10);
    return !prog->overflowed;
}

void __cdecl Material_EmitShaderChar(GfxAssembledShaderText *prog, char ch)
{
    if ( prog->overflowed )
    {
        Com_ScriptError("Shader text overflowed. You may need to increase R_SHADER_TEXT_SIZE_LIMIT.\n");
    }
    else
    {
        prog->string[prog->used++] = ch;
        if ( ch == 10 )
            ++prog->currentDestLine;
        prog->overflowed = prog->used == prog->total;
    }
}

bool __cdecl Material_IncludeShader(GfxAssembledShaderText *prog, char *includeName, bool isInLibDir)
{
    char extendedName[64]; // [esp+0h] [ebp-50h] BYREF
    const char *file; // [esp+44h] [ebp-Ch] BYREF
    unsigned int fileSize; // [esp+48h] [ebp-8h] BYREF
    bool hasLibPrefix; // [esp+4Fh] [ebp-1h]

    //hasLibPrefix = strnicmp(includeName, "lib/", 4u) == 0;
    hasLibPrefix = _strnicmp(includeName, "lib/", 4u) == 0;
    if ( isInLibDir && !hasLibPrefix )
    {
        Com_sprintf(extendedName, 0x40u, "lib/%s", includeName);
        includeName = extendedName;
    }
    if ( Material_FindCachedShaderText(includeName, &file, &fileSize) )
        return Material_GenerateShaderString_r(prog, includeName, file, fileSize, 1);
    Com_ScriptError("Didn't preload shader file '%s'\n", includeName);
    return 0;
}

char __cdecl Material_FindCachedShaderText(const char *filename, const char **data, unsigned int *byteCount)
{
    int top; // [esp+0h] [ebp-10h]
    int bot; // [esp+4h] [ebp-Ch]
    int comparison; // [esp+8h] [ebp-8h]
    int mid; // [esp+Ch] [ebp-4h]

    bot = 0;
    top = mtlLoadGlob.cachedShaderCount - 1;
    while ( bot <= top )
    {
        mid = (top + bot) >> 1;
        comparison = I_stricmp(filename, mtlLoadGlob.cachedShaderText[mid].name);
        if ( !comparison )
        {
            *byteCount = mtlLoadGlob.cachedShaderText[mid].textSize;
            *data = mtlLoadGlob.cachedShaderText[mid].text;
            return 1;
        }
        if ( comparison >= 0 )
            bot = mid + 1;
        else
            top = mid - 1;
    }
    return 0;
}

void __cdecl Material_FileIncludeFileAndLineNumber(
                GfxAssembledShaderText *prog,
                char *errorMessage,
                char **fileName,
                unsigned int *lineNumber)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    if ( Material_ParseLineNumber(errorMessage, lineNumber) )
    {
        if ( *lineNumber < prog->files[0].destLine
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                        4676,
                        0,
                        "*lineNumber >= prog->files[0].destLine\n\t%i, %i",
                        *lineNumber,
                        prog->files[0].destLine) )
        {
            __debugbreak();
        }
        for ( i = 1; i < prog->fileCount && *lineNumber >= prog->files[i].destLine; ++i )
            ;
        *fileName = prog->files[i - 1].fileName;
        *lineNumber = prog->files[i - 1].srcLine + *lineNumber - prog->files[i - 1].destLine;
    }
    else
    {
        *fileName = (char*)"Message format changed.    Update Material_FileIncludeFileAndLineNumber.";
        *lineNumber = 0;
    }
}

bool __cdecl Material_ParseLineNumber(char *errorMessage, unsigned int *lineNumber)
{
    const char *v2; // eax
    const char *lineNumberStart; // [esp+0h] [ebp-8h]
    int charNumber; // [esp+4h] [ebp-4h] BYREF

    v2 = strchr(errorMessage, 0x28u);
    lineNumberStart = v2;
    if ( !v2 )
        return 0;
    if ( sscanf(v2, "(%d,%d)", lineNumber, &charNumber) == 2 )
        return 1;
    return sscanf(lineNumberStart, "(%d)", lineNumber) == 1;
}

const char *IW_PROJECT_GAMEDIR = getenv("IW_PROJECT_GAMEDIR");
char __cdecl Material_FindCachedShaderDX(
                char *shaderText,
                unsigned int shaderTextLen,
                const char *entryPoint,
                const char *target,
                ID3DXBuffer **shader,
                const char *shaderName,
                int useUPDB)
{
    unsigned int shaderLen; // [esp+4h] [ebp-124h] BYREF
    char filename[264]; // [esp+8h] [ebp-120h] BYREF
    unsigned int checksum[4]; // [esp+114h] [ebp-14h] BYREF
    void *cachedShader; // [esp+124h] [ebp-4h] BYREF

    Com_BlockChecksum128((unsigned __int8 *)shaderText, shaderTextLen, useUPDB, (unsigned __int8 *)checksum);
    if ( IW_PROJECT_GAMEDIR )
        Com_sprintf(
            filename,
            0x104u,
            "%s/%s/shadercache%d/%s_%s_%8x%8x%8x%8x",
            IW_PROJECT_GAMEDIR,
            "pc",
            2,
            entryPoint,
            target,
            checksum[0],
            checksum[1],
            checksum[2],
            checksum[3]);
    else
        Com_sprintf(
            filename,
            0x104u,
            "%s/%s/shadercache%d/%s_%s_%8x%8x%8x%8x",
            "..",
            "pc",
            2,
            entryPoint,
            target,
            checksum[0],
            checksum[1],
            checksum[2],
            checksum[3]);
    if ( !Material_FindCachedShader(shaderText, shaderTextLen, filename, &cachedShader, &shaderLen) )
        return 0;
    if ( Material_CopyTextToDXBuffer((unsigned __int8 *)cachedShader, shaderLen, shader) )
    {
        Z_Free((char *)cachedShader, 32);
        return 1;
    }
    else
    {
        Z_Free((char *)cachedShader, 32);
        return 0;
    }
}

char __cdecl Material_FindCachedShader(
                const char *shaderText,
                unsigned int shaderTextLen,
                const char *filename,
                void **cachedShader,
                unsigned int *shaderLen)
{
    unsigned int v6; // eax
    unsigned int cachedShaderTextLen; // [esp+8h] [ebp-Ch] BYREF
    _iobuf *cacheFile; // [esp+Ch] [ebp-8h]
    char *cachedShaderText; // [esp+10h] [ebp-4h]

    Material_DeleteOldCachedShaders();
    *cachedShader = 0;
    *shaderLen = 0;
    cacheFile = fopen(filename, "rb");
    if ( !cacheFile )
        return 0;
    if ( fread(&cachedShaderTextLen, 4u, 1u, cacheFile) == 1 && cachedShaderTextLen == shaderTextLen )
    {
        cachedShaderText = (char *)Z_Malloc(cachedShaderTextLen, "Material_FindCachedShader", 32);
        v6 = fread(cachedShaderText, 1u, cachedShaderTextLen, cacheFile);
        if ( cachedShaderTextLen == v6
            && !memcmp(cachedShaderText, shaderText, cachedShaderTextLen)
            && fread(shaderLen, 4u, 1u, cacheFile) == 1 )
        {
            *cachedShader = Z_Malloc(*shaderLen, "Material_FindCachedShader", 32);
            if ( !*cachedShader
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                            4611,
                            0,
                            "%s",
                            "*cachedShader") )
            {
                __debugbreak();
            }
            fread(*cachedShader, 1u, *shaderLen, cacheFile);
            Z_Free(cachedShaderText, 32);
            fclose(cacheFile);
            return 1;
        }
        else
        {
            Z_Free(cachedShaderText, 32);
            fclose(cacheFile);
            return 0;
        }
    }
    else
    {
        fclose(cacheFile);
        return 0;
    }
}

void Material_DeleteOldCachedShaders()
{
    char dirname[264]; // [esp+8h] [ebp-110h] BYREF
    unsigned int oldShaderCacheVersion; // [esp+114h] [ebp-4h]

    static bool once = false;
    if ( !once )
    {
        once = 1;
        for ( oldShaderCacheVersion = 1; oldShaderCacheVersion < 2; ++oldShaderCacheVersion )
        {
            if ( IW_PROJECT_GAMEDIR )
                Com_sprintf(dirname, 0x104u, "%s/%s/shadercache%d", IW_PROJECT_GAMEDIR, "pc", oldShaderCacheVersion);
            else
                Com_sprintf(dirname, 0x104u, "%s/%s/shadercache%d", "..", "pc", oldShaderCacheVersion);
            Material_DeleteDirectory(dirname);
        }
        if ( IW_PROJECT_GAMEDIR )
            Com_sprintf(dirname, 0x104u, "%s/%s/shadercache%d", IW_PROJECT_GAMEDIR, "pc", 2);
        else
            Com_sprintf(dirname, 0x104u, "%s/%s/shadercache%d", "..", "pc", 2);
        Material_DeleteOldFilesInDirectory(dirname, 0xAu);
    }
}

void __cdecl Material_DeleteOldFilesInDirectory(const char *dirname, unsigned __int16 daysOld)
{
    unsigned int errorCode; // [esp+0h] [ebp-378h]
    HANDLE handle; // [esp+4h] [ebp-374h]
    _WIN32_FIND_DATAA findData; // [esp+8h] [ebp-370h] BYREF
    char fullfilename[264]; // [esp+148h] [ebp-230h] BYREF
    char fulldirname[268]; // [esp+250h] [ebp-128h] BYREF
    _SYSTEMTIME sysTime; // [esp+360h] [ebp-18h] BYREF
    _FILETIME fileTime; // [esp+370h] [ebp-8h] BYREF

    GetSystemTime(&sysTime);
    Material_SubtractDays(&sysTime, daysOld);
    SystemTimeToFileTime(&sysTime, &fileTime);
    Com_sprintf(fulldirname, 0x104u, "%s/*", dirname);
    handle = FindFirstFileA(fulldirname, &findData);
    if ( handle != (HANDLE)-1 )
    {
        do
        {
            if ( (findData.dwFileAttributes & 0x11) == 0 && CompareFileTime(&findData.ftLastAccessTime, &fileTime) < 0 )
            {
                Com_sprintf(fullfilename, 0x104u, "%s/%s", dirname, findData.cFileName);
                if ( !DeleteFileA(fullfilename) )
                {
                    errorCode = GetLastError();
                    Com_PrintError(1, "ERROR: Failed to delete %s errorCode %d\n", fullfilename, errorCode);
                }
            }
        }
        while ( FindNextFileA(handle, &findData) );
        FindClose(handle);
    }
}

void __cdecl Material_SubtractDays(_SYSTEMTIME *sysTime, unsigned __int16 daysOld)
{
    if ( daysOld > 0x14u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    4437,
                    0,
                    "%s",
                    "daysOld <= 20") )
    {
        __debugbreak();
    }
    if ( sysTime->wDay <= (int)daysOld )
    {
        sysTime->wDay = sysTime->wDay + 28 - daysOld;
        if ( sysTime->wMonth <= 1u )
        {
            sysTime->wMonth = 12;
            --sysTime->wYear;
        }
        else
        {
            --sysTime->wMonth;
        }
    }
    else
    {
        sysTime->wDay -= daysOld;
    }
}

void __cdecl Material_DeleteDirectory(const char *dirname)
{
    unsigned int errorCode; // [esp+0h] [ebp-360h]
    HANDLE handle; // [esp+4h] [ebp-35Ch]
    _WIN32_FIND_DATAA findData; // [esp+8h] [ebp-358h] BYREF
    char fullfilename[264]; // [esp+148h] [ebp-218h] BYREF
    char fulldirname[268]; // [esp+250h] [ebp-110h] BYREF

    Com_sprintf(fulldirname, 0x104u, "%s/*", dirname);
    handle = FindFirstFileA(fulldirname, &findData);
    if ( handle != (HANDLE)-1 )
    {
        do
        {
            if ( (findData.dwFileAttributes & 0x11) == 0 )
            {
                Com_sprintf(fullfilename, 0x104u, "%s/%s", dirname, findData.cFileName);
                if ( !DeleteFileA(fullfilename) )
                {
                    errorCode = GetLastError();
                    Com_PrintError(1, "ERROR: Failed to delete %s errorCode %d\n", fullfilename, errorCode);
                }
            }
        }
        while ( FindNextFileA(handle, &findData) );
        FindClose(handle);
    }
    RemoveDirectoryA(dirname);
}

char __cdecl Material_CopyTextToDXBuffer(unsigned __int8 *cachedShader, unsigned int shaderLen, ID3DXBuffer **shader)
{
    const char *v3; // eax
    unsigned __int8 *v5; // eax
    int hr; // [esp+0h] [ebp-4h]

    hr = D3DXCreateBuffer(shaderLen, shader);
    if ( hr >= 0 )
    {
        v5 = (unsigned __int8 *)(*shader)->GetBufferPointer();
        memcpy(v5, cachedShader, shaderLen);
        return 1;
    }
    else
    {
        v3 = R_ErrorDescription(hr);
        Com_PrintError(
            8,
            "ERROR: Material_CopyTextToDXBuffer: D3DXCreateBuffer(%d) failed: %s (0x%08x)\n",
            shaderLen,
            v3,
            hr);
        free(cachedShader);
        return 0;
    }
}

void __cdecl Material_CacheShaderDX(
                char *shaderText,
                unsigned int shaderTextLen,
                const char *entryPoint,
                const char *target,
                ID3DXBuffer *shader,
                int useUPDB)
{
    const void *v6; // eax
    const char *v7; // [esp+0h] [ebp-22Ch]
    char filename[264]; // [esp+4h] [ebp-228h] BYREF
    unsigned int checksum[4]; // [esp+10Ch] [ebp-120h] BYREF
    char dirname[268]; // [esp+11Ch] [ebp-110h] BYREF

    Com_BlockChecksum128((unsigned __int8 *)shaderText, shaderTextLen, useUPDB, (unsigned __int8 *)checksum);
    if ( IW_PROJECT_GAMEDIR )
    {
        v7 = IW_PROJECT_GAMEDIR;
        Com_sprintf(dirname, 0x104u, "%s/%s/shadercache%d", IW_PROJECT_GAMEDIR, "pc", 2);
    }
    else
    {
        v7 = "..";
        Com_sprintf(dirname, 0x104u, "%s/%s/shadercache%d", "..", "pc", 2);
    }
    Sys_Mkdir(dirname);
    Com_sprintf(
        filename,
        0x104u,
        "%s/%s_%s_%8x%8x%8x%8x",
        dirname,
        entryPoint,
        target,
        checksum[0],
        checksum[1],
        checksum[2],
        checksum[3]);
    //((void (__stdcall *)(ID3DXBuffer *, const char *))shader->GetBufferSize)(shader, v7);
    //v6 = (const void *)((int (__thiscall *)(ID3DXBuffer *))shader->GetBufferPointer)(shader);
    //Material_CacheShader(shaderText, shaderTextLen, filename, v6, (unsigned int)shader);

    Material_CacheShader(shaderText, shaderTextLen, filename, shader->GetBufferPointer(), shader->GetBufferSize());
}

void __cdecl Material_CacheShader(
                const char *shaderText,
                unsigned int shaderTextLen,
                const char *filename,
                const void *shaderBuffer,
                unsigned int shaderLen)
{
    char fileFullPath[1024]; // [esp+0h] [ebp-408h] BYREF
    _iobuf *cacheFile; // [esp+404h] [ebp-4h]

    cacheFile = fopen(filename, "wb");
    if ( cacheFile )
    {
        fwrite(&shaderTextLen, 4u, 1u, cacheFile);
        fwrite(shaderText, 1u, shaderTextLen, cacheFile);
        fwrite(&shaderLen, 4u, 1u, cacheFile);
        fwrite(shaderBuffer, 1u, shaderLen, cacheFile);
        fclose(cacheFile);
    }
    else
    {
        memset(fileFullPath, 0, sizeof(fileFullPath));
        if ( GetFullPathNameA(filename, 0x400u, fileFullPath, 0) )
            Com_PrintWarning(10, "Material_CacheShader: Failed to open for writing '%s'\n", fileFullPath);
        else
            Com_PrintWarning(10, "Material_CacheShader: Failed to open for writing '%s'\n", filename);
    }
}

void __cdecl Material_GetShaderTargetString(char *target, unsigned int maxChars, const char *prefix)
{
    Com_sprintf(target, maxChars, "%s_3_0", prefix);
}

char __cdecl Material_GetVertexShaderHashIndex(const char *shaderName, unsigned int *foundHashIndex)
{
    unsigned int hashIndex; // [esp+14h] [ebp-8h]

    if ( !shaderName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    7471,
                    0,
                    "%s",
                    "shaderName") )
    {
        __debugbreak();
    }
    if ( !foundHashIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    7472,
                    0,
                    "%s",
                    "foundHashIndex") )
    {
        __debugbreak();
    }
    for ( hashIndex = R_HashAssetName(shaderName) % 0x1400;
                mtlLoadGlob.vertexShaderHashTable[hashIndex];
                hashIndex = (hashIndex + 1) % 0x1400 )
    {
        if ( !strcmp(mtlLoadGlob.vertexShaderHashTable[hashIndex]->name, shaderName) )
        {
            *foundHashIndex = hashIndex;
            return 1;
        }
    }
    *foundHashIndex = hashIndex;
    return 0;
}

int __cdecl Material_ParseShaderVersion(const char **text)
{
    float versionNumber; // [esp+0h] [ebp-4h]

    versionNumber = Com_ParseFloat(text);
    return (int)(float)((float)(versionNumber * 10.0) + 0.5);
}

char __cdecl Material_SetPassShaderArguments_DX(
                char **text,
                char *shaderName,
                MaterialShaderType shaderType,
                const DWORD *program,
                unsigned __int16 *techFlags,
                ShaderParameterSet *paramSet,
                unsigned int argLimit,
                unsigned int *argCount,
                MaterialShaderArgument *args)
{
    const char *v9; // eax
    HRESULT v11; // [esp-4h] [ebp-10A8h]
    _D3DXSHADER_CONSTANTTABLE *constantTable; // [esp+0h] [ebp-10A4h]
    _D3DXSEMANTIC semantic[512]; // [esp+4h] [ebp-10A0h] BYREF
    LPD3DXCONSTANTTABLE constants; // [esp+1008h] [ebp-9Ch] BYREF
    HRESULT hr; // [esp+1010h] [ebp-94h]
    unsigned int v17; // [esp+1014h] [ebp-90h] BYREF
    char v18; // [esp+101Bh] [ebp-89h]
    _D3DXSEMANTIC v19[16]; // [esp+101Ch] [ebp-88h] BYREF
    unsigned int i; // [esp+10A0h] [ebp-4h]

    hr = D3DXGetShaderConstantTable(program, &constants);
    if ( hr >= 0 )
    {
        iassert(constants);
        constantTable = (_D3DXSHADER_CONSTANTTABLE*)constants->GetBufferPointer();
        paramSet->uniformInputCount = Material_PrepareToParseShaderArguments(constantTable, paramSet->uniformInputs);
        v18 = Material_ParseShaderArguments(
                        text,
                        shaderName,
                        shaderType,
                        paramSet->uniformInputs,
                        paramSet->uniformInputCount,
                        techFlags,
                        argLimit,
                        argCount,
                        args);
        //(*(void (__stdcall **)(int))(*(unsigned int *)constants + 8))(constants);
        constants->Release();
        if ( v18 )
        {
            UINT count;
            hr = D3DXGetShaderInputSemantics(program, semantic, &count);
            paramSet->varyingInputCount = 0;
            for ( i = 0; i < count; ++i )
            {
                Material_SetVaryingParameterDef(&semantic[i], &paramSet->varyingInputs[paramSet->varyingInputCount]);
                ++paramSet->varyingInputCount;
            }
            hr = D3DXGetShaderOutputSemantics(program, v19, &v17);
            paramSet->outputCount = 0;
            for ( i = 0; i < v17; ++i )
            {
                if ( v19[i].Usage )
                {
                    if ( v19[i].Usage != 11 )
                    {
                        Material_SetVaryingParameterDef(&v19[i], &paramSet->outputs[paramSet->outputCount]);
                        ++paramSet->outputCount;
                    }
                }
            }
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        v11 = hr;
        v9 = R_ErrorDescription(hr);
        Com_ScriptError("Couldn't get the constant table: %s (%08x)\n", v9, v11);
        return 0;
    }
}

unsigned int __cdecl Material_PrepareToParseShaderArguments(
                _D3DXSHADER_CONSTANTTABLE *constantTable,
                ShaderUniformDef *paramTable)
{
    unsigned int constantIndex; // [esp+0h] [ebp-8h]
    unsigned int usedCount; // [esp+4h] [ebp-4h]

    usedCount = 0;
    for ( constantIndex = 0; constantIndex < constantTable->Constants; ++constantIndex )
        usedCount += R_SetParameterDefArray(constantTable, constantIndex, &paramTable[usedCount]);
    return usedCount;
}

unsigned int __cdecl R_SetParameterDefArray(
                _D3DXSHADER_CONSTANTTABLE *constantTable,
                unsigned int constantIndex,
                ShaderUniformDef *paramDef)
{
    unsigned int result; // eax
    char *typeInfo; // [esp+4h] [ebp-18h]
    char *name; // [esp+8h] [ebp-14h]
    unsigned int paramDefIndex; // [esp+Ch] [ebp-10h]
    bool isTransposed; // [esp+13h] [ebp-9h]
    const _D3DXSHADER_CONSTANTINFO *constantInfo; // [esp+14h] [ebp-8h]
    ShaderParamType type; // [esp+18h] [ebp-4h]

    constantInfo = (const _D3DXSHADER_CONSTANTINFO *)&BufferOffset((char *)constantTable, constantTable->ConstantInfo)[20 * constantIndex];
    typeInfo = BufferOffset((char *)constantTable, constantInfo->TypeInfo);
    name = BufferOffset((char *)constantTable, constantInfo->Name);
    isTransposed = *(unsigned __int16 *)typeInfo == 3;
    switch ( *((_WORD *)typeInfo + 1) )
    {
        case 1:
            type = SHADER_PARAM_FLOAT4;
            goto LABEL_9;
        case 3:
            type = SHADER_PARAM_FLOAT4;
            goto LABEL_9;
        case 0xB:
            type = SHADER_PARAM_SAMPLER_1D;
            goto LABEL_9;
        case 0xC:
            type = SHADER_PARAM_SAMPLER_2D;
            goto LABEL_9;
        case 0xD:
            type = SHADER_PARAM_SAMPLER_3D;
            goto LABEL_9;
        case 0xE:
            type = SHADER_PARAM_SAMPLER_CUBE;
LABEL_9:
            for ( paramDefIndex = 0; paramDefIndex < constantInfo->RegisterCount; ++paramDefIndex )
            {
                paramDef->type = type;
                paramDef->name = name;
                paramDef->index = paramDefIndex;
                paramDef->resourceDest = paramDefIndex + constantInfo->RegisterIndex;
                paramDef->isTransposed = isTransposed;
                paramDef->isAssigned = 0;
                ++paramDef;
            }
            result = paramDefIndex;
            break;
        default:
            Com_ScriptError("Unknown constant type '%i'", *((unsigned __int16 *)typeInfo + 1));
            result = 0;
            break;
    }
    return result;
}

char *__cdecl BufferOffset(char *buffer, int offset)
{
    return &buffer[offset];
}

char __cdecl Material_ParseShaderArguments(
                char **text,
                char *shaderName,
                MaterialShaderType shaderType,
                ShaderUniformDef *paramTable,
                unsigned int paramCount,
                unsigned __int16 *techFlags,
                unsigned int argLimit,
                unsigned int *argCount,
                MaterialShaderArgument *args)
{
    unsigned __int16 v10[2]; // [esp+0h] [ebp-514Ch] BYREF
    unsigned __int16 consts[4]; // [esp+4h] [ebp-5148h] BYREF
    ShaderArgumentSource argSource; // [esp+Ch] [ebp-5140h] BYREF
    unsigned int usedCount; // [esp+20h] [ebp-512Ch] BYREF
    char dest[256]; // [esp+24h] [ebp-5128h] BYREF
    unsigned int i; // [esp+124h] [ebp-5028h]
    ShaderArgumentDest indexRange; // [esp+128h] [ebp-5024h] BYREF
    ShaderParamType paramType; // [esp+138h] [ebp-5014h] BYREF
    unsigned int arrayCount; // [esp+13Ch] [ebp-5010h]
    char *src; // [esp+140h] [ebp-500Ch]
    MaterialShaderArgument argTable; // [esp+144h] [ebp-5008h] BYREF
    char dst[256][64]; // [esp+1144h] [ebp-4008h] BYREF

    memset(dst, 0, sizeof(dst));
    if ( !techFlags
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    4299,
                    0,
                    "%s",
                    "techFlags") )
    {
        __debugbreak();
    }
    if ( !paramTable
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    4300,
                    0,
                    "%s",
                    "paramTable") )
    {
        __debugbreak();
    }
    usedCount = 0;
    if ( !Material_MatchToken((const char **)text, "{") )
        return 0;
    while ( 1 )
    {
        src = (char *)Com_Parse((const char **)text);
        if ( !*src )
        {
            Com_ScriptError("unexpected end-of-file\n");
            return 0;
        }
        if ( *src == 125 )
            break;
        I_strncpyz(dest, src, 256);
        arrayCount = Material_ElemCountForParamName(shaderName, paramTable, paramCount, dest, &paramType);
        if ( arrayCount )
        {
            if ( !Material_ParseIndexRange((const char **)text, arrayCount, &indexRange.indexRange) )
                return 0;
            indexRange.paramName = dest;
            if ( !Material_ParseArgumentSource(shaderType, (const char **)text, shaderName, paramType, &argSource) )
                return 0;
            if ( !Material_MatchToken((const char **)text, ";") )
                return 0;
            if ( !Material_AddShaderArgument(
                            shaderName,
                            shaderType,
                            &argSource,
                            &indexRange,
                            paramTable,
                            paramCount,
                            &usedCount,
                            &argTable,
                            dst) )
                return 0;
            if ( argSource.type == 4 )
            {
                switch ( argSource.u.codeIndex )
                {
                    case 9u:
                        *techFlags |= 1u;
                        break;
                    case 0xAu:
                        *techFlags |= 2u;
                        break;
                    case 0x13u:
                    case 0x14u:
                    case 0x15u:
                        *techFlags |= 0x40u;
                        break;
                }
            }
        }
        else
        {
            if ( !Material_MatchToken((const char **)text, "=") )
                return 0;
            Com_SkipRestOfLine((const char **)text);
        }
    }
    if ( usedCount == paramCount )
        return Material_SetShaderArguments(usedCount, &argTable, argLimit, argCount, args);
    for ( i = 0; i < paramCount; ++i )
    {
        if ( !paramTable[i].isAssigned )
        {
            indexRange.paramName = paramTable[i].name;
            indexRange.indexRange.first = paramTable[i].index;
            indexRange.indexRange.count = 1;
            indexRange.indexRange.isImplicit = 0;
            if ( Material_DefaultArgumentSource(
                         shaderType,
                         paramTable[i].name,
                         paramTable[i].type,
                         &indexRange.indexRange,
                         &argSource) )
            {
                if ( argSource.type == 5 )
                {
                    if ( argSource.u.codeIndex == 4 )
                        *techFlags |= 0x10u;
                }
                else if ( argSource.type == 4
                             && (argSource.u.codeIndex == 19 || argSource.u.codeIndex == 20 || argSource.u.codeIndex == 21) )
                {
                    *techFlags |= 0x40u;
                }
                if ( argSource.type == 3 && argSource.u.codeIndex == 120 )
                    *techFlags |= 0x100u;
                if ( argSource.type == 3 && (*techFlags & 0x20) == 0 )
                {
                    consts[0] = 81;
                    consts[1] = 82;
                    consts[2] = 83;
                    consts[3] = 84;
                    if ( CodeConstIsOneOf(argSource.u.codeIndex, consts, 4) )
                        *techFlags |= 0x20u;
                }
                if ( argSource.type == 3 && (*techFlags & 0x200) == 0 )
                {
                    v10[0] = 114;
                    v10[1] = 118;
                    if ( CodeConstIsOneOf(argSource.u.codeIndex, v10, 2) )
                        *techFlags |= 0x200u;
                }
                if ( !Material_AddShaderArgument(
                                shaderName,
                                shaderType,
                                &argSource,
                                &indexRange,
                                paramTable,
                                paramCount,
                                &usedCount,
                                &argTable,
                                dst) )
                    return 0;
            }
        }
    }
    if ( usedCount == paramCount )
        return Material_SetShaderArguments(usedCount, &argTable, argLimit, argCount, args);
    Com_PrintWarning(8, "Undefined shader parameter(s) in %s\n", shaderName);
    for ( i = 0; i < paramCount; ++i )
    {
        if ( !paramTable[i].isAssigned )
            Com_PrintWarning(8, "    %s\n", paramTable[i].name);
    }
    Com_PrintWarning(8, "%i parameter(s) were undefined\n", paramCount - usedCount);
    return 0;
}

bool __cdecl Material_ParseIndexRange(const char **text, unsigned int arrayCount, ShaderIndexRange *indexRange)
{
    unsigned int last; // [esp+4h] [ebp-4h]

    if ( Com_Parse(text)->token[0] == 91 )
    {
        indexRange->isImplicit = 0;
        indexRange->first = Com_ParseInt(text);
        if ( indexRange->first < arrayCount )
        {
            if ( Com_Parse(text)->token[0] == 44 )
            {
                last = Com_ParseInt(text);
                if ( last >= indexRange->first && last < arrayCount )
                {
                    return Material_MatchToken(text, "]");
                }
                else
                {
                    Com_ScriptError("ending index %i is not in the range [%i, %i]\n", last, indexRange->first, arrayCount - 1);
                    return 0;
                }
            }
            else
            {
                Com_UngetToken();
                indexRange->count = 1;
                return 1;
            }
        }
        else
        {
            Com_ScriptError("index %i is not in the range [0, %i]\n", indexRange->first, arrayCount - 1);
            return 0;
        }
    }
    else
    {
        Com_UngetToken();
        indexRange->first = 0;
        indexRange->count = arrayCount;
        indexRange->isImplicit = 1;
        return 1;
    }
}

unsigned int __cdecl Material_ElemCountForParamName(
                const char *shaderName,
                const ShaderUniformDef *paramTable,
                unsigned int paramCount,
                const char *name,
                ShaderParamType *paramType)
{
    unsigned int paramIndex; // [esp+14h] [ebp-8h]
    unsigned int count; // [esp+18h] [ebp-4h]

    count = 0;
    for ( paramIndex = 0; paramIndex < paramCount; ++paramIndex )
    {
        if ( !strcmp(name, paramTable[paramIndex].name) )
        {
            if ( count && paramTable[paramIndex].type != *paramType )
                Com_Error(ERR_DROP, "param type changed from %i to %i", paramTable[paramIndex].type, *paramType);
            *paramType = paramTable[paramIndex].type;
            if ( count <= paramTable[paramIndex].index )
                count = paramTable[paramIndex].index + 1;
            if ( !count
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                            3830,
                            0,
                            "%s",
                            "count > 0") )
            {
                __debugbreak();
            }
        }
    }
    return count;
}

bool __cdecl Material_ParseArgumentSource(
                MaterialShaderType shaderType,
                const char **text,
                const char *shaderName,
                ShaderParamType paramType,
                ShaderArgumentSource *argSource)
{
    const char *v6; // eax

    if ( !text
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp", 3841, 0, "%s", "text") )
    {
        __debugbreak();
    }
    if ( !*text
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp", 3842, 0, "%s", "*text") )
    {
        __debugbreak();
    }
    if ( !shaderName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    3843,
                    0,
                    "%s",
                    "shaderName") )
    {
        __debugbreak();
    }
    if ( !argSource
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    3844,
                    0,
                    "%s",
                    "argSource") )
    {
        __debugbreak();
    }
    if ( !Material_MatchToken(text, "=") )
        return 0;
    if ( paramType == SHADER_PARAM_FLOAT4 )
        return Material_ParseConstantSource(shaderType, text, argSource);
    if ( paramType > SHADER_PARAM_FLOAT4 && paramType <= SHADER_PARAM_SAMPLER_1D )
        return Material_ParseSamplerSource(text, argSource);
    v6 = va("unknown constant type '%i'\n", paramType);
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp", 3861, 1, v6) )
        __debugbreak();
    return 0;
}

bool __cdecl Material_ParseSamplerSource(const char **text, ShaderArgumentSource *argSource)
{
    parseInfo_t *v3; // eax
    parseInfo_t *token; // [esp+28h] [ebp-4h]

    token = Com_Parse(text);
    if ( !strcmp(token->token, "sampler") )
        return Material_CodeSamplerSource_r(text, 0, s_codeSamplers, argSource);
    if ( !strcmp(token->token, "material") )
    {
        if ( Material_MatchToken(text, ".") )
        {
            v3 = Com_Parse(text);
            argSource->type = 2;
            argSource->u.literalConst = (const float *)Material_RegisterString(v3->token);
            argSource->indexRange.first = 0;
            argSource->indexRange.count = 1;
            argSource->indexRange.isImplicit = 1;
            return argSource->u.literalConst != 0;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_ScriptError("expected 'sampler' or 'material', found '%s' instead\n", token->token);
        return 0;
    }
}

const char *__cdecl Material_RegisterString(char *string)
{
    const char *v1; // eax
    unsigned int v3; // [esp+0h] [ebp-48h]
    unsigned __int8 *buffer; // [esp+38h] [ebp-10h]
    unsigned int hash; // [esp+3Ch] [ebp-Ch]
    unsigned int hashIndex; // [esp+40h] [ebp-8h]

    hash = R_HashString(string);
    for ( hashIndex = hash & 0x3FF; mtlLoadGlob.stringHashTable[hashIndex].string; hashIndex = ((_WORD)hashIndex + 1)
                                                                                                                                                                                     & 0x3FF )
    {
        if ( mtlLoadGlob.stringHashTable[hashIndex].hash == hash
            && !strcmp(mtlLoadGlob.stringHashTable[hashIndex].string, string) )
        {
            if ( strcmp(mtlLoadGlob.stringHashTable[hashIndex].string, string) )
            {
                v1 = va("%s != %s", mtlLoadGlob.stringHashTable[hashIndex].string, string);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                                2209,
                                0,
                                "%s\n\t%s",
                                "!strcmp( mtlLoadGlob.stringHashTable[hashIndex].string, string )",
                                v1) )
                    __debugbreak();
            }
            return mtlLoadGlob.stringHashTable[hashIndex].string;
        }
    }
    if ( ++mtlLoadGlob.stringCount == 1024 )
        Com_Error(ERR_DROP, "More than %i string identifiers used by shaders", 1023);
    v3 = strlen(string);
    buffer = Material_Alloc(v3 + 1);
    memcpy(buffer, (unsigned __int8 *)string, v3 + 1);
    mtlLoadGlob.stringHashTable[hashIndex].string = (const char *)buffer;
    mtlLoadGlob.stringHashTable[hashIndex].hash = hash;
    return mtlLoadGlob.stringHashTable[hashIndex].string;
}

bool __cdecl Material_CodeSamplerSource_r(
                const char **text,
                int offset,
                const CodeSamplerSource *sourceTable,
                ShaderArgumentSource *argSource)
{
    int sourceIndex; // [esp+14h] [ebp-Ch]
    int additionalOffset; // [esp+18h] [ebp-8h] BYREF
    const char *token; // [esp+1Ch] [ebp-4h]

    if ( !text
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp", 3511, 0, "%s", "text") )
    {
        __debugbreak();
    }
    if ( !sourceTable
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    3512,
                    0,
                    "%s",
                    "sourceTable") )
    {
        __debugbreak();
    }
    if ( !Material_MatchToken(text, ".") )
        return 0;
    token = (const char *)Com_Parse(text);
    for ( sourceIndex = 0; ; ++sourceIndex )
    {
        if ( !sourceTable[sourceIndex].name )
        {
            Com_ScriptError("unknown sampler source '%s'\n", token);
            return 0;
        }
        if ( !strcmp(token, sourceTable[sourceIndex].name) )
            break;
    }
    if ( sourceTable[sourceIndex].subtable )
    {
        if ( sourceTable[sourceIndex].arrayCount )
        {
            if ( !Material_ParseArrayOffset(
                            text,
                            sourceTable[sourceIndex].arrayCount,
                            sourceTable[sourceIndex].arrayStride,
                            &additionalOffset) )
                return 0;
            offset += additionalOffset;
        }
        return Material_CodeSamplerSource_r(text, offset, sourceTable[sourceIndex].subtable, argSource);
    }
    else
    {
        argSource->type = 4;
        argSource->u.codeIndex = offset + sourceTable[sourceIndex].source;
        if ( sourceTable[sourceIndex].arrayCount )
        {
            if ( sourceTable[sourceIndex].arrayStride != 1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                            3538,
                            0,
                            "%s\n\t(sourceIndex) = %i",
                            "(sourceTable[sourceIndex].arrayStride == 1)",
                            sourceIndex) )
            {
                __debugbreak();
            }
            return Material_ParseIndexRange(text, sourceTable[sourceIndex].arrayCount, &argSource->indexRange);
        }
        else
        {
            argSource->indexRange.first = 0;
            argSource->indexRange.count = 1;
            argSource->indexRange.isImplicit = 1;
            return 1;
        }
    }
}

char __cdecl Material_ParseArrayOffset(const char **text, int arrayCount, int arrayStride, int *offset)
{
    int arrayIndex; // [esp+0h] [ebp-4h]

    if ( !Material_MatchToken(text, "[") )
        return 0;
    arrayIndex = Com_ParseInt(text);
    if ( arrayIndex >= 0 && arrayIndex < arrayCount )
    {
        if ( Material_MatchToken(text, "]") )
        {
            *offset = arrayStride * arrayIndex;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_ScriptError("array index must be in range [0, %i]\n", arrayCount - 1);
        return 0;
    }
}

bool __cdecl Material_ParseConstantSource(
                MaterialShaderType shaderType,
                const char **text,
                ShaderArgumentSource *argSource)
{
    float literal[4]; // [esp+28h] [ebp-14h] BYREF
    const char *token; // [esp+38h] [ebp-4h]

    token = (const char *)Com_Parse(text);
    if ( Material_ParseLiteral(text, token, literal) )
    {
        argSource->type = shaderType != MTL_VERTEX_SHADER ? 7 : 1;
        argSource->u.literalConst = Material_RegisterLiteral(literal);
        argSource->indexRange.first = 0;
        argSource->indexRange.count = 1;
        argSource->indexRange.isImplicit = 1;
        return argSource->u.literalConst != 0;
    }
    else if ( !strcmp(token, "constant") )
    {
        return Material_ParseCodeConstantSource_r(shaderType, text, 0, s_codeConsts, argSource);
    }
    else if ( !strcmp(token, "material") )
    {
        if ( Material_MatchToken(text, ".") )
        {
            token = (const char *)Com_Parse(text);
            argSource->type = shaderType != MTL_VERTEX_SHADER ? 6 : 0;
            argSource->u.literalConst = (const float *)Material_RegisterString((char *)token);
            argSource->indexRange.first = 0;
            argSource->indexRange.count = 1;
            argSource->indexRange.isImplicit = 1;
            return argSource->u.literalConst != 0;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_ScriptError("expected 'sampler' or 'material', found '%s' instead\n", token);
        return 0;
    }
}

float *__cdecl Material_RegisterLiteral(float *literal)
{
    float *v3; // [esp+4h] [ebp-Ch]
    float *v4; // [esp+8h] [ebp-8h]
    unsigned int literalIndex; // [esp+Ch] [ebp-4h]

    for ( literalIndex = 0; literalIndex < mtlLoadGlob.literalCount; ++literalIndex )
    {
        v4 = mtlLoadGlob.literalTable[literalIndex];
        if ( *literal == *v4 && literal[1] == v4[1] && literal[2] == v4[2] && literal[3] == v4[3] )
            return mtlLoadGlob.literalTable[literalIndex];
    }
    if ( literalIndex == 32 )
        Com_Error(ERR_DROP, "more than %i shader literals used", 32);
    ++mtlLoadGlob.literalCount;
    v3 = mtlLoadGlob.literalTable[literalIndex];
    *v3 = *literal;
    v3[1] = literal[1];
    v3[2] = literal[2];
    v3[3] = literal[3];
    return v3;
}

char __cdecl Material_ParseLiteral(const char **text, const char *token, float *literal)
{
    if ( !text
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp", 3633, 0, "%s", "text") )
    {
        __debugbreak();
    }
    if ( !token
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp", 3634, 0, "%s", "token") )
    {
        __debugbreak();
    }
    if ( !literal
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    3635,
                    0,
                    "%s",
                    "literal") )
    {
        __debugbreak();
    }
    *literal = 0.0f;
    literal[1] = 0.0f;
    literal[2] = 0.0f;
    literal[3] = 1.0f;
    if ( !strcmp(token, "float1") )
    {
        Material_ParseVector(text, 1, literal);
    }
    else if ( !strcmp(token, "float2") )
    {
        Material_ParseVector(text, 2, literal);
    }
    else if ( !strcmp(token, "float3") )
    {
        Material_ParseVector(text, 3, literal);
    }
    else
    {
        if ( strcmp(token, "float4") )
            return 0;
        Material_ParseVector(text, 4, literal);
    }
    return 1;
}

bool __cdecl Material_ParseVector(const char **text, int elemCount, float *vector)
{
    int elemIndex; // [esp+0h] [ebp-4h]

    if ( !Material_MatchToken(text, "(") )
        return 0;
    elemIndex = 0;
    while ( 1 )
    {
        vector[elemIndex++] = Com_ParseFloat(text);
        if ( elemIndex == elemCount )
            break;
        if ( !Material_MatchToken(text, ",") )
            return 0;
    }
    return Material_MatchToken(text, ")");
}

bool __cdecl Material_ParseCodeConstantSource_r(
                MaterialShaderType shaderType,
                const char **text,
                int offset,
                const CodeConstantSource *sourceTable,
                ShaderArgumentSource *argSource)
{
    int sourceIndex; // [esp+14h] [ebp-Ch]
    int additionalOffset; // [esp+18h] [ebp-8h] BYREF
    const char *token; // [esp+1Ch] [ebp-4h]

    if ( !text
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp", 3662, 0, "%s", "text") )
    {
        __debugbreak();
    }
    if ( !sourceTable
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    3663,
                    0,
                    "%s",
                    "sourceTable") )
    {
        __debugbreak();
    }
    if ( !argSource
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    3664,
                    0,
                    "%s",
                    "argSource") )
    {
        __debugbreak();
    }
    if ( !Material_MatchToken(text, ".") )
        return 0;
    token = (const char *)Com_Parse(text);
    for ( sourceIndex = 0; ; ++sourceIndex )
    {
        if ( !sourceTable[sourceIndex].name )
        {
            Com_ScriptError("unknown constant source '%s'\n", token);
            return 0;
        }
        if ( !strcmp(token, sourceTable[sourceIndex].name) )
            break;
    }
    if ( sourceTable[sourceIndex].arrayCount )
    {
        if ( !sourceTable[sourceIndex].subtable
            && (sourceTable[sourceIndex].source >= 0xC5u || sourceTable[sourceIndex].arrayStride != 1)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                        3677,
                        0,
                        "%s\n\t(sourceTable[sourceIndex].name) = %s",
                        "(sourceTable[sourceIndex].subtable || (sourceTable[sourceIndex].source < CONST_SRC_FIRST_CODE_MATRIX && sour"
                        "ceTable[sourceIndex].arrayStride == 1))",
                        sourceTable[sourceIndex].name) )
        {
            __debugbreak();
        }
        if ( sourceTable[sourceIndex].subtable )
        {
            if ( !Material_ParseArrayOffset(
                            text,
                            sourceTable[sourceIndex].arrayCount,
                            sourceTable[sourceIndex].arrayStride,
                            &additionalOffset) )
                return 0;
            offset += additionalOffset;
        }
        else
        {
            if ( sourceTable[sourceIndex].arrayStride != 1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                            3680,
                            1,
                            "%s\n\t(sourceIndex) = %i",
                            "(sourceTable[sourceIndex].arrayStride == 1)",
                            sourceIndex) )
            {
                __debugbreak();
            }
            if ( !Material_ParseIndexRange(text, sourceTable[sourceIndex].arrayCount, &argSource->indexRange) )
                return 0;
        }
    }
    if ( sourceTable[sourceIndex].subtable )
        return Material_ParseCodeConstantSource_r(shaderType, text, offset, sourceTable[sourceIndex].subtable, argSource);
    argSource->type = 2 * (shaderType != MTL_VERTEX_SHADER) + 3;
    argSource->u.codeIndex = offset + sourceTable[sourceIndex].source;
    if ( argSource->type != 3
        && !s_codeConstUpdateFreq[argSource->u.codeIndex]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    3697,
                    0,
                    "%s\n\t(argSource->u.codeIndex) = %i",
                    "((argSource->type == MTL_ARG_CODE_VERTEX_CONST) || s_codeConstUpdateFreq[argSource->u.codeIndex] != MTL_UPDATE_PER_PRIM)",
                    argSource->u.codeIndex) )
    {
        __debugbreak();
    }
    if ( !sourceTable[sourceIndex].arrayCount )
    {
        if ( argSource->u.codeIndex >= 0xC5u )
        {
            if ( !Material_ParseIndexRange(text, 4u, &argSource->indexRange) )
                return 0;
        }
        else
        {
            argSource->indexRange.first = 0;
            argSource->indexRange.count = 1;
            argSource->indexRange.isImplicit = 0;
        }
    }
    return 1;
}

bool __cdecl Material_DefaultArgumentSource(
                MaterialShaderType shaderType,
                const char *constantName,
                ShaderParamType paramType,
                const ShaderIndexRange *indexRange,
                ShaderArgumentSource *argSource)
{
    if ( !constantName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    3869,
                    0,
                    "%s",
                    "constantName") )
    {
        __debugbreak();
    }
    if ( !argSource
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    3870,
                    0,
                    "%s",
                    "argSource") )
    {
        __debugbreak();
    }
    if ( paramType )
        return paramType > SHADER_PARAM_FLOAT4
                && paramType <= SHADER_PARAM_SAMPLER_1D
                && Material_DefaultSamplerSource(constantName, indexRange, argSource);
    else
        return Material_DefaultConstantSource(shaderType, constantName, indexRange, argSource)
                || Material_UnknownShaderworksConstantSource(shaderType, constantName, indexRange, argSource);
}

bool __cdecl Material_DefaultSamplerSource(
                const char *constantName,
                const ShaderIndexRange *indexRange,
                ShaderArgumentSource *argSource)
{
    return Material_DefaultSamplerSourceFromTable(constantName, indexRange, s_defaultCodeSamplers, argSource) != 0;
}

char __cdecl Material_DefaultSamplerSourceFromTable(
                const char *constantName,
                const ShaderIndexRange *indexRange,
                const CodeSamplerSource *sourceTable,
                ShaderArgumentSource *argSource)
{
    int sourceIndex; // [esp+14h] [ebp-4h]

    if ( !constantName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    3583,
                    0,
                    "%s",
                    "constantName") )
    {
        __debugbreak();
    }
    if ( !sourceTable
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    3584,
                    0,
                    "%s",
                    "sourceTable") )
    {
        __debugbreak();
    }
    if ( !indexRange
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    3585,
                    0,
                    "%s",
                    "indexRange") )
    {
        __debugbreak();
    }
    if ( !argSource
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    3586,
                    0,
                    "%s",
                    "argSource") )
    {
        __debugbreak();
    }
    for ( sourceIndex = 0; sourceTable[sourceIndex].name; ++sourceIndex )
    {
        if ( !sourceTable[sourceIndex].subtable
            && !strcmp(constantName, sourceTable[sourceIndex].name)
            && Material_DefaultIndexRange(indexRange, sourceTable[sourceIndex].arrayCount, &argSource->indexRange) )
        {
            argSource->type = 4;
            argSource->u.codeIndex = sourceTable[sourceIndex].source;
            return 1;
        }
    }
    return 0;
}

char __cdecl Material_DefaultIndexRange(
                const ShaderIndexRange *indexRangeRef,
                unsigned int arrayCount,
                ShaderIndexRange *indexRangeSet)
{
    if ( arrayCount )
    {
        if ( indexRangeRef->count + indexRangeRef->first > arrayCount )
            return 0;
    }
    else if ( indexRangeRef->first || indexRangeRef->count != 1 )
    {
        return 0;
    }
    indexRangeSet->first = indexRangeRef->first;
    indexRangeSet->count = indexRangeRef->count;
    indexRangeSet->isImplicit = 0;
    return 1;
}

bool __cdecl Material_DefaultConstantSource(
                MaterialShaderType shaderType,
                const char *constantName,
                const ShaderIndexRange *indexRange,
                ShaderArgumentSource *argSource)
{
    return Material_DefaultConstantSourceFromTable(shaderType, constantName, indexRange, s_codeConsts, argSource)
            || Material_DefaultConstantSourceFromTable(shaderType, constantName, indexRange, s_defaultCodeConsts, argSource) != 0;
}

char __cdecl Material_DefaultConstantSourceFromTable(
                MaterialShaderType shaderType,
                const char *constantName,
                const ShaderIndexRange *indexRange,
                const CodeConstantSource *sourceTable,
                ShaderArgumentSource *argSource)
{
    char v5; // al
    unsigned int sourceIndex; // [esp+1Ch] [ebp-8h]

    for ( sourceIndex = 0; ; ++sourceIndex )
    {
        if ( !sourceTable[sourceIndex].name )
            return 0;
        if ( !sourceTable[sourceIndex].subtable && !strcmp(constantName, sourceTable[sourceIndex].name) )
        {
            if ( sourceTable[sourceIndex].source < 0xC5u )
            {
                v5 = sourceTable[sourceIndex].arrayCount > 1
                     ? Material_DefaultIndexRange(indexRange, sourceTable[sourceIndex].arrayCount, &argSource->indexRange)
                     : Material_DefaultIndexRange(indexRange, 1u, &argSource->indexRange);
            }
            else
            {
                if ( sourceTable[sourceIndex].arrayCount
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                                3771,
                                0,
                                "%s\n\t(constantName) = %s",
                                "(sourceTable[sourceIndex].arrayCount == 0)",
                                constantName) )
                {
                    __debugbreak();
                }
                v5 = Material_DefaultIndexRange(indexRange, 4u, &argSource->indexRange);
            }
            if ( v5 )
                break;
        }
    }
    argSource->type = 2 * (shaderType != MTL_VERTEX_SHADER) + 3;
    argSource->u.codeIndex = sourceTable[sourceIndex].source;
    if ( argSource->type != 3
        && !s_codeConstUpdateFreq[argSource->u.codeIndex]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    3783,
                    0,
                    "%s\n\t(argSource->u.codeIndex) = %i",
                    "((argSource->type == MTL_ARG_CODE_VERTEX_CONST) || s_codeConstUpdateFreq[argSource->u.codeIndex] != MTL_UPDATE_PER_PRIM)",
                    argSource->u.codeIndex) )
    {
        __debugbreak();
    }
    return 1;
}

bool __cdecl Material_UnknownShaderworksConstantSource(
                MaterialShaderType shaderType,
                const char *constantName,
                const ShaderIndexRange *indexRange,
                ShaderArgumentSource *argSource)
{
    float literalVal[4]; // [esp+0h] [ebp-10h] BYREF

    if ( I_strnicmp(constantName, "__", 2) )
        return 0;
    memset(literalVal, 0, sizeof(literalVal));
    argSource->type = shaderType != MTL_VERTEX_SHADER ? 7 : 1;
    argSource->u.literalConst = Material_RegisterLiteral(literalVal);
    argSource->indexRange.first = indexRange->first;
    argSource->indexRange.count = indexRange->count;
    argSource->indexRange.isImplicit = 1;
    return argSource->u.literalConst != 0;
}

char __cdecl Material_SetShaderArguments(
                unsigned int usedCount,
                MaterialShaderArgument *localArgs,
                unsigned int argLimit,
                unsigned int *argCount,
                MaterialShaderArgument *args)
{
    unsigned int usedCounta; // [esp+8h] [ebp+8h]

    if ( !args
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp", 4038, 0, "%s", "args") )
    {
        __debugbreak();
    }
    if ( !argCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    4039,
                    0,
                    "%s",
                    "argCount") )
    {
        __debugbreak();
    }
    if ( !usedCount )
        return 1;
    if ( *argCount + usedCount <= argLimit )
    {
        qsort(
            localArgs,
            usedCount,
            8u,
            (int (__cdecl *)(const void *, const void *))Material_CompareShaderArgumentsForCombining);
        usedCounta = Material_CombineShaderArguments(usedCount, localArgs);
        memcpy((unsigned __int8 *)&args[*argCount], (unsigned __int8 *)localArgs, 8 * usedCounta);
        *argCount += usedCounta;
        return 1;
    }
    else
    {
        Com_ScriptError("more than %i total shader arguments\n", argLimit);
        return 0;
    }
}

int __cdecl Material_CompareShaderArgumentsForCombining(unsigned __int16 *e0, unsigned __int16 *e1)
{
    int v3; // [esp+0h] [ebp-18h]
    int v4; // [esp+4h] [ebp-14h]

    v4 = *e0 == 4 || *e0 == 2;
    v3 = *e1 == 4 || *e1 == 2;
    if ( v4 == v3 )
        return e0[1] - e1[1];
    else
        return v4 - v3;
}

unsigned int __cdecl Material_CombineShaderArguments(unsigned int usedCount, MaterialShaderArgument *localArgs)
{
    MaterialArgumentDef v2; // ecx
    unsigned int srcIndex; // [esp+4h] [ebp-8h]
    unsigned int dstIndex; // [esp+8h] [ebp-4h]

    dstIndex = 0;
    for ( srcIndex = 1; srcIndex < usedCount; ++srcIndex )
    {
        if ( !Material_AttemptCombineShaderArguments(&localArgs[dstIndex], &localArgs[srcIndex]) )
        {
            ++dstIndex;
            LODWORD(v2.literalConst) = localArgs[srcIndex].u.nameHash;
            *(unsigned int *)&localArgs[dstIndex].type = *(unsigned int *)&localArgs[srcIndex].type;
            localArgs[dstIndex].u = v2;
        }
    }
    return dstIndex + 1;
}

char __cdecl Material_AttemptCombineShaderArguments(MaterialShaderArgument *arg0, const MaterialShaderArgument *arg1)
{
    if ( arg0->type != arg1->type )
        return 0;
    if ( arg0->type != 3 && arg0->type != 5 )
        return 0;
    if ( arg0->u.codeConst.rowCount + arg0->dest != arg1->dest )
        return 0;
    if ( arg0->u.codeConst.index < 0xC5u )
        return 0;
    if ( arg0->u.codeConst.index != arg1->u.codeConst.index )
        return 0;
    if ( arg0->u.codeConst.rowCount + arg0->u.codeConst.firstRow != arg1->u.codeConst.firstRow )
        return 0;
    if ( (arg1->u.codeConst.rowCount + arg0->u.codeConst.rowCount + arg0->u.codeConst.firstRow < 2
         || arg1->u.codeConst.rowCount + arg0->u.codeConst.rowCount + arg0->u.codeConst.firstRow > 4)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    4011,
                    0,
                    "arg0->u.codeConst.firstRow + arg0->u.codeConst.rowCount + arg1->u.codeConst.rowCount not in [2, 4]\n"
                    "\t%i not in [%i, %i]",
                    arg1->u.codeConst.rowCount + arg0->u.codeConst.rowCount + arg0->u.codeConst.firstRow,
                    2,
                    4) )
    {
        __debugbreak();
    }
    arg0->u.codeConst.rowCount += arg1->u.codeConst.rowCount;
    return 1;
}

bool __cdecl Material_AddShaderArgument(
                const char *shaderName,
                MaterialShaderType shaderType,
                ShaderArgumentSource *argSource,
                const ShaderArgumentDest *argDest,
                ShaderUniformDef *paramTable,
                unsigned int paramCount,
                unsigned int *usedCount,
                MaterialShaderArgument *argTable,
                char (*registerUsage)[64])
{
    bool result; // al
    const char *v10; // eax
    ShaderUniformDef *dest; // [esp+4h] [ebp-8h]
    ShaderUniformDef *desta; // [esp+4h] [ebp-8h]
    ShaderUniformDef *destb; // [esp+4h] [ebp-8h]
    ShaderUniformDef *destc; // [esp+4h] [ebp-8h]
    unsigned int indexOffset; // [esp+8h] [ebp-4h]
    unsigned int indexOffseta; // [esp+8h] [ebp-4h]

    if ( argSource->indexRange.isImplicit )
    {
        if ( argSource->indexRange.first
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                        4191,
                        0,
                        "%s",
                        "argSource->indexRange.first == 0") )
        {
            __debugbreak();
        }
        if ( argDest->indexRange.count > argSource->indexRange.count )
        {
            Com_ScriptError(
                "The destination needs %i entries, but the source can only provide %i",
                argDest->indexRange.count,
                argSource->indexRange.count);
            return 0;
        }
        argSource->indexRange.count = argDest->indexRange.count;
    }
    else if ( argDest->indexRange.count != argSource->indexRange.count )
    {
        Com_ScriptError(
            "The destination needs %i entries, but the source provides %i",
            argDest->indexRange.count,
            argSource->indexRange.count);
        return 0;
    }
    switch ( argSource->type )
    {
        case 0u:
        case 2u:
        case 6u:
            if ( argDest->indexRange.count == 1 )
            {
                if ( argSource->indexRange.first
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                                4257,
                                0,
                                "%s",
                                "argSource->indexRange.first == 0") )
                {
                    __debugbreak();
                }
                if ( argSource->indexRange.count != 1
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                                4258,
                                0,
                                "%s",
                                "argSource->indexRange.count == 1") )
                {
                    __debugbreak();
                }
                destc = Material_GetShaderArgumentDest(argDest->paramName, argDest->indexRange.first, paramTable, paramCount);
                if ( destc )
                {
                    if ( Material_AddShaderArgumentFromMaterial(
                                 shaderName,
                                 argDest->paramName,
                                 argSource->type,
                                 (char *)argSource->u.literalConst,
                                 destc,
                                 &argTable[*usedCount],
                                 registerUsage) )
                    {
                        ++*usedCount;
                        result = 1;
                    }
                    else
                    {
                        result = 0;
                    }
                }
                else
                {
                    result = 0;
                }
            }
            else
            {
                Com_ScriptError("Must assign material values one at a time");
                result = 0;
            }
            break;
        case 1u:
        case 7u:
            if ( argDest->indexRange.count == 1 )
            {
                dest = Material_GetShaderArgumentDest(argDest->paramName, argDest->indexRange.first, paramTable, paramCount);
                if ( dest )
                {
                    if ( Material_AddShaderArgumentFromLiteral(
                                 shaderName,
                                 argDest->paramName,
                                 argSource->type,
                                 argSource->u.literalConst,
                                 dest,
                                 &argTable[*usedCount],
                                 registerUsage) )
                    {
                        ++*usedCount;
                        result = 1;
                    }
                    else
                    {
                        result = 0;
                    }
                }
                else
                {
                    result = 0;
                }
            }
            else
            {
                Com_ScriptError("Must assign literals to a constant one row at a time");
                result = 0;
            }
            break;
        case 3u:
        case 5u:
            for ( indexOffset = 0; indexOffset < argDest->indexRange.count; ++indexOffset )
            {
                desta = Material_GetShaderArgumentDest(
                                    argDest->paramName,
                                    indexOffset + argDest->indexRange.first,
                                    paramTable,
                                    paramCount);
                if ( !desta )
                    return 0;
                if ( !Material_AddShaderArgumentFromCodeConst(
                                shaderName,
                                argDest->paramName,
                                argSource->type,
                                argSource->u.codeIndex,
                                indexOffset + LOWORD(argSource->indexRange.first),
                                desta,
                                &argTable[*usedCount],
                                registerUsage) )
                    return 0;
                ++*usedCount;
            }
            result = 1;
            break;
        case 4u:
            for ( indexOffseta = 0; indexOffseta < argDest->indexRange.count; ++indexOffseta )
            {
                destb = Material_GetShaderArgumentDest(
                                    argDest->paramName,
                                    indexOffseta + argDest->indexRange.first,
                                    paramTable,
                                    paramCount);
                if ( !destb )
                    return 0;
                Material_AddShaderArgumentFromCodeSampler(
                    argSource->type,
                    indexOffseta + argSource->indexRange.first + argSource->u.codeIndex,
                    destb,
                    &argTable[*usedCount]);
                ++*usedCount;
            }
            result = 1;
            break;
        default:
            v10 = va("unhandled case %i", argSource->type);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp", 4268, 1, v10) )
                __debugbreak();
            result = 0;
            break;
    }
    return result;
}

ShaderUniformDef *__cdecl Material_GetShaderArgumentDest(
                const char *paramName,
                unsigned int paramIndex,
                ShaderUniformDef *paramTable,
                unsigned int paramCount)
{
    unsigned int tableIndex; // [esp+14h] [ebp-4h]

    for ( tableIndex = 0; tableIndex < paramCount; ++tableIndex )
    {
        if ( paramTable[tableIndex].index == paramIndex && !strcmp(paramTable[tableIndex].name, paramName) )
        {
            if ( paramTable[tableIndex].isAssigned )
            {
                Com_ScriptError("parameter %s index %i already assigned\n", paramName, paramIndex);
                return 0;
            }
            else
            {
                paramTable[tableIndex].isAssigned = 1;
                return &paramTable[tableIndex];
            }
        }
    }
    if ( !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    4075,
                    1,
                    "unfound name should be caught earlier") )
        __debugbreak();
    return 0;
}

char __cdecl Material_AddShaderArgumentFromLiteral(
                const char *shaderName,
                const char *paramName,
                unsigned __int16 type,
                const float *literal,
                ShaderUniformDef *dest,
                MaterialShaderArgument *arg,
                char (*registerUsage)[64])
{
    arg->type = type;
    arg->dest = dest->resourceDest;
    if ( type == 7
        && arg->dest >= 0x100u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    4119,
                    0,
                    "arg->dest doesn't index R_MAX_PIXEL_SHADER_CONSTS\n\t%i not in [0, %i)",
                    arg->dest,
                    256) )
    {
        __debugbreak();
    }
    arg->u.codeSampler = (unsigned int)literal;
    return MaterialAddShaderArgument(shaderName, paramName, arg, registerUsage);
}

char __cdecl MaterialAddShaderArgument(
                const char *shaderName,
                const char *paramName,
                MaterialShaderArgument *arg,
                char (*registerUsage)[64])
{
    if ( arg->type > 1u && arg->type != 3 )
        return 1;
    if ( arg->dest < 0x100u )
    {
        if ( (*registerUsage)[64 * arg->dest] )
        {
            Com_ScriptError(
                "Vertex register collision at index %d in '%s' between '%s' and '%s'\n",
                arg->dest,
                shaderName,
                &(*registerUsage)[64 * arg->dest],
                paramName);
            return 0;
        }
        else
        {
            I_strncpyz(&(*registerUsage)[64 * arg->dest], paramName, 64);
            return 1;
        }
    }
    else
    {
        Com_ScriptError("Invalid vertex register index %d in '%s' for '%s'\n", arg->dest, shaderName, paramName);
        return 0;
    }
}

char __cdecl Material_AddShaderArgumentFromCodeConst(
                const char *shaderName,
                const char *paramName,
                unsigned __int16 type,
                unsigned int codeIndex,
                __int16 offset,
                ShaderUniformDef *dest,
                MaterialShaderArgument *arg,
                char (*registerUsage)[64])
{
    arg->type = type;
    arg->dest = dest->resourceDest;
    if ( type == 5
        && arg->dest >= 0x100u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    4136,
                    0,
                    "arg->dest doesn't index R_MAX_PIXEL_SHADER_CONSTS\n\t%i not in [0, %i)",
                    arg->dest,
                    256) )
    {
        __debugbreak();
    }
    if ( codeIndex < 0xC5 )
    {
        arg->u.codeConst.index = offset + codeIndex;
        arg->u.codeConst.firstRow = 0;
    }
    else
    {
        if ( dest->isTransposed )
            arg->u.codeConst.index = ((codeIndex - 197) ^ 2) + 197;
        else
            arg->u.codeConst.index = codeIndex;
        arg->u.codeConst.firstRow = offset;
    }
    arg->u.codeConst.rowCount = 1;
    return MaterialAddShaderArgument(shaderName, paramName, arg, registerUsage);
}

void __cdecl Material_AddShaderArgumentFromCodeSampler(
                unsigned __int16 type,
                unsigned int codeSampler,
                ShaderUniformDef *dest,
                MaterialShaderArgument *arg)
{
    arg->type = type;
    arg->dest = dest->resourceDest;
    arg->u.codeSampler = codeSampler;
}

char __cdecl Material_AddShaderArgumentFromMaterial(
                const char *shaderName,
                const char *paramName,
                unsigned __int16 type,
                char *name,
                ShaderUniformDef *dest,
                MaterialShaderArgument *arg,
                char (*registerUsage)[64])
{
    Material_RegisterString(name);
    arg->type = type;
    arg->dest = dest->resourceDest;
    if ( type == 6
        && arg->dest >= 0x100u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    4175,
                    0,
                    "arg->dest doesn't index R_MAX_PIXEL_SHADER_CONSTS\n\t%i not in [0, %i)",
                    arg->dest,
                    256) )
    {
        __debugbreak();
    }
    arg->u.codeSampler = R_HashString(name);
    return MaterialAddShaderArgument(shaderName, paramName, arg, registerUsage);
}

char __cdecl CodeConstIsOneOf(unsigned __int16 constCodeIndex, const unsigned __int16 *consts, int numConsts)
{
    int constIdx; // [esp+0h] [ebp-4h]

    for ( constIdx = 0; constIdx < numConsts; ++constIdx )
    {
        if ( consts[constIdx] == constCodeIndex )
            return 1;
    }
    return 0;
}

void __cdecl Material_SetVaryingParameterDef(const _D3DXSEMANTIC *semantic, ShaderVaryingDef *paramDef)
{
    paramDef->streamDest = Material_GetStreamDestForSemantic(semantic);
    paramDef->resourceDest = paramDef->streamDest;
    paramDef->name = Material_NameForStreamDest(paramDef->streamDest);
    paramDef->isAssigned = 0;
}

unsigned __int8 __cdecl Material_GetStreamDestForSemantic(const _D3DXSEMANTIC *semantic)
{
    unsigned int v1; // eax

    switch ( semantic->Usage )
    {
        case 0u:
            if ( semantic->UsageIndex )
                goto LABEL_19;
            LOBYTE(v1) = 0;
            break;
        case 1u:
            if ( semantic->UsageIndex )
                goto LABEL_19;
            LOBYTE(v1) = 19;
            break;
        case 3u:
            if ( semantic->UsageIndex )
                goto LABEL_19;
            LOBYTE(v1) = 1;
            break;
        case 5u:
            if ( semantic->UsageIndex >= 0xE )
                goto LABEL_19;
            v1 = semantic->UsageIndex + 5;
            break;
        case 0xAu:
            if ( semantic->UsageIndex >= 2 )
                goto LABEL_19;
            v1 = semantic->UsageIndex + 2;
            break;
        case 0xCu:
            if ( semantic->UsageIndex )
                goto LABEL_19;
            LOBYTE(v1) = 4;
            break;
        default:
LABEL_19:
            Com_Error(ERR_DROP, "Unknown shader input/output usage %i:%i\n", semantic->Usage, semantic->UsageIndex);
            LOBYTE(v1) = 0;
            break;
    }
    return v1;
}

char __cdecl Material_LoadDeclTypes(const char **text, MaterialPass *pass)
{
    if ( !pass->vertexShader
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    8060,
                    0,
                    "%s",
                    "pass->vertexShader") )
    {
        __debugbreak();
    }
    if ( !strcmp(Com_Parse(text)->token, "declTypes") )
    {
        while ( strcmp("}", Com_Parse(text)->token) )
            ;
    }
    else
    {
        Com_UngetToken();
    }
    return 1;
}

char __cdecl Material_LoadPassPixelShader(
                const char **text,
                unsigned __int16 *techFlags,
                ShaderParameterSet *paramSet,
                MaterialPass *pass,
                unsigned int argLimit,
                unsigned int *argCount,
                MaterialShaderArgument *args)
{
    parseInfo_t *shaderName; // [esp+8h] [ebp-8h]
    MaterialPixelShader *mtlShader; // [esp+Ch] [ebp-4h]

    memset(paramSet, 0, sizeof(ShaderParameterSet));
    if ( !Material_MatchToken(text, "pixelShader") )
        return 0;
    Material_ParseShaderVersion(text);
    shaderName = Com_Parse(text);
    mtlShader = Material_RegisterPixelShader(shaderName->token);
    if ( !mtlShader )
        return 0;
    pass->pixelShader = mtlShader;
    return Material_SetPassShaderArguments_DX(
                     (char **)text,
                     (char *)mtlShader->name,
                     MTL_PIXEL_SHADER,
                     (const DWORD *)&mtlShader[1],
                     techFlags,
                     paramSet,
                     argLimit,
                     argCount,
                     args);
}

MaterialPixelShader *__cdecl Material_RegisterPixelShader(char *shaderName)
{
    unsigned int hashIndex; // [esp+14h] [ebp-Ch] BYREF
    MaterialPixelShader *mtlShader; // [esp+1Ch] [ebp-4h]

    if ( Material_GetPixelShaderHashIndex(shaderName, &hashIndex) )
        return mtlLoadGlob.pixelShaderHashTable[hashIndex];
    ProfLoad_Begin("Load pixel shader");
    mtlShader = Material_LoadPixelShader(shaderName);
    ProfLoad_End();
    if ( mtlShader && !strcmp(mtlShader->name, shaderName) )
    {
        mtlLoadGlob.pixelShaderHashTable[hashIndex] = mtlShader;
        if ( ++mtlLoadGlob.pixelShaderCount == 20480 )
            Com_Error(ERR_DROP, "More than %i unique pixel shaders", 20479);
    }
    return mtlShader;
}

MaterialPixelShader *__cdecl Material_LoadPixelShader(char *shaderName)
{
    unsigned __int8 *v2; // eax
    const char *v3; // eax
    ID3DXBuffer *v4; // [esp-8h] [ebp-50h]
    unsigned int programSize; // [esp+10h] [ebp-38h]
    int hr; // [esp+18h] [ebp-30h]
    char target[16]; // [esp+1Ch] [ebp-2Ch] BYREF
    DWORD *program; // [esp+30h] [ebp-18h]
    unsigned int nameSize; // [esp+34h] [ebp-14h]
    ID3DXBuffer *shader; // [esp+3Ch] [ebp-Ch]
    unsigned int totalSize; // [esp+40h] [ebp-8h]
    MaterialPixelShader *mtlShader; // [esp+44h] [ebp-4h]

    Material_GetShaderTargetString(target, 0x10u, "ps");
    shader = Material_CompileShader(shaderName, MTL_PIXEL_SHADER, (char*)"ps_main", target);
    if ( !shader )
        return 0;
    programSize = shader->GetBufferSize();
    if ( !programSize
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    5274,
                    0,
                    "%s\n\t(programSize) = %i",
                    "(programSize > 0)",
                    0) )
    {
        __debugbreak();
    }
    nameSize = strlen(shaderName) + 1;
    totalSize = programSize + nameSize + 16;
    mtlShader = (MaterialPixelShader *)Material_Alloc(totalSize);
    program = (DWORD*)&mtlShader[1];
    mtlShader->name = (char *)&mtlShader[1] + programSize;
    memcpy((unsigned __int8 *)mtlShader->name, (unsigned __int8 *)shaderName, nameSize);
    v4 = shader;
    //v2 = (unsigned __int8 *)((int (__thiscall *)(ID3DXBuffer *))shader->GetBufferPointer)(shader);
    //memcpy((unsigned __int8 *)program, v2, (unsigned int)v4);
    memcpy(program, shader->GetBufferPointer(), programSize);

    hr = dx.device->CreatePixelShader(program, &mtlShader->prog.ps);
    //hr = ((int (__stdcall *)(IDirect3DDevice9 *, unsigned int *, MaterialPixelShaderProgram *, unsigned int))dx.device->CreatePixelShader)(
    //             dx.device,
    //             program,
    //             &mtlShader->prog,
    //             programSize);
    if ( hr >= 0 )
    {
        mtlShader->prog.loadDef.programSize = programSize >> 2;
        if ( 4 * mtlShader->prog.loadDef.programSize != programSize
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                        5340,
                        1,
                        "mtlShader->prog.loadDef.programSize * sizeof( mtlShader->prog.loadDef.program[0] ) == programSize\n"
                        "\t%i, %i",
                        4 * mtlShader->prog.loadDef.programSize,
                        programSize) )
        {
            __debugbreak();
        }
        mtlShader->prog.loadDef.program = program;
        shader->Release();
        return mtlShader;
    }
    else
    {
        v3 = R_ErrorDescription(hr);
        Com_ScriptError("pixel shader creation failed for %s: %s\n", shaderName, v3);
        return 0;
    }
}

char __cdecl Material_GetPixelShaderHashIndex(const char *shaderName, unsigned int *foundHashIndex)
{
    unsigned int hashIndex; // [esp+14h] [ebp-8h]

    if ( !shaderName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    7549,
                    0,
                    "%s",
                    "shaderName") )
    {
        __debugbreak();
    }
    if ( !foundHashIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    7550,
                    0,
                    "%s",
                    "foundHashIndex") )
    {
        __debugbreak();
    }
    for ( hashIndex = R_HashAssetName(shaderName) % 0x5000;
                mtlLoadGlob.pixelShaderHashTable[hashIndex];
                hashIndex = (hashIndex + 1) % 0x5000 )
    {
        if ( !strcmp(mtlLoadGlob.pixelShaderHashTable[hashIndex]->name, shaderName) )
        {
            *foundHashIndex = hashIndex;
            return 1;
        }
    }
    *foundHashIndex = hashIndex;
    return 0;
}

bool __cdecl Material_ValidateShaderLinkage(
                const ShaderVaryingDef *vertexOutputs,
                unsigned int vertexOutputCount,
                const ShaderVaryingDef *pixelInputs,
                unsigned int pixelInputCount)
{
    unsigned int paramIndex; // [esp+0h] [ebp-8h]
    unsigned int paramIndexa; // [esp+0h] [ebp-8h]
    bool isValid; // [esp+7h] [ebp-1h]

    isValid = 1;
    for ( paramIndex = 0; paramIndex < pixelInputCount; ++paramIndex )
    {
        if ( !Material_HasMatchingParameter(pixelInputs[paramIndex].resourceDest, vertexOutputs, vertexOutputCount)
            && !Material_HasMatchingParameter_BuggySdkWorkaround(
                        pixelInputs[paramIndex].resourceDest,
                        vertexOutputs,
                        vertexOutputCount) )
        {
            Com_ScriptError(
                "Pixel shader input '%s' doesn't have a corresponding vertex shader output.\n",
                pixelInputs[paramIndex].name);
            isValid = 0;
        }
    }
    for ( paramIndexa = 0; paramIndexa < vertexOutputCount; ++paramIndexa )
    {
        if ( !Material_HasMatchingParameter(vertexOutputs[paramIndexa].resourceDest, pixelInputs, pixelInputCount) )
            Material_HasMatchingParameter_BuggySdkWorkaround(
                vertexOutputs[paramIndexa].resourceDest,
                pixelInputs,
                pixelInputCount);
    }
    return isValid;
}

char __cdecl Material_HasMatchingParameter(
                unsigned __int8 find,
                const ShaderVaryingDef *paramTable,
                unsigned int paramCount)
{
    unsigned int paramIndex; // [esp+0h] [ebp-4h]

    for ( paramIndex = 0; paramIndex < paramCount; ++paramIndex )
    {
        if ( paramTable[paramIndex].resourceDest == find )
            return 1;
    }
    return 0;
}

char __cdecl Material_HasMatchingParameter_BuggySdkWorkaround(
                unsigned __int8 find,
                const ShaderVaryingDef *paramTable,
                unsigned int paramCount)
{
    if ( find == 2 )
        return Material_HasMatchingParameter(3u, paramTable, paramCount);
    if ( find == 3 )
        return Material_HasMatchingParameter(2u, paramTable, paramCount);
    return 0;
}

int __cdecl Material_CompareShaderArgumentsForRuntime(
                const MaterialShaderArgument *e0,
                const MaterialShaderArgument *e1)
{
    int updateFreq; // [esp+0h] [ebp-10h]
    int updateFreq_4; // [esp+4h] [ebp-Ch]

    updateFreq = Material_GetArgUpdateFrequency(e0);
    updateFreq_4 = Material_GetArgUpdateFrequency(e1);
    if ( updateFreq != updateFreq_4 )
        return updateFreq - updateFreq_4;
    if ( e0->type != e1->type )
        return e0->type - e1->type;
    if ( !e0->type || e0->type == 6 || e0->type == 2 )
        return e0->u.codeSampler < e1->u.codeSampler ? -1 : 1;
    return e0->dest - e1->dest;
}

int __cdecl Material_GetArgUpdateFrequency(const MaterialShaderArgument *arg)
{
    int type; // [esp+0h] [ebp-8h]
    MaterialUpdateFrequency updateFreq; // [esp+4h] [ebp-4h]
    MaterialUpdateFrequency updateFreqa; // [esp+4h] [ebp-4h]

    if ( !arg
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp", 8265, 0, "%s", "arg") )
    {
        __debugbreak();
    }
    type = arg->type;
    switch ( type )
    {
        case 3:
            return s_codeConstUpdateFreq[arg->u.codeConst.index];
        case 4:
            updateFreqa = s_codeSamplerUpdateFreq[arg->u.codeSampler];
            if ( updateFreqa != MTL_UPDATE_PER_OBJECT
                && updateFreqa != MTL_UPDATE_RARELY
                && updateFreqa != MTL_UPDATE_CUSTOM
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                            8279,
                            0,
                            "%s\n\t(arg->u.codeSampler) = %i",
                            "((updateFreq == MTL_UPDATE_PER_OBJECT) || (updateFreq == MTL_UPDATE_RARELY) || (updateFreq == MTL_UPDATE_CUSTOM))",
                            arg->u.codeSampler) )
            {
                __debugbreak();
            }
            return updateFreqa;
        case 5:
            updateFreq = s_codeConstUpdateFreq[arg->u.codeConst.index];
            if ( updateFreq != MTL_UPDATE_RARELY
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                            8274,
                            0,
                            "%s\n\t(arg->u.codeConst.index) = %i",
                            "(updateFreq == MTL_UPDATE_RARELY)",
                            arg->u.codeConst.index) )
            {
                __debugbreak();
            }
            return updateFreq;
        default:
            return 2;
    }
}

unsigned __int8 __cdecl Material_CountArgsWithUpdateFrequency(
                MaterialUpdateFrequency updateFreq,
                const MaterialShaderArgument *args,
                unsigned int argCount,
                unsigned int *firstArg)
{
    unsigned int matchCount; // [esp+0h] [ebp-4h]
    const MaterialShaderArgument *argsa; // [esp+10h] [ebp+Ch]
    unsigned int argCounta; // [esp+14h] [ebp+10h]

    argsa = &args[*firstArg];
    argCounta = argCount - *firstArg;
    for ( matchCount = 0;
                matchCount < argCounta && Material_GetArgUpdateFrequency(&argsa[matchCount]) == updateFreq;
                ++matchCount )
    {
        ;
    }
    *firstArg += matchCount;
    return matchCount;
}

char __cdecl Material_IgnoreTechnique(const char *name)
{
    int techniqueIndex; // [esp+18h] [ebp-4h]

    for ( techniqueIndex = 0; !techniqueIndex; techniqueIndex = 1 )
    {
        if ( !strcmp(name, "\"none\"") )
            return 1;
    }
    return 0;
}

unsigned __int8 __cdecl Material_TechniqueTypeForName(const char *name)
{
    const char *techniqueNames[130]; // [esp+14h] [ebp-210h]
    int techniqueIndex; // [esp+220h] [ebp-4h]

    techniqueNames[0] = "\"depth prepass\"";
    techniqueNames[1] = "\"build floatz\"";
    techniqueNames[2] = "\"build shadowmap depth\"";
    techniqueNames[3] = "\"build shadowmap color\"";
    techniqueNames[4] = "\"unlit\"";
    techniqueNames[5] = "\"emissive\"";
    techniqueNames[6] = "\"emissive shadow\"";
    techniqueNames[7] = "\"emissive nvintz\"";
    techniqueNames[8] = "\"emissive shadow nvintz\"";
    techniqueNames[9] = "\"emissive reflected\"";
    techniqueNames[10] = "\"lit\"";
    techniqueNames[11] = "\"lit sun\"";
    techniqueNames[12] = "\"lit sun shadow\"";
    techniqueNames[13] = "\"lit spot\"";
    techniqueNames[14] = "\"lit spot shadow\"";
    techniqueNames[15] = "\"lit omni\"";
    techniqueNames[16] = "\"lit omni shadow\"";
    techniqueNames[17] = "\"lit dlight\"";
    techniqueNames[18] = "\"lit sun dlight\"";
    techniqueNames[19] = "\"lit sun shadow dlight\"";
    techniqueNames[20] = "\"lit spot dlight\"";
    techniqueNames[21] = "\"lit spot shadow dlight\"";
    techniqueNames[22] = "\"lit omni dlight\"";
    techniqueNames[23] = "\"lit omni shadow dlight\"";
    techniqueNames[24] = "\"lit glight\"";
    techniqueNames[25] = "\"lit sun glight\"";
    techniqueNames[26] = "\"lit sun shadow glight\"";
    techniqueNames[27] = "\"lit spot glight\"";
    techniqueNames[28] = "\"lit spot shadow glight\"";
    techniqueNames[29] = "\"lit omni glight\"";
    techniqueNames[30] = "\"lit omni shadow glight\"";
    techniqueNames[31] = "\"lit dlight glight\"";
    techniqueNames[32] = "\"lit sun dlight glight\"";
    techniqueNames[33] = "\"lit sun shadow dlight glight\"";
    techniqueNames[34] = "\"lit spot dlight glight\"";
    techniqueNames[35] = "\"lit spot shadow dlight glight\"";
    techniqueNames[36] = "\"lit omni dlight glight\"";
    techniqueNames[37] = "\"lit omni shadow dlight glight\"";
    techniqueNames[38] = "\"lit alpha\"";
    techniqueNames[39] = "\"lit sun alpha\"";
    techniqueNames[40] = "\"lit sun shadow alpha\"";
    techniqueNames[41] = "\"lit spot alpha\"";
    techniqueNames[42] = "\"lit spot shadow alpha\"";
    techniqueNames[43] = "\"lit omni alpha\"";
    techniqueNames[44] = "\"lit omni shadow alpha\"";
    techniqueNames[45] = "\"lit remap\"";
    techniqueNames[46] = "\"lit sun remap\"";
    techniqueNames[47] = "\"lit sun shadow remap\"";
    techniqueNames[48] = "\"lit spot remap\"";
    techniqueNames[49] = "\"lit spot shadow remap\"";
    techniqueNames[50] = "\"lit omni remap\"";
    techniqueNames[51] = "\"lit omni shadow remap\"";
    techniqueNames[52] = "\"lit fade\"";
    techniqueNames[53] = "\"lit sun fade\"";
    techniqueNames[54] = "\"lit sun shadow fade\"";
    techniqueNames[55] = "\"lit spot fade\"";
    techniqueNames[56] = "\"lit spot shadow fade\"";
    techniqueNames[57] = "\"lit omni fade\"";
    techniqueNames[58] = "\"lit omni shadow fade\"";
    techniqueNames[59] = "\"lit charred\"";
    techniqueNames[60] = "\"lit fade charred\"";
    techniqueNames[61] = "\"lit sun charred\"";
    techniqueNames[62] = "\"lit sun fade charred\"";
    techniqueNames[63] = "\"lit sun shadow charred\"";
    techniqueNames[64] = "\"lit sun shadow fade charred\"";
    techniqueNames[65] = "\"lit spot charred\"";
    techniqueNames[66] = "\"lit spot fade charred\"";
    techniqueNames[67] = "\"lit spot shadow charred\"";
    techniqueNames[68] = "\"lit spot shadow fade charred\"";
    techniqueNames[69] = "\"lit omni charred\"";
    techniqueNames[70] = "\"lit omni fade charred\"";
    techniqueNames[71] = "\"lit omni shadow charred\"";
    techniqueNames[72] = "\"lit omni shadow fade charred\"";
    techniqueNames[73] = "\"lit instanced\"";
    techniqueNames[74] = "\"lit instanced sun\"";
    techniqueNames[75] = "\"lit instanced sun shadow\"";
    techniqueNames[76] = "\"lit instanced spot\"";
    techniqueNames[77] = "\"lit instanced spot shadow\"";
    techniqueNames[78] = "\"lit instanced omni\"";
    techniqueNames[79] = "\"lit instanced omni shadow\"";
    techniqueNames[80] = "\"lit nvintz\"";
    techniqueNames[81] = "\"lit sun nvintz\"";
    techniqueNames[82] = "\"lit sun shadow nvintz\"";
    techniqueNames[83] = "\"lit spot nvintz\"";
    techniqueNames[84] = "\"lit spot shadow nvintz\"";
    techniqueNames[85] = "\"lit omni nvintz\"";
    techniqueNames[86] = "\"lit omni shadow nvintz\"";
    techniqueNames[87] = "\"lit dlight nvintz\"";
    techniqueNames[88] = "\"lit sun dlight nvintz\"";
    techniqueNames[89] = "\"lit sun shadow dlight nvintz\"";
    techniqueNames[90] = "\"lit spot dlight nvintz\"";
    techniqueNames[91] = "\"lit spot shadow dlight nvintz\"";
    techniqueNames[92] = "\"lit omni dlight nvintz\"";
    techniqueNames[93] = "\"lit omni shadow dlight nvintz\"";
    techniqueNames[94] = "\"lit glight nvintz\"";
    techniqueNames[95] = "\"lit sun glight nvintz\"";
    techniqueNames[96] = "\"lit sun shadow glight nvintz\"";
    techniqueNames[97] = "\"lit spot glight nvintz\"";
    techniqueNames[98] = "\"lit spot shadow glight nvintz\"";
    techniqueNames[99] = "\"lit omni glight nvintz\"";
    techniqueNames[100] = "\"lit omni shadow glight nvintz\"";
    techniqueNames[101] = "\"lit dlight glight nvintz\"";
    techniqueNames[102] = "\"lit sun dlight glight nvintz\"";
    techniqueNames[103] = "\"lit sun shadow dlight glight nvintz\"";
    techniqueNames[104] = "\"lit spot dlight glight nvintz\"";
    techniqueNames[105] = "\"lit spot shadow dlight glight nvintz\"";
    techniqueNames[106] = "\"lit omni dlight glight nvintz\"";
    techniqueNames[107] = "\"lit omni shadow dlight glight nvintz\"";
    techniqueNames[108] = "\"lit instanced nvintz\"";
    techniqueNames[109] = "\"lit instanced sun nvintz\"";
    techniqueNames[110] = "\"lit instanced sun shadow nvintz\"";
    techniqueNames[111] = "\"lit instanced spot nvintz\"";
    techniqueNames[112] = "\"lit instanced spot shadow nvintz\"";
    techniqueNames[113] = "\"lit instanced omni nvintz\"";
    techniqueNames[114] = "\"lit instanced omni shadow nvintz\"";
    techniqueNames[115] = "\"light spot\"";
    techniqueNames[116] = "\"light omni\"";
    techniqueNames[117] = "\"light spot shadow\"";
    techniqueNames[118] = "\"light spot charred\"";
    techniqueNames[119] = "\"light omni charred\"";
    techniqueNames[120] = "\"light spot shadow charred\"";
    techniqueNames[121] = "\"fakelight normal\"";
    techniqueNames[122] = "\"fakelight view\"";
    techniqueNames[123] = "\"sunlight preview\"";
    techniqueNames[124] = "\"case texture\"";
    techniqueNames[125] = "\"solid wireframe\"";
    techniqueNames[126] = "\"shaded wireframe\"";
    techniqueNames[127] = "\"debug bumpmap\"";
    techniqueNames[128] = "\"debug bumpmap instanced\"";
    techniqueNames[129] = "\"impact mask\"";
    for ( techniqueIndex = 0; (unsigned int)techniqueIndex < 0x82; ++techniqueIndex )
    {
        if ( !strcmp(name, techniqueNames[techniqueIndex]) )
            return techniqueIndex;
    }
    return -126;
}

bool __cdecl MaterialTexture_IsMatureContent(unsigned __int8 texSemantic, char *texImageName)
{
    char *v3; // eax
    char FILENAME_PATTERN[8]; // [esp+0h] [ebp-Ch] BYREF

    strcpy(FILENAME_PATTERN, "_mature");
    if ( texSemantic == 11 || texSemantic == 1 || !texSemantic )
        return 0;
    v3 = strstr(texImageName, FILENAME_PATTERN);
    return v3 != 0;
}

Material *__cdecl Material_Load(const char *assetName, int imageTrack)
{
    Material *material; // [esp+0h] [ebp-18h]
    MaterialRaw *mtlRaw; // [esp+4h] [ebp-14h]
    int fileSize; // [esp+8h] [ebp-10h]
    int fileHandle; // [esp+Ch] [ebp-Ch] BYREF
    unsigned int materialType; // [esp+10h] [ebp-8h]
    unsigned int prefixLen; // [esp+14h] [ebp-4h]

    if ( !assetName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10524,
                    0,
                    "%s",
                    "assetName") )
    {
        __debugbreak();
    }
    if ( !*assetName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10525,
                    0,
                    "%s",
                    "assetName[0]") )
    {
        __debugbreak();
    }
    if ( *assetName == 42 )
        return Material_LoadLayered(assetName);
    for ( materialType = 1; materialType < 5; ++materialType )
    {
        prefixLen = g_materialTypeInfo[materialType].prefixLen;
        if ( !strncmp(assetName, g_materialTypeInfo[materialType].prefix, prefixLen) )
        {
            assetName += prefixLen;
            break;
        }
    }
    if ( materialType == 5 )
        materialType = 0;
    fileSize = Material_LoadFile(assetName, &fileHandle);
    if ( fileSize >= 0 )
    {
        if ( fileSize )
        {
            mtlRaw = (MaterialRaw *)Hunk_AllocateTempMemory(fileSize, "Material_Load");
            FS_Read((unsigned __int8 *)mtlRaw, fileSize, fileHandle);
            FS_FCloseFile(fileHandle);
            material = Material_LoadRaw(mtlRaw, materialType, imageTrack);
            Hunk_FreeTempMemory((char *)mtlRaw);
            return material;
        }
        else
        {
            FS_FCloseFile(fileHandle);
            Com_PrintError(8, "^1ERROR: material '%s' has zero length\n", assetName);
            return 0;
        }
    }
    else
    {
        if ( *assetName != 36 )
            Com_PrintError(8, "^1ERROR: Couldn't find material '%s'\n", assetName);
        return 0;
    }
}

Material *__cdecl Material_LoadLayered(const char *assetName)
{
    int v1; // eax
    int bspMaterialIndex; // [esp+8h] [ebp-2Ch]
    bool hasError; // [esp+Fh] [ebp-25h]
    int bspVersion; // [esp+10h] [ebp-24h]
    const char *name; // [esp+14h] [ebp-20h]
    unsigned int layerCount; // [esp+18h] [ebp-1Ch]
    bool expectNormal; // [esp+1Fh] [ebp-15h]
    const Material *mtl[4]; // [esp+20h] [ebp-14h] BYREF
    MaterialTechniqueSet *techSet; // [esp+30h] [ebp-4h]

    if ( *assetName != 42
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10256,
                    0,
                    "%s",
                    "assetName[0] == '*'") )
    {
        __debugbreak();
    }
    name = assetName + 1;
    layerCount = 0;
    hasError = 0;
    bspVersion = Com_GetBspVersion();
    while ( 1 )
    {
        bspMaterialIndex = 0;
        while ( isdigit(*name) )
            bspMaterialIndex = 10 * bspMaterialIndex + *name++ - 48;
        if ( layerCount >= 4
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                        10270,
                        0,
                        "%s",
                        "layerCount < MTL_LAYER_LIMIT") )
        {
            __debugbreak();
        }
        mtl[layerCount] = R_GetBspMaterial(bspMaterialIndex);
        if ( bspVersion < 10 )
        {
            if ( *name != 110
                && *name != 120
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                            10284,
                            0,
                            "%s\n\t(assetName) = %s",
                            "(*name == 'n' || *name == 'x')",
                            assetName) )
            {
                __debugbreak();
            }
            expectNormal = *name == 110;
            name += 7;
        }
        else
        {
            if ( *name != 110
                && *name != 95
                && *name
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                            10276,
                            0,
                            "%s\n\t(assetName) = %s",
                            "(*name == 'n' || *name == '_' || *name == '\\0')",
                            assetName) )
            {
                __debugbreak();
            }
            v1 = *name;
            expectNormal = v1 == 110;
            if ( v1 == 110 )
                ++name;
        }
        if ( Material_IsDefault(mtl[layerCount]) )
        {
            hasError = 1;
        }
        else if ( expectNormal != Material_HasNormalMap(mtl[layerCount]) )
        {
            if ( expectNormal )
                Com_PrintError(
                    1,
                    "In layered material, expected material '%s' %s; using default instead.    Recompile the bsp to fix.\n",
                    mtl[layerCount]->info.name,
                    "without a normal map to have one");
            else
                Com_PrintError(
                    1,
                    "In layered material, expected material '%s' %s; using default instead.    Recompile the bsp to fix.\n",
                    mtl[layerCount]->info.name,
                    "with a normal map to not have one");
            hasError = 1;
        }
        ++layerCount;
        if ( !*name )
            break;
        if ( *name != 95
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                        10304,
                        0,
                        "%s",
                        "*name == '_'") )
        {
            __debugbreak();
        }
        ++name;
    }
    if ( hasError )
        return 0;
    techSet = Material_RegisterLayeredTechniqueSet(mtl, layerCount);
    if ( !techSet )
        return 0;
    else
        return Material_CreateLayered(assetName, mtl, layerCount, techSet);
}

Material *__cdecl Material_CreateLayered(
                const char *name,
                const Material **layerMtl,
                unsigned int layerCount,
                MaterialTechniqueSet *techSet)
{
    unsigned __int8 *v4; // edi
    const MaterialTextureDef *v5; // eax
    char v7; // [esp+Bh] [ebp-535h]
    MaterialConstantDef *v8; // [esp+Ch] [ebp-534h]
    MaterialTextureDef *v9; // [esp+10h] [ebp-530h]
    float *literal; // [esp+34h] [ebp-50Ch]
    unsigned __int8 *memory; // [esp+44h] [ebp-4FCh]
    unsigned int oredSurfaceTypeBits; // [esp+48h] [ebp-4F8h]
    unsigned int texIndex; // [esp+4Ch] [ebp-4F4h]
    unsigned int texTableSize; // [esp+50h] [ebp-4F0h]
    unsigned int nameSize; // [esp+58h] [ebp-4E8h]
    unsigned int nameSizea; // [esp+58h] [ebp-4E8h]
    unsigned int andedGameFlags; // [esp+5Ch] [ebp-4E4h]
    unsigned int constTableSize; // [esp+60h] [ebp-4E0h]
    MaterialTextureDef *newTexEntry; // [esp+64h] [ebp-4DCh]
    unsigned int oredGameFlags; // [esp+68h] [ebp-4D8h]
    unsigned int oredLayeredSurfaceTypes; // [esp+6Ch] [ebp-4D4h]
    unsigned int stateBitsTable[130][2]; // [esp+70h] [ebp-4D0h] BYREF
    const MaterialConstantDef *oldConstTable; // [esp+484h] [ebp-BCh]
    unsigned int tintConstNameHash; // [esp+488h] [ebp-B8h]
    MaterialConstantDef *newConstEntry; // [esp+48Ch] [ebp-B4h]
    bool isTintSpecified; // [esp+492h] [ebp-AEh]
    unsigned __int8 constantCount; // [esp+493h] [ebp-ADh]
    const MaterialTextureDef *oldTexTable; // [esp+494h] [ebp-ACh]
    unsigned __int8 stateBitsEntry[130]; // [esp+498h] [ebp-A8h] BYREF
    Material *newMtl; // [esp+528h] [ebp-18h]
    unsigned int layerIndex; // [esp+52Ch] [ebp-14h]
    unsigned int constIndex; // [esp+530h] [ebp-10h]
    unsigned __int8 textureCount; // [esp+537h] [ebp-9h]
    unsigned int stateBitsCount; // [esp+538h] [ebp-8h]
    char layerChar; // [esp+53Fh] [ebp-1h]

    andedGameFlags = -1;
    oredGameFlags = 0;
    textureCount = 0;
    constantCount = 0;
    oredSurfaceTypeBits = 0;
    tintConstNameHash = R_HashString("colorTint");
    for ( layerIndex = 0; layerIndex < layerCount; ++layerIndex )
    {
        andedGameFlags &= layerMtl[layerIndex]->info.gameFlags;
        oredGameFlags |= layerMtl[layerIndex]->info.gameFlags;
        textureCount += layerMtl[layerIndex]->textureCount;
        constantCount += layerMtl[layerIndex]->constantCount;
        oredSurfaceTypeBits |= layerMtl[layerIndex]->info.surfaceTypeBits;
        if ( !Material_HasConstant(layerMtl[layerIndex], tintConstNameHash) )
            ++constantCount;
    }
    oredLayeredSurfaceTypes = 0;
    nameSize = strlen(name) + 1;
    for ( layerIndex = 0; layerIndex < layerCount; ++layerIndex )
    {
        oredLayeredSurfaceTypes |= (layerMtl[layerIndex]->info.layeredSurfaceTypes & 0x3F) << (6 * layerIndex);
        nameSize += strlen(layerMtl[layerIndex]->info.name);
        if ( layerIndex + 1 < layerCount )
            ++nameSize;
    }
    nameSizea = nameSize + 2;
    stateBitsCount = Material_CreateLayeredStateBitsTable(layerMtl, layerCount, techSet, stateBitsEntry, stateBitsTable);
    texTableSize = 16 * textureCount;
    constTableSize = 32 * constantCount;
    memory = Material_Alloc(nameSizea + texTableSize + constTableSize + 192);
    memset(memory, 0, nameSizea + texTableSize + constTableSize + 192);
    newMtl = (Material *)memory;
    if ( texTableSize )
        v9 = (MaterialTextureDef *)(memory + 192);
    else
        v9 = 0;
    newMtl->textureTable = v9;
    if ( constTableSize )
        v8 = (MaterialConstantDef *)&memory[texTableSize + 192];
    else
        v8 = 0;
    newMtl->localConstantTable = v8;
    newMtl->localTechniqueSet = techSet;
    newMtl->info.name = (const char *)&memory[texTableSize + 192 + constTableSize];
    I_strncpyz((char *)newMtl->info.name, name, nameSizea);
    I_strncat((char *)newMtl->info.name, nameSizea, "(");
    for ( layerIndex = 0; layerIndex < layerCount; ++layerIndex )
    {
        I_strncat((char *)newMtl->info.name, nameSizea, layerMtl[layerIndex]->info.name);
        if ( layerIndex + 1 < layerCount )
            I_strncat((char *)newMtl->info.name, nameSizea, ":");
    }
    I_strncat((char *)newMtl->info.name, nameSizea, ")");
    newMtl->info.gameFlags = oredGameFlags & 0xFFFFFFFB | andedGameFlags & 4;
    newMtl->info.sortKey = (*layerMtl)->info.sortKey;
    newMtl->info.surfaceTypeBits = oredSurfaceTypeBits;
    newMtl->info.layeredSurfaceTypes = oredLayeredSurfaceTypes;
    if ( layerCount >= 8
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    9943,
                    0,
                    "%s",
                    "layerCount < ( 1 << LAYERED_SURFACE_TYPES_LAYERCOUNT_BITS )") )
    {
        __debugbreak();
    }
    newMtl->info.layeredSurfaceTypes |= layerCount << 29;
    newMtl->textureCount = textureCount;
    newMtl->constantCount = constantCount;
    v4 = newMtl->stateBitsEntry;
    memcpy(newMtl->stateBitsEntry, stateBitsEntry, 0x80u);
    *((_WORD *)v4 + 64) = *(_WORD *)&stateBitsEntry[128];
    Material_SetStateBits(newMtl, stateBitsTable, stateBitsCount);
    newTexEntry = newMtl->textureTable;
    newConstEntry = newMtl->localConstantTable;
    for ( layerIndex = 0; layerIndex < layerCount; ++layerIndex )
    {
        if ( layerIndex )
            v7 = layerIndex + 48;
        else
            v7 = 0;
        layerChar = v7;
        oldTexTable = layerMtl[layerIndex]->textureTable;
        for ( texIndex = 0; texIndex < layerMtl[layerIndex]->textureCount; ++texIndex )
        {
            v5 = &oldTexTable[texIndex];
            newTexEntry->nameHash = v5->nameHash;
            *(unsigned int *)&newTexEntry->nameStart = *(unsigned int *)&v5->nameStart;
            *(unsigned int *)&newTexEntry->isMatureContent = *(unsigned int *)&v5->isMatureContent;
            newTexEntry->u.image = v5->u.image;
            if ( (newTexEntry->samplerState & 0x18) == 8
                && (newTexEntry->semantic == 2 || newTexEntry->semantic == 5 || newTexEntry->semantic == 8) )
            {
                newTexEntry->samplerState &= 0xE7u;
                newTexEntry->samplerState |= 0x10u;
            }
            if ( layerChar )
            {
                newTexEntry->nameEnd = layerChar;
                newTexEntry->nameHash = layerChar ^ (33 * newTexEntry->nameHash);
            }
            newTexEntry->isMatureContent = MaterialTexture_IsMatureContent(
                                                                             newTexEntry->semantic,
                                                                             (char *)newTexEntry->u.image->name);
            ++newTexEntry;
        }
        oldConstTable = layerMtl[layerIndex]->localConstantTable;
        isTintSpecified = 0;
        for ( constIndex = 0; constIndex < layerMtl[layerIndex]->constantCount; ++constIndex )
        {
            memcpy(newConstEntry, &oldConstTable[constIndex], sizeof(MaterialConstantDef));
            if ( layerChar )
            {
                Material_AppendCharToConstName(newConstEntry->name, layerChar);
                newConstEntry->nameHash = layerChar ^ (33 * newConstEntry->nameHash);
            }
            if ( oldConstTable[constIndex].nameHash == tintConstNameHash )
                isTintSpecified = 1;
            ++newConstEntry;
        }
        if ( !isTintSpecified )
        {
            strncpy(newConstEntry->name, "colorTint", 0xCu);
            newConstEntry->nameHash = tintConstNameHash;
            literal = newConstEntry->literal;
            newConstEntry->literal[0] = colorWhite[0];
            literal[1] = 1.0;
            literal[2] = 1.0;
            literal[3] = 1.0;
            if ( layerChar )
            {
                Material_AppendCharToConstName(newConstEntry->name, layerChar);
                newConstEntry->nameHash = layerChar ^ (33 * newConstEntry->nameHash);
            }
            ++newConstEntry;
        }
    }
    if ( newTexEntry - newMtl->textureTable != newMtl->textureCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10021,
                    0,
                    "newTexEntry - newMtl->textureTable == newMtl->textureCount\n\t%i, %i",
                    newTexEntry - newMtl->textureTable,
                    newMtl->textureCount) )
    {
        __debugbreak();
    }
    if ( newConstEntry - newMtl->localConstantTable != newMtl->constantCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10022,
                    0,
                    "newConstEntry - newMtl->constantTable == newMtl->constantCount\n\t%i, %i",
                    newConstEntry - newMtl->localConstantTable,
                    newMtl->constantCount) )
    {
        __debugbreak();
    }
    qsort(
        newMtl->textureTable,
        newMtl->textureCount,
        0x10u,
        (int (__cdecl *)(const void *, const void *))CompareHashedMaterialTextures);
    qsort(
        newMtl->localConstantTable,
        newMtl->constantCount,
        0x20u,
        (int (__cdecl *)(const void *, const void *))CompareHashedMaterialTextures);
    Material_SetMaterialDrawRegion(newMtl);
    if ( Material_Validate(newMtl) )
        return newMtl;
    else
        return 0;
}

void __cdecl Material_SetMaterialDrawRegion(Material *material)
{
    unsigned int cameraRegion; // [esp+0h] [ebp-4h]

    cameraRegion = Material_GetTechniqueSetDrawRegion(material->localTechniqueSet);
    if ( !cameraRegion )
        LOBYTE(cameraRegion) = material->info.sortKey >= 0x18u;
    material->cameraRegion = cameraRegion;
}

unsigned int __cdecl Material_GetTechniqueSetDrawRegion(MaterialTechniqueSet *techniqueSet)
{
    if ( !techniqueSet
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    8986,
                    0,
                    "%s",
                    "techniqueSet") )
    {
        __debugbreak();
    }
    if ( techniqueSet->techniques[10] )
        return 0;
    if ( techniqueSet->techniques[5] )
        return 2;
    return 3;
}

void __cdecl Material_SetStateBits(Material *material, unsigned int (*stateBitsTable)[2], unsigned int stateBitsCount)
{
    unsigned __int8 *v3; // [esp+0h] [ebp-4h]

    material->stateBitsCount = stateBitsCount;
    if ( stateBitsCount )
        v3 = Material_Alloc(8 * stateBitsCount);
    else
        v3 = 0;
    material->stateBitsTable = (GfxStateBits *)v3;
    memcpy((unsigned __int8 *)material->stateBitsTable, (unsigned __int8 *)stateBitsTable, 8 * stateBitsCount);
    Material_UpdateStateFlags(material);
    Material_UpdateTechniqueFlags(material);
}

void __cdecl Material_UpdateStateFlags(Material *mtl)
{
    unsigned int CullFlags; // esi
    unsigned int v2; // esi
    int v3; // esi
    int v4; // esi
    unsigned int v5; // esi
    unsigned int stateFlags; // [esp+4h] [ebp-8h]

    if ( mtl->localTechniqueSet )
    {
        CullFlags = Material_GetCullFlags(mtl);
        v2 = Material_GetCullShadowFlags(mtl) | CullFlags;
        v3 = Material_GetDecalFlags(mtl) | v2;
        v4 = Material_GetWritesDepthFlags(mtl) | v3;
        v5 = Material_GetUsesDepthBufferFlags(mtl) | v4;
        stateFlags = Material_GetUsesStencilBufferFlags(mtl) | v5;
    }
    else
    {
        LOBYTE(stateFlags) = 0;
    }
    mtl->stateFlags = stateFlags;
}

unsigned int __cdecl Material_GetCullFlags(Material *material)
{
    unsigned int techType; // [esp+4h] [ebp-18h]
    unsigned int cullBits; // [esp+8h] [ebp-14h]
    unsigned int cullFlags; // [esp+Ch] [ebp-10h]
    unsigned int techTypeCullFlags; // [esp+10h] [ebp-Ch]
    MaterialTechniqueSet *techniqueSet; // [esp+14h] [ebp-8h]

    cullFlags = -1;
    techniqueSet = material->localTechniqueSet;
    if ( !techniqueSet
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    9246,
                    0,
                    "%s",
                    "techniqueSet") )
    {
        __debugbreak();
    }
    for ( techType = 10; techType < 0x73; ++techType )
    {
        if ( techniqueSet->techniques[techType] )
        {
            cullBits = material->stateBitsTable[material->stateBitsEntry[techType]].loadBits[0] & 0xC000;
            if ( cullBits == 0x8000 )
            {
                techTypeCullFlags = 1;
            }
            else if ( cullBits == 49152 )
            {
                techTypeCullFlags = 2;
            }
            else
            {
                techTypeCullFlags = 0;
            }
            if ( cullFlags != -1 && cullFlags != techTypeCullFlags )
            {
                Com_PrintWarning(
                    8,
                    "Material '%s' uses technique set '%s' with mismatched cull modes.\n",
                    material->info.name,
                    techniqueSet->name);
                break;
            }
            cullFlags = techTypeCullFlags;
        }
    }
    if ( cullFlags == -1 )
        return 0;
    return cullFlags;
}

unsigned int __cdecl Material_GetCullShadowFlags(Material *material)
{
    unsigned int cullBits; // [esp+4h] [ebp-10h]
    MaterialTechniqueSet *techniqueSet; // [esp+Ch] [ebp-8h]

    techniqueSet = material->localTechniqueSet;
    if ( !techniqueSet
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    9288,
                    0,
                    "%s",
                    "techniqueSet") )
    {
        __debugbreak();
    }
    if ( !techniqueSet->techniques[2] )
        return 0;
    cullBits = material->stateBitsTable[material->stateBitsEntry[2]].loadBits[0] & 0xC000;
    if ( cullBits == 0x8000 )
        return 64;
    if ( cullBits == 49152 )
        return 128;
    return 0;
}

int __cdecl Material_GetDecalFlags(const Material *mtl)
{
    int v2; // [esp+0h] [ebp-Ch]
    MaterialTechniqueSet *techniqueSet; // [esp+8h] [ebp-4h]

    if ( !mtl->stateBitsTable )
        return 0;
    techniqueSet = mtl->localTechniqueSet;
    if ( !techniqueSet
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    9315,
                    0,
                    "%s",
                    "techniqueSet") )
    {
        __debugbreak();
    }
    if ( techniqueSet->techniques[4] )
        v2 = mtl->stateBitsEntry[4];
    else
        v2 = 0;
    return (mtl->stateBitsTable[v2].loadBits[1] & 0x30) != 0 ? 4 : 0;
}

int __cdecl Material_GetWritesDepthFlags(const Material *mtl)
{
    int v2; // [esp+0h] [ebp-Ch]
    MaterialTechniqueSet *techniqueSet; // [esp+8h] [ebp-4h]

    if ( !mtl->stateBitsTable )
        return 0;
    techniqueSet = mtl->localTechniqueSet;
    if ( !techniqueSet
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    9330,
                    0,
                    "%s",
                    "techniqueSet") )
    {
        __debugbreak();
    }
    if ( techniqueSet->techniques[4] )
        v2 = mtl->stateBitsEntry[4];
    else
        v2 = 0;
    return (mtl->stateBitsTable[v2].loadBits[1] & 1) != 0 ? 8 : 0;
}

unsigned int __cdecl Material_GetUsesDepthBufferFlags(const Material *mtl)
{
    const MaterialTechnique *technique; // [esp+0h] [ebp-14h]
    unsigned int techType; // [esp+4h] [ebp-10h]
    MaterialTechniqueSet *techniqueSet; // [esp+8h] [ebp-Ch]
    const GfxStateBits *refStateBits; // [esp+Ch] [ebp-8h]
    unsigned int passIndex; // [esp+10h] [ebp-4h]

    techniqueSet = mtl->localTechniqueSet;
    if ( !techniqueSet
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    9345,
                    0,
                    "%s",
                    "techniqueSet") )
    {
        __debugbreak();
    }
    for ( techType = 0; techType < 0x82; ++techType )
    {
        technique = techniqueSet->techniques[techType];
        if ( technique )
        {
            for ( passIndex = 0; passIndex < technique->passCount; ++passIndex )
            {
                refStateBits = &mtl->stateBitsTable[passIndex + mtl->stateBitsEntry[techType]];
                if ( (refStateBits->loadBits[1] & 0x3D) != 0 )
                    return 16;
                if ( (refStateBits->loadBits[1] & 2) == 0 )
                    return 16;
            }
        }
    }
    return 0;
}

unsigned int __cdecl Material_GetUsesStencilBufferFlags(const Material *mtl)
{
    const MaterialTechnique *technique; // [esp+0h] [ebp-14h]
    unsigned int techType; // [esp+4h] [ebp-10h]
    MaterialTechniqueSet *techniqueSet; // [esp+8h] [ebp-Ch]
    unsigned int passIndex; // [esp+10h] [ebp-4h]

    techniqueSet = mtl->localTechniqueSet;
    if ( !techniqueSet
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    9375,
                    0,
                    "%s",
                    "techniqueSet") )
    {
        __debugbreak();
    }
    for ( techType = 0; techType < 0x82; ++techType )
    {
        technique = techniqueSet->techniques[techType];
        if ( technique )
        {
            for ( passIndex = 0; passIndex < technique->passCount; ++passIndex )
            {
                if ( (mtl->stateBitsTable[passIndex + mtl->stateBitsEntry[techType]].loadBits[1] & 0xC0) != 0 )
                    return 32;
            }
        }
    }
    return 0;
}

void __cdecl Material_UpdateTechniqueFlags(Material *material)
{
    MaterialTechnique *technique; // [esp+0h] [ebp-10h]
    unsigned int techType; // [esp+4h] [ebp-Ch]
    MaterialTechniqueSet *techniqueSet; // [esp+8h] [ebp-8h]
    const GfxStateBits *refStateBits; // [esp+Ch] [ebp-4h]

    techniqueSet = material->localTechniqueSet;
    if ( !techniqueSet
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    9426,
                    0,
                    "%s",
                    "techniqueSet") )
    {
        __debugbreak();
    }
    for ( techType = 0; techType < 0x82; ++techType )
    {
        technique = techniqueSet->techniques[techType];
        if ( technique )
        {
            refStateBits = &material->stateBitsTable[material->stateBitsEntry[techType]];
            if ( (refStateBits->loadBits[1] & 0x3D) != 0 )
                technique->flags |= 0x80u;
            if ( (refStateBits->loadBits[1] & 2) == 0 )
                technique->flags |= 0x80u;
        }
    }
}

char __cdecl Material_HasConstant(const Material *mtl, unsigned int nameHash)
{
    unsigned int constantIndex; // [esp+0h] [ebp-4h]

    for ( constantIndex = 0; constantIndex < mtl->constantCount; ++constantIndex )
    {
        if ( mtl->localConstantTable[constantIndex].nameHash == nameHash )
            return 1;
    }
    return 0;
}

char __cdecl Material_Validate(const Material *material)
{
    int techType; // [esp+0h] [ebp-4h]

    for ( techType = 0; techType < 130; ++techType )
    {
        if ( material->localTechniqueSet->techniques[techType]
            && !Material_ValidateTechnique(material, material->localTechniqueSet->techniques[techType]) )
        {
            return 0;
        }
    }
    return 1;
}

char __cdecl Material_ValidateTechnique(const Material *material, const MaterialTechnique *technique)
{
    unsigned int passIndex; // [esp+8h] [ebp-4h]

    for ( passIndex = 0; passIndex < technique->passCount; ++passIndex )
    {
        if ( !Material_ValidatePassArguments(
                        material,
                        technique->name,
                        technique->passArray[passIndex].stableArgCount
                    + technique->passArray[passIndex].perObjArgCount
                    + technique->passArray[passIndex].perPrimArgCount,
                        technique->passArray[passIndex].localArgs) )
            return 0;
    }
    return 1;
}

char __cdecl Material_ValidatePassArguments(
                const Material *mtl,
                const char *techniqueName,
                unsigned int argCount,
                const MaterialShaderArgument *args)
{
    unsigned int argIndex; // [esp+0h] [ebp-8h]
    const char *argName; // [esp+4h] [ebp-4h]
    const char *argNamea; // [esp+4h] [ebp-4h]

    for ( argIndex = 0; argIndex < argCount; ++argIndex )
    {
        if ( args[argIndex].type && args[argIndex].type != 6 )
        {
            if ( args[argIndex].type == 2 && !Material_HasTexture(mtl, args[argIndex].u.codeSampler) )
            {
                argNamea = Material_StringFromHash(args[argIndex].u.codeSampler);
                Com_PrintError(
                    8,
                    "material '%s' using technique '%s' from techniqueSet '%s' doesn't expose a '%s' texture\n",
                    mtl->info.name,
                    techniqueName,
                    mtl->localTechniqueSet->name,
                    argNamea);
                return 0;
            }
        }
        else if ( !Material_HasConstant(mtl, args[argIndex].u.codeSampler) )
        {
            argName = Material_StringFromHash(args[argIndex].u.codeSampler);
            Com_PrintError(
                8,
                "material '%s' using technique '%s' from techniqueSet '%s' doesn't expose a '%s' constant\n",
                mtl->info.name,
                techniqueName,
                mtl->localTechniqueSet->name,
                argName);
            return 0;
        }
    }
    return 1;
}

const char *__cdecl Material_StringFromHash(unsigned int hash)
{
    unsigned int hashIndex; // [esp+0h] [ebp-4h]

    for ( hashIndex = hash & 0x3FF; mtlLoadGlob.stringHashTable[hashIndex].string; hashIndex = ((_WORD)hashIndex + 1)
                                                                                                                                                                                     & 0x3FF )
    {
        if ( mtlLoadGlob.stringHashTable[hashIndex].hash == hash )
            return mtlLoadGlob.stringHashTable[hashIndex].string;
    }
    return 0;
}

char __cdecl Material_HasTexture(const Material *mtl, unsigned int nameHash)
{
    unsigned int textureIndex; // [esp+0h] [ebp-4h]

    for ( textureIndex = 0; textureIndex < mtl->textureCount; ++textureIndex )
    {
        if ( mtl->textureTable[textureIndex].nameHash == nameHash )
            return 1;
    }
    return 0;
}

int __cdecl CompareHashedMaterialTextures(unsigned int *e0, unsigned int *e1)
{
    return *e0 < *e1 ? -1 : 1;
}

void __cdecl Material_AppendCharToConstName(char *name, char ch)
{
    unsigned int nameIndex; // [esp+4h] [ebp-4h]

    for ( nameIndex = 0; nameIndex < 0xC; ++nameIndex )
    {
        if ( !name[nameIndex] )
        {
            name[nameIndex] = ch;
            if ( nameIndex + 1 < 0xC )
                name[nameIndex + 1] = 0;
            return;
        }
    }
}

unsigned int __cdecl Material_CreateLayeredStateBitsTable(
                const Material **layerMtl,
                unsigned int layerCount,
                const MaterialTechniqueSet *techSet,
                unsigned __int8 *stateBitsEntry,
                unsigned int (*stateBitsTable)[2])
{
    unsigned int techType; // [esp+0h] [ebp-10h]
    unsigned int derivedStateBits[2]; // [esp+4h] [ebp-Ch] BYREF
    unsigned int stateBitsCount; // [esp+Ch] [ebp-4h] BYREF

    stateBitsCount = 0;
    for ( techType = 0; techType < 0x82; ++techType )
    {
        if ( techSet->techniques[techType] )
        {
            if ( !(*layerMtl)->localTechniqueSet->techniques[techType]
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                            9820,
                            0,
                            "%s",
                            "layerMtl[0]->techniqueSet->techniques[techType] != NULL") )
            {
                __debugbreak();
            }
            if ( (*layerMtl)->localTechniqueSet->techniques[techType]->passCount != 1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                            9821,
                            0,
                            "%s",
                            "layerMtl[0]->techniqueSet->techniques[techType]->passCount == 1") )
            {
                __debugbreak();
            }
            Material_GetLayeredStateBits(layerMtl, layerCount, techType, derivedStateBits);
            if ( (derivedStateBits[0] & 0x3800) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                            9823,
                            0,
                            "%s",
                            "derivedStateBits[0] & (GFXS0_ATEST_MASK | GFXS0_ATEST_DISABLE)") )
            {
                __debugbreak();
            }
            stateBitsEntry[techType] = Material_AddStateBitsArrayToTable(
                                                                     (const unsigned int (*)[2])derivedStateBits,
                                                                     1u,
                                                                     stateBitsTable,
                                                                     &stateBitsCount);
        }
        else
        {
            stateBitsEntry[techType] = -1;
        }
    }
    return stateBitsCount;
}

unsigned __int8 __cdecl Material_AddStateBitsArrayToTable(
                const unsigned int (*stateBitsForPass)[2],
                unsigned int passCount,
                unsigned int (*stateBitsTable)[2],
                unsigned int *stateBitsCount)
{
    unsigned int scan; // [esp+8h] [ebp-8h]
    unsigned int partialMatchCount; // [esp+Ch] [ebp-4h]

    for ( scan = 0; ; ++scan )
    {
        partialMatchCount = *stateBitsCount - scan;
        if ( *stateBitsCount == scan )
            break;
        if ( partialMatchCount > passCount )
            partialMatchCount = passCount;
        if ( !memcmp(&(*stateBitsTable)[2 * scan], stateBitsForPass, 8 * partialMatchCount) )
            break;
    }
    memcpy(
        (unsigned __int8 *)&(*stateBitsTable)[2 * *stateBitsCount],
        (unsigned __int8 *)&(*stateBitsForPass)[2 * partialMatchCount],
        8 * (passCount - partialMatchCount));
    *stateBitsCount += passCount - partialMatchCount;
    if ( scan != (unsigned __int8)scan
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    9229,
                    0,
                    "scan == static_cast< byte >( scan )\n\t%i, %i",
                    scan,
                    (unsigned __int8)scan) )
    {
        __debugbreak();
    }
    return scan;
}

void __cdecl Material_GetLayeredStateBits(
                const Material **layerMtl,
                unsigned int layerCount,
                unsigned int techType,
                unsigned int *stateBits)
{
    char v4; // [esp+7h] [ebp-Dh]
    const GfxStateBits *srcStateBitsa; // [esp+8h] [ebp-Ch]
    const GfxStateBits *srcStateBits; // [esp+8h] [ebp-Ch]
    unsigned int layerEntry; // [esp+Ch] [ebp-8h]
    unsigned int layerIndex; // [esp+10h] [ebp-4h]

    if ( (*layerMtl)->stateBitsEntry[techType] == 255
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    9757,
                    0,
                    "%s",
                    "layerMtl[0]->stateBitsEntry[techType] != UCHAR_MAX") )
    {
        __debugbreak();
    }
    srcStateBitsa = &(*layerMtl)->stateBitsTable[(*layerMtl)->stateBitsEntry[techType]];
    *stateBits = srcStateBitsa->loadBits[0];
    stateBits[1] = srcStateBitsa->loadBits[1];
    switch ( techType )
    {
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case ':':
        case '<':
        case '>':
        case '@':
        case 'B':
        case 'D':
        case 'F':
        case 'H':
            v4 = 1;
            break;
        default:
            v4 = 0;
            break;
    }
    if ( !v4 )
    {
        if ( (*stateBits & 0xF0) == 0x10 )
        {
            if ( (*stateBits & 0x3800) != 0x800 )
            {
                for ( layerIndex = 0; layerIndex < layerCount; ++layerIndex )
                {
                    layerEntry = layerMtl[layerIndex]->stateBitsEntry[techType];
                    if ( layerEntry != 255 )
                    {
                        srcStateBits = &layerMtl[layerIndex]->stateBitsTable[layerEntry];
                        if ( (srcStateBits->loadBits[0] & 0xF0) != 0x10 )
                        {
                            *stateBits &= 0xFFFFC000;
                            *stateBits |= 0x800u;
                            *stateBits |= 0x100u;
                            *stateBits |= 2u;
                            *stateBits |= srcStateBits->loadBits[0] & 0xF0;
                            return;
                        }
                    }
                }
            }
        }
        else if ( (*stateBits & 0xF) != 1 )
        {
            *stateBits &= 0xFFFFFFF0;
            *stateBits |= 2u;
        }
    }
}

bool __cdecl Material_HasNormalMap(const Material *mtl)
{
    const char *techSetName; // [esp+14h] [ebp-Ch]
    unsigned int texIndex; // [esp+18h] [ebp-8h]
    unsigned int normalMapNameHash; // [esp+1Ch] [ebp-4h]

    techSetName = mtl->localTechniqueSet->name;
    if ( !strncmp(techSetName, "w_", 2u) )
    {
        techSetName += 2;
    }
    else if ( !strncmp(techSetName, "wc_", 3u) )
    {
        techSetName += 3;
    }
    if ( !strncmp(techSetName, "swb_", 4u) )
        return 1;
    if ( !mtl
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp", 10062, 0, "%s", "mtl") )
    {
        __debugbreak();
    }
    normalMapNameHash = R_HashString("normalMap");
    for ( texIndex = 0; ; ++texIndex )
    {
        if ( texIndex >= mtl->textureCount )
            return 0;
        if ( mtl->textureTable[texIndex].nameHash == normalMapNameHash )
            break;
    }
    if ( mtl->textureTable[texIndex].nameStart != 110
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10070,
                    0,
                    "%s",
                    "mtl->textureTable[texIndex].nameStart == 'n'") )
    {
        __debugbreak();
    }
    if ( mtl->textureTable[texIndex].nameEnd != 112
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10071,
                    0,
                    "%s",
                    "mtl->textureTable[texIndex].nameEnd == 'p'") )
    {
        __debugbreak();
    }
    if ( mtl->textureTable[texIndex].semantic != 5
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10072,
                    0,
                    "%s",
                    "mtl->textureTable[texIndex].semantic == TS_NORMAL_MAP") )
    {
        __debugbreak();
    }
    return strcmp(mtl->textureTable[texIndex].u.image->name, "$identitynormalmap") != 0;
}

MaterialTechniqueSet *__cdecl Material_RegisterLayeredTechniqueSet(const Material **mtl, unsigned int layerCount)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v5; // eax
    const char *v6; // eax
    unsigned __int8 v7; // al
    char v8; // [esp+1Bh] [ebp-8A9h]
    char *v9; // [esp+20h] [ebp-8A4h]
    const char *name; // [esp+24h] [ebp-8A0h]
    char *v12; // [esp+2Ch] [ebp-898h]
    const char *v13; // [esp+40h] [ebp-884h]
    char *v15; // [esp+48h] [ebp-87Ch]
    char *v17; // [esp+50h] [ebp-874h]
    char buf[2052]; // [esp+54h] [ebp-870h] BYREF
    char *end; // [esp+858h] [ebp-6Ch]
    const char *base_techset_name; // [esp+85Ch] [ebp-68h]
    unsigned int newTechSetNameLen; // [esp+860h] [ebp-64h]
    MaterialWorldVertexFormat worldVertFormat; // [esp+864h] [ebp-60h]
    char layerToken; // [esp+86Bh] [ebp-59h]
    unsigned int normalMapCount; // [esp+86Ch] [ebp-58h]
    MaterialTechniqueSet *techSet; // [esp+870h] [ebp-54h] BYREF
    char newTechSetName[64]; // [esp+874h] [ebp-50h] BYREF
    unsigned int layerIndex; // [esp+8B8h] [ebp-Ch]
    const LayeredTechniqueSetName *lyrTechSetName; // [esp+8BCh] [ebp-8h]
    bool found; // [esp+8C3h] [ebp-1h]

    if ( layerCount - 1 >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10151,
                    0,
                    "layerCount - 1 doesn't index ARRAY_COUNT( s_worldVertFormatForLayerCount )\n\t%i not in [0, %i)",
                    layerCount - 1,
                    4) )
    {
        __debugbreak();
    }
    worldVertFormat = (MaterialWorldVertexFormat)s_stateMapDstStencilBitGroup[9].stateBitsMask[layerCount + 1];
    normalMapCount = 0;
    newTechSetNameLen = 0;
    found = 0;
    if ( layerCount == 2 )
    {
        base_techset_name = (*mtl)->localTechniqueSet->name;
        if ( !strncmp(base_techset_name, "w_", 2u) )
        {
            base_techset_name += 2;
        }
        else if ( !strncmp(base_techset_name, "wc_", 3u) )
        {
            base_techset_name += 3;
        }
        else
        {
            base_techset_name = 0;
        }
        if ( base_techset_name )
        {
            if ( !strncmp(base_techset_name, "swb_", 4u) )
            {
                lyrTechSetName = Material_GetLayeredTechniqueSetName(mtl[1]->localTechniqueSet->name);
                if ( lyrTechSetName )
                {
                    if ( *lyrTechSetName->nameChunk == 98 )
                    {
                        normalMapCount = 1;
                        strcpy(newTechSetName, "swl_");
                        v2 = strstr(lyrTechSetName->nameChunk, "n0");
                        if ( v2 )
                        {
                            v17 = (char *)&techSet + 3;
                            while ( *++v17 )
                                ;
                            strcpy(v17, "n1_");
                            ++normalMapCount;
                        }
                        v3 = strstr(lyrTechSetName->nameChunk, "d0");
                        if ( v3 )
                        {
                            v15 = (char *)&techSet + 3;
                            while ( *++v15 )
                                ;
                            strcpy(v15, "d1_");
                        }
                        v13 = &base_techset_name[strlen(base_techset_name + 4) + 5];
                        v12 = (char *)&techSet + 3;
                        while ( *++v12 )
                            ;
                        memcpy(v12, base_techset_name + 4, v13 - (base_techset_name + 4));
                        found = 1;
                    }
                }
            }
        }
    }
    if ( !found )
    {
        for ( layerIndex = 0; layerIndex < layerCount; ++layerIndex )
        {
            lyrTechSetName = Material_GetLayeredTechniqueSetName(mtl[layerIndex]->localTechniqueSet->name);
            if ( !lyrTechSetName )
            {
                Com_PrintWarning(
                    8,
                    "Material '%s' uses technique set '%s' which cannot be used in a layered material; using default instead.    Reco"
                    "mpile the bsp to fix.\n",
                    mtl[layerIndex]->info.name,
                    mtl[layerIndex]->localTechniqueSet->name);
                return 0;
            }
            layerToken = layerIndex + 48;
            if ( (char)(layerIndex + 48) == 48 )
            {
                if ( lyrTechSetName->namePrefixRegister )
                    newTechSetNameLen = Material_AppendTechniqueSetName(
                                                                newTechSetName,
                                                                newTechSetNameLen,
                                                                (char *)lyrTechSetName->namePrefixRegister,
                                                                layerToken);
            }
            else
            {
                newTechSetNameLen = Material_AppendTechniqueSetName(newTechSetName, newTechSetNameLen, (char*)"_", layerToken);
            }
            if ( lyrTechSetName )
            {
                v5 = strchr(lyrTechSetName->nameChunk, 0x6Eu);
                if ( v5 )
                    ++normalMapCount;
            }
            newTechSetNameLen = Material_AppendTechniqueSetName(
                                                        newTechSetName,
                                                        newTechSetNameLen,
                                                        (char *)lyrTechSetName->nameChunk,
                                                        layerToken);
        }
    }
    techSet = Material_RegisterTechniqueSet(newTechSetName);
    if ( techSet )
    {
        if ( normalMapCount > 1 )
            worldVertFormat = (MaterialWorldVertexFormat)(worldVertFormat + normalMapCount - 1);
        if ( techSet->worldVertFormat )
        {
            if ( techSet->worldVertFormat != worldVertFormat )
            {
                v6 = va("%i, %i", techSet->worldVertFormat, worldVertFormat);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                                10221,
                                0,
                                "%s\n\t%s",
                                "techSet->worldVertFormat == MTL_WORLDVERT_TEX_1_NRM_1 || techSet->worldVertFormat == worldVertFormat",
                                v6) )
                    __debugbreak();
            }
        }
        v7 = truncate_cast<unsigned char>((int)worldVertFormat);
        techSet->worldVertFormat = v7;
    }
    else
    {
        buf[0] = 0;
        end = buf;
        for ( layerIndex = 0; layerIndex < layerCount; ++layerIndex )
        {
            name = mtl[layerIndex]->info.name;
            v9 = end;
            do
            {
                v8 = *name;
                *v9++ = *name++;
            }
            while ( v8 );
            end += strlen(end);
            *end++ = 10;
        }
        *end++ = 0;
        Com_ScriptError(
            "Cannot create techset '%s'. The offending %i-layer combination is:\n%s",
            newTechSetName,
            layerCount,
            buf);
    }
    return techSet;
}

unsigned int __cdecl Material_AppendTechniqueSetName(char *name, unsigned int nameLen, char *append, char lyrToken)
{
    char v5; // [esp+3h] [ebp-1h]

    while ( *append )
    {
        if ( nameLen == 63 )
        {
            name[63] = 0;
            Com_Error(ERR_FATAL, "technique set name %s%s is too long", name, append);
        }
        if ( *append == 48 )
            v5 = lyrToken;
        else
            v5 = *append;
        name[nameLen++] = v5;
        ++append;
    }
    name[nameLen] = 0;
    return nameLen;
}

const LayeredTechniqueSetName *__cdecl Material_GetLayeredTechniqueSetName(const char *techSetName)
{
    unsigned int top; // [esp+0h] [ebp-10h]
    unsigned int bot; // [esp+4h] [ebp-Ch]
    int comparison; // [esp+8h] [ebp-8h]
    unsigned int mid; // [esp+Ch] [ebp-4h]
    const char *techSetNamea; // [esp+18h] [ebp+8h]

    if ( !strncmp(techSetName, "w_", 2u) )
    {
        techSetNamea = techSetName + 2;
    }
    else
    {
        if ( strncmp(techSetName, "wc_", 3u) )
            return 0;
        techSetNamea = techSetName + 3;
    }
    bot = 0;
    top = 121;
    do
    {
        mid = (top + bot) >> 1;
        comparison = I_strcmp(techSetNamea, s_lyrTechSetNames[mid].inputName);
        if ( comparison >= 0 )
        {
            if ( comparison <= 0 )
                return &s_lyrTechSetNames[mid];
            bot = mid + 1;
        }
        else
        {
            top = (top + bot) >> 1;
        }
    }
    while ( bot < top );
    return 0;
}

Material *__cdecl Material_LoadRaw(const MaterialRaw *mtlRaw, unsigned int materialType, int imageTrack)
{
    int v4; // ecx
    unsigned int v5; // eax
    unsigned int v6; // [esp+1Ch] [ebp-6Ch]
    float *literal; // [esp+3Ch] [ebp-4Ch]
    const float *v8; // [esp+40h] [ebp-48h]
    int texIndex; // [esp+44h] [ebp-44h]
    char *imageName; // [esp+48h] [ebp-40h]
    char *constName; // [esp+4Ch] [ebp-3Ch]
    char *strDest; // [esp+50h] [ebp-38h]
    const MaterialConstantDefRaw *constantTableRaw; // [esp+54h] [ebp-34h]
    Material *material; // [esp+60h] [ebp-28h]
    const char *tableEntryName; // [esp+68h] [ebp-20h]
    char *name; // [esp+6Ch] [ebp-1Ch]
    unsigned int prefixLen; // [esp+70h] [ebp-18h]
    MaterialTechniqueSet *techniqueSet; // [esp+74h] [ebp-14h] BYREF
    const MaterialTextureDefRaw *textureTableRaw; // [esp+78h] [ebp-10h]
    unsigned int constIndex; // [esp+7Ch] [ebp-Ch]
    void *materialMem; // [esp+80h] [ebp-8h]
    unsigned int texIndexIn; // [esp+84h] [ebp-4h]

    if ( !Material_FinishLoadingInstance(
                    mtlRaw,
                    g_materialTypeInfo[materialType].techniqueSetPrefix,
                    &techniqueSet,
                    materialType,
                    imageTrack) )
        return 0;
    name = (char *)mtlRaw + mtlRaw->info.nameOffset;
    prefixLen = g_materialTypeInfo[materialType].prefixLen;
    v6 = strlen(name);
    materialMem = Material_Alloc(prefixLen + v6 + 1 + 192);
    material = (Material *)materialMem;
    strDest = (char *)materialMem + 192;
    memcpy((unsigned __int8 *)materialMem + 192, (unsigned __int8 *)g_materialTypeInfo[materialType].prefix, prefixLen);
    memcpy((unsigned __int8 *)&strDest[prefixLen], (unsigned __int8 *)name, v6 + 1);
    material->info.name = strDest;

    iassert(!material->info.drawSurf.fields.materialSortedIndex);


    material->info.gameFlags = mtlRaw->info.gameFlags;
    v4 = (int)(0x3F00000 & mtlRaw->info.surfaceFlags) >> 20;
    if ( (_BYTE)v4 )
    {
        material->info.surfaceTypeBits = 1 << (v4 - 1);
        material->info.layeredSurfaceTypes = (unsigned __int8)v4;
    }
    else
    {
        material->info.surfaceTypeBits = 0;
        material->info.layeredSurfaceTypes = 0;
    }
    material->info.layeredSurfaceTypes |= 0x20000000u;
    if ( materialType != 3 && materialType != 4 )
        material->info.gameFlags &= ~2u;
    material->info.sortKey = mtlRaw->info.sortKey;
    material->info.textureAtlasRowCount = mtlRaw->info.textureAtlasRowCount;
    material->info.textureAtlasColumnCount = mtlRaw->info.textureAtlasColumnCount;
    material->textureCount = mtlRaw->textureCount;
    material->constantCount = mtlRaw->constantCount;
    material->localTechniqueSet = techniqueSet;
    if ( mtlRaw->textureCount )
    {
        material->textureTable = (MaterialTextureDef *)Material_Alloc(16 * mtlRaw->textureCount);
        textureTableRaw = (const MaterialTextureDefRaw *)((char *)mtlRaw + mtlRaw->textureTableOffset);
        texIndex = 0;
        for ( texIndexIn = 0; texIndexIn < mtlRaw->textureCount; ++texIndexIn )
        {
            tableEntryName = (char *)mtlRaw + textureTableRaw[texIndexIn].nameOffset;
            material->textureTable[texIndex].nameHash = R_HashString(tableEntryName);
            material->textureTable[texIndex].nameStart = *tableEntryName;
            material->textureTable[texIndex].nameEnd = tableEntryName[strlen(tableEntryName) - 1];
            material->textureTable[texIndex].samplerState = textureTableRaw[texIndexIn].samplerState;
            if ( (material->textureTable[texIndex].samplerState & 7) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                            10432,
                            0,
                            "%s",
                            "material->textureTable[texIndex].samplerState & SAMPLER_FILTER_MASK") )
            {
                __debugbreak();
            }
            material->textureTable[texIndex].semantic = textureTableRaw[texIndexIn].semantic;
            material->textureTable[texIndex].isMatureContent = 0;
            if ( material->textureTable[texIndex].semantic == 11 )
            {
                material->textureTable[texIndex++].u.image = (GfxImage *)Material_RegisterWaterImage((const MaterialWaterDef *)((char *)mtlRaw + textureTableRaw[texIndexIn].u.imageNameOffset));
            }
            else if ( material->textureTable[texIndex].semantic != 28 )
            {
                imageName = (char *)mtlRaw + textureTableRaw[texIndexIn].u.imageNameOffset;
                material->textureTable[texIndex].u.image = Image_Register(
                                                                                                         imageName,
                                                                                                         textureTableRaw[texIndexIn].semantic,
                                                                                                         imageTrack);
                material->textureTable[texIndex++].isMatureContent = MaterialTexture_IsMatureContent(
                                                                                                                             material->textureTable[texIndexIn].semantic,
                                                                                                                             imageName);
            }
        }
        material->textureCount = texIndex;
    }
    if ( mtlRaw->constantCount )
    {
        material->localConstantTable = (MaterialConstantDef *)Material_Alloc(32 * mtlRaw->constantCount);
        constantTableRaw = (const MaterialConstantDefRaw *)((char *)mtlRaw + mtlRaw->constantTableOffset);
        for ( constIndex = 0; constIndex < mtlRaw->constantCount; ++constIndex )
        {
            constName = (char *)mtlRaw + constantTableRaw[constIndex].nameOffset;
            v5 = R_HashString(constName);
            material->localConstantTable[constIndex].nameHash = v5;
            strncpy(material->localConstantTable[constIndex].name, constName, 0xCu);
            literal = material->localConstantTable[constIndex].literal;
            v8 = constantTableRaw[constIndex].literal;
            *literal = *v8;
            literal[1] = v8[1];
            literal[2] = v8[2];
            literal[3] = v8[3];
        }
    }
    Material_BuildStateBitsTable(material, mtlRaw->info.toolFlags, (unsigned int*)mtlRaw->refStateBits);
    Material_SetMaterialDrawRegion(material);
    if ( !Material_Validate(material) )
        return 0;
    if ( !material->localTechniqueSet
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10495,
                    0,
                    "%s",
                    "material->techniqueSet") )
    {
        __debugbreak();
    }
    if ( (material->info.gameFlags & 0x40) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10496,
                    0,
                    "%s",
                    "!(material->info.gameFlags & MTL_GAMEFLAG_CASTS_SHADOW)") )
    {
        __debugbreak();
    }
    if ( (mtlRaw->info.surfaceFlags & 0x40000) == 0
        && Material_GetTechnique(material, 2u)
        && (material->stateFlags & 4) == 0 )
    {
        material->info.gameFlags |= 0x40u;
    }
    if ( (mtlRaw->info.surfaceFlags & 0x80000) != 0
        && Material_GetTechnique(material, 2u)
        && (material->stateFlags & 4) == 0 )
    {
        material->info.gameFlags |= 1u;
    }
    if ( (mtlRaw->info.surfaceFlags & 0x100) != 0 )
        material->info.gameFlags |= 0x80u;
    if ( (mtlRaw->info.surfaceFlags & 0x200) != 0 )
        material->info.gameFlags |= 0x100u;
    if ( (mtlRaw->info.surfaceFlags & 0x800) != 0 )
        material->info.gameFlags |= 0x200u;
    return material;
}

water_t *__cdecl Material_RegisterWaterImage(const MaterialWaterDef *water)
{
    int v2; // [esp+0h] [ebp-58h]
    int v3; // [esp+4h] [ebp-54h]
    water_t setup; // [esp+10h] [ebp-48h] BYREF

    setup.M = truncate_cast<unsigned short>(water->textureWidth);
    setup.N = setup.M;
    setup.Lx = water->horizontalWorldLength;
    setup.Lz = water->verticalWorldLength;
    setup.gravity = 800.0f;
    setup.windvel = water->windSpeed;
    *(_QWORD *)setup.winddir = *(_QWORD *)water->windDirection;
    setup.amplitude = water->amplitude;
    setup.image = 0;
    if ( setup.M >> r_picmip_water->current.integer < 4 )
        v3 = 4;
    else
        v3 = setup.M >> r_picmip_water->current.integer;
    setup.M = v3;
    if ( setup.N >> r_picmip_water->current.integer < 4 )
        v2 = 4;
    else
        v2 = setup.N >> r_picmip_water->current.integer;
    setup.N = v2;
    return R_LoadWaterSetup(&setup);
}

void __cdecl Material_BuildStateBitsTable(Material *material, __int16 toolFlags, unsigned int *refStateBits)
{
    MaterialTechnique *v3; // [esp+0h] [ebp-1074h]
    unsigned int i; // [esp+4h] [ebp-1070h]
    int v5; // [esp+8h] [ebp-106Ch]
    unsigned int stateBitsTable[2]; // [esp+Ch] [ebp-1068h] BYREF
    _QWORD stateBitsOut[4]; // [esp+104Ch] [ebp-28h] BYREF
    unsigned int stateBitsCount; // [esp+106Ch] [ebp-8h] BYREF
    unsigned int j; // [esp+1070h] [ebp-4h]

    stateBitsCount = 0;
    for ( i = 0; i < 0x82; ++i )
    {
        v3 = material->localTechniqueSet->techniques[i];
        if ( v3 )
        {
            v5 = (int)&v3->passArray[v3->passCount];
            for ( j = 0; j < v3->passCount; ++j )
                Material_RemapStateBits(
                    material,
                    toolFlags,
                    *(const MaterialStateMap **)(v5 + 4 * j),
                    refStateBits,
                    (unsigned int *)&stateBitsOut[j]);
            material->stateBitsEntry[i] = Material_AddStateBitsArrayToTable(
                                                                            (const unsigned int (*)[2])stateBitsOut,
                                                                            v3->passCount,
                                                                            (unsigned int (*)[2])stateBitsTable,
                                                                            &stateBitsCount);
        }
        else
        {
            material->stateBitsEntry[i] = -1;
        }
    }
    Material_SetStateBits(material, (unsigned int (*)[2])stateBitsTable, stateBitsCount);
}

void __cdecl Material_RemapStateBits(
                const Material *material,
                __int16 toolFlags,
                const MaterialStateMap *stateMap,
                const unsigned int *refStateBits,
                unsigned int *stateBitsOut)
{
    unsigned int ruleSetIndex; // [esp+Ch] [ebp-4h]

    *stateBitsOut = *refStateBits;
    stateBitsOut[1] = refStateBits[1];
    for ( ruleSetIndex = 0; ruleSetIndex < 0xA; ++ruleSetIndex )
        Material_ApplyStateBitsRemapRuleSet(material, stateMap, ruleSetIndex, refStateBits, stateBitsOut);
    if ( (toolFlags & 0x200) == 0 && (stateBitsOut[1] & 0x30) == 0x10 )
    {
        stateBitsOut[1] &= 0xFFFFFFCF;
        stateBitsOut[1] = stateBitsOut[1];
    }
}

void __cdecl Material_ApplyStateBitsRemapRuleSet(
                const Material *material,
                const MaterialStateMap *stateMap,
                unsigned int ruleSetIndex,
                const unsigned int *refStateBits,
                unsigned int *stateBitsOut)
{
    const MaterialStateMapRuleSet *ruleSet; // [esp+4h] [ebp-8h]
    int ruleIndex; // [esp+8h] [ebp-4h]

    ruleSet = stateMap->ruleSet[ruleSetIndex];
    for ( ruleIndex = 0; ruleIndex < ruleSet->ruleCount; ++ruleIndex )
    {
        if ( (ruleSet->rules[ruleIndex].stateBitsMask[0] & *refStateBits) == ruleSet->rules[ruleIndex].stateBitsValue[0]
            && (ruleSet->rules[ruleIndex].stateBitsMask[1] & refStateBits[1]) == ruleSet->rules[ruleIndex].stateBitsValue[1] )
        {
            *stateBitsOut = ruleSet->rules[ruleIndex].stateBitsSet[0]
                                        | ruleSet->rules[ruleIndex].stateBitsClear[0] & *stateBitsOut;
            stateBitsOut[1] = ruleSet->rules[ruleIndex].stateBitsSet[1]
                                            | ruleSet->rules[ruleIndex].stateBitsClear[1] & stateBitsOut[1];
            return;
        }
    }
    Com_Error(
        ERR_FATAL,
        "No rule in stateMap '%s' rule set %i matched the current material state for material '%s'",
        stateMap->name,
        ruleSetIndex,
        material->info.name);
}

bool __cdecl Material_FinishLoadingInstance(
                const MaterialRaw *mtlRaw,
                const char *techniqueSetVertDeclPrefix,
                MaterialTechniqueSet **techniqueSet,
                unsigned int materialType,
                int imageTrack)
{
    MaterialConstantDefRaw *constantTable; // [esp+0h] [ebp-118h]
    int constantIndex; // [esp+4h] [ebp-114h]
    char techniqueSetName[260]; // [esp+8h] [ebp-110h] BYREF
    MaterialTextureDefRaw *textureTable; // [esp+110h] [ebp-8h]
    int textureIndex; // [esp+114h] [ebp-4h]

    if ( !mtlRaw
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp", 9697, 0, "%s", "mtlRaw") )
    {
        __debugbreak();
    }
    if ( mtlRaw->info.sortKey >= 0x40u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    9698,
                    0,
                    "mtlRaw->info.sortKey doesn't index 1 << MTL_SORT_PRIMARY_SORT_KEY_BITS\n\t%i not in [0, %i)",
                    mtlRaw->info.sortKey,
                    64) )
    {
        __debugbreak();
    }
    textureTable = (MaterialTextureDefRaw *)((char *)mtlRaw + mtlRaw->textureTableOffset);
    for ( textureIndex = 0; textureIndex < mtlRaw->textureCount; ++textureIndex )
    {
        if ( !Material_FinishLoadingTexdef(mtlRaw, &textureTable[textureIndex], materialType, imageTrack) )
            return 0;
    }
    mtlLoadGlob.sortMtlRaw = mtlRaw;
    qsort(
        textureTable,
        mtlRaw->textureCount,
        0xCu,
        (int (__cdecl *)(const void *, const void *))CompareRawMaterialTextures);
    mtlLoadGlob.sortMtlRaw = 0;
    constantTable = (MaterialConstantDefRaw *)((char *)mtlRaw + mtlRaw->constantTableOffset);
    for ( constantIndex = 0; constantIndex < mtlRaw->constantCount; ++constantIndex )
    {
        if ( !constantTable[constantIndex].nameOffset )
            return 0;
    }
    mtlLoadGlob.sortMtlRaw = mtlRaw;
    qsort(
        constantTable,
        mtlRaw->constantCount,
        0x14u,
        (int (__cdecl *)(const void *, const void *))CompareRawMaterialTextures);
    mtlLoadGlob.sortMtlRaw = 0;
    Com_sprintf(
        techniqueSetName,
        0x100u,
        "%s%s",
        techniqueSetVertDeclPrefix,
        (const char *)mtlRaw + mtlRaw->techSetNameOffset);
    *techniqueSet = Material_RegisterTechniqueSet(techniqueSetName);
    return *techniqueSet != 0;
}

bool __cdecl Material_FinishLoadingTexdef(
                const MaterialRaw *material,
                MaterialTextureDefRaw *texdef,
                unsigned int materialType,
                int imageTrack)
{
    if ( !texdef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp", 8664, 0, "%s", "texdef") )
    {
        __debugbreak();
    }
    if ( material->info.sortKey == 4
        && R_IsWorldMaterialType(materialType)
        && (texdef->samplerState & 0x18) == 8
        && (texdef->semantic == 2 || texdef->semantic == 5 || texdef->semantic == 8) )
    {
        texdef->samplerState &= 0xE7u;
        texdef->samplerState |= 0x10u;
    }
    if ( texdef->semantic == 11 )
        return Material_RegisterWaterImage((const MaterialWaterDef *)((char *)material + texdef->u.imageNameOffset)) != 0;
    if ( texdef->semantic == 28 )
        return 1;
    return Material_RegisterImage(material, texdef->u.imageNameOffset, texdef->semantic, imageTrack);
}

bool __cdecl Material_RegisterImage(
                const MaterialRaw *material,
                int imageNameOffset,
                unsigned __int8 semantic,
                int imageTrack)
{
    return Image_Register((char *)material + imageNameOffset, semantic, imageTrack) != 0;
}

bool __cdecl R_IsWorldMaterialType(unsigned int materialType)
{
    return materialType == 3 || materialType == 4;
}

int __cdecl CompareRawMaterialTextures(unsigned int *e0, unsigned int *e1)
{
    unsigned int v2; // esi
    const char *name_4; // [esp+8h] [ebp-4h]

    name_4 = (char *)mtlLoadGlob.sortMtlRaw + *e1;
    v2 = R_HashString((const char *)mtlLoadGlob.sortMtlRaw + *e0);
    return v2 < R_HashString(name_4) ? -1 : 1;
}

void __cdecl Material_PreLoadAllShaderText()
{
    int fileCountLib3; // [esp+160h] [ebp-20h] BYREF
    const char **shaderListLib1; // [esp+164h] [ebp-1Ch]
    int fileCountLib2; // [esp+168h] [ebp-18h] BYREF
    int cachedIndex; // [esp+16Ch] [ebp-14h]
    int fileIndex; // [esp+170h] [ebp-10h]
    const char **shaderListLib3; // [esp+174h] [ebp-Ch]
    const char **shaderListLib2; // [esp+178h] [ebp-8h]
    int fileCountLib1; // [esp+17Ch] [ebp-4h] BYREF

    shaderListLib1 = FS_ListFilesInLocation("shaders/lib/", (char*)"hlsl", FS_LIST_PURE_ONLY, &fileCountLib1, 8);
    shaderListLib2 = FS_ListFilesInLocation("shaders/", (char *)"hlsl", FS_LIST_PURE_ONLY, &fileCountLib2, 8);
    shaderListLib3 = FS_ListFilesInLocation("shaders/", (char *)"fx", FS_LIST_PURE_ONLY, &fileCountLib3, 8);
    mtlLoadGlob.cachedShaderCount = fileCountLib3 + fileCountLib2 + fileCountLib1;
    mtlLoadGlob.cachedShaderText = (GfxCachedShaderText *)Hunk_Alloc(
                                                                                                                    12 * (fileCountLib3 + fileCountLib2 + fileCountLib1),
                                                                                                                    "Material_PreLoadShaderText",
                                                                                                                    23);
    cachedIndex = 0;
    for ( fileIndex = 0; fileIndex < fileCountLib1; ++fileIndex )
        Material_PreLoadSingleShaderText(shaderListLib1[fileIndex], "lib/", &mtlLoadGlob.cachedShaderText[cachedIndex++]);
    for ( fileIndex = 0; fileIndex < fileCountLib2; ++fileIndex )
        Material_PreLoadSingleShaderText(
            shaderListLib2[fileIndex],
            "",
            &mtlLoadGlob.cachedShaderText[cachedIndex++]);
    for ( fileIndex = 0; fileIndex < fileCountLib3; ++fileIndex )
        Material_PreLoadSingleShaderText(
            shaderListLib3[fileIndex],
            "",
            &mtlLoadGlob.cachedShaderText[cachedIndex++]);
    //std::_Sort<GfxCachedShaderText *,int,bool (__cdecl *)(GfxCachedShaderText const &,GfxCachedShaderText const &)>(
    //    mtlLoadGlob.cachedShaderText,
    //    &mtlLoadGlob.cachedShaderText[mtlLoadGlob.cachedShaderCount],
    //    (signed int)(12 * mtlLoadGlob.cachedShaderCount) / 12,
    //    Material_CachedShaderTextLess);

    std::sort(mtlLoadGlob.cachedShaderText, mtlLoadGlob.cachedShaderText + mtlLoadGlob.cachedShaderCount, Material_CachedShaderTextLess);

    FS_FreeFileList(shaderListLib1);
    FS_FreeFileList(shaderListLib2);
    FS_FreeFileList(shaderListLib3);
}

void __cdecl Material_PreLoadSingleShaderText(const char *filename, const char *subdir, GfxCachedShaderText *cached)
{
    char *buffer; // [esp+20h] [ebp-58h]
    char *buffera; // [esp+20h] [ebp-58h]
    int fileHandle; // [esp+28h] [ebp-50h] BYREF
    int fileSize; // [esp+2Ch] [ebp-4Ch]
    char filepath[68]; // [esp+30h] [ebp-48h] BYREF

    Com_sprintf(filepath, 0x40u, "shaders/%s%s", subdir, filename);
    fileSize = FS_FOpenFileReadCurrentThread(filepath, &fileHandle);
    if ( fileSize < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10587,
                    0,
                    "%s",
                    "fileSize >= 0") )
    {
        __debugbreak();
    }
    buffer = (char *)Hunk_AllocAlign(
                                         strlen(subdir) + fileSize + strlen(filename) + 2,
                                         1,
                                         "Material_PreLoadSingleShaderText",
                                         23);
    cached->name = buffer;
    buffera = &buffer[sprintf(buffer, "%s%s", subdir, filename) + 1];
    cached->text = buffera;
    FS_Read((unsigned __int8 *)buffera, fileSize, fileHandle);
    FS_FCloseFile(fileHandle);
    buffera[fileSize] = 0;
    cached->textSize = fileSize;
}

void __cdecl Material_GetInfo(Material *handle, MaterialInfo *matInfo)
{
    if ( !handle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10662,
                    0,
                    "%s",
                    "handle") )
    {
        __debugbreak();
    }
    if ( !matInfo
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10663,
                    0,
                    "%s",
                    "matInfo") )
    {
        __debugbreak();
    }
    memcpy(matInfo, Material_FromHandle(handle), sizeof(MaterialInfo));
}

bool __cdecl Material_HasImpactMaskTech(const Material *material)
{
    return Material_GetTechniqueSet(material)->techniques[129] != 0;
}

bool __cdecl Material_IsForSeeThruBulletHoleDecal(const Material *mat)
{
    return Material_HasImpactMaskTech(mat) || Material_IsSeeThruParallaxDecal(mat);
}

bool __cdecl Material_IsSeeThruParallaxDecal(const Material *mat)
{
    return (mat->info.gameFlags & 0x100) != 0;
}

void __cdecl Material_Sort()
{
    PROF_SCOPED("Material_Sort"); // LWSS ADD

    if ( useFastFile->current.enabled )
        rgp.materialCount = DB_GetAllXAssetOfType(ASSET_TYPE_MATERIAL, (XAssetHeader *)rgp.sortedMaterials, 4096);
    Material_SortInternal(rgp.sortedMaterials, rgp.materialCount);
}

void __cdecl Material_SortInternal(Material **sortedMaterials, unsigned int materialCount)
{
    unsigned __int64 v2; // rax
    int v3; // ecx
    unsigned __int64 v4; // rax
    unsigned int v5; // ecx
    unsigned __int64 v6; // rax
    int v7; // ecx
    unsigned __int64 v8; // rax
    unsigned int v9; // ecx
    int techIdx; // [esp+98h] [ebp-10h]
    unsigned int sortedIndex; // [esp+9Ch] [ebp-Ch]
    Material *material; // [esp+A4h] [ebp-4h]

    //std::_Sort<Material * *,int,bool (__cdecl *)(Material const *,Material const *)>(
    //    (const GfxStaticModelDrawInst **)sortedMaterials,
    //    (const GfxStaticModelDrawInst **)&sortedMaterials[materialCount],
    //    (int)(4 * materialCount) >> 2,
    //    (bool (__cdecl *)(const GfxStaticModelDrawInst *, const GfxStaticModelDrawInst *))Material_Compare);

    std::sort(sortedMaterials, sortedMaterials + materialCount, Material_Compare);

    for ( sortedIndex = 0; sortedIndex < materialCount; ++sortedIndex )
    {
        material = sortedMaterials[sortedIndex];
        *(unsigned int *)&material->info.drawSurf.fields = 0;
        HIDWORD(material->info.drawSurf.packed) = 0;
        v2 = (unsigned __int64)(unsigned __int8)R_DrawSurfPrimarySortKey(material) << 58;
        v3 = HIDWORD(v2) | HIDWORD(material->info.drawSurf.packed) & 0x3FFFFFF;
        *(unsigned int *)&material->info.drawSurf.fields |= v2;
        HIDWORD(material->info.drawSurf.packed) = v3;
        v4 = (unsigned __int64)(R_DrawSurfStandardPrepassSortKey(material) & 3) << 55;
        v5 = HIDWORD(v4) | HIDWORD(material->info.drawSurf.packed) & 0xFE7FFFFF;
        *(unsigned int *)&material->info.drawSurf.fields |= v4;
        HIDWORD(material->info.drawSurf.packed) = v5;
        v6 = (unsigned __int64)((material->info.gameFlags & 0x40) != 0) << 20;
        v7 = HIDWORD(v6) | HIDWORD(material->info.drawSurf.packed);
        *(unsigned int *)&material->info.drawSurf.fields = v6 | *(unsigned int *)&material->info.drawSurf.fields & 0xFE0FFFFF;
        HIDWORD(material->info.drawSurf.packed) = v7;
        v8 = (unsigned __int64)(sortedIndex & 0xFFF) << 31;
        v9 = HIDWORD(v8) | HIDWORD(material->info.drawSurf.packed) & 0xFFFFF800;
        *(unsigned int *)&material->info.drawSurf.fields = v8 | *(unsigned int *)&material->info.drawSurf.fields & 0x7FFFFFFF;
        HIDWORD(material->info.drawSurf.packed) = v9;
        if ( material->localTechniqueSet )
        {
            material->localTechniqueSet->techsetFlags = 0;
            for ( techIdx = 80; techIdx < 115; ++techIdx )
            {
                if ( material->localTechniqueSet->techniques[techIdx] )
                {
                    material->localTechniqueSet->techsetFlags |= 1u;
                    break;
                }
            }
        }
    }
}

unsigned int __cdecl R_DrawSurfStandardPrepassSortKey(const Material *material)
{
    MaterialTechniqueSet *techSet; // [esp+0h] [ebp-8h]
    const MaterialTechnique *prepassTech; // [esp+4h] [ebp-4h]

    techSet = material->localTechniqueSet;
    if ( !techSet
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10691,
                    0,
                    "%s",
                    "techSet") )
    {
        __debugbreak();
    }
    prepassTech = techSet->techniques[0];
    if ( prepassTech )
    {
        if ( (material->stateFlags & 4) == 0 || Material_IsForSeeThruBulletHoleDecal(material) )
            return (prepassTech->flags & 4) == 0;
        else
            return 2;
    }
    else if ( techSet->techniques[1] )
    {
        return 3;
    }
    else
    {
        return 2;
    }
}

bool __cdecl Material_Compare(const Material *mtl0, const Material *mtl1)
{
    const char *v3; // eax
    const char *v4; // eax
    int writesDepth; // [esp+98h] [ebp-148h]
    int writesDepth_4; // [esp+9Ch] [ebp-144h]
    const MaterialTechnique *techniqueEmissive; // [esp+A0h] [ebp-140h]
    const MaterialTechnique *techniqueEmissive_4; // [esp+A4h] [ebp-13Ch]
    const MaterialTechnique *techniqueLit; // [esp+A8h] [ebp-138h]
    const MaterialTechnique *techniqueLit_4; // [esp+ACh] [ebp-134h]
    char name0[128]; // [esp+B0h] [ebp-130h] BYREF
    int hasTechniqueEmissive[2]; // [esp+130h] [ebp-B0h]
    int hasTechniqueLit[2]; // [esp+138h] [ebp-A8h]
    int prepass[2]; // [esp+140h] [ebp-A0h]
    char name1[128]; // [esp+148h] [ebp-98h] BYREF
    const MaterialTechniqueSet *techSet[2]; // [esp+1CCh] [ebp-14h]
    int comparison; // [esp+1D4h] [ebp-Ch]
    int hasLightmap[2]; // [esp+1D8h] [ebp-8h]

    if ( !mtl0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp", 10849, 0, "%s", "mtl0") )
    {
        __debugbreak();
    }
    if ( !mtl1
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp", 10850, 0, "%s", "mtl1") )
    {
        __debugbreak();
    }
    techSet[0] = Material_GetTechniqueSet(mtl0);
    techSet[1] = Material_GetTechniqueSet(mtl1);
    if ( (!techSet[0] || !techSet[1])
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10866,
                    0,
                    "%s",
                    "techSet[0] && techSet[1]") )
    {
        __debugbreak();
    }
    techniqueLit = Material_GetTechnique(mtl0, 0xAu);
    techniqueLit_4 = Material_GetTechnique(mtl1, 0xAu);
    hasTechniqueLit[0] = techniqueLit != 0;
    hasTechniqueLit[1] = techniqueLit_4 != 0;
    comparison = hasTechniqueLit[1] - hasTechniqueLit[0];
    if ( hasTechniqueLit[1] != hasTechniqueLit[0] )
        return comparison < 0;
    hasLightmap[0] = (mtl0->info.gameFlags & 2) != 0;
    hasLightmap[1] = (mtl1->info.gameFlags & 2) != 0;
    techniqueEmissive = Material_GetTechnique(mtl0, 5u);
    techniqueEmissive_4 = Material_GetTechnique(mtl1, 5u);
    hasTechniqueEmissive[0] = techniqueEmissive != 0;
    hasTechniqueEmissive[1] = techniqueEmissive_4 != 0;
    if ( hasTechniqueLit[0] )
    {
        if ( hasTechniqueEmissive[0]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                        10889,
                        0,
                        "%s",
                        "!hasTechniqueEmissive[0]") )
        {
            __debugbreak();
        }
        if ( hasTechniqueEmissive[1]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                        10890,
                        0,
                        "%s",
                        "!hasTechniqueEmissive[1]") )
        {
            __debugbreak();
        }
        comparison = mtl0->info.sortKey - mtl1->info.sortKey;
        if ( comparison )
            return comparison < 0;
        comparison = hasLightmap[1] - hasLightmap[0];
        if ( hasLightmap[1] != hasLightmap[0] )
            return comparison < 0;
    }
    else
    {
        if ( hasLightmap[0] )
        {
            v3 = va("material '%s' no techlit, but hasLightmap", mtl0->info.name);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                            10903,
                            0,
                            "%s\n\t%s",
                            "!hasLightmap[0]",
                            v3) )
                __debugbreak();
        }
        if ( hasLightmap[1] )
        {
            v4 = va("material '%s' no techlit, but hasLightmap", mtl1->info.name);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                            10904,
                            0,
                            "%s\n\t%s",
                            "!hasLightmap[1]",
                            v4) )
                __debugbreak();
        }
        comparison = hasTechniqueEmissive[1] - hasTechniqueEmissive[0];
        if ( hasTechniqueEmissive[1] != hasTechniqueEmissive[0] )
            return comparison < 0;
        comparison = mtl0->info.sortKey - mtl1->info.sortKey;
        if ( comparison )
            return comparison < 0;
    }
    prepass[0] = R_DrawSurfStandardPrepassSortKey(mtl0);
    prepass[1] = R_DrawSurfStandardPrepassSortKey(mtl1);
    comparison = prepass[0] - prepass[1];
    if ( prepass[0] != prepass[1] )
        return comparison < 0;
    writesDepth = (mtl0->stateFlags & 8) != 0;
    writesDepth_4 = (mtl1->stateFlags & 8) != 0;
    comparison = writesDepth_4 - writesDepth;
    if ( writesDepth_4 != writesDepth )
        return comparison < 0;
    if ( hasTechniqueLit[0] )
    {
        comparison = strcmp(techniqueLit->passArray[0].pixelShader->name, techniqueLit_4->passArray[0].pixelShader->name);
        if ( comparison )
            return comparison < 0;
        if ( writesDepth )
        {
            comparison = Material_ComparePixelConsts(mtl0, mtl1, 0xAu);
            if ( comparison )
                return comparison < 0;
        }
        Material_GetVertexShaderName(name0, techniqueLit->passArray, 128);
        Material_GetVertexShaderName(name1, techniqueLit_4->passArray, 128);
        comparison = strcmp(name0, name1);
        if ( comparison )
            return comparison < 0;
    }
    else if ( hasTechniqueEmissive[0] )
    {
        comparison = strcmp(
                                     techniqueEmissive->passArray[0].pixelShader->name,
                                     techniqueEmissive_4->passArray[0].pixelShader->name);
        if ( comparison )
            return comparison < 0;
        comparison = Material_ComparePixelConsts(mtl0, mtl1, 5u);
        if ( comparison )
            return comparison < 0;
        Material_GetVertexShaderName(name0, techniqueEmissive->passArray, 128);
        Material_GetVertexShaderName(name1, techniqueEmissive_4->passArray, 128);
        comparison = strcmp(name0, name1);
        if ( comparison )
            return comparison < 0;
    }
    comparison = strcmp(techSet[0]->name, techSet[1]->name);
    if ( comparison )
        return comparison < 0;
    if ( mtl0 == mtl1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10976,
                    0,
                    "%s",
                    "mtl0 != mtl1") )
    {
        __debugbreak();
    }
    comparison = strcmp(mtl0->info.name, mtl1->info.name);
    if ( !comparison
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10978,
                    0,
                    "%s",
                    "comparison") )
    {
        __debugbreak();
    }
    return comparison < 0;
}

void __cdecl Material_GetVertexShaderName(char *dest, const MaterialPass *pass, int destsize)
{
    if ( !pass->vertexShader
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10774,
                    0,
                    "%s",
                    "pass->vertexShader") )
    {
        __debugbreak();
    }
    I_strncpyz(dest, pass->vertexShader->name, destsize);
}

int __cdecl Material_ComparePixelConsts(const Material *mtl0, const Material *mtl1, unsigned __int8 techType)
{
    const MaterialPass *pass[2]; // [esp+10h] [ebp-18h] BYREF
    const MaterialTechnique *techniqueLit[2]; // [esp+18h] [ebp-10h]
    const Material *mtl[2]; // [esp+20h] [ebp-8h] BYREF

    techniqueLit[0] = Material_GetTechnique(mtl0, techType);
    techniqueLit[1] = Material_GetTechnique(mtl1, techType);
    if ( !techniqueLit[0]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10816,
                    0,
                    "%s",
                    "techniqueLit[0]") )
    {
        __debugbreak();
    }
    if ( !techniqueLit[1]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10817,
                    0,
                    "%s",
                    "techniqueLit[1]") )
    {
        __debugbreak();
    }
    mtl[0] = mtl0;
    mtl[1] = mtl1;
    pass[0] = techniqueLit[0]->passArray;
    pass[1] = techniqueLit[1]->passArray;
    return R_ComparePixelConsts(mtl, pass);
}

unsigned int __cdecl R_DrawSurfPrimarySortKey(const Material *material)
{
    if ( material->info.sortKey >= 0x40u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_load_obj.cpp",
                    10986,
                    0,
                    "material->info.sortKey doesn't index 1 << MTL_SORT_PRIMARY_SORT_KEY_BITS\n\t%i not in [0, %i)",
                    material->info.sortKey,
                    64) )
    {
        __debugbreak();
    }
    return material->info.sortKey;
}

bool g_scriptableConstantArrayBuilt;
char __cdecl R_FindScriptableConstantSource_ByName(const char *name, unsigned __int8 *dest)
{
    unsigned int i; // [esp+4h] [ebp-4h]

    if ( !g_scriptableConstantArrayBuilt )
    {
        R_BuildScritableConstantArray(g_scriptableConstantArray, 13);
        g_scriptableConstantArrayBuilt = 1;
    }
    for ( i = 0; i < 0xD; ++i )
    {
        if ( !I_stricmp(name, g_scriptableConstantArray[i].sourceName) )
        {
            *dest = g_scriptableConstantArray[i].source;
            return 1;
        }
    }
    return 0;
}

void __cdecl R_BuildScritableConstantArray(ScriptableConstant *array, int arraySize)
{
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < arraySize; ++i )
        array[i].sourceName = R_FindCodeConstantName_BySource(array[i].source);
}

const char *__cdecl R_FindCodeConstantName_BySource(unsigned __int8 src)
{
    const CodeConstantSource *source; // [esp+0h] [ebp-4h]

    for ( source = s_codeConsts; source && source->name; ++source )
    {
        if ( source->source == src )
            return source->name;
    }
    return 0;
}

const char *__cdecl R_FindScriptableConstantName_BySource(unsigned __int8 src)
{
    unsigned int i; // [esp+4h] [ebp-4h]

    if ( !g_scriptableConstantArrayBuilt )
    {
        R_BuildScritableConstantArray(g_scriptableConstantArray, 13);
        g_scriptableConstantArrayBuilt = 1;
    }
    for ( i = 0; i < 0xD; ++i )
    {
        if ( g_scriptableConstantArray[i].source == src )
            return g_scriptableConstantArray[i].sourceName;
    }
    return 0;
}

