/**
 * @file structs.h
 *
 * Various global structures.
 */
#ifndef _STRUCTS_H
#define _STRUCTS_H

DEVILUTION_BEGIN_NAMESPACE

#ifndef __AMIGA__
#define static_warning(x, msg) static_assert(x, msg)
#else
#define static_warning(x, msg)
#endif

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

// typedef int32_t WPARAM;
// typedef int32_t LPARAM;

//
// Handles
//
typedef void* HANDLE;

typedef HANDLE HMODULE, HDC, HINSTANCE;

typedef SDL_Event Dvl_Event;
typedef void (*WNDPROC)(const Dvl_Event*);

// typedef struct tagMSG {
// 	UINT message;
// 	WPARAM wParam;
// } MSG, *LPMSG;

//////////////////////////////////////////////////
// control
//////////////////////////////////////////////////

typedef struct POS32 {
	int x;
	int y;
} POS32;

typedef struct AREA32 {
	int w;
	int h;
} AREA32;

typedef struct RECT32 {
	int x;
	int y;
	int w;
	int h;
} RECT32;

typedef struct RECT_AREA32 {
	int x1;
	int y1;
	int x2;
	int y2;
} RECT_AREA32;

typedef struct CelImageBuf {
#if DEBUG_MODE
	WORD ciWidth; // number of images before loaded, but overwritten with width when loaded
	WORD ciFrameCnt; // number of images before loaded, but overwritten with width when loaded
#else
	DWORD ciWidth; // number of images before loaded, but overwritten with width when loaded
#endif
	BYTE imageData[32000]; // size does not matter, the struct is allocated dynamically
} CelImageBuf;

typedef struct CampaignMapEntry {
	BYTE ceDunType;
	BYTE ceIndex;
	BYTE ceLevel;
	BOOLEAN ceAvailable;
} CampaignMapEntry;

//////////////////////////////////////////////////
// items
//////////////////////////////////////////////////

typedef struct RANGE {
	BYTE from;
	BYTE to;
} RANGE;

typedef struct AffixData {
	BYTE PLPower; // item_effect_type
	int PLParam1;
	int PLParam2;
	RANGE PLRanges[NUM_IARS];
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
	uint16_t UICurs;
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
static_warning((sizeof(UniqItemData) & (sizeof(UniqItemData) - 1)) == 64, "Align UniqItemData to power of 2 for better performance.");
#endif

typedef struct ItemFileData {
	const char* ifName; // Map of item type .cel file names.
	int idSFX;          // sounds effect of dropping the item on ground (_sfx_id).
	int iiSFX;          // sounds effect of placing the item in the inventory (_sfx_id).
	int iAnimLen;       // item drop animation length
	ALIGNMENT64(3)
} ItemFileData;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_warning((sizeof(ItemFileData) & (sizeof(ItemFileData) - 1)) == 0, "Align ItemFileData to power of 2 for better performance.");
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
	ALIGNMENT(5, 4)
} ItemData;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_warning((sizeof(ItemData) & (sizeof(ItemData) - 1)) == 0, "Align ItemData to power of 2 for better performance.");
#endif

typedef struct ItemAffixStruct {
	BYTE asPower;
	union {
		struct {
			int asValue0;
			int asValue1;
		};
		struct {
			int asFrom;
			int asTo;
		};
	};
} ItemAffixStruct;

typedef struct ItemStruct {
	int32_t _iSeed;
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
	BYTE _iPrePower; // item_effect_type -- unused
	BYTE _iSufPower; // item_effect_type -- unused
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
	//BOOL _iPostDraw; // should be drawn during the post-phase (magic rock on the stand) -- unused
	BOOLEAN _iStatFlag;
	BOOLEAN _iIdentified;
	BYTE _iNumAffixes;
	BYTE _iUid; // unique_item_indexes
	int _ivalue;
	int _iIvalue;
	int _iAC;
	int _iCharges;
	int _iMaxCharges;
	int _iDurability;
	int _iMaxDur;
	int _iPLDam;
	int _iPLToHit;
	int _iPLStr;
	int _iPLMag;
	int _iPLDex;
	int _iPLVit;
	union {
		ItemAffixStruct _iAffixes[6];
		char _iPlrName[PLR_NAME_LEN];
	};
	ALIGNMENT(15, 13)
} ItemStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_warning((sizeof(ItemStruct) & (sizeof(ItemStruct) - 1)) == 0, "Align ItemStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// player
//////////////////////////////////////////////////

typedef struct PlrAnimType {
	char patTxt[4]; // suffix to select the player animation CL2
	int patGfxIdx;  // player_graphic_idx
} PlrAnimType;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_warning((sizeof(PlrAnimType) & (sizeof(PlrAnimType) - 1)) == 0, "Align PlrAnimType closer to power of 2 for better performance.");
#endif

typedef struct PlrAnimStruct {
	BYTE* paAnimData[NUM_DIRS];
	unsigned paFrames;
	int paAnimWidth;
} PlrAnimStruct;
#ifdef X86_32bit_COMP
static_warning((sizeof(PlrAnimStruct) & (sizeof(PlrAnimStruct) - 1)) == 32, "Align PlrAnimStruct closer to power of 2 for better performance.");
#elif defined(X86_64bit_COMP)
static_warning((sizeof(PlrAnimStruct) & (sizeof(PlrAnimStruct) - 1)) == 64, "Align PlrAnimStruct closer to power of 2 for better performance.");
#endif

typedef struct PlrSkillUse {
	BYTE _suSkill; // spell_id
	BYTE _suType;  // spell_type
	bool operator==(const PlrSkillUse & oval) const {
		//return _suSkill == oval._suSkill && _suType == oval._suType;
		return *(uint16_t*)&_suSkill == *(uint16_t*)&oval._suSkill;
	};
	bool operator!=(const PlrSkillUse & oval) const {
		return !(*this == oval);
	};
} PlrSkillUse;

typedef struct PlrSkillStruct {
	PlrSkillUse _psAttack; // attack skill
	PlrSkillUse _psMove;   // the movement skill
} PlrSkillStruct;
static_assert(sizeof(PlrSkillStruct) == 4 * sizeof(BYTE), "PlrSkillStruct is not packed tightly");

typedef struct PlayerStruct {
	int _pmode; // PLR_MODE
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
	int _px;      // Tile X-position where the player should be drawn
	int _py;      // Tile Y-position where the player should be drawn
	int _pfutx;   // Future tile X-position where the player will be at the end of its action
	int _pfuty;   // Future tile Y-position where the player will be at the end of its action
	int _poldx;   // Most recent tile X-position where the player was at the start of its action
	int _poldy;   // Most recent tile Y-position where the player was at the start of its action
	int _pxoff;   // Pixel X-offset from tile position where the player should be drawn
	int _pyoff;   // Pixel Y-offset from tile position where the player should be drawn
	int _pdir;    // Direction faced by player (direction enum)
	BYTE* _pAnimData;
	int _pAnimFrameLen; // Tick length of each frame in the current animation
	int _pAnimCnt;        // Increases by one each game tick, counting how close we are to _pAnimFrameLen
	unsigned _pAnimLen;   // Number of frames in current animation
	unsigned _pAnimFrame; // Current frame of animation.
	int _pAnimWidth;
	int _pAnimXOffset;
	unsigned _plid; // light id of the player
	unsigned _pvid; // vision id of the player
	PlrSkillStruct _pMainSkill; // the selected attack/movement skill for the primary action
	PlrSkillStruct _pAltSkill;  // the selected attack/movement skill for the secondary action
	PlrSkillStruct _pSkillHotKey[4];     // the skill selected by the hotkey
	PlrSkillStruct _pAltSkillHotKey[4];  // the skill selected by the alt-hotkey
	PlrSkillStruct _pSkillSwapKey[4];    // the skill selected by the hotkey after skill-set swap
	PlrSkillStruct _pAltSkillSwapKey[4]; // the skill selected by the alt-hotkey after skill-set swap
	BYTE _pSkillLvlBase[64]; // the skill levels of the player if they would not wear an item
	BYTE _pSkillActivity[64];
	unsigned _pSkillExp[64];
	uint64_t _pMemSkills;  // Bitmask of learned skills
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
	uint64_t _pISpells;  // Bitmask of skills available via equipped items (staff)
	BYTE _pSkillFlags;   // Bitmask of allowed skill-types (SFLAG_*)
	BOOLEAN _pInfraFlag; // unused
	BYTE _pgfxnum; // Bitmask indicating what variant of the sprite the player is using. Lower byte define weapon (anim_weapon_id) and higher values define armour (starting with anim_armor_id)
	BOOLEAN _pHasUnidItem; // whether the player has an unidentified (magic) item equipped
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
	BYTE _pIBaseHitBonus; // indicator whether the base BonusToHit of the items is positive/negative/neutral
	BYTE _pICritChance; // 200 == 100%
	uint16_t _pIBlockChance;
	unsigned _pIFlags; // item_special_effect
	BYTE _pIWalkSpeed;
	BYTE _pIRecoverySpeed;
	BYTE _pIBaseCastSpeed;
	BYTE _pAlign_B1;
	int _pIAbsAnyHit; // absorbed hit damage
	int _pIAbsPhyHit; // absorbed physical hit damage
	int8_t _pIBaseAttackSpeed;
	BYTE _pAlign_B2;
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
	ALIGNMENT(190, 104)
} PlayerStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_warning((sizeof(PlayerStruct) & (sizeof(PlayerStruct) - 1)) == 0, "Align PlayerStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// textdat
//////////////////////////////////////////////////

typedef struct TextData {
	const char* txtstr;
	BOOLEAN txtsfxset;
	BYTE txtdelay;
	int sfxnr;  // _sfx_id or sfx_set if txtsfxset is true
} TextData;

//////////////////////////////////////////////////
// missiles
//////////////////////////////////////////////////

typedef struct MissileData {
	int (*mAddProc)(int, int, int, int, int, int, int, int, int);
	void (*mProc)(int);
	BYTE mdFlags; // missile_flags
	BYTE mResist; // missile_resistance
	BYTE mFileNum; // missile_gfx_id
	int mlSFX; // sound effect when a missile is launched (_sfx_id)
	int miSFX; // sound effect on impact (_sfx_id)
	BYTE mlSFXCnt; // number of launch sound effects to choose from
	BYTE miSFXCnt; // number of impact sound effects to choose from
	BYTE mdPrSpeed; // speed of the projectile
	BYTE mdRange; // default range of the missile
	ALIGNMENT32(2)
} MissileData;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_warning((sizeof(MissileData) & (sizeof(MissileData) - 1)) == 0, "Align MissileData to power of 2 for better performance.");
#endif

typedef struct MisFileData {
	const char* mfName;
	const char* mfAnimTrans;
	int mfAnimFAmt;
	BOOLEAN mfDrawFlag;
	BOOLEAN mfAnimFlag;
	BOOLEAN mfLightFlag;
	BOOLEAN mfPreFlag;
	BYTE mfAnimFrameLen[16];
	BYTE mfAnimLen[16];
	int mfAnimWidth;
	int mfAnimXOffset; // could be calculated
	ALIGNMENT32(2)
} MisFileData;
#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_warning((sizeof(MisFileData) & (sizeof(MisFileData) - 1)) == 0, "Align MisFileData to power of 2 for better performance.");
#endif

typedef struct MissileStruct {
	int _miType;   // missile_id
	BYTE _miFlags; // missile_flags
	BYTE _miResist; // missile_resistance
	BYTE _miFileNum; // missile_gfx_id
	BOOLEAN _miDelFlag; // should be deleted
	int _miUniqTrans; // use unique color-transformation when drawing
	BOOLEAN _miDrawFlag; // should be drawn
	BOOLEAN _miAnimFlag;
	BOOLEAN _miLightFlag; // use light-transformation when drawing
	BOOLEAN _miPreFlag; // should be drawn in the pre-phase
	BYTE* _miAnimData;
	int _miAnimFrameLen; // Tick length of each frame in the current animation
	int _miAnimLen;   // Number of frames in current animation
	int _miAnimWidth;
	int _miAnimXOffset;
	int _miAnimCnt; // Increases by one each game tick, counting how close we are to _miAnimFrameLen
	int _miAnimAdd;
	int _miAnimFrame; // Current frame of animation.
	int _misx;    // Initial tile X-position
	int _misy;    // Initial tile Y-position
	int _mix;     // Tile X-position where the missile should be drawn
	int _miy;     // Tile Y-position where the missile should be drawn
	int _mixoff;  // Pixel X-offset from tile position where the missile should be drawn
	int _miyoff;  // Pixel Y-offset from tile position where the missile should be drawn
	int _mixvel;  // Missile tile (X - Y)-velocity while moving. This gets added onto _mitxoff each game tick
	int _miyvel;  // Missile tile (X + Y)-velocity while moving. This gets added onto _mityoff each game tick
	int _mitxoff; // How far the missile has travelled in its lifespan along the (X - Y)-axis. mix/miy/mxoff/myoff get updated every game tick based on this
	int _mityoff; // How far the missile has travelled in its lifespan along the (X + Y)-axis. mix/miy/mxoff/myoff get updated every game tick based on this
	int _miDir;   // The direction of the missile
	int _miSpllvl;
	int _miSource; // missile_source_type
	int _miCaster;
	int _miMinDam;
	int _miMaxDam;
	// int _miRndSeed;
	int _miRange;    // Time to live for the missile in game ticks, when negative the missile will be deleted
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
static_warning((sizeof(MissileStruct) & (sizeof(MissileStruct) - 1)) == 128, "Align MissileStruct closer to power of 2 for better performance.");
#elif defined(X86_64bit_COMP)
static_warning((sizeof(MissileStruct) & (sizeof(MissileStruct) - 1)) == 0, "Align MissileStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// effects/sound
//////////////////////////////////////////////////

typedef struct _Mix_Audio Mix_Audio;

typedef struct SoundSample final {
	Uint32 lastTc;
	Mix_Audio* soundData;

	void Release();
	bool IsPlaying() const;
	bool IsLoaded() const {
		return soundData != NULL;
	}
	void Play(int lVolume, int lPan, int channel);
	void SetChunk(BYTE* fileData, size_t dwBytes, bool stream);
	//int TrackLength();
} SoundSample;

typedef struct SFXData {
	BYTE bFlags; // sfx_flag
} SFXData;

typedef struct SFXFileData {
	const char* pszName;
} SFXFileData;

//////////////////////////////////////////////////
// monster
//////////////////////////////////////////////////

typedef struct MonAnimStruct {
	BYTE* maAnimData[NUM_DIRS];
	int maFrames;
	int maFrameLen;
} MonAnimStruct;
#ifdef X86_32bit_COMP
static_warning((sizeof(MonAnimStruct) & (sizeof(MonAnimStruct) - 1)) == 32, "Align MonAnimStruct closer to power of 2 for better performance.");
#elif defined(X86_64bit_COMP)
static_warning((sizeof(MonAnimStruct) & (sizeof(MonAnimStruct) - 1)) == 64, "Align MonAnimStruct closer to power of 2 for better performance.");
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
	unsigned mFlags;  // _monster_flag
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
	uint16_t mExp;
	ALIGNMENT(5, 2)
} MonsterData;
#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_warning((sizeof(MonsterData) & (sizeof(MonsterData) - 1)) == 0, "Align MonsterData to power of 2 for better performance.");
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
static_warning((sizeof(MonFileData) & (sizeof(MonFileData) - 1)) == 0, "Align MonFileData to power of 2 for better performance.");
#elif defined(X86_64bit_COMP)
static_warning((sizeof(MonFileData) & (sizeof(MonFileData) - 1)) == 64, "Align MonFileData to power of 2 for better performance.");
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
	unsigned cmFlags;    // _monster_flag
	int cmHit;           // hit chance (melee+projectile)
	int cmMinDamage;
	int cmMaxDamage;
	int cmHit2;          // hit chance of special melee attacks
	int cmMinDamage2;
	int cmMaxDamage2;
	int cmMagic;         // hit chance of magic-projectile
	int cmArmorClass;    // AC+evasion: used against physical-hit (melee+projectile)
	int cmEvasion;       // evasion: used against magic-projectile
	unsigned cmMagicRes; // resistances of the monster (_monster_resistance)
	unsigned cmExp;
	int cmWidth;
	int cmXOffset;
	BYTE cmAFNum;
	BYTE cmAFNum2;
	uint16_t cmAlign_0; // unused
	int cmMinHP;
	int cmMaxHP;
	ALIGNMENT(24, 17);
} MapMonData;
#ifdef X86_32bit_COMP
static_warning((sizeof(MapMonData) & (sizeof(MapMonData) - 1)) == 0, "Align MapMonData closer to power of 2 for better performance.");
#elif defined(X86_64bit_COMP)
static_warning((sizeof(MapMonData) & (sizeof(MapMonData) - 1)) == 512, "Align MapMonData closer to power of 2 for better performance.");
#endif
#pragma pack(pop)
typedef struct MonsterStruct {
	int _mmode; // MON_MODE
	unsigned _msquelch;
	BYTE _mMTidx;
	BYTE _mpathcount; // unused
	BYTE _mAlign_1;   // unused
	BYTE _mgoal;
	int _mgoalvar1;
	int _mgoalvar2;
	int _mgoalvar3;
	int _mx;           // Tile X-position where the monster should be drawn
	int _my;           // Tile Y-position where the monster should be drawn
	int _mfutx;        // Future tile X-position where the monster will be at the end of its action
	int _mfuty;        // Future tile Y-position where the monster will be at the end of its action
	int _moldx;        // Most recent tile X-position where the monster was at the start of its action
	int _moldy;        // Most recent tile Y-position where the monster was at the start of its action
	int _mxoff;        // Pixel X-offset from tile position where the monster should be drawn
	int _myoff;        // Pixel Y-offset from tile position where the monster should be drawn
	int _mdir;         // Direction faced by monster (direction enum)
	int _menemy;       // The current target of the monster. An index in to either a player(zero or positive) or a monster (negative)
	BYTE _menemyx;     // Future (except for teleporting) tile X-coordinate of the enemy
	BYTE _menemyy;     // Future (except for teleporting) tile Y-coordinate of the enemy
	BYTE _mListener;   // the player to whom the monster is talking to (unused)
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
	int _mVar6;
	int _mVar7;
	int _mVar8;
	int _mmaxhp;
	int _mhitpoints;
	int _mlastx; // the last known (future) tile X-coordinate of the enemy
	int _mlasty; // the last known (future) tile Y-coordinate of the enemy
	int32_t _mRndSeed;
	int32_t _mAISeed;
	BYTE _muniqtype;
	BYTE _muniqtrans;
	BYTE _mNameColor;  // color of the tooltip. white: normal, blue: pack; gold: unique. (text_color)
	BYTE _mlid;        // light id of the monster
	BYTE _mleader;     // the leader of the monster
	BYTE _mleaderflag; // the status of the monster's leader
	BYTE _mpacksize;   // the number of 'pack'-monsters close to their leader
	BYTE _mvid;        // vision id of the monster (for minions only)
	const char* _mName;
	uint16_t _mFileNum; // _monster_gfx_id
	BYTE _mLevel;
	BYTE _mSelFlag;
	MonsterAI _mAI;
	unsigned _mFlags;    // _monster_flag
	int _mHit;           // hit chance (melee+projectile)
	int _mMinDamage;
	int _mMaxDamage;
	int _mHit2;          // hit chance of special melee attacks
	int _mMinDamage2;
	int _mMaxDamage2;
	int _mMagic;         // hit chance of magic-projectile
	int _mArmorClass;    // AC+evasion: used against physical-hit (melee+projectile)
	int _mEvasion;       // evasion: used against magic-projectile
	unsigned _mMagicRes; // resistances of the monster (_monster_resistance)
	unsigned _mExp;
	int _mAnimWidth;
	int _mAnimXOffset;
	BYTE _mAFNum;  // action frame number of the attack animation
	BYTE _mAFNum2; // action frame number of the special animation
	uint16_t _mAlign_0; // unused
	int _mType; // _monster_id
	MonAnimStruct* _mAnims;
	ALIGNMENT(6, 2)
} MonsterStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_warning((sizeof(MonsterStruct) & (sizeof(MonsterStruct) - 1)) == 0, "Align MonsterStruct to power of 2 for better performance.");
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
	uint16_t mMagicRes;  // resistances in normal and nightmare difficulties (_monster_resistance)
	uint16_t mMagicRes2; // resistances in hell difficulty (_monster_resistance)
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
static_warning((sizeof(UniqMonData) & (sizeof(UniqMonData) - 1)) == 0, "Align UniqMonData to power of 2 for better performance.");
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
	//BYTE oBreak;
	BOOLEAN oMissFlag;
	BYTE oDoorFlag;   // object_door_type
	BYTE oSelFlag;
	BYTE oPreFlag;
	BOOLEAN oTrapFlag;
	BYTE oAlign[1];
} ObjectData;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_warning((sizeof(ObjectData) & (sizeof(ObjectData) - 1)) == 0, "Align ObjectData closer to power of 2 for better performance.");
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
	BYTE oBreak; // object_break_mode
	ALIGNMENT32(1)
} ObjFileData;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_warning((sizeof(ObjFileData) & (sizeof(ObjFileData) - 1)) == 0, "Align ObjFileData closer to power of 2 for better performance.");
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
	BYTE _oBreak; // object_break_mode
	BYTE _oTrapChance;
	BYTE _oAlign;
	BOOLEAN _oMissFlag;
	BYTE _oDoorFlag; // object_door_type
	BYTE _oSelFlag;
	BOOLEAN _oPreFlag;
	unsigned _olid; // light id of the object
	int32_t _oRndSeed;
	int _oVar1;
	int _oVar2;
	int _oVar3;
	int _oVar4;
	int _oVar5;
	int _oVar6;
	int _oVar7;
	int _oVar8;
	ALIGNMENT(8, 6)
} ObjectStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_warning((sizeof(ObjectStruct) & (sizeof(ObjectStruct) - 1)) == 0, "Align ObjectStruct to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// portal
//////////////////////////////////////////////////

typedef struct PortalStruct {
	int _rlevel; // the destination-level of the portal (dungeon_level). DLV_TOWN if not open.
	int _rx;
	int _ry;
	ALIGNMENT(1, 1)
} PortalStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_warning((sizeof(PortalStruct) & (sizeof(PortalStruct) - 1)) == 0, "Align PortalStruct closer to power of 2 for better performance.");
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
	void operator=(unsigned long val) {
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
	void operator=(long val) {
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
	PlrSkillStruct pSkillHotKey[4];     // the skill selected by the hotkey
	PlrSkillStruct pAltSkillHotKey[4];  // the skill selected by the alt-hotkey
	PlrSkillStruct pSkillSwapKey[4];    // the skill selected by the hotkey after skill-set swap
	PlrSkillStruct pAltSkillSwapKey[4]; // the skill selected by the alt-hotkey after skill-set swap
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
typedef struct LSaveGameDynLvlStruct {
	BYTE vdLevel;
	BYTE vdType;
} LSaveGameDynLvlStruct;

typedef struct LSaveGameHeaderStruct {
	LE_INT32 vhInitial;
	LE_UINT32 vhLogicTurn;
	LE_UINT32 vhSentCycle;
	LE_INT32 vhSeeds[NUM_LEVELS];
	LSaveGameDynLvlStruct vhDynLvls[NUM_DYNLVLS];
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
	BYTE vhDifficulty;
	BYTE vhTownWarps;
	BYTE vhWaterDone;
	BYTE vhAutoMapScale;
	BYTE vhMiniMapScale;
	BYTE vhNormalMapScale;
	LE_UINT32 vhLvlVisited;
} LSaveGameHeaderStruct;

typedef struct LSaveGameMetaStruct {
	LE_UINT32 vaboylevel;
	LE_INT32 vanumpremium;
	LE_INT32 vapremiumlevel;
	LE_INT32 vaAutoMapXOfs;
	LE_INT32 vaAutoMapYOfs;
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
	PkItemStruct viPkItem;
	LE_INT32 vix;
	LE_INT32 viy;
	BYTE viMagical;  // item_quality
	BYTE viSelFlag;
	BOOLEAN viFloorFlag;
	BOOLEAN viAnimFlag;
	int32_t viAnimDataAlign;      // PSX name -> ItemFrame
	uint32_t viAnimFrameLenAlign; // Tick length of each frame in the current animation
	LE_UINT32 viAnimCnt;      // Increases by one each game tick, counting how close we are to viAnimFrameLen
	LE_UINT32 viAnimLen;      // Number of frames in current animation
	LE_UINT32 viAnimFrame;    // Current frame of animation.
	// LE_INT32 viPostDraw;
} LSaveItemStruct;

typedef struct LSavePlayerStruct {
	LE_INT32 vpmode; // PLR_MODE
	LE_INT32 vpDestAction;
	LE_INT32 vpDestParam1;
	LE_INT32 vpDestParam2;
	LE_INT32 vpDestParam3; // the skill to be used in case of skill based actions
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
	LE_INT32 vpx;      // Tile X-position where the player should be drawn
	LE_INT32 vpy;      // Tile Y-position where the player should be drawn
	LE_INT32 vpfutx;   // Future tile X-position where the player will be at the end of its action
	LE_INT32 vpfuty;   // Future tile Y-position where the player will be at the end of its action
	LE_INT32 vpoldx;   // Most recent tile X-position where the player was at the start of its action
	LE_INT32 vpoldy;   // Most recent tile Y-position where the player was at the start of its action
	LE_INT32 vpxoff;   // Pixel X-offset from tile position where the player should be drawn
	LE_INT32 vpyoff;   // Pixel Y-offset from tile position where the player should be drawn
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
	PlrSkillStruct vpMainSkill; // the selected attack/movement skill for the primary action
	PlrSkillStruct vpAltSkill;  // the selected attack/movement skill for the secondary action
	PlrSkillStruct vpSkillHotKey[4];     // the skill selected by the hotkey
	PlrSkillStruct vpAltSkillHotKey[4];  // the skill selected by the alt-hotkey
	PlrSkillStruct vpSkillSwapKey[4];    // the skill selected by the hotkey after skill-set swap
	PlrSkillStruct vpAltSkillSwapKey[4]; // the skill selected by the alt-hotkey after skill-set swap
	BYTE vpSkillLvlBase[64]; // the skill levels of the player if they would not wear an item
	BYTE vpSkillActivity[64];
	LE_UINT32 vpSkillExp[64];
	LE_UINT64 vpMemSkills;  // Bitmask of learned skills
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
	LE_INT32 vmmode; // MON_MODE
	LE_UINT32 vmsquelch;
	BYTE vmMTidx;
	BYTE vmpathcount; // unused
	BYTE vmAlign_1;   // unused
	BYTE vmgoal;
	LE_INT32 vmgoalvar1;
	LE_INT32 vmgoalvar2;
	LE_INT32 vmgoalvar3;
	LE_INT32 vmx;           // Tile X-position where the monster should be drawn
	LE_INT32 vmy;           // Tile Y-position where the monster should be drawn
	LE_INT32 vmfutx;        // Future tile X-position where the monster will be at the end of its action
	LE_INT32 vmfuty;        // Future tile Y-position where the monster will be at the end of its action
	LE_INT32 vmoldx;        // Most recent tile X-position where the monster was at the start of its action
	LE_INT32 vmoldy;        // Most recent tile Y-position where the monster was at the start of its action
	LE_INT32 vmxoff;        // Pixel X-offset from tile position where the monster should be drawn
	LE_INT32 vmyoff;        // Pixel Y-offset from tile position where the monster should be drawn
	LE_INT32 vmdir;         // Direction faced by monster (direction enum)
	LE_INT32 vmenemy;       // The current target of the monster. An index in to either the plr or monster array depending on _mFlags (MFLAG_TARGETS_MONSTER)
	BYTE vmenemyx;          // Future (except for teleporting) tile X-coordinate of the enemy
	BYTE vmenemyy;          // Future (except for teleporting) tile Y-coordinate of the enemy
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
	LE_INT32 vmVar6;
	LE_INT32 vmVar7;
	LE_INT32 vmVar8;
	LE_INT32 vmmaxhp;
	LE_INT32 vmhitpoints;
	LE_INT32 vmlastx; // the last known (future) tile X-coordinate of the enemy
	LE_INT32 vmlasty; // the last known (future) tile Y-coordinate of the enemy
	LE_INT32 vmRndSeed;
	LE_INT32 vmAISeed;
	BYTE vmuniqtype;
	BYTE vmuniqtrans;
	BYTE vmNameColor;  // color of the tooltip. white: normal, blue: pack; gold: unique. (text_color)
	BYTE vmlid;        // light id of the monster
	BYTE vmleader;     // the leader of the monster
	BYTE vmleaderflag; // the status of the monster's leader
	BYTE vmpacksize;   // the number of 'pack'-monsters close to their leader
	BYTE vmvid;        // vision id of the monster (for minions only)
	INT vmNameAlign;
	LE_UINT16 vmFileNum;
	BYTE vmLevel;
	BYTE vmSelFlag;
	BYTE vmAI_aiType;   // MonsterAI.aiType
	BYTE vmAI_aiInt;    // MonsterAI.aiInt
	BYTE vmAI_aiParam1; // MonsterAI.aiParam1
	BYTE vmAI_aiParam2; // MonsterAI.aiParam2
	LE_UINT32 vmFlags;
	LE_INT32 vmHit;    // hit chance (melee+projectile)
	LE_INT32 vmMinDamage;
	LE_INT32 vmMaxDamage;
	LE_INT32 vmHit2;   // hit chance of special melee attacks
	LE_INT32 vmMinDamage2;
	LE_INT32 vmMaxDamage2;
	LE_INT32 vmMagic;      // hit chance of magic-projectile
	LE_INT32 vmArmorClass; // AC+evasion: used against physical-hit (melee+projectile)
	LE_INT32 vmEvasion;    // evasion: used against magic-projectile
	LE_UINT32 vmMagicRes;  // resistances of the monster (_monster_resistance)
	LE_UINT32 vmExp;
} LSaveMonsterStruct;

typedef struct LSaveMissileStruct {
	LE_INT32 vmiType;   // missile_id
	BYTE vmiFlags; // missile_flags
	BYTE vmiResist; // missile_resistance
	BYTE vmiFileNum; // missile_gfx_id
	BOOLEAN vmiDelFlag; // should be deleted
	LE_INT32 vmiUniqTrans; // use unique color-transformation when drawing
	BOOLEAN vmiDrawFlagAlign; // should be drawn
	BOOLEAN vmiAnimFlagAlign;
	BOOLEAN vmiLightFlagAlign; // use light-transformation when drawing
	BOOLEAN vmiPreFlagAlign; // should be drawn in the pre-phase
	INT vmiAnimDataAlign;
	INT vmiAnimFrameLenAlign; // Tick length of each frame in the current animation
	INT vmiAnimLenAlign;   // Number of frames in current animation
	INT vmiAnimWidthAlign;
	INT vmiAnimXOffsetAlign;
	LE_INT32 vmiAnimCnt; // Increases by one each game tick, counting how close we are to vmiAnimFrameLen
	LE_INT32 vmiAnimAdd;
	LE_INT32 vmiAnimFrame; // Current frame of animation.
	LE_INT32 vmisx;    // Initial tile X-position
	LE_INT32 vmisy;    // Initial tile Y-position
	LE_INT32 vmix;     // Tile X-position where the missile should be drawn
	LE_INT32 vmiy;     // Tile Y-position where the missile should be drawn
	LE_INT32 vmixoff;  // Pixel X-offset from tile position where the missile should be drawn
	LE_INT32 vmiyoff;  // Pixel Y-offset from tile position where the missile should be drawn
	LE_INT32 vmixvel;  // Missile tile (X - Y)-velocity while moving. This gets added onto _mitxoff each game tick
	LE_INT32 vmiyvel;  // Missile tile (X + Y)-velocity while moving. This gets added onto _mityoff each game tick
	LE_INT32 vmitxoff; // How far the missile has travelled in its lifespan along the (X - Y)-axis. mix/miy/mxoff/myoff get updated every game tick based on this
	LE_INT32 vmityoff; // How far the missile has travelled in its lifespan along the (X + Y)-axis. mix/miy/mxoff/myoff get updated every game tick based on this
	LE_INT32 vmiDir;   // The direction of the missile
	LE_INT32 vmiSpllvl;
	LE_INT32 vmiSource; // missile_source_type
	LE_INT32 vmiCaster;
	LE_INT32 vmiMinDam;
	LE_INT32 vmiMaxDam;
	LE_INT32 vmiRange; // Time to live for the missile in game ticks, when negative the missile will be deleted
	LE_UINT32 vmiLid;  // light id of the missile
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
	BYTE voBreak; // object_break_mode
	BYTE voTrapChance;
	BYTE voAlign;
	BOOLEAN voMissFlag;
	BYTE voDoorFlag; // object_door_type
	BYTE voSelFlag;
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
	LE_INT32 vrlevel;
	LE_INT32 vrx;
	LE_INT32 vry;
	LE_INT32 vrAlign0;
} LSavePortalStruct;

#pragma pack(pop)

//////////////////////////////////////////////////
// msg
//////////////////////////////////////////////////

#pragma pack(push, 1)
typedef struct CmdSkillUse {
	BYTE skill;
	int8_t from;
	bool operator==(const CmdSkillUse & oval) const {
		//return skill == oval.skill && from == oval.from;
		return *(uint16_t*)&skill == *(uint16_t*)&oval.skill;
	};
	bool operator!=(const CmdSkillUse & oval) const {
		return !(*this == oval);
	};
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

typedef struct TCmdNewLvl {
	BYTE bCmd;
	BYTE bPlayers;
	BYTE bFom;
	BYTE bLevel;
} TCmdNewLvl;

typedef struct TCmdCreateLvl {
	BYTE bCmd;
	BYTE clPlayers;
	LE_INT32 clSeed;
	BYTE clLevel;
	BYTE clType;
} TCmdCreateLvl;

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
	BYTE bsLen;
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
	BYTE spManaShield;
	BYTE spInvincible;
	BYTE spDestAction;
	LE_INT32 spDestParam1;
	LE_INT32 spDestParam2;
	LE_INT32 spDestParam3;
	LE_INT32 spDestParam4;
	LE_INT16 spTimer[NUM_PLRTIMERS];
	BYTE spx;      // Tile X-position where the player should be drawn
	BYTE spy;      // Tile Y-position where the player should be drawn
//	BYTE spfutx;   // Future tile X-position where the player will be at the end of its action
//	BYTE spfuty;   // Future tile Y-position where the player will be at the end of its action
//	BYTE spoldx;   // Most recent tile X-position where the player was at the start of its action
//	BYTE spoldy;   // Most recent tile Y-position where the player was at the start of its action
//	LE_INT32 spxoff;   // Pixel X-offset from tile position where the player should be drawn
//	LE_INT32 spyoff;   // Pixel Y-offset from tile position where the player should be drawn
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
	//BYTE smAlign_1;   // unused
	BYTE smGoal;
	LE_INT32 smGoalvar1;
	LE_INT32 smGoalvar2;
	LE_INT32 smGoalvar3;
	BYTE smx;          // Tile X-position where the monster should be drawn
	BYTE smy;          // Tile Y-position where the monster should be drawn
//	BYTE smfutx;       // Future tile X-position where the monster will be at the end of its action
//	BYTE smfuty;       // Future tile Y-position where the monster will be at the end of its action
//	BYTE smoldx;       // Most recent tile X-position where the monster was at the start of its action
//	BYTE smoldy;       // Most recent tile Y-position where the monster was at the start of its action
//	LE_INT32 smxoff;   // Pixel X-offset from tile position where the monster should be drawn
//	LE_INT32 smyoff;   // Pixel Y-offset from tile position where the monster should be drawn
	BYTE smdir;        // Direction faced by monster (direction enum)
	LE_INT32 smenemy;  // The current target of the monster. An index in to either a player(zero or positive) or a monster (negative)
	BYTE smenemyx;     // Future (except for teleporting) tile X-coordinate of the enemy
	BYTE smenemyy;     // Future (except for teleporting) tile Y-coordinate of the enemy
	BYTE smListener;   // the player to whom the monster is talking to (unused)
	BOOLEAN smDelFlag; // unused
	BYTE smAnimCnt;    // Increases by one each game tick, counting how close we are to _mAnimFrameLen
	BYTE smAnimFrame;  // Current frame of animation.
	LE_INT32 smVar1;
	LE_INT32 smVar2;
	LE_INT32 smVar3;
	LE_INT32 smVar4;
	LE_INT32 smVar5;
	LE_INT32 smVar6;
	LE_INT32 smVar7;
	LE_INT32 smVar8;
	LE_INT32 smHitpoints;
	BYTE smlastx; // the last known (future) tile X-coordinate of the enemy
	BYTE smlasty; // the last known (future) tile Y-coordinate of the enemy
	//BYTE smLeader; // the leader of the monster
	BYTE smLeaderflag; // the status of the monster's leader
	//BYTE smPacksize; // the number of 'pack'-monsters close to their leader
	//BYTE falign_CB;
	LE_UINT32 smFlags;
} TSyncLvlMonster;

typedef struct TSyncLvlMissile {
	LE_UINT16 smiMi;
	BYTE smiType;   // missile_id
	BYTE smiFileNum; // missile_gfx_id
	BYTE smiAnimCnt; // Increases by one each game tick, counting how close we are to _miAnimFrameLen
	int8_t smiAnimAdd;
	BYTE smiAnimFrame; // Current frame of animation.
	BYTE smiDir;   // The direction of the missile
	BYTE smisx;    // Initial tile X-position for missile
	BYTE smisy;    // Initial tile Y-position for missile
	BYTE smix;     // Tile X-position where the missile should be drawn
	BYTE smiy;     // Tile Y-position where the missile should be drawn
	LE_INT32 smixoff;  // Pixel X-offset from tile position where the missile should be drawn
	LE_INT32 smiyoff;  // Pixel Y-offset from tile position where the missile should be drawn
	LE_INT32 smixvel;  // Missile tile (X - Y)-velocity while moving. This gets added onto _mitxoff each game tick
	LE_INT32 smiyvel;  // Missile tile (X + Y)-velocity while moving. This gets added onto _mityoff each game tick
	LE_INT32 smitxoff; // How far the missile has travelled in its lifespan along the (X - Y)-axis. mix/miy/mxoff/myoff get updated every game tick based on this
	LE_INT32 smityoff; // How far the missile has travelled in its lifespan along the (X + Y)-axis. mix/miy/mxoff/myoff get updated every game tick based on this
	LE_INT32 smiSpllvl; // TODO: int?
	LE_INT32 smiSource; // TODO: int?
	LE_INT32 smiCaster; // TODO: int?
	LE_INT32 smiMinDam;
	LE_INT32 smiMaxDam;
	// LE_INT32 smiRndSeed;
	LE_INT32 smiRange; // Time to live for the missile in game ticks, when negative the missile will be deleted
	LE_INT32 smiVar1;
	LE_INT32 smiVar2;
	LE_INT32 smiVar3;
	LE_INT32 smiVar4;
	LE_INT32 smiVar5;
	LE_INT32 smiVar6;
	LE_INT32 smiVar7; // distance travelled in case of ARROW missiles
	LE_INT32 smiVar8; // last target in case of non-DOT missiles
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

typedef struct DDMonster {
	BYTE dmCmd;
	BYTE dmx;
	BYTE dmy;
	BYTE dmdir;
	BYTE dmleaderflag;
	BYTE dmWhoHit;
	BYTE dmSIdx;
	LE_UINT32 dmactive;
	LE_INT32 dmhitpoints;
} DDMonster;

typedef struct DDObject {
	BYTE bCmd;
} DDObject;

typedef struct DDItem {
	BYTE bCmd;
	BYTE x;
	BYTE y;
	PkItemStruct item;
} DDItem;

typedef struct DDLevel {
	DDItem item[MAXITEMS];
	DDObject object[MAXOBJECTS];
	DDMonster monster[MAXMONSTERS];
} DDLevel;

typedef struct DDDynLevel {
	LE_INT32 dlSeed; // the seed of the dynamic level
	BYTE dlLevel;    // the difficulty level of the dynamic level
	BYTE dlType;     // dungeon_type (random in case of DTYPE_TOWN)
} DDDynLevel;

typedef struct LocalLevel {
	BYTE automapsv[MAXDUNX][MAXDUNY]; // TODO: compress the data?
} LocalLevel;

typedef struct DDPortal {
	BYTE level;
	BYTE x;
	BYTE y;
} DDPortal;

typedef struct DDQuest {
	BYTE qstate;
	BYTE qlog;
	BYTE qvar1;
} DDQuest;

typedef struct DDJunk {
	// DDPortal jPortals[MAXPORTAL];
	// DDQuest jQuests[NUM_QUESTS];
	// DDDynLevel[NUM_DYNLVLS]
	BYTE jGolems[MAX_MINIONS];
} DDJunk;

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
			LocalLevel ddLocal[NUM_LEVELS]; // automap
			DDJunk ddJunk;                  // portals and quests
			DDLevel ddLevel[NUM_LEVELS];    // items/monsters/objects
			BYTE ddLevelPlrs[NUM_LEVELS];   // the number of players when the level was 'initialized'

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
	int _dLevelIdx;   // dungeon_level / NUM_LEVELS
	int _dLevelNum;   // index in AllLevels (dungeon_level / NUM_FIXLVLS)
	bool _dSetLvl;    // cached flag if the level is a set-level
	bool _dDynLvl;    // cached flag if the level is a dynamic-level
	int _dLevel;      // cached difficulty value of the level
	int _dType;       // cached type of the level (dungeon_type)
	int _dDunType;    // cached type of the dungeon (dungeon_gen_type)
	int _dLevelPlyrs; // cached number of players when the level was 'initialized'
	int _dLevelBonus; // cached level bonus
} LevelStruct;

typedef struct LevelFileData {
	const char* dSubtileSettings;
	const char* dTileFlags;
	const char* dMicroCels;
	const char* dMegaTiles;
	const char* dMiniTiles;
	const char* dSpecCels;
	const char* dLightTrns;
} LevelFileData;

typedef struct LevelData {
	BYTE dLevel;
	BOOLEAN dSetLvl;
	BYTE dType;    // dungeon_type
	BYTE dDunType; // dungeon_gen_type
	BYTE dMusic;   // _music_id
	BYTE dfindex;  // level_graphic_id
	BYTE dMicroTileLen;
	BYTE dBlocks;
	const char* dLevelName;
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
} LevelData;

#ifdef X86_32bit_COMP
static_warning((sizeof(LevelData) & (sizeof(LevelData) - 1)) == 0, "Align LevelData to power of 2 for better performance.");
#elif defined(X86_64bit_COMP)
static_warning((sizeof(LevelData) & (sizeof(LevelData) - 1)) == 64, "Align LevelData to power of 2 for better performance.");
#endif

typedef struct WarpStruct {
	int _wx;
	int _wy;
	int _wtype; // dungeon_warp_type
	int _wlvl;  // dungeon_level / _setlevels
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

typedef struct DynLevelStruct {
	// int32_t _dnSeed; -- stored in glSeedTbl
	// BYTE _dnPlayers; -- stored in gsDeltaData.ddLevelPlrs
	BYTE _dnLevel;
	BYTE _dnType;
} DynLevelStruct;

typedef struct QuestStruct {
	BYTE _qactive; // quest_state
	BYTE _qvar1; // quest parameter which is synchronized with the other players
	BYTE _qvar2; // quest parameter which is NOT synchronized with the other players
	BOOLEAN _qlog;
	unsigned _qmsg;
} QuestStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_warning((sizeof(QuestStruct) & (sizeof(QuestStruct) - 1)) == 0, "Align QuestStruct to power of 2 for better performance.");
#endif

typedef struct QuestData {
	BYTE _qdlvl; // dungeon_level
	BYTE _qslvl; // _setlevels
	int _qdmsg;  // _speech_id
	const char* _qlstr; // quest title
} QuestData;

//////////////////////////////////////////////////
// gamemenu/gmenu
//////////////////////////////////////////////////

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

//////////////////////////////////////////////////
// spells
//////////////////////////////////////////////////

typedef struct SkillDetails {
	int type; // skill_details_type
	int v0;
	int v1;
	int v2;
} SkillDetails;

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
static_warning((sizeof(SpellData) & (sizeof(SpellData) - 1)) == 0, "Align SpellData to power of 2 for better performance.");
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
	ALIGNMENT(6, 6)
} ROOMHALLNODE;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_warning((sizeof(ROOMHALLNODE) & (sizeof(ROOMHALLNODE) - 1)) == 0, "Align ROOMHALLNODE to power of 2 for better performance.");
#endif

typedef struct L1ROOM {
	int lrx;
	int lry;
	int lrw;
	int lrh;
} L1ROOM;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_warning((sizeof(L1ROOM) & (sizeof(L1ROOM) - 1)) == 0, "Align L1ROOM to power of 2 for better performance.");
#endif

typedef struct ThemePosDir {
	int tpdx;
	int tpdy;
	int tpdvar1;
	int tpdvar2; // unused
} ThemePosDir;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_warning((sizeof(ThemePosDir) & (sizeof(ThemePosDir) - 1)) == 0, "Align ThemePosDir to power of 2 for better performance.");
#endif

/** The number of generated rooms in cathedral. */
#define L1_MAXROOMS ((DSIZEX * DSIZEY) / sizeof(L1ROOM))
/** The number of generated rooms in catacombs. */
#define L2_MAXROOMS 32
static_assert(L2_MAXROOMS * sizeof(ROOMHALLNODE) <= (DSIZEX * DSIZEY), "RoomList is too large for DrlgMem.");
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
static_warning((sizeof(ThemeStruct) & (sizeof(ThemeStruct) - 1)) == 0, "Align ThemeStruct to power of 2 for better performance.");
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
static_warning((sizeof(LightListStruct) & (sizeof(LightListStruct) - 1)) == 0, "Align LightListStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// storm-net
//////////////////////////////////////////////////

typedef uint8_t plr_t;
typedef uint32_t cookie_t;
typedef uint32_t turn_t;

#pragma pack(push, 1)
typedef struct SNetGameData {
	LE_UINT32 ngVersionId;
	LE_INT32 ngSeed;
	BYTE ngDifficulty;
	BYTE ngTickRate;
	BYTE ngNetUpdateRate; // (was defaultturnssec in vanilla)
	BYTE ngMaxPlayers;
	SNetGameData() {
		memset(&ngVersionId, 0, sizeof(SNetGameData));
	}
	SNetGameData(const SNetGameData &other) {
		memcpy(&ngVersionId, &other, sizeof(SNetGameData));
	}
	void operator=(const SNetGameData &other) {
		memcpy(&ngVersionId, &other, sizeof(SNetGameData));
	};
} SNetGameData;

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

typedef struct SNetZtPlr {
	char npName[PLR_NAME_LEN];
	BYTE npClass;
	BYTE npLevel;
	BYTE npRank;
	BYTE npTeam;
} SNetZtPlr;

typedef struct SNetZtGame {
	char ngName[NET_MAX_GAMENAME_LEN + 1];
	SNetGameData ngData;
	SNetZtPlr ngPlayers[MAX_PLRS];
} SNetZtGame;
#pragma pack(pop)

typedef struct SNetEventHdr {
	unsigned eventid;
	unsigned playerid;
} SNetEventHdr;

typedef struct SNetJoinEvent {
	SNetEventHdr neHdr;
	const SNetGameData* neGameData;
	turn_t neTurn;
} SNetJoinEvent;

typedef struct SNetLeaveEvent {
	SNetEventHdr neHdr;
} SNetLeaveEvent;

typedef struct SNetPlrInfoEvent {
	SNetEventHdr neHdr;
	SNetZtPlr *nePlayers;
} SNetPlrInfoEvent;

typedef struct SNetTurnPkt {
	turn_t ntpTurn;
	unsigned ntpLen;
	BYTE data[32000]; // size does not matter, the struct is allocated dynamically
} SNetTurnPkt;

typedef struct SNetMsgPkt {
	int nmpPlr;
	unsigned nmpLen;
	BYTE data[32000]; // size does not matter, the struct is allocated dynamically
} SNetMsgPkt;

//////////////////////////////////////////////////
// diabloui
//////////////////////////////////////////////////

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
	BOOLEAN hiSaveFile;
	char hiName[PLR_NAME_LEN];
	int16_t hiStrength;
	int16_t hiMagic;
	int16_t hiDexterity;
	int16_t hiVitality;
} _uiheroinfo;

typedef struct _uigamedata {
	uint32_t aeVersionId;
	int32_t aeSeed;
	BYTE aeDifficulty;
	BYTE aeTickRate;
	BYTE aeNetUpdateRate; // (was defaultturnssec in vanilla)
	BYTE aeMaxPlayers;
	turn_t aeTurn;
	BYTE aePlayerId;
} _uigamedata;

//////////////////////////////////////////////////
// path
//////////////////////////////////////////////////

typedef struct PATHNODE {
	int totalCost;
	int remainingCost;
	int lastStepCost;
	int walkCost;
	int x;
	int y;
	struct PATHNODE* Parent;
	struct PATHNODE* Child[NUM_DIRS];
	struct PATHNODE* NextNode;
	ALIGNMENT64(6)
} PATHNODE;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_warning((sizeof(PATHNODE) & (sizeof(PATHNODE) - 1)) == 0, "Align PATHNODE closer to power of 2 for better performance.");
#endif

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

//////////////////////////////////////////////////
// trigs
//////////////////////////////////////////////////

typedef struct TriggerStruct {
	int _tx;
	int _ty;
	int _ttype; // dungeon_warp_type
	int _tlvl;  // dungeon_level
	int _tmsg;  // window_messages
} TriggerStruct;

//////////////////////////////////////////////////
// stores
//////////////////////////////////////////////////

typedef struct STextStruct {
	int _sx;         // starting position
	int _syoff;      // y-offset where the text should be printed
	union {
		char _sstr[112]; // the text
		struct {
			char _schr;     // placeholder to differentiate from a normal text
			const ItemStruct* _siItems[8];
		};
	};
	bool _sitemlist; // whether items should be drawn 
	bool _sjust;     // whether the string should be justified
	BYTE _sclr;      // the color of the string
	// bool _sline;
	bool _ssel;      // whether the line is selectable
	int _sval;       // integer value to be printed on the right side of the line
} STextStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_warning((sizeof(STextStruct) & (sizeof(STextStruct) - 1)) == 0, "Align STextStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// plrmsg
//////////////////////////////////////////////////

typedef struct _plrmsg {
	uint32_t time;
	BYTE player;
	BYTE lineBreak;
	char str[122];
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

typedef struct FileCl2Hdr {
	int32_t dlNumFrames;
	int32_t dlOffsets[dlNumFrames]; // address of an entry in dlCelFrames/dlgCelFrames
	int32_t dlFileSize/NextOffset;
} FileCl2Hdr;

typedef struct FileCl2 {
	FileCl2Hdr dlCl2Hdr;
	FileCelFrame dlCelFrames[dlNumFrames];
} FileCl2;

typedef struct FileCl2Group {
	int32_t dlgCelOffsets[dlgNumGroups]; // address of an entry in dlgCl2Data
	FileCl2Hdr dlgCl2Hdr[dlgNumGroups];
	FileCelFrame dlgCelFrames[dlgNumGroups * dlNumFrames];
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