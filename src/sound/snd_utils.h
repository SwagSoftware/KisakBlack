#pragma once

#include <universal/q_shared.h>

#define SND_2PI (2.0 * M_PI)

#define SND_SPEAKER_CONFIG_COUNT 3
#define SND_CHANNEL_MAP_VOLUME_COUNT 16

enum snd_speaker_flag : __int32
{                                       // XREF: snd_speaker_angle/r
                                        // snd_speaker_config/r ...
    SND_SPEAKER_FLAG_LEFT           = 0x1,
    SND_SPEAKER_FLAG_RIGHT          = 0x2,
    SND_SPEAKER_FLAG_CENTER         = 0x4,
    SND_SPEAKER_FLAG_LFE            = 0x8,
    SND_SPEAKER_FLAG_LEFT_SURROUND  = 0x10,
    SND_SPEAKER_FLAG_RIGHT_SURROUND = 0x20,
    SND_SPEAKER_FLAG_LEFT_REAR_SURROUND = 0x40,
    SND_SPEAKER_FLAG_RIGHT_REAR_SURROUND = 0x80,
};

struct snd_speaker_angle // sizeof=0x8
{                                       // XREF: snd_speaker_config/r
    snd_speaker_flag speaker;
    float angle;
};

struct snd_speaker_config // sizeof=0x54
{
    const char *name;
    snd_speaker_flag outputFormat;
    unsigned int speakerCount;
    unsigned int speakerMapCount;
    unsigned int angleCount;
    snd_speaker_angle angles[8];
};

struct snd_speaker_map;
struct snd_alias_list_t;
struct snd_pan;

unsigned int __cdecl SND_GetSpeakerConfigCount();
const snd_speaker_config *__cdecl Snd_GetSpeakerConfig(unsigned int index);
unsigned int __cdecl Snd_GetMixChannelCount(unsigned int speakerConfig);
float __cdecl Snd_PanMono(float angle);
float __cdecl Snd_NormalizeAngle(float x);
void __cdecl Snd_PanStereo(float angle, float boost, float *left, float *right);
void __cdecl Snd_Pan(unsigned int speakerCount, const float *angles, float toSound, float *levels);
bool __cdecl Snd_AngleInInterval(float angle, float left, float right);
void __cdecl Snd_Pan3d(
                const snd_speaker_config *config,
                const float *position,
                const float *forward,
                const float *listener,
                float centerSend,
                float lfeSend,
                float aliasOmni,
                snd_speaker_map *pan);
void __cdecl SND_EqualPowerFadeCoefs(float t, float *a, float *b);
void __cdecl SND_GetNearestPointOnSegment(
                const float *P,
                const float *segmentA,
                const float *segmentB,
                float *nearPoint);
void __cdecl SND_GetNearestPointOnStrip(
                const float *point,
                const float (*verts)[3],
                unsigned int count,
                float *position);
unsigned int __cdecl SND_HashAlias(const snd_alias_list_t *alias);
const char *__cdecl SND_GetAliasName(const snd_alias_list_t *alias);
float __cdecl SND_dBToLinear(float value);
float __cdecl SND_LinearToDb(float linear);
float __cdecl SND_LinearToDbSpl(float linear);
float __cdecl SND_dBSPLToLinear(float value);
int __cdecl SND_HashName(const char *name);
void __cdecl Snd_SpeakerMapSetVolume(snd_speaker_map *map, int in, int out, float volume);
void __cdecl Snd_SpeakerMapZero(snd_speaker_map *map);
int __cdecl Snd_SpeakerMapGetIndex(const snd_speaker_map *map, int in, int out);
float __cdecl Snd_SpeakerMapGetVolume(const snd_speaker_map *map, int in, int out);
void __cdecl SND_PanToSpeakermap(
                unsigned int inputChannelCount,
                unsigned int outputChannelCount,
                const snd_pan *pan,
                float centerSend,
                snd_speaker_map *map);
void __cdecl SND_PanToSpeakermap11(const snd_pan *pan, snd_speaker_map *map);
void __cdecl SND_PanToSpeakermap12(const snd_pan *pan, snd_speaker_map *map);
void __cdecl SND_PanToSpeakermap16(const snd_pan *pan, snd_speaker_map *map);
void __cdecl SND_PanToSpeakermap18(const snd_pan *pan, snd_speaker_map *map);
void __cdecl SND_PanToSpeakermap21(const snd_pan *pan, snd_speaker_map *map);
void __cdecl SND_PanToSpeakermap22(const snd_pan *pan, snd_speaker_map *map);
void __cdecl SND_PanToSpeakermap26(const snd_pan *pan, snd_speaker_map *map);
void __cdecl SND_PanToSpeakermap28(const snd_pan *pan, snd_speaker_map *map);
