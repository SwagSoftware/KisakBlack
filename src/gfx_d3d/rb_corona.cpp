#include "rb_corona.h"
#include "rb_pixelcost.h"
#include "rb_shade.h"
#include "rb_stats.h"
#include "rb_logfile.h"
#include "r_dvars.h"
#include "r_stream.h"
#include <qcommon/common.h>

CoronaState coronaState[4];
int coronaTimeLastUpdated;

void __cdecl RB_DrawCoronas(unsigned int localClientNum)
{
    const Material *coronaMaterial; // [esp+0h] [ebp-54h]
    Corona *v3; // [esp+18h] [ebp-3Ch]
    int m; // [esp+1Ch] [ebp-38h]
    int k; // [esp+24h] [ebp-30h]
    int j; // [esp+2Ch] [ebp-28h]
    int i; // [esp+34h] [ebp-20h]
    GfxLightCorona *info; // [esp+38h] [ebp-1Ch]
    int n; // [esp+40h] [ebp-14h]
    CoronaState *state; // [esp+44h] [ebp-10h]
    int frameTime; // [esp+4Ch] [ebp-8h]

    PROF_SCOPED("RB_DrawCoronas");

    if (localClientNum >= gfxCfg.maxClientViews
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_corona.cpp",
            439,
            0,
            "localClientNum doesn't index gfxCfg.maxClientViews\n\t%i not in [0, %i)",
            localClientNum,
            gfxCfg.maxClientViews))
    {
        __debugbreak();
    }
    state = &coronaState[localClientNum];
    if (pixelCostMode == GFX_PIXEL_COST_MODE_MEASURE_COST || pixelCostMode == GFX_PIXEL_COST_MODE_MEASURE_MSEC)
    {
        return;
    }
    if (coronaTimeLastUpdated && coronaTimeLastUpdated <= gfxCmdBufSourceState.sceneDef.time)
        frameTime = gfxCmdBufSourceState.sceneDef.time - coronaTimeLastUpdated;
    else
        frameTime = 10;
    coronaTimeLastUpdated = gfxCmdBufSourceState.sceneDef.time;
    if (tess.indexCount)
        RB_EndTessSurface();
    if (state->numPending < 0x10)
    {
        for (n = 0; n < 16; ++n)
        {
            if (!state->pending[n].inUse)
            {
                info = RB_FindBestCoronaToSpawn(state);
                if (info)
                {
                    ++state->numPending;
                    RB_SpawnCorona(&state->pending[n], info);
                }
            }
        }
    }
    if (rgp.world->coronaMaterial)
        coronaMaterial = rgp.world->coronaMaterial;
    else
        coronaMaterial = rgp.lightCoronaMaterial;
    for (i = 0; i < 16; ++i)
        RB_DrawCorona(&state->active[i], frameTime, coronaMaterial);
    for (j = 0; j < 16; ++j)
        RB_DrawCorona(&state->pending[j], frameTime, coronaMaterial);
    if (state->numActive == 16)
    {
        for (k = 0; k < 16; ++k)
        {
            if (state->active[k].inUse && state->active[k].targetVisibility <= 0.0)
            {
                RB_UnspawnCorona(&state->active[k]);
                --state->numActive;
                break;
            }
        }
    }
    for (m = 0; m < 16; ++m)
    {
        v3 = &state->pending[m];
        if (v3->inUse && state->pending[m].timeSinceSpawned > 200 && state->numActive < 0x10)
        {
            if (state->pending[m].wasEverVisible)
            {
                RB_PromoteCorona(state, v3);
                --state->numPending;
                ++state->numActive;
            }
            else
            {
                RB_UnspawnCorona(v3);
                --state->numPending;
            }
        }
    }
}

unsigned int id;
GfxLightCorona *__cdecl RB_FindBestCoronaToSpawn(CoronaState *state)
{
    int j; // [esp+4h] [ebp-10h]
    int i; // [esp+Ch] [ebp-8h]
    GfxLightCorona *info; // [esp+10h] [ebp-4h]

    if ( ++id >= rgp.world->coronaCount )
        id = 0;
    info = &rgp.world->coronas[id];
    for ( i = 0; i < 16; ++i )
    {
        if ( state->active[i].inUse && state->active[i].info == info )
            return 0;
    }
    for ( j = 0; j < 16; ++j )
    {
        if ( state->pending[j].inUse && state->pending[j].info == info )
            return 0;
    }
    return &rgp.world->coronas[id];
}

void __cdecl RB_SpawnCorona(Corona *corona, GfxLightCorona *info)
{
    if ( corona->inUse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_corona.cpp",
                    360,
                    0,
                    "%s",
                    "corona->inUse == false") )
    {
        __debugbreak();
    }
    RB_ResetCorona(corona);
    corona->inUse = 1;
    corona->info = info;
    corona->timeSinceSpawned = 0;
}

void __cdecl RB_ResetCorona(Corona *corona)
{
    unsigned int queryIndex; // [esp+0h] [ebp-4h]

    corona->inUse = 0;
    corona->info = 0;
    corona->hitNum = 0;
    corona->timeSinceSpawned = 0;
    corona->currentVisibility = 0.0f;
    corona->targetVisibility = 0.0f;
    corona->wasEverVisible = 0;
    for ( queryIndex = 0; queryIndex < 5; ++queryIndex )
        corona->queryIssued[queryIndex] = 0;
}

void __cdecl RB_UnspawnCorona(Corona *corona)
{
    if ( !corona->inUse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_corona.cpp",
                    371,
                    0,
                    "%s",
                    "corona->inUse == true") )
    {
        __debugbreak();
    }
    corona->inUse = 0;
}

void __cdecl RB_PromoteCorona(CoronaState *state, Corona *corona)
{
    Corona *newSlot; // [esp+0h] [ebp-8h]
    int n; // [esp+4h] [ebp-4h]

    for ( n = 0; n < 16; ++n )
    {
        newSlot = &state->active[n];
        if ( !newSlot->inUse )
        {
            RB_ResetCorona(newSlot);
            newSlot->inUse = 1;
            state->active[n].info = corona->info;
            state->active[n].hitNum = corona->hitNum;
            state->active[n].timeSinceSpawned = corona->timeSinceSpawned;
            state->active[n].currentVisibility = corona->currentVisibility;
            state->active[n].targetVisibility = corona->targetVisibility;
            state->active[n].wasEverVisible = corona->wasEverVisible;
            corona->inUse = 0;
            return;
        }
    }
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_corona.cpp", 399, 0, "oh") )
        __debugbreak();
}

void __cdecl RB_DrawCorona(Corona *corona, int frameTime, const Material *coronaMat)
{
    float scale; // [esp+4Ch] [ebp-8h]

    if ( corona->inUse )
    {
        corona->timeSinceSpawned += frameTime;
        corona->currentVisibility = R_UpdateOverTime(corona->currentVisibility, corona->targetVisibility, frameTime);
        if ( corona->targetVisibility > 0.0 )
            corona->wasEverVisible = 1;
        RB_SetIdentity();
        RB_DrawCoronaQuerySprite(corona);
        scale = (float)(corona->currentVisibility * corona->info->intensity) * 255.0;
        RB_DrawCoronaSprite(
            corona,
            coronaMat,
            (GfxColor)((unsigned __int8)SnapFloatToInt((float)(corona->info->color[2] * scale))
                             | ((unsigned __int8)SnapFloatToInt((float)(corona->info->color[1] * scale)) << 8)
                             | ((unsigned __int8)SnapFloatToInt((float)(corona->info->color[0] * scale)) << 16)
                             | 0xFF000000),
            corona->info->radius,
            0);
        if ( tess.indexCount )
            RB_EndTessSurface();
    }
}

double __cdecl R_UpdateOverTime(float fCurrent, float fGoal, int frametime)
{
    if ( frametime < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_corona.cpp",
                    70,
                    0,
                    "%s\n\t(frametime) = %i",
                    "(frametime >= 0)",
                    frametime) )
    {
        __debugbreak();
    }
    if ( !frametime )
        return fGoal;
    if ( fGoal <= fCurrent )
    {
        if ( fCurrent > fGoal )
        {
            fCurrent = fCurrent - (float)((float)((float)frametime * 1.0) / 100.0);
            if ( fGoal > fCurrent )
                return fGoal;
        }
    }
    else
    {
        fCurrent = (float)((float)((float)frametime * 1.0) / 100.0) + fCurrent;
        if ( fCurrent > fGoal )
            return fGoal;
    }
    return fCurrent;
}

void __cdecl RB_DrawCoronaSprite(Corona *corona, const Material *material, GfxColor color, float radius, int query)
{
    RB_SetTessTechnique(material, 4u);
    R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_FX);
    RB_SetIdentity();
    RB_TessCoronaBillboard(corona, color, radius, query);
    RB_EndTessSurface();
}
void __cdecl RB_TessCoronaBillboard(Corona *corona, GfxColor color, float radius, int query)
{
    float v4; // [esp+0h] [ebp-8Ch]
    GfxVertex *v5; // [esp+30h] [ebp-5Ch]
    GfxVertex *v6; // [esp+34h] [ebp-58h]
    GfxVertex *v7; // [esp+38h] [ebp-54h]
    GfxLightCorona *info; // [esp+48h] [ebp-44h]
    float distance; // [esp+4Ch] [ebp-40h]
    float transformedPosition; // [esp+54h] [ebp-38h]
    float transformedPosition_4; // [esp+58h] [ebp-34h]
    float transformedPosition_8; // [esp+5Ch] [ebp-30h]
    float transformedPosition_12; // [esp+60h] [ebp-2Ch]
    float sizeY; // [esp+64h] [ebp-28h]
    float sizeX; // [esp+74h] [ebp-18h]
    float position[3]; // [esp+7Ch] [ebp-10h] BYREF
    GfxVertex *vert; // [esp+88h] [ebp-4h]

    info = corona->info;
    position[0] = info->origin[0];
    position[1] = info->origin[1];
    position[2] = info->origin[2];
    transformedPosition = (float)((float)((float)(position[0]
        * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[0][0])
        + (float)(position[1]
            * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[1][0]))
        + (float)(position[2] * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[2][0]))
        + gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[3][0];
    transformedPosition_4 = (float)((float)((float)(position[0]
        * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[0][1])
        + (float)(position[1]
            * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[1][1]))
        + (float)(position[2] * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[2][1]))
        + gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[3][1];
    transformedPosition_8 = (float)((float)((float)(position[0]
        * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[0][2])
        + (float)(position[1]
            * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[1][2]))
        + (float)(position[2] * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[2][2]))
        + gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[3][2];
    transformedPosition_12 = (float)((float)((float)(position[0]
        * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[0][3])
        + (float)(position[1]
            * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[1][3]))
        + (float)(position[2] * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[2][3]))
        + gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[3][3];
    sizeY = radius / transformedPosition_12;
    sizeX = (float)((float)gfxCmdBufSourceState.renderTargetHeight * (float)(radius / transformedPosition_12))
        / (float)gfxCmdBufSourceState.renderTargetWidth;
    if (rg.renderHiResShot)
    {
        sizeX = (float)rg.hiResShotTiles * sizeX;
        sizeY = (float)rg.hiResShotTiles * sizeY;
    }
    vert = RB_SetTessQuad(color);
    vert->xyzw[0] = transformedPosition;
    vert->xyzw[1] = transformedPosition_4;
    vert->xyzw[2] = transformedPosition_8;
    vert->xyzw[3] = transformedPosition_12;
    v7 = vert + 1;
    vert[1].xyzw[0] = transformedPosition;
    v7->xyzw[1] = transformedPosition_4;
    v7->xyzw[2] = transformedPosition_8;
    v7->xyzw[3] = transformedPosition_12;
    v6 = vert + 2;
    vert[2].xyzw[0] = transformedPosition;
    v6->xyzw[1] = transformedPosition_4;
    v6->xyzw[2] = transformedPosition_8;
    v6->xyzw[3] = transformedPosition_12;
    v5 = vert + 3;
    vert[3].xyzw[0] = transformedPosition;
    v5->xyzw[1] = transformedPosition_4;
    v5->xyzw[2] = transformedPosition_8;
    v5->xyzw[3] = transformedPosition_12;
    vert->xyzw[0] = vert->xyzw[0] + (float)(sizeX * transformedPosition_12);
    vert->xyzw[1] = vert->xyzw[1] + (float)(sizeY * transformedPosition_12);
    vert->xyzw[2] = vert->xyzw[2] + (float)(0.000049999999 * transformedPosition_12);
    vert[1].xyzw[0] = vert[1].xyzw[0] + (float)(sizeX * transformedPosition_12);
    vert[1].xyzw[1] = vert[1].xyzw[1] - (float)(sizeY * transformedPosition_12);
    vert[1].xyzw[2] = vert[1].xyzw[2] + (float)(0.000049999999 * transformedPosition_12);
    vert[2].xyzw[0] = vert[2].xyzw[0] - (float)(sizeX * transformedPosition_12);
    vert[2].xyzw[1] = vert[2].xyzw[1] - (float)(sizeY * transformedPosition_12);
    vert[2].xyzw[2] = vert[2].xyzw[2] + (float)(0.000049999999 * transformedPosition_12);
    vert[3].xyzw[0] = vert[3].xyzw[0] - (float)(sizeX * transformedPosition_12);
    vert[3].xyzw[1] = vert[3].xyzw[1] + (float)(sizeY * transformedPosition_12);
    vert[3].xyzw[2] = vert[3].xyzw[2] + (float)(0.000049999999 * transformedPosition_12);
    Vec4DivideByW(vert->xyzw, vert->xyzw);
    Vec4DivideByW(vert[1].xyzw, vert[1].xyzw);
    Vec4DivideByW(vert[2].xyzw, vert[2].xyzw);
    Vec4DivideByW(vert[3].xyzw, vert[3].xyzw);
    if (!query)
    {
        v4 = Vec3DistanceSq(gfxCmdBufSourceState.viewParms3D->origin, position);
        distance = sqrtf(v4);
        vert->xyzw[3] = distance;
        vert[1].xyzw[3] = distance;
        vert[2].xyzw[3] = distance;
        vert[3].xyzw[3] = distance;
    }
}

void __cdecl Vec4DivideByW(const float *in, float *out)
{
    float invW; // [esp+4h] [ebp-4h]

    if ( fabs(in[3]) > 0.001 )
    {
        invW = 1.0 / in[3];
        *out = *in * invW;
        out[1] = in[1] * invW;
        out[2] = in[2] * invW;
        out[3] = 1.0f;
    }
}

GfxVertex *__cdecl RB_SetTessQuad(GfxColor color)
{
    unsigned __int16 vertCount; // [esp+10h] [ebp-8h]
    GfxVertex *vert; // [esp+14h] [ebp-4h]

    if ( tess.vertexCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_corona.cpp",
                    110,
                    0,
                    "%s",
                    "tess.vertexCount == 0") )
    {
        __debugbreak();
    }
    if ( tess.indexCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_corona.cpp",
                    111,
                    0,
                    "%s",
                    "tess.indexCount == 0") )
    {
        __debugbreak();
    }
    vertCount = tess.vertexCount;
    tess.indices[tess.indexCount] = LOWORD(tess.vertexCount) + 3;
    tess.indices[tess.indexCount + 1] = vertCount;
    tess.indices[tess.indexCount + 2] = vertCount + 2;
    tess.indices[tess.indexCount + 3] = vertCount + 2;
    tess.indices[tess.indexCount + 4] = vertCount;
    tess.indices[tess.indexCount + 5] = vertCount + 1;
    vert = &tess.verts[vertCount];
    vert->normal.packed = 1073643391;
    vert->texCoord[0] = 0.0f;
    vert->texCoord[1] = 0.0f;
    vert->color = color;
    vert[1].normal.packed = 1073643391;
    vert[1].texCoord[0] = 1.0f;
    vert[1].texCoord[1] = 0.0f;
    vert[1].color = color;
    vert[2].normal.packed = 1073643391;
    vert[2].texCoord[0] = 1.0f;
    vert[2].texCoord[1] = 1.0f;
    vert[2].color = color;
    vert[3].normal.packed = 1073643391;
    vert[3].texCoord[0] = 0.0f;
    vert[3].texCoord[1] = 1.0f;
    vert[3].color = color;
    tess.vertexCount += 4;
    tess.indexCount += 6;
    return vert;
}

void __cdecl RB_DrawCoronaQuerySprite(Corona *corona)
{
    float v1; // [esp+10h] [ebp-50h]
    float v2; // [esp+30h] [ebp-30h]
    float v3; // [esp+34h] [ebp-2Ch]
    unsigned int numVisible; // [esp+48h] [ebp-18h]
    unsigned int totalPixels; // [esp+4Ch] [ebp-14h]
    float targetVisibility; // [esp+50h] [ebp-10h]
    int error; // [esp+54h] [ebp-Ch]
    unsigned int queryIndex; // [esp+5Ch] [ebp-4h]

    if ( !rg.renderHiResShot )
    {
        if ( (!corona || !corona->inUse)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_corona.cpp",
                        247,
                        0,
                        "%s",
                        "corona && corona->inUse") )
        {
            __debugbreak();
        }
        queryIndex = r_glob.backEndFrameCount % (dx.gpuCount + 1);
        if ( queryIndex >= 5
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_corona.cpp",
                        254,
                        0,
                        "queryIndex doesn't index ARRAY_COUNT( corona->query )\n\t%i not in [0, %i)",
                        queryIndex,
                        5) )
        {
            __debugbreak();
        }
        if ( queryIndex >= 5
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_corona.cpp",
                        255,
                        0,
                        "queryIndex doesn't index ARRAY_COUNT( corona->queryMax )\n\t%i not in [0, %i)",
                        queryIndex,
                        5) )
        {
            __debugbreak();
        }
        if ( corona->query[queryIndex] && corona->queryMax[queryIndex] )
        {
            error = 0;
            if ( corona->queryIssued[queryIndex] )
            {
                numVisible = RB_HW_ReadOcclusionQuery(corona->query[queryIndex]);
                totalPixels = RB_HW_ReadOcclusionQuery(corona->queryMax[queryIndex]);
                if ( numVisible == -1 || totalPixels == -1 )
                    error = 1;
                if ( numVisible > totalPixels )
                    error = 1;
                if ( totalPixels )
                {
                    v2 = (double)numVisible / (double)totalPixels;
                    if ( (float)(v2 - 1.0) < 0.0 )
                        v3 = (double)numVisible / (double)totalPixels;
                    else
                        v3 = 1.0f;
                    if ( (float)(0.0 - v2) < 0.0 )
                        v1 = v3;
                    else
                        v1 = 0.0f;
                    targetVisibility = v1;
                }
                else
                {
                    targetVisibility = 0.0f;
                }
            }
            else
            {
                error = 1;
                targetVisibility = 0.0f;
            }
            if ( !error )
                corona->targetVisibility = targetVisibility;
            RB_HW_BeginOcclusionQuery(corona->query[queryIndex]);
            RB_DrawCoronaSprite(corona, rgp.additiveMaterial, (GfxColor)-16777216, 10.0, 1);
            corona->query[queryIndex]->Issue(1u);
            RB_HW_BeginOcclusionQuery(corona->queryMax[queryIndex]);
            RB_DrawCoronaSprite(corona, rgp.additiveMaterialNoDepth, (GfxColor)-16777216, 10.0, 1);
            //((void (__thiscall *)(IDirect3DQuery9 *, IDirect3DQuery9 *, int))corona->queryMax[queryIndex]->Issue)(
            //    corona->queryMax[queryIndex],
            //    corona->queryMax[queryIndex],
            //    1);
            corona->queryMax[queryIndex]->Issue(1);
            corona->queryIssued[queryIndex] = 1;
        }
        else
        {
            corona->targetVisibility = 0.0f;
        }
    }
}

void __cdecl RB_HW_BeginOcclusionQuery(IDirect3DQuery9 *query)
{
    iassert(query);
    query->Issue(D3DISSUE_BEGIN);
}

unsigned int __cdecl RB_HW_ReadOcclusionQuery(IDirect3DQuery9 *query)
{
    HRESULT hr; // [esp+4h] [ebp-8h]
    unsigned int pixelCount; // [esp+8h] [ebp-4h] BYREF

    while ( 1 )
    {
        hr = query->GetData(&pixelCount, 4, D3DGETDATA_FLUSH);
        if ( hr != 1 )
            break;
        Sleep(0);
    }
    if ( hr >= 0 )
        return pixelCount;
    else
        return -1;
}

void __cdecl RB_ResetCoronas()
{
    CoronaState *state; // [esp+8h] [ebp-Ch]
    unsigned int coronaIndex; // [esp+Ch] [ebp-8h]
    unsigned int coronaIndexa; // [esp+Ch] [ebp-8h]
    unsigned int viewIndex; // [esp+10h] [ebp-4h]

    for ( viewIndex = 0; viewIndex < gfxCfg.maxClientViews; ++viewIndex )
    {
        state = &coronaState[viewIndex];
        state->numActive = 0;
        state->numPending = 0;
        for ( coronaIndex = 0; coronaIndex < 0x10; ++coronaIndex )
            RB_ResetCorona(&state->active[coronaIndex]);
        for ( coronaIndexa = 0; coronaIndexa < 0x10; ++coronaIndexa )
            RB_ResetCorona(&state->pending[coronaIndexa]);
    }
}

void __cdecl RB_AllocCoronaSpriteQueries()
{
    CoronaState *state; // [esp+8h] [ebp-Ch]
    unsigned int coronaIndex; // [esp+Ch] [ebp-8h]
    unsigned int coronaIndexa; // [esp+Ch] [ebp-8h]
    unsigned int viewIndex; // [esp+10h] [ebp-4h]

    for ( viewIndex = 0; viewIndex < gfxCfg.maxClientViews; ++viewIndex )
    {
        state = &coronaState[viewIndex];
        for ( coronaIndex = 0; coronaIndex < 0x10; ++coronaIndex )
            RB_AllocCoronaSpriteQuery(&state->active[coronaIndex]);
        for ( coronaIndexa = 0; coronaIndexa < 0x10; ++coronaIndexa )
            RB_AllocCoronaSpriteQuery(&state->pending[coronaIndexa]);
    }
    RB_ResetCoronas();
}

void __cdecl RB_AllocCoronaSpriteQuery(Corona *corona)
{
    unsigned int queryIndex; // [esp+10h] [ebp-4h]

    for ( queryIndex = 0; queryIndex < 5; ++queryIndex )
    {
        corona->query[queryIndex] = RB_HW_AllocOcclusionQuery();
        corona->queryMax[queryIndex] = RB_HW_AllocOcclusionQuery();
    }
}

void __cdecl RB_FreeCoronaSpriteQueries()
{
    CoronaState *state; // [esp+8h] [ebp-Ch]
    unsigned int coronaIndex; // [esp+Ch] [ebp-8h]
    unsigned int coronaIndexa; // [esp+Ch] [ebp-8h]
    unsigned int viewIndex; // [esp+10h] [ebp-4h]

    for ( viewIndex = 0; viewIndex < gfxCfg.maxClientViews; ++viewIndex )
    {
        state = &coronaState[viewIndex];
        for ( coronaIndex = 0; coronaIndex < 0x10; ++coronaIndex )
            RB_FreeCoronaSpriteQuery(&state->active[coronaIndex]);
        for ( coronaIndexa = 0; coronaIndexa < 0x10; ++coronaIndexa )
            RB_FreeCoronaSpriteQuery(&state->pending[coronaIndexa]);
    }
}

void __cdecl RB_FreeCoronaSpriteQuery(Corona *corona)
{
    IDirect3DSurface9 *var; // [esp+0h] [ebp-Ch]
    IDirect3DQuery9 *varCopy; // [esp+4h] [ebp-8h]
    unsigned int queryIndex; // [esp+8h] [ebp-4h]

    for ( queryIndex = 0; queryIndex < 5; ++queryIndex )
    {
        if ( corona->queryMax[queryIndex] )
        {
            do
            {
                if ( r_logFile && r_logFile->current.integer )
                    RB_LogPrint("corona->queryMax[queryIndex]->Release()\n");
                varCopy = corona->queryMax[queryIndex];
                corona->queryMax[queryIndex] = 0;
                R_ReleaseAndSetNULL<IDirect3DIndexBuffer9>(
                    (IDirect3DSurface9 *)varCopy,
                    "corona->queryMax[queryIndex]",
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_corona.cpp",
                    609);
            }
            while ( alwaysfails );
        }
        if ( corona->query[queryIndex] )
        {
            do
            {
                if ( r_logFile )
                {
                    if ( r_logFile->current.integer )
                        RB_LogPrint("corona->query[queryIndex]->Release()\n");
                }
                var = (IDirect3DSurface9 *)corona->query[queryIndex];
                corona->query[queryIndex] = 0;
                R_ReleaseAndSetNULL<IDirect3DIndexBuffer9>(
                    var,
                    "corona->query[queryIndex]",
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_corona.cpp",
                    611);
            }
            while ( alwaysfails );
        }
    }
}

void __cdecl RB_DrawWaypoint(
    Material *material,
    GfxColor color,
    float *origin,
    float radius,
    bool screenSize,
    bool alignBottom,
    bool forceNear)
{
    float transformedPosition; // [esp+3Ch] [ebp-38h]
    float transformedPosition_4; // [esp+40h] [ebp-34h]
    float transformedPosition_8; // [esp+44h] [ebp-30h]
    float transformedPosition_12; // [esp+48h] [ebp-2Ch]
    float sizeY; // [esp+4Ch] [ebp-28h]
    float offset_4; // [esp+54h] [ebp-20h]
    float sizeX; // [esp+5Ch] [ebp-18h]
    float position_4; // [esp+68h] [ebp-Ch]
    float position_8; // [esp+6Ch] [ebp-8h]
    GfxVertex *vert; // [esp+70h] [ebp-4h]

    if (R_StreamTouchMaterialAndCheck(material, 0))
    {
        RB_SetIdentity();
        RB_SetTessTechnique(material, 4u);
        R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_FX);
        position_4 = origin[1];
        position_8 = origin[2];
        transformedPosition = (float)((float)((float)(*origin
            * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[0][0])
            + (float)(position_4
                * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[1][0]))
            + (float)(position_8 * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[2][0]))
            + gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[3][0];
        transformedPosition_4 = (float)((float)((float)(*origin
            * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[0][1])
            + (float)(position_4
                * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[1][1]))
            + (float)(position_8 * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[2][1]))
            + gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[3][1];
        transformedPosition_8 = (float)((float)((float)(*origin
            * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[0][2])
            + (float)(position_4
                * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[1][2]))
            + (float)(position_8 * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[2][2]))
            + gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[3][2];
        transformedPosition_12 = (float)((float)((float)(*origin
            * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[0][3])
            + (float)(position_4
                * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[1][3]))
            + (float)(position_8 * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[2][3]))
            + gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[3][3];
        if (screenSize)
            radius = radius * transformedPosition_12;
        sizeY = (float)(2.0 * radius) / transformedPosition_12;
        sizeX = (float)((float)gfxCmdBufSourceState.renderTargetHeight * sizeY)
            / (float)gfxCmdBufSourceState.renderTargetWidth;
        if (material && material->textureCount)
            sizeY = (float)((float)material->textureTable->u.image->height / (float)material->textureTable->u.image->width)
            * sizeY;
        if (rg.renderHiResShot)
        {
            sizeX = (float)rg.hiResShotTiles * sizeX;
            sizeY = (float)rg.hiResShotTiles * sizeY;
        }
        vert = RB_SetTessQuad(color);
        offset_4 = sizeY * transformedPosition_12;
        vert->xyzw[0] = transformedPosition;
        vert->xyzw[1] = transformedPosition_4;
        vert->xyzw[2] = transformedPosition_8;
        vert->xyzw[3] = transformedPosition_12;
        vert[1].xyzw[0] = transformedPosition;
        vert[1].xyzw[1] = transformedPosition_4;
        vert[1].xyzw[2] = transformedPosition_8;
        vert[1].xyzw[3] = transformedPosition_12;
        vert[2].xyzw[0] = transformedPosition;
        vert[2].xyzw[1] = transformedPosition_4;
        vert[2].xyzw[2] = transformedPosition_8;
        vert[2].xyzw[3] = transformedPosition_12;
        vert[3].xyzw[0] = transformedPosition;
        vert[3].xyzw[1] = transformedPosition_4;
        vert[3].xyzw[2] = transformedPosition_8;
        vert[3].xyzw[3] = transformedPosition_12;
        vert->xyzw[0] = vert->xyzw[0] - (float)(sizeX * transformedPosition_12);
        vert->xyzw[1] = vert->xyzw[1] + (float)(sizeY * transformedPosition_12);
        vert->xyzw[2] = vert->xyzw[2] + (float)(0.000049999999 * transformedPosition_12);
        vert[1].xyzw[0] = vert[1].xyzw[0] + (float)(sizeX * transformedPosition_12);
        vert[1].xyzw[1] = vert[1].xyzw[1] + (float)(sizeY * transformedPosition_12);
        vert[1].xyzw[2] = vert[1].xyzw[2] + (float)(0.000049999999 * transformedPosition_12);
        vert[2].xyzw[0] = vert[2].xyzw[0] + (float)(sizeX * transformedPosition_12);
        vert[2].xyzw[1] = vert[2].xyzw[1] - (float)(sizeY * transformedPosition_12);
        vert[2].xyzw[2] = vert[2].xyzw[2] + (float)(0.000049999999 * transformedPosition_12);
        vert[3].xyzw[0] = vert[3].xyzw[0] - (float)(sizeX * transformedPosition_12);
        vert[3].xyzw[1] = vert[3].xyzw[1] - (float)(sizeY * transformedPosition_12);
        vert[3].xyzw[2] = vert[3].xyzw[2] + (float)(0.000049999999 * transformedPosition_12);
        if (alignBottom)
        {
            vert->xyzw[1] = vert->xyzw[1] + offset_4;
            vert[1].xyzw[1] = vert[1].xyzw[1] + offset_4;
            vert[2].xyzw[1] = vert[2].xyzw[1] + offset_4;
            vert[3].xyzw[1] = vert[3].xyzw[1] + offset_4;
        }
        Vec4DivideByW(vert->xyzw, vert->xyzw);
        Vec4DivideByW(vert[1].xyzw, vert[1].xyzw);
        Vec4DivideByW(vert[2].xyzw, vert[2].xyzw);
        Vec4DivideByW(vert[3].xyzw, vert[3].xyzw);
        if (forceNear && vert->xyzw[2] > 0.0)
        {
            vert->xyzw[2] = 0.0f;
            vert[1].xyzw[2] = 0.0f;
            vert[2].xyzw[2] = 0.0f;
            vert[3].xyzw[2] = 0.0f;
        }
        RB_EndTessSurface();
        if (tess.indexCount)
            RB_EndTessSurface();
    }
}