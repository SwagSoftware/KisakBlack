#include "cm_world.h"
#include "com_profilemapload.h"
#include <cstring>
#include "cm_load.h"
#include <server_mp/sv_main_mp.h>
#include <xanim/xmodel_utils.h>
#include "cm_tracebox.h"
#include <server/sv_game.h>

cm_world_t cm_world;

void __cdecl CM_LinkWorld()
{
    ProfLoad_Begin("CM_LinkWorld");
    CM_ClearWorld();
    CM_LinkAllStaticModels();
    ProfLoad_End();
}

bool CM_ClearWorld()
{
    bool result; // eax
    unsigned int i; // [esp+Ch] [ebp-4h]

    memset(&cm_world, 0, sizeof(cm_world));
    CM_ModelBounds(0, cm_world.mins, cm_world.maxs);
    cm_world.freeHead = 2;
    for ( i = 2; i < 0x3FF; ++i )
        cm_world.sectors[i].tree.u.parent = i + 1;
    cm_world.sectors[1023].tree.u.parent = 0;
    cm_world.sectors[1].tree.axis = (float)(cm_world.maxs[1] - cm_world.mins[1]) >= (float)(cm_world.maxs[0]
                                                                                                                                                                                - cm_world.mins[0]);
    result = (float)(cm_world.maxs[1] - cm_world.mins[1]) >= (float)(cm_world.maxs[0] - cm_world.mins[0]);
    cm_world.sectors[1].tree.dist = (float)(cm_world.maxs[result] + cm_world.mins[result]) * 0.5;

    iassert(!cm_world.sectors[SECTOR_HEAD].tree.child[0]);
    iassert(!cm_world.sectors[SECTOR_HEAD].tree.child[1]);

    return result;
}

void __cdecl CM_UnlinkEntity(svEntity_s *ent)
{
    worldSector_s *node; // [esp+8h] [ebp-14h]
    int contents; // [esp+Ch] [ebp-10h]
    unsigned __int16 nodeIndex; // [esp+10h] [ebp-Ch]
    svEntity_s *scan; // [esp+14h] [ebp-8h]
    svEntity_s *scana; // [esp+14h] [ebp-8h]
    unsigned __int16 parentNodeIndex; // [esp+18h] [ebp-4h]
    unsigned __int16 parentNodeIndexa; // [esp+18h] [ebp-4h]

    nodeIndex = ent->worldSector;
    if (ent->worldSector)
    {
        node = &cm_world.sectors[nodeIndex];
        ent->worldSector = 0;
        iassert(node->contents.entities);

        if (&sv.svEntities[node->contents.entities - 1] == ent)
        {
            node->contents.entities = ent->nextEntityInWorldSector;
        }
        else
        {
            for (scan = &sv.svEntities[node->contents.entities - 1];
                &sv.svEntities[scan->nextEntityInWorldSector - 1] != ent;
                scan = &sv.svEntities[scan->nextEntityInWorldSector - 1])
            {
                iassert(scan->nextEntityInWorldSector);
            }
            scan->nextEntityInWorldSector = ent->nextEntityInWorldSector;
        }
        while (!node->contents.entities && !node->contents.staticModels && !node->tree.child[0] && !node->tree.child[1])
        {
            if (node->contents.contentsStaticModels
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp",
                    391,
                    0,
                    "%s",
                    "!node->contents.contentsStaticModels"))
            {
                __debugbreak();
            }
            node->contents.contentsEntities = 0;
            if (!node->tree.u.parent)
            {
                iassert(nodeIndex == SECTOR_HEAD);
                goto LABEL_33;
            }
            parentNodeIndex = node->tree.u.parent;
            node->tree.u.parent = cm_world.freeHead;
            cm_world.freeHead = nodeIndex;
            node = &cm_world.sectors[parentNodeIndex];
            if (node->tree.child[0] == nodeIndex)
            {
                node->tree.child[0] = 0;
            }
            else
            {
                iassert(node->tree.child[1] == nodeIndex);
                node->tree.child[1] = 0;
            }
            nodeIndex = parentNodeIndex;
        }
        while (1)
        {
        LABEL_33:
            //contents = cm_world.sectors[node->tree.child[1]].contents.contentsEntities
            //    | cm_world.sectors[node->tree.child[0]].contents.contentsEntities;
            //if (node->contents.entities)
            //{
            //    for (scana = (svEntity_s *)&sv.configstrings[180 * node->contents.entities + 3081];
            //        ;
            //        scana = (svEntity_s *)&sv.configstrings[180 * scana->nextEntityInWorldSector + 3081])
            //    {
            //        contents |= scana->linkcontents
            //            | *(_DWORD *)((char *)&sv.gentities->r.maxs[3] + (scana - sv.svEntities) * sv.gentitySize);
            //        if (!scana->nextEntityInWorldSector)
            //            break;
            //    }
            //}
            //node->contents.contentsEntities = contents;
            //parentNodeIndexa = node->tree.u.parent;
            //if (!parentNodeIndexa)
            //    break;
            //node = &cm_world.sectors[parentNodeIndexa];

            contents = cm_world.sectors[node->tree.child[1]].contents.contentsEntities | cm_world.sectors[node->tree.child[0]].contents.contentsEntities;
            if (node->contents.entities)
            {
                scana = &sv.svEntities[node->contents.entities - 1];
                for (gentity_s *i = SV_GEntityForSvEntity(scana); ; i = SV_GEntityForSvEntity(scana))
                {
                    contents |= i->r.contents;
                    if (!scana->nextEntityInWorldSector)
                        break;
                    scana = &sv.svEntities[scana->nextEntityInWorldSector - 1];
                }
            }
            node->contents.contentsEntities = contents;
            parentNodeIndexa = node->tree.u.parent;
            if (!parentNodeIndexa)
                break;
            node = &cm_world.sectors[parentNodeIndexa];
        }
    }
}

void __cdecl CM_LinkEntity(svEntity_s *ent, float *absmin, float *absmax, unsigned int clipHandle)
{
    worldSector_s *node; // [esp+18h] [ebp-30h]
    int contents; // [esp+1Ch] [ebp-2Ch]
    unsigned __int16 nodeIndex; // [esp+20h] [ebp-28h]
    float dist; // [esp+24h] [ebp-24h]
    float mins[2]; // [esp+28h] [ebp-20h] BYREF
    cmodel_t *cmod; // [esp+30h] [ebp-18h]
    cLeaf_s *leaf; // [esp+34h] [ebp-14h]
    float maxs[2]; // [esp+38h] [ebp-10h] BYREF
    int axis; // [esp+40h] [ebp-8h]
    int linkcontents; // [esp+44h] [ebp-4h]

    cmod = CM_ClipHandleToModel(clipHandle);
    leaf = &cmod->leaf;
    //contents = *(unsigned int *)(sv.bpsWindow[8]
    //                                         + ((char *)ent - (char *)sv.svEntities[0].baseline.s.lerp.apos.trBase) / 360 * sv.bpsWindow[9]
    //                                         + 264);
    contents = SV_GEntityForSvEntity(ent)->r.contents;

    if ( !contents
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp", 622, 0, "%s", "contents") )
    {
        __debugbreak();
    }

    linkcontents = leaf->terrainContents | leaf->brushContents;
    if (leaf->terrainContents == 0 && leaf->brushContents == 0)
    {
        CM_UnlinkEntity(ent);
    }
    else
    {
        while ( 1 )
        {
            mins[0] = cm_world.mins[0];
            mins[1] = cm_world.mins[1];
            maxs[0] = cm_world.maxs[0];
            maxs[1] = cm_world.maxs[1];
            for ( nodeIndex = 1; ; nodeIndex = node->tree.child[1] )
            {
                while ( 1 )
                {
                    cm_world.sectors[nodeIndex].contents.contentsEntities |= contents;
                    cm_world.sectors[nodeIndex].contents.contentsEntities |= linkcontents;
                    node = &cm_world.sectors[nodeIndex];
                    axis = node->tree.axis;
                    dist = node->tree.dist;
                    if ( absmin[axis] <= dist )
                        break;
                    mins[axis] = dist;
                    if ( !node->tree.child[0] )
                        goto LABEL_18;
                    nodeIndex = node->tree.child[0];
                }
                if ( dist <= absmax[axis] )
                    break;
                maxs[axis] = dist;
                if ( !node->tree.child[1] )
                    goto LABEL_18;
            }
            if ( nodeIndex == ent->worldSector && (ent->linkcontents & ~linkcontents) == 0 )
            {
                ent->linkcontents = linkcontents;
                ent->linkmin[0] = *absmin;
                ent->linkmin[1] = absmin[1];
                ent->linkmin[2] = absmin[2];
                ent->linkmax[0] = *absmax;
                ent->linkmax[1] = absmax[1];
                ent->linkmax[2] = absmax[2];
                return;
            }
LABEL_18:
            if ( !ent->worldSector )
                break;
            if ( nodeIndex == ent->worldSector && (ent->linkcontents & ~linkcontents) == 0 )
                goto LABEL_24;
            CM_UnlinkEntity(ent);
        }
        CM_AddEntityToNode(ent, nodeIndex);
LABEL_24:
        ent->linkcontents = linkcontents;
        ent->linkmin[0] = *absmin;
        ent->linkmin[1] = absmin[1];
        ent->linkmin[2] = absmin[2];
        ent->linkmax[0] = *absmax;
        ent->linkmax[1] = absmax[1];
        ent->linkmax[2] = absmax[2];
        CM_SortNode(nodeIndex, mins, maxs);
    }
}

void __cdecl CM_AddEntityToNode(svEntity_s *ent, unsigned __int16 childNodeIndex)
{
    unsigned int entnum = ent - sv.svEntities;

    unsigned __int16 *prevEnt = &cm_world.sectors[childNodeIndex].contents.entities;
    while ((unsigned int)*prevEnt - 1 <= entnum)
        prevEnt = &sv.svEntities[*prevEnt - 1].nextEntityInWorldSector;

    ent->worldSector = childNodeIndex;
    ent->nextEntityInWorldSector = *prevEnt;
    *prevEnt = entnum + 1;
}

void __cdecl CM_SortNode(unsigned __int16 nodeIndex, float *mins, float *maxs)
{
    int v3; // eax
    worldSector_s *node; // [esp+Ch] [ebp-28h]
    unsigned __int16 modelnum; // [esp+10h] [ebp-24h]
    svEntity_s *prevEnt; // [esp+14h] [ebp-20h]
    float dist; // [esp+18h] [ebp-1Ch]
    svEntity_s *ent; // [esp+1Ch] [ebp-18h]
    cStaticModel_s *staticModel; // [esp+20h] [ebp-14h]
    unsigned __int16 entnum; // [esp+24h] [ebp-10h]
    int axis; // [esp+28h] [ebp-Ch]
    cStaticModel_s *prevStaticModel; // [esp+2Ch] [ebp-8h]
    unsigned __int16 childNodeIndex; // [esp+30h] [ebp-4h]
    unsigned __int16 childNodeIndexa; // [esp+30h] [ebp-4h]

    v3 = nodeIndex;
    axis = cm_world.sectors[v3].tree.axis;
    node = &cm_world.sectors[v3];
    dist = cm_world.sectors[v3].tree.dist;
    prevEnt = 0;
    entnum = cm_world.sectors[v3].contents.entities;
    while ( entnum )
    {
        ent = &sv.svEntities[entnum - 1];
        if ( ent->linkmin[axis] <= dist )
        {
            if ( dist > ent->linkmax[axis] )
            {
                childNodeIndex = node->tree.child[1];
                if ( childNodeIndex )
                    goto LABEL_14;
                childNodeIndex = CM_AllocWorldSector(mins, maxs);
                if ( childNodeIndex )
                {
                    node->tree.child[1] = childNodeIndex;
                    cm_world.sectors[childNodeIndex].tree.u.parent = nodeIndex;
                    goto LABEL_14;
                }
            }
            goto skipEntity_1;
        }
        childNodeIndex = node->tree.child[0];
        if ( childNodeIndex )
        {
LABEL_14:
            entnum = ent->nextEntityInWorldSector;
            
            iassert(prevEnt || (&sv.svEntities[node->contents.entities - 1] == ent));
            iassert(!prevEnt || (&sv.svEntities[prevEnt->nextEntityInWorldSector - 1] == ent));

            CM_AddEntityToNode(ent, childNodeIndex);
            cm_world.sectors[childNodeIndex].contents.contentsEntities |= SV_GEntityForSvEntity(ent)->r.contents;
            cm_world.sectors[childNodeIndex].contents.contentsEntities |= ent->linkcontents;
            if ( prevEnt )
                prevEnt->nextEntityInWorldSector = entnum;
            else
                node->contents.entities = entnum;
        }
        else
        {
            childNodeIndex = CM_AllocWorldSector(mins, maxs);
            if ( childNodeIndex )
            {
                node->tree.child[0] = childNodeIndex;
                cm_world.sectors[childNodeIndex].tree.u.parent = nodeIndex;
                goto LABEL_14;
            }
skipEntity_1:
            prevEnt = &sv.svEntities[entnum - 1];
            entnum = ent->nextEntityInWorldSector;
        }
    }
    prevStaticModel = 0;
    modelnum = node->contents.staticModels;
    while ( modelnum )
    {
        staticModel = &cm.staticModelList[modelnum - 1];
        if ( staticModel->absmin[axis] <= dist )
        {
            if ( dist > staticModel->absmax[axis] )
            {
                childNodeIndexa = node->tree.child[1];
                if ( childNodeIndexa )
                    goto LABEL_38;
                childNodeIndexa = CM_AllocWorldSector(mins, maxs);
                if ( childNodeIndexa )
                {
                    node->tree.child[1] = childNodeIndexa;
                    cm_world.sectors[childNodeIndexa].tree.u.parent = nodeIndex;
                    goto LABEL_38;
                }
            }
            goto skipStaticModel;
        }
        childNodeIndexa = node->tree.child[0];
        if ( childNodeIndexa )
        {
LABEL_38:
            modelnum = staticModel->writable.nextModelInWorldSector;
            if ( !prevStaticModel
                && &cm.staticModelList[node->contents.staticModels - 1] != staticModel
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp",
                            592,
                            0,
                            "%s",
                            "prevStaticModel || (&cm.staticModelList[node->contents.staticModels - 1] == staticModel)") )
            {
                __debugbreak();
            }
            if ( prevStaticModel
                && &cm.staticModelList[prevStaticModel->writable.nextModelInWorldSector - 1] != staticModel
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp",
                            593,
                            0,
                            "%s",
                            "!prevStaticModel || (&cm.staticModelList[prevStaticModel->writable.nextModelInWorldSector - 1] == staticModel)") )
            {
                __debugbreak();
            }
            CM_AddStaticModelToNode(staticModel, childNodeIndexa);
            cm_world.sectors[childNodeIndexa].contents.contentsStaticModels |= XModelGetContents(staticModel->xmodel);
            if ( prevStaticModel )
                prevStaticModel->writable.nextModelInWorldSector = modelnum;
            else
                node->contents.staticModels = modelnum;
        }
        else
        {
            childNodeIndexa = CM_AllocWorldSector(mins, maxs);
            if ( childNodeIndexa )
            {
                node->tree.child[0] = childNodeIndexa;
                cm_world.sectors[childNodeIndexa].tree.u.parent = nodeIndex;
                goto LABEL_38;
            }
skipStaticModel:
            prevStaticModel = staticModel;
            modelnum = staticModel->writable.nextModelInWorldSector;
        }
    }
}

unsigned __int16 __cdecl CM_AllocWorldSector(float *mins, float *maxs)
{
    worldSector_s *node; // [esp+8h] [ebp-14h]
    unsigned __int16 nodeIndex; // [esp+Ch] [ebp-10h]
    float size[2]; // [esp+10h] [ebp-Ch]
    unsigned __int16 axis; // [esp+18h] [ebp-4h]

    nodeIndex = cm_world.freeHead;
    if ( !cm_world.freeHead )
        return 0;
    size[0] = *maxs - *mins;
    size[1] = maxs[1] - mins[1];
    axis = size[1] >= size[0];
    if ( size[size[1] >= size[0]] <= 512.0 )
        return 0;
    node = &cm_world.sectors[cm_world.freeHead];
    if ( node->contents.contentsStaticModels
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp",
                    100,
                    0,
                    "%s",
                    "!node->contents.contentsStaticModels") )
    {
        __debugbreak();
    }
    if ( node->contents.contentsEntities
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp",
                    101,
                    0,
                    "%s",
                    "!node->contents.contentsEntities") )
    {
        __debugbreak();
    }
    if ( node->contents.entities
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp",
                    102,
                    0,
                    "%s",
                    "!node->contents.entities") )
    {
        __debugbreak();
    }
    if ( node->contents.staticModels
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp",
                    103,
                    0,
                    "%s",
                    "!node->contents.staticModels") )
    {
        __debugbreak();
    }
    cm_world.freeHead = node->tree.u.parent;
    node->tree.axis = axis;
    node->tree.dist = (float)(maxs[axis] + mins[axis]) * 0.5;
    if ( node->tree.child[0]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp",
                    110,
                    0,
                    "%s",
                    "!node->tree.child[0]") )
    {
        __debugbreak();
    }
    if ( node->tree.child[1] )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp",
                        111,
                        0,
                        "%s",
                        "!node->tree.child[1]") )
            __debugbreak();
    }
    return nodeIndex;
}

void __cdecl CM_AddStaticModelToNode(cStaticModel_s *staticModel, unsigned __int16 childNodeIndex)
{
    unsigned int modelnum; // [esp+0h] [ebp-8h]
    cStaticModel_s *prevStaticModel; // [esp+4h] [ebp-4h]

    modelnum = staticModel - cm.staticModelList;
    for (prevStaticModel = (cStaticModel_s *)&cm_world.sectors[childNodeIndex].contents.staticModels;
        (unsigned int)prevStaticModel->writable.nextModelInWorldSector - 1 <= modelnum;
        prevStaticModel = &cm.staticModelList[prevStaticModel->writable.nextModelInWorldSector - 1])
    {
        ;
    }
    staticModel->writable.nextModelInWorldSector = prevStaticModel->writable.nextModelInWorldSector;
    prevStaticModel->writable.nextModelInWorldSector = modelnum + 1;
}

unsigned int CM_LinkAllStaticModels()
{
    unsigned int result; // eax
    cStaticModel_s *staticModel; // [esp+0h] [ebp-8h]
    unsigned int i; // [esp+4h] [ebp-4h]

    i = 0;
    for ( staticModel = cm.staticModelList; ; ++staticModel )
    {
        result = i;
        if ( i >= cm.numStaticModels )
            break;
        if ( !staticModel->xmodel
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp",
                        760,
                        0,
                        "%s",
                        "staticModel->xmodel") )
        {
            __debugbreak();
        }
        if ( XModelGetContents(staticModel->xmodel) )
            CM_LinkStaticModel(staticModel);
        ++i;
    }
    return result;
}

void __cdecl CM_LinkStaticModel(cStaticModel_s *staticModel)
{
    int contents; // [esp+0h] [ebp-24h]
    worldSector_s *node; // [esp+4h] [ebp-20h]
    unsigned __int16 nodeIndex; // [esp+8h] [ebp-1Ch]
    float dist; // [esp+Ch] [ebp-18h]
    float mins[2]; // [esp+10h] [ebp-14h] BYREF
    float maxs[2]; // [esp+18h] [ebp-Ch] BYREF
    int axis; // [esp+20h] [ebp-4h]

    contents = XModelGetContents(staticModel->xmodel);
    if ( !contents
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp", 709, 0, "%s", "contents") )
    {
        __debugbreak();
    }
    mins[0] = cm_world.mins[0];
    mins[1] = cm_world.mins[1];
    maxs[0] = cm_world.maxs[0];
    maxs[1] = cm_world.maxs[1];
    for ( nodeIndex = 1; ; nodeIndex = node->tree.child[1] )
    {
        while ( 1 )
        {
            cm_world.sectors[nodeIndex].contents.contentsStaticModels |= contents;
            node = &cm_world.sectors[nodeIndex];
            axis = node->tree.axis;
            dist = node->tree.dist;
            if ( staticModel->absmin[axis] <= dist )
                break;
            mins[axis] = dist;
            if ( !node->tree.child[0] )
                goto LABEL_12;
            nodeIndex = node->tree.child[0];
        }
        if ( dist <= staticModel->absmax[axis] )
            break;
        maxs[axis] = dist;
        if ( !node->tree.child[1] )
            break;
    }
LABEL_12:
    CM_AddStaticModelToNode(staticModel, nodeIndex);
    CM_SortNode(nodeIndex, mins, maxs);
}

int __cdecl CM_AreaEntities(const float *mins, const float *maxs, int *entityList, int maxcount, int contentmask)
{
    int count; // [esp+8h] [ebp-20h]
    areaParms_t ap; // [esp+Ch] [ebp-1Ch] BYREF

    PROF_SCOPED("CM_AreaEntities");

    ap.mins = mins;
    ap.maxs = maxs;
    ap.list = entityList;
    ap.count = 0;
    ap.maxcount = maxcount;
    ap.contentmask = contentmask;
    CM_AreaEntities_r(1u, &ap);
    count = ap.count;

    return count;
}

void __cdecl CM_AreaEntities_r(unsigned int nodeIndex, areaParms_t *ap)
{
    // kcod4
    worldSector_s *node; // [esp+0h] [ebp-14h]
    unsigned int nextNodeIndex; // [esp+4h] [ebp-10h]
    gentity_s *gcheck; // [esp+8h] [ebp-Ch]
    unsigned int entnum; // [esp+Ch] [ebp-8h]
    svEntity_s *svEnt;

    int n;
    for (node = &cm_world.sectors[nodeIndex]; (node->contents.contentsEntities & ap->contentmask) != 0; node = &cm_world.sectors[nodeIndex])
    {
        for (entnum = node->contents.entities; entnum; entnum = svEnt->nextEntityInWorldSector)
        {
            svEnt = &sv.svEntities[entnum - 1];
            gcheck = SV_GEntityForSvEntity(svEnt);

            if ((ap->contentmask & gcheck->r.contents) != 0
                && ap->maxs[0] >= gcheck->r.absmin[0]
                && ap->mins[0] <= gcheck->r.absmax[0]
                && ap->maxs[1] >= gcheck->r.absmin[1]
                && ap->mins[1] <= gcheck->r.absmax[1]
                && ap->maxs[2] >= gcheck->r.absmin[2]
                && ap->mins[2] <= gcheck->r.absmax[2]
                )
            {
                if (ap->count >= ap->maxcount)
                {
                    Com_DPrintf(16, "CM_AreaEntities: MAXCOUNT\n");
                    return;
                }

                ap->list[ap->count] = svEnt - sv.svEntities;
                ap->count++;
            }
        }
        if (node->tree.dist >= ap->maxs[node->tree.axis])
        {
            if (node->tree.dist <= ap->mins[node->tree.axis])
                return;
            nodeIndex = node->tree.child[1];
        }
        else if (node->tree.dist <= ap->mins[node->tree.axis])
        {
            nodeIndex = node->tree.child[0];
        }
        else
        {
            nextNodeIndex = node->tree.child[1];
            CM_AreaEntities_r(node->tree.child[0], ap);
            nodeIndex = nextNodeIndex;
        }
    }
}

void __cdecl CM_PointTraceStaticModels(trace_t *results, const float *start, const float *end, int contentmask)
{
    locTraceWork_t tw; // [esp+14h] [ebp-58h] BYREF
    _QWORD start_[2]; // [esp+4Ch] [ebp-20h] BYREF
    _QWORD end_[2]; // [esp+5Ch] [ebp-10h] BYREF

    PROF_SCOPED("pointtracestaticmodels");
    ////TraceExtents::TraceExtents(&tw.extents);
    tw.contents = contentmask;
    *(_QWORD *)tw.extents.start.vec.v = *(_QWORD *)start;
    tw.extents.start.vec.v[2] = start[2];
    *(_QWORD *)tw.extents.end.vec.v = *(_QWORD *)end;
    tw.extents.end.vec.v[2] = end[2];
    CM_CalcTraceExtents(&tw.extents);
    start_[0] = *(_QWORD *)tw.extents.start.vec.v;
    start_[1] = __PAIR64__(0, tw.extents.start.vec.u[2]);
    end_[0] = *(_QWORD *)tw.extents.end.vec.v;
    LODWORD(end_[1]) = tw.extents.end.vec.u[2];
    HIDWORD(end_[1]) = LODWORD(results->fraction);
    CM_PointTraceStaticModels_r(&tw, 1u, (const float *)start_, (const float *)end_, results);
}

void __cdecl CM_PointTraceStaticModels_r(
                locTraceWork_t *tw,
                unsigned __int16 nodeIndex,
                const float *p1_,
                const float *p2,
                trace_t *trace)
{
    float v5; // [esp+Ch] [ebp-40h]
    bool side; // [esp+10h] [ebp-3Ch]
    worldSector_s *node; // [esp+14h] [ebp-38h]
    unsigned __int16 modelnum; // [esp+18h] [ebp-34h]
    float t1; // [esp+1Ch] [ebp-30h]
    float frac; // [esp+20h] [ebp-2Ch]
    float p1[4]; // [esp+24h] [ebp-28h] BYREF
    float t2; // [esp+34h] [ebp-18h]
    float mid[4]; // [esp+38h] [ebp-14h] BYREF
    cStaticModel_s *check; // [esp+48h] [ebp-4h]

    p1[0] = *p1_;
    p1[1] = p1_[1];
    p1[2] = p1_[2];
    p1[3] = p1_[3];
    while ( 1 )
    {
        node = &cm_world.sectors[nodeIndex];
        if ( (tw->contents & node->contents.contentsStaticModels) == 0 )
            break;
        for ( modelnum = node->contents.staticModels; modelnum; modelnum = check->writable.nextModelInWorldSector )
        {
            check = &cm.staticModelList[modelnum - 1];
            if ( (tw->contents & XModelGetContents(check->xmodel)) != 0 )
            {
                if ( intersect_extents_aabb(&tw->extents, check->absmin, check->absmax, trace->fraction) )
                    CM_TraceStaticModel(check, trace, tw->extents.start.vec.v, tw->extents.end.vec.v, tw->contents);
            }
        }
        t1 = p1[node->tree.axis] - node->tree.dist;
        t2 = p2[node->tree.axis] - node->tree.dist;
        if ( (float)(t1 * t2) < 0.0 )
        {
            if ( p1[3] >= trace->fraction )
                return;
            if ( (float)(t1 - t2) == 0.0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp", 910, 0, "%s", "t1 - t2") )
            {
                __debugbreak();
            }
            frac = t1 / (float)(t1 - t2);
            if ( frac < 0.0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp", 912, 0, "%s", "frac >= 0") )
            {
                __debugbreak();
            }
            if ( frac > 1.0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp", 913, 0, "%s", "frac <= 1.f") )
            {
                __debugbreak();
            }
            mid[0] = (float)((float)(*p2 - p1[0]) * frac) + p1[0];
            mid[1] = (float)((float)(p2[1] - p1[1]) * frac) + p1[1];
            mid[2] = (float)((float)(p2[2] - p1[2]) * frac) + p1[2];
            mid[3] = (float)((float)(p2[3] - p1[3]) * frac) + p1[3];
            side = t2 >= 0.0;
            CM_PointTraceStaticModels_r(tw, node->tree.child[side], p1, mid, trace);
            nodeIndex = node->tree.child[1 - side];
            p1[0] = mid[0];
            p1[1] = mid[1];
            p1[2] = mid[2];
            p1[3] = mid[3];
        }
        else
        {
            if ( (float)(t2 - t1) < 0.0 )
                v5 = t2;
            else
                v5 = t1;
            nodeIndex = node->tree.child[v5 < 0.0];
        }
    }
}

int __cdecl CM_PointTraceStaticModelsComplete(const float *start, const float *end, int contentmask)
{
    staticmodeltrace_t clip; // [esp+8h] [ebp-3Ch] BYREF

    //TraceExtents::TraceExtents(&clip.extents);
    clip.contents = contentmask;
    *(_QWORD *)clip.extents.start.vec.v = *(_QWORD *)start;
    clip.extents.start.vec.v[2] = start[2];
    *(_QWORD *)clip.extents.end.vec.v = *(_QWORD *)end;
    clip.extents.end.vec.v[2] = end[2];
    CM_CalcTraceExtents(&clip.extents);
    return CM_PointTraceStaticModelsComplete_r(&clip, 1u, clip.extents.start.vec.v, clip.extents.end.vec.v);
}

int __cdecl CM_PointTraceStaticModelsComplete_r(
                const staticmodeltrace_t *clip,
                unsigned __int16 nodeIndex,
                const float *p1_,
                const float *p2)
{
    float v5; // [esp+Ch] [ebp-38h]
    bool side; // [esp+10h] [ebp-34h]
    worldSector_s *node; // [esp+14h] [ebp-30h]
    unsigned __int16 modelnum; // [esp+18h] [ebp-2Ch]
    float t1; // [esp+1Ch] [ebp-28h]
    float frac; // [esp+20h] [ebp-24h]
    float p1[3]; // [esp+24h] [ebp-20h] BYREF
    float t2; // [esp+30h] [ebp-14h]
    float mid[3]; // [esp+34h] [ebp-10h] BYREF
    cStaticModel_s *check; // [esp+40h] [ebp-4h]

    p1[0] = *p1_;
    p1[1] = p1_[1];
    for ( p1[2] = p1_[2]; ; p1[2] = mid[2] )
    {
        while ( 1 )
        {
            node = &cm_world.sectors[nodeIndex];
            if ( (clip->contents & node->contents.contentsStaticModels) == 0 )
                return 1;
            for ( modelnum = node->contents.staticModels; modelnum; modelnum = check->writable.nextModelInWorldSector )
            {
                check = &cm.staticModelList[modelnum - 1];
                if ( (clip->contents & XModelGetContents(check->xmodel)) != 0
                    && intersect_extents_aabb(&clip->extents, check->absmin, check->absmax, 1.0)
                    && !CM_TraceStaticModelComplete(check, clip->extents.start.vec.v, clip->extents.end.vec.v, clip->contents) )
                {
                    return 0;
                }
            }
            t1 = p1[node->tree.axis] - node->tree.dist;
            t2 = p2[node->tree.axis] - node->tree.dist;
            if ( (float)(t1 * t2) < 0.0 )
                break;
            if ( (float)(t2 - t1) < 0.0 )
                v5 = t2;
            else
                v5 = t1;
            nodeIndex = node->tree.child[v5 < 0.0];
        }
        if ( (float)(t1 - t2) == 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp", 993, 0, "%s", "t1 - t2") )
        {
            __debugbreak();
        }
        frac = t1 / (float)(t1 - t2);
        if ( frac < 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp", 995, 0, "%s", "frac >= 0") )
        {
            __debugbreak();
        }
        if ( frac > 1.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp", 996, 0, "%s", "frac <= 1.f") )
        {
            __debugbreak();
        }
        mid[0] = (float)((float)(*p2 - p1[0]) * frac) + p1[0];
        mid[1] = (float)((float)(p2[1] - p1[1]) * frac) + p1[1];
        mid[2] = (float)((float)(p2[2] - p1[2]) * frac) + p1[2];
        side = t2 >= 0.0;
        if ( !CM_PointTraceStaticModelsComplete_r(clip, node->tree.child[side], p1, mid) )
            break;
        nodeIndex = node->tree.child[1 - side];
        p1[0] = mid[0];
        p1[1] = mid[1];
    }
    return 0;
}

void __cdecl CM_ClipMoveToEntities(moveclip_t *clip, trace_t *trace)
{
    _QWORD start[2]; // [esp+1Ch] [ebp-20h] BYREF
    _QWORD end[2]; // [esp+2Ch] [ebp-10h] BYREF

    PROF_SCOPED("clipmoveentities");

    if ( trace->fraction > 1.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp",
                    1132,
                    0,
                    "%s\n\t(trace->fraction) = %g",
                    "(trace->fraction <= 1.f)",
                    trace->fraction) )
    {
        __debugbreak();
    }
    start[0] = *(_QWORD *)clip->extents.start.vec.v;
    LODWORD(start[1]) = clip->extents.start.vec.u[2];
    end[0] = *(_QWORD *)clip->extents.end.vec.v;
    LODWORD(end[1]) = clip->extents.end.vec.u[2];
    HIDWORD(start[1]) = 0;
    HIDWORD(end[1]) = LODWORD(trace->fraction);
    CM_ClipMoveToEntities_r(clip, 1u, (const float *)start, (const float *)end, trace);
}

void __cdecl CM_ClipMoveToEntities_r(
                const moveclip_t *clip,
                unsigned __int16 nodeIndex,
                const float *p1,
                const float *p2,
                trace_t *trace)
{
    float v5; // [esp+8h] [ebp-6Ch]
    float v6; // [esp+Ch] [ebp-68h]
    float v7; // [esp+14h] [ebp-60h]
    float v8; // [esp+18h] [ebp-5Ch]
    float v9; // [esp+1Ch] [ebp-58h]
    bool side; // [esp+20h] [ebp-54h]
    worldSector_s *node; // [esp+24h] [ebp-50h]
    float diff; // [esp+28h] [ebp-4Ch]
    float t1; // [esp+30h] [ebp-44h]
    float frac; // [esp+34h] [ebp-40h]
    float offset; // [esp+38h] [ebp-3Ch]
    float t2; // [esp+3Ch] [ebp-38h]
    float frac2; // [esp+40h] [ebp-34h]
    float absDiff; // [esp+44h] [ebp-30h]
    unsigned int entnum; // [esp+4Ch] [ebp-28h]
    float mid[4]; // [esp+50h] [ebp-24h] BYREF
    svEntity_s *check; // [esp+60h] [ebp-14h]
    float p[4]; // [esp+64h] [ebp-10h] BYREF

    p[0] = *p1;
    p[1] = p1[1];
    p[2] = p1[2];
    p[3] = p1[3];
    while ( 1 )
    {
        node = &cm_world.sectors[nodeIndex];
        if ( (clip->contentmask & node->contents.contentsEntities) == 0 )
            break;
        for ( entnum = node->contents.entities; entnum; entnum = check->nextEntityInWorldSector )
        {
            check = &sv.svEntities[entnum - 1];
            if ( (check->linkcontents & clip->contentmask) != 0 )
                SV_TraceCapsuleToEntity(clip, check, trace);
        }
        t1 = p[node->tree.axis] - node->tree.dist;
        t2 = p2[node->tree.axis] - node->tree.dist;
        offset = clip->outerSize[node->tree.axis];
        if ( (float)(t2 - t1) < 0.0 )
            v9 = p2[node->tree.axis] - node->tree.dist;
        else
            v9 = p[node->tree.axis] - node->tree.dist;
        if ( v9 < offset )
        {
            if ( (float)(t1 - t2) < 0.0 )
                v8 = p2[node->tree.axis] - node->tree.dist;
            else
                v8 = p[node->tree.axis] - node->tree.dist;
            if ( (-(offset)) < v8 )
            {
                if ( p[3] >= trace->fraction )
                    return;
                diff = t2 - t1;
                if ( (float)(t2 - t1) == 0.0 )
                {
                    side = 0;
                    frac = 1.0f;
                    frac2 = 0.0f;
                }
                else
                {
                    absDiff = fabs(diff);
                    if ( diff < 0.0 )
                        v7 = p[node->tree.axis] - node->tree.dist;
                    else
                        v7 = -t1;
                    frac2 = (float)(v7 - offset) * (float)(1.0 / absDiff);
                    frac = (float)(v7 + offset) * (float)(1.0 / absDiff);
                    side = diff >= 0.0;
                }
                if ( frac < 0.0
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp", 1102, 0, "%s", "frac >= 0") )
                {
                    __debugbreak();
                }
                if ( (float)(1.0 - frac) < 0.0 )
                    v6 = 1.0f;
                else
                    v6 = frac;
                mid[0] = (float)((float)(*p2 - p[0]) * v6) + p[0];
                mid[1] = (float)((float)(p2[1] - p[1]) * v6) + p[1];
                mid[2] = (float)((float)(p2[2] - p[2]) * v6) + p[2];
                mid[3] = (float)((float)(p2[3] - p[3]) * v6) + p[3];
                CM_ClipMoveToEntities_r(clip, node->tree.child[side], (const float*)p, (const float *)mid, trace);
                if ( frac2 > 1.0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp",
                                1112,
                                0,
                                "%s\n\t(frac2) = %g",
                                "(frac2 <= 1.f)",
                                frac2) )
                {
                    __debugbreak();
                }
                if ( (float)(frac2 - 0.0) < 0.0 )
                    v5 = 0.0f;
                else
                    v5 = frac2;
                p[0] = (float)((float)(*p2 - p[0]) * v5) + p[0];
                p[1] = (float)((float)(p2[1] - p[1]) * v5) + p[1];
                p[2] = (float)((float)(p2[2] - p[2]) * v5) + p[2];
                p[3] = (float)((float)(p2[3] - p[3]) * v5) + p[3];
                nodeIndex = node->tree.child[1 - side];
            }
            else
            {
                nodeIndex = node->tree.child[1];
            }
        }
        else
        {
            nodeIndex = node->tree.child[0];
        }
    }
}

int __cdecl CM_ClipSightTraceToEntities(sightclip_t *clip)
{
    return CM_ClipSightTraceToEntities_r(clip, 1u, clip->start, clip->end);
}

int __cdecl CM_ClipSightTraceToEntities_r(
                const sightclip_t *clip,
                unsigned __int16 nodeIndex,
                const float *p1,
                const float *p2)
{
    float v5; // [esp+8h] [ebp-68h]
    float v6; // [esp+Ch] [ebp-64h]
    float v7; // [esp+14h] [ebp-5Ch]
    float v8; // [esp+18h] [ebp-58h]
    float v9; // [esp+1Ch] [ebp-54h]
    bool side; // [esp+20h] [ebp-50h]
    worldSector_s *node; // [esp+24h] [ebp-4Ch]
    float diff; // [esp+28h] [ebp-48h]
    float t1; // [esp+30h] [ebp-40h]
    float frac; // [esp+34h] [ebp-3Ch]
    float offset; // [esp+38h] [ebp-38h]
    float t2; // [esp+3Ch] [ebp-34h]
    float frac2; // [esp+40h] [ebp-30h]
    float absDiff; // [esp+44h] [ebp-2Ch]
    unsigned int entnum; // [esp+4Ch] [ebp-24h]
    int hitNum; // [esp+50h] [ebp-20h]
    int hitNuma; // [esp+50h] [ebp-20h]
    float mid[3]; // [esp+54h] [ebp-1Ch] BYREF
    svEntity_s *check; // [esp+60h] [ebp-10h]
    float p[3]; // [esp+64h] [ebp-Ch] BYREF

    p[0] = *p1;
    p[1] = p1[1];
    p[2] = p1[2];
    while ( 1 )
    {
        while ( 1 )
        {
            while ( 1 )
            {
                node = &cm_world.sectors[nodeIndex];
                if ( (clip->contentmask & node->contents.contentsEntities) == 0 )
                    return 0;
                for ( entnum = node->contents.entities; entnum; entnum = check->nextEntityInWorldSector )
                {
                    check = &sv.svEntities[entnum - 1];
                    hitNum = SV_SightTraceCapsuleToEntity(clip, entnum - 1);
                    if ( hitNum )
                        return hitNum;
                }
                t1 = p[node->tree.axis] - node->tree.dist;
                t2 = p2[node->tree.axis] - node->tree.dist;
                offset = clip->outerSize[node->tree.axis];
                v9 = (float)(t2 - t1) < 0.0 ? p2[node->tree.axis] - node->tree.dist : p[node->tree.axis] - node->tree.dist;
                if ( v9 < offset )
                    break;
                nodeIndex = node->tree.child[0];
            }
            v8 = (float)(t1 - t2) < 0.0 ? p2[node->tree.axis] - node->tree.dist : p[node->tree.axis] - node->tree.dist;
            if ( (-(offset)) < v8 )
                break;
            nodeIndex = node->tree.child[1];
        }
        diff = t2 - t1;
        if ( (float)(t2 - t1) == 0.0 )
        {
            side = 0;
            frac = 1.0f;
            frac2 = 0.0f;
        }
        else
        {
            absDiff = fabs(diff);
            if ( diff < 0.0 )
                v7 = p[node->tree.axis] - node->tree.dist;
            else
                v7 = -t1;
            frac2 = (float)(v7 - offset) * (float)(1.0 / absDiff);
            frac = (float)(v7 + offset) * (float)(1.0 / absDiff);
            side = diff >= 0.0;
        }
        if ( frac < 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp", 1212, 0, "%s", "frac >= 0") )
        {
            __debugbreak();
        }
        v6 = (float)(1.0 - frac) < 0.0 ? 1.0f : frac;
        mid[0] = (float)((float)(*p2 - p[0]) * v6) + p[0];
        mid[1] = (float)((float)(p2[1] - p[1]) * v6) + p[1];
        mid[2] = (float)((float)(p2[2] - p[2]) * v6) + p[2];
        hitNuma = CM_ClipSightTraceToEntities_r(clip, node->tree.child[side], p, mid);
        if ( hitNuma )
            break;
        if ( frac2 > 1.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp",
                        1223,
                        0,
                        "%s\n\t(frac2) = %g",
                        "(frac2 <= 1.f)",
                        frac2) )
        {
            __debugbreak();
        }
        if ( (float)(frac2 - 0.0) < 0.0 )
            v5 = 0.0f;
        else
            v5 = frac2;
        p[0] = (float)((float)(*p2 - p[0]) * v5) + p[0];
        p[1] = (float)((float)(p2[1] - p[1]) * v5) + p[1];
        p[2] = (float)((float)(p2[2] - p[2]) * v5) + p[2];
        nodeIndex = node->tree.child[1 - side];
    }
    return hitNuma;
}

void __cdecl CM_PointTraceToEntities(pointtrace_t *clip, trace_t *trace, col_context_t *context)
{
    _QWORD start[2]; // [esp+18h] [ebp-20h] BYREF
    _QWORD end[2]; // [esp+28h] [ebp-10h] BYREF

    PROF_SCOPED("pointtraceentities");

    if ( trace->fraction > 1.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp",
                    1319,
                    0,
                    "%s\n\t(trace->fraction) = %g",
                    "(trace->fraction <= 1.f)",
                    trace->fraction) )
    {
        __debugbreak();
    }
    start[0] = *(_QWORD *)clip->extents.start.vec.v;
    LODWORD(start[1]) = clip->extents.start.vec.u[2];
    end[0] = *(_QWORD *)clip->extents.end.vec.v;
    LODWORD(end[1]) = clip->extents.end.vec.u[2];
    HIDWORD(start[1]) = 0;
    HIDWORD(end[1]) = LODWORD(trace->fraction);
    CM_PointTraceToEntities_r(clip, 1u, (const float *)start, (const float *)end, trace, context);
}

void __cdecl CM_PointTraceToEntities_r(
                pointtrace_t *clip,
                unsigned __int16 nodeIndex,
                const float *p1,
                const float *p2,
                trace_t *trace,
                col_context_t *context)
{
    float v6; // [esp+14h] [ebp-48h]
    worldSector_s *node; // [esp+24h] [ebp-38h]
    float t1; // [esp+28h] [ebp-34h]
    float frac; // [esp+2Ch] [ebp-30h]
    float t2; // [esp+30h] [ebp-2Ch]
    unsigned int entnum; // [esp+34h] [ebp-28h]
    float mid[4]; // [esp+38h] [ebp-24h] BYREF
    svEntity_s *check; // [esp+48h] [ebp-14h]
    float p[4]; // [esp+4Ch] [ebp-10h] BYREF

    p[0] = *p1;
    p[1] = p1[1];
    p[2] = p1[2];
    p[3] = p1[3];
    while ( 1 )
    {
        node = &cm_world.sectors[nodeIndex];
        if ( (clip->contentmask & node->contents.contentsEntities) == 0 )
            break;
        //_mm_prefetch((const char *)trace, 1);
        for ( entnum = node->contents.entities; entnum; entnum = check->nextEntityInWorldSector )
        {
            check = &sv.svEntities[entnum - 1];
            if ( !context->collide_entity_func || context->collide_entity_func(entnum - 1, context) )
            {
                //_mm_prefetch((const char *)(sv.bpsWindow[8] + sv.bpsWindow[9] * (entnum - 1) + 224), 1);
                //_mm_prefetch((const char *)&check->nextEntityInWorldSector, 1);
                SV_TracePointToEntity(clip, check, trace);
            }
        }
        t1 = p[node->tree.axis] - node->tree.dist;
        t2 = p2[node->tree.axis] - node->tree.dist;
        if ( (float)(t1 * t2) < 0.0 )
        {
            if ( p[3] >= trace->fraction )
                return;
            frac = t1 / (float)(t1 - t2);
            if ( frac < 0.0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp", 1294, 0, "%s", "frac >= 0") )
            {
                __debugbreak();
            }
            if ( frac > 1.0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp", 1295, 0, "%s", "frac <= 1.f") )
            {
                __debugbreak();
            }
            mid[0] = (float)((float)(*p2 - p[0]) * frac) + p[0];
            mid[1] = (float)((float)(p2[1] - p[1]) * frac) + p[1];
            mid[2] = (float)((float)(p2[2] - p[2]) * frac) + p[2];
            mid[3] = (float)((float)(p2[3] - p[3]) * frac) + p[3];
            CM_PointTraceToEntities_r(clip, node->tree.child[t2 >= 0.0], p, mid, trace, context);
            nodeIndex = node->tree.child[t2 < 0.0];
            p[0] = mid[0];
            p[1] = mid[1];
            p[2] = mid[2];
            p[3] = mid[3];
        }
        else
        {
            if ( (float)(t2 - t1) < 0.0 )
                v6 = p2[node->tree.axis] - node->tree.dist;
            else
                v6 = p[node->tree.axis] - node->tree.dist;
            nodeIndex = node->tree.child[v6 < 0.0];
        }
    }
}

int __cdecl CM_PointSightTraceToEntities(sightpointtrace_t *clip)
{
    return CM_PointSightTraceToEntities_r(clip, 1u, clip->start, clip->end);
}

int __cdecl CM_PointSightTraceToEntities_r(
                const sightpointtrace_t *clip,
                unsigned __int16 nodeIndex,
                const float *p1,
                const float *p2)
{
    float v5; // [esp+8h] [ebp-30h]
    worldSector_s *node; // [esp+10h] [ebp-28h]
    float t1; // [esp+14h] [ebp-24h]
    float frac; // [esp+18h] [ebp-20h]
    float t2; // [esp+1Ch] [ebp-1Ch]
    unsigned int entnum; // [esp+20h] [ebp-18h]
    int hitNum; // [esp+24h] [ebp-14h]
    int hitNuma; // [esp+24h] [ebp-14h]
    int hitNumb; // [esp+24h] [ebp-14h]
    int hitNumc; // [esp+24h] [ebp-14h]
    float mid[3]; // [esp+28h] [ebp-10h] BYREF
    svEntity_s *check; // [esp+34h] [ebp-4h]

    node = &cm_world.sectors[nodeIndex];
    if ( (clip->contentmask & node->contents.contentsEntities) == 0 )
        return 0;
    t1 = p1[node->tree.axis] - node->tree.dist;
    t2 = p2[node->tree.axis] - node->tree.dist;
    if ( (float)(t1 * t2) < 0.0 )
    {
        frac = t1 / (float)(t1 - t2);
        if ( frac < 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp", 1359, 0, "%s", "frac >= 0") )
        {
            __debugbreak();
        }
        if ( frac > 1.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_world.cpp", 1360, 0, "%s", "frac <= 1.f") )
        {
            __debugbreak();
        }
        mid[0] = (float)((float)(*p2 - *p1) * frac) + *p1;
        mid[1] = (float)((float)(p2[1] - p1[1]) * frac) + p1[1];
        mid[2] = (float)((float)(p2[2] - p1[2]) * frac) + p1[2];
        hitNuma = CM_PointSightTraceToEntities_r(clip, node->tree.child[t2 >= 0.0], p1, mid);
        if ( hitNuma )
            return hitNuma;
        hitNumb = CM_PointSightTraceToEntities_r(clip, node->tree.child[t2 < 0.0], mid, p2);
        if ( hitNumb )
            return hitNumb;
    }
    else
    {
        if ( (float)(t2 - t1) < 0.0 )
            v5 = p2[node->tree.axis] - node->tree.dist;
        else
            v5 = p1[node->tree.axis] - node->tree.dist;
        hitNum = CM_PointSightTraceToEntities_r(clip, node->tree.child[v5 < 0.0], p1, p2);
        if ( hitNum )
            return hitNum;
    }
    for ( entnum = node->contents.entities; entnum; entnum = check->nextEntityInWorldSector )
    {
        check = &sv.svEntities[entnum - 1];
        hitNumc = SV_SightTracePointToEntity(clip, entnum - 1);
        if ( hitNumc )
            return hitNumc;
    }
    return 0;
}

