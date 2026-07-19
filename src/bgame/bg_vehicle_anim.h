#pragma once

enum VehicleSeatPositions : __int32
{                                       // XREF: vehAnimClient_t/r
                                        // vehAnimClient_t/r
    VEH_POS_NONE          = -1,
    VEH_POS_DRIVER        = 0x0,
    VEH_POS_MIN_GUNNER    = 0x1,
    VEH_POS_GUNNER_1      = 0x1,
    VEH_POS_GUNNER_2      = 0x2,
    VEH_POS_GUNNER_3      = 0x3,
    VEH_POS_GUNNER_4      = 0x4,
    VEH_POS_MAX_GUNNER    = 0x4,
    VEH_POS_MIN_PASSENGER = 0x5,
    VEH_POS_PASSENGER_1   = 0x5,
    VEH_POS_PASSENGER_2   = 0x6,
    VEH_POS_PASSENGER_3   = 0x7,
    VEH_POS_PASSENGER_4   = 0x8,
    VEH_POS_PASSENGER_5   = 0x9,
    VEH_POS_PASSENGER_6   = 0xA,
    VEH_POS_MAX_PASSENGER = 0xA,
    NUM_VEHICLE_POSITIONS = 0xB,
};

struct vehAnimClient_t // sizeof=0x1C
{                                       // XREF: .data:clVehAnimClients/r
    bool initialized;                   // XREF: VehAnim_Init(void)+5/w
    // padding byte
    // padding byte
    // padding byte
    VehicleSeatPositions curSeat;       // XREF: VehAnim_Init(void)+A/w
    VehicleSeatPositions prevSeat;      // XREF: VehAnim_Init(void)+F/w
    bool moving;                        // XREF: VehAnim_Init(void)+14/w
    // padding byte
    // padding byte
    // padding byte
    int animStage;                      // XREF: VehAnim_Init(void)+19/w
    int animIndex;                      // XREF: VehAnim_Init(void)+1E/w
    int tagName;                        // XREF: VehAnim_Init(void)+23/w
};

struct entityState_s;
struct clientInfo_t;
struct XAnimTree_s;


void __cdecl VehAnim_RegisterDvars();
void __cdecl VehAnim_Init();
void __cdecl VehAnim_UpdatePosRot(
                int localClientNum,
                const entityState_s *es,
                clientInfo_t *ci,
                int handler,
                float *origin,
                float *angles);
void __cdecl VehAnim_GetAnimDelta(XAnimTree_s *tree, unsigned int animIndex, float *rot, float *trans);
