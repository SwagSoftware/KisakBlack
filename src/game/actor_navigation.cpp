#include "actor_navigation.h"
#include <game_mp/g_main_mp.h>
#include <game_mp/actor_mp.h>
#include "actor_senses.h"
#include <clientscript/cscr_stringlist.h>
#include <qcommon/cm_world.h>
#include <server_mp/sv_main_mp.h>
#include "g_debug.h"
#include <cgame/cg_drawtools.h>

float g_pathAttemptGoalPos[3];

float Path_GetPathDir(float *delta, const float *vFrom, const float *vTo)
{
    float fDist; // [esp+18h] [ebp-4h]

    Vec2Sub(vTo, vFrom, delta);
    
    iassert(!IS_NAN(delta[0]));
    iassert(!IS_NAN(delta[1]));
    iassert(delta[0] || delta[1]);
    
    fDist = Vec2Length(delta);

    iassert(fDist > 0);
    
    delta[0] = (1.0f / fDist) * delta[0];
    delta[1] = (1.0f / fDist) * delta[1];

    return fDist;
}

pathnode_t *__fastcall Path_GetNegotiationNode(const path_t *pPath)
{
    if ( pPath->wNegotiationStartNode <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    86,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode > 0") )
    {
        __debugbreak();
    }
    return Path_ConvertIndexToNode(pPath->pts[pPath->wNegotiationStartNode].iNodeNum);
}

void __fastcall Path_IncrementNodeUserCount(path_t *pPath)
{
    pathnode_t *negotiationNode; // [esp+4h] [ebp-4h]

    negotiationNode = Path_GetNegotiationNode(pPath);
    if ( !negotiationNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    104,
                    0,
                    "%s",
                    "negotiationNode") )
    {
        __debugbreak();
    }
    ++negotiationNode->dynamic.userCount;
}

void __fastcall Path_AddTrimmedAmount(path_t *pPath, const float *vStartPos)
{
    float v[2]; // [esp+24h] [ebp-2Ch] BYREF
    float dist; // [esp+2Ch] [ebp-24h]
    float height; // [esp+30h] [ebp-20h]
    float closestDist; // [esp+34h] [ebp-1Ch]
    float offset[2]; // [esp+38h] [ebp-18h] BYREF
    float amount; // [esp+40h] [ebp-10h]
    float closestAmount; // [esp+44h] [ebp-Ch]
    pathpoint_t *pt; // [esp+48h] [ebp-8h]
    int i; // [esp+4Ch] [ebp-4h]

    i = pPath->wPathLen - 1;
    if ( pPath->wNegotiationStartNode < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    198,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode >= 0") )
    {
        __debugbreak();
    }
    if ( i != -1
        && pPath->wNegotiationStartNode > i
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    199,
                    0,
                    "%s",
                    "i == -1 || pPath->wNegotiationStartNode <= i") )
    {
        __debugbreak();
    }
    if ( i > pPath->wNegotiationStartNode )
    {
        pt = &pPath->pts[i - 1];
        v[0] = *vStartPos - pt->vOrigPoint[0];
        v[1] = vStartPos[1] - pt->vOrigPoint[1];
        closestDist = Vec2Length(v);
        amount = 0.0f;
        closestAmount = 0.0f;
        if ( i >= pPath->wOrigPathLen
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        209,
                        0,
                        "%s",
                        "i < pPath->wOrigPathLen") )
        {
            __debugbreak();
        }
        do
        {
            pt = &pPath->pts[i];
            offset[0] = pt->vOrigPoint[0] - *vStartPos;
            offset[1] = pt->vOrigPoint[1] - vStartPos[1];
            dist = Vec2Length(offset);
            if ( closestDist >= dist )
            {
                closestDist = dist;
                closestAmount = amount;
            }
            if ( ++i >= pPath->wOrigPathLen )
                break;
            if ( pt->fDir2D[0] == 0.0
                && pt->fDir2D[1] == 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            226,
                            0,
                            "%s",
                            "pt->fDir2D[0] || pt->fDir2D[1]") )
            {
                __debugbreak();
            }
            height = fabs((float)(pt->fDir2D[0] * offset[1]) - (float)(pt->fDir2D[1] * offset[0]));
            if ( pt->fOrigLength <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            228,
                            0,
                            "%s",
                            "pt->fOrigLength > 0") )
            {
                __debugbreak();
            }
            amount = (float)(height * pt->fOrigLength) + amount;
        }
        while ( amount <= 21845.334 );
        pt = &pPath->pts[pPath->wPathLen - 2];
        height = Path_GetDistToPathSegment(vStartPos, pt);
        if ( pt->fOrigLength <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        235,
                        0,
                        "%s",
                        "pt->fOrigLength > 0") )
        {
            __debugbreak();
        }
        if ( pPath->fCurrLength <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        236,
                        0,
                        "%s",
                        "pPath->fCurrLength > 0") )
        {
            __debugbreak();
        }
        if ( pt->fOrigLength < pPath->fCurrLength
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        237,
                        0,
                        "pt->fOrigLength >= pPath->fCurrLength\n\t%g, %g",
                        pt->fOrigLength,
                        pPath->fCurrLength) )
        {
            __debugbreak();
        }
        closestAmount = (float)((float)(pt->fOrigLength - pPath->fCurrLength) * height) + closestAmount;
        if ( closestAmount < 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        240,
                        0,
                        "%s\n\t(closestAmount) = %g",
                        "(closestAmount >= 0)",
                        closestAmount) )
        {
            __debugbreak();
        }
        pPath->fLookaheadAmount = pPath->fLookaheadAmount + closestAmount;
    }
}

float Path_GetDistToPathSegment(const float *vStartPos, const pathpoint_t *pt)
{
    float offset[2]; // [esp+10h] [ebp-8h]

    iassert(pt);

    offset[0] = pt->vOrigPoint[0] - vStartPos[0];
    offset[1] = pt->vOrigPoint[1] - vStartPos[1];

    Vec2Sub(pt->vOrigPoint, vStartPos, offset);

    iassert(pt->fDir2D[0] || pt->fDir2D[1]);

    return I_fabs(((pt->fDir2D[0] * offset[1]) - (pt->fDir2D[1] * offset[0])));
}

int __cdecl Path_AStarAlgorithm_CustomSearchInfo_FindCloseNode_(
    path_t *pPath,
    team_t eTeam,
    const float *vStartPos,
    pathnode_t *pNodeFrom,
    const float *vGoalPos,
    int bIncludeGoalInPath,
    int bAllowNegotiationLinks,
    CustomSearchInfo_FindCloseNode *custom,
    int bIgnoreBadPlaces)
{
    double v10; // st7
    float v[7]; // [esp+8h] [ebp-C0h] BYREF
    pathnode_t *pEval; // [esp+24h] [ebp-A4h]
    pathnode_t *pCurrent; // [esp+2Ch] [ebp-9Ch]
    pathnode_t TopParent; // [esp+30h] [ebp-98h] BYREF
    float fApproxTotalCost; // [esp+B4h] [ebp-14h]
    pathnode_t *pInsert; // [esp+B8h] [ebp-10h]
    int i; // [esp+BCh] [ebp-Ch]
    float fCost; // [esp+C0h] [ebp-8h]
    pathnode_t *pSuccessor; // [esp+C4h] [ebp-4h]

    if (!vGoalPos
        && bIncludeGoalInPath
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            675,
            0,
            "%s",
            "vGoalPos || !bIncludeGoalInPath"))
    {
        __debugbreak();
    }
    if ((unsigned int)eTeam >= TEAM_NUM_TEAMS
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            676,
            0,
            "eTeam doesn't index ARRAY_COUNT( ((pathlink_t *) 0)->ubBadPlaceCount )\n\t%i not in [0, %i)",
            eTeam,
            4))
    {
        __debugbreak();
    }
    if (vGoalPos)
    {
        g_pathAttemptGoalPos[0] = *vGoalPos;
        g_pathAttemptGoalPos[1] = vGoalPos[1];
        g_pathAttemptGoalPos[2] = vGoalPos[2];
    }
    else
    {
        g_pathAttemptGoalPos[0] = 0.0f;
        g_pathAttemptGoalPos[1] = 0.0f;
        g_pathAttemptGoalPos[2] = 0.0f;
    }
    pNodeFrom->transient.iSearchFrame = ++level.iSearchFrame;
    pNodeFrom->transient.pParent = &TopParent;
    pNodeFrom->transient.pNextOpen = 0;
    pNodeFrom->transient.pPrevOpen = &TopParent;
    pNodeFrom->transient.fCost = 0.0f;
    TopParent.transient.pNextOpen = pNodeFrom;
LABEL_12:
    if (TopParent.transient.pNextOpen)
    {
        pCurrent = TopParent.transient.pNextOpen;
        TopParent.transient.pNextOpen = TopParent.transient.pNextOpen->transient.pNextOpen;
        if (TopParent.transient.pNextOpen)
            TopParent.transient.pNextOpen->transient.pPrevOpen = &TopParent;
        for (i = 0; ; ++i)
        {
            if (i >= pCurrent->dynamic.wLinkCount)
            {
                pCurrent->transient.pPrevOpen = 0;
                goto LABEL_12;
            }
            if (bIgnoreBadPlaces || !pCurrent->constant.Links[i].ubBadPlaceCount[eTeam])
            {
                pSuccessor = Path_ConvertIndexToNode(pCurrent->constant.Links[i].nodeNum);
                if (pSuccessor == pCurrent
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        725,
                        0,
                        "%s",
                        "pSuccessor != pCurrent"))
                {
                    __debugbreak();
                }
                //if (!CustomSearchInfo_FindCloseNode::IgnoreNode(custom, pSuccessor)
                if (!custom->IgnoreNode(pSuccessor)
                    && (pCurrent->constant.type != NODE_NEGOTIATION_BEGIN
                        || pSuccessor->constant.type != NODE_NEGOTIATION_END
                        || !pCurrent->dynamic.wOverlapCount && !pSuccessor->dynamic.wOverlapCount))
                {
                    if (pSuccessor->transient.iSearchFrame == level.iSearchFrame)
                    {
                        fCost = (float)(pCurrent->constant.Links[i].fDist * 1.0) + pCurrent->transient.fCost;
                        if (fCost >= pSuccessor->transient.fCost)
                            continue;
                        if (pSuccessor->transient.pPrevOpen)
                        {
                            pSuccessor->transient.pPrevOpen->transient.pNextOpen = pSuccessor->transient.pNextOpen;
                            if (pSuccessor->transient.pNextOpen)
                                pSuccessor->transient.pNextOpen->transient.pPrevOpen = pSuccessor->transient.pPrevOpen;
                        }
                    }
                    else
                    {
                        pSuccessor->transient.iSearchFrame = level.iSearchFrame;
                        v[0] = *vGoalPos - pSuccessor->constant.vOrigin[0];
                        v[1] = vGoalPos[1] - pSuccessor->constant.vOrigin[1];
                        v10 = Vec2Length(v);
                        pSuccessor->transient.fHeuristic = v10;
                        fCost = (float)(pCurrent->constant.Links[i].fDist * 1.0) + pCurrent->transient.fCost;
                    }
                    pSuccessor->transient.pParent = pCurrent;
                    pSuccessor->transient.fCost = fCost;
                    fApproxTotalCost = pSuccessor->transient.fCost + pSuccessor->transient.fHeuristic;
                    for (pInsert = &TopParent; pInsert->transient.pNextOpen; pInsert = pEval)
                    {
                        pEval = pInsert->transient.pNextOpen;
                        if ((float)(pEval->transient.fCost + pEval->transient.fHeuristic) >= fApproxTotalCost)
                            break;
                    }
                    if (!pInsert
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            780,
                            0,
                            "%s",
                            "pInsert"))
                    {
                        __debugbreak();
                    }
                    pSuccessor->transient.pPrevOpen = pInsert;
                    pSuccessor->transient.pNextOpen = pInsert->transient.pNextOpen;
                    pInsert->transient.pNextOpen = pSuccessor;
                    if (pSuccessor->transient.pNextOpen)
                        pSuccessor->transient.pNextOpen->transient.pPrevOpen = pSuccessor;
                }
            }
        }
    }
    return 0;
}

int Path_AStarAlgorithm_CustomSearchInfo_FindPath_(
    path_t *pPath,
    team_t eTeam,
    float *vStartPos,
    pathnode_t *pNodeFrom,
    const float *vGoalPos,
    int bIncludeGoalInPath,
    int bAllowNegotiationLinks,
    CustomSearchInfo_FindPath *custom,
    int bIgnoreBadPlaces)
{
    double v11; // st7
    int success; // [esp+28h] [ebp-A0h]
    pathnode_t *pCurrent; // [esp+2Ch] [ebp-9Ch]
    pathnode_t TopParent; // [esp+30h] [ebp-98h] BYREF
    float fApproxTotalCost; // [esp+B4h] [ebp-14h]
    pathnode_t *pInsert; // [esp+B8h] [ebp-10h]
    int i; // [esp+BCh] [ebp-Ch]
    float fCost; // [esp+C0h] [ebp-8h]
    pathnode_t *pSuccessor; // [esp+C4h] [ebp-4h]

    if (!vGoalPos
        && bIncludeGoalInPath
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            675,
            0,
            "%s",
            "vGoalPos || !bIncludeGoalInPath"))
    {
        __debugbreak();
    }
    if ((unsigned int)eTeam >= TEAM_NUM_TEAMS
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            676,
            0,
            "eTeam doesn't index ARRAY_COUNT( ((pathlink_t *) 0)->ubBadPlaceCount )\n\t%i not in [0, %i)",
            eTeam,
            4))
    {
        __debugbreak();
    }
    if (vGoalPos)
    {
        g_pathAttemptGoalPos[0] = *vGoalPos;
        g_pathAttemptGoalPos[1] = vGoalPos[1];
        g_pathAttemptGoalPos[2] = vGoalPos[2];
    }
    else
    {
        g_pathAttemptGoalPos[0] = 0.0f;
        g_pathAttemptGoalPos[1] = 0.0f;
        g_pathAttemptGoalPos[2] = 0.0f;
    }
    pNodeFrom->transient.iSearchFrame = ++level.iSearchFrame;
    pNodeFrom->transient.pParent = &TopParent;
    pNodeFrom->transient.pNextOpen = 0;
    pNodeFrom->transient.pPrevOpen = &TopParent;
    pNodeFrom->transient.fCost = 0.0f;
    TopParent.transient.pNextOpen = pNodeFrom;
LABEL_12:
    if (!TopParent.transient.pNextOpen)
        return 0;
    pCurrent = TopParent.transient.pNextOpen;
    if (TopParent.transient.pNextOpen != custom->m_pNodeTo)
    {
        TopParent.transient.pNextOpen = TopParent.transient.pNextOpen->transient.pNextOpen;
        if (TopParent.transient.pNextOpen)
            TopParent.transient.pNextOpen->transient.pPrevOpen = &TopParent;
        for (i = 0; ; ++i)
        {
            if (i >= pCurrent->dynamic.wLinkCount)
            {
                pCurrent->transient.pPrevOpen = 0;
                goto LABEL_12;
            }
            if (bIgnoreBadPlaces || !pCurrent->constant.Links[i].ubBadPlaceCount[eTeam])
            {
                pSuccessor = Path_ConvertIndexToNode(pCurrent->constant.Links[i].nodeNum);
                if (pSuccessor == pCurrent
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        725,
                        0,
                        "%s",
                        "pSuccessor != pCurrent"))
                {
                    __debugbreak();
                }
                if (IsNodeEnabled(pSuccessor)
                    && (pCurrent->constant.type != NODE_NEGOTIATION_BEGIN
                        || pSuccessor->constant.type != NODE_NEGOTIATION_END
                        || !pCurrent->dynamic.wOverlapCount && !pSuccessor->dynamic.wOverlapCount))
                {
                    if (pSuccessor->transient.iSearchFrame == level.iSearchFrame)
                    {
                        fCost = (float)(pCurrent->constant.Links[i].fDist * 1.0) + pCurrent->transient.fCost;
                        if (fCost >= pSuccessor->transient.fCost)
                            continue;
                        if (pSuccessor->transient.pPrevOpen)
                        {
                            pSuccessor->transient.pPrevOpen->transient.pNextOpen = pSuccessor->transient.pNextOpen;
                            if (pSuccessor->transient.pNextOpen)
                                pSuccessor->transient.pNextOpen->transient.pPrevOpen = pSuccessor->transient.pPrevOpen;
                        }
                    }
                    else
                    {
                        pSuccessor->transient.iSearchFrame = level.iSearchFrame;
                        //v11 = CustomSearchInfo_FindPath::EvaluateHeuristic(custom, pSuccessor, vGoalPos);
                        v11 = custom->EvaluateHeuristic(pSuccessor, vGoalPos);
                        pSuccessor->transient.fHeuristic = v11;
                        fCost = (float)(pCurrent->constant.Links[i].fDist * 1.0) + pCurrent->transient.fCost;
                    }
                    pSuccessor->transient.pParent = pCurrent;
                    pSuccessor->transient.fCost = fCost;
                    fApproxTotalCost = pSuccessor->transient.fCost + pSuccessor->transient.fHeuristic;
                    for (pInsert = &TopParent;
                        pInsert->transient.pNextOpen
                        && (float)(pInsert->transient.pNextOpen->transient.fCost
                            + pInsert->transient.pNextOpen->transient.fHeuristic) < fApproxTotalCost;
                        pInsert = pInsert->transient.pNextOpen)
                    {
                        ;
                    }
                    if (!pInsert
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            780,
                            0,
                            "%s",
                            "pInsert"))
                    {
                        __debugbreak();
                    }
                    pSuccessor->transient.pPrevOpen = pInsert;
                    pSuccessor->transient.pNextOpen = pInsert->transient.pNextOpen;
                    pInsert->transient.pNextOpen = pSuccessor;
                    if (pSuccessor->transient.pNextOpen)
                        pSuccessor->transient.pNextOpen->transient.pPrevOpen = pSuccessor;
                }
            }
        }
    }
    if (!pPath)
        return 1;
    success = Path_GeneratePath(
        pPath,
        eTeam,
        vStartPos,
        vGoalPos,
        pNodeFrom,
        TopParent.transient.pNextOpen,
        bIncludeGoalInPath,
        bAllowNegotiationLinks);
    if (pPath->wPathLen > pPath->wOrigPathLen
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            708,
            0,
            "%s",
            "pPath->wPathLen <= pPath->wOrigPathLen"))
    {
        __debugbreak();
    }
    if (!success
        && pPath->wOrigPathLen
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            709,
            0,
            "%s",
            "success || !pPath->wOrigPathLen"))
    {
        __debugbreak();
    }
    return success;
}

int Path_AStarAlgorithm_CustomSearchInfo_FindPathWithWidth_(
    path_t *pPath,
    team_t eTeam,
    float *vStartPos,
    pathnode_t *pNodeFrom,
    const float *vGoalPos,
    int bIncludeGoalInPath,
    int bAllowNegotiationLinks,
    CustomSearchInfo_FindPathWithWidth *custom,
    int bIgnoreBadPlaces)
{
    double v11; // st7
    int success; // [esp+30h] [ebp-A0h]
    pathnode_t *pCurrent; // [esp+34h] [ebp-9Ch]
    pathnode_t TopParent; // [esp+38h] [ebp-98h] BYREF
    float fApproxTotalCost; // [esp+BCh] [ebp-14h]
    pathnode_t *pInsert; // [esp+C0h] [ebp-10h]
    int i; // [esp+C4h] [ebp-Ch]
    float fCost; // [esp+C8h] [ebp-8h]
    pathnode_t *pSuccessor; // [esp+CCh] [ebp-4h]
    int savedregs; // [esp+D0h] [ebp+0h] BYREF

    if (!vGoalPos
        && bIncludeGoalInPath
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            675,
            0,
            "%s",
            "vGoalPos || !bIncludeGoalInPath"))
    {
        __debugbreak();
    }
    if ((unsigned int)eTeam >= TEAM_NUM_TEAMS
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            676,
            0,
            "eTeam doesn't index ARRAY_COUNT( ((pathlink_t *) 0)->ubBadPlaceCount )\n\t%i not in [0, %i)",
            eTeam,
            4))
    {
        __debugbreak();
    }
    if (vGoalPos)
    {
        g_pathAttemptGoalPos[0] = *vGoalPos;
        g_pathAttemptGoalPos[1] = vGoalPos[1];
        g_pathAttemptGoalPos[2] = vGoalPos[2];
    }
    else
    {
        g_pathAttemptGoalPos[0] = 0.0f;
        g_pathAttemptGoalPos[1] = 0.0f;
        g_pathAttemptGoalPos[2] = 0.0f;
    }
    pNodeFrom->transient.iSearchFrame = ++level.iSearchFrame;
    pNodeFrom->transient.pParent = &TopParent;
    pNodeFrom->transient.pNextOpen = 0;
    pNodeFrom->transient.pPrevOpen = &TopParent;
    pNodeFrom->transient.fCost = 0.0f;
    TopParent.transient.pNextOpen = pNodeFrom;
LABEL_12:
    if (!TopParent.transient.pNextOpen)
        return 0;
    pCurrent = TopParent.transient.pNextOpen;
    if (TopParent.transient.pNextOpen != custom->m_pNodeTo)
    {
        TopParent.transient.pNextOpen = TopParent.transient.pNextOpen->transient.pNextOpen;
        if (TopParent.transient.pNextOpen)
            TopParent.transient.pNextOpen->transient.pPrevOpen = &TopParent;
        for (i = 0; ; ++i)
        {
            if (i >= pCurrent->dynamic.wLinkCount)
            {
                pCurrent->transient.pPrevOpen = 0;
                goto LABEL_12;
            }
            if (bIgnoreBadPlaces || !pCurrent->constant.Links[i].ubBadPlaceCount[eTeam])
            {
                pSuccessor = Path_ConvertIndexToNode(pCurrent->constant.Links[i].nodeNum);
                if (pSuccessor == pCurrent
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        725,
                        0,
                        "%s",
                        "pSuccessor != pCurrent"))
                {
                    __debugbreak();
                }
                if (pCurrent->constant.type != NODE_NEGOTIATION_BEGIN
                    || pSuccessor->constant.type != NODE_NEGOTIATION_END
                    || !pCurrent->dynamic.wOverlapCount && !pSuccessor->dynamic.wOverlapCount)
                {
                    if (pSuccessor->transient.iSearchFrame == level.iSearchFrame)
                    {
                        fCost = (float)(pCurrent->constant.Links[i].fDist * pSuccessor->transient.costFactor)
                            + pCurrent->transient.fCost;
                        if (fCost >= pSuccessor->transient.fCost)
                            continue;
                        if (pSuccessor->transient.pPrevOpen)
                        {
                            pSuccessor->transient.pPrevOpen->transient.pNextOpen = pSuccessor->transient.pNextOpen;
                            if (pSuccessor->transient.pNextOpen)
                                pSuccessor->transient.pNextOpen->transient.pPrevOpen = pSuccessor->transient.pPrevOpen;
                        }
                    }
                    else
                    {
                        pSuccessor->transient.iSearchFrame = level.iSearchFrame;
                        //v11 = CustomSearchInfo_FindPathWithWidth::EvaluateHeuristic(
                        //    custom,
                        //    COERCE_FLOAT(&savedregs),
                        //    pSuccessor,
                        //    vGoalPos);
                        v11 = custom->EvaluateHeuristic(pSuccessor, vGoalPos);
                        pSuccessor->transient.fHeuristic = v11;
                        fCost = (float)(pCurrent->constant.Links[i].fDist * pSuccessor->transient.costFactor)
                            + pCurrent->transient.fCost;
                    }
                    pSuccessor->transient.pParent = pCurrent;
                    pSuccessor->transient.fCost = fCost;
                    fApproxTotalCost = pSuccessor->transient.fCost + pSuccessor->transient.fHeuristic;
                    for (pInsert = &TopParent;
                        pInsert->transient.pNextOpen
                        && (float)(pInsert->transient.pNextOpen->transient.fCost
                            + pInsert->transient.pNextOpen->transient.fHeuristic) < fApproxTotalCost;
                        pInsert = pInsert->transient.pNextOpen)
                    {
                        ;
                    }
                    if (!pInsert
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            780,
                            0,
                            "%s",
                            "pInsert"))
                    {
                        __debugbreak();
                    }
                    pSuccessor->transient.pPrevOpen = pInsert;
                    pSuccessor->transient.pNextOpen = pInsert->transient.pNextOpen;
                    pInsert->transient.pNextOpen = pSuccessor;
                    if (pSuccessor->transient.pNextOpen)
                        pSuccessor->transient.pNextOpen->transient.pPrevOpen = pSuccessor;
                }
            }
        }
    }
    if (!pPath)
        return 1;
    success = Path_GeneratePath(
        pPath,
        eTeam,
        vStartPos,
        vGoalPos,
        pNodeFrom,
        TopParent.transient.pNextOpen,
        bIncludeGoalInPath,
        bAllowNegotiationLinks);
    if (pPath->wPathLen > pPath->wOrigPathLen
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            708,
            0,
            "%s",
            "pPath->wPathLen <= pPath->wOrigPathLen"))
    {
        __debugbreak();
    }
    if (!success
        && pPath->wOrigPathLen
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            709,
            0,
            "%s",
            "success || !pPath->wOrigPathLen"))
    {
        __debugbreak();
    }
    return success;
}

int Path_AStarAlgorithm_CustomSearchInfo_FindPathNotCrossPlanes_(
    path_t *pPath,
    team_t eTeam,
    float *vStartPos,
    pathnode_t *pNodeFrom,
    const float *vGoalPos,
    int bIncludeGoalInPath,
    int bAllowNegotiationLinks,
    CustomSearchInfo_FindPathNotCrossPlanes *custom,
    int bIgnoreBadPlaces)
{
    double v11; // st7
    int success; // [esp+30h] [ebp-A0h]
    pathnode_t *pCurrent; // [esp+34h] [ebp-9Ch]
    pathnode_t TopParent; // [esp+38h] [ebp-98h] BYREF
    float fApproxTotalCost; // [esp+BCh] [ebp-14h]
    pathnode_t *pInsert; // [esp+C0h] [ebp-10h]
    int i; // [esp+C4h] [ebp-Ch]
    float fCost; // [esp+C8h] [ebp-8h]
    pathnode_t *pSuccessor; // [esp+CCh] [ebp-4h]

    if (!vGoalPos
        && bIncludeGoalInPath
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            675,
            0,
            "%s",
            "vGoalPos || !bIncludeGoalInPath"))
    {
        __debugbreak();
    }
    if ((unsigned int)eTeam >= TEAM_NUM_TEAMS
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            676,
            0,
            "eTeam doesn't index ARRAY_COUNT( ((pathlink_t *) 0)->ubBadPlaceCount )\n\t%i not in [0, %i)",
            eTeam,
            4))
    {
        __debugbreak();
    }
    if (vGoalPos)
    {
        g_pathAttemptGoalPos[0] = *vGoalPos;
        g_pathAttemptGoalPos[1] = vGoalPos[1];
        g_pathAttemptGoalPos[2] = vGoalPos[2];
    }
    else
    {
        g_pathAttemptGoalPos[0] = 0.0f;
        g_pathAttemptGoalPos[1] = 0.0f;
        g_pathAttemptGoalPos[2] = 0.0f;
    }
    pNodeFrom->transient.iSearchFrame = ++level.iSearchFrame;
    pNodeFrom->transient.pParent = &TopParent;
    pNodeFrom->transient.pNextOpen = 0;
    pNodeFrom->transient.pPrevOpen = &TopParent;
    pNodeFrom->transient.fCost = 0.0f;
    TopParent.transient.pNextOpen = pNodeFrom;
LABEL_12:
    if (!TopParent.transient.pNextOpen)
        return 0;
    pCurrent = TopParent.transient.pNextOpen;
    if (TopParent.transient.pNextOpen != custom->m_pNodeTo)
    {
        TopParent.transient.pNextOpen = TopParent.transient.pNextOpen->transient.pNextOpen;
        if (TopParent.transient.pNextOpen)
            TopParent.transient.pNextOpen->transient.pPrevOpen = &TopParent;
        for (i = 0; ; ++i)
        {
            if (i >= pCurrent->dynamic.wLinkCount)
            {
                pCurrent->transient.pPrevOpen = 0;
                goto LABEL_12;
            }
            if (bIgnoreBadPlaces || !pCurrent->constant.Links[i].ubBadPlaceCount[eTeam])
            {
                pSuccessor = Path_ConvertIndexToNode(pCurrent->constant.Links[i].nodeNum);
                if (pSuccessor == pCurrent
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        725,
                        0,
                        "%s",
                        "pSuccessor != pCurrent"))
                {
                    __debugbreak();
                }
                //if (!CustomSearchInfo_FindPathNotCrossPlanes::IgnoreNode(custom, pSuccessor)
                if (!custom->IgnoreNode(pSuccessor)
                    && (pCurrent->constant.type != NODE_NEGOTIATION_BEGIN
                        || pSuccessor->constant.type != NODE_NEGOTIATION_END
                        || !pCurrent->dynamic.wOverlapCount && !pSuccessor->dynamic.wOverlapCount))
                {
                    if (pSuccessor->transient.iSearchFrame == level.iSearchFrame)
                    {
                        fCost = (float)(pCurrent->constant.Links[i].fDist * 1.0) + pCurrent->transient.fCost;
                        if (fCost >= pSuccessor->transient.fCost)
                            continue;
                        if (pSuccessor->transient.pPrevOpen)
                        {
                            pSuccessor->transient.pPrevOpen->transient.pNextOpen = pSuccessor->transient.pNextOpen;
                            if (pSuccessor->transient.pNextOpen)
                                pSuccessor->transient.pNextOpen->transient.pPrevOpen = pSuccessor->transient.pPrevOpen;
                        }
                    }
                    else
                    {
                        pSuccessor->transient.iSearchFrame = level.iSearchFrame;
                        //v11 = CustomSearchInfo_FindPath::EvaluateHeuristic(custom, pSuccessor, vGoalPos);
                        v11 = custom->EvaluateHeuristic(pSuccessor, vGoalPos);
                        pSuccessor->transient.fHeuristic = v11;
                        fCost = (float)(pCurrent->constant.Links[i].fDist * 1.0) + pCurrent->transient.fCost;
                    }
                    pSuccessor->transient.pParent = pCurrent;
                    pSuccessor->transient.fCost = fCost;
                    fApproxTotalCost = pSuccessor->transient.fCost + pSuccessor->transient.fHeuristic;
                    for (pInsert = &TopParent;
                        pInsert->transient.pNextOpen
                        && (float)(pInsert->transient.pNextOpen->transient.fCost
                            + pInsert->transient.pNextOpen->transient.fHeuristic) < fApproxTotalCost;
                        pInsert = pInsert->transient.pNextOpen)
                    {
                        ;
                    }
                    if (!pInsert
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            780,
                            0,
                            "%s",
                            "pInsert"))
                    {
                        __debugbreak();
                    }
                    pSuccessor->transient.pPrevOpen = pInsert;
                    pSuccessor->transient.pNextOpen = pInsert->transient.pNextOpen;
                    pInsert->transient.pNextOpen = pSuccessor;
                    if (pSuccessor->transient.pNextOpen)
                        pSuccessor->transient.pNextOpen->transient.pPrevOpen = pSuccessor;
                }
            }
        }
    }
    if (!pPath)
        return 1;
    success = Path_GeneratePath(
        pPath,
        eTeam,
        vStartPos,
        vGoalPos,
        pNodeFrom,
        TopParent.transient.pNextOpen,
        bIncludeGoalInPath,
        bAllowNegotiationLinks);
    if (pPath->wPathLen > pPath->wOrigPathLen
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            708,
            0,
            "%s",
            "pPath->wPathLen <= pPath->wOrigPathLen"))
    {
        __debugbreak();
    }
    if (!success
        && pPath->wOrigPathLen
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            709,
            0,
            "%s",
            "success || !pPath->wOrigPathLen"))
    {
        __debugbreak();
    }
    return success;
}

int Path_AStarAlgorithm_CustomSearchInfo_FindPathInCylinderWithLOS_(
    path_t *pPath,
    team_t eTeam,
    float *vStartPos,
    pathnode_t *pNodeFrom,
    const float *vGoalPos,
    int bIncludeGoalInPath,
    int bAllowNegotiationLinks,
    CustomSearchInfo_FindPathInCylinderWithLOS *custom,
    int bIgnoreBadPlaces)
{
    double v11; // st7
    int success; // [esp+34h] [ebp-A0h]
    pathnode_t *pCurrent; // [esp+38h] [ebp-9Ch]
    pathnode_t TopParent; // [esp+3Ch] [ebp-98h] BYREF
    float fApproxTotalCost; // [esp+C0h] [ebp-14h]
    pathnode_t *pInsert; // [esp+C4h] [ebp-10h]
    int i; // [esp+C8h] [ebp-Ch]
    float fCost; // [esp+CCh] [ebp-8h]
    pathnode_t *pSuccessor; // [esp+D0h] [ebp-4h]

    if (!vGoalPos
        && bIncludeGoalInPath
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            675,
            0,
            "%s",
            "vGoalPos || !bIncludeGoalInPath"))
    {
        __debugbreak();
    }
    if ((unsigned int)eTeam >= TEAM_NUM_TEAMS
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            676,
            0,
            "eTeam doesn't index ARRAY_COUNT( ((pathlink_t *) 0)->ubBadPlaceCount )\n\t%i not in [0, %i)",
            eTeam,
            4))
    {
        __debugbreak();
    }
    if (vGoalPos)
    {
        g_pathAttemptGoalPos[0] = *vGoalPos;
        g_pathAttemptGoalPos[1] = vGoalPos[1];
        g_pathAttemptGoalPos[2] = vGoalPos[2];
    }
    else
    {
        g_pathAttemptGoalPos[0] = 0.0f;
        g_pathAttemptGoalPos[1] = 0.0f;
        g_pathAttemptGoalPos[2] = 0.0f;
    }
    pNodeFrom->transient.iSearchFrame = ++level.iSearchFrame;
    pNodeFrom->transient.pParent = &TopParent;
    pNodeFrom->transient.pNextOpen = 0;
    pNodeFrom->transient.pPrevOpen = &TopParent;
    pNodeFrom->transient.fCost = 0.0f;
    TopParent.transient.pNextOpen = pNodeFrom;
LABEL_12:
    if (!TopParent.transient.pNextOpen)
        return 0;
    pCurrent = TopParent.transient.pNextOpen;
    if (custom->m_fWithinDistSqrd <= Vec3DistanceSq(TopParent.transient.pNextOpen->constant.vOrigin, vGoalPos)
        || !Path_NodesVisible(pCurrent, custom->m_pNodeTo))
    {
        TopParent.transient.pNextOpen = TopParent.transient.pNextOpen->transient.pNextOpen;
        if (TopParent.transient.pNextOpen)
            TopParent.transient.pNextOpen->transient.pPrevOpen = &TopParent;
        for (i = 0; ; ++i)
        {
            if (i >= pCurrent->dynamic.wLinkCount)
            {
                pCurrent->transient.pPrevOpen = 0;
                goto LABEL_12;
            }
            if (bIgnoreBadPlaces || !pCurrent->constant.Links[i].ubBadPlaceCount[eTeam])
            {
                pSuccessor = Path_ConvertIndexToNode(pCurrent->constant.Links[i].nodeNum);
                if (pSuccessor == pCurrent
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        725,
                        0,
                        "%s",
                        "pSuccessor != pCurrent"))
                {
                    __debugbreak();
                }
                if (Actor_PointAtGoal(pSuccessor->constant.vOrigin, custom->goal)
                    && (pCurrent->constant.type != NODE_NEGOTIATION_BEGIN
                        || pSuccessor->constant.type != NODE_NEGOTIATION_END
                        || !pCurrent->dynamic.wOverlapCount && !pSuccessor->dynamic.wOverlapCount))
                {
                    if (pSuccessor->transient.iSearchFrame == level.iSearchFrame)
                    {
                        fCost = (float)(pCurrent->constant.Links[i].fDist * 1.0) + pCurrent->transient.fCost;
                        if (fCost >= pSuccessor->transient.fCost)
                            continue;
                        if (pSuccessor->transient.pPrevOpen)
                        {
                            pSuccessor->transient.pPrevOpen->transient.pNextOpen = pSuccessor->transient.pNextOpen;
                            if (pSuccessor->transient.pNextOpen)
                                pSuccessor->transient.pNextOpen->transient.pPrevOpen = pSuccessor->transient.pPrevOpen;
                        }
                    }
                    else
                    {
                        pSuccessor->transient.iSearchFrame = level.iSearchFrame;
                        //v11 = CustomSearchInfo_FindPathWithLOS::EvaluateHeuristic(custom, pSuccessor, vGoalPos);
                        v11 = custom->EvaluateHeuristic(pSuccessor, vGoalPos);
                        pSuccessor->transient.fHeuristic = v11;
                        fCost = (float)(pCurrent->constant.Links[i].fDist * 1.0) + pCurrent->transient.fCost;
                    }
                    pSuccessor->transient.pParent = pCurrent;
                    pSuccessor->transient.fCost = fCost;
                    fApproxTotalCost = pSuccessor->transient.fCost + pSuccessor->transient.fHeuristic;
                    for (pInsert = &TopParent;
                        pInsert->transient.pNextOpen
                        && (float)(pInsert->transient.pNextOpen->transient.fCost
                            + pInsert->transient.pNextOpen->transient.fHeuristic) < fApproxTotalCost;
                        pInsert = pInsert->transient.pNextOpen)
                    {
                        ;
                    }
                    if (!pInsert
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            780,
                            0,
                            "%s",
                            "pInsert"))
                    {
                        __debugbreak();
                    }
                    pSuccessor->transient.pPrevOpen = pInsert;
                    pSuccessor->transient.pNextOpen = pInsert->transient.pNextOpen;
                    pInsert->transient.pNextOpen = pSuccessor;
                    if (pSuccessor->transient.pNextOpen)
                        pSuccessor->transient.pNextOpen->transient.pPrevOpen = pSuccessor;
                }
            }
        }
    }
    if (!pPath)
        return 1;
    success = Path_GeneratePath(
        pPath,
        eTeam,
        vStartPos,
        vGoalPos,
        pNodeFrom,
        pCurrent,
        bIncludeGoalInPath,
        bAllowNegotiationLinks);
    if (pPath->wPathLen > pPath->wOrigPathLen
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            708,
            0,
            "%s",
            "pPath->wPathLen <= pPath->wOrigPathLen"))
    {
        __debugbreak();
    }
    if (!success
        && pPath->wOrigPathLen
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            709,
            0,
            "%s",
            "success || !pPath->wOrigPathLen"))
    {
        __debugbreak();
    }
    return success;
}

int Path_AStarAlgorithm_CustomSearchInfo_CouldAttack_(
    path_t *pPath,
    team_t eTeam,
    float *vStartPos,
    pathnode_t *pNodeFrom,
    const float *vGoalPos,
    int bIncludeGoalInPath,
    int bAllowNegotiationLinks,
    CustomSearchInfo_CouldAttack *custom,
    int bIgnoreBadPlaces)
{
    char v11; // [esp+3h] [ebp-A5h]
    int success; // [esp+8h] [ebp-A0h]
    pathnode_t *pCurrent; // [esp+Ch] [ebp-9Ch]
    pathnode_t TopParent; // [esp+10h] [ebp-98h] BYREF
    float fApproxTotalCost; // [esp+94h] [ebp-14h]
    pathnode_t *pInsert; // [esp+98h] [ebp-10h]
    int i; // [esp+9Ch] [ebp-Ch]
    float fCost; // [esp+A0h] [ebp-8h]
    pathnode_t *pSuccessor; // [esp+A4h] [ebp-4h]

    if (!vGoalPos
        && bIncludeGoalInPath
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            675,
            0,
            "%s",
            "vGoalPos || !bIncludeGoalInPath"))
    {
        __debugbreak();
    }
    if ((unsigned int)eTeam >= TEAM_NUM_TEAMS
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            676,
            0,
            "eTeam doesn't index ARRAY_COUNT( ((pathlink_t *) 0)->ubBadPlaceCount )\n\t%i not in [0, %i)",
            eTeam,
            4))
    {
        __debugbreak();
    }
    if (vGoalPos)
    {
        g_pathAttemptGoalPos[0] = *vGoalPos;
        g_pathAttemptGoalPos[1] = vGoalPos[1];
        g_pathAttemptGoalPos[2] = vGoalPos[2];
    }
    else
    {
        g_pathAttemptGoalPos[0] = 0.0f;
        g_pathAttemptGoalPos[1] = 0.0f;
        g_pathAttemptGoalPos[2] = 0.0f;
    }
    pNodeFrom->transient.iSearchFrame = ++level.iSearchFrame;
    pNodeFrom->transient.pParent = &TopParent;
    pNodeFrom->transient.pNextOpen = 0;
    pNodeFrom->transient.pPrevOpen = &TopParent;
    pNodeFrom->transient.fCost = 0.0f;
    TopParent.transient.pNextOpen = pNodeFrom;
LABEL_12:
    if (!TopParent.transient.pNextOpen)
        return 0;
    pCurrent = TopParent.transient.pNextOpen;
    if (Path_NodesVisible(TopParent.transient.pNextOpen, custom->m_pNodeTo))
    {
        custom->attackNode = pCurrent;
        v11 = 1;
    }
    else
    {
        v11 = 0;
    }
    if (!v11)
    {
        TopParent.transient.pNextOpen = TopParent.transient.pNextOpen->transient.pNextOpen;
        if (TopParent.transient.pNextOpen)
            TopParent.transient.pNextOpen->transient.pPrevOpen = &TopParent;
        for (i = 0; ; ++i)
        {
            if (i >= pCurrent->dynamic.wLinkCount)
            {
                pCurrent->transient.pPrevOpen = 0;
                goto LABEL_12;
            }
            if (bIgnoreBadPlaces || !pCurrent->constant.Links[i].ubBadPlaceCount[eTeam])
            {
                pSuccessor = Path_ConvertIndexToNode(pCurrent->constant.Links[i].nodeNum);
                if (pSuccessor == pCurrent
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        725,
                        0,
                        "%s",
                        "pSuccessor != pCurrent"))
                {
                    __debugbreak();
                }
                if (pCurrent->constant.type != NODE_NEGOTIATION_BEGIN
                    || pSuccessor->constant.type != NODE_NEGOTIATION_END
                    || !pCurrent->dynamic.wOverlapCount && !pSuccessor->dynamic.wOverlapCount)
                {
                    if (pSuccessor->transient.iSearchFrame == level.iSearchFrame)
                    {
                        fCost = (float)(pCurrent->constant.Links[i].fDist * 1.0) + pCurrent->transient.fCost;
                        if (fCost >= pSuccessor->transient.fCost)
                            continue;
                        if (pSuccessor->transient.pPrevOpen)
                        {
                            pSuccessor->transient.pPrevOpen->transient.pNextOpen = pSuccessor->transient.pNextOpen;
                            if (pSuccessor->transient.pNextOpen)
                                pSuccessor->transient.pNextOpen->transient.pPrevOpen = pSuccessor->transient.pPrevOpen;
                        }
                    }
                    else
                    {
                        pSuccessor->transient.iSearchFrame = level.iSearchFrame;
                        pSuccessor->transient.fHeuristic = 0.0f;
                        fCost = (float)(pCurrent->constant.Links[i].fDist * 1.0) + pCurrent->transient.fCost;
                    }
                    pSuccessor->transient.pParent = pCurrent;
                    pSuccessor->transient.fCost = fCost;
                    fApproxTotalCost = pSuccessor->transient.fCost + pSuccessor->transient.fHeuristic;
                    for (pInsert = &TopParent;
                        pInsert->transient.pNextOpen
                        && (float)(pInsert->transient.pNextOpen->transient.fCost
                            + pInsert->transient.pNextOpen->transient.fHeuristic) < fApproxTotalCost;
                        pInsert = pInsert->transient.pNextOpen)
                    {
                        ;
                    }
                    if (!pInsert
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            780,
                            0,
                            "%s",
                            "pInsert"))
                    {
                        __debugbreak();
                    }
                    pSuccessor->transient.pPrevOpen = pInsert;
                    pSuccessor->transient.pNextOpen = pInsert->transient.pNextOpen;
                    pInsert->transient.pNextOpen = pSuccessor;
                    if (pSuccessor->transient.pNextOpen)
                        pSuccessor->transient.pNextOpen->transient.pPrevOpen = pSuccessor;
                }
            }
        }
    }
    if (!pPath)
        return 1;
    success = Path_GeneratePath(
        pPath,
        eTeam,
        vStartPos,
        vGoalPos,
        pNodeFrom,
        pCurrent,
        bIncludeGoalInPath,
        bAllowNegotiationLinks);
    if (pPath->wPathLen > pPath->wOrigPathLen
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            708,
            0,
            "%s",
            "pPath->wPathLen <= pPath->wOrigPathLen"))
    {
        __debugbreak();
    }
    if (!success
        && pPath->wOrigPathLen
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            709,
            0,
            "%s",
            "success || !pPath->wOrigPathLen"))
    {
        __debugbreak();
    }
    return success;
}

int Path_AStarAlgorithm_CustomSearchInfo_FindPathClosestPossible_(
    path_t *pPath,
    team_t eTeam,
    float *vStartPos,
    pathnode_t *pNodeFrom,
    const float *vGoalPos,
    int bIncludeGoalInPath,
    int bAllowNegotiationLinks,
    CustomSearchInfo_FindPathClosestPossible *custom,
    int bIgnoreBadPlaces)
{
    double v11; // st7
    char v12; // [esp+17h] [ebp-B5h]
    float v13; // [esp+24h] [ebp-A8h]
    int success; // [esp+2Ch] [ebp-A0h]
    pathnode_t *pCurrent; // [esp+30h] [ebp-9Ch]
    pathnode_t TopParent; // [esp+34h] [ebp-98h] BYREF
    float fApproxTotalCost; // [esp+B8h] [ebp-14h]
    pathnode_t *pInsert; // [esp+BCh] [ebp-10h]
    int i; // [esp+C0h] [ebp-Ch]
    float fCost; // [esp+C4h] [ebp-8h]
    pathnode_t *pSuccessor; // [esp+C8h] [ebp-4h]

    if (!vGoalPos
        && bIncludeGoalInPath
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            675,
            0,
            "%s",
            "vGoalPos || !bIncludeGoalInPath"))
    {
        __debugbreak();
    }
    if ((unsigned int)eTeam >= TEAM_NUM_TEAMS
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            676,
            0,
            "eTeam doesn't index ARRAY_COUNT( ((pathlink_t *) 0)->ubBadPlaceCount )\n\t%i not in [0, %i)",
            eTeam,
            4))
    {
        __debugbreak();
    }
    if (vGoalPos)
    {
        g_pathAttemptGoalPos[0] = *vGoalPos;
        g_pathAttemptGoalPos[1] = vGoalPos[1];
        g_pathAttemptGoalPos[2] = vGoalPos[2];
    }
    else
    {
        g_pathAttemptGoalPos[0] = 0.0f;
        g_pathAttemptGoalPos[1] = 0.0f;
        g_pathAttemptGoalPos[2] = 0.0f;
    }
    pNodeFrom->transient.iSearchFrame = ++level.iSearchFrame;
    pNodeFrom->transient.pParent = &TopParent;
    pNodeFrom->transient.pNextOpen = 0;
    pNodeFrom->transient.pPrevOpen = &TopParent;
    pNodeFrom->transient.fCost = 0.0f;
    TopParent.transient.pNextOpen = pNodeFrom;
LABEL_12:
    if (!TopParent.transient.pNextOpen)
        return 0;
    pCurrent = TopParent.transient.pNextOpen;
    if (TopParent.transient.pNextOpen == custom->m_pNodeTo)
    {
        custom->m_pBestNode = custom->m_pNodeTo;
        v12 = 1;
    }
    else
    {
        v13 = Vec3DistanceSq(custom->m_pNodeTo->constant.vOrigin, TopParent.transient.pNextOpen->constant.vOrigin);
        if (custom->m_fBestScore > v13)
        {
            custom->m_fBestScore = v13;
            custom->m_pBestNode = pCurrent;
        }
        v12 = 0;
    }
    if (!v12)
    {
        TopParent.transient.pNextOpen = TopParent.transient.pNextOpen->transient.pNextOpen;
        if (TopParent.transient.pNextOpen)
            TopParent.transient.pNextOpen->transient.pPrevOpen = &TopParent;
        for (i = 0; ; ++i)
        {
            if (i >= pCurrent->dynamic.wLinkCount)
            {
                pCurrent->transient.pPrevOpen = 0;
                goto LABEL_12;
            }
            if (bIgnoreBadPlaces || !pCurrent->constant.Links[i].ubBadPlaceCount[eTeam])
            {
                pSuccessor = Path_ConvertIndexToNode(pCurrent->constant.Links[i].nodeNum);
                if (pSuccessor == pCurrent
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        725,
                        0,
                        "%s",
                        "pSuccessor != pCurrent"))
                {
                    __debugbreak();
                }
                if (pCurrent->constant.type != NODE_NEGOTIATION_BEGIN
                    || pSuccessor->constant.type != NODE_NEGOTIATION_END
                    || !pCurrent->dynamic.wOverlapCount && !pSuccessor->dynamic.wOverlapCount)
                {
                    if (pSuccessor->transient.iSearchFrame == level.iSearchFrame)
                    {
                        fCost = (float)(pCurrent->constant.Links[i].fDist * 1.0) + pCurrent->transient.fCost;
                        if (fCost >= pSuccessor->transient.fCost)
                            continue;
                        if (pSuccessor->transient.pPrevOpen)
                        {
                            pSuccessor->transient.pPrevOpen->transient.pNextOpen = pSuccessor->transient.pNextOpen;
                            if (pSuccessor->transient.pNextOpen)
                                pSuccessor->transient.pNextOpen->transient.pPrevOpen = pSuccessor->transient.pPrevOpen;
                        }
                    }
                    else
                    {
                        pSuccessor->transient.iSearchFrame = level.iSearchFrame;
                        //v11 = CustomSearchInfo_FindPathClosestPossible::EvaluateHeuristic(custom, pSuccessor, vGoalPos);
                        v11 = custom->EvaluateHeuristic(pSuccessor, vGoalPos);
                        pSuccessor->transient.fHeuristic = v11;
                        fCost = (float)(pCurrent->constant.Links[i].fDist * 1.0) + pCurrent->transient.fCost;
                    }
                    pSuccessor->transient.pParent = pCurrent;
                    pSuccessor->transient.fCost = fCost;
                    fApproxTotalCost = pSuccessor->transient.fCost + pSuccessor->transient.fHeuristic;
                    for (pInsert = &TopParent;
                        pInsert->transient.pNextOpen
                        && (float)(pInsert->transient.pNextOpen->transient.fCost
                            + pInsert->transient.pNextOpen->transient.fHeuristic) < fApproxTotalCost;
                        pInsert = pInsert->transient.pNextOpen)
                    {
                        ;
                    }
                    if (!pInsert
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            780,
                            0,
                            "%s",
                            "pInsert"))
                    {
                        __debugbreak();
                    }
                    pSuccessor->transient.pPrevOpen = pInsert;
                    pSuccessor->transient.pNextOpen = pInsert->transient.pNextOpen;
                    pInsert->transient.pNextOpen = pSuccessor;
                    if (pSuccessor->transient.pNextOpen)
                        pSuccessor->transient.pNextOpen->transient.pPrevOpen = pSuccessor;
                }
            }
        }
    }
    if (!pPath)
        return 1;
    success = Path_GeneratePath(
        pPath,
        eTeam,
        vStartPos,
        vGoalPos,
        pNodeFrom,
        pCurrent,
        bIncludeGoalInPath,
        bAllowNegotiationLinks);
    if (pPath->wPathLen > pPath->wOrigPathLen
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            708,
            0,
            "%s",
            "pPath->wPathLen <= pPath->wOrigPathLen"))
    {
        __debugbreak();
    }
    if (!success
        && pPath->wOrigPathLen
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
            709,
            0,
            "%s",
            "success || !pPath->wOrigPathLen"))
    {
        __debugbreak();
    }
    return success;
}

pathnode_t *__fastcall Path_FindCloseNode(
                team_t eTeam,
                pathnode_t *pNodeFrom,
                const float *vGoalPos,
                int bAllowNegotiationLinks)
{
    float v7; // [esp+10h] [ebp-18h]
    CustomSearchInfo_FindCloseNode info; // [esp+14h] [ebp-14h] BYREF

    if ( !pNodeFrom
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 854, 0, "%s", "pNodeFrom") )
    {
        __debugbreak();
    }
    if ( (pNodeFrom->constant.spawnflags & 1) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    855,
                    0,
                    "%s",
                    "(pNodeFrom->constant.spawnflags & PNF_DONTLINK) == 0") )
    {
        __debugbreak();
    }
    if ( !vGoalPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 856, 0, "%s", "vGoalPos") )
    {
        __debugbreak();
    }
    info.goalPos[0] = *vGoalPos;
    info.goalPos[1] = vGoalPos[1];
    info.goalPos[2] = vGoalPos[2];
    info.closestNode = pNodeFrom;
    v7 = vGoalPos[1] - pNodeFrom->constant.vOrigin[1];
    info.closestDistSq = (float)((float)(*vGoalPos - pNodeFrom->constant.vOrigin[0])
                                                         * (float)(*vGoalPos - pNodeFrom->constant.vOrigin[0]))
                                         + (float)(v7 * v7);
    Path_AStarAlgorithm_CustomSearchInfo_FindCloseNode_(
        0,
        eTeam,
        pNodeFrom->constant.vOrigin,
        pNodeFrom,
        vGoalPos,
        1,
        bAllowNegotiationLinks,
        &info,
        0);
    return info.closestNode;
}

int __fastcall Path_FindPath(
                path_t *pPath,
                team_t eTeam,
                const float *vStartPos,
                const float *vGoalPos,
                int bAllowNegotiationLinks)
{
    pathsort_t nodes[64]; // [esp+18h] [ebp-310h] BYREF
    pathnode_t *pNodeTo; // [esp+31Ch] [ebp-Ch]
    int nodeCount; // [esp+320h] [ebp-8h] BYREF
    pathnode_t *pNodeFrom; // [esp+324h] [ebp-4h]

    pNodeTo = Path_NearestNode(vGoalPos, nodes, -2, 192.0, &nodeCount, 64, NEAREST_NODE_DO_HEIGHT_CHECK);
    if ( !pNodeTo )
        return 0;
    pNodeFrom = Path_NearestNode(vStartPos, nodes, -2, 192.0, &nodeCount, 64, NEAREST_NODE_DO_HEIGHT_CHECK);
    if ( pNodeFrom )
        return Path_FindPathFromTo(pPath, eTeam, pNodeFrom, vStartPos, pNodeTo, vGoalPos, bAllowNegotiationLinks, 0);
    else
        return 0;
}

int __fastcall Path_FindPathFrom(
                path_t *pPath,
                team_t eTeam,
                pathnode_t *pNodeFrom,
                const float *vStartPos,
                const float *vGoalPos,
                int bAllowNegotiationLinks,
                int bIgnoreBadplaces)
{
    pathsort_t nodes[64]; // [esp+18h] [ebp-308h] BYREF
    pathnode_t *pNodeTo; // [esp+318h] [ebp-8h]
    int nodeCount; // [esp+31Ch] [ebp-4h] BYREF

    pNodeTo = Path_NearestNode(vGoalPos, nodes, -2, 192.0, &nodeCount, 64, NEAREST_NODE_DO_HEIGHT_CHECK);
    if ( pNodeTo )
        return Path_FindPathFromTo(
                         pPath,
                         eTeam,
                         pNodeFrom,
                         vStartPos,
                         pNodeTo,
                         vGoalPos,
                         bAllowNegotiationLinks,
                         bIgnoreBadplaces);
    else
        return 0;
}

int __fastcall Path_FindPathFromTo(
                path_t *pPath,
                team_t eTeam,
                pathnode_t *pNodeFrom,
                const float *vStartPos,
                pathnode_t *pNodeTo,
                const float *vGoalPos,
                int bAllowNegotiationLinks,
                int bIgnoreBadplaces)
{
    CustomSearchInfo_FindPath info; // [esp+B0h] [ebp-14h] BYREF

    LODWORD(info.negotiationOverlapCost) = ai_pathNegotiationOverlapCost->current.integer;
    if ( !pNodeFrom
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 992, 0, "%s", "pNodeFrom") )
    {
        __debugbreak();
    }
    if ( !pNodeTo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 993, 0, "%s", "pNodeTo") )
    {
        __debugbreak();
    }
    if ( (pNodeFrom->constant.spawnflags & 1) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    994,
                    0,
                    "%s",
                    "(pNodeFrom->constant.spawnflags & PNF_DONTLINK) == 0") )
    {
        __debugbreak();
    }
    if ( (pNodeTo->constant.spawnflags & 1) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    995,
                    0,
                    "%s",
                    "(pNodeTo->constant.spawnflags & PNF_DONTLINK) == 0") )
    {
        __debugbreak();
    }
    info.m_pNodeTo = pNodeTo;
    info.startPos[0] = *vStartPos;
    info.startPos[1] = vStartPos[1];
    info.startPos[2] = vStartPos[2];
    return Path_AStarAlgorithm_CustomSearchInfo_FindPath_(
                     pPath,
                     eTeam,
                     (float*)vStartPos,
                     pNodeFrom,
                     vGoalPos,
                     1,
                     bAllowNegotiationLinks,
                     &info,
                     bIgnoreBadplaces);
}

int __fastcall Path_FindPathWithWidth(
                path_t *pPath,
                team_t eTeam,
                const float *vStartPos,
                const float *vGoalPos,
                int bAllowNegotiationLinks,
                float width,
                float *perp)
{
    pathsort_t nodes[64]; // [esp+18h] [ebp-310h] BYREF
    pathnode_t *pNodeTo; // [esp+31Ch] [ebp-Ch]
    int nodeCount; // [esp+320h] [ebp-8h] BYREF
    pathnode_t *pNodeFrom; // [esp+324h] [ebp-4h]

    pNodeTo = Path_NearestNode(vGoalPos, nodes, -2, 192.0, &nodeCount, 64, NEAREST_NODE_DO_HEIGHT_CHECK);
    if ( !pNodeTo )
        return 0;
    pNodeFrom = Path_NearestNode(vStartPos, nodes, -2, 192.0, &nodeCount, 64, NEAREST_NODE_DO_HEIGHT_CHECK);
    if ( pNodeFrom )
        return Path_FindPathFromToWithWidth(
                         pPath,
                         eTeam,
                         pNodeFrom,
                         vStartPos,
                         pNodeTo,
                         vGoalPos,
                         bAllowNegotiationLinks,
                         width,
                         perp);
    else
        return 0;
}

int __fastcall Path_FindPathFromToWithWidth(
                path_t *pPath,
                team_t eTeam,
                pathnode_t *pNodeFrom,
                const float *vStartPos,
                pathnode_t *pNodeTo,
                const float *vGoalPos,
                int bAllowNegotiationLinks,
                float width,
                float *perp)
{
    CustomSearchInfo_FindPathWithWidth info; // [esp+B4h] [ebp-10h] BYREF

    if ( !pNodeFrom
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 1094, 0, "%s", "pNodeFrom") )
    {
        __debugbreak();
    }
    if ( !pNodeTo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 1095, 0, "%s", "pNodeTo") )
    {
        __debugbreak();
    }
    if ( (pNodeFrom->constant.spawnflags & 1) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    1096,
                    0,
                    "%s",
                    "(pNodeFrom->constant.spawnflags & PNF_DONTLINK) == 0") )
    {
        __debugbreak();
    }
    if ( (pNodeTo->constant.spawnflags & 1) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    1097,
                    0,
                    "%s",
                    "(pNodeTo->constant.spawnflags & PNF_DONTLINK) == 0") )
    {
        __debugbreak();
    }
    info.m_pNodeTo = pNodeTo;
    info.width = width;
    info.perp[0] = *perp;
    info.perp[1] = perp[1];
    pNodeFrom->transient.costFactor = 0.0f;
    return Path_AStarAlgorithm_CustomSearchInfo_FindPathWithWidth_(
                     pPath,
                     eTeam,
                     (float*)vStartPos,
                     pNodeFrom,
                     vGoalPos,
                     1,
                     bAllowNegotiationLinks,
                     &info,
                     0);
}

int __fastcall Path_FindPathFromNotCrossPlanes(
                path_t *pPath,
                team_t eTeam,
                pathnode_t *pNodeFrom,
                const float *vStartPos,
                const float *vGoalPos,
                float (*vNormal)[2],
                float *fDist,
                int iPlaneCount,
                int bAllowNegotiationLinks,
                int bIgnoreBadplaces)
{
    pathsort_t nodes[64]; // [esp+24h] [ebp-308h] BYREF
    pathnode_t *pNodeTo; // [esp+324h] [ebp-8h]
    int nodeCount; // [esp+328h] [ebp-4h] BYREF

    pNodeTo = Path_NearestNodeNotCrossPlanes(
                            vGoalPos,
                            nodes,
                            -2,
                            192.0,
                            vNormal,
                            fDist,
                            iPlaneCount,
                            &nodeCount,
                            64,
                            NEAREST_NODE_DO_HEIGHT_CHECK);
    if ( pNodeTo )
        return Path_FindPathFromToNotCrossPlanes(
                         pPath,
                         eTeam,
                         pNodeFrom,
                         vStartPos,
                         pNodeTo,
                         vGoalPos,
                         vNormal,
                         fDist,
                         iPlaneCount,
                         bAllowNegotiationLinks,
                         bIgnoreBadplaces);
    else
        return 0;
}

int __fastcall Path_FindPathFromToNotCrossPlanes(
                path_t *pPath,
                team_t eTeam,
                pathnode_t *pNodeFrom,
                const float *vStartPos,
                pathnode_t *pNodeTo,
                const float *vGoalPos,
                float (*vNormal)[2],
                float *fDist,
                int iPlaneCount,
                int bAllowNegotiationLinks,
                int bIgnoreBadplaces)
{
    CustomSearchInfo_FindPathNotCrossPlanes info; // [esp+24h] [ebp-20h] BYREF

    LODWORD(info.negotiationOverlapCost) = ai_pathNegotiationOverlapCost->current.integer;
    if ( !pNodeFrom
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 1207, 0, "%s", "pNodeFrom") )
    {
        __debugbreak();
    }
    if ( !pNodeTo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 1208, 0, "%s", "pNodeTo") )
    {
        __debugbreak();
    }
    if ( (pNodeFrom->constant.spawnflags & 1) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    1209,
                    0,
                    "%s",
                    "(pNodeFrom->constant.spawnflags & PNF_DONTLINK) == 0") )
    {
        __debugbreak();
    }
    if ( (pNodeTo->constant.spawnflags & 1) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    1210,
                    0,
                    "%s",
                    "(pNodeTo->constant.spawnflags & PNF_DONTLINK) == 0") )
    {
        __debugbreak();
    }
    info.m_pNodeTo = pNodeTo;
    info.m_iPlaneCount = iPlaneCount;
    info.m_vNormal = vNormal;
    info.m_fDist = fDist;
    info.startPos[0] = *vStartPos;
    info.startPos[1] = vStartPos[1];
    info.startPos[2] = vStartPos[2];
    //if ( CustomSearchInfo_FindPathNotCrossPlanes::IgnoreNode(&info, pNodeFrom) || CustomSearchInfo_FindPathNotCrossPlanes::IgnoreNode(&info, pNodeTo) )
    if ( info.IgnoreNode(pNodeFrom) || info.IgnoreNode(pNodeTo) )
    {
        return 0;
    }
    else
    {
        return Path_AStarAlgorithm_CustomSearchInfo_FindPathNotCrossPlanes_(
                         pPath,
                         eTeam,
                         (float*)vStartPos,
                         pNodeFrom,
                         vGoalPos,
                         1,
                         bAllowNegotiationLinks,
                         &info,
                         bIgnoreBadplaces);
    }
}

char __thiscall CustomSearchInfo_FindPathNotCrossPlanes::IgnoreNode(pathnode_t *pNode)
{
    int i; // [esp+Ch] [ebp-4h]

    for ( i = 0; i < this->m_iPlaneCount; ++i )
    {
        if ( (float)((float)(pNode->constant.vOrigin[0] * this->m_vNormal[i][0])
                             + (float)(pNode->constant.vOrigin[1] * this->m_vNormal[i][1])) > this->m_fDist[i] )
            return 1;
    }
    return 0;
}

int __fastcall Path_GeneratePath(
                path_t *pPath,
                team_t eTeam,
                const float *vStartPos,
                const float *vGoalPos,
                pathnode_t *pNodeFrom,
                pathnode_t *pNodeTo,
                int bIncludeGoalPos,
                int bAllowNegotiationLinks)
{
    const char *v9; // eax
    float fOrigLength; // [esp+10h] [ebp-68h]
    int v12; // [esp+18h] [ebp-60h]
    pathpoint_t *v16; // [esp+30h] [ebp-48h]
    pathpoint_t *v17; // [esp+38h] [ebp-40h]
    int excess; // [esp+54h] [ebp-24h]
    pathnode_t *pPrevNode; // [esp+58h] [ebp-20h]
    int iTotal; // [esp+5Ch] [ebp-1Ch]
    pathnode_t *pNode; // [esp+64h] [ebp-14h]
    pathnode_t *pNodea; // [esp+64h] [ebp-14h]
    int negotiationStartNode; // [esp+68h] [ebp-10h]
    int i; // [esp+6Ch] [ebp-Ch]
    int ia; // [esp+6Ch] [ebp-Ch]
    pathnode_t *pPrevPrevNode; // [esp+70h] [ebp-8h]
    int prevFlags; // [esp+74h] [ebp-4h]

    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 478, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( !pNodeFrom
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 479, 0, "%s", "pNodeFrom") )
    {
        __debugbreak();
    }
    if ( !pNodeTo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 480, 0, "%s", "pNodeTo") )
    {
        __debugbreak();
    }
    if ( !vGoalPos
        && bIncludeGoalPos
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    481,
                    0,
                    "%s",
                    "vGoalPos || !bIncludeGoalPos") )
    {
        __debugbreak();
    }
    Path_AddTrimmedAmount(pPath, vStartPos);
    iTotal = 0;
    pNode = pNodeTo;
    if ( bIncludeGoalPos )
    {
        if ( !vGoalPos
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 495, 0, "%s", "vGoalPos") )
        {
            __debugbreak();
        }
        pPath->pts[0].vOrigPoint[0] = *vGoalPos;
        pPath->pts[0].vOrigPoint[1] = vGoalPos[1];
        pPath->pts[0].vOrigPoint[2] = vGoalPos[2];
        if ( *vGoalPos == pNodeTo->constant.vOrigin[0] && vGoalPos[1] == pNodeTo->constant.vOrigin[1] )
        {
            pPath->pts[0].iNodeNum = Path_ConvertNodeToIndex(pNodeTo);
        }
        else
        {
            pPath->pts[0].iNodeNum = -1;
            iTotal = 1;
        }
    }
    else
    {
        pPath->pts[0].vOrigPoint[0] = pNodeTo->constant.vOrigin[0];
        pPath->pts[0].vOrigPoint[1] = pNodeTo->constant.vOrigin[1];
        pPath->pts[0].vOrigPoint[2] = pNodeTo->constant.vOrigin[2];
        pPath->pts[0].iNodeNum = Path_ConvertNodeToIndex(pNodeTo);
    }
    prevFlags = pPath->flags;
    pPath->flags = 0;
    if ( !pPath->wPathLen
        || (pPath->pts[0].vOrigPoint[0] != pPath->vFinalGoal[0]
         || pPath->pts[0].vOrigPoint[1] != pPath->vFinalGoal[1]
         || pPath->pts[0].vOrigPoint[2] != pPath->vFinalGoal[2]
            ? (v12 = 0)
            : (v12 = 1),
                !v12) )
    {
        if ( pPath->pts[0].vOrigPoint[0] != pPath->vFinalGoal[0] || pPath->pts[0].vOrigPoint[1] != pPath->vFinalGoal[1] )
            pPath->iPathEndTime = 0;
        pPath->vFinalGoal[0] = pPath->pts[0].vOrigPoint[0];
        pPath->vFinalGoal[1] = pPath->pts[0].vOrigPoint[1];
        pPath->vFinalGoal[2] = pPath->pts[0].vOrigPoint[2];
    }
    Path_Clear(pPath);
    pPath->wDodgeCount = 0;
    negotiationStartNode = 0;
    if ( pNodeTo )
    {
        for ( pPrevNode = pNodeTo->transient.pParent; ; pPrevNode = pPrevPrevNode )
        {
            ++iTotal;
            if ( pNode == pNodeFrom )
                break;
            if ( pPrevNode->constant.type == NODE_NEGOTIATION_BEGIN
                && pNode->constant.type == NODE_NEGOTIATION_END
                && pPrevNode->constant.target == pNode->constant.targetname )
            {
                if ( !bAllowNegotiationLinks )
                    return 0;
                negotiationStartNode = iTotal;
            }
            pPrevPrevNode = pPrevNode->transient.pParent;
            pPrevNode->transient.pParent = pNode;
            pNode = pPrevNode;
        }
    }
    if ( iTotal <= 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 564, 0, "%s", "iTotal > 0") )
    {
        __debugbreak();
    }
    excess = iTotal - 32;
    if ( iTotal - 32 > 0 )
    {
        iTotal = 32;
        pPath->flags |= 4u;
        negotiationStartNode -= excess;
        if ( negotiationStartNode < 0 )
            LOWORD(negotiationStartNode) = 0;
    }
    if ( iTotal <= 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 576, 0, "%s", "iTotal > 0") )
    {
        __debugbreak();
    }
    i = iTotal - 1;
    pNodea = pNodeFrom;
    while ( i > 0 )
    {
        v17 = &pPath->pts[i];
        v17->vOrigPoint[0] = pNodea->constant.vOrigin[0];
        v17->vOrigPoint[1] = pNodea->constant.vOrigin[1];
        v17->vOrigPoint[2] = pNodea->constant.vOrigin[2];
        v17->iNodeNum = Path_ConvertNodeToIndex(pNodea);
        --i;
        pNodea = pNodea->transient.pParent;
    }
    if ( excess > 0 )
    {
        pPath->pts[0].vOrigPoint[0] = pNodea->constant.vOrigin[0];
        pPath->pts[0].vOrigPoint[1] = pNodea->constant.vOrigin[1];
        pPath->pts[0].vOrigPoint[2] = pNodea->constant.vOrigin[2];
        pPath->pts[0].iNodeNum = Path_ConvertNodeToIndex(pNodea);
    }
    if ( pPath->wNegotiationStartNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    590,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode == 0") )
    {
        __debugbreak();
    }
    pPath->wNegotiationStartNode = negotiationStartNode;
    if ( pPath->wNegotiationStartNode < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    592,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode >= 0") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode > 0 )
        Path_IncrementNodeUserCount(pPath);
    pPath->pts[iTotal - 1].fOrigLength = 0.0f;
    pPath->pts[iTotal - 1].fDir2D[0] = 0.0f;
    pPath->pts[iTotal - 1].fDir2D[1] = 0.0f;
    v16 = &pPath->pts[iTotal - 1];
    pPath->vCurrPoint[0] = v16->vOrigPoint[0];
    pPath->vCurrPoint[1] = v16->vOrigPoint[1];
    pPath->vCurrPoint[2] = v16->vOrigPoint[2];
    for ( ia = 0; ia < iTotal - 1; ++ia )
        pPath->pts[ia].fOrigLength = Path_GetPathDir(
                                                                     pPath->pts[ia].fDir2D,
                                                                     pPath->pts[ia + 1].vOrigPoint,
                                                                     pPath->pts[ia].vOrigPoint);
    if ( iTotal <= 1 )
        fOrigLength = 0.0f;
    else
        fOrigLength = pPath->pts[iTotal - 2].fOrigLength;
    pPath->fCurrLength = fOrigLength;
    if ( iTotal <= 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 609, 0, "%s", "iTotal > 0") )
    {
        __debugbreak();
    }
    if ( iTotal > 32
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    610,
                    0,
                    "%s",
                    "iTotal <= PATH_MAX_POINTS") )
    {
        __debugbreak();
    }
    pPath->wPathLen = iTotal;
    pPath->wOrigPathLen = pPath->wPathLen;
    if ( bAllowNegotiationLinks )
        pPath->flags |= 0x10u;
    pPath->eTeam = eTeam;
    pPath->iPathTime = level.time;
    pPath->numReductions = 0;
    pPath->numIncreases = 0;
    if ( pPath->fLookaheadAmount == 0.0 )
        return 1;
    if ( (prevFlags & 0x180) != 0 )
    {
        if ( (prevFlags & 0x80) != 0 )
        {
            pPath->fLookaheadAmount = 21845.334f;
            pPath->minLookAheadNodes = 0;
        }
        else
        {
            pPath->fLookaheadAmount = 4096.0f;
            pPath->minLookAheadNodes = 2;
        }
        pPath->lookaheadDir[0] = 0.0f;
        pPath->lookaheadDir[1] = 0.0f;
        pPath->lookaheadDir[2] = 0.0f;
        Path_UpdateLookahead(pPath, vStartPos, 0, 0, 1, 0);
        pPath->minLookAheadNodes = 0;
    }
    else
    {
        Path_TransferLookahead(pPath, vStartPos);
    }
    if ( pPath->wNegotiationStartNode > pPath->lookaheadNextNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    646,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode") )
    {
        __debugbreak();
    }
    if ( pPath->lookaheadNextNode >= pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    647,
                    0,
                    "%s",
                    "pPath->lookaheadNextNode < pPath->wPathLen") )
    {
        __debugbreak();
    }
    if ( pPath->pts[pPath->lookaheadNextNode].fOrigLength < pPath->fLookaheadDistToNextNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    648,
                    0,
                    "%s",
                    "pPath->fLookaheadDistToNextNode <= pPath->pts[pPath->lookaheadNextNode].fOrigLength") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen > 1 && pPath->pts[pPath->wPathLen - 2].fOrigLength < pPath->fCurrLength )
    {
        v9 = va(
                     "pPath->fCurrLength: %g, pPath->pts[pPath->wPathLen - 2].fOrigLength: %g",
                     pPath->fCurrLength,
                     pPath->pts[pPath->wPathLen - 2].fOrigLength);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        649,
                        0,
                        "%s\n\t%s",
                        "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                        v9) )
            __debugbreak();
    }
    if ( pPath->wNegotiationStartNode
        && (pPath->pts[pPath->wNegotiationStartNode].iNodeNum < 0
         || pPath->pts[pPath->wNegotiationStartNode - 1].iNodeNum < 0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    650,
                    0,
                    "%s",
                    "!pPath->wNegotiationStartNode || (pPath->pts[pPath->wNegotiationStartNode].iNodeNum >= 0 && pPath->pts[pPath->"
                    "wNegotiationStartNode - 1].iNodeNum >= 0)") )
    {
        __debugbreak();
    }
    if ( pPath->fLookaheadDistToNextNode != 0.0
        && pPath->lookaheadNextNode >= pPath->wPathLen - 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    651,
                    0,
                    "%s",
                    "!pPath->fLookaheadDistToNextNode || (pPath->lookaheadNextNode < pPath->wPathLen - 1)") )
    {
        __debugbreak();
    }
    return 1;
}

void __fastcall Path_TransferLookahead(path_t *pPath, const float *vStartPos)
{
    const char *v2; // eax
    float fCurrLength; // [esp+14h] [ebp-A8h]
    float vDir[2]; // [esp+60h] [ebp-5Ch]
    float dist; // [esp+68h] [ebp-54h]
    float vLookaheadPos_4; // [esp+70h] [ebp-4Ch]
    float heighta; // [esp+78h] [ebp-44h]
    float height; // [esp+78h] [ebp-44h]
    float bestForwardDot; // [esp+7Ch] [ebp-40h]
    float closestTotalArea; // [esp+80h] [ebp-3Ch]
    float vStartDir[2]; // [esp+84h] [ebp-38h] BYREF
    float offset[2]; // [esp+8Ch] [ebp-30h] BYREF
    int bInFront; // [esp+94h] [ebp-28h]
    int bAtStart; // [esp+98h] [ebp-24h]
    float totalArea; // [esp+9Ch] [ebp-20h]
    float amount; // [esp+A0h] [ebp-1Ch]
    pathpoint_t *pt; // [esp+A4h] [ebp-18h]
    float fLength; // [esp+A8h] [ebp-14h]
    int i; // [esp+ACh] [ebp-10h]
    float prevDot; // [esp+B0h] [ebp-Ch]
    float dot; // [esp+B4h] [ebp-8h]
    float forwardDot; // [esp+B8h] [ebp-4h]

    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 348, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( pPath->wOrigPathLen != pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    349,
                    0,
                    "%s",
                    "pPath->wOrigPathLen == pPath->wPathLen") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    350,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode >= 0") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode >= pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    351,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode < pPath->wPathLen") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen > 1 && pPath->pts[pPath->wPathLen - 2].fOrigLength < pPath->fCurrLength )
    {
        v2 = va(
                     "pPath->fCurrLength: %g, pPath->pts[pPath->wPathLen - 2].fOrigLength: %g",
                     pPath->fCurrLength,
                     pPath->pts[pPath->wPathLen - 2].fOrigLength);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        352,
                        0,
                        "%s\n\t%s",
                        "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                        v2) )
            __debugbreak();
    }
    if ( pPath->fLookaheadDist == 0.0 )
    {
        Path_SetLookaheadToStart(pPath, vStartPos, 0);
        return;
    }
    totalArea = 0.0f;
    amount = pPath->fLookaheadAmount;
    if ( amount <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    362,
                    0,
                    "%s\n\t(amount) = %g",
                    "(amount > 0)",
                    amount) )
    {
        __debugbreak();
    }
    *(_QWORD *)vDir = *(_QWORD *)pPath->lookaheadDir;
    if ( vDir[0] == 0.0
        && vDir[1] == 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    364,
                    0,
                    "%s",
                    "vDir[0] || vDir[1]") )
    {
        __debugbreak();
    }
    vStartDir[0] = pPath->vCurrPoint[0] - *vStartPos;
    vStartDir[1] = pPath->vCurrPoint[1] - vStartPos[1];
    fLength = Vec2Normalize(vStartDir);
    prevDot = (float)((float)(vDir[0] * vStartDir[1]) - (float)(vDir[1] * vStartDir[0])) * fLength;
    bestForwardDot = (float)(vDir[0] * vStartDir[0]) + (float)(vDir[1] * vStartDir[1]);
    closestTotalArea = 0.0f;
    bAtStart = 1;
    i = pPath->wPathLen - 2;
    while ( i >= pPath->wNegotiationStartNode )
    {
        pt = &pPath->pts[i];
        offset[0] = pt->vOrigPoint[0] - *vStartPos;
        offset[1] = pt->vOrigPoint[1] - vStartPos[1];
        if ( bAtStart )
            fCurrLength = pPath->fCurrLength;
        else
            fCurrLength = pt->fOrigLength;
        fLength = fCurrLength;
        if ( pt->fDir2D[0] == 0.0
            && pt->fDir2D[1] == 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        379,
                        0,
                        "%s",
                        "pt->fDir2D[0] || pt->fDir2D[1]") )
        {
            __debugbreak();
        }
        heighta = (float)(pt->fDir2D[0] * offset[1]) - (float)(pt->fDir2D[1] * offset[0]);
        bInFront = (float)((float)((float)(pt->fDir2D[0] * vDir[1]) - (float)(pt->fDir2D[1] * vDir[0])) * heighta) > 0.0;
        height = fabs(heighta);
        dot = (float)(vDir[0] * offset[1]) - (float)(vDir[1] * offset[0]);

        if ( bInFront && (float)(dot * prevDot) < 0.0 )
        {
            Path_UpdateLookahead(pPath, vStartPos, 0, 1, 1, 0);
            return;
        }
        if ( (LODWORD(fLength) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        390,
                        0,
                        "%s",
                        "!IS_NAN(fLength)") )
        {
            __debugbreak();
        }
        if ( fLength <= 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 391, 0, "%s", "fLength > 0") )
        {
            __debugbreak();
        }
        totalArea = (float)(height * fLength) + totalArea;
        if ( totalArea >= amount )
        {
            if ( (LODWORD(height) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            395,
                            0,
                            "%s",
                            "!IS_NAN(height)") )
            {
                __debugbreak();
            }
            if ( height == 0.0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 396, 0, "%s", "height") )
            {
                __debugbreak();
            }
            if ( (LODWORD(fLength) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            397,
                            0,
                            "%s",
                            "!IS_NAN(fLength)") )
            {
                __debugbreak();
            }
            if ( fLength <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            398,
                            0,
                            "%s",
                            "fLength > 0") )
            {
                __debugbreak();
            }
            dist = (float)(totalArea - amount) / height;
            if ( pt->fDir2D[0] == 0.0
                && pt->fDir2D[1] == 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            402,
                            0,
                            "%s",
                            "pt->fDir2D[0] || pt->fDir2D[1]") )
            {
                __debugbreak();
            }
            vLookaheadPos_4 = (float)(  (-(dist)) * pt->fDir2D[1]) + pt->vOrigPoint[1];
            offset[0] = (float)((float)((-(dist)) * pt->fDir2D[0]) + pt->vOrigPoint[0])
                                - *vStartPos;
            offset[1] = vLookaheadPos_4 - vStartPos[1];
            Vec2Normalize(offset);
            forwardDot = (float)(vDir[0] * offset[0]) + (float)(vDir[1] * offset[1]);
            if ( bestForwardDot > forwardDot )
            {
                Path_SetLookaheadToStart(pPath, vStartPos, 1);
                if ( (pPath->flags & 2) == 0 && pPath->fLookaheadAmount > closestTotalArea )
                {
                    if ( closestTotalArea < 64.0 )
                        closestTotalArea = 64.0f;
                    pPath->fLookaheadAmount = closestTotalArea;
                }
                return;
            }
LABEL_79:
            Path_SetLookaheadToStart(pPath, vStartPos, 0);
            return;
        }
        prevDot = dot;
        Vec2Normalize(offset);
        forwardDot = (float)(vDir[0] * offset[0]) + (float)(vDir[1] * offset[1]);
        if ( forwardDot >= bestForwardDot )
        {
            bestForwardDot = forwardDot;
            closestTotalArea = totalArea;
        }
        --i;
        bAtStart = 0;
    }                                                                                         // while()
    if ( closestTotalArea == totalArea )
        goto LABEL_79;
    Path_SetLookaheadToStart(pPath, vStartPos, 1);
    if ( (pPath->flags & 2) == 0 && pPath->fLookaheadAmount > closestTotalArea )
    {
        if ( closestTotalArea < 64.0 )
            closestTotalArea = 64.0f;
        pPath->fLookaheadAmount = closestTotalArea;
    }
}

static int numIter = 1;

void __fastcall Path_SetLookaheadToStart(path_t *pPath, const float *vStartPos, int bTrimAmount)
{
    float v3; // [esp+0h] [ebp-24h]
    int i; // [esp+20h] [ebp-4h]

    pPath->lookaheadDir[0] = pPath->vCurrPoint[0] - *vStartPos;
    pPath->lookaheadDir[1] = pPath->vCurrPoint[1] - vStartPos[1];
    pPath->lookaheadDir[2] = pPath->vCurrPoint[2] - vStartPos[2];
    pPath->fLookaheadDist = Vec2Normalize(pPath->lookaheadDir);
    if ( pPath->fLookaheadDist == 0.0 )
        v3 = 0.0f;
    else
        v3 = pPath->lookaheadDir[2] / pPath->fLookaheadDist;
    pPath->lookaheadDir[2] = v3;
    pPath->fLookaheadDistToNextNode = 0.0f;
    pPath->lookaheadNextNode = pPath->wPathLen - 1;
    if ( ai_useBetterLookahead->current.enabled && !zombiemode->current.enabled )
        numIter = 3;
    for ( i = 0; i < numIter; ++i )
        Path_UpdateLookahead(pPath, vStartPos, 0, bTrimAmount, 1, 0);
}

int __fastcall Path_FindPathInCylinderWithLOS(
                path_t *pPath,
                team_t eTeam,
                const float *vStartPos,
                const float *vGoalPos,
                const actor_goal_s *goal,
                float fWithinDistSqrd,
                int bAllowNegotiationLinks)
{
    CustomSearchInfo_FindPathInCylinderWithLOS info; // [esp+C0h] [ebp-324h] BYREF
    pathsort_t nodes[64]; // [esp+DCh] [ebp-308h] BYREF
    int nodeCount; // [esp+3DCh] [ebp-8h] BYREF
    pathnode_t *pNodeFrom; // [esp+3E0h] [ebp-4h]

    LODWORD(info.negotiationOverlapCost) = ai_pathNegotiationOverlapCost->current.integer;
    info.m_pNodeTo = Path_NearestNode(vGoalPos, nodes, -2, 192.0, &nodeCount, 64, NEAREST_NODE_DO_HEIGHT_CHECK);
    if ( !info.m_pNodeTo )
        return 0;
    pNodeFrom = Path_NearestNode(vStartPos, nodes, -2, 192.0, &nodeCount, 64, NEAREST_NODE_DO_HEIGHT_CHECK);
    if ( !pNodeFrom )
        return 0;
    info.m_fWithinDistSqrd = fWithinDistSqrd;
    info.goal = goal;
    info.startPos[0] = *vStartPos;
    info.startPos[1] = vStartPos[1];
    info.startPos[2] = vStartPos[2];
    return Path_AStarAlgorithm_CustomSearchInfo_FindPathInCylinderWithLOS_(
                     pPath,
                     eTeam,
                     (float*)vStartPos,
                     pNodeFrom,
                     vGoalPos,
                     0,
                     bAllowNegotiationLinks,
                     &info,
                     0);
}

const pathnode_t *__fastcall Path_FindFacingNode(sentient_s *pSelf, sentient_s *pOther, sentient_info_t *pInfo)
{
    pathnode_t *pLastKnownNode; // [esp+0h] [ebp-D8h]
    float vGoalPos[3]; // [esp+B4h] [ebp-24h] BYREF
    CustomSearchInfo_CouldAttack info; // [esp+C0h] [ebp-18h] BYREF
    float vStartPos[3]; // [esp+C8h] [ebp-10h] BYREF
    pathnode_t *pNodeFrom; // [esp+D4h] [ebp-4h]

    info.m_pNodeTo = Sentient_NearestNode(pSelf);
    if ( !info.m_pNodeTo )
        return 0;
    if ( pInfo && pInfo->pLastKnownNode )
        pLastKnownNode = pInfo->pLastKnownNode;
    else
        pLastKnownNode = Sentient_NearestNode(pOther);
    pNodeFrom = pLastKnownNode;
    if ( !pLastKnownNode )
        return 0;
    if ( Path_NodesVisible(pNodeFrom, info.m_pNodeTo) )
        return pNodeFrom;
    Sentient_GetOrigin(pSelf, vGoalPos);
    if ( pInfo )
    {
        vStartPos[0] = pInfo->vLastKnownPos[0];
        vStartPos[1] = pInfo->vLastKnownPos[1];
        vStartPos[2] = pInfo->vLastKnownPos[2];
    }
    else
    {
        Sentient_GetOrigin(pOther, vStartPos);
    }
    if ( Path_AStarAlgorithm_CustomSearchInfo_CouldAttack_(
                 0,
                 pOther->eTeam,
                 vStartPos,
                 pNodeFrom,
                 vGoalPos,
                 0,
                 1,
                 &info,
                 0) )
    {
        return info.attackNode;
    }
    else
    {
        return 0;
    }
}

int __fastcall Path_FindPathGetCloseAsPossible(
                path_t *pPath,
                team_t eTeam,
                pathnode_t *pNodeFrom,
                const float *vStartPos,
                pathnode_t *pNodeTo,
                const float *vGoalPos,
                int bAllowNegotiationLinks)
{
    CustomSearchInfo_FindPathClosestPossible info; // [esp+BCh] [ebp-10h] BYREF

    LODWORD(info.negotiationOverlapCost) = ai_pathNegotiationOverlapCost->current.integer;
    info.m_fBestScore = FLT_MAX;
    info.m_pBestNode = 0;
    if ( !pNodeTo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 1770, 0, "%s", "pNodeTo") )
    {
        __debugbreak();
    }
    if ( !pNodeFrom
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 1771, 0, "%s", "pNodeFrom") )
    {
        __debugbreak();
    }
    if ( (pNodeFrom->constant.spawnflags & 1) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    1772,
                    0,
                    "%s",
                    "(pNodeFrom->constant.spawnflags & PNF_DONTLINK) == 0") )
    {
        __debugbreak();
    }
    info.m_pNodeTo = pNodeTo;
    if ( Path_AStarAlgorithm_CustomSearchInfo_FindPathClosestPossible_(
                 pPath,
                 eTeam,
                 (float*)vStartPos,
                 pNodeFrom,
                 vGoalPos,
                 1,
                 bAllowNegotiationLinks,
                 &info,
                 0) )
    {
        return 1;
    }
    if ( !info.m_pBestNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    1780,
                    0,
                    "%s",
                    "info.m_pBestNode") )
    {
        __debugbreak();
    }
    if ( !Path_GeneratePath(
                    pPath,
                    eTeam,
                    vStartPos,
                    info.m_pBestNode->constant.vOrigin,
                    pNodeFrom,
                    info.m_pBestNode,
                    1,
                    bAllowNegotiationLinks) )
        return 0;
    if ( pPath->wPathLen > pPath->wOrigPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    1785,
                    0,
                    "%s",
                    "pPath->wPathLen <= pPath->wOrigPathLen") )
    {
        __debugbreak();
    }
    return 1;
}

int __fastcall Path_ClipToGoal(path_t *pPath, const actor_goal_s *goal)
{
    int i; // [esp+8h] [ebp-4h]

    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 1893, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    1894,
                    0,
                    "%s",
                    "pPath->wPathLen >= 0") )
    {
        __debugbreak();
    }
    if ( !pPath->wPathLen )
        return 1;
    if ( !Actor_PointAtGoal(pPath->vCurrPoint, goal) )
        return 0;
    for ( i = pPath->wPathLen - 2; i >= 0; --i )
    {
        if ( !Actor_PointAtGoal(pPath->pts[i].vOrigPoint, goal) )
        {
            Path_TrimLastNodes(pPath, i + 1, 0);
            return 1;
        }
    }
    return 1;
}

void __cdecl Path_TrimLastNodes(path_t *pPath, int iNodeCount, bool bMaintainGoalPos)
{
    const char *v3; // eax
    const char *v4; // eax
    int newNegotiationNode; // [esp+18h] [ebp-4h]

    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 1804, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( iNodeCount < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    1805,
                    0,
                    "%s",
                    "iNodeCount >= 0") )
    {
        __debugbreak();
    }
    if ( iNodeCount >= pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    1806,
                    0,
                    "%s",
                    "iNodeCount < pPath->wPathLen") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    1807,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode >= 0") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode > pPath->lookaheadNextNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    1808,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode") )
    {
        __debugbreak();
    }
    if ( pPath->lookaheadNextNode >= pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    1809,
                    0,
                    "%s",
                    "pPath->lookaheadNextNode < pPath->wPathLen") )
    {
        __debugbreak();
    }
    if ( pPath->pts[pPath->lookaheadNextNode].fOrigLength < pPath->fLookaheadDistToNextNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    1810,
                    0,
                    "%s",
                    "pPath->fLookaheadDistToNextNode <= pPath->pts[pPath->lookaheadNextNode].fOrigLength") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen > 1 && pPath->pts[pPath->wPathLen - 2].fOrigLength < pPath->fCurrLength )
    {
        v3 = va(
                     "pPath->fCurrLength: %g, pPath->pts[pPath->wPathLen - 2].fOrigLength: %g",
                     pPath->fCurrLength,
                     pPath->pts[pPath->wPathLen - 2].fOrigLength);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        1811,
                        0,
                        "%s\n\t%s",
                        "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                        v3) )
            __debugbreak();
    }
    if ( iNodeCount )
    {
        pPath->wPathLen -= iNodeCount;
        pPath->wOrigPathLen -= iNodeCount;
        if ( pPath->wDodgeCount >= 0 )
        {
            pPath->wDodgeCount -= iNodeCount;
            if ( pPath->wDodgeCount < 0 )
                pPath->wDodgeCount = 0;
        }
        pPath->lookaheadNextNode -= iNodeCount;
        if ( pPath->lookaheadNextNode < 0 )
        {
            pPath->lookaheadNextNode = 0;
            pPath->fLookaheadDistToNextNode = 0.0f;
        }
        newNegotiationNode = pPath->wNegotiationStartNode - iNodeCount;
        if ( newNegotiationNode > 0 )
        {
            pPath->wNegotiationStartNode = newNegotiationNode;
        }
        else
        {
            if ( pPath->wNegotiationStartNode > 0 )
                Path_DecrementNodeUserCount(pPath);
            pPath->wNegotiationStartNode = 0;
        }
        if ( pPath->wPathLen <= 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        1846,
                        0,
                        "%s",
                        "pPath->wPathLen > 0") )
        {
            __debugbreak();
        }
        if ( pPath->wPathLen <= 1 )
        {
            if ( pPath->wNegotiationStartNode
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            1854,
                            0,
                            "%s",
                            "!pPath->wNegotiationStartNode") )
            {
                __debugbreak();
            }
            pPath->pts[0].vOrigPoint[0] = pPath->vCurrPoint[0];
            pPath->pts[0].vOrigPoint[1] = pPath->vCurrPoint[1];
            pPath->pts[0].vOrigPoint[2] = pPath->vCurrPoint[2];
            pPath->wOrigPathLen = 1;
            pPath->fCurrLength = 0.0f;
            pPath->pts[0].iNodeNum = -1;
            pPath->pts[0].fOrigLength = 0.0f;
            pPath->pts[0].fDir2D[0] = 0.0f;
            pPath->pts[0].fDir2D[1] = 0.0f;
        }
        else
        {
            memmove((unsigned __int8 *)pPath, (unsigned __int8 *)&pPath->pts[iNodeCount], 28 * pPath->wOrigPathLen);
        }
        if ( !bMaintainGoalPos )
        {
            pPath->vFinalGoal[0] = pPath->pts[0].vOrigPoint[0];
            pPath->vFinalGoal[1] = pPath->pts[0].vOrigPoint[1];
            pPath->vFinalGoal[2] = pPath->pts[0].vOrigPoint[2];
        }
        pPath->flags &= ~4u;
        pPath->iPathEndTime = 0;
        if ( pPath->wNegotiationStartNode > pPath->lookaheadNextNode
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        1872,
                        0,
                        "%s",
                        "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode") )
        {
            __debugbreak();
        }
        if ( pPath->lookaheadNextNode >= pPath->wPathLen
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        1873,
                        0,
                        "%s",
                        "pPath->lookaheadNextNode < pPath->wPathLen") )
        {
            __debugbreak();
        }
        if ( pPath->pts[pPath->lookaheadNextNode].fOrigLength < pPath->fLookaheadDistToNextNode
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        1874,
                        0,
                        "%s",
                        "pPath->fLookaheadDistToNextNode <= pPath->pts[pPath->lookaheadNextNode].fOrigLength") )
        {
            __debugbreak();
        }
        if ( pPath->wPathLen > 1 && pPath->pts[pPath->wPathLen - 2].fOrigLength < pPath->fCurrLength )
        {
            v4 = va(
                         "pPath->fCurrLength: %g, pPath->pts[pPath->wPathLen - 2].fOrigLength: %g",
                         pPath->fCurrLength,
                         pPath->pts[pPath->wPathLen - 2].fOrigLength);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            1875,
                            0,
                            "%s\n\t%s",
                            "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                            v4) )
                __debugbreak();
        }
        if ( Path_HasNegotiationNode(pPath)
            && (pPath->pts[pPath->wNegotiationStartNode].iNodeNum < 0
             || pPath->pts[pPath->wNegotiationStartNode - 1].iNodeNum < 0)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        1876,
                        0,
                        "%s",
                        "!Path_HasNegotiationNode( pPath ) || (pPath->pts[pPath->wNegotiationStartNode].iNodeNum >= 0 && pPath->pts[p"
                        "Path->wNegotiationStartNode - 1].iNodeNum >= 0)") )
        {
            __debugbreak();
        }
    }
}

void __fastcall Path_DecrementNodeUserCount(path_t *pPath)
{
    pathnode_t *negotiationNode; // [esp+4h] [ebp-4h]

    negotiationNode = Path_GetNegotiationNode(pPath);
    if ( !negotiationNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    121,
                    0,
                    "%s",
                    "negotiationNode") )
    {
        __debugbreak();
    }
    if ( negotiationNode->dynamic.userCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    122,
                    0,
                    "%s",
                    "negotiationNode->dynamic.userCount > 0") )
    {
        __debugbreak();
    }
    --negotiationNode->dynamic.userCount;
}

void __fastcall Path_BeginTrim(path_t *pPath, path_trim_t *pTrim)
{
    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 1926, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( !pTrim
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 1927, 0, "%s", "pTrim") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen )
    {
        pTrim->iDelta = -2;
        pTrim->iIndex = pPath->wPathLen + 1;
    }
}

int __fastcall Path_TrimToSeePoint(
                path_t *pPath,
                path_trim_t *pTrim,
                actor_s *pActor,
                float fMaxDistSqrd,
                int iIgnoreEntityNum,
                const float *vPoint)
{
    int v7; // edx
    float v8; // xmm0_4
    bool bAtStart; // [esp+18h] [ebp-20h]
    float vEyePos[3]; // [esp+1Ch] [ebp-1Ch] BYREF
    float vEyeOffset[3]; // [esp+28h] [ebp-10h] BYREF
    int i; // [esp+34h] [ebp-4h]

    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 1948, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( !pTrim
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 1949, 0, "%s", "pTrim") )
    {
        __debugbreak();
    }
    if ( !pActor
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 1950, 0, "%s", "pActor") )
    {
        __debugbreak();
    }
    if ( !vPoint
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 1951, 0, "%s", "vPoint") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen <= 0 )
        return 0;
    if ( pTrim->iDelta != -2
        && pTrim->iDelta != -1
        && pTrim->iDelta != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    1961,
                    0,
                    "%s",
                    "pTrim->iDelta == -2 || pTrim->iDelta == -1 || pTrim->iDelta == +1") )
    {
        __debugbreak();
    }
    i = pTrim->iDelta + pTrim->iIndex;
    if ( i < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 1964, 0, "%s", "i >= 0") )
    {
        __debugbreak();
    }
    if ( i >= pPath->wPathLen )
        return 0;
    Actor_GetEyeOffset(pActor, vEyeOffset);
    v7 = pPath->wPathLen - 1;
    bAtStart = i == v7;
    if ( i == v7 )
    {
        vEyePos[0] = pPath->vCurrPoint[0] + vEyeOffset[0];
        vEyePos[1] = pPath->vCurrPoint[1] + vEyeOffset[1];
        v8 = pPath->vCurrPoint[2];
    }
    else
    {
        vEyePos[0] = pPath->pts[i].vOrigPoint[0] + vEyeOffset[0];
        vEyePos[1] = pPath->pts[i].vOrigPoint[1] + vEyeOffset[1];
        v8 = pPath->pts[i].vOrigPoint[2];
    }
    vEyePos[2] = v8 + vEyeOffset[2];
    if ( Actor_CanSeePointFrom(pActor, vEyePos, vPoint, fMaxDistSqrd, iIgnoreEntityNum) )
    {
        if ( pTrim->iDelta == -2 && !bAtStart )
        {
            pTrim->iIndex = i;
            pTrim->iDelta = 1;
            return 1;
        }
        else
        {
            Path_TrimLastNodes(pPath, i, 0);
            return 0;
        }
    }
    else if ( pTrim->iDelta == 1 )
    {
        Path_TrimLastNodes(pPath, pTrim->iIndex, 0);
        return 0;
    }
    else if ( i )
    {
        if ( pTrim->iDelta != -2
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        2017,
                        0,
                        "%s",
                        "pTrim->iDelta == -2") )
        {
            __debugbreak();
        }
        pTrim->iIndex = i;
        if ( pTrim->iIndex == 1 )
            pTrim->iDelta = -1;
        return 1;
    }
    else
    {
        Actor_ClearPath(pActor);
        return 0;
    }
}

void __fastcall Path_Begin(path_t *pPath)
{
    memset(pPath, 0, sizeof(path_t));
    pPath->fLookaheadAmount = 21845.334f;
    pPath->wDodgeEntity = 1023;
}

void __fastcall Path_Clear(path_t *pPath)
{
    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 2050, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode > 0 )
    {
        Path_DecrementNodeUserCount(pPath);
        pPath->wNegotiationStartNode = 0;
    }
    pPath->wPathLen = 0;
    pPath->wOrigPathLen = 0;
}

bool __fastcall Path_Exists(const path_t *pPath)
{
    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 2072, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    return pPath->wPathLen > 0;
}

bool __fastcall Path_CompleteLookahead(const path_t *pPath)
{
    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 2086, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2087,
                    0,
                    "%s",
                    "pPath->wPathLen > 0") )
    {
        __debugbreak();
    }
    return (pPath->flags & 1) != 0;
}

bool __fastcall Path_AttemptedCompleteLookahead(const path_t *pPath)
{
    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 2102, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2103,
                    0,
                    "%s",
                    "pPath->wPathLen > 0") )
    {
        __debugbreak();
    }
    return (pPath->flags & 0x40) != 0;
}

bool __fastcall Path_UsesObstacleNegotiation(const path_t *pPath)
{
    const char *v1; // eax

    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 2118, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2119,
                    0,
                    "%s",
                    "pPath->wPathLen > 0") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2120,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode >= 0") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode > pPath->lookaheadNextNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2121,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode") )
    {
        __debugbreak();
    }
    if ( pPath->lookaheadNextNode >= pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2122,
                    0,
                    "%s",
                    "pPath->lookaheadNextNode < pPath->wPathLen") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen > 1 && pPath->pts[pPath->wPathLen - 2].fOrigLength < pPath->fCurrLength )
    {
        v1 = va(
                     "pPath->fCurrLength: %g, pPath->pts[pPath->wPathLen - 2].fOrigLength: %g",
                     pPath->fCurrLength,
                     pPath->pts[pPath->wPathLen - 2].fOrigLength);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        2123,
                        0,
                        "%s\n\t%s",
                        "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                        v1) )
            __debugbreak();
    }
    return pPath->wNegotiationStartNode > 0;
}

bool __fastcall Path_HasNegotiationNode(const path_t *path)
{
    return Path_Exists(path) && path->wNegotiationStartNode > 0;
}

bool __fastcall Path_AllowsObstacleNegotiation(const path_t *pPath)
{
    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 2154, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2155,
                    0,
                    "%s",
                    "pPath->wPathLen > 0") )
    {
        __debugbreak();
    }
    return (pPath->flags & 0x10) != 0;
}

void __fastcall Path_GetObstacleNegotiationScript(const path_t *pPath, scr_animscript_t *animscript)
{
    const char *v2; // eax
    pathnode_t *v3; // eax

    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 2171, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2172,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode > 0") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode >= pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2173,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode < pPath->wPathLen") )
    {
        __debugbreak();
    }
    if ( pPath->pts[pPath->wNegotiationStartNode].iNodeNum < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2174,
                    0,
                    "%s",
                    "pPath->pts[pPath->wNegotiationStartNode].iNodeNum >= 0") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen > 1 && pPath->pts[pPath->wPathLen - 2].fOrigLength < pPath->fCurrLength )
    {
        v2 = va(
                     "pPath->fCurrLength: %g, pPath->pts[pPath->wPathLen - 2].fOrigLength: %g",
                     pPath->fCurrLength,
                     pPath->pts[pPath->wPathLen - 2].fOrigLength);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        2175,
                        0,
                        "%s\n\t%s",
                        "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                        v2) )
            __debugbreak();
    }
    if ( !animscript
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 2176, 0, "%s", "animscript") )
    {
        __debugbreak();
    }
    v3 = Path_ConvertIndexToNode(pPath->pts[pPath->wNegotiationStartNode].iNodeNum);
    animscript->func = v3->constant.animscriptfunc;
    Scr_SetString(&animscript->name, v3->constant.animscript, SCRIPTINSTANCE_SERVER);
}

bool __fastcall Path_NeedsReevaluation(const path_t *pPath)
{
    pathnode_t *nextNode; // [esp+4h] [ebp-4h]

    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 2193, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2196,
                    0,
                    "%s",
                    "pPath->wPathLen > 0") )
    {
        __debugbreak();
    }
    if ( (pPath->flags & 4) != 0 && pPath->wPathLen <= 8 )
        return 1;
    if ( pPath->lookaheadNextNode < 0 )
        return 0;
    if ( pPath->pts[pPath->lookaheadNextNode].iNodeNum < 0 )
        return 0;
    nextNode = Path_ConvertIndexToNode(pPath->pts[pPath->lookaheadNextNode].iNodeNum);
    return nextNode && !nextNode->dynamic.wLinkCount;
}

int __fastcall Path_DistanceGreaterThan(path_t *pPath, float fDist)
{
    const char *v3; // eax
    float totalLength; // [esp+14h] [ebp-8h]
    int i; // [esp+18h] [ebp-4h]

    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 2370, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2371,
                    0,
                    "%s",
                    "pPath->wPathLen > 0") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2372,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode >= 0") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode > pPath->lookaheadNextNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2373,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode") )
    {
        __debugbreak();
    }
    if ( pPath->lookaheadNextNode >= pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2374,
                    0,
                    "%s",
                    "pPath->lookaheadNextNode < pPath->wPathLen") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen > 1 && pPath->pts[pPath->wPathLen - 2].fOrigLength < pPath->fCurrLength )
    {
        v3 = va(
                     "pPath->fCurrLength: %g, pPath->pts[pPath->wPathLen - 2].fOrigLength: %g",
                     pPath->fCurrLength,
                     pPath->pts[pPath->wPathLen - 2].fOrigLength);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        2375,
                        0,
                        "%s\n\t%s",
                        "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                        v3) )
            __debugbreak();
    }
    totalLength = pPath->fLookaheadDist + pPath->fLookaheadDistToNextNode;
    if ( pPath->wNegotiationStartNode )
        totalLength = totalLength + 128.0;
    if ( totalLength > fDist )
        return 1;
    if ( !pPath->lookaheadNextNode )
        return 0;
    i = pPath->lookaheadNextNode - 1;
    if ( i != pPath->wPathLen - 2 )
        goto LABEL_33;
    if ( pPath->fCurrLength < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2391,
                    0,
                    "%s",
                    "pPath->fCurrLength >= 0") )
    {
        __debugbreak();
    }
    totalLength = totalLength + pPath->fCurrLength;
    if ( totalLength > fDist )
        return 1;
    --i;
LABEL_33:
    while ( i >= 0 )
    {
        if ( pPath->pts[i].fOrigLength <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        2400,
                        0,
                        "%s",
                        "pPath->pts[i].fOrigLength > 0") )
        {
            __debugbreak();
        }
        totalLength = totalLength + pPath->pts[i].fOrigLength;
        if ( totalLength > fDist )
            return 1;
        --i;
    }
    return 0;
}

bool __fastcall Path_FailedLookahead(path_t *pPath)
{
    return pPath->fLookaheadAmount <= 0.001;
}

int __fastcall Path_PredictionTraceCheckForEntities(
                float *vStartPos,
                float *vEndPos,
                const int *entities,
                int entityCount,
                int entityIgnore,
                int mask,
                float *vTraceEndPos)
{
    sightclip_t clip; // [esp+14h] [ebp-58h] BYREF
    int entityIndex; // [esp+60h] [ebp-Ch]
    float stepheight; // [esp+64h] [ebp-8h]
    gentity_s *ent; // [esp+68h] [ebp-4h]

    clip.start[0] = *vStartPos;
    clip.start[1] = vStartPos[1];
    clip.start[2] = vStartPos[2];
    clip.end[0] = *vEndPos;
    clip.end[1] = vEndPos[1];
    clip.end[2] = vStartPos[2];
    clip.mins[0] = PREDICTION_TRACE_MIN[0];
    clip.mins[1] = PREDICTION_TRACE_MIN[1];
    clip.mins[2] = PREDICTION_TRACE_MIN[2];
    clip.maxs[0] = PREDICTION_TRACE_MAX[0];
    clip.maxs[1] = PREDICTION_TRACE_MAX[1];
    clip.maxs[2] = PREDICTION_TRACE_MAX[2];
    clip.contentmask = -1;
    clip.passEntityNum[1] = 1023;
    clip.passEntityNum[0] = 1023;
    for ( entityIndex = 0; entityIndex < entityCount; ++entityIndex )
    {
        ent = (gentity_s *)((char *)sv.gentities + entities[entityIndex] * sv.gentitySize);
        if ( ent && ent->actor && ent->actor->Physics.prone )
            stepheight = 10.0f;
        else
            stepheight = 18.0f;
        clip.mins[2] = stepheight + 0.0;
        if ( SV_SightTraceCapsuleToEntity(&clip, entities[entityIndex]) )
        {
            *vTraceEndPos = *vStartPos;
            vTraceEndPos[1] = vStartPos[1];
            vTraceEndPos[2] = vStartPos[2];
            return 2;
        }
    }
    return !Path_PredictionTrace(vStartPos, vEndPos, entityIgnore, mask, vTraceEndPos, 18.0, 1);
}

bool __fastcall Path_PredictionTrace(
                float *vStartPos,
                float *vEndPos,
                int entityIgnore,
                int mask,
                float *vTraceEndPos,
                float stepheight,
                int allowStartSolid)
{
    bool v10; // [esp+6Ch] [ebp-A4h]
    col_context_t context; // [esp+70h] [ebp-A0h] BYREF
    float vDown[3]; // [esp+9Ch] [ebp-74h] BYREF
    float vDest[3]; // [esp+A8h] [ebp-68h] BYREF
    float traceMin[3]; // [esp+B4h] [ebp-5Ch] BYREF
    trace_t trace; // [esp+C0h] [ebp-50h] BYREF
    float vSource[3]; // [esp+FCh] [ebp-14h] BYREF
    actor_s *actor; // [esp+108h] [ebp-8h]
    unsigned __int16 hitEntId; // [esp+10Ch] [ebp-4h]

    PROF_SCOPED("pathpredictiontrace");

    traceMin[0] = PREDICTION_TRACE_MIN[0];
    traceMin[1] = PREDICTION_TRACE_MIN[1];
    traceMin[2] = stepheight + 0.0;
    if ( !vStartPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 2559, 0, "%s", "vStartPos") )
    {
        __debugbreak();
    }
    if ( !vEndPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 2560, 0, "%s", "vEndPos") )
    {
        __debugbreak();
    }
    vDest[0] = *vEndPos;
    vDest[1] = vEndPos[1];
    vDest[2] = vStartPos[2];
    vSource[0] = *vStartPos;
    vSource[1] = vStartPos[1];
    vSource[2] = vStartPos[2];
    //col_context_t::col_context_t(&context);
    while ( 1 )
    {
        G_TraceCapsule(&trace, vSource, traceMin, PREDICTION_TRACE_MAX, vDest, entityIgnore, mask, &context);
        Vec3Lerp(vSource, vDest, trace.fraction, vTraceEndPos);
        if ( trace.fraction < 0.000099999997 )
        {
            return 0;
        }
        if ( trace.startsolid && !allowStartSolid )
        {
            return 0;
        }
        if ( !trace.allsolid && trace.fraction == 1.0 )
        {
            vSource[0] = *vTraceEndPos;
            vSource[1] = vTraceEndPos[1];
            vDown[0] = *vTraceEndPos;
            vDown[1] = vTraceEndPos[1];
            vDown[2] = vSource[2] - 48.0;
            G_TraceCapsule(&trace, vSource, traceMin, PREDICTION_TRACE_MAX, vDown, entityIgnore, mask, &context);
            Vec3Lerp(vSource, vDown, trace.fraction, vTraceEndPos);
            if ( vTraceEndPos[2] < vSource[2] || trace.fraction == 1.0 || trace.normal.vec.v[2] >= 0.69999999 )
            {
                vTraceEndPos[2] = vTraceEndPos[2] + stepheight;
                v10 = fabs(vTraceEndPos[2] - vEndPos[2]) < 48.0;
                return v10;
            }
            else
            {
                return 0;
            }
        }
        hitEntId = Trace_GetEntityHitId(&trace);
        if ( !level.gentities[hitEntId].sentient )
            break;
        actor = level.gentities[hitEntId].actor;
        if ( !actor )
        {
            return 0;
        }
        if ( (actor->eAnimMode != AI_ANIM_MOVE_CODE || !actor->moveMode)
            && (!actor->pPileUpActor || actor->pPileUpActor->ent->s.number != entityIgnore) )
        {
            return 0;
        }
        mask &= ~0x8000u;
LABEL_45:
        vSource[0] = *vTraceEndPos;
        vSource[1] = vTraceEndPos[1];
        vDown[0] = *vTraceEndPos;
        vDown[1] = vTraceEndPos[1];
        vDown[2] = vSource[2] - 48.0;
        G_TraceCapsule(&trace, vSource, traceMin, PREDICTION_TRACE_MAX, vDown, entityIgnore, mask, &context);
        Vec3Lerp(vSource, vDown, trace.fraction, vTraceEndPos);
        if ( vTraceEndPos[2] >= vSource[2] && trace.fraction != 1.0 && trace.normal.vec.v[2] < 0.69999999 )
        {
            return 0;
        }
        vSource[2] = vTraceEndPos[2] + stepheight;
        vDest[2] = vSource[2];
    }
    if ((float)((float)((float)(vSource[0] - *vTraceEndPos) * (float)(vSource[0] - *vTraceEndPos))
        + (float)((float)(vSource[1] - vTraceEndPos[1]) * (float)(vSource[1] - vTraceEndPos[1]))) >= 225.0)
    {
        goto LABEL_45;
    }
    return 0;
}

bool __fastcall Path_LookaheadPredictionTrace(path_t *pPath, float *vStartPos, float *vEndPos)
{
    float vTraceEndPos[3]; // [esp+10h] [ebp-14h] BYREF
    int entities[1]; // [esp+1Ch] [ebp-8h] BYREF
    int mask; // [esp+20h] [ebp-4h]

    //mask = (int)&loc_82000C + 5;
    mask = 0x820011;
    if ( pPath->wDodgeCount && pPath->wDodgeEntity != 1023 )
    {
        if ( level.gentities[pPath->wDodgeEntity].actor )
        {
            entities[0] = pPath->wDodgeEntity;
            return Path_PredictionTraceCheckForEntities(vStartPos, vEndPos, entities, 1, 1023, mask, vTraceEndPos) == 0;
        }
        mask = 0x2820011;
    }
    return Path_PredictionTrace(vStartPos, vEndPos, 1023, mask, vTraceEndPos, 18.0, 1);
}

int __fastcall Path_IsTrimmed(path_t *pPath)
{
    const char *v1; // eax
    pathpoint_t *pt; // [esp+1Ch] [ebp-4h]

    if ( !pPath->wOrigPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2851,
                    0,
                    "%s",
                    "pPath->wOrigPathLen") )
    {
        __debugbreak();
    }
    if ( !pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2852,
                    0,
                    "%s",
                    "pPath->wPathLen") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2853,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode >= 0") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode >= pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2854,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode < pPath->wPathLen") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen > 1 && pPath->pts[pPath->wPathLen - 2].fOrigLength < pPath->fCurrLength )
    {
        v1 = va(
                     "pPath->fCurrLength: %g, pPath->pts[pPath->wPathLen - 2].fOrigLength: %g",
                     pPath->fCurrLength,
                     pPath->pts[pPath->wPathLen - 2].fOrigLength);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        2855,
                        0,
                        "%s\n\t%s",
                        "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                        v1) )
            __debugbreak();
    }
    if ( pPath->wPathLen - 1 <= pPath->wNegotiationStartNode )
        return 0;
    pt = &pPath->pts[pPath->wPathLen - 1];
    if ( pt->iNodeNum < 0 )
        return 0;
    if ( pPath->pts[pPath->wPathLen - 2].iNodeNum < 0 )
        return 0;
    if ( (pPath->flags & 0x20) != 0 )
        return 0;
    if ( Path_FailedLookahead(pPath) )
        return 0;
    if ( pPath->wPathLen == pPath->wOrigPathLen
        && pPath->vCurrPoint[0] == pt->vOrigPoint[0]
        && pPath->vCurrPoint[1] == pt->vOrigPoint[1]
        && pPath->vCurrPoint[2] == pt->vOrigPoint[2] )
    {
        return 0;
    }
    return 1;
}

int __fastcall Path_GetForwardStartPos(path_t *pPath, const float *vStartPos, float *vForwardStartPos)
{
    float fLookaheadDist; // [esp+0h] [ebp-18h]

    if ( pPath->wPathLen <= 1 )
        return 0;
    if ( (pPath->flags & 1) != 0 )
        return 0;
    if ( (float)(60.0 - pPath->fLookaheadDist) < 0.0 )
        fLookaheadDist = 60.0f;
    else
        fLookaheadDist = pPath->fLookaheadDist;
    *vForwardStartPos = (float)(fLookaheadDist * pPath->lookaheadDir[0]) + *vStartPos;
    vForwardStartPos[1] = (float)(fLookaheadDist * pPath->lookaheadDir[1]) + vStartPos[1];
    return 1;
}

static float minSpeed = 3.0f;
void __fastcall Path_UpdateLookahead_NonCodeMove(path_t *pPath, const float *vPrevPos, const float *vStartPos)
{
    float fLookaheadAmount; // [esp+0h] [ebp-34h]
    pathpoint_t *pt; // [esp+14h] [ebp-20h]
    int i; // [esp+18h] [ebp-1Ch]
    float moveDir; // [esp+2Ch] [ebp-8h]
    float moveDir_4; // [esp+30h] [ebp-4h]

    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 3553, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( !vStartPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 3554, 0, "%s", "vStartPos") )
    {
        __debugbreak();
    }
    if ( !vPrevPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 3555, 0, "%s", "vPrevPos") )
    {
        __debugbreak();
    }
    moveDir = *vStartPos - *vPrevPos;
    moveDir_4 = vStartPos[1] - vPrevPos[1];
    if ( (float)((float)(moveDir * moveDir) + (float)(moveDir_4 * moveDir_4)) > (float)(minSpeed * minSpeed) )
    {
        for ( i = pPath->wPathLen - 2; i >= pPath->wNegotiationStartNode; --i )
        {
            pt = &pPath->pts[i];
            if ( (float)((float)(moveDir * (float)(pt->vOrigPoint[0] - *vPrevPos))
                                 + (float)(moveDir_4 * (float)(pt->vOrigPoint[1] - vPrevPos[1]))) >= 0.0 )
            {
                if ( (float)((float)(moveDir * (float)(pt->vOrigPoint[0] - *vStartPos))
                                     + (float)(moveDir_4 * (float)(pt->vOrigPoint[1] - vStartPos[1]))) <= 0.0
                    && i > pPath->wNegotiationStartNode )
                {
                    pPath->vCurrPoint[0] = pt->vOrigPoint[0];
                    pPath->vCurrPoint[1] = pt->vOrigPoint[1];
                    pPath->vCurrPoint[2] = pt->vOrigPoint[2];
                    pPath->fCurrLength = pPath->pts[i - 1].fOrigLength;
                    Path_RemoveCompletedPathPoints(pPath, i - 1);
                }
                break;
            }
        }
    }
    if ( (float)(pPath->fLookaheadAmount - 1024.0) < 0.0 )
        fLookaheadAmount = 1024.0f;
    else
        fLookaheadAmount = pPath->fLookaheadAmount;
    pPath->fLookaheadAmount = fLookaheadAmount;
    Path_UpdateLookahead(pPath, vStartPos, 0, 0, 0, 1);
}

void __fastcall Path_RemoveCompletedPathPoints(path_t *pPath, __int16 pathPointIndex)
{
    const char *v2; // eax
    const char *v3; // eax

    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 2889, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( pPath->wDodgeCount >= 0 )
    {
        pPath->wDodgeCount -= pPath->wPathLen;
        pPath->wPathLen = pathPointIndex + 2;
        pPath->wDodgeCount += pPath->wPathLen;
        if ( pPath->wDodgeCount < 0 )
            pPath->wDodgeCount = 0;
        if ( pPath->wPathLen > 1 && pPath->pts[pPath->wPathLen - 2].fOrigLength < pPath->fCurrLength )
        {
            v3 = va(
                         "pPath->fCurrLength: %g, pPath->pts[pPath->wPathLen - 2].fOrigLength: %g",
                         pPath->fCurrLength,
                         pPath->pts[pPath->wPathLen - 2].fOrigLength);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            2906,
                            0,
                            "%s\n\t%s",
                            "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                            v3) )
                __debugbreak();
        }
        if ( pPath->wPathLen
            && pPath->wNegotiationStartNode
            && (pPath->pts[pPath->wNegotiationStartNode].iNodeNum < 0
             || pPath->pts[pPath->wNegotiationStartNode - 1].iNodeNum < 0)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        2907,
                        0,
                        "%s",
                        "!pPath->wPathLen || !pPath->wNegotiationStartNode || (pPath->pts[pPath->wNegotiationStartNode].iNodeNum >= 0"
                        " && pPath->pts[pPath->wNegotiationStartNode - 1].iNodeNum >= 0)") )
        {
            __debugbreak();
        }
    }
    else
    {
        pPath->wPathLen = pathPointIndex + 2;
        if ( pPath->wPathLen > 1 && pPath->pts[pPath->wPathLen - 2].fOrigLength < pPath->fCurrLength )
        {
            v2 = va(
                         "pPath->fCurrLength: %g, pPath->pts[pPath->wPathLen - 2].fOrigLength: %g",
                         pPath->fCurrLength,
                         pPath->pts[pPath->wPathLen - 2].fOrigLength);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            2894,
                            0,
                            "%s\n\t%s",
                            "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                            v2) )
                __debugbreak();
        }
        if ( pPath->wPathLen
            && pPath->wNegotiationStartNode
            && (pPath->pts[pPath->wNegotiationStartNode].iNodeNum < 0
             || pPath->pts[pPath->wNegotiationStartNode - 1].iNodeNum < 0)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        2895,
                        0,
                        "%s",
                        "!pPath->wPathLen || !pPath->wNegotiationStartNode || (pPath->pts[pPath->wNegotiationStartNode].iNodeNum >= 0"
                        " && pPath->pts[pPath->wNegotiationStartNode - 1].iNodeNum >= 0)") )
        {
            __debugbreak();
        }
    }
}

void __fastcall Path_UpdateLookahead(
                path_t *pPath,
                const float *vStartPos,
                int bReduceLookaheadAmount,
                int bTrimAmount,
                int bAllowBacktrack,
                int bAllowRestore)
{
    const char *v6; // eax
    float v7; // [esp+10h] [ebp-30h]

    if ( pPath->wNegotiationStartNode < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3617,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode >= 0") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen <= pPath->wNegotiationStartNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3618,
                    0,
                    "%s",
                    "pPath->wPathLen > pPath->wNegotiationStartNode") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen > 1 && pPath->pts[pPath->wPathLen - 2].fOrigLength < pPath->fCurrLength )
    {
        v6 = va(
                     "pPath->fCurrLength: %g, pPath->pts[pPath->wPathLen - 2].fOrigLength: %g",
                     pPath->fCurrLength,
                     pPath->pts[pPath->wPathLen - 2].fOrigLength);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3619,
                        0,
                        "%s\n\t%s",
                        "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                        v6) )
            __debugbreak();
    }
    pPath->flags &= ~0x20u;
    if ( pPath->wNegotiationStartNode == pPath->wPathLen - 1 )
    {
        pPath->lookaheadDir[0] = pPath->vCurrPoint[0] - *vStartPos;
        pPath->lookaheadDir[1] = pPath->vCurrPoint[1] - vStartPos[1];
        pPath->lookaheadDir[2] = pPath->vCurrPoint[2] - vStartPos[2];
        pPath->fLookaheadDist = Vec2Normalize(pPath->lookaheadDir);
        if ( pPath->fLookaheadDist == 0.0 )
            v7 = 0.0f;
        else
            v7 = pPath->lookaheadDir[2] / pPath->fLookaheadDist;
        pPath->lookaheadDir[2] = v7;
        pPath->fLookaheadDistToNextNode = 0.0f;
        pPath->lookaheadNextNode = pPath->wNegotiationStartNode;
        pPath->flags |= 0x41u;
    }
    else
    {
        if ( (float)((float)(pPath->lookaheadDir[0] * (float)(pPath->vCurrPoint[0] - *vStartPos))
                             + (float)(pPath->lookaheadDir[1] * (float)(pPath->vCurrPoint[1] - vStartPos[1]))) > 0.0 )
        {
            if ( bAllowBacktrack && pPath->fLookaheadAmount >= 64.0 )
                Path_BacktrackCompletedPath(pPath, vStartPos);
        }
        else
        {
            if ( pPath->iPathTime < level.time && Path_NeedsReevaluation(pPath) )
            {
                Path_FindPath(pPath, pPath->eTeam, vStartPos, pPath->vFinalGoal, (pPath->flags & 0x10) != 0);
                return;
            }
            if ( (pPath->flags & 2) != 0 || pPath->fLookaheadAmount <= 16384.0 )
                Path_TrimCompletedPath(pPath, vStartPos);
            if ( bTrimAmount )
                Path_SubtractTrimmedAmount(pPath, vStartPos);
        }
        Path_CalcLookahead(pPath, vStartPos, bReduceLookaheadAmount, bAllowRestore);
    }
    Path_UpdateForwardLookahead(pPath, vStartPos);
}

void __fastcall Path_SubtractTrimmedAmount(path_t *pPath, const float *vStartPos)
{
    float height; // [esp+18h] [ebp-10h]
    float heighta; // [esp+18h] [ebp-10h]
    float amount; // [esp+1Ch] [ebp-Ch]
    pathpoint_t *pt; // [esp+20h] [ebp-8h]
    pathpoint_t *pta; // [esp+20h] [ebp-8h]
    int i; // [esp+24h] [ebp-4h]

    if ( pPath->wPathLen != pPath->wOrigPathLen )
    {
        if ( pPath->wPathLen <= 1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        265,
                        0,
                        "%s",
                        "pPath->wPathLen > 1") )
        {
            __debugbreak();
        }
        pt = &pPath->pts[pPath->wPathLen - 2];
        height = Path_GetDistToPathSegment(vStartPos, pt);
        if ( pt->fOrigLength <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        269,
                        0,
                        "%s",
                        "pt->fOrigLength > 0") )
        {
            __debugbreak();
        }
        if ( pPath->fCurrLength <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        270,
                        0,
                        "%s",
                        "pPath->fCurrLength > 0") )
        {
            __debugbreak();
        }
        if ( pt->fOrigLength < pPath->fCurrLength
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        271,
                        0,
                        "pt->fOrigLength >= pPath->fCurrLength\n\t%g, %g",
                        pt->fOrigLength,
                        pPath->fCurrLength) )
        {
            __debugbreak();
        }
        amount = (float)(pt->fOrigLength - pPath->fCurrLength) * height;
        for ( i = pPath->wPathLen - 1; i < pPath->wOrigPathLen - 1; ++i )
        {
            pta = &pPath->pts[i];
            heighta = Path_GetDistToPathSegment(vStartPos, pta);
            if ( pta->fOrigLength <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            279,
                            0,
                            "%s",
                            "pt->fOrigLength > 0") )
            {
                __debugbreak();
            }
            amount = (float)(heighta * pta->fOrigLength) + amount;
        }
        pPath->fLookaheadAmount = pPath->fLookaheadAmount - amount;
        if ( pPath->fLookaheadAmount < 64.0 )
            pPath->fLookaheadAmount = 64.0f;
    }
}

void __fastcall Path_TrimCompletedPath(path_t *pPath, const float *vStartPos)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    float d1; // [esp+3Ch] [ebp-34h]
    float dist; // [esp+40h] [ebp-30h]
    float fraction; // [esp+44h] [ebp-2Ch]
    int bAtStart; // [esp+50h] [ebp-20h]
    float d2; // [esp+54h] [ebp-1Ch]
    pathpoint_t *pt; // [esp+58h] [ebp-18h]
    float fLength; // [esp+5Ch] [ebp-14h]
    int i; // [esp+60h] [ebp-10h]
    int ia; // [esp+60h] [ebp-10h]
    pathpoint_t *prevPos; // [esp+64h] [ebp-Ch]

    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 2934, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2935,
                    0,
                    "%s",
                    "pPath->wPathLen > 0") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2936,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode >= 0") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode >= pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2937,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode < pPath->wPathLen") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen > 1 && pPath->pts[pPath->wPathLen - 2].fOrigLength < pPath->fCurrLength )
    {
        v2 = va(
                     "pPath->fCurrLength: %g, pPath->pts[pPath->wPathLen - 2].fOrigLength: %g",
                     pPath->fCurrLength,
                     pPath->pts[pPath->wPathLen - 2].fOrigLength);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        2938,
                        0,
                        "%s\n\t%s",
                        "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                        v2) )
            __debugbreak();
    }
    bAtStart = 1;
    for ( i = pPath->wPathLen - 2; i >= pPath->wNegotiationStartNode; --i )
    {
        pt = &pPath->pts[i];
        d2 = (float)(pPath->lookaheadDir[0] * (float)(pt->vOrigPoint[0] - *vStartPos))
             + (float)(pPath->lookaheadDir[1] * (float)(pt->vOrigPoint[1] - vStartPos[1]));
        if ( d2 > 0.0 )
        {
            if ( bAtStart )
            {
                prevPos = (pathpoint_t *)pPath->vCurrPoint;
                fLength = pPath->fCurrLength;
            }
            else
            {
                prevPos = &pPath->pts[i + 1];
                fLength = pt->fOrigLength;
            }
            d1 = (float)((float)(pPath->lookaheadDir[0] * (float)(prevPos->vOrigPoint[0] - *vStartPos))
                                 + (float)(pPath->lookaheadDir[1] * (float)(prevPos->vOrigPoint[1] - vStartPos[1])))
                 - 0.000099999997;
            if ( d1 > 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            2965,
                            0,
                            "%s\n\t(d1) = %g",
                            "(d1 <= 0)",
                            d1) )
            {
                __debugbreak();
            }
            if ( d2 <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            2966,
                            0,
                            "%s\n\t(d2) = %g",
                            "(d2 > 0)",
                            d2) )
            {
                __debugbreak();
            }
            if ( d1 - d2 >= 0.0 )
            {
                v3 = va("%g %g\n", d1, d2);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                                2967,
                                0,
                                "%s\n\t%s",
                                "d1 - d2 < 0",
                                v3) )
                    __debugbreak();
            }
            fraction = d1 / (d1 - d2);
            if ( fraction < 0.0 )
            {
                v4 = va("i: %d, pPath->wPathLen: %d, d1: %f, d2: %f, fraction: %f", i, pPath->wPathLen, d1, d2, fraction);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                                2971,
                                0,
                                "%s\n\t%s",
                                "fraction >= 0",
                                v4) )
                    __debugbreak();
            }
            if ( fraction > 1.0 )
            {
                v5 = va("i: %d, pPath->wPathLen: %d, d1: %f, d2: %f, fraction: %f", i, pPath->wPathLen, d1, d2, fraction);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                                2974,
                                0,
                                "%s\n\t%s",
                                "fraction <= 1.f",
                                v5) )
                    __debugbreak();
            }
            if ( (LODWORD(fLength) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            2976,
                            0,
                            "%s",
                            "!IS_NAN(fLength)") )
            {
                __debugbreak();
            }
            if ( fLength <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            2977,
                            0,
                            "%s",
                            "fLength > 0") )
            {
                __debugbreak();
            }
            dist = fraction * fLength;
            if ( fLength < (float)(fraction * fLength)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            2980,
                            0,
                            "%s",
                            "dist <= fLength") )
            {
                __debugbreak();
            }
            if ( dist < fLength )
            {
                if ( pt->fDir2D[0] == 0.0
                    && pt->fDir2D[1] == 0.0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                                2996,
                                0,
                                "%s",
                                "pt->fDir2D[0] || pt->fDir2D[1]") )
                {
                    __debugbreak();
                }
                if ( dist < 0.0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                                2997,
                                0,
                                "%s",
                                "dist >= 0") )
                {
                    __debugbreak();
                }
                pPath->vCurrPoint[0] = (float)(dist * pt->fDir2D[0]) + prevPos->vOrigPoint[0];
                pPath->vCurrPoint[1] = (float)(dist * pt->fDir2D[1]) + prevPos->vOrigPoint[1];
                pPath->vCurrPoint[2] = (float)((float)(pt->vOrigPoint[2] - prevPos->vOrigPoint[2]) * fraction)
                                                         + prevPos->vOrigPoint[2];
                pPath->fCurrLength = fLength - dist;
                if ( (LODWORD(pPath->fCurrLength) & 0x7F800000) == 0x7F800000
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                                3004,
                                0,
                                "%s",
                                "!IS_NAN(pPath->fCurrLength)") )
                {
                    __debugbreak();
                }
                if ( pPath->fCurrLength <= 0.0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                                3005,
                                0,
                                "%s",
                                "pPath->fCurrLength > 0") )
                {
                    __debugbreak();
                }
                if ( pt->fOrigLength < pPath->fCurrLength
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                                3006,
                                0,
                                "pt->fOrigLength >= pPath->fCurrLength\n\t%g, %g",
                                pt->fOrigLength,
                                pPath->fCurrLength) )
                {
                    __debugbreak();
                }
                Path_RemoveCompletedPathPoints(pPath, i);
            }
            else if ( i > pPath->wNegotiationStartNode )
            {
                ia = i - 1;
                pPath->vCurrPoint[0] = pt->vOrigPoint[0];
                pPath->vCurrPoint[1] = pt->vOrigPoint[1];
                pPath->vCurrPoint[2] = pt->vOrigPoint[2];
                pPath->fCurrLength = pPath->pts[ia].fOrigLength;
                Path_RemoveCompletedPathPoints(pPath, ia);
            }
            return;
        }
        bAtStart = 0;
    }
}

void __fastcall Path_BacktrackCompletedPath(path_t *pPath, const float *vStartPos)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    float fOrigLength; // [esp+18h] [ebp-6Ch]
    float d1; // [esp+48h] [ebp-3Ch]
    float dist; // [esp+4Ch] [ebp-38h]
    float height; // [esp+50h] [ebp-34h]
    float fraction; // [esp+54h] [ebp-30h]
    float offset; // [esp+58h] [ebp-2Ch]
    float offseta; // [esp+58h] [ebp-2Ch]
    float offset_4; // [esp+5Ch] [ebp-28h]
    float offset_4a; // [esp+5Ch] [ebp-28h]
    float d2; // [esp+60h] [ebp-24h]
    pathpoint_t *nextPt; // [esp+6Ch] [ebp-18h]
    float amount; // [esp+70h] [ebp-14h]
    float amounta; // [esp+70h] [ebp-14h]
    pathpoint_t *pt; // [esp+74h] [ebp-10h]
    float largestAmount; // [esp+78h] [ebp-Ch]
    int i; // [esp+7Ch] [ebp-8h]
    float amountInc; // [esp+80h] [ebp-4h]

    i = pPath->wPathLen - 1;
    if ( i <= pPath->wNegotiationStartNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3039,
                    0,
                    "%s",
                    "i > pPath->wNegotiationStartNode") )
    {
        __debugbreak();
    }
    largestAmount = pPath->fLookaheadAmount * 0.17647055;
    nextPt = &pPath->pts[i - 1];
    offset = nextPt->vOrigPoint[0] - *vStartPos;
    offset_4 = nextPt->vOrigPoint[1] - vStartPos[1];
    if ( nextPt->fDir2D[0] == 0.0
        && nextPt->fDir2D[1] == 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3045,
                    0,
                    "%s",
                    "nextPt->fDir2D[0] || nextPt->fDir2D[1]") )
    {
        __debugbreak();
    }
    height = fabs((float)(nextPt->fDir2D[0] * offset_4) - (float)(nextPt->fDir2D[1] * offset));
    (amount) = -(height * pPath->fCurrLength);
    if ( nextPt->fOrigLength < pPath->fCurrLength
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3050,
                    0,
                    "pPath->fCurrLength <= nextPt->fOrigLength\n\t%g, %g",
                    pPath->fCurrLength,
                    nextPt->fOrigLength) )
    {
        __debugbreak();
    }
    while ( 1 )
    {
        if ( i >= pPath->wOrigPathLen
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3054,
                        0,
                        "%s",
                        "i < pPath->wOrigPathLen") )
        {
            __debugbreak();
        }
        pt = &pPath->pts[i];
        offseta = pt->vOrigPoint[0] - *vStartPos;
        offset_4a = pt->vOrigPoint[1] - vStartPos[1];
        if ( (float)((float)(offseta * offseta) + (float)(offset_4a * offset_4a)) > 65536.0 )
        {
            pPath->flags |= 0x20u;
LABEL_127:
            pPath->vCurrPoint[0] = nextPt->vOrigPoint[0];
            pPath->vCurrPoint[1] = nextPt->vOrigPoint[1];
            pPath->vCurrPoint[2] = nextPt->vOrigPoint[2];
            if ( (LODWORD(nextPt->fOrigLength) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            3189,
                            0,
                            "%s",
                            "!IS_NAN(nextPt->fOrigLength)") )
            {
                __debugbreak();
            }
            if ( nextPt->fOrigLength <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            3190,
                            0,
                            "%s",
                            "nextPt->fOrigLength > 0") )
            {
                __debugbreak();
            }
            if ( i <= pPath->wNegotiationStartNode
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            3192,
                            0,
                            "%s",
                            "i > pPath->wNegotiationStartNode") )
            {
                __debugbreak();
            }
            if ( i <= pPath->wNegotiationStartNode + 1 )
                fOrigLength = 0.0f;
            else
                fOrigLength = pPath->pts[i - 2].fOrigLength;
            pPath->fCurrLength = fOrigLength;
            amounta = (float)(height * nextPt->fOrigLength) + amount;
            pPath->wPathLen = i;
            if ( pPath->wPathLen > 1 && pPath->pts[pPath->wPathLen - 2].fOrigLength < pPath->fCurrLength )
            {
                v6 = va(
                             "pPath->fCurrLength: %g, pPath->pts[pPath->wPathLen - 2].fOrigLength: %g",
                             pPath->fCurrLength,
                             pPath->pts[pPath->wPathLen - 2].fOrigLength);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                                3197,
                                0,
                                "%s\n\t%s",
                                "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                                v6) )
                    __debugbreak();
            }
            if ( Path_HasNegotiationNode(pPath)
                && (pPath->pts[pPath->wNegotiationStartNode].iNodeNum < 0
                 || pPath->pts[pPath->wNegotiationStartNode - 1].iNodeNum < 0)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            3198,
                            0,
                            "%s",
                            "!Path_HasNegotiationNode( pPath ) || (pPath->pts[pPath->wNegotiationStartNode].iNodeNum >= 0 && pPath->pts"
                            "[pPath->wNegotiationStartNode - 1].iNodeNum >= 0)") )
            {
                __debugbreak();
            }
            if ( amounta > 0.0 )
            {
                if ( (pPath->flags & 2) == 0 )
                    amounta = amounta * 0.75;
                pPath->fLookaheadAmount = pPath->fLookaheadAmount + amounta;
                if ( pPath->fLookaheadAmount > 65536.0 )
                    pPath->fLookaheadAmount = 65536.0f;
            }
            return;
        }
        d1 = (float)(pPath->lookaheadDir[0] * offseta) + (float)(pPath->lookaheadDir[1] * offset_4a);
        if ( d1 <= 0.0 )
            break;
        if ( nextPt->fOrigLength <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3069,
                        0,
                        "%s",
                        "nextPt->fOrigLength > 0") )
        {
            __debugbreak();
        }
        amountInc = height * nextPt->fOrigLength;
        amount = amount + amountInc;
        if ( amount >= largestAmount )
        {
            if ( largestAmount <= (float)(amount - amountInc)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            3075,
                            0,
                            "%s",
                            "amount - amountInc < largestAmount") )
            {
                __debugbreak();
            }
            if ( amountInc <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            3076,
                            0,
                            "%s",
                            "amountInc > 0") )
            {
                __debugbreak();
            }
            fraction = (float)(amount - largestAmount) / amountInc;
            if ( fraction < 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            3078,
                            0,
                            "%s",
                            "fraction >= 0") )
            {
                __debugbreak();
            }
            if ( fraction > 1.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            3079,
                            0,
                            "%s",
                            "fraction <= 1.f") )
            {
                __debugbreak();
            }
            amount = amount - amountInc;
            goto handleFraction;
        }
        if ( nextPt->fOrigLength <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3085,
                        0,
                        "%s",
                        "nextPt->fOrigLength > 0") )
        {
            __debugbreak();
        }
        if ( ++i > pPath->wOrigPathLen
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3088,
                        0,
                        "%s",
                        "i <= pPath->wOrigPathLen") )
        {
            __debugbreak();
        }
        if ( i >= pPath->wOrigPathLen )
        {
            pPath->fCurrLength = nextPt->fOrigLength;
            if ( (LODWORD(pPath->fCurrLength) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            3092,
                            0,
                            "%s",
                            "!IS_NAN(pPath->fCurrLength)") )
            {
                __debugbreak();
            }
            if ( pPath->fCurrLength <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            3093,
                            0,
                            "%s",
                            "pPath->fCurrLength > 0") )
            {
                __debugbreak();
            }
            pPath->vCurrPoint[0] = pt->vOrigPoint[0];
            pPath->vCurrPoint[1] = pt->vOrigPoint[1];
            pPath->vCurrPoint[2] = pt->vOrigPoint[2];
            pPath->wPathLen = pPath->wOrigPathLen;
            if ( pPath->wPathLen > 1 && pPath->pts[pPath->wPathLen - 2].fOrigLength < pPath->fCurrLength )
            {
                v2 = va(
                             "pPath->fCurrLength: %g, pPath->pts[pPath->wPathLen - 2].fOrigLength: %g",
                             pPath->fCurrLength,
                             pPath->pts[pPath->wPathLen - 2].fOrigLength);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                                3097,
                                0,
                                "%s\n\t%s",
                                "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                                v2) )
                    __debugbreak();
            }
            if ( Path_HasNegotiationNode(pPath)
                && (pPath->pts[pPath->wNegotiationStartNode].iNodeNum < 0
                 || pPath->pts[pPath->wNegotiationStartNode - 1].iNodeNum < 0)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            3098,
                            0,
                            "%s",
                            "!Path_HasNegotiationNode( pPath ) || (pPath->pts[pPath->wNegotiationStartNode].iNodeNum >= 0 && pPath->pts"
                            "[pPath->wNegotiationStartNode - 1].iNodeNum >= 0)") )
            {
                __debugbreak();
            }
            if ( amount > 0.0 )
            {
                if ( (pPath->flags & 2) == 0 )
                    amount = amount * 0.75;
                pPath->fLookaheadAmount = pPath->fLookaheadAmount + amount;
                if ( pPath->fLookaheadAmount > 65536.0 )
                    pPath->fLookaheadAmount = 65536.0;
            }
            return;
        }
        nextPt = pt;
        if ( pt->fDir2D[0] == 0.0
            && pt->fDir2D[1] == 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3113,
                        0,
                        "%s",
                        "nextPt->fDir2D[0] || nextPt->fDir2D[1]") )
        {
            __debugbreak();
        }
        height = fabs((float)(pt->fDir2D[0] * offset_4a) - (float)(pt->fDir2D[1] * offseta));
    }
    d2 = (float)(pPath->lookaheadDir[0] * (float)(nextPt->vOrigPoint[0] - *vStartPos))
         + (float)(pPath->lookaheadDir[1] * (float)(nextPt->vOrigPoint[1] - vStartPos[1]));
    if ( d2 <= 0.0 )
    {
        if ( d1 > 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 3183, 0, "%s", "d1 <= 0") )
        {
            __debugbreak();
        }
        goto LABEL_127;
    }
    if ( d1 > 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 3123, 0, "%s", "d1 <= 0") )
    {
        __debugbreak();
    }
    if ( (float)(d1 - d2) == 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 3124, 0, "%s", "d1 - d2") )
    {
        __debugbreak();
    }
    fraction = d1 / (float)(d1 - d2);
    if ( fraction < 0.0 )
    {
        v3 = va("i: %d, d1: %f, d2: %f, fraction: %f", i, d1, d2, fraction);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3128,
                        0,
                        "%s\n\t%s",
                        "fraction >= 0",
                        v3) )
            __debugbreak();
    }
    if ( fraction > 1.0 )
    {
        v4 = va("i: %d: d1: %f, d2: %f, fraction: %f", i, d1, d2, fraction);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3131,
                        0,
                        "%s\n\t%s",
                        "fraction <= 1.f",
                        v4) )
            __debugbreak();
    }
handleFraction:
    if ( nextPt->fOrigLength <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3134,
                    0,
                    "%s",
                    "nextPt->fOrigLength > 0") )
    {
        __debugbreak();
    }
    dist = fraction * nextPt->fOrigLength;
    if ( nextPt->fOrigLength < dist
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3137,
                    0,
                    "%s",
                    "dist <= nextPt->fOrigLength") )
    {
        __debugbreak();
    }
    if ( dist < nextPt->fOrigLength )
    {
        if ( dist < 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 3152, 0, "%s", "dist >= 0") )
        {
            __debugbreak();
        }
        if ( nextPt->fOrigLength <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3153,
                        0,
                        "%s",
                        "nextPt->fOrigLength > 0") )
        {
            __debugbreak();
        }
        pPath->fCurrLength = nextPt->fOrigLength - dist;
        if ( (LODWORD(pPath->fCurrLength) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3155,
                        0,
                        "%s",
                        "!IS_NAN(pPath->fCurrLength)") )
        {
            __debugbreak();
        }
        if ( pPath->fCurrLength <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3156,
                        0,
                        "%s",
                        "pPath->fCurrLength > 0") )
        {
            __debugbreak();
        }
        if ( nextPt->fOrigLength < pPath->fCurrLength
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3157,
                        0,
                        "nextPt->fOrigLength >= pPath->fCurrLength\n\t%g, %g",
                        nextPt->fOrigLength,
                        pPath->fCurrLength) )
        {
            __debugbreak();
        }
        amount = (float)(height * pPath->fCurrLength) + amount;
        if ( nextPt->fDir2D[0] == 0.0
            && nextPt->fDir2D[1] == 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3160,
                        0,
                        "%s",
                        "nextPt->fDir2D[0] || nextPt->fDir2D[1]") )
        {
            __debugbreak();
        }
        pPath->vCurrPoint[0] = (float)(dist * nextPt->fDir2D[0]) + pt->vOrigPoint[0];
        pPath->vCurrPoint[1] = (float)(dist * nextPt->fDir2D[1]) + pt->vOrigPoint[1];
        pPath->vCurrPoint[2] = (float)((float)(nextPt->vOrigPoint[2] - pt->vOrigPoint[2]) * fraction) + pt->vOrigPoint[2];
    }
    else
    {
        if ( --i <= pPath->wNegotiationStartNode )
            return;
        pPath->vCurrPoint[0] = nextPt->vOrigPoint[0];
        pPath->vCurrPoint[1] = nextPt->vOrigPoint[1];
        pPath->vCurrPoint[2] = nextPt->vOrigPoint[2];
        pPath->fCurrLength = pPath->pts[i - 1].fOrigLength;
    }
    pPath->wPathLen = i + 1;
    if ( pPath->wPathLen > 1 && pPath->pts[pPath->wPathLen - 2].fOrigLength < pPath->fCurrLength )
    {
        v5 = va(
                     "pPath->fCurrLength: %g, pPath->pts[pPath->wPathLen - 2].fOrigLength: %g",
                     pPath->fCurrLength,
                     pPath->pts[pPath->wPathLen - 2].fOrigLength);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3167,
                        0,
                        "%s\n\t%s",
                        "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                        v5) )
            __debugbreak();
    }
    if ( Path_HasNegotiationNode(pPath)
        && (pPath->pts[pPath->wNegotiationStartNode].iNodeNum < 0
         || pPath->pts[pPath->wNegotiationStartNode - 1].iNodeNum < 0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3168,
                    0,
                    "%s",
                    "!Path_HasNegotiationNode( pPath ) || (pPath->pts[pPath->wNegotiationStartNode].iNodeNum >= 0 && pPath->pts[pPa"
                    "th->wNegotiationStartNode - 1].iNodeNum >= 0)") )
    {
        __debugbreak();
    }
    if ( amount > 0.0 )
    {
        if ( (pPath->flags & 2) == 0 )
            amount = amount * 0.75;
        pPath->fLookaheadAmount = pPath->fLookaheadAmount + amount;
        if ( pPath->fLookaheadAmount > 65536.0 )
            pPath->fLookaheadAmount = 65536.0;
    }
}

void __fastcall Path_CalcLookahead(
                path_t *pPath,
                const float *vStartPos,
                int bReduceLookaheadAmount,
                int bAllowRestore)
{
    pathpoint_t *vCurrPoint; // [esp+10h] [ebp-58h]
    float fCurrLength; // [esp+14h] [ebp-54h]
    float vLookaheadPos[3]; // [esp+38h] [ebp-30h] BYREF
    float *prevPoint; // [esp+44h] [ebp-24h]
    float dist; // [esp+48h] [ebp-20h]
    float height; // [esp+4Ch] [ebp-1Ch]
    pathpoint_t *pt; // [esp+50h] [ebp-18h]
    float fLength; // [esp+54h] [ebp-14h]
    int bAtStart; // [esp+58h] [ebp-10h]
    float totalArea; // [esp+5Ch] [ebp-Ch]
    int i; // [esp+60h] [ebp-8h]
    float lookaheadAmount; // [esp+64h] [ebp-4h]

    iassert(pPath);
    iassert(pPath->wPathLen > 0);
    iassert(pPath->wNegotiationStartNode >= 0);
    iassert(pPath->wNegotiationStartNode < pPath->wPathLen);
    iassert(pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength);

    totalArea = 0.0f;
    lookaheadAmount = pPath->fLookaheadAmount;

    iassert(!IS_NAN(lookaheadAmount));
    iassert(lookaheadAmount > 0);

    bAtStart = 1;
    i = pPath->wPathLen - 2;
    while ( 1 )
    {
        if ( i < pPath->wNegotiationStartNode )
        {
            Path_CalcLookahead_Completed(pPath, vStartPos, bReduceLookaheadAmount, totalArea, bAllowRestore);
            return;
        }
        pt = &pPath->pts[i];
        nanassertvec3(pt->vOrigPoint);
        height = Path_GetDistToPathSegment(vStartPos, pt);
        if ( bAtStart )
            fCurrLength = pPath->fCurrLength;
        else
            fCurrLength = pt->fOrigLength;
        fLength = fCurrLength;
        iassert(fLength > 0);

        totalArea = (float)(height * fLength) + totalArea;
        if ( pPath->minLookAheadNodes == 2 )
            PathCalcLookahead_CheckMinLookaheadNodes(pPath, pt, i);
        if ( pPath->minLookAheadNodes + i <= pPath->wPathLen - 2 && totalArea >= lookaheadAmount )
            break;
        --i;
        bAtStart = 0;
    }
    dist = (float)(totalArea - lookaheadAmount) / height;
    iassert(fLength <= pt->fOrigLength);

    if ( dist > fLength )
        dist = fLength;
    vLookaheadPos[0] = (float)((-(dist)) * pt->fDir2D[0]) + pt->vOrigPoint[0];
    vLookaheadPos[1] = (float)((-(dist)) * pt->fDir2D[1]) + pt->vOrigPoint[1];
    if ( bAtStart )
        vCurrPoint = (pathpoint_t *)pPath->vCurrPoint;
    else
        vCurrPoint = pt + 1;
    prevPoint = (float *)vCurrPoint;
    vLookaheadPos[2] = pt->vOrigPoint[2]
                                     - (float)((float)((float)(pt->vOrigPoint[2] - vCurrPoint->vOrigPoint[2]) * dist) / fLength);
    pPath->flags &= 0xFFFFFFBE;
    Path_UpdateLookaheadAmount(
        pPath,
        (float*)vStartPos,
        vLookaheadPos,
        bReduceLookaheadAmount,
        dist,
        i,
        lookaheadAmount,
        bAllowRestore);
}

static float minLookaheadDist = 24.0f;
void __fastcall Path_UpdateLookaheadAmount(
                path_t *pPath,
                float *vStartPos,
                float *vLookaheadPos,
                int bReduceLookaheadAmount,
                float dist,
                int lookaheadNextNode,
                float maxLookaheadAmountIfReduce,
                int bAllowRestore)
{
    const char *v8; // eax
    float v9; // [esp+10h] [ebp-80h]
    float v10; // [esp+14h] [ebp-7Ch]
    float fOrigLength; // [esp+18h] [ebp-78h]
    bool v12; // [esp+1Ch] [ebp-74h]
    float fCurrLength; // [esp+20h] [ebp-70h]
    float v16; // [esp+50h] [ebp-40h]
    float prevHeight; // [esp+60h] [ebp-30h]
    float prevLookaheadAmount; // [esp+68h] [ebp-28h]
    float prevLookaheadPos[3]; // [esp+6Ch] [ebp-24h] BYREF
    float fLength; // [esp+78h] [ebp-18h]
    bool restorePrevLookahead; // [esp+7Fh] [ebp-11h]
    float prevLength; // [esp+80h] [ebp-10h]
    pathpoint_t *nextPathPt; // [esp+84h] [ebp-Ch]
    pathpoint_t *prevPathPt; // [esp+88h] [ebp-8h]
    bool lookaheadTrace; // [esp+8Fh] [ebp-1h]

    if ( pPath->wPathLen <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2671,
                    0,
                    "%s",
                    "pPath->wPathLen > 0") )
    {
        __debugbreak();
    }
    if ( pPath->lookaheadNextNode >= pPath->wPathLen - 1 )
    {
        pPath->fLookaheadDistToNextNode = 0.0f;
        pPath->lookaheadNextNode = pPath->wPathLen - 1;
    }
    prevLength = 0.0f;
    prevHeight = 0.0f;
    prevLookaheadAmount = pPath->fLookaheadAmount;
    restorePrevLookahead = 0;
    lookaheadTrace = 0;
    if ( bReduceLookaheadAmount )
    {
LABEL_7:
        Path_ReduceLookaheadAmount(pPath, maxLookaheadAmountIfReduce);
        goto LABEL_28;
    }
    if ( !Path_LookaheadPredictionTrace(pPath, vStartPos, vLookaheadPos) )
    {
        if ( pPath->lookaheadNextNode < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        2708,
                        0,
                        "%s",
                        "pPath->lookaheadNextNode >= 0") )
        {
            __debugbreak();
        }
        if ( pPath->lookaheadNextNode == pPath->wPathLen - 2 )
            fCurrLength = pPath->fCurrLength;
        else
            fCurrLength = pPath->pts[pPath->lookaheadNextNode].fOrigLength;
        fLength = fCurrLength;
        if ( (pPath->flags & 2) != 0
            && pPath->lookaheadNextNode < pPath->wPathLen
            && fLength >= pPath->fLookaheadDistToNextNode )
        {
            Path_ReduceLookaheadAmount(pPath, maxLookaheadAmountIfReduce);
            if ( pPath->lookaheadNextNode >= pPath->wPathLen
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            2714,
                            0,
                            "%s",
                            "pPath->lookaheadNextNode < pPath->wPathLen") )
            {
                __debugbreak();
            }
            if ( pPath->fLookaheadDistToNextNode != 0.0
                && pPath->lookaheadNextNode >= pPath->wPathLen - 1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            2715,
                            0,
                            "%s",
                            "!pPath->fLookaheadDistToNextNode || (pPath->lookaheadNextNode < pPath->wPathLen - 1)") )
            {
                __debugbreak();
            }
            return;
        }
        goto LABEL_7;
    }
    Path_IncreaseLookaheadAmount(pPath);
LABEL_28:
    v12 = lookaheadNextNode > pPath->lookaheadNextNode
         || lookaheadNextNode == pPath->lookaheadNextNode && dist > pPath->fLookaheadDistToNextNode;
    if ( ai_useBetterLookahead->current.enabled
        && !zombiemode->current.enabled
        && bAllowRestore
        && (prevLookaheadAmount > pPath->fLookaheadAmount || v12)
        && pPath->lookaheadNextNode <= pPath->wPathLen - 2
        && pPath->pts[pPath->lookaheadNextNode].fOrigLength >= pPath->fLookaheadDistToNextNode
        && pPath->wNegotiationStartNode <= 0
        && pPath->fLookaheadDist > minLookaheadDist )
    {
        nextPathPt = &pPath->pts[pPath->lookaheadNextNode];
        prevPathPt = &pPath->pts[pPath->lookaheadNextNode + 1];
        prevHeight = Path_GetDistToPathSegment(vStartPos, nextPathPt);
        if ( pPath->lookaheadNextNode == pPath->wPathLen - 2 )
            fOrigLength = pPath->fCurrLength;
        else
            fOrigLength = nextPathPt->fOrigLength;
        prevLength = fOrigLength;
        v16 = -pPath->fLookaheadDistToNextNode;
        prevLookaheadPos[0] = (float)(v16 * nextPathPt->fDir2D[0]) + nextPathPt->vOrigPoint[0];
        prevLookaheadPos[1] = (float)(v16 * nextPathPt->fDir2D[1]) + nextPathPt->vOrigPoint[1];
        prevLookaheadPos[2] = (float)(v16 * nextPathPt->fOrigLength) + nextPathPt->vOrigPoint[2];
        prevLookaheadPos[2] = nextPathPt->vOrigPoint[2]
                                                - (float)((float)((float)(nextPathPt->vOrigPoint[2] - prevPathPt->vOrigPoint[2])
                                                                                * pPath->fLookaheadDistToNextNode)
                                                                / fOrigLength);
        lookaheadTrace = Path_LookaheadPredictionTrace(pPath, vStartPos, prevLookaheadPos);
        if ( pPath->fLookaheadAmount < prevLookaheadAmount )
        {
            if ( !lookaheadTrace )
            {
                Path_SetLookaheadToStart(pPath, vStartPos, 0);
                return;
            }
            restorePrevLookahead = 1;
        }
        else
        {
            restorePrevLookahead = lookaheadTrace;
        }
    }
    if ( restorePrevLookahead )
    {
        pPath->lookaheadDir[0] = prevLookaheadPos[0] - *vStartPos;
        pPath->lookaheadDir[1] = prevLookaheadPos[1] - vStartPos[1];
        pPath->lookaheadDir[2] = prevLookaheadPos[2] - vStartPos[2];
        pPath->fLookaheadDist = Vec2Normalize(pPath->lookaheadDir);
        if ( pPath->fLookaheadDist == 0.0 )
            v10 = 0.0f;
        else
            v10 = pPath->lookaheadDir[2] / pPath->fLookaheadDist;
        pPath->lookaheadDir[2] = v10;
        if ( pPath->lookaheadNextNode != lookaheadNextNode || pPath->fLookaheadAmount >= prevLookaheadAmount )
            pPath->fLookaheadAmount = prevHeight * prevLength;
    }
    else
    {
        pPath->lookaheadDir[0] = *vLookaheadPos - *vStartPos;
        pPath->lookaheadDir[1] = vLookaheadPos[1] - vStartPos[1];
        pPath->lookaheadDir[2] = vLookaheadPos[2] - vStartPos[2];
        pPath->fLookaheadDist = Vec2Normalize(pPath->lookaheadDir);
        if ( pPath->fLookaheadDist == 0.0 )
            v9 = 0.0f;
        else
            v9 = pPath->lookaheadDir[2] / pPath->fLookaheadDist;
        pPath->lookaheadDir[2] = v9;
        pPath->fLookaheadDistToNextNode = dist;
        pPath->lookaheadNextNode = lookaheadNextNode;
    }
    if ( pPath->fLookaheadAmount <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2826,
                    0,
                    "%s",
                    "pPath->fLookaheadAmount > 0") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode > pPath->lookaheadNextNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2827,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode") )
    {
        __debugbreak();
    }
    if ( pPath->lookaheadNextNode >= pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2828,
                    0,
                    "%s",
                    "pPath->lookaheadNextNode < pPath->wPathLen") )
    {
        __debugbreak();
    }
    if ( pPath->pts[pPath->lookaheadNextNode].fOrigLength <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2829,
                    0,
                    "%s",
                    "pPath->pts[pPath->lookaheadNextNode].fOrigLength > 0") )
    {
        __debugbreak();
    }
    if ( pPath->pts[pPath->lookaheadNextNode].fOrigLength < pPath->fLookaheadDistToNextNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2830,
                    0,
                    "%s",
                    "pPath->fLookaheadDistToNextNode <= pPath->pts[pPath->lookaheadNextNode].fOrigLength") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen > 1 && pPath->pts[pPath->wPathLen - 2].fOrigLength < pPath->fCurrLength )
    {
        v8 = va(
                     "pPath->fCurrLength: %g, pPath->pts[pPath->wPathLen - 2].fOrigLength: %g",
                     pPath->fCurrLength,
                     pPath->pts[pPath->wPathLen - 2].fOrigLength);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        2831,
                        0,
                        "%s\n\t%s",
                        "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                        v8) )
            __debugbreak();
    }
    if ( pPath->fLookaheadDistToNextNode != 0.0
        && pPath->lookaheadNextNode >= pPath->wPathLen - 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2832,
                    0,
                    "%s",
                    "!pPath->fLookaheadDistToNextNode || (pPath->lookaheadNextNode < pPath->wPathLen - 1)") )
    {
        __debugbreak();
    }
    if ( pPath->lookaheadNextNode >= pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2833,
                    0,
                    "%s",
                    "pPath->lookaheadNextNode < pPath->wPathLen") )
    {
        __debugbreak();
    }
    if ( pPath->fLookaheadDistToNextNode != 0.0
        && pPath->lookaheadNextNode >= pPath->wPathLen - 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    2834,
                    0,
                    "%s",
                    "!pPath->fLookaheadDistToNextNode || (pPath->lookaheadNextNode < pPath->wPathLen - 1)") )
    {
        __debugbreak();
    }
}

void __fastcall Path_ReduceLookaheadAmount(path_t *pPath, float maxLookaheadAmountIfReduce)
{
    pPath->numIncreases = 0;
    ++pPath->numReductions;
    if ( (pPath->flags & 2) != 0 )
        pPath->fLookaheadAmount = maxLookaheadAmountIfReduce * 0.75;
    else
        pPath->fLookaheadAmount = maxLookaheadAmountIfReduce * 0.5625;
    if ( pPath->fLookaheadAmount < 0.001 )
        pPath->fLookaheadAmount = 0.001f;
    pPath->flags &= 0xFFFFFDFC;
}

static float momentumFactor = 1.5f;
void __fastcall Path_IncreaseLookaheadAmount(path_t *pPath)
{
    int v1; // eax
    float v3; // [esp+Ch] [ebp-14h]

    pPath->numReductions = 0;
    ++pPath->numIncreases;
    pPath->fLookaheadAmount = pPath->fLookaheadAmount * 1.1764705;
    if ( ai_useBetterLookahead->current.enabled && !zombiemode->current.enabled )
    {
        v3 = _Pow_int<float>(momentumFactor, pPath->numIncreases - 1);
        pPath->fLookaheadAmount = pPath->fLookaheadAmount * v3;
    }
    pPath->fLookaheadAmount = pPath->fLookaheadAmount + 6.4000001;
    if ( pPath->fLookaheadAmount > 65536.0 )
        pPath->fLookaheadAmount = 65536.0;
    if ( (pPath->flags & 2) != 0 )
        v1 = pPath->flags | 0x200;
    else
        v1 = pPath->flags | 2;
    pPath->flags = v1;
}

void __fastcall Path_CalcLookahead_Completed(
                path_t *pPath,
                const float *vStartPos,
                int bReduceLookaheadAmount,
                float totalArea,
                int bAllowRestore)
{
    float fLookaheadAmount; // [esp+10h] [ebp-1Ch]
    float v6; // [esp+14h] [ebp-18h]
    float *vLookaheadPos; // [esp+28h] [ebp-4h]

    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 3225, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode >= pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3226,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode < pPath->wPathLen") )
    {
        __debugbreak();
    }
    pPath->flags |= 0x41u;
    if ( pPath->wPathLen - 1 == pPath->wNegotiationStartNode )
        vLookaheadPos = pPath->vCurrPoint;
    else
        vLookaheadPos = pPath->pts[pPath->wNegotiationStartNode].vOrigPoint;
    Path_UpdateLookaheadAmount(
        pPath,
        (float*)vStartPos,
        vLookaheadPos,
        bReduceLookaheadAmount,
        0.0,
        pPath->wNegotiationStartNode,
        totalArea,
        bAllowRestore);

    if ( (float)(totalArea - 21845.334) < 0.0 )
        v6 = 21845.334f;
    else
        v6 = totalArea;
    if ( (float)(v6 - pPath->fLookaheadAmount) < 0.0 )
        fLookaheadAmount = v6;
    else
        fLookaheadAmount = pPath->fLookaheadAmount;

    pPath->fLookaheadAmount = fLookaheadAmount;
}

void __fastcall PathCalcLookahead_CheckMinLookaheadNodes(path_t *pPath, const pathpoint_t *pt, int currentNode)
{
    int v3; // ecx
    float n2n4[3]; // [esp+2Ch] [ebp-14h] BYREF
    float cos30; // [esp+38h] [ebp-8h]
    float dot; // [esp+3Ch] [ebp-4h]

    cos30 = 0.866f;
    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 3269, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( currentNode + 3 == pPath->wPathLen )
    {
        if ( currentNode >= pPath->wPathLen - 2
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3274,
                        0,
                        "%s",
                        "currentNode < pPath->wPathLen - 2") )
        {
            __debugbreak();
        }
        v3 = currentNode - 1;
        n2n4[0] = pPath->pts[v3].vOrigPoint[0] - pPath->pts[currentNode + 1].vOrigPoint[0];
        n2n4[1] = pPath->pts[v3].vOrigPoint[1] - pPath->pts[currentNode + 1].vOrigPoint[1];
        Vec2Normalize(n2n4);
        dot = (float)(pPath->pts[currentNode - 1].fDir2D[0] * n2n4[0])
                + (float)(pPath->pts[currentNode - 1].fDir2D[1] * n2n4[1]);
        if ( cos30 <= dot )
        {
            dot = (float)(pPath->pts[currentNode - 1].fDir2D[0] * pPath->pts[currentNode].fDir2D[0])
                    + (float)(pPath->pts[currentNode - 1].fDir2D[1] * pPath->pts[currentNode].fDir2D[1]);
            if ( dot >= 0.17299999 )
                ++pPath->minLookAheadNodes;
        }
    }
}

void __fastcall Path_UpdateForwardLookahead(path_t *pPath, const float *vStartPos)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    float *vOrigPoint; // [esp+38h] [ebp-70h]
    float lookaheadAmount; // [esp+60h] [ebp-48h]
    float d1; // [esp+64h] [ebp-44h]
    float vForwardStartPos[2]; // [esp+68h] [ebp-40h] BYREF
    float dist; // [esp+70h] [ebp-38h]
    float height; // [esp+74h] [ebp-34h]
    float fraction; // [esp+78h] [ebp-30h]
    float offset[2]; // [esp+7Ch] [ebp-2Ch]
    int bAtStart; // [esp+84h] [ebp-24h]
    float d2; // [esp+88h] [ebp-20h]
    pathpoint_t *prevPt; // [esp+8Ch] [ebp-1Ch]
    float totalArea; // [esp+90h] [ebp-18h]
    pathpoint_t *pt; // [esp+94h] [ebp-14h]
    float fLength; // [esp+98h] [ebp-10h]
    int i; // [esp+9Ch] [ebp-Ch]
    float prevOffset[2]; // [esp+A0h] [ebp-8h]

    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 3447, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3448,
                    0,
                    "%s",
                    "pPath->wPathLen > 0") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3449,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode >= 0") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode > pPath->lookaheadNextNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3450,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode") )
    {
        __debugbreak();
    }
    if ( pPath->lookaheadNextNode >= pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3451,
                    0,
                    "%s",
                    "pPath->lookaheadNextNode < pPath->wPathLen") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen > 1 && pPath->pts[pPath->wPathLen - 2].fOrigLength < pPath->fCurrLength )
    {
        v2 = va(
                     "pPath->fCurrLength: %g, pPath->pts[pPath->wPathLen - 2].fOrigLength: %g",
                     pPath->fCurrLength,
                     pPath->pts[pPath->wPathLen - 2].fOrigLength);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3452,
                        0,
                        "%s\n\t%s",
                        "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                        v2) )
            __debugbreak();
    }
    if ( (pPath->flags & 0x200) != 0 && Path_GetForwardStartPos(pPath, vStartPos, vForwardStartPos) )
    {
        prevOffset[0] = pPath->vCurrPoint[0] - vForwardStartPos[0];
        prevOffset[1] = pPath->vCurrPoint[1] - vForwardStartPos[1];
        if ( (float)((float)(pPath->lookaheadDir[0] * prevOffset[0]) + (float)(pPath->lookaheadDir[1] * prevOffset[1])) <= 0.0 )
        {
            bAtStart = 1;
            i = pPath->wPathLen - 2;
            while ( 1 )
            {
                if ( i < pPath->wNegotiationStartNode )
                {
                    pPath->forwardLookaheadDir2D[0] = pPath->lookaheadDir[0];
                    pPath->forwardLookaheadDir2D[1] = pPath->lookaheadDir[1];
                    return;
                }
                pt = &pPath->pts[i];
                offset[0] = pt->vOrigPoint[0] - vForwardStartPos[0];
                offset[1] = pt->vOrigPoint[1] - vForwardStartPos[1];
                d2 = (float)(pPath->lookaheadDir[0] * offset[0]) + (float)(pPath->lookaheadDir[1] * offset[1]);
                if ( d2 > 0.0 )
                    break;
                --i;
                bAtStart = 0;
            }
            prevPt = &pPath->pts[i + 1];
            if ( bAtStart )
            {
                prevOffset[0] = pPath->vCurrPoint[0] - vForwardStartPos[0];
                prevOffset[1] = pPath->vCurrPoint[1] - vForwardStartPos[1];
                fLength = pPath->fCurrLength;
            }
            else
            {
                prevOffset[0] = prevPt->vOrigPoint[0] - vForwardStartPos[0];
                prevOffset[1] = prevPt->vOrigPoint[1] - vForwardStartPos[1];
                fLength = pt->fOrigLength;
            }
            d1 = (float)(pPath->lookaheadDir[0] * prevOffset[0]) + (float)(pPath->lookaheadDir[1] * prevOffset[1]);
            if ( d1 > 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            3491,
                            0,
                            "%s\n\t(d1) = %g",
                            "(d1 <= 0)",
                            d1) )
            {
                __debugbreak();
            }
            if ( d2 <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            3492,
                            0,
                            "%s\n\t(d2) = %g",
                            "(d2 > 0)",
                            d2) )
            {
                __debugbreak();
            }
            if ( d1 - d2 == 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            3493,
                            0,
                            "%s\n\t(d1 - d2) = %g",
                            "(d1 - d2)",
                            d1 - d2) )
            {
                __debugbreak();
            }
            fraction = d1 / (d1 - d2);
            if ( fraction < 0.0 )
            {
                v3 = va("i: %d, pPath->wPathLen: %d, d1: %f, d2: %f, fraction: %f", i, pPath->wPathLen, d1, d2, fraction);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                                3497,
                                0,
                                "%s\n\t%s",
                                "fraction >= 0",
                                v3) )
                    __debugbreak();
            }
            if ( fraction > 1.0 )
            {
                v4 = va("i: %d, pPath->wPathLen: %d, d1: %f, d2: %f, fraction: %f", i, pPath->wPathLen, d1, d2, fraction);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                                3500,
                                0,
                                "%s\n\t%s",
                                "fraction <= 1.f",
                                v4) )
                    __debugbreak();
            }
            if ( fLength <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            3501,
                            0,
                            "%s\n\t(fLength) = %g",
                            "(fLength > 0)",
                            fLength) )
            {
                __debugbreak();
            }
            dist = fraction * fLength;
            lookaheadAmount = pPath->fLookaheadAmount;
            if ( lookaheadAmount <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            3506,
                            0,
                            "%s\n\t(lookaheadAmount) = %g",
                            "(lookaheadAmount > 0)",
                            lookaheadAmount) )
            {
                __debugbreak();
            }
            height = Path_GetDistToPathSegment(vForwardStartPos, pt);
            totalArea = (float)(fLength - dist) * height;
            if ( totalArea < lookaheadAmount )
            {
                while ( i > pPath->wNegotiationStartNode )
                {
                    pt = &pPath->pts[--i];
                    if ( pt->fOrigLength <= 0.0
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                                    3521,
                                    0,
                                    "%s\n\t(pt->fOrigLength) = %g",
                                    "(pt->fOrigLength > 0)",
                                    pt->fOrigLength) )
                    {
                        __debugbreak();
                    }
                    height = Path_GetDistToPathSegment(vForwardStartPos, pt);
                    totalArea = (float)(height * pt->fOrigLength) + totalArea;
                    if ( totalArea >= lookaheadAmount )
                        goto LABEL_54;
                }
                vOrigPoint = pPath->pts[pPath->wNegotiationStartNode].vOrigPoint;
                pPath->forwardLookaheadDir2D[0] = *vOrigPoint - vForwardStartPos[0];
                pPath->forwardLookaheadDir2D[1] = vOrigPoint[1] - vForwardStartPos[1];
                Vec2Normalize(pPath->forwardLookaheadDir2D);
            }
            else
            {
LABEL_54:
                Path_UpdateForwardLookahead_IncompletePath(pPath, pt, vForwardStartPos, totalArea - lookaheadAmount, height);
            }
        }
        else
        {
            pPath->forwardLookaheadDir2D[0] = pPath->lookaheadDir[0];
            pPath->forwardLookaheadDir2D[1] = pPath->lookaheadDir[1];
        }
    }
    else
    {
        pPath->forwardLookaheadDir2D[0] = pPath->lookaheadDir[0];
        pPath->forwardLookaheadDir2D[1] = pPath->lookaheadDir[1];
    }
}

void __fastcall Path_UpdateForwardLookahead_IncompletePath(
                path_t *pPath,
                const pathpoint_t *pt,
                const float *vForwardStartPos,
                float area,
                float height)
{
    float dist; // [esp+34h] [ebp-4h]

    if ( (LODWORD(height) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3412,
                    0,
                    "%s",
                    "!IS_NAN(height)") )
    {
        __debugbreak();
    }
    if ( height == 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3413,
                    0,
                    "%s\n\t(height) = %g",
                    "(height)",
                    height) )
    {
        __debugbreak();
    }
    dist = area / height;
    if ( pt->fDir2D[0] == 0.0
        && pt->fDir2D[1] == 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3416,
                    0,
                    "%s",
                    "pt->fDir2D[0] || pt->fDir2D[1]") )
    {
        __debugbreak();
    }
    pPath->forwardLookaheadDir2D[0] = (float)((-(dist)) * pt->fDir2D[0]) + pt->vOrigPoint[0];
    pPath->forwardLookaheadDir2D[1] = (float)((-(dist)) * pt->fDir2D[1]) + pt->vOrigPoint[1];
    pPath->forwardLookaheadDir2D[0] = pPath->forwardLookaheadDir2D[0] - *vForwardStartPos;
    pPath->forwardLookaheadDir2D[1] = pPath->forwardLookaheadDir2D[1] - vForwardStartPos[1];
    Vec2Normalize(pPath->forwardLookaheadDir2D);
}

void __fastcall Path_DebugDraw(path_t *pPath, float *vStartPos, int bDrawLookahead, int entId)
{
    float fLookaheadDist; // [esp+14h] [ebp-24h]
    float v2[3]; // [esp+1Ch] [ebp-1Ch] BYREF
    float v1[3]; // [esp+28h] [ebp-10h] BYREF
    int i; // [esp+34h] [ebp-4h]

    if ( pPath->wPathLen )
    {
        if ( bDrawLookahead )
        {
            v1[0] = *vStartPos;
            v1[1] = vStartPos[1];
            v1[2] = vStartPos[2];
            fLookaheadDist = pPath->fLookaheadDist;
            v2[0] = fLookaheadDist * pPath->lookaheadDir[0];
            v2[1] = fLookaheadDist * pPath->lookaheadDir[1];
            v2[2] = fLookaheadDist * pPath->lookaheadDir[2];
            v2[0] = v2[0] + *vStartPos;
            v2[1] = v2[1] + vStartPos[1];
            v2[2] = v2[2] + vStartPos[2];
            v1[2] = v1[2] + 16.0;
            v2[2] = v2[2] + 16.0;
            G_DebugLine(v1, v2, colorRed, 0);
        }
        i = pPath->wPathLen;
        v1[0] = *vStartPos;
        v1[1] = vStartPos[1];
        v1[2] = vStartPos[2];
        v1[2] = v1[2] + 16.0;
        if ( !i && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 3706, 0, "%s", "i") )
            __debugbreak();
        --i;
        v2[0] = pPath->vCurrPoint[0];
        v2[1] = pPath->vCurrPoint[1];
        v2[2] = pPath->vCurrPoint[2];
        v2[2] = v2[2] + 16.0;
        if ( i == pPath->wNegotiationStartNode - 1 )
            G_DebugLine(v1, v2, colorCyan, 0);
        else
            G_DebugLine(v1, v2, colorBlue, 0);
        v1[0] = v2[0];
        v1[1] = v2[1];
        v1[2] = v2[2];
        while ( i )
        {
            v2[0] = pPath->pts[--i].vOrigPoint[0];
            v2[1] = pPath->pts[i].vOrigPoint[1];
            v2[2] = pPath->pts[i].vOrigPoint[2];
            v2[2] = v2[2] + 16.0;
            if ( i == pPath->wNegotiationStartNode - 1 )
                G_DebugLine(v1, v2, colorCyan, 0);
            else
                G_DebugLine(v1, v2, colorBlue, 0);
            v1[0] = v2[0];
            v1[1] = v2[1];
            v1[2] = v2[2];
        }
        for ( i = pPath->wPathLen - 1; i >= 0; --i )
        {
            v1[0] = pPath->pts[i].vOrigPoint[0];
            v1[1] = pPath->pts[i].vOrigPoint[1];
            v1[2] = pPath->pts[i].vOrigPoint[2];
            v1[2] = v1[2] + 16.0;
        }
    }
}

bool __fastcall Path_WithinApproxDist(path_t *pPath, float checkDist)
{
    float fDist; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 3765, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( !pPath->wPathLen )
        return 1;
    if ( pPath->wNegotiationStartNode < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3770,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode >= 0") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode > pPath->lookaheadNextNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3771,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode") )
    {
        __debugbreak();
    }
    if ( pPath->lookaheadNextNode >= pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3772,
                    0,
                    "%s",
                    "pPath->lookaheadNextNode < pPath->wPathLen") )
    {
        __debugbreak();
    }
    fDist = pPath->fLookaheadDist + pPath->fLookaheadDistToNextNode;
    if ( pPath->lookaheadNextNode == pPath->wNegotiationStartNode )
        return checkDist > fDist;
    i = pPath->lookaheadNextNode - 1;
    if ( i == pPath->wPathLen - 2 )
    {
        if ( pPath->fCurrLength <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3783,
                        0,
                        "%s",
                        "pPath->fCurrLength > 0") )
        {
            __debugbreak();
        }
        fDist = fDist + pPath->fCurrLength;
        --i;
    }
    if ( fDist >= checkDist )
        return 0;
    while ( i >= pPath->wNegotiationStartNode )
    {
        if ( pPath->pts[i].fOrigLength <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3793,
                        0,
                        "%s",
                        "pPath->pts[i].fOrigLength > 0") )
        {
            __debugbreak();
        }
        fDist = fDist + pPath->pts[i].fOrigLength;
        if ( fDist >= checkDist )
            return 0;
        --i;
    }
    return 1;
}

ai_stance_e __fastcall Path_AllowedStancesForPath(path_t *pPath)
{
    const char *v1; // eax
    pathnode_t *v2; // eax
    pathnode_t *v3; // eax
    pathnode_t *v5; // eax
    pathnode_t *v6; // eax
    float *vOrigPoint; // [esp+18h] [ebp-Ch]
    int i; // [esp+20h] [ebp-4h]

    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 3814, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3815,
                    0,
                    "%s",
                    "pPath->wPathLen > 0") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3816,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode >= 0") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode > pPath->lookaheadNextNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3817,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode") )
    {
        __debugbreak();
    }
    if ( pPath->lookaheadNextNode >= pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3818,
                    0,
                    "%s",
                    "pPath->lookaheadNextNode < pPath->wPathLen") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen > 1 && pPath->pts[pPath->wPathLen - 2].fOrigLength < pPath->fCurrLength )
    {
        v1 = va(
                     "pPath->fCurrLength: %g, pPath->pts[pPath->wPathLen - 2].fOrigLength: %g",
                     pPath->fCurrLength,
                     pPath->pts[pPath->wPathLen - 2].fOrigLength);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3819,
                        0,
                        "%s\n\t%s",
                        "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                        v1) )
            __debugbreak();
    }
    i = pPath->wPathLen - 1;
    if ( i < pPath->wNegotiationStartNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3822,
                    0,
                    "%s",
                    "i >= pPath->wNegotiationStartNode") )
    {
        __debugbreak();
    }
    vOrigPoint = pPath->pts[i].vOrigPoint;
    if ( pPath->vCurrPoint[0] != *vOrigPoint
        || pPath->vCurrPoint[1] != vOrigPoint[1]
        || pPath->vCurrPoint[2] != vOrigPoint[2] )
    {
        --i;
    }
    while ( i >= pPath->wNegotiationStartNode )
    {
        if ( pPath->pts[i].iNodeNum >= 0 )
        {
            v2 = Path_ConvertIndexToNode(pPath->pts[i].iNodeNum);
            if ( Path_IsPathStanceNode(v2) )
                break;
        }
        --i;
    }
    if ( i < pPath->wNegotiationStartNode )
    {
        i = pPath->wPathLen - 1;
        if ( pPath->pts[i].iNodeNum < 0 )
            return STANCE_ANY;
        v3 = Path_ConvertIndexToNode(pPath->pts[i].iNodeNum);
        if ( !Path_IsPathStanceNode(v3) )
            return STANCE_ANY;
    }
    if ( pPath->pts[i].iNodeNum < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3837,
                    0,
                    "%s",
                    "pPath->pts[i].iNodeNum >= 0") )
    {
        __debugbreak();
    }
    v5 = Path_ConvertIndexToNode(pPath->pts[i].iNodeNum);
    if ( !Path_IsPathStanceNode(v5)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3838,
                    0,
                    "%s",
                    "Path_IsPathStanceNode( Path_ConvertIndexToNode( pPath->pts[i].iNodeNum ) )") )
    {
        __debugbreak();
    }
    v6 = Path_ConvertIndexToNode(pPath->pts[i].iNodeNum);
    return Path_AllowedStancesForNode(v6);
}

bool __cdecl Path_IsPathStanceNode(const pathnode_t *node)
{
    if ( !node && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.h", 226, 0, "%s", "node") )
        __debugbreak();
    return ((1 << node->constant.type) & 0x183FFC) == 0;
}

char __fastcall Path_AttemptDodge(
                path_t *pPath,
                const float *vOrg,
                const float *vDodgeStart,
                const float *vDodgeEnd,
                int startIndex,
                const int *entities,
                int entityCount,
                int entityIgnore,
                int mask,
                int bCheckLookahead)
{
    double PathDir; // st7
    double v12; // st7
    double v13; // st7
    const char *v14; // eax
    pathpoint_t *v17; // [esp+1Ch] [ebp-98h]
    pathpoint_t *v18; // [esp+24h] [ebp-90h]
    pathpoint_t *v19; // [esp+34h] [ebp-80h]
    float fLookaheadDist; // [esp+4Ch] [ebp-68h]
    float v21; // [esp+64h] [ebp-50h]
    float vNewDodgeEnd[3]; // [esp+68h] [ebp-4Ch] BYREF
    PredictionTraceResult result; // [esp+74h] [ebp-40h]
    float vTraceEndPos[3]; // [esp+78h] [ebp-3Ch] BYREF
    float dist; // [esp+84h] [ebp-30h]
    float vEnd[3]; // [esp+88h] [ebp-2Ch] BYREF
    float vNewDodgeStart[3]; // [esp+94h] [ebp-20h] BYREF
    int maxIndex; // [esp+A0h] [ebp-14h]
    pathpoint_t *pt; // [esp+A4h] [ebp-10h] BYREF
    float dot; // [esp+A8h] [ebp-Ch]
    float vDelta[2]; // [esp+ACh] [ebp-8h] BYREF

    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 3900, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3901,
                    0,
                    "%s",
                    "pPath->wPathLen > 0") )
    {
        __debugbreak();
    }
    if ( *vDodgeStart == *vDodgeEnd
        && vDodgeStart[1] == vDodgeEnd[1]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3902,
                    0,
                    "%s",
                    "!(vDodgeStart[0] == vDodgeEnd[0] && vDodgeStart[1] == vDodgeEnd[1])") )
    {
        __debugbreak();
    }
    if ( startIndex > pPath->lookaheadNextNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3903,
                    0,
                    "%s",
                    "startIndex <= pPath->lookaheadNextNode") )
    {
        __debugbreak();
    }
    if ( pPath->lookaheadNextNode >= pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    3904,
                    0,
                    "%s",
                    "pPath->lookaheadNextNode < pPath->wPathLen") )
    {
        __debugbreak();
    }
    Path_PredictionTraceCheckForEntities((float*)vOrg, (float *)vDodgeStart, entities, entityCount, entityIgnore, mask, vNewDodgeStart);
    Path_PredictionTraceCheckForEntities(
        vNewDodgeStart,
        (float *)vDodgeEnd,
        entities,
        entityCount,
        entityIgnore,
        mask,
        vNewDodgeEnd);
    pt = &pPath->pts[startIndex];
    if ( ai_showDodge->current.enabled )
    {
        CG_DebugLine(vDodgeStart, vNewDodgeStart, colorYellow, 0, 25);
        CG_DebugLine(vNewDodgeStart, vNewDodgeEnd, colorYellow, 0, 25);
        CG_DebugLine(vNewDodgeEnd, pt->vOrigPoint, colorYellow, 0, 25);
    }
    if ( startIndex < pPath->wNegotiationStartNode )
        goto fail_2;
    if ( Path_PredictionTraceCheckForEntities(
                 vNewDodgeEnd,
                 pt->vOrigPoint,
                 entities,
                 entityCount,
                 entityIgnore,
                 mask,
                 vTraceEndPos) == PTR_SUCCESS )
        goto LABEL_29;
    maxIndex = pPath->wPathLen - 2;
    if ( pPath->wDodgeCount > 0 )
        maxIndex -= pPath->wDodgeCount;
    if ( startIndex >= maxIndex )
        goto fail_2;
    do
    {
        pt = &pPath->pts[++startIndex + 1];
        v21 = vOrg[1] - pt->vOrigPoint[1];
    }
    while ( (float)((float)((float)(*vOrg - pt->vOrigPoint[0]) * (float)(*vOrg - pt->vOrigPoint[0])) + (float)(v21 * v21)) >= 1406.25
             && startIndex + 1 < maxIndex );
    pt = &pPath->pts[startIndex];

    if ( ai_showDodge->current.enabled )
        CG_DebugLine(vNewDodgeEnd, pt->vOrigPoint, colorMagenta, 0, 25);

    if ( Path_PredictionTraceCheckForEntities(
                 vNewDodgeEnd,
                 pt->vOrigPoint,
                 entities,
                 entityCount,
                 entityIgnore,
                 mask,
                 vTraceEndPos) )
    {
fail_2:
        if ( !bCheckLookahead )
            return 0;
        vDelta[0] = vNewDodgeEnd[0] - vNewDodgeStart[0];
        vDelta[1] = vNewDodgeEnd[1] - vNewDodgeStart[1];
        if ( Vec2Normalize(vDelta) > 15.0 )
        {
            vNewDodgeEnd[0] = (float)(-15.0 * vDelta[0]) + vNewDodgeEnd[0];
            vNewDodgeEnd[1] = (float)(-15.0 * vDelta[1]) + vNewDodgeEnd[1];
        }
        else
        {
            vNewDodgeEnd[0] = vNewDodgeStart[0];
            vNewDodgeEnd[1] = vNewDodgeStart[1];
            vNewDodgeEnd[2] = vNewDodgeStart[2];
        }
        startIndex = pPath->lookaheadNextNode;
        if ( pPath->wNegotiationStartNode < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3969,
                        0,
                        "%s",
                        "pPath->wNegotiationStartNode >= 0") )
        {
            __debugbreak();
        }
        if ( pPath->wNegotiationStartNode > startIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3970,
                        0,
                        "%s",
                        "pPath->wNegotiationStartNode <= startIndex") )
        {
            __debugbreak();
        }
        if ( startIndex >= pPath->wPathLen - 1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3971,
                        0,
                        "%s",
                        "startIndex < pPath->wPathLen - 1") )
        {
            __debugbreak();
        }
        pt = &pPath->pts[startIndex];
        fLookaheadDist = pPath->fLookaheadDist;
        vEnd[0] = (float)(fLookaheadDist * pPath->lookaheadDir[0]) + *vOrg;
        vEnd[1] = (float)(fLookaheadDist * pPath->lookaheadDir[1]) + vOrg[1];
        vEnd[2] = pt->vOrigPoint[2];
        if ( ai_showDodge->current.enabled )
            CG_DebugLine(vNewDodgeEnd, vEnd, colorRed, 0, 25);
        result = (PredictionTraceResult)Path_PredictionTraceCheckForEntities(
                             vNewDodgeEnd,
                             vEnd,
                             entities,
                             entityCount,
                             entityIgnore,
                             mask,
                             vTraceEndPos);
        if ( result == PTR_HIT_WORLD )
            return 0;
        if ( result == PTR_HIT_ENTITY )
        {
            vEnd[0] = vNewDodgeEnd[0] - *vOrg;
            vEnd[1] = vNewDodgeEnd[1] - vOrg[1];
            dot = (float)(vEnd[0] * pPath->lookaheadDir[0]) + (float)(vEnd[1] * pPath->lookaheadDir[1]);
            vEnd[0] = (float)((float)(dot + 15.0) * pPath->lookaheadDir[0]) + *vOrg;
            vEnd[1] = (float)((float)(dot + 15.0) * pPath->lookaheadDir[1]) + vOrg[1];
            if ( ai_showDodge->current.enabled )
                CG_DebugLine(vNewDodgeEnd, vEnd, colorCyan, 0, 25);
            if ( Path_PredictionTraceCheckForEntities(
                         vNewDodgeEnd,
                         vEnd,
                         entities,
                         entityCount,
                         entityIgnore,
                         mask,
                         vTraceEndPos) )
            {
                return 0;
            }
            if ( pPath->wNegotiationStartNode > startIndex
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            4002,
                            0,
                            "%s",
                            "pPath->wNegotiationStartNode <= startIndex") )
            {
                __debugbreak();
            }
            if ( ai_showDodge->current.enabled )
                CG_DebugLine(vEnd, pt->vOrigPoint, colorCyan, 0, 25);
            if ( Path_PredictionTraceCheckForEntities(
                         vEnd,
                         pt->vOrigPoint,
                         entities,
                         entityCount,
                         entityIgnore,
                         mask,
                         vTraceEndPos) )
            {
                return 0;
            }
            Path_CheckNodeCountForDodge(pPath, 3, &pt, &startIndex);
            PathDir = Path_GetPathDir(pt->fDir2D, vEnd, pt->vOrigPoint);
            pt->fOrigLength = PathDir;
            if ( (LODWORD(pt->fOrigLength) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            4015,
                            0,
                            "%s",
                            "!IS_NAN(pt->fOrigLength)") )
            {
                __debugbreak();
            }
            ++pt;
            ++startIndex;
            v19 = pt;
            pt->vOrigPoint[0] = vEnd[0];
            v19->vOrigPoint[1] = vEnd[1];
            v19->vOrigPoint[2] = vEnd[2];
            pt->iNodeNum = -1;
            Path_DodgeDrawRaisedLine(pt[-1].vOrigPoint, pt->vOrigPoint, colorBlue);
        }
        else
        {
            if ( ai_showDodge->current.enabled )
                CG_DebugLine(vEnd, pt->vOrigPoint, colorMdGrey, 0, 25);
            if ( Path_PredictionTraceCheckForEntities(
                         vEnd,
                         pt->vOrigPoint,
                         entities,
                         entityCount,
                         entityIgnore,
                         mask,
                         vTraceEndPos) )
            {
                return 0;
            }
            Path_CheckNodeCountForDodge(pPath, 3, &pt, &startIndex);
            dist = pPath->fLookaheadDistToNextNode;
            if ( dist <= 0.0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 4041, 0, "%s", "dist > 0") )
            {
                __debugbreak();
            }
            if ( (LODWORD(pt->fOrigLength) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            4043,
                            0,
                            "%s",
                            "!IS_NAN(pt->fOrigLength)") )
            {
                __debugbreak();
            }
            if ( pt->fOrigLength <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            4044,
                            0,
                            "%s",
                            "pt->fOrigLength > 0") )
            {
                __debugbreak();
            }
            if ( pt->fDir2D[0] == 0.0
                && pt->fDir2D[1] == 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            4045,
                            0,
                            "%s",
                            "pt->fDir2D[0] || pt->fDir2D[1]") )
            {
                __debugbreak();
            }
            if ( pt->fOrigLength < dist
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            4046,
                            0,
                            "%s",
                            "dist <= pt->fOrigLength") )
            {
                __debugbreak();
            }
            pt[1].vOrigPoint[0] = pt->vOrigPoint[0] - (float)(pt->fDir2D[0] * dist);
            pt[1].vOrigPoint[1] = pt->vOrigPoint[1] - (float)(pt->fDir2D[1] * dist);
            pt[1].vOrigPoint[2] = pt->vOrigPoint[2]
                                                    - (float)((float)((float)(pt->vOrigPoint[2] - pt[1].vOrigPoint[2]) * dist) / pt->fOrigLength);
            pt->fOrigLength = dist;
            if ( (LODWORD(pt->fOrigLength) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            4053,
                            0,
                            "%s",
                            "!IS_NAN(pt->fOrigLength)") )
            {
                __debugbreak();
            }
            if ( pt->fOrigLength <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                            4054,
                            0,
                            "%s",
                            "pt->fOrigLength > 0") )
            {
                __debugbreak();
            }
            ++pt;
            pPath->lookaheadNextNode = ++startIndex;
            pPath->fLookaheadDistToNextNode = 0.0f;
            pt->iNodeNum = -1;
            Path_DodgeDrawRaisedLine(pt[-1].vOrigPoint, pt->vOrigPoint, colorGreen);
        }
    }
    else
    {
LABEL_29:
        Path_CheckNodeCountForDodge(pPath, 2, &pt, &startIndex);
    }
    if ( pt != &pPath->pts[startIndex]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    4070,
                    0,
                    "%s",
                    "pt == &pPath->pts[startIndex]") )
    {
        __debugbreak();
    }
    pPath->wDodgeCount = 0;
    if ( pt->vOrigPoint[0] != vNewDodgeEnd[0] || pt->vOrigPoint[1] != vNewDodgeEnd[1] )
    {
        v12 = Path_GetPathDir(pt->fDir2D, vNewDodgeEnd, pt->vOrigPoint);
        pt->fOrigLength = v12;
        if ( (LODWORD(pt->fOrigLength) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        4076,
                        0,
                        "%s",
                        "!IS_NAN(pt->fOrigLength)") )
        {
            __debugbreak();
        }
        ++pt;
        ++startIndex;
        ++pPath->wDodgeCount;
        v18 = pt;
        pt->vOrigPoint[0] = vNewDodgeEnd[0];
        v18->vOrigPoint[1] = vNewDodgeEnd[1];
        v18->vOrigPoint[2] = vNewDodgeEnd[2];
        pt->iNodeNum = -1;
        Path_DodgeDrawRaisedLine(pt[-1].vOrigPoint, pt->vOrigPoint, colorLtGreen);
    }
    if ( vNewDodgeStart[0] != vNewDodgeEnd[0] || vNewDodgeStart[1] != vNewDodgeEnd[1] )
    {
        v13 = Path_GetPathDir(pt->fDir2D, vNewDodgeStart, vNewDodgeEnd);
        pt->fOrigLength = v13;
        if ( (LODWORD(pt->fOrigLength) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        4093,
                        0,
                        "%s",
                        "!IS_NAN(pt->fOrigLength)") )
        {
            __debugbreak();
        }
        ++pt;
        ++startIndex;
        ++pPath->wDodgeCount;
        v17 = pt;
        pt->vOrigPoint[0] = vNewDodgeStart[0];
        v17->vOrigPoint[1] = vNewDodgeStart[1];
        v17->vOrigPoint[2] = vNewDodgeStart[2];
        pt->iNodeNum = -1;
        Path_DodgeDrawRaisedLine(pt[-1].vOrigPoint, pt->vOrigPoint, colorMdYellow);
    }
    pPath->fCurrLength = pt[-1].fOrigLength;
    pPath->vCurrPoint[0] = vNewDodgeStart[0];
    pPath->vCurrPoint[1] = vNewDodgeStart[1];
    pPath->vCurrPoint[2] = vNewDodgeStart[2];
    pt->fOrigLength = 0.0f;
    pt->fDir2D[0] = 0.0f;
    pt->fDir2D[1] = 0.0f;
    pPath->flags &= 0xFFFFFDBC;
    pPath->wPathLen = startIndex + 1;
    pPath->wOrigPathLen = pPath->wPathLen;
    Path_TransferLookahead(pPath, vOrg);
    if ( pPath->wNegotiationStartNode > pPath->lookaheadNextNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    4121,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode") )
    {
        __debugbreak();
    }
    if ( pPath->lookaheadNextNode >= pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    4122,
                    0,
                    "%s",
                    "pPath->lookaheadNextNode < pPath->wPathLen") )
    {
        __debugbreak();
    }
    if ( pPath->pts[pPath->lookaheadNextNode].fOrigLength < pPath->fLookaheadDistToNextNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    4123,
                    0,
                    "%s",
                    "pPath->fLookaheadDistToNextNode <= pPath->pts[pPath->lookaheadNextNode].fOrigLength") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen > 1 && pPath->pts[pPath->wPathLen - 2].fOrigLength < pPath->fCurrLength )
    {
        v14 = va(
                        "pPath->fCurrLength: %g, pPath->pts[pPath->wPathLen - 2].fOrigLength: %g",
                        pPath->fCurrLength,
                        pPath->pts[pPath->wPathLen - 2].fOrigLength);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        4124,
                        0,
                        "%s\n\t%s",
                        "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                        v14) )
            __debugbreak();
    }
    if ( pPath->wPathLen
        && pPath->wNegotiationStartNode
        && (pPath->pts[pPath->wNegotiationStartNode].iNodeNum < 0
         || pPath->pts[pPath->wNegotiationStartNode - 1].iNodeNum < 0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    4125,
                    0,
                    "%s",
                    "!pPath->wPathLen || !pPath->wNegotiationStartNode || (pPath->pts[pPath->wNegotiationStartNode].iNodeNum >= 0 &"
                    "& pPath->pts[pPath->wNegotiationStartNode - 1].iNodeNum >= 0)") )
    {
        __debugbreak();
    }
    if ( pPath->fLookaheadDistToNextNode != 0.0
        && pPath->lookaheadNextNode >= pPath->wPathLen - 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    4126,
                    0,
                    "%s",
                    "!pPath->fLookaheadDistToNextNode || (pPath->lookaheadNextNode < pPath->wPathLen - 1)") )
    {
        __debugbreak();
    }
    return 1;
}

void __cdecl Path_CheckNodeCountForDodge(path_t *pPath, int numNeeded, pathpoint_t **pt, int *startIndex)
{
    int iNodeCount; // [esp+0h] [ebp-4h]

    iNodeCount = *startIndex + 1 - (32 - numNeeded);
    if ( iNodeCount > 0 )
    {
        Path_TrimLastNodes(pPath, iNodeCount, 0);
        *pt -= iNodeCount;
        *startIndex -= iNodeCount;
        if ( pPath->wNegotiationStartNode < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3854,
                        0,
                        "%s",
                        "pPath->wNegotiationStartNode >= 0") )
        {
            __debugbreak();
        }
        if ( pPath->wNegotiationStartNode > *startIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        3855,
                        0,
                        "%s",
                        "pPath->wNegotiationStartNode <= *startIndex") )
        {
            __debugbreak();
        }
    }
}

void __cdecl Path_DodgeDrawRaisedLine(const float *start, const float *end, const float *color)
{
    float startRaised[3]; // [esp+0h] [ebp-18h] BYREF
    float endRaised[3]; // [esp+Ch] [ebp-Ch] BYREF

    if ( ai_showDodge->current.enabled )
    {
        startRaised[0] = *start;
        startRaised[1] = start[1];
        startRaised[2] = start[2];
        endRaised[0] = *end;
        endRaised[1] = end[1];
        endRaised[2] = end[2];
        startRaised[2] = startRaised[2] + 50.0;
        endRaised[2] = endRaised[2] + 50.0;
        CG_DebugLine(startRaised, endRaised, color, 0, 25);
    }
}

bool __fastcall Path_MayFaceEnemy(path_t *pPath, float *vEnemyDir, float *vOrg)
{
    const char *v3; // eax
    float fLerp; // [esp+1Ch] [ebp-10h]

    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 4145, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    4146,
                    0,
                    "%s",
                    "pPath->wPathLen > 0") )
    {
        __debugbreak();
    }
    if ( !vEnemyDir
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 4147, 0, "%s", "vEnemyDir") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    4148,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode >= 0") )
    {
        __debugbreak();
    }
    if ( pPath->wNegotiationStartNode > pPath->lookaheadNextNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    4149,
                    0,
                    "%s",
                    "pPath->wNegotiationStartNode <= pPath->lookaheadNextNode") )
    {
        __debugbreak();
    }
    if ( pPath->lookaheadNextNode >= pPath->wPathLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                    4150,
                    0,
                    "%s",
                    "pPath->lookaheadNextNode < pPath->wPathLen") )
    {
        __debugbreak();
    }
    if ( pPath->wPathLen > 1 && pPath->pts[pPath->wPathLen - 2].fOrigLength < pPath->fCurrLength )
    {
        v3 = va(
                     "pPath->fCurrLength: %g, pPath->pts[pPath->wPathLen - 2].fOrigLength: %g",
                     pPath->fCurrLength,
                     pPath->pts[pPath->wPathLen - 2].fOrigLength);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp",
                        4151,
                        0,
                        "%s\n\t%s",
                        "pPath->wPathLen <= 1 || pPath->fCurrLength <= pPath->pts[pPath->wPathLen - 2].fOrigLength",
                        v3) )
            __debugbreak();
    }
    if ( level.time - pPath->iPathTime < 300 )
        return 1;
    if ( !Path_DistanceGreaterThan(pPath, 128.0) )
        return 1;
    fLerp = (float)((float)((float)(pPath->lookaheadDir[0] * *vEnemyDir) + (float)(pPath->lookaheadDir[1] * vEnemyDir[1]))
                                + 1.0)
                * 0.5;
    return (float)((float)(fLerp * -48.0) + 72.0) <= pPath->fLookaheadDist
            && (float)(level.time - pPath->iPathTime) < (float)((float)(fLerp * 3500.0) + 1500.0);
}

void __fastcall Path_TrimToBadPlaceLink(path_t *pPath, team_t eTeam)
{
    int i; // [esp+8h] [ebp-4h]

    if ( !pPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_navigation.cpp", 4203, 0, "%s", "pPath") )
    {
        __debugbreak();
    }
    for ( i = pPath->wPathLen - 2; i >= 0; --i )
    {
        if ( pPath->pts[i].iNodeNum >= 0
            && pPath->pts[i + 1].iNodeNum >= 0
            && (eTeam & 6) != 0
            && Path_IsBadPlaceLink(pPath->pts[i + 1].iNodeNum, pPath->pts[i].iNodeNum, eTeam) )
        {
            Path_TrimLastNodes(pPath, i, 1);
            return;
        }
    }
}

char __thiscall CustomSearchInfo_FindCloseNode::IgnoreNode(pathnode_t *pNode)
{
    float v3; // [esp+Ch] [ebp-Ch]
    float heightDist; // [esp+10h] [ebp-8h]
    float distSq; // [esp+14h] [ebp-4h]

    v3 = this->goalPos[1] - pNode->constant.vOrigin[1];
    distSq = (float)((float)(this->goalPos[0] - pNode->constant.vOrigin[0])
                                 * (float)(this->goalPos[0] - pNode->constant.vOrigin[0]))
                 + (float)(v3 * v3);
    if ( distSq >= this->closestDistSq )
        return 1;
    heightDist = pNode->constant.vOrigin[2] - this->goalPos[2];
    if ( (float)(heightDist * heightDist) > 6400.0 )
        return 1;
    this->closestDistSq = distSq;
    this->closestNode = pNode;
    return 0;
}

double __thiscall CustomSearchInfo_FindPath::EvaluateHeuristic(
                pathnode_t *pSuccessor,
                const float *vGoalPos)
{
    float v[2]; // [esp+18h] [ebp-Ch] BYREF
    float dist; // [esp+20h] [ebp-4h]

    v[0] = *vGoalPos - pSuccessor->constant.vOrigin[0];
    v[1] = vGoalPos[1] - pSuccessor->constant.vOrigin[1];
    dist = Vec2Length(v);
    dist = (float)((float)pSuccessor->dynamic.userCount * this->negotiationOverlapCost) + dist;
    if ( pSuccessor->constant.minUseDistSq > 1.0
        && (double)pSuccessor->constant.minUseDistSq > Vec3DistanceSq(pSuccessor->constant.vOrigin, this->startPos) )
    {
        return (float)(dist + this->negotiationOverlapCost);
    }
    return dist;
}

double  CustomSearchInfo_FindPathWithWidth::EvaluateHeuristic(
                pathnode_t *pSuccessor,
                const float *vGoalPos)
{
#if 0
    double v4; // xmm0_8
    long double v6; // [esp-18h] [ebp-3Ch]
    float v7; // [esp+8h] [ebp-1Ch] BYREF
    float dist; // [esp+Ch] [ebp-18h]
    float *vOrigin; // [esp+10h] [ebp-14h]
    float delta[3]; // [esp+14h] [ebp-10h]
    float retaddr; // [esp+24h] [ebp+0h]

    delta[1] = a2;
    delta[2] = retaddr;
    LODWORD(delta[0]) = this;
    vOrigin = pSuccessor->constant.vOrigin;
    v7 = pSuccessor->constant.vOrigin[0] - *vGoalPos;
    dist = pSuccessor->constant.vOrigin[1] - vGoalPos[1];
    v4 = (float)(fabs((float)((float)(this->perp[0] * v7) + (float)(this->perp[1] * dist)) - this->width)
                         * 0.0069077001);
    __libm_sse2_exp(v6);
    *(float *)&v4 = v4;
    pSuccessor->transient.costFactor = *(float *)&v4;
    return Vec2Length(&v7) * *(float *)&v4;
#endif

    // aislop
    // 
    // Compute delta vector from successor to goal
    float dx = pSuccessor->constant.vOrigin[0] - vGoalPos[0];
    float dy = pSuccessor->constant.vOrigin[1] - vGoalPos[1];

    // Project delta onto the "perp" vector, subtract width, scale
    float proj = (perp[0] * dx + perp[1] * dy) - width;
    float factor = expf(fabsf(proj) * 0.0069077001f); // approximate exponential decay

    // Store factor in successor costFactor
    pSuccessor->transient.costFactor = factor;

    // Return 2D distance to goal multiplied by the factor
    return Vec2Length(&dx) * factor;
}

double __thiscall CustomSearchInfo_FindPathWithLOS::EvaluateHeuristic(
                pathnode_t *pSuccessor,
                const float *vGoalPos)
{
    float v[2]; // [esp+18h] [ebp-Ch] BYREF
    float dist; // [esp+20h] [ebp-4h]

    v[0] = *vGoalPos - pSuccessor->constant.vOrigin[0];
    v[1] = vGoalPos[1] - pSuccessor->constant.vOrigin[1];
    dist = Vec2Length(v);
    dist = (float)((float)pSuccessor->dynamic.userCount * this->negotiationOverlapCost) + dist;
    if ( pSuccessor->constant.minUseDistSq > 1.0
        && pSuccessor->constant.minUseDistSq > Vec3DistanceSq(pSuccessor->constant.vOrigin, this->startPos) )
    {
        return (float)(dist + this->negotiationOverlapCost);
    }
    return dist;
}

double __thiscall CustomSearchInfo_FindPathClosestPossible::EvaluateHeuristic(
                pathnode_t *pSuccessor,
                const float *vGoalPos)
{
    float v[2]; // [esp+Ch] [ebp-Ch] BYREF
    float dist; // [esp+14h] [ebp-4h]

    v[0] = *vGoalPos - pSuccessor->constant.vOrigin[0];
    v[1] = vGoalPos[1] - pSuccessor->constant.vOrigin[1];
    dist = Vec2Length(v);
    return (float)((float)((float)pSuccessor->dynamic.userCount * this->negotiationOverlapCost) + dist);
}

