#include "cg_localents.h"

#include <cstring>
#include "cg_weapons.h"
#include <cgame_mp/cg_main_mp.h>
#include <bgame/bg_misc.h>

localEntity_s cg_localEntities[1][128];
localEntity_s cg_activeLocalEntities[1];
localEntity_s *cg_freeLocalEntities[1];

volatile unsigned int g_localEntThread;

void __cdecl CG_InitLocalEntities(int localClientNum)
{
    int entIter; // [esp+0h] [ebp-4h]

    memset(cg_localEntities[localClientNum], 0, sizeof(localEntity_s[128]));
    cg_activeLocalEntities[localClientNum].next = &cg_activeLocalEntities[localClientNum];
    cg_activeLocalEntities[localClientNum].prev = &cg_activeLocalEntities[localClientNum];
    cg_freeLocalEntities[localClientNum] = cg_localEntities[localClientNum];
    for (entIter = 0; entIter < 127; ++entIter)
        cg_localEntities[localClientNum][entIter].next = &cg_localEntities[localClientNum][entIter + 1];
}

localEntity_s *__cdecl CG_AllocLocalEntity(int localClientNum)
{
    localEntity_s *le; // [esp+0h] [ebp-4h]

    if (_InterlockedIncrement(&g_localEntThread) != 1
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_localents.cpp",
            65,
            0,
            "%s",
            "Sys_InterlockedIncrement( &g_localEntThread ) == 1"))
    {
        __debugbreak();
    }
    if (!cg_freeLocalEntities[localClientNum])
        CG_FreeLocalEntity(localClientNum, cg_activeLocalEntities[localClientNum].prev);
    le = cg_freeLocalEntities[localClientNum];
    cg_freeLocalEntities[localClientNum] = le->next;
    memset(le, 0, sizeof(localEntity_s));
    le->next = cg_activeLocalEntities[localClientNum].next;
    le->prev = &cg_activeLocalEntities[localClientNum];
    cg_activeLocalEntities[localClientNum].next->prev = le;
    cg_activeLocalEntities[localClientNum].next = le;
    if (_InterlockedDecrement(&g_localEntThread)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_localents.cpp",
            85,
            0,
            "%s",
            "Sys_InterlockedDecrement( &g_localEntThread ) == 0"))
    {
        __debugbreak();
    }
    return le;
}

void __cdecl CG_FreeLocalEntity(int localClientNum, localEntity_s *le)
{
    if ( !le->prev )
        Com_Error(ERR_DROP, "CG_FreeLocalEntity: not active");
    le->prev->next = le->next;
    le->next->prev = le->prev;
    le->next = cg_freeLocalEntities[localClientNum];
    cg_freeLocalEntities[localClientNum] = le;
}

void __cdecl CG_AddLocalEntityTracerBeams(int localClientNum)
{
    const cg_s *cgameGlob; // [esp+4h] [ebp-14h]
    int time; // [esp+8h] [ebp-10h]
    localEntity_s *prev; // [esp+Ch] [ebp-Ch]
    localEntity_s *le; // [esp+10h] [ebp-8h]
    localEntity_s *activeLocalEntities; // [esp+14h] [ebp-4h]

    if ( _InterlockedIncrement(&g_localEntThread) != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_localents.cpp",
                    158,
                    0,
                    "%s",
                    "Sys_InterlockedIncrement( &g_localEntThread ) == 1") )
    {
        __debugbreak();
    }
    activeLocalEntities = &cg_activeLocalEntities[localClientNum];
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    time = cgameGlob->time;
    for ( le = activeLocalEntities->prev; le != activeLocalEntities; le = prev )
    {
        prev = le->prev;
        if ( le->pos.trType != 3
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_localents.cpp",
                        170,
                        0,
                        "%s\n\t(le->pos.trType) = %i",
                        "(le->pos.trType == TR_LINEAR)",
                        le->pos.trType) )
        {
            __debugbreak();
        }
        if ( time >= le->endTime || time < le->pos.trTime )
        {
            CG_FreeLocalEntity(localClientNum, le);
        }
        else if ( time - le->pos.trTime >= cg_tracerNoDrawTime->current.integer )
        {
            CG_AddMovingTracer(cgameGlob, le, &cgameGlob->refdef);
        }
    }
    if ( _InterlockedDecrement(&g_localEntThread)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_localents.cpp",
                    183,
                    0,
                    "%s",
                    "Sys_InterlockedDecrement( &g_localEntThread ) == 0") )
    {
        __debugbreak();
    }
}

void __cdecl CG_AddMovingTracer(const cg_s *cgameGlob, localEntity_s *le, const refdef_s *refdef)
{
    float value; // [esp+8h] [ebp-6Ch]
    float v4; // [esp+Ch] [ebp-68h]
    float tracerLength; // [esp+34h] [ebp-40h]
    float dir[3]; // [esp+38h] [ebp-3Ch] BYREF
    float lengthFromClip; // [esp+44h] [ebp-30h]
    float start[3]; // [esp+48h] [ebp-2Ch] BYREF
    float end[3]; // [esp+54h] [ebp-20h] BYREF
    float lengthOfTracer; // [esp+60h] [ebp-14h]
    float startFromBase[3]; // [esp+64h] [ebp-10h]
    float lengthFromBase; // [esp+70h] [ebp-4h]

    BG_EvaluateTrajectory(&le->pos, cgameGlob->time, start);
    if ( ((LODWORD(le->pos.trDelta[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(le->pos.trDelta[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(le->pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_localents.cpp",
                    101,
                    0,
                    "%s",
                    "!IS_NAN((le->pos.trDelta)[0]) && !IS_NAN((le->pos.trDelta)[1]) && !IS_NAN((le->pos.trDelta)[2])") )
    {
        __debugbreak();
    }
    Vec3NormalizeTo(le->pos.trDelta, dir);
    startFromBase[0] = start[0] - le->pos.trBase[0];
    startFromBase[1] = start[1] - le->pos.trBase[1];
    startFromBase[2] = start[2] - le->pos.trBase[2];
    lengthFromBase = (float)((float)(startFromBase[0] * dir[0]) + (float)(startFromBase[1] * dir[1]))
                                 + (float)(startFromBase[2] * dir[2]);
    if ( lengthFromBase < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_localents.cpp",
                    106,
                    1,
                    "%s\n\t(lengthFromBase) = %g",
                    "(lengthFromBase >= 0.0f)",
                    lengthFromBase) )
    {
        __debugbreak();
    }
    lengthFromClip = le->tracerClipDist - lengthFromBase;
    if ( lengthFromClip < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_localents.cpp",
                    108,
                    1,
                    "%s\n\t(lengthFromClip) = %g",
                    "(lengthFromClip >= 0.0f)",
                    lengthFromClip) )
    {
        __debugbreak();
    }
    if ( le->leType == LE_MOVING_BULLET )
    {
        if ( (float)(cg_bulletLength->current.value - lengthFromClip) < 0.0 )
            value = cg_bulletLength->current.value;
        else
            value = lengthFromClip;
        lengthOfTracer = value;
    }
    else
    {
        if ( le->tracerLength == 0.0 )
            tracerLength = cg_tracerLength->current.value;
        else
            tracerLength = le->tracerLength;
        if ( (float)(tracerLength - lengthFromClip) < 0.0 )
            v4 = tracerLength;
        else
            v4 = lengthFromClip;
        lengthOfTracer = v4;
    }
    end[0] = (float)(lengthOfTracer * dir[0]) + start[0];
    end[1] = (float)(lengthOfTracer * dir[1]) + start[1];
    end[2] = (float)(lengthOfTracer * dir[2]) + start[2];
    CG_DrawTracer(start, end, refdef, le);
}

