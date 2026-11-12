#include "ragdoll_cmds.h"
#include <qcommon/cmd.h>
#include "ragdoll_update.h"
#include <clientscript/cscr_stringlist.h>

const char *geomNames[6] =
{ "n", "box", "brushmodel", "brush", "cyl", "cap" };

const char *jointNames[3] =
{ "none", "hinge", "swivel" };

struct $226EAEAF211E69CAF5BF90DB1F921A0C // sizeof=0x10
{                                       // XREF: .data:axisTable/r
    const char *name;                   // XREF: Ragdoll_ReadAxis+A6/r
    float axis[3];                      // XREF: Ragdoll_ReadAxis+C3/o
};
$226EAEAF211E69CAF5BF90DB1F921A0C axisTable[4] =
{
  { "x", { 1.0, 0.0, 0.0 } },
  { "y", { 0.0, 1.0, 0.0 } },
  { "z", { 0.0, 0.0, 1.0 } },
  { "n", { 0.0, 0.0, 0.0 } }
};

void __cdecl Ragdoll_Clear_f()
{
    unsigned int ragdoll; // [esp+10h] [ebp-4h]

    if ( Cmd_Argc() >= 2 )
    {
        ragdoll = atoi(Cmd_Argv(1));
        if ( ragdoll < RAGDOLL_MAX_DEFS )
        {
            memset(&ragdollDefs[ragdoll], 0, sizeof(RagdollDef));
            Ragdoll_ResetBodiesUsingDef();
        }
    }
}

void __cdecl Ragdoll_ResetBodiesUsingDef()
{
    for (RagdollBody &bodyRef : g_ragdoll_body_pool)
    {
        RagdollBody *body = &bodyRef;
        iassert(Ragdoll_BodyInUse(body));

        if (Ragdoll_BodyBound(body))
            Ragdoll_BodyNewState(body, BS_DOBJ_WAIT);
    }
}

bool __cdecl Ragdoll_BodyBound(RagdollBody *body)
{
    iassert(body);
    return body->state >= BS_VELOCITY_CAPTURE;
}

cmd_function_s Ragdoll_Clear_f_VAR;
cmd_function_s Ragdoll_Bone_f_VAR;
cmd_function_s Ragdoll_BaseLerpBone_f_VAR;
cmd_function_s Ragdoll_PinBone_f_VAR;
cmd_function_s Ragdoll_Joint_f_VAR;
cmd_function_s Ragdoll_Limit_f_VAR;
cmd_function_s Ragdoll_Selfpair_f_VAR;
void __cdecl Ragdoll_InitCommands()
{
    Cmd_AddCommandInternal("ragdoll_clear", Ragdoll_Clear_f, &Ragdoll_Clear_f_VAR);
    Cmd_AddCommandInternal("ragdoll_bone", Ragdoll_Bone_f, &Ragdoll_Bone_f_VAR);
    Cmd_AddCommandInternal("ragdoll_baselerp_bone", Ragdoll_BaseLerpBone_f, &Ragdoll_BaseLerpBone_f_VAR);
    Cmd_AddCommandInternal("ragdoll_pin_bone", Ragdoll_PinBone_f, &Ragdoll_PinBone_f_VAR);
    Cmd_AddCommandInternal("ragdoll_joint", Ragdoll_Joint_f, &Ragdoll_Joint_f_VAR);
    Cmd_AddCommandInternal("ragdoll_limit", Ragdoll_Limit_f, &Ragdoll_Limit_f_VAR);
    Cmd_AddCommandInternal("ragdoll_selfpair", Ragdoll_Selfpair_f, &Ragdoll_Selfpair_f_VAR);
}

void __cdecl Ragdoll_Bone_f()
{
    unsigned int ragdoll; // [esp+38h] [ebp-18h]
    RagdollDef *def; // [esp+3Ch] [ebp-14h]
    int parentBone; // [esp+40h] [ebp-10h]
    const char *name; // [esp+44h] [ebp-Ch]
    BoneDef *bone; // [esp+4Ch] [ebp-4h]

    if ( Cmd_Argc() >= 12 )
    {
        ragdoll = atoi(Cmd_Argv(1));
        if (ragdoll >= RAGDOLL_MAX_DEFS)
        {
            return;
        }

        def = &ragdollDefs[ragdoll];
        if ( def->numBones >= 14 )
        {
            Com_Printf(14, "Ragdoll: Too many ragdoll bones, max %d\n", 14);
            return;
        }

        bone = &def->boneDefs[def->numBones];
        for ( int i = 0; i < RAGDOLL_MAX_DEFS; ++i )
        {
            bone->animBoneNames[i] = 0;
            name = Cmd_Argv(i + 2);
            I_strncpyz(bone->animBoneTextNames[i], name, 20);
        }

        bone->radius = atof(Cmd_Argv(4));
        bone->percent = atof(Cmd_Argv(5));
        bone->mass = atof(Cmd_Argv(6));
        bone->friction = atof(Cmd_Argv(7));
        bone->mass = bone->mass * 0.001;
        parentBone = atoi(Cmd_Argv(8));

        if ( parentBone == -1 )
        {
            bone->parentBone = -1;
        }
        else
        {
            if ( parentBone >= def->numBones )
            {
                Com_Printf(14, "Ragdoll: Child bones must come after parent bones: %d\n", parentBone);
                return;
            }
            bone->parentBone = parentBone;
        }
        bone->mirror = atoi(Cmd_Argv(9)) != 0;
        if ( Ragdoll_ReadGeomType(10, bone) )
        {
            bone->buoyancy = atof(Cmd_Argv(11));
            if ( Ragdoll_ValidatePrecalcBoneDef(def, bone) )
            {
                ++def->numBones;
                Ragdoll_ResetBodiesUsingDef();
            }
            else
            {
                Com_Printf(14, "Ragdoll: Bone %d validation failed\n", def->numBones);
            }
        }
    }
}

char __cdecl Ragdoll_ReadGeomType(int arg, BoneDef *bone)
{
    const char *name; // [esp+24h] [ebp-8h]
    PhysicsGeomType idx; // [esp+28h] [ebp-4h]

    if ( Cmd_Argc() >= arg )
    {
        name = Cmd_Argv(arg);
        for ( idx = PHYS_GEOM_NONE; (unsigned int)idx < PHYS_GEOM_POINT; ++idx )
        {
            if ( !I_strnicmp(geomNames[idx], name, strlen(geomNames[idx])) )
            {
                bone->geomType = idx;
                return 1;
            }
        }
        Com_Printf(14, "Ragdoll: Unknown bone geom type %s\n", name);
        return 0;
    }
    else
    {
        Com_Printf(14, "Ragdoll: Missing geom type arg %d\n", arg);
        return 0;
    }
}

void __cdecl Ragdoll_Joint_f()
{
    unsigned int ragdoll; // [esp+18h] [ebp-Ch]
    RagdollDef *def; // [esp+1Ch] [ebp-8h]
    JointDef *joint; // [esp+20h] [ebp-4h]

    if ( Cmd_Argc() >= 4 )
    {
        ragdoll = atoi(Cmd_Argv(1));
        if ( ragdoll < RAGDOLL_MAX_DEFS )
        {
            def = &ragdollDefs[ragdoll];
            if ( def->numJoints < 28 )
            {
                joint = &def->jointDefs[def->numJoints];
                joint->bone = atoi(Cmd_Argv(2));
                if ( joint->bone < def->numBones )
                {
                    if ( joint->bone )
                    {
                        if ( Ragdoll_ReadJointType(3, joint) )
                        {
                            ++def->numJoints;
                            Ragdoll_ResetBodiesUsingDef();
                        }
                    }
                    else
                    {
                        Com_Printf(14, "Ragdoll: Joint referenced bone with no parent (0)\n");
                    }
                }
                else
                {
                    Com_Printf(14, "Ragdoll: Joint referenced nonexistent bone\n");
                }
            }
            else
            {
                Com_Printf(14, "Ragdoll: Too many ragdoll joints, max %d\n", 28);
            }
        }
    }
}

char __cdecl Ragdoll_ReadJointType(int arg, JointDef *joint)
{
    const char *name; // [esp+10h] [ebp-8h]
    JointType idx; // [esp+14h] [ebp-4h]

    if ( Cmd_Argc() >= arg )
    {
        name = Cmd_Argv(arg);
        for ( idx = RAGDOLL_JOINT_NONE; (unsigned int)idx < (RAGDOLL_JOINT_SWIVEL|RAGDOLL_JOINT_HINGE); ++idx )
        {
            if ( !I_stricmp(name, jointNames[idx]) )
            {
                joint->type = idx;
                return 1;
            }
        }
        Com_Printf(14, "Ragdoll: Unknown joint type %s\n", name);
        return 0;
    }
    else
    {
        Com_Printf(14, "Ragdoll: Missing joint type arg %d\n", arg);
        return 0;
    }
}

void __cdecl Ragdoll_Limit_f()
{
    float v5; // [esp+0h] [ebp-64h]
    float v6; // [esp+4h] [ebp-60h]
    float v7; // [esp+8h] [ebp-5Ch]
    float v8; // [esp+Ch] [ebp-58h]
    float v9; // [esp+10h] [ebp-54h]
    float v10; // [esp+14h] [ebp-50h]
    float v11; // [esp+18h] [ebp-4Ch]
    float v12; // [esp+1Ch] [ebp-48h]
    float v13; // [esp+20h] [ebp-44h]
    unsigned int ragdoll; // [esp+54h] [ebp-10h]
    RagdollDef *def; // [esp+58h] [ebp-Ch]
    int jointNum; // [esp+60h] [ebp-4h]

    if ( Cmd_Argc() >= 7 )
    {
        ragdoll = atoi(Cmd_Argv(1));
        if ( ragdoll < RAGDOLL_MAX_DEFS )
        {
            def = &ragdollDefs[ragdoll];
            if ( def->numJoints < 28 )
            {
                jointNum = atoi(Cmd_Argv(2));
                if ( jointNum < def->numJoints )
                {
                    if ( def->jointDefs[jointNum].numLimitAxes < 3 )
                    {
                        if ( Ragdoll_ReadAxis(3, def->jointDefs[jointNum].limitAxes[def->jointDefs[jointNum].numLimitAxes]) )
                        {
                            v9 = atof(Cmd_Argv(4));
                            def->jointDefs[jointNum].axisFriction[def->jointDefs[jointNum].numLimitAxes] = v9;
                            v8 = atof(Cmd_Argv(5));
                            def->jointDefs[jointNum].minAngles[def->jointDefs[jointNum].numLimitAxes] = DEG2RAD(v8);
                            v7 = atof(Cmd_Argv(6));
                            def->jointDefs[jointNum].maxAngles[def->jointDefs[jointNum].numLimitAxes] = DEG2RAD(v7);
                            v12 = def->jointDefs[jointNum].minAngles[def->jointDefs[jointNum].numLimitAxes];
                            if ( (float)(v12 - 3.1415927) < 0.0 )
                                v13 = def->jointDefs[jointNum].minAngles[def->jointDefs[jointNum].numLimitAxes];
                            else
                                v13 = M_PI;
                            if ( (float)(-3.1415927 - v12) < 0.0 )
                                v6 = v13;
                            else
                                v6 = -M_PI;
                            def->jointDefs[jointNum].minAngles[def->jointDefs[jointNum].numLimitAxes] = v6;
                            v10 = def->jointDefs[jointNum].maxAngles[def->jointDefs[jointNum].numLimitAxes];
                            if ( (float)(v10 - 3.1415927) < 0.0 )
                                v11 = def->jointDefs[jointNum].maxAngles[def->jointDefs[jointNum].numLimitAxes];
                            else
                                v11 = M_PI;
                            if ( (float)(-3.1415927 - v10) < 0.0 )
                                v5 = v11;
                            else
                                v5 = -M_PI;
                            def->jointDefs[jointNum].maxAngles[def->jointDefs[jointNum].numLimitAxes++] = v5;
                            Ragdoll_ResetBodiesUsingDef();
                        }
                    }
                    else
                    {
                        Com_Printf(14, "Ragdoll: Too many limit axes for joint %d, max %d\n", jointNum, 3);
                    }
                }
                else
                {
                    Com_Printf(14, "Ragdoll: Angular limit added to nonexistent joint %d\n", jointNum);
                }
            }
            else
            {
                Com_Printf(14, "Ragdoll: Too many ragdoll joints, max %d\n", 28);
            }
        }
    }
}

char __cdecl Ragdoll_ReadAxis(int arg, float *dest)
{
    float *axis; // [esp+0h] [ebp-20h]
    bool negate; // [esp+17h] [ebp-9h]
    unsigned int idx; // [esp+18h] [ebp-8h]
    const char *argv; // [esp+1Ch] [ebp-4h]

    if ( Cmd_Argc() >= arg )
    {
        negate = 0;
        argv = Cmd_Argv(arg);
        if ( *argv == 45 )
        {
            negate = 1;
            ++argv;
        }
        for ( idx = 0; ; ++idx )
        {
            if ( idx >= 4 )
            {
                Com_Printf(14, "Ragdoll: Unknown bone axis %s\n", argv);
                return 0;
            }
            if ( !I_stricmp(argv, axisTable[idx].name) )
                break;
        }
        axis = axisTable[idx].axis;
        Vec3Copy(axis, dest);
        if ( negate )
        {
            //*(unsigned int *)dest ^= _mask__NegFloat_;
            //*((unsigned int *)dest + 1) ^= _mask__NegFloat_;
            //*((unsigned int *)dest + 2) ^= _mask__NegFloat_;
            dest[0] = -dest[0];
            dest[1] = -dest[1];
            dest[2] = -dest[2];
        }
        return 1;
    }
    else
    {
        Com_Printf(14, "Ragdoll: Missing axis arg %d\n", arg);
        dest[0] = 0.0f;
        dest[1] = 0.0f;
        dest[2] = 0.0f;
        return 0;
    }
}

void __cdecl Ragdoll_Selfpair_f()
{
    unsigned int ragdoll; // [esp+18h] [ebp-10h]
    RagdollDef *def; // [esp+1Ch] [ebp-Ch]
    SelfPairDef *pair; // [esp+20h] [ebp-8h]
    int i; // [esp+24h] [ebp-4h]

    if ( Cmd_Argc() >= 4 )
    {
        ragdoll = atoi(Cmd_Argv(1));
        if ( ragdoll < RAGDOLL_MAX_DEFS )
        {
            def = &ragdollDefs[ragdoll];
            if ( def->numSelfPairs < 33 )
            {
                pair = &def->selfPairDefs[def->numSelfPairs];
                for ( i = 0; i < RAGDOLL_MAX_DEFS; ++i )
                {
                    pair->bones[i] = atoi(Cmd_Argv(i + 2));
                    if ( pair->bones[i] > def->numBones )
                    {
                        Com_Printf(14, "Ragdoll: Bad self collision pair bone %d\n", pair->bones[i]);
                        return;
                    }
                }
                ++def->numSelfPairs;
                Ragdoll_ResetBodiesUsingDef();
            }
            else
            {
                Com_Printf(14, "Ragdoll: Too many ragdoll self collision pairs, max %d\n", 33);
            }
        }
    }
}

void __cdecl Ragdoll_BaseLerpBone_f()
{
    int v3; // [esp+0h] [ebp-54h]
    int lerpTime; // [esp+18h] [ebp-3Ch]
    unsigned int ragdoll; // [esp+44h] [ebp-10h]
    RagdollDef *def; // [esp+48h] [ebp-Ch]
    const char *name; // [esp+4Ch] [ebp-8h]
    BaseLerpBoneDef *bone; // [esp+50h] [ebp-4h]

    if ( Cmd_Argc() >= 3 )
    {
        ragdoll = atoi(Cmd_Argv(1));
        if ( ragdoll < RAGDOLL_MAX_DEFS )
        {
            def = &ragdollDefs[ragdoll];
            if ( def->numBaseLerpBones < 9 )
            {
                bone = &def->baseLerpBoneDefs[def->numBaseLerpBones];
                name = Cmd_Argv(2);
                I_strncpyz(bone->animBoneTextName, name, 20);
                bone->animBoneName = 0;
                if ( Cmd_Argc() <= 2 )
                {
                    bone->lerpTime = ragdoll_baselerp_time->current.integer;
                }
                else
                {
                    bone->lerpTime = atoi(Cmd_Argv(3));
                    if ( bone->lerpTime < 6000 )
                        lerpTime = bone->lerpTime;
                    else
                        lerpTime = 6000;
                    if ( lerpTime > 100 )
                        v3 = lerpTime;
                    else
                        v3 = 100;
                    bone->lerpTime = v3;
                }
                if ( Cmd_Argc() <= 3 )
                {
                    bone->parentBoneIndex = 0;
                }
                else
                {
                    bone->parentBoneIndex = atoi(Cmd_Argv(4));
                }
                ++def->numBaseLerpBones;
            }
            else
            {
                Com_Printf(14, "Ragdoll: Too many base pose lerping bones, max %d\n", 9);
            }
        }
    }
}

void __cdecl Ragdoll_PinBone_f()
{
    unsigned int ragdoll; // [esp+20h] [ebp-10h]
    RagdollDef *def; // [esp+24h] [ebp-Ch]
    const char *name; // [esp+28h] [ebp-8h]
    BaseLerpBoneDef *bone; // [esp+2Ch] [ebp-4h]

    if ( Cmd_Argc() >= 3 )
    {
        ragdoll = atoi(Cmd_Argv(1));
        if ( ragdoll < RAGDOLL_MAX_DEFS )
        {
            def = &ragdollDefs[ragdoll];
            if ( def->numBaseLerpBones < 9 )
            {
                bone = &def->baseLerpBoneDefs[def->numBaseLerpBones];
                name = Cmd_Argv(2);
                bone->animBoneName = SL_FindString(name, SCRIPTINSTANCE_SERVER);
                if ( bone->animBoneName )
                {
                    bone->parentBoneIndex = atoi(Cmd_Argv(3));
                    if ( bone->parentBoneIndex < def->numBones )
                    {
                        bone->lerpTime = 0;
                        ++def->numBaseLerpBones;
                    }
                    else
                    {
                        Com_Printf(14, "Ragdoll: Pinned bone has invalid parent index %d\n", bone->parentBoneIndex);
                    }
                }
                else
                {
                    Com_Printf(14, "Ragdoll: Couldn't find pinned bone named %s\n", name);
                }
            }
            else
            {
                Com_Printf(14, "Ragdoll: Too many base pose lerping bones, max %d\n", 9);
            }
        }
    }
}

