#include "cg_event.h"
#include "cg_scr_main.h"
#include <clientscript/cscr_vm.h>
#include <clientscript/scr_const.h>
#include "cg_world.h"
#include <glass/glass_client.h>
#include <qcommon/dobj_management.h>
#include <xanim/dobj_utils.h>
#include <EffectsCore/fx_system.h>
#include <client_mp/cl_cgame_mp.h>
#include <server_mp/sv_init_mp.h>
#include <clientscript/cscr_stringlist.h>
#include <cgame_mp/cg_main_mp.h>
#include <DynEntity/DynEntity_client.h>
#include <demo/demo_playback.h>
#include <bgame/bg_weapons_def.h>
#include "cg_sound.h"
#include <sound/snd_public_async.h>
#include <cgame_mp/cg_ents_mp.h>
#include <client_mp/cl_input_mp.h>
#include <sound/snd_bank.h>
#include <bgame/bg_weapons_ammo.h>
#include "offhandweapons.h"
#include <cgame_mp/cg_servercmds_mp.h>
#include "cg_camerashake.h"
#include <cgame_mp/cg_compassfriendlies_mp.h>
#include <gfx_d3d/r_water_sim.h>
#include <ragdoll/ragdoll_update.h>
#include <gfx_d3d/r_foliage.h>
#include <bgame/bg_fire.h>
#include <universal/com_math_anglevectors.h>
#include <client/splitscreen.h>
#include <stringed/stringed_hooks.h>
#include <cgame_mp/cg_vehicles_mp.h>
#include "cg_main.h"
#include "cg_spawn.h"
#include "cg_bolt.h"
#include <bgame/bg_wind.h>
#include <client_mp/cl_ui_mp.h>
#include <cgame_mp/cg_scoreboard_mp.h>
#include <cgame_mp/cg_draw_mp.h>
#include <client/cl_console.h>
#include <cgame_mp/cg_animscripted_mp.h>
#include <bgame/bg_mantle.h>
#include <cgame/cg_effects_load_obj.h>

void __cdecl CG_SetWetness(int localClientNum, int entNum, float wetness, int invert)
{
    centity_s *ent; // [esp+8h] [ebp-4h]

    ent = CG_GetEntity(localClientNum, entNum);
    if ( ent )
    {
        ent->pose.wetness = wetness;
        if (invert)
        {
            //LODWORD(ent->pose.wetness) ^= _mask__NegFloat_;
            ent->pose.wetness = -ent->pose.wetness;
        }
    }
}

float scale = 0.5f;

void __cdecl CG_ExplosionEvent(
                int localClientNum,
                float *origin,
                float damageInner,
                float damageOuter,
                float radius,
                int mod)
{
    float v6; // [esp+14h] [ebp-1F4h]
    float v7; // [esp+24h] [ebp-1E4h]
    float v8; // [esp+28h] [ebp-1E0h]
    float v9; // [esp+2Ch] [ebp-1DCh]
    unsigned int value; // [esp+38h] [ebp-1D0h]
    float force[3]; // [esp+44h] [ebp-1C4h] BYREF
    int j; // [esp+50h] [ebp-1B8h]
    int i; // [esp+54h] [ebp-1B4h]
    int entitylist[100]; // [esp+58h] [ebp-1B0h] BYREF
    float mins[3]; // [esp+1E8h] [ebp-20h] BYREF
    centity_s *cent; // [esp+1F4h] [ebp-14h]
    int numEnts; // [esp+1F8h] [ebp-10h]
    float maxs[3]; // [esp+1FCh] [ebp-Ch] BYREF
    int savedregs; // [esp+208h] [ebp+0h] BYREF

    mins[0] = *origin - radius;
    mins[1] = origin[1] - radius;
    mins[2] = origin[2] - radius;
    maxs[0] = *origin + radius;
    maxs[1] = origin[1] + radius;
    maxs[2] = origin[2] + radius;
    for ( i = 0; i < 32; ++i )
    {
        cent = CG_GetEntity(localClientNum, i);
        if ( ((*((unsigned int *)cent + 201) >> 1) & 1) != 0 )
        {
            value = CScr_GetMeansOfDeathConstString(mod);
            Scr_AddConstString(value, SCRIPTINSTANCE_CLIENT);
            Scr_AddVector(origin, SCRIPTINSTANCE_CLIENT);
            Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
            CScr_Notify(localClientNum, cent, cscr_const.explode, 3u);
        }
    }
    numEnts = CG_AreaEntities(mins, maxs, entitylist, 100, -1);
    for ( j = 0; j < numEnts; ++j )
    {
        cent = CG_GetEntity(localClientNum, entitylist[j]);
        if ( cent->destructible )
        {
            CG_DestructibleRadiusDamage(cent, (const centity_s **)origin, damageInner, damageOuter, radius, mod, 1);
        }
        else if ( cent->pose.physObjId )
        {
            v7 = flrand(-1.0, 1.0);
            v8 = flrand(-1.0, 1.0);
            v9 = flrand(-1.0, 1.0);
            force[0] = v7;
            force[1] = v8;
            force[2] = v9;
            Vec3Normalize(force);
            v6 = scale * dynEnt_explodeForce->current.value;
            force[0] = v6 * force[0];
            force[1] = v6 * force[1];
            force[2] = v6 * force[2];
            Phys_ObjAddForce(cent->pose.physObjId, origin, force, 0);
        }
    }
    GlassCl_ExplosionEvent(localClientNum, origin, damageInner, damageOuter, radius, mod);
}

int __cdecl CG_GetBoneIndex(
                int localClientNum,
                unsigned int dobjHandle,
                unsigned int boneName,
                unsigned __int8 *boneIndex)
{
    const DObj *obj; // [esp+0h] [ebp-4h]

    if ( dobjHandle >= 0x601
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                    516,
                    0,
                    "dobjHandle doesn't index CLIENT_DOBJ_HANDLE_MAX\n\t%i not in [0, %i)",
                    dobjHandle,
                    1537) )
    {
        __debugbreak();
    }
    obj = Com_GetClientDObj(dobjHandle, localClientNum);
    if ( obj )
        return DObjGetBoneIndex(obj, boneName, boneIndex, -1);
    else
        return 0;
}

unsigned int __cdecl CG_PlayBoltedEffect(
                int localClientNum,
                const FxEffectDef *fxDef,
                int dobjHandle,
                unsigned int boneName)
{
    cg_s *LocalClientGlobals; // eax
    unsigned __int8 boneIndex; // [esp+3h] [ebp-5h] BYREF
    int time; // [esp+4h] [ebp-4h]

    boneIndex = -2;
    if ( boneName && CG_GetBoneIndex(localClientNum, dobjHandle, boneName, &boneIndex) )
    {
        time = CG_GetLocalClientGlobals(localClientNum)->time;
        return FX_PlayBoltedEffect(localClientNum, fxDef, time, dobjHandle, boneIndex);
    }
    else
    {
        LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
        return FX_PlayBoltedEffect(localClientNum, fxDef, LocalClientGlobals->time, dobjHandle, 511);
    }
}

void __cdecl CG_PlayFxOnTag(int localClientNum, centity_s *cent, int eventParm)
{
    unsigned int ConfigstringConst; // eax
    char *v4; // eax
    const char *v5; // eax
    unsigned int v6; // eax
    char *v7; // eax
    const char *v8; // eax
    unsigned __int16 tagName; // [esp+0h] [ebp-1Ch] BYREF
    int dobjHandle; // [esp+4h] [ebp-18h]
    const char *tagAndEffect; // [esp+8h] [ebp-14h]
    const FxEffectDef *fxDef; // [esp+Ch] [ebp-10h]
    const cgs_t *cgs; // [esp+10h] [ebp-Ch]
    int fxId; // [esp+14h] [ebp-8h]
    int csIndex; // [esp+18h] [ebp-4h]

    csIndex = eventParm + 2276;
    tagAndEffect = CL_GetConfigString(eventParm + 2276);
    if ( !Demo_IsPlaying() || I_strcmp(tagAndEffect, "") && I_strcmp(tagAndEffect + 1, "") )
    {
        if ( !*tagAndEffect )
        {
            ConfigstringConst = SV_GetConfigstringConst(csIndex);
            v4 = SL_ConvertToString(ConfigstringConst, SCRIPTINSTANCE_SERVER);
            v5 = va("tagAndEffect: '%s', server: '%s'", tagAndEffect, v4);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                            607,
                            0,
                            "%s\n\t%s",
                            "tagAndEffect[0]",
                            v5) )
                __debugbreak();
        }
        if ( !tagAndEffect[1] )
        {
            v6 = SV_GetConfigstringConst(csIndex);
            v7 = SL_ConvertToString(v6, SCRIPTINSTANCE_SERVER);
            v8 = va("tagAndEffect: '%s', server: '%s'", tagAndEffect, v7);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                            608,
                            0,
                            "%s\n\t%s",
                            "tagAndEffect[1]",
                            v8) )
                __debugbreak();
        }
        fxId = 10 * (tagAndEffect[1] - 48) + 100 * (*tagAndEffect - 48) + tagAndEffect[2] - 48;
        if ( Demo_IsPlaying() && (fxId <= 0 || fxId > 196) )
        {
            Com_PrintWarning(14, "FX id went beyond limit. TagAndEffect is %s\n", tagAndEffect);
        }
        else
        {
            if ( (fxId <= 0 || fxId >= 196)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                            617,
                            0,
                            "%s",
                            "fxId > 0 && fxId < MAX_EFFECT_NAMES") )
            {
                __debugbreak();
            }
            cgs = CG_GetLocalClientStaticGlobals(localClientNum);
            fxDef = cgs->fxs[fxId];
            if ( !fxDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp", 622, 0, "%s", "fxDef") )
                __debugbreak();
            dobjHandle = cent->nextState.number;
            tagName = SL_GetString((char *)tagAndEffect + 3, 0, SCRIPTINSTANCE_SERVER);
            CG_PlayBoltedEffect(localClientNum, fxDef, dobjHandle, tagName);
            Scr_SetString(&tagName, 0, SCRIPTINSTANCE_SERVER);
        }
    }
    else
    {
        Com_PrintWarning(14, "Didnt play fx for configStringIndex %d. Since the tag wasnt found.\n", csIndex);
    }
}

void __cdecl CG_DoFootstepEvent(
                int localClientNum,
                centity_s *cent,
                bool isPlayerView,
                int event,
                int surfaceType,
                bool quiet)
{
    if ( (CG_GetLocalClientGlobals(localClientNum)->nextSnap->ps.bobCycle & 0x7Fu) <= 0x3F )
    {
        if ( cg_footsteps->current.enabled )
            CScr_PlayerFootstepEvent(localClientNum, cent, isPlayerView, event, surfaceType, quiet);
        //BLOPS_NULLSUB();
        //BLOPS_NULLSUB();
    }
}

void __cdecl CG_DoJumpEvent(int localClientNum, centity_s *cent, bool isPlayerView, int surfaceType, bool quiet)
{
    CScr_PlayerJumpEvent(localClientNum, cent, isPlayerView, surfaceType, quiet);
    //BLOPS_NULLSUB();
}

void __cdecl CG_DoLandEvent(
                int localClientNum,
                centity_s *cent,
                bool isPlayerView,
                int surfaceType,
                bool quiet,
                bool damagePlayer)
{
    CScr_PlayerLandEvent(localClientNum, cent, isPlayerView, surfaceType, quiet, damagePlayer);
    //BLOPS_NULLSUB();
}

void __cdecl CG_DoFoliageEvent(int localClientNum, centity_s *cent, bool isPlayerView, bool quiet)
{
    CScr_PlayerFoliageEvent(localClientNum, cent, isPlayerView, quiet);
}

void __cdecl CG_EntityEvent(int localClientNum, centity_s *cent, int event)
{
    float *recoilAngles; // ecx
    float *recoilSpeed; // eax
    unsigned int AliasId; // eax
    unsigned int v7; // eax
    unsigned int v8; // eax
    cg_s *LocalClientGlobals; // eax
    unsigned int v10; // eax
    cg_s *v11; // eax
    float *v12; // edx
    float *v13; // ecx
    unsigned int v14; // eax
    float *v15; // ecx
    float *v16; // eax
    int WeaponIndexForName; // eax
    const WeaponDef *v18; // eax
    unsigned int v19; // eax
    unsigned int v20; // eax
    unsigned int v21; // eax
    centity_s *v22; // edx
    centity_s *v23; // edx
    unsigned int v24; // eax
    unsigned int v25; // eax
    unsigned int v26; // eax
    unsigned int v27; // eax
    unsigned int v28; // eax
    unsigned int v29; // eax
    unsigned int v30; // eax
    unsigned int v31; // eax
    centity_s *Entity; // eax
    centity_s *v33; // eax
    unsigned int v34; // eax
    unsigned int v35; // eax
    DObj *p; // [esp+Ch] [ebp-408h]
    unsigned int p_4; // [esp+10h] [ebp-404h]
    unsigned int attenuation_4; // [esp+18h] [ebp-3FCh]
    unsigned int attenuation_4a; // [esp+18h] [ebp-3FCh]
    unsigned int v40; // [esp+2Ch] [ebp-3E8h]
    char *v41; // [esp+30h] [ebp-3E4h]
    char *v42; // [esp+34h] [ebp-3E0h]
    unsigned int dtpLaunchSoundPlayer; // [esp+3Ch] [ebp-3D8h]
    float Height; // [esp+54h] [ebp-3C0h]
    unsigned int mantleSoundPlayer; // [esp+D0h] [ebp-344h]
    bool v52; // [esp+E4h] [ebp-330h]
    float *v53; // [esp+124h] [ebp-2F0h]
    snapshot_s *v54; // [esp+244h] [ebp-1D0h]
    snapshot_s *v55; // [esp+248h] [ebp-1CCh]
    snapshot_s *v56; // [esp+24Ch] [ebp-1C8h]
    snapshot_s *v57; // [esp+250h] [ebp-1C4h]
    snapshot_s *v58; // [esp+254h] [ebp-1C0h]
    snapshot_s *v59; // [esp+258h] [ebp-1BCh]
    snapshot_s *nextSnap; // [esp+260h] [ebp-1B4h]
    SndEntHandle ent; // [esp+268h] [ebp-1ACh]
    fake_centity_s *v62; // [esp+26Ch] [ebp-1A8h]
    fake_centity_s *fent; // [esp+270h] [ebp-1A4h]
    const FxEffectDef *fxDef; // [esp+280h] [ebp-194h]
    float v65[3]; // [esp+284h] [ebp-190h] BYREF
    XModel *model; // [esp+290h] [ebp-184h]
    float quat[4]; // [esp+294h] [ebp-180h] BYREF
    float force[3]; // [esp+2A4h] [ebp-170h] BYREF
    cgs_t *cgs; // [esp+2B0h] [ebp-164h]
    int fxId; // [esp+2B4h] [ebp-160h]
    float hitpos[3]; // [esp+2B8h] [ebp-15Ch] BYREF
    unsigned __int16 id; // [esp+2C4h] [ebp-150h]
    centity_s *v73; // [esp+2C8h] [ebp-14Ch]
    SndEntHandle sndEnt; // [esp+2CCh] [ebp-148h]
    float origin[3]; // [esp+2D0h] [ebp-144h] BYREF
    float orientation[3][3]; // [esp+2DCh] [ebp-138h] BYREF
    const char *tagName; // [esp+300h] [ebp-114h]
    DObj *obj; // [esp+304h] [ebp-110h]
    float soundPosition[3]; // [esp+308h] [ebp-10Ch] BYREF
    centity_s *attachedCorpse; // [esp+314h] [ebp-100h]
    const FxEffectDef *expFx; // [esp+318h] [ebp-FCh]
    float innerRadius; // [esp+31Ch] [ebp-F8h]
    unsigned int v83; // [esp+320h] [ebp-F4h] BYREF
    const FxEffectDef *v84; // [esp+324h] [ebp-F0h] BYREF
    int mod; // [esp+328h] [ebp-ECh]
    unsigned int v86; // [esp+32Ch] [ebp-E8h] BYREF
    const FxEffectDef *def; // [esp+330h] [ebp-E4h] BYREF
    float explosionRadius; // [esp+334h] [ebp-E0h]
    unsigned int outSnd; // [esp+338h] [ebp-DCh] BYREF
    float waterZ; // [esp+33Ch] [ebp-D8h]
    unsigned __int8 surfType; // [esp+343h] [ebp-D1h]
    const FxEffectDef *outFx; // [esp+344h] [ebp-D0h] BYREF
    bool waterSimEnabled; // [esp+34Bh] [ebp-C9h]
    PhysObjUserData *m_phys_user_data; // [esp+34Ch] [ebp-C8h]
    centity_s *v95; // [esp+350h] [ebp-C4h]
    float buoyancy_scale; // [esp+354h] [ebp-C0h]
    PhysObjUserData *userData; // [esp+358h] [ebp-BCh]
    centity_s *v98; // [esp+35Ch] [ebp-B8h]
    int flashIndex; // [esp+368h] [ebp-ACh]
    vehicle_info_t *info; // [esp+36Ch] [ebp-A8h]
    int weapon; // [esp+370h] [ebp-A4h]
    int gunnerIndex; // [esp+374h] [ebp-A0h]
    centity_s *v103; // [esp+378h] [ebp-9Ch]
    int v104; // [esp+37Ch] [ebp-98h]
    int v105; // [esp+380h] [ebp-94h]
    centity_s *attackerCent; // [esp+384h] [ebp-90h]
    entityType_t eType; // [esp+388h] [ebp-8Ch]
    const WeaponDef *weapDefDW; // [esp+38Ch] [ebp-88h]
    const WeaponDef *itemWeapDef; // [esp+390h] [ebp-84h]
    int index; // [esp+394h] [ebp-80h]
    const WeaponDef *WeapDef; // [esp+398h] [ebp-7Ch]
    unsigned int snd; // [esp+39Ch] [ebp-78h] BYREF
    const FxEffectDef *fx; // [esp+3A0h] [ebp-74h] BYREF
    float fallHeight; // [esp+3A4h] [ebp-70h]
    float dir[3]; // [esp+3A8h] [ebp-6Ch] BYREF
    bool isPlayerView; // [esp+3B7h] [ebp-5Dh]
    cg_s *cgameGlob; // [esp+3B8h] [ebp-5Ch]
    int viewDip; // [esp+3BCh] [ebp-58h]
    const WeaponDef *weaponDef; // [esp+3C0h] [ebp-54h]
    int offset; // [esp+3C4h] [ebp-50h]
    bool quiet; // [esp+3CBh] [ebp-49h]
    const entityState_s *p_nextState; // [esp+3CCh] [ebp-48h]
    int eventParm; // [esp+3D0h] [ebp-44h]
    const WeaponDef *meleeWeaponDef; // [esp+3D4h] [ebp-40h]
    unsigned int meleeWeapon; // [esp+3D8h] [ebp-3Ch]
    float axis[3][3]; // [esp+3DCh] [ebp-38h] BYREF
    int clientNum; // [esp+400h] [ebp-14h]
    float *position; // [esp+404h] [ebp-10h]
    const WeaponVariantDef *weaponVariantDef; // [esp+408h] [ebp-Ch]
    const playerState_s *ps; // [esp+40Ch] [ebp-8h]
    unsigned int weaponIdx; // [esp+410h] [ebp-4h]
    centity_s *centa; // [esp+420h] [ebp+Ch]

    if (event)
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        position = cent->pose.origin;
        ps = &cgameGlob->nextSnap->ps;
        p_nextState = &cent->nextState;
        eventParm = cent->nextState.eventParm;
        nextSnap = cgameGlob->nextSnap;
        v52 = (nextSnap->ps.otherFlags & 6) != 0 && p_nextState->number == nextSnap->ps.clientNum;
        isPlayerView = v52;
        if (cg_debugEvents->current.enabled)
            Com_Printf(21, "ent:%3i  event:%3i ", p_nextState->number, event);
        if (event <= 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                881,
                0,
                "%s\n\t(event) = %i",
                "(event > 0)",
                event))
        {
            __debugbreak();
        }
        if (event >= 206
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                882,
                0,
                "%s\n\t(event) = %i",
                "(event < EV_MAX_EVENTS)",
                event))
        {
            __debugbreak();
        }
        if (cg_debugEvents->current.enabled)
            Com_Printf(21, "CG_EntityEvent:%s\n", eventnames[event]);
        if (isPlayerView)
        {
            weaponIdx = cgameGlob->predictedPlayerState.weapon;
            if (BG_IsUseAsMeleeWeapon(weaponIdx))
                meleeWeapon = p_nextState->weapon;
            else
                meleeWeapon = cgameGlob->predictedPlayerState.meleeWeapon;
        }
        else
        {
            weaponIdx = p_nextState->weapon;
            if (p_nextState->eType != 1 || BG_IsUseAsMeleeWeapon(p_nextState->weapon))
                meleeWeapon = p_nextState->weapon;
            else
                meleeWeapon = p_nextState->lerp.u.player.meleeWeapon;
        }
        if (Demo_IsMovieCamera())
            isPlayerView = 0;
        clientNum = p_nextState->clientNum;
        if (clientNum >= com_maxclients->current.integer)
            clientNum = 0;
        weaponVariantDef = BG_GetWeaponVariantDef(weaponIdx);
        weaponDef = BG_GetWeaponDef(weaponIdx);
        meleeWeaponDef = BG_GetWeaponDef(meleeWeapon);
        quiet = (cgameGlob->bgs.clientinfo[clientNum].perks[1] & 0x40) != 0;
        offset = quiet ? 0x1F : 0;
        if (event < 111 || event > 141)
        {
            if (event < 142 || event > 172)
            {
                switch (event)
                {
                case EV_FOLIAGE_SOUND:
                    CG_DoFoliageEvent(localClientNum, cent, isPlayerView, quiet);
                    return;
                case EV_STOP_WEAPON_SOUND:
                    CG_StopWeaponSound(localClientNum, isPlayerView, weaponDef, p_nextState->number, (weaponstate_t)eventParm);
                    return;
                case EV_STOP_SOUND_ALIAS:
                    ent.field = CG_SndEntHandle(localClientNum, p_nextState->otherEntityNum).field;
                    SND_StopSoundAliasOnEnt(ent, p_nextState->loopSoundId);
                    return;
                case EV_SOUND_ALIAS:
                case EV_SOUND_ALIAS_NOTIFY:
                    nanassertvec3(p_nextState->lerp.pos.trBase);

                    //if (((LODWORD(p_nextState->lerp.pos.trBase[0]) & 0x7F800000) == 0x7F800000
                    //    || (LODWORD(p_nextState->lerp.pos.trBase[1]) & 0x7F800000) == 0x7F800000
                    //    || (LODWORD(p_nextState->lerp.pos.trBase[2]) & 0x7F800000) == 0x7F800000)
                    //    && !Assert_MyHandler(
                    //        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                    //        1917,
                    //        0,
                    //        "%s",
                    //        "!IS_NAN((es->lerp.pos.trBase)[0]) && !IS_NAN((es->lerp.pos.trBase)[1]) && !IS_NAN((es->lerp.pos.trBase)[2])"))
                    //{
                    //    __debugbreak();
                    //}
                    soundPosition[0] = p_nextState->lerp.pos.trBase[0];
                    soundPosition[1] = p_nextState->lerp.pos.trBase[1];
                    soundPosition[2] = p_nextState->lerp.pos.trBase[2];
                    if (p_nextState->un3.item)
                    {
                        obj = Com_GetClientDObj(p_nextState->otherEntityNum, localClientNum);
                        tagName = CL_GetConfigString(p_nextState->un3.item + 3115);
                        if (obj)
                        {
                            p_4 = SL_FindString(tagName, SCRIPTINSTANCE_SERVER);
                            p = obj;
                            Entity = CG_GetEntity(localClientNum, p_nextState->otherEntityNum);
                            if (CG_DObjGetWorldTagMatrix(&Entity->pose, p, p_4, orientation, origin))
                            {
                                soundPosition[0] = origin[0];
                                soundPosition[1] = origin[1];
                                soundPosition[2] = origin[2];
                            }
                            else
                            {
                                Com_Printf(
                                    9,
                                    "playsoundontag could not find tag %s on ent %d\n",
                                    tagName,
                                    p_nextState->otherEntityNum);
                            }
                        }
                        else
                        {
                            Com_Printf(9, "playsoundontag could not find dobj for ent %d\n", p_nextState->otherEntityNum);
                        }
                    }
                    CG_PlaySound(
                        localClientNum,
                        p_nextState->otherEntityNum,
                        soundPosition,
                        0,
                        event == 5,
                        1.0,
                        p_nextState->loopSoundId);
                    return;
                case EV_SOUND_BATTLECHAT_ALIAS:
                    CG_PlayBattleChatter(
                        localClientNum,
                        p_nextState->number,
                        (float*)p_nextState->lerp.pos.trBase,
                        p_nextState->loopSoundId,
                        p_nextState->un3.secondBcAlias);
                    return;
                case EV_STOPSOUNDS:
                    sndEnt.field = CG_SndEntHandle(localClientNum, p_nextState->number).field;
                    SND_StopSoundsOnEnt(sndEnt);
                    CG_SndKillAutoSimEnt(sndEnt);
                    return;
                case EV_STANCE_FORCE_STAND:
                    if (clientNum != cgameGlob->predictedPlayerState.clientNum)
                        goto LABEL_290;
                    CL_SetStance(localClientNum, CL_STANCE_STAND);
                    return;
                case EV_STANCE_FORCE_CROUCH:
                    if (clientNum != cgameGlob->predictedPlayerState.clientNum)
                        goto LABEL_290;
                    CL_SetStance(localClientNum, CL_STANCE_CROUCH);
                    return;
                case EV_STANCE_FORCE_PRONE:
                    if (clientNum != cgameGlob->predictedPlayerState.clientNum)
                        goto LABEL_290;
                    CL_SetStance(localClientNum, CL_STANCE_PRONE);
                    WeapDef = BG_GetWeaponDef(ps->weapon);
                    if (WeapDef->blocksProne && WeapDef->bIsCarriedKillstreakWeapon)
                    {
                        if (cgameGlob->weaponLatestPrimaryIdx)
                            CG_SelectWeaponIndex(localClientNum, cgameGlob->weaponLatestPrimaryIdx);
                        else
                            CycleWeapPrimary(localClientNum, 1, 1);
                    }
                    return;
                case EV_ITEM_PICKUP:
                case EV_AMMO_PICKUP:
                    index = p_nextState->eventParm;
                    if (index < 1 || index >= 2048)
                        return;
                    itemWeapDef = BG_GetWeaponDef(index);
                    if (event == 11)
                    {
                        recoilAngles = cgameGlob->recoilAngles;
                        cgameGlob->recoilAngles[0] = 0.0;
                        recoilAngles[1] = 0.0;
                        recoilAngles[2] = 0.0;
                        recoilSpeed = cgameGlob->recoilSpeed;
                        cgameGlob->recoilSpeed[0] = 0.0;
                        recoilSpeed[1] = 0.0;
                        recoilSpeed[2] = 0.0;
                        if (isPlayerView)
                            AliasId = SND_FindAliasId((char *)itemWeapDef->pickupSoundPlayer);
                        else
                            AliasId = SND_FindAliasId((char *)itemWeapDef->pickupSound);
                    }
                    else
                    {
                        if (event != 12)
                            goto LABEL_85;
                        if (isPlayerView)
                            AliasId = SND_FindAliasId((char *)itemWeapDef->ammoPickupSoundPlayer);
                        else
                            AliasId = SND_FindAliasId((char *)itemWeapDef->ammoPickupSound);
                    }
                    CG_PlaySound(localClientNum, p_nextState->number, 0, 0, 0, 1.0, AliasId);
                LABEL_85:
                    if (isPlayerView)
                        CG_ItemPickup(localClientNum, index);
                    break;
                case EV_NOAMMO:
                    if (!BG_WeaponIsClipOnly(weaponIdx) && !weaponDef->cancelAutoHolsterWhenEmpty)
                    {
                        if (isPlayerView)
                            v8 = SND_FindAliasId((char *)weaponDef->emptyFireSoundPlayer);
                        else
                            v8 = SND_FindAliasId((char *)weaponDef->emptyFireSound);
                        CG_PlaySound(localClientNum, p_nextState->number, 0, 0, 0, 1.0, v8);
                    }
                    if (isPlayerView)
                    {
                        if (!weaponDef->bDualWield
                            || (LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum),
                                CG_CheckIfDualWieldEmpty(&LocalClientGlobals->predictedPlayerState, 0)))
                        {
                            if (!weaponDef->bDualWield)
                                CG_OutOfAmmoChange(localClientNum);
                        }
                        else
                        {
                            CG_OutOfAmmoChange(localClientNum);
                        }
                    }
                    return;
                case EV_NOAMMO_LEFT:
                    if (!BG_WeaponIsClipOnly(weaponIdx) && !weaponDef->cancelAutoHolsterWhenEmpty)
                    {
                        weapDefDW = BG_GetWeaponDef(weaponDef->dualWieldWeaponIndex);
                        if (isPlayerView)
                            v10 = SND_FindAliasId((char *)weapDefDW->emptyFireSoundPlayer);
                        else
                            v10 = SND_FindAliasId((char *)weapDefDW->emptyFireSound);
                        CG_PlaySound(localClientNum, p_nextState->number, 0, 0, 0, 1.0, v10);
                    }
                    if (isPlayerView)
                    {
                        v11 = CG_GetLocalClientGlobals(localClientNum);
                        if (!CG_CheckIfDualWieldEmpty(&v11->predictedPlayerState, 0))
                            CG_OutOfAmmoChange(localClientNum);
                    }
                    return;
                case EV_EMPTYCLIP:
                case EV_RELOAD_START_NOTIFY:
                case EV_RELOAD_ADDAMMO:
                case EV_GRENADE_DROP:
                case EV_GRENADE_SUICIDE:
                case EV_JAM_WEAPON:
                case EV_ALLOWPITCH:
                    return;
                case EV_EMPTY_OFFHAND:
                    if (clientNum != cgameGlob->predictedPlayerState.clientNum)
                        goto LABEL_290;
                    if (isPlayerView)
                    {
                        CG_MenuShowNotify(localClientNum, 5);
                        CG_SwitchOffHandCmd(localClientNum);
                    }
                    return;
                case EV_RESET_ADS:
                    if (clientNum != cgameGlob->predictedPlayerState.clientNum)
                        goto LABEL_290;
                    if (isPlayerView)
                        CL_SetADS(localClientNum, 0);
                    return;
                case EV_RELOAD:
                    if (isPlayerView)
                        v7 = SND_FindAliasId((char *)weaponDef->reloadSoundPlayer);
                    else
                        v7 = SND_FindAliasId((char *)weaponDef->reloadSound);
                    goto LABEL_570;
                case EV_RELOAD_FROM_EMPTY:
                    if (isPlayerView)
                        v7 = SND_FindAliasId((char *)weaponDef->reloadEmptySoundPlayer);
                    else
                        v7 = SND_FindAliasId((char *)weaponDef->reloadEmptySound);
                    goto LABEL_570;
                case EV_RELOAD_START:
                    if (isPlayerView)
                        v7 = SND_FindAliasId((char *)weaponDef->reloadStartSoundPlayer);
                    else
                        v7 = SND_FindAliasId((char *)weaponDef->reloadStartSound);
                    goto LABEL_570;
                case EV_RELOAD_END:
                    if (isPlayerView)
                        v7 = SND_FindAliasId((char *)weaponDef->reloadEndSoundPlayer);
                    else
                        v7 = SND_FindAliasId((char *)weaponDef->reloadEndSound);
                    goto LABEL_570;
                case EV_RAISE_WEAPON:
                    v15 = cgameGlob->recoilAngles;
                    cgameGlob->recoilAngles[0] = 0.0;
                    v15[1] = 0.0;
                    v15[2] = 0.0;
                    v16 = cgameGlob->recoilSpeed;
                    cgameGlob->recoilSpeed[0] = 0.0;
                    v16[1] = 0.0;
                    v16[2] = 0.0;
                    if (isPlayerView)
                        v14 = SND_FindAliasId((char *)weaponDef->raiseSoundPlayer);
                    else
                        v14 = SND_FindAliasId((char *)weaponDef->raiseSound);
                    goto LABEL_180;
                case EV_FIRST_RAISE_WEAPON:
                    v12 = cgameGlob->recoilAngles;
                    cgameGlob->recoilAngles[0] = 0.0;
                    v12[1] = 0.0;
                    v12[2] = 0.0;
                    v13 = cgameGlob->recoilSpeed;
                    cgameGlob->recoilSpeed[0] = 0.0;
                    v13[1] = 0.0;
                    v13[2] = 0.0;
                    if (isPlayerView)
                        v14 = SND_FindAliasId((char *)weaponDef->firstRaiseSoundPlayer);
                    else
                        v14 = SND_FindAliasId((char *)weaponDef->firstRaiseSound);
                LABEL_180:
                    CG_PlaySound(localClientNum, p_nextState->number, 0, 0, 0, 1.0, v14);
                    cgameGlob->weaponForcedSelect = 0;
                    return;
                case EV_PUTAWAY_WEAPON:
                    if (isPlayerView)
                        v7 = SND_FindAliasId((char *)weaponDef->putawaySoundPlayer);
                    else
                        v7 = SND_FindAliasId((char *)weaponDef->putawaySound);
                    goto LABEL_570;
                case EV_WEAPON_ALT:
                    if (isPlayerView)
                        v7 = SND_FindAliasId((char *)weaponDef->altSwitchSoundPlayer);
                    else
                        v7 = SND_FindAliasId((char *)weaponDef->altSwitchSound);
                    goto LABEL_570;
                case EV_PULLBACK_WEAPON:
                    if (isPlayerView)
                        v7 = SND_FindAliasId((char *)weaponDef->pullbackSoundPlayer);
                    else
                        v7 = SND_FindAliasId((char *)weaponDef->pullbackSound);
                    goto LABEL_570;
                case EV_FIRE_WEAPON:
                case EV_FIRE_WEAPON_LASTSHOT:
                    if (((*((_DWORD *)cent + 201) >> 1) & 1) != 0 && cent->nextState.eType == 17 || cent->nextState.eType == 1)
                    {
                        Scr_AddConstString(cscr_const.face_shoot_single, SCRIPTINSTANCE_CLIENT);
                        CScr_Notify(localClientNum, cent, cscr_const.face, 1u);
                    }
                    CScr_NotifyNum(localClientNum, p_nextState->number, 0, cscr_const.face_shoot_single, 0);
                    CG_FireWeapon(localClientNum, cent, event, scr_const.tag_flash, 0, &cgameGlob->predictedPlayerState, 0);
                    return;
                case EV_FIRE_WEAPON_LEFT:
                case EV_FIRE_WEAPON_LASTSHOT_LEFT:
                    if (((*((_DWORD *)cent + 201) >> 1) & 1) != 0 && cent->nextState.eType == 17 || cent->nextState.eType == 1)
                    {
                        Scr_AddConstString(cscr_const.face_shoot_single, SCRIPTINSTANCE_CLIENT);
                        CScr_Notify(localClientNum, cent, cscr_const.face, 1u);
                    }
                    CScr_NotifyNum(localClientNum, p_nextState->number, 0, cscr_const.face_shoot_single, 0);
                    v18 = BG_GetWeaponDef(cgameGlob->predictedPlayerState.weapon);
                    CG_FireWeapon(
                        localClientNum,
                        cent,
                        event,
                        scr_const.tag_flash1,
                        v18->dualWieldWeaponIndex,
                        &cgameGlob->predictedPlayerState,
                        1);
                    return;
                case EV_RECHAMBER_WEAPON:
                    if (isPlayerView)
                        v7 = SND_FindAliasId((char *)weaponDef->rechamberSoundPlayer);
                    else
                        v7 = SND_FindAliasId((char *)weaponDef->rechamberSound);
                    goto LABEL_570;
                case EV_EJECT_BRASS:
                    CG_EjectWeaponBrass(localClientNum, p_nextState, event, p_nextState->weapon);
                    return;
                case EV_MELEE_SWIPE:
                    if (((*((_DWORD *)cent + 201) >> 1) & 1) != 0 && cent->nextState.eType == 17 || cent->nextState.eType == 1)
                    {
                        Scr_AddConstString(cscr_const.face_melee, SCRIPTINSTANCE_CLIENT);
                        CScr_Notify(localClientNum, cent, cscr_const.face, 1u);
                    }
                    CScr_NotifyNum(localClientNum, p_nextState->number, 0, cscr_const.face_melee, 0);
                    if (isPlayerView)
                        v7 = SND_FindAliasId((char *)meleeWeaponDef->meleeSwipeSoundPlayer);
                    else
                        v7 = SND_FindAliasId((char *)meleeWeaponDef->meleeSwipeSound);
                    goto LABEL_570;
                case EV_FIRE_MELEE:
                    if (((*((_DWORD *)cent + 201) >> 1) & 1) != 0 && cent->nextState.eType == 17 || cent->nextState.eType == 1)
                    {
                        Scr_AddConstString(cscr_const.face_melee, SCRIPTINSTANCE_CLIENT);
                        CScr_Notify(localClientNum, cent, cscr_const.face, 1u);
                    }
                    CScr_NotifyNum(localClientNum, p_nextState->number, 0, cscr_const.face_melee, 0);
                    DynEntCl_MeleeEvent(localClientNum, p_nextState->number);
                    GlassCl_MeleeEvent(localClientNum, p_nextState->number);
                    return;
                case EV_WEAPON_DEPLOYING:
                    if (isPlayerView)
                        v7 = SND_FindAliasId((char *)weaponDef->deploySoundPlayer);
                    else
                        v7 = SND_FindAliasId((char *)weaponDef->deploySound);
                    goto LABEL_570;
                case EV_WEAPON_FINISH_DEPLOYING:
                    if (isPlayerView)
                        v7 = SND_FindAliasId((char *)weaponDef->finishDeploySoundPlayer);
                    else
                        v7 = SND_FindAliasId((char *)weaponDef->finishDeploySound);
                    goto LABEL_570;
                case EV_WEAPON_BREAKING_DOWN:
                    if (isPlayerView)
                        v7 = SND_FindAliasId((char *)weaponDef->breakdownSoundPlayer);
                    else
                        v7 = SND_FindAliasId((char *)weaponDef->breakdownSound);
                    goto LABEL_570;
                case EV_WEAPON_FINISH_BREAKING_DOWN:
                    if (isPlayerView)
                        v7 = SND_FindAliasId((char *)weaponDef->finishBreakdownSoundPlayer);
                    else
                        v7 = SND_FindAliasId((char *)weaponDef->finishBreakdownSound);
                    goto LABEL_570;
                case EV_PREP_OFFHAND:
                    CG_PrepOffHand(localClientNum, p_nextState, p_nextState->eventParm, isPlayerView);
                    return;
                case EV_USE_OFFHAND:
                    CG_UseOffHand(localClientNum, cent, p_nextState->eventParm, isPlayerView);
                    return;
                case EV_SWITCH_OFFHAND:
                    if (clientNum != cgameGlob->predictedPlayerState.clientNum)
                        goto LABEL_290;
                    if (isPlayerView)
                        CG_SetEquippedOffHand(localClientNum, p_nextState->eventParm);
                    return;
                case EV_MELEE_HIT:
                    if (p_nextState->eventParm == 3)
                    {
                        CG_PlaySound(localClientNum, p_nextState->otherEntityNum, 0, 0, 0, 1.0, cgMedia.meleeDogHit);
                        CG_DogBloodEvent(localClientNum, p_nextState->otherEntityNum, position, p_nextState->un1.scale);
                    }
                    else if (meleeWeaponDef->meleeHitSound)
                    {
                        v20 = SND_FindAliasId((char *)meleeWeaponDef->meleeHitSound);
                        CG_PlaySound(localClientNum, p_nextState->otherEntityNum, 0, 0, 0, 1.0, v20);
                    }
                    else if (p_nextState->eventParm == 1)
                    {
                        CG_PlaySound(localClientNum, p_nextState->otherEntityNum, 0, 0, 0, 1.0, cgMedia.meleeKnifeHit);
                    }
                    else
                    {
                        CG_PlaySound(localClientNum, p_nextState->otherEntityNum, 0, 0, 0, 1.0, cgMedia.meleeHit);
                    }
                    return;
                case EV_MELEE_MISS:
                    if (p_nextState->eventParm == 2)
                    {
                        nanassertvec3(position);

                        CG_PlaySound(localClientNum, p_nextState->otherEntityNum, position, 0, 0, 1.0, cgMedia.meleeDogHitOther);
                    }
                    else
                    {
                        nanassertvec3(position);
                        if (meleeWeaponDef->meleeMissSound)
                        {
                            v21 = SND_FindAliasId((char *)meleeWeaponDef->meleeMissSound);
                            CG_PlaySound(localClientNum, p_nextState->otherEntityNum, position, 0, 0, 1.0, v21);
                        }
                        else if (p_nextState->eventParm == 1)
                        {
                            CG_PlaySound(
                                localClientNum,
                                p_nextState->otherEntityNum,
                                position,
                                0,
                                0,
                                1.0,
                                cgMedia.meleeKnifeHitOther);
                        }
                        else
                        {
                            CG_PlaySound(localClientNum, p_nextState->otherEntityNum, position, 0, 0, 1.0, cgMedia.meleeHitOther);
                        }
                    }
                    return;
                case EV_MELEE_BLOOD:
                    if (clientNum != cgameGlob->predictedPlayerState.clientNum)
                        goto LABEL_290;
                    if (isPlayerView)
                        CG_MeleeBloodEvent(localClientNum, cent, p_nextState->eventParm);
                    return;
                case EV_FIRE_WEAPON_MG42:
                case EV_FIRE_WEAPON_MG42A:
                    eType = ET_PLAYER;
                    CG_StartShakeCamera(localClientNum, 0.05f, 100, cent->pose.origin, 100.0);
                    if (event == 47)
                        CG_FireWeapon(localClientNum, cent, 47, scr_const.tag_flash, 0, &cgameGlob->nextSnap->ps, 0);
                    else
                        CG_FireWeapon(localClientNum, cent, event, scr_const.tag_flash_2, 0, &cgameGlob->nextSnap->ps, 0);
                    attackerCent = CG_GetEntity(localClientNum, eventParm);
                    if (((*((_DWORD *)attackerCent + 201) >> 1) & 1) != 0
                        && attackerCent->nextState.eType == eType
                        && ps->viewlocked_entNum != cent->nextState.number)
                    {
                        CG_CompassAddWeaponPingInfo(localClientNum, attackerCent, cent->pose.origin, 50);
                        //BG_EvalVehicleName();
                    }
                    if (((*((_DWORD *)attackerCent + 201) >> 1) & 1) != 0 && attackerCent->nextState.eType == 11)
                        CG_AddTurretWeaponPingInfo(localClientNum, attackerCent, cent->pose.origin, 50);
                    return;
                case EV_FIRE_QUADBARREL_1:
                    CG_FireWeapon(localClientNum, cent, event, scr_const.tag_flash, 0, &cgameGlob->nextSnap->ps, 0);
                    CG_FireWeapon(localClientNum, cent, event, scr_const.tag_flash_11, 0, &cgameGlob->nextSnap->ps, 0);
                    return;
                case EV_FIRE_QUADBARREL_2:
                    CG_FireWeapon(localClientNum, cent, event, scr_const.tag_flash_2, 0, &cgameGlob->nextSnap->ps, 0);
                    CG_FireWeapon(localClientNum, cent, event, scr_const.tag_flash_22, 0, &cgameGlob->nextSnap->ps, 0);
                    return;
                case EV_BULLET_HIT:
                    ByteToDir(p_nextState->eventParm, dir);
                    if (p_nextState->index.bone >= 0x100u
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                            1540,
                            0,
                            "%s",
                            "es->index.bone < 256"))
                    {
                        __debugbreak();
                    }
                    CG_BulletHitEvent(
                        localClientNum,
                        p_nextState->otherEntityNum,
                        p_nextState->groundEntityNum,
                        p_nextState->weapon,
                        p_nextState->lerp.u.turret.gunAngles,
                        cent->pose.origin,
                        dir,
                        dir,
                        p_nextState->surfType,
                        event,
                        p_nextState->un1.scale,
                        weaponDef->damage,
                        0,
                        p_nextState->index.bone);
                    return;
                case EV_BULLET_HIT_CLIENT_SMALL:
                case EV_BULLET_HIT_CLIENT_LARGE:
                    CG_BulletHitClientEvent(
                        localClientNum,
                        p_nextState->otherEntityNum,
                        p_nextState->weapon,
                        p_nextState->groundEntityNum,
                        p_nextState->lerp.u.turret.gunAngles,
                        cent->pose.origin,
                        p_nextState->surfType,
                        event,
                        weaponDef->damage);
                    return;
                case EV_DESTRUCTIBLE_BULLET_HIT:
                case EV_DESTRUCTIBLE_EXPLOSION_HIT:
                    CG_DestructibleHitEvent(localClientNum, event, p_nextState, 1);
                    return;
                case EV_GRENADE_BOUNCE:
                    if (p_nextState->surfType >= 0x1Fu
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                            1557,
                            0,
                            "es->surfType doesn't index SURF_TYPECOUNT\n\t%i not in [0, %i)",
                            p_nextState->surfType,
                            31))
                    {
                        __debugbreak();
                    }
                    nanassertvec3(position);
                    if (weaponDef->bounceSound)
                    {
                        v24 = SND_FindAliasId((char *)weaponDef->bounceSound[p_nextState->surfType]);
                        CG_PlaySound(localClientNum, 1022, position, 0, 0, 1.0, v24);
                    }
                    if (cgMedia.fx->table[11].nonflesh[p_nextState->surfType])
                    {
                        ByteToDir(p_nextState->eventParm, axis[0]);
                        Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                        FX_PlayOrientedEffect(
                            localClientNum,
                            cgMedia.fx->table[11].nonflesh[p_nextState->surfType],
                            cgameGlob->time,
                            position,
                            axis);
                    }
                    return;
                case EV_GRENADE_EXPLODE:
                    waterSimEnabled = r_watersim_enabled->current.enabled;
                    if (waterSimEnabled)
                        Height = R_WaterSimulationGetHeight(position);
                    else
                        Height = 0.0f;
                    waterZ = Height;
                    explosionRadius = (float)weaponDef->iExplosionRadius;
                    Ragdoll_ExplosionEvent(localClientNum, 0, position, 0.0, explosionRadius, vec3_origin, 1.0);
                    DynEntCl_ExplosionEvent(
                        localClientNum,
                        0,
                        position,
                        0.0,
                        explosionRadius,
                        (float *)vec3_origin,
                        1.0,
                        weaponDef->iExplosionInnerDamage,
                        weaponDef->iExplosionOuterDamage);
                    CG_ExplosionEvent(
                        localClientNum,
                        position,
                        (float)weaponDef->iExplosionInnerDamage,
                        (float)weaponDef->iExplosionOuterDamage,
                        explosionRadius,
                        4);
                    if (waterSimEnabled && waterZ > position[2])
                        R_WaterSimulationNotifyExplosion(position);
                    R_FoliageNotifyGrenadeExplosion(localClientNum, position);
                    if (p_nextState->surfType >= 0x1Fu
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                            1595,
                            0,
                            "es->surfType doesn't index SURF_TYPECOUNT\n\t%i not in [0, %i)",
                            p_nextState->surfType,
                            31))
                    {
                        __debugbreak();
                    }
                    ByteToDir(p_nextState->eventParm, axis[0]);
                    Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                    surfType = p_nextState->surfType;
                    if (waterSimEnabled && waterZ > position[2])
                    {
                        surfType = 20;
                        position[2] = waterZ - 36.0;
                    }
                    CG_ImpactEffectForWeapon(weaponIdx, surfType, 0, &outFx, &outSnd);
                    nanassertvec3(position);
                    if (outFx)
                        FX_PlayOrientedEffect(localClientNum, outFx, cgameGlob->time, position, axis);
                    CG_PlaySound(localClientNum, 1022, position, 0, 0, 1.0, outSnd);
                    if (weaponDef->projExplosionEffect)
                        FX_PlayOrientedEffect(localClientNum, weaponDef->projExplosionEffect, cgameGlob->time, position, axis);
                    v25 = SND_FindAliasId((char *)weaponDef->projExplosionSound);
                    CG_PlaySound(localClientNum, 1022, position, 0, 0, 1.0, v25);
                    return;
                case EV_ROCKET_EXPLODE:
                    goto $LN108;
                case EV_ROCKET_EXPLODE_NOMARKS:
                    cgameGlob->nomarks = 1;
                $LN108:
                    Ragdoll_ExplosionEvent(
                        localClientNum,
                        0,
                        position,
                        (float)weaponDef->iExplosionRadius,
                        (float)weaponDef->iExplosionRadius,
                        vec3_origin,
                        1.0);
                    DynEntCl_ExplosionEvent(
                        localClientNum,
                        0,
                        position,
                        (float)weaponDef->iExplosionRadius,
                        (float)weaponDef->iExplosionRadius,
                        (float *)vec3_origin,
                        1.0,
                        weaponDef->iExplosionInnerDamage,
                        weaponDef->iExplosionOuterDamage);
                    mod = 6;
                    if (cgameGlob->nomarks)
                        mod = 5;
                    CG_ExplosionEvent(
                        localClientNum,
                        position,
                        (float)weaponDef->iExplosionInnerDamage,
                        (float)weaponDef->iExplosionOuterDamage,
                        (float)weaponDef->iExplosionRadius,
                        mod);
                    if (p_nextState->surfType >= 0x1Fu
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                            1645,
                            0,
                            "es->surfType doesn't index SURF_TYPECOUNT\n\t%i not in [0, %i)",
                            p_nextState->surfType,
                            31))
                    {
                        __debugbreak();
                    }
                    ByteToDir(p_nextState->eventParm, axis[0]);
                    Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                    nanassertvec3(position);
                    CG_ImpactEffectForWeapon(weaponIdx, p_nextState->surfType, 0, &def, &v86);
                    if (def)
                        FX_PlayOrientedEffect(localClientNum, def, cgameGlob->time, position, axis);
                    CG_PlaySound(localClientNum, 1022, position, 0, 0, 1.0, v86);
                    if (weaponDef->projExplosionEffect)
                        FX_PlayOrientedEffect(localClientNum, weaponDef->projExplosionEffect, cgameGlob->time, position, axis);
                    v26 = SND_FindAliasId((char *)weaponDef->projExplosionSound);
                    CG_PlaySound(localClientNum, 1022, position, 0, 0, 1.0, v26);
                    cgameGlob->nomarks = 0;
                    return;
                case EV_FLASHBANG_EXPLODE:
                    nanassertvec3(position);
                    ByteToDir(p_nextState->eventParm, axis[0]);
                    Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                    if (weaponDef->projExplosionEffect)
                        FX_PlayOrientedEffect(localClientNum, weaponDef->projExplosionEffect, cgameGlob->time, position, axis);
                    v27 = SND_FindAliasId((char *)weaponDef->projExplosionSound);
                    CG_PlaySound(localClientNum, 1022, position, 0, 0, 1.0, v27);
                    return;
                case EV_CUSTOM_EXPLODE:
                    goto $LN101;
                case EV_CUSTOM_EXPLODE_NOMARKS:
                    cgameGlob->nomarks = 1;
                $LN101:
                    innerRadius = 0.0f;
                    Ragdoll_ExplosionEvent(
                        localClientNum,
                        0,
                        position,
                        0.0,
                        (float)weaponDef->iExplosionRadius,
                        vec3_origin,
                        1.0);
                    DynEntCl_ExplosionEvent(
                        localClientNum,
                        0,
                        position,
                        0.0,
                        (float)weaponDef->iExplosionRadius,
                        (float *)vec3_origin,
                        1.0,
                        weaponDef->iExplosionInnerDamage,
                        weaponDef->iExplosionOuterDamage);
                    CG_ExplosionEvent(
                        localClientNum,
                        position,
                        (float)weaponDef->iExplosionInnerDamage,
                        (float)weaponDef->iExplosionOuterDamage,
                        (float)weaponDef->iExplosionRadius,
                        15);
                    ByteToDir(p_nextState->eventParm, axis[0]);
                    Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                    nanassertvec3(position);
                    CG_ImpactEffectForWeapon(weaponIdx, p_nextState->surfType, 0, &v84, &v83);
                    if (v84)
                        FX_PlayOrientedEffect(localClientNum, v84, cgameGlob->time, position, axis);
                    CG_PlaySound(localClientNum, 1022, position, 0, 0, 1.0, v83);
                    if (weaponDef->projExplosionEffect)
                    {
                        Com_Printf(
                            14,
                            "Playing smoke grenade at %i at ( %f, %f, %f )\n",
                            p_nextState->lerp.u.actor.actorNum,
                            p_nextState->lerp.pos.trBase[0],
                            p_nextState->lerp.pos.trBase[1],
                            p_nextState->lerp.pos.trBase[2]);
                        switch (p_nextState->lerp.u.loopFx.period)
                        {
                        case '1':
                            expFx = weaponDef->projExplosionEffect2;
                            break;
                        case '2':
                            expFx = weaponDef->projExplosionEffect3;
                            break;
                        case '3':
                            expFx = weaponDef->projExplosionEffect5;
                            break;
                        case '4':
                            expFx = weaponDef->projExplosionEffect4;
                            break;
                        default:
                            expFx = weaponDef->projExplosionEffect;
                            break;
                        }
                        FX_PlayOrientedEffect(
                            localClientNum,
                            expFx,
                            p_nextState->lerp.u.actor.actorNum,
                            p_nextState->lerp.pos.trBase,
                            axis);
                    }
                    if (weaponDef->projExplosionSound)
                    {
                        nanassertvec3(position);

                        if ((p_nextState->lerp.eFlags & 0x4000) == 0
                            || cgameGlob->time - p_nextState->lerp.u.actor.actorNum < 200)
                        {
                            v28 = SND_FindAliasId((char *)weaponDef->projExplosionSound);
                            CG_PlaySound(localClientNum, 1022, position, 0, 0, 1.0, v28);
                        }
                    }
                    cgameGlob->nomarks = 0;
                    return;
                case EV_CHANGE_TO_DUD:
                    cgameGlob->nomarks = 1;
                    ByteToDir(p_nextState->eventParm, axis[0]);
                    Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                    if (cgMedia.fx->table[15].nonflesh[0])
                        FX_PlayOrientedEffect(localClientNum, cgMedia.fx->table[15].nonflesh[0], cgameGlob->time, position, axis);
                    cgameGlob->nomarks = 0;
                    return;
                case EV_DUD_EXPLODE:
                    if (p_nextState->surfType >= 0x1Fu
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                            1763,
                            0,
                            "es->surfType doesn't index SURF_TYPECOUNT\n\t%i not in [0, %i)",
                            p_nextState->surfType,
                            31))
                    {
                        __debugbreak();
                    }
                    ByteToDir(p_nextState->eventParm, axis[0]);
                    Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                    nanassertvec3(position);

                    CG_PlaySound(localClientNum, 1022, position, 0, 0, 1.0, cgMedia.bulletHitLargeSound[p_nextState->surfType]);
                    if (cgMedia.fx->table[15].nonflesh[p_nextState->surfType])
                        FX_PlayOrientedEffect(
                            localClientNum,
                            cgMedia.fx->table[15].nonflesh[p_nextState->surfType],
                            cgameGlob->time,
                            position,
                            axis);
                    if (weaponDef->projExplosionEffect)
                        FX_PlayOrientedEffect(localClientNum, weaponDef->projExplosionEffect, cgameGlob->time, position, axis);
                    v29 = SND_FindAliasId((char *)weaponDef->projExplosionSound);
                    CG_PlaySound(localClientNum, 1022, position, 0, 0, 1.0, v29);
                    if (weaponDef->projDudEffect)
                        FX_PlayOrientedEffect(localClientNum, weaponDef->projDudEffect, cgameGlob->time, position, axis);
                    goto LABEL_437;
                case EV_DUD_IMPACT:
                    if (p_nextState->surfType >= 0x1Fu
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                            1784,
                            0,
                            "es->surfType doesn't index SURF_TYPECOUNT\n\t%i not in [0, %i)",
                            p_nextState->surfType,
                            31))
                    {
                        __debugbreak();
                    }
                    ByteToDir(p_nextState->eventParm, axis[0]);
                    Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                    nanassertvec3(position);

                    CG_PlaySound(localClientNum, 1022, position, 0, 0, 1.0, cgMedia.bulletHitLargeSound[p_nextState->surfType]);
                    if (cgMedia.fx->table[15].nonflesh[p_nextState->surfType])
                        FX_PlayOrientedEffect(
                            localClientNum,
                            cgMedia.fx->table[15].nonflesh[p_nextState->surfType],
                            cgameGlob->time,
                            position,
                            axis);
                    if (weaponDef->projDudEffect)
                        FX_PlayOrientedEffect(localClientNum, weaponDef->projDudEffect, cgameGlob->time, position, axis);
                LABEL_437:
                    v30 = SND_FindAliasId((char *)weaponDef->projDudSound);
                    CG_PlaySound(localClientNum, 1022, position, 0, 0, 1.0, v30);
                    return;
                case EV_FIRE_EXPLODE:
                    CG_SetFireToTerrain(
                        position,
                        10.0,
                        p_nextState->attackerEntityNum,
                        p_nextState->weapon,
                        1.0,
                        0,
                        0,
                        TFS_DEFAULT);
                    if ((p_nextState->lerp.eFlags2 & 1) != 0)
                    {
                        AnglesToAxis(cent->pose.angles, axis);
                    }
                    else
                    {
                        ByteToDir(p_nextState->eventParm, axis[0]);
                        Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                    }
                    if (weaponDef->projExplosionEffect)
                        FX_PlayOrientedEffect(localClientNum, weaponDef->projExplosionEffect, cgameGlob->time, position, axis);
                    Ragdoll_ExplosionEvent(
                        localClientNum,
                        0,
                        position,
                        (float)weaponDef->iExplosionRadius,
                        (float)weaponDef->iExplosionRadius,
                        vec3_origin,
                        1.0);
                    DynEntCl_ExplosionEvent(
                        localClientNum,
                        0,
                        position,
                        (float)weaponDef->iExplosionRadius,
                        (float)weaponDef->iExplosionRadius,
                        (float *)vec3_origin,
                        1.0,
                        weaponDef->iExplosionInnerDamage,
                        weaponDef->iExplosionOuterDamage);
                    CG_ExplosionEvent(
                        localClientNum,
                        position,
                        (float)weaponDef->iExplosionInnerDamage,
                        (float)weaponDef->iExplosionOuterDamage,
                        (float)weaponDef->iExplosionRadius,
                        15);
                    nanassertvec3(position);

                    if (weaponDef->weapType == WEAPTYPE_PROJECTILE)
                        v31 = SND_FindAliasId((char *)weaponDef->projExplosionSound);
                    else
                        v31 = SND_FindAliasId((char *)weaponDef->detonateSound);
                    CG_PlaySound(localClientNum, 1022, position, 0, 0, 1.0, v31);
                    return;
                case EV_TIMED_FX:
                    ByteToDir(p_nextState->eventParm, axis[0]);
                    Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                    if (weaponDef->projExplosionEffect)
                        FX_PlayOrientedEffect(localClientNum, weaponDef->projExplosionEffect, cgameGlob->time, position, axis);
                    return;
                case EV_MOLOTOV_FLOAT:
                    CG_MolotovFloat(localClientNum, cent, p_nextState);
                    return;
                case EV_FAKE_FIRE:
                    CG_FakeFireWeapon(localClientNum, cent, p_nextState->eventParm, position, p_nextState->un1.scale);
                    return;
                case EV_PLAY_FX:
                    CG_PlayFx(localClientNum, cent, p_nextState->lerp.apos.trBase);
                    return;
                case EV_PLAY_FX_ON_TAG:
                    CG_PlayFxOnTag(localClientNum, cent, p_nextState->eventParm);
                    return;
                case EV_PHYS_EXPLOSION_SPHERE:
                    Ragdoll_ExplosionEvent(
                        localClientNum,
                        0,
                        position,
                        p_nextState->lerp.u.turret.gunAngles[0],
                        (float)p_nextState->eventParm,
                        vec3_origin,
                        p_nextState->lerp.u.turret.gunAngles[1]);
                    DynEntCl_ExplosionEvent(
                        localClientNum,
                        0,
                        position,
                        p_nextState->lerp.u.turret.gunAngles[0],
                        (float)p_nextState->eventParm,
                        (float *)vec3_origin,
                        p_nextState->lerp.u.turret.gunAngles[1],
                        (int)p_nextState->lerp.u.turret.gunAngles[2],
                        (int)p_nextState->lerp.u.turret.heatVal);
                    R_WaterSimulationNotifyExplosionEx(
                        position,
                        (int)(float)(4.0 * p_nextState->lerp.u.turret.gunAngles[1]),
                        4.0 * p_nextState->lerp.u.turret.gunAngles[1]);
                    CG_ExplosionEvent(
                        localClientNum,
                        position,
                        p_nextState->lerp.u.turret.gunAngles[2],
                        p_nextState->lerp.u.turret.heatVal,
                        (float)p_nextState->eventParm,
                        15);
                    return;
                case EV_PHYS_EXPLOSION_CYLINDER:
                    Ragdoll_ExplosionEvent(
                        localClientNum,
                        1,
                        position,
                        p_nextState->lerp.u.turret.gunAngles[0],
                        (float)p_nextState->eventParm,
                        vec3_origin,
                        p_nextState->lerp.u.turret.gunAngles[1]);
                    DynEntCl_ExplosionEvent(
                        localClientNum,
                        1,
                        position,
                        p_nextState->lerp.u.turret.gunAngles[0],
                        (float)p_nextState->eventParm,
                        (float *)vec3_origin,
                        p_nextState->lerp.u.turret.gunAngles[1],
                        0,
                        0);
                    R_WaterSimulationNotifyExplosionEx(
                        position,
                        (int)(float)(4.0 * p_nextState->lerp.u.turret.gunAngles[1]),
                        4.0 * p_nextState->lerp.u.turret.gunAngles[1]);
                    return;
                case EV_PHYS_EXPLOSION_JOLT:
                    Ragdoll_ExplosionEvent(
                        localClientNum,
                        1,
                        position,
                        p_nextState->lerp.u.turret.gunAngles[0],
                        (float)p_nextState->eventParm,
                        &p_nextState->lerp.u.turret.gunAngles[1],
                        1.0);
                    DynEntCl_ExplosionEvent(
                        localClientNum,
                        1,
                        position,
                        p_nextState->lerp.u.turret.gunAngles[0],
                        (float)p_nextState->eventParm,
                        (float*)&p_nextState->lerp.u.turret.gunAngles[1],
                        1.0,
                        0,
                        0);
                    return;
                case EV_PHYS_LAUNCH:
                    if (CL_LocalClient_IsFirstActive(localClientNum))
                    {
                        v73 = CG_GetEntity(localClientNum, p_nextState->otherEntityNum);
                        CG_PhysLaunch(localClientNum, v73, p_nextState);
                    }
                    return;
                case EV_CREATE_DYNENT:
                    if (loc_language->current.integer != 3) // KISAKTODO: loc_language??
                    {
                        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
                        model = cgs->gameModels[p_nextState->index.brushmodel];
                        if (model->physPreset)
                        {
                            AnglesToQuat(p_nextState->lerp.apos.trBase, quat);
                            v65[0] = p_nextState->lerp.pos.trBase[0];
                            v65[1] = p_nextState->lerp.pos.trBase[1];
                            v65[2] = p_nextState->lerp.pos.trBase[2];
                            hitpos[0] = p_nextState->lerp.pos.trDelta[0];
                            hitpos[1] = p_nextState->lerp.pos.trDelta[1];
                            hitpos[2] = p_nextState->lerp.pos.trDelta[2];
                            force[0] = p_nextState->lerp.apos.trDelta[0];
                            force[1] = p_nextState->lerp.apos.trDelta[1];
                            force[2] = p_nextState->lerp.apos.trDelta[2];
                            id = DynEntCl_CreateEntityModel(model, quat, v65, hitpos, force, 0, 0);
                            DynEntCl_SetFadeOut(id, DYNENT_DRAW_MODEL);
                            fxId = p_nextState->un1.scale;
                            if (fxId)
                            {
                                fxDef = cgs->fxs[fxId];
                                if (!fxDef
                                    && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                                        2088,
                                        0,
                                        "%s",
                                        "fxDef"))
                                {
                                    __debugbreak();
                                }
                                DynEntCl_PlayBoltedFX(fxDef, id);
                            }
                        }
                        else
                        {
                            Com_Printf(25, "Script spawning dyn ent with model with no physPreset %s\n", model->name);
                        }
                    }
                    return;
                case EV_BLOOD_IMPACTS:
                    v33 = CG_GetEntity(localClientNum, p_nextState->lerp.pos.trTime);
                    *((_DWORD *)v33 + 201) = ((p_nextState->eventParm == 0) << 6) | *((_DWORD *)v33 + 201) & 0xFFFFFFBF;
                    *((_DWORD *)v33 + 201) = ((p_nextState->eventParm == 2) << 7) | *((_DWORD *)v33 + 201) & 0xFFFFFF7F;
                    return;
                case EV_SETWETNESS:
                    CG_SetWetness(
                        localClientNum,
                        p_nextState->otherEntityNum,
                        (float)(p_nextState->eventParm & 0x7F) / 127.0,
                        (p_nextState->eventParm & 0x80) != 0);
                    return;
                case EV_EARTHQUAKE:
                    CG_StartShakeCamera(
                        localClientNum,
                        p_nextState->lerp.u.turret.gunAngles[0],
                        p_nextState->lerp.u.actor.team,
                        cent->pose.origin,
                        p_nextState->lerp.u.turret.gunAngles[1]);
                    return;
                case EV_DETONATE:
                    if (isPlayerView)
                        v7 = SND_FindAliasId((char *)weaponDef->detonateSoundPlayer);
                    else
                        v7 = SND_FindAliasId((char *)weaponDef->detonateSound);
                    goto LABEL_570;
                case EV_NIGHTVISION_WEAR:
                    if (isPlayerView && ((ps->eFlags & 0x300) != 0 || !**((_BYTE **)weaponVariantDef->szXAnims + 40)))
                        CG_PlaySound(localClientNum, p_nextState->number, 0, 0, 0, 1.0, cgMedia.nightVisionOn);
                    if (isPlayerView)
                        v7 = SND_FindAliasId((char *)weaponDef->nightVisionWearSoundPlayer);
                    else
                        v7 = SND_FindAliasId((char *)weaponDef->nightVisionWearSound);
                    goto LABEL_570;
                case EV_NIGHTVISION_REMOVE:
                    if (isPlayerView && ((ps->eFlags & 0x300) != 0 || !**((_BYTE **)weaponVariantDef->szXAnims + 41)))
                        CG_PlaySound(localClientNum, p_nextState->number, 0, 0, 0, 1.0, cgMedia.nightVisionOff);
                    if (isPlayerView)
                        v7 = SND_FindAliasId((char *)weaponDef->nightVisionRemoveSoundPlayer);
                    else
                        v7 = SND_FindAliasId((char *)weaponDef->nightVisionRemoveSound);
                    goto LABEL_570;
                case EV_OBITUARY:
                    CG_Obituary(localClientNum, p_nextState);
                    return;
                case EV_REVIVE_OBITUARY:
                    CG_ObituaryRevive(localClientNum, p_nextState);
                    return;
                case EV_DIRECTIONAL_HIT_INDICATOR:
                    CG_DirectionalHitIndicator(localClientNum, p_nextState);
                    return;
                case EV_NO_FRAG_GRENADE_HINT:
                    v58 = cgameGlob->nextSnap;
                    if ((v58->ps.otherFlags & 6) != 0 && p_nextState->number == v58->ps.clientNum)
                        CG_SetInvalidCmdHint(cgameGlob, INVALID_CMD_NO_AMMO_FRAG_GRENADE);
                    return;
                case EV_NO_SPECIAL_GRENADE_HINT:
                    v57 = cgameGlob->nextSnap;
                    if ((v57->ps.otherFlags & 6) != 0 && p_nextState->number == v57->ps.clientNum)
                        CG_SetInvalidCmdHint(cgameGlob, INVALID_CMD_NO_AMMO_SPECIAL_GRENADE);
                    return;
                case EV_TARGET_TOO_CLOSE_HINT:
                    v56 = cgameGlob->nextSnap;
                    if ((v56->ps.otherFlags & 6) != 0 && p_nextState->number == v56->ps.clientNum)
                        CG_SetInvalidCmdHint(cgameGlob, INVALID_CMD_LOCKON_REQUIRED);
                    return;
                case EV_TARGET_NOT_ENOUGH_CLEARANCE:
                    v55 = cgameGlob->nextSnap;
                    if ((v55->ps.otherFlags & 6) != 0 && p_nextState->number == v55->ps.clientNum)
                        CG_SetInvalidCmdHint(cgameGlob, INVALID_CMD_CANT_PLACE_TURRET);
                    return;
                case EV_LOCKON_REQUIRED_HINT:
                    v54 = cgameGlob->nextSnap;
                    if ((v54->ps.otherFlags & 6) != 0 && p_nextState->number == v54->ps.clientNum)
                        CG_SetInvalidCmdHint(cgameGlob, INVALID_CMD_NOT_ENOUGH_CLEARANCE);
                    return;
                case EV_FOOTSTEP_SPRINT:
                case EV_FOOTSTEP_RUN:
                case EV_FOOTSTEP_WALK:
                case EV_FOOTSTEP_CROUCHRUN:
                case EV_FOOTSTEP_CROUCHWALK:
                case EV_FOOTSTEP_PRONE:
                    CG_DoFootstepEvent(localClientNum, cent, isPlayerView, event, eventParm, quiet);
                    return;
                case EV_MANTLE:
                    if (isPlayerView)
                        mantleSoundPlayer = cgMedia.mantleSoundPlayer;
                    else
                        mantleSoundPlayer = cgMedia.mantleSound;
                    CG_PlaySound(localClientNum, p_nextState->number, 0, 0, 0, 1.0, mantleSoundPlayer);
                    return;
                case EV_JUMP:
                    CG_DoJumpEvent(localClientNum, cent, isPlayerView, eventParm, quiet);
                    return;
                case EV_FIRE_VEHICLE_TURRET:
                    CScr_NotifyNum(localClientNum, p_nextState->number, 0, cscr_const.face_shoot_single, 0);
                    v105 = 1;
                    CG_FireWeapon(localClientNum, cent, event, scr_const.tag_flash, 0, &cgameGlob->nextSnap->ps, 0);
                    CG_StartShakeCamera(localClientNum, 0.05f, 100, cent->pose.origin, 100.0);
                    CG_CompassAddVehicleWeaponPingInfo(localClientNum, cent, cent->pose.origin, 50);
                    return;
                case EV_FIRE_GUNNER_1:
                case EV_FIRE_GUNNER_2:
                case EV_FIRE_GUNNER_3:
                case EV_FIRE_GUNNER_4:
                case EV_FIRE_GUNNER_1A:
                case EV_FIRE_GUNNER_2A:
                case EV_FIRE_GUNNER_3A:
                case EV_FIRE_GUNNER_4A:
                    v104 = 1;
                    flashIndex = event - 174;
                    gunnerIndex = (event - 174) % 4;
                    info = (vehicle_info_t *)CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex);
                    weapon = info->gunnerWeaponIndex[gunnerIndex];
                    if (!weapon)
                    {
                        WeaponIndexForName = CG_GetWeaponIndexForName(info->gunnerWeapon[gunnerIndex]);
                        AssignToSmallerType<unsigned short>(&info->gunnerWeaponIndex[gunnerIndex], WeaponIndexForName);
                        weapon = info->gunnerWeaponIndex[gunnerIndex];
                    }
                    CG_FireWeapon(
                        localClientNum,
                        cent,
                        event,
                        scr_const.tag_flash_gunner[flashIndex],
                        weapon,
                        &cgameGlob->nextSnap->ps,
                        0);
                    if (cent->vehicle)
                        cent->vehicle->lastGunnerFire[gunnerIndex] = cgameGlob->time;
                    CG_StartShakeCamera(localClientNum, 0.05f, 100, cent->pose.origin, 100.0);
                    v103 = CG_GetEntity(localClientNum, eventParm);
                    if (v103
                        && ((*((_DWORD *)v103 + 201) >> 1) & 1) != 0
                        && v103->nextState.eType == v104
                        && ps->viewlocked_entNum != cent->nextState.number)
                    {
                        CG_CompassAddWeaponPingInfo(localClientNum, v103, cent->pose.origin, 50);
                        //BG_EvalVehicleName();
                    }
                    CG_CompassAddVehicleWeaponPingInfo(localClientNum, cent, cent->pose.origin, 50);
                    return;
                case EV_START_CAMERA_TWEEN:
                    CG_StartCameraTween(localClientNum, (float)p_nextState->eventParm * 0.001);
                    return;
                case EV_DESTRUCTIBLE_DISABLE_PIECES:
                    if (p_nextState->eventParm)
                        CG_DisableDestructiblePiece(localClientNum, p_nextState->eventParm);
                    else
                        CG_EnableAllDestructiblePieces();
                    return;
                case EV_CANNOTPLANT:
                    v59 = cgameGlob->nextSnap;
                    if ((v59->ps.otherFlags & 6) != 0 && p_nextState->number == v59->ps.clientNum)
                        CG_SetInvalidCmdHint(cgameGlob, (InvalidCmdHintType)(INVALID_CMD_CANT_EQUIP_WHILE_PRONE | INVALID_CMD_NO_AMMO_FRAG_GRENADE));
                    if (isPlayerView)
                        v7 = SND_FindAliasId((char *)weaponDef->emptyFireSoundPlayer);
                    else
                        v7 = SND_FindAliasId((char *)weaponDef->emptyFireSound);
                    goto LABEL_570;
                case EV_DTP_LAUNCH:
                    if (isPlayerView)
                        dtpLaunchSoundPlayer = cgMedia.dtpLaunchSoundPlayer;
                    else
                        dtpLaunchSoundPlayer = cgMedia.dtpLaunchSound;
                    CG_PlaySound(localClientNum, p_nextState->number, 0, 0, 0, 1.0, dtpLaunchSoundPlayer);
                    return;
                case EV_DTP_LAND:
                    if (isPlayerView)
                        CG_PlaySound(localClientNum, p_nextState->number, 0, 0, 0, 1.0, cgMedia.dtpLandSoundPlayer[eventParm]);
                    else
                        CG_PlaySound(localClientNum, p_nextState->number, 0, 0, 0, 1.0, cgMedia.dtpLandSound[eventParm]);
                    if (isPlayerView)
                        v42 = cgMedia.dtpSlideLoopSoundPlayer[eventParm];
                    else
                        v42 = cgMedia.dtpSlideLoopSound[eventParm];
                    CG_PlaySound(localClientNum, p_nextState->number, 0, 0, 0, 1.0, v42);
                    if (isPlayerView)
                    {
                        attenuation_4 = scr_const.j_elbow_le;
                        v34 = CG_WeaponDObjHandle(localClientNum);
                        CG_PlayBoltedEffect(localClientNum, cgMedia.fxDtpArmSlide1, v34, attenuation_4);
                        attenuation_4a = scr_const.j_wrist_le;
                        v35 = CG_WeaponDObjHandle(localClientNum);
                        CG_PlayBoltedEffect(localClientNum, cgMedia.fxDtpArmSlide2, v35, attenuation_4a);
                    }
                    else
                    {
                        CG_PlayBoltedEffect(localClientNum, cgMedia.fxPuff, p_nextState->number, scr_const.j_elbow_le);
                    }
                    Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
                    CScr_NotifyNum(localClientNum, p_nextState->number, 0, cscr_const.dtp_land, 1u);
                    return;
                case EV_SLIDE_START:
                    if (!cgameGlob->slideFakeEnt)
                        cgameGlob->slideFakeEnt = CG_Spawn(localClientNum);
                    v53 = cgameGlob->slideFakeEnt->pose.origin;
                    *v53 = cent->pose.origin[0];
                    v53[1] = cent->pose.origin[1];
                    v53[2] = cent->pose.origin[2];
                    fent = &cg_fakeEntitiesArray[512 * localClientNum - 1024 + cgameGlob->slideFakeEnt->nextState.number];
                    if (isPlayerView)
                        v41 = cgMedia.dtpSlideLoopSoundPlayer[eventParm];
                    else
                        v41 = cgMedia.dtpSlideLoopSound[eventParm];
                    fent->cent.nextState.loopSoundId = SND_FindAliasId(v41);
                    fent->cent.nextState.loopSoundFade = 0;
                    CG_PumpEntityLoopSound(0, &fent->cent);
                    if (isPlayerView)
                        v40 = cgMedia.playerSlidingStart_1p[eventParm];
                    else
                        v40 = cgMedia.playerSlidingStart_3p[eventParm];
                    CG_PlaySound(localClientNum, p_nextState->number, 0, 0, 0, 1.0, v40);
                    return;
                case EV_SLIDE_STOP:
                    if (cgameGlob->slideFakeEnt)
                    {
                        v62 = &cg_fakeEntitiesArray[512 * localClientNum - 1024 + cgameGlob->slideFakeEnt->nextState.number];
                        v62->cent.nextState.loopSoundId = 0;
                        CG_Free(localClientNum, v62->cent.nextState.number);
                        cgameGlob->slideFakeEnt = 0;
                    }
                    if (isPlayerView)
                        CG_PlaySound(localClientNum, p_nextState->number, 0, 0, 0, 1.0, cgMedia.playerSlidingStop_1p[eventParm]);
                    else
                        CG_PlaySound(localClientNum, p_nextState->number, 0, 0, 0, 1.0, cgMedia.playerSlidingStop_3p[eventParm]);
                    goto $LN22_6;
                case EV_SCOPE_ZOOM:
                $LN22_6:
                    if (isPlayerView)
                        v7 = SND_FindAliasId((char *)weaponDef->adsZoomSound);
                    else
                        v7 = SND_FindAliasId((char*)0);
                LABEL_570:
                    CG_PlaySound(localClientNum, p_nextState->number, 0, 0, 0, 1.0, v7);
                    return;
                case EV_STACKFIRE:
                    if (isPlayerView)
                    {
                        v19 = SND_FindAliasId((char *)weaponDef->stackSound);
                        CG_PlaySound(localClientNum, p_nextState->number, 0, 0, 0, 1.0, v19);
                    }
                    return;
                case EV_BOLT_IMPACT:
                    ByteToDir(p_nextState->eventParm, axis[0]);
                    Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                    nanassertvec3(position);

                    CG_ImpactEffectForWeapon(weaponIdx, p_nextState->surfType, 0, &fx, &snd);
                    if (fx)
                        FX_PlayOrientedEffect(localClientNum, fx, cgameGlob->time, cent->nextState.lerp.pos.trBase, axis);
                    CG_PlaySound(localClientNum, 1022, position, 0, 0, 1.0, snd);
                    if (!strlen(BG_GetWeaponDef(p_nextState->weapon)->szSpawnedGrenadeWeaponName))
                        CG_AddBolt(
                            localClientNum,
                            p_nextState->groundEntityNum,
                            p_nextState->weapon,
                            cent->nextState.lerp.pos.trBase,
                            cent->pose.angles,
                            p_nextState->index.bone,
                            cent->pose.physUserBody);
                    return;
                case EV_BOLT_IMPALE:
                    ByteToDir(p_nextState->eventParm, axis[0]);
                    Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
                    nanassertvec3(position);

                    CG_ImpactEffectForWeapon(weaponIdx, 7u, 0, &fx, &snd);
                    if (fx)
                        FX_PlayOrientedEffect(localClientNum, fx, cgameGlob->time, position, axis);
                    CG_PlaySound(localClientNum, 1022, position, 0, 0, 1.0, snd);
                    if (CL_LocalClient_IsFirstActive(localClientNum))
                    {
                        attachedCorpse = CG_GetEntity(localClientNum, p_nextState->otherEntityNum);
                        if (attachedCorpse)
                        {
                            if (attachedCorpse->pose.eType == 17)
                            {
                                if (!attachedCorpse->pose.isRagdoll
                                    && Com_IsRagdollTrajectory(&attachedCorpse->nextState.lerp.pos)
                                    && !attachedCorpse->pose.ragdollHandle)
                                {
                                    CG_CreateRagdollObject(localClientNum, attachedCorpse);
                                }
                                if (attachedCorpse->pose.isRagdoll && attachedCorpse->pose.ragdollHandle > 0)
                                    Ragdoll_Attach(
                                        localClientNum,
                                        attachedCorpse->pose.ragdollHandle,
                                        p_nextState->number,
                                        (hitLocation_t)p_nextState->index.bone);
                            }
                        }
                    }
                    return;
                case EV_FACE_EVENT:
                    if (eventParm >= 12
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                            2236,
                            0,
                            "%s",
                            "eventParm < XANIM_FACE_NUM_EVENTS"))
                    {
                        __debugbreak();
                    }
                    switch (eventParm)
                    {
                    case 0:
                        Scr_AddConstString(cscr_const.face_casual, SCRIPTINSTANCE_CLIENT);
                        break;
                    case 1:
                        Scr_AddConstString(cscr_const.face_alert, SCRIPTINSTANCE_CLIENT);
                        break;
                    case 2:
                        Scr_AddConstString(cscr_const.face_cqb, SCRIPTINSTANCE_CLIENT);
                        break;
                    case 3:
                        Scr_AddConstString(cscr_const.face_running, SCRIPTINSTANCE_CLIENT);
                        break;
                    case 4:
                        Scr_AddConstString(cscr_const.face_shoot_single, SCRIPTINSTANCE_CLIENT);
                        break;
                    case 5:
                        Scr_AddConstString(cscr_const.face_shoot_burst, SCRIPTINSTANCE_CLIENT);
                        break;
                    case 6:
                        Scr_AddConstString(cscr_const.face_melee, SCRIPTINSTANCE_CLIENT);
                        break;
                    case 7:
                        Scr_AddConstString(cscr_const.face_react, SCRIPTINSTANCE_CLIENT);
                        break;
                    case 8:
                        Scr_AddConstString(cscr_const.face_talk, SCRIPTINSTANCE_CLIENT);
                        break;
                    case 9:
                        Scr_AddConstString(cscr_const.face_talk_long, SCRIPTINSTANCE_CLIENT);
                        break;
                    case 10:
                        Scr_AddConstString(cscr_const.face_pain, SCRIPTINSTANCE_CLIENT);
                        break;
                    case 11:
                        Scr_AddConstString(cscr_const.face_death, SCRIPTINSTANCE_CLIENT);
                        break;
                    default:
                        Scr_AddConstString(cscr_const.none, SCRIPTINSTANCE_CLIENT);
                        break;
                    }
                    centa = CG_GetEntity(localClientNum, p_nextState->number);
                    if (((*((_DWORD *)centa + 201) >> 1) & 1) != 0)
                        CScr_Notify(localClientNum, centa, cscr_const.face, 1u);
                    return;
                case EV_SETLOCALWIND:
                    if (eventParm)
                        CG_EnableLocalWindSource(p_nextState->number);
                    else
                        CG_EnableLocalWindSource(1023);
                    return;
                case EV_FLOAT_LONGER:
                    v22 = CG_GetEntity(localClientNum, p_nextState->otherEntityNum);
                    *((_DWORD *)v22 + 201) |= 0x80000u;
                    return;
                case EV_FORCE_BUOYANCY:
                    v23 = CG_GetEntity(localClientNum, p_nextState->otherEntityNum);
                    *((_DWORD *)v23 + 201) |= 0x100000u;
                    return;
                case EV_DISABLE_DEPTH_BUOYANCY_ADJUSTMENTS:
                    v98 = CG_GetEntity(localClientNum, p_nextState->otherEntityNum);
                    userData = 0;
                    if (v98->nitrousVeh)
                    {
                        userData = v98->nitrousVeh->m_phys_user_data;
                    }
                    else if (v98->pose.physObjId && v98->pose.physObjId != -1)
                    {
                        userData = (PhysObjUserData *)v98->pose.physObjId;
                    }
                    if (userData)
                        userData->m_flags |= 8u;
                    return;
                case EV_SCALE_BUOYANCY:
                    v95 = CG_GetEntity(localClientNum, p_nextState->otherEntityNum);
                    buoyancy_scale = (float)p_nextState->eventParm * 0.001;
                    m_phys_user_data = 0;
                    if (v95->nitrousVeh)
                    {
                        m_phys_user_data = v95->nitrousVeh->m_phys_user_data;
                    }
                    else if (v95->pose.physObjId && v95->pose.physObjId != -1)
                    {
                        m_phys_user_data = (PhysObjUserData *)v95->pose.physObjId;
                    }
                    if (m_phys_user_data)
                        m_phys_user_data->buoyancy = (int)(float)((float)m_phys_user_data->buoyancy * buoyancy_scale);
                    return;
                case EV_MAX_EVENTS:
                    if (clientNum == cgameGlob->predictedPlayerState.clientNum)
                    {
                        if (isPlayerView && (ps->pm_flags & 4) == 0)
                            CG_SetInvalidStanceHint(cgameGlob, (invalid_cmd_hint_t)eventParm);
                    }
                    else
                    {
                    LABEL_290:
                        Com_DPrintf(21, "Event %s just for client %i was sent to other clients\n", eventnames[event], clientNum);
                    }
                    return;
                default:
                    Com_Error(ERR_DROP, "Unknown event: '%s'", eventnames[event]);
                    return;
                }
            }
            else
            {
                CG_DoLandEvent(localClientNum, cent, isPlayerView, event - 142, quiet, 1);
                if (clientNum == cgameGlob->predictedPlayerState.clientNum)
                {
                    fallHeight = (float)((float)((float)eventParm * 0.0099999998)
                        * (float)(bg_fallDamageMaxHeight->current.value - bg_fallDamageMinHeight->current.value))
                        + bg_fallDamageMinHeight->current.value;
                    if (fallHeight > 12.0)
                        viewDip = (int)(float)((float)((float)((float)(fallHeight - 12.0) / 26.0) * 4.0) + 4.0);
                    else
                        viewDip = 0;
                    if (viewDip > 24)
                        viewDip = 24;
                    if (viewDip > 0)
                    {
                        cgameGlob->landChange = 0.0 - (float)viewDip;
                        cgameGlob->landTime = cgameGlob->time;
                    }
                }
            }
        }
        else
        {
            CG_DoLandEvent(localClientNum, cent, isPlayerView, event - 111, quiet, 0);
            if (clientNum == cgameGlob->predictedPlayerState.clientNum)
            {
                cgameGlob->landChange = 0.0 - (float)eventParm;
                cgameGlob->landTime = cgameGlob->time;
            }
        }
    }
    else if (cg_debugEvents->current.enabled)
    {
        Com_Printf(21, "CG_EntityEvent:ZERO EVENT\n");
    }
}

bool __cdecl Com_IsRagdollTrajectory(const trajectory_t *trajectory)
{
    if ( !trajectory
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../universal/q_shared.h",
                    2755,
                    0,
                    "%s",
                    "trajectory") )
    {
        __debugbreak();
    }
    return trajectory->trType >= 0xCu && trajectory->trType <= 0xEu;
}

void __cdecl CG_Obituary(int localClientNum, const entityState_s *ent)
{
    const char *v2; // eax
    const char *v3; // eax
    weaponIconRatioType_t killIconRatio; // [esp+24h] [ebp-A0h]
    int mod; // [esp+28h] [ebp-9Ch]
    int attacker; // [esp+2Ch] [ebp-98h]
    bool iconHorzFlip; // [esp+33h] [ebp-91h]
    int weaponNum; // [esp+34h] [ebp-90h]
    float iconWidth; // [esp+38h] [ebp-8Ch]
    const cg_s *cgameGlob; // [esp+3Ch] [ebp-88h]
    int target; // [esp+40h] [ebp-84h]
    float iconHeight; // [esp+44h] [ebp-80h]
    char targetName[40]; // [esp+48h] [ebp-7Ch] BYREF
    char attackerName[40]; // [esp+70h] [ebp-54h] BYREF
    const clientInfo_t *attackerCI; // [esp+9Ch] [ebp-28h]
    const clientInfo_t *victimCI; // [esp+A0h] [ebp-24h]
    char attackerColor; // [esp+A7h] [ebp-1Dh]
    float baseIconSize; // [esp+A8h] [ebp-1Ch]
    const char *s; // [esp+ACh] [ebp-18h]
    char victimColor; // [esp+B3h] [ebp-11h]
    const clientInfo_t *playerCI; // [esp+B4h] [ebp-10h]
    const WeaponDef *weapDef; // [esp+B8h] [ebp-Ch]
    const playerState_s *ps; // [esp+BCh] [ebp-8h]
    Material *iconShader; // [esp+C0h] [ebp-4h]

    victimCI = 0;
    victimColor = 55;
    baseIconSize = 1.4f;
    target = ent->otherEntityNum;
    attacker = ent->attackerEntityNum;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    iconWidth = 1.4f;
    iconHeight = 1.4f;
    iconHorzFlip = 0;
    if ( ent->weaponModel )
    {
        mod = 0;
        weaponNum = ent->eventParm;
        weapDef = BG_GetWeaponDef(weaponNum);
        if ( weapDef->killIcon )
        {
            iconShader = weapDef->killIcon;
            iconHorzFlip = weapDef->flipKillIcon;
            killIconRatio = weapDef->killIconRatio;
            if ( killIconRatio )
            {
                if ( killIconRatio == WEAPON_ICON_RATIO_2TO1 )
                {
                    iconWidth = 1.4 * 2.0;
                }
                else
                {
                    if ( weapDef->killIconRatio != WEAPON_ICON_RATIO_4TO1 )
                    {
                        v2 = BG_WeaponName(weaponNum);
                        v3 = va("killIconRatio %d, weapon %s", weapDef->killIconRatio, v2);
                        if ( !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                                        128,
                                        0,
                                        "%s\n\t%s",
                                        "weapDef->killIconRatio == WEAPON_ICON_RATIO_4TO1",
                                        v3) )
                            __debugbreak();
                    }
                    iconWidth = 1.4 * 2.0;
                    iconHeight = 1.4 * 0.5;
                }
            }
        }
        else
        {
            iconShader = Material_RegisterHandle("hud_obit_death_suicide", 7);
        }
    }
    else
    {
        mod = ent->eventParm;
        weaponNum = 0;
        weapDef = 0;
        iconShader = Material_RegisterHandle("killicondied", 7);
    }
    switch ( mod )
    {
        case 7:
            iconShader = Material_RegisterHandle("hud_obit_knife", 7);
            iconWidth = baseIconSize;
            break;
        case 9:
            iconShader = Material_RegisterHandle("killiconheadshot", 7);
            iconWidth = baseIconSize;
            break;
        case 10:
            iconShader = Material_RegisterHandle("hud_obit_death_crush", 7);
            iconWidth = baseIconSize;
            break;
        case 12:
            iconShader = Material_RegisterHandle("hud_obit_death_falling", 7);
            iconWidth = baseIconSize;
            break;
        case 13:
        case 14:
            iconShader = Material_RegisterHandle("hud_obit_death_suicide", 7);
            iconWidth = baseIconSize;
            break;
        case 16:
            iconShader = Material_RegisterHandle("hud_obit_death_grenade_round", 7);
            iconWidth = baseIconSize;
            break;
        default:
            break;
    }
    if ( target >= 0 && target < com_maxclients->current.integer )
    {
        if ( (unsigned int)target >= 0x20
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                        189,
                        0,
                        "target doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        target,
                        32) )
        {
            __debugbreak();
        }
        victimCI = &cgameGlob->bgs.clientinfo[target];
        if ( !victimCI->infoValid )
            return;
        CL_GetClientName(localClientNum, target, targetName, 38, 1);
        victimColor = CG_DrawScoreboard_GetTeamColorIndex(victimCI->oldteam, localClientNum);
        if ( cgameGlob->clientNum >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                        197,
                        0,
                        "cgameGlob->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        cgameGlob->clientNum,
                        32) )
        {
            __debugbreak();
        }
        playerCI = &cgameGlob->bgs.clientinfo[cgameGlob->clientNum];
        if ( !playerCI->infoValid )
            return;
    }
    else
    {
        Com_Error(ERR_DROP, "CG_Obituary: target out of range");
    }
    if ( attacker < 0 || attacker >= com_maxclients->current.integer )
    {
        attacker = 1022;
        attackerCI = 0;
        attackerName[0] = 0;
        attackerColor = 55;
LABEL_40:
        ps = &cgameGlob->nextSnap->ps;
        if ( attacker == target )
        {
            attackerName[0] = 0;
        }
        else if ( weapDef && !strcmp(BG_WeaponName(weaponNum), "syrette_mp") )
        {
            if ( attacker == ps->clientNum )
            {
                if ( !cgameGlob->inKillCam )
                {
                    s = va("CGAME_YOUREVIVED", targetName);
                    CG_PriorityCenterPrint(localClientNum, s, 0);
                }
            }
            else if ( target == ps->clientNum && attackerCI )
            {
                s = va("CGAME_YOUWEREREVIVED", attackerName);
                CG_PriorityCenterPrint(localClientNum, s, 0);
            }
        }
        else if ( attacker == ps->clientNum )
        {
            if ( !cgameGlob->inKillCam )
                CG_DisplayPlayerCard(localClientNum, attackerCI, victimCI, 0);
        }
        else if ( target == ps->clientNum && attackerCI && !cgameGlob->inKillCam )
        {
            CG_DisplayPlayerCard(localClientNum, attackerCI, victimCI, 1);
        }
        if ( !cgameGlob->inKillCam )
            CL_DeathMessagePrint(
                localClientNum,
                attackerName,
                attackerColor,
                targetName,
                victimColor,
                iconShader,
                iconWidth,
                iconHeight,
                iconHorzFlip);
        return;
    }
    if ( (unsigned int)attacker >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                    215,
                    0,
                    "attacker doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    attacker,
                    32) )
    {
        __debugbreak();
    }
    attackerCI = &cgameGlob->bgs.clientinfo[attacker];
    if ( attackerCI->infoValid )
    {
        CL_GetClientName(localClientNum, attacker, attackerName, 38, 1);
        attackerColor = CG_DrawScoreboard_GetTeamColorIndex(attackerCI->oldteam, localClientNum);
        goto LABEL_40;
    }
}

void __cdecl CG_ObituaryRevive(int localClientNum, const entityState_s *ent)
{
    char *v2; // eax
    const cg_s *cgameGlob; // [esp+14h] [ebp-48h]
    char textBuffer[40]; // [esp+1Ch] [ebp-40h] BYREF
    const char *gamerTag; // [esp+48h] [ebp-14h]
    float baseIconSize; // [esp+4Ch] [ebp-10h]
    const char *s; // [esp+50h] [ebp-Ch]
    int clientNum; // [esp+54h] [ebp-8h]
    Material *iconShader; // [esp+58h] [ebp-4h]

    baseIconSize = 1.4f;
    s = 0;
    gamerTag = 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    clientNum = ent->eventParms[0];
    if ( CL_GetClientName(localClientNum, clientNum, textBuffer, 38, 1) )
    {
        gamerTag = textBuffer;
        if ( !cg_gameMessageWidth
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                        332,
                        0,
                        "%s",
                        "cg_gameMessageWidth") )
        {
            __debugbreak();
        }
        v2 = UI_SafeTranslateString("CGAME_NEEDS_REVIVE");
        s = va("%s %s", gamerTag, v2);
        iconShader = Material_RegisterHandle("hud_icon_syrette", 7);
        if ( !cgameGlob->inKillCam )
            CL_ReviveMessagePrint(localClientNum, (char *)s, iconShader, 1.4, 1.4, 0);
    }
    else
    {
        Com_PrintError(14, "Unable to get name for client num: %i\n", cgameGlob->clientNum);
    }
}

void __cdecl CG_DirectionalHitIndicator(int localClientNum, const entityState_s *ent)
{
    float *entOrigin; // [esp+Ch] [ebp-34h]
    int i; // [esp+1Ch] [ebp-24h]
    unsigned __int64 fullVictimEntBitArray; // [esp+20h] [ebp-20h]
    cg_s *cgameGlob; // [esp+2Ch] [ebp-14h]
    int slot; // [esp+30h] [ebp-10h]
    centity_s *cent; // [esp+34h] [ebp-Ch]
    int victimEntNum; // [esp+38h] [ebp-8h]

    fullVictimEntBitArray = ent->eventParms[0] + ((unsigned __int64)ent->eventParms[1] << 32);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    for ( victimEntNum = 0; victimEntNum < 32 && fullVictimEntBitArray; ++victimEntNum )
    {
        if ( (fullVictimEntBitArray & 1) != 0 )
        {
            slot = 0;
            for ( i = 0; i < 4; ++i )
            {
                if ( cgameGlob->directionalHitIndicator[i].time < cgameGlob->directionalHitIndicator[slot].time )
                    slot = i;
            }
            cent = CG_GetEntity(localClientNum, victimEntNum);
            if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp", 372, 0, "%s", "cent") )
                __debugbreak();
            if ( cent )
            {
                cgameGlob->directionalHitIndicator[slot].time = cgameGlob->snap->serverTime;
                entOrigin = cgameGlob->directionalHitIndicator[slot].entOrigin;
                *entOrigin = cent->pose.origin[0];
                entOrigin[1] = cent->pose.origin[1];
                entOrigin[2] = cent->pose.origin[2];
                cgameGlob->directionalHitIndicator[slot].entOrigin[2] = 0.0f;
            }
        }
        fullVictimEntBitArray >>= 1;
    }
}

void __cdecl CG_ItemPickup(int localClientNum, int weapIndex)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    weapDef = BG_GetWeaponDef(weapIndex);
    if ( weapDef->weapClass != WEAPCLASS_ITEM )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        if ( weapDef->offhandClass )
        {
            if ( !cgameGlob->equippedOffHand || cgameGlob->predictedPlayerState.offhandPrimary == PLAYER_OFFHAND_PRIMARY_GEAR )
                CG_SetEquippedOffHand(localClientNum, weapIndex);
        }
        else if ( !cgameGlob->weaponSelect )
        {
            CG_SelectWeaponIndex(localClientNum, weapIndex);
        }
    }
}

void __cdecl CG_PlayFx(int localClientNum, centity_s *cent, const float *angles)
{
    cg_s *LocalClientGlobals; // eax
    const FxEffectDef *fxDef; // [esp+4h] [ebp-30h]
    int fxId; // [esp+Ch] [ebp-28h]
    float axis[3][3]; // [esp+10h] [ebp-24h] BYREF

    fxId = cent->nextState.eventParm;
    if ( cent->nextState.eventParm && cent->nextState.eventParm < 0xC4u )
    {
        fxDef = CG_GetLocalClientStaticGlobals(localClientNum)->fxs[fxId];
        if ( !fxDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp", 574, 0, "%s", "fxDef") )
            __debugbreak();
        AnglesToAxis(angles, axis);
        LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
        FX_PlayOrientedEffect(localClientNum, fxDef, LocalClientGlobals->time, cent->pose.origin, axis);
    }
    else
    {
        Com_PrintError(21, "ERROR: CG_PlayFx called with invalid effect id %i\n", fxId);
    }
}

void __cdecl CG_SetInvalidCmdHint(cg_s *cgameGlob, InvalidCmdHintType hintType)
{
    if ( cgameGlob->invalidCmdHintType != hintType )
    {
        cgameGlob->invalidCmdHintType = hintType;
        cgameGlob->invalidCmdHintTime = cgameGlob->time;
    }
}

void __cdecl CG_SetInvalidStanceHint(cg_s *cgameGlob, invalid_cmd_hint_t type)
{
    switch ( type )
    {
        case INVALID_CMD_STAND_HINT:
            CG_SetInvalidCmdHint(cgameGlob, INVALID_CMD_STAND_BLOCKED);
            break;
        case INVALID_CMD_CROUCH_HINT:
            CG_SetInvalidCmdHint(cgameGlob, INVALID_CMD_CROUCH_BLOCKED);
            break;
        case INVALID_CMD_CROUCH_HINT|INVALID_CMD_STAND_HINT:
            CG_SetInvalidCmdHint(cgameGlob, INVALID_CMD_TARGET_TOO_CLOSE);
            break;
        default:
            if ( type )
            {
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                                710,
                                0,
                                "%s",
                                "type == INVALID_CMD_NO_HINT") )
                    __debugbreak();
            }
            break;
    }
}

void __cdecl CG_StopWeaponSound(
                int localClientNum,
                bool isPlayerView,
                const WeaponDef *weaponDef,
                int entitynum,
                weaponstate_t weaponstate)
{
    unsigned int AliasId; // eax
    unsigned int v6; // eax
    unsigned int v7; // eax
    unsigned int v8; // eax
    unsigned int v9; // eax

    switch ( weaponstate )
    {
        case WEAPON_RELOADING:
        case WEAPON_RELOADING_INTERUPT:
            if ( isPlayerView )
            {
                if ( weaponDef->reloadEmptySoundPlayer )
                {
                    AliasId = SND_FindAliasId((char *)weaponDef->reloadEmptySoundPlayer);
                    CG_StopSoundAlias(localClientNum, entitynum, AliasId);
                }
                if ( weaponDef->reloadSoundPlayer )
                {
                    v6 = SND_FindAliasId((char *)weaponDef->reloadSoundPlayer);
                    CG_StopSoundAlias(localClientNum, entitynum, v6);
                }
            }
            else
            {
                if ( weaponDef->reloadEmptySound )
                {
                    v7 = SND_FindAliasId((char *)weaponDef->reloadEmptySound);
                    CG_StopSoundAlias(localClientNum, entitynum, v7);
                }
                if ( weaponDef->reloadSound )
                {
                    v8 = SND_FindAliasId((char *)weaponDef->reloadSound);
                    CG_StopSoundAlias(localClientNum, entitynum, v8);
                }
            }
            break;
        case WEAPON_RELOAD_START:
        case WEAPON_RELOAD_START_INTERUPT:
            if ( isPlayerView )
                v9 = SND_FindAliasId((char *)weaponDef->reloadStartSoundPlayer);
            else
                v9 = SND_FindAliasId((char *)weaponDef->reloadStartSound);
            goto LABEL_19;
        case WEAPON_RELOAD_END:
            if ( isPlayerView )
                v9 = SND_FindAliasId((char *)weaponDef->reloadEndSoundPlayer);
            else
                v9 = SND_FindAliasId((char *)weaponDef->reloadEndSound);
LABEL_19:
            CG_StopSoundAlias(localClientNum, entitynum, v9);
            break;
        default:
            return;
    }
}

void __cdecl CG_PhysLaunch(int localClientNum, centity_s *cent, const entityState_s *es)
{
    float hitp[3]; // [esp+18h] [ebp-10h] BYREF
    bool relativeHitP; // [esp+27h] [ebp-1h]

    if ( CL_LocalClient_IsFirstActive(localClientNum) )
    {
        if ( cent->nitrousVeh )
        {
            relativeHitP = es->eventParm != 0;
            if ( relativeHitP )
            {
                hitp[0] = es->lerp.pos.trDelta[0];
                hitp[1] = es->lerp.pos.trDelta[1];
                hitp[2] = es->lerp.pos.trDelta[2];
            }
            else if ( (float)((float)((float)(es->lerp.pos.trDelta[0] * es->lerp.pos.trDelta[0])
                                                            + (float)(es->lerp.pos.trDelta[1] * es->lerp.pos.trDelta[1]))
                                            + (float)(es->lerp.pos.trDelta[2] * es->lerp.pos.trDelta[2])) <= 0.0 )
            {
                memset(hitp, 0, sizeof(hitp));
            }
            else
            {
                hitp[0] = es->lerp.pos.trDelta[0] + cent->pose.origin[0];
                hitp[1] = es->lerp.pos.trDelta[1] + cent->pose.origin[1];
                hitp[2] = es->lerp.pos.trDelta[2] + cent->pose.origin[2];
            }
            if ( es->un1.scale )
                cent->nitrousVeh->m_flags |= 0x1000u;
            Vehicle_Launch(localClientNum, cent, hitp, es->lerp.pos.trBase, relativeHitP);
        }
        else
        {
            if ( !cent->pose.isRagdoll && Com_IsRagdollTrajectory(&cent->nextState.lerp.pos) && !cent->pose.ragdollHandle )
                CG_CreateRagdollObject(localClientNum, cent);
            if ( cent->pose.isRagdoll && cent->pose.ragdollHandle > 0 )
                Ragdoll_Launch(
                    localClientNum,
                    cent->pose.ragdollHandle,
                    es->lerp.pos.trBase,
                    (hitLocation_t)es->eventParm);
        }
    }
}

void __cdecl CG_CheckEvents(int localClientNum, centity_s *cent)
{
    char *EntityTypeName; // eax

    if ( cent->nextState.eventSequence
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_event.cpp",
                    2355,
                    0,
                    "%s",
                    "!cent->nextState.eventSequence") )
    {
        __debugbreak();
    }
    if ( !cent->previousEventSequence )
    {
        cent->previousEventSequence = 1;
        EntityTypeName = BG_GetEntityTypeName(cent->nextState.eType);
        {
            PROF_SCOPED("CG_EntityEvent");
            ZoneTextF("Type: %s", EntityTypeName);

            CG_CalcEntityLerpPositions(localClientNum, cent);
            CG_EntityEvent(localClientNum, cent, cent->nextState.eType - 21);
        }
    }
}

void __cdecl CG_CheckEntEvents(int localClientNum, centity_s *cent)
{
    char *EntityTypeName; // eax
    __int16 v3; // [esp+8h] [ebp-10h]
    __int16 previousEventSequence; // [esp+Ah] [ebp-Eh]
    int event; // [esp+Ch] [ebp-Ch]
    unsigned __int16 oldEventParm; // [esp+10h] [ebp-8h]
    int i; // [esp+14h] [ebp-4h]

    if ( cent->nextState.eventSequence )
    {
        previousEventSequence = cent->previousEventSequence;
        if ( previousEventSequence <= cent->nextState.eventSequence + 64 )
            v3 = cent->previousEventSequence;
        else
            v3 = previousEventSequence - 256;
        cent->previousEventSequence = v3;
        if ( cent->nextState.eventSequence - cent->previousEventSequence > 4 )
            cent->previousEventSequence = cent->nextState.eventSequence - 4;
        if ( cent->previousEventSequence < cent->nextState.eventSequence )
        {
            if ( cent->nextState.eType != 17 )
            {
                CG_CalcEntityLerpPositions(localClientNum, cent);
                if ( CG_EntityLinked(localClientNum, cent) )
                    CG_LinkTransformForEntity(localClientNum, cent, cent->pose.origin, cent->pose.angles);
            }
            oldEventParm = cent->nextState.eventParm;
            for ( i = cent->previousEventSequence; i != cent->nextState.eventSequence; ++i )
            {
                event = cent->nextState.events[i & 3];
                cent->nextState.eventParm = cent->nextState.eventParms[i & 3];
                EntityTypeName = BG_GetEntityTypeName(event + 21);
                {
                    PROF_SCOPED("CG_EntityEvent");
                    ZoneTextF("Type: %s", EntityTypeName);

                    CG_EntityEvent(localClientNum, cent, event);
                }

            }
            cent->nextState.eventParm = oldEventParm;
            cent->previousEventSequence = cent->nextState.eventSequence;
        }
        else
        {
            cent->previousEventSequence = cent->nextState.eventSequence;
        }
    }
    else
    {
        cent->previousEventSequence = 0;
    }
}

bool __cdecl CG_CheckIfDualWieldEmpty(playerState_s *ps, unsigned int weapIndex)
{
    const WeaponDef *weapDef; // [esp+10h] [ebp-4h]

    if ( !weapIndex )
        weapIndex = ps->weapon;
    weapDef = BG_GetWeaponDef(weapIndex);
    return BG_GetAmmoInClip(ps, weapIndex) || BG_GetAmmoInClip(ps, weapDef->dualWieldWeaponIndex);
}

