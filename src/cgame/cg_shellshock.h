#pragma once

enum ShockViewTypes : __int32
{                                       // XREF: shellshock_parms_t::<unnamed_type_screenBlend>/r
    SHELLSHOCK_VIEWTYPE_BLURRED = 0x0,
    SHELLSHOCK_VIEWTYPE_FLASHED = 0x1,
    SHELLSHOCK_VIEWTYPE_NONE    = 0x2,
};

struct shellshock_parms_t // sizeof=0x204
{                                       // XREF: .data:shellshock_parms_t * bg_shellshockParms/r
                                        // cgs_t/r ...
    //shellshock_parms_t::<unnamed_type_screenBlend> screenBlend;
    // 00000000 struct shellshock_parms_t::<unnamed_type_screenBlend> // sizeof=0x14
    struct 
    {                                       // XREF: shellshock_parms_t/r
        int blurredFadeTime;                // XREF: CG_DrawActiveFrame(int,int,DemoType,CubemapShot,int,int)+12E2/w
        int blurredEffectTime;              // XREF: CG_DrawActiveFrame(int,int,DemoType,CubemapShot,int,int)+12DC/w
        int flashWhiteFadeTime;
        int flashShotFadeTime;
        ShockViewTypes type;
    } screenBlend;
                                        // XREF: CG_DrawActiveFrame(int,int,DemoType,CubemapShot,int,int)+12DC/w
                                        // CG_DrawActiveFrame(int,int,DemoType,CubemapShot,int,int)+12E2/w
    //shellshock_parms_t::<unnamed_type_view> view;
    struct //shellshock_parms_t::<unnamed_type_view> // sizeof=0xC
    {                                       // XREF: shellshock_parms_t/r
        int fadeTime;
        float kickRate;
        float kickRadius;
    } view;

    //shellshock_parms_t::<unnamed_type_sound> sound;
    struct //shellshock_parms_t::<unnamed_type_sound> // sizeof=0x1A0
    {                                       // XREF: shellshock_parms_t/r
        bool affect;
        char loop[64];
        char loopSilent[64];
        char end[64];
        char endAbort[64];
        // padding byte
        // padding byte
        // padding byte
        int fadeInTime;
        int fadeOutTime;
        float drylevel;
        float wetlevel;
        char roomtype[64];
        int modEndDelay;
        int loopFadeTime;
        int loopEndDelay;
        char snapshot[64];
    } sound;

    //shellshock_parms_t::<unnamed_type_lookControl> lookControl;
    struct //shellshock_parms_t::<unnamed_type_lookControl> // sizeof=0x14
    {                                       // XREF: shellshock_parms_t/r
        bool affect;
        // padding byte
        // padding byte
        // padding byte
        int fadeTime;
        float mouseSensitivity;
        float maxPitchSpeed;
        float maxYawSpeed;
    } lookControl;

    //shellshock_parms_t::<unnamed_type_movement> movement;
    struct //shellshock_parms_t::<unnamed_type_movement> // sizeof=0x4
    {                                       // XREF: shellshock_parms_t/r
        float scale;
    } movement;

    //shellshock_parms_t::<unnamed_type_animation> animation;
    struct //shellshock_parms_t::<unnamed_type_animation> // sizeof=0x1
    {                                       // XREF: shellshock_parms_t/r
        bool affect;
    } animation;
    // padding byte
    // padding byte
    // padding byte
    //shellshock_parms_t::<unnamed_type_visionSet> visionSet;
    struct //shellshock_parms_t::<unnamed_type_visionSet> // sizeof=0x28
    {                                       // XREF: shellshock_parms_t/r
        char name[32];
        int inTime;
        int outTime;
    } visionSet;
};

struct shellshock_t // sizeof=0x24
{                                       // XREF: cg_s/r
    const shellshock_parms_t *parms;
    int startTime;
    int duration;
    int loopEndTime;
    float sensitivity;
    float viewDelta[2];
    int hasSavedScreen;
    int visionSetStarted;
};

struct cg_s;

void __cdecl CG_PerturbCamera(cg_s *cgameGlob);
int __cdecl CG_DrawShellShockSavedScreenBlendBlurred(
                int localClientNum,
                const shellshock_parms_t *parms,
                int start,
                int duration);
void __cdecl SaveScreenToBuffer(int localClientNum);
int __cdecl CG_DrawShellShockSavedScreenBlendFlashed(
                int localClientNum,
                const shellshock_parms_t *parms,
                int start,
                int duration);
double __cdecl BlendSmooth(float percent);
void __cdecl EndShellShock(int localClientNum);
void __cdecl EndShellShockSound(int localClientNum);
void __cdecl EndShellShockLookControl(int localClientNum);
void __cdecl EndShellShockCamera(int localClientNum);
void __cdecl EndShellShockScreen(int localClientNum);
void __cdecl CG_UpdateShellShock(int localClientNum, const shellshock_parms_t *parms, int start, int duration);
void __cdecl UpdateShellShockSound(int localClientNum, const shellshock_parms_t *parms, int time, int duration);
void __cdecl UpdateShellShockLookControl(int localClientNum, const shellshock_parms_t *parms, int time, int duration);
void __cdecl UpdateShellShockCamera(int localClientNum, const shellshock_parms_t *parms, int time, int duration);
double __cdecl CubicInterpolate(float t, float x0, float x1, float x2, float x3);
void __cdecl UpdateShellShockVisionSet(int localClientNum, const shellshock_parms_t *parms, int time, int duration);
void __cdecl CG_StartShellShock(cg_s *cgameGlob, const shellshock_parms_t *parms, int start, int duration);
bool __cdecl CG_Flashbanged(int localClientNum);
bool __cdecl CG_Flared(int localClientNum);
