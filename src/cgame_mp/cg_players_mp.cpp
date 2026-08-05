#include "cg_players_mp.h"
#include "cg_local_mp.h"
#include <client_mp/cl_cgame_mp.h>
#include "cg_main_mp.h"
#include <qcommon/dobj_management.h>
#include "cg_ents_mp.h"
#include <bgame/bg_vehicle_anim.h>
#include <gfx_d3d/r_shader_constant_set.h>
#include <clientscript/cscr_vm.h>
#include <cgame/cg_scr_main.h>
#include <EffectsCore/fx_system.h>
#include <clientscript/scr_const.h>
#include <client/splitscreen.h>
#include <xanim/dobj_utils.h>
#include <cgame/cg_weapon_options.h>
#include <clientscript/cscr_stringlist.h>
#include <gfx_d3d/r_scene.h>
#include <xanim/xmodel_utils.h>
#include <universal/com_math_anglevectors.h>
#include <sound/snd_public_async.h>
#include <EffectsCore/fx_marks.h>
#include "cg_actors_mp.h"
#include <client/client.h>
#include <bgame/bg_mantle.h>
#include <xanim/xmodel.h>
#include <cgame/cg_drawtools.h>
#include <EffectsCore/fx_sprite.h>

float fCrouchHeightPanzer = 105.0f;
float fCrouchHeightT34 = 109.0f;
float handOfs = -5.0f;
float handOfs_0 = -6.0f;
float proneFeetDist = -40.0f;
float proneFeetYawOffset = -10.0f;




void __cdecl CG_AddAllPlayerSpriteDrawSurfs(int localClientNum)
{
    int entityIndex; // [esp+8h] [ebp-14h]
    const centity_s *cent; // [esp+10h] [ebp-Ch]
    snapshot_s *nextSnap; // [esp+14h] [ebp-8h]

    nextSnap = CG_GetLocalClientGlobals(localClientNum)->nextSnap;
    for ( entityIndex = 0; entityIndex < nextSnap->numEntities; ++entityIndex )
    {
        cent = CG_GetEntity(localClientNum, nextSnap->entities[entityIndex].number);
        if ( cent->nextState.eType == 1 )
            CG_AddPlayerSpriteDrawSurfs(localClientNum, cent);
    }
}

void __cdecl CG_AddPlayerSpriteDrawSurfs(int localClientNum, const centity_s *cent)
{
    cg_s *cgameGlob; // [esp+Ch] [ebp-1Ch]
    int iClientTeam; // [esp+10h] [ebp-18h]
    int secondaryHeight; // [esp+14h] [ebp-14h]
    Material *hMaterial; // [esp+18h] [ebp-10h]
    int iTeam; // [esp+1Ch] [ebp-Ch]
    const playerState_s *ps; // [esp+20h] [ebp-8h]
    const char *pszIcon; // [esp+24h] [ebp-4h]
    int savedregs; // [esp+28h] [ebp+0h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cent->nextState.clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                    167,
                    0,
                    "cent->nextState.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cent->nextState.clientNum,
                    32) )
    {
        __debugbreak();
    }
    if ( cgameGlob->bgs.clientinfo[cent->nextState.clientNum].infoValid )
    {
        if ( cent->nextState.clientNum >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                        170,
                        0,
                        "cent->nextState.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        cent->nextState.clientNum,
                        32) )
        {
            __debugbreak();
        }
        iTeam = cgameGlob->bgs.clientinfo[cent->nextState.clientNum].team;
        ps = &cgameGlob->nextSnap->ps;
        if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                        174,
                        0,
                        "ps->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        cgameGlob->nextSnap->ps.clientNum,
                        32) )
        {
            __debugbreak();
        }
        if ( cgameGlob->bgs.clientinfo[ps->clientNum].infoValid )
        {
            if ( ps->clientNum >= 0x20u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                            177,
                            0,
                            "ps->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                            ps->clientNum,
                            32) )
            {
                __debugbreak();
            }
            iClientTeam = cgameGlob->bgs.clientinfo[ps->clientNum].team;
            secondaryHeight = 0;
            if ( cent->nextState.iHeadIcon
                && (!cent->nextState.faction.iHeadIconTeam
                 || iClientTeam == 3
                 || cent->nextState.faction.iHeadIconTeam == iClientTeam) )
            {
                pszIcon = CL_GetConfigString(cent->nextState.iHeadIcon + 3099);
                hMaterial = Material_RegisterHandle(pszIcon, 7);
                if ( hMaterial )
                {
                    CG_AddPlayerSpriteDrawSurf(
                        localClientNum,
                        cent,
                        hMaterial,
                        cg_scriptIconSize->current.value,
                        0,
                        cg_constantSizeHeadIcons->current.enabled);
                    secondaryHeight = (int)cg_scriptIconSize->current.value + 16;
                }
            }
            if ( cent->nextState.number == cgameGlob->clientNum
                && cgameGlob->inKillCam
                && cgameGlob->killCamLookAtEntity == cgameGlob->clientNum )
            {
                CG_AddPlayerSpriteDrawSurf(
                    localClientNum,
                    cent,
                    cgMedia.youInKillCamMaterial,
                    cg_youInKillCamSize->current.value,
                    secondaryHeight,
                    1);
            }
            else if ( (cent->nextState.lerp.eFlags & 0x80) != 0 )
            {
                CG_AddPlayerSpriteDrawSurf(
                    localClientNum,
                    cent,
                    cgMedia.connectionMaterial,
                    cg_connectionIconSize->current.value,
                    secondaryHeight,
                    0);
            }
            else if ( (iTeam == iClientTeam && iClientTeam || iClientTeam == 3)
                         && (cent->nextState.lerp.eFlags & 0x200000) != 0 )
            {
                CG_AddPlayerSpriteDrawSurf(
                    localClientNum,
                    cent,
                    cgMedia.afkLightbulb,
                    cg_voiceIconSize->current.value,
                    secondaryHeight,
                    0);
            }
        }
    }
}

#if 0
void    CG_AddPlayerSpriteDrawSurf(
                int localClientNum,
                const centity_s *cent,
                Material *material,
                float additionalRadiusSize,
                int height,
                bool fixedScreenSize)
{
    float v7; // xmm0_4
    unsigned int v8[3]; // [esp+18h] [ebp-5Ch] BYREF
    FxSprite sprite; // [esp+24h] [ebp-50h] BYREF
    float v10; // [esp+44h] [ebp-30h]
    float v11; // [esp+48h] [ebp-2Ch]
    float origin[3]; // [esp+4Ch] [ebp-28h]
    DObj *obj; // [esp+58h] [ebp-1Ch]
    float radius; // [esp+5Ch] [ebp-18h]
    int renderFxFlags; // [esp+60h] [ebp-14h]
    int v16; // [esp+68h] [ebp-Ch]
    const cg_s *cgameGlob; // [esp+6Ch] [ebp-8h]
    const cg_s *retaddr; // [esp+74h] [ebp+0h]

    v16 = a1;
    cgameGlob = retaddr;
    if ( additionalRadiusSize < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                    107,
                    0,
                    "%s\n\t(additionalRadiusSize) = %g",
                    "(additionalRadiusSize >= 0)",
                    additionalRadiusSize) )
    {
        __debugbreak();
    }
    if ( !cg_headIconMinScreenRadius
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                    108,
                    0,
                    "%s",
                    "cg_headIconMinScreenRadius") )
    {
        __debugbreak();
    }
    renderFxFlags = (int)CG_GetLocalClientGlobals(localClientNum);
    radius = *(float *)(renderFxFlags + 44);
    obj = (DObj *)((*(unsigned int *)(LODWORD(radius) + 36) & 6) != 0
                            && cent->nextState.number == *(unsigned __int8 *)(LODWORD(radius) + 320));
    if ( !(_BYTE)obj || *(unsigned int *)(renderFxFlags + 0x40498) )
    {
        if ( fixedScreenSize )
        {
            LODWORD(origin[2]) = 3;
            v7 = (float)(additionalRadiusSize + 5.0) * 0.0043000001;
        }
        else
        {
            origin[2] = 0.0;
            v7 = additionalRadiusSize + 5.0;
        }
        origin[1] = v7;
        LODWORD(origin[0]) = Com_GetClientDObj(cent->nextState.number, localClientNum);
        if ( LODWORD(origin[0])
            && CG_DObjGetWorldTagPos(&cent->pose, (DObj *)LODWORD(origin[0]), scr_const.j_head, (float *)&sprite.flags) )
        {
            v11 = (float)((float)height + 21.0) + v11;
        }
        else
        {
            LODWORD(sprite.minScreenRadius) = cent->pose.origin;
            sprite.flags = LODWORD(cent->pose.origin[0]);
            v10 = cent->pose.origin[1];
            v11 = cent->pose.origin[2];
            v11 = (float)((float)height + 82.0) + v11;
        }
        v8[1] = sprite.flags;
        *(float *)&v8[2] = v10;
        *(float *)&sprite.material = v11;
        sprite.pos[0] = NAN;
        v8[0] = material;
        sprite.pos[1] = origin[1];
        LODWORD(sprite.pos[2]) = cg_headIconMinScreenRadius->current.integer;
        *(float *)sprite.rgbaColor = origin[2];
        if ( !cg_headIconMinScreenRadius
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                        146,
                        0,
                        "%s",
                        "cg_headIconMinScreenRadius") )
        {
            __debugbreak();
        }
        FX_SpriteAdd((FxSprite *)v8);
    }
}
#endif

// aislop
void CG_AddPlayerSpriteDrawSurf(
    int localClientNum,
    const centity_s *cent,
    Material *material,
    float additionalRadiusSize,
    int height,
    bool fixedScreenSize
) {
    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    snapshot_s *nextSnap = cgameGlob->nextSnap;
    bool renderFxFlags = (nextSnap->ps.otherFlags & 6) != 0 && cent->nextState.number == nextSnap->ps.clientNum;

    if (!renderFxFlags || cgameGlob->renderingThirdPerson) {
        float radius = 0.0f;
        float scale = 1.0f;

        if (fixedScreenSize) {
            radius = 3.0f;
            scale = (additionalRadiusSize + 5.0f) * 0.0043f;
        }
        else {
            scale = additionalRadiusSize + 5.0f;
        }

        float origin[3];
        DObj *obj = Com_GetClientDObj(cent->nextState.number, localClientNum);

        Material *spriteMaterial = NULL;
        if (obj && CG_DObjGetWorldTagPos(&cent->pose, obj, scr_const.j_head, origin)) {
            origin[1] += height + 21.0f;
        }
        else {
            origin[0] = cent->pose.origin[0];
            origin[1] = cent->pose.origin[1] + height + 82.0f;
            origin[2] = cent->pose.origin[2];
        }

        FxSprite sprite;
        sprite.material = spriteMaterial;
        sprite.pos[0] = origin[0];
        sprite.pos[1] = origin[1];
        sprite.pos[2] = NAN; // original code had NaN for Z
        sprite.rgbaColor[0] = *(int *)&scale; // original LODWORD(obj) hack
        sprite.radius = cg_headIconMinScreenRadius->current.integer;
        sprite.minScreenRadius = radius;

        FX_SpriteAdd(&sprite);
    }
}


int vehAnimLerpTime = 600;

char __cdecl CG_Player_ApplyVehicleAnimOffsets(
                int localClientNum,
                const entityState_s *es,
                centity_s *cent,
                clientInfo_t *ci)
{
    float v5; // [esp+8h] [ebp-84h]
    int i; // [esp+34h] [ebp-58h]
    float lerpFrac; // [esp+38h] [ebp-54h]
    DObj *vehicleObj; // [esp+3Ch] [ebp-50h]
    int boneIndex; // [esp+40h] [ebp-4Ch]
    cg_s *cgameGlob; // [esp+44h] [ebp-48h]
    float tagOrigin[3]; // [esp+48h] [ebp-44h] BYREF
    float frametime; // [esp+54h] [ebp-38h]
    const centity_s *pVehicleCEnt; // [esp+58h] [ebp-34h]
    float tagAxis[3][3]; // [esp+5Ch] [ebp-30h] BYREF
    float tagAngles[3]; // [esp+80h] [ebp-Ch] BYREF

    if ( (es->lerp.eFlags & 0x10000) == 0 )
        return 1;
    pVehicleCEnt = CG_GetEntity(localClientNum, cent->nextState.otherEntityNum);
    if ( ((*((unsigned int *)pVehicleCEnt + 201) >> 1) & 1) == 0 )
        return 0;
    vehicleObj = Com_GetClientDObj(pVehicleCEnt->nextState.number, localClientNum);
    if ( !vehicleObj )
        return 0;
    boneIndex = es->lerp.u.player.vehicleAnimBoneIndex - 1;
    if ( boneIndex < 0 )
        return 0;
    if ( CG_DObjGetWorldBoneMatrix(&pVehicleCEnt->pose, vehicleObj, boneIndex, tagAxis, tagOrigin) )
    {
        AxisToAngles(tagAxis, tagAngles);
        if ( !boneIndex )
            tagAngles[1] = tagAngles[1] + 90.0;
        VehAnim_UpdatePosRot(localClientNum, es, ci, 0, tagOrigin, tagAngles);
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        frametime = (float)(cgameGlob->time - ci->vehAnimLastTime) * 0.001;
        if ( frametime >= 0.0 && frametime <= 0.30000001 )
        {
            if ( ci->vehAnimLastBone != boneIndex )
            {
                ci->vehAnimLerpStartTime = cgameGlob->time;
                ci->vehAnimLerpEndTime = vehAnimLerpTime + ci->vehAnimLerpStartTime;
            }
        }
        else
        {
            ci->vehAnimLastBone = boneIndex;
            ci->vehAnimLerpEndTime = 0;
        }
        if ( ci->vehAnimLerpEndTime >= cgameGlob->time )
        {
            lerpFrac = (float)(cgameGlob->time - ci->vehAnimLerpStartTime)
                             / (float)(ci->vehAnimLerpEndTime - ci->vehAnimLerpStartTime);
            if ( (lerpFrac < 0.0 || lerpFrac > 1.0)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                            672,
                            0,
                            "%s",
                            "lerpFrac >= 0.f && lerpFrac <= 1.f") )
            {
                __debugbreak();
            }
            Vec3Lerp(ci->vehAnimLastOrg, tagOrigin, lerpFrac, cent->pose.origin);
            for ( i = 0; i < 3; ++i )
            {
                v5 = ci->vehAnimLastAngle[i];
                cent->pose.angles[i] = AngleNormalize180(tagAngles[i] - v5) * lerpFrac + v5;
            }
        }
        else
        {
            cent->pose.origin[0] = tagOrigin[0];
            cent->pose.origin[1] = tagOrigin[1];
            cent->pose.origin[2] = tagOrigin[2];
            cent->pose.angles[0] = tagAngles[0];
            cent->pose.angles[1] = tagAngles[1];
            cent->pose.angles[2] = tagAngles[2];
            ci->vehAnimLastOrg[0] = cent->pose.origin[0];
            ci->vehAnimLastOrg[1] = cent->pose.origin[1];
            ci->vehAnimLastOrg[2] = cent->pose.origin[2];
            ci->vehAnimLastAngle[0] = cent->pose.angles[0];
            ci->vehAnimLastAngle[1] = cent->pose.angles[1];
            ci->vehAnimLastAngle[2] = cent->pose.angles[2];
        }
        ci->vehAnimLastBone = boneIndex;
        ci->vehAnimLastTime = cgameGlob->time;
        return 1;
    }
    else
    {
        Com_PrintWarning(17, "WARNING: aborting player positioning on turret since 'tag_weapon' does not exist\n");
        return 0;
    }
}

void __cdecl CG_HighlightPlayer(int localClientNum, centity_s *cent, ShaderConstantSet *constantSet, bool isExtraCam)
{
    int clientNum; // ecx
    int v5; // [esp+2Ch] [ebp-E4h]
    int v6; // [esp+30h] [ebp-E0h]
    int v7; // [esp+34h] [ebp-DCh]
    int v8; // [esp+38h] [ebp-D8h]
    int integer; // [esp+40h] [ebp-D0h]
    int j; // [esp+68h] [ebp-A8h]
    float v11; // [esp+70h] [ebp-A0h]
    float expandedColor[4]; // [esp+74h] [ebp-9Ch] BYREF
    int i; // [esp+84h] [ebp-8Ch]
    int blinkFadeTime; // [esp+88h] [ebp-88h]
    float blinkFade; // [esp+8Ch] [ebp-84h]
    float enemyColor[4]; // [esp+90h] [ebp-80h] BYREF
    const clientInfo_t *client; // [esp+A0h] [ebp-70h]
    float heroLightingR[4]; // [esp+A4h] [ebp-6Ch] BYREF
    const cg_s *cgameGlob; // [esp+B4h] [ebp-5Ch]
    float corpseFade; // [esp+B8h] [ebp-58h]
    float heroLightingG[4]; // [esp+BCh] [ebp-54h] BYREF
    bool isEnemy; // [esp+CFh] [ebp-41h]
    float normalHeroLightingG[4]; // [esp+D0h] [ebp-40h] BYREF
    float heroLightingB[4]; // [esp+E0h] [ebp-30h] BYREF
    float normalHeroLightingR[4]; // [esp+F0h] [ebp-20h] BYREF
    float normalHeroLightingB[4]; // [esp+100h] [ebp-10h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    normalHeroLightingR[0] = 1.0f;
    memset(&normalHeroLightingR[1], 0, 12);
    normalHeroLightingG[0] = 0.0f;
    normalHeroLightingG[1] = 1.0f;
    normalHeroLightingG[2] = 0.0f;
    normalHeroLightingG[3] = 0.0f;
    normalHeroLightingB[0] = 0.0f;
    normalHeroLightingB[1] = 0.0f;
    normalHeroLightingB[2] = 1.0f;
    normalHeroLightingB[3] = 0.0f;
    heroLightingR[0] = 1.0f;
    memset(&heroLightingR[1], 0, 12);
    heroLightingG[0] = 0.0f;
    heroLightingG[1] = 1.0f;
    heroLightingG[2] = 0.0f;
    heroLightingG[3] = 0.0f;
    heroLightingB[0] = 0.0f;
    heroLightingB[1] = 0.0f;
    heroLightingB[2] = 1.0f;
    heroLightingB[3] = 0.0f;
    corpseFade = 0.0f;
    isEnemy = 0;
    R_UnmapShaderConstantSet(constantSet, 0);
    R_UnmapShaderConstantSet(constantSet, 1u);
    R_UnmapShaderConstantSet(constantSet, 2u);
    if ( ((*((unsigned int *)cent + 201) >> 1) & 1) != 0 )
    {
        clientNum = cent->nextState.clientNum;
        client = &cgameGlob->bgs.clientinfo[clientNum];
        if ( (const cg_s *)((char *)cgameGlob + clientNum * 1480) == (const cg_s *)-389096
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp", 853, 0, "%s", "client") )
        {
            __debugbreak();
        }
        if ( (cent->nextState.lerp.eFlags & 0x40000) == 0 )
            goto LABEL_11;
        if ( !cent->miscTime || cent->miscTime > cgameGlob->time )
            cent->miscTime = cgameGlob->time;
        corpseFade = (float)(cgameGlob->time - cent->miscTime) / (float)(cg_corpseHighlightFadeTime->current.value * 1000.0);
        if ( corpseFade < 1.0 )
        {
LABEL_11:
            if ( client->team == TEAM_FREE || client->team != cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team )
                isEnemy = 1;
            if ( !Flame_GetLocalClientSourceRange() )
            {
                if ( R_MapShaderConstantSet(constantSet, 3u, "scriptVector1") )
                {
                    if ( isEnemy )
                    {
                        blinkFadeTime = cgameGlob->time % cg_playerHighlightBlinkTime->current.integer
                                                    - cg_playerHighlightBlinkTime->current.integer / 2;
                        blinkFade = (float)((float)((float)(1.0 - cg_playerHighlightMinFade->current.value)
                                                                            * (
                                                                                    -fabs(
                                                                                        (float)blinkFadeTime
                                                                                    / (float)((float)cg_playerHighlightBlinkTime->current.integer / 2.0))))
                                                            + cg_playerHighlightMinFade->current.value)
                                            * (float)(1.0 - corpseFade);
                        if ( team_indicator->current.integer == 3 )
                            Dvar_GetUnpackedColor(cg_TeamColor_EnemyTeamAlt, enemyColor);
                        else
                            Dvar_GetUnpackedColor(cg_cameraSpikeEnemyColor, enemyColor);
                        for ( i = 0; i < 3; ++i )
                            enemyColor[i] = enemyColor[i] * cg_cameraSpikeHighlightBrightness->current.value;
                        R_SetShaderConstantSetValue(
                            constantSet,
                            3u,
                            (float)(enemyColor[0] * blinkFade) + (float)(1.0 - blinkFade),
                            (float)(enemyColor[1] * blinkFade) + (float)(1.0 - blinkFade),
                            (float)(enemyColor[2] * blinkFade) + (float)(1.0 - blinkFade),
                            1.0);
                    }
                    else
                    {
                        R_SetShaderConstantSetValue(constantSet, 3u, 1.0, 1.0, 1.0, 1.0);
                    }
                }
                if ( (cgameGlob->predictedPlayerState.eFlags & 0x4000) != 0 && cgameGlob->predictedPlayerState.vehicleType == 6
                    || (cgameGlob->predictedPlayerState.eFlags2 & 0x10000000) != 0 )
                {
                    if ( !isEnemy || (client->perks[1] & 8) != 0 )
                        return;
                    v11 = (float)((float)(1.0 - cg_playerHighlightMinFade->current.value)
                                            * (
                                                    fabs(
                                                        (float)(cgameGlob->time % cg_playerHighlightBlinkTime->current.integer
                                                                    - cg_playerHighlightBlinkTime->current.integer / 2)
                                                    / (float)((float)cg_playerHighlightBlinkTime->current.integer / 2.0))))
                            + cg_playerHighlightMinFade->current.value;
                    if ( team_indicator->current.integer == 3 )
                        Dvar_GetUnpackedColor(cg_TeamColor_EnemyTeamAlt, expandedColor);
                    else
                        Dvar_GetUnpackedColor(cg_playerHighlightEnemyColor, expandedColor);
                    for ( j = 0; j < 3; ++j )
                        expandedColor[j] = expandedColor[j] * cg_playerHighlightBrightness->current.value;
                    heroLightingR[0] = expandedColor[0];
                    memset(&heroLightingR[1], 0, 12);
                    heroLightingG[0] = 0.0f;
                    heroLightingG[1] = expandedColor[1];
                    heroLightingG[2] = 0.0f;
                    heroLightingG[3] = 0.0f;
                    heroLightingB[0] = 0.0f;
                    heroLightingB[1] = 0.0f;
                    heroLightingB[2] = expandedColor[2];
                    heroLightingB[3] = 0.0f;
                    Vec4Lerp(normalHeroLightingR, heroLightingR, v11, heroLightingR);
                    Vec4Lerp(normalHeroLightingG, heroLightingG, v11, heroLightingG);
                    Vec4Lerp(normalHeroLightingB, heroLightingB, v11, heroLightingB);
                }
            }
            if ( (CG_IsInfrared(localClientNum) || isExtraCam && isEnemy) && (client->perks[1] & 8) == 0 )
            {
                integer = cg_infraredHighlightOffset->current.integer;
                heroLightingR[0] = cg_infraredHighlightScale->current.value;
                heroLightingR[1] = 0.0f;
                heroLightingR[2] = 0.0f;
                LODWORD(heroLightingR[3]) = integer;
                v7 = cg_infraredHighlightScale->current.integer;
                v8 = cg_infraredHighlightOffset->current.integer;
                heroLightingG[0] = 0.0f;
                LODWORD(heroLightingG[1]) = v7;
                heroLightingG[2] = 0.0f;
                LODWORD(heroLightingG[3]) = v8;
                v5 = cg_infraredHighlightScale->current.integer;
                v6 = cg_infraredHighlightOffset->current.integer;
                heroLightingB[0] = 0.0f;
                heroLightingB[1] = 0.0f;
                LODWORD(heroLightingB[2]) = v5;
                LODWORD(heroLightingB[3]) = v6;
            }
            if ( corpseFade >= 0.0 )
            {
                Vec4Lerp(heroLightingR, normalHeroLightingR, corpseFade, heroLightingR);
                Vec4Lerp(heroLightingG, normalHeroLightingG, corpseFade, heroLightingG);
                Vec4Lerp(heroLightingB, normalHeroLightingB, corpseFade, heroLightingB);
            }
            if ( R_MapShaderConstantSet(constantSet, 0, "heroLightingR") )
                R_SetShaderConstantSetValue(constantSet, 0, heroLightingR);
            if ( R_MapShaderConstantSet(constantSet, 1u, "heroLightingG") )
                R_SetShaderConstantSetValue(constantSet, 1u, heroLightingG);
            if ( R_MapShaderConstantSet(constantSet, 2u, "heroLightingB") )
                R_SetShaderConstantSetValue(constantSet, 2u, heroLightingB);
        }
    }
}

int delta = 5;
void __cdecl CG_Player(int localClientNum, centity_s *cent)
{
    XModel **worldModel; // esi
    unsigned int String; // eax
    bool v4; // [esp+38h] [ebp-1D8h]
    float v5; // [esp+3Ch] [ebp-1D4h]
    bool v7; // [esp+44h] [ebp-1CCh]
    bool v8; // [esp+48h] [ebp-1C8h]
    float v9; // [esp+6Ch] [ebp-1A4h]
    snapshot_s *v10; // [esp+94h] [ebp-17Ch]
    snapshot_s *nextSnap; // [esp+98h] [ebp-178h]
    GfxScaledPlacement placement; // [esp+B8h] [ebp-158h] BYREF
    unsigned __int8 boneIndex; // [esp+DBh] [ebp-135h] BYREF
    renderOptions_s stowedWeaponOptions; // [esp+DCh] [ebp-134h]
    int i; // [esp+E0h] [ebp-130h]
    int burnedTime; // [esp+E4h] [ebp-12Ch]
    float v17[3]; // [esp+E8h] [ebp-128h] BYREF
    cg_s *LocalClientGlobals; // [esp+F4h] [ebp-11Ch]
    const playerState_s *ps; // [esp+F8h] [ebp-118h]
    float pos[3]; // [esp+FCh] [ebp-114h] BYREF
    float footPos_ri[3]; // [esp+108h] [ebp-108h] BYREF
    float footPos_le[3]; // [esp+114h] [ebp-FCh] BYREF
    float footMatrix[3][3]; // [esp+120h] [ebp-F0h] BYREF
    int time; // [esp+144h] [ebp-CCh]
    unsigned __int16 t; // [esp+148h] [ebp-C8h]
    int stowedWeaponModelIndex; // [esp+14Ch] [ebp-C4h]
    DObj *obj; // [esp+150h] [ebp-C0h]
    unsigned int primaryModelMask; // [esp+154h] [ebp-BCh]
    cg_s *cgameGlob; // [esp+158h] [ebp-B8h]
    int iClientNum; // [esp+15Ch] [ebp-B4h]
    ShaderConstantSet extraCamConstSet; // [esp+160h] [ebp-B0h] BYREF
    float burnFraction; // [esp+1E0h] [ebp-30h]
    const entityState_s *p_nextState; // [esp+1E4h] [ebp-2Ch]
    bool inClientTeam; // [esp+1EBh] [ebp-25h]
    int textureOverrideIndex; // [esp+1ECh] [ebp-24h]
    clientInfo_t *ci; // [esp+1F0h] [ebp-20h]
    ShaderConstantSet *extraCamConstSetPtr; // [esp+1F4h] [ebp-1Ch]
    const WeaponVariantDef *stowedWeaponDef; // [esp+1F8h] [ebp-18h]
    const WeaponVariantDef *primaryWeaponDef; // [esp+1FCh] [ebp-14h]
    unsigned int renderFxFlags; // [esp+200h] [ebp-10h]
    float lightingOrigin[3]; // [esp+204h] [ebp-Ch] BYREF

    CG_PlayerUpdateUserRigidBody(localClientNum, cent);
    p_nextState = &cent->nextState;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( (cent->nextState.lerp.eFlags & 0x20) == 0 )
    {
        if ( (p_nextState->lerp.eFlags & 0x40000) != 0 )
        {
            if ( (p_nextState->lerp.eFlags & 0x4000) != 0 )
                CG_UpdateMinigunSpin(localClientNum, cgameGlob, cent);
        }
        else
        {
            obj = Com_GetClientDObj(p_nextState->number, localClientNum);
            if ( obj )
            {
                iClientNum = p_nextState->clientNum;
                if ( p_nextState->clientNum >= 0x20u
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                                1014,
                                0,
                                "%s",
                                "(unsigned)es->clientNum < ARRAY_COUNT( cgameGlob->bgs.clientinfo )") )
                {
                    __debugbreak();
                }
                if ( (unsigned int)iClientNum >= 0x20
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                                1016,
                                0,
                                "iClientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                                iClientNum,
                                32) )
                {
                    __debugbreak();
                }
                ci = &cgameGlob->bgs.clientinfo[iClientNum];
                if (cent->tree != cgameGlob->bgs.clientinfo[iClientNum].pXAnimTree)
                {
                    cent->tree = ci->pXAnimTree;
                }

                {
                    PROF_SCOPED("script");
                    if (cent->tree)
                    {
                        v8 = cg_loadScripts && cg_loadScripts->current.enabled;
                        if (v8 && ((*((unsigned int *)cent + 201) >> 8) & 1) == 0)
                        {
                            *((unsigned int *)cent + 201) |= 0x100u;
                            Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
                            t = CScr_ExecEntThread(cent, cg_scr_data.playerspawned, 1u);
                            Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
                        }
                    }
                }
                
                CG_UpdateMinigunSpin(localClientNum, cgameGlob, cent);
                CG_UpdatePerkSounds(localClientNum, cgameGlob);

                {
                    PROF_SCOPED("sliding");

                    if (BG_IsSliding(ci))
                    {
                        time = CG_GetLocalClientGlobals(localClientNum)->time;
                        if (cent->nextSlideFX < time)
                        {
                            cent->nextSlideFX = delta + time;
                            if (obj)
                            {
                                CG_DObjGetWorldTagMatrix(&cent->pose, obj, scr_const.j_ankle_le, footMatrix, footPos_le);
                                CG_DObjGetWorldTagMatrix(&cent->pose, obj, scr_const.j_ankle_ri, footMatrix, footPos_ri);
                                pos[0] = 0.5 * (float)(footPos_le[0] + footPos_ri[0]);
                                pos[1] = 0.5 * (float)(footPos_le[1] + footPos_ri[1]);
                                pos[2] = 0.5 * (float)(footPos_le[2] + footPos_ri[2]);
                                FX_PlayOrientedEffect(localClientNum, cgMedia.helicopterLightSmoke, cgameGlob->time, pos, footMatrix);
                            }
                        }
                    }
                    else
                    {
                        cent->nextSlideFX = 0;
                    }
                }

                {
                    PROF_SCOPED("triggers");
                    if (CL_LocalClient_IsFirstActive(localClientNum))
                        CG_DoTouchTriggers(cent, localClientNum);
                }
 
                nextSnap = cgameGlob->nextSnap;
                v7 = (nextSnap->ps.otherFlags & 6) != 0 && p_nextState->number == nextSnap->ps.clientNum;
                if ( !v7 || cgameGlob->renderingThirdPerson )
                {
                    if ( cgameGlob->renderingThirdPerson == TP_FOR_SHADOW )
                    {
                        v10 = cgameGlob->nextSnap;
                        if ( (v10->ps.otherFlags & 6) != 0 && p_nextState->number == v10->ps.clientNum )
                            DObjSetFlag(obj, 0x20u, 1);
                        else
                            DObjSetFlag(obj, 0x20u, 0);
                    }
                    ps = &cgameGlob->nextSnap->ps;
                    if ( (ps->otherFlags & 2) != 0 && ps->clientNum == p_nextState->number )
                    {
                        LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
                        CG_DObjGetWorldTagPos(&LocalClientGlobals->predictedPlayerEntity.pose, obj, scr_const.tag_flash, v17);
                    }

                    PROF_SCOPED("animation");
                    BG_PlayerAnimation(localClientNum, p_nextState, ci);
                    if ( CG_Player_ApplyVehicleAnimOffsets(localClientNum, p_nextState, cent, ci) )
                    {
                        if ( (p_nextState->lerp.eFlags & 0x4300) != 0 && (p_nextState->lerp.eFlags2 & 0x10000000) == 0 )
                            CG_PlayerTurretPositionAndBlend(localClientNum, cent);
                        {
                            PROF_SCOPED("pre controllers");
                            CG_Player_PreControllers(obj, cent);
                        }
                        {
                            PROF_SCOPED("weapon visibility");
                            CG_UpdateWeaponVisibilityImmediate(localClientNum, cent);
                        }

                        PROF_SCOPED("R_AddDObjToScene");

                        lightingOrigin[0] = cent->pose.origin[0];
                        lightingOrigin[1] = cent->pose.origin[1];
                        lightingOrigin[2] = cent->pose.origin[2];
                        if ( (p_nextState->lerp.eFlags & 8) != 0 )
                        {
                            lightingOrigin[2] = lightingOrigin[2] + 12.0;
                        }
                        else if ( (p_nextState->lerp.eFlags & 4) != 0 )
                        {
                            lightingOrigin[2] = lightingOrigin[2] + 20.0;
                        }
                        else
                        {
                            lightingOrigin[2] = lightingOrigin[2] + 32.0;
                        }
                        if ( (p_nextState->lerp.eFlags2 & 0x200000) != 0 || ((*((unsigned int *)cent + 201) >> 5) & 1) != 0 )
                        {
                            if ( !cent->pose.startBurnTime )
                                cent->pose.startBurnTime = CG_GetLocalClientGlobals(localClientNum)->time;
                            burnedTime = CG_GetLocalClientGlobals(localClientNum)->time - cent->pose.startBurnTime;
                            burnFraction = (float)burnedTime / 2000.0;
                            if ( (float)(burnFraction - 1.0) < 0.0 )
                                v9 = burnFraction;
                            else
                                v9 = 1.0f;
                            if ( (float)(0.0 - burnFraction) < 0.0 )
                                v5 = v9;
                            else
                                v5 = 0.0f;
                            burnFraction = v5;
                        }
                        else
                        {
                            burnFraction = 0.0f;
                            cent->pose.startBurnTime = 0;
                        }
                        CG_HighlightPlayer(localClientNum, cent, &cent->pose.constantSet, 0);
                        stowedWeaponDef = BG_GetWeaponVariantDef(p_nextState->lerp.u.player.stowedWeapon);
                        stowedWeaponModelIndex = -1;
                        for ( i = DObjGetNumModels(obj) - 1; i >= 0; --i )
                        {
                            worldModel = stowedWeaponDef->weapDef->worldModel;
                            if ( *worldModel == DObjGetModel(obj, i) )
                            {
                                stowedWeaponModelIndex = i;
                                break;
                            }
                        }
                        primaryWeaponDef = BG_GetWeaponVariantDef(p_nextState->lerp.u.player.primaryWeapon);
                        primaryModelMask = -1;
                        if ( stowedWeaponModelIndex >= 0 )
                            primaryModelMask &= ~(1 << stowedWeaponModelIndex);
                        textureOverrideIndex = CG_SetupWeaponOptionsRender(
                                                                         localClientNum,
                                                                         cent->pose.origin,
                                                                         primaryWeaponDef,
                                                                         cent->nextState.renderOptions,
                                                                         primaryModelMask,
                                                                         &cent->pose.constantSet,
                                                                         0,
                                                                         -1);
                        if ( stowedWeaponModelIndex >= 0 && p_nextState->lerp.u.player.stowedWeaponCamo )
                        {
                            stowedWeaponOptions.i = 0;
                            stowedWeaponOptions.i = p_nextState->lerp.u.player.stowedWeaponCamo & 0x3F;
                            textureOverrideIndex = CG_SetupWeaponOptionsRender(
                                                                             localClientNum,
                                                                             cent->pose.origin,
                                                                             stowedWeaponDef,
                                                                             stowedWeaponOptions,
                                                                             1 << stowedWeaponModelIndex,
                                                                             0,
                                                                             0,
                                                                             textureOverrideIndex);
                        }
                        renderFxFlags = 4194308;
                        v4 = ci->team == cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].team && ci->team;
                        inClientTeam = v4;
                        if ( !CG_IsInfrared(localClientNum) || (ci->perks[1] & 8) != 0 )
                        {
                            if ( cent->fxHeartbeat )
                            {
                                FX_ThroughWithEffect(localClientNum, cent->fxHeartbeat, 1);
                                cent->fxHeartbeat = 0;
                            }
                        }
                        else
                        {
                            renderFxFlags |= 0x80u;
                            if ( !cent->fxHeartbeat && inClientTeam )
                            {
                                String = SL_FindString("j_spine4", SCRIPTINSTANCE_SERVER);
                                if ( DObjGetBoneIndex(obj, String, &boneIndex, -1) )
                                    cent->fxHeartbeat = FX_PlayBoltedEffect(localClientNum, cgMedia.infraredHeartbeat, 0, cent->nextState.number, boneIndex);
                            }
                        }
                        extraCamConstSetPtr = 0;
                        if ( cgameGlob->extraCamActive )
                        {
                            R_InitShaderConstantSet(&extraCamConstSet);
                            CG_HighlightPlayer(localClientNum, cent, &extraCamConstSet, 1);
                            extraCamConstSetPtr = &extraCamConstSet;
                        }
                        R_AddDObjToScene(
                            obj,
                            &cent->pose,
                            p_nextState->number,
                            renderFxFlags,
                            lightingOrigin,
                            0.0,
                            0.0,
                            burnFraction,
                            cent->pose.wetness,
                            -1,
                            textureOverrideIndex,
                            &cent->pose.constantSet,
                            extraCamConstSetPtr,
                            0.0,
                            1.0);
                        if ( (p_nextState->lerp.eFlags & 0x40000) == 0 )
                        {
                            PROF_SCOPED("CG_AddPlayerWeapon");
                            placement.base.origin[0] = cent->pose.origin[0];
                            placement.base.origin[1] = cent->pose.origin[1];
                            placement.base.origin[2] = cent->pose.origin[2];
                            AnglesToQuat(cent->pose.angles, placement.base.quat);
                            placement.scale = 1.0f;
                            CG_AddPlayerWeapon(localClientNum, &placement, 0, cent, 1);
                        }
                        CG_DrawWaterTrail(localClientNum, cent);
                    }
                }
            }
        }
    }
}

void __cdecl CG_PlayerTurretPositionAndBlend(int localClientNum, centity_s *cent)
{
    const char *v2; // eax
    const XModel *Model; // eax
    const char *Name; // eax
    const XModel *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *AnimDebugName; // eax
    const char *v9; // eax
    double v10; // st7
    unsigned __int16 tag_gunner_barrel1; // [esp+1Eh] [ebp-286h]
    float plyrAxis[3][3]; // [esp+80h] [ebp-224h] BYREF
    unsigned __int16 gunnerTags[4]; // [esp+A4h] [ebp-200h]
    int gunnerNum; // [esp+ACh] [ebp-1F8h]
    int seatNum; // [esp+B0h] [ebp-1F4h]
    float fHeightRatio; // [esp+B4h] [ebp-1F0h]
    unsigned __int16 tagName; // [esp+B8h] [ebp-1ECh]
    int iPrevBlend; // [esp+BCh] [ebp-1E8h]
    float fPrevTransZ; // [esp+C0h] [ebp-1E4h]
    DObj *obj; // [esp+C4h] [ebp-1E0h]
    int numVertChildren; // [esp+C8h] [ebp-1DCh]
    DObj *turretObj; // [esp+CCh] [ebp-1D8h]
    cg_s *cgameGlob; // [esp+D0h] [ebp-1D4h]
    float trans2[3]; // [esp+D4h] [ebp-1D0h] BYREF
    float tmpAxis[3][3]; // [esp+E0h] [ebp-1C4h] BYREF
    float yaw; // [esp+104h] [ebp-1A0h]
    float trans[3]; // [esp+108h] [ebp-19Ch] BYREF
    float tagOrigin[3]; // [esp+114h] [ebp-190h] BYREF
    int iBlend; // [esp+120h] [ebp-184h]
    unsigned int heightAnim; // [esp+124h] [ebp-180h]
    float fDelta; // [esp+128h] [ebp-17Ch]
    float fPrevBlend; // [esp+12Ch] [ebp-178h]
    float rot[2]; // [esp+130h] [ebp-174h] BYREF
    float tagAxis[3][3]; // [esp+138h] [ebp-16Ch] BYREF
    unsigned int leafAnim1; // [esp+15Ch] [ebp-148h]
    trace_t trace; // [esp+160h] [ebp-144h] BYREF
    int numHorChildren; // [esp+19Ch] [ebp-108h]
    const clientInfo_t *ci; // [esp+1A0h] [ebp-104h]
    float tagHeight; // [esp+1A4h] [ebp-100h]
    int i; // [esp+1A8h] [ebp-FCh]
    unsigned int baseAnim; // [esp+1ACh] [ebp-F8h]
    float vehTagOriginMat[4][3]; // [esp+1B0h] [ebp-F4h] BYREF
    float vehTagOriginMatInv[4][3]; // [esp+1E0h] [ebp-C4h] BYREF
    int clientNum; // [esp+210h] [ebp-94h]
    centity_s *pTurretCEnt; // [esp+214h] [ebp-90h]
    const lerpFrame_t *pLerpAnim; // [esp+218h] [ebp-8Ch]
    const WeaponDef *weapDef; // [esp+21Ch] [ebp-88h]
    float fBlend; // [esp+220h] [ebp-84h]
    float axis[4][3]; // [esp+224h] [ebp-80h] BYREF
    XAnimTree_s *pAnimTree; // [esp+254h] [ebp-50h]
    XAnim_s *pXAnims; // [esp+258h] [ebp-4Ch]
    bool isT34; // [esp+25Fh] [ebp-45h]
    unsigned int leafAnim2; // [esp+260h] [ebp-44h]
    float localYaw; // [esp+264h] [ebp-40h]
    float turretAxis[4][3]; // [esp+268h] [ebp-3Ch] BYREF
    float vDelta[3]; // [esp+298h] [ebp-Ch]

    if ( cent->nextState.otherEntityNum >= 32 && cent->nextState.otherEntityNum != 1023 )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        clientNum = cent->nextState.clientNum;
        if ( (unsigned int)clientNum >= 0x20
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                        316,
                        0,
                        "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        clientNum,
                        32) )
        {
            __debugbreak();
        }
        ci = &cgameGlob->bgs.clientinfo[clientNum];
        if ( ci->infoValid )
        {
            pLerpAnim = &ci->legs;
            if ( ci->legs.animationNumber )
            {
                if ( pLerpAnim->animation )
                {
                    if ( (pLerpAnim->animation->flags & 4) != 0 )
                    {
                        pTurretCEnt = CG_GetEntity(localClientNum, cent->nextState.otherEntityNum);
                        if ( ((*((unsigned int *)pTurretCEnt + 201) >> 1) & 1) != 0 )
                        {
                            turretObj = Com_GetClientDObj(pTurretCEnt->nextState.number, localClientNum);
                            if ( turretObj )
                            {
                                obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
                                if ( obj )
                                {
                                    if ( (cent->currentState.eFlags & 0x4000) != 0 )
                                    {
                                        if ( !CG_DObjGetWorldTagMatrix(
                                                        &pTurretCEnt->pose,
                                                        turretObj,
                                                        scr_const.tag_origin,
                                                        tagAxis,
                                                        tagOrigin)
                                            && !CG_DObjGetWorldTagMatrix(
                                                        &pTurretCEnt->pose,
                                                        turretObj,
                                                        scr_const.tag_origin,
                                                        tagAxis,
                                                        tagOrigin)
                                            && !Assert_MyHandler(
                                                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                                                        346,
                                                        0,
                                                        "%s",
                                                        "CG_DObjGetWorldTagMatrix( &pTurretCEnt->pose, turretObj, scr_const.tag_origin, tagAxis, tagOrigin )") )
                                        {
                                            __debugbreak();
                                        }
                                        *(_QWORD *)&vehTagOriginMat[0][0] = *(_QWORD *)&tagAxis[0][0];
                                        vehTagOriginMat[0][2] = tagAxis[0][2];
                                        vehTagOriginMat[1][0] = tagAxis[1][0];
                                        vehTagOriginMat[1][1] = tagAxis[1][1];
                                        vehTagOriginMat[1][2] = tagAxis[1][2];
                                        vehTagOriginMat[2][0] = tagAxis[2][0];
                                        vehTagOriginMat[2][1] = tagAxis[2][1];
                                        vehTagOriginMat[2][2] = tagAxis[2][2];
                                        *(_QWORD *)&vehTagOriginMat[3][0] = *(_QWORD *)tagOrigin;
                                        vehTagOriginMat[3][2] = tagOrigin[2];
                                        seatNum = cent->currentState.u.player.vehicleSeat;
                                        if ( seatNum < 1 || seatNum > 4 )
                                        {
                                            AxisToAngles(tagAxis, cent->pose.angles);
                                            cent->pose.origin[0] = tagOrigin[0];
                                            cent->pose.origin[1] = tagOrigin[1];
                                            cent->pose.origin[2] = tagOrigin[2];
                                        }
                                        else
                                        {
                                            gunnerTags[0] = scr_const.tag_gunner1;
                                            gunnerTags[1] = scr_const.tag_gunner2;
                                            gunnerTags[2] = scr_const.tag_gunner3;
                                            gunnerTags[3] = scr_const.tag_gunner4;
                                            gunnerNum = seatNum - 1;
                                            tagName = gunnerTags[seatNum - 1];
                                            if ( !CG_DObjGetWorldTagMatrix(&pTurretCEnt->pose, turretObj, tagName, tagAxis, tagOrigin)
                                                && !CG_DObjGetWorldTagMatrix(&pTurretCEnt->pose, turretObj, tagName, tagAxis, tagOrigin)
                                                && !Assert_MyHandler(
                                                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                                                            371,
                                                            0,
                                                            "%s",
                                                            "CG_DObjGetWorldTagMatrix( &pTurretCEnt->pose, turretObj, tagName, tagAxis, tagOrigin )") )
                                            {
                                                __debugbreak();
                                            }
                                            MatrixInverseOrthogonal43(vehTagOriginMat, vehTagOriginMatInv);
                                            MatrixMultiply(tagAxis, vehTagOriginMatInv, tmpAxis);
                                            AxisToAngles(tmpAxis, cent->pose.angles);
                                            MatrixTransformVector43Equals(tagOrigin, vehTagOriginMatInv[0]);
                                            cent->pose.origin[0] = tagOrigin[0];
                                            cent->pose.origin[1] = tagOrigin[1];
                                            cent->pose.origin[2] = tagOrigin[2];
                                        }
                                    }
                                    if ( (cent->currentState.eFlags & 0x4000) != 0 )
                                        tag_gunner_barrel1 = scr_const.tag_gunner_barrel1;
                                    else
                                        tag_gunner_barrel1 = scr_const.tag_weapon;
                                    tagName = tag_gunner_barrel1;
                                    if ( CG_DObjGetWorldTagMatrix(&pTurretCEnt->pose, turretObj, tag_gunner_barrel1, tagAxis, tagOrigin) )
                                    {
                                        if ( cgameGlob->frametime )
                                        {
                                            if ( (cent->currentState.eFlags & 0x4000) != 0 )
                                            {
                                                MatrixMultiply(tagAxis, vehTagOriginMatInv, tmpAxis);
                                                AxisCopy(tmpAxis, tagAxis);
                                                MatrixTransformVector43Equals(tagOrigin, vehTagOriginMatInv[0]);
                                            }
                                            if ( !pTurretCEnt->nextState.weapon
                                                && !Assert_MyHandler(
                                                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                                                            405,
                                                            0,
                                                            "%s",
                                                            "pTurretCEnt->nextState.weapon") )
                                            {
                                                __debugbreak();
                                            }
                                            weapDef = BG_GetWeaponDef(pTurretCEnt->nextState.weapon);
                                            if ( weapDef->weapClass != WEAPCLASS_TURRET
                                                && !Assert_MyHandler(
                                                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                                                            407,
                                                            0,
                                                            "%s",
                                                            "weapDef->weapClass == WEAPCLASS_TURRET") )
                                            {
                                                __debugbreak();
                                            }
                                            if ( weapDef->fAnimHorRotateInc == 0.0
                                                && !Assert_MyHandler(
                                                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                                                            408,
                                                            0,
                                                            "%s",
                                                            "weapDef->fAnimHorRotateInc") )
                                            {
                                                __debugbreak();
                                            }
                                            pAnimTree = ci->pXAnimTree;
                                            pXAnims = cgameGlob->bgs.animData->animScriptData.animTree.anims;
                                            baseAnim = pLerpAnim->animationNumber & 0xFFFFFBFF;
                                            yaw = vectosignedyaw(tagAxis[0]);
                                            localYaw = AngleNormalize180(yaw - pTurretCEnt->pose.angles[1]);
                                            isT34 = 0;
                                            if ( (cent->currentState.eFlags & 0x4000) != 0 )
                                            {
                                                Model = DObjGetModel(turretObj, 0);
                                                Name = XModelGetName(Model);
                                                if ( I_stristr(Name, "panzer") )
                                                {
                                                    tagOrigin[0] = (float)(handOfs_0 * tagAxis[0][0]) + tagOrigin[0];
                                                    tagOrigin[1] = (float)(handOfs_0 * tagAxis[0][1]) + tagOrigin[1];
                                                    tagOrigin[2] = (float)(handOfs_0 * tagAxis[0][2]) + tagOrigin[2];
                                                }
                                                else
                                                {
                                                    v5 = DObjGetModel(turretObj, 0);
                                                    v6 = XModelGetName(v5);
                                                    v7 = strstr(v6, "t34");
                                                    if ( v7 )
                                                    {
                                                        tagOrigin[0] = (float)(handOfs * tagAxis[0][0]) + tagOrigin[0];
                                                        tagOrigin[1] = (float)(handOfs * tagAxis[0][1]) + tagOrigin[1];
                                                        tagOrigin[2] = (float)(handOfs * tagAxis[0][2]) + tagOrigin[2];
                                                        isT34 = 1;
                                                    }
                                                }
                                                AnglesToAxis(vec3_origin, turretAxis);
                                                memset(turretAxis[3], 0, sizeof(float[3]));
                                                localYaw = 0.0f;
                                            }
                                            else
                                            {
                                                AnglesToAxis(pTurretCEnt->pose.angles, turretAxis);
                                                turretAxis[3][0] = pTurretCEnt->pose.origin[0];
                                                turretAxis[3][1] = pTurretCEnt->pose.origin[1];
                                                turretAxis[3][2] = pTurretCEnt->pose.origin[2];
                                            }
                                            vDelta[0] = cent->pose.origin[0] - turretAxis[3][0];
                                            vDelta[1] = cent->pose.origin[1] - turretAxis[3][1];
                                            vDelta[2] = cent->pose.origin[2] - turretAxis[3][2];
                                            tagHeight = (float)((float)(vDelta[0] * turretAxis[2][0]) + (float)(vDelta[1] * turretAxis[2][1]))
                                                                + (float)(vDelta[2] * turretAxis[2][2]);
                                            if ( (cent->currentState.eFlags & 0x4000) != 0 && ci->lerpLean > 0.0 )
                                            {
                                                if ( isT34 )
                                                    tagOrigin[2] = (float)((float)(1.0 - ci->lerpLean) * tagOrigin[2])
                                                                             + (float)(fCrouchHeightT34 * ci->lerpLean);
                                                else
                                                    tagOrigin[2] = (float)((float)(1.0 - ci->lerpLean) * tagOrigin[2])
                                                                             + (float)(fCrouchHeightPanzer * ci->lerpLean);
                                            }
                                            fDelta = tagHeight - (float)(tagOrigin[2] - turretAxis[3][2]);
                                            numVertChildren = XAnimGetNumChildren(pXAnims, baseAnim);
                                            fPrevTransZ = 0.0f;
                                            fPrevBlend = 0.0f;
                                            iPrevBlend = 0;
                                            leafAnim2 = 0;
                                            if ( !numVertChildren )
                                            {
                                                AnimDebugName = XAnimGetAnimDebugName(pXAnims, baseAnim);
                                                Com_Error(ERR_DROP, "Player anim '%s' has no children", AnimDebugName);
                                            }
                                            i = 0;
                                            do
                                            {
                                                heightAnim = XAnimGetChildAt(pXAnims, baseAnim, numVertChildren - 1 - i);
                                                numHorChildren = XAnimGetNumChildren(pXAnims, heightAnim);
                                                if ( !numHorChildren )
                                                {
                                                    v9 = XAnimGetAnimDebugName(pXAnims, heightAnim);
                                                    Com_Error(ERR_DROP, "Player anim '%s' has no children", v9);
                                                }
                                                if ( (cent->currentState.eFlags & 0x4000) != 0 )
                                                    fBlend = (float)numHorChildren * 0.5;
                                                else
                                                    fBlend = (float)((float)(numHorChildren - 1) * (float)(localYaw + weapDef->rightArc))
                                                                 / (float)(weapDef->leftArc + weapDef->rightArc);
                                                if ( fBlend >= 0.0 )
                                                {
                                                    if ( fBlend >= (float)(numHorChildren - 1) )
                                                        fBlend = (float)(numHorChildren - 1);
                                                }
                                                else
                                                {
                                                    fBlend = 0.0f;
                                                }
                                                iBlend = (int)fBlend;
                                                fBlend = fBlend - (float)(int)fBlend;
                                                leafAnim1 = XAnimGetChildAt(pXAnims, heightAnim, iBlend);
                                                XAnimGetAbsDelta(pXAnims, leafAnim1, rot, trans, 0.0);
                                                if ( fBlend != 0.0 )
                                                {
                                                    leafAnim2 = XAnimGetChildAt(pXAnims, heightAnim, iBlend + 1);
                                                    XAnimGetAbsDelta(pXAnims, leafAnim2, rot, trans2, 0.0);
                                                    Vec3Lerp(trans, trans2, fBlend, trans);
                                                }
                                                if ( trans[2] >= fDelta )
                                                    break;
                                                fPrevTransZ = trans[2];
                                                iPrevBlend = iBlend;
                                                fPrevBlend = fBlend;
                                                ++i;
                                            }
                                            while ( i < numVertChildren );
                                            XAnimClearTreeGoalWeightsStrict(pAnimTree, baseAnim, 0.0, -1);
                                            XAnimSetGoalWeight(obj, leafAnim1, 1.0 - fBlend, 0.0, 1.0, 0, 0, 0, -1);
                                            if ( fBlend != 0.0 )
                                                XAnimSetGoalWeight(obj, leafAnim2, fBlend, 0.0, 1.0, 0, 0, 0, -1);
                                            if ( i && i != numVertChildren )
                                            {
                                                if ( (float)(trans[2] - fPrevTransZ) == 0.0
                                                    && !Assert_MyHandler(
                                                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                                                                531,
                                                                0,
                                                                "%s",
                                                                "trans[2] - fPrevTransZ") )
                                                {
                                                    __debugbreak();
                                                }
                                                fHeightRatio = (float)(fDelta - fPrevTransZ) / (float)(trans[2] - fPrevTransZ);
                                                XAnimSetGoalWeight(obj, heightAnim, fHeightRatio, 0.0, 1.0, 0, 0, 0, -1);
                                                heightAnim = XAnimGetChildAt(pXAnims, baseAnim, numVertChildren - i);
                                                XAnimSetGoalWeight(obj, heightAnim, 1.0 - fHeightRatio, 0.0, 1.0, 0, 0, 0, -1);
                                                leafAnim1 = XAnimGetChildAt(pXAnims, heightAnim, iPrevBlend);
                                                XAnimSetGoalWeight(obj, leafAnim1, 1.0 - fPrevBlend, 0.0, 1.0, 0, 0, 0, -1);
                                                if ( fPrevBlend != 0.0 )
                                                {
                                                    leafAnim2 = XAnimGetChildAt(pXAnims, heightAnim, iPrevBlend + 1);
                                                    XAnimSetGoalWeight(obj, leafAnim2, fPrevBlend, 0.0, 1.0, 0, 0, 0, -1);
                                                }
                                            }
                                            else
                                            {
                                                XAnimSetGoalWeight(obj, heightAnim, 1.0, 0.0, 1.0, 0, 0, 0, -1);
                                            }
                                            XAnimCalcAbsDelta(obj, baseAnim, rot, trans);
                                            VectorAngleMultiply(trans, yaw);
                                            axis[3][0] = trans[0] + tagOrigin[0];
                                            axis[3][1] = trans[1] + tagOrigin[1];
                                            axis[3][2] = tagHeight + turretAxis[3][2];
                                            v10 = RotationToYaw(rot);
                                            yaw = v10 + yaw;
                                            YawToAxis(yaw, axis);
                                            AxisToAngles(axis, cent->pose.angles);
                                            cent->pose.origin[0] = axis[3][0];
                                            cent->pose.origin[1] = axis[3][1];
                                            cent->pose.origin[2] = axis[3][2];
                                            if ( (cent->currentState.eFlags & 0x4000) != 0 )
                                            {
                                                AnglesToAxis(cent->pose.angles, plyrAxis);
                                                MatrixMultiply(plyrAxis, vehTagOriginMat, tmpAxis);
                                                AxisToAngles(tmpAxis, cent->pose.angles);
                                                MatrixTransformVector43Equals(cent->pose.origin, vehTagOriginMat[0]);
                                            }
                                        }
                                    }
                                    else
                                    {
                                        v2 = SL_ConvertToString(tagName, SCRIPTINSTANCE_SERVER);
                                        Com_PrintWarning(
                                            17,
                                            "WARNING: aborting player positioning on turret since '%s' does not exist\n",
                                            v2);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void __cdecl CG_PlayerUpdateUserRigidBody(int localClientNum, centity_s *cent)
{
    float v2; // [esp+8h] [ebp-38h]
    float proneFeetPos[3]; // [esp+Ch] [ebp-34h] BYREF
    float dir[3]; // [esp+18h] [ebp-28h] BYREF
    float proneFeetAngle[3]; // [esp+24h] [ebp-1Ch] BYREF
    bool use_rigid_body; // [esp+32h] [ebp-Eh]
    bool use_prone_feet; // [esp+33h] [ebp-Dh]
    cg_s *cgameGlob; // [esp+34h] [ebp-Ch]
    const entityState_s *p_nextState; // [esp+38h] [ebp-8h]
    clientInfo_t *ci; // [esp+3Ch] [ebp-4h]
    int savedregs; // [esp+40h] [ebp+0h] BYREF

    use_rigid_body = 1;
    use_prone_feet = 0;
    p_nextState = &cent->nextState;
    if ( phys_userRigidBodies->current.enabled )
    {
        if ( cent->pose.isRagdoll )
            use_rigid_body = 0;
        else
            use_rigid_body = (p_nextState->lerp.eFlags & 0x20) == 0
                                        && (p_nextState->lerp.eFlags & 0x40000) == 0
                                        && Com_GetClientDObj(p_nextState->number, localClientNum) != 0;
    }
    else
    {
        use_rigid_body = 0;
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->inKillCam )
        use_rigid_body = 0;
    if ( (p_nextState->lerp.eFlags & 0x4000) != 0 && (p_nextState->lerp.eFlags2 & 0x10000000) == 0 )
        use_rigid_body = 0;
    if ( use_rigid_body )
    {
        if ( !cent->pose.physUserBody )
            cent->pose.physUserBody = (int)Phys_CreateUserBody(cent->pose.origin, cent->nextState.number, PHYS_GEOM_CYLINDER);
        if ( cent->pose.physUserBody )
            Phys_SetUserBody(cent->pose.physUserBody, cent->pose.origin);
        if ( (p_nextState->lerp.eFlags & 8) != 0 )
            use_prone_feet = 1;
    }
    else if ( cent->pose.physUserBody )
    {
        Phys_ObjDestroy(0, cent->pose.physUserBody);
        cent->pose.physUserBody = 0;
    }
    if ( p_nextState->clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                    755,
                    0,
                    "es->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    p_nextState->clientNum,
                    32) )
    {
        __debugbreak();
    }
    ci = &cgameGlob->bgs.clientinfo[p_nextState->clientNum];
    if ( use_prone_feet && !ci->infoValid )
        use_prone_feet = 0;
    if ( use_prone_feet && !cg_proneFeetCollisionHull->current.enabled )
        use_prone_feet = 0;
    if ( use_prone_feet )
    {
        v2 = ci->legs.yawAngle + proneFeetYawOffset;
        proneFeetAngle[0] = 0.0f;
        proneFeetAngle[1] = v2;
        proneFeetAngle[2] = 0.0f;
        AngleVectors(proneFeetAngle, dir, 0, 0);
        proneFeetPos[0] = (float)(proneFeetDist * dir[0]) + cent->pose.origin[0];
        proneFeetPos[1] = (float)(proneFeetDist * dir[1]) + cent->pose.origin[1];
        proneFeetPos[2] = (float)(proneFeetDist * dir[2]) + cent->pose.origin[2];
        if ( !cent->pose.physUserBodyProneFeet )
            cent->pose.physUserBodyProneFeet = (int)Phys_CreateUserBody(proneFeetPos, cent->nextState.number + 0x10000, PHYS_GEOM_CYLINDER_LARGE);
        if ( cent->pose.physUserBodyProneFeet )
            Phys_SetUserBody(cent->pose.physUserBodyProneFeet, proneFeetPos);
    }
    else if ( cent->pose.physUserBodyProneFeet )
    {
        Phys_ObjDestroy(0, cent->pose.physUserBodyProneFeet);
        cent->pose.physUserBodyProneFeet = 0;
    }
}

void __cdecl CG_UpdatePerkSounds(int localClientNum, const cg_s *cgameGlob)
{
    if ( (cgameGlob->predictedPlayerState.perks[0] & 0x80000000) == 0 )
        SND_SetSnapshot(SND_SNAPSHOT_ENEMIES, "default", 0.0, 1.0);
    else
        SND_SetSnapshot(SND_SNAPSHOT_ENEMIES, "mp_combat_awareness", 0.0, 1.0);
}

void __cdecl CG_Corpse(int localClientNum, centity_s *cent)
{
    float v2; // [esp+28h] [ebp-1D4h]
    float v3; // [esp+2Ch] [ebp-1D0h]
    unsigned __int16 t; // [esp+40h] [ebp-1BCh]
    DObj *obja; // [esp+44h] [ebp-1B8h]
    DObj *obj; // [esp+44h] [ebp-1B8h]
    cg_s *cgameGlob; // [esp+48h] [ebp-1B4h]
    ShaderConstantSet extraCamConstSet; // [esp+4Ch] [ebp-1B0h] BYREF
    float burnFraction; // [esp+C8h] [ebp-134h]
    cgs_t *cgs; // [esp+CCh] [ebp-130h]
    entityState_s *p_nextState; // [esp+D0h] [ebp-12Ch]
    int corpseIndex; // [esp+D4h] [ebp-128h]
    int textureOverrideIndex; // [esp+D8h] [ebp-124h]
    clientInfo_t *ci; // [esp+DCh] [ebp-120h]
    ShaderConstantSet *extraCamConstSetPtr; // [esp+E0h] [ebp-11Ch]
    FxMarkDObjUpdateContext markUpdateContext; // [esp+E4h] [ebp-118h] BYREF
    float lightingOrigin[3]; // [esp+1F0h] [ebp-Ch] BYREF

    p_nextState = &cent->nextState;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( (cent->nextState.lerp.eFlags & 0x40000) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                    1322,
                    0,
                    "%s",
                    "es->lerp.eFlags & EF_DEAD") )
    {
        __debugbreak();
    }
    if ( (p_nextState->lerp.eFlags & 0x20) == 0 && ((*((unsigned int *)cent + 201) >> 21) & 1) == 0 )
    {
        if ( CG_EntityNeedsScriptThread(localClientNum, cent) )
        {
            *((unsigned int *)cent + 201) |= 0x100u;
            Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
            t = CScr_ExecEntThread(cent, cg_scr_data.entityspawned, 1u);
            Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
        }
        corpseIndex = p_nextState->number - 32;
        if ( (unsigned int)corpseIndex >= 4
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                        1341,
                        0,
                        "%s",
                        "(unsigned)corpseIndex < MAX_CLIENT_CORPSES") )
        {
            __debugbreak();
        }
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        ci = &cgs->corpseinfo[corpseIndex];
        obja = Com_GetClientDObj(p_nextState->number, localClientNum);
        FX_MarkEntUpdateBegin(&markUpdateContext, obja, 0, 0);
        BG_UpdatePlayerDObj(localClientNum, obja, p_nextState, ci, 0);
        obj = Com_GetClientDObj(p_nextState->number, localClientNum);
        FX_MarkEntUpdateEnd(&markUpdateContext, localClientNum, p_nextState->number, obj, 0, 0);
        if ( obj )
        {
            BG_PlayerAnimation(localClientNum, p_nextState, ci);
            lightingOrigin[0] = cent->pose.origin[0];
            lightingOrigin[1] = cent->pose.origin[1];
            lightingOrigin[2] = cent->pose.origin[2];
            if ( (p_nextState->lerp.eFlags & 8) != 0 )
            {
                lightingOrigin[2] = lightingOrigin[2] + 12.0;
            }
            else if ( (p_nextState->lerp.eFlags & 4) != 0 )
            {
                lightingOrigin[2] = lightingOrigin[2] + 20.0;
            }
            else
            {
                lightingOrigin[2] = lightingOrigin[2] + 32.0;
            }
            if ( (p_nextState->lerp.eFlags2 & 0x200000) != 0 || ((*((unsigned int *)cent + 201) >> 5) & 1) != 0 )
            {
                if ( !cent->pose.startBurnTime )
                    cent->pose.startBurnTime = CG_GetLocalClientGlobals(localClientNum)->time;
                burnFraction = (float)(CG_GetLocalClientGlobals(localClientNum)->time - cent->pose.startBurnTime) / 2000.0;
                if ( (float)(burnFraction - 1.0) < 0.0 )
                    v3 = burnFraction;
                else
                    v3 = 1.0f;
                if ( (float)(0.0 - burnFraction) < 0.0 )
                    v2 = v3;
                else
                    v2 = 0.0f;
                burnFraction = v2;
            }
            else
            {
                burnFraction = 0.0f;
                cent->pose.startBurnTime = 0;
            }
            CG_HighlightPlayer(localClientNum, cent, &cent->pose.constantSet, 0);
            textureOverrideIndex = CG_SetupWeaponOptionsRender(
                                                             localClientNum,
                                                             cent->pose.origin,
                                                             0,
                                                             cent->nextState.renderOptions,
                                                             &cent->pose.constantSet,
                                                             0);
            extraCamConstSetPtr = 0;
            if ( cgameGlob->extraCamActive )
            {
                R_InitShaderConstantSet(&extraCamConstSet);
                CG_HighlightPlayer(localClientNum, cent, &extraCamConstSet, 1);
                extraCamConstSetPtr = &extraCamConstSet;
            }
            R_AddDObjToScene(
                obj,
                &cent->pose,
                p_nextState->number,
                0x400000u,
                lightingOrigin,
                0.0,
                0.0,
                burnFraction,
                0.0,
                -1,
                textureOverrideIndex,
                &cent->pose.constantSet,
                extraCamConstSetPtr,
                0.0,
                1.0);
        }
    }
}

void __cdecl CG_UpdatePlayerDObj(int localClientNum, centity_s *cent)
{
    DObj *dobj; // [esp+0h] [ebp-114h]
    DObj *dobja; // [esp+0h] [ebp-114h]
    cg_s *cgameGlob; // [esp+4h] [ebp-110h]
    FxMarkDObjUpdateContext markUpdateContext; // [esp+Ch] [ebp-108h] BYREF

    if ( ((*((unsigned int *)cent + 201) >> 1) & 1) != 0 )
    {
        if ( cent->nextState.clientNum != cent->nextState.number
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                        1416,
                        0,
                        "%s",
                        "es->clientNum == es->number") )
        {
            __debugbreak();
        }
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        if ( cent->nextState.clientNum >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                        1418,
                        0,
                        "es->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        cent->nextState.clientNum,
                        32) )
        {
            __debugbreak();
        }
        dobj = Com_GetClientDObj(cent->nextState.clientNum, localClientNum);
        FX_MarkEntUpdateBegin(&markUpdateContext, dobj, 0, 0);
        BG_UpdatePlayerDObj(
            localClientNum,
            dobj,
            &cent->nextState,
            &cgameGlob->bgs.clientinfo[cent->nextState.clientNum],
            0);
        dobja = Com_GetClientDObj(cent->nextState.clientNum, localClientNum);
        FX_MarkEntUpdateEnd(&markUpdateContext, localClientNum, cent->nextState.clientNum, dobja, 0, 0);
    }
}

void __cdecl CG_ResetPlayerEntity(int localClientNum, cg_s *cgameGlob, centity_s *cent, int resetAnimation)
{
    XAnimTree_s *pAnimTree; // [esp+1Ch] [ebp-10h]
    DObj *obj; // [esp+20h] [ebp-Ch]
    clientInfo_t *ci; // [esp+28h] [ebp-4h]

    if ( cent->nextState.clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                    1442,
                    0,
                    "es->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cent->nextState.clientNum,
                    32) )
    {
        __debugbreak();
    }
    ci = &cgameGlob->bgs.clientinfo[cent->nextState.clientNum];
    if ( (cent->nextState.lerp.eFlags & 0x40000) == 0 && resetAnimation )
    {
        pAnimTree = cgameGlob->bgs.clientinfo[cent->nextState.clientNum].pXAnimTree;
        if ( !pAnimTree
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp", 1450, 0, "%s", "pAnimTree") )
        {
            __debugbreak();
        }
        obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
        if ( obj )
        {
            if ( !DObjGetTree(obj)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                            1455,
                            0,
                            "%s",
                            "DObjGetTree( obj )") )
            {
                __debugbreak();
            }
            if ( DObjGetTree(obj) != pAnimTree
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                            1456,
                            0,
                            "%s",
                            "DObjGetTree( obj ) == pAnimTree") )
            {
                __debugbreak();
            }
            XAnimClearTreeGoalWeights(pAnimTree, 0, 0.0, -1);
            XAnimSetCompleteGoalWeight(obj, cgameGlob->bgs.animData->animScriptData.torsoAnim, 0.0, 0.0, 1.0, 0, 0, 0, -1);
            XAnimSetCompleteGoalWeight(obj, cgameGlob->bgs.animData->animScriptData.legsAnim, 1.0, 0.0, 1.0, 0, 0, 0, -1);
            XAnimSetCompleteGoalWeight(obj, cgameGlob->bgs.animData->animScriptData.turningAnim, 0.0, 0.0, 1.0, 0, 0, 0, -1);
            memset(&ci->legs, 0, sizeof(ci->legs));
            ci->legs.yawAngle = ci->playerAngles[1];
            ci->legs.yawing = 0;
            ci->legs.pitchAngle = 0.0f;
            ci->legs.pitching = 0;
            memset(&ci->torso, 0, sizeof(ci->torso));
            ci->torso.yawAngle = ci->playerAngles[1];
            ci->torso.yawing = 0;
            ci->torso.pitchAngle = ci->playerAngles[0];
            ci->torso.pitching = 0;
        }
    }
    if ( cg_debugPosition->current.enabled )
        Com_Printf(17, "%i ResetPlayerEntity yaw=%f\n", cent->nextState.number, ci->torso.yawAngle);
}

const char *__cdecl CG_GetTeamName(team_t team)
{
    const char *result; // eax
    const char *v2; // eax

    switch ( team )
    {
        case TEAM_FREE:
            result = "TEAM_FREE";
            break;
        case TEAM_AXIS:
            result = "TEAM_AXIS";
            break;
        case TEAM_ALLIES:
            result = "TEAM_ALLIES";
            break;
        case TEAM_SPECTATOR:
            result = "TEAM_SPECTATOR";
            break;
        default:
            v2 = va("Unhandled team index %i!", team);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp", 1501, 0, v2) )
                __debugbreak();
            result = "";
            break;
    }
    return result;
}

const char *__cdecl CG_GetFFATeamName(ffa_team_t team)
{
    const char *v2; // eax

    switch ( team )
    {
        case TEAM_FFA_NONE:
            return "TEAM_SPECTATOR";
        case TEAM_FFA_AXIS:
            return "TEAM_AXIS";
        case TEAM_FFA_ALLIES:
            return "TEAM_ALLIES";
    }
    v2 = va("Unhandled team index %i!", team);
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp", 1517, 0, v2) )
        __debugbreak();
    return "";
}

const char *__cdecl CG_GetOpposingTeamName(team_t team)
{
    const char *result; // eax
    const char *v2; // eax

    switch ( team )
    {
        case TEAM_FREE:
        case TEAM_SPECTATOR:
            result = CG_GetTeamName(team);
            break;
        case TEAM_AXIS:
            result = CG_GetTeamName(TEAM_ALLIES);
            break;
        case TEAM_ALLIES:
            result = CG_GetTeamName(TEAM_AXIS);
            break;
        default:
            v2 = va("Unhandled team index %i!", team);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp", 1535, 0, v2) )
                __debugbreak();
            result = "";
            break;
    }
    return result;
}

const char *__cdecl CG_GetPlayerTeamName(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]

    if ( CL_GetLocalClientConnectionState(localClientNum) < 9 )
        return CG_GetTeamName(TEAM_FREE);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                    1566,
                    0,
                    "cgameGlob->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cgameGlob->clientNum,
                    32) )
    {
        __debugbreak();
    }
    if ( cgameGlob->bgs.clientinfo[cgameGlob->clientNum].infoValid )
        return CG_GetTeamName(cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team);
    else
        return CG_GetTeamName(TEAM_FREE);
}

const char *__cdecl CG_GetPlayerOpposingTeamName(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]

    if ( CL_GetLocalClientConnectionState(localClientNum) < 9 )
        return CG_GetOpposingTeamName(TEAM_FREE);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                    1603,
                    0,
                    "cgameGlob->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cgameGlob->clientNum,
                    32) )
    {
        __debugbreak();
    }
    if ( cgameGlob->bgs.clientinfo[cgameGlob->clientNum].infoValid )
        return CG_GetOpposingTeamName(cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team);
    else
        return CG_GetOpposingTeamName(TEAM_FREE);
}

bool __cdecl CG_IsPlayerDead(int localClientNum)
{
    cg_s *cgameGlob; // [esp+4h] [ebp-8h]

    if ( CL_GetLocalClientConnectionState(localClientNum) < 9 )
        return 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                    1722,
                    0,
                    "cgameGlob->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cgameGlob->clientNum,
                    32) )
    {
        __debugbreak();
    }
    if ( !cgameGlob->bgs.clientinfo[cgameGlob->clientNum].infoValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                    1724,
                    0,
                    "%s",
                    "localPlayer->infoValid") )
    {
        __debugbreak();
    }
    return !cgameGlob->nextSnap->ps.stats[0]
            || (cgameGlob->nextSnap->ps.otherFlags & 4) == 0
            || (cgameGlob->nextSnap->ps.otherFlags & 2) != 0;
}

int __cdecl CG_GetPlayerClipAmmoCount(int localClientNum)
{
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    return BG_GetAmmoInClip(&cgameGlob->nextSnap->ps, cgameGlob->nextSnap->ps.weapon);
}

void __cdecl CG_UpdateWeaponVisibilityImmediate(int localClientNum, centity_s *cent)
{
    CG_UpdateWeaponVisibilityInternal(localClientNum, cent);
}


int hideCheckIntervalMin = 100;
int hideCheckIntervalMax = 200;
void __cdecl CG_UpdateWeaponVisibilityInternal(int localClientNum, centity_s *cent)
{
    int v2; // esi
    unsigned __int8 boneIndex; // [esp+7h] [ebp-55h] BYREF
    DObj *obj; // [esp+8h] [ebp-54h]
    float origin[3]; // [esp+Ch] [ebp-50h] BYREF
    cg_s *cgameGlob; // [esp+18h] [ebp-44h]
    int addKnife; // [esp+1Ch] [ebp-40h]
    entityState_s *p_nextState; // [esp+20h] [ebp-3Ch]
    unsigned int boneHandle; // [esp+24h] [ebp-38h]
    clientInfo_t *ci; // [esp+28h] [ebp-34h]
    XModel *weapModel; // [esp+2Ch] [ebp-30h]
    int addGrenade; // [esp+30h] [ebp-2Ch]
    float axis[3][3]; // [esp+34h] [ebp-28h] BYREF
    const WeaponDef *weapDef; // [esp+58h] [ebp-4h]

    if ( localClientNum >= 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                    2047,
                    0,
                    "%s",
                    "localClientNum < MAX_LOCAL_CLIENTS") )
    {
        __debugbreak();
    }
    if ( !cent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp", 2048, 0, "%s", "cent") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    p_nextState = &cent->nextState;
    if ( cent->nextState.clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                    2054,
                    0,
                    "es->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    p_nextState->clientNum,
                    32) )
    {
        __debugbreak();
    }
    ci = &cgameGlob->bgs.clientinfo[p_nextState->clientNum];
    if ( ci->iDObjWeapon )
    {
        weapDef = BG_GetWeaponDef(ci->iDObjWeapon);
        weapModel = weapDef->worldModel[ci->weaponModel];
        addKnife = BG_IsKnifeMeleeAnim(ci, cent->nextState.anim.torsoAnim);
        addGrenade = BG_IsGrenadeAnim(ci, cent->nextState.anim.torsoAnim);
        if ( !addGrenade )
            addGrenade = BG_IsGrenadeAnim(ci, cent->nextState.animState.state);
        if ( !addKnife || ci->usingKnife )
        {
            if ( addKnife || !ci->usingKnife )
            {
                if ( !addGrenade || ci->usingGrenade )
                {
                    if ( !addGrenade && ci->usingGrenade )
                    {
                        ci->usingGrenade = 0;
                        ci->dobjDirty = 1;
                    }
                }
                else
                {
                    ci->usingGrenade = 1;
                    ci->dobjDirty = 1;
                }
            }
            else
            {
                ci->usingKnife = 0;
                ci->dobjDirty = 1;
            }
        }
        else
        {
            ci->usingKnife = 1;
            ci->dobjDirty = 1;
        }
        if ( !ci->hideWeapon )
        {
            if ( ci->nextHideWeaponTime <= hideCheckIntervalMax + cgameGlob->time && ci->nextHideWeaponTime > cgameGlob->time )
                return;
            v2 = hideCheckIntervalMin + cgameGlob->time;
            ci->nextHideWeaponTime = rand() % (hideCheckIntervalMax - hideCheckIntervalMin) + v2;
        }
        if ( weapModel && (useFastFile->current.enabled || !XModelBad(weapModel)) )
        {
            boneHandle = CG_GetWeaponAttachBone(ci, weapDef->weapType, weapDef->inventoryType);
            obj = Com_GetClientDObj(p_nextState->number, localClientNum);
            boneIndex = -2;
            if ( DObjGetBoneIndex(obj, boneHandle, &boneIndex, -1) )
            {
                if ( CG_DObjGetWorldBoneMatrix(&cent->pose, obj, boneIndex, axis, origin) )
                {
                    if ( CG_IsWeaponVisible(localClientNum, cent, weapModel, origin, axis[0]) )
                    {
                        if ( ci->hideWeapon )
                        {
                            ci->hideWeapon = 0;
                            ci->dobjDirty = 1;
                        }
                    }
                    else if ( !ci->hideWeapon )
                    {
                        ci->hideWeapon = 1;
                        ci->dobjDirty = 1;
                    }
                }
            }
        }
    }
}

bool __cdecl CG_IsWeaponVisible(int localClientNum, centity_s *cent, XModel *weapModel, float *origin, float *forward)
{
    col_context_t v6; // [esp+Ch] [ebp-B8h] BYREF
    col_context_t context; // [esp+34h] [ebp-90h] BYREF
    float stock[3]; // [esp+5Ch] [ebp-68h] BYREF
    cg_s *cgameGlob; // [esp+68h] [ebp-5Ch]
    float end[3]; // [esp+6Ch] [ebp-58h] BYREF
    trace_t trace; // [esp+78h] [ebp-4Ch] BYREF
    float weapLen; // [esp+B4h] [ebp-10h] BYREF
    float eye[3]; // [esp+B8h] [ebp-Ch] BYREF

    if ( !weapModel
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp", 1996, 0, "%s", "weapModel") )
    {
        __debugbreak();
    }
    if ( !cent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp", 1997, 0, "%s", "cent") )
    {
        __debugbreak();
    }
    CG_CalcWeaponVisTrace(weapModel, origin, forward, stock, end, &weapLen);
    //col_context_t::col_context_t(&context);
    CG_TraceCapsule(&trace, stock, vec3_origin, vec3_origin, end, cent->nextState.number, 4097, &context);
    if ( !cg_drawWVisDebug
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp",
                    2006,
                    0,
                    "%s",
                    "cg_drawWVisDebug") )
    {
        __debugbreak();
    }
    if ( cg_drawWVisDebug->current.enabled )
    {
        if ( trace.fraction == 1.0 )
            CG_DebugLine(stock, end, colorRed, 1, 0);
        else
            CG_DebugLine(stock, end, colorYellow, 1, 0);
    }
    if ( (float)(weapLen - (float)(weapLen * trace.fraction)) <= 3.0 )
        return 1;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    eye[0] = cgameGlob->refdef.vieworg[0];
    eye[1] = cgameGlob->refdef.vieworg[1];
    eye[2] = cgameGlob->refdef.vieworg[2];
    //col_context_t::col_context_t(&v6);
    CG_TraceCapsule(&trace, eye, vec3_origin, vec3_origin, stock, cent->nextState.number, 4097, &v6);
    if ( cg_drawWVisDebug->current.enabled )
    {
        if ( trace.fraction == 1.0 )
            CG_DebugLine(eye, stock, colorGreen, 1, 0);
        else
            CG_DebugLine(eye, stock, colorBlue, 1, 0);
    }
    return trace.fraction == 1.0;
}

void __cdecl CG_CalcWeaponVisTrace(
                XModel *weapModel,
                float *origin,
                float *forward,
                float *start,
                float *end,
                float *modelLen)
{
    float v6; // [esp+0h] [ebp-24h]
    float mins[3]; // [esp+4h] [ebp-20h] BYREF
    float stockDist; // [esp+10h] [ebp-14h]
    float maxs[3]; // [esp+14h] [ebp-10h] BYREF
    const DObjAnimMat *baseMat; // [esp+20h] [ebp-4h]

    if ( !weapModel
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp", 1971, 0, "%s", "weapModel") )
    {
        __debugbreak();
    }
    if ( !modelLen
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp", 1972, 0, "%s", "modelLen") )
    {
        __debugbreak();
    }
    XModelGetBounds(weapModel, mins, maxs);
    baseMat = XModelGetBasePose(weapModel);
    stockDist = mins[0] - baseMat->trans[0];
    *modelLen = maxs[0] - mins[0];
    *start = (float)(stockDist * *forward) + *origin;
    start[1] = (float)(stockDist * forward[1]) + origin[1];
    start[2] = (float)(stockDist * forward[2]) + origin[2];
    v6 = *modelLen;
    *end = (float)(*modelLen * *forward) + *start;
    end[1] = (float)(v6 * forward[1]) + start[1];
    end[2] = (float)(v6 * forward[2]) + start[2];
}

float expand;
unsigned int _S2_1;
bool __cdecl CG_PlayerInViewFrustum(int localClientNum, const centity_s *cent)
{
    float center[3]; // [esp+8h] [ebp-Ch] BYREF

    if ( !cent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_players_mp.cpp", 2198, 0, "%s", "cent") )
    {
        __debugbreak();
    }
    if ( (_S2_1 & 1) == 0 )
    {
        _S2_1 |= 1u;
        expand = cg_playerFrustumHalfHeight->current.value * -1.0;
    }
    center[0] = cent->pose.origin[0];
    center[1] = cent->pose.origin[1];
    center[2] = cent->pose.origin[2];
    center[2] = center[2] + cg_playerFrustumHalfHeight->current.value;
    return R_CullPoint(localClientNum, center, expand) == 0;
}

