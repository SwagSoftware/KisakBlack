#include "cg_draw_names.h"

#include <cstring>
#include <client_mp/cl_ui_mp.h>
#include <cgame_mp/cg_local_mp.h>
#include "cg_main.h"
#include <cgame_mp/cg_ents_mp.h>
#include <cgame_mp/cg_main_mp.h>
#include <demo/demo_playback.h>
#include <clientscript/scr_const.h>
#include "cg_drawtools.h"
#include <gfx_d3d/r_font.h>
#include <client/cl_rank.h>
#include <client_mp/cl_cgame_mp.h>
#include <universal/com_math_anglevectors.h>
#include <qcommon/dobj_management.h>
#include <cgame_mp/cg_players_mp.h>
#include <gfx_d3d/r_dpvs.h>
#include "cg_world.h"
#include <bgame/bg_weapons.h>
#include <bgame/bg_weapons_def.h>
#include <EffectsCore/fx_system.h>
#include <win32/win_shared.h>

OverheadFade overheadFade[32];
DrawNameEntity drawNameEntities[32];
unsigned int numDrawNameEntities;
PlayerDetails playerDetails[1][32];

void __cdecl CG_ClearOverheadFade()
{
    memset(overheadFade, 0, sizeof(overheadFade));
}

void __cdecl CG_ClearPlayerDetails()
{
    memset(playerDetails, 0, sizeof(playerDetails));
}

void __cdecl CG_InitPlayerDetails(int localClientNum, int targetClientNum)
{
    PlayerDetails *currentPlayerDetails; // [esp+30h] [ebp-4h]

    currentPlayerDetails = &playerDetails[localClientNum][targetClientNum];
    if ( CL_GetClientName(localClientNum, targetClientNum, currentPlayerDetails->gamerTag, 32, 0)
        && (currentPlayerDetails->gamerTagDisplayLength = strlen(currentPlayerDetails->gamerTag),
                CL_GetClientName(localClientNum, targetClientNum, currentPlayerDetails->fullDisplayName, 38, 1))
        && (currentPlayerDetails->fullDisplayNameLength = strlen(currentPlayerDetails->fullDisplayName),
                CL_GetClientClanName(localClientNum, targetClientNum, currentPlayerDetails->clanDisplayName, 7)) )
    {
        currentPlayerDetails->clanDisplayLength = strlen(currentPlayerDetails->clanDisplayName);
    }
    else
    {
        Com_PrintError(14, "Unable to get name for client num: %i\n", targetClientNum);
    }
}

void __cdecl CG_DrawOverheadNames(int localClientNum, const centity_s *cent, float alpha, int mode)
{
    int v4; // eax
    int v5; // eax
    int v6; // eax
    int v7; // eax
    float v8; // [esp+38h] [ebp-F0h]
    float v9; // [esp+3Ch] [ebp-ECh]
    float v10; // [esp+40h] [ebp-E8h]
    float v11; // [esp+44h] [ebp-E4h]
    float v12; // [esp+4Ch] [ebp-DCh]
    float v13; // [esp+4Ch] [ebp-DCh]
    float v14; // [esp+54h] [ebp-D4h]
    float v15; // [esp+60h] [ebp-C8h]
    float v16; // [esp+60h] [ebp-C8h]
    float v17; // [esp+68h] [ebp-C0h]
    float v18; // [esp+68h] [ebp-C0h]
    float iconSize; // [esp+80h] [ebp-A8h]
    float newscale; // [esp+84h] [ebp-A4h]
    float aspect; // [esp+88h] [ebp-A0h]
    float distFrac; // [esp+90h] [ebp-98h]
    int prestige; // [esp+94h] [ebp-94h]
    Font_s *font; // [esp+98h] [ebp-90h]
    float nameX; // [esp+9Ch] [ebp-8Ch] BYREF
    float origin[3]; // [esp+A0h] [ebp-88h] BYREF
    bool drawRank; // [esp+AFh] [ebp-79h]
    ScreenPlacement *scrPlace; // [esp+B0h] [ebp-78h]
    const cg_s *cgameGlob; // [esp+B4h] [ebp-74h]
    Material *material; // [esp+B8h] [ebp-70h] BYREF
    int rank; // [esp+BCh] [ebp-6Ch]
    float distanceScale; // [esp+C0h] [ebp-68h]
    float viewPos[3]; // [esp+C4h] [ebp-64h] BYREF
    float textSize; // [esp+D0h] [ebp-58h]
    float distance; // [esp+D4h] [ebp-54h]
    const char *rankText; // [esp+D8h] [ebp-50h]
    const PlayerDetails *currentPlayerDetails; // [esp+DCh] [ebp-4Ch]
    float scaleY; // [esp+E0h] [ebp-48h] BYREF
    int entnum; // [esp+E4h] [ebp-44h]
    float glow[4]; // [esp+E8h] [ebp-40h] BYREF
    float scale; // [esp+F8h] [ebp-30h]
    const playerState_s *ps; // [esp+FCh] [ebp-2Ch]
    float color[4]; // [esp+100h] [ebp-28h] BYREF
    float x; // [esp+110h] [ebp-18h] BYREF
    float y; // [esp+114h] [ebp-14h] BYREF
    const char *text; // [esp+118h] [ebp-10h]
    float scaleX; // [esp+11Ch] [ebp-Ch] BYREF
    float distanceSq; // [esp+120h] [ebp-8h]
    float w; // [esp+124h] [ebp-4h]

    material = 0;
    drawRank = 1;
    text = "";
    if ( alpha > 0.001 )
    {
        entnum = cent->nextState.number;
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        scrPlace = &scrPlaceView[localClientNum];
        if ( cent->pose.eType != 14
            || !CG_IsVehicleRemoteControl(cent->nextState.vehicleState.vehicleInfoIndex)
            && (entnum = CG_GetDriverClientNumFromVehicle(localClientNum, cent), entnum < com_maxclients->current.integer) )
        {
            if ( (cent->nextState.lerp.eFlags & 0x40000) == 0 )
            {
                currentPlayerDetails = &playerDetails[localClientNum][entnum];
                ps = &cgameGlob->nextSnap->ps;
                font = UI_GetFontHandle(0, cg_overheadNamesFont->current.integer, 1.0);
                CG_InitPlayerDetails(localClientNum, entnum);
                if ( cgameGlob->clientNum != entnum
                    || (cgameGlob->predictedPlayerState.eFlags2 & 0x10000000) != 0
                    || Demo_IsMovieCamera() )
                {
                    if ( cent->nextState.eType == 14 )
                    {
                        CG_CalcVehicleNamePosition(localClientNum, cent, (float (*)[3])origin);
                    }
                    else if ( CachedTag_GetTagPos(
                                            cent,
                                            scr_const.j_head,
                                            origin,
                                            cg_overheadNamesTagUpdateInterval->current.integer,
                                            0) <= 0 )
                    {
                        origin[0] = cent->pose.origin[0];
                        origin[1] = cent->pose.origin[1];
                        origin[2] = cent->pose.origin[2];
                        origin[2] = origin[2] + 82.0;
                    }
                    else
                    {
                        origin[2] = origin[2] + 10.0;
                    }
                    switch ( mode )
                    {
                        case 0:
                            drawRank = 0;
                            text = currentPlayerDetails->gamerTag;
                            break;
                        case 1:
                            text = currentPlayerDetails->clanDisplayName;
                            break;
                        case 2:
                            text = (const char *)currentPlayerDetails;
                            break;
                        case 5:
                            text = "";
                            break;
                        default:
                            Com_PrintError(14, "Bad Overhead Display Mode for client num: %i\n", entnum);
                            break;
                    }
                    CG_RelativeTeamColor(entnum, color, localClientNum);
                    color[3] = alpha;
                    if ( CG_CalcNamePosition(localClientNum, origin, &nameX, &y) )
                    {
                        x = nameX;
                        viewPos[0] = cgameGlob->refdef.vieworg[0];
                        viewPos[1] = cgameGlob->refdef.vieworg[1];
                        viewPos[2] = cgameGlob->refdef.vieworg[2];
                        distanceSq = Vec3DistanceSq(viewPos, origin);
                        distance = sqrtf(distanceSq);
                        if ( (float)(cg_overheadNamesNearDist->current.value * cg_overheadNamesNearDist->current.value) <= distanceSq )
                        {
                            if ( distanceSq <= (float)(cg_overheadNamesFarDist->current.value * cg_overheadNamesFarDist->current.value) )
                            {
                                distFrac = (float)(distance - cg_overheadNamesNearDist->current.value)
                                                 / (float)(cg_overheadNamesFarDist->current.value - cg_overheadNamesNearDist->current.value);
                                distanceScale = (float)(distFrac * cg_overheadNamesFarScale->current.value) + (float)(1.0 - distFrac);
                            }
                            else
                            {
                                distanceScale = cg_overheadNamesFarScale->current.value;
                            }
                        }
                        else
                        {
                            distanceScale = 1.0f;
                        }
                        scale = R_NormalizedTextScale(font, cg_overheadNamesSize->current.value * distanceScale);
                        v17 = scale;
                        v18 = v17 / ScrPlace_HiResGetScale();
                        v15 = v18 * scrPlace->scaleRealToVirtual[1];
                        v16 = ScrPlace_HiResGetScale() * v15;
                        scaleX = v16 * scrPlace->scaleVirtualToReal[0];
                        scaleY = scale;
                        v4 = R_TextHeight(font);
                        textSize = (float)v4 * scale;
                        v11 = 0.5 * scaleX;
                        v5 = R_TextWidth(text, 32, font);
                        x = x - (float)(v11 * (float)v5);
                        v10 = floor((float)(x + 0.5));
                        x = v10;
                        v9 = floor((float)(y + 0.5));
                        y = v9;
                        Dvar_GetUnpackedColor(cg_overheadNamesGlow, glow);
                        glow[3] = glow[3] * alpha;
                        w = distance;
                        ScrPlace_HiResApplyRect(&x, &y, &scaleX, &scaleY);
                        CL_DrawTextPhysicalWithEffects(
                            (char *)text,
                            32,
                            font,
                            x,
                            y,
                            w,
                            scaleX,
                            scaleY,
                            color,
                            3,
                            glow,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0);
                        if ( drawRank )
                        {
                            if ( cl_playerRank->current.integer < 0 )
                                rank = cgameGlob->bgs.clientinfo[entnum].rank;
                            else
                                rank = cl_playerRank->current.integer;
                            if ( cl_playerPrestige->current.integer < 0 )
                                prestige = cgameGlob->bgs.clientinfo[entnum].prestige;
                            else
                                prestige = cl_playerPrestige->current.integer;
                            if ( !zombiemode->current.enabled )
                                CL_GetRankIcon(rank, prestige, &material);
                            if ( material )
                            {
                                iconSize = cg_overheadIconSize->current.value * textSize;
                                newscale = R_NormalizedTextScale(font, cg_overheadRankSize->current.value * distanceScale);
                                v14 = newscale / ScrPlace_HiResGetScale();
                                v12 = v14 * scrPlace->scaleRealToVirtual[1];
                                v13 = ScrPlace_HiResGetScale() * v12;
                                scaleX = v13 * scrPlace->scaleVirtualToReal[0];
                                aspect = scaleX / newscale;
                                rankText = CL_GetRankData(rank, MP_RANKTABLE_DISPLAYLEVEL);
                                v8 = (float)(2.0 * distanceScale) + iconSize;
                                v6 = R_TextWidth(rankText, 32, font);
                                x = x - (float)((float)(v8 + (float)((float)v6 * newscale)) * aspect);
                                color[0] = 1.0f;
                                color[1] = 1.0f;
                                color[2] = 1.0f;
                                color[3] = alpha;
                                CL_DrawStretchPicPhysicalW(
                                    x,
                                    y - (float)((float)(iconSize + textSize) * 0.5),
                                    w,
                                    iconSize * aspect,
                                    iconSize,
                                    0.0,
                                    0.0,
                                    1.0,
                                    1.0,
                                    color,
                                    material);
                                v7 = R_TextHeight(font);
                                x = (float)(iconSize * aspect) + x;
                                CL_DrawTextPhysical(
                                    (char *)rankText,
                                    3,
                                    font,
                                    x,
                                    (float)(0.25 * (float)((float)v7 * scale)) + y,
                                    w,
                                    newscale * aspect,
                                    newscale,
                                    color,
                                    3);
                            }
                        }
                    }
                }
            }
        }
    }
}

char __cdecl CG_CalcNamePosition(int localClientNum, float *origin, float *xOut, float *yOut)
{
    ScreenPlacement *scrPlace; // [esp+0h] [ebp-24h]
    float projections[3]; // [esp+8h] [ebp-1Ch] BYREF
    const refdef_s *refdef; // [esp+14h] [ebp-10h]
    float x; // [esp+18h] [ebp-Ch]
    float y; // [esp+1Ch] [ebp-8h]
    float w; // [esp+20h] [ebp-4h]

    refdef = &CG_GetLocalClientGlobals(localClientNum)->refdef;
    scrPlace = &scrPlaceView[localClientNum];
    CG_GetViewAxisProjections(refdef, origin, projections);
    w = projections[0];
    if ( projections[0] < 0.0 )
        return 0;
    x = projections[1] / refdef->tanHalfFovX;
    *xOut = (float)((float)(0.5 * scrPlace->realViewportSize[0]) * (float)(1.0 - (float)(x / w)))
                + scrPlace->realViewportBase[0];
    y = projections[2] / refdef->tanHalfFovY;
    *yOut = (float)((float)(0.5 * scrPlace->realViewportSize[1]) * (float)(1.0 - (float)(y / w)))
                + scrPlace->realViewportBase[1];
    return 1;
}

void __cdecl CG_GetViewAxisProjections(const refdef_s *refdef, const float *worldPoint, float *projections)
{
    float eyeDelta; // [esp+8h] [ebp-10h]
    float eyeDelta_4; // [esp+Ch] [ebp-Ch]
    float eyeDelta_8; // [esp+10h] [ebp-8h]
    int i; // [esp+14h] [ebp-4h]

    eyeDelta = *worldPoint - refdef->vieworg[0];
    eyeDelta_4 = worldPoint[1] - refdef->vieworg[1];
    eyeDelta_8 = worldPoint[2] - refdef->vieworg[2];
    for ( i = 0; i < 3; ++i )
        projections[i] = (float)((float)(eyeDelta * refdef->viewaxis[i][0]) + (float)(eyeDelta_4 * refdef->viewaxis[i][1]))
                                     + (float)(eyeDelta_8 * refdef->viewaxis[i][2]);
}

void __cdecl CG_CalcVehicleNamePosition(int localClientNum, const centity_s *cent, float (*origin)[3])
{
    float axis[3][3]; // [esp+Ch] [ebp-24h] BYREF

    (*origin)[0] = cent->pose.origin[0];
    (*origin)[1] = cent->pose.origin[1];
    (*origin)[2] = cent->pose.origin[2];
    AnglesToAxis(cent->pose.angles, axis);
    (*origin)[0] = (float)(-80.0 * axis[0][0]) + (*origin)[0];
    (*origin)[1] = (float)(-80.0 * axis[0][1]) + (*origin)[1];
    (*origin)[2] = (float)(-80.0 * axis[0][2]) + (*origin)[2];
    (*origin)[0] = (float)(0.0 * axis[1][0]) + (*origin)[0];
    (*origin)[1] = (float)(0.0 * axis[1][1]) + (*origin)[1];
    (*origin)[2] = (float)(0.0 * axis[1][2]) + (*origin)[2];
    (*origin)[0] = (float)(75.0 * axis[2][0]) + (*origin)[0];
    (*origin)[1] = (float)(75.0 * axis[2][1]) + (*origin)[1];
    (*origin)[2] = (float)(75.0 * axis[2][2]) + (*origin)[2];
}

double __cdecl CG_FadeCrosshairNameAlpha(int time, int startMsec, int lastMsec, int fadeInMsec, int fadeOutMsec)
{
    int timeSinceLastOver; // [esp+8h] [ebp-4h]

    timeSinceLastOver = time - lastMsec;
    if ( time - lastMsec >= fadeOutMsec )
        return 0.0;
    if ( lastMsec - startMsec >= fadeInMsec )
    {
        if ( timeSinceLastOver >= fadeOutMsec )
            return 1.0;
        else
            return (double)(fadeOutMsec - timeSinceLastOver) * 1.0 / (double)fadeOutMsec;
    }
    else if ( fadeInMsec )
    {
        return (float)((float)((float)(lastMsec - startMsec) * 1.0) / (float)fadeInMsec);
    }
    else
    {
        return 1.0f;
    }
}

char __cdecl CG_CanSeeFriendlyHead(int localClientNum, const centity_s *cent)
{
    snapshot_s *nextSnap; // [esp+Ch] [ebp-D8h]
    float targetDir; // [esp+40h] [ebp-A4h]
    float targetDir_4; // [esp+44h] [ebp-A0h]
    float targetDir_8; // [esp+48h] [ebp-9Ch]
    DObj *obj; // [esp+4Ch] [ebp-98h]
    const cg_s *cgameGlob; // [esp+54h] [ebp-90h]
    float contactEnd[3]; // [esp+58h] [ebp-8Ch] BYREF
    float start[3]; // [esp+64h] [ebp-80h] BYREF
    float end[3]; // [esp+70h] [ebp-74h] BYREF
    float eyeDelta[3]; // [esp+7Ch] [ebp-68h]
    trace_t trace; // [esp+88h] [ebp-5Ch] BYREF
    bool do_frustum_check; // [esp+C7h] [ebp-1Dh]
    const playerState_s *ps; // [esp+C8h] [ebp-1Ch]
    int clientNum; // [esp+CCh] [ebp-18h]
    float playerDir[3]; // [esp+D0h] [ebp-14h]
    unsigned __int16 hitEntId; // [esp+DCh] [ebp-8h]
    float dot; // [esp+E0h] [ebp-4h]

    if ( cg_drawThroughWalls->current.enabled )
        return 1;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->nextSnap->ps;
    obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
    if ( cent->nextState.eType != 1
        || !obj
        || !CachedTag_GetTagPos(cent, scr_const.j_head, end, cg_canSeeFriendlyFrustumUpdateInterval->current.integer, 0) )
    {
        end[0] = cent->pose.origin[0];
        end[1] = cent->pose.origin[1];
        end[2] = cent->pose.origin[2];
        end[2] = end[2] + 10.0;
    }
    start[0] = cgameGlob->refdef.vieworg[0];
    start[1] = cgameGlob->refdef.vieworg[1];
    start[2] = cgameGlob->refdef.vieworg[2];
    targetDir = end[0] - cgameGlob->refdef.vieworg[0];
    targetDir_4 = end[1] - cgameGlob->refdef.vieworg[1];
    targetDir_8 = end[2] - cgameGlob->refdef.vieworg[2];
    playerDir[0] = cgameGlob->refdef.viewaxis[0][0];
    playerDir[1] = cgameGlob->refdef.viewaxis[0][1];
    playerDir[2] = cgameGlob->refdef.viewaxis[0][2];
    dot = (float)((float)((float)(playerDir[0] * targetDir) + (float)(playerDir[1] * targetDir_4))
                            + (float)(playerDir[2] * targetDir_8))
            + 15.0;
    if ( dot < 0.0 )
        return 0;
    do_frustum_check = Vec3DistanceSq(cgameGlob->predictedPlayerState.origin, end) > cg_canSeeFriendlyFrustumMinDistance->current.value;
    if ( do_frustum_check )
        CG_PlayerInViewFrustum(localClientNum, cent);
    if ( do_frustum_check && R_CullPoint(cgameGlob->localClientNum, end, cg_canSeeFriendlyFrustumExpand->current.value) )
        return 0;
    eyeDelta[0] = end[0] - cgameGlob->refdef.vieworg[0];
    eyeDelta[1] = end[1] - cgameGlob->refdef.vieworg[1];
    eyeDelta[2] = end[2] - cgameGlob->refdef.vieworg[2];
    if ( (float)((float)((float)(eyeDelta[0] * cgameGlob->refdef.viewaxis[0][0])
                                         + (float)(eyeDelta[1] * cgameGlob->refdef.viewaxis[0][1]))
                         + (float)(eyeDelta[2] * cgameGlob->refdef.viewaxis[0][2])) < 0.0 )
        return 0;
    if ( Vec3DistanceSq(start, end) > cg_overheadNamesMaxDist->current.value * cg_overheadNamesMaxDist->current.value )
        return 0;
    if ( Demo_IsMovieCamera() )
        clientNum = -1;
    else
        clientNum = ps->clientNum;
    CG_LocationalTrace(&trace, start, end, clientNum, 0x280B001, 0, 0);
    nextSnap = cgameGlob->nextSnap;
    if ( (nextSnap->ps.otherFlags & 6) != 0 && cent->nextState.number == nextSnap->ps.clientNum )
    {
        if ( trace.fraction < 1.0 )
            return 0;
    }
    else
    {
        hitEntId = Trace_GetEntityHitId(&trace);
        if ( !CG_TraceEntIsEntity(hitEntId, cent) )
            return 0;
    }
    Vec3Lerp(start, end, trace.fraction, contactEnd);
    return 1;
}

bool __cdecl CG_TraceEntIsEntity(int traceEnt, const centity_s *cent)
{
    return traceEnt == cent->nextState.number;
}

void __cdecl CG_ScanForCrosshairEntity(int localClientNum)
{
    CG_ScanForCrosshairEntityInternal(localClientNum);
}

void __cdecl CG_ScanForCrosshairEntityInternal(int localClientNum)
{
    float enemyCrosshairRange; // [esp+78h] [ebp-D8h]
    int weaponIndex; // [esp+7Ch] [ebp-D4h]
    IgnoreEntParams ignoreEntParams; // [esp+84h] [ebp-CCh] BYREF
    col_context_t context; // [esp+90h] [ebp-C0h] BYREF
    const clientInfo_t *ci; // [esp+B8h] [ebp-98h]
    float diff[3]; // [esp+BCh] [ebp-94h]
    float fCheckDist; // [esp+C8h] [ebp-88h]
    int fadeOutTime; // [esp+D0h] [ebp-80h]
    team_t team; // [esp+D4h] [ebp-7Ch]
    float contactEnd[3]; // [esp+D8h] [ebp-78h] BYREF
    cg_s *cgameGlob; // [esp+E4h] [ebp-6Ch]
    bool owner; // [esp+EBh] [ebp-65h]
    float start[3]; // [esp+ECh] [ebp-64h] BYREF
    float end[3]; // [esp+F8h] [ebp-58h] BYREF
    team_t hitEntTeam = TEAM_FREE; // [esp+104h] [ebp-4Ch]
    centity_s *hitEnt; // [esp+108h] [ebp-48h]
    trace_t trace; // [esp+10Ch] [ebp-44h] BYREF
    const WeaponDef *weapDef; // [esp+148h] [ebp-8h]
    unsigned __int16 hitEntId; // [esp+14Ch] [ebp-4h]

    PROF_SCOPED("CG_ScanForCrosshairEntity");

    fadeOutTime = cg_friendlyNameFadeOut->current.integer;
    owner = 0;
    fCheckDist = 8192.0f;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_names.cpp",
                    716,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    cgameGlob->predictedPlayerState.weapFlags &= 0xFFFFFFE7;
    if ( !CG_Flashbanged(localClientNum) )
    {
        if ( CG_Flared(localClientNum) )
        {
            return;
        }
        start[0] = cgameGlob->refdef.vieworg[0];
        start[1] = cgameGlob->refdef.vieworg[1];
        start[2] = cgameGlob->refdef.vieworg[2];
        end[0] = (float)(fCheckDist * cgameGlob->refdef.viewaxis[0][0]) + start[0];
        end[1] = (float)(fCheckDist * cgameGlob->refdef.viewaxis[0][1]) + start[1];
        end[2] = (float)(fCheckDist * cgameGlob->refdef.viewaxis[0][2]) + start[2];
        ignoreEntParams.baseEntity = cgameGlob->nextSnap->ps.clientNum;
        ignoreEntParams.ignoreSelf = 1;
        ignoreEntParams.ignoreChildren = 0;
        ignoreEntParams.ignoreSiblings = 0;
        ignoreEntParams.parentEntity = -1;
        ignoreEntParams.ignoreParent = 0;
        ci = &cgameGlob->bgs.clientinfo[cgameGlob->predictedPlayerState.clientNum];
        if ( ci->attachedVehEntNum != 1023 )
        {
            ignoreEntParams.parentEntity = ci->attachedVehEntNum;
            ignoreEntParams.ignoreParent = 1;
            if ( !ci->attachedVehSeat
                && cgameGlob->predictedPlayerState.fWeaponPosFrac != 1.0
                && (cgameGlob->predictedPlayerState.eFlags2 & 0x10000000) == 0 )
            {
                Vec3Lerp(start, end, 0.05f, start);
            }
        }
        ////col_context_t::col_context_t(&context);
        context.ignoreEntParams = &ignoreEntParams;
        CG_LocationalTrace(&trace, start, end, cgameGlob->nextSnap->ps.clientNum, 0x280B001, 0, &context);
        if ( trace.fraction >= 1.0 )
        {
            hitEntId = 1023;
        }
        else
        {
            hitEntId = Trace_GetEntityHitId(&trace);
            Vec3Lerp(start, end, trace.fraction, contactEnd);
        }
        hitEnt = CG_GetEntity(localClientNum, hitEntId);
        if ( hitEnt->pose.eType == 14 && !CG_CheckIfDrivingRemoteControlVehicle(localClientNum, hitEntId) )
            hitEntId = CG_GetDriverClientNumFromVehicle(localClientNum, hitEnt);
        if ( hitEntId >= com_maxclients->current.integer
            && hitEnt->nextState.eType != 17
            && !CG_CheckIfDrivingRemoteControlVehicle(localClientNum, hitEntId) )
        {
            return;
        }
        if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_names.cpp",
                        789,
                        0,
                        "cgameGlob->nextSnap->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        cgameGlob->nextSnap->ps.clientNum,
                        32) )
        {
            __debugbreak();
        }
        team = cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].team;
        if ( team != TEAM_SPECTATOR )
        {
            if ( hitEnt->nextState.eType == 17 )
            {
                hitEntTeam = (team_t)hitEnt->nextState.lerp.u.actor.team;
                if ( cgameGlob->nextSnap->ps.clientNum == (int)hitEnt->nextState.faction.iHeadIconTeam >> 2 )
                    owner = 1;
            }
            else if ( hitEnt->nextState.eType == 14 || hitEnt->nextState.eType == 15 || hitEnt->nextState.eType == 16 )
            {
                hitEntTeam = (team_t)(hitEnt->nextState.faction.iHeadIconTeam & 3);
            }
            else if ( hitEntId < com_maxclients->current.integer )
            {
                hitEntTeam = cgameGlob->bgs.clientinfo[hitEntId].team;
            }
            if ( hitEntTeam == team && team || owner )
            {
                cgameGlob->predictedPlayerState.weapFlags |= 8u;
            }
            else
            {
                if ( (hitEnt->nextState.lerp.eFlags & 0x40000) != 0 )
                {
                    return;
                }
                if ( (cgameGlob->bgs.clientinfo[hitEntId].perks[1] & 4) != 0 )
                {
                    return;
                }
                diff[0] = hitEnt->pose.origin[0] - start[0];
                diff[1] = hitEnt->pose.origin[1] - start[1];
                diff[2] = hitEnt->pose.origin[2] - start[2];
                weaponIndex = CG_GetPlayerWeapon(&cgameGlob->predictedPlayerState, localClientNum);
                weapDef = BG_GetWeaponDef(weaponIndex);
                if ( (cgameGlob->predictedPlayerState.weapFlags & 0x400000) != 0 )
                    enemyCrosshairRange = 600.0f;
                else
                    enemyCrosshairRange = weapDef->enemyCrosshairRange;
                if ( (float)((float)((float)(diff[0] * diff[0]) + (float)(diff[1] * diff[1])) + (float)(diff[2] * diff[2])) > (float)(enemyCrosshairRange * enemyCrosshairRange) )
                {
                    return;
                }
                if ( !FX_ClientVisibilityTest(localClientNum, start, contactEnd) )
                {
                    return;
                }
                cgameGlob->predictedPlayerState.weapFlags |= 0x10u;
                fadeOutTime = cg_enemyNameFadeOut->current.integer;
            }
        }
        if ( FX_ClientVisibilityTest(localClientNum, start, contactEnd) )
        {
            if ( cgameGlob->crosshairClientNum != hitEntId
                || cgameGlob->time - cgameGlob->crosshairClientLastTime > fadeOutTime )
            {
                cgameGlob->crosshairClientNum = hitEntId;
                cgameGlob->crosshairClientStartTime = cgameGlob->time;
            }
            cgameGlob->crosshairClientLastTime = cgameGlob->time;
        }
        return;
    }
}

bool __cdecl CG_CheckIfDrivingRemoteControlVehicle(int localClientNum, unsigned __int16 EntId)
{
    bool result; // eax
    centity_s *ParentEnt; // [esp+10h] [ebp-10h]
    unsigned __int16 ParentID; // [esp+18h] [ebp-8h]
    centity_s *ent; // [esp+1Ch] [ebp-4h]

    ent = CG_GetEntity(localClientNum, EntId);
    CG_GetLocalClientGlobals(localClientNum);
    result = 0;
    if ( ent->pose.eType == 14 )
    {
        ParentID = CG_GetDriverClientNumFromVehicle(localClientNum, ent);
        if ( ParentID < com_maxclients->current.integer )
        {
            ParentEnt = CG_GetEntity(localClientNum, ParentID);
            if ( ((*((unsigned int *)ParentEnt + 201) >> 1) & 1) != 0 && (ParentEnt->nextState.lerp.eFlags2 & 0x10000000) != 0 )
                return 1;
        }
    }
    return result;
}

void __cdecl CG_DrawNames(int localClientNum)
{
    const centity_s *cent; // [esp+14h] [ebp-Ch]
    float alpha; // [esp+18h] [ebp-8h]
    int i; // [esp+1Ch] [ebp-4h]

    if ( (int)numDrawNameEntities > 0 )
    {
        qsort(drawNameEntities, numDrawNameEntities, 0x10u, (int (__cdecl *)(const void *, const void *))compareEntityDist);
        for ( i = 0; i < (int)numDrawNameEntities; ++i )
        {
            cent = CG_GetEntity(localClientNum, drawNameEntities[i].entnum);
            alpha = drawNameEntities[i].alpha;
            switch ( drawNameEntities[i].teamIndicator )
            {
                case 0:
                case 3:
                    CG_DrawOverheadNames(localClientNum, cent, alpha, 2);
                    break;
                case 1:
                    CG_DrawOverheadNames(localClientNum, cent, alpha, 0);
                    break;
                case 2:
                    CG_DrawOverheadNames(localClientNum, cent, alpha, 5);
                    break;
                default:
                    Com_PrintError(14, "Bad Overhead Team Indicator option for client num: %i\n", cent->nextState.clientNum);
                    break;
            }
        }
        numDrawNameEntities = 0;
    }
}

int __cdecl compareEntityDist(float *v1, float *v2)
{
    if ( v2[3] <= v1[3] )
        return -1;
    else
        return 1;
}

int __cdecl CG_GetTeamIndicator()
{
    return team_indicator->current.integer;
}

bool __cdecl ShouldDrawCrosshairNames(const cg_s *cgameGlob)
{
    if ( !cg_drawCrosshairNames->current.enabled )
        return 0;
    if ( Demo_IsGameHudHidden() )
        return 0;
    if ( cgameGlob->crosshairClientNum > com_maxclients->current.integer )
        return 0;
    if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_names.cpp",
                    1174,
                    0,
                    "cgameGlob->nextSnap->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cgameGlob->nextSnap->ps.clientNum,
                    32) )
    {
        __debugbreak();
    }
    if ( !cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].infoValid )
        return 0;
    if ( cgameGlob->renderingThirdPerson == TP_FOR_MODEL && (cgameGlob->predictedPlayerState.eFlags2 & 0x10000000) == 0 )
        return 0;
    if ( (cgameGlob->predictedPlayerState.eFlags & 0x4000) != 0 && cgameGlob->predictedPlayerState.vehicleType == 6 )
        return 0;
    if ( cgameGlob->crosshairClientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_names.cpp",
                    1191,
                    0,
                    "cgameGlob->crosshairClientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cgameGlob->crosshairClientNum,
                    32) )
    {
        __debugbreak();
    }
    return cgameGlob->bgs.clientinfo[cgameGlob->crosshairClientNum].infoValid != 0;
}

void __cdecl CG_DrawCrosshairNames(int localClientNum)
{
    int teamIndicator; // [esp+10h] [ebp-28h]
    int entityIndex; // [esp+14h] [ebp-24h]
    int inViewLast; // [esp+18h] [ebp-20h]
    const cg_s *cgameGlob; // [esp+1Ch] [ebp-1Ch]
    int myTeam; // [esp+20h] [ebp-18h]
    const snapshot_s *nextSnap; // [esp+24h] [ebp-14h]
    const centity_s *cent; // [esp+28h] [ebp-10h]
    int inViewStart; // [esp+2Ch] [ebp-Ch]
    int entnum; // [esp+30h] [ebp-8h]
    float alpha; // [esp+34h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( ShouldDrawCrosshairNames(cgameGlob) )
    {
        inViewStart = cgameGlob->crosshairClientStartTime;
        inViewLast = cgameGlob->crosshairClientLastTime;
        nextSnap = cgameGlob->nextSnap;
        for ( entityIndex = 0; entityIndex < nextSnap->numEntities; ++entityIndex )
        {
            entnum = nextSnap->entities[entityIndex].number;
            cent = CG_GetEntity(localClientNum, entnum);
            if ( cent->nextState.eType == 14 )
            {
                entnum = CG_GetDriverClientNumFromVehicle(localClientNum, cent);
                if ( entnum >= com_maxclients->current.integer )
                    continue;
            }
            if ( CG_IsValidCrosshairEntity(localClientNum, cgameGlob, cent, entnum) )
            {
                myTeam = cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].team;
                //BLOPS_NULLSUB();
                if ( myTeam == 3 || myTeam && myTeam == cgameGlob->bgs.clientinfo[cgameGlob->crosshairClientNum].team )
                    alpha = CG_FadeCrosshairNameAlpha(
                                        cgameGlob->time,
                                        inViewStart,
                                        inViewLast,
                                        cg_friendlyNameFadeIn->current.integer,
                                        cg_friendlyNameFadeOut->current.integer);
                else
                    alpha = CG_FadeCrosshairNameAlpha(
                                        cgameGlob->time,
                                        inViewStart,
                                        inViewLast,
                                        cg_enemyNameFadeIn->current.integer,
                                        cg_enemyNameFadeOut->current.integer);
                teamIndicator = CG_GetTeamIndicator();
                CG_AddDrawName(localClientNum, entnum, alpha, teamIndicator);
            }
        }
    }
}

void __cdecl CG_AddDrawName(int localClientNum, int entnum, float alpha, int teamIndicator)
{
    const cg_s *cgameGlob; // [esp+14h] [ebp-8h]
    const centity_s *cent; // [esp+18h] [ebp-4h]

    if ( alpha > 0.001 )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        cent = CG_GetEntity(localClientNum, entnum);
        if ( numDrawNameEntities >= 0x20
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_names.cpp",
                        1026,
                        0,
                        "%s",
                        "numDrawNameEntities < ARRAY_COUNT(drawNameEntities)") )
        {
            __debugbreak();
        }
        drawNameEntities[numDrawNameEntities].entnum = entnum;
        drawNameEntities[numDrawNameEntities].alpha = alpha;
        drawNameEntities[numDrawNameEntities].teamIndicator = teamIndicator;
        drawNameEntities[numDrawNameEntities++].dist = Vec3DistanceSq(cgameGlob->refdef.vieworg, cent->pose.origin);
    }
}

char __cdecl CG_IsValidCrosshairEntity(int localClientNum, const cg_s *cgameGlob, const centity_s *cent, int entNum)
{
    centity_s *vehCent; // [esp+Ch] [ebp-Ch]
    bool allowOccupant; // [esp+17h] [ebp-1h]

    if ( ((*((unsigned int *)cent + 201) >> 1) & 1) == 0 )
        return 0;
    if ( cent->nextState.eType != 1 && cent->nextState.eType != 14 )
        return 0;
    if ( (cent->nextState.lerp.eFlags & 0x20) != 0 && cent->nextState.eType != 14 )
        return 0;
    if ( cent->nextState.eType == 14 && CG_IsVehicleRemoteControl(cent->nextState.vehicleState.vehicleInfoIndex) )
        return 0;
    if ( entNum == cgameGlob->crosshairClientNum )
        return 1;
    allowOccupant = 0;
    if ( cgameGlob->bgs.clientinfo[entNum].infoValid && cgameGlob->bgs.clientinfo[entNum].attachedVehEntNum != 1023 )
    {
        vehCent = CG_GetEntity(localClientNum, cgameGlob->bgs.clientinfo[entNum].attachedVehEntNum);
        if ( vehCent )
            return CG_GetDriverClientNumFromVehicle(localClientNum, vehCent) == cgameGlob->crosshairClientNum;
    }
    return allowOccupant;
}

bool __cdecl ShouldDrawFriendlyName(int localClientNum, const centity_s *cent, int entnum, int team)
{
    bool v5; // [esp+0h] [ebp-Ch]
    snapshot_s *nextSnap; // [esp+4h] [ebp-8h]
    const cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( (cent->nextState.lerp.eFlags & 0x20) != 0 )
        return 0;
    if ( cent->nextState.eType != 1 )
        return 0;
    if ( !team )
        return 0;
    if ( team != 3 && team != cgameGlob->bgs.clientinfo[entnum].team )
        return 0;
    if ( (cgameGlob->nextSnap->ps.eFlags & 0x4000) != 0 && cgameGlob->nextSnap->ps.vehicleType == 6 )
        return 0;
    nextSnap = cgameGlob->nextSnap;
    v5 = (nextSnap->ps.otherFlags & 6) != 0 && entnum == nextSnap->ps.clientNum;
    return !v5 || cgameGlob->renderingThirdPerson;
}

int __cdecl GetVehicleDriverEntNum(const cg_s *cgameGlob, const centity_s *cent, int entnum)
{
    unsigned __int16 i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < com_maxclients->current.integer; ++i )
    {
        if ( cgameGlob->bgs.clientinfo[i].infoValid
            && cgameGlob->bgs.clientinfo[i].attachedVehEntNum == cent->nextState.number
            && !cgameGlob->bgs.clientinfo[i].attachedVehSeat )
        {
            return i;
        }
    }
    return entnum;
}

void __cdecl CG_DrawFriendlyNames(int localClientNum)
{
    int TeamIndicator; // eax
    bool v2; // [esp+8h] [ebp-38h]
    int entityIndex; // [esp+1Ch] [ebp-24h]
    int team; // [esp+20h] [ebp-20h]
    const cg_s *cgameGlob; // [esp+28h] [ebp-18h]
    const snapshot_s *nextSnap; // [esp+2Ch] [ebp-14h]
    const centity_s *cent; // [esp+30h] [ebp-10h]
    int entnum; // [esp+34h] [ebp-Ch]
    float alpha; // [esp+3Ch] [ebp-4h]

    if ( cg_drawFriendlyNames->current.enabled && !Demo_IsGameHudHidden() )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        team = cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].team;
        nextSnap = cgameGlob->nextSnap;
        v2 = CG_Flashbanged(localClientNum) || CG_Flared(localClientNum);
        for ( entityIndex = 0; entityIndex < nextSnap->numEntities; ++entityIndex )
        {
            entnum = nextSnap->entities[entityIndex].number;
            cent = CG_GetEntity(localClientNum, entnum);
            if ( cent->nextState.eType == 14 )
            {
                entnum = GetVehicleDriverEntNum(cgameGlob, cent, entnum);
                cent = CG_GetEntity(localClientNum, entnum);
            }
            if ( ShouldDrawFriendlyName(localClientNum, cent, entnum, team) )
            {
                if ( !v2 && CG_CanSeeFriendlyHead(localClientNum, cent) )
                {
                    if ( !overheadFade[entnum].visible )
                    {
                        overheadFade[entnum].visible = 1;
                        overheadFade[entnum].startTime = cgameGlob->time;
                    }
                }
                else
                {
                    overheadFade[entnum].visible = 0;
                }
                if ( overheadFade[entnum].visible )
                    overheadFade[entnum].lastTime = cgameGlob->time;
                alpha = CG_FadeCrosshairNameAlpha(
                                    cgameGlob->time,
                                    overheadFade[entnum].startTime,
                                    overheadFade[entnum].lastTime,
                                    cg_friendlyNameFadeIn->current.integer,
                                    cg_friendlyNameFadeOut->current.integer);
                TeamIndicator = CG_GetTeamIndicator();
                CG_AddDrawName(localClientNum, entnum, alpha, TeamIndicator);
            }
        }
    }
}

void __cdecl CG_DrawVisibleNames(int localClientNum)
{
    int TeamIndicator; // eax
    bool v2; // [esp+8h] [ebp-48h]
    bool v3; // [esp+Ch] [ebp-44h]
    snapshot_s *v4; // [esp+10h] [ebp-40h]
    unsigned __int16 i; // [esp+20h] [ebp-30h]
    bool noDrawOverride; // [esp+27h] [ebp-29h]
    int entityIndex; // [esp+28h] [ebp-28h]
    const cg_s *cgameGlob; // [esp+34h] [ebp-1Ch]
    const snapshot_s *nextSnap; // [esp+38h] [ebp-18h]
    const centity_s *cent; // [esp+3Ch] [ebp-14h]
    unsigned int timeNow; // [esp+40h] [ebp-10h]
    int entnum; // [esp+44h] [ebp-Ch]
    float alpha; // [esp+4Ch] [ebp-4h]

    if ( cg_drawFriendlyNames->current.enabled )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        nextSnap = cgameGlob->nextSnap;
        if ( Demo_IsPlaying() )
            timeNow = Sys_Milliseconds();
        else
            timeNow = cgameGlob->time;
        v3 = CG_Flashbanged(localClientNum) || CG_Flared(localClientNum);
        for ( entityIndex = 0; entityIndex < nextSnap->numEntities; ++entityIndex )
        {
            entnum = nextSnap->entities[entityIndex].number;
            cent = CG_GetEntity(localClientNum, entnum);
            noDrawOverride = 0;
            if ( cent->pose.eType == 14 )
            {
                for ( i = 0; i < com_maxclients->current.integer; ++i )
                {
                    if ( cgameGlob->bgs.clientinfo[i].infoValid
                        && cgameGlob->bgs.clientinfo[i].attachedVehEntNum == cent->nextState.number
                        && !cgameGlob->bgs.clientinfo[i].attachedVehSeat )
                    {
                        entnum = i;
                        noDrawOverride = 1;
                        break;
                    }
                }
            }
            if ( (cent->nextState.eType == 1 || cent->nextState.eType == 14)
                && (noDrawOverride || (cent->nextState.lerp.eFlags & 0x20) == 0) )
            {
                v4 = cgameGlob->nextSnap;
                v2 = (v4->ps.otherFlags & 6) != 0 && entnum == v4->ps.clientNum;
                if ( !v2 || cgameGlob->renderingThirdPerson )
                {
                    if ( !v3 && CG_CanSeeFriendlyHead(localClientNum, cent) )
                    {
                        if ( !overheadFade[entnum].visible )
                        {
                            overheadFade[entnum].visible = 1;
                            overheadFade[entnum].startTime = timeNow;
                        }
                    }
                    else
                    {
                        overheadFade[entnum].visible = 0;
                    }
                    if ( overheadFade[entnum].visible )
                        overheadFade[entnum].lastTime = timeNow;
                    alpha = CG_FadeCrosshairNameAlpha(
                                        timeNow,
                                        overheadFade[entnum].startTime,
                                        overheadFade[entnum].lastTime,
                                        cg_friendlyNameFadeIn->current.integer,
                                        cg_friendlyNameFadeOut->current.integer);
                    TeamIndicator = CG_GetTeamIndicator();
                    CG_AddDrawName(localClientNum, entnum, alpha, TeamIndicator);
                }
            }
        }
        CG_DrawNames(localClientNum);
    }
}

