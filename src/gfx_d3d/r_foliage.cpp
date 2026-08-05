#include "r_foliage.h"
#include <xanim/xmodel_utils.h>
#include "r_dvars.h"
#include "r_debug.h"
#include <universal/com_workercmds.h>
#include "r_warn.h"
#include "r_shade.h"
#include "r_state.h"

unsigned int g_fullySquashedModelBits[64];
ModelHashTable<ActiveModelNode, 63, 128> g_activeModelsHash;
unsigned int g_modelInfoCount;
unsigned int g_numDynSModelInMap;
unsigned int g_isFoliageModel[64];
ClientFoliageInfo g_foliageInfo[4];

FoliageShakeParams fsp_GrenadeExplosion = { 3.0, 200.0, 25.0, 10.0, 0.0 };
FoliageShakeParams fsp_WeaponFiring = { 1.5, 50.0, 6.0, 20.0, 20.0 };
FoliageShakeParams fsp_FlameThrower = { 2.0, 125.0, 25.0, 10.0, 40.0 };

float grassForceRad = 32.0f;
float grassForceVehicleRad = 100.0f;
float grassForceStrengthPlayerPos = 20.0f;
float grassForceStrengthVehiclePos = 30.0f;

const float MAX_SQUASH_AMOUNT = 0.5f;
const float COLOR_MULTIPLIER_AT_FULLSQUASH = 0.3f;

volatile unsigned int dyn_smodel_drawstateLimit = 1;
jqModule dyn_smodel_drawstateModule =
{
    .Name = "dyn_smodel_drawstate",
    .Type = JQ_WORKER_GENERIC,
    .Code = (int(__cdecl *)(jqBatch *))dyn_smodel_drawstateCallback
};
jqWorkerCmd dyn_smodel_drawstateWorkerCmd = { &dyn_smodel_drawstateModule, 36u, 0, 0, &dyn_smodel_drawstateLimit, NULL, 0u };

DrawStateWorkerSharedBuffer g_drawStateWorkerSharedBuffer;

unsigned int g_mainThreadFoliageInfo;

bool __cdecl ModelIsFullySquashed(unsigned int drawInstIndex)
{
    unsigned int v2; // [esp+0h] [ebp-10h]

    if ( drawInstIndex >> 5 < 0x40 )
        v2 = (g_fullySquashedModelBits[drawInstIndex >> 5] >> (drawInstIndex & 0x1F)) & 1;
    else
        v2 = 0;
    return v2 != 0;
}

void __cdecl SetModelIsFullySquashed(unsigned int drawInstIndex)
{
    if ( drawInstIndex >> 5 < 0x40 )
        g_fullySquashedModelBits[drawInstIndex >> 5] |= 1 << (drawInstIndex & 0x1F);
}

double __cdecl GetFoliageSquashFromHash(unsigned int drawInstIndex)
{
    ActiveModelNode *n; // [esp+Ch] [ebp-Ch]
    float squashAmt; // [esp+10h] [ebp-8h]

    if ( drawInstIndex >= 0x10000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_foliage.cpp",
                    190,
                    0,
                    "%s",
                    "drawInstIndex < (1 << (sizeof(unsigned short)*8))") )
    {
        __debugbreak();
    }
    squashAmt = 0.0f;
    if ( ModelIsFullySquashed(drawInstIndex) )
    {
        return 1.0f;
    }
    else
    {
        //n = ModelHashTable<ActiveModelNode,63,128>::FindByKey(&g_activeModelsHash, drawInstIndex);
        n = g_activeModelsHash.FindByKey(drawInstIndex);
        if ( n )
            return n->curSquashValue;
    }
    return squashAmt;
}

void __cdecl R_FoliageSetPerFramePersistantForces(
                PerFrameFoliageInfo *frameInfo,
                const ClientFoliageInfo *clientFoliageInfo)
{
    GrassForce *perFrameForce; // [esp+0h] [ebp-Ch]
    unsigned int forceIdx; // [esp+8h] [ebp-4h]

    frameInfo->numGrassPersistantForces = 0;
    for ( forceIdx = 0; forceIdx < 8; ++forceIdx )
    {
        if ( GrassPersistForceIsActive(&clientFoliageInfo->persistantForces[forceIdx]) )
        {
            perFrameForce = &frameInfo->grassPersistantForces[frameInfo->numGrassPersistantForces];
            perFrameForce->radius = clientFoliageInfo->persistantForces[forceIdx].radius;
            perFrameForce->strength = clientFoliageInfo->persistantForces[forceIdx].strength;
            perFrameForce->frequency = clientFoliageInfo->persistantForces[forceIdx].frequency;
            perFrameForce->worldPosition[0] = clientFoliageInfo->persistantForces[forceIdx].worldPosition[0];
            perFrameForce->worldPosition[1] = clientFoliageInfo->persistantForces[forceIdx].worldPosition[1];
            perFrameForce->worldPosition[2] = clientFoliageInfo->persistantForces[forceIdx].worldPosition[2];
            ++frameInfo->numGrassPersistantForces;
        }
    }
}

bool __cdecl GrassPersistForceIsActive(const GrassPersistForce *force)
{
    return force->isActive;
}

void __cdecl R_FoliageInitModels(GfxWorld *gfxWorld)
{
    unsigned int dynamicSModelCount; // [esp+0h] [ebp-8h]

    if ( (int)gfxWorld->dpvs.dynamicSModelCount > 2048 )
        dynamicSModelCount = 2048;
    else
        dynamicSModelCount = gfxWorld->dpvs.dynamicSModelCount;
    g_modelInfoCount = dynamicSModelCount;
    g_numDynSModelInMap = gfxWorld->dpvs.dynamicSModelCount;
    Com_Printf(8, "\n");
    Com_Printf(8, "Foliage: --------------------------\n");
    Com_Printf(8, "Foliage: smodelCount = %d\n", gfxWorld->dpvs.smodelCount);
    Com_Printf(8, "Foliage: dyn smodel count = %u (max=%u)\n", gfxWorld->dpvs.dynamicSModelCount, 2048);
    Com_Printf(8, "Foliage: sizeof(g_activeModelsHash) = %d, sizeof(g_fullySquashedModelBits) = %d\n", 1152, 256);
    Com_Printf(8, "Foliage: --------------------------\n\n");
    R_FoliageSetInitialBurnState();
    R_MarkDynFoliageModels(gfxWorld);
}

void __cdecl R_MarkDynFoliageModels(GfxWorld *gfxWorld)
{
    const MaterialTechnique *tech; // [esp+Ch] [ebp-30h]
    int techIdx; // [esp+10h] [ebp-2Ch]
    int surfIdx; // [esp+18h] [ebp-24h]
    Material **materialForSurf; // [esp+1Ch] [ebp-20h]
    int surfaceCount; // [esp+24h] [ebp-18h]
    XModel *xm; // [esp+2Ch] [ebp-10h]
    XSurface *surfaces; // [esp+30h] [ebp-Ch] BYREF
    bool isDynFoliage; // [esp+37h] [ebp-5h]
    unsigned int modelIdx; // [esp+38h] [ebp-4h]

    memset(g_isFoliageModel, 0, sizeof(g_isFoliageModel));
    for ( modelIdx = 0; modelIdx < g_modelInfoCount; ++modelIdx )
    {
        xm = gfxWorld->dpvs.smodelDrawInsts[modelIdx].model;
        if ( xm )
        {
            surfaceCount = XModelGetSurfaces(xm, &surfaces, 0);
            materialForSurf = XModelGetSkins(xm, 0);
            isDynFoliage = 0;
            for ( surfIdx = 0; surfIdx < surfaceCount; ++surfIdx )
            {
                if ( (*materialForSurf)->localTechniqueSet )
                {
                    for ( techIdx = 0; techIdx < 130; ++techIdx )
                    {
                        tech = (*materialForSurf)->localTechniqueSet->techniques[techIdx];
                        if ( tech && (tech->flags & 0x20) != 0 )
                        {
                            isDynFoliage = 1;
                            break;
                        }
                    }
                    if ( isDynFoliage )
                        break;
                }
                ++materialForSurf;
            }
            if ( isDynFoliage && modelIdx >> 5 < 0x40 )
                g_isFoliageModel[modelIdx >> 5] |= 1 << (modelIdx & 0x1F);
        }
    }
}

void __cdecl R_FoliageShutdown()
{
    g_modelInfoCount = 0;
}

void __cdecl R_FoliageSetInitialBurnState()
{
    //ModelHashTable<ActiveModelNode,63,128>::Init(&g_activeModelsHash);
    g_activeModelsHash.Init();
    memset(g_fullySquashedModelBits, 0, sizeof(g_fullySquashedModelBits));
}

//double __cdecl R_GetWindBestStrength(
//                const GrassWindForce *windForces,
//                unsigned int numWindForces,
//                const float *modelPlacement)
//{
//    long double v4; // [esp+0h] [ebp-20h]
//    const GrassWindForce *gforce; // [esp+8h] [ebp-18h]
//    float ratio; // [esp+10h] [ebp-10h]
//    unsigned int i; // [esp+14h] [ebp-Ch]
//    int best; // [esp+18h] [ebp-8h]
//    float best_str; // [esp+1Ch] [ebp-4h]
//
//    best = -1;
//    best_str = 0.0f;
//    for ( i = 0; i < numWindForces; ++i )
//    {
//        ratio = (float)((float)((float)(*modelPlacement - windForces[i].worldPosition[0])
//                                                    * (float)(*modelPlacement - windForces[i].worldPosition[0]))
//                                    + (float)((float)(modelPlacement[1] - windForces[i].worldPosition[1])
//                                                    * (float)(modelPlacement[1] - windForces[i].worldPosition[1])))
//                    * windForces[i].inverseRadiusSquared;
//        if ( ratio <= 1.0 )
//        {
//            __libm_sse2_sin(v4);
//            *(float *)&v4 = (float)(ratio * 3.1415927) / 2.0;
//            gforce = &windForces[i];
//            *((float *)&v4 + 1) = (float)(1.0 - *(float *)&v4) * gforce->strength;
//            if ( *((float *)&v4 + 1) > best_str || best == -1 )
//            {
//                best = i;
//                best_str = (float)(1.0 - *(float *)&v4) * gforce->strength;
//            }
//        }
//    }
//    return best_str;
//}

float R_GetWindBestStrength(
    const GrassWindForce *windForces,
    unsigned int numWindForces,
    const float *modelPlacement)
{
    int best = -1;
    float bestStrength = 0.0f;

    const float x = modelPlacement[0];
    const float y = modelPlacement[1];

    for (unsigned int i = 0; i < numWindForces; ++i)
    {
        const GrassWindForce &force = windForces[i];

        const float dx = x - force.worldPosition[0];
        const float dy = y - force.worldPosition[1];

        const float distSq = dx * dx + dy * dy;
        const float ratio = distSq * force.inverseRadiusSquared;

        if (ratio <= 1.0f)
        {
            const float t = (ratio * float(M_PI)) * 0.5f;
            const float s = sinf(t);

            const float strength = (1.0f - s) * force.strength;

            if (best == -1 || strength > bestStrength)
            {
                best = i;
                bestStrength = strength;
            }
        }
    }

    return bestStrength;
}


bool __cdecl R_StaticModelNeedsCharredTech(unsigned int modelDrawInstIndex)
{
    float squashAmt; // [esp+14h] [ebp-8h]
    unsigned int isDynFoliage; // [esp+18h] [ebp-4h]

    if ( modelDrawInstIndex >> 5 < 0x40 )
        isDynFoliage = (g_isFoliageModel[modelDrawInstIndex >> 5] >> (modelDrawInstIndex & 0x1F)) & 1;
    else
        isDynFoliage = 0;
    if ( isDynFoliage )
        return 0;
    if ( r_swrk_override_enable->current.enabled )
        return r_swrk_override_characterCharredAmount->current.value > 0.0;
    squashAmt = GetFoliageSquashFromHash(modelDrawInstIndex);
    return squashAmt > 0.0;
}

int draw_burn_foliage_debug;
void __cdecl R_FoliageNotifyBurn(float *loc, float rad, int *models, int models_count, bool instant)
{
    unsigned int dynamicSModelCount; // [esp+8h] [ebp-4C8h]
    ActiveModelNode newNode; // [esp+48h] [ebp-488h] BYREF
    unsigned __int16 v7; // [esp+50h] [ebp-480h]
    unsigned int drawInstIndex; // [esp+54h] [ebp-47Ch]
    int i; // [esp+58h] [ebp-478h]
    float maxs[3]; // [esp+5Ch] [ebp-474h] BYREF
    unsigned int MODELS_BUFFER_CAPACITY; // [esp+68h] [ebp-468h]
    float v12; // [esp+6Ch] [ebp-464h]
    float v13; // [esp+70h] [ebp-460h]
    float v14; // [esp+74h] [ebp-45Ch]
    int smodels[257]; // [esp+78h] [ebp-458h] BYREF
    int modelsCount; // [esp+47Ch] [ebp-54h] BYREF
    float mins[3]; // [esp+480h] [ebp-50h] BYREF
    ActiveModelNode n; // [esp+48Ch] [ebp-44h] BYREF
    unsigned __int16 key; // [esp+494h] [ebp-3Ch]
    const GfxStaticModelInst *smodelInst; // [esp+498h] [ebp-38h]
    unsigned int modelIndex; // [esp+49Ch] [ebp-34h]
    int mi; // [esp+4A0h] [ebp-30h]
    float max[3]; // [esp+4A4h] [ebp-2Ch] BYREF
    float ext[3]; // [esp+4B0h] [ebp-20h]
    float min[3]; // [esp+4BCh] [ebp-14h] BYREF
    float radSq; // [esp+4C8h] [ebp-8h]
    unsigned int maxSModelCount; // [esp+4CCh] [ebp-4h]

    if ( draw_burn_foliage_debug )
    {
        ext[0] = rad;
        ext[1] = rad;
        ext[2] = rad;
        max[0] = *loc + rad;
        max[1] = loc[1] + rad;
        max[2] = loc[2] + rad;
        min[0] = *loc - rad;
        min[1] = loc[1] - rad;
        min[2] = loc[2] - rad;
        R_AddDebugBox(&frontEndDataOut->debugGlobals, min, max, colorYellow);
    }
    radSq = rad * rad;
    if ( (int)rgp.world->dpvs.dynamicSModelCount > 2048 )
        dynamicSModelCount = 2048;
    else
        dynamicSModelCount = rgp.world->dpvs.dynamicSModelCount;
    maxSModelCount = dynamicSModelCount;
    if ( models )
    {
        for ( mi = 0; mi < models_count; ++mi )
        {
            modelIndex = models[mi];
            if ( modelIndex < maxSModelCount )
            {
                smodelInst = &rgp.world->dpvs.smodelInsts[modelIndex];
                if ( AABBOverlapsSphere(smodelInst->mins, smodelInst->maxs, radSq, loc) )
                {
                    key = modelIndex;
                    if ( instant )
                    {
                        SetModelIsFullySquashed(modelIndex);
                    }
                    //else if ( !ModelIsFullySquashed(modelIndex) && !ModelHashTable<ActiveModelNode,63,128>::FindByKey(&g_activeModelsHash, key) )
                    else if ( !ModelIsFullySquashed(modelIndex) && !g_activeModelsHash.FindByKey(key) )
                    {
                        n.curSquashValue = 0.0f;
                        n.modelIndex = key;
                        //ModelHashTable<ActiveModelNode,63,128>::Add(&g_activeModelsHash, &n);
                        g_activeModelsHash.Add(n);
                    }
                    if ( draw_burn_foliage_debug )
                        R_AddDebugBox(&frontEndDataOut->debugGlobals, smodelInst->mins, smodelInst->maxs, colorGreen);
                }
                else if ( draw_burn_foliage_debug )
                {
                    R_AddDebugBox(&frontEndDataOut->debugGlobals, smodelInst->mins, smodelInst->maxs, colorWhite);
                }
            }
        }
    }
    else
    {
        v12 = rad;
        v13 = rad;
        v14 = rad;
        maxs[0] = *loc + rad;
        maxs[1] = loc[1] + rad;
        maxs[2] = loc[2] + rad;
        mins[0] = *loc - rad;
        mins[1] = loc[1] - rad;
        mins[2] = loc[2] - rad;
        MODELS_BUFFER_CAPACITY = 256;
        modelsCount = 0;
        R_GetStaticModels(mins, maxs, smodels, &modelsCount, 256);
        for ( i = 0; i < modelsCount; ++i )
        {
            drawInstIndex = smodels[i];
            v7 = drawInstIndex;
            if ( instant )
            {
                SetModelIsFullySquashed(drawInstIndex);
            }
            //else if ( !ModelIsFullySquashed(drawInstIndex) && !ModelHashTable<ActiveModelNode,63,128>::FindByKey(&g_activeModelsHash, v7) )
            else if ( !ModelIsFullySquashed(drawInstIndex) && !g_activeModelsHash.FindByKey(v7) )
            {
                newNode.curSquashValue = 0.0f;
                newNode.modelIndex = v7;
                g_activeModelsHash.Add(newNode);
            }
        }
    }
}

bool __cdecl AABBOverlapsSphere(
                const float *boxMin,
                const float *boxMax,
                float sphereRadiusSquared,
                const float *sphereCenter)
{
    int i; // [esp+8h] [ebp-Ch]
    float d; // [esp+Ch] [ebp-8h]
    float s; // [esp+10h] [ebp-4h]
    float sa; // [esp+10h] [ebp-4h]

    d = 0.0f;
    for ( i = 0; i < 3; ++i )
    {
        if ( boxMin[i] <= sphereCenter[i] )
        {
            if ( sphereCenter[i] > boxMax[i] )
            {
                sa = sphereCenter[i] - boxMax[i];
                d = (float)(sa * sa) + d;
            }
        }
        else
        {
            s = sphereCenter[i] - boxMin[i];
            d = (float)(s * s) + d;
        }
    }
    return sphereRadiusSquared >= d;
}

void __cdecl AddNearMuzzleShakeForce(
                unsigned int id,
                const float *forcePos,
                const float *forwardVector,
                const FoliageShakeParams *params)
{
    float locForwardExtension; // [esp+0h] [ebp-30h]
    unsigned int i; // [esp+8h] [ebp-28h]
    GrassPersistForce *persistantForces; // [esp+Ch] [ebp-24h]
    GrassPersistForce *force; // [esp+10h] [ebp-20h]
    float force2client; // [esp+14h] [ebp-1Ch]
    float force2client_4; // [esp+18h] [ebp-18h]
    float force2client_8; // [esp+1Ch] [ebp-14h]
    bool isActiveForce; // [esp+23h] [ebp-Dh]
    int clientIdx; // [esp+2Ch] [ebp-4h]

    for ( clientIdx = 0; clientIdx < 4; ++clientIdx )
    {
        if ( g_foliageInfo[clientIdx].clientIsActive )
        {
            force2client = g_foliageInfo[clientIdx].clientPlayerPos[0] - *forcePos;
            force2client_4 = g_foliageInfo[clientIdx].clientPlayerPos[1] - forcePos[1];
            force2client_8 = g_foliageInfo[clientIdx].clientPlayerPos[2] - forcePos[2];
            if ( (float)((float)((float)(force2client * force2client) + (float)(force2client_4 * force2client_4))
                                 + (float)(force2client_8 * force2client_8)) <= (float)((float)(params->radius + 350.0)
                                                                                                                                            * (float)(params->radius + 350.0)) )
            {
                force = 0;
                isActiveForce = 0;
                persistantForces = g_foliageInfo[clientIdx].persistantForces;
                for ( i = 0; i < 8; ++i )
                {
                    if ( GrassPersistForceIsActive(&persistantForces[i]) )
                    {
                        if ( persistantForces[i].id == id )
                        {
                            force = &persistantForces[i];
                            isActiveForce = 1;
                            break;
                        }
                    }
                    else
                    {
                        force = &persistantForces[i];
                    }
                }
                if ( force )
                {
                    if ( isActiveForce )
                    {
                        if ( force->shakeForceState )
                        {
                            if ( force->shakeForceState == GrassPersistForce::ShakeForceState::MAINTAINING )
                            {
                                force->age = 0.0f;
                            }
                            else if ( force->shakeForceState == GrassPersistForce::ShakeForceState::DECAYING )
                            {
                                force->shakeForceState = GrassPersistForce::ShakeForceState::GAINING;
                            }
                        }
                    }
                    else
                    {
                        force->id = id;
                        force->age = 0.0f;
                        force->forceType = GrassPersistForce::ForceType::NEAR_MUZZLE_SHAKE;
                        force->strength = 0.0f;
                        force->trgForcePercent = 1.0f;
                        force->shakeForceState = GrassPersistForce::ShakeForceState::GAINING;
                        force->curForcePercent = 0.0f;
                        SetGrassPersistForceToActive(force);
                        force->maxStrength = params->strength;
                        force->radius = params->radius;
                        force->frequency = params->frequency;
                        force->maxAge = params->maxAge;
                    }
                    locForwardExtension = params->locForwardExtension;
                    force->worldPosition[0] = (float)(locForwardExtension * *forwardVector) + *forcePos;
                    force->worldPosition[1] = (float)(locForwardExtension * forwardVector[1]) + forcePos[1];
                    force->worldPosition[2] = (float)(locForwardExtension * forwardVector[2]) + forcePos[2];
                }
            }
        }
    }
}

void __cdecl SetGrassPersistForceToActive(GrassPersistForce *force)
{
    force->isActive = 1;
}

int draw_explosion_foliage_debug;
void __cdecl R_FoliageNotifyGrenadeExplosion(int localClientNum, float *loc)
{
    unsigned int v2; // [esp+Ch] [ebp-46Ch]
    const GfxStaticModelInst *inst; // [esp+1Ch] [ebp-45Ch]
    unsigned int modelIdx; // [esp+20h] [ebp-458h]
    float d2; // [esp+24h] [ebp-454h]
    float d2a; // [esp+24h] [ebp-454h]
    float s; // [esp+28h] [ebp-450h]
    float sa; // [esp+28h] [ebp-450h]
    float sb; // [esp+28h] [ebp-450h]
    int i; // [esp+38h] [ebp-440h]
    float max[3]; // [esp+3Ch] [ebp-43Ch] BYREF
    unsigned int MODELS_BUFFER_CAPACITY; // [esp+48h] [ebp-430h]
    float zero[3]; // [esp+4Ch] [ebp-42Ch] BYREF
    float rad; // [esp+58h] [ebp-420h]
    float ext[3]; // [esp+5Ch] [ebp-41Ch]
    int smodels[256]; // [esp+68h] [ebp-410h] BYREF
    int modelsCount; // [esp+468h] [ebp-10h] BYREF
    float min[3]; // [esp+46Ch] [ebp-Ch] BYREF

    rad = 8.0f;
    ext[0] = 8.0f;
    ext[1] = 8.0f;
    ext[2] = 8.0f;
    max[0] = *loc + 8.0;
    max[1] = loc[1] + 8.0;
    max[2] = loc[2] + 8.0;
    min[0] = *loc - 8.0;
    min[1] = loc[1] - 8.0;
    min[2] = loc[2] - 8.0;
    if ( draw_explosion_foliage_debug )
        R_AddDebugBox(&frontEndDataOut->debugGlobals, min, max, colorRed);
    MODELS_BUFFER_CAPACITY = 256;
    modelsCount = 0;
    R_GetStaticModels(min, max, smodels, &modelsCount, 256);
    for ( i = 0; i < modelsCount; ++i )
    {
        modelIdx = smodels[i];
        inst = &rgp.world->dpvs.smodelInsts[modelIdx];
        if ( modelIdx >> 5 < 0x40 )
            v2 = (g_isFoliageModel[modelIdx >> 5] >> (modelIdx & 0x1F)) & 1;
        else
            v2 = 0;
        if ( v2 )
        {
            s = *loc - (float)(inst->maxs[0] + (float)((float)(inst->maxs[0] - inst->maxs[0]) * 0.5));
            d2 = s * s;
            sa = loc[1] - (float)(inst->maxs[1] + (float)((float)(inst->maxs[1] - inst->maxs[1]) * 0.5));
            d2a = (float)(sa * sa) + d2;
            sb = loc[2] - (float)(inst->maxs[2] + (float)((float)(inst->maxs[2] - inst->maxs[2]) * 0.5));
            if ( (float)((float)(sb * sb) + d2a) < 100000.0 )
                SetModelIsFullySquashed(modelIdx);
        }
    }
    memset(zero, 0, sizeof(zero));
    AddNearMuzzleShakeForce(0, loc, zero, &fsp_GrenadeExplosion);
}

void __cdecl R_FoliageNofityWeaponFiring(
                int localClientNum,
                unsigned int id,
                const float *loc,
                const float *forwardVector,
                bool isFlameThrower)
{
    if ( isFlameThrower )
        AddNearMuzzleShakeForce(id, loc, forwardVector, &fsp_FlameThrower);
    else
        AddNearMuzzleShakeForce(id, loc, forwardVector, &fsp_WeaponFiring);
}

void __cdecl R_FoliageSetGrassWindForces(int clientIdx, float *pos, float radius, float strength)
{
    float *worldPosition; // [esp+0h] [ebp-8h]
    PerFrameFoliageInfo *foliageInfo; // [esp+4h] [ebp-4h]

    if ( r_grassWindForceEnable->current.enabled )
    {
        foliageInfo = &g_foliageInfo[clientIdx].perFrame[g_mainThreadFoliageInfo];
        if ( foliageInfo->numGrassWindForces < 0x10 )
        {
            worldPosition = foliageInfo->grassWindForces[foliageInfo->numGrassWindForces].worldPosition;
            *worldPosition = *pos;
            worldPosition[1] = pos[1];
            foliageInfo->grassWindForces[foliageInfo->numGrassWindForces].strength = strength;
            foliageInfo->grassWindForces[foliageInfo->numGrassWindForces++].inverseRadiusSquared = 1.0
                                                                                                                                                                                     / (float)(radius * radius);
        }
    }
}

void __cdecl DrawDebugAxis(const float *pos)
{
    float xend[3]; // [esp+Ch] [ebp-3Ch] BYREF
    float zend[3]; // [esp+18h] [ebp-30h] BYREF
    float yaxis[3]; // [esp+24h] [ebp-24h]
    float yend[3]; // [esp+30h] [ebp-18h] BYREF
    float zaxis[3]; // [esp+3Ch] [ebp-Ch]

    yaxis[0] = 0.0f;
    yaxis[1] = 1.0f;
    yaxis[2] = 0.0f;
    zaxis[0] = 0.0f;
    zaxis[1] = 0.0f;
    zaxis[2] = 1.0f;
    xend[0] = (float)(30.0 * 1.0) + *pos;
    xend[1] = (float)(30.0 * 0.0) + pos[1];
    xend[2] = (float)(30.0 * 0.0) + pos[2];
    yend[0] = (float)(30.0 * 0.0) + *pos;
    yend[1] = (float)(30.0 * 1.0) + pos[1];
    yend[2] = (float)(30.0 * 0.0) + pos[2];
    zend[0] = (float)(30.0 * 0.0) + *pos;
    zend[1] = (float)(30.0 * 0.0) + pos[1];
    zend[2] = (float)(30.0 * 1.0) + pos[2];
    R_AddDebugLine(&frontEndDataOut->debugGlobals, pos, xend, colorRed, 0);
    R_AddDebugLine(&frontEndDataOut->debugGlobals, pos, yend, colorGreen, 0);
    R_AddDebugLine(&frontEndDataOut->debugGlobals, pos, zend, colorBlue, 0);
}

void __cdecl AddInstantForce(const GrassForce *force)
{
    float force2client; // [esp+4h] [ebp-1Ch]
    float force2client_4; // [esp+8h] [ebp-18h]
    float force2client_8; // [esp+Ch] [ebp-14h]
    PerFrameFoliageInfo *foliageInfo; // [esp+14h] [ebp-Ch]
    int clientIdx; // [esp+1Ch] [ebp-4h]

    for ( clientIdx = 0; clientIdx < 4; ++clientIdx )
    {
        if ( g_foliageInfo[clientIdx].clientIsActive )
        {
            force2client = g_foliageInfo[clientIdx].clientPlayerPos[0] - force->worldPosition[0];
            force2client_4 = g_foliageInfo[clientIdx].clientPlayerPos[1] - force->worldPosition[1];
            force2client_8 = g_foliageInfo[clientIdx].clientPlayerPos[2] - force->worldPosition[2];
            if ( (float)((float)((float)(force2client * force2client) + (float)(force2client_4 * force2client_4))
                                 + (float)(force2client_8 * force2client_8)) <= (float)((float)(force->radius + 250.0)
                                                                                                                                            * (float)(force->radius + 250.0)) )
            {
                foliageInfo = &g_foliageInfo[clientIdx].perFrame[g_mainThreadFoliageInfo];
                if ( foliageInfo->numGrassInstantForces < 0x10 )
                    foliageInfo->grassInstantForces[foliageInfo->numGrassInstantForces++] = *force;
            }
        }
    }
}

int drawGrassForceBox;
void __cdecl R_FoliageNotifyVehiclePosition(int localClientNum, const float *vehiclePos)
{
    float mins[3]; // [esp+0h] [ebp-3Ch] BYREF
    float ext[3]; // [esp+Ch] [ebp-30h]
    float maxs[3]; // [esp+18h] [ebp-24h] BYREF
    GrassForce force; // [esp+24h] [ebp-18h] BYREF

    force.worldPosition[0] = *vehiclePos;
    force.worldPosition[1] = vehiclePos[1];
    force.worldPosition[2] = vehiclePos[2];
    force.radius = grassForceVehicleRad;
    force.strength = grassForceStrengthVehiclePos;
    force.frequency = 0.0f;
    AddInstantForce(&force);
    if ( drawGrassForceBox )
    {
        DrawDebugAxis(vehiclePos);
        ext[0] = grassForceVehicleRad;
        ext[1] = grassForceVehicleRad;
        ext[2] = grassForceVehicleRad;
        maxs[0] = *vehiclePos + grassForceVehicleRad;
        maxs[1] = vehiclePos[1] + grassForceVehicleRad;
        maxs[2] = vehiclePos[2] + grassForceVehicleRad;
        mins[0] = (float)(-1.0 * grassForceVehicleRad) + *vehiclePos;
        mins[1] = (float)(-1.0 * grassForceVehicleRad) + vehiclePos[1];
        mins[2] = (float)(-1.0 * grassForceVehicleRad) + vehiclePos[2];
        R_AddDebugBox(&frontEndDataOut->debugGlobals, mins, maxs, colorLtGrey);
    }
}

void __cdecl R_FoliageSetDefaultShaderConstants(GfxCmdBufSourceState *source)
{
    float zero[4]; // [esp+10h] [ebp-20h] BYREF
    float parms[4]; // [esp+20h] [ebp-10h] BYREF

    parms[0] = 1.0f;
    parms[1] = 0.0f;
    parms[2] = 1.0f;
    parms[3] = 1.0f;
    R_SetCodeConstantFromVec4(source, CONST_SRC_CODE_GRASS_PARMS, parms);

    memset(zero, 0, sizeof(zero));
    R_SetCodeConstantFromVec4(source, CONST_SRC_CODE_GRASS_FORCE0, zero);
    R_SetCodeConstantFromVec4(source, CONST_SRC_CODE_GRASS_FORCE1, zero);
    R_SetCodeConstantFromVec4(source, CONST_SRC_CODE_GRASS_WIND_FORCE0, zero);
}

void __cdecl R_SetCodeConstantFromVec4(GfxCmdBufSourceState *source, CodeConstant constant, const float *value)
{
    bcassert(constant, CONST_SRC_CODE_COUNT_FLOAT4);

    R_SetCodeConstant(source,
        constant,
        value[0],
        value[1],
        value[2],
        value[3]
    );
}

void __cdecl R_DirtyCodeConstant(GfxCmdBufSourceState *source, CodeConstant constant)
{
    bcassert(constant, ARRAY_COUNT(source->constVersions));

    ++source->constVersions[constant];
}

void __cdecl R_DynSModelInitGfxState(DynSModelGfxState *dynData)
{
    memset(dynData, 0, sizeof(DynSModelGfxState));
}

int __cdecl dyn_smodel_drawstateCallback(jqBatch *batch)
{
    DynSModelDrawStateCmd *cmd; // [esp+14h] [ebp-4h]

    cmd = (DynSModelDrawStateCmd *)jqLockData(batch);

    PROF_SCOPED("dyn_smodel_drawstate");

    R_CalcVisDynSModelDrawState(
        cmd->smodelList,
        cmd->listBegin,
        cmd->listEnd,
        cmd->frameInfo,
        cmd->view,
        cmd->dynData,
        cmd->persistForceIndexOffset,
        cmd->instantForceIndexOffset,
        cmd->boundsInfo);
    jqUnlockData(batch);
    return 0;
}

void __cdecl R_CalcVisDynSModelDrawState(
                const unsigned __int16 *modelIndexList,
                unsigned int listBegin,
                unsigned int listEnd,
                const PerFrameFoliageInfo *frameInfo,
                DynSModelClientView *view,
                DynSModelGfxState *dynData,
                unsigned int persistForceIndexOffset,
                unsigned int instantForceIndexOffset,
                const ForceBoundsInfo *boundsInfo)
{
    float WindBestStrength; // [esp+8h] [ebp-44h]
    DynSModelDrawState *v10; // [esp+Ch] [ebp-40h]
    int newIndex; // [esp+10h] [ebp-3Ch]
    int iforceIndex; // [esp+18h] [ebp-34h]
    unsigned int pforceIndex; // [esp+20h] [ebp-2Ch]
    float squash; // [esp+28h] [ebp-24h]
    unsigned __int16 drawStateIndex; // [esp+2Ch] [ebp-20h]
    const GfxStaticModelDrawInst *dpvsModelDrawInst; // [esp+30h] [ebp-1Ch]
    unsigned int forceIndices[2]; // [esp+34h] [ebp-18h]
    int numForces; // [esp+3Ch] [ebp-10h]
    const GfxStaticModelInst *dpvsModelInst; // [esp+40h] [ebp-Ch]
    unsigned int smodelIndex; // [esp+44h] [ebp-8h]
    unsigned int visModelIndex; // [esp+48h] [ebp-4h]

    for ( visModelIndex = listBegin; visModelIndex < listEnd; ++visModelIndex )
    {
        smodelIndex = modelIndexList[visModelIndex];
        if ( smodelIndex >= 0x800
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_foliage.cpp",
                        1427,
                        0,
                        "smodelIndex doesn't index MAX_NUM_DYNAMIC_SMODEL\n\t%i not in [0, %i)",
                        smodelIndex,
                        2048) )
        {
            __debugbreak();
        }
        squash = GetFoliageSquashFromHash(smodelIndex);
        forceIndices[0] = 0;
        forceIndices[1] = 0;
        numForces = 0;
        dpvsModelInst = &rgp.world->dpvs.smodelInsts[smodelIndex];
        if ( BoundsOverlap(
                     dpvsModelInst->mins,
                     dpvsModelInst->maxs,
                     (const float *)boundsInfo,
                     boundsInfo->persistForceBounds[1]) )
        {
            for ( pforceIndex = 0; pforceIndex < frameInfo->numGrassPersistantForces && numForces < 2; ++pforceIndex )
            {
                if ( AABBOverlapsSphere(
                             dpvsModelInst->mins,
                             dpvsModelInst->maxs,
                             frameInfo->grassPersistantForces[pforceIndex].radius
                         * frameInfo->grassPersistantForces[pforceIndex].radius,
                             frameInfo->grassPersistantForces[pforceIndex].worldPosition) )
                {
                    forceIndices[numForces++] = persistForceIndexOffset + pforceIndex;
                }
            }
        }
        if ( BoundsOverlap(
                     dpvsModelInst->mins,
                     dpvsModelInst->maxs,
                     boundsInfo->instantForceBounds[0],
                     boundsInfo->instantForceBounds[1]) )
        {
            for ( iforceIndex = 0; iforceIndex < (signed int)frameInfo->numGrassInstantForces && numForces < 2; ++iforceIndex )
            {
                if ( AABBOverlapsSphere(
                             dpvsModelInst->mins,
                             dpvsModelInst->maxs,
                             frameInfo->grassInstantForces[iforceIndex].radius * frameInfo->grassInstantForces[iforceIndex].radius,
                             frameInfo->grassInstantForces[iforceIndex].worldPosition) )
                {
                    forceIndices[numForces++] = instantForceIndexOffset + iforceIndex;
                }
            }
        }
        dpvsModelDrawInst = &rgp.world->dpvs.smodelDrawInsts[smodelIndex];
        if ( PointInBounds2D(
                     dpvsModelDrawInst->placement.origin,
                     boundsInfo->windForceBounds[0],
                     boundsInfo->windForceBounds[1]) )
        {
            WindBestStrength = R_GetWindBestStrength(
                                                     frameInfo->grassWindForces,
                                                     frameInfo->numGrassWindForces,
                                                     dpvsModelDrawInst->placement.origin);
        }
        else
        {
            WindBestStrength = 0.0f;
        }
        if ( squash == 0.0 && !numForces && WindBestStrength == 0.0 )
        {
            drawStateIndex = -1;
        }
        else
        {
            newIndex = _InterlockedExchangeAdd(&dynData->drawStateCount, 1u);
            if ( (unsigned int)newIndex >= 0x200 )
            {
                _InterlockedExchangeAdd(&dynData->drawStateCount, 0xFFFFFFFF);
                newIndex = -1;
            }
            if ( newIndex >= 0 )
            {
                drawStateIndex = newIndex;
                v10 = &dynData->drawState[(unsigned __int16)newIndex];
                v10->squash = squash;
                v10->force0Index = forceIndices[0];
                v10->force1Index = forceIndices[1];
                v10->windForceStrength = WindBestStrength;
            }
            else
            {
                drawStateIndex = -1;
                R_WarnOncePerFrame(R_WARN_DYN_SMODEL_MAX_DRAWSTATES, 512);
            }
        }
        view->drawStateIndex[smodelIndex] = drawStateIndex;
    }
}

void __cdecl R_DynSModelWaitWorker()
{
    PROF_SCOPED("R_DynSModelWaitWorker");
    Sys_AssistAndWaitWorkerCmdInternal(&dyn_smodel_drawstateWorkerCmd);
    g_drawStateWorkerSharedBuffer.inUse = 0;
}

void __cdecl R_DynSModelBuildClientView(
                const DynSModelClient *clientVis,
                DynSModelClientView *view,
                DynSModelGfxState *dynData)
{
    DWORD v4; // eax
    unsigned int v5; // [esp+8h] [ebp-218h]
    DynSModelDrawStateCmd cmd; // [esp+30h] [ebp-1F0h] BYREF
    unsigned int bitIndex; // [esp+54h] [ebp-1CCh]
    unsigned int indexLow; // [esp+58h] [ebp-1C8h]
    unsigned int smodelIndex; // [esp+5Ch] [ebp-1C4h]
    unsigned int visBits; // [esp+60h] [ebp-1C0h]
    int k; // [esp+64h] [ebp-1BCh]
    int uintIndex; // [esp+68h] [ebp-1B8h]
    float radius; // [esp+6Ch] [ebp-1B4h]
    float v14; // [esp+70h] [ebp-1B0h]
    float v15; // [esp+74h] [ebp-1ACh]
    const GrassWindForce *wForce; // [esp+78h] [ebp-1A8h]
    float v17[2]; // [esp+7Ch] [ebp-1A4h] BYREF
    float v18[2]; // [esp+84h] [ebp-19Ch] BYREF
    unsigned int j; // [esp+8Ch] [ebp-194h]
    float *v20; // [esp+90h] [ebp-190h]
    float v21; // [esp+94h] [ebp-18Ch]
    float v22; // [esp+98h] [ebp-188h]
    float v23; // [esp+9Ch] [ebp-184h]
    float *worldPosition; // [esp+A0h] [ebp-180h]
    float addedmins[3]; // [esp+A4h] [ebp-17Ch] BYREF
    float addedmaxs[3]; // [esp+B0h] [ebp-170h] BYREF
    unsigned int v27; // [esp+BCh] [ebp-164h]
    float (*constant)[4]; // [esp+C0h] [ebp-160h]
    float ext[3]; // [esp+C4h] [ebp-15Ch]
    const GrassForce *gforce; // [esp+D0h] [ebp-150h]
    float forceBounds[2][3]; // [esp+D4h] [ebp-14Ch] BYREF
    unsigned int i; // [esp+ECh] [ebp-134h]
    unsigned int forceConstantIndex; // [esp+F0h] [ebp-130h]
    int localClientNum; // [esp+F4h] [ebp-12Ch]
    unsigned int instantForceIndexOffset; // [esp+F8h] [ebp-128h]
    unsigned __int16 *visibleDynSModel; // [esp+FCh] [ebp-124h]
    ForceBoundsInfo *bounds; // [esp+100h] [ebp-120h]
    unsigned int visibleDynSModelCount; // [esp+104h] [ebp-11Ch]
    unsigned int persistForceIndexOffset; // [esp+108h] [ebp-118h]
    unsigned int visBitsAll[64]; // [esp+110h] [ebp-110h]
    unsigned int visSModelIndex; // [esp+210h] [ebp-10h]
    float forceFrequency[2]; // [esp+214h] [ebp-Ch]
    PerFrameFoliageInfo *frameInfo; // [esp+21Ch] [ebp-4h]

    PROF_SCOPED("R_DynSModelBuildClientView");

    localClientNum = clientVis->localClientNum;
    R_DynSModelWaitWorker();
    if ( g_drawStateWorkerSharedBuffer.inUse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_foliage.cpp",
                    1565,
                    0,
                    "%s",
                    "g_drawStateWorkerSharedBuffer.inUse == false") )
    {
        __debugbreak();
    }
    g_drawStateWorkerSharedBuffer.inUse = 1;
    frameInfo = &g_drawStateWorkerSharedBuffer.frameInfo;
    bounds = &g_drawStateWorkerSharedBuffer.forceBounds;
    memcpy(
        &g_drawStateWorkerSharedBuffer.frameInfo,
        &g_foliageInfo[localClientNum].perFrame[g_mainThreadFoliageInfo],
        sizeof(g_drawStateWorkerSharedBuffer.frameInfo));
    R_FoliageSetPerFramePersistantForces(&g_drawStateWorkerSharedBuffer.frameInfo, &g_foliageInfo[localClientNum]);
    forceFrequency[0] = 0.0;
    forceFrequency[1] = 0.0;
    persistForceIndexOffset = 1;
    instantForceIndexOffset = 9;
    view->forceFrequency[0] = 0.0f;
    view->grassForceConstants[0][0] = 0.0f;
    view->grassForceConstants[0][1] = 0.0f;
    view->grassForceConstants[0][2] = 0.0f;
    view->grassForceConstants[0][3] = 0.0f;
    ClearBounds((float *)bounds, bounds->persistForceBounds[1]);
    forceConstantIndex = 1;
    i = 0;
    while ( i < frameInfo->numGrassPersistantForces )
    {
        gforce = &frameInfo->grassPersistantForces[i];
        if ( (!forceConstantIndex || forceConstantIndex > 8)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_foliage.cpp",
                        1598,
                        0,
                        "forceConstantIndex not in [persistForceIndexOffset, persistForceIndexOffset + PerFrameFoliageInfo::MAX_PERSI"
                        "STANT_GRASS_FORCES - 1]\n"
                        "\t%i not in [%i, %i]",
                        forceConstantIndex,
                        1,
                        8) )
        {
            __debugbreak();
        }
        if ( forceConstantIndex >= 0x19
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_foliage.cpp",
                        1599,
                        0,
                        "forceConstantIndex doesn't index ARRAY_COUNT(view->grassForceConstants)\n\t%i not in [0, %i)",
                        forceConstantIndex,
                        25) )
        {
            __debugbreak();
        }
        constant = (float (*)[4])view->grassForceConstants[forceConstantIndex];
        (*constant)[0] = gforce->worldPosition[0];
        (*constant)[1] = gforce->worldPosition[1];
        (*constant)[2] = gforce->worldPosition[2];
        (*constant)[3] = 0.0f;
        (*constant)[2] = gforce->strength;
        (*constant)[3] = gforce->radius * gforce->radius;
        if ( forceConstantIndex >= 0x19
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_foliage.cpp",
                        1606,
                        0,
                        "forceConstantIndex doesn't index ARRAY_COUNT(view->forceFrequency)\n\t%i not in [0, %i)",
                        forceConstantIndex,
                        25) )
        {
            __debugbreak();
        }
        view->forceFrequency[forceConstantIndex] = gforce->frequency;
        ext[0] = gforce->radius;
        ext[1] = gforce->radius;
        ext[2] = gforce->radius;
        forceBounds[0][0] = gforce->worldPosition[0] - ext[0];
        forceBounds[0][1] = gforce->worldPosition[1] - ext[1];
        forceBounds[0][2] = gforce->worldPosition[2] - ext[2];
        forceBounds[1][0] = gforce->worldPosition[0] + ext[0];
        forceBounds[1][1] = gforce->worldPosition[1] + ext[1];
        forceBounds[1][2] = gforce->worldPosition[2] + ext[2];
        ExpandBounds(forceBounds[0], forceBounds[1], (float *)bounds, bounds->persistForceBounds[1]);
        ++i;
        ++forceConstantIndex;
    }
    ClearBounds(bounds->instantForceBounds[0], bounds->instantForceBounds[1]);
    forceConstantIndex = 9;
    v27 = 0;
    while ( v27 < frameInfo->numGrassInstantForces )
    {
        worldPosition = frameInfo->grassInstantForces[v27].worldPosition;
        if ( (forceConstantIndex < 9 || forceConstantIndex > 0x18)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_foliage.cpp",
                        1624,
                        0,
                        "forceConstantIndex not in [instantForceIndexOffset, ARRAY_COUNT(view->grassForceConstants) - 1]\n"
                        "\t%i not in [%i, %i]",
                        forceConstantIndex,
                        9,
                        24) )
        {
            __debugbreak();
        }
        if ( forceConstantIndex >= 0x19
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_foliage.cpp",
                        1625,
                        0,
                        "forceConstantIndex doesn't index ARRAY_COUNT(view->grassForceConstants)\n\t%i not in [0, %i)",
                        forceConstantIndex,
                        25) )
        {
            __debugbreak();
        }
        v20 = view->grassForceConstants[forceConstantIndex];
        *v20 = *worldPosition;
        v20[1] = worldPosition[1];
        v20[2] = worldPosition[2];
        v20[3] = 0.0f;
        v20[2] = worldPosition[4];
        v20[3] = worldPosition[3] * worldPosition[3];
        if ( forceConstantIndex >= 0x19
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_foliage.cpp",
                        1632,
                        0,
                        "forceConstantIndex doesn't index ARRAY_COUNT(view->forceFrequency)\n\t%i not in [0, %i)",
                        forceConstantIndex,
                        25) )
        {
            __debugbreak();
        }
        view->forceFrequency[forceConstantIndex] = 0.0f;
        v21 = worldPosition[3];
        v22 = worldPosition[3];
        v23 = worldPosition[3];
        addedmins[0] = *worldPosition - v21;
        addedmins[1] = worldPosition[1] - v22;
        addedmins[2] = worldPosition[2] - v23;
        addedmaxs[0] = *worldPosition + v21;
        addedmaxs[1] = worldPosition[1] + v22;
        addedmaxs[2] = worldPosition[2] + v23;
        ExpandBounds(addedmins, addedmaxs, bounds->instantForceBounds[0], bounds->instantForceBounds[1]);
        ++v27;
        ++forceConstantIndex;
    }
    ClearBounds2D(bounds->windForceBounds[0], bounds->windForceBounds[1]);
    for ( j = 0; j < frameInfo->numGrassWindForces; ++j )
    {
        wForce = &frameInfo->grassWindForces[j];
        radius = sqrtf(1.0 / frameInfo->grassWindForces[j].inverseRadiusSquared);
        v14 = radius;
        v15 = radius;
        v17[0] = wForce->worldPosition[0] - radius;
        v17[1] = frameInfo->grassWindForces[j].worldPosition[1] - radius;
        v18[0] = wForce->worldPosition[0] + radius;
        v18[1] = frameInfo->grassWindForces[j].worldPosition[1] + radius;
        ExpandBounds2D(v17, v18, bounds->windForceBounds[0], bounds->windForceBounds[1]);
    }
    dynData->maxDynSModelIndex = clientVis->numDynSModel;
    dynData->maxDynSModelIndexInMap = clientVis->numDynSModelInMap;
    for ( uintIndex = 0; uintIndex < 64; ++uintIndex )
        visBitsAll[uintIndex] = clientVis->visBitsSunShadow[uintIndex] | clientVis->visBitsCamera[uintIndex];
    visibleDynSModel = (unsigned __int16 *)&g_drawStateWorkerSharedBuffer;
    visibleDynSModelCount = 0;
    for ( k = 0; k < 64; ++k )
    {
        visBits = visBitsAll[k];
        if ( visBits )
        {
            while ( 1 )
            {
                if (!_BitScanReverse(&v4, visBits))
                    v4 = 63;// `CountLeadingZeros'::`2': : notFound;
                indexLow = v4 ^ 0x1F;
                if (indexLow >= 32 )
                    break;
                bitIndex = 31 - indexLow;
                visBits &= ~(1 << (31 - indexLow));
                smodelIndex = 31 - indexLow + 32 * k;
                if ( visibleDynSModelCount >= 0x800 )
                {
                    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_foliage.cpp", 1700, 0, "%s", "0") )
                        __debugbreak();
                }
                else
                {
                    if ( smodelIndex >= 0x10000
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_foliage.cpp",
                                    1695,
                                    0,
                                    "%s",
                                    "smodelIndex < ( 1 << ( sizeof(visibleDynSModel[0]) * 8 ) )") )
                    {
                        __debugbreak();
                    }
                    visibleDynSModel[visibleDynSModelCount++] = smodelIndex;
                }
            }
        }
    }
    memset(view, 0xFFu, 0x1000u);
    for ( visSModelIndex = 0; visSModelIndex < visibleDynSModelCount; visSModelIndex += v5 )
    {
        if ( (int)(visibleDynSModelCount - visSModelIndex) > 256 )
            v5 = 256;
        else
            v5 = visibleDynSModelCount - visSModelIndex;
        cmd.frameInfo = frameInfo;
        if ( visibleDynSModel != (unsigned __int16 *)&g_drawStateWorkerSharedBuffer
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_foliage.cpp",
                        1721,
                        0,
                        "%s",
                        "visibleDynSModel == g_drawStateWorkerSharedBuffer.visibleDynSModelList") )
        {
            __debugbreak();
        }
        cmd.smodelList = visibleDynSModel;
        cmd.listBegin = visSModelIndex;
        cmd.listEnd = v5 + visSModelIndex;
        cmd.view = view;
        cmd.dynData = dynData;
        cmd.persistForceIndexOffset = 1;
        cmd.instantForceIndexOffset = 9;
        cmd.boundsInfo = bounds;
        Sys_AddWorkerCmdInternal(&dyn_smodel_drawstateWorkerCmd, (unsigned __int8 *)&cmd, 0);
    }
}

void __cdecl R_FoliageSetStaticModelShaderConstants(
                GfxCmdBufContext *context,
                unsigned int smodelDrawIndex,
                const DynSModelClientView *smodelClientView,
                const DynSModelGfxState *smodelState)
{
    int v4; // eax
    int v5; // eax
    int v6; // eax
    int v7; // eax
    int v8; // eax
    int force1Index; // eax
    int v10; // eax
    int v11; // eax
    int v12; // eax
    int v13; // eax
    int v14; // eax
    int updated; // eax
    int v16; // eax
    int v17; // eax
    int v18; // eax
    int v19; // eax
    float v20; // [esp+44h] [ebp-A8h]
    float windForceStrength; // [esp+48h] [ebp-A4h]
    const float *v22; // [esp+50h] [ebp-9Ch]
    float value[4]; // [esp+68h] [ebp-84h] BYREF
    const DynSModelDrawState *drawState; // [esp+78h] [ebp-74h]
    FoliageShaderConstantBlock constBlock; // [esp+7Ch] [ebp-70h] BYREF
    float defaultGrassParm[4]; // [esp+C0h] [ebp-2Ch] BYREF
    int v27; // [esp+D0h] [ebp-1Ch]
    int updatedConstant; // [esp+D4h] [ebp-18h]
    float zero[4]; // [esp+D8h] [ebp-14h] BYREF
    unsigned __int16 drawStateIndex; // [esp+E8h] [ebp-4h]

    if ( !smodelState
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_foliage.cpp", 1758, 0, "%s", "smodelState") )
    {
        __debugbreak();
    }
    if ( smodelDrawIndex >= smodelState->maxDynSModelIndexInMap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_foliage.cpp",
                    1759,
                    0,
                    "%s",
                    "smodelDrawIndex < smodelState->maxDynSModelIndexInMap") )
    {
        __debugbreak();
    }
    memset(zero, 0, sizeof(zero));
    if ( smodelDrawIndex < smodelState->maxDynSModelIndex )
    {
        if ( !smodelClientView
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_foliage.cpp",
                        1779,
                        0,
                        "%s",
                        "smodelClientView") )
        {
            __debugbreak();
        }
        if ( smodelDrawIndex >= 0x800
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_foliage.cpp",
                        1780,
                        0,
                        "%s",
                        "smodelDrawIndex < MAX_NUM_DYNAMIC_SMODEL") )
        {
            __debugbreak();
        }
        drawStateIndex = smodelClientView->drawStateIndex[smodelDrawIndex];
        updatedConstant = 0;
        if ( drawStateIndex == 0xFFFF )
        {
            value[0] = 1.0f;
            value[1] = 0.0f;
            value[2] = 1.0f;
            value[3] = 1.0f;
            updated = R_UpdateCodeConstantFromVec4(context->source, CONST_SRC_CODE_GRASS_PARMS, value);
            updatedConstant |= updated;
            v16 = R_UpdateCodeConstantFromVec4(context->source, CONST_SRC_CODE_GRASS_FORCE0, zero);
            updatedConstant |= v16;
            v17 = R_UpdateCodeConstantFromVec4(context->source, CONST_SRC_CODE_GRASS_FORCE1, zero);
            updatedConstant |= v17;
            v18 = R_UpdateCodeConstantFromVec4(context->source, CONST_SRC_CODE_GRASS_WIND_FORCE0, zero);
            updatedConstant |= v18;
            v19 = R_UpdateCodeConstantFromVec4(context->source, CONST_SRC_CODE_CHARACTER_CHARRED_AMOUNT, zero);
            updatedConstant |= v19;
        }
        else
        {
            if ( drawStateIndex >= smodelState->drawStateCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_foliage.cpp",
                            1787,
                            0,
                            "%s",
                            "drawStateIndex < smodelState->drawStateCount") )
            {
                __debugbreak();
            }
            drawState = &smodelState->drawState[drawStateIndex];
            if ( smodelState->drawState[drawStateIndex].force0Index >= 0x19u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_foliage.cpp",
                            1790,
                            0,
                            "%s",
                            "drawState->force0Index < ARRAY_COUNT(smodelClientView->grassForceConstants)") )
            {
                __debugbreak();
            }
            if ( drawState->force1Index >= 0x19u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_foliage.cpp",
                            1791,
                            0,
                            "%s",
                            "drawState->force1Index < ARRAY_COUNT(smodelClientView->grassForceConstants)") )
            {
                __debugbreak();
            }
            R_GetGrassParmShaderConstant(drawState->squash, constBlock.grassParms);
            v22 = smodelClientView->grassForceConstants[drawState->force0Index];
            constBlock.grassForce0[0] = *v22;
            constBlock.grassForce0[1] = v22[1];
            constBlock.grassForce0[2] = v22[2];
            constBlock.grassForce0[3] = v22[3];
            force1Index = drawState->force1Index;
            constBlock.grassForce1[0] = smodelClientView->grassForceConstants[force1Index][0];
            constBlock.grassForce1[1] = smodelClientView->grassForceConstants[force1Index][1];
            constBlock.grassForce1[2] = smodelClientView->grassForceConstants[force1Index][2];
            constBlock.grassForce1[3] = smodelClientView->grassForceConstants[force1Index][3];
            v20 = smodelClientView->forceFrequency[drawState->force1Index];
            windForceStrength = drawState->windForceStrength;
            constBlock.windForce0[0] = smodelClientView->forceFrequency[drawState->force0Index];
            constBlock.windForce0[1] = v20;
            constBlock.windForce0[2] = windForceStrength;
            constBlock.windForce0[3] = 0.0f;
            v10 = R_UpdateCodeConstantFromVec4(context->source, CONST_SRC_CODE_GRASS_PARMS, constBlock.grassParms);
            updatedConstant |= v10;
            v11 = R_UpdateCodeConstantFromVec4(context->source, CONST_SRC_CODE_GRASS_FORCE0, constBlock.grassForce0);
            updatedConstant |= v11;
            v12 = R_UpdateCodeConstantFromVec4(context->source, CONST_SRC_CODE_GRASS_FORCE1, constBlock.grassForce1);
            updatedConstant |= v12;
            v13 = R_UpdateCodeConstantFromVec4(context->source, CONST_SRC_CODE_GRASS_WIND_FORCE0, constBlock.windForce0);
            updatedConstant |= v13;
            v14 = R_UpdateCodeConstant(context->source, CONST_SRC_CODE_CHARACTER_CHARRED_AMOUNT, drawState->squash / 0.5, 0.0, 0.0, 0.0);
            updatedConstant |= v14;
        }
        if ( updatedConstant )
            R_SetupPassVertexShaderArgs(*context);
    }
    else
    {
        v27 = 0;
        defaultGrassParm[0] = 1.0f;
        defaultGrassParm[1] = 0.0f;
        defaultGrassParm[2] = 1.0f;
        defaultGrassParm[3] = 1.0f;
        v4 = R_UpdateCodeConstantFromVec4(context->source, CONST_SRC_CODE_GRASS_PARMS, defaultGrassParm);
        v27 |= v4;
        v5 = R_UpdateCodeConstantFromVec4(context->source, CONST_SRC_CODE_GRASS_FORCE0, zero);
        v27 |= v5;
        v6 = R_UpdateCodeConstantFromVec4(context->source, CONST_SRC_CODE_GRASS_FORCE1, zero);
        v27 |= v6;
        v7 = R_UpdateCodeConstantFromVec4(context->source, CONST_SRC_CODE_GRASS_WIND_FORCE0, zero);
        v27 |= v7;
        v8 = R_UpdateCodeConstantFromVec4(context->source, CONST_SRC_CODE_CHARACTER_CHARRED_AMOUNT, zero);
        v27 |= v8;
        if ( v27 )
            R_SetupPassVertexShaderArgs(*context);
    }
}

int __cdecl R_UpdateCodeConstant(
                GfxCmdBufSourceState *source,
                CodeConstant constant,
                float x,
                float y,
                float z,
                float w)
{
    if ( source->input.consts[constant][0] == x
        && source->input.consts[constant][1] == y
        && source->input.consts[constant][2] == z
        && source->input.consts[constant][3] == w )
    {
        return 0;
    }
    R_SetCodeConstant(source, constant, x, y, z, w);
    return 1;
}

void __cdecl R_SetCodeConstant(GfxCmdBufSourceState *source, CodeConstant constant, float x, float y, float z, float w)
{
    bcassert(constant, CONST_SRC_CODE_COUNT_FLOAT4);

    source->input.consts[constant][0] = x;
    source->input.consts[constant][1] = y;
    source->input.consts[constant][2] = z;
    source->input.consts[constant][3] = w;

    R_DirtyCodeConstant(source, constant);
}

int __cdecl R_UpdateCodeConstantFromVec4(GfxCmdBufSourceState *source, CodeConstant constant, float *value)
{
    if ( source->input.consts[constant][0] == *value
        && source->input.consts[constant][1] == value[1]
        && source->input.consts[constant][2] == value[2]
        && source->input.consts[constant][3] == value[3] )
    {
        return 0;
    }
    R_SetCodeConstantFromVec4(source, constant, value);
    return 1;
}

void __cdecl R_GetGrassParmShaderConstant(float curSquashAmt, float *out)
{
    *out = 1.0 - curSquashAmt;
    out[1] = curSquashAmt / 0.5;
    out[2] = (float)(1.0 - curSquashAmt) * (float)(1.0 - curSquashAmt);
    out[3] = COLOR_MULTIPLIER_AT_FULLSQUASH;
}

void __cdecl R_DynSModelGetCounts(unsigned int *numDynSModelInMap, unsigned int *maxNumDynSModel)
{
    *numDynSModelInMap = g_numDynSModelInMap;
    *maxNumDynSModel = 2048;
}

ClientFoliageInfo::ClientFoliageInfo()
{
    int v2; // [esp+8h] [ebp-10h]
    GrassPersistForce *j; // [esp+Ch] [ebp-Ch]
    int v4; // [esp+10h] [ebp-8h]
    ClientFoliageInfo *i; // [esp+14h] [ebp-4h]

    v4 = 2;
    for ( i = this; --v4 >= 0; i = (ClientFoliageInfo *)((char *)i + 844) )
    {
        i->perFrame[0].numGrassInstantForces = 0;
        i->perFrame[0].numGrassPersistantForces = 0;
        i->perFrame[0].numGrassWindForces = 0;
    }
    v2 = 8;
    for ( j = this->persistantForces; --v2 >= 0; ++j )
    {
        j->age = 0.0f;
        j->maxAge = 0.0f;
        j->isActive = 0;
    }
    this->clientIsActive = 0;
    this->clientPlayerPos[0] = 0.0f;
    this->clientPlayerPos[1] = 0.0f;
    this->clientPlayerPos[2] = 0.0f;
    //return this;
}

