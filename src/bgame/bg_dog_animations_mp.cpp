#include "bg_dog_animations_mp.h"

#include <xanim/xanim.h>
#include "bg_misc.h"
#include <qcommon/common.h>
#include "bg_dog.h"
#include "bg_animation.h"
#include "bg_local.h"

XAnim_s *s_dogAnims;

const char *s_dogAnimNames[38] =
{
  "root",
  "german_shepherd_idle",
  "german_shepherd_walk",
  "german_shepherd_run_stop",
  "german_shepherd_run_pain",
  "german_shepherd_run_attack",
  "german_shepard_run_turn_90_left",
  "german_shepard_run_turn_90_right",
  "german_shepard_turn_90_left",
  "german_shepard_turn_90_right",
  "german_shepard_run_turn_180_left",
  "german_shepard_run_turn_180_right",
  "german_shepherd_run_start_180_L",
  "german_shepherd_run_start_180_R",
  "german_shepard_run_pain_hit_front",
  "german_shepard_run_pain_hit_front",
  "german_shepard_run_pain_hit_front",
  "german_shepard_run_pain_hit_front",
  "german_shepard_pain_hit_front",
  "german_shepard_pain_hit_back",
  "german_shepard_pain_hit_left",
  "german_shepard_pain_hit_right",
  "german_shepherd_death_front",
  "german_shepard_death_hit_back",
  "german_shepard_death_hit_left",
  "german_shepard_death_hit_right",
  "german_shepherd_run_flashbang",
  "german_shepherd_attack_player_miss",
  "german_shepherd_attack_player_miss_turnL",
  "german_shepherd_attack_player_miss_turnR",
  "german_shepard_run_attack_low",
  "german_shepherd_traverse_up_40",
  "german_shepherd_traverse_up_80",
  "german_shepherd_traverse_down_40",
  "german_shepherd_traverse_down_80",
  "german_shepherd_run_jump_40",
  "german_shepherd_run_jump_window_40",
  "german_shepherd_walk"
};

const char *s_dogAnimNamesRunStartKnob[2] =
{ "german_shepherd_run_start_knob", "german_shepherd_run_start" };

const char *s_dogAnimNamesRunKnob[4] =
{
  "german_shepherd_run_knob",
  "german_shepherd_run",
  "german_shepherd_run_lean_L",
  "german_shepherd_run_lean_R"
};

const char *s_dogAnimNamesAttackIdleKnob[4] =
{
  "german_shepherd_attackidle_knob",
  "german_shepherd_attackidle",
  "german_shepherd_attackidle_bark",
  "german_shepherd_attackidle_growl"
};

const char *s_dogAnimNamesLookDown[3] =
{
  "german_shepherd_look_2",
  "german_shepherd_look_down",
  "german_shepherd_attack_look_down"
};

const char *s_dogAnimNamesLookUp[3] =
{
  "german_shepherd_look_8",
  "german_shepherd_look_up",
  "german_shepherd_attack_look_up"
};

const char *s_dogAnimNamesLookRight[3] =
{
  "german_shepherd_look_4",
  "german_shepherd_look_right",
  "german_shepherd_attack_look_right"
};

const char *s_dogAnimNamesLookLeft[3] =
{
  "german_shepherd_look_6",
  "german_shepherd_look_left",
  "german_shepherd_attack_look_left"
};

static float s_animRate = 1.0f;

XAnim_s *__cdecl Dog_GetAnims()
{
    return s_dogAnims;
}

void __cdecl Dog_CreateAnims(void *(__cdecl *xanimAlloc)(unsigned int))
{
    signed int animIndex; // [esp+0h] [ebp-4h]

    if ( !s_dogAnims )
    {
        Dog_DebugPrint("Dog_CreateAnims");
        s_dogAnims = XAnimCreateAnims("DOG_ANIMS", 60, xanimAlloc);
        if ( !s_dogAnims
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_dog_animations_mp.cpp",
                        658,
                        0,
                        "%s",
                        "s_dogAnims") )
        {
            __debugbreak();
        }
        XAnimBlend(s_dogAnims, 0, s_dogAnimNames[0], 1u, 0x2Cu, 0);
        for ( animIndex = 1; animIndex < 38; ++animIndex )
            BG_CreateXAnim(s_dogAnims, animIndex, (char *)s_dogAnimNames[animIndex]);
        BG_Dog_CreateAnimChildGroup(0x26u, 0x2Du, s_dogAnimNamesRunStartKnob, 2, 2u);
        BG_Dog_CreateAnimChildGroup(0x27u, 0x2Eu, s_dogAnimNamesRunKnob, 4, 1u);
        BG_Dog_CreateAnimChildGroup(0x28u, 0x31u, s_dogAnimNamesAttackIdleKnob, 4, 0);
        BG_Dog_CreateAnimChildGroup(0x29u, 0x34u, s_dogAnimNamesLookDown, 3, 0x10u);
        BG_Dog_CreateAnimChildGroup(0x2Au, 0x36u, s_dogAnimNamesLookUp, 3, 0x10u);
        BG_Dog_CreateAnimChildGroup(0x2Bu, 0x38u, s_dogAnimNamesLookRight, 3, 0x10u);
        BG_Dog_CreateAnimChildGroup(0x2Cu, 0x3Au, s_dogAnimNamesLookLeft, 3, 0x10u);
        XAnimSetupSyncNodes(s_dogAnims);
    }
}

void __cdecl Dog_DebugPrint(const char *msg)
{
    if ( !msg
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_dog_animations_mp.cpp", 626, 0, "%s", "msg") )
    {
        __debugbreak();
    }
    if ( dog_debug->current.enabled )
        Com_Printf(17, "%s\n", msg);
}

void __cdecl BG_Dog_CreateAnimChildGroup(
                unsigned int index,
                unsigned int childindex,
                const char **animNames,
                int count,
                unsigned int flags)
{
    int animIndex; // [esp+0h] [ebp-4h]

    XAnimBlend(s_dogAnims, index, *animNames, childindex, count - 1, flags);
    for ( animIndex = 1; animIndex < count; ++animIndex )
        BG_CreateXAnim(s_dogAnims, animIndex + childindex - 1, (char *)animNames[animIndex]);
}

void __cdecl Dog_ShutdownAnims()
{
    s_dogAnims = 0;
}

void __cdecl BG_Actor_FastForwardAnimState(int localClientNum, const entityState_s *es, actorInfo_t *actorInfo)
{
    unsigned int animationState; // [esp+0h] [ebp-4h]

    animationState = es->animState.state & 0xFFFFFFBF;
    if (AIAnimationFuncTable[animationState].pfnSetAtEnd)
        AIAnimationFuncTable[animationState].pfnSetAtEnd(localClientNum, es, actorInfo);
}

void __cdecl BG_Dog_Look_At(int localClientNum, const entityState_s *es)
{
    float rightWeight; // [esp+20h] [ebp-14h]
    DObj *obj; // [esp+24h] [ebp-10h]
    float downWeight; // [esp+28h] [ebp-Ch]
    float leftWeight; // [esp+2Ch] [ebp-8h]
    float upWeight; // [esp+30h] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        upWeight = 0.0f;
        downWeight = 0.0f;
        leftWeight = 0.0f;
        rightWeight = 0.0f;
        if ( es->animState.fAimUpDown <= 0.0 )
            downWeight = -1.0 * es->animState.fAimUpDown;
        else
            upWeight = es->animState.fAimUpDown;
        if ( es->animState.fAimLeftRight <= 0.0 )
            rightWeight = -1.0 * es->animState.fAimLeftRight;
        else
            leftWeight = es->animState.fAimLeftRight;
        XAnimSetGoalWeight(obj, 0x2Au, upWeight, 0.05f, s_animRate, 0, 0, 0, -1);
        XAnimSetGoalWeight(obj, 0x29u, downWeight, 0.05f, s_animRate, 0, 0, 0, -1);
        XAnimSetGoalWeight(obj, 0x2Cu, leftWeight, 0.05f, s_animRate, 0, 0, 0, -1);
        XAnimSetGoalWeight(obj, 0x2Bu, rightWeight, 0.05f, s_animRate, 0, 0, 0, -1);
    }
}

void __cdecl BG_Dog_Move_Start(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    XAnimTree_s *v3; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        v3 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v3, 3u, 0.0, -1);
        XAnimSetCompleteGoalWeight(obj, 0x2Du, 1.0, 0.2, 1.0, 0, 0, 1, -1);
        XAnimSetCompleteGoalWeightKnob(
            obj,
            0x26u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Move_Stop(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    XAnimTree_s *v3; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0x2Du, 0.0, -1);
        v3 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v3, 0x27u, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            3u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Move_Run(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    XAnimTree_s *v4; // eax
    XAnimTree_s *v5; // eax
    XAnimTree_s *v6; // eax
    XAnimTree_s *v7; // eax
    DObj *obj; // [esp+1Ch] [ebp-10h]
    float right; // [esp+20h] [ebp-Ch] BYREF
    float left; // [esp+24h] [ebp-8h] BYREF
    float center; // [esp+28h] [ebp-4h] BYREF

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        if (prevState != ACTOR_ANIMATION_MOVE_WALK)
        {
            Tree = DObjGetTree(obj);
            XAnimClearTreeGoalWeights(Tree, 0x2Du, 0.0, -1);
            v4 = DObjGetTree(obj);
            XAnimClearTreeGoalWeights(v4, 3u, 0.0, -1);
            v5 = DObjGetTree(obj);
            XAnimClearTreeGoalWeights(v5, 0xAu, 0.2, -1);
            v6 = DObjGetTree(obj);
            XAnimClearTreeGoalWeights(v6, 0xBu, 0.2, -1);
            BG_Dog_Clear_Traverse_Anims(localClientNum, es, prevState);
            BG_Dog_Init_Normal_Look_At(localClientNum, es);
        }
        center = 0.0f;
        left = 0.0f;
        right = 0.0f;
        BG_Dog_GetRunAnimWeights(es->animState.fLeanAmount, &center, &left, &right);
        v7 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v7, 2u, 0.30000001, -1);
        XAnimSetCompleteGoalWeight(obj, 0x2Eu, center, 0.2, 1.0, 0, 0, 0, -1);
        XAnimSetCompleteGoalWeight(obj, 0x2Fu, left, 0.2, 1.0, 0, 0, 0, -1);
        XAnimSetCompleteGoalWeight(obj, 0x30u, right, 0.2, 1.0, 0, 0, 0, -1);
        XAnimSetCompleteGoalWeightKnob(
            obj,
            0x27u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            0,
            -1);
        BG_Dog_Look_At(localClientNum, es);
    }
}

void __cdecl BG_Dog_GetRunAnimWeights(float leanAmount, float *center, float *left, float *right)
{
    iassert(!IS_NAN(leanAmount));
    if ( leanAmount <= 0.0 )
    {
        if ( leanAmount < 0.0 && leanAmount > -0.94999999 )
            leanAmount = -0.95f;
    }
    else if ( leanAmount < 0.94999999 )
    {
        leanAmount = 0.95f;
    }
    if ( leanAmount <= 0.0 )
    {
        if ( leanAmount >= 0.0 )
        {
            *left = 0.0f;
            *right = 0.0f;
            *center = 1.0f;
        }
        else
        {
            *right = 0.0f;
            *left = (float)(leanAmount + 1.0) * 20.0;
            if ( *left > 1.0 )
                *left = 1.0f;
            if ( *left < 0.0 )
                *left = 0.0f;
            *center = 1.0 - *left;
        }
    }
    else
    {
        *left = 0.0f;
        *right = (float)(1.0 - leanAmount) * 20.0;
        if ( *right <= 1.0 )
        {
            if ( *right < 0.0 )
                *right = 0.0f;
        }
        else
        {
            *right = 1.0f;
        }
        *center = 1.0 - *right;
    }
}

void __cdecl BG_Dog_Init_Normal_Look_At(int localClientNum, const entityState_s *es)
{
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        BG_Dog_Clear_Look_At(localClientNum, es);
        XAnimSetGoalWeight(obj, 0x36u, 1.0, 0.0, s_animRate, 0, 0, 0, -1);
        XAnimSetGoalWeight(obj, 0x34u, 1.0, 0.0, s_animRate, 0, 0, 0, -1);
        XAnimSetGoalWeight(obj, 0x3Au, 1.0, 0.0, s_animRate, 0, 0, 0, -1);
        XAnimSetGoalWeight(obj, 0x38u, 1.0, 0.0, s_animRate, 0, 0, 0, -1);
    }
}

void __cdecl BG_Dog_Clear_Look_At(int localClientNum, const entityState_s *es)
{
    XAnimTree_s *Tree; // eax
    XAnimTree_s *v3; // eax
    XAnimTree_s *v4; // eax
    XAnimTree_s *v5; // eax
    DObj *obj; // [esp+8h] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0x29u, 0.0, -1);
        v3 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v3, 0x2Au, 0.0, -1);
        v4 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v4, 0x2Cu, 0.0, -1);
        v5 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v5, 0x2Bu, 0.0, -1);
    }
}

void __cdecl BG_Dog_Move_Run_Think(int localClientNum, const entityState_s *es)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-10h]
    float right; // [esp+20h] [ebp-Ch] BYREF
    float left; // [esp+24h] [ebp-8h] BYREF
    float center; // [esp+28h] [ebp-4h] BYREF

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        center = 0.0f;
        left = 0.0f;
        right = 0.0f;
        BG_Dog_GetRunAnimWeights(es->animState.fLeanAmount, &center, &left, &right);
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 2u, 0.30000001, -1);
        XAnimSetCompleteGoalWeight(obj, 0x2Eu, center, 0.2, 1.0, 0, 0, 0, -1);
        XAnimSetCompleteGoalWeight(obj, 0x2Fu, left, 0.2, 1.0, 0, 0, 0, -1);
        XAnimSetCompleteGoalWeight(obj, 0x30u, right, 0.2, 1.0, 0, 0, 0, -1);
        XAnimSetCompleteGoalWeightKnob(
            obj,
            0x27u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            0,
            -1);
        BG_Dog_Look_At(localClientNum, es);
    }
}

void __cdecl BG_Dog_Move_Walk(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    XAnimTree_s *v4; // eax
    XAnimTree_s *v5; // eax
    XAnimTree_s *v6; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        if ( prevState != ACTOR_ANIMATION_MOVE_RUN )
        {
            Tree = DObjGetTree(obj);
            XAnimClearTreeGoalWeights(Tree, 0x2Du, 0.0, -1);
            v4 = DObjGetTree(obj);
            XAnimClearTreeGoalWeights(v4, 3u, 0.0, -1);
            v5 = DObjGetTree(obj);
            XAnimClearTreeGoalWeights(v5, 0xAu, 0.2, -1);
            v6 = DObjGetTree(obj);
            XAnimClearTreeGoalWeights(v6, 0xBu, 0.2, -1);
            BG_Dog_Clear_Traverse_Anims(localClientNum, es, prevState);
        }
        XAnimSetCompleteGoalWeight(
            obj,
            2u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Move_Walk_Think(int localClientNum, const entityState_s *es)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0x27u, 0.30000001, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            2u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Move_Turn_Left(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    XAnimTree_s *v3; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        v3 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v3, 3u, 0.2, -1);
        BG_Dog_Clear_All_Turns(obj);
        XAnimSetCompleteGoalWeight(
            obj,
            8u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Clear_All_Turns(DObj *obj)
{
    XAnimTree_s *Tree; // eax
    XAnimTree_s *v2; // eax
    XAnimTree_s *v3; // eax
    XAnimTree_s *v4; // eax
    XAnimTree_s *v5; // eax
    XAnimTree_s *v6; // eax
    XAnimTree_s *v7; // eax
    XAnimTree_s *v8; // eax

    Tree = DObjGetTree(obj);
    XAnimClearTreeGoalWeights(Tree, 8u, 0.0, -1);
    v2 = DObjGetTree(obj);
    XAnimClearTreeGoalWeights(v2, 9u, 0.0, -1);
    v3 = DObjGetTree(obj);
    XAnimClearTreeGoalWeights(v3, 6u, 0.0, -1);
    v4 = DObjGetTree(obj);
    XAnimClearTreeGoalWeights(v4, 7u, 0.0, -1);
    v5 = DObjGetTree(obj);
    XAnimClearTreeGoalWeights(v5, 0xCu, 0.0, -1);
    v6 = DObjGetTree(obj);
    XAnimClearTreeGoalWeights(v6, 0xDu, 0.0, -1);
    v7 = DObjGetTree(obj);
    XAnimClearTreeGoalWeights(v7, 0xAu, 0.0, -1);
    v8 = DObjGetTree(obj);
    XAnimClearTreeGoalWeights(v8, 0xBu, 0.0, -1);
}

void __cdecl BG_Dog_Move_Turn_Right(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    XAnimTree_s *v3; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        v3 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v3, 3u, 0.2, -1);
        BG_Dog_Clear_All_Turns(obj);
        XAnimSetCompleteGoalWeight(
            obj,
            9u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Move_Run_Turn_Left(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    XAnimTree_s *v3; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        v3 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v3, 3u, 0.2, -1);
        BG_Dog_Clear_All_Turns(obj);
        XAnimSetCompleteGoalWeight(
            obj,
            6u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Move_Run_Turn_Right(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    XAnimTree_s *v3; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        v3 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v3, 3u, 0.2, -1);
        BG_Dog_Clear_All_Turns(obj);
        XAnimSetCompleteGoalWeight(
            obj,
            7u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Move_Turn_Around_Left(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    XAnimTree_s *v3; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        v3 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v3, 3u, 0.2, -1);
        BG_Dog_Clear_All_Turns(obj);
        XAnimSetCompleteGoalWeight(
            obj,
            0xCu,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Move_Turn_Around_Right(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    XAnimTree_s *v3; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        v3 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v3, 3u, 0.2, -1);
        BG_Dog_Clear_All_Turns(obj);
        XAnimSetCompleteGoalWeight(
            obj,
            0xDu,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Move_Run_Turn_Around_Left(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    XAnimTree_s *v3; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        v3 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v3, 3u, 0.2, -1);
        BG_Dog_Clear_All_Turns(obj);
        XAnimSetCompleteGoalWeight(
            obj,
            0xAu,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Move_Run_Turn_Around_Right(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    XAnimTree_s *v3; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        v3 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v3, 3u, 0.2, -1);
        BG_Dog_Clear_All_Turns(obj);
        XAnimSetCompleteGoalWeight(
            obj,
            0xBu,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Stop_Idle(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        if ( prevState < ACTOR_ANIMATION_STOP_IDLE || prevState > ACTOR_ANIMATION_STOP_ATTACKIDLE_GROWL )
            BG_Dog_Stop_Init(localClientNum, es);
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0x28u, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            1u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Stop_Init(int localClientNum, const entityState_s *es)
{
    XAnimTree_s *Tree; // eax
    XAnimTree_s *v3; // eax
    XAnimTree_s *v4; // eax
    DObj *obj; // [esp+8h] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        v3 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v3, 1u, 0.2, -1);
        v4 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v4, 0x28u, 0.2, -1);
        BG_Dog_Init_Attack_Look_At(localClientNum, es);
    }
}

void __cdecl BG_Dog_Init_Attack_Look_At(int localClientNum, const entityState_s *es)
{
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);

    if ( obj )
    {
        BG_Dog_Clear_Look_At(localClientNum, es);
        XAnimSetGoalWeight(obj, 0x37u, 1.0, 0.0, s_animRate, 0, 0, 0, -1);
        XAnimSetGoalWeight(obj, 0x35u, 1.0, 0.0, s_animRate, 0, 0, 0, -1);
        XAnimSetGoalWeight(obj, 0x3Bu, 1.0, 0.0, s_animRate, 0, 0, 0, -1);
        XAnimSetGoalWeight(obj, 0x39u, 1.0, 0.0, s_animRate, 0, 0, 0, -1);
    }
}

void __cdecl BG_Dog_Stop_AttackIdle(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    XAnimTree_s *v4; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        if ( prevState < ACTOR_ANIMATION_STOP_IDLE || prevState > ACTOR_ANIMATION_STOP_ATTACKIDLE_GROWL )
            BG_Dog_Stop_Init(localClientNum, es);
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 1u, 0.2, -1);
        v4 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v4, 0x28u, 0.1, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x31u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Stop_AttackIdle_Bark(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    XAnimTree_s *v4; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);

    if ( obj )
    {
        if ( prevState < ACTOR_ANIMATION_STOP_IDLE || prevState > ACTOR_ANIMATION_STOP_ATTACKIDLE_GROWL )
            BG_Dog_Stop_Init(localClientNum, es);
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 1u, 0.2, -1);
        v4 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v4, 0x28u, 0.1, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x32u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Stop_AttackIdle_Growl(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    XAnimTree_s *v4; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);

    if ( obj )
    {
        if ( prevState < ACTOR_ANIMATION_STOP_IDLE || prevState > ACTOR_ANIMATION_STOP_ATTACKIDLE_GROWL )
            BG_Dog_Stop_Init(localClientNum, es);
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 1u, 0.2, -1);
        v4 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v4, 0x28u, 0.1, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x33u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Pain_Main(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);

    if ( obj )
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            4u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Pain_Front(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);

    if ( obj )
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x12u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Pain_Back(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x13u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Pain_Left(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x14u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Pain_Right(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);

    if ( obj )
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x15u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Run_Pain_Front(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0xEu,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Run_Pain_Back(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0xFu,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Run_Pain_Left(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x10u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Run_Pain_Right(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if ( obj )
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x11u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Death_Front(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x16u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Death_Back(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x17u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Death_Left(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x18u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Death_Right(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x19u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Death_Fast_Forward_Front(int localClientNum, const entityState_s *es, actorInfo_t *actorInfo)
{
    BG_Dog_FastForwardAnim(0x16u, actorInfo);
}

void __cdecl BG_Dog_FastForwardAnim(unsigned int anim, actorInfo_t *actorInfo)
{
    XAnim_s *anims; // [esp+10h] [ebp-4h]

    anims = XAnimGetAnims(actorInfo->pXAnimTree);
    if ( anim && !XAnimIsLooped(anims, anim) && !XAnimGetNumChildren(anims, anim) )
        XAnimSetTime(actorInfo->pXAnimTree, anim, 1.0, 0xFFFFu);
}

void __cdecl BG_Dog_Death_Fast_Forward_Back(int localClientNum, const entityState_s *es, actorInfo_t *actorInfo)
{
    BG_Dog_FastForwardAnim(0x17u, actorInfo);
}

void __cdecl BG_Dog_Death_Fast_Forward_Left(int localClientNum, const entityState_s *es, actorInfo_t *actorInfo)
{
    BG_Dog_FastForwardAnim(0x18u, actorInfo);
}

void __cdecl BG_Dog_Death_Fast_Forward_Right(int localClientNum, const entityState_s *es, actorInfo_t *actorInfo)
{
    BG_Dog_FastForwardAnim(0x19u, actorInfo);
}

void __cdecl BG_Dog_Flashed(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x1Au,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Combat_AttackIdle(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x31u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Combat_AttackIdle_Bark(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x32u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Combat_AttackIdle_Growl(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x33u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Combat_Attack_Run(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            5u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Combat_Attack_Player_Close_Range(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.1, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x1Eu,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Combat_Attack_Miss(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.1, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x1Bu,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
        BG_Dog_Init_Normal_Look_At(localClientNum, es);
    }
}

void __cdecl BG_Dog_Combat_Attack_Miss_Left(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.1, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x1Cu,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Combat_Attack_Miss_Right(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.1, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x1Du,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Clear_Traverse_Anims(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    XAnimTree_s *v4; // eax
    XAnimTree_s *v5; // eax
    XAnimTree_s *v6; // eax
    XAnimTree_s *v7; // eax
    XAnimTree_s *v8; // eax
    DObj *obj; // [esp+8h] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj
        && (prevState == ACTOR_ANIMATION_TRAVERSE_JUMP_UP_40
            || prevState == ACTOR_ANIMATION_TRAVERSE_JUMP_UP_80
            || prevState == ACTOR_ANIMATION_TRAVERSE_JUMP_DOWN_40
            || prevState == ACTOR_ANIMATION_TRAVERSE_JUMP_DOWN_80
            || prevState == ACTOR_ANIMATION_TRAVERSE_WALLHOP
            || prevState == ACTOR_ANIMATION_TRAVERSE_WINDOW
            || prevState == ACTOR_ANIMATION_TRAVERSE_THROUGH_HOLE_42))
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0x1Fu, 0.30000001, -1);
        v4 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v4, 0x20u, 0.30000001, -1);
        v5 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v5, 0x21u, 0.30000001, -1);
        v6 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v6, 0x22u, 0.30000001, -1);
        v7 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v7, 0x23u, 0.30000001, -1);
        v8 = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(v8, 0x24u, 0.30000001, -1);
    }
}

void __cdecl BG_Dog_Traverse_Jump_Up_40(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x1Fu,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Traverse_Jump_Up_80(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x20u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Traverse_Jump_Down_40(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x21u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Traverse_Jump_Down_80(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x22u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Traverse_Wallhop(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x23u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Traverse_Window(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(
            obj,
            0x24u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            1,
            -1);
    }
}

void __cdecl BG_Dog_Traverse_Through_Hole_42(int localClientNum, const entityState_s *es, ActorAnimStates prevState)
{
    XAnimTree_s *Tree; // eax
    DObj *obj; // [esp+1Ch] [ebp-4h]

    obj = bgs->GetDObj(es->number, localClientNum);
    if (obj)
    {
        Tree = DObjGetTree(obj);
        XAnimClearTreeGoalWeights(Tree, 0, 0.2, -1);
        XAnimSetCompleteGoalWeight(obj, 0x2Eu, 1.0, 0.2, 1.0, 0, 0, 0, -1);
        XAnimSetCompleteGoalWeightKnob(
            obj,
            0x27u,
            1.0,
            0.2,
            s_animRate,
            bgs->animData->done_notify,
            2u,
            0,
            -1);
    }
}
