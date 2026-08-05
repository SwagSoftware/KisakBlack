#include "actor_badplace.h"
#include <universal/assertive.h>
#include "pathnode.h"
#include <game_mp/actor_mp.h>
#include <game_mp/g_main_mp.h>
#include <clientscript/cscr_stringlist.h>
#include <clientscript/cscr_vm.h>
#include "actor_events.h"
#include "g_debug.h"
#include <game_mp/g_utils_mp.h>
#include <server/sv_game.h>
#include "actor_state.h"
#include "actor_orientation.h"
#include "actor_team_move.h"
#include "actor_animapi.h"

#include <algorithm>

struct _pendedBadPlace // sizeof=0x2C
{                                       // XREF: .data:_pendedBadPlace * gPendendBadPlaces/r
    badplace_parms_t params;
    unsigned int name;
    int duration;
    int teamflags;
    int type;
};

char gFreePendingBadPlaces[256];
badplace_t g_badplaces[256];
_pendedBadPlace gPendendBadPlaces[256];
int gPendedBadPlaceCount;

void __cdecl Path_UpdateBadPlaceCount(badplace_t *place, int delta)
{
    const char *v2; // eax
    int savedregs; // [esp+4h] [ebp+0h] BYREF

    if ( !place && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp", 62, 0, "%s", "place") )
        __debugbreak();
    if ( delta != -1
        && delta != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp",
                    63,
                    0,
                    "%s\n\t(delta) = %i",
                    "(delta == -1 || delta == +1)",
                    delta) )
    {
        __debugbreak();
    }
    switch ( place->type )
    {
        case 1u:
        case 3u:
            Path_UpdateArcBadPlaceCount(&place->parms.arc, place->teamflags, delta);
            break;
        case 2u:
            Path_UpdateBrushBadPlaceCount(place->parms.brush.volume, place->teamflags, delta);
            break;
        case 4u:
            Path_UpdateLimitedDepthArcBadPlaceCount(
                &place->parms.arc,
                place->teamflags,
                delta,
                place->depth);
            break;
        default:
            v2 = va("unhandled bad place type %i", place->type);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp", 81, 0, v2) )
                __debugbreak();
            break;
    }
}

void __cdecl Path_UpdateBadPlaces()
{
    int i; // [esp+0h] [ebp-Ch]
    int ia; // [esp+0h] [ebp-Ch]
    int ib; // [esp+0h] [ebp-Ch]
    bool updateFlee; // [esp+7h] [ebp-5h]
    _pendedBadPlace *pendedBP; // [esp+8h] [ebp-4h]

    updateFlee = 0;
    for (i = 0; i < 256; ++i)
    {
        if (gFreePendingBadPlaces[i])
        {
            Path_UpdateBadPlaceCount(&g_badplaces[i], -1);
            g_badplaces[i].type = 0;
            Scr_SetString(&g_badplaces[i].name, 0, SCRIPTINSTANCE_SERVER);
            updateFlee = 1;
        }
    }
    memset(gFreePendingBadPlaces, 0, sizeof(gFreePendingBadPlaces));
    pendedBP = gPendendBadPlaces;
    for (ia = 0; ia < gPendedBadPlaceCount; ++ia)
    {
        if (pendedBP->type != 3
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp",
                167,
                0,
                "%s",
                "pendedBP->type == BADPLACE_REALLY_BAD"))
        {
            __debugbreak();
        }
        Path_MakeBadPlaceEx(pendedBP->name, pendedBP->duration, pendedBP->teamflags, pendedBP->type, &pendedBP->params);
        ++pendedBP;
    }
    if (gPendedBadPlaceCount)
    {
        Actor_BadPlacesChanged();
        gPendedBadPlaceCount = 0;
    }
    for (ib = 0; ib < 256; ++ib)
    {
        if (g_badplaces[ib].type && g_badplaces[ib].endtime < level.time)
            Path_FreeBadPlace(ib);
    }
    if (updateFlee)
        Actor_BadPlace_UpdateFleeingActors();
}

void __cdecl Path_FreeBadPlace(int index)
{
    if ( index >= 0 )
        gFreePendingBadPlaces[index] = 1;
}

int __cdecl Path_FindBadPlace(unsigned int name)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 256; ++i)
    {
        if (g_badplaces[i].name == name)
            return i;
    }
    return -1;
}

void __cdecl Path_MakeBadPlaceEx(unsigned int name, int duration, int teamflags, int type, badplace_parms_t *parms)
{
    badplace_t *place; // [esp+8h] [ebp-4h]

    if ( ai_enableBadPlaces->current.enabled )
    {
        if ( !parms
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp", 293, 0, "%s", "parms") )
        {
            __debugbreak();
        }
        if ( (teamflags <= 0 || teamflags >= 16)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp",
                        294,
                        0,
                        "%s\n\t(teamflags) = %i",
                        "(teamflags > 0 && teamflags < (1 << (sizeof( ((pathlink_t *) 0)->ubBadPlaceCount ) / (sizeof( ((pathlink_t *"
                        ") 0)->ubBadPlaceCount[0] ) * (sizeof( ((pathlink_t *) 0)->ubBadPlaceCount ) != 4 || sizeof( ((pathlink_t *) "
                        "0)->ubBadPlaceCount[0] ) <= 4)))))",
                        teamflags) )
        {
            __debugbreak();
        }
        if ( teamflags != (unsigned __int8)teamflags
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp",
                        295,
                        0,
                        "%s\n\t(teamflags) = %i",
                        "(teamflags == (byte) teamflags)",
                        teamflags) )
        {
            __debugbreak();
        }
        if ( type != (unsigned __int8)type
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp",
                        296,
                        0,
                        "%s\n\t(type) = %i",
                        "(type == (byte) type)",
                        type) )
        {
            __debugbreak();
        }
        place = Path_AllocBadPlace(name, duration);
        if ( place )
        {
            place->teamflags = teamflags;
            place->type = type;
            memcpy(&place->parms, parms, sizeof(place->parms));
            place->pingTime = level.time;
            place->depth = 10;
            Path_UpdateBadPlaceCount(place, 1);
        }
    }
}

badplace_t *__cdecl Path_AllocBadPlace(unsigned int name, int duration)
{
    int BadPlace; // eax
    int endtime; // [esp+10h] [ebp-10h]
    int lowest; // [esp+14h] [ebp-Ch]
    int best; // [esp+18h] [ebp-8h]
    int i; // [esp+1Ch] [ebp-4h]
    int ia; // [esp+1Ch] [ebp-4h]

    if (name)
    {
        BadPlace = Path_FindBadPlace(name);
        Path_FreeBadPlace(BadPlace);
    }
    endtime = duration + level.time;
    if (duration > 0)
    {
        if (duration < 250)
            Com_PrintWarning(
                18,
                "WARNING: A badplace was created with duration [%.2f second], which is less than the ping time [%.2f second]\n",
                (float)((float)duration * 0.001),
                0.25);
    }
    else
    {
        if (!name)
        {
            Scr_Error("anonymous bad places must have a duration", 0);
            return 0;
        }
        endtime = 0x7FFFFFFF;
    }
    for (i = 0; i < 256; ++i)
    {
        if (!g_badplaces[i].type)
        {
            Scr_SetString(&g_badplaces[i].name, name, SCRIPTINSTANCE_SERVER);
            g_badplaces[i].endtime = endtime;
            return &g_badplaces[i];
        }
    }
    best = 0;
    lowest = 0x7FFFFFFF;
    for (ia = 0; ia < 256; ++ia)
    {
        if (g_badplaces[ia].endtime - level.time < lowest && g_badplaces[ia].type == 3)
        {
            best = ia;
            lowest = g_badplaces[ia].endtime - level.time;
        }
    }
    Path_FreeBadPlace(best);
    Scr_SetString(&g_badplaces[best].name, name, SCRIPTINSTANCE_SERVER);
    g_badplaces[best].endtime = endtime;
    return &g_badplaces[best];
}

void __cdecl Path_DrawBadPlaces()
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 0x100; ++i)
    {
        if (g_badplaces[i].type)
            Path_DrawBadPlace(&g_badplaces[i]);
    }
}

void __cdecl Path_DrawBadPlace(badplace_t *place)
{
    const char *eax10; // eax
    float v2; // [esp+24h] [ebp-38h]
    float v0[3]; // [esp+28h] [ebp-34h] BYREF
    float dir1[3]; // [esp+34h] [ebp-28h] BYREF
    float radius; // [esp+40h] [ebp-1Ch]
    float v1[3]; // [esp+44h] [ebp-18h] BYREF
    float dir0[3]; // [esp+50h] [ebp-Ch] BYREF

    switch ( place->type )
    {
        case 1u:
        case 3u:
        case 4u:
            if ( (float)(place->parms.arc.radius * place->parms.arc.radius) == 0.0 )
            {
                v2 = Actor_EventDefaultRadiusSqrd(AI_EV_BADPLACE_ARC);
                radius = sqrtf(v2);
            }
            else
            {
                radius = place->parms.arc.radius;
            }
            if ( place->parms.arc.angle0 == 0.0 && place->parms.arc.angle1 == 360.0 )
            {
                *(_QWORD *)v0 = *(_QWORD *)place->parms.arc.origin;
                v0[2] = place->parms.arc.origin[2];
                G_DebugCircle(v0, radius, colorRed, 1, 1, 0);
                v0[2] = v0[2] - place->parms.arc.halfheight;
                G_DebugCircle(v0, radius, colorLtOrange, 1, 1, 0);
                v0[2] = (float)(place->parms.arc.halfheight * 2.0) + v0[2];
                G_DebugCircle(v0, radius, colorLtOrange, 1, 1, 0);
            }
            else
            {
                YawVectors(place->parms.arc.angle0, dir0, 0);
                YawVectors(place->parms.arc.angle1, dir1, 0);
                *(_QWORD *)v0 = *(_QWORD *)place->parms.arc.origin;
                v0[2] = place->parms.arc.origin[2];
                v0[2] = v0[2] - place->parms.arc.halfheight;
                G_DebugArc(v0, radius, place->parms.arc.angle0, place->parms.arc.angle1, colorRed, 1, 0);
                v1[0] = (float)(radius * dir0[0]) + v0[0];
                v1[1] = (float)(radius * dir0[1]) + v0[1];
                v1[2] = (float)(radius * dir0[2]) + v0[2];

                G_DebugLine(v0, v1, colorRed, 1);
                v1[0] = (float)(radius * dir1[0]) + v0[0];
                v1[1] = (float)(radius * dir1[1]) + v0[1];
                v1[2] = (float)(radius * dir1[2]) + v0[2];
                G_DebugLine(v0, v1, colorRed, 1);
                v0[2] = (float)(place->parms.arc.halfheight * 2.0) + v0[2];
                v1[0] = (float)(radius * dir0[0]) + v0[0];
                v1[1] = (float)(radius * dir0[1]) + v0[1];
                v1[2] = (float)(radius * dir0[2]) + v0[2];
                G_DebugArc(v0, radius, place->parms.arc.angle0, place->parms.arc.angle1, colorRed, 1, 0);
                v1[0] = (float)(radius * dir0[0]) + v0[0];
                v1[1] = (float)(radius * dir0[1]) + v0[1];
                v1[2] = (float)(radius * dir0[2]) + v0[2];
                G_DebugLine(v0, v1, colorRed, 1);
                v1[0] = (float)(radius * dir1[0]) + v0[0];
                v1[1] = (float)(radius * dir1[1]) + v0[1];
                v1[2] = (float)(radius * dir1[2]) + v0[2];
                G_DebugLine(v0, v1, colorRed, 1);
            }
            break;
        case 2u:
            G_DebugDrawBrushModel(place->parms.brush.volume, colorRed, 1, 0);
            break;
        default:
            eax10 = va("unhandled bad place type %i", place->type);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp", 478, 0, eax10) )
                __debugbreak();
            break;
    }
}

void __cdecl Path_InitBadPlaces()
{
    memset(g_badplaces, 0, sizeof(g_badplaces));
}

void __cdecl Path_ShutdownBadPlaces()
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 0x100; ++i)
        Scr_SetString(&g_badplaces[i].name, 0, SCRIPTINSTANCE_SERVER);
    memset(g_badplaces, 0, sizeof(g_badplaces));
}

bool __cdecl Actor_IsInAnyBadPlace(actor_s *self)
{
    bool result; // al
    const char *v2; // eax
    unsigned int badPlaceIndex; // [esp+14h] [ebp-8h]

    if ((float)G_rand() > (float)(32767.0 * self->badPlaceAwareness))
        return 0;
    for (badPlaceIndex = 0; ; ++badPlaceIndex)
    {
        if (badPlaceIndex >= 0x100)
            return 0;
        if (g_badplaces[badPlaceIndex].type)
            break;
    LABEL_4:
        ;
    }
    switch (g_badplaces[badPlaceIndex].type)
    {
    case 1u:
    case 3u:
    case 4u:
        if (!Actor_IsInsideArc(
            self,
            g_badplaces[badPlaceIndex].parms.arc.origin,
            g_badplaces[badPlaceIndex].parms.arc.radius,
            g_badplaces[badPlaceIndex].parms.arc.angle0,
            g_badplaces[badPlaceIndex].parms.arc.angle1,
            g_badplaces[badPlaceIndex].parms.arc.halfheight))
            goto LABEL_4;
        result = 1;
        break;
    case 2u:
        if (!SV_EntityContact(self->ent->r.mins, self->ent->r.maxs, g_badplaces[badPlaceIndex].parms.brush.volume))
            goto LABEL_4;
        result = 1;
        break;
    default:
        v2 = va("unhandled bad place type %i", g_badplaces[badPlaceIndex].type);
        if (!Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp", 637, 0, v2))
            __debugbreak();
        goto LABEL_4;
    }
    return result;
}

actor_s *Actor_BadPlace_UpdateFleeingActors()
{
    actor_s *result; // eax
    actor_s *actor; // [esp+0h] [ebp-4h]

    result = Actor_FirstActor(-1);
    for ( actor = result; actor; actor = result )
    {
        if ( actor->eState[actor->stateLevel] == AIS_BADPLACE_FLEE && !Actor_IsInAnyBadPlace(actor) )
            actor->aiBadPlace = AI_BADPLACE_NONE;
        result = Actor_NextActor(actor, -1);
    }
    return result;
}

bool __fastcall Actor_BadPlace_Flee_Start(actor_s *self, ai_state_t ePrevState)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp", 995, 0, "%s", "self") )
        __debugbreak();
    if ( !Actor_BadPlace_AttemptEscape(self) )
        self->aiBadPlace = AI_BADPLACE_NONE;
    return 1;
}

char __cdecl Actor_BadPlace_AttemptEscape(actor_s *self)
{
    pathsort_t potentialNodes[256]; // [esp+4h] [ebp-C10h] BYREF
    float maxFleeDist; // [esp+C04h] [ebp-10h]
    int potentialNodeIndex; // [esp+C08h] [ebp-Ch]
    pathnode_t *safeNode; // [esp+C0Ch] [ebp-8h]
    int potentialNodeCount; // [esp+C10h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp", 927, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp", 928, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    maxFleeDist = Actor_BadPlace_GetMaximumFleeRadius();
    if ( maxFleeDist < 0.0 )
        return 0;
    maxFleeDist = maxFleeDist + 100.0;
    safeNode = Actor_BadPlace_FindSafeNodeAlongPath(self);
    if ( safeNode && Actor_FindPathToNode(self, safeNode, 1) )
    {
        Actor_ClearKeepClaimedNode(self);
        Sentient_ClaimNode(self->sentient, safeNode);
        return 1;
    }
    else
    {
        potentialNodeCount = Actor_BadPlace_FindSafeNodeOutsideBadPlace(self, potentialNodes, maxFleeDist);
        for ( potentialNodeIndex = 0; potentialNodeIndex < potentialNodeCount; ++potentialNodeIndex )
        {
            safeNode = potentialNodes[potentialNodeIndex].node;
            if ( !safeNode
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp", 954, 0, "%s", "safeNode") )
            {
                __debugbreak();
            }
            if ( Actor_FindPathToNode(self, safeNode, 1) )
            {
                Actor_ClearKeepClaimedNode(self);
                Sentient_ClaimNode(self->sentient, safeNode);
                return 1;
            }
        }
        return 0;
    }
}

double __cdecl Actor_BadPlace_GetMaximumFleeRadius()
{
    const char *v0; // eax
    unsigned int badPlaceIndex; // [esp+4h] [ebp-8h]
    float maxRadius; // [esp+8h] [ebp-4h]

    maxRadius = -1.0f;
    for (badPlaceIndex = 0; badPlaceIndex < 0x100; ++badPlaceIndex)
    {
        if (g_badplaces[badPlaceIndex].type)
        {
            switch (g_badplaces[badPlaceIndex].type)
            {
            case 1u:
            case 3u:
            case 4u:
                if (g_badplaces[badPlaceIndex].parms.arc.radius > maxRadius)
                    maxRadius = g_badplaces[badPlaceIndex].parms.arc.radius;
                break;
            case 2u:
                if (g_badplaces[badPlaceIndex].parms.arc.origin[1] > maxRadius)
                    maxRadius = g_badplaces[badPlaceIndex].parms.arc.origin[1];
                break;
            default:
                v0 = va("unhandled bad place type %i", g_badplaces[badPlaceIndex].type);
                if (!Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp", 700, 0, v0))
                    __debugbreak();
                break;
            }
        }
    }
    return maxRadius;
}

int __cdecl Actor_BadPlace_FindSafeNodeOutsideBadPlace(actor_s *self, pathsort_t *potentialNodes, float maxFleeDist)
{
    pathnode_t *node; // [esp+164h] [ebp-C18h]
    int nodeIndex; // [esp+168h] [ebp-C14h]
    pathsort_t nodes[256]; // [esp+16Ch] [ebp-C10h] BYREF
    int nodeCount; // [esp+D70h] [ebp-Ch]
    int potentialNodeCount; // [esp+D74h] [ebp-8h]
    float metric; // [esp+D78h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp", 828, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp", 829, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( !potentialNodes
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp", 830, 0, "%s", "potentialNodes") )
    {
        __debugbreak();
    }
    if ( maxFleeDist <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp",
                    831,
                    0,
                    "%s",
                    "maxFleeDist > 0.0f") )
    {
        __debugbreak();
    }
    nodeCount = Path_NodesInCylinder(self->ent->r.currentOrigin, maxFleeDist, 80.0, nodes, 256, -2);
    potentialNodeCount = 0;
    for ( nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex )
    {
        node = nodes[nodeIndex].node;
        if ( !Actor_BadPlace_IsNodeInAnyBadPlace(node)
            && Path_CanClaimNode(node, self->sentient)
            && !Actor_BadPlace_HasPotentialNodeDuplicates(potentialNodes, potentialNodeCount, node) )
        {
            metric = Vec3DistanceSq(self->ent->r.currentOrigin, node->constant.vOrigin);
            if ( Path_IsCoverNode(node) )
                metric = metric * 0.89999998;
            potentialNodes[potentialNodeCount].metric = metric;
            potentialNodes[potentialNodeCount++].node = node;
        }
    }

    if (potentialNodeCount > 1)
    {
        //std::_Sort<GfxCachedShaderText *, int, bool(__cdecl *)(GfxCachedShaderText const &, GfxCachedShaderText const &)>(
        //    (GfxCachedShaderText *)potentialNodes,
        //    (GfxCachedShaderText *)&potentialNodes[potentialNodeCount],
        //    12 * potentialNodeCount / 12,
        //    (bool(__cdecl *)(const GfxCachedShaderText *, const GfxCachedShaderText *))Path_CompareNodesIncreasing);

        std::sort(&potentialNodes[0], &potentialNodes[potentialNodeCount], Path_CompareNodesIncreasing);
    }

    return potentialNodeCount;
}

int __cdecl Path_IsCoverNode(const pathnode_t *node)
{
    if ( !node && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.h", 225, 0, "%s", "node") )
        __debugbreak();
    return (1 << node->constant.type) & 0x83FFC;
}

char __cdecl Actor_BadPlace_HasPotentialNodeDuplicates(
                pathsort_t *potentialNodes,
                int potentialNodeCount,
                pathnode_t *checkNode)
{
    int potentialNodeIndex; // [esp+0h] [ebp-4h]

    if ( !potentialNodes
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp", 717, 0, "%s", "potentialNodes") )
    {
        __debugbreak();
    }
    if ( !checkNode
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp", 718, 0, "%s", "checkNode") )
    {
        __debugbreak();
    }
    for ( potentialNodeIndex = 0; potentialNodeIndex < potentialNodeCount; ++potentialNodeIndex )
    {
        if ( potentialNodes[potentialNodeIndex].node == checkNode )
            return 1;
    }
    return 0;
}

bool __cdecl Actor_BadPlace_IsNodeInAnyBadPlace(pathnode_t *node)
{
    bool result; // al
    const char *v2; // eax
    unsigned int badPlaceIndex; // [esp+14h] [ebp-8h]

    if (!node && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp", 780, 0, "%s", "node"))
        __debugbreak();
    for (badPlaceIndex = 0; ; ++badPlaceIndex)
    {
        if (badPlaceIndex >= 0x100)
            return 0;
        if (g_badplaces[badPlaceIndex].type)
            break;
    LABEL_5:
        ;
    }
    switch (g_badplaces[badPlaceIndex].type)
    {
    case 1u:
    case 3u:
    case 4u:
        if (!&g_badplaces[badPlaceIndex].parms
            || !Path_IsNodeInArc(
                node,
                g_badplaces[badPlaceIndex].parms.arc.origin,
                g_badplaces[badPlaceIndex].parms.arc.radius,
                g_badplaces[badPlaceIndex].parms.arc.angle0,
                g_badplaces[badPlaceIndex].parms.arc.angle1,
                g_badplaces[badPlaceIndex].parms.arc.halfheight))
        {
            goto LABEL_5;
        }
        result = 1;
        break;
    case 2u:
        if (!SV_EntityContact(
            node->constant.vOrigin,
            node->constant.vOrigin,
            g_badplaces[badPlaceIndex].parms.brush.volume))
            goto LABEL_5;
        result = 1;
        break;
    default:
        v2 = va("unhandled bad place type %i", g_badplaces[badPlaceIndex].type);
        if (!Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp", 803, 0, v2))
            __debugbreak();
        goto LABEL_5;
    }
    return result;
}

pathnode_t *__cdecl Actor_BadPlace_FindSafeNodeAlongPath(actor_s *self)
{
    int pathNodeIndex; // [esp+0h] [ebp-8h]
    pathnode_t *node; // [esp+4h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp", 883, 0, "%s", "self") )
        __debugbreak();
    if ( !Actor_HasPath(self) )
        return 0;
    for ( pathNodeIndex = self->Path.lookaheadNextNode + 1; pathNodeIndex >= 0; --pathNodeIndex )
    {
        if ( self->Path.pts[pathNodeIndex].iNodeNum > 0 )
        {
            node = Path_ConvertIndexToNode(self->Path.pts[pathNodeIndex].iNodeNum);
            if ( node )
            {
                if ( !Actor_BadPlace_IsNodeInAnyBadPlace(node) && Path_CanClaimNode(node, self->sentient) )
                    return node;
            }
        }
    }
    return 0;
}

actor_think_result_t __fastcall Actor_BadPlace_Flee_Think(actor_s *self)
{
    bool isInBadPlace; // [esp+32h] [ebp-2h]

    PROF_SCOPED("fleethink");

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_badplace.cpp", 1025, 0, "%s", "self") )
        __debugbreak();
    if ( self->aiBadPlace == AI_BADPLACE_NONE )
    {
        Actor_SetState(self, AIS_EXPOSED);
        return ACTOR_THINK_REPEAT;
    }
    Actor_PreThink(self);
    self->pszDebugInfo = "badplace_flee";
    if ( Actor_HasPath(self) )
    {
        Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY_OR_MOTION);
        Actor_TeamMoveBlockedClear(self);
        Actor_MoveAlongPathWithTeam(self, 1, 0, 0);
        if ( self->pCloseEnt.isDefined() || self->pPileUpActor )
            Actor_ClearPath(self);
        goto LABEL_22;
    }
    isInBadPlace = Actor_IsInAnyBadPlace(self);
    if ( isInBadPlace && Actor_NearClaimNode(self, 32.0) )
        Path_MarkNodeInvalid(self->sentient->pClaimedNode, self->sentient->eTeam);
    if ( isInBadPlace && Actor_BadPlace_AttemptEscape(self) )
    {
        Actor_AnimStop(self, &g_animScriptTable[self->species]->stop);
LABEL_22:
        Actor_PostThink(self);
        return ACTOR_THINK_DONE;
    }
    self->aiBadPlace = AI_BADPLACE_NONE;
    Actor_SetState(self, AIS_EXPOSED);
    return ACTOR_THINK_REPEAT;
}

void __fastcall Actor_BadPlace_Flee_Finish(actor_s *self, ai_state_t eNextState)
{
    ;
}

