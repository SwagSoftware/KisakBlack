#include "cg_compass.h"
#include <qcommon/common.h>
#include <cgame_mp/cg_newDraw_mp.h>
#include <client_mp/cl_cgame_mp.h>
#include <gfx_d3d/r_font.h>
#include <cgame_mp/cg_main_mp.h>
#include "cg_drawtools.h"
#include <gfx_d3d/r_rendercmds.h>
#include <ui/ui_atoms.h>
#include <game_mp/g_main_mp.h>
#include <win32/win_shared.h>

const dvar_t *compass;
const dvar_t *compassSize;
const dvar_t *compassMaxRange;
const dvar_t *compassMinRange;
const dvar_t *compassMinRadius;
const dvar_t *compassSoundPingFadeTime;
const dvar_t *compassClampIcons;
const dvar_t *compassFriendlyWidth;
const dvar_t *compassFriendlyHeight;
const dvar_t *compassPlayerWidth;
const dvar_t *compassPlayerHeight;
const dvar_t *compassCoords;
const dvar_t *compassECoordCutoff;
const dvar_t *compassRotation;
const dvar_t *compassTickertapeStretch;
const dvar_t *compassRadarPingFadeTime;
const dvar_t *compassRadarUpdateTime;
const dvar_t *compassRadarUpdateFastTime;
const dvar_t *compassSatelliteStaticImageFadeTime;
const dvar_t *compassSatellitePingFadeTime;
const dvar_t *compassStaticImageUpdateTime;
const dvar_t *compassSatelliteScanTime;
const dvar_t *compassRadarLineThickness;
const dvar_t *compassLocalRadarUpdateTime;
const dvar_t *compassLocalRadarRadius;
const dvar_t *compassObjectiveWidth;
const dvar_t *compassObjectiveHeight;
const dvar_t *compassObjectiveArrowWidth;
const dvar_t *compassObjectiveArrowHeight;
const dvar_t *compassObjectiveArrowOffset;
const dvar_t *compassObjectiveArrowRotateDist;
const dvar_t *compassObjectiveMaxRange;
const dvar_t *compassObjectiveMinAlpha;
const dvar_t *compassObjectiveNumRings;
const dvar_t *compassObjectiveRingTime;
const dvar_t *compassObjectiveRingSize;
const dvar_t *compassObjectiveTextScale;
const dvar_t *compassObjectiveTextHeight;
const dvar_t *compassObjectiveDrawLines;
const dvar_t *compassObjectiveIconWidth;
const dvar_t *compassObjectiveIconHeight;
const dvar_t *compassObjectiveIconWidthZombie;
const dvar_t *compassObjectiveIconHeightZombie;
const dvar_t *compassObjectiveNearbyDist;
const dvar_t *compassObjectiveMinDistRange;
const dvar_t *compassObjectiveDetailDist;
const dvar_t *compassObjectiveMinHeight;
const dvar_t *compassObjectiveMaxHeight;
const dvar_t *compassPartialType;
const dvar_t *compassSpectatorsSeeEnemies;
const dvar_t *compassEnemyFootstepMaxRange;
const dvar_t *compassEnemyFootstepMaxZ;
const dvar_t *compassEnemyFootstepMinSpeed;
const dvar_t *compassEnemyFootstepEnabled;
const dvar_t *compassShowEnemies;
const dvar_t *compassForcePlayerIcon;
const dvar_t *compassGridEnabled;
const dvar_t *compassGridRows;
const dvar_t *compassGridCols;
const dvar_t *compassGridAlign;
const dvar_t *compassScaleDuration;
const dvar_t *compassScaleDiff;
const dvar_t *compassScaleTimer;
const dvar_t *compassScaleReset;
const dvar_t *typeWriterCod7LetterFXTime;
const dvar_t *popInLetterFXTime;
const dvar_t *redactionDisplayTime;
const dvar_t *redactionFadeDuration;
const dvar_t *redactionStartStrokeTime;
const dvar_t *redactionStrokeTime;
const dvar_t *compassDebug;
const dvar_t *forceFullScreenMap;
const dvar_t *cg_showZombieMap;
const dvar_t *cg_hudMapRadarLineThickness;
const dvar_t *cg_hudMapFriendlyWidth;
const dvar_t *cg_hudMapFriendlyHeight;
const dvar_t *cg_hudMapPlayerWidth;
const dvar_t *cg_hudMapPlayerHeight;
const dvar_t *cg_hudMapBorderWidth;
const dvar_t *cg_showZombieControls;

bool gridPointStatus[9][9];

void __cdecl CG_CompassRegisterDvars()
{
    compass = _Dvar_RegisterBool("compass", 1, 0x1100u, "Display Compass");
    compassSize = _Dvar_RegisterFloat("compassSize", 1.0, 0.0, 3.4028235e38, 1u, "Scale the compass");
    compassMaxRange = _Dvar_RegisterFloat(
                                            "compassMaxRange",
                                            2500.0,
                                            0.000099999997,
                                            3.4028235e38,
                                            0x1081u,
                                            "The maximum range from the player in world space that objects will be shown on the compass");
    compassMinRange = _Dvar_RegisterFloat(
                                            "compassMinRange",
                                            0.000099999997,
                                            0.000099999997,
                                            3.4028235e38,
                                            1u,
                                            "The minimum range from the player in world space that objects will appear on the compass");
    compassMinRadius = _Dvar_RegisterFloat(
                                             "compassMinRadius",
                                             0.000099999997,
                                             0.000099999997,
                                             3.4028235e38,
                                             1u,
                                             "The minimum radius from the center of the compass that objects will appear.");
    compassSoundPingFadeTime = _Dvar_RegisterFloat(
                                                             "compassSoundPingFadeTime",
                                                             2.0,
                                                             0.0,
                                                             10.0,
                                                             0x81u,
                                                             "The time in seconds for the sound overlay on the compass to fade");
    compassClampIcons = _Dvar_RegisterBool(
                                                "compassClampIcons",
                                                1,
                                                0x81u,
                                                "If true, friendlies and enemy pings clamp to the edge of the radar.    If false, they disappear off the edge.");
    compassFriendlyWidth = _Dvar_RegisterFloat(
                                                     "compassFriendlyWidth",
                                                     18.75,
                                                     0.0,
                                                     3.4028235e38,
                                                     1u,
                                                     "The size of the friendly icon on the compass");
    compassFriendlyHeight = _Dvar_RegisterFloat(
                                                        "compassFriendlyHeight",
                                                        18.75,
                                                        0.0,
                                                        3.4028235e38,
                                                        1u,
                                                        "The size of the friendly icon on the compass");
    compassPlayerWidth = _Dvar_RegisterFloat(
                                                 "compassPlayerWidth",
                                                 18.75,
                                                 0.0,
                                                 3.4028235e38,
                                                 1u,
                                                 "The size of the player's icon on the compass");
    compassPlayerHeight = _Dvar_RegisterFloat(
                                                    "compassPlayerHeight",
                                                    18.75,
                                                    0.0,
                                                    3.4028235e38,
                                                    1u,
                                                    "The size of the player's icon on the compass");
    compassCoords = _Dvar_RegisterVec3(
                                        "compassCoords",
                                        (740.0),
                                        (3590.0),
                                        (400.0),
                                        0.0,
                                        3.4028235e38,
                                        1u,
                                        "x = North-South coord base value, \n"
                                        "y = East-West coord base value, \n"
                                        "z = scale (game units per coord unit)");
    compassECoordCutoff = _Dvar_RegisterFloat(
                                                    "compassECoordCutoff",
                                                    37.0,
                                                    0.0,
                                                    3.4028235e38,
                                                    1u,
                                                    "Left cutoff for the scrolling east-west coords");
    compassRotation = _Dvar_RegisterBool("compassRotation", 1, 1u, "Style of compass");
    compassTickertapeStretch = _Dvar_RegisterFloat(
                                                             "compassTickertapeStretch",
                                                             0.5,
                                                             0.0099999998,
                                                             1.0,
                                                             1u,
                                                             "How far the tickertape should stretch from its center.");
    compassRadarPingFadeTime = _Dvar_RegisterFloat(
                                                             "compassRadarPingFadeTime",
                                                             4.0,
                                                             0.0099999998,
                                                             60.0,
                                                             0x81u,
                                                             "How long an enemy is visible on the compass after it is detected by radar");
    compassRadarUpdateTime = _Dvar_RegisterFloat(
                                                         "compassRadarUpdateTime",
                                                         4.0,
                                                         0.0099999998,
                                                         60.0,
                                                         0x81u,
                                                         "Time between radar updates");
    compassRadarUpdateFastTime = _Dvar_RegisterFloat(
                                                                 "compassRadarUpdateFastTime",
                                                                 2.0,
                                                                 0.0099999998,
                                                                 60.0,
                                                                 0x81u,
                                                                 "Time between radar updates for fast update killstreak");
    compassSatelliteStaticImageFadeTime = _Dvar_RegisterFloat(
                                                                                    "compassSatelliteStaticImageFadeTime",
                                                                                    0.0,
                                                                                    0.0,
                                                                                    60.0,
                                                                                    0x81u,
                                                                                    "How long the static image is visible on the compass after it is detected by Satellite");
    compassSatellitePingFadeTime = _Dvar_RegisterFloat(
                                                                     "compassSatellitePingFadeTime",
                                                                     10.0,
                                                                     0.0099999998,
                                                                     60.0,
                                                                     0x81u,
                                                                     "How long an enemy is visible on the compass after it is detected by Satellite");
    compassStaticImageUpdateTime = _Dvar_RegisterInt(
                                                                     "compassStaticImageUpdateTime",
                                                                     5000,
                                                                     1,
                                                                     60000,
                                                                     0x81u,
                                                                     "Time between static image updates");
    compassSatelliteScanTime = _Dvar_RegisterInt(
                                                             "compassSatelliteScanTime",
                                                             1000,
                                                             1,
                                                             60000,
                                                             0x81u,
                                                             "Time taken for Satellite t scans");
    compassRadarLineThickness = _Dvar_RegisterFloat(
                                                                "compassRadarLineThickness",
                                                                0.40000001,
                                                                0.0099999998,
                                                                10.0,
                                                                0x81u,
                                                                "Thickness, relative to the compass size, of the radar texture that sweeps across the map");
    compassLocalRadarUpdateTime = _Dvar_RegisterFloat(
                                                                    "compassLocalRadarUpdateTime",
                                                                    2.25,
                                                                    0.0099999998,
                                                                    60.0,
                                                                    0x81u,
                                                                    "Time between local radar updates");
    compassLocalRadarRadius = _Dvar_RegisterFloat(
                                                            "compassLocalRadarRadius",
                                                            700.0,
                                                            0.0099999998,
                                                            10000.0,
                                                            0x81u,
                                                            "Maximum radius of the local radar");
    compassObjectiveWidth = _Dvar_RegisterFloat(
                                                        "compassObjectiveWidth",
                                                        20.0,
                                                        0.0,
                                                        3.4028235e38,
                                                        1u,
                                                        "The size of the objective on the compass");
    compassObjectiveHeight = _Dvar_RegisterFloat(
                                                         "compassObjectiveHeight",
                                                         20.0,
                                                         0.0,
                                                         3.4028235e38,
                                                         1u,
                                                         "The size of the objective on the compass");
    compassObjectiveArrowWidth = _Dvar_RegisterFloat(
                                                                 "compassObjectiveArrowWidth",
                                                                 20.0,
                                                                 0.0,
                                                                 3.4028235e38,
                                                                 1u,
                                                                 "The size of the objective arrow on the compass");
    compassObjectiveArrowHeight = _Dvar_RegisterFloat(
                                                                    "compassObjectiveArrowHeight",
                                                                    20.0,
                                                                    0.0,
                                                                    3.4028235e38,
                                                                    1u,
                                                                    "The size of the objective arrow on the compass");
    compassObjectiveArrowOffset = _Dvar_RegisterFloat(
                                                                    "compassObjectiveArrowOffset",
                                                                    2.0,
                                                                    0.0,
                                                                    3.4028235e38,
                                                                    1u,
                                                                    "The offset of the objective arrow inward from the edge of the compass map");
    compassObjectiveArrowRotateDist = _Dvar_RegisterFloat(
                                                                            "compassObjectiveArrowRotateDist",
                                                                            5.0,
                                                                            0.0,
                                                                            3.4028235e38,
                                                                            1u,
                                                                            "Distance from the corner of the compass map at which the objective arrow rotates to 45 degrees");
    compassObjectiveMaxRange = _Dvar_RegisterFloat(
                                                             "compassObjectiveMaxRange",
                                                             2048.0,
                                                             0.0,
                                                             3.4028235e38,
                                                             1u,
                                                             "The maximum range at which an objective is visible on the compass");
    compassObjectiveMinAlpha = _Dvar_RegisterFloat(
                                                             "compassObjectiveMinAlpha",
                                                             1.0,
                                                             0.0,
                                                             1.0,
                                                             1u,
                                                             "The minimum alpha for an objective at the edge of the compass");
    compassObjectiveNumRings = _Dvar_RegisterInt(
                                                             "compassObjectiveNumRings",
                                                             10,
                                                             0,
                                                             20,
                                                             1u,
                                                             "The number of rings when a new objective appears");
    compassObjectiveRingTime = _Dvar_RegisterInt(
                                                             "compassObjectiveRingTime",
                                                             10000,
                                                             0,
                                                             0x7FFFFFFF,
                                                             1u,
                                                             "The amount of time between each ring when an objective appears");
    compassObjectiveRingSize = _Dvar_RegisterFloat(
                                                             "compassObjectiveRingSize",
                                                             80.0,
                                                             0.0,
                                                             3.4028235e38,
                                                             1u,
                                                             "The maximum objective ring sige when a new objective appears on the compass");
    compassObjectiveTextScale = _Dvar_RegisterFloat(
                                                                "compassObjectiveTextScale",
                                                                0.30000001,
                                                                0.0000099999997,
                                                                3.4028235e38,
                                                                1u,
                                                                "Scale to apply to hud objectives");
    compassObjectiveTextHeight = _Dvar_RegisterFloat(
                                                                 "compassObjectiveTextHeight",
                                                                 18.0,
                                                                 0.0000099999997,
                                                                 3.4028235e38,
                                                                 1u,
                                                                 "Objective text height");
    compassObjectiveDrawLines = _Dvar_RegisterBool(
                                                                "compassObjectiveDrawLines",
                                                                1,
                                                                1u,
                                                                "Draw horizontal and vertical lines to the active target, if it is within the minimap boundries");
    compassObjectiveIconWidth = _Dvar_RegisterFloat(
                                                                "compassObjectiveIconWidth",
                                                                18.0,
                                                                0.0,
                                                                3.4028235e38,
                                                                1u,
                                                                "The size of the objective on the full map");
    compassObjectiveIconHeight = _Dvar_RegisterFloat(
                                                                 "compassObjectiveIconHeight",
                                                                 18.0,
                                                                 0.0,
                                                                 3.4028235e38,
                                                                 1u,
                                                                 "The size of the objective on the full map");
    compassObjectiveIconWidthZombie = _Dvar_RegisterFloat(
                                                                            "compassObjectiveIconWidthZombie",
                                                                            16.0,
                                                                            0.0,
                                                                            3.4028235e38,
                                                                            1u,
                                                                            "The size of the objective on the full map");
    compassObjectiveIconHeightZombie = _Dvar_RegisterFloat(
                                                                             "compassObjectiveIconHeightZombie",
                                                                             16.0,
                                                                             0.0,
                                                                             3.4028235e38,
                                                                             1u,
                                                                             "The size of the objective on the full map");
    compassObjectiveNearbyDist = _Dvar_RegisterFloat(
                                                                 "compassObjectiveNearbyDist",
                                                                 4.0,
                                                                 0.0099999998,
                                                                 3.4028235e38,
                                                                 0x1000u,
                                                                 "When an objective is closer than this distance (in meters), an \"Objective Nearby\" typ"
                                                                 "e of indicator is shown.");
    compassObjectiveMinDistRange = _Dvar_RegisterFloat(
                                                                     "compassObjectiveMinDistRange",
                                                                     1.0,
                                                                     0.0099999998,
                                                                     3.4028235e38,
                                                                     0x1000u,
                                                                     "The distance that objective transition effects play over, centered on compassObjectiveNearbyDist.");
    compassObjectiveDetailDist = _Dvar_RegisterFloat(
                                                                 "compassObjectiveDetailDist",
                                                                 10.0,
                                                                 0.0099999998,
                                                                 3.4028235e38,
                                                                 0x1000u,
                                                                 "When an objective is closer than this distance (in meters), the icon will not be drawn "
                                                                 "on the tickertape.");
    compassObjectiveMinHeight = _Dvar_RegisterFloat(
                                                                "compassObjectiveMinHeight",
                                                                -70.0,
                                                                -3.4028235e38,
                                                                0.0,
                                                                0x1000u,
                                                                "The minimum height that an objective is considered to be on this level");
    compassObjectiveMaxHeight = _Dvar_RegisterFloat(
                                                                "compassObjectiveMaxHeight",
                                                                70.0,
                                                                0.0,
                                                                3.4028235e38,
                                                                0x1000u,
                                                                "The maximum height that an objective is considered to be on this level");
    compassPartialType = _Dvar_RegisterInt("compassPartialType", 0, 0, 1, 1u, "The style of compass to use 2d or 3d");
    compassSpectatorsSeeEnemies = _Dvar_RegisterBool(
                                                                    "compassSpectatorsSeeEnemies",
                                                                    0,
                                                                    1u,
                                                                    "Spectators always see enemies on the map.");
    compassEnemyFootstepMaxRange = _Dvar_RegisterFloat(
                                                                     "compassEnemyFootstepMaxRange",
                                                                     500.0,
                                                                     0.0,
                                                                     3.4028235e38,
                                                                     0x80u,
                                                                     "The maximum distance at which an enemy may appear on the compass due to 'footsteps'");
    compassEnemyFootstepMaxZ = _Dvar_RegisterFloat(
                                                             "compassEnemyFootstepMaxZ",
                                                             100.0,
                                                             0.0,
                                                             3.4028235e38,
                                                             0x80u,
                                                             "The maximum vertical distance enemy may be from the player and appear on the compass due to 'footsteps'");
    compassEnemyFootstepMinSpeed = _Dvar_RegisterFloat(
                                                                     "compassEnemyFootstepMinSpeed",
                                                                     140.0,
                                                                     0.0,
                                                                     3.4028235e38,
                                                                     0x80u,
                                                                     "The minimum speed an enemy must be moving to appear on the compass due to 'footsteps'");
    compassEnemyFootstepEnabled = _Dvar_RegisterBool(
                                                                    "compassEnemyFootstepEnabled",
                                                                    0,
                                                                    0x80u,
                                                                    "Enables enemies showing on the compass because of moving rapidly nearby.");
    compassShowEnemies = _Dvar_RegisterBool("compassShowEnemies", 1, 0x80u, "Enables enemies showing on the compass.");
    compassForcePlayerIcon = _Dvar_RegisterBool(
                                                         "compassForcePlayerIcon",
                                                         0,
                                                         0x80u,
                                                         "Forces the player to always show as compassping_player on the compass.");
    compassGridEnabled = _Dvar_RegisterBool("compassGridEnabled", 1, 0, "Enables compass grid.");
    compassGridRows = _Dvar_RegisterInt("compassGridRows", 5, 1, 9, 0, "Grid row count.");
    compassGridCols = _Dvar_RegisterInt("compassGridCols", 5, 1, 9, 0, "Grid column count.");
    compassGridAlign = _Dvar_RegisterInt("compassGridAlign", 0, 0, 8, 0, "Position of letter and number in grid.");
    compassScaleDuration = _Dvar_RegisterInt(
                                                     "compassScaleDuration",
                                                     0,
                                                     0,
                                                     0x7FFFFFFF,
                                                     0,
                                                     "The amount of time to take scaling the compass map.");
    compassScaleDiff = _Dvar_RegisterInt(
                                             "compassScaleDiff",
                                             0,
                                             0x80000000,
                                             0x7FFFFFFF,
                                             0,
                                             "Difference from original compass max range in world space.");
    compassScaleTimer = _Dvar_RegisterInt("compassScaleTimer", 0, 0, 0x7FFFFFFF, 0, "Time scaling started.");
    compassScaleReset = _Dvar_RegisterBool(
                                                "compassScaleReset",
                                                0,
                                                0,
                                                "Whether to reset compassMaxRange to its default value.");
    typeWriterCod7LetterFXTime = _Dvar_RegisterInt(
                                                                 "typeWriterCod7LetterFXTime",
                                                                 50,
                                                                 1,
                                                                 0x7FFFFFFF,
                                                                 0x80u,
                                                                 "Time it takes for a letter to come in. ");
    popInLetterFXTime = _Dvar_RegisterInt(
                                                "popInLetterFXTime",
                                                40,
                                                1,
                                                0x7FFFFFFF,
                                                0x80u,
                                                "Time it takes for a letter to pop in. ");
    redactionDisplayTime = _Dvar_RegisterInt(
                                                     "redactionDisplayTime",
                                                     50,
                                                     1,
                                                     0x7FFFFFFF,
                                                     0x80u,
                                                     "Time that it should be displayed for. ");
    redactionFadeDuration = _Dvar_RegisterInt("redactionFadeDuration", 6500, 1, 0x7FFFFFFF, 0x80u, "Fade duration time. ");
    redactionStartStrokeTime = _Dvar_RegisterInt(
                                                             "redactionStartStrokeTime",
                                                             1000,
                                                             1,
                                                             0x7FFFFFFF,
                                                             0x80u,
                                                             "Time before the stroke starts. ");
    redactionStrokeTime = _Dvar_RegisterInt(
                                                    "redactionStrokeTime",
                                                    1000,
                                                    1,
                                                    0x7FFFFFFF,
                                                    0x80u,
                                                    "Time it takes to complete the redaction stroke. ");
    compassDebug = _Dvar_RegisterBool("compassDebug", 0, 1u, "Compass Debugging Mode");
    forceFullScreenMap = _Dvar_RegisterBool("forceFullScreenMap", 0, 1u, "Force the fullscreen map up");
    cg_showZombieMap = _Dvar_RegisterBool("cg_showZombieMap", 1, 0, "Show the zombie map up");
    cg_hudMapRadarLineThickness = _Dvar_RegisterFloat(
                                                                    "cg_hudMapRadarLineThickness",
                                                                    0.15000001,
                                                                    0.0099999998,
                                                                    10.0,
                                                                    0x81u,
                                                                    "Thickness, relative to the map width, of the radar texture that sweeps across the full screen map");
    cg_hudMapFriendlyWidth = _Dvar_RegisterFloat(
                                                         "cg_hudMapFriendlyWidth",
                                                         15.0,
                                                         0.0,
                                                         3.4028235e38,
                                                         1u,
                                                         "The size of the friendly icon on the full map");
    cg_hudMapFriendlyHeight = _Dvar_RegisterFloat(
                                                            "cg_hudMapFriendlyHeight",
                                                            15.0,
                                                            0.0,
                                                            3.4028235e38,
                                                            1u,
                                                            "The size of the friendly icon on the full map");
    cg_hudMapPlayerWidth = _Dvar_RegisterFloat(
                                                     "cg_hudMapPlayerWidth",
                                                     20.0,
                                                     0.0,
                                                     3.4028235e38,
                                                     1u,
                                                     "The size of the player's icon on the full map");
    cg_hudMapPlayerHeight = _Dvar_RegisterFloat(
                                                        "cg_hudMapPlayerHeight",
                                                        20.0,
                                                        0.0,
                                                        3.4028235e38,
                                                        1u,
                                                        "The size of the player's icon on the full map");
    cg_hudMapBorderWidth = _Dvar_RegisterFloat(
                                                     "cg_hudMapBorderWidth",
                                                     2.0,
                                                     0.0,
                                                     3.4028235e38,
                                                     1u,
                                                     "The size of the full map's border, filled by the CG_PLAYER_FULLMAP_BORDER ownerdraw");
    cg_showZombieControls = _Dvar_RegisterBool("cg_showZombieControls", 0, 0, "Show the zombie controller layout");
}

char __cdecl CG_IsSelectingLocation(int localClientNum)
{
    bool result; // [esp+3h] [ebp-5h]

    result = CG_GetLocalClientGlobals(localClientNum)->predictedPlayerState.locationSelectionInfo != 0;
    if ( forceFullScreenMap->current.enabled )
        return 1;
    return result;
}

bool __cdecl CG_WorldPosToCompass(
                CompassType compassType,
                const cg_s *cgameGlob,
                const rectDef_s *mapRect,
                const float *north,
                const float *playerWorldPos,
                const float *in,
                float *out,
                float *outClipped)
{
    bool clipped; // [esp+Fh] [ebp-19h]
    float posDelta_0; // [esp+10h] [ebp-18h]
    float posDelta0; // [esp+10h] [ebp-18h]
    float posDelta4; // [esp+14h] [ebp-14h]
    float posDelta_4; // [esp+14h] [ebp-14h]
    float pixPerInch; // [esp+18h] [ebp-10h]
    float outTemp; // [esp+1Ch] [ebp-Ch]
    float outTemp_4; // [esp+20h] [ebp-8h]
    float scale; // [esp+24h] [ebp-4h]
    float scalea; // [esp+24h] [ebp-4h]
    float scaleb; // [esp+24h] [ebp-4h]
    float scalec; // [esp+24h] [ebp-4h]

    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 361, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    if ( !mapRect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 362, 0, "%s", "mapRect") )
    {
        __debugbreak();
    }
    if ( !playerWorldPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 363, 0, "%s", "playerWorldPos") )
    {
        __debugbreak();
    }
    if ( !in && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 364, 0, "%s", "in") )
        __debugbreak();
    if ( compassType )
    {
        if ( compassType != COMPASS_TYPE_FULL
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                        382,
                        0,
                        "%s",
                        "compassType == COMPASS_TYPE_FULL") )
        {
            __debugbreak();
        }
        posDelta0 = *in - cgameGlob->compassMapUpperLeft[0];
        posDelta_4 = in[1] - cgameGlob->compassMapUpperLeft[1];
        outTemp = (float)((float)((float)((float)(cgameGlob->compassNorth[1] * posDelta0)
                                                                        - (float)(cgameGlob->compassNorth[0] * posDelta_4))
                                                        / cgameGlob->compassMapWorldSize[0])
                                        - 0.5)
                        * mapRect->w;
        //outTemp_4 = (float)((float)((float)((float)(COERCE_FLOAT(LODWORD(cgameGlob->compassNorth[1]) ^ _mask__NegFloat_)
        outTemp_4 = (float)((float)((float)((float)( -cgameGlob->compassNorth[1]
                                                                                            * posDelta_4)
                                                                            - (float)(cgameGlob->compassNorth[0] * posDelta0))
                                                            / cgameGlob->compassMapWorldSize[1])
                                            - 0.5)
                            * mapRect->h;
    }
    else
    {
        if ( !north && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 368, 0, "%s", "north") )
            __debugbreak();
        if ( compassMaxRange->current.value < 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                        370,
                        0,
                        "%s\n\t(compassMaxRange->current.value) = %g",
                        "(compassMaxRange->current.value >= 0.0f)",
                        compassMaxRange->current.value) )
        {
            __debugbreak();
        }
        pixPerInch = mapRect->h / compassMaxRange->current.value;
        posDelta_0 = pixPerInch * (float)(*in - *playerWorldPos);
        posDelta4 = pixPerInch * (float)(in[1] - playerWorldPos[1]);
        outTemp = (float)(north[1] * posDelta_0) - (float)(*north * posDelta4);
        //outTemp_4 = (float)(COERCE_FLOAT(*((unsigned int *)north + 1) ^ _mask__NegFloat_) * posDelta4) - (float)(*north * posDelta_0);
        outTemp_4 = (float)(-north[1] * posDelta4) - (float)(*north * posDelta_0);
    }
    clipped = 0;
    if ( outClipped && mapRect->w >= 0.0 && mapRect->h >= 0.0 )
    {
        *outClipped = outTemp;
        outClipped[1] = outTemp_4;
        if ( *outClipped <= (float)(mapRect->w * 0.5) )
        {
            //if ( (float)(COERCE_FLOAT(LODWORD(mapRect->w) ^ _mask__NegFloat_) * 0.5) > *outClipped )
            if ( ((-mapRect->w) * 0.5) > *outClipped)
            {
                //scalea = COERCE_FLOAT(COERCE_UNSIGNED_INT(mapRect->w * 0.5) ^ _mask__NegFloat_) / *outClipped;
                scalea = -(mapRect->w * 0.5f) / *outClipped;
                *outClipped = scalea * *outClipped;
                outClipped[1] = scalea * outClipped[1];
                clipped = 1;
            }
        }
        else
        {
            scale = (float)(mapRect->w * 0.5) / *outClipped;
            *outClipped = scale * *outClipped;
            outClipped[1] = scale * outClipped[1];
            clipped = 1;
        }
        if ( outClipped[1] <= (float)(mapRect->h * 0.5) )
        {
            //if ( (float)(COERCE_FLOAT(LODWORD(mapRect->h) ^ _mask__NegFloat_) * 0.5) > outClipped[1] )
            if ( ((-mapRect->h) * 0.5) > outClipped[1] )
            {
                //scalec = COERCE_FLOAT(COERCE_UNSIGNED_INT(mapRect->h * 0.5) ^ _mask__NegFloat_) / outClipped[1];
                scalec = -(mapRect->h * 0.5) / outClipped[1];
                *outClipped = scalec * *outClipped;
                outClipped[1] = scalec * outClipped[1];
                clipped = 1;
            }
        }
        else
        {
            scaleb = (float)(mapRect->h * 0.5) / outClipped[1];
            *outClipped = scaleb * *outClipped;
            outClipped[1] = scaleb * outClipped[1];
            clipped = 1;
        }
    }
    if ( out )
    {
        *out = outTemp;
        out[1] = outTemp_4;
    }
    return clipped;
}

char __cdecl CG_TextClippedToCompass(
                CompassType compassType,
                const cg_s *cgameGlob,
                const rectDef_s *mapRect,
                const float *north,
                const float *playerWorldPos,
                const float *in,
                float w,
                float h)
{
    bool clipped; // [esp+Fh] [ebp-25h]
    float inTemp; // [esp+18h] [ebp-1Ch]
    float inTemp_4; // [esp+1Ch] [ebp-18h]
    float posDelta; // [esp+20h] [ebp-14h]
    float posDeltaa; // [esp+20h] [ebp-14h]
    float posDelta_4; // [esp+24h] [ebp-10h]
    float posDelta_4a; // [esp+24h] [ebp-10h]
    float pixPerInch; // [esp+28h] [ebp-Ch]

    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 445, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    if ( !mapRect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 446, 0, "%s", "mapRect") )
    {
        __debugbreak();
    }
    if ( !playerWorldPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 447, 0, "%s", "playerWorldPos") )
    {
        __debugbreak();
    }
    if ( !in && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 448, 0, "%s", "in") )
        __debugbreak();
    if ( compassType )
    {
        if ( compassType != COMPASS_TYPE_FULL
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                        465,
                        0,
                        "%s",
                        "compassType == COMPASS_TYPE_FULL") )
        {
            __debugbreak();
        }
        posDeltaa = *in - cgameGlob->compassMapUpperLeft[0];
        posDelta_4a = in[1] - cgameGlob->compassMapUpperLeft[1];
        inTemp = (float)((float)((float)((float)(cgameGlob->compassNorth[1] * posDeltaa)
                                                                     - (float)(cgameGlob->compassNorth[0] * posDelta_4a))
                                                     / cgameGlob->compassMapWorldSize[0])
                                     - 0.5)
                     * mapRect->w;
        //inTemp_4 = (float)((float)((float)((float)(COERCE_FLOAT(LODWORD(cgameGlob->compassNorth[1]) ^ _mask__NegFloat_)
        inTemp_4 = (float)((float)((float)((float)( -cgameGlob->compassNorth[1]
                                                                                         * posDelta_4a)
                                                                         - (float)(cgameGlob->compassNorth[0] * posDeltaa))
                                                         / cgameGlob->compassMapWorldSize[1])
                                         - 0.5)
                         * mapRect->h;
    }
    else
    {
        if ( !north && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 452, 0, "%s", "north") )
            __debugbreak();
        if ( compassMaxRange->current.value < 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                        454,
                        0,
                        "%s\n\t(compassMaxRange->current.value) = %g",
                        "(compassMaxRange->current.value >= 0.0f)",
                        compassMaxRange->current.value) )
        {
            __debugbreak();
        }
        pixPerInch = mapRect->h / compassMaxRange->current.value;
        posDelta = pixPerInch * (float)(*in - *playerWorldPos);
        posDelta_4 = pixPerInch * (float)(in[1] - playerWorldPos[1]);
        inTemp = (float)(north[1] * posDelta) - (float)(*north * posDelta_4);
        //inTemp_4 = (float)(COERCE_FLOAT(*((unsigned int *)north + 1) ^ _mask__NegFloat_) * posDelta_4) - (float)(*north * posDelta);
        inTemp_4 = (float)(-north[1] * posDelta_4) - (float)(*north * posDelta);
    }
    clipped = 0;
    if ( w != 0.0 && h != 0.0 && mapRect->w >= 0.0 && mapRect->h >= 0.0 )
    {
        clipped = (float)(inTemp - (float)(w * 0.5)) > (float)(mapRect->w * 0.5)
                     //|| (float)(COERCE_FLOAT(LODWORD(mapRect->w) ^ _mask__NegFloat_) * 0.5) > (float)((float)(w * 0.5) + inTemp);
                     || (float)(-(mapRect->w) * 0.5) > (float)((float)(w * 0.5) + inTemp);
        if ( (float)(inTemp_4 - (float)(h * 0.5)) <= (float)(mapRect->h * 0.5) )
        {
            //if ( (float)(COERCE_FLOAT(LODWORD(mapRect->h) ^ _mask__NegFloat_) * 0.5) > (float)((float)(h * 0.5) + inTemp_4) )
            if ( (float)(-(mapRect->h) * 0.5) > (float)((float)(h * 0.5) + inTemp_4) )
                return 1;
        }
        else
        {
            return 1;
        }
    }
    return clipped;
}

void __cdecl CG_CompassCalcDimensions(
                CompassType compassType,
                const cg_s *cgameGlob,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                float *x,
                float *y,
                float *w,
                float *h)
{
    float mapAspectRatio; // [esp+0h] [ebp-1Ch]
    float basex; // [esp+4h] [ebp-18h]
    float fraction; // [esp+8h] [ebp-14h]
    float fractiona; // [esp+8h] [ebp-14h]
    float border; // [esp+Ch] [ebp-10h]
    float rectAspectRatio; // [esp+10h] [ebp-Ch]
    float basey; // [esp+14h] [ebp-8h]
    float center; // [esp+18h] [ebp-4h]

    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 512, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    if ( !parentRect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 513, 0, "%s", "parentRect") )
    {
        __debugbreak();
    }
    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 514, 0, "%s", "rect") )
        __debugbreak();
    if ( !x && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 515, 0, "%s", "x") )
        __debugbreak();
    if ( !y && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 516, 0, "%s", "y") )
        __debugbreak();
    if ( !w && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 517, 0, "%s", "w") )
        __debugbreak();
    if ( !h && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 518, 0, "%s", "h") )
        __debugbreak();
    if ( compassType )
    {
        if ( rect->w <= 0.0 || rect->h <= 0.0 )
            Com_Error(ERR_FATAL, "Compass ownerdraw had width or height of 0");
        if ( rect->w == 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 549, 0, "%s", "rect->w") )
        {
            __debugbreak();
        }
        if ( rect->h == 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 550, 0, "%s", "rect->h") )
        {
            __debugbreak();
        }
        if ( cgameGlob->compassMapWorldSize[0] == 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                        551,
                        0,
                        "%s",
                        "cgameGlob->compassMapWorldSize[0]") )
        {
            __debugbreak();
        }
        if ( cgameGlob->compassMapWorldSize[1] == 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                        552,
                        0,
                        "%s",
                        "cgameGlob->compassMapWorldSize[1]") )
        {
            __debugbreak();
        }
        rectAspectRatio = rect->w / rect->h;
        mapAspectRatio = cgameGlob->compassMapWorldSize[0] / cgameGlob->compassMapWorldSize[1];
        basex = parentRect->x + rect->x;
        basey = parentRect->y + rect->y;
        if ( mapAspectRatio <= rectAspectRatio )
        {
            fractiona = (float)(mapAspectRatio / rectAspectRatio) * rect->w;
            *x = (float)((float)(rect->w * 0.5) + basex) - (float)(fractiona * 0.5);
            *y = basey;
            *w = fractiona;
            *h = rect->h;
        }
        else
        {
            center = (float)(rect->h * 0.5) + basey;
            fraction = (float)(rectAspectRatio / mapAspectRatio) * rect->h;
            *x = basex;
            *y = center - (float)(fraction * 0.5);
            *w = rect->w;
            *h = fraction;
        }
        border = cg_hudMapBorderWidth->current.value;
        if ( border > (float)(*w * 0.25) )
            border = *w * 0.25;
        if ( border > (float)(*h * 0.25) )
            border = *h * 0.25;
        *x = *x + border;
        *y = *y + border;
        *w = *w - (float)(2.0 * border);
        *h = *h - (float)(2.0 * border);
    }
    else
    {
        *x = rect->x;
        *y = rect->y;
        *w = rect->w * compassSize->current.value;
        *h = rect->h * compassSize->current.value;
        //BLOPS_NULLSUB();
        if ( *w <= 0.0 || *h <= 0.0 )
            Com_Error(ERR_FATAL, "Compass ownerdraw had width or height of 0");
    }
}

double __cdecl CG_FadeCompass(int localClientNum, int displayStartTime, CompassType compassType)
{
    if ( compassType )
        return 1.0;
    else
        return CG_FadeHudMenu(
                         localClientNum,
                         hud_fade_compass,
                         displayStartTime,
                         (int)((float)(hud_fade_compass->current.value * 1000.0) + 9.313225746154785e-10));
}

double __cdecl CG_FadeCompassIcons(int localClientNum, bool ignoreGlobalScrambler)
{
    float fade; // [esp+0h] [ebp-8h]
    const cg_s *cgameGlob; // [esp+4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    fade = 1.0 - CG_GetScramblerEnemyAlpha(localClientNum);
    if ( !ignoreGlobalScrambler && cgameGlob->globalScramblerActive )
        fade = 0.0f;
    if ( fade < 0.0 )
        return 0.0f;
    return fade;
}

void __cdecl CG_CompassDrawPlayerBack(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Material *material,
                float *color)
{
    const cg_s *cgameGlob; // [esp+34h] [ebp-14h]
    float x; // [esp+38h] [ebp-10h] BYREF
    float y; // [esp+3Ch] [ebp-Ch] BYREF
    float h; // [esp+40h] [ebp-8h] BYREF
    float w; // [esp+44h] [ebp-4h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    color[3] = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    if ( color[3] != 0.0 && !compassPartialType->current.integer )
    {
        CG_CompassCalcDimensions(compassType, cgameGlob, parentRect, rect, &x, &y, &w, &h);
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            x,
            y,
            w,
            h,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            color,
            material);
    }
}

#if 0
void __cdecl CG_DrawPlayerPopUps(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                Material *material,
                int textStyle,
                itemDef_s *item,
                char *text,
                bool doSwing)
{
    double v10; // xmm0_8
    double v11; // xmm0_8
    double v12; // xmm0_8
    double v13; // xmm0_8
    long double v14; // [esp+40h] [ebp-7Ch]
    long double v15; // [esp+40h] [ebp-7Ch]
    long double v16; // [esp+40h] [ebp-7Ch]
    long double v17; // [esp+40h] [ebp-7Ch]
    float v18; // [esp+54h] [ebp-68h]
    float v19; // [esp+64h] [ebp-58h]
    float y; // [esp+6Ch] [ebp-50h]
    float x; // [esp+70h] [ebp-4Ch]
    float v22; // [esp+74h] [ebp-48h]
    float rotatedY; // [esp+78h] [ebp-44h]
    float rotatedX; // [esp+7Ch] [ebp-40h]
    float stringPixelLength; // [esp+84h] [ebp-38h]
    int ratio; // [esp+88h] [ebp-34h]
    cg_s *yScale; // [esp+8Ch] [ebp-30h]
    int g_msgTime; // [esp+94h] [ebp-28h]
    float cgameGlob; // [esp+98h] [ebp-24h]
    int swingLength; // [esp+A8h] [ebp-14h]
    const ScreenPlacement *swayMaxTime; // [esp+ACh] [ebp-10h]

    swayMaxTime = &scrPlaceView[localClientNum];
    cgameGlob = item->window.outlineColor[3];
    g_msgTime = 0;
    if ( text )
    {
        if ( color[3] != 0.0 )
        {
            yScale = CG_GetLocalClientGlobals(localClientNum);
            swingLength = yScale->popUpSwayStartTime;
            ratio = yScale->g_msgTime;
            if ( swingLength || !doSwing )
            {
                if ( swingLength && (float)ratio > (float)((float)swingLength + 2500.0) )
                {
                    yScale->popupRotationAngle = 0.0f;
                    yScale->popUpAngleDelta = 0.0f;
                }
                stringPixelLength = R_NormalizedTextScale(font, scale);
                if ( doSwing )
                {
                    rotatedX = (float)((float)((float)swingLength + 2500.0) - (float)ratio) / 2500.0;
                    if ( yScale->popupRotationAngle > (float)((float)(rotatedX * rotatedX) * 7.0) )
                        yScale->popUpAngleDelta = (-(0.5f)) * (float)(rotatedX * rotatedX);
                    if ( (float)((-(7.0f)) * (float)(rotatedX * rotatedX)) > yScale->popupRotationAngle )
                        yScale->popUpAngleDelta = (float)(rotatedX * rotatedX) * 0.5;
                    yScale->popupRotationAngle = yScale->popupRotationAngle + yScale->popUpAngleDelta;
                }
                rotatedY = (float)R_TextWidth(text, 0, font) * stringPixelLength;
                v22 = rect->y + 100.0;
                x = rect->x;
                y = rect->y;
                if ( doSwing && swingLength && (float)((float)swingLength + 2500.0) > (float)ratio )
                {
                    v10 = (float)(yScale->popupRotationAngle * 0.017453292);
                    __libm_sse2_cos(v14);
                    *(float *)&v10 = v10;
                    v19 = *(float *)&v10;
                    v11 = (float)(COERCE_FLOAT(LODWORD(yScale->popupRotationAngle) ^ _mask__NegFloat_) * 0.017453292);
                    __libm_sse2_sin(v15);
                    *(float *)&v11 = v11;
                    x = (float)((float)((float)((float)(1.0 - v19) * rotatedY) / 2.0) + rect->x) + (float)(*(float *)&v11 * v22);
                    v12 = (float)(yScale->popupRotationAngle * 0.017453292);
                    __libm_sse2_sin(v16);
                    *(float *)&v12 = v12;
                    v18 = *(float *)&v12;
                    v13 = (float)(COERCE_FLOAT(LODWORD(yScale->popupRotationAngle) ^ _mask__NegFloat_) * 0.017453292);
                    __libm_sse2_cos(v17);
                    *(float *)&v13 = v13;
                    y = (float)(rect->y - (float)((float)(v18 * rotatedY) / 2.0)) + (float)((float)(*(float *)&v13 * v22) - v22);
                    g_msgTime = LODWORD(yScale->popupRotationAngle);
                }
                if ( cgameGlob == 0.0 )
                    CL_DrawTextRotate(
                        swayMaxTime,
                        text,
                        256,
                        font,
                        x,
                        y,
                        *(float *)&g_msgTime,
                        rect->horzAlign,
                        rect->vertAlign,
                        stringPixelLength,
                        stringPixelLength,
                        color,
                        textStyle);
                else
                    CL_DrawTextWithEffects(
                        swayMaxTime,
                        text,
                        256,
                        font,
                        x,
                        y,
                        *(float *)&g_msgTime,
                        rect->horzAlign,
                        rect->vertAlign,
                        stringPixelLength,
                        stringPixelLength,
                        color,
                        item->window.style,
                        item->window.outlineColor,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0);
            }
        }
    }
}
#endif

// aislop
void CG_DrawPlayerPopUps(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    float scale,
    float *color,
    Material *material,
    int textStyle,
    itemDef_s *item,
    char *text,
    bool doSwing)
{
    if (!text || color[3] == 0.0f)
        return;

    cg_s *cgame = CG_GetLocalClientGlobals(localClientNum);
    const ScreenPlacement *place = &scrPlaceView[localClientNum];

    const int swingStart = cgame->popUpSwayStartTime;
    const int now = cgame->time;

    if (!swingStart && doSwing)
        return;

    if (swingStart && now > swingStart + 2500)
    {
        cgame->popupRotationAngle = 0.0f;
        cgame->popUpAngleDelta = 0.0f;
    }

    const float textScale = R_NormalizedTextScale(font, scale);
    const float textWidth =
        R_TextWidth(text, 0, font) * textScale;

    float x = rect->x;
    float y = rect->y;

    if (doSwing && swingStart && now < swingStart + 2500)
    {
        const float t =
            (float)(swingStart + 2500 - now) / 2500.0f;
        const float t2 = t * t;

        if (cgame->popupRotationAngle > t2 * 7.0f)
            cgame->popUpAngleDelta = -0.5f * t2;
        else if (cgame->popupRotationAngle < -t2 * 7.0f)
            cgame->popUpAngleDelta = 0.5f * t2;

        cgame->popupRotationAngle += cgame->popUpAngleDelta;

        const float angleRad =
            cgame->popupRotationAngle * (float)(M_PI / 180.0);

        const float cosA = cosf(angleRad);
        const float sinA = sinf(angleRad);

        const float swayRadius = rect->y + 100.0f;

        x = rect->x
            + ((1.0f - cosA) * textWidth * 0.5f)
            - sinA * swayRadius;

        y = rect->y
            - (sinA * textWidth * 0.5f)
            + (cosA * swayRadius - swayRadius);
    }

    const float rotation = cgame->popupRotationAngle;

    if (item->window.outlineColor[3] == 0.0f)
    {
        CL_DrawTextRotate(
            place,
            text,
            256,
            font,
            x,
            y,
            rotation,
            rect->horzAlign,
            rect->vertAlign,
            textScale,
            textScale,
            color,
            textStyle);
    }
    else
    {
        CL_DrawTextWithEffects(
            place,
            text,
            256,
            font,
            x,
            y,
            rotation,
            rect->horzAlign,
            rect->vertAlign,
            textScale,
            textScale,
            color,
            item->window.style,
            item->window.outlineColor,
            0, 0, 0, 0, 0, 0);
    }
}


void __cdecl CG_DrawPlayerCOD7TypeWriter(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                Material *material,
                int textStyle,
                itemDef_s *item,
                char *text)
{
    float xScale; // [esp+40h] [ebp-14h]
    const cg_s *cgameGlob; // [esp+48h] [ebp-Ch]

    if ( text )
    {
        if ( color[3] != 0.0 )
        {
            cgameGlob = CG_GetLocalClientGlobals(localClientNum);
            xScale = R_NormalizedTextScale(font, scale);
            CL_DrawTextWithCOD7TypeWriterEffects(
                &scrPlaceView[localClientNum],
                text,
                64,
                font,
                rect->x,
                rect->y,
                0.0,
                rect->horzAlign,
                rect->vertAlign,
                xScale,
                xScale,
                color,
                item->window.style,
                item->window.outlineColor,
                cgMedia.textDecodeCharacters,
                cgMedia.textDecodeCharactersGlow,
                cgameGlob->typewriterStartTime,
                typeWriterCod7LetterFXTime->current.integer,
                4000,
                1000);
        }
    }
}

void __cdecl CG_DrawPlayerRedationText(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                Material *material,
                int textStyle,
                itemDef_s *item,
                char *text)
{
    float xScale; // [esp+44h] [ebp-10h]
    const cg_s *cgameGlob; // [esp+4Ch] [ebp-8h]

    if ( text )
    {
        if ( color[3] != 0.0 )
        {
            cgameGlob = CG_GetLocalClientGlobals(localClientNum);
            xScale = R_NormalizedTextScale(font, scale);
            CL_DrawTextWithRedactEffects(
                &scrPlaceView[localClientNum],
                text,
                0x7FFFFFFF,
                font,
                rect->x,
                rect->y,
                rect->horzAlign,
                rect->vertAlign,
                xScale,
                xScale,
                color,
                item->window.style,
                item->window.outlineColor,
                cgMedia.textDecodeCharacters,
                cgMedia.textDecodeCharactersGlow,
                cgameGlob->typewriterStartTime,
                typeWriterCod7LetterFXTime->current.integer,
                redactionDisplayTime->current.integer,
                redactionFadeDuration->current.integer,
                redactionStartStrokeTime->current.integer,
                redactionStrokeTime->current.integer);
        }
    }
}

void __cdecl CG_DrawPlayerPopInText(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                Material *material,
                int textStyle,
                itemDef_s *item,
                char *text)
{
    float xScale; // [esp+40h] [ebp-14h]
    const cg_s *cgameGlob; // [esp+48h] [ebp-Ch]

    if ( text )
    {
        if ( color[3] != 0.0 )
        {
            cgameGlob = CG_GetLocalClientGlobals(localClientNum);
            xScale = R_NormalizedTextScale(font, scale);
            CL_DrawTextWithPopInEffects(
                &scrPlaceView[localClientNum],
                text,
                64,
                font,
                rect->x,
                rect->y,
                0.0,
                rect->horzAlign,
                rect->vertAlign,
                xScale,
                xScale,
                color,
                textStyle,
                item->window.outlineColor,
                cgMedia.textDecodeCharacters,
                cgMedia.textDecodeCharactersGlow,
                cgameGlob->popUpSwayStartTime,
                popInLetterFXTime->current.integer,
                4000,
                1000);
        }
    }
}

void __cdecl CG_CompassDrawPlayerNorthCoord(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Font_s *font,
                Material *material,
                float *color,
                int textStyle)
{
    int v8; // eax
    int v9; // eax
    float textW; // [esp+2Ch] [ebp-30h]
    float textWa; // [esp+2Ch] [ebp-30h]
    const cg_s *cgameGlob; // [esp+34h] [ebp-28h]
    float coord; // [esp+38h] [ebp-24h]
    float scale; // [esp+40h] [ebp-1Ch]
    float x; // [esp+44h] [ebp-18h] BYREF
    float y; // [esp+48h] [ebp-14h] BYREF
    float h; // [esp+4Ch] [ebp-10h] BYREF
    float smallscale; // [esp+50h] [ebp-Ch]
    char *text; // [esp+54h] [ebp-8h]
    float w; // [esp+58h] [ebp-4h] BYREF

    if ( compassType
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                    842,
                    0,
                    "%s",
                    "compassType == COMPASS_TYPE_PARTIAL") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    color[3] = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    if ( color[3] != 0.0 )
    {
        CG_CompassCalcDimensions(compassType, cgameGlob, parentRect, rect, &x, &y, &w, &h);
        coord = (float)((float)(1.0 / compassCoords->current.vector[2])
                                    * (float)((float)(cgameGlob->compassNorth[0] * cgameGlob->refdef.vieworg[0])
                                                    + (float)(cgameGlob->compassNorth[1] * cgameGlob->refdef.vieworg[1])))
                    + compassCoords->current.vector[1];
        v8 = R_TextHeight(font);
        scale = w / (float)v8;
        smallscale = scale * 0.80000001;
        text = va("%i ", (int)coord / 100);
        v9 = R_TextWidth(text, 10, font);
        textW = (float)v9 * smallscale;
        CL_DrawTextRotate(
            &scrPlaceView[localClientNum],
            text,
            40,
            font,
            (float)(w * 0.80000001) + x,
            y + h,
            -90.0,
            rect->horzAlign,
            rect->vertAlign,
            smallscale,
            smallscale,
            color,
            textStyle);
        y = y - textW;
        text = va("%i ", (int)coord % 100);
        textWa = (float)R_TextWidth(text, 10, font) * scale;
        CL_DrawTextRotate(
            &scrPlaceView[localClientNum],
            text,
            40,
            font,
            x + w,
            y + h,
            -90.0,
            rect->horzAlign,
            rect->vertAlign,
            scale,
            scale,
            color,
            textStyle);
        y = y - textWa;
        text = va("%.4f", (float)(coord - (float)(int)coord));
        CL_DrawTextRotate(
            &scrPlaceView[localClientNum],
            text + 2,
            40,
            font,
            x + w,
            y + h,
            -90.0,
            rect->horzAlign,
            rect->vertAlign,
            smallscale,
            smallscale,
            color,
            textStyle);
    }
}

void __cdecl CG_CompassDrawPlayerEastCoord(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Font_s *font,
                Material *material,
                float *color,
                int textStyle)
{
    int v8; // eax
    int v9; // eax
    float textW; // [esp+24h] [ebp-38h]
    float textWa; // [esp+24h] [ebp-38h]
    const cg_s *cgameGlob; // [esp+2Ch] [ebp-30h]
    float coord; // [esp+30h] [ebp-2Ch]
    float scale; // [esp+38h] [ebp-24h]
    float x; // [esp+3Ch] [ebp-20h] BYREF
    float y; // [esp+40h] [ebp-1Ch] BYREF
    float east[2]; // [esp+44h] [ebp-18h]
    float h; // [esp+4Ch] [ebp-10h] BYREF
    float smallscale; // [esp+50h] [ebp-Ch]
    char *text; // [esp+54h] [ebp-8h]
    float w; // [esp+58h] [ebp-4h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( compassType
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                    895,
                    0,
                    "%s",
                    "compassType == COMPASS_TYPE_PARTIAL") )
    {
        __debugbreak();
    }
    color[3] = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    if ( color[3] != 0.0 )
    {
        CG_CompassCalcDimensions(compassType, cgameGlob, parentRect, rect, &x, &y, &w, &h);
        east[0] = cgameGlob->compassNorth[1];
        east[1] = -cgameGlob->compassNorth[0];
        coord = (float)((float)(1.0 / compassCoords->current.vector[2])
                                    * (float)((float)(east[0] * cgameGlob->refdef.vieworg[0])
                                                    + (float)(east[1] * cgameGlob->refdef.vieworg[1])))
                    + compassCoords->current.value;
        v8 = R_TextHeight(font);
        scale = h / (float)v8;
        smallscale = scale * 0.85000002;
        text = va("%i ", (int)coord / 100);
        v9 = R_TextWidth(text, 10, font);
        textW = (float)v9 * smallscale;
        CL_DrawText(
            &scrPlaceView[localClientNum],
            text,
            40,
            font,
            x,
            (float)(h * 0.85000002) + y,
            rect->horzAlign,
            rect->vertAlign,
            smallscale,
            smallscale,
            color,
            textStyle);
        x = x + textW;
        text = va("%i ", (int)coord % 100);
        textWa = (float)R_TextWidth(text, 10, font) * scale;
        CL_DrawText(
            &scrPlaceView[localClientNum],
            text,
            40,
            font,
            x,
            y + h,
            rect->horzAlign,
            rect->vertAlign,
            scale,
            scale,
            color,
            textStyle);
        x = x + textWa;
        text = va("%.4f", (float)(coord - (float)(int)coord));
        CL_DrawText(
            &scrPlaceView[localClientNum],
            text + 2,
            40,
            font,
            x,
            y + h,
            rect->horzAlign,
            rect->vertAlign,
            smallscale,
            smallscale,
            color,
            textStyle);
    }
}

void __cdecl CG_CompassDrawPlayerNCoordScroll(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Font_s *font,
                Material *material,
                float *color,
                int textStyle)
{
    int v8; // eax
    int textW; // [esp+2Ch] [ebp-34h]
    const cg_s *cgameGlob; // [esp+30h] [ebp-30h]
    float coorda; // [esp+34h] [ebp-2Ch]
    float coord; // [esp+34h] [ebp-2Ch]
    float pixelY; // [esp+3Ch] [ebp-24h]
    float pixelPerCoord; // [esp+40h] [ebp-20h]
    float textHeight; // [esp+44h] [ebp-1Ch]
    float scale; // [esp+48h] [ebp-18h]
    float x; // [esp+4Ch] [ebp-14h] BYREF
    float y; // [esp+50h] [ebp-10h] BYREF
    float h; // [esp+54h] [ebp-Ch] BYREF
    char *text; // [esp+58h] [ebp-8h]
    float w; // [esp+5Ch] [ebp-4h] BYREF

    if ( compassType
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                    948,
                    0,
                    "%s",
                    "compassType == COMPASS_TYPE_PARTIAL") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    color[3] = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    if ( color[3] != 0.0 )
    {
        CG_CompassCalcDimensions(compassType, cgameGlob, parentRect, rect, &x, &y, &w, &h);
        coorda = (float)((float)(0.5 * compassMaxRange->current.value) / compassCoords->current.vector[2])
                     + (float)((float)((float)((float)(cgameGlob->compassNorth[0] * cgameGlob->refdef.vieworg[0])
                                                                     + (float)(cgameGlob->compassNorth[1] * cgameGlob->refdef.vieworg[1]))
                                                     / compassCoords->current.vector[2])
                                     + compassCoords->current.vector[1]);
        pixelPerCoord = (float)(compassCoords->current.vector[2] * h) / compassMaxRange->current.value;
        pixelY = y - (float)((float)(1.0 - (float)(coorda - (float)(int)coorda)) * pixelPerCoord);
        coord = coorda + (float)(1.0 - (float)(coorda - (float)(int)coorda));
        v8 = R_TextWidth("99", 2, font);
        scale = w / (float)v8;
        textHeight = (float)R_TextHeight(font) * scale;
        while ( (float)(y + textHeight) > pixelY )
        {
            pixelY = pixelY + pixelPerCoord;
            coord = coord - 1.0;
        }
        while ( (float)(y + h) > pixelY )
        {
            text = va("%2i", (int)coord % 100);
            textW = R_TextWidth(text, 10, font);
            CL_DrawText(
                &scrPlaceView[localClientNum],
                text,
                40,
                font,
                x,
                pixelY,
                rect->horzAlign,
                rect->vertAlign,
                w / (float)textW,
                w / (float)textW,
                color,
                textStyle);
            pixelY = pixelY + pixelPerCoord;
            coord = coord - 1.0;
        }
    }
}

void __cdecl CG_CompassDrawPlayerECoordScroll(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Font_s *font,
                Material *material,
                float *color,
                int textStyle)
{
    int v8; // eax
    int integerPortion; // [esp+24h] [ebp-40h]
    const cg_s *cgameGlob; // [esp+2Ch] [ebp-38h]
    float coord; // [esp+30h] [ebp-34h]
    float textWidth; // [esp+34h] [ebp-30h]
    float pixelPerCoord; // [esp+3Ch] [ebp-28h]
    float pixelX; // [esp+40h] [ebp-24h]
    float scale; // [esp+44h] [ebp-20h]
    float x; // [esp+48h] [ebp-1Ch] BYREF
    float y; // [esp+4Ch] [ebp-18h] BYREF
    float east[2]; // [esp+50h] [ebp-14h]
    float h; // [esp+58h] [ebp-Ch] BYREF
    char *text; // [esp+5Ch] [ebp-8h]
    float w; // [esp+60h] [ebp-4h] BYREF

    if ( compassType
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                    1010,
                    0,
                    "%s",
                    "compassType == COMPASS_TYPE_PARTIAL") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    color[3] = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    if ( color[3] != 0.0 )
    {
        CG_CompassCalcDimensions(compassType, cgameGlob, parentRect, rect, &x, &y, &w, &h);
        east[0] = cgameGlob->compassNorth[1];
        east[1] = -cgameGlob->compassNorth[0];
        coord = (float)((float)((float)((float)(east[0] * cgameGlob->refdef.vieworg[0])
                                                                    + (float)(east[1] * cgameGlob->refdef.vieworg[1]))
                                                    / compassCoords->current.vector[2])
                                    + compassCoords->current.value)
                    - (float)((float)(0.5 * compassMaxRange->current.value) / compassCoords->current.vector[2]);
        pixelPerCoord = (float)(compassCoords->current.vector[2] * w) / compassMaxRange->current.value;
        pixelX = x - (float)((float)(coord - (float)(int)coord) * pixelPerCoord);
        integerPortion = (int)coord;
        v8 = R_TextHeight(font);
        scale = h / (float)v8;
        textWidth = (float)R_TextWidth("99", 2, font) * scale;
        while ( (float)((float)(compassECoordCutoff->current.value * compassSize->current.value) + x) > pixelX )
        {
            pixelX = pixelX + pixelPerCoord;
            ++integerPortion;
        }
        while ( (float)((float)(x + w) - textWidth) > pixelX )
        {
            text = va("%2i", integerPortion % 100);
            CL_DrawText(
                &scrPlaceView[localClientNum],
                text,
                40,
                font,
                pixelX,
                y + h,
                rect->horzAlign,
                rect->vertAlign,
                scale,
                scale,
                color,
                textStyle);
            pixelX = pixelX + pixelPerCoord;
            ++integerPortion;
        }
    }
}

void __cdecl CG_CompassDrawStatic(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Material *material,
                float *color)
{
    float texCenter; // [esp+40h] [ebp-4Ch]
    float texCenter_4; // [esp+44h] [ebp-48h]
    float delta; // [esp+4Ch] [ebp-40h]
    float delta_4; // [esp+50h] [ebp-3Ch]
    const cg_s *cgameGlob; // [esp+54h] [ebp-38h]
    float texRadius; // [esp+58h] [ebp-34h]
    float scaleFinalT; // [esp+64h] [ebp-28h]
    float x; // [esp+68h] [ebp-24h] BYREF
    float y; // [esp+6Ch] [ebp-20h] BYREF
    float south[2]; // [esp+70h] [ebp-1Ch]
    float east[2]; // [esp+78h] [ebp-14h]
    float h; // [esp+80h] [ebp-Ch] BYREF
    float scaleFinalS; // [esp+84h] [ebp-8h]
    float w; // [esp+88h] [ebp-4h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->globalScramblerActive )
    {
        if ( cgameGlob->compassMapWorldSize[0] == 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                        1084,
                        0,
                        "%s",
                        "cgameGlob->compassMapWorldSize[0] != 0") )
        {
            __debugbreak();
        }
        if ( cgameGlob->compassMapWorldSize[1] == 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                        1085,
                        0,
                        "%s",
                        "cgameGlob->compassMapWorldSize[1] != 0") )
        {
            __debugbreak();
        }
        if ( compassType )
        {
            if ( compassType != COMPASS_TYPE_FULL
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                            1119,
                            0,
                            "%s",
                            "compassType == COMPASS_TYPE_FULL") )
            {
                __debugbreak();
            }
            CG_CompassCalcDimensions(compassType, cgameGlob, parentRect, rect, &x, &y, &w, &h);
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                x,
                y,
                w,
                h,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                color,
                material);
        }
        else
        {
            CG_ScaleCompass();
            east[0] = cgameGlob->compassNorth[1];
            east[1] = -cgameGlob->compassNorth[0];
            south[0] = -cgameGlob->compassNorth[0];
            south[1] = -cgameGlob->compassNorth[1];
            delta = cgameGlob->refdef.vieworg[0] - cgameGlob->compassMapUpperLeft[0];
            delta_4 = cgameGlob->refdef.vieworg[1] - cgameGlob->compassMapUpperLeft[1];
            texCenter = (float)((float)(delta * east[0]) + (float)(delta_4 * east[1])) / cgameGlob->compassMapWorldSize[0];
            texCenter_4 = (float)((float)(delta * south[0]) + (float)(delta_4 * south[1])) / cgameGlob->compassMapWorldSize[1];
            if ( cgameGlob->compassMapWorldSize[0] <= cgameGlob->compassMapWorldSize[1] )
            {
                texRadius = (float)(0.5 * compassMaxRange->current.value) / cgameGlob->compassMapWorldSize[1];
                scaleFinalS = cgameGlob->compassMapWorldSize[1] / cgameGlob->compassMapWorldSize[0];
                scaleFinalT = 1.0f;
            }
            else
            {
                texRadius = (float)(0.5 * compassMaxRange->current.value) / cgameGlob->compassMapWorldSize[0];
                scaleFinalS = 1.0f;
                scaleFinalT = cgameGlob->compassMapWorldSize[0] / cgameGlob->compassMapWorldSize[1];
            }
            CG_CompassCalcDimensions(COMPASS_TYPE_PARTIAL, cgameGlob, parentRect, rect, &x, &y, &w, &h);
            CL_DrawStretchPicRotatedST(
                &scrPlaceView[localClientNum],
                x,
                y,
                w,
                h,
                rect->horzAlign,
                rect->vertAlign,
                texCenter,
                texCenter_4,
                texRadius,
                scaleFinalS,
                scaleFinalT,
                0.0,
                color,
                material);
        }
    }
}

void __cdecl CG_UpdateEnemyScramblerAlpha(int localClientNum)
{
    float enemyScramblerAlpha; // [esp+14h] [ebp-14h]
    cg_s *cgameGlob; // [esp+1Ch] [ebp-Ch]
    bool enemy; // [esp+22h] [ebp-6h]
    float enemyScramblerDistance; // [esp+24h] [ebp-4h]

    enemyScramblerAlpha = 0.0f;
    enemy = 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->closestEnemyScrambler && ((*((unsigned int *)cgameGlob->closestEnemyScrambler + 201) >> 1) & 1) != 0 )
    {
        if ( (cgameGlob->closestEnemyScrambler->nextState.faction.iHeadIconTeam & 3) != cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team
            || (cgameGlob->closestEnemyScrambler->nextState.faction.iHeadIconTeam & 3) == 0
            && (int)cgameGlob->closestEnemyScrambler->nextState.faction.iHeadIconTeam >> 2 != cgameGlob->bgs.clientinfo[cgameGlob->clientNum].clientNum )
        {
            enemy = 1;
        }
        if ( enemy )
        {
            enemyScramblerDistance = Vec3DistanceSq(
                                                                 cgameGlob->predictedPlayerEntity.pose.origin,
                                                                 cgameGlob->closestEnemyScrambler->pose.origin);
            if ( enemyScramblerDistance <= 1000000.0 )
                enemyScramblerAlpha = 1.0 - (float)((float)(enemyScramblerDistance - 250000.0) / 750000.0);
        }
    }
    if ( enemyScramblerAlpha > 1.0 )
        enemyScramblerAlpha = 1.0f;
    cgameGlob->scramblerEnemyAlpha = enemyScramblerAlpha;
}

double __cdecl CG_GetScramblerEnemyAlpha(int localClientNum)
{
    return CG_GetLocalClientGlobals(localClientNum)->scramblerEnemyAlpha;
}

void __cdecl CG_CompassDrawScrambler(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Material *material,
                float *color)
{
    float texCenter; // [esp+40h] [ebp-4Ch]
    float texCenter_4; // [esp+44h] [ebp-48h]
    float delta; // [esp+4Ch] [ebp-40h]
    float delta_4; // [esp+50h] [ebp-3Ch]
    const cg_s *cgameGlob; // [esp+54h] [ebp-38h]
    float texRadius; // [esp+58h] [ebp-34h]
    float scaleFinalT; // [esp+64h] [ebp-28h]
    float x; // [esp+68h] [ebp-24h] BYREF
    float y; // [esp+6Ch] [ebp-20h] BYREF
    float south[2]; // [esp+70h] [ebp-1Ch]
    float east[2]; // [esp+78h] [ebp-14h]
    float h; // [esp+80h] [ebp-Ch] BYREF
    float scaleFinalS; // [esp+84h] [ebp-8h]
    float w; // [esp+88h] [ebp-4h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    color[3] = CG_GetScramblerEnemyAlpha(localClientNum);
    if ( color[3] != 0.0 )
    {
        if ( cgameGlob->compassMapWorldSize[0] == 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                        1204,
                        0,
                        "%s",
                        "cgameGlob->compassMapWorldSize[0] != 0") )
        {
            __debugbreak();
        }
        if ( cgameGlob->compassMapWorldSize[1] == 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                        1205,
                        0,
                        "%s",
                        "cgameGlob->compassMapWorldSize[1] != 0") )
        {
            __debugbreak();
        }
        if ( compassType )
        {
            if ( compassType != COMPASS_TYPE_FULL
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                            1239,
                            0,
                            "%s",
                            "compassType == COMPASS_TYPE_FULL") )
            {
                __debugbreak();
            }
            CG_CompassCalcDimensions(compassType, cgameGlob, parentRect, rect, &x, &y, &w, &h);
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                x,
                y,
                w,
                h,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                color,
                material);
        }
        else
        {
            CG_ScaleCompass();
            east[0] = cgameGlob->compassNorth[1];
            east[1] = -cgameGlob->compassNorth[0];
            south[0] = -cgameGlob->compassNorth[0];
            south[1] = -cgameGlob->compassNorth[1];
            delta = cgameGlob->refdef.vieworg[0] - cgameGlob->compassMapUpperLeft[0];
            delta_4 = cgameGlob->refdef.vieworg[1] - cgameGlob->compassMapUpperLeft[1];
            texCenter = (float)((float)(delta * east[0]) + (float)(delta_4 * east[1])) / cgameGlob->compassMapWorldSize[0];
            texCenter_4 = (float)((float)(delta * south[0]) + (float)(delta_4 * south[1])) / cgameGlob->compassMapWorldSize[1];
            if ( cgameGlob->compassMapWorldSize[0] <= cgameGlob->compassMapWorldSize[1] )
            {
                texRadius = (float)(0.5 * compassMaxRange->current.value) / cgameGlob->compassMapWorldSize[1];
                scaleFinalS = cgameGlob->compassMapWorldSize[1] / cgameGlob->compassMapWorldSize[0];
                scaleFinalT = 1.0f;
            }
            else
            {
                texRadius = (float)(0.5 * compassMaxRange->current.value) / cgameGlob->compassMapWorldSize[0];
                scaleFinalS = 1.0f;
                scaleFinalT = cgameGlob->compassMapWorldSize[0] / cgameGlob->compassMapWorldSize[1];
            }
            CG_CompassCalcDimensions(COMPASS_TYPE_PARTIAL, cgameGlob, parentRect, rect, &x, &y, &w, &h);
            CL_DrawStretchPicRotatedST(
                &scrPlaceView[localClientNum],
                x,
                y,
                w,
                h,
                rect->horzAlign,
                rect->vertAlign,
                texCenter,
                texCenter_4,
                texRadius,
                scaleFinalS,
                scaleFinalT,
                0.0,
                color,
                material);
        }
    }
}

void __cdecl CG_CompassDrawRound(int localClientNum, const rectDef_s *rect, Material *material, float *color)
{
    unsigned __int16 CodeConst_GenericParam0; // ax
    const cg_s *cgameGlob; // [esp+24h] [ebp-40h]
    rectDef_s scissorRect; // [esp+28h] [ebp-3Ch] BYREF
    float rotation; // [esp+40h] [ebp-24h]
    float x; // [esp+44h] [ebp-20h]
    float y; // [esp+48h] [ebp-1Ch]
    float h; // [esp+4Ch] [ebp-18h]
    _QWORD c0[2]; // [esp+50h] [ebp-14h] BYREF
    float w; // [esp+60h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    color[3] = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, COMPASS_TYPE_PARTIAL) * color[3];
    if ( color[3] != 0.0 )
    {
        rotation = cgameGlob->refdefViewAngles[1] - cgameGlob->compassNorthYaw;
        x = rect->x;
        y = rect->y;
        w = rect->w;
        h = rect->h;
        scissorRect = *rect;
        ScrPlace_ApplyRect(
            &scrPlaceView[localClientNum],
            &scissorRect.x,
            &scissorRect.y,
            &scissorRect.w,
            &scissorRect.h,
            scissorRect.horzAlign,
            scissorRect.vertAlign);
        c0[0] = *(_QWORD *)&scissorRect.x;
        c0[1] = *(_QWORD *)&scissorRect.w;
        CodeConst_GenericParam0 = GetCodeConst_GenericParam0();
        CL_SetCustomConstant(CodeConst_GenericParam0, (const float *)c0);
        CG_DrawRotatedPic(
            &scrPlaceView[localClientNum],
            x,
            y,
            w,
            h,
            rect->horzAlign,
            rect->vertAlign,
            rotation,
            color,
            material);
    }
}

void __cdecl CG_CompassDrawPlayerMap(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Material *material,
                float *color,
                bool grid)
{
    float texCenter; // [esp+40h] [ebp-74h]
    float texCenter_4; // [esp+44h] [ebp-70h]
    float delta; // [esp+4Ch] [ebp-68h]
    float delta_4; // [esp+50h] [ebp-64h]
    const cg_s *cgameGlob; // [esp+54h] [ebp-60h]
    float texRadius; // [esp+70h] [ebp-44h]
    float rotation; // [esp+74h] [ebp-40h]
    float scaleFinalT; // [esp+7Ch] [ebp-38h]
    int rows; // [esp+80h] [ebp-34h]
    float x; // [esp+84h] [ebp-30h] BYREF
    float y; // [esp+88h] [ebp-2Ch] BYREF
    float south[2]; // [esp+8Ch] [ebp-28h]
    float east[2]; // [esp+94h] [ebp-20h]
    float h; // [esp+9Ch] [ebp-18h] BYREF
    float scaleFinalS; // [esp+A0h] [ebp-14h]
    float w; // [esp+A4h] [ebp-10h] BYREF
    float mapCenter[2]; // [esp+A8h] [ebp-Ch]
    int cols; // [esp+B0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    color[3] = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType) * color[3];
    if ( grid )
        color[3] = CG_FadeCompassIcons(localClientNum, 0) * color[3];
    if ( color[3] != 0.0 )
    {
        mapCenter[0] = cgameGlob->refdef.vieworg[0];
        mapCenter[1] = cgameGlob->refdef.vieworg[1];
        if ( cgameGlob->compassMapWorldSize[0] == 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                        1313,
                        0,
                        "%s",
                        "cgameGlob->compassMapWorldSize[0] != 0") )
        {
            __debugbreak();
        }
        if ( cgameGlob->compassMapWorldSize[1] == 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                        1314,
                        0,
                        "%s",
                        "cgameGlob->compassMapWorldSize[1] != 0") )
        {
            __debugbreak();
        }
        cols = compassGridCols->current.integer;
        rows = compassGridRows->current.integer;
        if ( compassType )
        {
            if ( compassType != COMPASS_TYPE_FULL
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                            1360,
                            0,
                            "%s",
                            "compassType == COMPASS_TYPE_FULL") )
            {
                __debugbreak();
            }
            CG_CompassCalcDimensions(compassType, cgameGlob, parentRect, rect, &x, &y, &w, &h);
            if ( grid )
            {
                ScrPlace_ApplyRect(&scrPlaceView[localClientNum], &x, &y, &w, &h, rect->horzAlign, rect->vertAlign);
                R_AddCmdSetScissorValues(
                    1,
                    (int)(float)(x + 3.0),
                    (int)(float)(y + 3.0),
                    (int)(float)(w - 3.0),
                    (int)(float)(h - 3.0));
                CL_DrawStretchPicPhysical(x, y, w, h, 0.0, 0.0, (float)cols * 1.0, (float)rows * 1.0, color, material);
                R_AddCmdSetScissorValues(0, 0, 0, 0, 0);
            }
            else
            {
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    x,
                    y,
                    w,
                    h,
                    rect->horzAlign,
                    rect->vertAlign,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    color,
                    cgameGlob->compassMapMaterial);
            }
        }
        else
        {
            CG_ScaleCompass();
            east[0] = cgameGlob->compassNorth[1];
            east[1] = -cgameGlob->compassNorth[0];
            south[0] = -cgameGlob->compassNorth[0];
            south[1] = -cgameGlob->compassNorth[1];
            delta = mapCenter[0] - cgameGlob->compassMapUpperLeft[0];
            delta_4 = mapCenter[1] - cgameGlob->compassMapUpperLeft[1];
            texCenter = (float)((float)(delta * east[0]) + (float)(delta_4 * east[1])) / cgameGlob->compassMapWorldSize[0];
            texCenter_4 = (float)((float)(delta * south[0]) + (float)(delta_4 * south[1])) / cgameGlob->compassMapWorldSize[1];
            if ( cgameGlob->compassMapWorldSize[0] <= cgameGlob->compassMapWorldSize[1] )
            {
                texRadius = (float)(0.5 * compassMaxRange->current.value) / cgameGlob->compassMapWorldSize[1];
                scaleFinalS = cgameGlob->compassMapWorldSize[1] / cgameGlob->compassMapWorldSize[0];
                scaleFinalT = 1.0f;
            }
            else
            {
                texRadius = (float)(0.5 * compassMaxRange->current.value) / cgameGlob->compassMapWorldSize[0];
                scaleFinalS = 1.0f;
                scaleFinalT = cgameGlob->compassMapWorldSize[0] / cgameGlob->compassMapWorldSize[1];
            }
            if ( compassRotation->current.enabled )
                rotation = -(cgameGlob->refdefViewAngles[1] - cgameGlob->compassNorthYaw);
            else
                rotation = 0.0f;
            CG_CompassCalcDimensions(COMPASS_TYPE_PARTIAL, cgameGlob, parentRect, rect, &x, &y, &w, &h);
            if ( grid )
                CL_DrawStretchPicRotatedST(
                    &scrPlaceView[localClientNum],
                    x,
                    y,
                    w,
                    h,
                    rect->horzAlign,
                    rect->vertAlign,
                    (float)cols * texCenter,
                    (float)rows * texCenter_4,
                    texRadius,
                    (float)cols * scaleFinalS,
                    (float)rows * scaleFinalT,
                    rotation,
                    color,
                    material);
            else
                CL_DrawStretchPicRotatedST(
                    &scrPlaceView[localClientNum],
                    x,
                    y,
                    w,
                    h,
                    rect->horzAlign,
                    rect->vertAlign,
                    texCenter,
                    texCenter_4,
                    texRadius,
                    scaleFinalS,
                    scaleFinalT,
                    rotation,
                    color,
                    cgameGlob->compassMapMaterial);
        }
    }
}

float yawVector[3];

void __cdecl CG_CompassDrawPlayerMapLocationSelector(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Material *material,
                float *color)
{
    float texMax; // [esp+24h] [ebp-68h]
    float texMax_4; // [esp+28h] [ebp-64h]
    const char *mtlName; // [esp+2Ch] [ebp-60h]
    unsigned int mtlIndex; // [esp+30h] [ebp-5Ch]
    float diffAngles[3]; // [esp+34h] [ebp-58h] BYREF
    const cg_s *cgameGlob; // [esp+40h] [ebp-4Ch]
    float radius; // [esp+44h] [ebp-48h]
    Material *selectorMaterial; // [esp+48h] [ebp-44h]
    float posScreen[2]; // [esp+4Ch] [ebp-40h]
    float yawTo; // [esp+54h] [ebp-38h]
    float quadRad; // [esp+58h] [ebp-34h]
    float quadMax[2]; // [esp+5Ch] [ebp-30h]
    float quadMin[2]; // [esp+64h] [ebp-28h]
    rectDef_s scaledRect; // [esp+6Ch] [ebp-20h] BYREF
    float texMin[2]; // [esp+84h] [ebp-8h]

    if ( !parentRect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 1400, 0, "%s", "parentRect") )
    {
        __debugbreak();
    }
    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 1401, 0, "%s", "rect") )
        __debugbreak();
    if ( compassType != COMPASS_TYPE_FULL
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                    1403,
                    0,
                    "%s",
                    "compassType == COMPASS_TYPE_FULL") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->compassMapWorldSize[0] == 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                    1407,
                    0,
                    "%s",
                    "cgameGlob->compassMapWorldSize[0]") )
    {
        __debugbreak();
    }
    if ( cgameGlob->compassMapWorldSize[1] == 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                    1408,
                    0,
                    "%s",
                    "cgameGlob->compassMapWorldSize[1]") )
    {
        __debugbreak();
    }
    if ( cgameGlob->predictedPlayerState.locationSelectionInfo )
    {
        CG_CompassCalcDimensions(
            compassType,
            cgameGlob,
            parentRect,
            rect,
            &scaledRect.x,
            &scaledRect.y,
            &scaledRect.w,
            &scaledRect.h);
        mtlIndex = (cgameGlob->predictedPlayerState.locationSelectionInfo & 0xF) - 1;
        if ( mtlIndex >= 0xF
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                        1416,
                        0,
                        "mtlIndex doesn't index MAX_LOC_SEL_MTLS\n\t%i not in [0, %i)",
                        mtlIndex,
                        15) )
        {
            __debugbreak();
        }
        mtlName = CL_GetConfigString(mtlIndex + 1553);
        selectorMaterial = Material_RegisterHandle(mtlName, 7);
        radius = (float)(cgameGlob->predictedPlayerState.locationSelectionInfo >> 4) / 15.0;
        if ( radius > 0.0 )
        {
            quadRad = radius * scaledRect.h;
            posScreen[0] = (float)(scaledRect.w * cgameGlob->selectedLocation[0]) + scaledRect.x;
            posScreen[1] = (float)(scaledRect.h * cgameGlob->selectedLocation[1]) + scaledRect.y;
            texMin[0] = (float)((float)(scaledRect.x - posScreen[0]) / (float)(radius * scaledRect.h)) + 0.5;
            texMin[1] = (float)((float)(scaledRect.y - posScreen[1]) / (float)(radius * scaledRect.h)) + 0.5;
            texMax = (float)((float)((float)(scaledRect.x + scaledRect.w) - posScreen[0]) / (float)(radius * scaledRect.h))
                         + 0.5;
            texMax_4 = (float)((float)((float)(scaledRect.y + scaledRect.h) - posScreen[1]) / (float)(radius * scaledRect.h))
                             + 0.5;
            quadMin[0] = scaledRect.x;
            quadMin[1] = scaledRect.y;
            quadMax[0] = scaledRect.x + scaledRect.w;
            quadMax[1] = scaledRect.y + scaledRect.h;
            if ( texMin[0] < 0.0 )
            {
                quadMin[0] = (float)((float)((float)(0.0 - texMin[0]) / (float)(texMax - texMin[0]))
                                                     * (float)(quadMax[0] - quadMin[0]))
                                     + quadMin[0];
                texMin[0] = 0.0f;
            }
            if ( texMin[1] < 0.0 )
            {
                quadMin[1] = (float)((float)((float)(0.0 - texMin[1]) / (float)(texMax_4 - texMin[1]))
                                                     * (float)(quadMax[1] - quadMin[1]))
                                     + quadMin[1];
                texMin[1] = 0.0f;
            }
            if ( texMax > 1.0 )
                quadMax[0] = (float)((float)((float)(1.0 - texMin[0]) / (float)(texMax - texMin[0]))
                                                     * (float)(quadMax[0] - quadMin[0]))
                                     + quadMin[0];
            if ( texMax_4 > 1.0 )
                quadMax[1] = (float)((float)((float)(1.0 - texMin[1]) / (float)(texMax_4 - texMin[1]))
                                                     * (float)(quadMax[1] - quadMin[1]))
                                     + quadMin[1];
            yawTo = 0.0f;
            if ( cgameGlob->compassMapWorldSize[1] <= 0.0 || radius <= 0.0 )
                radius = radius * 250.0;
            else
                radius = (float)(cgameGlob->compassMapWorldSize[1] / 25.0) * radius;
            *(_QWORD *)yawVector = *(_QWORD *)cgameGlob->selectedYaw;
            vectoangles(yawVector, diffAngles);
            yawTo = diffAngles[1];
            CG_DrawRotatedPic(
                &scrPlaceView[localClientNum],
                posScreen[0] - (float)(radius * 0.5),
                posScreen[1] - (float)(radius * 0.5),
                radius,
                radius,
                rect->horzAlign,
                rect->vertAlign,
                diffAngles[1],
                color,
                selectorMaterial);
        }
    }
}

float yawVector_0[3];
void __cdecl CG_CompassDrawPlayerSelectedLocations(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Material *material,
                float *color)
{
    float diffAngles[3]; // [esp+24h] [ebp-74h] BYREF
    float yawTo; // [esp+30h] [ebp-68h]
    int i; // [esp+34h] [ebp-64h]
    float texMax[2]; // [esp+38h] [ebp-60h]
    const char *mtlName; // [esp+40h] [ebp-58h]
    int mtlIndex; // [esp+44h] [ebp-54h]
    int locationsSelected; // [esp+48h] [ebp-50h]
    const cg_s *cgameGlob; // [esp+4Ch] [ebp-4Ch]
    float radius; // [esp+50h] [ebp-48h]
    Material *selectorMaterial; // [esp+54h] [ebp-44h]
    float posScreen[2]; // [esp+58h] [ebp-40h]
    float quadRad; // [esp+60h] [ebp-38h]
    float quadMax[2]; // [esp+64h] [ebp-34h]
    float quadMin[2]; // [esp+6Ch] [ebp-2Ch]
    rectDef_s scaledRect; // [esp+74h] [ebp-24h] BYREF
    float texMin[2]; // [esp+8Ch] [ebp-Ch]
    bool lastSelected; // [esp+97h] [ebp-1h]

    locationsSelected = 0;
    if ( !parentRect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 1511, 0, "%s", "parentRect") )
    {
        __debugbreak();
    }
    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 1512, 0, "%s", "rect") )
        __debugbreak();
    if ( compassType != COMPASS_TYPE_FULL
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                    1514,
                    0,
                    "%s",
                    "compassType == COMPASS_TYPE_FULL") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->compassMapWorldSize[0] == 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                    1518,
                    0,
                    "%s",
                    "cgameGlob->compassMapWorldSize[0]") )
    {
        __debugbreak();
    }
    if ( cgameGlob->compassMapWorldSize[1] == 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                    1519,
                    0,
                    "%s",
                    "cgameGlob->compassMapWorldSize[1]") )
    {
        __debugbreak();
    }
    if ( cgameGlob->predictedPlayerState.locationSelectionInfo
        && cgameGlob->predictedPlayerState.locationSelectionType == LOC_SEL_MORTAR )
    {
        CG_CompassCalcDimensions(
            compassType,
            cgameGlob,
            parentRect,
            rect,
            &scaledRect.x,
            &scaledRect.y,
            &scaledRect.w,
            &scaledRect.h);
        mtlIndex = (cgameGlob->predictedPlayerState.locationSelectionInfo & 0xF) - 1;
        if ( (unsigned int)mtlIndex >= 0xF
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                        1530,
                        0,
                        "mtlIndex doesn't index MAX_LOC_SEL_MTLS\n\t%i not in [0, %i)",
                        mtlIndex,
                        15) )
        {
            __debugbreak();
        }
        mtlName = CL_GetConfigString(mtlIndex + 1553);
        selectorMaterial = Material_RegisterHandle((char *)mtlName, 7);
        lastSelected = 1;
        for ( i = 2; i >= 0; --i )
        {
            radius = (float)(cgameGlob->predictedPlayerState.locationSelectionInfo >> 4) / 15.0;
            if ( radius > 0.0 && (cgameGlob->selectedLocations[i][0] != -1.0 || cgameGlob->selectedLocations[i][1] != -1.0) )
            {
                if ( !lastSelected || (lastSelected = 0, cgameGlob->lastTimeSelected <= cgameGlob->time) )
                {
                    ++locationsSelected;
                    quadRad = radius * scaledRect.h;
                    posScreen[0] = (float)(scaledRect.w * cgameGlob->selectedLocations[i][0]) + scaledRect.x;
                    posScreen[1] = (float)(scaledRect.h * cgameGlob->selectedLocations[i][1]) + scaledRect.y;
                    texMin[0] = (float)((float)(scaledRect.x - posScreen[0]) / (float)(radius * scaledRect.h)) + 0.5;
                    texMin[1] = (float)((float)(scaledRect.y - posScreen[1]) / (float)(radius * scaledRect.h)) + 0.5;
                    texMax[0] = (float)((float)((float)(scaledRect.x + scaledRect.w) - posScreen[0])
                                                        / (float)(radius * scaledRect.h))
                                        + 0.5;
                    texMax[1] = (float)((float)((float)(scaledRect.y + scaledRect.h) - posScreen[1])
                                                        / (float)(radius * scaledRect.h))
                                        + 0.5;
                    quadMin[0] = scaledRect.x;
                    quadMin[1] = scaledRect.y;
                    quadMax[0] = scaledRect.x + scaledRect.w;
                    quadMax[1] = scaledRect.y + scaledRect.h;
                    if ( texMin[0] < 0.0 )
                    {
                        quadMin[0] = (float)((float)((float)(0.0 - texMin[0]) / (float)(texMax[0] - texMin[0]))
                                                             * (float)(quadMax[0] - quadMin[0]))
                                             + quadMin[0];
                        texMin[0] = 0.0f;
                    }
                    if ( texMin[1] < 0.0 )
                    {
                        quadMin[1] = (float)((float)((float)(0.0 - texMin[1]) / (float)(texMax[1] - texMin[1]))
                                                             * (float)(quadMax[1] - quadMin[1]))
                                             + quadMin[1];
                        texMin[1] = 0.0f;
                    }
                    if ( texMax[0] > 1.0 )
                    {
                        quadMax[0] = (float)((float)((float)(1.0 - texMin[0]) / (float)(texMax[0] - texMin[0]))
                                                             * (float)(quadMax[0] - quadMin[0]))
                                             + quadMin[0];
                        texMax[0] = 1.0f;
                    }
                    if ( texMax[1] > 1.0 )
                    {
                        quadMax[1] = (float)((float)((float)(1.0 - texMin[1]) / (float)(texMax[1] - texMin[1]))
                                                             * (float)(quadMax[1] - quadMin[1]))
                                             + quadMin[1];
                        texMax[1] = 1.0f;
                    }
                    yawTo = 0.0f;
                    if ( cgameGlob->compassMapWorldSize[1] <= 0.0 || radius <= 0.0 )
                        radius = radius * 250.0;
                    else
                        radius = (float)(cgameGlob->compassMapWorldSize[1] / 25.0) * radius;
                    *(_QWORD *)yawVector_0 = *(_QWORD *)cgameGlob->selectedYaw;
                    vectoangles(yawVector_0, diffAngles);
                    yawTo = diffAngles[1];
                    posScreen[0] = posScreen[0] - (float)(radius * 0.5);
                    posScreen[1] = posScreen[1] - (float)(radius * 0.5);
                    CG_DrawRotatedPic(
                        &scrPlaceView[localClientNum],
                        posScreen[0],
                        posScreen[1],
                        radius,
                        radius,
                        rect->horzAlign,
                        rect->vertAlign,
                        diffAngles[1],
                        color,
                        cgMedia.compass_mortar_selector);
                }
            }
        }
        if ( locationsSelected > 3
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                        1614,
                        0,
                        "%s",
                        "locationsSelected <= MORTAR_SELECTION_COUNT") )
        {
            __debugbreak();
        }
        Dvar_SetInt((dvar_s *)mortarStrikesLeft, 3 - locationsSelected);
    }
}

void __cdecl CG_CompassDrawPlayer(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                rectDef_s *rect,
                Material *material,
                float *color)
{
    float yawVector[2]; // [esp+30h] [ebp-58h] BYREF
    bool clipped; // [esp+3Bh] [ebp-4Dh]
    float outClipped; // [esp+3Ch] [ebp-4Ch] BYREF
    float v9; // [esp+40h] [ebp-48h]
    float xy[2]; // [esp+44h] [ebp-44h] BYREF
    const cg_s *cgameGlob; // [esp+4Ch] [ebp-3Ch]
    float angle; // [esp+50h] [ebp-38h]
    float centerY; // [esp+54h] [ebp-34h]
    rectDef_s scaledRect; // [esp+58h] [ebp-30h] BYREF
    float x; // [esp+70h] [ebp-18h]
    float y; // [esp+74h] [ebp-14h]
    const playerState_s *ps; // [esp+78h] [ebp-10h]
    float centerX; // [esp+7Ch] [ebp-Ch]
    float h; // [esp+80h] [ebp-8h]
    float w; // [esp+84h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    if ( (cgameGlob->predictedPlayerState.eFlags & 0x4000) != 0 && (ps->eFlags2 & 0x10000000) == 0 )
        return;
    color[3] = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, (CompassType)compassType) * color[3];
    color[3] = CG_FadeCompassIcons(localClientNum, 0) * color[3];
    if ( color[3] == 0.0 )
        return;
    CG_CompassCalcDimensions(
        (CompassType)compassType,
        cgameGlob,
        parentRect,
        rect,
        &scaledRect.x,
        &scaledRect.y,
        &scaledRect.w,
        &scaledRect.h);
    centerX = (float)(scaledRect.w * 0.5) + scaledRect.x;
    centerY = (float)(scaledRect.h * 0.5) + scaledRect.y;
    if ( compassType == COMPASS_TYPE_FULL)
    {
        w = cg_hudMapPlayerWidth->current.value;
        h = cg_hudMapPlayerHeight->current.value;
        xy[0] = 0.0f;
        xy[1] = 0.0f;
        if ( (cgameGlob->predictedPlayerState.otherFlags & 2) != 0 )
            CG_WorldPosToCompass(
                COMPASS_TYPE_FULL,
                cgameGlob,
                &scaledRect,
                0,
                cgameGlob->predictedPlayerState.origin,
                cgameGlob->predictedPlayerState.origin,
                0,
                xy);
        else
            CG_WorldPosToCompass(
                COMPASS_TYPE_FULL,
                cgameGlob,
                &scaledRect,
                0,
                cgameGlob->refdef.vieworg,
                cgameGlob->refdef.vieworg,
                0,
                xy);
        x = xy[0];
        y = xy[1];
    }
    else
    {
        if ( compassType
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                        1704,
                        0,
                        "%s",
                        "compassType == COMPASS_TYPE_PARTIAL") )
        {
            __debugbreak();
        }
        color[3] = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, (CompassType)compassType);
        if ( color[3] == 0.0 )
            return;
        w = compassPlayerWidth->current.value * compassSize->current.value;
        h = compassPlayerHeight->current.value * compassSize->current.value;
        //BLOPS_NULLSUB();
        x = 0.0f;
        y = 0.0f;
    }
    if ( (cgameGlob->predictedPlayerState.eFlags2 & 0x10000000) == 0 )
    {
        if ( compassType || !compassRotation->current.enabled )
            angle = AngleNormalize180(cgameGlob->compassNorthYaw - cgameGlob->refdefViewAngles[1]);
        else
            angle = 0.0f;
        goto LABEL_25;
    }
    outClipped = 0.0f;
    v9 = 0.0f;
    CG_CompassUpYawVector(cgameGlob, yawVector);
    clipped = CG_WorldPosToCompass(
                            (CompassType)compassType,
                            cgameGlob,
                            &scaledRect,
                            yawVector,
                            cgameGlob->refdef.vieworg,
                            cgameGlob->predictedPlayerState.origin,
                            0,
                            &outClipped);
    if ( !clipped || compassClampIcons->current.enabled )
    {
        x = outClipped;
        y = v9;
        if ( compassType || !compassRotation->current.enabled )
            angle = AngleNormalize360(cgameGlob->compassNorthYaw - cgameGlob->predictedPlayerEntity.nextState.lerp.apos.trBase[1]);
        else
            angle = AngleNormalize360(cgameGlob->refdefViewAngles[1] - cgameGlob->predictedPlayerEntity.nextState.lerp.apos.trBase[1]);
LABEL_25:
        CG_DrawRotatedPic(
            &scrPlaceView[localClientNum],
            (float)(centerX - (float)(w * 0.5)) + x,
            (float)(centerY - (float)(h * 0.5)) + y,
            w,
            h,
            rect->horzAlign,
            rect->vertAlign,
            angle,
            color,
            material);
    }
}

void __cdecl CG_CompassDrawBorder(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                rectDef_s *rect,
                Material *material,
                float *color)
{
    const cg_s *cgameGlob; // [esp+30h] [ebp-24h]
    float borderWidth; // [esp+34h] [ebp-20h]
    float border; // [esp+38h] [ebp-1Ch]
    rectDef_s scaledRect; // [esp+3Ch] [ebp-18h] BYREF

    if ( compassType != COMPASS_TYPE_FULL
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                    1769,
                    0,
                    "%s",
                    "compassType == COMPASS_TYPE_FULL") )
    {
        __debugbreak();
    }
    border = cg_hudMapBorderWidth->current.value;
    if ( border > 0.0 )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        color[3] = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
        if ( color[3] != 0.0 )
        {
            CG_CompassCalcDimensions(
                compassType,
                cgameGlob,
                parentRect,
                rect,
                &scaledRect.x,
                &scaledRect.y,
                &scaledRect.w,
                &scaledRect.h);
            if ( border > (float)(scaledRect.w * 0.5) )
                border = scaledRect.w * 0.5;
            if ( border > (float)(scaledRect.h * 0.5) )
                border = scaledRect.h * 0.5;
            borderWidth = border * 2.0;
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                scaledRect.x - border,
                (float)(scaledRect.y + scaledRect.h) - border,
                border * 2.0,
                border * 2.0,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.5,
                0.5,
                1.0,
                color,
                material);
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                scaledRect.x - border,
                scaledRect.y - border,
                borderWidth,
                borderWidth,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                0.5,
                0.5,
                color,
                material);
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                (float)(scaledRect.x + scaledRect.w) - border,
                (float)(scaledRect.y + scaledRect.h) - border,
                borderWidth,
                borderWidth,
                rect->horzAlign,
                rect->vertAlign,
                0.5,
                0.5,
                1.0,
                1.0,
                color,
                material);
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                (float)(scaledRect.x + scaledRect.w) - border,
                scaledRect.y - border,
                borderWidth,
                borderWidth,
                rect->horzAlign,
                rect->vertAlign,
                0.5,
                0.0,
                1.0,
                0.5,
                color,
                material);
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                scaledRect.x - border,
                scaledRect.y + border,
                border * 2.0,
                scaledRect.h - (float)(2.0 * border),
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.5,
                0.5,
                0.5,
                color,
                material);
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                (float)(scaledRect.x + scaledRect.w) - border,
                scaledRect.y + border,
                border * 2.0,
                scaledRect.h - (float)(2.0 * border),
                rect->horzAlign,
                rect->vertAlign,
                0.5,
                0.5,
                1.0,
                0.5,
                color,
                material);
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                scaledRect.x + border,
                scaledRect.y - border,
                scaledRect.w - (float)(2.0 * border),
                border * 2.0,
                rect->horzAlign,
                rect->vertAlign,
                0.5,
                0.0,
                0.5,
                0.5,
                color,
                material);
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                scaledRect.x + border,
                (float)(scaledRect.y + scaledRect.h) - border,
                scaledRect.w - (float)(2.0 * border),
                border * 2.0,
                rect->horzAlign,
                rect->vertAlign,
                0.5,
                0.5,
                0.5,
                1.0,
                color,
                material);
        }
    }
}

void __cdecl CG_CompassUpYawVector(const cg_s *cgameGlob, float *result)
{
    if ( compassRotation->current.enabled )
        YawVectors2D(cgameGlob->refdefViewAngles[1], result, 0);
    else
        *(double *)result = *(double *)cgameGlob->compassNorth;
}

#if 0
void    CG_CompassDrawTickertape(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Material *material,
                const float *color,
                Font_s *textFont,
                float textScale,
                int textStyle,
                bool drawObjectives)
{
    double v11; // xmm0_8
    long double v12; // [esp+30h] [ebp-BCh]
    long double v13; // [esp+38h] [ebp-B4h]
    float v14; // [esp+40h] [ebp-ACh]
    Material *v15; // [esp+44h] [ebp-A8h]
    float iconX; // [esp+4Ch] [ebp-A0h]
    float iconMaterial; // [esp+50h] [ebp-9Ch]
    float distWithRange; // [esp+54h] [ebp-98h]
    float textAlpha; // [esp+58h] [ebp-94h]
    float goalYawa; // [esp+64h] [ebp-88h]
    float goalYaw; // [esp+64h] [ebp-88h]
    float v22; // [esp+68h] [ebp-84h] BYREF
    float v23; // [esp+6Ch] [ebp-80h]
    int dist; // [esp+70h] [ebp-7Ch]
    float posDelta[2]; // [esp+74h] [ebp-78h]
    unsigned int *v26; // [esp+7Ch] [ebp-70h]
    const float *goalOrig; // [esp+80h] [ebp-6Ch]
    centity_s *cent; // [esp+84h] [ebp-68h]
    const objective_t *objective; // [esp+88h] [ebp-64h]
    int objIdx; // [esp+8Ch] [ebp-60h]
    float nearestDistHeightDelta; // [esp+90h] [ebp-5Ch]
    float nearestDist; // [esp+94h] [ebp-58h]
    float tapeAngleCenter; // [esp+98h] [ebp-54h]
    float tapeAngleStretch; // [esp+9Ch] [ebp-50h] BYREF
    float iconDrawY; // [esp+A0h] [ebp-4Ch] BYREF
    float iconY; // [esp+A4h] [ebp-48h] BYREF
    float iconW; // [esp+A8h] [ebp-44h] BYREF
    float iconH; // [esp+ACh] [ebp-40h] BYREF
    float x; // [esp+B0h] [ebp-3Ch] BYREF
    float y; // [esp+B4h] [ebp-38h]
    float w; // [esp+B8h] [ebp-34h]
    float h; // [esp+BCh] [ebp-30h]
    float tapeRight; // [esp+C0h] [ebp-2Ch]
    float tapeLeft; // [esp+C4h] [ebp-28h]
    float tapeCenter; // [esp+C8h] [ebp-24h] BYREF
    float tapeRotation; // [esp+CCh] [ebp-20h]
    int v47; // [esp+D0h] [ebp-1Ch]
    float colorMod[4]; // [esp+D4h] [ebp-18h]
    float defAlpha; // [esp+E4h] [ebp-8h]
    float retaddr; // [esp+ECh] [ebp+0h]

    colorMod[3] = a1;
    defAlpha = retaddr;
    if ( compassType
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                    2083,
                    0,
                    "%s",
                    "compassType == COMPASS_TYPE_PARTIAL") )
    {
        __debugbreak();
    }
    LODWORD(colorMod[2]) = CG_GetLocalClientGlobals(localClientNum);
    colorMod[1] = CG_FadeCompass(localClientNum, *(unsigned int *)(LODWORD(colorMod[2]) + 388548), compassType);
    if ( colorMod[1] != 0.0 )
    {
        if ( colorMod[1] > color[3] )
            colorMod[1] = color[3];
        tapeCenter = *color;
        tapeRotation = color[1];
        v47 = *((unsigned int *)color + 2);
        colorMod[0] = colorMod[1];
        tapeLeft = compassPartialType->current.value;
        if ( !LODWORD(tapeLeft) )
        {
            LODWORD(tapeRight) = COERCE_UNSIGNED_INT(*(float *)(LODWORD(colorMod[2]) + 373716) - *(float *)(LODWORD(colorMod[2]) + 388504)) ^ _mask__NegFloat_;
            h = tapeRight / 360.0;
            w = (float)(tapeRight / 360.0) - (float)(compassTickertapeStretch->current.value * 0.5);
            y = (float)(compassTickertapeStretch->current.value * 0.5) + (float)(tapeRight / 360.0);
            CG_CompassCalcDimensions(
                compassType,
                (const cg_s *)LODWORD(colorMod[2]),
                parentRect,
                rect,
                &iconY,
                &iconW,
                &iconH,
                &x);
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                iconY,
                iconW,
                iconH,
                x,
                rect->horzAlign,
                rect->vertAlign,
                w,
                0.0,
                y,
                1.0,
                &tapeCenter,
                material);
            if ( drawObjectives )
            {
                CalcCompassPointerSize(compassType, &tapeAngleStretch, &iconDrawY);
                tapeAngleCenter = (float)(x * 0.5) + iconW;
                nearestDist = tapeAngleCenter - (float)(iconDrawY * 0.5);
                nearestDistHeightDelta = (float)(compassTickertapeStretch->current.value * 360.0) * 0.5;
                *(float *)&objIdx = AngleNormalize360(COERCE_CONST_FLOAT(LODWORD(tapeRight) ^ _mask__NegFloat_));
                *(float *)&objective = FLT_MAX;
                cent = *(centity_s **)&FLOAT_0_0;
                for ( goalOrig = 0; (int)goalOrig < 32; goalOrig = (const float *)((char *)goalOrig + 1) )
                {
                    v26 = (unsigned int *)(*(unsigned int *)(LODWORD(colorMod[2]) + 44) + 48 * (unsigned int)goalOrig + 1416);
                    if ( *v26 == 4 || *v26 == 1 )
                    {
                        if ( v26[6] == 1023 )
                        {
                            LODWORD(posDelta[0]) = v26 + 1;
                        }
                        else
                        {
                            LODWORD(posDelta[1]) = CG_GetEntity(localClientNum, v26[6]);
                            LODWORD(posDelta[0]) = LODWORD(posDelta[1]) + 48;
                        }
                        colorMod[0] = colorMod[1];
                        if ( *(float *)LODWORD(posDelta[0]) != 0.0
                            || *(float *)(LODWORD(posDelta[0]) + 4) != 0.0
                            || *(float *)(LODWORD(posDelta[0]) + 8) != 0.0 )
                        {
                            dist = LODWORD(colorMod[2]) + 274140;
                            v22 = *(float *)LODWORD(posDelta[0]) - *(float *)(LODWORD(colorMod[2]) + 274140);
                            v23 = *(float *)(LODWORD(posDelta[0]) + 4) - *(float *)(LODWORD(colorMod[2]) + 274144);
                            goalYawa = Vec2Length(&v22);
                            goalYaw = goalYawa * 0.0254;
                            if ( *(float *)&objective > goalYaw )
                            {
                                *(float *)&objective = goalYaw;
                                *(float *)&cent = *(float *)(LODWORD(colorMod[2]) + 274148) - *(float *)(LODWORD(posDelta[0]) + 8);
                            }
                            v11 = v23;
                            __libm_sse2_atan2(v12, v13);
                            *(float *)&v11 = v11;
                            textAlpha = AngleNormalize360(
                                                        (float)((float)(*(float *)&v11 * 180.0) / 3.1415927)
                                                    - *(float *)(LODWORD(colorMod[2]) + 388504));
                            distWithRange = -AngleDelta(textAlpha, *(const float *)&objIdx);
                            if ( COERCE_FLOAT(LODWORD(nearestDistHeightDelta) ^ _mask__NegFloat_) <= distWithRange )
                            {
                                if ( distWithRange <= nearestDistHeightDelta )
                                    iconMaterial = (float)(distWithRange + nearestDistHeightDelta) / (float)(nearestDistHeightDelta * 2.0);
                                else
                                    iconMaterial = 1.0f;
                            }
                            else
                            {
                                iconMaterial = 0.0f;
                            }
                            iconX = colorMod[1];
                            v15 = CG_ObjectiveIcon(localClientNum, v26[8], 0);
                            v14 = (float)(iconH * iconMaterial) + iconY;
                            *((float *)&v13 + 1) = v14 - (float)(tapeAngleStretch * 0.5);
                            CL_DrawStretchPic(
                                &scrPlaceView[localClientNum],
                                *((float *)&v13 + 1),
                                nearestDist,
                                tapeAngleStretch,
                                iconDrawY,
                                rect->horzAlign,
                                rect->vertAlign,
                                0.0,
                                0.0,
                                1.0,
                                1.0,
                                &tapeCenter,
                                v15);
                            colorMod[0] = iconX;
                            DrawIconDistanceText(
                                localClientNum,
                                goalYaw,
                                v14,
                                tapeAngleCenter,
                                iconDrawY,
                                rect,
                                &tapeCenter,
                                textFont,
                                textScale,
                                textStyle);
                        }
                    }
                }
            }
        }
    }
}
#endif

// aislop
void CG_CompassDrawTickertape(
    int localClientNum,
    CompassType compassType,
    const rectDef_s *parentRect,
    const rectDef_s *rect,
    Material *material,
    const float *color,
    Font_s *textFont,
    float textScale,
    int textStyle,
    bool drawObjectives)
{
    if (compassType != COMPASS_TYPE_PARTIAL)
    {
        Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
            2083,
            0,
            "%s",
            "compassType == COMPASS_TYPE_PARTIAL");
        return;
    }

    const cg_s *cgame = CG_GetLocalClientGlobals(localClientNum);

    float fade =
        CG_FadeCompass(localClientNum, cgame->compassFadeTime, compassType);

    if (fade <= 0.0f)
        return;

    if (fade > color[3])
        fade = color[3];

    float colorMod[4] = {
        color[0],
        color[1],
        color[2],
        fade
    };

    if (compassPartialType->current.integer != 0)
        return;

    /* yaw setup */
    float yawDelta =
        AngleNormalize360(
            cgame->refdefViewAngles[1] - cgame->compassNorthYaw);

    float tapeCenter = yawDelta / 360.0f;
    float stretch = compassTickertapeStretch->current.value;

    float tapeLeft = tapeCenter - stretch * 0.5f;
    float tapeRight = tapeCenter + stretch * 0.5f;

    float x, y, w, h;
    CG_CompassCalcDimensions(
        compassType,
        cgame,
        parentRect,
        rect,
        &x,
        &y,
        &w,
        &h);

    CL_DrawStretchPic(
        &scrPlaceView[localClientNum],
        x,
        y,
        w,
        h,
        rect->horzAlign,
        rect->vertAlign,
        tapeLeft,
        0.0f,
        tapeRight,
        1.0f,
        colorMod,
        material);

    if (!drawObjectives)
        return;

    float iconW, iconH;
    CalcCompassPointerSize(compassType, &iconW, &iconH);

    float iconY = y + h * 0.5f;
    float iconDrawY = iconY - iconH * 0.5f;

    float angleRange = stretch * 360.0f * 0.5f;
    float angleCenter = AngleNormalize360(-yawDelta);

    for (int i = 0; i < 32; ++i)
    {
        const objective_t *obj =
            &cgame->nextSnap->ps.objective[i];

        if (obj->state != OBJST_CURRENT &&
            obj->state != OBJST_ACTIVE)
            continue;

        float target[3];

        if (obj->entNum == 1023)
        {
            target[0] = obj->origin[0];
            target[1] = obj->origin[1];
            target[2] = obj->origin[2];
        }
        else
        {
            centity_s *cent =
                CG_GetEntity(localClientNum, obj->entNum);
            target[0] = cent->pose.origin[0];
            target[1] = cent->pose.origin[1];
            target[2] = cent->pose.origin[2];
        }

        if (target[0] == 0.0f &&
            target[1] == 0.0f &&
            target[2] == 0.0f)
            continue;

        float delta[2];
        delta[0] = target[0] - cgame->refdef.vieworg[0];
        delta[1] = target[1] - cgame->refdef.vieworg[1];

        float distMeters =
            Vec2Length(delta) * 0.0254f;

        float yaw =
            AngleNormalize360(
                atan2f(delta[1], delta[0]) * (180.0f / M_PI)
                - cgame->compassNorthYaw);

        float angleDelta =
            -AngleDelta(yaw, angleCenter);

        float percent;
        if (angleDelta < -angleRange)
            percent = 0.0f;
        else if (angleDelta > angleRange)
            percent = 1.0f;
        else
            percent = (angleDelta + angleRange) / (angleRange * 2.0f);

        Material *icon =
            CG_ObjectiveIcon(localClientNum, obj->icon, 0);

        float iconX = x + w * percent;

        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            iconX - iconW * 0.5f,
            iconDrawY,
            iconW,
            iconH,
            rect->horzAlign,
            rect->vertAlign,
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            colorMod,
            icon);

        DrawIconDistanceText(
            localClientNum,
            distMeters,
            iconX,
            iconY,
            iconH,
            rect,
            colorMod,
            textFont,
            textScale,
            textStyle);
    }
}


void __cdecl CalcCompassPointerSize(CompassType compassType, float *w, float *h)
{
    if ( !w && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 1825, 0, "%s", "w") )
        __debugbreak();
    if ( !h && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 1826, 0, "%s", "h") )
        __debugbreak();
    if ( compassType )
    {
        if ( zombiemode->current.enabled )
        {
            *w = compassObjectiveIconWidthZombie->current.value;
            *h = compassObjectiveIconHeightZombie->current.value;
        }
        else
        {
            *w = compassObjectiveIconWidth->current.value;
            *h = compassObjectiveIconHeight->current.value;
        }
    }
    else
    {
        *w = compassObjectiveWidth->current.value * compassSize->current.value;
        *h = compassObjectiveHeight->current.value * compassSize->current.value;
    }
}

const float colorObjectiveText[4] = { 0.89999998, 1.0, 0.1, 1.0 };

void __cdecl DrawIconDistanceText(
                int localClientNum,
                float distance,
                float iconX,
                float iconY,
                float iconH,
                const rectDef_s *rect,
                const float *color,
                Font_s *textFont,
                float textScale,
                int textStyle)
{
    double v10; // st7
    char str[68]; // [esp+1Ch] [ebp-68h] BYREF
    float textWidth; // [esp+6Ch] [ebp-18h]
    float textHeight; // [esp+70h] [ebp-14h]
    float colorMod[4]; // [esp+74h] [ebp-10h] BYREF

    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 1999, 0, "%s", "rect") )
        __debugbreak();
    colorMod[0] = colorObjectiveText[0];
    colorMod[1] = colorObjectiveText[1];
    colorMod[2] = colorObjectiveText[2];
    colorMod[3] = color[3];
    if ( compassObjectiveDetailDist->current.value <= distance )
    {
        Com_sprintf(str, 0x40u, "%im", (int)distance);
    }
    else
    {
        v10 = CutFloat(distance);
        Com_sprintf(str, 0x40u, "%.1fm", v10);
    }
    textWidth = (float)UI_TextWidth(str, 20, textFont, textScale);
    textHeight = (float)UI_TextHeight(textFont, textScale);
    UI_DrawTextNoSnap(
        &scrPlaceView[localClientNum],
        str,
        0x7FFFFFFF,
        textFont,
        iconX - (float)(textWidth * 0.5),
        (float)(iconY + iconH) + 2.0,
        rect->horzAlign,
        rect->vertAlign,
        textScale,
        colorMod,
        textStyle);
}

double __cdecl CutFloat(float original)
{
    return (double)(int)(float)(original * 10.0) * 0.1;
}

void __cdecl CG_SetGridTable()
{
    const char *gridPoint; // [esp+14h] [ebp-1Ch]
    int tableRow; // [esp+18h] [ebp-18h]
    int row; // [esp+1Ch] [ebp-14h]
    const char *mapname; // [esp+20h] [ebp-10h]
    const StringTable *gridStringTable; // [esp+24h] [ebp-Ch] BYREF
    int tableCol; // [esp+28h] [ebp-8h]
    int col; // [esp+2Ch] [ebp-4h]

    if ( !G_ExitAfterToolComplete() )
    {
        memset(gridPointStatus, 0, sizeof(gridPointStatus));
        StringTable_GetAsset("mp/gridPointsTable.csv", (XAssetHeader *)&gridStringTable);
        if ( !gridStringTable
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 2260, 0, "%s", "gridStringTable") )
        {
            __debugbreak();
        }
        mapname = Dvar_GetString("ui_mapname");
        tableRow = StringTable_LookupRowNumForValue(gridStringTable, 0, mapname);
        if ( tableRow >= 0 )
        {
            tableCol = 1;
            col = 0;
            for ( gridPoint = StringTable_GetColumnValueForRow(gridStringTable, tableRow, 1);
                        strcmp(gridPoint, "");
                        gridPoint = StringTable_GetColumnValueForRow(gridStringTable, tableRow, tableCol) )
            {
                row = *gridPoint - 65;
                col = gridPoint[1] - 49;
                if ( row >= 9
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                                2281,
                                0,
                                "%s",
                                "row < MAX_GRID_ROWS") )
                {
                    __debugbreak();
                }
                if ( col >= 9
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                                2282,
                                0,
                                "%s",
                                "col < MAX_GRID_COLS") )
                {
                    __debugbreak();
                }
                gridPointStatus[row][col] = 1;
                ++tableCol;
            }
        }
    }
}

int count;
void __cdecl CG_ScaleCompass()
{
    unsigned int value; // eax
    int deltaDistance; // [esp+4h] [ebp-8h]
    int deltaTime; // [esp+8h] [ebp-4h]

    if ( compassScaleDuration->current.integer > 0 )
    {
        if ( compassScaleReset->current.enabled )
        {
            Dvar_SetInt((dvar_s *)compassScaleDiff, 2500 - (int)compassMaxRange->current.value);
            Dvar_SetBool((dvar_s *)compassScaleReset, 0);
        }
        deltaTime = compassScaleDuration->current.integer / 30;
        deltaDistance = compassScaleDiff->current.integer / 30;
        if ( compassScaleTimer->current.integer > 0 )
        {
            if ( compassScaleTimer->current.integer <= (signed int)Sys_Milliseconds() )
            {
                Dvar_SetFloat((dvar_s *)compassMaxRange, (float)((float)deltaDistance * 1.0) + compassMaxRange->current.value);
                Dvar_SetInt((dvar_s *)compassScaleTimer, deltaTime + compassScaleTimer->current.integer);
                if ( ++count >= 30 )
                {
                    Dvar_SetInt((dvar_s *)compassScaleDuration, 0);
                    Dvar_SetInt((dvar_s *)compassScaleTimer, 0);
                    Dvar_SetInt((dvar_s *)compassScaleDiff, 0);
                    count = 0;
                }
            }
        }
        else
        {
            value = Sys_Milliseconds();
            Dvar_SetInt((dvar_s *)compassScaleTimer, value);
        }
    }
}

void __cdecl CG_SetGridOffsets(
                const float *east,
                const float *south,
                float *eastOffsetLetter,
                float *southOffsetLetter)
{
    switch ( compassGridAlign->current.integer )
    {
        case 0:
            *eastOffsetLetter = *east / 4.0;
            eastOffsetLetter[1] = east[1] / 4.0;
            *southOffsetLetter = *south / 4.0;
            southOffsetLetter[1] = south[1] / 4.0;
            break;
        case 1:
            *eastOffsetLetter = *east / 2.0;
            eastOffsetLetter[1] = east[1] / 2.0;
            *southOffsetLetter = *south / 4.0;
            southOffsetLetter[1] = south[1] / 4.0;
            break;
        case 2:
            *eastOffsetLetter = *east * 0.75;
            eastOffsetLetter[1] = east[1] * 0.75;
            *southOffsetLetter = *south / 4.0;
            southOffsetLetter[1] = south[1] / 4.0;
            break;
        case 3:
            *eastOffsetLetter = *east / 4.0;
            eastOffsetLetter[1] = east[1] / 4.0;
            *southOffsetLetter = *south / 2.0;
            southOffsetLetter[1] = south[1] / 2.0;
            break;
        case 4:
            *eastOffsetLetter = *east / 2.0;
            eastOffsetLetter[1] = east[1] / 2.0;
            *southOffsetLetter = *south / 2.0;
            southOffsetLetter[1] = south[1] / 2.0;
            break;
        case 5:
            *eastOffsetLetter = *east * 0.75;
            eastOffsetLetter[1] = east[1] * 0.75;
            *southOffsetLetter = *south / 2.0;
            southOffsetLetter[1] = south[1] / 2.0;
            break;
        case 6:
            *eastOffsetLetter = *east / 4.0;
            eastOffsetLetter[1] = east[1] / 4.0;
            *southOffsetLetter = *south * 0.75;
            southOffsetLetter[1] = south[1] * 0.75;
            break;
        case 7:
            *eastOffsetLetter = *east / 2.0;
            eastOffsetLetter[1] = east[1] / 2.0;
            *southOffsetLetter = *south * 0.75;
            southOffsetLetter[1] = south[1] * 0.75;
            break;
        case 8:
            *eastOffsetLetter = *east * 0.75;
            eastOffsetLetter[1] = east[1] * 0.75;
            *southOffsetLetter = *south * 0.75;
            southOffsetLetter[1] = south[1] * 0.75;
            break;
        default:
            Com_Printf(16, "compassGridAlign set to unknown value. Defaulting to center.\n");
            *eastOffsetLetter = *east / 2.0;
            eastOffsetLetter[1] = east[1] / 2.0;
            *southOffsetLetter = *south / 2.0;
            southOffsetLetter[1] = south[1] / 2.0;
            break;
    }
}

void __cdecl CG_CompassDrawGridPoints(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                const float *color,
                Font_s *textFont,
                float textScale,
                int textStyle)
{
    float v8; // xmm0_4
    float v9; // xmm0_4
    float v10; // xmm0_4
    float yawVector[2]; // [esp+2Ch] [ebp-F0h] BYREF
    char numeral; // [esp+37h] [ebp-E5h]
    int number; // [esp+38h] [ebp-E4h]
    float gridStartNumber[2]; // [esp+3Ch] [ebp-E0h]
    float gridPosNumber[2]; // [esp+44h] [ebp-D8h]
    float eastOffsetNumber[2]; // [esp+4Ch] [ebp-D0h]
    float southOffsetNumber[2]; // [esp+54h] [ebp-C8h]
    float xy[2]; // [esp+5Ch] [ebp-C0h] BYREF
    float gridWidth; // [esp+64h] [ebp-B8h]
    float gridPosLetter[2]; // [esp+68h] [ebp-B4h] BYREF
    cg_s *cgameGlob; // [esp+70h] [ebp-ACh]
    float unplayableSpaceColor[4]; // [esp+74h] [ebp-A8h] BYREF
    rectDef_s scissorRect; // [esp+84h] [ebp-98h] BYREF
    float compassFadeOutAlpha; // [esp+9Ch] [ebp-80h]
    char letter; // [esp+A3h] [ebp-79h]
    float xyNoClip[2]; // [esp+A4h] [ebp-78h] BYREF
    char gridName[4]; // [esp+ACh] [ebp-70h] BYREF
    int row; // [esp+B0h] [ebp-6Ch]
    float centerY; // [esp+B4h] [ebp-68h]
    rectDef_s scaledRect; // [esp+B8h] [ebp-64h] BYREF
    float east[2]; // [esp+D0h] [ebp-4Ch] BYREF
    float south[2]; // [esp+D8h] [ebp-44h] BYREF
    float eastOffsetLetter[2]; // [esp+E0h] [ebp-3Ch] BYREF
    float centerX; // [esp+E8h] [ebp-34h]
    float fadedColor[4]; // [esp+ECh] [ebp-30h] BYREF
    float gridStartLetter[2]; // [esp+FCh] [ebp-20h]
    float h; // [esp+104h] [ebp-18h]
    float southOffsetLetter[2]; // [esp+108h] [ebp-14h] BYREF
    int col; // [esp+110h] [ebp-Ch]
    float gridHeight; // [esp+114h] [ebp-8h]
    float w; // [esp+118h] [ebp-4h]

    w = 0.0f;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                    2440,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    compassFadeOutAlpha = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
    compassFadeOutAlpha = CG_FadeCompassIcons(localClientNum, 0) * compassFadeOutAlpha;
    if ( compassFadeOutAlpha != 0.0 )
    {
        CG_CompassCalcDimensions(
            compassType,
            cgameGlob,
            parentRect,
            rect,
            &scaledRect.x,
            &scaledRect.y,
            &scaledRect.w,
            &scaledRect.h);
        centerX = (float)(scaledRect.w * 0.5) + scaledRect.x;
        centerY = (float)(scaledRect.h * 0.5) + scaledRect.y;
        fadedColor[0] = *color;
        fadedColor[1] = color[1];
        fadedColor[2] = color[2];
        fadedColor[3] = color[3];
        fadedColor[3] = fadedColor[3] * compassFadeOutAlpha;
        unplayableSpaceColor[3] = fadedColor[3];
        v8 = fadedColor[0] * 0.5;
        unplayableSpaceColor[0] = v8;
        v9 = fadedColor[1] * 0.5;
        unplayableSpaceColor[1] = v9;
        v10 = fadedColor[2] * 0.5;
        unplayableSpaceColor[2] = v10;
        CG_CompassUpYawVector(cgameGlob, yawVector);
        gridWidth = cgameGlob->compassMapWorldSize[0] / (float)compassGridCols->current.integer;
        gridHeight = cgameGlob->compassMapWorldSize[1] / (float)compassGridRows->current.integer;
        south[0] = (-(cgameGlob->compassNorth[0])) * gridHeight;
        south[1] = (-(cgameGlob->compassNorth[1])) * gridHeight;
        east[0] = cgameGlob->compassNorth[1] * gridWidth;
        east[1] = (-(cgameGlob->compassNorth[0])) * gridWidth;
        CG_SetGridOffsets(east, south, eastOffsetLetter, southOffsetLetter);
        gridStartLetter[0] = cgameGlob->compassMapUpperLeft[0] + southOffsetLetter[0];
        gridStartLetter[1] = cgameGlob->compassMapUpperLeft[1] + southOffsetLetter[1];
        gridStartLetter[0] = gridStartLetter[0] + eastOffsetLetter[0];
        gridStartLetter[1] = gridStartLetter[1] + eastOffsetLetter[1];
        gridStartNumber[0] = cgameGlob->compassMapUpperLeft[0] + southOffsetNumber[0];
        gridStartNumber[1] = cgameGlob->compassMapUpperLeft[1] + southOffsetNumber[1];
        gridStartNumber[0] = gridStartNumber[0] + eastOffsetNumber[0];
        gridStartNumber[1] = gridStartNumber[1] + eastOffsetNumber[1];
        if ( compassType == COMPASS_TYPE_PARTIAL )
        {
            textScale = textScale * compassSize->current.value;
            scissorRect = scaledRect;
            scissorRect.horzAlign = rect->horzAlign;
            scissorRect.vertAlign = rect->vertAlign;
            ScrPlace_ApplyRect(
                &scrPlaceView[localClientNum],
                &scissorRect.x,
                &scissorRect.y,
                &scissorRect.w,
                &scissorRect.h,
                scissorRect.horzAlign,
                scissorRect.vertAlign);
            R_AddCmdSetScissorValues(1, (int)scissorRect.x, (int)scissorRect.y, (int)scissorRect.w, (int)scissorRect.h);
        }
        row = 0;
        letter = 65;
        number = 1;
        while ( row < compassGridRows->current.integer )
        {
            gridPosLetter[0] = (float)((float)((float)row * 1.0) * south[0]) + gridStartLetter[0];
            gridPosLetter[1] = (float)((float)((float)row * 1.0) * south[1]) + gridStartLetter[1];
            gridPosNumber[0] = (float)((float)((float)row * 1.0) * south[0]) + gridStartNumber[0];
            gridPosNumber[1] = (float)((float)((float)row * 1.0) * south[1]) + gridStartNumber[1];
            col = 0;
            numeral = 49;
            while ( col < compassGridCols->current.integer )
            {
                gridName[0] = letter;
                gridName[1] = numeral;
                gridName[2] = 0;
                w = (float)UI_TextWidth(gridName, 3, textFont, textScale) * 1.0;
                ++numeral;
                h = (float)UI_TextHeight(textFont, textScale) * 1.0;
                CG_WorldPosToCompass(
                    compassType,
                    cgameGlob,
                    &scaledRect,
                    yawVector,
                    cgameGlob->refdef.vieworg,
                    gridPosLetter,
                    xyNoClip,
                    xy);
                if ( !CG_TextClippedToCompass(
                                compassType,
                                cgameGlob,
                                &scaledRect,
                                yawVector,
                                cgameGlob->refdef.vieworg,
                                gridPosLetter,
                                w,
                                h) )
                {
                    xyNoClip[0] = (float)(centerX - (float)(w * 0.5)) + xyNoClip[0];
                    xyNoClip[1] = (float)((float)(h * 0.5) + centerY) + xyNoClip[1];
                    if ( gridPointStatus[row][col] )
                        UI_DrawTextNoSnap(
                            &scrPlaceView[localClientNum],
                            gridName,
                            3,
                            textFont,
                            xyNoClip[0],
                            xyNoClip[1],
                            rect->horzAlign,
                            rect->vertAlign,
                            textScale,
                            unplayableSpaceColor,
                            textStyle);
                    else
                        UI_DrawTextNoSnap(
                            &scrPlaceView[localClientNum],
                            gridName,
                            3,
                            textFont,
                            xyNoClip[0],
                            xyNoClip[1],
                            rect->horzAlign,
                            rect->vertAlign,
                            textScale,
                            fadedColor,
                            textStyle);
                }
                ++col;
                gridPosLetter[0] = gridPosLetter[0] + east[0];
                gridPosLetter[1] = gridPosLetter[1] + east[1];
                gridPosNumber[0] = gridPosNumber[0] + east[0];
                gridPosNumber[1] = gridPosNumber[1] + east[1];
            }
            ++letter;
            ++row;
        }
        if ( compassType == COMPASS_TYPE_PARTIAL )
            R_AddCmdSetScissorValues(0, 0, 0, 0, 0);
    }
}

void __cdecl CG_GetGridFromPos(int localClientNum, float *pos, char *gridName)
{
    float compassUpperLeft[2]; // [esp+0h] [ebp-80h] BYREF
    char number; // [esp+Ah] [ebp-76h]
    bool clipped; // [esp+Bh] [ebp-75h]
    float gridWidth; // [esp+Ch] [ebp-74h]
    cg_s *cgameGlob; // [esp+10h] [ebp-70h]
    rectDef_s parentRect; // [esp+14h] [ebp-6Ch] BYREF
    int southOffset; // [esp+2Ch] [ebp-54h]
    char letter; // [esp+33h] [ebp-4Dh]
    int eastOffset; // [esp+34h] [ebp-4Ch]
    float compassPos[2]; // [esp+38h] [ebp-48h] BYREF
    int i; // [esp+40h] [ebp-40h]
    rectDef_s scaledRect; // [esp+44h] [ebp-3Ch] BYREF
    rectDef_s rect; // [esp+5Ch] [ebp-24h] BYREF
    float gridHeight; // [esp+74h] [ebp-Ch]
    float center[2]; // [esp+78h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    *(_QWORD *)&parentRect.w = *(_QWORD *)cgameGlob->compassMapWorldSize;
    parentRect.x = 0.0f;
    parentRect.y = 0.0f;
    parentRect.horzAlign = 1;
    parentRect.vertAlign = 1;
    rect.x = 0.0f;
    rect.y = 0.0f;
    *(_QWORD *)&rect.w = *(_QWORD *)&parentRect.w;
    rect.horzAlign = 1;
    rect.vertAlign = 1;
    CG_CompassCalcDimensions(
        COMPASS_TYPE_FULL,
        cgameGlob,
        &parentRect,
        &rect,
        &scaledRect.x,
        &scaledRect.y,
        &scaledRect.w,
        &scaledRect.h);
    center[0] = (float)(scaledRect.w * 0.5) + scaledRect.x;
    center[1] = (float)(scaledRect.h * 0.5) + scaledRect.y;
    gridWidth = scaledRect.w / (float)compassGridCols->current.integer;
    gridHeight = scaledRect.h / (float)compassGridRows->current.integer;
    clipped = CG_WorldPosToCompass(
                            COMPASS_TYPE_FULL,
                            cgameGlob,
                            &scaledRect,
                            cgameGlob->compassNorth,
                            cgameGlob->refdef.vieworg,
                            pos,
                            0,
                            compassPos);
    CG_WorldPosToCompass(
        COMPASS_TYPE_FULL,
        cgameGlob,
        &scaledRect,
        cgameGlob->compassNorth,
        cgameGlob->refdef.vieworg,
        cgameGlob->compassMapUpperLeft,
        0,
        compassUpperLeft);
    compassUpperLeft[0] = compassUpperLeft[0] + center[0];
    compassUpperLeft[1] = compassUpperLeft[1] + center[1];
    compassPos[0] = compassPos[0] + center[0];
    compassPos[1] = compassPos[1] + center[1];
    letter = 33;
    number = 64;
    if ( !clipped )
    {
        letter = 65;
        number = 49;
        eastOffset = (int)(float)((float)(compassUpperLeft[0] + compassPos[0]) / gridWidth);
        for ( i = 0; i < eastOffset; ++i )
            ++number;
        southOffset = (int)(float)((float)(compassUpperLeft[1] + compassPos[1]) / gridHeight);
        for ( i = 0; i < southOffset; ++i )
            ++letter;
    }
    Com_sprintf(gridName, 4u, "%c%c", letter, number);
}

float __cdecl CG_GetHudAlphaCompass(int localClientNum)
{
    cg_s *LocalClientGlobals; // eax

    LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
    return CG_FadeCompass(localClientNum, LocalClientGlobals->compassFadeTime, COMPASS_TYPE_PARTIAL);
}

void __cdecl CalcCompassFriendlySize(CompassType compassType, float *w, float *h)
{
    if ( !w && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 2732, 0, "%s", "w") )
        __debugbreak();
    if ( !h && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp", 2733, 0, "%s", "h") )
        __debugbreak();
    if ( compassType )
    {
        *w = cg_hudMapFriendlyWidth->current.value;
        *h = cg_hudMapFriendlyHeight->current.value;
    }
    else
    {
        *w = compassFriendlyWidth->current.value * compassSize->current.value;
        *h = compassFriendlyHeight->current.value * compassSize->current.value;
    }
}

void __cdecl CG_CompassDrawPlayerPointers_MiniMap(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Material *material,
                const float *color)
{
    float tempcolor; // [esp+3Ch] [ebp-ACh]
    float lerp; // [esp+40h] [ebp-A8h]
    char shouldRotateWithView; // [esp+4Bh] [ebp-9Dh]
    float iconAngle; // [esp+4Ch] [ebp-9Ch]
    Material *icon; // [esp+50h] [ebp-98h]
    float yawVector[2]; // [esp+54h] [ebp-94h] BYREF
    bool clipped; // [esp+5Fh] [ebp-89h]
    const objective_t *obj; // [esp+60h] [ebp-88h]
    float delta[3]; // [esp+64h] [ebp-84h] BYREF
    float xy[2]; // [esp+70h] [ebp-78h] BYREF
    float origin[3]; // [esp+78h] [ebp-70h] BYREF
    cg_s *cgameGlob; // [esp+84h] [ebp-64h]
    centity_s *cent; // [esp+88h] [ebp-60h]
    float objDist; // [esp+8Ch] [ebp-5Ch]
    float xyClipped[2]; // [esp+90h] [ebp-58h] BYREF
    float centerY; // [esp+98h] [ebp-50h]
    rectDef_s scaledRect; // [esp+9Ch] [ebp-4Ch] BYREF
    float x; // [esp+B4h] [ebp-34h]
    float y; // [esp+B8h] [ebp-30h]
    const playerState_s *ps; // [esp+BCh] [ebp-2Ch]
    int objNum; // [esp+C0h] [ebp-28h]
    float clipfade; // [esp+C4h] [ebp-24h]
    float centerX; // [esp+C8h] [ebp-20h]
    float h; // [esp+CCh] [ebp-1Ch] BYREF
    float fadeAlpha; // [esp+D0h] [ebp-18h]
    float w; // [esp+D4h] [ebp-14h] BYREF
    float newColor[4]; // [esp+D8h] [ebp-10h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->nextSnap )
    {
        fadeAlpha = CG_FadeCompass(localClientNum, cgameGlob->compassFadeTime, compassType);
        fadeAlpha = CG_FadeCompassIcons(localClientNum, 0) * fadeAlpha;
        if ( fadeAlpha != 0.0 )
        {
            CG_CompassCalcDimensions(
                compassType,
                cgameGlob,
                parentRect,
                rect,
                &scaledRect.x,
                &scaledRect.y,
                &scaledRect.w,
                &scaledRect.h);
            centerX = (float)(scaledRect.w * 0.5) + scaledRect.x;
            centerY = (float)(scaledRect.h * 0.5) + scaledRect.y;
            ps = &cgameGlob->nextSnap->ps;
            if ( ps->clientNum >= 0x20u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_compass.cpp",
                            2806,
                            0,
                            "ps->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                            ps->clientNum,
                            32) )
            {
                __debugbreak();
            }
            if ( cgameGlob->bgs.clientinfo[ps->clientNum].infoValid )
            {
                CG_CompassUpYawVector(cgameGlob, yawVector);
                for ( objNum = 0; objNum < 32; ++objNum )
                {
                    obj = &ps->objective[objNum];
                    if ( obj->state == OBJST_CURRENT || obj->state == OBJST_ACTIVE )
                    {
                        if ( obj->entNum == 1023 )
                        {
                            origin[0] = obj->origin[0];
                            origin[1] = obj->origin[1];
                            origin[2] = obj->origin[2];
                        }
                        else
                        {
                            cent = CG_GetEntity(localClientNum, obj->entNum);
                            origin[0] = cent->pose.origin[0];
                            origin[1] = cent->pose.origin[1];
                            origin[2] = cent->pose.origin[2];
                        }
                        delta[0] = origin[0] - cgameGlob->refdef.vieworg[0];
                        delta[1] = origin[1] - cgameGlob->refdef.vieworg[1];
                        delta[2] = origin[2] - cgameGlob->refdef.vieworg[2];
                        objDist = Vec2Length(delta);
                        newColor[0] = *color;
                        newColor[1] = color[1];
                        newColor[2] = color[2];
                        newColor[3] = color[3];
                        if ( obj->color )
                            Byte4UnpackRgba((const unsigned __int8 *)&obj->color, newColor);
                        if ( compassMaxRange->current.value < objDist )
                        {
                            if ( objDist < compassObjectiveMaxRange->current.value )
                            {
                                lerp = 0.0f;
                                if ( (float)(compassObjectiveMaxRange->current.value - compassMaxRange->current.value) != 0.0 )
                                    lerp = (float)(objDist - compassMaxRange->current.value)
                                             / (float)(compassObjectiveMaxRange->current.value - compassMaxRange->current.value);
                                newColor[3] = 1.0 - (float)((float)(1.0 - compassObjectiveMinAlpha->current.value) * lerp);
                            }
                            else
                            {
                                newColor[3] = compassObjectiveMinAlpha->current.value;
                            }
                        }
                        else
                        {
                            newColor[3] = 1.0f;
                        }
                        xyClipped[0] = 0.0f;
                        xyClipped[1] = 0.0f;
                        clipped = CG_WorldPosToCompass(
                                                compassType,
                                                cgameGlob,
                                                &scaledRect,
                                                yawVector,
                                                cgameGlob->refdef.vieworg,
                                                origin,
                                                xy,
                                                xyClipped);
                        xy[0] = xy[0] + centerX;
                        xy[1] = xy[1] + centerY;
                        xyClipped[0] = xyClipped[0] + centerX;
                        xyClipped[1] = xyClipped[1] + centerY;
                        shouldRotateWithView = CalcCompassPointerSizeObjective(compassType, &w, &h, obj, &scaledRect);
                        icon = CG_ObjectiveIcon(localClientNum, obj->icon, 0);
                        if ( newColor[3] > fadeAlpha )
                            newColor[3] = fadeAlpha;
                        x = xy[0] - (float)(w * 0.5);
                        y = xy[1] - (float)(h * 0.5);
                        clipfade = GetObjectiveFade(&scaledRect, x, y, w, h);
                        if ( clipfade > 0.0 && obj->state == OBJST_CURRENT )
                        {
                            tempcolor = newColor[3];
                            newColor[3] = newColor[3] * clipfade;
                            if ( xy[1] >= scaledRect.y && (float)(scaledRect.y + scaledRect.h) >= xy[1] )
                                UI_DrawHandlePic(
                                    &scrPlaceView[localClientNum],
                                    scaledRect.x,
                                    xy[1] - (float)(2.0 * 0.5),
                                    scaledRect.w,
                                    2.0,
                                    rect->horzAlign,
                                    rect->vertAlign,
                                    newColor,
                                    material);
                            if ( xy[0] >= scaledRect.x && (float)(scaledRect.x + scaledRect.w) >= xy[0] )
                                CG_DrawVLine(
                                    &scrPlaceView[localClientNum],
                                    xy[0],
                                    scaledRect.y,
                                    2.0,
                                    scaledRect.h,
                                    rect->horzAlign,
                                    rect->vertAlign,
                                    newColor,
                                    material);
                            newColor[3] = tempcolor;
                        }
                        x = xyClipped[0] - (float)(w * 0.5);
                        y = xyClipped[1] - (float)(h * 0.5);
                        iconAngle = 0.0f;
                        if ( shouldRotateWithView )
                            iconAngle = cgameGlob->refdefViewAngles[1];
                        UI_DrawHandlePicRotated(
                            &scrPlaceView[localClientNum],
                            x,
                            y,
                            w,
                            h,
                            rect->horzAlign,
                            rect->vertAlign,
                            newColor,
                            iconAngle,
                            icon);
                    }
                }
            }
        }
    }
}

double __cdecl GetObjectiveFade(const rectDef_s *clipRect, float x, float y, float width, float height)
{
    float maxclip; // [esp+4h] [ebp-4h]

    maxclip = 0.0f;
    if ( (float)((float)(clipRect->x - x) / width) > 0.0 )
        maxclip = (float)(clipRect->x - x) / width;
    if ( (float)((float)(clipRect->y - y) / height) > maxclip )
        maxclip = (float)(clipRect->y - y) / height;
    if ( (float)((float)((float)(x + width) - (float)(clipRect->x + clipRect->w)) / width) > maxclip )
        maxclip = (float)((float)(x + width) - (float)(clipRect->x + clipRect->w)) / width;
    if ( (float)((float)((float)(y + height) - (float)(clipRect->y + clipRect->h)) / height) > maxclip )
        maxclip = (float)((float)(y + height) - (float)(clipRect->y + clipRect->h)) / height;
    if ( maxclip > 1.0 )
        maxclip = 1.0f;
    return 1.0 - maxclip;
}

char __cdecl CalcCompassPointerSizeObjective(
                CompassType compassType,
                float *w,
                float *h,
                const objective_t *objective,
                const rectDef_s *mapRect)
{
    float pixPerInch; // [esp+0h] [ebp-8h]
    bool shouldRotateWithView; // [esp+7h] [ebp-1h]

    CalcCompassPointerSize(compassType, w, h);
    shouldRotateWithView = 0;
    if ( compassType == COMPASS_TYPE_PARTIAL )
    {
        pixPerInch = mapRect->h / compassMaxRange->current.value;
        if ( objective->size[0] != 0.0 )
        {
            *w = objective->size[0] * pixPerInch;
            shouldRotateWithView = 1;
        }
        if ( objective->size[1] != 0.0 )
        {
            *h = objective->size[1] * pixPerInch;
            return 1;
        }
    }
    return shouldRotateWithView;
}

bool __cdecl CG_IsShowingZombieMap()
{
    return 0;
}

