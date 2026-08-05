#include "ik_layers.h"
#include "ik_import.h"
#include "ik_math.h"
#include <universal/com_math_anglevectors.h>
#include <demo/demo_common.h>

// IK's are like onions. Onions have layers!

float frametimeCapTime = 0.1f;
float frametimeResetCap = 0.5f;
float frametimeCap = 0.2f;
float pitchScalarDefault = 0.6f;
float pitchScalarProne = 1.0f;
float velIdleMax = 100.0f;



float leftHandRotationStatic[3];
float leftHandOffsetStatic[3];

// aislop
void IK_Layer_TerrainMapping(IKState *ikState)
{
    IKLayerNames layerName = IKLAYER_TERRAIN_MAPPING;
    float layerLerp = IKImport_GetLayerLerp(ikState, IKLAYER_TERRAIN_MAPPING, 0.1f);

    float pelvisMat[4][4];

    if (layerLerp <= 0.0)
    {
        ikCalcBoneModelMatrix(ikState, IKBONE_PELVIS, (*ikState->matArrayPostIK)[0], pelvisMat);
        ikState->lastTerrainMappingPelvisZ = pelvisMat[3][2] + ikState->entityXform[3][2];
        return;
    }

    bool bIsClient = ikState->entityNum < 32;

    if (!ikState->bJointVarsValid)
        return;

    // ---- Debug foot test mode ----
    if (IKImport_GetVar_IK_Foot_Test())
    {
        if (ikState->ikBoneToObjBone[IKBONE_LFOOT] == 161
            || ikState->ikBoneToObjBone[IKBONE_RFOOT] == 161
            || ikState->lastFrameMS[9] == ikState->timeMS)
        {
            return;
        }

        ikState->lastFrameMS[9] = ikState->timeMS;

        if (IK_IsCalcBone(ikState, IKBONE_LFOOT))
        {
            float wave = (float)(ikState->lastFrameMS[9] % 2000) - 1000.0;
            ikState->footMats[0][3][2] = (float)((float)(6.0 * (fabs(wave))) / 1000.0)
                + ikState->footMats[0][3][2];
        }

        if (IK_IsCalcBone(ikState, IKBONE_RFOOT))
        {
            ikState->footMats[1][3][2] = (float)((float)(1.0
                - (float)((fabs((float)(ikState->lastFrameMS[9] % 2000) - 1000.0))
                    / 1000.0))
                * 6.0)
                + ikState->footMats[1][3][2];
        }

        return;
    }

    // ---- Main terrain mapping path ----
    if (ikState->ikBoneToObjBone[IKBONE_LFOOT] == 161
        || ikState->ikBoneToObjBone[IKBONE_RFOOT] == 161
        || (!IK_IsCalcBone(ikState, IKBONE_LFOOT) && !IK_IsCalcBone(ikState, IKBONE_RFOOT)))
    {
        return;
    }

    // Compute frametime
    float frametime = (float)(ikState->timeMS - ikState->lastFrameMS[9]) * 0.001;
    if (frametime <= 0.0)
    {
        if (Demo_IsPlaying())
            frametime = 0.1f;
        else
            frametime = 0.0f;
    }
    ikState->lastFrameMS[9] = ikState->timeMS;
    float frametimeBase = frametime;

    // Terrain mapping constants
    float IK_TERRAIN_MAPPING_LERP_UP_MIN_RATE = 30.0f;
    float IK_TERRAIN_MAPPING_LERP_DOWN_MIN_RATE = 30.0f;
    float IK_TERRAIN_MAPPING_LERP_SCALE = 6.0f;
    float IK_TERRAIN_MAPPING_CLAMP = 32.0f;
    float IK_TERRAIN_MAPPING_VELOCITY_MAX = 300.0f;
    float IK_TERRAIN_MAPPING_VELOCITY_SCALAR[2] = { 7.0f, 7.0f };
    float IK_TERRAIN_MAPPING_FRAMETIME_SCALAR_MAX = 8.0f;
    float IK_TERRAIN_MAPPING_FRAMETIME_SCALAR_MIN = 1.0f;

    float footTraceOffsetFromAnkle[2] = { 0.0f, 8.0f };
    float footTraceLiftNormal = 24.0f;
    float footTraceLiftTurret = 30.0f;
    float entityBoundsMinZ = 0.0f;
    float footTraceEndOffsetZMoving = 40.0f;
    float footTraceEndOffsetZIdle[2] = { 20.0f, 20.0f };
    float maxPelvisReachAdjust = -0.5f;
    float footInAirLiftReductionMaxInAir = 16.0f;
    float minPelvisHeightNormal = 6.0f;
    float minPelvisHeightCrouching = 13.0f;
    float minPelvisHeightTurret = 16.0f;
    float maxFootSpan = 18.0f;
    float maxFootDropIdle = 12.0f;
    float maxFootDropIdleCr = 15.0f;
    float maxFootDropMoving = 30.0f;
    float maxFootRaiseStanding = 17.0f;
    float maxFootRaiseCrouching = 17.0f;
    float maxFootRaiseTurret = 27.0f;
    float frametimeScalar = 1.0f;
    bool fastLerp = 0;

    float footTraceEndOffsetZ = footTraceEndOffsetZIdle[bIsClient];

    ikCalcBoneModelMatrix(ikState, IKBONE_PELVIS, (*ikState->matArrayPostIK)[0], pelvisMat);

    // Compute origin change to determine if lerping is valid
    float originChange[3];
    originChange[0] = ikState->entityXform[3][0] - ikState->lastTerrainMappingOrigin[0];
    originChange[1] = ikState->entityXform[3][1] - ikState->lastTerrainMappingOrigin[1];
    originChange[2] = ikState->entityXform[3][2] - ikState->lastTerrainMappingOrigin[2];

    float originDistSq = (float)((float)(originChange[0] * originChange[0]) + (float)(originChange[1] * originChange[1]))
        + (float)(originChange[2] * originChange[2]);

    bool doLerping = originDistSq < 4096.0;

    // Cap frametime, reset if too large
    if (frametime > frametimeCap)
    {
        if (frametime > frametimeResetCap)
        {
            ikState->lastTerrainMappingOrigin[0] = ikState->entityXform[3][0];
            ikState->lastTerrainMappingOrigin[1] = ikState->entityXform[3][1];
            ikState->lastTerrainMappingOrigin[2] = ikState->entityXform[3][2];
            ikState->lastTerrainMappingFootOffsetZ[0] = 0.0f;
            ikState->lastTerrainMappingFootOffsetZ[1] = 0.0f;
            ikState->lastTerrainMappingPelvisZ = pelvisMat[3][2] + ikState->entityXform[3][2];
        }
        frametime = frametimeCapTime;
        frametimeBase = frametimeCapTime;
        doLerping = 0;
    }

    // Velocity-based scaling
    float velSpeed = Vec3Length(ikState->velocity);
    float velRatio = velSpeed / IK_TERRAIN_MAPPING_VELOCITY_MAX;
    float velSpeedScale;
    if ((float)(1.0 - velRatio) < 0.0)
        velSpeedScale = 1.0f;
    else
        velSpeedScale = velRatio;

    float velTimeScalar = (float)((float)(IK_TERRAIN_MAPPING_VELOCITY_SCALAR[bIsClient]
        - IK_TERRAIN_MAPPING_FRAMETIME_SCALAR_MIN)
        * velSpeedScale)
        + IK_TERRAIN_MAPPING_FRAMETIME_SCALAR_MIN;

    footTraceEndOffsetZ = (float)((float)(1.0 - velSpeedScale) * footTraceEndOffsetZIdle[bIsClient])
        + (float)(footTraceEndOffsetZMoving * velSpeedScale);

    // Compute max foot drop based on crouch/velocity blend
    float crouchLerp = IKImport_GetLayerLerp(ikState, IKLAYER_CROUCH_ACTIVE);
    float maxFootDrop = (float)((float)(1.0 - velSpeedScale)
        * (float)((float)((float)(1.0 - crouchLerp) * maxFootDropIdle) + (float)(maxFootDropIdleCr * crouchLerp)))
        + (float)(maxFootDropMoving * velSpeedScale);

    // Compute max foot raise based on crouch/turret blend
    float crouchLerp2 = IKImport_GetLayerLerp(ikState, IKLAYER_CROUCH_ACTIVE);
    float turretLerp = IKImport_GetLayerLerp(ikState, IKLAYER_TURRET_OFFSET);
    float maxFootRaise = (float)((float)(1.0 - turretLerp)
        * (float)((float)((float)(1.0 - crouchLerp2) * maxFootRaiseStanding)
            + (float)(maxFootRaiseCrouching * crouchLerp2)))
        + (float)(maxFootRaiseTurret * turretLerp);

    // Compute min pelvis height based on crouch/turret blend
    float crouchLerp3 = IKImport_GetLayerLerp(ikState, IKLAYER_CROUCH_ACTIVE);
    float turretLerp2 = IKImport_GetLayerLerp(ikState, IKLAYER_TURRET_OFFSET);
    float minPelvisHeight = (float)((float)((float)((float)(1.0 - crouchLerp3) * minPelvisHeightNormal)
        + (float)(minPelvisHeightCrouching * crouchLerp3))
        * (float)(1.0 - turretLerp2))
        + (float)(minPelvisHeightTurret * turretLerp2);

    // Foot trace lift based on turret blend
    float turretLerp3 = IKImport_GetLayerLerp(ikState, IKLAYER_TURRET_OFFSET);
    float footTraceLift = (float)((float)(1.0 - turretLerp3) * footTraceLiftNormal) + (float)(footTraceLiftTurret * turretLerp3);

    // Update last terrain mapping origin
    ikState->lastTerrainMappingOrigin[0] = ikState->entityXform[3][0];
    ikState->lastTerrainMappingOrigin[1] = ikState->entityXform[3][1];
    ikState->lastTerrainMappingOrigin[2] = ikState->entityXform[3][2];

    // ---- Copy footMats and compute world-space foot matrices ----
    // worldFootMats[i] = footMats[i] * entityLegsXform
    float worldFootMats[2][4][4];
    float tmpMat[4][4];
    for (int i = 0; i < 2; ++i)
    {
        // Copy current footMat to tmpMat
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                tmpMat[r][c] = ikState->footMats[i][r][c];

        // 4x4 multiply: worldFootMats[i] = tmpMat * entityLegsXform
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                worldFootMats[i][r][c] = (float)((float)((float)(tmpMat[r][0] * ikState->entityLegsXform[0][c])
                    + (float)(tmpMat[r][1] * ikState->entityLegsXform[1][c]))
                    + (float)(tmpMat[r][2] * ikState->entityLegsXform[2][c]))
                    + (float)(tmpMat[r][3] * ikState->entityLegsXform[3][c]);
    }

    // ---- Per-foot ground trace and height adjustment ----
    float lowestGroundZ = 9999999.0f;
    float highestGroundZ = -9999999.0f;
    float maxPelvisReach = (float)((float)(ikState->jointVars[2].LowerLength + ikState->jointVars[2].UpperLength)
        + ikState->footHeightFromGround)
        + maxPelvisReachAdjust;

    float footTraceNormals[2][3];
    bool modifiedFeet[4];
    float groundOffsets[2];
    float startPos[3];
    float endPos[3];
    float start[3];
    float end[3];

    for (int i = 0; i < 2; ++i)
    {
        frametime = frametimeBase;
        float terrainZ = 0.0f;
        groundOffsets[i] = 0.0f;
        frametimeScalar = 1.0f;
        modifiedFeet[i] = 0;
        float footTraceBoxRadius = 3.0f;

        if (IKImport_GetVar_IK_Debug() == 1)
            IKImport_DrawLine(startPos, endPos, i);

        // Initialize foot trace normals from cached goal normals
        float goalNormalLen = Vec3Length(ikState->lastFootTraceGoalNormals[i]) - 1.0;
        if ((fabs(goalNormalLen)) >= 0.1)
        {
            // Goal normal is invalid, reset to up
            footTraceNormals[i][0] = 0.0f;
            footTraceNormals[i][1] = 0.0f;
            footTraceNormals[i][2] = 1.0f;
            ikState->lastFootTraceGoalNormals[i][0] = footTraceNormals[i][0];
            ikState->lastFootTraceGoalNormals[i][1] = footTraceNormals[i][1];
            ikState->lastFootTraceGoalNormals[i][2] = footTraceNormals[i][2];
        }
        else
        {
            // Copy cached goal normal
            footTraceNormals[i][0] = ikState->lastFootTraceGoalNormals[i][0];
            footTraceNormals[i][1] = ikState->lastFootTraceGoalNormals[i][1];
            footTraceNormals[i][2] = ikState->lastFootTraceGoalNormals[i][2];
        }

        // Check if foot position has moved enough to re-trace
        float footTraceCacheDistThreshold = 0.2f;
        float footDelta[2];
        footDelta[0] = worldFootMats[i][3][0] - ikState->lastTerrainMappingTraceFootPos[i][0];
        footDelta[1] = worldFootMats[i][3][1] - ikState->lastTerrainMappingTraceFootPos[i][1];
        float footDist = Vec2Length(footDelta);

        if (footDist <= footTraceCacheDistThreshold)
        {
            // Foot hasn't moved much, use cached result
            float cacheDelta[2];
            cacheDelta[0] = worldFootMats[i][3][0] - ikState->lastTerrainMappingTraceFootPos[i][0];
            cacheDelta[1] = worldFootMats[i][3][1] - ikState->lastTerrainMappingTraceFootPos[i][1];
            float cacheDist = Vec2Length(cacheDelta);
            if (footTraceCacheDistThreshold < cacheDist)
                terrainZ = ikState->lastTerrainMappingFootOffsetZ[i];
            else
                terrainZ = ikState->lastTerrainMappingTraceZ[i];
        }
        else
        {
            // Perform ground trace
            bool traceHitSomething = 0;
            float maxTraceZ = -99999.0f;
            float tracePos[3];
            float thisTracePos[3];
            float thisNormal[3];

            for (int n = 0; n < 2; ++n)
            {
                float sideSign = (float)(2 * (i == 0) - 1) * footTraceOffsetFromAnkle[n];
                startPos[0] = (float)(sideSign * worldFootMats[i][1][0]) + worldFootMats[i][3][0];
                startPos[1] = (float)(sideSign * worldFootMats[i][1][1]) + worldFootMats[i][3][1];
                startPos[2] = (float)(sideSign * worldFootMats[i][1][2]) + worldFootMats[i][3][2];
                startPos[2] = ikState->entityXform[3][2] + footTraceLift;
                endPos[0] = startPos[0];
                endPos[1] = startPos[1];
                endPos[2] = startPos[2];
                endPos[2] = (float)((float)(ikState->entityXform[3][2] - entityBoundsMinZ) - ikState->footHeightFromGround)
                    - footTraceEndOffsetZ;

                if (!IKImport_TraceBox(
                    ikState,
                    startPos,
                    endPos,
                    footTraceBoxRadius,
                    thisTracePos,
                    thisNormal,
                    n + 2 * i,
                    0,
                    1))
                {
                    continue;
                }

                if (thisTracePos[2] > maxTraceZ)
                {
                    tracePos[0] = thisTracePos[0];
                    tracePos[1] = thisTracePos[1];
                    tracePos[2] = thisTracePos[2];

                    float normalLenDelta = Vec3Length(thisNormal) - 1.0;
                    if ((fabs(normalLenDelta)) < 0.1)
                    {
                        footTraceNormals[i][0] = thisNormal[0];
                        footTraceNormals[i][1] = thisNormal[1];
                        footTraceNormals[i][2] = thisNormal[2];
                        ikState->lastFootTraceGoalNormals[i][0] = thisNormal[0];
                        ikState->lastFootTraceGoalNormals[i][1] = thisNormal[1];
                        ikState->lastFootTraceGoalNormals[i][2] = thisNormal[2];
                    }
                    maxTraceZ = thisTracePos[2];
                }
                traceHitSomething = 1;
            }

            if (traceHitSomething)
            {
                float traceFootAdjust = 0.0f;
                terrainZ = (float)(tracePos[2] - footTraceBoxRadius) + 0.0;

                // Terrain steepness and direction adjustments
                float lowerFootTerrainSteepnessScale = -1.0f;
                float lowerFootTerrainDotScale = 5.0f;
                float lowerFootTerrainDotClampMin = 0.0f;
                float lowerFootTerrainDotClampMax = 1.0f;

                float footDirectionToNormalDot = -((float)(ikState->lastFootTraceNormals[i][0] * worldFootMats[i][1][0])
                    + (float)(ikState->lastFootTraceNormals[i][1] * worldFootMats[i][1][1]));
                if (i == 1)
                    footDirectionToNormalDot = -footDirectionToNormalDot;

                // Clamp dot product
                if ((float)(footDirectionToNormalDot - lowerFootTerrainDotClampMax) < 0.0)
                    { /* keep footDirectionToNormalDot */ }
                else
                    footDirectionToNormalDot = lowerFootTerrainDotClampMax;

                float clampedDot = footDirectionToNormalDot;
                if ((float)(lowerFootTerrainDotClampMin - footDirectionToNormalDot) < 0.0)
                    { /* keep clampedDot */ }
                else
                    clampedDot = lowerFootTerrainDotClampMin;
                footDirectionToNormalDot = clampedDot;

                float normalXYLen = Vec2Length(ikState->lastFootTraceNormals[i]);
                terrainZ = terrainZ - normalXYLen * lowerFootTerrainSteepnessScale;
                terrainZ = terrainZ - (float)(footDirectionToNormalDot * lowerFootTerrainDotScale);

                // Reduce terrain lift when foot is in the air
                if (ikState->footMats[i][3][2] > ikState->footHeightFromGround && terrainZ > ikState->entityXform[3][2])
                {
                    float airRatio = (float)(terrainZ - ikState->entityXform[3][2]) / footInAirLiftReductionMaxInAir;
                    float airScale;
                    if (airRatio <= 1.0)
                        airScale = airRatio;
                    else
                        airScale = 1.0f;
                    terrainZ = terrainZ
                        - (float)((float)(ikState->footMats[i][3][2] - ikState->footHeightFromGround) * airScale);
                }

                ikState->lastTerrainMappingTraceZ[i] = terrainZ;

                if (IKImport_GetVar_IK_Debug() == 1)
                {
                    start[2] = tracePos[2];
                    end[2] = tracePos[2];
                    start[0] = tracePos[0] + 4.0;
                    start[1] = tracePos[1] + 4.0;
                    end[0] = tracePos[0] - 4.0;
                    end[1] = tracePos[1] - 4.0;
                    IKImport_DrawLine(start, end, 3);
                    start[2] = tracePos[2];
                    end[2] = tracePos[2];
                    start[0] = tracePos[0] - 4.0;
                    start[1] = tracePos[1] + 4.0;
                    end[0] = tracePos[0] + 4.0;
                    end[1] = tracePos[1] - 4.0;
                    IKImport_DrawLine(start, end, 3);
                }

                if (ikState->lerpInfo[3].goalState)
                {
                    ikState->lastTerrainMappingTraceFootPos[i][0] = worldFootMats[i][3][0];
                    ikState->lastTerrainMappingTraceFootPos[i][1] = worldFootMats[i][3][1];
                }
            }

            if (!traceHitSomething)
            {
                terrainZ = ikState->entityXform[3][2];
                ikState->lastTerrainMappingTraceZ[i] = terrainZ;
                footTraceNormals[i][0] = 0.0f;
                footTraceNormals[i][1] = 0.0f;
                footTraceNormals[i][2] = 1.0f;
                ikState->lastFootTraceGoalNormals[i][0] = footTraceNormals[i][0];
                ikState->lastFootTraceGoalNormals[i][1] = footTraceNormals[i][1];
                ikState->lastFootTraceGoalNormals[i][2] = footTraceNormals[i][2];
            }
        }

        // Clamp terrain Z to allowed range
        if ((float)(ikState->entityXform[3][2] - maxFootDrop) > terrainZ)
            terrainZ = ikState->entityXform[3][2] - maxFootDrop;
        if (terrainZ > (float)(ikState->entityXform[3][2] + maxFootRaise))
            terrainZ = ikState->entityXform[3][2] + maxFootRaise;

        // Smooth lerp terrain Z toward target
        if (doLerping)
        {
            float realZ = terrainZ;
            float terrainDiff = terrainZ - ikState->lastTerrainMappingFootOffsetZ[i];
            float terrainDiffNoLerpingThreshold = 128.0f;

            if ((fabs(terrainDiff)) < 128.0)
            {
                frametimeScalar = velTimeScalar;
                if (fastLerp)
                {
                    float fastLerpScalar[2] = { 5.0f, 10.0f };
                    frametimeScalar = frametimeScalar + fastLerpScalar[bIsClient];
                }
                else if ((fabs(realZ - terrainZ)) > IK_TERRAIN_MAPPING_CLAMP)
                {
                    float extraScalar = (float)((float)((fabs(realZ - terrainZ))
                        / IK_TERRAIN_MAPPING_CLAMP)
                        * 3.0)
                        + 1.0;
                    float maxScalar;
                    if (frametimeScalar <= extraScalar)
                        maxScalar = extraScalar;
                    else
                        maxScalar = frametimeScalar;
                    frametimeScalar = frametimeScalar + maxScalar;
                }

                if (!fastLerp && frametimeScalar > IK_TERRAIN_MAPPING_FRAMETIME_SCALAR_MAX)
                    frametimeScalar = IK_TERRAIN_MAPPING_FRAMETIME_SCALAR_MAX;

                frametime = frametime * frametimeScalar;
                terrainDiff = (float)(IK_TERRAIN_MAPPING_LERP_SCALE * frametime) * terrainDiff;

                if (terrainDiff <= 0.0)
                {
                    // Moving down
                    if (terrainDiff > (float)((-(IK_TERRAIN_MAPPING_LERP_DOWN_MIN_RATE)) * frametimeBase))
                        terrainDiff = (-(IK_TERRAIN_MAPPING_LERP_DOWN_MIN_RATE)) * frametimeBase;
                    float lerpedZ = ikState->lastTerrainMappingFootOffsetZ[i] + terrainDiff;
                    if (realZ <= lerpedZ)
                        terrainZ = lerpedZ;
                    else
                        terrainZ = realZ;
                }
                else
                {
                    // Moving up
                    if ((float)(IK_TERRAIN_MAPPING_LERP_UP_MIN_RATE * frametimeBase) > terrainDiff)
                        terrainDiff = IK_TERRAIN_MAPPING_LERP_UP_MIN_RATE * frametimeBase;
                    float lerpedZ = ikState->lastTerrainMappingFootOffsetZ[i] + terrainDiff;
                    if (lerpedZ <= realZ)
                        terrainZ = lerpedZ;
                    else
                        terrainZ = realZ;
                }
            }
        }

        // Apply foot offset with velocity/crouch scaling for downward offsets
        float footOffsetFromStandard = terrainZ - ikState->entityXform[3][2];
        float footOffsetAdjusted = footOffsetFromStandard;
        if (footOffsetFromStandard < 0.0)
        {
            float minScale = 1.0f;
            float crouchScale = IKImport_GetLayerLerp(ikState, IKLAYER_CROUCH_ACTIVE);
            float scaleVal = crouchScale + 1.0 - velSpeedScale;
            float clamped;
            if ((float)(scaleVal - 1.0) < 0.0)
                clamped = scaleVal;
            else
                clamped = 1.0f;
            if ((float)(minScale - scaleVal) < 0.0)
                { /* keep clamped */ }
            else
                clamped = minScale;
            footOffsetAdjusted = footOffsetAdjusted * clamped;
        }

        ikState->footMats[i][3][2] = (float)(layerLerp * footOffsetAdjusted) + ikState->footMats[i][3][2];

        if (lowestGroundZ > terrainZ)
            lowestGroundZ = terrainZ;
        if (terrainZ > highestGroundZ)
            highestGroundZ = terrainZ;
        ikState->lastTerrainMappingFootOffsetZ[i] = terrainZ;
    }

    // ---- Foot span clamping ----
    float footSpanDiff = ikState->lastTerrainMappingFootOffsetZ[0] - ikState->lastTerrainMappingFootOffsetZ[1];
    if ((fabs(footSpanDiff)) > maxFootSpan)
    {
        int lowerFoot = ikState->lastTerrainMappingFootOffsetZ[0] > ikState->lastTerrainMappingFootOffsetZ[1];
        float lowerAdjust = (float)((float)(ikState->lastTerrainMappingFootOffsetZ[lowerFoot == 0]
            - ikState->lastTerrainMappingFootOffsetZ[lowerFoot])
            - maxFootSpan)
            * layerLerp;
        ikState->footMats[lowerFoot][3][2] = ikState->footMats[lowerFoot][3][2] + lowerAdjust;
        ikState->lastTerrainMappingFootOffsetZ[lowerFoot] = ikState->lastTerrainMappingFootOffsetZ[lowerFoot]
            + lowerAdjust;
        lowestGroundZ = lowestGroundZ + lowerAdjust;
    }

    // ---- Pelvis height adjustment ----
    float pelvisMoveScale = 0.0f;
    float pelvisForceZAdjust = 0.0f;
    float pelvisMoveScaleMin = 0.75f;
    float notMovingLerp = IKImport_GetLayerLerp(ikState, IKLAYER_NOT_MOVING);
    float clampedNotMoving;
    if ((float)(notMovingLerp - 1.0) < 0.0)
        clampedNotMoving = notMovingLerp;
    else
        clampedNotMoving = 1.0f;
    if ((float)(pelvisMoveScaleMin - notMovingLerp) < 0.0)
        pelvisMoveScale = clampedNotMoving;
    else
        pelvisMoveScale = pelvisMoveScaleMin;

    float pelvisMovingZAdjust = 0.0f;
    pelvisForceZAdjust = (1.0 - IKImport_GetLayerLerp(ikState, IKLAYER_NOT_MOVING)) * 0.0;

    if (IK_IsCalcBone(ikState, IKBONE_PELVIS))
    {
        float zOfs = 0.0f;
        float PELVIS_ABS_SCALAR = 8.0f;

        float realPelvisZ;
        if (pelvisMat[3][2] <= (float)((float)(lowestGroundZ + maxPelvisReach) - ikState->entityXform[3][2]))
        {
            if ((float)((float)(highestGroundZ + minPelvisHeight) - ikState->entityXform[3][2]) <= pelvisMat[3][2])
                realPelvisZ = pelvisMat[3][2] + ikState->entityXform[3][2];
            else
                realPelvisZ = highestGroundZ + minPelvisHeight;
        }
        else
        {
            realPelvisZ = lowestGroundZ + maxPelvisReach;
        }

        float changeZ = realPelvisZ - ikState->lastTerrainMappingPelvisZ;
        if (doLerping)
        {
            float pelvisLerpRate = (float)((float)(frametimeBase * velTimeScalar) * pelvisMoveScale) * PELVIS_ABS_SCALAR;
            float pelvisLerpClamped;
            if ((float)(1.0 - pelvisLerpRate) < 0.0)
                pelvisLerpClamped = 1.0f;
            else
                pelvisLerpClamped = pelvisLerpRate;
            changeZ = changeZ * pelvisLerpClamped;
        }

        zOfs = (float)(changeZ + ikState->lastTerrainMappingPelvisZ)
            - (float)(pelvisMat[3][2] + ikState->entityXform[3][2]);
        ikState->lastTerrainMappingPelvisZ = ikState->lastTerrainMappingPelvisZ + changeZ;
        zOfs = zOfs * layerLerp;

        (*ikState->matArrayPostIK)[7][2] = (*ikState->matArrayPostIK)[7][2] + zOfs;

        if (!IK_GetStatus(ikState, IKSTATUS_MOUNTED_TURRET))
        {
            ikState->handMats[1][3][2] = ikState->handMats[1][3][2] + zOfs;
            ikState->handMats[0][3][2] = ikState->handMats[0][3][2] + zOfs;
        }

        ikState->bHasActiveLayers = 1;
        if (ikState->cacheActive)
        {
            memset(ikState->matArrayCache, 0, 0x5C0u);
            ikState->cacheActive = 0;
        }
        ikState->modifiedIKBones |= 2u;
    }

    // ---- Leg extension clamping ----
    float terrainLegExtendAdjust = 0.5f;
    if (!ikState->lerpInfo[1].goalState)
    {
        float parentMat[4][4];
        for (int i = 0; i < 2; ++i)
        {
            ikCalcBoneModelMatrix(
                ikState,
                ikState->localIkSystem->joints[i + 2].parentBone,
                (*ikState->matArrayPostIK)[0],
                parentMat);

            float dx = ikState->footMats[i][3][0] - parentMat[3][0];
            float dy = ikState->footMats[i][3][1] - parentMat[3][1];
            float aSq = (float)(dx * dx) + (float)(dy * dy);
            float cSq = Vec3DistanceSq(parentMat[3], ikState->footMats[i][3]);
            float dz = parentMat[3][2] - ikState->footMats[i][3][2];

            if ((float)((fabs(dz)) * (fabs(dz))) > aSq)
            {
                float maxLegLen = (float)(ikState->jointVars[i + 2].LowerLength + ikState->jointVars[i + 2].UpperLength)
                    - terrainLegExtendAdjust;
                float cMaxSq = maxLegLen * maxLegLen;
                if (cSq > (float)(maxLegLen * maxLegLen))
                {
                    float bSq = cMaxSq - aSq;
                    if ((float)(cMaxSq - aSq) > 0.1)
                    {
                        float bLen = sqrtf(bSq);
                        ikState->footMats[i][3][2] = parentMat[3][2] - bLen;
                    }
                }
            }
        }
    }

    // ---- Terrain normal rotation applied to foot matrices ----
    float terrainNormalLerpTime = 0.2f;
    float terrainNormalRangeCap = 0.40000001f;
    float terrainNormalMinZ = 0.2f;

    float invXform[4][4];
    float terrainAxis[4][4];

    for (int i = 0; i < 2; ++i)
    {
        float v_up[3] = { 0.0f, 0.0f, 1.0f };
        float angularMappingHeightOffsetMax = 4.0f;

        // Compute angular weight based on foot height offset
        float heightRatio = (float)(ikState->footMats[i][3][2] - ikState->footHeightFromGround) / 4.0;
        float clampedRatio;
        if ((float)(heightRatio - 1.0) < 0.0)
            clampedRatio = heightRatio;
        else
            clampedRatio = 1.0f;
        if ((float)(0.0 - heightRatio) < 0.0)
            { /* keep clampedRatio */ }
        else
            clampedRatio = 0.0f;
        float footOffsetAngularScale = 1.0 - clampedRatio;

        float angularWeight;
        if (ikState->lerpInfo[3].goalState)
            angularWeight = 1.0f;
        else
            angularWeight = footOffsetAngularScale;

        Vec3Lerp(v_up, footTraceNormals[i], angularWeight, footTraceNormals[i]);

        if (terrainNormalMinZ > footTraceNormals[i][2])
        {
            footTraceNormals[i][0] = v_up[0];
            footTraceNormals[i][1] = v_up[1];
            footTraceNormals[i][2] = v_up[2];
        }

        // Determine whether to lerp normals or snap
        float *lastNormal = ikState->lastFootTraceNormals[i];
        bool lastNormalIsZero = (lastNormal[0] == 0.0f && lastNormal[1] == 0.0f && lastNormal[2] == 0.0f);
        float lastNormalLenDelta = Vec3Length(ikState->lastFootTraceNormals[i]) - 1.0;
        bool lastNormalInvalid = (fabs(lastNormalLenDelta)) >= 0.1;

        if (!doLerping || lastNormalIsZero || lastNormalInvalid)
        {
            Vec3NormalizeTo(footTraceNormals[i], ikState->lastFootTraceNormals[i]);
        }
        else
        {
            float normalLerpFrac = frametime / terrainNormalLerpTime;
            float clampedFrac;
            if ((float)((float)(frametime / terrainNormalLerpTime) - 1.0) < 0.0)
                clampedFrac = normalLerpFrac;
            else
                clampedFrac = 1.0f;
            if ((float)(0.0 - normalLerpFrac) < 0.0)
                { /* keep clampedFrac */ }
            else
                clampedFrac = 0.0f;
            Vec3Lerp(ikState->lastFootTraceNormals[i], footTraceNormals[i], clampedFrac, ikState->lastFootTraceNormals[i]);
            Vec3Normalize(ikState->lastFootTraceNormals[i]);
        }

        // Cap terrain normal XY range
        float XYdist = Vec2Length(ikState->lastFootTraceNormals[i]);
        if (XYdist > terrainNormalRangeCap)
        {
            float capScale = terrainNormalRangeCap / XYdist;
            ikState->lastFootTraceNormals[i][0] = capScale * ikState->lastFootTraceNormals[i][0];
            ikState->lastFootTraceNormals[i][1] = capScale * ikState->lastFootTraceNormals[i][1];
            ikState->lastFootTraceNormals[i][2] = sqrtf(1.0 - (float)(terrainNormalRangeCap * terrainNormalRangeCap));
            Vec3Normalize(ikState->lastFootTraceNormals[i]);
        }

        // Build the final terrain normal (blended with layerLerp)
        float terrainNormal[3];
        if (layerLerp >= 1.0)
        {
            terrainNormal[0] = ikState->lastFootTraceNormals[i][0];
            terrainNormal[1] = ikState->lastFootTraceNormals[i][1];
            terrainNormal[2] = ikState->lastFootTraceNormals[i][2];
        }
        else
        {
            float upVec[3] = { 0.0f, 0.0f, 1.0f };
            Vec3Lerp(upVec, ikState->lastFootTraceNormals[i], layerLerp, terrainNormal);
            Vec3Normalize(terrainNormal);
        }

        // Build terrain axis from the terrain normal
        ikMatrixIdentity44(terrainAxis);

        // Transpose entityLegsXform into invXform (inline transpose)
        float *legsXform = ikState->entityLegsXform[0];
        if (ikState->entityLegsXform == invXform
            && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.h", 125, 0, "%s", "in != out"))
        {
            __debugbreak();
        }
        invXform[0][0] = legsXform[0];
        invXform[0][1] = legsXform[4];
        invXform[0][2] = legsXform[8];
        invXform[0][3] = legsXform[12];
        invXform[1][0] = legsXform[1];
        invXform[1][1] = legsXform[5];
        invXform[1][2] = legsXform[9];
        invXform[1][3] = legsXform[13];
        invXform[2][0] = legsXform[2];
        invXform[2][1] = legsXform[6];
        invXform[2][2] = legsXform[10];
        invXform[2][3] = legsXform[14];
        invXform[3][0] = legsXform[3];
        invXform[3][1] = legsXform[7];
        invXform[3][2] = legsXform[11];
        invXform[3][3] = legsXform[15];

        // Transform the terrain normal into local space for the Z axis
        ikMatrixTransformVector34(terrainNormal, invXform, terrainAxis[2]);

        // Set X axis to identity X, then build orthonormal basis
        terrainAxis[0][0] = 1.0f;
        terrainAxis[0][1] = 0.0f;
        terrainAxis[0][2] = 0.0;
        Vec3Cross(terrainAxis[2], terrainAxis[0], terrainAxis[1]);
        Vec3Normalize(terrainAxis[1]);
        Vec3Cross(terrainAxis[1], terrainAxis[2], terrainAxis[0]);
        Vec3Normalize(terrainAxis[0]);

        // Rotate foot matrix 3x3 rotation by terrainAxis
        // curFoot = copy of footMats[i] rotation rows
        // footMats[i] = curFoot * terrainAxis (3x3 only, row 0..2)
        float curFoot[3][3];
        curFoot[0][0] = ikState->footMats[i][0][0];
        curFoot[0][1] = ikState->footMats[i][0][1];
        curFoot[0][2] = ikState->footMats[i][0][2];
        curFoot[1][0] = ikState->footMats[i][1][0];
        curFoot[1][1] = ikState->footMats[i][1][1];
        curFoot[1][2] = ikState->footMats[i][1][2];
        curFoot[2][0] = ikState->footMats[i][2][0];
        curFoot[2][1] = ikState->footMats[i][2][1];
        curFoot[2][2] = ikState->footMats[i][2][2];

        float *out = ikState->footMats[i][0];
        out[0]  = (float)((float)(curFoot[0][0] * terrainAxis[0][0])
            + (float)(curFoot[0][1] * terrainAxis[1][0]))
            + (float)(curFoot[0][2] * terrainAxis[2][0]);
        out[1]  = (float)((float)(curFoot[0][0] * terrainAxis[0][1])
            + (float)(curFoot[0][1] * terrainAxis[1][1]))
            + (float)(curFoot[0][2] * terrainAxis[2][1]);
        out[2]  = (float)((float)(curFoot[0][0] * terrainAxis[0][2])
            + (float)(curFoot[0][1] * terrainAxis[1][2]))
            + (float)(curFoot[0][2] * terrainAxis[2][2]);
        out[3]  = 0.0f;
        out[4]  = (float)((float)(curFoot[1][0] * terrainAxis[0][0])
            + (float)(curFoot[1][1] * terrainAxis[1][0]))
            + (float)(curFoot[1][2] * terrainAxis[2][0]);
        out[5]  = (float)((float)(curFoot[1][0] * terrainAxis[0][1])
            + (float)(curFoot[1][1] * terrainAxis[1][1]))
            + (float)(curFoot[1][2] * terrainAxis[2][1]);
        out[6]  = (float)((float)(curFoot[1][0] * terrainAxis[0][2])
            + (float)(curFoot[1][1] * terrainAxis[1][2]))
            + (float)(curFoot[1][2] * terrainAxis[2][2]);
        out[7]  = 0.0f;
        out[8]  = (float)((float)(curFoot[2][0] * terrainAxis[0][0])
            + (float)(curFoot[2][1] * terrainAxis[1][0]))
            + (float)(curFoot[2][2] * terrainAxis[2][0]);
        out[9]  = (float)((float)(curFoot[2][0] * terrainAxis[0][1])
            + (float)(curFoot[2][1] * terrainAxis[1][1]))
            + (float)(curFoot[2][2] * terrainAxis[2][1]);
        out[10] = (float)((float)(curFoot[2][0] * terrainAxis[0][2])
            + (float)(curFoot[2][1] * terrainAxis[1][2]))
            + (float)(curFoot[2][2] * terrainAxis[2][2]);
        out[11] = 0.0f;
    }
}

// local variable allocation has failed, the output may be wrong!
void    IK_Layer_ApplyFootIK(
                IKState *ikState,
                IKJointBones *jointBones,
                IKJointVars *jointVars,
                float (*childMat)[4])
{
    bool flip = (jointBones->parentBone == IKBONE_RTHIGH);

    if (!ikState->bJointVarsValid)
        return;
    if (ikState->ikBoneToObjBone[jointBones->childBone] == 161)
        return;
    if (!IK_IsCalcBone(ikState, jointBones->childBone))
        return;

    if (flip)
        IK_FlipHack(childMat);

    float parentIKMat[4][4];
    float jointIKMat[4][4];

    ikSolveLegJoint(ikState, jointBones, jointVars, childMat[3], parentIKMat, jointIKMat, flip);

    float baseMat[4][4];
    ikCalcBoneModelMatrix(ikState, jointBones->baseBone, (*ikState->matArrayPostIK)[0], baseMat);

    // ----- PARENT BONE (thigh) -----
    // outRot = parentIKMat_rot * transpose(baseMat_rot)
    {
        float *out = ikState->matArrayPostIK[jointBones->parentBone][0];
        float t[3][4];
        t[0][0] = baseMat[0][0]; t[0][1] = baseMat[1][0]; t[0][2] = baseMat[2][0]; t[0][3] = 0.0f;
        t[1][0] = baseMat[0][1]; t[1][1] = baseMat[1][1]; t[1][2] = baseMat[2][1]; t[1][3] = 0.0f;
        t[2][0] = baseMat[0][2]; t[2][1] = baseMat[1][2]; t[2][2] = baseMat[2][2]; t[2][3] = 0.0f;

        out[0]  = parentIKMat[0][0]*t[0][0] + parentIKMat[0][1]*t[1][0] + parentIKMat[0][2]*t[2][0];
        out[1]  = parentIKMat[0][0]*t[0][1] + parentIKMat[0][1]*t[1][1] + parentIKMat[0][2]*t[2][1];
        out[2]  = parentIKMat[0][0]*t[0][2] + parentIKMat[0][1]*t[1][2] + parentIKMat[0][2]*t[2][2];
        out[3]  = 0.0f;
        out[4]  = parentIKMat[1][0]*t[0][0] + parentIKMat[1][1]*t[1][0] + parentIKMat[1][2]*t[2][0];
        out[5]  = parentIKMat[1][0]*t[0][1] + parentIKMat[1][1]*t[1][1] + parentIKMat[1][2]*t[2][1];
        out[6]  = parentIKMat[1][0]*t[0][2] + parentIKMat[1][1]*t[1][2] + parentIKMat[1][2]*t[2][2];
        out[7]  = 0.0f;
        out[8]  = parentIKMat[2][0]*t[0][0] + parentIKMat[2][1]*t[1][0] + parentIKMat[2][2]*t[2][0];
        out[9]  = parentIKMat[2][0]*t[0][1] + parentIKMat[2][1]*t[1][1] + parentIKMat[2][2]*t[2][1];
        out[10] = parentIKMat[2][0]*t[0][2] + parentIKMat[2][1]*t[1][2] + parentIKMat[2][2]*t[2][2];
        out[11] = 0.0f;

        float delta[3];
        delta[0] = parentIKMat[3][0] - baseMat[3][0];
        delta[1] = parentIKMat[3][1] - baseMat[3][1];
        delta[2] = parentIKMat[3][2] - baseMat[3][2];
        ikMatrixTransformVector34(delta, (const float (*)[4])t, out + 12);
        out[15] = 1.0f;
    }

    ikNormalizedMatrixAssert_func(ikState->matArrayPostIK[jointBones->parentBone]);
    ikState->bHasActiveLayers = 1;
    if (ikState->cacheActive)
    {
        memset(ikState->matArrayCache, 0, 0x5C0u);
        ikState->cacheActive = 0;
    }
    if (jointBones->parentBone != IKBONE_NONE)
        ikState->modifiedIKBones |= 1 << jointBones->parentBone;

    // ----- JOINT BONE (knee) -----
    // outRot = jointIKMat_rot * transpose(parentIKMat_rot)
    {
        float *out = ikState->matArrayPostIK[jointBones->jointBone][0];
        float t[3][4];
        t[0][0] = parentIKMat[0][0]; t[0][1] = parentIKMat[1][0]; t[0][2] = parentIKMat[2][0]; t[0][3] = 0.0f;
        t[1][0] = parentIKMat[0][1]; t[1][1] = parentIKMat[1][1]; t[1][2] = parentIKMat[2][1]; t[1][3] = 0.0f;
        t[2][0] = parentIKMat[0][2]; t[2][1] = parentIKMat[1][2]; t[2][2] = parentIKMat[2][2]; t[2][3] = 0.0f;

        out[0]  = jointIKMat[0][0]*t[0][0] + jointIKMat[0][1]*t[1][0] + jointIKMat[0][2]*t[2][0];
        out[1]  = jointIKMat[0][0]*t[0][1] + jointIKMat[0][1]*t[1][1] + jointIKMat[0][2]*t[2][1];
        out[2]  = jointIKMat[0][0]*t[0][2] + jointIKMat[0][1]*t[1][2] + jointIKMat[0][2]*t[2][2];
        out[3]  = 0.0f;
        out[4]  = jointIKMat[1][0]*t[0][0] + jointIKMat[1][1]*t[1][0] + jointIKMat[1][2]*t[2][0];
        out[5]  = jointIKMat[1][0]*t[0][1] + jointIKMat[1][1]*t[1][1] + jointIKMat[1][2]*t[2][1];
        out[6]  = jointIKMat[1][0]*t[0][2] + jointIKMat[1][1]*t[1][2] + jointIKMat[1][2]*t[2][2];
        out[7]  = 0.0f;
        out[8]  = jointIKMat[2][0]*t[0][0] + jointIKMat[2][1]*t[1][0] + jointIKMat[2][2]*t[2][0];
        out[9]  = jointIKMat[2][0]*t[0][1] + jointIKMat[2][1]*t[1][1] + jointIKMat[2][2]*t[2][1];
        out[10] = jointIKMat[2][0]*t[0][2] + jointIKMat[2][1]*t[1][2] + jointIKMat[2][2]*t[2][2];
        out[11] = 0.0f;

        float delta[3];
        delta[0] = jointIKMat[3][0] - parentIKMat[3][0];
        delta[1] = jointIKMat[3][1] - parentIKMat[3][1];
        delta[2] = jointIKMat[3][2] - parentIKMat[3][2];
        ikMatrixTransformVector34(delta, (const float (*)[4])t, out + 12);
        out[15] = 1.0f;
    }

    ikNormalizedMatrixAssert_func(ikState->matArrayPostIK[jointBones->jointBone]);
    if (jointBones->jointBone != IKBONE_NONE)
        ikState->modifiedIKBones |= 1 << jointBones->jointBone;

    if (jointVars->UpperLength + 1.0 <= Vec3Distance(parentIKMat[3], jointIKMat[3])
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_layers.cpp",
            728,
            0,
            "%s",
            "Vec3Distance( parentIKMat[3], jointIKMat[3] ) < jointVars->UpperLength + 1.f"))
    {
        __debugbreak();
    }
    if (jointVars->LowerLength + 5.0 <= Vec3Distance(childMat[3], jointIKMat[3])
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_layers.cpp",
            729,
            0,
            "%s",
            "Vec3Distance( childMat[3], jointIKMat[3] ) < jointVars->LowerLength + 5.f"))
    {
        __debugbreak();
    }

    if (flip)
        IK_FlipHack(childMat);

    // ----- CHILD BONE (foot) -----
    // outRot = childMat_rot * transpose(jointIKMat_rot)
    {
        float *out = ikState->matArrayPostIK[jointBones->childBone][0];
        float t[3][4];
        t[0][0] = jointIKMat[0][0]; t[0][1] = jointIKMat[1][0]; t[0][2] = jointIKMat[2][0]; t[0][3] = 0.0f;
        t[1][0] = jointIKMat[0][1]; t[1][1] = jointIKMat[1][1]; t[1][2] = jointIKMat[2][1]; t[1][3] = 0.0f;
        t[2][0] = jointIKMat[0][2]; t[2][1] = jointIKMat[1][2]; t[2][2] = jointIKMat[2][2]; t[2][3] = 0.0f;

        out[0]  = childMat[0][0]*t[0][0] + childMat[0][1]*t[1][0] + childMat[0][2]*t[2][0];
        out[1]  = childMat[0][0]*t[0][1] + childMat[0][1]*t[1][1] + childMat[0][2]*t[2][1];
        out[2]  = childMat[0][0]*t[0][2] + childMat[0][1]*t[1][2] + childMat[0][2]*t[2][2];
        out[3]  = 0.0f;
        out[4]  = childMat[1][0]*t[0][0] + childMat[1][1]*t[1][0] + childMat[1][2]*t[2][0];
        out[5]  = childMat[1][0]*t[0][1] + childMat[1][1]*t[1][1] + childMat[1][2]*t[2][1];
        out[6]  = childMat[1][0]*t[0][2] + childMat[1][1]*t[1][2] + childMat[1][2]*t[2][2];
        out[7]  = 0.0f;
        out[8]  = childMat[2][0]*t[0][0] + childMat[2][1]*t[1][0] + childMat[2][2]*t[2][0];
        out[9]  = childMat[2][0]*t[0][1] + childMat[2][1]*t[1][1] + childMat[2][2]*t[2][1];
        out[10] = childMat[2][0]*t[0][2] + childMat[2][1]*t[1][2] + childMat[2][2]*t[2][2];
        out[11] = 0.0f;

        float delta[3];
        delta[0] = childMat[3][0] - jointIKMat[3][0];
        delta[1] = childMat[3][1] - jointIKMat[3][1];
        delta[2] = childMat[3][2] - jointIKMat[3][2];
        ikMatrixTransformVector34(delta, (const float (*)[4])t, out + 12);
        out[15] = 1.0f;
    }

    ikNormalizedMatrixAssert_func(ikState->matArrayPostIK[jointBones->childBone]);
    if (jointBones->childBone != IKBONE_NONE)
        ikState->modifiedIKBones |= 1 << jointBones->childBone;
}

// aislop
void    IK_Layer_ApplyHandIK(
                IKState *ikState,
                IKJointBones *jointBones,
                IKJointVars *jointVars,
                float (*childMat)[4])
{
    bool flip = (jointBones->childBone == IKBONE_RHAND);

    if (!ikState->bJointVarsValid)
        return;
    if (ikState->ikBoneToObjBone[jointBones->childBone] == 161)
        return;
    if (!IK_IsCalcBone(ikState, jointBones->childBone))
        return;

    if (flip)
        IK_FlipHack(childMat);

    float parentIKMat[4][4];
    float jointIKMat[4][4];

    ikSolveArmJoint(ikState, jointBones, jointVars, childMat[3], parentIKMat, jointIKMat, flip);

    float baseMat[4][4];
    ikCalcBoneModelMatrix(ikState, jointBones->baseBone, (*ikState->matArrayPostIK)[0], baseMat);

    // ----- PARENT BONE (upper arm) -----
    // outRot = parentIKMat_rot * transpose(baseMat_rot)
    {
        float *out = ikState->matArrayPostIK[jointBones->parentBone][0];
        float t[3][4];
        t[0][0] = baseMat[0][0]; t[0][1] = baseMat[1][0]; t[0][2] = baseMat[2][0]; t[0][3] = 0.0f;
        t[1][0] = baseMat[0][1]; t[1][1] = baseMat[1][1]; t[1][2] = baseMat[2][1]; t[1][3] = 0.0f;
        t[2][0] = baseMat[0][2]; t[2][1] = baseMat[1][2]; t[2][2] = baseMat[2][2]; t[2][3] = 0.0f;

        out[0]  = parentIKMat[0][0]*t[0][0] + parentIKMat[0][1]*t[1][0] + parentIKMat[0][2]*t[2][0];
        out[1]  = parentIKMat[0][0]*t[0][1] + parentIKMat[0][1]*t[1][1] + parentIKMat[0][2]*t[2][1];
        out[2]  = parentIKMat[0][0]*t[0][2] + parentIKMat[0][1]*t[1][2] + parentIKMat[0][2]*t[2][2];
        out[3]  = 0.0f;
        out[4]  = parentIKMat[1][0]*t[0][0] + parentIKMat[1][1]*t[1][0] + parentIKMat[1][2]*t[2][0];
        out[5]  = parentIKMat[1][0]*t[0][1] + parentIKMat[1][1]*t[1][1] + parentIKMat[1][2]*t[2][1];
        out[6]  = parentIKMat[1][0]*t[0][2] + parentIKMat[1][1]*t[1][2] + parentIKMat[1][2]*t[2][2];
        out[7]  = 0.0f;
        out[8]  = parentIKMat[2][0]*t[0][0] + parentIKMat[2][1]*t[1][0] + parentIKMat[2][2]*t[2][0];
        out[9]  = parentIKMat[2][0]*t[0][1] + parentIKMat[2][1]*t[1][1] + parentIKMat[2][2]*t[2][1];
        out[10] = parentIKMat[2][0]*t[0][2] + parentIKMat[2][1]*t[1][2] + parentIKMat[2][2]*t[2][2];
        out[11] = 0.0f;

        float delta[3];
        delta[0] = parentIKMat[3][0] - baseMat[3][0];
        delta[1] = parentIKMat[3][1] - baseMat[3][1];
        delta[2] = parentIKMat[3][2] - baseMat[3][2];
        ikMatrixTransformVector34(delta, (const float (*)[4])t, out + 12);
        out[15] = 1.0f;
    }

    ikState->bHasActiveLayers = 1;
    if (ikState->cacheActive)
    {
        memset(ikState->matArrayCache, 0, 0x5C0u);
        ikState->cacheActive = 0;
    }
    if (jointBones->parentBone != IKBONE_NONE)
        ikState->modifiedIKBones |= 1 << jointBones->parentBone;

    // ----- JOINT BONE (forearm) -----
    // outRot = jointIKMat_rot * transpose(parentIKMat_rot)
    {
        float *out = ikState->matArrayPostIK[jointBones->jointBone][0];
        float t[3][4];
        t[0][0] = parentIKMat[0][0]; t[0][1] = parentIKMat[1][0]; t[0][2] = parentIKMat[2][0]; t[0][3] = 0.0f;
        t[1][0] = parentIKMat[0][1]; t[1][1] = parentIKMat[1][1]; t[1][2] = parentIKMat[2][1]; t[1][3] = 0.0f;
        t[2][0] = parentIKMat[0][2]; t[2][1] = parentIKMat[1][2]; t[2][2] = parentIKMat[2][2]; t[2][3] = 0.0f;

        out[0]  = jointIKMat[0][0]*t[0][0] + jointIKMat[0][1]*t[1][0] + jointIKMat[0][2]*t[2][0];
        out[1]  = jointIKMat[0][0]*t[0][1] + jointIKMat[0][1]*t[1][1] + jointIKMat[0][2]*t[2][1];
        out[2]  = jointIKMat[0][0]*t[0][2] + jointIKMat[0][1]*t[1][2] + jointIKMat[0][2]*t[2][2];
        out[3]  = 0.0f;
        out[4]  = jointIKMat[1][0]*t[0][0] + jointIKMat[1][1]*t[1][0] + jointIKMat[1][2]*t[2][0];
        out[5]  = jointIKMat[1][0]*t[0][1] + jointIKMat[1][1]*t[1][1] + jointIKMat[1][2]*t[2][1];
        out[6]  = jointIKMat[1][0]*t[0][2] + jointIKMat[1][1]*t[1][2] + jointIKMat[1][2]*t[2][2];
        out[7]  = 0.0f;
        out[8]  = jointIKMat[2][0]*t[0][0] + jointIKMat[2][1]*t[1][0] + jointIKMat[2][2]*t[2][0];
        out[9]  = jointIKMat[2][0]*t[0][1] + jointIKMat[2][1]*t[1][1] + jointIKMat[2][2]*t[2][1];
        out[10] = jointIKMat[2][0]*t[0][2] + jointIKMat[2][1]*t[1][2] + jointIKMat[2][2]*t[2][2];
        out[11] = 0.0f;

        float delta[3];
        delta[0] = jointIKMat[3][0] - parentIKMat[3][0];
        delta[1] = jointIKMat[3][1] - parentIKMat[3][1];
        delta[2] = jointIKMat[3][2] - parentIKMat[3][2];
        ikMatrixTransformVector34(delta, (const float (*)[4])t, out + 12);
        out[15] = 1.0f;
    }

    ikState->bHasActiveLayers = 1;
    if (ikState->cacheActive)
    {
        memset(ikState->matArrayCache, 0, 0x5C0u);
        ikState->cacheActive = 0;
    }
    if (jointBones->jointBone != IKBONE_NONE)
        ikState->modifiedIKBones |= 1 << jointBones->jointBone;

    if (flip)
        IK_FlipHack(childMat);

    // ----- CHILD BONE (hand) -----
    // outRot = childMat_rot * transpose(jointIKMat_rot)
    {
        float *out = ikState->matArrayPostIK[jointBones->childBone][0];
        float t[3][4];
        t[0][0] = jointIKMat[0][0]; t[0][1] = jointIKMat[1][0]; t[0][2] = jointIKMat[2][0]; t[0][3] = 0.0f;
        t[1][0] = jointIKMat[0][1]; t[1][1] = jointIKMat[1][1]; t[1][2] = jointIKMat[2][1]; t[1][3] = 0.0f;
        t[2][0] = jointIKMat[0][2]; t[2][1] = jointIKMat[1][2]; t[2][2] = jointIKMat[2][2]; t[2][3] = 0.0f;

        out[0]  = childMat[0][0]*t[0][0] + childMat[0][1]*t[1][0] + childMat[0][2]*t[2][0];
        out[1]  = childMat[0][0]*t[0][1] + childMat[0][1]*t[1][1] + childMat[0][2]*t[2][1];
        out[2]  = childMat[0][0]*t[0][2] + childMat[0][1]*t[1][2] + childMat[0][2]*t[2][2];
        out[3]  = 0.0f;
        out[4]  = childMat[1][0]*t[0][0] + childMat[1][1]*t[1][0] + childMat[1][2]*t[2][0];
        out[5]  = childMat[1][0]*t[0][1] + childMat[1][1]*t[1][1] + childMat[1][2]*t[2][1];
        out[6]  = childMat[1][0]*t[0][2] + childMat[1][1]*t[1][2] + childMat[1][2]*t[2][2];
        out[7]  = 0.0f;
        out[8]  = childMat[2][0]*t[0][0] + childMat[2][1]*t[1][0] + childMat[2][2]*t[2][0];
        out[9]  = childMat[2][0]*t[0][1] + childMat[2][1]*t[1][1] + childMat[2][2]*t[2][1];
        out[10] = childMat[2][0]*t[0][2] + childMat[2][1]*t[1][2] + childMat[2][2]*t[2][2];
        out[11] = 0.0f;

        float delta[3];
        delta[0] = childMat[3][0] - jointIKMat[3][0];
        delta[1] = childMat[3][1] - jointIKMat[3][1];
        delta[2] = childMat[3][2] - jointIKMat[3][2];
        ikMatrixTransformVector34(delta, (const float (*)[4])t, out + 12);
        out[15] = 1.0f;
    }

    ikState->bHasActiveLayers = 1;
    if (ikState->cacheActive)
    {
        memset(ikState->matArrayCache, 0, 0x5C0u);
        ikState->cacheActive = 0;
    }
    if (jointBones->childBone != IKBONE_NONE)
        ikState->modifiedIKBones |= 1 << jointBones->childBone;
}

// aislop
void IK_Layer_LeftHandOnGun(IKState *ikState)
{
    float layerLerp;
    float weapMat[4][4];
    float leftHandOffsetLocal[3];
    float leftHandRotationLocal[3];
    float leftHandIKMat[4][4];
    float weapTranspose[4][4];
    float workMat[4][4];
    float rotMat[4][4];
    float axis[3][3];
    float goalMat[4][4];
    float handMatCopy[4][4];
    float rightHandTranspose[4][4];

    layerLerp = IKImport_GetLayerLerp(ikState, IKLAYER_LEFT_HAND_ON_GUN, 0.15f);
    ikState->leftHandLastProximityLerp = 0.0f;
    if (layerLerp > 0.0
        && ikState->ikBoneToObjBone[10] != 161
        && ikState->ikBoneToObjBone[22] != 161
        && ikState->bJointVarsValid
        && IK_IsCalcBone(ikState, 10))
    {
        // ---------------------------------------------------------------
        // Step 1: Compute weapMat from IKBONE_RWEAPBONE (bone 22)
        // ---------------------------------------------------------------
        ikCalcBoneModelMatrix(ikState, 22, (*ikState->matArrayPostIK)[0], weapMat);

        // ---------------------------------------------------------------
        // Step 2: Static offset/rotation initialization (one-time)
        // ---------------------------------------------------------------
        static bool bGetMat = true;
        if (bGetMat)
        {
            leftHandOffsetStatic[0] = 0.0f;
            leftHandOffsetStatic[1] = 0.0f;
            leftHandOffsetStatic[2] = 0.0f;
            bGetMat = 0;
        }

        // ---------------------------------------------------------------
        // Step 3: Get left hand offset & rotation
        //   Either from dev gui vars or from ikState (with time-based
        //   lerp during weapon transitions)
        // ---------------------------------------------------------------
        static bool bHackUseDevGuiVars;
        if (IKImport_GetVar_IK_Hand_Tuning() || bHackUseDevGuiVars)
        {
            IKImport_GetVar_IK_Hand_Offset_Vec(leftHandOffsetStatic);
            IKImport_GetVar_IK_Hand_Rotation_Vec(leftHandRotationStatic);
            leftHandOffsetLocal[0] = leftHandOffsetStatic[0];
            leftHandOffsetLocal[1] = leftHandOffsetStatic[1];
            leftHandOffsetLocal[2] = leftHandOffsetStatic[2];
            leftHandRotationLocal[0] = leftHandRotationStatic[0];
            leftHandRotationLocal[1] = leftHandRotationStatic[1];
            leftHandRotationLocal[2] = leftHandRotationStatic[2];
        }
        else
        {
            int leftHandLerpTime = 100;
            if (ikState->leftHandChangeTime <= ikState->timeMS - 100)
            {
                leftHandOffsetLocal[0] = ikState->leftHandOffset[0];
                leftHandOffsetLocal[1] = ikState->leftHandOffset[1];
                leftHandOffsetLocal[2] = ikState->leftHandOffset[2];
                leftHandRotationLocal[0] = ikState->leftHandRotation[0];
                leftHandRotationLocal[1] = ikState->leftHandRotation[1];
                leftHandRotationLocal[2] = ikState->leftHandRotation[2];
            }
            else
            {
                float lerp = (float)(ikState->timeMS - ikState->leftHandChangeTime) / 100.0;
                float clamped;
                if ((float)(lerp - 1.0) < 0.0)
                    clamped = lerp;
                else
                    clamped = 1.0f;
                float fraction;
                if ((float)(0.0 - lerp) < 0.0)
                    fraction = clamped;
                else
                    fraction = 0.0f;
                Vec3Lerp(ikState->leftHandOffsetPrev, ikState->leftHandOffset, fraction, leftHandOffsetLocal);
                Vec3Lerp(ikState->leftHandRotationPrev, ikState->leftHandRotation, fraction, leftHandRotationLocal);
            }
            leftHandOffsetStatic[0] = leftHandOffsetLocal[0];
            leftHandOffsetStatic[1] = leftHandOffsetLocal[1];
            leftHandOffsetStatic[2] = leftHandOffsetLocal[2];
            leftHandRotationStatic[0] = leftHandRotationLocal[0];
            leftHandRotationStatic[1] = leftHandRotationLocal[1];
            leftHandRotationStatic[2] = leftHandRotationLocal[2];
        }

        // ---------------------------------------------------------------
        // Step 4: Compute leftHandIKMat = leftHand * transpose(weapMat)
        //   This stores the left hand position relative to the weapon
        // ---------------------------------------------------------------
        float *leftHandMat = ikState->handMats[0][0];

        // Assert: weapMat != weapTranspose (in != out)
        if (weapMat == (float (*)[4])weapTranspose
            && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.h", 165, 0, "%s", "in != out"))
        {
            __debugbreak();
        }

        // Build transpose of weapMat's 3x3 rotation
        weapTranspose[0][0] = weapMat[0][0];
        weapTranspose[0][1] = weapMat[1][0];
        weapTranspose[0][2] = weapMat[2][0];
        weapTranspose[0][3] = 0.0f;
        weapTranspose[1][0] = weapMat[0][1];
        weapTranspose[1][1] = weapMat[1][1];
        weapTranspose[1][2] = weapMat[2][1];
        weapTranspose[1][3] = 0.0f;
        weapTranspose[2][0] = weapMat[0][2];
        weapTranspose[2][1] = weapMat[1][2];
        weapTranspose[2][2] = weapMat[2][2];
        weapTranspose[2][3] = 0.0f;

        // leftHandIKMat 3x3 = leftHand * transpose(weapMat)
        leftHandIKMat[0][0] = (float)((float)(leftHandMat[0] * weapMat[0][0]) + (float)(leftHandMat[1] * weapMat[0][1]))
            + (float)(leftHandMat[2] * weapMat[0][2]);
        leftHandIKMat[0][1] = (float)((float)(leftHandMat[0] * weapMat[1][0]) + (float)(leftHandMat[1] * weapMat[1][1]))
            + (float)(leftHandMat[2] * weapMat[1][2]);
        leftHandIKMat[0][2] = (float)((float)(leftHandMat[0] * weapMat[2][0]) + (float)(leftHandMat[1] * weapMat[2][1]))
            + (float)(leftHandMat[2] * weapMat[2][2]);
        leftHandIKMat[0][3] = 0.0f;
        leftHandIKMat[1][0] = (float)((float)(leftHandMat[4] * weapMat[0][0]) + (float)(leftHandMat[5] * weapMat[0][1]))
            + (float)(leftHandMat[6] * weapMat[0][2]);
        leftHandIKMat[1][1] = (float)((float)(leftHandMat[4] * weapMat[1][0]) + (float)(leftHandMat[5] * weapMat[1][1]))
            + (float)(leftHandMat[6] * weapMat[1][2]);
        leftHandIKMat[1][2] = (float)((float)(leftHandMat[4] * weapMat[2][0]) + (float)(leftHandMat[5] * weapMat[2][1]))
            + (float)(leftHandMat[6] * weapMat[2][2]);
        leftHandIKMat[1][3] = 0.0f;
        leftHandIKMat[2][0] = (float)((float)(leftHandMat[8] * weapMat[0][0]) + (float)(leftHandMat[9] * weapMat[0][1]))
            + (float)(leftHandMat[10] * weapMat[0][2]);
        leftHandIKMat[2][1] = (float)((float)(leftHandMat[8] * weapMat[1][0]) + (float)(leftHandMat[9] * weapMat[1][1]))
            + (float)(leftHandMat[10] * weapMat[1][2]);
        leftHandIKMat[2][2] = (float)((float)(leftHandMat[8] * weapMat[2][0]) + (float)(leftHandMat[9] * weapMat[2][1]))
            + (float)(leftHandMat[10] * weapMat[2][2]);
        leftHandIKMat[2][3] = 0.0f;

        // Translation: transform (leftHandPos - weapPos) by weapTranspose
        float relPos[3];
        relPos[0] = leftHandMat[12] - weapMat[3][0];
        relPos[1] = leftHandMat[13] - weapMat[3][1];
        relPos[2] = leftHandMat[14] - weapMat[3][2];
        ikMatrixTransformVector34(relPos, (const float (*)[4])weapTranspose, leftHandIKMat[3]);
        leftHandIKMat[3][3] = 1.0f;

        // ---------------------------------------------------------------
        // Step 5: Set up grip position constants, compute distance-based
        //   proximity leftLerp
        // ---------------------------------------------------------------
        float minRangeBase = 3.0f;
        float maxRangeBase = 8.0f;
        float rangeShotgunAdd = 5.0f;
        float gripOffsetFromHandBone = 2.3f;
        float gripPosCenter[3];
        gripPosCenter[0] = -4.0f;
        gripPosCenter[1] = 0.1f;
        gripPosCenter[2] = -3.4f;
        float gripPosCenterShoulder[3];
        gripPosCenterShoulder[0] = -18.0f;
        gripPosCenterShoulder[1] = 1.6f;
        gripPosCenterShoulder[2] = 0.0f;
        float gripPosCenterEquipment[3];
        gripPosCenterEquipment[0] = 1.0f;
        gripPosCenterEquipment[1] = 7.0f;
        gripPosCenterEquipment[2] = 0.4f;

        float gripPos[3];
        gripPos[0] = (float)(2.3 * leftHandIKMat[0][0]) + leftHandIKMat[3][0];
        gripPos[1] = (float)(2.3 * leftHandIKMat[0][1]) + leftHandIKMat[3][1];
        gripPos[2] = (float)(2.3 * leftHandIKMat[0][2]) + leftHandIKMat[3][2];

        float minRange = IKImport_GetLayerLerp(ikState, IKLAYER_SHOTGUN_ACTIVE) * 5.0 + 3.0;
        float maxRange = IKImport_GetLayerLerp(ikState, IKLAYER_SHOTGUN_ACTIVE) * 5.0 + 8.0;

        float gunPlaneOffset;
        if (ikState->weaponOffsetType == IKWEAPOFS_SHOULDER)
        {
            gunPlaneOffset = Vec3Distance(gripPos, gripPosCenterShoulder);
        }
        else if (ikState->weaponOffsetType == IKWEAPOFS_GRENADE)
        {
            gunPlaneOffset = Vec3Distance(gripPos, gripPosCenterEquipment);
        }
        else
        {
            gunPlaneOffset = Vec3Distance(gripPos, gripPosCenter);
        }

        float leftLerp;
        if (minRange <= gunPlaneOffset)
        {
            if (maxRange <= gunPlaneOffset)
                leftLerp = 0.0f;
            else
                leftLerp = 1.0 - (float)((float)(gunPlaneOffset - minRange) / (float)(maxRange - minRange));
        }
        else
        {
            leftLerp = 1.0f;
        }
        if (!ikSystem.inViewer)
            layerLerp = layerLerp * leftLerp;
        ikState->leftHandLastProximityLerp = leftLerp;

        if (layerLerp >= 0.0099999998)
        {
            // ---------------------------------------------------------------
            // Step 6: Apply offset to leftHandIKMat translation
            // ---------------------------------------------------------------
            leftHandIKMat[3][0] = leftHandIKMat[3][0] + leftHandOffsetLocal[0];
            leftHandIKMat[3][1] = leftHandIKMat[3][1] + leftHandOffsetLocal[1];
            leftHandIKMat[3][2] = leftHandIKMat[3][2] + leftHandOffsetLocal[2];

            // ---------------------------------------------------------------
            // Step 7: Build rotMat from leftHandRotationLocal, then
            //   in-place leftHandIKMat[rot] *= rotMat (3x3 multiply
            //   using workMat copy)
            // ---------------------------------------------------------------
            AnglesToAxis(leftHandRotationLocal, axis);
            ikMatrixSet44(rotMat, vec3_origin, axis, 1.0);

            // Copy leftHandIKMat into workMat before in-place multiply
            for (int r = 0; r < 4; r++)
                for (int c = 0; c < 4; c++)
                    workMat[r][c] = leftHandIKMat[r][c];

            // In-place 3x3 multiply: leftHandIKMat[rot] = leftHandIKMat[rot] * rotMat
            // Note: uses workMat for columns already overwritten, preserving
            //   original evaluation order for binary-matching float casts
            leftHandIKMat[0][0] = (float)((float)(leftHandIKMat[0][0] * rotMat[0][0])
                + (float)(leftHandIKMat[0][1] * rotMat[1][0]))
                + (float)(leftHandIKMat[0][2] * rotMat[2][0]);
            leftHandIKMat[0][1] = (float)((float)(workMat[0][0] * rotMat[0][1]) + (float)(leftHandIKMat[0][1] * rotMat[1][1]))
                + (float)(leftHandIKMat[0][2] * rotMat[2][1]);
            leftHandIKMat[0][2] = (float)((float)(workMat[0][0] * rotMat[0][2]) + (float)(workMat[0][1] * rotMat[1][2]))
                + (float)(leftHandIKMat[0][2] * rotMat[2][2]);
            leftHandIKMat[0][3] = 0.0f;
            leftHandIKMat[1][0] = (float)((float)(leftHandIKMat[1][0] * rotMat[0][0])
                + (float)(leftHandIKMat[1][1] * rotMat[1][0]))
                + (float)(leftHandIKMat[1][2] * rotMat[2][0]);
            leftHandIKMat[1][1] = (float)((float)(workMat[1][0] * rotMat[0][1]) + (float)(leftHandIKMat[1][1] * rotMat[1][1]))
                + (float)(leftHandIKMat[1][2] * rotMat[2][1]);
            leftHandIKMat[1][2] = (float)((float)(workMat[1][0] * rotMat[0][2]) + (float)(workMat[1][1] * rotMat[1][2]))
                + (float)(leftHandIKMat[1][2] * rotMat[2][2]);
            leftHandIKMat[1][3] = 0.0f;
            leftHandIKMat[2][0] = (float)((float)(leftHandIKMat[2][0] * rotMat[0][0])
                + (float)(leftHandIKMat[2][1] * rotMat[1][0]))
                + (float)(leftHandIKMat[2][2] * rotMat[2][0]);
            leftHandIKMat[2][1] = (float)((float)(workMat[2][0] * rotMat[0][1]) + (float)(leftHandIKMat[2][1] * rotMat[1][1]))
                + (float)(leftHandIKMat[2][2] * rotMat[2][1]);
            leftHandIKMat[2][2] = (float)((float)(workMat[2][0] * rotMat[0][2]) + (float)(workMat[2][1] * rotMat[1][2]))
                + (float)(leftHandIKMat[2][2] * rotMat[2][2]);
            leftHandIKMat[2][3] = 0.0f;

            // ---------------------------------------------------------------
            // Step 8: Compute final hand matrix
            //   If layerLerp == 1.0: handMats[0] = leftHandIKMat * weapMat
            //   Else: goalMat = leftHandIKMat * weapMat, then lerp
            // ---------------------------------------------------------------
            if (layerLerp == 1.0)
            {
                // handMats[0] = leftHandIKMat * weapMat  (full 4x4 multiply)
                float *handMatOut = ikState->handMats[0][0];
                for (int r = 0; r < 4; r++)
                {
                    for (int c = 0; c < 4; c++)
                    {
                        handMatOut[r * 4 + c] = (float)((float)((float)(leftHandIKMat[r][0] * weapMat[0][c])
                            + (float)(leftHandIKMat[r][1] * weapMat[1][c]))
                            + (float)(leftHandIKMat[r][2] * weapMat[2][c]))
                            + (float)(leftHandIKMat[r][3] * weapMat[3][c]);
                    }
                }
            }
            else
            {
                // Save current handMats[0] before overwriting
                for (int r = 0; r < 4; r++)
                    for (int c = 0; c < 4; c++)
                        handMatCopy[r][c] = ikState->handMats[0][r][c];

                // goalMat = leftHandIKMat * weapMat  (full 4x4 multiply)
                for (int r = 0; r < 4; r++)
                {
                    for (int c = 0; c < 4; c++)
                    {
                        goalMat[r][c] = (float)((float)((float)(leftHandIKMat[r][0] * weapMat[0][c])
                            + (float)(leftHandIKMat[r][1] * weapMat[1][c]))
                            + (float)(leftHandIKMat[r][2] * weapMat[2][c]))
                            + (float)(leftHandIKMat[r][3] * weapMat[3][c]);
                    }
                }

                ikMatrixLerp44(handMatCopy, goalMat, layerLerp, ikState->handMats[0]);
            }

            // ---------------------------------------------------------------
            // Step 9: Recompute leftHandFromRightHandMat = leftHand * transpose(rightHand)
            // ---------------------------------------------------------------
            float *leftHandFromRightHandOut = ikState->leftHandFromRightHandMat[0];
            float *rightHandMat = ikState->handMats[1][0];
            float *leftHandResult = ikState->handMats[0][0];

            // Assert: rightHandMat != rightHandTranspose (in != out)
            if ((float *)ikState->handMats[1] == (float *)rightHandTranspose
                && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.h", 165, 0, "%s", "in != out"))
            {
                __debugbreak();
            }

            // Build transpose of rightHand's 3x3 rotation
            rightHandTranspose[0][0] = rightHandMat[0];
            rightHandTranspose[0][1] = rightHandMat[4];
            rightHandTranspose[0][2] = rightHandMat[8];
            rightHandTranspose[0][3] = 0.0f;
            rightHandTranspose[1][0] = rightHandMat[1];
            rightHandTranspose[1][1] = rightHandMat[5];
            rightHandTranspose[1][2] = rightHandMat[9];
            rightHandTranspose[1][3] = 0.0f;
            rightHandTranspose[2][0] = rightHandMat[2];
            rightHandTranspose[2][1] = rightHandMat[6];
            rightHandTranspose[2][2] = rightHandMat[10];
            rightHandTranspose[2][3] = 0.0f;

            // leftHandFromRightHand 3x3 = leftHand * transpose(rightHand)
            leftHandFromRightHandOut[0]  = (float)((float)(leftHandResult[0] * rightHandTranspose[0][0]) + (float)(leftHandResult[1] * rightHandTranspose[1][0])) + (float)(leftHandResult[2] * rightHandTranspose[2][0]);
            leftHandFromRightHandOut[1]  = (float)((float)(leftHandResult[0] * rightHandTranspose[0][1]) + (float)(leftHandResult[1] * rightHandTranspose[1][1])) + (float)(leftHandResult[2] * rightHandTranspose[2][1]);
            leftHandFromRightHandOut[2]  = (float)((float)(leftHandResult[0] * rightHandTranspose[0][2]) + (float)(leftHandResult[1] * rightHandTranspose[1][2])) + (float)(leftHandResult[2] * rightHandTranspose[2][2]);
            leftHandFromRightHandOut[3]  = 0.0f;
            leftHandFromRightHandOut[4]  = (float)((float)(leftHandResult[4] * rightHandTranspose[0][0]) + (float)(leftHandResult[5] * rightHandTranspose[1][0])) + (float)(leftHandResult[6] * rightHandTranspose[2][0]);
            leftHandFromRightHandOut[5]  = (float)((float)(leftHandResult[4] * rightHandTranspose[0][1]) + (float)(leftHandResult[5] * rightHandTranspose[1][1])) + (float)(leftHandResult[6] * rightHandTranspose[2][1]);
            leftHandFromRightHandOut[6]  = (float)((float)(leftHandResult[4] * rightHandTranspose[0][2]) + (float)(leftHandResult[5] * rightHandTranspose[1][2])) + (float)(leftHandResult[6] * rightHandTranspose[2][2]);
            leftHandFromRightHandOut[7]  = 0.0f;
            leftHandFromRightHandOut[8]  = (float)((float)(leftHandResult[8] * rightHandTranspose[0][0]) + (float)(leftHandResult[9] * rightHandTranspose[1][0])) + (float)(leftHandResult[10] * rightHandTranspose[2][0]);
            leftHandFromRightHandOut[9]  = (float)((float)(leftHandResult[8] * rightHandTranspose[0][1]) + (float)(leftHandResult[9] * rightHandTranspose[1][1])) + (float)(leftHandResult[10] * rightHandTranspose[2][1]);
            leftHandFromRightHandOut[10] = (float)((float)(leftHandResult[8] * rightHandTranspose[0][2]) + (float)(leftHandResult[9] * rightHandTranspose[1][2])) + (float)(leftHandResult[10] * rightHandTranspose[2][2]);
            leftHandFromRightHandOut[11] = 0.0f;

            // Translation: transform (leftHandPos - rightHandPos) by rightHandTranspose
            float relPos2[3];
            relPos2[0] = leftHandResult[12] - rightHandMat[12];
            relPos2[1] = leftHandResult[13] - rightHandMat[13];
            relPos2[2] = leftHandResult[14] - rightHandMat[14];
            ikMatrixTransformVector34(relPos2, (const float (*)[4])rightHandTranspose, leftHandFromRightHandOut + 12);
            leftHandFromRightHandOut[15] = 1.0f;

            // ---------------------------------------------------------------
            // Step 10: Apply wrist twist rotation
            // ---------------------------------------------------------------
            float twistRot[3];
            twistRot[0] = 0.0f;
            twistRot[1] = 0.0f;
            twistRot[2] = (float)(layerLerp * -0.89999998) * leftHandRotationLocal[0];
            ikRotateBone(ikState, IKBONE_LWRIST_TWIST, twistRot, 1);
        }
    }
}

void __cdecl IK_Layer_PlayerControllers(IKState *ikState)
{
    double v1; // st7
    float v2; // [esp+4h] [ebp-74h]
    float v3; // [esp+8h] [ebp-70h]
    float v4; // [esp+Ch] [ebp-6Ch]
    float v5; // [esp+10h] [ebp-68h]
    float v6; // [esp+14h] [ebp-64h]
    float v7; // [esp+1Ch] [ebp-5Ch]
    float v8; // [esp+20h] [ebp-58h]
    float rot_s[3]; // [esp+24h] [ebp-54h] BYREF
    int j; // [esp+30h] [ebp-48h]
    int i; // [esp+34h] [ebp-44h]
    float pelvisYawFeetScaleLeft[2]; // [esp+38h] [ebp-40h]
    float rot[3]; // [esp+40h] [ebp-38h] BYREF
    int ikLayerName; // [esp+4Ch] [ebp-2Ch]
    float pelvisYawFeetScaleRight[2]; // [esp+50h] [ebp-28h]
    float layerLerp; // [esp+58h] [ebp-20h]
    float pitchScalar; // [esp+5Ch] [ebp-1Ch]
    IKBoneNames ikControllerBones[6]; // [esp+60h] [ebp-18h]
    int savedregs; // [esp+78h] [ebp+0h] BYREF

    ikLayerName = 18;
    layerLerp = IKImport_GetLayerLerp(ikState, IKLAYER_PLAYER_CONTROLLERS, 0.0f);
    if ( layerLerp > 0.0 )
    {
        ikControllerBones[0] = IKBONE_SPINE;
        ikControllerBones[1] = IKBONE_SPINE1;
        ikControllerBones[2] = IKBONE_SPINE2;
        ikControllerBones[3] = IKBONE_NECK;
        ikControllerBones[4] = IKBONE_HEAD;
        ikControllerBones[5] = IKBONE_PELVIS;
        if ( ikState->hasControllers )
        {
            v8 = IKImport_GetLayerLerp(ikState, IKLAYER_PRONE_ACTIVE);
            pitchScalar = (float)((float)(1.0 - v8) * pitchScalarDefault) + (float)(pitchScalarProne * v8);
            memset(rot, 0, sizeof(rot));
            for ( i = 0; i < 6; ++i )
            {
                rot[0] = ikState->controllers[i][0] * pitchScalar;
                if ( IKImport_GetLayerLerp(ikState, IKLAYER_PRONE_SLOW_FADE_ACTIVE) > 0.0 )
                {
                    v1 = IKImport_GetLayerLerp(ikState, IKLAYER_PRONE_SLOW_FADE_ACTIVE);
                    v7 = v1 + v1;
                    if ( (float)(v7 - 1.0) < 0.0 )
                        v2 = v1 + v1;
                    else
                        v2 = 1.0f;
                    rot[0] = (float)(1.0 - v2) * rot[0];
                }
                ikRotateBone(ikState, ikControllerBones[i], rot, 0);
            }
            for ( j = 0; j < 6; ++j )
            {
                rot[0] = ikState->controllers[j][0];
                rot[1] = ikState->controllers[j][1];
                rot[2] = ikState->controllers[j][2];
                rot[0] = 0.0f;
                if ( ikControllerBones[j] == IKBONE_PELVIS && IKImport_GetLayerLerp(ikState, IKLAYER_CROUCH_ACTIVE) > 0.0 )
                {
                    v6 = IKImport_GetLayerLerp(ikState, IKLAYER_CROUCH_ACTIVE);
                    rot_s[0] = v6 * rot[0];
                    rot_s[1] = v6 * rot[1];
                    rot_s[2] = v6 * rot[2];
                    ikRotateBone(ikState, IKBONE_SPINE, rot_s, 0);
                    if ( IKImport_GetLayerLerp(ikState, IKLAYER_CROUCH_ACTIVE) < 1.0 )
                    {
                        v5 = 1.0 - IKImport_GetLayerLerp(ikState, IKLAYER_CROUCH_ACTIVE);
                        rot_s[0] = v5 * rot[0];
                        rot_s[1] = v5 * rot[1];
                        rot_s[2] = v5 * rot[2];
                        ikRotateBone(ikState, IKBONE_PELVIS, rot_s, 0);
                    }
                }
                else
                {
                    ikRotateBone(ikState, ikControllerBones[j], rot, 0);
                }
            }
            pelvisYawFeetScaleLeft[0] = 0.3f;
            pelvisYawFeetScaleLeft[1] = 0.3f;
            pelvisYawFeetScaleRight[0] = 0.6f;
            pelvisYawFeetScaleRight[1] = 0.6f;
            memset(rot, 0, sizeof(rot));
            v4 = IKImport_GetLayerLerp(ikState, IKLAYER_CROUCH_ACTIVE);
            rot[1] = (float)((float)((float)(1.0 - v4) * pelvisYawFeetScaleLeft[0]) + (float)(pelvisYawFeetScaleLeft[1] * v4))
                         * ikState->controllers[5][1];
            ikRotateBone(ikState, IKBONE_LFOOT, rot, 0);
            v3 = IKImport_GetLayerLerp(ikState, IKLAYER_CROUCH_ACTIVE);
            rot[1] = (float)((float)((float)(1.0 - v3) * pelvisYawFeetScaleRight[0]) + (float)(pelvisYawFeetScaleRight[1] * v3))
                         * ikState->controllers[5][1];
            ikRotateBone(ikState, IKBONE_RFOOT, rot, 0);
        }
    }
}

// aislopped
void IK_Layer_PlayerPitch(IKState *ikState, bool preControllers)
{
    float layerLerp;
    float playerAngles[3];
    float crouchLerp;
    float xySpeed;
    float movingLerp;
    float notMovingLerp;
    float ofsVec[3];
    float axis[3][3];

    layerLerp = IKImport_GetLayerLerp(ikState, IKLAYER_PLAYER_PITCH, 0.3f);
    if (layerLerp <= 0.0f)
        return;

    // Set up player pitch angle (yaw and roll zeroed out)
    playerAngles[0] = ikState->angles[0];
    playerAngles[1] = 0.0f;
    playerAngles[2] = 0.0f;

    crouchLerp = IKImport_GetLayerLerp(ikState, IKLAYER_CROUCH_ACTIVE);
    xySpeed = Vec2Length(ikState->velocity);

    // Compute movingLerp: clamp(xySpeed / velIdleMax, 0, 1)
    {
        float fraction = xySpeed / velIdleMax;
        float clamped;
        if ((float)(fraction - 1.0f) < 0.0f)
            clamped = fraction;
        else
            clamped = 1.0f;
        if ((float)(0.0f - fraction) < 0.0f)
            movingLerp = clamped;
        else
            movingLerp = 0.0f;
    }

    // Compute notMovingLerp: max of NOT_MOVING and NOT_MOVING_FAST layer lerps
    {
        float notMoving = IKImport_GetLayerLerp(ikState, IKLAYER_NOT_MOVING);
        float notMovingFast = IKImport_GetLayerLerp(ikState, IKLAYER_NOT_MOVING_FAST);
        if ((float)(notMoving - notMovingFast) < 0.0f)
            notMovingLerp = notMovingFast;
        else
            notMovingLerp = notMoving;
    }

    memset(ofsVec, 0, sizeof(ofsVec));
    AnglesToAxis(playerAngles, axis);

    // ---------------------------------------------------------------
    // Block 3: (!preControllers) Pelvis position offset from pitch
    // ---------------------------------------------------------------
    if (!preControllers)
    {
        float pitchAngle = playerAngles[0];
        float absPitch = (float)fabs(pitchAngle);
        float pitchFraction = absPitch / 90.0f;
        float pitchScale = (float)pow(pitchFraction, 1.3f) * layerLerp;

        // Choose offset vectors based on pitch direction and lerp with crouch
        if (pitchAngle >= 0.0f)
        {
            // Looking down
            float ofsDn[3]      = { -4.5f, 0.0f, -1.7f };
            float ofsDnCrouch[3] = { -6.5f, 0.0f,  8.5f };
            Vec3Lerp(ofsDn, ofsDnCrouch, crouchLerp, ofsVec);
        }
        else
        {
            // Looking up
            float ofsUp[3]       = { 5.1f, 0.0f, -2.5f };
            float ofsUpCrouch[3] = { 5.1f, 0.0f,  4.5f };
            Vec3Lerp(ofsUp, ofsUpCrouch, crouchLerp, ofsVec);

            float minigunScale = 1.0f - IKImport_GetLayerLerp(ikState, IKLAYER_MINIGUN_ACTIVE);
            ofsVec[0] = minigunScale * ofsVec[0];
            ofsVec[1] = minigunScale * ofsVec[1];
            ofsVec[2] = minigunScale * ofsVec[2];
        }

        // Scale Z by notMovingLerp, then scale all by pitchScale
        ofsVec[2] = ofsVec[2] * notMovingLerp;
        ofsVec[0] = pitchScale * ofsVec[0];
        ofsVec[1] = pitchScale * ofsVec[1];
        ofsVec[2] = pitchScale * ofsVec[2];

        // Twist offset from controller yaw
        {
            float ofsTwist[3]       = { 6.0f, 0.0f, -1.0f };
            float ofsTwistCrouch[3] = { 4.0f, 0.0f, -1.0f };
            float ofsTwistVec[3];
            float twistScaleCap = 1.0f;

            float yawFraction = ikState->controller_origin_angles[1] / 90.0f;
            float twistScale = (float)pow((float)fabs(yawFraction), 1.3f);

            if ((float)(1.0f - (float)fabs(yawFraction)) < 0.0f)
                twistScale = twistScaleCap;

            Vec3Lerp(ofsTwist, ofsTwistCrouch, crouchLerp, ofsTwistVec);

            ofsVec[0] = (float)(twistScale * ofsTwistVec[0]) + ofsVec[0];
            ofsVec[1] = (float)(twistScale * ofsTwistVec[1]) + ofsVec[1];
            ofsVec[2] = (float)(twistScale * ofsTwistVec[2]) + ofsVec[2];
        }

        // Apply ofsVec to pelvis translation (matArrayPostIK bone 7 = pelvis row 3)
        {
            float *pelvisTranslation = (*ikState->matArrayPostIK)[7];
            pelvisTranslation[0] = pelvisTranslation[0] + ofsVec[0];
            pelvisTranslation[1] = pelvisTranslation[1] + ofsVec[1];
            pelvisTranslation[2] = pelvisTranslation[2] + ofsVec[2];
        }

        ikState->bHasActiveLayers = 1;
        if (ikState->cacheActive)
        {
            memset(ikState->matArrayCache, 0, 0x5C0u);
            ikState->cacheActive = 0;
        }
        ikState->modifiedIKBones |= 2u;

        // If moving, adjust foot mats
        if (movingLerp > 0.0f)
        {
            float footVec[3];
            footVec[0] = ofsVec[0];
            footVec[1] = ofsVec[1];
            footVec[2] = 0.0f;

            ikState->footMats[0][3][0] = (float)(movingLerp * footVec[0]) + ikState->footMats[0][3][0];
            ikState->footMats[0][3][1] = (float)(movingLerp * footVec[1]) + ikState->footMats[0][3][1];
            ikState->footMats[0][3][2] = (float)(movingLerp * footVec[2]) + ikState->footMats[0][3][2];

            ikState->footMats[1][3][0] = (float)(movingLerp * footVec[0]) + ikState->footMats[1][3][0];
            ikState->footMats[1][3][1] = (float)(movingLerp * footVec[1]) + ikState->footMats[1][3][1];
            ikState->footMats[1][3][2] = (float)(movingLerp * footVec[2]) + ikState->footMats[1][3][2];
        }
    }

    // ---------------------------------------------------------------
    // Block 4: (preControllers) Pitch rotation + weapon offset
    // ---------------------------------------------------------------
    if (preControllers)
    {
        // Pelvis rotation (both scales are 0.0 so this block is always skipped)
        float pelvisPitchScaleUp = 0.0f;
        float pelvisPitchScaleDn = 0.0f;
        float pelvisPitchDir;
        if (playerAngles[0] > 0.0f)
            pelvisPitchDir = pelvisPitchScaleDn;
        else
            pelvisPitchDir = pelvisPitchScaleUp;

        float pelvisPitchScale = (float)(1.0f - movingLerp) * pelvisPitchDir;
        if (pelvisPitchScale > 0.0f)
        {
            float pelvisScale = layerLerp * pelvisPitchScale;
            float rot[3];
            rot[0] = pelvisScale * playerAngles[0];
            rot[1] = pelvisScale * playerAngles[1];
            rot[2] = pelvisScale * playerAngles[2];
            ikRotateBone(ikState, IKBONE_PELVIS, rot, 0);
        }

        // Weapon pitch offset with pow(1.1) scale
        float pitchAngle = playerAngles[0];
        float absPitch = (float)fabs(pitchAngle);
        float weaponPitchScale = (float)pow(absPitch / 90.0f, 1.1f) * layerLerp;

        bool bIsPitchDirectionUp = (pitchAngle <= 0.0f);

        float weapOfs[3];
        IKImport_GetPitchWeaponOffset(ikState, bIsPitchDirectionUp, weapOfs);
        weapOfs[0] = weaponPitchScale * weapOfs[0];
        weapOfs[1] = weaponPitchScale * weapOfs[1];
        weapOfs[2] = weaponPitchScale * weapOfs[2];

        // Add weapon offset to both hand mat translations
        ikState->handMats[0][3][0] = ikState->handMats[0][3][0] + weapOfs[0];
        ikState->handMats[0][3][1] = ikState->handMats[0][3][1] + weapOfs[1];
        ikState->handMats[0][3][2] = ikState->handMats[0][3][2] + weapOfs[2];

        ikState->handMats[1][3][0] = ikState->handMats[1][3][0] + weapOfs[0];
        ikState->handMats[1][3][1] = ikState->handMats[1][3][1] + weapOfs[1];
        ikState->handMats[1][3][2] = ikState->handMats[1][3][2] + weapOfs[2];

        // If looking up (pitch < 0): rotate neck/head and recompute right hand
        if (pitchAngle < 0.0f)
        {
            float headMat[4][4];
            float headTranspose[4][4];
            float rightHandFromHead[4][4];
            float rightHandResult[4][4];

            // Compute head bone world-space matrix
            ikCalcBoneModelMatrix(ikState, 6, (*ikState->matArrayPostIK)[0], headMat);

            float *rightHandMat = ikState->handMats[1][0];

            // Assert: headMat != headTranspose (in != out)
            if (headMat == (float (*)[4])headTranspose
                && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.h", 165, 0, "%s", "in != out"))
            {
                __debugbreak();
            }

            // Build transpose of headMat's 3x3 rotation
            headTranspose[0][0] = headMat[0][0];
            headTranspose[0][1] = headMat[1][0];
            headTranspose[0][2] = headMat[2][0];
            headTranspose[0][3] = 0.0f;
            headTranspose[1][0] = headMat[0][1];
            headTranspose[1][1] = headMat[1][1];
            headTranspose[1][2] = headMat[2][1];
            headTranspose[1][3] = 0.0f;
            headTranspose[2][0] = headMat[0][2];
            headTranspose[2][1] = headMat[1][2];
            headTranspose[2][2] = headMat[2][2];
            headTranspose[2][3] = 0.0f;

            // rightHandFromHead = rightHand * transpose(headMat)  (3x3 rotation part)
            // Row 0: rightHandMat row 0 * headTranspose columns
            rightHandFromHead[0][0] = (float)((float)(rightHandMat[0] * headMat[0][0]) + (float)(rightHandMat[1] * headMat[0][1]))
                + (float)(rightHandMat[2] * headMat[0][2]);
            rightHandFromHead[0][1] = (float)((float)(rightHandMat[0] * headMat[1][0]) + (float)(rightHandMat[1] * headMat[1][1]))
                + (float)(rightHandMat[2] * headMat[1][2]);
            rightHandFromHead[0][2] = (float)((float)(rightHandMat[0] * headMat[2][0]) + (float)(rightHandMat[1] * headMat[2][1]))
                + (float)(rightHandMat[2] * headMat[2][2]);
            rightHandFromHead[0][3] = 0.0f;

            // Row 1
            rightHandFromHead[1][0] = (float)((float)(rightHandMat[4] * headMat[0][0]) + (float)(rightHandMat[5] * headMat[0][1]))
                + (float)(rightHandMat[6] * headMat[0][2]);
            rightHandFromHead[1][1] = (float)((float)(rightHandMat[4] * headMat[1][0]) + (float)(rightHandMat[5] * headMat[1][1]))
                + (float)(rightHandMat[6] * headMat[1][2]);
            rightHandFromHead[1][2] = (float)((float)(rightHandMat[4] * headMat[2][0]) + (float)(rightHandMat[5] * headMat[2][1]))
                + (float)(rightHandMat[6] * headMat[2][2]);
            rightHandFromHead[1][3] = 0.0f;

            // Row 2
            rightHandFromHead[2][0] = (float)((float)(rightHandMat[8] * headMat[0][0]) + (float)(rightHandMat[9] * headMat[0][1]))
                + (float)(rightHandMat[10] * headMat[0][2]);
            rightHandFromHead[2][1] = (float)((float)(rightHandMat[8] * headMat[1][0]) + (float)(rightHandMat[9] * headMat[1][1]))
                + (float)(rightHandMat[10] * headMat[1][2]);
            rightHandFromHead[2][2] = (float)((float)(rightHandMat[8] * headMat[2][0]) + (float)(rightHandMat[9] * headMat[2][1]))
                + (float)(rightHandMat[10] * headMat[2][2]);
            rightHandFromHead[2][3] = 0.0f;

            // Row 3 (translation): transform (rightHandPos - headPos) by headTranspose
            {
                float relPos[3];
                relPos[0] = rightHandMat[12] - headMat[3][0];
                relPos[1] = rightHandMat[13] - headMat[3][1];
                relPos[2] = rightHandMat[14] - headMat[3][2];
                ikMatrixTransformVector34(relPos, (const float (*)[4])headTranspose, rightHandFromHead[3]);
            }
            rightHandFromHead[3][3] = 1.0f;

            // Rotate neck by 70% * 30% of pitch
            {
                float neckScale = (float)(layerLerp * 0.69999999f) * 0.30000001f;
                float rot[3];
                rot[0] = neckScale * playerAngles[0];
                rot[1] = neckScale * playerAngles[1];
                rot[2] = neckScale * playerAngles[2];
                ikRotateBone(ikState, IKBONE_NECK, rot, 0);
            }

            // Rotate head by 30% * 30% of pitch
            {
                float headScale = (float)(layerLerp * 0.30000001f) * 0.30000001f;
                float rot[3];
                rot[0] = headScale * playerAngles[0];
                rot[1] = headScale * playerAngles[1];
                rot[2] = headScale * playerAngles[2];
                ikRotateBone(ikState, IKBONE_HEAD, rot, 0);
            }

            // Recompute headMat after neck/head rotation
            ikCalcBoneModelMatrix(ikState, 6, (*ikState->matArrayPostIK)[0], headMat);

            // rightHandResult = rightHandFromHead * headMat  (full 4x4 multiply)
            for (int r = 0; r < 4; r++)
            {
                for (int c = 0; c < 4; c++)
                {
                    rightHandResult[r][c] =
                        (float)((float)((float)(rightHandFromHead[r][0] * headMat[0][c])
                            + (float)(rightHandFromHead[r][1] * headMat[1][c]))
                            + (float)(rightHandFromHead[r][2] * headMat[2][c]))
                            + (float)(rightHandFromHead[r][3] * headMat[3][c]);
                }
            }

            // Write result back to rightHand mat (ikState->handMats[1])
            for (int r = 0; r < 4; r++)
            {
                rightHandMat[r * 4 + 0] = rightHandResult[r][0];
                rightHandMat[r * 4 + 1] = rightHandResult[r][1];
                rightHandMat[r * 4 + 2] = rightHandResult[r][2];
                rightHandMat[r * 4 + 3] = rightHandResult[r][3];
            }
        }

        // leftHand = leftHandFromRightHandMat * rightHand  (full 4x4 multiply)
        {
            float *leftHandMat = ikState->handMats[0][0];
            float *rightHandMat = ikState->handMats[1][0];
            float leftHandResult[4][4];

            // Read leftHandFromRightHandMat and rightHand into local copies
            float lhFromRh[4][4];
            float rh[4][4];
            for (int r = 0; r < 4; r++)
            {
                for (int c = 0; c < 4; c++)
                {
                    lhFromRh[r][c] = ikState->leftHandFromRightHandMat[r][c];
                    rh[r][c] = rightHandMat[r * 4 + c];
                }
            }

            // Multiply: leftHandResult = lhFromRh * rh
            for (int r = 0; r < 4; r++)
            {
                for (int c = 0; c < 4; c++)
                {
                    leftHandResult[r][c] =
                        (float)((float)((float)(lhFromRh[r][0] * rh[0][c])
                            + (float)(lhFromRh[r][1] * rh[1][c]))
                            + (float)(lhFromRh[r][2] * rh[2][c]))
                            + (float)(lhFromRh[r][3] * rh[3][c]);
                }
            }

            // Write result back to leftHand mat (ikState->handMats[0])
            for (int r = 0; r < 4; r++)
            {
                leftHandMat[r * 4 + 0] = leftHandResult[r][0];
                leftHandMat[r * 4 + 1] = leftHandResult[r][1];
                leftHandMat[r * 4 + 2] = leftHandResult[r][2];
                leftHandMat[r * 4 + 3] = leftHandResult[r][3];
            }
        }
    }

    // ---------------------------------------------------------------
    // Block 5: Apply final ofsVec to both hand mat translations
    // ---------------------------------------------------------------
    ikState->handMats[0][3][0] = ikState->handMats[0][3][0] + ofsVec[0];
    ikState->handMats[0][3][1] = ikState->handMats[0][3][1] + ofsVec[1];
    ikState->handMats[0][3][2] = ikState->handMats[0][3][2] + ofsVec[2];

    ikState->handMats[1][3][0] = ikState->handMats[1][3][0] + ofsVec[0];
    ikState->handMats[1][3][1] = ikState->handMats[1][3][1] + ofsVec[1];
    ikState->handMats[1][3][2] = ikState->handMats[1][3][2] + ofsVec[2];

    // ---------------------------------------------------------------
    // Block 6: Recompute leftHandFromRightHandMat = leftHand * transpose(rightHand)
    // ---------------------------------------------------------------
    {
        float *leftHandFromRightHandOut = ikState->leftHandFromRightHandMat[0];
        float *rightHandMat = ikState->handMats[1][0];
        float *leftHandMat = ikState->handMats[0][0];

        // Assert: rightHandMat != rightHandTranspose (in != out)
        float rightHandTranspose[4][4];
        if ((float *)ikState->handMats[1] == (float *)rightHandTranspose
            && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.h", 165, 0, "%s", "in != out"))
        {
            __debugbreak();
        }

        // Build transpose of rightHand's 3x3 rotation
        rightHandTranspose[0][0] = rightHandMat[0];
        rightHandTranspose[0][1] = rightHandMat[4];
        rightHandTranspose[0][2] = rightHandMat[8];
        rightHandTranspose[0][3] = 0.0f;
        rightHandTranspose[1][0] = rightHandMat[1];
        rightHandTranspose[1][1] = rightHandMat[5];
        rightHandTranspose[1][2] = rightHandMat[9];
        rightHandTranspose[1][3] = 0.0f;
        rightHandTranspose[2][0] = rightHandMat[2];
        rightHandTranspose[2][1] = rightHandMat[6];
        rightHandTranspose[2][2] = rightHandMat[10];
        rightHandTranspose[2][3] = 0.0f;

        // leftHandFromRightHand 3x3 = leftHand * transpose(rightHand)
        leftHandFromRightHandOut[0]  = (float)((float)(leftHandMat[0] * rightHandTranspose[0][0]) + (float)(leftHandMat[1] * rightHandTranspose[1][0])) + (float)(leftHandMat[2] * rightHandTranspose[2][0]);
        leftHandFromRightHandOut[1]  = (float)((float)(leftHandMat[0] * rightHandTranspose[0][1]) + (float)(leftHandMat[1] * rightHandTranspose[1][1])) + (float)(leftHandMat[2] * rightHandTranspose[2][1]);
        leftHandFromRightHandOut[2]  = (float)((float)(leftHandMat[0] * rightHandTranspose[0][2]) + (float)(leftHandMat[1] * rightHandTranspose[1][2])) + (float)(leftHandMat[2] * rightHandTranspose[2][2]);
        leftHandFromRightHandOut[3]  = 0.0f;
        leftHandFromRightHandOut[4]  = (float)((float)(leftHandMat[4] * rightHandTranspose[0][0]) + (float)(leftHandMat[5] * rightHandTranspose[1][0])) + (float)(leftHandMat[6] * rightHandTranspose[2][0]);
        leftHandFromRightHandOut[5]  = (float)((float)(leftHandMat[4] * rightHandTranspose[0][1]) + (float)(leftHandMat[5] * rightHandTranspose[1][1])) + (float)(leftHandMat[6] * rightHandTranspose[2][1]);
        leftHandFromRightHandOut[6]  = (float)((float)(leftHandMat[4] * rightHandTranspose[0][2]) + (float)(leftHandMat[5] * rightHandTranspose[1][2])) + (float)(leftHandMat[6] * rightHandTranspose[2][2]);
        leftHandFromRightHandOut[7]  = 0.0f;
        leftHandFromRightHandOut[8]  = (float)((float)(leftHandMat[8] * rightHandTranspose[0][0]) + (float)(leftHandMat[9] * rightHandTranspose[1][0])) + (float)(leftHandMat[10] * rightHandTranspose[2][0]);
        leftHandFromRightHandOut[9]  = (float)((float)(leftHandMat[8] * rightHandTranspose[0][1]) + (float)(leftHandMat[9] * rightHandTranspose[1][1])) + (float)(leftHandMat[10] * rightHandTranspose[2][1]);
        leftHandFromRightHandOut[10] = (float)((float)(leftHandMat[8] * rightHandTranspose[0][2]) + (float)(leftHandMat[9] * rightHandTranspose[1][2])) + (float)(leftHandMat[10] * rightHandTranspose[2][2]);
        leftHandFromRightHandOut[11] = 0.0f;

        // Translation: transform (leftHandPos - rightHandPos) by rightHandTranspose
        float relPos[3];
        relPos[0] = leftHandMat[12] - rightHandMat[12];
        relPos[1] = leftHandMat[13] - rightHandMat[13];
        relPos[2] = leftHandMat[14] - rightHandMat[14];
        ikMatrixTransformVector34(relPos, (const float (*)[4])rightHandTranspose, leftHandFromRightHandOut + 12);
        leftHandFromRightHandOut[15] = 1.0f;
    }
}

// aislop
void    IK_Layer_PreventHeadClip(IKState *ikState)
{
    float layerLerp = IKImport_GetLayerLerp(ikState, IKLAYER_PREVENT_HEAD_CLIP, 0.3f);
    if (layerLerp <= 0.0f)
        return;

    float headMat[4][4];
    ikCalcBoneModelMatrix(ikState, IKBONE_HEAD, (*ikState->matArrayPostIK)[0], headMat);

    float headClipTraceRadius = 0.0f;
    float headClipTraceDist = 12.0f;
    float headClipPlayerCollisionHullRadius = 14.0f;

    if (Vec2Length(headMat[3]) > headClipPlayerCollisionHullRadius - headClipTraceRadius)
    {
        Vec2Normalize(headMat[3]);
        float scale = headClipPlayerCollisionHullRadius - headClipTraceRadius;
        headMat[3][0] = scale * headMat[3][0];
        headMat[3][1] = scale * headMat[3][1];
    }

    // headMatWorld = headMat * entityLegsXform
    float headMatWorld[4][4];
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            headMatWorld[r][c] = (float)((float)(headMat[r][0] * ikState->entityLegsXform[0][c])
                + (float)(headMat[r][1] * ikState->entityLegsXform[1][c]))
                + (float)((float)(headMat[r][2] * ikState->entityLegsXform[2][c])
                + (float)(headMat[r][3] * ikState->entityLegsXform[3][c]));

    float fwd[3] = { headMatWorld[0][0], headMatWorld[0][1], headMatWorld[0][2] };
    float start[3] = { headMatWorld[3][0], headMatWorld[3][1], headMatWorld[3][2] };
    float end[3];
    end[0] = (float)(headClipTraceDist * fwd[0]) + start[0];
    end[1] = (float)(headClipTraceDist * fwd[1]) + start[1];
    end[2] = (float)(headClipTraceDist * fwd[2]) + start[2];

    float traceHitFraction = 0.0f;
    float traceHitPos[3];
    float traceHitNormal[3];

    if (!IKImport_TraceBox(ikState, start, end, headClipTraceRadius, traceHitPos, traceHitNormal, -1, &traceHitFraction, 0))
        return;

    float headClipMaxPelvisShift = 12.0f;

    // torsoFromLegsMat = entityXform * transpose(entityLegsXform)
    float torsoFromLegsMat[4][4];
    {
        float *legs = ikState->entityLegsXform[0];
        float *torso = ikState->entityXform[0];

        float t[3][4];
        t[0][0] = legs[0];  t[0][1] = legs[4];  t[0][2] = legs[8];  t[0][3] = 0.0f;
        t[1][0] = legs[1];  t[1][1] = legs[5];  t[1][2] = legs[9];  t[1][3] = 0.0f;
        t[2][0] = legs[2];  t[2][1] = legs[6];  t[2][2] = legs[10]; t[2][3] = 0.0f;

        for (int r = 0; r < 3; r++)
        {
            torsoFromLegsMat[r][0] = torso[r*4+0]*t[0][0] + torso[r*4+1]*t[1][0] + torso[r*4+2]*t[2][0];
            torsoFromLegsMat[r][1] = torso[r*4+0]*t[0][1] + torso[r*4+1]*t[1][1] + torso[r*4+2]*t[2][1];
            torsoFromLegsMat[r][2] = torso[r*4+0]*t[0][2] + torso[r*4+1]*t[1][2] + torso[r*4+2]*t[2][2];
            torsoFromLegsMat[r][3] = 0.0f;
        }

        float delta[3];
        delta[0] = torso[12] - legs[12];
        delta[1] = torso[13] - legs[13];
        delta[2] = torso[14] - legs[14];
        ikMatrixTransformVector34(delta, (const float (*)[4])t, torsoFromLegsMat[3]);
        torsoFromLegsMat[3][3] = 1.0f;
    }

    float shift = (float)(-(1.0f - traceHitFraction)) * headClipMaxPelvisShift;
    float shiftX = shift * torsoFromLegsMat[0][0];
    float shiftY = shift * torsoFromLegsMat[0][1];

    float *pelvis = (*ikState->matArrayPostIK)[7];
    pelvis[0] += shiftX;
    pelvis[1] += shiftY;

    ikState->handMats[1][3][0] += shiftX;
    ikState->handMats[1][3][1] += shiftY;

    ikState->handMats[0][3][0] += shiftX;
    ikState->handMats[0][3][1] += shiftY;

    ikState->footMats[1][3][0] += shiftX;
    ikState->footMats[1][3][1] += shiftY;

    ikState->footMats[0][3][0] += shiftX;
    ikState->footMats[0][3][1] += shiftY;

    ikState->bHasActiveLayers = 1;
    if (ikState->cacheActive)
    {
        memset(ikState->matArrayCache, 0, 0x5C0u);
        ikState->cacheActive = 0;
    }
    ikState->modifiedIKBones |= 2u;
}

// aislop
void    IK_ProcessLayers(IKState *ikState)
{
    if (!ikState->bJointVarsValid)
        return;

    if (ikSystem.inViewer)
    {
        ikState->bHasActiveLayers = 1;
        ikState->bHasArmSolverLayer = 1;
    }

    //BG_EvalVehicleName();

    if (ikState->ikBoneToObjBone[IKBONE_LHAND] == 161
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_layers.cpp", 1773, 0, "%s",
            "IK_IsBoneAnimated( ikState, IKBONE_LHAND )"))
    {
        __debugbreak();
    }
    ikCalcBoneModelMatrix(ikState, IKBONE_LHAND, (*ikState->matArrayPostIK)[0], ikState->handMats[0]);

    if (ikState->ikBoneToObjBone[IKBONE_RHAND] == 161
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_layers.cpp", 1776, 0, "%s",
            "IK_IsBoneAnimated( ikState, IKBONE_RHAND )"))
    {
        __debugbreak();
    }
    ikCalcBoneModelMatrix(ikState, IKBONE_RHAND, (*ikState->matArrayPostIK)[0], ikState->handMats[1]);

    // leftHandFromRightHandMat = leftHand * transpose(rightHand)
    {
        float *lh = ikState->handMats[0][0];
        float *rh = ikState->handMats[1][0];
        float *out = ikState->leftHandFromRightHandMat[0];

        float t[3][4];
        t[0][0] = rh[0];  t[0][1] = rh[4];  t[0][2] = rh[8];  t[0][3] = 0.0f;
        t[1][0] = rh[1];  t[1][1] = rh[5];  t[1][2] = rh[9];  t[1][3] = 0.0f;
        t[2][0] = rh[2];  t[2][1] = rh[6];  t[2][2] = rh[10]; t[2][3] = 0.0f;

        for (int r = 0; r < 3; r++)
        {
            out[r*4+0] = lh[r*4+0]*t[0][0] + lh[r*4+1]*t[1][0] + lh[r*4+2]*t[2][0];
            out[r*4+1] = lh[r*4+0]*t[0][1] + lh[r*4+1]*t[1][1] + lh[r*4+2]*t[2][1];
            out[r*4+2] = lh[r*4+0]*t[0][2] + lh[r*4+1]*t[1][2] + lh[r*4+2]*t[2][2];
            out[r*4+3] = 0.0f;
        }

        float delta[3];
        delta[0] = lh[12] - rh[12];
        delta[1] = lh[13] - rh[13];
        delta[2] = lh[14] - rh[14];
        ikMatrixTransformVector34(delta, (const float (*)[4])t, out + 12);
        out[15] = 1.0f;
    }

    if (ikState->ikBoneToObjBone[IKBONE_LFOOT] == 161
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_layers.cpp", 1782, 0, "%s",
            "IK_IsBoneAnimated( ikState, IKBONE_LFOOT )"))
    {
        __debugbreak();
    }
    ikCalcBoneModelMatrix(ikState, IKBONE_LFOOT, (*ikState->matArrayPostIK)[0], ikState->footMats[0]);

    if (ikState->ikBoneToObjBone[IKBONE_RFOOT] == 161
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_layers.cpp", 1785, 0, "%s",
            "IK_IsBoneAnimated( ikState, IKBONE_RFOOT )"))
    {
        __debugbreak();
    }
    ikCalcBoneModelMatrix(ikState, IKBONE_RFOOT, (*ikState->matArrayPostIK)[0], ikState->footMats[1]);

    IK_Layer_LeftHandOnGun(ikState);

    if (IKImport_IsClient(ikState))
    {
        IK_Layer_PlayerPitch(ikState, true);

        // Save rightHand relative to spine before controllers modify spine
        float spineMat[4][4];
        ikCalcBoneModelMatrix(ikState, IKBONE_SPINE1, (*ikState->matArrayPostIK)[0], spineMat);

        float rhFromSpine[4][4];
        {
            float *rh = ikState->handMats[1][0];
            float t[3][4];
            t[0][0] = spineMat[0][0]; t[0][1] = spineMat[1][0]; t[0][2] = spineMat[2][0]; t[0][3] = 0.0f;
            t[1][0] = spineMat[0][1]; t[1][1] = spineMat[1][1]; t[1][2] = spineMat[2][1]; t[1][3] = 0.0f;
            t[2][0] = spineMat[0][2]; t[2][1] = spineMat[1][2]; t[2][2] = spineMat[2][2]; t[2][3] = 0.0f;

            for (int r = 0; r < 3; r++)
            {
                rhFromSpine[r][0] = rh[r*4+0]*t[0][0] + rh[r*4+1]*t[1][0] + rh[r*4+2]*t[2][0];
                rhFromSpine[r][1] = rh[r*4+0]*t[0][1] + rh[r*4+1]*t[1][1] + rh[r*4+2]*t[2][1];
                rhFromSpine[r][2] = rh[r*4+0]*t[0][2] + rh[r*4+1]*t[1][2] + rh[r*4+2]*t[2][2];
                rhFromSpine[r][3] = 0.0f;
            }

            float delta[3];
            delta[0] = rh[12] - spineMat[3][0];
            delta[1] = rh[13] - spineMat[3][1];
            delta[2] = rh[14] - spineMat[3][2];
            ikMatrixTransformVector34(delta, (const float (*)[4])t, rhFromSpine[3]);
            rhFromSpine[3][3] = 1.0f;
        }

        IK_Layer_PlayerControllers(ikState);

        // Recompose rightHand = rhFromSpine * newSpineMat
        ikCalcBoneModelMatrix(ikState, IKBONE_SPINE1, (*ikState->matArrayPostIK)[0], spineMat);
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++)
                ikState->handMats[1][r][c] = (float)((float)(rhFromSpine[r][0] * spineMat[0][c])
                    + (float)(rhFromSpine[r][1] * spineMat[1][c]))
                    + (float)((float)(rhFromSpine[r][2] * spineMat[2][c])
                    + (float)(rhFromSpine[r][3] * spineMat[3][c]));

        // leftHand = leftHandFromRightHandMat * rightHand
        float lhFromRh[4][4];
        memcpy(lhFromRh, ikState->leftHandFromRightHandMat, sizeof(lhFromRh));
        float rhMat[4][4];
        memcpy(rhMat, ikState->handMats[1], sizeof(rhMat));
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++)
                ikState->handMats[0][r][c] = (float)((float)(lhFromRh[r][0] * rhMat[0][c])
                    + (float)(lhFromRh[r][1] * rhMat[1][c]))
                    + (float)((float)(lhFromRh[r][2] * rhMat[2][c])
                    + (float)(lhFromRh[r][3] * rhMat[3][c]));
    }
    else
    {
        IK_Layer_PlayerControllers(ikState);
    }

    if (IKImport_IsClient(ikState))
    {
        IK_Layer_PlayerPitch(ikState, false);
        IK_Layer_PreventHeadClip(ikState);
    }

    IK_Layer_TerrainMapping(ikState);

    if (ikState->bHasActiveLayers)
    {
        if (ikState->bHasLegSolverLayer)
        {
            IK_Layer_ApplyFootIK(ikState, &ikState->localIkSystem->joints[2], &ikState->jointVars[2], ikState->footMats[0]);
            IK_Layer_ApplyFootIK(ikState, &ikState->localIkSystem->joints[3], &ikState->jointVars[3], ikState->footMats[1]);
        }
        if (ikState->bHasArmSolverLayer)
        {
            IK_Layer_ApplyHandIK(ikState, ikState->localIkSystem->joints, ikState->jointVars, ikState->handMats[0]);
            IK_Layer_ApplyHandIK(ikState, &ikState->localIkSystem->joints[1], &ikState->jointVars[1], ikState->handMats[1]);
        }
    }
}

