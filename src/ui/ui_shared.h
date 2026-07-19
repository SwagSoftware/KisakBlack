#pragma once
#include <gfx_d3d/r_font.h>
#include <universal/com_expressions.h>
#include <database/database.h>

enum language_t : __int32
{                                       // XREF: Item_ListBox_PaintTextElem/r
                                        // Item_ListBox_LanguageScale/r
    LANGUAGE_ENGLISH        = 0x0,
    LANGUAGE_FRENCH         = 0x1,
    LANGUAGE_FRENCHCANADIAN = 0x2,
    LANGUAGE_GERMAN         = 0x3,
    LANGUAGE_AUSTRIAN       = 0x4,
    LANGUAGE_ITALIAN        = 0x5,
    LANGUAGE_SPANISH        = 0x6,
    LANGUAGE_BRITISH        = 0x7,
    LANGUAGE_RUSSIAN        = 0x8,
    LANGUAGE_POLISH         = 0x9,
    LANGUAGE_KOREAN         = 0xA,
    LANGUAGE_JAPANESE       = 0xB,
    LANGUAGE_CZECH          = 0xC,
    MAX_LANGUAGES           = 0xD,
};

enum itemGroup_t : __int32
{                                       // XREF: itemInfo_t/r
                                        // LiveStats_UpdateGroupStatsMilestones/r ...
    ITEMGROUP_SMG                = 0x0,
    ITEMGROUP_FIRST              = 0x0,
    ITEMGROUP_ASSAULT            = 0x1,
    ITEMGROUP_CQB                = 0x2,
    ITEMGROUP_LMG                = 0x3,
    ITEMGROUP_SNIPER             = 0x4,
    ITEMGROUP_PISTOL             = 0x5,
    ITEMGROUP_LAUNCHER           = 0x6,
    ITEMGROUP_SPECIAL            = 0x7,
    ITEMGROUP_SHOTGUN            = 0x8,
    ITEMGROUP_EXPLOSIVE          = 0x9,
    ITEMGROUP_GRENADE            = 0xA,
    ITEMGROUP_MASTERKEY          = 0xB,
    ITEMGROUP_GRENADELAUNCHER    = 0xC,
    ITEMGROUP_FLAMETHROWER       = 0xD,
    ITEMGROUP_SPECIALTY          = 0xE,
    ITEMGROUP_SPECIALGRENADE     = 0xF,
    ITEMGROUP_INVENTORY          = 0x10,
    ITEMGROUP_HEAD               = 0x11,
    ITEMGROUP_HEADGEAR           = 0x12,
    ITEMGROUP_BODY               = 0x13,
    ITEMGROUP_MISCWEAPON         = 0x14,
    ITEMGROUP_FEATURE            = 0x15,
    ITEMGROUP_DEATHSTREAK        = 0x16,
    ITEMGROUP_GLOBAL_ITEMS_START = 0x17,
    ITEMGROUP_KILLSTREAK         = 0x17,
    ITEMGROUP_COUNT              = 0x18,
    ITEMGROUP_INVALID            = -1,
};
inline itemGroup_t &operator++(itemGroup_t &t)
{
    t = static_cast<itemGroup_t>((static_cast<int>(t) + 1));
    return t;
}
inline itemGroup_t operator++(itemGroup_t &t, int)
{
    itemGroup_t old = t;
    t = static_cast<itemGroup_t>((static_cast<int>(t) + 1));
    return old;
}

struct ScreenPlacement;
struct ScreenPlacementStack;

enum UILocalVarType : __int32
{                                                                             // XREF: UILocalVar/r
    UILOCALVAR_INT = 0x0,
    UILOCALVAR_FLOAT = 0x1,
    UILOCALVAR_STRING = 0x2,
};

struct UILocalVar // sizeof=0xC
{                                                                             // XREF: UILocalVarContext/r
    UILocalVarType type;
    const char *name;
    //UILocalVar::<unnamed_type_u> u;
    union //UILocalVar::<unnamed_type_u> // sizeof=0x4
    {                                                                             // XREF: UILocalVar/r
        int integer;
        float value;
        const char *string;
    } u;
};

struct UILocalVarContext // sizeof=0xC00
{                                                                             // XREF: UiContext/r
    UILocalVar table[256];
};

struct BlurStackEntry // sizeof=0x40
{                                                                             // XREF: UiContext/r
    char menuName[64];
};

struct serverFilter_s // sizeof=0x8
{                                       // XREF: .rdata:serverFilter_s const * const serverFilters/r
                                        // serverFilter_t/r
    const char *description;            // XREF: UI_GetServerFilter(int)+33/r
    const char *basedir;                // XREF: UI_GetServerFilter(int)+3A/r
};

struct rectDef_s // sizeof=0x18
{                                                                             // XREF: .data:rect/r
    float x;                                                        // XREF: CG_CompassDrawRound(int,rectDef_s const *,Material *,float * const)+A6/w
    float y;                                                        // XREF: CG_CompassDrawRound(int,rectDef_s const *,Material *,float * const)+AC/w
    float w;                                                        // XREF: CG_CompassDrawRound(int,rectDef_s const *,Material *,float * const)+B2/w
    float h;                                                        // XREF: CG_CompassDrawRound(int,rectDef_s const *,Material *,float * const)+B8/w
    int horzAlign;                                            // XREF: CG_CompassDrawRound(int,rectDef_s const *,Material *,float * const)+BE/w
    int vertAlign;                                            // XREF: CG_CompassDrawRound(int,rectDef_s const *,Material *,float * const)+C4/w
};

struct columnInfo_s // sizeof=0x20
{                                                                             // XREF: listBoxDef_s/r
    int elementStyle;
    int maxChars;
    rectDef_s rect;
};

struct windowDef_t // sizeof=0xA4
{                                                                             // XREF: menuDef_t/r
    const char *name;
    rectDef_s rect;
    rectDef_s rectClient;
    const char *group;
    unsigned __int8 style;
    unsigned __int8 border;
    unsigned __int8 modal;
    unsigned __int8 frameSides;
    float frameTexSize;
    float frameSize;
    int ownerDraw;
    int ownerDrawFlags;
    float borderSize;
    int staticFlags;
    int dynamicFlags[1];
    int nextTime;
    float foreColor[4];
    float backColor[4];
    float borderColor[4];
    float outlineColor[4];
    float rotation;
    Material *background;
};

struct rectData_s // sizeof=0x40
{                                                                             // XREF: rectData_t/r
    ExpressionStatement rectXExp;
    ExpressionStatement rectYExp;
    ExpressionStatement rectWExp;
    ExpressionStatement rectHExp;
};

struct animParamsDef_t // sizeof=0x6C
{                                                                             // XREF: UIAnimInfo/r
                                                                                // UIAnimInfo/r ...
    const char *name;
    rectDef_s rectClient;
    float borderSize;
    float foreColor[4];
    float backColor[4];
    float borderColor[4];
    float outlineColor[4];
    float textScale;
    float rotation;
    struct GenericEventHandler *onEvent;
};

struct UIAnimInfo // sizeof=0xEC
{
    int animStateCount;
    animParamsDef_t **animStates;
    animParamsDef_t currentAnimState;
    animParamsDef_t nextAnimState;
    int animating;
    int animStartTime;
    int animDuration;
};

struct ScriptCondition // sizeof=0x10
{                                                                             // XREF: ScriptConditionNext/r
    bool fireOnTrue;
    // padding byte
    // padding byte
    // padding byte
    int constructID;
    int blockID;
    ScriptCondition *next;
};

struct __declspec(align(8)) menuDef_t // sizeof=0x190
{                                                                             // XREF: XAssetPoolEntry<menuDef_t>/r
    windowDef_t window;
    const char *font;
    int fullScreen;
    int ui3dWindowId;
    int itemCount;
    int fontIndex;
    int cursorItem[1];
    int fadeCycle;
    int priority;
    float fadeClamp;
    float fadeAmount;
    float fadeInAmount;
    float blurRadius;
    int openSlideSpeed;
    int closeSlideSpeed;
    int openSlideDirection;
    int closeSlideDirection;
    rectDef_s initialRectInfo;
    int openFadingTime;
    int closeFadingTime;
    int fadeTimeCounter;
    int slideTimeCounter;
    GenericEventHandler *onEvent;
    struct ItemKeyHandler *onKey;
    ExpressionStatement visibleExp;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 showBits;
    unsigned __int64 hideBits;
    const char *allowedBinding;
    const char *soundName;
    int imageTrack;
    int control;
    float focusColor[4];
    float disableColor[4];
    ExpressionStatement rectXExp;
    ExpressionStatement rectYExp;
    struct itemDef_s **items;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct menuClientNum_t // sizeof=0x8
{                                                                             // XREF: UiContext/r
    menuDef_t *menu;
    int localClientNum;
};

enum uiMenuCommand_t : __int32
{                                                                             // XREF: uiInfo_s/r
    UIMENU_NONE = 0x0,
    UIMENU_MAIN = 0x1,
    UIMENU_INGAME = 0x2,
    UIMENU_PREGAME = 0x3,
    UIMENU_POSTGAME = 0x4,
    UIMENU_WM_QUICKMESSAGE = 0x5,
    UIMENU_SCRIPT_POPUP = 0x6,
    UIMENU_SCOREBOARD = 0x7,
    UIMENU_GAMERCARD = 0x8,
    UIMENU_MUTEERROR = 0x9,
    UIMENU_ENDOFGAME = 0xA,
};

struct MenuList // sizeof=0xC
{                                                                             // XREF: $CD64A558AFC89A5F4974E935559855BB/r
                                                                                // XAssetPoolEntry<MenuList>/r
    const char *name;
    int menuCount;                                            // XREF: Menu_New+7F/r
    // Menu_New:loc_77E40A/r ...
    menuDef_t **menus;                                    // XREF: UI_LoadMenu_LoadObj+17/w
    // Menu_New+9F/r ...
};

struct ConversionArguments // sizeof=0x28
{                                                                             // XREF: ?LiveNews_PopulateFriendNews@@YAXH_KPAD@Z/r
    int argCount;                                             // XREF: CG_DrawWarText:loc_4D6B37/w
    const char *args[9];                                // XREF: CG_DrawWarText+325/w
};

struct Material;
struct Font_s;
struct snd_alias_list_t;

struct pc_token_s // sizeof=0x410
{                                       // XREF: pc_token_t/r
    int type;                           // XREF: PC_Int_Parse:loc_7774F4/r
    int subtype;                        // XREF: PC_EventScript_Parse+6FC/r
    int intvalue;                       // XREF: PC_Int_Parse+AC/r
    float floatvalue;                   // XREF: PC_Float_Parse+B2/r
    char string[1024];                  // XREF: PC_String_Parse:loc_77741E/o
};

struct CachedAssets_t // sizeof=0x58
{                                       // XREF: sharedUiInfo_t/r
    Material *scrollBarArrowUp;         // XREF: UI_AssetCache+4E/w
    Material *scrollBarArrowDown;       // XREF: UI_AssetCache+3A/w
    Material *scrollBarArrowLeft;       // XREF: UI_AssetCache+62/w
                                        // Item_ListBox_Paint+F4/r
    Material *scrollBarArrowRight;      // XREF: UI_AssetCache+76/w
                                        // Item_ListBox_Paint+1C5/r
    Material *scrollBar;                // XREF: UI_AssetCache+26/w
                                        // Item_ListBox_Paint+160/r ...
    Material *scrollBarThumb;           // XREF: UI_AssetCache+8A/w
                                        // Item_ListBox_Paint:loc_77292A/r ...
    Material *sliderBar;                // XREF: UI_AssetCache+9E/w
    Material *sliderThumb;              // XREF: UI_AssetCache+B2/w
                                        // UI_DrawStatsMilestonesFeederProgressBar(int,int,rectDef_s const *,int,int)+2C7/r ...
    Material *whiteMaterial;            // XREF: UI_FillRectPhysical(float,float,float,float,float const * const)+3/r
                                        // UI_FillRectPhysical(float,float,float,float,float const * const)+C/r ...
    Material *cursor;                   // XREF: UI_Refresh(int)+355/r
                                        // UI_AssetCache+102/w
    Material *dvarlistArrowLeft;        // XREF: UI_AssetCache+C6/w
                                        // Item_Text_Paint+364/r
    Material *dvarlistArrowRight;       // XREF: UI_AssetCache+DA/w
                                        // Item_Text_Paint+3D3/r
    Material *line;                     // XREF: UI_DrawPrettyLine(int,float * const,float * const,float,int,int,float * const)+24A/r
                                        // UI_AssetCache+EE/w
    Material *blur;                     // XREF: UI_Project_AssetCache(void)+12/w
                                        // UI_DrawBlurMaterial+AB/r
    Material *lineGraph;                // XREF: UI_Project_AssetCache(void)+26/w
                                        // UI_DrawLineGraphSegment(int,float * const,float * const,rectDef_s *)+113/r
    Font_s *bigFont;                    // XREF: UI_GetFontHandleDefault(float)+5E/r
                                        // UI_GetFontHandle(ScreenPlacement const *,int,float)+A/r ...
    Font_s *smallFont;                  // XREF: UI_GetFontHandleDefault(float)+1C/r
                                        // UI_GetFontHandle(ScreenPlacement const *,int,float)+1A/r ...
    Font_s *consoleFont;                // XREF: UI_GetFontHandle(ScreenPlacement const *,int,float)+3A/r
                                        // UI_AssetCache+13E/w
    Font_s *boldFont;                   // XREF: UI_GetFontHandle(ScreenPlacement const *,int,float)+BC/r
                                        // UI_AssetCache+152/w
    Font_s *textFont;                   // XREF: UI_GetFontHandleDefault(float):loc_745F05/r
                                        // UI_GetFontHandle(ScreenPlacement const *,int,float)+2A/r ...
    Font_s *extraBigFont;               // XREF: UI_GetFontHandleDefault(float)+3D/r
                                        // UI_GetFontHandle(ScreenPlacement const *,int,float)+4A/r ...
    snd_alias_list_t *itemFocusSound;
};

struct gameTypeInfo // sizeof=0x74
{                                       // XREF: sharedUiInfo_t/r
                                        // sharedUiInfo_t/r ...
    char gameType[12];                  // XREF: UI_FeederItemText_CustomGametypes(int)+21/o
                                        // UI_FeederItemText_GametypesBase(int,int,int,int,itemDef_s *,listBoxDef_s *,Material * *)+82/o ...
    char gameTypeName[32];              // XREF: UI_GetGameTypeDisplayShortName(char const *)+44/o
                                        // UI_GetGameTypesList_FastFile+165/o ...
    char gameTypeShortName[32];         // XREF: UI_GetGameTypeDisplayNameCaps(char const *)+44/o
                                        // UI_GetGameTypesList_FastFile+1EB/o ...
    char gameTypeNameCaps[32];          // XREF: UI_LoadCustomMatchGameTypes(void)+13C/w
                                        // UI_LoadCustomMatchGameTypes(void)+14E/w
    int splitscreen;                    // XREF: UI_GetBasicTrainingGameTypeCount+30/r
                                        // UI_GetBasicTrainingGametypeIdForNum+30/r ...
    int basictraining;                  // XREF: UI_Project_FeederCount(int,int,float,listBoxDef_s *):$LN42_6/r
};

struct customGameTypeInfo // sizeof=0xF0
{                                       // XREF: sharedUiInfo_t/r
    char gameType[12];
    char gameTypeName[32];
    char gameTypeDesc[128];
    char fileName[32];
    char gameTypeNameCaps[32];
    int splitscreen;                    // XREF: UI_Project_FeederCount(int,int,float,listBoxDef_s *):$LN41_9/r
                                        // UI_Project_FeederCount(int,int,float,listBoxDef_s *):loc_734656/r ...
};

struct mapInfo // sizeof=0x130
{                                       // XREF: sharedUiInfo_t/r
    char mapName[32];                   // XREF: UI_GetMapLoadNameForCurrentIndex(int)+3B/o
                                        // UI_GetMapLoadNameForCurrentIndex(int)+5C/o ...
    char mapLoadName[24];               // XREF: UI_LoadArenas(void)+1C3/o
                                        // UI_LoadMaps(void)+23F/o ...
    char mapNameCaps[32];               // XREF: UI_LoadMaps(void)+27C/w
                                        // UI_LoadMaps(void)+291/w
    int splitscreen;                    // XREF: UI_FeederDoubleClickDlcMaps(int,int,int)+1E/o
                                        // UI_LoadMaps(void)+1F9/o
    char mapPackType[16];               // XREF: UI_FeederDoubleClickDlcMaps(int,int,int)+C/r
                                        // UI_LoadMaps(void)+214/w
    int mapPackTypeIndex;               // XREF: UI_LoadArenas(void)+20E/o
                                        // UI_GetLevelShot(int)+38/o
    char imageName[42];
    // padding byte
    // padding byte
    const char *opponentName;
    int teamMembers;                    // XREF: UI_LoadArenas(void)+25B/w
                                        // UI_LoadArenas(void)+305/r ...
    int typeBits;
    int timeToBeat[32];                 // XREF: UI_LoadArenas(void)+1DD/w
                                        // UI_GetLevelShot(int)+24/r ...
    Material *levelShot;                // XREF: UI_GetListIndexFromMapIndex(int)+6E/r
                                        // UI_MapCountByGameType(void)+3E/w ...
    int active;                         // XREF: UI_LoadMaps(void)+105/o
};

struct contentPackInfo // sizeof=0x10
{                                       // XREF: sharedUiInfo_t/r
    char mapPackName[16];
};

struct modInfo_t // sizeof=0x8
{                                       // XREF: sharedUiInfo_t/r
    const char *modName;                // XREF: UI_FeederItemText_Mods+17/r
                                        // UI_FeederItemText_Mods+24/r ...
    const char *modDescr;               // XREF: UI_FeederCount(int,int,float,listBoxDef_s *):$LN22_13/r
                                        // UI_FeederItemText_Mods+C/r ...
};

struct serverStatusInfo_s // sizeof=0x14A4
{                                       // XREF: $D349527935E9920CD1D59819DFE4C1D2/r
                                        // sharedUiInfo_t/r ...
    char address[64];
    const char *lines[128][8];          // XREF: UI_FeederCount(int,int,float,listBoxDef_s *):$LN19_12/r
                                        // UI_FeederItemText_ServerStatus+C/r ...
    char text[1024];
    char pings[96];
    int numLines;                       // XREF: UI_BuildFindPlayerList(void)+F6/r
};

struct pendingServer_t // sizeof=0x8C
{                                       // XREF: pendingServerStatus_t/r
    char adrstr[64];
    char name[64];
    int startTime;
    int serverNum;
    int valid;
};

struct pendingServerStatus_t // sizeof=0x8C4
{                                       // XREF: sharedUiInfo_t/r
    int num;
    pendingServer_t server[16];         // XREF: UI_FeederCount(int,int,float,listBoxDef_s *):$LN18_16/r
};

struct pinglist_t // sizeof=0x44
{                                       // XREF: serverStatus_s/r
    char adrstr[64];
    int start;
};

struct serverStatus_s // sizeof=0x14180
{                                       // XREF: $A48C1D473939E002B61DABFCB910C7B1/r
                                        // serverStatus_t/r
    pinglist_t pingList[16];
    int numqueriedservers;
    int currentping;
    int nextpingtime;
    int maxservers;
    int refreshtime;
    int numServers;
    int sortKey;
    int sortDir;
    int lastCount;
    int refreshActive;
    int currentServer;
    int displayServers[20000];
    int numDisplayServers;
    int serverCount;
    int numPlayersOnServers;
    int nextDisplayRefresh;
    int nextSortTime;
    Material *currentServerPreview;
    int motdLen;
    int motdWidth;
    int motdPaintX;
    int motdPaintX2;
    int motdOffset;
    int motdTime;
    char motd[1024];
    unsigned __int8 displayServersFriendNameInd[100];
};

struct sharedUiInfo_t // sizeof=0x261D0
{                                       // XREF: .data:sharedUiInfo_t sharedUiInfo/r
    CachedAssets_t assets;              // XREF: UI_FillRectPhysical(float,float,float,float,float const * const)+3/r
                                        // UI_FillRectPhysical(float,float,float,float,float const * const)+C/r ...
    // padding byte                     // XREF: UI_FeederCount(int,int,float,listBoxDef_s *):loc_72DF92/r
                                        // UI_BuildPlayerList+6A/w ...
    // padding byte
    // padding byte
    // padding byte
    int playerCount;                    // XREF: UI_BuildPlayerList+C9/o
                                        // UI_BuildPlayerList+E0/o ...
    char playerNames[32][32];
    char teamNames[32][32];             // XREF: UI_BuildPlayerList+5D/o
                                        // UI_BuildPlayerList+B3/w ...
    int playerClientNums[32];           // XREF: UI_LoadArenas(void)+2CA/r
                                        // UI_GetGameTypeDisplayName(char const *)+19/r ...
    int numGameTypes;                   // XREF: UI_LoadArenas(void)+2D8/o
                                        // UI_GetGameTypeDisplayName(char const *)+27/o ...
    gameTypeInfo gameTypes[32];         // XREF: UI_OverrideCursorPos_CustomGametypes(int,listBoxDef_s *)+B/r
                                        // UI_GetGameTypeDisplayName(char const *)+44/o ...
    int numCustomGameTypes;
    customGameTypeInfo customGameTypes[32];
                                        // XREF: UI_Project_FeederCount(int,int,float,listBoxDef_s *):$LN41_9/r
                                        // UI_Project_FeederCount(int,int,float,listBoxDef_s *):loc_734656/r ...
    int numCustomMatchGameTypes;        // XREF: UI_FeederItemText_GametypesBase(int,int,int,int,itemDef_s *,listBoxDef_s *,Material * *)+109/o
                                        // UI_FeederItemText_GametypesBase(int,int,int,int,itemDef_s *,listBoxDef_s *,Material * *)+17F/o ...
    gameTypeInfo customMatchGameTypes[32];
                                        // XREF: UI_GetBasicTrainingGameTypeCount+30/r
                                        // UI_FeederItemText_CustomGametypes(int)+21/o ...
    int gameTypeMapCount[32];           // XREF: UI_OwnerDrawWidth(int,Font_s *,float)+5C/r
                                        // UI_JoinGameType_HandleKey(int,int):loc_7548AD/r ...
    int numJoinGameTypes;               // XREF: UI_BuildServerDisplayList(int,uiInfo_s *,int)+784/o
                                        // UI_GetGameTypesList(void)+29/o ...
    gameTypeInfo joinGameTypes[32];     // XREF: UI_Project_FeederCount(int,int,float,listBoxDef_s *):$LN42_6/r
                                        // UI_FeederItemText_Maps(int,itemDef_s *,int,int,Material * *)+65/r ...
    int mapCount;                       // XREF: UI_FeederItemText_Maps(int,itemDef_s *,int,int,Material * *)+76/o
                                        // UI_LoadArenas(void)+18F/o ...
    mapInfo mapList[128];               // XREF: UI_FeederDoubleClickDlcMaps(int,int,int)+C/r
                                        // UI_FeederDoubleClickDlcMaps(int,int,int)+1E/o ...
    contentPackInfo contentPackList[8];
    Material *serverHardwareIconList[10];
                                        // XREF: UI_FeederItemText_Mods+43/r
                                        // UI_RunMenuScript(int,int,char const * *,char const *)+5FE/r ...
    modInfo_t modList[64];              // XREF: UI_FeederCount(int,int,float,listBoxDef_s *):$LN22_13/r
                                        // UI_FeederItemText_Mods+C/r ...
    int modCount;                       // XREF: UI_FeederSelection(int,int,float,int)+152/w
                                        // UI_RunMenuScript(int,int,char const * *,char const *)+2DD/w ...
    int modIndex;                       // XREF: BG_UnlockablesPurchaseCurrentItemAttachmentCmd+9/r
                                        // BG_UnlockablesPurchaseCurrentItemCmd+3/r ...
    int itemIndex;                      // XREF: UI_Project_FeederCount(int,int,float,listBoxDef_s *)+203/w
                                        // UI_FeederItemText_ItemInSlot+B8/w ...
    int itemNum;                        // XREF: UI_FeederItemText_CommonListColorHandler(int,bool,bool,int)+1B/w
                                        // UI_FeederItemText_CommonListColorHandler(int,bool,bool,int)+63/w ...
    float itemColor[4];                 // XREF: BG_UnlockablesPurchaseCurrentItemAttachmentCmd+3/r
                                        // BG_UnlockablesEquipClassCurrentAttachmentCmd+B8/r ...
    int attachmentNum;                  // XREF: UI_Project_FeederCount(int,int,float,listBoxDef_s *)+13F/w
                                        // UI_Project_FeederCount(int,int,float,listBoxDef_s *)+144/r ...
    int numAttachments;                 // XREF: UI_Project_FeederCount(int,int,float,listBoxDef_s *)+19D/w
                                        // UI_Project_FeederCount(int,int,float,listBoxDef_s *)+1AC/r ...
    int numItemsInSlot;                 // XREF: BG_UnlockablesPurchaseCurrentItemOptionCmd+D/r
                                        // BG_UnlockablesEquipClassCurrentOptionCmd+C1/r ...
    int optionIndex;                    // XREF: GetSortedStatsForFriendOrSelf+12A/r
                                        // ChangeSortedStatsPivot+33/r ...
    int sortedItemPivot;                // XREF: GetSortedStatsForFriendOrSelf+94/r
                                        // GetSortedStatsForFriendOrSelf+ED/r ...
    int numSortedItems;                 // XREF: BG_UnlockablesPurchaseClanTagFeatureCmd+3/r
                                        // UI_FeederSelection(int,int,float,int)+1E4/w ...
    int clanTagFeature;
    //$A48C1D473939E002B61DABFCB910C7B1 ___u31;
    union //$A48C1D473939E002B61DABFCB910C7B1 // sizeof=0x1566C
    {                                       // XREF: UI_FeederCount(int,int,float,listBoxDef_s *)+E8/r
                                            // UI_FeederCount(int,int,float,listBoxDef_s *):loc_72DF42/r ...
        //char serverStatus[82460];
        serverStatus_s serverStatus;
        char cuckery[82460]; // dont ask

        //$D349527935E9920CD1D59819DFE4C1D2 __s1;
    };

    //struct
    //{
        char serverStatusAddress[64];
        char serverStatusSecurityID[8];
        serverStatusInfo_s serverStatusInfo;
    //};
                                        // XREF: UI_FeederCount(int,int,float,listBoxDef_s *)+E8/r
                                        // UI_FeederCount(int,int,float,listBoxDef_s *):loc_72DF42/r ...
    serverStatusInfo_s serverStatusInfoScoreBoard;
                                        // XREF: UI_FeederCount(int,int,float,listBoxDef_s *):$LN19_12/r
                                        // UI_FeederItemText_ServerStatus+C/r ...
    int nextServerStatusRefresh;
    int nextServerStatusScoreBoardRefresh;
    pendingServerStatus_t pendingServerStatus;
                                        // XREF: UI_FeederCount(int,int,float,listBoxDef_s *):$LN18_16/r
                                        // UI_FeederItemText_ServerStatusScoreboard+C/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 visibilityBits[1];
    unsigned __int64 localVisibilityBits[1];
    int contractIndex;
    Material *loadingScreen;
};

struct cursor_t // sizeof=0x8
{                                       // XREF: UiContext/r
    float x;
    float y;
};

struct scrollInfo_s // sizeof=0x20
{                                       // XREF: .data:scrollInfo/r
                                        // scrollInfo_t/r
    int nextScrollTime;                 // XREF: Item_StartCapture+70/w
    int nextAdjustTime;                 // XREF: Item_StartCapture+81/w
    int adjustValue;                    // XREF: Item_StartCapture+86/w
    int scrollKey;                      // XREF: Item_StartCapture+93/w
                                        // Item_StartCapture+E3/w ...
    float xStart;                       // XREF: Item_StartCapture+F9/w
                                        // Item_StartCapture+180/w
    float yStart;                       // XREF: Item_StartCapture+109/w
                                        // Item_StartCapture+190/w
    itemDef_s *item;                    // XREF: Item_StartCapture+B1/w
                                        // Item_StartCapture+EB/w ...
    int scrollDir;                      // XREF: Item_StartCapture+A8/w
};

struct UiContext // sizeof=0x1828
{                                       // XREF: .data:uiInfo_s * uiInfoArray/r
                                        // uiInfo_s/r ...
    int contextIndex;
    float bias;
    int realTime;
    int frameTime;
    cursor_t cursor;
    cursor_t prevCursor;
    int isCursorVisible;
    int screenWidth;
    int screenHeight;
    float screenAspect;
    float FPS;
    float blurRadiusOut;
    BlurStackEntry blurMenuStack[8];
    int blurMenuStackIndex;
    menuDef_t *Menus[600];
    int menuCount;
    menuClientNum_t menuStack[16];
    int openMenuCount;
    bool enableSlideAndFadeEffectForMenu;
    // padding byte
    // padding byte
    // padding byte
    UILocalVarContext localVars;
};

struct uiInfo_s // sizeof=0x2764
{                                                                             // XREF: uiInfo_t/r
    UiContext uiDC;
    int playerRefresh;
    int playerIndex;
    int timeIndex;
    int previousTimes[4];
    uiMenuCommand_t currentMenuType;
    bool allowScriptMenuResponse;
    char toastPopupTitle[256];
    char toastPopupDesc[256];
    char toastPopupIconName[256];
    // padding byte
    // padding byte
    // padding byte
    int toastPopupDuration;
    int toastPopupTimeCounter;
    bool toastPopupOpened;
    // padding byte
    // padding byte
    // padding byte
    int contractIndex;
    char findPlayerName[1024];
    char foundPlayerServerAddresses[16][64];
    char foundPlayerServerNames[16][64];
    int numFoundPlayerServers;
    int nextFindPlayerRefresh;
};

struct __declspec(align(4)) nestingStackElem_t // sizeof=0xC
{                                       // XREF: nestingStack_t/r
    int constructID;
    int blockID;
    bool outcome;
    // padding byte
    // padding byte
    // padding byte
};

struct nestingStack_t // sizeof=0x304
{                                       // XREF: ?Item_RunSingleEventScript@@YAXHPAUUiContext@@PAUitemDef_s@@PAUGenericEventScript@@@Z/r
    int top;                            // XREF: Item_RunSingleEventScript(int,UiContext *,itemDef_s *,GenericEventScript *)+9/w
    nestingStackElem_t members[64];
};

struct GenericEventScript // sizeof=0x2C
{                                                                             // XREF: GenericEventScriptNext/r
    ScriptCondition *prerequisites;
    ExpressionStatement condition;
    int type;
    bool fireOnTrue;
    // padding byte
    // padding byte
    // padding byte
    const char *action;
    int blockID;
    int constructID;
    GenericEventScript *next;
};


struct MenuCell // sizeof=0xC
{                                                                             // XREF: ItemParse_menuItemsDef/r
    int type;                                                     // XREF: ItemParse_menuItemsDef+375/r
    int maxChars;                                             // XREF: ItemParse_menuItemsDef+340/r
    // ItemParse_menuItemsDef+38A/r
    char *stringValue;
};

struct MenuRow // sizeof=0x18
{
    MenuCell *cells;
    char *eventName;
    char *onFocusEventName;
    bool disableArg;
    // padding byte
    // padding byte
    // padding byte
    int status;
    int name;
};

struct listBoxDef_s // sizeof=0x29C
{                                                                             // XREF: listBoxDef_t/r
    int mousePos;
    int cursorPos[1];
    int startPos[1];
    int endPos[1];
    int drawPadding;
    float elementWidth;
    float elementHeight;
    int numColumns;
    float special;
    columnInfo_s columnInfo[16];
    int notselectable;
    int noScrollBars;
    int usePaging;
    float selectBorder[4];
    float disableColor[4];
    float focusColor[4];
    float elementHighlightColor[4];
    float elementBackgroundColor[4];
    Material *selectIcon;
    Material *backgroundItemListbox;
    Material *highlightTexture;
    int noBlinkingHighlight;
    MenuRow *rows;
    int maxRows;
    int rowCount;
};

struct multiDef_s // sizeof=0x18C
{                                                                             // XREF: multiDef_t/r
    const char *dvarList[32];
    const char *dvarStr[32];
    float dvarValue[32];
    int count;
    int actionOnEnterPressOnly;
    int strDef;
};

struct editFieldDef_s // sizeof=0x24
{                                                                             // XREF: editFieldDef_t/r
    int cursorPos[1];
    float minVal;
    float maxVal;
    float defVal;
    float range;
    int maxChars;
    int maxCharsGotoNext;
    int maxPaintChars;
    int paintOffset;
};

struct enumDvarDef_s // sizeof=0x4
{                                                                             // XREF: enumDvarDef_t/r
    const char *enumDvarName;
};

union focusDefData_t // sizeof=0x4
{                                                                             // XREF: focusItemDef_s/r
    listBoxDef_s *listBox;
    multiDef_s *multi;
    editFieldDef_s *editField;
    enumDvarDef_s *enumDvar;
    void *data;
};

struct focusItemDef_s // sizeof=0x18
{                                                                             // XREF: focusItemDef_t/r
    const char *mouseEnterText;
    const char *mouseExitText;
    const char *mouseEnter;
    const char *mouseExit;
    struct ItemKeyHandler *onKey;
    focusDefData_t focusTypeData;
};

union textDefData_t // sizeof=0x4
{                                                                             // XREF: textDef_s/r
    focusItemDef_s *focusItemDef;
    struct gameMsgDef_s *gameMsgDef;
    void *data;
};


struct textDef_s // sizeof=0x44
{                                                                             // XREF: textDef_t/r
    rectDef_s textRect[1];
    int alignment;
    int fontEnum;
    int itemFlags;
    int textAlignMode;
    float textalignx;
    float textaligny;
    float textscale;
    int textStyle;
    const char *text;
    struct textExp_s *textExpData;
    textDefData_t textTypeData;
};

struct imageDef_s // sizeof=0x10
{                                                                             // XREF: imageDef_t/r
    ExpressionStatement materialExp;
};

struct ownerDrawDef_s // sizeof=0x10
{                                                                             // XREF: ownerDrawDef_t/r
    ExpressionStatement dataExp;
};

union itemDefData_t // sizeof=0x4
{                                                                             // XREF: itemDef_s/r
    textDef_s *textDef;
    imageDef_s *imageDef;
    focusItemDef_s *blankButtonDef;
    ownerDrawDef_s *ownerDrawDef;
    void *data;
};

struct GenericEventHandler // sizeof=0xC
{                                                                             // XREF: GenericEventHandlerNext/r
    const char *name;
    GenericEventScript *eventScript;
    GenericEventHandler *next;
};

struct ItemKeyHandler // sizeof=0xC
{                                                                             // XREF: ItemKeyHandlerNext/r
    int key;
    GenericEventScript *keyScript;
    ItemKeyHandler *next;
};

struct __declspec(align(8)) itemDef_s // sizeof=0x110
{                                                                             // XREF: itemDef_t/r
                                                                                // Menu_RunLeaveFocusScript/r ...
    windowDef_t window;
    int type;
    int dataType;
    int imageTrack;
    const char *dvar;
    const char *dvarTest;
    const char *enableDvar;
    int dvarFlags;
    itemDefData_t typeData;
    menuDef_t *parent;
    rectData_s *rectExpData;
    ExpressionStatement visibleExp;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 showBits;
    unsigned __int64 hideBits;
    ExpressionStatement forecolorAExp;
    int ui3dWindowId;
    GenericEventHandler *onEvent;
    UIAnimInfo *animInfo;                             // XREF: Menu_RunLeaveFocusScript+10/w
    // Menu_RunFocusScript+10/w ...
// padding byte
// padding byte
// padding byte
// padding byte
};

void __cdecl Menu_Setup(UiContext *dc);
void __cdecl LerpColor(float *a, float *b, float *c, float t);
int __cdecl Color_Parse(const char **p, float (*c)[4]);
int __cdecl String_Parse(const char **p, char *out, int len);
bool __cdecl Command_Parse(char **p, char *out, int len);
bool __cdecl Rect_ContainsPoint(int contextIndex, const rectDef_s *rect, float x, float y);
itemDef_s *__cdecl Menu_GetMatchingItemByNumber(menuDef_t *menu, int index, char *name);
void __cdecl Script_SetColor(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_SetBackground(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
itemDef_s *__cdecl Menu_FindItemByName(menuDef_t *menu, const char *p);
void __cdecl UI_UpdateContextItem(
                UiContext *dc,
                const char *menuName,
                const char *itemName,
                float offsetX,
                float offsetY);
void __cdecl UI_UpdateContextItemInternal(
                UiContext *dc,
                const char *menuName,
                const char *itemName,
                float offsetX,
                float offsetY);
void __cdecl Item_SnapRectClientToMouse(UiContext *dc, itemDef_s *item, float x, float y);
void __cdecl UI_AnimateItem(
                int localClientNum,
                UiContext *dc,
                const char *menuName,
                const char *itemName,
                const char *newStateName,
                int milliseconds);
void __cdecl UI_AnimateItemInternal(
                int localClientNum,
                UiContext *dc,
                const char *menuName,
                const char *itemName,
                const char *newStateName,
                int milliseconds,
                bool sysTime);
animParamsDef_t *__cdecl Item_FindStateByName(itemDef_s *item, const char *p);
void __cdecl Item_AnimateToState(
                int localClientNum,
                UiContext *dc,
                itemDef_s *item,
                animParamsDef_t *state,
                int milliseconds,
                bool sysTime);
void __cdecl Item_CompleteAnimation(itemDef_s *item);
void __cdecl Item_UpdateAnimation(int localClientNum, UiContext *dc, itemDef_s *item);
void __cdecl Item_LerpAnimationParameters(int localClientNum, int contextIndex, itemDef_s *item, float lerpValue);
void __cdecl UI_AnimateItemSysTime(
                int localClientNum,
                UiContext *dc,
                const char *menuName,
                const char *itemName,
                const char *newStateName,
                int milliseconds);
void __cdecl UI_SetShaderTime(int localClientNum);
void __cdecl Script_SetItemColor(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Item_SetColor(int contextIndex, itemDef_s *item, const char *name, float *color);
int __cdecl Menu_ItemsMatchingGroup(menuDef_t *menu, char *name);
int __cdecl Menus_MenuIsInStack(UiContext *dc, menuDef_t *menu);
int __cdecl Menus_MenuInStackLocalClientNum(UiContext *dc, menuDef_t *menu);
menuDef_t *__cdecl Menus_FindByName(const UiContext *dc, const char *p);
void __cdecl Menus_HideByName(const UiContext *dc, const char *menuName);
void __cdecl Menus_ShowByName(const UiContext *dc, const char *windowName);
void __cdecl Menus_CloseByName(int localClientNum, UiContext *dc, const char *p);
void __cdecl Menus_Close(int localClientNum, UiContext *dc, menuDef_t *menu);
void    Menu_RunLeaveFocusScript(
                int localClientNum,
                UiContext *dc,
                menuDef_t *menu);
itemDef_s *__cdecl Menu_ClearFocus(int localClientNum, UiContext *dc, menuDef_t *menu);
int __cdecl Menus_RemoveFromStack(int localClientNum, UiContext *dc, menuDef_t *pMenu);
void __cdecl Menu_GainFocusDueToClose(int localClientNum, UiContext *dc, menuDef_t *menu);
void __cdecl Menu_CallOnFocusDueToOpen(int localClientNum, UiContext *dc, menuDef_t *menu);
void    Menu_RunFocusScript(int localClientNum, UiContext *dc, menuDef_t *menu);
void    Menu_RunCloseScript(int localClientNum, UiContext *dc, menuDef_t *menu);
void __cdecl Menus_CompleteItemAnimations(int localClientNum, UiContext *dc, menuDef_t *menu);
void __cdecl Menus_ClearFlagsOnClose(int localClientNum, UiContext *dc, menuDef_t *menu);
void __cdecl Menus_CloseImmediateByName(int localClientNum, UiContext *dc, const char *p);
void __cdecl Menus_RemoveMenu(XAssetHeader header);
char __cdecl Item_DoesEventExist(itemDef_s *item, const char *eventName);
char __cdecl Menus_DoesEventExist(menuDef_t *menu, const char *eventName);
void __cdecl Menus_SetFocusToItem(int localClientNum, UiContext *dc, const char *menuName, const char *itemName);
itemDef_s *__cdecl Menu_FindFirstFocusableItemByName(int localClientNum, UiContext *dc, menuDef_t *menu, const char *p);
void __cdecl Menus_CloseAll(int localClientNum, UiContext *dc);
void __cdecl Menus_CloseAllBehindMain(int localClientNum, UiContext *dc);
void __cdecl UI_ClearErrors();
void __cdecl Script_Show(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Menu_ShowItemByName(int contextIndex, menuDef_t *menu, char *p, int bShow);
void __cdecl Item_Show(int contextIndex, itemDef_s *item, int bShow);
void __cdecl Script_Hide(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_FadeIn(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Menu_FadeItemByName(int contextIndex, menuDef_t *menu, char *p, int fadeOut);
void __cdecl Item_Fade(int contextIndex, itemDef_s *item, int fadeOut);
void __cdecl Script_FadeOut(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_ShowMenu(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_HideMenu(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_Open(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_OpenImmediate(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_OpenForGameType(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_CloseForGameType(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_ActivateBlur(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
char __cdecl RemoveMenuFromBlurStack(int localClientNum, UiContext *dc, const char *menuName);
void __cdecl Script_DeactivateBlur(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_SetDvarStringUsingTable(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
const char *__cdecl Script_TableLookupParse(UiContext *dc, const char **args);
void __cdecl Script_OpParse(int *operand, const char **args);
int __cdecl Script_IntParse(UiContext *dc, const char **args);
void __cdecl Script_Close(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_CloseImmediate(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_InGameOpen(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_InGameClose(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_FocusFirstInMenu(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
itemDef_s *__cdecl Menu_FocusFirstSelectableItem(int localClientNum, UiContext *dc, menuDef_t *menu);
int __cdecl Menu_GetCursorItem(int contextIndex, menuDef_t *menu);
void __cdecl Script_SetFocus(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_SetFocusByDvar(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_ChangeState(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_SetDvar(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_ResetDvar(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_ExecKeyHandler(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_ExecKeyPress(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_Exec(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_ExecHandler(
                int localClientNum,
                int controllerIndex,
                itemDef_s *item,
                char **args,
                void (__cdecl *textCallback)(int, int, itemDef_s *, const char *));
void __cdecl Script_AddTextWrapper(int localClientNum, int controllerIndex, itemDef_s *__formal, const char *text);
void __cdecl Script_ExecNow(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_ExecDvar(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_ExecOnDvarStringValue(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_ConditionalExecHandler(
                int localClientNum,
                UiContext *dc,
                itemDef_s *item,
                char **args,
                bool (__cdecl *shouldExec)(const char *, const char *),
                void (__cdecl *textCallback)(int, int, itemDef_s *, const char *));
inline void __cdecl Script_ConditionalExecHandler(
    int localClientNum,
    UiContext *dc,
    itemDef_s *item,
    const char **args,
    bool(__cdecl *shouldExec)(const char *, const char *),
    void(__cdecl *textCallback)(int, int, itemDef_s *, const char *))
{
    Script_ConditionalExecHandler(localClientNum, dc, item, (char **)args, shouldExec, textCallback);
}
bool __cdecl Script_ExecIfStringsEqual(const char *dvarValue, const char *testValue);
void __cdecl Script_ExecOnDvarIntValue(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
bool __cdecl Script_ExecIfIntsEqual(const char *dvarValue, const char *testValue);
void __cdecl Script_ExecOnDvarFloatValue(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
bool __cdecl Script_ExecIfFloatsEqual(const char *dvarValue, const char *testValue);
void __cdecl Script_ExecNowOnDvarStringValue(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_ExecNowOnDvarIntValue(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_ExecNowOnDvarFloatValue(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_RespondOnDvarStringValue(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_ConditionalResponseHandler(
                int localClientNum,
                UiContext *dc,
                itemDef_s *item,
                const char **args,
                bool (__cdecl *shouldRespond)(const char *, const char *));
void __cdecl Script_RespondOnDvarIntValue(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_RespondOnDvarFloatValue(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_SetLocalVarBool(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
UILocalVarContext *__cdecl Script_ParseLocalVar(UiContext *dc, const char **args);
void __cdecl Script_SetLocalVarInt(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_SetLocalVarFloat(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_SetLocalVarString(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_FeederTop(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_FeederBottom(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_SetDvarFromLocString(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_Play(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_ScriptMenuResponse(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
void __cdecl Script_SetUIVisibilityBit(int localClientNum, UiContext *dc, itemDef_s *item, const char **args);
bool __cdecl Item_IsOutcomeAccepted(bool outcome, bool fireOnTrue);
char __cdecl Item_ArePrereqsSatisfied(GenericEventScript *eventScript, nestingStack_t *stack);
void __cdecl Item_MarkProcessed(GenericEventScript *eventScript, nestingStack_t *stack, bool isConditionSatisfied);
char __cdecl Item_IsBlockProcessed(GenericEventScript *eventScript, nestingStack_t *stack, bool *result);
void __cdecl Item_RunSingleEventScript(
                int localClientNum,
                UiContext *dc,
                itemDef_s *item,
                GenericEventScript *eventScript);
bool __cdecl Item_IsConditionSatisfied(
                int localClientNum,
                UiContext *dc,
                itemDef_s *item,
                GenericEventScript *eventScript);
void __cdecl Item_RunEventScript(int localClientNum, UiContext *dc, itemDef_s *item, const char *eventName);
void __cdecl Item_RunScript(int localClientNum, UiContext *dc, itemDef_s *item, const char *s);
void __cdecl UIAnimation_RunEventScript(
                int localClientNum,
                UiContext *dc,
                itemDef_s *item,
                animParamsDef_t *animState,
                const char *eventName);
void __cdecl UI_RegisterCmds();
void __cdecl UI_SetLocalVarBool_f();
void __cdecl UI_SetLocalVarInt_f();
void __cdecl UI_SetLocalVarFloat_f();
void __cdecl UI_SetLocalVarString_f();
void __cdecl UI_FadeItem_f();
void __cdecl UI_ShowItem_f();
void __cdecl UI_HideItem_f();
void __cdecl UI_ShowMenu_f();
void __cdecl UI_HideMenu_f();
void __cdecl UI_OpenMenu_f();
void __cdecl UI_CloseMenu_f();
void __cdecl UI_OpenMenuImmediate_f();
void __cdecl UI_CloseMenuImmediate_f();
void __cdecl UI_ChangeMenuOpenSlideDirection_f();
void __cdecl UI_ChangeMenuCloseSlideDirection_f();
void __cdecl UI_OpenToastPopup_f();
void __cdecl UI_FocusItem_f();
void __cdecl UI_MoveFeeder_f();
void __cdecl UI_AddToFeeder_f();
void __cdecl UI_AddToFeederExtended_f();
void __cdecl UI_ClearFeeder_f();
void __cdecl UI_ClearFeederWithoutResetCursor_f();
void __cdecl UI_RaiseFeederEvent_f();
void __cdecl UI_RefreshFeeder_f();
void __cdecl UI_RefreshFeederSelection_f();
void __cdecl UI_ChangeRowStatus_f();
void __cdecl UI_PlaySound_f();
void __cdecl UI_ValidatePrivateMatchGametype_f();
void __cdecl UI_SetActiveMenu_f();
int __cdecl Item_SetFocus(int localClientNum, UiContext *dc, itemDef_s *item, float x, float y);
textDef_s *__cdecl Item_GetTextRect(int contextIndex, const itemDef_s *item);
const char *__cdecl Item_ListBox_MaxScroll(int localClientNum, int contextIndex, itemDef_s *item);
int __cdecl Item_ListBox_Viewmax(int localClientNum, int contextIndex, itemDef_s *item);
bool __cdecl Window_IsHorizontal(const windowDef_t *w);
double __cdecl Item_ListBox_GetMaxHeight(int localClientNum, int contextIndex, itemDef_s *item);
bool __cdecl Item_ListBox_RequiresScroll(int localClientNum, int contextIndex, itemDef_s *item);
void __cdecl Item_ListBox_SetCursorPos(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                int max,
                int viewmax,
                int newCursorPos,
                bool autoScroll);
void __cdecl Item_ListBox_Scroll(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                int max,
                int scrollmax,
                int viewmax,
                int delta);
bool __cdecl Item_IsTextField(const itemDef_s *item);
void __cdecl Item_TextField_BeginEdit(int localClientNum, int contextIndex, itemDef_s *item);
void __cdecl Menus_Open(int localClientNum, UiContext *dc, menuDef_t *menu);
void __cdecl Menus_AddToStack(int localClientNum, UiContext *dc, menuDef_t *pMenu);
void __cdecl Menu_LoseFocusDueToOpen(int localClientNum, UiContext *dc, menuDef_t *menu);
int __cdecl Menus_OpenByName(int localClientNum, UiContext *dc, const char *p);
int __cdecl Menus_OpenImmediateByName(int localClientNum, UiContext *dc, const char *p);
void    Menus_SetupOpenMenu(int localClientNum, UiContext *dc, menuDef_t *menu);
itemDef_s *__cdecl Menu_SetNextCursorItem(
                int localClientNum,
                UiContext *dc,
                menuDef_t *menu,
                int usedTab,
                int usedKeyboard);
int __cdecl Item_HasFocusEvent(int localClientNum, UiContext *dc, itemDef_s *item);
void __cdecl Menus_PrintAllLoadedMenus(UiContext *dc);
int    Menu_CheckOnKey(int localClientNum, UiContext *dc, menuDef_t *menu, int key);
void    Menu_HandleKey(int localClientNum, UiContext *dc, menuDef_t *menu, int key, int down);
itemDef_s *__cdecl Item_LoseFocus(int localClientNum, UiContext *dc, itemDef_s *item);
bool __cdecl Item_IsModal(itemDef_s *item);
int __cdecl Item_ListBox_HandleKey(int localClientNum, UiContext *dc, itemDef_s *item, int key, int down, int force);
void __cdecl Item_ListBox_Page(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                int max,
                int scrollmax,
                int viewmax,
                int delta);
bool __cdecl Item_TextField_HandleKey(int localClientNum, UiContext *dc, itemDef_s *item, int key);
int __cdecl Menu_GetItemIndex(menuDef_t *menu, itemDef_s *item);
void __cdecl Item_TextField_EnsureCursorVisible(int contextIndex, itemDef_s *item, const char *text);
int __cdecl Item_HandleKey(int localClientNum, UiContext *dc, itemDef_s *item, int key, int down);
int __cdecl Item_OwnerDraw_HandleKey(itemDef_s *item, int key);
int __cdecl Item_YesNo_HandleKey(UiContext *dc, itemDef_s *item, int key);
bool __cdecl Item_ShouldHandleKey(UiContext *dc, itemDef_s *item, int key);
bool __cdecl Item_ContainsMouse(UiContext *dc, itemDef_s *item);
int __cdecl Item_Multi_HandleKey(UiContext *dc, itemDef_s *item, int key);
int __cdecl Item_Multi_CountSettings(itemDef_s *item);
int __cdecl Item_Multi_FindDvarByValue(itemDef_s *item);
int __cdecl Item_List_NextEntryForKey(int key, int current, int count);
int __cdecl Item_DvarEnum_HandleKey(UiContext *dc, itemDef_s *item, int key);
int __cdecl Item_DvarEnum_CountSettings(itemDef_s *item);
int __cdecl Item_DvarEnum_EnumIndex(itemDef_s *item);
void __cdecl Item_StartCapture(int localClientNum, UiContext *dc, itemDef_s *item, int key);
int __cdecl Item_Slider_OverSlider(int contextIndex, itemDef_s *item, float x, float y);
double __cdecl Item_Slider_ThumbPosition(itemDef_s *item);
double __cdecl Item_GetRectPlacementX(int alignX, float x0, float containerWidth, float selfWidth);
int __cdecl Item_ListBox_OverLB(int localClientNum, int contextIndex, itemDef_s *item, float x, float y);
double __cdecl Item_ListBox_ScrollHeight(int localClientNum, int contextIndex, itemDef_s *item);
double __cdecl Item_ListBox_ThumbPosition(int localClientNum, int contextIndex, itemDef_s *item);
void __cdecl Scroll_ListBox_AutoFunc(int localClientNum, UiContext *dc, void *p);
void __cdecl Scroll_ListBox_ThumbFunc(int localClientNum, UiContext *dc, scrollInfo_s *p);
void __cdecl Scroll_Slider_ThumbFunc(int localClientNum, UiContext *dc, itemDef_s **p);
void __cdecl Scroll_Slider_SetThumbPos(UiContext *dc, itemDef_s *item);
int __cdecl Item_Slider_HandleKey(UiContext *dc, itemDef_s *item, int key);
void __cdecl Item_Action(int localClientNum, UiContext *dc, itemDef_s *item);
itemDef_s *__cdecl Menu_SetPrevCursorItem(int localClientNum, UiContext *dc, menuDef_t *menu, int usedKeyboard);
void __cdecl Menus_HandleOOBClick(int localClientNum, UiContext *dc, menuDef_t *menu, int key, int down);
int __cdecl Display_VisibleMenuCount(UiContext *dc);
rectDef_s *__cdecl Item_CorrectedTextRect(int contextIndex, itemDef_s *item);
const rectDef_s *__cdecl Item_GetNonTextRect(int contextIndex, const itemDef_s *item);
char __cdecl Menu_ItemsAreAnimating(int localClientNum, UiContext *dc, menuDef_t *menu);
void __cdecl Item_TextColor(UiContext *dc, itemDef_s *item, float (*newColor)[4]);
void __cdecl Fade(
                int *flags,
                float *f,
                float clamp,
                int *nextTime,
                int offsetTime,
                int bFlags,
                float fadeAmount,
                float fadeInAmount,
                UiContext *dc);
double __cdecl Item_GetTextPlacementX(int alignX, float x0, float containerWidth, float selfWidth);
double __cdecl Item_GetTextPlacementY(int alignY, float y0, float containerHeight, float selfHeight);
int __cdecl UI_PickWordWrapLineWidth(
                const char *text,
                int bufferSize,
                Font_s *font,
                float normalizedScale,
                int targetLineCount,
                int widthGuess,
                int widthLimit);
void __cdecl UI_DrawWrappedText(
                const ScreenPlacement *scrPlace,
                const char *text,
                const rectDef_s *rect,
                Font_s *font,
                float x,
                float y,
                float scale,
                const float *color,
                int style,
                char textAlignMode,
                rectDef_s *textRect);
void __cdecl DrawWrappedText(
                const ScreenPlacement *scrPlace,
                const char *text,
                const rectDef_s *rect,
                Font_s *font,
                float x,
                float y,
                float scale,
                const float *color,
                int style,
                char textAlignMode,
                rectDef_s *textRect,
                bool subtitle,
                const float *subtitleGlowColor,
                bool cinematic);
void __cdecl UI_DrawWrappedTextSubtitled(
                const ScreenPlacement *scrPlace,
                const char *text,
                const rectDef_s *rect,
                Font_s *font,
                float x,
                float y,
                float scale,
                const float *color,
                int style,
                char textAlignMode,
                rectDef_s *textRect,
                const float *subtitleGlowColor,
                bool cinematic);
char __cdecl UI_GetSticksTranslatedString(int localClientNum, const char *command, char *keys);
int __cdecl UI_GetKeyBindingLocalizedString(int localClientNum, const char *command, char *keys, int bindNum);
void __cdecl GetPlatformCommand(int localClientNum, const char *command, const char **consoleCommand);
int __cdecl Display_KeyBindPending();
int __cdecl Item_Bind_HandleKey(int localClientNum, UiContext *dc, itemDef_s *item, int key, int down);
menuDef_t *__cdecl Menu_GetFocused(UiContext *dc);
void __cdecl Menu_SetFeederSelection(
                int localClientNum,
                UiContext *dc,
                menuDef_t *menu,
                int feeder,
                int index,
                const char *name);
int __cdecl Menu_GetFeederSelection(UiContext *dc, menuDef_t *menu, int feeder, const char *name);
int __cdecl Menus_AnyFullScreenVisible(UiContext *dc);
int __cdecl Menu_HandleMouseMove(int localClientNum, UiContext *dc, menuDef_t *menu);
void __cdecl Item_MouseEnter(int localClientNum, UiContext *dc, itemDef_s *item, float x, float y);
void __cdecl Item_ListBox_MouseEnter(int localClientNum, int contextIndex, itemDef_s *item, float x, float y);
int __cdecl Item_ListBox_GetIndexForMouse(int contextIndex, itemDef_s *item, float x, float y);
void __cdecl Item_ListBox_ModalSelection(int localClientNum, int contextIndex, itemDef_s *item, int x, int y);
void __cdecl Item_Listbox_Mouse_Column_Set(int localClientNum, int contextIndex, itemDef_s *item, int x, int y);
void __cdecl Item_MouseLeave(int localClientNum, UiContext *dc, itemDef_s *item);
void __cdecl Item_SetMouseOver(const UiContext *dc, itemDef_s *item, int focus);
bool __cdecl IsVisible(char flags);
char    Menu_IsVisible(int localClientNum, UiContext *dc, menuDef_t *menu);
void __cdecl Menu_PerformTransitionEffects(int localClientNum, UiContext *dc, menuDef_t *menu);
char    Menu_Paint(
                int localClientNum,
                UiContext *dc,
                ScreenPlacementStack *scrPlaceViewStack,
                menuDef_t *menu,
                const ScreenPlacement *UI3DOverrideId);
void __cdecl Window_Paint(
                int localClientNum,
                UiContext *dc,
                windowDef_t *w,
                float fadeAmount,
                float fadeInAmount,
                float fadeClamp,
                float fadeCycle,
                int itemType,
                itemDef_s *item);
void __cdecl Item_Paint(int localClientNum, UiContext *dc, itemDef_s *item);
void __cdecl Item_Text_Paint(int localClientNum, UiContext *dc, itemDef_s *item);
Font_s *__cdecl Item_GetFont(int contextIndex, const itemDef_s *item);
void __cdecl Item_SetTextExtents(int contextIndex, itemDef_s *item, const char *text);
void __cdecl ToWindowCoords(float *x, float *y, const windowDef_t *window);
void __cdecl Item_Text_AutoWrapped_Paint(
                int contextIndex,
                itemDef_s *item,
                const char *text,
                const float *color,
                bool subtitle,
                const float *subtitleGlowColor,
                bool cinematic);
void __cdecl Item_TextField_Paint(int localClientNum, UiContext *dc, itemDef_s *item);
void __cdecl Item_YesNo_Paint(int localClientNum, UiContext *dc, itemDef_s *item);
void __cdecl Item_Multi_Paint(int localClientNum, UiContext *dc, itemDef_s *item);
const char *__cdecl Item_Multi_Setting(itemDef_s *item);
void __cdecl Item_DvarEnum_Paint(int localClientNum, UiContext *dc, itemDef_s *item);
const char *__cdecl Item_DvarEnum_Setting(itemDef_s *item);
void __cdecl Item_Slider_Paint(UiContext *dc, itemDef_s *item);
double __cdecl Item_GetRectPlacementY(int alignY, float y0, float containerHeight, float selfHeight);
void __cdecl Item_Bind_Paint(int localClientNum, UiContext *dc, itemDef_s *item);
void __cdecl Item_ListBox_Paint(int localClientNum, UiContext *dc, itemDef_s *item);
double __cdecl Item_ListBox_ThumbDrawPosition(int localClientNum, UiContext *dc, itemDef_s *item);
void __cdecl Item_ListBox_PaintTextElem(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                itemGroup_t row,
                int col,
                int maxChars,
                int halignment,
                int valignment,
                float x,
                float y,
                float w,
                float h,
                bool onFocus);
double __cdecl Item_GetTextAlignAdj(int alignment, float width, float textWidth);
double __cdecl Item_ListBox_LanguageScale(language_t language, int feederID, int col);
void __cdecl Item_ListBox_PaintBackground(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                float x,
                float y,
                int row);
// local variable allocation has failed, the output may be wrong!
void __cdecl Item_ListBox_PaintHighlight(int localClientNum, int contextIndex, itemDef_s *item, float x, float y);
void __cdecl Item_OwnerDraw_Paint(int localClientNum, UiContext *dc, itemDef_s *item);
void __cdecl Item_GameMsgWindow_Paint(int localClientNum, UiContext *dc, itemDef_s *item);
int __cdecl Menu_Count(UiContext *dc);
void __cdecl Menu_PaintAll_BeginVisibleList(char *stringBegin, unsigned int stringSize);
void __cdecl Menu_PaintAll_AppendToVisibleList(char *stringBegin, unsigned int stringSize, const char *stringToAppend);
void __cdecl Menu_PaintAll_DrawVisibleList(char *stringBegin, UiContext *dc);
void __cdecl Menu_PaintAll(int localClientNum, UiContext *dc);
int __cdecl GetBlurFocus(int localClientNum, UiContext *dc);
void __cdecl UI_SetSystemCursorPos(UiContext *dc, float x, float y);
int __cdecl Display_MouseMove(int localClientNum, UiContext *dc);
int __cdecl UI_ResetCachedMousePosition(int localClientNum, UiContext *dc);
int __cdecl Menu_OverActiveItem(int contextIndex, menuDef_t *menu, float x, float y);
void __cdecl UI_AddMenuList(int localClientNum, UiContext *dc, MenuList *menuList, int close);
void __cdecl UI_AddMenu(int localClientNum, UiContext *dc, menuDef_t *menu, int close);
void __cdecl UI_PlaySound(char context, char *aliasname);
void __cdecl UI_ClearLocalUIVisibilityBits(int localClientNum);
void __cdecl UI_SetLocalUIVisbilityBit(int localClientNum, int bitShift, int value);
int __cdecl UI_GetMenuScreen();
int __cdecl UI_GetForcedMenuScreen();
int __cdecl UI_GetMenuScreenForError();
char __cdecl Menu_DoesMenuOrParentsHaveControlFlag(UiContext *dc, menuDef_t *menu, int controlFlag);
void __cdecl UI_SetLoadingScreenMaterial(const char *name);



extern sharedUiInfo_t sharedUiInfo;

extern int g_editingField;
