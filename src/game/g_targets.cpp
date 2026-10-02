#include "g_targets.h"
#include <server_mp/sv_init_mp.h>
#include <clientscript/cscr_vm.h>
#include <game_mp/g_spawn_mp.h>
#include <game_mp/g_utils_mp.h>
#include <client_mp/g_client_mp.h>
#include <server/sv_game.h>
#include <server_mp/sv_main_mp.h>
#include <clientscript/scr_const.h>


struct //$A5C519FFED38118F396585C413DE405F // sizeof=0x384
{                                       // XREF: .data:targGlob/r
    target_t targets[32];               // XREF: G_InitTargets(void)+2C/w
    unsigned int targetCount;           // XREF: G_InitTargets(void)+4/w
} targGlob;

void __cdecl G_InitTargets()
{
    targGlob.targetCount = 0;
    for ( int i = 0; i < 32; ++i )
    {
        targGlob.targets[i].ent = 0;
        SV_SetConfigstring(i + 387, 0);
    }
}

void __cdecl Scr_Target_SetShader()
{
    const char *v0; // eax
    char *String; // eax
    const char *v2; // eax
    int v3; // [esp+0h] [ebp-414h]
    unsigned int targetIndex; // [esp+4h] [ebp-410h]
    char configString[1024]; // [esp+Ch] [ebp-408h] BYREF
    gentity_s *ent; // [esp+410h] [ebp-4h]

    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) < 2 )
        Scr_Error("Too few arguments\n", 0);
    ent = Scr_GetEntity(0);
    for ( targetIndex = 0; targetIndex < 0x20 && targGlob.targets[targetIndex].ent != ent; ++targetIndex )
        ;
    if ( targetIndex == 32 )
    {
        v0 = va("Entity %i is not a target", ent->s.number);
        Scr_Error(v0, 0);
    }
    if ( *Scr_GetString(1u, SCRIPTINSTANCE_SERVER) )
    {
        String = Scr_GetString(1u, SCRIPTINSTANCE_SERVER);
        v3 = G_MaterialIndex(String);
    }
    else
    {
        v3 = -1;
    }
    targGlob.targets[targetIndex].materialIndex = v3;
    SV_GetConfigstring(targetIndex + 387, configString, 1024);
    v2 = va("%i", targGlob.targets[targetIndex].materialIndex);
    Info_SetValueForKey(configString, "mat", v2);
    SV_SetConfigstring(targetIndex + 387, configString);
}

void __cdecl Scr_Target_SetOffscreenShader()
{
    const char *v0; // eax
    char *String; // eax
    const char *v2; // eax
    int v3; // [esp+0h] [ebp-414h]
    unsigned int targetIndex; // [esp+4h] [ebp-410h]
    char configString[1024]; // [esp+Ch] [ebp-408h] BYREF
    gentity_s *ent; // [esp+410h] [ebp-4h]

    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) < 2 )
        Scr_Error("Too few arguments\n", 0);
    ent = Scr_GetEntity(0);
    for ( targetIndex = 0; targetIndex < 0x20 && targGlob.targets[targetIndex].ent != ent; ++targetIndex )
        ;
    if ( targetIndex == 32 )
    {
        v0 = va("Entity %i is not a target", ent->s.number);
        Scr_Error(v0, 0);
    }
    if ( *Scr_GetString(1u, SCRIPTINSTANCE_SERVER) )
    {
        String = Scr_GetString(1u, SCRIPTINSTANCE_SERVER);
        v3 = G_MaterialIndex(String);
    }
    else
    {
        v3 = -1;
    }
    targGlob.targets[targetIndex].offscreenMaterialIndex = v3;
    SV_GetConfigstring(targetIndex + 387, configString, 1024);
    v2 = va("%i", targGlob.targets[targetIndex].offscreenMaterialIndex);
    Info_SetValueForKey(configString, "offmat", v2);
    SV_SetConfigstring(targetIndex + 387, configString);
}

void __cdecl Scr_Target_GetArray()
{
    unsigned int targIdx; // [esp+0h] [ebp-4h]

    Scr_MakeArray(SCRIPTINSTANCE_SERVER);
    for ( targIdx = 0; targIdx < MAX_TARGETS; ++targIdx )
    {
        if ( targGlob.targets[targIdx].ent )
        {
            if ( targGlob.targets[targIdx].ent->r.inuse )
            {
                Scr_AddEntity(targGlob.targets[targIdx].ent, SCRIPTINSTANCE_SERVER);
                Scr_AddArray(SCRIPTINSTANCE_SERVER);
            }
        }
    }
}

unsigned int __cdecl Target_GetTargetIndex(gentity_s *ent)
{
    for ( unsigned int targIdx = 0; targIdx < MAX_TARGETS; ++targIdx )
    {
        if ( targGlob.targets[targIdx].ent == ent )
            return targIdx;
    }

    return MAX_TARGETS;
}

void __cdecl Scr_Target_IsTarget()
{
    gentity_s *ent; // [esp+4h] [ebp-4h]

    if ( !Scr_GetNumParam(SCRIPTINSTANCE_SERVER) )
        Scr_Error("Too few arguments\n", 0);
    ent = Scr_GetEntity(0);
    if ( Target_GetTargetIndex(ent) == MAX_TARGETS )
        Scr_AddBool(0, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddBool(1, SCRIPTINSTANCE_SERVER);
}

void __cdecl Scr_Target_SetTurretAquire()
{
    int v0; // edx
    unsigned int targIdx; // [esp+0h] [ebp-Ch]
    bool ignoreTarg; // [esp+4h] [ebp-8h]
    gentity_s *ent; // [esp+8h] [ebp-4h]

    ignoreTarg = 1;
    ent = Scr_GetEntity(0);
    targIdx = Target_GetTargetIndex(ent);
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 1 )
        ignoreTarg = Scr_GetInt(1u, SCRIPTINSTANCE_SERVER) != 0;
    if ( targIdx != MAX_TARGETS )
    {
        if ( ignoreTarg )
            v0 = targGlob.targets[targIdx].flags & 0xFFFFFFFB;
        else
            v0 = targGlob.targets[targIdx].flags | 4;
        targGlob.targets[targIdx].flags = v0;
    }
}

void __cdecl Scr_Target_Set()
{
    char *v1; // eax
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    float *v6; // [esp+0h] [ebp-414h]
    float *offset; // [esp+4h] [ebp-410h]
    unsigned int targetIndex; // [esp+8h] [ebp-40Ch]
    char configString[1024]; // [esp+Ch] [ebp-408h] BYREF
    gentity_s *ent; // [esp+410h] [ebp-4h]

    if (!Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("Too few arguments\n", 0);
    ent = Scr_GetEntity(0);
    for (targetIndex = 0; targetIndex < 0x20 && targGlob.targets[targetIndex].ent != ent; ++targetIndex)
        ;
    if (targetIndex == MAX_TARGETS)
    {
        if (targGlob.targetCount >= 0x20)
            Scr_Error("Maximum number of targets exceeded", 0);
        for (targetIndex = 0; targetIndex < 0x20 && targGlob.targets[targetIndex].ent; ++targetIndex)
            ;
        bcassert(targetIndex, MAX_TARGETS);
        
        targGlob.targets[targetIndex].ent = ent;
        ent->flags |= 0x1000000u;
        targGlob.targets[targetIndex].materialIndex = -1;
        targGlob.targets[targetIndex].offscreenMaterialIndex = -1;
        offset = targGlob.targets[targetIndex].offset;
        *offset = 0.0f;
        offset[1] = 0.0f;
        offset[2] = 0.0f;
        ++targGlob.targetCount;
    }
    if ((unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) <= 1)
    {
        v6 = targGlob.targets[targetIndex].offset;
        *v6 = 0.0f;
        v6[1] = 0.0f;
        v6[2] = 0.0f;
    }
    else
    {
        Scr_GetVector(1u, targGlob.targets[targetIndex].offset, SCRIPTINSTANCE_SERVER);
    }
    targGlob.targets[targetIndex].flags &= ~4u;
    configString[0] = 0;
    v1 = va("%i", ent->s.number);
    Info_SetValueForKey(configString, "ent", v1);
    v2 = va(
        "%i %i %i",
        (int)targGlob.targets[targetIndex].offset[0],
        (int)targGlob.targets[targetIndex].offset[1],
        (int)targGlob.targets[targetIndex].offset[2]);
    Info_SetValueForKey(configString, "offs", v2);
    v3 = va("%i", targGlob.targets[targetIndex].materialIndex);
    Info_SetValueForKey(configString, "mat", v3);
    v4 = va("%i", targGlob.targets[targetIndex].offscreenMaterialIndex);
    Info_SetValueForKey(configString, "offmat", v4);
    v5 = va("%i", targGlob.targets[targetIndex].flags);
    Info_SetValueForKey(configString, "flags", v5);
    SV_SetConfigstring(targetIndex + 387, configString);
}

bool Targ_Remove(gentity_s *ent)
{
    unsigned int targetIndex; // [esp+0h] [ebp-4h]

    for ( targetIndex = 0; ; ++targetIndex )
    {
        if ( targetIndex >= MAX_TARGETS )
            return 0;
        if ( targGlob.targets[targetIndex].ent == ent )
            break;
    }
    targGlob.targets[targetIndex].ent = 0;
    targGlob.targetCount--;
    bcassert(targGlob.targetCount, MAX_TARGETS);
    
    SV_SetConfigstring(CS_TARGETS + targetIndex, (char *)"");
    return 1;
}

void __cdecl Targ_RemoveAll()
{
    for ( unsigned int targetIndex = 0; targetIndex < MAX_TARGETS; ++targetIndex )
    {
        if ( targGlob.targets[targetIndex].ent )
        {
            targGlob.targets[targetIndex].ent = 0;
            targGlob.targetCount--;
            bcassert(targGlob.targetCount, MAX_TARGETS);
            
            SV_SetConfigstring(CS_TARGETS + targetIndex, (char *)"");
        }
    }
}

void __cdecl Scr_Target_Remove()
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    if ( !Scr_GetNumParam(SCRIPTINSTANCE_SERVER) )
        Scr_Error("Too few arguments\n", 0);
    ent = Scr_GetEntity(0);
    if ( !Targ_Remove(ent) )
    {
        Scr_Error(va("Entity %i is not a target", ent->s.number), 0);
    }
}

void __cdecl Scr_Target_IsInCircle()
{
    float radius; // [esp+0h] [ebp-Ch]
    float screenPos[2]; // [esp+4h] [ebp-8h] BYREF

    radius = Scr_GetFloat(3u, SCRIPTINSTANCE_SERVER);
    if ( ScrGetTargetScreenPos(screenPos)
        && (float)(radius * radius) > (float)((float)(screenPos[0] * screenPos[0]) + (float)(screenPos[1] * screenPos[1])) )
    {
        Scr_AddBool(1u, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Scr_AddBool(0, SCRIPTINSTANCE_SERVER);
    }
}

char __cdecl ScrGetTargetScreenPos(float *screenPos)
{
    const char *v1; // eax
    gentity_s *v2; // eax
    float pos[3]; // [esp+18h] [ebp-28h] BYREF
    float viewPos[3]; // [esp+24h] [ebp-1Ch] BYREF
    unsigned int targetIndex; // [esp+30h] [ebp-10h]
    gentity_s *ent; // [esp+34h] [ebp-Ch]
    float fov; // [esp+38h] [ebp-8h]
    gentity_s *player; // [esp+3Ch] [ebp-4h]

    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) < 2 )
        Scr_Error("Too few arguments\n", 0);
    ent = Scr_GetEntity(0);
    player = Scr_GetEntity(1u);
    if ( !player->client )
    {
        v1 = va("entity %i is not a player", player->s.number);
        Scr_ObjectError(v1, SCRIPTINSTANCE_SERVER);
    }
    fov = Scr_GetFloat(2u, SCRIPTINSTANCE_SERVER);
    if ( fov <= 0.0 )
        Scr_ParamError(2u, "FOV must be positive", SCRIPTINSTANCE_SERVER);
    for ( targetIndex = 0; targetIndex < 0x20 && targGlob.targets[targetIndex].ent != ent; ++targetIndex )
        ;
    if ( targetIndex == 32 )
    {
        pos[0] = ent->r.currentOrigin[0];
        pos[1] = ent->r.currentOrigin[1];
        pos[2] = ent->r.currentOrigin[2] + 30.0;
    }
    else
    {
        v2 = targGlob.targets[targetIndex].ent;
        pos[0] = v2->r.currentOrigin[0] + targGlob.targets[targetIndex].offset[0];
        pos[1] = v2->r.currentOrigin[1] + targGlob.targets[targetIndex].offset[1];
        pos[2] = v2->r.currentOrigin[2] + targGlob.targets[targetIndex].offset[2];
    }
    G_GetPlayerViewOrigin(&player->client->ps, viewPos);
    pos[0] = pos[0] - viewPos[0];
    pos[1] = pos[1] - viewPos[1];
    pos[2] = pos[2] - viewPos[2];
    return G_WorldDirToScreenPos(player, fov, pos, screenPos);
}

char __cdecl G_WorldDirToScreenPos(const gentity_s *player, float fov_x, const float *worldDir, float *outScreenPos)
{
#if 0
    float dirCamera[3]; // [esp+0h] [ebp-40h] BYREF
    float vertTan; // [esp+Ch] [ebp-34h]
    float tanHalfFovX; // [esp+10h] [ebp-30h]
    float tanHalfFovY; // [esp+14h] [ebp-2Ch]
    float cameraMtx[3][3]; // [esp+18h] [ebp-28h] BYREF
    float horzTan; // [esp+3Ch] [ebp-4h]

    if ( fov_x <= 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_targets.cpp", 401, 0, "%s", "fov_x > 0") )
    {
        __debugbreak();
    }
    G_GetPlayerViewDirection(player, cameraMtx[0], cameraMtx[1], cameraMtx[2]);
    MatrixTransposeTransformVector(worldDir, cameraMtx, dirCamera);
    if ( dirCamera[0] <= 0.0 )
        return 0;
    horzTan = dirCamera[1] / dirCamera[0];
    vertTan = dirCamera[2] / dirCamera[0];
    __libm_sse2_tan(*(long double *)dirCamera);
    tanHalfFovX = (float)(DEG2RAD(fov_x)) * 0.5;
    tanHalfFovY = tanHalfFovX * 0.75;
    if ( tanHalfFovX <= 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_targets.cpp", 415, 1, "%s", "tanHalfFovX > 0") )
    {
        __debugbreak();
    }
    if ( tanHalfFovY <= 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_targets.cpp", 416, 1, "%s", "tanHalfFovY > 0") )
    {
        __debugbreak();
    }
    *outScreenPos = (float)(-320.0 * horzTan) / tanHalfFovX;
    outScreenPos[1] = (float)(-240.0 * vertTan) / tanHalfFovY;
    return 1;
#else
    // AISLOP! (sse tan)
    float dirCamera[3];
    float cameraMtx[3][3];
    float horzTan, vertTan;
    float tanHalfFovX, tanHalfFovY;
    float halfFovRad;

    if (fov_x <= 0.0f &&
        !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_targets.cpp",
            401,
            0,
            "%s",
            "fov_x > 0"))
    {
        __debugbreak();
    }

    G_GetPlayerViewDirection(player,
        cameraMtx[0],
        cameraMtx[1],
        cameraMtx[2]);

    MatrixTransposeTransformVector(worldDir, cameraMtx, dirCamera);

    /* Behind camera */
    if (dirCamera[0] <= 0.0f)
        return 0;

    horzTan = dirCamera[1] / dirCamera[0];
    vertTan = dirCamera[2] / dirCamera[0];

    /* ---- Correct tan() usage ---- */

    halfFovRad = ((float)DEG2RAD(fov_x)) * 0.5f;
    tanHalfFovX = tanf(halfFovRad);

    /* IW engine uses 4:3 virtual projection (320x240) */
    tanHalfFovY = tanHalfFovX * 0.75f;

    if (tanHalfFovX <= 0.0f &&
        !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_targets.cpp",
            415,
            1,
            "%s",
            "tanHalfFovX > 0"))
    {
        __debugbreak();
    }

    if (tanHalfFovY <= 0.0f &&
        !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_targets.cpp",
            416,
            1,
            "%s",
            "tanHalfFovY > 0"))
    {
        __debugbreak();
    }

    outScreenPos[0] = (-320.0f * horzTan) / tanHalfFovX;
    outScreenPos[1] = (-240.0f * vertTan) / tanHalfFovY;

    return 1;
#endif
}

void __cdecl Scr_Target_IsInRect()
{
    float halfWidth; // [esp+8h] [ebp-10h]
    float screenPos[2]; // [esp+Ch] [ebp-Ch] BYREF
    float halfHeight; // [esp+14h] [ebp-4h]

    halfWidth = Scr_GetFloat(3u, SCRIPTINSTANCE_SERVER);
    halfHeight = Scr_GetFloat(4u, SCRIPTINSTANCE_SERVER);
    if ( ScrGetTargetScreenPos(screenPos)
        && halfWidth > fabs(screenPos[0])
        && halfHeight > fabs(screenPos[1]) )
    {
        Scr_AddBool(1u, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Scr_AddBool(0, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl Scr_Target_StartLockOn()
{
    const char *v0; // eax
    float duration; // [esp+0h] [ebp-Ch]
    gentity_s *ent; // [esp+8h] [ebp-4h]

    ent = Scr_GetEntity(0);
    duration = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
    v0 = va("%c %i %i", 84, ent->s.number, (int)(float)(duration * 1000.0));
    SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, v0);
}

void __cdecl Scr_Target_ClearLockOn()
{
    const char *v0; // eax

    v0 = va("%c %i %i", 84, 1023, 0);
    SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, v0);
}

char __cdecl G_TargetGetOffset(const gentity_s *targ, float *result)
{
    unsigned int targetIndex; // [esp+4h] [ebp-4h]

    targetIndex = GetTargetIdx(targ);
    if ( targetIndex == 32 )
    {
        *result = 0.0f;
        result[1] = 0.0f;
        result[2] = 0.0f;
        return 0;
    }
    else
    {
        *result = targGlob.targets[targetIndex].offset[0];
        result[1] = targGlob.targets[targetIndex].offset[1];
        result[2] = targGlob.targets[targetIndex].offset[2];
        return 1;
    }
}

unsigned int __cdecl GetTargetIdx(const gentity_s *ent)
{
    unsigned int targetIdx; // [esp+0h] [ebp-4h]

    if ( !ent )
        return 32;
    for ( targetIdx = 0; targetIdx < 0x20; ++targetIdx )
    {
        if ( targGlob.targets[targetIdx].ent == ent )
            return targetIdx;
    }
    return 32;
}

bool __cdecl G_TargetAttackProfileTop(const gentity_s *ent)
{
    unsigned int targIdx; // [esp+0h] [ebp-4h]

    targIdx = GetTargetIdx(ent);
    return targIdx != 32 && (targGlob.targets[targIdx].flags & 1) != 0;
}

void __cdecl Scr_Target_SetAttackMode()
{
    const char *v0; // eax
    const char *v1; // eax
    unsigned int targIdx; // [esp+0h] [ebp-410h]
    unsigned int modeName; // [esp+4h] [ebp-40Ch]
    char configString[1024]; // [esp+8h] [ebp-408h] BYREF
    gentity_s *ent; // [esp+40Ch] [ebp-4h]

    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) < 2 )
        Scr_Error("Too few arguments\n", 0);
    ent = Scr_GetEntity(0);
    targIdx = GetTargetIdx(ent);
    if ( targIdx == 32 )
    {
        v0 = va("Entity %i is not a target", ent->s.number);
        Scr_Error(v0, 0);
    }
    modeName = Scr_GetConstString(1u, SCRIPTINSTANCE_SERVER);
    if ( modeName == scr_const.top )
    {
        targGlob.targets[targIdx].flags |= 1u;
    }
    else if ( modeName == scr_const.direct )
    {
        targGlob.targets[targIdx].flags &= ~1u;
    }
    else
    {
        Scr_Error("Incorrect mode name passed to target_setAttackMode().\n", 0);
    }
    SV_GetConfigstring(targIdx + 387, configString, 1024);
    v1 = va("%i", targGlob.targets[targIdx].flags);
    Info_SetValueForKey(configString, "flags", v1);
    SV_SetConfigstring(targIdx + 387, configString);
}

void __cdecl Scr_Target_SetJavelinOnly()
{
    const char *v0; // eax
    int v1; // ecx
    const char *v2; // eax
    unsigned int targIdx; // [esp+0h] [ebp-410h]
    char configString[1024]; // [esp+8h] [ebp-408h] BYREF
    gentity_s *ent; // [esp+40Ch] [ebp-4h]

    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) < 2 )
        Scr_Error("Too few arguments\n", 0);
    ent = Scr_GetEntity(0);
    targIdx = GetTargetIdx(ent);
    if ( targIdx == 32 )
    {
        v0 = va("Entity %i is not a target", ent->s.number);
        Scr_Error(v0, 0);
    }
    if ( Scr_GetInt(1u, SCRIPTINSTANCE_SERVER) )
        v1 = targGlob.targets[targIdx].flags | 2;
    else
        v1 = targGlob.targets[targIdx].flags & 0xFFFFFFFD;
    targGlob.targets[targIdx].flags = v1;
    SV_GetConfigstring(targIdx + 387, configString, 1024);
    v2 = va("%i", targGlob.targets[targIdx].flags);
    Info_SetValueForKey(configString, "flags", v2);
    SV_SetConfigstring(targIdx + 387, configString);
}

target_t *__cdecl Target_GetTargetArray()
{
    return targGlob.targets;
}

