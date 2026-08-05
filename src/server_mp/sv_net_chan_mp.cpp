#include "sv_net_chan_mp.h"
#include "sv_main_mp.h"

bool __cdecl SV_Netchan_TransmitNextFragment(client_t *client, netchan_t *chan)
{
    bool res; // [esp+3h] [ebp-1h]

    res = Netchan_TransmitNextFragment(chan);
    if ( !chan->unsentFragments )
        SV_Netchan_OutgoingSequenceIncremented(client, chan);
    return res;
}

void __cdecl SV_Netchan_OutgoingSequenceIncremented(client_t *client, netchan_t *chan)
{
    clientSnapshot_t *frame; // [esp+0h] [ebp-4h]

    frame = &client->frames[chan->outgoingSequence & 0x1F];
    memset(frame, 0, sizeof(clientSnapshot_t));
    frame->matchState = svs.nextSnapshotMatchStates;
    frame->first_entity = svs.nextSnapshotEntities;
    frame->first_client = svs.nextSnapshotClients;
}

bool __cdecl SV_Netchan_Transmit(client_t *client, unsigned __int8 *data, int length, bool reliable)
{
    bool res; // [esp+3h] [ebp-1h]

    res = Netchan_Transmit(&client->header.netchan, length, (char *)data, reliable);
    if ( !client->header.netchan.unsentFragments )
        SV_Netchan_OutgoingSequenceIncremented(client, &client->header.netchan);
    return res;
}

void __cdecl SV_Netchan_AddOOBProfilePacket(int iLength)
{
    if ( net_profile->current.integer )
    {
        NetProf_PrepProfiling(&svs.OOBProf);
        NetProf_AddPacket(&svs.OOBProf.send, iLength, 0);
    }
}

