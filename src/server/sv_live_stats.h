#pragma once

int __cdecl SV_GetClientNumForOnlineUserID(unsigned __int64 uid);
unsigned __int64 __cdecl SV_GetBdUidFromFileInfo(_QWORD *data);
client_t *__cdecl SV_GetClientFromFileInfo(_QWORD *data);
unsigned __int64 __cdecl SV_GetPlayerXuid(unsigned int clientNum);
