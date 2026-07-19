#include "cg_main.h"
#include "cg_local.h"
#include <bgame/bg_weapons_def.h>
#include <universal/com_loadutils.h>
#include <bgame/bg_misc.h>
#include <cgame_mp/cg_main_mp.h>
#include <cgame_mp/cg_vehicles_mp.h>
#include <client_mp/cl_cgame_mp.h>
#include <bgame/bg_misctables.h>

ViewModelInfo *cg_viewModelArray;
weaponInfo_s *cg_weaponsArray[1];

bool cgHasClientSystemBeenInitialzed;

int __cdecl CG_irand(int min, int max)
{
    __int64 result; // [esp+14h] [ebp-8h]

    if ( min > max
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_main.cpp", 46, 0, "%s", "min <= max") )
    {
        __debugbreak();
    }
    result = rand() * (__int64)(max - min) / 0x7FFF + min;
    if ( (min > result || result > max)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_main.cpp",
                    51,
                    0,
                    "%s\n\t(result) = %i",
                    "(min <= result && result <= max)",
                    (unsigned int)result) )
    {
        __debugbreak();
    }
    return result;
}

double __cdecl CG_flrand(float min, float max)
{
    float result; // [esp+8h] [ebp-4h]

    result = (float)((float)((float)(max - min) * (float)rand()) / 32767.0) + min;
    if ( (result < min || max < result)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_main.cpp",
                    64,
                    0,
                    "%s\n\t(result) = %g",
                    "(min <= result && result <= max)",
                    result) )
    {
        __debugbreak();
    }
    return result;
}

int __cdecl CG_rand()
{
    return rand();
}

float __cdecl CG_random()
{
    return Com_Random();
}

bool __cdecl CG_HasClientSystemBeenInitialzed()
{
    return cgHasClientSystemBeenInitialzed;
}

void __cdecl CG_ClientDoneWithInitialization()
{
    cgHasClientSystemBeenInitialzed = 1;
}

void __cdecl CG_ResetClientInitializationState()
{
    cgHasClientSystemBeenInitialzed = 0;
}

int __cdecl CG_GetWeaponIndexForName(char *name)
{
    return BG_GetWeaponIndexForName(name);
}

void __cdecl CG_RegisterVehicle(const char *name, __int16 index)
{
    double v2; // xmm0_8
    int WeaponIndexForName; // eax
    long double v4; // [esp+0h] [ebp-406Ch]
    unsigned __int8 *v5; // [esp+8h] [ebp-4064h]
    const char *v6; // [esp+Ch] [ebp-4060h]
    int j; // [esp+14h] [ebp-4058h]
    int i; // [esp+18h] [ebp-4054h]
    char string[64]; // [esp+1Ch] [ebp-4050h] BYREF
    char *pszBuffer; // [esp+5Ch] [ebp-4010h]
    unsigned __int8 *dst; // [esp+60h] [ebp-400Ch]
    char loadBuffer[16388]; // [esp+64h] [ebp-4008h] BYREF

    if ( name && *name )
    {
        sprintf(string, "vehicles/%s", name);
        pszBuffer = Com_LoadInfoString(string, "vehicle file", "VEHICLEFILE", loadBuffer);
        dst = (unsigned __int8 *)&bg_vehicleInfos[index];
        memset(dst, 0, 0x1DD8u);
        v6 = name;
        v5 = dst;
        HIDWORD(v4) = (int)dst;
        do
        {
            BYTE3(v4) = *v6;
            *v5++ = *v6++;
        }
        while ( BYTE3(v4) );
        if ( ParseConfigStringToStruct(
                     dst,
                     s_vehicleFields,
                     s_numVehicleFields,
                     pszBuffer,
                     25,
                     (int (__cdecl *)(unsigned __int8 *, const char *, const int, const int))VEH_ParseSpecificField,
                     BG_StringCopy) )
        {
            for ( i = 0; i < 4; ++i )
            {
                v2 = (float)((float)(*(float *)&dst[4 * i + 2788] * 0.5) * 0.017453292);
                //__libm_sse2_cos(v4);
                //*(float *)&v2 = v2;
                //*(unsigned int *)&dst[4 * i + 2788] = LODWORD(v2);
                *(float *)&dst[4 * i + 2788] = cos(v2);
            }
            for ( j = 0; j < 4; ++j )
            {
                WeaponIndexForName = CG_GetWeaponIndexForName((char *)&dst[64 * j + 772]);
                AssignToSmallerType<unsigned short>((unsigned __int16 *)&dst[2 * j + 1028], WeaponIndexForName);
            }
            ++bg_numVehicleInfos;
        }
        else
        {
            Com_Error(ERR_DROP, "Cannot parse vehicle on client: %s", name);
        }
    }
}

void __cdecl CG_RegisterVehicleMaterial(__int16 index)
{
    if ( index >= bg_numVehicleInfos
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_main.cpp",
                    144,
                    0,
                    "%s",
                    "index < bg_numVehicleInfos") )
    {
        __debugbreak();
    }
    bg_vehicleInfos[index].compassIconMaterial = Material_RegisterHandle(bg_vehicleInfos[index].compassIcon, 7);
}

void __cdecl CG_Veh_Init()
{
    char *name; // [esp+0h] [ebp-Ch]
    __int16 j; // [esp+4h] [ebp-8h]
    signed int i; // [esp+8h] [ebp-4h]

    i = 3148;
    j = 0;
    while ( i <= 3179 && j < 32 )
    {
        name = CL_GetConfigString(i);
        if ( *name )
            CG_RegisterVehicle(name, j++);
        ++i;
    }
}

void __cdecl CG_Veh_RegisterMaterials()
{
    __int16 index; // [esp+0h] [ebp-4h]

    for ( index = 0; index < bg_numVehicleInfos; ++index )
        CG_RegisterVehicleMaterial(index);
}

char __cdecl CG_VehicleUpdateField(const char *vehicleInfoName, char *keyValue)
{
    int i; // [esp+4h] [ebp-4h]

    if ( !vehicleInfoName || !*vehicleInfoName )
        return 0;
    for ( i = 0; i < bg_numVehicleInfos; ++i )
    {
        if ( !I_stricmp(vehicleInfoName, bg_vehicleInfos[i].name) )
            ParseConfigStringToStruct(
                (unsigned __int8 *)&bg_vehicleInfos[i],
                s_vehicleFields,
                s_numVehicleFields,
                keyValue,
                25,
                (int (__cdecl *)(unsigned __int8 *, const char *, const int, const int))VEH_ParseSpecificField,
                BG_StringCopy);
    }
    CG_UpdateVehicleBindings(0);
    return 1;
}

bool __cdecl CG_IsVehicleRemoteControl(int index)
{
    if ( index < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_main.cpp", 207, 0, "%s", "index >= 0") )
    {
        __debugbreak();
    }
    if ( index >= 32
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_main.cpp",
                    208,
                    0,
                    "%s",
                    "index < MAX_VEHICLE_FILES") )
    {
        __debugbreak();
    }
    return CG_GetVehicleInfo(index)->remoteControl != 0;
}

void __cdecl CG_UpdateUIDeviceContexts(int localClientNum, int time)
{
    cgDC[localClientNum].realTime = time;
}

double __cdecl CG_GetDefaultFovForView()
{
    if ( cg_thirdPerson->current.integer )
        return cg_fov_default_thirdperson->current.value;
    else
        return cg_fov_default->current.value;
}

void __cdecl CG_UpdateFov(float value)
{
    if ( cg_fov->current.value != value )
        Dvar_SetFloat((dvar_s*)cg_fov, value);
}

void __cdecl CG_SetThirdPerson(int value)
{
    float DefaultFovForView; // [esp+0h] [ebp-4h]

    if ( cg_thirdPerson->current.integer != value )
    {
        Dvar_SetInt((dvar_s *)cg_thirdPerson, value);
        Dvar_SetBoolByName("cg_spectateThirdPerson", value > 0);
    }
    DefaultFovForView = CG_GetDefaultFovForView();
    CG_UpdateFov(DefaultFovForView);
}

bool __cdecl CG_IsEntWalkable(int localClientNum, int entityNum)
{
    return CG_GetEntity(localClientNum, entityNum)->nextState.eType != 1;
}

int __cdecl CScriptMover_UpdateMove(
    trajectory_t *pTr,
    float *vCurrPos,
    float fSpeed,
    float fMidTime,
    float fDecelTime,
    const float *vPos1,
    const float *vPos2,
    const float *vPos3)
{
    float fDelta; // [esp+38h] [ebp-14h]
    float vMove[3]; // [esp+3Ch] [ebp-10h] BYREF
    int trDuration; // [esp+48h] [ebp-4h]

    trDuration = (int)(float)(fMidTime * 1000.0);
    if (pTr->trType == 8 && trDuration > 0)
    {
        pTr->trTime = bgs->time;
        pTr->trDuration = trDuration;
        pTr->trBase[0] = *vPos1;
        pTr->trBase[1] = vPos1[1];
        pTr->trBase[2] = vPos1[2];
        vMove[0] = *vPos2 - *vPos1;
        vMove[1] = vPos2[1] - vPos1[1];
        vMove[2] = vPos2[2] - vPos1[2];
        if (!trDuration
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_main.cpp", 313, 0, "%s", "trDuration"))
        {
            __debugbreak();
        }
        fDelta = 1000.0 / (float)trDuration;
        pTr->trDelta[0] = fDelta * vMove[0];
        pTr->trDelta[1] = fDelta * vMove[1];
        pTr->trDelta[2] = fDelta * vMove[2];
        if (((LODWORD(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
            || (LODWORD(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
            || (LODWORD(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_main.cpp",
                316,
                0,
                "%s",
                "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])"))
        {
            __debugbreak();
        }
        pTr->trType = 4;
        return 0;
    }
    if ((pTr->trType == 8 && trDuration <= 0 || pTr->trType == 4) && fDecelTime > 0.0)
    {
        pTr->trTime = bgs->time;
        pTr->trDuration = (int)(float)(fDecelTime * 1000.0);
        pTr->trBase[0] = *vPos2;
        pTr->trBase[1] = vPos2[1];
        pTr->trBase[2] = vPos2[2];
        vMove[0] = *vPos3 - *vPos2;
        vMove[1] = vPos3[1] - vPos2[1];
        vMove[2] = vPos3[2] - vPos2[2];
        Vec3Normalize(vMove);
        vMove[0] = fSpeed * vMove[0];
        vMove[1] = fSpeed * vMove[1];
        vMove[2] = fSpeed * vMove[2];
        pTr->trDelta[0] = vMove[0];
        pTr->trDelta[1] = vMove[1];
        pTr->trDelta[2] = vMove[2];
        if (((LODWORD(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
            || (LODWORD(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
            || (LODWORD(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_main.cpp",
                330,
                0,
                "%s",
                "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])"))
        {
            __debugbreak();
        }
        pTr->trType = 9;
        return 0;
    }
    if (pTr->trType == 6 || pTr->trType == 13)
    {
        BG_EvaluateTrajectory(pTr, bgs->time, pTr->trBase);
    }
    else
    {
        pTr->trBase[0] = *vPos3;
        pTr->trBase[1] = vPos3[1];
        pTr->trBase[2] = vPos3[2];
    }
    pTr->trTime = bgs->time;
    pTr->trType = 0;
    return 1;
}
