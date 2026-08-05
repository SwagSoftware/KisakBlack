#include "cg_ents_mp.h"
#include "cg_main_mp.h"
#include <xanim/dobj_utils.h>
#include <clientscript/scr_const.h>
#include <sound/snd_bank.h>
#include <cgame/cg_sound.h>
#include <cgame/cg_event.h>
#include <gfx_d3d/r_scene.h>
#include <universal/com_math_anglevectors.h>
#include <bgame/bg_local.h>
#include <xanim/xmodel.h>
#include <bgame/bg_slidemove.h>
#include <qcommon/dobj_management.h>
#include <demo/demo_playback.h>
#include "cg_scr_main_mp.h"
#include <game/g_debug.h>
#include <xanim/xanim_clientnotify.h>
#include <clientscript/cscr_vm.h>
#include <cgame/cg_scr_main.h>
#include "cg_animscripted_mp.h"
#include "cg_vehicles_mp.h"
#include <cgame/cg_colltree.h>
#include <cgame/cg_world.h>
#include <EffectsCore/fx_marks.h>
#include <cgame/cg_drawtools.h>
#include "cg_actors_mp.h"
#include <EffectsCore/fx_unique_handle.h>
#include <EffectsCore/fx_system.h>
#include <qcommon/com_bsp.h>
#include <client/splitscreen.h>
#include <ragdoll/ragdoll.h>
#include <server_mp/sv_init_mp.h>
#include <cgame/cg_main.h>
#include <clientscript/cscr_memorytree.h>
#include <gfx_d3d/r_workercmds_common.h>
#include <cgame/cg_vehicle.h>
#include <bgame/bg_weapons_ammo.h>
#include <cgame/cg_draw_indicators.h>
#include <turret/turret_placement.h>
#include <client_mp/cl_cgame_mp.h>
#include <cgame/cg_weapon_options.h>
#include "cg_players_mp.h"
#include "cg_pose_mp.h"
#include <xanim/dobj_skel.h>
#include <bgame/bg_dog_animations_mp.h>
#include "cg_animtree_mp.h"
#include <ragdoll/ragdoll_update.h>
#include <physics/destructible.h>
#include "cg_draw_mp.h"

const unsigned short *controller_names[6] =
{
    &scr_const.back_low,
    &scr_const.back_mid,
    &scr_const.back_up,
    &scr_const.neck,
    &scr_const.head,
    &scr_const.pelvis,
};

int g_processEvents;

unsigned int __cdecl CG_GetShadowHintForRefEntity(__int16 eFlags)
{
    unsigned int v2; // [esp+0h] [ebp-4h]

    v2 = eFlags & 0xE000;
    if ( v2 > 0x6000 )
    {
        if ( v2 == 0x8000 )
            return 2048;
        if ( v2 == 40960 )
            return 256;
    }
    else
    {
        switch ( v2 )
        {
            case 0x6000u:
                return 1536;
            case 0x2000u:
                return 512;
            case 0x4000u:
                return 1024;
        }
    }
    return 0;
}

void __cdecl CG_Player_PreControllers(DObj *obj, centity_s *cent)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-10h]
    clientInfo_t *ci; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(cent->pose.localClientNum);
    cent->pose.eType = cent->nextState.eType;
    if ( cent->pose.eType != cent->nextState.eType
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    325,
                    0,
                    "%s",
                    "cent->pose.eType == cent->nextState.eType") )
    {
        __debugbreak();
    }
    if ( cent->nextState.clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    327,
                    0,
                    "es->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cent->nextState.clientNum,
                    32) )
    {
        __debugbreak();
    }
    ci = &cgameGlob->bgs.clientinfo[cent->nextState.clientNum];
    if ( ci->infoValid )
    {
        BG_Player_DoControllersSetup(&cent->nextState, ci, cgameGlob->frametime);
        for ( i = 0; i < 6; ++i )
            DObjGetBoneIndex(obj, *controller_names[i], &cent->pose.player.tag[i], -1);
        cent->pose.fx.triggerTime = (int)&ci->control;
    }
    else
    {
        cent->pose.fx.triggerTime = 0;
    }
}

void __cdecl CG_mg42_OverheatUpdate(int localClientNum, const DObj *obj, centity_s *cent)
{
    unsigned int AliasId; // eax
    char *overheatSoundAlias; // [esp+8h] [ebp-Ch]
    const FxEffectDef *overheatEffect; // [esp+Ch] [ebp-8h]
    const WeaponDef *weapDef; // [esp+10h] [ebp-4h]

    if ( cent->nextState.lerp.u.turret.overheating )
    {
        if ( ((*((unsigned int *)cent + 201) >> 10) & 1) == 0 )
        {
            weapDef = BG_GetWeaponDef(cent->nextState.weapon);
            overheatSoundAlias = (char *)weapDef->overheatSound;
            overheatEffect = weapDef->worldLastShotEjectEffect;
            if ( CG_GetLocalClientGlobals(localClientNum)->predictedPlayerState.viewlocked_entNum == cent->nextState.number )
            {
                overheatSoundAlias = (char *)weapDef->overheatSoundPlayer;
                overheatEffect = weapDef->viewLastShotEjectEffect;
            }
            AliasId = SND_FindAliasId(overheatSoundAlias);
            CG_PlaySound(localClientNum, cent->nextState.number, cent->pose.origin, 0, 0, 1.0, AliasId);
            if ( overheatEffect )
                CG_PlayBoltedEffect(localClientNum, overheatEffect, cent->nextState.number, scr_const.tag_flash);
            *((unsigned int *)cent + 201) |= 0x400u;
        }
    }
    else
    {
        *((unsigned int *)cent + 201) &= ~0x400u;
    }
}

float firingOffset = -1.0f;
float firingRate = 0.15f;

void __cdecl CG_mg42_PreControllers(DObj *obj, centity_s *cent)
{
    bool v2; // [esp+4h] [ebp-64h]
    float v3; // [esp+8h] [ebp-60h]
    float v4; // [esp+10h] [ebp-58h]
    float v5; // [esp+18h] [ebp-50h]
    float v6; // [esp+20h] [ebp-48h]
    float v7; // [esp+28h] [ebp-40h]
    float frameInterpolation; // [esp+30h] [ebp-38h]
    float fc; // [esp+48h] [ebp-20h]
    const cg_s *cgameGlob; // [esp+54h] [ebp-14h]

    if ( cent->nextState.eType != 11
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    384,
                    0,
                    "%s",
                    "cent->nextState.eType == ET_MG42") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(cent->pose.localClientNum);
    v2 = (cgameGlob->predictedPlayerState.eFlags & 0x300) != 0
        && cgameGlob->predictedPlayerState.viewlocked_entNum == cent->nextState.number
        && cgameGlob->renderingThirdPerson == TP_OFF;
    cent->pose.turret.playerUsing = v2;
    if ( (cent->nextState.lerp.eFlags & 0x40) != 0 )
    {
        fc = cos((float)((float)((float)((float)(360.0 * (float)((float)cgameGlob->time * 0.001)) / firingRate) - 180.0)
                                     * 0.017453292));
        cent->pose.turret.firingOffset = (float)((float)(fc + 1.0) * firingOffset) / 2.0;
    }
    else
    {
        cent->pose.player.nextRippleTime = 0;
    }
    if ( cent->pose.turret.playerUsing )
    {
        cent->pose.fx.triggerTime = (int)cgameGlob->predictedPlayerState.viewangles;
        cent->pose.cullIn = 0;
        cent->pose.turret.barrelPitch = 0.0f;
    }
    else
    {
        v7 = cent->currentState.u.turret.gunAngles[0];
        frameInterpolation = cgameGlob->frameInterpolation;
        cent->pose.turret.angles.pitch = AngleNormalize180(cent->nextState.lerp.u.turret.gunAngles[0] - v7)
                                                                     * frameInterpolation
                                                                     + v7;
        v5 = cent->currentState.u.turret.gunAngles[1];
        v6 = cgameGlob->frameInterpolation;
        cent->pose.turret.angles.yaw = AngleNormalize180(cent->nextState.lerp.u.turret.gunAngles[1] - v5) * v6 + v5;
        v3 = cent->currentState.u.turret.gunAngles[2];
        v4 = cgameGlob->frameInterpolation;
        cent->pose.turret.barrelPitch = AngleNormalize180(cent->nextState.lerp.u.turret.gunAngles[2] - v3) * v4 + v3;
    }
    DObjGetBoneIndex(obj, scr_const.tag_aim, &cent->pose.turret.tag_aim, -1);
    DObjGetBoneIndex(obj, scr_const.tag_aim_pitch, &cent->pose.turret.tag_aim_pivot, -1);
    DObjGetBoneIndex(obj, scr_const.tag_aim_animated, &cent->pose.turret.tag_aim_animated, -1);
    DObjGetBoneIndex(obj, scr_const.tag_flash, &cent->pose.turret.tag_flash, -1);
    DObjGetBoneIndex(obj, scr_const.tag_weapon, &cent->pose.turret.tag_weapon, -1);
    DObjGetTree(obj);
    cent->pose.player.waterHeight = cent->nextState.lerp.u.turret.pivotOffset;
}

float g_entMoveTolVec[3] = { 16.0, 16.0, 16.0 };

#if 0
// local variable allocation has failed, the output may be wrong!
void    CG_UpdateBModelWorldBounds(int localClientNum, centity_s *cent, int forceFilter)
{
    int v4; // [esp-14h] [ebp-1F8h]
    __int64 v5; // [esp-8h] [ebp-1ECh] BYREF
    float HARD_CODED_MIP1_RADIUS; // [esp+0h] [ebp-1E4h]
    float maxs[4]; // [esp+4h] [ebp-1E0h] BYREF
    float mins[8]; // [esp+14h] [ebp-1D0h]
    float v9; // [esp+34h] [ebp-1B0h]
    float v10; // [esp+38h] [ebp-1ACh]
    float v11; // [esp+3Ch] [ebp-1A8h]
    float4 rotatedBounds[2]; // [esp+40h] [ebp-1A4h] BYREF
    int v13; // [esp+60h] [ebp-184h]
    __int64 v14; // [esp+64h] [ebp-180h]
    int v15; // [esp+6Ch] [ebp-178h]
    int v16; // [esp+70h] [ebp-174h]
    int v17; // [esp+74h] [ebp-170h]
    int v18; // [esp+78h] [ebp-16Ch]
    int v19; // [esp+7Ch] [ebp-168h]
    __int64 v20; // [esp+80h] [ebp-164h]
    int v21; // [esp+88h] [ebp-15Ch]
    int v22; // [esp+8Ch] [ebp-158h]
    int v23; // [esp+90h] [ebp-154h]
    _BYTE *v24; // [esp+94h] [ebp-150h]
    __int64 v25; // [esp+98h] [ebp-14Ch]
    int v26; // [esp+A0h] [ebp-144h]
    int v27; // [esp+A4h] [ebp-140h]
    __int64 v28; // [esp+A8h] [ebp-13Ch]
    int v29; // [esp+B0h] [ebp-134h]
    int v30; // [esp+B4h] [ebp-130h]
    int v31; // [esp+B8h] [ebp-12Ch]
    int v32; // [esp+BCh] [ebp-128h]
    int v33; // [esp+C0h] [ebp-124h]
    __int64 v34; // [esp+C4h] [ebp-120h]
    int v35; // [esp+CCh] [ebp-118h]
    int v36; // [esp+D0h] [ebp-114h]
    int v37; // [esp+D4h] [ebp-110h]
    _BYTE *v38; // [esp+D8h] [ebp-10Ch]
    __int64 v39; // [esp+DCh] [ebp-108h]
    int v40; // [esp+E4h] [ebp-100h]
    int v41; // [esp+E8h] [ebp-FCh]
    __int64 v42; // [esp+ECh] [ebp-F8h]
    int v43; // [esp+F4h] [ebp-F0h]
    int v44; // [esp+F8h] [ebp-ECh]
    int v45; // [esp+FCh] [ebp-E8h]
    int v46; // [esp+100h] [ebp-E4h]
    int v47; // [esp+104h] [ebp-E0h]
    __int64 v48; // [esp+108h] [ebp-DCh]
    int v49; // [esp+110h] [ebp-D4h]
    int v50; // [esp+114h] [ebp-D0h]
    int v51; // [esp+118h] [ebp-CCh]
    __int64 v52; // [esp+11Ch] [ebp-C8h]
    int v53; // [esp+124h] [ebp-C0h]
    int v54; // [esp+128h] [ebp-BCh]
    _BYTE *v55; // [esp+12Ch] [ebp-B8h]
    __int64 v56; // [esp+130h] [ebp-B4h]
    int v57; // [esp+138h] [ebp-ACh]
    int v58; // [esp+13Ch] [ebp-A8h]
    float v59; // [esp+140h] [ebp-A4h]
    float v60; // [esp+144h] [ebp-A0h]
    float v61; // [esp+148h] [ebp-9Ch]
    float v62; // [esp+14Ch] [ebp-98h]
    __int64 v63; // [esp+150h] [ebp-94h]
    float v64; // [esp+158h] [ebp-8Ch]
    float v65; // [esp+15Ch] [ebp-88h]
    _BYTE *v66; // [esp+160h] [ebp-84h]
    __int64 v67; // [esp+164h] [ebp-80h]
    float v68; // [esp+16Ch] [ebp-78h]
    float v69; // [esp+170h] [ebp-74h]
    _BYTE *v70; // [esp+174h] [ebp-70h]
    float v71; // [esp+178h] [ebp-6Ch]
    float v72; // [esp+17Ch] [ebp-68h]
    float v73; // [esp+180h] [ebp-64h]
    float v74; // [esp+184h] [ebp-60h]
    float *origin; // [esp+188h] [ebp-5Ch]
    float v76[3]; // [esp+18Ch] [ebp-58h] BYREF
    _BYTE axis[68]; // [esp+198h] [ebp-4Ch] OVERLAPPED BYREF
    GfxBrushModel *bmodel; // [esp+1DCh] [ebp-8h]
    GfxBrushModel *retaddr; // [esp+1E4h] [ebp+0h]

    *(unsigned int *)&axis[64] = a1;
    bmodel = retaddr;
    *(unsigned int *)&axis[56] = R_GetBrushModel(cent->nextState.index.brushmodel);
    *(_QWORD *)&axis[24] = *(_QWORD *)(*(unsigned int *)&axis[56] + 28);
    *(_QWORD *)&axis[32] = *(_QWORD *)(*(unsigned int *)&axis[56] + 36);
    *(_QWORD *)&axis[40] = *(_QWORD *)(*(unsigned int *)&axis[56] + 40);
    *(_QWORD *)&axis[48] = *(_QWORD *)(*(unsigned int *)&axis[56] + 48);
    AnglesToAxis(cent->pose.angles, (float (*)[3])v76);
    origin = cent->pose.origin;
    v71 = v76[0];
    v72 = v76[1];
    v73 = v76[2];
    v74 = 0.0f;
    v70 = axis;
    v67 = *(_QWORD *)axis;
    v68 = *(float *)&axis[8];
    v69 = 0.0f;
    v66 = &axis[12];
    v63 = *(_QWORD *)&axis[12];
    v64 = *(float *)&axis[20];
    v65 = 0.0f;
    v59 = cent->pose.origin[0];
    v60 = cent->pose.origin[1];
    v61 = cent->pose.origin[2];
    v62 = 0.0f;
    LODWORD(v56) = *(unsigned int *)&axis[24];
    HIDWORD(v56) = *(unsigned int *)&axis[24];
    v57 = *(unsigned int *)&axis[24];
    v58 = *(unsigned int *)&axis[24];
    v55 = &axis[40];
    LODWORD(v52) = *(unsigned int *)&axis[40];
    HIDWORD(v52) = *(unsigned int *)&axis[40];
    v53 = *(unsigned int *)&axis[40];
    v54 = *(unsigned int *)&axis[40];
    if ( v76[0] >= 0.0 )
        v51 = 0;
    else
        v51 = -1;
    LODWORD(v48) = v51;
    if ( v72 >= 0.0 )
        v47 = 0;
    else
        v47 = -1;
    HIDWORD(v48) = v47;
    if ( v73 >= 0.0 )
        v46 = 0;
    else
        v46 = -1;
    v49 = v46;
    if ( v74 >= 0.0 )
        v45 = 0;
    else
        v45 = -1;
    v50 = v45;
    v42 = v52 & v48 | v56 & ~v48;
    v43 = v53 & v49 | v57 & ~v49;
    v44 = v54 & v50 | v58 & ~v50;
    v39 = v56 & v48 | v52 & ~v48;
    v40 = v57 & v49 | v53 & ~v49;
    v41 = v58 & v50 | v54 & ~v50;
    LODWORD(v56) = *(unsigned int *)&axis[28];
    HIDWORD(v56) = *(unsigned int *)&axis[28];
    v57 = *(unsigned int *)&axis[28];
    v58 = *(unsigned int *)&axis[28];
    v38 = &axis[40];
    LODWORD(v52) = *(unsigned int *)&axis[44];
    HIDWORD(v52) = *(unsigned int *)&axis[44];
    v53 = *(unsigned int *)&axis[44];
    v54 = *(unsigned int *)&axis[44];
    if ( *(float *)&v67 >= 0.0 )
        v37 = 0;
    else
        v37 = -1;
    LODWORD(v34) = v37;
    if ( *((float *)&v67 + 1) >= 0.0 )
        v33 = 0;
    else
        v33 = -1;
    HIDWORD(v34) = v33;
    if ( v68 >= 0.0 )
        v32 = 0;
    else
        v32 = -1;
    v35 = v32;
    if ( v69 >= 0.0 )
        v31 = 0;
    else
        v31 = -1;
    v36 = v31;
    v28 = v52 & v34 | v56 & ~v34;
    v29 = v53 & v35 | v57 & ~v35;
    v30 = v54 & v36 | v58 & ~v36;
    v25 = v56 & v34 | v52 & ~v34;
    v26 = v57 & v35 | v53 & ~v35;
    v27 = v58 & v36 | v54 & ~v36;
    LODWORD(v56) = *(unsigned int *)&axis[32];
    HIDWORD(v56) = *(unsigned int *)&axis[32];
    v57 = *(unsigned int *)&axis[32];
    v58 = *(unsigned int *)&axis[32];
    v24 = &axis[40];
    LODWORD(v52) = *(unsigned int *)&axis[48];
    HIDWORD(v52) = *(unsigned int *)&axis[48];
    v53 = *(unsigned int *)&axis[48];
    v54 = *(unsigned int *)&axis[48];
    if ( *(float *)&v63 >= 0.0 )
        v23 = 0;
    else
        v23 = -1;
    LODWORD(v20) = v23;
    if ( *((float *)&v63 + 1) >= 0.0 )
        v19 = 0;
    else
        v19 = -1;
    HIDWORD(v20) = v19;
    if ( v64 >= 0.0 )
        v18 = 0;
    else
        v18 = -1;
    v21 = v18;
    if ( v65 >= 0.0 )
        v17 = 0;
    else
        v17 = -1;
    v22 = v17;
    v14 = v52 & v20 | v56 & ~v20;
    v15 = v53 & v21 | v57 & ~v21;
    v16 = v54 & v22 | v58 & ~v22;
    *(_QWORD *)&rotatedBounds[1].unitVec[1].packed = v56 & v20 | v52 & ~v20;
    rotatedBounds[1].u[3] = v57 & v21 | v53 & ~v21;
    v13 = v58 & v22 | v54 & ~v22;
    v9 = (float)(*(float *)&v14 * *(float *)&v63)
         + (float)((float)(*(float *)&v28 * *(float *)&v67) + (float)((float)(*(float *)&v42 * v71) + v59));
    v10 = (float)(*((float *)&v14 + 1) * *((float *)&v63 + 1))
            + (float)((float)(*((float *)&v28 + 1) * *((float *)&v67 + 1)) + (float)((float)(*((float *)&v42 + 1) * v72) + v60));
    v11 = (float)(*(float *)&v15 * v64)
            + (float)((float)(*(float *)&v29 * v68) + (float)((float)(*(float *)&v43 * v73) + v61));
    rotatedBounds[0].v[0] = (float)(*(float *)&v16 * v65)
                                                + (float)((float)(*(float *)&v30 * v69) + (float)((float)(*(float *)&v44 * v74) + v62));
    LODWORD(mins[7]) = &rotatedBounds[0].v[1];
    rotatedBounds[0].v[1] = (float)(*(float *)&v39 * v71) + v59;
    rotatedBounds[0].v[2] = (float)(*((float *)&v39 + 1) * v72) + v60;
    rotatedBounds[0].v[3] = (float)(*(float *)&v40 * v73) + v61;
    rotatedBounds[1].v[0] = (float)(*(float *)&v41 * v74) + v62;
    LODWORD(mins[6]) = &rotatedBounds[0].v[1];
    LODWORD(mins[5]) = &rotatedBounds[0].v[1];
    rotatedBounds[0].v[1] = (float)(*(float *)&v25 * *(float *)&v67) + rotatedBounds[0].v[1];
    rotatedBounds[0].v[2] = (float)(*((float *)&v25 + 1) * *((float *)&v67 + 1)) + rotatedBounds[0].v[2];
    rotatedBounds[0].v[3] = (float)(*(float *)&v26 * v68) + rotatedBounds[0].v[3];
    rotatedBounds[1].v[0] = (float)(*(float *)&v27 * v69) + rotatedBounds[1].v[0];
    LODWORD(mins[4]) = &rotatedBounds[0].v[1];
    LODWORD(mins[3]) = &rotatedBounds[0].v[1];
    rotatedBounds[0].v[1] = (float)(rotatedBounds[1].v[1] * *(float *)&v63) + rotatedBounds[0].v[1];
    rotatedBounds[0].v[2] = (float)(rotatedBounds[1].v[2] * *((float *)&v63 + 1)) + rotatedBounds[0].v[2];
    rotatedBounds[0].v[3] = (float)(rotatedBounds[1].v[3] * v64) + rotatedBounds[0].v[3];
    rotatedBounds[1].v[0] = (float)(*(float *)&v13 * v65) + rotatedBounds[1].v[0];
    maxs[1] = v9;
    maxs[2] = v10;
    maxs[3] = v11;
    mins[0] = rotatedBounds[0].v[0];
    v5 = *(_QWORD *)&rotatedBounds[0].unitVec[1].packed;
    HARD_CODED_MIP1_RADIUS = rotatedBounds[0].v[3];
    maxs[0] = rotatedBounds[1].v[0];
    if ( forceFilter )
        goto LABEL_41;
    if ( !VecLessThan(*(float **)&axis[56], &maxs[1]) || !VecLessThan((float *)&v5, (float *)(*(unsigned int *)&axis[56] + 12)) )
    {
        maxs[1] = maxs[1] - g_entMoveTolVec[0];
        maxs[2] = maxs[2] - g_entMoveTolVec[1];
        maxs[3] = maxs[3] - g_entMoveTolVec[2];
        *(float *)&v5 = *(float *)&v5 + g_entMoveTolVec[0];
        *((float *)&v5 + 1) = *((float *)&v5 + 1) + g_entMoveTolVec[1];
        HARD_CODED_MIP1_RADIUS = HARD_CODED_MIP1_RADIUS + g_entMoveTolVec[2];
LABEL_41:
        *(float *)(*(unsigned int *)&axis[56] + 24) = 100.0 * 100.0;
        maxs[1] = maxs[1] + COERCE_FLOAT(LODWORD(100.0f) ^ _mask__NegFloat_);
        maxs[2] = maxs[2] + COERCE_FLOAT(LODWORD(100.0f) ^ _mask__NegFloat_);
        maxs[3] = maxs[3] + COERCE_FLOAT(LODWORD(100.0f) ^ _mask__NegFloat_);
        *(float *)&v5 = *(float *)&v5 + 100.0;
        *((float *)&v5 + 1) = *((float *)&v5 + 1) + 100.0;
        HARD_CODED_MIP1_RADIUS = HARD_CODED_MIP1_RADIUS + 100.0;
        **(_QWORD **)&axis[56] = *(_QWORD *)&maxs[1];
        *(float *)(*(unsigned int *)&axis[56] + 8) = maxs[3];
        v4 = *(unsigned int *)&axis[56] + 12;
        *(_QWORD *)(*(unsigned int *)&axis[56] + 12) = v5;
        *(float *)(v4 + 8) = HARD_CODED_MIP1_RADIUS;
        R_LinkBModelEntity(localClientNum, cent->nextState.number, *(GfxBrushModel **)&axis[56]);
    }
}
#endif

// aislop - fixing the stack here is aids since it's reused
void CG_UpdateBModelWorldBounds(
    int localClientNum,
    centity_s *cent,
    int forceFilter
)
{
    GfxBrushModel *model;
    float axis[3][3];
    float mins[3], maxs[3];
    float rotatedMins[3], rotatedMaxs[3];
    float modelMins[3], modelMaxs[3];
    const float *origin;

    model = R_GetBrushModel((unsigned short)cent->nextState.index.brushmodel);

    // local-space bounds
    modelMins[0] = model->bounds[0][0];
    modelMins[1] = model->bounds[0][1];
    modelMins[2] = model->bounds[0][2];

    modelMaxs[0] = model->bounds[1][0];
    modelMaxs[1] = model->bounds[1][1];
    modelMaxs[2] = model->bounds[1][2];

    // orientation
    AnglesToAxis(cent->pose.angles, axis);
    origin = cent->pose.origin;

    // rotate AABB (branchless sign-selection)
    for (int i = 0; i < 3; ++i)
    {
        rotatedMins[i] = origin[i];
        rotatedMaxs[i] = origin[i];

        for (int j = 0; j < 3; ++j)
        {
            float a = axis[j][i];
            float minv = modelMins[j];
            float maxv = modelMaxs[j];

            if (a >= 0.0f)
            {
                rotatedMins[i] += a * minv;
                rotatedMaxs[i] += a * maxv;
            }
            else
            {
                rotatedMins[i] += a * maxv;
                rotatedMaxs[i] += a * minv;
            }
        }
    }

    mins[0] = rotatedMins[0];
    mins[1] = rotatedMins[1];
    mins[2] = rotatedMins[2];

    maxs[0] = rotatedMaxs[0];
    maxs[1] = rotatedMaxs[1];
    maxs[2] = rotatedMaxs[2];

    // early out unless forced or bounds changed
    if (!forceFilter)
    {
        if (VecLessThan(model->writable.mins, mins) &&
            VecLessThan(maxs, model->writable.maxs))
        {
            return;
        }

        // movement tolerance expansion
        mins[0] -= g_entMoveTolVec[0];
        mins[1] -= g_entMoveTolVec[1];
        mins[2] -= g_entMoveTolVec[2];

        maxs[0] += g_entMoveTolVec[0];
        maxs[1] += g_entMoveTolVec[1];
        maxs[2] += g_entMoveTolVec[2];
    }

    // hard-coded MIP1 expansion
    model->writable.mip1radiusSq = 100.0f * 100.0f;

    mins[0] -= 100.0f;
    mins[1] -= 100.0f;
    mins[2] -= 100.0f;

    maxs[0] += 100.0f;
    maxs[1] += 100.0f;
    maxs[2] += 100.0f;

    // commit
    model->writable.mins[0] = mins[0];
    model->writable.mins[1] = mins[1];
    model->writable.mins[2] = mins[2];

    model->writable.maxs[0] = maxs[0];
    model->writable.maxs[1] = maxs[1];
    model->writable.maxs[2] = maxs[2];

    R_LinkBModelEntity(
        localClientNum,
        cent->nextState.number,
        model
    );
}

bool __cdecl VecLessThan(float *a, float *b)
{
    float v3; // [esp+4h] [ebp-14h]
    float v4; // [esp+8h] [ebp-10h]

    if ( (float)((float)(*a - *b) - (float)(a[1] - b[1])) < 0.0 )
        v4 = a[1] - b[1];
    else
        v4 = *a - *b;
    if ( (float)(v4 - (float)(a[2] - b[2])) < 0.0 )
        v3 = a[2] - b[2];
    else
        v3 = v4;
    return v3 <= 0.0;
}

double __cdecl CG_ScriptMover_GetBurnFraction(int localClientNum, centity_s *cent)
{
    float v3; // [esp+0h] [ebp-14h]
    float v4; // [esp+4h] [ebp-10h]
    int time; // [esp+8h] [ebp-Ch]
    float burnFraction; // [esp+10h] [ebp-4h]
    float burnFractiona; // [esp+10h] [ebp-4h]

    burnFraction = 0.0f;
    if ( cent->pose.startBurnTime )
    {
        time = CG_GetLocalClientGlobals(localClientNum)->time;
        burnFractiona = (float)(time - cent->pose.startBurnTime) / 2000.0;
        if ( (float)(burnFractiona - 1.0) < 0.0 )
            v4 = (float)(time - cent->pose.startBurnTime) / 2000.0;
        else
            v4 = 1.0f;
        if ( (float)(0.0 - burnFractiona) < 0.0 )
            v3 = v4;
        else
            v3 = 0.0f;
        burnFraction = v3;
    }
    if ( cent->destructible )
        return (float)CG_DestructibleGetBurnFrac(cent->destructible);
    return burnFraction;
}

DObj *__cdecl CG_ScriptMover_GetDObj(int localClientNum, centity_s *cent)
{
    const cg_s *LocalClientGlobals; // eax
    const cgs_t *cgs; // [esp+8h] [ebp-10h]
    XModel *model; // [esp+14h] [ebp-4h]

    if ( cent->destructible )
    {
        model = Destructible_GetDDef(cent)->model;
    }
    else
    {
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
        model = cgs->gameModels[CG_WhatModelShouldLocalPlayerSee(
                                                            localClientNum,
                                                            LocalClientGlobals,
                                                            cent,
                                                            cent->nextState.faction.iHeadIconTeam & 3,
                                                            cent->nextState.index.brushmodel,
                                                            cent->nextState.enemyModel)];
    }
    return CG_PreProcess_GetDObj(localClientNum, cent->nextState.number, cent->nextState.eType, model, 0);
}

static void __cdecl Phys_AnglesToMat44(const float *angles, const float *origin, phys_mat44 *outMat)
{
    float axis[3][3];

    AnglesToAxis(angles, axis);
    Phys_Vec3ToNitrousVec(axis[0], &outMat->x);
    Phys_Vec3ToNitrousVec(axis[1], &outMat->y);
    Phys_Vec3ToNitrousVec(axis[2], &outMat->z);
    outMat->w.x = origin[0];
    outMat->w.y = origin[1];
    outMat->w.z = origin[2];
}

// aislop
void CG_AdjustPositionForMover(
    int localClientNum,
    const float *in,
    int moverNum,
    int fromTime,
    int toTime,
    float *out,
    float *outDeltaAngles)
{
    const centity_s *cent;
    cg_s *cg;
    int snapDelta;
    float fracFrom;
    float fracTo;
    float moverPosFrom[3];
    float moverPosTo[3];
    float moverAngFrom[3];
    float moverAngTo[3];
    phys_mat44 matFrom;
    phys_mat44 matTo;
    phys_vec3 input;
    phys_vec3 local;
    phys_vec3 world;

    if ( outDeltaAngles )
    {
        outDeltaAngles[0] = 0.0f;
        outDeltaAngles[1] = 0.0f;
        outDeltaAngles[2] = 0.0f;
    }

    if ( (unsigned int)(moverNum - 1) > 0x3FCu
        || (cent = CG_GetEntity(localClientNum, moverNum),
            !ShouldAdjustPositionForMover(cent) && !vehicle_riding->current.enabled) )
    {
        out[0] = in[0];
        out[1] = in[1];
        out[2] = in[2];
        return;
    }

    cg = CG_GetLocalClientGlobals(localClientNum);
    snapDelta = cg->nextSnap->serverTime - cg->snap->serverTime;

    if ( snapDelta <= 0 )
    {
        fracFrom = 0.0f;
        fracTo = 0.0f;
    }
    else
    {
        fracFrom = (float)(fromTime - cg->snap->serverTime) / (float)snapDelta;
        fracTo = (float)(toTime - cg->snap->serverTime) / (float)snapDelta;
    }

    if ( cent->currentState.pos.trType == TR_INTERPOLATE )
    {
        float p0[3];
        float p1[3];

        BG_EvaluateTrajectory(&cent->currentState.pos, fromTime, p0);
        BG_EvaluateTrajectory(&cent->nextState.lerp.pos, toTime, p1);

        moverPosFrom[0] = (p1[0] - p0[0]) * fracFrom + p0[0];
        moverPosFrom[1] = (p1[1] - p0[1]) * fracFrom + p0[1];
        moverPosFrom[2] = (p1[2] - p0[2]) * fracFrom + p0[2];

        moverPosTo[0] = (p1[0] - p0[0]) * fracTo + p0[0];
        moverPosTo[1] = (p1[1] - p0[1]) * fracTo + p0[1];
        moverPosTo[2] = (p1[2] - p0[2]) * fracTo + p0[2];
    }
    else
    {
        BG_EvaluateTrajectory(&cent->currentState.pos, fromTime, moverPosFrom);
        BG_EvaluateTrajectory(&cent->currentState.pos, toTime, moverPosTo);
    }

    if ( cent->currentState.apos.trType == TR_INTERPOLATE )
    {
        float a0[3];
        float a1[3];

        BG_EvaluateTrajectory(&cent->currentState.apos, fromTime, a0);
        BG_EvaluateTrajectory(&cent->nextState.lerp.apos, toTime, a1);

        LerpAngleVector(a0, a1, fracFrom, moverAngFrom);
        LerpAngleVector(a0, a1, fracTo, moverAngTo);
    }
    else
    {
        BG_EvaluateTrajectory(&cent->currentState.apos, fromTime, moverAngFrom);
        BG_EvaluateTrajectory(&cent->currentState.apos, toTime, moverAngTo);
    }

    Phys_AnglesToMat44(moverAngFrom, moverPosFrom, &matFrom);
    Phys_AnglesToMat44(moverAngTo, moverPosTo, &matTo);

    Phys_Vec3ToNitrousVec(in, &input);
    phys_full_inv_multiply(&local, &matFrom, &input);
    phys_full_multiply(&world, &matTo, &local);
    Phys_NitrousVecToVec3(&world, out);

    if ( outDeltaAngles )
    {
        outDeltaAngles[0] = moverAngTo[0] - moverAngFrom[0];
        outDeltaAngles[1] = moverAngTo[1] - moverAngFrom[1];
        outDeltaAngles[2] = moverAngTo[2] - moverAngFrom[2];
    }
}


void __cdecl LerpAngleVector(float *from, const float *to, float frac, float *result)
{
    float v4; // [esp+4h] [ebp-24h]
    float v5; // [esp+10h] [ebp-18h]
    float v6; // [esp+1Ch] [ebp-Ch]

    v6 = *from;
    *result = AngleNormalize180(*to - *from) * frac + v6;
    v5 = from[1];
    result[1] = AngleNormalize180(to[1] - v5) * frac + v5;
    v4 = from[2];
    result[2] = AngleNormalize180(to[2] - v4) * frac + v4;
}

bool __cdecl ShouldAdjustPositionForMover(const centity_s *cent)
{
    bool result; // al

    switch ( cent->nextState.eType )
    {
        case 6:
        case 0xC:
        case 0xD:
        case 0xE:
        case 0x10:
            result = 1;
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

void __cdecl CG_SetFrameInterpolation(int localClientNum)
{
    int delta; // [esp+0h] [ebp-8h]
    cg_s *cgameGlob; // [esp+4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->snap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    1043,
                    0,
                    "%s",
                    "cgameGlob->snap") )
    {
        __debugbreak();
    }
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    1044,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    delta = cgameGlob->nextSnap->serverTime - cgameGlob->snap->serverTime;
    if ( delta )
    {
        cgameGlob->frameInterpolation = (float)(cgameGlob->time - cgameGlob->snap->serverTime) / (float)delta;
        if ( cgameGlob->frameInterpolation < 0.0 )
            cgameGlob->frameInterpolation = 0.0f;
    }
    else
    {
        cgameGlob->frameInterpolation = 0.0f;
    }
}

float footColor[4][4] =
{
  { 1.0, 0.0, 0.0, 1.0 },
  { 1.0, 1.0, 0.0, 1.0 },
  { 1.0, 1.0, 1.0, 1.0 },
  { 0.0, 1.0, 1.0, 1.0 }
};

void __cdecl CScr_GetFootColor(eFoot foot, float *color)
{
    float *v2; // [esp+0h] [ebp-4h]

    if ( (unsigned int)foot >= FOOTSTEP_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    1071,
                    0,
                    "foot doesn't index FOOTSTEP_COUNT\n\t%i not in [0, %i)",
                    foot,
                    4) )
    {
        __debugbreak();
    }
    v2 = footColor[foot];
    *color = *v2;
    color[1] = v2[1];
    color[2] = v2[2];
    color[3] = v2[3];
}

void __cdecl CG_PlayClientSoundNoteTracks(
                int localClientNum,
                int entnum,
                float *origin,
                const char *notifyName,
                bool isViewArms)
{
    unsigned int AliasId; // eax
    const DObj *obj; // [esp+Ch] [ebp-8h]

    if ( !notifyName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 1137, 0, "%s", "notifyName") )
    {
        __debugbreak();
    }
    obj = Com_GetClientDObj(entnum, localClientNum);
    if ( (!obj || !DObjIsPlayerShadow(obj) || isViewArms) && !Demo_IsMovieCamera() && CG_isSoundAlias(notifyName) )
    {
        AliasId = SND_FindAliasId((char *)notifyName + 6);
        CG_PlaySound(localClientNum, entnum, origin, 0, 0, 1.0, AliasId);
    }
}

bool __cdecl CG_isSoundAlias(const char *notifyName)
{
    return I_strnicmp(notifyName, "sndnt#", 6) == 0;
}

void __cdecl CG_DebugDrawFootFalls(int localClientNum, const centity_s *cent, eFoot foot)
{
    unsigned __int16 FootTag; // ax
    float axis[3][3]; // [esp+38h] [ebp-68h] BYREF
    DObj *obj; // [esp+5Ch] [ebp-44h]
    float starColor[4]; // [esp+60h] [ebp-40h] BYREF
    float footMatrix[4][3]; // [esp+70h] [ebp-30h] BYREF

    obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
    if ( obj
        && (FootTag = CScr_GetFootTag(foot), CG_DObjGetWorldTagMatrix(&cent->pose, obj, FootTag, footMatrix, footMatrix[3])) )
    {
        if ( foot == FOOTSTEP_FRONTLEFT || foot == FOOTSTEP_REARLEFT )
            MatrixNegateXY(footMatrix, footMatrix);
        MatrixSwapXYNegateX(footMatrix, footMatrix);
    }
    else
    {
        //axis[0][0] = 0.0f;
        //*(_QWORD *)&axis[0][1] = __PAIR64__(LODWORD(1.0f), 0);
        //*(_QWORD *)&axis[1][0] = __PAIR64__(LODWORD(-1.0f), 0);
        //axis[1][2] = 0.0f;
        //*(_QWORD *)&axis[2][0] = __PAIR64__(0, LODWORD(1.0f));
        //axis[2][2] = 0.0f;
        axis[0][0] = 0.0f;
        axis[0][1] = 0.0f;
        axis[0][2] = 1.0f;

        axis[1][0] = 0.0f;
        axis[1][1] = -1.0f;
        axis[1][2] = 0.0f;

        axis[2][0] = 1.0f;
        axis[2][1] = 0.0f;
        axis[2][2] = 0.0f;

        memcpy(footMatrix, axis, 0x24u);
        footMatrix[3][0] = cent->pose.origin[0];
        footMatrix[3][1] = cent->pose.origin[1];
        footMatrix[3][2] = cent->pose.origin[2];
    }
    CScr_GetFootColor(foot, starColor);
    G_DebugStar(footMatrix[3], starColor, 30);
}

void __cdecl MatrixNegateXY(const float (*in)[3], float (*out)[3])
{
    ((*out)[0]) = -((*in)[0]);
    ((*out)[1]) = -((*in)[1]);
    ((*out)[2]) = -((*in)[2]);
    ((*out)[3]) = -((*in)[3]);
    ((*out)[4]) = -((*in)[4]);
    ((*out)[5]) = -((*in)[5]);
    (*out)[6] = (*in)[6];
    (*out)[7] = (*in)[7];
    (*out)[8] = (*in)[8];
}

void __cdecl MatrixSwapXYNegateX(const float (*in)[3], float (*out)[3])
{
    float x; // [esp+10h] [ebp-18h]
    float x_4; // [esp+14h] [ebp-14h]
    float x_8; // [esp+18h] [ebp-10h]
    float y_4; // [esp+20h] [ebp-8h]
    float y_8; // [esp+24h] [ebp-4h]

    x = (*in)[0];
    x_4 = (*in)[1];
    x_8 = (*in)[2];
    y_4 = (*in)[4];
    y_8 = (*in)[5];
    ((*out)[0]) = -((*in)[3]);
    ((*out)[1]) = -(y_4);
    ((*out)[2]) = -(y_8);
    (*out)[3] = x;
    (*out)[4] = x_4;
    (*out)[5] = x_8;
    (*out)[6] = (*in)[6];
    (*out)[7] = (*in)[7];
    (*out)[8] = (*in)[8];
}

void __cdecl CG_DoFootsteps(int localClientNum, centity_s *cent)
{
    if ( ((*((unsigned int *)cent + 201) >> 1) & 1) != 0 )
    {
        CG_GetLocalClientGlobals(localClientNum);
        if ( ((*((unsigned int *)cent + 201) >> 11) & 1) != 0 )
        {
            CG_DoFootStep(localClientNum, cent, FOOTSTEP_FRONTLEFT);
            *((unsigned int *)cent + 201) &= ~0x800u;
        }
        if ( ((*((unsigned int *)cent + 201) >> 12) & 1) != 0 )
        {
            CG_DoFootStep(localClientNum, cent, FOOTSTEP_FRONTRIGHT);
            *((unsigned int *)cent + 201) &= ~0x1000u;
        }
        if ( ((*((unsigned int *)cent + 201) >> 13) & 1) != 0 )
        {
            CG_DoFootStep(localClientNum, cent, FOOTSTEP_REARLEFT);
            *((unsigned int *)cent + 201) &= ~0x2000u;
        }
        if ( ((*((unsigned int *)cent + 201) >> 14) & 1) != 0 )
        {
            CG_DoFootStep(localClientNum, cent, FOOTSTEP_REARRIGHT);
            *((unsigned int *)cent + 201) &= ~0x4000u;
        }
    }
}

void __cdecl CG_DoFootStep(int localClientNum, centity_s *cent, eFoot foot)
{
    CScr_PlayDogstepSound(localClientNum, cent, foot);
    if ( cg_footprintsDebug->current.integer )
        CG_DebugDrawFootFalls(localClientNum, cent, foot);
}

void __cdecl CG_ProcessClientNote(
                XAnimClientNotify *note,
                unsigned int entityNum,
                cg_s *cgameGlob,
                centity_s *cent,
                int localClientNum)
{
    const char *NotifyStringName; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    unsigned int NotetrackCLName; // eax
    unsigned int NotifyName; // eax
    const char *v11; // eax
    char *value; // [esp+0h] [ebp-8h]
    unsigned __int16 t; // [esp+4h] [ebp-4h]

    //if ( XAnimClientNotify::IsClientAnimNotify(note) )
    if (note->IsClientAnimNotify())
    {
        //NotetrackCLName = XAnimClientNotify::GetNotetrackCLName(note);
        NotetrackCLName = note->GetNotetrackCLName();
        Scr_AddConstString(NotetrackCLName, SCRIPTINSTANCE_CLIENT);
        //NotifyName = XAnimClientNotify::GetNotifyName(note);
        NotifyName = note->GetNotifyName();
        CScr_NotifyNum(localClientNum, entityNum, 0, NotifyName, 1u);
    }
    else
    {
        if ( (note->GetNotifyType() & 1) != 0 && entityNum < 0x20)
        {
            NotifyStringName = note->GetNotifyStringName();
            if ( I_stricmp(NotifyStringName, "anim_gunhand = \"left\"") )
            {
                v6 = note->GetNotifyStringName();
                if ( !I_stricmp(v6, "anim_gunhand = \"right\"") )
                {
                    cgameGlob->bgs.clientinfo[entityNum].leftHandGun = 0;
                    cgameGlob->bgs.clientinfo[entityNum].dobjDirty = 1;
                }
            }
            else
            {
                cgameGlob->bgs.clientinfo[entityNum].leftHandGun = 1;
                cgameGlob->bgs.clientinfo[entityNum].dobjDirty = 1;
            }
        }
        v7 = note->GetNotifyStringName();
        if ( I_strnicmp(v7, "dogstep_", 8) )
        {
            v8 = note->GetNotifyStringName();
            if ( !I_strnicmp(v8, "sound_", 6) )
            {
                if ( entityNum >= 0x400
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                                1319,
                                0,
                                "entityNum doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
                                entityNum,
                                1024) )
                {
                    __debugbreak();
                }
                value = (char *)note->GetNotifyStringName();
                Scr_AddString(value, SCRIPTINSTANCE_CLIENT);
                CScr_AddEntity(cent, (unsigned __int16)localClientNum);
                Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
                t = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_data.dogSoundNotify, 3u);
                Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
            }
        }
        else
        {
            if ( entityNum >= 0x400
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                            1285,
                            0,
                            "entityNum doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
                            entityNum,
                            1024) )
            {
                __debugbreak();
            }
            if ( cent->nextState.eType == 17 )
            {
                if ( note->GetNotifyStringName()[8] == 76
                    || note->GetNotifyStringName()[8] == 108 )
                {
                    if ( note->GetNotifyStringName()[9] == 70
                        || note->GetNotifyStringName()[9] == 102 )
                    {
                        *((unsigned int *)cent + 201) |= 0x800u;
                    }
                    else
                    {
                        *((unsigned int *)cent + 201) |= 0x2000u;
                    }
                }
                else if ( note->GetNotifyStringName()[8] == 82
                             || note->GetNotifyStringName()[8] == 114 )
                {
                    if ( note->GetNotifyStringName()[9] == 70
                        || note->GetNotifyStringName()[9] == 102 )
                    {
                        *((unsigned int *)cent + 201) |= 0x1000u;
                    }
                    else
                    {
                        *((unsigned int *)cent + 201) |= 0x4000u;
                    }
                }
                else if ( note->GetNotifyStringName()[8] == 70
                             || note->GetNotifyStringName()[8] == 102 )
                {
                    *((unsigned int *)cent + 201) |= 0x800u;
                    *((unsigned int *)cent + 201) |= 0x1000u;
                }
                else
                {
                    *((unsigned int *)cent + 201) |= 0x2000u;
                    *((unsigned int *)cent + 201) |= 0x4000u;
                }
            }
        }
    }
    v11 = note->GetNotifyStringName();
    CG_PlayClientSoundNoteTracks(localClientNum, entityNum, cent->pose.origin, v11, 0);
}

void __cdecl CScr_AddEntity(centity_s *cent, int clientNum)
{
    if ( !cent
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_scr_main.h", 418, 0, "%s", "cent") )
    {
        __debugbreak();
    }
    if ( cent->nextState.number > 0x600u
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_scr_main.h",
                    419,
                    0,
                    "cent->nextState.number not in [0, MAX_LOCAL_CENTITIES]\n\t%i not in [%i, %i]",
                    cent->nextState.number,
                    0,
                    1536) )
    {
        __debugbreak();
    }
    Scr_AddEntityNum(cent->nextState.number, 0, SCRIPTINSTANCE_CLIENT, clientNum);
}

void __cdecl CG_ProcessFakeEntClientNoteTracks(int localClientNum, int entityNum)
{
    int i; // [esp+8h] [ebp-18h]
    XAnimClientNotifyList *pNotifies; // [esp+Ch] [ebp-14h]
    cg_s *cgameGlob; // [esp+10h] [ebp-10h]
    centity_s *cent; // [esp+14h] [ebp-Ch]
    XAnimClientNotifyList *pList; // [esp+18h] [ebp-8h]
    int iListSize; // [esp+1Ch] [ebp-4h]

    pList = (XAnimClientNotifyList *)DObjGetClientNotifies();
    if ( pList )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        cent = CG_GetEntity(localClientNum, entityNum);
        pNotifies = pList->GetNotifyList();
        iListSize = pList->m_numNotifies;
        for ( i = 0; i < iListSize; ++i )
            CG_ProcessClientNote((XAnimClientNotify *)pNotifies + i, entityNum, cgameGlob, cent, localClientNum);
    }
}

void __cdecl CG_AddPacketEntity(int localClientNum, unsigned int entnum)
{
    double v2; // st7
    bool v3; // [esp+4h] [ebp-D8h]
    bool v5; // [esp+Ch] [ebp-D0h]
    bool v6; // [esp+10h] [ebp-CCh]
    bool v8; // [esp+18h] [ebp-C4h]
    float *v10; // [esp+48h] [ebp-94h]
    float *v11; // [esp+4Ch] [ebp-90h]
    float v12; // [esp+64h] [ebp-78h]
    float frameInterpolation; // [esp+6Ch] [ebp-70h]
    snapshot_s *nextSnap; // [esp+84h] [ebp-58h]
    float radius; // [esp+90h] [ebp-4Ch]
    const DObj *obj; // [esp+94h] [ebp-48h]
    float newAngles[3]; // [esp+98h] [ebp-44h] BYREF
    cg_s *LocalClientGlobals; // [esp+A4h] [ebp-38h]
    clientInfo_t *ci; // [esp+A8h] [ebp-34h]
    int vehSlot; // [esp+ACh] [ebp-30h]
    float origin[3]; // [esp+B4h] [ebp-28h]
    cg_s *cgameGlob; // [esp+C0h] [ebp-1Ch]
    centity_s *cent; // [esp+C4h] [ebp-18h]
    float angles[3]; // [esp+C8h] [ebp-14h]
    bool entMoved; // [esp+D7h] [ebp-5h]
    unsigned int eType; // [esp+D8h] [ebp-4h]
    int savedregs; // [esp+DCh] [ebp+0h] BYREF

    PROF_SCOPED("CG_AddPacketEntity");

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cent = CG_GetEntity(localClientNum, entnum);
    eType = cent->nextState.eType;
    if ( Demo_IsPlaying() )
    {
        nextSnap = cgameGlob->nextSnap;
        if ( (nextSnap->ps.otherFlags & 6) != 0 && entnum == nextSnap->ps.clientNum )
        {
            return;
        }
    }
    origin[0] = cent->pose.origin[0];
    origin[1] = cent->pose.origin[1];
    origin[2] = cent->pose.origin[2];
    angles[0] = cent->pose.angles[0];
    angles[1] = cent->pose.angles[1];
    angles[2] = cent->pose.angles[2];
    entMoved = 0;
    if ((eType == 6 || eType == 13) && cent->nextState.solid == 0xFFFFFF)
    {
        CG_CalcEntityLerpPositions(localClientNum, cent);
        if ( CG_EntityLinked(localClientNum, cent) )
            CG_LinkTransformForEntity(localClientNum, cent, cent->pose.origin, cent->pose.angles);
        v5 = cent->pose.origin[0] == origin[0] && cent->pose.origin[1] == origin[1] && cent->pose.origin[2] == origin[2];
        v3 = 1;
        if ( v5 )
        {
            if ( cent->pose.angles[0] == angles[0] && cent->pose.angles[1] == angles[1] && cent->pose.angles[2] == angles[2] )
                v3 = 0;
        }
        entMoved = v3;
        if ( v3 )
            CG_UpdateBModelWorldBounds(localClientNum, cent, 0);
    }
    else
    {
        if ( eType == 1
            && CG_VehEntityUsingVehicle(localClientNum, entnum)
            && !CG_VehEntityUsingRemoteControlVehicle(localClientNum, entnum) )
        {
            CG_VehSeatTransformForPlayer(localClientNum, entnum, cent->pose.origin, newAngles);
            vehSlot = CG_VehPlayerVehicleSeat(localClientNum, entnum);
            LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
            ci = &LocalClientGlobals->bgs.clientinfo[cent->nextState.clientNum];
            v12 = cent->currentState.u.turret.gunAngles[0];
            frameInterpolation = LocalClientGlobals->frameInterpolation;
            v2 = AngleNormalize180(cent->nextState.lerp.u.turret.gunAngles[0] - v12);
            ci->lerpLean = v2 * frameInterpolation + v12;
        }
        else
        {
            CG_CalcEntityLerpPositions(localClientNum, cent);
            if ( CG_EntityLinked(localClientNum, cent) )
                CG_LinkTransformForEntity(localClientNum, cent, cent->pose.origin, cent->pose.angles);
        }
        v8 = cent->pose.origin[0] == origin[0] && cent->pose.origin[1] == origin[1] && cent->pose.origin[2] == origin[2];
        v6 = 1;
        if ( v8 )
        {
            if ( cent->pose.angles[0] == angles[0] && cent->pose.angles[1] == angles[1] && cent->pose.angles[2] == angles[2] )
                v6 = 0;
        }
        entMoved = v6;
        if ( Vec3DistanceSq(cent->pose.origin, cg_entityOriginArray[localClientNum][entnum]) > 256.0 )
        {
            v10 = cg_entityOriginArray[localClientNum][entnum];
            v11 = cent->pose.origin;
            *v10 = cent->pose.origin[0];
            v10[1] = v11[1];
            v10[2] = v11[2];
            obj = Com_GetClientDObj(entnum, localClientNum);
            if ( obj )
            {
                radius = DObjGetRadius(obj) + 16.0;
                R_LinkDObjEntity(localClientNum, entnum, cent->pose.origin, radius);
            }
        }
    }
    if ( cent->pose.physObjId == -1 )
    {
        if ( localClientNum
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_world.h",
                        17,
                        0,
                        "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        localClientNum,
                        1) )
        {
            __debugbreak();
        }
        if ( entnum >= 0x600
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_world.h",
                        18,
                        0,
                        "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                        entnum,
                        1536) )
        {
            __debugbreak();
        }
        if ( localClientNum
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                        108,
                        0,
                        "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        localClientNum,
                        1) )
        {
            __debugbreak();
        }
        if ( entnum >= 0x600
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                        109,
                        0,
                        "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                        entnum,
                        1536) )
        {
            __debugbreak();
        }
        if ( cgEntCollNodes[localClientNum][entnum].sector )
            CG_UnlinkEntity(localClientNum, entnum);
    }
    else
    {
        if ( localClientNum
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_world.h",
                        17,
                        0,
                        "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        localClientNum,
                        1) )
        {
            __debugbreak();
        }
        if ( entnum >= 0x600
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_world.h",
                        18,
                        0,
                        "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                        entnum,
                        1536) )
        {
            __debugbreak();
        }
        if ( localClientNum
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                        108,
                        0,
                        "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        localClientNum,
                        1) )
        {
            __debugbreak();
        }
        if ( entnum >= 0x600
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                        109,
                        0,
                        "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                        entnum,
                        1536) )
        {
            __debugbreak();
        }
        if ( cgEntCollNodes[localClientNum][entnum].sector )
        {
            if ( entMoved )
                CG_LinkEntity(localClientNum, entnum);
        }
        else if ( CG_EntityNeedsLinked(localClientNum, entnum) )
        {
            CG_LinkEntity(localClientNum, entnum);
        }
        CG_UpdateClientDobjPartBits(cent, entnum, localClientNum);
        CG_ProcessEntity(localClientNum, cent);
    }
}

void __cdecl CG_UpdateClientDobjPartBits(centity_s *cent, unsigned int entnum, int localClientNum)
{
    int i; // [esp+8h] [ebp-40h]
    unsigned int partBits[5]; // [esp+Ch] [ebp-3Ch] BYREF
    cgs_t *cgs; // [esp+20h] [ebp-28h]
    cg_s *cgameGlob; // [esp+24h] [ebp-24h]
    entityState_s *p_nextState; // [esp+28h] [ebp-20h]
    clientInfo_t *ci; // [esp+2Ch] [ebp-1Ch]
    DObj *obj; // [esp+30h] [ebp-18h]
    unsigned int oldPartBits[5]; // [esp+34h] [ebp-14h] BYREF

    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 1452, 0, "%s", "cent") )
        __debugbreak();
    obj = Com_GetClientDObj(entnum, localClientNum);
    if ( obj )
    {
        if ( cent->nextState.eType == 1 || cent->nextState.eType == 2 )
        {
            ci = 0;
            cgameGlob = CG_GetLocalClientGlobals(localClientNum);
            p_nextState = &cent->nextState;
            if ( cent->nextState.eType == 1 )
            {
                ci = &cgameGlob->bgs.clientinfo[p_nextState->clientNum];
            }
            else
            {
                cgs = CG_GetLocalClientStaticGlobals(localClientNum);
                ci = (clientInfo_t *)((char *)&cgs[-3] + 1480 * p_nextState->number - 3208);
            }
            DObjGetHidePartBits(obj, oldPartBits);
            if ( ci->hideWeapon && p_nextState->clientNum == entnum )
            {
                for ( i = 0; i < 5; ++i )
                    partBits[i] = ci->weaponHideBits[i] | cent->nextState.partBits[i];
                DObjSetHidePartBits(obj, partBits);
                FX_MarkEntUpdateHidePartBits(oldPartBits, partBits, localClientNum, entnum);
            }
            else
            {
                if ( memcmp(oldPartBits, cent->nextState.partBits, 0x14u) )
                {
                    ci->dobjDirty = 1;
                    BG_UpdatePlayerDObj(localClientNum, obj, p_nextState, ci, 0);
                }
                DObjSetHidePartBits(obj, cent->nextState.partBits);
                FX_MarkEntUpdateHidePartBits(oldPartBits, cent->nextState.partBits, localClientNum, entnum);
            }
        }
        else if ( !cent->destructible )
        {
            DObjGetHidePartBits(obj, oldPartBits);
            DObjSetHidePartBits(obj, cent->nextState.partBits);
            FX_MarkEntUpdateHidePartBits(oldPartBits, cent->nextState.partBits, localClientNum, entnum);
        }
    }
}

int __cdecl CG_AddPacketEntities(int localClientNum)
{
    DelayListInfo delayList; // [esp+58h] [ebp-34h] BYREF
    cg_s *cgameGlob; // [esp+64h] [ebp-28h]
    bool contextKey; // [esp+6Bh] [ebp-21h]
    centity_s *cent; // [esp+6Ch] [ebp-20h]
    int numEntities; // [esp+70h] [ebp-1Ch]
    cgs_t *cgs; // [esp+74h] [ebp-18h]
    int num; // [esp+78h] [ebp-14h]
    int entnum; // [esp+7Ch] [ebp-10h]
    int nextStateOffset; // [esp+80h] [ebp-Ch]
    int postPSEntNum; // [esp+84h] [ebp-8h] BYREF
    unsigned int eType; // [esp+88h] [ebp-4h]

    nextStateOffset = 488;

    PROF_SCOPED("add packet ents");

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    contextKey = cgs[1].processedSnapshotNum;
    cgs[1].processedSnapshotNum = LOBYTE(cgs[1].processedSnapshotNum) == 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgameGlob->rumbleScale = 0.0f;
    numEntities = cgameGlob->nextSnap->numEntities;
    numEntities = cgameGlob->nextSnap->numEntities;
    if (numEntities)
        _mm_prefetch((const char *)&CG_GetEntity(localClientNum, cgameGlob->nextSnap->entities[0].number)->nextState, 1);
    postPSEntNum = 1023;

    {
        PROF_SCOPED("server ents");

        DelayListInit(&delayList);
        for (num = 0; num < numEntities; ++num)
        {
            entnum = cgameGlob->nextSnap->entities[num].number;
            if (num + 1 < numEntities)
                _mm_prefetch(
                    (const char *)&CG_GetEntity(localClientNum, cgameGlob->nextSnap->entities[num + 1].number)->nextState,
                    1);
            cent = CG_GetEntity(localClientNum, entnum);
            if (EntPacketUpdateNeedsDelayed(localClientNum, cent, contextKey))
                DelayListAdd(&delayList, cent);
            else
                UpdatePacketEnt(localClientNum, entnum, cgameGlob->time, &postPSEntNum, contextKey);
        }
        UpdateDelayedPacketEnts(localClientNum, &delayList, &postPSEntNum, contextKey);
    }

    {
        PROF_SCOPED("fake ents");
        for (num = 0; num < 512; ++num)
            CG_ProcessFakeEntity(localClientNum, &cg_fakeEntitiesArray[512 * localClientNum + num]);
    }
    
    if ( g_processEvents )
    {
        PROF_SCOPED("process events");

        for ( num = 0; num < cgameGlob->nextSnap->numEntities; ++num )
        {
            entnum = cgameGlob->nextSnap->entities[num].number;
            cent = CG_GetEntity(localClientNum, entnum);
            eType = cent->nextState.eType;
            if ( eType != 8 && eType != 9 )
            {
                if ( CG_ShouldDelayEntityPacketPostPS(localClientNum, cent) )
                {
                    if ( postPSEntNum == 1023
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                                    2217,
                                    0,
                                    "%s",
                                    "postPSEntNum != ENTITYNUM_NONE") )
                    {
                        __debugbreak();
                    }
                }
                else if ( eType < 0x15 )
                {
                    CG_CheckEntEvents(localClientNum, cent);
                }
                else
                {
                    CG_CheckEvents(localClientNum, cent);
                }
            }
        }
    }

    return postPSEntNum;
}

void __cdecl CG_ProcessFakeEntity(int localClientNum, fake_centity_s *fakeEnt)
{
    bool v2; // [esp+1Ch] [ebp-90h]
    bool v4; // [esp+24h] [ebp-88h]
    unsigned int number; // [esp+34h] [ebp-78h]
    float *v6; // [esp+40h] [ebp-6Ch]
    float *v7; // [esp+44h] [ebp-68h]
    unsigned __int16 t; // [esp+60h] [ebp-4Ch]
    float up[3]; // [esp+64h] [ebp-48h] BYREF
    float color[4]; // [esp+70h] [ebp-3Ch] BYREF
    bool entMoved; // [esp+83h] [ebp-29h]
    float radius; // [esp+84h] [ebp-28h]
    DObj *obj; // [esp+88h] [ebp-24h]
    float origin[3]; // [esp+8Ch] [ebp-20h]
    float angles[3]; // [esp+98h] [ebp-14h]
    cg_s *cgameGlob; // [esp+A4h] [ebp-8h]
    centity_s *cent; // [esp+A8h] [ebp-4h]
    int savedregs; // [esp+ACh] [ebp+0h] BYREF

    cent = &fakeEnt->cent;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cg_fakeEntitiesInuseArray[512 * localClientNum + fakeEnt - &cg_fakeEntitiesArray[512 * localClientNum]] )
    {
        CG_UpdateFakeEntityLink(localClientNum, cent);
        CG_FakeEntityEffects(localClientNum, cent);
        if ( cent->nextState.loopSoundId )
        {
            if ( cent->nextState.loopSoundFade < 0 )
            {
                cent->nextState.loopSoundFade += LOWORD(cgameGlob->frametime);
                if ( cent->nextState.loopSoundFade >= 0 )
                {
                    cent->nextState.loopSoundId = 0;
                    cent->nextState.loopSoundFade = 0;
                }
            }
        }
        switch ( cent->nextState.eType )
        {
            case 0:
                if ( ((*((unsigned int *)cent + 201) >> 15) & 1) != 0 )
                {
                    if ( !cent->pose.player.tag[0] )
                    {
                        CG_LinkEntity(localClientNum, cent->nextState.number);
                        cent->pose.player.tag[0] = 1;
                    }
                    if ( cg_debug_triggers->current.enabled )
                    {
                        color[0] = 0.0f;
                        color[1] = 1.0f;
                        color[2] = 0.0f;
                        color[3] = 1.0f;
                        if ( (*((unsigned int *)cent + 201) & 0x10000) != 0 )
                        {
                            up[0] = 0.0f;
                            up[1] = 0.0f;
                            up[2] = 1.0f;
                            G_DebugCircleEx(cent->pose.origin, cent->pose.turret.barrelPitch, up, color, 0, 1);
                        }
                        else
                        {
                            CG_DebugBox(
                                cent->pose.origin,
                                &cent->pose.turret.barrelPitch,
                                &cent->pose.turret.firingOffset,
                                0.0,
                                color,
                                1,
                                1);
                        }
                    }
                }
                CG_General(localClientNum, cent);
                break;
            case 6:
                goto $LN21_3;
            case 8:
                //_mm_prefetch((const char *)&cent->pose.108, 1);
                CG_Fx(localClientNum, cent);
                break;
            case 9:
                CG_LoopFx(localClientNum, cent);
                break;
            case 0xA:
                CG_PrimaryLight(localClientNum, cent);
$LN21_3:
                origin[0] = cent->pose.origin[0];
                origin[1] = cent->pose.origin[1];
                origin[2] = cent->pose.origin[2];
                angles[0] = cent->pose.angles[0];
                angles[1] = cent->pose.angles[1];
                angles[2] = cent->pose.angles[2];
                CG_CalcEntityLerpPositions(localClientNum, cent);
                if ( CG_EntityLinked(localClientNum, cent) )
                    CG_LinkTransformForEntity(localClientNum, cent, cent->pose.origin, cent->pose.angles);
                if ( Vec3DistanceSq(cent->pose.origin, cg_entityOriginArray[localClientNum][cent->nextState.number]) > 256.0 )
                {
                    v6 = cg_entityOriginArray[localClientNum][cent->nextState.number];
                    v7 = cent->pose.origin;
                    *v6 = cent->pose.origin[0];
                    v6[1] = v7[1];
                    v6[2] = v7[2];
                    if (cent->nextState.solid == 0xFFFFFF)
                    {
                        CG_UpdateBModelWorldBounds(localClientNum, cent, 0);
                    }
                    else
                    {
                        obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
                        if ( obj )
                        {
                            radius = DObjGetRadius(obj) + 16.0;
                            R_LinkDObjEntity(localClientNum, cent->nextState.number, cent->pose.origin, radius);
                        }
                    }
                }
                number = cent->nextState.number;
                if ( localClientNum
                    && !Assert_MyHandler(
                                "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_world.h",
                                17,
                                0,
                                "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                                localClientNum,
                                1) )
                {
                    __debugbreak();
                }
                if ( number >= 0x600
                    && !Assert_MyHandler(
                                "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_world.h",
                                18,
                                0,
                                "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                                number,
                                1536) )
                {
                    __debugbreak();
                }
                if ( localClientNum
                    && !Assert_MyHandler(
                                "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                                108,
                                0,
                                "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                                localClientNum,
                                1) )
                {
                    __debugbreak();
                }
                if ( number >= 0x600
                    && !Assert_MyHandler(
                                "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                                109,
                                0,
                                "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                                number,
                                1536) )
                {
                    __debugbreak();
                }
                if ( cgEntCollNodes[localClientNum][number].sector )
                {
                    v4 = cent->pose.origin[0] == origin[0]
                        && cent->pose.origin[1] == origin[1]
                        && cent->pose.origin[2] == origin[2];
                    v2 = 1;
                    if ( v4 )
                    {
                        if ( cent->pose.angles[0] == angles[0]
                            && cent->pose.angles[1] == angles[1]
                            && cent->pose.angles[2] == angles[2] )
                        {
                            v2 = 0;
                        }
                    }
                    entMoved = v2;
                    if ( v2 )
                        CG_LinkEntity(localClientNum, cent->nextState.number);
                }
                else if ( CG_EntityNeedsLinked(localClientNum, cent->nextState.number) )
                {
                    CG_LinkEntity(localClientNum, cent->nextState.number);
                }
                if ( ((*((unsigned int *)cent + 201) >> 18) & 1) != 0 )
                    CG_CompassUpdateVehicleInfo(localClientNum, cent->nextState.number);
                CG_ScriptMover(localClientNum, cent);
                break;
            default:
                break;
        }
        if ( CG_EntityNeedsScriptThread(localClientNum, cent) )
        {
            *((unsigned int *)cent + 201) |= 0x100u;
            Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
            t = CScr_ExecEntThread(cent, cg_scr_data.entityspawned, 1u);
            Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
        }
    }
}

void __cdecl CG_General(int localClientNum, centity_s *cent)
{
    DObj *obj; // [esp+28h] [ebp-18h]
    const cgs_t *cgs; // [esp+30h] [ebp-10h]
    float lightingOrigin[3]; // [esp+34h] [ebp-Ch] BYREF

    if ( (cent->nextState.lerp.eFlags & 0x20) == 0 )
    {
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        obj = CG_PreProcess_GetDObj(
                        localClientNum,
                        cent->nextState.number,
                        cent->nextState.eType,
                        cgs->gameModels[cent->nextState.index.brushmodel],
                        0);
        if ( obj )
        {
            CG_GetLightingOrigin(cent, lightingOrigin);
            R_AddDObjToScene(
                obj,
                &cent->pose,
                cent->nextState.number,
                0,
                lightingOrigin,
                0.0,
                0.0,
                0.0,
                0.0,
                -1,
                -1,
                0,
                0,
                0.0,
                1.0);
        }
    }
}

void __cdecl CG_GetLightingOrigin(centity_s *cent, float *lightingOrigin)
{
    *lightingOrigin = (float)(cent->pose.absmin[0] + cent->pose.absmax[0]) * 0.5;
    lightingOrigin[1] = (float)(cent->pose.absmin[1] + cent->pose.absmax[1]) * 0.5;
    lightingOrigin[2] = (float)(cent->pose.absmin[2] + cent->pose.absmax[2]) * 0.5;
}

void __cdecl CG_FakeEntityEffects(int localClientNum, centity_s *cent)
{
    if ( cent->nextState.loopSoundId )
        CG_PumpEntityLoopSound(localClientNum, cent);
}

void __cdecl CG_ScriptMover(int localClientNum, centity_s *cent)
{
    const GfxBrushModel *BrushModel; // eax
    unsigned int lightingOriginToleranceSq; // [esp+20h] [ebp-24h]
    float burnFraction; // [esp+28h] [ebp-1Ch]
    unsigned int renderFxFlags; // [esp+2Ch] [ebp-18h]
    DObj *obj; // [esp+30h] [ebp-14h]
    entityState_s *s1; // [esp+34h] [ebp-10h]
    float lightingOrigin[3]; // [esp+38h] [ebp-Ch] BYREF

    s1 = &cent->nextState;
    if ( (cent->nextState.lerp.eFlags & 0x20) != 0 )
    {
        if (cent->nextState.solid != 0xFFFFFF && (cent->nextState.clientLinkInfo.flags & 1) != 0)
        {
            CG_ScriptMover_GetDObj(localClientNum, cent);
        }
    }
    else if (cent->nextState.solid == 0xFFFFFF)
    {
        lightingOriginToleranceSq = s1->number;
        BrushModel = R_GetBrushModel(cent->nextState.index.brushmodel);
        R_AddBrushModelToSceneFromAngles(BrushModel, cent->pose.origin, cent->pose.angles, lightingOriginToleranceSq, 0);
    }
    else
    {
        obj = CG_ScriptMover_GetDObj(localClientNum, cent);
        if ( obj && (cent->nextState.lerp.eFlags & 0x200) == 0 )
        {
            CG_GetLightingOrigin(cent, lightingOrigin);
            renderFxFlags = 0;
            if ( cent->destructible )
            {
                if ( cent->pose.destructiblePose || Destructible_XDollActive(cent->destructible) )
                    renderFxFlags = 4;
            }
            else if ( cent->pose.isRagdoll )
            {
                renderFxFlags = 4;
            }
            if ( (cent->nextState.lerp.eFlags2 & 0x4000000) != 0 )
                renderFxFlags |= 0x200000u;
            burnFraction = CG_ScriptMover_GetBurnFraction(localClientNum, cent);
            R_AddDObjToScene(
                obj,
                &cent->pose,
                s1->number,
                renderFxFlags,
                lightingOrigin,
                0.0,
                0.0,
                burnFraction,
                cent->pose.wetness,
                -1,
                -1,
                0,
                0,
                0.0,
                1.0);
        }
    }
}

void __cdecl CG_LoopFx(int localClientNum, centity_s *cent)
{
    const cg_s *cgameGlob; // [esp+Ch] [ebp-Ch]
    int period; // [esp+10h] [ebp-8h]
    float cullDist; // [esp+14h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cullDist = cent->nextState.lerp.u.turret.gunAngles[0];
    if ( cullDist == 0.0
        || Vec3DistanceSq(cgameGlob->predictedPlayerState.origin, cent->pose.origin) < cullDist * cullDist )
    {
        period = cent->nextState.lerp.u.loopFx.period;
        if ( period <= 0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 1630, 0, "%s", "period > 0") )
        {
            __debugbreak();
        }
        if ( !UniqueHandleToEffect(localClientNum, cent->pose.fx.effect) )
        {
            cent->pose.fx.effect = CG_StartFx(localClientNum, cent, cgameGlob->time);
            if ( !UniqueHandleToEffect(localClientNum, cent->pose.fx.effect) )
                return;
            cent->pose.fx.triggerTime = period + cgameGlob->time;
        }
        while ( cgameGlob->time >= cent->pose.fx.triggerTime && UniqueHandleToEffect(localClientNum, cent->pose.fx.effect) )
        {
            FX_RetriggerEffect(localClientNum, cent->pose.fx.effect, cent->pose.fx.triggerTime);
            cent->pose.fx.triggerTime += period;
        }
    }
}

void __cdecl CG_PrimaryLight(int localClientNum, centity_s *cent)
{
    unsigned int brushmodel; // esi
    unsigned int PrimaryLightCount; // eax
    const char *v4; // eax
    float v5; // [esp+2Ch] [ebp-48h]
    float v6; // [esp+30h] [ebp-44h]
    cg_s *cgameGlob; // [esp+34h] [ebp-40h]
    GfxLight *light; // [esp+40h] [ebp-34h]
    float oldColor[4]; // [esp+44h] [ebp-30h] BYREF
    const ComPrimaryLight *refLight; // [esp+54h] [ebp-20h]
    float lightAngles[3]; // [esp+58h] [ebp-1Ch] BYREF
    float newColor[4]; // [esp+64h] [ebp-10h] BYREF

    if ( cent->nextState.eType != 10
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    1693,
                    0,
                    "%s",
                    "cent->nextState.eType == ET_PRIMARY_LIGHT") )
    {
        __debugbreak();
    }
    if ( !cent->nextState.index.brushmodel
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    1694,
                    0,
                    "%s",
                    "cent->nextState.index.primaryLight != PRIMARY_LIGHT_NONE") )
    {
        __debugbreak();
    }
    brushmodel = cent->nextState.index.brushmodel;
    if ( brushmodel >= Com_GetPrimaryLightCount() )
    {
        PrimaryLightCount = Com_GetPrimaryLightCount();
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                        1695,
                        0,
                        "cent->nextState.index.primaryLight doesn't index Com_GetPrimaryLightCount()\n\t%i not in [0, %i)",
                        cent->nextState.index.brushmodel,
                        PrimaryLightCount) )
            __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    light = &cgameGlob->refdef.primaryLights[cent->nextState.index.brushmodel];
    refLight = Com_GetPrimaryLight(cent->nextState.index.brushmodel);
    Byte4UnpackRgba((const unsigned __int8 *)&cent->currentState.u, oldColor);
    Byte4UnpackRgba((const unsigned __int8 *)&cent->nextState.lerp.u, newColor);
    v6 = cent->currentState.u.turret.gunAngles[1];
    oldColor[0] = v6 * oldColor[0];
    oldColor[1] = v6 * oldColor[1];
    oldColor[2] = v6 * oldColor[2];
    v5 = cent->nextState.lerp.u.turret.gunAngles[1];
    newColor[0] = v5 * newColor[0];
    newColor[1] = v5 * newColor[1];
    newColor[2] = v5 * newColor[2];
    Vec3Lerp(oldColor, newColor, cgameGlob->frameInterpolation, light->color);
    light->diffuseColor[0] = light->color[0];
    light->diffuseColor[1] = light->color[1];
    light->diffuseColor[2] = light->color[2];
    light->specularColor[0] = light->color[0];
    light->specularColor[1] = light->color[1];
    light->specularColor[2] = light->color[2];
    light->specularColor[3] = 1.0f;
    light->diffuseColor[3] = 1.0f;
    if ( refLight->rotationLimit < 1.0 )
    {
        BG_EvaluateTrajectory(&cent->nextState.lerp.apos, cgameGlob->time, lightAngles);
        AngleVectors(lightAngles, light->dir, 0, 0);
        (light->dir[0]) = -(light->dir[0]);
        (light->dir[1]) = -(light->dir[1]);
        (light->dir[2]) = -(light->dir[2]);
        SpotLightViewMatrix(light->dir, light->angles[2], light->viewMatrix.m);
        if ( refLight->rotationLimit > -1.0 )
            CG_ClampPrimaryLightDir(light, refLight);
    }
    if ( refLight->translationLimit > 0.0 )
    {
        BG_EvaluateTrajectory(&cent->nextState.lerp.pos, cgameGlob->time, light->origin);
        CG_ClampPrimaryLightOrigin(light, refLight);
    }
    light->radius = (float)((float)(cent->nextState.lerp.u.turret.gunAngles[2] - cent->currentState.u.turret.gunAngles[2])
                                                * cgameGlob->frameInterpolation)
                                + cent->currentState.u.turret.gunAngles[2];
    light->cosHalfFovOuter = (float)((float)(cent->nextState.lerp.u.primaryLight.cosHalfFovOuter
                                                                                 - cent->currentState.u.primaryLight.cosHalfFovOuter)
                                                                 * cgameGlob->frameInterpolation)
                                                 + cent->currentState.u.primaryLight.cosHalfFovOuter;
    light->cosHalfFovInner = (float)((float)(cent->nextState.lerp.u.turret.heatVal - cent->currentState.u.turret.heatVal)
                                                                 * cgameGlob->frameInterpolation)
                                                 + cent->currentState.u.turret.heatVal;
    light->exponent = (int)(float)((float)(cent->nextState.lerp.u.primaryLight.colorAndExp[3]
                                                                             - cent->currentState.u.primaryLight.colorAndExp[3])
                                                             * cgameGlob->frameInterpolation)
                                    + cent->currentState.u.primaryLight.colorAndExp[3];
    if ( light->cosHalfFovOuter <= 0.0 || light->cosHalfFovInner <= light->cosHalfFovOuter || light->cosHalfFovInner > 1.0 )
    {
        v4 = va("%g, %g", light->cosHalfFovOuter, light->cosHalfFovInner);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                        1737,
                        0,
                        "%s\n\t%s",
                        "0.0f < light->cosHalfFovOuter && light->cosHalfFovOuter < light->cosHalfFovInner && light->cosHalfFovInner <= 1.0f",
                        v4) )
            __debugbreak();
    }
}

unsigned int __cdecl Com_GetPrimaryLightCount()
{
    if ( !comWorld.isInUse
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../qcommon/com_bsp_api.h",
                    34,
                    0,
                    "%s",
                    "comWorld.isInUse") )
    {
        __debugbreak();
    }
    return comWorld.primaryLightCount;
}

const ComPrimaryLight *__cdecl Com_GetPrimaryLight(unsigned int primaryLightIndex)
{
    if ( !comWorld.isInUse
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../qcommon/com_bsp_api.h",
                    47,
                    0,
                    "%s",
                    "comWorld.isInUse") )
    {
        __debugbreak();
    }
    if ( primaryLightIndex >= comWorld.primaryLightCount
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../qcommon/com_bsp_api.h",
                    48,
                    0,
                    "primaryLightIndex doesn't index comWorld.primaryLightCount\n\t%i not in [0, %i)",
                    primaryLightIndex,
                    comWorld.primaryLightCount) )
    {
        __debugbreak();
    }
    return &comWorld.primaryLights[primaryLightIndex];
}

void __cdecl CG_ClampPrimaryLightOrigin(GfxLight *light, const ComPrimaryLight *refLight)
{
    float v2; // [esp+4h] [ebp-24h]
    float deltaLenSq; // [esp+14h] [ebp-14h]
    float lightDelta; // [esp+1Ch] [ebp-Ch]
    float lightDelta_4; // [esp+20h] [ebp-8h]
    float lightDelta_8; // [esp+24h] [ebp-4h]

    lightDelta = light->origin[0] - refLight->origin[0];
    lightDelta_4 = light->origin[1] - refLight->origin[1];
    lightDelta_8 = light->origin[2] - refLight->origin[2];
    deltaLenSq = (float)((float)(lightDelta * lightDelta) + (float)(lightDelta_4 * lightDelta_4))
                         + (float)(lightDelta_8 * lightDelta_8);
    if ( (float)(refLight->translationLimit * refLight->translationLimit) <= deltaLenSq )
    {
        v2 = refLight->translationLimit / sqrtf(deltaLenSq);
        light->origin[0] = (float)(v2 * lightDelta) + refLight->origin[0];
        light->origin[1] = (float)(v2 * lightDelta_4) + refLight->origin[1];
        light->origin[2] = (float)(v2 * lightDelta_8) + refLight->origin[2];
    }
}

void __cdecl CG_ClampPrimaryLightDir(GfxLight *light, const ComPrimaryLight *refLight)
{
    double v2; // st7
    const char *v3; // eax
    float rotationLimit; // [esp+38h] [ebp-38h]
    float cosTurnAngle; // [esp+5Ch] [ebp-14h]
    float perpendicular_4; // [esp+64h] [ebp-Ch]
    float perpendicular_8; // [esp+68h] [ebp-8h]
    float perpScale; // [esp+6Ch] [ebp-4h]

    cosTurnAngle = (float)((float)(light->dir[0] * refLight->dir[0]) + (float)(light->dir[1] * refLight->dir[1]))
                             + (float)(light->dir[2] * refLight->dir[2]);
    if ( cosTurnAngle < refLight->rotationLimit )
    {
        perpendicular_4 = (float)(COERCE_FLOAT(-(cosTurnAngle)) * refLight->dir[1]) + light->dir[1];
        perpendicular_8 = (float)(COERCE_FLOAT(-(cosTurnAngle)) * refLight->dir[2]) + light->dir[2];
        perpScale = sqrtf(
                                    (float)(1.0 - (float)(refLight->rotationLimit * refLight->rotationLimit))
                                / (float)(1.0 - (float)(cosTurnAngle * cosTurnAngle)));
        rotationLimit = refLight->rotationLimit;
        light->dir[0] = (float)(rotationLimit * refLight->dir[0])
                                    + (float)(perpScale
                                                    * (float)((float)(COERCE_FLOAT(-(cosTurnAngle)) * refLight->dir[0]) + light->dir[0]));
        light->dir[1] = (float)(rotationLimit * refLight->dir[1]) + (float)(perpScale * perpendicular_4);
        light->dir[2] = (float)(rotationLimit * refLight->dir[2]) + (float)(perpScale * perpendicular_8);
        if ( !Vec3IsNormalized(light->dir) )
        {
            v2 = Vec3Length(light->dir);
            v3 = va("(%g %g %g) len %g", light->dir[0], light->dir[1], light->dir[2], v2);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                            1677,
                            0,
                            "%s\n\t%s",
                            "Vec3IsNormalized( light->dir )",
                            v3) )
                __debugbreak();
        }
        iassert(I_fabs(Vec3Dot(light->dir, refLight->dir) - refLight->rotationLimit) <= 0.001f);
        //if ( (fabs((float)((float)((float)(light->dir[0] * refLight->dir[0]) + (float)(light->dir[1] * refLight->dir[1])) + (float)(light->dir[2] * refLight->dir[2])) - refLight->rotationLimit)) > 0.001
        //    && !Assert_MyHandler(
        //                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
        //                1678,
        //                0,
        //                "%s",
        //                "I_fabs( Vec3Dot( light->dir, refLight->dir ) - refLight->rotationLimit ) <= 0.001f") )
        //{
        //    __debugbreak();
        //}
    }
}

bool __cdecl CG_ShouldDelayEntityPacketPostPS(int localClientNum, centity_s *cent)
{
    int viewlocked_entNum; // [esp+0h] [ebp-Ch]
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    1883,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 1884, 0, "%s", "cent") )
        __debugbreak();
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 1887, 1, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    if ( cent->nextState.eType != 11 )
        return 0;
    if ( (cgameGlob->predictedPlayerState.eFlags & 0x300) != 0 )
        viewlocked_entNum = cgameGlob->predictedPlayerState.viewlocked_entNum;
    else
        viewlocked_entNum = 1023;
    if ( cent->nextState.number == viewlocked_entNum )
        return 1;
    return CG_IsCarriedTurretEnt(localClientNum, cent);
}

bool __cdecl CG_IsCarriedTurretEnt(int localClientNum, centity_s *cent)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 445, 0, "%s", "cent") )
        __debugbreak();
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    return cent->nextState.eType == 11
            && cent->nextState.otherEntityNum == cgameGlob->predictedPlayerState.clientNum
            && (cent->nextState.lerp.u.turret.flags & 8) != 0;
}

void __cdecl UpdatePacketEnt(int localClientNum, int entnum, int timeNow, int *postPSEntNum, bool contextKey)
{
    unsigned int v5; // eax
    centity_s *cent; // [esp+Ch] [ebp-8h]
    unsigned int eType; // [esp+10h] [ebp-4h]

    cent = CG_GetEntity(localClientNum, entnum);
    eType = cent->nextState.eType;
    if ( eType < 0x15 )
    {
        *((unsigned int *)cent + 201) |= (unsigned int)0x800000;
        if ( contextKey )
            v5 = *((_DWORD *)cent + 201) | 0x1000000;
        else
            v5 = *((unsigned int *)cent + 201) & 0xFEFFFFFF;
        *((unsigned int *)cent + 201) = v5;
        if ( eType < 8 || eType > 9 )
        {
            if ( CG_ShouldDelayEntityPacketPostPS(localClientNum, cent) )
            {
                if ( *postPSEntNum == 1023 )
                    *postPSEntNum = entnum;
                else
                    Com_PrintError(
                        14,
                        "More than one entity is trying to be delayed to player state use logic; we already had %i set, but %i is req"
                        "uesting it too and will be ignored. Probably having multiple turrets in use or carried.\n",
                        *postPSEntNum,
                        entnum);
            }
            else
            {
                CG_AddPacketEntity(localClientNum, entnum);
            }
        }
        else
        {
            CG_AddPacketFxEntity(localClientNum, entnum);
        }
    }
}

void __cdecl DelayListInit(DelayListInfo *listInfo)
{
    if ( !listInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 1974, 0, "%s", "listInfo") )
    {
        __debugbreak();
    }
    listInfo->head = 0;
    listInfo->latest = 0;
    listInfo->size = 0;
}

void __cdecl DelayListAdd(DelayListInfo *listInfo, centity_s *cent)
{
    centity_s *centAssert; // [esp+0h] [ebp-4h]

    if ( !listInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 1984, 0, "%s", "listInfo") )
    {
        __debugbreak();
    }
    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 1985, 0, "%s", "cent") )
        __debugbreak();
    for ( centAssert = listInfo->head; centAssert; centAssert = centAssert->updateDelayedNext )
    {
        if ( centAssert == cent
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                        1989,
                        0,
                        "%s",
                        "centAssert != cent") )
        {
            __debugbreak();
        }
    }
    cent->updateDelayedNext = 0;
    if ( listInfo->head )
    {
        if ( !listInfo->latest
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                        2003,
                        0,
                        "%s",
                        "listInfo->latest") )
        {
            __debugbreak();
        }
        listInfo->latest->updateDelayedNext = cent;
        listInfo->latest = cent;
        ++listInfo->size;
    }
    else
    {
        if ( listInfo->latest
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                        1996,
                        0,
                        "%s",
                        "listInfo->latest == NULL") )
        {
            __debugbreak();
        }
        listInfo->head = cent;
        listInfo->latest = cent;
        listInfo->size = 1;
    }
}

bool __cdecl EntPacketUpdateNeedsDelayed(int localClientNum, centity_s *cent, bool contextKey)
{
    bool v4; // [esp+3h] [ebp-9h]
    centity_s *centParent; // [esp+4h] [ebp-8h]
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    centParent = CG_EntGetLinkToParent(localClientNum, cent);
    if ( !centParent )
        return 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    v4 = ((*((unsigned int *)centParent + 201) >> 23) & 1) != 0
        && contextKey == ((*((unsigned int *)centParent + 201) & 0x1000000) != 0);
    return !v4 && centParent->nextState.number != cgameGlob->predictedPlayerState.clientNum;
}

void __cdecl UpdateDelayedPacketEnts(int localClientNum, DelayListInfo *delayList, int *postPSEntNum, bool contextKey)
{
    centity_s *centNext; // [esp+0h] [ebp-1Ch]
    int entnum; // [esp+4h] [ebp-18h]
    centity_s *cent; // [esp+8h] [ebp-14h]
    int sizeAtStart; // [esp+Ch] [ebp-10h]
    centity_s *centListPrev; // [esp+10h] [ebp-Ch]
    int delays; // [esp+14h] [ebp-8h]
    cg_s *cgameGlob; // [esp+18h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    while ( delayList->size )
    {
        sizeAtStart = delayList->size;
        centListPrev = 0;
        delays = 0;
        cent = delayList->head;
        while ( cent )
        {
            if ( EntPacketUpdateNeedsDelayed(localClientNum, cent, contextKey) )
            {
                if ( ++delays >= 100 )
                {
                    Com_PrintWarning(14, "UpdateDelayedPacketEnts() appears stuck on ent #%i.\n", cent->nextState.number);
                    return;
                }
                centListPrev = cent;
                cent = cent->updateDelayedNext;
            }
            else
            {
                entnum = cent->nextState.number;
                centNext = cent->updateDelayedNext;
                DelayListRemove(delayList, cent, centListPrev);
                UpdatePacketEnt(localClientNum, entnum, cgameGlob->time, postPSEntNum, contextKey);
                cent = centNext;
            }
        }
        if ( delayList->size >= sizeAtStart
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                        2094,
                        0,
                        "delayList->size < sizeAtStart\n\t%i, %i",
                        delayList->size,
                        sizeAtStart) )
        {
            __debugbreak();
        }
        if ( delayList->size >= sizeAtStart )
        {
            Com_PrintWarning(14, "UpdateDelayedPacketEnts(): delayList stuck with %i entries left.\n", delayList->size);
            return;
        }
    }
}

void __cdecl DelayListRemove(DelayListInfo *listInfo, centity_s *cent, centity_s *centPrev)
{
    if ( !listInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 2012, 0, "%s", "listInfo") )
    {
        __debugbreak();
    }
    if ( listInfo->size <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    2013,
                    0,
                    "%s",
                    "listInfo->size > 0") )
    {
        __debugbreak();
    }
    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 2014, 0, "%s", "cent") )
        __debugbreak();
    if ( cent == listInfo->latest )
        listInfo->latest = centPrev;
    if ( cent == listInfo->head )
        listInfo->head = listInfo->head->updateDelayedNext;
    if ( centPrev )
        centPrev->updateDelayedNext = cent->updateDelayedNext;
    if ( --listInfo->size < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    2025,
                    0,
                    "%s",
                    "listInfo->size >= 0") )
    {
        __debugbreak();
    }
}

void __cdecl CG_DObjUpdateInfo(const cg_s *cgameGlob, DObj *obj, int notifyFlags)
{
    DObjUpdateClientInfo(obj, (float)cgameGlob->frametime * 0.001, notifyFlags);
}

int __cdecl CG_DObjGetWorldBoneMatrix(const cpose_t *pose, DObj *obj, int boneIndex, float (*tagMat)[3], float *origin)
{
    DObjAnimMat *mat; // [esp+4Ch] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 2297, 0, "%s", "obj") )
        __debugbreak();
    if ( !pose && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 2298, 0, "%s", "pose") )
        __debugbreak();
    mat = CG_DObjGetLocalBoneMatrix(pose, obj, boneIndex);
    if ( !mat )
        return 0;
    ConvertQuatToMat(mat, tagMat);
    *origin = mat->trans[0];
    origin[1] = mat->trans[1];
    origin[2] = mat->trans[2];
    return 1;
}

DObjAnimMat *__cdecl CG_DObjGetLocalBoneMatrix(const cpose_t *pose, DObj *obj, int boneIndex)
{
    DObjAnimMat *mat; // [esp+4h] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 2256, 0, "%s", "obj") )
        __debugbreak();
    CG_DObjCalcBone(pose, obj, boneIndex);
    mat = DObjGetRotTransArray(obj);
    if ( mat )
        return &mat[boneIndex];
    else
        return 0;
}

int __cdecl CG_DObjGetWorldTagMatrix(
                const cpose_t *pose,
                DObj *obj,
                unsigned int tagName,
                float (*tagMat)[3],
                float *origin)
{
    DObjAnimMat *mat; // [esp+4Ch] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 2320, 0, "%s", "obj") )
        __debugbreak();
    if ( !pose && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 2321, 0, "%s", "pose") )
        __debugbreak();
    mat = CG_DObjGetLocalTagMatrix(pose, obj, tagName);
    if ( !mat )
        return 0;
    ConvertQuatToMat(mat, tagMat);
    *origin = mat->trans[0];
    origin[1] = mat->trans[1];
    origin[2] = mat->trans[2];
    return 1;
}

DObjAnimMat *__cdecl CG_DObjGetLocalTagMatrix(const cpose_t *pose, DObj *obj, unsigned int tagName)
{
    unsigned __int8 boneIndex; // [esp+3h] [ebp-1h] BYREF

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 2277, 0, "%s", "obj") )
        __debugbreak();
    boneIndex = -2;
    if ( DObjGetBoneIndex(obj, tagName, &boneIndex, -1) )
        return CG_DObjGetLocalBoneMatrix(pose, obj, boneIndex);
    else
        return 0;
}

int __cdecl CG_DObjGetWorldTagPos(const cpose_t *pose, DObj *obj, unsigned int tagName, float *pos)
{
    DObjAnimMat *matEA; // [esp+8h] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 2344, 0, "%s", "obj") )
        __debugbreak();
    if ( !pose && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 2345, 0, "%s", "pose") )
        __debugbreak();
    matEA = CG_DObjGetLocalTagMatrix(pose, obj, tagName);
    if ( !matEA )
        return 0;
    *pos = matEA->trans[0];
    pos[1] = matEA->trans[1];
    pos[2] = matEA->trans[2];
    return 1;
}

const cpose_t *__cdecl CG_GetPose(int localClientNum, unsigned int handle)
{
    if ( handle > 0x600
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    2393,
                    0,
                    "%s\n\t(handle) = %i",
                    "(handle >= 0 && handle < ((((1<<10) + 512)) + 1))",
                    handle) )
    {
        __debugbreak();
    }
    if ( (int)handle < 1536 )
        return &CG_GetEntity(localClientNum, handle)->pose;
    if ( (int)(handle - 1536) >= 2048
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    2401,
                    0,
                    "%s\n\t(handle) = %i",
                    "(handle >= (((1<<10) + 512)) && handle - (((1<<10) + 512)) < ( 1 << 11 ))",
                    handle) )
    {
        __debugbreak();
    }
    return &CG_GetLocalClientGlobals(localClientNum)->viewModelPose;
}

unsigned int __cdecl CG_StartFx(int localClientNum, centity_s *cent, int startAtTime)
{
    const FxEffectDef *fxDef; // [esp+0h] [ebp-30h]
    int fxId; // [esp+8h] [ebp-28h]
    float axis[3][3]; // [esp+Ch] [ebp-24h] BYREF

    AnglesToAxis(cent->pose.angles, axis);
    fxId = cent->nextState.un1.scale;
    if ( (!cent->nextState.un1.scale || cent->nextState.un1.scale > 0xC3u)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    2501,
                    0,
                    "fxId not in [1, MAX_EFFECT_NAMES - 1]\n\t%i not in [%i, %i]",
                    fxId,
                    1,
                    195) )
    {
        __debugbreak();
    }
    fxDef = CG_GetLocalClientStaticGlobals(localClientNum)->fxs[fxId];
    if ( !fxDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 2504, 0, "%s", "fxDef") )
    {
        __debugbreak();
    }
    return FX_SpawnOrientedEffect(localClientNum, fxDef, startAtTime, cent->pose.origin, axis, 0x3FFu);
}

void __cdecl CG_Fx(int localClientNum, centity_s *cent)
{
    const char *v2; // eax
    cg_s *LocalClientGlobals; // eax
    const char *v4; // eax
    int time; // [esp+24h] [ebp-4h]

    if ( cent->pose.fx.triggerTime != cent->nextState.time2 )
    {
        if ( UniqueHandleToEffect(localClientNum, cent->pose.fx.effect) )
        {
            time = CG_GetLocalClientGlobals(localClientNum)->time;
            v2 = va(
                         "%i A(%g %g %g) %i %i ",
                         time,
                         cent->pose.origin[0],
                         cent->pose.origin[1],
                         cent->pose.origin[2],
                         cent->pose.fx.triggerTime,
                         cent->nextState.time2);
            FX_AssertAllocatedEffect(localClientNum, cent->pose.fx.effect, v2);
            FX_ThroughWithEffect(localClientNum, cent->pose.fx.effect, 1);
        }
        cent->pose.fx.triggerTime = 0;
        cent->pose.fx.effect = CG_StartFx(localClientNum, cent, cent->nextState.time2);
        if ( UniqueHandleToEffect(localClientNum, cent->pose.fx.effect) )
        {
            LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
            v4 = va(
                         "%i B(%g %g %g) %i %i ",
                         LocalClientGlobals->time,
                         cent->pose.origin[0],
                         cent->pose.origin[1],
                         cent->pose.origin[2],
                         cent->pose.fx.triggerTime,
                         cent->nextState.time2);
            FX_AssertAllocatedEffect(localClientNum, cent->pose.fx.effect, v4);
            cent->pose.fx.triggerTime = cent->nextState.time2;
        }
    }
}

bool __cdecl IsLocalClientDriving(const cg_s *cgameGlob, int vehicleEntNum)
{
    bool local_client_driving; // [esp+3h] [ebp-1h]

    if ( Demo_IsPlaying() )
        return 0;
    if ( cgameGlob->predictedPlayerState.pm_type == 4 || (cgameGlob->predictedPlayerState.otherFlags & 2) != 0 )
        return 0;
    local_client_driving = 0;
    if ( (cgameGlob->predictedPlayerState.eFlags & 0x4000) != 0 )
        return cgameGlob->predictedPlayerState.viewlocked_entNum == vehicleEntNum;
    return local_client_driving;
}

char __cdecl AnyLocalClientDriving(centity_s *cent)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]
    int localClientNum; // [esp+4h] [ebp-4h]

    if ( Demo_IsPlaying() )
        return 0;
    for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
    {
        if ( CL_LocalClient_IsActive(localClientNum) )
        {
            cgameGlob = CG_GetLocalClientGlobals(localClientNum);
            if ( cgameGlob->nextSnap )
            {
                if ( IsLocalClientDriving(cgameGlob, cent->nextState.number) )
                    return 1;
            }
        }
    }
    return 0;
}

bool __cdecl ShouldInterpolateFromNitrousVehicleLocally(const cg_s *cgameGlob, centity_s *cent)
{
    if ( cgameGlob->inKillCam )
        return 0;
    if ( Demo_IsPlaying() )
        return 0;
    if ( AnyLocalClientDriving(cent) )
        return 1;
    return cent->nitrousVeh->m_owner != 0;
}

bool __cdecl ShouldUpdateNitrousVehicleFromNetwork(const cg_s *cgameGlob)
{
    return cgameGlob->inKillCam == 0;
}

void __cdecl CG_CreateRagdollObject(int localClientNum, centity_s *cent)
{
    const RagdollBody *RagdollForDObj; // eax
    cg_s *cgameGlob; // [esp+4h] [ebp-8h]
    bool shareRagdoll; // [esp+Ah] [ebp-2h]
    bool reset; // [esp+Bh] [ebp-1h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    reset = !cgameGlob->inKillCam && !Demo_IsPlaying() && (cent->nextState.lerp.eFlags & 0x10) != 0;
    if ( cent->nextState.eType == 2 && cent->nextState.clientNum == cgameGlob->clientNum && cgameGlob->inKillCam )
    {
        shareRagdoll = 0;
        RagdollForDObj = Ragdoll_CreateRagdollForDObj(
                                             localClientNum,
                                             0,
                                             cent->nextState.number,
                                             reset,
                                             0);
    }
    else
    {
        shareRagdoll = 1;
        RagdollForDObj = Ragdoll_CreateRagdollForDObj(
                                             localClientNum,
                                             0,
                                             cent->nextState.number,
                                             reset,
                                             1);
    }
    if ( shareRagdoll )
        cent->pose.ragdollHandle = (int)RagdollForDObj;
    else
        cent->pose.killcamRagdollHandle = (int)RagdollForDObj;
    cent->pose.isRagdoll = 1;
}

void __cdecl CG_CalcEntityLerpPositions(int localClientNum, centity_s *cent)
{
    __int16 infoIdx; // [esp+20h] [ebp-18h]
    const vehicle_info_t *vehicleInfo; // [esp+24h] [ebp-14h]
    cg_s *cgameGlob; // [esp+28h] [ebp-10h]
    cgs_t *cgs; // [esp+2Ch] [ebp-Ch]
    unsigned int corpseIndex; // [esp+30h] [ebp-8h]
    clientInfo_t *ci; // [esp+34h] [ebp-4h]
    clientInfo_t *cia; // [esp+34h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cent->nextState.eType != 14 || !sv_clientSideVehicles->current.enabled || cgameGlob->inKillCam )
        goto LABEL_27;
    infoIdx = cent->nextState.vehicleState.vehicleInfoIndex;
    vehicleInfo = CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex);
    if ( !vehicleInfo->name[0] )
        vehicleInfo = BG_GetVehicleInfo(infoIdx);
    if ( vehicleInfo->name[0] <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    3027,
                    0,
                    "%s",
                    "vehicleInfo->name[0] > 0") )
    {
        __debugbreak();
    }
    if ( vehicleInfo->isNitrous && !cent->nitrousVeh && cent->nextState.un3.item )
    {
        if ( Demo_IsPlaying() )
            CG_InterpolateEntityPosition(cgameGlob, cent, localClientNum);
        Sys_EnterCriticalSection(CRITSECT_PHYSICS);
        //cent->nitrousVeh = NitrousVehicle::add_vehicle(cent->nextState.number);
        cent->nitrousVeh = NitrousVehicle::add_vehicle(cent->nextState.number);

        if (cent->nitrousVeh)
        {
            //NitrousVehicle::init(cent->nitrousVeh, localClientNum, cent, &vehicleInfo->nitrousVehParams);
            cent->nitrousVeh->init(localClientNum, cent, &vehicleInfo->nitrousVehParams);
        }
        Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
    }
    if ( cent->nitrousVeh )
    {
        CG_InterpolateVehicle(cgameGlob, cent, cgameGlob->time);
    }
    else
    {
LABEL_27:
        if ( cent->currentState.pos.trType == 10 )
            goto LABEL_28;
        if ( cent->pose.physObjId && cent->pose.physObjId != -1 )
        {
            Phys_ObjDestroy(0, cent->pose.physObjId);
            cent->pose.physObjId = 0;
            cent->currentState.pos.trBase[0] = cent->pose.origin[0];
            cent->currentState.pos.trBase[1] = cent->pose.origin[1];
            cent->currentState.pos.trBase[2] = cent->pose.origin[2];
            cent->currentState.apos.trBase[0] = cent->pose.angles[0];
            cent->currentState.apos.trBase[1] = cent->pose.angles[1];
            cent->currentState.apos.trBase[2] = cent->pose.angles[2];
        }
        if ( cent->currentState.pos.trType == 1 && cent->nextState.lerp.pos.trType != 10
            || cent->currentState.pos.trType == 4 && cent->nextState.number < 32 )
        {
LABEL_28:
            CG_InterpolateEntityPosition(cgameGlob, cent, localClientNum);
        }
        else
        {
            BG_EvaluateTrajectory(&cent->currentState.pos, cgameGlob->time, cent->pose.origin);
            BG_EvaluateTrajectory(&cent->currentState.apos, cgameGlob->time, cent->pose.angles);
            if ( cent->nextState.eType == 1 )
            {
                if ( cent->nextState.clientNum >= 0x20u
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                                3090,
                                0,
                                "cent->nextState.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                                cent->nextState.clientNum,
                                32) )
                {
                    __debugbreak();
                }
                ci = &cgameGlob->bgs.clientinfo[cent->nextState.clientNum];
                ci->lerpMoveDir = (float)cent->nextState.lerp.u.loopFx.period;
                ci->playerAngles[0] = cent->pose.angles[0];
                ci->playerAngles[1] = cent->pose.angles[1];
                ci->playerAngles[2] = cent->pose.angles[2];
                cent->pose.angles[0] = 0.0f;
                cent->pose.angles[2] = 0.0f;
                ci->lerpLean = cent->nextState.lerp.u.turret.gunAngles[0];
            }
            else if ( cent->nextState.eType == 2 )
            {
                corpseIndex = cent->nextState.number - 32;
                if ( corpseIndex >= 4
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                                3102,
                                0,
                                "%s",
                                "(unsigned)corpseIndex < MAX_CLIENT_CORPSES") )
                {
                    __debugbreak();
                }
                cgs = CG_GetLocalClientStaticGlobals(localClientNum);
                cia = &cgs->corpseinfo[corpseIndex];
                cgs->corpseinfo[corpseIndex].lerpMoveDir = (float)cent->nextState.lerp.u.loopFx.period;
                cia->playerAngles[0] = cent->pose.angles[0];
                cia->playerAngles[1] = cent->pose.angles[1];
                cia->playerAngles[2] = cent->pose.angles[2];
                cent->pose.angles[0] = 0.0f;
                cent->pose.angles[2] = 0.0f;
                cia->lerpLean = cent->nextState.lerp.u.turret.gunAngles[0];
            }
            if ( cent != &cgameGlob->predictedPlayerEntity && (!cent->linkInfo || !cent->linkInfo->linkEnt) )
                CG_AdjustPositionForMover(
                    localClientNum,
                    cent->pose.origin,
                    cent->nextState.groundEntityNum,
                    cgameGlob->snap->serverTime,
                    cgameGlob->time,
                    cent->pose.origin,
                    0);
            CG_InterpolateEntityPosition(cgameGlob, cent, localClientNum);
            if ( CG_IsRagdollTrajectory(&cent->currentState.pos) )
                CG_CalcEntityRagdollPositions(localClientNum, cent);
        }
    }
}

void __cdecl CG_InterpolateEntityPosition(cg_s *cgameGlob, centity_s *cent, int localClientNum)
{
    double v3; // st7
    double v4; // st7
    double v5; // st7
    double v6; // st7
    double v7; // st7
    double v8; // st7
    float v9; // [esp+1Ch] [ebp-7Ch]
    float v10; // [esp+24h] [ebp-74h]
    float *playerAngles; // [esp+30h] [ebp-68h]
    float period; // [esp+38h] [ebp-60h]
    float v13; // [esp+44h] [ebp-54h]
    float v14; // [esp+50h] [ebp-48h]
    float v15; // [esp+5Ch] [ebp-3Ch]
    fake_centity_s *fakeEnt; // [esp+70h] [ebp-28h]
    int time; // [esp+74h] [ebp-24h]
    float next[3]; // [esp+78h] [ebp-20h] BYREF
    float f; // [esp+84h] [ebp-14h]
    clientInfo_t *ci; // [esp+88h] [ebp-10h]
    float current[3]; // [esp+8Ch] [ebp-Ch] BYREF

    if ( !cgameGlob->snap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    2549,
                    0,
                    "%s",
                    "cgameGlob->snap") )
    {
        __debugbreak();
    }
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    2550,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    if ( cent->nextState.number < 1024 )
    {
        f = cgameGlob->frameInterpolation;
        BG_EvaluateTrajectory(&cent->currentState.pos, cgameGlob->snap->serverTime, current);
        BG_EvaluateTrajectory(&cent->nextState.lerp.pos, cgameGlob->nextSnap->serverTime, next);
        Vec3Lerp(current, next, f, cent->pose.origin);
        BG_EvaluateTrajectory(&cent->currentState.apos, cgameGlob->snap->serverTime, current);
        BG_EvaluateTrajectory(&cent->nextState.lerp.apos, cgameGlob->nextSnap->serverTime, next);
        v15 = current[0];
        v3 = AngleNormalize180(next[0] - current[0]);
        cent->pose.angles[0] = v3 * f + v15;
        v14 = current[1];
        v4 = AngleNormalize180(next[1] - current[1]);
        cent->pose.angles[1] = v4 * f + v14;
        v13 = current[2];
        v5 = AngleNormalize180(next[2] - current[2]);
        cent->pose.angles[2] = v5 * f + v13;
        if ( cent->nextState.eType == 1 )
        {
            if ( cent->nextState.clientNum >= 0x20u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                            2616,
                            0,
                            "cent->nextState.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                            cent->nextState.clientNum,
                            32) )
            {
                __debugbreak();
            }
            ci = &cgameGlob->bgs.clientinfo[cent->nextState.clientNum];
            period = (float)cent->currentState.u.loopFx.period;
            v6 = AngleNormalize180((float)cent->nextState.lerp.u.loopFx.period - period);
            ci->lerpMoveDir = v6 * f + period;
            playerAngles = ci->playerAngles;
            ci->playerAngles[0] = cent->pose.angles[0];
            playerAngles[1] = cent->pose.angles[1];
            playerAngles[2] = cent->pose.angles[2];
            cent->pose.angles[0] = 0.0f;
            cent->pose.angles[2] = 0.0f;
            v10 = cent->currentState.u.turret.gunAngles[0];
            v7 = AngleNormalize180(cent->nextState.lerp.u.turret.gunAngles[0] - v10);
            ci->lerpLean = v7 * f + v10;
        }
        if ( cent->nextState.eType == 17 )
        {
            v9 = (float)cent->currentState.u.actor.proneInfo.fBodyPitch * 0.0054931641;
            v8 = AngleNormalize180((float)((float)cent->nextState.lerp.u.actor.proneInfo.fBodyPitch * 0.0054931641) - v9);
            cent->pose.turret.angles.yaw = v8 * f + v9;
        }
    }
    else
    {
        time = cgameGlob->time;
        BG_EvaluateTrajectory(&cent->nextState.lerp.pos, time, cent->pose.origin);
        fakeEnt = CG_GetFakeEntity(localClientNum, cent->nextState.number);
        if ( &fakeEnt->cent != cent
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                        2567,
                        0,
                        "%s",
                        "&fakeEnt->cent == cent") )
        {
            __debugbreak();
        }
        if ( (fakeEnt->flags & 1) != 0 && time > cent->nextState.lerp.pos.trDuration + cent->nextState.lerp.pos.trTime )
        {
            fakeEnt->flags &= ~1u;
            CScr_NotifyNum(localClientNum, cent->nextState.number, 0, cscr_const.movedone, 0);
        }
        BG_EvaluateTrajectory(&cent->nextState.lerp.apos, time, cent->pose.angles);
        if ( (fakeEnt->flags & 2) != 0 && time > cent->nextState.lerp.apos.trDuration + cent->nextState.lerp.apos.trTime )
        {
            if ( !cent->cScriptMover
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                            2584,
                            0,
                            "%s",
                            "cent->cScriptMover != NULL") )
            {
                __debugbreak();
            }
            if ( CScriptMover_UpdateMove(
                         &cent->nextState.lerp.apos,
                         cent->pose.origin,
                         cent->cScriptMover->speed,
                         cent->cScriptMover->midTime,
                         cent->cScriptMover->decelTime,
                         cent->cScriptMover->pos1,
                         cent->cScriptMover->pos2,
                         cent->cScriptMover->pos3) )
            {
                fakeEnt->flags &= ~2u;
                CScr_NotifyNum(localClientNum, cent->nextState.number, 0, cscr_const.rotatedone, 0);
            }
        }
    }
}

fake_centity_s *__cdecl CG_GetFakeEntity(int localClientNum, int entityIndex)
{
    if ( localClientNum
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                    1852,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    if ( (entityIndex < 1024 || entityIndex > 1536)
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                    1853,
                    0,
                    "entityIndex not in [MAX_GENTITIES, MAX_LOCAL_CENTITIES]\n\t%i not in [%i, %i]",
                    entityIndex,
                    1024,
                    1536) )
    {
        __debugbreak();
    }
    return &cg_fakeEntitiesArray[512 * localClientNum - 1024 + entityIndex];
}

void __cdecl CG_InterpolateVehicle(cg_s *cgameGlob, centity_s *cent, int curTime)
{
    trajectory_t v3; // [esp-44h] [ebp-ACh] BYREF
    trajectory_t v4; // [esp-20h] [ebp-88h] BYREF
    centity_s *v5; // [esp+8h] [ebp-60h]
    float timeDifference; // [esp+18h] [ebp-50h]
    trajectory_t trPos; // [esp+1Ch] [ebp-4Ch] BYREF
    float f; // [esp+40h] [ebp-28h]
    trajectory_t trAPos; // [esp+44h] [ebp-24h] BYREF
    int savedregs; // [esp+68h] [ebp+0h] BYREF

    if ( IsLocalClientDriving(cgameGlob, cent->nextState.number) )
    {
        CG_InterpolateVehicleDriver(cgameGlob, cent, curTime);
    }
    else
    {
        if ( !cgameGlob->nextSnap
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                        2760,
                        0,
                        "%s",
                        "cgameGlob->nextSnap") )
        {
            __debugbreak();
        }
        if ( !cent->nitrousVeh
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                        2761,
                        0,
                        "%s",
                        "cent->nitrousVeh") )
        {
            __debugbreak();
        }
        if ( cent->nitrousVeh->m_phys_user_data )
        {
            if ( !cgameGlob->snap
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                            2767,
                            0,
                            "%s",
                            "cgameGlob->snap") )
            {
                __debugbreak();
            }
            f = cgameGlob->frameInterpolation;
            if ( ShouldInterpolateFromNitrousVehicleLocally(cgameGlob, cent) )
            {
                //NitrousVehicle::GetEntityPose(cent->nitrousVeh, cent->pose.origin, cent->pose.angles, curTime, 0);
                cent->nitrousVeh->GetEntityPose(cent->pose.origin, cent->pose.angles, curTime, false);
            }
            else
            {
                if ( cent->nextState.lerp.pos.trType == 10
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                                2777,
                                0,
                                "%s",
                                "cent->nextState.lerp.pos.trType != TR_PHYSICS") )
                {
                    __debugbreak();
                }
                CG_InterpolateEntityPosition(cgameGlob, cent, 0);
                v5 = cent;
                if ( ShouldUpdateNitrousVehicleFromNetwork(cgameGlob) )
                {
                    CG_LerpTrajectory(cgameGlob, &cent->currentState.pos, &cent->nextState.lerp.pos, f, &trPos, 0);
                    CG_LerpTrajectory(cgameGlob, &cent->currentState.apos, &cent->nextState.lerp.apos, f, &trAPos, 1);
                    timeDifference = (float)(Phys_GetCurrentTime() - cgameGlob->nextSnap->physicsTime) / 1000.0;
                    memcpy(&v4, &trAPos, sizeof(v4));
                    memcpy(&v3, &trPos, sizeof(v3));
                    cent->nitrousVeh->update_from_network(v3, v4, &cent->nextState.lerp.u.vehicle, timeDifference);
                    //NitrousVehicle::update_from_network(
                    //    cent->nitrousVeh,
                    //    v3,
                    //    v4,
                    //    (LerpEntityStateVehicle *)&cent->nextState.lerp.u,
                    //    timeDifference);
                    //NitrousVehicle::GetEntityPose(cent->nitrousVeh, cent->pose.origin, cent->pose.angles, curTime, 0);
                    cent->nitrousVeh->GetEntityPose(cent->pose.origin, cent->pose.angles, curTime, false);
                }
            }
        }
    }
}

void __cdecl CG_LerpTrajectory(
                const cg_s *cgameGlob,
                trajectory_t *trFrom,
                trajectory_t *trTo,
                float lerp,
                trajectory_t *trResult,
                bool isAngle)
{
    float v6; // [esp+8h] [ebp-3Ch]
    float v7; // [esp+Ch] [ebp-38h]
    float v8; // [esp+20h] [ebp-24h]
    float v9; // [esp+2Ch] [ebp-18h]
    float v10; // [esp+38h] [ebp-Ch]

    if ( isAngle )
    {
        v10 = trFrom->trBase[0];
        trResult->trBase[0] = AngleNormalize180(trTo->trBase[0] - v10) * lerp + v10;
        v9 = trFrom->trBase[1];
        trResult->trBase[1] = AngleNormalize180(trTo->trBase[1] - v9) * lerp + v9;
        v8 = trFrom->trBase[2];
        trResult->trBase[2] = AngleNormalize180(trTo->trBase[2] - v8) * lerp + v8;
    }
    else
    {
        Vec3Lerp(trFrom->trBase, trTo->trBase, lerp, trResult->trBase);
    }
    Vec3Lerp(trFrom->trDelta, trTo->trDelta, lerp, trResult->trDelta);
    v7 = floor((float)((float)(trTo->trDuration - trFrom->trDuration) * lerp));
    trResult->trDuration = trFrom->trDuration + (int)v7;
    v6 = floor((float)((float)(cgameGlob->nextSnap->physicsTime - cgameGlob->snap->physicsTime) * lerp));
    trResult->trTime = cgameGlob->snap->physicsTime + (int)v6;
    trResult->trType = trFrom->trType;
}

void __cdecl CG_InterpolateVehicleDriver(const cg_s *cgameGlob, centity_s *cent, int curTime)
{
    trajectory_t v3; // [esp-4Ch] [ebp-60h] BYREF
    trajectory_t v4; // [esp-28h] [ebp-3Ch] BYREF
    float timeDifference; // [esp+10h] [ebp-4h]
    int savedregs; // [esp+14h] [ebp+0h] BYREF

    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    2659,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    if ( !cent->nitrousVeh
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    2660,
                    0,
                    "%s",
                    "cent->nitrousVeh") )
    {
        __debugbreak();
    }
    if ( cent->nitrousVeh->m_phys_user_data )
    {
        //NitrousVehicle::GetEntityPose(cent->nitrousVeh, cent->pose.origin, cent->pose.angles, curTime, 1);
        cent->nitrousVeh->GetEntityPose(cent->pose.origin, cent->pose.angles, curTime, true);
        timeDifference = (float)(Phys_GetCurrentTime() - cgameGlob->nextSnap->physicsTime) / 1000.0;
        if ( cent->nitrousVeh->m_lastNetworkTime != cgameGlob->nextSnap->serverTime )
        {
            if ( cgameGlob->inKillCam )
            {
                memcpy(&v4, &cent->nextState.lerp.apos, sizeof(v4));
                memcpy(&v3, &cent->nextState.lerp.pos, sizeof(v3));
                //NitrousVehicle::update_from_network(
                //    cent->nitrousVeh,
                //    (phys_vec3 *)&savedregs,
                //    v3,
                //    v4,
                //    (LerpEntityStateVehicle *)&cent->nextState.lerp.u,
                //    timeDifference);
                cent->nitrousVeh->update_from_network(v3, v4, &cent->nextState.lerp.u.vehicle, timeDifference);
            }
            cent->nitrousVeh->m_lastNetworkTime = cgameGlob->nextSnap->serverTime;
        }
    }
}

void __cdecl CG_CalcEntityRagdollPositions(int localClientNum, centity_s *cent)
{
    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 2982, 0, "%s", "cent") )
        __debugbreak();
    if ( !CG_IsRagdollTrajectory(&cent->currentState.pos)
        && !CG_IsRagdollTrajectory(&cent->currentState.apos)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    2983,
                    0,
                    "%s",
                    "CG_IsRagdollTrajectory( &cent->currentState.pos ) || CG_IsRagdollTrajectory( &cent->currentState.apos )") )
    {
        __debugbreak();
    }
    if ( ((*((unsigned int *)cent + 201) >> 21) & 1) != 0 )
    {
        if ( cent->pose.ragdollHandle )
        {
            Ragdoll_Remove(cent->pose.ragdollHandle);
            cent->pose.ragdollHandle = 0;
        }
    }
    else
    {
        if ( !cent->pose.ragdollHandle && !cent->pose.killcamRagdollHandle )
            CG_CreateRagdollObject(localClientNum, cent);
        if ( cent->pose.ragdollHandle > 0 || cent->pose.killcamRagdollHandle > 0 )
            CG_UpdateRagdollPose(cent);
    }
}

void __cdecl CG_UpdateRagdollPose(centity_s *cent)
{
    if ( cent->pose.ragdollHandle <= 0
        && cent->pose.killcamRagdollHandle <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    2971,
                    0,
                    "%s",
                    "(cent->pose.ragdollHandle > RAGDOLL_INVALID) || (cent->pose.killcamRagdollHandle > RAGDOLL_INVALID)") )
    {
        __debugbreak();
    }
    if (cent->pose.killcamRagdollHandle <= 0)
    {
        Ragdoll_GetRootOrigin(cent->pose.ragdollHandle, cent->pose.origin);
    }
    else
    {
        Ragdoll_GetRootOrigin(cent->pose.killcamRagdollHandle, cent->pose.origin);
    }
}

void __cdecl CG_UpdateTags(centity_s *ent)
{
    int k; // [esp+0h] [ebp-Ch]
    int j; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    if ( ent->pose.isRagdoll )
    {
        if ( ent->pose.killcamRagdollHandle > 0 )
            Ragdoll_RebindBody(ent->pose.killcamRagdollHandle);
        if ( ent->pose.ragdollHandle > 0 )
            Ragdoll_RebindBody(ent->pose.ragdollHandle);
    }
    if ( ent->nextState.eType == 14 || ent->nextState.eType == 16 )
    {
        ent->pose.vehicle.tag_body = -2;
        ent->pose.vehicle.tag_turret = -2;
        ent->pose.vehicle.tag_barrel = -2;
        ent->pose.vehicle.tag_barrel_recoil = -2;
        for ( i = 0; i < 4; ++i )
        {
            ent->pose.vehicle.tag_gunner_turret[i] = -2;
            ent->pose.vehicle.tag_gunner_barrel[i] = -2;
        }
        for ( j = 0; j < 6; ++j )
            ent->pose.vehicle.wheelBoneIndex[j] = -2;
        for ( k = 0; k < 4; ++k )
            ent->pose.vehicle.tag_extra_tank_wheels[k] = -2;
    }
}

DObj *__cdecl CG_PreProcess_GetDObj(int localClientNum, int entIndex, int entType, XModel *model, XModel *model2)
{
    unsigned int v5; // eax
    unsigned int v6; // eax
    unsigned int v7; // eax
    XAnimTree_s *Tree; // [esp+0h] [ebp-140h]
    unsigned int v10; // [esp+4h] [ebp-13Ch]
    float *v11; // [esp+10h] [ebp-130h]
    unsigned int number; // [esp+14h] [ebp-12Ch]
    DObj *obj; // [esp+28h] [ebp-118h]
    unsigned int numModels; // [esp+2Ch] [ebp-114h]
    unsigned __int16 numModelsa; // [esp+2Ch] [ebp-114h]
    XAnimTree_s *animTree; // [esp+30h] [ebp-110h]
    centity_s *cent; // [esp+34h] [ebp-10Ch]
    bool dobjExisted; // [esp+3Bh] [ebp-105h]
    XAnim_s *anims; // [esp+3Ch] [ebp-104h]
    DObjModel_s dobjModels[32]; // [esp+40h] [ebp-100h] BYREF

    animTree = 0;
    obj = Com_GetClientDObj(entIndex, localClientNum);
    cent = CG_GetEntity(localClientNum, entIndex);
    dobjExisted = obj != 0;
    if ( obj && (!model || !CG_CheckDObjInfoMatches(localClientNum, entIndex, entType, model)) )
    {
        if ( cent->pose.physObjId != -1 && cent->pose.physObjId )
        {
            number = cent->nextState.number;
            if ( localClientNum
                && !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_world.h",
                            17,
                            0,
                            "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                            localClientNum,
                            1) )
            {
                __debugbreak();
            }
            if ( number >= 0x600
                && !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_world.h",
                            18,
                            0,
                            "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                            number,
                            1536) )
            {
                __debugbreak();
            }
            if ( localClientNum
                && !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                            108,
                            0,
                            "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                            localClientNum,
                            1) )
            {
                __debugbreak();
            }
            if ( number >= 0x600
                && !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                            109,
                            0,
                            "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                            number,
                            1536) )
            {
                __debugbreak();
            }
            if ( cgEntCollNodes[localClientNum][number].sector )
                CG_UnlinkEntity(localClientNum, cent->nextState.number);
            Phys_ObjDestroy(0, cent->pose.physObjId);
            cent->pose.physObjId = 0;
        }
        FX_MarkEntDetachAll(localClientNum, entIndex);
        CG_SafeDObjFree(localClientNum, entIndex);
        obj = 0;
    }
    if ( !obj && model )
    {
        if ( cent->tree
            && !cent->destructible
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                        3335,
                        0,
                        "%s",
                        "!cent->tree || cent->destructible") )
        {
            __debugbreak();
        }
        if ( !cent->tree )
        {
            anims = CG_GetAnimations(localClientNum, entIndex, entType);
            if ( anims )
                Tree = XAnimCreateTree(anims, (void *(__cdecl *)(unsigned int))CG_AllocAnimTree);
            else
                Tree = 0;
            animTree = Tree;
            cent->tree = Tree;
        }
        dobjModels[0].model = model;
        dobjModels[0].boneName = 0;
        dobjModels[0].ignoreCollision = 0;
        numModels = 1;
        if ( model2 )
        {
            dobjModels[1].model = model2;
            dobjModels[1].boneName = 0;
            dobjModels[1].ignoreCollision = 1;
            numModels = 2;
        }
        v5 = CG_AddVehicleAttachedModel(cent, dobjModels, numModels, localClientNum);
        v6 = CG_AddScriptMoverAttachedModel(cent, dobjModels, v5, localClientNum);
        v7 = CG_DestructibleUpdate(cent, dobjModels, v6, localClientNum);
        numModelsa = CG_AddClientScriptAttachedModel(cent, dobjModels, v7, localClientNum);
        obj = Com_ClientDObjCreate(dobjModels, numModelsa, animTree, entIndex, localClientNum);
        DObjSetHidePartBits(obj, cent->nextState.partBits);
        if ( entIndex < 1536 )
            CG_SetDObjInfo(localClientNum, entIndex, entType, model);
        v11 = cg_entityOriginArray[localClientNum][cent->nextState.number];
        *v11 = 131072.0f;
        v11[1] = 131072.0f;
        v11[2] = 131072.0f;
        CG_UpdateTags(cent);
    }
    if ( !dobjExisted && obj )
    {
        v10 = cent->nextState.number;
        if ( localClientNum
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_world.h",
                        17,
                        0,
                        "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        localClientNum,
                        1) )
        {
            __debugbreak();
        }
        if ( v10 >= 0x600
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_world.h",
                        18,
                        0,
                        "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                        v10,
                        1536) )
        {
            __debugbreak();
        }
        if ( localClientNum
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                        108,
                        0,
                        "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        localClientNum,
                        1) )
        {
            __debugbreak();
        }
        if ( v10 >= 0x600
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_colltree.h",
                        109,
                        0,
                        "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                        v10,
                        1536) )
        {
            __debugbreak();
        }
        if ( cgEntCollNodes[localClientNum][v10].sector )
            CG_UnlinkEntity(localClientNum, cent->nextState.number);
    }
    return obj;
}

XAnim_s *__cdecl CG_GetAnimations(int localClientNum, int entIndex, int entType)
{
    centity_s *cent; // [esp+Ch] [ebp-4h]

    if ( entType == 11 )
    {
        cent = CG_GetEntity(localClientNum, entIndex);
        return CG_GetMG42Anims(cent);
    }
    else if ( entType == 17 || entType == 19 )
    {
        return Dog_GetAnims();
    }
    else
    {
        return 0;
    }
}

XAnim_s *__cdecl CG_GetMG42Anims(centity_s *cent)
{
    const WeaponVariantDef *weapVariantDef; // [esp+0h] [ebp-8h]
    XAnim_s *pAnims; // [esp+4h] [ebp-4h]

    if ( !cent->nextState.weapon
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    255,
                    0,
                    "%s",
                    "cent->nextState.weapon") )
    {
        __debugbreak();
    }
    weapVariantDef = BG_GetWeaponVariantDef(cent->nextState.weapon);
    pAnims = XAnimCreateAnims("MG42", 3, Hunk_AllocXAnimClient);
    if ( !pAnims
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 259, 0, "%s", "pAnims") )
    {
        __debugbreak();
    }
    XAnimBlend(pAnims, 0, "root", 1u, 2u, 0);
    BG_CreateXAnim(pAnims, 1u, *((char **)weapVariantDef->szXAnims + 1));
    BG_CreateXAnim(pAnims, 2u, *((char **)weapVariantDef->szXAnims + 3));
    return pAnims;
}

unsigned int __cdecl CG_AddClientScriptAttachedModel(
                centity_s *cent,
                DObjModel_s *dobjModels,
                unsigned int numModels,
                int localClientNum)
{
    const cgs_t *cgs; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    if ( cent->nextState.eType != 16 )
    {
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        for ( i = 0; i < 2; ++i )
        {
            if ( cent->attachModelNames[i] )
            {
                dobjModels[numModels].model = cgs->gameModels[cent->attachModelNames[i]];
                dobjModels[numModels++].boneName = cent->attachTagNames[i];
            }
        }
    }
    return numModels;
}

unsigned int __cdecl CG_AddVehicleAttachedModel(
                centity_s *cent,
                DObjModel_s *dobjModels,
                unsigned int numModels,
                int localClientNum)
{
    char *tagName; // [esp+4h] [ebp-10h]
    const cgs_t *cgs; // [esp+8h] [ebp-Ch]
    int i; // [esp+Ch] [ebp-8h]
    int modelIndex; // [esp+10h] [ebp-4h]

    if ( cent->nextState.eType == 14 || cent->nextState.eType == 16 )
    {
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        for ( i = 0; i < 2; ++i )
        {
            modelIndex = cent->nextState.vehicleState.attachModelIndex[i];
            if ( !cent->nextState.vehicleState.attachModelIndex[i] )
                break;
            if ( numModels >= 0x20
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                            3250,
                            0,
                            "%s",
                            "numModels < DOBJ_MAX_SUBMODELS") )
            {
                __debugbreak();
            }
            dobjModels[numModels].model = cgs->gameModels[modelIndex];
            if ( !dobjModels[numModels].model
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                            3252,
                            0,
                            "%s",
                            "dobjModels[numModels].model") )
            {
                __debugbreak();
            }
            if ( !cent->nextState.vehicleState.attachModelIndex[i]
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                            3253,
                            0,
                            "%s",
                            "cent->nextState.vehicleState.attachModelIndex[i]") )
            {
                __debugbreak();
            }
            tagName = CL_GetConfigString(cent->nextState.lerp.u.primaryLight.colorAndExp[i] + 3115);
            dobjModels[numModels].boneName = SL_FindString(tagName, SCRIPTINSTANCE_SERVER);
            dobjModels[numModels++].ignoreCollision = 0;
        }
    }
    return numModels;
}

unsigned int __cdecl CG_AddScriptMoverAttachedModel(
                centity_s *cent,
                DObjModel_s *dobjModels,
                unsigned int numModels,
                int localClientNum)
{
    char *tagName; // [esp+4h] [ebp-10h]
    const cgs_t *cgs; // [esp+8h] [ebp-Ch]
    int i; // [esp+Ch] [ebp-8h]
    int modelIndex; // [esp+10h] [ebp-4h]

    if ( cent->nextState.eType == 6 )
    {
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        for ( i = 0; i < 4; ++i )
        {
            modelIndex = *(&cent->nextState.lerp.u.vehicle.gunnerAngles[0].pitch + i);
            if ( !*(&cent->nextState.lerp.u.vehicle.gunnerAngles[0].pitch + i) )
                break;
            if ( numModels >= 0x20
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                            3281,
                            0,
                            "%s",
                            "numModels < DOBJ_MAX_SUBMODELS") )
            {
                __debugbreak();
            }
            dobjModels[numModels].model = cgs->gameModels[modelIndex];
            if ( !dobjModels[numModels].model
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                            3283,
                            0,
                            "%s",
                            "dobjModels[numModels].model") )
            {
                __debugbreak();
            }
            if ( !*(&cent->nextState.lerp.u.vehicle.gunnerAngles[0].pitch + i)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                            3284,
                            0,
                            "%s",
                            "cent->nextState.lerp.u.scriptMover.attachModelIndex[i]") )
            {
                __debugbreak();
            }
            tagName = CL_GetConfigString(cent->nextState.lerp.u.primaryLight.colorAndExp[i] + 3115);
            dobjModels[numModels].boneName = SL_FindString(tagName, SCRIPTINSTANCE_SERVER);
            dobjModels[numModels++].ignoreCollision = 0;
        }
    }
    return numModels;
}

void __cdecl CG_DObjCalcBone(const cpose_t *pose, DObj *obj, int boneIndex)
{
    DObjAnimMat *boneMatrix; // [esp+8h] [ebp-1Ch] BYREF
    int partBits[5]; // [esp+10h] [ebp-14h] BYREF

    iassert(obj);
    iassert(pose);

    DObjLock(obj);

    if ( CL_DObjCreateSkelForBone(obj, boneIndex, &boneMatrix) )
    {
        DObjUnlock(obj);
    }
    else
    {
        PROF_SCOPED("CG_DObjCalcBone");

        DObjGetHierarchyBits(obj, boneIndex, partBits);
        CG_DoControllers(pose, obj, partBits);
        DObjCalcSkel(obj, partBits);
        DObjUnlock(obj);
    }
}

void __cdecl CG_ClearUnion(int localClientNum, centity_s *cent)
{
    switch ( cent->pose.eTypeUnion )
    {
        case 0u:
        case 0xBu:
        case 0x11u:
        case 0x13u:
            *(_QWORD *)&cent->pose.player.control = 0;
            *(_QWORD *)cent->pose.general.mins = 0;
            *(_QWORD *)&cent->pose.general.mins[2] = 0;
            *(_QWORD *)&cent->pose.general.maxs[1] = 0;
            break;
        case 1u:
            *(_QWORD *)&cent->pose.player.control = 0;
            *(_QWORD *)cent->pose.general.mins = 0;
            *(_QWORD *)&cent->pose.general.mins[2] = 0;
            break;
        case 8u:
        case 9u:
            FX_ThroughWithEffect(localClientNum, cent->pose.fx.effect, 1);
            *(_QWORD *)&cent->pose.player.control = 0;
            break;
        case 0xCu:
        case 0xEu:
            //memset(&cent->pose.108, 0, sizeof(cent->pose.108));
            memset(&cent->pose.vehicle, 0, sizeof(cent->pose.vehicle)); // biggest union member
            break;
        default:
            break;
    }
    cent->pose.eTypeUnion = 0;
}

void __cdecl CG_SetUnionType(int localClientNum, centity_s *cent)
{
    //memset(&cent->pose.108, 0, sizeof(cent->pose.108));
    memset(&cent->pose.vehicle, 0, sizeof(cent->pose.vehicle)); // biggest union member
    switch ( cent->nextState.eType )
    {
        case 1:
        case 8:
        case 9:
        case 0xB:
        case 0xC:
        case 0xE:
        case 0x11:
            cent->pose.eTypeUnion = cent->nextState.eType;
            if ( cent->pose.eTypeUnion != cent->nextState.eType
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                            3512,
                            0,
                            "%s",
                            "cent->pose.eTypeUnion == cent->nextState.eType") )
            {
                __debugbreak();
            }
            break;
        default:
            cent->pose.eTypeUnion = 0;
            break;
    }
}

void __cdecl CG_UpdatePoseUnion(int localClientNum, centity_s *cent)
{
    CG_ClearUnion(localClientNum, cent);
    CG_SetUnionType(localClientNum, cent);
}

void __cdecl CG_ProcessEntity(int localClientNum, centity_s *cent)
{
    unsigned __int16 t; // [esp+54h] [ebp-10h]

    CG_EntityEffects(localClientNum, cent);
    if ( cent->nextState.eType != cent->pose.eTypeUnion )
        CG_UpdatePoseUnion(localClientNum, cent);
    switch ( cent->nextState.eType )
    {
        case ET_GENERAL:
        {
            PROF_SCOPED("CG_General");
            CG_General(localClientNum, cent);
            break;
        }
        case ET_PLAYER:
        {
            PROF_SCOPED("CG_Player");
            CG_Player(localClientNum, cent);
            break;
        }
        case ET_PLAYER_CORPSE:
        {
            PROF_SCOPED("CG_Corpse");
            CG_Corpse(localClientNum, cent);
            break;
        }
        case ET_ITEM:
        {
            PROF_SCOPED("CG_Item");
            CG_Item(localClientNum, cent);
            break;
        }
        case ET_MISSILE:
        {
            PROF_SCOPED("CG_Missile");
            CG_Missile(localClientNum, cent);
            break;
        }
        case ET_INVISIBLE:
        case ET_SOUND_BLEND:
        case ET_STREAMER_HINT:
            break;
        case ET_SCRIPTMOVER:
        {
            PROF_SCOPED("CG_ScriptMover");
            CG_ScriptMover(localClientNum, cent);
            break;
        }
        case ET_PRIMARY_LIGHT:
        {
            PROF_SCOPED("CG_PrimaryLight");
            CG_PrimaryLight(localClientNum, cent);
            break;
        }
        case ET_MG42:
        {
            PROF_SCOPED("CG_mg42");
            CG_mg42(localClientNum, cent);
            CG_CompassUpdateTurretInfo(localClientNum, cent->nextState.number);
            break;
        }
        case ET_HELICOPTER:
        {
            PROF_SCOPED("CG_Vehicle");
            CG_Vehicle(localClientNum, cent);
            break;
        }
        case ET_PLANE:
        {
            PROF_SCOPED("CG_ScriptMover(ET_PLANE)"); // LWSS modified prof name here (re-used in asm with above scriptmover)
            CG_CompassUpdateVehicleInfo(localClientNum, cent->nextState.number);
            CG_ScriptMover(localClientNum, cent);
            break;
        }
        case ET_VEHICLE:
        case ET_VEHICLE_CORPSE:
        {
            PROF_SCOPED("CG_Vehicle");
            CG_Vehicle(localClientNum, cent);
            break;
        }
        case ET_ACTOR:
        {
            CG_Actor(localClientNum, cent);
            CG_CompassUpdateDogInfo(localClientNum, cent->nextState.number);
            break;
        }
        case ET_ACTOR_CORPSE:
            CG_ActorCorpse(localClientNum, cent);
            break;
        default:
            Com_Error(ERR_DROP, "Bad entity type: %i", cent->nextState.eType);
            break;
    }
    if ( CG_EntityNeedsScriptThread(localClientNum, cent) )
    {
        *((unsigned int *)cent + 201) |= 0x100u;
        Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
        t = CScr_ExecEntThread(cent, cg_scr_data.entityspawned, 1u);
        Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
    }
    CG_ClientFlagCallback(localClientNum, cent);
    CG_ClientFlagResetAll(cent);
}

void __cdecl CG_Item(int localClientNum, centity_s *cent)
{
    unsigned int NumWeapons; // eax
    DObj *obj; // [esp+40h] [ebp-30h]
    const WeaponVariantDef *weapVariantDef; // [esp+48h] [ebp-28h]
    int textureOverrideIndex; // [esp+4Ch] [ebp-24h]
    unsigned __int8 weapModel; // [esp+53h] [ebp-1Dh]
    unsigned int weapIdx; // [esp+54h] [ebp-1Ch]
    unsigned int renderFxFlags; // [esp+58h] [ebp-18h]
    float lightingOrigin[3]; // [esp+5Ch] [ebp-14h] BYREF
    const WeaponDef *weapDef; // [esp+68h] [ebp-8h]
    const playerState_s *ps; // [esp+6Ch] [ebp-4h]

    ps = CG_GetPredictedPlayerState(localClientNum);
    if ( cent->nextState.un3.item >= 2048 )
        Com_Error(ERR_DROP, "Bad item index %i on entity", cent->nextState.un3.item);
    if ( (cent->nextState.lerp.eFlags & 0x20) == 0
        && (((unsigned int)0x800000 & cent->nextState.lerp.eFlags2) == 0 || (ps->perks[1] & 0x200) != 0) )
    {
        weapIdx = cent->nextState.un3.item % 2048;
        if ( weapIdx < BG_GetNumWeapons() )
        {
            weapModel = cent->nextState.un3.item / 2048;
            weapVariantDef = BG_GetWeaponVariantDef(weapIdx);
            weapDef = BG_GetWeaponDef(weapIdx);
            if ( weapDef->bUseDroppedModelAsStowed
                && *((unsigned int *)weapDef->worldModel + 1)
                && weapDef->weapClass != WEAPCLASS_GRENADE
                && weapDef->weapClass != WEAPCLASS_KILLSTREAK_ALT_STORED_WEAPON )
            {
                weapModel = 1;
            }
            if ( !weapDef
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 196, 0, "%s", "weapDef") )
            {
                __debugbreak();
            }
            if ( !weapDef->worldModel[weapModel] )
                Com_Error(ERR_DROP, "No XModel loaded for item index %i, weap index %i, model %i (%s)", cent->nextState.un3.item, weapIdx, weapModel, weapVariantDef->szDisplayName);
            obj = CG_PreProcess_GetDObj(
                            localClientNum,
                            cent->nextState.number,
                            cent->nextState.eType,
                            weapDef->worldModel[weapModel],
                            0);
            if ( obj )
            {
                lightingOrigin[0] = cent->pose.origin[0];
                lightingOrigin[1] = cent->pose.origin[1];
                lightingOrigin[2] = cent->pose.origin[2] + 4.0;
                textureOverrideIndex = CG_SetupWeaponOptionsRender(
                                                                 localClientNum,
                                                                 cent->pose.origin,
                                                                 weapVariantDef,
                                                                 cent->nextState.renderOptions,
                                                                 0,
                                                                 0);
                renderFxFlags = 0;
                if ( (cent->nextState.lerp.eFlags2 & 0x4000000) != 0 )
                    renderFxFlags = 0x200000;
                R_AddDObjToScene(
                    obj,
                    &cent->pose,
                    cent->nextState.number,
                    renderFxFlags,
                    lightingOrigin,
                    0.0,
                    0.0,
                    0.0,
                    0.0,
                    -1,
                    textureOverrideIndex,
                    0,
                    0,
                    0.0,
                    1.0);
            }
        }
        else if ( weapIdx >= BG_GetNumWeapons() )
        {
            NumWeapons = BG_GetNumWeapons();
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                            181,
                            0,
                            "weapIdx doesn't index BG_GetNumWeapons()\n\t%i not in [0, %i)",
                            weapIdx,
                            NumWeapons) )
                __debugbreak();
        }
    }
}

void __cdecl CG_EntityEffects(int localClientNum, centity_s *cent)
{
    if ( cent->nextState.loopSoundId )
        CG_PumpEntityLoopSound(localClientNum, cent);
    if ( CG_IsRagdollTrajectory(&cent->currentState.pos) )
        CG_CalcEntityLerpPositions(localClientNum, cent);
}

void __cdecl CG_mg42(int localClientNum, centity_s *cent)
{
    cg_s *LocalClientGlobals; // eax
    int i; // [esp+3Ch] [ebp-8Ch]
    float radius; // [esp+44h] [ebp-84h]
    usercmd_s PlayerCmd; // [esp+48h] [ebp-80h] BYREF
    int CmdIndex; // [esp+80h] [ebp-48h]
    playerState_s *p_predictedPlayerState; // [esp+84h] [ebp-44h]
    const clientInfo_t *client; // [esp+88h] [ebp-40h]
    DObj *obj; // [esp+8Ch] [ebp-3Ch]
    team_t team; // [esp+90h] [ebp-38h]
    cg_s *cgameGlob; // [esp+94h] [ebp-34h]
    team_t clTeam; // [esp+98h] [ebp-30h]
    bool isOwner; // [esp+9Fh] [ebp-29h]
    int inPlacement; // [esp+A0h] [ebp-28h]
    const cgs_t *cgs; // [esp+A4h] [ebp-24h]
    int goodPlacement; // [esp+A8h] [ebp-20h]
    unsigned int renderFxFlags; // [esp+ACh] [ebp-1Ch]
    float lightingOrigin[3]; // [esp+B0h] [ebp-18h] BYREF
    const playerState_s *ps; // [esp+BCh] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+C0h] [ebp-8h]
    const entityState_s *ns; // [esp+C4h] [ebp-4h]

    inPlacement = 0;
    goodPlacement = 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    ns = &cent->nextState;
    if ( (cent->nextState.lerp.eFlags & 0x20) == 0 )
    {
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        weapDef = BG_GetWeaponDef(ns->weapon);
        if ( !weapDef
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 479, 0, "%s", "weapDef") )
        {
            __debugbreak();
        }
        if ( weapDef )
        {
            client = &cgameGlob->bgs.clientinfo[cgameGlob->clientNum];
            clTeam = client->team;
            team = (team_t)(cent->nextState.faction.iHeadIconTeam & 3);
            isOwner = cgameGlob->clientNum == (int)cent->nextState.faction.iHeadIconTeam >> 2;
            if ( !*((unsigned int *)weapDef->worldModel + 1)
                || (ps->perks[1] & 0x800) == 0
                || (cent->nextState.lerp.u.turret.flags & 8) != 0
                || clTeam == team && (clTeam || isOwner) )
            {
                obj = CG_PreProcess_GetDObj(localClientNum, ns->number, ns->eType, cgs->gameModels[ns->index.brushmodel], 0);
            }
            else
            {
                obj = CG_PreProcess_GetDObj(
                                localClientNum,
                                ns->number,
                                ns->eType,
                                weapDef->worldModel[1],
                                cgs->gameModels[ns->index.brushmodel]);
            }
            if ( obj )
            {
                CG_mg42_PreControllers(obj, cent);
                CG_mg42_OverheatUpdate(localClientNum, obj, cent);
                if ( !CG_PlayerUsingScopedTurret(localClientNum)
                    || CG_GetLocalClientGlobals(localClientNum)->renderingThirdPerson
                    || (LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum),
                            p_predictedPlayerState = &LocalClientGlobals->predictedPlayerState,
                            LocalClientGlobals->predictedPlayerState.viewlocked_entNum != ns->number) )
                {
                    if ( CG_IsCarriedTurretEnt(localClientNum, cent) )
                    {
                        for ( i = 0; i < 2; ++i )
                            PlayerCmd.button_bits.array[i] = 0;
                        inPlacement = 1;
                        goodPlacement = Turret_PlaceTurret_Client(localClientNum, ns, cent->pose.origin, cent->pose.angles);
                        radius = DObjGetRadius(obj) + 16.0;
                        R_LinkDObjEntity(localClientNum, ns->number, cent->pose.origin, radius);
                        CmdIndex = CL_GetCurrentCmdNumber(localClientNum);
                        CL_GetUserCmd(localClientNum, CmdIndex, &PlayerCmd);
                        if ( !goodPlacement
                            && PlayerCmd.button_bits.testBit(0)
                            && cgameGlob->invalidCmdHintType != INVALID_CMD_CANT_EQUIP_WHILE_PRONE )
                        {
                            cgameGlob->invalidCmdHintType = INVALID_CMD_CANT_EQUIP_WHILE_PRONE;
                            cgameGlob->invalidCmdHintTime = cgameGlob->time;
                        }
                    }
                    lightingOrigin[0] = cent->pose.origin[0];
                    lightingOrigin[1] = cent->pose.origin[1];
                    lightingOrigin[2] = cent->pose.origin[2];
                    lightingOrigin[2] = lightingOrigin[2] + 32.0;
                    renderFxFlags = 4;
                    if ( (ns->lerp.eFlags2 & 0x4000000) != 0 )
                        renderFxFlags |= 0x200000u;
                    R_AddDObjToScene(
                        obj,
                        &cent->pose,
                        ns->number,
                        renderFxFlags,
                        lightingOrigin,
                        0.0,
                        0.0,
                        0.0,
                        0.0,
                        -1,
                        -1,
                        0,
                        0,
                        0.0,
                        1.0);
                }
            }
        }
    }
}

//00000000 union LerpEntityStateActor::<unnamed_type_index> // sizeof=0x4
//00000000 {                                       // XREF: LerpEntityStateActor/r
//00000000     int actorNum;
//00000000     int corpseNum;
//00000000 };
void __cdecl CG_Missile(int localClientNum, centity_s *cent)
{
    unsigned int weapon; // esi
    unsigned int AliasId; // eax
    unsigned int v4; // eax
    DObj *obj; // [esp+38h] [ebp-20h]
    entityState_s *s1; // [esp+3Ch] [ebp-1Ch]
    const cg_s *cgameGlob; // [esp+40h] [ebp-18h]
    //  LerpEntityStateActor::<unnamed_type_index> time; // [esp+44h] [ebp-14h]
    int time;
    float lightingOrigin[3]; // [esp+48h] [ebp-10h] BYREF
    const WeaponDef *weapDef; // [esp+54h] [ebp-4h]

    s1 = &cent->nextState;
    if ( (cent->nextState.lerp.eFlags & 0x20) == 0 )
    {
        time = CG_GetLocalClientGlobals(localClientNum)->time;
        if ( cent->nextState.lerp.u.actor.actorNum <= time )
        {
            weapon = cent->nextState.weapon;
            if ( weapon >= BG_GetNumWeapons() )
                cent->nextState.weapon = 0;
            CG_GetLocalClientWeaponInfo(localClientNum, cent->nextState.weapon);
            weapDef = BG_GetWeaponDef(cent->nextState.weapon);
            if ( weapDef->bRetrievable )
            {
                if ( cent->miscTime
                    || (float)((float)((float)(cent->currentState.pos.trDelta[0] * cent->currentState.pos.trDelta[0])
                                                     + (float)(cent->currentState.pos.trDelta[1] * cent->currentState.pos.trDelta[1]))
                                     + (float)(cent->currentState.pos.trDelta[2] * cent->currentState.pos.trDelta[2])) > 0.000001 )
                {
                    if ( cent->miscTime || !CG_EntGetLinkToParent(localClientNum, cent) )
                    {
                        if ( cent->miscTime < cent->nextState.lerp.u.actor.actorNum )
                            cent->miscTime = 0;
                    }
                    else
                    {
                        cent->miscTime = time;
                    }
                }
                else
                {
                    cent->miscTime = time;
                }
            }
            AliasId = SND_FindAliasId(weapDef->projectileSound);
            CG_PlaySound(localClientNum, cent->nextState.number, cent->pose.origin, 0, 0, 1.0, AliasId);
            if ( IsWeaponRetrieveable(localClientNum, cent, weapDef, cent->nextState.weapon) )
                obj = CG_PreProcess_GetDObj(
                                localClientNum,
                                cent->nextState.number,
                                cent->nextState.eType,
                                weapDef->worldModel[2],
                                weapDef->projectileModel);
            else
                obj = CG_PreProcess_GetDObj(
                                localClientNum,
                                cent->nextState.number,
                                cent->nextState.eType,
                                weapDef->projectileModel,
                                0);
            cgameGlob = CG_GetLocalClientGlobals(localClientNum);
            if ( ((*((unsigned int *)cent + 201) >> 4) & 1) != 0 )
            {
                if ( cent->lastTrailTime > cgameGlob->time )
                {
                    if ( cent->fxTrailHandle )
                    {
                        FX_ThroughWithEffect(localClientNum, cent->fxTrailHandle, 1);
                        if ( weapDef->projTrailEffect )
                            cent->fxTrailHandle = CG_PlayBoltedEffect(
                                                                            localClientNum,
                                                                            weapDef->projTrailEffect,
                                                                            s1->number,
                                                                            scr_const.tag_fx);
                    }
                    if ( cent->fxProjExplosion )
                    {
                        FX_ThroughWithEffect(localClientNum, cent->fxProjExplosion, 1);
                        if ( weapDef->projExplosion == WEAPPROJEXP_NAPALMBLOB )
                            cent->fxProjExplosion = CG_PlayBoltedEffect(
                                                                                localClientNum,
                                                                                weapDef->projExplosionEffect,
                                                                                s1->number,
                                                                                scr_const.tag_origin);
                    }
                }
            }
            else
            {
                *((unsigned int *)cent + 201) |= 0x10u;
                cent->fxTrailHandle = 0;
                cent->fxProjExplosion = 0;
                if ( weapDef->projTrailEffect )
                    cent->fxTrailHandle = CG_PlayBoltedEffect(
                                                                    localClientNum,
                                                                    weapDef->projTrailEffect,
                                                                    s1->number,
                                                                    scr_const.tag_fx);
                v4 = SND_FindAliasId(weapDef->projIgnitionSound);
                CG_PlaySound(localClientNum, s1->number, cent->pose.origin, 0, 0, 1.0, v4);
                if ( weapDef->projExplosion == WEAPPROJEXP_NAPALMBLOB )
                    cent->fxProjExplosion = CG_PlayBoltedEffect(
                                                                        localClientNum,
                                                                        weapDef->projExplosionEffect,
                                                                        s1->number,
                                                                        scr_const.tag_origin);
                if ( weapDef->weapType == WEAPTYPE_BOMB )
                    CG_AddArtilleryPing(localClientNum, cent->nextState.lerp.pos.trBase);
            }
            cent->lastTrailTime = cgameGlob->time;
            if ( obj )
            {
                if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_BALLISTIC )
                    SetBallisticAngles(localClientNum, cent);
                lightingOrigin[0] = cent->pose.origin[0];
                lightingOrigin[1] = cent->pose.origin[1];
                lightingOrigin[2] = cent->pose.origin[2];
                lightingOrigin[2] = lightingOrigin[2] + 4.0;
                R_AddDObjToScene(obj, &cent->pose, s1->number, 0, lightingOrigin, 0.0, 0.0, 0.0, 0.0, -1, -1, 0, 0, 0.0, 1.0);
                CG_AddHudGrenade((cg_s*)cgameGlob, cent);
                if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED )
                    CG_CompassUpdateGuidedMissileInfo(localClientNum, cent->nextState.number);
            }
        }
    }
}

void __cdecl SetBallisticAngles(int localClientNum, centity_s *cent)
{
    float snap_dir[3]; // [esp+8h] [ebp-6Ch] BYREF
    int project_time; // [esp+14h] [ebp-60h]
    float dir[3]; // [esp+18h] [ebp-5Ch] BYREF
    float origin[3]; // [esp+24h] [ebp-50h] BYREF
    float nextsnap_spin[3]; // [esp+30h] [ebp-44h] BYREF
    const cg_s *cgameGlob; // [esp+3Ch] [ebp-38h]
    float next_snap_dir[3]; // [esp+40h] [ebp-34h] BYREF
    float f; // [esp+4Ch] [ebp-28h]
    float origin_projected[3]; // [esp+50h] [ebp-24h] BYREF
    float snap_spin[3]; // [esp+5Ch] [ebp-18h] BYREF
    float spin[3]; // [esp+68h] [ebp-Ch] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->nextSnap )
    {
        f = cgameGlob->frameInterpolation;
        project_time = 5;
        BG_EvaluateTrajectory(&cent->currentState.pos, cgameGlob->snap->serverTime, origin);
        BG_EvaluateTrajectory(&cent->currentState.pos, cgameGlob->snap->serverTime + 5, origin_projected);
        snap_dir[0] = origin_projected[0] - origin[0];
        snap_dir[1] = origin_projected[1] - origin[1];
        snap_dir[2] = origin_projected[2] - origin[2];
        BG_EvaluateTrajectory(&cent->nextState.lerp.pos, cgameGlob->nextSnap->serverTime, origin);
        BG_EvaluateTrajectory(&cent->nextState.lerp.pos, cgameGlob->nextSnap->serverTime + 5, origin_projected);
        next_snap_dir[0] = origin_projected[0] - origin[0];
        next_snap_dir[1] = origin_projected[1] - origin[1];
        next_snap_dir[2] = origin_projected[2] - origin[2];
        Vec3Lerp(snap_dir, next_snap_dir, f, dir);
        vectoangles(dir, cent->pose.angles);
        BG_EvaluateTrajectory(&cent->currentState.apos, cgameGlob->snap->serverTime, snap_spin);
        BG_EvaluateTrajectory(&cent->nextState.lerp.apos, cgameGlob->nextSnap->serverTime, nextsnap_spin);
        Vec3Lerp(snap_spin, nextsnap_spin, f, spin);
        cent->pose.angles[2] = spin[2];
    }
}

bool __cdecl IsWeaponRetrieveable(
                int localClientNum,
                const centity_s *cent,
                const WeaponDef *weapDef,
                unsigned int weapIndex)
{
    const cg_s *cgameGlob; // [esp+1Ch] [ebp-Ch]
    bool bRetrievable; // [esp+23h] [ebp-5h]
    const playerState_s *ps; // [esp+24h] [ebp-4h]

    if ( !weapDef->bRetrievable )
        return 0;
    if ( !*((unsigned int *)weapDef->worldModel + 2) )
        return 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = CG_GetPredictedPlayerState(localClientNum);
    if ( !BG_PlayerHasWeapon(ps, weapIndex) )
        return 0;
    if ( (ps->perks[1] & 0x20000) != 0 )
    {
        if ( BG_WeaponAmmo(ps, weapIndex) > 1 )
            return 0;
    }
    else if ( BG_WeaponAmmo(ps, weapIndex) > 0 )
    {
        return 0;
    }
    if ( !cent->miscTime )
        return 0;
    bRetrievable = cgameGlob->time - cent->miscTime > cg_retrieveHintTime->current.integer;
    if ( cent->linkInfo )
    {
        if ( cent->linkInfo->linkEnt != 1023 )
            return cgameGlob->time - cent->miscTime > cg_retrieveHintTimeStuck->current.integer;
    }
    return bRetrievable;
}

void __cdecl CG_Vehicle(int localClientNum, centity_s *cent)
{
    int ShouldLocalPlayerSee; // eax
    unsigned int ShadowHintForRefEntity; // eax
    float v4; // [esp+0h] [ebp-80h]
    float v5; // [esp+4h] [ebp-7Ch]
    char v6; // [esp+10h] [ebp-70h]
    DObj *obj; // [esp+44h] [ebp-3Ch]
    cg_s *cgameGlob; // [esp+50h] [ebp-30h]
    int driver; // [esp+58h] [ebp-28h]
    const cgs_t *cgs; // [esp+5Ch] [ebp-24h]
    float lightingOrigin[3]; // [esp+64h] [ebp-1Ch] BYREF
    float materialTime2; // [esp+70h] [ebp-10h]
    float materialTime; // [esp+74h] [ebp-Ch]
    int altXModel; // [esp+78h] [ebp-8h]
    entityState_s *ns; // [esp+7Ch] [ebp-4h]

    PROF_SCOPED("CG_Vehicle");

    ns = &cent->nextState;
    if ( (cent->nextState.lerp.eFlags & 0x20) != 0 )
    {
        if ( cent->nitrousVeh )
        {
            NitrousVehicle::remove_vehicle(cent->nitrousVeh);
            cent->nitrousVeh = 0;
        }
        if ( (cent->currentState.eFlags & 0x20) == 0 )
            CScr_NotifyNum(localClientNum, cent->nextState.number, 0, cscr_const.hidden, 0);
    }
    else
    {
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        driver = CG_GetDriverClientNumFromVehicle(localClientNum, cent);
        if ( driver == 1023 )
            ShouldLocalPlayerSee = CG_WhatModelShouldLocalPlayerSee(
                                                             localClientNum,
                                                             cgameGlob,
                                                             cent,
                                                             cent->nextState.faction.iHeadIconTeam & 3,
                                                             ns->un3.item,
                                                             cent->nextState.enemyModel);
        else
            ShouldLocalPlayerSee = CG_WhatModelShouldLocalPlayerSee(
                                                             localClientNum,
                                                             cgameGlob,
                                                             cent,
                                                             cgameGlob->bgs.clientinfo[driver].team,
                                                             ns->un3.item,
                                                             cent->nextState.enemyModel);
        obj = CG_PreProcess_GetDObj(localClientNum, ns->number, ns->eType, cgs->gameModels[ShouldLocalPlayerSee], 0);
        if ( obj )
        {
            CG_Vehicle_PreControllers(localClientNum, obj, cent);
            if ( ns->eType == 14 || ns->eType == 12 )
            {
                CG_UpdateVehicleSounds(localClientNum, cent);
                CG_UpdateVehicleNotifies(localClientNum, cent);
            }
            lightingOrigin[0] = cent->pose.origin[0];
            lightingOrigin[1] = cent->pose.origin[1];
            lightingOrigin[2] = cent->pose.origin[2];
            lightingOrigin[2] = lightingOrigin[2] + 32.0;
            materialTime = (float)((float)CG_GetLocalClientGlobals(localClientNum)->time * 0.001)
                                     - cent->vehicle->materialTime;
            materialTime2 = (float)((float)CG_GetLocalClientGlobals(localClientNum)->time * 0.001)
                                        - cent->vehicle->materialTime2;
            altXModel = CG_Destructible_GetModelIndexFromLabel();
            v6 = altXModel;
            v5 = materialTime2;
            v4 = materialTime;
            ShadowHintForRefEntity = CG_GetShadowHintForRefEntity(ns->lerp.eFlags);
            R_AddDObjToScene(
                obj,
                &cent->pose,
                ns->number,
                ShadowHintForRefEntity | 4,
                lightingOrigin,
                v4,
                v5,
                0.0,
                0.0,
                v6,
                -1,
                0,
                0,
                0.0,
                1.0);
            if ( ns->eType == 14 )
            {
                CG_CompassUpdateVehicleInfo(localClientNum, ns->number);
                CG_CompassUpdateVehicleOccupantInfo(localClientNum, ns->number);
                CG_UpdateVehicleWaterFX(localClientNum, cent);
            }
            else if ( ns->eType == 12 )
            {
                CG_CompassUpdateHelicopterInfo(localClientNum, ns->number);
            }
        }
    }
}

void __cdecl CG_ClientFlagCallback(int localClientNum, centity_s *cent)
{
    unsigned __int16 t; // [esp+0h] [ebp-Ch]
    bool flagSet; // [esp+4h] [ebp-8h]
    int flagNum; // [esp+8h] [ebp-4h]

    if ( !G_ExitAfterToolComplete() )
    {
        if ( !cg_scr_data.clientFlagCB
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                        3540,
                        0,
                        "%s\n\t(cg_scr_data.clientFlagCB) = %p",
                        "(cg_scr_data.clientFlagCB != 0)",
                        0) )
        {
            __debugbreak();
        }
        if ( !cg_scr_data.clientFlagAsValCB
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                        3541,
                        0,
                        "%s\n\t(cg_scr_data.clientFlagAsValCB) = %p",
                        "(cg_scr_data.clientFlagAsValCB != 0)",
                        0) )
        {
            __debugbreak();
        }
        for ( flagNum = 0; flagNum < 16; ++flagNum )
        {
            if ( CG_ClientFlagIsActive(cent, flagNum) )
            {
                flagSet = CG_ClientFlagIsSet(cent, flagNum);
                Scr_AddInt(flagSet, SCRIPTINSTANCE_CLIENT);
                Scr_AddInt(flagNum, SCRIPTINSTANCE_CLIENT);
                Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
                t = CScr_ExecEntThread(cent, cg_scr_data.clientFlagCB, 3u);
                Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
            }
        }
        *((unsigned int *)cent + 201) &= ~0x400000u;
    }
}

void __cdecl CG_AddPacketFxEntity(int localClientNum, int entnum)
{
    centity_s *cent; // [esp+8h] [ebp-4h]

    cent = CG_GetEntity(localClientNum, entnum);
    CG_CalcFxEntityLerpPositions(cent);
    CG_ProcessFxEntity(localClientNum, cent);
}

void __cdecl CG_ProcessFxEntity(int localClientNum, centity_s *cent)
{
    iassert(cent->nextState.eType == cent->pose.eType);

    if ( cent->nextState.eType == ET_LOOP_FX)
    {
        PROF_SCOPED("CG_LoopFx");
        CG_LoopFx(localClientNum, cent);
        return;
    }

    iassert(cent->nextState.eType == ET_FX);

    PROF_SCOPED("CG_Fx");
    CG_Fx(localClientNum, cent);
}

void __cdecl CG_CalcFxEntityLerpPositions(centity_s *cent)
{
    cent->pose.origin[0] = cent->nextState.lerp.pos.trBase[0];
    cent->pose.origin[1] = cent->nextState.lerp.pos.trBase[1];
    cent->pose.origin[2] = cent->nextState.lerp.pos.trBase[2];
    cent->pose.angles[0] = cent->nextState.lerp.apos.trBase[0];
    cent->pose.angles[1] = cent->nextState.lerp.apos.trBase[1];
    cent->pose.angles[2] = cent->nextState.lerp.apos.trBase[2];
}

void __cdecl CG_GetPoseOrigin(const cpose_t *pose, float *origin)
{
    if ( !pose && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 3759, 0, "%s", "pose") )
        __debugbreak();
    *origin = pose->origin[0];
    origin[1] = pose->origin[1];
    origin[2] = pose->origin[2];
}

void __cdecl CG_GetPoseAxis(const cpose_t *pose, float (*axis)[3])
{
    if ( !pose && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 3771, 0, "%s", "pose") )
        __debugbreak();
    AnglesToAxis(pose->angles, axis);
}

void __cdecl CG_GetPoseQuat(const cpose_t *pose, float *quat)
{
    if ( !pose && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 3777, 0, "%s", "pose") )
        __debugbreak();
    AnglesToQuat(pose->angles, quat);
}

float *__cdecl CG_GetEntityOrigin(int localClientNum, unsigned int entnum)
{
    return CG_GetEntity(localClientNum, entnum)->pose.origin;
}

GfxSkinCacheEntry *__cdecl CG_GetSkinCacheEntry(const cpose_t *pose)
{
    return (GfxSkinCacheEntry*)&pose->skinCacheEntry;
}

void __cdecl CG_PredictiveSkinCEntity(GfxSceneEntity *sceneEnt)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]
    cpose_t *pose; // [esp+4h] [ebp-4h]

    if ( !sceneEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp", 3817, 0, "%s", "sceneEnt") )
    {
        __debugbreak();
    }
    pose = sceneEnt->info.pose;
    cgameGlob = CG_GetLocalClientGlobals(pose->localClientNum);
    if ( !cgameGlob || !cgameGlob->cameraMode )
    {
        if ( pose->cullIn == 1 )
        {
            pose->cullIn = 0;
            R_UpdateXModelBoundsDelayed(sceneEnt);
        }
        else if ( pose->cullIn == 2 )
        {
            pose->cullIn = 0;
            R_SkinGfxEntityDelayed(sceneEnt);
        }
    }
}

void __cdecl CG_SetOrigin(centity_s *ent, const float *origin)
{
    ent->nextState.lerp.pos.trBase[0] = *origin;
    ent->nextState.lerp.pos.trBase[1] = origin[1];
    ent->nextState.lerp.pos.trBase[2] = origin[2];
    ent->nextState.lerp.pos.trType = 0;
    ent->nextState.lerp.pos.trTime = 0;
    ent->nextState.lerp.pos.trDuration = 0;
    ent->nextState.lerp.pos.trDelta[0] = 0.0f;
    ent->nextState.lerp.pos.trDelta[1] = 0.0f;
    ent->nextState.lerp.pos.trDelta[2] = 0.0f;
    ent->pose.origin[0] = *origin;
    ent->pose.origin[1] = origin[1];
    ent->pose.origin[2] = origin[2];
}

void __cdecl CG_SetAngle(centity_s *ent, const float *angle)
{
    ent->nextState.lerp.apos.trBase[0] = *angle;
    ent->nextState.lerp.apos.trBase[1] = angle[1];
    ent->nextState.lerp.apos.trBase[2] = angle[2];
    ent->nextState.lerp.apos.trType = 0;
    ent->nextState.lerp.apos.trTime = 0;
    ent->nextState.lerp.apos.trDuration = 0;
    ent->nextState.lerp.apos.trDelta[0] = 0.0f;
    ent->nextState.lerp.apos.trDelta[1] = 0.0f;
    ent->nextState.lerp.apos.trDelta[2] = 0.0f;
    ent->pose.angles[0] = *angle;
    ent->pose.angles[1] = angle[1];
    ent->pose.angles[2] = angle[2];
}

bool test_1;
int __cdecl CG_WhatModelShouldLocalPlayerSee(
                int localClientNum,
                const cg_s *cgameGlob,
                const centity_s *cent,
                int centTeam,
                int friendlyModel,
                int enemyModel)
{
    int model; // [esp+0h] [ebp-Ch]
    team_t localPlayerTeam; // [esp+8h] [ebp-4h]

    model = friendlyModel;
    localPlayerTeam = cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team;
    if ( test_1 )
        model = enemyModel;
    if ( !enemyModel || (int)cent->nextState.faction.iHeadIconTeam >> 2 == cgameGlob->clientNum )
        return friendlyModel;
    if ( !centTeam )
        return enemyModel;
    if ( (cgameGlob->predictedPlayerState.otherFlags & 2) != 0 && !cgameGlob->inKillCam )
        localPlayerTeam = cgameGlob->bgs.clientinfo[cgameGlob->predictedPlayerState.clientNum].team;
    if ( localPlayerTeam != centTeam )
        return enemyModel;
    return model;
}

int __cdecl CG_GetDriverClientNumFromVehicle(int localClientNum, const centity_s *vehicle)
{
    const cg_s *cgameGlob; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    for ( i = 0;
                i < com_maxclients->current.integer
         && (!cgameGlob->bgs.clientinfo[i].infoValid
            || cgameGlob->bgs.clientinfo[i].attachedVehEntNum != vehicle->nextState.number
            || cgameGlob->bgs.clientinfo[i].attachedVehSeat);
                ++i )
    {
        ;
    }
    if ( i < com_maxclients->current.integer )
        return i;
    else
        return 1023;
}

void __cdecl CG_ClientFlagResetAll(centity_s *cent)
{
    if ( !cent
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    3942,
                    0,
                    "%s\n\t(cent) = %p",
                    "(cent != 0)",
                    0) )
    {
        __debugbreak();
    }
    cent->flagIndex = 0;
    cent->flagState = 0;
}

void __cdecl CG_ClientFlagSet(centity_s *cent, unsigned int flagNum)
{
    if ( flagNum >= 0x10
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    3950,
                    0,
                    "flagNum doesn't index NUM_CLIENT_FLAGS\n\t%i not in [0, %i)",
                    flagNum,
                    16) )
    {
        __debugbreak();
    }
    if ( !cent
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    3951,
                    0,
                    "%s\n\t(cent) = %p",
                    "(cent != 0)",
                    0) )
    {
        __debugbreak();
    }
    cent->flagIndex |= 1 << flagNum;
    cent->flagState |= 1 << flagNum;
    *((unsigned int *)cent + 201) |= 0x400000u;
}

void __cdecl CG_ClientFlagClear(centity_s *cent, unsigned int flagNum)
{
    if ( flagNum >= 0x10
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    3960,
                    0,
                    "flagNum doesn't index NUM_CLIENT_FLAGS\n\t%i not in [0, %i)",
                    flagNum,
                    16) )
    {
        __debugbreak();
    }
    if ( !cent
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    3961,
                    0,
                    "%s\n\t(cent) = %p",
                    "(cent != 0)",
                    0) )
    {
        __debugbreak();
    }
    cent->flagIndex |= 1 << flagNum;
    cent->flagState &= ~(unsigned __int16)(1 << flagNum);
    *((unsigned int *)cent + 201) |= 0x400000u;
}

bool __cdecl CG_ClientFlagIsActive(const centity_s *cent, unsigned int flagNum)
{
    if ( flagNum >= 0x10
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    3970,
                    0,
                    "flagNum doesn't index NUM_CLIENT_FLAGS\n\t%i not in [0, %i)",
                    flagNum,
                    16) )
    {
        __debugbreak();
    }
    if ( !cent
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    3971,
                    0,
                    "%s\n\t(cent) = %p",
                    "(cent != 0)",
                    0) )
    {
        __debugbreak();
    }
    return ((1 << flagNum) & cent->flagIndex) != 0;
}

bool __cdecl CG_ClientFlagIsSet(const centity_s *cent, unsigned int flagNum)
{
    if ( flagNum >= 0x10
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    3978,
                    0,
                    "flagNum doesn't index NUM_CLIENT_FLAGS\n\t%i not in [0, %i)",
                    flagNum,
                    16) )
    {
        __debugbreak();
    }
    if ( !cent
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_ents_mp.cpp",
                    3979,
                    0,
                    "%s\n\t(cent) = %p",
                    "(cent != 0)",
                    0) )
    {
        __debugbreak();
    }
    return ((1 << flagNum) & cent->flagState) != 0;
}

char *__cdecl CG_AllocAnimTree(int size)
{
    return MT_Alloc(size, 5, SCRIPTINSTANCE_SERVER);
}

