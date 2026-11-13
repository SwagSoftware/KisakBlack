#pragma once
#include "ui_main.h"
#include <bgame/bg_emblems.h>
#include <universal/offline.h>

struct EmblemFilterState // sizeof=0xC
{                                       // XREF: .data:s_filterStates/r
    const char *loc;                    // XREF: UI_EmblemGetStateDisplay(int,int)+2A/r
    unsigned int include;               // XREF: UI_EmblemGetFilterCount(int,int,int)+6B/r
                                        // UI_EmblemGetFilterCount(int,int,int)+89/r ...
    unsigned int exclude;               // XREF: UI_EmblemGetFilterCount(int,int,int)+98/r
                                        // UI_EmblemGetFilterIconID(int,int,int,int)+98/r
};

void __cdecl UI_DrawCustomEmblem(int contextIndex, const rectDef_s *rect, const float *color);
void __cdecl UI_DrawCustomEmblemInternal(
                int contextIndex,
                const rectDef_s *rect,
                const float *color,
                CompositeEmblemLayer *layers,
                int layerCount,
                bool withSpinner);
void __cdecl UI_DrawEmblemSelector(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                const rectDef_s *rect,
                const float *color);
void __cdecl UI_DrawPlayerEmblemServer(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                const rectDef_s *rect,
                const float *color);
void __cdecl UI_DrawPlayerEmblem(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                const rectDef_s *rect,
                const float *color);
void __cdecl UI_DrawPlayerEmblemByXuid(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                const rectDef_s *rect,
                const float *color,
                unsigned __int64 xuid);
void __cdecl UI_DrawEmblemIconThumbnail(int contextIndex, const rectDef_s *rect, const float *color);
void __cdecl UI_DrawEmblemColors(int contextIndex, const rectDef_s *rect);
void __cdecl UI_EmblemUpdate(int localClientNum);
void __cdecl UI_EmblemClampLayer(CompositeEmblemLayer *layer);
int __cdecl UI_EmblemGetSelectedLayer();
bool __cdecl UI_EmblemCanOutlineLayer(unsigned int layer);
bool __cdecl UI_EmblemCanDuplicateLayer(int controllerIndex);
void __cdecl UI_EmblemDuplicate_f();
bool __cdecl UI_EmblemIsModified(int controllerIndex);
const char *__cdecl UI_EmblemGetLayerName(unsigned int layer);
__int16 __cdecl UI_EmblemGetSelectedBackgroundID();
char *__cdecl UI_EmblemGetStateDisplay(int controllerIndex, int idx);
char *__cdecl UI_EmblemGetCategoryDisplay(int controllerIndex, int idx);
int __cdecl UI_EmblemGetFilterCount(int controllerIndex, int state, int category);
__int16 __cdecl UI_EmblemGetFilterIconID(int controllerIndex, int state, int category, int index);
void __cdecl UI_EmblemSelect(unsigned int layer);
void __cdecl UI_EmblemSelect_f();
void __cdecl UI_EmblemReset_f();
void __cdecl UI_EmblemClear_f();
void __cdecl UI_EmblemClearAll_f();
void __cdecl UI_EmblemRotate_f();
void __cdecl UI_EmblemMove_f();
void __cdecl UI_EmblemScale_f();
void __cdecl UI_EmblemIcon_f();
void __cdecl UI_EmblemPalette_f();
void __cdecl UI_EmblemPaletteCycle_f();
int __cdecl UI_EmblemCyclePalette(int index, int direction);
void __cdecl UI_EmblemPalettePick_f();
void __cdecl UI_EmblemOutline_f();
void __cdecl UI_EmblemToggleOutline_f();
void __cdecl UI_EmblemFlip_f();
void __cdecl UI_EmblemToggleFlip_f();
void __cdecl UI_EmblemMoveLayer_f();
void __cdecl UI_EmblemMoveLayer(int controllerIndex, int targetz);
void __cdecl UI_EmblemMoveLayerRelative_f();
void __cdecl UI_EmblemCopy_f();
void __cdecl UI_EmblemBeginEdit_f();
void __cdecl UI_EmblemEndEdit_f();
void __cdecl UI_EmblemSetProfile_f();
void __cdecl UI_EmblemGetProfile_f();
void __cdecl UI_EmblemSelectBackground_f();
void __cdecl UI_EmblemDump_f();
void __cdecl UI_EmblemClearDefaults_f();
void __cdecl UI_EmblemSetAsDefault_f();
void __cdecl UI_EmblemMoveUpRepeatEnabled_f();
void __cdecl UI_EmblemMoveDownRepeatEnabled_f();
void __cdecl UI_EmblemMoveLeftRepeatEnabled_f();
void __cdecl UI_EmblemMoveRightRepeatEnabled_f();
void __cdecl UI_EmblemScaleUpRepeatEnabled_f();
void __cdecl UI_EmblemScaleDownRepeatEnabled_f();
void __cdecl UI_EmblemRotateLeftRepeatEnabled_f();
void __cdecl UI_EmblemRotateRightRepeatEnabled_f();
void __cdecl UI_EmblemRepeatBttnsLooseFocus_f();
void __cdecl UI_EmblemRegisterCmds();
