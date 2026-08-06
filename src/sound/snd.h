#pragma once
#include <universal/q_shared.h>
#include <game/teams.h>
#include "snd_dsp.h"

#define SND_ENT_NONE 0xFFF
#define SND_ENT_NO_STOP 0xFFE
#define MAX_VARIANTS 64

#define SND_PLAYBACKID_NOTPLAYED -1
#define SND_PLAYBACK_COUNT 148 // 0x94

#define SND_INVALID_HASH 0

#define SND_EPSILON 0.0000152879

#define SND_INVALID_CURVE -1

enum snd_flux_type_t : __int32
{                                       // XREF: SND_SetVoiceStartFlux/r
    SND_FLUX_TYPE_NONE             = 0x0,
    SND_FLUX_TYPE_LEFT_OF_PLAYER   = 0x1,
    SND_FLUX_TYPE_CENTER_OF_PLAYER = 0x2,
    SND_FLUX_TYPE_RIGHT_OF_PLAYER  = 0x3,
    SND_FLUX_TYPE_RANDOM_PLAYER    = 0x4,
    SND_FLUX_TYPE_LEFT_OF_SHOT     = 0x5,
    SND_FLUX_TYPE_CENTER_OF_SHOT   = 0x6,
    SND_FLUX_TYPE_RIGHT_OF_SHOT    = 0x7,
    SND_FLUX_TYPE_COUNT            = 0x8,
};

enum snd_length_type : __int32
{                                                                             // XREF: sndLengthNotifyInfo/r
                                                                                // ?SND_AddLengthNotify@@YA_NHPAXW4snd_length_type@@@Z/r ...
        SND_LENGTH_NOTIFY_NONE         = 0x0,
        SND_LENGTH_NOTIFY_SCRIPT     = 0x1,
        SND_LENGTH_NOTIFY_SUBTITLE = 0x2,
        SND_LENGTH_NOTIFY_COUNT        = 0x3,
};

enum snd_limit_type_t : __int32
{                                                                             // XREF: SND_Limit/r
                                                                                // ?SND_PlaySoundAlias@@YAHPBUsnd_alias_list_t@@MTSndEntHandle@@QBMHH2PAUsnd_playback@@PAUsnd_occlusion_start_cache@@@Z/r ...
        SND_LIMIT_NONE         = 0x0,
        SND_LIMIT_OLDEST     = 0x1,
        SND_LIMIT_REJECT     = 0x2,
        SND_LIMIT_PRIORITY = 0x3,
        SND_LIMIT_COUNT        = 0x4,
};

enum snd_entity_update : __int32
{                                                                             // XREF: snd_voice_t/r
        SND_ENTITY_UPDATE_ALWAYS = 0x0,
        SND_ENTITY_UPDATE_NEVER    = 0x1,
};

enum SndFileLoadingState : __int32
{                                                                             // XREF: SndFileSpecificVoiceInfo/r
                                                                                // ?SND_SetSoundFileVoiceInfo@@YAXHHHHHW4SndFileLoadingState@@@Z/r
        SFLS_UNLOADED = 0x0,
        SFLS_LOADING    = 0x1,
        SFLS_LOADED     = 0x2,
};


 enum snd_asset_format : __int32
 {                                                                             // XREF: snd_asset/r
         SND_ASSET_FORMAT_PCMS16    = 0x0,
         SND_ASSET_FORMAT_PCMS24    = 0x1,
         SND_ASSET_FORMAT_PCMS32    = 0x2,
         SND_ASSET_FORMAT_IEEE        = 0x3,
         SND_ASSET_FORMAT_XMA4        = 0x4,
         SND_ASSET_FORMAT_MP3         = 0x5,
         SND_ASSET_FORMAT_MSADPCM = 0x6,
         SND_ASSET_FORMAT_WMA         = 0x7,
 };

 enum snd_asset_flags : __int32
 {                                                                             // XREF: snd_asset/r
         SND_ASSET_FLAG_DEFAULT                 = 0x0,
         SND_ASSET_FLAG_LOOPING                 = 0x1,
         SND_ASSET_FLAG_PAD_LOOP_BUFFER = 0x2,
 };

enum snd_asset_channel : __int32
{                                                                             // XREF: snd_asset/r
        SND_ASSET_CHANNEL_L     = 0x1,
        SND_ASSET_CHANNEL_R     = 0x2,
        SND_ASSET_CHANNEL_C     = 0x4,
        SND_ASSET_CHANNEL_LFE = 0x8,
        SND_ASSET_CHANNEL_LS    = 0x10,
        SND_ASSET_CHANNEL_RS    = 0x20,
        SND_ASSET_CHANNEL_LB    = 0x40,
        SND_ASSET_CHANNEL_RB    = 0x80,
};


struct snd_asset // sizeof=0x38
{                                                                             // XREF: LoadedSound/r
                                                                                // snd_stream/r
        unsigned int version;
        unsigned int frame_count;
        unsigned int frame_rate;
        unsigned int channel_count;
        unsigned int header_size;
        unsigned int block_size;
        unsigned int buffer_size;
        snd_asset_format format;
        snd_asset_channel channel_flags;
        snd_asset_flags flags;
        unsigned int seek_table_count;
        unsigned int *seek_table;
        unsigned int data_size;
        char *data;
};

struct LoadedSound // sizeof=0x3C
{
        const char *name;
        snd_asset sound;
};

struct PrimedSound // sizeof=0xC
{
        const char *name;
        char *buffer;
        unsigned int size;
};

struct StreamedSound // sizeof=0x8
{
        char *filename;
        PrimedSound *primeSnd;
};

union SoundFileRef // sizeof=0x4
{                                                                             // XREF: SoundFile/r
        LoadedSound *loadSnd;
        StreamedSound *streamSnd;
};


struct __declspec(align(4)) SoundFile // sizeof=0x8
{
        SoundFileRef u;
        unsigned __int8 type;
        unsigned __int8 exists;
        // padding byte
        // padding byte
};


struct __declspec(align(2)) snd_alias_t // sizeof=0x54
{
        const char *name;
        unsigned int id;
        const char *subtitle;
        const char *secondaryname;
        SoundFile *soundFile;
        unsigned int flags;
        unsigned int duck;
        unsigned int contextType;
        unsigned int contextValue;
        unsigned __int16 fluxTime;
        unsigned __int16 startDelay;
        unsigned __int16 reverbSend;
        unsigned __int16 centerSend;
        unsigned __int16 volMin;
        unsigned __int16 volMax;
        unsigned __int16 teamVolMod;
        unsigned __int16 pitchMin;
        unsigned __int16 pitchMax;
        unsigned __int16 teamPitchMod;
        unsigned __int16 distMin;
        unsigned __int16 distMax;
        unsigned __int16 distReverbMax;
        unsigned __int16 envelopMin;
        unsigned __int16 envelopMax;
        unsigned __int16 envelopPercentage;
        unsigned __int8 minPriorityThreshold;
        unsigned __int8 maxPriorityThreshold;
        unsigned __int8 probability;
        unsigned __int8 occlusionLevel;
        unsigned __int8 occlusionWetDry;
        unsigned __int8 minPriority;
        unsigned __int8 maxPriority;
        unsigned __int8 pan;
        unsigned __int8 dryCurve;
        unsigned __int8 wetCurve;
        unsigned __int8 dryMinCurve;
        unsigned __int8 wetMinCurve;
        unsigned __int8 limitCount;
        unsigned __int8 entityLimitCount;
        unsigned __int8 snapshotGroup;
        // padding byte
};


struct snd_alias_list_t // sizeof=0x14
{                                                                             // XREF: XAssetPoolEntry<snd_alias_list_t>/r
        const char *name;
        unsigned int id;
        snd_alias_t *head;
        int count;
        int sequence;
};



union SndEntHandle // sizeof=0x4
{                                                                             // XREF: CG_EntityEvent(int,centity_s *,int)+3E5C/w
    //SndEntHandle::<unnamed_type_field> field;
    struct // sizeof=0x4
    {                                                                             // XREF: SndEntHandle/r
            unsigned __int32 entIndex : 12;
            unsigned __int32 useCount : 7;
            unsigned __int32 localClientNum : 2;
            unsigned __int32 isStationary : 1;
            unsigned __int32 isInKillCam : 1;
            unsigned __int32 team : 3;
            unsigned __int32 tag : 6;
    } field;
    int handle;

    constexpr SndEntHandle() : handle(0) {}
    constexpr SndEntHandle(int init) : handle(init) {}
};

struct __declspec(align(4)) snd_occlusion_start_cache // sizeof=0x14
{                                                                             // XREF: ?SNDL_Play@@YAHIHMTSndEntHandle@@QBM1_NPAUsnd_playback@@@Z/r
                                                                                // SND_UpdateStaticSounds/r ...
        float position[3];                                    // XREF: SND_UpdateStaticSounds+220/w
                                                                                // SND_UpdateStaticSounds+223/w ...
        float value;                                                // XREF: SND_UpdateStaticSounds+229/w
                                                                                // SND_UpdateStaticSounds+364/w ...
        bool valid;                                                 // XREF: SND_UpdateStaticSounds+22C/w
                                                                                // SND_UpdateStaticSounds+367/w ...
        // padding byte
        // padding byte
        // padding byte
};


struct snd_playback // sizeof=0x10
{                                                                             // XREF: snd_local_t/r
        int id;                                                         // XREF: SND_ResetPlaybacks(void)+25/w
                                                                                // SND_AllocatePlayback(void)+67/r ...
        float attenuation;                                    // XREF: SND_AllocatePlayback(void)+8F/w
        int lengthMs;                                             // XREF: SND_AllocatePlayback(void)+9D/w
        int playedMs;                                             // XREF: SND_AllocatePlayback(void)+AD/w
};


struct SndStartAliasInfo // sizeof=0x3C
{                                                                             // XREF: ?SND_PlaySoundAlias@@YAHPBUsnd_alias_list_t@@MTSndEntHandle@@QBMHH2PAUsnd_playback@@PAUsnd_occlusion_start_cache@@@Z/r
        const snd_alias_t *alias;                     // XREF: SND_PlaySoundAlias(snd_alias_list_t const *,float,SndEntHandle,float const * const,int,int,float const * const,snd_playback *,snd_occlusion_start_cache *)+7BD/w
        SndEntHandle sndEnt;                                // XREF: SND_PlaySoundAlias(snd_alias_list_t const *,float,SndEntHandle,float const * const,int,int,float const * const,snd_playback *,snd_occlusion_start_cache *)+7C3/w
        float org[3];                                             // XREF: SND_PlaySoundAlias(snd_alias_list_t const *,float,SndEntHandle,float const * const,int,int,float const * const,snd_playback *,snd_occlusion_start_cache *)+7CD/w
                                                                                // SND_PlaySoundAlias(snd_alias_list_t const *,float,SndEntHandle,float const * const,int,int,float const * const,snd_playback *,snd_occlusion_start_cache *)+7DA/w ...
        float dir[3];                                             // XREF: SND_PlaySoundAlias(snd_alias_list_t const *,float,SndEntHandle,float const * const,int,int,float const * const,snd_playback *,snd_occlusion_start_cache *)+7F9/w
                                                                                // SND_PlaySoundAlias(snd_alias_list_t const *,float,SndEntHandle,float const * const,int,int,float const * const,snd_playback *,snd_occlusion_start_cache *)+806/w ...
        float volModStart;                                    // XREF: SND_PlaySoundAlias(snd_alias_list_t const *,float,SndEntHandle,float const * const,int,int,float const * const,snd_playback *,snd_occlusion_start_cache *)+7B5/w
        int timeshift;                                            // XREF: SND_PlaySoundAlias(snd_alias_list_t const *,float,SndEntHandle,float const * const,int,int,float const * const,snd_playback *,snd_occlusion_start_cache *)+844/w
        float fadetime;                                         // XREF: SND_PlaySoundAlias(snd_alias_list_t const *,float,SndEntHandle,float const * const,int,int,float const * const,snd_playback *,snd_occlusion_start_cache *)+856/w
        int startDelay;                                         // XREF: SND_PlaySoundAlias(snd_alias_list_t const *,float,SndEntHandle,float const * const,int,int,float const * const,snd_playback *,snd_occlusion_start_cache *)+84E/w
                                                                                // SND_PlaySoundAlias(snd_alias_list_t const *,float,SndEntHandle,float const * const,int,int,float const * const,snd_playback *,snd_occlusion_start_cache *)+8EB/r ...
        snd_occlusion_start_cache *ocache;    // XREF: SND_PlaySoundAlias(snd_alias_list_t const *,float,SndEntHandle,float const * const,int,int,float const * const,snd_playback *,snd_occlusion_start_cache *)+85E/w
        snd_playback *playback;                         // XREF: SND_PlaySoundAlias(snd_alias_list_t const *,float,SndEntHandle,float const * const,int,int,float const * const,snd_playback *,snd_occlusion_start_cache *)+864/w
        int playbackId;                                         // XREF: SND_PlaySoundAlias(snd_alias_list_t const *,float,SndEntHandle,float const * const,int,int,float const * const,snd_playback *,snd_occlusion_start_cache *)+872/w
                                                                                // SND_PlaySoundAlias(snd_alias_list_t const *,float,SndEntHandle,float const * const,int,int,float const * const,snd_playback *,snd_occlusion_start_cache *)+87C/w
};

struct snd_fader_t // sizeof=0xC
{                                                                             // XREF: snd_voice_t/r
                                                                                // snd_voice_t/r ...
        float value;                                                // XREF: SND_UpdateMasterVolumes+22/r
                                                                                // SND_Init(void)+1A9/w ...
        float goal;                                                 // XREF: SND_UpdateMasterVolumes+2F/r
                                                                                // SND_Init(void)+1A1/r
        float rate;
};

struct SndFileSpecificVoiceInfo // sizeof=0x14
{                                                                             // XREF: snd_voice_t/r
        SndFileLoadingState loadingState;     // XREF: SD_PauseVoice(int)+1C/r
                                                                                // SD_UnpauseVoice(int,int)+C/r ...
        int srcChannelCount;
        int baserate;
        int endtime;                                                // XREF: SND_UnpauseSounds+85/r
                                                                                // SND_UnpauseSounds+97/w
        int totalMsec;                                            // XREF: SND_GetVoiceLength(int)+39/r
};

struct sndLengthNotifyInfo // sizeof=0x24
{                                                                             // XREF: snd_voice_t/r
        snd_length_type id[4];
        void *data[4];
        int count;
};

struct snd_speaker_map // sizeof=0x48
{                                                                             // XREF: snd_voice_t/r
                                                                                // snd_voice_t/r ...
        int input_channel_count;
        int output_channel_count;
        float volumes[16];
};

struct snd_voice_t // sizeof=0x1D8
{                                                                             // XREF: snd_local_t/r
        SndFileSpecificVoiceInfo soundFileInfo;
                                                                                // XREF: SND_UnpauseSounds+85/r
                                                                                // SND_UnpauseSounds+97/w ...
        SndEntHandle sndEnt;                                // XREF: SND_ContinueLoopingSound(uint,float,SndEntHandle,float const * const,int,snd_playback *)+54/r
                                                                                // SND_GetPlayingInfo+94/r ...
        snd_entity_update entity_update;
        int group;
        int startDelay;
        int startTime;                                            // XREF: SND_GetPlayingInfo+B3/r
                                                                                // SND_GetPlayingInfo+C7/r ...
        int looptime;
        int totalMsec;
        snd_playback *playback;
        int playbackId;                                         // XREF: SND_SetPlaybackIdNotPlayed(int)+3E/w
                                                                                // SND_ContinueLoopingSound(uint,float,SndEntHandle,float const * const,int,snd_playback *)+109/r ...
        int firstPlaybackId;
        sndLengthNotifyInfo lengthNotifyInfo;
        const snd_alias_t *alias;                     // XREF: SND_ContinueLoopingSound(uint,float,SndEntHandle,float const * const,int,snd_playback *)+68/r
                                                                                // SND_ContinueLoopingSound(uint,float,SndEntHandle,float const * const,int,snd_playback *)+8A/r ...
        float offset[3];
        float position[3];                                    // XREF: SND_SetPosition(int,float const * const)+40/o
        float velocity[3];
        float orientation[3][3];
        float direction[3];
        float fluxVelocity[3];
        float baseDistance;
        float distanceAttenuation;
        float reverbAttenuation;
        float cylinderAttenuation;
        float volModSeed;
        float volModStart;
        float pitchModSeed;
        float globalPriority;                             // XREF: Snd_GetLowestPriority+8B/r
                                                                                // SND_Limit+8D/r ...
        snd_fader_t doppler;
        snd_fader_t fade;
        snd_fader_t script_fade;
        snd_fader_t script_pitch;
        snd_fader_t losOcclusion;
        int losHitCache;
        int closestListenerIndex;                     // XREF: DebugDrawWorldSound3D+14F/r
        int voiceStartTime;
        float dryLevel;
        float wetLevel;
        float futzBlend;
        snd_speaker_map pan;
        snd_speaker_map panGoal;
        bool paused;                                                // XREF: SND_PauseSounds+67/w
                                                                                // SND_UnpauseSounds+50/r ...
        bool timescale;
        bool positionUpdated;
        bool pitchShift;
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        __int64 played;
};

struct snd_curve // sizeof=0x64
{
        char name[32];
        unsigned int id;
        float points[8][2];
};
static_assert(sizeof(snd_curve) == 100);

struct __declspec(align(4)) snd_game_state // sizeof=0x10
{                                                                             // XREF: snd_local_t/r
        float timescale;                                        // XREF: SNDL_Update(void)+A5/r
                                                                                // SND_Init(void)+1B9/w ...
        unsigned int cgTime;                                // XREF: SNDL_SetGameState(bool,bool,float,uint,uint)+53/w
        unsigned int seed;                                    // XREF: SND_PickSoundAliasFromList(snd_alias_list_t const *,int):loc_934E73/r
                                                                                // SND_PickSoundAliasFromList(snd_alias_list_t const *,int)+1CC/o ...
        bool gamePaused;                                        // XREF: SND_UpdatePause+6/r
                                                                                // SNDL_SetGameState(bool,bool,float,uint,uint)+3D/w
        bool mature;                                                // XREF: SNDL_SetGameState(bool,bool,float,uint,uint)+35/w
        // padding byte
        // padding byte
};

enum snd_category_t : __int32
{                                                                             // XREF: snd_group/r
        SND_CATEGORY_SFX     = 0x0,
        SND_CATEGORY_MUSIC = 0x1,
        SND_CATEGORY_VOICE = 0x2,
        SND_CATEGORY_UI        = 0x3,
        SND_CATEGORY_COUNT = 0x4,
};

struct snd_group // sizeof=0x50
{
        char name[32];
        char parentName[32];
        unsigned int id;
        int parentIndex;
        snd_category_t category;
        unsigned __int16 attenuationSp;
        unsigned __int16 attenuationMp;
};
static_assert(sizeof(snd_group) == 80);

struct snd_pan // sizeof=0x3C
{
        char name[32];
        unsigned int id;
        float front;
        float back;
        float center;
        float lfe;
        float left;
        float right;
};
static_assert(sizeof(snd_pan) == 60);

struct snd_snapshot_group // sizeof=0x20
{
        char name[32];
};
static_assert(sizeof(snd_snapshot_group) == 32);

struct snd_context // sizeof=0x28
{
        unsigned int type;
        unsigned int valueCount;
        unsigned int values[8];
};
static_assert(sizeof(snd_context) == 40);

struct snd_master // sizeof=0xB0
{
        char name[32];
        unsigned int id;
        float notchE;
        float notchG;
        float notchF;
        float notchQ;
        float lowE;
        float lowG;
        float lowF;
        float lowQ;
        float peak1E;
        float peak1G;
        float peak1F;
        float peak1Q;
        float peak2E;
        float peak2G;
        float peak2F;
        float peak2Q;
        float hiE;
        float hiG;
        float hiF;
        float hiQ;
        float eqG;
        float compE;
        float compPG;
        float compMG;
        float compT;
        float compR;
        float compTA;
        float compTR;
        float limitE;
        float limitPG;
        float limitMG;
        float limitT;
        float limitR;
        float limitTA;
        float limitTR;
};
static_assert(sizeof(snd_master) == 176);

struct SndDriverGlobals // sizeof=0x34
{                                                                             // XREF: XAssetPoolEntry<SndDriverGlobals>/r
    const char *name;
    unsigned int groupCount;
    snd_group *groups; // hehe
    unsigned int curveCount;
    snd_curve * curves;
    unsigned int panCount;
    snd_pan *pans;
    unsigned int snapshotGroupCount;
    snd_snapshot_group * snapshotGroups;
    unsigned int contextCount;
    snd_context *contexts;
    unsigned int masterCount;
    snd_master * masters;
};
static_assert(sizeof(SndDriverGlobals) == 52);

struct __declspec(align(4)) snd_enveffect // sizeof=0x20
{                                                                             // XREF: snd_local_t/r
        unsigned int reverbId;
        float drylevel;
        float drygoal;
        float dryrate;
        float wetlevel;
        float wetgoal;
        float wetrate;
        bool active;                                                // XREF: SNDL_SetEnvironmentEffects(int,uint,float,float,int)+226/r
                                                                                // SNDL_DeactivateEnvironmentEffects(int,int)+EC/r
        // padding byte
        // padding byte
        // padding byte
};

struct snd_listener // sizeof=0x3C
{                                                                             // XREF: snd_local_t/r
        orientation_t orient;                             // XREF: SND_UpdateVoice(snd_voice_t *,float)+743/r
                                                                                // SND_UpdateVoice(snd_voice_t *,float)+778/r ...
        int clientNum;                                            // XREF: SNDL_SetListener(int,int,team_t,float const * const,float const (* const)[3])+4FA/w
                                                                                // SNDL_SetListener(int,int,team_t,float const * const,float const (* const)[3])+6AE/w
        bool active;                                                // XREF: SND_ActiveListenerCount(void)+2B/r
                                                                                // SND_GetListenerIndexNearestToOrigin(float const * const)+50/r ...
        // padding byte
        // padding byte
        // padding byte
        team_t team;                                                // XREF: SND_IsOnSameTeam+1F/r
                                                                                // SNDL_SetListener(int,int,team_t,float const * const,float const (* const)[3])+6CA/w
};


struct snd_line_emitter // sizeof=0x20
{                                                                             // XREF: snd_local_t/r
        unsigned int id;                                        // XREF: SND_UpdateStaticSounds+3C/r
                                                                                // SND_UpdateStaticSounds+60/r ...
        float origin[2][3];                                 // XREF: SND_UpdateStaticSounds+BD/o
                                                                                // SND_UpdateStaticSounds+FB/o ...
        const snd_alias_list_t *alias;            // XREF: SND_UpdateStaticSounds+51/r
                                                                                // SND_UpdateStaticSounds+75/w ...
};

struct snd_loop_emitter // sizeof=0x14
{                                                                             // XREF: snd_local_t/r
        unsigned int id;                                        // XREF: SND_UpdateStaticSounds+2B6/r
                                                                                // SND_UpdateStaticSounds+2DA/r ...
        float origin[3];                                        // XREF: SND_UpdateStaticSounds+3AA/o
                                                                                // SNDL_PlayLoopAt(uint,float const * const)+88/o ...
        const snd_alias_list_t *alias;            // XREF: SND_UpdateStaticSounds+2CB/r
                                                                                // SND_UpdateStaticSounds+2EF/w ...
};

struct snd_snapshot // sizeof=0x15C
{
    char name[32];
    unsigned int id;
    char occlusionName[32];
    unsigned int occlusionId;
    float fadeIn;
    float fadeOut;
    float distance;
    unsigned int fadeInCurve;
    unsigned int fadeOutCurve;
    float attenuation[64];
};
static_assert(sizeof(snd_snapshot) == 348);

struct snd_snapshot_category // sizeof=0x60C
{                                                                             // XREF: snd_local_t/r
        snd_fader_t attenuation[64];
        snd_fader_t occlusion[64];
        unsigned int snapshot;                            // XREF: SNDL_SetSnapshot(snd_snapshot_type,uint,float,float)+81/w
                                                                                // SNDL_SetSnapshot(snd_snapshot_type,uint,float,float)+C3/w
        float length;                                             // XREF: SNDL_SetSnapshot(snd_snapshot_type,uint,float,float)+95/w
                                                                                // SNDL_SetSnapshot(snd_snapshot_type,uint,float,float)+D7/w
        float amount;                                             // XREF: SNDL_SetSnapshot(snd_snapshot_type,uint,float,float)+AB/w
                                                                                // SNDL_SetSnapshot(snd_snapshot_type,uint,float,float)+ED/w
};

struct snd_ent_state // sizeof=0x48
{                                                                             // XREF: snd_local_t/r
        SndEntHandle handle;                                // XREF: SND_FindEntState(SndEntHandle,bool)+3C1/r
        float origin[3];
        float velocity[3];
        float orientation[3][3];                        // XREF: .rdata:00D146D5/o
                                                                                // .rdata:00D148C1/o
        unsigned int lastUsed;                            // XREF: SND_FindEntState(SndEntHandle,bool)+2C6/r
                                                                                // SND_FindEntState(SndEntHandle,bool)+3B4/r ...
        snd_ent_state *next;                                // XREF: SND_FindEntState(SndEntHandle,bool)+39A/r
};

enum snd_command_type : __int32
{                                                                             // XREF: snd_command/r
        SND_COMMAND_NOP                                 = 0x0,
        SND_COMMAND_ALIAS_NAME                    = 0x1,
        SND_COMMAND_PLAY                                = 0x2,
        SND_COMMAND_STOP_ALIAS                    = 0x3,
        SND_COMMAND_STOP_ENT                        = 0x4,
        SND_COMMAND_CINEMATIC_START         = 0x5,
        SND_COMMAND_CINEMATIC_END             = 0x6,
        SND_COMMAND_DISCONNECT_LISTENER = 0x7,
        SND_COMMAND_SET_LISTENER                = 0x8,
        SND_COMMAND_STOP_SOUNDS                 = 0x9,
        SND_COMMAND_FADE_IN                         = 0xA,
        SND_COMMAND_FADE_OUT                        = 0xB,
        SND_COMMAND_SET_ENVIRONMENT_EFFECTS = 0xC,
        SND_COMMAND_DEACTIVATE_ENVIRONMENT_EFFECTS = 0xD,
        SND_COMMAND_SET_PLAYBACK_ATTENUATION = 0xE,
        SND_COMMAND_SET_PLAYBACK_ATTENUATION_RATE = 0xF,
        SND_COMMAND_SET_PLAYBACK_PITCH    = 0x10,
        SND_COMMAND_SET_PLAYBACK_PITCH_RATE = 0x11,
        SND_COMMAND_STOP_PLAYBACK             = 0x12,
        SND_COMMAND_SNAPSHOT                        = 0x13,
        SND_COMMAND_SET_ENT_STATE             = 0x14,
        SND_COMMAND_SET_GAME_STATE            = 0x15,
        SND_COMMAND_PLAY_LOOP_AT                = 0x16,
        SND_COMMAND_STOP_LOOP_AT                = 0x17,
        SND_COMMAND_PLAY_LINE_AT                = 0x18,
        SND_COMMAND_STOP_LINE_AT                = 0x19,
        SND_COMMAND_GAME_RESET                    = 0x1A,
        SND_COMMAND_SET_CONTEXT                 = 0x1B,
        SND_COMMAND_SCRIPT_TIMESCALE        = 0x1C,
        SND_COMMAND_UPDATE_LOOPS                = 0x1D,
        SND_COMMAND_COUNT                             = 0x1E,
};

struct snd_occlusion_trace_t // sizeof=0x24
{                                                                             // XREF: snd_local_t/r
        float listener[3];
        float playback[3];                                    // XREF: SND_LosOcclusionUpdate(void)+157/o
        int cache;                                                    // XREF: SND_LosOcclusionUpdate(void)+E9/w
                                                                                // SND_LosOcclusionUpdate(void)+1C1/w
        int id;                                                         // XREF: SND_LosOcclusionUpdate(void)+B8/r
                                                                                // SND_LosOcclusionUpdate(void)+10B/w ...
        float occlusion;                                        // XREF: SND_LosOcclusionUpdate(void)+198/w
};


struct snd_alias_name_context // sizeof=0x34
{                                                                             // XREF: snd_command_context/r
        char name[48];
        unsigned int id;
};

struct snd_play_context // sizeof=0x30
{                                                                             // XREF: snd_command_context/r
        unsigned int alias;
        int fadeTimeMs;
        float attenuation;
        SndEntHandle entHandle;
        int notify;
        float position[3];
        float direction[3];
        snd_playback *playback;
};

struct snd_stop_alias_context // sizeof=0x8
{                                                                             // XREF: snd_command_context/r
        SndEntHandle ent;
        unsigned int alias_name;
};

struct snd_stop_ent_context // sizeof=0x4
{                                                                             // XREF: snd_command_context/r
        SndEntHandle ent;
};

struct snd_cinematic_start_context // sizeof=0x4
{                                                                             // XREF: snd_command_context/r
        float volume;
};

struct snd_disconnect_listener_context // sizeof=0x4
{                                                                             // XREF: snd_command_context/r
        int listener;
};

struct snd_set_listener_context // sizeof=0x3C
{                                                                             // XREF: snd_command_context/r
        int listener;
        int clientNum;
        team_t team;
        float origin[3];
        float axis[3][3];
};

enum snd_stop_sound_flags : __int32
{                                                                             // XREF: snd_stop_sounds_context/r
                                                                                // ?SND_StopSounds@@YAXW4snd_stop_sound_flags@@@Z/r ...
        SND_STOP_ALL            = 0x0,
        SND_STOP_STREAMED = 0x1,
        SND_EVERY_SINGLE_ONE_DONT_ASK_ANY_QUESTIONS = 0x2,
};
struct snd_stop_sounds_context // sizeof=0x4
{                                                                             // XREF: snd_command_context/r
        snd_stop_sound_flags flags;
};

struct snd_set_environment_effects_context // sizeof=0x14
{                                                                             // XREF: snd_command_context/r
        int priority;
        unsigned int id;
        float drylevel;
        float wetlevel;
        int fademsec;
};

struct snd_deactivate_environment_effects_context // sizeof=0x8
{                                                                             // XREF: snd_command_context/r
        int priority;
        int fademsec;
};

struct snd_set_playback_attenuation_context // sizeof=0x8
{                                                                             // XREF: snd_command_context/r
        int id;
        float attenuation;
};

struct snd_set_playback_attenuation_rate_context // sizeof=0x8
{                                                                             // XREF: snd_command_context/r
        int id;
        float rate;
};

struct snd_set_playback_pitch_context // sizeof=0x8
{                                                                             // XREF: snd_command_context/r
        int id;
        float pitch;
};

struct snd_set_playback_pitch_rate_context // sizeof=0x8
{                                                                             // XREF: snd_command_context/r
        int id;
        float rate;
};

struct snd_stop_playback_context // sizeof=0x4
{                                                                             // XREF: snd_command_context/r
        int id;
};

enum snd_snapshot_type : __int32
{                                                                             // XREF: snd_snapshot_context/r
                                                                                // ?SND_SetSnapshot@@YAXW4snd_snapshot_type@@PBDMM@Z/r ...
        SND_SNAPSHOT_ALIAS            = 0x0,
        SND_SNAPSHOT_AMBIENT        = 0x1,
        SND_SNAPSHOT_SCRIPT         = 0x2,
        SND_SNAPSHOT_SHELLSHOCK = 0x3,
        SND_SNAPSHOT_BREATH         = 0x4,
        SND_SNAPSHOT_ADS                = 0x5,
        SND_SNAPSHOT_MENU             = 0x6,
        SND_SNAPSHOT_ENEMIES        = 0x7,
        SND_SNAPSHOT_HEALTH         = 0x8,
        SND_SNAPSHOT_DEBUG            = 0x9,
        SND_SNAPSHOT_LEVELFADE    = 0xA,
        SND_SNAPSHOT_COUNT            = 0xB,
};

struct snd_snapshot_context // sizeof=0x10
{                                                                             // XREF: snd_command_context/r
        snd_snapshot_type type;
        unsigned int id;
        float length;
        float amount;
};

struct snd_set_ent_state_context // sizeof=0x40
{                                                                             // XREF: snd_command_context/r
        SndEntHandle handle;
        float origin[3];
        float velocity[3];
        float orientation[3][3];
};

struct snd_set_game_state_context // sizeof=0x10
{                                                                             // XREF: snd_command_context/r
        bool is_paused;
        bool is_mature;
        // padding byte
        // padding byte
        float timescale;
        unsigned int cg_time;
        unsigned int seed;
};

struct snd_script_timescale // sizeof=0x4
{                                                                             // XREF: snd_command_context/r
        float value;
};

struct snd_loop_at // sizeof=0x10
{                                                                             // XREF: snd_command_context/r
        unsigned int id;
        float origin[3];
};

struct snd_line_at // sizeof=0x1C
{                                                                             // XREF: snd_command_context/r
        unsigned int id;
        float origin[2][3];
};

struct snd_set_context // sizeof=0x8
{                                                                             // XREF: snd_command_context/r
        unsigned int type;
        unsigned int value;
};

union snd_command_context // sizeof=0x40
{                                                                             // XREF: .rdata:00D146D9/o
    snd_alias_name_context alias_name;
    snd_play_context play;
    snd_stop_alias_context stop_alias;
    snd_stop_ent_context stop_ent;
    snd_cinematic_start_context notify_cinematic_start;
    snd_disconnect_listener_context disconnect_listener;
    snd_set_listener_context set_listener;
    snd_stop_sounds_context stop_sounds;
    snd_set_environment_effects_context set_environment_effects;
    snd_deactivate_environment_effects_context deactivate_environment_effects;
    snd_set_playback_attenuation_context set_playback_attenuation;
    snd_set_playback_attenuation_rate_context set_playback_attenuation_rate;
    snd_set_playback_pitch_context set_playback_pitch;
    snd_set_playback_pitch_rate_context set_playback_pitch_rate;
    snd_stop_playback_context stop_playback;
    snd_snapshot_context snapshot;
    snd_set_ent_state_context set_ent_state;
    snd_set_game_state_context set_game_state;
    snd_loop_at loop_at;
    snd_line_at line_at;
    snd_set_context set_context;
    snd_script_timescale script_timescale;

    snd_command_context() { memset(this, 0, sizeof(*this)); }
};

struct snd_command // sizeof=0x4C
{                                                                             // XREF: snd_local_t/r
        snd_command_type type;                            // XREF: SND_GetNewCommand(void)+A7/r
        snd_command_context context;                // XREF: .rdata:00D146D9/o
                                                                                // .rdata:00D146DD/o ...
        unsigned int id;
        int timestamp;
};

enum snd_notify_type : __int32
{                                                                             // XREF: snd_notify/r
        SND_NOTIFY_NOP                         = 0x0,
        SND_NOTIFY_ENT_UPDATE            = 0x1,
        SND_NOTIFY_SUBTITLE                = 0x2,
        SND_NOTIFY_LENGTH                    = 0x3,
        SND_NOTIFY_PLAYBACK_UPDATE = 0x4,
        SND_NOTIFY_PLAYBACK_FREE     = 0x5,
};

struct snd_subtitle // sizeof=0x8
{                                                                             // XREF: snd_notify_context/r
        unsigned int lengthMs;
        const char *subtitle;
};

struct snd_playback_update // sizeof=0x10
{                                                                             // XREF: snd_notify_context/r
        snd_playback *playback;
        float attenuation;
        unsigned int lengthMs;
        unsigned int playedMs;
};

struct snd_ent_update // sizeof=0x4
{                                                                             // XREF: snd_notify_context/r
        SndEntHandle handle;
};

struct snd_playback_free // sizeof=0x4
{                                                                             // XREF: snd_notify_context/r
        snd_playback *playback;
};

struct snd_length // sizeof=0x8
{                                                                             // XREF: snd_notify_context/r
        unsigned int lengthMs;
        unsigned int ent;
};

union snd_notify_context // sizeof=0x10
{                                                                             // XREF: snd_notify/r
    snd_ent_update ent_update;
    snd_subtitle subtitle;
    snd_length length;
    snd_playback_free playback_free;
    snd_playback_update playback_update;

    snd_notify_context() { memset(this, 0, sizeof(*this)); }
};

struct snd_notify // sizeof=0x14
{                                                                             // XREF: snd_local_t/r
        snd_notify_type type;                             // XREF: SND_GetNewNotify(void)+B0/r
        snd_notify_context context;
};

struct snd_reverse_hash // sizeof=0x38
{                                                                             // XREF: snd_local_t/r
        char name[48];
        unsigned int hash;                                    // XREF: SND_LogLookupAlias(uint,char *)+35/r
                                                                                // SND_LogRegisterString(char const *,uint)+44/r ...
        unsigned int timestamp;                         // XREF: SND_LogLookupAlias(uint,char *)+4C/w
                                                                                // SND_LogRegisterString(char const *,uint)+17/r ...
};

struct snd_local_t // sizeof=0x31730
{                                                                             // XREF: .data:snd_local_t g_snd/r
        int init;                                                     // XREF: SND_Active(void)+3/r
                                                                                // SND_Init(void)+6/r ...
        int paused;                                                 // XREF: SND_SetVoiceStartInfo(int,SndStartAliasInfo *)+14B/r
                                                                                // SND_UpdatePause+33/r ...
        snd_game_state gameState;                     // XREF: SND_PickSoundAliasFromList(snd_alias_list_t const *,int):loc_934E73/r
                                                                                // SND_PickSoundAliasFromList(snd_alias_list_t const *,int)+1CC/o ...
        float timescale;                                        // XREF: SNDL_Update(void)+EE/r
                                                                                // SNDL_Update(void)+FA/w ...
        float scriptTimescale;                            // XREF: SNDL_Update(void)+AD/r
                                                                                // SNDL_Update(void)+BD/r ...
        int time;                                                     // XREF: SND_SetVoiceStartInfo(int,SndStartAliasInfo *)+18B/r
                                                                                // SND_SetVoiceStartSeeds+77/r ...
        int looptime;                                             // XREF: SND_SetVoiceStartInfo(int,SndStartAliasInfo *)+143/r
                                                                                // SND_ContinueLoopingSound_Internal+124/r ...
        int pausetime;                                            // XREF: SND_UpdatePause+4B/w
                                                                                // SND_UpdatePause+57/w ...
        unsigned int frame;                                 // XREF: SNDL_Update(void)+91/r
                                                                                // SNDL_Update(void)+9A/w ...
        SndDriverGlobals *global_constants; // XREF: SND_Init(void)+CB/w
                                                                                // SND_Init(void)+D0/r ...
        int inCinematic;                                        // XREF: SNDL_NotifyCinematicStart(float):loc_9476A2/r
                                                                                // SNDL_NotifyCinematicStart(float)+49/w ...
        int cinematicTimestamp;                         // XREF: SNDL_NotifyCinematicStart(float)+62/w
                                                                                // SNDL_NotifyCinematicEnd(void)+54/w
        int cinematicUpdate;                                // XREF: SNDL_NotifyCinematicStart(float)+53/w
                                                                                // SNDL_NotifyCinematicEnd(void)+45/w
        int forcePause;                                         // XREF: SND_UpdatePause+11/r
                                                                                // SND_Init(void)+252/w
        //int playbackIdCounter;                            // XREF: SND_AcquirePlaybackId(void)+3/o
        volatile unsigned int playbackIdCounter;                            // XREF: SND_AcquirePlaybackId(void)+3/o
                                                                                // SND_Init(void)+2A9/w
        snd_fader_t volume;                                 // XREF: SND_UpdateMasterVolumes+D/o
                                                                                // SND_UpdateMasterVolumes+22/r ...
        unsigned int defaultHash;                     // XREF: SND_GetCurrentReverb(void):loc_935ECF/r
                                                                                // SND_Init(void)+8D/w ...
        const snd_curve *defaultCurve;            // XREF: SND_Init(void)+134/w
                                                                                // SND_UpdateSnapshot(float)+9/r
        snd_enveffect envEffects[3];                // XREF: SND_Init(void)+1C1/o
                                                                                // SNDL_SetEnvironmentEffects(int,uint,float,float,int)+15C/o ...
        snd_enveffect *effect;                            // XREF: SND_GetCurrentReverb(void):loc_935EB2/r
                                                                                // SND_GetCurrentReverb(void)+3B/r ...
        snd_listener listeners[1];                    // XREF: SND_ActiveListenerCount(void)+2B/r
                                                                                // SND_GetListenerIndexNearestToOrigin(float const * const)+50/r ...
        unsigned int voiceGroup;                        // XREF: SND_InitGroups+70/w
        unsigned int announcerGroup;                // XREF: SND_LimitVoice+8E/r
                                                                                // SND_LimitVoice+D6/r ...
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        snd_voice_t voice[74];                            // XREF: SND_SetPosition(int,float const * const)+40/o
                                                                                // SND_SetPlaybackIdNotPlayed(int)+3E/w ...
        unsigned int voiceAliasHash[74];        // XREF: SND_ResetVoiceInfo(int)+43/w
                                                                                // SND_SetVoiceStartInfo(int,SndStartAliasInfo *)+85/r ...
        float voicePositionCache[74][3];        // XREF: SND_UpdateVoicePosition+EAA/o
        unsigned int currentContexts[16];     // XREF: SND_GetCurrentContext(uint)+1C/r
                                                                                // SNDL_GameReset(void)+63/o ...
        snd_line_emitter lineEmitters[128]; // XREF: SND_UpdateStaticSounds+3C/r
                                                                                // SND_UpdateStaticSounds+51/r ...
        snd_loop_emitter loopEmitters[256]; // XREF: SND_UpdateStaticSounds+2B6/r
                                                                                // SND_UpdateStaticSounds+2CB/r ...
        float hdrLoad;
        float maximumPriority;                            // XREF: SND_Init(void)+170/w
        snd_fader_t minimumPriority;                // XREF: SND_Init(void)+17E/o
                                                                                // SND_Init(void)+194/o ...
        snd_snapshot_category snapshotCategories[SND_SNAPSHOT_COUNT];
                                                                                // XREF: SND_InitSnapshot(void)+27/o
                                                                                // SND_InitSnapshot(void)+83/o ...
        float snapshotAttenuation[64];            // XREF: SND_UpdateSnapshot(float)+ADE/w
                                                                                // SND_UpdateSnapshot(float)+B60/r ...
        float snapshotOcclusion[64];                // XREF: SND_UpdateSnapshot(float)+AF5/w
                                                                                // SND_UpdateSnapshot(float)+BE9/r ...
        snd_dsp_meters meters[8];                     // XREF: SND_Init(void)+319/o
                                                                                // SDXA2MasterNoVoiceBusEffect::Process(uint,uint,float *)+29/o ...
        unsigned int logCommandCount;             // XREF: SND_LogCommandQHWM(uint)+3/r
                                                                                // SND_LogCommandQHWM(uint)+10/w
        unsigned int logNotifyCount;                // XREF: SND_LogNotifyQHWM(uint)+3/r
                                                                                // SND_LogNotifyQHWM(uint)+10/w
        unsigned int updated;
        snd_ent_state entState[296];                // XREF: SND_FindEntState(SndEntHandle,bool)+2C6/r
                                                                                // SND_FindEntState(SndEntHandle,bool)+2D9/o ...
        snd_ent_state *entStateIndex[1536]; // XREF: SND_FindEntState(SndEntHandle,bool)+175/r
                                                                                // SND_FindEntState(SndEntHandle,bool)+2ED/r ...
        snd_playback playbacks[148];                // XREF: SND_ResetPlaybacks(void)+25/w
                                                                                // SND_AllocatePlayback(void)+67/r ...
        unsigned int playbacksInUse;                // XREF: SND_AllocatePlayback(void)+C5/r
                                                                                // SND_AllocatePlayback(void)+CE/w ...
        int command_init;                                     // XREF: SND_InitCommands(void)+3F/w
                                                                                // SND_GetNewCommand(void)+4/r ...
        snd_command commands[1024];                 // XREF: SND_InitCommands(void)+A/o
                                                                                // SND_GetNewCommand(void)+A7/r ...
        unsigned int command_free;                    // XREF: SND_GetNewCommand(void):loc_94DE3F/r
                                                                                // SND_GetNewCommand(void)+B0/r ...
        unsigned int command_id;                        // XREF: SND_CommandPush(snd_command *):loc_94DFAA/r
                                                                                // SND_CommandPush(snd_command *)+103/r ...
        unsigned int command_id_commit;         // XREF: SND_CommandPump(void)+B2/w
        snd_command *command_q[1024];             // XREF: SND_InitCommands(void)+1E/o
                                                                                // SND_CommandPush(snd_command *)+13A/r ...
        unsigned int command_q_head;                // XREF: SND_InitCommands(void)+2B/w
                                                                                // SND_GetNewCommand(void):loc_94DE10/r ...
        unsigned int command_q_tail;                // XREF: SND_InitCommands(void)+35/w
                                                                                // SND_GetNewCommand(void)+41/r ...
        bool command_process_flag;                    // XREF: SND_InitCommands(void)+49/w
                                                                                // SND_GetProcessCommandFlag(void)+3/r
        // padding byte
        // padding byte
        // padding byte
        snd_notify notifies[512];                     // XREF: SND_GetNewNotify(void)+B0/r
                                                                                // SND_GetNewNotify(void)+C2/o ...
        unsigned int notify_free;                     // XREF: SND_GetNewNotify(void):loc_94F018/r
                                                                                // SND_GetNewNotify(void)+B9/r ...
        snd_notify *notify_q[512];                    // XREF: SND_NotifyPush(snd_notify *)+1A4/r
                                                                                // SND_NotifyPush(snd_notify *)+1DC/w ...
        unsigned int notify_q_head;                 // XREF: SND_GetNewNotify(void):loc_94EFE6/r
                                                                                // SND_NotifyPush(snd_notify *):loc_94F13F/r ...
        unsigned int notify_q_tail;                 // XREF: SND_GetNewNotify(void)+44/r
                                                                                // SND_NotifyPush(snd_notify *)+7D/r ...
        int asyncRunning;
        float proximityOrigin[3];                     // XREF: SND_UpdateProximity(void)+3E/w
                                                                                // SND_UpdateProximity(void)+4E/w ...
        float proximityFront[3];                        // XREF: SND_UpdateProximity(void)+E/w
                                                                                // SND_UpdateProximity(void)+1E/w ...
        float proximityDistance[4];                 // XREF: SND_UpdateProximity(void)+97/r
                                                                                // SND_LosOcclusionCmd(void)+81/o
        float proximityReflectance[4];            // XREF: SND_LosOcclusionCmd(void):loc_94A76C/o
        float proximityLateModifier[4];         // XREF: SD_PreUpdate(int)+8E/r
                                                                                // SD_PreUpdate(int)+9E/r ...
        snd_occlusion_trace_t occlusionTraces[74];
                                                                                // XREF: SND_LosOcclusionCmd(void)+46/o
                                                                                // SND_LosOcclusionUpdate(void)+B8/r ...
        int occlusionRunning;                             // XREF: SND_LosOcclusionCmd(void)+10/r
                                                                                // SND_LosOcclusionThreadMain(uint):loc_94A9F8/r ...
        unsigned int framesSinceLastOcclusionUpdate;
                                                                                // XREF: SND_LosOcclusionUpdate(void):loc_94A9AB/w
                                                                                // SND_LosOcclusionUpdate(void):loc_94A9C1/r ...
        unsigned int traceSequence;                 // XREF: SND_Init(void)+2FE/w
                                                                                // SNDL_GameReset(void)+15D/w ...
        unsigned int lastTraceSequence;         // XREF: SND_Init(void)+308/w
                                                                                // SNDL_GameReset(void)+167/w ...
        unsigned int logTimestamp;                    // XREF: SND_InitLog(void)+67/w
                                                                                // SND_LogLookupAlias(uint,char *)+46/r ...
        unsigned int logSkipPlayedHash[512]; // XREF: SND_InitLog(void)+A/o
                                                                                // SND_LogPlayedAliasId(uint)+34/o
        unsigned int logSkipPlayedTimestamp[512];
                                                                                // XREF: SND_InitLog(void)+1E/o
                                                                                // SND_LogPlayedAliasId(uint):loc_9497EF/o
        unsigned int logSkipMissingHash[128]; // XREF: SND_InitLog(void)+32/o
                                                                                // SND_LogMissingAliasId(uint)+70/o ...
        unsigned int logSkipMissingTimestamp[128];
                                                                                // XREF: SND_InitLog(void)+46/o
                                                                                // SND_LogMissingAliasId(uint):loc_94974B/o ...
        snd_reverse_hash logReverseHash[128]; // XREF: SND_InitLog(void)+5A/o
                                                                                // SND_LogLookupAlias(uint,char *)+35/r ...
};


bool __cdecl SND_ShouldInit();
void __cdecl SND_SetPosition(unsigned int index, float *org);
unsigned int __cdecl SND_ActiveListenerCount();
int __cdecl SND_GetListenerIndexNearestToOrigin(const float *origin);
void __cdecl SND_GetNearestListenerPosition(const float *position, float *listener);
int __cdecl SND_SetPlaybackIdNotPlayed(unsigned int index);
int __cdecl SND_AcquirePlaybackId();
char __cdecl SND_AddLengthNotify(int playbackId, const char *lengthNotifyData, snd_length_type id);
void __cdecl DoLengthNotify(unsigned int msec, const char *lengthNotifyData, snd_length_type id);
void __cdecl SND_StartLengthNotify(unsigned int index, unsigned int totalMsec);
void __cdecl SND_ResetVoiceInfo(int index);
double __cdecl SND_GetSeed(unsigned int key, unsigned int global_age);
void __cdecl SND_SetVoiceStartInfo(unsigned int index, SndStartAliasInfo *SndStartAliasInfo);
float SND_FaderGetGoal(snd_fader_t *fader);
float SND_FaderGetValue(snd_fader_t *fader);
void __cdecl SND_FaderSetRate(snd_fader_t *fader, float r);
void __cdecl SND_FaderSetGoal(snd_fader_t *fader, float g);
bool __cdecl SND_IsAliasPausable(const snd_alias_t *alias);
void __cdecl SND_SetVoiceStartFlux(snd_voice_t *voice, float *player);
void __cdecl SND_SetVoiceStartSeeds(const snd_alias_t *alias, snd_voice_t *voice);
void __cdecl SND_SetVoiceStartFades(float fadetime, snd_voice_t *voice);
void __cdecl SND_FaderSetRateTime(snd_fader_t *fader, float time);
void __cdecl SND_FaderSetValue(snd_fader_t *fader, float v);
void __cdecl SND_FaderUpdate(snd_fader_t *fader, float dt);
void __cdecl SND_UpdateVoicePosition(snd_voice_t *voice, const float *startPosition);
void __cdecl SND_SetSoundFileVoiceInfo(
                unsigned int voiceIndex,
                int srcChannelCount,
                int baserate,
                unsigned int total_msec,
                int start_msec,
                SndFileLoadingState loadingState);
unsigned int __cdecl SND_FindFreeVoice(SndStartAliasInfo *startAliasInfo);
double __cdecl Snd_GetGlobalPriority(const snd_alias_t *alias, float volume);
double __cdecl Snd_GetGlobalPriorityVolume(const snd_alias_t *alias, const float *org);
void __cdecl Snd_GetLowestPriority(float *priority, int *channel, unsigned int start, unsigned int count);
unsigned int __cdecl SND_ContinueLoopingSound(
                unsigned int aliasId,
                float volumeScale,
                SndEntHandle sndEnt,
                float *org,
                int fadeTime,
                snd_playback *playback);
void __cdecl SND_ContinueLoopingSound_Internal(
                unsigned int voiceIndex,
                float volumeScale,
                int fadeTime,
                float *org,
                snd_playback *playback);
unsigned int __cdecl SND_GetCurrentContext(unsigned int type);
snd_alias_t *__cdecl SND_PickSoundAliasFromList(snd_alias_list_t *aliasList, int objectId);
snd_alias_t *__cdecl SND_PickSoundAlias(const char *name, int objectid);
void __cdecl SND_AssertValidData(const snd_alias_t *alias);
unsigned int __cdecl SND_PlaySoundAlias(
                const snd_alias_list_t *aliasList,
                float volumeScale,
                SndEntHandle sndEnt,
                float *org,
                int timeshift,
                int fadeTime,
                const float *direction,
                snd_playback *playback,
                snd_occlusion_start_cache *ocache);
char __cdecl SND_CheckValidSecondary(const snd_alias_t *alias, const snd_alias_t *secondaryAlias);
char __cdecl SND_Limit(
                const char *name,
                unsigned int aliasHash,
                snd_limit_type_t limitType,
                int limitCount,
                SndEntHandle ent,
                bool useEnt,
                float priority);
void __cdecl SND_GetPlayingInfo(
                unsigned int aliasHash,
                int *pcount,
                int *poldest,
                int *least,
                SndEntHandle ent,
                bool useEnt);
char __cdecl SND_LimitVoice(const snd_alias_t *alias, SndEntHandle ent);
bool __cdecl SND_IsAliasVoice(const snd_alias_t *alias);
int __cdecl SND_GetCurrentReverb();
void __cdecl SND_UpdateDebugAlias();
void __cdecl SNDL_Update();
void SND_UpdatePause();
void SND_PauseSounds();
void SND_UnpauseSounds();
void __cdecl SND_UpdateMasterVolumes(float dt);
void __cdecl SND_UpdateVoices(int frametime);
void __cdecl SND_UpdateRoomEffects(int frametime);
void SND_UpdateStaticSounds();
void __cdecl SND_StopVoice(int voiceIndex);
int __cdecl SND_Active();
void __cdecl SND_Init();
void SND_InitGroups();
void __cdecl SND_Shutdown();
void __cdecl SND_ShutdownVoices();
void __cdecl SND_InitSnapshot();
void __cdecl SND_UpdateSnapshot(float dt);
double __cdecl SND_GetPitch(snd_voice_t *voice);
bool __cdecl SND_IsAliasTimescale(const snd_alias_t *alias);
bool __cdecl SND_IsOnSameTeam(unsigned int listenerIndex, SndEntHandle entNum);
double __cdecl SND_GetBaseLevel(const snd_voice_t *voice);
double __cdecl SND_GetDryLevel(const snd_voice_t *voice);
double __cdecl SND_GetWetLevel(const snd_voice_t *voice);
double __cdecl SND_GetDistance(const snd_voice_t *channel);
double __cdecl SND_GetLpfLevel(const snd_voice_t *voice);
double __cdecl I_fmap(float minx, float maxx, float miny, float maxy, float x);
unsigned int __cdecl SND_GetVoiceLength(unsigned int voiceIndex);
void __cdecl SND_FixupStereoPan(snd_speaker_map *pan);
void __cdecl SND_UpdatePanFilter(float dt, snd_voice_t *voice);
void __cdecl SND_UpdateVoice(snd_voice_t *voice, float dt);
double __cdecl Snd_DistanceCurveEval(
                const snd_curve *minCurve,
                const snd_curve *curve,
                float minDist,
                float maxDist,
                float dist);
double __cdecl SND_GetOmni(const snd_voice_t *voice);
snd_voice_t *__cdecl SND_GetPlaybackVoice(int playbackId);
bool __cdecl SND_IsStream(unsigned int index);

bool SND_IsAliasSpatial(const snd_alias_t *alias);
float SND_AliasGetMinPriorityThreshold(const snd_alias_t *alias);
float SND_AliasGetMaxPriorityThreshold(const snd_alias_t *alias);
float SND_AliasGetMaxPriority(const snd_alias_t *alias);
float SND_AliasGetMinPriority(const snd_alias_t *alias);
float SND_AliasGetVolMin(const snd_alias_t *alias);
float SND_AliasGetVolMax(const snd_alias_t *alias);


extern snd_local_t g_snd;