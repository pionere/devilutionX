/**
 * @file structs.h
 *
 * Various global structures.
 */

DEVILUTION_BEGIN_NAMESPACE

#if INT_MAX == INT32_MAX && INTPTR_MAX == INT32_MAX
#define X86_32bit_COMP
#elif INT_MAX == INT32_MAX && INTPTR_MAX == INT64_MAX
#define X86_64bit_COMP
#endif
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

//////////////////////////////////////////////////
// items
//////////////////////////////////////////////////

typedef struct AffixData {
	const char* PLName;
	BYTE PLPower;
	int PLParam1;
	int PLParam2;
	BYTE PLMinLvl;
	int PLIType;
	BYTE PLGOE;
	BOOL PLDouble;
	BOOL PLOk;
	int PLMinVal;
	int PLMaxVal;
	int PLMultVal;
} AffixData;

typedef struct UniqItemData {
	const char* UIName;
	BYTE UIItemId;
	BYTE UIMinLvl;
	int UIValue;
	BYTE UIPower1;
	int UIParam1a;
	int UIParam1b;
	BYTE UIPower2;
	int UIParam2a;
	int UIParam2b;
	BYTE UIPower3;
	int UIParam3a;
	int UIParam3b;
	BYTE UIPower4;
	int UIParam4a;
	int UIParam4b;
	BYTE UIPower5;
	int UIParam5a;
	int UIParam5b;
	BYTE UIPower6;
	int UIParam6a;
	int UIParam6b;
#ifdef X86_32bit_COMP
	int alignment[3];
#elif defined(X86_64bit_COMP)
	int alignment[2];
#endif
} UniqItemData;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(UniqItemData) & (sizeof(UniqItemData) - 1)) == 64, "Align UniqItemData to power of 2 for better performance.");
#endif

typedef struct ItemFileData {
	const char* ifName; // Map of item type .cel file names.
	int idSFX;          // sounds effect of dropping the item on ground.
	int iiSFX;          // sounds effect of placing the item in the inventory.
	int iAnimLen;       // item drop animation length
#ifdef X86_64bit_COMP
	int alignment[2];
#endif
} ItemFileData;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(ItemFileData) & (sizeof(ItemFileData) - 1)) == 0, "Align ItemFileData to power of 2 for better performance.");
#endif

typedef struct ItemData {
	BYTE iRnd;
	BYTE iMinMLvl;
	BYTE iItemId; // unique_base_item
	const char* iName;
	int iCurs; // item_cursor_graphic
	int itype; // item_type
	int iMiscId; // item_misc_id
	int iSpell; // spell_id
	BYTE iClass; // item_class
	BYTE iLoc; // item_equip_type
	BYTE iDamType;
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
#ifdef X86_32bit_COMP
	int alignment[5];
#elif defined(X86_64bit_COMP)
	int alignment[2];
#endif
} ItemData;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(ItemData) & (sizeof(ItemData) - 1)) == 0, "Align ItemData to power of 2 for better performance.");
#endif

typedef struct ItemStruct {
	int _iSeed;
	WORD _iIdx;
	WORD _iCreateInfo;
	union {
		int _ix;
		int _iPHolder; // parent index of a placeholder entry in InvList
	};
	int _iy;
	int _iCurs;		// item_cursor_graphic
	int _itype;
	int _iMiscId;	// item_misc_id
	int _iSpell;	// spell_id
	BYTE _iClass;	// item_class enum
	BYTE _iLoc;		// item_equip_type
	BYTE _iDamType;
	BYTE _iMinDam;
	BYTE _iMaxDam;
	BYTE _iBaseCrit;
	BYTE _iMinStr;
	BYTE _iMinMag;
	BYTE _iMinDex;
	BOOL _iAnimFlag;
	BYTE* _iAnimData;        // PSX name -> ItemFrame
	unsigned _iAnimFrameLen; // Tick length of each frame in the current animation
	unsigned _iAnimCnt;      // Increases by one each game tick, counting how close we are to _iAnimFrameLen
	unsigned _iAnimLen;      // Number of frames in current animation
	unsigned _iAnimFrame;    // Current frame of animation.
	//int _iAnimWidth;
	//int _iAnimXOffset;
	BOOL _iPostDraw;
	BOOL _iIdentified;
	char _iName[32];
	BYTE _iPrePower;
	BYTE _iSufPower;
	BYTE _iSelFlag;
	BYTE _iMagical;	// item_quality
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
	char _iPLLight;
	char _iSplLvlAdd;
	BYTE _iManaSteal;
	BYTE _iLifeSteal;
	BYTE _iPLCrit;
	int _iUid;
	BYTE _iFMinDam;
	BYTE _iFMaxDam;
	BYTE _iLMinDam;
	BYTE _iLMaxDam;
	BYTE _iMMinDam;
	BYTE _iMMaxDam;
	BYTE _iAMinDam;
	BYTE _iAMaxDam;
	int _iVAdd;
	int _iVMult;
	BOOL _iStatFlag;
#ifdef X86_32bit_COMP
	int alignment[5];
#elif defined(X86_64bit_COMP)
	int alignment[4];
#endif
} ItemStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(ItemStruct) & (sizeof(ItemStruct) - 1)) == 0, "Align ItemStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// player
//////////////////////////////////////////////////

typedef struct PlayerStruct {
	int _pmode;
	char walkpath[MAX_PATH_LENGTH + 1];
	int destAction;
	int destParam1;
	int destParam2;
	int destParam3;
	int destParam4;
	BOOLEAN _pActive;
	BOOLEAN _pInvincible;
	BOOLEAN _pLvlChanging; // True when the player is transitioning between levels
	BYTE _pDunLevel;
	BYTE _pClass;
	BYTE _pLevel;
	BYTE _pRank;
	BYTE _pTeam;
	WORD _pStatPts;
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
	int _pxvel;   // Pixel X-velocity while walking. Indirectly applied to _pxoff via _pvar6
	int _pyvel;   // Pixel Y-velocity while walking. Indirectly applied to _pyoff via _pvar7
	int _pdir;    // Direction faced by player (direction enum)
	BYTE* _pAnimData;
	int _pAnimFrameLen; // Tick length of each frame in the current animation
	int _pAnimCnt;   // Increases by one each game tick, counting how close we are to _pAnimFrameLen
	unsigned _pAnimLen;   // Number of frames in current animation
	unsigned _pAnimFrame; // Current frame of animation.
	int _pAnimWidth;
	int _pAnimXOffset;
	unsigned _plid;
	unsigned _pvid;
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
	BYTE _pSkillLvl[64];
	BYTE _pSkillActivity[64];
	unsigned _pSkillExp[64];
	uint64_t _pMemSkills;  // Bitmask of learned skills
	uint64_t _pAblSkills;  // Bitmask of abilities
	uint64_t _pScrlSkills; // Bitmask of skills available via scrolls
	char _pName[PLR_NAME_LEN];
	WORD _pBaseStr;
	WORD _pBaseMag;
	WORD _pBaseDex;
	WORD _pBaseVit;
	int _pHPBase;    // the hp of the player if they would not wear an item
	int _pMaxHPBase; // the maximum hp of the player without items
	int _pManaBase;    // the mana of the player if they would not wear an item
	int _pMaxManaBase; // the maximum mana of the player without items
	int _pStrength;
	int _pMagic;
	int _pDexterity;
	int _pVitality;
	int _pHitPoints; // the current hp of the player
	int _pMaxHP;     // the maximum hp of the player
	int _pMana;        // the current mana of the player
	int _pMaxMana;     // the maximum mana of the player
	int _pVar1;
	int _pVar2;
	int _pVar3;
	int _pVar4;
	int _pVar5;
	int _pVar6;
	int _pVar7;
	int _pVar8;
	int _pGFXLoad;
	BYTE* _pNAnim[NUM_DIRS]; // Stand animations
	unsigned _pNFrames;
	int _pNWidth;
	BYTE* _pWAnim[NUM_DIRS]; // Walk animations
	unsigned _pWFrames;
	int _pWWidth;
	BYTE* _pAAnim[NUM_DIRS]; // Attack animations
	unsigned _pAFrames;
	int _pAWidth;
	unsigned _pAFNum;
	BYTE* _pLAnim[NUM_DIRS]; // Lightning spell cast animations
	BYTE* _pFAnim[NUM_DIRS]; // Fire spell cast animations
	BYTE* _pTAnim[NUM_DIRS]; // Generic spell cast animations
	unsigned _pSFrames;
	int _pSWidth;
	unsigned _pSFNum;
	BYTE* _pHAnim[NUM_DIRS]; // Getting hit animations
	unsigned _pHFrames;
	int _pHWidth;
	BYTE* _pDAnim[NUM_DIRS]; // Death animations
	unsigned _pDFrames;
	int _pDWidth;
	BYTE* _pBAnim[NUM_DIRS]; // Block animations
	unsigned _pBFrames;
	int _pBWidth;
	ItemStruct _pHoldItem;
	ItemStruct _pInvBody[NUM_INVLOC];
	ItemStruct _pSpdList[MAXBELTITEMS];
	ItemStruct _pInvList[NUM_INV_GRID_ELEM];
	int _pGold;
	BOOLEAN _pInfraFlag;
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
	char _pMagResist;
	char _pFireResist;
	char _pLghtResist;
	char _pAcidResist;
	int _pIHitChance;
	BYTE _pSkillFlags;    // Bitmask of allowed skill-types (SFLAG_*)
	BYTE _pIBaseHitBonus; // indicator whether the base BonusToHit of the items is positive/negative/neutral
	BYTE _pICritChance; // 200 == 100%
	BYTE _pIBlockChance;
	uint64_t _pISpells; // Bitmask of staff spell
	unsigned _pIFlags;
	unsigned _pIFlags2; // unused at the moment, but removing it causes inconsistency in case of X86_32bit_COMP...
	int _pIGetHit;
	char _pISplLvlAdd;
	char _pIArrowVelBonus; // _pISplCost in vanilla code
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
	BYTE* _pNData;
	BYTE* _pWData;
	BYTE* _pAData;
	BYTE* _pLData;
	BYTE* _pFData;
	BYTE* _pTData;
	BYTE* _pHData;
	BYTE* _pDData;
	BYTE* _pBData;
#ifdef X86_32bit_COMP
	int alignment[205];
#elif defined(X86_64bit_COMP)
	int alignment[118];
#endif
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
	int sfxnr;
} TextData;

//////////////////////////////////////////////////
// missiles
//////////////////////////////////////////////////

// TPDEF PTR FCN VOID MIADDPRC
// TPDEF PTR FCN VOID MIPROC

typedef struct MissileData {
	BYTE mName;
	int (*mAddProc)(int, int, int, int, int, int, char, int, int);
	void (*mProc)(int);
	BOOL mDraw;
	BYTE mType;
	BYTE mdFlags;
	BYTE mResist;
	BYTE mFileNum;
	int mlSFX;
	int miSFX;
	BYTE mlSFXCnt;
	BYTE miSFXCnt;
#ifdef X86_64bit_COMP
	int alignment[4];
#endif
} MissileData;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(MissileData) & (sizeof(MissileData) - 1)) == 0, "Align MissileData to power of 2 for better performance.");
#endif

typedef struct MisFileData {
	int mfAnimFAmt;
	const char* mfName;
	const char* mfAnimTrans;
	int mfFlags;
	BYTE mfAnimFrameLen[16];
	BYTE mfAnimLen[16];
	int mfAnimWidth;
	int mfAnimXOffset;
#ifdef X86_32bit_COMP
	int alignment[2];
#elif defined(X86_64bit_COMP)
	int alignment[14];
#endif
} MisFileData;
#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(MisFileData) & (sizeof(MisFileData) - 1)) == 0, "Align MisFileData to power of 2 for better performance.");
#endif

typedef struct MissileStruct {
	int _miType;   // Type of projectile (MIS_*)
	BYTE _miSubType; // Sub-Type of projectile 
	BYTE _miFlags;
	BYTE _miResist;
	BYTE _miAnimType;
	BOOL _miAnimFlag;
	BYTE* _miAnimData;
	int _miAnimFrameLen; // Tick length of each frame in the current animation
	int _miAnimLen;   // Number of frames in current animation
	int _miAnimWidth;
	int _miAnimXOffset;
	int _miAnimCnt; // Increases by one each game tick, counting how close we are to _miAnimFrameLen
	int _miAnimAdd;
	int _miAnimFrame; // Current frame of animation.
	BOOL _miDelFlag; // Indicate weather the missile should be deleted
	BOOL _miDrawFlag;
	BOOL _miLightFlag;
	BOOL _miPreFlag;
	int _miUniqTrans;
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
	int _miSource;
	int _miCaster;
	int _miMinDam;
	int _miMaxDam;
	int _miRndSeed;
	int _miRange; // Time to live for the missile in game ticks, when 0 the missile will be marked for deletion via _miDelFlag
	int _miDist; // Used for arrows to measure distance travelled (increases by 1 each game tick). Higher value is a penalty for accuracy calculation when hitting enemy
	unsigned _miLid;
	int _miVar1;
	int _miVar2;
	int _miVar3;
	int _miVar4;
	int _miVar5;
	int _miVar6;
	int _miVar7;
	int _miVar8;
#ifdef X86_32bit_COMP
	int alignment[4];
#elif defined(X86_64bit_COMP)
	int alignment[18];
#endif
} MissileStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(MissileStruct) & (sizeof(MissileStruct) - 1)) == 128, "Align MissileStruct closer to power of 2 for better performance.");
#elif defined(X86_64bit_COMP)
static_assert((sizeof(MissileStruct) & (sizeof(MissileStruct) - 1)) == 0, "Align MissileStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// effects/sound
//////////////////////////////////////////////////

typedef struct SoundSample final {
	Uint32 nextTc;
	struct Mix_Chunk* soundData;

	void Release();
	bool IsPlaying();
	bool IsLoaded() {
		return soundData != NULL;
	}
	void Play(int lVolume, int lPan, int channel = -1);
	void Stop();
	void SetChunk(BYTE* fileData, DWORD dwBytes);
	//int TrackLength();
} SoundSample;

typedef struct SFXStruct {
	BYTE bFlags;
	const char* pszName;
	union {
		struct {
			Uint32 sh11;
			void* sh01;
		};
		SoundSample pSnd;
	};
} SFXStruct;
//static_assert(sizeof(SoundSample) == sizeof(Uint32) + sizeof(void*), "SFXStruct initalization might fail.");

//////////////////////////////////////////////////
// monster
//////////////////////////////////////////////////

typedef struct AnimStruct {
	BYTE* aCelData;
	BYTE* aData[NUM_DIRS];
	int aFrames;
	int aFrameLen;
#ifdef X86_32bit_COMP
	int alignment[1];
#endif
} AnimStruct;
#ifdef X86_32bit_COMP
static_assert((sizeof(AnimStruct) & (sizeof(AnimStruct) - 1)) == 32, "Align AnimStruct closer to power of 2 for better performance.");
#elif defined(X86_64bit_COMP)
static_assert((sizeof(AnimStruct) & (sizeof(AnimStruct) - 1)) == 64, "Align AnimStruct closer to power of 2 for better performance.");
#endif

typedef struct MonsterData {
	int moFileNum;
	const char* mTransFile;
	const char* mName;
	BYTE mLevel;
	BYTE mSelFlag;
	BYTE mAi;
	BYTE mInt;
	int mMinHP;
	int mMaxHP;
	int mFlags;
	uint16_t mHit; // BUGFIX: Some monsters overflow this value on high difficulty (fixed)
	BYTE mMinDamage;
	BYTE mMaxDamage;
	uint16_t mHit2; // BUGFIX: Some monsters overflow this value on high difficulty (fixed)
	BYTE mMinDamage2;
	BYTE mMaxDamage2;
	BYTE mMagic;
	BYTE mMagic2;     // unused
	BYTE mArmorClass; // AC+evasion: used against physical-hit (melee+projectile)
	BYTE mEvasion;    // evasion: used against magic-projectile
	BYTE mMonstClass; // unused
	uint16_t mMagicRes;
	uint16_t mMagicRes2;
	uint16_t mTreasure;
	uint16_t mExp;
#ifdef X86_32bit_COMP
	int alignment[3];
#endif
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

typedef struct MapMonData {
	int cmType;
	BOOL cmPlaceScatter;
	AnimStruct cmAnims[NUM_MON_ANIM];
	SoundSample cmSnds[NUM_MON_SFX][2];
	int cmWidth;
	int cmXOffset;
	BYTE cmDeadval;
	BOOLEAN cmSndSpecial;
	BYTE cmAFNum;
	BYTE cmAFNum2;
	const MonsterData* cmData;
#ifdef X86_32bit_COMP
	int alignment[2];
#endif
} MapMonData;
#ifdef X86_32bit_COMP
static_assert((sizeof(MapMonData) & (sizeof(MapMonData) - 1)) == 256, "Align MapMonData closer to power of 2 for better performance.");
#elif defined(X86_64bit_COMP)
static_assert((sizeof(MapMonData) & (sizeof(MapMonData) - 1)) == 512, "Align MapMonData closer to power of 2 for better performance.");
#endif

typedef struct MonsterStruct { // note: missing field _mAFNum
	int _mmode; /* MON_MODE */
	unsigned _msquelch;
	BYTE _mMTidx;
	BYTE _mpathcount;
	BYTE _mWhoHit;
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
	int _mxvel;             // Pixel X-velocity while walking. Applied to _mxoff
	int _myvel;             // Pixel Y-velocity while walking. Applied to _myoff
	int _mdir;              // Direction faced by monster (direction enum)
	int _menemy;            // The current target of the monster. An index in to either the plr or monster array based on the _meflag value.
	BYTE _menemyx;          // X-coordinate of enemy (usually correspond's to the enemy's futx value)
	BYTE _menemyy;          // Y-coordinate of enemy (usually correspond's to the enemy's futy value)
	BYTE _mListener;        // the player to whom the monster is talking to
	BYTE* _mAnimData;
	int _mAnimFrameLen; // Tick length of each frame in the current animation
	int _mAnimCnt;   // Increases by one each game tick, counting how close we are to _mAnimFrameLen
	int _mAnimLen;   // Number of frames in current animation
	int _mAnimFrame; // Current frame of animation.
	BOOL _mDelFlag;
	int _mVar1;
	int _mVar2;
	int _mVar3;
	int _mVar4;
	int _mVar5;
	int _mVar6; // Used as _mxoff but with a higher range so that we can correctly apply velocities of a smaller number
	int _mVar7; // Used as _myoff but with a higher range so that we can correctly apply velocities of a smaller number
	int _mVar8; // Value used to measure progress for moving from one tile to another
	int _mmaxhp;
	int _mhitpoints;
	int _lastx; // the last known X-coordinate of the enemy
	int _lasty; // the last known Y-coordinate of the enemy
	int _mRndSeed;
	int _mAISeed;
	BYTE _uniqtype;
	BYTE _uniqtrans;
	BYTE _udeadval;
	BYTE mlid;
	BYTE leader; // the leader of the monster
	BYTE leaderflag; // the status of the monster's leader
	BYTE packsize; // the number of 'pack'-monsters close to their leader
	BYTE falign_CB;
	BYTE _mLevel;
	BYTE _mSelFlag;
	BYTE _mAi;
	BYTE _mInt;
	int _mFlags;
	uint16_t _mHit; // BUGFIX: Some monsters overflow this value on high difficulty (fixed)
	BYTE _mMinDamage;
	BYTE _mMaxDamage;
	uint16_t _mHit2; // BUGFIX: Some monsters overflow this value on high difficulty (fixed)
	BYTE _mMinDamage2;
	BYTE _mMaxDamage2;
	BYTE _mMagic;
	BYTE _mMagic2;     // unused
	BYTE _mArmorClass; // AC+evasion: used against physical-hit (melee+projectile)
	BYTE _mEvasion;    // evasion: used against magic-projectile
	BYTE _mAFNum;
	BYTE _mAFNum2;
	uint16_t _mMagicRes;
	uint16_t _mTreasure;
	uint16_t _mExp;
	int mtalkmsg;
	const char* mName;
	MapMonData* MType;
	int _mType;
	AnimStruct* _mAnims;
	int _mAnimWidth;
	int _mAnimXOffset;
#ifdef X86_32bit_COMP
	int alignment[9];
#elif defined(X86_64bit_COMP)
	int alignment[2];
#endif
} MonsterStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(MonsterStruct) & (sizeof(MonsterStruct) - 1)) == 0, "Align MonsterStruct to power of 2 for better performance.");
#endif

typedef struct UniqMonData {
	int mtype;
	const char* mName;
	const char* mTrnName;
	BYTE muLevelIdx; // level-index to place the monster
	BYTE muLevel;    // difficulty level of the monster
	uint16_t mmaxhp;
	BYTE mAi;
	BYTE mInt;
	BYTE mMinDamage;
	BYTE mMaxDamage;
	BYTE mMinDamage2;
	BYTE mMaxDamage2;
	uint16_t mMagicRes;
	BYTE mUnqAttr;
	BYTE mUnqHit;
	BYTE mUnqAC;
	BYTE mQuestId;
	int mtalkmsg;
#ifdef X86_64bit_COMP
	int alignment[4];
#endif
} UniqMonData;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(UniqMonData) & (sizeof(UniqMonData) - 1)) == 0, "Align UniqMonData to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// objects
//////////////////////////////////////////////////

typedef struct ObjectData {
	BYTE ofindex;
	BYTE oLvlTypes;
	BYTE oSetLvlType;
	BYTE otheme;
	BYTE oquest;
	//BOOLEAN oAnimFlag;
	BYTE oAnimBaseFrame; // The starting/base frame of (initially) non-animated objects
	//int oAnimFrameLen; // Tick length of each frame in the current animation
	//int oAnimLen;   // Number of frames in current animation
	//int oAnimWidth;
	//int oSFX;
	//BYTE oSFXCnt;
	//BOOL oSolidFlag;
	//BOOL oMissFlag;
	//BOOL oLightFlag;
	//BYTE oBreak;
	BYTE oDoorFlag;
	BYTE oSelFlag;
	BOOLEAN oTrapFlag;
#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
	int alignment[1];
#endif
} ObjectData;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(ObjectData) & (sizeof(ObjectData) - 1)) == 0, "Align ObjectData closer to power of 2 for better performance.");
#endif

typedef struct ObjFileData {
	const char* ofName;
	int oSFX;
	BYTE oSFXCnt;
	BOOLEAN oAnimFlag;
	int oAnimFrameLen; // Tick length of each frame in the current animation
	int oAnimLen;   // Number of frames in current animation
	int oAnimWidth;
	BOOLEAN oSolidFlag;
	BOOLEAN oMissFlag;
	BOOLEAN oLightFlag;
	BYTE oBreak;
#ifdef X86_32bit_COMP
	int alignment[1];
#endif
} ObjFileData;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(ObjFileData) & (sizeof(ObjFileData) - 1)) == 0, "Align ObjFileData closer to power of 2 for better performance.");
#endif

typedef struct ObjectStruct {
	int _otype;
	int _ox;
	int _oy;
	int _oSFX;
	BYTE _oSFXCnt;
	BOOLEAN _oAnimFlag;
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
	BOOLEAN _oLightFlag;
	BYTE _oBreak; // object_break_mode
	BYTE _oDoorFlag; // object_door_type
	BYTE _oSelFlag; // check
	BOOLEAN _oTrapFlag;
	BOOL _oPreFlag;
	unsigned _olid;
	int _oRndSeed;
	int _oVar1;
	int _oVar2;
	int _oVar3;
	int _oVar4;
	int _oVar5;
	int _oVar6;
	int _oVar7;
	int _oVar8;
#ifdef X86_32bit_COMP
	int alignment[7];
#elif defined(X86_64bit_COMP)
	int alignment[4];
#endif
} ObjectStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(ObjectStruct) & (sizeof(ObjectStruct) - 1)) == 0, "Align ObjectStruct to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// portal
//////////////////////////////////////////////////

typedef struct PortalStruct {
	BOOLEAN _wopen;
	int x;
	int y;
	int level;
} PortalStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(PortalStruct) & (sizeof(PortalStruct) - 1)) == 0, "Align PortalStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// pack
//////////////////////////////////////////////////

#pragma pack(push, 1)
typedef struct PkItemStruct {
	INT dwSeed;
	WORD wIndx;
	WORD wCI;
	BYTE bId;
	BYTE bDur;
	BYTE bMDur;
	BYTE bCh;
	BYTE bMCh;
	WORD wValue;
	DWORD dwBuff;
} PkItemStruct;

typedef struct PkPlayerStruct {
	BYTE px;
	BYTE py;
	char pName[PLR_NAME_LEN];
	BOOLEAN pLvlChanging;
	BYTE pDunLevel;
	BYTE pClass;
	BYTE pLevel;
	BYTE pRank;
	BYTE pTeam;
	WORD pStatPts;
	BYTE pLightRad;
	BYTE pManaShield;
	WORD pTimer[NUM_PLRTIMERS];
	DWORD pExperience;
	WORD pBaseStr;
	WORD pBaseMag;
	WORD pBaseDex;
	WORD pBaseVit;
	INT pHPBase;
	INT pMaxHPBase;
	INT pManaBase;
	INT pMaxManaBase;
	BYTE pAtkSkillHotKey[4];         // the attack skill selected by the hotkey
	BYTE pAtkSkillTypeHotKey[4];     // the (RSPLTYPE_)type of the attack skill selected by the hotkey
	BYTE pMoveSkillHotKey[4];        // the movement skill selected by the hotkey
	BYTE pMoveSkillTypeHotKey[4];    // the (RSPLTYPE_)type of the movement skill selected by the hotkey
	BYTE pAltAtkSkillHotKey[4];      // the attack skill selected by the alt-hotkey
	BYTE pAltAtkSkillTypeHotKey[4];  // the (RSPLTYPE_)type of the attack skill selected by the alt-hotkey
	BYTE pAltMoveSkillHotKey[4];     // the movement skill selected by the alt-hotkey
	BYTE pAltMoveSkillTypeHotKey[4]; // the (RSPLTYPE_)type of the movement skill selected by the alt-hotkey
	BYTE pSkillLvl[64];
	BYTE pSkillActivity[64];
	DWORD pSkillExp[64];
	uint64_t pMemSkills;
	PkItemStruct pHoldItem;
	PkItemStruct pInvBody[NUM_INVLOC];
	PkItemStruct pSpdList[MAXBELTITEMS];
	PkItemStruct pInvList[NUM_INV_GRID_ELEM];
	char pInvGrid[NUM_INV_GRID_ELEM];
	INT pNumInv;
} PkPlayerStruct;
#pragma pack(pop)

//////////////////////////////////////////////////
// msg
//////////////////////////////////////////////////

#pragma pack(push, 1)
typedef struct CmdSkillUse {
	BYTE skill;
	char from;
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
	WORD wParam1;
} TCmdLocParam1;

typedef struct TCmdParam1 {
	BYTE bCmd;
	WORD wParam1;
} TCmdParam1;

typedef struct TCmdParam2 {
	BYTE bCmd;
	WORD wParam1;
	WORD wParam2;
} TCmdParam2;

typedef struct TCmdParam3 {
	BYTE bCmd;
	WORD wParam1;
	WORD wParam2;
	WORD wParam3;
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

typedef struct TCmdLocAttack {
	BYTE bCmd;
	BYTE x;
	BYTE y;
	CmdSkillUse lau;
} TCmdLocAttack;

typedef struct TCmdLocSkill {
	BYTE bCmd;
	BYTE x;
	BYTE y;
	CmdSkillUse lsu;
} TCmdLocSkill;

typedef struct TCmdPlrAttack {
	BYTE bCmd;
	BYTE paPnum;
	CmdSkillUse pau;
} TCmdPlrAttack;

typedef struct TCmdPlrSkill {
	BYTE bCmd;
	BYTE psPnum;
	CmdSkillUse psu;
} TCmdPlrSkill;

typedef struct TCmdMonstAttack {
	BYTE bCmd;
	WORD maMnum;
	CmdSkillUse mau;
} TCmdMonstAttack;

typedef struct TCmdMonstSkill {
	BYTE bCmd;
	WORD msMnum;
	CmdSkillUse msu;
} TCmdMonstSkill;

typedef struct TCmdMonstDamage {
	BYTE bCmd;
	BYTE mdLevel;
	WORD mdMnum;
	INT mdHitpoints;
} TCmdMonstDamage;

typedef struct TCmdMonstKill {
	BYTE bCmd;
	BYTE mkPnum;
	WORD mkMnum;
	WORD mkExp;
	BYTE mkMonLevel;
	BYTE mkX;
	BYTE mkY;
	BYTE mkDir;
	BYTE mkLevel;
} TCmdMonstKill;

typedef struct TCmdGolem {
	BYTE bCmd;
	BYTE goMonLevel;
	BYTE goX;
	BYTE goY;
	BYTE goEnemy;
	BYTE goDunLevel;
} TCmdGolem;

typedef struct TCmdShrine {
	BYTE bCmd;
	BYTE shType;
	INT shSeed;
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
	PkItemStruct item;
} TCmdRPItem;

typedef struct TCmdStore1 {
	BYTE bCmd;
	BYTE stCmd;
	BYTE stLoc;
	INT stValue;
} TCmdStore1;

typedef struct TCmdStore2 {
	BYTE bCmd;
	BYTE stCmd;
	PkItemStruct item;
	INT stValue;
} TCmdStore2;

typedef struct TCmdPlrInfoHdr {
	BYTE bCmd;
	WORD wOffset;
	WORD wBytes;
} TCmdPlrInfoHdr;

typedef struct TCmdString {
	BYTE bCmd;
	char str[MAX_SEND_STR_LEN];
} TCmdString;

typedef struct TFakeDropPlr {
	BYTE bCmd;
	DWORD dwReason;
} TFakeDropPlr;

typedef struct TSyncHeader {
	BYTE bCmd;
	BYTE bLevel;
	WORD wLen;
	/*BYTE bObjId;
	BYTE bObjCmd;
	BYTE bItemI;
	BYTE bItemX;
	BYTE bItemY;
	WORD wItemIndx;
	WORD wItemCI;
	DWORD dwItemSeed;
	BYTE bItemId;
	BYTE bItemDur;
	BYTE bItemMDur;
	BYTE bItemCh;
	BYTE bItemMCh;
	WORD wItemVal;
	DWORD dwItemBuff;
	BYTE bPInvLoc;
	WORD wPInvIndx;
	WORD wPInvCI;
	DWORD dwPInvSeed;
	BYTE bPInvId;
#ifdef HELLFIRE
	WORD wToHit;
	WORD wMaxDam;
	BYTE bMinStr;
	BYTE bMinMag;
	BYTE bMinDex;
	BYTE bAC;
#endif*/
} TSyncHeader;

typedef struct TSyncMonster {
	BYTE _mndx;
	BYTE _mx;
	BYTE _my;
	BYTE _mdir;
	BYTE _menemy;
	DWORD _mactive;
	INT	_mhitpoints;
} TSyncMonster;

typedef struct TurnPktHdr {
	INT php;
	//INT pmhp;
	INT pmp;
	//INT pmmp;
	BYTE px;
	BYTE py;
	WORD wCheck;
	WORD wLen;
} TurnPktHdr;

typedef struct TurnPkt {
	TurnPktHdr hdr;
	BYTE body[MAX_NETMSG_SIZE - sizeof(TurnPktHdr)];
} TurnPkt;

typedef struct MsgPktHdr {
	WORD wCheck;
	WORD wLen;
} MsgPktHdr;

typedef struct MsgPkt {
	MsgPktHdr hdr;
	BYTE body[MAX_NETMSG_SIZE - sizeof(MsgPktHdr)];
} MsgPkt;

typedef struct DMonsterStr {
	BYTE _mCmd;
	BYTE _mx;
	BYTE _my;
	BYTE _mdir;
	BYTE _menemy;
	DWORD _mactive;
	INT _mhitpoints;
	BYTE _mWhoHit;
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

typedef struct DBuffer {
	BOOLEAN compressed;
	BYTE content[sizeof(DLevel) + 1];
} DBuffer;

typedef struct DeltaData {
	union {
		struct {
			DJunk ddJunk;					// portals and quests
			DLevel ddLevel[NUM_LEVELS];		// items/monsters/objects
			LocalLevel ddLocal[NUM_LEVELS];	// automap
			DBuffer ddSendRecvBuf;			// Buffer to send/receive delta info
			unsigned ddSendRecvOffset;		// offset in the buffer
			bool ddLevelChanged[NUM_LEVELS];
			bool ddJunkChanged;
		};
		BYTE ddBuffer[FILEBUFF];
	};
} DeltaData;

typedef struct TCmdSendJoinLevel {
	BYTE bCmd;
	BYTE lLevel;
	BYTE px;
	BYTE py;
	WORD lTimer1;
	WORD lTimer2;
} TCmdSendJoinLevel;

typedef struct TCmdAckJoinLevel {
	BYTE bCmd;
	BYTE lManashield;
	WORD lTimer1;
	WORD lTimer2;
} TCmdAckJoinLevel;
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
	const char* dSetLvlPreDun;
	const char* dSetLvlDun;
	BYTE dSetLvlDunX;
	BYTE dSetLvlDunY;
	BYTE dMonTypes[32];
#ifdef X86_32bit_COMP
	int alignment[7];
#endif
} LevelData;

#ifdef X86_32bit_COMP
static_assert((sizeof(LevelData) & (sizeof(LevelData) - 1)) == 0, "Align LevelData to power of 2 for better performance.");
#elif defined(X86_64bit_COMP)
static_assert((sizeof(LevelData) & (sizeof(LevelData) - 1)) == 128, "Align LevelData to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// quests
//////////////////////////////////////////////////

typedef struct QuestStruct {
	BYTE _qactive;
	BYTE _qvar1;
	BYTE _qvar2;
	BOOLEAN _qlog;
	unsigned _qmsg;
	int _qtx;
	int _qty;
} QuestStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(QuestStruct) & (sizeof(QuestStruct) - 1)) == 0, "Align QuestStruct to power of 2 for better performance.");
#endif

typedef struct QuestData {
	BYTE _qdlvl;
	BYTE _qslvl;
	int _qdmsg;
	const char* _qlstr;
} QuestData;

//////////////////////////////////////////////////
// gamemenu/gmenu
//////////////////////////////////////////////////

// TPDEF PTR FCN VOID TMenuFcn

typedef struct TMenuItem {
	const char* pszStr;
	void (*fnMenu)(bool); /* fix, should have one arg */
	uint32_t dwFlags;
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
	BYTE sName;
	BYTE sManaCost;
	BYTE sType;
	BYTE sIcon;
	const char* sNameText;
	char sBookLvl;
	char sStaffLvl;
	char sScrollLvl;
	BOOLEAN sTargeted;
	BYTE scCurs; // cursor for scrolls/runes
	BYTE spCurs; // cursor for spells
	BYTE sFlags; // the required flags(SFLAG*) to use the skill
	BYTE sMinInt;
	BYTE sSFX;
	BYTE sMissile;
	BYTE sManaAdj;
	BYTE sMinMana;
	WORD sStaffMin;
	WORD sStaffMax;
	int sBookCost;
	int sStaffCost; // == sScrollCost
#ifdef X86_64bit_COMP
	int alignment[6];
#endif
} SpellData;
#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(SpellData) & (sizeof(SpellData) - 1)) == 0, "Align SpellData to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// towners
//////////////////////////////////////////////////

typedef struct TownerStruct {
	int _ttype;
	int _tx;    // Tile X-position of NPC
	int _ty;    // Tile Y-position of NPC
	int _txoff; // Sprite X-offset (unused)
	int _tyoff; // Sprite Y-offset (unused)
	int _txvel; // X-velocity during movement (unused)
	int _tyvel; // Y-velocity during movement (unused)
	int _tdir;  // Facing of NPC (unused)
	BYTE* _tAnimData;
	int _tAnimFrameLen; // Tick length of each frame in the current animation
	int _tAnimCnt;   // Increases by one each game tick, counting how close we are to _tAnimFrameLen
	int _tAnimLen;   // Number of frames in current animation
	int _tAnimFrame; // Current frame of animation.
	int _tAnimFrameCnt;
	int _tAnimWidth;
	int _tAnimXOffset;
	int _tAnimOrder; // char would suffice
	//BYTE _tListener; // unused
	int _tStoreId; // BYTE would suffice
	int _tStoreTalk;
	int _tGossipStart;
	int _tGossipEnd;
	//BOOL _tSelFlag; // unused
	int _tSeed;
	const char* _tName;
#ifdef X86_32bit_COMP
	int alignment[9];
#elif defined(X86_64bit_COMP)
	int alignment[6];
#endif
} TownerStruct;
#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(TownerStruct) & (sizeof(TownerStruct) - 1)) == 0, "Align TownerStruct to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// scrollrt
//////////////////////////////////////////////////

typedef struct ScrollStruct {
	int _sxoff; // X-offset of camera position. This usually corresponds to a negative version of plr[myplr]._pxoff
	int _syoff; // Y-offset of camera position. This usually corresponds to a negative version of plr[myplr]._pyoff
	int _sdx;
	int _sdy;
	int _sdir;
} ScrollStruct;

typedef struct ViewportStruct {
	int _vColumns; // number of tiles in one row
	int _vRows;    // number of tiles in one column
	int _vOffsetX; // X-offset in a back buffer
	int _vOffsetY; // Y-offset in a back buffer
	int _vShiftX; // X-shift in a dPiece
	int _vShiftY; // Y-shift in a dPiece
} ViewportStruct;

//////////////////////////////////////////////////
// gendung
//////////////////////////////////////////////////

typedef struct THEME_LOC {
	int x;
	int y;
	BYTE ttval;
	int width;
	int height;
} THEME_LOC;

typedef struct MICROS {
	uint16_t mt[16];
} MICROS;

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

//////////////////////////////////////////////////
// themes
//////////////////////////////////////////////////

typedef struct ThemeStruct {
	BYTE ttype;
	BYTE ttval;
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
	BOOLEAN _ldel;
	BOOLEAN _lunflag;
	BOOLEAN _lmine;
	int _xoff;
	int _yoff;
} LightListStruct;

#if defined(X86_32bit_COMP) || defined(X86_64bit_COMP)
static_assert((sizeof(LightListStruct) & (sizeof(LightListStruct) - 1)) == 0, "Align LightListStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// dead
//////////////////////////////////////////////////

typedef struct DeadStruct {
	BYTE* _deadData[NUM_DIRS];
	int _deadFrame;
	int _deadWidth;
	int _deadXOffset;
	BYTE _deadtrans;
#ifdef X86_64bit_COMP
	int alignment[12];
#endif
} DeadStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(DeadStruct) & (sizeof(DeadStruct) - 1)) == 32, "Align DeadStruct closer to power of 2 for better performance.");
#elif defined(X86_64bit_COMP)
static_assert((sizeof(DeadStruct) & (sizeof(DeadStruct) - 1)) == 0, "Align DeadStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// diabloui
//////////////////////////////////////////////////

// TPDEF PTR FCN VOID PLAYSND

typedef struct _uidefaultstats {
	WORD dsStrength;
	WORD dsMagic;
	WORD dsDexterity;
	WORD dsVitality;
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

//////////////////////////////////////////////////
// storm-net
//////////////////////////////////////////////////

#pragma pack(push, 1)
typedef struct SNetGameData {
	INT dwSeed;
	DWORD dwVersionId;
	BYTE bPlayerId; // internal-only!
	BYTE bDifficulty;
	BYTE bTickRate;
	BYTE bNetUpdateRate; // (was defaultturnssec in vanilla)
	BYTE bMaxPlayers;
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
#ifdef X86_32bit_COMP
	int alignment[3];
#elif defined(X86_64bit_COMP)
	int alignment[8];
#endif
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
	DWORD count[2];
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

typedef struct _FILEHEADER {
	uint32_t signature;
	uint32_t headersize;
	uint32_t filesize;
	WORD version;
	WORD sectorsizeid;
	uint32_t hashoffset;
	uint32_t blockoffset;
	uint32_t hashcount;
	uint32_t blockcount;
	char pad[72];
} _FILEHEADER;

typedef struct _HASHENTRY {
	uint32_t hashcheck[2];
	uint32_t lcid;
	uint32_t block;
} _HASHENTRY;

typedef struct _BLOCKENTRY {
	uint32_t offset;
	uint32_t sizealloc;
	uint32_t sizefile;
	uint32_t flags;
} _BLOCKENTRY;

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
	char str[144];
} _plrmsg;

//////////////////////////////////////////////////
// capture
//////////////////////////////////////////////////

typedef struct _PcxHeader {
	BYTE Manufacturer;
	BYTE Version;
	BYTE Encoding;
	BYTE BitsPerPixel;
	WORD Xmin;
	WORD Ymin;
	WORD Xmax;
	WORD Ymax;
	WORD HDpi;
	WORD VDpi;
	BYTE Colormap[48];
	BYTE Reserved;
	BYTE NPlanes;
	WORD BytesPerLine;
	WORD PaletteInfo;
	WORD HscreenSize;
	WORD VscreenSize;
	BYTE Filler[54];
} PCXHEADER;

//////////////////////////////////////////////////
// encrypt
//////////////////////////////////////////////////

typedef struct TDataInfo {
	BYTE* srcData;
	DWORD srcOffset;
	BYTE* destData;
	DWORD destOffset;
	DWORD size;
} TDataInfo;

DEVILUTION_END_NAMESPACE
