#include "sv_world.h"

#include <bgame/bg_public.h>
#include "sv_game.h"
#include <qcommon/cm_world.h>
#include <universal/com_math_anglevectors.h>
#include <bgame/bg_slidemove.h>
#include <qcommon/cm_test.h>
#include <qcommon/cm_load.h>
#include <qcommon/dobj_management.h>
#include <cgame/cg_world.h>
#include <qcommon/cm_tracebox.h>
#include <bgame/bg_misc.h>
#include <game_mp/g_utils_mp.h>
#include <game_mp/g_main_mp.h>
#include <glass/glass_server.h>
#include <DynEntity/DynEntity_server.h>
#include <qcommon/common.h>

unsigned int __cdecl SV_ClipHandleForEntity(const gentity_s *ent)
{
    if ( ent->r.bmodel )
        return ent->s.index.brushmodel;
    else
        return CM_TempBoxModel(ent->r.mins, ent->r.maxs, ent->r.contents);
}

void __cdecl SV_UnlinkEntity(gentity_s *gEnt)
{
    svEntity_s *v1; // eax

    v1 = SV_SvEntityForGentity(gEnt);
    gEnt->r.linked = 0;
    CM_UnlinkEntity(v1);
}

void    SV_LinkEntity(gentity_s *gEnt)
{
    int v2; // eax
    float v3[3]; // [esp-Ch] [ebp-2E8h] BYREF
    //float v4; // [esp-8h] [ebp-2E4h]
    //float v5; // [esp-4h] [ebp-2E0h]
    float absmax[3]; // [esp+0h] [ebp-2DCh] BYREF
    const DObj *absmin; // [esp+Ch] [ebp-2D0h]
    unsigned int clipHandle; // [esp+10h] [ebp-2CCh]
    unsigned __int16 cluster[4]; // [esp+14h] [ebp-2C8h]
    int num_leafs; // [esp+1Ch] [ebp-2C0h]
    unsigned __int16 leafs[130]; // [esp+20h] [ebp-2BCh] BYREF
    int lastLeaf; // [esp+124h] [ebp-1B8h] BYREF
    int m; // [esp+128h] [ebp-1B4h]
    float max; // [esp+12Ch] [ebp-1B0h]
    float *maxs; // [esp+130h] [ebp-1ACh]
    float *v16; // [esp+134h] [ebp-1A8h]
    float *mins; // [esp+138h] [ebp-1A4h]
    float *v18; // [esp+13Ch] [ebp-1A0h]
    phys_vec3 aabb_mn; // [esp+140h] [ebp-19Ch] BYREF
    phys_vec3 aabb_mn_4; // [esp+150h] [ebp-18Ch] BYREF
    phys_mat44 xform; // [esp+160h] [ebp-17Ch] BYREF
    float v22[9]; // [esp+1ACh] [ebp-130h] BYREF
    phys_vec3 ctr; // [esp+1D0h] [ebp-10Ch] BYREF
    float ctr_4; // [esp+1E0h] [ebp-FCh]
    float ctr_8; // [esp+1E4h] [ebp-F8h]
    float ctr_12; // [esp+1E8h] [ebp-F4h]
    float v27; // [esp+1F4h] [ebp-E8h]
    float v28; // [esp+1F8h] [ebp-E4h]
    float v29; // [esp+1FCh] [ebp-E0h]
    float v30; // [esp+200h] [ebp-DCh]
    float v31; // [esp+204h] [ebp-D8h]
    float v32; // [esp+208h] [ebp-D4h]
    float v33; // [esp+214h] [ebp-C8h]
    float v34; // [esp+218h] [ebp-C4h]
    float v35; // [esp+21Ch] [ebp-C0h]
    phys_vec3 rvec; // [esp+220h] [ebp-BCh] BYREF
    float rvec_4; // [esp+230h] [ebp-ACh]
    float rvec_8; // [esp+234h] [ebp-A8h]
    float rvec_12; // [esp+238h] [ebp-A4h]
    float v40; // [esp+244h] [ebp-98h]
    float v41; // [esp+248h] [ebp-94h]
    float v42; // [esp+24Ch] [ebp-90h]
    float v43; // [esp+250h] [ebp-8Ch]
    float v44; // [esp+254h] [ebp-88h]
    float v45; // [esp+258h] [ebp-84h]
    float v46; // [esp+264h] [ebp-78h]
    float v47; // [esp+268h] [ebp-74h]
    float v48; // [esp+26Ch] [ebp-70h]
    phys_vec3 mx; // [esp+270h] [ebp-6Ch] BYREF
    phys_vec3 mn; // [esp+280h] [ebp-5Ch] BYREF
    int v51; // [esp+29Ch] [ebp-40h]
    int v52; // [esp+2A0h] [ebp-3Ch]
    float v53; // [esp+2A4h] [ebp-38h]
    int v54; // [esp+2A8h] [ebp-34h]
    int v55; // [esp+2ACh] [ebp-30h]
    float *angles; // [esp+2B0h] [ebp-2Ch]
    float *currentOrigin; // [esp+2B4h] [ebp-28h]
    float *currentAngles; // [esp+2B8h] [ebp-24h]
    int k; // [esp+2BCh] [ebp-20h]
    int j; // [esp+2C0h] [ebp-1Ch]
    int i; // [esp+2C4h] [ebp-18h]
    svEntity_s *ent; // [esp+2C8h] [ebp-14h]
    //_UNKNOWN *v63[2]; // [esp+2D0h] [ebp-Ch] BYREF
    //int vars0; // [esp+2DCh] [ebp+0h]
    //
    //v63[0] = a1;
    //v63[1] = (_UNKNOWN *)vars0;
    if (!gEnt->r.inuse
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp", 83, 0, "%s", "gEnt->r.inuse"))
    {
        __debugbreak();
    }
    ent = SV_SvEntityForGentity(gEnt);
    if (gEnt->r.bmodel)
    {
        gEnt->s.solid = 0xFFFFFF;
    }
    else if ((gEnt->r.contents & 0x2008001) != 0)
    {
        i = (int)gEnt->r.maxs[0];
        if (i < 1)
            i = 1;
        if (i > 255)
            i = 255;
        j = (int)(float)((-(gEnt->r.mins[2])) + 1.0);
        if (j < 1)
            j = 1;
        if (j > 255)
            j = 255;
        k = (int)(float)(gEnt->r.maxs[2] + 32.0);
        if (k < 1)
            k = 1;
        if (k > 255)
            k = 255;
        gEnt->s.solid = i | (j << 8) | (k << 16);
        if (gEnt->s.solid == 0xFFFFFF)
            gEnt->s.solid = 1;
    }
    else
    {
        gEnt->s.solid = 0;
    }
    currentAngles = gEnt->r.currentAngles;
    currentOrigin = gEnt->r.currentOrigin;
    angles = (float *)LODWORD(gEnt->r.currentAngles[0]);
    if (((unsigned int)angles & 0x7F800000) == 0x7F800000
        || (v55 = *((_DWORD *)currentAngles + 1), (v55 & 0x7F800000) == 0x7F800000)
        || (v54 = *((_DWORD *)currentAngles + 2), (v54 & 0x7F800000) == 0x7F800000))
    {
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp",
            131,
            0,
            "%s",
            "!IS_NAN((angles)[0]) && !IS_NAN((angles)[1]) && !IS_NAN((angles)[2])"))
            __debugbreak();
    }
    v53 = *currentOrigin;
    if ((LODWORD(v53) & 0x7F800000) == 0x7F800000
        || (v52 = *((_DWORD *)currentOrigin + 1), (v52 & 0x7F800000) == 0x7F800000)
        || (v51 = *((_DWORD *)currentOrigin + 2), (v51 & 0x7F800000) == 0x7F800000))
    {
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp",
            132,
            0,
            "%s",
            "!IS_NAN((origin)[0]) && !IS_NAN((origin)[1]) && !IS_NAN((origin)[2])"))
            __debugbreak();
    }
    SnapAngles(currentAngles);
    if (gEnt->r.bmodel)
    {
        Phys_Vec3ToNitrousVec(gEnt->r.mins, mn);
        Phys_Vec3ToNitrousVec(gEnt->r.maxs, mx);
        v48 = mx.x - mn.x;
        v47 = mx.y - mn.y;
        v46 = mx.z - mn.z;
        v43 = mx.x - mn.x;
        v44 = mx.y - mn.y;
        v45 = mx.z - mn.z;
        v42 = 0.5 * (float)(mx.x - mn.x);
        v41 = 0.5 * (float)(mx.y - mn.y);
        v40 = 0.5 * (float)(mx.z - mn.z);
        rvec_4 = v42;
        rvec_8 = v41;
        rvec_12 = v40;
        rvec.x = v42;
        rvec.y = v41;
        rvec.z = v40;
        v35 = mx.x + mn.x;
        v34 = mx.y + mn.y;
        v33 = mx.z + mn.z;
        v30 = mx.x + mn.x;
        v31 = mx.y + mn.y;
        v32 = mx.z + mn.z;
        v29 = 0.5 * (float)(mx.x + mn.x);
        v28 = 0.5 * (float)(mx.y + mn.y);
        v27 = 0.5 * (float)(mx.z + mn.z);
        ctr_4 = v29;
        ctr_8 = v28;
        ctr_12 = v27;
        ctr.x = v29;
        ctr.y = v28;
        ctr.z = v27;
        AnglesToAxis(gEnt->r.currentAngles, (float (*)[3])v22);
        Phys_AxisToNitrousMat((float (*)[3])v22, xform);
        Phys_Vec3ToNitrousVec(gEnt->r.currentOrigin, xform.w);
        phys_calc_world_aabb(ctr, rvec, xform, &aabb_mn, &aabb_mn_4);
        Phys_NitrousVecToVec3(aabb_mn, gEnt->r.absmin);
        Phys_NitrousVecToVec3(aabb_mn_4, gEnt->r.absmax);
    }
    else if (gEnt->sentient)
    {
        v18 = gEnt->r.absmin;
        mins = gEnt->r.mins;
        gEnt->r.absmin[0] = *currentOrigin + gEnt->r.mins[0];
        v18[1] = currentOrigin[1] + mins[1];
        v18[2] = currentOrigin[2] + mins[2];
        v16 = gEnt->r.absmax;
        maxs = gEnt->r.maxs;
        gEnt->r.absmax[0] = *currentOrigin + gEnt->r.maxs[0];
        v16[1] = currentOrigin[1] + maxs[1];
        v16[2] = currentOrigin[2] + maxs[2];
    }
    else
    {
        max = RadiusFromBounds(gEnt->r.mins, gEnt->r.maxs);
        for (m = 0; m < 3; ++m)
        {
            gEnt->r.absmin[m] = currentOrigin[m] - max;
            gEnt->r.absmax[m] = currentOrigin[m] + max;
        }
    }
    gEnt->r.absmin[0] = gEnt->r.absmin[0] - 1.0;
    gEnt->r.absmin[1] = gEnt->r.absmin[1] - 1.0;
    gEnt->r.absmin[2] = gEnt->r.absmin[2] - 1.0;
    gEnt->r.absmax[0] = gEnt->r.absmax[0] + 1.0;
    gEnt->r.absmax[1] = gEnt->r.absmax[1] + 1.0;
    gEnt->r.absmax[2] = gEnt->r.absmax[2] + 1.0;
    ent->numClusters = 0;
    ent->lastCluster = 0;
    if ((gEnt->r.svFlags & 0x19) == 0)
    {
        num_leafs = CM_BoxLeafnums(gEnt->r.absmin, gEnt->r.absmax, leafs, 128, &lastLeaf);
        if (!num_leafs)
        {
        LABEL_42:
            CM_UnlinkEntity(ent);
            return;
        }
        for (*(_DWORD *)&cluster[2] = 0; *(int *)&cluster[2] < num_leafs; ++*(_DWORD *)&cluster[2])
        {
            *(_DWORD *)cluster = CM_LeafCluster(leafs[*(_DWORD *)&cluster[2]]);
            if (*(_DWORD *)cluster != -1)
            {
                ent->clusternums[ent->numClusters++] = *(_DWORD *)cluster;
                if (ent->numClusters == 16)
                    break;
            }
        }
        if (*(_DWORD *)&cluster[2] != num_leafs)
        {
            v2 = CM_LeafCluster(lastLeaf);
            ent->lastCluster = v2;
        }
    }
    gEnt->r.linked = 1;
    if (!gEnt->r.contents)
        goto LABEL_42;
    clipHandle = SV_ClipHandleForEntity(gEnt);
    absmin = Com_GetServerDObj(gEnt->s.number);
    if (absmin && (gEnt->r.svFlags & 6) != 0)
    {
        if ((gEnt->r.svFlags & 2) != 0)
        {
            absmax[0] = *currentOrigin + actorLocationalMins[0];
            absmax[1] = currentOrigin[1] + actorLocationalMins[1];
            absmax[2] = currentOrigin[2] + actorLocationalMins[2];
            v3[0] = *currentOrigin + actorLocationalMaxs[0];
            v3[1] = currentOrigin[1] + actorLocationalMaxs[1];
            v3[2] = currentOrigin[2] + actorLocationalMaxs[2];
        }
        else
        {
            DObjGetBounds(absmin, absmax, v3);
            absmax[0] = *currentOrigin + absmax[0];
            absmax[1] = currentOrigin[1] + absmax[1];
            absmax[2] = currentOrigin[2] + absmax[2];
            v3[0] += currentOrigin[0];
            v3[1] += currentOrigin[1];
            v3[2] += currentOrigin[2];
        }
        CM_LinkEntity(ent, absmax, v3, clipHandle);
    }
    else
    {
        CM_LinkEntity(ent, gEnt->r.absmin, gEnt->r.absmax, clipHandle);
    }
}

void __cdecl SnapAngles(float *vAngles)
{
    float delta; // [esp+10h] [ebp-Ch]
    int rounded; // [esp+14h] [ebp-8h]
    int i; // [esp+18h] [ebp-4h]

    for ( i = 0; i < 3; ++i )
    {
        rounded = SnapFloatToInt(vAngles[i]);
        delta = (float)rounded - vAngles[i];
        if ( (float)(delta * delta) < 0.0000010000001 )
            vAngles[i] = (float)rounded;
    }
}

void __cdecl SV_TraceCapsuleToEntity(const moveclip_t *clip, svEntity_s *check, trace_t *trace)
{
    DObj *ServerDObj; // [esp+4h] [ebp-4Ch]
    unsigned __int16 v4; // [esp+8h] [ebp-48h]
    unsigned __int16 number; // [esp+Ah] [ebp-46h]
    PhysGeomList *geoms; // [esp+1Ch] [ebp-34h]
    gentity_s *touch; // [esp+20h] [ebp-30h]
    unsigned int clipHandlea; // [esp+24h] [ebp-2Ch]
    unsigned int clipHandle; // [esp+24h] [ebp-2Ch]
    float absmin[3]; // [esp+28h] [ebp-28h] BYREF
    DObj *dobj; // [esp+34h] [ebp-1Ch]
    const float *angles; // [esp+38h] [ebp-18h]
    float absmax[3]; // [esp+3Ch] [ebp-14h] BYREF
    int entnum; // [esp+48h] [ebp-8h]
    float oldFraction; // [esp+4Ch] [ebp-4h]

    entnum = ((char *)check - (char *)sv.svEntities) / sizeof(svEntity_s);
    //touch = (gentity_s *)(sv.bpsWindow[8] + entnum * sv.bpsWindow[9]);
    touch = SV_GentityNum(entnum);

    if ( (touch->r.contents & clip->contentmask) != 0
        && (clip->passEntityNum == 1023
         || entnum != clip->passEntityNum
         && (!touch->r.ownerNum.isDefined()
            || touch->r.ownerNum.entnum() != clip->passEntityNum
            && touch->r.ownerNum.entnum() != clip->passOwnerNum)) )
    {
        absmin[0] = touch->r.absmin[0] + clip->mins[0];
        absmin[1] = touch->r.absmin[1] + clip->mins[1];
        absmin[2] = touch->r.absmin[2] + clip->mins[2];
        absmax[0] = touch->r.absmax[0] + clip->maxs[0];
        absmax[1] = touch->r.absmax[1] + clip->maxs[1];
        absmax[2] = touch->r.absmax[2] + clip->maxs[2];
        if ( intersect_extents_aabb(&clip->extents, absmin, absmax, trace->fraction) )
        {
            if ( BG_ClipMoveToDobj(&touch->s) )
                ServerDObj = Com_GetServerDObj(touch->s.number);
            else
                ServerDObj = 0;
            dobj = ServerDObj;
            if ( ServerDObj )
            {
                if ( DObjHasCollmap(dobj) )
                {
                    if ( DObjHasContents(dobj, clip->contentmask) )
                    {
                        geoms = DObjGetCollmap(dobj);
                        if ( geoms )
                        {
                            clipHandlea = CM_TempBrushModel(geoms);
                            oldFraction = trace->fraction;
                            CM_TransformedBoxTrace(
                                trace,
                                clip->extents.start.vec.v,
                                clip->extents.end.vec.v,
                                clip->mins,
                                clip->maxs,
                                clipHandlea,
                                clip->contentmask,
                                touch->r.currentOrigin,
                                touch->r.currentAngles);
                            if ( trace->fraction < oldFraction )
                            {
                                if ( touch->s.number != LOWORD(touch->s.number)
                                    && !Assert_MyHandler(
                                                "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp",
                                                350,
                                                0,
                                                "%s",
                                                "touch->s.number == static_cast<unsigned short>( touch->s.number )") )
                                {
                                    __debugbreak();
                                }
                                number = touch->s.number;
                                if ( !trace
                                    && !Assert_MyHandler(
                                                "c:\\projects_pc\\cod\\codsrc\\src\\server_mp\\../qcommon/cm_public.h",
                                                175,
                                                0,
                                                "%s",
                                                "trace") )
                                {
                                    __debugbreak();
                                }
                                trace->hitType = TRACE_HITTYPE_ENTITY;
                                trace->hitId = number;
                            }
                        }
                    }
                }
            }
            else if ( touch->r.bmodel || touch->s.eType != 6 )
            {
                clipHandle = SV_ClipHandleForEntity(touch);
                angles = touch->r.currentAngles;
                if ( !touch->r.bmodel )
                    angles = vec3_origin;
                oldFraction = trace->fraction;
                CM_TransformedBoxTrace(
                    trace,
                    clip->extents.start.vec.v,
                    clip->extents.end.vec.v,
                    clip->mins,
                    clip->maxs,
                    clipHandle,
                    clip->contentmask,
                    touch->r.currentOrigin,
                    angles);
                if ( trace->fraction < oldFraction )
                {
                    if ( touch->s.number != LOWORD(touch->s.number)
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp",
                                    376,
                                    0,
                                    "%s",
                                    "touch->s.number == static_cast<unsigned short>( touch->s.number )") )
                    {
                        __debugbreak();
                    }
                    v4 = touch->s.number;
                    if ( !trace
                        && !Assert_MyHandler(
                                    "c:\\projects_pc\\cod\\codsrc\\src\\server_mp\\../qcommon/cm_public.h",
                                    175,
                                    0,
                                    "%s",
                                    "trace") )
                    {
                        __debugbreak();
                    }
                    trace->hitType = TRACE_HITTYPE_ENTITY;
                    trace->hitId = v4;
                }
            }
        }
    }
}

void __cdecl SV_TracePointToEntity(const pointtrace_t *clip, svEntity_s *check, trace_t *trace)
{
    const char *v3; // eax
    unsigned __int16 number; // [esp+26h] [ebp-126h]
    unsigned __int16 v5; // [esp+4Ah] [ebp-102h]
    float v6; // [esp+50h] [ebp-FCh]
    float *v7; // [esp+68h] [ebp-E4h]
    float *v8; // [esp+70h] [ebp-DCh]
    gentity_s *touch; // [esp+A0h] [ebp-ACh]
    float entAxis[4][3]; // [esp+A4h] [ebp-A8h] BYREF
    unsigned int clipHandle; // [esp+D4h] [ebp-78h]
    DObj *obj; // [esp+D8h] [ebp-74h]
    float absmin[3]; // [esp+DCh] [ebp-70h] BYREF
    float localStart[3]; // [esp+E8h] [ebp-64h] BYREF
    const float *angles; // [esp+F4h] [ebp-58h]
    DObjTrace_s objTrace; // [esp+F8h] [ebp-54h] BYREF
    float localEnd[3]; // [esp+118h] [ebp-34h] BYREF
    float absmax[3]; // [esp+124h] [ebp-28h] BYREF
    int entnum; // [esp+130h] [ebp-1Ch]
    int partBits[5]; // [esp+134h] [ebp-18h] BYREF
    float oldFraction; // [esp+148h] [ebp-4h]

    entnum = ((char *)check - (char *)sv.svEntities) / sizeof(svEntity_s);
    //touch = (gentity_s *)(sv.bpsWindow[8] + entnum * sv.bpsWindow[9]);
    touch = SV_GentityNum(entnum);

    if ( (touch->r.contents & clip->contentmask) == 0
        || clip->ignoreEntParams
        && clip->ignoreEntParams->baseEntity != 1023
        && (clip->ignoreEntParams->ignoreSelf && entnum == clip->ignoreEntParams->baseEntity
         || clip->ignoreEntParams->ignoreParent && entnum == clip->ignoreEntParams->parentEntity
         || touch->r.ownerNum.isDefined()
         && (clip->ignoreEntParams->ignoreSiblings
            && touch->r.ownerNum.entnum() == clip->ignoreEntParams->parentEntity
            && entnum != clip->ignoreEntParams->baseEntity
            || clip->ignoreEntParams->ignoreChildren
            && touch->r.ownerNum.entnum() == clip->ignoreEntParams->baseEntity)))
    {
        return;
    }
    obj = SV_LocationalTraceDObj(clip, touch);
    if ( !obj )
    {
        PROF_SCOPED("SV_TracePointToEntity 2");
        if ( touch->r.bmodel || touch->s.eType != 6 )
        {
            if ( (check->linkcontents & clip->contentmask) == 0 )
            {
                return;
            }
            if ( !intersect_extents_aabb(&clip->extents, touch->r.absmin, touch->r.absmax, trace->fraction) )
            {
                return;
            }
            clipHandle = SV_ClipHandleForEntity(touch);
            angles = touch->r.currentAngles;
            if ( !touch->r.bmodel )
                angles = vec3_origin;
            oldFraction = trace->fraction;
            CM_TransformedBoxTrace(
                trace,
                clip->extents.start.vec.v,
                clip->extents.end.vec.v,
                vec3_origin,
                vec3_origin,
                clipHandle,
                clip->contentmask,
                touch->r.currentOrigin,
                angles);
            if ( trace->fraction >= oldFraction )
            {
                return;
            }
            trace->modelIndex = 0;
            trace->partName = 0;
            trace->boneIndex = 0;
            trace->partGroup = 0;
            if ( touch->s.number != LOWORD(touch->s.number)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp",
                            577,
                            0,
                            "%s",
                            "touch->s.number == static_cast<unsigned short>( touch->s.number )") )
            {
                __debugbreak();
            }
            number = touch->s.number;
            if ( !trace
                && !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\server_mp\\../qcommon/cm_public.h",
                            175,
                            0,
                            "%s",
                            "trace") )
            {
                __debugbreak();
            }
            trace->hitType = TRACE_HITTYPE_ENTITY;
            trace->hitId = number;
            trace->cflags = touch->r.contents;
        }
        return;
    }
    PROF_SCOPED("SV_TracePointToEntity 1");
    if ( (touch->r.svFlags & 4) != 0 )
    {
        if ( !DObjHasContents(obj, clip->contentmask) )
        {
            return;
        }
        absmin[0] = touch->r.absmin[0];
        absmin[1] = touch->r.absmin[1];
        absmin[2] = touch->r.absmin[2];
        absmax[0] = touch->r.absmax[0];
        absmax[1] = touch->r.absmax[1];
        absmax[2] = touch->r.absmax[2];
        entAxis[3][0] = touch->r.currentOrigin[0];
        entAxis[3][1] = touch->r.currentOrigin[1];
        entAxis[3][2] = touch->r.currentOrigin[2];
    }
    else
    {
        if ( !clip->priorityMap
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp", 485, 0, "%s", "clip->priorityMap") )
        {
            __debugbreak();
        }
        entAxis[3][0] = touch->r.currentOrigin[0];
        entAxis[3][1] = touch->r.currentOrigin[1];
        entAxis[3][2] = touch->r.currentOrigin[2];
        if ( touch->client )
            v8 = actorLocationalMinsBig;
        else
            v8 = actorLocationalMins;
        absmin[0] = entAxis[3][0] + *v8;
        absmin[1] = entAxis[3][1] + v8[1];
        absmin[2] = entAxis[3][2] + v8[2];
        if ( touch->client )
            v7 = actorLocationalMaxsBig;
        else
            v7 = actorLocationalMaxs;
        absmax[0] = entAxis[3][0] + *v7;
        absmax[1] = entAxis[3][1] + v7[1];
        absmax[2] = entAxis[3][2] + v7[2];
    }
    if ( !intersect_extents_aabb(&clip->extents, absmin, absmax, trace->fraction) )
    {
        return;
    }
    AnglesToAxis(touch->r.currentAngles, entAxis);
    MatrixTransposeTransformVector43(clip->extents.start.vec.v, entAxis, localStart);
    MatrixTransposeTransformVector43(clip->extents.end.vec.v, entAxis, localEnd);
    objTrace.fraction = trace->fraction;
    if ( (touch->r.svFlags & 4) != 0 )
    {
        DObjGeomTracelinePartBits(obj, clip->contentmask, partBits);
        G_DObjCalcPose(touch, partBits);
        DObjGeomTraceline(obj, localStart, localEnd, clip->contentmask, &objTrace);
    }
    else
    {
        DObjTracelinePartBits(obj, partBits);
        G_DObjCalcPose(touch, partBits);
        DObjTraceline(obj, localStart, localEnd, clip->priorityMap, &objTrace);
    }
    if ( g_debugLocDamage->current.integer == 2 )
        SV_XModelDebugBoxes(touch, colorWhite, partBits, 50);
    if ( objTrace.fraction >= trace->fraction )
    {
        return;
    }
    if ( (objTrace.fraction >= 1.0 || objTrace.fraction < 0.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp",
                    526,
                    0,
                    "%s\n\t(objTrace.fraction) = %g",
                    "(objTrace.fraction < 1.0f && objTrace.fraction >= 0)",
                    objTrace.fraction) )
    {
        __debugbreak();
    }
    trace->fraction = objTrace.fraction;
    trace->sflags = objTrace.sflags;
    trace->modelIndex = objTrace.modelIndex;
    trace->partName = objTrace.partName;
    trace->boneIndex = objTrace.localBoneIndex;
    trace->partGroup = objTrace.partGroup;
    MatrixTransformVector(objTrace.normal, entAxis, trace->normal.vec.v);
    v6 = Vec3Length(trace->normal.vec.v) - 1.0;
    if ( fabs(v6) >= 0.01 && Vec3Length(trace->normal.vec.v) >= 0.01 )
    {
        v3 = va("%g %g %g", trace->normal.vec.v[0], trace->normal.vec.v[1], trace->normal.vec.v[2]);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp",
                        535,
                        0,
                        "%s\n\t%s",
                        "(I_fabs( Vec3Length( trace->normal ) - 1.0f ) < 0.01) || (Vec3Length( trace->normal ) < 0.01)",
                        v3) )
            __debugbreak();
    }
    trace->walkable = trace->normal.vec.v[2] >= 0.69999999;
    if ( touch->s.number != LOWORD(touch->s.number)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp",
                    538,
                    0,
                    "%s",
                    "touch->s.number == static_cast<unsigned short>( touch->s.number )") )
    {
        __debugbreak();
    }
    v5 = touch->s.number;
    if ( !trace
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\server_mp\\../qcommon/cm_public.h", 175, 0, "%s", "trace") )
    {
        __debugbreak();
    }
    trace->hitType = TRACE_HITTYPE_ENTITY;
    trace->hitId = v5;
    trace->cflags = touch->r.contents;
}

DObj *__cdecl SV_LocationalTraceDObj(const pointtrace_t *clip, const gentity_s *touch)
{
    if ( !clip->bLocational )
        return 0;
    if ( (touch->r.svFlags & 6) == 0 )
        return 0;
    if ( (touch->r.svFlags & 2) == 0 || clip->priorityMap )
        return Com_GetServerDObj(touch->s.number);
    return 0;
}

int __cdecl SV_SightTraceCapsuleToEntity(const sightclip_t *clip, int entnum)
{
    float v3; // xmm0_4
    int v4; // eax
    DObj *ServerDObj; // [esp+4h] [ebp-3Ch]
    PhysGeomList *geoms; // [esp+Ch] [ebp-34h]
    int i; // [esp+10h] [ebp-30h]
    gentity_s *touch; // [esp+14h] [ebp-2Ch]
    unsigned int clipHandlea; // [esp+1Ch] [ebp-24h]
    unsigned int clipHandle; // [esp+1Ch] [ebp-24h]
    float boxmins[3]; // [esp+20h] [ebp-20h]
    DObj *dobj; // [esp+2Ch] [ebp-14h]
    float boxmaxs[3]; // [esp+30h] [ebp-10h]
    const float *angles; // [esp+3Ch] [ebp-4h]

    touch = (gentity_s *)((char *)sv.gentities + entnum * sv.gentitySize);
    if ( (touch->r.contents & clip->contentmask) == 0 )
        return 0;
    if ( clip->passEntityNum[0] != 1023 )
    {
        if ( entnum == clip->passEntityNum[0] )
            return 0;
        if ( touch->r.ownerNum.isDefined() && touch->r.ownerNum.entnum() == clip->passEntityNum[0])
            return 0;
    }
    if ( clip->passEntityNum[1] != 1023 )
    {
        if ( entnum == clip->passEntityNum[1] )
            return 0;
        if ( touch->r.ownerNum.isDefined() && touch->r.ownerNum.entnum() == clip->passEntityNum[1])
            return 0;
    }
    for ( i = 0; i < 3; ++i )
    {
        if ( clip->end[i] <= clip->start[i] )
        {
            boxmins[i] = (float)(clip->end[i] + clip->mins[i]) - 1.0;
            v3 = (float)(clip->start[i] + clip->maxs[i]) + 1.0;
        }
        else
        {
            boxmins[i] = (float)(clip->start[i] + clip->mins[i]) - 1.0;
            v3 = (float)(clip->end[i] + clip->maxs[i]) + 1.0;
        }
        boxmaxs[i] = v3;
    }
    if ( touch->r.absmin[0] > boxmaxs[0]
        || touch->r.absmin[1] > boxmaxs[1]
        || touch->r.absmin[2] > boxmaxs[2]
        || boxmins[0] > touch->r.absmax[0]
        || boxmins[1] > touch->r.absmax[1]
        || boxmins[2] > touch->r.absmax[2] )
    {
        return 0;
    }
    angles = touch->r.currentAngles;
    if ( BG_ClipMoveToDobj(&touch->s) )
        ServerDObj = Com_GetServerDObj(touch->s.number);
    else
        ServerDObj = 0;
    dobj = ServerDObj;
    if ( ServerDObj )
    {
        if ( !DObjHasCollmap(dobj) )
            return 0;
        if ( !DObjHasContents(dobj, clip->contentmask) )
            return 0;
        geoms = DObjGetCollmap(dobj);
        if ( !geoms )
            return 0;
        clipHandlea = CM_TempBrushModel(geoms);
        v4 = CM_TransformedBoxSightTrace(
                     0,
                     clip->start,
                     clip->end,
                     clip->mins,
                     clip->maxs,
                     clipHandlea,
                     clip->contentmask,
                     touch->r.currentOrigin,
                     angles);
    }
    else
    {
        clipHandle = SV_ClipHandleForEntity(touch);
        if ( !touch->r.bmodel )
            angles = vec3_origin;
        v4 = CM_TransformedBoxSightTrace(
                     0,
                     clip->start,
                     clip->end,
                     clip->mins,
                     clip->maxs,
                     clipHandle,
                     clip->contentmask,
                     touch->r.currentOrigin,
                     angles);
    }
    if ( v4 )
        return -1;
    else
        return 0;
}

int __cdecl SV_SightTracePointToEntity(const sightpointtrace_t *clip, int entnum)
{
    int v3; // eax
    DObj *v4; // [esp+4h] [ebp-124h]
    float *v5; // [esp+Ch] [ebp-11Ch]
    float *v6; // [esp+14h] [ebp-114h]
    PhysGeomList *geoms; // [esp+40h] [ebp-E8h]
    gentity_s *touch; // [esp+48h] [ebp-E0h]
    float entAxis[4][3]; // [esp+50h] [ebp-D8h] BYREF
    unsigned int clipHandle; // [esp+80h] [ebp-A8h]
    TraceExtents extents; // [esp+84h] [ebp-A4h] BYREF
    DObj *obj; // [esp+B8h] [ebp-70h]
    float absmin[3]; // [esp+BCh] [ebp-6Ch] BYREF
    float localStart[3]; // [esp+C8h] [ebp-60h] BYREF
    const float *angles; // [esp+D4h] [ebp-54h]
    DObjTrace_s objTrace; // [esp+D8h] [ebp-50h] BYREF
    float localEnd[3]; // [esp+F8h] [ebp-30h] BYREF
    float absmax[3]; // [esp+104h] [ebp-24h] BYREF
    int passEntityIdx; // [esp+110h] [ebp-18h]
    int partBits[5]; // [esp+114h] [ebp-14h] BYREF

    //TraceExtents::TraceExtents(&extents);
    touch = (gentity_s *)((char *)sv.gentities + entnum * sv.gentitySize);
    if ( (touch->r.contents & clip->contentmask) == 0 )
        return 0;
    for ( passEntityIdx = 0; passEntityIdx < 2; ++passEntityIdx )
    {
        if ( clip->passEntityNum[passEntityIdx] != 1023 )
        {
            if ( entnum == clip->passEntityNum[passEntityIdx] )
                return 0;
            if ( touch->r.ownerNum.isDefined() )
            {
                v3 = touch->r.ownerNum.entnum();
                if ( v3 == clip->passEntityNum[passEntityIdx] )
                    return 0;
            }
        }
    }
    obj = SV_LocationalSightTraceDObj(clip, touch);
    if ( obj )
    {
        if ( (touch->r.svFlags & 4) != 0 )
        {
            if ( !DObjHasContents(obj, clip->contentmask) )
                return 0;
            entAxis[3][0] = touch->r.currentOrigin[0];
            entAxis[3][1] = touch->r.currentOrigin[1];
            entAxis[3][2] = touch->r.currentOrigin[2];
            DObjGetBounds(obj, absmin, absmax);
            absmin[0] = entAxis[3][0] + absmin[0];
            absmin[1] = entAxis[3][1] + absmin[1];
            absmin[2] = entAxis[3][2] + absmin[2];
            absmax[0] = entAxis[3][0] + absmax[0];
            absmax[1] = entAxis[3][1] + absmax[1];
            absmax[2] = entAxis[3][2] + absmax[2];
        }
        else
        {
            if ( !clip->priorityMap
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp",
                            721,
                            0,
                            "%s",
                            "clip->priorityMap") )
            {
                __debugbreak();
            }
            entAxis[3][0] = touch->r.currentOrigin[0];
            entAxis[3][1] = touch->r.currentOrigin[1];
            entAxis[3][2] = touch->r.currentOrigin[2];
            if ( touch->client )
                v6 = actorLocationalMinsBig;
            else
                v6 = actorLocationalMins;
            absmin[0] = entAxis[3][0] + *v6;
            absmin[1] = entAxis[3][1] + v6[1];
            absmin[2] = entAxis[3][2] + v6[2];
            if ( touch->client )
                v5 = actorLocationalMaxsBig;
            else
                v5 = actorLocationalMaxs;
            absmax[0] = entAxis[3][0] + *v5;
            absmax[1] = entAxis[3][1] + v5[1];
            absmax[2] = entAxis[3][2] + v5[2];
        }
        *(_QWORD *)extents.start.vec.v = *(_QWORD *)clip->start;
        extents.start.vec.u[2] = LODWORD(clip->start[2]);
        *(_QWORD *)extents.end.vec.v = *(_QWORD *)clip->end;
        extents.end.vec.u[2] = LODWORD(clip->end[2]);
        CM_CalcTraceExtents(&extents);
        if ( !intersect_extents_aabb(&extents, absmin, absmax, 1.0) )
            return 0;
        AnglesToAxis(touch->r.currentAngles, entAxis);
        MatrixTransposeTransformVector43(extents.start.vec.v, entAxis, localStart);
        MatrixTransposeTransformVector43(extents.end.vec.v, entAxis, localEnd);
        objTrace.fraction = 1.0f;
        if ( (touch->r.svFlags & 4) != 0 )
        {
            DObjGeomTracelinePartBits(obj, clip->contentmask, partBits);
            G_DObjCalcPose(touch, partBits);
            DObjGeomTraceline(obj, localStart, localEnd, clip->contentmask, &objTrace);
        }
        else
        {
            DObjTracelinePartBits(obj, partBits);
            G_DObjCalcPose(touch, partBits);
            DObjTraceline(obj, localStart, localEnd, clip->priorityMap, &objTrace);
        }
        if ( objTrace.fraction < 1.0 )
            return -1;
    }
    else if ( touch->r.bmodel || touch->s.eType != 6 )
    {
        v4 = touch->s.eType == 14 ? Com_GetServerDObj(touch->s.number) : 0;
        if ( v4 )
        {
            if ( !DObjHasCollmap(v4) )
                return 0;
            geoms = DObjGetCollmap(v4);
            if ( !geoms )
                return 0;
            clipHandle = CM_TempBrushModel(geoms);
            angles = touch->r.currentAngles;
        }
        else
        {
            clipHandle = SV_ClipHandleForEntity(touch);
            angles = touch->r.currentAngles;
            if ( !touch->r.bmodel )
                angles = vec3_origin;
        }
        if ( CM_TransformedBoxSightTrace(
                     0,
                     clip->start,
                     clip->end,
                     vec3_origin,
                     vec3_origin,
                     clipHandle,
                     clip->contentmask,
                     touch->r.currentOrigin,
                     angles) )
        {
            return -1;
        }
    }
    return 0;
}

DObj *__cdecl SV_LocationalSightTraceDObj(const sightpointtrace_t *clip, const gentity_s *touch)
{
    if ( !clip->locational )
        return 0;
    if ( (touch->r.svFlags & 6) == 0 )
        return 0;
    if ( (touch->r.svFlags & 2) == 0 || clip->priorityMap )
        return Com_GetServerDObj(touch->s.number);
    return 0;
}

void __cdecl SV_SetupIgnoreEntParams(IgnoreEntParams *ignoreEntParams, int baseEntity)
{
    gentity_s *base; // [esp+4h] [ebp-4h]

    ignoreEntParams->baseEntity = baseEntity;
    if ( baseEntity == 1023 )
    {
        ignoreEntParams->parentEntity = -1;
    }
    else
    {
        base = (gentity_s *)((char *)sv.gentities + baseEntity * sv.gentitySize);
        if ( base->r.ownerNum.isDefined() )
            ignoreEntParams->parentEntity = base->r.ownerNum.entnum();
        else
            ignoreEntParams->parentEntity = -1;
    }
    ignoreEntParams->ignoreSelf = 1;
    ignoreEntParams->ignoreChildren = 1;
    ignoreEntParams->ignoreSiblings = 1;
    ignoreEntParams->ignoreParent = 0;
}

void __cdecl SV_TracePoint(trace_t *results, const float *start, const float *end, col_context_t *context)
{
    IgnoreEntParams *ignoreEntParams; // esi
    const char *v5; // eax
    int v6; // [esp+4h] [ebp-9Ch]
    pointtrace_t clip; // [esp+58h] [ebp-48h] BYREF

    PROF_SCOPED("SV_TracePoint");

    nanassertvec3(start);
    nanassertvec3(end);

    CM_BoxTrace(results, start, end, vec3_origin, vec3_origin, context->mask, context);
    if ( (LODWORD(results->fraction) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp",
                    838,
                    0,
                    "%s",
                    "!IS_NAN(results->fraction)") )
    {
        __debugbreak();
    }
    if ( results->fraction == 1.0 )
    {
        if ( !results
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\server_mp\\../qcommon/cm_public.h",
                        175,
                        0,
                        "%s",
                        "trace") )
        {
            __debugbreak();
        }
        results->hitType = TRACE_HITTYPE_NONE;
        results->hitId = 0;
    }
    else
    {
        if ( !results
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\server_mp\\../qcommon/cm_public.h",
                        175,
                        0,
                        "%s",
                        "trace") )
        {
            __debugbreak();
        }
        results->hitType = TRACE_HITTYPE_ENTITY;
        results->hitId = 1022;
    }
    if ( results->fraction == 0.0 )
    {
        return;
    }
    //TraceExtents::TraceExtents(&clip.extents);
    if ( context->staticmodels )
    {
        CM_PointTraceStaticModels(results, start, end, context->mask);
        if ( (LODWORD(results->fraction) & 0x7F800000) == 0x7F800000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp",
                        854,
                        0,
                        "%s",
                        "!IS_NAN(results->fraction)") )
        {
            __debugbreak();
        }
        if ( results->fraction == 0.0 )
        {
            return;
        }
    }
    clip.contentmask = context->mask;
    *(_QWORD *)clip.extents.start.vec.v = *(_QWORD *)start;
    clip.extents.start.vec.v[2] = start[2];
    *(_QWORD *)clip.extents.end.vec.v = *(_QWORD *)end;
    clip.extents.end.vec.v[2] = end[2];
    CM_CalcTraceExtents(&clip.extents);
    clip.ignoreEntParams = context->ignoreEntParams;
    clip.bLocational = context->locational;
    clip.priorityMap = context->priorityMap;
    
    if (context->ignoreEntParams->baseEntity != 1023 && context->ignoreEntParams->parentEntity != -1)
    {
        //if (!EntHandle::isDefined((EntHandle *)((char *)&sv.gentities->r.ownerNum + context->ignoreEntParams->baseEntity * sv.gentitySize))
        if (!(((EntHandle *)((char *)&sv.gentities->r.ownerNum + context->ignoreEntParams->baseEntity * sv.gentitySize))->isDefined())
            //|| (ignoreEntParams = context->ignoreEntParams, ignoreEntParams->parentEntity != EntHandle::entnum((EntHandle *)((char *)&sv.gentities->r.ownerNum + ignoreEntParams->baseEntity * sv.gentitySize))))
            || (ignoreEntParams = context->ignoreEntParams, ignoreEntParams->parentEntity != ((EntHandle *)((char *)&sv.gentities->r.ownerNum + ignoreEntParams->baseEntity * sv.gentitySize))->entnum()))
        {
            //if (EntHandle::isDefined((EntHandle *)((char *)&sv.gentities->r.ownerNum + context->ignoreEntParams->baseEntity * sv.gentitySize)))
            if ((((EntHandle *)((char *)&sv.gentities->r.ownerNum + context->ignoreEntParams->baseEntity * sv.gentitySize)))->isDefined())
            {
                //v7 = EntHandle::entnum((EntHandle *)((char *)&sv.gentities->r.ownerNum + context->ignoreEntParams->baseEntity * sv.gentitySize)->entnum());
                v5 = va(
                    "base: %d; parent: %d; base's parent: %d\n",
                    context->ignoreEntParams->baseEntity,
                    context->ignoreEntParams->parentEntity,
                    ((EntHandle *)((char *)&sv.gentities->r.ownerNum + context->ignoreEntParams->baseEntity * sv.gentitySize))->entnum());
            }
            else
            {
                v5 = va(
                    "base: %d; parent: %d; base's parent: %d\n",
                    context->ignoreEntParams->baseEntity,
                    context->ignoreEntParams->parentEntity,
                    1023);
            }
            if (!Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp",
                867,
                0,
                "%s\n\t%s",
                "context.ignoreEntParams->baseEntity == ENTITYNUM_NONE || context.ignoreEntParams->parentEntity == -1 || (S"
                "V_GentityNum( context.ignoreEntParams->baseEntity )->r.ownerNum.isDefined() && (uint)context.ignoreEntPara"
                "ms->parentEntity == SV_GentityNum( context.ignoreEntParams->baseEntity )->r.ownerNum.entnum())",
                v5))
                __debugbreak();
        }
    }

    CM_PointTraceToEntities(&clip, results, context);
    DynEntSv_PointTrace(&clip, results);
    GlassSv_PointTrace(&clip, results);
}

void __cdecl SV_TraceCapsule(
                trace_t *results,
                const float *start,
                const float *mins,
                const float *maxs,
                const float *end,
                col_context_t *context)
{
    IgnoreEntParams *ignoreEntParams; // esi
    const char *v7; // eax
    int v8; // [esp+4h] [ebp-D8h]
    moveclip_t clip; // [esp+64h] [ebp-78h] BYREF
    float delta[3]; // [esp+D0h] [ebp-Ch]

    PROF_SCOPED("SV_TraceCapsule");

    if ( !mins && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp", 885, 0, "%s", "mins") )
        __debugbreak();
    if ( !maxs && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp", 885, 0, "%s", "maxs") )
        __debugbreak();
    if ( *maxs < *mins
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp", 885, 0, "%s", "maxs[0] >= mins[0]") )
    {
        __debugbreak();
    }
    if ( maxs[1] < mins[1]
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp", 885, 0, "%s", "maxs[1] >= mins[1]") )
    {
        __debugbreak();
    }
    if ( maxs[2] < mins[2]
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp", 885, 0, "%s", "maxs[2] >= mins[2]") )
    {
        __debugbreak();
    }

    iassert(!IS_NAN((mins)[0]) && !IS_NAN((mins)[1]) && !IS_NAN((mins)[2]));
    iassert(!IS_NAN((maxs)[0]) && !IS_NAN((maxs)[1]) && !IS_NAN((maxs)[2]));
    iassert(!IS_NAN((start)[0]) && !IS_NAN((start)[1]) && !IS_NAN((start)[2]));
    iassert(!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2]));

    CM_BoxTrace(results, start, end, mins, maxs, context->mask, context);

    if ( (LODWORD(results->fraction) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp",
                    892,
                    0,
                    "%s",
                    "!IS_NAN(results->fraction)") )
    {
        __debugbreak();
    }
    if ( results->fraction == 1.0 )
    {
        if ( !results
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\server_mp\\../qcommon/cm_public.h",
                        175,
                        0,
                        "%s",
                        "trace") )
        {
            __debugbreak();
        }
        results->hitType = TRACE_HITTYPE_NONE;
        results->hitId = 0;
    }
    else
    {
        if ( !results
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\server_mp\\../qcommon/cm_public.h",
                        175,
                        0,
                        "%s",
                        "trace") )
        {
            __debugbreak();
        }
        results->hitType = TRACE_HITTYPE_ENTITY;
        results->hitId = 1022;
    }
    if ( results->fraction == 0.0 )
    {
        return;
    }
    else
    {
        //TraceExtents::TraceExtents(&clip.extents);
        clip.contentmask = context->mask;
        clip.passEntityNum = context->ignoreEntParams->baseEntity;
        if (context->ignoreEntParams->baseEntity != 1023 && context->ignoreEntParams->parentEntity != -1)
        {
            //if (!EntHandle::isDefined((EntHandle *)((char *)&sv.gentities->r.ownerNum + context->ignoreEntParams->baseEntity * sv.gentitySize))
            if (!((EntHandle *)((char *)&sv.gentities->r.ownerNum + context->ignoreEntParams->baseEntity * sv.gentitySize))->isDefined()
                //|| (ignoreEntParams = context->ignoreEntParams, ignoreEntParams->parentEntity != EntHandle::entnum((EntHandle *)((char *)&sv.gentities->r.ownerNum + ignoreEntParams->baseEntity * sv.gentitySize))))
                || (ignoreEntParams = context->ignoreEntParams, ignoreEntParams->parentEntity != ((EntHandle *)((char *)&sv.gentities->r.ownerNum + ignoreEntParams->baseEntity * sv.gentitySize))->entnum()))
            {
                //if (EntHandle::isDefined((EntHandle *)((char *)&sv.gentities->r.ownerNum + context->ignoreEntParams->baseEntity * sv.gentitySize)))
                if ((((EntHandle *)((char *)&sv.gentities->r.ownerNum + context->ignoreEntParams->baseEntity * sv.gentitySize)))->isDefined())
                {
                    //v9 = EntHandle::entnum((EntHandle *)((char *)&sv.gentities->r.ownerNum + context->ignoreEntParams->baseEntity * sv.gentitySize));
                    v7 = va(
                        "base: %d; parent: %d; base's parent: %d\n",
                        context->ignoreEntParams->baseEntity,
                        context->ignoreEntParams->parentEntity,
                        ((EntHandle *)((char *)&sv.gentities->r.ownerNum + context->ignoreEntParams->baseEntity * sv.gentitySize))->entnum());
                }
                else
                {
                    v7 = va(
                        "base: %d; parent: %d; base's parent: %d\n",
                        context->ignoreEntParams->baseEntity,
                        context->ignoreEntParams->parentEntity,
                        1023);
                }
                if (!Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp",
                    907,
                    0,
                    "%s\n\t%s",
                    "context.ignoreEntParams->baseEntity == ENTITYNUM_NONE || context.ignoreEntParams->parentEntity == -1 || "
                    "(SV_GentityNum( context.ignoreEntParams->baseEntity )->r.ownerNum.isDefined() && (uint)context.ignoreEnt"
                    "Params->parentEntity == SV_GentityNum( context.ignoreEntParams->baseEntity )->r.ownerNum.entnum())",
                    v7))
                    __debugbreak();
            }
        }
        clip.passOwnerNum = context->ignoreEntParams->parentEntity;
        clip.outerSize[0] = *maxs - *mins;
        clip.outerSize[1] = maxs[1] - mins[1];
        clip.outerSize[2] = maxs[2] - mins[2];
        clip.maxs[0] = 0.5 * clip.outerSize[0];
        clip.maxs[1] = 0.5 * clip.outerSize[1];
        clip.maxs[2] = 0.5 * clip.outerSize[2];
        clip.mins[0] = -1.0 * (float)(0.5 * clip.outerSize[0]);
        clip.mins[1] = -1.0 * (float)(0.5 * clip.outerSize[1]);
        clip.mins[2] = -1.0 * (float)(0.5 * clip.outerSize[2]);
        clip.outerSize[0] = (float)(0.5 * clip.outerSize[0]) + 1.0;
        clip.outerSize[1] = (float)(0.5 * clip.outerSize[1]) + 1.0;
        clip.outerSize[2] = (float)(0.5 * clip.outerSize[2]) + 1.0;
        delta[0] = *maxs + *mins;
        delta[1] = maxs[1] + mins[1];
        delta[2] = maxs[2] + mins[2];
        delta[0] = 0.5 * delta[0];
        delta[1] = 0.5 * delta[1];
        delta[2] = 0.5 * delta[2];
        clip.extents.start.vec.v[0] = *start + delta[0];
        clip.extents.start.vec.v[1] = start[1] + delta[1];
        clip.extents.start.vec.v[2] = start[2] + delta[2];
        clip.extents.end.vec.v[0] = *end + delta[0];
        clip.extents.end.vec.v[1] = end[1] + delta[1];
        clip.extents.end.vec.v[2] = end[2] + delta[2];
        CM_CalcTraceExtents(&clip.extents);
        CM_ClipMoveToEntities(&clip, results);
        DynEntSv_ClipMoveTrace(&clip, results);
        GlassSv_ClipMoveTrace(&clip, results);
    }
}

bool __cdecl SV_SightTraceCapsule(
                int *hitNum,
                const float *start,
                const float *mins,
                const float *maxs,
                const float *end,
                col_context_t *context)
{
    bool v7; // [esp+40h] [ebp-60h]
    sightclip_t clip; // [esp+48h] [ebp-58h] BYREF
    float delta[3]; // [esp+94h] [ebp-Ch]

    PROF_SCOPED("SV_SightTrace");

    if ( !mins && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp", 941, 0, "%s", "mins") )
        __debugbreak();
    if ( !maxs && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp", 941, 0, "%s", "maxs") )
        __debugbreak();
    if ( *maxs < *mins
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp", 941, 0, "%s", "maxs[0] >= mins[0]") )
    {
        __debugbreak();
    }
    if ( maxs[1] < mins[1]
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp", 941, 0, "%s", "maxs[1] >= mins[1]") )
    {
        __debugbreak();
    }
    if ( maxs[2] < mins[2]
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp", 941, 0, "%s", "maxs[2] >= mins[2]") )
    {
        __debugbreak();
    }
    nanassertvec3(mins);
    nanassertvec3(maxs);
    nanassertvec3(start);
    nanassertvec3(end);

    *hitNum = CM_BoxSightTrace(*hitNum, start, end, mins, maxs, 0, context->mask);
    if ( *hitNum )
    {
        return 0;
    }
    else
    {
        if ( (float)((float)((float)((float)((float)(*maxs - *mins) + maxs[1]) - mins[1]) + maxs[2]) - mins[2]) == 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server\\sv_world.cpp",
                        950,
                        0,
                        "%s",
                        "(maxs[0] - mins[0] + maxs[1] - mins[1] + maxs[2] - mins[2])") )
        {
            __debugbreak();
        }
        clip.contentmask = context->mask;
        clip.passEntityNum[0] = context->passEntityNum0;
        clip.passEntityNum[1] = context->passEntityNum1;
        clip.outerSize[0] = *maxs - *mins;
        clip.outerSize[1] = maxs[1] - mins[1];
        clip.outerSize[2] = maxs[2] - mins[2];
        clip.outerSize[0] = 0.5 * clip.outerSize[0];
        clip.outerSize[1] = 0.5 * clip.outerSize[1];
        clip.outerSize[2] = 0.5 * clip.outerSize[2];
        clip.maxs[0] = clip.outerSize[0];
        clip.maxs[1] = clip.outerSize[1];
        clip.maxs[2] = clip.outerSize[2];
        clip.mins[0] = -1.0 * clip.outerSize[0];
        clip.mins[1] = -1.0 * clip.outerSize[1];
        clip.mins[2] = -1.0 * clip.outerSize[2];
        clip.outerSize[0] = clip.outerSize[0] + 1.0;
        clip.outerSize[1] = clip.outerSize[1] + 1.0;
        clip.outerSize[2] = clip.outerSize[2] + 1.0;
        delta[0] = *maxs + *mins;
        delta[1] = maxs[1] + mins[1];
        delta[2] = maxs[2] + mins[2];
        delta[0] = 0.5 * delta[0];
        delta[1] = 0.5 * delta[1];
        delta[2] = 0.5 * delta[2];
        clip.start[0] = *start + delta[0];
        clip.start[1] = start[1] + delta[1];
        clip.start[2] = start[2] + delta[2];
        clip.end[0] = *end + delta[0];
        clip.end[1] = end[1] + delta[1];
        clip.end[2] = end[2] + delta[2];
        *hitNum = CM_ClipSightTraceToEntities(&clip);
        v7 = *hitNum == 0;
        return v7;
    }
}

bool __cdecl SV_SightTracePoint(int *hitNum, const float *start, const float *end, col_context_t *context)
{
    bool v5; // [esp+30h] [ebp-3Ch]
    sightpointtrace_t clip; // [esp+3Ch] [ebp-30h] BYREF

    PROF_SCOPED("SV_SightTracePoint");

    nanassertvec3(start);
    nanassertvec3(end);

    *hitNum = CM_SightTracePoint(*hitNum, start, end, context);
    if ( *hitNum )
    {
        return 0;
    }
    else if ( !context->staticmodels || CM_PointTraceStaticModelsComplete(start, end, context->mask) )
    {
        clip.contentmask = context->mask;
        clip.start[0] = *start;
        clip.start[1] = start[1];
        clip.start[2] = start[2];
        clip.end[0] = *end;
        clip.end[1] = end[1];
        clip.end[2] = end[2];
        clip.passEntityNum[0] = context->passEntityNum0;
        clip.passEntityNum[1] = context->passEntityNum1;
        clip.locational = context->locational;
        clip.priorityMap = context->priorityMap;
        *hitNum = CM_PointSightTraceToEntities(&clip);
        v5 = *hitNum == 0;
        return v5;
    }
    else
    {
        return 0;
    }
}

int __cdecl SV_PointContents(float *p, int passEntityNum, int contentmask)
{
    int v3; // eax
    int entityList[1025]; // [esp+4h] [ebp-1018h] BYREF
    int v6; // [esp+1008h] [ebp-14h]
    unsigned int model; // [esp+100Ch] [ebp-10h]
    gentity_s *ent; // [esp+1010h] [ebp-Ch]
    int v9; // [esp+1014h] [ebp-8h]
    int i; // [esp+1018h] [ebp-4h]

    v6 = CM_PointContents(p, 0);
    v9 = CM_AreaEntities(p, p, entityList, 1024, contentmask);
    for ( i = 0; i < v9; ++i )
    {
        if ( entityList[i] != passEntityNum )
        {
            ent = (gentity_s *)((char *)sv.gentities + entityList[i] * sv.gentitySize);
            model = SV_ClipHandleForEntity(ent);
            v3 = CM_TransformedPointContents(p, model, ent->r.currentOrigin, ent->r.currentAngles);
            v6 |= v3;
        }
    }
    return contentmask & v6;
}

void __cdecl G_TraceCapsule(
                trace_t *results,
                const float *start,
                const float *mins,
                const float *maxs,
                const float *end,
                int passEntityNum,
                int contentmask,
                col_context_t *context)
{
    IgnoreEntParams ignoreEntParams; // [esp+8h] [ebp-10h] BYREF

    PROF_SCOPED("G_TraceCapsule");

    SV_SetupIgnoreEntParams(&ignoreEntParams, passEntityNum);
    context->ignoreEntParams = &ignoreEntParams;
    context->mask = contentmask;
    SV_TraceCapsule(results, start, mins, maxs, end, context);
}

void __cdecl G_LocationalTrace(
                trace_t *results,
                const float *start,
                const float *end,
                int passEntityNum,
                int contentmask,
                unsigned __int8 *priorityMap,
                int (__cdecl *collide_entity_func)(int, col_context_t *))
{
    IgnoreEntParams ignoreEntParams; // [esp+0h] [ebp-34h] BYREF

    SV_SetupIgnoreEntParams(&ignoreEntParams, passEntityNum);

    col_context_t context(contentmask);

    context.ignoreEntParams = &ignoreEntParams;
    context.init_locational(passEntityNum);
    context.priorityMap = priorityMap;
    context.collide_entity_func = collide_entity_func;
    SV_TracePoint(results, start, end, &context);
}

void __cdecl G_LocationalTraceAllowChildren(
                trace_t *results,
                const float *start,
                const float *end,
                int passEntityNum,
                int contentmask,
                unsigned __int8 *priorityMap)
{
    IgnoreEntParams ignoreEntParams; // [esp+0h] [ebp-34h] BYREF

    SV_SetupIgnoreEntParams(&ignoreEntParams, passEntityNum);
    ignoreEntParams.ignoreChildren = 0;

    col_context_t context(contentmask); // [esp+Ch] [ebp-28h] BYREF

    context.ignoreEntParams = &ignoreEntParams;
    context.init_locational(passEntityNum);
    context.priorityMap = priorityMap;
    SV_TracePoint(results, start, end, &context);
}

