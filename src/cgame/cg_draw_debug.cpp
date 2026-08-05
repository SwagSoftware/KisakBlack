#include "cg_draw_debug.h"
#include <client/screen_placement.h>
#include <cgame_mp/cg_main_mp.h>
#include <gfx_d3d/r_font.h>
#include "cg_drawtools.h"
#include <qcommon/mem_track.h>
#include <gfx_d3d/r_init.h>
#include <game_mp/g_main_mp.h>
#include <server_mp/sv_main_mp.h>
#include <bgame/bg_misc.h>
#include <sound/snd_debug.h>
#include <win32/win_shared.h>
#include <ui/ui_atoms.h>
#include <aim_assist/aim_assist.h>
#include <gfx_d3d/r_utils.h>
#include <client_mp/cl_cgame_mp.h>
#include <bgame/bg_pmove.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/cscr_memorytree.h>
#include <EffectsCore/fx_profile.h>
#include <clientscript/cscr_stringlist.h>
#include <clientscript/scr_const.h>
#include <qcommon/sv_msg_write_mp.h>
#include <EffectsCore/fx_dvars.h>
#include <ragdoll/ragdoll.h>
#include <glass/glass_client.h>
#include <server_mp/sv_bot_mp.h>
#include <gfx_d3d/r_foliage.h>
#include "cg_perf.h"
#include <physics/physics_system_internal.h>
#include <gfx_d3d/r_dvars.h>
#include <tl/tl_system.h>

__int16 entity_counts[65536];

trViewStatistics_t rendererViewStats[1];
trStatistics_t rendererStats;

const char *g_TagNames[6] =
{
  "tag_weapon_left",
  "tag_weapon_right",
  "tag_inhand",
  "tag_stowed_back",
  "tag_weapon_chest",
  "1st person view pos"
};

int(__cdecl *CG_SoundCompares[6])(const void *, const void *) =
{
  (int(*)(const void*, const void*))CG_CompareSndInfoPriority,
  (int(*)(const void*, const void*))CG_CompareSndInfoChannel,
  (int(*)(const void*, const void*))CG_CompareSndInfoAlias,
  (int(*)(const void*, const void*))CG_CompareSndInfoDryLevel,
  (int(*)(const void*, const void*))CG_CompareSndInfoEntity,
  (int(*)(const void*, const void*))CG_CompareSndInfoDistance
};

float g_TagColors[6][4] =
{
  { 0.89999998, 0.2, 0.2, 1.0 },
  { 0.2, 0.2, 0.89999998, 1.0 },
  { 0.2, 0.89999998, 0.2, 1.0 },
  { 0.89999998, 0.2, 0.89999998, 1.0 },
  { 0.2, 0.89999998, 0.89999998, 1.0 },
  { 0.89999998, 0.89999998, 0.2, 1.0 }
};

double __cdecl CG_DrawAnimTagInfo(const ScreenPlacement *scrPlace, float posY)
{
    int v2; // eax
    int i; // [esp+20h] [ebp-4h]

    if ( !cgMedia.smallDevFont )
        cgMedia.smallDevFont = CL_RegisterFont("fonts/smalldevfont", 1);
    for ( i = 0; i < 6; ++i )
    {
        v2 = R_TextWidth("tag name", 0, cgMedia.smallDevFont);
        posY += CG_CornerDebugPrint(
                         scrPlace,
                         (float)(scrPlace->virtualViewableMax[0] - scrPlace->virtualViewableMin[0]) + cg_debugInfoCornerOffset->current.value,
                         posY,
                         (float)v2 * 1.0,
                         (char*)g_TagNames[i],
                         (char*)"tag name",
                         (const float*)g_TagColors[i]);
    }
    return posY;
}

double __cdecl CG_CornerDebugPrint(
                const ScreenPlacement *sP,
                float posX,
                float posY,
                float labelWidth,
                char *text,
                char *label,
                const float *color)
{
    int v8; // [esp+20h] [ebp-10h]
    int textDelta; // [esp+24h] [ebp-Ch]
    int yDelta; // [esp+28h] [ebp-8h]
    int labelDelta; // [esp+2Ch] [ebp-4h]

    if ( cg_drawFPSLabels->current.enabled )
    {
        textDelta = CG_DrawDevString(sP, posX - labelWidth, posY, 1.0, 1.1, text, color, 6, cgMedia.smallDevFont);
        labelDelta = CG_DrawDevString(
                                     sP,
                                     posX - labelWidth,
                                     posY,
                                     1.0,
                                     1.1,
                                     label,
                                     colorWhiteFaded,
                                     5,
                                     cgMedia.smallDevFont);
        if ( textDelta < labelDelta )
            v8 = labelDelta;
        else
            v8 = textDelta;
        yDelta = v8;
    }
    else
    {
        yDelta = CG_DrawDevString(sP, posX, posY, 1.0, 1.1, text, color, 6, cgMedia.smallDevFont);
    }
    return (double)yDelta * 0.75;
}

void __cdecl CG_DrawUpperRightDebugInfo(int localClientNum)
{
    meminfo_t meminfo; // [esp+8h] [ebp-1C8h] BYREF
    float y; // [esp+1CCh] [ebp-4h]

    track_getbasicinfo(&meminfo);
    R_TrackStatistics(0);
    y = cg_debugInfoCornerOffset->current.vector[1];
    if ( Dvar_GetInt("debug_show_viewpos") )
        y = CG_DrawViewpos(&scrPlaceFull, y, localClientNum);
    if ( cg_drawFPS->current.integer )
        y = CG_DrawFPS(localClientNum, &scrPlaceFull, y, &meminfo);
    if ( com_statmon->current.enabled )
        y = CG_DrawStatmon(&scrPlaceFull, y);
    if ( cg_drawAnimAttachTags->current.enabled )
        y = CG_DrawAnimTagInfo(&scrPlaceFull, y);
    if ( cg_drawSnapshot->current.enabled )
        y = CG_DrawSnapshot(localClientNum, y);
    if ( cg_drawSnapshotTime->current.enabled )
        y = DrawSnapshotInfo(&scrPlaceFull, y);
    if ( g_listEntityCounts )
    {
        if ( g_listEntityCounts->current.integer )
            DrawEntityCounts(&scrPlaceFull, y);
    }
}

double __cdecl CG_DrawSnapshot(int localClientNum, float posY)
{
    char *v2; // eax
    char *v3; // eax
    float v5; // [esp+18h] [ebp-1Ch]
    char *str; // [esp+1Ch] [ebp-18h]
    const ScreenPlacement *scrPlace; // [esp+20h] [ebp-14h]
    const cg_s *cgameGlob; // [esp+24h] [ebp-10h]
    float posX; // [esp+2Ch] [ebp-8h]
    const cgs_t *cgs; // [esp+30h] [ebp-4h]
    float posYa; // [esp+40h] [ebp+Ch]
    float posYb; // [esp+40h] [ebp+Ch]
    float posYc; // [esp+40h] [ebp+Ch]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    scrPlace = &scrPlaceView[localClientNum];
    posX = (float)(scrPlace->virtualViewableMax[0] - scrPlace->virtualViewableMin[0])
             + cg_debugInfoCornerOffset->current.value;
    v5 = (float)R_TextWidth(" server time", 0, cgMedia.smallDevFont) * 1.0;
    posYa = CG_CornerDebugPrintCaption(scrPlace, posX, posY, v5, (char*)"-Snapshot-", colorGreenFaded) + posY;
    v2 = va("%i", cgameGlob->nextSnap->serverTime);
    posYb = CG_CornerDebugPrint(scrPlace, posX, posYa, v5, v2, (char *)" server time", colorWhite) + posYa;
    v3 = va("%i", cgameGlob->latestSnapshotNum);
    posYc = CG_CornerDebugPrint(scrPlace, posX, posYb, v5, v3, (char *)" snap num", colorWhite) + posYb;
    str = va("%i", cgs->serverCommandSequence);
    return (float)(CG_CornerDebugPrint(scrPlace, posX, posYc, v5, str, (char *)" cmd", colorWhite) + posYc);
}

double __cdecl CG_CornerDebugPrintCaption(
                const ScreenPlacement *sP,
                float posX,
                float posY,
                float labelWidth,
                char *text,
                const float *color)
{
    int yDelta; // [esp+20h] [ebp-4h]

    if ( cg_drawFPSLabels->current.enabled )
        yDelta = CG_DrawDevString(sP, posX - labelWidth, posY, 1.0, 1.1, text, color, 7, cgMedia.smallDevFont);
    else
        yDelta = CG_DrawDevString(sP, posX, posY, 1.0, 1.1, text, color, 6, cgMedia.smallDevFont);
    return (double)yDelta * 0.75;
}

double __cdecl CG_DrawStatmon(const ScreenPlacement *scrPlace, float y)
{
    return y;
}

float CG_DrawViewposoffset = 50.0f;
double __cdecl CG_DrawViewpos(const ScreenPlacement *scrPlace, float y, int localClientNum)
{
    const char *String; // eax
    double v5; // [esp+0h] [ebp-34h]
    double v6; // [esp+8h] [ebp-2Ch]
    double posX; // [esp+10h] [ebp-24h]
    double labelWidth; // [esp+18h] [ebp-1Ch]
    double v9; // [esp+20h] [ebp-14h]
    float farRight; // [esp+28h] [ebp-Ch]
    const cg_s *cgameGlob; // [esp+2Ch] [ebp-8h]
    char *s; // [esp+30h] [ebp-4h]

    if ( CG_GetLocalClientGlobals(localClientNum)->nextSnap )
    {
        farRight = (float)((float)(scrPlace->virtualViewableMax[0] - scrPlace->virtualViewableMin[0])
                                         + cg_debugInfoCornerOffset->current.value)
                         - CG_DrawViewposoffset;
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        v9 = cgameGlob->refdefViewAngles[0];
        labelWidth = cgameGlob->refdefViewAngles[1];
        posX = cgameGlob->refdef.vieworg[2];
        v6 = cgameGlob->refdef.vieworg[1];
        v5 = cgameGlob->refdef.vieworg[0];
        String = Dvar_GetString("mapname");
        s = va("%s (%.1f %.1f %.1f) %.1f %.1f", String, v5, v6, posX, labelWidth, v9);
        return (float)(CG_CornerDebugPrint(scrPlace, farRight, y, 0.0, s, (char *)"", colorWhite) + y);
    }
    return y;
}

double __cdecl DrawEntityCounts(const ScreenPlacement *scrPlace, float posY)
{
    double v3; // st7
    double v5; // st7
    char *v6; // eax
    char *v7; // eax
    char *v8; // eax
    char *v9; // eax
    char *v10; // eax
    char *v11; // eax
    char *v12; // eax
    char *v13; // eax
    char *EntityTypeName; // eax
    char *v15; // eax
    char *v16; // eax
    char *v17; // eax
    int v18; // [esp+14h] [ebp-90h]
    char *v19; // [esp+1Ch] [ebp-88h]
    float v20; // [esp+28h] [ebp-7Ch]
    float v21; // [esp+38h] [ebp-6Ch]
    float v22; // [esp+3Ch] [ebp-68h]
    float v23; // [esp+40h] [ebp-64h]
    float v24; // [esp+48h] [ebp-5Ch]
    float v25; // [esp+4Ch] [ebp-58h]
    unsigned __int16 index; // [esp+54h] [ebp-50h]
    gentity_s *ent; // [esp+58h] [ebp-4Ch]
    fake_centity_s *cent; // [esp+5Ch] [ebp-48h]
    int j; // [esp+60h] [ebp-44h]
    gentity_s *e; // [esp+64h] [ebp-40h]
    int numUsedEntities; // [esp+68h] [ebp-3Ch]
    char *str; // [esp+6Ch] [ebp-38h]
    char *stra; // [esp+6Ch] [ebp-38h]
    char *strb; // [esp+6Ch] [ebp-38h]
    char *strc; // [esp+6Ch] [ebp-38h]
    char *strd; // [esp+6Ch] [ebp-38h]
    int listEntityCountsCurrent; // [esp+70h] [ebp-34h]
    int destructibles; // [esp+74h] [ebp-30h]
    float labelWidth; // [esp+7Ch] [ebp-28h]
    float labelWidtha; // [esp+7Ch] [ebp-28h]
    float labelWidthb; // [esp+7Ch] [ebp-28h]
    int total; // [esp+80h] [ebp-24h]
    int scriptmovers; // [esp+84h] [ebp-20h]
    float posX; // [esp+88h] [ebp-1Ch]
    int sound; // [esp+8Ch] [ebp-18h]
    int lights; // [esp+90h] [ebp-14h]
    int i; // [esp+94h] [ebp-10h]
    signed int ia; // [esp+94h] [ebp-10h]
    int ib; // [esp+94h] [ebp-10h]
    int ic; // [esp+94h] [ebp-10h]
    int fx; // [esp+98h] [ebp-Ch]
    const float *color; // [esp+9Ch] [ebp-8h]
    const float *colora; // [esp+9Ch] [ebp-8h]
    int triggers; // [esp+A0h] [ebp-4h]
    float posYa; // [esp+B0h] [ebp+Ch]
    float posYb; // [esp+B0h] [ebp+Ch]
    float posYc; // [esp+B0h] [ebp+Ch]
    float posYd; // [esp+B0h] [ebp+Ch]
    float posYe; // [esp+B0h] [ebp+Ch]
    float posYf; // [esp+B0h] [ebp+Ch]
    float posYg; // [esp+B0h] [ebp+Ch]
    float posYh; // [esp+B0h] [ebp+Ch]
    float posYi; // [esp+B0h] [ebp+Ch]
    float posYj; // [esp+B0h] [ebp+Ch]
    float posYk; // [esp+B0h] [ebp+Ch]

    listEntityCountsCurrent = g_listEntityCounts->current.integer;
    if (sv.num_entities > g_maxEntsInSnapshot->current.integer)
        Dvar_SetInt((dvar_s*)g_maxEntsInSnapshot, sv.num_entities);
    posX = (float)(scrPlace->virtualViewableMax[0] - scrPlace->virtualViewableMin[0])
        + cg_debugInfoCornerOffset->current.value;
    if (listEntityCountsCurrent || cg_drawFPS && cg_drawFPS->current.integer)
    {
        v25 = (float)R_TextWidth(" Ent Count", 0, cgMedia.smallDevFont) * 1.0;
        numUsedEntities = sv.num_entities;
        for (e = level.firstFreeEnt; e; e = e->nextFree)
            --numUsedEntities;
        str = va("%s: %d\n", " Ent Count", numUsedEntities);
        color = colorWhite;
        if (numUsedEntities <= 925)
        {
            if (numUsedEntities > 900)
                color = colorYellow;
            v3 = CG_CornerDebugPrint(scrPlace, posX, posY, v25, str, (char *)"", color);
        }
        else
        {
            v3 = CG_CornerDebugPrint(scrPlace, posX, posY, v25, str, (char *)"", colorRed);
        }
        posY = v3 + posY;
    }
    if (!listEntityCountsCurrent)
        return posY;
    v24 = (float)R_TextWidth(" Ent Count High", 0, cgMedia.smallDevFont) * 1.0;
    stra = va("%s: %d\n", " Ent Count High", g_maxEntsInSnapshot->current.integer);
    colora = colorWhite;
    if (g_maxEntsInSnapshot->current.integer <= 925)
    {
        if (g_maxEntsInSnapshot->current.integer > 900)
            colora = colorYellow;
        v5 = CG_CornerDebugPrint(scrPlace, posX, posY, v24, stra, (char *)"", colora);
    }
    else
    {
        v5 = CG_CornerDebugPrint(scrPlace, posX, posY, v24, stra, (char *)"", colorRed);
    }
    posYa = v5 + posY;
    total = 0;
    lights = 0;
    fx = 0;
    destructibles = 0;
    sound = 0;
    triggers = 0;
    scriptmovers = 0;
    for (j = 0; j < 512; ++j)
    {
        cent = &cg_fakeEntitiesArray[j];
        if (cg_fakeEntitiesInuseArray[812 * j / 812])
        {
            ++total;
            switch (cent->cent.nextState.eType)
            {
            case 6:
                ++scriptmovers;
                break;
            case 8:
            case 9:
                ++fx;
                break;
            case 0xA:
                ++lights;
                break;
            default:
                if (cent->cent.nextState.loopSoundId)
                {
                    ++sound;
                }
                else if (cent->cent.destructible)
                {
                    ++destructibles;
                }
                else if (((*((_DWORD *)&cent->cent + 201) >> 15) & 1) != 0)
                {
                    ++triggers;
                }
                break;
            }
        }
    }
    if (total != cg_fakeEntitiesInuseCount[0]
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_debug.cpp",
            449,
            0,
            "%s",
            "total == cg_fakeEntitiesInuseCount[0]"))
    {
        __debugbreak();
    }
    v23 = (float)R_TextWidth("wwwwwwwwwwwwwwww", 0, cgMedia.smallDevFont) * 1.0;
    if (listEntityCountsCurrent == 2)
    {
        v6 = va("%d", total);
        posYb = CG_CornerDebugPrint(scrPlace, posX, posYa, v23, v6, (char*)" CEnt Count High", colorWhite) + posYa;
        v7 = va("%d", fx);
        posYc = CG_CornerDebugPrint(scrPlace, posX, posYb, v23, v7, (char *)" CEnt fx", colorWhite) + posYb;
        v8 = va("%d", sound);
        posYd = CG_CornerDebugPrint(scrPlace, posX, posYc, v23, v8, (char *)" CEnt sound", colorWhite) + posYc;
        v9 = va("%d", scriptmovers);
        posYe = CG_CornerDebugPrint(scrPlace, posX, posYd, v23, v9, (char *)" CEnt scriptmovers", colorWhite) + posYd;
        v10 = va("%d", lights);
        posYf = CG_CornerDebugPrint(scrPlace, posX, posYe, v23, v10, (char *)" CEnt lights", colorWhite) + posYe;
        v11 = va("%d", destructibles);
        posYg = CG_CornerDebugPrint(scrPlace, posX, posYf, v23, v11, (char *)" CEnt dest", colorWhite) + posYf;
        v12 = va("%d", triggers);
        posYh = CG_CornerDebugPrint(scrPlace, posX, posYg, v23, v12, (char *)" CEnt triggers", colorWhite) + posYg;
        strb = va("%d", total - (scriptmovers + triggers + destructibles + sound + fx + lights));
        posYa = CG_CornerDebugPrint(scrPlace, posX, posYh, v23, strb, (char *)" CEnt misc", colorWhite) + posYh;
    }
    if (listEntityCountsCurrent > 2 && listEntityCountsCurrent <= 5)
    {
        memset(entity_counts, 0, sizeof(entity_counts));
        for (i = 0; ; ++i)
        {
            if (i >= 1024)
            {
                for (ia = 0; ia < 0x10000; ++ia)
                {
                    if (entity_counts[ia] > 0)
                    {
                        v22 = (float)R_TextWidth(" replay time", 0, cgMedia.smallDevFont) * 1.0;
                        if (ia)
                        {
                            v19 = SL_ConvertToString(ia, SCRIPTINSTANCE_SERVER);
                            v13 = va("%s: %d\n", v19, entity_counts[ia]);
                        }
                        else
                        {
                            v13 = va("%s: %d\n", "<unknown>", entity_counts[0]);
                        }
                        posYa = CG_CornerDebugPrint(scrPlace, posX, posYa, v22, v13, (char *)"", colorWhite) + posYa;
                    }
                }
                goto LABEL_79;
            }
            ent = &g_entities[i];
            if (ent->r.inuse)
            {
                index = ent->classname;
                if (listEntityCountsCurrent == 3)
                {
                    if ((ent->r.svFlags & 1) != 0)
                        continue;
                    goto LABEL_58;
                }
                if (listEntityCountsCurrent == 4)
                {
                    if ((ent->r.svFlags & 1) != 0)
                        continue;
                    goto LABEL_58;
                }
                if ((ent->r.svFlags & 1) == 0 && ent->classname == scr_const.script_model)
                {
                    index = ent->targetname;
                LABEL_58:
                    ++entity_counts[index];
                    continue;
                }
            }
        }
    }
    if (listEntityCountsCurrent == 6)
    {
        memset(entity_counts, 0, sizeof(entity_counts));
        for (ib = 0; ib < 1024; ++ib)
        {
            if (g_entities[ib].r.inuse)
                ++entity_counts[g_entities[ib].s.eType];
        }
        for (ic = 0; ic < 21; ++ic)
        {
            if (entity_counts[ic] > 0)
            {
                v21 = (float)R_TextWidth(" replay time", 0, cgMedia.smallDevFont) * 1.0;
                v18 = entity_counts[ic];
                EntityTypeName = BG_GetEntityTypeName(ic);
                strc = va("%s: %d\n", EntityTypeName, v18);
                posYa = CG_CornerDebugPrint(scrPlace, posX, posYa, v21, strc, (char *)"", colorWhite) + posYa;
            }
        }
    }
LABEL_79:
    labelWidth = (float)R_TextWidth(" ents in snapshot", 0, cgMedia.smallDevFont) * 1.0;
    v15 = va("ents in snapshot: %d\n", g_entsInSnapshot->current.integer);
    posYi = CG_CornerDebugPrint(scrPlace, posX, posYa, labelWidth, v15, (char *)"", colorWhite) + posYa;
    labelWidtha = (float)R_TextWidth(" ents client_once", 0, cgMedia.smallDevFont) * 1.0;
    v16 = va("ents client_once: %d\n", g_entsClientOnce);
    posYj = CG_CornerDebugPrint(scrPlace, posX, posYi, labelWidtha, v16, (char *)"", colorWhite) + posYi;
    labelWidthb = (float)R_TextWidth(" ents delta xxxxx", 0, cgMedia.smallDevFont) * 1.0;
    v17 = va("ents delta compared: %d\n", g_entsDeltaCompared);
    posYk = CG_CornerDebugPrint(scrPlace, posX, posYj, labelWidthb, v17, (char *)"", colorWhite) + posYj;
    v20 = (float)R_TextWidth(" ents transmitted", 0, cgMedia.smallDevFont) * 1.0;
    strd = va("ents transmitted: %d\n", g_entsTransmitted);
    return (float)(CG_CornerDebugPrint(scrPlace, posX, posYk, v20, strd, (char *)"", colorWhite) + posYk);
}

double __cdecl DrawSnapshotInfo(const ScreenPlacement *scrPlace, float posY)
{
    return posY;
}

int __cdecl CG_CompareSndInfoChannel(unsigned int *va, unsigned int *vb)
{
    if ( va[48] >= vb[48] )
        return va[48] > vb[48];
    else
        return -1;
}

int __cdecl CG_CompareSndInfoPriority(float *va, float *vb)
{
    if ( va[49] > vb[49] )
        return -1;
    if ( vb[49] <= va[49] )
        return CG_CompareSndInfoChannel((unsigned int*)va, (unsigned int *)vb);
    return 1;
}

int __cdecl CG_CompareSndInfoAlias(snd_overlay_info *va, snd_overlay_info *vb)
{
    int x; // [esp+8h] [ebp-4h]

    x = CG_CompareSndIsActive(va, vb);
    if ( x )
        return x;
    else
        return I_stricmp(va->name, vb->name);
}

int __cdecl CG_CompareSndIsActive(snd_overlay_info *a, snd_overlay_info *b)
{
    if ( a->pszSampleName[0] && !b->pszSampleName[0] )
        return -1;
    if ( !a->pszSampleName[0] && b->pszSampleName[0] )
        return 1;
    return !a->pszSampleName[0] && !b->pszSampleName[0];
}

int __cdecl CG_CompareSndInfoDryLevel(snd_overlay_info *va, snd_overlay_info *vb)
{
    int x; // [esp+8h] [ebp-4h]

    x = CG_CompareSndIsActive(va, vb);
    if ( x )
        return x;
    if ( va->fDry > vb->fDry )
        return -1;
    if ( vb->fDry <= va->fDry )
        return CG_CompareSndInfoChannel((unsigned int *)va, (unsigned int *)vb);
    return 1;
}

int __cdecl CG_CompareSndInfoDistance(snd_overlay_info *va, snd_overlay_info *vb)
{
    int x; // [esp+8h] [ebp-4h]

    x = CG_CompareSndIsActive(va, vb);
    if ( x )
        return x;
    if ( va->fDistance > vb->fDistance )
        return 1;
    if ( vb->fDistance <= va->fDistance )
        return CG_CompareSndInfoChannel((unsigned int *)va, (unsigned int *)vb);
    return -1;
}

int __cdecl CG_CompareSndInfoEntity(snd_overlay_info *va, snd_overlay_info *vb)
{
    int x; // [esp+8h] [ebp-4h]

    x = CG_CompareSndIsActive(va, vb);
    if ( x )
        return x;
    if ( va->iEntity > vb->iEntity )
        return -1;
    if ( va->iEntity >= vb->iEntity )
        return CG_CompareSndInfoChannel((unsigned int *)va, (unsigned int *)vb);
    return 1;
}

void __cdecl CG_DrawPerformanceWarnings()
{
    signed int time; // [esp+20h] [ebp-18h]
    const statmonitor_s *stats; // [esp+24h] [ebp-14h] BYREF
    int i; // [esp+28h] [ebp-10h]
    float x; // [esp+2Ch] [ebp-Ch]
    float y; // [esp+30h] [ebp-8h]
    int statCount; // [esp+34h] [ebp-4h] BYREF

    time = Sys_Milliseconds();
    StatMon_GetStatsArray(&stats, &statCount);
    x = 2.0f;
    y = 200.0f;
    for ( i = 0; i < statCount; ++i )
    {
        if ( stats[i].endtime >= time )
            UI_DrawHandlePic(&scrPlaceFull, x, y, 32.0, 32.0, 1, 1, 0, stats[i].material);
        x = x + 34.0;
        if ( (float)(x + 32.0) > 68.0 )
        {
            x = 2.0f;
            y = y + 34.0;
        }
    }
}

void __cdecl CG_DrawDebugOverlays(int localClientNum)
{
    if ( cg_drawModelAxis->current.integer != -1 )
        CG_DrawModelBoneAxis(localClientNum);
    if ( player_debugHealth->current.enabled )
        CG_DrawDebugPlayerHealth(localClientNum);
    AimAssist_DrawDebugOverlay(localClientNum);
    if ( player_debugSprint->current.enabled )
        CG_DrawDebugPlayerSprint(localClientNum);
}

void __cdecl CG_DrawModelBoneAxis(int localClientNum)
{
    const char *boneName; // [esp+14h] [ebp-10h] BYREF
    const cg_s *cgameGlob; // [esp+18h] [ebp-Ch]
    float x; // [esp+1Ch] [ebp-8h]
    float y; // [esp+20h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( R_PickEntityBone(
                 -1,
                 cgameGlob->refdef.vieworg,
                 cgameGlob->refdef.viewaxis[0],
                 cg_drawModelAxis->current.integer,
                 (char **)&boneName) )
    {
        x = 24.0f;
        y = 240.0f;
        CG_DrawSmallDevStringColor(&scrPlaceView[localClientNum], 24.0, 240.0, (char *)boneName, colorWhite, 5);
    }
}

void __cdecl CG_DrawDebugPlayerHealth(int localClientNum)
{
    float v1; // [esp+30h] [ebp-20h]
    float v2; // [esp+34h] [ebp-1Ch]
    cg_s *cgameGlob; // [esp+38h] [ebp-18h]
    float health; // [esp+3Ch] [ebp-14h]
    float healtha; // [esp+3Ch] [ebp-14h]
    float color[4]; // [esp+40h] [ebp-10h] BYREF

    if ( !player_debugHealth->current.enabled
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_debug.cpp",
                    1646,
                    0,
                    "%s",
                    "player_debugHealth->current.enabled") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->predictedPlayerState.stats[0] && cgameGlob->predictedPlayerState.stats[2] )
    {
        health = (float)cgameGlob->predictedPlayerState.stats[0] / (float)cgameGlob->predictedPlayerState.stats[2];
        if ( (float)(health - 1.0) < 0.0 )
            v2 = (float)cgameGlob->predictedPlayerState.stats[0] / (float)cgameGlob->predictedPlayerState.stats[2];
        else
            v2 = 1.0f;
        if ( (float)(0.0 - health) < 0.0 )
            v1 = v2;
        else
            v1 = 0.0f;
        healtha = v1;
    }
    else
    {
        healtha = 0.0f;
    }
    memset(color, 0, 12);
    color[3] = 1.0f;
    CL_DrawStretchPic(
        &scrPlaceView[localClientNum],
        10.0,
        10.0,
        100.0,
        10.0,
        1,
        1,
        0.0,
        0.0,
        1.0,
        1.0,
        color,
        cgMedia.whiteMaterial);
    color[0] = 0.0f;
    color[1] = 1.0f;
    color[2] = 0.0f;
    color[3] = 1.0f;
    CL_DrawStretchPic(
        &scrPlaceView[localClientNum],
        10.0,
        10.0,
        100.0 * healtha,
        10.0,
        1,
        1,
        0.0,
        0.0,
        healtha,
        1.0,
        color,
        cgMedia.whiteMaterial);
}

void __cdecl CG_DrawDebugPlayerSprint(int localClientNum)
{
    float lerp; // [esp+38h] [ebp-28h]
    int time; // [esp+40h] [ebp-20h]
    float timeRemain; // [esp+44h] [ebp-1Ch]
    float timeRemaina; // [esp+44h] [ebp-1Ch]
    const playerState_s *ps; // [esp+48h] [ebp-18h]
    float color[4]; // [esp+4Ch] [ebp-14h] BYREF
    float timeMax; // [esp+5Ch] [ebp-4h]

    time = CG_GetLocalClientGlobals(localClientNum)->time;
    ps = CG_GetPredictedPlayerState(localClientNum);
    if ( ps->sprintState.sprintDelay && time - ps->sprintState.lastSprintEnd < ps->sprintState.sprintCooldown )
    {
        timeRemain = (float)(time - ps->sprintState.lastSprintEnd);
        timeMax = (float)ps->sprintState.sprintCooldown;
        if ( timeMax == 0.0 )
            return;
        lerp = (float)((float)ps->sprintState.sprintCooldown - timeRemain) / timeMax;
        Vec4Lerp(colorYellow, colorRed, lerp, color);
    }
    else
    {
        timeRemaina = (float)PM_GetSprintLeft(ps, time);
        timeMax = (float)BG_GetMaxSprintTime(ps);
        if ( timeMax == 0.0 )
            return;
        lerp = timeRemaina / timeMax;
        Vec4Lerp(colorYellow, colorGreen, timeRemaina / timeMax, color);
    }
    color[3] = 0.75f;
    CL_DrawStretchPic(
        &scrPlaceView[localClientNum],
        0.0,
        0.0,
        700.0 * lerp,
        10.0,
        1,
        3,
        0.0,
        0.0,
        1.0,
        1.0,
        color,
        cgMedia.whiteMaterial);
}

void __cdecl CG_DrawFullScreenDebugOverlays()
{
    CG_DrawPerformanceWarnings();
    if ( fx_mark_profile->current.integer <= 0 )
    {
        if ( fx_profile->current.integer <= 0 )
        {
            if ( fx_priority_debug->current.integer <= 0 )
            {
                if ( snd_drawInfo->current.integer )
                {
                    CG_DrawSoundOverlay(&scrPlaceFull);
                }
                else if ( cg_drawScriptUsage->current.enabled )
                {
                    CG_DrawScriptUsage(&scrPlaceFull);
                }
                else
                {
                    Ragdoll_DebugDraw();
                    Destructible_DebugRender();
                    GlassCl_DrawDebug();
                    Bot_DrawDebug(&scrPlaceFull);
                }
            }
            else
            {
                CG_DrawFxPriorityDebug(fx_priority_debug->current.integer - 1);
            }
        }
        else
        {
            CG_DrawFxProfile(fx_profile->current.integer - 1);
        }
    }
    else
    {
        CG_DrawFxMarkProfile(fx_mark_profile->current.integer - 1);
    }
}

void __cdecl CG_DrawScriptUsage(const ScreenPlacement *scrPlace)
{
    int NumScriptVars; // eax
    char *v2; // eax
    int NumScriptVarsParent; // eax
    char *v4; // eax
    int NumScriptVarsChild; // eax
    char *v6; // eax
    int NumScriptThreads; // eax
    char *v8; // eax
    int StringUsage; // eax
    char *v10; // eax
    int v11; // eax
    char *v12; // eax
    int v13; // eax
    char *v14; // eax
    int v15; // eax
    char *v16; // eax
    int v17; // eax
    char *v18; // eax
    int v19; // eax
    char *v20; // eax

    NumScriptVars = Scr_GetNumScriptVars(SCRIPTINSTANCE_SERVER);
    v2 = va("num vars:        %d", NumScriptVars);
    CG_DrawStringExt(scrPlace, 480.0, 80.0, v2, colorWhite, 1, 1, 16.0);
    NumScriptVarsParent = Scr_GetNumScriptVarsParent(SCRIPTINSTANCE_SERVER);
    v4 = va("parent:        %d", NumScriptVarsParent);
    CG_DrawStringExt(scrPlace, 480.0, 96.0, v4, colorWhite, 1, 1, 16.0);
    NumScriptVarsChild = Scr_GetNumScriptVarsChild(SCRIPTINSTANCE_SERVER);
    v6 = va("child:        %d", NumScriptVarsChild);
    CG_DrawStringExt(scrPlace, 480.0, 112.0, v6, colorWhite, 1, 1, 16.0);
    NumScriptThreads = Scr_GetNumScriptThreads(SCRIPTINSTANCE_SERVER);
    v8 = va("num threads: %d", NumScriptThreads);
    CG_DrawStringExt(scrPlace, 480.0, 128.0, v8, colorWhite, 1, 1, 16.0);
    StringUsage = Scr_GetStringUsage(SCRIPTINSTANCE_SERVER);
    v10 = va("string usage: %d", StringUsage);
    CG_DrawStringExt(scrPlace, 480.0, 144.0, v10, colorWhite, 1, 1, 16.0);
    v11 = Scr_GetNumScriptVars(SCRIPTINSTANCE_CLIENT);
    v12 = va("num vars:        %d", v11);
    CG_DrawStringExt(scrPlace, 480.0, 160.0, v12, colorWhite, 1, 1, 16.0);
    v13 = Scr_GetNumScriptVarsParent(SCRIPTINSTANCE_CLIENT);
    v14 = va("parent:        %d", v13);
    CG_DrawStringExt(scrPlace, 480.0, 176.0, v14, colorWhite, 1, 1, 16.0);
    v15 = Scr_GetNumScriptVarsChild(SCRIPTINSTANCE_CLIENT);
    v16 = va("child:        %d", v15);
    CG_DrawStringExt(scrPlace, 480.0, 192.0, v16, colorWhite, 1, 1, 16.0);
    v17 = Scr_GetNumScriptThreads(SCRIPTINSTANCE_CLIENT);
    v18 = va("num threads: %d", v17);
    CG_DrawStringExt(scrPlace, 480.0, 208.0, v18, colorWhite, 1, 1, 16.0);
    v19 = Scr_GetStringUsage(SCRIPTINSTANCE_CLIENT);
    v20 = va("string usage: %d", v19);
    CG_DrawStringExt(scrPlace, 480.0, 224.0, v20, colorWhite, 1, 1, 16.0);
}

void __cdecl CG_DrawSoundOverlay(const ScreenPlacement *scrPlace)
{
    const char *v1; // [esp+44h] [ebp-782Ch]
    const char *v2; // [esp+48h] [ebp-7828h]
    snd_overlay_info *v3; // [esp+4Ch] [ebp-7824h]
    snd_overlay_info info[128]; // [esp+50h] [ebp-7820h] BYREF
    snd_overlay_type type; // [esp+7854h] [ebp-1Ch]
    int v6; // [esp+7858h] [ebp-18h]
    signed int i; // [esp+785Ch] [ebp-14h]
    float x; // [esp+7860h] [ebp-10h]
    float y; // [esp+7864h] [ebp-Ch]
    char *string; // [esp+7868h] [ebp-8h]
    unsigned int num; // [esp+786Ch] [ebp-4h]

    type = (snd_overlay_type)snd_drawInfo->current.integer;
    num = SND_GetSoundOverlay(type, info, 128);
    qsort(info, num, 0xF0u, CG_SoundCompares[snd_drawSort->current.integer]);
    v6 = 0;
    for ( i = 0; i < (int)num; ++i )
    {
        if ( info[i].pszSampleName[0] )
            ++v6;
    }
    x = 2.0f;
    y = 82.0f;
    string = va(
                         "%s sounds    reverb: %s    channels in use %d    sort by %s",
                         *(const char **)(snd_drawInfo->domain.integer.max + 4 * type),
                         "",
                         v6,
                         *(const char **)(snd_drawSort->domain.integer.max + 4 * type));
    CG_DrawStringExt(scrPlace, x, y, string, colorWhite, 0, 1, 14.0);
    y = y + 10.0;
    for ( i = 0; i < (int)num; ++i )
    {
        if ( info[i].pszSampleName[0] )
        {
            v3 = &info[i];
            v1 = &v3->pszSampleName[1];
            v2 = &v3->pszSampleName[strlen(v3->pszSampleName) + 1];
            if ( (unsigned int)(v2 - &v3->pszSampleName[1]) > 0x19 )
                v3 = (snd_overlay_info *)((char *)v3 + strlen(v3->pszSampleName) - 25);
            string = va(
                                 "%2i %-28s ^7pri^3% 4.0f ^7los^3%+.2d ^7d/w^3%+.2d/%+.2d ^7dst^3% 5.0f ^7pit^3% 1.2f ^7ent^3% .4d ^7%-25s",
                                 info[i].channel,
                                 info[i].name,
                                 info[i].fGlobalPriority,
                                 (int)info[i].fLosOcclusion,
                                 (int)info[i].fDry,
                                 (int)info[i].fWet,
                                 info[i].fDistance,
                                 info[i].fPitch,
                                 info[i].iEntity,
                                 v3->pszSampleName);
        }
        else
        {
            string = va("%2i", i);
        }
        CG_DrawStringExt(scrPlace, x, y, string, colorWhite, 0, 1, 14.0);
        y = y + 10.0;
    }
}

void __cdecl CG_DrawFxProfile(int localClientNum)
{
    float profilePos[2]; // [esp+0h] [ebp-8h] BYREF

    profilePos[0] = 0.0f;
    profilePos[1] = 9.0f;
    FX_DrawProfile(localClientNum, (void (__cdecl *)(char *))CG_DrawFxText, profilePos);
}

void __cdecl CG_DrawFxText(char *text, float *profilePos)
{
    if ( !text && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_debug.cpp", 1590, 0, "%s", "text") )
        __debugbreak();
    CL_DrawText(
        &scrPlaceFull,
        text,
        0x7FFFFFFF,
        cgMedia.smallDevFont,
        *profilePos,
        profilePos[1],
        1,
        1,
        0.75,
        0.75,
        colorWhiteFaded,
        128);
    profilePos[1] = profilePos[1] + 9.0;
}

void __cdecl CG_DrawFxMarkProfile(unsigned int localClientNum)
{
    float profilePos[2]; // [esp+0h] [ebp-8h] BYREF

    profilePos[0] = 0.0f;
    profilePos[1] = 9.0f;
    FX_DrawMarkProfile(localClientNum, (void (__cdecl *)(char *))CG_DrawFxText, profilePos);
}

void __cdecl CG_DrawFxPriorityDebug(int localClientNum)
{
    float profilePos[2]; // [esp+0h] [ebp-8h] BYREF

    profilePos[0] = 0.0f;
    profilePos[1] = 9.0f;
    FX_DrawPriorityDebug(localClientNum, (void (__cdecl *)(char *))CG_DrawFxPriorityText, profilePos);
}

void __cdecl CG_DrawFxPriorityText(char *text, float *profilePos)
{
    if ( !text && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_debug.cpp", 1600, 0, "%s", "text") )
        __debugbreak();
    CL_DrawText(
        &scrPlaceFull,
        text,
        0x7FFFFFFF,
        cgMedia.smallDevFont,
        *profilePos,
        profilePos[1],
        1,
        1,
        0.75,
        0.75,
        colorWhiteFaded,
        128);
    profilePos[1] = profilePos[1] + 9.0;
}

int offs = -70;

double __cdecl CG_DrawFPS(int localClientNum, const ScreenPlacement *scrPlace, float y, meminfo_t *meminfo)
{
    char *v4; // eax
    const float *frac_8; // [esp+14h] [ebp-108h]
    bool v7; // [esp+48h] [ebp-D4h]
    bool v8; // [esp+4Ch] [ebp-D0h]
    float frac; // [esp+50h] [ebp-CCh]
    float v10; // [esp+54h] [ebp-C8h]
    float v11; // [esp+58h] [ebp-C4h]
    float v12; // [esp+7Ch] [ebp-A0h]
    float v13; // [esp+80h] [ebp-9Ch]
    unsigned int numSModel; // [esp+B8h] [ebp-64h] BYREF
    char v15; // [esp+BFh] [ebp-5Dh]
    unsigned int numSModelMax; // [esp+C0h] [ebp-5Ch] BYREF
    float v17; // [esp+C4h] [ebp-58h]
    bool shouldDraw; // [esp+CBh] [ebp-51h]
    float ratio; // [esp+CCh] [ebp-50h]
    int maxAllowed; // [esp+D0h] [ebp-4Ch] BYREF
    const char *typeName; // [esp+D4h] [ebp-48h] BYREF
    int numUsed; // [esp+D8h] [ebp-44h] BYREF
    int type; // [esp+DCh] [ebp-40h]
    int fpsMax; // [esp+E0h] [ebp-3Ch]
    trViewStatistics_t *viewstats; // [esp+E4h] [ebp-38h]
    float freeMem; // [esp+E8h] [ebp-34h]
    int fps; // [esp+ECh] [ebp-30h]
    float farRight; // [esp+F0h] [ebp-2Ch]
    float labelWidth; // [esp+F4h] [ebp-28h]
    float lerp; // [esp+F8h] [ebp-24h]
    float varColor[4]; // [esp+FCh] [ebp-20h] BYREF
    unsigned int frameCount; // [esp+10Ch] [ebp-10h]
    int fpsMin; // [esp+110h] [ebp-Ch]
    char *s; // [esp+114h] [ebp-8h]
    const float *color; // [esp+118h] [ebp-4h]
    float yf; // [esp+12Ch] [ebp+10h]
    float ya; // [esp+12Ch] [ebp+10h]
    float yb; // [esp+12Ch] [ebp+10h]
    float yg; // [esp+12Ch] [ebp+10h]
    float yc; // [esp+12Ch] [ebp+10h]
    float yh; // [esp+12Ch] [ebp+10h]
    float yi; // [esp+12Ch] [ebp+10h]
    float yd; // [esp+12Ch] [ebp+10h]
    float yj; // [esp+12Ch] [ebp+10h]
    float yk; // [esp+12Ch] [ebp+10h]
    float yl; // [esp+12Ch] [ebp+10h]
    float ym; // [esp+12Ch] [ebp+10h]
    float ye; // [esp+12Ch] [ebp+10h]
    float yn; // [esp+12Ch] [ebp+10h]
    float yo; // [esp+12Ch] [ebp+10h]
    float yp; // [esp+12Ch] [ebp+10h]
    float yq; // [esp+12Ch] [ebp+10h]
    float yr; // [esp+12Ch] [ebp+10h]
    float ys; // [esp+12Ch] [ebp+10h]
    float yt; // [esp+12Ch] [ebp+10h]
    float yu; // [esp+12Ch] [ebp+10h]
    float yv; // [esp+12Ch] [ebp+10h]
    float yw; // [esp+12Ch] [ebp+10h]
    float yx; // [esp+12Ch] [ebp+10h]
    float yy; // [esp+12Ch] [ebp+10h]
    float yz; // [esp+12Ch] [ebp+10h]
    float yba; // [esp+12Ch] [ebp+10h]

    viewstats = &rendererViewStats[localClientNum];
    frameCount = R_GetFrameCount();
    farRight = (float)(scrPlace->virtualViewableMax[0] - scrPlace->virtualViewableMin[0])
                     + cg_debugInfoCornerOffset->current.value;
    labelWidth = (float)R_TextWidth("wwwwwwwwwwww", 0, cgMedia.smallDevFont) * 1.0;
    fpsMin = (int)((float)(1000.0 / (float)cg_perfInfo.frame.max) + 9.313225746154785e-10);
    fpsMax = (int)((float)(1000.0 / (float)cg_perfInfo.frame.min) + 9.313225746154785e-10);
    fps = (int)((float)(1000.0 / cg_perfInfo.frame.average) + 9.313225746154785e-10);
    v12 = (float)((float)((float)fps - 60.0) + 20.0) / 20.0;
    if ( (float)(v12 - 1.0) < 0.0 )
        v13 = (float)((float)((float)(int)((float)(1000.0 / cg_perfInfo.frame.average) + 9.313225746154785e-10) - 60.0)
                                + 20.0)
                / 20.0;
    else
        v13 = 1.0f;
    if ( (float)(0.0 - v12) < 0.0 )
        frac = v13;
    else
        frac = 0.0f;
    lerp = frac;
    Vec4Lerp(colorRed, colorWhite, frac, varColor);
    color = varColor;
    s = va("(%i-%i, %i) %i", fpsMin, fpsMax, (int)(cg_perfInfo.frame.variance + 9.313225746154785e-10), fps);
    yf = CG_CornerDebugPrint(scrPlace, farRight, y, labelWidth, s, (char*)" FPS", color) + y;
    s = va("(%i-%i) %1.2f", cg_perfInfo.frame.min, cg_perfInfo.frame.max, cg_perfInfo.frame.average);
    ya = CG_CornerDebugPrint(scrPlace, farRight, yf, labelWidth, s, (char*)" cg ms/frame", colorWhite) + yf;
    if ( cg_drawFPS->current.integer >= 2 )
    {
        s = va("(%2i-%2i) %1.2f", cg_perfInfo.server.min, cg_perfInfo.server.max, cg_perfInfo.server.average);
        if ( cg_perfInfo.server.average >= 40.0 )
        {
            if ( cg_perfInfo.server.average >= 50.0 )
                color = colorRed;
            else
                color = colorYellow;
        }
        else
        {
            color = colorWhite;
        }
        yb = CG_CornerDebugPrint(scrPlace, farRight, ya, labelWidth, s, (char *)" sv ms/frame", color) + ya;
        s = va("(%2i-%2i) %1.2f", cg_perfInfo.script.min, cg_perfInfo.script.max, cg_perfInfo.script.average);
        if ( cg_perfInfo.script.average >= 40.0 )
        {
            if ( cg_perfInfo.script.average >= 50.0 )
                color = colorRed;
            else
                color = colorYellow;
        }
        else
        {
            color = colorWhite;
        }
        ya = CG_CornerDebugPrint(scrPlace, farRight, yb, labelWidth, s, (char *)" script ms/frame", color) + yb;
    }
    freeMem = (float)((float)RETURN_ZERO32() / 1024.0) / 1024.0;
    if ( freeMem >= 5.0 )
    {
        if ( freeMem >= 10.0 )
            color = colorWhiteFaded;
        else
            color = colorRedFaded;
    }
    else if ( CG_Flash(500) )
    {
        color = colorRedFaded;
    }
    else
    {
        color = colorWhiteFaded;
    }
    s = va("%3.1f", freeMem);
    if ( cg_drawFPS->current.integer >= 2 )
        ya = CG_CornerDebugPrint(scrPlace, farRight, ya, labelWidth, s, (char *)" lvl free", colorWhite) + ya;
    s = va(
                "%d/%d",
                g_physics_system->m_list_rigid_body.m_list_count,
                g_physics_system->m_list_rigid_body.m_list_count_high_water);
    yg = CG_CornerDebugPrint(scrPlace, farRight, ya, labelWidth, s, (char *)" phys objects", colorWhite) + ya;
    s = va(
                "%d/%d",
                g_physics_system->m_list_rbc_ragdoll.m_list_count / 11,
                g_physics_system->m_list_rbc_ragdoll.m_list_count_high_water / 11);
    yc = CG_CornerDebugPrint(scrPlace, farRight, yg, labelWidth, s, (char *)" ragdolls", colorWhite) + yg;
    if ( cg_drawFPS->current.integer >= 2 )
    {
        s = va("(%2i-%2i) %1.2f", cg_perfInfo.cscript.min, cg_perfInfo.cscript.max, cg_perfInfo.cscript.average);
        if ( cg_perfInfo.cscript.average >= 40.0 )
        {
            if ( cg_perfInfo.cscript.average >= 50.0 )
                color = colorRed;
            else
                color = colorYellow;
        }
        else
        {
            color = colorWhite;
        }
        yh = CG_CornerDebugPrint(scrPlace, farRight, yc, labelWidth, s, (char*)" cscript ms/frame", color) + yc;
        rendererStats.views = rendererViewStats;
        R_TrackStatistics(&rendererStats);
        yi = CG_CornerDebugPrintCaption(scrPlace, farRight, yh, labelWidth, (char *)"-Scene-", colorGreenFaded) + yh;
        s = va("%i", viewstats->c_viewIndexes / 3);
        yd = CG_CornerDebugPrint(scrPlace, farRight, yi, labelWidth, s, (char *)" view tris", colorWhite) + yi;
        if ( viewstats->c_shadowIndexes )
        {
            s = va("%i", viewstats->c_shadowIndexes / 3);
            yd = CG_CornerDebugPrint(scrPlace, farRight, yd, labelWidth, s, (char *)" shadow tris", colorWhite) + yd;
        }
        s = va("%i", viewstats->c_indexes / 3);
        yj = CG_CornerDebugPrint(scrPlace, farRight, yd, labelWidth, s, (char *)" raw geo tris", colorWhite) + yd;
        s = va("%i", viewstats->c_fxIndexes / 3);
        yk = CG_CornerDebugPrint(scrPlace, farRight, yj, labelWidth, s, (char *)" raw fx tris", colorWhite) + yj;
        s = va("%i", viewstats->c_batches);
        yl = CG_CornerDebugPrint(scrPlace, farRight, yk, labelWidth, s, (char *)" prim", colorWhite) + yk;
        ym = CG_CornerDebugPrintCaption(scrPlace, farRight, yl, labelWidth, (char *)"-Level-", colorGreenFaded) + yl;
        s = va("%d", rendererStats.c_imageUsage.total / 0x100000);
        ye = CG_CornerDebugPrint(scrPlace, farRight, ym, labelWidth, s, (char *)" tex", colorWhite) + ym;
        if ( rendererStats.c_imageUsage.minspec > 50331648 )
        {
            color = colorRed;
            s = va("(!budget is %g!) %d", 48.0, rendererStats.c_imageUsage.minspec / 0x100000);
        }
        else
        {
            color = colorWhite;
            s = va("%d", rendererStats.c_imageUsage.minspec / 0x100000);
        }
        yn = CG_CornerDebugPrint(scrPlace, farRight, ye, labelWidth, s, (char *)" min pc tex", color) + ye;
        s = va("%i", meminfo->nonSwapMinSpecTotal / 0x100000);
        yc = CG_CornerDebugPrint(scrPlace, farRight, yn, labelWidth, s, (char *)" min pc mem", colorWhite) + yn;
    }
    if ( !r_reflectionProbeGenerate->current.enabled && cg_drawBudgets->current.integer > 0 )
    {
        typeName = 0;
        numUsed = 0;
        maxAllowed = 0;
        for ( type = 0; type < 43; ++type )
        {
            DB_GetAssetTypeUsageInfo((XAssetType)type, &typeName, &numUsed, &maxAllowed);
            ratio = (float)numUsed / (float)maxAllowed;
            switch ( cg_drawBudgets->current.integer )
            {
                case 1:
                    v8 = maxAllowed >= 32 && (ratio > 0.94999999 || numUsed >= maxAllowed - 1);
                    shouldDraw = v8;
                    break;
                case 2:
                    shouldDraw = maxAllowed >= 64;
                    break;
                case 3:
                    v7 = maxAllowed >= 32 && maxAllowed < 64;
                    shouldDraw = v7;
                    break;
                default:
                    shouldDraw = maxAllowed < 32;
                    break;
            }
            if ( shouldDraw )
            {
                if ( ratio < 0.94999999 )
                {
                    if ( ratio < 0.88 )
                        color = colorWhite;
                    else
                        color = colorYellow;
                }
                else
                {
                    color = colorRed;
                }
                s = va("%i / %i", numUsed, maxAllowed);
                frac_8 = color;
                v4 = va(" %s", typeName);
                yc = CG_CornerDebugPrint(scrPlace, farRight, yc, labelWidth, s, v4, frac_8) + yc;
            }
        }
    }
    if ( !r_reflectionProbeGenerate->current.enabled && cg_drawDynSModelBudget->current.enabled )
    {
        R_DynSModelGetCounts(&numSModel, &numSModelMax);
        v17 = (double)numSModel / (double)numSModelMax;
        v15 = 1;
        if ( v17 < 0.94999999 )
        {
            if ( v17 < 0.89999998 )
            {
                if ( v17 < 0.80000001 )
                    v15 = 0;
                else
                    color = colorYellow;
            }
            else
            {
                color = colorOrange;
            }
        }
        else
        {
            color = colorRed;
        }
        if ( v15 )
        {
            s = va("%i / %i", numSModel, numSModelMax);
            yc = CG_CornerDebugPrint(scrPlace, farRight, yc, labelWidth, s, (char *)" dyn smodel", color) + yc;
        }
    }
    if ( cg_profile_physics->current.enabled )
    {
        v11 = (double)sv_flame_proftimer.avr / tlPcTicksPerMS;
        s = va("%4.2f", v11);
        yo = CG_CornerDebugPrint(scrPlace, farRight, yc, labelWidth, s, (char *)" sv flame", colorWhite) + yc;
        s = va("%lld", sv_flame_proftimer.calls);
        yp = CG_CornerDebugPrint(scrPlace, farRight, yo, labelWidth, s, (char *)" sv calls", colorWhite) + yo;
        v10 = (double)cl_flame_proftimer.avr / tlPcTicksPerMS;
        s = va("%4.2f", v10);
        yq = CG_CornerDebugPrint(scrPlace, farRight, yp, labelWidth, s, (char *)" cl flame", colorWhite) + yp;
        s = va("%lld", cl_flame_proftimer.calls);
        yc = CG_CornerDebugPrint(scrPlace, farRight, yq, labelWidth, s, (char *)" cl calls", colorWhite) + yq;
    }
    if ( g_bDebugRenderEntityBrushes->current.enabled )
    {
        yr = CG_CornerDebugPrint(
                     scrPlace,
                     farRight,
                     yc,
                     (float)offs + labelWidth,
            (char *)"                                     ",
                     (char *)"",
                     colorWhite)
             + yc;
        ys = CG_CornerDebugPrint(
                     scrPlace,
                     farRight,
                     yr,
                     (float)offs + labelWidth,
            (char *)"** ENTITY BRUSHES *",
                     (char *)"",
                     colorWhite)
             + yr;
        yt = CG_CornerDebugPrint(
                     scrPlace,
                     farRight,
                     ys,
                     (float)offs + labelWidth,
            (char *)"*                                 *",
                     (char *)"",
                     colorWhite)
             + ys;
        yu = CG_CornerDebugPrint(
                     scrPlace,
                     farRight,
                     yt,
                     (float)offs + labelWidth,
            (char *)"*    server ents        *",
                     (char *)"",
                     colorCyan)
             + yt;
        yv = CG_CornerDebugPrint(
                     scrPlace,
                     farRight,
                     yu,
                     (float)offs + labelWidth,
            (char *)"*    server trigs     *",
                     (char *)"",
                     colorRed)
             + yu;
        yw = CG_CornerDebugPrint(
                     scrPlace,
                     farRight,
                     yv,
                     (float)offs + labelWidth,
            (char *)"*    dyn ents             *",
                     (char *)"",
                     colorGreen)
             + yv;
        yx = CG_CornerDebugPrint(
                     scrPlace,
                     farRight,
                     yw,
                     (float)offs + labelWidth,
            (char *)"*    cl only ents     *",
                     (char *)"",
                     colorYellow)
             + yw;
        yy = CG_CornerDebugPrint(
                     scrPlace,
                     farRight,
                     yx,
                     (float)offs + labelWidth,
            (char *)"*    client trigs     *",
                     (char *)"",
                     colorWhite)
             + yx;
        yz = CG_CornerDebugPrint(
                     scrPlace,
                     farRight,
                     yy,
                     (float)offs + labelWidth,
            (char *)"*                                 *",
                     (char *)"",
                     colorWhite)
             + yy;
        yba = CG_CornerDebugPrint(
                        scrPlace,
                        farRight,
                        yz,
                        (float)offs + labelWidth,
            (char *)"*******************",
                        (char *)"",
                        colorWhite)
                + yz;
        return (float)(CG_CornerDebugPrint(
                                         scrPlace,
                                         farRight,
                                         yba,
                                         (float)offs + labelWidth,
            (char *)"                                     ",
                                         (char *)"",
                                         colorWhite)
                                 + yba);
    }
    return yc;
}

bool __cdecl CG_Flash(int timeMs)
{
    return (int)Sys_Milliseconds() % (2 * timeMs) < timeMs;
}

void __cdecl CG_DrawVersion()
{
    Font_s *font; // [esp+1Ch] [ebp-30h]
    float shadowColor[4]; // [esp+20h] [ebp-2Ch] BYREF
    float color[4]; // [esp+30h] [ebp-1Ch] BYREF
    float fontScale; // [esp+40h] [ebp-Ch]
    float h; // [esp+44h] [ebp-8h]
    float w; // [esp+48h] [ebp-4h]

    fontScale = 0.25f;
    color[0] = 0.4f;
    color[1] = 0.7f;
    color[2] = 1.0f;
    color[3] = 0.7f;
    memset(shadowColor, 0, 12);
    shadowColor[3] = 0.7f;
    font = UI_GetFontHandle(&scrPlaceFullUnsafe, 0, 0.5);
    if ( font )
    {
        w = (float)UI_TextWidth(version->current.string, 0, font, fontScale);
        h = (float)UI_TextHeight(font, fontScale);
        UI_DrawText(
            &scrPlaceFullUnsafe,
            (char *)version->current.integer,
            0x7FFFFFFF,
            font,
            (float)(-(w) - cg_drawVersionX->current.value) + 1.0,
            (float)(-(h) - cg_drawVersionY->current.value) + 1.0,
            3,
            3,
            fontScale,
            shadowColor,
            0);
        UI_DrawText(
            &scrPlaceFullUnsafe,
            (char *)version->current.integer,
            0x7FFFFFFF,
            font,
            -(w) - cg_drawVersionX->current.value,
            -(h) - cg_drawVersionY->current.value,
            3,
            3,
            fontScale,
            color,
            0);
    }
}

unsigned int quick_print_buffer_pos;
char quick_print_buffer[5120];

void CG_QuickPrint(const char *fmt, ...)
{
    va_list va; // [esp+14h] [ebp+Ch] BYREF

    va_start(va, fmt);
    quick_print_buffer_pos += _vsnprintf(
                                                            &quick_print_buffer[quick_print_buffer_pos],
                                                            5119 - quick_print_buffer_pos,
                                                            fmt,
                                                            va);
    if ( (int)quick_print_buffer_pos >= 3840 )
        CG_QuickPrintFlush();
}

FILE *file;
unsigned int _S2_0;
void __cdecl CG_QuickPrintFlush()
{
    if ( quick_print_buffer_pos )
    {
        if ( (_S2_0 & 1) == 0 )
        {
            _S2_0 |= 1u;
            file = fopen("quickprint.log", "wt");
        }
        fwrite(quick_print_buffer, quick_print_buffer_pos, 1u, file);
        quick_print_buffer_pos = 0;
        quick_print_buffer[0] = 0;
    }
}

