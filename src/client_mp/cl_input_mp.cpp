#include "cl_input_mp.h"
#include "cl_main_mp.h"
#include <client/client.h>
#include <devgui/devgui.h>
#include <devgui/devgui_input.h>
#include <cgame_mp/cg_newDraw_mp.h>
#include <win32/win_input.h>
#include <universal/mem_largelocal.h>
#include <cgame_mp/cg_main_mp.h>
#include <demo/demo_playback.h>
#include <win32/win_shared.h>
#include <universal/com_shared.h>
#include "cl_parse_mp.h"
#include "cl_net_chan_mp.h"
#include <win32/win_net.h>
#include <client/cl_keys.h>
#include <qcommon/com_clients.h>
#include <win32/win_gamepad.h>
#include "cl_cgame_mp.h"
#include <aim_assist/aim_assist.h>
#include <cgame/cg_gamepad.h>
#include <monkey/monkey.h>
#include <universal/com_math_anglevectors.h>
#include <cgame/cg_main.h>

kbutton_t playersKb[1][47];
bool gAttackEdgeDetected[1];
bool latch[1];

const dvar_t *cl_analog_attack_threshold;
const dvar_t *cl_stanceHoldTime;
const dvar_t *cl_dtpHoldTime;
const dvar_t *cl_nodelta;
const dvar_t *cl_inputTimeScaleFrac;
const dvar_t *cl_dblTapMaxHoldTime;
const dvar_t *cl_dblTapMaxDelayTime;


void __cdecl CL_SetStance(int localClientNum, StanceState stance)
{
    if ( !playersKb[localClientNum][27].active && !playersKb[localClientNum][11].active )
        CL_GetLocalClientGlobals(localClientNum)->stance = stance;
}

void __cdecl IN_UpdateUseCount()
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-8h]

    LocalClientGlobals = CL_GetLocalClientGlobals(0);
    if ( CL_GetLocalClientConnectionState(0) >= 6 )
    {
        if ( com_frameTime - LocalClientGlobals->useTime >= cl_dblTapMaxHoldTime->current.integer )
            LocalClientGlobals->useCount = 0;
        else
            ++LocalClientGlobals->useCount;
        LocalClientGlobals->useHeld = 0;
    }
}

void __cdecl IN_UpdateUseHeld()
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-8h]

    LocalClientGlobals = CL_GetLocalClientGlobals(0);
    if ( CL_GetLocalClientConnectionState(0) >= 6 && !LocalClientGlobals->useHeld )
    {
        if ( com_frameTime - LocalClientGlobals->useTime >= cl_dblTapMaxDelayTime->current.integer )
            LocalClientGlobals->useCount = 0;
        LocalClientGlobals->useHeld = 1;
        LocalClientGlobals->useTime = com_frameTime;
    }
}

void __cdecl IN_CenterView()
{
    clientActive_t *LocalClientGlobals; // edx

    if ( CL_GetLocalClientConnectionState(0) >= 6 )
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(0);
        LocalClientGlobals->viewangles[0] = -LocalClientGlobals->snap.ps.delta_angles[0];
    }
}

bool __cdecl IN_IsTalkKeyHeld()
{
    return cl_talking->current.enabled || playersKb[0][46].active;
}

int __cdecl CL_MouseEvent(int x, int y, int dx, int dy)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-8h]
    clientUIActive_t *clUI; // [esp+4h] [ebp-4h]

    if ( DevGui_IsActive() )
    {
        DevGui_MouseEvent(dx, dy);
        return 1;
    }
    else
    {
        clUI = CL_GetLocalClientUIGlobals(0);
        LocalClientGlobals = CL_GetLocalClientGlobals(0);
        if ( LocalClientGlobals )
        {
            if ( (clUI->keyCatchers & 0x10) == 0 || UI_KeysBypassMenu(0) )
            {
                CL_ShowSystemCursor(0);
                LocalClientGlobals->mouseDx[LocalClientGlobals->mouseIndex] += dx;
                LocalClientGlobals->mouseDy[LocalClientGlobals->mouseIndex] += dy;
                return 1;
            }
            else
            {
                UI_MouseEvent(0, x, y);
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
}

void __cdecl CL_SetCursorPos(unsigned int x, unsigned int y)
{
    IN_SetCursorPos(x, y);
}

void __cdecl CL_ShowSystemCursor(bool show)
{
    IN_ShowSystemCursor(show);
}

bool __cdecl CL_KeyActiveOrPressed(int localClientNum, unsigned int kbButton)
{
    if ( kbButton >= 0x2F
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_input_mp.cpp",
                    2284,
                    0,
                    "kbButton doesn't index NUM_BUTTONS\n\t%i not in [0, %i)",
                    kbButton,
                    47) )
    {
        __debugbreak();
    }
    return playersKb[localClientNum][kbButton].active || playersKb[localClientNum][kbButton].wasPressed;
}

void __cdecl CL_UpdateCmdButton(
                int localClientNum,
                bitarray<51> *cmdButton_bits,
                int kbButton,
                int buttonBit,
                int numBits)
{
    int i; // [esp+0h] [ebp-10h]
    int val; // [esp+4h] [ebp-Ch]
    kbutton_t *kb; // [esp+Ch] [ebp-4h]

    kb = playersKb[localClientNum];
    if ( buttonBit >= 0 && (kb[kbButton].active || kb[kbButton].wasPressed) )
    {
        if ( numBits == 1 )
        {
            cmdButton_bits->setBit(buttonBit);
        }
        else
        {
            val = (int)(float)((float)((1 << numBits) - 1) * kb[kbButton].val);
            for ( i = 0; i < numBits; ++i )
            {
                if ((val & (1 << i)) != 0)
                {
                    cmdButton_bits->setBit(i + buttonBit);
                }
            }
        }
    }
    kb[kbButton].wasPressed = 0;
}

void __cdecl CL_WritePacket(int localClientNum)
{
    int v1; // eax
    int v2; // eax
    unsigned __int8 *v3; // ecx
    unsigned __int8 *v4; // edx
    int v6; // [esp-Ch] [ebp-918h]
    char *v7; // [esp+8h] [ebp-904h]
    const char *v8; // [esp+Ch] [ebp-900h]
    int j; // [esp+5Ch] [ebp-8B0h]
    bool forceAngles; // [esp+63h] [ebp-8A9h]
    usercmd_s nullcmd; // [esp+68h] [ebp-8A4h] BYREF
    clientActive_t *cl; // [esp+9Ch] [ebp-870h]
    LargeLocal compressedBuf_large_local(2048); // [esp+A0h] [ebp-86Ch] BYREF
    usercmd_s *oldcmd; // [esp+A8h] [ebp-864h]
    int compressedSize; // [esp+ACh] [ebp-860h]
    connstate_t connstate; // [esp+B0h] [ebp-85Ch]
    unsigned __int8 data[2052]; // [esp+B4h] [ebp-858h] BYREF
    unsigned __int8 (*compressedBuf)[2048]; // [esp+8BCh] [ebp-50h]
    msg_t buf; // [esp+8C0h] [ebp-4Ch] BYREF
    int oldPacketNum; // [esp+8F0h] [ebp-1Ch]
    clientConnection_t *clc; // [esp+8F4h] [ebp-18h]
    int key; // [esp+8F8h] [ebp-14h]
    int i; // [esp+8FCh] [ebp-10h]
    usercmd_s *cmd; // [esp+900h] [ebp-Ch]
    int count; // [esp+904h] [ebp-8h]
    int packetNum; // [esp+908h] [ebp-4h]

    //for ( j = 0; j < 2; ++j )
    //    nullcmd.button_bits.array[j] = 0;

    //LargeLocal::LargeLocal(&compressedBuf_large_local, 2048);
    //compressedBuf = (unsigned __int8 (*)[2048])LargeLocal::GetBuf(&compressedBuf_large_local);
    compressedBuf = (unsigned __int8 (*)[2048])compressedBuf_large_local.GetBuf();
    clc = CL_GetLocalClientConnection(localClientNum);
    connstate = CL_GetLocalClientConnectionState(localClientNum);
    if ( clc->demoplaying
        || Demo_IsPlaying()
        || Demo_IsShutdownInProgress()
        || connstate == CA_CINEMATIC
        || connstate == CA_LOGO )
    {
        return;
        //LargeLocal::~LargeLocal(&compressedBuf_large_local);
    }

    cl = CL_GetLocalClientGlobals(localClientNum);
    MSG_SetDefaultUserCmd(&cl->snap.ps, &nullcmd);
    oldcmd = &nullcmd;
    cmd = &nullcmd;
    memset(&buf, 0, sizeof(buf));
    MSG_Init(&buf, data, 2048);
    MSG_WriteByte(&buf, cl->serverId);
    MSG_WriteLong(&buf, clc->serverMessageSequence);
    MSG_WriteLong(&buf, clc->serverCommandSequence);
    for ( i = clc->reliableAcknowledge + 1; i <= clc->reliableSequence; ++i )
    {
        MSG_WriteBits(&buf, 2, 3u);
        MSG_WriteLong(&buf, i);
        MSG_WriteString(&buf, clc->reliableCommands[i & 0x7F]);
    }
    oldPacketNum = (clc->netchan.outgoingSequence - 1 - cl_packetdup->current.integer) & 0x1F;
    count = cl->cmdNumber - cl->outPackets[oldPacketNum].p_cmdNumber;
    if ( count > 32 )
    {
        count = 32;
        Com_Printf(14, "MAX_PACKET_USERCMDS\n");
    }
    if ( count >= 1 )
    {
        if ( cl_showSend->current.enabled )
            Com_Printf(14, "(%i)", count);
        if ( clc->demowaiting || clc->demoRequestUncompressedPacket )
        {
            clc->demoRequestUncompressedPacketTime = Sys_Milliseconds();
            MSG_WriteBits(&buf, 1, 3u);
        }
        else if ( !cl_nodelta->current.enabled
                        && cl->snap.valid
                        && clc->serverMessageSequence == cl->snap.messageNum )
        {
            MSG_WriteBits(&buf, 0, 3u);
        }
        else
        {
            MSG_WriteBits(&buf, 1, 3u);
        }
        MSG_WriteByte(&buf, count);
        key = clc->checksumFeed;
        key ^= clc->serverMessageSequence;
        v1 = Com_HashKey(clc->serverCommands[clc->serverCommandSequence & 0x7F], 32);
        key ^= v1;
        if ( cl_debugMessageKey->current.enabled )
        {
            v7 = &clc->serverCommands[clc->serverCommandSequence & 0x7F][1];
            v8 = clc->serverCommands[clc->serverCommandSequence & 0x7F];
            v6 = clc->serverCommandSequence & 0x7F;
            v2 = Com_HashKey(v8, 32);
            Com_Printf(
                14,
                "key:%i, checksumFeed:%i, messageAcknowledge:%i, Com_HashKey:%i, servercommand(%i):'%s', len:%i\n",
                key,
                clc->checksumFeed,
                clc->serverMessageSequence,
                v2,
                v6,
                v8,
                &v8[strlen(v8) + 1] - v7);
        }
        forceAngles = CG_IsRemoteGuidingMissile(localClientNum);
        for ( i = 0; i < count; ++i )
        {
            cmd = &cl->cmds[((unsigned __int8)cl->cmdNumber - (_BYTE)count + (_BYTE)i + 1) & 0x7F];
            MSG_WriteDeltaUsercmdKey(&buf, key, oldcmd, cmd, forceAngles);
            oldcmd = cmd;
        }
        MSG_WriteLong(&buf, LODWORD(cl->cgameOrigin[0]));
        MSG_WriteLong(&buf, LODWORD(cl->cgameOrigin[1]));
        MSG_WriteLong(&buf, LODWORD(cl->cgameOrigin[2]));

        nanassertvec3(cl->cgameVelocity);

        MSG_WriteLong(&buf, cl->cgamePredictedDataServerTime);
        CL_SavePredictedOriginForServerTime(
            cl,
            cl->cgamePredictedDataServerTime,
            cl->cgameOrigin,
            cl->cgameVelocity,
            cl->cgameViewangles,
            cl->cgameBobCycle,
            cl->cgameMovementDir);
        if ( cl->cgameVehicle.inVehicle )
        {
            MSG_WriteBit1(&buf);
            MSG_WriteLong(&buf, LODWORD(cl->cgameVehicle.origin[0]));
            MSG_WriteLong(&buf, LODWORD(cl->cgameVehicle.origin[1]));
            MSG_WriteLong(&buf, LODWORD(cl->cgameVehicle.origin[2]));
            MSG_WriteLong(&buf, LODWORD(cl->cgameVehicle.angles[0]));
            MSG_WriteLong(&buf, LODWORD(cl->cgameVehicle.angles[1]));
            MSG_WriteLong(&buf, LODWORD(cl->cgameVehicle.angles[2]));
            MSG_WriteLong(&buf, LODWORD(cl->cgameVehicle.tVel[0]));
            MSG_WriteLong(&buf, LODWORD(cl->cgameVehicle.tVel[1]));
            MSG_WriteLong(&buf, LODWORD(cl->cgameVehicle.tVel[2]));
            MSG_WriteLong(&buf, LODWORD(cl->cgameVehicle.aVel[0]));
            MSG_WriteLong(&buf, LODWORD(cl->cgameVehicle.aVel[1]));
            MSG_WriteLong(&buf, LODWORD(cl->cgameVehicle.aVel[2]));
        }
        else
        {
            MSG_WriteBit0(&buf);
        }
    }
    MSG_WriteBits(&buf, 3, 3u);
    if ( buf.cursize < 9
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_input_mp.cpp",
                    3004,
                    0,
                    "%s",
                    "buf.cursize >= CL_ENCODE_START") )
    {
        __debugbreak();
    }
    v3 = buf.data;
    v4 = (unsigned __int8 *)compressedBuf;
    *(unsigned int *)compressedBuf = *(unsigned int *)buf.data;
    *((unsigned int *)v4 + 1) = *((unsigned int *)v3 + 1);
    v4[8] = v3[8];
    if ( buf.cursize > 0x800u )
        Com_Error(ERR_DROP, "Overflow compressed msg buf in CL_WritePacket()");
    compressedSize = MSG_WriteBitsCompress(0, buf.data + 9, buf.cursize - 9, &(*compressedBuf)[9], 2039) + 9;
    packetNum = clc->netchan.outgoingSequence & 0x1F;
    cl->outPackets[packetNum].p_realtime = cls.realtime;
    cl->outPackets[packetNum].p_serverTime = oldcmd->serverTime;
    cl->outPackets[packetNum].p_cmdNumber = cl->cmdNumber;
    clc->lastPacketSentTime = cls.realtime;
    if ( cl_showSend->current.enabled )
    {
        Com_Printf(14, "%i to %s\n", compressedSize, NET_AdrToString(clc->netchan.remoteAddress));
    }
    CL_Netchan_Transmit(&clc->netchan, (unsigned __int8 *)compressedBuf, compressedSize);
    while ( clc->netchan.unsentFragments )
        CL_Netchan_TransmitNextFragment(&clc->netchan);
    //LargeLocal::~LargeLocal(&compressedBuf_large_local);
}

bool __cdecl CG_IsRemoteGuidingMissile(int localClientNum)
{
    return (CG_GetLocalClientGlobals(localClientNum)->predictedPlayerState.eFlags2 & 0x40000) != 0;
}

void __cdecl CL_SendCmd(int localClientNum)
{
    if ( CL_ReadyToSendPacket(localClientNum) )
        CL_WritePacket(localClientNum);
}

bool __cdecl CL_ReadyToSendPacket(int localClientNum)
{
    connstate_t connstate; // [esp+8h] [ebp-Ch]
    int oldPacketNum; // [esp+Ch] [ebp-8h]
    clientConnection_t *clc; // [esp+10h] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    if ( clc->demoplaying )
        return 0;
    if ( Demo_IsPlaying() )
        return 0;
    connstate = CL_GetLocalClientConnectionState(localClientNum);
    if ( connstate == CA_CINEMATIC || connstate == CA_LOGO )
        return 0;
    if ( cls.downloadTempName[0] && cls.realtime - clc->lastPacketSentTime < 50 )
        return 0;
    if ( connstate != CA_ACTIVE
        && connstate != CA_PRIMED
        && !cls.downloadTempName[0]
        && cls.realtime - clc->lastPacketSentTime < 1000 )
    {
        return 0;
    }
    if ( clc->netchan.remoteAddress.type == NA_LOOPBACK )
        return 1;
    if ( Sys_IsLANAddress(clc->netchan.remoteAddress) )
        return 1;
    oldPacketNum = ((unsigned __int8)clc->netchan.outgoingSequence - 1) & 0x1F;
    return cls.realtime - CL_GetLocalClientGlobals(localClientNum)->outPackets[oldPacketNum].p_realtime >= 1000 / cl_maxpackets->current.integer;
}

void __cdecl CL_CreateCmdsDuringConnection(int localClientNum)
{
    connstate_t clcState; // [esp+0h] [ebp-4h]

    clcState = CL_GetLocalClientConnectionState(localClientNum);
    if ( clcState >= CA_CONNECTED && clcState != CA_ACTIVE )
        CL_CreateNewCommands(localClientNum);
}

void __cdecl CL_CreateNewCommands(int localClientNum)
{
    usercmd_s result; // [esp+8h] [ebp-78h] BYREF
    _BYTE v2[52]; // [esp+44h] [ebp-3Ch] BYREF
    clientActive_t *LocalClientGlobals; // [esp+78h] [ebp-8h]
    int cmdNum; // [esp+7Ch] [ebp-4h]

    if ( CL_GetLocalClientConnectionState(localClientNum) >= 9 )
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
        cmdNum = ++LocalClientGlobals->cmdNumber & 0x7F;
        memcpy(v2, CL_CreateCmd(&result, localClientNum), sizeof(v2));
        memcpy(&LocalClientGlobals->cmds[cmdNum], v2, sizeof(LocalClientGlobals->cmds[cmdNum]));
    }
}

usercmd_s *__cdecl CL_CreateCmd(usercmd_s *result, int localClientNum)
{
    int ControllerIndex; // eax
    int i; // [esp+18h] [ebp-50h]
    clientActive_t *LocalClientGlobals; // [esp+1Ch] [ebp-4Ch]
    float oldAngles; // [esp+24h] [ebp-44h]
    usercmd_s cmd; // [esp+30h] [ebp-38h] BYREF

    for ( i = 0; i < 2; ++i )
        cmd.button_bits.array[i] = 0;
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    oldAngles = LocalClientGlobals->viewangles[0];
    CL_AdjustAngles(localClientNum);
    memset(&cmd, 0, sizeof(cmd));
    if ( !Key_IsCatcherActive(localClientNum, 8) || !CG_HandleLocationSelectionInput(localClientNum, &cmd) )
    {
        CL_CmdButtons(localClientNum, &cmd);
        CL_KeyMove(localClientNum, &cmd);
        CL_MouseMove(localClientNum, &cmd);
        ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        if ( GPad_IsActive(ControllerIndex) )
            CL_GamepadMove(localClientNum, &cmd);
        CL_RandomMove(&cmd);
        if ( (float)(LocalClientGlobals->viewangles[0] - oldAngles) <= 90.0 )
        {
            if ( (float)(oldAngles - LocalClientGlobals->viewangles[0]) > 90.0 )
                LocalClientGlobals->viewangles[0] = oldAngles - 90.0;
        }
        else
        {
            LocalClientGlobals->viewangles[0] = oldAngles + 90.0;
        }
        if ( CG_GetLocalClientGlobals(localClientNum)->cameraMode == 2 )
            CL_ClampViewAngle(localClientNum, 0, -30.0, 60.0);
        if ( Demo_IsThirdPersonCamera() )
            CL_ClampViewAngle(localClientNum, 0, -90.0, 90.0);
        if ( Demo_IsMovieCamera() )
        {
            if ( LocalClientGlobals->viewangles[0] > 180.0 )
                LocalClientGlobals->viewangles[0] = LocalClientGlobals->viewangles[0] - 360.0;
            CL_ClampViewAngle(localClientNum, 0, -90.0, 90.0);
        }
    }
    CL_FinishMove(localClientNum, &cmd);
    memcpy(result, &cmd, sizeof(usercmd_s));
    return result;
}

void __cdecl CL_AdjustAngles(int localClientNum)
{
    float v1; // [esp+0h] [ebp-20h]
    float v2; // [esp+4h] [ebp-1Ch]
    float v3; // [esp+8h] [ebp-18h]
    float v4; // [esp+Ch] [ebp-14h]
    clientActive_t *LocalClientGlobals; // [esp+10h] [ebp-10h]
    kbutton_t *kb; // [esp+14h] [ebp-Ch]
    float speed; // [esp+1Ch] [ebp-4h]

    kb = playersKb[localClientNum];
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if ( kb[9].active )
        speed = (float)((float)cls.frametime * 0.001) * cl_anglespeedkey->current.value;
    else
        speed = (float)cls.frametime * 0.001;
    if ( !kb[8].active )
    {
        v4 = speed * cl_yawspeed->current.value;
        LocalClientGlobals->viewangles[1] = LocalClientGlobals->viewangles[1] - CL_KeyState(kb + 1) * v4;
        v3 = speed * cl_yawspeed->current.value;
        LocalClientGlobals->viewangles[1] = CL_KeyState(kb) * v3 + LocalClientGlobals->viewangles[1];
    }
    v2 = speed * cl_pitchspeed->current.value;
    LocalClientGlobals->viewangles[0] = LocalClientGlobals->viewangles[0] - CL_KeyState(kb + 4) * v2;
    v1 = speed * cl_pitchspeed->current.value;
    LocalClientGlobals->viewangles[0] = CL_KeyState(kb + 5) * v1 + LocalClientGlobals->viewangles[0];
}

double __cdecl CL_KeyState(kbutton_t *key)
{
    signed int msec; // [esp+8h] [ebp-4h]

    msec = key->msec;
    key->msec = 0;
    if ( key->active )
    {
        if ( key->downtime )
            msec += com_frameTime - key->downtime;
        else
            msec = com_frameTime;
        key->downtime = com_frameTime;
    }
    if ( msec <= 0 )
        return 0.0;
    if ( msec >= frame_msec )
        return 1.0;
    if ( !frame_msec
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_input_mp.cpp", 313, 0, "%s", "frame_msec") )
    {
        __debugbreak();
    }
    return (double)msec / (double)frame_msec;
}

void __cdecl CL_KeyMove(int localClientNum, usercmd_s *cmd)
{
    float v2; // [esp+4h] [ebp-30h]
    float v3; // [esp+14h] [ebp-20h]
    clientActive_t *LocalClientGlobals; // [esp+20h] [ebp-14h]
    int sidea; // [esp+24h] [ebp-10h]
    int side; // [esp+24h] [ebp-10h]
    int sideb; // [esp+24h] [ebp-10h]
    kbutton_t *kb; // [esp+2Ch] [ebp-8h]
    int forwarda; // [esp+30h] [ebp-4h]
    int forward; // [esp+30h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    kb = playersKb[localClientNum];
    if ( (LocalClientGlobals->snap.ps.eFlags & 0x4000) != 0 )
    {
        LocalClientGlobals->wasInVehicle = 1;
    }
    else if ( LocalClientGlobals->wasInVehicle )
    {
        kb[11].active = 0;
        kb[11].down[1] = 0;
        kb[11].down[0] = 0;
        LocalClientGlobals->wasInVehicle = 0;
    }
    if ( playersKb[localClientNum][KB_PRONE].active || playersKb[localClientNum][11].active )
    {
        if ( playersKb[localClientNum][27].active )
        {
            cmd->button_bits.setBit(8);
            cmd->button_bits.resetBit(9);
        }
        else
        {
            cmd->button_bits.setBit(9);
            cmd->button_bits.resetBit(8);
        }
        cmd->button_bits.setBit(12);
    }
    else
    {
        CL_StanceButtonUpdate(localClientNum);
        CL_AddCurrentStanceToCmd(localClientNum, cmd);
    }

    CL_DoubleTapButtonUpdate(localClientNum, cmd);

    if ( kb[9].active == !LocalClientGlobals->usingAds )
        cmd->button_bits.setBit(0xBu);
    else
        cmd->button_bits.resetBit(0xBu);

    v3 = (float)127;
    sidea = (int)(CL_KeyState(kb + 7) * v3);
    side = sidea - (int)(CL_KeyState(kb + 6) * v3);
    forwarda = (int)(CL_KeyState(kb + 2) * v3);
    forward = forwarda - (int)(CL_KeyState(kb + 3) * v3);
    if ( !kb[3].active )
    {
        if ( kb[30].active || kb[30].wasPressed )
        {
            cmd->button_bits.setBit(1);
            kb[30].wasPressed = 0;
        }
        else
        {
            cmd->button_bits.resetBit(1);
        }
    }
    if ( kb[8].active && !cmd->button_bits.testBit(1) )
    {
        v2 = (float)127;
        sideb = side + (int)(CL_KeyState(kb + 1) * v2);
        side = sideb - (int)(CL_KeyState(kb) * v2);
    }
    cmd->forwardmove = ClampChar(forward);
    cmd->rightmove = ClampChar(side);
}

void __cdecl CL_StanceButtonUpdate(int localClientNum)
{
    int v1; // [esp+0h] [ebp-14h]
    bool v2; // [esp+4h] [ebp-10h]
    clientActive_t *LocalClientGlobals; // [esp+8h] [ebp-Ch]
    bool sprinting; // [esp+Fh] [ebp-5h]

    v2 = playersKb[localClientNum][27].active || playersKb[localClientNum][11].active;
    if ( v2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_input_mp.cpp",
                    1522,
                    0,
                    "%s",
                    "!IN_IsTempStanceKeyActive( localClientNum )") )
    {
        __debugbreak();
    }
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if ( LocalClientGlobals->stanceHeld )
    {
        sprinting = CG_IsSprinting(localClientNum);
        v1 = sprinting ? cl_dtpHoldTime->current.integer : cl_stanceHoldTime->current.integer;
        if ( com_frameTime - LocalClientGlobals->stanceTime >= v1 )
        {
            if ( sprinting )
            {
                LocalClientGlobals->stance = CL_STANCE_DIVE_TO_PRONE;
            }
            else if ( LocalClientGlobals->stancePosition == CL_STANCE_PRONE )
            {
                LocalClientGlobals->stance = CL_STANCE_STAND;
            }
            else
            {
                LocalClientGlobals->stance = CL_STANCE_PRONE;
            }
            LocalClientGlobals->stanceHeld = 0;
        }
    }
}

bool __cdecl CG_IsSprinting(int localClientNum)
{
    const playerState_s *ps; // [esp+4h] [ebp-4h]

    ps = CG_GetPredictedPlayerState(localClientNum);
    return ps->viewHeightCurrent >= 60.0 && (ps->pm_flags & 0x8000) != 0;
}

void __cdecl CL_AddCurrentStanceToCmd(int localClientNum, usercmd_s *cmd)
{
    StanceState stance; // [esp+0h] [ebp-8h]
    clientActive_t *LocalClientGlobals; // [esp+4h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    stance = LocalClientGlobals->stance;
    switch ( stance )
    {
        case CL_STANCE_CROUCH:
            cmd->button_bits.setBit(9);
            cmd->button_bits.resetBit(8);
            goto LABEL_12;
        case CL_STANCE_PRONE:
LABEL_7:
            cmd->button_bits.setBit(8u);
            cmd->button_bits.resetBit(9u);
            goto LABEL_12;
        case CL_STANCE_DIVE_TO_PRONE:
            cmd->button_bits.setBit(0x2Cu);
            goto LABEL_7;
    }
    if ( LocalClientGlobals->stance
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_input_mp.cpp",
                    1563,
                    0,
                    "%s",
                    "cl->stance == CL_STANCE_STAND") )
    {
        __debugbreak();
    }
    cmd->button_bits.resetBit(8u);
    cmd->button_bits.resetBit(9u);
LABEL_12:
    cmd->button_bits.resetBit(0xCu);
}

void __cdecl CL_DoubleTapButtonUpdate(int localClientNum, usercmd_s *cmd)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if ( LocalClientGlobals->useCount == 2 )
    {
        cmd->button_bits.setBit(0x32u);
        LocalClientGlobals->useCount = 0;
        LocalClientGlobals->useTime = 0;
    }
}

void __cdecl CL_GamepadMove(int localClientNum, usercmd_s *cmd)
{
    float v2; // xmm0_4
    int j; // [esp+0h] [ebp-88h]
    int i; // [esp+8h] [ebp-80h]
    AimInput aimInput; // [esp+10h] [ebp-78h] BYREF
    AimOutput aimOutput; // [esp+40h] [ebp-48h] BYREF
    clientActive_t *LocalClientGlobals; // [esp+50h] [ebp-38h]
    float side; // [esp+54h] [ebp-34h]
    float timeMsec; // [esp+58h] [ebp-30h]
    float attack; // [esp+5Ch] [ebp-2Ch]
    kbutton_t *kb; // [esp+60h] [ebp-28h]
    float yaw; // [esp+64h] [ebp-24h]
    int rightMove; // [esp+68h] [ebp-20h]
    float forward; // [esp+6Ch] [ebp-1Ch]
    float pitch; // [esp+70h] [ebp-18h]
    int pitchMove; // [esp+74h] [ebp-14h]
    int forwardMove; // [esp+78h] [ebp-10h]
    float moveScale; // [esp+7Ch] [ebp-Ch]
    int yawMove; // [esp+80h] [ebp-8h]
    float length; // [esp+84h] [ebp-4h]

    if ( gpad_enabled )
    {
        if ( gpad_enabled->current.enabled )
        {
            LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
            if ( (LocalClientGlobals->snap.ps.pm_flags & 0x800) == 0 && LocalClientGlobals->snap.ps.weaponstate != 35 )
            {
                pitch = CL_GamepadAxisValue(localClientNum, 4u);
                pitch = (float)(Dvar_GetBool("input_invertPitch") ? 1 : -1) * pitch;
                yaw = -CL_GamepadAxisValue(localClientNum, 3u);
                forward = CL_GamepadAxisValue(localClientNum, 1u);
                side = CL_GamepadAxisValue(localClientNum, 0);
                attack = CL_GamepadAxisValue(localClientNum, 5u);
                moveScale = 127.0f;
                if ( fabs(side) > 0.0
                    || fabs(forward) > 0.0 )
                {
                    if ( fabs(side) <= fabs(forward) )
                    {
                        length = side / forward;
                        v2 = sqrtf((float)((float)(side / forward) * (float)(side / forward)) + 1.0);
                    }
                    else
                    {
                        length = forward / side;
                        v2 = sqrtf((float)((float)(forward / side) * (float)(forward / side)) + 1.0);
                    }
                    moveScale = v2 * moveScale;
                }
                forwardMove = (int)(float)(forward * moveScale);
                rightMove = (int)(float)(side * moveScale);
                pitchMove = (int)(float)(pitch * moveScale);
                yawMove = (int)(float)(yaw * moveScale);
                cmd->rightmove = ClampChar(rightMove + cmd->rightmove);
                cmd->forwardmove = ClampChar(forwardMove + cmd->forwardmove);
                cmd->pitchmove = ClampChar(pitchMove + cmd->pitchmove);
                cmd->yawmove = ClampChar(yawMove + cmd->yawmove);
                kb = playersKb[localClientNum];
                if ( kb[9].active == !LocalClientGlobals->usingAds )
                    cmd->button_bits.setBit(0xBu);
                if ( !kb[3].active )
                {
                    if ( kb[30].active || kb[30].wasPressed )
                    {
                        cmd->button_bits.setBit(1u);
                        kb[30].wasPressed = 0;
                    }
                    else
                    {
                        cmd->button_bits.resetBit(1u);
                    }
                }
                if ( attack >= cl_analog_attack_threshold->current.value )
                    cmd->button_bits.setBit(0);
                CG_HandleSpecialStateInput(localClientNum, &cmd->button_bits);
                if ( (Demo_IsPlaying() || (LocalClientGlobals->snap.ps.eFlags2 & 0x40000) == 0)
                    && CG_ShouldUpdateViewAngles(localClientNum) )
                {
                    if ( Demo_IsPlaying() )
                        timeMsec = (float)cls.inputRealMsec * 1.0;
                    else
                        timeMsec = (float)cls.frametime * 1.0;
                    for ( i = 0; i < 2; ++i )
                        aimInput.button_bits.array[i] = 0;
                    aimInput.deltaTime = 0.001 * timeMsec;
                    aimInput.pitch = LocalClientGlobals->viewangles[0];
                    aimInput.pitchAxis = pitch;
                    aimInput.pitchMax = LocalClientGlobals->cgameMaxPitchSpeed;
                    aimInput.yaw = LocalClientGlobals->viewangles[1];
                    aimInput.yawAxis = yaw;
                    aimInput.yawMax = LocalClientGlobals->cgameMaxYawSpeed;
                    aimInput.forwardAxis = forward;
                    aimInput.rightAxis = side;
                    for ( j = 0; j < 2; ++j )
                        aimInput.button_bits.array[j] = cmd->button_bits.array[j];
                    aimInput.localClientNum = localClientNum;
                    AimAssist_UpdateGamePadInput(&aimInput, &aimOutput);
                    LocalClientGlobals->viewangles[0] = aimOutput.pitch;
                    LocalClientGlobals->viewangles[1] = aimOutput.yaw;
                    cmd->meleeChargeYaw = aimOutput.meleeChargeYaw;
                    cmd->meleeChargeDist = aimOutput.meleeChargeDist;
                }
            }
        }
    }
}

void __cdecl CL_RandomMove(usercmd_s *cmd)
{
    float v1; // xmm0_4
    unsigned int i; // [esp+Ch] [ebp-C8h]
    clientActive_t *LocalClientGlobals; // [esp+10h] [ebp-C4h]
    float side; // [esp+14h] [ebp-C0h]
    float attack; // [esp+18h] [ebp-BCh]
    float yaw; // [esp+20h] [ebp-B4h]
    float forward; // [esp+28h] [ebp-ACh]
    int pitchMove; // [esp+30h] [ebp-A4h]
    int buttonFreqTable[11][3]; // [esp+34h] [ebp-A0h]
    int forwardMove; // [esp+C0h] [ebp-14h]
    float moveScale; // [esp+C4h] [ebp-10h]
    int yawMove; // [esp+C8h] [ebp-Ch]
    float length; // [esp+CCh] [ebp-8h]
    float anglespeed; // [esp+D0h] [ebp-4h]

    if ( Monkey_UseRandomInput() )
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(0);
        if ( (LocalClientGlobals->snap.ps.pm_flags & 0x800) == 0 )
        {
            yaw = -flrand(0.0, 0.5);
            forward = flrand(0.0, 1.0);
            side = flrand(0.0, 1.0);
            attack = flrand(0.0, 1.0);
            *(_QWORD *)&buttonFreqTable[0][0] = 17;
            buttonFreqTable[0][2] = 200;
            *(_QWORD *)&buttonFreqTable[1][0] = 0x10000001ELL;
            buttonFreqTable[1][2] = 200;
            *(_QWORD *)&buttonFreqTable[2][0] = 0x200000015LL;
            buttonFreqTable[2][2] = 5000;
            *(_QWORD *)&buttonFreqTable[3][0] = 0x400000017LL;
            buttonFreqTable[3][2] = 1000;
            *(_QWORD *)&buttonFreqTable[4][0] = 0x500000018LL;
            buttonFreqTable[4][2] = 1000;
            *(_QWORD *)&buttonFreqTable[5][0] = 0x600000019LL;
            buttonFreqTable[5][2] = 5000;
            *(_QWORD *)&buttonFreqTable[6][0] = 0x70000001ALL;
            buttonFreqTable[6][2] = 5000;
            *(_QWORD *)&buttonFreqTable[7][0] = 0x80000001BLL;
            buttonFreqTable[7][2] = 5000;
            *(_QWORD *)&buttonFreqTable[8][0] = 0x90000001CLL;
            buttonFreqTable[8][2] = 5000;
            *(_QWORD *)&buttonFreqTable[9][0] = 0xA0000000ALL;
            buttonFreqTable[9][2] = 200;
            *(_QWORD *)&buttonFreqTable[10][0] = 0xF00000014LL;
            buttonFreqTable[10][2] = 1000;
            for ( i = 0; i < 0xB; ++i )
            {
                if ( irand(0, buttonFreqTable[i][2]) == 1 )
                    CL_UpdateCmdButton(0, &cmd->button_bits, buttonFreqTable[i][0], buttonFreqTable[i][1], 1);
            }
            moveScale = 127.0f;
            if ( fabs(side) > 0.0
                || fabs(forward) > 0.0 )
            {
                if ( fabs(side) <= fabs(forward) )
                {
                    length = side / forward;
                    v1 = sqrtf((float)((float)(side / forward) * (float)(side / forward)) + 1.0);
                }
                else
                {
                    length = forward / side;
                    v1 = sqrtf((float)((float)(forward / side) * (float)(forward / side)) + 1.0);
                }
                moveScale = v1 * moveScale;
            }
            forwardMove = (int)(float)(forward * moveScale);
            pitchMove = (int)(float)(0.0 * moveScale);
            yawMove = (int)(float)(yaw * moveScale);
            cmd->rightmove = ClampChar((int)(float)(side * moveScale) + cmd->rightmove);
            cmd->forwardmove = ClampChar(forwardMove + cmd->forwardmove);
            cmd->pitchmove = ClampChar(pitchMove + cmd->pitchmove);
            cmd->yawmove = ClampChar(yawMove + cmd->yawmove);
            if ( playersKb[0][9].active != LocalClientGlobals->usingAds )
                CL_UpdateCmdButton(0, &cmd->button_bits, 8, 11, 1);
            if ( !playersKb[0][3].active )
            {
                if ( playersKb[0][30].active || playersKb[0][30].wasPressed )
                {
                    cmd->button_bits.setBit(1u);
                    playersKb[0][30].wasPressed = 0;
                }
                else
                {
                    cmd->button_bits.resetBit(1u);
                }
            }
            if ( attack >= cl_analog_attack_threshold->current.value )
                cmd->button_bits.setBit(0);
            anglespeed = (float)cls.frametime * 0.001;
            if ( playersKb[0][9].active )
                anglespeed = anglespeed * cl_anglespeedkey->current.value;
            LocalClientGlobals->viewangles[0] = (float)((float)(anglespeed * cl_pitchspeed->current.value) * 0.0)
                                                                                + LocalClientGlobals->viewangles[0];
            LocalClientGlobals->viewangles[1] = (float)((float)(anglespeed * cl_yawspeed->current.value) * yaw)
                                                                                + LocalClientGlobals->viewangles[1];
        }
    }
}

void __cdecl CL_MouseMove(int localClientNum, usercmd_s *cmd)
{
    float v2; // [esp+10h] [ebp-E0h]
    float v3; // [esp+14h] [ebp-DCh]
    float v4; // [esp+20h] [ebp-D0h]
    float v5; // [esp+24h] [ebp-CCh]
    int j; // [esp+38h] [ebp-B8h]
    int i; // [esp+40h] [ebp-B0h]
    float v8; // [esp+5Ch] [ebp-94h]
    float v9; // [esp+64h] [ebp-8Ch]
    clientActive_t *LocalClientGlobals; // [esp+84h] [ebp-6Ch]
    float rate; // [esp+88h] [ebp-68h]
    float delta; // [esp+8Ch] [ebp-64h]
    float deltaa; // [esp+8Ch] [ebp-64h]
    AimInput aimInput; // [esp+90h] [ebp-60h] BYREF
    kbutton_t *kb; // [esp+C0h] [ebp-30h]
    float mx; // [esp+C4h] [ebp-2Ch] BYREF
    float my; // [esp+C8h] [ebp-28h] BYREF
    AimOutput aimOutput; // [esp+CCh] [ebp-24h] BYREF
    int pitchMove; // [esp+DCh] [ebp-14h]
    int yawMove; // [esp+E0h] [ebp-10h]
    float moveScale; // [esp+E4h] [ebp-Ch]
    float cap; // [esp+E8h] [ebp-8h]
    float accelSensitivity; // [esp+ECh] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    CL_GetMouseMovement(LocalClientGlobals, &mx, &my);
    if ( frame_msec )
    {
        rate = sqrtf((float)(mx * mx) + (float)(my * my)) / (double)frame_msec;
        accelSensitivity = (float)(rate * cl_mouseAccel->current.value) + cl_sensitivity->current.value;
        accelSensitivity = accelSensitivity * LocalClientGlobals->cgameFOVSensitivityScale;
        if ( rate != 0.0 && cl_showMouseRate->current.enabled )
            Com_Printf(14, "%f : %f\n", rate, accelSensitivity);
        if ( (LocalClientGlobals->snap.ps.pm_flags & 0x800) == 0 && LocalClientGlobals->snap.ps.weaponstate != 35 )
        {
            mx = mx * accelSensitivity;
            my = my * accelSensitivity;
            if ( mx != 0.0 || my != 0.0 )
            {
                yawMove = 0;
                pitchMove = 0;
                moveScale = 127.0f;
                kb = playersKb[localClientNum];
                if ( kb[8].active )
                {
                    cmd->rightmove = ClampChar((int)((float)(m_side->current.value * mx) + 9.313225746154785e-10) + cmd->rightmove);
                }
                else
                {
                    delta = m_yaw->current.value * mx;
                    if ( LocalClientGlobals->cgameMaxYawSpeed > 0.0 )
                    {
                        cap = (double)frame_msec * LocalClientGlobals->cgameMaxYawSpeed * 0.001;
                        if ( (float)(delta - cap) < 0.0 )
                            v9 = delta;
                        else
                            v9 = cap;
                        if ( (float)((-(cap)) - v9) < 0.0 )
                            v5 = v9;
                        else
                            v5 = -cap;
                        delta = v5;
                    }
                    LocalClientGlobals->viewangles[1] = LocalClientGlobals->viewangles[1] - delta;
                    if ( (float)(moveScale - mx) < 0.0 )
                        v4 = moveScale;
                    else
                        v4 = mx;
                    yawMove = -(int)v4;
                }
                if ( (kb[13].active || cl_freelook->current.enabled) && !kb[8].active )
                {
                    deltaa = m_pitch->current.value * my;
                    if ( LocalClientGlobals->cgameMaxPitchSpeed > 0.0 )
                    {
                        cap = (double)frame_msec * LocalClientGlobals->cgameMaxPitchSpeed * 0.001;
                        if ( (float)(deltaa - cap) < 0.0 )
                            v8 = deltaa;
                        else
                            v8 = cap;
                        if ( (float)((-(cap)) - v8) < 0.0 )
                            v3 = v8;
                        else
                            v3 = -cap;
                        deltaa = v3;
                    }
                    LocalClientGlobals->viewangles[0] = LocalClientGlobals->viewangles[0] + deltaa;
                    if ( (float)(moveScale - my) < 0.0 )
                        v2 = moveScale;
                    else
                        v2 = my;
                    pitchMove = (int)v2;
                }
                else
                {
                    cmd->forwardmove = ClampChar(cmd->forwardmove - (int)((float)(m_forward->current.value * my)
                                                                                                                            + 9.313225746154785e-10));
                }
                cmd->pitchmove = ClampChar(pitchMove + cmd->pitchmove);
                cmd->yawmove = ClampChar(yawMove + cmd->yawmove);
            }
            if ( (LocalClientGlobals->snap.ps.eFlags2 & 0x40000) == 0 )
            {
                for ( i = 0; i < 2; ++i )
                    aimInput.button_bits.array[i] = 0;
                aimInput.deltaTime = (float)cls.frametime * 0.001;
                aimInput.pitch = LocalClientGlobals->viewangles[0];
                aimInput.pitchAxis = 0.0f;
                aimInput.pitchMax = 0.0f;
                aimInput.yaw = LocalClientGlobals->viewangles[1];
                memset(&aimInput.yawAxis, 0, 16);
                for ( j = 0; j < 2; ++j )
                    aimInput.button_bits.array[j] = cmd->button_bits.array[j];
                aimInput.localClientNum = 0;
                AimAssist_UpdateMouseInput(&aimInput, &aimOutput);
                LocalClientGlobals->viewangles[0] = aimOutput.pitch;
                LocalClientGlobals->viewangles[1] = aimOutput.yaw;
                cmd->meleeChargeYaw = aimOutput.meleeChargeYaw;
                cmd->meleeChargeDist = aimOutput.meleeChargeDist;
            }
        }
    }
}

void __cdecl CL_GetMouseMovement(clientActive_t *cl, float *mx, float *my)
{
    float v3; // xmm0_4

    if ( !mx && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_input_mp.cpp", 2148, 0, "%s", "mx") )
        __debugbreak();
    if ( !my && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_input_mp.cpp", 2149, 0, "%s", "my") )
        __debugbreak();
    if ( m_filter->current.enabled )
    {
        *mx = (float)(cl->mouseDx[1] + cl->mouseDx[0]) * 0.5;
        v3 = (float)(cl->mouseDy[1] + cl->mouseDy[0]) * 0.5;
    }
    else
    {
        *mx = (float)cl->mouseDx[cl->mouseIndex];
        v3 = (float)cl->mouseDy[cl->mouseIndex];
    }
    *my = v3;
    cl->mouseIndex ^= 1u;
    cl->mouseDx[cl->mouseIndex] = 0;
    cl->mouseDy[cl->mouseIndex] = 0;
}

void __cdecl CL_CmdButtons(int localClientNum, usercmd_s *cmd)
{
    bool v3; // [esp+4h] [ebp-Ch]
    clientActive_t *LocalClientGlobals; // [esp+8h] [ebp-8h]

    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 17, 0, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 18, 13, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 19, 14, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 20, 15, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 21, 2, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 22, 3, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 23, 4, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 24, 5, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 25, 6, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 26, 7, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 27, 8, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 28, 9, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 10, 10, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 29, 24, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 32, 19, 3);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 33, 22, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 34, 23, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 35, 28, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 36, 34, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 37, 35, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 38, 36, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 39, 37, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 45, 38, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 40, 39, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 41, 40, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 42, 41, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 43, 42, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 44, 43, 1);
    v3 = Key_IsCatcherActive(localClientNum, -1) && !cl_bypassMouseInput->current.enabled;
    if ( v3 && UI_GetActiveMenu(localClientNum) != UIMENU_SCOREBOARD )
        cmd->button_bits.setBit(0x1Du);
    if ( CL_GetLocalClientConnectionState(localClientNum) > CA_CONNECTED )
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
        if ( LocalClientGlobals->snap.ps.pm_type == 2
            || LocalClientGlobals->snap.ps.pm_type == 3
            || LocalClientGlobals->snap.ps.pm_type == 4 )
        {
            CL_UpdateCmdButton(localClientNum, &cmd->button_bits, 12, 10, 1);
        }
    }
}

void __cdecl CL_FinishMove(int localClientNum, usercmd_s *cmd)
{
    int serverTime; // [esp+4h] [ebp-18h]
    int k; // [esp+8h] [ebp-14h]
    int j; // [esp+10h] [ebp-Ch]
    int i; // [esp+14h] [ebp-8h]
    clientActive_t *LocalClientGlobals; // [esp+18h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    cmd->weapon = LocalClientGlobals->cgameUserCmdWeapon;
    cmd->offHandIndex = LocalClientGlobals->cgameUserCmdOffHandIndex;
    cmd->lastWeaponAltModeSwitch = LocalClientGlobals->cgameUserCmdLastWeaponForAlt;
    if ( LocalClientGlobals->serverTime - LocalClientGlobals->snap.serverTime > 5000 )
        serverTime = LocalClientGlobals->snap.serverTime + 5000;
    else
        serverTime = LocalClientGlobals->serverTime;
    cmd->serverTime = serverTime;
    for ( i = 0; i < 3; ++i )
        cmd->angles[i] = (unsigned __int16)(int)(float)((float)(LocalClientGlobals->viewangles[i]
                                                                                                                    + LocalClientGlobals->cgameKickAngles[i])
                                                                                                    * 182.04445);
    for ( j = 0; j < 2; ++j )
        cmd->button_bits.array[j] |= LocalClientGlobals->cgameExtraButton_bits.array[j];
    for ( k = 0; k < 2; ++k )
        LocalClientGlobals->cgameExtraButton_bits.array[k] = 0;
}

char __cdecl CG_HandleLocationSelectionInput(int localClientNum, usercmd_s *cmd)
{
    float v3; // [esp+38h] [ebp-68h]
    float v4; // [esp+3Ch] [ebp-64h]
    float v5; // [esp+40h] [ebp-60h]
    float v6; // [esp+44h] [ebp-5Ch]
    int i; // [esp+48h] [ebp-58h]
    float yawVector[3]; // [esp+4Ch] [ebp-54h] BYREF
    clientActive_t *LocalClientGlobals; // [esp+58h] [ebp-48h]
    float cursorSpeedScalar; // [esp+5Ch] [ebp-44h]
    float mapAspectRatio; // [esp+60h] [ebp-40h]
    float diffAngles[3]; // [esp+64h] [ebp-3Ch] BYREF
    cg_s *cgameGlob; // [esp+70h] [ebp-30h]
    float right; // [esp+74h] [ebp-2Ch]
    float mx; // [esp+78h] [ebp-28h] BYREF
    float my; // [esp+7Ch] [ebp-24h] BYREF
    float frametime; // [esp+80h] [ebp-20h]
    float yawRight; // [esp+84h] [ebp-1Ch]
    LocSelInputState locSelInputState; // [esp+88h] [ebp-18h]
    float magnitude; // [esp+8Ch] [ebp-14h]
    float up; // [esp+90h] [ebp-10h]
    float yawUp; // [esp+94h] [ebp-Ch]
    float yawCheck; // [esp+98h] [ebp-8h]
    float cursorRotationSpeedScalar; // [esp+9Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->predictedPlayerState.locationSelectionInfo )
    {
        Key_RemoveCatcher(localClientNum, -9);
        return 0;
    }
    CL_AddCurrentStanceToCmd(localClientNum, cmd);
    cmd->button_bits.setBit(0x1Du);
    frametime = (float)cgameGlob->frametime * 0.001;
    mapAspectRatio = cgameGlob->compassMapWorldSize[0] / cgameGlob->compassMapWorldSize[1];
    locSelInputState = playerKeys[localClientNum].locSelInputState;
    cursorSpeedScalar = 0.1f;
    cursorRotationSpeedScalar = 2.0f;
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    CL_GetMouseMovement(LocalClientGlobals, &mx, &my);
    if ( mx != 0.0 || my != 0.0 )
    {
        if ( locSelInputState == LOC_SEL_INPUT_YAW )
        {
            *(_QWORD *)yawVector = *(_QWORD *)cgameGlob->selectedYaw;
            yawVector[2] = 0.0f;
            vectoangles(yawVector, diffAngles);
            diffAngles[0] = 0.0f;
            diffAngles[1] = (float)((float)((float)((float)cg_mapLocationSelectionRotationSpeed->current.integer * mx)
                                                                        * frametime)
                                                        * cursorRotationSpeedScalar)
                                        + diffAngles[1];
            AngleVectors(diffAngles, yawVector, 0, 0);
            *(_QWORD *)cgameGlob->selectedYaw = *(_QWORD *)yawVector;
            return 1;
        }
        cgameGlob->selectedLocation[0] = (float)((float)((float)(mx * cg_mapLocationSelectionCursorSpeed->current.value)
                                                                                                     * frametime)
                                                                                     * cursorSpeedScalar)
                                                                     + cgameGlob->selectedLocation[0];
        cgameGlob->selectedLocation[1] = (float)((float)((float)((float)(my * mapAspectRatio)
                                                                                                                     * cg_mapLocationSelectionCursorSpeed->current.value)
                                                                                                     * frametime)
                                                                                     * cursorSpeedScalar)
                                                                     + cgameGlob->selectedLocation[1];
    }
    up = CL_GamepadAxisValue(localClientNum, 1u);
    right = CL_GamepadAxisValue(localClientNum, 0);
    magnitude = (float)(up * up) + (float)(right * right);
    if ( magnitude > 1.0 )
    {
        magnitude = sqrtf(magnitude);
        up = up / magnitude;
        right = right / magnitude;
    }
    cgameGlob->selectedLocation[0] = (float)((float)(right * cg_mapLocationSelectionCursorSpeed->current.value) * frametime)
                                                                 + cgameGlob->selectedLocation[0];
    cgameGlob->selectedLocation[1] = cgameGlob->selectedLocation[1]
                                                                 - (float)((float)((float)(up * mapAspectRatio)
                                                                                                 * cg_mapLocationSelectionCursorSpeed->current.value)
                                                                                 * frametime);
    yawUp = CL_GamepadAxisValue(localClientNum, 4u);
    yawRight = CL_GamepadAxisValue(localClientNum, 3u);
    if ( cgameGlob->predictedPlayerState.locationSelectionType == LOC_SEL_NAPALM
        || cgameGlob->predictedPlayerState.locationSelectionType == LOC_SEL_AIRSTRIKE )
    {
        if ( yawUp != 0.0 && yawRight != 0.0 )
        {
            yawCheck = yawUp * yawRight;
            if ( yawUp > 0.5 || yawUp < -0.5 || yawRight > 0.5 || yawRight < -0.5 || yawCheck < -0.125 || yawCheck > 0.125 )
            {
                cgameGlob->selectedYaw[0] = yawUp;
                cgameGlob->selectedYaw[1] = yawRight;
            }
        }
    }
    else
    {
        cgameGlob->selectedLocation[0] = (float)((float)(yawRight * cg_mapLocationSelectionCursorSpeed->current.value)
                                                                                     * frametime)
                                                                     + cgameGlob->selectedLocation[0];
        cgameGlob->selectedLocation[1] = cgameGlob->selectedLocation[1]
                                                                     - (float)((float)((float)(yawUp * mapAspectRatio)
                                                                                                     * cg_mapLocationSelectionCursorSpeed->current.value)
                                                                                     * frametime);
        cgameGlob->selectedYaw[0] = 0.0f;
        cgameGlob->selectedYaw[1] = 0.0f;
    }
    v6 = cgameGlob->selectedLocation[0];
    if ( 1.0 <= 0.0
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 730, 0, "%s", "min < max") )
    {
        __debugbreak();
    }
    if ( v6 >= 0.0 )
    {
        if ( v6 <= 1.0 )
            v5 = v6;
        else
            v5 = 1.0f;
    }
    else
    {
        v5 = 0.0f;
    }
    cgameGlob->selectedLocation[0] = v5;
    v4 = cgameGlob->selectedLocation[1];
    if ( 1.0 <= 0.0
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 730, 0, "%s", "min < max") )
    {
        __debugbreak();
    }
    if ( v4 >= 0.0 )
    {
        if ( v4 <= 1.0 )
            v3 = v4;
        else
            v3 = 1.0f;
    }
    else
    {
        v3 = 0.0f;
    }
    cgameGlob->selectedLocation[1] = v3;
    if ( locSelInputState && locSelInputState != LOC_SEL_INPUT_YAW )
    {
        cmd->selectedLocation[0] = (int)((float)(cgameGlob->selectedLocation[0] * 255.0) + 9.313225746154785e-10) + 0x80;
        cmd->selectedLocation[1] = (int)((float)(cgameGlob->selectedLocation[1] * 255.0) + 9.313225746154785e-10) + 0x80;
        if ( cgameGlob->lastTimeSelected + 200 < cgameGlob->time && locSelInputState == LOC_SEL_INPUT_CONFIRM )
        {
            for ( i = 0; i < 3; ++i )
            {
                if ( cgameGlob->selectedLocations[i][0] == -1.0 )
                {
                    cgameGlob->lastTimeSelected = cgameGlob->time;
                    cgameGlob->selectedLocations[i][0] = cgameGlob->selectedLocation[0];
                    cgameGlob->selectedLocations[i][1] = cgameGlob->selectedLocation[1];
                    break;
                }
            }
            *(_QWORD *)yawVector = *(_QWORD *)cgameGlob->selectedYaw;
            yawVector[2] = 0.0f;
            vectoangles(yawVector, diffAngles);
            diffAngles[1] = 360.0 - diffAngles[1];
            cmd->selectedYaw = (int)(diffAngles[1] + 9.313225746154785e-10) / 2;
            cmd->button_bits.setBit(0x10u);
        }
        if ( locSelInputState == LOC_SEL_INPUT_CANCEL )
            cmd->button_bits.setBit(0x11u);
    }
    return 1;
}

void __cdecl CL_Input(int localClientNum)
{
    if ( CL_GetLocalClientConnectionState(localClientNum) == 10 )
        CL_CreateNewCommands(localClientNum);
}

cmd_function_s IN_CenterView_VAR;
cmd_function_s IN_UpDown_VAR;
cmd_function_s IN_UpUp_VAR;
cmd_function_s IN_DownDown_VAR;
cmd_function_s IN_DownUp_VAR;
cmd_function_s IN_LeftDown_VAR;
cmd_function_s IN_LeftUp_VAR;
cmd_function_s IN_RightDown_VAR;
cmd_function_s IN_RightUp_VAR;
cmd_function_s IN_ForwardDown_VAR;
cmd_function_s IN_ForwardUp_VAR;
cmd_function_s IN_BackDown_VAR;
cmd_function_s IN_BackUp_VAR;
cmd_function_s IN_LookupDown_VAR;
cmd_function_s IN_LookupUp_VAR;
cmd_function_s IN_LookdownDown_VAR;
cmd_function_s IN_LookdownUp_VAR;
cmd_function_s IN_StrafeDown_VAR;
cmd_function_s IN_StrafeUp_VAR;
cmd_function_s IN_MoveleftDown_VAR;
cmd_function_s IN_MoveleftUp_VAR;
cmd_function_s IN_MoverightDown_VAR;
cmd_function_s IN_MoverightUp_VAR;
cmd_function_s IN_SpeedDown_VAR;
cmd_function_s IN_SpeedUp_VAR;
cmd_function_s IN_Attack_Down_VAR;
cmd_function_s IN_Attack_Up_VAR;
cmd_function_s IN_Melee_Down_VAR;
cmd_function_s IN_Melee_Up_VAR;
cmd_function_s IN_Breath_Down_VAR;
cmd_function_s IN_Breath_Up_VAR;
cmd_function_s IN_MeleeBreath_Down_VAR;
cmd_function_s IN_MeleeBreath_Up_VAR;
cmd_function_s IN_Frag_Down_VAR;
cmd_function_s IN_Frag_Up_VAR;
cmd_function_s IN_Smoke_Down_VAR;
cmd_function_s IN_Smoke_Up_VAR;
cmd_function_s IN_BreathSprint_Down_VAR;
cmd_function_s IN_BreathSprint_Up_VAR;
cmd_function_s IN_Activate_Down_VAR;
cmd_function_s IN_Activate_Up_VAR;
cmd_function_s IN_Reload_Down_VAR;
cmd_function_s IN_Reload_Up_VAR;
cmd_function_s IN_UseReload_Down_VAR;
cmd_function_s IN_UseReload_Up_VAR;
cmd_function_s IN_LeanLeft_Down_VAR;
cmd_function_s IN_LeanLeft_Up_VAR;
cmd_function_s IN_LeanRight_Down_VAR;
cmd_function_s IN_LeanRight_Up_VAR;
cmd_function_s IN_Prone_Down_VAR;
cmd_function_s IN_Prone_Up_VAR;
cmd_function_s IN_Stance_Down_VAR;
cmd_function_s IN_Stance_Up_VAR;
cmd_function_s IN_MLookDown_VAR;
cmd_function_s IN_MLookUp_VAR;
cmd_function_s IN_ToggleADS_VAR;
cmd_function_s IN_LeaveADS_VAR;
cmd_function_s IN_Throw_Down_VAR;
cmd_function_s IN_Throw_Up_VAR;
cmd_function_s IN_Speed_Throw_Down_VAR;
cmd_function_s IN_Speed_Throw_Up_VAR;
cmd_function_s IN_ToggleADS_Throw_Down_VAR;
cmd_function_s IN_ToggleADS_Throw_Up_VAR;
cmd_function_s IN_Gas_Down_VAR;
cmd_function_s IN_Gas_Up_VAR;
cmd_function_s IN_Reverse_Down_VAR;
cmd_function_s IN_Reverse_Up_VAR;
cmd_function_s IN_Handbrake_Down_VAR;
cmd_function_s IN_Handbrake_Up_VAR;
cmd_function_s IN_SwitchSeat_Down_VAR;
cmd_function_s IN_SwitchSeat_Up_VAR;
cmd_function_s IN_VehicleAttack_Down_VAR;
cmd_function_s IN_VehicleAttack_Up_VAR;
cmd_function_s IN_VehicleBoost_Down_VAR;
cmd_function_s IN_VehicleBoost_Up_VAR;
cmd_function_s IN_VehicleAttackSecond_Down_VAR;
cmd_function_s IN_VehicleAttackSecond_Up_VAR;
cmd_function_s IN_VehicleMoveUp_Down_VAR;
cmd_function_s IN_VehicleMoveUp_Up_VAR;
cmd_function_s IN_VehicleMoveDown_Down_VAR;
cmd_function_s IN_VehicleMoveDown_Up_VAR;
cmd_function_s IN_VehicleSpecialAbility_Down_VAR;
cmd_function_s IN_VehicleSpecialAbility_Up_VAR;
cmd_function_s IN_VehicleFirePickup_Down_VAR;
cmd_function_s IN_VehicleFirePickup_Up_VAR;
cmd_function_s IN_VehicleSwapPickup_Down_VAR;
cmd_function_s IN_VehicleSwapPickup_Up_VAR;
cmd_function_s IN_VehicleDropDeployable_Down_VAR;
cmd_function_s IN_VehicleDropDeployable_Up_VAR;
cmd_function_s IN_LowerStance_VAR;
cmd_function_s IN_RaiseStance_VAR;
cmd_function_s IN_ToggleCrouch_VAR;
cmd_function_s IN_ToggleProne_VAR;
cmd_function_s IN_GoProne_VAR;
cmd_function_s IN_GoCrouch_VAR;
cmd_function_s IN_GoStandDown_VAR;
cmd_function_s IN_GoStandUp_VAR;
cmd_function_s IN_SpecNext_Down_VAR;
cmd_function_s IN_SpecNext_Up_VAR;
cmd_function_s IN_SpecPrev_Down_VAR;
cmd_function_s IN_SpecPrev_Up_VAR;
cmd_function_s IN_ToggleSpec_Down_VAR;
cmd_function_s IN_ToggleSpec_Up_VAR;
cmd_function_s IN_ToggleView_VAR;
cmd_function_s IN_TalkDown_VAR;
cmd_function_s IN_TalkUp_VAR;
cmd_function_s IN_SprintDown_VAR;
cmd_function_s IN_SprintUp_VAR;

void __cdecl CL_InitInput()
{
    Cmd_AddCommandInternal("centerview", IN_CenterView, &IN_CenterView_VAR);
    Cmd_AddCommandInternal("+moveup", IN_UpDown, &IN_UpDown_VAR);
    Cmd_AddCommandInternal("-moveup", IN_UpUp, &IN_UpUp_VAR);
    Cmd_AddCommandInternal("+movedown", IN_DownDown, &IN_DownDown_VAR);
    Cmd_AddCommandInternal("-movedown", IN_DownUp, &IN_DownUp_VAR);
    Cmd_AddCommandInternal("+left", IN_LeftDown, &IN_LeftDown_VAR);
    Cmd_AddCommandInternal("-left", IN_LeftUp, &IN_LeftUp_VAR);
    Cmd_AddCommandInternal("+right", IN_RightDown, &IN_RightDown_VAR);
    Cmd_AddCommandInternal("-right", IN_RightUp, &IN_RightUp_VAR);
    Cmd_AddCommandInternal("+forward", IN_ForwardDown, &IN_ForwardDown_VAR);
    Cmd_AddCommandInternal("-forward", IN_ForwardUp, &IN_ForwardUp_VAR);
    Cmd_AddCommandInternal("+back", IN_BackDown, &IN_BackDown_VAR);
    Cmd_AddCommandInternal("-back", IN_BackUp, &IN_BackUp_VAR);
    Cmd_AddCommandInternal("+lookup", IN_LookupDown, &IN_LookupDown_VAR);
    Cmd_AddCommandInternal("-lookup", IN_LookupUp, &IN_LookupUp_VAR);
    Cmd_AddCommandInternal("+lookdown", IN_LookdownDown, &IN_LookdownDown_VAR);
    Cmd_AddCommandInternal("-lookdown", IN_LookdownUp, &IN_LookdownUp_VAR);
    Cmd_AddCommandInternal("+strafe", IN_StrafeDown, &IN_StrafeDown_VAR);
    Cmd_AddCommandInternal("-strafe", IN_StrafeUp, &IN_StrafeUp_VAR);
    Cmd_AddCommandInternal("+moveleft", IN_MoveleftDown, &IN_MoveleftDown_VAR);
    Cmd_AddCommandInternal("-moveleft", IN_MoveleftUp, &IN_MoveleftUp_VAR);
    Cmd_AddCommandInternal("+moveright", IN_MoverightDown, &IN_MoverightDown_VAR);
    Cmd_AddCommandInternal("-moveright", IN_MoverightUp, &IN_MoverightUp_VAR);
    Cmd_AddCommandInternal("+speed", IN_SpeedDown, &IN_SpeedDown_VAR);
    Cmd_AddCommandInternal("-speed", IN_SpeedUp, &IN_SpeedUp_VAR);
    Cmd_AddCommandInternal("+attack", IN_Attack_Down, &IN_Attack_Down_VAR);
    Cmd_AddCommandInternal("-attack", IN_Attack_Up, &IN_Attack_Up_VAR);
    Cmd_AddCommandInternal("+melee", IN_Melee_Down, &IN_Melee_Down_VAR);
    Cmd_AddCommandInternal("-melee", IN_Melee_Up, &IN_Melee_Up_VAR);
    Cmd_AddCommandInternal("+holdbreath", IN_Breath_Down, &IN_Breath_Down_VAR);
    Cmd_AddCommandInternal("-holdbreath", IN_Breath_Up, &IN_Breath_Up_VAR);
    Cmd_AddCommandInternal("+melee_breath", IN_MeleeBreath_Down, &IN_MeleeBreath_Down_VAR);
    Cmd_AddCommandInternal("-melee_breath", IN_MeleeBreath_Up, &IN_MeleeBreath_Up_VAR);
    Cmd_AddCommandInternal("+frag", IN_Frag_Down, &IN_Frag_Down_VAR);
    Cmd_AddCommandInternal("-frag", IN_Frag_Up, &IN_Frag_Up_VAR);
    Cmd_AddCommandInternal("+smoke", IN_Smoke_Down, &IN_Smoke_Down_VAR);
    Cmd_AddCommandInternal("-smoke", IN_Smoke_Up, &IN_Smoke_Up_VAR);
    Cmd_AddCommandInternal("+breath_sprint", IN_BreathSprint_Down, &IN_BreathSprint_Down_VAR);
    Cmd_AddCommandInternal("-breath_sprint", IN_BreathSprint_Up, &IN_BreathSprint_Up_VAR);
    Cmd_AddCommandInternal("+activate", IN_Activate_Down, &IN_Activate_Down_VAR);
    Cmd_AddCommandInternal("-activate", IN_Activate_Up, &IN_Activate_Up_VAR);
    Cmd_AddCommandInternal("+reload", IN_Reload_Down, &IN_Reload_Down_VAR);
    Cmd_AddCommandInternal("-reload", IN_Reload_Up, &IN_Reload_Up_VAR);
    Cmd_AddCommandInternal("+usereload", IN_UseReload_Down, &IN_UseReload_Down_VAR);
    Cmd_AddCommandInternal("-usereload", IN_UseReload_Up, &IN_UseReload_Up_VAR);
    Cmd_AddCommandInternal("+leanleft", IN_LeanLeft_Down, &IN_LeanLeft_Down_VAR);
    Cmd_AddCommandInternal("-leanleft", IN_LeanLeft_Up, &IN_LeanLeft_Up_VAR);
    Cmd_AddCommandInternal("+leanright", IN_LeanRight_Down, &IN_LeanRight_Down_VAR);
    Cmd_AddCommandInternal("-leanright", IN_LeanRight_Up, &IN_LeanRight_Up_VAR);
    Cmd_AddCommandInternal("+prone", IN_Prone_Down, &IN_Prone_Down_VAR);
    Cmd_AddCommandInternal("-prone", IN_Prone_Up, &IN_Prone_Up_VAR);
    Cmd_AddCommandInternal("+stance", IN_Stance_Down, &IN_Stance_Down_VAR);
    Cmd_AddCommandInternal("-stance", IN_Stance_Up, &IN_Stance_Up_VAR);
    Cmd_AddCommandInternal("+mlook", IN_MLookDown, &IN_MLookDown_VAR);
    Cmd_AddCommandInternal("-mlook", IN_MLookUp, &IN_MLookUp_VAR);
    Cmd_AddCommandInternal("toggleads", IN_ToggleADS, &IN_ToggleADS_VAR);
    Cmd_AddCommandInternal("leaveads", IN_LeaveADS, &IN_LeaveADS_VAR);
    Cmd_AddCommandInternal("+throw", IN_Throw_Down, &IN_Throw_Down_VAR);
    Cmd_AddCommandInternal("-throw", IN_Throw_Up, &IN_Throw_Up_VAR);
    Cmd_AddCommandInternal("+speed_throw", IN_Speed_Throw_Down, &IN_Speed_Throw_Down_VAR);
    Cmd_AddCommandInternal("-speed_throw", IN_Speed_Throw_Up, &IN_Speed_Throw_Up_VAR);
    Cmd_AddCommandInternal("+toggleads_throw", IN_ToggleADS_Throw_Down, &IN_ToggleADS_Throw_Down_VAR);
    Cmd_AddCommandInternal("-toggleads_throw", IN_ToggleADS_Throw_Up, &IN_ToggleADS_Throw_Up_VAR);
    Cmd_AddCommandInternal("+gas", IN_Gas_Down, &IN_Gas_Down_VAR);
    Cmd_AddCommandInternal("-gas", IN_Gas_Up, &IN_Gas_Up_VAR);
    Cmd_AddCommandInternal("+reverse", IN_Reverse_Down, &IN_Reverse_Down_VAR);
    Cmd_AddCommandInternal("-reverse", IN_Reverse_Up, &IN_Reverse_Up_VAR);
    Cmd_AddCommandInternal("+handbrake", IN_Handbrake_Down, &IN_Handbrake_Down_VAR);
    Cmd_AddCommandInternal("-handbrake", IN_Handbrake_Up, &IN_Handbrake_Up_VAR);
    Cmd_AddCommandInternal("+switchseat", IN_SwitchSeat_Down, &IN_SwitchSeat_Down_VAR);
    Cmd_AddCommandInternal("-switchseat", IN_SwitchSeat_Up, &IN_SwitchSeat_Up_VAR);
    Cmd_AddCommandInternal("+vehicleattack", IN_VehicleAttack_Down, &IN_VehicleAttack_Down_VAR);
    Cmd_AddCommandInternal("-vehicleattack", IN_VehicleAttack_Up, &IN_VehicleAttack_Up_VAR);
    Cmd_AddCommandInternal("+vehicleboost", IN_VehicleBoost_Down, &IN_VehicleBoost_Down_VAR);
    Cmd_AddCommandInternal("-vehicleboost", IN_VehicleBoost_Up, &IN_VehicleBoost_Up_VAR);
    Cmd_AddCommandInternal("+vehicleattacksecond", IN_VehicleAttackSecond_Down, &IN_VehicleAttackSecond_Down_VAR);
    Cmd_AddCommandInternal("-vehicleattacksecond", IN_VehicleAttackSecond_Up, &IN_VehicleAttackSecond_Up_VAR);
    Cmd_AddCommandInternal("+vehiclemoveup", IN_VehicleMoveUp_Down, &IN_VehicleMoveUp_Down_VAR);
    Cmd_AddCommandInternal("-vehiclemoveup", IN_VehicleMoveUp_Up, &IN_VehicleMoveUp_Up_VAR);
    Cmd_AddCommandInternal("+vehiclemovedown", IN_VehicleMoveDown_Down, &IN_VehicleMoveDown_Down_VAR);
    Cmd_AddCommandInternal("-vehiclemovedown", IN_VehicleMoveDown_Up, &IN_VehicleMoveDown_Up_VAR);
    Cmd_AddCommandInternal("+vehiclespecialability", IN_VehicleSpecialAbility_Down, &IN_VehicleSpecialAbility_Down_VAR);
    Cmd_AddCommandInternal("-vehiclespecialability", IN_VehicleSpecialAbility_Up, &IN_VehicleSpecialAbility_Up_VAR);
    Cmd_AddCommandInternal("+vehiclefirepickup", IN_VehicleFirePickup_Down, &IN_VehicleFirePickup_Down_VAR);
    Cmd_AddCommandInternal("-vehiclefirepickup", IN_VehicleFirePickup_Up, &IN_VehicleFirePickup_Up_VAR);
    Cmd_AddCommandInternal("+vehicleswappickup", IN_VehicleSwapPickup_Down, &IN_VehicleSwapPickup_Down_VAR);
    Cmd_AddCommandInternal("-vehicleswappickup", IN_VehicleSwapPickup_Up, &IN_VehicleSwapPickup_Up_VAR);
    Cmd_AddCommandInternal("+vehicledropdeployable", IN_VehicleDropDeployable_Down, &IN_VehicleDropDeployable_Down_VAR);
    Cmd_AddCommandInternal("-vehicledropdeployable", IN_VehicleDropDeployable_Up, &IN_VehicleDropDeployable_Up_VAR);
    Cmd_AddCommandInternal("lowerstance", IN_LowerStance, &IN_LowerStance_VAR);
    Cmd_AddCommandInternal("raisestance", IN_RaiseStance, &IN_RaiseStance_VAR);
    Cmd_AddCommandInternal("togglecrouch", IN_ToggleCrouch, &IN_ToggleCrouch_VAR);
    Cmd_AddCommandInternal("toggleprone", IN_ToggleProne, &IN_ToggleProne_VAR);
    Cmd_AddCommandInternal("goprone", IN_GoProne, &IN_GoProne_VAR);
    Cmd_AddCommandInternal("gocrouch", IN_GoCrouch, &IN_GoCrouch_VAR);
    Cmd_AddCommandInternal("+gostand", IN_GoStandDown, &IN_GoStandDown_VAR);
    Cmd_AddCommandInternal("-gostand", IN_GoStandUp, &IN_GoStandUp_VAR);
    Cmd_AddCommandInternal("+specNext", IN_SpecNext_Down, &IN_SpecNext_Down_VAR);
    Cmd_AddCommandInternal("-specNext", IN_SpecNext_Up, &IN_SpecNext_Up_VAR);
    Cmd_AddCommandInternal("+specPrev", IN_SpecPrev_Down, &IN_SpecPrev_Down_VAR);
    Cmd_AddCommandInternal("-specPrev", IN_SpecPrev_Up, &IN_SpecPrev_Up_VAR);
    Cmd_AddCommandInternal("+toggleSpec", IN_ToggleSpec_Down, &IN_ToggleSpec_Down_VAR);
    Cmd_AddCommandInternal("-toggleSpec", IN_ToggleSpec_Up, &IN_ToggleSpec_Up_VAR);
    Cmd_AddCommandInternal("toggleView", IN_ToggleView, &IN_ToggleView_VAR);
    Cmd_AddCommandInternal("+talk", IN_TalkDown, &IN_TalkDown_VAR);
    Cmd_AddCommandInternal("-talk", IN_TalkUp, &IN_TalkUp_VAR);
    Cmd_AddCommandInternal("+sprint", IN_SprintDown, &IN_SprintDown_VAR);
    Cmd_AddCommandInternal("-sprint", IN_SprintUp, &IN_SprintUp_VAR);
    cl_analog_attack_threshold = _Dvar_RegisterFloat(
                                                                 "cl_analog_attack_threshold",
                                                                 0.80000001,
                                                                 0.000099999997,
                                                                 1.0,
                                                                 0,
                                                                 "The threshold before firing");
    cl_stanceHoldTime = _Dvar_RegisterInt(
                                                "cl_stanceHoldTime",
                                                300,
                                                0,
                                                1000,
                                                0,
                                                "The time to hold the stance button before the player goes prone");
    cl_dtpHoldTime = _Dvar_RegisterInt(
                                         "cl_dtpHoldTime",
                                         200,
                                         0,
                                         1000,
                                         0,
                                         "The time to hold the stance button while sprinting before the player dives to prone");
    cl_nodelta = _Dvar_RegisterBool("cl_nodelta", 0, 0, "The server does not send snapshot deltas");
    cl_inputTimeScaleFrac = _Dvar_RegisterFloat(
                                                        "cl_inputTimeScaleFrac",
                                                        0.0,
                                                        0.0,
                                                        1.0,
                                                        0x40u,
                                                        "Set how much of the time scale to use in the view angles turn rate");
    cl_dblTapMaxHoldTime = _Dvar_RegisterInt(
                                                     "cl_dblTapMaxHoldTime",
                                                     500,
                                                     0,
                                                     5000,
                                                     0,
                                                     "The maximum amount of time (ms) the player can hold the button to be considered a double tap");
    cl_dblTapMaxDelayTime = _Dvar_RegisterInt(
                                                        "cl_dblTapMaxDelayTime",
                                                        500,
                                                        0,
                                                        5000,
                                                        0,
                                                        "The maximum amount of time (ms) between button presses to be considered a double tap");
}

void __cdecl IN_MLookDown()
{
    playersKb[0][13].active = 1;
}

void __cdecl IN_MLookUp()
{
    if ( CL_GetLocalClientConnectionState(0) >= 6 )
    {
        playersKb[0][13].active = 0;
        if ( !cl_freelook->current.enabled )
            IN_CenterView();
    }
}

void __cdecl IN_UpDown()
{
    StanceState stance; // [esp+0h] [ebp-10h]
    bool v1; // [esp+4h] [ebp-Ch]
    clientActive_t *LocalClientGlobals; // [esp+8h] [ebp-8h]

    IN_KeyDown(&playersKb[0][12]);
    v1 = playersKb[0][27].active || playersKb[0][11].active;
    if ( !v1 && CL_GetLocalClientConnectionState(0) >= 6 )
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(0);
        stance = LocalClientGlobals->stance;
        if ( stance == CL_STANCE_CROUCH )
        {
            LocalClientGlobals->stance = CL_STANCE_STAND;
        }
        else if ( stance == CL_STANCE_PRONE )
        {
            LocalClientGlobals->stance = CL_STANCE_CROUCH;
        }
        else
        {
            if ( LocalClientGlobals->stance )
            {
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_input_mp.cpp",
                                388,
                                1,
                                "%s",
                                "cl->stance == CL_STANCE_STAND") )
                    __debugbreak();
            }
            IN_KeyDown(&playersKb[0][10]);
        }
    }
}

void __cdecl IN_KeyDown(kbutton_t *b)
{
    const char *c; // [esp+18h] [ebp-8h]
    const char *ca; // [esp+18h] [ebp-8h]
    const char *cb; // [esp+18h] [ebp-8h]
    int k; // [esp+1Ch] [ebp-4h]

    c = Cmd_Argv(1);
    if ( *c )
        k = atoi(c);
    else
        k = -1;
    ca = Cmd_Argv(3);
    if ( *ca )
        b->val = atof(ca);
    else
        b->val = 1.0f;
    if ( k != b->down[0] && k != b->down[1] )
    {
        if ( b->down[0] )
        {
            if ( b->down[1] )
            {
                Com_Printf(14, "Three keys down for a button!\n");
                return;
            }
            b->down[1] = k;
        }
        else
        {
            b->down[0] = k;
        }
        if ( !b->active )
        {
            cb = Cmd_Argv(2);
            b->downtime = atoi(cb);
            b->active = 1;
            b->wasPressed = 1;
        }
    }
}

void __cdecl IN_UpUp()
{
    IN_KeyUp(&playersKb[0][12]);
    IN_KeyUp(&playersKb[0][10]);
}

void __cdecl IN_KeyUp(kbutton_t *b)
{
    unsigned int v1; // edx
    const char *c; // [esp+10h] [ebp-Ch]
    const char *ca; // [esp+10h] [ebp-Ch]
    unsigned int uptime; // [esp+14h] [ebp-8h]
    int k; // [esp+18h] [ebp-4h]

    c = Cmd_Argv(1);
    if ( !*c )
    {
        b->down[1] = 0;
        b->down[0] = 0;
        b->active = 0;
        return;
    }
    k = atoi(c);
    if ( b->down[0] == k )
    {
        b->down[0] = 0;
    }
    else
    {
        if ( b->down[1] != k )
            return;
        b->down[1] = 0;
    }
    if ( !b->down[0] && !b->down[1] )
    {
        b->active = 0;
        ca = Cmd_Argv(2);
        uptime = atoi(ca);
        if ( uptime )
            v1 = b->msec + uptime - b->downtime;
        else
            v1 = b->msec + (frame_msec >> 1);
        b->msec = v1;
        b->val = 0.0f;
        b->active = 0;
    }
}

void __cdecl IN_DownDown()
{
    IN_KeyDown(&playersKb[0][11]);
}

void __cdecl IN_DownUp()
{
    IN_KeyUp(&playersKb[0][11]);
}

void __cdecl IN_LeftDown()
{
    IN_KeyDown(playersKb[0]);
}

void __cdecl IN_LeftUp()
{
    IN_KeyUp(playersKb[0]);
}

void __cdecl IN_RightDown()
{
    IN_KeyDown(&playersKb[0][1]);
}

void __cdecl IN_RightUp()
{
    IN_KeyUp(&playersKb[0][1]);
}

void __cdecl IN_ForwardDown()
{
    IN_KeyDown(&playersKb[0][2]);
}

void __cdecl IN_ForwardUp()
{
    IN_KeyUp(&playersKb[0][2]);
}

void __cdecl IN_BackDown()
{
    IN_KeyDown(&playersKb[0][3]);
}

void __cdecl IN_BackUp()
{
    IN_KeyUp(&playersKb[0][3]);
}

void __cdecl IN_LookupDown()
{
    IN_KeyDown(&playersKb[0][4]);
}

void __cdecl IN_LookupUp()
{
    IN_KeyUp(&playersKb[0][4]);
}

void __cdecl IN_LookdownDown()
{
    IN_KeyDown(&playersKb[0][5]);
}

void __cdecl IN_LookdownUp()
{
    IN_KeyUp(&playersKb[0][5]);
}

void __cdecl IN_MoveleftDown()
{
    IN_KeyDown(&playersKb[0][6]);
}

void __cdecl IN_MoveleftUp()
{
    IN_KeyUp(&playersKb[0][6]);
}

void __cdecl IN_MoverightDown()
{
    IN_KeyDown(&playersKb[0][7]);
}

void __cdecl IN_MoverightUp()
{
    IN_KeyUp(&playersKb[0][7]);
}

void __cdecl IN_SpeedDown()
{
    IN_LeaveADS();
    IN_KeyDown(&playersKb[0][9]);
}

void __cdecl IN_LeaveADS()
{
    if ( CL_GetLocalClientConnectionState(0) >= 6 )
        CL_GetLocalClientGlobals(0)->usingAds = 0;
}

void __cdecl IN_SpeedUp()
{
    IN_KeyUp(&playersKb[0][9]);
}

void __cdecl IN_StrafeDown()
{
    IN_KeyDown(&playersKb[0][8]);
}

void __cdecl IN_StrafeUp()
{
    IN_KeyUp(&playersKb[0][8]);
}

void __cdecl IN_Attack_Down()
{
    int v0; // [esp+0h] [ebp-1Ch]
    int v1; // [esp+8h] [ebp-14h]
    clientActive_t *client; // [esp+Ch] [ebp-10h]
    signed int curTime; // [esp+14h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+18h] [ebp-4h]

    client = CL_GetLocalClientGlobals(0);
    curTime = Sys_Milliseconds();
    if ( !client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_input_mp.cpp", 546, 0, "%s", "client") )
    {
        __debugbreak();
    }
    if ( client->snap.valid )
    {
        weapDef = BG_GetWeaponDef(client->snap.ps.weapon);
        if ( weapDef && !gAttackEdgeDetected[0] && curTime - client->lastFireTime < weapDef->iJamFireTime )
        {
            v1 = (int)(float)((float)(60000.0 / (float)(curTime - client->lastFireTime))
                                            - (float)(60000.0 / (float)weapDef->iJamFireTime)) < 100
                 ? (int)(float)((float)(60000.0 / (float)(curTime - client->lastFireTime))
                                            - (float)(60000.0 / (float)weapDef->iJamFireTime))
                 : 100;
            v0 = v1 > 0 ? v1 : 0;
            if ( v0 > curTime % 100 )
                IN_KeyDown(&playersKb[0][45]);
        }
        if ( !gAttackEdgeDetected[0] )
        {
            gAttackEdgeDetected[0] = 1;
            client->lastFireTime = curTime;
        }
    }
    IN_KeyDown(&playersKb[0][17]);
}

void __cdecl IN_Attack_Up()
{
    gAttackEdgeDetected[0] = 0;
    IN_KeyUp(&playersKb[0][17]);
    IN_KeyUp(&playersKb[0][45]);
}

void __cdecl IN_Breath_Down()
{
    IN_KeyDown(&playersKb[0][18]);
}

void __cdecl IN_Breath_Up()
{
    IN_KeyUp(&playersKb[0][18]);
}

void __cdecl IN_MeleeBreath_Down()
{
    IN_KeyDown(&playersKb[0][21]);
    IN_KeyDown(&playersKb[0][18]);
}

void __cdecl IN_MeleeBreath_Up()
{
    IN_KeyUp(&playersKb[0][21]);
    IN_KeyUp(&playersKb[0][18]);
}

void __cdecl IN_Frag_Down()
{
    IN_KeyDown(&playersKb[0][19]);
}

void __cdecl IN_Frag_Up()
{
    IN_KeyUp(&playersKb[0][19]);
}

void __cdecl IN_Smoke_Down()
{
    if ( !playersKb[0][24].active )
    {
        CG_GetLocalClientGlobals(0);
        IN_KeyDown(&playersKb[0][20]);
    }
}

void __cdecl IN_Smoke_Up()
{
    CG_GetLocalClientGlobals(0);
    IN_KeyUp(&playersKb[0][20]);
}

void __cdecl IN_Gas_Down()
{
    IN_KeyDown(&playersKb[0][32]);
}

void __cdecl IN_Gas_Up()
{
    IN_KeyUp(&playersKb[0][32]);
}

void __cdecl IN_Reverse_Down()
{
    IN_KeyDown(&playersKb[0][33]);
}

void __cdecl IN_Reverse_Up()
{
    IN_KeyUp(&playersKb[0][33]);
}

void __cdecl IN_Handbrake_Down()
{
    IN_KeyDown(&playersKb[0][34]);
}

void __cdecl IN_Handbrake_Up()
{
    IN_KeyUp(&playersKb[0][34]);
}

void __cdecl IN_SwitchSeat_Down()
{
    IN_KeyDown(&playersKb[0][35]);
}

void __cdecl IN_SwitchSeat_Up()
{
    IN_KeyUp(&playersKb[0][35]);
}

void __cdecl IN_VehicleBoost_Down()
{
    IN_KeyDown(&playersKb[0][44]);
}

void __cdecl IN_VehicleBoost_Up()
{
    IN_KeyUp(&playersKb[0][44]);
}

void __cdecl IN_VehicleAttack_Down()
{
    IN_KeyDown(&playersKb[0][36]);
}

void __cdecl IN_VehicleAttack_Up()
{
    IN_KeyUp(&playersKb[0][36]);
}

void __cdecl IN_VehicleAttackSecond_Down()
{
    IN_KeyDown(&playersKb[0][37]);
}

void __cdecl IN_VehicleAttackSecond_Up()
{
    IN_KeyUp(&playersKb[0][37]);
}

void __cdecl IN_VehicleMoveUp_Down()
{
    IN_KeyDown(&playersKb[0][38]);
}

void __cdecl IN_VehicleMoveUp_Up()
{
    IN_KeyUp(&playersKb[0][38]);
}

void __cdecl IN_VehicleMoveDown_Down()
{
    IN_KeyDown(&playersKb[0][39]);
}

void __cdecl IN_VehicleMoveDown_Up()
{
    IN_KeyUp(&playersKb[0][39]);
}

void __cdecl IN_VehicleSpecialAbility_Down()
{
    IN_KeyDown(&playersKb[0][40]);
}

void __cdecl IN_VehicleSpecialAbility_Up()
{
    IN_KeyUp(&playersKb[0][40]);
}

void __cdecl IN_VehicleFirePickup_Down()
{
    IN_KeyDown(&playersKb[0][41]);
}

void __cdecl IN_VehicleFirePickup_Up()
{
    IN_KeyUp(&playersKb[0][41]);
}

void __cdecl IN_VehicleSwapPickup_Down()
{
    IN_KeyDown(&playersKb[0][42]);
}

void __cdecl IN_VehicleSwapPickup_Up()
{
    IN_KeyUp(&playersKb[0][42]);
}

void __cdecl IN_VehicleDropDeployable_Down()
{
    IN_KeyDown(&playersKb[0][43]);
}

void __cdecl IN_VehicleDropDeployable_Up()
{
    IN_KeyUp(&playersKb[0][43]);
}

void __cdecl IN_BreathSprint_Down()
{
    IN_KeyDown(&playersKb[0][18]);
    IN_KeyDown(&playersKb[0][30]);
}

void __cdecl IN_BreathSprint_Up()
{
    IN_KeyUp(&playersKb[0][18]);
    IN_KeyUp(&playersKb[0][30]);
}

void __cdecl IN_Melee_Down()
{
    IN_KeyDown(&playersKb[0][21]);
}

void __cdecl IN_Melee_Up()
{
    IN_KeyUp(&playersKb[0][21]);
}

void __cdecl IN_Activate_Down()
{
    IN_UpdateUseHeld();
    IN_KeyDown(&playersKb[0][22]);
}

void __cdecl IN_Activate_Up()
{
    IN_UpdateUseCount();
    IN_KeyUp(&playersKb[0][22]);
}

void __cdecl IN_Reload_Down()
{
    IN_UpdateUseHeld();
    IN_KeyDown(&playersKb[0][23]);
}

void __cdecl IN_Reload_Up()
{
    IN_UpdateUseCount();
    IN_KeyUp(&playersKb[0][23]);
}

void __cdecl IN_UseReload_Down()
{
    IN_UpdateUseHeld();
    IN_KeyDown(&playersKb[0][24]);
}

void __cdecl IN_UseReload_Up()
{
    IN_UpdateUseCount();
    IN_KeyUp(&playersKb[0][24]);
}

void __cdecl IN_LeanLeft_Down()
{
    IN_KeyDown(&playersKb[0][25]);
}

void __cdecl IN_LeanLeft_Up()
{
    IN_KeyUp(&playersKb[0][25]);
}

void __cdecl IN_LeanRight_Down()
{
    IN_KeyDown(&playersKb[0][26]);
}

void __cdecl IN_LeanRight_Up()
{
    IN_KeyUp(&playersKb[0][26]);
}

void __cdecl IN_Prone_Down()
{
    IN_KeyDown(&playersKb[0][27]);
}

void __cdecl IN_Prone_Up()
{
    IN_KeyUp(&playersKb[0][27]);
}

void __cdecl IN_Stance_Down()
{
    clientActive_t *LocalClientGlobals; // [esp+4h] [ebp-8h]

    if ( CL_GetLocalClientConnectionState(0) >= 6 )
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(0);
        if ( (LocalClientGlobals->snap.ps.eFlags & 0x4000) != 0 )
        {
            IN_DownDown();
        }
        else if ( !playersKb[0][27].active && !playersKb[0][11].active )
        {
            if ( CG_IsSprinting(0) )
            {
                LocalClientGlobals->stanceHeld = 1;
                LocalClientGlobals->stanceTime = com_frameTime;
            }
            else
            {
                LocalClientGlobals->stanceHeld = 1;
                LocalClientGlobals->stancePosition = LocalClientGlobals->stance;
                LocalClientGlobals->stanceTime = com_frameTime;
                if ( LocalClientGlobals->stancePosition != CL_STANCE_CROUCH )
                    LocalClientGlobals->stance = CL_STANCE_CROUCH;
            }
        }
    }
}

void __cdecl IN_Stance_Up()
{
    clientActive_t *LocalClientGlobals; // [esp+4h] [ebp-8h]

    if ( CL_GetLocalClientConnectionState(0) >= 6 )
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(0);
        if ( (LocalClientGlobals->snap.ps.eFlags & 0x4000) != 0 )
        {
            IN_DownUp();
        }
        else if ( !playersKb[0][27].active && !playersKb[0][11].active )
        {
            if ( CG_IsSprinting(0) )
            {
                LocalClientGlobals->stance = CL_STANCE_CROUCH;
                LocalClientGlobals->stanceHeld = 0;
            }
            if ( LocalClientGlobals->stanceHeld && LocalClientGlobals->stancePosition == CL_STANCE_CROUCH )
                LocalClientGlobals->stance = CL_STANCE_STAND;
            LocalClientGlobals->stanceHeld = 0;
        }
    }
}

void __cdecl IN_ToggleADS()
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-8h]

    if ( CL_GetLocalClientConnectionState(0) >= 6 )
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(0);
        if ( playersKb[0][29].val >= 0.1 )
        {
            if ( !latch[0] )
            {
                LocalClientGlobals->usingAds = !LocalClientGlobals->usingAds;
                latch[0] = 1;
            }
        }
        else
        {
            latch[0] = 0;
        }
    }
}

void __cdecl IN_Throw_Down()
{
    IN_KeyDown(&playersKb[0][29]);
}

void __cdecl IN_Throw_Up()
{
    IN_KeyUp(&playersKb[0][29]);
}

void __cdecl IN_ToggleADS_Throw_Down()
{
    IN_Throw_Down();
    IN_ToggleADS();
}

void __cdecl IN_ToggleADS_Throw_Up()
{
    IN_Throw_Up();
    IN_ToggleADS();
}

void __cdecl IN_Speed_Throw_Down()
{
    IN_SpeedDown();
    IN_Throw_Down();
}

void __cdecl IN_Speed_Throw_Up()
{
    IN_SpeedUp();
    IN_Throw_Up();
}

void __cdecl IN_LowerStance()
{
    StanceState stance; // [esp+0h] [ebp-10h]
    clientActive_t *LocalClientGlobals; // [esp+8h] [ebp-8h]

    if ( CL_GetLocalClientConnectionState(0) >= 6 && !playersKb[0][27].active && !playersKb[0][11].active )
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(0);
        stance = LocalClientGlobals->stance;
        if ( stance )
        {
            if ( stance == CL_STANCE_CROUCH )
            {
                LocalClientGlobals->stance = CL_STANCE_PRONE;
            }
            else if ( LocalClientGlobals->stance != CL_STANCE_PRONE
                         && !Assert_MyHandler(
                                     "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_input_mp.cpp",
                                     1195,
                                     1,
                                     "%s",
                                     "cl->stance == CL_STANCE_PRONE") )
            {
                __debugbreak();
            }
        }
        else
        {
            LocalClientGlobals->stance = CL_STANCE_CROUCH;
        }
    }
}

void __cdecl IN_RaiseStance()
{
    StanceState stance; // [esp+0h] [ebp-10h]
    clientActive_t *LocalClientGlobals; // [esp+8h] [ebp-8h]

    if ( CL_GetLocalClientConnectionState(0) >= 6 && !playersKb[0][27].active && !playersKb[0][11].active )
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(0);
        stance = LocalClientGlobals->stance;
        if ( stance == CL_STANCE_CROUCH )
        {
            LocalClientGlobals->stance = CL_STANCE_STAND;
        }
        else if ( stance == CL_STANCE_PRONE )
        {
            LocalClientGlobals->stance = CL_STANCE_CROUCH;
        }
        else if ( LocalClientGlobals->stance )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_input_mp.cpp",
                            1224,
                            1,
                            "%s",
                            "cl->stance == CL_STANCE_STAND") )
                __debugbreak();
        }
    }
}

void __cdecl IN_ToggleCrouch()
{
    CL_ToggleStance(0, CL_STANCE_CROUCH);
}

void __cdecl CL_ToggleStance(int localClientNum, StanceState preferredStance)
{
    clientActive_t *LocalClientGlobals; // ecx

    if ( !playersKb[localClientNum][27].active && !playersKb[localClientNum][11].active )
    {
        if ( CG_IsSprinting(localClientNum) && preferredStance == CL_STANCE_PRONE )
            preferredStance = CL_STANCE_DIVE_TO_PRONE;
        LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
        LocalClientGlobals->stance = preferredStance != LocalClientGlobals->stance ? preferredStance : CL_STANCE_STAND;
    }
}

void __cdecl IN_ToggleProne()
{
    CL_ToggleStance(0, CL_STANCE_PRONE);
}

void __cdecl IN_GoProne()
{
    CL_SetStance(0, CL_STANCE_PRONE);
}

void __cdecl IN_GoCrouch()
{
    CL_SetStance(0, CL_STANCE_CROUCH);
}

void __cdecl IN_GoStandDown()
{
    if ( CL_GetLocalClientConnectionState(0) >= 6 )
    {
        IN_KeyDown(&playersKb[0][12]);
        if ( CL_GetLocalClientGlobals(0)->stance )
            CL_SetStance(0, CL_STANCE_STAND);
        else
            IN_KeyDown(&playersKb[0][10]);
    }
}

void __cdecl IN_GoStandUp()
{
    if ( CL_GetLocalClientConnectionState(0) >= 6 )
    {
        IN_KeyUp(&playersKb[0][12]);
        IN_KeyUp(&playersKb[0][10]);
    }
}

void __cdecl IN_SprintDown()
{
    IN_KeyDown(&playersKb[0][30]);
}

void __cdecl IN_SprintUp()
{
    IN_KeyUp(&playersKb[0][30]);
}

void __cdecl IN_SpecNext_Down()
{
    IN_KeyDown(&playersKb[0][15]);
}

void __cdecl IN_SpecNext_Up()
{
    IN_KeyUp(&playersKb[0][15]);
}

void __cdecl IN_SpecPrev_Down()
{
    IN_KeyDown(&playersKb[0][16]);
}

void __cdecl IN_SpecPrev_Up()
{
    IN_KeyUp(&playersKb[0][16]);
}

void __cdecl IN_ToggleSpec_Down()
{
    IN_KeyDown(&playersKb[0][14]);
}

void __cdecl IN_ToggleSpec_Up()
{
    IN_KeyUp(&playersKb[0][14]);
}

void __cdecl IN_ToggleView()
{
    const cg_s *cgameGlob; // [esp+4h] [ebp-8h]
    bool thirdperson; // [esp+Bh] [ebp-1h]

    thirdperson = cg_thirdPerson->current.integer <= 0;
    if ( (CL_GetLocalClientGlobals(0)->snap.ps.otherFlags & 2) != 0 )
    {
        cgameGlob = CG_GetLocalClientGlobals(0);
        if ( !thirdperson || cgameGlob->inKillCam )
            CG_SetThirdPerson(0);
        else
            CG_SetThirdPerson(1);
    }
}

void __cdecl IN_TalkDown()
{
    IN_KeyDown(&playersKb[0][46]);
}

void __cdecl IN_TalkUp()
{
    IN_KeyUp(&playersKb[0][46]);
}

void __cdecl CL_ShutdownInput()
{
    Cmd_RemoveCommand("mouseMove");
    Cmd_RemoveCommand("remoteKey");
    Cmd_RemoveCommand("centerview");
    Cmd_RemoveCommand("+moveup");
    Cmd_RemoveCommand("-moveup");
    Cmd_RemoveCommand("+movedown");
    Cmd_RemoveCommand("-movedown");
    Cmd_RemoveCommand("+left");
    Cmd_RemoveCommand("-left");
    Cmd_RemoveCommand("+right");
    Cmd_RemoveCommand("-right");
    Cmd_RemoveCommand("+forward");
    Cmd_RemoveCommand("-forward");
    Cmd_RemoveCommand("+back");
    Cmd_RemoveCommand("-back");
    Cmd_RemoveCommand("+lookup");
    Cmd_RemoveCommand("-lookup");
    Cmd_RemoveCommand("+lookdown");
    Cmd_RemoveCommand("-lookdown");
    Cmd_RemoveCommand("+strafe");
    Cmd_RemoveCommand("-strafe");
    Cmd_RemoveCommand("+moveleft");
    Cmd_RemoveCommand("-moveleft");
    Cmd_RemoveCommand("+moveright");
    Cmd_RemoveCommand("-moveright");
    Cmd_RemoveCommand("+speed");
    Cmd_RemoveCommand("-speed");
    Cmd_RemoveCommand("+attack");
    Cmd_RemoveCommand("-attack");
    Cmd_RemoveCommand("+melee");
    Cmd_RemoveCommand("-melee");
    Cmd_RemoveCommand("+holdbreath");
    Cmd_RemoveCommand("-holdbreath");
    Cmd_RemoveCommand("+melee_breath");
    Cmd_RemoveCommand("-melee_breath");
    Cmd_RemoveCommand("+frag");
    Cmd_RemoveCommand("-frag");
    Cmd_RemoveCommand("+smoke");
    Cmd_RemoveCommand("-smoke");
    Cmd_RemoveCommand("+breath_sprint");
    Cmd_RemoveCommand("-breath_sprint");
    Cmd_RemoveCommand("+activate");
    Cmd_RemoveCommand("-activate");
    Cmd_RemoveCommand("+reload");
    Cmd_RemoveCommand("-reload");
    Cmd_RemoveCommand("+usereload");
    Cmd_RemoveCommand("-usereload");
    Cmd_RemoveCommand("+leanleft");
    Cmd_RemoveCommand("-leanleft");
    Cmd_RemoveCommand("+leanright");
    Cmd_RemoveCommand("-leanright");
    Cmd_RemoveCommand("+prone");
    Cmd_RemoveCommand("-prone");
    Cmd_RemoveCommand("+stance");
    Cmd_RemoveCommand("-stance");
    Cmd_RemoveCommand("+mlook");
    Cmd_RemoveCommand("-mlook");
    Cmd_RemoveCommand("toggleads");
    Cmd_RemoveCommand("leaveads");
    Cmd_RemoveCommand("+throw");
    Cmd_RemoveCommand("-throw");
    Cmd_RemoveCommand("+speed_throw");
    Cmd_RemoveCommand("-speed_throw");
    Cmd_RemoveCommand("+toggleads_throw");
    Cmd_RemoveCommand("-toggleads_throw");
    Cmd_RemoveCommand("+gas");
    Cmd_RemoveCommand("-gas");
    Cmd_RemoveCommand("+reverse");
    Cmd_RemoveCommand("-reverse");
    Cmd_RemoveCommand("+handbrake");
    Cmd_RemoveCommand("-handbrake");
    Cmd_RemoveCommand("lowerstance");
    Cmd_RemoveCommand("raisestance");
    Cmd_RemoveCommand("togglecrouch");
    Cmd_RemoveCommand("toggleprone");
    Cmd_RemoveCommand("goprone");
    Cmd_RemoveCommand("gocrouch");
    Cmd_RemoveCommand("+gostand");
    Cmd_RemoveCommand("-gostand");
    Cmd_RemoveCommand("+sprint");
    Cmd_RemoveCommand("-sprint");
    Cmd_RemoveCommand("+red");
    Cmd_RemoveCommand("-red");
    Cmd_RemoveCommand("+green");
    Cmd_RemoveCommand("-green");
    Cmd_RemoveCommand("+blue");
    Cmd_RemoveCommand("-blue");
    Cmd_RemoveCommand("+yellow");
    Cmd_RemoveCommand("-yellow");
    Cmd_RemoveCommand("+specNext");
    Cmd_RemoveCommand("-specNext");
    Cmd_RemoveCommand("+specPrev");
    Cmd_RemoveCommand("-specPrev");
    Cmd_RemoveCommand("+toggleSpec");
    Cmd_RemoveCommand("-toggleSpec");
    Cmd_RemoveCommand("toggleView");
    Cmd_RemoveCommand("+talk");
    Cmd_RemoveCommand("-talk");
}

void __cdecl CL_ClearKeys(int localClientNum)
{
    memset(playersKb[localClientNum], 0, sizeof(kbutton_t[47]));
}

