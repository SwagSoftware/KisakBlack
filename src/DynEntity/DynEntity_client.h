#pragma once
#include <gfx_d3d/fxprimitives.h>
#include <xanim/xmodel.h>

#include "DynEntity_gamestate.h"
#include "DynEntity_server.h"
#include "DynEntity_coll.h"
#include <physics/phys_broad_phase.h>
#include <qcommon/common.h>

enum PhysWorld : __int32
{                                       // XREF: ?DynEnt_DestroyPhysics@@YAXW4PhysWorld@@PBUDynEntityDef@@PAUDynEntityClient@@@Z/r
    PHYS_WORLD_DYNENT  = 0x0,
    PHYS_WORLD_FX      = 0x1,
    PHYS_WORLD_RAGDOLL = 0x2,
    PHYS_WORLD_COUNT   = 0x3,
};

enum DynEntityType : __int32
{                                       // XREF: DynEntityDef/r
                                        // ?DynEnt_GetEntityProps@@YAPBUDynEntityProps@@W4DynEntityType@@@Z/r
    DYNENT_TYPE_INVALID = 0x0,
    DYNENT_TYPE_CLUTTER = 0x1,
    DYNENT_TYPE_DESTRUCT = 0x2,
    DYNENT_TYPE_COUNT = 0x3,
};

struct DynEnt_BurnData // sizeof=0x8
{
    unsigned __int16 id;
    // padding byte
    // padding byte
    unsigned int fx;
};

struct DynEnt_FadeData // sizeof=0x8
{                                       // XREF: .data:DynEnt_FadeData * gDynEntFadeData/r
    unsigned __int16 id;                // XREF: DynEntCl_RemoveFromFadeList+24/r
                                        // DynEntCl_RemoveFromFadeList+48/r ...
    // padding byte
    // padding byte
    int startTime;                      // XREF: DynEntCl_RemoveFromFadeList+4F/r
                                        // DynEntCl_RemoveFromFadeList+60/w ...
};

struct DynEntityPose // sizeof=0x20
{
    GfxPlacement pose;
    float radius;
};

struct DynEntityDef // sizeof=0x54
{
    DynEntityType type;
    GfxPlacement pose;
    XModel *xModel;
    XModel *destroyedxModel;
    unsigned __int16 brushModel;
    unsigned __int16 physicsBrushModel;
    const FxEffectDef *destroyFx;
    unsigned int destroySound;
    XModelPieces *destroyPieces;
    PhysPreset *physPreset;
    __int16 physConstraints[4];
    int health;
    int flags;
    int contents;
    unsigned __int16 targetname;
    unsigned __int16 target;
};

struct DynEntityClient // sizeof=0x14
{
    int physObjId;
    unsigned __int16 flags;
    unsigned __int16 lightingHandle;
    int health;
    unsigned __int16 burnTime;
    unsigned __int16 fadeTime;
    int physicsStartTime;
};

void __cdecl DynEntCl_RegisterDvars();
XModel *__cdecl DynEntCl_GetCurrentXModel(const DynEntityDef *dynEntDef, const DynEntityClient *dynEntClient);
bool __cdecl DynEntCl_IsHealthGone(const DynEntityDef *dynEntDef, const DynEntityClient *dynEntClient);
void __cdecl DynEntCl_LinkModel(unsigned __int16 dynEntId);
const DynEntityDef *__cdecl DynEnt_GetEntityDef(unsigned __int16 dynEntId, DynEntityDrawType drawType);
void __cdecl DynEntCl_LinkBrush(unsigned __int16 dynEntId);
double __cdecl DynEntCl_UpdateBModelWorldBounds(const DynEntityDef *dynEntDef, const GfxPlacement *pose);
void __cdecl DynEntCl_UnlinkEntity(unsigned __int16 dynEntId, DynEntityCollType drawType);
void __cdecl DynEnt_DestroyPhysics(PhysWorld worldIndex, const DynEntityDef *dynEntDef, DynEntityClient *dynEntClient);
void __cdecl DynEntCl_Shutdown(int localClientNum);
void __cdecl DynEntCl_InitEntities(int localClientNum);
DynEntityColl *__cdecl DynEnt_GetEntityColl(DynEntityCollType collType, unsigned __int16 dynEntId);
void __cdecl DynEntCl_DestroyEntityModel(unsigned __int16 id);
void __cdecl DynEnt_FixupLightConstraints(int localClientNum, int entnum, int primaryLightIndex);
void __cdecl DynEntCl_ProcessEntities(int localClientNum, int time);
void __cdecl DynEnt_UpdateLightConstraint(
                int localClientNum,
                const DynEntityDef *dynEntDef,
                const DynEntityPose *dynEntPose);
void __cdecl DynEnt_UpdateConstraints(int localClientNum, DynEntityClient *dynEntClient, const DynEntityDef *dynEntDef);
int __cdecl DynEnt_GetDamage(DynEntityClient *dynEntClient, const DynEntityDef *dynEntDef);
void    DynEnt_SetupConstraints(const DynEntityDef *dynEntDef);
int __cdecl DynEnt_GetSurfaceType(const DynEntityDef *dynEntDef);
void __cdecl DynEnt_CalcPhysPreset(const DynEntityDef *dynEntDef);
int __cdecl DynEntCl_CreatePhysObj(
                const DynEntityDef *dynEntDef,
                DynEntityClient *dynEntClient,
                const GfxPlacement *pose);


unsigned __int16 __cdecl DynEntCl_CreateEntityModel(
                XModel *model,
                const float *quat,
                const float *origin,
                const float *hitPos,
                const float *hitDir,
                int mod,
                PhysPreset *preset);
unsigned __int16 __cdecl DynEntCl_AddEntityModel();
void DynEntCl_CleanUpOldModels();
void __cdecl DynEntCl_PointTrace(const pointtrace_t *clip, trace_t *results);
void __cdecl DynEntCl_PointTrace_r(
                DynEntityCollType drawType,
                const pointtrace_t *clip,
                unsigned int sectorIndex,
                float *p1,
                float *p2,
                trace_t *results);
void __cdecl DynEntCl_AreaEntities_r(
                DynEntityCollType drawType,
                unsigned int sectorIndex,
                DynEntityAreaParms *areaParms);
unsigned __int16 __cdecl DynEntCl_AreaEntities(
                DynEntityDrawType drawType,
                const float *mins,
                const float *maxs,
                int contentMask,
                unsigned __int16 dynEntMaxCount,
                unsigned __int16 *dynEntList);
void __cdecl DynEntCl_TurnOffLight(const DynEntityDef *dynEntDef);
void __cdecl DynEntCl_Damage(
                int localClientNum,
                unsigned __int16 dynEntId,
                DynEntityCollType drawType,
                const float *hitPos,
                const float *hitDir,
                int damage);
void __cdecl DynEntCl_PlayEventFx(const FxEffectDef *def, const float *origin, const float (*axis)[3]);
unsigned int __cdecl DynEntCl_PlayBoltedFX(const FxEffectDef *fx, unsigned __int16 absDynEntId);
void __cdecl DynEntCl_RemoveFromFadeList(unsigned __int16 absDynEntId);
void __cdecl DynEntCl_SetFadeOut(unsigned __int16 dynEntId, DynEntityDrawType drawType);
void __cdecl DynEnt_UpdateFading(int currentTime);
double __cdecl DynEntCl_GetFadeTime(unsigned __int16 dynEntId, DynEntityDrawType drawType);
void __cdecl DynEntCl_SetBurning(unsigned __int16 dynEntId, DynEntityDrawType drawType, bool burning);
void DynEnt_ResetFading();
void DynEnt_ResetBurning();
void DynEnt_UpdateBurning();
void __cdecl DynEntCl_FlameDamage(
                int localClientNum,
                unsigned __int16 dynEntId,
                DynEntityCollType drawType,
                const float *hitPos,
                const float *hitVel,
                int damage);
void __cdecl DynEntCl_SetBurnFrac(unsigned __int16 dynEntId, DynEntityDrawType drawType, float frac);
double __cdecl DynEntCl_GetBurnTime(unsigned __int16 dynEntId, DynEntityDrawType drawType);
void __cdecl DynEntCl_MeleeEvent(int localClientNum, int attackerEntNum);
void __cdecl DynEntCl_EntityImpactEvent(
                const trace_t *trace,
                int localClientNum,
                int sourceEntityNum,
                const float *start,
                float *hitPos,
                bool isMelee);
void __cdecl DynEntCl_PlayImpactEffects(
                int localClientNum,
                int sourceEntityNum,
                unsigned int surfType,
                float *hitPos,
                const float *hitNormal);
char __cdecl DynEntCl_EventNeedsProcessed(int localClientNum, int sourceEntityNum);
char __cdecl DynEntCl_DynEntImpactEvent(
                int localClientNum,
                int sourceEntityNum,
                const float *start,
                const float *end,
                int damage,
                bool isMelee);
void __cdecl DynEntCl_ExplosionEvent(
                int localClientNum,
                bool isCylinder,
                float *origin,
                float innerRadius,
                float outerRadius,
                float *impulse,
                float inScale,
                int innerDamage,
                int outerDamage);
unsigned int __cdecl DynEntCl_GetClosestEntities(
                DynEntityCollType drawType,
                float *radiusMins,
                float *radiusMaxs,
                float *origin,
                unsigned __int16 *hitEnts,
                bool isCylinder);
void __cdecl DynEntCl_DestroyEvent(
                int localClientNum,
                unsigned __int16 dynEntId,
                DynEntityCollType drawType,
                const float *hitPos,
                const float *hitDir);
void __cdecl CreateRopes(int localClientNum);
void __cdecl MapHitLocationToRagdollBoneName(hitLocation_t hitLoc, unsigned int *boneName);


extern const dvar_t *dynEnt_bulletForce;
extern const dvar_t *dynEnt_explodeForce;
extern const dvar_t *dynEnt_explodeUpbias;
extern const dvar_t *dynEnt_explodeSpinScale;
extern const dvar_t *dynEnt_explodeMinForce;
extern const dvar_t *dynEnt_explodeMaxEnts;
extern const dvar_t *dynEnt_spawnedLimit;
extern const dvar_t *dynEnt_sentientAutoActivate;