/**
 * @file structs.h
 *
 * Various global structures.
 */

DEVILUTION_BEGIN_NAMESPACE

#if INT_MAX == INT32_MAX && SHRT_MAX == INT16_MAX
#define X86_32bit_COMP
#endif
//////////////////////////////////////////////////
// control
//////////////////////////////////////////////////

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
	const char *iSName;
	char iMinMLvl;
	short iDurability;
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
	int alignment[1];
#endif
} ItemDataStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(ItemDataStruct) & (sizeof(ItemDataStruct) - 1)) == 0, "Align ItemDataStruct to power of 2 for better performance.");
#endif

typedef struct ItemGetRecordStruct {
	int nSeed;
	unsigned short wCI;
	int nIndex;
	unsigned int dwTimestamp;
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
	unsigned char *_iAnimData; // PSX name -> ItemFrame
	int _iAnimLen;             // Number of frames in current animation
	int _iAnimFrame;           // Current frame of animation.
	int _iAnimWidth;
	int _iAnimWidth2; // width 2?
	BOOL _iPostDraw;
	BOOL _iIdentified;
	char _iName[64];
	char _iIName[64];
	BYTE _iSelFlag;
	BYTE _iMagical;
	BYTE _iLoc;
	// item_class enum
	BYTE _iClass;
	int _iCurs;
	int _ivalue;
	int _iIvalue;
	BYTE _iMinDam;
	BYTE _iMaxDam;
	int _iAC;
	// item_special_effect
	int _iFlags;
	// item_misc_id
	int _iMiscId;
	// spell_id
	int _iSpell;
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
	int _iPLMana;
	int _iPLHP;
	int _iPLDamMod;
	int _iPLGetHit;
	char _iPLLight;
	BYTE _iDamType;
	char _iSplLvlAdd;
	char _iRequest;
	BYTE _iManaSteal;
	BYTE _iLifeSteal;
	int _iUid;
	BYTE _iFMinDam;
	BYTE _iFMaxDam;
	BYTE _iLMinDam;
	BYTE _iLMaxDam;
	BYTE _iMMinDam;
	BYTE _iMMaxDam;
	BYTE _iHMinDam;
	BYTE _iHMaxDam;
	int _iPLEnAc;
	BYTE _iPrePower;
	BYTE _iSufPower;
	int _iVAdd;
	int _iVMult;
	BYTE _iMinStr;
	BYTE _iMinMag;
	BYTE _iMinDex;
	BOOL _iStatFlag;
	int _iFlags2; // _oldlight or _iInvalid reused to store additional item effects
#ifdef X86_32bit_COMP
	int alignment[12];
#endif
} ItemStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(ItemStruct) & (sizeof(ItemStruct) - 1)) == 256, "Align ItemStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// player
//////////////////////////////////////////////////

typedef struct PlayerStruct {
	int _pmode;
	char walkpath[MAX_PATH_LENGTH];
	BOOLEAN plractive;
	int destAction;
	int destParam1;
	int destParam2;
	int destParam3;
	int destParam4;
	int plrlevel;
	int _px;      // Tile X-position of player
	int _py;      // Tile Y-position of player
	int _pfutx;   // Future tile X-position of player. Set at start of walking animation
	int _pfuty;   // Future tile Y-position of player. Set at start of walking animation
	int _ptargx;  // unused
	int _ptargy;  // unused
	int _pownerx; // unused
	int _pownery; // unused
	int _poldx;   // Most recent X-position in dPlayer.
	int _poldy;   // Most recent Y-position in dPlayer.
	int _pxoff;   // Player sprite's pixel X-offset from tile.
	int _pyoff;   // Player sprite's pixel Y-offset from tile.
	int _pxvel;   // Pixel X-velocity while walking. Indirectly applied to _pxoff via _pvar6
	int _pyvel;   // Pixel Y-velocity while walking. Indirectly applied to _pyoff via _pvar7
	int _pdir;    // Direction faced by player (direction enum)
	int _nextdir; // unused
	int _pgfxnum; // Bitmask indicating what variant of the sprite the player is using. Lower byte define weapon (anim_weapon_id) and higher values define armour (starting with anim_armor_id)
	unsigned char *_pAnimData;
	int _pAnimDelay; // Tick length of each frame in the current animation
	int _pAnimCnt;   // Increases by one each game tick, counting how close we are to _pAnimDelay
	unsigned _pAnimLen;   // Number of frames in current animation
	unsigned _pAnimFrame; // Current frame of animation.
	int _pAnimWidth;
	int _pAnimWidth2;
	int _peflag; // unused
	int _plid;
	int _pvid;
	int _pSpell;
	char _pSplType; // unused
	char _pSplFrom;
	int _pTSpell;
	char _pTSplType; // unused
	int _pRSpell;
	// enum spell_type
	char _pRSplType;
	int _pSBkSpell; // unused
	char _pSBkSplType; // unused
	char _pSplLvl[64];
	uint64_t _pMemSpells;  // Bitmask of learned spells
	uint64_t _pAblSpells;  // Bitmask of abilities
	uint64_t _pScrlSpells; // Bitmask of spells avalible via scrolls
	UCHAR _pSpellFlags;
	int _pSplHotKey[4];
	char _pSplTHotKey[4];
	int _pwtype;
	BOOLEAN _pBlockFlag;
	BOOLEAN _pInvincible;
	char _pLightRad;
	BOOLEAN _pLvlChanging; // True when the player is transitioning between levels
	char _pName[PLR_NAME_LEN];
	// plr_class enum value.
	// TODO: this could very well be `enum plr_class _pClass`
	// since there are 3 bytes of alingment after this field.
	// it could just be that the compiler optimized away all accesses to
	// the higher bytes by using byte instructions, since all possible values
	// of plr_class fit into one byte.
	char _pClass;
	int _pStrength;
	int _pBaseStr;
	int _pMagic;
	int _pBaseMag;
	int _pDexterity;
	int _pBaseDex;
	int _pVitality;
	int _pBaseVit;
	int _pStatPts;
	int _pBaseToBlk;
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
	char _pLevel;
	BOOLEAN _pLvlUp; // _pMaxLvl in vanilla code
	int _pExperience;
	int _pMaxExp; // unused
	int _pNextExper;
	char _pArmorClass;
	char _pMagResist;
	char _pFireResist;
	char _pLghtResist;
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
	BOOLEAN _pLvlVisited[NUMLEVELS];
	BOOLEAN _pSLvlVisited[NUMLEVELS]; // only 10 used
	int _pGFXLoad;
	unsigned char *_pNAnim[8]; // Stand animations
	unsigned _pNFrames;
	int _pNWidth;
	unsigned char *_pWAnim[8]; // Walk animations
	unsigned _pWFrames;
	int _pWWidth;
	unsigned char *_pAAnim[8]; // Attack animations
	unsigned _pAFrames;
	int _pAWidth;
	unsigned _pAFNum;
	unsigned char *_pLAnim[8]; // Lightning spell cast animations
	unsigned char *_pFAnim[8]; // Fire spell cast animations
	unsigned char *_pTAnim[8]; // Generic spell cast animations
	unsigned _pSFrames;
	int _pSWidth;
	unsigned _pSFNum;
	unsigned char *_pHAnim[8]; // Getting hit animations
	unsigned _pHFrames;
	int _pHWidth;
	unsigned char *_pDAnim[8]; // Death animations
	unsigned _pDFrames;
	int _pDWidth;
	unsigned char *_pBAnim[8]; // Block animations
	unsigned _pBFrames;
	int _pBWidth;
	ItemStruct InvBody[NUM_INVLOC];
	ItemStruct InvList[NUM_INV_GRID_ELEM];
	int _pNumInv;
	char InvGrid[NUM_INV_GRID_ELEM];
	ItemStruct SpdList[MAXBELTITEMS];
	ItemStruct HoldItem;
	int _pISlMinDam;
	int _pISlMaxDam;
	int _pIBlMinDam;
	int _pIBlMaxDam;
	int _pIPcMinDam;
	int _pIPcMaxDam;
	int _pIAC;
	int _pIHitChance;
	BYTE _pIBaseACBonus; // indicator whether the base ACBonus of the items is positive/negative/neutral
	BYTE _pIBaseDamBonus; // indicator whether the base DamageBonus of the items is positive/negative/neutral
	BYTE _pIBaseHitBonus; // indicator whether the base BonusToHit of the items is positive/negative/neutral
	BYTE _pICritChance; // 200 == 100%
	int _pIMagToHit;
	uint64_t _pISpells; // Bitmask of staff spell
	int _pIFlags;
	int _pIGetHit;
	char _pISplLvlAdd;
	char _pIArrowVelBonus; // _pISplCost in vanilla code
	BYTE _pILifeSteal;
	BYTE _pIManaSteal;
	int _pIFlags2; // _pISplDur in vanilla code
	int _pIEnAc;
	int _pIFMinDam;
	int _pIFMaxDam;
	int _pILMinDam;
	int _pILMaxDam;
	int _pIMMinDam;
	int _pIMMaxDam;
	int _pIHMinDam;
	int _pIHMaxDam;
	int _pOilType;
	unsigned char pTownWarps;
	unsigned char pDungMsgs;
	unsigned char pLvlLoad;
	unsigned char pBattleNet;
	BYTE pManaShield;
	unsigned char pDungMsgs2;
	char bReserved[4];
	short wReserved[7];
	DWORD pDiabloKillLevel;
	int dwReserved[7];
	unsigned char *_pNData;
	unsigned char *_pWData;
	unsigned char *_pAData;
	unsigned char *_pLData;
	unsigned char *_pFData;
	unsigned char *_pTData;
	unsigned char *_pHData;
	unsigned char *_pDData;
	unsigned char *_pBData;
	void *pReserved;
#ifdef X86_32bit_COMP
#ifdef HELLFIRE
	int alignment[479];
#else
	int alignment[483];
#endif
#endif
} PlayerStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(PlayerStruct) & (sizeof(PlayerStruct) - 1)) == 16384, "Align PlayerStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// textdat
//////////////////////////////////////////////////

typedef struct TextDataStruct {
	const char *txtstr;
	BOOL scrlltxt;
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
	unsigned char mType;
	unsigned char mdFlags;
	unsigned char mResist;
	unsigned char mFileNum;
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
	unsigned char mfAnimDelay[16];
	unsigned char mfAnimLen[16];
	int mfAnimWidth[16];
	int mfAnimWidth2[16];
#ifdef X86_32bit_COMP
	int alignment[5];
#endif
} MisFileData;
#ifdef X86_32bit_COMP
static_assert((sizeof(MisFileData) & (sizeof(MisFileData) - 1)) == 128, "Align MisFileData to power of 2 for better performance.");
#endif

typedef struct MissileStruct {
	int _miType;  // Type of projectile (missile_id)
	int _mix;     // Tile X-position of the missile
	int _miy;     // Tile Y-position of the missile
	int _mixoff;  // Sprite pixel X-offset for the missile
	int _miyoff;  // Sprite pixel Y-offset for the missile
	int _mixvel;  // Missile tile X-velocity while walking. This gets added onto _mitxoff each game tick
	int _miyvel;  // Missile tile Y-velocity while walking. This gets added onto _mitxoff each game tick
	int _misx;    // Initial tile X-position for missile
	int _misy;    // Initial tile Y-position for missile
	int _mitxoff; // How far the missile has travelled in its lifespan along the X-axis. mix/miy/mxoff/myoff get updated every game tick based on this
	int _mityoff; // How far the missile has travelled in its lifespan along the Y-axis. mix/miy/mxoff/myoff get updated every game tick based on this
	int _miDir;   // The direction of the missile
	int _miSpllvl;
	BOOL _miDelFlag; // Indicate weather the missile should be deleted
	BYTE _miAnimType;
	int _miAnimFlags;
	unsigned char *_miAnimData;
	int _miAnimDelay; // Tick length of each frame in the current animation
	int _miAnimLen;   // Number of frames in current animation
	int _miAnimWidth;
	int _miAnimWidth2;
	int _miAnimCnt; // Increases by one each game tick, counting how close we are to _pAnimDelay
	int _miAnimAdd;
	int _miAnimFrame; // Current frame of animation.
	BOOL _miDrawFlag;
	BOOL _miLightFlag;
	BOOL _miPreFlag;
	int _miUniqTrans;
	int _miRange; // Time to live for the missile in game ticks, oncs 0 the missile will be marked for deletion via _miDelFlag
	int _miSource;
	int _miCaster;
	int _miDam;
	BOOL _miHitFlag; // unused
	int _miDist; // Used for arrows to measure distance travelled (increases by 1 each game tick). Higher value is a penalty for accuracy calculation when hitting enemy
	int _miLid;
	int _miRnd;
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
#endif
} MissileStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(MissileStruct) & (sizeof(MissileStruct) - 1)) == 128, "Align MissileStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// effects/sound
//////////////////////////////////////////////////

typedef struct TSnd {
	const char *sound_path;
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
	BYTE *CMem;
	BYTE *Data[8];
	int Frames;
	int Rate;
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
	BOOL has_trans;
	const char *TransFile;
	int Frames[6];
	int Rate[6];
	const char *mName;
	BYTE mMinDLvl;
	BYTE mMaxDLvl;
	char mLevel;
	char mAi;
	int mMinHP;
	int mMaxHP;
	int mFlags;
	unsigned char mInt;
	unsigned short mHit; // BUGFIX: Some monsters overflow this value on high difficultys (fixed)
	unsigned char mAFNum;
	unsigned char mMinDamage;
	unsigned char mMaxDamage;
	unsigned short mHit2; // BUGFIX: Some monsters overflow this value on high difficulty (fixed)
	unsigned char mAFNum2;
	unsigned char mMinDamage2;
	unsigned char mMaxDamage2;
	unsigned char mArmorClass;
	char mMonstClass;
	unsigned short mMagicRes;
	unsigned short mMagicRes2;
	unsigned short mTreasure;
	char mSelFlag;
	unsigned short mExp;
} MonsterData;
#ifdef X86_32bit_COMP
static_assert((sizeof(MonsterData) & (sizeof(MonsterData) - 1)) == 0, "Align MonsterData to power of 2 for better performance.");
#endif

typedef struct CMonster {
	int mtype;
	int mPlaceFlags;
	AnimStruct Anims[6];
	TSnd *Snds[4][2];
	int width;
	int width2;
	int mMinHP;
	int mMaxHP;
	BOOL has_special;
	unsigned char mAFNum; // unused
	char mdeadval;
	MonsterData *MData;
	// A TRN file contains a sequence of colour transitions, represented
	// as indexes into a palette. (a 256 byte array of palette indices)
	BYTE *trans_file;
#ifdef X86_32bit_COMP
	int alignment[6];
#endif
} CMonster;
#ifdef X86_32bit_COMP
static_assert((sizeof(CMonster) & (sizeof(CMonster) - 1)) == 256, "Align CMonster closer to power of 2 for better performance.");
#endif

typedef struct MonsterStruct { // note: missing field _mAFNum
	int _mMTidx;
	int _mmode; /* MON_MODE */
	unsigned char _mgoal;
	int _mgoalvar1;
	int _mgoalvar2;
	int _mgoalvar3;
	int field_18;
	unsigned char _pathcount;
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
	int _menemy;            // The current target of the mosnter. An index in to either the plr or monster array based on the _meflag value.
	unsigned char _menemyx; // X-coordinate of enemy (usually correspond's to the enemy's futx value)
	unsigned char _menemyy; // Y-coordinate of enemy (usually correspond's to the enemy's futy value)
	short falign_52;        // probably _mAFNum (unused)
	unsigned char *_mAnimData;
	int _mAnimDelay; // Tick length of each frame in the current animation
	int _mAnimCnt;   // Increases by one each game tick, counting how close we are to _pAnimDelay
	int _mAnimLen;   // Number of frames in current animation
	int _mAnimFrame; // Current frame of animation.
	BOOL _meflag; // unused
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
	unsigned char _mAi;
	unsigned char _mint;
	short falign_9A;
	int _mFlags;
	BYTE _msquelch;
	int falign_A4;
	int _lastx;
	int _lasty;
	int _mRndSeed;
	int _mAISeed;
	int falign_B8;
	unsigned char _uniqtype;
	unsigned char _uniqtrans;
	char _udeadval;
	char mWhoHit;
	char mLevel;
	unsigned short mExp;
	unsigned short mHit;
	unsigned char mMinDamage;
	unsigned char mMaxDamage;
	unsigned short mHit2;
	unsigned char mMinDamage2;
	unsigned char mMaxDamage2;
	unsigned char mArmorClass;
	char falign_CB;
	unsigned short mMagicRes;
	int mtalkmsg;
	unsigned char leader;
	unsigned char leaderflag;
	unsigned char packsize;
	unsigned char mlid;
	const char *mName;
	CMonster *MType;
	MonsterData *MData;
	int _mType;
	AnimStruct *_mAnims;
	int _mAnimWidth;
	int _mAnimWidth2;
#ifdef X86_32bit_COMP
	int alignment[3];
#endif
} MonsterStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(MonsterStruct) & (sizeof(MonsterStruct) - 1)) == 0, "Align MonsterStruct to power of 2 for better performance.");
#endif

typedef struct UniqMonstStruct {
	int mtype;
	const char *mName;
	const char *mTrnName;
	unsigned char mlevel;
	unsigned short mmaxhp;
	unsigned char mAi;
	unsigned char mint;
	unsigned char mMinDamage;
	unsigned char mMaxDamage;
	unsigned char mMinDamage2;
	unsigned char mMaxDamage2;
	unsigned short mMagicRes;
	unsigned short mUnqAttr;
	unsigned char mUnqVar1;
	unsigned char mUnqVar2;
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
	char ominlvl;
	char omaxlvl;
	char olvltype;
	char otheme;
	char oquest;
	int oAnimFlag;
	int oAnimDelay; // Tick length of each frame in the current animation
	int oAnimLen;   // Number of frames in current animation
	int oAnimWidth;
	BOOL oSolidFlag;
	BOOL oMissFlag;
	BOOL oLightFlag;
	char oBreak;
	char oSelFlag;
	BOOL oTrapFlag;
#ifdef X86_32bit_COMP
	int alignment[1];
#endif
} ObjDataStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(ObjDataStruct) & (sizeof(ObjDataStruct) - 1)) == 32, "Align ObjDataStruct closer to power of 2 for better performance.");
#endif

typedef struct ObjectStruct {
	int _otype;
	int _ox;
	int _oy;
	int _oLight;
	int _oAnimFlag;
	unsigned char *_oAnimData;
	int _oAnimDelay; // Tick length of each frame in the current animation
	int _oAnimCnt;   // Increases by one each game tick, counting how close we are to _pAnimDelay
	int _oAnimLen;   // Number of frames in current animation
	int _oAnimFrame; // Current frame of animation.
	int _oAnimWidth;
	int _oAnimWidth2;
	BOOL _oDelFlag;
	char _oBreak; // check
	BOOL _oSolidFlag;
	BOOL _oMissFlag;
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
	int alignment[2];
#endif
} ObjectStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(ObjectStruct) & (sizeof(ObjectStruct) - 1)) == 0, "Align ObjectStruct to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// portal
//////////////////////////////////////////////////

typedef struct PortalStruct {
	BOOL open;
	int x;
	int y;
	int level;
	int ltype;
	BOOL setlvl;
} PortalStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(PortalStruct) & (sizeof(PortalStruct) - 1)) == 16, "Align PortalStruct closer to power of 2 for better performance.");
#endif

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

typedef struct TCmdLocParam1 {
	BYTE bCmd;
	BYTE x;
	BYTE y;
	WORD wParam1;
} TCmdLocParam1;

typedef struct TCmdLocParam2 {
	BYTE bCmd;
	BYTE x;
	BYTE y;
	WORD wParam1;
	WORD wParam2;
} TCmdLocParam2;

typedef struct TCmdLocBParam3 {
	BYTE bCmd;
	BYTE x;
	BYTE y;
	BYTE bParam1;
	BYTE bParam2;
	BYTE bParam3;
} TCmdLocBParam3;

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

typedef struct TCmdWBParam4 {
	BYTE bCmd;
	WORD wParam1;
	BYTE bParam2;
	BYTE bParam3;
	BYTE bParam4;
} TCmdWBParam4;

typedef struct TCmdBParam1 {
	BYTE bCmd;
	BYTE bParam1;
} TCmdBParam1;

typedef struct TCmdGolem {
	BYTE bCmd;
	BYTE _mx;
	BYTE _my;
	BYTE _mdir;
	char _menemy;
	int _mhitpoints;
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
	WORD wIndx;
	WORD wCI;
	int dwSeed;
	BYTE bId;
	BYTE bDur;
	BYTE bMDur;
	BYTE bCh;
	BYTE bMCh;
	WORD wValue;
	DWORD dwBuff;
	int dwTime;
#ifdef HELLFIRE
	WORD wToHit;
	WORD wMaxDam;
	BYTE bMinStr;
	BYTE bMinMag;
	BYTE bMinDex;
	BYTE bAC;
#endif
} TCmdGItem;

typedef struct TCmdPItem {
	BYTE bCmd;
	BYTE x;
	BYTE y;
	WORD wIndx;
	WORD wCI;
	int dwSeed;
	BYTE bId;
	BYTE bDur;
	BYTE bMDur;
	BYTE bCh;
	BYTE bMCh;
	WORD wValue;
	DWORD dwBuff;
#ifdef HELLFIRE
	WORD wToHit;
	WORD wMaxDam;
	BYTE bMinStr;
	BYTE bMinMag;
	BYTE bMinDex;
	BYTE bAC;
#endif
} TCmdPItem;

typedef struct TCmdChItem {
	BYTE bCmd;
	BYTE bLoc;
	WORD wIndx;
	WORD wCI;
	int dwSeed;
	BOOLEAN bId;
} TCmdChItem;

typedef struct TCmdDelItem {
	BYTE bCmd;
	BYTE bLoc;
} TCmdDelItem;

typedef struct TCmdDwParam2 {
	BYTE bCmd;
	DWORD dwParam1;
	DWORD dwParam2;
} TCmdDwParam2;

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
	BYTE bObjId;
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
#endif
} TSyncHeader;

typedef struct TSyncMonster {
	BYTE _mndx;
	BYTE _mx;
	BYTE _my;
	BYTE _menemy;
	BYTE _mdelta;
} TSyncMonster;

typedef struct TPktHdr {
	BYTE px;
	BYTE py;
	BYTE targx;
	BYTE targy;
	int php;
	int pmhp;
	BYTE bstr;
	BYTE bmag;
	BYTE bdex;
	WORD wCheck;
	WORD wLen;
} TPktHdr;

typedef struct TPkt {
	TPktHdr hdr;
	BYTE body[493];
} TPkt;

typedef struct DMonsterStr {
	BYTE _mx;
	BYTE _my;
	BYTE _mdir;
	BYTE _menemy;
	BYTE _mactive;
	int _mhitpoints;
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
	BYTE automapsv[DMAXX][DMAXY];
} LocalLevel;

typedef struct DPortal {
	BYTE x;
	BYTE y;
	BYTE level;
	BYTE ltype;
	BYTE setlvl;
} DPortal;

typedef struct MultiQuests {
	BYTE qstate;
	BYTE qlog;
	BYTE qvar1;
} MultiQuests;

typedef struct DJunk {
	DPortal portal[MAXPORTAL];
	MultiQuests quests[MAXMULTIQUESTS];
} DJunk;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct TMegaPkt {
	struct TMegaPkt *pNext;
	DWORD dwSpaceLeft;
	BYTE data[32000];
} TMegaPkt;
#pragma pack(pop)

typedef struct TBuffer {
	DWORD dwNextWriteOffset;
	BYTE bData[4096];
} TBuffer;

//////////////////////////////////////////////////
// quests
//////////////////////////////////////////////////

typedef struct QuestStruct {
	unsigned char _qlevel;
	unsigned char _qtype;
	unsigned char _qactive;
	int _qtx;
	int _qty;
	unsigned char _qslvl;
	unsigned char _qidx;
	unsigned int _qmsg;
	unsigned char _qvar1;
	unsigned char _qvar2;
	BOOL _qlog;
#ifdef X86_32bit_COMP
	int alignment[1];
#endif
} QuestStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(QuestStruct) & (sizeof(QuestStruct) - 1)) == 0, "Align QuestStruct to power of 2 for better performance.");
#endif

typedef struct QuestData {
	unsigned char _qdlvl;
	char _qdmultlvl;
	unsigned char _qdtype;
	unsigned char _qdrnd;
	unsigned char _qslvl;
	int _qflags; /* unsigned char */
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
	void (*fnMenu)(BOOL); /* fix, should have one arg */
} TMenuItem;

// TPDEF PTR FCN VOID TMenuUpdateFcn

//////////////////////////////////////////////////
// spells
//////////////////////////////////////////////////

typedef struct SpellData {
	unsigned char sName;
	unsigned char sManaCost;
	unsigned char sType;
	const char *sNameText;
	char sBookLvl;
	char sStaffLvl;
	char sScrollLvl;
	BOOLEAN sTargeted;
	BYTE scCurs; // cursor for scrolls/runes
	BYTE spCurs; // cursor for spells
	BOOLEAN sTownSpell;
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
	int _tmode;
	int _ttype;
	int _tx;    // Tile X-position of NPC
	int _ty;    // Tile Y-position of NPC
	int _txoff; // Sprite X-offset (unused)
	int _tyoff; // Sprite Y-offset (unused)
	int _txvel; // X-velocity during movement (unused)
	int _tyvel; // Y-velocity during movement (unused)
	int _tdir;  // Facing of NPC (unused)
	unsigned char *_tAnimData;
	int _tAnimDelay; // Tick length of each frame in the current animation
	int _tAnimCnt;   // Increases by one each game tick, counting how close we are to _pAnimDelay
	int _tAnimLen;   // Number of frames in current animation
	int _tAnimFrame; // Current frame of animation.
	int _tAnimFrameCnt;
	char _tAnimOrder;
	int _tAnimWidth;
	int _tAnimWidth2;
	int _tTenPer;
	int _teflag;
	int _tbtcnt;
	int _tSelFlag;
	BOOL _tMsgSaid;
	TNQ qsts[MAXQUESTS];
	int _tSeed;
	int _tVar1;
	int _tVar2;
	int _tVar3;
	int _tVar4;
	char _tName[PLR_NAME_LEN];
	unsigned char *_tNAnim[8];
	int _tNFrames;
	unsigned char *_tNData;
#ifdef X86_32bit_COMP
#ifndef HELLFIRE
	int alignment[6];
#endif
#endif
} TownerStruct;
#ifdef X86_32bit_COMP
static_assert((sizeof(TownerStruct) & (sizeof(TownerStruct) - 1)) == 0, "Align TownerStruct to power of 2 for better performance.");
#endif

typedef struct QuestTalkData {
	int _qinfra;
	int _qblkm;
	int _qgarb;
	int _qzhar;
	int _qveil;
	int _qmod;
	int _qbutch;
	int _qbol;
	int _qblind;
	int _qblood;
	int _qanvil;
	int _qwarlrd;
	int _qking;
	int _qpw;
	int _qbone;
	int _qvb;
#ifdef HELLFIRE
	int _qgrv;
	int _qfarm;
	int _qgirl;
	int _qtrade;
	int _qdefiler;
	int _qnakrul;
	int _qjersy;
	int _qhf8;
#endif
} QuestTalkData;

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

typedef struct HALLNODE {
	int nHallx1;
	int nHally1;
	int nHallx2;
	int nHally2;
	int nHalldir;
	struct HALLNODE *pNext;
} HALLNODE;

typedef struct ROOMNODE {
	int nRoomx1;
	int nRoomy1;
	int nRoomx2;
	int nRoomy2;
	int nRoomDest;
} ROOMNODE;

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
	int _lradius;
	int _lid;
	int _ldel;
	int _lunflag;
	int _lunx;
	int _luny;
	int _lunr;
	int _xoff;
	int _yoff;
	int _lflags;
} LightListStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(LightListStruct) & (sizeof(LightListStruct) - 1)) == 32, "Align LightListStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// dead
//////////////////////////////////////////////////

typedef struct DeadStruct {
	unsigned char *_deadData[8];
	int _deadFrame;
	int _deadWidth;
	int _deadWidth2;
	char _deadtrans;
} DeadStruct;

#ifdef X86_32bit_COMP
static_assert((sizeof(DeadStruct) & (sizeof(DeadStruct) - 1)) == 32, "Align DeadStruct closer to power of 2 for better performance.");
#endif

//////////////////////////////////////////////////
// diabloui
//////////////////////////////////////////////////

// TPDEF PTR FCN VOID PLAYSND

typedef struct _gamedata {
	int dwSeed;
	BYTE bDiff;
	BYTE bRate;
} _gamedata;

typedef struct _uidefaultstats {
	WORD strength;
	WORD magic;
	WORD dexterity;
	WORD vitality;
} _uidefaultstats;

typedef struct _uiheroinfo {
	struct _uiheroinfo *next;
	char name[16];
	WORD level;
	BYTE heroclass;
	BYTE herorank;
	WORD strength;
	WORD magic;
	WORD dexterity;
	WORD vitality;
	int gold;
	int hassaved;
} _uiheroinfo;

// TPDEF PTR FCN UCHAR ENUMHEROPROC
// TPDEF PTR FCN UCHAR ENUMHEROS
// TPDEF PTR FCN UCHAR CREATEHERO
// TPDEF PTR FCN UCHAR DELETEHERO
// TPDEF PTR FCN UCHAR GETDEFHERO

// TPDEF PTR FCN INT PROGRESSFCN

//////////////////////////////////////////////////
// storm
//////////////////////////////////////////////////

// TPDEF PTR FCN UCHAR SMSGIDLEPROC
// TPDEF PTR FCN VOID SMSGHANDLER

typedef struct _SNETCAPS {
	DWORD size;
	DWORD flags;
	DWORD maxmessagesize;
	DWORD maxqueuesize;
	DWORD maxplayers;
	DWORD bytessec;
	DWORD latencyms;
	DWORD defaultturnssec;
	DWORD defaultturnsintransit;
} _SNETCAPS;

typedef struct _SNETEVENT {
	DWORD eventid;
	DWORD playerid;
	void *data;
	DWORD databytes;
} _SNETEVENT;

// TPDEF PTR FCN UCHAR SNETABORTPROC
// TPDEF PTR FCN UCHAR SNETCATEGORYPROC
// TPDEF PTR FCN UCHAR SNETCHECKAUTHPROC
// TPDEF PTR FCN UCHAR SNETCREATEPROC
// TPDEF PTR FCN UCHAR SNETDRAWDESCPROC
// TPDEF PTR FCN UCHAR SNETENUMDEVICESPROC
// TPDEF PTR FCN UCHAR SNETENUMGAMESPROC
// TPDEF PTR FCN UCHAR SNETENUMPROVIDERSPROC
// TPDEF PTR FCN VOID SNETEVENTPROC
// TPDEF PTR FCN UCHAR SNETGETARTPROC
// TPDEF PTR FCN UCHAR SNETGETDATAPROC
// TPDEF PTR FCN INT SNETMESSAGEBOXPROC
// TPDEF PTR FCN UCHAR SNETPLAYSOUNDPROC
// TPDEF PTR FCN UCHAR SNETSELECTEDPROC
// TPDEF PTR FCN UCHAR SNETSTATUSPROC

typedef struct _SNETPLAYERDATA {
	int size;
	char *playername;
	char *playerdescription;
	int reserved;
} _SNETPLAYERDATA;

typedef struct _SNETPROGRAMDATA {
	int size;
	const char *programname;
	const char *programdescription;
	int programid;
	int versionid;
	int reserved1;
	int maxplayers;
	_gamedata *initdata;
	int initdatabytes;
	void *reserved2;
	int optcategorybits;
	char *cdkey;
	char *registereduser;
	int spawned;
	int lcid;
} _SNETPROGRAMDATA;

typedef struct _SNETVERSIONDATA {
	int size;
	const char *versionstring;
	const char *executablefile;
	const char *originalarchivefile;
	const char *patcharchivefile;
} _SNETVERSIONDATA;

typedef struct _SNETUIDATA {
	int size;
	int uiflags;
	HWND parentwindow;
	void (*artcallback)();
	void (*authcallback)();
	void (*createcallback)();
	void (*drawdesccallback)();
	void (*selectedcallback)();
	void (*messageboxcallback)();
	void (*soundcallback)();
	void (*statuscallback)();
	void (*getdatacallback)();
	void (*categorycallback)();
	void (*categorylistcallback)();
	void (*newaccountcallback)();
	void (*profilecallback)();
	const char **profilefields;
	void (*profilebitmapcallback)();
	int (*selectnamecallback)(
	    const struct _SNETPROGRAMDATA *,
	    const struct _SNETPLAYERDATA *,
	    const struct _SNETUIDATA *,
	    const struct _SNETVERSIONDATA *,
	    DWORD provider, /* e.g. 'BNET', 'IPXN', 'MODM', 'SCBL' */
	    char *, DWORD,  /* character name will be copied here */
	    char *, DWORD,  /* character "description" will be copied here (used to advertise games) */
	    BOOL *          /* new character? - unsure about this */
	);
	void (*changenamecallback)();
} _SNETUIDATA;

// TPDEF PTR FCN UCHAR SNETSPIBIND
// TPDEF PTR FCN UCHAR SNETSPIQUERY

//////////////////////////////////////////////////
// pack
//////////////////////////////////////////////////

#pragma pack(push, 1)
typedef struct PkItemStruct {
	DWORD iSeed;
	WORD iCreateInfo;
	WORD idx;
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
	char destAction;
	char destParam1;
	char destParam2;
	BYTE plrlevel;
	BYTE px;
	BYTE py;
	BYTE targx;
	BYTE targy;
	char pName[PLR_NAME_LEN];
	char pClass;
	BYTE pBaseStr;
	BYTE pBaseMag;
	BYTE pBaseDex;
	BYTE pBaseVit;
	char pLevel;
	BYTE pStatPts;
	int pExperience;
	int pGold;
	int pHPBase;
	int pMaxHPBase;
	int pManaBase;
	int pMaxManaBase;
	char pSplLvl[37]; // Should be MAX_SPELLS but set to 37 to make save games compatible
	uint64_t pMemSpells;
	PkItemStruct InvBody[NUM_INVLOC];
	PkItemStruct InvList[NUM_INV_GRID_ELEM];
	char InvGrid[NUM_INV_GRID_ELEM];
	BYTE _pNumInv;
	PkItemStruct SpdList[MAXBELTITEMS];
	char bReserved0[3];
	char pBattleNet;
	BOOLEAN pManaShield;
	char bReserved1[7];
	char pSplLvl2[10]; // Hellfire spells
	short wReserved8;
	DWORD pDiabloKillLevel;
	char pSplHotKey[4];
	char pSplTHotKey[4];
#if INT_MAX == INT64_MAX
	int dwReserved0;
#endif
	int dwReserved1[5];
} PkPlayerStruct;
#pragma pack(pop)

//////////////////////////////////////////////////
// path
//////////////////////////////////////////////////

typedef struct PATHNODE {
	char f;
	char h;
	char g;
	int x;
	int y;
	struct PATHNODE *Parent;
	struct PATHNODE *Child[8];
	struct PATHNODE *NextNode;
} PATHNODE;

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

#pragma pack(push, 1)
typedef struct TMsg TMsg;

typedef struct TMsgHdr {
	TMsg *pNext;
	int dwTime;
	BYTE bLen;
} TMsgHdr;

typedef struct TMsg {
	TMsgHdr hdr;
	// this is actually alignment padding, but the message body is appended to the struct
	// so it's convenient to use byte-alignment and name it "body"
	unsigned char body[3];
} TMsg;
#pragma pack(pop)

//////////////////////////////////////////////////
// mpqapi
//////////////////////////////////////////////////

typedef struct _FILEHEADER {
	int signature;
	int headersize;
	int filesize;
	WORD version;
	short sectorsizeid;
	int hashoffset;
	int blockoffset;
	int hashcount;
	int blockcount;
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
	BOOL _sjust;
	char _sclr;
	int _sline;
	BOOL _ssel;
	int _sval;
} STextStruct;

//////////////////////////////////////////////////
// wave
//////////////////////////////////////////////////

typedef struct MEMFILE {
	DWORD end;
	LONG offset;
	DWORD buf_len;
	DWORD dist;
	DWORD bytes_to_read;
	BYTE *buf;
	HANDLE file;
} MEMFILE;

//////////////////////////////////////////////////
// plrmsg
//////////////////////////////////////////////////

typedef struct _plrmsg {
	DWORD time;
	unsigned char player;
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
