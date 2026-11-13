#include <server_mp/sv_main_mp.h>

#include "sv_live_stats.h"
#include <qcommon/common.h>

int __cdecl SV_GetClientNumForOnlineUserID(unsigned __int64 uid)
{
    int i; // [esp+8h] [ebp-8h]
    int retval; // [esp+Ch] [ebp-4h]

    retval = -1;
    for ( i = 0; i < com_maxclients->current.integer; ++i )
    {
        if ( svs.clients[i].header.state > CS_ZOMBIE && svs.clients[i].onlineUserID == uid )
            return i;
    }
    return retval;
}

unsigned __int64 __cdecl SV_GetBdUidFromFileInfo(_QWORD *data)
{
    return data[34];
}

client_t *__cdecl SV_GetClientFromFileInfo(_QWORD *data)
{
    unsigned __int64 uid; // [esp+0h] [ebp-18h]
    int clientNum; // [esp+Ch] [ebp-Ch]
    client_t *client; // [esp+10h] [ebp-8h]

    client = 0;
    if ( data )
    {
        uid = data[34];
        Com_DPrintf(15, "Read uid %llu, finding client\n", uid);
        clientNum = SV_GetClientNumForOnlineUserID(uid);
        if ( clientNum < 0 )
        {
            Com_DPrintf(15, "Warning: Couldn't find matching client for onlineUserID %llu\n", uid);
        }
        else
        {
            if ( (unsigned int)clientNum >= com_maxclients->current.integer
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_live_stats.cpp",
                            49,
                            0,
                            "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                            clientNum,
                            com_maxclients->current.integer) )
            {
                __debugbreak();
            }
            return &svs.clients[clientNum];
        }
    }
    return client;
}

unsigned __int64 __cdecl SV_GetPlayerXuid(unsigned int clientNum)
{
    if ( clientNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_live_stats.cpp",
                    64,
                    0,
                    "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    clientNum,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    return svs.clients[clientNum].onlineUserID;
}

