#include "flame_class_stream.h"
#include "flame_class_drips.h"
#include <gfx_d3d/r_scene.h>
#include "flame_cull.h"
#include "flame_class_chunk.h"
#include <qcommon/common.h>
#include <cgame/cg_scr_main.h>
#include <demo/demo_playback.h>
#include <EffectsCore/fx_update.h>
#include <universal/CurveManager.h>
#include <EffectsCore/fx_beam.h>

flameStream_s *flameStreamsFree;
flameStream_s *flameStreamsUsed;

flameStream_s flameStreams[64]; 
flameRenderList_s flameStreamRenderList[64];

flameStream_s sv_flameStreams[64];
flameStream_s *sv_flameStreamsFree;
flameStream_s *sv_flameStreamsUsed;

int g_StreamCount;
int g_StreamCountWaterMark;

void __cdecl Flame_Class_Stream_Init()
{
    int i; // [esp+0h] [ebp-4h]

    memset(&flameStreams, 0, 0x2300u);
    Flame_List_Init((flameGeneric_s*)&flameStreams[0], 140, 64);
    flameStreamsFree = (flameStream_s *)&flameStreams;
    flameStreamsUsed = 0;
    for (i = 0; i < 64; ++i)
    {
        flameStreams[i].renderList = &flameStreamRenderList[i];
    }
    memset((void *)&sv_flameStreams[0], 0, sizeof(sv_flameStreams));
    Flame_List_Init(&sv_flameStreams[0].gen, 140, 64);
    sv_flameStreamsFree = sv_flameStreams;
    sv_flameStreamsUsed = 0;
    ++g_StreamCount;
}

flameStream_s *__cdecl Flame_Class_Stream_Alloc(bool is_server)
{
    flameStream_s *freeStream; // [esp+0h] [ebp-4h]

    if ( is_server )
        freeStream = sv_flameStreamsFree;
    else
        freeStream = flameStreamsFree;
    if ( freeStream )
    {
        freeStream->chunkList = 0;
        if ( !is_server )
            freeStream->renderList->flameRend = 0;
        freeStream->flameVars = 0;
        if ( is_server )
        {
            Flame_List_Move_Global(
                &freeStream->gen,
                (flameGeneric_s **)&sv_flameStreamsFree,
                (flameGeneric_s **)&sv_flameStreamsUsed);
        }
        else
        {
            Flame_List_Move_Global(
                &freeStream->gen,
                (flameGeneric_s **)&flameStreamsFree,
                (flameGeneric_s **)&flameStreamsUsed);
            if ( ++g_StreamCount > g_StreamCountWaterMark )
                g_StreamCountWaterMark = g_StreamCount;
        }
    }
    return freeStream;
}

void __cdecl Flame_Class_Stream_Free(bool is_server, flameStream_s *flameStream)
{
    flameSource_t *source; // [esp+0h] [ebp-4h]
    flameSource_t *sourcea; // [esp+0h] [ebp-4h]

    if ( is_server )
    {
        Flame_Class_Drips_Free_If_Stream(is_server, flameStream);
        source = SV_Flame_Source_Get(flameStream->entityNum);
        if ( source->currentStream == flameStream )
            source->currentStream = 0;
        Flame_List_Move_Global(
            &flameStream->gen,
            (flameGeneric_s **)&sv_flameStreamsUsed,
            (flameGeneric_s **)&sv_flameStreamsFree);
    }
    else
    {
        sourcea = Flame_Source_Get(flameStream->entityNum);
        if ( sourcea->currentStream == flameStream )
            sourcea->currentStream = 0;
        Flame_List_Move_Global(
            &flameStream->gen,
            (flameGeneric_s **)&flameStreamsUsed,
            (flameGeneric_s **)&flameStreamsFree);
        --g_StreamCount;
    }
}

void __cdecl Flame_Class_Stream_Light_Chunks(const flameStream_s *stream)
{
    float color_fade; // [esp+14h] [ebp-50h]
    float radius_flutter; // [esp+18h] [ebp-4Ch]
    float blue_flutter; // [esp+1Ch] [ebp-48h]
    int point_count; // [esp+20h] [ebp-44h]
    float median_point[3]; // [esp+24h] [ebp-40h] BYREF
    float red_flutter; // [esp+30h] [ebp-34h]
    const flameChunk_s *thisChunk; // [esp+34h] [ebp-30h]
    float green_flutter; // [esp+38h] [ebp-2Ch]
    float dummyAxis[3][3]; // [esp+3Ch] [ebp-28h] BYREF
    const flameChunk_s *nextChunk; // [esp+60h] [ebp-4h]

    memset(dummyAxis, 0, sizeof(dummyAxis));
    point_count = 0;
    memset(median_point, 0, sizeof(median_point));
    for ( thisChunk = stream->chunkList; thisChunk; thisChunk = nextChunk )
    {
        nextChunk = (const flameChunk_s *)thisChunk->gen.listLocal.next;
        median_point[0] = median_point[0] + thisChunk->gen.phys.origin[0];
        median_point[1] = median_point[1] + thisChunk->gen.phys.origin[1];
        median_point[2] = median_point[2] + thisChunk->gen.phys.origin[2];
        ++point_count;
    }
    if ( point_count > 0 )
    {
        median_point[0] = median_point[0] / (float)point_count;
        median_point[1] = median_point[1] / (float)point_count;
        median_point[2] = median_point[2] / (float)point_count;
        if ( point_count == 1 || point_count == 2 )
            color_fade = 0.2f;
        else
            color_fade = 1.0f;
        radius_flutter = Flame_CRandom(0) * stream->flameVars->flameVar_streamPrimaryLightRadiusFlutter;
        red_flutter = Flame_CRandom(0) * stream->flameVars->flameVar_streamPrimaryLightFlutterR;
        blue_flutter = Flame_CRandom(0) * stream->flameVars->flameVar_streamPrimaryLightFlutterG;
        green_flutter = Flame_CRandom(0) * stream->flameVars->flameVar_streamPrimaryLightFlutterB;
        R_AddOmniLightToScene(
            median_point,
            dummyAxis,
            (stream->flameVars->flameVar_streamPrimaryLightRadius + radius_flutter),
            (float)(stream->flameVars->flameVar_streamPrimaryLightR + red_flutter) * color_fade,
            (float)(stream->flameVars->flameVar_streamPrimaryLightG + green_flutter) * color_fade,
            (float)(stream->flameVars->flameVar_streamPrimaryLightB + blue_flutter) * color_fade);
    }
}

void __cdecl Flame_Class_Stream_Age(bool is_server)
{
    flameStream_s *nextTrav; // [esp+0h] [ebp-8h]
    flameStream_s *trav; // [esp+4h] [ebp-4h]

    if ( is_server )
        trav = sv_flameStreamsUsed;
    else
        trav = flameStreamsUsed;
    while ( trav )
    {
        nextTrav = (flameStream_s *)trav->gen.listGlobal.next;
        Flame_Cull_Stream_Chunks(is_server, trav);
        if ( !trav->chunkList )
            Flame_Class_Stream_Free(is_server, trav);
        trav = nextTrav;
    }
}

void __cdecl Flame_Class_Stream_Fire_Chunks(
                flameSource_t *source,
                flameWeaponConfig_t *weaponConfig,
                int inTime,
                flameRender_s *flameRend)
{
    double v4; // st7
    double v5; // st7
    float v7; // [esp+28h] [ebp-CCh]
    float fRand; // [esp+58h] [ebp-9Ch]
    int totalTime; // [esp+5Ch] [ebp-98h]
    flameChunk_s *newChunk; // [esp+60h] [ebp-94h]
    bool spawnSmoke; // [esp+67h] [ebp-8Dh]
    int thisTime; // [esp+68h] [ebp-8Ch]
    float diffAngle; // [esp+6Ch] [ebp-88h]
    float diffAngle_4; // [esp+70h] [ebp-84h]
    float diffAngle_4a; // [esp+70h] [ebp-84h]
    float diffAngle_8; // [esp+74h] [ebp-80h]
    float diffAngle_8a; // [esp+74h] [ebp-80h]
    int scaledDuration; // [esp+80h] [ebp-74h]
    flameChunkSpawnVars_t spawnVars; // [esp+84h] [ebp-70h] BYREF
    float velocityAdd[3]; // [esp+CCh] [ebp-28h] BYREF
    flameStream_s *stream; // [esp+D8h] [ebp-1Ch]
    int stepTime; // [esp+DCh] [ebp-18h]
    int lastTime; // [esp+E0h] [ebp-14h]
    flameTable *fTable; // [esp+E4h] [ebp-10h]
    float diffOrigin[3]; // [esp+E8h] [ebp-Ch] BYREF

    newChunk = 0;
    spawnSmoke = 0;
    fTable = weaponConfig->fTable;
    if ( flame_use_dvars && flame_use_dvars->current.enabled )
    {
        fTable = Flame_FindFlameTable(flameVar_editingFlameTable->current.string);
        Flame_DVarsToFlameVars(fTable);
        Com_Printf(16, "FLAMETHROWER TEST ACTIVE\n%s\n", flameVar_editingFlameTable->current.string);
    }
    memset(diffOrigin, 0, sizeof(diffOrigin));
    diffAngle = 0.0f;
    diffAngle_4 = 0.0f;
    diffAngle_8 = 0.0f;
    if ( weaponConfig->bIsFiring == source->bIsFiring && source->currentStream
        || (source->currentStream = Flame_Class_Stream_Alloc(source->is_server_alloc)) != 0 )
    {
        stream = source->currentStream;
        stream->gen.stream = stream;
        *((unsigned int *)&stream->gen + 23) &= 0xFFFFFFF8;
        stream->entityNum = source->entityNum;
        stream->flameVars = fTable;
        if ( !source->is_server_alloc )
            stream->renderList->flameRend = flameRend;
        stream->damage = weaponConfig->damage;
        stream->damageDuration = weaponConfig->damageDuration;
        stream->damageInterval = weaponConfig->damageInterval;
        stream->isKillcamFlame = source->isKillcamFlame;
        stream->firingThroughGeo = source->firingThroughGeo;
        stepTime = (int)(float)((float)((float)(fTable->flameVar_streamChunkStartSize
                                                                                    * fTable->flameVar_streamChunkDistScalar)
                                                                    * 1000.0)
                                                    / fTable->flameVar_streamChunkSpeed);
        if ( !stepTime )
            stepTime = 1;
        scaledDuration = (int)(float)(fTable->flameVar_streamChunkDuration * 1000.0);
        if ( stream->chunkList )
        {
            diffOrigin[0] = weaponConfig->origin[0] - source->origin[0];
            diffOrigin[1] = weaponConfig->origin[1] - source->origin[1];
            diffOrigin[2] = weaponConfig->origin[2] - source->origin[2];
            diffAngle_4a = weaponConfig->angle[1] - source->angle[1];
            diffAngle_8a = weaponConfig->angle[2] - source->angle[2];
            diffAngle = AngleNormalize180(weaponConfig->angle[0] - source->angle[0]);
            diffAngle_4 = AngleNormalize180(diffAngle_4a);
            diffAngle_8 = AngleNormalize180(diffAngle_8a);
            lastTime = source->lastUsedTime;
            thisTime = stepTime + lastTime;
            if ( weaponConfig->entityOrigin[0] == source->entityOrigin[0]
                && weaponConfig->entityOrigin[1] == source->entityOrigin[1]
                && weaponConfig->entityOrigin[2] == source->entityOrigin[2] )
            {
                spawnSmoke = 1;
            }
            memset(velocityAdd, 0, sizeof(velocityAdd));
        }
        else
        {
            lastTime = inTime;
            thisTime = inTime;
            memset(velocityAdd, 0, sizeof(velocityAdd));
        }
        totalTime = inTime - lastTime;
        while ( thisTime <= inTime )
        {
            if ( inTime - thisTime < 25 )
            {
                spawnVars.time = thisTime;
                spawnVars.duration = scaledDuration;
                if ( thisTime >= inTime )
                {
                    spawnVars.origin[0] = weaponConfig->origin[0];
                    spawnVars.origin[1] = weaponConfig->origin[1];
                    spawnVars.origin[2] = weaponConfig->origin[2];
                    spawnVars.angle[0] = weaponConfig->angle[0];
                    spawnVars.angle[1] = weaponConfig->angle[1];
                    spawnVars.angle[2] = weaponConfig->angle[2];
                }
                else
                {
                    spawnVars.origin[0] = (float)((float)(1.0 - (float)((float)(inTime - thisTime) / (float)totalTime))
                                                                            * diffOrigin[0])
                                                            + source->origin[0];
                    spawnVars.origin[1] = (float)((float)(1.0 - (float)((float)(inTime - thisTime) / (float)totalTime))
                                                                            * diffOrigin[1])
                                                            + source->origin[1];
                    spawnVars.origin[2] = (float)((float)(1.0 - (float)((float)(inTime - thisTime) / (float)totalTime))
                                                                            * diffOrigin[2])
                                                            + source->origin[2];
                    v7 = 1.0 - (float)((float)(inTime - thisTime) / (float)totalTime);
                    spawnVars.angle[0] = (float)(v7 * diffAngle) + source->angle[0];
                    spawnVars.angle[1] = (float)(v7 * diffAngle_4) + source->angle[1];
                    spawnVars.angle[2] = (float)(v7 * diffAngle_8) + source->angle[2];
                }
                spawnVars.speed = fTable->flameVar_streamChunkSpeed;
                spawnVars.decel = fTable->flameVar_streamChunkDecel;
                spawnVars.gravityStart = fTable->flameVar_streamChunkGravityStart;
                spawnVars.gravityEnd = fTable->flameVar_streamChunkGravityEnd;
                fRand = Flame_SwayRand(fTable->flameVar_streamSizeRandSinWave, fTable->flameVar_streamSizeRandCosWave, thisTime)
                            * 0.5
                            + 0.5;
                spawnVars.sizeStart = (float)(1.0 - (float)(fRand * fTable->flameVar_streamChunkStartSizeRand))
                                                        * fTable->flameVar_streamChunkStartSize;
                spawnVars.sizeEnd = (float)(1.0 - (float)(fRand * fTable->flameVar_streamChunkEndSizeRand))
                                                    * fTable->flameVar_streamChunkEndSize;
                spawnVars.sizeMax = fTable->flameVar_streamChunkMaxSize;
                spawnVars.sizeRate = (float)(spawnVars.sizeEnd - spawnVars.sizeStart) / (float)scaledDuration;
                newChunk = Flame_Class_Chunk_Spawn(
                                         source->is_server_alloc,
                                         &spawnVars,
                                         (flameGeneric_s **)&stream->chunkList,
                                         velocityAdd,
                                         fTable);
                if ( newChunk )
                {
                    newChunk->gen.stream = stream;
                    if ( spawnSmoke
                        && thisTime - stream->lastSmokeChunkTime > (int)(float)(1000.0 * fTable->flameVar_streamSmokeChunkInterval) )
                    {
                        v4 = Flame_Random(source->is_server_alloc);
                        newChunk->spawnSmokeInterval = (int)((v4 * fTable->flameVar_streamSmokeChunkRandFrac
                                                                                                + fTable->flameVar_streamSmokeChunkMinFrac)
                                                                                             * 1000.0);
                        stream->lastSmokeChunkTime = thisTime;
                    }
                    if ( thisTime - stream->lastDripChunkTime > (int)(float)(1000.0 * fTable->flameVar_streamDripsChunkInterval) )
                    {
                        v5 = Flame_Random(source->is_server_alloc);
                        newChunk->spawnDripsInterval = (int)((v5 * fTable->flameVar_streamDripsChunkRandFrac
                                                                                                + fTable->flameVar_streamDripsChunkMinFrac)
                                                                                             * 1000.0);
                        stream->lastDripChunkTime = thisTime;
                    }
                    Flame_Phys_Update_Item_Stream(&newChunk->gen, inTime);
                }
            }
            lastTime = thisTime;
            thisTime += stepTime;
        }
        Flame_Phys_Update_Items(source->is_server_alloc);
        if ( newChunk )
        {
            source->origin[0] = spawnVars.origin[0];
            source->origin[1] = spawnVars.origin[1];
            source->origin[2] = spawnVars.origin[2];
            source->angle[0] = spawnVars.angle[0];
            source->angle[1] = spawnVars.angle[1];
            source->angle[2] = spawnVars.angle[2];
            source->entityOrigin[0] = weaponConfig->entityOrigin[0];
            source->entityOrigin[1] = weaponConfig->entityOrigin[1];
            source->entityOrigin[2] = weaponConfig->entityOrigin[2];
            source->lastUsedTime = lastTime;
        }
    }
}

int curve;
void __cdecl Flame_Class_Stream_Render_Item(
                int localClientNum,
                const flameStream_s *stream,
                Material *material,
                Material *material2,
                float (*verts)[3],
                bool isFire)
{
    float v6; // xmm0_4
    float v7; // xmm0_4
    float dist; // [esp+38h] [ebp-BCh]
    float lastDist; // [esp+3Ch] [ebp-B8h]
    signed int dobjHandle; // [esp+40h] [ebp-B4h]
    unsigned __int8 boneIndex; // [esp+47h] [ebp-ADh]
    cg_s *LocalClientGlobals; // [esp+48h] [ebp-ACh]
    float minDist; // [esp+4Ch] [ebp-A8h]
    const flameSource_t *source; // [esp+54h] [ebp-A0h]
    int numSegments; // [esp+5Ch] [ebp-98h]
    FxBeam beam; // [esp+64h] [ebp-90h] BYREF
    orientation_t orient; // [esp+B4h] [ebp-40h] BYREF
    int segCount; // [esp+E4h] [ebp-10h]
    const flameChunk_s *chunk; // [esp+E8h] [ebp-Ch]
    int beamLength; // [esp+ECh] [ebp-8h]
    float segDist; // [esp+F0h] [ebp-4h]

    beamLength = 0;
    if ( material )
    {
        if ( material2 )
        {
            source = Flame_Source_Get(stream->entityNum);
            if ( source )
            {
                CG_GetLocalClientGlobals(localClientNum);
                numSegments = isFire
                                        ? (int)stream->flameVars->flameVar_streamFlameNumSegments
                                        : (int)stream->flameVars->flameVar_streamFuelNumSegments;
                if ( source->currentStream == stream )
                {
                    chunk = stream->chunkList;
                    if ( chunk )
                    {
                        if ( isFire )
                        {
                            beam.beginRadius = stream->flameVars->flameVar_streamFlameSizeStart;
                            beam.endRadius = stream->flameVars->flameVar_streamFlameSizeEnd;
                        }
                        else
                        {
                            beam.beginRadius = stream->flameVars->flameVar_streamFuelSizeStart;
                            beam.endRadius = stream->flameVars->flameVar_streamFuelSizeEnd;
                        }
                        dobjHandle = source->firstDobjHandle;
                        boneIndex = source->firstBoneIndex;
                        LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
                        if ( !CG_IsLocalPlayer(source->entityNum)
                            || (LocalClientGlobals->nextSnap->ps.otherFlags & 2) != 0
                            && LocalClientGlobals->renderingThirdPerson == TP_FOR_MODEL
                            || Demo_IsThirdPersonCamera()
                            || Demo_IsMovieCamera() )
                        {
                            dobjHandle = source->thirdDobjHandle;
                            boneIndex = source->thirdBoneIndex;
                        }
                        if ( FX_GetBoneOrientation(localClientNum, dobjHandle, boneIndex, &orient) )
                        {
                            beam.begin[0] = orient.origin[0];
                            beam.begin[1] = orient.origin[1];
                            beam.begin[2] = orient.origin[2];
                            beam.end[0] = orient.origin[0];
                            beam.end[1] = orient.origin[1];
                            beam.end[2] = orient.origin[2];
                        }
                        else
                        {
                            memset(&beam, 0, 24);
                        }
                        curve = cCurveManager::GetFreeCurve();
                        cCurveManager::AddNodeToCurve(curve, beam.begin);
                        
                        if ( isFire )
                            v6 = stream->flameVars->flameVar_streamFlameLength * stream->flameVars->flameVar_streamFlameLength;
                        else
                            v6 = stream->flameVars->flameVar_streamFuelLength * stream->flameVars->flameVar_streamFuelLength;
                        beamLength = (int)v6;
                        minDist = 144.0f;
                        if ( dobjHandle < 32 )
                            minDist = 14400.0f;
                        segCount = 1;
                        do
                        {
                            dist = Vec3DistanceSq(beam.begin, chunk->gen.phys.origin);
                            lastDist = Vec3DistanceSq(beam.end, chunk->gen.phys.origin);
                            if ( lastDist > 1.0
                                && (dist > minDist && (float)beamLength > dist || !chunk->gen.listLocal.next && (float)beamLength > dist) )
                            {
                                beam.end[0] = chunk->gen.phys.origin[0];
                                beam.end[1] = chunk->gen.phys.origin[1];
                                beam.end[2] = chunk->gen.phys.origin[2];
                                cCurveManager::AddNodeToCurve(curve, beam.end);
                                ++segCount;
                            }
                            chunk = (const flameChunk_s *)chunk->gen.listLocal.next;
                        }
                        while ( chunk && segCount < 100 );
                        cCurveManager::SetCurveBSpline(curve);
                        cCurveManager::SortCurve(curve, beam.begin, 1);
                        cCurveManager::BuildCurve(curve);
                        segCount = 0;
                        segDist = 0.0f;
                        while ( segCount < numSegments )
                        {
                            cCurveManager::GetPos(curve, segDist, &(*verts)[3 * segCount]);
                            if ( ((LODWORD((*verts)[3 * segCount]) & 0x7F800000) == 0x7F800000
                                 || (LODWORD((*verts)[3 * segCount + 1]) & 0x7F800000) == 0x7F800000
                                 || (LODWORD((*verts)[3 * segCount + 2]) & 0x7F800000) == 0x7F800000)
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\flame\\flame_class_stream.cpp",
                                            635,
                                            0,
                                            "%s",
                                            "!IS_NAN((verts[segCount])[0]) && !IS_NAN((verts[segCount])[1]) && !IS_NAN((verts[segCount])[2])") )
                            {
                                __debugbreak();
                            }
                            ++segCount;
                            segDist = segDist + (float)(1.0 / (float)numSegments);
                        }
                        beam.beginColor.packed = -1;
                        beam.endColor.packed = -1;
                        beam.material = material;
                        beam.material2 = material2;
                        beam.segmentVerts = (float *)verts;
                        beam.segmentCount = numSegments - 1;
                        beam.perpSegmentCount = 1;
                        beam.wiggleDist = 0.0f;
                        beam.flags = 14;
                        if ( isFire )
                            v7 = 1000.0 * stream->flameVars->flameVar_streamFlameAnimLoopTime;
                        else
                            v7 = 1000.0 * stream->flameVars->flameVar_streamFuelAnimLoopTime;
                        beam.animFrac = (float)(source->lastUsedTime % (int)v7) / (float)(int)v7;
                        FX_Beam_Add(&beam);
                        cCurveManager::FreeCurve(curve);
                    }
                }
            }
        }
    }
}

void __cdecl Flame_Class_Stream_Render_All(int localClientNum)
{
    const flameStream_s *stream; // [esp+0h] [ebp-4h]

    for ( stream = flameStreamsUsed; stream; stream = (const flameStream_s *)stream->gen.listGlobal.next )
    {
        if ( stream->flameVars->flameVar_streamFuelLength > 0.1 )
            Flame_Class_Stream_Render_Item(
                localClientNum,
                stream,
                stream->flameVars->streamFuel,
                stream->flameVars->streamFuel2,
                stream->renderList->fuelVerts,
                0);
        if ( stream->flameVars->flameVar_streamFlameLength > 0.1 )
            Flame_Class_Stream_Render_Item(
                localClientNum,
                stream,
                stream->flameVars->streamFlame,
                stream->flameVars->streamFlame2,
                stream->renderList->flameVerts,
                1);
    }
}

void __cdecl CG_Flame_Render()
{
    const flameStream_s *stream; // [esp+14h] [ebp-4h]

    PROF_SCOPED("CG_Flame_Render");

    stream = flameStreamsUsed;
    if ( flameStreamsUsed && !Flame_GetLocalClientSourceRange() )
    {
        return;
    }
    while ( stream )
    {
        if ( stream->flameVars->flameVar_streamFlameLength > 0.1 )
            Flame_Class_Stream_Light_Chunks(stream);
        stream = (const flameStream_s *)stream->gen.listGlobal.next;
    }
}

