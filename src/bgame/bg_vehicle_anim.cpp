#include "bg_vehicle_anim.h"
#include "bg_local.h"

#include <universal/dvar.h>

#include <cstring>
#include <universal/com_math_anglevectors.h>
#include <universal/com_math.h>
#include "bg_misc.h"
#include <xanim/xanim.h>

vehAnimClient_t clVehAnimClients[1];
vehAnimClient_t svVehAnimClients[32];

const dvar_t *vehanim_enable;
const dvar_t *vehanim_debug;


void __cdecl VehAnim_RegisterDvars()
{
    vehanim_enable = _Dvar_RegisterBool("vehanim_enable", 1, 0x80u, "Enable player vehicle anims");
    vehanim_debug = _Dvar_RegisterBool("vehanim_debug", 0, 0x80u, "Show debug information for vehicle anims");
}

void __cdecl VehAnim_Init()
{
    *(unsigned int *)&clVehAnimClients[0].initialized = 0;
    clVehAnimClients[0].curSeat = VEH_POS_DRIVER;
    clVehAnimClients[0].prevSeat = VEH_POS_DRIVER;
    *(unsigned int *)&clVehAnimClients[0].moving = 0;
    clVehAnimClients[0].animStage = 0;
    clVehAnimClients[0].animIndex = 0;
    clVehAnimClients[0].tagName = 0;
    memset(svVehAnimClients, 0, sizeof(svVehAnimClients));
}

void __cdecl VehAnim_UpdatePosRot(
                int localClientNum,
                const entityState_s *es,
                clientInfo_t *ci,
                int handler,
                float *origin,
                float *angles)
{
    float *v6; // [esp+8h] [ebp-90h]
    float v7; // [esp+Ch] [ebp-8Ch]
    float finalAxis[3][3]; // [esp+10h] [ebp-88h] BYREF
    float trans[3]; // [esp+34h] [ebp-64h] BYREF
    float tempAxis[3][3]; // [esp+40h] [ebp-58h] BYREF
    float yaw; // [esp+64h] [ebp-34h]
    float rot[2]; // [esp+68h] [ebp-30h] BYREF
    int i; // [esp+70h] [ebp-28h]
    float axis[3][3]; // [esp+74h] [ebp-24h] BYREF

    if ( vehanim_enable->current.enabled )
    {
        VehAnim_GetAnimDelta(ci->pXAnimTree, es->animState.state & 0xFFFFFBFF, rot, trans);
        AnglesToAxis(angles, axis);
        for ( i = 0; i < 3; ++i )
        {
            v6 = axis[i];
            v7 = trans[i];
            *origin = (float)(v7 * *v6) + *origin;
            origin[1] = (float)(v7 * v6[1]) + origin[1];
            origin[2] = (float)(v7 * v6[2]) + origin[2];
        }
        yaw = RotationToYaw(rot);
        YawToAxis(yaw, tempAxis);
        MatrixMultiply(tempAxis, axis, finalAxis);
        AxisToAngles(finalAxis, angles);
    }
}

void __cdecl VehAnim_GetAnimDelta(XAnimTree_s *tree, unsigned int animIndex, float *rot, float *trans)
{
    float frac; // [esp+4h] [ebp-8h]
    XAnim_s *pXAnims; // [esp+8h] [ebp-4h]

    BG_CheckThread();
    iassert(bgs);
    //if ( !*(unsigned int *)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 8)
    //    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_vehicle_anim.cpp", 144, 0, "%s", "bgs") )
    //{
    //    __debugbreak();
    //}
    //pXAnims = *(XAnim_s **)(**(unsigned int **)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 8) + 578412);
    pXAnims = bgs->animData->animScriptData.animTree.anims;
    frac = XAnimGetTime(tree, animIndex);
    XAnimGetAbsDelta(pXAnims, animIndex, rot, trans, frac);
}

