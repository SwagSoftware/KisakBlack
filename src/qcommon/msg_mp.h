#pragma once
#include <universal/q_shared.h>
#include <qcommon/bitarray.h>
#include <qcommon/xn_types.h>
#include "ent.h"

enum compressionType_t : unsigned __int8 // LWSS: Not a real enum name
{
    COMPRESSION_TYPE_NONE = 0,
    COMPRESSION_TYPE_HUFFMAN = 1,
    COMPRESSION_TYPE_ZLIB = 2
};

enum netsrc_t : __int32
{                                                                             // XREF: msg_t/r netchan_t/r ...
        NS_CLIENT1        = 0x0,
        NS_SERVER         = 0x1,
        NS_MAXCLIENTS = 0x1,
        NS_PACKET         = 0x2,
};

enum netFieldTypes_t : __int32
{                                       // XREF: ?Demo_GetNetFieldListForType@@YAPBUNetFieldList@@W4netFieldTypes_t@@@Z/r
                                        // ?MSG_GetNetFieldList@@YAPBUNetFieldList@@W4netFieldTypes_t@@_N@Z/r
    NET_FIELD_TYPE_ARCHIVED_ENTITY = 0x0,
    NET_FIELD_TYPE_CLIENT_STATE    = 0x1,
    NET_FIELD_TYPE_PLAYERSTATE     = 0x2,
    NET_FIELD_TYPE_OBJECTIVE       = 0x3,
    NET_FIELD_TYPE_HUDELEMS        = 0x4,
    NET_FIELD_TYPE_MATCHSTATE      = 0x5,
    NET_FIELD_TYPE_COUNT           = 0x6,
};

struct __declspec(align(4)) msg_bookmark_t // sizeof=0x18
{                                                                             // XREF: demoKeyFrame/r
                                                                                // ?Demo_ProcessPlayback@@YAXXZ/r
        int overflowed;
        int cursize;
        int bit;
        int lastEntityRef;
        int readcount;
        unsigned __int8 bitByte;
        // padding byte
        // padding byte
        // padding byte
};

struct msg_t // sizeof=0x30
{                                                                             // XREF: .data:msg_t fakemsg/r
    int overflowed;                                         // XREF: BG_EmblemsWriteString(short,CompositeEmblemLayer *,int)+44/r
    int readOnly;
    unsigned __int8 *data;                            // XREF: BG_EmblemsWriteString(short,CompositeEmblemLayer *,int)+88/r
    unsigned __int8 *splitData;
    int maxsize;                                                // XREF: CL_ReadDemoNetworkPacket+116/r
    int cursize;                                                // XREF: BG_EmblemsWriteString(short,CompositeEmblemLayer *,int)+84/r
    int splitSize;
    int readcount;                                            // XREF: CL_ReadDemoNetworkPacket+180/w
    int bit;                                                        // XREF: SV_SendClientVoiceData(client_t *):loc_71EFFC/r
    int lastEntityRef;
    int flush;
    netsrc_t targetLocalNetID;
};

struct NetFieldList // sizeof=0xC
{                                       // XREF: .rdata:s_entityNetFieldList/r
    const struct NetField *array;
    unsigned int count;
    const char *fieldArrayName;
};

struct NetField // sizeof=0x1C
{                                                                             // XREF: .rdata:helicopterEntityStateFields/r
                                                                                // .rdata:planeStateFields/r ...
        const char *name;
        int offset;
        int size;
        int bits;
        unsigned __int8 changeHints;
        // padding byte
        // padding byte
        // padding byte
        const char *bitsStr;
        const char *changeHintsStr;
};

struct archivedEntityShared_t // sizeof=0x20
{                                                                             // XREF: archivedEntity_s/r
        int svFlags;                                                // XREF: SV_ArchiveSnapshot(msg_t *)+86A/w
                                                                                // SV_ArchiveSnapshot(msg_t *)+87F/r ...
        int clientMask[1];                                    // XREF: SV_ArchiveSnapshot(msg_t *)+89A/w
        float absmin[3];                                        // XREF: SV_ArchiveSnapshot(msg_t *)+8BC/w
                                                                                // SV_ArchiveSnapshot(msg_t *)+8CF/w ...
        float absmax[3];                                        // XREF: SV_ArchiveSnapshot(msg_t *)+906/w
                                                                                // SV_ArchiveSnapshot(msg_t *)+919/w ...
};

struct archivedEntity_s // sizeof=0x100
{                                                                             // XREF: svEntity_s/r
                                                                                // archivedEntity_t/r ...
        entityState_s s;                                        // XREF: CM_GetWaterHeight(float const * const,float,float)+107/o
                                                                                // CM_UnlinkEntity(svEntity_s *)+75/o ...
        archivedEntityShared_t r;                     // XREF: SV_ArchiveSnapshot(msg_t *)+86A/w
                                                                                // SV_ArchiveSnapshot(msg_t *)+87F/r ...
};

struct __declspec(align(2)) usercmd_s // sizeof=0x34
{                                                                             // XREF: clientSession_t/r
                                                                                // clientSession_t/r ...
        int serverTime;                                         // XREF: CG_CalcVehicleViewValues(int)+1543/r
                                                                                // CG_Calc3rdPersonVehicleViewValues(int)+29F/r ...
        bitarray<51> button_bits;                     // XREF: PM_AirMove+15/o
                                                                                // PM_SwimMove+15/o ...
        int angles[3];                                            // XREF: CG_CheckPlayerMovement+B/o
                                                                                // CG_CheckPlayerMovement+E/o
        unsigned __int16 weapon;
        unsigned __int16 offHandIndex;
        unsigned __int16 lastWeaponAltModeSwitch;
        char forwardmove;                                     // XREF: CG_RadiantCamCalcView(int)+73/r
                                                                                // CG_CheckPlayerMovement:loc_49A9CE/r
        char rightmove;                                         // XREF: CG_RadiantCamCalcView(int)+81/r
                                                                                // CG_CheckPlayerMovement+26/r
        char upmove;
        char pitchmove;                                         // XREF: CG_CalcVehicleViewValues(int)+1516/r
                                                                                // CG_Calc3rdPersonVehicleViewValues(int)+249/r ...
        char yawmove;                                             // XREF: CG_CalcVehicleViewValues(int)+130D/r
                                                                                // CG_CalcVehicleViewValues(int)+131D/r ...
        // padding byte
        float meleeChargeYaw;
        unsigned __int8 meleeChargeDist;
        // padding byte
        // padding byte
        // padding byte
        float rollmove;
        char selectedLocation[2];
        unsigned __int8 selectedYaw;
        // padding byte
};

struct netFieldOrderInfo_t // sizeof=0x79C
{                                       // XREF: .data:netFieldOrderInfo_t orderInfo/r
    int entState[79];
    int arcEntState[10];                // XREF: MSG_DumpNetFieldChanges_f(void)+D/o
    int matchState[17];
    int clientState[52];                // XREF: MSG_DumpNetFieldChanges_f(void)+14/o
    int playerState[179];               // XREF: MSG_DumpNetFieldChanges_f(void)+1B/o
    int actorState[98];
    int objective[9];                   // XREF: MSG_DumpNetFieldChanges_f(void)+22/o
    int hudElem[43];                    // XREF: MSG_DumpNetFieldChanges_f(void)+29/o
};

struct playerState_s;
struct hudelem_s;

float (*__cdecl MSG_GetMapCenter())[3];
int __cdecl GetMinBitCountForNum(unsigned int num);
void __cdecl MSG_Init(msg_t *buf, unsigned __int8 *data, int length);
void __cdecl MSG_InitReadOnly(msg_t *buf, unsigned __int8 *data, int length);
void __cdecl MSG_InitReadOnlySplit(msg_t *buf, unsigned __int8 *data, int length, unsigned __int8 *data2, int length2);
void __cdecl MSG_BeginReading(msg_t *msg);
void __cdecl MSG_Discard(msg_t *msg);
int __cdecl MSG_GetUsedBitCount(const msg_t *msg);
int __cdecl MSG_GetNumBitsRead(const msg_t *msg);
void __cdecl MSG_SetBookmark(const msg_t *msg, msg_bookmark_t *bookmark);
void __cdecl MSG_GotoBookmark(msg_t *msg, const msg_bookmark_t *bookmark);
void __cdecl MSG_WriteBits(msg_t *msg, int value, unsigned int bits);
void __cdecl MSG_WriteBit0(msg_t *msg);
void __cdecl MSG_WriteBit1(msg_t *msg);
int __cdecl MSG_ReadBits(msg_t *msg, unsigned int bits);
int __cdecl MSG_GetByte(msg_t *msg, int where);
int __cdecl MSG_ReadBit(msg_t *msg);
unsigned int __cdecl MSG_CompressWithZLib(
                unsigned __int8 *from,
                unsigned int inSizeBytes,
                unsigned __int8 *to,
                unsigned int outSizeBytes);
int __cdecl MSG_WriteBitsCompress(
                bool trainHuffman,
                unsigned __int8 *from,
                int fromSizeBytes,
                unsigned __int8 *to,
                int toSizeBytes);
void __cdecl showHuffmanData();
unsigned int __cdecl MSG_DecompressWithZLib(
                unsigned __int8 *from,
                unsigned int inSizeBytes,
                unsigned __int8 *to,
                unsigned int outSizeBytes);
unsigned int __cdecl MSG_ReadBitsCompress(
                unsigned __int8 *from,
                int fromSizeBytes,
                unsigned __int8 *to,
                unsigned int toSizeBytes);
void __cdecl MSG_Embed(msg_t *dest, msg_t *src);
void __cdecl MSG_UnEmbed(msg_t *msg);
void __cdecl MSG_WriteByte(msg_t *msg, unsigned __int8 c);
void __cdecl MSG_WriteData(msg_t *buf, unsigned __int8 *data, unsigned int length);
void __cdecl MSG_WriteShort(msg_t *msg, unsigned short c);
void __cdecl MSG_WriteLong(msg_t *msg, int c);
void __cdecl MSG_WriteFloat(msg_t *sb, float f);
void __cdecl MSG_WriteInt64(msg_t *msg, unsigned __int64 c);
void __cdecl MSG_WriteString(msg_t *sb, const char *s);
void __cdecl MSG_WriteBigString(msg_t *sb, char *s);
void __cdecl MSG_WriteAngle(msg_t *sb, unsigned int bits, float f);
void __cdecl MSG_WriteAngle16(msg_t *sb, float f);
void __cdecl MSG_WriteRangedFloatBits(msg_t *sb, float f, float begin, float end, unsigned int bits);
int __cdecl MSG_ReadByte(msg_t *msg);
int __cdecl MSG_ReadShort(msg_t *msg);
int __cdecl MSG_ReadLong(msg_t *msg);
unsigned __int64 __cdecl MSG_ReadInt64(msg_t *msg);
float __cdecl MSG_ReadFloat(msg_t *msg);
char *__cdecl MSG_ReadString(msg_t *msg, char *string, unsigned int maxChars);
char *__cdecl MSG_ReadBigString(msg_t *msg);
char *__cdecl MSG_ReadStringLine(msg_t *msg, char *string, unsigned int maxChars);
double __cdecl MSG_ReadAngle(msg_t *msg, unsigned int bits);
double __cdecl MSG_ReadAngle16(msg_t *msg);
double __cdecl MSG_ReadRangedFloatBits(msg_t *sb, float begin, float end, unsigned int bits);
void __cdecl MSG_ReadData(msg_t *msg, unsigned __int8 *data, int len);
void __cdecl MSG_WriteDeltaKey(msg_t *msg, int key, int oldV, int newV, unsigned int bits);
unsigned int __cdecl MSG_ReadDeltaKey(msg_t *msg, int key, int oldV, unsigned int bits);
void __cdecl MSG_WriteKey(msg_t *msg, int key, int newV, unsigned int bits);
unsigned int __cdecl MSG_ReadKey(msg_t *msg, int key, unsigned int bits);
void __cdecl MSG_WriteDeltaKeyByte(msg_t *msg, char key, char oldV, char newV);
int __cdecl MSG_ReadDeltaKeyByte(msg_t *msg, unsigned __int8 key, int oldV);
void __cdecl MSG_WriteDeltaKeyShort(msg_t *msg, __int16 key, __int16 oldV, __int16 newV);
int __cdecl MSG_ReadDeltaKeyShort(msg_t *msg, __int16 key, int oldV);
void __cdecl MSG_SetDefaultUserCmd(playerState_s *ps, usercmd_s *cmd);
void __cdecl MSG_WriteDeltaUsercmdKey(
                msg_t *msg,
                int key,
                const usercmd_s *from,
                const usercmd_s *to,
                int forceSendAngles);
void __cdecl MSG_ReadDeltaUsercmdKey(msg_t *msg, int key, const usercmd_s *from, usercmd_s *to);
void __cdecl MSG_ClearLastReferencedEntity(msg_t *msg);
double __cdecl MSG_ReadDemoRoundedFloat(
                msg_t *msg,
                int bits,
                float oldValue,
                unsigned int diffBits,
                unsigned int fullBits);
double __cdecl MSG_ReadRangedFloat(
                msg_t *msg,
                float oldValue,
                float begin,
                float end,
                int precision,
                bool oftenZeroOrOne);
int __cdecl MSG_ReadEntityIndex(msg_t *msg, unsigned int indexBits);
void __cdecl MSG_ReadDeltaField(
                msg_t *msg,
                int time,
                char *from,
                char *to,
                const NetField *field,
                int print,
                bool noXor);
int __cdecl MSG_ReadDeltaTime(msg_t *msg, int timeBase);
int __cdecl MSG_ReadDeltaGroundEntity(msg_t *msg);
int __cdecl MSG_ReadDeltaEventField(msg_t *msg);
int __cdecl MSG_ReadDeltaEventParamField(msg_t *msg);
int __cdecl MSG_ReadEFlags(msg_t *msg, int oldFlags);
double __cdecl MSG_ReadOriginFloat(int bits, msg_t *msg, float oldValue);
double __cdecl MSG_ReadOriginZFloat(msg_t *msg, float oldValue);
int __cdecl MSG_ReadDeltaEntityStruct(
                msg_t *msg,
                int time,
                char *from,
                char *to,
                unsigned int number,
                int totalFields,
                int skippedFieldBits);
int __cdecl MSG_ReadDeltaArchivedEntity(
                msg_t *msg,
                int time,
                const archivedEntity_s *from,
                archivedEntity_s *to,
                unsigned int number);
int __cdecl MSG_ReadAppendedDeltaStruct(
                msg_t *msg,
                int time,
                char *from,
                char *to,
                unsigned int number,
                int numFields,
                char indexBits,
                const NetField *stateFields,
                int skippedFieldBits);
void __cdecl MSG_ReadDeltaPlayerstate(
    int localClientNum,
    msg_t *msg,
    int time,
    const playerState_s *from,
    playerState_s *to,
    bool predictedFieldsIgnoreXor);
void __cdecl MSG_ReadDeltaHudElems(msg_t *msg, int time, const hudelem_s *from, hudelem_s *to, int count);
void __cdecl MSG_InitHuffman();
void MSG_initHuffmanInternal();
void __cdecl MSG_DumpNetFieldChanges_f();

// These prolly should use the const int as the array size, but yeah
extern const int numEventEntityStateFields;
extern const NetField eventEntityStateFields[69];
extern const int numActorStateFields;
extern const NetField actorStateFields[69];
extern const int numVehicleEntityStateFields;
extern const NetField vehicleEntityStateFields[71];
extern const NetField planeStateFields[66];
extern const NetField helicopterEntityStateFields[71];
extern const int numTurretEntityStateFields;
extern const NetField turretEntityStateFields[69];
extern const int numLoopFxEntityStateFields;
extern const NetField loopFxEntityStateFields[69];
extern const NetField fxStateFields[68];
extern const int numSoundBlendEntityStateFields;
extern const NetField soundBlendEntityStateFields[68];
extern const NetField scriptMoverStateFields[72];
extern const int numMissileEntityStateFields;
extern const NetField missileEntityStateFields[69];
extern const int numItemEntityStateFields;
extern const NetField itemEntityStateFields[69];
extern const int numCorpseEntityStateFields;
extern const NetField corpseEntityStateFields[68];
extern const int numPlayerEntityStateFields;
extern const NetField playerEntityStateFields[74];
extern const int numArchivedEntityFields;
extern const NetField archivedEntityFields[8];
extern const NetField entityStateFields[69];

extern const int numClientStateFields;
extern const NetField clientStateFields[50];
extern const int numPlayerStateFields;
extern const NetField playerStateFields[179];
extern const int numObjectiveFields;
extern const NetField objectiveFields[9];
extern const int numMatchStateFields;
extern const NetField matchStateFields[15];


// should be const
static NetFieldList s_entityNetFieldList[22] =
{
  { entityStateFields, 69u, "entityStateFields" },
  { playerEntityStateFields, 74u, "playerEntityStateFields" },
  { corpseEntityStateFields, 68u, "corpseEntityStateFields" },
  { itemEntityStateFields, 69u, "itemEntityStateFields" },
  { missileEntityStateFields, 69u, "missileEntityStateFields" },
  { entityStateFields, 69u, "entityStateFields" },
  { scriptMoverStateFields, 72u, "scriptMoverStateFields" },
  { soundBlendEntityStateFields, 68u, "soundBlendEntityStateFields" },
  { fxStateFields, 68u, "fxStateFields" },
  { loopFxEntityStateFields, 69u, "loopFxEntityStateFields" },
  { entityStateFields, 69u, "entityStateFields" },
  { turretEntityStateFields, 69u, "turretEntityStateFields" },
  { helicopterEntityStateFields, 71u, "helicopterEntityStateFields" },
  { planeStateFields, 66u, "planeStateFields" },
  { vehicleEntityStateFields, 71u, "vehicleEntityStateFields" },
  { entityStateFields, 69u, "entityStateFields" },
  { entityStateFields, 69u, "entityStateFields" },
  { actorStateFields, 69u, "actorStateFields" },
  { actorStateFields, 69u, "actorStateFields" },
  { actorStateFields, 69u, "actorStateFields" },
  { entityStateFields, 69u, "entityStateFields" },
  { eventEntityStateFields, 69u, "eventEntityStateFields" }
};

static const NetField hudElemFields[43] =
{
  { "ui3dWindow", 109, 1, 8, 0u, "8", "0" },
  { "color.rgba", 16, 4, -85, 0u, "MSG_FIELD_RGBA", "0" },
  { "fadeStartTime", 24, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "fromColor.rgba", 20, 4, -85, 0u, "MSG_FIELD_RGBA", "0" },
  { "y", 4, 4, -91, 0u, "MSG_FIELD_ORIGINY", "0" },
  { "type", 100, 1, 4, 0u, "4", "0" },
  { "materialIndex", 104, 1, 8, 0u, "8", "0" },
  { "height", 76, 2, 10, 0u, "10", "0" },
  { "width", 74, 2, 10, 0u, "10", "0" },
  { "x", 0, 4, -92, 0u, "MSG_FIELD_ORIGINX", "0" },
  { "fadeTime", 70, 2, 16, 0u, "16", "0" },
  { "z", 8, 4, -90, 0u, "MSG_FIELD_ORIGINZ", "0" },
  { "value", 52, 4, 0, 0u, "0", "0" },
  { "alignScreen", 103, 1, 8, 0u, "8", "0" },
  { "sort", 56, 4, 0, 0u, "0", "0" },
  { "alignOrg", 102, 1, 4, 0u, "4", "0" },
  { "offscreenMaterialIdx", 105, 1, 8, 0u, "8", "0" },
  { "fontScale", 12, 4, -86, 0u, "MSG_FIELD_FONTSCALE", "0" },
  { "text", 86, 2, 10, 0u, "LOCALIZEDSTRINGINDEXBITS", "0" },
  { "font", 101, 1, 4, 0u, "4", "0" },
  { "scaleStartTime", 28, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "scaleTime", 82, 2, 16, 0u, "16", "0" },
  { "fromWidth", 78, 2, 10, 0u, "10", "0" },
  { "fromHeight", 80, 2, 10, 0u, "10", "0" },
  { "targetEntNum", 68, 2, 10, 0u, "GENTITYNUM_BITS", "0" },
  { "glowColor.rgba", 60, 4, -85, 0u, "MSG_FIELD_RGBA", "0" },
  { "fxBirthTime", 64, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "soundID", 108, 1, 5, 0u, "HUDELEM_SOUND_ID_BITS", "0" },
  { "fxLetterTime", 88, 2, 12, 0u, "12", "0" },
  { "fxDecayStartTime", 90, 2, 16, 0u, "16", "0" },
  { "fxDecayDuration", 92, 2, 16, 0u, "16", "0" },
  { "fxRedactDecayStartTime", 94, 2, 16, 0u, "16", "0" },
  { "fxRedactDecayDuration", 96, 2, 16, 0u, "16", "0" },
  { "flags", 98, 2, 13, 0u, "HUDELEMFLAGBITS", "0" },
  { "label", 72, 2, 10, 0u, "LOCALIZEDSTRINGINDEXBITS", "0" },
  { "time", 44, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "moveStartTime", 40, 4, -97, 0u, "MSG_FIELD_TIME", "0" },
  { "moveTime", 84, 2, 16, 0u, "16", "0" },
  { "fromX", 32, 4, -99, 0u, "MSG_FIELD_HUDELEMCOORD", "0" },
  { "fromY", 36, 4, -99, 0u, "MSG_FIELD_HUDELEMCOORD", "0" },
  { "fromAlignScreen", 107, 1, 8, 0u, "8", "0" },
  { "fromAlignOrg", 106, 1, 4, 0u, "4", "0" },
  { "duration", 48, 4, 32, 0u, "32", "0" }
};

static NetFieldList s_otherNetFieldList[6] =
{
  { archivedEntityFields, 8u, "archivedEntityFields" },
  { clientStateFields, 50u, "clientStateFields" },
  { playerStateFields, 179u, "playerStateFields" },
  { objectiveFields, 9u, "objectiveFields" },
  { hudElemFields, 43u, "hudElemFields" },
  { matchStateFields, 15u, "matchStateFields" }
};