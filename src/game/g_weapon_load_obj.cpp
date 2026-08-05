#include "g_weapon_load_obj.h"

#include <cstring>
#include <universal/com_memory.h>
#include <universal/com_files.h>
#include <universal/q_parse.h>
#include <qcommon/common.h>

const char *accuracyDirName[3] = { "aivsai", "aivsplayer", NULL };


char __cdecl G_ParseWeaponAccurayGraphs(WeaponDef *weaponDef)
{
    unsigned int size; // [esp+4h] [ebp-8Ch]
    int weaponType; // [esp+8h] [ebp-88h]
    int accuracyGraphKnotCount; // [esp+Ch] [ebp-84h] BYREF
    float accuracyGraphKnots[16][2]; // [esp+10h] [ebp-80h] BYREF

    for ( weaponType = 0; weaponType < 2; ++weaponType )
    {
        memset(accuracyGraphKnots, 0, sizeof(accuracyGraphKnots));
        accuracyGraphKnotCount = 0;
        if ( !G_ParseWeaponAccurayGraphInternal(
                        weaponDef,
                        accuracyDirName[weaponType],
                        weaponDef->accuracyGraphName[weaponType],
                        accuracyGraphKnots,
                        &accuracyGraphKnotCount) )
            return 0;
        if ( accuracyGraphKnotCount > 0 )
        {
            size = 8 * accuracyGraphKnotCount;
            weaponDef->accuracyGraphKnots[weaponType] = (float (*)[2])Hunk_AllocLowAlign(
                                                                                                                                    8 * accuracyGraphKnotCount,
                                                                                                                                    4,
                                                                                                                                    "G_ParseWeaponAccurayGraphs",
                                                                                                                                    10);
            weaponDef->originalAccuracyGraphKnots[weaponType] = weaponDef->accuracyGraphKnots[weaponType];
            memcpy((unsigned __int8 *)weaponDef->accuracyGraphKnots[weaponType], (unsigned __int8 *)accuracyGraphKnots, size);
            weaponDef->accuracyGraphKnotCount[weaponType] = accuracyGraphKnotCount;
            weaponDef->originalAccuracyGraphKnotCount[weaponType] = weaponDef->accuracyGraphKnotCount[weaponType];
        }
    }
    return 1;
}

char __cdecl G_ParseWeaponAccurayGraphInternal(
                WeaponDef *weaponDef,
                const char *dirName,
                const char *graphName,
                float (*knots)[2],
                int *knotCount)
{
    signed int v6; // [esp+10h] [ebp-405Ch]
    char string[64]; // [esp+14h] [ebp-4058h] BYREF
    char buffer[16388]; // [esp+54h] [ebp-4018h] BYREF
    const char *last; // [esp+405Ch] [ebp-10h]
    int knotCounta; // [esp+4060h] [ebp-Ch] BYREF
    int f; // [esp+4064h] [ebp-8h] BYREF
    int len; // [esp+4068h] [ebp-4h]

    last = "WEAPONACCUFILE";
    len = strlen("WEAPONACCUFILE");
    if ( !weaponDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon_load_obj.cpp", 88, 0, "%s", "weaponDef") )
    {
        __debugbreak();
    }
    if ( !graphName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon_load_obj.cpp", 89, 0, "%s", "graphName") )
    {
        __debugbreak();
    }
    if ( !knots
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon_load_obj.cpp", 90, 0, "%s", "knots") )
    {
        __debugbreak();
    }
    if ( !knotCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon_load_obj.cpp", 91, 0, "%s", "knotCount") )
    {
        __debugbreak();
    }
    if ( !dirName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon_load_obj.cpp", 92, 0, "%s", "dirName") )
    {
        __debugbreak();
    }
    if ( weaponDef->weapType && weaponDef->weapType != WEAPTYPE_PROJECTILE && weaponDef->weapType != WEAPTYPE_GAS )
        return 1;
    if ( !*graphName )
        return 1;
    sprintf(string, "accuracy/%s/%s", dirName, graphName);
    v6 = FS_FOpenFileByMode(string, &f, FS_READ);
    if ( v6 >= 0 )
    {
        FS_Read((unsigned __int8 *)buffer, len, f);
        buffer[len] = 0;
        if ( !strncmp(buffer, last, len) )
        {
            if ( v6 - len < 0x4000 )
            {
                memset(buffer, 0, 0x4000u);
                FS_Read((unsigned __int8 *)buffer, v6 - len, f);
                buffer[v6 - len] = 0;
                FS_FCloseFile(f);
                knotCounta = 0;
                if ( G_ParseAIWeaponAccurayGraphFile(buffer, string, knots, &knotCounta) )
                {
                    *knotCount = knotCounta;
                    return 1;
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                Com_PrintWarning(15, "WARNING: \"%s\" Is too long of an ai weapon accuracy file to parse\n", string);
                FS_FCloseFile(f);
                return 0;
            }
        }
        else
        {
            Com_PrintWarning(15, "WARNING: \"%s\" does not appear to be an ai weapon accuracy file\n", string);
            FS_FCloseFile(f);
            return 0;
        }
    }
    else
    {
        Com_PrintWarning(15, "WARNING: Could not load ai weapon accuracy file '%s'\n", string);
        return 0;
    }
}

char __cdecl G_ParseAIWeaponAccurayGraphFile(
                const char *buffer,
                const char *fileName,
                float (*knots)[2],
                int *knotCount)
{
    int v4; // eax
    long double v5; // st7
    long double v6; // st7
    int knotCountIndex; // [esp+0h] [ebp-8h]
    parseInfo_t *tokenb; // [esp+4h] [ebp-4h]
    parseInfo_t *token; // [esp+4h] [ebp-4h]
    parseInfo_t *tokena; // [esp+4h] [ebp-4h]

    if ( !buffer
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon_load_obj.cpp", 23, 0, "%s", "buffer") )
    {
        __debugbreak();
    }
    if ( !fileName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon_load_obj.cpp", 24, 0, "%s", "fileName") )
    {
        __debugbreak();
    }
    if ( !knots
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon_load_obj.cpp", 25, 0, "%s", "knots") )
    {
        __debugbreak();
    }
    if ( !knotCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon_load_obj.cpp", 26, 0, "%s", "knotCount") )
    {
        __debugbreak();
    }
    Com_BeginParseSession(fileName);
    tokenb = Com_Parse(&buffer);
    v4 = atoi(tokenb->token);
    *knotCount = v4;
    knotCountIndex = 0;
    while ( 1 )
    {
        token = Com_Parse(&buffer);
        if ( !token->token[0] )
            break;
        if ( token->token[0] == 125 )
            break;
        v5 = atof(token->token);
        (*knots)[2 * knotCountIndex] = v5;
        tokena = Com_Parse(&buffer);
        if ( !tokena->token[0] || tokena->token[0] == 125 )
            break;
        v6 = atof(tokena->token);
        (*knots)[2 * knotCountIndex++ + 1] = v6;
        if ( knotCountIndex >= 16 )
        {
            Com_PrintWarning(15, "WARNING: \"%s\" has too many graph knots\n", fileName);
            Com_EndParseSession();
            return 0;
        }
    }
    Com_EndParseSession();
    if ( knotCountIndex == *knotCount )
    {
        if ( (*knots)[2 * knotCountIndex - 2] == 1.0 )
        {
            return 1;
        }
        else
        {
            Com_PrintError(15, "ERROR: \"%s\" Range must be 0.0 to 1.0\n", fileName);
            return 0;
        }
    }
    else
    {
        Com_PrintError(15, "ERROR: \"%s\" Error in parsing an ai weapon accuracy file\n", fileName);
        return 0;
    }
}

