/**
 * @file structs.h
 *
 * Various global structures.
 */

DEVILUTION_BEGIN_NAMESPACE

#if INT_MAX == INT32_MAX && INTPTR_MAX == INT32_MAX
#define X86_32bit_COMP
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

typedef struct PLStruct {
	const char *PLName;
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
} PLStruct;

typedef struct UItemStruct {
	const char *UIName;
	char UIItemId;
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
#endif
} UItemStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(UItemStruct) & (sizeof(UItemStruct) - 1)) == 64, "Align UItemStruct to power of 2 for better performance.");
#endif

typedef struct ItemDataStruct {
	int iRnd;
	BYTE iClass;
	BYTE iLoc;
	int iCurs;
	char itype;
	char iItemId;
	const char *iName;
	char iMinMLvl;
	int16_t iDurability;
	BYTE iDamType;
	BYTE iMinDam;
	BYTE iMaxDam;
	int iMinAC;
	int iMaxAC;
	BYTE iMinStr;
	BYTE iMinMag;
	BYTE iMinDex;
	// item_misc_id
	int iMiscId;
	// spell_id
	int iSpell;
	BOOLEAN iUsable;
	int iValue;
#ifdef X86_32bit_COMP
	int alignment[2];
#endif
} ItemDataStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(ItemDataStruct) & (sizeof(ItemDataStruct) - 1)) == 0, "Align ItemDataStruct to power of 2 for better performance.");
#endif

typedef struct ItemGetRecordStruct {
	INT nSeed;
	WORD nIndex;
	WORD wCI;
	unsigned int dwTimestamp;
#ifdef X86_32bit_COMP
	int alignment[1];
#endif
} ItemGetRecordStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(ItemGetRecordStruct) & (sizeof(ItemGetRecordStruct) - 1)) == 0, "Align ItemGetRecordStruct to power of 2 for better performance.");
#endif

typedef struct ItemStruct {
	int _iSeed;
	WORD _iIdx;
	WORD _iCreateInfo;
	int _itype;
	int _ix;
	int _iy;
	BOOL _iAnimFlag;
	BYTE *_iAnimData;        // PSX name -> ItemFrame
	unsigned _iAnimFrameLen; // Tick length of each frame in the current animation
	unsigned _iAnimCnt;      // Increases by one each game tick, counting how close we are to _iAnimFrameLen
	unsigned _iAnimLen;      // Number of frames in current animation
	unsigned _iAnimFrame;    // Current frame of animation.
	//int _iAnimWidth;
	//int _iAnimXOffset;
	BOOL _iPostDraw;
	BOOL _iIdentified;
	char _iName[32];
	BYTE _iSelFlag;
	BYTE _iMagical;	// item_quality
	BYTE _iLoc;		// item_equip_type
	BYTE _iClass;	// item_class enum
	int _iCurs;		// item_cursor_graphic
	int _ivalue;
	int _iIvalue;
	BYTE _iMinDam;
	BYTE _iMaxDam;
	int _iAC;
	int _iFlags;	// item_special_effect
	int _iMiscId;	// item_misc_id
	int _iSpell;	// spell_id
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
	BYTE _iDamType;
	char _iSplLvlAdd;
	BOOLEAN _iRequest;
	BYTE _iManaSteal;
	BYTE _iLifeSteal;
	int _iUid;
	BYTE _iFMinDam;
	BYTE _iFMaxDam;
	BYTE _iLMinDam;
	BYTE _iLMaxDam;
	BYTE _iMMinDam;
	BYTE _iMMaxDam;
	BYTE _iAMinDam;
	BYTE _iAMaxDam;
	int _iPLEnAc;
	BYTE _iPrePower;
	BYTE _iSufPower;
	int _iVAdd;
	int _iVMult;
	BYTE _iMinStr;
	BYTE _iMinMag;
	BYTE _iMinDex;
	BOOL _iStatFlag;
#ifdef X86_32bit_COMP
	int alignment[4];
#endif
} ItemStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(ItemStruct) & (sizeof(ItemStruct) - 1)) == 0, "Align ItemStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// player
//////////////////////////////////////////////////

typedef struct PlayerStruct {
	int _pmode;
	char walkpath[MAX_PATH_LENGTH + 1];
	int destAction;
	union {
		int destParam1;
		struct {
			BYTE destParam1a;
			BYTE destParam1b;
			BYTE destParam1c;
			BYTE destParam1d;
		};
	};
	int destParam2;
	int destParam3;
	int destParam4;
	BOOLEAN plractive;
	BOOLEAN _pLvlChanging; // True when the player is transitioning between levels
	BYTE plrlevel;
	BYTE _pTeam;
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
	int _pgfxnum; // Bitmask indicating what variant of the sprite the player is using. Lower byte define weapon (anim_weapon_id) and higher values define armour (starting with anim_armor_id)
	BYTE *_pAnimData;
	int _pAnimFrameLen; // Tick length of each frame in the current animation
	int _pAnimCnt;   // Increases by one each game tick, counting how close we are to _pAnimFrameLen
	unsigned _pAnimLen;   // Number of frames in current animation
	unsigned _pAnimFrame; // Current frame of animation.
	int _pAnimWidth;
	int _pAnimXOffset;
	int _plid;
	int _pvid;
	BYTE _pAtkSkill;         // the selected attack skill for the primary action
	BYTE _pAtkSkillType;     // the (RSPLTYPE_)type of the attack skill for the primary action
	BYTE _pMoveSkill;        // the selected movement skill for the primary action
	BYTE _pMoveSkillType;    // the (RSPLTYPE_)type of the movement skill for the primary action
	BYTE _pAltAtkSkill;      // the selected attack skill for the secondary action
	BYTE _pAltAtkSkillType;  // the (RSPLTYPE_)type of the attack skill for the secondary action
	BYTE _pAltMoveSkill;     // the selected movement skill for the secondary action
	BYTE _pAltMoveSkillType; // the (RSPLTYPE_)type of the movement skill for the secondary action
	BYTE _pTSpell;   // the spell to cast after the target is selected
	char _pTSplFrom; // the source of the spell after the target is selected
	char _pOilFrom;
	BYTE _pSkillLvl[64];
	BYTE _pSkillActivity[64];
	unsigned _pSkillExp[64];
	uint64_t _pMemSkills;  // Bitmask of learned skills
	uint64_t _pAblSkills;  // Bitmask of abilities
	uint64_t _pScrlSkills; // Bitmask of skills available via scrolls
	BYTE _pAtkSkillHotKey[4];         // the attack skill selected by the hotkey
	BYTE _pAtkSkillTypeHotKey[4];     // the (RSPLTYPE_)type of the attack skill selected by the hotkey
	BYTE _pMoveSkillHotKey[4];        // the movement skill selected by the hotkey
	BYTE _pMoveSkillTypeHotKey[4];    // the (RSPLTYPE_)type of the movement skill selected by the hotkey
	BYTE _pAltAtkSkillHotKey[4];      // the attack skill selected by the alt-hotkey
	BYTE _pAltAtkSkillTypeHotKey[4];  // the (RSPLTYPE_)type of the attack skill selected by the alt-hotkey
	BYTE _pAltMoveSkillHotKey[4];     // the movement skill selected by the alt-hotkey
	BYTE _pAltMoveSkillTypeHotKey[4]; // the (RSPLTYPE_)type of the movement skill selected by the alt-hotkey
	BYTE _pSkillFlags;     // Bitmask of allowed skill-types (SFLAG_*)
	BOOLEAN _pInvincible;
	int16_t _pTimer[NUM_PLRTIMERS];
	char _pName[PLR_NAME_LEN];
	// plr_class enum value.
	// TODO: this could very well be `enum plr_class _pClass`
	// since there are 3 bytes of alingment after this field.
	// it could just be that the compiler optimized away all accesses to
	// the higher bytes by using byte instructions, since all possible values
	// of plr_class fit into one byte.
	BYTE _pClass;
	BYTE _pLevel;
	BYTE _pLightRad;
	BYTE _pManaShield;
	WORD _pBaseStr;
	WORD _pBaseMag;
	WORD _pBaseDex;
	WORD _pBaseVit;
	int _pStrength;
	int _pMagic;
	int _pDexterity;
	int _pVitality;
	int _pHPBase;
	int _pMaxHPBase;
	int _pHitPoints;
	int _pMaxHP;
	int _pHPPer;
	int _pManaBase;
	int _pMaxManaBase;
	int _pMana;
	int _pMaxMana;
	int _pManaPer;
	WORD _pStatPts;
	BOOLEAN _pLvlUp;
	BYTE _pDiabloKillLevel;
	unsigned _pExperience;
	unsigned _pNextExper;
	char _pMagResist;
	char _pFireResist;
	char _pLghtResist;
	char _pAcidResist;
	int _pGold;
	BOOL _pInfraFlag;
	int _pVar1;
	int _pVar2;
	int _pVar3;
	int _pVar4;
	int _pVar5;
	int _pVar6;
	int _pVar7;
	int _pVar8;
	BOOLEAN _pLvlVisited[NUMLEVELS + NUM_SETLVL];
	int _pGFXLoad;
	BYTE *_pNAnim[NUM_DIRS]; // Stand animations
	unsigned _pNFrames;
	int _pNWidth;
	BYTE *_pWAnim[NUM_DIRS]; // Walk animations
	unsigned _pWFrames;
	int _pWWidth;
	BYTE *_pAAnim[NUM_DIRS]; // Attack animations
	unsigned _pAFrames;
	int _pAWidth;
	unsigned _pAFNum;
	BYTE *_pLAnim[NUM_DIRS]; // Lightning spell cast animations
	BYTE *_pFAnim[NUM_DIRS]; // Fire spell cast animations
	BYTE *_pTAnim[NUM_DIRS]; // Generic spell cast animations
	unsigned _pSFrames;
	int _pSWidth;
	unsigned _pSFNum;
	BYTE *_pHAnim[NUM_DIRS]; // Getting hit animations
	unsigned _pHFrames;
	int _pHWidth;
	BYTE *_pDAnim[NUM_DIRS]; // Death animations
	unsigned _pDFrames;
	int _pDWidth;
	BYTE *_pBAnim[NUM_DIRS]; // Block animations
	unsigned _pBFrames;
	int _pBWidth;
	ItemStruct InvBody[NUM_INVLOC];
	ItemStruct SpdList[MAXBELTITEMS];
	ItemStruct InvList[NUM_INV_GRID_ELEM];
	char InvGrid[NUM_INV_GRID_ELEM];
	int _pNumInv;
	ItemStruct HoldItem;
	int _pISlMinDam;
	int _pISlMaxDam;
	int _pIBlMinDam;
	int _pIBlMaxDam;
	int _pIPcMinDam;
	int _pIPcMaxDam;
	int _pIEvasion;
	int _pIAC;
	int _pIHitChance;
	BYTE _pIBaseHitBonus; // indicator whether the base BonusToHit of the items is positive/negative/neutral
	BYTE _pICritChance; // 200 == 100%
	BYTE _pIBlockChance;
	uint64_t _pISpells; // Bitmask of staff spell
	int _pIFlags;
	int _pIGetHit;
	char _pISplLvlAdd;
	char _pIArrowVelBonus; // _pISplCost in vanilla code
	BYTE _pILifeSteal;
	BYTE _pIManaSteal;
	int _pIEnAc;
	int _pIFMinDam;
	int _pIFMaxDam;
	int _pILMinDam;
	int _pILMaxDam;
	int _pIMMinDam;
	int _pIMMaxDam;
	int _pIAMinDam;
	int _pIAMaxDam;
	BYTE pTownWarps;
	BYTE palign_CB;
	BYTE pDungMsgs;
	BYTE pDungMsgs2;
	BYTE *_pNData;
	BYTE *_pWData;
	BYTE *_pAData;
	BYTE *_pLData;
	BYTE *_pFData;
	BYTE *_pTData;
	BYTE *_pHData;
	BYTE *_pDData;
	BYTE *_pBData;
#ifdef X86_32bit_COMP
#ifdef HELLFIRE
	int alignment[180];
#else
	int alignment[182];
#endif
#endif
} PlayerStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(PlayerStruct) & (sizeof(PlayerStruct) - 1)) == 0, "Align PlayerStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// textdat
//////////////////////////////////////////////////

typedef struct TextDataStruct {
	const char *txtstr;
	BOOLEAN scrlltxt;
	BOOLEAN txtsfxset;
	int txtspd;
	int sfxnr;
} TextDataStruct;

//////////////////////////////////////////////////
// missiles
//////////////////////////////////////////////////

// TPDEF PTR FCN VOID MIADDPRC
// TPDEF PTR FCN VOID MIPROC

typedef struct MissileData {
	unsigned char mName;
	int (*mAddProc)(int, int, int, int, int, int, char, int, int);
	void (*mProc)(int);
	BOOL mDraw;
	BYTE mType;
	BYTE mdFlags;
	BYTE mResist;
	BYTE mFileNum;
	int mlSFX;
	int miSFX;
	unsigned char mlSFXCnt;
	unsigned char miSFXCnt;
} MissileData;

#ifdef X86_32bit_COMP
static_assert((sizeof(MissileData) & (sizeof(MissileData) - 1)) == 0, "Align MissileData to power of 2 for better performance.");
#endif

typedef struct MisFileData {
	int mfAnimFAmt;
	const char *mfName;
	int mfFlags;
	unsigned char mfAnimFrameLen[16];
	unsigned char mfAnimLen[16];
	int mfAnimWidth;
	int mfAnimXOffset;
#ifdef X86_32bit_COMP
	int alignment[3];
#endif
} MisFileData;
#ifdef X86_32bit_COMP
static_assert((sizeof(MisFileData) & (sizeof(MisFileData) - 1)) == 0, "Align MisFileData to power of 2 for better performance.");
#endif

typedef struct MissileStruct {
	int _miType;   // Type of projectile (MIS_*)
	BYTE _miSubType; // Sub-Type of projectile 
	BYTE _miFlags;
	BYTE _miResist;
	BYTE _miAnimType;
	BOOL _miAnimFlag;
	unsigned char *_miAnimData;
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
	int _miRange; // Time to live for the missile in game ticks, oncs 0 the missile will be marked for deletion via _miDelFlag
	int _miSource;
	int _miCaster;
	int _miMinDam;
	int _miMaxDam;
	int _miDist; // Used for arrows to measure distance travelled (increases by 1 each game tick). Higher value is a penalty for accuracy calculation when hitting enemy
	int _miLid;
	int _miVar1;
	int _miVar2;
	int _miVar3;
	int _miVar4;
	int _miVar5;
	int _miVar6;
	int _miVar7;
	int _miVar8;
#ifdef X86_32bit_COMP
	int alignment[5];
#endif
} MissileStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(MissileStruct) & (sizeof(MissileStruct) - 1)) == 128, "Align MissileStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// effects/sound
//////////////////////////////////////////////////

typedef struct TSnd {
	SoundSample *DSB;
	int start_tc;
} TSnd;

typedef struct TSFX {
	unsigned char bFlags;
	const char *pszName;
	TSnd *pSnd;
} TSFX;

//////////////////////////////////////////////////
// monster
//////////////////////////////////////////////////

typedef struct AnimStruct {
	BYTE *aCelData;
	BYTE *aData[NUM_DIRS];
	int aFrames;
	int aFrameLen;
#ifdef X86_32bit_COMP
	int alignment[1];
#endif
} AnimStruct;
#ifdef X86_32bit_COMP
static_assert((sizeof(AnimStruct) & (sizeof(AnimStruct) - 1)) == 32, "Align AnimStruct closer to power of 2 for better performance.");
#endif

typedef struct MonsterData {
	int width;
	int mImage;
	const char *GraphicType;
	BOOL has_special;
	const char *sndfile;
	BOOL snd_special;
	const char *TransFile;
	int mAnimFrames[NUM_MON_ANIM];
	int mAnimFrameLen[NUM_MON_ANIM];
	const char *mName;
	BYTE mLevel;
	BYTE mAi;
	int mMinHP;
	int mMaxHP;
	int mFlags;
	BYTE mInt;
	uint16_t mHit; // BUGFIX: Some monsters overflow this value on high difficultys (fixed)
	BYTE mMagic;
	BYTE mAFNum;
	BYTE mMinDamage;
	BYTE mMaxDamage;
	uint16_t mHit2; // BUGFIX: Some monsters overflow this value on high difficulty (fixed)
	BYTE mMagic2;     // unused
	BYTE mAFNum2;
	BYTE mMinDamage2;
	BYTE mMaxDamage2;
	BYTE mArmorClass; // AC+evasion: used against physical-hit (melee+projectile)
	BYTE mEvasion;    // evasion: used against magic-projectile
	BYTE mMonstClass; // unused
	uint16_t mMagicRes;
	uint16_t mMagicRes2;
	uint16_t mTreasure;
	BYTE mSelFlag;
	uint16_t mExp;
#ifdef X86_32bit_COMP
	int alignment[1];
#endif
} MonsterData;
#ifdef X86_32bit_COMP
static_assert((sizeof(MonsterData) & (sizeof(MonsterData) - 1)) == 0, "Align MonsterData to power of 2 for better performance.");
#endif

typedef struct CMonster {
	int cmType;
	BOOL cmPlaceScatter;
	AnimStruct cmAnims[NUM_MON_ANIM];
	TSnd *cmSnds[4][2];
	int cmWidth;
	int cmXOffset;
	BYTE cmDeadval;
	const MonsterData *cmData;
#ifdef X86_32bit_COMP
	int alignment[10];
#endif
} CMonster;
#ifdef X86_32bit_COMP
static_assert((sizeof(CMonster) & (sizeof(CMonster) - 1)) == 256, "Align CMonster closer to power of 2 for better performance.");
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
	BYTE *_mAnimData;
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
	int _mFlags;
	int _lastx; // the last known X-coordinate of the enemy
	int _lasty; // the last known Y-coordinate of the enemy
	int _mRndSeed;
	int _mAISeed;
	BYTE _uniqtype;
	BYTE _uniqtrans;
	BYTE _udeadval;
	BYTE mlid;
	BYTE falign_CB;
	BYTE leader;
	BYTE leaderflag;
	BYTE packsize;
	BYTE mLevel;
	BYTE _mSelFlag;
	BYTE _mAi;
	BYTE _mint;
	uint16_t _mHit; // BUGFIX: Some monsters overflow this value on high difficultys (fixed)
	BYTE _mMagic;
	BYTE _mAFNum;
	BYTE _mMinDamage;
	BYTE _mMaxDamage;
	uint16_t _mHit2; // BUGFIX: Some monsters overflow this value on high difficulty (fixed)
	BYTE _mMagic2;     // unused
	BYTE _mAFNum2;
	BYTE _mMinDamage2;
	BYTE _mMaxDamage2;
	BYTE _mArmorClass; // AC+evasion: used against physical-hit (melee+projectile)
	BYTE _mEvasion;    // evasion: used against magic-projectile
	uint16_t mMagicRes;
	uint16_t mExp;
	int mtalkmsg;
	const char *mName;
	CMonster *MType;
	const MonsterData *MData;
	int _mType;
	AnimStruct *_mAnims;
	int _mAnimWidth;
	int _mAnimXOffset;
#ifdef X86_32bit_COMP
	int alignment[8];
#endif
} MonsterStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(MonsterStruct) & (sizeof(MonsterStruct) - 1)) == 0, "Align MonsterStruct to power of 2 for better performance.");
#endif

typedef struct UniqMonstStruct {
	int mtype;
	const char *mName;
	const char *mTrnName;
	BYTE muLevelIdx; // level-index to place the monster
	BYTE muLevel;    // difficulty level of the monster
	uint16_t mmaxhp;
	BYTE mAi;
	BYTE mint;
	BYTE mMinDamage;
	BYTE mMaxDamage;
	BYTE mMinDamage2;
	BYTE mMaxDamage2;
	uint16_t mMagicRes;
	BYTE mUnqAttr;
	BYTE mUnqHit;
	BYTE mUnqAC;
	char mQuestId;
	int mtalkmsg;
} UniqMonstStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(UniqMonstStruct) & (sizeof(UniqMonstStruct) - 1)) == 0, "Align UniqMonstStruct to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// objects
//////////////////////////////////////////////////

typedef struct ObjDataStruct {
	char oload;
	char ofindex;
	BYTE oLvlTypes;
	BYTE oSetLvlType;
	char otheme;
	char oquest;
	BOOL oAnimFlag;
	int oAnimBaseFrame; // The starting/base frame of (initially) non-animated objects
	int oAnimFrameLen; // Tick length of each frame in the current animation
	int oAnimLen;   // Number of frames in current animation
	int oAnimWidth;
	BOOL oSolidFlag;
	BOOL oMissFlag;
	BOOL oLightFlag;
	char oBreak;
	char oSelFlag;
	BOOL oTrapFlag;
#ifdef X86_32bit_COMP
	int alignment[4];
#endif
} ObjDataStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(ObjDataStruct) & (sizeof(ObjDataStruct) - 1)) == 0, "Align ObjDataStruct closer to power of 2 for better performance.");
#endif

typedef struct ObjectStruct {
	int _otype;
	int _ox;
	int _oy;
	BOOL _oAnimFlag;
	unsigned char *_oAnimData;
	int _oAnimFrameLen; // Tick length of each frame in the current animation
	int _oAnimCnt;   // Increases by one each game tick, counting how close we are to _oAnimFrameLen
	int _oAnimLen;   // Number of frames in current animation
	int _oAnimFrame; // Current frame of animation.
	int _oAnimWidth;
	int _oAnimXOffset;
	//BOOL _oDelFlag;
	BOOL _oSolidFlag;
	BOOL _oMissFlag;
	BOOL _oLight;
	char _oBreak; // check
	char _oSelFlag; // check
	BOOL _oPreFlag;
	BOOL _oTrapFlag;
	BOOL _oDoorFlag;
	int _olid;
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
	int alignment[4];
#endif
} ObjectStruct;

#ifdef X86_32bit_COMP
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

#ifdef X86_32bit_COMP
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

typedef struct _FILETIME {
	DWORD dwLowDateTime;
	DWORD dwHighDateTime;
} FILETIME;

typedef struct PkPlayerStruct {
	FILETIME archiveTime;
	BYTE plrlevel;
	BYTE pTeam;
	BYTE px;
	BYTE py;
	char pName[PLR_NAME_LEN];
	BYTE pClass;
	BYTE pLevel;
	BYTE pLightRad;
	BYTE pManaShield;
	WORD pTimer[NUM_PLRTIMERS];
	WORD pBaseStr;
	WORD pBaseMag;
	WORD pBaseDex;
	WORD pBaseVit;
	BYTE pDiabloKillLevel;
	WORD pStatPts;
	DWORD pExperience;
	INT pGold;
	INT pHPBase;
	INT pMaxHPBase;
	INT pManaBase;
	INT pMaxManaBase;
	BYTE pSkillLvl[64];
	BYTE pSkillActivity[64];
	DWORD pSkillExp[64];
	uint64_t pMemSkills;
	PkItemStruct InvBody[NUM_INVLOC];
	PkItemStruct SpdList[MAXBELTITEMS];
	PkItemStruct InvList[NUM_INV_GRID_ELEM];
	char InvGrid[NUM_INV_GRID_ELEM];
	BYTE _pNumInv;
	BYTE pAtkSkillHotKey[4];         // the attack skill selected by the hotkey
	BYTE pAtkSkillTypeHotKey[4];     // the (RSPLTYPE_)type of the attack skill selected by the hotkey
	BYTE pMoveSkillHotKey[4];        // the movement skill selected by the hotkey
	BYTE pMoveSkillTypeHotKey[4];    // the (RSPLTYPE_)type of the movement skill selected by the hotkey
	BYTE pAltAtkSkillHotKey[4];      // the attack skill selected by the alt-hotkey
	BYTE pAltAtkSkillTypeHotKey[4];  // the (RSPLTYPE_)type of the attack skill selected by the alt-hotkey
	BYTE pAltMoveSkillHotKey[4];     // the movement skill selected by the alt-hotkey
	BYTE pAltMoveSkillTypeHotKey[4]; // the (RSPLTYPE_)type of the movement skill selected by the alt-hotkey
} PkPlayerStruct;
#pragma pack(pop)

//////////////////////////////////////////////////
// msg
//////////////////////////////////////////////////

#pragma pack(push, 1)
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

typedef struct TCmdLocAttack {
	BYTE bCmd;
	BYTE x;
	BYTE y;
	BYTE laSkill;
	BYTE laLevel;
} TCmdLocAttack;

typedef struct TCmdLocSkill {
	BYTE bCmd;
	BYTE x;
	BYTE y;
	BYTE lsSkill;
	BYTE lsFrom;
	BYTE lsLevel;
} TCmdLocSkill;

typedef struct TCmdPlrAttack {
	BYTE bCmd;
	BYTE paPnum;
	BYTE paSkill;
	BYTE paLevel;
} TCmdPlrAttack;

typedef struct TCmdPlrSkill {
	BYTE bCmd;
	BYTE psPnum;
	BYTE psSkill;
	BYTE psFrom;
	BYTE psLevel;
} TCmdPlrSkill;

typedef struct TCmdPlrDamage {
	BYTE bCmd;
	BYTE pdPnum;
	DWORD pdDamage;
} TCmdPlrDamage;

typedef struct TCmdMonstAttack {
	BYTE bCmd;
	WORD maMnum;
	BYTE maSkill;
	BYTE maLevel;
} TCmdMonstAttack;

typedef struct TCmdMonstSkill {
	BYTE bCmd;
	WORD msMnum;
	BYTE msSkill;
	BYTE msFrom;
	BYTE msLevel;
} TCmdMonstSkill;

typedef struct TCmdMonstDamage {
	BYTE bCmd;
	WORD mdMnum;
	INT mdHitpoints;
	DWORD mdDamage;
} TCmdMonstDamage;

typedef struct TCmdMonstKill {
	BYTE bCmd;
	WORD mkMnum;
	BYTE mkLevel;
	BYTE mkX;
	BYTE mkY;
	BYTE mkDir;
	BYTE mkPnum;
} TCmdMonstKill;

typedef struct TCmdGolem {
	BYTE bCmd;
	BYTE _mx;
	BYTE _my;
	BYTE _mdir;
	BYTE _menemy;
	INT _mhitpoints;
	BYTE _currlevel;
} TCmdGolem;

typedef struct TCmdQuest {
	BYTE bCmd;
	BYTE q;
	BYTE qstate;
	BYTE qlog;
	BYTE qvar1;
} TCmdQuest;

typedef struct TCmdGItem {
	BYTE bCmd;
	BYTE bMaster;
	BYTE bPnum;
	BYTE bCursitem;
	BYTE bLevel;
	BYTE x;
	BYTE y;
	PkItemStruct item;
	INT dwTime;
} TCmdGItem;

typedef struct TCmdPItem {
	BYTE bCmd;
	BYTE x;
	BYTE y;
	PkItemStruct item;
} TCmdPItem;

typedef struct TCmdChItem {
	BYTE bCmd;
	BYTE bLoc;
	INT dwSeed;
	WORD wIndx;
	WORD wCI;
	BYTE bCh;
	BYTE bMCh;
	BOOLEAN bId;
} TCmdChItem;

typedef struct TCmdPlrInfoHdr {
	BYTE bCmd;
	WORD wOffset;
	WORD wBytes;
} TCmdPlrInfoHdr;

typedef struct TCmdString {
	BYTE bCmd;
	char str[MAX_SEND_STR_LEN];
} TCmdString;

typedef struct TFakeCmdPlr {
	BYTE bCmd;
	BYTE bPlr;
} TFakeCmdPlr;

typedef struct TFakeDropPlr {
	BYTE bCmd;
	BYTE bPlr;
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
	BYTE _menemy;
	BYTE _mdelta;
} TSyncMonster;

typedef struct TPktHdr {
	INT php;
	INT pmhp;
	INT pmp;
	INT pmmp;
	BYTE px;
	BYTE py;
	WORD wCheck;
	WORD wLen;
} TPktHdr;

typedef struct TPkt {
	TPktHdr hdr;
	BYTE body[MAX_NETMSG_SIZE - sizeof(TPktHdr)];
} TPkt;

typedef struct DMonsterStr {
	BYTE _mx;
	BYTE _my;
	BYTE _mdir;
	BYTE _menemy;
	DWORD _mactive;
	INT _mhitpoints;
} DMonsterStr;

typedef struct DObjectStr {
	BYTE bCmd;
} DObjectStr;

typedef struct DLevel {
	TCmdPItem item[MAXITEMS];
	DObjectStr object[MAXOBJECTS];
	DMonsterStr monster[MAXMONSTERS];
} DLevel;

typedef struct LocalLevel {
	BOOLEAN automapsv[DMAXX][DMAXY];
} LocalLevel;

typedef struct DPortal {
	BYTE x;
	BYTE y;
	BYTE level;
} DPortal;

typedef struct MultiQuests {
	BYTE qstate;
	BYTE qlog;
	BYTE qvar1;
} MultiQuests;

typedef struct DJunk {
	DPortal portal[MAXPORTAL];
	MultiQuests quests[NUM_QUESTS];
} DJunk;

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

typedef struct TMegaPkt {
	struct TMegaPkt *pNext;
	DWORD dwSpaceLeft;
	BYTE data[32000];
} TMegaPkt;
#pragma pack(pop)

typedef struct DMegaPkt {
	struct DMegaPkt *dmpNext;
	int dmpPlr;
	unsigned dmpLen;
	BYTE dmpCmd;
	BYTE data[32000];
} DMegaPkt;

typedef struct TBuffer {
	DWORD dwDataSize;
	BYTE bData[4096];
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

typedef struct LevelDataStruct {
	BYTE dLevel;
	BOOLEAN dSetLvl;
	BYTE dType;
	BYTE dDunType;
	BYTE dMusic;
	BYTE dMicroTileLen;
	BYTE dBlocks;
	const char *dLevelName;
	const char *dAutomapData;
	const char *dSolidTable;
	const char *dDunCels;
	const char *dMegaTiles;
	const char *dLvlPieces;
	const char *dSpecCels;
	const char *dPalName;
	const char *dLoadCels;
	const char *dLoadPal;
	const char *dSetLvlPreDun;
	const char *dSetLvlDun;
	BYTE dSetLvlDunX;
	BYTE dSetLvlDunY;
	BYTE dMonTypes[32];
#ifdef X86_32bit_COMP
	int alignment[9];
#endif
} LevelDataStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(LevelDataStruct) & (sizeof(LevelDataStruct) - 1)) == 0, "Align LevelDataStruct to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// quests
//////////////////////////////////////////////////

typedef struct QuestStruct {
	BYTE _qidx;
	BYTE _qactive;
	int _qtx;
	int _qty;
	unsigned int _qmsg;
	unsigned char _qvar1;
	unsigned char _qvar2;
	BOOL _qlog;
#ifdef X86_32bit_COMP
	int alignment[2];
#endif
} QuestStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(QuestStruct) & (sizeof(QuestStruct) - 1)) == 0, "Align QuestStruct to power of 2 for better performance.");
#endif

typedef struct QuestData {
	BYTE _qdlvl;
	BYTE _qslvl;
	int _qdmsg;
	const char *_qlstr;
} QuestData;

//////////////////////////////////////////////////
// gamemenu/gmenu
//////////////////////////////////////////////////

// TPDEF PTR FCN VOID TMenuFcn

typedef struct TMenuItem {
	DWORD dwFlags;
	const char *pszStr;
	void (*fnMenu)(bool); /* fix, should have one arg */
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
	const char *sNameText;
	char sBookLvl;
	char sStaffLvl;
	char sScrollLvl;
	BOOLEAN sTargeted;
	BYTE scCurs; // cursor for scrolls/runes
	BYTE spCurs; // cursor for spells
	BYTE sFlags; // the required flags(SFLAG*) to use the skill
	BYTE sMinInt;
	unsigned char sSFX;
	unsigned char sMissile;
	unsigned char sManaAdj;
	unsigned char sMinMana;
	WORD sStaffMin;
	WORD sStaffMax;
	int sBookCost;
	int sStaffCost; // == sScrollCost
} SpellData;
#ifdef X86_32bit_COMP
static_assert((sizeof(SpellData) & (sizeof(SpellData) - 1)) == 0, "Align SpellData to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// towners
//////////////////////////////////////////////////

typedef struct TNQ {
	unsigned char _qsttype;
	unsigned char _qstmsg;
	BOOLEAN _qstmsgact;
} TNQ;

typedef struct TownerStruct {
	int _ttype;
	int _tx;    // Tile X-position of NPC
	int _ty;    // Tile Y-position of NPC
	int _txoff; // Sprite X-offset (unused)
	int _tyoff; // Sprite Y-offset (unused)
	int _txvel; // X-velocity during movement (unused)
	int _tyvel; // Y-velocity during movement (unused)
	int _tdir;  // Facing of NPC (unused)
	BYTE *_tAnimData;
	int _tAnimFrameLen; // Tick length of each frame in the current animation
	int _tAnimCnt;   // Increases by one each game tick, counting how close we are to _tAnimFrameLen
	int _tAnimLen;   // Number of frames in current animation
	int _tAnimFrame; // Current frame of animation.
	int _tAnimFrameCnt;
	int _tAnimWidth;
	int _tAnimXOffset;
	char _tAnimOrder;
	BYTE _tListener;
	BYTE _tStoreId;
	int _tStoreTalk;
	BOOL _tSelFlag;
	int _tSeed;
	const char *_tName;
#ifdef X86_32bit_COMP
	int alignment[11];
#endif
} TownerStruct;
#ifdef X86_32bit_COMP
static_assert((sizeof(TownerStruct) & (sizeof(TownerStruct) - 1)) == 0, "Align TownerStruct to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// gendung
//////////////////////////////////////////////////

typedef struct ScrollStruct {
	int _sxoff; // X-offset of camera position. This usually corresponds to a negative version of plr[myplr]._pxoff
	int _syoff; // Y-offset of camera position. This usually corresponds to a negative version of plr[myplr]._pyoff
	int _sdx;
	int _sdy;
	int _sdir;
} ScrollStruct;

typedef struct THEME_LOC {
	int x;
	int y;
	int ttval;
	int width;
	int height;
} THEME_LOC;

typedef struct MICROS {
	WORD mt[16];
} MICROS;

//////////////////////////////////////////////////
// drlg
//////////////////////////////////////////////////

typedef struct ShadowStruct {
	unsigned char strig;
	unsigned char s1;
	unsigned char s2;
	unsigned char s3;
	unsigned char nv1;
	unsigned char nv2;
	unsigned char nv3;
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
	char ttype; /* aligned 4 */
	int ttval;
} ThemeStruct;

#ifdef X86_32bit_COMP
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

#ifdef X86_32bit_COMP
static_assert((sizeof(LightListStruct) & (sizeof(LightListStruct) - 1)) == 0, "Align LightListStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// dead
//////////////////////////////////////////////////

typedef struct DeadStruct {
	unsigned char *_deadData[NUM_DIRS];
	int _deadFrame;
	int _deadWidth;
	int _deadXOffset;
	char _deadtrans;
} DeadStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(DeadStruct) & (sizeof(DeadStruct) - 1)) == 32, "Align DeadStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// diabloui
//////////////////////////////////////////////////

// TPDEF PTR FCN VOID PLAYSND

typedef struct _uidefaultstats {
	WORD strength;
	WORD magic;
	WORD dexterity;
	WORD vitality;
} _uidefaultstats;

typedef struct _uiheroinfo {
	char name[16];
	WORD level;
	BYTE heroclass;
	BYTE herorank;
	WORD strength;
	WORD magic;
	WORD dexterity;
	WORD vitality;
	bool hassaved;
} _uiheroinfo;

//////////////////////////////////////////////////
// storm
//////////////////////////////////////////////////

typedef struct _SNETGAMEDATA {
	INT dwSeed;
	DWORD dwVersionId;
	BYTE bPlayerId; // internal-only!
	BYTE bDifficulty;
	BYTE bTickRate;
	BYTE bMaxPlayers;
} _SNETGAMEDATA;

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

typedef struct _SNETEVENT {
	DWORD eventid;
	DWORD playerid;
	void *_eData;
	DWORD databytes;
} _SNETEVENT;

//////////////////////////////////////////////////
// path
//////////////////////////////////////////////////

typedef struct PATHNODE {
	BYTE totalCost;
	BYTE remainingCost;
	BYTE walkCost;
	int x;
	int y;
	struct PATHNODE *Parent;
	struct PATHNODE *Child[NUM_DIRS];
	struct PATHNODE *NextNode;
#ifdef X86_32bit_COMP
	int alignment[3];
#endif
} PATHNODE;

#ifdef X86_32bit_COMP
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
	TMsgHdr *pNext;
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
	char _sstr[128];
	bool _sjust;
	BYTE _sclr;
	bool _sline;
	bool _ssel;
	int _sval;
} STextStruct;

//////////////////////////////////////////////////
// plrmsg
//////////////////////////////////////////////////

typedef struct _plrmsg {
	DWORD time;
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
	BYTE *srcData;
	DWORD srcOffset;
	BYTE *destData;
	DWORD destOffset;
	DWORD size;
} TDataInfo;

DEVILUTION_END_NAMESPACE
