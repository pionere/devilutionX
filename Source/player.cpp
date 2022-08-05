/**
 * @file player.cpp
 *
 * Implementation of player functionality, leveling, actions, creation, loading, etc.
 */
#include "all.h"
#include "plrctrls.h"

DEVILUTION_BEGIN_NAMESPACE

int mypnum;
PlayerStruct players[MAX_PLRS];
/* Counter to suppress animations in case the current player is changing the level. */
BYTE gbLvlLoad;
/** The current player while processing the players. */
BYTE gbGameLogicPnum;
bool _gbPlrGfxSizeLoaded = false;
int plr_lframe_size;
int plr_wframe_size;
int plr_aframe_size;
int plr_fframe_size;
int plr_qframe_size;
int plr_hframe_size;
int plr_bframe_size;
int plr_sframe_size;
int plr_dframe_size;

/** Maps from armor animation to letter used in graphic files. */
const char ArmourChar[4] = {
	'L', // light
	'M', // medium
	'H', // heavy
	0
};
/** Maps from weapon animation to letter used in graphic files. */
const char WepChar[10] = {
	'N', // unarmed
	'U', // no weapon + shield
	'S', // sword + no shield
	'D', // sword + shield
	'B', // bow
	'A', // axe
	'M', // blunt + no shield
	'H', // blunt + shield
	'T', // staff
	0
};
/** Maps from player class to letter used in graphic files. */
const char CharChar[NUM_CLASSES] = { 'W', 'R', 'S',
#ifdef HELLFIRE
//	'M', 'B', 'C'
	'M', 'R', 'W'
#endif
};
/** Maps from player class to the folder of graphic files. */
const char *const gfxClassTbl[NUM_CLASSES] = { "Warrior", "Rogue", "Sorceror",
#ifdef HELLFIRE
//	"Monk", "Bard", "Barbarian"
	"Monk", "Rogue", "Warrior"
#endif
};
/** Maps from player class to the string shown to the player. */
const char *const ClassStrTbl[NUM_CLASSES] = { "Warrior", "Rogue", "Sorceror",
#ifdef HELLFIRE
	"Monk", "Bard", "Barbarian"
#endif
};

/*
 * Specifies the X and Y offsets to try when a player is entering the level or resurrected.
 * The base position is the location of the portal or the body of the dead player.
 */
const int plrxoff2[NUM_DIRS + 1] = { 0, 1, 1, 0, -1, 0, -1, 1, -1 };
const int plryoff2[NUM_DIRS + 1] = { 0, 1, 0, 1, -1, -1, 0, -1, 1 };
/**
 * Specifies the number of frames of each animation for each player class.
   STAND, ATTACK, WALK, BLOCK, DEATH, SPELL, GOTHIT
 */
const BYTE PlrGFXAnimLens[NUM_CLASSES][NUM_PLR_ANIMS] = {
	// clang-format off
	{ 10, 16, 8, 2, 20, 20, 6 },
	{  8, 18, 8, 4, 20, 16, 7 },
	{  8, 16, 8, 6, 20, 12, 8 },
#ifdef HELLFIRE
	{  8, 16, 8, 3, 20, 18, 6 },
	{  8, 18, 8, 4, 20, 16, 7 },
	{ 10, 16, 8, 2, 20, 20, 6 },
#endif
	// clang-format on
};
/** Specifies the frame of attack and spell animation for which the action is triggered, for each player class. */
const BYTE PlrGFXAnimActFrames[NUM_CLASSES][2] = {
	// clang-format off
	{  9, 14 },
	{ 10, 12 },
	{ 12,  8 },
#ifdef HELLFIRE
	{ 12, 13 },
	{ 10, 12 },
	{  9, 14 },
#endif
	// clang-format on
};
/** Specifies the length of a frame for each animation. */
const BYTE PlrAnimFrameLens[NUM_PLR_ANIMS] = { 4, 1, 1, 3, 2, 1, 1 };

/** Maps from player_class to starting stat in strength. */
const int StrengthTbl[NUM_CLASSES] = {
	// clang-format off
	20,
	15,
	10,
#ifdef HELLFIRE
	20,
	15,
	35,
#endif
	// clang-format on
};
/** Maps from player_class to starting stat in magic. */
const int MagicTbl[NUM_CLASSES] = {
	// clang-format off
	10,
	20,
	30,
#ifdef HELLFIRE
	15,
	20,
	 0,
#endif
	// clang-format on
};
/** Maps from player_class to starting stat in dexterity. */
const int DexterityTbl[NUM_CLASSES] = {
	// clang-format off
	20,
	25,
	20,
#ifdef HELLFIRE
	20,
	25,
	10,
#endif
	// clang-format on
};
/** Maps from player_class to starting stat in vitality. */
const int VitalityTbl[NUM_CLASSES] = {
	// clang-format off
	30,
	20,
	20,
#ifdef HELLFIRE
	25,
	20,
	35,
#endif
	// clang-format on
};
const BYTE Abilities[NUM_CLASSES] = {
	SPL_REPAIR, SPL_DISARM, SPL_RECHARGE
#ifdef HELLFIRE
	, SPL_WHITTLE, SPL_IDENTIFY, SPL_BUCKLE
#endif
};

/** Specifies the experience point limit of each player level. */
const unsigned PlrExpLvlsTbl[MAXCHARLEVEL + 1] = {
	0,
	2000,
	4620,
	8040,
	12489,
	18258,
	25712,
	35309,
	47622,
	63364,
	83419,
	108879,
	141086,
	181683,
	231075,
	313656,
	424067,
	571190,
	766569,
	1025154,
	1366227,
	1814568,
	2401895,
	3168651,
	4166200,
	5459523,
	7130496,
	9281874,
	12042092,
	15571031,
	20066900,
	25774405,
	32994399,
	42095202,
	53525811,
	67831218,
	85670061,
	107834823,
	135274799,
	169122009,
	210720231,
	261657253,
	323800420,
	399335440,
	490808349,
	601170414,
	733825617,
	892680222,
	1082908612,
	1310707109,
	1583495809
};

/** Specifies the experience point limit of skill-level. */
const unsigned SkillExpLvlsTbl[MAXSPLLEVEL + 1] = {
	8040,
	25712,
	63364,
	141086,
	313656,
	766569,
	1814568,
	4166200,
	9281874,
	20066900,
	42095202,
	85670061,
	169122009,
	323800420,
	601170414,
	1082908612,
};

#if DEBUG_MODE
/** Maps from facing direction to scroll-direction. */
static const char dir2sdir[NUM_DIRS] = { SDIR_S, SDIR_SW, SDIR_W, SDIR_NW, SDIR_N, SDIR_NE, SDIR_E, SDIR_SE };
#endif

static void SetPlayerGPtrs(BYTE *pData, BYTE *(&pAnim)[8])
{
	int i;

	for (i = 0; i < lengthof(pAnim); i++) {
		pAnim[i] = const_cast<BYTE *>(CelGetFrameStart(pData, i));
	}
}

static inline void GetPlrGFXCells(int pc, const char **szCel, const char **cs)
{
/*#ifdef HELLFIRE
	if (pc == PC_BARD && diabdat_mpqs[MPQ_HF_BARD] == NULL)
		pc = PC_ROGUE;
	else if (pc == PC_BARBARIAN && diabdat_mpqs[MPQ_HF_BARB] == NULL)
		pc = PC_WARRIOR;
#endif*/
	*szCel = &CharChar[pc];
	*cs = gfxClassTbl[pc];
}

void LoadPlrGFX(int pnum, unsigned gfxflag)
{
	char prefix[16];
	char pszName[DATA_ARCHIVE_MAX_PATH];
	const char *szCel, *cs;
	BYTE *pData;
	BYTE *(*pAnim)[8];
	unsigned i, mask;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("LoadPlrGFX: illegal player %d", pnum);
	}

	GetPlrGFXCells(plr._pClass, &szCel, &cs);
	snprintf(prefix, sizeof(prefix), "%c%c%c", *szCel, ArmourChar[plr._pgfxnum >> 4], WepChar[plr._pgfxnum & 0xF]);

	for (i = 0, mask = gfxflag; i < NUM_PFIDXs; i++, mask >>= 1) {
		if (!(mask & 1))
			continue;

		switch (i) {
		case PFIDX_STAND:
			szCel = currLvl._dType != DTYPE_TOWN ? "AS" : "ST";
			pData = plr._pNData;
			pAnim = &plr._pNAnim;
			break;
		case PFIDX_WALK:
			szCel = currLvl._dType != DTYPE_TOWN ? "AW" : "WL";
			pData = plr._pWData;
			pAnim = &plr._pWAnim;
			break;
		case PFIDX_ATTACK:
			szCel = "AT";
			pData = plr._pAData;
			pAnim = &plr._pAAnim;
			break;
		case PFIDX_HIT:
			szCel = "HT";
			pData = plr._pHData;
			pAnim = &plr._pHAnim;
			break;
		case PFIDX_LIGHTNING:
			szCel = "LM";
			pData = plr._pLData;
			pAnim = &plr._pLAnim;
			break;
		case PFIDX_FIRE:
			szCel = "FM";
			pData = plr._pFData;
			pAnim = &plr._pFAnim;
			break;
		case PFIDX_MAGIC:
			szCel = "QM";
			pData = plr._pTData;
			pAnim = &plr._pTAnim;
			break;
		case PFIDX_DEATH:
			assert((plr._pgfxnum & 0xF) == ANIM_ID_UNARMED);
			szCel = "DT";
			pData = plr._pDData;
			pAnim = &plr._pDAnim;
			break;
		case PFIDX_BLOCK:
			szCel = "BL";
			pData = plr._pBData;
			pAnim = &plr._pBAnim;
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}

		snprintf(pszName, sizeof(pszName), "PlrGFX\\%s\\%s\\%s%s.CL2", cs, prefix, prefix, szCel);
		LoadFileWithMem(pszName, pData);
		SetPlayerGPtrs(pData, *pAnim);
		plr._pGFXLoad |= 1 << i;
	}
}

void InitPlayerGFX(int pnum)
{
	unsigned gfxflag;
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("InitPlayerGFX: illegal player %d", pnum);
	}
	plr._pGFXLoad = 0;
	if (plr._pHitPoints < (1 << 6)) {
		plr._pgfxnum = ANIM_ID_UNARMED;
		gfxflag = PFILE_DEATH;
	} else {
		gfxflag = PFILE_NONDEATH;
		if (currLvl._dType == DTYPE_TOWN)
			gfxflag &= ~(PFILE_ATTACK | PFILE_HIT | PFILE_BLOCK);
		else if (!(plr._pSkillFlags & SFLAG_BLOCK))
			gfxflag &= ~PFILE_BLOCK;
	}
	LoadPlrGFX(pnum, gfxflag);
}

static unsigned GetPlrGFXSize(const char *szCel)
{
	int c;
	const char *a, *w, *cc, *cst;
	DWORD dwSize, dwMaxSize;
	HANDLE hsFile;
	char pszName[DATA_ARCHIVE_MAX_PATH];
	char Type[16];

	dwMaxSize = 0;

	for (c = 0; c < NUM_CLASSES; c++) {
		GetPlrGFXCells(c, &cc, &cst);
		for (a = &ArmourChar[0]; *a != '\0'; a++) {
			for (w = &WepChar[0]; *w != '\0'; w++) { // BUGFIX loads non-existing animations; DT is only for N, BL is only for U, D & H (fixed)
				if (szCel[0] == 'D' /*&& szCel[1] == 'T'*/ && *a != 'L' && *w != 'N') {
					continue; //Death has no weapon or armor
				}
				/* BUGFIX monks can block unarmed and without shield (fixed)
				if (szCel[0] == 'B' && szCel[1] == 'L' && (*w != 'U' && *w != 'D' && *w != 'H')) {
					continue; //No block without weapon
				}*/
				snprintf(Type, sizeof(Type), "%c%c%c", *cc, *a, *w);
				snprintf(pszName, sizeof(pszName), "PlrGFX\\%s\\%s\\%s%s.CL2", cst, Type, Type, szCel);
				hsFile = SFileOpenFile(pszName);
				if (hsFile != NULL) {
					dwSize = SFileGetFileSize(hsFile);
					SFileCloseFile(hsFile);
					if (dwMaxSize < dwSize) {
						dwMaxSize = dwSize;
					}
				}
			}
		}
	}

	return dwMaxSize;
}

void InitPlrGFXMem(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("InitPlrGFXMem: illegal player %d", pnum);
	}

	if (!_gbPlrGfxSizeLoaded) {
		_gbPlrGfxSizeLoaded = true;

		// STAND -- ST: TOWN, AS: DUNGEON
		plr_sframe_size = std::max(GetPlrGFXSize("ST"), GetPlrGFXSize("AS"));
		// WALK -- WL: TOWN, AW: DUNGEON
		plr_wframe_size = std::max(GetPlrGFXSize("WL"), GetPlrGFXSize("AW"));
		// ATTACK
		plr_aframe_size = GetPlrGFXSize("AT");
		// HIT
		plr_hframe_size = GetPlrGFXSize("HT");
		// LIGHTNING
		plr_lframe_size = GetPlrGFXSize("LM");
		// FIRE
		plr_fframe_size = GetPlrGFXSize("FM");
		// MAGIC
		plr_qframe_size = GetPlrGFXSize("QM");
		// DEATH
		plr_dframe_size = GetPlrGFXSize("DT");
		// BLOCK
		plr_bframe_size = GetPlrGFXSize("BL");
	}
	assert(plr._pNData == NULL);
	plr._pNData = DiabloAllocPtr(plr_sframe_size);
	assert(plr._pWData == NULL);
	plr._pWData = DiabloAllocPtr(plr_wframe_size);
	assert(plr._pAData == NULL);
	plr._pAData = DiabloAllocPtr(plr_aframe_size);
	assert(plr._pHData == NULL);
	plr._pHData = DiabloAllocPtr(plr_hframe_size);
	assert(plr._pLData == NULL);
	plr._pLData = DiabloAllocPtr(plr_lframe_size);
	assert(plr._pFData == NULL);
	plr._pFData = DiabloAllocPtr(plr_fframe_size);
	assert(plr._pTData == NULL);
	plr._pTData = DiabloAllocPtr(plr_qframe_size);
	assert(plr._pDData == NULL);
	plr._pDData = DiabloAllocPtr(plr_dframe_size);
	assert(plr._pBData == NULL);
	plr._pBData = DiabloAllocPtr(plr_bframe_size);

	plr._pGFXLoad = 0;
}

void FreePlayerGFX(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("FreePlayerGFX: illegal player %d", pnum);
	}

	MemFreeDbg(plr._pNData);
	MemFreeDbg(plr._pWData);
	MemFreeDbg(plr._pAData);
	MemFreeDbg(plr._pHData);
	MemFreeDbg(plr._pLData);
	MemFreeDbg(plr._pFData);
	MemFreeDbg(plr._pTData);
	MemFreeDbg(plr._pDData);
	MemFreeDbg(plr._pBData);
	plr._pGFXLoad = 0;
}

/**
 * @brief Sets the new Player Animation with all relevant information for rendering

 * @param pnum Player Id
 * @param anims Pointer to Animation Data
 * @param dir the direction of the player
 * @param numFrames Number of Frames in Animation
 * @param frameLen the length of a single animation frame
 * @param width Width of sprite
*/
static void NewPlrAnim(int pnum, BYTE **anims, int dir, unsigned numFrames, int frameLen, int width) //, int numSkippedFrames /*= 0*/, bool processAnimationPending /*= false*/, int stopDistributingAfterFrame /*= 0*/)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("NewPlrAnim: illegal player %d", pnum);
	}
	plr._pdir = dir;
	plr._pAnimData = anims[dir];
	plr._pAnimLen = numFrames;
	plr._pAnimFrame = 1;
	plr._pAnimCnt = (gbGameLogicProgress < GLP_PLAYERS_DONE && gbGameLogicPnum <= pnum) ? -1 : 0;
	plr._pAnimFrameLen = frameLen;
	plr._pAnimWidth = width;
	plr._pAnimXOffset = (width - TILE_WIDTH) >> 1;
}

/*static void ClearPlrPVars(int pnum)
{
	/ *if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("ClearPlrPVars: illegal player %d", pnum);
	}
	plr._pVar1 = 0;
	plr._pVar2 = 0;
	plr._pVar3 = 0;
	plr._pVar4 = 0;
	plr._pVar5 = 0;
	plr._pVar6 = 0;
	plr._pVar7 = 0;
	plr._pVar8 = 0;* /
}*/

void SetPlrAnims(int pnum)
{
	int pc, gn;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("SetPlrAnims: illegal player %d", pnum);
	}
	plr._pNWidth = 96 * ASSET_MPL;
	plr._pWWidth = 96 * ASSET_MPL;
	plr._pAWidth = 128 * ASSET_MPL;
	plr._pHWidth = 96 * ASSET_MPL;
	plr._pSWidth = 96 * ASSET_MPL;
	plr._pDWidth = 128 * ASSET_MPL;
	plr._pBWidth = 96 * ASSET_MPL;

	pc = plr._pClass;
	plr._pAFNum = PlrGFXAnimActFrames[pc][0];
	plr._pSFNum = PlrGFXAnimActFrames[pc][1];

	plr._pNFrames = PlrGFXAnimLens[pc][PA_STAND];
	plr._pAFrames = PlrGFXAnimLens[pc][PA_ATTACK];
	plr._pWFrames = PlrGFXAnimLens[pc][PA_WALK];
	plr._pBFrames = PlrGFXAnimLens[pc][PA_BLOCK];
	plr._pDFrames = PlrGFXAnimLens[pc][PA_DEATH];
	plr._pSFrames = PlrGFXAnimLens[pc][PA_SPELL];
	plr._pHFrames = PlrGFXAnimLens[pc][PA_GOTHIT];

	gn = plr._pgfxnum & 0xF;
	switch (pc) {
	case PC_WARRIOR:
		if (gn == ANIM_ID_BOW) {
			plr._pNFrames = 8;
			plr._pAWidth = 96 * ASSET_MPL;
			plr._pAFNum = 11;
		} else if (gn == ANIM_ID_AXE) {
			plr._pAFrames = 20;
			plr._pAFNum = 10;
		} else if (gn == ANIM_ID_STAFF) {
			// plr._pAFrames = 16;
			plr._pAFNum = 11;
		}
		break;
	case PC_ROGUE:
		if (gn == ANIM_ID_AXE) {
			plr._pAFrames = 22;
			plr._pAFNum = 13;
		} else if (gn == ANIM_ID_BOW) {
			plr._pAFrames = 12;
			plr._pAFNum = 7;
		} else if (gn == ANIM_ID_STAFF) {
			plr._pAFrames = 16;
			plr._pAFNum = 11;
		}
		break;
	case PC_SORCERER:
		plr._pSWidth = 128 * ASSET_MPL;
		if (gn == ANIM_ID_UNARMED) {
			plr._pAFrames = 20;
		} else if (gn == ANIM_ID_UNARMED_SHIELD) {
			plr._pAFNum = 9;
		} else if (gn == ANIM_ID_BOW) {
			plr._pAFrames = 20;
			plr._pAFNum = 16;
		} else if (gn == ANIM_ID_AXE) {
			plr._pAFrames = 24;
			plr._pAFNum = 16;
		}
		break;
#ifdef HELLFIRE
	case PC_MONK:
		plr._pNWidth = 112 * ASSET_MPL;
		plr._pWWidth = 112 * ASSET_MPL;
		plr._pAWidth = 130 * ASSET_MPL;
		plr._pHWidth = 98 * ASSET_MPL;
		plr._pSWidth = 114 * ASSET_MPL;
		plr._pDWidth = 160 * ASSET_MPL;
		plr._pBWidth = 98 * ASSET_MPL;

		switch (gn) {
		case ANIM_ID_UNARMED:
		case ANIM_ID_UNARMED_SHIELD:
			plr._pAFrames = 12;
			plr._pAFNum = 7;
			break;
		case ANIM_ID_BOW:
			plr._pAFrames = 20;
			plr._pAFNum = 14;
			break;
		case ANIM_ID_AXE:
			plr._pAFrames = 23;
			plr._pAFNum = 14;
			break;
		case ANIM_ID_STAFF:
			plr._pAFrames = 13;
			plr._pAFNum = 8;
			break;
		}
		break;
	case PC_BARD:
		if (gn == ANIM_ID_AXE) {
			plr._pAFrames = 22;
			plr._pAFNum = 13;
		} else if (gn == ANIM_ID_BOW) {
			plr._pAFrames = 12;
			plr._pAFNum = 11;
		} else if (gn == ANIM_ID_STAFF) {
			plr._pAFrames = 16;
			plr._pAFNum = 11;
		} else if (gn == ANIM_ID_SWORD_SHIELD || gn == ANIM_ID_SWORD) {
			plr._pAFrames = 10; // TODO: check for onehanded swords or daggers?
		}
		break;
	case PC_BARBARIAN:
		if (gn == ANIM_ID_AXE) {
			plr._pAFrames = 20;
			plr._pAFNum = 8;
		} else if (gn == ANIM_ID_BOW) {
			plr._pNFrames = 8;
			plr._pAWidth = 96 * ASSET_MPL;
			plr._pAFNum = 11;
		} else if (gn == ANIM_ID_STAFF) {
			//plr._pAFrames = 16;
			plr._pAFNum = 11;
		} else if (gn == ANIM_ID_MACE || gn == ANIM_ID_MACE_SHIELD) {
			plr._pAFNum = 8;
		}
		break;
#endif
	default:
		ASSUME_UNREACHABLE
		break;
	}
	if (currLvl._dType == DTYPE_TOWN) {
		plr._pNFrames = 20;
		//plr._pWFrames = 8;
	}
}

/**
 * @param c plr_classes value
 */
void CreatePlayer(const _uiheroinfo &heroinfo)
{
	int val, hp, mana;
	int i, pnum = 0;

	memset(&plr, 0, sizeof(PlayerStruct));
	//SetRndSeed(SDL_GetTicks());

	plr._pLevel = heroinfo.hiLevel;
	plr._pClass = heroinfo.hiClass;
	//plr._pRank = heroinfo.hiRank;
	copy_cstr(plr._pName, heroinfo.hiName);

	val = heroinfo.hiStrength;
	//plr._pStrength = val;
	plr._pBaseStr = val;

	val = heroinfo.hiDexterity;
	//plr._pDexterity = val;
	plr._pBaseDex = val;

	val = heroinfo.hiVitality;
	//plr._pVitality = val;
	plr._pBaseVit = val;

	hp = val << (6 + 1);
	/*plr._pHitPoints = plr._pMaxHP = */plr._pHPBase = plr._pMaxHPBase = hp;

	val = heroinfo.hiMagic;
	//plr._pMagic = val;
	plr._pBaseMag = val;

	mana = val << (6 + 1);
	/*plr._pMana = plr._pMaxMana = */plr._pManaBase = plr._pMaxManaBase = mana;

	//plr._pNextExper = PlrExpLvlsTbl[1];
	plr._pLightRad = 10;

	//plr._pAblSkills = SPELL_MASK(Abilities[c]);
	//plr._pAblSkills |= SPELL_MASK(SPL_WALK) | SPELL_MASK(SPL_ATTACK) | SPELL_MASK(SPL_RATTACK) | SPELL_MASK(SPL_BLOCK);

	//plr._pAtkSkill = SPL_ATTACK;
	//plr._pAtkSkillType = RSPLTYPE_ABILITY;
	//plr._pMoveSkill = SPL_WALK;
	//plr._pMoveSkillType = RSPLTYPE_ABILITY;
	//plr._pAltAtkSkill = SPL_INVALID;
	//plr._pAltAtkSkillType = RSPLTYPE_INVALID;
	//plr._pAltMoveSkill = SPL_INVALID;
	//plr._pAltMoveSkillType = RSPLTYPE_INVALID;

	for (i = 0; i < lengthof(plr._pAtkSkillHotKey); i++)
		plr._pAtkSkillHotKey[i] = SPL_INVALID;
	for (i = 0; i < lengthof(plr._pAtkSkillTypeHotKey); i++)
		plr._pAtkSkillTypeHotKey[i] = RSPLTYPE_INVALID;
	for (i = 0; i < lengthof(plr._pMoveSkillHotKey); i++)
		plr._pMoveSkillHotKey[i] = SPL_INVALID;
	for (i = 0; i < lengthof(plr._pMoveSkillTypeHotKey); i++)
		plr._pMoveSkillTypeHotKey[i] = RSPLTYPE_INVALID;
	for (i = 0; i < lengthof(plr._pAltAtkSkillHotKey); i++)
		plr._pAltAtkSkillHotKey[i] = SPL_INVALID;
	for (i = 0; i < lengthof(plr._pAltAtkSkillTypeHotKey); i++)
		plr._pAltAtkSkillTypeHotKey[i] = RSPLTYPE_INVALID;
	for (i = 0; i < lengthof(plr._pAltMoveSkillHotKey); i++)
		plr._pAltMoveSkillHotKey[i] = SPL_INVALID;
	for (i = 0; i < lengthof(plr._pAltMoveSkillTypeHotKey); i++)
		plr._pAltMoveSkillTypeHotKey[i] = RSPLTYPE_INVALID;

	if (plr._pClass == PC_SORCERER) {
		plr._pSkillLvlBase[SPL_FIREBOLT] = 2;
		plr._pSkillExp[SPL_FIREBOLT] = SkillExpLvlsTbl[1];
		plr._pMemSkills = SPELL_MASK(SPL_FIREBOLT);
	}

	CreatePlrItems(pnum);

	// TODO: at the moment player is created and right after that unpack is called
	//  this makes the two calls below unnecessary, but CreatePlayer would be more
	//  complete if these are enabled...
	//InitPlayer(pnum);
	//CalcPlrInv(pnum, false);

	//SetRndSeed(0);
}

/*
 * @brief Find a place for the given player starting from its current location.
 *
 * TODO: In the original code it was possible to auto-townwarp after resurrection.
 *       The new solution prevents this, but in some cases it could be useful
 *       (in some cases it is annoying).
 *
 */
static void PlacePlayer(int pnum)
{
	int i, j, nx, ny;
	const char* cr;

	for (i = 0; i < lengthof(plrxoff2); i++) {
		nx = plr._px + plrxoff2[i];
		ny = plr._py + plryoff2[i];

		if (PosOkActor(nx, ny) && PosOkPortal(nx, ny)) {
			break;
		}
	}

	if (i == 0)
		return;

	if (i == lengthof(plrxoff2)) {
		static_assert(DBORDERX >= 16 && DBORDERY >= 16, "PlacePlayer expects a large enough border.");
		for (i = 2; i < 16; i++) {
			cr = &CrawlTable[CrawlNum[i]];
			for (j = (BYTE)*cr; j > 0; j--) {
				nx = plr._px + *++cr;
				ny = plr._py + *++cr;
				if (PosOkActor(nx, ny) && PosOkPortal(nx, ny)) {
					i = 16;
					j = 0;
				}
			}
		}
	}

	plr._px = nx;
	plr._py = ny;
}

/*
 * Initialize player fields at startup(unpack).
 *  - calculate derived values
 */
void InitPlayer(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("InitPlayer: illegal player %d", pnum);
	}
	// calculate derived values
	CalculateGold(pnum);

	plr._pNextExper = PlrExpLvlsTbl[plr._pLevel];

	plr._pAblSkills = SPELL_MASK(Abilities[plr._pClass]);
	plr._pAblSkills |= SPELL_MASK(SPL_WALK) | SPELL_MASK(SPL_BLOCK)
		| SPELL_MASK(SPL_ATTACK) | SPELL_MASK(SPL_RATTACK);

	plr.walkpath[MAX_PATH_LENGTH] = DIR_NONE;
}

/*
 * Initialize players on the current level.
 */
void InitLvlPlayer(int pnum, bool entering)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("InitLvlPlayer: illegal player %d", pnum);
	}
	assert(plr._pDunLevel == currLvl._dLevelIdx);
	if (currLvl._dLevelIdx != DLV_TOWN) {
		plr._pSkillFlags	|= SFLAG_DUNGEON;
	} else {
		plr._pSkillFlags	&= ~SFLAG_DUNGEON;
	}

	CalcPlrItemVals(pnum, false);
	if (currLvl._dType == DTYPE_TOWN && plr._pHitPoints < (1 << 6))
		PlrSetHp(pnum, (1 << 6));

	InitPlayerGFX(pnum); // for the local player this is necessary only if switching from or to town
	SetPlrAnims(pnum);

	if (entering) {
		PlacePlayer(pnum);
		// RemovePlrFromMap(pnum);
		FixPlayerLocation(pnum);
		// dPlayer[plr._px][plr._py] = pnum + 1;

		plr._pdir = DIR_S;
		PlrStartStand(pnum);
		// TODO: randomize AnimFrame/AnimCnt for live players?
		// plr._pAnimFrame = RandRange(1, plr._pNFrames - 1);
		// plr._pAnimCnt = random_(2, 3);

		ClrPlrPath(pnum);
		plr.destAction = ACTION_NONE;
	} else {
		if (pnum == mypnum) {
			assert(plr._pfutx == plr._px);
			assert(plr._poldx == plr._px);
			assert(plr._pfuty == plr._py);
			assert(plr._poldy == plr._py);
			assert(plr._pxoff == 0);
			assert(plr._pyoff == 0);
			FixPlayerLocation(pnum);
		}
		if (plr._pmode != PM_DEATH && plr._pmode != PM_DYING) {
			dPlayer[plr._px][plr._py] = pnum + 1;
			if (plr._pmode == PM_WALK2) {
				dPlayer[plr._poldx][plr._poldy] = -(pnum + 1);
			} else if (plr._pmode == PM_WALK) {
				dPlayer[plr._pfutx][plr._pfuty] = -(pnum + 1);
			} else if (plr._pmode == PM_CHARGE) {
				dPlayer[plr._px][plr._py] = -(pnum + 1);
			}
		} else {
			dFlags[plr._px][plr._py] |= BFLAG_DEAD_PLAYER;
		}
		SyncPlrAnim(pnum);
	}

	if (pnum == mypnum) {
		plr._plid = AddLight(plr._poldx, plr._poldy, plr._pLightRad);
	} else {
		plr._plid = NO_LIGHT;
	}
	plr._pvid = AddVision(plr._poldx, plr._poldy, std::max(PLR_MIN_VISRAD, (int)plr._pLightRad), pnum == mypnum);
}

void RemoveLvlPlayer(int pnum)
{
	if (plr._pDunLevel == currLvl._dLevelIdx) {
		AddUnLight(plr._plid);
		AddUnVision(plr._pvid);
		//if (pnum == mypnum) {
		//	// to show the current player on the last frames before changing the level
		//	// RemovePlrFromMap(pnum); should be skipped. This is no longer necessary
		//	// because fade-out is turned off.
		//}
		RemovePlrFromMap(pnum);
		static_assert(MAX_MINIONS == MAX_PLRS, "RemoveLvlPlayer requires that owner of a monster has the same id as the monster itself.");
		if (currLvl._dLevelIdx != DLV_TOWN && monsters[pnum]._mmode <= MM_INGAME_LAST) {
			MonStartKill(pnum, pnum);
		}
	}
}

void NextPlrLevel(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("NextPlrLevel: illegal player %d", pnum);
	}
	plr._pLevel++;

	plr._pStatPts += 4;

	plr._pNextExper = PlrExpLvlsTbl[plr._pLevel];

	PlrFillHp(pnum);
	PlrFillMana(pnum);

	CalcPlrInv(pnum, false); // last parameter should not matter

	if (pnum == mypnum) {
		gbLvlUp = true;
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
		if (sgbControllerActive)
			FocusOnCharInfo();
#endif
	}
}

static void AddPlrSkillExp(int pnum, int lvl, unsigned exp)
{
	int i, n = 0;
	BYTE shr, sn, sl;
	unsigned xp, dLvl;
	BYTE skills[NUM_SPELLS];

	// collect the active skills below a level limit
	lvl += 8;
	for (i = 0; i < NUM_SPELLS; i++) {
		if (plr._pSkillActivity[i] != 0 && (4 * plr._pSkillLvlBase[i]) < lvl) {
			skills[n] = i;
			n++;
		}
	}

	// calculate modifier
	static_assert(NUM_SPELLS < 64, "Optimization requires the highest bit to be free.");
	shr = 3;
	for (i = 1; i < n; i <<= 1) {
		shr++;
	}

	// raise the exp of the active skills
	for (i = 0; i < n; i++) {
		sn = skills[i];
		plr._pSkillActivity[sn]--;

		sl = plr._pSkillLvlBase[sn];
		dLvl = lvl - (4 * sl);
		xp = (exp * dLvl) >> shr; // / (8 * n);

		xp += plr._pSkillExp[sn];
		if (xp > SkillExpLvlsTbl[MAXSPLLEVEL] - 1) {
			xp = SkillExpLvlsTbl[MAXSPLLEVEL] - 1;
		}

		plr._pSkillExp[sn] = xp;
		while (xp >= SkillExpLvlsTbl[sl]) {
			sl++;
		}
		if (sl == plr._pSkillLvlBase[sn])
			continue;
		assert(sl <= MAXSPLLEVEL);
		plr._pSkillLvlBase[sn] = sl;
		CalcPlrItemVals(pnum, false);
	}
}

void AddPlrExperience(int pnum, int lvl, unsigned exp)
{
	unsigned expCap;
	int dLvl;

	//if (pnum != mypnum) {
	//	return;
	//}

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("AddPlrExperience: illegal player %d", pnum);
	}

	//if (plr._pHitPoints < (1 << 6)) {
	//	return;
	//}
	// assert(plr._pmode != PM_DEATH);

	// Add xp to the used skills
	AddPlrSkillExp(pnum, lvl, exp);

	// Adjust xp based on difference in level between player and monster
	dLvl = 8 + lvl - plr._pLevel;
	if (dLvl <= 0) {
		return;
	}
	exp = exp * dLvl / 8;

	// Prevent power leveling
	if (IsMultiGame) {
		assert(plr._pLevel >= 0 && plr._pLevel <= MAXCHARLEVEL);
		// cap to 1/32 of current levels xp
		expCap = plr._pNextExper / 32;
		if (exp > expCap) {
			exp = expCap;
		}
		// cap to 128 * current level
		expCap = 128 * plr._pLevel;
		if (exp > expCap) {
			exp = expCap;
		}
	}

	plr._pExperience += exp;
	if (plr._pExperience > PlrExpLvlsTbl[MAXCHARLEVEL] - 1) {
		plr._pExperience = PlrExpLvlsTbl[MAXCHARLEVEL] - 1;
	}

	// Increase player level if applicable
	while (plr._pExperience >= plr._pNextExper) {
		assert(plr._pLevel < MAXCHARLEVEL);
		NextPlrLevel(pnum);
	}
}

static bool PlrDirOK(int pnum, int dir)
{
	int px, py;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrDirOK: illegal player %d", pnum);
	}

	px = plr._px + offset_x[dir];
	py = plr._py + offset_y[dir];

	//assert(px >= DBORDERX - 1 && px < DBORDERX + DSIZEX + 1);
	//assert(py >= DBORDERY - 1 && px < DBORDERY + DSIZEX + 1);
	//assert(dPiece[px][py] != 0);
	if (/*px < 0 || !dPiece[px][py] ||*/ !PosOkActor(px, py)) {
		return false;
	}

	if (dir == DIR_E) {
		return !nSolidTable[dPiece[px][py + 1]];
	}

	if (dir == DIR_W) {
		return !nSolidTable[dPiece[px + 1][py]];
	}

	return true;
}

static void StartPlrKill(int pnum, int dmgtype)
{
	if (currLvl._dType == DTYPE_TOWN) {
		PlrSetHp(pnum, (1 << 6));
		return;
	}

	if (plr._pmode == PM_DEATH || plr._pmode == PM_DYING)
		return;

	plr._pmode = PM_DYING;
	plr._pInvincible = TRUE;

	plr._pManaShield = 0;
	plr._pTimer[PLTR_INFRAVISION] = 0;
	plr._pTimer[PLTR_RAGE] = 0;

	if (pnum == mypnum) {
		gbDeathflag = MDM_DYING;
		NetSendCmdBParam1(CMD_PLRDEAD, dmgtype);
	}

	if (plr._pDunLevel == currLvl._dLevelIdx) {
		if (plr._pgfxnum != ANIM_ID_UNARMED) {
			plr._pgfxnum = ANIM_ID_UNARMED;
			plr._pGFXLoad = 0;
			SetPlrAnims(pnum);
		}

		if (!(plr._pGFXLoad & PFILE_DEATH)) {
			LoadPlrGFX(pnum, PFILE_DEATH);
		}

		NewPlrAnim(pnum, plr._pDAnim, plr._pdir, plr._pDFrames, PlrAnimFrameLens[PA_DEATH], plr._pDWidth);

		RemovePlrFromMap(pnum);
		PlaySfxLoc(sgSFXSets[SFXS_PLR_71][plr._pClass], plr._px, plr._py);
		dFlags[plr._px][plr._py] |= BFLAG_DEAD_PLAYER;
		FixPlayerLocation(pnum);

		plr._pVar7 = pnum == mypnum ? 32 : 0; // DEATH_DELAY
	}
	CalcPlrInv(pnum, false);
	//PlrSetHp(pnum, 0);
	//PlrSetMana(pnum, 0);
}

/*void PlrClrTrans(int x, int y)
{
	int i, j;

	for (i = y - 1; i <= y + 1; i++) {
		for (j = x - 1; j <= x + 1; j++) {
			TransList[dTransVal[j][i]] = false;
		}
	}
}

void PlrDoTrans(int x, int y)
{
	int i, j;

	if (currLvl._dType != DTYPE_CATHEDRAL && currLvl._dType != DTYPE_CATACOMBS) {
		TransList[1] = true;
	} else {
		for (i = y - 1; i <= y + 1; i++) {
			for (j = x - 1; j <= x + 1; j++) {
				if (!nSolidTable[dPiece[j][i]] && dTransVal[j][i]) {
					TransList[dTransVal[j][i]] = true;
				}
			}
		}
	}
}*/

void FixPlayerLocation(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("FixPlayerLocation: illegal player %d", pnum);
	}
	plr._pfutx = plr._poldx = plr._px;
	plr._pfuty = plr._poldy = plr._py;
	plr._pxoff = 0;
	plr._pyoff = 0;
	if (pnum == mypnum) {
		ScrollInfo._sxoff = 0;
		ScrollInfo._syoff = 0;
		ScrollInfo._sdir = SDIR_NONE;
		ViewX = plr._px; // - ScrollInfo._sdx;
		ViewY = plr._py; // - ScrollInfo._sdy;
	}
}

static void AssertFixPlayerLocation(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("FixPlayerLocation: illegal player %d", pnum);
	}
	assert(plr._pfutx == plr._px);
	assert(plr._poldx == plr._px);
	assert(plr._pfuty == plr._py);
	assert(plr._poldy == plr._py);
	assert(plr._pxoff == 0);
	assert(plr._pyoff == 0);
	if (pnum == mypnum) {
		assert(ScrollInfo._sxoff == 0);
		assert(ScrollInfo._syoff == 0);
		assert(ScrollInfo._sdir == SDIR_NONE);
		assert(ViewX == plr._px); // - ScrollInfo._sdx;
		assert(ViewY == plr._py); // - ScrollInfo._sdy;
	}
}

static void StartStand(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("StartStand: illegal player %d", pnum);
	}

	plr._pmode = PM_STAND;

	if (!(plr._pGFXLoad & PFILE_STAND)) {
		LoadPlrGFX(pnum, PFILE_STAND);
	}

	NewPlrAnim(pnum, plr._pNAnim, plr._pdir, plr._pNFrames, PlrAnimFrameLens[PA_STAND], plr._pNWidth);
}

void PlrStartStand(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrStartStand: illegal player %d", pnum);
	}
	if (plr._pHitPoints >= (1 << 6)) {
		StartStand(pnum);
		RemovePlrFromMap(pnum);
		dPlayer[plr._px][plr._py] = pnum + 1;
		FixPlayerLocation(pnum);
	} else {
		StartPlrKill(pnum, DMGTYPE_UNKNOWN);
	}
}

static void StartWalkStand(int pnum)
{
	plr._pmode = PM_STAND;
}

/*static void PlrStartWalkStand(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrStartWalkStand: illegal player %d", pnum);
	}
	plr._pmode = PM_STAND;
	plr._pfutx = plr._px;
	plr._pfuty = plr._py;
	plr._pxoff = 0;
	plr._pyoff = 0;

	if (pnum == mypnum) {
		ScrollInfo._sxoff = 0;
		ScrollInfo._syoff = 0;
		ScrollInfo._sdir = SDIR_NONE;
		ViewX = plr._px; // - ScrollInfo._sdx;
		ViewY = plr._py; // - ScrollInfo._sdy;
	}
}*/

static void PlrChangeLightOff(int pnum)
{
	int x, y;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrChangeLightOff: illegal player %d", pnum);
	}
	x = plr._pxoff + 2 * plr._pyoff;
	y = 2 * plr._pyoff - plr._pxoff;

	x = x / (TILE_WIDTH / 8); // ASSET_MPL * 8 ?
	y = y / (TILE_WIDTH / 8);

	CondChangeLightOff(plr._plid, x, y);
}

static void PlrChangeOffset(int pnum)
{
	// int px, py;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrChangeOffset: illegal player %d", pnum);
	}

	// px = plr._pVar6 >> PLR_WALK_SHIFT; // WALK_XOFF
	// py = plr._pVar7 >> PLR_WALK_SHIFT; // WALK_YOFF

	plr._pVar6 += plr._pVar4; // WALK_XOFF <- WALK_XVEL
	plr._pVar7 += plr._pVar5; // WALK_YOFF <- WALK_YVEL

	plr._pxoff = plr._pVar6 >> PLR_WALK_SHIFT;
	plr._pyoff = plr._pVar7 >> PLR_WALK_SHIFT;

	// px -= plr._pxoff;
	// py -= plr._pyoff;

	if (pnum == mypnum /*&& ScrollInfo._sdir != SDIR_NONE*/) {
		assert(ScrollInfo._sdir != SDIR_NONE);
		// ScrollInfo._sxoff += px;
		// ScrollInfo._syoff += py;
		ScrollInfo._sxoff = -plr._pxoff;
		ScrollInfo._syoff = -plr._pyoff;
		// TODO: follow with the cursor if a monster is selected? (does not work well with upscale)
		//if (gbActionBtnDown && (px | py) != 0 && pcursmonst != MON_NONE)
		//	SetCursorPos(MouseX + px, MouseY + py);
	}

	//if (plr._plid != NO_LIGHT)
		PlrChangeLightOff(pnum);
}

/**
 * @brief Starting a move action towards NW, N, NE or W
 */
static void StartWalk1(int pnum, int xvel, int yvel, int dir)
{
	int px, py;

	plr._pmode = PM_WALK;
	plr._pVar4 = xvel;   // WALK_XVEL : velocity of the player in the X-direction
	plr._pVar5 = yvel;   // WALK_YVEL : velocity of the player in the Y-direction
	plr._pxoff = 0;
	plr._pyoff = 0;
	//plr._pVar3 = dir; // Player's direction when ending movement.
	plr._pVar6 = 0;      // WALK_XOFF : _pxoff value in a higher range
	plr._pVar7 = 0;      // WALK_YOFF : _pyoff value in a higher range
	plr._pVar8 = 0;      // WALK_TICK : speed helper

	px = plr._px;
	py = plr._py;
	assert(plr._poldx == px);
	assert(plr._poldy == py);

	px += offset_x[dir];
	py += offset_y[dir];
	plr._pfutx = /*plr._pVar1 =*/ px; // the Player's x-coordinate after the movement
	plr._pfuty = /*plr._pVar2 =*/ py; // the Player's y-coordinate after the movement

	dPlayer[px][py] = -(pnum + 1);
}

/**
 * @brief Starting a move action towards SW, S, SE or E
 */
#if defined(__clang__) || defined(__GNUC__)
__attribute__((no_sanitize("shift-base")))
#endif
static void StartWalk2(int pnum, int xvel, int yvel, int xoff, int yoff, int dir)
{
	int px, py;

	plr._pmode = PM_WALK2;
	plr._pVar4 = xvel;       // WALK_XVEL : velocity of the player in the X-direction
	plr._pVar5 = yvel;       // WALK_YVEL : velocity of the player in the Y-direction
	plr._pxoff = xoff;       // Offset player sprite to align with their previous tile position
	plr._pyoff = yoff;
	plr._pVar6 = xoff << PLR_WALK_SHIFT;  // WALK_XOFF : _pxoff value in a higher range
	plr._pVar7 = yoff << PLR_WALK_SHIFT;  // WALK_YOFF : _pyoff value in a higher range
	//plr._pVar3 = dir;      // Player's direction when ending movement.
	plr._pVar8 = 0;          // WALK_TICK : speed helper

	px = plr._px;
	py = plr._py;
	assert(plr._poldx == px);
	assert(plr._poldy == py);
	dPlayer[px][py] = -(pnum + 1);
	px += offset_x[dir];
	py += offset_y[dir];
	plr._px = plr._pfutx = px; // Move player to the next tile to maintain correct render order
	plr._py = plr._pfuty = py;
	dPlayer[px][py] = pnum + 1;
	if (pnum == mypnum) {
		ViewX = px;
		ViewY = py;
		ScrollInfo._sxoff = -xoff;
		ScrollInfo._syoff = -yoff;
	}
	//if (plr._plid != NO_LIGHT) {
		ChangeLightXY(plr._plid, plr._px, plr._py);
		PlrChangeLightOff(pnum);
	//}
}

static bool StartWalk(int pnum)
{
	int dir, i, xvel3, xvel, yvel;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("StartWalk: illegal player %d", pnum);
	}

	assert(plr.walkpath[MAX_PATH_LENGTH] == DIR_NONE);
	dir = plr.walkpath[0];
	for (i = 0; i < MAX_PATH_LENGTH; i++) {
		plr.walkpath[i] = plr.walkpath[i + 1];
	}

	//dir = walk2dir[dir];
	if (!PlrDirOK(pnum, dir)) {
		return false;
	}

#if DEBUG_MODE
	for (i = 0; i < NUM_CLASSES; i++)
		assert(PlrGFXAnimLens[i][PA_WALK] == PlrGFXAnimLens[PC_WARRIOR][PA_WALK]);
#endif
	xvel3 = (TILE_WIDTH << PLR_WALK_SHIFT) / (PlrGFXAnimLens[PC_WARRIOR][PA_WALK]);
	xvel = (TILE_WIDTH << PLR_WALK_SHIFT) / (PlrGFXAnimLens[PC_WARRIOR][PA_WALK] * 2);
	yvel = (TILE_HEIGHT << PLR_WALK_SHIFT) / (PlrGFXAnimLens[PC_WARRIOR][PA_WALK] * 2);
	switch (dir) {
	case DIR_N:
		StartWalk1(pnum, 0, -xvel, dir);
		break;
	case DIR_NE:
		StartWalk1(pnum, xvel, -yvel, dir);
		break;
	case DIR_E:
		StartWalk2(pnum, xvel3, 0, -TILE_WIDTH, 0, dir);
		break;
	case DIR_SE:
		StartWalk2(pnum, xvel, yvel, -TILE_WIDTH/2, -TILE_HEIGHT/2, dir);
		break;
	case DIR_S:
		StartWalk2(pnum, 0, xvel, 0, -TILE_HEIGHT, dir);
		break;
	case DIR_SW:
		StartWalk2(pnum, -xvel, yvel, TILE_WIDTH/2, -TILE_HEIGHT/2, dir);
		break;
	case DIR_W:
		StartWalk1(pnum, -xvel3, 0, dir);
		break;
	case DIR_NW:
		StartWalk1(pnum, -xvel, -yvel, dir);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	if (!(plr._pGFXLoad & PFILE_WALK)) {
		LoadPlrGFX(pnum, PFILE_WALK);
	}

	NewPlrAnim(pnum, plr._pWAnim, dir, plr._pWFrames, PlrAnimFrameLens[PA_WALK], plr._pWWidth);

	if (pnum == mypnum) {
		// assert(ScrollInfo._sdx == 0);
		// assert(ScrollInfo._sdy == 0);
		// assert(plr._poldx == ViewX);
		// assert(plr._poldy == ViewY);
		// ScrollInfo._sdx = plr._poldx - ViewX;
		// ScrollInfo._sdy = plr._poldy - ViewY;

#if DEBUG_MODE
		for (int i = 0; i < lengthof(dir2sdir); i++)
			assert(dir2sdir[i] == 1 + OPPOSITE(i));
#endif
		dir = 1 + OPPOSITE(dir); // == dir2sdir[dir];
		/*if (!gbZoomInFlag) {
			if (abs(ScrollInfo._sdx) >= 3 || abs(ScrollInfo._sdy) >= 3) {
				ScrollInfo._sdir = SDIR_NONE;
			} else {
				ScrollInfo._sdir = dir;
			}
		} else if (abs(ScrollInfo._sdx) >= 2 || abs(ScrollInfo._sdy) >= 2) {
			ScrollInfo._sdir = SDIR_NONE;
		} else {*/
			ScrollInfo._sdir = dir;
		//}
	}
	return true;
}

static bool StartAttack(int pnum)
{
	int i, dx, dy, sn, sl, dir;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("StartAttack: illegal player %d", pnum);
	}

	i = plr.destParam1;
	switch (plr.destAction) {
	case ACTION_ATTACK:
		dx = i;
		dy = plr.destParam2;
		sn = plr.destParam3;
		sl = plr.destParam4;
		break;
	case ACTION_ATTACKMON:
		dx = monsters[i]._mfutx;
		dy = monsters[i]._mfuty;
		if (abs(plr._px - dx) > 1 || abs(plr._py - dy) > 1)
			return false;
		sn = plr.destParam3;
		sl = plr.destParam4;
		break;
	case ACTION_ATTACKPLR:
		dx = plx(i)._pfutx;
		dy = plx(i)._pfuty;
		if (abs(plr._px - dx) > 1 || abs(plr._py - dy) > 1)
			return false;
		sn = plr.destParam3;
		sl = plr.destParam4;
		break;
	case ACTION_OPERATE:
		dx = plr.destParam2;
		dy = plr.destParam3;
		if (abs(plr._px - dx) > 1 || abs(plr._py - dy) > 1)
			return false;
		assert(abs(dObject[dx][dy]) == i + 1);
		if (objects[i]._oBreak != OBM_BREAKABLE) {
			OperateObject(pnum, i, false);
			return true;
		}
		sn = SPL_ATTACK;
		sl = 0;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	dir = GetDirection(plr._px, plr._py, dx, dy);
	plr._pmode = PM_ATTACK;
	plr._pVar5 = sn; // ATTACK_SKILL
	plr._pVar6 = sl; // ATTACK_SKILL_LEVEL
	plr._pVar7 = 0;  // ATTACK_ACTION_PROGRESS : 'flags' of sfx and hit
	plr._pVar8 = 0;  // ATTACK_TICK : speed helper

	if (!(plr._pGFXLoad & PFILE_ATTACK)) {
		LoadPlrGFX(pnum, PFILE_ATTACK);
	}
	NewPlrAnim(pnum, plr._pAAnim, dir, plr._pAFrames, PlrAnimFrameLens[PA_ATTACK], plr._pAWidth);

	AssertFixPlayerLocation(pnum);
	return true;
}

static void StartRangeAttack(int pnum)
{
	int i, dx, dy, sn, sl, dir;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("StartRangeAttack: illegal player %d", pnum);
	}

	i = plr.destParam1;
	switch (plr.destAction) {
	case ACTION_RATTACK:
		dx = i;
		dy = plr.destParam2;
		break;
	case ACTION_RATTACKMON:
		dx = monsters[i]._mfutx;
		dy = monsters[i]._mfuty;
		break;
	case ACTION_RATTACKPLR:
		dx = plx(i)._pfutx;
		dy = plx(i)._pfuty;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	sn = plr.destParam3;
	sl = plr.destParam4;

	plr._pVar1 = dx;    // RATTACK_TARGET_X
	plr._pVar2 = dy;    // RATTACK_TARGET_Y
	plr._pVar5 = sn;    // RATTACK_SKILL
	plr._pVar6 = sl;    // RATTACK_SKILL_LEVEL
	plr._pVar7 = FALSE; // RATTACK_ACTION_PROGRESS : 'flag' of launch
	plr._pVar8 = 0;     // RATTACK_TICK : speed helper
	plr._pmode = PM_RATTACK;

	dir = GetDirection(plr._px, plr._py, dx, dy);

	if (!(plr._pGFXLoad & PFILE_ATTACK)) {
		LoadPlrGFX(pnum, PFILE_ATTACK);
	}
	NewPlrAnim(pnum, plr._pAAnim, dir, plr._pAFrames, PlrAnimFrameLens[PA_ATTACK], plr._pAWidth);

	AssertFixPlayerLocation(pnum);
}

static void StartBlock(int pnum, int dir)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("StartBlock: illegal player %d", pnum);
	}

	plr._pmode = PM_BLOCK;
	plr._pVar1 = 0; // BLOCK_EXTENSION : extended blocking
	if (!(plr._pGFXLoad & PFILE_BLOCK)) {
		LoadPlrGFX(pnum, PFILE_BLOCK);
	}
	NewPlrAnim(pnum, plr._pBAnim, dir, plr._pBFrames, PlrAnimFrameLens[PA_BLOCK], plr._pBWidth);

	AssertFixPlayerLocation(pnum);
}

static void StartSpell(int pnum)
{
	int i, dx, dy;
	player_graphic gfx;
	BYTE **anim;
	const SpellData *sd;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("StartSpell: illegal player %d", pnum);
	}

	i = plr.destParam1;
	switch (plr.destAction) {
	case ACTION_SPELL:
		dx = i;
		dy = plr.destParam2;
		break;
	case ACTION_SPELLMON:
		dx = monsters[i]._mfutx;
		dy = monsters[i]._mfuty;
		break;
	case ACTION_SPELLPLR:
		// preserve target information for the resurrect spell
		if (plr.destParam3 == SPL_RESURRECT) // SPELL_NUM
			plr.destParam4 = i;              // SPELL_LEVEL
		dx = plx(i)._pfutx;
		dy = plx(i)._pfuty;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	plr._pVar1 = dx;                    // SPELL_TARGET_X
	plr._pVar2 = dy;                    // SPELL_TARGET_Y
	plr._pVar3 = plr.destParam3;        // SPELL_NUM : the spell to be cast
	plr._pVar4 = plr.destParam4;        // SPELL_LEVEL
	plr._pVar7 = FALSE;                 // SPELL_ACTION_PROGRESS : 'flag' of cast
	plr._pVar8 = 0;                     // SPELL_TICK : speed helper
	plr._pmode = PM_SPELL;

	sd = &spelldata[plr._pVar3]; // SPELL_NUM
	if (sd->sSkillFlags & SDFLAG_TARGETED)
		plr._pdir = GetDirection(plr._px, plr._py, dx, dy);
	switch (sd->sType) {
	case STYPE_FIRE:
		gfx = PFILE_FIRE;
		anim = plr._pFAnim;
		break;
	case STYPE_LIGHTNING:
		gfx = PFILE_LIGHTNING;
		anim = plr._pLAnim;
		break;
	case STYPE_MAGIC:
		gfx = PFILE_MAGIC;
		anim = plr._pTAnim;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	if (!(plr._pGFXLoad & gfx)) {
		LoadPlrGFX(pnum, gfx);
	}
	NewPlrAnim(pnum, anim, plr._pdir, plr._pSFrames, PlrAnimFrameLens[PA_SPELL], plr._pSWidth);

	PlaySfxLoc(sd->sSFX, plr._px, plr._py);

	AssertFixPlayerLocation(pnum);
}

static void StartPickItem(int pnum)
{
	int i, x, y;

	if (pnum != mypnum)
		return;
	if (pcursicon != CURSOR_HAND)
		return;

	i = plr.destParam1;
	x = abs(plr._px - items[i]._ix);
	y = abs(plr._py - items[i]._iy);
	if (x > 1 || y > 1)
		return;

	NetSendCmdGItem(plr.destAction == ACTION_PICKUPAITEM ? CMD_AUTOGETITEM : CMD_GETITEM, i);
}

static void StartTalk(int pnum)
{
	int mnum, x, y;
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("StartTalk: illegal player %d", pnum);
	}

	mnum = plr.destParam1;
	x = abs(plr._px - monsters[mnum]._mx);
	y = abs(plr._py - monsters[mnum]._my);
	if (x > 1 || y > 1)
		return;

	if (currLvl._dLevelIdx == DLV_TOWN) {
		if (pnum == mypnum)
			TalkToTowner(mnum);
	} else
		TalktoMonster(mnum, pnum);
}

void RemovePlrFromMap(int pnum)
{
	int pp, dx, dy, y, x;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("RemovePlrFromMap: illegal player %d", pnum);
	}

	dx = plr._poldx;
	dy = plr._poldy;
	assert(dx >= DBORDERX && dx < DBORDERX + DSIZEX);
	assert(dy >= DBORDERY && dy < DBORDERY + DSIZEY);

	pp = pnum + 1;
	for (x = dx - 1; x <= dx + 1; x++) {
		for (y = dy - 1; y <= dy + 1; y++) {
			if (abs(dPlayer[x][y]) == pp) {
				dPlayer[x][y] = 0;
			}
		}
	}
}

void StartPlrHit(int pnum, int dam, bool forcehit, int dir)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("StartPlrHit: illegal player %d", pnum);
	}

	assert(plr._pHitPoints >= (1 << 6));

	if (!forcehit && plr._pManaShield != 0)
		return;

	PlaySfxLoc(sgSFXSets[SFXS_PLR_69][plr._pClass], plr._px, plr._py, 2);

	if (!forcehit && (dam << 2) < plr._pMaxHP)
		return;

	dir = dir == DIR_NONE ? plr._pdir : OPPOSITE(dir);

	if (!(plr._pGFXLoad & PFILE_HIT)) {
		LoadPlrGFX(pnum, PFILE_HIT);
	}
	NewPlrAnim(pnum, plr._pHAnim, dir, plr._pHFrames, PlrAnimFrameLens[PA_GOTHIT], plr._pHWidth);

	plr._pmode = PM_GOTHIT;
	RemovePlrFromMap(pnum);
	dPlayer[plr._px][plr._py] = pnum + 1;
	FixPlayerLocation(pnum);
}

#if defined(__clang__) || defined(__GNUC__)
__attribute__((no_sanitize("shift-base")))
#endif
void SyncPlrKill(int pnum, int dmgtype)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("SyncPlrKill: illegal player %d", pnum);
	}

	if (plr._pmode == PM_DEATH) {
		return;
	}

	if (plr._pmode != PM_DYING) {
		StartPlrKill(pnum, dmgtype);
	}

	plr._pmode = PM_DEATH;
}

void SyncPlrResurrect(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("SyncPlrResurrect: illegal player %d", pnum);
	}

	if (plr._pmode != PM_DEATH)
		return;

	if (pnum == mypnum) {
		gbDeathflag = MDM_ALIVE;
		gamemenu_off();
	}

	ClrPlrPath(pnum);
	plr.destAction = ACTION_NONE;
	plr._pmode = PM_STAND;
	plr._pInvincible = FALSE;

	PlrSetHp(pnum, std::min(10 << 6, plr._pMaxHP));

	CalcPlrInv(pnum, false);

	if (plr._pDunLevel == currLvl._dLevelIdx) {
		PlacePlayer(pnum);
		PlrStartStand(pnum);
	}
}

static void InitLevelChange(int pnum)
{
	RemoveLvlPlayer(pnum);

	ClrPlrPath(pnum);
	plr.destAction = ACTION_NONE;
	plr._pLvlChanging = TRUE;
	plr._pmode = PM_NEWLVL;
	plr._pInvincible = TRUE;
}

#if defined(__clang__) || defined(__GNUC__)
__attribute__((no_sanitize("shift-base")))
#endif
void StartNewLvl(int pnum, int fom, int lvl)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("StartNewLvl: illegal player %d", pnum);
	}
	InitLevelChange(pnum);

	switch (fom) {
	case DVL_DWM_NEXTLVL:
	case DVL_DWM_PREVLVL:
	case DVL_DWM_RTNLVL:
	case DVL_DWM_TWARPDN:
	case DVL_DWM_SETLVL:
		break;
	case DVL_DWM_TWARPUP:
		if (pnum == mypnum) {
			assert(currLvl._dType >= 1);
			static_assert((int)TWARP_CATHEDRAL == (int)DTYPE_CATHEDRAL - 1, "Dtype to Warp conversion requires matching enums I.");
			static_assert((int)TWARP_CATACOMB == (int)DTYPE_CATACOMBS - 1, "Dtype to Warp conversion requires matching enums II.");
			static_assert((int)TWARP_CAVES == (int)DTYPE_CAVES - 1, "Dtype to Warp conversion requires matching enums III.");
			static_assert((int)TWARP_HELL == (int)DTYPE_HELL - 1, "Dtype to Warp conversion requires matching enums IV.");
#ifdef HELLFIRE
			static_assert((int)TWARP_NEST == (int)DTYPE_NEST - 1, "Dtype to Warp conversion requires matching enums V.");
			static_assert((int)TWARP_CRYPT == (int)DTYPE_CRYPT - 1, "Dtype to Warp conversion requires matching enums VI.");
#endif
			gbTWarpFrom = (currLvl._dType - 1);
			gbTownWarps |= 1 << gbTWarpFrom;
		}
		break;
	default:
		net_assert(0);
		ASSUME_UNREACHABLE
	}
	net_assert(lvl < NUM_LEVELS);
	plr._pDunLevel = lvl;
	if (pnum == mypnum) {
		PostMessage(fom, 0);
	}
}

void RestartTownLvl(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("RestartTownLvl: illegal player %d", pnum);
	}
	InitLevelChange(pnum);

	plr._pDunLevel = DLV_TOWN;

	PlrSetHp(pnum, (1 << 6));

	// CalcPlrInv(pnum, false); -- no need to calc, InitLvlPlayer should take care about it

	if (pnum == mypnum) {
		gbDeathflag = MDM_ALIVE;
		gamemenu_off();
		PostMessage(DVL_DWM_RETOWN, 0);
	}
}

void StartTWarp(int pnum, int pidx)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("StartWarpLvl: illegal player %d", pnum);
	}
	InitLevelChange(pnum);

	if (plr._pDunLevel != DLV_TOWN) {
		plr._pDunLevel = DLV_TOWN;
	} else {
		plr._pDunLevel = portals[pidx].level;
		static_assert(MAXPORTAL == MAX_PLRS, "StartTWarp uses pnum as portal-id.");
		if (pidx == pnum) {
			DeactivatePortal(pidx);
		}
	}

	if (pnum == mypnum) {
		UseCurrentPortal(pidx);
		PostMessage(DVL_DWM_WARPLVL, 0);
	}
}

static inline void PlrStepAnim(int pnum)
{
	plr._pAnimCnt++;
	if (plr._pAnimCnt >= plr._pAnimFrameLen) {
		plr._pAnimCnt = 0;
		plr._pAnimFrame++;
	}
}

static void PlrDoWalk(int pnum)
{
	int px, py;
	bool skipAnim;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrDoWalk: illegal player %d", pnum);
	}

	plr._pVar8++; // WALK_TICK
	if (plr._pIFlags & (ISPL_FASTESTWALK | ISPL_FASTERWALK | ISPL_FASTWALK)) {
		if (plr._pIFlags & ISPL_FASTESTWALK) {
			skipAnim = true;
		} else if (plr._pIFlags & ISPL_FASTERWALK) {
			skipAnim = (plr._pVar8 & 1) == 1;
		} else { // if (plr._pIFlags & ISPL_FASTWALK) {
			skipAnim = (plr._pVar8 & 3) == 2;
		}
		if (skipAnim) {
			PlrStepAnim(pnum);
			PlrChangeOffset(pnum);
		}
	}
	assert(PlrAnimFrameLens[PA_WALK] == 1);
	if ((plr._pAnimFrame & 3) == 3) {
		PlaySfxLoc(PS_WALK1, plr._px, plr._py);
	}

	assert(PlrAnimFrameLens[PA_WALK] == 1);
	if (plr._pAnimFrame < plr._pWFrames) {
		PlrChangeOffset(pnum);
		return;
	}

	dPlayer[plr._poldx][plr._poldy] = 0;
	px = plr._pfutx;
	py = plr._pfuty;

	ChangeLightXYOff(plr._plid, px, py);
	ChangeVisionXY(plr._pvid, px, py);
	plr._px = px;
	plr._py = py;
	FixPlayerLocation(pnum);
	dPlayer[px][py] = pnum + 1;

	if (plr.walkpath[0] != DIR_NONE) {
		//PlrStartWalkStand(pnum);
		StartWalkStand(pnum);
	} else {
		//PlrStartStand(pnum);
		StartStand(pnum);
	}

	//ClearPlrPVars(pnum);
}

static void ReduceItemDur(ItemStruct* pi, BYTE iLoc, int pnum)
{
	if (pi->_iDurability == DUR_INDESTRUCTIBLE)
		return;

	pi->_iDurability--;
	if (pi->_iDurability != 0)
		return;
	pi->_iDurability = 1;
	if (pnum == mypnum)
		NetSendCmdDelItem(iLoc);
}

static void WeaponDur(int pnum, int durrnd)
{
	ItemStruct* pi;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("WeaponDur: illegal player %d", pnum);
	}
	// assert(durrnd > 0 && durrnd < 0xFFFF);
	if (random_low(3, durrnd) != 0) {
		return;
	}

	// check dual-wield
	pi = &plr._pInvBody[INVLOC_HAND_RIGHT];
	if (pi->_itype != ITYPE_NONE && pi->_iClass == ICLASS_WEAPON && random_(3, 2) != 0) {
		ReduceItemDur(pi, INVLOC_HAND_RIGHT, pnum);
		return;
	}

	// check weapon in left hand
	pi = &plr._pInvBody[INVLOC_HAND_LEFT];
	if (pi->_itype != ITYPE_NONE) {
		assert(pi->_iClass == ICLASS_WEAPON);
		ReduceItemDur(pi, INVLOC_HAND_LEFT, pnum);
		return;
	}

	// check shield in right hand if left hand is empty
	pi = &plr._pInvBody[INVLOC_HAND_RIGHT];
	if (pi->_itype != ITYPE_NONE) {
		assert(pi->_itype == ITYPE_SHIELD);
		ReduceItemDur(pi, INVLOC_HAND_RIGHT, pnum);
		return;
	}
}

static bool PlrHitMonst(int pnum, int sn, int sl, int mnum)
{
	MonsterStruct* mon;
	int hper, dam, skdam, damsl, dambl, dampc;
	unsigned tmp, hitFlags;
	bool tmac, ret;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("PlrHitMonst: illegal monster %d", mnum);
	}

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrHitMonst: illegal player %d", pnum);
	}

	mon = &monsters[mnum];

	hper = plr._pIHitChance - mon->_mArmorClass;
	if (sn == SPL_SWIPE) {
		hper -= 30 - sl * 2;
	}
	if (!CheckHit(hper) && mon->_mmode != MM_STONE)
#if DEBUG_MODE
		if (!debug_mode_god_mode)
#endif
			return false;

	if (CheckMonsterHit(mnum, &ret))
		return ret;

	dam = 0;
	tmac = (plr._pIFlags & ISPL_PENETRATE_PHYS) != 0;
	damsl = plr._pISlMaxDam;
	if (damsl != 0)
		dam += CalcMonsterDam(mon->_mMagicRes, MISR_SLASH, plr._pISlMinDam, damsl, tmac);
	dambl = plr._pIBlMaxDam;
	if (dambl != 0)
		dam += CalcMonsterDam(mon->_mMagicRes, MISR_BLUNT, plr._pIBlMinDam, dambl, tmac);
	dampc = plr._pIPcMaxDam;
	if (dampc != 0)
		dam += CalcMonsterDam(mon->_mMagicRes, MISR_PUNCTURE, plr._pIPcMinDam, dampc, tmac);

	tmp = sn == SPL_SWIPE ? 800 : 200;
	if (random_low(6, tmp) < plr._pICritChance) {
		dam <<= 1;
	}

	if (sn == SPL_SWIPE) {
		dam = (dam * (48 + sl)) >> 6;
	}
	if (plr._pILifeSteal != 0) {
		skdam = (dam * plr._pILifeSteal) >> 7;
		PlrIncHp(pnum, skdam);
	}
	if (plr._pIManaSteal != 0) {
		skdam = (dam * plr._pIManaSteal) >> 7;
		PlrIncMana(pnum, skdam);
	}

	int fdam = plr._pIFMaxDam;
	if (fdam != 0)
		fdam = CalcMonsterDam(mon->_mMagicRes, MISR_FIRE, plr._pIFMinDam, fdam, false);
	int ldam = plr._pILMaxDam;
	if (ldam != 0)
		ldam = CalcMonsterDam(mon->_mMagicRes, MISR_LIGHTNING, plr._pILMinDam, ldam, false);
	int mdam = plr._pIMMaxDam;
	if (mdam != 0)
		mdam = CalcMonsterDam(mon->_mMagicRes, MISR_MAGIC, plr._pIMMinDam, mdam, false);
	int adam = 0;
	if (adam != 0)
		adam = CalcMonsterDam(mon->_mMagicRes, MISR_ACID, plr._pIAMinDam, adam, false);

	if ((fdam | ldam | mdam | adam) != 0) {
		dam += fdam + ldam + mdam + adam;
		AddElementalExplosion(mon->_mx, mon->_my, fdam, ldam, mdam, adam);
	}

	//if (pnum == mypnum) {
		mon->_mhitpoints -= dam;
	//}

	if (mon->_mhitpoints < (1 << 6)) {
		MonStartKill(mnum, pnum);
	} else {
		hitFlags = plr._pIFlags;
		//if (hitFlags & ISPL_NOHEALMON) {
		//	mon->_mFlags |= MFLAG_NOHEAL;
		//}
		if (hitFlags & ISPL_KNOCKBACK) {
			MonGetKnockback(mnum, plr._px, plr._py);
		}
		MonStartHit(mnum, pnum, dam, hitFlags);
	}
	return true;
}

static bool PlrHitPlr(int offp, int sn, int sl, int pnum)
{
	int hper, blkper, dam, damsl, dambl, dampc;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrHitPlr: illegal target player %d", pnum);
	}

	if ((unsigned)offp >= MAX_PLRS) {
		dev_fatal("PlrHitPlr: illegal attacking player %d", offp);
	}

	if (plx(offp)._pTeam == plr._pTeam || plr._pInvincible)
		return false;

	hper = plx(offp)._pIHitChance - plr._pIAC;
	if (sn == SPL_SWIPE) {
		hper -= 30 - sl * 2;
	}

	if (!CheckHit(hper))
		return false;

	blkper = plr._pIBlockChance;
	if (blkper != 0
	 && (plr._pmode == PM_STAND || plr._pmode == PM_BLOCK)) {
		// assert(plr._pSkillFlags & SFLAG_BLOCK);
		blkper = blkper - (plx(offp)._pLevel << 1);
		if (blkper > random_(5, 100)) {
			PlrStartBlock(pnum, OPPOSITE(plx(offp)._pdir));
			return true;
		}
	}

	dam = 0;
	damsl = plx(offp)._pISlMaxDam;
	if (damsl != 0)
		dam += CalcPlrDam(pnum, MISR_SLASH, plx(offp)._pISlMinDam, damsl);
	dambl = plx(offp)._pIBlMaxDam;
	if (dambl != 0)
		dam += CalcPlrDam(pnum, MISR_BLUNT, plx(offp)._pIBlMinDam, dambl);
	dampc = plx(offp)._pIPcMaxDam;
	if (dampc != 0)
		dam += CalcPlrDam(pnum, MISR_PUNCTURE, plx(offp)._pIPcMinDam, dampc);

	if (random_(6, sn == SPL_SWIPE ? 800 : 200) < plx(offp)._pICritChance) {
		dam <<= 1;
	}

	if (sn == SPL_SWIPE) {
		dam = (dam * (48 + sl)) >> 6;
	}
	if (plx(offp)._pILifeSteal != 0) {
		PlrIncHp(offp, (dam * plx(offp)._pILifeSteal) >> 7);
	}

	int fdam = plx(offp)._pIFMaxDam;
	if (fdam != 0) {
		fdam = CalcPlrDam(pnum, MISR_FIRE, plx(offp)._pIFMinDam, fdam);
	}
	int ldam = plx(offp)._pILMaxDam;
	if (ldam != 0) {
		ldam = CalcPlrDam(pnum, MISR_LIGHTNING, plx(offp)._pILMinDam, ldam);
	}
	int mdam = plx(offp)._pIMMaxDam;
	if (mdam != 0) {
		mdam = CalcPlrDam(pnum, MISR_LIGHTNING, plx(offp)._pIMMinDam, mdam);
	}
	int adam = plx(offp)._pIAMaxDam;
	if (adam != 0) {
		adam = CalcPlrDam(pnum, MISR_ACID, plx(offp)._pIAMinDam, adam);
	}
	if ((fdam | ldam | mdam | adam) != 0) {
		dam += fdam + ldam + mdam + adam;
		AddElementalExplosion(plr._px, plr._py, fdam, ldam, mdam, adam);
	}

	if (!PlrDecHp(pnum, dam, DMGTYPE_PLAYER))
		StartPlrHit(pnum, dam, false, plx(offp)._pdir);
	return true;
}

static bool PlrTryHit(int pnum, int sn, int sl, int dx, int dy)
{
	int mpo;

	mpo = dMonster[dx][dy];
	if (mpo != 0) {
		mpo = mpo >= 0 ? mpo - 1 : -(mpo + 1);
		return PlrHitMonst(pnum, sn, sl, mpo);
	}
	mpo = dPlayer[dx][dy];
	if (mpo != 0) {
		mpo = mpo >= 0 ? mpo - 1 : -(mpo + 1);
		return PlrHitPlr(pnum, sn, sl, mpo);
	}
	mpo = dObject[dx][dy];
	if (mpo != 0) {
		mpo = mpo >= 0 ? mpo - 1 : -(mpo + 1);
		if (objects[mpo]._oBreak == OBM_BREAKABLE) {
			OperateObject(pnum, mpo, false);
			return true;
		}
	}
	return false;
}

static void PlrDoAttack(int pnum)
{
	int dir, hitcnt;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrDoAttack: illegal player %d", pnum);
	}

	plr._pVar8++; // ATTACK_TICK
	if (plr._pIFlags & ISPL_FASTESTATTACK) {
		PlrStepAnim(pnum);
	} else if (plr._pIFlags & ISPL_FASTERATTACK) {
		if ((plr._pVar8 & 1) == 1)
			PlrStepAnim(pnum);
	} else if (plr._pIFlags & ISPL_FASTATTACK) {
		if ((plr._pVar8 & 3) == 2)
			PlrStepAnim(pnum);
	} else if (plr._pIFlags & ISPL_QUICKATTACK) {
		if ((plr._pVar8 & 7) == 4)
			PlrStepAnim(pnum);
	}
	if (plr._pAnimFrame < plr._pAFNum - 1)
		return;
	if (plr._pVar7 == 0) { // ATTACK_ACTION_PROGRESS
		plr._pVar7++;
		PlaySfxLoc(PS_SWING, plr._px, plr._py, 2);
	}
	if (plr._pAnimFrame == plr._pAFNum - 1) {
		return;
	}
	dir = plr._pdir;
	if (plr._pVar7 == 1) {
		plr._pVar7++;

		hitcnt = PlrTryHit(pnum, plr._pVar5, plr._pVar6, // ATTACK_SKILL_LEVEL
			plr._px + offset_x[dir], plr._py + offset_y[dir]);
		if (plr._pVar5 == SPL_SWIPE) {
			hitcnt += PlrTryHit(pnum, SPL_SWIPE, plr._pVar6,
				plr._px + offset_x[(dir + 1) & 7], plr._py + offset_y[(dir + 1) & 7]);

			hitcnt += PlrTryHit(pnum, SPL_SWIPE, plr._pVar6,
				plr._px + offset_x[(dir + 7) & 7], plr._py + offset_y[(dir + 7) & 7]);
		}

		if (hitcnt != 0) {
			WeaponDur(pnum, 40 - hitcnt * 8);
		}
	}
	assert(PlrAnimFrameLens[PA_ATTACK] == 1);
	if (plr._pAnimFrame < plr._pAFrames)
		return;

	//PlrStartStand(pnum);
	StartStand(pnum);
	//ClearPlrPVars(pnum);
}

static void PlrDoRangeAttack(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrDoRangeAttack: illegal player %d", pnum);
	}

	plr._pVar8++; // RATTACK_TICK
	if (plr._pIFlags & ISPL_FASTESTATTACK) {
		PlrStepAnim(pnum);
	} else if (plr._pIFlags & ISPL_FASTERATTACK) {
		if ((plr._pVar8 & 1) == 1)
			PlrStepAnim(pnum);
	} else if (plr._pIFlags & ISPL_FASTATTACK) {
		if ((plr._pVar8 & 3) == 2)
			PlrStepAnim(pnum);
	} else if (plr._pIFlags & ISPL_QUICKATTACK) {
		if ((plr._pVar8 & 7) == 4)
			PlrStepAnim(pnum);
	}
	if (plr._pAnimFrame < plr._pAFNum)
		return;

	if (!plr._pVar7) { // RATTACK_ACTION_PROGRESS
		plr._pVar7 = TRUE;
		AddMissile(plr._px, plr._py, plr._pVar1, plr._pVar2, plr._pdir, // RATTACK_TARGET_X, RATTACK_TARGET_X
			 spelldata[plr._pVar5].sMissile, MST_PLAYER, pnum, plr._pVar6); // RATTACK_SKILL_LEVEL

		WeaponDur(pnum, 40);
	}
	assert(PlrAnimFrameLens[PA_ATTACK] == 1);
	if (plr._pAnimFrame < plr._pAFrames)
		return;

	//PlrStartStand(pnum);
	StartStand(pnum);
	//ClearPlrPVars(pnum);
}

static void ShieldDur(int pnum)
{
	ItemStruct *pi;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("ShieldDur: illegal player %d", pnum);
	}

	pi = &plr._pInvBody[INVLOC_HAND_RIGHT];
	if (pi->_itype == ITYPE_SHIELD) {
		ReduceItemDur(pi, INVLOC_HAND_RIGHT, pnum);
//		return;
	}
//#ifdef HELLFIRE
//	pi = &plr._pInvBody[INVLOC_HAND_LEFT];
//	if (pi->_itype == ITYPE_STAFF)
//		ReduceItemDur(pi, INVLOC_HAND_LEFT, pnum);
//#endif
}

void PlrStartBlock(int pnum, int dir)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrStartBlock: illegal player %d", pnum);
	}

	if (plr._pHitPoints < (1 << 6)) {
		StartPlrKill(pnum, DMGTYPE_UNKNOWN); // BUGFIX: is this really necessary?
		return;
	}

	if (plr._pmode != PM_BLOCK) {
		assert(plr._pmode == PM_STAND);
		AssertFixPlayerLocation(pnum);
		StartBlock(pnum, dir);
	}

	PlaySfxLoc(IS_ISWORD, plr._px, plr._py);
	if (random_(3, 10) == 0) {
		ShieldDur(pnum);
	}
}

static void PlrDoBlock(int pnum)
{
	int extlen;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrDoBlock: illegal player %d", pnum);
	}
	if (plr._pVar1 != 0) { // BLOCK_EXTENSION
		plr._pVar1--;
		plr._pAnimCnt--;
		return;
	}

	if (plr._pIFlags & ISPL_FASTBLOCK) {
		PlrStepAnim(pnum);
	}

	if (plr._pAnimFrame > plr._pBFrames || (plr._pAnimFrame == plr._pBFrames && plr._pAnimCnt >= PlrAnimFrameLens[PA_BLOCK] - 1)) {
		if (plr.destAction == ACTION_BLOCK) {
			// extend the blocking animation TODO: does not work with too fast animations (WARRIORs) in faster/fastest games
			plr.destAction = ACTION_NONE;
			plr._pAnimData = plr._pBAnim[plr.destParam1];
			plr._pAnimFrame = plr._pBFrames;
			plr._pAnimCnt = PlrAnimFrameLens[PA_BLOCK] - 2;
			extlen = plr._pBFrames * 4;
			if (plr._pIFlags & ISPL_FASTBLOCK) {
				extlen >>= 1;
				if (extlen < 8)
					extlen = 8;
			}
			plr._pVar1 = extlen; // BLOCK_EXTENSION
		} else {
			//PlrStartStand(pnum);
			StartStand(pnum);
			//ClearPlrPVars(pnum);
		}
	}
}

static void ArmorDur(int pnum)
{
	ItemStruct *pi, *pio;
	BYTE loc;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("ArmorDur: illegal player %d", pnum);
	}

	loc = INVLOC_CHEST;
	pi = &plr._pInvBody[INVLOC_CHEST];
	pio = &plr._pInvBody[INVLOC_HEAD];
	if (pi->_itype == ITYPE_NONE) {
		if (pio->_itype == ITYPE_NONE)
			return; // neither chest nor head equipment
		// only head
		pi = pio;
		loc = INVLOC_HEAD;
	} else if (pio->_itype != ITYPE_NONE && random_(8, 3) == 0) {
		// head + chest -> 33% chance to damage the head
		pi = pio;
		loc = INVLOC_HEAD;
	}

	ReduceItemDur(pi, loc, pnum);
}

static void PlrDoSpell(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrDoSpell: illegal player %d", pnum);
	}
	plr._pVar8++; // SPELL_TICK
	if (plr._pIFlags & ISPL_FASTESTCAST) {
		PlrStepAnim(pnum);
	} else if (plr._pIFlags & ISPL_FASTERCAST) {
		if ((plr._pVar8 & 1) == 1)
			PlrStepAnim(pnum);
	} else if (plr._pIFlags & ISPL_FASTCAST) {
		if ((plr._pVar8 & 3) == 2)
			PlrStepAnim(pnum);
	}

	if (plr._pAnimFrame < plr._pSFNum)
		return;

	if (!plr._pVar7) { // SPELL_ACTION_PROGRESS
		plr._pVar7 = TRUE;

		AddMissile(plr._px, plr._py, plr._pVar1, plr._pVar2, plr._pdir, // SPELL_TARGET_X, SPELL_TARGET_Y
			spelldata[plr._pVar3].sMissile, MST_PLAYER, pnum, plr._pVar4); // SPELL_NUM, SPELL_LEVEL
	}
	assert(PlrAnimFrameLens[PA_SPELL] == 1);
	if (plr._pAnimFrame < plr._pSFrames)
		return;

	//PlrStartStand(pnum);
	StartStand(pnum);
	//ClearPlrPVars(pnum);
}

void KnockbackPlr(int pnum, int dir)
{
	int oldx, oldy, newx, newy;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrKnockback: illegal player %d", pnum);
	}

	assert(plr._pmode != PM_DEATH && plr._pmode != PM_DYING);

	if (plr._pmode != PM_GOTHIT)
		StartPlrHit(pnum, 0, true, dir);

	oldx = plr._px;
	oldy = plr._py;
	if (!PathWalkable(oldx, oldy, dir2pdir[dir]))
		return;

	newx = oldx + offset_x[dir];
	newy = oldy + offset_y[dir];
	if (PosOkPlayer(pnum, newx, newy)) {
		RemovePlrFromMap(pnum);
		plr._px = newx;
		plr._py = newy;
		ChangeLightXYOff(plr._plid, newx, newy);
		ChangeVisionXY(plr._pvid, newx, newy);
		dPlayer[newx][newy] = pnum + 1;
		FixPlayerLocation(pnum);
	}
}

static void PlrDoGotHit(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrDoGotHit: illegal player %d", pnum);
	}
	plr._pVar8++; // GOTHIT_TICK
	if (plr._pIFlags & ISPL_FASTESTRECOVER) {
		PlrStepAnim(pnum);
	} else if (plr._pIFlags & ISPL_FASTERRECOVER) {
		if ((plr._pVar8 & 1) == 1)
			PlrStepAnim(pnum);
	} else if (plr._pIFlags & ISPL_FASTRECOVER) {
		if ((plr._pVar8 & 3) == 2)
			PlrStepAnim(pnum);
	}
	assert(PlrAnimFrameLens[PA_GOTHIT] == 1);
	if (plr._pAnimFrame < plr._pHFrames)
		return;
	//PlrStartStand(pnum);
	StartStand(pnum);
	//ClearPlrPVars(pnum);
	if (random_(3, 4) != 0) {
		ArmorDur(pnum);
	}
}

static void PlrDoDeath(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrDoDeath: illegal player %d", pnum);
	}

	if (plr._pAnimFrame == plr._pAnimLen) {
		assert(PlrAnimFrameLens[PA_DEATH] > 1);
		plr._pAnimCnt = 0;
		if (plr._pVar7 > 0 && --plr._pVar7 == 0) { // DEATH_DELAY
			if (pnum == mypnum) {
				gbDeathflag = MDM_DEAD;
				if (!IsMultiGame) {
					gamemenu_on();
				}
			}
		}
		//dFlags[plr._px][plr._py] |= BFLAG_DEAD_PLAYER;
	}
}

static bool CheckNewPath(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("CheckNewPath: illegal player %d", pnum);
	}
	if (plr._pHitPoints < (1 << 6)) {
		StartPlrKill(pnum, DMGTYPE_UNKNOWN); // BUGFIX: is this really necessary?
		return false;
	}

	if (plr.destAction == ACTION_ATTACKMON) {
		MakePlrPath(pnum, monsters[plr.destParam1]._mfutx, monsters[plr.destParam1]._mfuty, false);
	} else if (plr.destAction == ACTION_ATTACKPLR) {
		MakePlrPath(pnum, plx(plr.destParam1)._pfutx, plx(plr.destParam1)._pfuty, false);
	}

	if (plr.walkpath[0] != DIR_NONE) {
		if (plr._pmode == PM_STAND) {
			/* commented out because this should not happen
			if (pnum == mypnum) {
				if (plr.destAction == ACTION_ATTACKMON || plr.destAction == ACTION_ATTACKPLR) {
					if (StartAttack(pnum)) {
						ClrPlrPath(pnum);
						plr.destAction = ACTION_NONE;
						return;
					}
				}
			}*/

			if (!StartWalk(pnum)) {
				//PlrStartStand(pnum);
				StartStand(pnum);
				plr.destAction = ACTION_NONE;
				return false;
			}
			return true;
		}

		return false;
	}
	if (plr.destAction == ACTION_NONE) {
		return false;
	}

	if (plr._pmode == PM_STAND) {
		switch (plr.destAction) {
		case ACTION_WALK:
			break;
		case ACTION_OPERATE:
		case ACTION_ATTACK:
		case ACTION_ATTACKMON:
		case ACTION_ATTACKPLR:
			StartAttack(pnum);
			break;
		case ACTION_RATTACK:
		case ACTION_RATTACKMON:
		case ACTION_RATTACKPLR:
			StartRangeAttack(pnum);
			break;
		case ACTION_SPELL:
		case ACTION_SPELLMON:
		case ACTION_SPELLPLR:
			StartSpell(pnum);
			break;
		case ACTION_BLOCK:
			StartBlock(pnum, plr.destParam1);
			break;
		case ACTION_PICKUPITEM:
		case ACTION_PICKUPAITEM:
			StartPickItem(pnum);
			break;
		case ACTION_TALK:
			StartTalk(pnum);
			break;
		default:
			ASSUME_UNREACHABLE
		}

		AssertFixPlayerLocation(pnum);
		plr.destAction = ACTION_NONE;

		return plr._pmode != PM_STAND;
	}

	if (plr._pmode == PM_ATTACK && plr._pAnimFrame > plr._pAFNum) {
		if (plr.destAction == ACTION_ATTACK
		 || plr.destAction == ACTION_ATTACKMON
		 || plr.destAction == ACTION_ATTACKPLR
		 || plr.destAction == ACTION_OPERATE) {
			StartAttack(pnum);
			plr.destAction = ACTION_NONE;
		}
	} else if (plr._pmode == PM_RATTACK && plr._pAnimFrame > plr._pAFNum) {
		if (plr.destAction == ACTION_RATTACK
		 || plr.destAction == ACTION_RATTACKMON
		 || plr.destAction == ACTION_RATTACKPLR) {
			StartRangeAttack(pnum);
			plr.destAction = ACTION_NONE;
		}
	} else if (plr._pmode == PM_SPELL && plr._pAnimFrame > plr._pSFNum) {
		if (plr.destAction == ACTION_SPELL
		 || plr.destAction == ACTION_SPELLMON
		 || plr.destAction == ACTION_SPELLPLR) {
			StartSpell(pnum);
			plr.destAction = ACTION_NONE;
		}
	}
	return false;
}

#if DEBUG_MODE || DEV_MODE
static void ValidatePlayer(int pnum)
{
	PlayerStruct *p;
	ItemStruct *pi;
	uint64_t msk;
	int gt, i;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("ValidatePlayer: illegal player %d", pnum);
	}
	p = &plr;
	//if (p->_pLevel > MAXCHARLEVEL)
	//	p->_pLevel = MAXCHARLEVEL;
	assert(p->_pLevel <= MAXCHARLEVEL);
	//if (p->_pExperience > p->_pNextExper)
	//	p->_pExperience = p->_pNextExper;
	assert(p->_pExperience <= p->_pNextExper);

	int numerrors = 0;
	gt = 0;
	pi = p->_pInvList;
	for (i = 0; i < NUM_INV_GRID_ELEM; i++, pi++) {
		if (pi->_itype == ITYPE_GOLD) {
			//if (pi->_ivalue > GOLD_MAX_LIMIT)
			//	pi->_ivalue = GOLD_MAX_LIMIT;
			assert(pi->_ivalue <= GOLD_MAX_LIMIT);
			gt += pi->_ivalue;
		} else if (numerrors == 0 && pi->_itype != ITYPE_NONE) {
			if (pi->_itype == ITYPE_PLACEHOLDER) {
				int par = pi->_iPHolder;
				if ((unsigned)par >= NUM_INV_GRID_ELEM) {
					snprintf(gbNetMsg, sizeof(gbNetMsg), "InvItem %d bad ph of %d", i, pnum);
					NetSendCmdString(0xFF);
					numerrors++;
				} else {
					ItemStruct* is = &p->_pInvList[par];
					if (is->_itype == ITYPE_NONE) {
						snprintf(gbNetMsg, sizeof(gbNetMsg), "InvItem %d null ph of %d", i, pnum);
						NetSendCmdString(0xFF);
						numerrors++;
					} else if (is->_itype == ITYPE_PLACEHOLDER) {
						snprintf(gbNetMsg, sizeof(gbNetMsg), "InvItem %d ph ph of %d", i, pnum);
						NetSendCmdString(0xFF);
						numerrors++;
					} else {
						int xx = InvItemWidth[is->_iCurs + CURSOR_FIRSTITEM] / INV_SLOT_SIZE_PX;
						int yy = InvItemHeight[is->_iCurs + CURSOR_FIRSTITEM] / INV_SLOT_SIZE_PX;
						int x = i % 10;
						int y = i / 10;
						int px = par % 10;
						int py = par / 10;
						if (x < px || x >= px + xx) {
							snprintf(gbNetMsg, sizeof(gbNetMsg), "InvI %d lx %d:%d par%d:%d s%d:%d of %d", i, x, y, px, py, xx, yy, pnum);
							NetSendCmdString(0xFF);
							numerrors++;
						} else if (y > py || y < py - yy) {
							snprintf(gbNetMsg, sizeof(gbNetMsg), "InvI %d ly %d:%d par%d:%d s%d:%d of %d", i, x, y, px, py, xx, yy, pnum);
							NetSendCmdString(0xFF);
							numerrors++;
						}
					}
				}
			} else {
				int xx = InvItemWidth[pi->_iCurs + CURSOR_FIRSTITEM] / INV_SLOT_SIZE_PX;
				int yy = InvItemWidth[pi->_iCurs + CURSOR_FIRSTITEM] / INV_SLOT_SIZE_PX;
				for (int jj = 0; jj < yy; jj++) {
					for (int ii = 0; ii < xx; ii++) {
						if (ii != 0 || jj != 0) {
							ItemStruct* is = &p->_pInvList[i + ii - jj * 10];
							if (is->_itype != ITYPE_PLACEHOLDER) {
								snprintf(gbNetMsg, sizeof(gbNetMsg), "InvItem %d no ph %d %d of %d", i, ii, jj, pnum);
								NetSendCmdString(0xFF);
								numerrors++;
								ii = xx;
								jj = yy;
							} else if (is->_iPHolder != i) {
								snprintf(gbNetMsg, sizeof(gbNetMsg), "InvItem %d bad ph %d %d vs %d of %d", i, ii, jj, is->_iPHolder, pnum);
								NetSendCmdString(0xFF);
								numerrors++;
								ii = xx;
								jj = yy;
							}
						}
					}
				}
			}
		}
	}
	//p->_pGold = gt;
	if (p->_pGold != gt) {
		snprintf(gbNetMsg, sizeof(gbNetMsg), "Gold %d vs calcGold %d of %d", p->_pGold, gt, pnum);
		NetSendCmdString(0xFF);
	}
	//assert(p->_pGold == gt);

	msk = 0;
	for (i = 1; i < NUM_SPELLS; i++) {
		if (spelldata[i].sBookLvl != SPELL_NA) {
			msk |= SPELL_MASK(i);
			//if (p->_pSkillLvlBase[i] > MAXSPLLEVEL)
			//	p->_pSkillLvlBase[i] = MAXSPLLEVEL;
			assert(p->_pSkillLvlBase[i] <= MAXSPLLEVEL);
		}
	}
	//p->_pMemSkills &= msk;
	assert((p->_pMemSkills & ~msk) == 0);
}
#endif

/*static void CheckCheatStats(int pnum)
{
	if (plr._pStrength > 750) {
		plr._pStrength = 750;
	}

	if (plr._pDexterity > 750) {
		plr._pDexterity = 750;
	}

	if (plr._pMagic > 750) {
		plr._pMagic = 750;
	}

	if (plr._pVitality > 750) {
		plr._pVitality = 750;
	}

	if (plr._pHitPoints > 128000) {
		plr._pHitPoints = 128000;
	}

	if (plr._pMana > 128000) {
		plr._pMana = 128000;
	}
}*/

void ProcessPlayers()
{
	int pnum;
	bool raflag;

	if ((unsigned)mypnum >= MAX_PLRS) {
		dev_fatal("ProcessPlayers: illegal player %d", mypnum);
	}

	if (gbLvlLoad > 0) {
		gbLvlLoad--;
	}
#ifndef NOSOUND
	if (gnSfxDelay > 0) {
		gnSfxDelay--;
		if (gnSfxDelay == 0) {
			InitQTextMsg(gnSfxNum);
		}
	}
#endif
	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (!plr._pActive || currLvl._dLevelIdx != plr._pDunLevel)
			continue;
		gbGameLogicPnum = pnum;
#if DEBUG_MODE || DEV_MODE
		ValidatePlayer(pnum);
#endif
		if (plr._pInvincible && plr._pmode != PM_DEATH && plr._pmode != PM_DYING) {
			if (plr._pLvlChanging)
				continue;
			if (plr.destAction != ACTION_NONE)
				plr._pInvincible = 0;
			else
				plr._pInvincible--;
		}
		{
			//CheckCheatStats(pnum);

			if (plr._pHitPoints < (1 << 6) && !plr._pInvincible) {
				StartPlrKill(pnum, DMGTYPE_UNKNOWN);
			}
			if ((plr._pIFlags & ISPL_DRAINLIFE) && currLvl._dLevelIdx != DLV_TOWN && !plr._pInvincible) {
				PlrDecHp(pnum, 4, DMGTYPE_NPC);
			}
			if (plr._pTimer[PLTR_INFRAVISION] != 0) {
				plr._pTimer[PLTR_INFRAVISION]--;
				if (plr._pTimer[PLTR_INFRAVISION] == 0) {
					CalcPlrItemVals(pnum, false); // last parameter should not matter
				}
			}

			int16_t nextTimer = plr._pTimer[PLTR_RAGE];
			if (nextTimer != 0) {
				plr._pTimer[PLTR_RAGE] = nextTimer + (nextTimer < 0 ? 1 : -1);
				if (plr._pTimer[PLTR_RAGE] == 0) {
					if (nextTimer >= 0) {
						plr._pTimer[PLTR_RAGE] = -RAGE_COOLDOWN_TICK;
						PlaySfxLoc(sgSFXSets[SFXS_PLR_72][plr._pClass], plr._px, plr._py);
					}
					CalcPlrItemVals(pnum, false); // last parameter should not matter
				}
			}

			do {
				switch (plr._pmode) {
				case PM_STAND:
					break;
				case PM_WALK:
				case PM_WALK2:
					PlrDoWalk(pnum);
					break;
				case PM_CHARGE:
					break;
				case PM_ATTACK:
					PlrDoAttack(pnum);
					break;
				case PM_RATTACK:
					PlrDoRangeAttack(pnum);
					break;
				case PM_BLOCK:
					PlrDoBlock(pnum);
					break;
				case PM_SPELL:
					PlrDoSpell(pnum);
					break;
				case PM_GOTHIT:
					PlrDoGotHit(pnum);
					break;
				case PM_DYING:
				case PM_DEATH:
					PlrDoDeath(pnum);
					break;
				case PM_NEWLVL:
					break;
				default:
					ASSUME_UNREACHABLE
				}
				raflag = CheckNewPath(pnum);
			} while (raflag);

			plr._pAnimCnt++;
			if (plr._pAnimCnt >= plr._pAnimFrameLen) {
				plr._pAnimCnt = 0;
				plr._pAnimFrame++;
				if (plr._pAnimFrame > plr._pAnimLen) {
					plr._pAnimFrame = 1;
				}
			}
		}
	}
	gbGameLogicPnum = 0;
}

void ClrPlrPath(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("ClrPlrPath: illegal player %d", pnum);
	}
	plr.walkpath[0] = DIR_NONE;
	//memset(plr.walkpath, DIR_NONE, sizeof(plr.walkpath));
}

void MissToPlr(int mi, bool hit)
{
	MissileStruct* mis;
	MonsterStruct* mon;
	int pnum, oldx, oldy, mpnum, dist, minbl, maxbl, dam, hper, blkper;
	unsigned hitFlags;
	bool ret;

	if ((unsigned)mi >= MAXMISSILES) {
		dev_fatal("MissToPlr: Invalid missile %d", mi);
	}
	mis = &missile[mi];
	pnum = mis->_miSource;
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("MissToPlr: Invalid player %d", pnum);
	}
	//dPlayer[plr._px][plr._py] = pnum + 1;
	/*assert(plr._pfutx == plr._px);
	assert(plr._poldx == plr._px);
	assert(plr._pfuty == plr._py);
	assert(plr._poldy == plr._py);
	assert(plr._pxoff == 0);
	assert(plr._pyoff == 0);
	if (pnum == mypnum)
		FixPlayerLocation(pnum);*/
	//ChangeLightXYOff(plr._plid, plr._px, plr._py);
	//ChangeVisionXY(plr._pvid, plr._px, plr._py);
	ChangeLightOff(plr._plid, 0, 0);
	if (!hit || plr._pHitPoints < (1 << 6)) {
		assert(plr._pdir == mis->_miDir);
		PlrStartStand(pnum);
		return;
	}
	//if (mis->_miSpllvl < 10)
		StartPlrHit(pnum, 0, true, OPPOSITE(plr._pdir));
	//else
	//	PlaySfxLoc(IS_BHIT, x, y);
	dist = (int)mis->_miDist - 24;
	if (dist < 0)
		return;
	if (dist > 32)
		dist = 32;
	minbl = plr._pIChMinDam;
	maxbl = plr._pIChMaxDam;
	if (maxbl != 0) {
		minbl = ((64 + dist) * minbl) >> 5;
		maxbl = ((64 + dist) * maxbl) >> 5;
	}

	oldx = mis->_mix;
	oldy = mis->_miy;
	mpnum = dMonster[oldx][oldy];
	if (mpnum > 0) {
		mpnum--;
		if (mpnum < MAX_MINIONS)
			return;
		//PlrHitMonst(pnum, SPL_CHARGE, mis->_miSpllvl, mpnum);
		mon = &monsters[mpnum];

		hper = mis->_miSpllvl * 16 - mon->_mArmorClass;
		if (!CheckHit(hper) && mon->_mmode != MM_STONE)
#if DEBUG_MODE
			if (!debug_mode_god_mode)
#endif
				return;

		if (CheckMonsterHit(mpnum, &ret))
			return;

		dam = CalcMonsterDam(mon->_mMagicRes, MISR_BLUNT, minbl, maxbl, false);

		//if (random_(151, 200) < plr._pICritChance)
		//	dam <<= 1;

		//if (pnum == mypnum) {
			mon->_mhitpoints -= dam;
		//}

		if (mon->_mhitpoints < (1 << 6)) {
			MonStartKill(mpnum, pnum);
		} else {
			hitFlags = plr._pIFlags;
			//if (hitFlags & ISPL_NOHEALMON)
			//	mon->_mFlags |= MFLAG_NOHEAL;

			if (hitFlags & ISPL_KNOCKBACK)
				MonGetKnockback(mpnum, plr._px, plr._py);

			MonStartHit(mpnum, pnum, dam, ISPL_STUN);
		}
		return;
	}
	mpnum = dPlayer[oldx][oldy];
	if (mpnum > 0) {
		mpnum--;
		//PlrHitPlr(pnum, SPL_CHARGE, mis->_miSpllvl, mpnum);
		if (plx(mpnum)._pTeam == plr._pTeam || plx(mpnum)._pInvincible)
			return;
		hper = mis->_miSpllvl * 16 - plx(mpnum)._pIAC;

		if (!CheckHit(hper))
			return;
		blkper = plx(mpnum)._pIBlockChance;
		if (blkper != 0
		 && (plx(mpnum)._pmode == PM_STAND || plx(mpnum)._pmode == PM_BLOCK)) {
			// assert(plr._pSkillFlags & SFLAG_BLOCK);
			blkper = blkper - (plr._pLevel << 1);
			if (blkper > random_(5, 100)) {
				PlrStartBlock(mpnum, OPPOSITE(plr._pdir));
				return;
			}
		}
		dam = CalcPlrDam(mpnum, MISR_BLUNT, minbl, maxbl);

		//if (random_(151, 200) < plr._pICritChance)
		//	dam <<= 1;
		if (!PlrDecHp(mpnum, dam, DMGTYPE_PLAYER))
			StartPlrHit(mpnum, dam, true, plr._pdir);
		return;
	}
}

bool PosOkActor(int x, int y)
{
	int oi;

	if ((nSolidTable[dPiece[x][y]] | dPlayer[x][y] | dMonster[x][y]) != 0)
		return false;

	oi = dObject[x][y];
	if (oi != 0) {
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		if (objects[oi]._oSolidFlag)
			return false;
	}

	return true;
}

bool PosOkPlayer(int pnum, int x, int y)
{
	int mpo;

	if (IN_DUNGEON_AREA(x, y) && /*dPiece[x][y] != 0 &&*/ !nSolidTable[dPiece[x][y]]) {
		mpo = dMonster[x][y];
		if (mpo != 0) {
			// additional checks commented out because a player (or a charging monster)
			// should not walk over a dying monster
			// (AddDead does not check dMonster before reseting its value)
			//if (mpo < 0) {
			//	return false;
			//}
			//if (monsters[mpo - 1]._mhitpoints >= (1 << 6)) {
				return false;
			//}
			//if (currLvl._dLevelIdx == DLV_TOWN) {
			//	return false;
			//}
		}
		mpo = dObject[x][y];
		if (mpo != 0) {
			mpo = mpo >= 0 ? mpo - 1 : -(mpo + 1);
			if (objects[mpo]._oSolidFlag) {
				return false;
			}
		}
		mpo = dPlayer[x][y];
		if (mpo != 0) {
			mpo = mpo >= 0 ? mpo - 1 : -(mpo + 1);
			// check commented out because a player should not walk over a dying player (looks bad)
			return mpo == pnum/*|| plx(mpo)._pHitPoints < (1 << 6)*/;
		}

		return true;
	}

	return false;
}

bool MakePlrPath(int pnum, int xx, int yy, bool endspace)
{
	int sx, sy, path;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("MakePlrPath: illegal player %d", pnum);
	}

	sx = plr._pfutx;
	sy = plr._pfuty;
	path = FindPath(PosOkPlayer, pnum, sx, sy, xx, yy, plr.walkpath);
	if (path < 0) {
		return false;
	}

	if (path != 0 && !endspace) {
		path--;
	}

	plr.walkpath[path] = DIR_NONE;
	return true;
}

void SyncPlrAnim(int pnum)
{
	PlayerStruct* p;
	BYTE** anim;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("SyncPlrAnim: illegal player %d", pnum);
	}
	p = &plr;
	switch (p->_pmode) {
	case PM_STAND:
	case PM_NEWLVL:
		p->_pAnimFrameLen = PlrAnimFrameLens[PA_STAND];
		p->_pAnimLen = p->_pNFrames;
		p->_pAnimWidth = p->_pNWidth;
		p->_pAnimXOffset = (p->_pNWidth - TILE_WIDTH) >> 1;
		anim = p->_pNAnim;
		break;
	case PM_WALK:
	case PM_WALK2:
	case PM_CHARGE: // TODO: this should be PA_SPELL + p->_pTAnim, but does not really matter...
		p->_pAnimFrameLen = PlrAnimFrameLens[PA_WALK];
		p->_pAnimLen = p->_pWFrames;
		p->_pAnimWidth = p->_pWWidth;
		p->_pAnimXOffset = (p->_pWWidth - TILE_WIDTH) >> 1;
		anim = p->_pWAnim;
		break;
	case PM_ATTACK:
	case PM_RATTACK:
		p->_pAnimFrameLen = PlrAnimFrameLens[PA_ATTACK];
		p->_pAnimLen = p->_pAFrames;
		p->_pAnimWidth = p->_pAWidth;
		p->_pAnimXOffset = (p->_pAWidth - TILE_WIDTH) >> 1;
		anim = p->_pAAnim;
		break;
	case PM_BLOCK:
		p->_pAnimFrameLen = PlrAnimFrameLens[PA_BLOCK];
		p->_pAnimLen = p->_pBFrames;
		p->_pAnimWidth = p->_pBWidth;
		p->_pAnimXOffset = (p->_pBWidth - TILE_WIDTH) >> 1;
		anim = p->_pBAnim;
		break;
	case PM_GOTHIT:
		p->_pAnimFrameLen = PlrAnimFrameLens[PA_GOTHIT];
		p->_pAnimLen = p->_pHFrames;
		p->_pAnimWidth = p->_pHWidth;
		p->_pAnimXOffset = (p->_pHWidth - TILE_WIDTH) >> 1;
		anim = p->_pHAnim;
		break;
	case PM_DYING:
	case PM_DEATH:
		p->_pAnimFrameLen = PlrAnimFrameLens[PA_DEATH];
		p->_pAnimLen = p->_pDFrames;
		p->_pAnimWidth = p->_pDWidth;
		p->_pAnimXOffset = (p->_pDWidth - TILE_WIDTH) >> 1;
		anim = p->_pDAnim;
		break;
	case PM_SPELL:
		p->_pAnimFrameLen = PlrAnimFrameLens[PA_SPELL];
		p->_pAnimLen = p->_pSFrames;
		p->_pAnimWidth = p->_pSWidth;
		p->_pAnimXOffset = (p->_pSWidth - TILE_WIDTH) >> 1;
		switch (spelldata[p->_pVar3].sType) { // SPELL_NUM
		case STYPE_FIRE:      anim = p->_pFAnim; break;
		case STYPE_LIGHTNING: anim = p->_pLAnim; break;
		case STYPE_MAGIC:     anim = p->_pTAnim; break;
		default: ASSUME_UNREACHABLE; anim = p->_pFAnim; break;
		}
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	p->_pAnimData = anim[p->_pdir];
}

/*void CheckStats(int pnum)
{
	const int *stats;

	if ((unsigned)pnum >= MAX_PLRS) {
		app_fatal("CheckStats: illegal player %d", pnum);
	}
	stats = MaxStats[plr._pClass];
	if (plr._pBaseStr > stats[ATTRIB_STR]) {
		plr._pBaseStr = stats[ATTRIB_STR];
	} else if (plr._pBaseStr < 0) {
		plr._pBaseStr = 0;
	}
	if (plr._pBaseMag > stats[ATTRIB_MAG]) {
		plr._pBaseMag = stats[ATTRIB_MAG];
	} else if (plr._pBaseMag < 0) {
		plr._pBaseMag = 0;
	}
	if (plr._pBaseDex > stats[ATTRIB_DEX]) {
		plr._pBaseDex = stats[ATTRIB_DEX];
	} else if (plr._pBaseDex < 0) {
		plr._pBaseDex = 0;
	}
	if (plr._pBaseVit > stats[ATTRIB_VIT]) {
		plr._pBaseVit = stats[ATTRIB_VIT];
	} else if (plr._pBaseVit < 0) {
		plr._pBaseVit = 0;
	}
}*/

void PlrSetHp(int pnum, int val)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrSetHp: illegal player %d", pnum);
	}
	plr._pHitPoints = val;
	plr._pHPBase = val + plr._pMaxHPBase - plr._pMaxHP;

	if (pnum == mypnum)
		gbRedrawFlags |= REDRAW_HP_FLASK;
}

void PlrSetMana(int pnum, int val)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrSetMana: illegal player %d", pnum);
	}
	if (plr._pIFlags & ISPL_NOMANA)
		val = 0;
	plr._pMana = val;
	plr._pManaBase = val - (plr._pMaxMana - plr._pMaxManaBase);

	if (pnum == mypnum)
		gbRedrawFlags |= REDRAW_MANA_FLASK;
}

void PlrFillHp(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrFillHp: illegal player %d", pnum);
	}
	plr._pHitPoints = plr._pMaxHP;
	plr._pHPBase = plr._pMaxHPBase;
	if (pnum == mypnum)
		gbRedrawFlags |= REDRAW_HP_FLASK;
}

void PlrFillMana(int pnum)
{
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("PlrSetMana: illegal player %d", pnum);
	}
	if (plr._pIFlags & ISPL_NOMANA)
		return;
	plr._pMana = plr._pMaxMana;
	plr._pManaBase = plr._pMaxManaBase;
	if (pnum == mypnum)
		gbRedrawFlags |= REDRAW_MANA_FLASK;
}

void PlrIncHp(int pnum, int hp)
{
	assert(hp >= 0);
	plr._pHitPoints += hp;
	if (plr._pHitPoints > plr._pMaxHP)
		plr._pHitPoints = plr._pMaxHP;
	plr._pHPBase += hp;
	if (plr._pHPBase > plr._pMaxHPBase)
		plr._pHPBase = plr._pMaxHPBase;
	if (pnum == mypnum)
		gbRedrawFlags |= REDRAW_HP_FLASK;
}

void PlrIncMana(int pnum, int mana)
{
	assert(mana >= 0);
	if (plr._pIFlags & ISPL_NOMANA)
		return;

	plr._pMana += mana;
	if (plr._pMana > plr._pMaxMana) {
		plr._pMana = plr._pMaxMana;
	}
	plr._pManaBase += mana;
	if (plr._pManaBase > plr._pMaxManaBase) {
		plr._pManaBase = plr._pMaxManaBase;
	}
	if (pnum == mypnum)
		gbRedrawFlags |= REDRAW_MANA_FLASK;
}

bool PlrDecHp(int pnum, int hp, int dmgtype)
{
	assert(hp >= 0);
	if (plr._pManaShield != 0) {
		static_assert(MAXSPLLEVEL <= 16, "PlrDecHp does not give bonus for high level manashield.");
		hp -= (hp * (std::min(plr._pManaShield, (BYTE)16))) >> 6;
		if (plr._pMana >= hp) {
			PlrDecMana(pnum, hp);
			return false;
		}
		hp -= plr._pMana;
		PlrSetMana(pnum, 0);
		if (pnum == mypnum)
			NetSendCmd(CMD_REMSHIELD);
	}
	plr._pHPBase -= hp;
	plr._pHitPoints -= hp;
	if (plr._pHitPoints < (1 << 6)) {
		StartPlrKill(pnum, dmgtype);
		return true;
	}
	if (pnum == mypnum)
		gbRedrawFlags |= REDRAW_HP_FLASK;
	return false;
}

void PlrDecMana(int pnum, int mana)
{
	assert(mana >= 0);
	if (plr._pIFlags & ISPL_NOMANA)
		return;

	plr._pMana -= mana;
	plr._pManaBase -= mana;
	if (pnum == mypnum) {
		if (plr._pMana <= 0 && plr._pManaShield != 0)
			NetSendCmd(CMD_REMSHIELD);
		gbRedrawFlags |= REDRAW_MANA_FLASK;
	}
}

void IncreasePlrStr(int pnum)
{
	int v;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("IncreasePlrStr: illegal player %d", pnum);
	}
	if (plr._pStatPts <= 0)
		return;
	plr._pStatPts--;
	switch (plr._pClass) {
	case PC_WARRIOR:	v = (((plr._pBaseStr - StrengthTbl[PC_WARRIOR]) % 5) == 2) ? 3 : 2; break;
	case PC_ROGUE:		v = 1; break;
	case PC_SORCERER:	v = 1; break;
#ifdef HELLFIRE
	case PC_MONK:		v = 2; break;
	case PC_BARD:		v = 1; break;
	case PC_BARBARIAN:	v = 3; break;
#endif
	default:
		ASSUME_UNREACHABLE
		break;
	}
	//plr._pStrength += v;
	plr._pBaseStr += v;

	CalcPlrInv(pnum, true);
}

void IncreasePlrMag(int pnum)
{
	int v, ms;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("IncreasePlrMag: illegal player %d", pnum);
	}
	if (plr._pStatPts <= 0)
		return;
	plr._pStatPts--;
	switch (plr._pClass) {
	case PC_WARRIOR:	v = 1; break;
	case PC_ROGUE:		v = 2; break;
	case PC_SORCERER:	v = 3; break;
#ifdef HELLFIRE
	case PC_MONK:		v = (((plr._pBaseMag - MagicTbl[PC_MONK]) % 3) == 1) ? 2 : 1; break;
	case PC_BARD:		v = (((plr._pBaseMag - MagicTbl[PC_BARD]) % 3) == 1) ? 2 : 1; break;
	case PC_BARBARIAN:	v = 1; break;
#endif
	default:
		ASSUME_UNREACHABLE
		break;
	}

	//plr._pMagic += v;
	plr._pBaseMag += v;

	ms = v << (6 + 1);

	plr._pMaxManaBase += ms;
	//plr._pMaxMana += ms;
	//if (!(plr._pIFlags & ISPL_NOMANA)) {
		plr._pManaBase += ms;
		//plr._pMana += ms;
	//}

	CalcPlrInv(pnum, true);
}

void IncreasePlrDex(int pnum)
{
	int v;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("IncreasePlrDex: illegal player %d", pnum);
	}
	if (plr._pStatPts <= 0)
		return;
	plr._pStatPts--;
	switch (plr._pClass) {
	case PC_WARRIOR:	v = (((plr._pBaseDex - DexterityTbl[PC_WARRIOR]) % 3) == 1) ? 2 : 1; break;
	case PC_ROGUE:		v = 3; break;
	case PC_SORCERER:	v = (((plr._pBaseDex - DexterityTbl[PC_SORCERER]) % 3) == 1) ? 2 : 1; break;
#ifdef HELLFIRE
	case PC_MONK:		v = 2; break;
	case PC_BARD:		v = 3; break;
	case PC_BARBARIAN:	v = 1; break;
#endif
	default:
		ASSUME_UNREACHABLE
		break;
	}

	//plr._pDexterity += v;
	plr._pBaseDex += v;

	CalcPlrInv(pnum, true);
}

void IncreasePlrVit(int pnum)
{
	int v, ms;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("IncreasePlrVit: illegal player %d", pnum);
	}
	if (plr._pStatPts <= 0)
		return;
	plr._pStatPts--;
	switch (plr._pClass) {
	case PC_WARRIOR:	v = 2; break;
	case PC_ROGUE:		v = 1; break;
	case PC_SORCERER:	v = (((plr._pBaseVit - VitalityTbl[PC_SORCERER]) % 3) == 1) ? 2 : 1; break;
#ifdef HELLFIRE
	case PC_MONK:		v = (((plr._pBaseVit - VitalityTbl[PC_MONK]) % 3) == 1) ? 2 : 1; break;
	case PC_BARD:		v = (((plr._pBaseVit - VitalityTbl[PC_BARD]) % 3) == 1) ? 2 : 1; break;
	case PC_BARBARIAN:	v = 2; break;
#endif
	default:
		ASSUME_UNREACHABLE
		break;
	}

	//plr._pVitality += v;
	plr._pBaseVit += v;

	ms = v << (6 + 1);

	plr._pHPBase += ms;
	plr._pMaxHPBase += ms;
	//plr._pHitPoints += ms;
	//plr._pMaxHP += ms;

	CalcPlrInv(pnum, true);
}

void RestorePlrHpVit(int pnum)
{
	int hp;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("RestorePlrHpVit: illegal player %d", pnum);
	}
	// base hp
	hp = plr._pBaseVit << (6 + 1);

	// check the delta
	hp -= plr._pMaxHPBase;
	assert(hp >= 0);

	// restore the lost hp
	plr._pMaxHPBase += hp;
	//plr._pMaxHP += hp;

	// fill hp
	plr._pHPBase = plr._pMaxHPBase;
	//PlrFillHp(pnum);

	CalcPlrInv(pnum, true);
}

DEVILUTION_END_NAMESPACE
