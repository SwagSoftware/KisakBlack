#include "r_outdoor.h"
#include "r_bsp.h"
#include <universal/assertive.h>
#include "r_image.h"
#include <universal/com_memory.h>
#include "r_image_load_obj.h"

OutdoorGlob outdoorGlob;

const int outdoorMapSize[3] = { 512, 512, 256 };

void __cdecl R_RegisterOutdoorImage(GfxWorld *world, const float *outdoorMin, const float *outdoorMax)
{
    if ( !world && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_outdoor.cpp", 191, 0, "%s", "world") )
        __debugbreak();
    if ( world->numOutdoorBounds )
        Outdoor_ApplyBoundingBox((const float *)world->outdoorBounds, world->outdoorBounds->bounds[1]);
    else
        Outdoor_ApplyBoundingBox(outdoorMin, outdoorMax);
    Outdoor_UpdateTransforms();
    Outdoor_SetRendererOutdoorLookupMatrix(world);
    world->outdoorImage = Image_Register((char*)"$outdoor", 1u, 0);
    if ( !world->outdoorImage
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_outdoor.cpp",
                    202,
                    0,
                    "%s",
                    "world->outdoorImage") )
    {
        __debugbreak();
    }
}

void __cdecl Outdoor_ApplyBoundingBox(const float *outdoorMin, const float *outdoorMax)
{
    int dimIter; // [esp+4h] [ebp-4h]

    *(_QWORD *)&outdoorGlob.bbox[0][0] = *(_QWORD *)outdoorMin;
    outdoorGlob.bbox[0][2] = outdoorMin[2];
    outdoorGlob.bbox[1][0] = *outdoorMax;
    outdoorGlob.bbox[1][1] = outdoorMax[1];
    outdoorGlob.bbox[1][2] = outdoorMax[2];
    for ( dimIter = 0; dimIter != 3; ++dimIter )
    {
        if ( outdoorGlob.bbox[0][dimIter] == 131072.0 )
        {
            if ( outdoorGlob.scale[dimIter - 3] != -131072.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_outdoor.cpp",
                            96,
                            1,
                            "%s",
                            "outdoorGlob.bbox[SUPREMUM][dimIter] == MIN_WORLD_COORD") )
            {
                __debugbreak();
            }
            outdoorGlob.bbox[0][dimIter] = 0.0f;
            outdoorGlob.scale[dimIter - 3] = 0.0f;
        }
        if ( outdoorGlob.scale[dimIter - 3] < outdoorGlob.bbox[0][dimIter]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_outdoor.cpp",
                        100,
                        1,
                        "%s",
                        "outdoorGlob.bbox[SUPREMUM][dimIter] >= outdoorGlob.bbox[INFIMUM][dimIter]") )
        {
            __debugbreak();
        }
        if ( (float)(outdoorGlob.scale[dimIter - 3] - outdoorGlob.bbox[0][dimIter]) < 1.0 )
        {
            outdoorGlob.bbox[0][dimIter] = outdoorGlob.bbox[0][dimIter] - 0.5;
            outdoorGlob.scale[dimIter - 3] = outdoorGlob.scale[dimIter - 3] + 0.5;
        }
        if ( outdoorGlob.scale[dimIter - 3] <= outdoorGlob.bbox[0][dimIter]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_outdoor.cpp",
                        106,
                        1,
                        "%s",
                        "outdoorGlob.bbox[SUPREMUM][dimIter] > outdoorGlob.bbox[INFIMUM][dimIter]") )
        {
            __debugbreak();
        }
    }
}

int Outdoor_UpdateTransforms()
{
    int result; // eax
    int dimension; // [esp+0h] [ebp-4h]

    for ( dimension = 0; dimension != 3; ++dimension )
    {
        outdoorGlob.scale[dimension] = (float)(outdoorMapSize[dimension] - 1)
                                                                 / (float)(outdoorGlob.scale[dimension - 3] - outdoorGlob.bbox[0][dimension]);
        outdoorGlob.invScale[dimension] = 1.0 / outdoorGlob.scale[dimension];
        //outdoorGlob.add[dimension] = COERCE_FLOAT(LODWORD(outdoorGlob.bbox[0][dimension]) ^ _mask__NegFloat_) * outdoorGlob.scale[dimension];
        outdoorGlob.add[dimension] = -((outdoorGlob.bbox[0][dimension])) * outdoorGlob.scale[dimension];
        result = dimension + 1;
    }
    return result;
}

void __cdecl Outdoor_SetRendererOutdoorLookupMatrix(GfxWorld *world)
{
    float outdoorScale[3]; // [esp+0h] [ebp-1Ch]
    float outdoorTranslate[3]; // [esp+Ch] [ebp-10h]
    int dimIter; // [esp+18h] [ebp-4h]

    for ( dimIter = 0; dimIter != 3; ++dimIter )
    {
        outdoorScale[dimIter] = 1.0 / (float)(outdoorGlob.scale[dimIter - 3] - outdoorGlob.bbox[0][dimIter]);
        //outdoorTranslate[dimIter] = COERCE_FLOAT(LODWORD(outdoorGlob.bbox[0][dimIter]) ^ _mask__NegFloat_) * outdoorScale[dimIter];
        outdoorTranslate[dimIter] = -((outdoorGlob.bbox[0][dimIter])) * outdoorScale[dimIter];
    }
    MatrixIdentity44(world->outdoorLookupMatrix);
    world->outdoorLookupMatrix[0][0] = outdoorScale[0];
    world->outdoorLookupMatrix[1][1] = outdoorScale[1];
    world->outdoorLookupMatrix[2][2] = outdoorScale[2];
    *(_QWORD *)&world->outdoorLookupMatrix[3][0] = *(_QWORD *)outdoorTranslate;
    world->outdoorLookupMatrix[3][2] = outdoorTranslate[2];
}

void __cdecl R_GenerateOutdoorImage(GfxImage *outdoorImage)
{
    outdoorGlob.pic = (unsigned __int8 *)Hunk_AllocateTempMemory(
                                                                                 outdoorMapSize[1] * outdoorMapSize[0],
                                                                                 "Outdoor_TempHunkAllocatePic");
    Outdoor_ComputeTexels();
    Image_Generate2D(outdoorImage, outdoorGlob.pic, outdoorMapSize[0], outdoorMapSize[1], D3DFMT_L8);
    Outdoor_TempHunkFreePic();
}

void Outdoor_TempHunkFreePic()
{
    if ( !outdoorGlob.pic
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_outdoor.cpp", 54, 0, "%s", "outdoorGlob.pic") )
    {
        __debugbreak();
    }
    Hunk_FreeTempMemory((char *)outdoorGlob.pic);
}

void Outdoor_ComputeTexels()
{
    float zWorld; // [esp+28h] [ebp-18h]
    float yWorld; // [esp+2Ch] [ebp-14h]
    unsigned __int8 *texelPtr; // [esp+30h] [ebp-10h]
    int x; // [esp+34h] [ebp-Ch]
    int y; // [esp+38h] [ebp-8h]

    if ( !outdoorGlob.pic
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_outdoor.cpp", 173, 0, "%s", "outdoorGlob.pic") )
    {
        __debugbreak();
    }
    texelPtr = outdoorGlob.pic;
    for ( y = 0; y != outdoorMapSize[1]; ++y )
    {
        yWorld = (float)((float)((float)y + 0.5) - outdoorGlob.add[1]) * outdoorGlob.invScale[1];
        for ( x = 0; x != outdoorMapSize[0]; ++x )
        {
            zWorld = Outdoor_TraceHeightInWorld(
                                 (float)((float)((float)x + 0.5) - outdoorGlob.add[0]) * outdoorGlob.invScale[0],
                                 yWorld);
            Outdoor_WriteToPic(zWorld, texelPtr++);
        }
    }
}

double __cdecl Outdoor_TraceHeightInWorld(float worldX, float worldY)
{
    float traceEndHeight; // [esp+28h] [ebp-5Ch]
    float traceStartHeight; // [esp+2Ch] [ebp-58h]
    trace_t results; // [esp+30h] [ebp-54h] BYREF
    float traceStart[3]; // [esp+6Ch] [ebp-18h] BYREF
    float traceEnd[3]; // [esp+78h] [ebp-Ch] BYREF

    //col_context_t::col_context_t(&context, 8241);
    col_context_t context(0x2031); // [esp+0h] [ebp-84h] BYREF

    traceStartHeight = outdoorGlob.bbox[1][2] + 1.0;
    traceEndHeight = outdoorGlob.bbox[0][2] - 1.0;
    traceStart[0] = worldX;
    traceStart[1] = worldY;
    traceStart[2] = outdoorGlob.bbox[1][2] + 1.0;
    traceEnd[0] = worldX;
    traceEnd[1] = worldY;
    traceEnd[2] = outdoorGlob.bbox[0][2] - 1.0;
    memset(&results, 0, sizeof(results));
    results.fraction = 1.0f;
    CM_BoxTrace(&results, traceStart, traceEnd, vec3_origin, vec3_origin, 8241, &context);
    return (traceEndHeight - traceStartHeight) * results.fraction + traceStartHeight;
}

void __cdecl Outdoor_WriteToPic(float zWorld, unsigned __int8 *outByte)
{
    int zTexture; // [esp+20h] [ebp-4h]

    zTexture = Outdoor_TransformToTextureClamped(2, zWorld);
    *outByte = zTexture;
    if ( zTexture != *outByte
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_outdoor.cpp",
                    164,
                    0,
                    "%s",
                    "zTexture == *outByte") )
    {
        __debugbreak();
    }
}

int __cdecl Outdoor_TransformToTextureClamped(int dimension, float inWorld)
{
    int v4; // [esp+4h] [ebp-18h]
    int unclamped; // [esp+14h] [ebp-8h]

    unclamped = (int)((float)((float)(inWorld * outdoorGlob.scale[dimension]) + outdoorGlob.add[dimension])
                                    - 0.4999999990686774);
    v4 = outdoorMapSize[dimension] - 1;
    if ( v4 <= 0
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 719, 0, "%s", "min < max") )
    {
        __debugbreak();
    }
    if ( unclamped < 0 )
        return 0;
    if ( unclamped <= v4 )
        return unclamped;
    return v4;
}

