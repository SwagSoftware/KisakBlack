#include "destructible.h"
#include <game_mp/g_utils_mp.h>
#include <game_mp/g_main_mp.h>
#include <cgame_mp/cg_local_mp.h>
#include "xdoll.h"
#include <universal/mem_userhunk.h>
#include <xanim/xmodel_utils.h>
#include <server_mp/sv_init_mp.h>
#include <sound/snd_bank.h>
#include <xanim/dobj.h>
#include <xanim/xmodel.h>
#include <cgame_mp/cg_main_mp.h>
#include <clientscript/scr_const.h>
#include <game_mp/g_spawn_mp.h>
#include <bgame/bg_public.h>
#include <game/g_scr_vehicle.h>
#include <DynEntity/DynEntity_client.h>
#include <game/g_scr_mover.h>
#include <universal/com_math_anglevectors.h>
#include <cgame_mp/cg_pose_mp.h>
#include <cgame_mp/cg_ents_mp.h>
#include <client_mp/cl_cgame_mp.h>
#include <qcommon/dobj_management.h>
#include <client/splitscreen.h>
#include <EffectsCore/fx_system.h>
#include <cgame/cg_event.h>
#include <cgame/cg_sound.h>
#include <EffectsCore/fx_unique_handle.h>
#include <xanim/dobj_utils.h>
#include <EffectsCore/fx_marks.h>
#include <cgame_mp/cg_animtree_mp.h>
#include <clientscript/cscr_vm.h>
#include <cgame/cg_main.h>
#include <universal/com_memory.h>
#include "destructibledef_load_obj.h"
#include "physics_system.h"
#include <cgame/cg_drawtools.h>
#include <clientscript/cscr_stringlist.h>
#include <cgame/cg_scr_main.h>

float bullet_force = 1.4f;
float grenade_force = 10.0f;
float burn_force = 1.5f;

Destructible *g_DebugRenderDestructible;
int g_DebugLastPieceIndex[32];
int g_DebugLastPieceDamage[32];

DestructiblePose s_cgDestructiblePoses[2];

destructible_event_t g_destructible_events[64];
int g_destructible_events_count;

unsigned __int8 g_cgPieceArrayBuffer[73728];
int s_num_destructible_gamestates[1];
destructible_gamestate s_destructible_gamestates[1][32];

int g_numDisabledLables;
int g_disabledLabels[10];

Destructible s_destructibles[64];
unsigned int s_numDestructibles;
int cg_numDisabledLables;

int cg_disabledLabels[10];
Destructible *cg_destructibles[1];

HunkUser *g_pieceArrayHunk;
HunkUser *g_cgPieceArrayHunk;

int cg_updateTime[1];
unsigned int cg_numDestructibles[1];

unsigned __int8 g_pieceArrayBuffer[73728];

bool __cdecl hasLabel(DestructibleDef *ddef, unsigned __int16 label)
{
    return Destructible_GetPieceIndexForLabel(ddef, label) >= 0;
}

int __cdecl Destructible_GetPieceIndexForLabel(DestructibleDef *ddef, unsigned __int16 enableLabel)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < ddef->numPieces; ++i )
    {
        if ( ddef->pieces[i].enableLabel == enableLabel )
            return i;
    }
    return -1;
}

int __cdecl Destructible_GetPieceMaxHealth(DestructibleDef *ddef, int pieceIndex)
{
    if ( pieceIndex < 0 || pieceIndex >= ddef->numPieces )
        return -1;
    else
        return ddef->pieces[pieceIndex].health;
}

int __cdecl Destructible_GetPieceHealth(Destructible *destructible, int pieceIndex)
{
    if ( pieceIndex < 0 || pieceIndex >= destructible->ddef->numPieces )
        return -1;
    else
        return destructible->pieceArray[pieceIndex].health;
}

void __cdecl EnableAllDestructiblePieces()
{
    g_numDisabledLables = 0;
}

void __cdecl DisableDestructiblePiece(int label)
{
    unsigned int j; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    for ( i = 0; i < g_numDisabledLables; ++i )
    {
        if ( g_disabledLabels[i] == label )
            return;
    }
    g_disabledLabels[g_numDisabledLables++] = label;
    for ( j = 0; j < s_numDestructibles; ++j )
    {
        if (hasLabel(s_destructibles[j].ddef, label))
            G_DObjUpdate(&g_entities[s_destructibles[j].entNum]);
    }
}

char __cdecl IsDestructiblePieceDisabled(int label)
{
    int i; // [esp+0h] [ebp-4h]

    if ( !label )
        return 0;
    for ( i = 0; i < g_numDisabledLables; ++i )
    {
        if ( g_disabledLabels[i] == label )
            return 1;
    }
    return 0;
}

void __cdecl CG_EnableAllDestructiblePieces()
{
    cg_numDisabledLables = 0;
}

void __cdecl CG_DisableDestructiblePiece(int localClientNum, int label)
{
    centity_s *cent; // [esp+8h] [ebp-Ch]
    unsigned int j; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    for ( i = 0; i < cg_numDisabledLables; ++i )
    {
        if ( cg_disabledLabels[i] == label )
            return;
    }
    cg_disabledLabels[cg_numDisabledLables++] = label;
    for ( j = 0; j < 0x90; ++j )
    {
        if ( cg_destructibles[localClientNum][j].entNum != 1023 && hasLabel(cg_destructibles[localClientNum][j].ddef, label) )
        {
            cent = CG_GetEntity(localClientNum, cg_destructibles[localClientNum][j].entNum);
            CG_DestructibleUpdateDObj(cent);
        }
    }
}

char __cdecl CG_IsDestructiblePieceDisabled(int label)
{
    int i; // [esp+0h] [ebp-4h]

    if ( !label )
        return 0;
    for ( i = 0; i < cg_numDisabledLables; ++i )
    {
        if ( cg_disabledLabels[i] == label )
            return 1;
    }
    return 0;
}

int __cdecl CG_GetDestructibleIndex(int localClientNum, const Destructible *destructible)
{
    if ( !destructible
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp", 418, 0, "%s", "destructible") )
    {
        __debugbreak();
    }
    return destructible - cg_destructibles[localClientNum] + 1;
}

Destructible *__cdecl CG_GetDestructibleByIndex(int localClientNum, int index)
{
    unsigned int indexa; // [esp+Ch] [ebp+Ch]

    indexa = index - 1;
    if ( indexa >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                    434,
                    0,
                    "index doesn't index MAX_DESTRUCTIBLES\n\t%i not in [0, %i)",
                    indexa,
                    64) )
    {
        __debugbreak();
    }
    return &cg_destructibles[localClientNum][indexa];
}

Destructible *__cdecl Destructible_GetFreeDestructible()
{
    const char *v1; // eax
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 64; ++i )
    {
        if ( s_destructibles[i].entNum == 1023 )
        {
            ++s_numDestructibles;
            s_destructibles[i].oldestBurnTime = 0;
            s_destructibles[i].destructiblePoseID = 0;
            return &s_destructibles[i];
        }
    }
    v1 = va("Ran out of destructibles Max is %d", 64);
    Com_Error(ERR_DROP, v1);
    return 0;
}

Destructible *__cdecl Destructible_GetDestructible(int entNum)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 64; ++i )
    {
        if ( s_destructibles[i].entNum == entNum )
            return &s_destructibles[i];
    }
    return 0;
}

void __cdecl Destructible_FreeDestructible(Destructible *destructible)
{
    int i; // [esp+0h] [ebp-4h]

    if ( destructible->entNum != 1023 )
    {
        for ( i = 0; i < destructible->pieceCount; ++i )
            XDoll_DestroyXDoll(destructible->pieceArray[i].xdollHandle);
        Hunk_UserFree(g_pieceArrayHunk, destructible->pieceArray);
        destructible->pieceArray = 0;
        --s_numDestructibles;
        destructible->entNum = 1023;
        destructible->ddef = 0;
        destructible->oldestBurnTime = 0;
        destructible->flags = 0;
        memset(destructible->states, 0, sizeof(destructible->states));
        if ( destructible->destructiblePoseID )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                            498,
                            0,
                            "%s",
                            "destructible->destructiblePoseID == NULL") )
                __debugbreak();
        }
        destructible->destructiblePoseID = 0;
    }
}

Destructible *__cdecl CG_GetFreeDestructible(int localClientNum, int entNum)
{
    int j; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 144; ++i )
    {
        if ( cg_destructibles[localClientNum][i].entNum == entNum )
        {
            cg_destructibles[localClientNum][i].oldestBurnTime = 0;
            cg_destructibles[localClientNum][i].destructiblePoseID = 0;
            return &cg_destructibles[localClientNum][i];
        }
    }
    for ( j = 0; j < 144; ++j )
    {
        if ( cg_destructibles[localClientNum][j].entNum == 1023 )
        {
            ++cg_numDestructibles[localClientNum];
            cg_destructibles[localClientNum][j].oldestBurnTime = 0;
            cg_destructibles[localClientNum][j].destructiblePoseID = 0;
            return &cg_destructibles[localClientNum][j];
        }
    }
    Com_Error(ERR_DROP, "Ran out of cg destructibles");
    return 0;
}

void __cdecl CG_FreeDestructible(int localClientNum, Destructible *destructible)
{
    int i; // [esp+0h] [ebp-4h]

    if ( destructible->entNum != 1023 && destructible->ddef )
    {
        for ( i = 0; i < destructible->ddef->numPieces; ++i )
            XDoll_DestroyXDoll(destructible->pieceArray[i].xdollHandle);
        Hunk_UserFree(g_cgPieceArrayHunk, destructible->pieceArray);
        destructible->pieceArray = 0;
        --cg_numDestructibles[localClientNum];
        destructible->entNum = 1023;
        destructible->ddef = 0;
        destructible->oldestBurnTime = 0;
        destructible->flags = 0;
        memset(destructible->states, 0, sizeof(destructible->states));
        if ( destructible->destructiblePoseID )
            CG_FreeDestructiblePose(localClientNum, destructible->destructiblePoseID);
        destructible->destructiblePoseID = 0;
    }
}

void __cdecl CG_InitDestructibles(int localClientNum)
{
    Destructible *v1; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    memset(cg_destructibles[localClientNum], 0, 0x60C0u);
    for ( i = 0; i < 144; ++i )
    {
        v1 = &cg_destructibles[localClientNum][i];
        v1->entNum = 1023;
        v1->states[0].time = 1;
        v1->states[0].state.modelState0 = 0x7FFFFFFF;
    }
    cg_numDestructibles[localClientNum] = 0;
    cg_updateTime[localClientNum] = 0;
}

void __cdecl G_InitDestructibles()
{
    Destructible *v0; // eax
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 64; ++i )
    {
        s_destructibles[i].entNum = 1023;
        v0 = &s_destructibles[i];
        v0->states[0].time = 1;
        v0->states[0].state.modelState0 = 0x7FFFFFFF;
    }
}

int __cdecl Destructible_GetNumStagesForPiece(DestructibleDef *ddef, int pieceIndex)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 5; ++i )
    {
        if ( !ddef->pieces[pieceIndex].stages[i].showBone )
            return i;
    }
    return -1;
}

int __cdecl Destructible_GetShowBoneNameForPiece(Destructible *destructible, int pieceIndex)
{
    int i; // [esp+0h] [ebp-14h]
    int boneName; // [esp+8h] [ebp-Ch]
    int stage; // [esp+Ch] [ebp-8h]
    int stagea; // [esp+Ch] [ebp-8h]
    DestructibleDef *ddef; // [esp+10h] [ebp-4h]

    ddef = destructible->ddef;
    stage = Destructible_GetDestructibleStage(ddef, pieceIndex, destructible->pieceArray[pieceIndex].health);
    boneName = 0;
    if ( stage >= 0 )
        return ddef->pieces[pieceIndex].stages[stage].showBone;
    for ( i = 0; i < 5; ++i )
    {
        if ( !ddef->pieces[pieceIndex].stages[i].showBone )
        {
            stagea = i - 1;
            goto LABEL_9;
        }
    }
    stagea = 0;
LABEL_9:
    if ( stagea < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp", 684, 0, "%s", "stage >= 0") )
    {
        __debugbreak();
    }
    if ( ddef->pieces[pieceIndex].stages[stagea].physPreset && destructible->pieceArray[pieceIndex].xdollHandle )
        return ddef->pieces[pieceIndex].stages[stagea].showBone;
    return boneName;
}

int __cdecl Destructible_GetDestructibleStage(const DestructibleDef *ddef, int pieceIndex, int health)
{
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; ; ++i )
    {
        if ( i >= 5 )
            return -1;
        if ( (float)health >= (float)((float)ddef->pieces[pieceIndex].health * ddef->pieces[pieceIndex].stages[i].breakHealth) )
        {
            if ( ddef->pieces[pieceIndex].stages[i].showBone )
                return i;
            else
                return -1;
        }
        if ( !ddef->pieces[pieceIndex].stages[i].showBone )
            break;
    }
    if ( pieceIndex )
        return -1;
    else
        return i - 1;
}

void __cdecl Destructible_GetHideParts(Destructible *destructible, unsigned int *partBits)
{
    unsigned __int8 boneIndex; // [esp+7h] [ebp-15h] BYREF
    int boneName; // [esp+8h] [ebp-14h]
    unsigned __int8 k; // [esp+Fh] [ebp-Dh]
    int j; // [esp+10h] [ebp-Ch]
    unsigned __int8 i; // [esp+17h] [ebp-5h]
    DestructibleDef *ddef; // [esp+18h] [ebp-4h]

    ddef = destructible->ddef;
    for ( i = 0; i < ddef->numPieces; ++i )
    {
        for ( j = 0; j < 5; ++j )
            partBits[j] |= ddef->pieces[i].hideBones[j];
    }
    for ( k = 0; k < ddef->numPieces; ++k )
    {
        boneName = Destructible_GetShowBoneNameForPiece(destructible, k);
        if ( boneName )
        {
            boneIndex = 0;
            if ( XModelGetBoneIndex(ddef->model, boneName, 0, &boneIndex) )
                partBits[(int)boneIndex >> 5] &= ~(0x80000000 >> (boneIndex & 0x1F));
        }
    }
}

int __cdecl Destructible_GetPieceParentBone(DestructibleDef *ddef, int pieceIndex)
{
    return XModelGetParentBoneName(ddef->model, ddef->pieces[pieceIndex].stages[0].showBone);
}

PhysPreset *__cdecl Destructible_GetPiecePhysPreset(DestructibleDef *ddef, int pieceIndex)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 5; ++i )
    {
        if ( ddef->pieces[pieceIndex].stages[i].physPreset )
            return ddef->pieces[pieceIndex].stages[i].physPreset;
    }
    return 0;
}

unsigned int __cdecl DestructibleUpdate(gentity_s *ent, DObjModel_s *dobjModels, unsigned int numModels)
{
    const char *v4; // eax
    XModel *pristineModel; // eax
    int stage; // [esp+8h] [ebp-814h]
    char *loopSoundName; // [esp+Ch] [ebp-810h]
    unsigned __int8 i; // [esp+13h] [ebp-809h]
    char buffer[1024]; // [esp+14h] [ebp-808h] BYREF
    char destructibleName[1024]; // [esp+414h] [ebp-408h] BYREF
    DestructibleDef *ddef; // [esp+818h] [ebp-4h]

    if ( (ent->s.lerp.eFlags & 0x20000) == 0
        || ent->s.eType == 1
        || ent->s.eType == 2
        || ent->s.eType == 17
        || ent->s.eType == 19 )
    {
        return numModels;
    }
    if ( ent->destructible )
    {
        if ( ent->destructible->ddef )
        {
            ddef = ent->destructible->ddef;
        }
        else
        {
            SV_GetConfigstring(ent->s.un1.scale + 3180, destructibleName, 1024);
            ddef = DestructibleDefPrecache(destructibleName);
            ent->destructible->ddef = ddef;
            if ( ddef )
                ent->destructible->pieceCount = ddef->numPieces;
        }
        if ( !ddef
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp", 759, 0, "%s", "ddef") )
        {
            __debugbreak();
        }
    }
    else
    {
        SV_GetConfigstring(ent->s.un1.scale + 3180, buffer, 1024);
        ddef = DestructibleDefPrecache(buffer);
    }
    if ( !ddef )
        return numModels;
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp", 771, 0, "%s", "ent") )
        __debugbreak();
    if ( !dobjModels
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp", 772, 0, "%s", "dobjModels") )
    {
        __debugbreak();
    }
    if ( ent->destructible )
    {
        if ( ent->destructible->entNum != ent->s.number
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                        786,
                        0,
                        "%s",
                        "ent->destructible->entNum == ent->s.number") )
        {
            __debugbreak();
        }
    }
    else
    {
        ent->destructible = Destructible_GetFreeDestructible();
        ent->destructible->ddef = ddef;
        if ( !ddef
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp", 778, 0, "%s", "ddef") )
        {
            __debugbreak();
        }
        ent->health = ddef->pieces->health;
        ent->destructible->pieceCount = ddef->numPieces;
    }
    if ( ent->destructible )
    {
        if ( ent->s.number == 1023
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                        791,
                        0,
                        "%s",
                        "ent->s.number != ENTITYNUM_NONE") )
        {
            __debugbreak();
        }
        ent->destructible->entNum = ent->s.number;
    }
    else
    {
        Com_Error(ERR_DROP, "Hit max destructible count [%d] when creating %s", 64, ddef->name);
    }
    if ( !ent->destructible->ddef
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                    799,
                    0,
                    "%s",
                    "ent->destructible->ddef") )
    {
        __debugbreak();
    }
    if ( numModels != 1
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp", 802, 0, "%s", "numModels == 1") )
    {
        __debugbreak();
    }
    if ( (ent->destructible->flags & 2) == 0 )
    {
        if ( !g_pieceArrayHunk )
            g_pieceArrayHunk = Hunk_UserCreateFromBuffer(
                                                     g_pieceArrayBuffer,
                                                     73728,
                                                     HU_SCHEME_FIRSTFIT,
                                                     0,
                                                     0,
                                                     "DesctructiblePieces",
                                                     0);
        ent->destructible->pieceArray = (DESTRUCTIBLE_PIECE_INFO *)Hunk_UserAlloc(
                                                                                                                                 g_pieceArrayHunk,
                                                                                                                                 24 * ddef->numPieces,
                                                                                                                                 4,
                                                                                                                                 0);
        memset(ent->destructible->pieceArray, 0, 24 * ddef->numPieces);
        if ( !ent->destructible->pieceArray
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                        813,
                        0,
                        "%s",
                        "ent->destructible->pieceArray") )
        {
            __debugbreak();
        }
    }
    for ( i = 0; i < ddef->numPieces; ++i )
    {
        if ( !ddef->pieces[i].stages[0].showBone )
        {
            v4 = va("Destructible %s Piece %d first stage can't find bone", ddef->name, i);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                            817,
                            0,
                            "%s\n\t%s",
                            "ddef->pieces[i].stages[0].showBone",
                            v4) )
                __debugbreak();
        }
        if ( (ent->destructible->flags & 2) == 0 )
        {
            ent->destructible->pieceArray[i].health = ddef->pieces[i].health;
            ent->destructible->pieceArray[i].burnData.fx = 0;
            ent->destructible->pieceArray[i].burnData.burnTime = 0;
            ent->destructible->pieceArray[i].burnData.sndId = 0;
        }
        if ( IsDestructiblePieceDisabled(ddef->pieces[i].enableLabel) )
        {
            ent->destructible->pieceArray[i].health = -1;
        }
        else if ( !i )
        {
            if ( (ent->destructible->flags & 4) != 0 )
                pristineModel = ddef->pristineModel;
            else
                pristineModel = ddef->model;
            dobjModels->model = pristineModel;
            if ( level.initializing )
                G_SetModel(ent, (char *)dobjModels->model->name);
            if ( (ent->destructible->flags & 2) == 0 )
            {
                stage = Destructible_GetDestructibleStage(ddef, 0, ent->destructible->pieceArray->health);
                loopSoundName = (char *)ddef->pieces->stages[stage].loopSound;
                if ( loopSoundName )
                {
                    ent->s.loopSoundId = SND_FindAliasId(loopSoundName);
                    ent->s.loopSoundFade = 1000;
                }
            }
        }
    }
    if ( ent->destructible )
    {
        ent->takedamage = 1;
        ent->destructible->flags |= 2u;
        Destructible_GetHideParts(ent->destructible, ent->s.partBits);
    }
    return numModels;
}

char __cdecl Destructible_GetPieceIndexFromBoneName(
                Destructible *destructible,
                int modelIndex,
                unsigned int boneName,
                unsigned __int8 *index)
{
    int i; // [esp+4h] [ebp-8h]
    DestructibleDef *ddef; // [esp+8h] [ebp-4h]

    ddef = destructible->ddef;
    *index = 0;
    if ( modelIndex )
        return 0;
    for ( i = 0; i < ddef->numPieces; ++i )
    {
        if ( Destructible_GetShowBoneNameForPiece(destructible, i) == boneName )
        {
            *index = i;
            return 1;
        }
    }
    return 1;
}

int __cdecl CG_Destructible_GetModelIndexFromLabel()
{
    //    centity_s *self,
    //DObj *obj,
    // unsigned int label
    return -1; // function is simplified by IDA because result is the same even if check passes
}

int __cdecl GetHealthFromState(
                const LerpEntityStateDestructibleHit *state,
                DestructibleDef *ddef,
                unsigned __int8 pieceIndex)
{
    DestructiblePiece *piece; // [esp+0h] [ebp-8h]
    int stage; // [esp+4h] [ebp-4h]

    if ( pieceIndex >= ddef->numPieces
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                    954,
                    0,
                    "%s",
                    "pieceIndex < ddef->numPieces") )
    {
        __debugbreak();
    }
    piece = &ddef->pieces[pieceIndex];
    if ( (state->modelState0 & (1 << pieceIndex)) != 0 )
        return piece->health;
    if ( (state->modelState1 & (1 << pieceIndex)) != 0 )
    {
        stage = 1;
    }
    else if ( (state->modelState2 & (1 << pieceIndex)) != 0 )
    {
        stage = 2;
    }
    else if ( (state->modelState3 & (1 << pieceIndex)) != 0 )
    {
        stage = 3;
    }
    else
    {
        if ( (state->modelState4 & (1 << pieceIndex)) == 0 )
            return -1;
        stage = 4;
    }
    return (int)(float)((float)piece->health * piece->stages[stage - 1].breakHealth) - 1;
}

void __cdecl DestructibleEventSetPieceState(gentity_s *self, LerpEntityStateDestructibleHit *state)
{
    int stage; // [esp+8h] [ebp-10h]
    unsigned __int8 i; // [esp+Fh] [ebp-9h]
    Destructible *destructible; // [esp+10h] [ebp-8h]
    DestructibleDef *ddef; // [esp+14h] [ebp-4h]

    destructible = self->destructible;
    ddef = destructible->ddef;
    state->modelState0 = 0;
    state->modelState1 = 0;
    state->modelState2 = 0;
    state->modelState3 = 0;
    state->modelState4 = 0;
    for ( i = 0; i < ddef->numPieces; ++i )
    {
        if ( CG_IsDestructiblePieceDisabled(ddef->pieces[i].enableLabel) )
            stage = -1;
        else
            stage = Destructible_GetDestructibleStage(ddef, i, destructible->pieceArray[i].health);
        if ( (stage < -1 || stage > 4)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                        994,
                        0,
                        "stage not in [-1, 4]\n\t%i not in [%i, %i]",
                        stage,
                        -1,
                        4) )
        {
            __debugbreak();
        }
        switch ( stage )
        {
            case 0:
                state->modelState0 |= 1 << i;
                break;
            case 1:
                state->modelState1 |= 1 << i;
                break;
            case 2:
                state->modelState2 |= 1 << i;
                break;
            case 3:
                state->modelState3 |= 1 << i;
                break;
            case 4:
                state->modelState4 |= 1 << i;
                break;
            default:
                continue;
        }
    }
}

void __cdecl DestructibleExplosiveDamageEvent(
                gentity_s *self,
                const float *point,
                float radius,
                unsigned __int8 id)
{
    gentity_s *tent; // [esp+1Ch] [ebp-4h]

    tent = G_TempEntity(point, EV_DESTRUCTIBLE_EXPLOSION_HIT);
    if ( tent )
    {
        AssignToSmallerType<short>(&tent->s.otherEntityNum, self->s.number);

        nanassertvec3(point);

        tent->s.lerp.pos.trBase[0] = *point;
        tent->s.lerp.pos.trBase[1] = point[1];
        tent->s.lerp.pos.trBase[2] = point[2];
        tent->s.lerp.pos.trDelta[0] = 0.0f;
        tent->s.lerp.pos.trDelta[1] = 0.0f;
        tent->s.lerp.pos.trDelta[2] = 0.0f;
        tent->s.lerp.apos.trBase[0] = 0.0f;
        tent->s.lerp.apos.trBase[1] = 0.0f;
        tent->s.lerp.apos.trBase[2] = 0.0f;
        tent->s.lerp.apos.trDelta[0] = 0.0f;
        tent->s.lerp.apos.trDelta[1] = 0.0f;
        tent->s.lerp.apos.trDelta[2] = 0.0f;
        tent->s.un1.destructibleid = id;
        DestructibleEventSetPieceState(self, (LerpEntityStateDestructibleHit *)&tent->s.lerp.u);
    }
}

void __cdecl DestructibleBulletDamageEvent(
                gentity_s *self,
                const float *point,
                const float *dir,
                unsigned __int8 id)
{
    gentity_s *tent; // [esp+1Ch] [ebp-4h]

    tent = G_TempEntity(point, EV_DESTRUCTIBLE_BULLET_HIT);
    if ( tent )
    {
        AssignToSmallerType<short>(&tent->s.otherEntityNum, self->s.number);

        nanassertvec3(point);

        tent->s.lerp.pos.trBase[0] = *point;
        tent->s.lerp.pos.trBase[1] = point[1];
        tent->s.lerp.pos.trBase[2] = point[2];
        tent->s.lerp.pos.trDelta[0] = *dir;
        tent->s.lerp.pos.trDelta[1] = dir[1];
        tent->s.lerp.pos.trDelta[2] = dir[2];
        tent->s.lerp.apos.trBase[0] = 0.0f;
        tent->s.lerp.apos.trBase[1] = 0.0f;
        tent->s.lerp.apos.trBase[2] = 0.0f;
        tent->s.lerp.apos.trDelta[0] = 0.0f;
        tent->s.lerp.apos.trDelta[1] = 0.0f;
        tent->s.lerp.apos.trDelta[2] = 0.0f;
        tent->s.un1.destructibleid = id;
        DestructibleEventSetPieceState(self, (LerpEntityStateDestructibleHit *)&tent->s.lerp.u);
    }
}

bool __cdecl DamagePiece(
                gentity_s *self,
                unsigned __int8 index,
                int damage,
                const float *point,
                const float *dir,
                int mod,
                bool damageChildren,
                int excludeChild,
                gentity_s *attacker,
                int recursiveDepth)
{
    __int16 v11; // ax
    unsigned int AliasId; // eax
    bool v13; // bl
    bool v14; // bl
    bool v15; // [esp+17h] [ebp-A9h]
    int pieceDamage; // [esp+28h] [ebp-98h]
    unsigned __int8 k; // [esp+2Fh] [ebp-91h]
    float force[3]; // [esp+30h] [ebp-90h] BYREF
    float scale; // [esp+3Ch] [ebp-84h]
    unsigned int soundIndex; // [esp+40h] [ebp-80h]
    const char *loopSoundName; // [esp+44h] [ebp-7Ch]
    int break_health; // [esp+48h] [ebp-78h]
    int current_health; // [esp+4Ch] [ebp-74h]
    int damageNeeded; // [esp+50h] [ebp-70h]
    const char *notify; // [esp+54h] [ebp-6Ch]
    int i; // [esp+58h] [ebp-68h]
    DestructibleStage *stage; // [esp+5Ch] [ebp-64h]
    float tagMat[4][3]; // [esp+60h] [ebp-60h] BYREF
    int j; // [esp+90h] [ebp-30h]
    int prev_stage_index; // [esp+94h] [ebp-2Ch]
    DestructibleStage *new_stage; // [esp+98h] [ebp-28h]
    bool needFireUpdate; // [esp+9Eh] [ebp-22h]
    bool damageParent; // [esp+9Fh] [ebp-21h]
    Destructible *destructible; // [esp+A0h] [ebp-20h]
    DestructibleStage *prev_stage; // [esp+A4h] [ebp-1Ch]
    bool needDObjUpdate; // [esp+ABh] [ebp-15h]
    __int16 prev_health; // [esp+ACh] [ebp-14h]
    DestructibleDef *ddef; // [esp+B0h] [ebp-10h]
    int new_stage_index; // [esp+B4h] [ebp-Ch]
    __int16 new_health; // [esp+B8h] [ebp-8h]

    needDObjUpdate = 0;
    destructible = self->destructible;
    ddef = destructible->ddef;
    if ( recursiveDepth > 20 )
    {
        Com_Printf(20, "Warning: Destructible (%s): The recusrion in DamagePiece is too deep.", ddef->name);
        return 0;
    }
    if ( index >= ddef->numPieces
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                    1067,
                    0,
                    "%s",
                    "index<ddef->numPieces") )
    {
        __debugbreak();
    }
    v11 = ClampShort(damage);
    Destructible_SetDebugDamagePiece(index, v11);
    prev_health = destructible->pieceArray[index].health;
    new_health = prev_health - ClampShort(damage);
    if ( new_health < -1 )
        new_health = -1;
    prev_stage_index = Destructible_GetDestructibleStage(ddef, index, prev_health);
    new_stage_index = Destructible_GetDestructibleStage(ddef, index, new_health);
    prev_stage = 0;
    new_stage = 0;
    if ( prev_stage_index >= 0 )
        prev_stage = &ddef->pieces[index].stages[prev_stage_index];
    if ( new_stage_index >= 0 )
        new_stage = &ddef->pieces[index].stages[new_stage_index];
    if ( mod == 7 && prev_stage && (prev_stage->flags & 8) == 0 )
        return needDObjUpdate;
    if ( mod != 4 && mod != 6 && mod != 15 )
    {
        if ( prev_stage && (prev_stage->flags & 1) != 0 )
            return needDObjUpdate;
        for ( j = prev_stage_index + 1; j <= new_stage_index; ++j )
        {
            if ( (ddef->pieces[index].stages[j].flags & 1) != 0 )
            {
                new_health = (int)(float)((float)ddef->pieces[index].health * ddef->pieces[index].stages[j].breakHealth) + 1;
                new_stage_index = Destructible_GetDestructibleStage(ddef, index, new_health);
                new_stage = &ddef->pieces[index].stages[new_stage_index];
                break;
            }
        }
    }
    if ( prev_stage_index != new_stage_index
        && prev_stage
        && prev_stage->physPreset
        && (prev_stage->flags & 4) != 0
        && Destructible_HasParent(ddef, index)
        && destructible->pieceArray[ddef->pieces[index].parentPiece].health > 0 )
    {
        return 0;
    }
    v15 = prev_stage && (prev_stage->flags & 2) != 0;
    damageParent = v15;
    destructible->pieceArray[index].health = new_health;
    if ( prev_stage_index == new_stage_index )
    {
        if ( prev_stage_index >= 0 && !ddef->clientOnly && ddef->pieces[index].damageSound )
        {
            if ( !cg_vehicle_piece_damagesfx_threshold
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                            1151,
                            0,
                            "%s",
                            "cg_vehicle_piece_damagesfx_threshold") )
            {
                __debugbreak();
            }
            if ( (float)damage >= cg_vehicle_piece_damagesfx_threshold->current.value )
            {
                G_GetTagTransform(self, ddef, index, tagMat);
                AliasId = SND_FindAliasId((char *)ddef->pieces[index].damageSound);
                G_PlaySoundAliasAtPoint(tagMat[3], AliasId);
            }
        }
    }
    else
    {
        needDObjUpdate = 1;
        stage = 0;
        if ( prev_stage_index >= 0 )
        {
            for ( i = prev_stage_index; i < 5; ++i )
            {
                if ( ddef->pieces[index].stages[i].showBone && (i < new_stage_index || new_stage_index < 0) )
                {
                    stage = &ddef->pieces[index].stages[i];
                    notify = stage->breakNotify;
                    if ( notify )
                    {
                        Scr_DestructibleCallback(self, scr_const.broken, (char *)notify, attacker);
                        if ( self->scr_vehicle )
                        {
                            if (self->scr_vehicle->nitrousVehicle)
                            {
                                //NitrousVehicle::destructible_damage(self->scr_vehicle->nitrousVehicle, notify, new_stage_index);
                                self->scr_vehicle->nitrousVehicle->destructible_damage(notify, new_stage_index);
                            }
                        }
                    }
                }
            }
        }
        if ( prev_stage && new_stage_index >= 0 && new_stage->maxTime > 0.0 )
        {
            break_health = (__int16)(int)(float)((float)ddef->pieces[index].health * new_stage->breakHealth);
            current_health = destructible->pieceArray[index].health;
            damageNeeded = current_health - break_health + 1;
            Scr_DestructibleCallback(self, scr_const.breakafter, index, new_stage->maxTime, damageNeeded);
        }
        if ( !index )
        {
            self->s.loopSoundId = 0;
            self->s.loopSoundFade = 0;
            if ( new_stage_index >= 0 )
            {
                loopSoundName = new_stage->loopSound;
                if ( loopSoundName )
                {
                    soundIndex = SND_FindAliasId((char *)loopSoundName);
                    self->s.loopSoundId = soundIndex;
                    self->s.loopSoundFade = 1000;
                }
            }
            if ( destructible->pieceArray->health <= 0 )
                Scr_Notify(self, scr_const.destructible_base_piece_death, 0);
        }
    }
    if ( !index && self->s.eType == 6 && new_stage && ddef->model->physPreset )
    {
        force[0] = *dir;
        force[1] = dir[1];
        force[2] = dir[2];
        switch ( mod )
        {
            case 3:
            case 4:
            case 5:
            case 6:
            case 15:
                scale = dynEnt_explodeForce->current.value;
                break;
            case 17:
                scale = 0.1f;
                break;
            default:
                scale = dynEnt_bulletForce->current.value;
                break;
        }
        force[0] = scale * force[0];
        force[1] = scale * force[1];
        force[2] = scale * force[2];
        ScriptMover_SetupPhysicsLaunch(&self->s.lerp.pos, &self->s.lerp.apos, point, force);
    }
    needFireUpdate = 0;
    if ( mod == 17 )
    {
        if ( level.time - destructible->oldestBurnTime > 30000 )
        {
            needFireUpdate = 1;
            destructible->oldestBurnTime = level.time;
        }
        if ( !destructible->pieceArray[index].burnData.burnTime )
            needFireUpdate = 1;
        destructible->pieceArray[index].burnData.burnTime = level.time;
    }
    if ( damageChildren )
    {
        for ( k = 0; k < ddef->numPieces; ++k )
        {
            if ( ddef->pieces[k].parentPiece == index && index != excludeChild )
            {
                pieceDamage = (int)(float)((float)damage * ddef->pieces[k].parentDamagePercent);
                if ( pieceDamage )
                {
                    v13 = needDObjUpdate;
                    needDObjUpdate = DamagePiece(
                                                         self,
                                                         k,
                                                         pieceDamage,
                                                         point,
                                                         dir,
                                                         mod,
                                                         damageChildren,
                                                         excludeChild,
                                                         attacker,
                                                         recursiveDepth + 1)
                                                || v13;
                }
            }
        }
    }
    if ( damageParent && Destructible_HasParent(ddef, index) )
    {
        v14 = needDObjUpdate;
        return DamagePiece(
                         self,
                         ddef->pieces[index].parentPiece,
                         damage,
                         point,
                         dir,
                         mod,
                         1,
                         index,
                         attacker,
                         recursiveDepth + 1)
                || v14;
    }
    return needDObjUpdate;
}

bool __cdecl Destructible_HasParent(const DestructibleDef *ddef, unsigned __int8 index)
{
    return ddef->pieces[index].parentPiece < ddef->numPieces;
}

double __cdecl DestructibleRadiusDamage(
                gentity_s *self,
                const float *point,
                float damgeInner,
                float damgeOuter,
                float radius,
                int mod,
                gentity_s *attacker)
{
    bool v8; // bl
    bool v9; // bl
    bool v10; // bl
    float v12; // [esp+10h] [ebp-16Ch]
    float v13; // [esp+14h] [ebp-168h]
    bool v14; // [esp+18h] [ebp-164h]
    float v15; // [esp+1Ch] [ebp-160h]
    float v16; // [esp+20h] [ebp-15Ch]
    float v17; // [esp+24h] [ebp-158h]
    float v18; // [esp+28h] [ebp-154h]
    float health; // [esp+2Ch] [ebp-150h]
    float *v20; // [esp+78h] [ebp-104h]
    float absorbedDamage; // [esp+84h] [ebp-F8h]
    unsigned __int8 boneIndex; // [esp+8Fh] [ebp-EDh] BYREF
    XModel *model; // [esp+90h] [ebp-ECh]
    float offset[3]; // [esp+94h] [ebp-E8h] BYREF
    DObjAnimMat *mat; // [esp+A0h] [ebp-DCh]
    float boneLoc[3]; // [esp+A4h] [ebp-D8h] BYREF
    DestructiblePiece *piece; // [esp+B0h] [ebp-CCh]
    float damage; // [esp+B4h] [ebp-C8h]
    unsigned int boneName; // [esp+B8h] [ebp-C4h]
    float dir[3]; // [esp+BCh] [ebp-C0h] BYREF
    float hitp[3]; // [esp+C8h] [ebp-B4h] BYREF
    float dist; // [esp+D4h] [ebp-A8h]
    float distSquared; // [esp+D8h] [ebp-A4h]
    unsigned __int8 index; // [esp+DFh] [ebp-9Dh]
    float leftTreadPoint[3]; // [esp+E0h] [ebp-9Ch] BYREF
    bool needsDObjUpdate; // [esp+EFh] [ebp-8Dh]
    float smallestArmorDistance; // [esp+F0h] [ebp-8Ch]
    float rightTreadDamage; // [esp+F4h] [ebp-88h]
    float ent_axis[4][3]; // [esp+F8h] [ebp-84h] BYREF
    float nonBaseDamage; // [esp+128h] [ebp-54h]
    float leftTreadDir[3]; // [esp+12Ch] [ebp-50h] BYREF
    float maxTreadShielding; // [esp+138h] [ebp-44h]
    float radiusSquared; // [esp+13Ch] [ebp-40h]
    float entityDamage; // [esp+140h] [ebp-3Ch]
    Destructible *destructible; // [esp+144h] [ebp-38h]
    int rightTreadIndex; // [esp+148h] [ebp-34h]
    float rightTreadPoint[3]; // [esp+14Ch] [ebp-30h] BYREF
    bool hitArmor; // [esp+15Bh] [ebp-21h]
    float coreDistance; // [esp+15Ch] [ebp-20h]
    DestructibleDef *ddef; // [esp+160h] [ebp-1Ch]
    float nonBaseMax; // [esp+164h] [ebp-18h]
    float rightTreadDir[3]; // [esp+168h] [ebp-14h] BYREF
    float leftTreadDamage; // [esp+174h] [ebp-8h]
    int leftTreadIndex; // [esp+178h] [ebp-4h]

    destructible = self->destructible;
    ddef = destructible->ddef;
    if ( ddef->clientOnly )
        return 0.0;
    needsDObjUpdate = 0;
    radiusSquared = radius * radius;
    Destructible_SetDebugRender(destructible);
    entityDamage = 0.0f;
    nonBaseDamage = 0.0f;
    coreDistance = 0.0f;
    smallestArmorDistance = FLT_MAX;
    AnglesToAxis(self->r.currentAngles, ent_axis);
    ent_axis[3][0] = self->r.currentOrigin[0];
    ent_axis[3][1] = self->r.currentOrigin[1];
    ent_axis[3][2] = self->r.currentOrigin[2];
    leftTreadIndex = -1;
    leftTreadDamage = 0.0f;
    rightTreadIndex = -1;
    rightTreadDamage = 0.0f;
    memset(leftTreadPoint, 0, sizeof(leftTreadPoint));
    memset(leftTreadDir, 0, sizeof(leftTreadDir));
    memset(rightTreadPoint, 0, sizeof(rightTreadPoint));
    memset(rightTreadDir, 0, sizeof(rightTreadDir));
    hitArmor = 0;
    for ( index = 0; index < ddef->numPieces; ++index )
    {
        piece = &ddef->pieces[index];
        boneName = Destructible_GetShowBoneNameForPiece(destructible, index);
        if ( boneName )
        {
            boneIndex = 0;
            model = ddef->model;
            mat = G_DObjGetLocalTagMatrix(self, boneName);
            if ( XModelGetBoneIndex(model, boneName, 0, &boneIndex) )
            {
                if ( !mat )
                {
                    Com_PrintError(0, "ERROR: expected bone not found for destructible while calculating radius damage\n");
                    continue;
                }
                v20 = model->localBoneInfo[boneIndex].offset;
                offset[0] = *v20;
                offset[1] = v20[1];
                offset[2] = v20[2];
                MatrixTransformVectorQuatTrans(offset, mat, boneLoc);
                MatrixTransformVector43(boneLoc, ent_axis, hitp);
            }
            else
            {
                hitp[0] = self->r.currentOrigin[0];
                hitp[1] = self->r.currentOrigin[1];
                hitp[2] = self->r.currentOrigin[2];
            }
        }
        else if ( index || !self->scr_vehicle )
        {
            hitp[0] = self->r.currentOrigin[0];
            hitp[1] = self->r.currentOrigin[1];
            hitp[2] = self->r.currentOrigin[2];
        }
        else if ( !VEH_ClampPointToBoundingBox(self, point, hitp) )
        {
            hitp[0] = *point;
            hitp[1] = point[1];
            hitp[2] = point[2];
        }
        dir[0] = hitp[0] - *point;
        dir[1] = hitp[1] - point[1];
        dir[2] = hitp[2] - point[2];
        distSquared = (float)((float)(dir[0] * dir[0]) + (float)(dir[1] * dir[1])) + (float)(dir[2] * dir[2]);
        if ( distSquared < radiusSquared )
        {
            dist = sqrtf(distSquared);
            if ( index )
            {
                if ( destructible->pieceArray[index].health > 0
                    && piece->enableLabel != scr_const.left_tread
                    && piece->enableLabel != scr_const.right_tread
                    && smallestArmorDistance > dist )
                {
                    smallestArmorDistance = dist;
                }
            }
            else
            {
                coreDistance = dist;
            }
            damage = (float)((float)(damgeInner - damgeOuter) * (float)(1.0 - (float)(dist / radius))) + damgeOuter;
            damage = damage * piece->explosiveDamageScale;
            if ( dist <= 0.001 )
            {
                dir[0] = 0.0f;
                dir[1] = 0.0f;
                dir[2] = 1.0f;
            }
            else
            {
                dir[0] = (float)(1.0 / dist) * dir[0];
                dir[1] = (float)(1.0 / dist) * dir[1];
                dir[2] = (float)(1.0 / dist) * dir[2];
            }
            if ( piece->entityDamageTransfer > 0.0 && (float)(piece->entityDamageTransfer * damage) > entityDamage )
                entityDamage = piece->entityDamageTransfer * damage;
            if ( piece->enableLabel == scr_const.left_tread )
            {
                leftTreadIndex = index;
                leftTreadDamage = damage;
                leftTreadPoint[0] = hitp[0];
                leftTreadPoint[1] = hitp[1];
                leftTreadPoint[2] = hitp[2];
                leftTreadDir[0] = dir[0];
                leftTreadDir[1] = dir[1];
                leftTreadDir[2] = dir[2];
            }
            else if ( piece->enableLabel == scr_const.right_tread )
            {
                rightTreadIndex = index;
                rightTreadDamage = damage;
                rightTreadPoint[0] = hitp[0];
                rightTreadPoint[1] = hitp[1];
                rightTreadPoint[2] = hitp[2];
                rightTreadDir[0] = dir[0];
                rightTreadDir[1] = dir[1];
                rightTreadDir[2] = dir[2];
            }
            else
            {
                if ( index && destructible->pieceArray[index].health > 0 )
                {
                    absorbedDamage = damage - (float)(piece->entityDamageTransfer * damage);
                    if ( absorbedDamage < 0.0 )
                        absorbedDamage = 0.0f;
                    if ( absorbedDamage > nonBaseDamage )
                        nonBaseDamage = absorbedDamage;
                }
                v8 = needsDObjUpdate;
                needsDObjUpdate = DamagePiece(self, index, (int)damage, hitp, dir, mod, 1, -1, attacker, 0) || v8;
            }
        }
    }
    if ( (float)((float)ddef->pieces->health - (float)(vehicle_damage_max_shielding->current.value * entityDamage)) < 0.0 )
        health = (float)ddef->pieces->health;
    else
        health = vehicle_damage_max_shielding->current.value * entityDamage;
    nonBaseMax = health;
    if ( (float)(health - nonBaseDamage) < 0.0 )
        v18 = nonBaseMax;
    else
        v18 = nonBaseDamage;
    nonBaseDamage = v18;
    if ( leftTreadIndex >= 0 )
    {
        if ( (float)((float)(leftTreadDamage - nonBaseDamage) - 0.0) < 0.0 )
            v17 = 0.0f;
        else
            v17 = leftTreadDamage - nonBaseDamage;
        leftTreadDamage = v17;
        v9 = needsDObjUpdate;
        needsDObjUpdate = DamagePiece(self, leftTreadIndex, (int)v17, leftTreadPoint, leftTreadDir, mod, 1, -1, attacker, 0)
                                     || v9;
    }
    if ( rightTreadIndex >= 0 )
    {
        if ( (float)((float)(rightTreadDamage - nonBaseDamage) - 0.0) < 0.0 )
            v16 = 0.0f;
        else
            v16 = rightTreadDamage - nonBaseDamage;
        rightTreadDamage = v16;
        v10 = needsDObjUpdate;
        needsDObjUpdate = DamagePiece(
                                                self,
                                                rightTreadIndex,
                                                (int)v16,
                                                rightTreadPoint,
                                                rightTreadDir,
                                                mod,
                                                1,
                                                -1,
                                                attacker,
                                                0)
                                     || v10;
    }
    if ( (float)(leftTreadDamage - rightTreadDamage) < 0.0 )
        v15 = rightTreadDamage;
    else
        v15 = leftTreadDamage;
    maxTreadShielding = v15;
    if ( needsDObjUpdate )
    {
        DestructibleExplosiveDamageEvent(self, point, radius, mod);
        G_DObjUpdate(self);
    }
    v14 = smallestArmorDistance < 1.0 || coreDistance > 1.0;
    hitArmor = v14;
    if ( self->scr_vehicle )
    {
        if ( (float)((float)(maxTreadShielding
                                             - (float)(ddef->pieces[leftTreadIndex].entityDamageTransfer * maxTreadShielding))
                             - 0.0) < 0.0 )
            v13 = 0.0f;
        else
            v13 = maxTreadShielding - (float)(ddef->pieces[leftTreadIndex].entityDamageTransfer * maxTreadShielding);
        maxTreadShielding = v13;
        if ( !hitArmor )
            nonBaseDamage = 0.0f;
        if ( (float)(nonBaseDamage - maxTreadShielding) < 0.0 )
            v12 = maxTreadShielding;
        else
            v12 = nonBaseDamage;
        nonBaseDamage = v12;
        if ( (float)((float)(entityDamage - v12) - 0.0) < 0.0 )
            return 0.0f;
        else
            return (float)(entityDamage - v12);
    }
    return entityDamage;
}

int __cdecl DestructibleDamage(
                gentity_s *self,
                gentity_s *attacker,
                const float *dir,
                const float *point,
                int damage,
                int mod,
                unsigned int modelIndex,
                unsigned int partName)
{
    float v9; // xmm0_4
    unsigned __int8 v10; // [esp+37h] [ebp-21h] BYREF
    int randDamage; // [esp+38h] [ebp-20h]
    unsigned __int8 index; // [esp+3Eh] [ebp-1Ah]
    bool needsDObjUpdate; // [esp+3Fh] [ebp-19h]
    float hitd[3]; // [esp+40h] [ebp-18h] BYREF
    DestructibleDef *ddef; // [esp+4Ch] [ebp-Ch]
    int entityDamage; // [esp+50h] [ebp-8h]
    Destructible *destructible; // [esp+54h] [ebp-4h]
    int damagea; // [esp+70h] [ebp+18h]

    destructible = self->destructible;
    entityDamage = 0;
    if ( destructible )
    {
        ddef = destructible->ddef;
        if ( ddef->clientOnly )
            return 0;
        Destructible_SetDebugRender(destructible);

        nanassertvec3(point);
        nanassertvec3(dir);

        hitd[0] = *dir;
        hitd[1] = dir[1];
        hitd[2] = dir[2];
        if ( (float)((float)((float)(hitd[0] * hitd[0]) + (float)(hitd[1] * hitd[1])) + (float)(hitd[2] * hitd[2])) < 0.001 )
        {
            hitd[0] = 0.0f;
            hitd[1] = 0.0f;
            hitd[2] = 1.0f;
        }
        if ( mod == 5 || mod == 3 || mod == 16 )
            Vec3NormalizeFast(hitd);
        if ( mod == 17 )
        {
            needsDObjUpdate = 0;
            for ( index = 0; index < ddef->numPieces; ++index )
            {
                randDamage = G_rand() % damage + 1;
                if ( DamagePiece(self, index, randDamage, point, hitd, 17, 0, -1, attacker, 0) )
                    needsDObjUpdate = 1;
            }
            if ( needsDObjUpdate )
            {
                DestructibleBulletDamageEvent(self, point, hitd, 17);
                G_DObjUpdate(self);
            }
            return damage;
        }
        else if ( Destructible_GetPieceIndexFromBoneName(self->destructible, modelIndex, partName, &v10) )
        {
            if ( mod == 5 || mod == 3 || mod == 15 )
            {
                damagea = (int)(float)((float)damage * ddef->pieces[v10].explosiveDamageScale);
            }
            else if ( mod == 7 )
            {
                damagea = (int)(float)((float)damage * ddef->pieces[v10].meleeDamageScale);
            }
            else
            {
                if ( mod == 16 )
                    v9 = (float)damage * ddef->pieces[v10].impactDamageScale;
                else
                    v9 = (float)damage * ddef->pieces[v10].bulletDamageScale;
                damagea = (int)v9;
            }
            if ( mod != 7 && ddef->pieces[v10].entityDamageTransfer > 0.0 )
                entityDamage = (int)(float)((float)damagea * ddef->pieces[v10].entityDamageTransfer);
            if ( DamagePiece(self, v10, damagea, point, hitd, mod, 1, -1, attacker, 0) )
            {
                DestructibleBulletDamageEvent(self, point, hitd, mod);
                G_DObjUpdate(self);
            }
        }
    }
    return entityDamage;
}

void __cdecl CG_DestructibleClientOnlyCreate(centity_s *ent, DestructibleDef *ddef)
{
    if ( !ddef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp", 1647, 0, "%s", "ddef") )
        __debugbreak();
    ent->destructible = CG_GetFreeDestructible(ent->pose.localClientNum, ent->nextState.number);
    ent->destructible->ddef = ddef;
    if ( ent->nextState.number == 1023
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                    1650,
                    0,
                    "%s",
                    "ent->nextState.number != ENTITYNUM_NONE") )
    {
        __debugbreak();
    }
    ent->destructible->entNum = ent->nextState.number;
    ent->nextState.eType = ET_SCRIPTMOVER;
    ent->nextState.lerp.pos.trBase[0] = ent->pose.origin[0];
    ent->nextState.lerp.pos.trBase[1] = ent->pose.origin[1];
    ent->nextState.lerp.pos.trBase[2] = ent->pose.origin[2];
    ent->currentState.pos.trBase[0] = ent->pose.origin[0];
    ent->currentState.pos.trBase[1] = ent->pose.origin[1];
    ent->currentState.pos.trBase[2] = ent->pose.origin[2];
    ent->nextState.lerp.apos.trBase[0] = ent->pose.angles[0];
    ent->nextState.lerp.apos.trBase[1] = ent->pose.angles[1];
    ent->nextState.lerp.apos.trBase[2] = ent->pose.angles[2];
    ent->currentState.apos.trBase[0] = ent->pose.angles[0];
    ent->currentState.apos.trBase[1] = ent->pose.angles[1];
    ent->currentState.apos.trBase[2] = ent->pose.angles[2];
    ent->nextState.lerp.eFlags |= 0x20000u;
    CG_PreProcess_GetDObj(ent->pose.localClientNum, ent->nextState.number, ent->nextState.eType, ddef->model, 0);
    *(_QWORD *)ent->nextState.lerp.u.turret.gunAngles = 0;
    *(_QWORD *)ent->nextState.lerp.u.scriptMover.attachModelIndex = 0;
    *((_QWORD *)&ent->nextState.lerp.u.createDynEnt + 2) = 0;
    ent->nextState.lerp.u.player.meleeWeapon = 1023;
}

unsigned int __cdecl CG_DestructibleUpdate(
                centity_s *ent,
                DObjModel_s *dobjModels,
                unsigned int numModels,
                int localClientNum)
{
    XModel *v5; // eax
    char *loopSoundName; // [esp+14h] [ebp-14h]
    unsigned __int8 j; // [esp+1Ah] [ebp-Eh]
    unsigned __int8 i; // [esp+1Bh] [ebp-Dh]
    char *destructibleName; // [esp+1Ch] [ebp-Ch]
    DestructibleDef *ddef; // [esp+20h] [ebp-8h]
    destructible_gamestate *dgs; // [esp+24h] [ebp-4h]

    if ( (ent->nextState.lerp.eFlags & 0x20000) == 0 || ent->nextState.eType == 1 || ent->nextState.eType == 2 )
        return numModels;
    if ( ent->nextState.eType != 14 && ent->nextState.eType != 16 && ent->nextState.eType != 6 )
        return numModels;
    if ( ent->destructible )
    {
        ddef = ent->destructible->ddef;
        if ( !ddef
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp", 1686, 0, "%s", "ddef") )
        {
            __debugbreak();
        }
    }
    else
    {
        destructibleName = CL_GetConfigString(ent->nextState.un1.scale + 3180);
        if ( !destructibleName
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                        1691,
                        0,
                        "%s",
                        "destructibleName") )
        {
            __debugbreak();
        }
        ddef = DestructibleDefPrecache(destructibleName);
        if ( !ddef
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp", 1694, 0, "%s", "ddef") )
        {
            __debugbreak();
        }
    }
    if ( !ddef )
        return numModels;
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp", 1700, 0, "%s", "ent") )
        __debugbreak();
    if ( !dobjModels
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp", 1701, 0, "%s", "dobjModels") )
    {
        __debugbreak();
    }
    if ( ent->destructible )
    {
        if ( ent->destructible->entNum != ent->nextState.number
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                        1710,
                        0,
                        "%s",
                        "ent->destructible->entNum == ent->nextState.number") )
        {
            __debugbreak();
        }
    }
    else
    {
        ent->destructible = CG_GetFreeDestructible(localClientNum, ent->nextState.number);
        ent->destructible->ddef = ddef;
    }
    if ( !ent->destructible->ddef
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                    1713,
                    0,
                    "%s",
                    "ent->destructible->ddef") )
    {
        __debugbreak();
    }
    if ( ent->destructible )
    {
        if ( ent->nextState.number == 1023
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                        1717,
                        0,
                        "%s",
                        "ent->nextState.number != ENTITYNUM_NONE") )
        {
            __debugbreak();
        }
        ent->destructible->entNum = ent->nextState.number;
    }
    else
    {
        Com_Error(ERR_DROP, "Hit max destructible count [%d] when creating %s", 64, ddef->name);
    }
    if ( numModels != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                    1725,
                    0,
                    "%s",
                    "numModels == 1") )
    {
        __debugbreak();
    }
    dgs = Destructible_FindGameState(ent->nextState.number, localClientNum);
    if ( (ent->destructible->flags & 2) == 0 )
    {
        if ( !g_cgPieceArrayHunk )
            g_cgPieceArrayHunk = Hunk_UserCreateFromBuffer(
                                                         g_cgPieceArrayBuffer,
                                                         73728,
                                                         HU_SCHEME_FIRSTFIT,
                                                         0,
                                                         0,
                                                         "DesctructiblePieces",
                                                         0);
        ent->destructible->pieceArray = (DESTRUCTIBLE_PIECE_INFO *)Hunk_UserAlloc(
                                                                                                                                 g_cgPieceArrayHunk,
                                                                                                                                 24 * ddef->numPieces,
                                                                                                                                 4,
                                                                                                                                 0);
        memset(ent->destructible->pieceArray, 0, 24 * ddef->numPieces);
        if ( !ent->destructible->pieceArray
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                        1741,
                        0,
                        "%s",
                        "ent->destructible->pieceArray") )
        {
            __debugbreak();
        }
        for ( i = 0; i < ddef->numPieces; ++i )
        {
            ent->destructible->pieceArray[i].health = ddef->pieces[i].health;
            if ( dgs )
                ent->destructible->pieceArray[i].health = dgs->health[i];
            ent->destructible->pieceArray[i].burnData.fx = 0;
            ent->destructible->pieceArray[i].burnData.burnTime = 0;
            ent->destructible->pieceArray[i].burnData.sndId = 0;
        }
        if ( ddef->pristineModel )
            ent->destructible->flags |= 4u;
    }
    for ( j = 0; j < ddef->numPieces; ++j )
    {
        if ( !ddef->pieces[j].stages[0].showBone
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                        1762,
                        0,
                        "%s",
                        "ddef->pieces[i].stages[0].showBone") )
        {
            __debugbreak();
        }
        if ( !CG_IsDestructiblePieceDisabled(ddef->pieces[j].enableLabel) && !j )
        {
            v5 = (ent->destructible->flags & 4) != 0 ? ddef->pristineModel : ddef->model;
            dobjModels->model = v5;
            ent->nextState.loopSoundId = 0;
            ent->nextState.loopSoundFade = 1000;
            loopSoundName = (char *)ddef->pieces->stages[Destructible_GetDestructibleStage(
                                                                                                         ddef,
                                                                                                         0,
                                                                                                         ent->destructible->pieceArray->health)].loopSound;
            if ( loopSoundName )
            {
                ent->nextState.loopSoundId = SND_FindAliasId(loopSoundName);
                ent->nextState.loopSoundFade = 1000;
            }
        }
    }
    if ( dgs )
        Destructible_RemoveGameState(ent->nextState.number, localClientNum);
    ent->destructible->flags |= 2u;
    if ( (ent->destructible->flags & 4) != 0 )
    {
        ent->nextState.partBits[0] = 0;
        ent->nextState.partBits[1] = 0;
        ent->nextState.partBits[2] = 0;
        ent->nextState.partBits[3] = 0;
        ent->nextState.partBits[4] = 0;
    }
    else
    {
        Destructible_GetHideParts(ent->destructible, ent->nextState.partBits);
    }
    return numModels;
}

destructible_gamestate *__cdecl Destructible_FindGameState(int entityNum, int localClientNum)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < s_num_destructible_gamestates[localClientNum]; ++i )
    {
        if ( s_destructible_gamestates[localClientNum][i].entityNum == entityNum )
            return &s_destructible_gamestates[localClientNum][i];
    }
    return 0;
}

char __cdecl Destructible_RemoveGameState(int entityNum, int localClientNum)
{
    int i; // [esp+8h] [ebp-4h]

    for ( i = 0; i < s_num_destructible_gamestates[localClientNum]; ++i )
    {
        if ( s_destructible_gamestates[localClientNum][i].entityNum == entityNum )
        {
            memcpy(
                &s_destructible_gamestates[localClientNum][i],
                &s_destructible_gamestates[localClientNum][--s_num_destructible_gamestates[localClientNum]],
                sizeof(s_destructible_gamestates[localClientNum][i]));
            return 1;
        }
    }
    return 0;
}

int __cdecl CG_SpawnPhysObjForBone(
                const centity_s *cent,
                int pieceIndex,
                unsigned int boneName,
                const float *hitp,
                const float *hitd,
                int mod)
{
    int xdollHandle; // [esp+14h] [ebp-1Ch]
    PhysPreset *physPreset; // [esp+18h] [ebp-18h]
    int localClientNum; // [esp+1Ch] [ebp-14h]
    DestructibleDef *ddef; // [esp+20h] [ebp-10h]
    float dist2; // [esp+24h] [ebp-Ch]
    const cg_s *cg; // [esp+28h] [ebp-8h]

    cg = CG_GetLocalClientGlobals(0);
    dist2 = Vec3DistanceSq(cg->predictedPlayerState.origin, hitp);
    if ( (float)(phys_piecesSpawnDistanceCutoff->current.value * phys_piecesSpawnDistanceCutoff->current.value) <= dist2 )
        return 0;
    ddef = cent->destructible->ddef;
    localClientNum = cent->pose.localClientNum;
    physPreset = Destructible_GetPiecePhysPreset(ddef, pieceIndex);
    if ( pieceIndex >= ddef->numPieces
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                    1827,
                    0,
                    "%s",
                    "pieceIndex<ddef->numPieces") )
    {
        __debugbreak();
    }
    if ( cent->destructible->pieceArray[pieceIndex].xdollHandle )
    {
        xdollHandle = cent->destructible->pieceArray[pieceIndex].xdollHandle;
        XDoll_SetCollides(xdollHandle, 1);
    }
    else
    {
        xdollHandle = XDoll_CreateXDollForBoneLaunch(
                                        localClientNum,
                                        cent->nextState.number,
                                        pieceIndex,
                                        boneName,
                                        physPreset);
    }
    if ( xdollHandle )
        XDoll_ApplyForce(xdollHandle, hitp, hitd, mod);
    return xdollHandle;
}

unsigned __int16 __cdecl CG_DestructibleSpawnDynEnt(
                const centity_s *cent,
                XModel *model,
                float *origin,
                float (*mat)[3],
                const float *hitp,
                const float *hitd,
                int mod)
{
    col_context_t context; // [esp+1Ch] [ebp-A8h] BYREF
    float dir[3]; // [esp+80h] [ebp-44h] BYREF
    float start[3]; // [esp+8Ch] [ebp-38h] BYREF
    float end[3]; // [esp+98h] [ebp-2Ch] BYREF
    float quat[4]; // [esp+A4h] [ebp-20h] BYREF
    float localHitD[3]; // [esp+B4h] [ebp-10h] BYREF
    unsigned __int16 dynEntId; // [esp+C0h] [ebp-4h]

    AxisToQuat(mat, quat);
    localHitD[0] = *hitd;
    localHitD[1] = hitd[1];
    localHitD[2] = hitd[2];
    if ( cent->vehicle )
    {
        if ( mod == 1 || mod == 2 )
            Vec3Normalize(localHitD);
        mod = 2;
    }
    if ( cent )
    {
        if ( cent->vehicle )
        {
            trace_t trace; // [esp+44h] [ebp-80h] BYREF
            start[0] = cent->pose.origin[0];
            start[1] = cent->pose.origin[1];
            start[2] = cent->pose.origin[2];
            start[2] = start[2] + 10.0;
            dir[0] = *origin - start[0];
            dir[1] = origin[1] - start[1];
            dir[2] = origin[2] - start[2];
            Vec3Normalize(dir);
            end[0] = (float)(10.0 * dir[0]) + *origin;
            end[1] = (float)(10.0 * dir[1]) + origin[1];
            end[2] = (float)(10.0 * dir[2]) + origin[2];
            //col_context_t::col_context_t(&context);
            CG_TraceCapsule(
                &trace,
                start,
                vec3_origin,
                vec3_origin,
                end,
                cent->nextState.number,
                0x280EC93,
                &context);
            if ( trace.startsolid || trace.fraction < 1.0 )
                return -1;
        }
    }
    dynEntId = DynEntCl_CreateEntityModel(model, quat, origin, hitp, localHitD, mod, 0);
    if ( cent->vehicle )
        DynEntCl_SetFadeOut(dynEntId, DYNENT_DRAW_MODEL);
    return dynEntId;
}

int __cdecl G_GetTagTransform(gentity_s *ent, DestructibleDef *ddef, int pieceIndex, float (*tagMat)[3])
{
    int boneName; // [esp+Ch] [ebp-4h]

    boneName = Destructible_GetPieceParentBone(ddef, pieceIndex);
    if ( boneName && Com_GetServerDObj(ent->s.number) && G_DObjGetWorldTagMatrix(ent, boneName, tagMat) )
        return boneName;
    AnglesToAxis(ent->r.currentAngles, tagMat);
    (*tagMat)[9] = ent->r.currentOrigin[0];
    (*tagMat)[10] = ent->r.currentOrigin[1];
    (*tagMat)[11] = ent->r.currentOrigin[2];
    return boneName;
}

void __cdecl CG_DestructibleBreakPiece(
                centity_s *self,
                DestructiblePiece *piece,
                int pieceIndex,
                int stage,
                int nextStage,
                const float *hitp,
                const float *hitd,
                int mod)
{
    cg_s *LocalClientGlobals; // eax
    unsigned int AliasId; // eax
    float max; // [esp+4h] [ebp-108h]
    float v11; // [esp+Ch] [ebp-100h]
    float v12; // [esp+10h] [ebp-FCh]
    float v13; // [esp+14h] [ebp-F8h]
    float v14; // [esp+18h] [ebp-F4h]
    float v15; // [esp+1Ch] [ebp-F0h]
    float spin_scale; // [esp+28h] [ebp-E4h]
    float v17; // [esp+2Ch] [ebp-E0h]
    float v18; // [esp+30h] [ebp-DCh]
    float v19; // [esp+34h] [ebp-D8h]
    float power; // [esp+38h] [ebp-D4h]
    PhysConstraint *data; // [esp+50h] [ebp-BCh]
    float forward[3]; // [esp+54h] [ebp-B8h] BYREF
    //float v23; // [esp+58h] [ebp-B4h]
    //float v24; // [esp+5Ch] [ebp-B0h]
    float v25[6]; // [esp+60h] [ebp-ACh] BYREF
    float angles[3]; // [esp+78h] [ebp-94h] BYREF
    const PhysConstraint *c; // [esp+84h] [ebp-88h]
    float popOutDir[3]; // [esp+88h] [ebp-84h] BYREF
    float newHitp[3]; // [esp+94h] [ebp-78h] BYREF
    float randDir[3]; // [esp+A0h] [ebp-6Ch]
    float forceAngles[3]; // [esp+ACh] [ebp-60h] BYREF
    XModel *model; // [esp+B8h] [ebp-54h]
    int i; // [esp+BCh] [ebp-50h]
    unsigned int boneName; // [esp+C0h] [ebp-4Ch]
    unsigned int fxHandle; // [esp+C4h] [ebp-48h]
    int entityNum; // [esp+C8h] [ebp-44h]
    int localClientNum; // [esp+CCh] [ebp-40h]
    float tagOrigin[3]; // [esp+D0h] [ebp-3Ch] BYREF
    float tagMat[3][3]; // [esp+DCh] [ebp-30h] BYREF
    unsigned __int16 dynEntId; // [esp+100h] [ebp-Ch]
    DestructibleDef *ddef; // [esp+104h] [ebp-8h]
    bool burning; // [esp+10Bh] [ebp-1h]

    localClientNum = self->pose.localClientNum;
    entityNum = self->nextState.number;
    ddef = self->destructible->ddef;
    boneName = CG_GetTagTransform(self, self->destructible->ddef, pieceIndex, tagOrigin, tagMat);

    if (boneName && self->nitrousVeh)
    {
        //NitrousVehicle::damage(self->nitrousVeh, boneName, nextStage);
        self->nitrousVeh->damage(boneName, nextStage);
    }

    if ( pieceIndex >= self->destructible->ddef->numPieces
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                    1958,
                    0,
                    "%s",
                    "pieceIndex<self->destructible->ddef->numPieces") )
    {
        __debugbreak();
    }
    burning = self->destructible->pieceArray[pieceIndex].burnData.burnTime != 0;
    if ( CL_LocalClient_IsFirstActive(localClientNum) )
    {
        for ( i = 0; i < 3; ++i )
        {
            model = piece->stages[stage].spawnModel[i];
            if ( model )
            {
                if ( model->physPreset )
                {
                    if ( piece->physConstraints && piece->physConstraints->data[0].type == CONSTRAINT_LAUNCH )
                    {
                        c = piece->physConstraints->data;
                        forceAngles[0] = self->pose.angles[0] + c->scale[0];
                        forceAngles[1] = self->pose.angles[1] + c->scale[1];
                        forceAngles[2] = self->pose.angles[2] + c->scale[2];
                        AngleVectors(forceAngles, popOutDir, 0, 0);
                        power = c->power;
                        popOutDir[0] = power * popOutDir[0];
                        popOutDir[1] = power * popOutDir[1];
                        popOutDir[2] = power * popOutDir[2];
                        v17 = G_flrand(-1.0, 1.0);
                        v18 = G_flrand(-1.0, 1.0);
                        v19 = G_flrand(-1.0, 1.0);
                        randDir[0] = v17;
                        randDir[1] = v18;
                        randDir[2] = v19;
                        spin_scale = c->spin_scale;
                        newHitp[0] = (float)(spin_scale * v17) + *hitp;
                        newHitp[1] = (float)(spin_scale * v18) + hitp[1];
                        newHitp[2] = (float)(spin_scale * v19) + hitp[2];
                        dynEntId = CG_DestructibleSpawnDynEnt(self, model, tagOrigin, tagMat, newHitp, popOutDir, mod);
                    }
                    else
                    {
                        dynEntId = CG_DestructibleSpawnDynEnt(self, model, tagOrigin, tagMat, hitp, hitd, mod);
                    }
                    if ( burning && dynEntId != 0xFFFF )
                    {
                        DynEntCl_SetBurning(dynEntId, DYNENT_DRAW_MODEL, 1);
                        max = CG_DestructibleGetBurnFrac(self->destructible);
                        DynEntCl_SetBurnFrac(dynEntId, DYNENT_DRAW_MODEL, max);
                    }
                }
                else
                {
                    Com_PrintWarning(
                        21,
                        "Spawn model %s for destructible %s doesn't have physPreset.",
                        model->name,
                        self->destructible->ddef->name);
                }
            }
        }
    }
    if ( piece->stages[stage].physPreset && nextStage < 0 && CL_LocalClient_IsFirstActive(localClientNum) )
    {
        if ( piece->physConstraints && piece->physConstraints->data[0].type == CONSTRAINT_LAUNCH )
        {
            data = piece->physConstraints->data;
            angles[0] = self->pose.angles[0] + piece->physConstraints->data[0].scale[0];
            angles[1] = self->pose.angles[1] + data->scale[1];
            angles[2] = self->pose.angles[2] + data->scale[2];
            AngleVectors(angles, forward, 0, 0);
            v15 = data->power;
            forward[0] = v15 * forward[0];
            forward[1] = v15 * forward[1];
            forward[2] = v15 * forward[2];
            v12 = G_flrand(-1.0, 1.0);
            v13 = G_flrand(-1.0, 1.0);
            v14 = G_flrand(-1.0, 1.0);
            v25[3] = v12;
            v25[4] = v13;
            v25[5] = v14;
            v11 = data->spin_scale;
            v25[0] = (float)(v11 * v12) + *hitp;
            v25[1] = (float)(v11 * v13) + hitp[1];
            v25[2] = (float)(v11 * v14) + hitp[2];
            self->destructible->pieceArray[pieceIndex].xdollHandle = CG_SpawnPhysObjForBone(
                                                                                                                                 self,
                                                                                                                                 pieceIndex,
                                                                                                                                 piece->stages[stage].showBone,
                                                                                                                                 v25,
                                                                                                                                 forward,
                                                                                                                                 mod);
        }
        else
        {
            self->destructible->pieceArray[pieceIndex].xdollHandle = CG_SpawnPhysObjForBone(
                                                                                                                                 self,
                                                                                                                                 pieceIndex,
                                                                                                                                 piece->stages[stage].showBone,
                                                                                                                                 hitp,
                                                                                                                                 hitd,
                                                                                                                                 mod);
        }
    }

    if (piece->stages[stage].breakNotify && self->nitrousVeh)
    {
        //NitrousVehicle::destructible_damage(self->nitrousVeh, piece->stages[stage].breakNotify, nextStage);
        self->nitrousVeh->destructible_damage(piece->stages[stage].breakNotify, nextStage);
    }

    if (self->destructible->pieceArray[pieceIndex].fx)
    {
        //FX_ThroughWithEffect(localClientNum, self->destructible->pieceArray[pieceIndex].fx, 0);
        FX_ThroughWithEffect(localClientNum, self->destructible->pieceArray[pieceIndex].fx, 0);
    }

    if ( piece->stages[stage].breakEffect )
    {
        if ( boneName )
        {
            fxHandle = CG_PlayBoltedEffect(localClientNum, piece->stages[stage].breakEffect, entityNum, boneName);
        }
        else if ( self->nextState.eType == 14 || self->nextState.eType == 16 )
        {
            fxHandle = CG_PlayBoltedEffect(localClientNum, piece->stages[stage].breakEffect, entityNum, 0);
        }
        else
        {
            LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
            fxHandle = FX_PlayOrientedEffect(
                                     localClientNum,
                                     piece->stages[stage].breakEffect,
                                     LocalClientGlobals->time,
                                     tagOrigin,
                                     tagMat);
        }
        self->destructible->pieceArray[pieceIndex].fx = fxHandle;
    }
    if ( piece->stages[stage].breakSound )
    {
        AliasId = SND_FindAliasId((char *)piece->stages[stage].breakSound);
        CG_PlaySound(localClientNum, entityNum, tagOrigin, 0, 0, 1.0, AliasId);
    }
    if ( ddef->clientOnly && nextStage >= 0 && piece->stages[nextStage].maxTime > 0.0 )
        CScr_DestructibleCallback(
            self,
            cscr_const.breakafter,
            pieceIndex,
            piece->stages[nextStage].maxTime,
            self->destructible->pieceArray[pieceIndex].health
        - (__int16)(int)(float)((float)ddef->pieces[pieceIndex].health * piece->stages[nextStage].breakHealth)
        + 1);
}

unsigned int __cdecl CG_GetTagTransform(
                centity_s *self,
                DestructibleDef *ddef,
                int pieceIndex,
                float *tagOrigin,
                float (*tagMat)[3])
{
    DObj *obj; // [esp+4h] [ebp-8h]
    unsigned int boneName; // [esp+8h] [ebp-4h]

    boneName = ddef->pieces[pieceIndex].stages[0].showBone;
    if ( ddef->pieces[pieceIndex].stages[0].showBone )
    {
        obj = Com_GetClientDObj(self->nextState.number, self->pose.localClientNum);
        if ( obj )
        {
            if ( CG_DObjGetWorldTagMatrix(&self->pose, obj, boneName, tagMat, tagOrigin) )
                return boneName;
        }
    }
    AnglesToAxis(self->pose.angles, tagMat);
    *tagOrigin = self->pose.origin[0];
    tagOrigin[1] = self->pose.origin[1];
    tagOrigin[2] = self->pose.origin[2];
    return boneName;
}

void __cdecl CG_DestructibleSetBurn(
                int localClientNum,
                centity_s *self,
                Destructible *destructible,
                int pieceIndex,
                bool burn)
{
    cg_s *LocalClientGlobals; // eax
    float axis[3][3]; // [esp+1Ch] [ebp-2Ch] BYREF
    const FxEffectDef *fxDef; // [esp+40h] [ebp-8h]
    DestructibleDef *ddef; // [esp+44h] [ebp-4h]

    ddef = destructible->ddef;
    if ( pieceIndex >= ddef->numPieces
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                    2089,
                    0,
                    "%s",
                    "pieceIndex<ddef->numPieces") )
    {
        __debugbreak();
    }
    if ( burn )
    {
        if ( !destructible->oldestBurnTime )
        {
            if ( !self
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp", 2096, 0, "%s", "self") )
            {
                __debugbreak();
            }
            if ( !UniqueHandleToEffect(self->pose.localClientNum, destructible->pieceArray[pieceIndex].burnData.fx) )
            {
                fxDef = ddef->pieces[pieceIndex].burnEffect;
                if ( fxDef )
                {
                    AnglesToAxis(self->pose.angles, axis);
                    LocalClientGlobals = CG_GetLocalClientGlobals(self->pose.localClientNum);
                    destructible->pieceArray[pieceIndex].burnData.fx = FX_SpawnOrientedEffect(
                                                                                                                             self->pose.localClientNum,
                                                                                                                             fxDef,
                                                                                                                             LocalClientGlobals->time,
                                                                                                                             self->pose.origin,
                                                                                                                             axis,
                                                                                                                             self->nextState.number);
                }
            }
            destructible->pieceArray[pieceIndex].burnData.burnTime = CG_GetLocalClientGlobals(self->pose.localClientNum)->time;
            if ( !destructible->oldestBurnTime )
                destructible->oldestBurnTime = CG_GetLocalClientGlobals(self->pose.localClientNum)->time;
        }
    }
    else
    {
        if ( UniqueHandleToEffect(localClientNum, destructible->pieceArray[pieceIndex].burnData.fx) )
            FX_ThroughWithEffect(localClientNum, destructible->pieceArray[pieceIndex].burnData.fx, 1);
        destructible->pieceArray[pieceIndex].burnData.fx = 0;
        destructible->pieceArray[pieceIndex].burnData.burnTime = 0;
    }
}

void __cdecl CG_DestructibleUpdateBurn(int localClientNum, Destructible *destructible)
{
    int v2; // [esp+0h] [ebp-10h]
    int i; // [esp+4h] [ebp-Ch]
    int currentTime; // [esp+8h] [ebp-8h]
    DestructibleDef *ddef; // [esp+Ch] [ebp-4h]

    if ( !destructible->ddef )
        destructible->oldestBurnTime = 0;
    if ( destructible->oldestBurnTime )
    {
        ddef = destructible->ddef;
        v2 = RETURN_ZERO32();
        currentTime = CG_GetLocalClientGlobals(v2)->time;
        for ( i = 0; i < ddef->numPieces; ++i )
        {
            if ( destructible->pieceArray[i].burnData.burnTime )
            {
                if ( currentTime - destructible->pieceArray[i].burnData.burnTime > 60000 )
                    CG_DestructibleSetBurn(localClientNum, 0, destructible, i, 0);
            }
        }
    }
}

void __cdecl CG_DestructibleUpdateXDoll(int localClientNum, Destructible *destructible)
{
    centity_s *cent; // [esp+8h] [ebp-10h]
    int i; // [esp+Ch] [ebp-Ch]
    bool hasXdoll; // [esp+12h] [ebp-6h]
    bool needsDObjUpdate; // [esp+13h] [ebp-5h]
    DestructibleDef *ddef; // [esp+14h] [ebp-4h]

    if ( destructible->ddef && (destructible->flags & 1) != 0 )
    {
        ddef = destructible->ddef;
        hasXdoll = 0;
        needsDObjUpdate = 0;
        for ( i = 0; i < ddef->numPieces; ++i )
        {
            if ( destructible->pieceArray[i].xdollHandle )
            {
                if ( XDoll_GetStableTime(destructible->pieceArray[i].xdollHandle) <= 60000 )
                {
                    hasXdoll = 1;
                }
                else
                {
                    XDoll_DestroyXDoll(destructible->pieceArray[i].xdollHandle);
                    destructible->pieceArray[i].xdollHandle = 0;
                    needsDObjUpdate = 1;
                }
            }
        }
        if ( !hasXdoll )
            destructible->flags &= ~1u;
        if ( needsDObjUpdate )
        {
            cent = CG_GetEntity(localClientNum, destructible->entNum);
            CG_DestructibleUpdateDObj(cent);
        }
    }
}

double __cdecl CG_DestructibleGetBurnFrac(Destructible *destructible)
{
    cg_s *LocalClientGlobals; // eax
    float v4; // [esp+4h] [ebp-Ch]
    float burnFraction; // [esp+8h] [ebp-8h]

    if ( !destructible->oldestBurnTime )
        return 0.0;
    LocalClientGlobals = CG_GetLocalClientGlobals(0);
    burnFraction = (float)(LocalClientGlobals->time - destructible->oldestBurnTime) / 5000.0;
    if ( (float)(burnFraction - 1.0) < 0.0 )
        v4 = (float)(LocalClientGlobals->time - destructible->oldestBurnTime) / 5000.0;
    else
        v4 = 1.0f;
    if ( (float)(0.0 - burnFraction) < 0.0 )
        return v4;
    else
        return 0.0f;
}

bool __cdecl CG_DestructibleIsClientOnly(Destructible *destructible)
{
    return destructible->ddef->clientOnly > 0;
}

void __cdecl CG_UpdateDestructibles(int localClientNum, int msec)
{
    unsigned int i; // [esp+40h] [ebp-8h]

    _mm_prefetch((const char *)cg_destructibles[localClientNum], 1);
    _mm_prefetch((const char *)&cg_destructibles[localClientNum][1], 1);
    _mm_prefetch((const char *)&cg_destructibles[localClientNum][2], 1);
    _mm_prefetch((const char *)&cg_destructibles[localClientNum][3], 1);
    _mm_prefetch((const char *)&cg_destructibles[localClientNum][4], 1);
    _mm_prefetch((const char *)&cg_destructibles[localClientNum][5], 1);
    _mm_prefetch((const char *)&cg_destructibles[localClientNum][6], 1);
    cg_updateTime[localClientNum] += msec;
    if ( cg_updateTime[localClientNum] > 300 )
    {
        cg_updateTime[localClientNum] -= 300;
        for ( i = 0; i < cg_numDestructibles[localClientNum]; ++i )
        {
            _mm_prefetch((const char *)&cg_destructibles[localClientNum][i + 7], 1);
            if ( cg_destructibles[localClientNum][i].entNum != 1023 )
            {
                CG_DestructibleUpdateBurn(localClientNum, &cg_destructibles[localClientNum][i]);
                CG_DestructibleUpdateXDoll(localClientNum, &cg_destructibles[localClientNum][i]);
            }
        }
    }
    CG_DestructibleUpdatePoses(localClientNum, msec);
}

void __cdecl CG_DestructibleUpdateDObj(centity_s *self)
{
    DObj *obj; // [esp+4h] [ebp-1Ch]
    unsigned int oldPartBits[5]; // [esp+Ch] [ebp-14h] BYREF

    if ( self->destructible )
    {
        obj = Com_GetClientDObj(self->nextState.number, self->pose.localClientNum);
        if ( obj )
        {
            Destructible_GetHideParts(self->destructible, self->nextState.partBits);
            if ( self->nextState.partBits[4] ^ obj->hidePartBits[4]
                 | self->nextState.partBits[3] ^ obj->hidePartBits[3]
                 | self->nextState.partBits[2] ^ obj->hidePartBits[2]
                 | self->nextState.partBits[1] ^ obj->hidePartBits[1]
                 | self->nextState.partBits[0] ^ obj->hidePartBits[0] )
            {
                DObjGetHidePartBits(obj, oldPartBits);
                DObjSetHidePartBits(obj, self->nextState.partBits);
                FX_MarkEntUpdateHidePartBits(
                    oldPartBits,
                    self->nextState.partBits,
                    self->pose.localClientNum,
                    self->nextState.number);
            }
        }
    }
}

void __cdecl CG_DestructibleRadiusDamage(
                centity_s *self,
                const centity_s **point,
                float damgeInner,
                float damgeOuter,
                float radius,
                int mod,
                int lazyprocess)
{
    bool v7; // bl
    float v8; // [esp+10h] [ebp-B4h]
    float v9; // [esp+14h] [ebp-B0h]
    float v10; // [esp+18h] [ebp-ACh]
    const centity_s **p_attacker; // [esp+28h] [ebp-9Ch]
    unsigned __int8 boneIndex; // [esp+33h] [ebp-91h] BYREF
    float ent_axis[4][3]; // [esp+34h] [ebp-90h] BYREF
    XModel *model; // [esp+64h] [ebp-60h]
    DObj *obj; // [esp+68h] [ebp-5Ch]
    DestructiblePiece *piece; // [esp+6Ch] [ebp-58h]
    float damage; // [esp+70h] [ebp-54h]
    unsigned int boneName; // [esp+74h] [ebp-50h]
    float dir[3]; // [esp+78h] [ebp-4Ch] BYREF
    float hitp[3]; // [esp+84h] [ebp-40h] BYREF
    float dist; // [esp+90h] [ebp-34h]
    float distSquared; // [esp+94h] [ebp-30h]
    float randDir[3]; // [esp+98h] [ebp-2Ch]
    float spin_scale; // [esp+A4h] [ebp-20h]
    unsigned __int8 index; // [esp+ABh] [ebp-19h]
    destructible_event_t *de; // [esp+ACh] [ebp-18h]
    bool needsDObjUpdate; // [esp+B3h] [ebp-11h]
    float radiusSquared; // [esp+B4h] [ebp-10h]
    float entityDamage; // [esp+B8h] [ebp-Ch]
    Destructible *destructible; // [esp+BCh] [ebp-8h]
    DestructibleDef *ddef; // [esp+C0h] [ebp-4h]

    if ( lazyprocess )
    {
        if ( g_destructible_events_count >= 64 )
        {
            Com_PrintWarning(20, "CG_DestructibleRadiusDamage: Destructible events buffer full.\n");
            Destructible_DumpEventInfo();
        }
        else
        {
            de = &g_destructible_events[g_destructible_events_count++];
            de->type = 1;
            de->ehe.localClientNum = (int)self;
            p_attacker = &de->ed.attacker;
            de->erd.point[0] = *(float *)point;
            p_attacker[1] = point[1];
            p_attacker[2] = point[2];
            de->ed.dir[2] = damgeInner;
            de->ed.point[0] = damgeOuter;
            de->ed.point[1] = radius;
            de->erd.mod = mod;
        }
    }
    else
    {
        destructible = self->destructible;
        if ( destructible )
        {
            ddef = destructible->ddef;
            if ( !ddef->clientOnly )
            {
                damgeInner = 0.0f;
                damgeOuter = 0.0f;
            }
            if ( (destructible->flags & 4) != 0 )
            {
                destructible->flags &= ~4u;
                CG_SafeDObjFree(self->pose.localClientNum, self->nextState.number);
                CG_PreProcess_GetDObj(self->pose.localClientNum, self->nextState.number, self->nextState.eType, ddef->model, 0);
            }
            needsDObjUpdate = 0;
            radiusSquared = radius * radius;
            Destructible_SetDebugRender(destructible);
            entityDamage = 0.0f;
            for ( index = 0; index < ddef->numPieces; ++index )
            {
                piece = &ddef->pieces[index];
                boneName = Destructible_GetShowBoneNameForPiece(self->destructible, index);
                if ( boneName )
                {
                    obj = Com_GetClientDObj(self->nextState.number, self->pose.localClientNum);
                    if ( obj )
                    {
                        model = ddef->model;
                        boneIndex = 0;
                        CG_DObjGetWorldTagMatrix(&self->pose, obj, boneName, ent_axis, ent_axis[3]);
                        if ( XModelGetBoneIndex(model, boneName, 0, &boneIndex) )
                        {
                            MatrixTransformVector43(model->localBoneInfo[boneIndex].offset, ent_axis, hitp);
                        }
                        else
                        {
                            hitp[0] = self->pose.origin[0];
                            hitp[1] = self->pose.origin[1];
                            hitp[2] = self->pose.origin[2];
                        }
                    }
                    else
                    {
                        hitp[0] = self->pose.origin[0];
                        hitp[1] = self->pose.origin[1];
                        hitp[2] = self->pose.origin[2];
                    }
                }
                else
                {
                    hitp[0] = self->pose.origin[0];
                    hitp[1] = self->pose.origin[1];
                    hitp[2] = self->pose.origin[2];
                }
                spin_scale = 7.0f;
                v8 = G_flrand(-1.0, 1.0);
                v9 = G_flrand(-1.0, 1.0);
                v10 = G_flrand(-1.0, 1.0);
                randDir[0] = v8;
                randDir[1] = v9;
                randDir[2] = v10;
                hitp[0] = (float)(spin_scale * v8) + hitp[0];
                hitp[1] = (float)(spin_scale * v9) + hitp[1];
                hitp[2] = (float)(spin_scale * v10) + hitp[2];
                dir[0] = hitp[0] - *(float *)point;
                dir[1] = hitp[1] - *((float *)point + 1);
                dir[2] = hitp[2] - *((float *)point + 2);
                distSquared = (float)((float)(dir[0] * dir[0]) + (float)(dir[1] * dir[1])) + (float)(dir[2] * dir[2]);
                if ( distSquared < radiusSquared )
                {
                    dist = sqrtf(distSquared);
                    damage = (float)((float)(damgeInner - damgeOuter) * (float)(1.0 - (float)(dist / radius))) + damgeOuter;
                    damage = damage * piece->explosiveDamageScale;
                    if ( dist <= 0.001 )
                    {
                        dir[0] = 0.0f;
                        dir[1] = 0.0f;
                        dir[2] = 1.0f;
                    }
                    else
                    {
                        dir[0] = (float)(1.0 / dist) * dir[0];
                        dir[1] = (float)(1.0 / dist) * dir[1];
                        dir[2] = (float)(1.0 / dist) * dir[2];
                    }
                    if ( piece->entityDamageTransfer > 0.0 && (float)(damage * piece->entityDamageTransfer) > entityDamage )
                        entityDamage = damage * piece->entityDamageTransfer;
                    v7 = needsDObjUpdate;
                    needsDObjUpdate = CG_DamagePiece(self->pose.localClientNum, self, index, (int)damage, hitp, dir, mod, 1, 0)
                                                    | v7;
                }
            }
            if ( needsDObjUpdate )
                CG_DestructibleUpdateDObj(self);
        }
    }
}

void Destructible_DumpEventInfo()
{
    centity_s *cent; // [esp+0h] [ebp-Ch]
    int i; // [esp+8h] [ebp-4h]

    Com_PrintWarning(20, "***DESTRUCTIBLE EVENT INFO DUMP BEGIN***\n");
    Com_PrintWarning(20, "NUM DESTRUCTIBLE EVENTS: %d\n", g_destructible_events_count);
    for ( i = 0; i < g_destructible_events_count; ++i )
    {
        Com_PrintWarning(20, "Event %d:\n", i);
        if ( g_destructible_events[i].type == 2 )
        {
            Com_PrintWarning(20, "\tDestructible HITEVENT.\n");
        }
        else
        {
            cent = g_destructible_events[i].ed.self;
            if ( cent->destructible )
            {
                if ( cent->destructible->ddef )
                {
                    Com_PrintWarning(20, "\tEnt Num: %d\n", cent->destructible->entNum);
                    Com_PrintWarning(20, "\tName: ");
                    if ( cent->destructible->ddef->name )
                        Com_PrintWarning(20, "%s\n", cent->destructible->ddef->name);
                    else
                        Com_PrintWarning(20, "[Blank]\n");
                    if ( cent->destructible->ddef->model && cent->destructible->ddef->model->name )
                        Com_PrintWarning(20, "\tModel Name: %s\n", cent->destructible->ddef->model->name);
                    if ( cent->destructible->ddef->pristineModel && cent->destructible->ddef->pristineModel->name )
                        Com_PrintWarning(20, "\tModel Name: %s\n", cent->destructible->ddef->pristineModel->name);
                    Com_PrintWarning(20, "\tNum Pieces: %d\n", cent->destructible->ddef->numPieces);
                    Com_PrintWarning(20, "\tClient Only: ");
                    if ( cent->destructible->ddef->clientOnly )
                        Com_PrintWarning(20, "YES\n");
                    else
                        Com_PrintWarning(20, "NO\n");
                }
                else
                {
                    Com_PrintWarning(20, "\tNull destructible def pointer\n");
                }
            }
            else
            {
                Com_PrintWarning(20, "\tNull destructible pointer\n");
            }
        }
    }
    Com_PrintWarning(20, "***DESTRUCTIBLE EVENT INFO DUMP END***\n");
}

char __cdecl CG_DamagePiece(
                int localClientNum,
                centity_s *self,
                unsigned __int8 index,
                int damage,
                const float *point,
                const float *dir,
                int mod,
                bool damageChildren,
                int recursiveDepth)
{
    char *loopSoundName; // [esp+1Ch] [ebp-38h]
    int pieceDamage; // [esp+20h] [ebp-34h]
    unsigned __int8 i; // [esp+27h] [ebp-2Dh]
    int j; // [esp+28h] [ebp-2Ch]
    int prev_stage_index; // [esp+2Ch] [ebp-28h]
    DestructibleStage *new_stage; // [esp+30h] [ebp-24h]
    Destructible *destructible; // [esp+34h] [ebp-20h]
    DestructibleStage *prev_stage; // [esp+38h] [ebp-1Ch]
    char needDObjUpdate; // [esp+3Fh] [ebp-15h]
    __int16 prev_health; // [esp+40h] [ebp-14h]
    DestructibleDef *ddef; // [esp+44h] [ebp-10h]
    int new_stage_index; // [esp+48h] [ebp-Ch]
    __int16 new_health; // [esp+4Ch] [ebp-8h]
    bool explosiveDamage; // [esp+53h] [ebp-1h]

    destructible = self->destructible;
    ddef = destructible->ddef;
    if ( recursiveDepth > 20 )
    {
        Com_Printf(20, "Warning: Destructible (%s): The recusrion in DamagePiece is too deep.", ddef->name);
        return 0;
    }
    if ( index >= ddef->numPieces
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                    2343,
                    0,
                    "%s",
                    "index<ddef->numPieces") )
    {
        __debugbreak();
    }
    Destructible_SetDebugDamagePiece(index, damage);
    prev_health = destructible->pieceArray[index].health;
    new_health = prev_health - damage;
    if ( (__int16)(prev_health - damage) < -1 )
        new_health = -1;
    prev_stage_index = Destructible_GetDestructibleStage(ddef, index, prev_health);
    new_stage_index = Destructible_GetDestructibleStage(ddef, index, new_health);
    prev_stage = 0;
    new_stage = 0;
    if ( prev_stage_index >= 0 )
        prev_stage = &ddef->pieces[index].stages[prev_stage_index];
    if ( new_stage_index >= 0 )
        new_stage = &ddef->pieces[index].stages[new_stage_index];
    if ( mod == 7 && prev_stage && (prev_stage->flags & 0xFFFFFFF7) != 0 )
        return 0;
    explosiveDamage = mod == 4 || mod == 6 || mod == 15;
    if ( mod == 15 || mod == 17 )
        CG_DestructibleSetBurn(localClientNum, self, destructible, index, 1);
    if ( !explosiveDamage )
    {
        if ( prev_stage && (prev_stage->flags & 1) != 0 )
            return 0;
        for ( j = prev_stage_index + 1; j <= new_stage_index; ++j )
        {
            if ( (ddef->pieces[index].stages[j].flags & 1) != 0 )
            {
                new_health = (int)(float)((float)ddef->pieces[index].health * ddef->pieces[index].stages[j].breakHealth) + 1;
                new_stage_index = Destructible_GetDestructibleStage(ddef, index, new_health);
                break;
            }
        }
    }
    needDObjUpdate = CG_DestructibleDamage(self, dir, point, index, new_health, mod, 0, recursiveDepth == 0);
    if ( damageChildren && prev_stage_index >= 0 )
    {
        for ( i = 0; i < ddef->numPieces; ++i )
        {
            if ( ddef->pieces[i].parentPiece == index )
            {
                pieceDamage = (int)(float)((float)damage * ddef->pieces[i].parentDamagePercent);
                if ( pieceDamage )
                    needDObjUpdate |= CG_DamagePiece(
                                                            localClientNum,
                                                            self,
                                                            i,
                                                            pieceDamage,
                                                            point,
                                                            dir,
                                                            mod,
                                                            damageChildren,
                                                            recursiveDepth + 1);
            }
        }
    }
    if ( needDObjUpdate && prev_stage )
        CG_NotifyDestructibleScripts(localClientNum, self, prev_stage);
    if ( prev_stage_index != new_stage_index && !index )
    {
        self->nextState.loopSoundId = 0;
        self->nextState.loopSoundFade = 250;
        if ( new_stage_index >= 0 )
        {
            loopSoundName = (char *)new_stage->loopSound;
            if ( loopSoundName )
            {
                self->nextState.loopSoundId = SND_FindAliasId(loopSoundName);
                self->nextState.loopSoundFade = 1000;
            }
        }
    }
    return needDObjUpdate;
}

void __cdecl CG_NotifyDestructibleScripts(int localClientNum, centity_s *self, DestructibleStage *prev_stage)
{
    char *notify; // [esp+0h] [ebp-4h]

    notify = (char *)prev_stage->breakNotify;
    if ( notify )
    {
        Scr_AddString(notify, SCRIPTINSTANCE_CLIENT);
        CScr_NotifyNum(localClientNum, self->nextState.number, 0, cscr_const.broken, 1u);
    }
}

void __cdecl CG_StoreDestructibleState(centity_s *cent, DestructibleState newState)
{
    int i; // [esp+8h] [ebp-24h]
    int j; // [esp+Ch] [ebp-20h]
    DestructibleState tempState; // [esp+10h] [ebp-1Ch] BYREF

    for ( j = 0; j < 5; ++j )
    {
        if ( !cent->destructible->states[j].time && !cent->destructible->bHasBeenHit )
        {
            if ( cent->nitrousVeh || j )
            {
                memcpy(&cent->destructible->states[j], &newState, sizeof(cent->destructible->states[j]));
                cent->destructible->bHasBeenHit = 1;
            }
            else
            {
                cent->destructible->bHasBeenHit = 1;
                cent->destructible->states[0].time = 1;
                cent->destructible->states[0].state.modelState0 = 0x7FFFFFFF;
            }
            return;
        }
    }
    for ( i = 0; i < 5; ++i )
    {
        if ( i == 4 )
        {
            memcpy(&cent->destructible->states[4], &newState, sizeof(cent->destructible->states[4]));
            return;
        }
        memcpy(&tempState, &cent->destructible->states[i + 1], sizeof(tempState));
        memcpy(&cent->destructible->states[i], &tempState, sizeof(cent->destructible->states[i]));
    }
}

void __cdecl CG_RestartDestructibles(int localClientNum)
{
    const cg_s *cgameGlob; // [esp+0h] [ebp-10h]
    const snapshot_s *nextSnap; // [esp+4h] [ebp-Ch]
    Destructible *self; // [esp+8h] [ebp-8h]
    unsigned int i; // [esp+Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    nextSnap = cgameGlob->nextSnap;
    if ( nextSnap )
    {
        for ( i = 0; i < cg_numDestructibles[localClientNum]; ++i )
        {
            self = &cg_destructibles[localClientNum][i];
            if ( self )
            {
                if ( cgameGlob->time - nextSnap->ps.deltaTime <= cgameGlob->time )
                    CG_DestructibleRewindToTime(localClientNum, self, cgameGlob->time - nextSnap->ps.deltaTime);
            }
        }
    }
}

void __cdecl CG_DestructibleRewindToTime(int localClientNum, Destructible *destructible, int time)
{
    __int16 newHealth; // [esp+8h] [ebp-34h]
    unsigned __int8 i; // [esp+Fh] [ebp-2Dh]
    int j; // [esp+10h] [ebp-2Ch]
    LerpEntityStateDestructibleHit state; // [esp+14h] [ebp-28h] BYREF
    centity_s *cent; // [esp+2Ch] [ebp-10h]
    bool hasOldState; // [esp+33h] [ebp-9h]
    DestructibleDef *ddef; // [esp+34h] [ebp-8h]
    bool isDestroyed; // [esp+3Bh] [ebp-1h]

    ddef = destructible->ddef;
    hasOldState = 0;
    isDestroyed = 0;
    for ( j = 0; j < 5; ++j )
    {
        if ( destructible->states[j].time )
        {
            if ( destructible->states[j].time < time )
            {
                state.modelState0 = destructible->states[j].state.modelState0;
                state.modelState1 = destructible->states[j].state.modelState1;
                state.modelState2 = destructible->states[j].state.modelState2;
                state.dummy = destructible->states[j].state.dummy;
                state.modelState3 = destructible->states[j].state.modelState3;
                state.modelState4 = destructible->states[j].state.modelState4;
                hasOldState = 1;
            }
        }
    }
    if ( hasOldState )
    {
        cent = CG_GetEntity(localClientNum, destructible->entNum);
        if ( (cent->nextState.lerp.eFlags2 & 8) != 0 )
            isDestroyed = 1;
        for ( i = 0; i < ddef->numPieces; ++i )
        {
            newHealth = GetHealthFromState(&state, ddef, i);
            if ( isDestroyed )
                newHealth = -1;
            destructible->pieceArray[i].health = newHealth;
        }
        CG_DestructibleUpdateDObj(cent);
    }
}

void __cdecl CG_DestructibleHitEvent(int localClientNum, int event, const entityState_s *es, int lazyprocess)
{
    DestructibleState v4; // [esp-1Ch] [ebp-D8h] BYREF
    float v5; // [esp+8h] [ebp-B4h]
    float *v6; // [esp+Ch] [ebp-B0h]
    float *v7; // [esp+10h] [ebp-ACh]
    float *origin; // [esp+14h] [ebp-A8h]
    //float *trDelta; // [esp+18h] [ebp-A4h]
    //float *trBase; // [esp+1Ch] [ebp-A0h]
    float bone_axis[4][3]; // [esp+28h] [ebp-94h] BYREF
    DObj *obj; // [esp+58h] [ebp-64h]
    unsigned int boneName; // [esp+5Ch] [ebp-60h]
    float hitp[3]; // [esp+60h] [ebp-5Ch] BYREF
    float dist; // [esp+6Ch] [ebp-50h]
    int newHealth; // [esp+70h] [ebp-4Ch]
    unsigned __int8 i; // [esp+77h] [ebp-45h]
    DestructibleState newState; // [esp+78h] [ebp-44h] BYREF
    float dir[3]; // [esp+94h] [ebp-28h] BYREF
    const cg_s *cgameGlob; // [esp+A0h] [ebp-1Ch]
    DestructibleDef *ddef; // [esp+A4h] [ebp-18h]
    float point[3]; // [esp+A8h] [ebp-14h] BYREF
    destructible_event_t *de; // [esp+B4h] [ebp-8h]
    centity_s *self; // [esp+B8h] [ebp-4h]

    if ( lazyprocess )
    {
        if ( g_destructible_events_count >= 64 )
        {
            Com_PrintWarning(20, "CG_DestructibleHitEvent: Destructible events buffer full.\n");
            Destructible_DumpEventInfo();
        }
        else
        {
            de = &g_destructible_events[g_destructible_events_count++];
            de->type = 2;
            de->ehe.localClientNum = localClientNum;
            de->ehe.event = event;
            de->ehe.es = es;
        }
    }
    else
    {
        self = CG_GetEntity(localClientNum, es->otherEntityNum);
        if ( self && self->destructible )
        {
            ddef = self->destructible->ddef;
            if ( (self->destructible->flags & 4) != 0 )
            {
                self->destructible->flags &= ~4u;
                CG_SafeDObjFree(self->pose.localClientNum, self->nextState.number);
                CG_PreProcess_GetDObj(self->pose.localClientNum, self->nextState.number, self->nextState.eType, ddef->model, 0);
            }
            //trBase = (float*)es->lerp.pos.trBase;
            point[0] = es->lerp.pos.trBase[0];
            point[1] = es->lerp.pos.trBase[1];
            point[2] = es->lerp.pos.trBase[2];
            //trDelta = (float *)es->lerp.pos.trDelta;
            dir[0] = es->lerp.pos.trDelta[0];
            dir[1] = es->lerp.pos.trDelta[1];
            dir[2] = es->lerp.pos.trDelta[2];
            cgameGlob = CG_GetLocalClientGlobals(localClientNum);
            if ( !cgameGlob->inKillCam )
            {
                *(_QWORD *)&newState.state.modelState0 = *(_QWORD *)es->lerp.u.turret.gunAngles;
                *(_QWORD *)&newState.state.modelState2 = *(_QWORD *)es->lerp.u.scriptMover.attachModelIndex;
                *(_QWORD *)&newState.state.modelState3 = *((_QWORD *)&es->lerp.u.createDynEnt + 2);
                newState.time = cgameGlob->time;
                memcpy(&v4, &newState, sizeof(v4));
                CG_StoreDestructibleState(self, v4);
            }
            for ( i = 0; i < ddef->numPieces; ++i )
            {
                newHealth = GetHealthFromState((const LerpEntityStateDestructibleHit *)&es->lerp.u, ddef, i);
                if ( newHealth < self->destructible->pieceArray[i].health )
                {
                    if ( event == 55 )
                    {
                        memset(hitp, 0, sizeof(hitp));
                        boneName = Destructible_GetShowBoneNameForPiece(self->destructible, i);
                        if ( boneName )
                        {
                            obj = Com_GetClientDObj(self->nextState.number, self->pose.localClientNum);
                            if ( obj )
                            {
                                CG_DObjGetWorldTagMatrix(&self->pose, obj, boneName, bone_axis, bone_axis[3]);
                                v7 = bone_axis[3];
                                *(_QWORD *)hitp = *(_QWORD *)&bone_axis[3][0];
                                hitp[2] = bone_axis[3][2];
                            }
                            else
                            {
                                origin = self->pose.origin;
                                hitp[0] = self->pose.origin[0];
                                hitp[1] = self->pose.origin[1];
                                hitp[2] = self->pose.origin[2];
                            }
                        }
                        else
                        {
                            v6 = self->pose.origin;
                            hitp[0] = self->pose.origin[0];
                            hitp[1] = self->pose.origin[1];
                            hitp[2] = self->pose.origin[2];
                        }
                        dir[0] = hitp[0] - point[0];
                        dir[1] = hitp[1] - point[1];
                        dir[2] = hitp[2] - point[2];
                        dist = (float)((float)((float)(hitp[0] - point[0]) * (float)(hitp[0] - point[0]))
                                                 + (float)((float)(hitp[1] - point[1]) * (float)(hitp[1] - point[1])))
                                 + (float)((float)(hitp[2] - point[2]) * (float)(hitp[2] - point[2]));
                        if ( dist <= 0.001 )
                        {
                            dir[0] = 0.0f;
                            dir[1] = 0.0f;
                            dir[2] = 1.0f;
                        }
                        else
                        {
                            v5 = 1.0 / sqrtf(dist);
                            dir[0] = v5 * dir[0];
                            dir[1] = v5 * dir[1];
                            dir[2] = v5 * dir[2];
                        }
                    }
                    CG_DestructibleDamage(self, dir, point, i, newHealth, es->un1.scale, 1, 1);
                }
            }
        }
    }
}

void __cdecl CG_ProcessDestructibleEvents()
{
    int type; // [esp+14h] [ebp-18h]
    destructible_event_t *de; // [esp+20h] [ebp-Ch]
    int i; // [esp+24h] [ebp-8h]

    PROF_SCOPED("CG_ProcessDestructibleEvents");

    for ( i = 0; i < g_destructible_events_count; ++i )
    {
        de = &g_destructible_events[i];
        type = de->type;
        if ( de->type )
        {
            if ( type == 1 )
            {
                CG_DestructibleRadiusDamage(
                    de->ed.self,
                    &de->ed.attacker,
                    de->ed.dir[2],
                    de->ed.point[0],
                    de->ed.point[1],
                    de->erd.mod,
                    0);
            }
            else if ( type == 2 )
            {
                CG_DestructibleHitEvent(de->ehe.localClientNum, de->ehe.event, de->ehe.es, 0);
            }
        }
        else
        {
            CG_DestructibleDamage(
                de->ed.self,
                de->ed.attacker,
                de->ed.dir,
                de->ed.point,
                de->ed.damage,
                de->ed.mod,
                de->ed.modelIndex,
                de->ed.partName,
                0);
        }
    }
    g_destructible_events_count = 0;
}

void __cdecl CG_DestructibleDamage(
                centity_s *self,
                const centity_s *attacker,
                const float *dir,
                const float *point,
                int damage,
                int mod,
                unsigned int modelIndex,
                unsigned int partName,
                int lazyprocess)
{
    int randDamage; // [esp+1Ch] [ebp-20h]
    unsigned __int8 i; // [esp+23h] [ebp-19h]
    destructible_event_t *de; // [esp+24h] [ebp-18h]
    Destructible *destructible; // [esp+30h] [ebp-Ch]
    char needDObjUpdate; // [esp+36h] [ebp-6h]
    unsigned __int8 index; // [esp+37h] [ebp-5h] BYREF
    DestructibleDef *ddef; // [esp+38h] [ebp-4h]

    if ( lazyprocess )
    {
        if ( g_destructible_events_count >= 64 )
        {
            Com_PrintWarning(20, "CG_DestructibleDamage: Destructible events buffer full.\n");
            Destructible_DumpEventInfo();
        }
        else
        {
            de = &g_destructible_events[g_destructible_events_count++];
            de->type = 0;
            de->ehe.localClientNum = (int)self;
            de->ehe.event = (int)attacker;
            de->ed.dir[0] = *dir;
            de->ed.dir[1] = dir[1];
            de->ed.dir[2] = dir[2];
            de->ed.point[0] = *point;
            de->ed.point[1] = point[1];
            de->ed.point[2] = point[2];
            de->ed.damage = damage;
            de->ed.mod = mod;
            de->ed.modelIndex = modelIndex;
            de->ed.partName = partName;
        }
    }
    else
    {
        PROF_SCOPED("CG_DestructibleDamage2");

        destructible = self->destructible;
        if ( destructible )
        {
            ddef = destructible->ddef;
            if ( (destructible->flags & 4) != 0 )
            {
                destructible->flags &= ~4u;
                CG_SafeDObjFree(self->pose.localClientNum, self->nextState.number);
                CG_PreProcess_GetDObj(self->pose.localClientNum, self->nextState.number, self->nextState.eType, ddef->model, 0);
            }
            needDObjUpdate = 0;
            Destructible_GetPieceIndexFromBoneName(destructible, modelIndex, partName, &index);
            if ( index >= ddef->numPieces
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                            2905,
                            0,
                            "%s",
                            "index<ddef->numPieces") )
            {
                __debugbreak();
            }
            if ( ddef->clientOnly )
            {
                Destructible_SetDebugRender(destructible);
                if ( mod == 17 )
                {
                    for ( i = 0; i < ddef->numPieces; ++i )
                    {
                        randDamage = CG_rand() % damage + 1;
                        needDObjUpdate = CG_DamagePiece(self->pose.localClientNum, self, i, randDamage, point, dir, 17, 1, 0);
                    }
                }
                else
                {
                    needDObjUpdate = CG_DamagePiece(
                                                         self->pose.localClientNum,
                                                         self,
                                                         index,
                                                         (int)(float)((float)damage * ddef->pieces[index].bulletDamageScale),
                                                         point,
                                                         dir,
                                                         mod,
                                                         1,
                                                         0);
                }
            }
            else
            {
                needDObjUpdate = CG_DestructibleDamage(
                                                     self,
                                                     dir,
                                                     point,
                                                     index,
                                                     destructible->pieceArray[index].health,
                                                     mod,
                                                     0,
                                                     1);
            }
            if ( needDObjUpdate )
                CG_DestructibleUpdateDObj(self);
        }
    }
}

bool __cdecl CG_DestructibleDamage(
                centity_s *self,
                const float *dir,
                const float *point,
                signed int pieceIndex,
                int newHealth,
                int mod,
                bool updateDObj,
                bool applyForce)
{
    unsigned int AliasId; // eax
    int XDollForConstraints; // eax
    int v11; // [esp+8h] [ebp-90h]
    bool v12; // [esp+1Fh] [ebp-79h]
    PhysPreset *physPreset; // [esp+20h] [ebp-78h]
    int nextStage; // [esp+24h] [ebp-74h]
    int curStage; // [esp+28h] [ebp-70h]
    int numModelsForPiece; // [esp+30h] [ebp-68h]
    bool hadPhysics; // [esp+37h] [ebp-61h]
    float tagOrigin[3]; // [esp+38h] [ebp-60h] BYREF
    float tagMat[3][3]; // [esp+44h] [ebp-54h] BYREF
    int damage; // [esp+68h] [ebp-30h]
    DestructiblePiece *piece; // [esp+6Ch] [ebp-2Ch]
    bool breakPieceAlreadyAppliedForce; // [esp+73h] [ebp-25h]
    __int16 prev_health; // [esp+74h] [ebp-24h]
    int newDModel; // [esp+78h] [ebp-20h]
    int prevDModel; // [esp+7Ch] [ebp-1Ch]
    DestructibleDef *ddef; // [esp+80h] [ebp-18h]
    int localClientNum; // [esp+88h] [ebp-10h]
    Destructible *destructible; // [esp+8Ch] [ebp-Ch]
    bool needDObjUpdate; // [esp+93h] [ebp-5h]
    int entity_index; // [esp+94h] [ebp-4h]

    PROF_SCOPED("CG_DestructibleDamage1");

    if ( !self->destructible )
        self->destructible = Destructible_GetDestructible(self->nextState.number);
    destructible = self->destructible;
    entity_index = self->nextState.number;
    localClientNum = self->pose.localClientNum;
    needDObjUpdate = 0;
    if ( destructible )
    {
        ddef = destructible->ddef;
        if ( (destructible->flags & 4) != 0 )
        {
            destructible->flags &= ~4u;
            CG_SafeDObjFree(self->pose.localClientNum, self->nextState.number);
            CG_PreProcess_GetDObj(self->pose.localClientNum, self->nextState.number, self->nextState.eType, ddef->model, 0);
        }
        if ( pieceIndex >= ddef->numPieces
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                        2972,
                        0,
                        "%s",
                        "(int)pieceIndex<ddef->numPieces") )
        {
            __debugbreak();
        }
        prev_health = destructible->pieceArray[pieceIndex].health;
        destructible->pieceArray[pieceIndex].health = newHealth;
        prevDModel = Destructible_GetDestructibleStage(ddef, pieceIndex, prev_health);
        newDModel = Destructible_GetDestructibleStage(ddef, pieceIndex, newHealth);
        if ( mod == 17 )
            CG_DestructibleSetBurn(self->pose.localClientNum, self, destructible, pieceIndex, 1);
        if ( newDModel != -1
            && ddef->pieces[pieceIndex].physConstraints
            && ddef->pieces[pieceIndex].physConstraints->data[0].type == CONSTRAINT_FAKE_SHAKE )
        {
            CG_DestructibleShake(localClientNum, self, pieceIndex, dir, point, mod);
        }
        if ( ddef->clientOnly )
        {
            if ( newDModel == prevDModel && prevDModel != -1 && newHealth < prev_health )
            {
                piece = &ddef->pieces[pieceIndex];
                if ( piece->damageSound )
                {
                    damage = prev_health - newHealth;
                    if ( (float)damage >= cg_vehicle_piece_damagesfx_threshold->current.value )
                    {
                        CG_GetTagTransform(self, ddef, pieceIndex, tagOrigin, tagMat);
                        AliasId = SND_FindAliasId((char *)piece->damageSound);
                        CG_PlaySound(self->pose.localClientNum, self->nextState.number, tagOrigin, 0, 0, 1.0, AliasId);
                    }
                }
            }
        }
        breakPieceAlreadyAppliedForce = 0;
        if ( newDModel != prevDModel && prevDModel != -1 )
        {
            numModelsForPiece = Destructible_GetNumStagesForPiece(ddef, pieceIndex);
            if ( newDModel == -1 )
                v11 = numModelsForPiece;
            else
                v11 = newDModel;
            hadPhysics = self->destructible->pieceArray[pieceIndex].xdollHandle != 0;
            for ( curStage = prevDModel; curStage < v11; ++curStage )
            {
                nextStage = curStage + 1;
                if ( curStage == v11 - 1 && newDModel == -1 )
                    nextStage = -1;
                CG_DestructibleBreakPiece(self, &ddef->pieces[pieceIndex], pieceIndex, curStage, nextStage, point, dir, mod);
            }
            if ( !hadPhysics && self->destructible->pieceArray[pieceIndex].xdollHandle )
                breakPieceAlreadyAppliedForce = 1;
            needDObjUpdate = 1;
            if ( updateDObj )
                CG_DestructibleUpdateDObj(self);
        }
        if ( !destructible->pieceArray[pieceIndex].xdollHandle )
        {
            physPreset = Destructible_GetPiecePhysPreset(ddef, pieceIndex);
            XDollForConstraints = XDoll_CreateXDollForConstraints(
                                                            localClientNum,
                                                            entity_index,
                                                            pieceIndex,
                                                            newHealth,
                                                            ddef->pieces[pieceIndex].physConstraints,
                                                            physPreset);
            destructible->pieceArray[pieceIndex].xdollHandle = XDollForConstraints;
        }
        if ( destructible->pieceArray[pieceIndex].xdollHandle )
        {
            destructible->flags |= 1u;
            XDoll_UpdateHealth(destructible->pieceArray[pieceIndex].xdollHandle, newHealth);
            if ( applyForce && !breakPieceAlreadyAppliedForce )
                XDoll_ApplyForce(destructible->pieceArray[pieceIndex].xdollHandle, point, dir, mod);
        }
    }
    v12 = needDObjUpdate;
    return v12;
}

unsigned __int8 *__cdecl CM_Hunk_AllocDestructible(unsigned int size)
{
    return Hunk_Alloc(size, "CM_Hunk_AllocDestructible", 11);
}

DestructibleDef *__cdecl DestructibleDefPrecache(const char *name)
{
    if ( useFastFile->current.enabled )
        return DestructibleDefPrecache_FastFile(name);
    else
        return DestructibleDefPrecache_LoadObj(name);
}

DestructibleDef *__cdecl DestructibleDefPrecache_LoadObj(const char *name)
{
    DestructibleDef *destructibleDef; // [esp+0h] [ebp-4h]
    DestructibleDef *destructibleDefa; // [esp+0h] [ebp-4h]

    if ( !*name )
        return 0;
    destructibleDef = (DestructibleDef *)Hunk_FindDataForFile(9, name);
    if ( destructibleDef )
        return destructibleDef;
    destructibleDefa = DestructibleDefLoadFile(name, (void *(__cdecl *)(int))CM_Hunk_AllocDestructible);
    if ( !destructibleDefa )
        return 0;
    destructibleDefa->name = Hunk_SetDataForFile(
                                                         9,
                                                         name,
                                                         destructibleDefa,
                                                         (void *(__cdecl *)(int))CM_Hunk_AllocDestructible);
    return destructibleDefa;
}

DestructibleDef *__cdecl DestructibleDefPrecache_FastFile(const char *name)
{
    if ( !*name )
        return 0;
    return DB_FindXAssetHeader(ASSET_TYPE_DESTRUCTIBLEDEF, (char*)name, 1, -1).destructibleDef;
}

int __cdecl Destructible_GetPieceCount(Destructible *obj)
{
    return obj->ddef->numPieces;
}

unsigned int __cdecl Destructible_XDollActive(Destructible *obj)
{
    return obj->flags & 1;
}

int __cdecl Destructible_GetXDollHandle(Destructible *obj, int piece_index)
{
    return obj->pieceArray[piece_index].xdollHandle;
}

environment_rigid_body *__cdecl Destructible_GetRigidBody()
{
    return phys_sys::get_environment_rigid_body();
}

char __cdecl Destructible_NeedsGameStateUpdate(const Destructible *d)
{
    int prev_stage_index; // [esp+8h] [ebp-10h]
    int i; // [esp+10h] [ebp-8h]
    DestructibleDef *ddef; // [esp+14h] [ebp-4h]

    ddef = d->ddef;
    for ( i = 0; i < ddef->numPieces; ++i )
    {
        prev_stage_index = Destructible_GetDestructibleStage(ddef, i, ddef->pieces[i].health);
        if ( prev_stage_index != Destructible_GetDestructibleStage(ddef, i, d->pieceArray[i].health) )
            return 1;
    }
    return 0;
}

void __cdecl Destructible_WriteGameState(msg_t *msg)
{
    int j; // [esp+0h] [ebp-10h]
    const Destructible *d; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]
    int numberDestructiblesSent; // [esp+Ch] [ebp-4h]

    numberDestructiblesSent = 0;
    for ( i = 0; i < 64 && numberDestructiblesSent < 32; ++i )
    {
        if ( s_destructibles[i].entNum != 1023 )
        {
            d = &s_destructibles[i];
            if ( Destructible_NeedsGameStateUpdate(d) )
            {
                ++numberDestructiblesSent;
                MSG_WriteShort(msg, d->entNum);
                MSG_WriteByte(msg, d->ddef->numPieces);
                for ( j = 0; j < d->ddef->numPieces; ++j )
                    MSG_WriteShort(msg, d->pieceArray[j].health);
            }
        }
    }
    MSG_WriteShort(msg, 1023);
}

void __cdecl Destructible_ReadGameState(msg_t *msg, int localClientNum)
{
    int i; // [esp+4h] [ebp-10h]
    int numPieces; // [esp+8h] [ebp-Ch]
    destructible_gamestate *dgs; // [esp+Ch] [ebp-8h]
    int entityNum; // [esp+10h] [ebp-4h]

    s_num_destructible_gamestates[localClientNum] = 0;
    for ( entityNum = MSG_ReadShort(msg); entityNum != 1023; entityNum = MSG_ReadShort(msg) )
    {
        numPieces = MSG_ReadByte(msg);
        dgs = &s_destructible_gamestates[localClientNum][s_num_destructible_gamestates[localClientNum]];
        dgs->entityNum = entityNum;
        dgs->numPieces = numPieces;
        for ( i = 0; i < numPieces; ++i )
            dgs->health[i] = MSG_ReadShort(msg);
        ++s_num_destructible_gamestates[localClientNum];
    }
}

void __cdecl Demo_Destructible_WriteGameState(msg_t *msg, int localClientNum)
{
    int j; // [esp+0h] [ebp-Ch]
    const Destructible *d; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    for ( i = 0; i < 64; ++i )
    {
        if ( cg_destructibles[localClientNum][i].entNum != 1023 )
        {
            d = &cg_destructibles[localClientNum][i];
            MSG_WriteShort(msg, d->entNum);
            MSG_WriteShort(msg, i);
            MSG_WriteByte(msg, d->ddef->numPieces);
            for ( j = 0; j < d->ddef->numPieces; ++j )
                MSG_WriteShort(msg, d->pieceArray[j].health);
        }
    }
    MSG_WriteShort(msg, 1023);
}

void __cdecl Demo_Destructible_ReadGameState(msg_t *msg, int localClientNum)
{
    Destructible *d; // [esp+Ch] [ebp-18h]
    centity_s *cent; // [esp+10h] [ebp-14h]
    int entityNum; // [esp+18h] [ebp-Ch]
    int numPieces; // [esp+1Ch] [ebp-8h]
    int i; // [esp+20h] [ebp-4h]
    int ia; // [esp+20h] [ebp-4h]

kisak123: // this was a psycho name
    for ( entityNum = MSG_ReadShort(msg); entityNum != 1023; entityNum = MSG_ReadShort(msg) )
    {
        d = &cg_destructibles[localClientNum][MSG_ReadShort(msg)];
        numPieces = MSG_ReadByte(msg);
        if ( d->entNum != entityNum || !d->ddef || d->ddef->numPieces != numPieces || !d->pieceArray )
        {
            for ( i = 0; i < numPieces; ++i )
                MSG_ReadShort(msg);
            goto kisak123;
        }
        for ( ia = 0; ia < numPieces; ++ia )
        {
            d->pieceArray[ia].health = MSG_ReadShort(msg);
            d->pieceArray[ia].xdollHandle = 0;
        }
        d->flags &= ~1u;
        d->destructiblePoseID = 0;
        cent = CG_GetEntity(localClientNum, d->entNum);
        CG_DestructibleUpdateDObj(cent);
    }
}

unsigned __int8 __cdecl CG_GetFreeDestructiblePose()
{
    unsigned __int8 i; // [esp+3h] [ebp-1h]

    for ( i = 0; i < 2u; ++i )
    {
        if ( !s_cgDestructiblePoses[i].destructibleID )
            return i + 1;
    }
    return 0;
}

void __cdecl CG_FreeDestructiblePose(int localClientNum, unsigned __int8 destructiblePoseID)
{
    int i; // [esp+Ch] [ebp-10h]
    DestructiblePose *dpose; // [esp+10h] [ebp-Ch]
    Destructible *destructible; // [esp+14h] [ebp-8h]

    dpose = CG_GetDestructiblePose(destructiblePoseID);
    if ( !dpose
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp", 3568, 0, "%s", "dpose") )
    {
        __debugbreak();
    }
    if ( !dpose->destructibleID
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                    3569,
                    0,
                    "%s",
                    "dpose->destructibleID") )
    {
        __debugbreak();
    }
    destructible = CG_GetDestructibleByIndex(localClientNum, dpose->destructibleID);
    CG_GetEntity(localClientNum, destructible->entNum)->pose.destructiblePose = 0;
    destructible->destructiblePoseID = 0;
    dpose->destructibleID = 0;
    for ( i = 0; i < 32; ++i )
        dpose->bones[i].boneid = 0;
    dpose->numBones = 0;
}

DestructiblePose *__cdecl CG_GetDestructiblePose(__int16 index)
{
    __int16 indexa; // [esp+8h] [ebp+8h]

    indexa = index - 1;
    if ( (unsigned int)indexa >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                    3588,
                    0,
                    "%s\n\t(index) = %i",
                    "(index >= 0 && index < 2)",
                    indexa) )
    {
        __debugbreak();
    }
    return &s_cgDestructiblePoses[indexa];
}

void __cdecl CG_DestructibleShake(
                int localClientNum,
                centity_s *cent,
                int pieceIndex,
                const float *dir,
                const float *hitp,
                int mod)
{
    float *angles_vel; // [esp+10h] [ebp-ECh]
    float *v7; // [esp+14h] [ebp-E8h]
    float *angles_original; // [esp+1Ch] [ebp-E0h]
    float tohitp[3]; // [esp+20h] [ebp-DCh] BYREF
    float bonepos[3]; // [esp+2Ch] [ebp-D0h]
    float local_avel[3]; // [esp+38h] [ebp-C4h] BYREF
    float avel[3]; // [esp+44h] [ebp-B8h] BYREF
    float axis[3][3]; // [esp+50h] [ebp-ACh] BYREF
    float boneQuat[4]; // [esp+74h] [ebp-88h] BYREF
    float tempBoneMtx[4][3]; // [esp+84h] [ebp-78h] BYREF
    const DObjAnimMat *worldBoneMtx; // [esp+B4h] [ebp-48h]
    float controlAngles[3]; // [esp+B8h] [ebp-44h] BYREF
    const DObjAnimMat *boneMtxList; // [esp+C4h] [ebp-38h]
    unsigned __int8 boneIndex; // [esp+CBh] [ebp-31h] BYREF
    XModel *model; // [esp+CCh] [ebp-30h]
    float rot[3]; // [esp+D0h] [ebp-2Ch]
    float scale; // [esp+DCh] [ebp-20h]
    Destructible_BonePose *bonepose; // [esp+E0h] [ebp-1Ch]
    int i; // [esp+E4h] [ebp-18h]
    PhysConstraints *c; // [esp+E8h] [ebp-14h]
    DestructiblePose *dpose; // [esp+ECh] [ebp-10h]
    Destructible *destructible; // [esp+F0h] [ebp-Ch]
    DestructibleDef *ddef; // [esp+F4h] [ebp-8h]
    int attachBoneHash; // [esp+F8h] [ebp-4h]

    destructible = cent->destructible;
    ddef = destructible->ddef;
    dpose = 0;
    attachBoneHash = Destructible_GetShowBoneNameForPiece(destructible, pieceIndex);
    if ( attachBoneHash )
    {
        if ( destructible->destructiblePoseID )
        {
            dpose = CG_GetDestructiblePose(destructible->destructiblePoseID);
        }
        else
        {
            destructible->destructiblePoseID = CG_GetFreeDestructiblePose();
            cent->pose.destructiblePose = destructible->destructiblePoseID;
            dpose = CG_GetDestructiblePose(destructible->destructiblePoseID);
            dpose->destructibleID = CG_GetDestructibleIndex(localClientNum, destructible);
        }
        if ( dpose )
        {
            c = ddef->pieces[pieceIndex].physConstraints;
            for ( i = 0; i < 32; ++i )
            {
                bonepose = &dpose->bones[i];
                if ( !bonepose->boneid || bonepose->boneid == attachBoneHash )
                {
                    if ( !bonepose->boneid )
                    {
                        model = ddef->model;
                        if ( XModelGetBoneIndex(model, attachBoneHash, 0, &boneIndex) )
                        {
                            boneMtxList = XModelGetBasePose(model);
                            worldBoneMtx = &boneMtxList[boneIndex];
                            boneQuat[0] = worldBoneMtx->quat[0];
                            boneQuat[1] = worldBoneMtx->quat[1];
                            boneQuat[2] = worldBoneMtx->quat[2];
                            boneQuat[3] = worldBoneMtx->quat[3];
                            QuatToAxis(boneQuat, tempBoneMtx);
                            AxisToSignedAngles(tempBoneMtx, controlAngles);
                            angles_original = bonepose->angles_original;
                            bonepose->angles_original[0] = controlAngles[0];
                            angles_original[1] = controlAngles[1];
                            angles_original[2] = controlAngles[2];
                            bonepose->boneid = attachBoneHash;
                            bonepose->pieceIndex = pieceIndex;
                            ++dpose->numBones;
                        }
                    }
                    if ( bonepose->boneid != attachBoneHash
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                                    3652,
                                    0,
                                    "%s",
                                    "bonepose.boneid == (int)attachBoneHash") )
                    {
                        __debugbreak();
                    }
                    switch ( mod )
                    {
                        case 4:
                        case 6:
                        case 15:
                            goto $LN4_164;
                        case 17:
                            scale = burn_force;
$LN4_164:
                            scale = grenade_force;
                            break;
                        default:
                            scale = bullet_force;
                            break;
                    }
                    if ( mod == 17 )
                    {
                        rot[1] = 0.0f;
                        rot[0] = burn_force * 0.5;
                        rot[2] = burn_force;
                    }
                    else
                    {
                        bonepos[0] = cent->pose.origin[0];
                        bonepos[1] = cent->pose.origin[1];
                        bonepos[2] = cent->pose.origin[2];
                        tohitp[0] = *hitp - bonepos[0];
                        tohitp[1] = hitp[1] - bonepos[1];
                        tohitp[2] = hitp[2] - bonepos[2];
                        Vec3Cross(tohitp, dir, avel);
                        AnglesToAxis(bonepose->angles_original, axis);
                        Vec3RotateTranspose(avel, axis, local_avel);
                        rot[1] = (float)(avel[2] * scale) * c->data[0].scale[2];
                        rot[0] = (float)(local_avel[1] * scale) * c->data[0].scale[1];
                        rot[2] = (float)(local_avel[0] * scale) * c->data[0].scale[0];
                    }
                    angles_vel = bonepose->angles_vel;
                    v7 = bonepose->angles_vel;
                    bonepose->angles_vel[0] = bonepose->angles_vel[0] + rot[0];
                    angles_vel[1] = v7[1] + rot[1];
                    angles_vel[2] = v7[2] + rot[2];
                    if ( ((LODWORD(bonepose->angles_vel[0]) & 0x7F800000) == 0x7F800000
                         || (LODWORD(bonepose->angles_vel[1]) & 0x7F800000) == 0x7F800000
                         || (LODWORD(bonepose->angles_vel[2]) & 0x7F800000) == 0x7F800000)
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                                    3696,
                                    0,
                                    "%s",
                                    "!IS_NAN((bonepose.angles_vel)[0]) && !IS_NAN((bonepose.angles_vel)[1]) && !IS_NAN((bonepose.angles_vel)[2])") )
                    {
                        __debugbreak();
                    }
                    return;
                }
            }
        }
    }
}

void __cdecl Vec3RotateTranspose(const float *in, const float (*matrix)[3], float *out)
{
    if ( in == out
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_vector.h", 533, 0, "%s", "in != out") )
    {
        __debugbreak();
    }
    *out = (float)((float)(*in * (*matrix)[0]) + (float)(in[1] * (float)(*matrix)[3]))
             + (float)(in[2] * (float)(*matrix)[6]);
    out[1] = (float)((float)(*in * (float)(*matrix)[1]) + (float)(in[1] * (float)(*matrix)[4]))
                 + (float)(in[2] * (float)(*matrix)[7]);
    out[2] = (float)((float)(*in * (float)(*matrix)[2]) + (float)(in[1] * (float)(*matrix)[5]))
                 + (float)(in[2] * (float)(*matrix)[8]);
}

void __cdecl CG_DestructibleUpdatePoses(int localClientNum, int msec)
{
    int k; // [esp+18h] [ebp-28h]
    PhysConstraints *c; // [esp+1Ch] [ebp-24h]
    float spring_k; // [esp+20h] [ebp-20h]
    float damp_k; // [esp+24h] [ebp-1Ch]
    Destructible_BonePose *bpose; // [esp+28h] [ebp-18h]
    int boneindex; // [esp+2Ch] [ebp-14h]
    Destructible *destructible; // [esp+30h] [ebp-10h]
    DestructiblePose *pose; // [esp+34h] [ebp-Ch]
    unsigned __int8 i; // [esp+3Bh] [ebp-5h]
    float delta_t; // [esp+3Ch] [ebp-4h]

    delta_t = (float)msec * 0.001;
    for ( i = 0; i < 2u; ++i )
    {
        pose = &s_cgDestructiblePoses[i];
        if ( pose->destructibleID )
        {
            destructible = CG_GetDestructibleByIndex(localClientNum, pose->destructibleID);
            for ( boneindex = 0; boneindex < pose->numBones; ++boneindex )
            {
                bpose = &pose->bones[boneindex];
                if ( !bpose->boneid
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp",
                                3717,
                                0,
                                "%s",
                                "bpose.boneid") )
                {
                    __debugbreak();
                }
                if ( destructible->ddef )
                {
                    c = destructible->ddef->pieces[bpose->pieceIndex].physConstraints;
                    if ( c )
                    {
                        damp_k = c->data[0].damp;
                        spring_k = c->data[0].power;
                        for ( k = 0; k < 3; ++k )
                        {
                            bpose->angles_vel[k] = bpose->angles_vel[k]
                                                                     - (float)((float)(spring_k * bpose->angles_offset[k]) * delta_t);
                            bpose->angles_vel[k] = bpose->angles_vel[k] - (float)((float)(bpose->angles_vel[k] * damp_k) * delta_t);
                            bpose->angles_offset[k] = (float)(bpose->angles_vel[k] * delta_t) + bpose->angles_offset[k];
                        }
                        if ( (float)((float)((float)(bpose->angles_vel[0] * bpose->angles_vel[0])
                                                             + (float)(bpose->angles_vel[1] * bpose->angles_vel[1]))
                                             + (float)(bpose->angles_vel[2] * bpose->angles_vel[2])) < 0.000099999997
                            && (float)((float)((float)(bpose->angles_offset[0] * bpose->angles_offset[0])
                                                             + (float)(bpose->angles_offset[1] * bpose->angles_offset[1]))
                                             + (float)(bpose->angles_offset[2] * bpose->angles_offset[2])) < 0.000099999997 )
                        {
                            memcpy(bpose, &pose->bones[--pose->numBones], sizeof(Destructible_BonePose));
                            pose->bones[pose->numBones].boneid = 0;
                            if ( !pose->numBones )
                            {
                                CG_FreeDestructiblePose(localClientNum, i + 1);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void __cdecl Destructible_SetDebugRender(Destructible *destructible)
{
    int i; // [esp+0h] [ebp-4h]

    g_DebugRenderDestructible = destructible;
    for ( i = 0; i < 32; ++i )
        g_DebugLastPieceIndex[i] = -1;
}

void __cdecl Destructible_SetDebugDamagePiece(int piece, int damage)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 32; ++i )
    {
        if ( g_DebugLastPieceIndex[i] == -1 )
        {
            g_DebugLastPieceIndex[i] = piece;
            g_DebugLastPieceDamage[i] = damage;
            return;
        }
    }
}

int __cdecl Destructible_IsDebugDamagePiece(int piece)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 32; ++i )
    {
        if ( g_DebugLastPieceIndex[i] == piece )
            return g_DebugLastPieceDamage[i];
    }
    return -1;
}

void __cdecl Destructible_DebugRender()
{
    char *v0; // eax
    char *v1; // eax
    int v2; // eax
    Font_s *smallDevFont; // [esp+1Ch] [ebp-44h]
    Font_s *v4; // [esp+1Ch] [ebp-44h]
    char *boneName; // [esp+24h] [ebp-3Ch]
    unsigned int boneNameHash; // [esp+28h] [ebp-38h]
    int damage; // [esp+2Ch] [ebp-34h]
    int health; // [esp+30h] [ebp-30h]
    int stage; // [esp+34h] [ebp-2Ch]
    float color[4]; // [esp+38h] [ebp-28h] BYREF
    unsigned __int8 i; // [esp+4Bh] [ebp-15h]
    DestructibleDef *ddef; // [esp+4Ch] [ebp-14h]
    const gentity_s *ent; // [esp+50h] [ebp-10h]
    float x; // [esp+54h] [ebp-Ch]
    float y; // [esp+58h] [ebp-8h]
    float fontScale; // [esp+5Ch] [ebp-4h]

    if ( g_destructibleDraw )
    {
        if ( g_destructibleDraw->current.enabled )
        {
            fontScale = 0.7f;
            if ( g_DebugRenderDestructible )
            {
                if ( g_DebugRenderDestructible->entNum != 1023 )
                {
                    x = 8.0f;
                    y = 10.0f;
                    ddef = g_DebugRenderDestructible->ddef;
                    ent = &g_entities[g_DebugRenderDestructible->entNum];
                    smallDevFont = cgMedia.smallDevFont;
                    v0 = va("%s Health %d Entnum %d", ddef->name, ent->health, g_DebugRenderDestructible->entNum);
                    y = (float)CG_DrawDevString(scrPlaceView, 8.0, 10.0, fontScale, fontScale, v0, colorWhite, 5, smallDevFont)
                        + 10.0;
                    for ( i = 0; i < ddef->numPieces; ++i )
                    {
                        health = g_DebugRenderDestructible->pieceArray[i].health;
                        stage = Destructible_GetDestructibleStage(ddef, i, health);
                        damage = Destructible_IsDebugDamagePiece(i);
                        if ( stage == -1 )
                        {
                            color[0] = colorRed[0];
                            color[1] = 0.0;
                            color[2] = 0.0;
                            color[3] = 1.0;
                        }
                        else if ( stage )
                        {
                            color[0] = colorYellow[0];
                            color[1] = 1.0;
                            color[2] = 0.0;
                            color[3] = 1.0;
                        }
                        else
                        {
                            if ( health == ddef->pieces[i].health )
                            {
                                color[0] = colorGreen[0];
                                color[1] = 1.0;
                            }
                            else
                            {
                                color[0] = colorLtGreen[0];
                                color[1] = 0.75;
                            }
                            color[2] = 0.0;
                            color[3] = 1.0;
                        }
                        if ( damage < 0 )
                        {
                            v4 = cgMedia.smallDevFont;
                            v1 = va("Piece %d: Health %d, Stage %d", i, health, stage);
                        }
                        else
                        {
                            boneNameHash = Destructible_GetShowBoneNameForPiece(g_DebugRenderDestructible, i);
                            boneName = SL_ConvertToString(boneNameHash, SCRIPTINSTANCE_SERVER);
                            v4 = cgMedia.smallDevFont;
                            v1 = va("Piece %d: Health %d, Stage %d PrevDamage %d %s", i, health, stage, damage, boneName);
                        }
                        v2 = CG_DrawDevString(scrPlaceView, x, y, fontScale, fontScale, v1, color, 5, v4);
                        y = (float)v2 + y;
                    }
                }
            }
        }
    }
}

DestructibleDef *__cdecl Destructible_GetDDef(const centity_s *cent)
{
    if ( cent->destructible )
        return cent->destructible->ddef;
    else
        return 0;
}

DestructibleDef *__cdecl Destructible_GetDDef(const Destructible *const obj)
{
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\destructible.cpp", 3870, 0, "%s", "obj") )
        __debugbreak();
    return obj->ddef;
}

void __cdecl Scr_DestructibleCallback(gentity_s *self, unsigned __int16 event, int piece, float time, int damage)
{
    unsigned __int16 callback; // [esp+8h] [ebp-4h]

    if (g_scr_data.destructible_callback)
    {
        Scr_AddInt(damage, SCRIPTINSTANCE_SERVER);
        Scr_AddFloat(time, SCRIPTINSTANCE_SERVER);
        Scr_AddInt(piece, SCRIPTINSTANCE_SERVER);
        Scr_AddConstString(event, SCRIPTINSTANCE_SERVER);
        callback = Scr_ExecEntThread(self, g_scr_data.destructible_callback, 4u);
        Scr_FreeThread(callback, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl Scr_DestructibleCallback(gentity_s *self, unsigned __int16 event, char *notify, gentity_s *attacker)
{
    unsigned __int16 callback; // [esp+0h] [ebp-4h]

    if (g_scr_data.destructible_callback)
    {
        if ( attacker )
            Scr_AddEntity(attacker, SCRIPTINSTANCE_SERVER);
        else
            Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
        Scr_AddString(notify, SCRIPTINSTANCE_SERVER);
        Scr_AddConstString(event, SCRIPTINSTANCE_SERVER);
        callback = Scr_ExecEntThread(self, g_scr_data.destructible_callback, 3u);
        Scr_FreeThread(callback, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl CScr_DestructibleCallback(centity_s *self, unsigned __int16 event, int piece, float time, int damage)
{
    unsigned __int16 callback; // [esp+8h] [ebp-4h]

    if ( cg_scr_data.destructible_callback )
    {
        Scr_AddInt(damage, SCRIPTINSTANCE_CLIENT);
        Scr_AddFloat(time, SCRIPTINSTANCE_CLIENT);
        Scr_AddInt(piece, SCRIPTINSTANCE_CLIENT);
        Scr_AddConstString(event, SCRIPTINSTANCE_CLIENT);
        callback = CScr_ExecEntThread(self, cg_scr_data.destructible_callback, 4u);
        Scr_FreeThread(callback, SCRIPTINSTANCE_CLIENT);
    }
}

Destructible_BonePose::Destructible_BonePose()
{
    this->boneid = 0;
    this->pieceIndex = 0;
    this->angles_original[0] = 0.0f;
    this->angles_original[1] = 0.0f;
    this->angles_original[2] = 0.0f;
    this->angles_offset[0] = 0.0f;
    this->angles_offset[1] = 0.0f;
    this->angles_offset[2] = 0.0f;
    this->angles_vel[0] = 0.0f;
    this->angles_vel[1] = 0.0f;
    this->angles_vel[2] = 0.0f;
}

