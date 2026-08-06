#include "r_dvars.h"
#include "r_sky.h"

#include <cstddef>
#include <qcommon/threads.h>
#include "r_reflection_probe.h"
#include "r_warn.h"
#include <universal/q_shared.h>
#include "r_init.h"

const dvar_s *r_ignore;
const dvar_s *r_clipCodec;
const dvar_s *r_clipSize;
const dvar_s *r_clipFPS;
const dvar_s *vid_xpos;
const dvar_s *vid_ypos;
const dvar_s *r_fullscreen;
const dvar_s *r_gamma;
const dvar_s *r_ignorehwgamma;
const dvar_s *r_texFilterAnisoMax;
const dvar_s *r_texFilterDisable;
const dvar_s *r_texFilterAnisoMin;
const dvar_s *r_texFilterMipMode;
const dvar_s *r_texFilterMipBias;
const dvar_s *r_fullbright;
const dvar_s *r_debugShader;
const dvar_s *r_lightConflicts;
const dvar_s *r_gpuSync;
const dvar_s *r_multiGpu;
const dvar_s *r_skinCache;
const dvar_s *r_fastSkin;
const dvar_s *r_smc_enable;
const dvar_s *r_pretess;
const dvar_s *r_lodScaleRigid;
const dvar_s *r_lodBiasRigid;
const dvar_s *r_lodScaleSkinned;
const dvar_s *r_lodBiasSkinned;
const dvar_s *r_fovScaleThresholdSkinned;
const dvar_s *r_fovScaleThresholdRigid;
const dvar_s *r_worldLod;
const dvar_s *r_znear;
const dvar_s *r_znear_depthhack;
const dvar_s *r_zfar;
const dvar_s *r_fog;
const dvar_s *r_polygonOffsetScale;
const dvar_s *r_polygonOffsetBias;
const dvar_s *r_picmip_manual;
const dvar_s *r_picmip;
const dvar_s *r_picmip_bump;
const dvar_s *r_picmip_spec;
const dvar_s *r_picmip_water;
const dvar_s *r_envMapSpecular;
const dvar_s *r_lightMap;
const dvar_s *r_colorMap;
const dvar_s *r_normalMap;
const dvar_s *r_specularRoughnessMap;
const dvar_s *r_specularColorScale;
const dvar_s *r_diffuseColorScale;
const dvar_s *r_useLayeredMaterials;
const dvar_s *r_loadForRenderer;
const dvar_s *r_showLodInfo;
const dvar_s *r_showTris;
const dvar_s *r_showTriCounts;
const dvar_s *r_showSurfCounts;
const dvar_s *r_showVertCounts;
const dvar_s *r_resampleScene;
const dvar_s *r_showPenetration;
const dvar_s *r_showPixelCost;
const dvar_s *r_xdebug;
const dvar_s *r_debugLineWidth;
const dvar_s *r_vc_makelog;
const dvar_s *r_vc_showlog;
const dvar_s *r_showLightGrid;
const dvar_s *r_showLightingOrigins;
const dvar_s *r_showMissingLightGrid;
const dvar_s *r_cacheSModelLighting;
const dvar_s *r_cacheModelLighting;
const dvar_s *r_lightTweakAmbient;
const dvar_s *r_lightTweakDiffuseFraction;
const dvar_s *r_lightTweakSunLight;
const dvar_s *r_lightTweakAmbientColor;
const dvar_s *r_lightTweakSunColor;
const dvar_s *r_lightTweakSunDiffuseColor;
const dvar_s *r_lightTweakSunDirection;
const dvar_s *r_ui3d_h;
const dvar_s *r_bloomStreakYTintControl;
const dvar_s *r_filmTweakLightTint;
const dvar_s *r_envMapOverride;
const dvar_s *r_envMapMinIntensity;
const dvar_s *r_envMapMaxIntensity;
const dvar_s *r_envMapExponent;
const dvar_s *r_envMapSunIntensity;
const dvar_s *r_showSunDirectionDebug;
const dvar_s *r_showOutdoorVolumeDebug;
const dvar_s *r_materialXYZ;
const dvar_s *r_collectActiveMaterials;
const dvar_s *r_drawPrimHistogram;
const dvar_s *r_logFile;
const dvar_s *r_norefresh;
const dvar_s *r_scaleViewport;
const dvar_s *r_smp_backend;
const dvar_s *r_smp_worker;
const dvar_s *r_aaAlpha;
const dvar_s *r_aaSamples;
const dvar_s *r_vsync;
const dvar_s *r_clear;
const dvar_s *r_clearColor;
const dvar_s *r_clearColor2;
const dvar_s *r_setFrameBufferAlpha;
const dvar_s *r_drawSun;
const dvar_s *r_drawWorld;
const dvar_s *r_drawEntities;
const dvar_s *r_drawPoly;
const dvar_s *r_drawPrimFloor;
const dvar_s *r_skipDrawTris;
const dvar_s *r_drawWater;
const dvar_s *r_lockPvs;
const dvar_s *r_skipPvs;
const dvar_s *r_portalBevels;
const dvar_s *r_portalBevelsOnly;
const dvar_s *r_singleCell;
const dvar_s *r_showForcedInvisibleCells;
const dvar_s *r_portalWalkLimit;
const dvar_s *r_portalMinClipArea;
const dvar_s *r_portalMinRecurseDepth;
const dvar_s *r_enableOccluders;
const dvar_s *r_showPortals;
const dvar_s *r_showOccluders;
const dvar_s *r_showAabbTrees;
const dvar_s *r_showSModelNames;
const dvar_s *r_showDynEntModelNames;
const dvar_s *r_showDObjModelNames;
const dvar_s *r_showEntModelNames;
const dvar_s *r_showTess;
const dvar_s *r_showCullBModels;
const dvar_s *r_showCullSModels;
const dvar_s *r_showCullXModels;
const dvar_s *r_showFbColorDebug;
const dvar_s *r_showFloatZDebug;
const dvar_s *r_showCollision;
const dvar_s *r_showCollisionGroups;
const dvar_s *r_showCollisionPolyType;
const dvar_s *r_showCollisionDepthTest;
const dvar_s *r_showCollisionDist;
const dvar_s *r_floatz;
const dvar_s *r_depthPrepass;
const dvar_s *r_highLodDist;
const dvar_s *r_mediumLodDist;
const dvar_s *r_lowLodDist;
const dvar_s *r_lowestLodDist;
const dvar_s *r_forceLod;
const dvar_s *r_modelVertColor;
const dvar_s *sm_enable;
const dvar_s *sm_sunEnable;
const dvar_s *sm_spotEnable;
const dvar_s *sm_maxLights;
const dvar_s *sm_spotShadowFadeTime;
const dvar_s *sm_lightScore_eyeProjectDist;
const dvar_s *sm_lightScore_spotProjectFrac;
const dvar_s *sm_showOverlay;
const dvar_s *sm_showOverlayDepthBounds;
const dvar_s *sm_showSpotAxis;
const dvar_s *sm_sunAlwaysCastsShadow;
const dvar_s *sm_polygonOffsetScale;
const dvar_s *sm_polygonOffsetBias;
const dvar_s *sm_sunSampleSizeNear;
const dvar_s *sm_sunShadowCenter;
const dvar_s *sm_sunShadowScale;
const dvar_s *sm_sunShadowSmall;
const dvar_s *sm_sunShadowSmallEnable;
const dvar_s *r_backBufferSize;
const dvar_s *r_backBufferSizeY;
const dvar_s *sm_spotShadowLargeRadiusScale;
const dvar_s *sm_strictCull;
const dvar_s *sm_fastSunShadow;
const dvar_s *sm_qualitySpotShadow;
const dvar_s *sm_fullResSpotShadowEnable;
const dvar_s *sm_debugFastSunShadow;
const dvar_s *r_stream;
const dvar_s *r_streamCheckAabb;
const dvar_s *r_streamClear;
const dvar_s *r_streamDebug;
const dvar_s *r_streamFull;
const dvar_s *r_streamFakeLagMsec;
const dvar_s *r_streamMaxDist;
const dvar_s *r_streamShowStats;
const dvar_s *r_streamShowList;
const dvar_s *r_streamSize;
const dvar_s *r_streamTaint;
const dvar_s *r_streamFreezeState;
const dvar_s *r_streamLog;
const dvar_s *r_streamHiddenPush;
const dvar_s *r_streamLowDetail;
const dvar_s *r_stereoTurretShift;
const dvar_s *r_blur_allowed;
const dvar_s *r_blur;
const dvar_s *r_distortion;
const dvar_s *r_flame_allowed;
const dvar_s *r_filmLut;
const dvar_s *r_filmTweakLut;
const dvar_s *r_enablePlayerShadow;
const dvar_s *r_enableFlashlight;
const dvar_s *r_waterFogTest;
const dvar_s *r_contrast;
const dvar_s *r_brightness;
const dvar_s *r_desaturation;
const dvar_s *r_filmTweakBrightness;
const dvar_s *r_filmTweakDesaturation;
const dvar_s *r_filmTweakInvert;
const dvar_s *r_filmUseTweaks;
const dvar_s *r_filmTweakEnable;
const dvar_s *r_filmTweakHue;
const dvar_s *r_filmTweakSaturation;
const dvar_s *r_filmTweakColorTemp;
const dvar_s *r_filmTweakDarkTint;
const dvar_s *r_filmTweakMidTint;
const dvar_s *r_filmTweakMidStart;
const dvar_s *r_filmTweakMidEnd;
const dvar_s *r_filmTweakDarkFeather;
const dvar_s *r_filmTweakLightFeather;
const dvar_s *r_filmTweakContrast;
const dvar_s *r_filmTweakBleach;
const dvar_s *r_filmTweakRangeDebug;
const dvar_s *r_sCurveShoulderStrength;
const dvar_s *r_sCurveLinearStrength;
const dvar_s *r_sCurveLinearAngle;
const dvar_s *r_sCurveToeStrength;
const dvar_s *r_sCurveToeNumerator;
const dvar_s *r_sCurveToeDenominator;
const dvar_s *r_sCurveEnable;
const dvar_s *r_flimCurveBlack;
const dvar_s *r_flimCurveWhite;
const dvar_s *r_filmCurveSoftClip;
const dvar_s *r_waterWaveAngle;
const dvar_s *r_waterWaveWavelength;
const dvar_s *r_waterWaveAmplitude;
const dvar_s *r_waterWavePhase;
const dvar_s *r_waterWaveSteepness;
const dvar_s *r_waterWaveSpeed;
const dvar_s *r_dof_enable;
const dvar_s *r_dof_tweak;
const dvar_s *r_dof_nearBlur;
const dvar_s *r_dof_farBlur;
const dvar_s *r_dof_viewModelStart;
const dvar_s *r_dof_viewModelEnd;
const dvar_s *r_dof_nearStart;
const dvar_s *r_dof_nearEnd;
const dvar_s *r_dof_farStart;
const dvar_s *r_dof_farEnd;
const dvar_s *r_dof_bias;
const dvar_s *r_dof_showdebug;
const dvar_s *r_motionblur_enable;
const dvar_s *r_motionblur_frameBased_enable;
const dvar_s *r_motionblur_numberOfSamples;
const dvar_s *r_motionblur_blurOrigin;
const dvar_s *r_motionblur_directionFactor;
const dvar_s *r_motionblur_positionFactor;
const dvar_s *r_motionblur_maxblur;
const dvar_s *r_flameFX_enable;
const dvar_s *r_flameFX_distortionScaleFactor;
const dvar_s *r_flameFX_magnitude;
const dvar_s *r_flameFX_FPS;
const dvar_s *r_flameFX_fadeDuration;
const dvar_s *r_waterSheetingFX_allowed;
const dvar_s *r_waterSheetingFX_enable;
const dvar_s *r_drawDynEnts;
const dvar_s *r_drawBModels;
const dvar_s *r_drawSModels;
const dvar_s *r_drawXModels;
const dvar_s *r_drawSkinnedModels;
const dvar_s *r_dlightLimit;
const dvar_s *r_dlightMaxFullScreenRadius;
const dvar_s *r_dlightMaxNonFullScreenRadius;
const dvar_s *r_dobjLimit;
const dvar_s *r_modelLimit;
const dvar_s *r_brushLimit;
const dvar_s *r_spotLightFovInnerFraction;
const dvar_s *r_spotLightStartRadius;
const dvar_s *r_spotLightEndRadius;
const dvar_s *r_spotLightShadows;
const dvar_s *r_spotLightSModelShadows;
const dvar_s *r_spotLightEntityShadows;
const dvar_s *r_spotLightBrightness;
const dvar_s *r_flashLightFovInnerFraction;
const dvar_s *r_flashLightStartRadius;
const dvar_s *r_flashLightEndRadius;
const dvar_s *r_flashLightShadows;
const dvar_s *r_flashLightBrightness;
const dvar_s *r_flashLightOffset;
const dvar_s *r_flashLightRange;
const dvar_s *r_flashLightColor;
const dvar_s *r_flashLightBobAmount;
const dvar_s *r_flashLightBobRate;
const dvar_s *r_flashLightSpecularScale;
const dvar_s *r_flashLightFlickerAmount;
const dvar_s *r_flashLightFlickerRate;
const dvar_s *r_drawPrimCap;
const dvar_s *r_waterSheetingFX_distortionScaleFactor;
const dvar_s *r_waterSheetingFX_magnitude;
const dvar_s *r_waterSheetingFX_radius;
const dvar_s *r_waterSheetingFX_fadeDuration;
const dvar_s *r_reviveFX_debug;
const dvar_s *r_reviveFX_fadeDuration;
const dvar_s *r_reviveFX_edgeColorTemp;
const dvar_s *r_reviveFX_edgeSaturation;
const dvar_s *r_reviveFX_edgeScale;
const dvar_s *r_reviveFX_edgeContrast;
const dvar_s *r_reviveFX_edgeOffset;
const dvar_s *r_reviveFX_edgeAmount;
const dvar_s *r_reviveFX_edgeMaskAdjust;
const dvar_s *r_genericFilter_enable;
const dvar_s *r_superFlare_enable;
const dvar_s *r_superFlare_debug;
const dvar_s *r_poisonFX_debug_enable;
const dvar_s *r_poisonFX_debug_amount;
const dvar_s *r_poisonFX_pulse;
const dvar_s *r_poisonFX_warpX;
const dvar_s *r_poisonFX_warpY;
const dvar_s *r_poisonFX_dvisionA;
const dvar_s *r_poisonFX_dvisionX;
const dvar_s *r_poisonFX_dvisionY;
const dvar_s *r_poisonFX_blurMin;
const dvar_s *r_poisonFX_blurMax;
const dvar_s *r_showHDRalpha;
const dvar_s *r_fullHDRrendering;
const dvar_s *r_lutvar_0;
const dvar_s *r_lutvar_1;
const dvar_s *r_lutvar_2;
const dvar_s *r_lutvar_3;
const dvar_s *r_lutvar_4;
const dvar_s *r_lutvar_5;
const dvar_s *r_lutvar_6;
const dvar_s *r_lutvar_7;
const dvar_s *r_lutvar_8;
const dvar_s *r_lutvar_9;
const dvar_s *r_lutvar_10;
const dvar_s *r_lutvar_11;
const dvar_s *r_lutvar_12;
const dvar_s *r_lutvar_13;
const dvar_s *r_lutvar_14;
const dvar_s *r_lutvar_15;
const dvar_s *r_exposureTweak;
const dvar_s *r_exposureValue;
const dvar_s *r_bloomTweaks;
const dvar_s *r_bloomBlurRadius;
const dvar_s *r_bloomTintWeights;
const dvar_s *r_bloomColorScale;
const dvar_s *r_bloomTintScale;
const dvar_s *r_bloomCurveBreakpoint;
const dvar_s *r_bloomCurveLoBlack;
const dvar_s *r_bloomCurveLoGamma;
const dvar_s *r_bloomCurveLoWhite;
const dvar_s *r_bloomCurveLoRemapBlack;
const dvar_s *r_bloomCurveLoRemapWhite;
const dvar_s *r_bloomCurveHiBlack;
const dvar_s *r_bloomCurveHiGamma;
const dvar_s *r_bloomCurveHiWhite;
const dvar_s *r_bloomCurveHiRemapBlack;
const dvar_s *r_bloomCurveHiRemapWhite;
const dvar_s *r_bloomExpansionControl;
const dvar_s *r_bloomExpansionWeights;
const dvar_s *r_bloomExpansionSource;
const dvar_s *r_bloomPersistence;
const dvar_s *r_bloomStreakXLevels0;
const dvar_s *r_bloomStreakXLevels1;
const dvar_s *r_bloomStreakXInnerTint;
const dvar_s *r_bloomStreakXOuterTint;
const dvar_s *r_bloomStreakXTintControl;
const dvar_s *r_bloomStreakXTint;
const dvar_s *r_bloomStreakYLevels0;
const dvar_s *r_bloomStreakYLevels1;
const dvar_s *r_bloomStreakYInnerTint;
const dvar_s *r_bloomStreakYOuterTint;
const dvar_s *r_bloomStreakYTint;
const dvar_s *r_finalShiftX;
const dvar_s *r_finalShiftY;
const dvar_s *r_primaryLightUseTweaks;
const dvar_s *r_primaryLightTweakDiffuseStrength;
const dvar_s *r_primaryLightTweakSpecularStrength;
const dvar_s *r_lightGridEnableTweaks;
const dvar_s *r_lightGridUseTweakedValues;
const dvar_s *r_lightGridIntensity;
const dvar_s *r_lightGridContrast;
const dvar_s *r_heroLighting;
const dvar_s *r_heroLightSaturation;
const dvar_s *r_heroLightColorTemp;
const dvar_s *r_heroLightScale;
const dvar_s *r_enableDlights;
const dvar_s *r_debugHDRdlight;
const dvar_s *r_debugHDRdlight_scale;
const dvar_s *r_debugHDRdlight_distance;
const dvar_s *r_debugHDRdlight_radius;
const dvar_s *r_debugHDRdlight_color;
const dvar_s *r_num_viewports;
const dvar_s *r_pix_material;
const dvar_s *r_outdoorAwayBias;
const dvar_s *r_outdoorDownBias;
const dvar_s *r_outdoorFeather;
const dvar_s *r_sun_from_dvars;
const dvar_s *developer;
//const dvar_s *sv_cheats;
const dvar_s *com_statmon;
const dvar_s *r_sse_skinning;
const dvar_s *r_monitor;
const dvar_s *r_aspectRatio;
const dvar_s *r_customMode;
const dvar_s *r_open_automate;
const dvar_s *r_gfxopt_dynamic_foliage;
const dvar_s *r_gfxopt_water_simulation;
const dvar_s *r_allow_intz;
const dvar_s *r_allow_null_rt;
const dvar_s *r_use_driver_convergence;
const dvar_s *r_convergence;
const dvar_s *r_multithreaded_device;
const dvar_s *r_warm_dpvs;
const dvar_s *r_warm_bsp;
const dvar_s *r_warm_static;
const dvar_s *r_warm_dobj;
const dvar_s *r_staticModelDumpLodInfo;
const dvar_s *r_grassBurn;
const dvar_s *r_grassEnable;
const dvar_s *r_skyTransition;
const dvar_s *r_treeScale;
const dvar_s *r_testScale;
const dvar_s *r_skyColorTemp;
const dvar_s *r_debugLayers;
const dvar_s *r_burnedDestructibleColor;
const dvar_s *r_swrk_override_enable;
const dvar_s *r_swrk_override_characterCharredAmount;
const dvar_s *r_swrk_override_characterDissolveColor;
const dvar_s *r_swrk_override_wetness;
const dvar_s *r_debugShowPrimaryLights;
const dvar_s *r_debugShowDynamicLights;
const dvar_s *r_debugShowCoronas;
const dvar_s *r_use_separate_char_tech;
const dvar_s *r_drawDebugFogParams;
const dvar_s *r_debugShowPrimaryLightLines;
const dvar_s *r_seethru_decal_enable;
const dvar_s *r_tension_enable;
const dvar_s *r_MaterialParameterTweak;
const dvar_s *r_ui3d_debug_display;
const dvar_s *r_ui3d_use_debug_values;
const dvar_s *r_ui3d_x;
const dvar_s *r_ui3d_y;
const dvar_s *r_ui3d_w;
const dvar_s *r_missile_cam_debug_display;
const dvar_s *r_extracam_custom_aspectratio;
const dvar_s *r_extracam_shadowmap_enable;
const dvar_s *r_shader_constant_set_debug_range;
const dvar_s *r_shader_constant_set_enable;
const dvar_s *r_sky_intensity_showDebugDisplay;
const dvar_s *r_reflectionProbeGenerate;
const dvar_s *r_ignoreHwGamma;
const dvar_s *r_sky_intensity_useDebugValues;
const dvar_s *r_sky_intensity_angle0;
const dvar_s *r_sky_intensity_angle1;
const dvar_s *r_sky_intensity_factor0;
const dvar_s *r_sky_intensity_factor1;
const dvar_s *r_fog_disable;
const dvar_s *r_grassWindForceEnable;
const dvar_s *r_dpvs_useCellForceInvisibleBits;
const dvar_s *r_remotescreen_renderlastframe;
const dvar_s *r_offscreenCasterLodScale;
const dvar_s *r_dynSModelBurnValueDisplayRange;
const dvar_s *r_useHidePartbits;
const dvar_s *r_smp_worker_threads;
const dvar_s *r_smp_worker_thread[8];

const char *showCollisionGroupsNames[4] =
{ "All", "Brush", "Terrain", NULL };

const char *showCollisionPolyTypeNames[4] =
{ "All", "Wireframe", "Interior", NULL };

const char *r_forceLodNames[6] =
{ "high", "medium", "low", "lowest", "none", NULL };

const char *sm_showOverlayNames[4] =
{ "off", "sun", "spot", NULL };

const char *s_aspectRatioNames[5] =
{ "auto", "standard", "wide 16:10", "wide 16:9", NULL };

const char *showPrimaryLightLinesNames[6] =
{ "off", "static models", "scene models", "dobj", "dyn ent models", NULL };

const char *showCollisionNames[11] =
{
  "None",
  "All",
  "Player",
  "Bullet",
  "Missile",
  "Vehicle",
  "Monster",
  "Item",
  "CanShoot",
  "AINoSight",
  NULL
};

const char *r_clearNames[6] =
{ "never", "dev-only blink", "blink", "steady", "fog color", NULL };

const char *s_aaAlphaNames[4] =
{ "off", "dither (fast)", "supersample (nice)", NULL };

const char *r_showTessNames[7] =
{ "off", "tech", "techset", "material", "vertexshader", "pixelshader", NULL };

const char *fbColorDebugNames[4] =
{ "None", "Screen", "Feedback", NULL };


char *names[8] =
{
  (char*)"r_smp_worker_thread0",
  (char*)"r_smp_worker_thread1",
  (char*)"r_smp_worker_thread2",
  (char*)"r_smp_worker_thread3",
  (char*)"r_smp_worker_thread4",
  (char*)"r_smp_worker_thread5",
  (char*)"r_smp_worker_thread6",
  (char*)"r_smp_worker_thread7"
};

const char *gpuSyncNames[4] =
{ "off", "adaptive", "aggressive", NULL };

const char *debugShaderNames[6] =
{ "none", "normal", "basisTangent", "basisBinormal", "basisNormal", NULL };

const char *codecNames[3] =
{ "MJPEG", "VP8", NULL };

const char *clipSizeNames[5] =
{ "360", "480", "720", "1080", NULL };

const char *mipFilterNames[5] =
{ "Unchanged", "Force Trilinear", "Force Bilinear", "Force Mipmaps Off", NULL };

const char *colorMapNames[5] =
{ "Black", "Unchanged", "White", "Gray", NULL };

const char *normalMapNames[3] =
{ "Flat", "Unchanged", NULL };

const char *specularRoughnessMapNames[6] =
{
  "Unchanged",
  "Matte Smooth",
  "Matte Rough",
  "Shiny Smooth",
  "Shiny Rough",
  NULL
};

const char *lodInfoNames[6] =
{
  "None",
  "All",
  "Current LOD",
  "Current Distance",
  "Auto LOD Distance Only",
  NULL
};

const char *showPenetrationNames[4] =
{ "off", "flash penetrable materials", "flash non-penetrable materials", NULL };

const char *showPixelCostNames[5] =
{ "off", "timing", "use depth", "ignore depth", NULL };



void __cdecl R_RegisterDvars()
{
    int value; // eax
    signed int i; // [esp+20h] [ebp-4h]

    R_RegisterSunDvars();
    r_ignore = _Dvar_RegisterInt("r_ignore", 0, 0x80000000, 0x7FFFFFFF, 0, "used for debugging anything");
    r_clipCodec = _Dvar_RegisterEnum("r_clipCodec", codecNames, 0, 1u, "Codec used when rendering clips.");
    r_clipSize = _Dvar_RegisterEnum("r_clipSize", clipSizeNames, 0, 1u, "Render clip size.");
    r_clipFPS = _Dvar_RegisterInt("r_clipFPS", 24, 10, 60, 1u, "Render clip FPS.");
    vid_xpos = _Dvar_RegisterInt("vid_xpos", 3, -4096, 4096, 1u, "Game window horizontal position");
    vid_ypos = _Dvar_RegisterInt("vid_ypos", 22, -4096, 4096, 1u, "game window vertical position");
    //r_fullscreen = _Dvar_RegisterBool("r_fullscreen", 1, 0x21u, "Display game full screen");
    r_fullscreen = _Dvar_RegisterBool("r_fullscreen", 0, 0x21u, "Display game full screen");
    r_gamma = _Dvar_RegisterFloat("r_gamma", 0.89999998, 0.5, 3.0, 1u, "Gamma value");
    if ( r_reflectionProbeGenerate && r_reflectionProbeGenerate->current.enabled )
        Dvar_SetFloat((dvar_s *)r_gamma, 0.89999998);
    r_ignoreHwGamma = _Dvar_RegisterBool("r_ignorehwgamma", 0, 0x21u, "Ignore hardware gamma");
    r_texFilterAnisoMax = _Dvar_RegisterInt(
                                                    "r_texFilterAnisoMax",
                                                    16,
                                                    1,
                                                    16,
                                                    1u,
                                                    "Maximum anisotropy to use for texture filtering");
    r_texFilterDisable = _Dvar_RegisterBool(
                                                 "r_texFilterDisable",
                                                 0,
                                                 0x80u,
                                                 "Disables all texture filtering (uses nearest only.)");
    r_texFilterAnisoMin = _Dvar_RegisterInt(
                                                    "r_texFilterAnisoMin",
                                                    1,
                                                    1,
                                                    16,
                                                    1u,
                                                    "Minimum anisotropy to use for texture filtering (overridden by max)");
    r_texFilterMipMode = _Dvar_RegisterEnum(
                                                 "r_texFilterMipMode",
                                                 mipFilterNames,
                                                 0,
                                                 1u,
                                                 "Forces all mipmaps to use a particular blend between levels (or disables mipping.)");
    r_texFilterMipBias = _Dvar_RegisterFloat("r_texFilterMipBias", 0.0, -16.0, 15.99, 0x80u, "Change the mipmap bias");
    r_fullbright = _Dvar_RegisterBool("r_fullbright", 0, 0x80u, "Toggles rendering without lighting");
    r_debugShader = _Dvar_RegisterEnum("r_debugShader", debugShaderNames, 0, 0x80u, "Enable shader debugging information");
    r_lightConflicts = _Dvar_RegisterBool("r_lightConflicts", 0, 0x80u, "Shows conflicts between primary lights");
    r_gpuSync = _Dvar_RegisterEnum(
                                "r_gpuSync",
                                gpuSyncNames,
                                1,
                                0,
                                "GPU synchronization type (used to improve mouse responsiveness)");
#ifdef KISAK_MULTI_GPU
    r_multiGpu = _Dvar_RegisterBool("r_multiGpu", 1, 1u, "Use multiple GPUs");
#else
    r_multiGpu = _Dvar_RegisterBool("r_multiGpu", 0, 1u, "Use multiple GPUs");
#endif
    r_skinCache = _Dvar_RegisterBool("r_skinCache", 1, 0, "Enable cache for vertices of animated models");
    r_fastSkin = _Dvar_RegisterBool("r_fastSkin", 0, 1u, "Enable fast model skinning");
    r_smc_enable = _Dvar_RegisterBool("r_smc_enable", 1, 0, "Enable static model cache");
    r_pretess = _Dvar_RegisterBool("r_pretess", 1, 0, "Batch surfaces to reduce primitive count");
    r_lodScaleRigid = _Dvar_RegisterFloat(
                                            "r_lodScaleRigid",
                                            1.0,
                                            0.0,
                                            3.4028235e38,
                                            0x80u,
                                            "Scale the level of detail distance for rigid models (larger reduces detail)");
    r_lodBiasRigid = _Dvar_RegisterFloat(
                                         "r_lodBiasRigid",
                                         0.0,
                                         -3.4028235e38,
                                         3.4028235e38,
                                         0x80u,
                                         "Bias the level of detail distance for rigid models (negative increases detail)");
    r_lodScaleSkinned = _Dvar_RegisterFloat(
                                                "r_lodScaleSkinned",
                                                1.0,
                                                0.0,
                                                3.4028235e38,
                                                0x80u,
                                                "Scale the level of detail distance for skinned models (larger reduces detail)");
    r_lodBiasSkinned = _Dvar_RegisterFloat(
                                             "r_lodBiasSkinned",
                                             0.0,
                                             -3.4028235e38,
                                             3.4028235e38,
                                             0x80u,
                                             "Bias the level of detail distance for skinned models (negative increases detail)");
    r_fovScaleThresholdSkinned = _Dvar_RegisterFloat(
                                                                 "r_fovScaleThresholdSkinned",
                                                                 2.4000001,
                                                                 0.0,
                                                                 3.4028235e38,
                                                                 1u,
                                                                 "Threshold FOV LOD scaling must pass before taking effect for skinned models");
    r_fovScaleThresholdRigid = _Dvar_RegisterFloat(
                                                             "r_fovScaleThresholdRigid",
                                                             2.4000001,
                                                             0.0,
                                                             3.4028235e38,
                                                             1u,
                                                             "Threshold FOV LOD scaling must pass before taking effect for rigid models");
    r_worldLod = _Dvar_RegisterBool("r_worldLod", 0, 0x80u, "Set to zero to force world lods to highest");
    r_znear = _Dvar_RegisterFloat(
                            "r_znear",
                            4.0,
                            1.0,
                            10000.0,
                            0x5000u,
                            "Things closer than this aren't drawn.    Reducing this increases z-fighting in the distance.");
    r_znear_depthhack = _Dvar_RegisterFloat("r_znear_depthhack", 0.1, 0.001, 16.0, 0x80u, "Viewmodel near clip plane");
    r_zfar = _Dvar_RegisterFloat(
                         "r_zfar",
                         0.0,
                         0.0,
                         3.4028235e38,
                         0x80u,
                         "Change the distance at which culling fog reaches 100% opacity; 0 is off");
    r_fog = _Dvar_RegisterBool("r_fog", 1, 0x80u, "Set to 0 to disable fog");
    r_polygonOffsetScale = _Dvar_RegisterFloat(
                                                     "r_polygonOffsetScale",
                                                     -1.0,
                                                     -4.0,
                                                     0.0,
                                                     1u,
                                                     "Offset scale for decal polygons; bigger values z-fight less but poke through walls more");
    r_polygonOffsetBias = _Dvar_RegisterFloat(
                                                    "r_polygonOffsetBias",
                                                    -1.0,
                                                    -16.0,
                                                    0.0,
                                                    1u,
                                                    "Offset bias for decal polygons; bigger values z-fight less but poke through walls more");
    r_picmip_manual = _Dvar_RegisterBool(
                                            "r_picmip_manual",
                                            0,
                                            1u,
                                            "If 0, picmip is set automatically.    If 1, picmip is set based on the other r_picmip dvars.");
    r_picmip = _Dvar_RegisterInt(
                             "r_picmip",
                             0,
                             0,
                             3,
                             1u,
                             "Picmip level of color maps.    If r_picmip_manual is 0, this is read-only.");
    r_picmip_bump = _Dvar_RegisterInt(
                                        "r_picmip_bump",
                                        0,
                                        0,
                                        3,
                                        1u,
                                        "Picmip level of normal maps.    If r_picmip_manual is 0, this is read-only.");
    r_picmip_spec = _Dvar_RegisterInt(
                                        "r_picmip_spec",
                                        0,
                                        0,
                                        3,
                                        1u,
                                        "Picmip level of specular maps.    If r_picmip_manual is 0, this is read-only.");
    r_picmip_water = _Dvar_RegisterInt("r_picmip_water", 0, 0, 1, 0x21u, "Picmip level of water maps.");
    r_envMapSpecular = _Dvar_RegisterBool("r_envMapSpecular", 1, 0, "Enables environment map specular lighting");
    r_lightMap = _Dvar_RegisterEnum(
                                 "r_lightMap",
                                 colorMapNames,
                                 1,
                                 0x80u,
                                 "Replace all lightmaps with pure black or pure white");
    r_colorMap = _Dvar_RegisterEnum(
                                 "r_colorMap",
                                 colorMapNames,
                                 1,
                                 0x80u,
                                 "Replace all color maps with pure black or pure white");
    r_normalMap = _Dvar_RegisterEnum(
                                    "r_normalMap",
                                    normalMapNames,
                                    1,
                                    0x80u,
                                    "Replace all normal maps with a flat normal map");
    r_specularRoughnessMap = _Dvar_RegisterEnum(
                                                         "r_specularRoughnessMap",
                                                         specularRoughnessMapNames,
                                                         0,
                                                         0x80u,
                                                         "Replace all specular and roughness maps");
    r_specularColorScale = _Dvar_RegisterFloat(
                                                     "r_specularColorScale",
                                                     1.0,
                                                     0.0,
                                                     100.0,
                                                     0x1080u,
                                                     "Set greater than 1 to brighten specular highlights");
    r_diffuseColorScale = _Dvar_RegisterFloat(
                                                    "r_diffuseColorScale",
                                                    1.0,
                                                    0.0,
                                                    100.0,
                                                    0x1080u,
                                                    "Globally scale the diffuse color of all point lights");
    r_useLayeredMaterials = _Dvar_RegisterBool(
                                                        "r_useLayeredMaterials",
                                                        0,
                                                        0x20u,
                                                        "Set to true to use layered materials on shader model 3 hardware");
    r_loadForRenderer = _Dvar_RegisterBool(
                                                "r_loadForRenderer",
                                                1,
                                                0x20u,
                                                "Set to false to disable dx allocations (for dedicated server mode)");
    r_showLodInfo = _Dvar_RegisterEnum("r_showLodInfo", lodInfoNames, 0, 0x80u, "Show LOD info");
    r_showTris = _Dvar_RegisterInt("r_showTris", 0, 0, 3, 0x80u, "Show triangle outlines");
    r_showTriCounts = _Dvar_RegisterBool("r_showTriCounts", 0, 0x80u, "Triangle count for each rendered entity");
    r_showSurfCounts = _Dvar_RegisterBool("r_showSurfCounts", 0, 0x80u, "Surface count for each rendered entity");
    r_showVertCounts = _Dvar_RegisterBool("r_showVertCounts", 0, 0x80u, "Vertex count for each entity");
    r_resampleScene = _Dvar_RegisterBool(
                                            "r_resampleScene",
                                            1,
                                            0x80u,
                                            "Upscale the frame buffer with sharpen filter and color correction.");
    r_showPenetration = _Dvar_RegisterEnum(
                                                "r_showPenetration",
                                                showPenetrationNames,
                                                0,
                                                0x80u,
                                                "Shows materials tagged with 'nopenetrate'");
    r_showPixelCost = _Dvar_RegisterEnum(
                                            "r_showPixelCost",
                                            showPixelCostNames,
                                            0,
                                            0x80u,
                                            "Shows how expensive it is to draw every pixel on the screen");
    r_xdebug = _Dvar_RegisterInt("r_xdebug", 0, 0, 3, 0x80u, "xmodel/xanim debug rendering");
    r_debugLineWidth = _Dvar_RegisterFloat("r_debugLineWidth", 1.0, 0.0, 16.0, 1u, "Width of server side debug lines");
    r_vc_makelog = _Dvar_RegisterInt(
                                     "r_vc_makelog",
                                     0,
                                     0,
                                     2,
                                     0x20u,
                                     "Enable logging of light grid points for the vis cache.    1 starts from scratch, 2 appends.");
    r_vc_showlog = _Dvar_RegisterInt(
                                     "r_vc_showlog",
                                     0,
                                     0,
                                     1024,
                                     0,
                                     "Show this many rows of light grid points for the vis cache");
    r_showLightGrid = _Dvar_RegisterBool("r_showLightGrid", 0, 0x80u, "Show light grid debugging information");
    r_showLightingOrigins = _Dvar_RegisterBool("r_showLightingOrigins", 0, 0x80u, "Show lighting origins for models");
    r_showMissingLightGrid = _Dvar_RegisterBool(
                                                         "r_showMissingLightGrid",
                                                         1,
                                                         0,
                                                         "Use rainbow colors for entities that are outside the light grid");
    r_cacheSModelLighting = _Dvar_RegisterBool(
                                                        "r_cacheSModelLighting",
                                                        1,
                                                        0,
                                                        "Speed up static model lighting by caching previous results");
    r_cacheModelLighting = _Dvar_RegisterBool(
                                                     "r_cacheModelLighting",
                                                     1,
                                                     0,
                                                     "Speed up model lighting by caching previous results");
    r_lightTweakAmbient = _Dvar_RegisterFloat("r_lightTweakAmbient", 0.1, 0.0, 4.0, 0x240u, "Ambient light strength");
    r_lightTweakDiffuseFraction = _Dvar_RegisterFloat(
                                                                    "r_lightTweakDiffuseFraction",
                                                                    0.5,
                                                                    0.0,
                                                                    1.0,
                                                                    0x240u,
                                                                    "diffuse light fraction");
    r_lightTweakSunLight = _Dvar_RegisterFloat("r_lightTweakSunLight", 1.0, 0.0, 32.0, 0x1200u, "Sunlight strength");
    r_lightTweakAmbientColor = _Dvar_RegisterColor(
                                                             "r_lightTweakAmbientColor",
                                                             1.0,
                                                             0.0,
                                                             0.0,
                                                             1.0,
                                                             0x240u,
                                                             "Light ambient color");
    r_lightTweakSunColor = _Dvar_RegisterLinearRGB(
                                                     (char*)"r_lightTweakSunColor",
                                                     1.0,
                                                     1.0,
                                                     0.0,
                                                     0.0,
                                                     1.0,
                                                     0x280u,
                                                     "Sun color");
    r_lightTweakSunDiffuseColor = _Dvar_RegisterColor(
                                                                    "r_lightTweakSunDiffuseColor",
                                                                    0.0,
                                                                    0.0,
                                                                    1.0,
                                                                    1.0,
                                                                    0x240u,
                                                                    "Sun diffuse color");
    r_lightTweakSunDirection = _Dvar_RegisterVec3(
                                                                                        "r_lightTweakSunDirection",
                                                                                        0.0,
                                                                                        0.0,
                                                                                        0.0,
                                                                                        -360.0,
                                                                                        360.0,
                                                                                        0x1280u,
                                                                                        "Sun direction in degrees");
    r_envMapOverride = _Dvar_RegisterBool("r_envMapOverride", 0, 0, "Min reflection intensity based on glancing angle.");
    r_envMapMinIntensity = _Dvar_RegisterFloat(
                                                     "r_envMapMinIntensity",
                                                     0.2,
                                                     0.0,
                                                     2.0,
                                                     0,
                                                     "Min reflection intensity based on glancing angle.");
    r_envMapMaxIntensity = _Dvar_RegisterFloat(
                                                     "r_envMapMaxIntensity",
                                                     0.5,
                                                     0.0099999998,
                                                     2.0,
                                                     0,
                                                     "Max reflection intensity based on glancing angle.");
    r_envMapExponent = _Dvar_RegisterFloat("r_envMapExponent", 5.0, 0.05f, 20.0, 0, "Reflection exponent.");
    r_envMapSunIntensity = _Dvar_RegisterFloat(
                                                     "r_envMapSunIntensity",
                                                     2.0,
                                                     0.0,
                                                     4.0,
                                                     0,
                                                     "Max sun specular intensity intensity with env map materials.");
    r_showSunDirectionDebug = _Dvar_RegisterBool(
                                                            "r_showSunDirectionDebug",
                                                            0,
                                                            0x80u,
                                                            "Toggles the display of sun direction debug");
    r_showOutdoorVolumeDebug = _Dvar_RegisterBool(
                                                             "r_showOutdoorVolumeDebug",
                                                             0,
                                                             0x80u,
                                                             "Toggles the display of particle outdoor volume debug");
    r_materialXYZ = _Dvar_RegisterColorXYZ(
                                        "r_materialXYZ",
                                        3.8599999,
                                        3.98,
                                        4.5500002,
                                        0.0,
                                        100.0,
                                        0,
                                        "material color checker XYZ d65 coords");
    r_collectActiveMaterials = _Dvar_RegisterBool(
                                                             "r_collectActiveMaterials",
                                                             0,
                                                             0,
                                                             "print active materials (IN REMOTE CONSOLE)");
    r_drawPrimHistogram = _Dvar_RegisterBool(
                                                    "r_drawPrimHistogram",
                                                    0,
                                                    0x80u,
                                                    "Draws a histogram of the sizes of each primitive batch");
    r_logFile = _Dvar_RegisterInt(
                                "r_logFile",
                                0,
                                0,
                                0x7FFFFFFF,
                                0,
                                "Write all graphics hardware calls for this many frames to a logfile");
    r_norefresh = _Dvar_RegisterBool("r_norefresh", 0, 0x80u, "Skips all rendering.    Useful for benchmarking.");
    r_scaleViewport = _Dvar_RegisterFloat(
                                            "r_scaleViewport",
                                            1.0,
                                            0.0,
                                            1.0,
                                            0x80u,
                                            "Scale 3D viewports by this fraction.    Use this to see if framerate is pixel shader bound.");
    r_smp_backend = _Dvar_RegisterBool("r_smp_backend", 1, 0, "Process renderer back end in a separate thread");
    r_smp_worker = _Dvar_RegisterBool("r_smp_worker", 1, 0, "Process renderer front end in a separate thread");
    value = Sys_GetDefaultWorkerThreadsCount();
    r_smp_worker_threads = _Dvar_RegisterInt("r_smp_worker_threads", value, 2, 8, 0, "Number of worker threads");
    for ( i = 0; i < r_smp_worker_threads->current.integer; ++i )
        r_smp_worker_thread[i] = R_RegisterWorkerThreadDvar(names[i], i);
    r_aaAlpha = _Dvar_RegisterEnum("r_aaAlpha", s_aaAlphaNames, 1, 1u, "Transparency anti-aliasing method");
    r_aaSamples = _Dvar_RegisterInt(
                                    "r_aaSamples",
                                    1,
                                    1,
                                    16,
                                    0x21u,
                                    "Anti-aliasing sample count; 1 disables anti-aliasing");
    r_vsync = _Dvar_RegisterBool(
                            "r_vsync",
                            1,
                            0x21u,
                            "Enable v-sync before drawing the next frame to avoid 'tearing' artifacts.");
    r_clear = _Dvar_RegisterEnum("r_clear", r_clearNames, 2, 0, "Controls how the color buffer is cleared");
    r_clearColor = _Dvar_RegisterColor(
                                     "r_clearColor",
                                     0.5,
                                     0.75,
                                     1.0,
                                     1.0,
                                     0,
                                     "Color to clear the screen to when clearing the frame buffer");
    r_clearColor2 = _Dvar_RegisterColor(
                                        "r_clearColor2",
                                        1.0,
                                        0.5,
                                        0.0,
                                        1.0,
                                        0,
                                        "Color to clear every second frame to (for use during development)");
    r_setFrameBufferAlpha = _Dvar_RegisterBool(
                                                        "r_setFrameBufferAlpha",
                                                        0,
                                                        0,
                                                        "Sets the rendered pixels' alpha to 255 and the background to 0");
    r_drawSun = _Dvar_RegisterBool("r_drawSun", 1, 1u, "Enable sun effects");
    r_drawWorld = _Dvar_RegisterBool("r_drawWorld", 1, 0x80u, "Enable world rendering");
    r_drawEntities = _Dvar_RegisterBool("r_drawEntities", 1, 0x80u, "Enable entity rendering");
    r_drawPoly = _Dvar_RegisterBool("r_drawPoly", 1, 0x80u, "Enable poly rendering");
    r_drawDynEnts = _Dvar_RegisterBool("r_drawDynEnts", 1, 0x80u, "Enable dynamic entity rendering");
    r_drawBModels = _Dvar_RegisterBool("r_drawBModels", 1, 0x80u, "Enable brush model rendering");
    r_drawSModels = _Dvar_RegisterBool("r_drawSModels", 1, 0x80u, "Enable static model rendering");
    r_drawXModels = _Dvar_RegisterBool("r_drawXModels", 1, 0x80u, "Enable xmodel rendering");
    r_drawSkinnedModels = _Dvar_RegisterBool("r_drawSkinnedModels", 1, 0x80u, "Enable skinned rendering");
    r_dlightLimit = _Dvar_RegisterInt(
                                        "r_dlightLimit",
                                        4,
                                        0,
                                        4,
                                        0,
                                        "Maximum number of dynamic lights drawn simultaneously");
    r_dlightMaxFullScreenRadius = _Dvar_RegisterFloat(
                                                                    "r_dlightMaxFullScreenRadius",
                                                                    200.0,
                                                                    1.0,
                                                                    10000.0,
                                                                    0,
                                                                    "Maximum radius to limit dlights to in fullscreen.");
    r_dlightMaxNonFullScreenRadius = _Dvar_RegisterFloat(
                                                                         "r_dlightMaxNonFullScreenRadius",
                                                                         200.0,
                                                                         1.0,
                                                                         10000.0,
                                                                         0,
                                                                         "Maximum radius to limit dlights to when not fullscreen");
    r_dobjLimit = _Dvar_RegisterInt("r_dobjLimit", 512, 0, 1024, 0x80u, "Maximum number of DObjs drawn simultaneously");
    r_modelLimit = _Dvar_RegisterInt(
                                     "r_modelLimit",
                                     1024,
                                     0,
                                     1024,
                                     0x80u,
                                     "Maximum number of models drawn simultaneously");
    r_brushLimit = _Dvar_RegisterInt(
                                     "r_brushLimit",
                                     512,
                                     0,
                                     1024,
                                     0x80u,
                                     "Maximum number of brushes drawn simultaneously");
    r_spotLightFovInnerFraction = _Dvar_RegisterFloat(
                                                                    "r_spotLightFovInnerFraction",
                                                                    0.69999999,
                                                                    0.0,
                                                                    0.99000001,
                                                                    0x80u,
                                                                    "Relative Inner FOV angle for the dynamic spot light. 0 is full fade 0.99 is almost no fade.");
    r_spotLightStartRadius = _Dvar_RegisterFloat(
                                                         "r_spotLightStartRadius",
                                                         36.0,
                                                         0.0,
                                                         1200.0,
                                                         0x1080u,
                                                         "Radius of the circle at the start of the spot light in inches.");
    r_spotLightEndRadius = _Dvar_RegisterFloat(
                                                     "r_spotLightEndRadius",
                                                     196.0,
                                                     1.0,
                                                     1200.0,
                                                     0x1080u,
                                                     "Radius of the circle at the end of the spot light in inches.");
    r_spotLightShadows = _Dvar_RegisterBool("r_spotLightShadows", 1, 0x80u, "Enable shadows for spot lights.");
    r_spotLightSModelShadows = _Dvar_RegisterBool(
                                                             "r_spotLightSModelShadows",
                                                             1,
                                                             0x80u,
                                                             "Enable static model shadows for spot lights.");
    r_spotLightEntityShadows = _Dvar_RegisterBool(
                                                             "r_spotLightEntityShadows",
                                                             1,
                                                             0x80u,
                                                             "Enable entity shadows for spot lights.");
    r_spotLightBrightness = _Dvar_RegisterFloat(
                                                        "r_spotLightBrightness",
                                                        14.0,
                                                        0.0,
                                                        16.0,
                                                        0x1080u,
                                                        "Brightness scale for spot light to get overbrightness from the 0-1 particle color range.");
    r_flashLightFovInnerFraction = _Dvar_RegisterFloat(
                                                                     "r_flashLightFovInnerFraction",
                                                                     0.69999999,
                                                                     0.0,
                                                                     0.99000001,
                                                                     0x1000u,
                                                                     "Relative Inner FOV angle for the dynamic flash light. 0 is full fade 0.99 is almost no fade.");
    r_flashLightStartRadius = _Dvar_RegisterFloat(
                                                            "r_flashLightStartRadius",
                                                            0.0,
                                                            0.0,
                                                            1200.0,
                                                            0x1000u,
                                                            "Radius of the circle at the start of the flash light in inches.");
    r_flashLightEndRadius = _Dvar_RegisterFloat(
                                                        "r_flashLightEndRadius",
                                                        300.0,
                                                        1.0,
                                                        1200.0,
                                                        0x1000u,
                                                        "Radius of the circle at the end of the flash light in inches.");
    r_flashLightShadows = _Dvar_RegisterBool("r_flashLightShadows", 1, 0x1000u, "Enable shadows for flash light.");
    r_flashLightBrightness = _Dvar_RegisterFloat(
                                                         "r_flashLightBrightness",
                                                         10.0,
                                                         0.0,
                                                         25.0,
                                                         0x1000u,
                                                         "Brightness scale for flash light.");
    r_flashLightOffset = _Dvar_RegisterVec3(
                                                 "r_flashLightOffset",
                                                 0.0,
                                                 2.0,
                                                 1.0,
                                                 -100.0,
                                                 100.0,
                                                 0x1000u,
                                                 "Relative position of the Flashlight.");
    r_flashLightRange = _Dvar_RegisterFloat(
                                                "r_flashLightRange",
                                                350.0,
                                                1.0,
                                                1200.0,
                                                0x1000u,
                                                "Distance of the flash light in inches.");
    r_flashLightColor = _Dvar_RegisterVec3(
                                                "r_flashLightColor",
                                                1.0,
                                                1.0,
                                                1.0,
                                                0.0,
                                                1.0,
                                                0x1000u,
                                                "Flashlight Color.");
    r_flashLightBobAmount = _Dvar_RegisterVec3(
                                                        "r_flashLightBobAmount",
                                                        (0.0),
                                                        (0.0),
                                                        (0.0),
                                                        0.0,
                                                        50.0,
                                                        0x1000u,
                                                        "Flashlight Movement Amount.");
    r_flashLightBobRate = _Dvar_RegisterVec3(
                                                    "r_flashLightBobRate",
                                                    (0.0),
                                                    (0.2),
                                                    (0.30000001),
                                                    0.0,
                                                    2.0,
                                                    0x1000u,
                                                    "Flashlight Movement Rate.");
    r_flashLightSpecularScale = _Dvar_RegisterFloat(
                                                                "r_flashLightSpecularScale",
                                                                1.0,
                                                                0.0,
                                                                10.0,
                                                                0x1000u,
                                                                "Flashlight Specular Scale.");
    r_flashLightFlickerAmount = _Dvar_RegisterFloat(
                                                                "r_flashLightFlickerAmount",
                                                                0.0,
                                                                0.0,
                                                                1.0,
                                                                0x1000u,
                                                                "Flashlight Flicker Amount.");
    r_flashLightFlickerRate = _Dvar_RegisterFloat(
                                                            "r_flashLightFlickerRate",
                                                            65.0,
                                                            0.1,
                                                            65.0,
                                                            0x1000u,
                                                            "Flashlight Flicker Rate (Times per second).");
    r_drawPrimCap = _Dvar_RegisterInt(
                                        "r_drawPrimCap",
                                        0,
                                        -1,
                                        10000,
                                        0x80u,
                                        "Only draw primitive batches with less than this many triangles");
    r_drawPrimFloor = _Dvar_RegisterInt(
                                            "r_drawPrimFloor",
                                            0,
                                            0,
                                            10000,
                                            0x80u,
                                            "Only draw primitive batches with more than this many triangles");
    r_skipDrawTris = _Dvar_RegisterBool("r_skipDrawTris", 0, 0x80u, "Skip drawing primitive tris.");
    r_drawWater = _Dvar_RegisterBool("r_drawWater", 1, 0x80u, "Enable water rendering");
    r_lockPvs = _Dvar_RegisterBool(
                                "r_lockPvs",
                                0,
                                0x80u,
                                "Lock the viewpoint used for determining what is visible to the current position and direction");
    r_skipPvs = _Dvar_RegisterBool(
                                "r_skipPvs",
                                0,
                                0x80u,
                                "Skipt the determination of what is in the potentially visible set (disables most drawing)");
    r_portalBevels = _Dvar_RegisterFloat(
                                         "r_portalBevels",
                                         0.69999999,
                                         0.0,
                                         1.0,
                                         1u,
                                         "Helps cull geometry by angles of portals that are acute when projected onto the screen, value is th"
                                         "e cosine of the angle");
    r_portalBevelsOnly = _Dvar_RegisterBool(
                                                 "r_portalBevelsOnly",
                                                 0,
                                                 0,
                                                 "Use screen-space bounding box of portals rather than the actual shape of the portal projected onto the screen");
    r_singleCell = _Dvar_RegisterBool(
                                     "r_singleCell",
                                     0,
                                     0x80u,
                                     "Only draw things in the same cell as the camera.    Most useful for seeing how big the current cell is.");
    r_showForcedInvisibleCells = _Dvar_RegisterBool(
                                                                 "r_showForcedInvisibleCells",
                                                                 0,
                                                                 0x80u,
                                                                 "Only draw cells that are forced to be off.");
    r_portalWalkLimit = _Dvar_RegisterInt(
                                                "r_portalWalkLimit",
                                                0,
                                                0,
                                                100,
                                                0x80u,
                                                "Stop portal recursion after this many iterations.    Useful for debugging portal errors.");
    r_portalMinClipArea = _Dvar_RegisterFloat(
                                                    "r_portalMinClipArea",
                                                    0.02,
                                                    0.0,
                                                    1.0,
                                                    0,
                                                    "Don't clip child portals by a parent portal smaller than this fraction of the screen area.");
    r_portalMinRecurseDepth = _Dvar_RegisterInt(
                                                            "r_portalMinRecurseDepth",
                                                            2,
                                                            0,
                                                            100,
                                                            0x80u,
                                                            "Ignore r_portalMinClipArea for portals with fewer than this many parent portals.");
    r_enableOccluders = _Dvar_RegisterBool("r_enableOccluders", 1, 0x80u, "Enable occluders");
    r_showPortals = _Dvar_RegisterInt("r_showPortals", 0, 0, 3, 0x80u, "Show portals for debugging");
    r_showOccluders = _Dvar_RegisterInt("r_showOccluders", 0, 0, 4, 0x80u, "Show occluders for debugging");
    r_showAabbTrees = _Dvar_RegisterInt(
                                            "r_showAabbTrees",
                                            0,
                                            0,
                                            2,
                                            0x80u,
                                            "Show axis-aligned bounding box trees used in potentially visibility set determination");
    r_showSModelNames = _Dvar_RegisterBool("r_showSModelNames", 0, 0x80u, "Show static model names");
    r_showDynEntModelNames = _Dvar_RegisterBool("r_showDynEntModelNames", 0, 0x80u, "Show dynamic entity model names");
    r_showDObjModelNames = _Dvar_RegisterFloat(
                                                     "r_showDObjModelNames",
                                                     -1.0,
                                                     -1.0,
                                                     3000.0,
                                                     0x80u,
                                                     "Show dobj model names within range");
    r_showEntModelNames = _Dvar_RegisterFloat(
                                                    "r_showEntModelNames",
                                                    -1.0,
                                                    -1.0,
                                                    3000.0,
                                                    0x80u,
                                                    "Show entity model names within range");
    r_showTess = _Dvar_RegisterEnum("r_showTess", r_showTessNames, 0, 0x80u, "Show details for each surface");
    r_showCullBModels = _Dvar_RegisterBool("r_showCullBModels", 0, 0x80u, "Show culled brush models");
    r_showCullSModels = _Dvar_RegisterBool("r_showCullSModels", 0, 0x80u, "Show culled static models");
    r_showCullXModels = _Dvar_RegisterBool("r_showCullXModels", 0, 0x80u, "Show culled xmodels");
    r_showFbColorDebug = _Dvar_RegisterEnum(
                                                 "r_showFbColorDebug",
                                                 fbColorDebugNames,
                                                 0,
                                                 0x80u,
                                                 "Show front buffer color debugging information");
    r_showFloatZDebug = _Dvar_RegisterBool(
                                                "r_showFloatZDebug",
                                                0,
                                                0x80u,
                                                "Show float z buffer used to eliminate hard edges on particles near geometry");
    r_showCollision = _Dvar_RegisterEnum(
                                            "r_showCollision",
                                            showCollisionNames,
                                            0,
                                            0x80u,
                                            "Show the collision surfaces for the selected mask types");
    r_showCollisionGroups = _Dvar_RegisterEnum(
                                                        "r_showCollisionGroups",
                                                        showCollisionGroupsNames,
                                                        0,
                                                        0x80u,
                                                        "Select whether to show the terrain, brush or all collision surface groups");
    r_showCollisionPolyType = _Dvar_RegisterEnum(
                                                            "r_showCollisionPolyType",
                                                            showCollisionPolyTypeNames,
                                                            0,
                                                            0x80u,
                                                            "Select whether to display the collision surfaces as wireframe, poly interiors, or both");
    r_showCollisionDepthTest = _Dvar_RegisterBool(
                                                             "r_showCollisionDepthTest",
                                                             1,
                                                             0x80u,
                                                             "Select whether to use depth test in collision surfaces display");
    r_showCollisionDist = _Dvar_RegisterFloat(
                                                    "r_showCollisionDist",
                                                    500.0,
                                                    1.0,
                                                    3.4028235e38,
                                                    0x80u,
                                                    "Maximum distance to show collision surfaces");
    r_floatz = _Dvar_RegisterBool(
                             "r_floatz",
                             1,
                             0x20u,
                             "Allocate a float z buffer (required for effects such as floatz, dof, and laser light)");
    r_depthPrepass = _Dvar_RegisterBool("r_depthPrepass", 1, 1u, "Enable depth prepass (usually improves performance)");
    r_highLodDist = _Dvar_RegisterFloat(
                                        "r_highLodDist",
                                        -1.0,
                                        -1.0,
                                        3.4028235e38,
                                        0x80u,
                                        "Distance for high level of detail");
    r_mediumLodDist = _Dvar_RegisterFloat(
                                            "r_mediumLodDist",
                                            -1.0,
                                            -1.0,
                                            3.4028235e38,
                                            0x80u,
                                            "Distance for medium level of detail");
    r_lowLodDist = _Dvar_RegisterFloat(
                                     "r_lowLodDist",
                                     -1.0,
                                     -1.0,
                                     3.4028235e38,
                                     0x80u,
                                     "Distance for low level of detail");
    r_lowestLodDist = _Dvar_RegisterFloat(
                                            "r_lowestLodDist",
                                            -1.0,
                                            -1.0,
                                            3.4028235e38,
                                            0x80u,
                                            "Distance for lowest level of detail");
    r_forceLod = _Dvar_RegisterEnum("r_forceLod", r_forceLodNames, 4, 0x80u, "Force all level of detail to this level");
    r_modelVertColor = _Dvar_RegisterBool(
                                             "r_modelVertColor",
                                             1,
                                             0xA0u,
                                             "Set to 0 to replace all model vertex colors with white when loaded");
    sm_enable = _Dvar_RegisterBool("sm_enable", 1, 0, "Enable shadow mapping");
    sm_sunEnable = _Dvar_RegisterBool("sm_sunEnable", 1, 0x1000u, "Enable sun shadow mapping from script");
    sm_spotEnable = _Dvar_RegisterBool("sm_spotEnable", 1, 0x1000u, "Enable spot shadow mapping from script");
    sm_maxLights = _Dvar_RegisterInt("sm_maxLights", 4, 0, 4, 1u, "Limits how many primary lights can have shadow maps");
    sm_spotShadowFadeTime = _Dvar_RegisterFloat(
                                                        "sm_spotShadowFadeTime",
                                                        1.0,
                                                        0.0099999998,
                                                        5.0,
                                                        0,
                                                        "How many seconds it takes for a primary light shadow map to fade in or out");
    sm_lightScore_eyeProjectDist = _Dvar_RegisterFloat(
                                                                     "sm_lightScore_eyeProjectDist",
                                                                     64.0,
                                                                     0.0,
                                                                     1024.0,
                                                                     0,
                                                                     "When picking shadows for primary lights, measure distance from a point this far in fr"
                                                                     "ont of the camera.");
    sm_lightScore_spotProjectFrac = _Dvar_RegisterFloat(
                                                                        "sm_lightScore_spotProjectFrac",
                                                                        0.125,
                                                                        0.0,
                                                                        1.0,
                                                                        0,
                                                                        "When picking shadows for primary lights, measure distance to a point this fraction o"
                                                                        "f the light's radius along it's shadow direction.");
    sm_showOverlay = _Dvar_RegisterEnum("sm_showOverlay", sm_showOverlayNames, 0, 0x80u, "Show shadow map overlay");
    sm_showOverlayDepthBounds = _Dvar_RegisterVec2(
                                                                "sm_showOverlayDepthBounds",
                                                                (0.25),
                                                                (0.75),
                                                                0.0,
                                                                1.0,
                                                                0x80u,
                                                                "Near and far depth values for the shadow map overlay");
    sm_showSpotAxis = _Dvar_RegisterBool("sm_showSpotAxis", 0, 0x80u, "Show spot shadow origin and orientation");
    sm_sunAlwaysCastsShadow = _Dvar_RegisterBool(
                                                            "sm_sunAlwaysCastsShadow",
                                                            0,
                                                            0x1080u,
                                                            "forces sun to always do shadow mapping, from script");
    sm_polygonOffsetScale = _Dvar_RegisterFloat("sm_polygonOffsetScale", 2.0, 0.0, 8.0, 0, "Shadow map offset scale");
    sm_polygonOffsetBias = _Dvar_RegisterFloat("sm_polygonOffsetBias", 10.0, 0.0, 32.0, 0x1080u, "Shadow map offset bias");
    sm_sunSampleSizeNear = _Dvar_RegisterFloat("sm_sunSampleSizeNear", 0.25, 0.0625, 32.0, 0x1080u, "Shadow sample size");
    sm_sunShadowCenter = _Dvar_RegisterVec3(
                                                 "sm_sunShadowCenter",
                                                 (0.0),
                                                 (0.0),
                                                 (0.0),
                                                 -3.4028235e38,
                                                 3.4028235e38,
                                                 0x1080u,
                                                 "Sun shadow center, 0 0 0 means don't override");
    sm_sunShadowScale = _Dvar_RegisterFloat("sm_sunShadowScale", 1.0, 0.25, 1.0, 0x1080u, "Sun shadow scale optimization");
    sm_sunShadowSmall = _Dvar_RegisterBool("sm_sunShadowSmall", 0, 0, "force quarter resolution sun shadow map");
    sm_sunShadowSmallEnable = _Dvar_RegisterBool("sm_sunShadowSmallEnable", 0, 0, "use quarter resolution sun shadow map");
    r_backBufferSize = _Dvar_RegisterInt("r_backBufferSize", 960, 128, 1024, 1u, "Size of Back Buffer");
    r_backBufferSizeY = _Dvar_RegisterInt("r_backBufferSizeY", 544, 128, 608, 1u, "Size of Back Buffer Y");
    sm_spotShadowLargeRadiusScale = _Dvar_RegisterFloat(
                                                                        "sm_spotShadowLargeRadiusScale",
                                                                        1.0,
                                                                        0.0,
                                                                        5.0,
                                                                        0x1080u,
                                                                        "Radius scaler for large spot shadow switch over");
    sm_strictCull = _Dvar_RegisterBool("sm_strictCull", 1, 0x80u, "Strict shadow map cull");
    sm_fastSunShadow = _Dvar_RegisterBool("sm_fastSunShadow", 1, 0x80u, "Fast sun shadow");
    sm_qualitySpotShadow = _Dvar_RegisterBool("sm_qualitySpotShadow", 1, 0x80u, "Fast spot shadow");
    sm_fullResSpotShadowEnable = _Dvar_RegisterBool(
                                                                 "sm_fullResSpotShadowEnable",
                                                                 0,
                                                                 0x1080u,
                                                                 "Provides high-res spot shadows for the only one spotlight in the scene.");
    sm_debugFastSunShadow = _Dvar_RegisterBool("sm_debugFastSunShadow", 0, 0x80u, "Debug fast sun shadow");
    r_stream = _Dvar_RegisterInt("r_stream", 7, 0, 7, 0x80u, "Stream high mip levels (1=world|2=xmodels|4=bmodels)");
    r_streamCheckAabb = _Dvar_RegisterBool("r_streamCheckAabb", 0, 0x80u, "Enables runtime checking of stream aabb tree");
    r_streamClear = _Dvar_RegisterBool("r_streamClear", 0, 0x80u, "Set true to clear all streamed highmip levels");
    r_streamDebug = _Dvar_RegisterBool(
                                        "r_streamDebug",
                                        0,
                                        0x80u,
                                        "Corrupts high mip levels which should not be visible, rather than unloading them.");
    r_streamFull = _Dvar_RegisterBool(
                                     "r_streamFull",
                                     0,
                                     0x10u,
                                     "Don't let anyone take any of your precious stream buffer.    Some people need to debug texture streamin"
                                     "g.    Steve needs to debug texture streaming.");
    r_streamFakeLagMsec = _Dvar_RegisterInt(
                                                    "r_streamFakeLagMsec",
                                                    0,
                                                    0,
                                                    2000,
                                                    0x80u,
                                                    "Number of msec to add to streaming to roughly fake DVD lag");
    r_streamMaxDist = _Dvar_RegisterFloat(
                                            "r_streamMaxDist",
                                            600.0,
                                            0.0,
                                            100000.0,
                                            0x80u,
                                            "Maximum distance to predictively stream from");
    r_streamShowStats = _Dvar_RegisterBool("r_streamShowStats", 0, 0x80u, "Show streaming stats");
    r_streamShowList = _Dvar_RegisterBool("r_streamShowList", 0, 0x80u, "Show list of streamed textures");
    r_streamSize = _Dvar_RegisterInt(
                                     "r_streamSize",
                                     0,
                                     0,
                                     0x40000,
                                     0x80u,
                                     "Limit size of stream buffer in KB (0 is no limit)");
    r_streamTaint = _Dvar_RegisterInt(
                                        "r_streamTaint",
                                        0,
                                        0,
                                        5,
                                        0x80u,
                                        "Corrupt loaded mips (1-2: solid roygbiv to show mip rendered; 3-5: color coded imageIndex, difference is alpha)");
    r_streamFreezeState = _Dvar_RegisterBool(
                                                    "r_streamFreezeState",
                                                    0,
                                                    0x5000u,
                                                    "Freeze the state of all streaming memory - don't allow streaming or un-streaming of any images.");
    r_streamLog = _Dvar_RegisterInt(
                                    "r_streamLog",
                                    0,
                                    0,
                                    4,
                                    0x80u,
                                    "Enable logging of all streaming commands executed - printed to Output and console");
    r_streamHiddenPush = _Dvar_RegisterFloat(
                                                 "r_streamHiddenPush",
                                                 4.0,
                                                 1.0,
                                                 1000.0,
                                                 0x80u,
                                                 "Scale the distance of hidden objects by this amount");
    r_streamLowDetail = _Dvar_RegisterBool(
                                                "r_streamLowDetail",
                                                1,
                                                0x80u,
                                                "Force lowest detail mips to always be loaded (disable this for more memory in release builds)");
    _Dvar_RegisterFloat("r_stereoTurretShift", 0.0, 0.0, 10000.0, 0x1000u, "3D turret shift");
    r_blur_allowed = _Dvar_RegisterBool("r_blur_allowed", 1, 1u, "Allow blur.");
    r_blur = _Dvar_RegisterFloat("r_blur", 0.0, 0.0, 32.0, 0x80u, "Dev tweak to blur the screen");
    r_distortion = _Dvar_RegisterBool("r_distortion", 1, 1u, "Enable distortion");
    r_flame_allowed = _Dvar_RegisterBool("r_flame_allowed", 1, 1u, "Allow flame effect.");
    r_filmLut = _Dvar_RegisterFloat("r_filmLut", -1.0, -1.0, 9.0, 1u, "Film LUT Index.");
    r_filmTweakLut = _Dvar_RegisterInt("r_filmTweakLut", -1, -1, 9, 0x1000u, "Tweak Film LUT Index.");
    r_enablePlayerShadow = _Dvar_RegisterBool("r_enablePlayerShadow", 0, 0x5000u, "Enable First Person Player Shadow.");
    r_enableFlashlight = _Dvar_RegisterInt("r_enableFlashlight", 0, 0, 2, 0x1000u, "Enable Player Flashlight.");
    r_waterFogTest = _Dvar_RegisterBool("r_waterFogTest", 0, 0x80u, "Enable Water Fog Test.");
    r_contrast = _Dvar_RegisterFloat("r_contrast", 1.0, 0.0, 4.0, 0x80u, "Contrast adjustment");
    r_brightness = _Dvar_RegisterFloat("r_brightness", 0.0, -1.0, 1.0, 0x80u, "Brightness adjustment");
    r_desaturation = _Dvar_RegisterFloat("r_desaturation", 1.0, 0.0, 4.0, 0x80u, "Desaturation adjustment");
    r_filmTweakBrightness = _Dvar_RegisterFloat(
                                                        "r_filmTweakBrightness",
                                                        0.0,
                                                        -1.0,
                                                        1.0,
                                                        0x1000u,
                                                        "Tweak dev var; film color brightness");
    r_filmTweakDesaturation = _Dvar_RegisterFloat(
                                                            "r_filmTweakDesaturation",
                                                            0.2,
                                                            0.0,
                                                            1.0,
                                                            0x1000u,
                                                            "Tweak dev var; Desaturation applied after all 3D drawing");
    r_filmTweakInvert = _Dvar_RegisterBool("r_filmTweakInvert", 0, 0x1000u, "Tweak dev var; enable inverted video");
    r_filmUseTweaks = _Dvar_RegisterBool("r_filmUseTweaks", 0, 0x80u, "Overide film effects with tweak dvar values");
    r_filmTweakEnable = _Dvar_RegisterBool("r_filmTweakEnable", 0, 0x1000u, "Tweak dev var; enable film color effects");
    r_filmTweakHue = _Dvar_RegisterVec3(
                                         "r_filmTweakHue",
                                         (0.0),
                                         (0.0),
                                         (0.0),
                                         -180.0,
                                         180.0,
                                         0x1000u,
                                         "Tweak dev var; film color Hue dark - mid - light");
    r_filmTweakSaturation = _Dvar_RegisterVec3(
                                                        "r_filmTweakSaturation",
                                                        (1.0),
                                                        (1.0),
                                                        (1.0),
                                                        0.0,
                                                        2.0,
                                                        0x1000u,
                                                        "Tweak dev var; film color Saturation dark - mid - light");
    r_filmTweakColorTemp = _Dvar_RegisterVec3(
                                                     "r_filmTweakColorTemp",
                                                     (6500.0),
                                                     (6500.0),
                                                     (6500.0),
                                                     1650.0,
                                                     25000.0,
                                                     0x1000u,
                                                     "Tweak dev var; film color Color Temp dark - mid - light");
    r_filmTweakDarkTint = _Dvar_RegisterVec3(
                                                    "r_filmTweakDarkTint",
                                                    (1.0),
                                                    (1.0),
                                                    (1.0),
                                                    0.0,
                                                    6.0,
                                                    0x1000u,
                                                    "Tweak dev var; film color Dark Tint");
    r_filmTweakMidTint = _Dvar_RegisterVec3(
                                                 "r_filmTweakMidTint",
                                                 (1.0),
                                                 (1.0),
                                                 (1.0),
                                                 0.0,
                                                 6.0,
                                                 0x1000u,
                                                 "Tweak dev var; film color Mid Tint");
    r_filmTweakLightTint = _Dvar_RegisterVec3(
                                                                                                    "r_filmTweakLightTint",
                                                                                                    (1.0),
                                                                                                    (1.0),
                                                                                                    (1.0),
                                                                                                    0.0,
                                                                                                    6.0,
                                                                                                    0x1000u,
                                                                                                    "Tweak dev var; film color Light Tint");
    r_filmTweakMidStart = _Dvar_RegisterFloat(
                                                    "r_filmTweakMidStart",
                                                    0.25,
                                                    0.0,
                                                    1.0,
                                                    0x1000u,
                                                    "Tweak dev var; film color Mid Start");
    r_filmTweakMidEnd = _Dvar_RegisterFloat(
                                                "r_filmTweakMidEnd",
                                                0.75,
                                                0.0,
                                                1.0,
                                                0x1000u,
                                                "Tweak dev var; film color Mid End");
    r_filmTweakDarkFeather = _Dvar_RegisterFloat(
                                                         "r_filmTweakDarkFeather",
                                                         0.0,
                                                         0.0,
                                                         1.0,
                                                         0x1000u,
                                                         "Tweak dev var; film color Dark range feather amount");
    r_filmTweakLightFeather = _Dvar_RegisterFloat(
                                                            "r_filmTweakLightFeather",
                                                            0.0,
                                                            0.0,
                                                            1.0,
                                                            0x1000u,
                                                            "Tweak dev var; film color Light range feather amount");
    r_filmTweakContrast = _Dvar_RegisterVec3(
                                                    "r_filmTweakContrast",
                                                    (1.0),
                                                    (1.0),
                                                    (1.0),
                                                    0.0,
                                                    2.0,
                                                    0x1000u,
                                                    "Tweak dev var; film color Contrast dark - mid - light");
    r_filmTweakBleach = _Dvar_RegisterVec3(
                                                "r_filmTweakBleach",
                                                (0.0),
                                                (0.0),
                                                (0.0),
                                                0.0,
                                                1.0,
                                                0x1000u,
                                                "Tweak dev var; film color Bleach dark - mid - light");
    r_filmTweakRangeDebug = _Dvar_RegisterBool(
                                                        "r_filmTweakRangeDebug",
                                                        0,
                                                        0x80u,
                                                        "Tweak dev var; enable dark - mid - light debug display");
    r_sCurveShoulderStrength = _Dvar_RegisterFloat(
                                                             "r_sCurveShoulderStrength",
                                                             0.1,
                                                             0.0,
                                                             1.0,
                                                             0x1000u,
                                                             "S-Curve Shoulder Strength");
    r_sCurveLinearStrength = _Dvar_RegisterFloat(
                                                         "r_sCurveLinearStrength",
                                                         0.5,
                                                         0.0,
                                                         1.0,
                                                         0x1000u,
                                                         "S-Curve Linear Strength");
    r_sCurveLinearAngle = _Dvar_RegisterFloat("r_sCurveLinearAngle", 0.125, 0.0, 1.0, 0x1000u, "S-Curve Linear Angle");
    r_sCurveToeStrength = _Dvar_RegisterFloat("r_sCurveToeStrength", 0.1, 0.0, 1.0, 0x1000u, "S-Curve Toe Strength");
    r_sCurveToeNumerator = _Dvar_RegisterFloat(
                                                     "r_sCurveToeNumerator",
                                                     0.0099999998,
                                                     0.0,
                                                     1.0,
                                                     0x1000u,
                                                     "S-Curve Toe Numerator");
    r_sCurveToeDenominator = _Dvar_RegisterFloat(
                                                         "r_sCurveToeDenominator",
                                                         0.57499999,
                                                         0.0,
                                                         1.0,
                                                         0x1000u,
                                                         "S-Curve Toe Denominator");
    r_sCurveEnable = _Dvar_RegisterInt("r_sCurveEnable", 1, 0, 3, 0x1000u, "S-Curve On-Off");
    r_flimCurveBlack = _Dvar_RegisterFloat("r_flimCurveBlack", 0.0, 0.0, 32.0, 0x1000u, "film curve black point");
    r_flimCurveWhite = _Dvar_RegisterFloat("r_flimCurveWhite", 8.0, 1.0, 32.0, 0x1000u, "film curve white point");
    r_filmCurveSoftClip = _Dvar_RegisterFloat("r_filmCurveSoftClip", 1.0, 0.0, 1.0, 0x1000u, "film curve soft clip point");
    r_waterWaveAngle = _Dvar_RegisterVec4(
                                             "r_waterWaveAngle",
                                             (0.0),
                                             (0.0),
                                             (0.0),
                                             (0.0),
                                             0.0,
                                             360.0,
                                             0x5000u,
                                             "water wave angle");
    r_waterWaveWavelength = _Dvar_RegisterVec4(
                                                        "r_waterWaveWavelength",
                                                        (1.0),
                                                        (1.0),
                                                        (1.0),
                                                        (1.0),
                                                        1.0,
                                                        1024.0,
                                                        0x5000u,
                                                        "water wave wavelength");
    r_waterWaveAmplitude = _Dvar_RegisterVec4(
                                                     "r_waterWaveAmplitude",
                                                     (0.0),
                                                     (0.0),
                                                     (0.0),
                                                     (0.0),
                                                     0.0,
                                                     256.0,
                                                     0x5000u,
                                                     "water wave amplitude");
    r_waterWavePhase = _Dvar_RegisterVec4(
                                             "r_waterWavePhase",
                                             (0.0),
                                             (0.0),
                                             (0.0),
                                             (0.0),
                                             0.0,
                                             6.283185,
                                             0x5000u,
                                             "water wave phase");
    r_waterWaveSteepness = _Dvar_RegisterVec4(
                                                     "r_waterWaveSteepness",
                                                     (0.0),
                                                     (0.0),
                                                     (0.0),
                                                     (0.0),
                                                     0.0,
                                                     1.0,
                                                     0x5000u,
                                                     "water wave steepness");
    r_waterWaveSpeed = _Dvar_RegisterVec4(
                                             "r_waterWaveSpeed",
                                             (1.0),
                                             (1.0),
                                             (1.0),
                                             (1.0),
                                             0.0,
                                             2.0,
                                             0x5000u,
                                             "water wave speed");
    r_dof_enable = _Dvar_RegisterBool("r_dof_enable", 1, 0x4001u, "Enable the depth of field effect");
    r_dof_tweak = _Dvar_RegisterBool(
                                    "r_dof_tweak",
                                    0,
                                    0x5000u,
                                    "Use dvars to set the depth of field effect; overrides r_dof_enable");
    r_dof_nearBlur = _Dvar_RegisterFloat(
                                         "r_dof_nearBlur",
                                         6.0,
                                         4.0,
                                         10.0,
                                         0x4080u,
                                         "Sets the radius of the gaussian blur used by depth of field, in pixels at 640x480");
    r_dof_farBlur = _Dvar_RegisterFloat(
                                        "r_dof_farBlur",
                                        1.8,
                                        0.0,
                                        10.0,
                                        0x4080u,
                                        "Sets the radius of the gaussian blur used by depth of field, in pixels at 640x480");
    r_dof_viewModelStart = _Dvar_RegisterFloat(
                                                     "r_dof_viewModelStart",
                                                     2.0,
                                                     0.0,
                                                     128.0,
                                                     0x4080u,
                                                     "Depth of field viewmodel start distance, in inches");
    r_dof_viewModelEnd = _Dvar_RegisterFloat(
                                                 "r_dof_viewModelEnd",
                                                 8.0,
                                                 0.0,
                                                 128.0,
                                                 0x4080u,
                                                 "Depth of field viewmodel end distance, in inches");
    r_dof_nearStart = _Dvar_RegisterFloat(
                                            "r_dof_nearStart",
                                            10.0,
                                            0.0,
                                            1000.0,
                                            0x4080u,
                                            "Depth of field near start distance, in inches");
    r_dof_nearEnd = _Dvar_RegisterFloat(
                                        "r_dof_nearEnd",
                                        60.0,
                                        0.0,
                                        1000.0,
                                        0x4080u,
                                        "Depth of field near end distance, in inches");
    r_dof_farStart = _Dvar_RegisterFloat(
                                         "r_dof_farStart",
                                         1000.0,
                                         0.0,
                                         20000.0,
                                         0x4080u,
                                         "Depth of field far start distance, in inches");
    r_dof_farEnd = _Dvar_RegisterFloat(
                                     "r_dof_farEnd",
                                     7000.0,
                                     0.0,
                                     20000.0,
                                     0x4080u,
                                     "Depth of field far end distance, in inches");
    r_dof_bias = _Dvar_RegisterFloat(
                                 "r_dof_bias",
                                 0.5,
                                 0.1,
                                 3.0,
                                 0x4080u,
                                 "Depth of field bias as a power function (like gamma); less than 1 is sharper");
    r_dof_showdebug = _Dvar_RegisterBool("r_dof_showdebug", 0, 0x80u, "displays DOF parameter debug");
    r_motionblur_enable = _Dvar_RegisterBool("r_motionblur_enable", 0, 0x1001u, "Enable the motion blur effect");
    r_motionblur_frameBased_enable = _Dvar_RegisterBool(
                                                                         "r_motionblur_frameBased_enable",
                                                                         0,
                                                                         0x1001u,
                                                                         "Enable object based motion blur effect");
    r_motionblur_numberOfSamples = _Dvar_RegisterFloat(
                                                                     "r_motionblur_numberOfSamples",
                                                                     1.0,
                                                                     1.0,
                                                                     100.0,
                                                                     0x1081u,
                                                                     "Tweak dev var; sets the number of pixels sampled");
    r_motionblur_blurOrigin = _Dvar_RegisterVec4(
                                                            "r_motionblur_blurOrigin",
                                                            (0.5),
                                                            (0.5),
                                                            (0.0),
                                                            (1.0),
                                                            0.0,
                                                            1.0,
                                                            0x80u,
                                                            "Tweak dev var; origin of blur");
    r_motionblur_directionFactor = _Dvar_RegisterFloat(
                                                                     "r_motionblur_directionFactor",
                                                                     0.001,
                                                                     0.001,
                                                                     0.05f,
                                                                     0x1081u,
                                                                     "Tweak dev var; blur magnitude due to direction change");
    r_motionblur_positionFactor = _Dvar_RegisterFloat(
                                                                    "r_motionblur_positionFactor",
                                                                    0.0099999998,
                                                                    0.001,
                                                                    4.0,
                                                                    0x1081u,
                                                                    "Tweak dev var; blur magnitude due to position change");
    r_motionblur_maxblur = _Dvar_RegisterFloat(
                                                     "r_motionblur_maxblur",
                                                     30.0,
                                                     1.0,
                                                     40.0,
                                                     0x1001u,
                                                     "Caps the number of pixels sampled for blur");
    r_flameFX_enable = _Dvar_RegisterBool("r_flameFX_enable", 0, 1u, "Enable the flamethrower effect");
    r_flameFX_distortionScaleFactor = _Dvar_RegisterVec4(
                                                                            "r_flameFX_distortionScaleFactor",
                                                                            (0.0),
                                                                            (1.0),
                                                                            (1.0),
                                                                            (0.51191801),
                                                                            0.0,
                                                                            1.0,
                                                                            0x1081u,
                                                                            "Distortion uv scales (Default to 1)");
    r_flameFX_magnitude = _Dvar_RegisterFloat(
                                                    "r_flameFX_magnitude",
                                                    0.021514701,
                                                    0.0,
                                                    1.0,
                                                    0x1081u,
                                                    "Distortion magnitude");
    r_flameFX_FPS = _Dvar_RegisterInt("r_flameFX_FPS", 15, 1, 60, 0x1081u, "fire frames per sec");
    r_flameFX_fadeDuration = _Dvar_RegisterFloat(
                                                         "r_flameFX_fadeDuration",
                                                         0.5,
                                                         0.0,
                                                         15.0,
                                                         0x1001u,
                                                         "Sets fade duration in seconds");
    r_waterSheetingFX_allowed = _Dvar_RegisterBool("r_waterSheetingFX_allowed", 1, 1u, "Enable the water sheeting effect");
    r_waterSheetingFX_enable = _Dvar_RegisterBool("r_waterSheetingFX_enable", 0, 1u, "Enable the water sheeting effect");
    r_waterSheetingFX_distortionScaleFactor = _Dvar_RegisterVec4(
                                                                                            "r_waterSheetingFX_distortionScaleFactor",
                                                                                            (0.021961),
                                                                                            (1.0),
                                                                                            (0.0),
                                                                                            (0.0),
                                                                                            0.0,
                                                                                            1.0,
                                                                                            1u,
                                                                                            "Distortion uv scales (Default to 1)");
    r_waterSheetingFX_magnitude = _Dvar_RegisterFloat(
                                                                    "r_waterSheetingFX_magnitude",
                                                                    0.065538801,
                                                                    0.0,
                                                                    1.0,
                                                                    1u,
                                                                    "Distortion magnitude");
    r_waterSheetingFX_radius = _Dvar_RegisterFloat(
                                                             "r_waterSheetingFX_radius",
                                                             4.4405098,
                                                             0.0,
                                                             32.0,
                                                             1u,
                                                             "Tweak dev var; Glow radius in pixels at 640x480");
    r_waterSheetingFX_fadeDuration = _Dvar_RegisterFloat(
                                                                         "r_waterSheetingFX_fadeDuration",
                                                                         2.0,
                                                                         0.0,
                                                                         15.0,
                                                                         1u,
                                                                         "Sets fade duration in seconds");
    r_reviveFX_debug = _Dvar_RegisterBool("r_reviveFX_debug", 0, 1u, "Enable the revive effect debug");
    r_reviveFX_fadeDuration = _Dvar_RegisterFloat(
                                                            "r_reviveFX_fadeDuration",
                                                            5.0,
                                                            0.0,
                                                            15.0,
                                                            0x1001u,
                                                            "Sets fade duration in seconds");
    r_reviveFX_edgeColorTemp = _Dvar_RegisterFloat(
                                                             "r_reviveFX_edgeColorTemp",
                                                             6500.0,
                                                             1650.0,
                                                             25000.0,
                                                             0x5000u,
                                                             "edge color temp");
    r_reviveFX_edgeSaturation = _Dvar_RegisterFloat(
                                                                "r_reviveFX_edgeSaturation",
                                                                1.0,
                                                                0.0,
                                                                2.0,
                                                                0x5000u,
                                                                "edge saturation");
    r_reviveFX_edgeScale = _Dvar_RegisterVec3(
                                                     "r_reviveFX_edgeScale",
                                                     (1.0),
                                                     (1.0),
                                                     (1.0),
                                                     0.0,
                                                     2.0,
                                                     0x5000u,
                                                     "edge scale (tint) adjust RGB");
    r_reviveFX_edgeContrast = _Dvar_RegisterVec3(
                                                            "r_reviveFX_edgeContrast",
                                                            (1.0),
                                                            (1.0),
                                                            (1.0),
                                                            0.0,
                                                            2.0,
                                                            0x5000u,
                                                            "edge contrast adjust RGB");
    r_reviveFX_edgeOffset = _Dvar_RegisterVec3(
                                                        "r_reviveFX_edgeOffset",
                                                        (0.0),
                                                        (0.0),
                                                        (0.0),
                                                        -2.0,
                                                        2.0,
                                                        0x5000u,
                                                        "edge tv style brightness adjust RGB");
    r_reviveFX_edgeAmount = _Dvar_RegisterFloat("r_reviveFX_edgeAmount", 0.0, 0.0, 1.0, 0x5000u, "edge effect amount");
    r_reviveFX_edgeMaskAdjust = _Dvar_RegisterFloat(
                                                                "r_reviveFX_edgeMaskAdjust",
                                                                0.0,
                                                                -1.0,
                                                                1.0,
                                                                0x5000u,
                                                                "edge mask expansion contraction");
    r_genericFilter_enable = _Dvar_RegisterBool("r_genericFilter_enable", 0, 0x4000u, "Enable the genericFilter material");
    r_superFlare_enable = _Dvar_RegisterBool("r_superFlare_enable", 1, 0x4000u, "Enable the superFlare effect");
    r_superFlare_debug = _Dvar_RegisterBool(
                                                 "r_superFlare_debug",
                                                 0,
                                                 0x4000u,
                                                 "Enable the superFlare debug occlusion circles");
    r_poisonFX_debug_enable = _Dvar_RegisterBool("r_poisonFX_debug_enable", 0, 0x4000u, "Enable the poison effect");
    r_poisonFX_debug_amount = _Dvar_RegisterFloat(
                                                            "r_poisonFX_debug_amount",
                                                            1.0,
                                                            0.0,
                                                            1.0,
                                                            0x4000u,
                                                            "Poison effect amount");
    r_poisonFX_pulse = _Dvar_RegisterFloat("r_poisonFX_pulse", 1.1, 1.0, 2.0, 0x4000u, "pulse rate for distortion");
    r_poisonFX_warpX = _Dvar_RegisterFloat(
                                             "r_poisonFX_warpX",
                                             0.2,
                                             -0.30000001,
                                             0.30000001,
                                             0x4000u,
                                             "amount of distortion in x");
    r_poisonFX_warpY = _Dvar_RegisterFloat(
                                             "r_poisonFX_warpY",
                                             -0.15000001,
                                             -0.30000001,
                                             0.30000001,
                                             0x4000u,
                                             "amount of distortion in y");
    r_poisonFX_dvisionA = _Dvar_RegisterFloat("r_poisonFX_dvisionA", 0.0, -360.0, 360.0, 0x4000u, "double vision angle");
    r_poisonFX_dvisionX = _Dvar_RegisterFloat(
                                                    "r_poisonFX_dvisionX",
                                                    0.0,
                                                    -20.0,
                                                    20.0,
                                                    0x4000u,
                                                    "amount of double vision x");
    r_poisonFX_dvisionY = _Dvar_RegisterFloat(
                                                    "r_poisonFX_dvisionY",
                                                    0.0,
                                                    -20.0,
                                                    20.0,
                                                    0x4000u,
                                                    "amount of double vision y");
    r_poisonFX_blurMin = _Dvar_RegisterFloat("r_poisonFX_blurMin", 0.0, 0.0, 20.0, 0x5000u, "blur min");
    r_poisonFX_blurMax = _Dvar_RegisterFloat("r_poisonFX_blurMax", 4.0, 0.0, 20.0, 0x5000u, "blur max");
    r_showHDRalpha = _Dvar_RegisterBool("r_showHDRalpha", 0, 0x4000u, "debug display expensive HDR alphas");
    r_fullHDRrendering = _Dvar_RegisterInt("r_fullHDRrendering", 0, 0, 2, 0x4000u, "enable full HDR rendering");
    r_lutvar_0 = _Dvar_RegisterVec4(
                                 "r_lutvar_0",
                                 (0.0),
                                 (0.0),
                                 (0.0),
                                 (1.0),
                                 -100.0,
                                 100.0,
                                 0x4000u,
                                 "lut data");
    r_lutvar_1 = _Dvar_RegisterVec4(
                                 "r_lutvar_1",
                                 (0.0),
                                 (0.0),
                                 (0.0),
                                 (1.0),
                                 -100.0,
                                 100.0,
                                 0x4000u,
                                 "lut data");
    r_lutvar_2 = _Dvar_RegisterVec4(
                                 "r_lutvar_2",
                                 (0.0),
                                 (0.0),
                                 (0.0),
                                 (1.0),
                                 -100.0,
                                 100.0,
                                 0x4000u,
                                 "lut data");
    r_lutvar_3 = _Dvar_RegisterVec4(
                                 "r_lutvar_3",
                                 (0.0),
                                 (0.0),
                                 (0.0),
                                 (1.0),
                                 -100.0,
                                 100.0,
                                 0x4000u,
                                 "lut data");
    r_lutvar_4 = _Dvar_RegisterVec4(
                                 "r_lutvar_4",
                                 (0.0),
                                 (0.0),
                                 (0.0),
                                 (1.0),
                                 -100.0,
                                 100.0,
                                 0x4000u,
                                 "lut data");
    r_lutvar_5 = _Dvar_RegisterVec4(
                                 "r_lutvar_5",
                                 (0.0),
                                 (0.0),
                                 (0.0),
                                 (1.0),
                                 -100.0,
                                 100.0,
                                 0x4000u,
                                 "lut data");
    r_lutvar_6 = _Dvar_RegisterVec4(
                                 "r_lutvar_6",
                                 (0.0),
                                 (0.0),
                                 (0.0),
                                 (1.0),
                                 -100.0,
                                 100.0,
                                 0x4000u,
                                 "lut data");
    r_lutvar_7 = _Dvar_RegisterVec4(
                                 "r_lutvar_7",
                                 (0.0),
                                 (0.0),
                                 (0.0),
                                 (1.0),
                                 -100.0,
                                 100.0,
                                 0x4000u,
                                 "lut data");
    r_lutvar_8 = _Dvar_RegisterVec4(
                                 "r_lutvar_8",
                                 (0.0),
                                 (0.0),
                                 (0.0),
                                 (1.0),
                                 -100.0,
                                 100.0,
                                 0x4000u,
                                 "lut data");
    r_lutvar_9 = _Dvar_RegisterVec4(
                                 "r_lutvar_9",
                                 (0.0),
                                 (0.0),
                                 (0.0),
                                 (1.0),
                                 -100.0,
                                 100.0,
                                 0x4000u,
                                 "lut data");
    r_lutvar_10 = _Dvar_RegisterVec4(
                                    "r_lutvar_10",
                                    (0.0),
                                    (0.0),
                                    (0.0),
                                    (1.0),
                                    -100.0,
                                    100.0,
                                    0x4000u,
                                    "lut data");
    r_lutvar_11 = _Dvar_RegisterVec4(
                                    "r_lutvar_11",
                                    (0.0),
                                    (0.0),
                                    (0.0),
                                    (1.0),
                                    -100.0,
                                    100.0,
                                    0x4000u,
                                    "lut data");
    r_lutvar_12 = _Dvar_RegisterVec4(
                                    "r_lutvar_12",
                                    (0.0),
                                    (0.0),
                                    (0.0),
                                    (1.0),
                                    -100.0,
                                    100.0,
                                    0x4000u,
                                    "lut data");
    r_lutvar_13 = _Dvar_RegisterVec4(
                                    "r_lutvar_13",
                                    (0.0),
                                    (0.0),
                                    (0.0),
                                    (1.0),
                                    -100.0,
                                    100.0,
                                    0x4000u,
                                    "lut data");
    r_lutvar_14 = _Dvar_RegisterVec4(
                                    "r_lutvar_14",
                                    (0.0),
                                    (0.0),
                                    (0.0),
                                    (1.0),
                                    -100.0,
                                    100.0,
                                    0x4000u,
                                    "lut data");
    r_lutvar_15 = _Dvar_RegisterVec4(
                                    "r_lutvar_15",
                                    (0.0),
                                    (0.0),
                                    (0.0),
                                    (1.0),
                                    -100.0,
                                    100.0,
                                    0x4000u,
                                    "lut data");
    r_exposureTweak = _Dvar_RegisterBool("r_exposureTweak", 0, 0x5000u, "enable the exposure dvar tweak");
    r_exposureValue = _Dvar_RegisterFloat("r_exposureValue", 1.0, 0.0, 16.0, 0x5000u, "exposure");
    r_bloomTweaks = _Dvar_RegisterBool("r_bloomTweaks", 0, 0x5000u, "enbale bloom tweaks");
    r_bloomBlurRadius = _Dvar_RegisterFloat("r_bloomBlurRadius", 1.75, 1.0, 3.0, 0x5000u, "bloom blur radius");
    r_bloomTintWeights = _Dvar_RegisterVec4(
                                                 "r_bloomTintWeights",
                                                 (0.25),
                                                 (0.5),
                                                 (0.25),
                                                 (0.0),
                                                 0.0,
                                                 1.0,
                                                 0x5000u,
                                                 "tint weights RGB, desaturation amount");
    r_bloomColorScale = _Dvar_RegisterVec4(
                                                "r_bloomColorScale",
                                                (0.0),
                                                (0.0),
                                                (0.0),
                                                (0.0),
                                                0.0,
                                                8.0,
                                                0x5000u,
                                                "color scale RGB, desaturation start");
    r_bloomTintScale = _Dvar_RegisterVec4(
                                             "r_bloomTintScale",
                                             (0.0),
                                             (0.0),
                                             (0.0),
                                             (1.0),
                                             0.0,
                                             8.0,
                                             0x5000u,
                                             "tint scale RGB, desaturation end");
    r_bloomCurveBreakpoint = _Dvar_RegisterVec4(
                                                         "r_bloomCurveBreakpoint",
                                                         (1.0),
                                                         (1.0),
                                                         (1.0),
                                                         (1.0),
                                                         0.0,
                                                         8.0,
                                                         0x5000u,
                                                         "lo hi curve breakpoint RGBT");
    r_bloomCurveLoBlack = _Dvar_RegisterVec4(
                                                    "r_bloomCurveLoBlack",
                                                    (0.0),
                                                    (0.0),
                                                    (0.0),
                                                    (0.0),
                                                    0.0,
                                                    1.0,
                                                    0x5000u,
                                                    "curve lo black point RGBT");
    r_bloomCurveLoGamma = _Dvar_RegisterVec4(
                                                    "r_bloomCurveLoGamma",
                                                    (1.0),
                                                    (1.0),
                                                    (1.0),
                                                    (1.0),
                                                    0.1,
                                                    9.9899998,
                                                    0x5000u,
                                                    "curve lo gamma ramp RGBT");
    r_bloomCurveLoWhite = _Dvar_RegisterVec4(
                                                    "r_bloomCurveLoWhite",
                                                    (1.0),
                                                    (1.0),
                                                    (1.0),
                                                    (1.0),
                                                    0.0,
                                                    1.0,
                                                    0x5000u,
                                                    "curve lo white point RGBT");
    r_bloomCurveLoRemapBlack = _Dvar_RegisterVec4(
                                                             "r_bloomCurveLoRemapBlack",
                                                             (0.0),
                                                             (0.0),
                                                             (0.0),
                                                             (0.0),
                                                             0.0,
                                                             1.0,
                                                             0x5000u,
                                                             "curve lo output black point RGBT");
    r_bloomCurveLoRemapWhite = _Dvar_RegisterVec4(
                                                             "r_bloomCurveLoRemapWhite",
                                                             (1.0),
                                                             (1.0),
                                                             (1.0),
                                                             (1.0),
                                                             0.0,
                                                             1.0,
                                                             0x5000u,
                                                             "curve lo output white point RGBT");
    r_bloomCurveHiBlack = _Dvar_RegisterVec4(
                                                    "r_bloomCurveHiBlack",
                                                    (0.0),
                                                    (0.0),
                                                    (0.0),
                                                    (0.0),
                                                    0.0,
                                                    1.0,
                                                    0x5000u,
                                                    "curve hi black point RGBT");
    r_bloomCurveHiGamma = _Dvar_RegisterVec4(
                                                    "r_bloomCurveHiGamma",
                                                    (1.0),
                                                    (1.0),
                                                    (1.0),
                                                    (1.0),
                                                    0.1,
                                                    9.9899998,
                                                    0x5000u,
                                                    "curve hi gamma ramp RGBT");
    r_bloomCurveHiWhite = _Dvar_RegisterVec4(
                                                    "r_bloomCurveHiWhite",
                                                    (1.0),
                                                    (1.0),
                                                    (1.0),
                                                    (1.0),
                                                    0.0,
                                                    1.0,
                                                    0x5000u,
                                                    "curve hi white point RGBT");
    r_bloomCurveHiRemapBlack = _Dvar_RegisterVec4(
                                                             "r_bloomCurveHiRemapBlack",
                                                             (0.0),
                                                             (0.0),
                                                             (0.0),
                                                             (0.0),
                                                             0.0,
                                                             1.0,
                                                             0x5000u,
                                                             "curve hi output black point RGBT");
    r_bloomCurveHiRemapWhite = _Dvar_RegisterVec4(
                                                             "r_bloomCurveHiRemapWhite",
                                                             (1.0),
                                                             (1.0),
                                                             (1.0),
                                                             (1.0),
                                                             0.0,
                                                             1.0,
                                                             0x5000u,
                                                             "curve hi output white point RGBT");
    r_bloomExpansionControl = _Dvar_RegisterVec4(
                                                            "r_bloomExpansionControl",
                                                            (0.5),
                                                            (0.5),
                                                            (0.5),
                                                            (0.5),
                                                            0.0,
                                                            1.0,
                                                            0x5000u,
                                                            "expansion amount x&y and expansion target x&y ");
    r_bloomExpansionWeights = _Dvar_RegisterVec4(
                                                            "r_bloomExpansionWeights",
                                                            (0.0),
                                                            (0.0),
                                                            (0.0),
                                                            (0.0),
                                                            0.0,
                                                            1.0,
                                                            0x5000u,
                                                            "amount of expansion to lerp in RGBT");
    r_bloomExpansionSource = _Dvar_RegisterInt("r_bloomExpansionSource", 0, 0, 1, 0x5000u, "bloom expansion source point");
    r_bloomPersistence = _Dvar_RegisterFloat("r_bloomPersistence", 0.0, 0.0, 1.0, 0x5000u, "amount of bloom feedback");
    r_bloomStreakXLevels0 = _Dvar_RegisterVec4(
                                                        "r_bloomStreakXLevels0",
                                                        (0.25),
                                                        (0.5),
                                                        (0.25),
                                                        (1.0),
                                                        0.0,
                                                        9.9899998,
                                                        0x5000u,
                                                        "level weights RGB, level gamma");
    r_bloomStreakXLevels1 = _Dvar_RegisterVec4(
                                                        "r_bloomStreakXLevels1",
                                                        (0.0),
                                                        (1.0),
                                                        (0.0),
                                                        (1.0),
                                                        0.0,
                                                        1.0,
                                                        0x5000u,
                                                        "level in black white, out black white");
    r_bloomStreakXInnerTint = _Dvar_RegisterVec3(
                                                            "r_bloomStreakXInnerTint",
                                                            (1.0),
                                                            (1.0),
                                                            (1.0),
                                                            0.0,
                                                            1.0,
                                                            0x5000u,
                                                            "streak inner tint");
    r_bloomStreakXOuterTint = _Dvar_RegisterVec3(
                                                            "r_bloomStreakXOuterTint",
                                                            (1.0),
                                                            (1.0),
                                                            (1.0),
                                                            0.0,
                                                            1.0,
                                                            0x5000u,
                                                            "streak outer tint");
    r_bloomStreakXTintControl = _Dvar_RegisterVec4(
                                                                "r_bloomStreakXTintControl",
                                                                (0.25),
                                                                (0.5),
                                                                (0.25),
                                                                (1.0),
                                                                0.0,
                                                                1.0,
                                                                0x5000u,
                                                                "streak saturation weights, saturation");
    r_bloomStreakXTint = _Dvar_RegisterVec3(
                                                 "r_bloomStreakXTint",
                                                 (0.0),
                                                 (0.0),
                                                 (0.0),
                                                 0.0,
                                                 2.0,
                                                 0x5000u,
                                                 "streak tint");
    r_bloomStreakYLevels0 = _Dvar_RegisterVec4(
                                                        "r_bloomStreakYLevels0",
                                                        (0.25),
                                                        (0.5),
                                                        (0.25),
                                                        (1.0),
                                                        0.0,
                                                        9.9899998,
                                                        0x5000u,
                                                        "level weights RGB, level gamma");
    r_bloomStreakYLevels1 = _Dvar_RegisterVec4(
                                                        "r_bloomStreakYLevels1",
                                                        (0.0),
                                                        (1.0),
                                                        (0.0),
                                                        (1.0),
                                                        0.0,
                                                        1.0,
                                                        0x5000u,
                                                        "level in black white, out black white");
    r_bloomStreakYInnerTint = _Dvar_RegisterVec3(
                                                            "r_bloomStreakYInnerTint",
                                                            (1.0),
                                                            (1.0),
                                                            (1.0),
                                                            0.0,
                                                            1.0,
                                                            0x5000u,
                                                            "streak inner tint");
    r_bloomStreakYOuterTint = _Dvar_RegisterVec3(
                                                            "r_bloomStreakYOuterTint",
                                                            (1.0),
                                                            (1.0),
                                                            (1.0),
                                                            0.0,
                                                            1.0,
                                                            0x5000u,
                                                            "streak outer tint");
    r_bloomStreakYTintControl = _Dvar_RegisterVec4(
                                                                                                    "r_bloomStreakYTintControl",
                                                                                                    (0.25),
                                                                                                    (0.5),
                                                                                                    (0.25),
                                                                                                    (1.0),
                                                                                                    0.0,
                                                                                                    1.0,
                                                                                                    0x5000u,
                                                                                                    "streak saturation weights, saturation");
    r_bloomStreakYTint = _Dvar_RegisterVec3(
                                                 "r_bloomStreakYTint",
                                                 (0.0),
                                                 (0.0),
                                                 (0.0),
                                                 0.0,
                                                 2.0,
                                                 0x5000u,
                                                 "streak tint");
    r_finalShiftX = _Dvar_RegisterVec3(
                                        "r_finalShiftX",
                                        (0.0),
                                        (0.0),
                                        (0.0),
                                        -5.0,
                                        5.0,
                                        0x5000u,
                                        "r,g,b channel shift x");
    r_finalShiftY = _Dvar_RegisterVec3(
                                        "r_finalShiftY",
                                        (0.0),
                                        (0.0),
                                        (0.0),
                                        -5.0,
                                        5.0,
                                        0x5000u,
                                        "r,g,b channel shift y");
    r_primaryLightUseTweaks = _Dvar_RegisterBool(
                                                            "r_primaryLightUseTweaks",
                                                            0,
                                                            0x80u,
                                                            "Use primary light intensity tweaks");
    r_primaryLightTweakDiffuseStrength = _Dvar_RegisterFloat(
                                                                                 "r_primaryLightTweakDiffuseStrength",
                                                                                 1.0,
                                                                                 0.0,
                                                                                 10.0,
                                                                                 0x80u,
                                                                                 "Tweak the diffuse intensity for primary lights");
    r_primaryLightTweakSpecularStrength = _Dvar_RegisterFloat(
                                                                                    "r_primaryLightTweakSpecularStrength",
                                                                                    1.0,
                                                                                    0.0,
                                                                                    10.0,
                                                                                    0x80u,
                                                                                    "Tweak the specular intensity for primary lights");
    r_lightGridEnableTweaks = _Dvar_RegisterBool(
                                                            "r_lightGridEnableTweaks",
                                                            0,
                                                            0x1100u,
                                                            "Enable tweaks of the light color from the light grid");
    r_lightGridUseTweakedValues = _Dvar_RegisterBool(
                                                                    "r_lightGridUseTweakedValues",
                                                                    1,
                                                                    0,
                                                                    "Use tweaked values instead of default");
    r_lightGridIntensity = _Dvar_RegisterFloat(
                                                     "r_lightGridIntensity",
                                                     1.0,
                                                     0.0,
                                                     2.0,
                                                     0x1100u,
                                                     "Adjust the intensity of light color from the light grid");
    r_lightGridContrast = _Dvar_RegisterFloat(
                                                    "r_lightGridContrast",
                                                    0.0,
                                                    -1.0,
                                                    1.0,
                                                    0x1100u,
                                                    "Adjust the contrast of light color from the light grid");
    r_heroLighting = _Dvar_RegisterBool("r_heroLighting", 1, 0, "Enable hero-only lighting");
    r_heroLightSaturation = _Dvar_RegisterFloat(
                                                        "r_heroLightSaturation",
                                                        1.0,
                                                        0.0,
                                                        1.0,
                                                        0x4000u,
                                                        "hero diffuse light saturation");
    r_heroLightColorTemp = _Dvar_RegisterFloat(
                                                     "r_heroLightColorTemp",
                                                     6500.0,
                                                     1650.0,
                                                     25000.0,
                                                     0x4000u,
                                                     "hero diffuse light temp");
    r_heroLightScale = _Dvar_RegisterVec3(
                                             "r_heroLightScale",
                                             (1.0),
                                             (1.0),
                                             (1.0),
                                             0.0,
                                             2.0,
                                             0x5000u,
                                             "hero diffuse light scale");
    r_enableDlights = _Dvar_RegisterBool("r_enableDlights", 1, 0x4000u, "toggle multipass dlights rendering");
    r_debugHDRdlight = _Dvar_RegisterInt("r_debugHDRdlight", 0, 0, 4, 0x4000u, "debug dlights mode");
    r_debugHDRdlight_scale = _Dvar_RegisterFloat("r_debugHDRdlight_scale", 1.0, 1.0, 64.0, 0x4000u, "dlights color scale");
    r_debugHDRdlight_distance = _Dvar_RegisterFloat(
                                                                "r_debugHDRdlight_distance",
                                                                250.0,
                                                                0.0,
                                                                500.0,
                                                                0x4000u,
                                                                "distance dlights appear around camera");
    r_debugHDRdlight_radius = _Dvar_RegisterFloat(
                                                            "r_debugHDRdlight_radius",
                                                            150.0,
                                                            0.0,
                                                            500.0,
                                                            0x4000u,
                                                            "dlights falloff radius");
    r_debugHDRdlight_color = _Dvar_RegisterVec3(
                                                         "r_debugHDRdlight_color",
                                                         (0.25),
                                                         (0.80000001),
                                                         (1.0),
                                                         0.0,
                                                         1.0,
                                                         0x4000u,
                                                         "dlights debug color");
    r_num_viewports = _Dvar_RegisterInt("r_num_viewports", 0, 0, 4, 0, "number of viewports to expect to render");
    r_pix_material = _Dvar_RegisterBool("r_pix_material", 1, 0, "Enable pix material names");
    r_outdoorAwayBias = _Dvar_RegisterFloat(
                                                "r_outdoorAwayBias",
                                                32.0,
                                                -3.4028235e38,
                                                3.4028235e38,
                                                0x1000u,
                                                "Affects the height map lookup for making sure snow doesn't go indoors");
    r_outdoorDownBias = _Dvar_RegisterFloat(
                                                "r_outdoorDownBias",
                                                0.0,
                                                -3.4028235e38,
                                                3.4028235e38,
                                                0x1000u,
                                                "Affects the height map lookup for making sure snow doesn't go indoors");
    r_outdoorFeather = _Dvar_RegisterFloat(
                                             "r_outdoorFeather",
                                             8.0,
                                             -3.4028235e38,
                                             3.4028235e38,
                                             0x1000u,
                                             "Outdoor z-feathering value");
    Dvar_SetModified(r_outdoorFeather);
    r_sun_from_dvars = _Dvar_RegisterBool(
                                             "r_sun_from_dvars",
                                             0,
                                             0x80u,
                                             "Set sun flare values from dvars rather than the level");
#ifdef _DEBUG // LWSS ADD ( why are there 3 of these )
    developer = _Dvar_RegisterInt("developer", 1, 0, 2, 0, "Enable development environment");
#else
    developer = _Dvar_RegisterInt("developer", 0, 0, 2, 0, "Enable development environment");
#endif
    sv_cheats = _Dvar_RegisterBool("sv_cheats", 1, 0x48u, "Allow server side cheats");
    com_statmon = _Dvar_RegisterBool("com_statmon", 0, 0, "Draw stats monitor");
    r_sse_skinning = _Dvar_RegisterBool("r_sse_skinning", 1, 0, "Use Streaming SIMD Extensions for skinning");
    r_monitor = _Dvar_RegisterInt(
                                "r_monitor",
                                0,
                                0,
                                8,
                                0x21u,
                                "Index of the monitor to use in a multi monitor system; 0 picks automatically.");
    r_aspectRatio = _Dvar_RegisterEnum(
                                        "r_aspectRatio",
                                        s_aspectRatioNames,
                                        0,
                                        0x21u,
                                        "Screen aspect ratio.    Most widescreen monitors are 16:10 instead of 16:9.");
    r_customMode = _Dvar_RegisterString(
                                     "r_customMode",
                                     (char *)"",
                                     0x21u,
                                     "Special resolution mode for the remote debugger");
    r_open_automate = _Dvar_RegisterBool("r_open_automate", 0, 0, "Enable Nvidia Open Automate testing");
    r_gfxopt_dynamic_foliage = _Dvar_RegisterBool(
                                                             "r_gfxopt_dynamic_foliage",
                                                             1,
                                                             1u,
                                                             "Enable tree rustle, grass wind react etc");
    r_gfxopt_water_simulation = _Dvar_RegisterBool("r_gfxopt_water_simulation", 1, 1u, "Enable advanced water simulation");
    r_allow_intz = _Dvar_RegisterBool("r_allow_intz", 1, 1u, "Enable support for the intz depth buffer format.");
    r_allow_null_rt = _Dvar_RegisterBool("r_allow_null_rt", 1, 1u, "Enable support for the null render target format.");
    r_use_driver_convergence = _Dvar_RegisterBool(
                                                             "r_use_driver_convergence",
                                                             0,
                                                             1u,
                                                             "Use the driver convergence values instead of the game defined values.");
    r_convergence = _Dvar_RegisterFloat("r_convergence", 6.06253, -3.4028235e38, 3.4028235e38, 0, "Stereo convergence.");
    // LWSS: changed to 1 by default (2011 BLOPS default)
    r_multithreaded_device = _Dvar_RegisterBool("r_multithreaded_device", 1, 0x21u, "Create a multithreaded d3d device.");
    r_warm_dpvs = _Dvar_RegisterBool("r_warm_dpvs", 0, 0, "shader warming: disable dpvs culling");
    r_warm_bsp = _Dvar_RegisterBool("r_warm_bsp", 0, 0, "shader warming: bsp objects");
    r_warm_static = _Dvar_RegisterBool("r_warm_static", 0, 0, "shader warming: static objects");
    r_warm_dobj = _Dvar_RegisterBool("r_warm_dobj", 0, 0, "shader warming: d objects");
    R_WarnInitDvars();
    r_staticModelDumpLodInfo = _Dvar_RegisterBool(
                                                             "r_staticModelDumpLodInfo",
                                                             0,
                                                             0,
                                                             "Dump static model info for the next frame.");
    R_ReflectionProbeRegisterDvars();
    r_grassBurn = _Dvar_RegisterFloat("r_grassBurn", 0.0, -1.0, 0.0, 0, "test");
    r_grassEnable = _Dvar_RegisterBool("r_grassEnable", 1, 0, "Diables grass movement");
    r_skyTransition = _Dvar_RegisterFloat("r_skyTransition", 0.0, 0.0, 1.0, 0x1000u, "Sky transition blend factor.");
    r_treeScale = _Dvar_RegisterInt("r_treeScale", 0, 0, 1, 0x1000u, "Envmap Scale.");
    r_testScale = _Dvar_RegisterFloat("r_testScale", 0.0, 0.0, 1.0, 0x1000u, "Specular Scale.");
    r_skyColorTemp = _Dvar_RegisterFloat("r_skyColorTemp", 6500.0, 1650.0, 25000.0, 0x1000u, "Sky box color temp.");
    r_debugLayers = _Dvar_RegisterBool("r_debugLayers", 0, 0x80u, "Highlight Alpha Blended Polygons.");
    r_burnedDestructibleColor = _Dvar_RegisterFloat(
                                                                "r_burnedDestructibleColor",
                                                                0.40000001,
                                                                0.0,
                                                                1.0,
                                                                0,
                                                                "color of burned destructibles");
    r_swrk_override_enable = _Dvar_RegisterBool("r_swrk_override_enable", 0, 0, "Enable shaderworks overrides");
    r_swrk_override_characterCharredAmount = _Dvar_RegisterFloat(
                                                                                         "r_swrk_override_characterCharredAmount",
                                                                                         0.0,
                                                                                         0.0,
                                                                                         1.0,
                                                                                         0x80u,
                                                                                         "overrides the __characterCharredAmount Shaderworks tweak");
    r_swrk_override_characterDissolveColor = _Dvar_RegisterVec4(
                                                                                         "r_swrk_override_characterDissolveColor",
                                                                                         (0.0),
                                                                                         (0.0),
                                                                                         (0.0),
                                                                                         (0.0),
                                                                                         0.0,
                                                                                         10.0,
                                                                                         0x80u,
                                                                                         "overrides the __characterDissolveColor Shaderworks tweak");
    r_swrk_override_wetness = _Dvar_RegisterFloat(
                                                            "r_swrk_override_wetness",
                                                            0.0,
                                                            0.0,
                                                            1.0,
                                                            0x80u,
                                                            "Overrides the character wetness.");
    r_debugShowPrimaryLights = _Dvar_RegisterBool(
                                                             "r_debugShowPrimaryLights",
                                                             0,
                                                             0x80u,
                                                             "shows location of primary lights");
    r_debugShowDynamicLights = _Dvar_RegisterBool(
                                                             "r_debugShowDynamicLights",
                                                             0,
                                                             0x80u,
                                                             "shows location of dynamic lights");
    r_debugShowCoronas = _Dvar_RegisterBool("r_debugShowCoronas", 0, 0x80u, "shows location of coronas");
    r_use_separate_char_tech = _Dvar_RegisterBool(
                                                             "r_use_separate_char_tech",
                                                             0,
                                                             0x80u,
                                                             "enables the optimization of a separate charred material tech");
    r_drawDebugFogParams = _Dvar_RegisterBool("r_drawDebugFogParams", 0, 0x80u, "enables the fog debug display");
    r_debugShowPrimaryLightLines = _Dvar_RegisterEnum(
                                                                     "r_debugShowPrimaryLightLines",
                                                                     showPrimaryLightLinesNames,
                                                                     0,
                                                                     0x80u,
                                                                     "show model primary light lines");
    r_seethru_decal_enable = _Dvar_RegisterBool(
                                                         "r_seethru_decal_enable",
                                                         !r_glob.isMultiplayer,
                                                         0x1080u,
                                                         "Toggles see-through impact holes");
    r_tension_enable = _Dvar_RegisterBool("r_tension_enable", 1, 0x1080u, "Toggles tension");
    _Dvar_RegisterVec4(
        "r_MaterialParameterTweak",
        (0.0),
        (0.0),
        (0.0),
        (0.0),
        0.0,
        1.0,
        0x280u,
        "Modifies currently selected material parameter");
    r_ui3d_debug_display = _Dvar_RegisterBool("r_ui3d_debug_display", 0, 0x80u, "Show UI3D debug overlay");
    r_ui3d_use_debug_values = _Dvar_RegisterInt("r_ui3d_use_debug_values", -1, -1, 8, 0x80u, "Use UI debug values");
    r_ui3d_x = _Dvar_RegisterFloat("r_ui3d_x", 0.0, 0.0, 1.0, 0x80u, "ui3d texture window x");
    r_ui3d_y = _Dvar_RegisterFloat("r_ui3d_y", 0.0, 0.0, 1.0, 0x80u, "ui3d texture window y");
    r_ui3d_w = _Dvar_RegisterFloat("r_ui3d_w", 1.0, 0.0, 1.0, 0x80u, "ui3d texture window width");
    r_ui3d_h = _Dvar_RegisterFloat(
                                                                                                    "r_ui3d_h",
                                                                                                    1.0,
                                                                                                    0.0,
                                                                                                    1.0,
                                                                                                    0x80u,
                                                                                                    "ui3d texture window height");
    r_missile_cam_debug_display = _Dvar_RegisterInt(
                                                                    "r_missile_cam_debug_display",
                                                                    0,
                                                                    0,
                                                                    10,
                                                                    0x80u,
                                                                    "Show missile camera debug overlay");
    r_extracam_custom_aspectratio = _Dvar_RegisterFloat(
                                                                        "r_extracam_custom_aspectratio",
                                                                        -1.0,
                                                                        -1.0,
                                                                        5.0,
                                                                        0x1000u,
                                                                        "custom extracam aspect ratio (width/height). Set to -1 to use default aspect ratio");
    r_extracam_shadowmap_enable = _Dvar_RegisterBool(
                                                                    "r_extracam_shadowmap_enable",
                                                                    0,
                                                                    0x1000u,
                                                                    "enables extra cam shadowmapping");
    r_shader_constant_set_debug_range = _Dvar_RegisterFloat(
                                                                                "r_shader_constant_set_debug_range",
                                                                                -1.0,
                                                                                -2.0,
                                                                                2000.0,
                                                                                0x80u,
                                                                                "How far to draw scs debug view");
    r_shader_constant_set_enable = _Dvar_RegisterBool(
                                                                     "r_shader_constant_set_enable",
                                                                     1,
                                                                     0x80u,
                                                                     "enables shader constant sets");
    r_sky_intensity_showDebugDisplay = _Dvar_RegisterBool(
                                                                             "r_sky_intensity_showDebugDisplay",
                                                                             0,
                                                                             0x80u,
                                                                             "Enables sky intensity debugging display");
    r_sky_intensity_useDebugValues = _Dvar_RegisterBool(
                                                                         "r_sky_intensity_useDebugValues",
                                                                         0,
                                                                         0x80u,
                                                                         "use dvar values for sky intensity");
    r_sky_intensity_angle0 = _Dvar_RegisterFloat(
                                                         "r_sky_intensity_angle0",
                                                         90.0,
                                                         -90.0,
                                                         90.0,
                                                         0x80u,
                                                         "angle0 of sky intensity");
    r_sky_intensity_angle1 = _Dvar_RegisterFloat(
                                                         "r_sky_intensity_angle1",
                                                         90.0,
                                                         -90.0,
                                                         90.0,
                                                         0x80u,
                                                         "angle1 of sky intensity");
    r_sky_intensity_factor0 = _Dvar_RegisterFloat(
                                                            "r_sky_intensity_factor0",
                                                            1.0,
                                                            0.0,
                                                            10.0,
                                                            0x80u,
                                                            "angle0 of sky intensity");
    r_sky_intensity_factor1 = _Dvar_RegisterFloat(
                                                            "r_sky_intensity_factor1",
                                                            1.0,
                                                            0.0,
                                                            10.0,
                                                            0x80u,
                                                            "angle1 of sky intensity");
    r_fog_disable = _Dvar_RegisterBool("r_fog_disable", 0, 0x5000u, "Disables fog");
    r_grassWindForceEnable = _Dvar_RegisterBool(
                                                         "r_grassWindForceEnable",
                                                         1,
                                                         0x80u,
                                                         "Enables wind gusts that affect dynFoliage grass");
    r_dpvs_useCellForceInvisibleBits = _Dvar_RegisterBool(
                                                                             "r_dpvs_useCellForceInvisibleBits",
                                                                             1,
                                                                             0x80u,
                                                                             "turns on/off the script-driven cell vis bits");
    r_remotescreen_renderlastframe = _Dvar_RegisterBool(
                                                                         "r_remotescreen_renderlastframe",
                                                                         1,
                                                                         0x80u,
                                                                         "toggles if remote screen update draws the last frame");
    r_offscreenCasterLodScale = _Dvar_RegisterFloat(
                                                                "r_offscreenCasterLodScale",
                                                                2.5,
                                                                1.0,
                                                                5.0,
                                                                0x4000u,
                                                                "Off-screen shadow casters LOD distance scale");
    r_dynSModelBurnValueDisplayRange = _Dvar_RegisterFloat(
                                                                             "r_dynSModelBurnValueDisplayRange",
                                                                             -1.0,
                                                                             -1.0,
                                                                             1000.0,
                                                                             0x80u,
                                                                             "smodel's with in this range will show burn values");
    r_useHidePartbits = _Dvar_RegisterBool("r_useHidePartbits", 1, 0x80u, "toggles hide partbits usage");
}

const dvar_s *__cdecl R_RegisterWorkerThreadDvar(char *name, unsigned int workerIndex)
{
    const char *helpString; // [esp+0h] [ebp-Ch]
    bool defaultState; // [esp+7h] [ebp-5h]
    unsigned __int16 flags; // [esp+8h] [ebp-4h]

    flags = 0x4000;
    defaultState = 1;
    helpString = "Enable worker thread";
    if ( workerIndex >= Sys_GetDefaultWorkerThreadsCount() || !sys_smp_allowed->current.enabled )
    {
        flags = 64;
        defaultState = 0;
        helpString = "Worker thread always disabled; not enough hardware threads";
    }
    return _Dvar_RegisterBool(name, defaultState, flags, helpString);
}

char __cdecl R_CheckDvarModified(const dvar_s *dvar)
{
    if ( !dvar->modified )
        return 0;
    Dvar_ClearModified(dvar);
    return 1;
}

