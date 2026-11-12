#include "DynEntity_pieces.h"

#include <universal/dvar.h>
#include <gfx_d3d/r_gfx.h>
#include <win32/win_common.h>
#include <physics/phys_main.h>
#include <gfx_d3d/r_dpvs.h>
#include <xanim/xmodel.h>
#include <qcommon/common.h>
#include <bgame/bg_weapons_view.h>
#include <physics/phys_colgeom.h>
#include <physics/physpreset_load_obj.h>

const dvar_s *dynEntPieces_velocity;
const dvar_s *dynEntPieces_angularVelocity;
const dvar_s *dynEntPieces_impactForce;

int numPieces;

BreakablePiece g_breakablePieces[100];

void __cdecl DynEntPieces_RegisterDvars()
{
    dynEntPieces_velocity = _Dvar_RegisterVec3(
                                                        "dynEntPieces_velocity",
                                                        (0.0),
                                                        (0.0),
                                                        (0.0),
                                                        -1000.0,
                                                        1000.0,
                                                        0x80u,
                                                        "Initial breakable pieces velocity");
    dynEntPieces_angularVelocity = _Dvar_RegisterVec3(
                                                                     "dynEntPieces_angularVelocity",
                                                                     (0.0),
                                                                     (0.0),
                                                                     (0.0),
                                                                     -180.0,
                                                                     180.0,
                                                                     0x80u,
                                                                     "Initial breakable pieces angular velocity");
    dynEntPieces_impactForce = _Dvar_RegisterFloat(
                                                             "dynEntPieces_impactForce",
                                                             1000.0,
                                                             0.0,
                                                             1000000.0,
                                                             0x80u,
                                                             "Force applied when breakable is destroyed");
}

void __cdecl DynEntPieces_AddDrawSurfs()
{
    GfxScaledPlacement placement; // [esp+4h] [ebp-24h] BYREF
    int i; // [esp+24h] [ebp-4h]

    for ( i = 0; i < numPieces; ++i )
    {
        if ( g_breakablePieces[i].active )
        {
            Sys_EnterCriticalSection(CRITSECT_PHYSICS);
            Phys_ObjGetInterpolatedState(g_breakablePieces[i].physObjId, placement.base.origin, placement.base.quat);
            placement.scale = 1.0f;
            Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
            R_FilterXModelIntoScene(g_breakablePieces[i].model, &placement, 0, &g_breakablePieces[i].lightingHandle, 0.0);
        }
    }
}

char __cdecl DynEntPieces_SpawnPhysicsModel(
                int localClientNum,
                const XModel *model,
                const float *offset,
                const float *origin,
                const float (*axis)[3],
                const float *hitPos,
                const float *hitDir,
                PhysPreset *physPreset)
{
    int surfFlags; // [esp+8h] [ebp-74h]
    PhysPreset *v10; // [esp+Ch] [ebp-70h]
    float forceDir[3]; // [esp+18h] [ebp-64h] BYREF
    float velocity[3]; // [esp+24h] [ebp-58h] BYREF
    float angularVelocity[3]; // [esp+30h] [ebp-4Ch] BYREF
    int physObjId; // [esp+3Ch] [ebp-40h]
    float mins[3]; // [esp+40h] [ebp-3Ch] BYREF
    float quat[4]; // [esp+4Ch] [ebp-30h] BYREF
    int stype; // [esp+5Ch] [ebp-20h]
    float maxs[3]; // [esp+60h] [ebp-1Ch] BYREF
    float worldOffset[3]; // [esp+6Ch] [ebp-10h] BYREF
    PhysPreset *preset; // [esp+78h] [ebp-4h]

    if ( !model
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_pieces.cpp", 123, 0, "%s", "model") )
    {
        __debugbreak();
    }
    XModelGetBounds(model, mins, maxs);
    if ( mins[0] == maxs[0] || mins[1] == maxs[1] || mins[2] == maxs[2] )
    {
        Com_PrintWarning(1, "Failed to spawn pieces model '%s'.    No bounds.\n", model->name);
        return 0;
    }
    else
    {
        MatrixTransformVector(offset, axis, worldOffset);
        worldOffset[0] = worldOffset[0] + *origin;
        worldOffset[1] = worldOffset[1] + origin[1];
        worldOffset[2] = worldOffset[2] + origin[2];
        AxisToQuat(axis, quat);
        velocity[0] = dynEntPieces_velocity->current.value;
        velocity[1] = dynEntPieces_velocity->current.vector[1];
        velocity[2] = dynEntPieces_velocity->current.vector[2];
        angularVelocity[0] = dynEntPieces_angularVelocity->current.value;
        angularVelocity[1] = dynEntPieces_angularVelocity->current.vector[1];
        angularVelocity[2] = dynEntPieces_angularVelocity->current.vector[2];
        if ( physPreset )
            v10 = physPreset;
        else
            v10 = model->physPreset;
        preset = v10;
        velocity[2] = velocity[2] + v10->piecesUpwardVelocity;
        Sys_EnterCriticalSection(CRITSECT_PHYSICS);
        if ( model->collSurfs )
            surfFlags = model->collSurfs->surfFlags;
        else
            surfFlags = 13631488;
        stype = (surfFlags & 0x3F00000) >> 20;
        physObjId = (int)DynEntPieces_SpawnPhysObj(
                                             model->name,
                                             stype,
                                             mins,
                                             maxs,
                                             worldOffset,
                                             quat,
                                             velocity,
                                             angularVelocity,
                                             model->physPreset);
        Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
        if ( physObjId )
        {
            DynEntPieces_CalcForceDir(hitDir, preset->piecesSpreadFraction, forceDir);
            Phys_ObjBulletImpact(
                physObjId,
                hitPos,
                forceDir,
                dynEntPieces_impactForce->current.value,
                preset->bulletForceScale);
            g_breakablePieces[numPieces].physObjId = physObjId;
            g_breakablePieces[numPieces].model = model;
            g_breakablePieces[numPieces].lightingHandle = 0;
            g_breakablePieces[numPieces++].active = 1;
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

PhysObjUserData *__cdecl DynEntPieces_SpawnPhysObj(
                const char *modelName,
                int stype,
                const float *mins,
                const float *maxs,
                const float *position,
                const float *quat,
                const float *velocity,
                float *angularVelocity,
                const PhysPreset *physPreset)
{
    gjk_base_t *aabb_gjk_geom; // eax
    PhysObjUserData *physObjId; // [esp+0h] [ebp-Ch]
    gjk_geom_list_t gjk_geom_list; // [esp+4h] [ebp-8h] BYREF
    int savedregs; // [esp+Ch] [ebp+0h] BYREF

    if ( numPieces < 100 && physPreset )
    {
        gjk_geom_list.m_first_geom = 0;
        gjk_geom_list.m_geom_count = 0;
        aabb_gjk_geom = create_aabb_gjk_geom(mins, maxs, stype, &g_empty_collision_visitor);
        //gjk_geom_list_t::add_geom(&gjk_geom_list, aabb_gjk_geom);
        gjk_geom_list.add_geom(aabb_gjk_geom);
        physObjId = Phys_ObjCreate(0, position, quat, velocity, physPreset, &gjk_geom_list, 1, -1);
        if ( physObjId )
        {
            Phys_ObjSetAngularVelocity( (int)physObjId, angularVelocity);
            return physObjId;
        }
        else
        {
            Com_PrintWarning(1, "Failed to create physics object for '%s'.\n", modelName);
            return 0;
        }
    }
    else
    {
        Com_PrintWarning(1, "Failed to spawn pieces model '%s'.    It is missing physics preset.\n", modelName);
        return 0;
    }
}

void __cdecl DynEntPieces_CalcForceDir(const float *hitDir, float spreadFraction, float *forceDir)
{
    float outDir[3]; // [esp+8h] [ebp-Ch] BYREF

    outDir[0] = (float)((float)((float)rand() / 32767.0) * 2.0) - 1.0;
    outDir[1] = (float)((float)((float)rand() / 32767.0) * 2.0) - 1.0;
    outDir[2] = (float)((float)((float)rand() / 32767.0) * 2.0) - 1.0;
    Vec3Lerp(hitDir, outDir, spreadFraction, forceDir);
}

void __cdecl DynEntPieces_SpawnPieces(
                int localClientNum,
                const XModelPieces *pieces,
                const float *origin,
                const float (*axis)[3],
                const float *hitPos,
                const float *hitDir)
{
    int pieceIndex; // [esp+0h] [ebp-4h]

    if ( !pieces
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_pieces.cpp", 172, 0, "%s", "pieces") )
    {
        __debugbreak();
    }
    for ( pieceIndex = 0; pieceIndex < pieces->numpieces; ++pieceIndex )
        DynEntPieces_SpawnPhysicsModel(
            localClientNum,
            pieces->pieces[pieceIndex].model,
            pieces->pieces[pieceIndex].offset,
            origin,
            axis,
            hitPos,
            hitDir,
            0);
}

