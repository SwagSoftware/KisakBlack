#include "sv_main_mp.h"
#include "server/server.h"
#include <qcommon/common.h>
#include <client_mp/sv_client_mp.h>
#include <qcommon/sv_msg_write.h>
#include <string.h>
#include <stdarg.h>
#include "sv_voice_mp.h"
#include <client_mp/cl_main_mp.h>
#include <qcommon/dvar_cmds.h>
#include <qcommon/threads.h>
#include "sv_init_mp.h"
#include <qcommon/cmd.h>
#include <game_mp/g_cmds_mp.h>
#include <qcommon/files.h>
#include <game_mp/g_main_mp.h>
#include <qcommon/com_gamemodes.h>
#include <game_mp/pregame.h>
#include <universal/com_files.h>
#include "sv_main_pc_mp.h"
#include <universal/offline.h>
#include "sv_net_chan_mp.h"
#include <server/sv_game.h>
#include <win32/win_shared.h>
#include <client/cl_debugdata.h>
#include "sv_bot_mp.h"
#include <win32/win_main.h>
#include <universal/com_workercmds.h>
#include <universal/com_tasks.h>
#include <cgame/cg_perf.h>
#include <clientscript/cscr_debugger.h>
#include <universal/com_memory.h>
#include <clientscript/cscr_vm.h>
#include <setjmp.h>
#include <win32/win_steam.h>

serverStatic_t svs;
serverStaticHeader_t svsHeader;
int svsHeaderValid;
server_t sv;

bool g_shouldKillLocalServer = false; // not real name
char *g_shutdownMsg; // not real name

int gameInitialized;

unsigned __int8 tempServerMsgBuf[65536];

char string_0[1024];
char *__cdecl SV_ExpandNewlines(char *in)
{
    unsigned int l; // [esp+0h] [ebp-4h]

    l = 0;
    while ( *in && l < 0x3FD )
    {
        if ( *in == 10 )
        {
            string_0[l] = 92;
            string_0[l + 1] = 110;
            l += 2;
            goto LABEL_10;
        }
        if ( *in == 20 || *in == 21 )
        {
            ++in;
        }
        else
        {
            string_0[l++] = *in;
LABEL_10:
            ++in;
        }
    }
    string_0[l] = 0;
    return string_0;
}

void __cdecl SV_AddServerCommand(client_t *client, svscmd_type type, char *cmd)
{
    svscmd_info_t *v3; // edx
    int ackIndex; // [esp+4h] [ebp-14h]
    int from; // [esp+8h] [ebp-10h]
    int index; // [esp+Ch] [ebp-Ch]
    int i; // [esp+10h] [ebp-8h]
    int to; // [esp+14h] [ebp-4h]

    if ( !client->bIsTestClient && !client->bIsDemoClient )
    {
        if ( client->reliableSequence - client->reliableAcknowledge < 64 && client->header.state == CS_ACTIVE
            || (SV_CullIgnorableServerCommands(client), type) )
        {
            to = SV_CanReplaceServerCommand(client, cmd);
            if ( to < 0 )
            {
                ++client->reliableSequence;
            }
            else
            {
                for ( from = to + 1; from <= client->reliableSequence; ++from )
                {
                    v3 = &client->reliableCommandInfo[to & 0x7F];
                    v3->cmd = client->reliableCommandInfo[from & 0x7F].cmd;
                    v3->time = client->reliableCommandInfo[from & 0x7F].time;
                    v3->type = client->reliableCommandInfo[from & 0x7F].type;
                    ++to;
                }
            }
            if ( client->reliableSequence - client->reliableAcknowledge == 129 )
            {
                Com_Printf(15, "===== pending server commands =====\n");
                for ( i = client->reliableAcknowledge + 1; i <= client->reliableSequence; ++i )
                    Com_Printf(
                        15,
                        "cmd %5d: %8d: %s\n",
                        i,
                        client->reliableCommandInfo[i & 0x7F].time,
                        client->reliableCommandInfo[i & 0x7F].cmd);
                Com_Printf(15, "cmd %5d: %8d: %s\n", i, svs.time, cmd);
                NET_OutOfBandPrint(NS_SERVER, client->header.netchan.remoteAddress, "disconnect");
                SV_DelayDropClient(client, "EXE_SERVERCOMMANDOVERFLOW");
                type = SV_CMD_RELIABLE;
                cmd = va("%c \"EXE_SERVERCOMMANDOVERFLOW\"", 119);
            }
            index = client->reliableSequence & 0x7F;
            MSG_WriteReliableCommandToBuffer(
                cmd,
                &client->reliableCommandInfo[index].cmd,
                client->reliableCommandBuffer,
                0x4000,
                &client->reliableCommandBufferNext);
            client->reliableCommandInfo[index].time = svs.time;
            client->reliableCommandInfo[index].type = type;
            ackIndex = ((unsigned __int8)client->reliableAcknowledge + 1) & 0x7F;
            if ( ackIndex != index
                && client->reliableCommandInfo[index].cmd <= client->reliableCommandInfo[ackIndex].cmd
                && client->reliableCommandInfo[ackIndex].cmd < &client->reliableCommandBuffer[client->reliableCommandBufferNext] )
            {
                NET_OutOfBandPrint(NS_SERVER, client->header.netchan.remoteAddress, "disconnect");
                SV_DelayDropClient(client, "EXE_SERVERCOMMANDOVERFLOW");
            }
        }
    }
}

int __cdecl SV_CanReplaceServerCommand(client_t *client, const char *cmd)
{
    int result; // eax
    int index; // [esp+1Ch] [ebp-8h]
    int i; // [esp+20h] [ebp-4h]

    for ( i = client->reliableSent + 1; ; ++i )
    {
        if ( i > client->reliableSequence )
            return -1;
        index = i & 0x7F;
        if ( client->reliableCommandInfo[index].type )
        {
            if ( client->reliableCommandInfo[index].type != 1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_mp.cpp",
                            350,
                            0,
                            "%s",
                            "client->reliableCommandInfo[index].type == SV_CMD_RELIABLE") )
            {
                __debugbreak();
            }
            if ( *cmd == *client->reliableCommandInfo[index].cmd && (*cmd < 120 || *cmd > 122) )
                break;
        }
LABEL_2:
        ;
    }
    if ( !strcmp(cmd + 1, (const char *)client->reliableCommandInfo[index].cmd + 1) )
        return i;
    switch ( *cmd )
    {
        case 'C':
        case 'a':
        case 'q':
        case 't':
            result = i;
            break;
        case 'd':
        case 'v':
            if ( cmd[1] != 32
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_mp.cpp",
                            378,
                            0,
                            "%s",
                            "cmd[1] == ' '") )
            {
                __debugbreak();
            }
            if ( !SV_IsFirstTokenEqual(cmd + 2, (const char *)client->reliableCommandInfo[index].cmd + 2) )
                goto LABEL_2;
            result = i;
            break;
        default:
            goto LABEL_2;
    }
    return result;
}

bool __cdecl SV_IsFirstTokenEqual(const char *str1, const char *str2)
{
    while ( *str1 && *str2 && *str1 != 32 && *str2 != 32 )
    {
        if ( *str1 != *str2 )
            return 0;
        ++str1;
        ++str2;
    }
    return (!*str1 || *str1 == 32) && (!*str2 || *str2 == 32);
}

void __cdecl SV_CullIgnorableServerCommands(client_t *client)
{
    svscmd_info_t *v1; // edx
    int from; // [esp+0h] [ebp-10h]
    int fromIndex; // [esp+4h] [ebp-Ch]
    int to; // [esp+Ch] [ebp-4h]

    to = client->reliableSent + 1;
    for ( from = to; from <= client->reliableSequence; ++from )
    {
        fromIndex = from & 0x7F;
        if ( client->reliableCommandInfo[fromIndex].type )
        {
            if ( (to & 0x7F) != fromIndex )
            {
                v1 = &client->reliableCommandInfo[to & 0x7F];
                v1->cmd = client->reliableCommandInfo[fromIndex].cmd;
                v1->time = client->reliableCommandInfo[fromIndex].time;
                v1->type = client->reliableCommandInfo[fromIndex].type;
            }
            ++to;
        }
    }
    client->reliableSequence = to - 1;
}

unsigned __int8 tempServerCommandBuf[65536];
void SV_SendServerCommand(client_t *cl, svscmd_type type, const char *fmt, ...)
{
    client_t *client; // [esp+0h] [ebp-Ch]
    int j; // [esp+4h] [ebp-8h]
    va_list va; // [esp+20h] [ebp+14h] BYREF

    va_start(va, fmt);
    _vsnprintf((char *)tempServerCommandBuf, 0x10000u, fmt, va);
    if ( cl )
    {
        SV_AddServerCommand(cl, type, (char *)tempServerCommandBuf);
    }
    else
    {
        if ( IsDedicatedServer() && !strncmp((const char *)tempServerCommandBuf, "print", 5u) )
        {
            Com_Printf(15, "broadcast: %s\n", SV_ExpandNewlines((char *)tempServerCommandBuf));
        }

        j = 0;
        client = svs.clients;
        while ( j < com_maxclients->current.integer )
        {
            if ( client->header.state >= CS_CLIENTLOADING )
                SV_AddServerCommand(client, type, (char *)tempServerCommandBuf);
            ++j;
            ++client;
        }
    }
}

client_t *__cdecl SV_FindClientByAddress(netadr_t from, int qport)
{
    client_t *j; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]

    i = 0;
    for ( j = svs.clients; ; ++j )
    {
        if ( i >= com_maxclients->current.integer )
            return 0;
        if ( j->header.state
            && NET_CompareBaseAdr(from, j->header.netchan.remoteAddress)
            && j->header.netchan.qport == qport )
        {
            break;
        }
        ++i;
    }
    if ( j->header.netchan.remoteAddress.port != from.port )
    {
        Com_Printf(15, "SV_ReadPackets: fixing up a translated port\n");
        j->header.netchan.remoteAddress.port = from.port;
    }
    return j;
}

void __cdecl SV_VoicePacket(netadr_t from, msg_t *msg)
{
    client_t *ClientByAddress; // [esp+0h] [ebp-Ch]
    int qport; // [esp+4h] [ebp-8h]

    qport = MSG_ReadShort(msg);
    ClientByAddress = SV_FindClientByAddress(from, qport);
    if ( ClientByAddress && ClientByAddress->header.state != CS_ZOMBIE )
    {
        ClientByAddress->lastPacketTime = svs.time;
        if ( ClientByAddress->header.state >= CS_ACTIVE )
        {
            if ( !ClientByAddress->gentity
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_mp.cpp",
                            606,
                            0,
                            "%s",
                            "cl->gentity") )
            {
                __debugbreak();
            }
            SV_UserVoice(ClientByAddress, msg);
        }
        else
        {
            SV_PreGameUserVoice(ClientByAddress, msg);
        }
    }
}

void __cdecl SVC_Status(netadr_t from, XNKID *secID)
{
    char *v2; // eax
    const char *v3; // eax
    char *v4; // eax
    const char *v5; // eax
    char v6; // [esp+23h] [ebp-8469h]
    char *p_s; // [esp+28h] [ebp-8464h]
    char *v8; // [esp+2Ch] [ebp-8460h]
    int argIndex; // [esp+30h] [ebp-845Ch]
    char dest[1024]; // [esp+34h] [ebp-8458h] BYREF
    char s[16384]; // [esp+434h] [ebp-8058h] BYREF
    char *text_in; // [esp+4438h] [ebp-4054h]
    unsigned __int8 dst[16436]; // [esp+443Ch] [ebp-4050h] BYREF
    const char *v15; // [esp+8470h] [ebp-1Ch]
    const char *String; // [esp+8474h] [ebp-18h]
    unsigned __int8 *v17; // [esp+8478h] [ebp-14h]
    char *value; // [esp+847Ch] [ebp-10h]
    int v19; // [esp+8480h] [ebp-Ch]
    int v20; // [esp+8484h] [ebp-8h]

    v19 = 0;
    memset(dst, 0, 0x4029u);
    v8 = Dvar_InfoString(0, 4);
    p_s = s;
    do
    {
        v6 = *v8;
        *p_s++ = *v8++;
    }
    while ( v6 );
    if ( Dvar_GetBool("fs_restrict") )
    {
        v2 = Info_ValueForKey(s, (char*)"sv_keywords");
        Com_sprintf(dest, 0x400u, "demo %s", v2);
        Info_SetValueForKey(s, (char *)"sv_keywords", dest);
    }
    if ( !Sys_IsServerThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_mp.cpp",
                    772,
                    0,
                    "%s",
                    "Sys_IsServerThread()") )
    {
        __debugbreak();
    }
    String = Dvar_GetString("g_password");
    if ( String && *String )
        Info_SetValueForKey(s, (char *)"pswrd", "1");
    else
        Info_SetValueForKey(s, (char *)"pswrd", "0");
    value = (char *)Dvar_GetString("ui_serverinfomessage");
    if ( value && *value )
        Info_SetValueForKey(s, (char *)"ui_serverinfomessage", value);
    v3 = Dvar_GetString("sv_geolocation");
    Info_SetValueForKey(s, (char *)"geolocation", v3);
    v15 = Dvar_GetString("fs_game");
    if ( !sv_pure->current.enabled || v15 && *v15 )
    {
        v19 = 1;
    }
    else
    {
        text_in = (char *)Dvar_GetString("sv_referencedIwdNames");
        if ( *text_in )
        {
            SV_Cmd_TokenizeString(text_in);
            v20 = SV_Cmd_Argc();
            for ( argIndex = 0; argIndex < v20; ++argIndex )
            {
                v4 = (char *)SV_Cmd_Argv(argIndex);
                if ( !FS_iwIwd(v4, (char *)"main") )
                {
                    v19 = 1;
                    break;
                }
            }
            SV_Cmd_EndTokenizedString();
        }
    }
    v5 = va("%i", v19);
    Info_SetValueForKey(s, (char *)"mod", v5);
    dst[0] = 5;
    v17 = &dst[1];
    *(XNKID *)&dst[1] = *secID;
    v17 = &dst[9];
    memcpy(&dst[9], (unsigned __int8 *)s, strlen(s));
    NET_OutOfBandPrint(NS_SERVER, from, (char*)dst);
}

void __cdecl SVC_StatusScoreBoard(netadr_t from, XNKID *secID)
{
    team_t ClientTeam; // eax
    int v3; // eax
    int ClientRank; // [esp-1Ch] [ebp-44BCh]
    int ClientScore; // [esp-18h] [ebp-44B8h]
    int ClientKills; // [esp-14h] [ebp-44B4h]
    int ClientAssists; // [esp-10h] [ebp-44B0h]
    int ClientDeaths; // [esp-Ch] [ebp-44ACh]
    int ping; // [esp-8h] [ebp-44A8h]
    const char *v10; // [esp+0h] [ebp-44A0h]
    char v11; // [esp+13h] [ebp-448Dh]
    unsigned __int8 *v12; // [esp+18h] [ebp-4488h]
    char *p_dest; // [esp+1Ch] [ebp-4484h]
    client_t *v14; // [esp+30h] [ebp-4470h]
    char string[32]; // [esp+34h] [ebp-446Ch] BYREF
    int v16; // [esp+54h] [ebp-444Ch]
    unsigned __int8 dst[16436]; // [esp+58h] [ebp-4448h] BYREF
    unsigned __int8 *v18; // [esp+408Ch] [ebp-414h]
    int i; // [esp+4090h] [ebp-410h]
    unsigned int count; // [esp+4094h] [ebp-40Ch]
    char dest[1024]; // [esp+4098h] [ebp-408h] BYREF

    if ( !Sys_IsServerThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_mp.cpp",
                    838,
                    0,
                    "%s",
                    "Sys_IsServerThread()") )
    {
        __debugbreak();
    }
    memset(tempServerMsgBuf, 0, sizeof(tempServerMsgBuf));
    memset(dst, 0, 0x4000u);
    count = 0;
    for ( i = 0; i < com_maxclients->current.integer; ++i )
    {
        v14 = &svs.clients[i];
        if ( v14->header.state >= CS_CONNECTED )
        {
            if ( v14->clanAbbrev[0] )
                _snprintf(string, 0x20u, "[%s]%s", v14->clanAbbrev, v14->name);
            else
                _snprintf(string, 0x20u, "%s", v14->name);
            ping = v14->ping;
            if ( gameInitialized )
            {
                ClientDeaths = G_GetClientDeaths(v14 - svs.clients);
                ClientAssists = G_GetClientAssists(v14 - svs.clients);
                ClientKills = G_GetClientKills(v14 - svs.clients);
                ClientScore = G_GetClientScore(v14 - svs.clients);
                ClientRank = G_GetClientRank(v14 - svs.clients);
                ClientTeam = G_GetClientTeam(v14 - svs.clients);
                Com_sprintf(
                    dest,
                    0x400u,
                    "%i %i %i %i %i %i %i %s\n",
                    ClientTeam,
                    ClientRank,
                    ClientScore,
                    ClientKills,
                    ClientAssists,
                    ClientDeaths,
                    ping,
                    string);
            }
            else
            {
                v3 = G_GetClientRank(v14 - svs.clients);
                Com_sprintf(dest, 0x400u, "%i %i %i %i %i %i %i %s\n", 3, v3, 0, 0, 0, ping, string, "kisak swag"); // KISAKTODO: last string sprintf, wrong
            }
            v16 = strlen(dest);
            if ( (int)(v16 + count) >= 0x10000 )
                break;
            p_dest = dest;
            v12 = &tempServerMsgBuf[count];
            do
            {
                v11 = *p_dest;
                *v12++ = *p_dest++;
            }
            while ( v11 );
            count += v16;
        }
    }
    dst[0] = 7;
    v18 = &dst[1];
    *(XNKID *)&dst[1] = *secID;
    v18 = &dst[9];
    memcpy(&dst[9], tempServerMsgBuf, count);
    NET_OutOfBandPrint(NS_SERVER, from, (char*)dst);
}

void __cdecl SVC_Info(netadr_t from, XNKID *secID, bool quick)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    const char *v12; // eax
    const char *v13; // eax
    char *v14; // eax
    const char *v15; // eax
    const char *v16; // eax
    const char *v17; // eax
    int Int; // eax
    const char *v19; // eax
    int LicenseType; // eax
    const char *v21; // eax
    const char *v22; // eax
    const char *String; // eax
    const char *v24; // eax
    int v25; // eax
    const char *v26; // eax
    int v27; // eax
    const char *v28; // eax
    int clientCount; // [esp+38h] [ebp-840h]
    int friendlyfire; // [esp+3Ch] [ebp-83Ch]
    char infostring[1028]; // [esp+40h] [ebp-838h] BYREF
    const char *refIwdNames; // [esp+444h] [ebp-434h]
    int hardcore; // [esp+448h] [ebp-430h]
    const char *gamedir; // [esp+44Ch] [ebp-42Ch]
    int killcam; // [esp+450h] [ebp-428h]
    int maxclients; // [esp+454h] [ebp-424h]
    const char *password; // [esp+458h] [ebp-420h]
    int privateClientCount; // [esp+45Ch] [ebp-41Ch]
    char *ptr; // [esp+460h] [ebp-418h]
    int i; // [esp+464h] [ebp-414h]
    int serverModded; // [esp+468h] [ebp-410h]
    int count; // [esp+46Ch] [ebp-40Ch]
    char response[1028]; // [esp+470h] [ebp-408h] BYREF

    serverModded = 0;
    privateClientCount = 0;
    for ( i = 0; i < Com_GetPrivateClients(); ++i )
    {
        if ( svs.clients[i].header.state >= CS_CONNECTED )
            ++privateClientCount;
    }
    if ( (unsigned int)privateClientCount >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_mp.cpp",
                    1277,
                    0,
                    "privateClientCount doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    privateClientCount,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    clientCount = privateClientCount;
    for ( i = Com_GetPrivateClients(); i < com_maxclients->current.integer; ++i )
    {
        if ( !svs.clients[i].bIsDemoClient && svs.clients[i].header.state >= CS_CONNECTED )
            ++clientCount;
    }
    infostring[0] = 0;
    if ( strlen(sv_mapname->current.string) )
    {
        if ( xblive_wagermatch && xblive_wagermatch->current.enabled && Pregame_GetState() == PREGAME_GAMESTARTED )
            clientCount = sv_maxclients->current.integer;
        v3 = va("%i", 1044);
        Info_SetValueForKey(infostring, (char*)"protocol", v3);
        if ( clientCount )
        {
            v4 = va("%i", clientCount);
            Info_SetValueForKey(infostring, (char *)"clients", v4);
        }
        maxclients = sv_maxclients->current.integer - (Com_GetPrivateClients() - privateClientCount);
        if ( maxclients > 0 )
        {
            v5 = va("%i", maxclients);
            Info_SetValueForKey(infostring, (char *)"sv_maxclients", v5);
        }
        if ( sv_pure->current.enabled || fs_numServerIwds )
            Info_SetValueForKey(infostring, (char *)"pure", "1");
        if ( sv_minPing->current.integer )
        {
            v6 = va("%i", sv_minPing->current.integer);
            Info_SetValueForKey(infostring, (char *)"minPing", v6);
        }
        if ( sv_maxPing->current.integer )
        {
            v7 = va("%i", sv_maxPing->current.integer);
            Info_SetValueForKey(infostring, (char *)"maxPing", v7);
        }
        gamedir = Dvar_GetString("fs_game");
        if ( *gamedir )
            Info_SetValueForKey(infostring, (char *)"game", gamedir);
        if ( sv_allowAnonymous->current.enabled )
        {
            v8 = va("%i", sv_allowAnonymous->current.color[0]);
            Info_SetValueForKey(infostring, (char *)"sv_allowAnonymous", v8);
        }
        if ( sv_disableClientConsole->current.enabled )
        {
            v9 = va("%i", sv_disableClientConsole->current.color[0]);
            Info_SetValueForKey(infostring, (char *)"con_disabled", v9);
        }
        password = Dvar_GetString("g_password");
        if ( password && *password )
            Info_SetValueForKey(infostring, (char *)"pswrd", "1");
        friendlyfire = Dvar_GetInt("scr_team_fftype");
        if ( friendlyfire )
        {
            v10 = va("%i", friendlyfire);
            Info_SetValueForKey(infostring, (char *)"ff", v10);
        }
        killcam = Dvar_GetInt("scr_killcam");
        if ( killcam )
        {
            v11 = va("%i", killcam);
            Info_SetValueForKey(infostring, (char *)"kc", v11);
        }
        hardcore = Dvar_GetInt("scr_hardcore");
        if ( hardcore )
        {
            v12 = va("%i", hardcore);
            Info_SetValueForKey(infostring, (char *)"hc", v12);
        }
        if (IsDedicatedServer())
        {
            Info_SetValueForKey(infostring, (char *)"hw", va("%i", 2));
        }
        else
        {
            Info_SetValueForKey(infostring, (char *)"hw", va("%i", 6));
        }
        if ( !sv_pure->current.enabled || gamedir && *gamedir )
        {
            serverModded = 1;
        }
        else
        {
            refIwdNames = Dvar_GetString("sv_referencedIwdNames");
            if ( *refIwdNames )
            {
                SV_Cmd_TokenizeString(refIwdNames);
                count = SV_Cmd_Argc();
                for ( i = 0; i < count; ++i )
                {
                    v14 = (char *)SV_Cmd_Argv(i);
                    if ( !FS_iwIwd(v14, (char*)"main") )
                    {
                        serverModded = 1;
                        break;
                    }
                }
                SV_Cmd_EndTokenizedString();
            }
        }
        v15 = va("%i", serverModded);
        Info_SetValueForKey(infostring, "mod", v15);
        v16 = va("%i", sv_voice->current.color[0]);
        Info_SetValueForKey(infostring, "voice", v16);
        v17 = va("%i", livesteam_sv_vac->current.color[0]);
        Info_SetValueForKey(infostring, "pb", v17);
        Int = Dvar_GetInt("scr_testclients");
        v19 = va("%i", Int);
        Info_SetValueForKey(infostring, "bots", v19);
        LicenseType = SV_GetLicenseType();
        v21 = va("%i", LicenseType);
        Info_SetValueForKey(infostring, "licensetype", v21);
        if ( xblive_wagermatch && xblive_wagermatch->current.enabled )
            v22 = va("%i", 1);
        else
            v22 = va("%i", 0);
        Info_SetValueForKey(infostring, "wager", v22);
        String = Dvar_GetString("sv_geolocation");
        v24 = va("%s", String);
        Info_SetValueForKey(infostring, "geolocation", v24);
        if (!IsDedicatedServer())
        {
            Info_SetValueForKey(infostring, "countrycode", LiveSteam_GetCountryCode());
        }
        if ( Dvar_GetBool("xblive_wagermatch") )
        {
            v25 = Dvar_GetInt("scr_wagerbet");
            v26 = va("%i", v25);
            Info_SetValueForKey(infostring, "wagerbet", v26);
        }
        if ( Dvar_GetBool("playlist_enabled") )
        {
            v27 = Dvar_GetInt("playlist");
            v28 = va("%i", v27);
            Info_SetValueForKey(infostring, "playlist", v28);
        }
        if ( xblive_basictraining && xblive_basictraining->current.enabled )
            Info_SetValueForKey(infostring, "basic", "1");
        if ( Dvar_GetBool("custom_class_name") )
            Info_SetValueForKey(infostring, "customclass", "1");
        response[0] = quick ? 9 : 3;
        ptr = &response[1];
        *(XNKID *)&response[1] = *secID;
        ptr = &response[9];
        memcpy(
            (unsigned __int8 *)&response[9],
            (unsigned __int8 *)infostring,
            &infostring[strlen(infostring) + 1] - &infostring[1] + 1);
        NET_OutOfBandPrint(NS_SERVER, from, response); // this should work, it's still used in other places in the codebase
    }
}

void __cdecl SV_ConnectionlessPacket(netadr_t from, msg_t *msg)
{
    char *fromAddr; // [esp+0h] [ebp-41Ch]
    client_t *v4; // [esp+4h] [ebp-418h]
    client_t *clients; // [esp+4h] [ebp-418h]
    const char *c; // [esp+8h] [ebp-414h]
    char strBuf[1024]; // [esp+Ch] [ebp-410h] BYREF
    int clientIndex; // [esp+40Ch] [ebp-10h]
    char *s; // [esp+410h] [ebp-Ch]
    int i; // [esp+414h] [ebp-8h]

    clientIndex = -1;
    MSG_BeginReading(msg);
    MSG_ReadLong(msg);
    SV_Netchan_AddOOBProfilePacket(msg->cursize);

    //if (!I_strnicmp((const char *)msg->data + 4, "pb_", 3))
    //{
    //    i = 0;
    //    v4 = svs.clients;
    //    while (i < com_maxclients->current.integer)
    //    {
    //        if (v4->header.state
    //            && NET_CompareBaseAdr(from, v4->header.netchan.remoteAddress)
    //            && v4->header.netchan.remoteAddress.port == from.port)
    //        {
    //            clientIndex = i;
    //            break;
    //        }
    //        ++i;
    //        ++v4;
    //    }
    //    if (msg->data[7] != 67 && msg->data[7] != 49 && msg->data[7] != 74)
    //        PbSvAddEvent(13, clientIndex, msg->cursize - 4, (char *)msg->data + 4);
    //
    //    return;
    //}


    s = MSG_ReadStringLine(msg, strBuf, 0x400u);
    SV_Cmd_TokenizeString(s);
    c = SV_Cmd_Argv(0);

    if ( sv_packet_info->current.enabled )
    {
        Com_Printf(15, "SV packet %s : %s\n", NET_AdrToString(from), c);
    }

    if (!I_stricmp(c, "v"))
    {
        SV_VoicePacket(from, msg);
    }
    //else if (!I_stricmp(c, "getinfo"))
    //{
    //    // removed in blops
    //}
    else if (!I_stricmp(c, "getchallenge"))
    {
        SV_GetChallenge(from);
    }
    else if ( !I_stricmp(c, "connect") )
    {
        // LWSS: Remove punkbuster junk
        //if ( NET_IsLocalAddress(from) )
        //{
        //    PbPassConnectString("localhost", (char *)msg->data);
        //}
        //else
        //{
        //    fromAddr = NET_AdrToString(from);
        //    PbPassConnectString(fromAddr, (char *)msg->data);
        //}
        SV_DirectConnect(from);
    }
    else if (!I_stricmp(c, "steamauth"))
    {
        SV_SteamAuthClient(from, msg);
    }
    else if (!I_stricmp(c, "rcon"))
    {
        NET_DeferPacketToClient(&from, msg);
    }
    //else if (!I_stricmp(c, "disconnect"))
    //{
    //    // removed in blops
    //}
    else if ( !I_stricmp(c, "XX") )
    {
        i = 0;
        clients = svs.clients;
        while ( i < com_maxclients->current.integer )
        {
            if ( clients->header.state
                && NET_CompareBaseAdr(from, clients->header.netchan.remoteAddress)
                && svs.time > clients->lastPacketTime )
            {
                clients->lastPacketTime = svs.time;
            }
            ++i;
            ++clients;
        }
    }

    SV_Cmd_EndTokenizedString();
}

void __cdecl SV_PacketEvent(netadr_t from, msg_t *msg)
{
    client_t *ClientByAddress; // [esp+0h] [ebp-Ch]
    int qport; // [esp+4h] [ebp-8h]

    if ( !Sys_IsServerThread()
        && sv.smp
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_mp.cpp",
                    1836,
                    0,
                    "%s",
                    "Sys_IsServerThread() || !sv.smp") )
    {
        __debugbreak();
    }
    if ( msg->cursize >= 4 && *(unsigned int *)msg->data == -1 )
    {
        SV_ConnectionlessPacket(from, msg);
    }
    else
    {
        SV_ResetSkeletonCache();
        MSG_BeginReading(msg);
        MSG_ReadLong(msg);
        qport = MSG_ReadShort(msg);
        ClientByAddress = SV_FindClientByAddress(from, qport);
        if ( ClientByAddress )
        {
            if ( Netchan_Process(&ClientByAddress->header.netchan, msg) )
            {
                ClientByAddress->serverId = MSG_ReadByte(msg);
                ClientByAddress->messageAcknowledge = MSG_ReadLong(msg);
                if ( ClientByAddress->messageAcknowledge >= 0 )
                {
                    ClientByAddress->reliableAcknowledge = MSG_ReadLong(msg);
                    if ( ClientByAddress->reliableSequence - ClientByAddress->reliableAcknowledge < 128 )
                    {
                        if ( ClientByAddress->header.state != CS_ZOMBIE )
                        {
                            iassert(bgs == 0);
                            ClientByAddress->lastPacketTime = svs.time;
                            SV_ExecuteClientMessage(ClientByAddress, msg);
                        }
                    }
                    else
                    {
                        Com_Printf(
                            15,
                            "Out of range reliableAcknowledge message from %s - cl->reliableSequence is %i, reliableAcknowledge is %i\n",
                            ClientByAddress->name,
                            ClientByAddress->reliableSequence,
                            ClientByAddress->reliableAcknowledge);
                        ClientByAddress->reliableAcknowledge = ClientByAddress->reliableSequence;
                    }
                }
                else
                {
                    Com_Printf(
                        15,
                        "Invalid reliableAcknowledge message from %s - reliableAcknowledge is %i\n",
                        ClientByAddress->name,
                        ClientByAddress->reliableAcknowledge);
                }
            }
        }
        else
        {
            NET_OutOfBandPrint(NS_SERVER, from, "disconnect");
        }
    }
}

void __cdecl SV_CalcPings()
{
    int j; // [esp+4h] [ebp-18h]
    client_t *v1; // [esp+8h] [ebp-14h]
    int total; // [esp+10h] [ebp-Ch]
    int i; // [esp+14h] [ebp-8h]
    int count; // [esp+18h] [ebp-4h]

    for ( i = 0; i < com_maxclients->current.integer; ++i )
    {
        v1 = &svs.clients[i];
        if ( v1->header.state == CS_ACTIVE )
        {
            if ( v1->gentity )
            {
                total = 0;
                count = 0;
                for ( j = 0; j < 32; ++j )
                {
                    if ( v1->frames[j].messageAcked > 0 )
                    {
                        ++count;
                        total += v1->frames[j].messageAcked - v1->frames[j].messageSent;
                    }
                }
                if ( count )
                {
                    v1->ping = total / count;
                    if ( v1->ping > 999 )
                    {
                        if ( sv_showPingSpam->current.integer )
                            Com_DPrintf(15, "Giving %s a 999 ping - >999 calculated ping:\n", v1->name);
                        v1->ping = 999;
                    }
                }
                else if ( v1->header.netchan.remoteAddress.type )
                {
                    if ( sv_showPingSpam->current.integer )
                        Com_DPrintf(15, "Giving %s a 999 ping - !count:\n", v1->name);
                    v1->ping = 999;
                }
                else
                {
                    v1->ping = 0;
                }
            }
            else
            {
                if ( sv_showPingSpam->current.integer )
                    Com_DPrintf(15, "Giving %s a 999 ping - not a gentity\n", v1->name);
                v1->ping = 999;
            }
        }
        else
        {
            v1->ping = 999;
        }
    }
}

void __cdecl SV_FreeClientScriptId(client_t *cl)
{
    if ( (unsigned int)(cl - svs.clients) >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_mp.cpp",
                    2060,
                    0,
                    "cl - svs.clients doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    cl - svs.clients,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    Com_Printf(15, "SV_FreeClientScriptId: %d, %d -> 0\n", cl - svs.clients, cl->scriptId);
    if ( !cl->scriptId
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_mp.cpp", 2066, 0, "%s", "cl->scriptId") )
    {
        __debugbreak();
    }
    Scr_FreeValue(SCRIPTINSTANCE_SERVER, cl->scriptId);
    cl->scriptId = 0;
}

void __cdecl SV_CheckTimeouts()
{
    client_t *drop; // [esp+Ch] [ebp-18h]
    int connectdroppoint; // [esp+10h] [ebp-14h]
    int zombiepoint; // [esp+18h] [ebp-Ch]
    int droppoint; // [esp+1Ch] [ebp-8h]
    int clientNum; // [esp+20h] [ebp-4h]

    PROF_SCOPED("SV_CheckTimeouts");

    droppoint = svs.time - 1000 * sv_timeout->current.integer;
    connectdroppoint = svs.time - 1000 * sv_connectTimeout->current.integer;
    zombiepoint = svs.time - 1000 * sv_zombietime->current.integer;
    clientNum = 0;
    drop = svs.clients;
    while ( clientNum < com_maxclients->current.integer )
    {
        if ( drop->lastPacketTime > svs.time )
            drop->lastPacketTime = svs.time;
        if ( drop->bIsTestClient || drop->bIsDemoClient )
        {
            if ( drop->header.state == CS_ZOMBIE )
            {
                if ( drop->bIsDemoClient )
                    Com_Printf(0, "Going zombie --> free for client %s(%s)\n", drop->name, "demo");
                else
                    Com_Printf(0, "Going zombie --> free for client %s(%s)\n", drop->name, "test");
                drop->header.state = CS_FREE;
                drop->lastPacketTime = 0;
            }
        }
        else if ( drop->header.state == CS_ZOMBIE && drop->lastPacketTime < zombiepoint )
        {
            Com_Printf(15, "Going from CS_ZOMBIE to CS_FREE for client #%i\n", clientNum);
            drop->header.state = CS_FREE;
            drop->lastPacketTime = 0;
        }
        else if ( drop->header.state == CS_ACTIVE && drop->lastPacketTime < droppoint )
        {
            if ( ++drop->timeoutCount > 5 )
            {
                Com_Printf(15, "Timing out ACTIVE client #%i(%s)\n", clientNum, drop->name);
                SV_DropClient(drop, "EXE_TIMEDOUT", 1, 1);
            }
        }
        else if ( drop->header.state <= CS_ZOMBIE || drop->lastPacketTime >= connectdroppoint )
        {
            drop->timeoutCount = 0;
        }
        else if ( ++drop->timeoutCount > 5 )
        {
            Com_Printf(15, "Timing out client #%i(%s), state was %i\n", clientNum, drop->name, drop->header.state);
            SV_DropClient(drop, "EXE_TIMEDOUT", 1, 1);
        }
        ++clientNum;
        ++drop;
    }
}

int __cdecl SV_CheckPaused()
{
    client_t *clients; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    int count; // [esp+8h] [ebp-4h]

    if ( !cl_paused->current.integer )
        return 0;
    count = 0;
    i = 0;
    clients = svs.clients;
    while ( i < com_maxclients->current.integer )
    {
        if ( clients->header.state >= CS_CONNECTED )
            ++count;
        ++i;
        ++clients;
    }
    if ( count <= 1 )
    {
        Dvar_SetInt((dvar_s *)sv_paused, 1);
        return 1;
    }
    else
    {
        Dvar_SetInt((dvar_s *)sv_paused, 0);
        return 0;
    }
}

void    SV_RunFrame()
{
    unsigned int v1; // eax
    unsigned int start; // [esp+0h] [ebp-4h]

    start = Sys_Milliseconds();
    SV_ResetSkeletonCache();
    CL_FlushDebugServerData();
    G_RunFrame(svs.time);
    sv.physicsTime = Phys_GetCurrentTime();
    CL_UpdateDebugServerData();
    v1 = Sys_Milliseconds();
    SV_UpdatePerformanceFrame(v1 - start);
}

int serverPreviousFrameTimes[10];
int serverDebugFrame;
void __cdecl SV_UpdatePerformanceFrame(int time)
{
    int total; // [esp+0h] [ebp-10h]
    volatile int maxTime; // [esp+4h] [ebp-Ch]
    volatile int minTime; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    minTime = 0x7FFFFFFF;
    maxTime = 0;
    serverPreviousFrameTimes[serverDebugFrame % 10] = time;
    if (++serverDebugFrame >= 10)
    {
        total = 0;
        for (i = 0; i < 10; ++i)
        {
            total += serverPreviousFrameTimes[i];
            if (minTime > serverPreviousFrameTimes[i])
                minTime = serverPreviousFrameTimes[i];
            if (maxTime < serverPreviousFrameTimes[i])
                maxTime = serverPreviousFrameTimes[i];
        }
        sv.serverFrameTime = (int)(float)((float)total / 10.0);
        if (minTime <= 0)
            minTime = 1;
        sv.serverFrameTimeMin = minTime;
        sv.serverFrameTimeMax = maxTime;
    }
}

void __cdecl SV_UpdateBots()
{
    int j; // [esp+4h] [ebp-44h]
    client_t *bot; // [esp+8h] [ebp-40h]
    usercmd_s botcmd; // [esp+Ch] [ebp-3Ch] BYREF
    int i; // [esp+44h] [ebp-4h]

    for ( j = 0; j < 2; ++j )
        botcmd.button_bits.array[j] = 0;
    SV_ResetSkeletonCache();
    i = 0;
    bot = svs.clients;
    while ( i < com_maxclients->current.integer )
    {
        if ( bot->header.state != CS_FREE && bot->header.state != CS_RECONNECTING && bot->bIsTestClient && !bot->bIsDemoClient )
        {
            SV_BotThink(bot, &botcmd);
            SV_ClientThink(bot, &botcmd);
        }
        ++i;
        ++bot;
    }
}

void __cdecl SV_InitThreadVariables()
{
    //*(unsigned int *)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 8) = 0;
    bgs = 0;
    CL_DebugInitSVThreadVariables();
}

void __cdecl SV_InitServerThread()
{
    if ( !Sys_SpawnServerThread((void (__cdecl *)(unsigned int))SV_ServerThread) )
        Sys_Error((char*)"Failed to create server thread");
}

int g_startServer;
int g_checkServerTime;
void     SV_ServerThread(unsigned int threadContext)
{
    void *Value; // eax
    LARGE_INTEGER v3; // [esp+8h] [ebp-38h] BYREF
    unsigned intv4; // [esp+10h] [ebp-30h]
    int v5; // [esp+14h] [ebp-2Ch]
    __int64 v6; // [esp+18h] [ebp-28h]
    LARGE_INTEGER PerformanceCount; // [esp+20h] [ebp-20h] BYREF
    unsigned int CurrentThreadId; // [esp+28h] [ebp-18h]
    int v9; // [esp+2Ch] [ebp-14h]
    unsigned __int64 runStart; // [esp+30h] [ebp-10h]
    int timeout; // [esp+38h] [ebp-8h]
    int start; // [esp+3Ch] [ebp-4h]

    if ( threadContext != 10
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_mp.cpp",
                    2528,
                    0,
                    "threadContext == THREAD_CONTEXT_SERVER\n\t%i, %i",
                    threadContext,
                    10) )
    {
        __debugbreak();
    }

    do
        Value = Sys_GetValue(2);
    while ( _setjmp(*(jmp_buf *)Value) );
    //while ( _setjmp3(Value, 0) );

    SV_InitThreadVariables();
    Sys_InitServerEvents();
    SV_ClearServerThreadOwnsGame();
    Sys_ServerCompleted();
    while ( 1 )
    {
        //BLOPS_NULLSUB();
        timeout = 1000 / sv_network_fps->current.integer;
        {
            PROF_SCOPED("wait start server");
            g_checkServerTime = timeout + Sys_Milliseconds();
            if (sv_assistWorkers->current.enabled)
            {
                start = Sys_WaitStartServer(0);
                g_startServer = start;
                while (!start && (int)Sys_Milliseconds() < g_checkServerTime)
                {
                    Sys_AssistSingle();
                    start = Sys_WaitStartServer(0);
                    g_startServer = start;
                }
            }
            else
            {
                start = Sys_WaitStartServer(timeout);
                g_startServer = start;
            }
            CurrentThreadId = GetCurrentThreadId();
            v9 = 0;
        }

        G_ClearVehicleInputs();
        SV_RunEventLoop();
        if ( Sys_IsServerThread() )
            TaskManager2_PickUpDeferredTasks();
        if ( start )
        {
            SV_IncServerThreadOwnsGame();
            SV_CalcPings();

            {
                PROF_SCOPED("SERVER: Frame");
                QueryPerformanceCounter(&PerformanceCount);
                runStart = PerformanceCount.QuadPart;
                SV_PreFrame();
                {
                    PROF_SCOPED("SERVER: run frame");
                    SV_RunFrame();
                }
                {
                    PROF_SCOPED("SERVER: post frame");
                    SV_PostFrame();
                }

                QueryPerformanceCounter(&v3);
                gRunFrameTicks = v3.QuadPart - runStart;
            }

            SV_RunEventLoop();
            Sys_SleepServer();
            SV_DecServerThreadOwnsGame();
        }
        gjk_collision_epilog(1);
        Sys_ServerCompleted();
    }
}

void __cdecl SV_PostFrame()
{
    Scr_UpdateDebugger(SCRIPTINSTANCE_SERVER);
    SV_CheckTimeouts();
    SV_SendClientMessages();

    // LWSS ADD: Steam Periodic Auth Check
    if (IsDedicatedServer())
    {
        Steam_CheckClients();
    }
    // LWSS END
}

void SV_RunEventLoop()
{
    if ( !Sys_IsServerThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_mp.cpp",
                    2461,
                    0,
                    "%s",
                    "Sys_IsServerThread()") )
    {
        __debugbreak();
    }
    Sys_WaitAllowServerNetworkLoop();
    Sys_ResetServerNetworkCompletedEvent();
    if ( sv.allowNetPackets )
    {
        PROF_SCOPED("SERVER: msg recv");
        SV_IncServerThreadOwnsGame();
        Com_ServerPacketEvent();
        SV_DecServerThreadOwnsGame();
    }
    Sys_SetServerNetworkCompletedEvent();
}

void __cdecl SV_WaitServer()
{
    bool i; // eax
    unsigned int timeout; // [esp+8h] [ebp-4h]
    unsigned int timeouta; // [esp+8h] [ebp-4h]

    iassert(Sys_IsMainThread());

    if ( com_inServerFrame )
    {
        com_inServerFrame = 0;

        iassert(com_sv_running);
        iassert(com_sv_running->current.enabled);

        SV_AllowPackets(0);

        {
            PROF_SCOPED("wait server");
            timeout = !Sys_AssistNeeded();
            for (i = Sys_WaitServer(timeout); !i; i = Sys_WaitServer(timeouta))
            {
                Com_CheckSyncFrame();
                Sys_AssistSingle();
                timeouta = !Sys_AssistNeeded();
            }
        }
    }
}

void __cdecl SV_InitSnapshot()
{
    if ( com_inServerFrame
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_mp.cpp",
                    2766,
                    0,
                    "%s",
                    "!com_inServerFrame") )
    {
        __debugbreak();
    }
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_mp.cpp",
                    2768,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    Sys_InitServerEvents();
    sv.smp = sv_smp->current.enabled;
    sv.state = SS_GAME;
    sv.inFrame = 0;
}

void __cdecl SV_KillLocalServer()
{
    if ( com_sv_running->current.enabled )
        g_shouldKillLocalServer = 1;
}

void __cdecl SV_PreFrame()
{
    char *v0; // eax

    {
        PROF_SCOPED("SERVER: bots");
        SV_UpdateBots();
    }
    
    if ( (dvar_modifiedFlags & 8) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_mp.cpp",
                    2903,
                    0,
                    "%s",
                    "!(dvar_modifiedFlags & DVAR_SYSTEMINFO)") )
    {
        __debugbreak();
    }
    if ( (dvar_modifiedFlags & 4) != 0 )
    {
        v0 = Dvar_InfoString(0, 4);
        SV_SetConfigstring(0, v0);
        dvar_modifiedFlags &= ~4u;
    }
    if ( (dvar_modifiedFlags & 0x100) != 0 )
    {
        SV_SetConfig(23, 150, 256);
        dvar_modifiedFlags &= ~0x100u;
    }
}

int __cdecl SV_Frame(int controllerIndex, int msec)
{
    Hunk_CheckTempMemoryClear();
    Hunk_CheckTempMemoryHighClear();
    if ( g_shouldKillLocalServer )
    {
        if ( g_shutdownMsg && *g_shutdownMsg )
            Com_Shutdown(g_shutdownMsg);
        else
            Com_Shutdown("EXE_SERVERKILLED");
        g_shouldKillLocalServer = 0;
        g_shutdownMsg = 0;
        return msec;
    }
    else if ( com_sv_running->current.enabled )
    {
        if ( !sv_pure->current.enabled && fs_numServerIwds )
            FS_ShutdownServerIwdNames();
        if ( SV_CheckPaused() )
            SV_WaitServer();
        else
            SV_FrameInternal(controllerIndex, msec);
        return msec;
    }
    else
    {
        return msec;
    }
}

void __cdecl SV_FrameInternal(int controllerIndex, int msec)
{
    int frameMsec; // [esp+0h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_mp.cpp",
                    2928,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( msec < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_mp.cpp", 2929, 0, "%s", "msec >= 0") )
    {
        __debugbreak();
    }
    frameMsec = 1000 / sv_fps->current.integer;
    if ( sv.timeResidual >= frameMsec
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_mp.cpp",
                    2934,
                    0,
                    "%s",
                    "sv.timeResidual < frameMsec") )
    {
        __debugbreak();
    }
    sv.timeResidual += msec;
    if ( sv.timeResidual >= frameMsec && !SV_CheckOverflow() )
    {
        if ( sv.smp )
            SV_WakeServer();
        do
        {
            if ( sv.smp )
            {
                if ( !com_inServerFrame
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_mp.cpp",
                                2984,
                                0,
                                "%s",
                                "com_inServerFrame") )
                {
                    __debugbreak();
                }
                SV_WaitServer();
                sv.timeResidual -= frameMsec;
                svs.time += frameMsec;
                Com_CheckError();
                SV_WakeServer();
            }
            else
            {
                sv.timeResidual -= frameMsec;
                svs.time += frameMsec;
                SV_WakeServer();
                SV_WaitServer();
                Com_CheckError();
            }
            Scr_SetLoading(0, SCRIPTINSTANCE_SERVER);
        }
        while ( sv.timeResidual >= frameMsec );
    }
}

void SV_WakeServer()
{
    if ( !com_inServerFrame )
    {
        com_inServerFrame = 1;
        Sys_WakeServer();
        SV_AllowPackets(1);
    }
}

char __cdecl SV_CheckOverflow()
{
    const char *v0; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    char mapname[68]; // [esp+0h] [ebp-48h] BYREF

    if ( svs.time <= 1879048192 )
    {
        if ( svs.nextSnapshotEntities < 2147483646 - svs.numSnapshotEntities )
        {
            if ( svs.nextCachedSnapshotEntities < 2147483646 - svs.numCachedSnapshotEntities )
            {
                if ( svs.nextCachedSnapshotClients < 2147483646 - svs.numCachedSnapshotClients )
                {
                    if ( svs.nextArchivedSnapshotFrames < 2147482446 )
                    {
                        if ( svs.nextArchivedSnapshotBuffer < 2130706430 )
                        {
                            if ( svs.nextCachedSnapshotFrames < 2147483134 )
                            {
                                if ( svs.nextSnapshotClients < 2147483646 - svs.numSnapshotClients )
                                {
                                    return 0;
                                }
                                else
                                {
                                    I_strncpyz(mapname, sv_mapname->current.string, 64);
                                    Com_Shutdown("EXE_SERVERRESTARTMISC numSnapshotClients");
                                    v8 = va("map %s\n", mapname);
                                    Cbuf_AddText(0, v8);
                                    return 1;
                                }
                            }
                            else
                            {
                                I_strncpyz(mapname, sv_mapname->current.string, 64);
                                Com_Shutdown("EXE_SERVERRESTARTMISC nextCachedSnapshotFrames");
                                v7 = va("map %s\n", mapname);
                                Cbuf_AddText(0, v7);
                                return 1;
                            }
                        }
                        else
                        {
                            I_strncpyz(mapname, sv_mapname->current.string, 64);
                            Com_Shutdown("EXE_SERVERRESTARTMISC nextArchivedSnapshotBuffer");
                            v6 = va("map %s\n", mapname);
                            Cbuf_AddText(0, v6);
                            return 1;
                        }
                    }
                    else
                    {
                        I_strncpyz(mapname, sv_mapname->current.string, 64);
                        Com_Shutdown("EXE_SERVERRESTARTMISC nextArchivedSnapshotFrames");
                        v5 = va("map %s\n", mapname);
                        Cbuf_AddText(0, v5);
                        return 1;
                    }
                }
                else
                {
                    I_strncpyz(mapname, sv_mapname->current.string, 64);
                    Com_Shutdown("EXE_SERVERRESTARTMISC nextCachedSnapshotClients");
                    v4 = va("map %s\n", mapname);
                    Cbuf_AddText(0, v4);
                    return 1;
                }
            }
            else
            {
                I_strncpyz(mapname, sv_mapname->current.string, 64);
                Com_Shutdown("EXE_SERVERRESTARTMISC nextChachedSnapshotEntities");
                v3 = va("map %s\n", mapname);
                Cbuf_AddText(0, v3);
                return 1;
            }
        }
        else
        {
            I_strncpyz(mapname, sv_mapname->current.string, 64);
            Com_Shutdown("EXE_SERVERRESTARTMISC numSnapshotEntities");
            v2 = va("map %s\n", mapname);
            Cbuf_AddText(0, v2);
            return 1;
        }
    }
    else
    {
        I_strncpyz(mapname, sv_mapname->current.string, 64);
        Com_Shutdown("EXE_SERVERRESTARTTIMEWRAP");
        v0 = va("map %s\n", mapname);
        Cbuf_AddText(0, v0);
        return 1;
    }
}

void __cdecl SV_AllowPackets(bool allow)
{
    iassert(Sys_IsMainThread());

    PROF_SCOPED("SV_AllowPackets");

    Sys_WaitServerNetworkCompleted();
    Sys_ResetServerAllowNetworkEvent();
    sv.allowNetPackets = allow;
    Sys_SetServerAllowNetworkEvent();
}

