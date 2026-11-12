#include "ragdoll.h"
#include <cgame/cg_drawtools.h>
#include <client/screen_placement.h>
#include <qcommon/dobj_management.h>
#include <cgame_mp/cg_ents_mp.h>
#include "ragdoll_controller.h"
#include "ragdoll_update.h"
#include <clientscript/cscr_stringlist.h>
#include <qcommon/threads.h>
#include "ragdoll_cmds.h"
#include <qcommon/com_clients.h>
#include <qcommon/cmd.h>

const dvar_t *ragdoll_enable;
const dvar_t *ragdoll_debug;
const dvar_t *ragdoll_fps;
const dvar_t *ragdoll_max_life;
const dvar_t *ragdoll_max_simulating;
const dvar_t *ragdoll_explode_force;
const dvar_t *ragdoll_explode_upbias;
const dvar_t *ragdoll_bullet_force;
const dvar_t *ragdoll_bullet_upbias;
const dvar_t *ragdoll_baselerp_time;
const dvar_t *ragdoll_jointlerp_time;
const dvar_t *ragdoll_rotvel_scale;
const dvar_t *ragdoll_jitter_scale;
const dvar_t *ragdoll_self_collision_scale;
const dvar_t *ragdoll_dump_anims;

phys_free_list<RagdollBody> g_ragdoll_body_pool;

RagdollDef ragdollDefs[RAGDOLL_MAX_DEFS];

bool ragdollInited;

void __cdecl Ragdoll_DebugDraw()
{
    int total; // [esp+30h] [ebp-1Ch]
    int states[RAGDOLL_NUM_STATES]; // [esp+34h] [ebp-18h] BYREF

    if ( ragdoll_debug->current.integer )
    {
        memset(states, 0, sizeof(states));
        total = 0;
        for ( RagdollBody &bodyRef : g_ragdoll_body_pool )
        {
            RagdollBody *body = &bodyRef;
            iassert(Ragdoll_BodyInUse(body));
            bcassert((int)body->state, RAGDOLL_NUM_STATES);
            ++states[(int)body->state];
            ++total;
        }
        CG_DrawStringExt(&scrPlaceFull, 0.0, 72.0, va("RB Total: %d", total), colorGreen, 0, 1, 12.0);
        CG_DrawStringExt(&scrPlaceFull, 0.0, 84.0, va("RB State: %d %d %d %d %d %d", states[0], states[1], states[2], states[3], states[4], states[5]), colorGreen, 0, 1, 12.0);
    }
}

bool __cdecl Ragdoll_BodyInUse(RagdollBody *body)
{
    iassert(body);
    return body->references > 0;
}

RagdollDef *__cdecl Ragdoll_BodyDef(RagdollBody *body)
{
    iassert(body);
    bcassert(body->ragdollDef, RAGDOLL_MAX_DEFS);

    return &ragdollDefs[body->ragdollDef];
}

DObj *__cdecl Ragdoll_BodyDObj(RagdollBody *body)
{
    iassert(body);
    
    if ( body->obj )
        return body->obj;

    iassert(body->dobj != DOBJ_HANDLE_NONE);

    return Com_GetClientDObj(body->dobj, body->localClientNum);
}

const cpose_t *__cdecl Ragdoll_BodyPose(RagdollBody *body)
{
    iassert(body);

    if ( body->pose )
        return body->pose;

    iassert(body->dobj != DOBJ_HANDLE_NONE);

    return CG_GetPose(body->localClientNum, body->dobj);
}

void __cdecl Ragdoll_BodyRootOrigin(RagdollBody *body, float *origin)
{
    if ( Ragdoll_BodyPoseValid(body) )
    {
        BoneOrientation *boneOrientation = Ragdoll_BodyBoneOrientations(body);
        Vec3Copy(boneOrientation->origin, origin);
    }
}

bool __cdecl Ragdoll_BodyPoseValid(RagdollBody *body)
{
    iassert(body);
    return body->state >= BS_TUNNEL_TEST;
}

void __cdecl Ragdoll_GetRootOrigin(int ragdollHandle, float *origin)
{
    RagdollBody *body; // [esp+0h] [ebp-4h]

    body = (RagdollBody *)Ragdoll_HandleBody(ragdollHandle);

    iassert(body && Ragdoll_BodyInUse(body));

    if ( Ragdoll_BodyPoseValid(body) )
        Ragdoll_BodyRootOrigin(body, origin);
}

int __cdecl Ragdoll_CountPhysicsBodies()
{
    int running = 0;

    for (RagdollBody &bodyRef : g_ragdoll_body_pool)
    {
        RagdollBody *body = &bodyRef;
        iassert(Ragdoll_BodyInUse(body));

        if (Ragdoll_BodyHasPhysics(body))
            running++;
    }

    return running;
}

bool __cdecl Ragdoll_BodyHasPhysics(RagdollBody *body)
{
    iassert(body);
    return body->state >= BS_DOBJ_WAIT && body->state <= BS_RUNNING;
}

const RagdollBody *__cdecl Ragdoll_CreateRagdollForDObj(
                int localClientNum,
                int ragdollDef,
                int dobj,
                bool reset,
                bool share)
{
    const RagdollBody *ragdoll; // [esp+0h] [ebp-8h]
    RagdollBody *body; // [esp+4h] [ebp-4h]

    iassert(dobj != DOBJ_HANDLE_NONE);

    if ( !Ragdoll_BindDef(1u) )
        return 0;

    if ( Ragdoll_CountPhysicsBodies() >= ragdoll_max_simulating->current.integer )
        return 0;

    if ( share )
    {
        ragdoll = Ragdoll_ReferenceDObjBody(dobj);
        if ( !ragdoll )
            ragdoll = Ragdoll_GetUnusedBody();
    }
    else
    {
        ragdoll = Ragdoll_GetUnusedBody();
    }

    if ( ragdoll )
    {
        body = (RagdollBody *)Ragdoll_HandleBody((int)ragdoll);
        if ( reset )
            Ragdoll_BodyNewState(body, BS_DEAD);
        if ( body->state == BS_DEAD )
        {
            body->dobj = (int)dobj;
            body->ragdollDef = 1;
            body->localClientNum = localClientNum;
            Ragdoll_BodyNewState(body, BS_DOBJ_WAIT);
        }
        return ragdoll;
    }
    else
    {
        Com_Printf(20, "Ragdoll allocation failed, out of ragdoll bodies (obj %d)\n", dobj);
        return 0;
    }
}

RagdollBody *__cdecl Ragdoll_GetUnusedBody()
{
    RagdollBody *body; // [esp+1Ch] [ebp-4h]

    if ( g_ragdoll_body_pool.m_list_count >= 32 )
        return 0;

    //body = phys_free_list<RagdollBody>::add(&g_ragdoll_body_pool, 0, "phys_free_list error: out of memory.");
    body = g_ragdoll_body_pool.add(0, "phys_free_list error: out of memory.");
    if ( !body )
        return 0;
    Ragdoll_InitBody(body);
    Ragdoll_BodyNewState(body, BS_DEAD);

    iassert(body->references == 0);
    ++body->references;

    return body;
}

void __cdecl Ragdoll_InitBody(RagdollBody *body)
{
    iassert(body);
    memset(body, 0, sizeof(RagdollBody));
}

RagdollBody *__cdecl Ragdoll_ReferenceDObjBody(int dobj)
{
    for (RagdollBody &bodyRef : g_ragdoll_body_pool)
    {
        RagdollBody *body = &bodyRef;
        iassert(Ragdoll_BodyInUse(body));

        if (body->dobj == dobj)
        {
            ++body->references;
            return body;
        }
    }
    return NULL;
}

char __cdecl Ragdoll_BindDef(unsigned int ragdollDef)
{
    RagdollDef *def; // [esp+0h] [ebp-18h]
    int nameIdx; // [esp+4h] [ebp-14h]
    BoneDef *boneDef; // [esp+8h] [ebp-10h]
    BaseLerpBoneDef *lerpBoneDef; // [esp+10h] [ebp-8h]
    int parentIdx; // [esp+14h] [ebp-4h]

    if ( !Ragdoll_ValidateDef(ragdollDef) )
        return 0;
    def = &ragdollDefs[ragdollDef];
    if ( def->bound )
        return 1;
    boneDef = def->boneDefs;
    for ( int i = 0; i < def->numBones; ++i )
    {
        for ( nameIdx = 0; nameIdx < 2; ++nameIdx )
        {
            if ( I_stricmp(boneDef->animBoneTextNames[nameIdx], "none") )
            {
                boneDef->animBoneNames[nameIdx] = SL_FindString(boneDef->animBoneTextNames[nameIdx], SCRIPTINSTANCE_SERVER);
                if ( !boneDef->animBoneNames[nameIdx] )
                    return 0;
            }
            else
            {
                boneDef->animBoneNames[nameIdx] = 0;
            }
        }
        ++boneDef;
    }
    lerpBoneDef = def->baseLerpBoneDefs;
    for ( int i = 0; i < def->numBaseLerpBones; ++i )
    {
        lerpBoneDef->animBoneName = SL_FindString(lerpBoneDef->animBoneTextName, SCRIPTINSTANCE_SERVER);
        if ( !lerpBoneDef->animBoneName )
            return 0;
        if ( !lerpBoneDef->parentBoneIndex )
        {
            boneDef = def->boneDefs;
            for ( parentIdx = 0; parentIdx < def->numBones; ++parentIdx )
            {
                if ( boneDef->animBoneNames[1] == lerpBoneDef->animBoneName )
                {
                    lerpBoneDef->parentBoneIndex = parentIdx;
                    break;
                }
                ++boneDef;
            }
            if ( parentIdx == def->numBones )
                return 0;
        }
        ++lerpBoneDef;
    }
    def->bound = 1;
    return 1;
}

bool __cdecl Ragdoll_ValidateDef(unsigned int ragdollDef)
{
    bcassert(ragdollDef, RAGDOLL_MAX_DEFS);
    return ragdollDefs[ragdollDef].numBones != 0;
}

void __cdecl Ragdoll_Remove(int ragdoll)
{
    RagdollBody *body; // [esp+0h] [ebp-4h]

    body = (RagdollBody *)Ragdoll_HandleBody(ragdoll);

    iassert(Ragdoll_BodyInUse(body));

    if ( body->references == 1 )
    {
        Ragdoll_BodyNewState(body, BS_DEAD);
        Ragdoll_FreeBody(ragdoll);
    }
    else
    {
        --body->references;
    }
}

void __cdecl Ragdoll_FreeBody(int ragdollBody)
{
    RagdollBody *body; // [esp+24h] [ebp-4h]

    body = (RagdollBody *)Ragdoll_HandleBody(ragdollBody);

    iassert(body);
    iassert(body->references == 1);

    Ragdoll_BodyNewState(body, BS_DEAD);
    body->references = 0;

    //phys_free_list<RagdollBody>::remove(&g_ragdoll_body_pool, body);
    g_ragdoll_body_pool.remove(body);
}

void __cdecl Ragdoll_InitDvars()
{
    ragdoll_enable               = _Dvar_RegisterBool ("ragdoll_enable",               1,                            0,     "Turn on ragdoll death animations");
    ragdoll_debug                = _Dvar_RegisterInt  ("ragdoll_debug",                0,            0,    0x7FFFFFFF, 0x80u, "Draw ragdoll debug info (bitflags)");
    ragdoll_fps                  = _Dvar_RegisterInt  ("ragdoll_fps",                  20,           0,    100,        0x80u, "Ragdoll update frames per second");
    ragdoll_max_life             = _Dvar_RegisterInt  ("ragdoll_max_life",             9000,         0,    0x7FFFFFFF, 0x80u, "Max lifetime of a ragdoll system in msec");
    ragdoll_max_simulating       = _Dvar_RegisterInt  ("ragdoll_max_simulating",       16,           0,    32,         1u,    "Max number of simultaneous active ragdolls");
    ragdoll_explode_force        = _Dvar_RegisterFloat("ragdoll_explode_force",        18000.0f,     0.0f, 60000.0f,   0x80u, "Explosive force applied to ragdolls");
    ragdoll_explode_upbias       = _Dvar_RegisterFloat("ragdoll_explode_upbias",       0.80000001f,  0.0f, 2.0f,       0x80u, "Upwards bias applied to ragdoll explosion effects");
    ragdoll_bullet_force         = _Dvar_RegisterFloat("ragdoll_bullet_force",         500.0f,       0.0f, 10000.0f,   0x80u, "Bullet force applied to ragdolls");
    ragdoll_bullet_upbias        = _Dvar_RegisterFloat("ragdoll_bullet_upbias",        0.5f,         0.0f, 10000.0f,   0x80u, "Upward bias applied to ragdoll bullet effects");
    ragdoll_baselerp_time        = _Dvar_RegisterInt  ("ragdoll_baselerp_time",        1000,         100,  6000,       0x80u, "Default time ragdoll baselerp bones take to reach the base pose");
    ragdoll_jointlerp_time       = _Dvar_RegisterInt  ("ragdoll_jointlerp_time",       3000,         100,  6000,       0x80u, "Default time taken to lerp down ragdoll joint friction");
    ragdoll_rotvel_scale         = _Dvar_RegisterFloat("ragdoll_rotvel_scale",         1.0f,         0.0f, 2000.0f,    0x80u, "Ragdoll rotational velocity estimate scale");
    ragdoll_jitter_scale         = _Dvar_RegisterFloat("ragdoll_jitter_scale",         1.0f,         0.0f, 10.0f,      0x80u, "Scale up or down the effect of physics jitter on ragdolls");
    ragdoll_self_collision_scale = _Dvar_RegisterFloat("ragdoll_self_collision_scale", 1.2f,         0.1f, 10.0f,      0x80u, "Scale the size of the collision capsules used to prevent ragdoll limbs from interpenetrating");
    ragdoll_dump_anims           = _Dvar_RegisterBool ("ragdoll_dump_anims",           0,                            0,     "Dump animation data when ragdoll fails");
}

bool ragdollFirstInit;
void __cdecl Ragdoll_Register()
{
    int ControllerIndex; // eax

    iassert(Sys_IsMainThread());
    iassert(!ragdollFirstInit);

    Ragdoll_InitDvars();
    Ragdoll_InitCommands();
    ControllerIndex = Com_LocalClient_GetControllerIndex(0);
    Cmd_ExecuteSingleCommand(0, ControllerIndex, (char*)"exec ragdoll.cfg");
    ragdollFirstInit = 1;
}

void __cdecl Ragdoll_Init()
{
    int i; // [esp+0h] [ebp-4h]

    iassert(Sys_IsMainThread());
    iassert(ragdollFirstInit);

    if ( !ragdollInited )
    {
        for ( i = 0; i < 2; ++i )
        {
            ragdollDefs[i].bound = 0;
            ragdollDefs[i].inUse = 0;
        }
        ragdollInited = 1;
    }
}

void __cdecl Ragdoll_Shutdown()
{
    iassert(Sys_IsMainThread());
    ragdollInited = 0;
}