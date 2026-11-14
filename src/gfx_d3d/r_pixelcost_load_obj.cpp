#include "r_pixelcost_load_obj.h"
#include <universal/assertive.h>
#include <qcommon/common.h>

const GfxPixelCostColorCode s_pixelCostColorCodeTable[6] =
{
  { 0u, 0u, 0u, 0u, "black" },
  { 40u, 0u, 255u, 0u, "green" },
  { 56u, 255u, 255u, 0u, "yellow" },
  { 72u, 255u, 128u, 0u, "orange" },
  { 128u, 255u, 0u, 0u, "red" },
  { 255u, 255u, 255u, 255u, "white" }
};


void __cdecl RB_PixelCost_BuildColorCodeMap(unsigned __int8 (*pixels)[4], int pixelCount)
{
    float lerp; // [esp+50h] [ebp-18h]
    int codeIndex; // [esp+60h] [ebp-8h]
    int pixelIndex; // [esp+64h] [ebp-4h]

    if ( pixelCount != 256
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_pixelcost_load_obj.cpp",
                    41,
                    0,
                    "%s\n\t(pixelCount) = %i",
                    "(pixelCount == 256)",
                    pixelCount) )
    {
        __debugbreak();
    }
    codeIndex = -1;
    for ( pixelIndex = 0; pixelIndex < 256; ++pixelIndex )
    {
        if ( s_pixelCostColorCodeTable[codeIndex + 1].count == pixelIndex )
        {
            ++codeIndex;
            *(unsigned int *)&(*pixels)[4 * pixelIndex] = s_pixelCostColorCodeTable[codeIndex].b
                                                                                        | (s_pixelCostColorCodeTable[codeIndex].g << 8)
                                                                                        | (s_pixelCostColorCodeTable[codeIndex].r << 16)
                                                                                        | 0xFF000000;
        }
        else
        {
            lerp = (float)(pixelIndex - s_pixelCostColorCodeTable[codeIndex].count)
                     / (float)(s_pixelCostColorCodeTable[codeIndex + 1].count - s_pixelCostColorCodeTable[codeIndex].count);
            (*pixels)[4 * pixelIndex] = SnapFloatToInt((float)((float)s_pixelCostColorCodeTable[codeIndex].b
                                                                                            + (float)((float)(s_pixelCostColorCodeTable[codeIndex + 1].b
                                                                                                                            - s_pixelCostColorCodeTable[codeIndex].b)
                                                                                                            * lerp)));
            (*pixels)[4 * pixelIndex + 1] = SnapFloatToInt((float)((float)s_pixelCostColorCodeTable[codeIndex].g
                                                                                                    + (float)((float)(s_pixelCostColorCodeTable[codeIndex + 1].g
                                                                                                                                    - s_pixelCostColorCodeTable[codeIndex].g)
                                                                                                                    * lerp)));
            (*pixels)[4 * pixelIndex + 2] = SnapFloatToInt((float)((float)s_pixelCostColorCodeTable[codeIndex].r
                                                                                                    + (float)((float)(s_pixelCostColorCodeTable[codeIndex + 1].r
                                                                                                                                    - s_pixelCostColorCodeTable[codeIndex].r)
                                                                                                                    * lerp)));
            (*pixels)[4 * pixelIndex + 3] = -1;
        }
    }
}

void __cdecl R_PixelCost_PrintColorCodeKey()
{
    unsigned int codeIndex; // [esp+10h] [ebp-4h]

    Com_Printf(8, "\n---- PIXEL COST KEY ----\n");
    for ( codeIndex = 0; codeIndex < 6; ++codeIndex )
        Com_Printf(
            8,
            "    %3.0f%%: %s\n",
            (float)((float)s_pixelCostColorCodeTable[codeIndex].count * 1.5625),
            s_pixelCostColorCodeTable[codeIndex].colorName);
    Com_Printf(8, "----------------------\n\n");
}

