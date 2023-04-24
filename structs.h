/**
 * @file structs.h
 *
 * Various global structures.
 */

DEVILUTION_BEGIN_NAMESPACE

#ifndef _STRUCTS_H
#define _STRUCTS_H

#if INT_MAX == INT32_MAX && INTPTR_MAX == INT32_MAX
#define X86_32bit_COMP
#define ALIGNMENT(x86, x64) int alignment[x86];
#define ALIGNMENT32(num) int alignment[num];
#define ALIGNMENT64(num)
#define ALIGN { 0 }
#define ALIGN32 { 0 }
#define ALIGN64
#elif INT_MAX == INT32_MAX && INTPTR_MAX == INT64_MAX
#define X86_64bit_COMP
#define ALIGNMENT(x86, x64) int alignment[x64];
#define ALIGNMENT32(num)
#define ALIGNMENT64(num) int alignment[num];
#define ALIGN { 0 }
#define ALIGN32
#define ALIGN64 { 0 }
#else
#define ALIGNMENT(x86, x64)
#define ALIGNMENT32(num)
#define ALIGNMENT64(num)
#define ALIGN
#define ALIGN32
#define ALIGN64
#endif

//////////////////////////////////////////////////
// miniwin
//////////////////////////////////////////////////

typedef int32_t INT;
typedef uint8_t BOOLEAN;

typedef uint32_t DWORD;
typedef int BOOL;
typedef unsigned char BYTE;
typedef uint16_t WORD;

typedef unsigned int UINT;

typedef int32_t WPARAM;
typedef int32_t LPARAM;

//
// Handles
//
typedef void* HANDLE;

typedef HANDLE HMODULE, HDC, HINSTANCE;

typedef void (*WNDPROC)(UINT, WPARAM);

typedef struct tagMSG {
	UINT message;
	WPARAM wParam;
} MSG, *LPMSG;

//////////////////////////////////////////////////
// control
//////////////////////////////////////////////////

typedef struct POS32 {
	int x;
	int y;
} POS32;

typedef struct RECT32 {
	int x;
	int y;
	int w;
	int h;
} RECT32;

typedef struct CelImageBuf {
#if DEBUG_MODE
	WORD ciWidth; // number of images before loaded, but overwritten with width when loaded
	WORD ciFrameCnt; // number of images before loaded, but overwritten with width when loaded
#else
	DWORD ciWidth; // number of images before loaded, but overwritten with width when loaded
#endif
	BYTE imageData[32000]; // size does not matter, the struct is allocated dynamically
} CelImageBuf;

//////////////////////////////////////////////////
// items
//////////////////////////////////////////////////

typedef struct AffixData {
	BYTE PLPower; // item_effect_type
	int PLParam1;
	int PLParam2;
	BYTE PLMinLvl;
	int PLIType; // affix_item_type
	BOOLEAN PLDouble;
	BOOLEAN PLOk;
	int PLMinVal;
	int PLMaxVal;
	int PLMultVal;
} AffixData;

typedef struct UniqItemData {
	const char* UIName;
	BYTE UIUniqType; // unique_item_type
	BYTE UIMinLvl;
	int UIValue;
	BYTE UIPower1; // item_effect_type
	int UIParam1a;
	int UIParam1b;
	BYTE UIPower2; // item_effect_type
	int UIParam2a;
	int UIParam2b;
	BYTE UIPower3; // item_effect_type
	int UIParam3a;
	int UIParam3b;
	BYTE UIPower4; // item_effect_type
	int UIParam4a;
	int UIParam4b;
	BYTE UIPower5; // item_effect_type
	int UIParam5a;
	int UIParam5b;
	BYTE UIPower6; // item_effect_type
	int UIParam6a;
	int UIParam6b;
	ALIGNMENT(3, 2)
} UniqItemData;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(UniqItemData) & (sizeof(UniqItemData) - 1)) == 64, "Align UniqItemData to power of 2 for better performance.");
#endif

typedef struct ItemFileData {
	const char* ifName; // Map of item type .cel file names.
	int idSFX;          // sounds effect of dropping the item on ground (_sfx_id).
	int iiSFX;          // sounds effect of placing the item in the inventory (_sfx_id).
	int iAnimLen;       // item drop animation length
	ALIGNMENT64(2)
} ItemFileData;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(ItemFileData) & (sizeof(ItemFileData) - 1)) == 0, "Align ItemFileData to power of 2 for better performance.");
#endif

typedef struct ItemData {
	const char* iName;
	BYTE iRnd;
	BYTE iMinMLvl;
	BYTE iUniqType; // unique_item_type
	int iCurs;      // item_cursor_graphic
	int itype;      // item_type
	int iMiscId;    // item_misc_id
	int iSpell;     // spell_id
	BYTE iClass;    // item_class
	BYTE iLoc;      // item_equip_type
	BYTE iDamType;  // item_damage_type
	BYTE iMinDam;
	BYTE iMaxDam;
	BYTE iBaseCrit;
	BYTE iMinStr;
	BYTE iMinMag;
	BYTE iMinDex;
	BOOLEAN iUsable;
	BYTE iMinAC;
	BYTE iMaxAC;
	BYTE iDurability;
	int iValue;
	ALIGNMENT(5, 3)
} ItemData;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(ItemData) & (sizeof(ItemData) - 1)) == 0, "Align ItemData to power of 2 for better performance.");
#endif

typedef struct ItemStruct {
	int _iSeed;
	uint16_t _iIdx;        // item_indexes
	uint16_t _iCreateInfo; // icreateinfo_flag
	union {
		int _ix;
		int _iPHolder; // parent index of a placeholder entry in InvList
	};
	int _iy;
	int _iCurs;   // item_cursor_graphic
	int _itype;   // item_type
	int _iMiscId; // item_misc_id
	int _iSpell;  // spell_id
	BYTE _iClass; // item_class enum
	BYTE _iLoc;   // item_equip_type
	BYTE _iDamType; // item_damage_type
	BYTE _iMinDam;
	BYTE _iMaxDam;
	BYTE _iBaseCrit;
	BYTE _iMinStr;
	BYTE _iMinMag;
	BYTE _iMinDex;
	BOOLEAN _iUsable; // can be placed in belt, can be consumed/used or stacked (if max durability is not 1)
	BYTE _iPrePower; // item_effect_type
	BYTE _iSufPower; // item_effect_type
	BYTE _iMagical;	// item_quality
	BYTE _iSelFlag;
	BOOLEAN _iFloorFlag;
	BOOLEAN _iAnimFlag;
	BYTE* _iAnimData;        // PSX name -> ItemFrame
	unsigned _iAnimFrameLen; // Tick length of each frame in the current animation
	unsigned _iAnimCnt;      // Increases by one each game tick, counting how close we are to _iAnimFrameLen
	unsigned _iAnimLen;      // Number of frames in current animation
	unsigned _iAnimFrame;    // Current frame of animation.
	//int _iAnimWidth;
	//int _iAnimXOffset;
	BOOL _iPostDraw; // should be drawn during the post-phase (magic rock on the stand)
	BOOL _iIdentified;
	char _iName[32];
	int _ivalue;
	int _iIvalue;
	int _iAC;
	int _iFlags;	// item_special_effect
	int _iCharges;
	int _iMaxCharges;
	int _iDurability;
	int _iMaxDur;
	int _iPLDam;
	int _iPLToHit;
	int _iPLAC;
	int _iPLStr;
	int _iPLMag;
	int _iPLDex;
	int _iPLVit;
	int _iPLFR;
	int _iPLLR;
	int _iPLMR;
	int _iPLAR;
	int _iPLMana;
	int _iPLHP;
	int _iPLDamMod;
	int _iPLGetHit;
	int8_t _iPLLight;
	int8_t _iPLSkillLevels;
	BYTE _iPLSkill;
	int8_t _iPLSkillLvl;
	BYTE _iPLManaSteal;
	BYTE _iPLLifeSteal;
	BYTE _iPLCrit;
	BOOLEAN _iStatFlag;
	int _iUid; // unique_item_indexes
	BYTE _iPLFMinDam;
	BYTE _iPLFMaxDam;
	BYTE _iPLLMinDam;
	BYTE _iPLLMaxDam;
	BYTE _iPLMMinDam;
	BYTE _iPLMMaxDam;
	BYTE _iPLAMinDam;
	BYTE _iPLAMaxDam;
	int _iVAdd;
	int _iVMult;
	ALIGNMENT(7, 6)
} ItemStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(ItemStruct) & (sizeof(ItemStruct) - 1)) == 0, "Align ItemStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// player
//////////////////////////////////////////////////

typedef struct PlrAnimType {
	char patTxt[4]; // suffix to select the player animation CL2
	int patGfxIdx;  // player_graphic_idx
} PlrAnimType;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(PlrAnimType) & (sizeof(PlrAnimType) - 1)) == 0, "Align PlrAnimType closer to power of 2 for better performance.");
#endif

typedef struct PlrAnimStruct {
	BYTE* paAnimData[NUM_DIRS];
	unsigned paFrames;
	int paAnimWidth;
} PlrAnimStruct;
#ifdef X86_32bit_COMP
static_assert((sizeof(PlrAnimStruct) & (sizeof(PlrAnimStruct) - 1)) == 32, "Align PlrAnimStruct closer to power of 2 for better performance.");
#elif defined(X86_64bit_COMP)
static_assert((sizeof(PlrAnimStruct) & (sizeof(PlrAnimStruct) - 1)) == 64, "Align PlrAnimStruct closer to power of 2 for better performance.");
#endif

typedef struct PlayerStruct {
	int _pmode; // PLR_MODE
	int8_t _pWalkpath[MAX_PATH_LENGTH + 1];
	int _pDestAction;
	int _pDestParam1;
	int _pDestParam2;
	int _pDestParam3; // the skill to be used in case of skill based actions
	int _pDestParam4; // the level of the skill to be used in case of skill based actions
	BOOLEAN _pActive;
	BYTE _pInvincible;
	BOOLEAN _pLvlChanging; // True when the player is transitioning between levels
	BYTE _pDunLevel; // dungeon_level
	BYTE _pClass; // plr_class
	BYTE _pLevel;
	BYTE _pRank;
	BYTE _pTeam;
	uint16_t _pStatPts;
	BYTE _pLightRad;
	BYTE _pManaShield;
	int16_t _pTimer[NUM_PLRTIMERS];
	unsigned _pExperience;
	unsigned _pNextExper;
	int _px;      // Tile X-position of player
	int _py;      // Tile Y-position of player
	int _pfutx;   // Future tile X-position of player. Set at start of walking animation
	int _pfuty;   // Future tile Y-position of player. Set at start of walking animation
	int _poldx;   // Most recent X-position in dPlayer.
	int _poldy;   // Most recent Y-position in dPlayer.
	int _pxoff;   // Player sprite's pixel X-offset from tile.
	int _pyoff;   // Player sprite's pixel Y-offset from tile.
	int _pdir;    // Direction faced by player (direction enum)
	BYTE* _pAnimData;
	int _pAnimFrameLen; // Tick length of each frame in the current animation
	int _pAnimCnt;   // Increases by one each game tick, counting how close we are to _pAnimFrameLen
	unsigned _pAnimLen;   // Number of frames in current animation
	unsigned _pAnimFrame; // Current frame of animation.
	int _pAnimWidth;
	int _pAnimXOffset;
	unsigned _plid; // light id of the player
	unsigned _pvid; // vision id of the player
	BYTE _pAtkSkill;         // the selected attack skill for the primary action
	BYTE _pAtkSkillType;     // the (RSPLTYPE_)type of the attack skill for the primary action
	BYTE _pMoveSkill;        // the selected movement skill for the primary action
	BYTE _pMoveSkillType;    // the (RSPLTYPE_)type of the movement skill for the primary action
	BYTE _pAltAtkSkill;      // the selected attack skill for the secondary action
	BYTE _pAltAtkSkillType;  // the (RSPLTYPE_)type of the attack skill for the secondary action
	BYTE _pAltMoveSkill;     // the selected movement skill for the secondary action
	BYTE _pAltMoveSkillType; // the (RSPLTYPE_)type of the movement skill for the secondary action
	BYTE _pAtkSkillHotKey[4];         // the attack skill selected by the hotkey
	BYTE _pAtkSkillTypeHotKey[4];     // the (RSPLTYPE_)type of the attack skill selected by the hotkey
	BYTE _pMoveSkillHotKey[4];        // the movement skill selected by the hotkey
	BYTE _pMoveSkillTypeHotKey[4];    // the (RSPLTYPE_)type of the movement skill selected by the hotkey
	BYTE _pAltAtkSkillHotKey[4];      // the attack skill selected by the alt-hotkey
	BYTE _pAltAtkSkillTypeHotKey[4];  // the (RSPLTYPE_)type of the attack skill selected by the alt-hotkey
	BYTE _pAltMoveSkillHotKey[4];     // the movement skill selected by the alt-hotkey
	BYTE _pAltMoveSkillTypeHotKey[4]; // the (RSPLTYPE_)type of the movement skill selected by the alt-hotkey
	BYTE _pSkillLvlBase[64]; // the skill levels of the player if they would not wear an item
	BYTE _pSkillActivity[64];
	unsigned _pSkillExp[64];
	uint64_t _pMemSkills;  // Bitmask of learned skills
	uint64_t _pAblSkills;  // Bitmask of abilities
	uint64_t _pInvSkills;  // Bitmask of skills available via items in inventory (scrolls or runes)
	char _pName[PLR_NAME_LEN];
	uint16_t _pBaseStr;
	uint16_t _pBaseMag;
	uint16_t _pBaseDex;
	uint16_t _pBaseVit;
	int _pHPBase;    // the hp of the player if they would not wear an item
	int _pMaxHPBase; // the maximum hp of the player without items
	int _pManaBase;    // the mana of the player if they would not wear an item
	int _pMaxManaBase; // the maximum mana of the player without items
	int _pVar1;
	int _pVar2;
	int _pVar3;
	int _pVar4;
	int _pVar5;
	int _pVar6;
	int _pVar7;
	int _pVar8;
	int _pGFXLoad; // flags of the loaded gfx('s)  (player_graphic_flag)
	PlrAnimStruct _pAnims[NUM_PGXS];
	unsigned _pAFNum; // action frame number of the attack animation
	unsigned _pSFNum; // action frame number of the spell animation
	ItemStruct _pHoldItem;
	ItemStruct _pInvBody[NUM_INVLOC];
	ItemStruct _pSpdList[MAXBELTITEMS];
	ItemStruct _pInvList[NUM_INV_GRID_ELEM];
	int _pGold;
	int _pStrength;
	int _pMagic;
	int _pDexterity;
	int _pVitality;
	int _pHitPoints; // the current hp of the player
	int _pMaxHP;     // the maximum hp of the player
	int _pMana;      // the current mana of the player
	int _pMaxMana;   // the maximum mana of the player
	BYTE _pSkillLvl[64]; // the skill levels of the player
	BOOLEAN _pInfraFlag;
	BYTE _pgfxnum; // Bitmask indicating what variant of the sprite the player is using. Lower byte define weapon (anim_weapon_id) and higher values define armour (starting with anim_armor_id)
	BOOLEAN _pHasUnidItem; // whether the player has an unidentified (magic) item equipped
	BYTE _pAlign_B0;
	int _pISlMinDam; // min slash-damage (swords, axes)
	int _pISlMaxDam; // max slash-damage (swords, axes)
	int _pIBlMinDam; // min blunt-damage (maces, axes)
	int _pIBlMaxDam; // max blunt-damage (maces, axes)
	int _pIPcMinDam; // min puncture-damage (bows, daggers)
	int _pIPcMaxDam; // max puncture-damage (bows, daggers)
	int _pIChMinDam; // min charge-damage (shield charge)
	int _pIChMaxDam; // max charge-damage (shield charge)
	int _pIEvasion;
	int _pIAC;
	int8_t _pMagResist;
	int8_t _pFireResist;
	int8_t _pLghtResist;
	int8_t _pAcidResist;
	int _pIHitChance;
	BYTE _pSkillFlags;    // Bitmask of allowed skill-types (SFLAG_*)
	BYTE _pIBaseHitBonus; // indicator whether the base BonusToHit of the items is positive/negative/neutral
	BYTE _pICritChance; // 200 == 100%
	BYTE _pIBlockChance;
	uint64_t _pISpells; // Bitmask of skills available via equipped items (staff)
	unsigned _pIFlags;
	BYTE _pIWalkSpeed;
	BYTE _pIRecoverySpeed;
	BYTE _pIBaseCastSpeed;
	BYTE _pAlign_B1;
	int _pIGetHit;
	BYTE _pIBaseAttackSpeed;
	int8_t _pIArrowVelBonus; // _pISplCost in vanilla code
	BYTE _pILifeSteal;
	BYTE _pIManaSteal;
	int _pIFMinDam; // min fire damage (item's added fire damage)
	int _pIFMaxDam; // max fire damage (item's added fire damage)
	int _pILMinDam; // min lightning damage (item's added lightning damage)
	int _pILMaxDam; // max lightning damage (item's added lightning damage)
	int _pIMMinDam; // min magic damage (item's added magic damage)
	int _pIMMaxDam; // max magic damage (item's added magic damage)
	int _pIAMinDam; // min acid damage (item's added acid damage)
	int _pIAMaxDam; // max acid damage (item's added acid damage)
	BYTE* _pAnimFileData[NUM_PGXS]; // file-pointers of the animations
	ALIGNMENT(187, 102)
} PlayerStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(PlayerStruct) & (sizeof(PlayerStruct) - 1)) == 0, "Align PlayerStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// textdat
//////////////////////////////////////////////////

typedef struct TextData {
	const char* txtstr;
	BOOLEAN scrlltxt;
	BOOLEAN txtsfxset;
	int txtspd;
	int sfxnr;  // _sfx_id or sfx_set if txtsfxset is true
} TextData;

//////////////////////////////////////////////////
// missiles
//////////////////////////////////////////////////

// TPDEF PTR FCN VOID MIADDPRC
// TPDEF PTR FCN VOID MIPROC

typedef struct MissileData {
	int (*mAddProc)(int, int, int, int, int, int, int, int, int);
	void (*mProc)(int);
	BYTE mdFlags; // missile_flags
	BYTE mResist; // missile_resistance
	BYTE mFileNum; // missile_gfx_id
	BOOLEAN mDrawFlag;
	int mlSFX; // sound effect when a missile is launched (_sfx_id)
	int miSFX; // sound effect on impact (_sfx_id)
	BYTE mlSFXCnt; // number of launch sound effects to choose from
	BYTE miSFXCnt; // number of impact sound effects to choose from
	ALIGNMENT(2, 7)
} MissileData;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(MissileData) & (sizeof(MissileData) - 1)) == 0, "Align MissileData to power of 2 for better performance.");
#endif

typedef struct MisFileData {
	int mfAnimFAmt;
	const char* mfName;
	const char* mfAnimTrans;
	int mfFlags; // missile_anim_flags
	BYTE mfAnimFrameLen[16];
	BYTE mfAnimLen[16];
	int mfAnimWidth;
	int mfAnimXOffset; // could be calculated
	ALIGNMENT(2, 14)
} MisFileData;
#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(MisFileData) & (sizeof(MisFileData) - 1)) == 0, "Align MisFileData to power of 2 for better performance.");
#endif

typedef struct MissileStruct {
	int _miType;   // missile_id
	BYTE _miFlags; // missile_flags
	BYTE _miResist; // missile_resistance
	BYTE _miFileNum; // missile_gfx_id
	BOOLEAN _miDrawFlag; // should be drawn
	int _miUniqTrans; // use unique color-transformation when drawing
	BOOLEAN _miDelFlag; // should be deleted
	BOOLEAN _miLightFlag; // use light-transformation when drawing
	BOOLEAN _miPreFlag; // should be drawn in the pre-phase
	BOOLEAN _miAnimFlag;
	BYTE* _miAnimData;
	int _miAnimFrameLen; // Tick length of each frame in the current animation
	int _miAnimLen;   // Number of frames in current animation
	int _miAnimWidth;
	int _miAnimXOffset;
	int _miAnimCnt; // Increases by one each game tick, counting how close we are to _miAnimFrameLen
	int _miAnimAdd;
	int _miAnimFrame; // Current frame of animation.
	int _misx;    // Initial tile X-position for missile
	int _misy;    // Initial tile Y-position for missile
	int _mix;     // Tile X-position of the missile
	int _miy;     // Tile Y-position of the missile
	int _mixoff;  // Sprite pixel X-offset for the missile
	int _miyoff;  // Sprite pixel Y-offset for the missile
	int _mixvel;  // Missile tile X-velocity while walking. This gets added onto _mitxoff each game tick
	int _miyvel;  // Missile tile Y-velocity while walking. This gets added onto _mitxoff each game tick
	int _mitxoff; // How far the missile has travelled in its lifespan along the X-axis. mix/miy/mxoff/myoff get updated every game tick based on this
	int _mityoff; // How far the missile has travelled in its lifespan along the Y-axis. mix/miy/mxoff/myoff get updated every game tick based on this
	int _miDir;   // The direction of the missile
	int _miSpllvl;
	int _miSource; // missile_source_type
	int _miCaster;
	int _miMinDam;
	int _miMaxDam;
	// int _miRndSeed;
	int _miRange;
	unsigned _miLid; // light id of the missile
	int _miVar1;
	int _miVar2;
	int _miVar3;
	int _miVar4;
	int _miVar5;
	int _miVar6;
	int _miVar7; // distance travelled in case of ARROW missiles
	int _miVar8; // last target in case of non-DOT missiles
	ALIGNMENT(10, 24)
} MissileStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(MissileStruct) & (sizeof(MissileStruct) - 1)) == 128, "Align MissileStruct closer to power of 2 for better performance.");
#elif defined(X86_64bit_COMP)
static_assert((sizeof(MissileStruct) & (sizeof(MissileStruct) - 1)) == 0, "Align MissileStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// effects/sound
//////////////////////////////////////////////////

typedef struct _Mix_Audio Mix_Audio;

typedef struct SoundSample final {
	Uint32 nextTc;
	Mix_Audio* soundData;

	void Release();
	bool IsPlaying();
	bool IsLoaded() {
		return soundData != NULL;
	}
	void Play(int lVolume, int lPan, int channel);
	void SetChunk(BYTE* fileData, size_t dwBytes, bool stream);
	//int TrackLength();
} SoundSample;

typedef struct SFXStruct {
	BYTE bFlags; // sfx_flag
	const char* pszName;
	SoundSample pSnd;
} SFXStruct;

//////////////////////////////////////////////////
// monster
//////////////////////////////////////////////////

typedef struct MonAnimStruct {
	BYTE* maAnimData[NUM_DIRS];
	int maFrames;
	int maFrameLen;
} MonAnimStruct;
#ifdef X86_32bit_COMP
static_assert((sizeof(MonAnimStruct) & (sizeof(MonAnimStruct) - 1)) == 32, "Align MonAnimStruct closer to power of 2 for better performance.");
#elif defined(X86_64bit_COMP)
static_assert((sizeof(MonAnimStruct) & (sizeof(MonAnimStruct) - 1)) == 64, "Align MonAnimStruct closer to power of 2 for better performance.");
#endif

typedef struct MonsterAI {
	BYTE aiType; // _monster_ai
	BYTE aiInt;
	BYTE aiParam1;
	BYTE aiParam2;
} MonsterAI;

typedef struct MonsterData {
	uint16_t moFileNum; // _monster_gfx_id
	BYTE mLevel;
	BYTE mSelFlag;
	const char* mTransFile;
	const char* mName;
	MonsterAI mAI;
	uint16_t mMinHP;
	uint16_t mMaxHP;
	int mFlags;       // _monster_flag
	uint16_t mHit;    // hit chance (melee+projectile)
	BYTE mMinDamage;
	BYTE mMaxDamage;
	uint16_t mHit2;   // hit chance of special melee attacks
	BYTE mMinDamage2;
	BYTE mMaxDamage2;
	BYTE mMagic;      // hit chance of magic-projectile
	BYTE mMagic2;     // unused
	BYTE mArmorClass; // AC+evasion: used against physical-hit (melee+projectile)
	BYTE mEvasion;    // evasion: used against magic-projectile
	uint16_t mMagicRes;  // resistances in normal and nightmare difficulties (_monster_resistance)
	uint16_t mMagicRes2; // resistances in hell difficulty (_monster_resistance)
	uint16_t mTreasure;  // unique drops of monsters + no-drop flag (unique_item_indexes + _monster_treasure)
	uint16_t mExp;
	ALIGNMENT(5, 2)
} MonsterData;
#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(MonsterData) & (sizeof(MonsterData) - 1)) == 0, "Align MonsterData to power of 2 for better performance.");
#endif

typedef struct MonFileData {
	int moImage;
	const char* moGfxFile;
	const char* moSndFile;
	int moAnimFrames[NUM_MON_ANIM];
	int moAnimFrameLen[NUM_MON_ANIM];
	BYTE moWidth;
	BOOLEAN moSndSpecial;
	BYTE moAFNum;
	BYTE moAFNum2;
} MonFileData;
#ifdef X86_32bit_COMP
static_assert((sizeof(MonFileData) & (sizeof(MonFileData) - 1)) == 0, "Align MonFileData to power of 2 for better performance.");
#elif defined(X86_64bit_COMP)
static_assert((sizeof(MonFileData) & (sizeof(MonFileData) - 1)) == 64, "Align MonFileData to power of 2 for better performance.");
#endif
#pragma pack(push, 1)
typedef struct MapMonData {
	int cmType;
	BOOL cmPlaceScatter;
	SoundSample cmSnds[NUM_MON_SFX][2];
	BYTE* cmAnimData[NUM_MON_ANIM];
	MonAnimStruct cmAnims[NUM_MON_ANIM];
	const char* cmName;
	uint16_t cmFileNum;
	BYTE cmLevel;
	BYTE cmSelFlag;
	MonsterAI cmAI;
	int cmFlags;
	uint16_t cmHit;    // hit chance (melee+projectile)
	BYTE cmMinDamage;
	BYTE cmMaxDamage;
	uint16_t cmHit2;   // hit chance of special melee attacks
	BYTE cmMinDamage2;
	BYTE cmMaxDamage2;
	BYTE cmMagic;      // hit chance of magic-projectile
	BYTE cmMagic2;     // unused
	BYTE cmArmorClass; // AC+evasion: used against physical-hit (melee+projectile)
	BYTE cmEvasion;    // evasion: used against magic-projectile
	uint16_t cmMagicRes;  // resistances of the monster
	uint16_t cmTreasure;  // unique drops of monsters + no-drop flag
	unsigned cmExp;
	int cmWidth;
	int cmXOffset;
	BYTE cmAFNum;
	BYTE cmAFNum2;
	uint16_t cmAlign_0; // unused
	uint16_t cmMinHP;
	uint16_t cmMaxHP;
	ALIGNMENT32(31)
} MapMonData;
#ifdef X86_32bit_COMP
static_assert((sizeof(MapMonData) & (sizeof(MapMonData) - 1)) == 0, "Align MapMonData closer to power of 2 for better performance.");
#elif defined(X86_64bit_COMP)
static_assert((sizeof(MapMonData) & (sizeof(MapMonData) - 1)) == 640, "Align MapMonData closer to power of 2 for better performance.");
#endif
#pragma pack(pop)
typedef struct MonsterStruct {
	int _mmode; /* MON_MODE */
	unsigned _msquelch;
	BYTE _mMTidx;
	BYTE _mpathcount; // unused
	BYTE _mWhoHit;    // unused in multiplayer games
	BYTE _mgoal;
	int _mgoalvar1;
	int _mgoalvar2;
	int _mgoalvar3;
	int _mx;                // Tile X-position of monster
	int _my;                // Tile Y-position of monster
	int _mfutx;             // Future tile X-position of monster. Set at start of walking animation
	int _mfuty;             // Future tile Y-position of monster. Set at start of walking animation
	int _moldx;             // Most recent X-position in dMonster.
	int _moldy;             // Most recent Y-position in dMonster.
	int _mxoff;             // Monster sprite's pixel X-offset from tile.
	int _myoff;             // Monster sprite's pixel Y-offset from tile.
	int _mdir;              // Direction faced by monster (direction enum)
	int _menemy;            // The current target of the monster. An index in to either the plr or monster array based on the _meflag value.
	BYTE _menemyx;          // X-coordinate of enemy (usually correspond's to the enemy's futx value)
	BYTE _menemyy;          // Y-coordinate of enemy (usually correspond's to the enemy's futy value)
	BYTE _mListener;        // the player to whom the monster is talking to (unused)
	BOOLEAN _mDelFlag; // unused
	BYTE* _mAnimData;
	int _mAnimFrameLen; // Tick length of each frame in the current animation
	int _mAnimCnt;   // Increases by one each game tick, counting how close we are to _mAnimFrameLen
	int _mAnimLen;   // Number of frames in current animation
	int _mAnimFrame; // Current frame of animation.
	int _mVar1;
	int _mVar2;
	int _mVar3; // Used to store the original mode of a stoned monster. Not 'thread' safe -> do not use for anything else! 
	int _mVar4;
	int _mVar5;
	int _mVar6; // Used as _mxoff but with a higher range so that we can correctly apply velocities of a smaller number
	int _mVar7; // Used as _myoff but with a higher range so that we can correctly apply velocities of a smaller number
	int _mVar8; // Value used to measure progress for moving from one tile to another
	int _mmaxhp;
	int _mhitpoints;
	int _mlastx; // the last known X-coordinate of the enemy
	int _mlasty; // the last known Y-coordinate of the enemy
	int _mRndSeed;
	int _mAISeed;
	BYTE _muniqtype;
	BYTE _muniqtrans;
	BYTE _mNameColor; // color of the tooltip. white: normal, blue: pack; gold: unique. (text_color)
	BYTE _mlid; // light id of the monster
	BYTE _mleader; // the leader of the monster
	BYTE _mleaderflag; // the status of the monster's leader
	BYTE _mpacksize; // the number of 'pack'-monsters close to their leader
	BYTE _mvid; // vision id of the monster (for minions only)
	const char* _mName;
	uint16_t _mFileNum; // _monster_gfx_id
	BYTE _mLevel;
	BYTE _mSelFlag;
	MonsterAI _mAI;
	int _mFlags;       // _monster_flag
	uint16_t _mHit;    // hit chance (melee+projectile)
	BYTE _mMinDamage;
	BYTE _mMaxDamage;
	uint16_t _mHit2;   // hit chance of special melee attacks
	BYTE _mMinDamage2;
	BYTE _mMaxDamage2;
	BYTE _mMagic;      // hit chance of magic-projectile
	BYTE _mMagic2;     // unused
	BYTE _mArmorClass; // AC+evasion: used against physical-hit (melee+projectile)
	BYTE _mEvasion;    // evasion: used against magic-projectile
	uint16_t _mMagicRes;  // resistances of the monster (_monster_resistance)
	uint16_t _mTreasure;  // unique drops of monsters + no-drop flag (unique_item_indexes + _monster_treasure)
	unsigned _mExp;
	int _mAnimWidth;
	int _mAnimXOffset;
	BYTE _mAFNum;  // action frame number of the attack animation
	BYTE _mAFNum2; // action frame number of the special animation
	uint16_t _mAlign_0; // unused
	int _mType; // _monster_id
	MonAnimStruct* _mAnims;
	ALIGNMENT(12, 7)
} MonsterStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(MonsterStruct) & (sizeof(MonsterStruct) - 1)) == 0, "Align MonsterStruct to power of 2 for better performance.");
#endif

typedef struct MonEnemyStruct {
	int _meLastDir; // direction
	int _meRealDir; // direction
	int _meRealDist;
} MonEnemyStruct;

typedef struct UniqMonData {
	int mtype; // _monster_id
	const char* mName;
	const char* mTrnName;
	BYTE muLevelIdx; // level-index to place the monster (dungeon_level)
	BYTE muLevel;    // difficulty level of the monster
	uint16_t mmaxhp;
	MonsterAI mAI;
	BYTE mMinDamage;
	BYTE mMaxDamage;
	BYTE mMinDamage2;
	BYTE mMaxDamage2;
	uint16_t mMagicRes;  // _monster_resistance
	uint16_t mMagicRes2; // _monster_resistance
	BYTE mUnqFlags;// _uniq_monster_flag
	BYTE mUnqHit;  // to-hit (melee+projectile) bonus
	BYTE mUnqHit2; // to-hit (special melee attacks) bonus
	BYTE mUnqMag;  // to-hit (magic-projectile) bonus
	BYTE mUnqEva;  // evasion bonus
	BYTE mUnqAC;   // armor class bonus
	BYTE mQuestId; // quest_id
	int mtalkmsg;  // _speech_id
	ALIGNMENT(6, 2)
} UniqMonData;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(UniqMonData) & (sizeof(UniqMonData) - 1)) == 0, "Align UniqMonData to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// objects
//////////////////////////////////////////////////

typedef struct ObjectData {
	BYTE ofindex;     // object_graphic_id
	BYTE oLvlTypes;   // dungeon_type_mask
	BYTE otheme;      // theme_id
	BYTE oquest;      // quest_id
	//BYTE oAnimFlag;
	BYTE oAnimBaseFrame; // The starting/base frame of (initially) non-animated objects
	//int oAnimFrameLen; // Tick length of each frame in the current animation
	//int oAnimLen;   // Number of frames in current animation
	//int oAnimWidth;
	//int oSFX;
	//BYTE oSFXCnt;
	BYTE oLightRadius;
	int8_t oLightOffX;
	int8_t oLightOffY;
	BYTE oProc;       // object_proc_func
	BYTE oModeFlags;  // object_mode_flags
	//BOOL oSolidFlag;
	//BOOL oMissFlag;
	//BYTE oBreak;
	BYTE oDoorFlag;   // object_door_type
	BYTE oSelFlag;
	BOOLEAN oTrapFlag;
	BYTE oAlign[3];
} ObjectData;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(ObjectData) & (sizeof(ObjectData) - 1)) == 0, "Align ObjectData closer to power of 2 for better performance.");
#endif

typedef struct ObjFileData {
	const char* ofName;
	int oSFX; // _sfx_id
	BYTE oSFXCnt;
	BYTE oAnimFlag; // object_anim_mode
	int oAnimFrameLen; // Tick length of each frame in the current animation
	int oAnimLen;   // Number of frames in current animation
	int oAnimWidth;
	BOOLEAN oSolidFlag;
	BOOLEAN oMissFlag;
	BYTE oBreak; // object_break_mode
	ALIGNMENT32(1)
} ObjFileData;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(ObjFileData) & (sizeof(ObjFileData) - 1)) == 0, "Align ObjFileData closer to power of 2 for better performance.");
#endif

typedef struct ObjectStruct {
	int _otype; // _object_id
	int _ox;    // Tile X-position of the object
	int _oy;    // Tile Y-position of the object
	int _oSFX;  // _sfx_id
	BYTE _oSFXCnt;
	BYTE _oAnimFlag;  // object_anim_mode
	BYTE _oProc;      // object_proc_func
	BYTE _oModeFlags; // object_mode_flags
	BYTE* _oAnimData;
	int _oAnimFrameLen; // Tick length of each frame in the current animation
	int _oAnimCnt;   // Increases by one each game tick, counting how close we are to _oAnimFrameLen
	int _oAnimLen;   // Number of frames in current animation
	int _oAnimFrame; // Current frame of animation.
	int _oAnimWidth;
	int _oAnimXOffset;
	//BOOL _oDelFlag;
	BOOLEAN _oSolidFlag;
	BOOLEAN _oMissFlag;
	BYTE _oAlign;
	BYTE _oBreak; // object_break_mode
	BYTE _oDoorFlag; // object_door_type
	BYTE _oSelFlag;
	BYTE _oTrapChance;
	BOOLEAN _oPreFlag;
	unsigned _olid; // light id of the object
	int _oRndSeed;
	int _oVar1;
	int _oVar2;
	int _oVar3;
	int _oVar4;
	int _oVar5;
	int _oVar6;
	int _oVar7;
	int _oVar8;
	ALIGNMENT(8, 5)
} ObjectStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(ObjectStruct) & (sizeof(ObjectStruct) - 1)) == 0, "Align ObjectStruct to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// portal
//////////////////////////////////////////////////

typedef struct PortalStruct {
	BOOLEAN _ropen;
	BYTE _rAlign0;
	BYTE _rAlign1;
	BYTE _rAlign2;
	int _rx;
	int _ry;
	int _rlevel; // dungeon_level
} PortalStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(PortalStruct) & (sizeof(PortalStruct) - 1)) == 0, "Align PortalStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// endianness
//////////////////////////////////////////////////

typedef struct LE_UINT16 {
	uint16_t _value;

	void operator=(uint16_t val) {
		_value = SwapLE16(val);
	};
	//void operator=(const LE_UINT16& val) {
	//	_value = val._value;
	//};
	template <class T>
	void operator=(T) = delete;

	bool operator==(const LE_UINT16 & oval) const {
		return _value == oval._value;
	};
	bool operator!=(const LE_UINT16& oval) const {
		return _value != oval._value;
	};
	operator uint16_t() const { return SwapLE16(_value); }
} LE_UINT16;

typedef struct LE_INT16 {
	int16_t _value;

	void operator=(int16_t val) {
		_value = SwapLE16(val);
	};
	//void operator=(const LE_INT32& val) {
	//	_value = val._value;
	//};
	template <class T>
	void operator=(T) = delete;

	bool operator==(const LE_INT16 & oval) const {
		return _value == oval._value;
	};
	bool operator!=(const LE_INT16& oval) const {
		return _value != oval._value;
	};
	operator int16_t() const { return SwapLE16(_value); }
} LE_INT16;

typedef struct LE_UINT32 {
	uint32_t _value;

	void operator=(unsigned val) {
		_value = SwapLE32(val);
	};
#if INT_MAX != INT32_MAX
	void operator=(uint32_t val) {
		_value = SwapLE32(val);
	};
#endif
	//void operator=(const LE_UINT32& val) {
	//	_value = val._value;
	//};
	template <class T>
	void operator=(T) = delete;

	bool operator==(const LE_UINT32 & oval) const {
		return _value == oval._value;
	};
	bool operator!=(const LE_UINT32& oval) const {
		return _value != oval._value;
	};
	operator unsigned() const { return (uint32_t)SwapLE32(_value); }
} LE_UINT32;

typedef struct LE_INT32 {
	int32_t _value;

	void operator=(int val) {
		_value = SwapLE32(val);
	};
#if INT_MAX != INT32_MAX
	void operator=(int32_t val) {
		_value = SwapLE32(val);
	};
#endif
	void operator=(const LE_INT32& val) {
		_value = val._value;
	};
	template <class T>
	void operator=(T) = delete;

	bool operator==(const LE_INT32 & oval) const {
		return _value == oval._value;
	};
	bool operator!=(const LE_INT32& oval) const {
		return _value != oval._value;
	};
	operator int() const { return (int32_t)SwapLE32(_value); }
} LE_INT32;

typedef struct LE_UINT64 {
	uint64_t _value;

	void operator=(uint64_t val) {
		_value = SwapLE64(val);
	};
	//void operator=(const LE_UINT64& val) {
	//	_value = val._value;
	//};
	template <class T>
	void operator=(T) = delete;

	//bool operator==(const LE_UINT64 & oval) const {
	//	return _value == oval._value;
	//};
	//bool operator!=(const LE_UINT64& oval) const {
	//	return _value != oval._value;
	//};
	operator uint64_t() const { return (uint64_t)SwapLE64(_value); }
} LE_UINT64;

//////////////////////////////////////////////////
// pack
//////////////////////////////////////////////////

#pragma pack(push, 1)
typedef struct PkItemStruct {
	LE_INT32 dwSeed;
	LE_UINT16 wIndx;
	LE_UINT16 wCI;
	BYTE bId;
	BYTE bDur;
	BYTE bMDur;
	BYTE bCh;
	BYTE bMCh;
	LE_UINT16 wValue;
	LE_UINT32 dwBuff;

	bool PkItemEq(const PkItemStruct& opkItem) const {
		return memcmp(&dwSeed, &opkItem.dwSeed, sizeof(dwSeed) + sizeof(wIndx) + sizeof(wCI)) == 0;
	};
	bool PkItemEq(const ItemStruct& opkItem) const {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		return opkItem._iSeed == dwSeed && opkItem._iIdx == wIndx && opkItem._iCreateInfo == wCI;
#else
		static_assert(offsetof(PkItemStruct, wCI) - offsetof(PkItemStruct, dwSeed) == offsetof(ItemStruct, _iCreateInfo) - offsetof(ItemStruct, _iSeed), "PkItemStruct compares the whole memory in one step I.");
		static_assert(offsetof(PkItemStruct, wIndx) - offsetof(PkItemStruct, dwSeed) == offsetof(ItemStruct, _iIdx) - offsetof(ItemStruct, _iSeed), "PkItemStruct compares the whole memory in one step II.");
		return memcmp(&dwSeed, &opkItem._iSeed, sizeof(dwSeed) + sizeof(wIndx) + sizeof(wCI)) == 0;
#endif
	};
} PkItemStruct;

typedef struct PkPlayerStruct {
	//BYTE px;
	//BYTE py;
	char pName[PLR_NAME_LEN];
	BOOLEAN pLvlChanging;
	BYTE pDunLevel;
	BYTE pClass;
	BYTE pLevel;
	BYTE pRank;
	BYTE pTeam;
	LE_UINT16 pStatPts;
	//BYTE pLightRad;
	//BYTE pManaShield;
	//LE_INT16 pTimer[NUM_PLRTIMERS];
	LE_UINT32 pExperience;
	LE_UINT16 pBaseStr;
	LE_UINT16 pBaseMag;
	LE_UINT16 pBaseDex;
	LE_UINT16 pBaseVit;
	LE_INT32 pHPBase;
	LE_INT32 pMaxHPBase;
	LE_INT32 pManaBase;
	LE_INT32 pMaxManaBase;
	BYTE pAtkSkillHotKey[4];         // the attack skill selected by the hotkey
	BYTE pAtkSkillTypeHotKey[4];     // the (RSPLTYPE_)type of the attack skill selected by the hotkey
	BYTE pMoveSkillHotKey[4];        // the movement skill selected by the hotkey
	BYTE pMoveSkillTypeHotKey[4];    // the (RSPLTYPE_)type of the movement skill selected by the hotkey
	BYTE pAltAtkSkillHotKey[4];      // the attack skill selected by the alt-hotkey
	BYTE pAltAtkSkillTypeHotKey[4];  // the (RSPLTYPE_)type of the attack skill selected by the alt-hotkey
	BYTE pAltMoveSkillHotKey[4];     // the movement skill selected by the alt-hotkey
	BYTE pAltMoveSkillTypeHotKey[4]; // the (RSPLTYPE_)type of the movement skill selected by the alt-hotkey
	BYTE pSkillLvlBase[64];
	BYTE pSkillActivity[64];
	LE_UINT32 pSkillExp[64];
	LE_UINT64 pMemSkills;
	PkItemStruct pHoldItem;
	PkItemStruct pInvBody[NUM_INVLOC];
	PkItemStruct pSpdList[MAXBELTITEMS];
	PkItemStruct pInvList[NUM_INV_GRID_ELEM];
	int8_t pInvGrid[NUM_INV_GRID_ELEM];
	LE_INT32 pNumInv; // unused
} PkPlayerStruct;
#pragma pack(pop)

//////////////////////////////////////////////////
// load/save
//////////////////////////////////////////////////

#pragma pack(push, 1)
typedef struct LSaveGameHeaderStruct {
	LE_INT32 vhInitial;
	LE_UINT32 vhLogicTurn;
	LE_UINT32 vhSentCycle;
	LE_INT32 vhLvlDifficulty;
	LE_UINT32 vhSeeds[NUM_LEVELS];
	LE_INT32 vhCurrSeed;
	LE_INT32 vhViewX;
	LE_INT32 vhViewY;
	LE_INT32 vhScrollXOff;
	LE_INT32 vhScrollYOff;
	LE_INT32 vhScrollDir;
	LE_INT32 vhHPPer;
	LE_INT32 vhManaPer;
	BYTE vhLvlUpFlag;
	BYTE vhAutomapflag;
	BYTE vhZoomInFlag;
	BYTE vhInvflag;
	LE_INT32 vhNumActiveWindows;
	BYTE vhActiveWindows[NUM_WNDS];
	BYTE vhTownWarps;
	BYTE vhWaterDone;
	LE_UINT32 vhAutoMapScale;
	LE_INT32 vhAutoMapXOfs;
	LE_INT32 vhAutoMapYOfs;
	LE_UINT32 vhLvlVisited;
} LSaveGameHeaderStruct;

typedef struct LSaveGameMetaStruct {
	LE_UINT32 vaboylevel;
	LE_INT32 vanumpremium;
	LE_INT32 vapremiumlevel;
	LE_INT32 vanumlights;
	LE_INT32 vanumvision;
} LSaveGameMetaStruct;

typedef struct LSaveGameLvlMetaStruct {
	LE_INT32 vvnummonsters;
	LE_INT32 vvnummissiles;
	LE_INT32 vvnumobjects;
	LE_INT32 vvnumitems;
} LSaveGameLvlMetaStruct;

typedef struct LSaveItemStruct {
	LE_INT32 viSeed;
	LE_UINT16 viIdx;
	LE_UINT16 viCreateInfo;
	LE_INT32 vix;
	LE_INT32 viy;
	LE_INT32 viCurs;   // item_cursor_graphic
	LE_INT32 vitype;   // item_type
	LE_INT32 viMiscId; // item_misc_id
	LE_INT32 viSpell;  // spell_id
	BYTE viClass; // item_class enum
	BYTE viLoc;   // item_equip_type
	BYTE viDamType; // item_damage_type
	BYTE viMinDam;
	BYTE viMaxDam;
	BYTE viBaseCrit;
	BYTE viMinStr;
	BYTE viMinMag;
	BYTE viMinDex;
	BOOLEAN viUsable;
	BYTE viPrePower; // item_effect_type
	BYTE viSufPower; // item_effect_type
	BYTE viMagical;	// item_quality
	BYTE viSelFlag;
	BOOLEAN viFloorFlag;
	BOOLEAN viAnimFlag;
	int32_t viAnimDataAlign;        // PSX name -> ItemFrame
	uint32_t viAnimFrameLenAlign; // Tick length of each frame in the current animation
	LE_UINT32 viAnimCnt;      // Increases by one each game tick, counting how close we are to viAnimFrameLen
	LE_UINT32 viAnimLen;      // Number of frames in current animation
	LE_UINT32 viAnimFrame;    // Current frame of animation.
	LE_INT32 viPostDraw;
	LE_INT32 viIdentified;
	char viName[32];
	LE_INT32 vivalue;
	LE_INT32 viIvalue;
	LE_INT32 viAC;
	LE_INT32 viFlags;	// item_special_effect
	LE_INT32 viCharges;
	LE_INT32 viMaxCharges;
	LE_INT32 viDurability;
	LE_INT32 viMaxDur;
	LE_INT32 viPLDam;
	LE_INT32 viPLToHit;
	LE_INT32 viPLAC;
	LE_INT32 viPLStr;
	LE_INT32 viPLMag;
	LE_INT32 viPLDex;
	LE_INT32 viPLVit;
	LE_INT32 viPLFR;
	LE_INT32 viPLLR;
	LE_INT32 viPLMR;
	LE_INT32 viPLAR;
	LE_INT32 viPLMana;
	LE_INT32 viPLHP;
	LE_INT32 viPLDamMod;
	LE_INT32 viPLGetHit;
	int8_t viPLLight;
	int8_t viPLSkillLevels;
	BYTE viPLSkill;
	int8_t viPLSkillLvl;
	BYTE viPLManaSteal;
	BYTE viPLLifeSteal;
	BYTE viPLCrit;
	BOOLEAN viStatFlag;
	LE_INT32 viUid;
	BYTE viPLFMinDam;
	BYTE viPLFMaxDam;
	BYTE viPLLMinDam;
	BYTE viPLLMaxDam;
	BYTE viPLMMinDam;
	BYTE viPLMMaxDam;
	BYTE viPLAMinDam;
	BYTE viPLAMaxDam;
	LE_INT32 viVAdd;
	LE_INT32 viVMult;
} LSaveItemStruct;

typedef struct LSavePlayerStruct {
	LE_INT32 vpmode; // PLR_MODE
	int8_t vpWalkpath[MAX_PATH_LENGTH + 1];
	LE_INT32 vpDestAction;
	LE_INT32 vpDestParam1;
	LE_INT32 vpDestParam2;
	LE_INT32 vpDestParam3;	// the skill to be used in case of skill based actions
	LE_INT32 vpDestParam4; // the level of the skill to be used in case of skill based actions
	BOOLEAN vpActive;
	BYTE vpInvincible;
	BOOLEAN vpLvlChanging; // True when the player is transitioning between levels
	BYTE vpDunLevel; // dungeon_level
	BYTE vpClass; // plr_class
	BYTE vpLevel;
	BYTE vpRank;
	BYTE vpTeam;
	LE_UINT16 vpStatPts;
	BYTE vpLightRad;
	BYTE vpManaShield;
	LE_INT16 vpTimer[NUM_PLRTIMERS];
	LE_UINT32 vpExperience;
	LE_UINT32 vpNextExper;
	LE_INT32 vpx;      // Tile X-position of player
	LE_INT32 vpy;      // Tile Y-position of player
	LE_INT32 vpfutx;   // Future tile X-position of player. Set at start of walking animation
	LE_INT32 vpfuty;   // Future tile Y-position of player. Set at start of walking animation
	LE_INT32 vpoldx;   // Most recent X-position in dPlayer.
	LE_INT32 vpoldy;   // Most recent Y-position in dPlayer.
	LE_INT32 vpxoff;   // Player sprite's pixel X-offset from tile.
	LE_INT32 vpyoff;   // Player sprite's pixel Y-offset from tile.
	LE_INT32 vpdir;    // Direction faced by player (direction enum)
	INT vpAnimDataAlign;
	INT vpAnimFrameLenAlign; // Tick length of each frame in the current animation
	LE_INT32 vpAnimCnt;   // Increases by one each game tick, counting how close we are to vpAnimFrameLen
	uint32_t vpAnimLenAlign;   // Number of frames in current animation
	LE_UINT32 vpAnimFrame; // Current frame of animation.
	INT vpAnimWidthAlign;
	INT vpAnimXOffsetAlign;
	LE_UINT32 vplid; // light id of the player
	LE_UINT32 vpvid; // vision id of the player
	BYTE vpAtkSkill;         // the selected attack skill for the primary action
	BYTE vpAtkSkillType;     // the (RSPLTYPE_)type of the attack skill for the primary action
	BYTE vpMoveSkill;        // the selected movement skill for the primary action
	BYTE vpMoveSkillType;    // the (RSPLTYPE_)type of the movement skill for the primary action
	BYTE vpAltAtkSkill;      // the selected attack skill for the secondary action
	BYTE vpAltAtkSkillType;  // the (RSPLTYPE_)type of the attack skill for the secondary action
	BYTE vpAltMoveSkill;     // the selected movement skill for the secondary action
	BYTE vpAltMoveSkillType; // the (RSPLTYPE_)type of the movement skill for the secondary action
	BYTE vpAtkSkillHotKey[4];         // the attack skill selected by the hotkey
	BYTE vpAtkSkillTypeHotKey[4];     // the (RSPLTYPE_)type of the attack skill selected by the hotkey
	BYTE vpMoveSkillHotKey[4];        // the movement skill selected by the hotkey
	BYTE vpMoveSkillTypeHotKey[4];    // the (RSPLTYPE_)type of the movement skill selected by the hotkey
	BYTE vpAltAtkSkillHotKey[4];      // the attack skill selected by the alt-hotkey
	BYTE vpAltAtkSkillTypeHotKey[4];  // the (RSPLTYPE_)type of the attack skill selected by the alt-hotkey
	BYTE vpAltMoveSkillHotKey[4];     // the movement skill selected by the alt-hotkey
	BYTE vpAltMoveSkillTypeHotKey[4]; // the (RSPLTYPE_)type of the movement skill selected by the alt-hotkey
	BYTE vpSkillLvlBase[64]; // the skill levels of the player if they would not wear an item
	BYTE vpSkillActivity[64];
	LE_UINT32 vpSkillExp[64];
	LE_UINT64 vpMemSkills;  // Bitmask of learned skills
	LE_UINT64 vpAblSkills;  // Bitmask of abilities
	LE_UINT64 vpInvSkills;  // Bitmask of skills available via items in inventory (scrolls or runes)
	char vpName[PLR_NAME_LEN];
	LE_UINT16 vpBaseStr;
	LE_UINT16 vpBaseMag;
	LE_UINT16 vpBaseDex;
	LE_UINT16 vpBaseVit;
	LE_INT32 vpHPBase;    // the hp of the player if they would not wear an item
	LE_INT32 vpMaxHPBase; // the maximum hp of the player without items
	LE_INT32 vpManaBase;    // the mana of the player if they would not wear an item
	LE_INT32 vpMaxManaBase; // the maximum mana of the player without items
	LE_INT32 vpVar1;
	LE_INT32 vpVar2;
	LE_INT32 vpVar3;
	LE_INT32 vpVar4;
	LE_INT32 vpVar5;
	LE_INT32 vpVar6;
	LE_INT32 vpVar7;
	LE_INT32 vpVar8;
	//int _pGFXLoad; // flags of the loaded gfx('s)  (player_graphic_flag)
	//PlrAnimStruct _pAnims[NUM_PGXS];
	//unsigned _pAFNum;
	//unsigned _pSFNum;
	LSaveItemStruct vpHoldItem;
	LSaveItemStruct vpInvBody[NUM_INVLOC];
	LSaveItemStruct vpSpdList[MAXBELTITEMS];
	LSaveItemStruct vpInvList[NUM_INV_GRID_ELEM];
	LE_INT32 vpGold;
} LSavePlayerStruct;

typedef struct LSaveMonsterStruct {
	LE_INT32 vmmode; /* MON_MODE */
	LE_UINT32 vmsquelch;
	BYTE vmMTidx;
	BYTE vmpathcount; // unused
	BYTE vmWhoHit;
	BYTE vmgoal;
	LE_INT32 vmgoalvar1;
	LE_INT32 vmgoalvar2;
	LE_INT32 vmgoalvar3;
	LE_INT32 vmx;           // Tile X-position of monster
	LE_INT32 vmy;           // Tile Y-position of monster
	LE_INT32 vmfutx;        // Future tile X-position of monster. Set at start of walking animation
	LE_INT32 vmfuty;        // Future tile Y-position of monster. Set at start of walking animation
	LE_INT32 vmoldx;        // Most recent X-position in dMonster.
	LE_INT32 vmoldy;        // Most recent Y-position in dMonster.
	LE_INT32 vmxoff;        // Monster sprite's pixel X-offset from tile.
	LE_INT32 vmyoff;        // Monster sprite's pixel Y-offset from tile.
	LE_INT32 vmdir;         // Direction faced by monster (direction enum)
	LE_INT32 vmenemy;       // The current target of the monster. An index in to either the plr or monster array based on the vmeflag value.
	BYTE vmenemyx;          // X-coordinate of enemy (usually correspond's to the enemy's futx value)
	BYTE vmenemyy;          // Y-coordinate of enemy (usually correspond's to the enemy's futy value)
	BYTE vmListener;        // the player to whom the monster is talking to (unused)
	BOOLEAN vmDelFlag; // unused
	INT vmAnimDataAlign;
	INT vmAnimFrameLenAlign; // Tick length of each frame in the current animation
	LE_INT32 vmAnimCnt;   // Increases by one each game tick, counting how close we are to vmAnimFrameLen
	INT vmAnimLenAlign;   // Number of frames in current animation
	LE_INT32 vmAnimFrame; // Current frame of animation.
	LE_INT32 vmVar1;
	LE_INT32 vmVar2;
	LE_INT32 vmVar3; // Used to store the original mode of a stoned monster. Not 'thread' safe -> do not use for anything else! 
	LE_INT32 vmVar4;
	LE_INT32 vmVar5;
	LE_INT32 vmVar6; // Used as _mxoff but with a higher range so that we can correctly apply velocities of a smaller number
	LE_INT32 vmVar7; // Used as _myoff but with a higher range so that we can correctly apply velocities of a smaller number
	LE_INT32 vmVar8; // Value used to measure progress for moving from one tile to another
	LE_INT32 vmmaxhp;
	LE_INT32 vmhitpoints;
	LE_INT32 vmlastx; // the last known X-coordinate of the enemy
	LE_INT32 vmlasty; // the last known Y-coordinate of the enemy
	LE_INT32 vmRndSeed;
	LE_INT32 vmAISeed;
	BYTE vmuniqtype;
	BYTE vmuniqtrans;
	BYTE vmNameColor;
	BYTE vmlid;
	BYTE vmleader; // the leader of the monster
	BYTE vmleaderflag; // the status of the monster's leader
	BYTE vmpacksize; // the number of 'pack'-monsters close to their leader
	BYTE vmvid; // vision id of the monster (for minions only)
	INT vmNameAlign;
	LE_UINT16 vmFileNum;
	BYTE vmLevel;
	BYTE vmSelFlag;
	BYTE vmAI_aiType;   // MonsterAI.aiType
	BYTE vmAI_aiInt;    // MonsterAI.aiInt
	BYTE vmAI_aiParam1; // MonsterAI.aiParam1
	BYTE vmAI_aiParam2; // MonsterAI.aiParam2
	LE_INT32 vmFlags;
	LE_UINT16 vmHit;    // hit chance (melee+projectile)
	BYTE vmMinDamage;
	BYTE vmMaxDamage;
	LE_UINT16 vmHit2;   // hit chance of special melee attacks
	BYTE vmMinDamage2;
	BYTE vmMaxDamage2;
	BYTE vmMagic;      // hit chance of magic-projectile
	BYTE vmMagic2;     // unused
	BYTE vmArmorClass; // AC+evasion: used against physical-hit (melee+projectile)
	BYTE vmEvasion;    // evasion: used against magic-projectile
	LE_UINT16 vmMagicRes;  // resistances of the monster
	LE_UINT16 vmTreasure;  // unique drops of monsters + no-drop flag
	LE_UINT32 vmExp;
} LSaveMonsterStruct;

typedef struct LSaveMissileStruct {
	LE_INT32 vmiType;   // missile_id
	BYTE vmiFlags; // missile_flags
	BYTE vmiResist; // missile_resistance
	BYTE vmiFileNum; // missile_gfx_id
	BOOLEAN vmiDrawFlag; // should be drawn
	LE_INT32 vmiUniqTrans; // use unique color-transformation when drawing
	BOOLEAN vmiDelFlag; // should be deleted
	BOOLEAN vmiLightFlag; // use light-transformation when drawing
	BOOLEAN vmiPreFlag; // should be drawn in the pre-phase
	BOOLEAN vmiAnimFlag;
	INT vmiAnimDataAlign;
	INT vmiAnimFrameLenAlign; // Tick length of each frame in the current animation
	INT vmiAnimLenAlign;   // Number of frames in current animation
	INT vmiAnimWidthAlign;
	INT vmiAnimXOffsetAlign;
	LE_INT32 vmiAnimCnt; // Increases by one each game tick, counting how close we are to vmiAnimFrameLen
	LE_INT32 vmiAnimAdd;
	LE_INT32 vmiAnimFrame; // Current frame of animation.
	LE_INT32 vmisx;    // Initial tile X-position for missile
	LE_INT32 vmisy;    // Initial tile Y-position for missile
	LE_INT32 vmix;     // Tile X-position of the missile
	LE_INT32 vmiy;     // Tile Y-position of the missile
	LE_INT32 vmixoff;  // Sprite pixel X-offset for the missile
	LE_INT32 vmiyoff;  // Sprite pixel Y-offset for the missile
	LE_INT32 vmixvel;  // Missile tile X-velocity while walking. This gets added onto vmitxoff each game tick
	LE_INT32 vmiyvel;  // Missile tile Y-velocity while walking. This gets added onto vmitxoff each game tick
	LE_INT32 vmitxoff; // How far the missile has travelled in its lifespan along the X-axis. mix/miy/mxoff/myoff get updated every game tick based on this
	LE_INT32 vmityoff; // How far the missile has travelled in its lifespan along the Y-axis. mix/miy/mxoff/myoff get updated every game tick based on this
	LE_INT32 vmiDir;   // The direction of the missile
	LE_INT32 vmiSpllvl;
	LE_INT32 vmiSource; // missile_source_type
	LE_INT32 vmiCaster;
	LE_INT32 vmiMinDam;
	LE_INT32 vmiMaxDam;
	LE_INT32 vmiRange;
	LE_UINT32 vmiLid; // light id of the missile
	LE_INT32 vmiVar1;
	LE_INT32 vmiVar2;
	LE_INT32 vmiVar3;
	LE_INT32 vmiVar4;
	LE_INT32 vmiVar5;
	LE_INT32 vmiVar6;
	LE_INT32 vmiVar7; // distance travelled in case of ARROW missiles
	LE_INT32 vmiVar8; // last target in case of non-DOT missiles
} LSaveMissileStruct;

typedef struct LSaveObjectStruct {
	LE_INT32 votype; // _object_id
	LE_INT32 vox;
	LE_INT32 voy;
	LE_INT32 voSFX; // ssfx_id
	BYTE voSFXCnt;
	BYTE voAnimFlag;
	BYTE voProc;
	BYTE voModeFlags;
	INT voAnimDataAlign;
	LE_INT32 voAnimFrameLen; // Tick length of each frame in the current animation
	LE_INT32 voAnimCnt;   // Increases by one each game tick, counting how close we are to voAnimFrameLen
	LE_INT32 voAnimLen;   // Number of frames in current animation
	LE_INT32 voAnimFrame; // Current frame of animation.
	INT voAnimWidthAlign;
	INT voAnimXOffsetAlign;
	BOOLEAN voSolidFlag;
	BOOLEAN voMissFlag;
	BYTE voAlign;
	BYTE voBreak; // object_break_mode
	BYTE voDoorFlag; // object_door_type
	BYTE voSelFlag;
	BYTE voTrapChance;
	BOOLEAN voPreFlag;
	LE_UINT32 volid; // light id of the object
	LE_INT32 voRndSeed;
	LE_INT32 voVar1;
	LE_INT32 voVar2;
	LE_INT32 voVar3;
	LE_INT32 voVar4;
	LE_INT32 voVar5;
	LE_INT32 voVar6;
	LE_INT32 voVar7;
	LE_INT32 voVar8;
} LSaveObjectStruct;

typedef struct LSaveQuestStruct {
	BYTE vqactive;
	BYTE vqvar1; // quest parameter which is synchronized with the other players
	BYTE vqvar2; // quest parameter which is NOT synchronized with the other players
	BOOLEAN vqlog;
	LE_UINT32 vqmsg;
} LSaveQuestStruct;

typedef struct LSaveLightListStruct {
	LE_INT32 vlx;
	LE_INT32 vly;
	LE_INT32 vlunx;
	LE_INT32 vluny;
	BYTE vlradius;
	BYTE vlunr;
	int8_t vlunxoff;
	int8_t vlunyoff;
	BOOLEAN vldel;
	BOOLEAN vlunflag;
	BOOLEAN vlmine;
	BYTE vlAlign2;
	LE_INT32 vlxoff;
	LE_INT32 vlyoff;
} LSaveLightListStruct;

typedef struct LSavePortalStruct {
	BOOLEAN vropen;
	BYTE vrAlign0;
	BYTE vrAlign1;
	BYTE vrAlign2;
	LE_INT32 vrx;
	LE_INT32 vry;
	LE_INT32 vrlevel;
} LSavePortalStruct;

#pragma pack(pop)

//////////////////////////////////////////////////
// msg
//////////////////////////////////////////////////

#pragma pack(push, 1)
typedef struct CmdSkillUse {
	BYTE skill;
	int8_t from;
} CmdSkillUse;

typedef struct TCmd {
	BYTE bCmd;
} TCmd;

typedef struct TCmdLoc {
	BYTE bCmd;
	BYTE x;
	BYTE y;
} TCmdLoc;

typedef struct TCmdLocBParam1 {
	BYTE bCmd;
	BYTE x;
	BYTE y;
	BYTE bParam1;
} TCmdLocBParam1;

typedef struct TCmdLocBParam2 {
	BYTE bCmd;
	BYTE x;
	BYTE y;
	BYTE bParam1;
	BYTE bParam2;
} TCmdLocBParam2;

typedef struct TCmdLocParam1 {
	BYTE bCmd;
	BYTE x;
	BYTE y;
	LE_UINT16 wParam1;
} TCmdLocParam1;

typedef struct TCmdParam1 {
	BYTE bCmd;
	LE_UINT16 wParam1;
} TCmdParam1;

typedef struct TCmdParamBW {
	BYTE bCmd;
	BYTE byteParam;
	LE_UINT16 wordParam;
} TCmdParamBW;

typedef struct TCmdParam3 {
	BYTE bCmd;
	LE_UINT16 wParam1;
	LE_UINT16 wParam2;
	LE_UINT16 wParam3;
} TCmdParam3;

typedef struct TCmdBParam1 {
	BYTE bCmd;
	BYTE bParam1;
} TCmdBParam1;

typedef struct TCmdBParam2 {
	BYTE bCmd;
	BYTE bParam1;
	BYTE bParam2;
} TCmdBParam2;

typedef struct TCmdItemOp {
	BYTE bCmd;
	BYTE ioIdx;
	CmdSkillUse iou;
} TCmdItemOp;

typedef struct TCmdLocSkill {
	BYTE bCmd;
	BYTE x;
	BYTE y;
	CmdSkillUse lsu;
} TCmdLocSkill;

typedef struct TCmdLocDisarm {
	BYTE bCmd;
	BYTE x;
	BYTE y;
	BYTE oi;
	int8_t from;
} TCmdLocDisarm;

typedef struct TCmdPlrSkill {
	BYTE bCmd;
	BYTE psPnum;
	CmdSkillUse psu;
} TCmdPlrSkill;

typedef struct TCmdMonSkill {
	BYTE bCmd;
	LE_UINT16 msMnum;
	CmdSkillUse msu;
} TCmdMonSkill;

typedef struct TCmdMonstDamage {
	BYTE bCmd;
	BYTE mdLevel;
	LE_UINT16 mdMnum;
	LE_INT32 mdHitpoints;
} TCmdMonstDamage;

typedef struct TCmdMonstKill {
	TCmdLocBParam1 mkParam1;
	BYTE mkPnum;
	LE_UINT16 mkMnum;
	LE_UINT32 mkExp;
	BYTE mkMonLevel;
	BYTE mkDir;
} TCmdMonstKill;

typedef struct TCmdMonstSummon {
	TCmdLocBParam1 mnParam1;
	BYTE mnDir;
	BYTE mnSIdx;
	LE_UINT16 mnMnum;
	LE_INT32 mnMaxHp;
} TCmdMonstSummon;

typedef struct TCmdGolem {
	BYTE bCmd;
	BYTE goMonLevel;
	BYTE goX;
	BYTE goY;
	BYTE goDunLevel;
} TCmdGolem;

typedef struct TCmdShrine {
	BYTE bCmd;
	BYTE shType;
	LE_INT32 shSeed;
} TCmdShrine;

typedef struct TCmdQuest {
	BYTE bCmd;
	BYTE q;
	BYTE qstate;
	BYTE qlog;
	BYTE qvar1;
} TCmdQuest;

typedef struct TCmdGItem {
	BYTE bCmd;
	BYTE bLevel;
	BYTE x;
	BYTE y;
	BOOLEAN fromFloor;
	PkItemStruct item;
} TCmdGItem;

typedef struct TCmdPItem {
	BYTE bCmd;
	BYTE bLevel;
	BYTE x;
	BYTE y;
} TCmdPItem;

typedef struct TCmdRPItem {
	BYTE bCmd;
	BYTE bLevel;
	BYTE x;
	BYTE y;
	BOOLEAN bFlipFlag;
	PkItemStruct item;
} TCmdRPItem;

typedef struct TCmdStore1 {
	BYTE bCmd;
	BYTE stCmd;
	BYTE stLoc;
	LE_INT32 stValue;
} TCmdStore1;

typedef struct TCmdStore2 {
	BYTE bCmd;
	BYTE stCmd;
	PkItemStruct item;
	LE_INT32 stValue;
} TCmdStore2;

typedef struct TCmdJoinLevel {
	BYTE bCmd;
	BYTE lLevel;
	BYTE px;
	BYTE py;
	LE_INT32 php;
	LE_INT32 pmp;
	LE_INT16 lTimer1;
	LE_INT16 lTimer2;
	BYTE pManaShield; // TODO: remove this and from TSyncLvlPlayer and add to PkPlayerStruct?
	BYTE itemsDur[NUM_INVELEM + 1];
} TCmdJoinLevel;

typedef struct TMsgLargeHdr {
	BYTE bCmd;
	LE_UINT16 wBytes;
} TMsgLargeHdr;

typedef struct TMsgString {
	BYTE bCmd;
	char str[MAX_SEND_STR_LEN];
} TMsgString;

typedef struct TMsgFakeDropPlr {
	BYTE bCmd;
	BYTE bReason;
} TMsgFakeDropPlr;

typedef struct TSyncHeader {
	BYTE bCmd;
	BYTE bLevel;
	LE_UINT16 wLen;
	/*BYTE bObjId;
	BYTE bObjCmd;
	BYTE bItemI;
	BYTE bItemX;
	BYTE bItemY;
	LE_UINT16 wItemIndx;
	LE_UINT16 wItemCI;
	LE_UINT32 dwItemSeed;
	BYTE bItemId;
	BYTE bItemDur;
	BYTE bItemMDur;
	BYTE bItemCh;
	BYTE bItemMCh;
	LE_UINT16 wItemVal;
	LE_UINT32 dwItemBuff;
	BYTE bPInvLoc;
	LE_UINT16 wPInvIndx;
	LE_UINT16 wPInvCI;
	LE_UINT32 dwPInvSeed;
	BYTE bPInvId;
#ifdef HELLFIRE
	LE_UINT16 wToHit;
	LE_UINT16 wMaxDam;
	BYTE bMinStr;
	BYTE bMinMag;
	BYTE bMinDex;
	BYTE bAC;
#endif*/
} TSyncHeader;

typedef struct TSyncMonster {
	BYTE nmndx;
	BYTE nmx;
	BYTE nmy;
	BYTE nmdir;
	BYTE nmleaderflag;
	LE_UINT32 nmactive;
	LE_INT32 nmhitpoints;
} TSyncMonster;

typedef struct TSyncLvlPlayer {
	BYTE spMode;
	BYTE spWalkpath[MAX_PATH_LENGTH];
	BYTE spManaShield;
	BYTE spInvincible;
	BYTE spDestAction;
	LE_INT32 spDestParam1;
	LE_INT32 spDestParam2;
	LE_INT32 spDestParam3;
	LE_INT32 spDestParam4;
	LE_INT16 spTimer[NUM_PLRTIMERS];
	BYTE spx;      // Tile X-position of player
	BYTE spy;      // Tile Y-position of player
	BYTE spfutx;   // Future tile X-position of player. Set at start of walking animation
	BYTE spfuty;   // Future tile Y-position of player. Set at start of walking animation
	BYTE spoldx;   // Most recent X-position in dPlayer.
	BYTE spoldy;   // Most recent Y-position in dPlayer.
//	LE_INT32 spxoff;   // Player sprite's pixel X-offset from tile.
//	LE_INT32 spyoff;   // Player sprite's pixel Y-offset from tile.
	BYTE spdir;    // Direction faced by player (direction enum)
	BYTE spAnimFrame; // Current frame of animation.
	BYTE spAnimCnt;   // Increases by one each game tick, counting how close we are to _pAnimFrameLen
	LE_INT32 spHPBase;
	LE_INT32 spManaBase;
	LE_INT32 spVar1;
	LE_INT32 spVar2;
	LE_INT32 spVar3;
	LE_INT32 spVar4;
	LE_INT32 spVar5;
	LE_INT32 spVar6;
	LE_INT32 spVar7;
	LE_INT32 spVar8;
	BYTE spItemsDur[NUM_INVELEM + 1];
} TSyncLvlPlayer;

typedef struct TSyncLvlMonster {
	LE_UINT16 smMnum;
	BYTE smMode; /* MON_MODE */
	LE_UINT32 smSquelch;
	//BYTE _mMTidx;
	//BYTE smPathcount; // unused
	//BYTE smWhoHit;
	BYTE smGoal;
	LE_INT32 smGoalvar1;
	LE_INT32 smGoalvar2;
	LE_INT32 smGoalvar3;
	BYTE smx;                // Tile X-position of monster
	BYTE smy;                // Tile Y-position of monster
	BYTE smfutx;             // Future tile X-position of monster. Set at start of walking animation
	BYTE smfuty;             // Future tile Y-position of monster. Set at start of walking animation
	BYTE smoldx;             // Most recent X-position in dMonster.
	BYTE smoldy;             // Most recent Y-position in dMonster.
//	LE_INT32 smxoff;             // Monster sprite's pixel X-offset from tile.
//	LE_INT32 smyoff;             // Monster sprite's pixel Y-offset from tile.
	BYTE smdir;              // Direction faced by monster (direction enum)
	LE_INT32 smEnemy;            // The current target of the monster. An index in to either the plr or monster array based on the _meflag value.
	BYTE smEnemyx;          // X-coordinate of enemy (usually correspond's to the enemy's futx value)
	BYTE smEnemyy;          // Y-coordinate of enemy (usually correspond's to the enemy's futy value)
	BYTE smListener;        // the player to whom the monster is talking to (unused)
	BOOLEAN smDelFlag; // unused
	BYTE smAnimCnt;   // Increases by one each game tick, counting how close we are to _mAnimFrameLen
	BYTE smAnimFrame; // Current frame of animation.
	LE_INT32 smVar1;
	LE_INT32 smVar2;
	LE_INT32 smVar3;
	LE_INT32 smVar4;
	LE_INT32 smVar5;
	LE_INT32 smVar6; // Used as _mxoff but with a higher range so that we can correctly apply velocities of a smaller number
	LE_INT32 smVar7; // Used as _myoff but with a higher range so that we can correctly apply velocities of a smaller number
	LE_INT32 smVar8; // Value used to measure progress for moving from one tile to another
	LE_INT32 smHitpoints;
	BYTE smLastx; // the last known X-coordinate of the enemy
	BYTE smLasty; // the last known Y-coordinate of the enemy
	//BYTE smLeader; // the leader of the monster
	BYTE smLeaderflag; // the status of the monster's leader
	//BYTE smPacksize; // the number of 'pack'-monsters close to their leader
	//BYTE falign_CB;
	LE_INT32 smFlags;
} TSyncLvlMonster;

typedef struct TSyncLvlMissile {
	LE_UINT16 smiMi;
	BYTE smiType;   // missile_id
	BYTE smiFileNum; // missile_gfx_id
	BOOLEAN smiDrawFlag;
	BYTE smiUniqTrans;
	BOOLEAN smiLightFlag;
	BOOLEAN smiPreFlag;
	BYTE smiAnimCnt; // Increases by one each game tick, counting how close we are to _miAnimFrameLen
	int8_t smiAnimAdd;
	BYTE smiAnimFrame; // Current frame of animation.
	BYTE smiDir;   // The direction of the missile
	BYTE smisx;    // Initial tile X-position for missile
	BYTE smisy;    // Initial tile Y-position for missile
	BYTE smix;     // Tile X-position of the missile
	BYTE smiy;     // Tile Y-position of the missile
	LE_INT32 smixoff;  // Sprite pixel X-offset for the missile
	LE_INT32 smiyoff;  // Sprite pixel Y-offset for the missile
	LE_INT32 smixvel;  // Missile tile X-velocity while walking. This gets added onto _mitxoff each game tick
	LE_INT32 smiyvel;  // Missile tile Y-velocity while walking. This gets added onto _mitxoff each game tick
	LE_INT32 smitxoff; // How far the missile has travelled in its lifespan along the X-axis. mix/miy/mxoff/myoff get updated every game tick based on this
	LE_INT32 smityoff; // How far the missile has travelled in its lifespan along the Y-axis. mix/miy/mxoff/myoff get updated every game tick based on this
	LE_INT32 smiSpllvl; // TODO: int?
	LE_INT32 smiSource; // TODO: int?
	LE_INT32 smiCaster; // TODO: int?
	LE_INT32 smiMinDam;
	LE_INT32 smiMaxDam;
	// LE_INT32 smiRndSeed;
	LE_INT32 smiRange; // Time to live for the missile in game ticks, when 0 the missile will be marked for deletion via _miDelFlag
	LE_INT32 smiVar1;
	LE_INT32 smiVar2;
	LE_INT32 smiVar3;
	LE_INT32 smiVar4;
	LE_INT32 smiVar5;
	LE_INT32 smiVar6;
	LE_INT32 smiVar7;
	LE_INT32 smiVar8;
	BYTE smiLidRadius;
} TSyncLvlMissile;

typedef struct TurnPktHdr {
	// LE_INT32 php;
	// LE_INT32 pmhp;
	// LE_INT32 pmp;
	// LE_INT32 pmmp;
	// BYTE px;
	// BYTE py;
	// LE_UINT16 wCheck;
	LE_UINT16 wLen;
} TurnPktHdr;

typedef struct TurnPkt {
	TurnPktHdr hdr;
	BYTE body[NET_TURN_MSG_SIZE - sizeof(TurnPktHdr)];
} TurnPkt;

typedef struct MsgPktHdr {
	// LE_UINT16 wCheck;
	LE_UINT16 wLen;
} MsgPktHdr;

typedef struct NormalMsgPkt {
	MsgPktHdr hdr;
	BYTE body[NET_NORMAL_MSG_SIZE - sizeof(MsgPktHdr)];
} NormalMsgPkt;

typedef struct DMonsterStr {
	BYTE dmCmd;
	BYTE dmx;
	BYTE dmy;
	BYTE dmdir;
	BYTE dmleaderflag;
	BYTE dmWhoHit;
	BYTE dmSIdx;
	LE_UINT32 dmactive;
	LE_INT32 dmhitpoints;
} DMonsterStr;

typedef struct DObjectStr {
	BYTE bCmd;
} DObjectStr;

typedef struct DItemStr {
	BYTE bCmd;
	BYTE x;
	BYTE y;
	PkItemStruct item;
} DItemStr;

typedef struct DLevel {
	DItemStr item[MAXITEMS];
	DObjectStr object[MAXOBJECTS];
	DMonsterStr monster[MAXMONSTERS];
} DLevel;

typedef struct LocalLevel {
	BOOLEAN automapsv[DMAXX][DMAXY];
} LocalLevel;

typedef struct DPortal {
	BYTE level;
	BYTE x;
	BYTE y;
} DPortal;

typedef struct DQuest {
	BYTE qstate;
	BYTE qlog;
	BYTE qvar1;
} DQuest;

typedef struct DJunk {
	DPortal jPortals[MAXPORTAL];
	DQuest jQuests[NUM_QUESTS];
	BYTE jGolems[MAX_MINIONS];
} DJunk;

typedef struct LDLevel {
	BYTE ldNumMonsters;
	BYTE ldMissActive[MAXMISSILES];
	LE_UINT16 wLen; // length of ldContent
	BYTE ldContent[MAX_PLRS * sizeof(TSyncLvlPlayer) + MAXMONSTERS * sizeof(TSyncLvlMonster) + MAXMISSILES * sizeof(TSyncLvlMissile)];
} LDLevel;

typedef struct DBuffer {
	BOOLEAN compressed;
	BYTE content[NET_LARGE_MSG_SIZE - sizeof(BOOLEAN) - sizeof(TMsgLargeHdr) - sizeof(MsgPktHdr)];
} DBuffer;

typedef struct DeltaDataEnd {
	BYTE numChunks;
	LE_UINT32 turn;
} DeltaDataEnd;

typedef struct LevelDeltaEnd {
	BYTE numChunks;
	BYTE level;
	LE_UINT32 turn;
} LevelDeltaEnd;

typedef struct TMsgLarge {
	TMsgLargeHdr tpHdr;
	DBuffer tpData;
} TMsgLarge;

typedef struct LargeMsgPkt {
	MsgPktHdr apHdr;
	TMsgLarge apMsg;
} LargeMsgPkt;

typedef struct DeltaData {
	union {
		struct {
			DJunk ddJunk;					// portals and quests
			DLevel ddLevel[NUM_LEVELS];		// items/monsters/objects
			LocalLevel ddLocal[NUM_LEVELS];	// automap
			bool ddLevelChanged[NUM_LEVELS];
			bool ddJunkChanged;

			LargeMsgPkt ddSendRecvPkt; // Buffer to send/receive delta info
			unsigned ddSendRecvOffset; // offset in the buffer
			BYTE ddDeltaSender;        // the pnum of the delta-sender
			BYTE ddRecvLastCmd;        // type of the last received delta-chunk (NMSG_DLEVEL_ or NMSG_LVL_DELTA_)
		};
		BYTE ddBuffer[FILEBUFF];
	};
} DeltaData;
#pragma pack(pop)

typedef struct TMegaPkt {
	struct TMegaPkt* pNext;
	unsigned dwSpaceLeft;
	BYTE data[0x8000 - sizeof(TMegaPkt*) - sizeof(unsigned)];
} TMegaPkt;

typedef struct DMegaPkt {
	struct DMegaPkt* dmpNext;
	int dmpPlr;
	unsigned dmpLen;
	BYTE dmpCmd;
	BYTE data[32000]; // size does not matter, the struct is allocated dynamically
} DMegaPkt;

typedef struct TBuffer {
	unsigned dwDataSize;
	BYTE bData[4096 - sizeof(unsigned)];
} TBuffer;

//////////////////////////////////////////////////
// levels
//////////////////////////////////////////////////

typedef struct LevelStruct {
	BYTE _dLevelIdx;  // index in AllLevels
	BOOLEAN _dSetLvl; // cached flag if the level is a set-level
	BYTE _dLevel;     // cached difficulty value of the level
	BYTE _dType;      // cached type of the level
	BYTE _dDunType;   // cached type of the dungeon
} LevelStruct;

typedef struct LevelData {
	BYTE dLevel;
	BOOLEAN dSetLvl;
	BYTE dType;
	BYTE dDunType;
	BYTE dMusic;
	BYTE dMicroTileLen;
	BYTE dBlocks;
	const char* dLevelName;
	const char* dAutomapData;
	const char* dSolidTable;
	const char* dMicroFlags;
	const char* dMicroCels;
	const char* dMegaTiles;
	const char* dMiniTiles;
	const char* dSpecCels;
	const char* dPalName;
	const char* dLoadCels;
	const char* dLoadPal;
	BOOLEAN dLoadBarOnTop;
	BYTE dLoadBarColor;
	BYTE dMonDensity;
	BYTE dObjDensity;
	BYTE dSetLvlDunX;
	BYTE dSetLvlDunY;
	BYTE dSetLvlWarp; // dungeon_warp_type
	BYTE dSetLvlPiece; // _setpiece_type
	BYTE dMonTypes[32];
	ALIGNMENT(9, 2)
} LevelData;

#ifdef X86_32bit_COMP
static_assert((sizeof(LevelData) & (sizeof(LevelData) - 1)) == 0, "Align LevelData to power of 2 for better performance.");
#elif defined(X86_64bit_COMP)
static_assert((sizeof(LevelData) & (sizeof(LevelData) - 1)) == 128, "Align LevelData to power of 2 for better performance.");
#endif

typedef struct WarpStruct {
	int _wx;
	int _wy;
	int _wtype; // dungeon_warp_type
} WarpStruct;

typedef struct SetPieceStruct {
	BYTE* _spData;
	int _spx;
	int _spy;
	int _sptype; // _setpiece_type
} SetPieceStruct;

typedef struct SetPieceData {
	const char* _spdDunFile;
	const char* _spdPreDunFile;
} SetPieceData;

//////////////////////////////////////////////////
// quests
//////////////////////////////////////////////////

typedef struct QuestStruct {
	BYTE _qactive;
	BYTE _qvar1; // quest parameter which is synchronized with the other players
	BYTE _qvar2; // quest parameter which is NOT synchronized with the other players
	BOOLEAN _qlog;
	unsigned _qmsg;
} QuestStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(QuestStruct) & (sizeof(QuestStruct) - 1)) == 0, "Align QuestStruct to power of 2 for better performance.");
#endif

typedef struct QuestData {
	BYTE _qdlvl; // dungeon level
	BYTE _qslvl; // setmap level
	int _qdmsg;  // _speech_id
	const char* _qlstr; // quest title
} QuestData;

//////////////////////////////////////////////////
// gamemenu/gmenu
//////////////////////////////////////////////////

// TPDEF PTR FCN VOID TMenuFcn

typedef struct TMenuItem {
	const char* pszStr;
	void (*fnMenu)(bool); /* fix, should have one arg */
	uint32_t dwFlags; // _gmenu_flags
	//union {
	//	uint32_t dwMenuParam;
	//	struct {
			uint16_t wMenuParam1;
			uint16_t wMenuParam2;
	//	};
	//};
} TMenuItem;

// TPDEF PTR FCN VOID TMenuUpdateFcn

//////////////////////////////////////////////////
// spells
//////////////////////////////////////////////////

typedef struct SpellData {
	BYTE sManaCost;
	BYTE sType;
	BYTE sIcon;
	const char* sNameText;
	BYTE sBookLvl;   // minimum level for books
	BYTE sStaffLvl;  // minimum level for staves
	BYTE sScrollLvl; // minimum level for scrolls/runes
	BYTE sSkillFlags; // flags (SDFLAG*) of the skill
	BYTE scCurs; // cursor for scrolls/runes
	BYTE spCurs; // cursor for spells
	BYTE sUseFlags; // the required flags(SFLAG*) to use the skill
	BYTE sMinInt;
	BYTE sSFX;
	BYTE sMissile;
	BYTE sManaAdj;
	BYTE sMinMana;
	uint16_t sStaffMin;
	uint16_t sStaffMax;
	int sBookCost;
	int sStaffCost; // == sScrollCost == sRuneCost
	ALIGNMENT64(6)
} SpellData;
#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(SpellData) & (sizeof(SpellData) - 1)) == 0, "Align SpellData to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// scrollrt
//////////////////////////////////////////////////

typedef struct ScrollStruct {
	int _sxoff; // X-offset of camera position. This usually corresponds to a negative version of plr[myplr]._pxoff
	int _syoff; // Y-offset of camera position. This usually corresponds to a negative version of plr[myplr]._pyoff
	// int _sdx;
	// int _sdy;
	int _sdir;
} ScrollStruct;

typedef struct ViewportStruct {
	unsigned _vColumns; // number of tiles in one row
	unsigned _vRows;    // number of tiles in one column
	int _vOffsetX; // X-offset in a back buffer
	int _vOffsetY; // Y-offset in a back buffer
	int _vShiftX; // X-shift in a dPiece
	int _vShiftY; // Y-shift in a dPiece
} ViewportStruct;

//////////////////////////////////////////////////
// gendung
//////////////////////////////////////////////////

//////////////////////////////////////////////////
// drlg
//////////////////////////////////////////////////

typedef struct ShadowPattern {
	union {
		struct {
			BYTE sh11;
			BYTE sh01;
			BYTE sh10;
			BYTE sh00;
		};
		uint32_t asUInt32;
	};
} ShadowPattern;

typedef struct ShadowStruct {
	ShadowPattern shPattern;
	ShadowPattern shMask;
	BYTE nv1;
	BYTE nv2;
	BYTE nv3;
} ShadowStruct;

typedef struct ROOMHALLNODE {
	int nRoomParent;
	int nRoomx1;
	int nRoomy1;
	int nRoomx2;
	int nRoomy2;
	int nHallx1;
	int nHally1;
	int nHallx2;
	int nHally2;
	int nHalldir;
} ROOMHALLNODE;

typedef struct L1ROOM {
	BYTE lrx;
	BYTE lry;
	BYTE lrw;
	BYTE lrh;
} L1ROOM;

typedef struct ThemePosDir {
	BYTE tpdx;
	BYTE tpdy;
	BYTE tpdvar1;
	BYTE tpdvar2; // unused
} ThemePosDir;

/** The number of generated rooms in cathedral. */
#define L1_MAXROOMS ((DSIZEX * DSIZEY) / sizeof(L1ROOM))
/** The number of generated rooms in catacombs. */
#define L2_MAXROOMS 32
/** Possible matching locations in a theme room. */
#define THEME_LOCS ((DSIZEX * DSIZEY) / sizeof(ThemePosDir))

typedef struct DrlgMem {
	union {
		L1ROOM L1RoomList[L1_MAXROOMS];     // drlg_l1
		ROOMHALLNODE RoomList[L2_MAXROOMS]; // drlg_l2
		BYTE transvalMap[DMAXX][DMAXY];     // drlg_l1, drlg_l2, drlg_l3, drlg_l4
		BYTE transDirMap[DSIZEX][DSIZEY];   // drlg_l1, drlg_l2, drlg_l3, drlg_l4 (gendung)
		BYTE lockoutMap[DMAXX][DMAXY];      // drlg_l3
		BYTE dungBlock[L4BLOCKX][L4BLOCKY]; // drlg_l4
		ThemePosDir thLocs[THEME_LOCS];     // themes
	};
} DrlgMem;

//////////////////////////////////////////////////
// themes
//////////////////////////////////////////////////

typedef struct ThemeStruct {
	int _tsx1; // top-left corner of the theme-room
	int _tsy1;
	int _tsx2; // bottom-right corner of the theme-room
	int _tsy2;
	BYTE _tsType;
	BYTE _tsTransVal;
	BYTE _tsObjVar1;
	BYTE _tsObjVar2; // unused
	int _tsObjX;
	int _tsObjY;
	ALIGNMENT(1, 1)
} ThemeStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(ThemeStruct) & (sizeof(ThemeStruct) - 1)) == 0, "Align ThemeStruct to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// inv
//////////////////////////////////////////////////

typedef struct InvXY {
	int X;
	int Y;
} InvXY;

//////////////////////////////////////////////////
// lighting
//////////////////////////////////////////////////

typedef struct LightListStruct {
	int _lx;
	int _ly;
	int _lunx;
	int _luny;
	BYTE _lradius;
	BYTE _lunr;
	int8_t _lunxoff;
	int8_t _lunyoff;
	BOOLEAN _ldel;
	BOOLEAN _lunflag;
	BOOLEAN _lmine;
	BYTE _lAlign2;
	int _lxoff;
	int _lyoff;
} LightListStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(LightListStruct) & (sizeof(LightListStruct) - 1)) == 0, "Align LightListStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// diabloui
//////////////////////////////////////////////////

// TPDEF PTR FCN VOID PLAYSND

typedef struct _uidefaultstats {
	uint16_t dsStrength;
	uint16_t dsMagic;
	uint16_t dsDexterity;
	uint16_t dsVitality;
} _uidefaultstats;

typedef struct _uiheroinfo {
	BYTE hiIdx;
	BYTE hiLevel;
	BYTE hiClass;
	BYTE hiRank;
	char hiName[16];
	int16_t hiStrength;
	int16_t hiMagic;
	int16_t hiDexterity;
	int16_t hiVitality;
	BOOL hiHasSaved;
} _uiheroinfo;

typedef struct _uigamedata {
	uint32_t aeVersionId;
	int32_t aeSeed;
	BYTE aeDifficulty;
	BYTE aeTickRate;
	BYTE aeNetUpdateRate; // (was defaultturnssec in vanilla)
	BYTE aeMaxPlayers;
	BYTE aePlayerId;
} _uigamedata;

//////////////////////////////////////////////////
// storm-net
//////////////////////////////////////////////////

#pragma pack(push, 1)
typedef struct SNetGameData {
	LE_UINT32 ngVersionId;
	LE_INT32 ngSeed;
	BYTE ngDifficulty;
	BYTE ngTickRate;
	BYTE ngNetUpdateRate; // (was defaultturnssec in vanilla)
	BYTE ngMaxPlayers;
} SNetGameData;
#pragma pack(pop)

/*typedef struct _SNETCAPS {
	//DWORD size;
	DWORD flags;
	DWORD maxmessagesize;
	DWORD maxqueuesize;
	DWORD maxplayers;
	DWORD bytessec;
	DWORD latencyms;
	DWORD defaultturnssec;
	DWORD defaultturnsintransit;
} _SNETCAPS;*/

typedef struct SNetEvent {
	unsigned eventid;
	unsigned playerid;
	BYTE* _eData;
	unsigned databytes;
} SNetEvent;

typedef struct SNetTurnPkt {
	uint32_t nmpTurn;
	unsigned nmpLen;
	BYTE data[32000]; // size does not matter, the struct is allocated dynamically
} SNetTurnPkt;

//////////////////////////////////////////////////
// path
//////////////////////////////////////////////////

typedef struct PATHNODE {
	BYTE totalCost;
	BYTE remainingCost;
	BYTE walkCost;
	int x;
	int y;
	struct PATHNODE* Parent;
	struct PATHNODE* Child[NUM_DIRS];
	struct PATHNODE* NextNode;
	ALIGNMENT(3, 8)
} PATHNODE;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(PATHNODE) & (sizeof(PATHNODE) - 1)) == 0, "Align PATHNODE closer to power of 2 for better performance.");
#endif

// TPDEF PTR FCN UCHAR CHECKFUNC1

// TPDEF PTR FCN UCHAR CHECKFUNC

//////////////////////////////////////////////////
// sha
//////////////////////////////////////////////////

typedef struct SHA1Context {
	DWORD state[5];
#if DEBUG_MODE
	DWORD count[2];
#endif
	char buffer[64];
} SHA1Context;

//////////////////////////////////////////////////
// tmsg
//////////////////////////////////////////////////

typedef struct TMsgHdr {
	TMsgHdr* pNext;
	uint32_t dwTime;
	//BYTE bLen;
	// this is actually alignment padding, but the message body is appended to the struct
	// so it's convenient to use byte-alignment and name it "body"
	//BYTE body[3];
} TMsgHdr;

//////////////////////////////////////////////////
// mpqapi
//////////////////////////////////////////////////

#pragma pack(push, 1)
typedef struct FileMpqHeader {
	uint32_t pqSignature;
	uint32_t pqHeaderSize;
	uint32_t pqFileSize;
	uint16_t pqVersion;
	uint16_t pqSectorSizeId;
	uint32_t pqHashOffset;
	uint32_t pqBlockOffset;
	uint32_t pqHashCount;
	uint32_t pqBlockCount;
	char pqPad[72];
} FileMpqHeader;

typedef struct FileMpqHashEntry {
	uint32_t hqHashA;
	uint32_t hqHashB;
	uint16_t hqLocale;
	uint16_t hqPlatform;
	uint32_t hqBlock;
} FileMpqHashEntry;

typedef struct FileMpqBlockEntry {
	uint32_t bqOffset;
	uint32_t bqSizeAlloc;
	uint32_t bqSizeFile;
	uint32_t bqFlags;
} FileMpqBlockEntry;
#pragma pack(pop)

// TPDEF PTR FCN UCHAR TGetNameFcn

// TPDEF PTR FCN VOID TCrypt

//////////////////////////////////////////////////
// trigs
//////////////////////////////////////////////////

typedef struct TriggerStruct {
	int _tx;
	int _ty;
	int _tmsg;
	int _tlvl;
} TriggerStruct;

//////////////////////////////////////////////////
// stores
//////////////////////////////////////////////////

typedef struct STextStruct {
	int _sx;
	int _syoff;
	char _sstr[112];
	bool _sjust;
	BYTE _sclr;
	bool _sline;
	bool _ssel;
	int _sval;
} STextStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(STextStruct) & (sizeof(STextStruct) - 1)) == 0, "Align STextStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// plrmsg
//////////////////////////////////////////////////

typedef struct _plrmsg {
	Uint32 time;
	BYTE player;
	char str[123];
} _plrmsg;

//////////////////////////////////////////////////
// capture
//////////////////////////////////////////////////

typedef struct FilePcxHeader {
	BYTE Manufacturer;
	BYTE Version;
	BYTE Encoding;
	BYTE BitsPerPixel;
	LE_UINT16 Xmin;
	LE_UINT16 Ymin;
	LE_UINT16 Xmax;
	LE_UINT16 Ymax;
	LE_UINT16 HDpi;
	LE_UINT16 VDpi;
	BYTE Colormap[48];
	BYTE Reserved;
	BYTE NPlanes;
	LE_UINT16 BytesPerLine;
	LE_UINT16 PaletteInfo;
	LE_UINT16 HscreenSize;
	LE_UINT16 VscreenSize;
	BYTE Filler[54];
} FilePcxHeader;

//////////////////////////////////////////////////
// encrypt
//////////////////////////////////////////////////

typedef struct TDataInfo {
	BYTE* const srcData;
	size_t srcOffset;
	BYTE* const destData;
	size_t destOffset;
	const size_t size;
	TDataInfo(BYTE* src, BYTE* dst, size_t s) : srcData(src), srcOffset(0), destData(dst), destOffset(0), size(s) {
	}
} TDataInfo;

/*
//////////////////////////////////////////////////
// data files
//////////////////////////////////////////////////

typedef struct FileDun {
	int16_t duWidth;
	int16_t duHeight;
	int16_t dtMegaTiles[duHeight][duWidth]; // pMegaTiles
	int16_t duItems[duHeight * 2][duWidth * 2]; // unused
	int16_t duMonsters[duHeight * 2][duWidth * 2]; // MonstConvTbl
	int16_t duObjects[duHeight * 2][duWidth * 2]; // ObjConvTbl
} FileDun;

typedef struct FileAmpFlag {
	BYTE diType;  // MAPFLAG_TYPE
	BYTE diFlags; // MAPFLAG_
} FileAmpFlag;
typedef struct FileAmp { // automaptype
	FileAmpFlag dsMegaTileFlags[dwMegas]; // piece_flag
} FileAmp;

typedef struct FileTil { // pMegaTiles
	int16_t dtTiles[dwMegas][4]; // dtMegaTiles: FileMin-index + 1
} FileTil;

typedef struct FileMin { // pMicroPieces
	int16_t dmTileBlocks[dwTiles][blocks]; // dwTiles: MAXTILES, blocks: [10 | 16], dmTileBlock: ASSET_MPL == 1 ? (encoding << 12 | FileCelLvl-index + 1) : (FileCelLvl-index + 1)
} FileMin;

typedef struct FileTmi { // microFlags
	BYTE diTileFlags[dwTiles]; // piece_micro_flag
} FileTmi;
typedef struct FileSol {
	BYTE dsTileFlags[dwTiles]; // piece_flag
} FileSol;

typedef struct FileCelLvl { // pMicroCels
	int32_t dclOffsets[dwCels]; // address of entry in dclCelData
	BYTE dclCelData[dwCels][celSize]; // cel data depending on the encoding. if ASSET_MPL == 1, the first byte is the encoding
} FileCelLvl;

typedef struct FileCelFrame {
	int16_t dcfHeader[5]; // address of an fragment/chunk in dcfCelData (optional in case of FileCel)
	BYTE dcfCelData[];
} FileCelFrame;

typedef struct FileCel {
	int32_t dcNumFrames;
	int32_t dcOffsets[dcNumFrames]; // address of an entry in dcCelFrames
	int32_t dcFileSize/NextOffset;
	FileCelFrame dcCelFrames[dcNumFrames];
} FileCel;

typedef struct FileCelGroup {
	int32_t dcgCelOffsets[dcNumCels]; // address of an entry in dcgCelData
	FileCel dcgCelData[dcNumCels];
} FileCelGroup;

typedef struct FileCl2 {
	int32_t dlNumFrames;
	int32_t dlOffsets[dcNumFrames]; // address of an entry in dcCelFrames
	int32_t dcFileSize/NextOffset;
	FileCelFrame dcCelFrames[dcNumFrames];
} FileCl2;

typedef struct FileCl2Group {
	int32_t dlgCelOffsets[dcNumCels]; // address of an entry in dlgCl2Data
	FileCl2 dlgCl2Data[dcNumCels];
} FileCl2Group;

typedef struct FilePal {
	BYTE dpColors[256][3];
} FilePal;

typedef struct FileTrn {
	BYTE drColors[256];
} FileTrn;

*/

DEVILUTION_END_NAMESPACE

#endif /* _STRUCTS_H */