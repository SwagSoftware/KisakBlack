#pragma once
#include <xanim/dobj.h>
#include <universal/dvar.h>
#include <cgame_mp/cg_predict_mp.h>
#include <universal/offline.h>

struct __declspec(align(8)) UIViewer // sizeof=0xA16C0
{                                       // XREF: .data:uiViewer/r
    enum Mode : __int32
    {                                       // XREF: UIViewer/r
        MODE_PLAYER_FIRST             = 0x0,
        MODE_PLAYER                   = 0x0,
        MODE_EDIT_PLAYER_HEAD         = 0x1,
        MODE_EDIT_PLAYER_BODY         = 0x2,
        MODE_EDIT_PLAYER_FACE_PATTERN = 0x3,
        MODE_EDIT_PLAYER_FACE_PATTERN_COLOR = 0x4,
        MODE_CHOOSE_CLASS_PLAYER      = 0x5,
        MODE_COMBAT_RECORD            = 0x6,
        MODE_WEAPON_FIRST             = 0x7,
        MODE_WEAPON                   = 0x7,
        MODE_EDIT_WEAPON              = 0x8,
        MODE_EDIT_RETICLE             = 0x9,
        MODE_EDIT_RETICLE_COLOR       = 0xA,
        MODE_EDIT_LENS                = 0xB,
        MODE_EDIT_CAMO                = 0xC,
        MODE_EDIT_ATTACHMENT_TOP      = 0xD,
        MODE_EDIT_ATTACHMENT_BOTTOM   = 0xE,
        MODE_EDIT_ATTACHMENT_TRIGGER  = 0xF,
        MODE_EDIT_ATTACHMENT_MUZZLE   = 0x10,
        MODE_EDIT_TAG                 = 0x11,
        MODE_EDIT_EMBLEM              = 0x12,
        MODE_AAR_WEAPON               = 0x13,
        MODE_CHOOSE_CLASS_WEAPON      = 0x14,
        MODE_WEAPON_LAST              = 0x14,
        MODE_MAX                      = 0x15,
    };
    enum WeaponSlot : __int32
    {                                       // XREF: UIViewer::WeaponParams/r
        WEAPON_PRIMARY   = 0x0,
        WEAPON_SECONDARY = 0x1,
        WEAPON_SLOT_MAX  = 0x2,
    };
    enum //UIViewer::<unnamed_tag> : __int32
    {
        MAX_DOBJ_DRAW        = 0x1,
        DOBJ_POOL_SIZE       = 0x1,
        NUM_CAMERA_OVERRIDES = 0x100,
    };

    struct CameraParams // sizeof=0x18
    {                                       // XREF: UIViewer/r UIViewer/r ...
        float lookAt[3];
        float dist;
        float yaw;                          // XREF: UIViewer::DrawDobj::Draw(float,float)+220/r
        float pitch;

        void __thiscall SetRefDef(refdef_s *refdef);
    };
    struct PlayerParams // sizeof=0x10
    {                                       // XREF: UIViewer::State/r
        int bodyIndex;                      // XREF: UIViewer::Update(float)+1B7/w
        int headIndex;                      // XREF: UIViewer::Update(float)+1D9/w
        int facePatternIndex;               // XREF: UIViewer::Update(float)+25D/w
        int faceColorIndex;                 // XREF: UIViewer::Update(float)+283/w

        PlayerParams();
        PlayerParams(int initialValue) :
            bodyIndex(initialValue),
            headIndex(initialValue),
            facePatternIndex(initialValue),
            faceColorIndex(initialValue)
        {
        }
    };
    struct WeaponParams // sizeof=0x20
    {                                       // XREF: UIViewer::State/r
        UIViewer::WeaponSlot weaponSlot;    // XREF: UIViewer::Update(float)+2A3/w
        int weaponIndex;                    // XREF: UIViewer::Update(float):loc_782377/r
        int attachTopIndex;                 // XREF: UIViewer::Update(float)+2A9/w
        int attachBottomIndex;              // XREF: UIViewer::Update(float)+2AC/w
        int attachTriggerIndex;             // XREF: UIViewer::Update(float)+2AF/w
        int attachMuzzleIndex;              // XREF: UIViewer::Update(float)+2B2/w
        renderOptions_s weaponOptions;      // XREF: UIViewer::Update(float)+2B5/w
        int currentAttachmentPoint;         // XREF: UIViewer::Update(float)+2B8/w

        WeaponParams(UIViewer::WeaponSlot slot);
        WeaponParams() = default;
        WeaponParams(int initialValue)
            : weaponSlot((UIViewer::WeaponSlot)initialValue),
            weaponIndex(initialValue),
            attachTopIndex(initialValue),
            attachBottomIndex(initialValue),
            attachTriggerIndex(initialValue),
            attachMuzzleIndex(initialValue),
            weaponOptions(initialValue),
            currentAttachmentPoint(initialValue)
        {
        }
    };
    struct __declspec(align(4)) State // sizeof=0x58
    {                                       // XREF: UIViewer/r
        int mode;                           // XREF: UIViewer::AddPlayerToScene(UIViewer::PlayerParams const &,UIViewer::WeaponParams const &,bool)+23/w
        char faction[32];                   // XREF: UIViewer::AddPlayerToScene(UIViewer::PlayerParams const &,UIViewer::WeaponParams const &,bool)+5B/o
        UIViewer::PlayerParams playerParams;
        UIViewer::WeaponParams weaponParams;
        bool locked;                        // XREF: UIViewer::AddPlayerToScene(UIViewer::PlayerParams const &,UIViewer::WeaponParams const &,bool)+56/w

        State();

        void __thiscall Invalidate();
        bool __thiscall operator!=(const UIViewer::State *o);
        bool __thiscall HasPlayerChanged(const UIViewer::State *o);
    };
    struct DrawDobj // sizeof=0x20C
    {                                       // XREF: UIViewer/r
        enum //UIViewer::DrawDobj::<unnamed_tag> : __int32
        {
            SHADER_CONST_CLAN_NAME_1 = 0x0,
            SHADER_CONST_CLAN_NAME_2 = 0x1,
            SHADER_CONST_CLAN_NAME_3 = 0x2,
            SHADER_CONST_FACEPAINT   = 0x3,
            SHADER_CONST_HERO_R      = 0x4,
            SHADER_CONST_HERO_G      = 0x5,
            SHADER_CONST_HERO_B      = 0x6,
            SHADER_CONST_MAX         = 0x7,
        };
        DObj *dobj;                         // XREF: UIViewer::GetDObj(int,int)+C/r
        const WeaponVariantDef *weaponVariantDef;
        cpose_t pose;
        float yaw;
        bool depthHack;
        // padding byte
        // padding byte
        // padding byte
        renderOptions_s weaponOptions;
        bool heroLighting;
        // padding byte
        // padding byte
        // padding byte
        int startTime;
        bool locked;
        // padding byte
        // padding byte
        // padding byte
        ShaderConstantSet constantSet;


        void __thiscall Init();
        void __thiscall Set(
            const UIViewer::WeaponParams *weapParam,
            const WeaponVariantDef *wvd,
            bool hero);
        void __thiscall Set(
            const UIViewer::PlayerParams *playerParams,
            const UIViewer::WeaponParams *weapParam,
            const WeaponVariantDef *wvd,
            bool hero,
            bool lock);
        char __thiscall Draw(float deltaTime, float lightMultiplier);
    };

    struct CameraParamOverrides // sizeof=0x50
    {                                       // XREF: UIViewer/r
        char weapon[20];
        char attachment[20];
        char params[32];
        int row;
        int col;
    };

    bool inited;                        // XREF: _dynamic_initializer_for__uiViewer__+6/w
    bool streamerInited;
    bool mapLoaded;
    bool show;
    bool deferredShow;
    bool ingame;
    // padding byte
    // padding byte
    UIViewer::Mode mode;
    bool cameraAnim;
    // padding byte
    // padding byte
    // padding byte
    int cameraAnimStartTime;
    UIViewer::CameraParams camera;      // XREF: UIViewer::DrawDobj::Draw(float,float)+220/r
    UIViewer::CameraParams cameraAnimStart;
    UIViewer::CameraParams cameraAnimEnd;
    char playerCameraName[64];
    const dvar_s *loaded;
    const dvar_s *streaming;
    const dvar_s *showDvar;
    const dvar_s *modeDvar;
    const dvar_s *ingameDvar;
    const dvar_s *customname;
    const dvar_s *equipWeapon;
    const dvar_s *weaponDvar;
    const dvar_s *attachmentDvar;
    const dvar_s *playerCameraDvar;
    const dvar_s *cameraPitchRange;
    const dvar_s *allowCameraPitch;
    const dvar_s *lightMultiplier;
    const dvar_s *fov;
    const dvar_s *highlightColor1;
    const dvar_s *highlightColor2;
    const dvar_s *heroHighlight;        // XREF: UIViewer::DrawDobj::Draw(float,float)+129/r
    const dvar_s *sceneOrigin;          // XREF: UIViewer::CameraParams::SetRefDef(refdef_s &)+E7/r
    const dvar_s *cameraControl;
    const dvar_s *viewerMsg;
    const dvar_s *dumpAnims;
    const dvar_s *timeScale;
    const dvar_s *drawAttachPoints;     // XREF: UIViewer::DrawDobj::Draw(float,float)+2BB/r
    UIViewer::State prevState;          // XREF: _dynamic_initializer_for__uiViewer__+D/o
    float prevSceneOrigin[3];
    char dobjsBuffer[124];
    DObj *dobjsPool[1];
    int nextDobj;
    UIViewer::DrawDobj drawDobjs[1];    // XREF: UIViewer::GetDObj(int,int)+C/r
    int nDrawDobjs;
    int controllerIndex;                // XREF: UIViewer::SetControllerIndexCmd(void)+3/w
    XAnimTree_s *animTree;
    pmove_t pmove;                      // XREF: _dynamic_initializer_for__uiViewer__+17/o
                                        // _dynamic_atexit_destructor_for__uiViewer__+3/w
    playerState_s ps;
    int currentPlayerWeapon;
    unsigned int playerWeaponHistory[4];
    int nextPlayerWeaponHistory;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    bgsAnim_s bgsAnim;
    bgs_t bgs;
    const StringTable *cameraPosTable;
    const StringTable *bodyHeadTable;
    char lastCameraWeapon[32];          // XREF: UIViewer::ResetCameraCmd(void)+A/o
    char lastCameraAttachment[32];      // XREF: UIViewer::ResetCameraCmd(void)+5/o
    UIViewer::CameraParamOverrides cameraOverrides[256];
    int nCameraOverrides;
    // padding byte
    // padding byte
    // padding byte
    // padding byte


    void __thiscall Init(bool _ingame);
    void __thiscall Shutdown();
    void __thiscall LoadMap();
    void __thiscall Show();
    void __thiscall Hide();
    void __thiscall UpdateCamera(refdef_s *refdef, float deltaTime);
    DObj *__thiscall AllocDobj(unsigned int *entnum);
    centity_s *__thiscall GetPlayerEntity();
    XAnimTree_s *__thiscall GetAnimTree();
    bgs_t *__thiscall GetBGS();
    void __thiscall FreeDobjs();
    int __thiscall GetLoadout(loadoutSlot_t loadoutSlot, bool defaultIfZero);

    void __thiscall Update(float deltaTime);
    void __thiscall DrawScene(unsigned int eyeToRender);
    const WeaponVariantDef *__thiscall GetWeaponVariantDef(const WeaponParams *weapParams);
    const WeaponVariantDef *__thiscall AddWeaponXmodel(const WeaponParams *weapParams, DObjModel_s *dobjModel, bool world);
    const WeaponVariantDef *__thiscall AddWeaponXmodel(const WeaponVariantDef *weaponVariantDef, DObjModel_s *dobjModel, bool world);
    void __thiscall HideWeaponTags(DObj *dobj, const WeaponVariantDef *weapon, unsigned int *partBits, bool dualWield);
    void __thiscall HideWeaponTags(DObj *dobj, const unsigned __int16 *hideTags, unsigned int *partBits);
    void __thiscall UnhideWeaponTags(DObj *dobj, const unsigned __int16 *hideTags, unsigned int *partBits);
    char __thiscall AddBodyPart(const char *partName, const char *partType, DObjModel_s *dobjModel, const char *bone);
    void __thiscall UpdatePlayerDObj(const PlayerParams *playerParams, const WeaponVariantDef *weaponVariantDef, const WeaponParams *weapParams, bool locked);
    void __thiscall UpdatePlayerAnim(const WeaponVariantDef *weaponVariantDef);
    const char *__thiscall GetFaction();
    void __thiscall AddPlayerToScene(const PlayerParams *playerParams, const WeaponParams *weapParams, bool locked);
    int *__thiscall GetAttachmentIndexPtr(WeaponParams *weapParams);
    int __thiscall GetAttachmentIndex(WeaponParams *weapParams);
    void __thiscall SetAttachmentIndex(WeaponParams *weapParams, int index);
    void __thiscall ClearAttachmentIndex(WeaponParams *weapParams);
    const char *__thiscall GetAttachmentSlotName();
    void __thiscall AddWeaponToScene(
        WeaponParams *weapParams,
        WeaponParams *baseWeapParams);
    char __thiscall SetCameraPos(char *string, bool animate);
    char __thiscall SetCameraPos(char *weapon, char *attachment, bool animate);
    void __thiscall SetCameraPos(
        const float *lookat,
        float dist,
        float yaw,
        float pitch,
        bool animate);
    void __thiscall InitFov();
    static void __cdecl SetCameraPosCmd();
    static void __cdecl ResetCameraCmd();
    static void __cdecl SetControllerIndexCmd();
    static DObj *__cdecl GetDObj(int handle);
    void __thiscall Draw(int localClientNum, bool _ingame, unsigned int eyeToRender);
    void __thiscall SetupStreamer();
};

void __cdecl UI_ViewerRotateLeftRepeatEnabled_f();
void __cdecl UI_ViewerRotateRightRepeatEnabled_f();
void __cdecl ForceLoadWeapon(XAssetHeader header);
void __cdecl UI_ViewerDraw(unsigned int eyeToRender);
void __cdecl UI_ViewerShutdown();
void __cdecl UI_ViewerCheckStreamer();
eAttachmentPoint __cdecl GetCurrentAttachPoint();

